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
};

static std::vector<uint8_t> g_data0, g_arena0, g_data1, g_arena1;

static bool check(const rewrite::Entry *e, const Test &t, int iterations, uint64_t seed) {
    static const struct { unsigned short bits; const char *name; } fpu[] = {{0x000, "24"}, {0x200, "53"}, {0x300, "64"}};
    Rng rng{seed};
    for (int it = 0; it < iterations; it++) {
        t.setup(rng);
        if (t.data) memcpy(g_data0.data(), (void *)DATA, DATA_SIZE);
        memcpy(g_arena0.data(), g_arena, sizeof g_arena);
        for (auto &p : fpu) {
            set_fpu(p.bits);
            Result a = t.call((void *)e->at);
            if (t.data) memcpy(g_data1.data(), (void *)DATA, DATA_SIZE);
            memcpy(g_arena1.data(), g_arena, sizeof g_arena);
            if (t.data) memcpy((void *)DATA, g_data0.data(), DATA_SIZE);
            memcpy(g_arena, g_arena0.data(), sizeof g_arena);
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

static void stub(uintptr_t at, void *with) {
    uint8_t *p = (uint8_t *)at;
    p[0] = 0xe9;
    *(int32_t *)(p + 1) = (int32_t)((uintptr_t)with - (at + 5));
}

static void install_stubs() {
    stub(0xbce000, (void *)stub_free);
    stub(0xbce010, (void *)stub_free_aligned);
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
    for (Test *x : {&owned, &pair, &shared, &unlink, &base}) {
        x->iterations = 20;
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
    auto all = tests();
    // one line per rewrite; a rewrite of many copies is checked on each copy
    std::vector<const char *> done;
    for (const rewrite::Entry *e = rewrite::list(); e; e = e->next) {
        bool seen = false;
        for (auto n : done) if (!strcmp(n, e->name)) seen = true;
        if (seen) continue;
        done.push_back(e->name);
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
