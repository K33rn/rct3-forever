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
static void *__fastcall stub_deleting(void *self, void *, uint32_t flags) {
    record(0x11, self);
    record(0x11, (void *)(uintptr_t)flags);
    return self;
}
static void __fastcall stub_plain_dtor(void *self) { record(0x12, self); }
static void *__fastcall stub_head(void *self) { record(0x40, self); return arena<void>(0x300); }
static void __fastcall stub_fastcall2(void *ecx, void *edx) { record(0x42, ecx); record(0x42, edx); }
static void *__fastcall stub_next(void *n) { record(0x41, n); return *(void **)n; }
static void __thiscall stub_thiscall2(void *self, uint32_t a0, uint32_t a1) { record(0x20, self); record(0x20, (void *)(uintptr_t)a0); record(0x20, (void *)(uintptr_t)a1); }
static void __thiscall stub_thiscall1(void *self, void *arg) { record(0x21, self); record(0x21, arg); }
static void *__fastcall stub_alloc_ecx(void *ecx) { record(0x22, ecx); return (void *)0x30001234; }
static void __thiscall stub_reserve(void *self, uint32_t n) { record(0x30, self); record(0x30, (void *)(uintptr_t)n); }   // a plain destructor, no flag argument
static void *__fastcall stub_class_of(void *self) {
    record(0x11, self);
    return *arena<void *>(0x1f0);
}
// loading arrays of structs: the item type's resize (gives the array data at 0x3000) and the fields' Load
static void *__fastcall stub_array_resize(void **array, void *, int32_t count) {
    record(0x30, (void *)(uintptr_t)count);
    array[0] = arena<void>(0x3000);
    array[1] = (void *)(uintptr_t)count;
    return (void *)(uintptr_t)count;
}
static void __fastcall stub_field_load(void *self, void *, void *item, void *stream, void *loader) {
    record(0x31, self);
    record(0x31, item);
    record(0x31, loader);
}
// the runtime's vector destructor iterator (destroys `count` items of `size` bytes) and a handle for an object
static void __stdcall stub_vector_destroy(void *items, uint32_t size, int32_t count, void *dtor) {
    record(0xddcf39, items);
    record(0xddcf39, (void *)(uintptr_t)size);
    record(0xddcf39, (void *)(uintptr_t)count);
    record(0xddcf39, dtor);
}
static void *__fastcall stub_handle_of(void *object) {
    record(0x805700, object);
    return arena<void>(0x380);
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

// start-up initialisers. The temporary String is on the stack, at a different address in the original and the
// rewrite: what is recorded is whether a call gets that temporary (1), not its address. The String's constructor
// gives either the temporary or another object (arena 0x24), the name registered is arena 0x20.
static void *g_string;
static void *__fastcall stub_string(void *self, void *, const char *text, int32_t flag) {
    g_string = self;
    record(0x40, (void *)text);
    record(0x40, (void *)(uintptr_t)flag);
    return *arena<uint32_t>(0x24) & 1 ? self : arena<void>(0x180);
}
static uint32_t __fastcall stub_register_name(void *s) {
    record(0x41, s == g_string ? (void *)1 : s);
    return *arena<uint32_t>(0x20);
}
static void __fastcall stub_string_free(void *self) { record(0x42, self == g_string ? (void *)1 : self); }
// memory for a class description or a section's name (arena 0x200; none when arena 0x28 says so)
static void *__cdecl stub_alloc(uint32_t size) {
    record(0xbcdfc0, (void *)(uintptr_t)size);
    return *arena<uint32_t>(0x28) % 4 ? arena<void>(0x200) : nullptr;
}
static void *__fastcall stub_class_description(void *self, void *, int32_t a, const char *name, void *factory) {
    record(0x50, self);
    record(0x50, (void *)(uintptr_t)a);
    record(0x50, (void *)name);
    record(0x50, factory);
    return *arena<uint32_t>(0x2c) & 1 ? self : arena<void>(0x280);
}
// the wide string: its count from arena 0x30 (1 or 2), its text pointer arena 0x34
static void *__fastcall stub_wide_string(int32_t *self, void *, const char *text) {
    record(0x60, self);
    record(0x60, (void *)text);
    self[0] = *arena<int32_t>(0x30);
    self[3] = *arena<int32_t>(0x34);
    return self;
}
// the section takes a reference to its name, or not (arena 0x38); sometimes the name it was given is replaced
// (arena 0x3c) by the wide string at 0x2c0, so whose count is dropped after is checked too
static void __fastcall stub_section(void *self, void *, int32_t **name, uint32_t kind) {
    record(0x61, self);
    record(0x61, *name);
    record(0x61, (void *)(uintptr_t)kind);
    (*name)[0] += *arena<int32_t>(0x38);
    if (*arena<uint32_t>(0x3c) % 4 == 0) *name = arena<int32_t>(0x2c0);
}

// stand-ins for one copy's test, several at a time
static std::vector<std::pair<uintptr_t, uint64_t>> g_saved_many;
template <class Range> static void stubs_for_now_range(const Range &list) {
    for (auto &s : list) {
        uint64_t was = 0;
        memcpy(&was, (void *)s.first, 5);
        g_saved_many.push_back({s.first, was});
        stub(s.first, s.second);
    }
}
static void stubs_for_now(std::initializer_list<std::pair<uintptr_t, void *>> list) { stubs_for_now_range(list); }
static void stubs_for_now(const std::vector<std::pair<uint32_t, void *>> &list) { stubs_for_now_range(list); }
static void unstub_many() {
    for (auto s = g_saved_many.rbegin(); s != g_saved_many.rend(); ++s) memcpy((void *)s->first, &s->second, 5);
    g_saved_many.clear();
}

// texts by name. Shared texts {references, ..., +0xC characters}: one from C text (made in the memory given, its
// count from arena 0x30), the shared empty one (0x240) and the one with the suffix added (0x2C0), their counts set
// by the test. The suffixed one is written to the result given, which is on the stack: what is recorded is the
// text a pointer points to, not the pointer.
static int32_t *__fastcall stub_text_from_c(int32_t *self, void *, const char *c) {
    record(0x70, self);
    record(0x70, (void *)c);
    self[0] = *arena<int32_t>(0x30);
    self[3] = *arena<int32_t>(0x34);
    return self;
}
static int32_t *__fastcall stub_shared_empty(char flag) {
    record(0x71, (void *)(uintptr_t)(uint8_t)flag);
    return arena<int32_t>(0x240);
}
static int32_t **__fastcall stub_text_concat(int32_t **self, void *, int32_t **out, const char *c) {
    record(0x72, *self);
    record(0x72, (void *)c);
    *out = arena<int32_t>(0x2c0);
    return out;
}
// the reference takes one on its name, or not (arena 0x38)
static void __fastcall stub_text_ref(void *self, void *, int32_t **name, void *texts) {
    record(0x73, self);
    record(0x73, *name);
    record(0x73, texts);
    (*name)[0] += *arena<int32_t>(0x38);
}
// GameText::Copy: the named reference is on the stack (recorded as 1, as the String above); its text (+8) is a
// wide C text at 0x300 or none (arena 0x3c)
static void *g_text_ref;
static void *__fastcall stub_named_text_ref(uint8_t *self, void *, const char *name, void *texts, uint32_t suffix) {
    g_text_ref = self;
    record(0x74, (void *)name);
    record(0x74, texts);
    record(0x74, (void *)(uintptr_t)suffix);
    *(void **)(self + 8) = *arena<uint32_t>(0x3c) % 3 ? arena<void>(0x300) : nullptr;
    return self;
}
static void *__fastcall stub_text_from_wide(void *self, void *, const wchar_t *w) {
    record(0x75, self);
    record(0x75, (void *)w);
    return self;
}
static void *__fastcall stub_wide_empty(char flag) {
    record(0x76, (void *)(uintptr_t)(uint8_t)flag);
    return arena<void>(0x260);
}
static void __fastcall stub_text_ref_free(void *self) { record(0x77, self == g_text_ref ? (void *)1 : self); }
// key commands: the default keys are made on the stack (recorded as 1, as the String above)
static void *g_keys;
static void *__fastcall stub_key_combination(void *self, void *, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    g_keys = self;
    record(0x78, (void *)(uintptr_t)a);
    record(0x78, (void *)(uintptr_t)b);
    record(0x78, (void *)(uintptr_t)c);
    record(0x78, (void *)(uintptr_t)d);
    return *arena<uint32_t>(0x24) & 1 ? self : arena<void>(0x180);
}
// start-up texts: the copy made of a text (0x280, its count from arena 0x30) and the NamedTextRef made from it
static void *__fastcall stub_text_copy(int32_t **self, void *, int32_t **from) {
    record(0x7a, *from);
    *self = arena<int32_t>(0x280);
    (*self)[0] = *arena<int32_t>(0x30);
    return self;
}
static void *__fastcall stub_named_from_text(void *self, void *, int32_t **name, void *texts, uint32_t suffix) {
    record(0x7b, self);
    record(0x7b, *name);
    record(0x7b, texts);
    record(0x7b, (void *)(uintptr_t)suffix);
    (*name)[0] += *arena<int32_t>(0x38);
    return self;
}
// memory from the arena in turn (as the aligned allocation above): each text made gets its own
static void *__cdecl stub_alloc_next(uint32_t size) {
    record(0xbcdfc0, (void *)(uintptr_t)size);
    uint32_t &next = *arena<uint32_t>(ALLOC_NEXT);
    if (next < ALLOC_START) next = ALLOC_START;
    void *p = arena<void>(next);
    next += (size + 15) & ~15u;
    return p;
}
// a missing interface reported: the source file and line
static void __fastcall stub_report_interface(const char *file, int32_t line) {
    record(0xbec8a0, (void *)file);
    record(0xbec8a0, (void *)(uintptr_t)line);
}
// a scenery behaviour registered: its name (recorded by what it points to) and factory
static void __fastcall stub_register_behaviour(void *self, void *, int32_t **name, void *factory) {
    record(0x7d, self);
    record(0x7d, *name);
    record(0x7d, factory);
    (*name)[0] += *arena<int32_t>(0x38);
}
// factories' constructors: what they're given; they give the object or another one (arena 0x24)
static void *__fastcall stub_ctor3(void *self, void *, uint32_t a, uint32_t b, uint32_t c) {
    record(0x7e, self);
    record(0x7e, (void *)(uintptr_t)a);
    record(0x7e, (void *)(uintptr_t)b);
    record(0x7e, (void *)(uintptr_t)c);
    return *arena<uint32_t>(0x24) & 1 ? self : arena<void>(0x180);
}
static void *__fastcall stub_ctor4(void *self, void *, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    record(0x7f, self);
    record(0x7f, (void *)(uintptr_t)a);
    record(0x7f, (void *)(uintptr_t)b);
    record(0x7f, (void *)(uintptr_t)c);
    record(0x7f, (void *)(uintptr_t)d);
    return *arena<uint32_t>(0x24) & 1 ? self : arena<void>(0x180);
}
// arrays of objects: the runtime's vector constructor iterator, and an item's assignment from another
static void __stdcall stub_vector_construct(void *items, uint32_t size, int32_t count, void *ctor, void *dtor) {
    record(0xddcf99, items);
    record(0xddcf99, (void *)(uintptr_t)size);
    record(0xddcf99, (void *)(uintptr_t)count);
    record(0xddcf99, ctor);
    record(0xddcf99, dtor);
}
static void *__fastcall stub_assign(void *self, void *, void *from) {
    record(0x80, self);
    record(0x80, from);
    return self;
}
// constructors with no argument or one, giving the object or another one (arena 0x24)
static void *__fastcall stub_ctor0(void *self) {
    record(0x81, self);
    return *arena<uint32_t>(0x24) & 1 ? self : arena<void>(0x180);
}
static void *__fastcall stub_ctor1(void *self, void *, uint32_t a) {
    record(0x82, self);
    record(0x82, (void *)(uintptr_t)a);
    return *arena<uint32_t>(0x24) & 1 ? self : arena<void>(0x180);
}
// a temporary reference on the stack: what it holds is recorded (its table, handle, value, flag, whether it points
// at itself), not where it is; its destructor takes it out of the handle's list again
static void record_reference(uint32_t fn, uint32_t *ref) {
    record(fn, (void *)(uintptr_t)ref[0]);
    record(fn, (void *)(uintptr_t)ref[1]);
    record(fn, (void *)(uintptr_t)(ref[4] == (uint32_t)(uintptr_t)ref));
    record(fn, (void *)(uintptr_t)ref[5]);
    record(fn, (void *)(uintptr_t)((uint8_t *)ref)[0x18]);
}
static void __fastcall stub_with_reference(void *self, void *, uint32_t *ref) {
    record(0x83, self);
    record_reference(0x83, ref);
}
static uint32_t __fastcall stub_reference_free(uint32_t *ref) {
    record_reference(0x84, ref);
    uint32_t **node = (uint32_t **)(ref + 2);
    node[0][1] = (uint32_t)(uintptr_t)node[1];
    node[1][0] = (uint32_t)(uintptr_t)node[0];
    return *arena<uint32_t>(0x20);
}
// an array's own resize, as Array::Reserve calls it
static uint32_t __fastcall stub_resize(void *self, void *, uint32_t n) {
    record(0x85, self);
    record(0x85, (void *)(uintptr_t)n);
    return n * 3;
}
// interface parts: the base constructor and an owner's activation (slot 1)
static void __fastcall stub_gui_base(void *self, void *, void *owner, uint32_t a, uint32_t b, uint32_t c) {
    record(0x86, self);
    record(0x86, owner);
    record(0x86, (void *)(uintptr_t)a);
    record(0x86, (void *)(uintptr_t)b);
    record(0x86, (void *)(uintptr_t)c);
}
static void __fastcall stub_activate(void *self) { record(0x87, self); }
// resource types: the base constructor, given the three texts (recorded by what they point to)
// objective condition names: the C text and wide text joined (into 0x2C0), and the text reference made from the copy
// (on the stack, recorded as 1; its text 0x300 or none)
static int32_t **__fastcall stub_text_join(int32_t **out, const char *c, int32_t **w) {
    record(0x88, (void *)c);
    record(0x88, *w);
    *out = arena<int32_t>(0x2c0);
    return out;
}
static void *__fastcall stub_named_ref_text(uint8_t *self, void *, int32_t **name, void *texts, uint32_t suffix) {
    g_text_ref = self;
    record(0x89, *name);
    record(0x89, texts);
    record(0x89, (void *)(uintptr_t)suffix);
    *(void **)(self + 8) = *arena<uint32_t>(0x3c) % 3 ? arena<void>(0x300) : nullptr;
    return self;
}
// destructors with a member: the member's field (for the test), and a member's destructor
static uint32_t g_member_field;
static void __fastcall stub_member_destructor(void *self) { record(0x8a, self); }
// constructors with two or five arguments
// a base class's constructor: sets its own table (so the order of the tables shows)
static void __fastcall stub_base_ctor(uint32_t *self) {
    record(0x8d, self);
    record(0x8d, (void *)(uintptr_t)self[0]);
    self[0] = 0xba5e0000;
}
static void *__fastcall stub_ctor2(void *self, void *, uint32_t a, uint32_t b) {
    record(0x8b, self);
    record(0x8b, (void *)(uintptr_t)a);
    record(0x8b, (void *)(uintptr_t)b);
    return *arena<uint32_t>(0x24) & 1 ? self : arena<void>(0x180);
}
static void *__fastcall stub_ctor5(void *self, void *, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    record(0x8c, self);
    uint32_t v[5] = {a, b, c, d, e};
    for (uint32_t x : v) record(0x8c, (void *)(uintptr_t)x);
    return *arena<uint32_t>(0x24) & 1 ? self : arena<void>(0x180);
}
// a Direct3D device's SetRenderState
static uint32_t __stdcall stub_set_render_state(void *device, uint32_t state, uint32_t value) {
    record(0x8e, device);
    record(0x8e, (void *)(uintptr_t)state);
    record(0x8e, (void *)(uintptr_t)value);
    return state * 7 + value;
}
// a free list's block maker: records the block size; gives the list an item (0x370), or none (then its free count
// is 0, as when the game runs out of memory for a block)
static void __fastcall stub_free_list_make(uint32_t *base, void *, uint32_t size) {
    record(0x8f, base);
    record(0x8f, (void *)(uintptr_t)size);
    if (*arena<uint32_t>(0x24) % 2) {
        base[4] = (uint32_t)(uintptr_t)arena<void>(0x370);
        *arena<void *>(0x370) = nullptr;
        base[6] += 1;
    } else {
        base[6] = 0;
    }
}
// singletons: memory (0x200, sometimes none) and a constructor that keeps the object in its global
static uint32_t g_singleton_global;
static void *__cdecl stub_alloc_some(uint32_t size) {
    record(0xbcdfc0, (void *)(uintptr_t)size);
    return *arena<uint32_t>(0x28) % 4 ? arena<void>(0x200) : nullptr;
}
static void *__fastcall stub_singleton_ctor(void *self, void *, uint32_t a, uint32_t b, uint32_t c) {
    record(0x90, self);
    record(0x90, (void *)(uintptr_t)a);
    record(0x90, (void *)(uintptr_t)b);
    record(0x90, (void *)(uintptr_t)c);
    *(void **)g_singleton_global = self;
    return self;
}
static uint32_t g_members[2];   // the fields of the members a destructor deletes
static uint32_t g_node;         // a listener's node field
// the runtime's vector constructor iterator without a destructor
static void __stdcall stub_vector_construct4(void *items, uint32_t size, int32_t count, void *ctor) {
    record(0x402b90, items);
    record(0x402b90, (void *)(uintptr_t)size);
    record(0x402b90, (void *)(uintptr_t)count);
    record(0x402b90, ctor);
}
// quadtrees: finding a node (records what it is given: the box's corner and size by value), a node's Add and
// MarkChanged
static int32_t __fastcall stub_find_node(int32_t zero, uint32_t depth, void *origin, void *size, uint32_t *corner,
                                         uint32_t *box) {
    record(0xcdb600, (void *)(uintptr_t)zero);
    record(0xcdb600, (void *)(uintptr_t)depth);
    record(0xcdb600, origin);
    record(0xcdb600, size);
    record(0xcdb600, (void *)(uintptr_t)corner[0]);
    record(0xcdb600, (void *)(uintptr_t)corner[1]);
    record(0xcdb600, (void *)(uintptr_t)box[0]);
    record(0xcdb600, (void *)(uintptr_t)box[1]);
    return *arena<int32_t>(0x20);
}
static uint32_t __fastcall stub_node_add(void *node, void *, void *item, void *a, void *b) {
    record(0x91, node);
    record(0x91, item);
    record(0x91, a);
    record(0x91, b);
    return 0x1234;
}
static void __fastcall stub_mark_changed(void *tree, void *, int32_t node) {
    record(0x92, tree);
    record(0x92, (void *)(uintptr_t)node);
}
// the forwarded method (one per slot of a part): records its slot, who and the two arguments
template <int SLOT> static uint32_t __fastcall stub_forward(void *self, void *, uint32_t a, uint32_t b) {
    record(0x93, (void *)(uintptr_t)SLOT);
    record(0x93, self);
    record(0x93, (void *)(uintptr_t)a);
    record(0x93, (void *)(uintptr_t)b);
    return 0xf0d2d3 + SLOT;
}
// behaviour create: the constructor (ecx + three args, edx ignored) and the link (ecx + owner); both give the object
static void *__fastcall stub_behaviour_ctor(void *self, void *, uint32_t a, uint32_t b, uint32_t c) {
    record(0x94, self);
    record(0x94, (void *)(uintptr_t)a);
    record(0x94, (void *)(uintptr_t)b);
    record(0x94, (void *)(uintptr_t)c);
    return self;
}
static void __fastcall stub_link(void *self, void *, void *owner) {
    record(0x95, self);
    record(0x95, owner);
}
static void *__fastcall stub_resource_base(void *self, void *, int32_t **engine, int32_t **extension, uint32_t one,
                                           int32_t **name) {
    record(0x7c, self);
    record(0x7c, *engine);
    record(0x7c, *extension);
    record(0x7c, (void *)(uintptr_t)one);
    record(0x7c, *name);
    return self;
}
static void __fastcall stub_key_command(void *self, void *, uint32_t group, uint32_t setting, uint32_t name,
                                        void *keys, uint32_t e, uint32_t f) {
    record(0x79, self);
    record(0x79, (void *)(uintptr_t)group);
    record(0x79, (void *)(uintptr_t)setting);
    record(0x79, (void *)(uintptr_t)name);
    record(0x79, keys == g_keys ? (void *)1 : keys);
    record(0x79, (void *)(uintptr_t)e);
    record(0x79, (void *)(uintptr_t)f);
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
    stub(0xddcf39, (void *)stub_vector_destroy);
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
    // reference constructors: a new reference at 0x600 (filled with noise), joining the null handle's list
    // (0x400) or the list of the handle 0x805700 gives for the object at 0x500 (0x380, which has a reference)
    Test reference_null{0x460440, [=](Rng &r) {
                            handles(r);
                            for (int i = 0; i < 8; i++) arena<uint32_t>(0x600)[i] = r.next();
                        },
                        [](void *fn) {
                            return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x600))};
                        }};
    Test reference_to{0x410330, [=](Rng &r) {
                          handles(r);
                          for (int i = 0; i < 8; i++) arena<uint32_t>(0x600)[i] = r.next();
                      },
                      [](void *fn) {
                          return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *, void *, void *))fn)(
                              arena<void>(0x600), nullptr, arena<void>(0x500))};
                      }};
    reference_to.prepare = [](const rewrite::Entry *) { stub_for_now(0x805700, (void *)stub_handle_of); };
    reference_to.finish = [](const rewrite::Entry *) { unstub(); };
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
    // list fields: as the struct arrays, the list's first node at the field's place in the object; nodes at 0x3000
    // (0x10 apart: next, ?, item), 0..5 of them before the last
    Test list_save{0x533250,
                   [=](Rng &r) {
                       struct_array(r);
                       int32_t offset = *arena<int32_t>(0x628);
                       int n = r.range(0, 5);
                       for (int k = 0; k <= n; k++) {
                           uint32_t *node = arena<uint32_t>(0x3000 + 0x10 * k);
                           node[0] = k < n ? (uint32_t)(uintptr_t)arena<void>(0x3010 + 0x10 * k) : 0;
                           node[2] = (uint32_t)(uintptr_t)arena<void>(0x3400 + 0x20 * k);
                       }
                       *(void **)arena<uint8_t>(0x800 + offset) = arena<void>(0x3000);
                   },
                   struct_array_save};
    list_save.iterations = 20;
    list_save.data = false;
    t.push_back(list_save);
    // EngineEntryField::Size: the field at 0x600 (fields at 0x680), its +0x28 a base pointer 0x640, the array
    // {data at 0x3000, count 0..6} `offset` into it; the fields' Size stubbed (as the struct-array size test)
    for (uintptr_t at : {(uintptr_t)0x460660, (uintptr_t)0x460870}) {
    Test entry_size{at,
                    [=](Rng &r) {
                        fresh(r);
                        static void *field_vt[3] = {nullptr, nullptr, (void *)stub_field_size};
                        uint8_t *f = arena<uint8_t>(0x600);
                        *(void **)(f + 0xc) = arena<void>(0x6c0);
                        *(int32_t *)(f + 0x18) = r.range(0, 3);
                        for (int j = 0; j < 3; j++) {
                            *(void **)arena<uint8_t>(0x680 + 0x10 * j) = field_vt;
                            arena<void *>(0x6c0)[j] = arena<void>(0x680 + 0x10 * j);
                        }
                        int offset = r.range(0, 0x40) * 4;
                        *(void **)(f + 0x28) = arena<uint8_t>(0x640) - offset;   // base + offset lands at 0x640
                        *(void **)arena<uint8_t>(0x640) = arena<void>(0x3000);
                        *(int32_t *)arena<uint8_t>(0x644) = r.range(-1, 6);
                        *arena<int32_t>(8) = r.range(8, 0x1000);
                        arena<uint32_t>(0x10)[0] = (uint32_t)offset;
                    },
                    [](void *fn) {
                        return Result{(uint32_t)((int32_t(__fastcall *)(void *, void *, int32_t))fn)(
                            arena<void>(0x600), nullptr, (int32_t)arena<uint32_t>(0x10)[0])};
                    }};
    entry_size.iterations = 20;
    entry_size.data = false;
    t.push_back(entry_size);
    }
    // one copy of each struct size
    for (uintptr_t at : {0x63fa30, 0x4c17d0, 0x54cd80, 0x512d40, 0x455970, 0x460960, 0x44ec90, 0x425260, 0x512b30,
                         0x815ac0, 0x4c19e0, 0x46da20, 0x98cd50, 0x81d860, 0xb248b0, 0x6be040, 0x460540, 0x460750}) {
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

    // struct-array loads: the field at 0x600 (its description at 0x640, offset 0..0xFC; fields at 0x680, up to 3),
    // the object at 0x800, the stream at 0xA00 (position sometimes inside its buffer), the loader 0x900; the
    // count is kept small so the items stay in the arena
    Test array_load{0x425470,
                    [=](Rng &r) {
                        fresh(r);
                        static void *field_vt[2] = {nullptr, (void *)stub_field_load};
                        uint8_t *f = arena<uint8_t>(0x600);
                        *(void **)(f + 4) = arena<void>(0x640);
                        int offset = r.range(0, 0x3f) * 4;
                        *(int32_t *)arena<uint8_t>(0x640 + 0x28) = offset;
                        *(void **)(f + 0xc) = arena<void>(0x6c0);
                        *(int32_t *)(f + 0x10) = r.range(0, 3);
                        for (int j = 0; j < 3; j++) {
                            *(void **)arena<uint8_t>(0x680 + 0x10 * j) = field_vt;
                            arena<void *>(0x6c0)[j] = arena<void>(0x680 + 0x10 * j);
                        }
                        for (int i = 0; i < 64; i++) arena<uint32_t>(0xa00)[i] = r.next();
                        uint8_t *s = arena<uint8_t>(0xa00);
                        int start = r.range(0, 20);
                        *(int32_t *)(s + 4) = r.range(0, 40);
                        *(int32_t *)(s + 8) = start;
                        *(int32_t *)(s + 0xc) = start + r.range(0, 40);
                        // the count comes from the stream: keep it small where it is read from the buffer
                        for (int k = 0; k < 64; k++) arena<int32_t>(0xa14)[k] = r.range(-1, 5);
                    },
                    [](void *fn) {
                        ((void(__fastcall *)(void *, void *, void *, void *, void *))fn)(
                            arena<void>(0x600), nullptr, arena<void>(0x800), arena<void>(0xa00), arena<void>(0x900));
                        return Result{};
                    }};
    array_load.prepare = [](const rewrite::Entry *e) {
        stub_for_now(rewrite::load_constants(e->at)[2], (void *)stub_array_resize);
    };
    array_load.finish = [](const rewrite::Entry *) { unstub(); };

    // global clean-ups: the copy's global points at an object at 0x100 (or is null) whose slot 0 records its call;
    // the global is game data, so each run sets it from the arena (0x1f8); it is saved and put back around each copy
    static uint32_t cleanup_g, cleanup_saved;
    Test cleanup{0x40e050,
                 [=](Rng &r) {
                     fresh(r);
                     static void *vt[1] = {(void *)stub_deleting};
                     arena<void *>(0x100)[0] = vt;
                     *arena<void *>(0x1f8) = r.next() % 4 ? arena<void>(0x100) : nullptr;
                 },
                 [](void *fn) {
                     *(void **)cleanup_g = *arena<void *>(0x1f8);
                     ((void(__cdecl *)())fn)();
                     return Result{*(uint32_t *)cleanup_g};
                 }};
    cleanup.prepare = [](const rewrite::Entry *e) {
        cleanup_g = rewrite::cleanup_global(e->at);
        cleanup_saved = *(uint32_t *)cleanup_g;
    };
    // freeing arrays of objects: the pointer at 0x100 is null or points at items at 0x204 (count at 0x200)
    Test array_free{0x420b40,
                    [=](Rng &r) {
                        fresh(r);
                        *arena<int32_t>(0x200) = r.range(0, 6);
                        *arena<void *>(0x100) = r.next() % 4 ? arena<void>(0x204) : nullptr;
                    },
                    [](void *fn) {
                        ((void(__fastcall *)(void *))fn)(arena<void>(0x100));
                        return Result{*arena<uint32_t>(0x100)};
                    }};
    // table setters: an object at 0x100 filled with noise
    Test set_table{0x402a40, [=](Rng &r) { fresh(r); },
                   [](void *fn) {
                       ((void(__fastcall *)(void *))fn)(arena<void>(0x100));
                       return Result{};
                   }};
    cleanup.finish = [](const rewrite::Entry *) { *(uint32_t *)cleanup_g = cleanup_saved; };

    // start-up initialisers: the game functions they call are stood in for while each copy is checked; the global
    // they set is game data, so it is the result, and it is put back after each copy
    static uint32_t start_g, start_saved;
    auto start_setup = [=](Rng &r) {
        fresh(r);
        for (int i = 0; i < 8; i++) arena<uint32_t>(0x20)[i] = r.next();
        *arena<int32_t>(0x30) = r.range(1, 2);
        *arena<int32_t>(0x38) = r.range(0, 1);
        *arena<int32_t>(0x2c0) = r.range(1, 2);
        arena<int32_t>(0x2c0)[3] = (int32_t)r.next();
        *arena<uint32_t>(0x28) |= 1;   // a section's name always gets memory (the original crashes without)
    };
    auto start_call = [](void *fn) {
        ((void(__cdecl *)())fn)();
        return Result{*(uint32_t *)start_g};
    };
    auto start_finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)start_g = start_saved;
    };
    Test start_name{0xea8790, [=](Rng &r) { fresh(r); for (int i = 0; i < 8; i++) arena<uint32_t>(0x20)[i] = r.next(); },
                    start_call};
    start_name.prepare = [](const rewrite::Entry *e) {
        start_g = rewrite::start_constants(e->at)[2];
        start_saved = *(uint32_t *)start_g;
        stubs_for_now({{0xbec4b0, (void *)stub_string}, {0xbeca70, (void *)stub_register_name},
                       {0xbec5e0, (void *)stub_string_free}});
    };
    start_name.finish = start_finish;
    Test start_class{0xea86c0, [=](Rng &r) { fresh(r); for (int i = 0; i < 8; i++) arena<uint32_t>(0x20)[i] = r.next(); },
                     start_call};
    start_class.prepare = [](const rewrite::Entry *e) {
        start_g = rewrite::start_constants(e->at)[3];
        start_saved = *(uint32_t *)start_g;
        stubs_for_now({{0xbcdfc0, (void *)stub_alloc}, {0x805fd0, (void *)stub_class_description}});
    };
    start_class.finish = start_finish;
    Test start_section{0xea93c0, start_setup, start_call};
    start_section.prepare = [](const rewrite::Entry *e) {
        start_g = rewrite::start_constants(e->at)[3];
        start_saved = *(uint32_t *)start_g;
        stubs_for_now({{0xbcdfc0, (void *)stub_alloc}, {0xbcf850, (void *)stub_wide_string},
                       {0x46fad0, (void *)stub_section}});
    };
    start_section.finish = start_finish;

    // NamedTextRef: the reference at 0x100 (noise), its name C text at 0x340 or none, the texts table 0x380, with
    // the suffix or not (any value: only its low byte counts); the counts of the texts made are 1 or 2
    auto text_counts = [](Rng &r) {
        *arena<int32_t>(0x30) = r.range(1, 2);
        *arena<int32_t>(0x34) = (int32_t)r.next();
        *arena<int32_t>(0x38) = r.range(0, 1);
        *arena<int32_t>(0x240) = r.range(1, 2);
        arena<int32_t>(0x240)[3] = (int32_t)r.next();
        *arena<int32_t>(0x2c0) = r.range(1, 2);
        arena<int32_t>(0x2c0)[3] = (int32_t)r.next();
    };
    Test named_text_ref{0x403f30,
                        [=](Rng &r) {
                            fresh(r);
                            text_counts(r);
                            *arena<uint32_t>(0x28) = 1;   // memory for the name (the original crashes without)
                            arena<uint32_t>(0x10)[0] = r.next() % 4 ? 0x340 : 0;
                            arena<uint32_t>(0x10)[1] = r.next() % 2 ? r.next() & 0xff00 : r.next();
                        },
                        [](void *fn) {
                            uint32_t name = arena<uint32_t>(0x10)[0];
                            return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *, void *, void *, void *,
                                                                                       uint32_t))fn)(
                                arena<void>(0x100), nullptr, name ? arena<void>(name) : nullptr, arena<void>(0x380),
                                arena<uint32_t>(0x10)[1])};
                        }};
    named_text_ref.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)stub_alloc}, {0xbcfb10, (void *)stub_text_from_c},
                       {0xbd0520, (void *)stub_shared_empty}, {0x403e10, (void *)stub_text_concat},
                       {0xbd15a0, (void *)stub_text_ref}});
    };
    named_text_ref.finish = [](const rewrite::Entry *) { unstub_many(); };
    // GameText::Copy: the result at 0x180; memory for the copy sometimes runs out
    Test game_text_copy{0x41f210,
                        [=](Rng &r) {
                            fresh(r);
                            for (int i = 0; i < 8; i++) arena<uint32_t>(0x20)[i] = r.next();
                        },
                        [](void *fn) {
                            return Result{(uint32_t)(uintptr_t)((void *(__stdcall *)(void *))fn)(arena<void>(0x180))};
                        }};
    game_text_copy.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0x4040f0, (void *)stub_named_text_ref}, {0xbcdfc0, (void *)stub_alloc},
                       {0xbcf7d0, (void *)stub_text_from_wide}, {0xbd0490, (void *)stub_wide_empty},
                       {0xbd1880, (void *)stub_text_ref_free}});
    };
    game_text_copy.finish = [](const rewrite::Entry *) { unstub_many(); };
    // NamedTextRef from a text: the reference at 0x100, its name the text at 0x240 (held at 0x1e0), the texts
    // table 0x380, with the suffix or not
    Test text_ref_from_text{0x403e50,
                            [=](Rng &r) {
                                fresh(r);
                                text_counts(r);
                                *arena<void *>(0x1e0) = arena<void>(0x240);
                                arena<uint32_t>(0x10)[1] = r.next() % 2 ? r.next() & 0xff00 : r.next();
                            },
                            [](void *fn) {
                                return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *, void *, void *,
                                                                                           void *, uint32_t))fn)(
                                    arena<void>(0x100), nullptr, arena<void>(0x1e0), arena<void>(0x380),
                                    arena<uint32_t>(0x10)[1])};
                            }};
    text_ref_from_text.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0x403e10, (void *)stub_text_concat}, {0xbd15a0, (void *)stub_text_ref}});
    };
    text_ref_from_text.finish = [](const rewrite::Entry *) { unstub_many(); };
    // start-up key commands
    Test start_key{0xeb2580, [=](Rng &r) { fresh(r); for (int i = 0; i < 8; i++) arena<uint32_t>(0x20)[i] = r.next(); },
                   [](void *fn) {
                       ((void(__cdecl *)())fn)();
                       return Result{};
                   }};
    start_key.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbd1940, (void *)stub_key_combination}, {0x6b2720, (void *)stub_key_command}});
    };
    start_key.finish = [](const rewrite::Entry *) { unstub_many(); };
    // start-up texts (memory for the text always there: the original crashes without)
    Test start_text{0xeca490, [=](Rng &r) { fresh(r); text_counts(r); *arena<uint32_t>(0x28) = 1; },
                    [](void *fn) {
                        ((void(__cdecl *)())fn)();
                        return Result{};
                    }};
    start_text.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)stub_alloc}, {0xbcf850, (void *)stub_text_from_c},
                       {0x403be0, (void *)stub_text_concat}, {0xbd01c0, (void *)stub_text_copy},
                       {0x410c90, (void *)stub_named_from_text}});
    };
    start_text.finish = [](const rewrite::Entry *) { unstub_many(); };
    // resource types: the object at 0x100; each text in memory of its own (0x2000 on), its count from arena 0x30
    // (memory always there: the original crashes without)
    Test resource_type{0xbb28e0, [=](Rng &r) { fresh(r); text_counts(r); *arena<uint32_t>(0x28) = 1; },
                       [](void *fn) {
                           return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x100))};
                       }};
    resource_type.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)stub_alloc_next}, {0xbcf850, (void *)stub_text_from_c},
                       {0xc2ffb0, (void *)stub_resource_base}});
    };
    resource_type.finish = [](const rewrite::Entry *) { unstub_many(); };
    // QueryInterface that reports a miss: the QueryInterface test's object and class, a source file (0x3f0) or
    // none, and a line
    Test query_report{0x42a430,
                      [=](Rng &r) {
                          query.setup(r);
                          arena<uint32_t>(0x10)[0] = r.next() % 4 ? 0x3f0 : 0;
                          arena<uint32_t>(0x10)[1] = r.next();
                      },
                      [](void *fn) {
                          uint32_t o = arena<uint32_t>(0)[0], file = arena<uint32_t>(0x10)[0];
                          return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *, void *, int32_t))fn)(
                              o ? arena<void>(o) : nullptr, file ? arena<void>(file) : nullptr,
                              arena<int32_t>(0x10)[1])};
                      }};
    query_report.prepare = [](const rewrite::Entry *e) {
        g_query_id = rewrite::start_constants(e->at)[1];
        stubs_for_now({{0xbec8a0, (void *)stub_report_interface}});
    };
    query_report.finish = [](const rewrite::Entry *) { unstub_many(); };
    // wide texts from globals: the global (game data, put back after each copy) points at 0x300 or holds nothing;
    // the object's text (+8) is 0x340 or none; the result at 0x180; memory sometimes runs out
    static uint32_t wide_g, wide_saved;
    auto wide_setup = [=](Rng &r) {
        fresh(r);
        *arena<uint32_t>(0x28) = r.next();
        *(void **)wide_g = r.next() % 4 ? arena<void>(0x300) : nullptr;
        *arena<void *>(0x308) = r.next() % 4 ? arena<void>(0x340) : nullptr;
    };
    auto wide_prepare = [](const rewrite::Entry *e) {
        wide_g = rewrite::start_constants(e->at)[1];
        wide_saved = *(uint32_t *)wide_g;
        stubs_for_now({{0xbcdfc0, (void *)stub_alloc}, {0xbcf7d0, (void *)stub_text_from_wide},
                       {0xbd0490, (void *)stub_wide_empty}});
    };
    auto wide_finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)wide_g = wide_saved;
    };
    auto wide_stdcall = [](void *fn) {
        return Result{(uint32_t)(uintptr_t)((void *(__stdcall *)(void *))fn)(arena<void>(0x180))};
    };
    Test wide_copy_object{0x6d2f00,
                          [=](Rng &r) {
                              wide_setup(r);
                              *(void **)wide_g = arena<void>(0x300);   // the original reads the object unchecked
                          },
                          wide_stdcall};
    Test wide_copy_global{0x4ce710, wide_setup, wide_stdcall};
    Test wide_copy_this{0x6dcdf0, wide_setup, [](void *fn) {
                            return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x180))};
                        }};
    for (Test *x : {&wide_copy_object, &wide_copy_global, &wide_copy_this}) {
        x->prepare = wide_prepare;
        x->finish = wide_finish;
    }
    // start-up scenery behaviours (memory for the name always there: the original crashes without)
    Test start_behaviour{0xeaaad0, [=](Rng &r) { fresh(r); text_counts(r); *arena<uint32_t>(0x28) = 1; },
                         [](void *fn) {
                             ((void(__cdecl *)())fn)();
                             return Result{};
                         }};
    start_behaviour.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)stub_alloc}, {0xbcf850, (void *)stub_text_from_c},
                       {0x833f90, (void *)stub_register_behaviour}});
    };
    start_behaviour.finish = [](const rewrite::Entry *) { unstub_many(); };
    // factories: memory sometimes runs out; the copy's own constructor is stood in for
    Test factory2{0x430b70, [=](Rng &r) { fresh(r); for (int i = 0; i < 8; i++) arena<uint32_t>(0x20)[i] = r.next(); },
                  [](void *fn) {
                      return Result{(uint32_t)(uintptr_t)((void *(__stdcall *)(uint32_t, uint32_t))fn)(
                          arena<uint32_t>(0x20)[2], arena<uint32_t>(0x20)[3])};
                  }};
    factory2.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{0xbcdfc0, (void *)stub_alloc}, {rewrite::start_constants(e->at)[2], (void *)stub_ctor3}});
    };
    factory2.finish = [](const rewrite::Entry *) { unstub_many(); };
    // from two fields of the object at 0x100 (noise)
    Test factory_fields{0x633140, [=](Rng &r) { fresh(r); for (int i = 0; i < 8; i++) arena<uint32_t>(0x20)[i] = r.next(); },
                        [](void *fn) {
                            return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x100))};
                        }};
    factory_fields.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{0xbcdfc0, (void *)stub_alloc}, {rewrite::start_constants(e->at)[4], (void *)stub_ctor4}});
    };
    factory_fields.finish = [](const rewrite::Entry *) { unstub_many(); };
    // ObjectArray::SetCapacity: the array {items, count} at 0x100, its items at 0x410 (the count 0..5 at the start of
    // the header in front: 0x40C or 0x400) or none; the new count 0..6, sometimes the same
    Test object_setcap{0x420c10,
                       [=](Rng &r) {
                           fresh(r);
                           bool items = r.next() % 4;
                           int32_t count = items ? r.range(0, 5) : r.range(0, 2);
                           *arena<int32_t>(0x400) = *arena<int32_t>(0x40c) = count;
                           *arena<void *>(0x100) = items ? arena<void>(0x410) : nullptr;
                           arena<int32_t>(0x100)[1] = count;
                           arena<int32_t>(0x10)[0] = r.next() % 4 ? r.range(0, 6) : count;
                       },
                       [](void *fn) {
                           return Result{((uint32_t(__fastcall *)(void *, void *, int32_t))fn)(
                               arena<void>(0x100), nullptr, arena<int32_t>(0x10)[0])};
                       }};
    object_setcap.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{0xddcf99, (void *)stub_vector_construct}, {rewrite::start_constants(e->at)[5], (void *)stub_assign}});
    };
    object_setcap.finish = [](const rewrite::Entry *) { unstub_many(); };
    // global factories: the global (game data, put back after each copy) is empty or holds 0x1c0 (for the lazy ones);
    // memory sometimes runs out; the copy's constructor is stood in for
    static uint32_t gf_global, gf_saved;
    auto gf_setup = [=](Rng &r) {
        fresh(r);
        for (int i = 0; i < 8; i++) arena<uint32_t>(0x20)[i] = r.next();
        *arena<void *>(0x1f8) = r.next() % 3 ? nullptr : arena<void>(0x1c0);
    };
    auto gf_call = [](void *fn) {
        *(void **)gf_global = *arena<void *>(0x1f8);   // game data: set from the arena before each run
        uint32_t r = (uint32_t)(uintptr_t)((void *(__cdecl *)())fn)();
        return Result{r ^ *(uint32_t *)gf_global};
    };
    auto gf_finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)gf_global = gf_saved;
    };
    auto gf_prepare = [](uint32_t global, uint32_t ctor, void *stand_in) {
        gf_global = global;
        gf_saved = *(uint32_t *)global;
        stubs_for_now({{0xbcdfc0, (void *)stub_alloc}, {ctor, stand_in}});
    };
    Test global_factory{0x436210, gf_setup, gf_call};
    global_factory.prepare = [=](const rewrite::Entry *e) {
        const uint32_t *c = rewrite::start_constants(e->at);
        gf_prepare(c[3], c[2], (void *)stub_ctor0);
    };
    Test lazy_factory{0x44cc40, gf_setup, gf_call};
    lazy_factory.prepare = [=](const rewrite::Entry *e) {
        const uint32_t *c = rewrite::start_constants(e->at);
        gf_prepare(c[1], c[3], (void *)stub_ctor0);
    };
    Test lazy_factory_init{0x40dfe0, gf_setup, gf_call};
    lazy_factory_init.prepare = [=](const rewrite::Entry *e) {
        const uint32_t *c = rewrite::start_constants(e->at);
        gf_prepare(c[1], c[4], (void *)stub_ctor1);
    };
    for (Test *x : {&global_factory, &lazy_factory, &lazy_factory_init}) x->finish = gf_finish;
    // destructors putting back tables: the object at 0x100 (noise); the member's destructor is stood in for
    Test dtor_tables{0x426c90, [=](Rng &r) { fresh(r); }, obj_call};
    dtor_tables.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{rewrite::start_constants(e->at)[3], (void *)stub_destructor}});
    };
    dtor_tables.finish = [](const rewrite::Entry *) { unstub_many(); };
    // ObjectArray::Resize: as SetCapacity's
    Test object_resize{0x420b70, object_setcap.setup, object_setcap.call};
    object_resize.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xddcf99, (void *)stub_vector_construct}}); };
    object_resize.finish = [](const rewrite::Entry *) { unstub_many(); };
    // calls with a temporary reference: the object at 0x100, the object referred to 0x300, whose handle (0x380, from
    // the stand-in) has its list's head at 0x3c0 with one reference (0x3d0) in it
    Test with_reference{0x536ee0,
                        [=](Rng &r) {
                            fresh(r);
                            for (int i = 0; i < 8; i++) arena<uint32_t>(0x20)[i] = r.next();
                            *arena<void *>(0x3a0) = arena<void>(0x3c0);
                            *arena<void *>(0x3c0) = arena<void>(0x3d0);
                            arena<void *>(0x3d0)[1] = arena<void>(0x3c0);
                        },
                        [](void *fn) {
                            return Result{((uint32_t(__fastcall *)(void *, void *, void *))fn)(
                                arena<void>(0x100), nullptr, arena<void>(0x300))};
                        }};
    with_reference.prepare = [](const rewrite::Entry *e) {
        const uint32_t *c = rewrite::start_constants(e->at);
        stubs_for_now({{0x805700, (void *)stub_handle_of}, {c[3], (void *)stub_with_reference},
                       {c[4], (void *)stub_reference_free}});
    };
    with_reference.finish = [](const rewrite::Entry *) { unstub_many(); };
    // PointerArray::Resize: the array {count, capacity, step, data} at 0x100, its data 0x400 or none; the new
    // capacity 0..8, sometimes the same (the result isn't compared: for the same capacity the original leaves eax)
    Test pointer_resize{0xbd6750,
                        [=](Rng &r) {
                            fresh(r);
                            for (int i = 0; i < 16; i++) arena<uint32_t>(0x400)[i] = r.next();
                            bool data = r.next() % 4;
                            uint32_t *a = arena<uint32_t>(0x100);
                            a[1] = r.range(0, 8);
                            a[0] = data ? r.range(0, (int)a[1]) : 0;
                            a[3] = data ? (uint32_t)(uintptr_t)arena<void>(0x400) : 0;
                            arena<uint32_t>(0x10)[0] = r.next() % 4 ? r.range(0, 8) : a[1];
                        },
                        [](void *fn) {
                            ((uint32_t(__fastcall *)(void *, void *, uint32_t))fn)(arena<void>(0x100), nullptr,
                                                                                 arena<uint32_t>(0x10)[0]);
                            return Result{};
                        }};
    // Array::Reserve: capacity 0..5, step 0..3, n 0..20; the copy's own resize is stood in for
    Test array_reserve{0xbd7580,
                       [=](Rng &r) {
                           fresh(r);
                           uint32_t *a = arena<uint32_t>(0x100);
                           a[1] = r.range(0, 5);
                           a[2] = r.next() % 2 ? 0 : r.range(1, 3);
                           arena<uint32_t>(0x10)[0] = r.range(0, 20);
                       },
                       [](void *fn) {
                           return Result{((uint32_t(__fastcall *)(void *, void *, uint32_t))fn)(
                               arena<void>(0x100), nullptr, arena<uint32_t>(0x10)[0])};
                       }};
    array_reserve.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{rewrite::start_constants(e->at)[1], (void *)stub_resize}});
    };
    array_reserve.finish = [](const rewrite::Entry *) { unstub_many(); };
    // interface parts: the shared owner (the global MANAGER, game data: set from the arena before each run and put
    // back after each copy) at 0x300, a given owner at 0x340 or none; each active or not
    static uint32_t gui_manager, gui_saved;
    auto gui_setup = [=](Rng &r) {
        fresh(r);
        for (int i = 0; i < 8; i++) arena<uint32_t>(0x20)[i] = r.next();
        static void *vt[2] = {nullptr, (void *)stub_activate};
        *arena<void *>(0x300) = vt;
        *arena<void *>(0x340) = vt;
        arena<uint8_t>(0x300)[4] = r.next() % 2;
        arena<uint8_t>(0x340)[4] = r.next() % 2;
        *arena<void *>(0x1f8) = arena<void>(0x300);
        arena<uint32_t>(0x10)[0] = r.next() % 3 ? 0x340 : 0;
    };
    auto gui_prepare = [](const rewrite::Entry *e, int at) {
        gui_manager = rewrite::start_constants(e->at)[at];
        gui_saved = *(uint32_t *)gui_manager;
        stubs_for_now({{0xbcdfc0, (void *)stub_alloc}, {0x6d7450, (void *)stub_gui_base}});
    };
    auto gui_finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)gui_manager = gui_saved;
    };
    Test gui_init{0x6d79e0, gui_setup, [](void *fn) {
                      *(void **)gui_manager = *arena<void *>(0x1f8);
                      uint32_t owner = arena<uint32_t>(0x10)[0];
                      return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *, void *, void *, uint32_t,
                                                                                 uint32_t))fn)(
                          arena<void>(0x100), nullptr, owner ? arena<void>(owner) : nullptr, arena<uint32_t>(0x20)[2],
                          arena<uint32_t>(0x20)[3])};
                  }};
    gui_init.prepare = [=](const rewrite::Entry *e) { gui_prepare(e, 1); };
    Test gui_make{0x6d9a20, gui_setup, [](void *fn) {
                      *(void **)gui_manager = *arena<void *>(0x1f8);
                      return Result{(uint32_t)(uintptr_t)((void *(__cdecl *)())fn)()};
                  }};
    gui_make.prepare = [=](const rewrite::Entry *e) { gui_prepare(e, 2); };
    Test gui_create2{0x6d9980, gui_setup, [](void *fn) {
                         *(void **)gui_manager = *arena<void *>(0x1f8);
                         return Result{(uint32_t)(uintptr_t)((void *(__stdcall *)(uint32_t, uint32_t))fn)(
                             arena<uint32_t>(0x20)[2], arena<uint32_t>(0x20)[3])};
                     }};
    gui_create2.prepare = [=](const rewrite::Entry *e) { gui_prepare(e, 2); };
    for (Test *x : {&gui_init, &gui_make, &gui_create2}) x->finish = gui_finish;
    // two owned blocks: the object at 0x100, each field a block or nothing (the field is up to 0xFC on)
    // objective condition names (memory for the name always there: the original crashes without; each text made
    // in memory of its own)
    Test condition_name{0x6dc430,
                        [=](Rng &r) {
                            fresh(r);
                            text_counts(r);
                            for (int i = 0; i < 8; i++) arena<uint32_t>(0x20)[i] = r.next();
                            *arena<int32_t>(0x30) = r.range(1, 2);
                        },
                        [](void *fn) {
                            return Result{(uint32_t)(uintptr_t)((void *(__stdcall *)(void *))fn)(arena<void>(0x180))};
                        }};
    condition_name.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)stub_alloc_next}, {0xbcf850, (void *)stub_text_from_c},
                       {0x403ce0, (void *)stub_text_join}, {0xbd01c0, (void *)stub_text_copy},
                       {0x410c90, (void *)stub_named_ref_text}, {0xbcf7d0, (void *)stub_text_from_wide},
                       {0xbd0490, (void *)stub_wide_empty}, {0xbd1880, (void *)stub_text_ref_free}});
    };
    condition_name.finish = [](const rewrite::Entry *) { unstub_many(); };
    // more factories: the copy's constructor stood in for (three arguments); memory sometimes runs out
    auto ctor3_prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{0xbcdfc0, (void *)stub_alloc}, {rewrite::start_constants(e->at)[2], (void *)stub_ctor3}});
    };
    auto any_setup = [=](Rng &r) {
        fresh(r);
        for (int i = 0; i < 8; i++) arena<uint32_t>(0x20)[i] = r.next();
    };
    Test factory_k3{0x4aee60, any_setup, [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__cdecl *)())fn)()}; }};
    factory_k3.prepare = ctor3_prepare;
    factory_k3.finish = [](const rewrite::Entry *) { unstub_many(); };
    Test field_factory{0x6d4300, any_setup, [](void *fn) {
                           return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x100))};
                       }};
    field_factory.prepare = ctor3_prepare;
    field_factory.finish = [](const rewrite::Entry *) { unstub_many(); };
    // a global factory with one argument: as the global factories above
    Test global_factory_k1{0x4137a0, gf_setup, gf_call};
    global_factory_k1.prepare = [=](const rewrite::Entry *e) {
        const uint32_t *c = rewrite::start_constants(e->at);
        gf_prepare(c[4], c[2], (void *)stub_ctor1);
    };
    global_factory_k1.finish = gf_finish;
    // interfaces of what a reference refers to: the object at 0x4000 (its field a reference to the handle at 0x3000),
    // the handle's object 0x3100 or none, with the class and interfaces of the QueryInterface test
    static const uint32_t *ref_c;
    Test ref_interface{0x4d1800,
                       [=](Rng &r) {
                           fresh(r);
                           const uint32_t *c = ref_c;
                           for (int i = 0; i < 0x1400 / 4; i++) arena<uint32_t>(0x3000)[i] = r.next();
                           static void *vt[2] = {nullptr, (void *)stub_class_of};
                           *(void **)arena<uint8_t>(0x4000 + c[1]) = arena<void>(0x3000);
                           *(void **)arena<uint8_t>(0x300c) = r.next() % 5 ? arena<void>(0x3100) : nullptr;
                           arena<void *>(0x3100)[0] = vt;
                           uint8_t *cls = arena<uint8_t>(0x200);
                           *arena<void *>(0x1f0) = r.next() % 5 ? cls : nullptr;
                           *(void **)(cls + 0x14) = arena<void>(0x240);
                           *(int32_t *)(cls + 0x18) = r.range(0, 8);
                           for (int i = 0; i < 8; i++) {
                               arena<void *>(0x240)[i] = r.next() % 4 ? arena<void>(0x300 + 0x20 * i) : nullptr;
                               *(int32_t *)arena<uint8_t>(0x300 + 0x20 * i + 0x10) = r.range(-64, 64) * 4;
                           }
                           *(int32_t *)c[2] = r.range(0, 9);
                       },
                       [](void *fn) {
                           return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x4000))};
                       }};
    ref_interface.prepare = [](const rewrite::Entry *e) { ref_c = rewrite::start_constants(e->at); };
    // destructors with a member: the object at 0x800 (noise), the member deleted at 0x300 or none; the base
    // destructor (and the member's) stood in for
    Test dtor_delete_member{0x490070,
                            [=](Rng &r) {
                                fresh(r);
                                static void *vt[1] = {(void *)stub_deleting};
                                arena<void *>(0x300)[0] = vt;
                                for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x800)[i] = r.next();
                                arena<uint32_t>(0x10)[0] = r.next() % 4 ? 0x300 : 0;
                            },
                            [](void *fn) {
                                uint32_t member = arena<uint32_t>(0x10)[0];
                                *(void **)(arena<uint8_t>(0x800) + g_member_field) = member ? arena<void>(member) : nullptr;
                                ((void(__fastcall *)(void *))fn)(arena<void>(0x800));
                                return Result{};
                            }};
    dtor_delete_member.prepare = [](const rewrite::Entry *e) {
        const uint32_t *c = rewrite::start_constants(e->at);
        g_member_field = c[3];
        stubs_for_now({{c[4], (void *)stub_destructor}});
    };
    dtor_delete_member.finish = [](const rewrite::Entry *) { unstub_many(); };
    Test dtor_member_base{0x42e3b0, [=](Rng &r) { fresh(r); }, obj_call};
    dtor_member_base.prepare = [](const rewrite::Entry *e) {
        const uint32_t *c = rewrite::start_constants(e->at);
        stubs_for_now({{c[2], (void *)stub_member_destructor}, {c[3], (void *)stub_destructor}});
    };
    dtor_member_base.finish = [](const rewrite::Entry *) { unstub_many(); };
    // factories with constants: the copy's constructor stood in for (2 or 5 arguments, or 4 around the two given)
    auto ctor_prepare = [](void *stand_in) {
        return [=](const rewrite::Entry *e) {
            stubs_for_now({{0xbcdfc0, (void *)stub_alloc}, {rewrite::start_constants(e->at)[2], stand_in}});
        };
    };
    auto cdecl_call = [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__cdecl *)())fn)()}; };
    Test factory_k2{0x5f80d0, any_setup, cdecl_call};
    factory_k2.prepare = ctor_prepare((void *)stub_ctor2);
    Test factory_k5{0x6d02c0, any_setup, cdecl_call};
    factory_k5.prepare = ctor_prepare((void *)stub_ctor5);
    Test factory_2k{0x464620, any_setup, [](void *fn) {
                        return Result{(uint32_t)(uintptr_t)((void *(__stdcall *)(uint32_t, uint32_t))fn)(
                            arena<uint32_t>(0x20)[2], arena<uint32_t>(0x20)[3])};
                    }};
    factory_2k.prepare = ctor_prepare((void *)stub_ctor4);
    for (Test *x : {&factory_k2, &factory_k5, &factory_2k}) x->finish = [](const rewrite::Entry *) { unstub_many(); };
    // constructors: the base constructor stood in for; the object at 0x100
    Test ctor_base_table{0x471ef0, [=](Rng &r) { fresh(r); }, [](void *fn) {
                             return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x100))};
                         }};
    ctor_base_table.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{rewrite::start_constants(e->at)[1], (void *)stub_base_ctor}});
    };
    ctor_base_table.finish = [](const rewrite::Entry *) { unstub_many(); };
    // two vectors and a value: the object at 0x100, the vectors anywhere in 0xE0..0x140 (overlapping it, too)
    Test set_two_vectors{0x4884b0,
                         [=](Rng &r) {
                             fresh(r);
                             for (int i = 0; i < 8; i++) arena<uint32_t>(0xe0)[i] = r.next();
                             arena<uint32_t>(0x10)[0] = 0xe0 + 4 * r.range(0, 24);
                             arena<uint32_t>(0x10)[1] = 0xe0 + 4 * r.range(0, 24);
                             arena<uint32_t>(0x10)[2] = r.next();
                         },
                         [](void *fn) {
                             return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *, void *, void *, void *,
                                                                                        uint32_t))fn)(
                                 arena<void>(0x100), nullptr, arena<void>(arena<uint32_t>(0x10)[0]),
                                 arena<void>(arena<uint32_t>(0x10)[1]), arena<uint32_t>(0x10)[2])};
                         }};
    // a lazy factory with an argument, and one given its arguments in registers
    Test lazy_factory_k1{0x425920, gf_setup, gf_call};
    lazy_factory_k1.prepare = [=](const rewrite::Entry *e) {
        const uint32_t *c = rewrite::start_constants(e->at);
        gf_prepare(c[1], c[4], (void *)stub_ctor1);
    };
    lazy_factory_k1.finish = gf_finish;
    Test factory_regs{0x7fd540, any_setup, [](void *fn) {
                          return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(uint32_t, uint32_t))fn)(
                              arena<uint32_t>(0x20)[2], arena<uint32_t>(0x20)[3])};
                      }};
    factory_regs.prepare = ctor_prepare((void *)stub_ctor2);
    factory_regs.finish = [](const rewrite::Entry *) { unstub_many(); };
    // render states: the object at 0x100 (the device at +0x24C is 0x300 with SetRenderState at slot 57), the value
    // the same as the kept one or not
    Test render_state{0xc6f000,
                      [=](Rng &r) {
                          fresh(r);
                          static void *vt[64];
                          vt[0xe4 / 4] = (void *)stub_set_render_state;
                          *arena<void *>(0x300) = vt;
                          for (int i = 0; i < 0x420 / 4; i++) arena<uint32_t>(0x800)[i] = r.next() % 4;
                          *(void **)arena<uint8_t>(0x800 + 0x24c) = arena<void>(0x300);
                          arena<uint32_t>(0x10)[0] = r.next() % 4;
                      },
                      [](void *fn) {
                          return Result{((uint32_t(__fastcall *)(void *, void *, uint32_t))fn)(
                              arena<void>(0x800), nullptr, arena<uint32_t>(0x10)[0])};
                      }};
    // free lists: the copy's globals (game data, put back after each copy) set from the arena before each run; the
    // list's items in the arena; the block maker stood in for (it may leave the list empty)
    static uint32_t fl_base, fl_saved[8];
    Test free_list_take{0xbd2110,
                        [=](Rng &r) {
                            fresh(r);
                            for (int i = 0; i < 8; i++) arena<uint32_t>(0x20)[i] = r.next() % 6;
                            arena<void *>(0x20)[4] = r.next() % 3 ? arena<void>(0x340) : nullptr;
                            *arena<void *>(0x340) = r.next() % 2 ? arena<void>(0x350) : nullptr;
                        },
                        [](void *fn) {
                            memcpy((void *)fl_base, arena<void>(0x20), 0x20);
                            uint32_t r = (uint32_t)(uintptr_t)((void *(__cdecl *)())fn)();
                            memcpy(arena<void>(0x60), (void *)fl_base, 0x20);
                            return Result{r};
                        }};
    free_list_take.prepare = [](const rewrite::Entry *e) {
        const uint32_t *c = rewrite::start_constants(e->at);
        fl_base = c[1];
        memcpy(fl_saved, (void *)fl_base, 0x20);
        stubs_for_now({{c[2], (void *)stub_free_list_make}});
    };
    free_list_take.finish = [](const rewrite::Entry *) {
        unstub_many();
        memcpy((void *)fl_base, fl_saved, 0x20);
    };
    // listeners: the object at 0x100, its node (at the copy's field) linked between 0x300 and 0x310 (always
    // properly: a broken list stops at a breakpoint in both)
    Test listener_dtor{0x440ed0,
                       [=](Rng &r) {
                           fresh(r);
                           uint8_t *node = arena<uint8_t>(0x100 + g_node);
                           *(void **)node = arena<void>(0x300);
                           *(void **)(node + 4) = arena<void>(0x310);
                           arena<void *>(0x300)[1] = node;
                           arena<void *>(0x310)[0] = node;
                       },
                       [](void *fn) {
                           return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x100))};
                       }};
    // destructors with a member (none for a null object): the object at 0x100 or null
    Test dtor_member_if{0x4d3b50, [=](Rng &r) { fresh(r); arena<uint32_t>(0x10)[0] = r.next() % 5 ? 0x100 : 0; },
                        [](void *fn) {
                            uint32_t o = arena<uint32_t>(0x10)[0];
                            ((void(__fastcall *)(void *))fn)(o ? arena<void>(o) : nullptr);
                            return Result{};
                        }};
    dtor_member_if.prepare = [](const rewrite::Entry *e) {
        const uint32_t *c = rewrite::start_constants(e->at);
        stubs_for_now({{c[2], (void *)stub_member_destructor}, {c[3], (void *)stub_destructor}});
    };
    dtor_member_if.finish = [](const rewrite::Entry *) { unstub_many(); };
    // lazy singletons: the global (game data) set before each run, empty or holding 0x380 (active or not); the
    // constructor stood in for keeps the new object (at 0x200) in the global
    static uint32_t sg_global, sg_saved;
    Test lazy_singleton{0x436100,
                        [=](Rng &r) {
                            fresh(r);
                            *arena<uint32_t>(0x28) = 1;   // memory always there: without, the original reads a null global
                            *arena<void *>(0x1f8) = r.next() % 3 ? nullptr : arena<void>(0x380);
                            arena<uint8_t>(0x380)[0x2c] = r.next() % 2;
                            arena<uint8_t>(0x200)[0x2c] = r.next() % 2;
                        },
                        [](void *fn) {
                            *(void **)sg_global = *arena<void *>(0x1f8);
                            uint32_t r = (uint32_t)(uintptr_t)((void *(__cdecl *)())fn)();
                            return Result{r ^ *(uint32_t *)sg_global};
                        }};
    lazy_singleton.prepare = [](const rewrite::Entry *e) {
        const uint32_t *c = rewrite::start_constants(e->at);
        sg_global = g_singleton_global = c[1];
        sg_saved = *(uint32_t *)sg_global;
        stubs_for_now({{0xbcdfc0, (void *)stub_alloc_some}, {c[6], (void *)stub_singleton_ctor},
                       {0x800860, (void *)stub_activate}});
    };
    lazy_singleton.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)sg_global = sg_saved;
    };
    listener_dtor.prepare = [](const rewrite::Entry *e) { g_node = rewrite::start_constants(e->at)[2]; };
    // listeners whose broken list is left as it is: the object at 0x100, its node (at the copy's field) linked
    // between 0x300 and 0x310, each link sometimes missing or not pointing back
    Test listener_quiet{0x413680,
                        [=](Rng &r) {
                            fresh(r);
                            uint8_t *node = arena<uint8_t>(0x100 + g_node);
                            *(void **)node = r.next() % 6 ? arena<void>(0x300) : nullptr;
                            *(void **)(node + 4) = r.next() % 6 ? arena<void>(0x310) : nullptr;
                            arena<void *>(0x300)[1] = r.next() % 6 ? (void *)node : arena<void>(0x380);
                            arena<void *>(0x310)[0] = r.next() % 6 ? (void *)node : arena<void>(0x380);
                        },
                        [](void *fn) {
                            return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x100))};
                        }};
    listener_quiet.prepare = [](const rewrite::Entry *e) { g_node = rewrite::start_constants(e->at)[2]; };
    // destructors deleting two members (each at 0x300 or 0x340, or none) after putting back tables; or putting
    // back tables and destroying a member in place; the base (and the member's) destructor stood in for
    Test dtor_delete2{0x6157f0,
                      [=](Rng &r) {
                          fresh(r);
                          static void *vt[1] = {(void *)stub_deleting};
                          arena<void *>(0x300)[0] = vt;
                          arena<void *>(0x340)[0] = vt;
                          for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x800)[i] = r.next();
                          arena<uint32_t>(0x10)[0] = r.next() % 4 ? 0x300 : 0;
                          arena<uint32_t>(0x10)[1] = r.next() % 4 ? 0x340 : 0;
                      },
                      [](void *fn) {
                          for (int k = 0; k < 2; k++) {
                              uint32_t m = arena<uint32_t>(0x10)[k];
                              *(void **)(arena<uint8_t>(0x800) + g_members[k]) = m ? arena<void>(m) : nullptr;
                          }
                          ((void(__fastcall *)(void *))fn)(arena<void>(0x800));
                          return Result{};
                      }};
    dtor_delete2.prepare = [](const rewrite::Entry *e) {
        const uint32_t *c = rewrite::start_constants(e->at);
        g_members[0] = c[1];
        g_members[1] = c[2];
        stubs_for_now({{c[3], (void *)stub_destructor}});
    };
    dtor_delete2.finish = [](const rewrite::Entry *) { unstub_many(); };
    Test dtor5_delete2{0x65d0d0, dtor_delete2.setup, dtor_delete2.call};
    dtor5_delete2.prepare = dtor_delete2.prepare;
    dtor5_delete2.finish = dtor_delete2.finish;
    Test dtor_tables_member{0x41b5b0, [=](Rng &r) { fresh(r); for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x800)[i] = r.next(); },
                            [](void *fn) {
                                ((void(__fastcall *)(void *))fn)(arena<void>(0x800));
                                return Result{};
                            }};
    dtor_tables_member.prepare = [](const rewrite::Entry *e) {
        const uint32_t *c = rewrite::start_constants(e->at);
        stubs_for_now({{c[2], (void *)stub_member_destructor}, {c[3], (void *)stub_destructor}});
    };
    dtor_tables_member.finish = [](const rewrite::Entry *) { unstub_many(); };
    // quadtree nodes: the node at 0x100 {first free, used, capacity 0..8, items at 0x4000 or none}, the items' in-use
    // flags random; the corners (any bits: NaNs too) at 0x300 and 0x320, the item at 0x340
    Test quadtree_add{0x430520,
                      [=](Rng &r) {
                          fresh(r);
                          for (int i = 0; i < 0x100 / 4; i++) arena<uint32_t>(0x300)[i] = r.next();
                          for (int i = 0; i < 64; i++) arena<uint32_t>(0x4000)[i] = r.next();
                          int32_t *n = arena<int32_t>(0x100);
                          int32_t capacity = r.next() % 4 ? r.range(1, 8) : 0;
                          n[5] = capacity;
                          n[6] = capacity ? (int32_t)(uintptr_t)arena<void>(0x4000) : 0;
                          n[3] = r.range(0, capacity);
                          n[4] = r.range(n[3], capacity);
                          for (int k = 0; k < 8; k++) arena<uint32_t>(0x4000)[k * 8 + 7] = r.next() % 2;
                      },
                      [](void *fn) {
                          return Result{(uint32_t)((int32_t(__fastcall *)(void *, void *, void *, void *, void *))fn)(
                              arena<void>(0x100), nullptr, arena<void>(0x340), arena<void>(0x300), arena<void>(0x320))};
                      }};
    quadtree_add.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x402b90, (void *)stub_vector_construct4}}); };
    quadtree_add.finish = [](const rewrite::Entry *) { unstub_many(); };
    // quadtree inserts: the tree at 0x100 (its nodes at 0x4000), the item 0x340, the corners at 0x300 and 0x320 (any
    // floats); FindNode, the node's Add and MarkChanged stood in for
    Test quadtree_insert{0x4311c0,
                         [=](Rng &r) {
                             fresh(r);
                             for (int i = 0; i < 16; i++) arena<float>(0x300)[i] = r.any_float();
                             arena<void *>(0x100)[2] = arena<void>(0x4000);
                             *arena<int32_t>(0x20) = r.range(0, 20);
                         },
                         [](void *fn) {
                             return Result{((uint32_t(__fastcall *)(void *, void *, void *, void *, void *))fn)(
                                 arena<void>(0x100), nullptr, arena<void>(0x340), arena<void>(0x300), arena<void>(0x320))};
                         }};
    quadtree_insert.prepare = [](const rewrite::Entry *e) {
        const uint32_t *c = rewrite::start_constants(e->at);
        stubs_for_now({{0xcdb600, (void *)stub_find_node}, {c[1], (void *)stub_node_add}, {c[2], (void *)stub_mark_changed}});
    };
    quadtree_insert.finish = [](const rewrite::Entry *) { unstub_many(); };
    // a forwarded interface call: self at 0x4000; the reference at REF -> handle 0x3000 (its object at +0xC is 0x3100
    // or none); the object's class/interfaces as the QueryInterface test, but the id's entry offset is 0 so the
    // interface is the object, whose +0x40 (0x3140) holds a part with a stub in every slot
    // (each records its slot). Flag/cache sometimes
    // preset.
    static const uint32_t *fwd_c;
    Test interface_forward{0x4ab940,
                           [=](Rng &r) {
                               fresh(r);
                               const uint32_t *c = fwd_c;
                               static void *obj_vt[2] = {nullptr, (void *)stub_class_of};
                               static void *part_vt[8] = {(void *)stub_forward<0>, (void *)stub_forward<1>,
                                                          (void *)stub_forward<2>, (void *)stub_forward<3>,
                                                          (void *)stub_forward<4>, (void *)stub_forward<5>,
                                                          (void *)stub_forward<6>, (void *)stub_forward<7>};
                               uint8_t *self = arena<uint8_t>(0x4000);
                               *(void **)(self + c[1]) = arena<void>(0x3000);       // REF
                               *(void **)arena<uint8_t>(0x300c) = r.next() % 5 ? arena<void>(0x3100) : nullptr;
                               arena<void *>(0x3100)[0] = obj_vt;
                               *arena<void *>(0x3140) = part_vt;
                               uint8_t *cls = arena<uint8_t>(0x200);
                               *arena<void *>(0x1f0) = r.next() % 5 ? cls : nullptr;
                               *(void **)(cls + 0x14) = arena<void>(0x240);
                               *(int32_t *)(cls + 0x18) = r.range(1, 8);
                               for (int i = 0; i < 8; i++) {
                                   arena<void *>(0x240)[i] = r.next() % 4 ? arena<void>(0x3200 + 0x20 * i) : nullptr;
                                   *(int32_t *)arena<uint8_t>(0x3200 + 0x20 * i + 0x10) = 0;   // offset 0: interface = object
                               }
                               *(int32_t *)c[4] = r.range(0, 9);                    // ID
                               self[c[2]] = r.next() % 3 == 0;                      // FLAG preset
                               *(void **)(self + c[3]) = r.next() % 2 ? arena<void>(0x3100) : nullptr;   // CACHE
                               arena<uint32_t>(0x10)[0] = r.next();
                               arena<uint32_t>(0x10)[1] = r.next();
                           },
                           [](void *fn) {
                               return Result{((uint32_t(__fastcall *)(void *, void *, uint32_t, uint32_t))fn)(
                                   arena<void>(0x4000), nullptr, arena<uint32_t>(0x10)[0], arena<uint32_t>(0x10)[1])};
                           }};
    interface_forward.prepare = [](const rewrite::Entry *e) { fwd_c = rewrite::start_constants(e->at); };
    // ObjectArray::Construct: the object at 0x100, n 0..6; the vector constructor stood in for (it gives the items at
    // 0x3000 via stub_vector_construct? no - the real one is stubbed to only record). Memory from the arena.
    Test object_construct{0x7404f0,
                          [=](Rng &r) {
                              fresh(r);
                              *arena<uint32_t>(0x28) = r.next();   // alloc sometimes fails
                              arena<uint32_t>(0x10)[0] = (uint32_t)r.range(0, 6);
                          },
                          [](void *fn) {
                              ((void *(__fastcall *)(void *, void *, int32_t))fn)(arena<void>(0x100), nullptr,
                                                                                (int32_t)arena<uint32_t>(0x10)[0]);
                              return Result{(arena<uint32_t>(0x100)[0] << 1) ^ arena<uint32_t>(0x100)[1]};
                          }};
    object_construct.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfd0, (void *)stub_alloc}, {0xddcf99, (void *)stub_vector_construct}});
    };
    object_construct.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Behaviour::Create: the owner at 0x340; memory always there (the original crashes without); the copy's
    // constructor and link stood in for, the shared activate too; the object's active flag random
    Test behaviour_create{0x46dfa0,
                          [=](Rng &r) {
                              fresh(r);
                              *arena<uint32_t>(0x28) = 1;
                              arena<uint8_t>(0x200)[0x2c] = (uint8_t)r.next();
                          },
                          [](void *fn) {
                              return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x340))};
                          }};
    behaviour_create.prepare = [](const rewrite::Entry *e) {
        const uint32_t *c = rewrite::start_constants(e->at);
        stubs_for_now({{0xbcdfc0, (void *)stub_alloc}, {c[2], (void *)stub_behaviour_ctor}, {c[3], (void *)stub_link},
                       {0x800860, (void *)stub_activate}});
    };
    behaviour_create.finish = [](const rewrite::Entry *) { unstub_many(); };
    // ObjectPair::Release: the object at 0x100; +4 -> obj at 0x300, +0 -> obj at 0x340; each a count (1 or 2) at +0
    // and a block pointer at +0xC; the frees stood in for
    Test pair_release{0x4a35b0,
                      [=](Rng &r) {
                          fresh(r);
                          arena<void *>(0x100)[1] = arena<void>(0x300);
                          arena<void *>(0x100)[0] = arena<void>(0x340);
                          arena<int32_t>(0x300)[0] = r.range(1, 2);
                          arena<uint32_t>(0x300)[3] = r.next();
                          arena<int32_t>(0x340)[0] = r.range(1, 2);
                          arena<uint32_t>(0x340)[3] = r.next();
                      },
                      [](void *fn) {
                          ((void(__fastcall *)(void *))fn)(arena<void>(0x100));
                          return Result{};
                      }};
    // SortedMap::SortedMap: the map at 0x100, n 0..6 and cap -1..6; the two allocations stood in for
    Test sorted_map_ctor{0x7fbd70,
                         [=](Rng &r) {
                             fresh(r);
                             arena<uint32_t>(0x10)[0] = (uint32_t)r.range(0, 6);
                             arena<uint32_t>(0x10)[1] = (uint32_t)r.range(-1, 6);
                         },
                         [](void *fn) {
                             ((void *(__fastcall *)(void *, void *, int32_t, int32_t))fn)(
                                 arena<void>(0x100), nullptr, (int32_t)arena<uint32_t>(0x10)[0],
                                 (int32_t)arena<uint32_t>(0x10)[1]);
                             uint32_t h = 0;
                             for (int i = 0; i < 6; i++) h = h * 31 + arena<uint32_t>(0x100)[i];
                             return Result{h};
                         }};
    sorted_map_ctor.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbcdfd0, (void *)stub_alloc_next}}); };
    sorted_map_ctor.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Frustum::FromCamera: the frustum at 0x100, the camera at 0x300 (any floats, NaNs included), flags random
    Test frustum{0x42fa40,
                 [=](Rng &r) {
                     fresh(r);
                     for (int i = 0; i < 64; i++) arena<float>(0x300)[i] = r.any_float();
                     arena<uint32_t>(0x10)[0] = r.next();
                 },
                 [](void *fn) {
                     ((void *(__fastcall *)(void *, void *, void *, uint32_t))fn)(arena<void>(0x100), nullptr,
                                                                                arena<void>(0x300), arena<uint32_t>(0x10)[0]);
                     uint32_t h = 0;
                     for (int i = 0; i < 0x64 / 4; i++) h = h * 31 + arena<uint32_t>(0x100)[i];
                     return Result{h};
                 }};
    // Interface::Float: self at 0x4000; the reference at REF -> handle 0x3000 (object at +0xC = 0x3100); the class and
    // interfaces of the QueryInterface test with the id's entry offset 0, so the interface is the object; the inner
    // object at interface+INNER = 0x3200 holds two floats (any bits) at +F1 and +F2; the COND field is random.
    static const uint32_t *iflt_c;
    Test interface_float{0x52c870,
                         [=](Rng &r) {
                             fresh(r);
                             const uint32_t *c = iflt_c;
                             static void *vt[2] = {nullptr, (void *)stub_class_of};
                             uint8_t *self = arena<uint8_t>(0x4000);
                             *(void **)(self + c[1]) = arena<void>(0x3000);          // REF
                             *(void **)arena<uint8_t>(0x300c) = arena<void>(0x3100); // handle's object
                             arena<void *>(0x3100)[0] = vt;
                             uint8_t *cls = arena<uint8_t>(0x200);
                             *arena<void *>(0x1f0) = cls;
                             *(void **)(cls + 0x14) = arena<void>(0x240);
                             int32_t count = r.range(1, 8);
                             *(int32_t *)(cls + 0x18) = count;
                             for (int i = 0; i < 8; i++) {
                                 arena<void *>(0x240)[i] = arena<void>(0x3400 + 0x20 * i);
                                 *(int32_t *)arena<uint8_t>(0x3400 + 0x20 * i + 0x10) = 0;   // offset 0: interface = object
                             }
                             *(int32_t *)c[4] = r.range(0, count - 1);                // ID, always in range (so the interface resolves)
                             *(void **)arena<uint8_t>(0x3100 + c[6]) = arena<void>(0x3200);   // inner object
                             for (int i = 0; i < 64; i++) arena<float>(0x3200)[i] = r.any_float();
                             self[c[2]] = r.next() % 3 == 0;                          // FLAG preset
                             *(void **)(self + c[3]) = arena<void>(0x3100);           // CACHE
                             *(uint32_t *)(self + c[5]) = r.next() % 2 ? 0 : r.next(); // COND
                         },
                         [](void *fn) {
                             Result x;
                             x.f = ((long double(__fastcall *)(void *))fn)(arena<void>(0x4000));
                             return x;
                         }};
    interface_float.prepare = [](const rewrite::Entry *e) { iflt_c = rewrite::start_constants(e->at); };
    // Array8::SetCapacity: the array {data at 0x400 or none, count/capacity at +4} at 0x100; n 0..6; the alloc always
    // succeeds (the copy runs even on failure, so both would crash); the constructor stood in for
    Test trivial_setcap{0x46f560,
                        [=](Rng &r) {
                            fresh(r);
                            for (int i = 0; i < 64; i++) arena<uint32_t>(0x400)[i] = r.next();
                            bool data = r.next() % 4;
                            *arena<void *>(0x100) = data ? arena<void>(0x400) : nullptr;
                            arena<int32_t>(0x100)[1] = data ? r.range(0, 5) : 0;
                            arena<int32_t>(0x10)[0] = r.range(0, 6);
                        },
                        [](void *fn) {
                            ((int32_t(__fastcall *)(void *, void *, int32_t))fn)(arena<void>(0x100), nullptr,
                                                                                arena<int32_t>(0x10)[0]);
                            return Result{};
                        }};
    trivial_setcap.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfd0, (void *)stub_alloc_next}, {0x402b90, (void *)stub_vector_construct4}});
    };
    trivial_setcap.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Destructor::Members: the object at 0x800; every 4-aligned slot up to +0x200 points to a distinct deletable
    // object (so whichever members a copy deletes are set up, and the stub records which, revealing the order) or is
    // null; each copy's base destructor is stood in for (by its address from the constants). Tables land in the arena.
    Test member_dtor{0x45c3b0,
                     [=](Rng &r) {
                         fresh(r);
                         for (int i = 0; i < 0x300 / 4; i++) arena<uint32_t>(0x800)[i] = r.next();
                         static void *del_vt[2] = {(void *)stub_deleting, (void *)stub_plain_dtor};
                         for (int off = 0; off < 0x300; off += 4) {
                             if (r.next() % 3) {
                                 uint32_t da = 0x3000 + (off / 4) * 0x20;   // 0x20 apart so refcount words don't overlap
                                 void *d = arena<void>(da);
                                 *(void **)d = del_vt;
                                 for (int w = 1; w < 8; w++) arena<uint32_t>(da)[w] = r.next() % 4;  // small refcounts so release fires
                                 *(void **)arena<uint8_t>(0x800 + off) = d;
                             } else {
                                 *(void **)arena<uint8_t>(0x800 + off) = nullptr;
                             }
                         }
                     },
                     [](void *fn) {
                         ((void(__fastcall *)(void *))fn)(arena<void>(0x800));
                         return Result{};
                     }};
    member_dtor.prepare = [](const rewrite::Entry *e) {
        const uint32_t *c = rewrite::start_constants(e->at);   // 1..3 thiscall destructor targets, 4..6 operator-delete targets
        std::vector<std::pair<uint32_t, void *>> st;
        for (int i = 1; i <= 3; i++)
            if (c[i]) st.push_back({c[i], (void *)stub_destructor});
        for (int i = 4; i <= 6; i++)
            if (c[i]) st.push_back({c[i], (void *)stub_free});
        stubs_for_now(st);
    };
    member_dtor.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Object::DelMemberField: if(this->[OFF]) delete it (slot 0, flag 1); this->[OFF]=0. del target stubbed via vtable.
    static uint32_t g_dmf_off;
    Test delmemberfield{0x436b20,
                        [=](Rng &r) {
                            fresh(r);
                            static void *dvt[1] = {(void *)stub_deleting};
                            if (r.next() % 2) { void *d = arena<void>(0x8000); *(void **)d = dvt; *(void **)(arena<uint8_t>(0x100) + g_dmf_off) = d; }
                            else *(void **)(arena<uint8_t>(0x100) + g_dmf_off) = nullptr;
                        },
                        [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    delmemberfield.prepare = [](const rewrite::Entry *e) { g_dmf_off = rewrite::start_constants(e->at)[1]; };
    // Object::InitEdx: helper(this, GLOBAL in edx); return this. helper (0xbcde60) stubbed (fastcall, records ecx/edx).
    Test initedx{0x437620,
                 [=](Rng &r) { fresh(r); },
                 [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    initedx.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbcde60, (void *)stub_fastcall2}}); };
    initedx.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Object::DerefChain: a=this->[O0]->[O1]; if(word a->[O2]) return ***(a->[O3]). No stubs; build the pointer chain.
    static uint32_t g_dc0, g_dc1, g_dc2, g_dc3;
    Test derefchain{0x426140,
                    [=](Rng &r) {
                        fresh(r);
                        *(void **)(arena<uint8_t>(0x100) + g_dc0) = arena<void>(0x600);          // this->[O0]
                        *(void **)(arena<uint8_t>(0x600) + g_dc1) = arena<void>(0x700);          // ->[O1] = a
                        *(uint16_t *)(arena<uint8_t>(0x700) + g_dc2) = (uint16_t)(r.next() | 1); // a->[O2] (word) != 0
                        *(void **)(arena<uint8_t>(0x700) + g_dc3) = arena<void>(0x800);          // a->[O3] = c
                        *(void **)arena<uint8_t>(0x800) = arena<void>(0x880);
                        *(void **)arena<uint8_t>(0x880) = arena<void>(0x900);
                        arena<uint32_t>(0x900)[0] = r.next();                                    // ***  = result
                    },
                    [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    derefchain.prepare = [](const rewrite::Entry *e) { const uint32_t *c = rewrite::start_constants(e->at); g_dc0 = c[1]; g_dc1 = c[2]; g_dc2 = c[3]; g_dc3 = c[4]; };
    // Object::ListDrain: node=head(this); while(node){ M1(node); node=next(node); }. head/next/M1 stubbed; chain 0x300->0x320->0.
    static uint32_t g_ld_m1;
    Test listdrain{0x59c780,
                   [=](Rng &r) { fresh(r); *(void **)arena<uint8_t>(0x300) = arena<void>(0x320); *(void **)arena<uint8_t>(0x320) = nullptr; },
                   [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    listdrain.prepare = [](const rewrite::Entry *e) { g_ld_m1 = rewrite::start_constants(e->at)[1]; stubs_for_now({{0x59b030, (void *)stub_head}, {0x5d49d0, (void *)stub_next}, {g_ld_m1, (void *)stub_destructor}}); };
    listdrain.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Object::QuadCollect: recursive quadtree collect. Bounded tree (8 nodes, depth-1 recursion, appends < 0x40 so
    // no flush). this at 0x100 (nodes at 0x400); each node's children buffer at 0x500+i*0x40; collector at 0x900.
    static uint32_t g_qc_coff;
    Test quadcollect{0x7b72c0,
                     [=](Rng &r) {
                         fresh(r);
                         *(void **)arena<uint8_t>(0x108) = arena<void>(0x400);   // this->[8] = nodes
                         for (int i = 0; i < 8; i++) {
                             uint8_t *nd = arena<uint8_t>(0x400 + i * 0x1c);
                             *(uint32_t *)(nd + 0) = (i == 0) ? (r.next() & 0xf) : 0;   // f_0 (recursion only from node 0)
                             *(uint32_t *)(nd + 0xc) = r.next();                        // f_c
                             *(int32_t *)(nd + 0x10) = (int32_t)(r.next() % 3);         // f_10 (0..2 children)
                             *(void **)(nd + 0x18) = arena<void>(0x500 + i * 0x40);     // f_18 = children
                             for (int c = 0; c < 2; c++) {
                                 uint8_t *ch = arena<uint8_t>(0x500 + i * 0x40 + c * 0x20);
                                 *(uint32_t *)(ch + 0x1c) = r.next() & 1 ? r.next() : 0;  // f_1c (filter)
                                 *(uint32_t *)(ch + 0x18) = r.next();                     // f_18 (value)
                             }
                         }
                         *(void **)(arena<uint8_t>(0x800) + g_qc_coff) = arena<void>(0x900);  // arg1->[COFF] = collector
                         *(int32_t *)arena<uint8_t>(0x900 + 0x104) = 0;                       // collector count
                     },
                     [](void *fn) { ((void(__thiscall *)(void *, int32_t, void *))fn)(arena<void>(0x100), 0, arena<void>(0x800)); return Result{}; }};
    quadcollect.prepare = [](const rewrite::Entry *e) { g_qc_coff = rewrite::start_constants(e->at)[1]; };
    // Object::GridGrow: ensure records cover `level` then subdivide record[level]. reserve stubbed (noop);
    // malloc/free are the global stubs. this at 0x100 (f0 = records buffer at 0x400, f4 = 0); level 0..6.
    static int32_t g_gg_level;
    Test gridgrow{0x850240,
                  [=](Rng &r) {
                      fresh(r);
                      *(void **)arena<uint8_t>(0x100) = arena<void>(0x400);   // this->f0 = records buffer
                      *(int32_t *)arena<uint8_t>(0x104) = 0;                   // this->f4 = 0
                      g_gg_level = (int32_t)(r.next() % 7);                    // 0..6
                  },
                  [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), g_gg_level)}; }};
    gridgrow.prepare = [](const rewrite::Entry *e) { stubs_for_now({{rewrite::start_constants(e->at)[1], (void *)stub_reserve}}); };
    gridgrow.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Object::FreeTwoTable: *this = table; operator delete this->[off0]; this->[off1] (0xbce010 globally stubbed).
    static uint32_t g_ftt_o0, g_ftt_o1;
    Test freetwotable{0x88f900,
                      [=](Rng &r) { fresh(r); *(void **)(arena<uint8_t>(0x100) + g_ftt_o0) = (r.next() % 2) ? arena<void>(0x8000) : nullptr; *(void **)(arena<uint8_t>(0x100) + g_ftt_o1) = (r.next() % 2) ? arena<void>(0x8080) : nullptr; },
                      [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{arena<uint32_t>(0x100)[0]}; }};
    freetwotable.prepare = [](const rewrite::Entry *e) { const uint32_t *c = rewrite::start_constants(e->at); g_ftt_o0 = c[2]; g_ftt_o1 = c[3]; };
    // Object::CondCallSet: if((int)arg > this->[4]) method(this,arg); this->[8]=arg. method stubbed (thiscall,1).
    static uint32_t g_ccs_m, g_ccs_arg;
    Test condcallset{0x7e6b80,
                     [=](Rng &r) { fresh(r); *(int32_t *)arena<uint8_t>(0x104) = (int32_t)r.next(); g_ccs_arg = r.next(); },
                     [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), g_ccs_arg); return Result{arena<uint32_t>(0x100)[2]}; }};
    condcallset.prepare = [](const rewrite::Entry *e) { g_ccs_m = rewrite::start_constants(e->at)[1]; stubs_for_now({{g_ccs_m, (void *)stub_thiscall1}}); };
    condcallset.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Object::AllocStore: r = alloc((this&~0xff)|1); *this = r; return this. alloc stubbed (records ecx, fixed return).
    static uint32_t g_als_a;
    Test allocstore{0x4027f0,
                    [=](Rng &r) { fresh(r); arena<uint32_t>(0x100)[0] = r.next(); },
                    [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{arena<uint32_t>(0x100)[0]}; }};
    allocstore.prepare = [](const rewrite::Entry *e) { g_als_a = rewrite::start_constants(e->at)[1]; stubs_for_now({{g_als_a, (void *)stub_alloc_ecx}}); };
    allocstore.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Object::CtorBaseArg: base(this, argimm); *this = table; return this. base stubbed (thiscall, 1 arg).
    static uint32_t g_cba_b;
    Test ctorbasearg{0x9685d0,
                     [=](Rng &r) { fresh(r); arena<uint32_t>(0x100)[0] = r.next(); },
                     [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    ctorbasearg.prepare = [](const rewrite::Entry *e) { g_cba_b = rewrite::start_constants(e->at)[1]; stubs_for_now({{g_cba_b, (void *)stub_thiscall1}}); };
    ctorbasearg.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Object::DbgWrap2: method(global); helper(argimm). method stubbed (thiscall), helper stubbed (cdecl 1-arg).
    static uint32_t g_dw_m, g_dw_h;
    Test dbgwrap2{0xf042f0,
                  [=](Rng &r) { fresh(r); },
                  [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    dbgwrap2.prepare = [](const rewrite::Entry *e) { const uint32_t *c = rewrite::start_constants(e->at); g_dw_m = c[2]; g_dw_h = c[3]; stubs_for_now({{g_dw_m, (void *)stub_destructor}, {g_dw_h, (void *)stub_free}}); };
    dbgwrap2.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Object::FreeTwoFields: operator delete this->[OFF0]; operator delete this->[OFF1] (0xbce010 globally stubbed).
    static uint32_t g_ftf_o0, g_ftf_o1;
    Test freetwofields{0x426770,
                       [=](Rng &r) {
                           fresh(r);
                           *(void **)(arena<uint8_t>(0x100) + g_ftf_o0) = (r.next() % 2) ? arena<void>(0x300) : nullptr;
                           *(void **)(arena<uint8_t>(0x100) + g_ftf_o1) = (r.next() % 2) ? arena<void>(0x380) : nullptr;
                       },
                       [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    freetwofields.prepare = [](const rewrite::Entry *e) { const uint32_t *c = rewrite::start_constants(e->at); g_ftf_o0 = c[1]; g_ftf_o1 = c[2]; };
    // Object::ArrayPush: reserve(count+1); this->[0xc][count]=*arg; ++count. reserve stubbed (thiscall, 1 arg).
    static uint32_t g_ap_r;
    Test arraypush{0xbd8060,
                   [=](Rng &r) {
                       fresh(r);
                       *(uint32_t *)arena<uint8_t>(0x100) = r.next() % 6;              // count 0..5
                       *(void **)arena<uint8_t>(0x10c) = arena<void>(0x300);            // array buffer
                       for (int i = 0; i < 8; i++) arena<uint32_t>(0x300)[i] = r.next();
                       arena<uint32_t>(0x200)[0] = r.next();                            // *arg
                   },
                   [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200)); return Result{}; }};
    arraypush.prepare = [](const rewrite::Entry *e) { g_ap_r = rewrite::start_constants(e->at)[1]; stubs_for_now({{g_ap_r, (void *)stub_thiscall1}}); };
    arraypush.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Object::CondForward: if(this->[0xc]>=0) method(this->[0], this->[0xc], this->[8]). method stubbed (thiscall,2).
    static uint32_t g_cf_m;
    Test condforward{0x851c70,
                     [=](Rng &r) { fresh(r); for (int i = 0; i < 16; i++) arena<uint32_t>(0x100)[i] = r.next(); *(int32_t *)arena<uint8_t>(0x10c) = (r.next() & 1) ? (int32_t)r.next() : -(int32_t)(r.next() | 1); },
                     [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    condforward.prepare = [](const rewrite::Entry *e) { g_cf_m = rewrite::start_constants(e->at)[1]; stubs_for_now({{g_cf_m, (void *)stub_thiscall2}}); };
    condforward.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Object::GlobalMemberCall: if(this) method(*global+4, this). method stubbed (thiscall, 1 arg).
    static uint32_t g_gmc_m, g_gmc_self;
    Test globalmembercall{0xc0a840,
                          [=](Rng &r) { fresh(r); g_gmc_self = r.next() & 1; },
                          [](void *fn) { ((void(__fastcall *)(void *))fn)(g_gmc_self ? arena<void>(0x100) : nullptr); return Result{}; }};
    globalmembercall.prepare = [](const rewrite::Entry *e) { g_gmc_m = rewrite::start_constants(e->at)[2]; stubs_for_now({{g_gmc_m, (void *)stub_thiscall1}}); };
    globalmembercall.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Object::GetField: return this->[OFF]. OFF per copy.
    static uint32_t g_gf_off;
    Test getfield{0xcc2610,
                  [=](Rng &r) { fresh(r); for (int i = 0; i < 64; i++) arena<uint32_t>(0x100)[i] = r.next(); },
                  [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    getfield.prepare = [](const rewrite::Entry *e) { g_gf_off = rewrite::start_constants(e->at)[1]; };
    // Object::DbgSingleton: method(global); the checked-build esp assertion is left real (no-op). method stubbed.
    static uint32_t g_ds_m;
    Test dbgsingleton{0xcc17f0,
                      [=](Rng &r) { fresh(r); },
                      [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    dbgsingleton.prepare = [](const rewrite::Entry *e) { g_ds_m = rewrite::start_constants(e->at)[2]; stubs_for_now({{g_ds_m, (void *)stub_destructor}}); };
    dbgsingleton.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Object::CallThenTail: A(this); then tail-call B(this). Both stubbed (void thiscall).
    static uint32_t g_ctt_a, g_ctt_b;
    Test callthentail{0x530130,
                      [=](Rng &r) { fresh(r); arena<uint32_t>(0x100)[0] = r.next(); },
                      [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    callthentail.prepare = [](const rewrite::Entry *e) { const uint32_t *c = rewrite::start_constants(e->at); g_ctt_a = c[1]; g_ctt_b = c[2]; stubs_for_now({{g_ctt_a, (void *)stub_destructor}, {g_ctt_b, (void *)stub_plain_dtor}}); };
    callthentail.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Object::CountList: count a null-terminated this->[0]-linked chain. Build a chain of N nodes at 0x200,0x210,...
    Test countlist{0x5908f0,
                   [=](Rng &r) {
                       fresh(r);
                       int cnt = (int)(r.next() % 9) + 1;        // 1..9 chain nodes
                       for (int i = 0; i < cnt; i++)
                           *(void **)arena<uint8_t>(0x200 + i * 0x10) = (i + 1 < cnt) ? arena<void>(0x200 + (i + 1) * 0x10) : nullptr;
                       *(void **)arena<uint8_t>(0x100) = arena<void>(0x200);   // this->[0] = first node
                   },
                   [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    // Object::Forward2: method(this, a0, a1); return this. method stubbed (thiscall, 2 args).
    static uint32_t g_f2_m, g_f2_a0, g_f2_a1;
    Test forward2{0x66a350,
                  [=](Rng &r) { fresh(r); g_f2_a0 = r.next(); g_f2_a1 = r.next(); },
                  [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x100), g_f2_a0, g_f2_a1)}; }};
    forward2.prepare = [](const rewrite::Entry *e) { g_f2_m = rewrite::start_constants(e->at)[1]; stubs_for_now({{g_f2_m, (void *)stub_thiscall2}}); };
    forward2.finish = [](const rewrite::Entry *) { unstub_many(); };
    // DeletingDestructor (member-free variant): dtor(this); if(flags&1) free(this); return this. Both stubbed.
    static uint32_t g_d2_dtor, g_d2_free, g_d2_flag;
    Test deleting2{0xbc0e50,
                   [=](Rng &r) { fresh(r); arena<uint32_t>(0x100)[0] = r.next(); g_d2_flag = r.next(); },
                   [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *, void *, uint32_t))fn)(arena<void>(0x100), nullptr, g_d2_flag)}; }};
    deleting2.prepare = [](const rewrite::Entry *e) { const uint32_t *c = rewrite::start_constants(e->at); g_d2_dtor = c[1]; g_d2_free = c[2]; stubs_for_now({{g_d2_dtor, (void *)stub_destructor}, {g_d2_free, (void *)stub_destructor}}); };
    deleting2.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Destructor::TablesThenBase: set two table fields on this, then tail-call the (stubbed) base destructor.
    static uint32_t g_ttb_base;
    Test tablesbase{0x41d050,
                    [=](Rng &r) { fresh(r); for (int i = 0; i < 16; i++) arena<uint32_t>(0x100)[i] = r.next(); },
                    [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    tablesbase.prepare = [](const rewrite::Entry *e) { g_ttb_base = rewrite::start_constants(e->at)[1]; stubs_for_now({{g_ttb_base, (void *)stub_destructor}}); };
    tablesbase.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Object::FreeField: if this->[OFF] is non-null, operator delete it (globally stubbed), then null the field.
    static uint32_t g_ff_off;
    Test freefield{0x42ff30,
                   [=](Rng &r) {
                       fresh(r);
                       *(void **)(arena<uint8_t>(0x100) + g_ff_off) = (r.next() % 2) ? arena<void>(0x300) : nullptr;
                   },
                   [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    freefield.prepare = [](const rewrite::Entry *e) { g_ff_off = rewrite::start_constants(e->at)[1]; };
    // Object::SetFields: set some fields of this to constants, return this. No calls/params; pure memory + return.
    Test setfields{0x42d200,
                   [=](Rng &r) { fresh(r); for (int i = 0; i < 16; i++) arena<uint32_t>(0x100)[i] = r.next(); },
                   [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    // Object::GetRef: *out = this->[OFF] (a refcounted object), then ++ its refcount; returns out. OFF per copy.
    static uint32_t g_getref_off;
    Test getref{0x439740,
                [=](Rng &r) {
                    fresh(r);
                    uint8_t *self = arena<uint8_t>(0x1000);
                    *(void **)(self + g_getref_off) = arena<void>(0x200);   // this->[OFF] = p
                    arena<uint32_t>(0x200)[0] = r.next();                    // p's refcount (first dword)
                    arena<uint32_t>(0x100)[0] = r.next();                    // out slot (overwritten)
                },
                [](void *fn) {
                    void *ret = ((void *(__thiscall *)(void *, void **))fn)(arena<void>(0x1000), (void **)arena<void>(0x100));
                    return Result{(uint32_t)(uintptr_t)ret};
                }};
    getref.prepare = [](const rewrite::Entry *e) { g_getref_off = rewrite::start_constants(e->at)[1]; };
    Test free_two{0x42fd80,
                  [=](Rng &r) {
                      fresh(r);
                      for (int i = 0; i < 64; i++) arena<void *>(0x100)[i] = block(r);
                  },
                  obj_call};
    // Text::Widen (registers: ecx size, edx buffer at 0x800, edi C text at 0x400): sizes 0 (no limit) to 40, texts
    // of up to 60 characters; the result is eax, its top bit flipped if edx, esi or edi didn't come back the same
    Test text_widen{0x401050,
                    [=](Rng &r) {
                        fresh(r);
                        int len = r.range(0, 60);
                        for (int i = 0; i < 64; i++) arena<uint8_t>(0x400)[i] = i < len ? (uint8_t)r.range(1, 255) : 0;
                        for (int i = 0; i < 64; i++) arena<uint32_t>(0x800)[i] = r.next();
                        arena<uint32_t>(0x10)[0] = r.next() % 6 ? r.range(1, 40) : 0;
                    },
                    [](void *fn) {
                        uint32_t eax, ecx = arena<uint32_t>(0x10)[0], edx = (uint32_t)(uintptr_t)arena<void>(0x800),
                                      esi = 0x51515151, edi = (uint32_t)(uintptr_t)arena<void>(0x400);
                        __asm__ volatile("call *%[f]"
                                         : "=a"(eax), "+c"(ecx), "+d"(edx), "+S"(esi), "+D"(edi)
                                         : [f] "b"(fn)
                                         : "memory", "cc");
                        bool kept = edx == (uint32_t)(uintptr_t)arena<void>(0x800) && esi == 0x51515151 &&
                                    edi == (uint32_t)(uintptr_t)arena<void>(0x400);
                        return Result{eax ^ (kept ? 0 : 0x80000000u)};
                    }};
    // Text::Create: the result at 0x180; memory sometimes runs out
    Test text_create{0x45c000, [=](Rng &r) { fresh(r); text_counts(r); *arena<uint32_t>(0x28) = r.next(); },
                     [](void *fn) {
                         return Result{(uint32_t)(uintptr_t)((void *(__stdcall *)(void *))fn)(arena<void>(0x180))};
                     }};
    text_create.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)stub_alloc}, {0xbcf850, (void *)stub_text_from_c},
                       {0xbcfb10, (void *)stub_text_from_c}});
    };
    text_create.finish = [](const rewrite::Entry *) { unstub_many(); };
    Test text_create2{0x475020, text_create.setup,
                      [](void *fn) {
                          return Result{(uint32_t)(uintptr_t)((void *(__stdcall *)(void *, uint32_t))fn)(
                              arena<void>(0x180), 0)};
                      }};
    text_create2.prepare = text_create.prepare;
    text_create2.finish = text_create.finish;
    Test text_create3{0xb82f50, text_create.setup,
                      [](void *fn) {
                          return Result{(uint32_t)(uintptr_t)((void *(__stdcall *)(void *, uint32_t, uint32_t))fn)(
                              arena<void>(0x180), 0, 0)};
                      }};
    text_create3.prepare = text_create.prepare;
    text_create3.finish = text_create.finish;
    // Table20::Free: the table at 0x100 {entries at 0x400 (count -1..4) or none (count -1..0: with a count, both
    // crash)}; each entry's pointers at 0x600 + 0x40 * i, their count -1..5
    Test table20_free{0x80f710,
                      [=](Rng &r) {
                          fresh(r);
                          for (int i = 0; i < 0x200 / 4; i++) arena<uint32_t>(0x400)[i] = r.next();
                          for (int i = 0; i < 0x100 / 4; i++) arena<uint32_t>(0x600)[i] = r.next();
                          *arena<void *>(0x100) = r.next() % 5 ? arena<void>(0x400) : nullptr;
                          arena<int32_t>(0x100)[1] = *arena<void *>(0x100) ? r.range(-1, 4) : r.range(-1, 0);
                          for (int i = 0; i < 4; i++) {
                              uint8_t *e = arena<uint8_t>(0x400 + 0x14 * i);
                              *(void **)(e + 4) = arena<void>(0x600 + 0x40 * i);
                              *(int32_t *)(e + 8) = r.range(-1, 5);
                          }
                      },
                      [](void *fn) {
                          ((void(__fastcall *)(void *))fn)(arena<void>(0x100));
                          return Result{};
                      }};

    for (Test *x : {&owned, &pair, &shared, &unlink, &base, &assign, &delete_target, &load, &construct, &pool_free,
                    &deleting, &query, &module, &cached, &array_load, &cleanup, &reference_null, &reference_to, &array_free, &set_table,
                    &start_name, &start_class, &start_section, &named_text_ref, &game_text_copy,
                    &text_ref_from_text, &start_key, &text_widen, &text_create, &text_create2, &text_create3, &table20_free, &start_text, &resource_type, &query_report,
                    &wide_copy_object, &wide_copy_global, &wide_copy_this, &start_behaviour, &factory2, &factory_fields, &object_setcap,
                    &global_factory, &lazy_factory, &lazy_factory_init, &dtor_tables, &free_two,
                    &object_resize, &with_reference, &pointer_resize, &array_reserve, &gui_init, &gui_make, &gui_create2,
                    &condition_name, &factory_k3, &field_factory, &global_factory_k1, &ref_interface,
                    &dtor_delete_member, &dtor_member_base, &factory_k2, &factory_k5, &factory_2k,
                    &ctor_base_table, &set_two_vectors, &lazy_factory_k1, &factory_regs, &render_state,
                    &free_list_take, &listener_dtor, &dtor_member_if, &lazy_singleton, &listener_quiet,
                    &dtor_delete2, &dtor5_delete2, &dtor_tables_member, &quadtree_add, &quadtree_insert, &object_construct, &behaviour_create, &pair_release, &member_dtor, &getref, &setfields, &freefield, &tablesbase, &deleting2, &forward2, &countlist, &callthentail, &dbgsingleton, &getfield, &globalmembercall, &condforward, &arraypush, &delmemberfield, &freetwofields, &dbgwrap2, &ctorbasearg, &allocstore, &condcallset, &freetwotable, &gridgrow, &quadcollect, &listdrain, &derefchain, &initedx, &sorted_map_ctor, &frustum, &interface_float, &trivial_setcap,
                    &interface_forward}) {
        x->iterations = (x == &deleting || x == &query || x == &module || x == &start_name) ? 4 : 20;   // thousands of copies: fewer cases each
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
