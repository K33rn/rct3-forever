// equiv.exe: proves rewritten functions do exactly what the game's own ones do.
//
// usage: equiv.exe "<game folder>\\RCT3plus.exe" [iterations]
//
// It loads your RCT3plus.exe into memory at the address the game runs at (0x400000), the way Windows would but
// without starting it, so the game's own functions can be called there. For every rewrite that has a test below it
// builds random game state, runs the original, keeps what it returned and the memory it left (the game's whole
// .data section and the test objects), puts everything back, runs the rewrite, and compares, bit for bit. Each
// test runs with the FPU at 24-, 53- and 64-bit precision. No game code is part of this program; it reads it from
// your copy each time.
#include "../../forever/src/rewrite/rewrite.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <functional>

// ---- what forever.dll provides, for the rewrites ------------------------------------------------------------

void flog(const char *fmt, ...) {}
int setting(const char *, const char *, int def) { return def; }
bool replace_function(const char *, uintptr_t, const uint8_t *, size_t, void *) { return false; }
void *replace_import(uintptr_t, void *) { return nullptr; }

// ---- the game image -----------------------------------------------------------------------------------------

static const uint32_t TIMESTAMP = 0x434e4089, BASE = 0x400000, IMAGE_SIZE = 0x104c000;
static const uintptr_t DATA = 0x10e7000;
static const size_t DATA_SIZE = 0x339000;

// By the time main() runs, Windows has usually put something at 0x400000. So the tester starts a copy of itself
// held before its first instruction, takes the game's address range in it, and lets that copy do the work.
static int run_child(int argc, char **argv) {
    std::vector<char> cmd(GetCommandLineA(), GetCommandLineA() + strlen(GetCommandLineA()));
    const char *flag = " --child";
    cmd.insert(cmd.end(), flag, flag + strlen(flag) + 1);
    char self[MAX_PATH];
    GetModuleFileNameA(nullptr, self, MAX_PATH);
    STARTUPINFOA si = {sizeof si};
    PROCESS_INFORMATION pi;
    if (!CreateProcessA(self, cmd.data(), nullptr, nullptr, FALSE, CREATE_SUSPENDED, nullptr, nullptr, &si, &pi)) {
        printf("can't start the tester\n");
        return 2;
    }
    if (VirtualAllocEx(pi.hProcess, (void *)BASE, IMAGE_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE) !=
        (void *)BASE) {
        printf("0x400000 isn't free in the tester:\n");
        MEMORY_BASIC_INFORMATION m;
        for (uintptr_t a = BASE; a < BASE + IMAGE_SIZE && VirtualQueryEx(pi.hProcess, (void *)a, &m, sizeof m);
             a = (uintptr_t)m.BaseAddress + m.RegionSize)
            if (m.State != MEM_FREE)
                printf("  %08lx +%lx state %lx type %lx\n", (unsigned long)(uintptr_t)m.BaseAddress,
                       (unsigned long)m.RegionSize, m.State, m.Type);
        TerminateProcess(pi.hProcess, 2);
        return 2;
    }
    ResumeThread(pi.hThread);
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD code = 2;
    GetExitCodeProcess(pi.hProcess, &code);
    return (int)code;
}

static bool load_game(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) { printf("can't open %s\n", path); return false; }
    std::vector<uint8_t> file;
    fseek(f, 0, SEEK_END);
    file.resize(ftell(f));
    fseek(f, 0, SEEK_SET);
    fread(file.data(), 1, file.size(), f);
    fclose(f);
    auto dos = (IMAGE_DOS_HEADER *)file.data();
    auto nt = (IMAGE_NT_HEADERS32 *)(file.data() + dos->e_lfanew);
    if (nt->FileHeader.TimeDateStamp != TIMESTAMP) { printf("not the supported RCT3plus.exe\n"); return false; }
    uint8_t *img = (uint8_t *)BASE;   // committed by run_child
    memcpy(img, file.data(), nt->OptionalHeader.SizeOfHeaders);
    auto sec = IMAGE_FIRST_SECTION(nt);
    for (int i = 0; i < nt->FileHeader.NumberOfSections; i++)
        memcpy(img + sec[i].VirtualAddress, file.data() + sec[i].PointerToRawData,
               sec[i].SizeOfRawData < sec[i].Misc.VirtualSize ? sec[i].SizeOfRawData : sec[i].Misc.VirtualSize);
    return true;
}

// ---- random test data ---------------------------------------------------------------------------------------

struct Rng {
    uint64_t s;
    uint32_t next() { s ^= s << 13; s ^= s >> 7; s ^= s << 17; return (uint32_t)(s >> 16); }
    int32_t range(int32_t lo, int32_t hi) { return lo + (int32_t)(next() % (uint32_t)(hi - lo + 1)); }
    float real(float lo, float hi) { return lo + (hi - lo) * (next() & 0xffffff) / 16777216.0f; }
    // mostly ordinary values, sometimes any bit pattern (infinities, NaNs, denormals)
    float any_float() {
        if (next() % 8) return real(-1000, 1000);
        uint32_t b = next();
        float f;
        memcpy(&f, &b, 4);
        return f;
    }
};

// Test objects live here; it's compared too.
alignas(16) static uint8_t g_arena[1 << 16];
template <class T> static T *arena(size_t offset) { return (T *)(g_arena + offset); }

// ---- running original and rewrite ---------------------------------------------------------------------------

struct Result {
    uint32_t i = 0;
    long double f = 0;   // a float result as it came back in st(0)
    bool operator==(const Result &o) const { return i == o.i && !memcmp(&f, &o.f, 10); }
};

static void set_fpu(unsigned short precision_bits) {
    unsigned short cw;
    __asm__ volatile("fnstcw %0" : "=m"(cw));
    cw = (cw & ~0x300) | precision_bits;
    __asm__ volatile("fldcw %0" : : "m"(cw));
}

// A test for the rewrite at `at`; for a rewrite of many copies (REWRITE_COPIES) it runs on every copy.
struct Test {
    uintptr_t at;
    std::function<void(Rng &)> setup;
    std::function<Result(void *fn)> call;
    int iterations = 0;   // 0: the number given on the command line
    bool data = true;     // also compare the game's .data section (slow; not needed for helpers that only touch
                          // their arguments)
    // for template rewrites: set up for the copy being checked (its own constants), and undo that afterwards
    std::function<void(const rewrite::Entry *)> prepare, finish;
};

static std::vector<uint8_t> g_data0, g_arena0, g_data1, g_arena1;

// A test that crashes (in the original or the rewrite) is reported, and fails the run.
static const char *g_testing = "";
static int g_testing_case;
static const char *g_testing_which = "";
static LONG WINAPI crashed(EXCEPTION_POINTERS *x) {
    printf("  CRASHED %s, case %d, in the %s: exception %08lx at %p\n", g_testing, g_testing_case, g_testing_which,
           x->ExceptionRecord->ExceptionCode, x->ExceptionRecord->ExceptionAddress);
    ExitProcess(3);
}

static bool check_copy(const rewrite::Entry *e, const Test &t, int iterations, uint64_t seed);
static bool check(const rewrite::Entry *e, const Test &t, int iterations, uint64_t seed) {
    if (t.prepare) t.prepare(e);
    bool same = check_copy(e, t, iterations, seed);
    if (t.finish) t.finish(e);
    return same;
}

static bool check_copy(const rewrite::Entry *e, const Test &t, int iterations, uint64_t seed) {
    static const struct { unsigned short bits; const char *name; } fpu[] = {{0x000, "24"}, {0x200, "53"}, {0x300, "64"}};
    Rng rng{seed};
    if (getenv("EQUIV_VERBOSE")) printf("    0x%06x\n", (unsigned)e->at);
    for (int it = 0; it < iterations; it++) {
        t.setup(rng);
        if (t.data) memcpy(g_data0.data(), (void *)DATA, DATA_SIZE);
        memcpy(g_arena0.data(), g_arena, sizeof g_arena);
        for (auto &p : fpu) {
            set_fpu(p.bits);
            g_testing = e->name, g_testing_case = it, g_testing_which = "original";
            Result a = t.call((void *)e->at);
            if (t.data) memcpy(g_data1.data(), (void *)DATA, DATA_SIZE);
            memcpy(g_arena1.data(), g_arena, sizeof g_arena);
            if (t.data) memcpy((void *)DATA, g_data0.data(), DATA_SIZE);
            memcpy(g_arena, g_arena0.data(), sizeof g_arena);
            g_testing_which = "rewrite";
            Result b = t.call(e->with);
            bool same = a == b && (!t.data || !memcmp(g_data1.data(), (void *)DATA, DATA_SIZE)) &&
                        !memcmp(g_arena1.data(), g_arena, sizeof g_arena);
            if (t.data) memcpy((void *)DATA, g_data0.data(), DATA_SIZE);
            memcpy(g_arena, g_arena0.data(), sizeof g_arena);
            if (!same) {
                // (the C runtime's printf has no 80-bit long double, so the float results are shown as doubles)
                printf("  DIFFERENT %s, case %d, %s-bit FPU: original %08x / %.9g, rewrite %08x / %.9g%s\n",
                       e->name, it, p.name, a.i, (double)a.f, b.i, (double)b.f, a == b ? " (memory differs)" : "");
                set_fpu(0x200);
                return false;
            }
        }
    }
    set_fpu(0x200);
    return true;
}

template <class T> static T &at(uintptr_t a) { return *(T *)a; }

// ---- stand-ins for game functions the rewrites call -----------------------------------------------------------
// The game's memory functions can't run here (its C runtime isn't started), and what matters is what gets freed,
// in which order: these record each call at the end of the arena, which is compared.

static const size_t CALL_LOG = sizeof g_arena - 0x1000;   // {count, then (function, argument) pairs}

static void record(uint32_t fn, void *arg) {
    uint32_t *log = arena<uint32_t>(CALL_LOG);
    if (log[0] < 500) {
        log[1 + 2 * log[0]] = fn;
        log[2 + 2 * log[0]] = (uint32_t)(uintptr_t)arg;
        log[0]++;
    }
}
static void __cdecl stub_free(void *p) { record(0xbce000, p); }
static void __cdecl stub_free_aligned(void *p) { record(0xbce010, p); }

// allocations come from the arena in order (offset 0x2000 on; the next free offset is kept in the arena too)
static const size_t ALLOC_NEXT = sizeof g_arena - 0x1004, ALLOC_START = 0x2000;
static void *__cdecl stub_alloc_aligned(uint32_t size) {
    record(0xbcdfd0, (void *)(uintptr_t)size);
    uint32_t &next = *arena<uint32_t>(ALLOC_NEXT);
    if (next < ALLOC_START) next = ALLOC_START;
    void *p = arena<void>(next);
    next += (size + 15) & ~15u;
    return p;
}
static void __fastcall stub_release_handle(void *handle, void *, int32_t flag) {
    record(0x8055c0, handle);
    record(0x8055c0, (void *)(uintptr_t)flag);
}
static void *g_null_handle;
static void *__cdecl stub_null_handle() {
    record(0x805750, g_null_handle);
    return g_null_handle;
}
static void __fastcall stub_stream_read(void *stream, void *, uint8_t *into, int32_t size) {
    record(0x809210, stream);
    record(0x809210, (void *)(uintptr_t)size);
    for (int32_t i = 0; i < size; i++) into[i] = (uint8_t)(0xa5 ^ (i * 7));
}
static uint64_t __fastcall stub_resolve_id(void *loader, void *, uint32_t low, uint32_t high) {
    record(0x808840, loader);
    record(0x808840, (void *)(uintptr_t)low);
    record(0x808840, (void *)(uintptr_t)high);
    return ((uint64_t)(high ^ 0x55aa) << 32) | (low * 3 + 1);
}
static void *__stdcall stub_handle_for(uint32_t low, uint32_t high) {
    record(0x805650, (void *)(uintptr_t)low);
    record(0x805650, (void *)(uintptr_t)high);
    return g_null_handle;
}
// virtual functions of test objects: record who was called with what, return something recognisable
static uint32_t __fastcall stub_virtual0(void *self) {
    record(0x1, self);
    return 0x5ca1ab1e;
}
static void __fastcall stub_destroy(void *self, void *, int32_t flag) {
    record(0x2, self);
    record(0x2, (void *)(uintptr_t)flag);
}

// the save writer: record what is written (lengths and counts, 4 bytes at a time)
static void __fastcall stub_writer_write(void *writer, void *, const void *data, int32_t size) {
    record(0xbe4060, writer);
    record(0xbe4060, (void *)(uintptr_t)size);
    record(0xbe4060, (void *)(uintptr_t)(size == 4 ? *(const uint32_t *)data : 0));
}
// fields of saved structs
static int32_t __fastcall stub_field_size(void *self, void *, void *item) {
    record(0x3, self);
    record(0x3, item);
    return (int32_t)(((uintptr_t)self ^ (uintptr_t)item) & 0xfff);
}
static void __fastcall stub_field_save(void *self, void *, void *item, void *writer) {
    record(0x4, self);
    record(0x4, item);
    record(0x4, writer);
}
static int32_t __fastcall stub_own_size(void *self, void *, void *object) {
    record(0x5, self);
    record(0x5, object);
    return *arena<int32_t>(8);
}

static void stub(uintptr_t at, void *with) {
    uint8_t *p = (uint8_t *)at;
    p[0] = 0xe9;
    *(int32_t *)(p + 1) = (int32_t)((uintptr_t)with - (at + 5));
}

// a stand-in put over one game function for one copy's test, and taken off again
static uint8_t g_saved[5];
static uintptr_t g_saved_at;
static void stub(uintptr_t at, void *with);
static void stub_for_now(uintptr_t at, void *with) {
    memcpy(g_saved, (void *)at, 5);
    g_saved_at = at;
    stub(at, with);
}
static void unstub() {
    if (g_saved_at) memcpy((void *)g_saved_at, g_saved, 5);
    g_saved_at = 0;
}
// the destructor a deleting destructor calls, and the virtual slot 1 of a test object (its class's interfaces)
static void __fastcall stub_destructor(void *self) { record(0x10, self); }
static void *__fastcall stub_class_of(void *self) {
    record(0x11, self);
    return *arena<void *>(0x1f0);
}
static uintptr_t g_query_id;   // the global holding the id the copy being checked asks for

// start-up globals: the module list's start-up (fills {table, error} from the arena), the global's constructor
// 0xBCDF40 (takes that pair by value) and the game's atexit
struct StartResult {
    uint32_t table;
    int32_t error;
};
static StartResult *__fastcall stub_module_start(StartResult *out) {
    record(0x20, nullptr);
    out->table = arena<uint32_t>(0x10)[0];
    out->error = arena<int32_t>(0x10)[1];
    return out;
}
static void __fastcall stub_global_from_start(void *global, void *, StartResult r) {
    record(0x21, global);
    record(0x21, (void *)(uintptr_t)r.table);
    record(0x21, (void *)(uintptr_t)r.error);
}
static int __cdecl stub_atexit(void *fn) {
    record(0x22, fn);
    return 0;
}

static void install_stubs() {
    stub(0xbce000, (void *)stub_free);
    stub(0xbce010, (void *)stub_free_aligned);
    stub(0xbcdfd0, (void *)stub_alloc_aligned);
    stub(0x8055c0, (void *)stub_release_handle);
    stub(0x805750, (void *)stub_null_handle);
    stub(0x809210, (void *)stub_stream_read);
    stub(0x808840, (void *)stub_resolve_id);
    stub(0x805650, (void *)stub_handle_for);
    stub(0xbe4060, (void *)stub_writer_write);
    stub(0xbcdf40, (void *)stub_global_from_start);
    stub(0xddc870, (void *)stub_atexit);
}

// ---- the tests ----------------------------------------------------------------------------------------------

static std::vector<Test> tests() {
    std::vector<Test> t;
    auto u32 = [](Rng &r) { return r.next() ^ (r.next() << 16); };

    // Random
    t.push_back({0x475170, [=](Rng &r) { at<uint32_t>(0x10ec9e0) = 0; arena<uint32_t>(0)[0] = u32(r); },
                 [](void *fn) { ((void(__fastcall *)(uint32_t))fn)(*arena<uint32_t>(0)); return Result{}; }});
    t.push_back({0x475220, [=](Rng &r) { at<uint32_t>(0x10ec9e0) = u32(r); },
                 [](void *fn) { return Result{((uint32_t(__cdecl *)())fn)()}; }});
    t.push_back({0x4751e0, [=](Rng &r) { at<uint32_t>(0x10ec9e0) = u32(r); },
                 [](void *fn) { return Result{((uint32_t(__cdecl *)())fn)()}; }});
    t.push_back({0x475180, [=](Rng &r) { at<uint32_t>(0x10ec9e0) = u32(r); },
                 [](void *fn) { Result x; x.f = ((long double(__cdecl *)())fn)(); return x; }});

    // Park entrances: a list of up to 50 entrances in the arena
    auto entrances = [=](Rng &r) {
        int n = r.range(-2, 50);
        at<int32_t>(0x13061a8) = n;
        void **list = arena<void *>(0);
        for (int i = 0; i < 64; i++) list[i] = (void *)(uintptr_t)(0x1000 + i * 16);
        at<void **>(0x13061a4) = list;
        at<uint32_t>(0x10ec9e0) = u32(r);
        *arena<int32_t>(1024) = n > 0 ? r.range(0, n - 1) : 0;
    };
    t.push_back({0x708960, entrances, [](void *fn) { return Result{((uint32_t(__cdecl *)())fn)()}; }});
    t.push_back({0x708970, entrances, [](void *fn) {
                     return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(int32_t))fn)(*arena<int32_t>(1024))};
                 }});
    t.push_back({0x708980, entrances,
                 [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__cdecl *)())fn)()}; }});

    // Guest intensity: a guest object at arena + 0x100 (fields at +0x434..+0x43c)
    auto guest = [=](Rng &r) {
        uint8_t *g = arena<uint8_t>(0x100);
        *(int32_t *)(g + 0x434) = r.range(-2, 5);
        *(float *)(g + 0x438) = r.any_float();
        *(float *)(g + 0x43c) = r.any_float();
        *arena<int32_t>(0) = r.range(-3, 6);
        static const uint8_t flags[] = {0, 1, 2, 0xff};
        *arena<uint32_t>(4) = flags[r.next() % 4] | (r.next() & 0xffffff00);   // only the low byte is the flag
    };
    t.push_back({0x758bb0, guest, [](void *fn) {
                     ((void(__fastcall *)(void *, void *, int32_t, uint32_t))fn)(arena<void>(0x100), nullptr,
                                                                                  *arena<int32_t>(0), *arena<uint32_t>(4));
                     return Result{};
                 }});
    t.push_back({0x758c30, guest, [](void *fn) {
                     Result x;
                     x.f = ((long double(__fastcall *)(void *))fn)(arena<void>(0x100));
                     return x;
                 }});

    // Ride: the score log at [ride + 0x12c], 16 parts
    t.push_back({0x4bfc00,
                 [=](Rng &r) {
                     uint8_t *ride = arena<uint8_t>(0x100);
                     float *log = arena<float>(0x800);
                     *(float **)(ride + 0x12c) = log;
                     for (int i = 0; i < 16; i++) log[i] = r.any_float();
                     *arena<int32_t>(0) = r.range(0, 15);
                     *arena<float>(4) = r.any_float();
                 },
                 [](void *fn) {
                     ((void(__fastcall *)(void *, void *, int32_t, float))fn)(arena<void>(0x100), nullptr,
                                                                              *arena<int32_t>(0), *arena<float>(4));
                     return Result{};
                 }});

    // Interfaces: an object whose class description has `count` slots, some empty; sometimes no description,
    // sometimes no object
    static void *s_info;
    struct Info {
        static void *__fastcall get(void *) { return s_info; }
    };
    auto object = [=](uintptr_t id_global) {
        return [=](Rng &r) {
            void **vt = arena<void *>(0x200);
            vt[0] = nullptr;
            vt[1] = (void *)Info::get;
            *arena<void **>(0x100) = vt;
            uint8_t *info = arena<uint8_t>(0x300);
            int count = r.range(0, 12);
            void **slots = arena<void *>(0x400);
            *(void ***)(info + 0x14) = slots;
            *(int32_t *)(info + 0x18) = count;
            for (int i = 0; i < 16; i++) {
                uint8_t *e = arena<uint8_t>(0x500 + i * 0x20);
                *(int32_t *)(e + 0x10) = r.range(-64, 4096);
                slots[i] = r.next() % 4 ? e : nullptr;
            }
            s_info = r.next() % 8 ? info : nullptr;
            at<int32_t>(id_global) = r.range(-3, 14);
            *arena<uint32_t>(0) = r.next() % 10 ? 0x100 : 0;   // which object: the test one, or null
        };
    };
    auto lookup = [](void *fn) {
        void *obj = *arena<uint32_t>(0) ? arena<void>(*arena<uint32_t>(0)) : nullptr;
        return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(obj)};
    };
    t.push_back({0x49ac80, object(0x13cde74), lookup});
    t.push_back({0x4bd9d0, object(0x13d0680), lookup});

    // Helpers with many copies: each copy is checked, 20 cases each, arguments only
    auto block = [](Rng &r) -> void * { return r.next() % 3 ? arena<void>(0x400 + (r.next() % 64) * 16) : nullptr; };
    auto obj_call = [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; };
    auto fresh = [](Rng &r) {
        for (int i = 0; i < 64; i++) arena<uint32_t>(0x100)[i] = r.next();
        arena<uint32_t>(CALL_LOG)[0] = 0;
    };
    Test owned{0x410960, [=](Rng &r) { fresh(r); arena<void *>(0x100)[0] = block(r); }, obj_call};
    Test pair{0x408190, [=](Rng &r) { fresh(r); arena<void *>(0x100)[0] = block(r); arena<void *>(0x100)[1] = block(r); },
              obj_call};
    Test shared{0x4012e0,
                [=](Rng &r) {
                    fresh(r);
                    uint8_t *b = arena<uint8_t>(0x200);
                    *(int32_t *)b = r.range(0, 3);   // 1: this was the last reference
                    *(void **)(b + 0xc) = block(r);
                    arena<void *>(0x100)[0] = b;
                },
                obj_call};
    // a properly linked node between two neighbours (a broken list stops the game at a breakpoint, not tested)
    Test unlink{0x40a890,
                [=](Rng &r) {
                    fresh(r);
                    void **node = arena<void *>(0x100), **prev = arena<void *>(0x200), **next = arena<void *>(0x300);
                    prev[0] = (void *)(uintptr_t)r.next(); prev[1] = node;
                    next[0] = node; next[1] = (void *)(uintptr_t)r.next();
                    node[0] = prev; node[1] = next;
                },
                obj_call};
    Test base{0x441fc0, fresh, obj_call};

    // Arrays {data, capacity} of items of any size (up to 44 bytes); data null means capacity 0 (the game never
    // has items without data)
    auto array = [=](Rng &r) {
        fresh(r);
        *arena<uint32_t>(ALLOC_NEXT) = 0;
        int cap = r.range(0, 8);
        uint32_t *items = arena<uint32_t>(0x800);
        for (int i = 0; i < 0x400 / 4; i++) items[i] = r.next();
        arena<void *>(0x100)[0] = cap ? items : nullptr;
        arena<int32_t>(0x100)[1] = cap;
        *arena<int32_t>(0) = r.next() % 4 ? r.range(0, 10) : cap;
    };
    auto array_call = [](void *fn) {
        return Result{(uint32_t)((int32_t(__fastcall *)(void *, void *, int32_t))fn)(arena<void>(0x100), nullptr,
                                                                                     *arena<int32_t>(0))};
    };
    // one copy of each item size; the others of the same size are checked by the same test
    for (uintptr_t at : {0x417c20, 0x493860, 0x937710, 0x445340, 0x4bf730, 0x50c9f0, 0x80ea30, 0x454a40,
                         0x41b910, 0x468960, 0xb99590, 0x617370, 0x557ce0, 0xa33c90}) {
        Test x{at, array, array_call};
        x.iterations = 20;
        x.data = false;
        t.push_back(x);
    }

    // References: handles at 0x300 (current), 0x380 (the other reference's), 0x400 (the null handle), each with a
    // circular list whose head node is at handle + 0x40; the reference at 0x100 sits in the current handle's list
    auto handles = [=](Rng &r) {
        fresh(r);
        static void *vt[2] = {(void *)stub_virtual0, nullptr};
        static void *object_vt[1] = {(void *)stub_destroy};
        for (uint32_t h : {0x300u, 0x380u, 0x400u}) {
            uint8_t *hp = arena<uint8_t>(h);
            for (int i = 0; i < 8; i++) ((uint32_t *)hp)[i] = r.next();
            *(int32_t *)(hp + 8) = r.range(1, 5);
            *(void **)(hp + 0xc) = nullptr;
            void **head = (void **)(hp + 0x40);
            head[0] = head; head[1] = head;
            *(void ***)(hp + 0x20) = head;
        }
        g_null_handle = arena<void>(0x400);
        // the current handle's object: sometimes none
        void **object = arena<void *>(0x500);
        object[0] = object_vt;
        *(void **)arena<uint8_t>(0x30c) = r.next() % 3 ? object : nullptr;
        auto link = [](uint32_t ref, uint32_t handle) {
            void **node = arena<void *>(ref + 8), **head = arena<void *>(handle + 0x40);
            node[0] = head; node[1] = head; head[0] = node; head[1] = node;
            arena<void *>(ref)[1] = arena<void>(handle);
        };
        arena<void *>(0x100)[0] = vt;
        link(0x100, 0x300);
        arena<void *>(0x180)[0] = vt;
        link(0x180, 0x380);
    };
    Test assign{0x410100, handles, [](void *fn) {
                    return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *, void *, void *))fn)(
                        arena<void>(0x100), nullptr, arena<void>(0x180))};
                }};
    Test delete_target{0x490800, handles,
                       [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};

    // a reference field read from a save stream at 0x700, whose position is sometimes inside its buffer (read from
    // there) and sometimes not (read by the stream); the reference at 0x100 sits `offset` into the object
    Test load{0x44eeb0,
              [=](Rng &r) {
                  handles(r);
                  for (int i = 0; i < 64; i++) arena<uint32_t>(0x700)[i] = r.next();
                  uint8_t *stream = arena<uint8_t>(0x700);
                  int start = r.range(0, 20);
                  *(int32_t *)(stream + 4) = r.range(0, 40);
                  *(int32_t *)(stream + 8) = start;
                  *(int32_t *)(stream + 0xc) = start + r.range(0, 40);
                  int offset = r.range(4, 0x80);
                  *(int32_t *)arena<uint8_t>(0x640 + 0x28) = offset;
                  arena<void *>(0x600)[1] = arena<void>(0x640);
                  arena<uint32_t>(0)[0] = 0x100 - offset;
              },
              [](void *fn) {
                  return Result{((uint32_t(__fastcall *)(void *, void *, void *, void *, void *))fn)(
                      arena<void>(0x600), nullptr, arena<void>(arena<uint32_t>(0)[0]), arena<void>(0x700),
                      arena<void>(0x900))};
              }};

    // a new reference: joins the null handle's list
    Test construct{0x42e030, handles, [](void *fn) {
                       return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x100))};
                   }};

    // a pool set at 0x100 of 4 pools at 0x200 (20 bytes each), each with up to 3 blocks of up to 4 items and a free
    // list with room for all of them; freeing the last item of a pool frees its blocks
    Test pool_free{0x8501c0,
                   [=](Rng &r) {
                       fresh(r);
                       arena<void *>(0x100)[0] = arena<void>(0x200);
                       for (int k = 0; k < 4; k++) {
                           int32_t *p = arena<int32_t>(0x200 + 20 * k);
                           int per = r.range(0, 4), blocks = r.range(0, 3);
                           p[0] = per;
                           p[1] = (int32_t)(uintptr_t)arena<void>(0x300 + 0x10 * k);
                           for (int i = 0; i < 4; i++) arena<uint32_t>(0x300 + 0x10 * k)[i] = r.next();
                           p[2] = blocks;
                           p[3] = (int32_t)(uintptr_t)arena<void>(0x400 + 0x40 * k);
                           p[4] = per * blocks ? r.range(per * blocks - 2 > 0 ? per * blocks - 2 : 0, per * blocks - 1)
                                               : 0;
                       }
                       arena<int32_t>(0)[0] = r.range(0, 3);
                       arena<uint32_t>(0)[1] = r.next();
                   },
                   [](void *fn) {
                       ((void(__fastcall *)(void *, void *, int32_t, uint32_t))fn)(
                           arena<void>(0x100), nullptr, arena<int32_t>(0)[0], arena<uint32_t>(0)[1]);
                       return Result{};
                   }};

    // a struct array field at 0x600 (its fields: up to 3 at 0x680), the array {data, count} `offset` into the object
    // at 0x800, items at 0x3000; the save writer at 0xa00
    auto struct_array = [=](Rng &r) {
        fresh(r);
        static void *own_vt[3] = {nullptr, nullptr, (void *)stub_own_size};
        static void *field_vt[4] = {nullptr, nullptr, (void *)stub_field_size, (void *)stub_field_save};
        uint8_t *f = arena<uint8_t>(0x600);
        *(void **)f = own_vt;
        *(void **)(f + 0xc) = arena<void>(0x6c0);
        *(int32_t *)(f + 0x18) = r.range(0, 3);
        int offset = r.range(0, 0x40) * 4;
        *(int32_t *)(f + 0x28) = offset;
        for (int j = 0; j < 3; j++) {
            *(void **)arena<uint8_t>(0x680 + 0x10 * j) = field_vt;
            arena<void *>(0x6c0)[j] = arena<void>(0x680 + 0x10 * j);
        }
        uint8_t *array = arena<uint8_t>(0x800 + offset);
        *(void **)array = arena<void>(0x3000);
        *(int32_t *)(array + 4) = r.range(-1, 6);
        for (int i = 0; i < 16; i++) arena<uint32_t>(0xa00)[i] = r.next();
        *arena<int32_t>(8) = r.range(8, 0x1000);
    };
    auto struct_array_size = [](void *fn) {
        return Result{(uint32_t)((int32_t(__fastcall *)(void *, void *, void *))fn)(arena<void>(0x600), nullptr,
                                                                                   arena<void>(0x800))};
    };
    auto struct_array_save = [](void *fn) {
        ((void(__fastcall *)(void *, void *, void *, void *))fn)(arena<void>(0x600), nullptr, arena<void>(0x800),
                                                                 arena<void>(0xa00));
        return Result{};
    };
    // one copy of each struct size
    for (uintptr_t at : {0x63fa30, 0x4c17d0, 0x54cd80, 0x512d40, 0x455970, 0x460960, 0x44ec90, 0x425260, 0x512b30,
                         0x815ac0, 0x4c19e0, 0x46da20, 0x98cd50, 0x81d860, 0xb248b0, 0x6be040}) {
        Test x{at, struct_array, struct_array_save};
        x.iterations = 20;
        x.data = false;
        t.push_back(x);
    }
    for (uintptr_t at : {0x63fb60, 0x4c1900, 0x54ceb0, 0x512e70, 0x455aa0, 0x460a90, 0x44edc0, 0x425390, 0x512c60,
                         0x815bf0, 0x4c1b10, 0xa02020, 0x46db50, 0x98ce80, 0x81d990, 0xb249e0, 0x6be170}) {
        Test x{at, struct_array, struct_array_size};
        x.iterations = 20;
        x.data = false;
        t.push_back(x);
    }

    // deleting destructors: each copy calls its own destructor (stood in for while it is checked) and frees the
    // object when bit 0 of its flag is set
    Test deleting{0x402ff0,
                  [=](Rng &r) {
                      fresh(r);
                      arena<uint32_t>(0)[0] = r.next() & 3;
                  },
                  [](void *fn) {
                      return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *, void *, uint32_t))fn)(
                          arena<void>(0x100), nullptr, arena<uint32_t>(0)[0])};
                  }};
    deleting.prepare = [](const rewrite::Entry *e) {
        uint8_t *p = (uint8_t *)e->at;   // push esi; mov esi, ecx; call destructor
        stub_for_now(e->at + 8 + *(int32_t *)(p + 4), (void *)stub_destructor);
    };
    deleting.finish = [](const rewrite::Entry *) { unstub(); };

    // QueryInterface: a test object at 0x100 whose class (virtual slot 1) is sometimes missing; the class's table of
    // interfaces at 0x240 (some empty) with entries at 0x300; the id is set in the copy's own global (sometimes out
    // of range), and sometimes the object is null
    Test query{0x40a140,
               [=](Rng &r) {
                   fresh(r);
                   static void *vt[2] = {nullptr, (void *)stub_class_of};
                   arena<void *>(0x100)[0] = vt;
                   uint8_t *cls = arena<uint8_t>(0x200);
                   *arena<void *>(0x1f0) = r.next() % 5 ? cls : nullptr;
                   *(void **)(cls + 0x14) = arena<void>(0x240);
                   *(int32_t *)(cls + 0x18) = r.range(0, 8);
                   for (int i = 0; i < 8; i++) {
                       arena<void *>(0x240)[i] = r.next() % 4 ? arena<void>(0x300 + 0x20 * i) : nullptr;
                       *(int32_t *)arena<uint8_t>(0x300 + 0x20 * i + 0x10) = r.range(-64, 64) * 4;
                   }
                   *(int32_t *)g_query_id = r.range(0, 9);
                   arena<uint32_t>(0)[0] = r.next() % 6 ? 0x100 : 0;
               },
               [](void *fn) {
                   uint32_t o = arena<uint32_t>(0)[0];
                   return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(o ? arena<void>(o) : nullptr)};
               }};
    query.prepare = [](const rewrite::Entry *e) { g_query_id = *(uint32_t *)(e->at + 0x12); };

    // start-up globals: the copy's module start-up is stood in for while it is checked (sub esp, 8; mov ecx, esp;
    // call start); its result's error is sometimes -1 (no error)
    Test module{0xea85e0,
                [=](Rng &r) {
                    fresh(r);
                    arena<uint32_t>(0x10)[0] = r.next();
                    arena<int32_t>(0x10)[1] = r.next() % 2 ? -1 : r.range(0, 100);
                },
                [](void *fn) {
                    ((void(__cdecl *)())fn)();
                    return Result{};
                }};
    module.prepare = [](const rewrite::Entry *e) {
        stub_for_now(e->at + 10 + *(int32_t *)(e->at + 6), (void *)stub_module_start);
    };
    module.finish = [](const rewrite::Entry *) { unstub(); };

    // cached interfaces: the object at 0x4000 (its flag sometimes already set), the reference's handle at 0x3000,
    // the referenced object at 0x3100 (sometimes none) with the class and interfaces of the QueryInterface test
    static const uint32_t *cached_c;
    Test cached{0x4ab6f0,
                [=](Rng &r) {
                    fresh(r);
                    const uint32_t *c = cached_c;
                    for (int i = 0; i < 0x1400 / 4; i++) arena<uint32_t>(0x3000)[i] = r.next();
                    static void *vt[2] = {nullptr, (void *)stub_class_of};
                    uint8_t *self = arena<uint8_t>(0x4000);
                    self[c[1]] = r.next() % 4 == 0;
                    *(void **)(self + c[2]) = arena<void>(0x3000);
                    *(void **)arena<uint8_t>(0x3000 + c[3]) = r.next() % 5 ? arena<void>(0x3100) : nullptr;
                    arena<void *>(0x3100)[0] = vt;
                    uint8_t *cls = arena<uint8_t>(0x200);
                    *arena<void *>(0x1f0) = r.next() % 5 ? cls : nullptr;
                    *(void **)(cls + 0x14) = arena<void>(0x240);
                    *(int32_t *)(cls + 0x18) = r.range(0, 8);
                    for (int i = 0; i < 8; i++) {
                        arena<void *>(0x240)[i] = r.next() % 4 ? arena<void>(0x300 + 0x20 * i) : nullptr;
                        *(int32_t *)arena<uint8_t>(0x300 + 0x20 * i + 0x10) = r.range(-64, 64) * 4;
                    }
                    *(int32_t *)c[4] = r.range(0, 9);
                },
                [](void *fn) {
                    return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x4000))};
                }};
    cached.prepare = [](const rewrite::Entry *e) { cached_c = rewrite::cached_constants(e->at); };

    for (Test *x : {&owned, &pair, &shared, &unlink, &base, &assign, &delete_target, &load, &construct, &pool_free,
                    &deleting, &query, &module, &cached}) {
        x->iterations = (x == &deleting || x == &query || x == &module) ? 4 : 20;   // thousands of copies: fewer cases each
        x->data = false;
        t.push_back(*x);
    }

    // Attraction value at +0x70
    t.push_back({0xa88450, [=](Rng &r) { *(int32_t *)arena<uint8_t>(0x170) = (int32_t)u32(r); },
                 [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; }});

    // GraphedValue::Push: {data, index, capacity, count}, data of up to 64 floats
    t.push_back({0x611af0,
                 [=](Rng &r) {
                     uint8_t *g = arena<uint8_t>(0x100);
                     int cap = r.range(1, 64);
                     *(float **)g = arena<float>(0x800);
                     *(int32_t *)(g + 4) = r.range(0, cap - 1);
                     *(int32_t *)(g + 8) = cap;
                     *(int32_t *)(g + 12) = r.range(0, cap);
                     for (int i = 0; i < 64; i++) arena<float>(0x800)[i] = r.any_float();
                     *arena<float>(0) = r.any_float();
                 },
                 [](void *fn) {
                     ((void(__fastcall *)(void *, void *, float))fn)(arena<void>(0x100), nullptr, *arena<float>(0));
                     return Result{};
                 }});
    return t;
}

static const char *g_only = getenv("EQUIV_ONLY");   // only the rewrites whose name has this in it

int main(int argc, char **argv) {
    if (argc < 2) { printf("usage: equiv.exe \"<game folder>\\RCT3plus.exe\" [iterations]\n"); return 2; }
    if (strcmp(argv[argc - 1], "--child")) return run_child(argc, argv);
    argc--;
    setvbuf(stdout, nullptr, _IONBF, 0);
    if (!load_game(argv[1])) return 2;
    int iterations = argc > 2 ? atoi(argv[2]) : 300;
    g_data0.resize(DATA_SIZE); g_data1.resize(DATA_SIZE);
    g_arena0.resize(sizeof g_arena); g_arena1.resize(sizeof g_arena);
    // check the code before the stubs change any of it
    int bad = 0, ok = 0, untested = 0;
    for (const rewrite::Entry *e = rewrite::list(); e; e = e->next)
        if (memcmp((void *)e->at, e->original, e->original_len)) {
            printf("  %-34s 0x%06x  the exe's code there isn't the known version\n", e->name, (unsigned)e->at);
            bad++;
        }
    install_stubs();
    SetUnhandledExceptionFilter(crashed);
    auto all = tests();
    // one line per rewrite; a rewrite of many copies is checked on each copy
    std::vector<const char *> done;
    for (const rewrite::Entry *e = rewrite::list(); e; e = e->next) {
        bool seen = false;
        for (auto n : done) if (!strcmp(n, e->name)) seen = true;
        if (seen) continue;
        done.push_back(e->name);
        if (g_only && !strstr(e->name, g_only)) continue;
        const Test *t = nullptr;
        for (auto &x : all)
            for (const rewrite::Entry *c = rewrite::list(); c; c = c->next)
                if (c->at == x.at && !strcmp(c->name, e->name)) t = &x;
        int copies = 0;
        for (const rewrite::Entry *c = e; c; c = c->next) if (!strcmp(c->name, e->name)) copies++;
        if (!t) { printf("  %-34s 0x%06x  NO TEST\n", e->name, (unsigned)e->at); untested++; continue; }
        int n = t->iterations ? t->iterations : iterations;
        bool same = true;
        for (const rewrite::Entry *c = e; c && same; c = c->next)
            if (!strcmp(c->name, e->name)) same = check(c, *t, n, 0x9e3779b97f4a7c15ull ^ c->at);
        if (same && copies == 1)
            printf("  %-34s 0x%06x  same in %d cases x 3 FPU precisions\n", e->name, (unsigned)e->at, n);
        else if (same)
            printf("  %-34s %3d copies  each the same in %d cases x 3 FPU precisions\n", e->name, copies, n);
        same ? ok++ : bad++;
    }
    printf("%d the same, %d different, %d without a test\n", ok, bad, untested);
    return bad || untested ? 1 : 0;
}
