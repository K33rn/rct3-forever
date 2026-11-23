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
#include <stdarg.h>
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
alignas(16) static uint8_t g_arena[1 << 17];
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
    if (x->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && x->ExceptionRecord->NumberParameters >= 2)
        printf("  (a %s of %p)\n", x->ExceptionRecord->ExceptionInformation[0] ? "write" : "read", (void *)x->ExceptionRecord->ExceptionInformation[1]);
    fflush(stdout);   // (the output may be a pipe, buffered: ExitProcess would lose it)
    ExitProcess(3);
}
// EQUIV_FAULTS: every access violation reported as it happens (first chance, before the game's own exception frames)
static LONG WINAPI fault_seen(EXCEPTION_POINTERS *x) {
    if (x->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
        printf("  FAULT %s, case %d, in the %s: at %p, a %s of %p\n", g_testing, g_testing_case, g_testing_which, x->ExceptionRecord->ExceptionAddress,
               x->ExceptionRecord->ExceptionInformation[0] ? "write" : "read", (void *)x->ExceptionRecord->ExceptionInformation[1]);
        printf("    (the stack's top: %08lx %08lx %08lx)\n", ((unsigned long *)x->ContextRecord->Esp)[0], ((unsigned long *)x->ContextRecord->Esp)[1],
               ((unsigned long *)x->ContextRecord->Esp)[2]);
        fflush(stdout);
    }
    return EXCEPTION_CONTINUE_SEARCH;
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
            static uint8_t g_arena_rewrite[sizeof g_arena];
            memcpy(g_arena_rewrite, g_arena, sizeof g_arena);
            bool same = a == b && (!t.data || !memcmp(g_data1.data(), (void *)DATA, DATA_SIZE)) &&
                        !memcmp(g_arena1.data(), g_arena, sizeof g_arena);
            if (t.data) memcpy((void *)DATA, g_data0.data(), DATA_SIZE);
            memcpy(g_arena, g_arena0.data(), sizeof g_arena);
            if (!same) {
                // (the C runtime's printf has no 80-bit long double, so the float results are shown as doubles)
                printf("  DIFFERENT %s at 0x%x, case %d, %s-bit FPU: original %08x / %.9g, rewrite %08x / %.9g%s\n",
                       e->name, (unsigned)e->at, it, p.name, a.i, (double)a.f, b.i, (double)b.f, a == b ? " (memory differs)" : "");
                if (getenv("EQUIV_DIFF"))   // where the arena differs: offset, original, rewrite (dwords)
                    for (uint32_t o = 0, shown = 0; o < sizeof g_arena && shown < 12; o += 4)
                        if (memcmp(g_arena1.data() + o, g_arena_rewrite + o, 4)) {
                            printf("    +0x%04x: %08x %08x\n", o, *(uint32_t *)(g_arena1.data() + o), *(uint32_t *)(g_arena_rewrite + o));
                            shown++;
                        }
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

static const size_t CALL_LOG = sizeof g_arena - 0x9000;   // {count, then (function, argument) pairs}

static void record(uint32_t fn, void *arg) {
    uint32_t *log = arena<uint32_t>(CALL_LOG);
    if (log[0] < 4000) {
        log[1 + 2 * log[0]] = fn;
        log[2 + 2 * log[0]] = (uint32_t)(uintptr_t)arg;
        log[0]++;
    }
}
static void *g_text_ref;
// a value given to a stand-in: a temporary on the stack is told by what it is (the text made last: 0x7e57; other:
// its first dword), never by where it is, which differs between the original's run and the rewrite's
static uint8_t g_titled_flags[2];   // (two game flags the titled test puts back after each call)
static bool g_built_other;   // the member stand-ins return a pointer of their own (so what a constructor returns is told from its memory)
// (an object built for the titled windows' test gets a table whose slot 0x20 gives its size: two floats from the
// arena at 0x8f00, picked by the object's address so different children differ, NaN and infinity among them)
static void *__thiscall titled_child_size(void *self, void *scratch) {   // (the size put where asked, and that place given back)
    record(0x6b1641, self);
    memcpy(scratch, arena<uint8_t>(0x8f00) + ((uintptr_t)self >> 2 & 7) * 8, 8);
    return scratch;
}
// the other slots a titled constructor calls: a stand-in per slot (noting the slot, the object and what it is given; giving
// a place in the arena), its count from titled_vslots, filled in when the test is prepared
static void *arg_value(uint32_t v);
template <int SLOT> static void *__thiscall titled_v0(void *self) { record(0x60000 + SLOT, self); return arena<void>(0x8f00); }
template <int SLOT> static void *__thiscall titled_v1(void *self, uint32_t a) { record(0x60000 + SLOT, self); record(0x60000 + SLOT, arg_value(a)); return arena<void>(0x8f00); }
template <int SLOT> static void *__thiscall titled_v2(void *self, uint32_t a, uint32_t b) {
    record(0x60000 + SLOT, self); record(0x60000 + SLOT, arg_value(a)); record(0x60000 + SLOT, arg_value(b)); return arena<void>(0x8f00);
}
template <int SLOT> static void *__thiscall titled_v3(void *self, uint32_t a, uint32_t b, uint32_t c) {
    record(0x60000 + SLOT, self); record(0x60000 + SLOT, arg_value(a)); record(0x60000 + SLOT, arg_value(b)); record(0x60000 + SLOT, arg_value(c));
    return arena<void>(0x8f00);
}
template <int SLOT> static void *__thiscall titled_v4(void *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    record(0x60000 + SLOT, self); record(0x60000 + SLOT, arg_value(a)); record(0x60000 + SLOT, arg_value(b)); record(0x60000 + SLOT, arg_value(c));
    record(0x60000 + SLOT, arg_value(d)); return arena<void>(0x8f00);
}
template <int... S> static void titled_v_fill(void *(*rows)[0x80], std::integer_sequence<int, S...>) {
    ((rows[0][S] = (void *)titled_v0<S>, rows[1][S] = (void *)titled_v1<S>, rows[2][S] = (void *)titled_v2<S>, rows[3][S] = (void *)titled_v3<S>,
      rows[4][S] = (void *)titled_v4<S>), ...);
}
static void *__thiscall stub_iface_info(void *self);
static void *titled_child_table[0x80] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, (void *)titled_child_size};
extern const uint32_t titled_vslots[];
static void titled_tables_ready() {   // each slot the constructors call: its stand-in, taking as many as they give it
    static void *rows[5][0x80];
    titled_v_fill(rows, std::make_integer_sequence<int, 0x80>{});
    for (int k = 0; titled_vslots[k] != 0xffffffffu; k += 2) titled_child_table[titled_vslots[k] / 4] = rows[titled_vslots[k + 1]][titled_vslots[k] / 4];
    titled_child_table[1] = (void *)stub_iface_info;   // (slot 1, taking nothing: an object's class info, as the casts read it)
}
static void *new_wide_block();
// 0x9A0B90's hand-written loop: its count (0x53C790) from the case, 0 to 7; the name source's texts (0x53DD80: into the
// place given, for the index) new each time
// Report::MissingResourceSymbol (and its copies) on a slot holding a resource reference's place: the reference noted by what
// it is, with what it resolved to (+8)
static void __thiscall stub_missing_symbol(void **self) {
    record(0xab, arg_value((uint32_t)(uintptr_t)*self));
    record(0xab, *(void **)((uint8_t *)*self + 8));
}
// the calls giving a text back by value (titled_out_callees): given the place for it first; a new text put there, and the
// place given back
static void *__thiscall stub_text_out1(void *self, void **out) { record(0xac, self); *out = new_wide_block(); return out; }
static void *__thiscall stub_text_out2(void *self, void **out, uint32_t a) {
    record(0xac, self); record(0xac, arg_value(a)); *out = new_wide_block(); return out;
}
static void *__thiscall stub_text_out3(void *self, void **out, uint32_t a, uint32_t b) {
    record(0xac, self); record(0xac, arg_value(a)); record(0xac, arg_value(b)); *out = new_wide_block(); return out;
}
static void *__thiscall stub_text_out4(void *self, void **out, uint32_t a, uint32_t b, uint32_t c) {
    record(0xac, self); record(0xac, arg_value(a)); record(0xac, arg_value(b)); record(0xac, arg_value(c)); *out = new_wide_block(); return out;
}
static void *__thiscall stub_text_out5(void *self, void **out, uint32_t a, uint32_t b, uint32_t c, uint32_t e) {
    record(0xac, self); record(0xac, arg_value(a)); record(0xac, arg_value(b)); record(0xac, arg_value(c)); record(0xac, arg_value(e));
    *out = new_wide_block(); return out;
}
extern const uint32_t titled_out_callees[];
// Text::Copy (0xBD01C0) making a text temporary: the text the holder given holds noted, a new text put in the temporary
static void *__thiscall stub_titled_text_copy(void **self, void **from) {
    record(0xad, from ? *from : nullptr);
    *self = new_wide_block();
    return self;
}
// NamedTextRef's constructor naming the reference by a counted text (titled_text_refs_by_text): as the one naming it by a
// literal (stub_named_text_ref), the text the holder given holds noted instead of the literal
static void *__fastcall stub_named_text_ref(uint8_t *self, void *, const char *name, void *texts, uint32_t suffix);
static void *__fastcall stub_named_text_ref_text(uint8_t *self, void *edx, void **holder, void *texts, uint32_t suffix) {
    record(0xae, holder ? *holder : nullptr);
    return stub_named_text_ref(self, edx, nullptr, texts, suffix);
}
extern const uint32_t titled_text_refs_by_text[];
static uint32_t __thiscall stub_titled_count8(void *self) { record(0xa9, self); return arena<uint32_t>(0x4c)[0] >> 3 & 7; }
static void *__thiscall stub_titled_text_out(void *self, void **out, int32_t i) {
    record(0xaa, self); record(0xaa, (void *)(uintptr_t)i);
    *out = new_wide_block();
    return out;
}
static void *__cdecl stub_plain_alloc(uint32_t size);
// 0x423DE0 (an array sized: its place at +0, count at +4) for 0x61DA30's hand-written loop: the array made, filled with
// marks, so the loop's stores show
static void __thiscall stub_titled_sized(uint32_t *self, uint32_t n) {
    record(0xac, self); record(0xac, (void *)(uintptr_t)n);
    if (n > 0x40) n = 0x40;
    uint32_t *array = (uint32_t *)stub_plain_alloc(4 * n);
    for (uint32_t q = 0; q < n; q++) array[q] = 0x5a5a0000u + q;
    self[0] = (uint32_t)(uintptr_t)array, self[1] = n;
}
extern const uint32_t titled_child_holders[];
static uint32_t g_child_holders[8], g_child_holder_count;   // (the copy being checked: places in its children that counted texts go into)
static uint32_t g_child_arrays[4][3], g_child_array_count;   // (and arrays of counted texts in them: offset, elements)
extern const uint32_t titled_child_arrays[];
// one of 256 small blocks at 0x13000, picked by an object and a member, whose dwords point at the block itself (so reads
// through it land, and a list there is a valid empty one)
static void *titled_landing(void *self, uint32_t k) {
    const uint32_t pick = ((uint32_t)(uintptr_t)self * 0x9e3779b1u + k * 0x85ebca6bu) >> 24;
    uint32_t *block = arena<uint32_t>(0x13000 + pick * 0x20), *made = arena<uint32_t>(0x12c00 + pick * 4);
    // (past the last block, 2 KB of places in it: code reading a block further than its 0x20 bytes lands there)
    uint32_t *after = arena<uint32_t>(0x15000), *last = arena<uint32_t>(0x13000 + 255 * 0x20);
    if (after[0] != (uint32_t)(uintptr_t)last || after[0x1ff] != (uint32_t)(uintptr_t)last)
        for (int q = 0; q < 0x200; q++) after[q] = (uint32_t)(uintptr_t)last;
    if (*made != 0x6c6f6f70u + pick)   // ("loop", kept below the blocks: made, so later builds leave what was written into it)
        for (int q = 0; q < 8; q++) block[q] = (uint32_t)(uintptr_t)block;
    *made = 0x6c6f6f70u + pick;
    return block;
}
static void *built_result(void *self) {
    if (!g_built_other) return self;
    if (*(void **)self == titled_child_table)
        return (uint8_t *)self + 8;   // (made already, or one the case filled: what was written into it since is kept, so it shows)
    *(void **)self = titled_child_table;
    *(void **)((uint8_t *)self + 8) = titled_child_table;
    // (its members told apart: each points at one of 256 small blocks at 0x13000, picked by the object and the member, whose
    // dwords point at the block itself, so a chain of reads through members lands, and a list there is a valid empty one)
    for (int k = 1; k < 0x28; k++) ((uint32_t *)self)[2 + k] = (uint32_t)(uintptr_t)titled_landing(self, (uint32_t)k);
    for (int k = 0; k < 5; k++) {   // (its rectangle, +0x7C..+0x8C as a window's: plain floats, each its own, so reads of it are told apart)
        const float f = (float)((uint32_t)(uintptr_t)self >> 4 & 0x3ff) * 0.25f + (float)k * 7.5f;
        memcpy((uint8_t *)self + 0x7c + 4 * k, &f, 4);
    }
    // (the places in children that counted texts are put into: a text there, as the child's own constructor leaves one)
    for (uint32_t k = 0; k < g_child_array_count; k++)   // (an array of counted texts at each, at both places)
        if (g_child_arrays[k][0] >= 0x10 && g_child_arrays[k][0] < 0x400 && g_child_arrays[k][1] <= 64)
            for (uint32_t at = 0; at <= 8; at += 8) {
                void **array = (void **)stub_plain_alloc(4 * g_child_arrays[k][1]);
                for (uint32_t q = 0; q < g_child_arrays[k][1]; q++) array[q] = new_wide_block();
                *(void ***)((uint8_t *)self + at + g_child_arrays[k][0]) = array;
            }
    for (uint32_t k = 0; k < g_child_holder_count; k++)
        if (g_child_holders[k] >= 0x10 && g_child_holders[k] < 0x400) {   // (at the memory's place and the result's, 8 further)
            *(void **)((uint8_t *)self + g_child_holders[k]) = new_wide_block();
            *(void **)((uint8_t *)self + 8 + g_child_holders[k]) = new_wide_block();
        }
    return (uint8_t *)self + 8;
}
static void __thiscall titled_layout(void *self, uint32_t relation, void *to, const uint32_t *rect) {
    record(0x6b1640, self); record(0x6b1640, (void *)(uintptr_t)relation); record(0x6b1640, to);
    for (int k = 0; k < 4; k++) record(0x6b1640, (void *)(uintptr_t)rect[k]);
}
static void *arg_value(uint32_t v) {
    uintptr_t here = (uintptr_t)&v;
    if (g_text_ref && v == (uint32_t)(uintptr_t)g_text_ref) return (void *)0x7e57;   // (none made: a 0 is a 0)
    if (v > here && v - here < 0x4000) return (void *)(uintptr_t)(0x57ac0000u ^ *(uint32_t *)(uintptr_t)v);
    return (void *)(uintptr_t)v;
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
static int32_t __thiscall stub_size_item(void *self, void *arg) { record(0x57, self); record(0x57, arg); return 0x10; }
static void *__thiscall stub_grid_h2(void *obj, int32_t level) { record(0x50, obj); record(0x50, (void *)(uintptr_t)level); return arena<void>(0x600); }
static void __thiscall stub_grid_h1(void *obj, int32_t a, void *b) { record(0x51, obj); record(0x51, (void *)(uintptr_t)a); record(0x51, b); }
static void __thiscall stub_grid_h3(void *dst, void *src) { record(0x52, dst); record(0x52, src); }
static uint32_t __fastcall stub_helper3(uint32_t a, uint32_t b, int32_t c) { record(0x47, (void *)(uintptr_t)a); return 0x1234u; }
static int32_t __fastcall stub_pos_count(void *self) { record(0x45, self); return 7; }
static int32_t __fastcall stub_sub_val(void *self) { record(0x46, self); return 5; }
static void *__fastcall stub_head(void *self) { record(0x40, self); return arena<void>(0x300); }
static void __fastcall stub_fastcall2(void *ecx, void *edx) { record(0x42, ecx); record(0x42, edx); }
static void *__fastcall stub_next(void *n) { record(0x41, n); return *(void **)n; }
static void __thiscall stub_thiscall2(void *self, uint32_t a0, uint32_t a1) { record(0x20, self); record(0x20, (void *)(uintptr_t)a0); record(0x20, (void *)(uintptr_t)a1); }
static void __thiscall stub_thiscall1(void *self, void *arg) { record(0x21, self); record(0x21, arg); }
static void *__fastcall stub_alloc_ecx(void *ecx) { record(0x22, ecx); return (void *)0x30001234; }
static void __thiscall stub_shared_free(void *pool, void *item) { record(0x60, pool); record(0x60, item); }
static void *__thiscall stub_shared_convert(void *value, uint32_t kind) {
    record(0x61, value); record(0x61, (void *)(uintptr_t)kind);
    return (kind & 1) ? arena<void>(0x5e0) : nullptr;
}
static void __thiscall stub_row_field(void *self, void *row, void *stream, uint32_t context) {
    record(0x62, self); record(0x62, row); record(0x62, stream); record(0x62, (void *)(uintptr_t)context);
}
static void __thiscall stub_rows_resize(void *store, int32_t count) { record(0x63, store); record(0x63, (void *)(uintptr_t)count); }
static int32_t __thiscall stub_rows_size(void *self, int32_t offset) { record(0x64, self); record(0x64, (void *)(uintptr_t)offset); return arena<int32_t>(0x10)[1]; }
static void __thiscall stub_row_save(void *self, void *row, void *stream) { record(0x65, self); record(0x65, row); record(0x65, stream); }
static void __thiscall stub_stream_write(void *stream, int32_t *value, int32_t size) { record(0x66, stream); record(0x66, (void *)(uintptr_t)*value); record(0x66, (void *)(uintptr_t)size); }
static void *__fastcall stub_item_table(void *self) { record(0x67, self); return arena<void>(0x4a0); }
static void *__thiscall stub_item_find(void *self, uint32_t item, uint32_t arg) { record(0x68, self); record(0x68, (void *)(uintptr_t)item); record(0x68, (void *)(uintptr_t)arg); return arena<void>(0x4b0); }
static uint8_t __fastcall stub_item_check(void *self) { record(0x69, self); return arena<int32_t>(0x20)[0]-- == 0 ? 2 : 0; }
static uint8_t __fastcall stub_mapped_lookup(uint8_t kind, uint8_t side, int32_t *value, int32_t step) {
    record(0x6a, (void *)(uintptr_t)kind); record(0x6a, (void *)(uintptr_t)side); record(0x6a, (void *)(uintptr_t)step);
    *value += step;
    return kind != 3;
}
static int __cdecl stub_same_pointer(const char *a, const char *b) { record(0x6b, (void *)a); record(0x6b, (void *)b); return a != b; }
static int32_t __fastcall stub_item_count(void *self) { record(0x6c, self); return 3; }
static uint32_t __fastcall stub_item_value(void *item) { record(0x6d, item); return (uint32_t)(uintptr_t)item ^ 0x5a5a; }
static uint8_t __fastcall stub_name_place(void *name, void *table, int32_t count, int32_t *at, int32_t *at2) {
    record(0x6e, name); record(0x6e, table); record(0x6e, (void *)(uintptr_t)count);
    (void)at2;
    *at = arena<int32_t>(0x20)[0];
    return 0;
}
static uint8_t __thiscall stub_mapped_then(void *self, uint32_t a, uint32_t b, uint32_t side, int32_t value) {
    record(0x6f, self); record(0x6f, (void *)(uintptr_t)a); record(0x6f, (void *)(uintptr_t)b);
    record(0x6f, (void *)(uintptr_t)side); record(0x6f, (void *)(uintptr_t)value);
    return (uint8_t)(a + b);
}
static void *__thiscall stub_clone(void *item, int32_t flag) {
    record(0x70, item); record(0x70, (void *)(uintptr_t)flag);
    return arena<void>(0x600 + arena<int32_t>(0x20)[0]++ * 0x60);
}
static uint8_t __fastcall stub_report(uint32_t file, uint32_t line, uint32_t a, uint32_t b) {
    record(0x71, (void *)(uintptr_t)file); record(0x71, (void *)(uintptr_t)line); record(0x71, (void *)(uintptr_t)a); record(0x71, (void *)(uintptr_t)b);
    return 0;
}
static uint32_t __thiscall stub_item_arg(void *item, uint32_t arg) { record(0x72, item); record(0x72, (void *)(uintptr_t)arg); return arg ^ 0x77; }
static void __thiscall stub_field_ctor(void *self, uint32_t a, uint32_t d, uint32_t k, uint32_t b) {
    record(0x73, self); record(0x73, (void *)(uintptr_t)a); record(0x73, (void *)(uintptr_t)d); record(0x73, (void *)(uintptr_t)k); record(0x73, (void *)(uintptr_t)b);
}
static uint8_t __thiscall stub_ask(void *self, uint32_t a, uint32_t b) { record(0x78, self); record(0x78, (void *)(uintptr_t)a); record(0x78, (void *)(uintptr_t)b); return (uint8_t)((a ^ b) & 2); }
static void __stdcall stub_destroy_each(void *items, uint32_t size, int32_t count, void *destroy) {
    record(0x79, items); record(0x79, (void *)(uintptr_t)size); record(0x79, (void *)(uintptr_t)count); record(0x79, destroy);
}
static void *__fastcall stub_lookup_a(void *self) { record(0x7a, self); return arena<void>(0x500); }
static void *__fastcall stub_lookup_b(void *self) { record(0x7b, self); return arena<void>(0x510); }
static uint8_t __fastcall stub_name_found(void *name, void *table, int32_t count, int32_t *at, int32_t *at2) {
    record(0x7c, name); record(0x7c, table); record(0x7c, (void *)(uintptr_t)count);
    (void)at2;
    *at = arena<int32_t>(0x20)[0];
    return (uint8_t)arena<int32_t>(0x20)[1] * 3;
}
static void *__thiscall stub_ride_entry(void *self, int32_t i, uint32_t a, uint32_t b) {
    record(0x7d, self); record(0x7d, (void *)(uintptr_t)i); record(0x7d, (void *)(uintptr_t)a); record(0x7d, (void *)(uintptr_t)b);
    return arena<void>(0xe00 + i * 0x10);
}
#include "wave1_stubs.inc"
#include "wave2_stubs.inc"
#include "wave3_stubs.inc"
#include "wave4_stubs.inc"
static void __thiscall stub_item_copy(uint32_t *to, uint32_t *from) { record(0x7e, to); record(0x7e, from); to[0] = from[0] ^ 0x77; }
static void __fastcall stub_destroy_b(void *self) { record(0x7f, self); }
static void __thiscall stub_quad_update(void *tree, int32_t n) { record(0x61, tree); record(0x61, (void *)(intptr_t)n); }
static void __thiscall stub_quad_take_all(void *tree, int32_t n, void *query) { record(0x62, tree); record(0x62, (void *)(intptr_t)n); record(0x62, query); }
static void __thiscall stub_quad_walk(void *tree, void *query, int32_t n, void *low, void *size) {
    record(0x63, tree); record(0x63, query); record(0x63, (void *)(intptr_t)n); record(0x63, low); record(0x63, size);
}
static void __thiscall stub_show(void *object) { record(0x64, object); }
static void __thiscall stub_show_zero(void *object, int32_t zero) { record(0x65, object); record(0x65, (void *)(intptr_t)zero); }
static void __thiscall stub_flush(void *collector, void *list, int32_t n) { record(0x66, collector); record(0x66, list); record(0x66, (void *)(intptr_t)n); }
static void __thiscall stub_list_grow(uint32_t *list, int32_t capacity) { record(0x67, list); record(0x67, (void *)(intptr_t)capacity); list[1] = capacity; }
static void *__thiscall stub_object_info(uint8_t *object) { record(0x68, object); return *(uint32_t *)(object + 0x40) & 1 ? arena<void>(0x8000) : nullptr; }
static uint32_t __thiscall stub_owner_of(uint32_t *owned) { record(0x69, owned); return *owned & 3; }
static void __thiscall stub_flush_count(void *query, int32_t count) { record(0x6a, query); record(0x6a, (void *)(intptr_t)count); }
static void __thiscall stub_draw(void *drawer, void *entry, void *normal, uint32_t extra) { record(0x6b, drawer); record(0x6b, entry); record(0x6b, normal); record(0x6b, (void *)(uintptr_t)extra); }
static void *__cdecl stub_plain_alloc(uint32_t size) {
    record(0xbcdfc0, (void *)(uintptr_t)size);
    uint32_t &next = *arena<uint32_t>(ALLOC_NEXT);
    if (next < ALLOC_START) next = ALLOC_START;
    void *p = arena<void>(next);
    next += (size + 15) & ~15u;
    if (g_built_other && size <= 0x4000) {   // (the window test: what is handed out reads as places in the landing blocks, as built
                                             // objects' members; and 16 bytes past it, as the stand-ins give their objects 8 further)
        next += 0x10;
        for (uint32_t q = 0; q < (size + 0x10) / 4; q++) ((uint32_t *)p)[q] = (uint32_t)(uintptr_t)titled_landing(p, q);
    }
    return p;
}
static void *__thiscall stub_name_text(uint32_t *self, const char *text) { record(0x6c, self); record(0x6c, (void *)text); self[0] = 1; self[3] = 0x1234; return self; }
static void *__thiscall stub_type_by_name(void *self, void **name) { record(0x6d, self); record(0x6d, *name); return arena<uint32_t>(0x10)[2] & 1 ? arena<void>(0x600) : nullptr; }
static void *__thiscall stub_item_make(void *memory, int32_t a, int32_t b, int32_t c) { record(0x6e, memory); record(0x6e, (void *)(intptr_t)(a + b + c)); return memory; }
static void __thiscall stub_item_set_type(void *item, uint32_t type, int32_t zero) { record(0x6f, item); record(0x6f, (void *)(uintptr_t)type); record(0x6f, (void *)(intptr_t)zero); }
static void *__thiscall stub_new_handle(void *item) { record(0x70, item); return arena<void>(0x700); }
static void __thiscall stub_slot_told(void *slot) { record(0x71, slot); }
static void __thiscall stub_tool_setup(void *global, int32_t zero, void **a, void **b, void **c) {
    record(0x72, global); record(0x72, (void *)(intptr_t)zero); record(0x72, *a); record(0x72, *b); record(0x72, *c);
}
// a reference made on the stack: its contents are recorded, not where it is
static void record_ref(uint32_t tag, const uint32_t *ref) {
    record(tag, (void *)(uintptr_t)ref[0]); record(tag, (void *)(uintptr_t)ref[1]); record(tag, (void *)(uintptr_t)(ref[4] == (uintptr_t)ref)); record(tag, (void *)(uintptr_t)ref[5]); record(tag, (void *)(uintptr_t)(ref[6] & 0xff));
    const uint32_t *head = (const uint32_t *)(uintptr_t)ref[3];
    record(tag, (void *)(uintptr_t)(head && head[0] == (uintptr_t)&ref[2]));
}
static void __thiscall stub_ref_add(void *member, const uint32_t *ref) { record(0x73, member); record_ref(0x73, ref); }
static void __thiscall stub_ref_release(uint32_t *ref) {   // unlinks it, as the real release does
    record_ref(0x74, ref);
    ((uint32_t *)(uintptr_t)ref[2])[1] = ref[3];
    ((uint32_t *)(uintptr_t)ref[3])[0] = ref[2];
}
static void __thiscall stub_set_up(void *self) { record(0x75, self); }
static void __thiscall stub_slot_free(void *node, uint32_t slot) { record(0x76, node); record(0x76, (void *)(uintptr_t)slot); }
// wide strings for the window constructors: blocks {references, ?, ?, data} made in the arena
static void *new_wide_block() {
    uint32_t *block = (uint32_t *)stub_plain_alloc(0x10);
    block[0] = 1;
    block[3] = 0x5678;
    return block;
}
static void *__fastcall stub_wide_empty_shared(uint32_t flag) { record(0x77, (void *)(uintptr_t)(flag & 0xff)); return arena<void>(0x8800); }
static void *__fastcall stub_wide_empty_other(uint32_t flag) { record(0x76, (void *)(uintptr_t)(flag & 0xff)); return arena<void>(0x8810); }   // 0xBD0520
static void *__thiscall stub_wide_assign_c(void *self, void **out, const char *text) { record(0x78, (void *)text); *out = new_wide_block(); return out; }
static void *__thiscall stub_wide_assign(void *self, void **out, void **text) { record(0x79, *text); *out = new_wide_block(); return out; }
static void *__thiscall stub_wide_copy(void **out, void *title) { record(0x7a, title); *out = new_wide_block(); return out; }
static void __thiscall stub_window_base(void *self, void *parent, void **text) { record(0x7b, self); record(0x7b, parent); record(0x7b, *text); }
static void __thiscall stub_member_built(void *self) { record(0x7c, self); }
static uintptr_t g_window_title;
// titled window constructors (Window::Titled): the base class's constructor, by how many values it takes; the text it
// gets is recorded by what it holds (it can be a copy on the stack)
static void titled_base_record(void *self, void *parent, void **text) { record(0x7d, self); record(0x7d, parent); record(0x7d, text ? *text : nullptr); }
static void *__thiscall stub_titled_base2(void *self, void *p, void **t) { titled_base_record(self, p, t); return self; }
static void *__thiscall stub_titled_base3(void *self, void *p, void **t, uint32_t a) { titled_base_record(self, p, t); record(0x7d, arg_value(a)); return self; }
static void *__thiscall stub_titled_base4(void *self, void *p, void **t, uint32_t a, uint32_t b) {
    titled_base_record(self, p, t); record(0x7d, arg_value(a)); record(0x7d, arg_value(b)); return self;
}
static void *__thiscall stub_titled_base5(void *self, void *p, void **t, uint32_t a, uint32_t b, uint32_t c) {
    titled_base_record(self, p, t); record(0x7d, arg_value(a)); record(0x7d, arg_value(b)); record(0x7d, arg_value(c)); return self;
}
static void *__thiscall stub_titled_base6(void *self, void *p, void **t, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    titled_base_record(self, p, t); record(0x7d, arg_value(a)); record(0x7d, arg_value(b)); record(0x7d, arg_value(c)); record(0x7d, arg_value(d));
    return self;
}
static void *__thiscall stub_built5(void *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    record(0x81, self); record(0x81, arg_value(a)); record(0x81, arg_value(b)); record(0x81, arg_value(c)); record(0x81, arg_value(d));
    record(0x81, arg_value(e)); return built_result(self);
}
extern const uint32_t class_bases[], class_value_calls[], class_ref_setters[], class_read_globals[];
static uint32_t g_class_read_saved[128];
extern const uint32_t titled_text_holders[];
extern const uint32_t titled_compared_fields[];
static uint32_t g_titled_at;   // (the copy being checked)
extern const uint32_t titled_iface_args[], titled_class_globals[], titled_vcall_args[], titled_iface_places[];
static uint32_t g_titled_vcall_mask;   // (the arguments the copy makes virtual calls on: objects with the stand-ins' table)
static uint32_t g_titled_class_saved[16];
static uint32_t g_titled_iface_mask;   // (the arguments the copy casts to an interface: objects with a class info)
static void *__thiscall stub_iface_info(void *self) {   // an object's class info (slot 1): the case's, or none
    record(0x83, self);
    return arena<uint32_t>(0x9a00)[15] ? arena<void>(0x9a00) : nullptr;
}
static void *titled_iface_table[2] = {nullptr, (void *)stub_iface_info};
extern const uint32_t titled_info[], titled_callees[], titled_nullable[], titled_object_globals[], titled_written_globals[], titled_text_globals[], value_info[], record_info[];
static uint32_t g_titled_text_saved[8], g_titled_read_saved[256];
extern const uint32_t titled_read_globals[];
static uint32_t g_titled_written_saved[16];
static uint32_t g_titled_object_saved[32];
// (the objects the globals called on hold: 32 of them in the arena, the first 16 at 0xD000, the rest at 0x11000)
static uint32_t titled_object_place(uint32_t k) { return k < 16 ? 0xd000 + k * 0x100 : 0x11000 + (k - 16) * 0x100; }
extern const uint32_t titled_singletons[];
extern const uint32_t titled_list_globals[];
extern const uint32_t titled_window_lists[];
static uint32_t g_titled_list_saved[8];
static uint32_t g_titled_single_global, g_titled_single_fn;   // (the copy's singleton made on first use: its global, its constructor)
extern const uint32_t dtor_steps_info[];
extern const uint32_t table_make_info[];
// Text::Make6A39D0: a text for a child window from a name, its window and a value: recorded by what they are; the
// text gets a wide string of its own
// a stack temporary made by a fastcall (titled_fast_temps): edx and the values noted, a counted text put in the temporary
static void *__fastcall stub_fast_temp0(void **self, uint32_t v) { record(0x89, arg_value(v)); *self = new_wide_block(); return self; }
static void *__fastcall stub_fast_temp1(void **self, uint32_t v, uint32_t a) { record(0x89, arg_value(v)); record(0x89, arg_value(a)); *self = new_wide_block(); return self; }
static void *__fastcall stub_fast_temp2(void **self, uint32_t v, uint32_t a, uint32_t b) {
    record(0x89, arg_value(v)); record(0x89, arg_value(a)); record(0x89, arg_value(b)); *self = new_wide_block(); return self;
}
static void *__fastcall stub_fast_temp3(void **self, uint32_t v, uint32_t a, uint32_t b, uint32_t c) {
    record(0x89, arg_value(v)); record(0x89, arg_value(a)); record(0x89, arg_value(b)); record(0x89, arg_value(c)); *self = new_wide_block(); return self;
}
extern const uint32_t titled_fast_temps[];
static void *__thiscall stub_text_for_child(void **self, void **name, void *window, uint32_t value) {
    record(0x86, name ? *name : nullptr); record(0x86, window); record(0x86, (void *)(uintptr_t)value);
    *self = new_wide_block();
    return self;
}
// 0x4B7940 (a text holder set from a name and a value): the name's text and the value noted, a text of its own put in
static void *__thiscall stub_text_assign2(void **self, void **name, uint32_t value) {
    record(0x87, name ? *name : nullptr); record(0x87, (void *)(uintptr_t)value);
    *self = new_wide_block();
    return self;
}
static uintptr_t g_titled_title;
static uint32_t g_titled_args, g_titled_nullable, g_titled_byte_flags, g_titled_parent = 1, g_titled_text = 2;
extern const uint32_t titled_byte_flags[];
static uint32_t g_window_args;   // 2 or 3, from the copy's ret
// saved classes' constructors: the base, members' and fields' constructors (by how many values they take), the field
// list's add, the description's name step and the object info
static void __thiscall stub_class_base4(void *self, void *description, uint32_t a, uint32_t b, uint32_t c) {   // (a base taking one more)
    record(0x80, self); record(0x80, description); record(0x80, (void *)(uintptr_t)a); record(0x80, (void *)(uintptr_t)b); record(0x80, (void *)(uintptr_t)c);
}
static uint32_t __cdecl stub_class_value() {   // (a function taking nothing whose value a member gets: a value from the case, noted)
    record(0x7a, nullptr);
    return arena<uint32_t>(0x4c)[0] * 0x9e3779b1u;
}
static uint32_t g_class_random_n;   // (the draws so far in this run)
// the copy's end counts a chain a global points to and clears a flag global when it is empty (class_chains): the test
// points the chain's global at a chain of the case's length (arena 0x9400, 16 bytes a node) and notes the flag after
extern const uint32_t class_chains[];
static uintptr_t g_class_chain, g_class_chain_flag;
// and one adding the object to a game object's array (class_owner_arrays): the global points at arena 0x9500, an array of
// the case's room and count (full or not) with items at 0x9600
extern const uint32_t class_owner_arrays[];
extern const uint32_t class_counts[];
extern const uint32_t class_instances[];   // (a hand-written end's instance global, which the scan below doesn't find)
extern const uint32_t class_text_args[];   // (the copies given counted texts through arguments: each points at a holder)
static uint32_t g_class_text_args;   // (and one raising a game count: given 0 or not by the case, noted after)
static uintptr_t g_class_count;
static bool g_class_slot_array;   // (0x44F240: the game array 0x1106D0C given the case's, and noted after)
static uint32_t g_class_slot_saved[2];
static uint32_t g_class_count_saved;
static uintptr_t g_class_owner;
static uint32_t g_class_owner_saved;
static uint32_t g_class_chain_saved[2];
// 0x6C3BE0 (a list of named entries the game fills, made on the stack): {table, entries, room, count} as the case
// decides (entries or none; room or none; the first entry's name or none)
// 0x4BF730 (an array of 12-byte entries given a size): noted; the array gets items at arena 0x9300 (the case's values in them)
// 0x47D160 (0x44F240's part): noted; the part gets an inner object (arena 0x10800), a counted text at +0x44 (0x10900)
// and its +0x18 and byte +0x11 as the case decides
static void *__thiscall stub_class_part_47d160(uint8_t *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f) {
    record(0xa7, self); record(0xa7, (void *)(uintptr_t)a); record(0xa7, (void *)(uintptr_t)b); record(0xa7, (void *)(uintptr_t)c);
    record(0xa7, arg_value(d)); record(0xa7, arg_value(e)); record(0xa7, (void *)(uintptr_t)f);
    const uint32_t q = arena<uint32_t>(0x8b68)[0];
    uint32_t *text = arena<uint32_t>(0x10900);
    text[0] = q % 3, text[1] = 0x7100, text[2] = 0x6100, text[3] = 0x5100;
    *(void **)(self + 0xc) = arena<void>(0x10800);
    *(void **)(self + 0x44) = text;
    *(uint32_t *)(self + 0x18) = q >> 4 & 1;
    self[0x11] = q >> 5 & 1;
    return self;
}
static void __thiscall stub_class_inner_ce4770(void *self, void *a) { record(0xa8, self); record(0xa8, a); }
static void __thiscall stub_class_entries_sized(uint32_t *array, uint32_t n) {
    record(0x9f, array); record(0x9f, (void *)(uintptr_t)n);
    array[0] = (uint32_t)(uintptr_t)arena<void>(0x9300);
    array[1] = n;
}
static void *__thiscall stub_class_name_list(uint32_t *list) {
    record(0x9e, nullptr);
    const uint32_t c = arena<uint32_t>(0x8b50)[0];
    list[0] = 0xf622f0;
    list[1] = c & 3 ? (uint32_t)(uintptr_t)arena<void>(0x9200) : 0;
    list[2] = c & 2 ? 1 + (c >> 8) % 9 : 0;
    list[3] = list[2];
    arena<uint32_t>(0x9200)[0] = (uint32_t)(uintptr_t)arena<void>(0x9240);
    arena<uint32_t>(0x9240)[1] = c & 4 ? (uint32_t)(uintptr_t)arena<void>(0x9280) : 0;
    return list;
}
// an object the instance global holds already: its table's slot 0 (the deleting destructor) notes what it was given
static void __thiscall stub_class_old_delete(void *self, uint32_t flag) { record(0x9d, self); record(0x9d, (void *)(uintptr_t)flag); }
static void *const g_class_old_table[2] = {(void *)stub_class_old_delete, (void *)stub_class_old_delete};
// a game object the copy makes when its global holds none (class_singletons): its constructor's stand-in notes what it was
// given and puts the object in the global, as the real one does
extern const uint32_t class_singletons[];
static uintptr_t g_class_single;          // (the copy's global, or 0)
static uint32_t g_class_single_saved;
static void *__thiscall stub_class_single(void *self, uint32_t a) {
    record(0x9c, self); record(0x9c, (void *)(uintptr_t)a);
    if (g_class_single) *(void *volatile *)g_class_single = self;
    return self;
}
static float __cdecl stub_class_random() {   // (Random::Ran0: a new value in [0, 1) for each draw, from the case, noted)
    record(0x475180, nullptr);
    uint32_t h = arena<uint32_t>(0x4c)[0] + 0x9e3779b9u * ++g_class_random_n;
    h ^= h >> 15, h *= 0x2c1b3c6du, h ^= h >> 12, h *= 0x297a2d39u, h ^= h >> 15;
    return (float)(h >> 8) * (1.0f / 16777216.0f);
}
static void __thiscall stub_class_base(void *self, void *description, uint32_t a, uint32_t b) { record(0x80, self); record(0x80, description); record(0x80, (void *)(uintptr_t)a); record(0x80, (void *)(uintptr_t)b); }
static void *__thiscall stub_built6(void *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f) {
    record(0x81, self); record(0x81, arg_value(a)); record(0x81, arg_value(b)); record(0x81, arg_value(c)); record(0x81, arg_value(d));
    record(0x81, arg_value(e)); record(0x81, arg_value(f)); return built_result(self);
}
static void *__thiscall stub_built7(void *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f, uint32_t g) {
    record(0x81, self); record(0x81, arg_value(a)); record(0x81, arg_value(b)); record(0x81, arg_value(c)); record(0x81, arg_value(d));
    record(0x81, arg_value(e)); record(0x81, arg_value(f)); record(0x81, arg_value(g)); return built_result(self);
}
static void *__thiscall stub_built8(void *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f, uint32_t g, uint32_t h) {
    record(0x81, self); record(0x81, arg_value(a)); record(0x81, arg_value(b)); record(0x81, arg_value(c)); record(0x81, arg_value(d));
    record(0x81, arg_value(e)); record(0x81, arg_value(f)); record(0x81, arg_value(g)); record(0x81, arg_value(h)); return built_result(self);
}
static void *__thiscall stub_built0(void *self) { record(0x81, self); return built_result(self); }
static void *__thiscall stub_built1(void *self, uint32_t a) { record(0x81, self); record(0x81, arg_value(a)); return built_result(self); }
// a singleton's constructor (it keeps itself in its global, as the game's does)
// (its members, past what built_result fills, point into the arena's filled places, as a global's object's do: so reads
// through them land)
static void single_members(void *self) {
    for (int k = 0x2a; k < 0x40; k++) ((uint32_t *)self)[k] = (uint32_t)(uintptr_t)(g_arena + 0xc000 + (k * 7 % 16) * 0x40);
    for (int k = 3; k < 0x2a; k++) ((uint32_t *)self)[k] = (uint32_t)(uintptr_t)(g_arena + 0xc000 + ((k * 5 + 3) % 16) * 0x40);
}
static void *__thiscall stub_single0(void *self) { void *r = stub_built0(self); single_members(self); *(void *volatile *)(uintptr_t)g_titled_single_global = self; return r; }
static void *__thiscall stub_single1(void *self, uint32_t a) { void *r = stub_built1(self, a); single_members(self); *(void *volatile *)(uintptr_t)g_titled_single_global = self; return r; }
static void *__thiscall stub_built2(void *self, uint32_t a, uint32_t b) { record(0x81, self); record(0x81, arg_value(a)); record(0x81, arg_value(b)); return built_result(self); }
static void *__thiscall stub_built3(void *self, uint32_t a, uint32_t b, uint32_t c) { record(0x81, self); record(0x81, arg_value(a)); record(0x81, arg_value(b)); record(0x81, arg_value(c)); return built_result(self); }
static void *__thiscall stub_single2(void *self, uint32_t a, uint32_t b) { void *r = stub_built2(self, a, b); single_members(self); *(void *volatile *)(uintptr_t)g_titled_single_global = self; return r; }
static void *__thiscall stub_single3(void *self, uint32_t a, uint32_t b, uint32_t c) { void *r = stub_built3(self, a, b, c); single_members(self); *(void *volatile *)(uintptr_t)g_titled_single_global = self; return r; }
static void *__thiscall stub_built4(void *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    record(0x81, self); record(0x81, arg_value(a)); record(0x81, arg_value(b)); record(0x81, arg_value(c)); record(0x81, arg_value(d)); return built_result(self);
}
// the same, for a function called on no object (it never reads ecx: whatever ecx holds is left alone); what it gives back
// is an object of its own, from the arena's memory
extern const uint32_t titled_value_callees[];
// a call whose result is a counted text's holder (titled_holder_callees): what it was given noted, a holder with a text
// of its own given back
extern const uint32_t titled_holder_callees[];
static void *holder_result() { void **h = (void **)stub_plain_alloc(0x10); *h = new_wide_block(); return h; }
static void *__thiscall stub_holder0(void *self) { record(0x8a, self); return holder_result(); }
static void *__thiscall stub_holder1(void *self, uint32_t a) { record(0x8a, self); record(0x8a, arg_value(a)); return holder_result(); }
static void *__thiscall stub_holder2(void *self, uint32_t a, uint32_t b) { record(0x8a, self); record(0x8a, arg_value(a)); record(0x8a, arg_value(b)); return holder_result(); }
static void *__thiscall stub_holder3(void *self, uint32_t a, uint32_t b, uint32_t c) {
    record(0x8a, self); record(0x8a, arg_value(a)); record(0x8a, arg_value(b)); record(0x8a, arg_value(c)); return holder_result();
}
// (the table of an argument's object whose virtual calls give back a counted text's holder: each slot that stand-in)
static void *const titled_holder_table[16] = {(void *)stub_holder0, (void *)stub_holder0, (void *)stub_holder0, (void *)stub_holder0,
                                               (void *)stub_holder0, (void *)stub_holder0, (void *)stub_holder0, (void *)stub_holder0,
                                               (void *)stub_holder0, (void *)stub_holder0, (void *)stub_holder0, (void *)stub_holder0,
                                               (void *)stub_holder0, (void *)stub_holder0, (void *)stub_holder0, (void *)stub_holder0};
extern const uint32_t titled_holder_args[];
static uint32_t g_titled_holder_mask;
static void *__thiscall stub_plain_value0(void *) {   // a plain function taken as a number: the case's, either sign, one per call
    uint32_t &n = *arena<uint32_t>(ALLOC_NEXT - 8);
    record(0x84, nullptr);
    return (void *)(uintptr_t)((arena<uint32_t>(0x4c)[0] ^ n++ * 0x9e3779b1u) * 0x85ebca6bu);
}
static void *plain_result() { return built_result(stub_plain_alloc(0x200)); }   // (0x200: room for members read far in)
// the same for a fastcall (ecx and edx values, noted as values; what it gives back an object of its own)
static void *__fastcall stub_fast0(uint32_t c, uint32_t d) { record(0x85, arg_value(c)); record(0x85, arg_value(d)); return plain_result(); }
static void *__fastcall stub_fast1(uint32_t c, uint32_t d, uint32_t a) { record(0x85, arg_value(c)); record(0x85, arg_value(d)); record(0x85, arg_value(a)); return plain_result(); }
static void *__fastcall stub_fast2(uint32_t c, uint32_t d, uint32_t a, uint32_t b) {
    record(0x85, arg_value(c)); record(0x85, arg_value(d)); record(0x85, arg_value(a)); record(0x85, arg_value(b)); return plain_result();
}
static void *__fastcall stub_fast3(uint32_t c, uint32_t d, uint32_t a, uint32_t b, uint32_t e) {
    record(0x85, arg_value(c)); record(0x85, arg_value(d)); record(0x85, arg_value(a)); record(0x85, arg_value(b)); record(0x85, arg_value(e)); return plain_result();
}
static void *__fastcall stub_fast4(uint32_t c, uint32_t d, uint32_t a, uint32_t b, uint32_t e, uint32_t f) {
    record(0x85, arg_value(c)); record(0x85, arg_value(d)); record(0x85, arg_value(a)); record(0x85, arg_value(b)); record(0x85, arg_value(e)); record(0x85, arg_value(f));
    return plain_result();
}
static void *__thiscall stub_plain0(void *) { record(0x82, nullptr); return plain_result(); }
static void *__thiscall stub_plain1(void *, uint32_t a) { record(0x82, arg_value(a)); return plain_result(); }
static void *__thiscall stub_plain2(void *, uint32_t a, uint32_t b) { record(0x82, arg_value(a)); record(0x82, arg_value(b)); return plain_result(); }
static void *__thiscall stub_plain3(void *, uint32_t a, uint32_t b, uint32_t c) { record(0x82, arg_value(a)); record(0x82, arg_value(b)); record(0x82, arg_value(c)); return plain_result(); }
static void *__thiscall stub_plain4(void *, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    record(0x82, arg_value(a)); record(0x82, arg_value(b)); record(0x82, arg_value(c)); record(0x82, arg_value(d)); return plain_result();
}
// the same, for a function the game makes with edx = the window being made (noted too)
static void *__fastcall stub_built_edx0(void *self, void *edx) { record(0x81, self); record(0x81, arg_value((uint32_t)(uintptr_t)edx)); return built_result(self); }
static void *__fastcall stub_built_edx1(void *self, void *edx, uint32_t a) {
    record(0x81, self); record(0x81, arg_value((uint32_t)(uintptr_t)edx)); record(0x81, arg_value(a)); return built_result(self);
}
static void *__fastcall stub_built_edx2(void *self, void *edx, uint32_t a, uint32_t b) {
    record(0x81, self); record(0x81, arg_value((uint32_t)(uintptr_t)edx)); record(0x81, arg_value(a)); record(0x81, arg_value(b)); return built_result(self);
}
static void *__fastcall stub_built_edx3(void *self, void *edx, uint32_t a, uint32_t b, uint32_t c) {
    record(0x81, self); record(0x81, arg_value((uint32_t)(uintptr_t)edx)); record(0x81, arg_value(a)); record(0x81, arg_value(b)); record(0x81, arg_value(c));
    return built_result(self);
}
// the same, for a function given a run's locals: what a place on the stack holds is noted for 16 bytes
static void built_rect_arg(uint32_t v) {
    uintptr_t here = (uintptr_t)&v;
    if (v > here && v - here < 0x4000)
        for (int k = 0; k < 4; k++) record(0x81, (void *)(uintptr_t)(0x57ac0000u ^ ((uint32_t *)(uintptr_t)v)[k]));
    else record(0x81, arg_value(v));
}
static void *__thiscall stub_built_rect1(void *self, uint32_t a) { record(0x81, self); built_rect_arg(a); return built_result(self); }
static void *__thiscall stub_built_rect2(void *self, uint32_t a, uint32_t b) { record(0x81, self); built_rect_arg(a); built_rect_arg(b); return built_result(self); }
static void *__thiscall stub_built_rect3(void *self, uint32_t a, uint32_t b, uint32_t c) {
    record(0x81, self); built_rect_arg(a); built_rect_arg(b); built_rect_arg(c); return built_result(self);
}
static void *__thiscall stub_built_rect4(void *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    record(0x81, self); built_rect_arg(a); built_rect_arg(b); built_rect_arg(c); built_rect_arg(d); return built_result(self);
}
static void __thiscall stub_field_add(void *list, void *field) { record(0x82, list); record(0x82, field); }
static void __thiscall stub_struct_total(void *field) { record(0x86, field); }   // a struct field closed (slot 0, 0x808D90)
static void *__thiscall stub_list_member(void *self, void *list) { record(0x87, self); record(0x87, list); return self; }   // ListMember::ListMember (0xCD2B30)
static void *__thiscall stub_member_built1(void *self, void *a) { record(0x88, self); record(0x88, a); return self; }   // a member constructor given one value (0x981EE0: the object it belongs to)
static void *__thiscall stub_name_make(uint32_t *text, uintptr_t name, uint32_t zero) { record(0x83, (void *)name); record(0x83, (void *)(uintptr_t)zero); text[0] = 0x4e414d45; return text; }
static void __thiscall stub_name_give(void *description, uint32_t *text) { record(0x84, description); record(0x84, (void *)(uintptr_t)text[0]); }
static void __thiscall stub_name_free(uint32_t *text) { record(0x85, (void *)(uintptr_t)text[0]); }
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
static void *__fastcall stub_member_handle_of(void *object) {   // Class::Construct's: a handle with a list at 0x600
    record(0x805700, object);
    return arena<void>(0x600);
}
// Class::Construct: stand-ins for the functions its steps call on members and on new objects (class_member_fns: each
// with how many values it takes). They record what they were given and give back something other than the object, so a
// member that should get the memory is told from one that should get the result. The array member's constructor
// (0x611A80) also fills in {items, count}, which the constructors that mark the array's end read back.
extern const uint32_t class_member_fns[];
static void *__thiscall stub_class_member0(void *self) { record(0x90, self); return (uint8_t *)self + 8; }
static void *__thiscall stub_class_member1(void *self, void *a) { record(0x91, self); record(0x91, a); return (uint8_t *)self + 8; }
static void *__thiscall stub_class_member2(void *self, void *a, void *b) { record(0x92, self); record(0x92, a); record(0x92, b); return (uint8_t *)self + 8; }
static void *__thiscall stub_class_member3(void *self, void *a, void *b, void *c) { record(0x93, self); record(0x93, a); record(0x93, b); record(0x93, c); return (uint8_t *)self + 8; }
static void *__thiscall stub_class_member4(void *self, void *a, void *b, void *c, void *d) {
    record(0x94, self); record(0x94, a); record(0x94, b); record(0x94, c); record(0x94, d);
    return (uint8_t *)self + 8;
}
static void *__thiscall stub_class_array(uint32_t *self, void *a, void *b) {
    record(0x95, self); record(0x95, a); record(0x95, b);
    self[0] = (uint32_t)(uintptr_t)arena<void>(0x8900);
    self[1] = (uint32_t)((uintptr_t)self >> 2) % 61;   // a count of its own for each member
    return self;
}
// and for the hand-written ends: the shared lighting object (0x425920; its list at +0x78 is arena 0x8B00), a find in that
// list given five values, a member text made on the object (which notes a flag the end sets before it)
static void *__cdecl stub_class_lighting() { record(0x96, nullptr); return arena<void>(0x8a00); }
static uint32_t __thiscall stub_class_find5(void *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    record(0x97, self); record(0x97, (void *)(uintptr_t)a); record(0x97, (void *)(uintptr_t)b); record(0x97, (void *)(uintptr_t)c);
    record(0x97, (void *)(uintptr_t)d); record(0x97, (void *)(uintptr_t)e);
    return *arena<uint32_t>(0x8b00) + a * 7 + b * 5 + c * 3 + d;
}
static void __thiscall stub_class_info_text(uint8_t *self) { record(0x98, self); record(0x98, (void *)(uintptr_t)self[0x2b0]); }
// a resource reference made on the stack (its place differs between the two runs: what is noted is what it holds), a
// named text made from one, the reference let go
static void *__thiscall stub_class_reference(uint32_t *self) { record(0x99, nullptr); self[0] = 0xfa5388, self[1] = *arena<uint32_t>(0x8b04); return self; }
static void *__thiscall stub_class_named_text(void *self, uint32_t name, const uint32_t *reference, uint32_t flag) {
    record(0x9a, self); record(0x9a, (void *)(uintptr_t)name); record(0x9a, (void *)(uintptr_t)reference[1]); record(0x9a, (void *)(uintptr_t)flag);
    record(0x9a, (void *)(uintptr_t)arena<uint32_t>(0xa000)[0x290 / 4]);   // the store before it done
    return self;
}
extern const uint32_t class_ref_setters2[];
static void *__thiscall stub_class_named_text2(void *self, uint32_t name, const uint32_t *reference) {
    record(0xa6, self); record(0xa6, (void *)(uintptr_t)name); record(0xa6, (void *)(uintptr_t)reference[1]);
    return self;
}
static void __thiscall stub_class_unreference(uint32_t *self) { record(0x9b, (void *)(uintptr_t)self[1]); }
// set going with a value (noting a value set just before), and a member made for the object with a pair on the stack
static void __thiscall stub_class_start(uint8_t *self, uint32_t a) { record(0x9c, self); record(0x9c, (void *)(uintptr_t)a); record(0x9c, (void *)(uintptr_t)*(uint32_t *)(self + 0x6c)); }
// a window record made from two pairs (passed by place: what they hold is noted) and a value; the water's texts table
// made (setting the global it is kept in); a named text found in a table
static void *__thiscall stub_class_record_pairs(void *self, const uint32_t *a, const uint32_t *b, int32_t c) {
    record(0x9f, self); record(0x9f, (void *)(uintptr_t)a[0]); record(0x9f, (void *)(uintptr_t)a[1]); record(0x9f, (void *)(uintptr_t)b[0]);
    record(0x9f, (void *)(uintptr_t)b[1]); record(0x9f, (void *)(intptr_t)c);
    return (uint8_t *)self + 4;
}
static void __cdecl stub_class_water_table() {   // (sometimes it can't be made: the global stays null)
    record(0xa0, nullptr);
    uint32_t &tries = arena<uint32_t>(0x8b20)[2];
    if (++tries + arena<uint32_t>(0x8b20)[1] % 3 > 2) *(uint32_t *)0x133a274 = arena<uint32_t>(0x8b20)[1];
}
// (the name is noted by what it says: it may be made in a buffer on the stack, at a different place in each run; the text
// made holds that and the table, which its use and its letting go note)
static uint32_t text_hash(const char *s) { uint32_t h = 5381; while (*s) h = h * 33 + (uint8_t)*s++; return h; }
static void *__thiscall stub_class_named_in(uint32_t *self, const char *name, uint32_t table, uint32_t flag) {
    record(0xa1, (void *)(uintptr_t)text_hash(name)); record(0xa1, (void *)(uintptr_t)table); record(0xa1, (void *)(uintptr_t)flag);
    if ((uintptr_t)self - (uintptr_t)arena<void>(0) < sizeof g_arena) record(0xa1, self);   // a member (not on the stack)
    self[1] = text_hash(name) ^ table;
    return self;
}
static void __thiscall stub_class_text_assign(void *self, const uint32_t *from) { record(0xa5, self); record(0xa5, (void *)(uintptr_t)from[1]); }
static void __thiscall stub_class_text_release(const uint32_t *self) { record(0xa6, (void *)(uintptr_t)self[1]); }
static int __cdecl stub_class_sprintf(char *out, const char *format, ...) {
    va_list list;
    va_start(list, format);
    int n = vsnprintf(out, 0x80, format, list);
    va_end(list);
    record(0xa7, (void *)(uintptr_t)text_hash(out));
    return n;
}
static void __thiscall stub_class_water_step(void *self) { record(0xa8, self); record(0xa8, (void *)(uintptr_t)*(uint32_t *)0x133a270); }
static void __thiscall stub_class_water_list(uint32_t *list, uint32_t n) {   // made to hold n entries (each list its own place; a count of its own)
    record(0xa9, list); record(0xa9, (void *)(uintptr_t)n);
    uint32_t &made = *arena<uint32_t>(0x8b34);
    list[0] = (uint32_t)(uintptr_t)arena<void>(0x9000 + 0x400 * (made++ % 3));
    list[1] = (n + made * 3) % 7;
}
// a member given a value; a member given a reference made on the stack (what it holds is noted: its table, handle,
// owner set to itself, flag), and that reference let go (unlinked from the handle's list, as the game's does)
// a named text made in a temporary (what it holds noted when it is let go; sometimes it has no characters), and a wide
// text copied from characters
static void *__thiscall stub_class_named_make(uint32_t *self, uint32_t name, uint32_t table, uint32_t flag) {
    record(0xae, (void *)(uintptr_t)name); record(0xae, (void *)(uintptr_t)table); record(0xae, (void *)(uintptr_t)flag);
    self[1] = name ^ table;
    self[2] = (name ^ *arena<uint32_t>(0x8b30)) % 3 ? (uint32_t)(uintptr_t)arena<void>(0x8c00 + name % 0x40) : 0;
    return self;
}
static void *__thiscall stub_class_wide_from(uint32_t *self, const void *characters) {
    record(0xaf, (void *)characters);
    self[0] = 1, self[3] = 0x6000;
    return self;
}
// slots made for a count (their own place in the arena, each slot's block set or not)
static void __thiscall stub_class_slots(uint32_t *self, uint32_t count) {
    record(0xad, self); record(0xad, (void *)(uintptr_t)count);
    self[0] = (uint32_t)(uintptr_t)arena<void>(0x9c00);
    for (uint32_t k = 0; k < 2 * count; k++) arena<uint32_t>(0x9c00)[k] = (k & 1) || (k * 7 + count) % 3 ? 0x9c80 + k : 0;
}
static void __thiscall stub_class_given(void *self, uint32_t value) { record(0xaa, self); record(0xaa, (void *)(uintptr_t)value); }
static void __thiscall stub_class_given_reference(void *self, const uint32_t *reference) {
    record(0xab, self); record(0xab, (void *)(uintptr_t)reference[0]); record(0xab, (void *)(uintptr_t)reference[1]);
    record(0xab, (void *)(uintptr_t)(reference[4] == (uint32_t)(uintptr_t)reference)); record(0xab, (void *)(uintptr_t)reference[5]);
    record(0xab, (void *)(uintptr_t)*(const uint8_t *)&reference[6]);
}
static void __thiscall stub_class_reference_gone(uint32_t *reference) {
    record(0xac, (void *)(uintptr_t)reference[1]);
    uint32_t *next = (uint32_t *)(uintptr_t)reference[2], *previous = (uint32_t *)(uintptr_t)reference[3];
    previous[0] = (uint32_t)(uintptr_t)next;
    next[1] = (uint32_t)(uintptr_t)previous;
}
static uint32_t g_resource_manager, g_water_manager, g_scale_values[2];   // the game's resource manager and water manager globals, put back after the test
static uint32_t g_water_table;   // the game's water texts table global (0x133A274), put back after the test
// a text made from C text (count 1; the name noted), and the resource manager's two lookups (the text noted, an answer
// of their own, and sometimes another holder of the text, so letting it go is seen)
static void *__thiscall stub_class_text_c(int32_t *self, uint32_t name) { record(0xa2, (void *)(uintptr_t)name); self[0] = 1, self[3] = 0x5000 + name % 0x100; return self; }
static uint32_t class_lookup(uint32_t tag, void *manager, int32_t **text) {
    record(tag, manager); record(tag, *text);
    if (*arena<uint32_t>(0x8b30) & 1) ++**text;
    *arena<uint32_t>(0x8b30) = *arena<uint32_t>(0x8b30) * 0x9e3779b9u + 7;
    return (uint32_t)(uintptr_t)*text ^ tag;
}
static uint32_t __thiscall stub_class_lookup(void *manager, int32_t **text) { return class_lookup(0xa3, manager, text); }
static uint32_t __thiscall stub_class_lookup2(void *manager, int32_t **text) { return class_lookup(0xa4, manager, text); }
static uint32_t __thiscall stub_class_lookup3(void *manager, int32_t **text) { return class_lookup(0xa5, manager, text); }
static void __thiscall stub_class_after_figures(uint8_t *self) { record(0x9e, self); record(0x9e, (void *)(uintptr_t)*(uint32_t *)(self + 0x1bc)); }
static void *__thiscall stub_class_from_pair(void *self, void *object, const uint32_t *pair) {
    record(0x9d, self); record(0x9d, object); record(0x9d, (void *)(uintptr_t)pair[0]); record(0x9d, (void *)(uintptr_t)pair[1]);
    return self;
}
static uint32_t g_weather_defaults[4];   // the game's weather defaults (0x10F1CF8..0x10F1D04), put back after the test
static uintptr_t g_class_description;   // the default description global of the Class::Construct copy being checked
static uint32_t g_class_args;           // and how many arguments it takes (2 to 7)
static uintptr_t g_class_instance;      // and the global it gives the object at the end, if any
static uintptr_t g_class_tail_globals[4];   // and the globals it reads at the end (a game list, a count), if any
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

// Destructor::Steps (tools/dtor_ops.py): stand-ins that record which they are. A member's table has 16 entries taking
// 0, 1 or 2 arguments; the direct calls (members' and bases' destructors on the object, and frees of a pointer) each
// get their own numbered stand-in.
template <int SLOT> static void __thiscall dtor_virtual0(void *self) { record(0x7000 + SLOT, self); }
template <int SLOT> static void __thiscall dtor_virtual1(void *self, uint32_t a) { record(0x7100 + SLOT, self); record(0x7100 + SLOT, (void *)(uintptr_t)a); }
template <int SLOT> static void __thiscall dtor_virtual2(void *self, uint32_t a, uint32_t b) {
    record(0x7200 + SLOT, self); record(0x7200 + SLOT, (void *)(uintptr_t)a); record(0x7200 + SLOT, (void *)(uintptr_t)b);
}
template <int K> static void __thiscall dtor_direct(void *self) { record(0x7400 + K, self); }
template <int K> static void __thiscall dtor_direct1(void *self, uint32_t a) { record(0x7900 + K, self); record(0x7900 + K, (void *)(uintptr_t)a); }
template <int K> static void __thiscall dtor_direct2(void *self, uint32_t a, uint32_t b) {
    record(0x7a00 + K, self); record(0x7a00 + K, (void *)(uintptr_t)a); record(0x7a00 + K, (void *)(uintptr_t)b);
}
template <int K> static void __thiscall dtor_direct3(void *self, uint32_t a, uint32_t b, uint32_t c) {
    record(0x7b00 + K, self); record(0x7b00 + K, (void *)(uintptr_t)a); record(0x7b00 + K, (void *)(uintptr_t)b); record(0x7b00 + K, (void *)(uintptr_t)c);
}
template <int K> static void __cdecl dtor_freed(void *p) { record(0x7600 + K, p); }
#define DTOR16(f) (void *)&f<0>, (void *)&f<1>, (void *)&f<2>, (void *)&f<3>, (void *)&f<4>, (void *)&f<5>, (void *)&f<6>, (void *)&f<7>, \
                  (void *)&f<8>, (void *)&f<9>, (void *)&f<10>, (void *)&f<11>, (void *)&f<12>, (void *)&f<13>, (void *)&f<14>, (void *)&f<15>
#define DTOR48(f) DTOR16(f), (void *)&f<16>, (void *)&f<17>, (void *)&f<18>, (void *)&f<19>, (void *)&f<20>, (void *)&f<21>, (void *)&f<22>, (void *)&f<23>, \
                  (void *)&f<24>, (void *)&f<25>, (void *)&f<26>, (void *)&f<27>, (void *)&f<28>, (void *)&f<29>, (void *)&f<30>, (void *)&f<31>, \
                  (void *)&f<32>, (void *)&f<33>, (void *)&f<34>, (void *)&f<35>, (void *)&f<36>, (void *)&f<37>, (void *)&f<38>, (void *)&f<39>, \
                  (void *)&f<40>, (void *)&f<41>, (void *)&f<42>, (void *)&f<43>, (void *)&f<44>, (void *)&f<45>, (void *)&f<46>, (void *)&f<47>
static void *g_dtor_tables[3][16] = {{DTOR16(dtor_virtual0)}, {DTOR16(dtor_virtual1)}, {DTOR16(dtor_virtual2)}};
static void *g_dtor_direct[4][48] = {{DTOR48(dtor_direct)}, {DTOR48(dtor_direct1)}, {DTOR48(dtor_direct2)}, {DTOR48(dtor_direct3)}};
static void *g_dtor_freed[48] = {DTOR48(dtor_freed)};
static void __stdcall dtor_vec_dtor(void *items, uint32_t size, int32_t count, void *dtor) {
    record(0x7800, items); record(0x7800, (void *)(uintptr_t)size); record(0x7800, (void *)(intptr_t)count); record(0x7800, dtor);
}
static const uint32_t *g_dtor_info;   // the copy under test: {address, steps, then five values a step}
static std::vector<std::pair<uint32_t, uint32_t>> g_dtor_globals;   // the globals it writes or reads, and what they held

// GE_Device::GE_Device: its stand-ins fold what they are given into a running sum at arena 0x20 (there are far more
// calls than the call log holds). The assembler's answers come from a seed (0x28) and a count of calls (0x24): the
// result (sometimes a failure), whether it gives code, and how many bytes the code is.
static void dev_note(uint32_t tag, uint32_t value) {
    uint32_t &sum = *arena<uint32_t>(0x20);
    sum = (sum ^ tag) * 0x01000193u;
    sum = (sum ^ value) * 0x01000193u;
}
static uint32_t dev_next() {
    uint32_t &count = *arena<uint32_t>(0x24);
    uint32_t x = *arena<uint32_t>(0x28) + 0x9e3779b9u * ++count;
    x ^= x >> 15; x *= 0x2c1b3c6du; x ^= x >> 12; x *= 0x297a2d39u; x ^= x >> 15;
    return x;
}
static void *__thiscall dev_base(void *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    dev_note(1, (uint32_t)(uintptr_t)self); dev_note(1, a); dev_note(1, b); dev_note(1, c); dev_note(1, d);
    return self;
}
static void __stdcall dev_array(void *items, uint32_t size, int32_t count, void *ctor) {
    dev_note(2, (uint32_t)(uintptr_t)items); dev_note(2, size); dev_note(2, (uint32_t)count); dev_note(2, (uint32_t)(uintptr_t)ctor);
}
static void *__thiscall dev_forward(void *self, uint32_t a, uint32_t b) { dev_note(3, (uint32_t)(uintptr_t)self); dev_note(3, a); dev_note(3, b); return self; }
static uint32_t __thiscall dev_text_init(void *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    dev_note(4, (uint32_t)(uintptr_t)self); dev_note(4, a); dev_note(4, b); dev_note(4, c); dev_note(4, d);
    return 1;
}
static uint32_t __thiscall dev_name(void *self, uint32_t name) { dev_note(5, (uint32_t)(uintptr_t)self); dev_note(5, name); return name * 3 + 1; }
static void *dev_take(uint32_t size) {   // memory from the arena's 0x2000 on (a size that makes no sense takes 0x100)
    uint32_t &next = *arena<uint32_t>(0x2c);
    if (next < 0x2000) next = 0x2000;
    void *p = arena<void>(next);
    next += size > 0x100 ? 0x100 : (size + 15) & ~15u;
    return p;
}
static void *__cdecl dev_alloc(uint32_t size) {   // sometimes there is no memory
    dev_note(6, size);
    return dev_next() % 16 == 0 ? nullptr : dev_take(size);
}
static void *__cdecl dev_alloc_aligned(uint32_t size) { dev_note(7, size); return dev_take(size); }
static uint32_t __stdcall dev_buffer_release(void *self) { dev_note(8, (uint32_t)(uintptr_t)self); return 0; }
static uint8_t *__stdcall dev_buffer_pointer(void *self) { dev_note(9, (uint32_t)(uintptr_t)self); return arena<uint8_t>(0x7000 + (*arena<uint32_t>(0x34) & 0xfc)); }
static uint32_t __stdcall dev_buffer_size(void *self) { dev_note(10, (uint32_t)(uintptr_t)self); return *arena<uint32_t>(0x30); }
static void *g_dev_buffer_table[5] = {nullptr, nullptr, (void *)dev_buffer_release, (void *)dev_buffer_pointer, (void *)dev_buffer_size};
static void **g_dev_buffer = g_dev_buffer_table;
static int32_t __stdcall dev_assemble(const char *text, uint32_t length, void *a, void *b, uint32_t flags, void ***code, void *errors) {
    dev_note(11, (uint32_t)(uintptr_t)text); dev_note(11, length); dev_note(11, (uint32_t)(uintptr_t)a); dev_note(11, (uint32_t)(uintptr_t)b);
    dev_note(11, flags); dev_note(11, (uint32_t)(uintptr_t)errors);
    const uint32_t x = dev_next();
    const uint32_t kind = (x >> 8) % 10;
    *arena<uint32_t>(0x30) = kind == 0 ? 0 : kind == 1 ? 4 : kind == 2 ? 7 : 8 + (x >> 16) % 41;   // bytes: also none, too few, not whole values
    *arena<uint32_t>(0x34) = x >> 3;
    *code = x % 5 == 0 ? nullptr : (void **)&g_dev_buffer;
    return x % 7 == 0 ? (int32_t)0x80004005 : 0;
}
static const char *__stdcall dev_error_text(int32_t result) { dev_note(12, (uint32_t)result); return (const char *)0xfba084; }
static void __cdecl dev_report(const char *format, const char *file, int32_t line, const char *what) {
    dev_note(13, (uint32_t)(uintptr_t)format); dev_note(13, (uint32_t)(uintptr_t)file); dev_note(13, (uint32_t)line); dev_note(13, (uint32_t)(uintptr_t)what);
}
static void *__thiscall dev_state(void *self, void *device) { dev_note(14, (uint32_t)(uintptr_t)self); dev_note(14, (uint32_t)(uintptr_t)device); return self; }
static void __thiscall dev_pass_shaders(void *self) { dev_note(15, (uint32_t)(uintptr_t)self); }
static void __thiscall dev_clear(void *self, uint32_t a) { dev_note(16, (uint32_t)(uintptr_t)self); dev_note(16, a); }

// Cheats::Init: its stand-ins (using the device test's running sum, call count and memory). The texts are {count, ...,
// buffer}: the first comes back with a count of 1; the join gives a new text with a count of 1 and sometimes leaves
// another holder on the first; the entry's constructor takes its name by value and usually lets it go again.
static void *__cdecl cheat_alloc(uint32_t size) {   // an entry (0x54 bytes) sometimes can't be had
    dev_note(30, size);
    return size == 0x54 && dev_next() % 6 == 0 ? nullptr : dev_take(size);
}
static void __cdecl cheat_free_aligned(void *p) { dev_note(31, (uint32_t)(uintptr_t)p); }
static void __cdecl cheat_free(void *p) { dev_note(32, (uint32_t)(uintptr_t)p); }
static void *__thiscall cheat_text(uint32_t *self, const char *from) {
    dev_note(33, (uint32_t)(uintptr_t)self); dev_note(33, (uint32_t)(uintptr_t)from);
    self[0] = 1, self[3] = 0xb0000000u | (uint32_t)(uintptr_t)self;
    return self;
}
static void *__thiscall cheat_join(uint32_t **first, uint32_t **out, const char *ending) {
    dev_note(34, (uint32_t)(uintptr_t)*first); dev_note(34, (uint32_t)(uintptr_t)ending);
    if (dev_next() % 4 == 0) ++**first;
    uint32_t *made = (uint32_t *)dev_take(0x10);
    made[0] = 1, made[3] = 0xc0000000u | (uint32_t)(uintptr_t)made;
    *out = made;
    return out;
}
static void *__thiscall cheat_entry(void *self, uint32_t zero, uint32_t *name, uint32_t number) {
    dev_note(35, (uint32_t)(uintptr_t)self); dev_note(35, zero); dev_note(35, (uint32_t)(uintptr_t)name); dev_note(35, number);
    if (dev_next() % 3) --*name;
    return dev_next() % 5 ? self : (uint8_t *)self + 4;
}

// RCTDesktop::CreateWindows: its stand-ins (with the device test's running sum, call count and memory, and the cheats
// test's text helpers). The desktop is at arena 0x8000; some stand-ins also note a field of it the rewrite must have set
// (or not yet set) by then. The first allocation (the game's own name) always succeeds, later ones sometimes fail.
static uint32_t *desk_self() { return arena<uint32_t>(0x8000); }
static void *__cdecl desk_alloc(uint32_t size) {
    dev_note(40, size);
    bool first = ++*arena<uint32_t>(0x30) == 1;
    return !first && dev_next() % 5 == 0 ? nullptr : dev_take(size);
}
static uint32_t *desk_text() {
    uint32_t *made = (uint32_t *)dev_take(0x10);
    made[0] = 1, made[3] = 0xd0000000u | (uint32_t)(uintptr_t)made;
    return made;
}
static void *__fastcall desk_empty(uint32_t c) { dev_note(41, c & 0xff); return desk_text(); }
static void *__thiscall desk_join_wide(uint32_t **first, uint32_t **out, uint32_t **other) {   // sometimes the title's own text, or another holder
    dev_note(42, (uint32_t)(uintptr_t)*first); dev_note(42, (uint32_t)(uintptr_t)*other);
    if (dev_next() % 3 == 0) *out = *other, ++**out;
    else *out = desk_text();
    return dev_next() % 4 == 0 ? arena<void>(0x7180) : out;   // sometimes another holder (0x7180)
}
static void *__thiscall desk_copy(uint32_t **self, void *from) {   // gives back the copy, or sometimes another holder
    dev_note(43, (uint32_t)(uintptr_t)from);
    *self = desk_text();
    return dev_next() % 4 == 0 ? arena<void>(0x7180) : self;
}
static void *__thiscall desk_base(uint32_t *self, void *description, uint32_t **title) {
    dev_note(44, (uint32_t)(uintptr_t)self); dev_note(44, (uint32_t)(uintptr_t)description); dev_note(44, (uint32_t)(uintptr_t)*title);
    dev_note(44, self[0]);   // not yet the desktop's table
    return self;
}
static void __thiscall desk_init(void *self) { dev_note(45, (uint32_t)(uintptr_t)self); dev_note(45, desk_self()[0]); dev_note(45, desk_self()[0xd4 / 4]); }
static void *__thiscall desk_list(void *self, uint32_t file, uint32_t line, uint32_t zero, void *desktop, uint32_t global) {
    dev_note(46, (uint32_t)(uintptr_t)self); dev_note(46, file); dev_note(46, line); dev_note(46, zero); dev_note(46, (uint32_t)(uintptr_t)desktop); dev_note(46, global);
    dev_note(46, desk_self()[0x414 / 4]); dev_note(46, desk_self()[0x6578 / 4]);
    return dev_next() % 2 ? self : nullptr;
}
static void *__thiscall desk_list_value(void *self, uint32_t zero, void *desktop, uint32_t value) {
    dev_note(47, (uint32_t)(uintptr_t)self); dev_note(47, zero); dev_note(47, (uint32_t)(uintptr_t)desktop); dev_note(47, value);
    return self;
}
static void __thiscall desk_key(uint32_t *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    dev_note(48, a); dev_note(48, b); dev_note(48, c); dev_note(48, d);
    self[0] = a, self[1] = b, self[2] = c, self[3] = d, self[4] = (a != 0) + (b != 0) + (c != 0) + (d != 0);
}
static void *__thiscall desk_list_key(void *self, uint32_t file, uint32_t line, uint32_t zero, void *desktop, const uint32_t *key) {
    dev_note(49, (uint32_t)(uintptr_t)self); dev_note(49, file); dev_note(49, line); dev_note(49, zero); dev_note(49, (uint32_t)(uintptr_t)desktop);
    for (int k = 0; k < 5; k++) dev_note(49, key[k]);
    return self;
}
static void *__thiscall desk_window(void *self, uint32_t a, uint32_t b) {
    dev_note(50, (uint32_t)(uintptr_t)self); dev_note(50, a); dev_note(50, b); dev_note(50, desk_self()[0x6550 / 4]);
    return dev_next() % 3 ? self : (uint8_t *)self + 8;
}
static void *__thiscall desk_message(void *self, void *desktop) {
    dev_note(51, (uint32_t)(uintptr_t)self); dev_note(51, (uint32_t)(uintptr_t)desktop); dev_note(51, desk_self()[0x6560 / 4]); dev_note(51, desk_self()[0x6558 / 4]);
    return dev_next() % 3 ? self : (uint8_t *)self + 8;
}
static void __thiscall desk_show(void *self, uint32_t a) {
    dev_note(52, (uint32_t)(uintptr_t)self); dev_note(52, a); dev_note(52, desk_self()[0x94 / 4]); dev_note(52, arena<uint8_t>(0x7000)[4]);
}

// DebugEditor::Number: its stand-ins (the desktop test's allocation, texts and running sum). The editor is at arena
// 0x8000. Texts given by their place are noted by what they hold; window parts give back themselves or another place.
static void __thiscall ed_empty_text(uint32_t **into, uint32_t a) { dev_note(60, a); *into = desk_text(); }
static void *__cdecl ed_alloc(uint32_t size) {   // (texts, 16 bytes, always: the game lets a missing one go and crashes)
    dev_note(67, size);
    return size != 0x10 && dev_next() % 5 == 0 ? nullptr : dev_take(size);
}
static void *__thiscall ed_base(uint32_t *self, uint32_t **name, uint32_t a, uint32_t b, uint32_t c) {
    dev_note(61, (uint32_t)(uintptr_t)self); dev_note(61, (uint32_t)(uintptr_t)*name); dev_note(61, a); dev_note(61, b); dev_note(61, c);
    dev_note(61, self[0]);   // not yet the editor's table
    return self;
}
static void *__thiscall ed_add(uint32_t *self, void *child) {
    dev_note(62, (uint32_t)(uintptr_t)self); dev_note(62, (uint32_t)(uintptr_t)child); dev_note(62, self[0]);
    return self;
}
static void *__thiscall ed_font(uint8_t *self, uint32_t **text, uint32_t size, uint32_t flags) {
    dev_note(63, (uint32_t)(uintptr_t)self); dev_note(63, (uint32_t)(uintptr_t)*text); dev_note(63, size); dev_note(63, flags);
    return dev_next() % 3 ? self : self + 4;
}
static void *__thiscall ed_part(uint8_t *self, uint32_t **text, void *inner, const uint32_t *size, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    dev_note(64, (uint32_t)(uintptr_t)self); dev_note(64, (uint32_t)(uintptr_t)*text); dev_note(64, (uint32_t)(uintptr_t)inner);
    dev_note(64, size[0]); dev_note(64, size[1]); dev_note(64, a); dev_note(64, b); dev_note(64, c); dev_note(64, d); dev_note(64, e);
    return dev_next() % 3 ? self : self + 8;
}
static void *__thiscall ed_slider(uint8_t *self, uint32_t **text, void *range, const uint32_t *size, uint32_t a, uint32_t b) {
    dev_note(65, (uint32_t)(uintptr_t)self); dev_note(65, (uint32_t)(uintptr_t)*text); dev_note(65, (uint32_t)(uintptr_t)range);
    dev_note(65, size[0]); dev_note(65, size[1]); dev_note(65, a); dev_note(65, b);
    return self;
}
static void *__thiscall ed_record(uint8_t *self, uint32_t **text, uint32_t a, void *range, uint32_t colour, uint32_t b) {
    dev_note(66, (uint32_t)(uintptr_t)self); dev_note(66, (uint32_t)(uintptr_t)*text); dev_note(66, a); dev_note(66, (uint32_t)(uintptr_t)range);
    dev_note(66, colour); dev_note(66, b);
    return dev_next() % 2 ? self : self + 0x10;
}

// Folder::LoadFiles: its stand-ins. The object is at arena 0x8000; the folder walker's walk is at 0x9000 {its table, the
// entry it is on, how many entries there are, a seed}. Texts made from C text keep that text's address at +4 (so
// String::Compare can compare them); an entry's name is ".", "..", or a name of its own.
static const uint32_t lf_rows[][4] = {
#define LOAD_FOLDERS(at, clear, folder, load, ...) {at, clear, folder, load},
#include "../../forever/src/rewrite/load_folders.inc"
#undef LOAD_FOLDERS
};
static void *__cdecl lf_alloc(uint32_t size) { dev_note(70, size); return dev_take(size); }
static void *__thiscall lf_text_c(uint32_t *self, uint32_t text) { dev_note(71, text); self[0] = 1, self[1] = text, self[3] = 0x7000; return self; }
static void __thiscall lf_clear(void *self) { dev_note(72, (uint32_t)(uintptr_t)self); }
static void __thiscall lf_load(void *self, uint32_t **name) { dev_note(73, (uint32_t)(uintptr_t)self); dev_note(73, (*name)[1]); }
static uint32_t *lf_walk() { return arena<uint32_t>(0x9000); }
static uint8_t __thiscall lf_valid(void *) { dev_note(74, lf_walk()[1]); return lf_walk()[1] < lf_walk()[2]; }
static uint8_t __thiscall lf_is_file(void *) { dev_note(75, lf_walk()[1]); return (lf_walk()[1] * 7 + lf_walk()[3]) % 4 != 0; }
static void __thiscall lf_next(void *) { dev_note(76, lf_walk()[1]); lf_walk()[1]++; }
static void *lf_table[5] = {nullptr, (void *)lf_valid, (void *)lf_next, nullptr, (void *)lf_is_file};
static void *__thiscall lf_walker(uint32_t *self, uint32_t **folder, uint32_t **pattern) {
    dev_note(77, (*folder)[1]); dev_note(77, (*pattern)[1]);
    lf_walk()[0] = (uint32_t)(uintptr_t)lf_table;
    self[0] = 0x5151, self[1] = (uint32_t)(uintptr_t)lf_walk();
    return self;
}
static uint32_t **__thiscall lf_entry_name(uint32_t *self, uint32_t **into) {   // ".", "..", or a name of its own
    dev_note(78, self[0]);
    uint32_t *name = desk_text();
    uint32_t pick = (lf_walk()[1] + lf_walk()[3] / 3) % 5;
    name[1] = pick == 0 ? 0xf27994 : pick == 1 ? 0xf27840 : 0x100 + lf_walk()[1];
    *into = name;
    return into;
}
static int32_t __thiscall lf_compare(uint32_t **self, uint32_t **other) { dev_note(79, (*self)[1]); dev_note(79, (*other)[1]); return (int32_t)((*self)[1] - (*other)[1]); }
static void *__thiscall lf_copy(uint32_t **self, uint32_t **from) { dev_note(80, (*from)[1]); uint32_t *made = desk_text(); made[1] = (*from)[1] ^ 0x1000; *self = made; return self; }
static void __thiscall lf_walker_gone(uint32_t *self) { dev_note(81, self[0]); }

// Window::MessageNumbered: its stand-ins. The six globals of the copy being checked are set for each case (none, or a
// wide text in the arena) and put back after; texts keep what they were made from at +4.
static const uint32_t msg_rows[][7] = {
#define MESSAGES(at, p0, p1, p2, p3, p4, p5, ...) {at, p0, p1, p2, p3, p4, p5},
#include "../../forever/src/rewrite/messages.inc"
#undef MESSAGES
};
static uint32_t g_msg_globals[6], g_msg_saved[6];
static void *__cdecl msg_alloc(uint32_t size) { dev_note(85, size); return dev_take(size); }
static void *__thiscall msg_wide(uint32_t *self, const void *characters) {
    dev_note(86, (uint32_t)(uintptr_t)characters);
    self[0] = 1, self[1] = (uint32_t)(uintptr_t)characters, self[3] = 0x7100;
    return dev_next() % 4 ? self : self;   // (the text made in place)
}
static void *__thiscall msg_text_c(uint32_t *self, const char *text) { dev_note(87, text_hash(text)); self[0] = 1, self[1] = text_hash(text), self[3] = 0x7200; return self; }
static int __cdecl msg_sprintf(char *out, const char *format, ...) {
    va_list list;
    va_start(list, format);
    int n = vsnprintf(out, 0x20, format, list);
    va_end(list);
    dev_note(88, (uint32_t)(uintptr_t)format); dev_note(88, text_hash(out));
    return n;
}
static void __fastcall msg_replace(uint32_t **text, uint32_t **mark, uint32_t **with) {   // every mark replaced: noted, the text changed in place
    dev_note(89, (*text)[1]); dev_note(89, (*mark)[1]); dev_note(89, (*with)[1]);
    (*text)[1] = (*text)[1] * 31 + (*with)[1];
}

// Money::FormatNamed: its stand-ins. The copy's table of ten named texts and its fallback (game globals) are set for each
// case to named texts in the arena (0x7600 on: {.., .., their characters or none}) and put back after.
static const uint32_t mf_rows[][4] = {
#define MONEY_FORMATS(at, table, letter, fallback, ...) {at, table, letter, fallback},
#include "../../forever/src/rewrite/money_formats.inc"
#undef MONEY_FORMATS
};
static uint32_t g_mf_table, g_mf_fallback, g_mf_saved[11];
static void *__thiscall mf_language(uint32_t **self) { dev_note(90, 0); uint32_t *t = desk_text(); t[1] = 0x4c414e47; *self = t; return self; }
static void *__thiscall mf_join(uint32_t **self, uint32_t **out, uint32_t **other) {
    dev_note(91, (*self)[1]); dev_note(91, (*other)[1]);
    uint32_t *t = desk_text(); t[1] = (*self)[1] * 33 + (*other)[1]; *out = t;
    return out;
}
static void *__thiscall mf_copy(uint32_t **self, uint32_t **from) { dev_note(92, (*from)[1]); uint32_t *t = desk_text(); t[1] = (*from)[1]; *self = t; return self; }
static void *__thiscall mf_named(uint32_t *self, uint32_t **name, uint32_t table, uint32_t flag) {   // found (bit 0 of +4) or not, by the name
    dev_note(93, (*name)[1]); dev_note(93, table); dev_note(93, flag);
    uint32_t pick = ((*name)[1] ^ *arena<uint32_t>(0x44)) % 3;
    self[1] = pick != 0, self[2] = pick == 2 ? (uint32_t)(uintptr_t)arena<void>(0x7700) : 0;
    return self;
}
static uint8_t __thiscall mf_bit0(uint32_t *self, uint32_t a) { dev_note(94, a); return (uint8_t)(self[1] & 1); }
static void __thiscall mf_named_gone(uint32_t *self) { dev_note(95, self[1]); }

// Condition::CurrentText: its stand-ins. The condition at arena 0x8000 has a table whose slot 0x68 gives its value (from
// arena 0x48, as a float); texts keep what they hold at +4.
static void __fastcall ct_put_together(uint32_t **into, const void *characters, uint32_t **with) {
    dev_note(96, (uint32_t)(uintptr_t)*into); dev_note(96, (uint32_t)(uintptr_t)characters); dev_note(96, (*with)[1]);
    uint32_t *t = desk_text(); t[1] = (uint32_t)(uintptr_t)characters * 7 + (*with)[1]; *into = t;
}
static long double __thiscall ct_value(void *self) { dev_note(97, (uint32_t)(uintptr_t)self); return *arena<float>(0x48); }
static void *ct_table[27];
static void *__thiscall ct_timing(void *self, uint32_t **out, int32_t part) {
    dev_note(98, (uint32_t)(uintptr_t)self); dev_note(98, (uint32_t)part);
    uint32_t *t = desk_text(); t[1] = (uint32_t)part; *out = t;
    return out;
}

// Objectives::LabelledNumber: its stand-ins (the copy's own label function among them). Texts keep what they hold at +4.
static const uint32_t ot_rows[][2] = {
#define NUMBER_TEXTS(at, label, ...) {at, label},
#define MONEY_TEXTS(at, label, ...) {at, label},
#include "../../forever/src/rewrite/number_texts.inc"
#include "../../forever/src/rewrite/money_texts.inc"
#undef NUMBER_TEXTS
#undef MONEY_TEXTS
};
static void *__fastcall ot_decimal(uint32_t **into, int32_t value) { dev_note(100, (uint32_t)value); uint32_t *t = desk_text(); t[1] = (uint32_t)value; *into = t; return into; }
static void *__fastcall ot_money(uint32_t **into, int32_t value, uint32_t flag) {
    dev_note(101, (uint32_t)value); dev_note(101, flag);
    uint32_t *t = desk_text(); t[1] = (uint32_t)value * 3 + 1; *into = t;
    return into;
}
static void *__thiscall ot_label(uint32_t **into) { dev_note(102, 0); uint32_t *t = desk_text(); t[1] = 0x1abe1; *into = t; return into; }

// Text::NumberedName, Report::MissingResourceSymbol, Animal::Point: their stand-ins
static void *__fastcall ms_box(uint32_t **text, uint32_t **title) { dev_note(105, (*text)[1]); dev_note(105, (*title)[1]); return nullptr; }
static uint8_t __thiscall ms_flag(uint32_t *self, uint32_t a) { dev_note(106, a); return (uint8_t)(self[1] & 1); }
static void *__thiscall ms_name(uint32_t **self, const uint8_t *from) { dev_note(107, (uint32_t)(uintptr_t)from); uint32_t *t = desk_text(); t[1] = 0x4e414d45; *self = t; return self; }
static void __thiscall ap_rect(void *window, const uint32_t *rect) {
    dev_note(108, (uint32_t)(uintptr_t)window);
    for (int k = 0; k < 4; k++) dev_note(108, rect[k]);
}
// the numbered names' register wrappers are called with esi set (where the text goes) and the number on the stack
__attribute__((naked)) static void *call_with_esi(void *fn, void *out, int32_t number) {
    __asm__("pushl %esi\n\tmovl 12(%esp), %esi\n\tpushl 16(%esp)\n\tcalll *12(%esp)\n\tpopl %esi\n\tret");
}

// Text::DecimalWithUnit: its stand-ins (the copy's own unit function among them)
static const uint32_t dt_rows[][2] = {
#define DECIMAL_TEXTS(at, unit, ...) {at, unit},
#include "../../forever/src/rewrite/decimal_texts.inc"
#undef DECIMAL_TEXTS
};
static void __thiscall dt_point(char *written) {   // the language's decimal point: noted, and the point made a comma
    dev_note(110, text_hash(written));
    for (char *c = written; *c; c++)
        if (*c == '.') *c = ',';
}
static void *__fastcall dt_from_c_and(uint32_t **into, const char *text, uint32_t **then) {
    dev_note(111, text_hash(text)); dev_note(111, (*then)[1]);
    uint32_t *t = desk_text(); t[1] = text_hash(text) ^ (*then)[1]; *into = t;
    return into;
}

// Effect::WithinBox: its stand-ins (the copy's own three functions), and its rows
static const uint32_t eb_rows[][4] = {
#define BOUNDS_A(at, update, other, grow, ...) {at, update, other, grow},
#define BOUNDS_B(at, update, other, grow, ...) {at, update, other, grow},
#include "../../forever/src/rewrite/bounds_a.inc"
#include "../../forever/src/rewrite/bounds_b.inc"
#undef BOUNDS_A
#undef BOUNDS_B
};
static void __thiscall eb_update(uint8_t *self, uint32_t a) {   // brought up to date: noted, and its place moved a little
    record(0xb0, self); record(0xb0, (void *)(uintptr_t)a);
    *(float *)(self + 0xc) += 1.0f;
}
static int32_t __thiscall eb_other(void *self, uint32_t a, void *box) { record(0xb1, self); record(0xb1, box); return 7; }
static int32_t __thiscall eb_grow(void *self, void *box, uint32_t a, void *place, void *size) {
    record(0xb2, self); record(0xb2, box); record(0xb2, (void *)(uintptr_t)a); record(0xb2, place); record(0xb2, size);
    return (int32_t)(*arena<uint32_t>(0x44) & 0xff);
}

// Effect::RayHit: its stand-ins (the copy's own three functions, and the ray-box test noting the box bit for bit)
static const uint32_t ef_rows[][4] = {
#define EFFECT_FLAGS(at, update, other, hit, ...) {at, update, other, hit},
#include "../../forever/src/rewrite/effect_flags.inc"
#undef EFFECT_FLAGS
};
static uint8_t __fastcall ef_slabs(void *origin, void *direction, const uint32_t *low, const uint32_t *high, uint32_t *out) {
    record(0xb3, origin); record(0xb3, direction);
    for (int k = 0; k < 3; k++) record(0xb3, (void *)(uintptr_t)low[k]), record(0xb3, (void *)(uintptr_t)high[k]);
    *out = 0x5a5a;
    return (uint8_t)((low[0] ^ high[2] ^ *arena<uint32_t>(0x44)) & 1);
}

// Window::TitledFromDescription: its stand-ins (the desktop test's texts, a named text reference, the base window)
static const uint32_t tw_rows[][4] = {
#define TITLED_WINDOWS(at, description, table, second_table, ...) {at, description, table, second_table},
#include "../../forever/src/rewrite/titled_windows.inc"
#undef TITLED_WINDOWS
};
static uint32_t g_tw_global, g_tw_saved;
static void *__thiscall tw_reference(uint32_t *self, uint32_t name, uint32_t table, uint32_t flag) {
    dev_note(115, name); dev_note(115, table); dev_note(115, flag);
    self[1] = name ^ 0x55;
    return dev_next() % 3 ? self : (void *)arena<void>(0x7180);
}
static void *__thiscall tw_base(uint32_t *self, void *description, uint32_t **title, uint32_t *reference, uint32_t a) {
    dev_note(116, (uint32_t)(uintptr_t)self); dev_note(116, (uint32_t)(uintptr_t)description); dev_note(116, (uint32_t)(uintptr_t)*title);
    dev_note(116, reference[1]); dev_note(116, a); dev_note(116, self[0]);
    return self;
}
static void __thiscall tw_reference_gone(uint32_t *self) { dev_note(117, self[1]); }

// Folder::SpecialOrDocuments: its stand-ins. Texts keep what they hold at +4 (the empty text: the hash of ""). The
// special folder's path is sometimes empty; whether a path is there comes from the arena (0x44).
static const uint32_t fp_rows[][4] = {
#define FOLDER_PATHS(at, folder, under_documents, fallback, ...) {at, folder, under_documents, fallback},
#include "../../forever/src/rewrite/folder_paths.inc"
#undef FOLDER_PATHS
};
static void *__fastcall fp_special(uint32_t **into, uint32_t folder) {
    dev_note(120, folder);
    uint32_t *t = desk_text(); t[1] = dev_next() % 2 ? 5381 : folder * 77; *into = t;
    return into;
}
static int32_t __thiscall fp_compare(uint32_t **self, uint32_t **other) { dev_note(121, (*self)[1]); dev_note(121, (*other)[1]); return (int32_t)((*self)[1] - (*other)[1]); }
static void *__thiscall fp_documents(uint32_t **into) { dev_note(122, 0); uint32_t *t = desk_text(); t[1] = 0xd0c5; *into = t; return into; }
static void *__thiscall fp_fallback(uint32_t **into) { dev_note(123, 0); uint32_t *t = desk_text(); t[1] = 0xfa11; *into = t; return into; }
static uint8_t __thiscall fp_there(uint32_t **path) { dev_note(124, (*path)[1]); return (uint8_t)(((*path)[1] ^ *arena<uint32_t>(0x44)) & 1); }

// Designer::DefaultName: texts as the game keeps them, {count, length, room, characters (two bytes each)}, all in the
// arena (from the device test's memory), with stand-ins that work on them for real: made from C text, from n
// characters, a part, a copy, compared, given room (made their own first when shared), looked up by name (found or not
// by what they say), and the by-value check (0x401310, which lets its text go).
struct DnText {
    int32_t count;
    uint32_t length, room;
    uint16_t *characters;
};
static DnText *dn_make(DnText *t, const uint16_t *from, uint32_t length) {
    t->count = 1, t->length = length, t->room = (length + 16) & ~15u;
    t->characters = (uint16_t *)dev_take(2 * t->room);
    for (uint32_t k = 0; k < length; k++) t->characters[k] = from[k];
    t->characters[length] = 0;
    return t;
}
static uint32_t dn_hash(const DnText *t) {
    uint32_t h = 5381;
    for (uint32_t k = 0; k < t->length; k++) h = h * 33 + t->characters[k];
    return h;
}
static void *__thiscall dn_text_c(DnText *self, const char *text) {
    uint16_t wide[64];
    uint32_t n = 0;
    while (text[n] && n < 63) wide[n] = (uint8_t)text[n], n++;
    dev_note(130, n);
    return dn_make(self, wide, n);
}
static void *__thiscall dn_from_n(DnText *self, const uint16_t *from, uint32_t length) { dev_note(131, length); return dn_make(self, from, length); }
static void *__thiscall dn_part(DnText **self, DnText **into, uint32_t from, uint32_t length) {
    dev_note(132, from); dev_note(132, length);
    const DnText *t = *self;
    if (from > t->length) from = t->length;
    if (length > t->length - from) length = t->length - from;
    *into = dn_make((DnText *)dev_take(0x10), t->characters + from, length);
    return into;
}
static int32_t __thiscall dn_compare(DnText **self, DnText **other) {
    const DnText *a = *self, *b = *other;
    dev_note(133, dn_hash(a)); dev_note(133, dn_hash(b));
    for (uint32_t k = 0; k < a->length && k < b->length; k++)
        if (a->characters[k] != b->characters[k]) return (int32_t)a->characters[k] - b->characters[k];
    return (int32_t)a->length - (int32_t)b->length;
}
static void *__thiscall dn_copy(DnText **into, DnText **from) { dev_note(134, dn_hash(*from)); *into = dn_make((DnText *)dev_take(0x10), (*from)->characters, (*from)->length); return into; }
static void __thiscall dn_room(DnText **self, uint32_t need) {
    dev_note(135, need);
    DnText *t = *self;
    if (t->count > 1 || t->room <= need) {
        DnText *made = dn_make((DnText *)dev_take(0x10), t->characters, t->length);
        if (made->room <= need) {
            uint16_t *more = (uint16_t *)dev_take(2 * (need + 16));
            for (uint32_t k = 0; k <= made->length; k++) more[k] = made->characters[k];
            made->characters = more, made->room = need + 16;
        }
        --t->count;
        *self = made;
    }
}
static void *__thiscall dn_lookup(uint32_t *self, DnText **key, uint32_t table, uint32_t flag) {
    const uint32_t h = dn_hash(*key);
    dev_note(136, h); dev_note(136, table); dev_note(136, flag);
    self[1] = (h ^ *arena<uint32_t>(0x44)) & 1, self[2] = h % 3 ? (uint32_t)(uintptr_t)arena<void>(0x7700) : 0;
    return self;
}
static uint8_t __thiscall dn_found(uint32_t *self, uint32_t a) { dev_note(137, a); return (uint8_t)self[1]; }
static void __thiscall dn_lookup_gone(uint32_t *self) { dev_note(138, self[1]); }
static void *__thiscall dn_wide(DnText *self, const uint16_t *characters) { dev_note(139, (uint32_t)(uintptr_t)characters); return dn_make(self, characters, 3); }
static uint8_t __stdcall dn_check(DnText *given) {   // (takes its text by value: lets it go)
    dev_note(140, dn_hash(given));
    --given->count;
    return (uint8_t)((given->length ^ *arena<uint32_t>(0x48)) % 7 == 0);
}

// Designer::Save: its stand-ins (on the Designer::DefaultName test's texts), and its rows
static const uint32_t ds_rows[][3] = {
#define DESIGNER_SAVES(at, ending, write, ...) {at, ending, write},
#include "../../forever/src/rewrite/designer_saves.inc"
#undef DESIGNER_SAVES
};
static DnText *ds_c(const char *text) {
    uint16_t wide[64];
    uint32_t n = 0;
    while (text[n] && n < 63) wide[n] = (uint8_t)text[n], n++;
    return dn_make((DnText *)dev_take(0x10), wide, n);
}
static DnText *ds_join_texts(const DnText *a, const DnText *b) {
    uint16_t wide[128];
    uint32_t n = 0;
    for (uint32_t k = 0; k < a->length && n < 127; k++) wide[n++] = a->characters[k];
    for (uint32_t k = 0; k < b->length && n < 127; k++) wide[n++] = b->characters[k];
    return dn_make((DnText *)dev_take(0x10), wide, n);
}
static void *__fastcall ds_empty(uint32_t c) { dev_note(141, c & 0xff); return ds_c(""); }
static void *__thiscall ds_join(DnText **self, DnText **out, DnText **other) { dev_note(142, dn_hash(*self)); dev_note(142, dn_hash(*other)); *out = ds_join_texts(*self, *other); return out; }
static uint8_t __thiscall ds_has_ending(DnText **path, DnText *ending) {   // (takes the ending by value: lets it go)
    dev_note(143, dn_hash(*path)); dev_note(143, dn_hash(ending));
    --ending->count;
    return (uint8_t)(((*path)->length ^ *arena<uint32_t>(0x50)) & 1);
}
static uint8_t __thiscall ds_there(DnText **path) { dev_note(144, dn_hash(*path)); return (uint8_t)((dn_hash(*path) ^ *arena<uint32_t>(0x54)) & 1); }
static void *__thiscall ds_named(uint32_t *self, uint32_t name, uint32_t table, uint32_t flag) {
    dev_note(145, name); dev_note(145, table); dev_note(145, flag);
    self[1] = name;   // (what letting it go notes)
    self[2] = (name ^ *arena<uint32_t>(0x58)) % 3 ? (uint32_t)(uintptr_t)arena<void>(0x7700) : 0;
    return self;
}
static void __fastcall ds_replace(DnText **text, DnText **mark, DnText **with) {
    dev_note(146, dn_hash(*text)); dev_note(146, dn_hash(*mark)); dev_note(146, dn_hash(*with));
    *text = ds_join_texts(*text, *with);
}
static void *__thiscall ds_window(void *self, uint32_t a, uint32_t b, uint32_t c) { dev_note(147, a); dev_note(147, b); dev_note(147, c); return (uint8_t *)self + 4; }
static void __thiscall ds_buttons(void *self, DnText *a, DnText *b, uint32_t c, uint32_t d, DnText *e, uint32_t f) {
    dev_note(148, (uint32_t)(uintptr_t)self); dev_note(148, dn_hash(a)); dev_note(148, dn_hash(b)); dev_note(148, c); dev_note(148, d);
    dev_note(148, dn_hash(e)); dev_note(148, f);
    dev_note(148, (uint32_t)a->count), dev_note(148, (uint32_t)b->count);
    dev_note(148, (uint32_t)(uintptr_t)a), dev_note(148, (uint32_t)(uintptr_t)b), dev_note(148, (uint32_t)(uintptr_t)e);   // (made in the arena: which is which)
}
static void __thiscall ds_given(void *self, DnText *text) { dev_note(149, (uint32_t)(uintptr_t)self); dev_note(149, dn_hash(text)); dev_note(149, (uint32_t)text->count); }

// Designer::OpenFiles: its stand-ins (texts keep what they hold at +4, as the folder test's), the window's table (slot
// 8 gives its size, from the arena) and the designer's (slot 0xE4/4 notes the window kept)
static const uint32_t of_rows[][7] = {
#define OPEN_FILES(at, folder_at, folder, size, make, give, build, ...) {at, folder_at, folder, size, make, give, build},
#include "../../forever/src/rewrite/open_files.inc"
#undef OPEN_FILES
};
static void *__thiscall of_size(void *self, uint32_t *scratch) { dev_note(150, (uint32_t)(uintptr_t)self); return arena<void>(0x8300); }
static void *of_window_table[9] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, (void *)of_size};
static void *__thiscall of_make(uint32_t *self, uint32_t a, uint32_t b, void *c) {
    dev_note(151, a); dev_note(151, b); dev_note(151, (uint32_t)(uintptr_t)c);
    self[0] = (uint32_t)(uintptr_t)of_window_table;
    return self;
}
static void __thiscall of_layout(void *self, uint32_t relation, void *to, const uint32_t *rect) {
    dev_note(152, (uint32_t)(uintptr_t)self); dev_note(152, relation); dev_note(152, (uint32_t)(uintptr_t)to);
    for (int k = 0; k < 4; k++) dev_note(152, rect[k]);
}
static void __thiscall of_give(void *self, uint32_t value) { dev_note(153, (uint32_t)(uintptr_t)self); dev_note(153, value); }
static void __thiscall of_build(void *self) { dev_note(154, (uint32_t)(uintptr_t)self); }
static void __thiscall of_told(uint32_t *self) { dev_note(155, self[0xe0 / 4]); dev_note(155, self[0xe4 / 4]); }
static void *of_designer_table[0xe4 / 4 + 1];

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
static void *__fastcall stub_named_text_ref(uint8_t *self, void *, const char *name, void *texts, uint32_t suffix) {
    g_text_ref = self;
    memset(self, 0xa5, 0x1c);   // all of it, as the real one: a rewrite with too small a space for it overruns its stack
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
static void __fastcall stub_text_ref_free(void *self) {   // (one on the stack told by what it holds)
    if (self == g_text_ref) {
        record(0x77, (void *)1);
        g_text_ref = nullptr;   // (let go: its place no longer holds it, and may be used again for something else)
    } else
        record(0x77, arg_value((uint32_t)(uintptr_t)self));
}
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

// The tests in tools/equiv/tests/*.inc (one file per topic, so people working at the same time never edit the
// same lines): build.py lists them in out/equiv_tests.inc. Each file is read twice: with EQUIV_PART 1 here
// (its stand-ins, at file scope) and with EQUIV_PART 2 at the end of tests() (its tests).
#define EQUIV_PART 1
#include "equiv_tests.inc"
#undef EQUIV_PART

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
            for (int i = -4; i < 0; i++) slots[i] = r.next() % 4 ? arena<uint8_t>(0x500 + (i + 4) * 0x20) : nullptr;   // a negative id reads below
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
                       {0xbcf7d0, (void *)stub_text_from_wide}, {0xbd0490, (void *)stub_wide_empty_shared},
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
    // Object::VFwd2Ret: obj=this->[O0]; obj->vtbl[2](obj,arg,this->[O1]); return arg. vtbl[2] stubbed (thiscall,2 args).
    static uint32_t g_vf2_o0, g_vf2_o1, g_vf2_a;
    Test vfwd2ret{0x50b310,
                  [=](Rng &r) {
                      fresh(r);
                      g_vf2_a = r.next();
                      static void *vt[3] = {nullptr, nullptr, (void *)stub_thiscall2};
                      *(void **)(arena<uint8_t>(0x100) + g_vf2_o0) = arena<void>(0x300); // this->[O0] = obj
                      *(void **)arena<uint8_t>(0x300) = vt;                              // obj's vtable
                      *(uint32_t *)(arena<uint8_t>(0x100) + g_vf2_o1) = r.next();        // this->[O1]
                  },
                  [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), g_vf2_a)}; }};
    vfwd2ret.prepare = [](const rewrite::Entry *e) { const uint32_t *c = rewrite::start_constants(e->at); g_vf2_o0 = c[1]; g_vf2_o1 = c[2]; };
    // Object::SetFieldTail: this->[OFF]=arg; method(this). method stubbed (thiscall, no args).
    static uint32_t g_sft_off, g_sft_m, g_sft_a;
    Test setfieldtail{0x4c1e80,
                      [=](Rng &r) { fresh(r); g_sft_a = r.next(); },
                      [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), g_sft_a); return Result{*(uint32_t *)(arena<uint8_t>(0x100) + g_sft_off)}; }};
    setfieldtail.prepare = [](const rewrite::Entry *e) { const uint32_t *c = rewrite::start_constants(e->at); g_sft_off = c[1]; g_sft_m = c[2]; stubs_for_now({{g_sft_m, (void *)stub_plain_dtor}}); };
    setfieldtail.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Object::Fwd2Clear: method(this,a0,a1); this->[0x30]=0. method stubbed (thiscall, 2 args).
    static uint32_t g_f2c_m, g_f2c_a0, g_f2c_a1;
    Test fwd2clear{0x40b190,
                   [=](Rng &r) { fresh(r); g_f2c_a0 = r.next(); g_f2c_a1 = r.next(); },
                   [](void *fn) { ((void(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x100), g_f2c_a0, g_f2c_a1); return Result{arena<uint32_t>(0x100)[12]}; }};
    fwd2clear.prepare = [](const rewrite::Entry *e) { g_f2c_m = rewrite::start_constants(e->at)[1]; stubs_for_now({{g_f2c_m, (void *)stub_thiscall2}}); };
    fwd2clear.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Object::SetTableVCall: *this=TABLE; obj=this->[4]; obj->vtbl[2](obj,obj). vtbl[2] stubbed (thiscall, 1 arg).
    Test settablevcall{0xbe9410,
                       [=](Rng &r) {
                           fresh(r);
                           static void *vt[3] = {nullptr, nullptr, (void *)stub_thiscall1};
                           *(void **)arena<uint8_t>(0x300) = vt;                 // obj's vtable
                           *(void **)arena<uint8_t>(0x104) = arena<void>(0x300); // this->[4] = obj
                       },
                       [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{arena<uint32_t>(0x100)[0]}; }};
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
    // Object::VCountProduct: n=vtbl[33](this); if n<=0 ret 0; else vtbl[33](this)*sub(this). stubs return 7 and 5.
    Test vcountproduct{0x4aede0,
                       [=](Rng &r) {
                           fresh(r);
                           static void *vt[34];
                           vt[33] = (void *)stub_pos_count;
                           *(void **)arena<uint8_t>(0x100) = vt; // this's vtable
                       },
                       [](void *fn) { return Result{((uint32_t(__thiscall *)(void *))fn)(arena<void>(0x100))}; }};
    vcountproduct.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x4bfa20, (void *)stub_sub_val}}); };
    vcountproduct.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Object::CopyRecords: n=(*src)*144+16 bytes copied src->this; return this. No stubs; src a filled buffer.
    Test copyrecords{0xc71b00,
                     [=](Rng &r) {
                         fresh(r);
                         for (int i = 0; i < 0x200; i++) arena<uint8_t>(0x600)[i] = (uint8_t)r.next(); // src buffer
                         *(uint32_t *)arena<uint8_t>(0x600) = r.next() % 4u;                            // count 0..3 -> n<=448
                     },
                     [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x600))}; }};
    // Object::CondNotify: if this->[OFF] && byte[G1] then singleton(*0x13d0db8)->vtbl[17](obj). vtbl[17]=stub_thiscall1.
    static uint32_t g_cn_off, g_cn_g1;
    Test condnotify{0x919a50,
                    [=](Rng &r) {
                        fresh(r);
                        static void *vt[18];
                        vt[17] = (void *)stub_thiscall1;
                        *(void **)(arena<uint8_t>(0x100) + g_cn_off) = arena<void>(0x300); // this->[OFF] = obj (nonzero)
                        *(uint8_t *)(uintptr_t)g_cn_g1 = 1;                                // global flag != 0
                        *(void **)(uintptr_t)0x13d0db8 = arena<void>(0x400);               // global singleton
                        *(void **)arena<uint8_t>(0x400) = vt;                              // singleton's vtable
                    },
                    [](void *fn) { ((void(__thiscall *)(void *))fn)(arena<void>(0x100)); return Result{0u}; }};
    condnotify.prepare = [](const rewrite::Entry *e) { const uint32_t *c = rewrite::start_constants(e->at); g_cn_off = c[1]; g_cn_g1 = c[2]; };
    // Object::SetTableTail: *(this+OFF)=TABLE; tail-call BASE(this). BASE stubbed (returns const). this always nonzero.
    static uint32_t g_stt_off, g_stt_base;
    Test settabletail{0x40f200,
                      [=](Rng &r) { fresh(r); },
                      [](void *fn) { uint32_t rv = ((uint32_t(__thiscall *)(void *))fn)(arena<void>(0x100)); return Result{rv ^ *(uint32_t *)(arena<uint8_t>(0x100) + g_stt_off)}; }};
    settabletail.prepare = [](const rewrite::Entry *e) { const uint32_t *c = rewrite::start_constants(e->at); g_stt_off = c[1]; g_stt_base = c[3]; stubs_for_now({{g_stt_base, (void *)stub_pos_count}}); };
    settabletail.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Object::CondVRelease: if this->[0x104] then this->vtbl[0](this+4, that); clear it. vtbl[0]=stub_thiscall2.
    Test condvrel{0x7b24a0,
                  [=](Rng &r) {
                      fresh(r);
                      static void *vt[1] = {(void *)stub_thiscall2};
                      *(void **)arena<uint8_t>(0x100) = vt;                            // this's vtable
                      *(void **)(arena<uint8_t>(0x100) + 0x104) = arena<void>(0x300);  // this->[0x104] = obj
                  },
                  [](void *fn) { ((void(__thiscall *)(void *))fn)(arena<void>(0x100)); return Result{*(uint32_t *)(arena<uint8_t>(0x100) + 0x104)}; }};
    // Object::BranchOff: if *(int)this==-1 ret this->[OFF]; else ret this - sub(this). sub stubbed.
    static uint32_t g_bo_off, g_bo_sub;
    static int32_t g_bo_key;   // what the copy compares with: it differs between copies
    Test branchoff{0xbb3e70,
                   [=](Rng &r) { fresh(r); int pick = r.range(0, 3); *(int32_t *)arena<void>(0x100) = pick == 0 ? g_bo_key : pick == 1 ? -1 : pick == 2 ? 2 : 1; },
                   [](void *fn) { return Result{((uint32_t(__thiscall *)(void *))fn)(arena<void>(0x100))}; }};
    branchoff.prepare = [](const rewrite::Entry *e) { const uint32_t *c = rewrite::start_constants(e->at); g_bo_key = *(int8_t *)(e->at + 5); g_bo_off = c[1]; g_bo_sub = c[2]; stubs_for_now({{g_bo_sub, (void *)stub_pos_count}}); };
    branchoff.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Object::CondVCallH: r=helper(a0,0,-1); if r this->vtbl[1](r). helper returns 0x1234; vtbl[1]=stub_thiscall1.
    static uint32_t g_cvh_h, g_cvh_a;
    Test condvcallh{0x67b980,
                    [=](Rng &r) {
                        fresh(r);
                        g_cvh_a = r.next();
                        static void *vt[2] = {nullptr, (void *)stub_thiscall1};
                        *(void **)arena<uint8_t>(0x100) = vt; // this's vtable
                    },
                    [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), g_cvh_a); return Result{0u}; }};
    condvcallh.prepare = [](const rewrite::Entry *e) { g_cvh_h = rewrite::start_constants(e->at)[1]; stubs_for_now({{g_cvh_h, (void *)stub_helper3}}); };
    condvcallh.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Object::TableVRel: *this=TABLE; if this->[OFF] then obj->vtbl[2](obj,obj); clear. vtbl[2]=stub_thiscall1.
    static uint32_t g_tvr_off;
    Test tablevrel{0xc5c390,
                   [=](Rng &r) {
                       fresh(r);
                       static void *vt[3] = {nullptr, nullptr, (void *)stub_thiscall1};
                       *(void **)(arena<uint8_t>(0x100) + g_tvr_off) = arena<void>(0x300);
                       *(void **)arena<uint8_t>(0x300) = vt;
                   },
                   [](void *fn) { ((void(__thiscall *)(void *))fn)(arena<void>(0x100)); return Result{arena<uint32_t>(0x100)[0] ^ *(uint32_t *)(arena<uint8_t>(0x100) + g_tvr_off)}; }};
    tablevrel.prepare = [](const rewrite::Entry *e) { g_tvr_off = rewrite::start_constants(e->at)[2]; };
    // Object::TableDtor: obj=this->[8]; *this=TABLE; if obj: dtor(obj); delete(obj). dtor stubbed; delete global-stubbed.
    static uint32_t g_tdt_dtor;
    Test tabledtor{0x5d0700,
                   [=](Rng &r) { fresh(r); *(void **)(arena<uint8_t>(0x100) + 8) = arena<void>(0x300); },
                   [](void *fn) { ((void(__thiscall *)(void *))fn)(arena<void>(0x100)); return Result{arena<uint32_t>(0x100)[0]}; }};
    tabledtor.prepare = [](const rewrite::Entry *e) { g_tdt_dtor = rewrite::start_constants(e->at)[2]; stubs_for_now({{g_tdt_dtor, (void *)stub_plain_dtor}}); };
    tabledtor.finish = [](const rewrite::Entry *) { unstub_many(); };
    static int32_t g_bsm_key;
    // Object::BSearchMap: sorted int keys at this->[8], parallel vals at this->[0xc], count this->[0x10]; return val or -1.
    Test bsearchmap{0x40b7d0,
                    [=](Rng &r) {
                        fresh(r);
                        int n = r.range(1, 10);
                        int32_t *keys = arena<int32_t>(0x300);
                        uint32_t *vals = arena<uint32_t>(0x400);
                        int32_t k = r.range(-5, 5);
                        for (int i = 0; i < n; i++) { keys[i] = k; k += r.range(1, 4); vals[i] = r.next(); } // strictly ascending
                        *(void **)(arena<uint8_t>(0x100) + 8) = keys;
                        *(void **)(arena<uint8_t>(0x100) + 0xc) = vals;
                        *(int32_t *)(arena<uint8_t>(0x100) + 0x10) = n;
                        g_bsm_key = (r.next() & 1) ? keys[r.range(0, n - 1)] : (int32_t)r.range(-8, 40); // hit or miss
                    },
                    [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), g_bsm_key)}; }};
    // Object::Compact4: compact arrays [0]/[4] dropping zero-valued [0] entries, rebuild [8]/[0xc] via scratch global G.
    static uint32_t g_c4_g;
    Test compact4{0x40b860,
                  [=](Rng &r) {
                      fresh(r);
                      int n = r.range(1, 8);
                      uint32_t *a0 = arena<uint32_t>(0x300), *a4 = arena<uint32_t>(0x380), *a8 = arena<uint32_t>(0x400), *ac = arena<uint32_t>(0x480);
                      for (int i = 0; i < n; i++) { a0[i] = (r.next() % 3 == 0) ? 0 : (r.next() | 1); a4[i] = r.next(); a8[i] = 0; ac[i] = r.range(0, n - 1); }
                      *(void **)(arena<uint8_t>(0x100) + 0) = a0;
                      *(void **)(arena<uint8_t>(0x100) + 4) = a4;
                      *(void **)(arena<uint8_t>(0x100) + 8) = a8;
                      *(void **)(arena<uint8_t>(0x100) + 0xc) = ac;
                      *(int32_t *)(arena<uint8_t>(0x100) + 0x10) = n;
                      for (int i = 0; i < 16; i++) ((uint32_t *)(uintptr_t)g_c4_g)[i] = 0; // clear scratch remap
                  },
                  [](void *fn) { ((void(__thiscall *)(void *))fn)(arena<void>(0x100)); return Result{(uint32_t)*(int32_t *)(arena<uint8_t>(0x100) + 0x10)}; }};
    compact4.prepare = [](const rewrite::Entry *e) { g_c4_g = rewrite::start_constants(e->at)[1]; };
    // Object::StructCopy44: copy 0x44 bytes arg->this; return this. No stubs.
    Test structcopy44{0x46c480,
                      [=](Rng &r) { fresh(r); for (int i = 0; i < 0x48; i++) arena<uint8_t>(0x300)[i] = (uint8_t)r.next(); },
                      [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x300))}; }};
    // Object::GridInsert: grid level grow/reshape. H1/H2/H3 stubbed; exercise early-return, flag!=0, flag==0 paths.
    static uint32_t g_gi_h1, g_gi_h2, g_gi_h3, g_gi_flag;
    Test gridinsert{0x851e80,
                    [=](Rng &r) {
                        fresh(r);
                        *(void **)(arena<uint8_t>(0x100) + 0) = arena<void>(0x200);
                        *(int32_t *)(arena<uint8_t>(0x100) + 4) = r.range(0, 4);
                        *(void **)(arena<uint8_t>(0x100) + 8) = arena<void>(0x400);
                        *(int32_t *)(arena<uint8_t>(0x100) + 0xc) = r.range(-1, 3);
                        *(int32_t *)(arena<uint8_t>(0x100) + 0x10) = r.range(0, 6);
                        g_gi_flag = r.next() & 1;
                    },
                    [](void *fn) {
                        ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), g_gi_flag);
                        return Result{*(uint32_t *)(arena<uint8_t>(0x100) + 0xc) ^ (*(uint32_t *)(arena<uint8_t>(0x100) + 0x10) << 8) ^ (uint32_t)(uintptr_t)*(void **)(arena<uint8_t>(0x100) + 8)};
                    }};
    gridinsert.prepare = [](const rewrite::Entry *e) { const uint32_t *c = rewrite::start_constants(e->at); g_gi_h1 = c[1]; g_gi_h2 = c[2]; g_gi_h3 = c[3]; stubs_for_now({{g_gi_h1, (void *)stub_grid_h1}, {g_gi_h2, (void *)stub_grid_h2}, {g_gi_h3, (void *)stub_grid_h3}}); };
    gridinsert.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Object::GridRemove: grid level shrink; reuses the grid-helper stubs. Exercise count<=1, flag!=0, flag==0 paths.
    static uint32_t g_grm_h1, g_grm_h2, g_grm_h3, g_grm_flag;
    Test gridremove{0x851f70,
                    [=](Rng &r) {
                        fresh(r);
                        *(void **)(arena<uint8_t>(0x100) + 0) = arena<void>(0x200);
                        *(int32_t *)(arena<uint8_t>(0x100) + 4) = r.range(0, 4);
                        *(void **)(arena<uint8_t>(0x100) + 8) = arena<void>(0x400);
                        *(int32_t *)(arena<uint8_t>(0x100) + 0xc) = r.range(-1, 4);
                        *(int32_t *)(arena<uint8_t>(0x100) + 0x10) = r.range(0, 8);
                        g_grm_flag = r.next() & 1;
                    },
                    [](void *fn) {
                        ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), g_grm_flag);
                        return Result{*(uint32_t *)(arena<uint8_t>(0x100) + 0xc) ^ (*(uint32_t *)(arena<uint8_t>(0x100) + 0x10) << 8) ^ (uint32_t)(uintptr_t)*(void **)(arena<uint8_t>(0x100) + 8)};
                    }};
    gridremove.prepare = [](const rewrite::Entry *e) { const uint32_t *c = rewrite::start_constants(e->at); g_grm_h1 = c[1]; g_grm_h2 = c[2]; g_grm_h3 = c[3]; stubs_for_now({{g_grm_h1, (void *)stub_grid_h1}, {g_grm_h2, (void *)stub_grid_h2}, {g_grm_h3, (void *)stub_grid_h3}}); };
    gridremove.finish = [](const rewrite::Entry *) { unstub_many(); };
    // Object::SizeSum: total=8; for each row, for each item this->[0xc][i]: total+=item->vtbl[2](rowdata). vtbl[2]=stub_size_item(->0x10).
    Test sizesum{0x533590,
                 [=](Rng &r) {
                     fresh(r);
                     static void *itemvt[3] = {nullptr, nullptr, (void *)stub_size_item};
                     int inner = r.range(0, 4), outer = r.range(0, 4);
                     void **items = arena<void *>(0x600);
                     for (int i = 0; i < 4; i++) { *(void **)arena<uint8_t>(0x700 + i * 0x40) = itemvt; items[i] = arena<void>(0x700 + i * 0x40); }
                     *(uint32_t *)(arena<uint8_t>(0x100) + 0x28) = (uint32_t)(uintptr_t)arena<void>(0x300); // base; arg=0 -> record=0x300
                     *(uint32_t *)(arena<uint8_t>(0x100) + 0x18) = (uint32_t)inner;
                     *(uint32_t *)(arena<uint8_t>(0x100) + 0xc) = (uint32_t)(uintptr_t)items;
                     *(void **)arena<uint8_t>(0x300) = arena<void>(0x500); // record->[0] = rowdata base
                     *(int32_t *)(arena<uint8_t>(0x300) + 4) = outer;       // record->[4] = outerCount
                 },
                 [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), 0)}; }};
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
                    &dtor_delete2, &dtor5_delete2, &dtor_tables_member, &quadtree_add, &quadtree_insert, &object_construct, &behaviour_create, &pair_release, &member_dtor, &getref, &setfields, &freefield, &tablesbase, &deleting2, &forward2, &countlist, &callthentail, &dbgsingleton, &getfield, &globalmembercall, &condforward, &arraypush, &delmemberfield, &freetwofields, &dbgwrap2, &ctorbasearg, &allocstore, &condcallset, &freetwotable, &gridgrow, &quadcollect, &listdrain, &sizesum, &gridremove, &gridinsert, &structcopy44, &compact4, &bsearchmap, &tablevrel, &tabledtor, &condvcallh, &condvrel, &branchoff, &settabletail, &condnotify, &copyrecords, &vcountproduct, &derefchain, &initedx, &settablevcall, &fwd2clear, &setfieldtail, &vfwd2ret, &sorted_map_ctor, &frustum, &interface_float, &trivial_setcap,
                    &interface_forward}) {
        x->iterations = (x == &deleting || x == &query || x == &module || x == &start_name) ? 4 : 20;   // thousands of copies: fewer cases each
        x->data = false;
        t.push_back(*x);
    }

    // IdMap::FindOrAdd: the map at 0x100, its four arrays at 0x400.. with up to 12 ids, sorted with gaps so that ids
    // land before, between, after and on the ones present; a full map grows (from nothing: 200)
    Test id_map{0x408240,
                [=](Rng &r) {
                    fresh(r);
                    *arena<uint32_t>(ALLOC_NEXT) = 0;
                    int count = r.range(0, 12), id = r.range(-20, 0);
                    uint32_t *m = arena<uint32_t>(0x100);
                    for (int a = 0; a < 4; a++) {
                        m[a] = (uint32_t)(uintptr_t)arena<void>(0x400 + a * 0x100);
                        for (int i = 0; i < 64; i++) arena<uint32_t>(0x400 + a * 0x100)[i] = r.next();
                    }
                    for (int i = 0; i < count; i++) {
                        arena<int32_t>(0x600)[i] = id;
                        id += r.range(1, 3);
                    }
                    m[4] = (uint32_t)count;
                    m[5] = (uint32_t)(r.range(0, 1) ? count : count + r.range(1, 5));
                    *arena<int32_t>(0x10) = r.range(-24, id + 4);
                },
                [](void *fn) {
                    return Result{((uint32_t(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), *arena<int32_t>(0x10))};
                }};
    id_map.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbcdfd0, (void *)stub_alloc_next}}); };
    id_map.finish = [](const rewrite::Entry *) { unstub_many(); };
    id_map.data = false;
    t.push_back(id_map);
    Test id_map_free{0x408210,
                     [=](Rng &r) { fresh(r); },
                     [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    id_map_free.data = false;
    t.push_back(id_map_free);
    auto quick = [&](Test &x) { x.data = false; t.push_back(x); };
    // List::Insert, before a node of a ring 0x300..0x360 (nodes {next, prev, value}); the new node comes from the
    // arena, and `a` is sometimes that same address
    auto list_ring = [=](Rng &r) {
        fresh(r);
        *arena<uint32_t>(ALLOC_NEXT) = 0;
        for (int i = 0; i < 4; i++) {
            void **n = arena<void *>(0x300 + i * 0x20);
            n[0] = arena<void>(0x300 + (i + 1) % 4 * 0x20);
            n[1] = arena<void>(0x300 + (i + 3) % 4 * 0x20);
            n[2] = (void *)(uintptr_t)r.next();
        }
        arena<uint32_t>(0x10)[0] = r.next();
        arena<uint32_t>(0x10)[1] = r.next();
        arena<uint32_t>(0x10)[2] = r.range(0, 3);
    };
    Test list_insert{0x476830,
                     [=](Rng &r) {
                         list_ring(r);
                         if (r.range(0, 2) == 0) arena<uint32_t>(0x10)[0] = (uint32_t)(uintptr_t)arena<void>(ALLOC_START);
                     },
                     [](void *fn) {
                         uint32_t *a = arena<uint32_t>(0x10);
                         return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *, uint32_t, uint32_t, void *, uint32_t))fn)(
                             arena<void>(0x100), arena<void>(0x40), a[0], a[1], arena<void>(0x300 + a[2] * 0x20), 0x1234)};
                     }};
    list_insert.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbcdfc0, (void *)stub_alloc_next}}); };
    list_insert.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(list_insert);
    // List::Erase, of a node of the ring, the list's own node (self + 0xc is node 0, so self is 0x2f4) or node `b`
    Test list_erase{0x476880,
                    [=](Rng &r) {
                        list_ring(r);
                        arena<uint32_t>(0x10)[1] = (uint32_t)(uintptr_t)arena<void>(0x300 + r.range(0, 3) * 0x20);
                    },
                    [](void *fn) {
                        uint32_t *a = arena<uint32_t>(0x10);
                        return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *, uint32_t, uint32_t, void *))fn)(
                            arena<void>(0x2f4), arena<void>(0x40), a[0], a[1], arena<void>(0x300 + a[2] * 0x20))};
                    }};
    quick(list_erase);
    // IntArray::Assign: from an array of up to 12 at 0x180 (data 0x400) to one at 0x100
    Test int_assign{0x423fb0,
                    [=](Rng &r) {
                        fresh(r);
                        *arena<uint32_t>(ALLOC_NEXT) = 0;
                        arena<void *>(0x100)[0] = arena<void>(0x500);
                        arena<int32_t>(0x100)[1] = r.range(0, 12);
                        arena<void *>(0x180)[0] = arena<void>(0x400);
                        arena<int32_t>(0x180)[1] = r.range(0, 12);
                        for (int i = 0; i < 12; i++) arena<uint32_t>(0x400)[i] = r.next();
                    },
                    [](void *fn) {
                        return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x180))};
                    }};
    int_assign.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbcdfd0, (void *)stub_alloc_next}}); };
    int_assign.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(int_assign);
    // BlockPool::Return: 3 pools of 20 bytes at 0x200, blocks listed at 0x400.., returned lists at 0x500..; 0 is none
    Test pool_return{0x80f990,
                     [=](Rng &r) {
                         fresh(r);
                         arena<void *>(0x100)[0] = arena<void>(0x200);
                         for (int k = 0; k < 3; k++) {
                             int32_t *p = arena<int32_t>(0x200 + 20 * k);
                             int per = r.range(0, 3), blocks = r.range(0, 3);
                             p[0] = per; p[2] = blocks;
                             p[1] = (int32_t)(uintptr_t)arena<void>(0x400 + k * 0x20);
                             p[3] = (int32_t)(uintptr_t)arena<void>(0x500 + k * 0x40);
                             p[4] = per * blocks ? r.range(0, per * blocks - 1) : 0;
                             for (int i = 0; i < 3; i++) arena<uint32_t>(0x400 + k * 0x20)[i] = r.next();
                         }
                         arena<int32_t>(0x10)[0] = r.range(0, 2);
                         arena<uint32_t>(0x10)[1] = r.next();
                     },
                     [](void *fn) {
                         ((void(__thiscall *)(void *, int32_t, uint32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<uint32_t>(0x10)[1]);
                         return Result{};
                     }};
    quick(pool_return);
    // SharedSlots::Set: 4 slots at 0x108; defaults table 0x180 (some null when the value is null); shared values at
    // 0x5a0.. with low reference counts; the value is null, or of the default's kind or another
    Test shared_set{0xc0d780,
                    [=](Rng &r) {
                        fresh(r);
                        uint32_t *self = arena<uint32_t>(0x100);
                        self[1] = (uint32_t)(uintptr_t)arena<void>(0x180);
                        uint16_t *v[5];
                        for (int i = 0; i < 5; i++) {
                            v[i] = arena<uint16_t>(0x5a0 + i * 0x10);
                            v[i][0] = (uint16_t)r.range(1, 2);
                            v[i][1] = (uint16_t)r.range(1, 3);
                        }
                        int value = r.range(0, 2), index = r.range(0, 3);
                        for (int i = 0; i < 4; i++) {
                            arena<void *>(0x180)[i] = (value == 0 && r.range(0, 3) == 0) ? nullptr : (void *)v[r.range(0, 1)];
                            self[2 + i] = r.range(0, 3) ? (uint32_t)(uintptr_t)v[r.range(2, 3)] : 0;
                        }
                        arena<uint32_t>(0x10)[0] = (uint32_t)index;
                        arena<void *>(0x10)[1] = value ? (void *)v[4] : nullptr;
                        *(uint32_t *)0x13e3318 = (uint32_t)(uintptr_t)arena<void>(0x5f0);
                    },
                    [](void *fn) {
                        ((void(__thiscall *)(void *, uint32_t, void *))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], arena<void *>(0x10)[1]);
                        return Result{};
                    }};
    shared_set.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xc09bf0, (void *)stub_plain_dtor}, {0xca1f80, (void *)stub_shared_free}, {0xc0cff0, (void *)stub_shared_convert}});
    };
    shared_set.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(shared_set);
    // Rows::Load: self at 0x100 (record 0x1a0 with +0x28 = 0x300, the store `offset` past it), up to 3 field loaders;
    // the stream at 0x700 holds {size, count} in its buffer, or runs out so the stream reads them
    Test rows_load{0x44f150,
                   [=](Rng &r) {
                       fresh(r);
                       static void *vt[2] = {nullptr, (void *)stub_row_field};
                       uint8_t *self = arena<uint8_t>(0x100);
                       int offset = r.range(0, 0x40);
                       *(void **)(self + 4) = arena<void>(0x1a0);
                       *(uint32_t *)arena<uint8_t>(0x1a0 + 0x28) = (uint32_t)(uintptr_t)arena<void>(0x300);
                       *(int32_t *)(self + 0x10) = r.range(0, 3);
                       *(void **)(self + 0xc) = arena<void>(0x3c0);
                       for (int i = 0; i < 4; i++) {
                           arena<void *>(0x3c0)[i] = arena<void>(0x400 + i * 0x10);
                           arena<void *>(0x400 + i * 0x10)[0] = vt;
                       }
                       *(void **)arena<uint8_t>(0x300 + offset) = arena<void>(0x500);
                       uint8_t *s = arena<uint8_t>(0x700);
                       int start = r.range(0, 8), inside = r.range(0, 3);
                       *(int32_t *)(s + 4) = start;
                       *(int32_t *)(s + 8) = start;
                       *(int32_t *)(s + 0xc) = start + (inside ? 8 : r.range(0, 7));
                       *(int32_t *)(s + 0x14) = (int32_t)r.next();
                       *(int32_t *)(s + 0x18) = r.range(-1, 4);
                       arena<int32_t>(0x10)[0] = offset;
                       arena<uint32_t>(0x10)[1] = r.next();
                   },
                   [](void *fn) {
                       ((void(__thiscall *)(void *, int32_t, void *, uint32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<void>(0x700), arena<uint32_t>(0x10)[1]);
                       return Result{};
                   }};
    rows_load.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{(uintptr_t)(e->at + 0x84 + 5 + *(int32_t *)(e->at + 0x85)), (void *)stub_rows_resize}});
    };
    rows_load.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(rows_load);
    // cached interfaces: a handle at 0x340 whose object at 0x360 has a class (0x200) with entries for ids 1 and 2
    // (offsets 0 and 8); the id's global decides, and sometimes there is no object or no class
    static uint32_t *g_iface_id;
    auto iface_handle = [=](Rng &r) {
        static void *vt[2] = {(void *)stub_deleting, (void *)stub_class_of};
        arena<void *>(0x360)[0] = vt;
        arena<void *>(0x368)[0] = vt;
        *(void **)arena<uint8_t>(0x340 + 0xc) = r.range(0, 4) ? arena<void>(0x360) : nullptr;
        uint8_t *cls = arena<uint8_t>(0x200);
        *arena<void *>(0x1f0) = r.range(0, 4) ? cls : nullptr;
        *(void **)(cls + 0x14) = arena<void>(0x240);
        *(int32_t *)(cls + 0x18) = 3;
        arena<void *>(0x240)[0] = nullptr;
        arena<void *>(0x240)[1] = arena<void>(0x260);
        arena<void *>(0x240)[2] = arena<void>(0x280);
        *(int32_t *)arena<uint8_t>(0x260 + 0x10) = 0;
        *(int32_t *)arena<uint8_t>(0x280 + 0x10) = 8;
        *g_iface_id = (uint32_t)r.range(0, 3);
    };
    // List::DrainOwners: head at 0x400, 0-3 nodes at 0x420.., then the end node (next null); records at 0x500..
    Test list_drain{0x551390,
                    [=](Rng &r) {
                        fresh(r);
                        iface_handle(r);
                        static void *vt[1] = {(void *)stub_deleting};
                        arena<void *>(0x3a0)[0] = vt;
                        int n = r.range(0, 3);
                        void **prev = arena<void *>(0x400);
                        for (int i = 0; i < n; i++) {
                            void **node = arena<void *>(0x420 + i * 0x20);
                            prev[0] = node; node[1] = prev; prev = node;
                            uint8_t *rec = arena<uint8_t>(0x500 + i * 0x40);
                            node[2] = rec;
                            *(void **)(rec + 4) = arena<void>(0x340);
                            bool kept = r.range(0, 2) == 0;
                            rec[0x18] = kept;
                            *(void **)(rec + 0x14) = kept && r.range(0, 1) ? arena<void>(0x3a0) : nullptr;
                        }
                        void **end = arena<void *>(0x4e0);
                        prev[0] = end; end[0] = nullptr; end[1] = prev;
                    },
                    [](void *fn) {
                        ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x400));
                        return Result{};
                    }};
    list_drain.prepare = [](const rewrite::Entry *e) {
        g_iface_id = *(uint32_t **)(e->at + 0x6a);
        stubs_for_now({{(uintptr_t)(e->at + 0xa3 + 5 + *(int32_t *)(e->at + 0xa4)), (void *)stub_plain_dtor}});
    };
    list_drain.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(list_drain);
    // Notify::Pass: self at 0x800, with the interface sometimes already kept and each listener sometimes there
    Test notify{0x5c0a30,
                [=](Rng &r) {
                    fresh(r);
                    iface_handle(r);
                    static void *part_vt[5] = {nullptr, (void *)stub_thiscall2, (void *)stub_thiscall2, (void *)stub_thiscall2, (void *)stub_thiscall2};
                    arena<void *>(0x360 + 0x40)[0] = part_vt;
                    arena<void *>(0x368 + 0x40)[0] = part_vt;
                    uint8_t *self = arena<uint8_t>(0x800);
                    *(void **)(self + 0x160) = arena<void>(0x340);
                    bool kept = r.range(0, 2) == 0;
                    self[0x174] = kept;
                    *(void **)(self + 0x170) = kept && r.range(0, 1) ? arena<void>(0x360) : nullptr;
                    *(void **)(self + 0x2b4) = r.range(0, 1) ? arena<void>(0x3e0) : nullptr;
                    *(void **)(self + 0x2b8) = r.range(0, 1) ? arena<void>(0x3f0) : nullptr;
                    arena<uint32_t>(0x10)[0] = r.next();
                    arena<uint32_t>(0x10)[1] = r.next();
                },
                [](void *fn) {
                    ((void(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x10)[0], arena<uint32_t>(0x10)[1]);
                    return Result{};
                }};
    notify.prepare = [](const rewrite::Entry *e) {
        g_iface_id = (uint32_t *)0x12f561c;
        stubs_for_now({{(uintptr_t)(e->at + 0x10 + 5 + *(int32_t *)(e->at + 0x11)), (void *)stub_thiscall2},
                       {(uintptr_t)(e->at + 0xd6 + 5 + *(int32_t *)(e->at + 0xd7)), (void *)stub_thiscall2}});
    };
    notify.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(notify);
    // ObjectRef::AssignExtra: as ObjectRef::Assign
    Test assign_extra{0x6e75c0, handles, [](void *fn) {
                          return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *, void *, void *))fn)(
                              arena<void>(0x100), nullptr, arena<void>(0x180))};
                      }};
    quick(assign_extra);
    // RefArray::Remove: up to 5 references of 20 bytes at 0x800, each the only one in its handle's list (0xa00..); the
    // object to remove is one of theirs or nobody's
    Test ref_remove{0x731880,
                    [=](Rng &r) {
                        fresh(r);
                        static void *vt[1] = {(void *)stub_virtual0};
                        int n = r.range(0, 5);
                        arena<void *>(0x100)[0] = arena<void>(0x800);
                        arena<int32_t>(0x100)[2] = n;
                        for (int i = 0; i < 5; i++) {
                            uint32_t h = 0xa00 + i * 0x80, ref = 0x800 + i * 20;
                            uint8_t *hp = arena<uint8_t>(h);
                            *(int32_t *)(hp + 8) = r.range(1, 5);
                            *(uint32_t *)(hp + 0xc) = 0x1000 + (uint32_t)r.range(0, 4);
                            void **head = (void **)(hp + 0x40), **node = arena<void *>(ref + 8);
                            *(void ***)(hp + 0x20) = head;
                            node[0] = head; node[1] = head; head[0] = node; head[1] = node;
                            arena<void *>(ref)[0] = vt;
                            arena<void *>(ref)[1] = hp;
                        }
                        arena<uint32_t>(0x10)[0] = 0x1000 + (uint32_t)r.range(0, 5);
                    },
                    [](void *fn) {
                        ((void(__fastcall *)(void *, void *, uint32_t))fn)(arena<void>(0x100), nullptr, arena<uint32_t>(0x10)[0]);
                        return Result{};
                    }};
    quick(ref_remove);
    // Rows::Save: self at 0x100 (vtable slot 2 gives the size; +0x28 is the store's base), up to 3 savers; the stream at
    // 0x700 has a running checksum; the rows are at 0x500
    Test rows_save{0x533460,
                   [=](Rng &r) {
                       fresh(r);
                       static void *vt[3] = {nullptr, nullptr, (void *)stub_rows_size};
                       static void *field_vt[4] = {nullptr, nullptr, nullptr, (void *)stub_row_save};
                       uint8_t *self = arena<uint8_t>(0x100);
                       int offset = r.range(0, 0x40);
                       *(void **)self = vt;
                       *(uint32_t *)(self + 0x28) = (uint32_t)(uintptr_t)arena<void>(0x300);
                       *(int32_t *)(self + 0x18) = r.range(0, 3);
                       *(void **)(self + 0xc) = arena<void>(0x3c0);
                       for (int i = 0; i < 4; i++) {
                           arena<void *>(0x3c0)[i] = arena<void>(0x400 + i * 0x10);
                           arena<void *>(0x400 + i * 0x10)[0] = field_vt;
                       }
                       *(void **)arena<uint8_t>(0x300 + offset) = arena<void>(0x500);
                       *(int32_t *)arena<uint8_t>(0x300 + offset + 4) = r.range(-1, 4);
                       *(uint32_t *)arena<uint8_t>(0x710) = r.next();
                       *(uint32_t *)arena<uint8_t>(0x714) = r.next();
                       arena<int32_t>(0x10)[0] = offset;
                       arena<int32_t>(0x10)[1] = r.range(4, 0x10000);
                   },
                   [](void *fn) {
                       ((void(__thiscall *)(void *, int32_t, void *))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<void>(0x700));
                       return Result{};
                   }};
    rows_save.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbe4060, (void *)stub_stream_write}}); };
    rows_save.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(rows_save);
    // Items::AnyMatch: up to 4 items at 0x3c0; the interface kept or found; the check passes after 0-5 calls
    Test any_match{0x5d68e0,
                   [=](Rng &r) {
                       fresh(r);
                       iface_handle(r);
                       uint8_t *self = arena<uint8_t>(0x800);
                       *(void **)(self + 0x54) = arena<void>(0x340);
                       bool kept = r.range(0, 1);
                       self[0x68] = kept;
                       *(void **)(self + 0x64) = kept ? arena<void>(0x360) : nullptr;
                       *(int32_t *)(self + 0xe8) = r.range(0, 4);
                       *(void **)(self + 0xe4) = arena<void>(0x3c0);
                       for (int i = 0; i < 4; i++) arena<uint32_t>(0x3c0)[i] = r.next();
                       arena<int32_t>(0x20)[0] = r.range(0, 5);
                       arena<uint32_t>(0x10)[0] = r.next();
                   },
                   [](void *fn) {
                       return Result{((uint8_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x10)[0])};
                   }};
    any_match.prepare = [](const rewrite::Entry *e) {
        g_iface_id = (uint32_t *)0x12f54e0;
        stubs_for_now({{0x59acc0, (void *)stub_item_table}, {0x5a7300, (void *)stub_item_find},
                       {(uintptr_t)(e->at + 0x73 + 5 + *(int32_t *)(e->at + 0x74)), (void *)stub_item_check}});
    };
    any_match.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(any_match);
    // Items::EachInterface: up to 3 references of 28 bytes at 0x900, kept or found through the handle
    Test each_interface{0x563420,
                        [=](Rng &r) {
                            fresh(r);
                            iface_handle(r);
                            uint8_t *self = arena<uint8_t>(0x800);
                            int n = r.range(0, 3);
                            *(int32_t *)(self + 0xc4) = n;
                            *(void **)(self + 0xc0) = arena<void>(0x900);
                            for (int i = 0; i < 3; i++) {
                                uint8_t *ref = arena<uint8_t>(0x900 + i * 0x1c);
                                *(void **)(ref + 4) = arena<void>(0x340);
                                bool kept = r.range(0, 1);
                                ref[0x18] = kept;
                                *(void **)(ref + 0x14) = kept ? arena<void>(0x3a0) : nullptr;
                            }
                            arena<uint32_t>(0x10)[0] = r.next();
                            arena<uint32_t>(0x10)[1] = r.next();
                        },
                        [](void *fn) {
                            ((void(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x10)[0], arena<uint32_t>(0x10)[1]);
                            return Result{};
                        }};
    each_interface.prepare = [](const rewrite::Entry *e) {
        g_iface_id = (uint32_t *)0x12f50d4;
        stubs_for_now({{(uintptr_t)(e->at + 0x7b + 5 + *(int32_t *)(e->at + 0x7c)), (void *)stub_thiscall2}});
    };
    each_interface.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(each_interface);
    // Compare::Mapped: two objects at 0x100 and 0x300; the lookup (stubbed) changes the value by its step, and fails
    // for some kinds
    Test mapped{0x71d1e0,
                [=](Rng &r) {
                    fresh(r);
                    uint8_t *a = arena<uint8_t>(0x100), *b = arena<uint8_t>(0x300);
                    *(int32_t *)(a + 0xa4) = r.range(0, 3);
                    *(int32_t *)(b + 0xa4) = r.range(0, 3);
                    a[0x8d] = (uint8_t)r.range(0, 4);
                    b[0x8d] = (uint8_t)r.range(0, 4);
                    arena<uint32_t>(0x10)[0] = (uint32_t)r.range(0, 1) | 0xabcd00;
                },
                [](void *fn) {
                    return Result{((uint8_t(__stdcall *)(void *, void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x300), arena<uint32_t>(0x10)[0])};
                }};
    mapped.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x71b0f0, (void *)stub_mapped_lookup}}); };
    mapped.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(mapped);
    // List::FindNamed: a list of up to 3 items at +OFF; the name is one of theirs, another, empty or null
    static uint32_t g_named_off;
    Test find_named{0xc0ccf0,
                    [=](Rng &r) {
                        fresh(r);
                        uint8_t *self = arena<uint8_t>(0x800);
                        int n = r.range(0, 3);
                        *(void **)(self + g_named_off) = arena<void>(0x400);
                        for (int i = 0; i <= n; i++) {
                            void **node = arena<void *>(0x400 + i * 0x20);
                            node[0] = i < n ? arena<void>(0x420 + i * 0x20) : nullptr;
                            node[2] = arena<void>(0x500 + i * 0x10);
                            arena<void *>(0x500 + i * 0x10)[1] = arena<void>(0x600 + i * 8);
                            *arena<char>(0x600 + i * 8) = 'a';
                        }
                        *arena<char>(0x640) = 0;
                        int pick = r.range(0, 5);
                        arena<void *>(0x10)[0] = pick == 5 ? nullptr : pick == 4 ? arena<void>(0x640) : arena<void>(0x600 + pick * 8);
                    },
                    [](void *fn) {
                        return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x800), arena<void *>(0x10)[0])};
                    }};
    find_named.prepare = [](const rewrite::Entry *e) {
        g_named_off = *(uint32_t *)(e->at + 0x11);
        stubs_for_now({{0xde22df, (void *)stub_same_pointer}});
    };
    find_named.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(find_named);
    // List::LeaveIfLinked: a node between two neighbours, each link sometimes missing or wrong
    Test leave_linked{0xc0dd30,
                      [=](Rng &r) {
                          fresh(r);
                          void **n = arena<void *>(0x300), **next = arena<void *>(0x320), **prev = arena<void *>(0x340);
                          int broken = r.range(0, 7);
                          n[0] = broken == 1 ? nullptr : (void *)next;
                          n[1] = broken == 2 ? nullptr : (void *)prev;
                          next[1] = broken == 3 ? (void *)prev : (void *)n;
                          prev[0] = broken == 4 ? (void *)next : (void *)n;
                      },
                      [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x300)); return Result{}; }};
    leave_linked.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{(uintptr_t)(e->at + 0x1f + 5 + *(int32_t *)(e->at + 0x20)), (void *)stub_plain_dtor}});
    };
    leave_linked.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(leave_linked);
    // Shared::Delete: with and without the flag
    Test shared_delete{0xc0a5b0,
                       [=](Rng &r) {
                           fresh(r);
                           *(uint32_t *)0x13e3318 = (uint32_t)(uintptr_t)arena<void>(0x5f0);
                           arena<uint32_t>(0x10)[0] = (uint32_t)r.range(0, 3);
                       },
                       [](void *fn) {
                           return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0])};
                       }};
    shared_delete.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{(uintptr_t)(e->at + 3 + 5 + *(int32_t *)(e->at + 4)), (void *)stub_plain_dtor}, {0xca1f80, (void *)stub_shared_free}});
    };
    shared_delete.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(shared_delete);
    // Array<32>::SetCapacity: as the other sizes
    Test setcap32{0x446710, array, [](void *fn) {
                      return Result{((uint32_t(__fastcall *)(void *, void *, int32_t))fn)(arena<void>(0x100), nullptr, *arena<int32_t>(0))};
                  }};
    quick(setcap32);
    // ObjectRef::SetMember: the reference `off` into self sits at 0x100, and the object's handle is 0x380
    static uint32_t g_member_off;
    Test ref_member{0x6dfa50, handles, [](void *fn) {
                        ((void(__fastcall *)(void *, void *, void *))fn)(arena<uint8_t>(0x100) - g_member_off, nullptr, arena<void>(0x500));
                        return Result{};
                    }};
    ref_member.prepare = [](const rewrite::Entry *e) {
        g_member_off = *(uint32_t *)(e->at + 4);
        stubs_for_now({{0x805700, (void *)stub_handle_of}});
    };
    ref_member.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(ref_member);
    // RefArray::Push: 6 references at 0x800, a full array asks for more room first
    Test ref_push{0x732270,
                  [=](Rng &r) {
                      handles(r);
                      static void *vt[1] = {(void *)stub_virtual0};
                      int n = r.range(0, 5);
                      arena<void *>(0x100)[0] = arena<void>(0x800);
                      arena<int32_t>(0x100)[1] = r.range(0, 1) ? n : n + 1;
                      arena<int32_t>(0x100)[2] = n;
                      for (int i = 0; i < 6; i++) {
                          uint32_t ref = 0x800 + i * 20;
                          uint8_t *hp = arena<uint8_t>(0xa00 + i * 0x80);
                          *(int32_t *)(hp + 8) = r.range(1, 5);
                          void **head = (void **)(hp + 0x40), **node = arena<void *>(ref + 8);
                          *(void ***)(hp + 0x20) = head;
                          node[0] = head; node[1] = head; head[0] = node; head[1] = node;
                          arena<void *>(ref)[0] = vt;
                          arena<void *>(ref)[1] = hp;
                      }
                  },
                  [](void *fn) {
                      ((void(__fastcall *)(void *, void *, void *))fn)(arena<void>(0x100), nullptr, arena<void>(0x500));
                      return Result{};
                  }};
    ref_push.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x805700, (void *)stub_handle_of}, {0x5501e0, (void *)stub_reserve}}); };
    ref_push.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(ref_push);
    // Items::ByIndex: self at 0x800 with a class count of 3 (stub); an index below, in or past the range
    auto by_index_setup = [=](Rng &r) {
        fresh(r);
        iface_handle(r);
        static void *vt[0x4f];
        vt[0x4e] = (void *)stub_item_count;
        uint8_t *self = arena<uint8_t>(0x800);
        *(void **)self = vt;
        *(void **)(self + 0x208) = arena<void>(0x340);
        // the interface always exists in these (the game reads through it without a check)
        *(void **)arena<uint8_t>(0x34c) = arena<void>(0x360);
        *arena<void *>(0x1f0) = arena<void>(0x200);
        *g_iface_id = 1;
        bool kept = r.range(0, 1);
        self[0x21c] = kept;
        *(void **)(self + 0x218) = kept ? arena<void>(0x360) : nullptr;
        *(void **)arena<uint8_t>(0x360 + 0x40) = arena<void>(0x1000);
        arena<int32_t>(0x10)[0] = r.range(-2, 4);
    };
    auto by_index_call = [](void *fn) {
        return Result{((uint32_t(__thiscall *)(void *, int32_t))fn)(arena<void>(0x800), arena<int32_t>(0x10)[0])};
    };
    Test by_index{0x531a80, [=](Rng &r) { by_index_setup(r); }, by_index_call};
    by_index.prepare = [](const rewrite::Entry *e) {
        g_iface_id = (uint32_t *)0x12f5338;
        stubs_for_now({{(uintptr_t)(e->at + 0x7a + 5 + *(int32_t *)(e->at + 0x7b)), (void *)stub_item_value}});
    };
    by_index.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(by_index);
    // the unchecked copies pass a negative index on (the items' getter is a stand-in, so nothing is read there)
    Test by_index_unchecked{0x531580, [=](Rng &r) { by_index_setup(r); }, by_index_call};
    by_index_unchecked.prepare = [](const rewrite::Entry *e) {
        g_iface_id = (uint32_t *)0x12f5338;
        stubs_for_now({{(uintptr_t)(e->at + 0x7d + 5 + *(int32_t *)(e->at + 0x7e)), (void *)stub_item_value}});
    };
    by_index_unchecked.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(by_index_unchecked);
    // Names::Find: a sorted table of up to 8 names (two letters, some bytes above 0x7f, some prefixes of others); the
    // name is one of them or near one
    Test names_find{0x7fb900,
                    [=](Rng &r) {
                        fresh(r);
                        static const char *const pool[] = {"\xc1", "\xc1z", "\xf0q", "a", "ab", "abc", "b", "ba", "c", "cc", "z"};
                        int n = r.range(0, 8), at = r.range(0, 10 - n);
                        for (int i = 0; i < 8; i++) {
                            char *s = arena<char>(0x400 + i * 8);
                            memset(s, 0, 8);
                            strcpy(s, pool[at + (i < n ? i : 0)]);
                            arena<void *>(0x300)[i] = s;
                        }
                        char *key = arena<char>(0x480);
                        memset(key, 0, 8);
                        strcpy(key, pool[r.range(0, 10)]);
                        if (r.range(0, 3) == 0) { key[strlen(key)] = 'm'; }
                        arena<void *>(0x4a0)[0] = key;
                        arena<int32_t>(0x10)[0] = n;
                        arena<int32_t>(0x20)[0] = -1;
                    },
                    [](void *fn) {
                        return Result{((uint8_t(__fastcall *)(void *, void *, int32_t, void *, uint32_t))fn)(
                            arena<void>(0x4a0), arena<void>(0x300), arena<int32_t>(0x10)[0], arena<void>(0x20), 0)};
                    }};
    quick(names_find);
    // Chained::Destroy: self is the 1st-4th of a chain from +0x3c of the owner at 0x300
    Test chained{0xca3810,
                 [=](Rng &r) {
                     fresh(r);
                     int n = r.range(1, 4), who = r.range(0, n - 1);
                     uint32_t *owner = arena<uint32_t>(0x300);
                     uint32_t *link = owner + 0x3c / 4;
                     for (int i = 0; i < n; i++) {
                         uint32_t *x = arena<uint32_t>(0x400 + i * 0x20);
                         *link = (uint32_t)(uintptr_t)x;
                         link = x + 5;
                         x[4] = (uint32_t)(uintptr_t)owner;
                     }
                     *link = 0;
                     arena<int32_t>(0x10)[0] = who;
                 },
                 [](void *fn) {
                     ((void(__fastcall *)(void *))fn)(arena<void>(0x400 + arena<int32_t>(0x10)[0] * 0x20));
                     return Result{};
                 }};
    quick(chained);
    // Words::Reset: to 0 or some words
    Test words_reset{0xc479c0,
                     [=](Rng &r) {
                         fresh(r);
                         *arena<uint32_t>(ALLOC_NEXT) = 0;
                         arena<uint32_t>(0x10)[0] = r.range(0, 1) ? (uint32_t)r.range(1, 9) : 0;
                     },
                     [](void *fn) {
                         return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0])};
                     }};
    words_reset.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbcdfd0, (void *)stub_alloc_next}}); };
    words_reset.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(words_reset);
    // Words::Resize: from up to 8 words to 0-10 (sometimes the same)
    Test words_resize{0x665780,
                      [=](Rng &r) {
                          fresh(r);
                          *arena<uint32_t>(ALLOC_NEXT) = 0;
                          int cap = r.range(0, 8);
                          arena<void *>(0x100)[0] = arena<void>(0x400);
                          arena<int32_t>(0x100)[1] = cap;
                          for (int i = 0; i < 8; i++) arena<uint32_t>(0x400)[i] = r.next();
                          arena<int32_t>(0x10)[0] = r.range(0, 3) ? r.range(0, 10) : cap;
                      },
                      [](void *fn) {
                          ((void(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0]);
                          return Result{};
                      }};
    words_resize.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbcdfd0, (void *)stub_alloc_next}}); };
    words_resize.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(words_resize);
    // Slots::Free: up to 8 slots at 0x400, some in use; the one freed is the last or another
    Test slots_free{0x42f6f0,
                    [=](Rng &r) {
                        fresh(r);
                        uint8_t *self = arena<uint8_t>(0x100);
                        int end = r.range(1, 8);
                        *(void **)(self + 0x18) = arena<void>(0x400);
                        *(int32_t *)(self + 0x10) = end;
                        *(int32_t *)(self + 0xc) = r.range(0, end);
                        for (int i = 0; i < 8; i++) *(uint32_t *)arena<uint8_t>(0x400 + i * 32 + 0x1c) = (uint32_t)r.range(0, 1);
                        arena<int32_t>(0x10)[0] = r.range(0, 1) ? end - 1 : r.range(0, end - 1);
                    },
                    [](void *fn) {
                        ((void(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0]);
                        return Result{};
                    }};
    quick(slots_free);
    // Prime::After: small numbers, and the wrap of the largest
    Test prime{0xbd6620,
               [=](Rng &r) { fresh(r); arena<uint32_t>(0x10)[0] = r.range(0, 9) ? (uint32_t)r.range(0, 3000) : 0xffffffffu; },
               [](void *fn) { return Result{((uint32_t(__fastcall *)(uint32_t))fn)(arena<uint32_t>(0x10)[0])}; }};
    quick(prime);
    // Array<1>::SetCapacity: as the other sizes
    Test setcap1{0x5b9b20, array, [](void *fn) {
                     return Result{((uint32_t(__fastcall *)(void *, void *, int32_t))fn)(arena<void>(0x100), nullptr, *arena<int32_t>(0))};
                 }};
    quick(setcap1);
    // ObjectRef::AssignWide: as ObjectRef::Assign, with a whole word after the extra
    Test assign_wide{0x749cf0, handles, [](void *fn) {
                         return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *, void *, void *))fn)(
                             arena<void>(0x100), nullptr, arena<void>(0x180))};
                     }};
    quick(assign_wide);
    // Names::Add: parallel arrays at 0x300 and 0x380; the find (stubbed, with the real one's stack clean-up) picks the place
    Test names_add{0x7fbe30,
                   [=](Rng &r) {
                       fresh(r);
                       uint8_t *self = arena<uint8_t>(0x800);
                       int count = r.range(0, 6);
                       *(void **)self = arena<void>(0x300);
                       *(void **)(self + 8) = arena<void>(0x380);
                       *(int32_t *)(self + 4) = r.range(0, 1) ? count : count + 2;
                       *(int32_t *)(self + 0x10) = r.range(1, 4);
                       *(int32_t *)(self + 0x14) = count;
                       for (int i = 0; i < 8; i++) { arena<uint32_t>(0x300)[i] = r.next(); arena<uint32_t>(0x380)[i] = r.next(); }
                       arena<uint32_t>(0x400)[0] = r.next();
                       arena<uint32_t>(0x410)[0] = r.next();
                       arena<int32_t>(0x20)[0] = r.range(0, count);
                   },
                   [](void *fn) {
                       ((void(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x800), arena<void>(0x400), arena<void>(0x410));
                       return Result{};
                   }};
    names_add.prepare = [](const rewrite::Entry *e) {
        auto target = [&](uint32_t at) { return (uintptr_t)(e->at + at + 4 + *(int32_t *)(e->at + at)); };
        stubs_for_now({{target(0x15), (void *)stub_reserve}, {target(0x1e), (void *)stub_reserve}, {target(0x39), (void *)stub_name_place}});
    };
    names_add.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(names_add);
    // Linked::Delete: linked between two neighbours, with and without the flag
    Test linked_delete{0x453ed0,
                       [=](Rng &r) {
                           fresh(r);
                           void **n = arena<void *>(0x104), **next = arena<void *>(0x320), **prev = arena<void *>(0x340);
                           n[0] = next; n[1] = prev; next[1] = n; prev[0] = n;
                           arena<uint32_t>(0x10)[0] = (uint32_t)r.range(0, 3);
                       },
                       [](void *fn) {
                           return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0])};
                       }};
    quick(linked_delete);
    // Compare::MappedThen: as Compare::Mapped, handing on to a stand-in
    Test mapped_then{0x71ea70,
                     [=](Rng &r) {
                         fresh(r);
                         uint8_t *a = arena<uint8_t>(0x100), *b = arena<uint8_t>(0x300);
                         a[0x88] = (uint8_t)r.next(); a[0x89] = (uint8_t)r.next();
                         *(int32_t *)(b + 0xa4) = r.range(0, 3);
                         b[0x8d] = (uint8_t)r.range(0, 4);
                         arena<uint32_t>(0x10)[0] = (uint32_t)r.range(0, 1) | 0xabcd00;
                     },
                     [](void *fn) {
                         return Result{((uint8_t(__thiscall *)(void *, void *, void *, uint32_t))fn)(arena<void>(0x500), arena<void>(0x100), arena<void>(0x300), arena<uint32_t>(0x10)[0])};
                     }};
    mapped_then.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x71b0f0, (void *)stub_mapped_lookup}, {0x71d6e0, (void *)stub_mapped_then}}); };
    mapped_then.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(mapped_then);
    // Refs::Visit: up to 3 references, all to the handle at 0x340, shown to a visitor at 0x3b0
    Test refs_visit{0x730b80,
                    [=](Rng &r) {
                        fresh(r);
                        iface_handle(r);
                        static void *vt[1] = {(void *)stub_thiscall1};
                        arena<void *>(0x3b0)[0] = vt;
                        arena<void *>(0x100)[0] = arena<void>(0x800);
                        arena<int32_t>(0x100)[2] = r.range(0, 3);
                        for (int i = 0; i < 3; i++) arena<void *>(0x800 + i * 20)[1] = arena<void>(0x340);
                    },
                    [](void *fn) {
                        ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x3b0));
                        return Result{};
                    }};
    refs_visit.prepare = [](const rewrite::Entry *e) { g_iface_id = *(uint32_t **)(e->at + 0x2b); };
    quick(refs_visit);
    // Cloned::Copy: from a list of up to 3 items; clones come from the arena at 0x600..
    Test cloned{0xc0d940,
                [=](Rng &r) {
                    fresh(r);
                    int n = r.range(0, 3);
                    arena<void *>(0x904 / 4 * 4)[0] = arena<void>(0x400);
                    for (int i = 0; i <= n; i++) {
                        void **node = arena<void *>(0x400 + i * 0x20);
                        node[0] = i < n ? arena<void>(0x420 + i * 0x20) : nullptr;
                        node[2] = (void *)(uintptr_t)r.next();
                    }
                    arena<int32_t>(0x20)[0] = 0;
                },
                [](void *fn) {
                    return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x800), arena<void>(0x900))};
                }};
    cloned.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{(uintptr_t)(e->at + 0x45 + 5 + *(int32_t *)(e->at + 0x46)), (void *)stub_clone}});
    };
    cloned.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(cloned);
    // Checked::ByIndex: a table of 3 entries at 0x400; the count is sometimes too low, so the report is made
    Test checked{0x68fe00,
                 [=](Rng &r) {
                     fresh(r);
                     uint8_t *self = arena<uint8_t>(0x800);
                     *(int32_t *)(self + 0x164) = r.range(0, 3);
                     *(void **)(self + 0x190) = arena<void>(0x400);
                     for (int i = 0; i < 6; i++) arena<uint32_t>(0x400)[i] = r.next();
                     arena<int32_t>(0x10)[0] = r.range(0, 2);
                     arena<uint32_t>(0x10)[1] = r.next();
                 },
                 [](void *fn) {
                     return Result{((uint32_t(__thiscall *)(void *, int32_t, uint32_t))fn)(arena<void>(0x800), arena<int32_t>(0x10)[0], arena<uint32_t>(0x10)[1])};
                 }};
    checked.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{0x68f640, (void *)stub_report}, {(uintptr_t)(e->at + 0x3e + 5 + *(int32_t *)(e->at + 0x3f)), (void *)stub_item_arg}});
    };
    checked.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(checked);
    // Pool::Take: a free list at 0x300 (and the word before it, read when the pool stays full), items at 0x400
    Test pool_take{0x42dda0,
                   [=](Rng &r) {
                       fresh(r);
                       uint32_t *self = arena<uint32_t>(0x100);
                       int cap = r.range(0, 6);
                       self[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                       self[1] = (uint32_t)(uintptr_t)arena<void>(0x300);
                       self[2] = (uint32_t)r.range(0, cap);
                       self[3] = (uint32_t)cap;
                       for (int i = -1; i < 6; i++) arena<int32_t>(0x300)[i] = r.range(0, 3);
                   },
                   [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    pool_take.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{(uintptr_t)(e->at + 0x1b + 5 + *(int32_t *)(e->at + 0x1c)), (void *)stub_reserve}});
    };
    pool_take.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(pool_take);
    // ObjectRef::SetMemberNear: as SetMember, for references close enough for a one-byte offset
    Test ref_member_near{0x6df9e0, handles, [](void *fn) {
                             ((void(__fastcall *)(void *, void *, void *))fn)(arena<uint8_t>(0x100) - g_member_off, nullptr, arena<void>(0x500));
                             return Result{};
                         }};
    ref_member_near.prepare = [](const rewrite::Entry *e) {
        g_member_off = *(uint8_t *)(e->at + 4);
        stubs_for_now({{0x805700, (void *)stub_handle_of}});
    };
    ref_member_near.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(ref_member_near);
    // QueueSurface::Activate: the entry at 0x900 (the game crashes without one, so that isn't compared), kept or found
    Test queue_surface{0x73aa80,
                       [=](Rng &r) {
                           fresh(r);
                           g_iface_id = (uint32_t *)0x130646c;
                           iface_handle(r);
                           *(void **)arena<uint8_t>(0x34c) = arena<void>(0x360);
                           *arena<void *>(0x1f0) = arena<void>(0x200);
                           arena<void *>(0x240)[1] = arena<void>(0x260);
                           *(int32_t *)arena<uint8_t>(0x270) = 0x5a0;   // the entry, from the object at 0x360
                           *g_iface_id = 1;
                           uint8_t *self = arena<uint8_t>(0x800), *entry = arena<uint8_t>(0x900);
                           *(void **)(self + 0x4c) = arena<void>(0x340);
                           bool kept = r.range(0, 1);
                           self[0x60] = kept;
                           *(void **)(self + 0x5c) = kept ? (void *)entry : nullptr;
                           entry[0x2c] = (uint8_t)r.range(0, 1);
                           entry[0x168] = (uint8_t)r.range(0, 1);
                           *(int32_t *)(entry + 0x16c) = r.range(0, 5);
                       },
                       [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x800)); return Result{}; }};
    queue_surface.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0x800860, (void *)stub_plain_dtor}, {0x7397d0, (void *)stub_plain_dtor}, {0xbe8230, (void *)stub_thiscall2}});
    };
    queue_surface.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(queue_surface);
    // Global::Delete: the object at 0x500 in the copy's global, or none
    static uint32_t g_gd_global;
    Test global_delete{0x409ec0,
                       [=](Rng &r) { fresh(r); arena<uint32_t>(0x10)[0] = r.range(0, 2) ? (uint32_t)(uintptr_t)arena<void>(0x500) : 0; },
                       [](void *fn) { *(uint32_t *)g_gd_global = arena<uint32_t>(0x10)[0]; ((void(__cdecl *)())fn)(); return Result{*(uint32_t *)g_gd_global}; }};   // set here: the game's globals aren't put back between the two runs
    global_delete.prepare = [](const rewrite::Entry *e) {
        g_gd_global = *(uint32_t *)(e->at + 2);
        stubs_for_now({{(uintptr_t)(e->at + 0xd + 5 + *(int32_t *)(e->at + 0xe)), (void *)stub_plain_dtor}});
    };
    global_delete.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(global_delete);
    // Field::Construct: the constructor is a stand-in
    Test field_construct{0x423b70,
                         [=](Rng &r) { fresh(r); arena<uint32_t>(0x10)[0] = r.next(); arena<uint32_t>(0x10)[1] = r.next(); },
                         [](void *fn) {
                             return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], arena<uint32_t>(0x10)[1])};
                         }};
    field_construct.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x8088c0, (void *)stub_field_ctor}}); };
    field_construct.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(field_construct);
    // Event::IfAccepted: the interface kept or found; the answer depends on the arguments
    Test if_accepted{0xaa90d0,
                     [=](Rng &r) {
                         fresh(r);
                         iface_handle(r);
                         uint8_t *self = arena<uint8_t>(0x800);
                         *(void **)(self + 0xf8) = arena<void>(0x340);
                         bool kept = r.range(0, 1);
                         self[0x10c] = kept;
                         *(void **)(self + 0x108) = kept ? arena<void>(0x360) : nullptr;
                         arena<uint32_t>(0x10)[0] = r.next();
                         arena<uint32_t>(0x10)[1] = r.next();
                     },
                     [](void *fn) {
                         ((void(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x10)[0], arena<uint32_t>(0x10)[1]);
                         return Result{};
                     }};
    if_accepted.prepare = [](const rewrite::Entry *e) {
        g_iface_id = (uint32_t *)0x13ce3bc;
        stubs_for_now({{(uintptr_t)(e->at + 0x62 + 5 + *(int32_t *)(e->at + 0x63)), (void *)stub_ask},
                       {(uintptr_t)(e->at + 0x6f + 5 + *(int32_t *)(e->at + 0x70)), (void *)stub_thiscall2}});
    };
    if_accepted.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(if_accepted);
    // Refs::InterfaceAt: up to 3 references at 0x900; the index may be past the end
    static uint32_t g_ria_count, g_ria_array;
    Test ref_interface_at{0x561240,
                          [=](Rng &r) {
                              fresh(r);
                              iface_handle(r);
                              uint8_t *self = arena<uint8_t>(0x1800);
                              *(int32_t *)(self + g_ria_count) = r.range(0, 3);
                              *(void **)(self + g_ria_array) = arena<void>(0x900);
                              for (int i = 0; i < 3; i++) {
                                  uint8_t *ref = arena<uint8_t>(0x900 + i * 0x1c);
                                  *(void **)(ref + 4) = arena<void>(0x340);
                                  bool kept = r.range(0, 1);
                                  ref[0x18] = kept;
                                  *(void **)(ref + 0x14) = kept ? arena<void>(0x3a0) : nullptr;
                              }
                              arena<int32_t>(0x10)[0] = r.range(0, 3);
                          },
                          [](void *fn) {
                              return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, int32_t))fn)(arena<void>(0x1800), arena<int32_t>(0x10)[0])};
                          }};
    ref_interface_at.prepare = [](const rewrite::Entry *e) {
        g_ria_count = *(uint32_t *)(e->at + 6);
        g_ria_array = *(uint32_t *)(e->at + 0xe);
        g_iface_id = *(uint32_t **)(e->at + 0x39);
    };
    quick(ref_interface_at);
    // Grid::Collect: 5 nodes (a root and 4 children) of up to 3 entries, items at 0x700.. matching the query's masks or
    // not; the buffer holds 3, so it sometimes fills
    static uint32_t g_gc_query;
    Test grid_collect{0x48a4e0,
                      [=](Rng &r) {
                          fresh(r);
                          static void *vt[2] = {nullptr, (void *)stub_thiscall1};
                          arena<void *>(0x100)[2] = arena<void>(0x400);
                          for (int k = 0; k < 5; k++) {
                              uint32_t *n = arena<uint32_t>(0x400 + k * 0x1c);
                              n[0] = k ? 0 : (uint32_t)r.range(0, 15) | ((uint32_t)r.range(0, 1) << 4);
                              n[3] = (uint32_t)r.range(0, 1);
                              n[4] = (uint32_t)r.range(0, 3);
                              n[6] = (uint32_t)(uintptr_t)arena<void>(0xa00 + k * 0x80);
                              for (int i = 0; i < 3; i++) {
                                  uint32_t *e = arena<uint32_t>(0xa00 + k * 0x80 + i * 0x20);
                                  e[7] = (uint32_t)r.range(0, 3);
                                  e[6] = (uint32_t)(uintptr_t)arena<void>(0x700 + (k * 3 + i) * 0x20);
                                  uint32_t *item = arena<uint32_t>(0x700 + (k * 3 + i) * 0x20);
                                  item[2] = r.next(); item[6] = (uint32_t)r.range(0, 3); item[7] = (uint32_t)r.range(0, 3);
                              }
                          }
                          uint32_t *q = arena<uint32_t>(0x600);
                          q[0] = (uint32_t)(uintptr_t)vt;
                          q[2] = 1; q[3] = 1; q[4] = (uint32_t)r.range(0, 1); q[5] = (uint32_t)r.range(0, 1);
                          q[6] = 0; q[7] = (uint32_t)r.range(1, 3);
                          q[8] = (uint32_t)(uintptr_t)arena<void>(0x680);
                          *(void **)arena<uint8_t>(0x800 + g_gc_query) = q;
                      },
                      [](void *fn) {
                          ((void(__thiscall *)(void *, int32_t, void *))fn)(arena<void>(0x100), 0, arena<void>(0x800));
                          return Result{};
                      }};
    grid_collect.prepare = [](const rewrite::Entry *e) { g_gc_query = *(uint8_t *)(e->at + 0x46); };
    quick(grid_collect);
    // Settings::Copy: from noise to noise, the padding kept
    Test settings_copy{0x425b80,
                       [=](Rng &r) {
                           fresh(r);
                           for (int i = 0; i < 0x40; i++) { arena<uint32_t>(0x400)[i] = r.next(); arena<uint32_t>(0x600)[i] = r.next(); }
                       },
                       [](void *fn) {
                           return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x400), arena<void>(0x600))};
                       }};
    quick(settings_copy);
    // Rows::SaveBytes: as Rows::Save
    Test rows_save_bytes{0x5ba680, rows_save.setup, rows_save.call};
    rows_save_bytes.prepare = rows_save.prepare;
    rows_save_bytes.finish = rows_save.finish;
    quick(rows_save_bytes);
    // Cached::Forward: the interface kept or found, the method a stand-in
    static uint32_t g_cf2_ref, g_cf2_flag, g_cf2_cache;
    Test cached_forward{0x532a20,
                        [=](Rng &r) {
                            fresh(r); iface_handle(r);
                            uint8_t *self = arena<uint8_t>(0x800);
                            *(void **)(self + g_cf2_ref) = arena<void>(0x340);
                            bool kept = r.range(0, 1);
                            self[g_cf2_flag] = kept;
                            *(void **)(self + g_cf2_cache) = kept ? arena<void>(0x3a0) : nullptr;
                        },
                        [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    cached_forward.prepare = [](const rewrite::Entry *e) {
        g_cf2_flag = *(uint32_t *)(e->at + 5); g_cf2_ref = *(uint32_t *)(e->at + 0xf); g_cf2_cache = *(uint32_t *)(e->at + 0x46);
        g_iface_id = *(uint32_t **)(e->at + 0x28);
        stubs_for_now({{(uintptr_t)(e->at + 0x59 + 5 + *(int32_t *)(e->at + 0x5a)), (void *)stub_item_value}});
    };
    cached_forward.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(cached_forward);
    // Words::Append: up to 8 words into a new block
    static uint32_t g_wa_count, g_wa_data;
    Test words_append{0xcc8320,
                      [=](Rng &r) {
                          fresh(r);
                          *arena<uint32_t>(ALLOC_NEXT) = 0;
                          uint8_t *self = arena<uint8_t>(0x800);
                          *(int32_t *)(self + g_wa_count) = r.range(0, 8);
                          *(void **)(self + g_wa_data) = arena<void>(0x400);
                          for (int i = 0; i < 8; i++) arena<uint32_t>(0x400)[i] = r.next();
                          arena<uint32_t>(0x10)[0] = r.next();
                      },
                      [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x10)[0]); return Result{}; }};
    words_append.prepare = [](const rewrite::Entry *e) {
        g_wa_count = *(uint8_t *)(e->at + 5); g_wa_data = *(uint8_t *)(e->at + 9);
        stubs_for_now({{0xbcdfd0, (void *)stub_alloc_next}});
    };
    words_append.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(words_append);
    // Vector::FreePair: the items after their header, sometimes none; the second block sometimes none
    static uint32_t g_vf_header;
    Test vector_pair{0x40c550,
                     [=](Rng &r) {
                         fresh(r);
                         arena<void *>(0x100)[0] = r.range(0, 2) ? arena<void>(0x500 + g_vf_header) : nullptr;
                         arena<void *>(0x100)[1] = r.range(0, 1) ? arena<void>(0x600) : nullptr;
                         *arena<int32_t>(0x500) = r.range(0, 5);
                     },
                     [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    vector_pair.prepare = [](const rewrite::Entry *e) {
        g_vf_header = 0x100 - *(uint8_t *)(e->at + 0xb);
        stubs_for_now({{0xddcf39, (void *)stub_destroy_each}});
    };
    vector_pair.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(vector_pair);
    static uint32_t g_vff_off;
    Test vector_field{0x48a420,
                      [=](Rng &r) {
                          fresh(r);
                          *(void **)arena<uint8_t>(0x100 + g_vff_off) = r.range(0, 2) ? arena<void>(0x504) : nullptr;
                          *arena<int32_t>(0x500) = r.range(0, 5);
                      },
                      [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    vector_field.prepare = [](const rewrite::Entry *e) {
        g_vff_off = *(uint8_t *)(e->at + 2);
        stubs_for_now({{0xddcf39, (void *)stub_destroy_each}});
    };
    vector_field.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(vector_field);
    // Level::Reached: both lookups stand-ins; the level below, at or above the copy's, or none; the field set or not
    static uint32_t g_lr_off;
    Test level_reached{0x4e28f0,
                       [=](Rng &r) {
                           fresh(r);
                           arena<uint8_t>(0x510)[1] = (uint8_t)r.range(0, 5);
                           *(uint32_t *)arena<uint8_t>(0x800 + g_lr_off) = (uint32_t)r.range(0, 1);
                       },
                       [](void *fn) { return Result{((uint8_t(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    level_reached.prepare = [](const rewrite::Entry *e) {
        g_lr_off = *(uint32_t *)(e->at + 0x20);
        stubs_for_now({{0xbb4280, (void *)stub_lookup_a}, {0xbb3e70, (void *)stub_lookup_b}});
    };
    level_reached.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(level_reached);
    // Recycled::Delete: with and without the flag; the list's head and count are set at the call
    static uint32_t g_rd_head, g_rd_count;
    Test recycled{0xbe8fb0,
                  [=](Rng &r) { fresh(r); arena<uint32_t>(0x10)[0] = (uint32_t)r.range(0, 3); arena<uint32_t>(0x10)[1] = r.next(); arena<uint32_t>(0x10)[2] = r.next(); },
                  [](void *fn) {
                      *(uint32_t *)g_rd_head = arena<uint32_t>(0x10)[1];
                      *(uint32_t *)g_rd_count = arena<uint32_t>(0x10)[2];
                      uint32_t got = (uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0]);
                      return Result{got ^ *(uint32_t *)g_rd_head ^ (*(uint32_t *)g_rd_count * 3)};
                  }};
    recycled.prepare = [](const rewrite::Entry *e) {
        g_rd_head = *(uint32_t *)(e->at + 0x10); g_rd_count = *(uint32_t *)(e->at + 0x17);
        stubs_for_now({{(uintptr_t)(e->at + 3 + 5 + *(int32_t *)(e->at + 4)), (void *)stub_plain_dtor}});
    };
    recycled.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(recycled);
    // OwnerList::Destroy: a list of 0-3 items, some null, then the end node
    Test owner_list{0xc0b4e0,
                    [=](Rng &r) {
                        fresh(r);
                        static void *vt[1] = {(void *)stub_deleting};
                        int n = r.range(0, 3);
                        arena<void *>(0x100)[1] = arena<void>(0x400);
                        for (int i = 0; i <= n; i++) {
                            void **node = arena<void *>(0x400 + i * 0x20);
                            node[0] = i < n ? arena<void>(0x420 + i * 0x20) : nullptr;
                            arena<void *>(0x500 + i * 0x10)[0] = vt;
                            node[2] = r.range(0, 3) ? arena<void>(0x500 + i * 0x10) : nullptr;
                        }
                    },
                    [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(owner_list);
    // SharedBlock::Release: down to none or not
    static uint32_t g_sbr_off;
    Test block_release{0x44a0f0,
                       [=](Rng &r) {
                           fresh(r);
                           *(void **)arena<uint8_t>(0x100 + g_sbr_off) = arena<void>(0x500);
                           arena<int32_t>(0x500)[0] = r.range(1, 3);
                           arena<uint32_t>(0x500)[3] = r.next();
                       },
                       [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    block_release.prepare = [](const rewrite::Entry *e) { g_sbr_off = *(uint8_t *)(e->at + 2); };
    quick(block_release);
    // Field::Delete: with and without the flag
    Test field_delete{0x401200,
                      [=](Rng &r) { fresh(r); arena<uint32_t>(0x10)[0] = (uint32_t)r.range(0, 3); },
                      [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0])}; }};
    quick(field_delete);
    // Debug wrappers
    Test debug_forward{0xcc1d70, fresh, [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    debug_forward.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x11 + 5 + *(int32_t *)(e->at + 0x12)), (void *)stub_item_value}}); };
    debug_forward.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(debug_forward);
    Test debug_delete{0xcc1cd0,
                      [=](Rng &r) { fresh(r); arena<uint32_t>(0x10)[0] = (uint32_t)r.range(0, 3); },
                      [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0])}; }};
    debug_delete.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x11 + 5 + *(int32_t *)(e->at + 0x12)), (void *)stub_plain_dtor}}); };
    debug_delete.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(debug_delete);
    // Grid::MarkUp: 21 nodes, some already flagged
    Test grid_mark{0x42f670,
                   [=](Rng &r) {
                       fresh(r);
                       arena<void *>(0x100)[2] = arena<void>(0x400);
                       for (int k = 0; k < 21; k++) arena<uint32_t>(0x400 + k * 0x1c)[0] = r.range(0, 3) ? 0xf : 0x1f;
                       arena<int32_t>(0x10)[0] = r.range(0, 20);
                   },
                   [](void *fn) { ((void(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0]); return Result{}; }};
    quick(grid_mark);
    // Names::Value: the find a stand-in, found or not
    Test names_value{0x7fbdf0,
                     [=](Rng &r) {
                         fresh(r);
                         uint8_t *self = arena<uint8_t>(0x800);
                         *(void **)self = arena<void>(0x300);
                         *(void **)(self + 8) = arena<void>(0x380);
                         *(int32_t *)(self + 0x14) = r.range(0, 6);
                         arena<int32_t>(0x20)[0] = r.range(0, 6);
                         arena<int32_t>(0x20)[1] = r.range(0, 1);
                     },
                     [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x800), arena<void>(0x400))}; }};
    names_value.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x17 + 5 + *(int32_t *)(e->at + 0x18)), (void *)stub_name_found}}); };
    names_value.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(names_value);
    // Pool::TakeSized: as Pool::Take
    Test pool_take_sized{0x409b60, pool_take.setup, pool_take.call};
    pool_take_sized.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x1b + 5 + *(int32_t *)(e->at + 0x1c)), (void *)stub_reserve}}); };
    pool_take_sized.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(pool_take_sized);
    // Array::NextIndex: full or not
    Test next_index{0x7c0a60,
                    [=](Rng &r) { fresh(r); int n = r.range(0, 5); arena<int32_t>(0x100)[2] = n; arena<int32_t>(0x100)[1] = r.range(0, 1) ? n : n + 2; },
                    [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    next_index.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x11 + 5 + *(int32_t *)(e->at + 0x12)), (void *)stub_reserve}}); };
    next_index.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(next_index);
    // ObjectRef::ConstructMember: at +OFF of noise, joining the null handle's list
    Test member_construct{0x518950,
                          [=](Rng &r) { handles(r); for (int i = 0; i < 16; i++) arena<uint32_t>(0x600)[i] = r.next(); },
                          [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x600))}; }};
    quick(member_construct);
    // ObjectRef::CopyTo: the handle at 0x380 held at +OFF, the new reference at 0x600
    static uint32_t g_rc_off;
    Test ref_copy{0x6dfad0,
                  [=](Rng &r) {
                      handles(r);
                      *(void **)arena<uint8_t>(0x900 + g_rc_off) = arena<void>(0x380);
                      for (int i = 0; i < 8; i++) arena<uint32_t>(0x600)[i] = r.next();
                  },
                  [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x900), arena<void>(0x600))}; }};
    ref_copy.prepare = [](const rewrite::Entry *e) { g_rc_off = *(uint32_t *)(e->at + 0x10); };
    quick(ref_copy);
    // TrackedRide::PassOn: 3 lists of up to 2 references, kept or found
    Test ride_pass{0x5d5cb0,
                   [=](Rng &r) {
                       fresh(r); iface_handle(r);
                       uint8_t *self = arena<uint8_t>(0x800);
                       for (int l = 0; l < 3; l++) {
                           *(void **)(self + 0xac + l * 8) = arena<void>(0x900 + l * 0x40);
                           *(int32_t *)(self + 0xb0 + l * 8) = r.range(0, 2);
                           for (int i = 0; i < 2; i++) {
                               uint8_t *ref = arena<uint8_t>(0x900 + l * 0x40 + i * 0x1c);
                               *(void **)(ref + 4) = arena<void>(0x340);
                               bool kept = r.range(0, 1);
                               ref[0x18] = kept;
                               *(void **)(ref + 0x14) = kept ? arena<void>(0x3a0) : nullptr;
                           }
                       }
                       arena<uint32_t>(0x10)[0] = r.next(); arena<uint32_t>(0x10)[1] = r.next();
                   },
                   [](void *fn) { ((void(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x800), arena<uint32_t>(0x10)[0], arena<uint32_t>(0x10)[1]); return Result{}; }};
    ride_pass.prepare = [](const rewrite::Entry *e) {
        g_iface_id = (uint32_t *)0x12f537c;
        stubs_for_now({{(uintptr_t)(e->at + 0x13 + 5 + *(int32_t *)(e->at + 0x14)), (void *)stub_thiscall2},
                       {(uintptr_t)(e->at + 0x8b + 5 + *(int32_t *)(e->at + 0x8c)), (void *)stub_thiscall2}});
    };
    ride_pass.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(ride_pass);
    // Ride::PassOn: each listener sometimes there; `a` sometimes 0; up to 3 entries, flagged or not
    Test ride_pass2{0x511950,
                    [=](Rng &r) {
                        fresh(r);
                        uint8_t *self = arena<uint8_t>(0x1000);
                        auto maybe = [&](uint32_t off) { *(void **)(self + off) = r.range(0, 1) ? arena<void>(0x3e0) : nullptr; };
                        maybe(0x418); maybe(0x404);
                        for (int k = 0; k < 4; k++) maybe(0x438 + k * 0x90);
                        for (int k = 0; k < 2; k++) maybe(0x674 + k * 0x50);
                        *(void **)(self + 0x310) = arena<void>(0x900);
                        *(int32_t *)(self + 0x314) = r.range(0, 3);
                        for (int i = 0; i < 3; i++) {
                            uint8_t *ref = arena<uint8_t>(0x900 + i * 0x1c);
                            ref[0x18] = 1;
                            *(void **)(ref + 0x14) = arena<void>(0x1800 + i * 0x200);
                            arena<uint8_t>(0x1800 + i * 0x200)[0x1c1] = (uint8_t)r.range(0, 1);
                        }
                        arena<uint32_t>(0x10)[0] = r.range(0, 1) ? r.next() : 0;
                        arena<uint32_t>(0x10)[1] = r.next();
                    },
                    [](void *fn) { ((void(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x1000), arena<uint32_t>(0x10)[0], arena<uint32_t>(0x10)[1]); return Result{}; }};
    ride_pass2.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{(uintptr_t)(e->at + 0x1d + 5 + *(int32_t *)(e->at + 0x1e)), (void *)stub_thiscall2},
                       {(uintptr_t)(e->at + 0x37 + 5 + *(int32_t *)(e->at + 0x38)), (void *)stub_thiscall1},
                       {(uintptr_t)(e->at + 0xbf + 5 + *(int32_t *)(e->at + 0xc0)), (void *)stub_plain_dtor},
                       {0x510d90, (void *)stub_ride_entry}});
    };
    ride_pass2.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(ride_pass2);
    // a list from the head link at 0x3e0: up to 3 nodes at 0x420.. then the end node at 0x4e0; each value in the arena
    auto value_list = [=](Rng &r, int n) {
        void **prev = arena<void *>(0x3e0);
        for (int i = 0; i < n; i++) {
            void **node = arena<void *>(0x420 + i * 0x20);
            prev[0] = node; node[1] = prev; prev = node;
            node[2] = arena<void>(0x500 + i * 0x10);
        }
        void **end = arena<void *>(0x4e0);
        prev[0] = end; end[0] = nullptr; end[1] = prev; end[2] = nullptr;
    };
    // List::RemoveValue: the value one of theirs or nobody's
    Test list_remove{0x435730,
                     [=](Rng &r) {
                         fresh(r);
                         value_list(r, r.range(0, 3));
                         arena<void *>(0x100)[0] = arena<void *>(0x3e0)[0];
                         arena<void *>(0x10)[0] = arena<void>(0x500 + r.range(0, 3) * 0x10);
                     },
                     [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void *>(0x10)[0])}; }};
    quick(list_remove);
    // Linked::DeleteAt: linked soundly or not, with and without the flag
    static uint32_t g_lda_off;
    Test linked_at{0x413810,
                   [=](Rng &r) {
                       fresh(r);
                       void **n = (void **)arena<uint8_t>(0x100 + g_lda_off), **next = arena<void *>(0x320), **prev = arena<void *>(0x340);
                       int broken = r.range(0, 5);
                       n[0] = broken == 1 ? nullptr : (void *)next;
                       n[1] = broken == 2 ? nullptr : (void *)prev;
                       next[1] = broken == 3 ? (void *)prev : (void *)n;
                       prev[0] = broken == 4 ? (void *)next : (void *)n;
                       arena<uint32_t>(0x10)[0] = (uint32_t)r.range(0, 3);
                   },
                   [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0])}; }};
    linked_at.prepare = [](const rewrite::Entry *e) { g_lda_off = *(uint8_t *)(e->at + 5); };
    quick(linked_at);
    // List::DeleteValues: some values null, the list sometimes broken
    Test list_delete{0x49bc60,
                     [=](Rng &r) {
                         fresh(r);
                         int n = r.range(0, 3);
                         value_list(r, n);
                         arena<void *>(0x100)[0] = arena<void *>(0x3e0)[0];
                         for (int i = 0; i < n; i++) if (!r.range(0, 3)) arena<void *>(0x420 + i * 0x20)[2] = nullptr;
                         if (n && !r.range(0, 3)) arena<void *>(0x420 + r.range(0, n - 1) * 0x20)[1] = nullptr;
                     },
                     [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    list_delete.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x3b + 5 + *(int32_t *)(e->at + 0x3c)), (void *)stub_plain_dtor}}); };
    list_delete.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(list_delete);
    // Compare::MappedOne: as Compare::Mapped
    Test mapped_one{0x71d0a0, mapped.setup, mapped.call};
    mapped_one.prepare = mapped.prepare;
    mapped_one.finish = mapped.finish;
    quick(mapped_one);
    // Refs::First: none or some, through the handle at 0x340
    Test refs_first{0x730b30,
                    [=](Rng &r) {
                        fresh(r); iface_handle(r);
                        arena<void *>(0x100)[0] = arena<void>(0x800);
                        arena<int32_t>(0x100)[2] = r.range(0, 2);
                        arena<void *>(0x800)[1] = arena<void>(0x340);
                    },
                    [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    refs_first.prepare = [](const rewrite::Entry *e) { g_iface_id = *(uint32_t **)(e->at + 0x21); };
    quick(refs_first);
    // Bounds::Reset: over noise
    Test bounds{0x891920, fresh, [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    quick(bounds);
    // SharedSlots::Destroy: each slot a shared value or none
    Test slots_destroy{0xc0d740,
                       [=](Rng &r) {
                           fresh(r);
                           for (int i = 0; i < 3; i++) { arena<uint16_t>(0x5a0 + i * 0x10)[0] = (uint16_t)r.range(1, 6); }
                           for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x100)[2 + i] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x5a0 + r.range(0, 2) * 0x10) : 0;
                           *(uint32_t *)0x13e3318 = (uint32_t)(uintptr_t)arena<void>(0x5f0);
                       },
                       [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    slots_destroy.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xc09bf0, (void *)stub_plain_dtor}, {0xca1f80, (void *)stub_shared_free}}); };
    slots_destroy.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(slots_destroy);
    // Chain::Delete: a next object at 0x3000 whose own next is sometimes another at 0x6000
    static uint32_t g_cd_off;
    Test chain_delete{0x42bd60,
                      [=](Rng &r) {
                          fresh(r);
                          *(void **)arena<uint8_t>(0x1000 + g_cd_off) = r.range(0, 2) ? arena<void>(0x3000) : nullptr;
                          *(void **)arena<uint8_t>(0x3000 + g_cd_off) = r.range(0, 1) ? arena<void>(0x6000) : nullptr;
                          *(void **)arena<uint8_t>(0x6000 + g_cd_off) = nullptr;
                          arena<uint32_t>(0x10)[0] = (uint32_t)r.range(0, 3);
                      },
                      [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x1000), arena<uint32_t>(0x10)[0])}; }};
    chain_delete.prepare = [](const rewrite::Entry *e) { g_cd_off = *(uint32_t *)(e->at + 5); };
    quick(chain_delete);
    // Level::Value: as Level::Reached
    Test level_value{0x4d4430,
                     [=](Rng &r) {
                         fresh(r);
                         arena<uint8_t>(0x510)[1] = (uint8_t)r.range(0, 5);
                         *(uint32_t *)arena<uint8_t>(0x800 + g_lr_off) = r.next();
                     },
                     [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x800))}; }};
    level_value.prepare = level_reached.prepare;
    level_value.finish = level_reached.finish;
    quick(level_value);
#include "wave1_tests.inc"
#include "wave2_tests.inc"
#include "wave3_tests.inc"
#include "wave4_tests.inc"
    // Pool::Grow: from up to 4 items (some free) to a size around that; the constructor and the copier are stand-ins
    auto pool_grow_setup = [=](Rng &r) {
        fresh(r);
        *arena<uint32_t>(ALLOC_NEXT) = 0;
        int cap = r.range(0, 4);
        uint32_t *self = arena<uint32_t>(0x100);
        self[0] = cap ? (uint32_t)(uintptr_t)arena<void>(0x400) : 0;
        self[1] = cap ? (uint32_t)(uintptr_t)arena<void>(0x700) : 0;
        self[2] = (uint32_t)r.range(0, cap);
        self[3] = (uint32_t)cap;
        for (int i = 0; i < 0xc0; i++) arena<uint32_t>(0x400)[i] = r.next();
        for (int i = 0; i < 4; i++) arena<int32_t>(0x700)[i] = r.range(0, 3);
        arena<int32_t>(0x10)[0] = cap + r.range(-1, 4);
    };
    auto pool_grow_call = [](void *fn) { ((void(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0]); return Result{}; };
    Test pool_grow16{0x42d730, pool_grow_setup, pool_grow_call};
    pool_grow16.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbcdfd0, (void *)stub_alloc_next}, {0x402b90, (void *)stub_vector_construct4}}); };
    pool_grow16.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(pool_grow16);
    Test pool_grow8{0xcef970, pool_grow_setup, pool_grow_call};
    pool_grow8.prepare = pool_grow16.prepare; pool_grow8.finish = pool_grow16.finish;
    quick(pool_grow8);
    Test pool_grow_copy{0x8ffa60, pool_grow_setup, pool_grow_call};
    pool_grow_copy.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{0xbcdfd0, (void *)stub_alloc_next}, {0x402b90, (void *)stub_vector_construct4},
                       {(uintptr_t)(e->at + 0x9c + 5 + *(int32_t *)(e->at + 0x9d)), (void *)stub_item_copy}});
    };
    pool_grow_copy.finish = pool_grow16.finish;
    quick(pool_grow_copy);
    // Grid::GrowM8 and M4: as Object::GridGrow
    static int32_t g_ggm_level;
    Test grid_grow_m8{0x850870,
                      [=](Rng &r) {
                          fresh(r);
                          *(void **)arena<uint8_t>(0x100) = arena<void>(0x400);
                          *(int32_t *)arena<uint8_t>(0x104) = 0;   // as Object::GridGrow's test: levels start empty
                          for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x400)[i] = 0;
                          g_ggm_level = r.range(0, 6);
                      },
                      [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), g_ggm_level)}; }};
    grid_grow_m8.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x27 + 5 + *(int32_t *)(e->at + 0x28)), (void *)stub_reserve}}); };
    grid_grow_m8.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(grid_grow_m8);
    Test grid_grow_m4{0x850030, grid_grow_m8.setup, grid_grow_m8.call};
    grid_grow_m4.prepare = grid_grow_m8.prepare; grid_grow_m4.finish = grid_grow_m8.finish;
    quick(grid_grow_m4);
    // solo destructors: members and bases are stand-ins; the shared blocks are counted down to none or not
    Test solo_pair{0x53cbd0,
                   [=](Rng &r) {
                       fresh(r);
                       for (int k = 0; k < 2; k++) {
                           int32_t *b = arena<int32_t>(0x500 + k * 0x20);
                           b[0] = r.range(1, 2); b[3] = (int32_t)r.next();
                       }
                       for (uint32_t o : {0x4u, 0x8u, 0x34u, 0x38u}) *(void **)arena<uint8_t>(0x100 + o) = arena<void>(o == 0x4 || o == 0x34 ? 0x500 : 0x520);
                   },
                   [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(solo_pair);
    static uint32_t g_scd_off;
    Test solo_counted{0xc6c210,
                      [=](Rng &r) {
                          fresh(r);
                          static void *vt[1] = {(void *)stub_deleting};
                          arena<void *>(0x500)[0] = vt;
                          arena<int32_t>(0x500)[1] = r.range(1, 2);
                          *(void **)arena<uint8_t>(0x100 + g_scd_off) = arena<void>(0x500);
                      },
                      [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    solo_counted.prepare = [](const rewrite::Entry *e) {
        g_scd_off = *(uint32_t *)(e->at + 0x1f);
        stubs_for_now({{(uintptr_t)(e->at + 0x4f + 5 + *(int32_t *)(e->at + 0x50)), (void *)stub_plain_dtor}});
    };
    solo_counted.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(solo_counted);
    Test solo_tables_member{0xac9810, fresh, [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    solo_tables_member.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x40f730, (void *)stub_plain_dtor}, {0xac7eb0, (void *)stub_plain_dtor}}); };
    solo_tables_member.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(solo_tables_member);
    Test solo_tables_owned{0x40dca0,
                           [=](Rng &r) {
                               fresh(r);
                               static void *vt[1] = {(void *)stub_deleting};
                               arena<void *>(0x500)[0] = vt;
                               arena<void *>(0x100)[0xb] = r.range(0, 1) ? arena<void>(0x500) : nullptr;
                           },
                           [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    solo_tables_owned.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x40db80, (void *)stub_plain_dtor}}); };
    solo_tables_owned.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(solo_tables_owned);
    static uint32_t g_stb_off;
    Test solo_table_block{0x86f7b0,
                          [=](Rng &r) { fresh(r); *(void **)arena<uint8_t>(0x100 + g_stb_off) = r.range(0, 1) ? arena<void>(0x500) : nullptr; },
                          [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    solo_table_block.prepare = [](const rewrite::Entry *e) { g_stb_off = *(uint8_t *)(e->at + 0x25); };
    quick(solo_table_block);
    Test solo_table_member{0x555d70, fresh, [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    solo_table_member.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{(uintptr_t)(e->at + 0x2e + 5 + *(int32_t *)(e->at + 0x2f)), (void *)stub_plain_dtor}, {0x6a1df0, (void *)stub_destroy_b}});
    };
    solo_table_member.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(solo_table_member);
    // Array::ResizeSized: from up to 4 items to 0-6; the constructor and the assign are stand-ins
    Test solo_resize{0x46cfb0,
                     [=](Rng &r) {
                         fresh(r);
                         *arena<uint32_t>(ALLOC_NEXT) = 0;
                         int n = r.range(0, 4);
                         arena<void *>(0x100)[0] = n ? arena<void>(0x400) : nullptr;
                         arena<int32_t>(0x100)[1] = n;
                         arena<int32_t>(0x10)[0] = r.range(0, 3) ? r.range(0, 6) : n;
                     },
                     [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0])}; }};
    solo_resize.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{0xbcdfd0, (void *)stub_alloc_next}, {0x402b90, (void *)stub_vector_construct4},
                       {(uintptr_t)(e->at + 0x9c + 5 + *(int32_t *)(e->at + 0x9d)), (void *)stub_item_copy}});
    };
    solo_resize.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(solo_resize);
    // Record::CopyFields: two records of random bytes; only the copied ranges may change
    Test copy_fields{0x407350,
                     [](Rng &r) {
                         for (int i = 0; i < 0x200; i++) arena<uint32_t>(0x2000)[i] = r.next();
                         arena<uint32_t>(CALL_LOG)[0] = 0;
                     },
                     [](void *fn) { return Result{(uint32_t)((uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x2000), arena<void>(0x2400)) - (uintptr_t)arena<void>(0))}; }};
    quick(copy_fields);
    Test copy_and_set{0x9c60c0, copy_fields.setup, copy_fields.call};
    quick(copy_and_set);
    Test set_fields{0x41f910, copy_fields.setup,
                    [](void *fn) { return Result{(uint32_t)((uintptr_t)((void *(__thiscall *)(void *))fn)(arena<void>(0x2000)) - (uintptr_t)arena<void>(0))}; }};
    quick(set_fields);
    Test copy_ranges{0x445440, copy_fields.setup, copy_fields.call};
    quick(copy_ranges);
    Test set_ranges{0x8defc0, copy_fields.setup, set_fields.call};
    quick(set_ranges);
    // Video::YuvTable: the four tables (0x13E37C0, 4 KB) made random, then all of them hashed after the fill
    Test yuv_table{0xef42a0, [](Rng &r) { arena<uint32_t>(0x10)[0] = r.next(); },
                   [](void *fn) {
                       uint32_t seed = arena<uint32_t>(0x10)[0], *tables = (uint32_t *)0x13e37c0;
                       for (int i = 0; i < 0x400; i++) tables[i] = seed = seed * 1103515245 + 12345;   // set here: globals aren't put back between runs
                       ((void(__cdecl *)())fn)();
                       uint32_t hash = 2166136261u;
                       for (int i = 0; i < 0x400; i++) hash = (hash ^ tables[i]) * 16777619u;
                       return Result{hash};
                   }};
    quick(yuv_table);
    // Planes::TestBox: six planes and a box near them; some normals 0, now and then any bit pattern
    Test planes_test_box{0x42fb10,
                         [](Rng &r) {
                             float *f = arena<float>(0x100);
                             for (int i = 0; i < 24; i++) f[i] = r.range(0, 5) ? r.real(-10, 10) : 0.0f;
                             for (int i = 0; i < 12; i++) arena<float>(0x200)[i] = r.range(0, 30) ? r.real(-10, 10) : r.any_float();
                             // most planes pass (almost) through a corner of the box, so rounding decides the answer
                             const float *m = arena<float>(0x200), *lo = arena<float>(0x210), *sz = arena<float>(0x220);
                             for (int k = 0; k < 6; k++) {
                                 float *p = f + k * 4;
                                 // the corner that decides "inside" (furthest against the normal) or "outside"
                                 bool in = r.range(0, 5) != 0;
                                 float hx = lo[0] + sz[0], hz = sz[1] + lo[1];
                                 double x = (p[0] > 0) == in ? hx : lo[0], y = (p[1] > 0) == in ? m[2] : m[1], z = (p[2] > 0) == in ? hz : lo[1];
                                 p[3] = (float)(-(p[0] * x + p[1] * y + p[2] * z) * (1 + (r.range(-2, 2)) * 3e-8));
                             }
                         },
                         [](void *fn) {
                             return Result{(uint32_t)((int32_t(__thiscall *)(void *, void *, void *, void *))fn)(arena<void>(0x100), arena<float>(0x200),
                                                                                                         arena<float>(0x210), arena<float>(0x220))};
                         }};
    quick(planes_test_box);
    // QuadTree::UpdateBounds: 21 nodes (three levels), most marked changed, each with up to 3 items; ranges random,
    // sometimes NaN; the count of nodes sometimes cuts the last level short
    Test quad_update_bounds{0x430770,
                            [](Rng &r) {
                                uint32_t *tree = arena<uint32_t>(0x100);
                                tree[0] = r.next();
                                tree[1] = r.range(0, 3) ? 21 : r.range(0, 21);
                                tree[2] = (uint32_t)(uintptr_t)arena<void>(0x1000);
                                for (int i = 0; i < 21; i++) {
                                    uint32_t *node = arena<uint32_t>(0x1000 + i * 0x1c);
                                    node[0] = (r.range(0, 3) ? 0x10 : 0) | (r.next() & 0xffef);
                                    auto pick = [&](float span) { int k = r.range(0, 9); return k == 0 ? r.any_float() : k == 1 ? 0.0f : k == 2 ? -0.0f : r.real(-span, span); };
                                    float lo = pick(100), hi = pick(100);
                                    memcpy(&node[1], &lo, 4);
                                    memcpy(&node[2], &hi, 4);
                                    node[3] = r.range(0, 3) ? 0 : r.next();
                                    node[4] = r.range(0, 3);
                                    node[5] = r.next();
                                    node[6] = (uint32_t)(uintptr_t)arena<void>(0x2000 + i * 0x60);
                                    for (int k = 0; k < 3; k++) {
                                        uint32_t *item = arena<uint32_t>(0x2000 + i * 0x60 + k * 0x20);
                                        for (int w = 0; w < 8; w++) item[w] = r.next();
                                        float a = pick(200), b = pick(200);
                                        memcpy(&item[1], &a, 4);
                                        memcpy(&item[4], &b, 4);
                                        item[7] = r.range(0, 3) ? 1 : 0;
                                    }
                                }
                                arena<int32_t>(0x10)[0] = r.range(0, 4) ? 0 : r.range(0, 4);
                            },
                            [](void *fn) { ((void(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0]); return Result{}; }};
    quick(quad_update_bounds);
    // SphereQuery::Set and Box::Overlap: floats near each other, sometimes equal, signed zeros, NaNs
    auto geo_pick = [](Rng &r) { int k = r.range(0, 11); return k == 0 ? r.any_float() : k == 1 ? 0.0f : k == 2 ? -0.0f : k == 3 ? 1.0f : r.real(-20, 20); };
    Test sphere_query_set{0x488660,
                          [=](Rng &r) {
                              for (int i = 0; i < 16; i++) arena<uint32_t>(0x100)[i] = r.next();
                              for (int i = 0; i < 3; i++) arena<float>(0x200)[i] = geo_pick(r);
                              arena<float>(0x10)[0] = geo_pick(r);
                              arena<uint32_t>(0x10)[1] = r.next();
                          },
                          [](void *fn) {
                              ((void(__thiscall *)(void *, void *, float, uint32_t))fn)(arena<void>(0x100), arena<void>(0x200), arena<float>(0x10)[0], arena<uint32_t>(0x10)[1]);
                              return Result{};
                          }};
    quick(sphere_query_set);
    Test box_overlap{0x488710,
                     [=](Rng &r) {
                         float *box = arena<float>(0x100);
                         for (int i = 0; i < 3; i++) {
                             box[i] = geo_pick(r);
                             box[i + 3] = r.range(0, 7) ? box[i] + r.real(0, 20) : geo_pick(r);
                         }
                         float *mid = arena<float>(0x200), *low = arena<float>(0x210), *size = arena<float>(0x220);
                         // corners on or near the box's sides now and then
                         low[0] = r.range(0, 2) ? geo_pick(r) : box[r.range(0, 1) * 3];
                         low[1] = r.range(0, 2) ? geo_pick(r) : box[2 + r.range(0, 1) * 3];
                         size[0] = r.range(0, 2) ? r.real(0, 20) : box[3] - low[0];
                         size[1] = r.range(0, 2) ? r.real(0, 20) : box[5] - low[1];
                         mid[0] = r.next();
                         mid[1] = r.range(0, 2) ? geo_pick(r) : box[1];
                         mid[2] = r.range(0, 2) ? mid[1] + r.real(0, 20) : box[4];
                     },
                     [](void *fn) {
                         return Result{(uint32_t)((int32_t(__thiscall *)(void *, void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x200), arena<void>(0x210), arena<void>(0x220))};
                     }};
    quick(box_overlap);
    // Heap::SiftDown, Build and Sort: up to 40 entries with keys often equal, signed zeros and NaNs
    auto heap_setup = [=](Rng &r) {
        uint32_t *heap = arena<uint32_t>(0x100);
        heap[0] = (uint32_t)(uintptr_t)arena<void>(0x1000);
        heap[1] = r.next();
        heap[2] = r.range(0, 4) ? r.range(0, 40) : r.range(0, 3);
        for (int i = 0; i < 40; i++) {
            uint32_t *e = arena<uint32_t>(0x1000 + i * 24);
            for (int w = 1; w < 6; w++) e[w] = r.next();
            float k = r.range(0, 9) ? (float)r.range(-5, 5) : geo_pick(r);
            memcpy(e, &k, 4);
        }
        int32_t *args = arena<int32_t>(0x10);
        args[1] = r.range(0, (int32_t)heap[2]);
        args[0] = args[1] ? r.range(0, args[1] - 1) : 0;
    };
    Test heap_sift{0x80c530, heap_setup,
                   [](void *fn) { ((void(__thiscall *)(void *, int32_t, int32_t, uint32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<int32_t>(0x10)[1], 7); return Result{}; }};
    quick(heap_sift);
    Test heap_build{0x80c9c0, heap_setup, [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), 7); return Result{}; }};
    quick(heap_build);
    Test heap_sort{0x80cba0, heap_setup, heap_build.call};
    quick(heap_sort);
    Test corner_offsets{0x80c340,
                        [=](Rng &r) {
                            for (int i = 0; i < 2; i++) arena<uint32_t>(0x100)[i] = r.next();
                            for (int i = 0; i < 3; i++) arena<float>(0x200)[i] = geo_pick(r);
                        },
                        [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200)); return Result{}; }};
    quick(corner_offsets);
    // Vector::AngleBetween(before, a, b, c): four vectors, some of them 0 (a length of 0 is passed over), some short or
    // long so the angle lands outside a turn either way, some odd values (signed zeros, NaNs, huge ones)
    Test vector_angle_between{0xc5a5a0,
                              [=](Rng &r) {
                                  for (int v = 0; v < 4; v++) {
                                      const int kind = r.range(0, 5);
                                      const float scale = kind == 0 ? 0.0f : kind == 1 ? r.real(0, 0.5f) : kind == 2 ? r.real(5, 60) : 1.0f;
                                      for (int i = 0; i < 3; i++) arena<float>(0x100 + v * 0x10)[i] = kind == 4 ? geo_pick(r) : scale * r.real(-1, 1);
                                  }
                              },
                              [](void *fn) {
                                  ((void(__fastcall *)(void *, void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x110), arena<void>(0x120), arena<void>(0x130));
                                  return Result{};
                              }};
    quick(vector_angle_between);
    // Quad::Normal(corners, five outputs): four corner normals (16 bytes apart), sometimes one the other's opposite so a
    // sum is 0 (its z set to 1), sometimes odd values
    Test quad_normal{0x727ea0,
                     [=](Rng &r) {
                         for (int v = 0; v < 4; v++)
                             for (int i = 0; i < 4; i++) arena<float>(0x100 + v * 0x10)[i] = r.range(0, 7) ? r.real(-1, 1) : geo_pick(r);
                         if (r.range(0, 2) == 0) {   // (two corners that cancel)
                             const int a = r.range(0, 3), b = (a + 1 + r.range(0, 2)) % 4;
                             for (int i = 0; i < 3; i++) arena<float>(0x100 + b * 0x10)[i] = -arena<float>(0x100 + a * 0x10)[i];
                         }
                         for (int i = 0; i < 20; i++) arena<uint32_t>(0x200)[i] = r.next();
                     },
                     [](void *fn) {
                         ((void(__fastcall *)(void *, void *, void *, void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x200), arena<void>(0x210),
                                                                                                 arena<void>(0x220), arena<void>(0x230), arena<void>(0x240));
                         return Result{};
                     }};
    quick(quad_normal);
    // Matrix::Invert3: rows 16 bytes apart (the fourth float of each random), sometimes singular, sometimes odd values
    Test matrix_invert3{0xbee0c0,
                        [=](Rng &r) {
                            for (int i = 0; i < 12; i++) arena<float>(0x100)[i] = (i & 3) == 3 ? r.real(-5, 5) : r.range(0, 9) ? r.real(-3, 3) : geo_pick(r);
                            if (r.range(0, 5) == 0)   // (a row twice: no inverse)
                                for (int i = 0; i < 3; i++) arena<float>(0x100)[8 + i] = arena<float>(0x100)[r.range(0, 1) * 4 + i];
                        },
                        [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(matrix_invert3);
    // Matrix::Transform(out, v, m): out apart from v, or the same place
    Test matrix_transform{0x42f0b0,
                          [=](Rng &r) {
                              for (int i = 0; i < 4; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-50, 50) : geo_pick(r);
                              for (int i = 0; i < 16; i++) arena<float>(0x200)[i] = r.range(0, 9) ? r.real(-2, 2) : geo_pick(r);
                              for (int i = 0; i < 4; i++) arena<uint32_t>(0x140)[i] = r.next();
                              arena<uint32_t>(0x10)[0] = r.range(0, 3) == 0;
                          },
                          [](void *fn) {
                              ((void(__fastcall *)(void *, void *, void *))fn)(arena<void>(arena<uint32_t>(0x10)[0] ? 0x100 : 0x140), arena<void>(0x100), arena<void>(0x200));
                              return Result{};
                          }};
    quick(matrix_transform);
    // Vector::RotateAround(v, axis, angle) and RotateAroundSinCos(v, axis, sin, cos): a unit axis (or not quite), any
    // angle (some large)
    auto rotation_case = [=](Rng &r) {
        float a[3], len = 0;
        for (int i = 0; i < 3; i++) a[i] = r.real(-1, 1), len += a[i] * a[i];
        len = len > 0 ? sqrtf(len) : 1;
        for (int i = 0; i < 3; i++) arena<float>(0x100)[i] = r.range(0, 7) ? a[i] / len : geo_pick(r);
        for (int i = 0; i < 3; i++) arena<float>(0x110)[i] = r.range(0, 9) ? r.real(-30, 30) : geo_pick(r);
        const float angle = r.range(0, 7) ? r.real(-7, 7) : r.real(-1e6f, 1e6f);
        arena<float>(0x120)[0] = angle, arena<float>(0x120)[1] = sinf(angle), arena<float>(0x120)[2] = r.range(0, 4) ? cosf(angle) : r.real(-1, 1);
    };
    Test vector_rotate_around{0xbfb0b0, rotation_case,
                              [](void *fn) { ((void(__thiscall *)(void *, void *, float))fn)(arena<void>(0x110), arena<void>(0x100), arena<float>(0x120)[0]); return Result{}; }};
    quick(vector_rotate_around);
    Test vector_rotate_around_sin_cos{0xbfb1c0, rotation_case,
                                      [](void *fn) {
                                          ((void(__thiscall *)(void *, void *, float, float))fn)(arena<void>(0x110), arena<void>(0x100), arena<float>(0x120)[1], arena<float>(0x120)[2]);
                                          return Result{};
                                      }};
    quick(vector_rotate_around_sin_cos);
    // Bezier::Point(out, p0, p1, p2, p3, t): t mostly within [0, 1], sometimes outside
    Test bezier_point{0xbcbf90,
                      [=](Rng &r) {
                          for (int i = 0; i < 16; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-100, 100) : geo_pick(r);
                          arena<float>(0x10)[0] = r.range(0, 5) ? r.real(0, 1) : r.range(0, 1) ? r.real(-3, 3) : geo_pick(r);
                      },
                      [](void *fn) {
                          ((void(__fastcall *)(void *, void *, void *, void *, void *, float))fn)(arena<void>(0x200), arena<void>(0x100), arena<void>(0x110),
                                                                                                  arena<void>(0x120), arena<void>(0x130), arena<float>(0x10)[0]);
                          return Result{};
                      }};
    quick(bezier_point);
    // Plane::FromPoints(a, b, c, normal, d) and Plane::FromPointsC9B970(plane, a, b, c): three points, sometimes on one
    // line or two the same (no normal: its z set to 1)
    auto plane_case = [=](Rng &r) {
        for (int i = 0; i < 12; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-40, 40) : geo_pick(r);
        const int k = r.range(0, 5);
        if (k == 0) for (int i = 0; i < 3; i++) arena<float>(0x120)[i] = arena<float>(0x100)[i];   // (c on a)
        if (k == 1) for (int i = 0; i < 3; i++) arena<float>(0x120)[i] = arena<float>(0x100)[i] + 2 * (arena<float>(0x110)[i] - arena<float>(0x100)[i]);
        for (int i = 0; i < 8; i++) arena<uint32_t>(0x200)[i] = r.next();
    };
    Test plane_from_points{0xcdce20, plane_case,
                           [](void *fn) {
                               ((void(__stdcall *)(void *, void *, void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x110), arena<void>(0x120), arena<void>(0x200), arena<void>(0x210));
                               return Result{};
                           }};
    quick(plane_from_points);
    Test plane_from_points_c9b970{0xc9b970, plane_case,
                                  [](void *fn) {
                                      ((void(__thiscall *)(void *, void *, void *, void *))fn)(arena<void>(0x200), arena<void>(0x100), arena<void>(0x110), arena<void>(0x120));
                                      return Result{};
                                  }};
    quick(plane_from_points_c9b970);
    // TrackPiece::Lerp5BC210(a, t, b): ten whole numbers each, t mostly within [0, 1] (some halves, so rounding decides)
    Test track_piece_lerp{0x5bc210,
                          [=](Rng &r) {
                              for (int i = 0; i < 20; i++) arena<int32_t>(0x100)[i] = r.range(0, 5) ? r.range(-5000, 5000) : (int32_t)r.next();
                              arena<float>(0x10)[0] = r.range(0, 3) == 0 ? 0.5f : r.range(0, 5) ? r.real(0, 1) : geo_pick(r);
                          },
                          [](void *fn) { ((void(__thiscall *)(void *, float, void *))fn)(arena<void>(0x100), arena<float>(0x10)[0], arena<void>(0x128)); return Result{}; }};
    quick(track_piece_lerp);
    // Fence::CrossA905F0(out, p1..p8): two lines, sometimes parallel or nearly so
    Test fence_cross{0xa905f0,
                     [=](Rng &r) {
                         for (int i = 0; i < 8; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-50, 50) : geo_pick(r);
                         if (r.range(0, 4) == 0) arena<float>(0x100)[3] = arena<float>(0x100)[2] * arena<float>(0x100)[7] / arena<float>(0x100)[6];   // (det near 0)
                     },
                     [](void *fn) {
                         const float *p = arena<float>(0x100);
                         float *got = ((float *(__thiscall *)(void *, float, float, float, float, float, float, float, float))fn)(arena<void>(0x200), p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
                         return Result{(uint32_t)(got == arena<float>(0x200))};
                     }};
    quick(fence_cross);
    // Water::Scale8E6C10(this, p, s1, s2, out, -): the water's three vectors (+0xE0, +0xF0, +0x100), a point, two scales
    Test water_scale{0x8e6c10,
                     [=](Rng &r) {
                         for (int i = 0; i < 0x50; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-60, 60) : geo_pick(r);
                         for (int i = 0; i < 12; i++) arena<uint32_t>(0x300)[i] = r.next();
                         arena<float>(0x10)[0] = r.real(-3, 3), arena<float>(0x10)[1] = r.range(0, 9) ? r.real(-3, 3) : geo_pick(r);
                     },
                     [](void *fn) {
                         ((void(__thiscall *)(void *, void *, float, float, void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x2c0), arena<float>(0x10)[0],
                                                                                                 arena<float>(0x10)[1], arena<void>(0x300), 0);
                         return Result{};
                     }};
    quick(water_scale);
    // Water::Point8E6D20(this, m, h, low, high): a matrix's rows, the water's vector at +0xE0, a scale; zeros and odd values
    Test water_point{0x8e6d20,
                     [=](Rng &r) {
                         for (int i = 0; i < 0x50; i++) arena<float>(0x100)[i] = r.range(0, 7) ? r.real(-60, 60) : r.range(0, 1) ? 0.0f : geo_pick(r);
                         for (int i = 0; i < 12; i++) arena<float>(0x2c0)[i] = r.range(0, 7) ? r.real(-60, 60) : r.range(0, 1) ? -0.0f : geo_pick(r);
                         if (r.range(0, 3) == 0) arena<uint32_t>(0x2c0)[r.range(0, 11)] = 0x7fc00000u | (r.range(0, 1) << 31);   // (a NaN term: to the low side)
                         for (int i = 0; i < 8; i++) arena<uint32_t>(0x300)[i] = r.next();
                         arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-3, 3) : geo_pick(r);
                     },
                     [](void *fn) {
                         ((void(__thiscall *)(void *, void *, float, void *, void *))fn)(arena<void>(0x100), arena<void>(0x2c0), arena<float>(0x10)[0], arena<void>(0x300), arena<void>(0x310));
                         return Result{};
                     }};
    quick(water_point);
    // Water::Normal8E6150(this, v): a direction, sometimes straight up or down (nothing flat: the fixed answer)
    Test water_normal{0x8e6150,
                      [=](Rng &r) {
                          for (int i = 0; i < 0x50; i++) arena<uint32_t>(0x100)[i] = r.next();
                          for (int i = 0; i < 3; i++) arena<float>(0x10)[i] = r.range(0, 9) ? r.real(-2, 2) : geo_pick(r);
                          if (r.range(0, 4) == 0) arena<float>(0x10)[0] = arena<float>(0x10)[2] = r.range(0, 1) ? 0.0f : r.real(-1e-3f, 1e-3f);
                      },
                      [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x10)); return Result{}; }};
    quick(water_normal);
    // Rect::Union / Intersect / UnionInto / IntersectInto(this, out, other): two rectangles, sides sometimes equal,
    // sometimes any bits at all (NaNs, signalling ones: they come out quiet where the game loads them)
    auto rect_case = [=](Rng &r) {
        for (int i = 0; i < 8; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-100, 100) : geo_pick(r);
        for (int i = 0; i < 4; i++) if (r.range(0, 5) == 0) arena<float>(0x110)[i] = arena<float>(0x100)[i];
        if (r.range(0, 4) == 0) arena<uint32_t>(0x100)[r.range(0, 7)] = r.next();
        if (r.range(0, 6) == 0) arena<uint32_t>(0x100)[r.range(0, 7)] = 0x7f800001u + (uint32_t)r.range(0, 0x3fffff);   // (signalling)
        for (int i = 0; i < 4; i++) arena<uint32_t>(0x200)[i] = r.next();
    };
    auto rect_call = [](void *fn) { ((void(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x200), arena<void>(0x110)); return Result{}; };
    Test rect_union{0xbe7690, rect_case, rect_call};
    quick(rect_union);
    Test rect_intersect{0xbe7520, rect_case, rect_call};
    quick(rect_intersect);
    Test rect_union_into{0xbe75e0, rect_case, rect_call};
    quick(rect_union_into);
    Test rect_intersect_into{0xbe7470, rect_case, rect_call};
    quick(rect_intersect_into);
    // Matrix::Transpose / Transpose2 and Matrix::From4x3(in, out): any bits, signalling NaNs among them
    auto bits_case = [=](Rng &r) {
        for (int i = 0; i < 16; i++) arena<float>(0x100)[i] = r.real(-9, 9);
        for (int i = 0; i < 16; i++) if (r.range(0, 3) == 0) arena<uint32_t>(0x100)[i] = r.next();
        if (r.range(0, 3) == 0) arena<uint32_t>(0x100)[r.range(0, 15)] = 0x7f800001u + (uint32_t)r.range(0, 0x3fffff);
        for (int i = 0; i < 16; i++) arena<uint32_t>(0x200)[i] = r.next();
    };
    auto two_call = [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200)); return Result{}; };
    Test matrix_transpose{0xc73730, bits_case, two_call};
    quick(matrix_transpose);
    Test matrix_transpose2{0xc73680, bits_case, two_call};
    quick(matrix_transpose2);
    Test matrix_from_4x3{0x856550, bits_case, two_call};
    quick(matrix_from_4x3);
    // Quaternion::ToMatrix(q, out): unit quaternions mostly, sometimes any values
    Test quaternion_to_matrix{0xb00c00,
                              [=](Rng &r) {
                                  float q[4], n = 0;
                                  for (int i = 0; i < 4; i++) q[i] = r.real(-1, 1), n += q[i] * q[i];
                                  n = n > 0 ? sqrtf(n) : 1;
                                  for (int i = 0; i < 4; i++) arena<float>(0x100)[i] = r.range(0, 7) ? q[i] / n : geo_pick(r);
                                  for (int i = 0; i < 12; i++) arena<uint32_t>(0x200)[i] = r.next();
                              },
                              two_call};
    quick(quaternion_to_matrix);
    // CameraAngles::Clamp(this): tilts inside, outside and at the limits, NaNs; turns of any size
    Test camera_angles_clamp{0xa61b90,
                             [=](Rng &r) {
                                 for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next();
                                 for (int k = 0; k < 2; k++) {
                                     const int pick = r.range(0, 6);
                                     arena<float>(0x130)[k] = pick < 3 ? r.real(-3, 3) : pick == 3 ? 1.5677f : pick == 4 ? -1.5677f : geo_pick(r);
                                     if (pick == 6) arena<uint32_t>(0x130)[k] = r.range(0, 1) ? 0x7f800001u + r.range(0, 99) : r.next();
                                 }
                                 arena<float>(0x138)[0] = r.range(0, 5) ? r.real(-40, 40) : geo_pick(r);
                             },
                             [](void *fn) { ((void(__thiscall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(camera_angles_clamp);
    // Vector::TransformPoint(out, p, m) and TransformPoint2(m, out, p): out apart from p, or the same place
    auto transform_case = [=](Rng &r) {
        for (int i = 0; i < 3; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-50, 50) : geo_pick(r);
        for (int i = 0; i < 16; i++) arena<float>(0x200)[i] = r.range(0, 9) ? r.real(-2, 2) : geo_pick(r);
        for (int i = 0; i < 4; i++) arena<uint32_t>(0x140)[i] = r.next();
        arena<uint32_t>(0x10)[0] = r.range(0, 3) == 0;
    };
    Test vector_transform_point{0x481360, transform_case,
                                [](void *fn) {
                                    ((void(__fastcall *)(void *, void *, void *))fn)(arena<void>(arena<uint32_t>(0x10)[0] ? 0x100 : 0x140), arena<void>(0x100), arena<void>(0x200));
                                    return Result{};
                                }};
    quick(vector_transform_point);
    Test vector_transform_point2{0xbb6050, transform_case,
                                 [](void *fn) {
                                     ((void(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x200), arena<void>(arena<uint32_t>(0x10)[0] ? 0x100 : 0x140), arena<void>(0x100));
                                     return Result{};
                                 }};
    quick(vector_transform_point2);
    // Quad::EdgeMiddles(quad, out0..out4): four corners, now and then a signalling NaN among them
    Test quad_edge_middles{0x727de0,
                           [=](Rng &r) {
                               for (int i = 0; i < 16; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-60, 60) : geo_pick(r);
                               if (r.range(0, 4) == 0) arena<uint32_t>(0x100)[r.range(0, 15)] = 0x7f800001u + (uint32_t)r.range(0, 0x3fffff);
                               for (int i = 0; i < 20; i++) arena<uint32_t>(0x200)[i] = r.next();
                           },
                           [](void *fn) {
                               ((void(__fastcall *)(void *, void *, void *, void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x200), arena<void>(0x210),
                                                                                                       arena<void>(0x220), arena<void>(0x230), arena<void>(0x240));
                               return Result{};
                           }};
    quick(quad_edge_middles);
    // Spline::SegmentPoints(this, n, p0, p1, p2, p3): four spline points (0x24 bytes each), segment 0, 1 or 2
    Test spline_segment_points{0x4240b0,
                               [=](Rng &r) {
                                   arena<uint32_t>(0x144)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                                   for (int i = 0; i < 36; i++) arena<float>(0x400)[i] = r.range(0, 9) ? r.real(-80, 80) : geo_pick(r);
                                   if (r.range(0, 4) == 0) arena<uint32_t>(0x400)[r.range(0, 35)] = 0x7f800001u + (uint32_t)r.range(0, 0x3fffff);
                                   for (int i = 0; i < 16; i++) arena<uint32_t>(0x200)[i] = r.next();
                                   arena<int32_t>(0x10)[0] = r.range(0, 2);
                               },
                               [](void *fn) {
                                   ((void(__thiscall *)(void *, int32_t, void *, void *, void *, void *))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<void>(0x200),
                                                                                                           arena<void>(0x210), arena<void>(0x220), arena<void>(0x230));
                                   return Result{};
                               }};
    quick(spline_segment_points);
    // Points::UpdateDepth(this, n): the view values in this (+0x18 to +0x48), a list of three points (0x70 bytes each)
    Test points_update_depth{0x42ff50,
                             [=](Rng &r) {
                                 for (int i = 0; i < 0x20; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-3, 3) : geo_pick(r);
                                 arena<uint32_t>(0x100)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                                 for (int i = 0; i < 0x54; i++) arena<float>(0x400)[i] = r.range(0, 9) ? r.real(-90, 90) : geo_pick(r);
                                 arena<int32_t>(0x10)[0] = r.range(0, 2);
                             },
                             [](void *fn) { ((void(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0]); return Result{}; }};
    quick(points_update_depth);
    // Bits::PackScaled(v): small and huge whole numbers
    Test bits_pack_scaled{0xc197d0,
                          [=](Rng &r) { arena<int32_t>(0x10)[0] = r.range(0, 3) ? r.range(-100000, 100000) : (int32_t)r.next(); },
                          [](void *fn) { return Result{((uint32_t(__stdcall *)(int32_t))fn)(arena<int32_t>(0x10)[0])}; }};
    quick(bits_pack_scaled);
    // Vector::SideDirection(this, out): the vector at +0x3F0, sometimes flat on the ground's other axis (length 0)
    Test vector_side_direction{0x4262f0,
                               [=](Rng &r) {
                                   for (int i = 0; i < 3; i++) arena<float>(0x4f0)[i] = r.range(0, 9) ? r.real(-5, 5) : geo_pick(r);
                                   if (r.range(0, 4) == 0) arena<float>(0x4f0)[0] = arena<float>(0x4f0)[2] = 0.0f;
                                   for (int i = 0; i < 4; i++) arena<uint32_t>(0x200)[i] = r.next();
                               },
                               [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200)); return Result{}; }};
    quick(vector_side_direction);
    // Segment::Direction(this, out): two parts (+0x2C, +0x28) whose +4 points at a point; sometimes the same point
    Test segment_direction{0x5be7f0,
                           [=](Rng &r) {
                               arena<uint32_t>(0x12c)[0] = (uint32_t)(uintptr_t)arena<void>(0x300), arena<uint32_t>(0x128)[0] = (uint32_t)(uintptr_t)arena<void>(0x340);
                               arena<uint32_t>(0x304)[0] = (uint32_t)(uintptr_t)arena<void>(0x380), arena<uint32_t>(0x344)[0] = (uint32_t)(uintptr_t)arena<void>(r.range(0, 4) ? 0x3a0 : 0x380);
                               for (int i = 0; i < 12; i++) arena<float>(0x380)[i] = r.range(0, 9) ? r.real(-50, 50) : geo_pick(r);
                               for (int i = 0; i < 4; i++) arena<uint32_t>(0x200)[i] = r.next();
                           },
                           [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200)); return Result{}; }};
    quick(segment_direction);
    // Lerp::Current(this): an amount below 1, at 1, above, or odd
    Test lerp_current{0xc219d0,
                      [=](Rng &r) {
                          for (int i = 0; i < 12; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-70, 70) : geo_pick(r);
                          const int k = r.range(0, 5);
                          arena<float>(0x130)[0] = k < 3 ? r.real(0, 1) : k == 3 ? 1.0f : k == 4 ? r.real(1, 3) : geo_pick(r);
                      },
                      [](void *fn) { return Result{(uint32_t)(((float *(__thiscall *)(void *))fn)(arena<void>(0x100)) == arena<float>(0x100))}; }};
    quick(lerp_current);
    // Math::Round(this, a, b): two values and the fractions kept in this
    Test math_round{0xcdb7b0,
                    [=](Rng &r) {
                        for (int i = 0; i < 6; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-5, 5) : geo_pick(r);
                        arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-300, 300) : geo_pick(r), arena<float>(0x10)[1] = r.range(0, 9) ? r.real(-300, 300) : geo_pick(r);
                    },
                    [](void *fn) { ((void(__thiscall *)(void *, float, float))fn)(arena<void>(0x100), arena<float>(0x10)[0], arena<float>(0x10)[1]); return Result{}; }};
    quick(math_round);
    // VerletPoint::Init(this, position, velocity, owner)
    Test verlet_point_init{0xa6b190,
                           [=](Rng &r) {
                               for (int i = 0; i < 0x14; i++) arena<uint32_t>(0x100)[i] = r.next();
                               for (int i = 0; i < 6; i++) arena<float>(0x200)[i] = r.range(0, 9) ? r.real(-60, 60) : geo_pick(r);
                               arena<uint32_t>(0x10)[0] = r.next();
                           },
                           [](void *fn) {
                               ((void(__thiscall *)(void *, void *, void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x200), arena<void>(0x210), arena<uint32_t>(0x10)[0]);
                               return Result{};
                           }};
    quick(verlet_point_init);
    // Particle::SetColours(this, first, second)
    Test particle_set_colours{0x9c6210,
                              [=](Rng &r) { for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x100)[i] = r.next(); arena<uint32_t>(0x10)[0] = r.next(), arena<uint32_t>(0x10)[1] = r.next(); },
                              [](void *fn) { ((void(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], arena<uint32_t>(0x10)[1]); return Result{}; }};
    quick(particle_set_colours);
    // Grid::SnapIndex(flag, v, size, scale, tolerance): values on and between steps, tolerances small and large
    Test grid_snap_index{0xbc4ff0,
                         [=](Rng &r) {
                             const float size = r.range(0, 5) ? r.real(0.5f, 8) : geo_pick(r);
                             const int k = r.range(-20, 20);
                             arena<float>(0x10)[0] = r.range(0, 2) ? k * size + r.real(-0.2f, 0.2f) : r.real(-200, 200);
                             arena<float>(0x10)[1] = size, arena<float>(0x10)[2] = r.range(0, 5) && size != 0 ? 1.0f / size : geo_pick(r);
                             arena<float>(0x10)[3] = r.range(0, 4) ? r.real(0, 0.3f) : geo_pick(r);
                             arena<uint32_t>(0x100)[0] = r.next();
                         },
                         [](void *fn) {
                             const float *a = arena<float>(0x10);
                             return Result{(uint32_t)((int32_t(__thiscall *)(void *, float, float, float, float))fn)(arena<void>(0x100), a[0], a[1], a[2], a[3])};
                         }};
    quick(grid_snap_index);
    // Progress::Advance(this, time): progress values below 1 and near it, counts, times
    Test progress_advance{0xb34ef0,
                          [=](Rng &r) {
                              for (int i = 0; i < 0x60; i++) arena<uint32_t>(0x100)[i] = r.next();
                              arena<float>(0x234)[0] = r.range(0, 9) ? r.real(0, 1.1f) : geo_pick(r);
                              arena<float>(0x250)[0] = r.range(0, 9) ? r.real(0, 1.1f) : geo_pick(r);
                              arena<int32_t>(0x1d8)[0] = r.range(-5, 40);
                              arena<float>(0x10)[0] = r.range(0, 9) ? r.real(0, 3) : geo_pick(r);
                          },
                          [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x10)[0])}; }};
    quick(progress_advance);
    // Camera::SetFromHeight(this, height, lift): a field of view, a direction, the flag that adds bit 0
    Test camera_set_from_height{0x7455f0,
                                [=](Rng &r) {
                                    for (int i = 0; i < 0xb0; i++) arena<uint32_t>(0x100)[i] = r.next();
                                    for (int i = 0; i < 3; i++) arena<float>(0x340)[i] = r.real(-1, 1);
                                    arena<float>(0x390)[0] = r.range(0, 9) ? r.real(0.2f, 2.5f) : geo_pick(r);
                                    arena<uint8_t>(0x3a7)[0] = (uint8_t)r.range(0, 1);
                                    arena<float>(0x10)[0] = r.range(0, 9) ? r.real(1, 300) : geo_pick(r), arena<float>(0x10)[1] = r.real(-20, 20);
                                },
                                [](void *fn) { ((void(__thiscall *)(void *, float, float))fn)(arena<void>(0x100), arena<float>(0x10)[0], arena<float>(0x10)[1]); return Result{}; }};
    quick(camera_set_from_height);
    // Terrain::Fill8BE510 / Make8BE610: points, colours, a base and scale; the vertex and index buffers and the count
    auto terrain_case = [=](Rng &r) {
        for (int i = 0; i < 16; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-90, 90) : geo_pick(r);
        for (int i = 0; i < 0x30; i++) arena<uint32_t>(0x400)[i] = r.next();
        for (int i = 0; i < 8; i++) arena<uint32_t>(0x500)[i] = r.next();
        arena<uint32_t>(0x200)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<uint32_t>(0x204)[0] = (uint32_t)(uintptr_t)arena<void>(0x500);
        arena<uint16_t>(0x208)[0] = (uint16_t)r.next();
        for (int i = 0; i < 6; i++) arena<uint32_t>(0x20)[i] = r.next();
        arena<float>(0x20)[4] = r.real(-20, 20), arena<float>(0x20)[5] = r.range(0, 9) ? r.real(-3, 3) : geo_pick(r);
    };
    Test terrain_fill{0x8be510, terrain_case,
                      [](void *fn) {
                          const uint32_t *k = arena<uint32_t>(0x20);
                          ((void(__fastcall *)(void *, void *, void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, float, float, void *, void *, void *))fn)(
                              arena<void>(0x100), arena<void>(0x110), arena<void>(0x120), k[0], k[1], k[2], k[3], k[0] ^ k[1], arena<float>(0x20)[4], arena<float>(0x20)[5],
                              arena<void>(0x200), arena<void>(0x204), arena<void>(0x208));
                          return Result{};
                      }};
    quick(terrain_fill);
    Test terrain_make{0x8be610, terrain_case,
                      [](void *fn) {
                          const uint32_t *k = arena<uint32_t>(0x20);
                          ((void(__fastcall *)(void *, void *, void *, void *, uint32_t, uint32_t, uint32_t, uint32_t, float, float, void *, void *, void *))fn)(
                              arena<void>(0x100), arena<void>(0x110), arena<void>(0x120), arena<void>(0x130), k[0], k[1], k[2], k[3], arena<float>(0x20)[4],
                              arena<float>(0x20)[5], arena<void>(0x200), arena<void>(0x204), arena<void>(0x208));
                          return Result{};
                      }};
    quick(terrain_make);
    // Vector::StepBCC090(out, p0, p1, p2, p3, t)
    Test vector_step_bcc090{0xbcc090,
                            [=](Rng &r) {
                                for (int i = 0; i < 16; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-100, 100) : geo_pick(r);
                                arena<float>(0x10)[0] = r.range(0, 5) ? r.real(0, 1) : r.range(0, 1) ? r.real(-3, 3) : geo_pick(r);
                            },
                            [](void *fn) {
                                ((void(__fastcall *)(void *, void *, void *, void *, void *, float))fn)(arena<void>(0x200), arena<void>(0x100), arena<void>(0x110),
                                                                                                        arena<void>(0x120), arena<void>(0x130), arena<float>(0x10)[0]);
                                return Result{};
                            }};
    quick(vector_step_bcc090);
    // Water::Compare95DE00(vertices, indices, count, a, b, c, d, sum, low): heights with NaNs (signalling too) now and then
    Test water_compare{0x95de00,
                       [=](Rng &r) {
                           for (int i = 0; i < 16; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-90, 90) : geo_pick(r);
                           if (r.range(0, 4) == 0) arena<uint32_t>(0x100)[r.range(0, 3) * 4 + 1] = r.range(0, 1) ? 0x7f800001u + r.range(0, 99) : 0xffc00000u;
                           for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x400)[i] = r.next();
                           arena<uint32_t>(0x200)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<uint32_t>(0x204)[0] = (uint32_t)(uintptr_t)arena<void>(0x500);
                           arena<uint32_t>(0x208)[0] = r.next();
                           arena<float>(0x20c)[0] = r.real(-100, 100), arena<float>(0x210)[0] = r.range(0, 9) ? r.real(-100, 100) : geo_pick(r);
                       },
                       [](void *fn) {
                           ((void(__fastcall *)(void *, void *, void *, void *, void *, void *, void *, void *, void *))fn)(
                               arena<void>(0x200), arena<void>(0x204), arena<void>(0x208), arena<void>(0x100), arena<void>(0x110), arena<void>(0x120), arena<void>(0x130),
                               arena<void>(0x20c), arena<void>(0x210));
                           return Result{};
                       }};
    quick(water_compare);
    // Text::Layout690F00(out, c0, w0, w1, c1, w2, c2, w3, c3): colours in 0..1 weighted (some out of range, odd)
    Test text_layout_690f00{0x690f00,
                            [=](Rng &r) {
                                for (int i = 0; i < 16; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(0, 1) : geo_pick(r);
                                for (int i = 0; i < 4; i++) arena<float>(0x10)[i] = r.range(0, 9) ? r.real(0, 1) : r.real(-3, 3);
                                arena<uint32_t>(0x200)[0] = r.next();
                            },
                            [](void *fn) {
                                const float *w = arena<float>(0x10);
                                ((void(__fastcall *)(void *, void *, float, float, void *, float, void *, float, void *))fn)(
                                    arena<void>(0x200), arena<void>(0x100), w[0], w[1], arena<void>(0x110), w[2], arena<void>(0x120), w[3], arena<void>(0x130));
                                return Result{};
                            }};
    quick(text_layout_690f00);
    // Water::Fill95DD10(vertices, indices, count, a, b, c, sum, low)
    Test water_fill{0x95dd10,
                    [=](Rng &r) {
                        for (int i = 0; i < 12; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-90, 90) : geo_pick(r);
                        if (r.range(0, 4) == 0) arena<uint32_t>(0x100)[r.range(0, 2) * 4 + 1] = r.range(0, 1) ? 0x7f800001u + r.range(0, 99) : 0xffc00000u;
                        for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x400)[i] = r.next();
                        arena<uint32_t>(0x200)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<uint32_t>(0x204)[0] = (uint32_t)(uintptr_t)arena<void>(0x500);
                        arena<uint32_t>(0x208)[0] = r.next();
                        arena<float>(0x20c)[0] = r.real(-100, 100), arena<float>(0x210)[0] = r.range(0, 9) ? r.real(-100, 100) : geo_pick(r);
                    },
                    [](void *fn) {
                        ((void(__fastcall *)(void *, void *, void *, void *, void *, void *, void *, void *))fn)(
                            arena<void>(0x200), arena<void>(0x204), arena<void>(0x208), arena<void>(0x100), arena<void>(0x110), arena<void>(0x120), arena<void>(0x20c),
                            arena<void>(0x210));
                        return Result{};
                    }};
    quick(water_fill);
    // Fence::QuadA91C80(n, records, indices, colour, 20 values)
    Test fence_quad{0xa91c80,
                    [=](Rng &r) {
                        for (int i = 0; i < 0x60; i++) arena<uint32_t>(0x400)[i] = r.next();
                        for (int i = 0; i < 24; i++) arena<uint32_t>(0x20)[i] = r.next();
                        arena<int32_t>(0x20)[0] = r.range(0, 2);
                    },
                    [](void *fn) {
                        const uint32_t *a = arena<uint32_t>(0x20);
                        ((void(__stdcall *)(int32_t, void *, void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
                                            uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))fn)(
                            (int32_t)a[0], arena<void>(0x400), arena<void>(0x5c0), a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9], a[10], a[11], a[12], a[13], a[14],
                            a[15], a[16], a[17], a[18], a[19], a[20], a[21]);
                        return Result{};
                    }};
    quick(fence_quad);
    // Record::Fill9CD960(this, point, rows, value): rows with signalling NaNs now and then
    Test record_fill{0x9cd960,
                     [=](Rng &r) {
                         for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x100)[i] = r.next();
                         for (int i = 0; i < 0x40; i++) arena<float>(0x200)[i] = r.real(-50, 50);
                         if (r.range(0, 3) == 0) arena<uint32_t>(0x200)[r.range(0, 0x3f)] = 0x7f800001u + r.range(0, 99);
                         for (int i = 0; i < 3; i++) arena<uint32_t>(0x300)[i] = r.next();
                         arena<uint32_t>(0x10)[0] = r.next();
                     },
                     [](void *fn) { ((void(__thiscall *)(void *, void *, void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x300), arena<void>(0x200), arena<uint32_t>(0x10)[0]); return Result{}; }};
    quick(record_fill);
    // Animal::ClampA5EDC0(this): values within, beyond, at the limits, NaNs
    Test animal_clamp{0xa5edc0,
                      [=](Rng &r) {
                          for (int k = 0; k < 3; k++) {
                              const int pick = r.range(0, 6);
                              arena<float>(0x100)[k] = pick < 3 ? r.real(-3000, 3000) : pick == 3 ? 1000.0f : pick == 4 ? -1000.0f : geo_pick(r);
                              if (pick == 6) arena<uint32_t>(0x100)[k] = r.range(0, 1) ? 0x7f800001u + r.range(0, 99) : r.next();
                          }
                      },
                      [](void *fn) { ((void(__thiscall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(animal_clamp);
    // Waterfall::Set919430(this, a, b, c, d, e)
    Test waterfall_set{0x919430,
                       [=](Rng &r) {
                           for (int i = 0; i < 0xd0; i++) arena<uint32_t>(0x100)[i] = r.next();
                           for (int i = 0; i < 0x30; i++) arena<float>(0x500)[i] = r.real(-50, 50);
                           if (r.range(0, 3) == 0) arena<uint32_t>(0x500)[r.range(0, 0x2f)] = 0x7f800001u + r.range(0, 99);
                       },
                       [](void *fn) {
                           ((void(__thiscall *)(void *, void *, void *, void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x500), arena<void>(0x510), arena<void>(0x520),
                                                                                                   arena<void>(0x540), arena<void>(0x5a0));
                           return Result{};
                       }};
    quick(waterfall_set);
    // TileCorners::Pack(this, c1..c8): two-bit values
    Test tile_corners_pack{0x727890,
                           [=](Rng &r) { for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next(); for (int i = 0; i < 8; i++) arena<uint32_t>(0x20)[i] = r.range(0, 3); },
                           [](void *fn) {
                               const uint32_t *c = arena<uint32_t>(0x20);
                               ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), c[0], c[1], c[2],
                                                                                                                                                   c[3], c[4], c[5], c[6], c[7]);
                               return Result{};
                           }};
    quick(tile_corners_pack);
    // Scenery::Normal87A370(out, a, b, t): two records, sometimes with no direction (a length of 0)
    Test scenery_normal{0x87a370,
                        [=](Rng &r) {
                            for (int i = 0; i < 18; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-40, 40) : geo_pick(r);
                            if (r.range(0, 4) == 0) for (int i = 3; i < 6; i++) arena<float>(0x100)[i] = arena<float>(0x100)[9 + i] = 0.0f;
                            for (int i = 0; i < 9; i++) arena<uint32_t>(0x200)[i] = r.next();
                            arena<float>(0x10)[0] = r.range(0, 5) ? r.real(0, 1) : geo_pick(r);
                        },
                        [](void *fn) { ((void(__fastcall *)(void *, void *, void *, float))fn)(arena<void>(0x200), arena<void>(0x100), arena<void>(0x124), arena<float>(0x10)[0]); return Result{}; }};
    quick(scenery_normal);
    // Vector::NormalBEDC00(this): three rows, sometimes row 2 or the cross product 0
    Test vector_normal_bedc00{0xbedc00,
                              [=](Rng &r) {
                                  for (int i = 0; i < 12; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-3, 3) : geo_pick(r);
                                  if (r.range(0, 5) == 0) for (int i = 8; i < 11; i++) arena<float>(0x100)[i] = 0.0f;
                                  if (r.range(0, 5) == 0) for (int i = 4; i < 7; i++) arena<float>(0x100)[i] = 0.0f;
                              },
                              [](void *fn) { return Result{(uint32_t)(((float *(__thiscall *)(void *))fn)(arena<void>(0x100)) == arena<float>(0x100))}; }};
    quick(vector_normal_bedc00);
    // Particles::TransformC(this): the shared origin and matrix (game globals, set for the case and put back), a list
    // of four entries, n near the count
    static uint32_t particle_globals_saved[16];
    Test particles_transform_c{0x9cdba0,
                               [=](Rng &r) {
                                   for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next();
                                   for (int i = 0; i < 3; i++) arena<float>(0x110)[i] = r.range(0, 9) ? r.real(-90, 90) : geo_pick(r);
                                   arena<uint32_t>(0x130)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                                   arena<int32_t>(0x104)[0] = r.range(1, 4), arena<int32_t>(0x134)[0] = r.range(0, 3), arena<int32_t>(0x138)[0] = r.range(0, 5);
                                   for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x400)[i] = r.next();
                                   for (int i = 0; i < 16; i++) arena<float>(0x40)[i] = r.range(0, 9) ? r.real(-3, 3) : geo_pick(r);
                               },
                               [](void *fn) {
                                   for (int i = 0; i < 16; i++) particle_globals_saved[i] = ((volatile uint32_t *)0x1341580)[i], ((volatile uint32_t *)0x1341580)[i] = arena<uint32_t>(0x40)[i];
                                   ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                                   for (int i = 0; i < 16; i++) ((volatile uint32_t *)0x1341580)[i] = particle_globals_saved[i];
                                   return Result{};
                               }};
    quick(particles_transform_c);
    // Vector::NormalBEE460(out, v): small and larger turn vectors (under the small value: the identity)
    Test vector_normal_bee460{0xbee460,
                              [=](Rng &r) {
                                  const float scale = r.range(0, 4) == 0 ? 1e-4f : r.real(0, 4);
                                  for (int i = 0; i < 3; i++) arena<float>(0x10)[i] = r.range(0, 9) ? scale * r.real(-1, 1) : geo_pick(r);
                                  for (int i = 0; i < 12; i++) arena<uint32_t>(0x100)[i] = r.next();
                              },
                              [](void *fn) { return Result{(uint32_t)(((float *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x10)) == arena<float>(0x100))}; }};
    quick(vector_normal_bee460);
    // Ride::Case7AD8A0(code): codes in and around 0x80-0x100, any other
    Test ride_case{0x7ad8a0, [=](Rng &r) { arena<uint32_t>(0x10)[0] = r.range(0, 5) ? (uint32_t)r.range(0x78, 0x108) : r.next(); },
                   [](void *fn) { return Result{((uint32_t(__fastcall *)(uint32_t))fn)(arena<uint32_t>(0x10)[0])}; }};
    quick(ride_case);
    // Matrices::Get(this, out, n): a list of three matrices, n inside or outside it
    Test matrices_get{0xceefb0,
                      [=](Rng &r) {
                          for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x400)[i] = r.next();
                          for (int i = 0; i < 16; i++) arena<uint32_t>(0x200)[i] = r.next();
                          arena<int32_t>(0x12c)[0] = 3, arena<uint32_t>(0x130)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                          arena<int32_t>(0x10)[0] = r.range(-2, 4);
                      },
                      [](void *fn) { ((void(__thiscall *)(void *, void *, int32_t))fn)(arena<void>(0x100), arena<void>(0x200), arena<int32_t>(0x10)[0]); return Result{}; }};
    quick(matrices_get);
    // Object::Matrix(this, out): a part or none
    Test object_matrix{0x4cc210,
                       [=](Rng &r) {
                           for (int i = 0; i < 0x30; i++) arena<uint32_t>(0x600)[i] = r.next();
                           for (int i = 0; i < 16; i++) arena<uint32_t>(0x200)[i] = r.next();
                           arena<uint32_t>(0x51c)[0] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x600) : 0;
                       },
                       [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200)); return Result{}; }};
    quick(object_matrix);
    // HandleRef::FromRef(this, from): the record copied from, its two objects with their lists (one node each) and the
    // counted value
    Test handle_ref_from_ref{0x98ac70,
                             [=](Rng &r) {
                                 for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x100)[i] = r.next();
                                 for (int i = 0; i < 0x80; i++) arena<uint32_t>(0x200)[i] = r.next();
                                 uint32_t *from = arena<uint32_t>(0x200);
                                 from[1] = (uint32_t)(uintptr_t)arena<void>(0x300), from[6] = (uint32_t)(uintptr_t)arena<void>(0x380), from[8] = (uint32_t)(uintptr_t)arena<void>(0x3a0);
                                 arena<uint32_t>(0x320)[0] = (uint32_t)(uintptr_t)arena<void>(0x340), arena<uint32_t>(0x340)[0] = (uint32_t)(uintptr_t)arena<void>(0x360);
                                 arena<uint32_t>(0x3c0)[0] = (uint32_t)(uintptr_t)arena<void>(0x3e0), arena<uint32_t>(0x3e0)[0] = (uint32_t)(uintptr_t)arena<void>(0x3f0);
                                 arena<uint32_t>(0x308)[0] = r.range(0, 9), arena<uint32_t>(0x3a8)[0] = r.range(0, 9), arena<uint32_t>(0x380)[0] = r.range(0, 9);
                             },
                             [](void *fn) { return Result{(uint32_t)(((uint32_t *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200)) == arena<uint32_t>(0x100))}; }};
    quick(handle_ref_from_ref);
    // TrackImporter::MapPieceCode(code, flag): codes 0-9, flags with other upper bits
    Test track_importer_map_piece_code{0xa3fab0,
                                       [=](Rng &r) { arena<int32_t>(0x10)[0] = r.range(0, 9), arena<uint32_t>(0x10)[1] = r.range(0, 1) ? r.next() & ~0xffu : r.next(); },
                                       [](void *fn) { return Result{(uint32_t)((int32_t(__stdcall *)(int32_t, uint32_t))fn)(arena<int32_t>(0x10)[0], arena<uint32_t>(0x10)[1])}; }};
    quick(track_importer_map_piece_code);
    // Object59F270::Object59F270(this, m, a, b, c, point, flag)
    Test object_59f270{0x59f270,
                       [=](Rng &r) {
                           for (int i = 0; i < 0x30; i++) arena<uint32_t>(0x100)[i] = r.next();
                           for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x200)[i] = r.next();
                           for (int i = 0; i < 5; i++) arena<uint32_t>(0x10)[i] = r.next();
                       },
                       [](void *fn) {
                           const uint32_t *a = arena<uint32_t>(0x10);
                           return Result{(uint32_t)(((uint32_t *(__thiscall *)(void *, void *, uint32_t, uint32_t, uint32_t, void *, uint32_t))fn)(
                                                        arena<void>(0x100), arena<void>(0x200), a[0], a[1], a[2], arena<void>(0x240), a[3]) == arena<uint32_t>(0x100))};
                       }};
    quick(object_59f270);
    // Rect::SideCell(this, n, x, y): sides 0-3 and others, n inside and past the length
    Test rect_side_cell{0xbad250,
                        [=](Rng &r) {
                            for (int i = 0; i < 0x80; i++) arena<uint32_t>(0x100)[i] = r.next();
                            arena<uint32_t>(0x2e4)[0] = r.range(0, 5), arena<int32_t>(0x2ec)[0] = r.range(0, 9), arena<int32_t>(0x10)[0] = r.range(-2, 10);
                            arena<uint32_t>(0x20)[0] = r.next();
                        },
                        [](void *fn) {
                            ((void(__thiscall *)(void *, int32_t, void *, void *))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<uint8_t>(0x20), arena<uint8_t>(0x21));
                            return Result{};
                        }};
    quick(rect_side_cell);
    // Object::SetHeld(this, on): switched on, off, or left; the partner (+0x104)
    Test object_set_held{0xc5e9f0,
                         [=](Rng &r) {
                             for (int i = 0; i < 0x50; i++) arena<uint32_t>(0x100)[i] = r.next();
                             for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x300)[i] = r.next();
                             arena<uint32_t>(0x204)[0] = (uint32_t)(uintptr_t)arena<void>(0x300);
                             arena<uint8_t>(0x210)[0] = (uint8_t)r.range(0, 2);
                             arena<uint32_t>(0x10)[0] = (r.next() & ~0xffu) | (uint32_t)r.range(0, 2);
                         },
                         [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0]); return Result{}; }};
    quick(object_set_held);
    // Grid::CurvedOffsets(grid, x, a, b, y): grid sizes (including 1: a division by 0), places along it
    Test grid_curved_offsets{0x7c3370,
                             [=](Rng &r) {
                                 for (int i = 0; i < 8; i++) arena<uint32_t>(0x100)[i] = r.next();
                                 arena<uint8_t>(0x104)[0] = (uint8_t)r.range(1, 40), arena<uint8_t>(0x105)[0] = (uint8_t)r.range(1, 40);
                                 arena<float>(0x10)[0] = r.range(0, 9) ? r.real(0, 40) : geo_pick(r), arena<float>(0x10)[1] = r.range(0, 9) ? r.real(0, 40) : geo_pick(r);
                                 arena<uint32_t>(0x200)[0] = r.next(), arena<uint32_t>(0x204)[0] = r.next();
                             },
                             [](void *fn) {
                                 ((void(__fastcall *)(void *, void *, float, float, void *))fn)(arena<void>(0x100), arena<void>(0x200), arena<float>(0x10)[0], arena<float>(0x10)[1], arena<void>(0x204));
                                 return Result{};
                             }};
    quick(grid_curved_offsets);
    // Wobble::Update(this, -): phases above 2, between, at or below 0
    Test wobble_update{0x414270,
                       [=](Rng &r) {
                           for (int i = 0; i < 0x60; i++) arena<uint32_t>(0x100)[i] = r.next();
                           for (int i = 0; i < 3; i++) arena<float>(0x240)[i] = r.range(0, 9) ? r.real(-3, 3) : geo_pick(r);
                           arena<float>(0x250)[0] = r.range(0, 9) ? r.real(-1, 4) : geo_pick(r);
                       },
                       [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), 0); return Result{}; }};
    quick(wobble_update);
    // DistanceConstraint::DistanceConstraint(this, a, b, k, flag, w): two points (weights first, positions at +0x10),
    // weights that sometimes add to 0
    Test distance_constraint{0xa6b310,
                             [=](Rng &r) {
                                 for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next();
                                 for (int i = 0; i < 0x20; i++) arena<float>(0x200)[i] = r.range(0, 9) ? r.real(-40, 40) : geo_pick(r);
                                 if (r.range(0, 4) == 0) arena<float>(0x240)[0] = -arena<float>(0x200)[0];
                                 arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-5, 5) : geo_pick(r), arena<uint32_t>(0x10)[1] = r.next(), arena<uint32_t>(0x10)[2] = r.next();
                             },
                             [](void *fn) {
                                 return Result{(uint32_t)(((uint32_t *(__thiscall *)(void *, void *, void *, float, uint32_t, uint32_t))fn)(
                                                              arena<void>(0x100), arena<void>(0x200), arena<void>(0x240), arena<float>(0x10)[0], arena<uint32_t>(0x10)[1],
                                                              arena<uint32_t>(0x10)[2]) == arena<uint32_t>(0x100))};
                             }};
    quick(distance_constraint);
    // Grid::RotateCoords(this, x, y, x_out, y_out): rotations 0-5, a grid or none
    Test grid_rotate_coords{0x838c00,
                            [=](Rng &r) {
                                for (int i = 0; i < 0x60; i++) arena<uint32_t>(0x100)[i] = r.next();
                                arena<uint32_t>(0x26c)[0] = r.range(0, 5) ? (uint32_t)(uintptr_t)arena<void>(0x400) : 0;
                                arena<uint32_t>(0x27c)[0] = r.range(0, 5);
                                arena<int32_t>(0x410)[0] = r.range(1, 60), arena<int32_t>(0x414)[0] = r.range(1, 60);
                                arena<int32_t>(0x10)[0] = r.range(0, 60), arena<int32_t>(0x10)[1] = r.range(0, 60);
                                arena<uint32_t>(0x20)[0] = r.next(), arena<uint32_t>(0x24)[0] = r.next();
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(void *, int32_t, int32_t, void *, void *))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<int32_t>(0x10)[1], arena<void>(0x20), arena<void>(0x24));
                                return Result{};
                            }};
    quick(grid_rotate_coords);
    // Matrix::FacingRotation(out, facing): facings 0-5
    Test matrix_facing_rotation{0x585870,
                                [=](Rng &r) { for (int i = 0; i < 16; i++) arena<uint32_t>(0x100)[i] = r.next(); arena<uint32_t>(0x10)[0] = r.range(0, 5); },
                                [](void *fn) { ((void(__stdcall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0]); return Result{}; }};
    quick(matrix_facing_rotation);
    // Animation::FrameAt(this, step): an info with one track (frames and rate), times near the wrap
    Test animation_frame_at{0x911c80,
                            [=](Rng &r) {
                                for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x100)[i] = r.next();
                                arena<uint32_t>(0x130)[0] = (uint32_t)(uintptr_t)arena<void>(0x300);
                                arena<uint32_t>(0x308)[0] = (uint32_t)(uintptr_t)arena<void>(0x340);
                                arena<uint16_t>(0x370)[0] = 1, arena<uint32_t>(0x374)[0] = (uint32_t)(uintptr_t)arena<void>(0x3c0);
                                arena<uint32_t>(0x3c0)[0] = (uint32_t)(uintptr_t)arena<void>(0x3e0);
                                arena<int32_t>(0x3e4)[0] = r.range(1, 60), arena<float>(0x3e8)[0] = r.range(0, 9) ? r.real(1, 30) : geo_pick(r);
                                arena<float>(0x120)[0] = r.real(0, 2.2f);
                                arena<float>(0x10)[0] = r.range(0, 9) ? r.real(0, 0.5f) : geo_pick(r);
                            },
                            [](void *fn) { ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x10)[0]); return Result{}; }};
    quick(animation_frame_at);
    // AvatarEditor::Tick(this, time): the two timers, their periods, the two switches
    Test avatar_editor_tick{0x749120,
                            [=](Rng &r) {
                                for (int i = 0; i < 0x380; i++) arena<uint32_t>(0x100)[i] = r.next();
                                arena<float>(0xe0c)[0] = r.real(0, 10), arena<float>(0xe10)[0] = r.range(0, 9) ? r.real(0.5f, 6) : geo_pick(r), arena<float>(0xe18)[0] = r.real(0, 3);
                                arena<uint8_t>(0xe6c)[0] = (uint8_t)(r.range(0, 3) == 0), arena<uint32_t>(0x118)[0] = r.range(0, 3) ? 1 : 0;
                                arena<float>(0x10)[0] = r.range(0, 9) ? r.real(0, 2) : geo_pick(r);
                            },
                            [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x10)); return Result{}; }};
    quick(avatar_editor_tick);
    // Points::MidpointLength(a, b, n): two lists of three points
    Test points_midpoint_length{0x92dfd0,
                                [=](Rng &r) {
                                    arena<uint32_t>(0x104)[0] = (uint32_t)(uintptr_t)arena<void>(0x200), arena<uint32_t>(0x114)[0] = (uint32_t)(uintptr_t)arena<void>(0x300);
                                    for (int i = 0; i < 0x40; i++) arena<float>(0x200)[i] = r.range(0, 9) ? r.real(-80, 80) : geo_pick(r);
                                    arena<int32_t>(0x10)[0] = r.range(0, 2);
                                },
                                [](void *fn) {
                                    const float v = (float)((long double(__stdcall *)(void *, void *, int32_t))fn)(arena<void>(0x100), arena<void>(0x110), arena<int32_t>(0x10)[0]);
                                    uint32_t bits;
                                    memcpy(&bits, &v, 4);
                                    return Result{bits};
                                }};
    quick(points_midpoint_length);
    // Frame::Turn(this, angle): axis pairs, values at +0x30..+0x54 (signed zeros and NaNs now and then)
    Test frame_turn{0xbd4c90,
                    [=](Rng &r) {
                        for (int i = 0; i < 0x18; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-2, 2) : geo_pick(r);
                        arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-7, 7) : geo_pick(r);
                    },
                    [](void *fn) { ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x10)[0]); return Result{}; }};
    quick(frame_turn);
    // Matrix::InverseTransformPoint(m, out, p)
    Test matrix_inverse_transform_point{0x47d7d0,
                                        [=](Rng &r) {
                                            for (int i = 0; i < 16; i++) arena<float>(0x200)[i] = r.range(0, 9) ? r.real(-3, 3) : geo_pick(r);
                                            for (int i = 0; i < 3; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-60, 60) : geo_pick(r);
                                            for (int i = 0; i < 4; i++) arena<uint32_t>(0x140)[i] = r.next();
                                        },
                                        [](void *fn) { ((void(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x200), arena<void>(0x140), arena<void>(0x100)); return Result{}; }};
    quick(matrix_inverse_transform_point);
    // Effect::Set7B2730(this, point, r)
    Test effect_set{0x7b2730,
                    [=](Rng &r) {
                        for (int i = 0; i < 0x14; i++) arena<uint32_t>(0x100)[i] = r.next();
                        for (int i = 0; i < 3; i++) arena<float>(0x200)[i] = r.range(0, 9) ? r.real(-60, 60) : geo_pick(r);
                        arena<float>(0x10)[0] = r.range(0, 9) ? r.real(0, 20) : geo_pick(r);
                    },
                    [](void *fn) { ((void(__thiscall *)(void *, void *, float))fn)(arena<void>(0x100), arena<void>(0x200), arena<float>(0x10)[0]); return Result{}; }};
    quick(effect_set);
    // Table::Map16(n, flag): 0-7 and flags with other bits
    Test table_map16{0xa3fb60, [=](Rng &r) { arena<int32_t>(0x10)[0] = r.range(0, 7), arena<uint32_t>(0x10)[1] = r.range(0, 1) ? r.next() & ~0xffu : r.next(); },
                     [](void *fn) { return Result{(uint32_t)((int32_t(__stdcall *)(int32_t, uint32_t))fn)(arena<int32_t>(0x10)[0], arena<uint32_t>(0x10)[1])}; }};
    quick(table_map16);
    // Sound::SetVolumeScale(v): the current sound (the game global set for the call, sometimes none), its channel 0-3
    // with the channel tables' entries set for the call and put back
    static uint32_t sound_saved[9];
    Test sound_set_volume_scale{0x8f8660,
                                [=](Rng &r) {
                                    for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x100)[i] = r.next();
                                    arena<uint32_t>(0x118)[0] = r.range(0, 3), arena<uint32_t>(0x10c)[0] = (uint32_t)(uintptr_t)arena<void>(0x200);
                                    arena<uint32_t>(0x20)[0] = r.range(0, 4) ? (uint32_t)(uintptr_t)arena<void>(0x100) : 0;
                                    for (int i = 0; i < 4; i++) arena<uint32_t>(0x30)[i] = r.range(0, 2) == 0, arena<float>(0x40)[i] = r.real(0, 1);
                                    arena<float>(0x10)[0] = r.range(0, 9) ? r.real(0, 1.4f) : geo_pick(r);
                                },
                                [](void *fn) {
                                    sound_saved[0] = *(volatile uint32_t *)0x1339f58;
                                    for (int i = 0; i < 4; i++) sound_saved[1 + i] = ((volatile uint32_t *)0x12f0168)[i], sound_saved[5 + i] = ((volatile uint32_t *)0x10edd40)[i];
                                    *(volatile uint32_t *)0x1339f58 = arena<uint32_t>(0x20)[0];
                                    for (int i = 0; i < 4; i++) ((volatile uint32_t *)0x12f0168)[i] = arena<uint32_t>(0x30)[i], ((volatile uint32_t *)0x10edd40)[i] = arena<uint32_t>(0x40)[i];
                                    ((void(__stdcall *)(float))fn)(arena<float>(0x10)[0]);
                                    *(volatile uint32_t *)0x1339f58 = sound_saved[0];
                                    for (int i = 0; i < 4; i++) ((volatile uint32_t *)0x12f0168)[i] = sound_saved[1 + i], ((volatile uint32_t *)0x10edd40)[i] = sound_saved[5 + i];
                                    return Result{};
                                }};
    quick(sound_set_volume_scale);
    // Tabs::Animate(this, time): up to four tabs, one selected (or none of them)
    Test tabs_animate{0x61c280,
                      [=](Rng &r) {
                          for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x100)[i] = r.next();
                          arena<int32_t>(0x1e8)[0] = r.range(-1, 4), arena<uint32_t>(0x1e0)[0] = r.range(0, 5), arena<uint32_t>(0x1e4)[0] = (uint32_t)(uintptr_t)arena<void>(0x300);
                          for (int i = 0; i < 4; i++) arena<float>(0x300)[i] = r.range(0, 9) ? r.real(-120, 30) : geo_pick(r);
                          arena<float>(0x10)[0] = r.range(0, 9) ? r.real(0, 0.2f) : geo_pick(r);
                      },
                      [](void *fn) { ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x10)[0]); return Result{}; }};
    quick(tabs_animate);
    // Speed::Scale(this, k): factors below, at and above 1 (and near 1000), k near 1 and not
    Test speed_scale{0xa5fd90,
                     [=](Rng &r) {
                         for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next();
                         arena<float>(0x120)[0] = r.range(0, 9) ? r.real(0.5f, 1100) : geo_pick(r);
                         arena<float>(0x10)[0] = r.range(0, 4) == 0 ? 1.0f + r.real(-1e-4f, 1e-4f) : r.range(0, 9) ? r.real(0, 3) : geo_pick(r);
                     },
                     [](void *fn) { ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x10)[0]); return Result{}; }};
    quick(speed_scale);
    // Blink::Due(this, once): periods from the game's table (entries 0-3), timers under and past them
    Test blink_due{0x757eb0,
                   [=](Rng &r) {
                       for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x100)[i] = r.next();
                       arena<uint32_t>(0x1b8)[0] = r.range(0, 3);
                       arena<float>(0x1bc)[0] = r.range(0, 9) ? r.real(0, 8) : geo_pick(r);
                       arena<uint32_t>(0x10)[0] = (r.next() & ~0xffu) | (uint32_t)r.range(0, 1);
                   },
                   [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0]) & 0xff}; }};
    quick(blink_due);
    // Triangle::FromBase(out, base, direction, side)
    Test triangle_from_base{0xc1c2e0,
                            [=](Rng &r) {
                                for (int i = 0; i < 6; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-50, 50) : geo_pick(r);
                                for (int i = 0; i < 6; i++) arena<uint32_t>(0x200)[i] = r.next();
                            },
                            [](void *fn) { ((void(__thiscall *)(void *, void *, void *, void *))fn)(arena<void>(0x200), arena<void>(0x100), arena<void>(0x108), arena<void>(0x110)); return Result{}; }};
    quick(triangle_from_base);
    // Cursor::MoveByScreen(this, delta): the game's guard and scale globals set for the call (first use or not) and put
    // back, the screen object (0x12F83F0) the arena's
    static uint32_t cursor_saved[5];
    Test cursor_move_by_screen{0x502520,
                               [=](Rng &r) {
                                   for (int i = 0; i < 0x10; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-5, 5) : geo_pick(r);
                                   for (int i = 0; i < 0x24; i++) arena<float>(0x300)[i] = r.real(0, 900);
                                   arena<float>(0x10)[0] = r.real(-30, 30), arena<float>(0x10)[1] = r.real(-30, 30);
                                   arena<uint32_t>(0x20)[0] = r.next(), arena<uint32_t>(0x20)[1] = (uint32_t)r.range(0, 1);
                                   arena<float>(0x20)[2] = r.real(0, 0.1f), arena<float>(0x20)[3] = r.real(0, 0.1f);
                               },
                               [](void *fn) {
                                   const uint32_t places[5] = {0x12f506c, 0x12f5068, 0x12f5060, 0x12f5064, 0x12f83f0};
                                   for (int i = 0; i < 5; i++) cursor_saved[i] = *(volatile uint32_t *)(uintptr_t)places[i];
                                   for (int i = 0; i < 4; i++) *(volatile uint32_t *)(uintptr_t)places[i] = arena<uint32_t>(0x20)[i];
                                   *(volatile uint32_t *)0x12f83f0 = (uint32_t)(uintptr_t)arena<void>(0x300);
                                   ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x10));
                                   uint32_t after = 0;
                                   for (int i = 0; i < 4; i++) after = after * 31 + *(volatile uint32_t *)(uintptr_t)places[i];
                                   for (int i = 0; i < 5; i++) *(volatile uint32_t *)(uintptr_t)places[i] = cursor_saved[i];
                                   return Result{after};
                               }};
    quick(cursor_move_by_screen);
    // Grid3::Rotate / RotateBack(x, y, rotation, x_out, y_out): rotations 0-5
    auto grid3_case = [=](Rng &r) { arena<int32_t>(0x10)[0] = r.range(-1, 3), arena<int32_t>(0x10)[1] = r.range(-1, 3), arena<uint32_t>(0x10)[2] = r.range(0, 5), arena<uint32_t>(0x20)[0] = r.next(), arena<uint32_t>(0x20)[1] = r.next(); };
    auto grid3_call = [](void *fn) {
        const int32_t *a = arena<int32_t>(0x10);
        ((void(__stdcall *)(int32_t, int32_t, uint32_t, void *, void *))fn)(a[0], a[1], (uint32_t)a[2], arena<void>(0x20), arena<void>(0x24));
        return Result{};
    };
    Test grid3_rotate{0x8750c0, grid3_case, grid3_call};
    quick(grid3_rotate);
    Test grid3_rotate_back{0x875020, grid3_case, grid3_call};
    quick(grid3_rotate_back);
    // Segment::MiddleOfEnds2(this, out): two lists of one to three points
    Test segment_middle_of_ends2{0x5bcee0,
                                 [=](Rng &r) {
                                     arena<uint32_t>(0x12c)[0] = (uint32_t)(uintptr_t)arena<void>(0x200), arena<uint32_t>(0x128)[0] = (uint32_t)(uintptr_t)arena<void>(0x210);
                                     arena<uint32_t>(0x200)[0] = r.range(1, 3), arena<uint32_t>(0x204)[0] = (uint32_t)(uintptr_t)arena<void>(0x300);
                                     arena<uint32_t>(0x210)[0] = r.range(1, 3), arena<uint32_t>(0x214)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                                     for (int i = 0; i < 0x40; i++) arena<float>(0x300)[i] = r.range(0, 9) ? r.real(-80, 80) : geo_pick(r);
                                     for (int i = 0; i < 4; i++) arena<uint32_t>(0x500)[i] = r.next();
                                 },
                                 [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x500)); return Result{}; }};
    quick(segment_middle_of_ends2);
    // Frame::Flip(this): values with signalling NaNs now and then, the flag 0 or not
    Test frame_flip{0x50bd80,
                    [=](Rng &r) {
                        for (int i = 0; i < 0x40; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-9, 9) : geo_pick(r);
                        if (r.range(0, 3) == 0) arena<uint32_t>(0x1d0)[r.range(0, 6)] = 0x7f800001u + r.range(0, 99);
                        arena<uint8_t>(0x10c)[0] = (uint8_t)(r.range(0, 1) ? 0 : r.range(1, 255));
                    },
                    [](void *fn) { ((void(__thiscall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(frame_flip);
    // Terrain::SlopeShade(this, x, y): a 4x4 grid of cells, height differences giving greys below, inside and above
    Test terrain_slope_shade{0x6b6400,
                             [=](Rng &r) {
                                 arena<uint8_t>(0x104)[0] = 4, arena<uint32_t>(0x124)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                                 for (int i = 0; i < 0x60; i++) arena<float>(0x400)[i] = r.range(0, 9) ? r.real(-20, 20) : geo_pick(r);
                                 arena<uint32_t>(0x10)[0] = (r.next() & ~0xffu) | (uint32_t)r.range(0, 3), arena<uint32_t>(0x10)[1] = (r.next() & ~0xffu) | (uint32_t)r.range(0, 3);
                             },
                             [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], arena<uint32_t>(0x10)[1])}; }};
    quick(terrain_slope_shade);
    // CameraAngles::Clamp2(this)
    Test camera_angles_clamp2{0xa5f530,
                              [=](Rng &r) {
                                  for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next();
                                  const int pick = r.range(0, 6);
                                  arena<float>(0x120)[0] = pick < 3 ? r.real(-3, 3) : pick == 3 ? 1.5677f : pick == 4 ? -1.5677f : geo_pick(r);
                                  if (pick == 6) arena<uint32_t>(0x120)[0] = r.range(0, 1) ? 0x7f800001u + r.range(0, 99) : r.next();
                                  arena<float>(0x124)[0] = r.range(0, 5) ? r.real(-40, 40) : geo_pick(r);
                              },
                              [](void *fn) { ((void(__thiscall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(camera_angles_clamp2);
    // UnitSquare::Rotate(x, y, rotation, x_out, y_out): rotations 0-5, signalling NaNs now and then
    Test unit_square_rotate{0x875160,
                            [=](Rng &r) {
                                arena<float>(0x10)[0] = r.range(0, 9) ? r.real(0, 1) : geo_pick(r), arena<float>(0x10)[1] = r.range(0, 9) ? r.real(0, 1) : geo_pick(r);
                                if (r.range(0, 5) == 0) arena<uint32_t>(0x10)[r.range(0, 1)] = 0x7f800001u + r.range(0, 99);
                                arena<uint32_t>(0x18)[0] = r.range(0, 5), arena<uint32_t>(0x20)[0] = r.next(), arena<uint32_t>(0x24)[0] = r.next();
                            },
                            [](void *fn) {
                                ((void(__stdcall *)(float, float, uint32_t, void *, void *))fn)(arena<float>(0x10)[0], arena<float>(0x10)[1], arena<uint32_t>(0x18)[0], arena<void>(0x20), arena<void>(0x24));
                                return Result{};
                            }};
    quick(unit_square_rotate);
    // Window::OutsideEllipse(this, p): window sizes, points inside and outside, +0xD4
    Test window_outside_ellipse{0x63a920,
                                [=](Rng &r) {
                                    for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x100)[i] = r.next();
                                    arena<float>(0x17c)[0] = r.real(0, 50), arena<float>(0x180)[0] = r.real(0, 50), arena<float>(0x184)[0] = r.real(60, 300), arena<float>(0x188)[0] = r.real(60, 300);
                                    arena<float>(0x1d4)[0] = r.range(0, 9) ? r.real(0, 0.5f) : geo_pick(r);
                                    arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-50, 350) : geo_pick(r), arena<float>(0x10)[1] = r.real(-50, 350);
                                },
                                [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x10))}; }};
    quick(window_outside_ellipse);
    // Window::ArcA45670(this, out, angle, r, h)
    Test window_arc{0xa45670,
                    [=](Rng &r) {
                        for (int i = 0; i < 0x40; i++) arena<float>(0x100)[i] = r.real(0, 900);
                        arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-7, 7) : geo_pick(r), arena<float>(0x10)[1] = r.real(0, 200), arena<float>(0x10)[2] = r.range(0, 9) ? r.real(0, 300) : geo_pick(r);
                        arena<uint32_t>(0x20)[0] = r.next(), arena<uint32_t>(0x24)[0] = r.next();
                    },
                    [](void *fn) {
                        const float *a = arena<float>(0x10);
                        ((void(__thiscall *)(void *, void *, float, float, float))fn)(arena<void>(0x100), arena<void>(0x20), a[0], a[1], a[2]);
                        return Result{};
                    }};
    quick(window_arc);
    // Request::Request(this, owner, a, b, c, flag, value, d, e): the owner's list with one node, the game's request count
    // (set for the call, sometimes about to wrap, and put back)
    static uint32_t request_saved;
    Test request_request{0xc51870,
                         [=](Rng &r) {
                             for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next();
                             arena<uint32_t>(0x210)[0] = (uint32_t)(uintptr_t)arena<void>(0x300), arena<uint32_t>(0x300)[0] = (uint32_t)(uintptr_t)arena<void>(0x340);
                             arena<uint32_t>(0x340)[1] = r.next();
                             for (int i = 0; i < 8; i++) arena<uint32_t>(0x20)[i] = r.next();
                             arena<uint32_t>(0x40)[0] = r.range(0, 3) ? r.range(0, 1000) : 0xffffffffu;
                         },
                         [](void *fn) {
                             request_saved = *(volatile uint32_t *)0x13e6c94;
                             *(volatile uint32_t *)0x13e6c94 = arena<uint32_t>(0x40)[0];
                             const uint32_t *a = arena<uint32_t>(0x20);
                             uint32_t *got = ((uint32_t *(__thiscall *)(void *, void *, uint32_t, uint32_t, uint32_t, uint32_t, void *, uint32_t, uint32_t))fn)(
                                 arena<void>(0x100), arena<void>(0x200), a[0], a[1], a[2], a[3], arena<void>(0x30), a[5], a[6]);
                             const uint32_t after = *(volatile uint32_t *)0x13e6c94;
                             *(volatile uint32_t *)0x13e6c94 = request_saved;
                             return Result{after * 2 + (uint32_t)(got == arena<uint32_t>(0x100))};
                         }};
    quick(request_request);
    // Sound::DistanceFade(this): the listener (game global, set for the call and put back), a voice with near and far
    static uint32_t listener_saved[3];
    Test sound_distance_fade{0x47bd90,
                             [=](Rng &r) {
                                 arena<uint32_t>(0x10c)[0] = (uint32_t)(uintptr_t)arena<void>(0x200);
                                 for (int i = 0; i < 3; i++) arena<float>(0x260)[i] = r.real(-80, 80), arena<float>(0x20)[i] = r.real(-80, 80);
                                 arena<float>(0x280)[0] = r.range(0, 9) ? r.real(0, 40) : geo_pick(r), arena<float>(0x284)[0] = r.range(0, 9) ? r.real(30, 150) : geo_pick(r);
                             },
                             [](void *fn) {
                                 for (int i = 0; i < 3; i++) listener_saved[i] = ((volatile uint32_t *)0x10f4620)[i], ((volatile uint32_t *)0x10f4620)[i] = arena<uint32_t>(0x20)[i];
                                 const float v = (float)((long double(__thiscall *)(void *))fn)(arena<void>(0x100));
                                 for (int i = 0; i < 3; i++) ((volatile uint32_t *)0x10f4620)[i] = listener_saved[i];
                                 uint32_t bits;
                                 memcpy(&bits, &v, 4);
                                 return Result{bits};
                             }};
    quick(sound_distance_fade);
    // Light::FadeAlpha(this, out, x, y)
    Test light_fade_alpha{0x959f30,
                          [=](Rng &r) {
                              for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x100)[i] = r.next();
                              arena<float>(0x13c)[0] = r.real(-50, 50), arena<float>(0x140)[0] = r.real(-50, 50), arena<float>(0x144)[0] = r.range(0, 9) ? r.real(0, 20) : geo_pick(r);
                              arena<float>(0x148)[0] = r.range(0, 9) ? r.real(10, 60) : geo_pick(r), arena<float>(0x138)[0] = r.range(0, 9) ? r.real(0, 1.5f) : geo_pick(r);
                              arena<float>(0x10)[0] = r.real(-80, 80), arena<float>(0x10)[1] = r.real(-80, 80);
                          },
                          [](void *fn) { ((void(__thiscall *)(void *, void *, float, float))fn)(arena<void>(0x100), arena<void>(0x20), arena<float>(0x10)[0], arena<float>(0x10)[1]); return Result{}; }};
    quick(light_fade_alpha);
    // Slider::SetValue(this, fraction)
    Test slider_set_value{0xca6f90,
                          [=](Rng &r) {
                              arena<uint32_t>(0x104)[0] = (uint32_t)(uintptr_t)arena<void>(0x200), arena<uint32_t>(0x200)[0] = r.next();
                              arena<float>(0x108)[0] = r.real(-50, 50), arena<float>(0x10c)[0] = r.range(0, 9) ? r.real(50, 300) : geo_pick(r), arena<float>(0x110)[0] = r.range(0, 9) ? r.real(0.5f, 10) : geo_pick(r);
                              arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-0.2f, 1.2f) : geo_pick(r);
                          },
                          [](void *fn) { ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x10)[0]); return Result{}; }};
    quick(slider_set_value);
    // Matrix::Copy3x4(from, to): any bits, signalling NaNs among them
    Test matrix_copy3x4{0xb1f750,
                        [=](Rng &r) {
                            for (int i = 0; i < 12; i++) arena<uint32_t>(0x100)[i] = r.range(0, 3) ? r.next() : 0x7f800001u + r.range(0, 99);
                            for (int i = 0; i < 12; i++) arena<uint32_t>(0x200)[i] = r.next();
                        },
                        [](void *fn) { ((void(__fastcall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200)); return Result{}; }};
    quick(matrix_copy3x4);
    // Record::Copy485690(this, from): the shared part's count
    Test record_copy_485690{0x485690,
                            [=](Rng &r) {
                                for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x100)[i] = r.next(), arena<uint32_t>(0x200)[i] = r.next();
                                arena<uint32_t>(0x200)[0] = (uint32_t)(uintptr_t)arena<void>(0x300), arena<uint32_t>(0x300)[0] = r.range(0, 9);
                            },
                            [](void *fn) { return Result{(uint32_t)(((uint32_t *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200)) == arena<uint32_t>(0x100))}; }};
    quick(record_copy_485690);
    // Object3CC90::Object3CC90(this, owner, m)
    Test object_3cc90{0x58a6c0,
                      [=](Rng &r) { for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x100)[i] = r.next(), arena<uint32_t>(0x200)[i] = r.next(); arena<uint32_t>(0x10)[0] = r.next(); },
                      [](void *fn) {
                          return Result{(uint32_t)(((uint32_t *(__thiscall *)(void *, uint32_t, void *))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], arena<void>(0x200)) == arena<uint32_t>(0x100))};
                      }};
    quick(object_3cc90);
    // Object::SetMatrixAndMore(this, -, m, v)
    Test object_set_matrix_and_more{0xbbd210,
                                    [=](Rng &r) { for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x100)[i] = r.next(), arena<uint32_t>(0x200)[i] = r.next(); },
                                    [](void *fn) { ((void(__thiscall *)(void *, uint32_t, void *, void *))fn)(arena<void>(0x100), 0, arena<void>(0x200), arena<void>(0x240)); return Result{}; }};
    quick(object_set_matrix_and_more);
    // Range::Clip(this, out, other): overlapping and separate ranges
    Test range_clip{0xc517a0,
                    [=](Rng &r) { for (int i = 0; i < 4; i++) arena<int32_t>(0x100)[i] = r.range(0, 5) ? r.range(-40, 40) : (int32_t)r.next(); arena<uint32_t>(0x200)[0] = r.next(), arena<uint32_t>(0x204)[0] = r.next(); },
                    [](void *fn) { ((void(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x200), arena<void>(0x108)); return Result{}; }};
    quick(range_clip);
    // Footprint::RotateCell(add_x, add_y, x, y, x_out, y_out, turn, footprint): turns 0-5
    Test footprint_rotate_cell{0x8275d0,
                               [=](Rng &r) {
                                   for (int i = 0; i < 8; i++) arena<int32_t>(0x20)[i] = r.range(-9, 9);
                                   arena<uint32_t>(0x20)[6] = r.range(0, 5);
                                   arena<int32_t>(0x110)[0] = r.range(1, 9), arena<int32_t>(0x114)[0] = r.range(1, 9);
                                   arena<int32_t>(0x200)[0] = r.range(-9, 9), arena<int32_t>(0x204)[0] = r.range(-9, 9);
                               },
                               [](void *fn) {
                                   const int32_t *a = arena<int32_t>(0x20);
                                   ((void(__fastcall *)(int32_t, int32_t, int32_t, int32_t, void *, void *, uint32_t, void *))fn)(a[0], a[1], a[2], a[3], arena<void>(0x200), arena<void>(0x204),
                                                                                                                                   (uint32_t)a[6], arena<void>(0x100));
                                   return Result{};
                               }};
    quick(footprint_rotate_cell);
    // Curve::LogBlend(a, b, x): two points either way round, x between and beyond
    Test curve_log_blend{0xce4920,
                         [=](Rng &r) {
                             arena<float>(0x100)[0] = r.real(-50, 50), arena<float>(0x100)[1] = r.real(0, 10);
                             arena<float>(0x110)[0] = r.real(-50, 50), arena<float>(0x110)[1] = r.range(0, 9) ? r.real(0, 10) : geo_pick(r);
                             arena<float>(0x10)[0] = r.range(0, 9) ? r.real(0, 10) : geo_pick(r);
                         },
                         [](void *fn) {
                             const float v = (float)((long double(__fastcall *)(void *, void *, float))fn)(arena<void>(0x100), arena<void>(0x110), arena<float>(0x10)[0]);
                             uint32_t bits;
                             memcpy(&bits, &v, 4);
                             return Result{bits};
                         }};
    quick(curve_log_blend);
    // Columns::Hit(this, n, p): a list of four columns, points near them
    Test columns_hit{0x884890,
                     [=](Rng &r) {
                         for (int i = 0; i < 0x50; i++) arena<uint32_t>(0x100)[i] = r.next();
                         arena<float>(0x17c)[0] = r.real(0, 40), arena<float>(0x184)[0] = r.real(100, 500), arena<float>(0x1dc)[0] = r.range(0, 9) ? r.real(1, 10) : geo_pick(r);
                         arena<uint32_t>(0x1fc)[0] = (uint32_t)(uintptr_t)arena<void>(0x300);
                         for (int i = 0; i < 4; i++) arena<float>(0x300)[i] = r.real(0, 10);
                         arena<int32_t>(0x10)[0] = r.range(0, 3), arena<float>(0x20)[0] = r.range(0, 9) ? r.real(0, 500) : geo_pick(r);
                     },
                     [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, int32_t, void *))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<void>(0x20))}; }};
    quick(columns_hit);
    // Vector::NormaliseFast(v): tiny, ordinary and odd vectors
    Test vector_normalise_fast{0x94ca00,
                               [=](Rng &r) {
                                   const float scale = r.range(0, 4) == 0 ? 1e-4f : r.real(0, 30);
                                   for (int i = 0; i < 3; i++) arena<float>(0x100)[i] = r.range(0, 9) ? scale * r.real(-1, 1) : geo_pick(r);
                               },
                               [](void *fn) { ((void(__thiscall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(vector_normalise_fast);
    // Ease::Out(t, total): totals under 2 and over, t near the end and not
    Test ease_out{0x5d3970,
                  [=](Rng &r) {
                      const float total = r.range(0, 9) ? r.real(0, 12) : geo_pick(r);
                      arena<float>(0x10)[0] = r.range(0, 9) ? total * r.real(0, 1) : geo_pick(r), arena<float>(0x10)[1] = total;
                  },
                  [](void *fn) {
                      const float v = (float)((long double(__stdcall *)(float, float))fn)(arena<float>(0x10)[0], arena<float>(0x10)[1]);
                      uint32_t bits;
                      memcpy(&bits, &v, 4);
                      return Result{bits};
                  }};
    quick(ease_out);
    // Colour::FromARGB(out, colour): both copies
    for (const uint32_t at : {0x454850u, 0x435a40u}) {
        Test colour_from_argb{at,
                              [=](Rng &r) { for (int i = 0; i < 4; i++) arena<uint32_t>(0x100)[i] = r.next(); arena<uint32_t>(0x10)[0] = r.next(); },
                              [](void *fn) {
                                  return Result{(uint32_t)(((float *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0]) == arena<float>(0x100))};
                              }};
        quick(colour_from_argb);
    }
    // Segment::Middle2(this, out)
    Test segment_middle2{0x5bce60,
                         [=](Rng &r) {
                             arena<uint32_t>(0x12c)[0] = (uint32_t)(uintptr_t)arena<void>(0x200), arena<uint32_t>(0x128)[0] = (uint32_t)(uintptr_t)arena<void>(0x220);
                             arena<uint32_t>(0x204)[0] = (uint32_t)(uintptr_t)arena<void>(0x300), arena<uint32_t>(0x224)[0] = (uint32_t)(uintptr_t)arena<void>(0x340);
                             for (int i = 0; i < 3; i++) arena<float>(0x300)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r), arena<float>(0x340)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r);
                             for (int i = 0; i < 3; i++) arena<uint32_t>(0x20)[i] = r.next();
                         },
                         [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x20)); return Result{}; }};
    quick(segment_middle2);
    // Colour::Blend(this, t, other)
    Test colour_blend{0x5bc0b0,
                      [=](Rng &r) {
                          for (int i = 0; i < 4; i++) arena<int32_t>(0x100)[i] = r.range(0, 9) ? r.range(-300, 300) : (int32_t)r.next(), arena<int32_t>(0x200)[i] = r.range(0, 9) ? r.range(-300, 300) : (int32_t)r.next();
                          arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-0.2f, 1.2f) : geo_pick(r);
                      },
                      [](void *fn) { ((void(__thiscall *)(void *, float, void *))fn)(arena<void>(0x100), arena<float>(0x10)[0], arena<void>(0x200)); return Result{}; }};
    quick(colour_blend);
    // Curve::LinearBlend(a, b, x)
    Test curve_linear_blend{0xce47b0,
                            [=](Rng &r) {
                                arena<float>(0x100)[0] = r.real(0, 10), arena<float>(0x100)[1] = r.range(0, 9) ? r.real(-50, 50) : geo_pick(r);
                                arena<float>(0x110)[0] = r.range(0, 9) ? r.real(0, 10) : geo_pick(r), arena<float>(0x110)[1] = r.range(0, 9) ? r.real(-50, 50) : geo_pick(r);
                                if (r.range(0, 9) == 0) arena<uint32_t>(0x100)[1 + 4 * r.range(0, 1)] = 0x7f800001u + r.range(0, 99);
                                arena<float>(0x10)[0] = r.range(0, 9) ? r.real(0, 10) : geo_pick(r);
                            },
                            [](void *fn) {
                                const float v = (float)((long double(__fastcall *)(void *, void *, float))fn)(arena<void>(0x100), arena<void>(0x110), arena<float>(0x10)[0]);
                                uint32_t bits;
                                memcpy(&bits, &v, 4);
                                return Result{bits};
                            }};
    quick(curve_linear_blend);
    // Average::AddCell(this, cell)
    Test average_add_cell{0x7c32f0,
                          [=](Rng &r) {
                              arena<float>(0x2d0)[0] = r.range(0, 9) ? (float)r.range(0, 50) : geo_pick(r), arena<float>(0x2d4)[0] = r.real(0, 255), arena<float>(0x2d8)[0] = r.range(0, 9) ? r.real(0, 255) : geo_pick(r);
                              arena<uint32_t>(0x188)[0] = r.next();
                          },
                          [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x100)); return Result{}; }};
    quick(average_add_cell);
    // Object::PartMatrix(this, out)
    Test object_part_matrix{0xa637e0,
                            [=](Rng &r) {
                                arena<uint32_t>(0x1e0)[0] = (uint32_t)(uintptr_t)arena<void>(0x300);
                                for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x300)[i] = r.next(), arena<uint32_t>(0x20)[i] = r.next();
                            },
                            [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x20)); return Result{}; }};
    quick(object_part_matrix);
    // Window::PartFor(this, n): numbers 0-12
    Test window_part_for{0x619890,
                         [=](Rng &r) { for (int i = 0; i < 0x30; i++) arena<uint32_t>(0x200)[i] = r.next(); arena<uint32_t>(0x10)[0] = r.range(0, 12); },
                         [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0])}; }};
    quick(window_part_for);
    // IntSlider::SetValue(this, fraction) and SetValue2: ranges either way round, fractions in and out of 0-1
    for (const uint32_t at : {0xca77b0u, 0xca7620u}) {
        Test int_slider_set_value{at,
                                  [=](Rng &r) {
                                      arena<uint32_t>(0x104)[0] = (uint32_t)(uintptr_t)arena<void>(0x200), arena<uint32_t>(0x200)[0] = r.next();
                                      arena<int32_t>(0x108)[0] = r.range(0, 5) ? r.range(-100, 100) : (int32_t)r.next(), arena<int32_t>(0x10c)[0] = r.range(0, 5) ? r.range(-100, 300) : (int32_t)r.next();
                                      arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-0.2f, 1.2f) : geo_pick(r);
                                  },
                                  [](void *fn) { ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x10)[0]); return Result{}; }};
        quick(int_slider_set_value);
    }
    // Vector2::SnapToGrid(out, p, spacing)
    Test vector2_snap_to_grid{0x617290,
                              [=](Rng &r) {
                                  arena<float>(0x100)[0] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r), arena<float>(0x100)[1] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r);
                                  arena<float>(0x10)[0] = r.range(0, 9) ? r.real(0.5f, 40) : geo_pick(r), arena<uint32_t>(0x20)[0] = r.next(), arena<uint32_t>(0x24)[0] = r.next();
                              },
                              [](void *fn) { ((void(__fastcall *)(void *, void *, float))fn)(arena<void>(0x20), arena<void>(0x100), arena<float>(0x10)[0]); return Result{}; }};
    quick(vector2_snap_to_grid);
    // Vector::LerpPoint(out, a, b, t)
    Test vector_lerp_point{0x4259f0,
                           [=](Rng &r) {
                               for (int i = 0; i < 3; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r), arena<float>(0x110)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r);
                               arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-0.2f, 1.2f) : geo_pick(r);
                               for (int i = 0; i < 4; i++) arena<uint32_t>(0x20)[i] = r.next();
                           },
                           [](void *fn) { ((void(__fastcall *)(void *, void *, void *, float))fn)(arena<void>(0x20), arena<void>(0x100), arena<void>(0x110), arena<float>(0x10)[0]); return Result{}; }};
    quick(vector_lerp_point);
    // Sound::SetLevels(this, a, b): levels in and out of 0-1, NaNs (signalling too)
    Test sound_set_levels{0x47dd10,
                          [=](Rng &r) {
                              for (int i = 0; i < 2; i++) {
                                  arena<float>(0x10)[i] = r.range(0, 9) ? r.real(-0.5f, 1.5f) : geo_pick(r);
                                  if (r.range(0, 9) == 0) arena<uint32_t>(0x10)[i] = 0x7f800001u + r.range(0, 99);
                              }
                              arena<uint32_t>(0x15c)[0] = r.next(), arena<uint32_t>(0x160)[0] = r.next();
                          },
                          [](void *fn) { ((void(__thiscall *)(void *, float, float))fn)(arena<void>(0x100), arena<float>(0x10)[0], arena<float>(0x10)[1]); return Result{}; }};
    quick(sound_set_levels);
    // Matrix::ScaleOffset2D(this, origin, scale)
    Test matrix_scale_offset_2d{0x4668d0,
                                [=](Rng &r) {
                                    for (int i = 0; i < 16; i++) arena<uint32_t>(0x100)[i] = r.next();
                                    for (int i = 0; i < 2; i++) arena<float>(0x200)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r), arena<float>(0x210)[i] = r.range(0, 9) ? r.real(-4, 4) : geo_pick(r);
                                },
                                [](void *fn) { ((void(__fastcall *)(void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x200), arena<void>(0x210)); return Result{}; }};
    quick(matrix_scale_offset_2d);
    // Record::CopyCC6200(this, from)
    Test record_copy_cc6200{0xcc6200,
                            [=](Rng &r) { arena<uint32_t>(0x104)[0] = (uint32_t)(uintptr_t)arena<void>(0x300); for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x300)[i] = r.next(), arena<uint32_t>(0x200)[i] = r.next(); },
                            [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200)); return Result{}; }};
    quick(record_copy_cc6200);
    // Heap::Swap(this, n): eight entries in the shared table (game global, set for the call and put back), a list of five
    static uint32_t heap_table_saved;
    Test heap_swap{0x49a740,
                   [=](Rng &r) {
                       arena<uint32_t>(0x40)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                       for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x400)[i] = r.next();
                       for (int i = 0; i < 8; i++) arena<uint32_t>(0x40c)[4 * i] = r.range(0, 7);
                       arena<uint32_t>(0x110)[0] = r.range(1, 8), arena<uint32_t>(0x114)[0] = (uint32_t)(uintptr_t)arena<void>(0x200), arena<uint32_t>(0x118)[0] = (uint32_t)(uintptr_t)arena<void>(0x280);
                       for (int i = 0; i < 8; i++) arena<uint32_t>(0x200)[i] = r.next(), arena<uint32_t>(0x280)[i] = r.range(0, 7);
                       arena<uint32_t>(0x10)[0] = r.range(0, 7);
                   },
                   [](void *fn) {
                       heap_table_saved = *(volatile uint32_t *)0x12f46dc;
                       *(volatile uint32_t *)0x12f46dc = (uint32_t)(uintptr_t)arena<void>(0x40);
                       ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0]);
                       *(volatile uint32_t *)0x12f46dc = heap_table_saved;
                       return Result{};
                   }};
    quick(heap_swap);
    // VerletPoint::SetVelocity(this, v)
    Test verlet_point_set_velocity{0xa6b2a0,
                                   [=](Rng &r) {
                                       for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next();
                                       for (int i = 0; i < 3; i++) arena<float>(0x110)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r), arena<float>(0x200)[i] = r.range(0, 9) ? r.real(-50, 50) : geo_pick(r);
                                   },
                                   [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200)); return Result{}; }};
    quick(verlet_point_set_velocity);
    // Vector::TransformPointInPlace(p, m)
    Test vector_transform_point_in_place{0x4bb400,
                                         [=](Rng &r) {
                                             for (int i = 0; i < 16; i++) arena<float>(0x200)[i] = r.range(0, 9) ? r.real(-3, 3) : geo_pick(r);
                                             for (int i = 0; i < 3; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r);
                                         },
                                         [](void *fn) { ((void(__fastcall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200)); return Result{}; }};
    quick(vector_transform_point_in_place);
    // Vector::Lerp(out, a, b, t), Vector::Lerp2 and Vector::LerpMarked (this, a, b, t on the stack)
    for (const uint32_t at : {0xce16b0u, 0x466a60u, 0xa67e40u}) {
        Test vector_lerp{at,
                         [=](Rng &r) {
                             for (int i = 0; i < 3; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r), arena<float>(0x110)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r);
                             arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-0.2f, 1.2f) : geo_pick(r);
                             for (int i = 0; i < 5; i++) arena<uint32_t>(0x20)[i] = r.next();
                         },
                         [at](void *fn) {
                             if (at == 0xa67e40u)
                                 ((void(__thiscall *)(void *, void *, void *, float))fn)(arena<void>(0x20), arena<void>(0x100), arena<void>(0x110), arena<float>(0x10)[0]);
                             else
                                 ((void(__fastcall *)(void *, void *, void *, float))fn)(arena<void>(0x20), arena<void>(0x100), arena<void>(0x110), arena<float>(0x10)[0]);
                             return Result{};
                         }};
        quick(vector_lerp);
    }
    // ResourceRefFA57D0 / FA5798 constructors (this, owner, b, c, d, e, kind): the owner's list with one node
    for (const uint32_t at : {0xbebee0u, 0xbeb9a0u}) {
        Test resource_ref{at,
                          [=](Rng &r) {
                              for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next();
                              arena<uint32_t>(0x200 + (at == 0xbebee0u ? 0x244 : 0x214))[0] = (uint32_t)(uintptr_t)arena<void>(0x500);
                              arena<uint32_t>(0x500)[0] = (uint32_t)(uintptr_t)arena<void>(0x540), arena<uint32_t>(0x540)[1] = r.next();
                              for (int i = 0; i < 5; i++) arena<uint32_t>(0x20)[i] = r.next();
                          },
                          [](void *fn) {
                              const uint32_t *a = arena<uint32_t>(0x20);
                              return Result{(uint32_t)(((uint32_t *(__thiscall *)(void *, void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<void>(0x200), a[0], a[1],
                                                                                                                                                a[2], a[3], a[4]) == arena<uint32_t>(0x100))};
                          }};
        quick(resource_ref);
    }
    // VerletPoint::Interpolated(this, out): the global fraction set for the call and put back
    static uint32_t verlet_fraction_saved;
    Test verlet_point_interpolated{0xa67ee0,
                                   [=](Rng &r) {
                                       for (int i = 4; i < 11; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r);
                                       arena<float>(0x10)[0] = r.range(0, 9) ? r.real(0, 1) : geo_pick(r);
                                       for (int i = 0; i < 3; i++) arena<uint32_t>(0x20)[i] = r.next();
                                   },
                                   [](void *fn) {
                                       verlet_fraction_saved = *(volatile uint32_t *)0x13429e0;
                                       *(volatile float *)0x13429e0 = arena<float>(0x10)[0];
                                       ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x20));
                                       *(volatile uint32_t *)0x13429e0 = verlet_fraction_saved;
                                       return Result{};
                                   }};
    quick(verlet_point_interpolated);
    // Motion::DistanceAt(t, v0, v1, T)
    Test motion_distance_at{0xa88680,
                            [=](Rng &r) {
                                arena<int32_t>(0x10)[0] = r.range(0, 9) ? r.range(-5, 300) : (int32_t)r.next();
                                for (int i = 1; i < 3; i++) arena<float>(0x10)[i] = r.range(0, 9) ? r.real(-10, 40) : geo_pick(r);
                                arena<float>(0x10)[3] = r.range(0, 9) ? r.real(0, 200) : geo_pick(r);
                                if (r.range(0, 19) == 0) arena<uint32_t>(0x10)[3] = 0x7f800001u + r.range(0, 99);
                            },
                            [](void *fn) {
                                const float *a = arena<float>(0x10);
                                const float v = (float)((long double(__stdcall *)(int32_t, float, float, float))fn)(arena<int32_t>(0x10)[0], a[1], a[2], a[3]);
                                uint32_t bits;
                                memcpy(&bits, &v, 4);
                                return Result{bits};
                            }};
    quick(motion_distance_at);
    // Terrain::CornerPosition(this, out, x, y, corner): a 6x6 grid, corners 0-3 and odd ones
    Test terrain_corner_position{0x8c0100,
                                 [=](Rng &r) {
                                     arena<uint8_t>(0x104)[0] = 6;
                                     for (int i = 3; i < 7; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r);
                                     arena<uint32_t>(0x124)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                                     for (int i = 0; i < 0x120; i++) arena<uint32_t>(0x400)[i] = r.next();
                                     arena<uint32_t>(0x10)[0] = r.range(0, 5) | (r.next() & 0xffffff00u), arena<uint32_t>(0x10)[1] = r.range(0, 5) | (r.next() & 0xffffff00u);
                                     arena<uint32_t>(0x10)[2] = (r.range(0, 9) ? r.range(0, 3) : r.range(-3, 7) & 0xff) | (r.next() & 0xffffff00u);
                                     for (int i = 0; i < 3; i++) arena<uint32_t>(0x20)[i] = r.next();
                                 },
                                 [](void *fn) {
                                     const uint32_t *a = arena<uint32_t>(0x10);
                                     ((void(__thiscall *)(void *, void *, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<void>(0x20), a[0], a[1], a[2]);
                                     return Result{};
                                 }};
    quick(terrain_corner_position);
    // Ease::In(t, total)
    Test ease_in{0x5d3900,
                 [=](Rng &r) {
                     const float total = r.range(0, 9) ? r.real(0, 12) : geo_pick(r);
                     arena<float>(0x10)[0] = r.range(0, 9) ? total * r.real(0, 1) : geo_pick(r), arena<float>(0x10)[1] = total;
                 },
                 [](void *fn) {
                     const float v = (float)((long double(__stdcall *)(float, float))fn)(arena<float>(0x10)[0], arena<float>(0x10)[1]);
                     uint32_t bits;
                     memcpy(&bits, &v, 4);
                     return Result{bits};
                 }};
    quick(ease_in);
    // Colour::Pack(this, c)
    Test colour_pack{0x9e84e0,
                     [=](Rng &r) {
                         for (int i = 0; i < 4; i++) arena<float>(0x200)[i] = r.range(0, 9) ? r.real(-0.2f, 1.2f) : geo_pick(r);
                         arena<uint32_t>(0x108)[0] = r.next();
                     },
                     [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200)); return Result{}; }};
    quick(colour_pack);
    // Tile::EdgePoint(out, x, y, direction): directions 0-9 and 16
    Test tile_edge_point{0xabc300,
                         [=](Rng &r) {
                             arena<int32_t>(0x10)[0] = r.range(0, 9) ? r.range(-300, 300) : (int32_t)r.next(), arena<int32_t>(0x10)[1] = r.range(0, 9) ? r.range(-300, 300) : (int32_t)r.next();
                             arena<uint32_t>(0x10)[2] = r.range(0, 5) ? 1u << r.range(0, 3) : r.range(0, 9) == 0 ? 16u : (uint32_t)r.range(0, 9);
                             arena<uint32_t>(0x20)[0] = r.next(), arena<uint32_t>(0x20)[1] = r.next();
                         },
                         [](void *fn) {
                             const int32_t *a = arena<int32_t>(0x10);
                             ((void(__fastcall *)(void *, int32_t, int32_t, uint32_t))fn)(arena<void>(0x20), a[0], a[1], (uint32_t)a[2]);
                             return Result{};
                         }};
    quick(tile_edge_point);
    // WildAnimal::ClampB42390(this): flagged and not, NaNs (signalling too)
    Test wild_animal_clamp{0xb42390,
                           [=](Rng &r) {
                               arena<uint8_t>(0x1ec)[0] = (uint8_t)r.next();
                               for (int i = 0; i < 3; i++) {
                                   arena<float>(0x2e0)[i] = r.range(0, 9) ? r.real(-5, 5) : geo_pick(r);
                                   if (r.range(0, 12) == 0) arena<uint32_t>(0x2e0)[i] = 0x7f800001u + r.range(0, 99);
                               }
                               arena<uint32_t>(0x248)[0] = r.next();
                           },
                           [](void *fn) { ((void(__thiscall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(wild_animal_clamp);
    // Counts::AddByKind(this, thing, level): kinds 0-0x2C, levels around 11.1
    Test counts_add_by_kind{0x5bc150,
                            [=](Rng &r) {
                                for (int i = 0; i < 12; i++) arena<uint32_t>(0x100)[i] = r.range(0, 99);
                                arena<uint32_t>(0x210)[0] = r.range(0, 9) ? r.range(0, 0x2c) : r.next();
                                arena<float>(0x10)[0] = r.range(0, 9) ? r.real(10, 12.2f) : geo_pick(r);
                            },
                            [](void *fn) { ((void(__thiscall *)(void *, void *, float))fn)(arena<void>(0x100), arena<void>(0x200), arena<float>(0x10)[0]); return Result{}; }};
    quick(counts_add_by_kind);
    // Guest::EntryRemaining(this, n): three entries, in use or not, totals of any sign
    Test guest_entry_remaining{0xb9ab80,
                               [=](Rng &r) {
                                   arena<uint32_t>(0x174)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                                   for (int i = 0; i < 3; i++) {
                                       uint8_t *e = arena<uint8_t>(0x420 + 0xb8 * i);
                                       e[0] = r.range(0, 2) ? (uint8_t)r.range(1, 255) : 0;
                                       *(int32_t *)(e + 0xc) = r.range(0, 5) ? r.range(-5, 100) : (int32_t)r.next(), *(int32_t *)(e + 0x14) = r.range(0, 5) ? r.range(-10, 150) : (int32_t)r.next();
                                   }
                                   arena<int32_t>(0x10)[0] = r.range(0, 2);
                               },
                               [](void *fn) {
                                   const float v = (float)((long double(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0]);
                                   uint32_t bits;
                                   memcpy(&bits, &v, 4);
                                   return Result{bits};
                               }};
    quick(guest_entry_remaining);
    // Path::Case7289D0(this, a_out, b_out): kinds 0-7
    Test path_case_7289d0{0x7289d0,
                          [=](Rng &r) {
                              arena<uint8_t>(0x160)[0] = (uint8_t)r.range(0, 7), arena<int32_t>(0x1a4)[0] = r.range(0, 5) ? r.range(-100, 100) : (int32_t)r.next();
                              arena<uint32_t>(0x20)[0] = r.next(), arena<uint32_t>(0x24)[0] = r.next();
                          },
                          [](void *fn) { ((void(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x20), arena<void>(0x24)); return Result{}; }};
    quick(path_case_7289d0);
    // WaterDrops::EmitterCount(this, n): intervals of any sign, NaNs
    Test water_drops_emitter_count{0x94e000,
                                   [=](Rng &r) {
                                       arena<uint32_t>(0x2a4)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                                       for (int i = 0; i < 3; i++) {
                                           float *e = arena<float>(0x400 + 0xd0 * i);
                                           e[3] = r.range(0, 9) ? r.real(-1, 5) : geo_pick(r), e[4] = r.range(0, 9) ? r.real(0, 100) : geo_pick(r);
                                       }
                                       arena<int32_t>(0x10)[0] = r.range(0, 2);
                                   },
                                   [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0])}; }};
    quick(water_drops_emitter_count);
    // Object::KindCode(this): kinds 0-11, values around the cap
    Test object_kind_code{0xaad7f0,
                          [=](Rng &r) { arena<uint32_t>(0x220)[0] = r.range(0, 9) ? r.range(0, 11) : r.next(), arena<int32_t>(0x224)[0] = r.range(0, 5) ? r.range(-40, 60) : (int32_t)r.next(); },
                          [](void *fn) { return Result{((uint32_t(__thiscall *)(void *))fn)(arena<void>(0x100))}; }};
    quick(object_kind_code);
    // Object::SetKind(this, kind): kinds -3 to 12
    Test object_set_kind{0xa93990,
                         [=](Rng &r) { arena<int32_t>(0x10)[0] = r.range(0, 9) ? r.range(-3, 12) : (int32_t)r.next(); arena<uint32_t>(0x240)[0] = r.next(); },
                         [](void *fn) { ((void(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0]); return Result{}; }};
    quick(object_set_kind);
    // Water::Entry94DD40(this, n, level, v, w): modes 0-2, levels of any sign
    Test water_entry_94dd40{0x94dd40,
                            [=](Rng &r) {
                                arena<uint32_t>(0x2a4)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                                for (int i = 0; i < 0x68; i++) arena<uint32_t>(0x400)[i] = r.next();
                                for (int i = 0; i < 2; i++) arena<uint32_t>(0x460 + 0xd0 * i)[0] = r.range(0, 2);
                                arena<int32_t>(0x10)[0] = r.range(0, 1), arena<float>(0x14)[0] = r.range(0, 9) ? r.real(-5, 5) : geo_pick(r);
                                for (int i = 0; i < 4; i++) arena<uint32_t>(0x20)[i] = r.next();
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(void *, int32_t, float, void *, uint32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<float>(0x14)[0], arena<void>(0x20), arena<uint32_t>(0x2c)[0]);
                                return Result{};
                            }};
    quick(water_entry_94dd40);
    // Effect::Fraction466470(this, near, far, angle, aspect)
    Test effect_fraction_466470{0x466470,
                                [=](Rng &r) {
                                    for (int i = 0; i < 16; i++) arena<uint32_t>(0x100)[i] = r.next();
                                    arena<float>(0x10)[0] = r.range(0, 9) ? r.real(0.01f, 5) : geo_pick(r), arena<float>(0x10)[1] = r.range(0, 9) ? r.real(10, 5000) : geo_pick(r);
                                    arena<float>(0x10)[2] = r.range(0, 9) ? r.real(0.2f, 2.5f) : geo_pick(r), arena<float>(0x10)[3] = r.range(0, 9) ? r.real(0.5f, 2.5f) : geo_pick(r);
                                },
                                [](void *fn) {
                                    const float *a = arena<float>(0x10);
                                    ((void(__thiscall *)(void *, float, float, float, float))fn)(arena<void>(0x100), a[0], a[1], a[2], a[3]);
                                    return Result{};
                                }};
    quick(effect_fraction_466470);
    // Animations::Set(this, n, a, b, count, rate)
    Test animations_set{0x94dec0,
                        [=](Rng &r) {
                            arena<uint32_t>(0x2a4)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                            for (int i = 0; i < 0x68; i++) arena<uint32_t>(0x400)[i] = r.next();
                            arena<int32_t>(0x10)[0] = r.range(0, 1), arena<uint32_t>(0x14)[0] = r.next(), arena<uint32_t>(0x18)[0] = r.next();
                            arena<int32_t>(0x1c)[0] = r.range(0, 9) ? r.range(-2, 60) : (int32_t)r.next(), arena<float>(0x20)[0] = r.range(0, 9) ? r.real(1, 60) : geo_pick(r);
                        },
                        [](void *fn) {
                            const uint32_t *a = arena<uint32_t>(0x10);
                            ((void(__thiscall *)(void *, int32_t, uint32_t, uint32_t, int32_t, float))fn)(arena<void>(0x100), (int32_t)a[0], a[1], a[2], (int32_t)a[3], arena<float>(0x20)[0]);
                            return Result{};
                        }};
    quick(animations_set);
    // Tracker::SetVelocity(this, p)
    Test tracker_set_velocity{0x8b5230,
                              [=](Rng &r) {
                                  for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next();
                                  for (int i = 0; i < 3; i++) arena<float>(0x11c)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r), arena<float>(0x200)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r);
                              },
                              [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200)); return Result{}; }};
    quick(tracker_set_velocity);
    // VerletPoint::Velocity(this, out)
    Test verlet_point_velocity{0xa6b240,
                               [=](Rng &r) {
                                   for (int i = 4; i < 11; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r);
                                   for (int i = 0; i < 3; i++) arena<uint32_t>(0x20)[i] = r.next();
                               },
                               [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x20)); return Result{}; }};
    quick(verlet_point_velocity);
    // PoolComplex::ValueAAE350(this, flag): modes 0-9
    Test pool_complex_value{0xaae350,
                            [=](Rng &r) { arena<int32_t>(0x224)[0] = r.range(0, 5) ? r.range(-5, 9) : (int32_t)r.next(), arena<uint32_t>(0x230)[0] = r.range(0, 9); arena<uint32_t>(0x10)[0] = r.next() & 0x1ff; },
                            [](void *fn) {
                                const float v = (float)((long double(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0]);
                                uint32_t bits;
                                memcpy(&bits, &v, 4);
                                return Result{bits};
                            }};
    quick(pool_complex_value);
    // Mesh::PointAndNormal(this, point, position, normal): no mesh, a negative point, two parts of four points
    Test mesh_point_and_normal{0x489a10,
                               [=](Rng &r) {
                                   arena<uint32_t>(0x100)[0] = r.range(0, 9) ? (uint32_t)(uintptr_t)arena<void>(0x200) : 0;
                                   arena<uint32_t>(0x208)[0] = (uint32_t)(uintptr_t)arena<void>(0x300);
                                   arena<uint32_t>(0x318)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<uint32_t>(0x334)[0] = (uint32_t)(uintptr_t)arena<void>(0x500);
                                   for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x400)[i] = r.next(), arena<uint32_t>(0x500)[i] = r.next();
                                   arena<int32_t>(0x14c)[0] = r.range(0, 9) ? r.range(0, 9) : -r.range(1, 9);
                                   arena<uint32_t>(0x150)[0] = (uint32_t)r.range(0, 1) << 16 | r.range(0, 3);
                                   for (int i = 0; i < 6; i++) arena<uint32_t>(0x20)[i] = r.next();
                               },
                               [](void *fn) { ((void(__thiscall *)(void *, void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x140), arena<void>(0x20), arena<void>(0x2c)); return Result{}; }};
    quick(mesh_point_and_normal);
    // Vector::LerpInto(a, b, t, out)
    Test vector_lerp_into{0xb47a30,
                          [=](Rng &r) {
                              for (int i = 0; i < 3; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r), arena<float>(0x110)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r);
                              arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-0.2f, 1.2f) : geo_pick(r);
                              for (int i = 0; i < 3; i++) arena<uint32_t>(0x20)[i] = r.next();
                          },
                          [](void *fn) { ((void(__fastcall *)(void *, void *, float, void *))fn)(arena<void>(0x100), arena<void>(0x110), arena<float>(0x10)[0], arena<void>(0x20)); return Result{}; }};
    quick(vector_lerp_into);
    // Progress::Advance2(this, dt)
    Test progress_advance2{0xb34fa0,
                           [=](Rng &r) { arena<float>(0x238)[0] = r.range(0, 9) ? r.real(0, 1.1f) : geo_pick(r), arena<float>(0x10)[0] = r.range(0, 9) ? r.real(0, 3) : geo_pick(r); },
                           [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x10)[0])}; }};
    quick(progress_advance2);
    // Vector4::Add(this, v)
    Test vector4_add{0xcc4890,
                     [=](Rng &r) { for (int i = 0; i < 4; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r), arena<float>(0x200)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r); },
                     [](void *fn) { return Result{(uint32_t)(((float *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200)) == arena<float>(0x100))}; }};
    quick(vector4_add);
    // Fireworks::AddTime9E0D70(this, n): the game's limit set for the call and put back
    static uint32_t firework_limit_saved;
    Test fireworks_add_time{0x9e0d70,
                            [=](Rng &r) {
                                arena<int32_t>(0x13c)[0] = r.range(0, 5) ? r.range(-500, 5000) : (int32_t)r.next(), arena<int32_t>(0x128)[0] = r.range(0, 9) ? r.range(0, 500) : (int32_t)r.next();
                                arena<float>(0x114)[0] = r.range(0, 9) ? r.real(0, 50) : geo_pick(r), arena<float>(0x15c)[0] = r.range(0, 9) ? r.real(0.5f, 9) : geo_pick(r);
                                arena<int32_t>(0x10)[0] = r.range(0, 9) ? r.range(-20, 40) : (int32_t)r.next(), arena<int32_t>(0x14)[0] = r.range(0, 9) ? r.range(-5, 4000) : (int32_t)r.next();
                                arena<uint32_t>(0x158)[0] = r.next();
                            },
                            [](void *fn) {
                                firework_limit_saved = *(volatile uint32_t *)0x10f0e80;
                                *(volatile int32_t *)0x10f0e80 = arena<int32_t>(0x14)[0];
                                ((void(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0]);
                                *(volatile uint32_t *)0x10f0e80 = firework_limit_saved;
                                return Result{};
                            }};
    quick(fireworks_add_time);
    // Colour::ToARGB(out, r, g, b, a)
    Test colour_to_argb{0x402c40,
                        [=](Rng &r) { for (int i = 0; i < 4; i++) arena<float>(0x10)[i] = r.range(0, 9) ? r.real(-0.2f, 1.2f) : geo_pick(r); arena<uint32_t>(0x100)[0] = r.next(); },
                        [](void *fn) {
                            const float *a = arena<float>(0x10);
                            return Result{(uint32_t)(((uint32_t *(__thiscall *)(void *, float, float, float, float))fn)(arena<void>(0x100), a[0], a[1], a[2], a[3]) == arena<uint32_t>(0x100))};
                        }};
    quick(colour_to_argb);
    // Screen::ToUnit(this, out, p)
    Test screen_to_unit{0x483f40,
                        [=](Rng &r) {
                            arena<float>(0x17c)[0] = r.real(0, 50), arena<float>(0x180)[0] = r.real(0, 50), arena<float>(0x184)[0] = r.range(0, 9) ? r.real(100, 2000) : geo_pick(r), arena<float>(0x188)[0] = r.real(100, 2000);
                            arena<float>(0x10)[0] = r.range(0, 9) ? r.real(0, 2000) : geo_pick(r), arena<float>(0x10)[1] = r.real(0, 2000);
                            arena<uint32_t>(0x20)[0] = r.next(), arena<uint32_t>(0x24)[0] = r.next();
                        },
                        [](void *fn) { ((void(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x20), arena<void>(0x10)); return Result{}; }};
    quick(screen_to_unit);
    // FloatSlider::SetValue, SetValue4, SetValue8 (this, fraction)
    for (const uint32_t at : {0xca7a30u, 0xca7c00u, 0xca7dd0u}) {
        Test float_slider_set_value{at,
                                    [=](Rng &r) {
                                        arena<uint32_t>(0x104)[0] = (uint32_t)(uintptr_t)arena<void>(0x200);
                                        for (int i = 0; i < 3; i++) arena<uint32_t>(0x200)[i] = r.next();
                                        arena<float>(0x108)[0] = r.range(0, 9) ? r.real(-50, 50) : geo_pick(r), arena<float>(0x10c)[0] = r.range(0, 9) ? r.real(50, 300) : geo_pick(r);
                                        arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-0.2f, 1.2f) : geo_pick(r);
                                    },
                                    [](void *fn) { ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x10)[0]); return Result{}; }};
        quick(float_slider_set_value);
    }
    // Colour::PackArgs(out, r, g, b, a)
    Test colour_pack_args{0xb96f90,
                          [=](Rng &r) { for (int i = 0; i < 4; i++) arena<float>(0x10)[i] = r.range(0, 9) ? r.real(-0.2f, 1.2f) : geo_pick(r); arena<uint32_t>(0x100)[0] = r.next(); },
                          [](void *fn) {
                              const float *a = arena<float>(0x10);
                              return Result{((uint32_t(__thiscall *)(void *, float, float, float, float))fn)(arena<void>(0x100), a[0], a[1], a[2], a[3])};
                          }};
    quick(colour_pack_args);
    // Colour::PackTo(c, out)
    Test colour_pack_to{0x425700,
                        [=](Rng &r) { for (int i = 0; i < 4; i++) arena<float>(0x10)[i] = r.range(0, 9) ? r.real(-0.2f, 1.2f) : geo_pick(r); arena<uint32_t>(0x100)[0] = r.next(); },
                        [](void *fn) { return Result{(uint32_t)(((uint32_t *(__thiscall *)(void *, void *))fn)(arena<void>(0x10), arena<void>(0x100)) == arena<uint32_t>(0x100))}; }};
    quick(colour_pack_to);
    // Spring::Constants(this, a_out, b_out, c_out): with and without c_out
    Test spring_constants{0x912c20,
                          [=](Rng &r) {
                              arena<float>(0x104)[0] = r.range(0, 9) ? r.real(1, 50) : geo_pick(r), arena<float>(0x10c)[0] = r.range(0, 9) ? r.real(-1, 0.9f) : geo_pick(r);
                              for (int i = 0; i < 3; i++) arena<uint32_t>(0x20)[i] = r.next();
                              arena<uint32_t>(0x10)[0] = r.range(0, 2);
                          },
                          [](void *fn) {
                              ((void(__thiscall *)(void *, void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x20), arena<void>(0x24), arena<uint32_t>(0x10)[0] ? arena<void>(0x28) : nullptr);
                              return Result{};
                          }};
    quick(spring_constants);
    // Vector::Lerp3(out, a, b, -, t)
    Test vector_lerp3{0xc5a860,
                      [=](Rng &r) {
                          for (int i = 0; i < 3; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r), arena<float>(0x110)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r);
                          arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-0.2f, 1.2f) : geo_pick(r);
                          for (int i = 0; i < 3; i++) arena<uint32_t>(0x20)[i] = r.next();
                      },
                      [](void *fn) {
                          ((void(__thiscall *)(void *, void *, void *, uint32_t, float))fn)(arena<void>(0x20), arena<void>(0x100), arena<void>(0x110), 0, arena<float>(0x10)[0]);
                          return Result{};
                      }};
    quick(vector_lerp3);
    // Particle::Launch(this, p, flag, a, b, h): signalling NaNs now and then
    Test particle_launch{0xa17520,
                         [=](Rng &r) {
                             for (int i = 0; i < 0x28; i++) arena<uint32_t>(0x100)[i] = r.next();
                             for (int i = 0; i < 3; i++) arena<float>(0x200)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r);
                             if (r.range(0, 9) == 0) arena<uint32_t>(0x200)[r.range(0, 2)] = 0x7f800001u + r.range(0, 99);
                             for (int i = 0; i < 3; i++) arena<uint32_t>(0x10)[i] = r.next();
                             arena<float>(0x1c)[0] = r.range(0, 9) ? r.real(0, 20) : geo_pick(r);
                         },
                         [](void *fn) {
                             const uint32_t *a = arena<uint32_t>(0x10);
                             ((void(__thiscall *)(void *, void *, uint32_t, uint32_t, uint32_t, float))fn)(arena<void>(0x100), arena<void>(0x200), a[0], a[1], a[2], arena<float>(0x1c)[0]);
                             return Result{};
                         }};
    quick(particle_launch);
    // Animations::SetMove(this, n, from, to, w)
    Test animations_set_move{0x94de60,
                             [=](Rng &r) {
                                 arena<uint32_t>(0x2a4)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                                 for (int i = 0; i < 0x68; i++) arena<uint32_t>(0x400)[i] = r.next();
                                 for (int i = 0; i < 8; i++) arena<uint32_t>(0x20)[i] = r.next();
                                 arena<int32_t>(0x10)[0] = r.range(0, 1);
                             },
                             [](void *fn) {
                                 ((void(__thiscall *)(void *, int32_t, void *, void *, uint32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<void>(0x20), arena<void>(0x30), arena<uint32_t>(0x3c)[0]);
                                 return Result{};
                             }};
    quick(animations_set_move);
    // Request::CopyFrom(this, owner, from): the owner's list with one node
    Test request_copy_from{0xc51260,
                           [=](Rng &r) {
                               for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next(), arena<uint32_t>(0x600)[i] = r.next();
                               arena<uint32_t>(0x210)[0] = (uint32_t)(uintptr_t)arena<void>(0x300), arena<uint32_t>(0x300)[0] = (uint32_t)(uintptr_t)arena<void>(0x340);
                               arena<uint32_t>(0x340)[1] = r.next();
                           },
                           [](void *fn) {
                               return Result{(uint32_t)(((uint32_t *(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x200), arena<void>(0x600)) == arena<uint32_t>(0x100))};
                           }};
    quick(request_copy_from);
    // Contact::Make and Make2 (out, a, b, v, pair): signalling NaNs now and then
    for (const uint32_t at : {0xaf8e30u, 0xaf75d0u}) {
        Test contact_make{at,
                          [=](Rng &r) {
                              for (int i = 0; i < 6; i++) arena<float>(0x200)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r);
                              if (r.range(0, 5) == 0) arena<uint32_t>(0x200)[r.range(0, 5)] = 0x7f800001u + r.range(0, 99);
                              for (int i = 0; i < 9; i++) arena<uint32_t>(0x100)[i] = r.next();
                              for (int i = 0; i < 3; i++) arena<uint32_t>(0x20)[i] = r.next();
                          },
                          [](void *fn) {
                              ((void(__stdcall *)(void *, void *, void *, uint32_t, void *))fn)(arena<void>(0x100), arena<void>(0x200), arena<void>(0x20c), arena<uint32_t>(0x20)[0], arena<void>(0x24));
                              return Result{};
                          }};
        quick(contact_make);
    }
    // Path::Cell704A50(this): flagged and not
    Test path_cell_704a50{0x704a50,
                          [=](Rng &r) {
                              for (int i = 0; i < 6; i++) arena<int32_t>(0x2a8)[i] = r.range(0, 5) ? r.range(-5000, 5000) : (int32_t)r.next();
                              arena<uint8_t>(0x2b0)[0] = r.range(0, 1) ? (uint8_t)r.range(1, 255) : 0;
                              arena<uint32_t>(0x2bc)[0] = r.next(), arena<uint32_t>(0x2c0)[0] = r.next();
                          },
                          [](void *fn) { ((void(__thiscall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(path_cell_704a50);
    // Blink::Advance(this, dt)
    Test blink_advance{0x6b60a0,
                       [=](Rng &r) { arena<float>(0x6110)[0] = r.range(0, 9) ? r.real(0, 1) : geo_pick(r), arena<float>(0x10)[0] = r.range(0, 9) ? r.real(0, 2.5f) : geo_pick(r); },
                       [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x10)); return Result{}; }};
    quick(blink_advance);
    // Particle::FirstColour and SecondColour (this, out)
    for (const uint32_t at : {0x9c62d0u, 0x9c6330u}) {
        Test particle_colour{at,
                             [=](Rng &r) { for (int i = 0; i < 6; i++) arena<float>(0x1ac)[i] = r.range(0, 9) ? r.real(-0.2f, 1.2f) : geo_pick(r); arena<uint32_t>(0x20)[0] = r.next(); },
                             [](void *fn) { return Result{(uint32_t)(((uint32_t *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x20)) == arena<uint32_t>(0x20))}; }};
        quick(particle_colour);
    }
    // Object::Position(this, out): with and without the first part, signalling NaNs now and then
    Test object_position{0x548b40,
                         [=](Rng &r) {
                             arena<uint32_t>(0x51c)[0] = r.range(0, 1) ? (uint32_t)(uintptr_t)arena<void>(0x600) : 0, arena<uint32_t>(0x518)[0] = (uint32_t)(uintptr_t)arena<void>(0x700);
                             for (int i = 0; i < 3; i++) arena<float>(0x690)[i] = r.real(-500, 500), arena<float>(0x790)[i] = r.real(-500, 500);
                             if (r.range(0, 5) == 0) arena<uint32_t>(0x690 + 0x100 * r.range(0, 1))[r.range(0, 2)] = 0x7f800001u + r.range(0, 99);
                             for (int i = 0; i < 3; i++) arena<uint32_t>(0x20)[i] = r.next();
                         },
                         [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x20)); return Result{}; }};
    quick(object_position);
    // Object::CopyArea(this, other): no other, other without an area, with one
    Test object_copy_area{0xaae2d0,
                          [=](Rng &r) {
                              for (int i = 0; i < 5; i++) arena<uint32_t>(0x244)[i] = r.next(), arena<uint32_t>(0x444)[i] = r.next();
                              if (r.range(0, 1)) arena<uint8_t>(0x444)[0] = 0;
                              arena<uint32_t>(0x10)[0] = r.range(0, 5) ? (uint32_t)(uintptr_t)arena<void>(0x300) : 0;
                          },
                          [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), (void *)(uintptr_t)arena<uint32_t>(0x10)[0]); return Result{}; }};
    quick(object_copy_area);
    // ResourceRefFA5D28::ResourceRefFA5D28(this, owner, value, kind)
    Test resource_ref_fa5d28{0xbff8d0,
                             [=](Rng &r) {
                                 for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next();
                                 arena<uint32_t>(0x2c4)[0] = (uint32_t)(uintptr_t)arena<void>(0x500);
                                 arena<uint32_t>(0x500)[0] = (uint32_t)(uintptr_t)arena<void>(0x540), arena<uint32_t>(0x540)[1] = r.next();
                                 arena<uint32_t>(0x20)[0] = r.next(), arena<uint32_t>(0x20)[1] = r.next();
                             },
                             [](void *fn) {
                                 return Result{(uint32_t)(((uint32_t *(__thiscall *)(void *, void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<void>(0x200), arena<uint32_t>(0x20)[0], arena<uint32_t>(0x20)[1]) ==
                                                          arena<uint32_t>(0x100))};
                             }};
    quick(resource_ref_fa5d28);
    // AdMenu::ScaleLevel(a, b): ratios that hit each level, and others (no division by zero or overflow)
    Test ad_menu_scale_level{0x45e430,
                             [=](Rng &r) {
                                 const int32_t b = r.range(1, 40) * (r.range(0, 5) ? 1 : -1);
                                 static const int32_t hits[] = {1, 2, 4, 8, 16, 32, 64, 3, 0, -1, 65, 128};
                                 arena<int32_t>(0x10)[0] = r.range(0, 3) ? hits[r.range(0, 11)] * b / 8 + r.range(-1, 1) : r.range(-1000, 1000);
                                 arena<int32_t>(0x10)[1] = b;
                             },
                             [](void *fn) { return Result{((uint32_t(__stdcall *)(int32_t, int32_t))fn)(arena<int32_t>(0x10)[0], arena<int32_t>(0x10)[1])}; }};
    quick(ad_menu_scale_level);
    // ResourceRefFA5CF0::ResourceRefFA5CF0(this, owner, value, kind)
    Test resource_ref_fa5cf0{0xbff2b0,
                             [=](Rng &r) {
                                 for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next();
                                 arena<uint32_t>(0x294)[0] = (uint32_t)(uintptr_t)arena<void>(0x500);
                                 arena<uint32_t>(0x500)[0] = (uint32_t)(uintptr_t)arena<void>(0x540), arena<uint32_t>(0x540)[1] = r.next();
                                 arena<uint32_t>(0x20)[0] = r.next(), arena<uint32_t>(0x20)[1] = r.next();
                             },
                             [](void *fn) {
                                 return Result{(uint32_t)(((uint32_t *(__thiscall *)(void *, void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<void>(0x200), arena<uint32_t>(0x20)[0], arena<uint32_t>(0x20)[1]) ==
                                                          arena<uint32_t>(0x100))};
                             }};
    quick(resource_ref_fa5cf0);
    // PackedSlots::Set(this, id, v, a, b, c, d): blocks 0-2, slots 0-3
    Test packed_slots_set{0x9cedb0,
                          [=](Rng &r) {
                              for (int b = 0; b < 3; b++) arena<uint32_t>(0x100 + (b + 12) * 16)[0] = (uint32_t)(uintptr_t)arena<void>(0x1000 + 0x100 * b);
                              for (int i = 0; i < 0xc0; i++) arena<uint32_t>(0x1000)[i] = r.next();
                              arena<uint32_t>(0x10)[0] = (uint32_t)r.range(0, 2) << 20 | r.range(0, 3);
                              for (int i = 0; i < 7; i++) arena<uint32_t>(0x20)[i] = r.next();
                          },
                          [](void *fn) {
                              const uint32_t *a = arena<uint32_t>(0x20);
                              ((void(__thiscall *)(void *, int32_t, void *, uint32_t, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), (int32_t)arena<uint32_t>(0x10)[0], arena<void>(0x20), a[3], a[4], a[5], a[6]);
                              return Result{};
                          }};
    quick(packed_slots_set);
    // Walker::Step(w): directions and slopes 0-6
    Test walker_step{0x713650,
                     [=](Rng &r) {
                         for (int i = 0; i < 12; i++) arena<int32_t>(0x100)[i] = r.range(-50, 50);
                         arena<uint32_t>(0x128)[0] = r.range(0, 9) ? r.range(0, 5) : r.next(), arena<uint32_t>(0x124)[0] = r.range(0, 9) ? r.range(0, 6) : r.next();
                     },
                     [](void *fn) { ((void(__stdcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(walker_step);
    // Record::Init633B30(this, v): the game's list (global, set for the call and put back) with one node
    static uint32_t record_list_saved;
    Test record_init_633b30{0x633b30,
                            [=](Rng &r) {
                                for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next();
                                arena<uint32_t>(0x300)[0] = r.next(), arena<uint32_t>(0x300)[1] = (uint32_t)(uintptr_t)arena<void>(0x340), arena<uint32_t>(0x340)[0] = r.next();
                                arena<uint32_t>(0x10)[0] = r.next();
                            },
                            [](void *fn) {
                                record_list_saved = *(volatile uint32_t *)0x10f0d70;
                                *(volatile uint32_t *)0x10f0d70 = (uint32_t)(uintptr_t)arena<void>(0x300);
                                uint32_t *got = ((uint32_t *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0]);
                                *(volatile uint32_t *)0x10f0d70 = record_list_saved;
                                return Result{(uint32_t)(got == arena<uint32_t>(0x100))};
                            }};
    quick(record_init_633b30);
    // Ride::Offset(this, out): kinds 0-5
    Test ride_offset{0x5061b0,
                     [=](Rng &r) { arena<uint32_t>(0x2c8)[0] = r.range(0, 5); for (int i = 0; i < 3; i++) arena<uint32_t>(0x2e0)[i] = r.next(), arena<uint32_t>(0x20)[i] = r.next(); },
                     [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x20)); return Result{}; }};
    quick(ride_offset);
    // ByteSlider::SetValue and CharSlider::SetValue (this, fraction)
    for (const uint32_t at : {0xca81b0u, 0xca80a0u}) {
        Test byte_slider_set_value{at,
                                   [=](Rng &r) {
                                       arena<uint32_t>(0x104)[0] = (uint32_t)(uintptr_t)arena<void>(0x200), arena<uint32_t>(0x200)[0] = r.next();
                                       arena<uint8_t>(0x108)[0] = (uint8_t)r.next(), arena<uint8_t>(0x109)[0] = (uint8_t)r.next();
                                       arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-0.2f, 1.2f) : geo_pick(r);
                                   },
                                   [](void *fn) { ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x10)[0]); return Result{}; }};
        quick(byte_slider_set_value);
    }
    // Window::FadeC5E410(this, v)
    Test window_fade_c5e410{0xc5e410,
                            [=](Rng &r) {
                                arena<float>(0x1dc)[0] = r.range(0, 9) ? r.real(-0.5f, 1.5f) : geo_pick(r), arena<uint32_t>(0x1e0)[0] = r.next(), arena<uint8_t>(0x19c)[0] = (uint8_t)r.next();
                                arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-0.5f, 1.5f) : geo_pick(r);
                            },
                            [](void *fn) { ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x10)[0]); return Result{}; }};
    quick(window_fade_c5e410);
    // Bar::Fill(this): the game's maximum set for the call and put back
    static uint32_t bar_max_saved;
    Test bar_fill{0x436d70,
                  [=](Rng &r) { arena<float>(0x3e8)[0] = r.range(0, 9) ? r.real(-50, 600) : geo_pick(r), arena<float>(0x10)[0] = r.range(0, 9) ? r.real(1, 500) : geo_pick(r); },
                  [](void *fn) {
                      bar_max_saved = *(volatile uint32_t *)0x10eda7c;
                      *(volatile float *)0x10eda7c = arena<float>(0x10)[0];
                      const float v = (float)((long double(__thiscall *)(void *))fn)(arena<void>(0x100));
                      *(volatile uint32_t *)0x10eda7c = bar_max_saved;
                      uint32_t bits;
                      memcpy(&bits, &v, 4);
                      return Result{bits};
                  }};
    quick(bar_fill);
    // Angle::Sector(angle)
    Test angle_sector{0xbc4f90,
                      [=](Rng &r) { arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-7, 13) : geo_pick(r); },
                      [](void *fn) { return Result{((uint32_t(__stdcall *)(float))fn)(arena<float>(0x10)[0])}; }};
    quick(angle_sector);
    // Counter::AddFraction(this, rate)
    Test counter_add_fraction{0xcdb720,
                              [=](Rng &r) {
                                  arena<uint32_t>(0x100)[0] = r.next(), arena<uint32_t>(0x104)[0] = r.next(), arena<float>(0x108)[0] = r.range(0, 9) ? r.real(0, 1) : geo_pick(r);
                                  arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-50, 300) : geo_pick(r);
                              },
                              [](void *fn) { ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x10)[0]); return Result{}; }};
    quick(counter_add_fraction);
    // Vector4::Scale(this, k)
    Test vector4_scale{0xcc48f0,
                       [=](Rng &r) { for (int i = 0; i < 4; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r); arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-4, 4) : geo_pick(r); },
                       [](void *fn) { return Result{(uint32_t)(((float *(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x10)[0]) == arena<float>(0x100))}; }};
    quick(vector4_scale);
    // WAS_PlayWithItem::AddProgress(this, d)
    Test was_add_progress{0xb23990,
                          [=](Rng &r) { arena<float>(0x230)[0] = r.range(0, 9) ? r.real(-0.2f, 1.2f) : geo_pick(r), arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-0.5f, 0.5f) : geo_pick(r); },
                          [](void *fn) { ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x10)[0]); return Result{}; }};
    quick(was_add_progress);
    // Object7FC84::Object7FC84(this, owner, p, v, f1, f2, f3)
    Test object_7fc84{0x9883d0,
                      [=](Rng &r) { for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next(), arena<uint32_t>(0x20)[i] = r.next(); },
                      [](void *fn) {
                          const uint32_t *a = arena<uint32_t>(0x20);
                          return Result{(uint32_t)(((uint32_t *(__thiscall *)(void *, uint32_t, void *, uint32_t, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), a[0], arena<void>(0x40), a[2], a[3], a[4], a[5]) ==
                                                   arena<uint32_t>(0x100))};
                      }};
    quick(object_7fc84);
    // ScrollBar::SetPageSize(this, size): signalling NaNs now and then
    Test scroll_bar_set_page_size{0xc5e3c0,
                                  [=](Rng &r) {
                                      arena<float>(0x1e0)[0] = r.range(0, 9) ? r.real(0, 1) : geo_pick(r), arena<uint32_t>(0x1dc)[0] = r.next();
                                      arena<float>(0x10)[0] = r.range(0, 9) ? r.real(0, 1.5f) : geo_pick(r);
                                      if (r.range(0, 19) == 0) arena<uint32_t>(0x10)[0] = 0x7f800001u + r.range(0, 99);
                                  },
                                  [](void *fn) { ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x10)[0]); return Result{}; }};
    quick(scroll_bar_set_page_size);
    // Points::Within(a, b, d): the low byte
    Test points_within{0x925d20,
                       [=](Rng &r) {
                           for (int i = 0; i < 3; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-50, 50) : geo_pick(r), arena<float>(0x110)[i] = r.range(0, 9) ? r.real(-50, 50) : geo_pick(r);
                           arena<float>(0x10)[0] = r.range(0, 9) ? r.real(0, 120) : geo_pick(r);
                       },
                       [](void *fn) { return Result{(uint32_t)((uint8_t(__stdcall *)(void *, void *, float))fn)(arena<void>(0x100), arena<void>(0x110), arena<float>(0x10)[0])}; }};
    quick(points_within);
    // HandleRef::FromRefBit(this, other, n): a handle with a list of one node
    Test handle_ref_from_ref_bit{0x7a7040,
                                 [=](Rng &r) {
                                     for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x100)[i] = r.next();
                                     arena<uint32_t>(0x204)[0] = (uint32_t)(uintptr_t)arena<void>(0x300);
                                     arena<uint32_t>(0x308)[0] = r.range(0, 9), arena<uint32_t>(0x320)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                                     arena<uint32_t>(0x400)[0] = (uint32_t)(uintptr_t)arena<void>(0x440), arena<uint32_t>(0x440)[1] = r.next();
                                     arena<uint32_t>(0x10)[0] = r.range(0, 40);
                                 },
                                 [](void *fn) {
                                     return Result{(uint32_t)(((uint32_t *(__thiscall *)(void *, void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x200), arena<uint32_t>(0x10)[0]) == arena<uint32_t>(0x100))};
                                 }};
    quick(handle_ref_from_ref_bit);
    // Angle::Octant(this): angles in degrees, both signs, past a turn
    Test angle_octant{0x4412c0,
                      [=](Rng &r) { arena<float>(0x148)[0] = r.range(0, 9) ? r.real(-1500, 1500) : geo_pick(r), arena<float>(0x154)[0] = r.range(0, 9) ? r.real(-400, 400) : geo_pick(r); },
                      [](void *fn) { return Result{((uint32_t(__thiscall *)(void *))fn)(arena<void>(0x100))}; }};
    quick(angle_octant);
    // Batches::InitC9C3C0(this, owner): the owner's list with one node
    Test batches_init{0xc9c3c0,
                      [=](Rng &r) {
                          for (int i = 0; i < 0x30; i++) arena<uint32_t>(0x100)[i] = r.next();
                          arena<uint32_t>(0x214)[0] = (uint32_t)(uintptr_t)arena<void>(0x500), arena<uint32_t>(0x260)[0] = r.next();
                          arena<uint32_t>(0x500)[0] = (uint32_t)(uintptr_t)arena<void>(0x540), arena<uint32_t>(0x540)[1] = r.next();
                      },
                      [](void *fn) { return Result{(uint32_t)(((uint32_t *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200)) == arena<uint32_t>(0x100))}; }};
    quick(batches_init);
    // Segment3D::Segment3D(this, a, b, p, q)
    Test segment3d{0xc46e20,
                   [=](Rng &r) { for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next(), arena<uint32_t>(0x20)[i] = r.next(); },
                   [](void *fn) {
                       const uint32_t *a = arena<uint32_t>(0x20);
                       return Result{(uint32_t)(((uint32_t *(__thiscall *)(void *, uint32_t, uint32_t, void *, void *))fn)(arena<void>(0x100), a[0], a[1], arena<void>(0x30), arena<void>(0x40)) == arena<uint32_t>(0x100))};
                   }};
    quick(segment3d);
    // Window::CaseA45110(n): cases -1 to 7
    Test window_case_a45110{0xa45110,
                            [=](Rng &r) { arena<uint32_t>(0x10)[0] = r.range(0, 9) ? r.range(0, 7) : r.next(); },
                            [](void *fn) {
                                const float v = (float)((long double(__stdcall *)(uint32_t))fn)(arena<uint32_t>(0x10)[0]);
                                uint32_t bits;
                                memcpy(&bits, &v, 4);
                                return Result{bits};
                            }};
    quick(window_case_a45110);
    // Graph::PointFor(this, out, v, below)
    Test graph_point_for{0x99d300,
                         [=](Rng &r) {
                             arena<float>(0x180)[0] = r.real(0, 50), arena<float>(0x188)[0] = r.range(0, 9) ? r.real(50, 400) : geo_pick(r);
                             arena<uint32_t>(0x1dc)[0] = (uint32_t)(uintptr_t)arena<void>(0x300);
                             arena<float>(0x3dc)[0] = r.range(0, 9) ? r.real(0, 10) : geo_pick(r), arena<float>(0x38c)[0] = r.real(-100, 100);
                             arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-50, 50) : geo_pick(r), arena<float>(0x10)[1] = r.range(0, 9) ? r.real(-1.5f, 1.5f) : geo_pick(r);
                             arena<uint32_t>(0x18)[0] = r.next() & 0x1ff, arena<uint32_t>(0x20)[0] = r.next(), arena<uint32_t>(0x24)[0] = r.next();
                         },
                         [](void *fn) {
                             ((void(__thiscall *)(void *, void *, void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x20), arena<void>(0x10), arena<uint32_t>(0x18)[0]);
                             return Result{};
                         }};
    quick(graph_point_for);
    // Ride::Flip548A90(this): signalling NaNs now and then
    Test ride_flip{0x548a90,
                   [=](Rng &r) {
                       for (int i = 0; i < 11; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r);
                       if (r.range(0, 3) == 0) arena<uint32_t>(0x100)[r.range(0, 10)] = 0x7f800001u + r.range(0, 99);
                   },
                   [](void *fn) { ((void(__thiscall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(ride_flip);
    // Colour::ToRGB(out, r, g, b)
    Test colour_to_rgb{0x402bf0,
                       [=](Rng &r) { for (int i = 0; i < 3; i++) arena<float>(0x10)[i] = r.range(0, 9) ? r.real(-0.2f, 1.2f) : geo_pick(r); arena<uint32_t>(0x100)[0] = r.next(); },
                       [](void *fn) {
                           const float *a = arena<float>(0x10);
                           return Result{(uint32_t)(((uint32_t *(__thiscall *)(void *, float, float, float))fn)(arena<void>(0x100), a[0], a[1], a[2]) == arena<uint32_t>(0x100))};
                       }};
    quick(colour_to_rgb);
    // Park::Copy9E0DD0(this, from): the game's origin set for the call and put back
    static uint32_t park_origin_saved[3];
    Test park_copy_9e0dd0{0x9e0dd0,
                          [=](Rng &r) {
                              for (int i = 0; i < 7; i++) arena<uint32_t>(0x100)[i] = r.next();
                              for (int i = 0; i < 3; i++) arena<float>(0x200)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r), arena<float>(0x20)[i] = r.real(-500, 500);
                              if (r.range(0, 9) == 0) arena<uint32_t>(0x200)[r.range(0, 2)] = 0x7f800001u + r.range(0, 99);
                          },
                          [](void *fn) {
                              for (int i = 0; i < 3; i++) park_origin_saved[i] = ((volatile uint32_t *)0x14002c0)[i], ((volatile uint32_t *)0x14002c0)[i] = arena<uint32_t>(0x20)[i];
                              ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200));
                              for (int i = 0; i < 3; i++) ((volatile uint32_t *)0x14002c0)[i] = park_origin_saved[i];
                              return Result{};
                          }};
    quick(park_copy_9e0dd0);
    // Water::Entry94DE10(this, n, a, b)
    Test water_entry_94de10{0x94de10,
                            [=](Rng &r) {
                                arena<uint32_t>(0x2a4)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                                for (int i = 0; i < 0x68; i++) arena<uint32_t>(0x400)[i] = r.next();
                                for (int i = 0; i < 8; i++) arena<uint32_t>(0x20)[i] = r.next();
                                arena<int32_t>(0x10)[0] = r.range(0, 1);
                            },
                            [](void *fn) { ((void(__thiscall *)(void *, int32_t, void *, void *))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<void>(0x20), arena<void>(0x30)); return Result{}; }};
    quick(water_entry_94de10);
    // Guest::Take758590(this, n, kind): the low byte
    Test guest_take{0x758590,
                    [=](Rng &r) {
                        for (int i = 0; i < 7; i++) arena<int32_t>(0x100)[i] = r.range(-20, 100);
                        arena<int32_t>(0x10)[0] = r.range(-10, 120), arena<uint32_t>(0x14)[0] = r.range(0, 9) ? r.range(0, 6) : r.next();
                    },
                    [](void *fn) { return Result{(uint32_t)((uint8_t(__thiscall *)(void *, int32_t, uint32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<uint32_t>(0x14)[0])}; }};
    quick(guest_take);
    // Window::Init65FB30(this, v): the game's list (global, set for the call and put back) with one node
    static uint32_t window_list_saved;
    Test window_init_65fb30{0x65fb30,
                            [=](Rng &r) {
                                for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next();
                                arena<uint32_t>(0x300)[0] = r.next(), arena<uint32_t>(0x300)[1] = (uint32_t)(uintptr_t)arena<void>(0x340), arena<uint32_t>(0x340)[0] = r.next();
                                arena<uint32_t>(0x10)[0] = r.next();
                            },
                            [](void *fn) {
                                window_list_saved = *(volatile uint32_t *)0x10eecb0;
                                *(volatile uint32_t *)0x10eecb0 = (uint32_t)(uintptr_t)arena<void>(0x300);
                                void *got = ((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0]);
                                *(volatile uint32_t *)0x10eecb0 = window_list_saved;
                                return Result{(uint32_t)(got == arena<void>(0x100))};
                            }};
    quick(window_init_65fb30);
    // Style::ColourFor(this, state, highlighted)
    Test style_colour_for{0x58fd30,
                          [=](Rng &r) {
                              arena<uint32_t>(0x2cc)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                              for (int i = 0; i < 10; i++) arena<uint32_t>(0x400)[i] = r.next();
                              arena<uint32_t>(0x10)[0] = r.range(0, 9) ? r.range(0, 4) : r.next(), arena<uint32_t>(0x10)[1] = r.range(0, 2) ? 0 : r.next();
                          },
                          [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], arena<uint32_t>(0x10)[1])}; }};
    quick(style_colour_for);
    // SortedInts::Find(key, list, count, place, -): sorted lists of 0-9 numbers (repeats too), keys in and out
    Test sorted_ints_find{0x7fb9c0,
                          [=](Rng &r) {
                              const int32_t n = r.range(0, 9);
                              int32_t v = r.range(-20, 20);
                              for (int i = 0; i < n; i++) arena<int32_t>(0x200)[i] = v, v += r.range(0, 4);
                              arena<int32_t>(0x10)[0] = r.range(-25, 50), arena<int32_t>(0x14)[0] = r.range(0, 9) ? n : r.range(-3, 0);
                              arena<uint32_t>(0x20)[0] = r.next();
                          },
                          [](void *fn) {
                              return Result{(uint32_t)((uint8_t(__fastcall *)(void *, void *, int32_t, void *, uint32_t))fn)(arena<void>(0x10), arena<void>(0x200), arena<int32_t>(0x14)[0], arena<void>(0x20), 0)};
                          }};
    quick(sorted_ints_find);
    // FlagSlots::Get(this, flag) and Set(this, flag, value): every flag, and others
    for (const uint32_t at : {0x5d11c0u, 0x5d10f0u}) {
        Test flag_slots{at,
                        [=](Rng &r) {
                            for (int i = 0; i < 10; i++) arena<uint32_t>(0x100)[i] = r.next();
                            const int pick = r.range(0, 12);
                            arena<int32_t>(0x10)[0] = pick < 10 ? 1 << pick : pick == 10 ? r.range(-3, 0x210) : pick == 11 ? 0x400 : (int32_t)r.next();
                            arena<uint32_t>(0x14)[0] = r.next();
                        },
                        [at](void *fn) {
                            if (at == 0x5d11c0u)
                                return Result{((uint32_t(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0])};
                            ((void(__thiscall *)(void *, int32_t, uint32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<uint32_t>(0x14)[0]);
                            return Result{};
                        }};
        quick(flag_slots);
    }
    // Bits::AllSet(this, start, end): block sizes 1-9, a list of four words (some all set), ranges of any sign
    Test bits_all_set{0xc16660,
                      [=](Rng &r) {
                          arena<int32_t>(0x104)[0] = r.range(1, 9) * (r.range(0, 9) ? 1 : -1), arena<uint32_t>(0x10c)[0] = (uint32_t)(uintptr_t)arena<void>(0x200), arena<int32_t>(0x110)[0] = r.range(0, 4);
                          for (int i = 0; i < 4; i++) arena<uint32_t>(0x200)[i] = r.range(0, 2) ? 0xffffffffu : r.next();
                          arena<int32_t>(0x10)[0] = r.range(-40, 200), arena<int32_t>(0x10)[1] = r.range(-40, 300);
                      },
                      [](void *fn) { return Result{(uint32_t)((uint8_t(__thiscall *)(void *, int32_t, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<int32_t>(0x10)[1])}; }};
    quick(bits_all_set);
    // Value::MoveToward(value, rate, target, dt): equal, near and far targets either side
    Test value_move_toward{0x9e85f0,
                           [=](Rng &r) {
                               const float v = r.range(0, 9) ? r.real(-50, 50) : geo_pick(r);
                               arena<float>(0x100)[0] = v;
                               const int pick = r.range(0, 5);
                               arena<float>(0x10)[0] = r.range(0, 9) ? r.real(0, 20) : geo_pick(r);
                               arena<float>(0x10)[1] = pick == 0 ? v : pick == 1 ? v + r.real(-1e-8f, 1e-8f) : r.range(0, 9) ? v + r.real(-10, 10) : geo_pick(r);
                               arena<float>(0x10)[2] = r.range(0, 9) ? r.real(0, 0.5f) : geo_pick(r);
                           },
                           [](void *fn) {
                               const float *a = arena<float>(0x10);
                               return Result{(uint32_t)((uint8_t(__thiscall *)(void *, float, float, float))fn)(arena<void>(0x100), a[0], a[1], a[2])};
                           }};
    quick(value_move_toward);
    // Keys::Find and Find2 (this, t): 1-8 keys with rising times, t before, inside and after
    for (const uint32_t at : {0xe02670u, 0xe025f0u}) {
        Test keys_find{at,
                       [=](Rng &r) {
                           const int32_t n = r.range(1, 8);
                           const int base = at == 0xe02670u ? 0x10c : 0x104;
                           arena<int32_t>(base)[0] = n, arena<uint32_t>(base + 4)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                           float t = r.real(-5, 5);
                           for (int i = 0; i < n; i++) arena<float>(0x400)[4 * i] = t, t += r.range(0, 5) ? r.real(0, 3) : 0;
                           arena<float>(0x10)[0] = r.range(0, 2) == 0 ? arena<float>(0x400)[4 * r.range(0, n - 1)] : r.range(0, 9) ? r.real(-8, 25) : geo_pick(r);   // exact key times too
                       },
                       [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x10)[0])}; }};
        quick(keys_find);
    }
    // SortedTable::FindVersion(this, key, place): 0-8 sorted 64-bit values (repeats, negatives), keys in and out
    Test sorted_table_find_version{0x7a2cf0,
                                   [=](Rng &r) {
                                       const int32_t n = r.range(0, 8);
                                       arena<int32_t>(0x104)[0] = r.range(0, 9) ? n : -r.range(1, 3), arena<uint32_t>(0x108)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                                       int64_t v = (int64_t)r.range(-5, 5) << (r.range(0, 1) ? 32 : 0);
                                       for (int i = 0; i < n; i++) {
                                           arena<uint32_t>(0x404 + 24 * i)[0] = (uint32_t)(uintptr_t)arena<void>(0x800 + 8 * i);
                                           *arena<int64_t>(0x800 + 8 * i) = v;
                                           v += r.range(0, 3) == 0 ? 0 : r.range(0, 1) ? (int64_t)r.range(1, 3) << 32 : r.range(1, 3);
                                       }
                                       *arena<int64_t>(0x220) = r.range(0, 2) && n ? *arena<int64_t>(0x800 + 8 * r.range(0, n - 1)) : ((int64_t)r.range(-6, 9) << 32) + r.range(-3, 9);
                                       arena<uint32_t>(0x20)[0] = r.next();
                                   },
                                   [](void *fn) { return Result{(uint32_t)((uint8_t(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x200), arena<void>(0x20))}; }};
    quick(sorted_table_find_version);
    // Lod::LevelFor(this, n, fallback)
    Test lod_level_for{0xbc0b00,
                       [=](Rng &r) {
                           arena<uint32_t>(0x144)[0] = r.range(0, 5) ? r.next() | 1 : 0, arena<uint32_t>(0x140)[0] = r.range(0, 1) ? r.next() : 0;
                           arena<uint8_t>(0x102)[0] = (uint8_t)r.range(0, 7), arena<uint8_t>(0x104)[0] = (uint8_t)r.range(0, 6), arena<uint8_t>(0x105)[0] = (uint8_t)r.range(0, 3);
                           arena<int32_t>(0x10)[0] = r.range(-3, 9), arena<uint32_t>(0x14)[0] = r.next() & 0x1ff;
                       },
                       [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, int32_t, uint32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<uint32_t>(0x14)[0])}; }};
    quick(lod_level_for);
    // SortedPointers::FindXY(this, x, y, flagged_only, place): 0-8 entries sorted by (y, x), some flagged
    Test sorted_pointers_find_xy{0x8fef80,
                                 [=](Rng &r) {
                                     const int32_t n = r.range(0, 8);
                                     arena<uint32_t>(0x130)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<int32_t>(0x134)[0] = n;
                                     int32_t x = r.range(-3, 3), y = r.range(-3, 3);
                                     for (int i = 0; i < n; i++) {
                                         arena<uint32_t>(0x400)[i] = (uint32_t)(uintptr_t)arena<void>(0x800 + 16 * i);
                                         arena<int32_t>(0x800 + 16 * i)[0] = x, arena<int32_t>(0x804 + 16 * i)[0] = y, arena<uint8_t>(0x808 + 16 * i)[0] = r.range(0, 2) ? 0 : (uint8_t)r.range(1, 255);
                                         if (r.range(0, 1)) x += r.range(1, 2); else y += r.range(1, 2), x = r.range(-3, 3);
                                     }
                                     const int pick = n ? r.range(0, n) : 0;
                                     arena<int32_t>(0x10)[0] = pick < n ? arena<int32_t>(0x800 + 16 * pick)[0] : r.range(-4, 10), arena<int32_t>(0x10)[1] = pick < n ? arena<int32_t>(0x804 + 16 * pick)[0] : r.range(-4, 10);
                                     arena<uint32_t>(0x10)[2] = r.next() & 0x1ff, arena<uint32_t>(0x10)[3] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x30) : 0;
                                     arena<uint32_t>(0x30)[0] = r.next();
                                 },
                                 [](void *fn) {
                                     const uint32_t *a = arena<uint32_t>(0x10);
                                     return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, int32_t, int32_t, uint32_t, void *))fn)(arena<void>(0x100), (int32_t)a[0], (int32_t)a[1], a[2], (void *)(uintptr_t)a[3])};
                                 }};
    quick(sorted_pointers_find_xy);
    // Refs20::Refs20(this, owner): references null or not
    Test refs20{0xc09fe0,
                [=](Rng &r) {
                    for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x100)[i] = r.next();
                    for (int i = 0; i < 24; i++) arena<uint32_t>(0x200)[i] = r.range(0, 2) ? (uint32_t)(uintptr_t)arena<void>(0x400 + 4 * r.range(0, 30)) : 0;
                    for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x400)[i] = r.next();
                },
                [](void *fn) { return Result{(uint32_t)(((uint32_t *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200)) == arena<uint32_t>(0x100))}; }};
    quick(refs20);
    // Value::Band2(v)
    Test value_band2{0x7c3080,
                     [=](Rng &r) {
                         static const uint32_t at[] = {0xf27050, 0xf27058, 0xf71cac, 0xf71ca8};
                         arena<float>(0x10)[0] = r.range(0, 3) ? *(const float *)(uintptr_t)at[r.range(0, 3)] * r.real(0.8f, 1.2f) : r.range(0, 1) ? *(const float *)(uintptr_t)at[r.range(0, 3)] : geo_pick(r);
                     },
                     [](void *fn) { return Result{(uint32_t)((uint8_t(__stdcall *)(float))fn)(arena<float>(0x10)[0])}; }};
    quick(value_band2);
    // Entries::FindMatching(object, list, out): 0-6 entries, a match or not, signalling NaNs now and then
    Test entries_find_matching{0xb78630,
                               [=](Rng &r) {
                                   const int32_t n = r.range(0, 6);
                                   arena<uint32_t>(0x200)[0] = (uint32_t)(uintptr_t)arena<void>(0x1000), arena<int32_t>(0x208)[0] = n;
                                   for (int i = 0; i < n; i++) {
                                       uint32_t *e = arena<uint32_t>(0x1000 + 0x70 * i);
                                       e[0] = r.range(0, 9) ? r.next() : 0x7f800001u + r.range(0, 99), e[3] = r.range(0, 2), e[4] = r.range(0, 2);
                                   }
                                   arena<uint32_t>(0x10c)[0] = r.range(0, 2), arena<uint32_t>(0x110)[0] = r.range(0, 2);
                                   arena<uint32_t>(0x20)[0] = r.next();
                               },
                               [](void *fn) { return Result{(uint32_t)((uint8_t(__stdcall *)(void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x200), arena<void>(0x20))}; }};
    quick(entries_find_matching);
    // Sizes::Pick(base, scale)
    Test sizes_pick{0x9d9c40,
                    [=](Rng &r) { arena<uint32_t>(0x10)[0] = r.range(0, 9) ? r.range(0, 3) : r.next(), arena<uint32_t>(0x10)[1] = r.range(0, 9) ? r.range(0, 3) : r.next(); },
                    [](void *fn) { return Result{((uint32_t(__fastcall *)(uint32_t, uint32_t))fn)(arena<uint32_t>(0x10)[0], arena<uint32_t>(0x10)[1])}; }};
    quick(sizes_pick);
    // Gate::SetState(gate, action): actions 0-6, the second value 0-3
    Test gate_set_state{0x4ee3a0,
                        [=](Rng &r) { arena<uint32_t>(0x120)[0] = r.next(), arena<uint32_t>(0x124)[0] = r.range(0, 3); arena<uint32_t>(0x10)[0] = r.range(0, 9) ? r.range(0, 6) : r.next(); },
                        [](void *fn) { ((void(__stdcall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0]); return Result{}; }};
    quick(gate_set_state);
    // Quad::UpdateCounts(this): the game's totals set for the call, read back and put back
    static uint32_t quad_totals_saved[2];
    Test quad_update_counts{0x8fdb30,
                            [=](Rng &r) {
                                for (int i = 2; i < 6; i++) arena<uint32_t>(0x100)[i] = r.range(0, 4) ? r.next() | 1 : 0;
                                arena<uint32_t>(0x118)[0] = r.range(0, 1) ? 4 : 0, arena<uint32_t>(0x11c)[0] = r.range(0, 1) ? 6 : 0;
                                arena<uint32_t>(0x10)[0] = r.range(0, 100), arena<uint32_t>(0x14)[0] = r.range(0, 100);
                            },
                            [](void *fn) {
                                volatile uint32_t *g = (volatile uint32_t *)0x133a028;
                                quad_totals_saved[0] = g[0], quad_totals_saved[1] = g[1];
                                g[0] = arena<uint32_t>(0x10)[0], g[1] = arena<uint32_t>(0x14)[0];
                                ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                                const uint32_t got = g[0] * 1000 + g[1];
                                g[0] = quad_totals_saved[0], g[1] = quad_totals_saved[1];
                                return Result{got};
                            }};
    quick(quad_update_counts);
    // String::Find and FindLast (this, text, from): a string of a's and b's, texts of 0-3 of them
    for (const uint32_t at : {0xbcf460u, 0xbcf600u}) {
        Test string_find{at,
                         [=](Rng &r) {
                             arena<uint32_t>(0x100)[0] = (uint32_t)(uintptr_t)arena<void>(0x200);
                             const uint32_t len = r.range(0, 12);
                             arena<uint32_t>(0x204)[0] = len, arena<uint32_t>(0x20c)[0] = (uint32_t)(uintptr_t)arena<void>(0x301);
                             for (int i = 0; i < 0x20; i++) arena<char>(0x300)[i] = r.range(0, 5) ? "ab"[r.range(0, 1)] : 0;
                             const int tlen = r.range(0, 3);
                             for (int i = 0; i < tlen; i++) arena<char>(0x400)[i] = "ab"[r.range(0, 1)];
                             arena<char>(0x400)[tlen] = 0;
                             arena<uint32_t>(0x10)[0] = r.range(0, 5) ? r.range(0, 14) : 0xffffffffu;   // (a large start on an empty string reads anywhere)
                         },
                         [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x400), arena<uint32_t>(0x10)[0])}; }};
        quick(string_find);
    }
    // Refs20::CopyFrom(this, other)
    Test refs20_copy_from{0xc0a060,
                          [=](Rng &r) {
                              for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x100)[i] = r.next();
                              arena<uint32_t>(0x204)[0] = r.next();
                              for (int i = 0; i < 24; i++) arena<uint32_t>(0x208)[i] = r.range(0, 2) ? (uint32_t)(uintptr_t)arena<void>(0x400 + 4 * r.range(0, 30)) : 0;
                              for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x400)[i] = r.next();
                          },
                          [](void *fn) { return Result{(uint32_t)(((uint32_t *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200)) == arena<uint32_t>(0x100))}; }};
    quick(refs20_copy_from);
    // Buckets::ClearLists(this, n): three buckets of 0-3 nodes, n from -2 to 4
    Test buckets_clear_lists{0xc1aea0,
                             [=](Rng &r) {
                                 arena<uint32_t>(0x104)[0] = 3, arena<uint32_t>(0x110)[0] = (uint32_t)(uintptr_t)arena<void>(0x200);
                                 for (int b = 0; b < 3; b++) {
                                     arena<uint32_t>(0x200)[b] = (uint32_t)(uintptr_t)arena<void>(0x220 + 4 * b);
                                     const int n = r.range(0, 3);
                                     uint32_t next = (uint32_t)(uintptr_t)arena<void>(0x300 + 0x40 * b + 0x30);
                                     arena<uint32_t>(0x300 + 0x40 * b + 0x30)[0] = 0;
                                     for (int i = n - 1; i >= 0; i--) {
                                         uint32_t *node = arena<uint32_t>(0x300 + 0x40 * b + 0x10 * i);
                                         node[0] = next, node[2] = (uint32_t)(uintptr_t)arena<void>(0x800 + 0x40 * (4 * b + i));
                                         next = (uint32_t)(uintptr_t)node;
                                     }
                                     arena<uint32_t>(0x220 + 4 * b)[0] = next;
                                 }
                                 for (int i = 0; i < 0xc0; i++) arena<uint32_t>(0x800)[i] = r.next();
                                 arena<int32_t>(0x10)[0] = r.range(-2, 4);
                             },
                             [](void *fn) { ((void(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0]); return Result{}; }};
    quick(buckets_clear_lists);
    // Nodes::MatrixFor(this, n, shared): flags of every kind
    Test nodes_matrix_for{0xcc68c0,
                          [=](Rng &r) {
                              arena<uint32_t>(0x100)[0] = (uint32_t)(uintptr_t)arena<void>(0x200), arena<uint32_t>(0x104)[0] = 0x10000;
                              arena<uint32_t>(0x210)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<uint32_t>(0x218)[0] = 0x20000;
                              for (int i = 0; i < 3; i++) arena<uint32_t>(0x454 + 0x60 * i)[0] = (r.next() & 0xf00) | r.range(0, 3), arena<uint32_t>(0x458 + 0x60 * i)[0] = r.range(0, 99);
                              arena<int32_t>(0x10)[0] = r.range(0, 2), arena<uint32_t>(0x20)[0] = r.next();
                          },
                          [](void *fn) {
                              const uintptr_t got = (uintptr_t)((void *(__thiscall *)(void *, int32_t, void *))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<void>(0x20));
                              return Result{(uint32_t)got};
                          }};
    quick(nodes_matrix_for);
    // Guest::AddValue759690(this, a, b)
    Test guest_add_value{0x759690,
                         [=](Rng &r) {
                             arena<float>(0x50c)[0] = r.range(0, 9) ? r.real(0, 1) : geo_pick(r), arena<float>(0x51c)[0] = r.range(0, 9) ? r.real(0, 1) : geo_pick(r);
                             arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-1, 1) : geo_pick(r), arena<float>(0x10)[1] = r.range(0, 9) ? r.real(-1, 1) : geo_pick(r);
                         },
                         [](void *fn) { ((void(__thiscall *)(void *, float, float))fn)(arena<void>(0x100), arena<float>(0x10)[0], arena<float>(0x10)[1]); return Result{}; }};
    quick(guest_add_value);
    // Filter::Accepts(this, item): kinds 0-4, the option on or off
    Test filter_accepts{0x758880,
                        [=](Rng &r) { arena<uint8_t>(0x220)[0] = (uint8_t)r.range(0, 4), arena<uint8_t>(0x221)[0] = r.range(0, 1) ? 0 : (uint8_t)r.range(1, 255); arena<uint32_t>(0x22c)[0] = r.range(0, 1) ? 1u << r.range(0, 7) : r.next() & r.next(); },
                        [](void *fn) { return Result{(uint32_t)((uint8_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200))}; }};
    quick(filter_accepts);
    // List::FindSelected(this, id, -): a list of 0-4 nodes with ids 0-3
    Test list_find_selected{0x9f6490,
                            [=](Rng &r) {
                                arena<uint32_t>(0x1e4)[0] = (uint32_t)(uintptr_t)arena<void>(0x300), arena<uint32_t>(0x1dc)[0] = (uint32_t)(uintptr_t)arena<void>(0x500);
                                arena<uint32_t>(0x65c)[0] = r.next(), arena<uint32_t>(0x660)[0] = r.next();
                                const int n = r.range(0, 4);
                                for (int i = 0; i < n; i++) {
                                    arena<uint32_t>(0x300 + 0x10 * i)[0] = (uint32_t)(uintptr_t)arena<void>(0x300 + 0x10 * (i + 1)), arena<uint32_t>(0x308 + 0x10 * i)[0] = (uint32_t)(uintptr_t)arena<void>(0x400 + 8 * i);
                                    arena<uint32_t>(0x400 + 8 * i)[0] = r.range(0, 3), arena<uint32_t>(0x404 + 8 * i)[0] = r.next();
                                }
                                arena<uint32_t>(0x300 + 0x10 * n)[0] = 0;
                                arena<uint32_t>(0x10)[0] = r.range(0, 3);
                            },
                            [](void *fn) { ((void(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], 0); return Result{}; }};
    quick(list_find_selected);
    // Bounds::Grow(this, min_x, min_y, max_x, max_y, v)
    Test bounds_grow{0x8e7130,
                     [=](Rng &r) {
                         for (int i = 0; i < 5; i++) arena<float>(0x290)[i] = r.range(0, 9) ? r.real(-50, 50) : geo_pick(r);
                         for (int i = 0; i < 4; i++) arena<float>(0x10)[i] = r.range(0, 9) ? r.real(-60, 60) : geo_pick(r);
                         arena<uint32_t>(0x20)[0] = r.next();
                     },
                     [](void *fn) {
                         const float *a = arena<float>(0x10);
                         ((void(__thiscall *)(void *, float, float, float, float, uint32_t))fn)(arena<void>(0x100), a[0], a[1], a[2], a[3], arena<uint32_t>(0x20)[0]);
                         return Result{};
                     }};
    quick(bounds_grow);
    // Indices::CopyOffset(to, from, count, offset): counts 0-13, no offset or one (large ones too)
    Test indices_copy_offset{0xbeecb0,
                             [=](Rng &r) {
                                 for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next(), arena<uint32_t>(0x200)[i] = r.next();
                                 arena<uint32_t>(0x10)[0] = r.range(0, 13), arena<uint32_t>(0x10)[1] = r.range(0, 2) ? 0 : r.range(0, 1) ? r.range(1, 999) : r.next();
                             },
                             [](void *fn) { ((void(__fastcall *)(void *, void *, uint32_t, uint32_t))fn)(arena<void>(0x200), arena<void>(0x100), arena<uint32_t>(0x10)[0], arena<uint32_t>(0x10)[1]); return Result{}; }};
    quick(indices_copy_offset);
    // Vertices::Gather and Gather2 (positions, normals, n, out, source, offset): 1-4 vertices
    for (const uint32_t at : {0xccf9f0u, 0xccf970u}) {
        Test vertices_gather{at,
                             [=](Rng &r) {
                                 for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x400)[i] = r.next(), arena<uint32_t>(0x500)[i] = r.next();
                                 const int n = r.range(1, 4);
                                 arena<uint32_t>(0x10)[0] = n;
                                 for (int i = 0; i < n; i++) {
                                     arena<uint16_t>(0x600 + 12 * i)[0] = (uint16_t)r.range(0, 7), arena<uint16_t>(0x602 + 12 * i)[0] = (uint16_t)r.range(0, 7);
                                     arena<float>(0x604 + 12 * i)[0] = r.range(0, 9) ? r.real(0, 1) : geo_pick(r), arena<float>(0x608 + 12 * i)[0] = r.range(0, 9) ? r.real(0, 1) : geo_pick(r);
                                 }
                                 arena<float>(0x20)[0] = r.range(0, 9) ? r.real(-1, 1) : geo_pick(r), arena<float>(0x20)[1] = r.real(-1, 1);
                                 for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x800)[i] = r.next();
                             },
                             [](void *fn) {
                                 ((void(__fastcall *)(void *, void *, uint32_t, void *, void *, void *))fn)(arena<void>(0x400), arena<void>(0x500), arena<uint32_t>(0x10)[0], arena<void>(0x800), arena<void>(0x600), arena<void>(0x20));
                                 return Result{};
                             }};
        quick(vertices_gather);
    }
    // Curve::Falloff(kind, -, v, limit): kinds 0-5
    Test curve_falloff{0x8ae2f0,
                       [=](Rng &r) {
                           arena<uint32_t>(0x10)[0] = r.range(0, 9) ? r.range(0, 5) : r.next();
                           arena<float>(0x10)[1] = r.range(0, 9) ? r.real(0, 20) : geo_pick(r), arena<float>(0x10)[2] = r.range(0, 9) ? r.real(1, 10) : geo_pick(r);
                       },
                       [](void *fn) {
                           const float v = (float)((long double(__fastcall *)(uint32_t, uint32_t, float, float))fn)(arena<uint32_t>(0x10)[0], 0, arena<float>(0x10)[1], arena<float>(0x10)[2]);
                           uint32_t bits;
                           memcpy(&bits, &v, 4);
                           return Result{bits};
                       }};
    quick(curve_falloff);
    // Guest::Band(v): the game's unit set for the call and put back
    static uint32_t guest_unit_saved;
    Test guest_band{0xa883a0,
                    [=](Rng &r) { arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-1, 7) : geo_pick(r), arena<float>(0x14)[0] = r.range(0, 9) ? r.real(0.5f, 2) : geo_pick(r); },
                    [](void *fn) {
                        guest_unit_saved = *(volatile uint32_t *)0x13cdf20;
                        *(volatile float *)0x13cdf20 = arena<float>(0x14)[0];
                        const uint32_t got = ((uint32_t(__stdcall *)(float))fn)(arena<float>(0x10)[0]);
                        *(volatile uint32_t *)0x13cdf20 = guest_unit_saved;
                        return Result{got};
                    }};
    quick(guest_band);
    // Scenery::Turn827530(this, x, y, x_out, y_out): turns 0-5, cells in and out
    Test scenery_turn{0x827530,
                      [=](Rng &r) {
                          arena<uint32_t>(0x100)[0] = (uint32_t)(uintptr_t)arena<void>(0x200), arena<int32_t>(0x210)[0] = r.range(1, 6), arena<int32_t>(0x214)[0] = r.range(1, 6);
                          arena<int32_t>(0x104)[0] = r.range(-3, 3), arena<int32_t>(0x108)[0] = r.range(-3, 3), arena<uint32_t>(0x110)[0] = r.range(0, 9) ? r.range(0, 5) : r.next();
                          arena<int32_t>(0x10)[0] = r.range(-4, 9), arena<int32_t>(0x10)[1] = r.range(-4, 9), arena<int32_t>(0x20)[0] = r.range(-9, 9), arena<int32_t>(0x24)[0] = r.range(-9, 9);
                      },
                      [](void *fn) {
                          return Result{(uint32_t)((uint8_t(__thiscall *)(void *, int32_t, int32_t, void *, void *))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<int32_t>(0x10)[1], arena<void>(0x20), arena<void>(0x24))};
                      }};
    quick(scenery_turn);
    // SortedPointers::Find(key, list, count, place, -): 0-8 objects sorted by (+4 unsigned, +0xC signed)
    Test sorted_pointers_find{0x87a6d0,
                              [=](Rng &r) {
                                  const int32_t n = r.range(0, 8);
                                  uint32_t a = r.range(0, 1) ? r.range(0, 3) : 0x7ffffffeu + r.range(0, 3);
                                  int32_t b = r.range(-3, 3);
                                  for (int i = 0; i < n; i++) {
                                      arena<uint32_t>(0x400)[i] = (uint32_t)(uintptr_t)arena<void>(0x800 + 16 * i);
                                      arena<uint32_t>(0x804 + 16 * i)[0] = a, arena<int32_t>(0x80c + 16 * i)[0] = b;
                                      if (r.range(0, 2)) b += r.range(0, 2); else a += r.range(1, 2), b = r.range(-3, 3);
                                  }
                                  arena<uint32_t>(0x10)[0] = (uint32_t)(uintptr_t)arena<void>(0x200);
                                  const int pick = r.range(0, n);
                                  arena<uint32_t>(0x204)[0] = pick < n ? arena<uint32_t>(0x804 + 16 * pick)[0] : r.range(0, 5), arena<int32_t>(0x20c)[0] = pick < n ? arena<int32_t>(0x80c + 16 * pick)[0] : r.range(-4, 6);
                                  arena<int32_t>(0x14)[0] = r.range(0, 9) ? n : -r.range(0, 2), arena<uint32_t>(0x20)[0] = r.next();
                              },
                              [](void *fn) {
                                  return Result{(uint32_t)((uint8_t(__fastcall *)(void *, void *, int32_t, void *, uint32_t))fn)(arena<void>(0x10), arena<void>(0x400), arena<int32_t>(0x14)[0], arena<void>(0x20), 0)};
                              }};
    quick(sorted_pointers_find);
    // SceneryItem::RemoveLink(this, object): two lists of 0-5 objects with repeats
    Test scenery_item_remove_link{0x83fbc0,
                                  [=](Rng &r) {
                                      arena<uint32_t>(0x154)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<int32_t>(0x158)[0] = r.range(0, 5);
                                      arena<uint32_t>(0x160)[0] = (uint32_t)(uintptr_t)arena<void>(0x440), arena<int32_t>(0x164)[0] = r.range(0, 5);
                                      for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x400)[i] = r.range(0, 4);
                                      arena<uint32_t>(0x10)[0] = r.range(0, 4);
                                  },
                                  [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0]); return Result{}; }};
    quick(scenery_item_remove_link);
    // WideString::Find(this, text, from): a wide string of a's and b's, texts of 0-3 of them
    Test wide_string_find{0xbcedc0,
                          [=](Rng &r) {
                              arena<uint32_t>(0x100)[0] = (uint32_t)(uintptr_t)arena<void>(0x200);
                              arena<uint32_t>(0x204)[0] = r.range(0, 12), arena<uint32_t>(0x20c)[0] = (uint32_t)(uintptr_t)arena<void>(0x302);
                              for (int i = 0; i < 0x20; i++) arena<uint16_t>(0x300)[i] = r.range(0, 5) ? "ab"[r.range(0, 1)] : 0;
                              const int tlen = r.range(0, 3);
                              for (int i = 0; i < tlen; i++) arena<uint16_t>(0x400)[i] = "ab"[r.range(0, 1)];
                              arena<uint16_t>(0x400)[tlen] = 0;
                              arena<uint32_t>(0x10)[0] = r.range(0, 14);
                          },
                          [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x400), arena<uint32_t>(0x10)[0])}; }};
    quick(wide_string_find);
    // Buffer::LayOut(this): 0-5 parts of kinds 0-3, alignments 1-16
    Test buffer_lay_out{0xcb78e0,
                        [=](Rng &r) {
                            arena<uint32_t>(0x100)[0] = r.next(), arena<uint32_t>(0x104)[0] = r.range(1, 16), arena<uint32_t>(0x108)[0] = r.range(0, 5);
                            arena<uint32_t>(0x114)[0] = (uint32_t)(uintptr_t)arena<void>(0x200), arena<uint32_t>(0x240)[0] = r.range(0, 100);
                            arena<uint32_t>(0x130)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                            for (int i = 0; i < 5; i++) arena<uint8_t>(0x401 + 0x28 * i)[0] = (uint8_t)r.range(0, 3), arena<uint32_t>(0x408 + 0x28 * i)[0] = r.range(0, 50), arena<uint32_t>(0x404 + 0x28 * i)[0] = r.next();
                        },
                        [](void *fn) { return Result{(uint32_t)((uint8_t(__thiscall *)(void *))fn)(arena<void>(0x100))}; }};
    quick(buffer_lay_out);
    // VertexLayout::Add(this, element): counts 14-17, types 0-15, high bytes 0, 1, 2, 0x7F-0x82, 0xFF
    Test vertex_layout_add{0x413e40,
                           [=](Rng &r) {
                               for (int i = 0; i < 8; i++) arena<uint32_t>(0x100)[i] = r.next();
                               arena<uint32_t>(0x120)[0] = r.range(0, 3) ? r.range(0, 15) : r.range(16, 17), arena<uint32_t>(0x124)[0] = r.range(0, 200);
                               static const uint8_t highs[] = {0, 1, 2, 0x7f, 0x80, 0x81, 0x82, 0xff};
                               arena<uint32_t>(0x10)[0] = (uint32_t)highs[r.range(0, 7)] << 8 | r.range(0, 15) | (r.next() & 0xffff0000u);
                           },
                           [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0]); return Result{}; }};
    quick(vertex_layout_add);
    // Bird::ClampA0A340(this)
    Test bird_clamp{0xa0a340,
                    [=](Rng &r) { for (int i = 0; i < 3; i++) arena<float>(0x1b0)[i] = r.range(0, 9) ? r.real(-3000, 3000) * (r.range(0, 1) ? 1.0f : 0.01f) : geo_pick(r); },
                    [](void *fn) { ((void(__thiscall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(bird_clamp);
    // Car::Angle5294A0(a, b, c, out)
    Test car_angle{0x5294a0,
                   [=](Rng &r) { for (int i = 0; i < 3; i++) arena<float>(0x10)[i] = r.range(0, 9) ? r.real(-7, 7) : geo_pick(r); arena<uint32_t>(0x20)[0] = r.next(); },
                   [](void *fn) {
                       const float *a = arena<float>(0x10);
                       return Result{(uint32_t)((uint8_t(__stdcall *)(float, float, float, void *))fn)(a[0], a[1], a[2], arena<void>(0x20))};
                   }};
    quick(car_angle);
    // Pairs::SortOnce(this): marked or not, 0-6 pairs
    Test pairs_sort_once{0x940ae0,
                         [=](Rng &r) {
                             arena<uint32_t>(0x100)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<int32_t>(0x108)[0] = r.range(0, 9) ? r.range(0, 6) : -1;
                             arena<uint8_t>(0x10c)[0] = r.range(0, 3) ? 0 : (uint8_t)r.range(1, 255);
                             for (int i = 0; i < 6; i++) arena<uint32_t>(0x400)[2 * i] = r.next(), arena<float>(0x404)[2 * i] = r.range(0, 9) ? (float)r.range(-3, 3) : geo_pick(r);
                         },
                         [](void *fn) { ((void(__thiscall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(pairs_sort_once);
    // FrameTime::Smooth(this, dt): capped or not, the average set or not, around the 100th frame
    Test frame_time_smooth{0x4141e0,
                           [=](Rng &r) {
                               arena<float>(0x21c)[0] = r.range(0, 1) ? 0.0f : r.range(0, 9) ? r.real(0, 0.2f) : geo_pick(r);
                               arena<uint8_t>(0x210)[0] = r.range(0, 2) ? 0 : (uint8_t)r.range(1, 255), arena<int32_t>(0x20c)[0] = r.range(95, 105);
                               arena<float>(0x214)[0] = r.real(0, 5), arena<uint32_t>(0x218)[0] = r.next();
                               arena<float>(0x10)[0] = r.range(0, 9) ? r.real(0, 0.2f) : geo_pick(r);
                           },
                           [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x10)); return Result{}; }};
    quick(frame_time_smooth);
    // Grid::Contains(this, p): points around a grid, far ones too (the byte cut)
    Test grid_contains{0x493930,
                       [=](Rng &r) {
                           arena<float>(0x10c)[0] = r.real(-50, 50), arena<float>(0x110)[0] = r.real(-50, 50), arena<float>(0x114)[0] = r.range(0, 9) ? r.real(0.5f, 8) : geo_pick(r), arena<float>(0x118)[0] = r.real(0.5f, 8);
                           arena<uint8_t>(0x104)[0] = (uint8_t)r.range(0, 20), arena<uint8_t>(0x105)[0] = (uint8_t)r.range(0, 20);
                           const float reach = r.range(0, 4) ? 200.0f : 3000.0f;
                           arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-reach, reach) : geo_pick(r), arena<float>(0x10)[1] = r.real(-reach, reach);
                       },
                       [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x10))}; }};
    quick(grid_contains);
    // Terrain::LodDistance(this): levels 0-6, cell sides either way round
    Test terrain_lod_distance{0x8f35b0,
                              [=](Rng &r) {
                                  arena<uint32_t>(0x100)[0] = (uint32_t)(uintptr_t)arena<void>(0x200), arena<uint32_t>(0x104)[0] = r.range(0, 9) ? r.range(0, 6) : r.next();
                                  arena<uint8_t>(0x204)[0] = (uint8_t)r.next(), arena<uint8_t>(0x205)[0] = (uint8_t)r.next();
                                  arena<float>(0x214)[0] = r.range(0, 9) ? r.real(0.5f, 8) : geo_pick(r), arena<float>(0x218)[0] = r.range(0, 9) ? r.real(0.5f, 8) : geo_pick(r);
                              },
                              [](void *fn) {
                                  const float v = (float)((long double(__thiscall *)(void *))fn)(arena<void>(0x100));
                                  uint32_t bits;
                                  memcpy(&bits, &v, 4);
                                  return Result{bits};
                              }};
    quick(terrain_lod_distance);
    // Condition::Test(this, v): operators 0-9, values equal to the number now and then
    Test condition_test{0xbb5880,
                        [=](Rng &r) {
                            arena<uint32_t>(0x100)[0] = r.range(0, 9) ? r.range(0, 9) : r.next(), arena<float>(0x108)[0] = r.range(0, 9) ? (float)r.range(-3, 3) : geo_pick(r);
                            arena<float>(0x10)[0] = r.range(0, 2) ? arena<float>(0x108)[0] : r.range(0, 9) ? (float)r.range(-3, 3) : geo_pick(r);
                        },
                        [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x10)[0])}; }};
    quick(condition_test);
    // Path::Bounds71DC50(this, x, y, grow)
    Test path_bounds{0x71dc50,
                     [=](Rng &r) {
                         for (int i = 0; i < 5; i++) arena<uint8_t>(0x19c)[i] = (uint8_t)r.range(0, 9);
                         arena<uint32_t>(0x10)[0] = r.range(0, 9) | (r.next() & 0xffffff00u), arena<uint32_t>(0x10)[1] = r.range(0, 9) | (r.next() & 0xffffff00u), arena<uint32_t>(0x10)[2] = r.next() & 0x1ff;
                     },
                     [](void *fn) {
                         const uint32_t *a = arena<uint32_t>(0x10);
                         ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), a[0], a[1], a[2]);
                         return Result{};
                     }};
    quick(path_bounds);
    // Nodes::MarkChain(this, n, force): six nodes chained at random, some marked
    Test nodes_mark_chain{0xcc75a0,
                          [=](Rng &r) {
                              arena<uint32_t>(0x110)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                              for (int i = 0; i < 6; i++) {
                                  arena<int32_t>(0x450 + 0x60 * i)[0] = r.range(-1, 5);
                                  arena<uint32_t>(0x454 + 0x60 * i)[0] = (r.next() & 0xd00) | (r.range(0, 3) ? 0 : 0x800) | r.range(0, 3);
                              }
                              arena<int32_t>(0x10)[0] = r.range(0, 5), arena<uint32_t>(0x14)[0] = r.next() & 0x1ff;
                          },
                          [](void *fn) { ((void(__thiscall *)(void *, int32_t, uint32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<uint32_t>(0x14)[0]); return Result{}; }};
    quick(nodes_mark_chain);
    // String::CompareRaw(a, b): missing, empty and short texts over a few characters (high ones too)
    Test string_compare_raw{0xcd5750,
                            [=](Rng &r) {
                                static const char chars[] = {'a', 'b', (char)0xe9};
                                for (int s = 0; s < 2; s++) {
                                    const int n = r.range(0, 4);
                                    for (int i = 0; i < n; i++) arena<char>(0x200 + 0x20 * s)[i] = chars[r.range(0, 2)];
                                    arena<char>(0x200 + 0x20 * s)[n] = 0;
                                    arena<uint32_t>(0x10)[s] = r.range(0, 5) ? (uint32_t)(uintptr_t)arena<void>(0x200 + 0x20 * s) : 0;
                                }
                            },
                            [](void *fn) { return Result{((uint32_t(__stdcall *)(void *, void *))fn)((void *)(uintptr_t)arena<uint32_t>(0x10)[0], (void *)(uintptr_t)arena<uint32_t>(0x10)[1])}; }};
    quick(string_compare_raw);
    // Key::FromSmallList(list, n): 0-5 numbers of any sign
    Test key_from_small_list{0x9f8430,
                             [=](Rng &r) { for (int i = 0; i < 6; i++) arena<int32_t>(0x200)[i] = r.range(0, 5) ? r.range(-3, 40) : (int32_t)r.next(); arena<int32_t>(0x10)[0] = r.range(0, 5); },
                             [](void *fn) {
                                 const uint64_t k = ((uint64_t(__stdcall *)(void *, int32_t))fn)(arena<void>(0x200), arena<int32_t>(0x10)[0]);
                                 return Result{(uint32_t)k ^ (uint32_t)(k >> 32) * 0x9e3779b9u};
                             }};
    quick(key_from_small_list);
    // GridMarks::MarkRect0(this, x0, y0, x1, y1): a 6x5 grid, rectangles in and past it
    Test grid_marks_mark_rect0{0x874cf0,
                               [=](Rng &r) {
                                   arena<int32_t>(0x148)[0] = 6, arena<int32_t>(0x14c)[0] = 5, arena<uint32_t>(0x150)[0] = (uint32_t)(uintptr_t)arena<void>(0x1000);
                                   for (int i = 0; i < 4; i++) arena<int32_t>(0x154)[i] = r.range(-2, 8);
                                   for (int i = 0; i < 30; i++) arena<uint8_t>(0x1000 + 0x34 * i)[0] = (uint8_t)r.next();
                                   for (int i = 0; i < 4; i++) arena<int32_t>(0x10)[i] = r.range(-3, 9);
                               },
                               [](void *fn) {
                                   const int32_t *a = arena<int32_t>(0x10);
                                   ((void(__thiscall *)(void *, int32_t, int32_t, int32_t, int32_t))fn)(arena<void>(0x100), a[0], a[1], a[2], a[3]);
                                   return Result{};
                               }};
    quick(grid_marks_mark_rect0);
    // Key::Compare(a, b): keys differing in one value (or none), unsigned and signed
    Test key_compare{0x9731c0,
                     [=](Rng &r) {
                         uint32_t *a = arena<uint32_t>(0x10), *b = arena<uint32_t>(0x40);
                         for (int i = 0; i < 3; i++) arena<int32_t>(0x200)[i] = r.range(-2, 2);
                         for (int i = 0; i < 6; i++) a[i] = r.range(0, 3) ? (uint32_t)r.range(-2, 2) : r.next();
                         a[2] = (uint32_t)(uintptr_t)arena<void>(0x200);
                         for (int i = 0; i < 6; i++) b[i] = a[i];
                         for (int i = 0; i < 3; i++) arena<int32_t>(0x220)[i] = arena<int32_t>(0x200)[i];
                         b[2] = (uint32_t)(uintptr_t)arena<void>(0x220);
                         const int which = r.range(0, 8);
                         if (which < 6 && which != 2) b[which] = r.range(0, 1) ? (uint32_t)r.range(-2, 2) : r.next();
                         else if (which < 9 && which >= 6) arena<int32_t>(0x220)[which - 6] = r.range(-3, 3);
                     },
                     [](void *fn) {
                         const uint32_t *a = arena<uint32_t>(0x10), *b = arena<uint32_t>(0x40);
                         return Result{((uint32_t(__stdcall *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))fn)(
                             a[0], a[1], a[2], a[3], a[4], a[5], b[0], b[1], b[2], b[3], b[4], b[5])};
                     }};
    quick(key_compare);
    // Vertex::SizeFromFormat(format): any bits
    Test vertex_size_from_format{0xbd4060,
                                 [=](Rng &r) { arena<uint32_t>(0x10)[0] = r.next(); },
                                 [](void *fn) { return Result{((uint32_t(__fastcall *)(uint32_t))fn)(arena<uint32_t>(0x10)[0])}; }};
    quick(vertex_size_from_format);
    // PhysicsWorld::AtRest(this): 0-4 particles, the game's limit set for the call and put back
    static uint32_t rest_limit_saved;
    Test physics_world_at_rest{0xa6ef50,
                               [=](Rng &r) {
                                   const int n = r.range(0, 4);
                                   arena<int32_t>(0x110)[0] = n, arena<uint32_t>(0x10c)[0] = (uint32_t)(uintptr_t)arena<void>(0x200);
                                   for (int i = 0; i < n; i++) {
                                       arena<uint32_t>(0x200)[i] = (uint32_t)(uintptr_t)arena<void>(0x400 + 0x40 * i);
                                       for (int j = 4; j < 7; j++) {
                                           const float p = r.real(-50, 50);
                                           arena<float>(0x400 + 0x40 * i)[j] = p, arena<float>(0x400 + 0x40 * i)[j + 4] = r.range(0, 9) ? p + r.real(-0.02f, 0.02f) : geo_pick(r);
                                       }
                                   }
                                   arena<float>(0x10)[0] = r.range(0, 9) ? r.real(0, 2) : geo_pick(r);
                               },
                               [](void *fn) {
                                   rest_limit_saved = *(volatile uint32_t *)0x13c2a04;
                                   *(volatile float *)0x13c2a04 = arena<float>(0x10)[0];
                                   const uint32_t got = ((uint8_t(__thiscall *)(void *))fn)(arena<void>(0x100));
                                   *(volatile uint32_t *)0x13c2a04 = rest_limit_saved;
                                   return Result{got};
                               }};
    quick(physics_world_at_rest);
    // Terrain::CellColourAt(this, x, y, h, out): a 3x3 grid, chains of layers through extra ones
    Test terrain_cell_colour_at{0xafa0e0,
                                [=](Rng &r) {
                                    arena<int32_t>(0x130)[0] = 3, arena<int32_t>(0x12c)[0] = 3, arena<uint32_t>(0x114)[0] = (uint32_t)(uintptr_t)arena<void>(0x1000);
                                    for (int i = 0; i < 14; i++) {
                                        uint8_t *e = arena<uint8_t>(0x1000 + 0x2c * i);
                                        const float lo = r.real(0, 20);
                                        *(float *)e = lo, *(float *)(e + 4) = lo + r.real(0, 8), *(uint32_t *)(e + 0x10) = r.next() & 3;
                                        *(int32_t *)(e + 0xc) = r.range(0, 2) ? -1 : r.range(9, 13), *(uint32_t *)(e + 0x24) = r.next();
                                    }
                                    for (int i = 9; i < 14; i++) if (arena<int32_t>(0x100c + 0x2c * i)[0] <= i) arena<int32_t>(0x100c + 0x2c * i)[0] = -1;
                                    arena<int32_t>(0x10)[0] = r.range(-1, 3), arena<int32_t>(0x10)[1] = r.range(-1, 3), arena<float>(0x18)[0] = r.range(0, 9) ? r.real(0, 30) : geo_pick(r);
                                    arena<uint32_t>(0x20)[0] = r.next();
                                },
                                [](void *fn) {
                                    return Result{(uint32_t)((uint8_t(__thiscall *)(void *, int32_t, int32_t, float, void *))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<int32_t>(0x10)[1], arena<float>(0x18)[0], arena<void>(0x20))};
                                }};
    quick(terrain_cell_colour_at);
    // Segment::Middle(this, out): an object with both ends
    Test segment_middle{0xbc6140,
                        [=](Rng &r) {
                            arena<uint32_t>(0x118)[0] = 0x400000 | (r.next() & ~0x400000u), arena<int32_t>(0x1e4)[0] = r.range(2, 5);
                            arena<uint32_t>(0x210)[0] = (uint32_t)(uintptr_t)arena<void>(0x300), arena<uint32_t>(0x20c)[0] = (uint32_t)(uintptr_t)arena<void>(0x320);
                            arena<uint32_t>(0x304)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<uint32_t>(0x324)[0] = (uint32_t)(uintptr_t)arena<void>(0x440);
                            for (int i = 0; i < 3; i++) arena<float>(0x400)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r), arena<float>(0x440)[i] = r.range(0, 9) ? r.real(-500, 500) : geo_pick(r);
                            for (int i = 0; i < 3; i++) arena<uint32_t>(0x20)[i] = r.next();
                        },
                        [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x20)); return Result{}; }};
    quick(segment_middle);
    // Screen::AreaScale(out): the GUI area and screen (game globals, set for the call and put back)
    static uint32_t area_saved[2];
    Test screen_area_scale{0x6acb30,
                           [=](Rng &r) {
                               arena<float>(0x27c)[0] = r.real(0, 50), arena<float>(0x280)[0] = r.real(0, 50), arena<float>(0x284)[0] = r.range(0, 9) ? r.real(0, 900) : geo_pick(r), arena<float>(0x288)[0] = r.real(0, 900);
                               arena<int32_t>(0x304)[0] = r.range(0, 9) ? r.range(1, 2000) : (int32_t)r.next(), arena<int32_t>(0x308)[0] = r.range(0, 9) ? r.range(1, 2000) : (int32_t)r.next();
                               arena<uint32_t>(0x10)[0] = r.range(0, 5) ? (uint32_t)(uintptr_t)arena<void>(0x200) : 0;
                               arena<uint32_t>(0x20)[0] = r.next(), arena<uint32_t>(0x24)[0] = r.next();
                           },
                           [](void *fn) {
                               area_saved[0] = *(volatile uint32_t *)0x12f83f0, area_saved[1] = *(volatile uint32_t *)0x12f4618;
                               *(volatile uint32_t *)0x12f83f0 = arena<uint32_t>(0x10)[0], *(volatile uint32_t *)0x12f4618 = (uint32_t)(uintptr_t)arena<void>(0x300);
                               void *got = ((void *(__fastcall *)(void *))fn)(arena<void>(0x20));
                               *(volatile uint32_t *)0x12f83f0 = area_saved[0], *(volatile uint32_t *)0x12f4618 = area_saved[1];
                               return Result{(uint32_t)(got == arena<void>(0x20))};
                           }};
    quick(screen_area_scale);
    // Particles::SetCount(this, f): detail on or off, fades in and out of 0-1, small counts
    Test particles_set_count{0x9cbbb0,
                             [=](Rng &r) {
                                 arena<int32_t>(0x164)[0] = r.range(0, 3) ? r.range(-2, 3) : r.range(0, 200), arena<uint32_t>(0x168)[0] = r.range(0, 1);
                                 arena<uint32_t>(0x1a4)[0] = (uint32_t)(uintptr_t)arena<void>(0x300), arena<float>(0x390)[0] = r.range(0, 9) ? r.real(0, 2) : geo_pick(r);
                                 arena<float>(0x1b8)[0] = r.range(0, 9) ? r.real(-0.5f, 1.5f) : geo_pick(r), arena<float>(0x1a8)[0] = r.range(0, 9) ? r.real(0, 50) : geo_pick(r);
                                 arena<uint32_t>(0x1bc)[0] = r.next(), arena<uint32_t>(0x1c0)[0] = r.next();
                                 arena<float>(0x10)[0] = r.range(0, 9) ? r.real(0, 2) : geo_pick(r);
                             },
                             [](void *fn) { ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x10)[0]); return Result{}; }};
    quick(particles_set_count);
    // Ease::InOut(t, total)
    Test ease_in_out{0x5d3850,
                     [=](Rng &r) {
                         const float total = r.range(0, 9) ? r.real(0, 12) : geo_pick(r);
                         arena<float>(0x10)[0] = r.range(0, 9) ? total * r.real(0, 1) : geo_pick(r), arena<float>(0x10)[1] = total;
                     },
                     [](void *fn) {
                         const float v = (float)((long double(__stdcall *)(float, float))fn)(arena<float>(0x10)[0], arena<float>(0x10)[1]);
                         uint32_t bits;
                         memcpy(&bits, &v, 4);
                         return Result{bits};
                     }};
    quick(ease_in_out);
    // SinCos::Lookup(table, angle, a, b): angles round several turns, signalling NaNs in the table now and then
    Test sin_cos_lookup{0x7c4690,
                        [=](Rng &r) {
                            for (int i = 0; i < 256; i++) arena<uint32_t>(0x1000)[i] = r.range(0, 30) ? r.next() : 0x7f800001u + r.range(0, 99);
                            arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-30, 30) : geo_pick(r);
                            arena<uint32_t>(0x20)[0] = r.next(), arena<uint32_t>(0x24)[0] = r.next();
                        },
                        [](void *fn) {
                            ((void(__thiscall *)(void *, float, void *, void *))fn)(arena<void>(0x1000), arena<float>(0x10)[0], arena<void>(0x20), arena<void>(0x24));
                            return Result{};
                        }};
    quick(sin_cos_lookup);
    // Entries::CountNegative(this): 0-11 entries (the four-at-a-time part and the rest)
    Test entries_count_negative{0x46c6c0,
                                [=](Rng &r) {
                                    arena<int32_t>(0x170)[0] = r.range(0, 11), arena<uint32_t>(0x16c)[0] = (uint32_t)(uintptr_t)arena<void>(0x1000);
                                    for (int i = 0; i < 11; i++) arena<float>(0x1040 + 0x50 * i)[0] = r.range(0, 9) ? r.real(-5, 5) : geo_pick(r);
                                },
                                [](void *fn) { return Result{((uint32_t(__thiscall *)(void *))fn)(arena<void>(0x100))}; }};
    quick(entries_count_negative);
    // Box::ClosestPoint(out, min, max, p): signalling NaNs now and then
    Test box_closest_point{0x88cfd0,
                           [=](Rng &r) {
                               for (int i = 0; i < 3; i++) {
                                   const float lo = r.real(-50, 50);
                                   arena<float>(0x100)[i] = lo, arena<float>(0x110)[i] = lo + r.real(0, 40), arena<float>(0x120)[i] = r.range(0, 9) ? r.real(-80, 100) : geo_pick(r);
                               }
                               if (r.range(0, 9) == 0) arena<uint32_t>(0x100 + 0x10 * r.range(0, 2))[r.range(0, 2)] = 0x7f800001u + r.range(0, 99);
                               for (int i = 0; i < 3; i++) arena<uint32_t>(0x20)[i] = r.next();
                           },
                           [](void *fn) {
                               return Result{(uint32_t)(((void *(__fastcall *)(void *, void *, void *, void *))fn)(arena<void>(0x20), arena<void>(0x100), arena<void>(0x110), arena<void>(0x120)) == arena<void>(0x20))};
                           }};
    quick(box_closest_point);
    // PairMap::Find(this, a, b): 0-9 sorted pairs, pairs in and out
    Test pair_map_find{0xca45c0,
                       [=](Rng &r) {
                           const int32_t n = r.range(0, 9);
                           arena<int32_t>(0x114)[0] = n, arena<uint32_t>(0x120)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                           int32_t a = r.range(-3, 3), b = r.range(-3, 3);
                           for (int i = 0; i < n; i++) {
                               arena<int32_t>(0x400 + 12 * i)[0] = a, arena<int32_t>(0x404 + 12 * i)[0] = b, arena<uint32_t>(0x408 + 12 * i)[0] = r.next();
                               if (r.range(0, 1)) b += r.range(1, 2); else a += r.range(1, 2), b = r.range(-3, 3);
                           }
                           const int pick = r.range(0, n);
                           arena<int32_t>(0x10)[0] = pick < n ? arena<int32_t>(0x400 + 12 * pick)[0] : r.range(-4, 12), arena<int32_t>(0x10)[1] = pick < n ? arena<int32_t>(0x404 + 12 * pick)[0] : r.range(-4, 6);
                       },
                       [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, int32_t, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<int32_t>(0x10)[1])}; }};
    quick(pair_map_find);
    // Slots::Reserve(this): 0-12 slots of kinds 0-6
    Test slots_reserve{0xa517c0,
                       [=](Rng &r) {
                           arena<int32_t>(0x110)[0] = r.range(0, 12), arena<uint32_t>(0x10c)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<int32_t>(0x114)[0] = r.range(-3, 2);
                           for (int i = 0; i < 12; i++) arena<int32_t>(0x400)[2 * i] = r.range(0, 1) ? r.range(3, 5) : r.range(0, 6), arena<uint32_t>(0x404)[2 * i] = r.next();
                       },
                       [](void *fn) { ((void(__thiscall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(slots_reserve);
    // WideString::CompareRaw(a, b): missing, empty and short texts, a string length shorter than its text
    Test wide_string_compare_raw{0xcd60e0,
                                 [=](Rng &r) {
                                     static const uint16_t chars[] = {'a', 'b', 0xe9, 0};
                                     arena<uint32_t>(0x200)[0] = (uint32_t)(uintptr_t)arena<void>(0x300);
                                     arena<uint32_t>(0x304)[0] = r.range(0, 4), arena<uint32_t>(0x30c)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                                     for (int i = 0; i < 5; i++) arena<uint16_t>(0x400)[i] = chars[r.range(0, 9) ? r.range(0, 2) : 3];
                                     const int n = r.range(0, 4);
                                     for (int i = 0; i < n; i++) arena<uint16_t>(0x500)[i] = r.range(0, 2) ? arena<uint16_t>(0x400)[i] : chars[r.range(0, 2)];
                                     arena<uint16_t>(0x500)[n] = 0;
                                     arena<uint32_t>(0x10)[0] = r.range(0, 5) ? (uint32_t)(uintptr_t)arena<void>(0x200) : 0, arena<uint32_t>(0x10)[1] = r.range(0, 5) ? (uint32_t)(uintptr_t)arena<void>(0x500) : 0;
                                 },
                                 [](void *fn) { return Result{((uint32_t(__stdcall *)(void *, void *))fn)((void *)(uintptr_t)arena<uint32_t>(0x10)[0], (void *)(uintptr_t)arena<uint32_t>(0x10)[1])}; }};
    quick(wide_string_compare_raw);
    // Grid::SpiralStep(grid, cx, cy, dx, dy): small grids, offsets round the spiral
    Test grid_spiral_step{0xb35000,
                          [=](Rng &r) {
                              arena<uint8_t>(0x104)[0] = (uint8_t)r.range(1, 6), arena<uint8_t>(0x105)[0] = (uint8_t)r.range(1, 6);
                              arena<int32_t>(0x10)[0] = r.range(-1, 6), arena<int32_t>(0x10)[1] = r.range(-1, 6), arena<int32_t>(0x20)[0] = r.range(-4, 4), arena<int32_t>(0x24)[0] = r.range(-4, 4);
                          },
                          [](void *fn) {
                              return Result{((uint32_t(__fastcall *)(void *, int32_t, int32_t, void *, void *))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<int32_t>(0x10)[1], arena<void>(0x20), arena<void>(0x24))};
                          }};
    quick(grid_spiral_step);
    // Angle::TurnToward(this, target, step, tolerance): the low byte
    Test angle_turn_toward{0x7c28a0,
                           [=](Rng &r) {
                               arena<float>(0x210)[0] = r.range(0, 9) ? r.real(-4, 4) : geo_pick(r);
                               arena<float>(0x10)[0] = r.range(0, 5) == 0 ? arena<float>(0x210)[0] : r.range(0, 9) ? r.real(-4, 4) : geo_pick(r);
                               arena<float>(0x10)[1] = r.range(0, 9) ? r.real(0, 1) : geo_pick(r), arena<float>(0x10)[2] = r.range(0, 9) ? r.real(0, 0.3f) : geo_pick(r);
                           },
                           [](void *fn) {
                               const float *a = arena<float>(0x10);
                               return Result{(uint32_t)((uint8_t(__thiscall *)(void *, float, float, float))fn)(arena<void>(0x100), a[0], a[1], a[2])};
                           }};
    quick(angle_turn_toward);
    // Box::Overshoot2D(out, p, min, max)
    Test box_overshoot_2d{0x9f8340,
                          [=](Rng &r) {
                              for (int i = 0; i < 3; i++) {
                                  const float lo = r.real(-50, 50);
                                  arena<float>(0x100)[i] = lo, arena<float>(0x110)[i] = lo + r.real(0, 40), arena<float>(0x120)[i] = r.range(0, 9) ? r.real(-80, 100) : geo_pick(r);
                              }
                              for (int i = 0; i < 3; i++) arena<uint32_t>(0x20)[i] = r.next();
                          },
                          [](void *fn) { ((void(__stdcall *)(void *, void *, void *, void *))fn)(arena<void>(0x20), arena<void>(0x120), arena<void>(0x100), arena<void>(0x110)); return Result{}; }};
    quick(box_overshoot_2d);
    // Ticker::Update(this): timers either side of 0.25 and 0.5, kinds 0-4
    Test ticker_update{0x7ea210,
                       [=](Rng &r) {
                           arena<float>(0x5d8)[0] = r.range(0, 9) ? r.real(0, 1) : geo_pick(r), arena<float>(0x5dc)[0] = r.range(0, 9) ? r.real(-5, 5) : geo_pick(r);
                           arena<uint32_t>(0x514)[0] = r.range(0, 4), arena<uint32_t>(0x510)[0] = r.next();
                       },
                       [](void *fn) { ((void(__thiscall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(ticker_update);
    // Polygon::Area(this): 0-11 points (the four-at-a-time part and the rest)
    Test polygon_area{0x90c170,
                      [=](Rng &r) {
                          arena<int32_t>(0x128)[0] = r.range(0, 9) ? r.range(0, 11) : -1, arena<uint32_t>(0x124)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                          for (int i = 0; i < 22; i++) arena<float>(0x400)[i] = r.range(0, 19) ? r.real(-100, 100) : geo_pick(r);
                      },
                      [](void *fn) {
                          const float v = (float)((long double(__thiscall *)(void *))fn)(arena<void>(0x100));
                          uint32_t bits;
                          memcpy(&bits, &v, 4);
                          return Result{bits};
                      }};
    quick(polygon_area);
    // Entries::Highest(this): 0-11 entries, equal values now and then
    Test entries_highest{0xc35e50,
                         [=](Rng &r) {
                             arena<int32_t>(0x1b4)[0] = r.range(0, 11), arena<uint32_t>(0x1a0)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                             for (int i = 0; i < 11; i++) arena<float>(0x404 + 12 * i)[0] = r.range(0, 9) ? (float)r.range(-3, 3) : geo_pick(r);
                         },
                         [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *))fn)(arena<void>(0x100))}; }};
    quick(entries_highest);
    // Mesh::SetPrimitiveCount(this, n): types 0-7, n from the buffers or given
    Test mesh_set_primitive_count{0xbe8ba0,
                                  [=](Rng &r) {
                                      for (int i = 0; i < 0x14; i++) arena<uint32_t>(0x100)[i] = r.next();
                                      arena<uint16_t>(0x106)[0] = (uint16_t)r.range(0, 7);
                                      arena<uint32_t>(0x11c)[0] = r.range(0, 1) ? (uint32_t)(uintptr_t)arena<void>(0x300) : 0, arena<uint32_t>(0x14c)[0] = r.range(0, 1);
                                      arena<uint32_t>(0x130)[0] = (uint32_t)(uintptr_t)arena<void>(0x340);
                                      arena<int32_t>(0x308)[0] = r.range(-3, 30), arena<int32_t>(0x350)[0] = r.range(-3, 30);
                                      arena<uint32_t>(0x10)[0] = r.range(0, 2) ? 0x40000000u : r.range(0, 1) ? (uint32_t)r.range(-3, 30) : r.next();
                                  },
                                  [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0]); return Result{}; }};
    quick(mesh_set_primitive_count);
    // VerletPoints::AllSettled(this, speed): 0-4 points, the game's limit set for the call and put back
    static uint32_t settle_limit_saved;
    Test verlet_points_all_settled{0xa6eff0,
                                   [=](Rng &r) {
                                       const int n = r.range(0, 4);
                                       arena<int32_t>(0x110)[0] = n, arena<uint32_t>(0x10c)[0] = (uint32_t)(uintptr_t)arena<void>(0x200);
                                       for (int i = 0; i < n; i++) {
                                           arena<uint32_t>(0x200)[i] = (uint32_t)(uintptr_t)arena<void>(0x400 + 0x40 * i);
                                           for (int j = 4; j < 7; j++) {
                                               const float p = r.real(-50, 50);
                                               arena<float>(0x400 + 0x40 * i)[j] = p, arena<float>(0x400 + 0x40 * i)[j + 4] = r.range(0, 9) ? p + r.real(-0.02f, 0.02f) : geo_pick(r);
                                           }
                                       }
                                       arena<float>(0x10)[0] = r.range(0, 9) ? r.real(0, 2) : geo_pick(r), arena<uint32_t>(0x20)[0] = r.next();
                                   },
                                   [](void *fn) {
                                       settle_limit_saved = *(volatile uint32_t *)0x13c2a04;
                                       *(volatile float *)0x13c2a04 = arena<float>(0x10)[0];
                                       const uint32_t got = ((uint8_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x20));
                                       *(volatile uint32_t *)0x13c2a04 = settle_limit_saved;
                                       return Result{got};
                                   }};
    quick(verlet_points_all_settled);
    // Objects::AnyNear(p): the game's list (global, set for the call and put back) of 1-4 objects
    static uint32_t near_list_saved;
    Test objects_any_near{0x7a2180,
                          [=](Rng &r) {
                              const int n = r.range(1, 4);
                              for (int i = 0; i <= n; i++) arena<uint32_t>(0x300 + 0x10 * i)[0] = i < n ? (uint32_t)(uintptr_t)arena<void>(0x300 + 0x10 * (i + 1)) : 0;
                              for (int i = 0; i < n; i++) {
                                  arena<uint32_t>(0x308 + 0x10 * i)[0] = (uint32_t)(uintptr_t)arena<void>(0x1000 + 0x100 * i);
                                  for (int j = 0; j < 3; j++) arena<float>(0x1080 + 0x100 * i)[j] = r.range(0, 9) ? r.real(-30, 30) : geo_pick(r);
                              }
                              for (int j = 0; j < 3; j++) arena<float>(0x10)[j] = r.real(-30, 30);
                          },
                          [](void *fn) {
                              near_list_saved = *(volatile uint32_t *)0x10efb90;
                              *(volatile uint32_t *)0x10efb90 = (uint32_t)(uintptr_t)arena<void>(0x300);
                              const uint32_t got = ((uint8_t(__fastcall *)(void *))fn)(arena<void>(0x10));
                              *(volatile uint32_t *)0x10efb90 = near_list_saved;
                              return Result{got};
                          }};
    quick(objects_any_near);
    // TileCorners::Count(this, a, b, c): flagged, the even patterns and mixed ones
    Test tile_corners_count{0x727b00,
                            [=](Rng &r) {
                                arena<uint8_t>(0x134)[0] = r.range(0, 4) ? 0 : (uint8_t)r.range(1, 255);
                                arena<uint16_t>(0x132)[0] = r.range(0, 1) ? (uint16_t)(0x1111 * r.range(0, 7)) : (uint16_t)r.next();
                                for (int i = 0; i < 3; i++) arena<uint16_t>(0x20)[2 * i] = (uint16_t)r.next();
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(void *, void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x20), arena<void>(0x24), arena<void>(0x28));
                                return Result{};
                            }};
    quick(tile_corners_count);
    // Entry::Compare(a, b): kinds 0-3, entries differing in one value (or none)
    Test entry_compare{0x84d2d0,
                       [=](Rng &r) {
                           int32_t *a = arena<int32_t>(0x100), *b = arena<int32_t>(0x200);
                           for (int i = 0; i < 7; i++) a[i] = r.range(0, 2) ? r.range(-2, 2) : (int32_t)r.next();
                           a[2] = r.range(0, 3), ((uint8_t *)a)[0x18] = r.range(0, 1) ? 0 : (uint8_t)r.range(1, 255);
                           for (int i = 0; i < 7; i++) b[i] = a[i];
                           const int which = r.range(0, 7);
                           if (which < 6) b[which] = which == 2 ? r.range(0, 3) : r.range(0, 1) ? r.range(-2, 2) : (int32_t)r.next();
                           else if (which == 6) ((uint8_t *)b)[0x18] = r.range(0, 1) ? 0 : (uint8_t)r.range(1, 255);
                       },
                       [](void *fn) { return Result{((uint32_t(__stdcall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200))}; }};
    quick(entry_compare);
    // Lists::Remove(this, item): two lists of 0-5 items with repeats
    Test lists_remove{0xa6bfd0,
                      [=](Rng &r) {
                          arena<uint32_t>(0x16c)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<int32_t>(0x174)[0] = r.range(0, 5);
                          arena<uint32_t>(0x178)[0] = (uint32_t)(uintptr_t)arena<void>(0x440), arena<int32_t>(0x180)[0] = r.range(0, 5);
                          for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x400)[i] = r.range(0, 1) ? r.range(0, 4) : r.next();
                          arena<uint32_t>(0x10)[0] = r.range(0, 4);
                      },
                      [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0]); return Result{}; }};
    quick(lists_remove);
    // SortedTable::Find20, SortedArray::Find, SortedTable::Find8 (key, table, count, place, -): 0-9 sorted unsigned numbers
    for (const uint32_t at : {0x90c5c0u, 0x801450u, 0x4453d0u}) {
        Test sorted_table_find{at,
                               [=](Rng &r) {
                                   const int stride = at == 0x90c5c0u ? 20 : at == 0x801450u ? 4 : 8;
                                   const int32_t n = r.range(0, 9);
                                   uint32_t v = r.range(0, 1) ? r.range(0, 5) : 0x7ffffffcu + r.range(0, 5);
                                   for (int i = 0; i < n; i++) arena<uint32_t>(0x400 + stride * i)[0] = v, v += r.range(0, 3);
                                   const int pick = r.range(0, n);
                                   arena<uint32_t>(0x10)[0] = pick < n ? arena<uint32_t>(0x400 + stride * pick)[0] : v - r.range(0, 30);
                                   arena<int32_t>(0x14)[0] = r.range(0, 9) ? n : -r.range(0, 2), arena<uint32_t>(0x20)[0] = r.next();
                               },
                               [](void *fn) {
                                   return Result{(uint32_t)((uint8_t(__fastcall *)(void *, void *, int32_t, void *, uint32_t))fn)(arena<void>(0x10), arena<void>(0x400), arena<int32_t>(0x14)[0], arena<void>(0x20), 0)};
                               }};
        quick(sorted_table_find);
    }
    // SortedPointers::FindBy8(key, list, count, place, -)
    Test sorted_pointers_find_by8{0xa90980,
                                  [=](Rng &r) {
                                      const int32_t n = r.range(0, 9);
                                      uint32_t v = r.range(0, 1) ? r.range(0, 5) : 0x7ffffffcu + r.range(0, 5);
                                      for (int i = 0; i < n; i++) arena<uint32_t>(0x400)[i] = (uint32_t)(uintptr_t)arena<void>(0x800 + 16 * i), arena<uint32_t>(0x808 + 16 * i)[0] = v, v += r.range(0, 3);
                                      const int pick = r.range(0, n);
                                      arena<uint32_t>(0x10)[0] = (uint32_t)(uintptr_t)arena<void>(0x200), arena<uint32_t>(0x208)[0] = pick < n ? arena<uint32_t>(0x808 + 16 * pick)[0] : v - r.range(0, 30);
                                      arena<int32_t>(0x14)[0] = r.range(0, 9) ? n : -r.range(0, 2), arena<uint32_t>(0x20)[0] = r.next();
                                  },
                                  [](void *fn) {
                                      return Result{(uint32_t)((uint8_t(__fastcall *)(void *, void *, int32_t, void *, uint32_t))fn)(arena<void>(0x10), arena<void>(0x400), arena<int32_t>(0x14)[0], arena<void>(0x20), 0)};
                                  }};
    quick(sorted_pointers_find_by8);
    // SortedTable::Find64(this, key_lo, key_hi, place): 0-8 sorted signed 64-bit values
    Test sorted_table_find64{0x9f9130,
                             [=](Rng &r) {
                                 const int32_t n = r.range(0, 8);
                                 arena<int32_t>(0x18c)[0] = n, arena<uint32_t>(0x184)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                                 int64_t v = (int64_t)r.range(-5, 5) << (r.range(0, 1) ? 32 : 0);
                                 for (int i = 0; i < n; i++) {
                                     arena<uint32_t>(0x400)[i] = (uint32_t)(uintptr_t)arena<void>(0x800 + 8 * i), *arena<int64_t>(0x800 + 8 * i) = v;
                                     v += r.range(0, 3) == 0 ? 0 : r.range(0, 1) ? (int64_t)r.range(1, 3) << 32 : r.range(1, 3);
                                 }
                                 *arena<int64_t>(0x10) = r.range(0, 2) && n ? *arena<int64_t>(0x800 + 8 * r.range(0, n - 1)) : ((int64_t)r.range(-6, 9) << 32) + r.range(-3, 9);
                                 arena<uint32_t>(0x20)[0] = r.next();
                             },
                             [](void *fn) {
                                 return Result{(uint32_t)((uint8_t(__thiscall *)(void *, uint32_t, int32_t, void *))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], arena<int32_t>(0x10)[1], arena<void>(0x20))};
                             }};
    quick(sorted_table_find64);
    // Pairs::FindSorted(list, a, b, count): 0-9 sorted pairs
    Test pairs_find_sorted{0xc22b60,
                           [=](Rng &r) {
                               const int32_t n = r.range(0, 9);
                               int32_t a = r.range(-3, 3), b = r.range(-3, 3);
                               for (int i = 0; i < n; i++) {
                                   arena<int32_t>(0x400)[2 * i] = a, arena<int32_t>(0x404)[2 * i] = b;
                                   if (r.range(0, 1)) b += r.range(1, 2); else a += r.range(1, 2), b = r.range(-3, 3);
                               }
                               const int pick = r.range(0, n);
                               arena<int32_t>(0x10)[0] = pick < n ? arena<int32_t>(0x400)[2 * pick] : r.range(-4, 12), arena<int32_t>(0x10)[1] = pick < n ? arena<int32_t>(0x404)[2 * pick] : r.range(-4, 6);
                               arena<int32_t>(0x18)[0] = r.range(0, 9) ? n : -r.range(1, 2);
                           },
                           [](void *fn) {
                               return Result{((uint32_t(__thiscall *)(void *, int32_t, int32_t, int32_t))fn)(arena<void>(0x400), arena<int32_t>(0x10)[0], arena<int32_t>(0x10)[1], arena<int32_t>(0x18)[0])};
                           }};
    quick(pairs_find_sorted);
    // SortedPointers::FindXY2 and Whirlpool::FindCell (this, x, y, place): 0-8 entries sorted by x then y (or row then column)
    for (const uint32_t at : {0x8feda0u, 0x8fed20u}) {
        Test find_xy2{at,
                      [=](Rng &r) {
                          const int32_t n = r.range(0, 8);
                          const bool rows = at == 0x8fed20u;
                          arena<uint32_t>(rows ? 0x110 : 0x114)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<int32_t>(0x118)[0] = n;
                          int32_t first = r.range(-3, 3), second = r.range(-3, 3);
                          for (int i = 0; i < n; i++) {
                              arena<uint32_t>(0x400)[i] = (uint32_t)(uintptr_t)arena<void>(0x800 + 16 * i);
                              arena<int32_t>(0x800 + 16 * i)[rows ? 1 : 0] = first, arena<int32_t>(0x800 + 16 * i)[rows ? 0 : 1] = second;
                              if (r.range(0, 1)) second += r.range(1, 2); else first += r.range(1, 2), second = r.range(-3, 3);
                          }
                          const int pick = n ? r.range(0, n) : 0;
                          arena<int32_t>(0x10)[0] = pick < n ? arena<int32_t>(0x800 + 16 * pick)[0] : r.range(-4, 10), arena<int32_t>(0x10)[1] = pick < n ? arena<int32_t>(0x804 + 16 * pick)[0] : r.range(-4, 10);
                          arena<uint32_t>(0x18)[0] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x30) : 0, arena<uint32_t>(0x30)[0] = r.next();
                      },
                      [](void *fn) {
                          return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, int32_t, int32_t, void *))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<int32_t>(0x10)[1], (void *)(uintptr_t)arena<uint32_t>(0x18)[0])};
                      }};
        quick(find_xy2);
    }
    // Frame::PickAxis(this, out, frame)
    Test frame_pick_axis{0x581530,
                         [=](Rng &r) { arena<uint32_t>(0x104)[0] = r.range(0, 1) ? 0 : r.next(); for (int i = 0; i < 12; i++) arena<uint32_t>(0x200)[i] = r.next(), arena<uint32_t>(0x20)[i % 3] = r.next(); },
                         [](void *fn) { return Result{(uint32_t)(((void *(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x20), arena<void>(0x200)) == arena<void>(0x20))}; }};
    quick(frame_pick_axis);
    // Mesh::CountForId(this, id, vertices, indices): 0-6 parts with ids 0-2 and every shape bit pattern
    Test mesh_count_for_id{0x95b610,
                           [=](Rng &r) {
                               arena<int32_t>(0x11c)[0] = r.range(0, 6), arena<uint32_t>(0x114)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                               for (int i = 0; i < 6; i++) arena<uint16_t>(0x408 + 16 * i)[0] = (uint16_t)r.range(0, 2), arena<uint8_t>(0x40a + 16 * i)[0] = (uint8_t)r.next();
                               arena<uint32_t>(0x10)[0] = r.range(0, 9) ? r.range(0, 2) : 0x10001u, arena<uint32_t>(0x20)[0] = r.next(), arena<uint32_t>(0x24)[0] = r.next();
                           },
                           [](void *fn) { ((void(__thiscall *)(void *, uint32_t, void *, void *))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], arena<void>(0x20), arena<void>(0x24)); return Result{}; }};
    quick(mesh_count_for_id);
    // Pairs::IndexOf(this, a, b): 0-4 ids with 0-3 partners each
    Test pairs_index_of{0xbc2a30,
                        [=](Rng &r) {
                            const int n = r.range(0, 4);
                            arena<uint32_t>(0x108)[0] = (uint32_t)n | (r.range(0, 1) ? r.next() & 0xffff0000u : 0);
                            if (arena<uint32_t>(0x108)[0] > 4) arena<uint32_t>(0x108)[0] = n;
                            arena<uint32_t>(0x124)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<uint32_t>(0x118)[0] = (uint32_t)(uintptr_t)arena<void>(0x440), arena<uint32_t>(0x128)[0] = (uint32_t)(uintptr_t)arena<void>(0x480);
                            for (int i = 0; i < 4; i++) arena<uint16_t>(0x400)[i] = (uint16_t)r.range(0, 3), arena<uint16_t>(0x440)[2 * i] = (uint16_t)r.range(0, 3), arena<uint16_t>(0x442)[2 * i] = (uint16_t)r.range(0, 8);
                            for (int i = 0; i < 12; i++) arena<uint16_t>(0x480)[i] = (uint16_t)r.range(0, 3);
                            arena<uint32_t>(0x10)[0] = r.range(0, 3) | (r.next() & 0xffff0000u), arena<uint32_t>(0x14)[0] = r.range(0, 3) | (r.next() & 0xffff0000u);
                        },
                        [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], arena<uint32_t>(0x14)[0])}; }};
    quick(pairs_index_of);
    // Lights::ColourFor(this, out, id): 0-6 lights with ids 0-3
    Test lights_colour_for{0xa35440,
                           [=](Rng &r) {
                               arena<int32_t>(0x208)[0] = r.range(0, 6);
                               for (int i = 0; i < 6; i++) {
                                   arena<uint32_t>(0x1f0)[i] = r.range(0, 3), arena<uint32_t>(0x1d8)[i] = (uint32_t)(uintptr_t)arena<void>(0x400 + 0x100 * i);
                                   for (int j = 0; j < 4; j++) arena<uint32_t>(0x47c + 0x100 * i)[j] = r.next();
                               }
                               arena<uint32_t>(0x10)[0] = r.range(0, 3);
                               for (int j = 0; j < 4; j++) arena<uint32_t>(0x20)[j] = r.next();
                           },
                           [](void *fn) { return Result{(uint32_t)(((void *(__thiscall *)(void *, void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x20), arena<uint32_t>(0x10)[0]) == arena<void>(0x20))}; }};
    quick(lights_colour_for);
    // Keys::RemoveAt(this, n): lists of 2-8 entries
    Test keys_remove_at{0xe042b0,
                        [=](Rng &r) {
                            const int n = r.range(2, 8);
                            arena<int32_t>(0x114)[0] = n, arena<uint32_t>(0x110)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                            for (int i = 0; i < 24; i++) arena<uint32_t>(0x400)[i] = r.range(0, 100);
                            arena<int32_t>(0x10)[0] = r.range(0, n - 2);
                        },
                        [](void *fn) { ((void(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0]); return Result{}; }};
    quick(keys_remove_at);
    // Settings::Downgrade(this)
    Test settings_downgrade{0x5d13c0,
                            [=](Rng &r) { for (int i = 0; i < 10; i++) arena<int32_t>(0x100)[i] = r.range(0, 9); },
                            [](void *fn) { ((void(__thiscall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(settings_downgrade);
    // Box::ThinnestAxis(min, max): equal sides now and then
    Test box_thinnest_axis{0x80e090,
                           [=](Rng &r) { for (int i = 0; i < 3; i++) arena<float>(0x100)[i] = (float)r.range(-5, 5), arena<float>(0x110)[i] = r.range(0, 9) ? (float)r.range(-2, 8) : geo_pick(r); },
                           [](void *fn) { return Result{((uint32_t(__fastcall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x110))}; }};
    quick(box_thinnest_axis);
    // Vector::Compare(a, b): vectors equal in some parts
    Test vector_compare{0xbfb2b0,
                        [=](Rng &r) {
                            for (int i = 0; i < 3; i++) arena<float>(0x100)[i] = r.range(0, 9) ? (float)r.range(-1, 1) : geo_pick(r), arena<float>(0x110)[i] = r.range(0, 9) ? (float)r.range(-1, 1) : geo_pick(r);
                            if (r.range(0, 4) == 0) arena<uint32_t>(0x100 + 0x10 * r.range(0, 1))[r.range(0, 2)] = 0x7fc00000u;   // NaNs count as not equal
                        },
                        [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x110))}; }};
    quick(vector_compare);
    // Window::FirstFocusable(this): two lists of 0-3 nodes, children null or flagged
    Test window_first_focusable{0x6adc50,
                                [=](Rng &r) {
                                    for (int l = 0; l < 2; l++) {
                                        const int n = r.range(0, 3);
                                        arena<uint32_t>(l ? 0x14c : 0x164)[0] = (uint32_t)(uintptr_t)arena<void>(0x400 + 0x100 * l);
                                        for (int i = 0; i <= n; i++) {
                                            uint32_t *node = arena<uint32_t>(0x400 + 0x100 * l + 0x10 * i);
                                            node[0] = i < n ? (uint32_t)(uintptr_t)arena<void>(0x400 + 0x100 * l + 0x10 * (i + 1)) : 0;
                                            node[2] = r.range(0, 4) ? (uint32_t)(uintptr_t)arena<void>(0x1000 + 0x100 * (4 * l + i)) : 0;
                                            arena<uint32_t>(0x1094 + 0x100 * (4 * l + i))[0] = r.range(0, 3) ? r.next() & ~4u : r.next();
                                        }
                                    }
                                },
                                [](void *fn) { return Result{((uint32_t(__thiscall *)(void *))fn)(arena<void>(0x100))}; }};
    quick(window_first_focusable);
    // SortedPointers::FindInt(this, key, place): 0-8 objects sorted by a signed value
    Test sorted_pointers_find_int{0x8fd720,
                                  [=](Rng &r) {
                                      const int32_t n = r.range(0, 8);
                                      arena<uint32_t>(0x100)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<int32_t>(0x104)[0] = n;
                                      int32_t v = r.range(-5, 5);
                                      for (int i = 0; i < n; i++) arena<uint32_t>(0x400)[i] = (uint32_t)(uintptr_t)arena<void>(0x800 + 16 * i), arena<int32_t>(0x800 + 16 * i)[0] = v, v += r.range(1, 3);
                                      const int pick = r.range(0, n);
                                      arena<int32_t>(0x10)[0] = pick < n ? arena<int32_t>(0x800 + 16 * pick)[0] : r.range(-8, 30);
                                      arena<uint32_t>(0x14)[0] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x20) : 0, arena<uint32_t>(0x20)[0] = r.next();
                                  },
                                  [](void *fn) {
                                      return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, int32_t, void *))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], (void *)(uintptr_t)arena<uint32_t>(0x14)[0])};
                                  }};
    quick(sorted_pointers_find_int);
    // Description::Compare(this, other): descriptions differing in one place (or none)
    Test description_compare{0x713110,
                             [=](Rng &r) {
                                 uint32_t *a = arena<uint32_t>(0x100), *b = arena<uint32_t>(0x200);
                                 for (int i = 0; i < 12; i++) a[i] = r.next();
                                 a[5] = (uint32_t)(uintptr_t)arena<void>(0x300), arena<uint32_t>(0x300)[0] = r.next(), arena<uint32_t>(0x300)[1] = r.next();
                                 for (int i = 0; i < 12; i++) b[i] = a[i];
                                 b[5] = (uint32_t)(uintptr_t)arena<void>(0x320), arena<uint32_t>(0x320)[0] = arena<uint32_t>(0x300)[0], arena<uint32_t>(0x320)[1] = arena<uint32_t>(0x300)[1];
                                 const int which = r.range(0, 12);
                                 if (which == 0) ((uint8_t *)b)[0] ^= 1;
                                 else if (which == 1 || which == 2) ((uint8_t *)b)[1] ^= 1;
                                 else if (which < 6) b[which - 2] ^= 1;
                                 else if (which < 8) arena<uint32_t>(0x320)[which - 6] ^= 1;
                                 else if (which < 11) b[which + 1] ^= 1;
                             },
                             [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200))}; }};
    quick(description_compare);
    // Entries::FindByKey(this, mask, a, b, c, d): 0-5 entries close to the key
    Test entries_find_by_key{0x4cc880,
                             [=](Rng &r) {
                                 const int n = r.range(0, 5);
                                 arena<int32_t>(0x19c)[0] = n, arena<uint32_t>(0x194)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                                 for (int i = 0; i < 12; i++) {   // (inactive entries do not count toward the number)
                                     uint32_t *e = arena<uint32_t>(0x400 + 0x2c * i);
                                     ((uint8_t *)e)[0] = r.range(0, 3) || i >= 7 ? 1 : 0, e[1] = r.range(0, 3), e[2] = r.range(0, 1), e[3] = r.range(0, 1), e[4] = r.range(0, 1), e[5] = r.range(0, 1);
                                     e[7] = (uint32_t)(uintptr_t)arena<void>(0x1000 + 0x20 * i), arena<uint32_t>(0x100c + 0x20 * i)[0] = r.next();
                                 }
                                 arena<uint32_t>(0x10)[0] = r.range(0, 3);
                                 for (int i = 1; i < 5; i++) arena<uint32_t>(0x10)[i] = r.range(0, 1);
                             },
                             [](void *fn) {
                                 const uint32_t *a = arena<uint32_t>(0x10);
                                 return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), a[0], a[1], a[2], a[3], a[4])};
                             }};
    quick(entries_find_by_key);
    // SortedIndex::LowerBound(this, entry): 0-8 entries sorted high to low through an index, equal values too
    Test sorted_index_lower_bound{0x80e3a0,
                                  [=](Rng &r) {
                                      const int32_t n = r.range(0, 8);
                                      arena<int32_t>(0x148)[0] = n, arena<uint32_t>(0x144)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<uint32_t>(0x140)[0] = (uint32_t)(uintptr_t)arena<void>(0x800);
                                      float v = (float)r.range(5, 15);
                                      for (int i = 0; i < n; i++) arena<int32_t>(0x400)[i] = 7 - i, arena<float>(0x808 + 0x1c * (7 - i))[0] = v, v -= (float)r.range(0, 2);
                                      arena<float>(0x28)[0] = r.range(0, 9) ? (float)r.range(-2, 16) : geo_pick(r);
                                  },
                                  [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x20))}; }};
    quick(sorted_index_lower_bound);
    // Terrain::CellHeightRange(this, x, y, low, high): a 4x4 grid, signalling NaNs now and then
    Test terrain_cell_height_range{0x8bcf80,
                                   [=](Rng &r) {
                                       arena<uint8_t>(0x104)[0] = 4, arena<uint32_t>(0x124)[0] = (uint32_t)(uintptr_t)arena<void>(0x1000);
                                       for (int i = 0; i < 16 * 6; i++) arena<float>(0x1000)[i] = r.range(0, 19) ? r.real(-10, 10) : geo_pick(r);
                                       if (r.range(0, 9) == 0) arena<uint32_t>(0x1000)[r.range(0, 95)] = 0x7f800001u + r.range(0, 99);
                                       arena<uint32_t>(0x10)[0] = r.range(0, 3) | (r.next() & 0xffffff00u), arena<uint32_t>(0x10)[1] = r.range(0, 3) | (r.next() & 0xffffff00u);
                                       arena<uint32_t>(0x20)[0] = r.next(), arena<uint32_t>(0x24)[0] = r.next();
                                   },
                                   [](void *fn) {
                                       ((void(__thiscall *)(void *, uint32_t, uint32_t, void *, void *))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], arena<uint32_t>(0x10)[1], arena<void>(0x20), arena<void>(0x24));
                                       return Result{};
                                   }};
    quick(terrain_cell_height_range);
    // SinTable::Sin(table, angle): signalling NaNs in the table now and then
    Test sin_table_sin{0xcd0660,
                       [=](Rng &r) {
                           for (int i = 0; i < 256; i++) arena<uint32_t>(0x1000)[i] = r.range(0, 30) ? r.next() : 0x7f800001u + r.range(0, 99);
                           arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-30, 30) : geo_pick(r);
                       },
                       [](void *fn) {
                           const float v = (float)((long double(__thiscall *)(void *, float))fn)(arena<void>(0x1000), arena<float>(0x10)[0]);
                           uint32_t bits;
                           memcpy(&bits, &v, 4);
                           return Result{bits};
                       }};
    quick(sin_table_sin);
    // Math::IntSqrt(n): 0 to past 2^30
    Test math_int_sqrt{0xdf88f0,
                       [=](Rng &r) { arena<int32_t>(0x10)[0] = r.range(0, 2) ? r.range(0, 1000) : r.range(0, 9) ? (int32_t)(r.next() >> (1 + r.range(0, 20))) : 0x40000000 + r.range(-2, 2); },
                       [](void *fn) { return Result{((uint32_t(__cdecl *)(int32_t))fn)(arena<int32_t>(0x10)[0])}; }};
    quick(math_int_sqrt);
    // ListNode::IsLinked(node): linked, half-linked, null neighbours, null node
    Test list_node_is_linked{0xcc21f0,
                             [=](Rng &r) {
                                 uint32_t *n = arena<uint32_t>(0x200), *a = arena<uint32_t>(0x220), *b = arena<uint32_t>(0x240);
                                 n[0] = r.range(0, 5) ? (uint32_t)(uintptr_t)a : 0, n[1] = r.range(0, 5) ? (uint32_t)(uintptr_t)b : 0;
                                 a[1] = r.range(0, 3) ? (uint32_t)(uintptr_t)n : 0, b[0] = r.range(0, 3) ? (uint32_t)(uintptr_t)n : 0;
                                 arena<uint32_t>(0x10)[0] = r.range(0, 9) ? (uint32_t)(uintptr_t)n : 0;
                             },
                             [](void *fn) { return Result{(uint32_t)((uint8_t(__thiscall *)(void *))fn)((void *)(uintptr_t)arena<uint32_t>(0x10)[0])}; }};
    quick(list_node_is_linked);
    // Rect::AddCell(this, x, y)
    Test rect_add_cell{0x71bfe0,
                       [=](Rng &r) {
                           for (int i = 0; i < 4; i++) arena<uint8_t>(0x192)[i] = (uint8_t)r.range(0, 9);
                           arena<uint32_t>(0x10)[0] = r.range(0, 9) | (r.next() & 0xffffff00u), arena<uint32_t>(0x10)[1] = r.range(0, 9) | (r.next() & 0xffffff00u);
                       },
                       [](void *fn) { ((void(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], arena<uint32_t>(0x10)[1]); return Result{}; }};
    quick(rect_add_cell);
    // Rect::GrowByOne(this, x0, y0, x1, y1): corners at the grid's edges and inside
    Test rect_grow_by_one{0x8bcff0,
                          [=](Rng &r) {
                              arena<uint8_t>(0x104)[0] = (uint8_t)r.range(0, 9), arena<uint8_t>(0x105)[0] = (uint8_t)r.range(0, 9);
                              for (int i = 0; i < 4; i++) arena<uint8_t>(0x20 + 4 * i)[0] = (uint8_t)r.range(0, 10);
                          },
                          [](void *fn) {
                              ((void(__thiscall *)(void *, void *, void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x20), arena<void>(0x24), arena<void>(0x28), arena<void>(0x2c));
                              return Result{};
                          }};
    quick(rect_grow_by_one);
    // PointerArray::RemoveSwap(this, value) and ParticleBaseDesc::RemoveLink(this, value): 0-5 values with repeats
    for (const uint32_t at : {0x80ee80u, 0x9ceb20u}) {
        Test array_remove{at,
                          [=](Rng &r) {
                              const bool swap = at == 0x80ee80u;
                              arena<uint32_t>(swap ? 0x18c : 0x170)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<int32_t>(swap ? 0x194 : 0x178)[0] = r.range(0, 5);
                              for (int i = 0; i < 8; i++) arena<uint32_t>(0x400)[i] = r.range(0, 4);
                              arena<uint32_t>(0x10)[0] = r.range(0, 4);
                          },
                          [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0]); return Result{}; }};
        quick(array_remove);
    }
    // Queue::HasId(this, id): a queue of six slots, a range within it
    Test queue_has_id{0x9465e0,
                      [=](Rng &r) {
                          arena<uint32_t>(0x168)[0] = r.range(0, 2), arena<uint32_t>(0x174)[0] = r.range(0, 2);
                          arena<uint32_t>(0x164)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<uint32_t>(0x17c)[0] = r.range(0, 4), arena<uint32_t>(0x180)[0] = r.range(0, 6);
                          for (int i = 0; i < 6; i++) {
                              arena<uint32_t>(0x400)[i] = r.range(0, 4) ? (uint32_t)(uintptr_t)arena<void>(0x800 + 0x40 * i) : 0;
                              arena<uint32_t>(0x83c + 0x40 * i)[0] = r.range(0, 2), arena<uint32_t>(0x838 + 0x40 * i)[0] = r.range(0, 2) ? r.next() : 0;
                          }
                          arena<uint32_t>(0x10)[0] = r.range(0, 2);
                      },
                      [](void *fn) { return Result{(uint32_t)((uint8_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0])}; }};
    quick(queue_has_id);
    // WideString::Compare(this, other): wide strings of 0-4 a/b characters
    Test wide_string_compare{0xbce970,
                             [=](Rng &r) {
                                 for (int s = 0; s < 2; s++) {
                                     arena<uint32_t>(0x100 + 4 * s)[0] = (uint32_t)(uintptr_t)arena<void>(0x200 + 0x40 * s);
                                     arena<uint32_t>(0x204 + 0x40 * s)[0] = r.range(0, 4), arena<uint32_t>(0x20c + 0x40 * s)[0] = (uint32_t)(uintptr_t)arena<void>(0x400 + 0x20 * s);
                                     for (int i = 0; i < 4; i++) arena<uint16_t>(0x400 + 0x20 * s)[i] = r.range(0, 5) ? "ab"[r.range(0, 1)] : (uint16_t)r.next();
                                 }
                             },
                             [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x104))}; }};
    quick(wide_string_compare);
    // Matrix::Ortho(this, a, b, w, h)
    Test matrix_ortho{0x4664e0,
                      [=](Rng &r) {
                          for (int i = 0; i < 16; i++) arena<uint32_t>(0x100)[i] = r.next();
                          for (int i = 0; i < 4; i++) arena<float>(0x10)[i] = r.range(0, 9) ? r.real(-100, 1000) : geo_pick(r);
                      },
                      [](void *fn) {
                          const float *a = arena<float>(0x10);
                          ((void(__thiscall *)(void *, float, float, float, float))fn)(arena<void>(0x100), a[0], a[1], a[2], a[3]);
                          return Result{};
                      }};
    quick(matrix_ortho);
    // Vertices::SetSecond(this): 0-11 values (the four-at-a-time part and the rest)
    Test vertices_set_second{0x8ef180,
                             [=](Rng &r) {
                                 arena<int32_t>(0x108)[0] = r.range(-1, 3), arena<int32_t>(0x10c)[0] = r.range(0, 3);
                                 arena<uint32_t>(0x110)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<uint32_t>(0x114)[0] = (uint32_t)(uintptr_t)arena<void>(0x800);
                                 for (int i = 0; i < 16; i++) arena<uint32_t>(0x400)[i] = r.next();
                                 for (int i = 0; i < 64; i++) arena<uint32_t>(0x800)[i] = r.next();
                             },
                             [](void *fn) { ((void(__thiscall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(vertices_set_second);
    // Pieces::PairAllowed(a, b): kinds 0, 0x4A, 0x4C and others, types 0-6
    Test pieces_pair_allowed{0xbc5eb0,
                             [=](Rng &r) {
                                 static const int32_t kinds[] = {0, 0, 0x4a, 0x4c, 0x4b, 7};
                                 for (int s = 0; s < 2; s++) arena<int32_t>(0x110 + 0x20 * s)[0] = kinds[r.range(0, 5)], arena<int32_t>(0x108 + 0x20 * s)[0] = r.range(0, 6);
                             },
                             [](void *fn) { return Result{((uint32_t(__fastcall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x120))}; }};
    quick(pieces_pair_allowed);
    // TerrainLayers::SetFlag(this, list, flag, value): 0-4 layers
    Test terrain_layers_set_flag{0xafa080,
                                 [=](Rng &r) {
                                     arena<uint32_t>(0x108)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<uint32_t>(0x114)[0] = (uint32_t)(uintptr_t)arena<void>(0x800);
                                     for (int i = 0; i < 8; i++) arena<int32_t>(0x400)[i] = r.range(0, 5);
                                     for (int i = 0; i < 6 * 11; i++) arena<uint32_t>(0x800)[i] = r.next();
                                     arena<int32_t>(0x20)[0] = r.range(0, 3), arena<int32_t>(0x24)[0] = r.range(0, 4);
                                     arena<uint32_t>(0x10)[0] = r.next() & 0x1ff, arena<uint32_t>(0x14)[0] = r.next(), arena<uint8_t>(0x134)[0] = (uint8_t)r.next();
                                 },
                                 [](void *fn) {
                                     ((void(__thiscall *)(void *, void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<void>(0x20), arena<uint32_t>(0x10)[0], arena<uint32_t>(0x14)[0]);
                                     return Result{};
                                 }};
    quick(terrain_layers_set_flag);
    // Area::Move(this, dx, dy): 0-5 items
    Test area_move{0x7339b0,
                   [=](Rng &r) {
                       for (int i = 0; i < 0x10; i++) arena<int32_t>(0x140)[i] = r.range(-100, 100);
                       arena<uint32_t>(0x164)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<int32_t>(0x168)[0] = r.range(0, 5);
                       for (int i = 0; i < 10; i++) arena<int32_t>(0x400)[i] = r.range(-100, 100);
                       arena<int32_t>(0x10)[0] = r.range(-20, 20), arena<int32_t>(0x14)[0] = r.range(-20, 20);
                   },
                   [](void *fn) { ((void(__thiscall *)(void *, int32_t, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<int32_t>(0x14)[0]); return Result{}; }};
    quick(area_move);
    // FreeList::Release(this, head, n): a chain of up to six entries, n in it or not
    Test free_list_release{0x8bfef0,
                           [=](Rng &r) {
                               const uint32_t count = r.range(1, 6);
                               arena<uint32_t>(0x100)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<uint32_t>(0x104)[0] = count, arena<uint32_t>(0x108)[0] = r.range(0, 9);
                               for (uint32_t i = 0; i < 8; i++) arena<uint32_t>(0x404 + 16 * i)[0] = r.range(0, 2) ? (i + 1 + r.range(0, 1)) : 0xffffffffu;
                               arena<uint32_t>(0x20)[0] = r.range(0, 3) ? r.range(0, count - 1) : count + 1;
                               arena<uint32_t>(0x10)[0] = r.range(0, count - 1);
                           },
                           [](void *fn) { ((void(__thiscall *)(void *, void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x20), arena<uint32_t>(0x10)[0]); return Result{}; }};
    quick(free_list_release);
    // Timers::Tick55(this): a list of 1-3 objects with flagged timers about to run out
    Test timers_tick55{0x7a9150,
                       [=](Rng &r) {
                           const int n = r.range(1, 3);
                           arena<uint32_t>(0x60c)[0] = (uint32_t)(uintptr_t)arena<void>(0x800);
                           for (int i = 0; i <= n; i++) arena<uint32_t>(0x800 + 0x10 * i)[0] = i < n ? (uint32_t)(uintptr_t)arena<void>(0x800 + 0x10 * (i + 1)) : 0;
                           for (int i = 0; i < n; i++) {
                               arena<uint32_t>(0x808 + 0x10 * i)[0] = (uint32_t)(uintptr_t)arena<void>(0x1000 + 0x200 * i);
                               arena<uint32_t>(0x1014 + 0x200 * i)[0] = r.next();
                               for (int c = 0; c < 0x37; c++) arena<int32_t>(0x1018 + 0x200 * i)[c] = r.range(-2, 2);
                           }
                       },
                       [](void *fn) { ((void(__thiscall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(timers_tick55);
    // ProgramInfo::MinSize(this, out): sizes either side of the minimum, signalling NaNs now and then
    Test program_info_min_size{0x621370,
                               [=](Rng &r) {
                                   arena<float>(0x1f8)[0] = r.range(0, 9) ? r.real(300, 800) : geo_pick(r), arena<float>(0x1fc)[0] = r.range(0, 9) ? r.real(100, 300) : geo_pick(r);
                                   if (r.range(0, 9) == 0) arena<uint32_t>(0x1f8)[r.range(0, 1)] = 0x7f800001u + r.range(0, 99);
                                   arena<uint32_t>(0x20)[0] = r.next(), arena<uint32_t>(0x24)[0] = r.next();
                               },
                               [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x20)); return Result{}; }};
    quick(program_info_min_size);
    // Damping::For(speed): the game's scale set for the call and put back
    static uint32_t damping_scale_saved;
    Test damping_for{0x5bada0,
                     [=](Rng &r) { arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-5, 30) : geo_pick(r), arena<float>(0x14)[0] = r.range(0, 9) ? r.real(0, 2) : geo_pick(r); },
                     [](void *fn) {
                         damping_scale_saved = *(volatile uint32_t *)0x12f55cc;
                         *(volatile float *)0x12f55cc = arena<float>(0x14)[0];
                         const float v = (float)((long double(__stdcall *)(float))fn)(arena<float>(0x10)[0]);
                         *(volatile uint32_t *)0x12f55cc = damping_scale_saved;
                         uint32_t bits;
                         memcpy(&bits, &v, 4);
                         return Result{bits};
                     }};
    quick(damping_for);
    // Counts::AddByLevel(this, v): values round the four levels
    Test counts_add_by_level{0x5bc050,
                             [=](Rng &r) {
                                 for (int i = 0; i < 4; i++) arena<int32_t>(0x100)[i] = r.range(0, 9);
                                 static const uint32_t at[] = {0xf3ef7c, 0xf3ef78, 0xf3ef74, 0xf3ef70};
                                 const float level = *(const float *)(uintptr_t)at[r.range(0, 3)];
                                 arena<float>(0x10)[0] = r.range(0, 9) ? (r.range(0, 2) ? level * r.real(0.8f, 1.2f) : level) : geo_pick(r);
                             },
                             [](void *fn) { ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x10)[0]); return Result{}; }};
    quick(counts_add_by_level);
    // Grid::ContainsId(this, id, x0, y0, x1, y1): a 6x5 grid of ids 0-7, rectangles inside it (empty ones too)
    Test grid_contains_id{0x9375c0,
                          [=](Rng &r) {
                              arena<int32_t>(0x130)[0] = 6, arena<uint32_t>(0x138)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                              for (int i = 0; i < 30; i++) arena<uint16_t>(0x400)[i] = (uint16_t)r.range(0, 7);
                              arena<uint32_t>(0x10)[0] = r.range(0, 8) | (r.next() & 0xffff0000u);
                              arena<int32_t>(0x14)[0] = r.range(0, 5), arena<int32_t>(0x18)[0] = r.range(0, 4), arena<int32_t>(0x1c)[0] = r.range(0, 5), arena<int32_t>(0x20)[0] = r.range(0, 4);
                          },
                          [](void *fn) {
                              const int32_t *a = arena<int32_t>(0x14);
                              return Result{(uint32_t)((uint8_t(__thiscall *)(void *, uint32_t, int32_t, int32_t, int32_t, int32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], a[0], a[1], a[2], a[3])};
                          }};
    quick(grid_contains_id);
    // Bytes3List::Equal(this, list, n): lists matching, differing in a byte, or of another count
    Test bytes3_list_equal{0xc44360,
                           [=](Rng &r) {
                               const uint32_t n = r.range(0, 4);
                               arena<uint32_t>(0x100)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<uint32_t>(0x104)[0] = r.range(0, 4) ? 0 : 1, arena<uint32_t>(0x108)[0] = r.range(0, 4) ? n : n + 1;
                               for (int i = 0; i < 16; i++) arena<uint8_t>(0x400)[i] = (uint8_t)r.next(), arena<uint8_t>(0x500)[i] = arena<uint8_t>(0x400)[i];
                               if (r.range(0, 1)) arena<uint8_t>(0x500)[r.range(0, 15)] ^= 1;
                               arena<uint32_t>(0x10)[0] = n;
                           },
                           [](void *fn) { return Result{(uint32_t)((uint8_t(__thiscall *)(void *, void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x500), arena<uint32_t>(0x10)[0])}; }};
    quick(bytes3_list_equal);
    // Bytes3::Equal(this, other)
    Test bytes3_equal{0xcc4a30,
                      [=](Rng &r) { for (int i = 0; i < 4; i++) arena<uint8_t>(0x100)[i] = (uint8_t)r.next(), arena<uint8_t>(0x200)[i] = r.range(0, 2) ? arena<uint8_t>(0x100)[i] : (uint8_t)r.next(); },
                      [](void *fn) { return Result{(uint32_t)((uint8_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200))}; }};
    quick(bytes3_equal);
    // Objects::HasWithBit(this, object, bit): a list of 1-3 objects with ids 0-2, bits 0-40
    Test objects_has_with_bit{0x7a8f30,
                              [=](Rng &r) {
                                  const int n = r.range(1, 3);
                                  arena<uint32_t>(0x60c)[0] = (uint32_t)(uintptr_t)arena<void>(0x800);
                                  for (int i = 0; i <= n; i++) arena<uint32_t>(0x800 + 0x10 * i)[0] = i < n ? (uint32_t)(uintptr_t)arena<void>(0x800 + 0x10 * (i + 1)) : 0;
                                  for (int i = 0; i < n; i++) arena<uint32_t>(0x808 + 0x10 * i)[0] = (uint32_t)(uintptr_t)arena<void>(0x1000 + 0x40 * i), arena<uint32_t>(0x1004 + 0x40 * i)[0] = r.range(0, 2), arena<uint32_t>(0x1014 + 0x40 * i)[0] = r.next();
                                  arena<uint32_t>(0x204)[0] = r.range(0, 2), arena<uint32_t>(0x10)[0] = r.range(0, 2) ? 0 : r.range(0, 40);
                              },
                              [](void *fn) { return Result{(uint32_t)((uint8_t(__thiscall *)(void *, void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x200), arena<uint32_t>(0x10)[0])}; }};
    quick(objects_has_with_bit);
    // Cars::TotalLoad(this): 0-5 cars, some without the parts
    Test cars_total_load{0x9892a0,
                         [=](Rng &r) {
                             arena<int32_t>(0x208)[0] = r.range(0, 5), arena<uint32_t>(0x204)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                             for (int i = 0; i < 5; i++) {
                                 arena<uint32_t>(0x448 + 0x58 * i)[0] = r.range(0, 4) ? (uint32_t)(uintptr_t)arena<void>(0x1000 + 0x200 * i) : 0;
                                 arena<uint32_t>(0x10cc + 0x200 * i)[0] = r.range(0, 4) ? (uint32_t)(uintptr_t)arena<void>(0x3000 + 0x100 * i) : 0;
                                 arena<int32_t>(0x30c4 + 0x100 * i)[0] = r.range(0, 3) ? r.range(-100, 1000) : (int32_t)r.next();
                             }
                         },
                         [](void *fn) { return Result{((uint32_t(__thiscall *)(void *))fn)(arena<void>(0x100))}; }};
    quick(cars_total_load);
    // String::Compare(this, other): strings of 0-4 characters (high ones too)
    Test string_compare{0xbcf040,
                        [=](Rng &r) {
                            for (int s = 0; s < 2; s++) {
                                arena<uint32_t>(0x100 + 4 * s)[0] = (uint32_t)(uintptr_t)arena<void>(0x200 + 0x40 * s);
                                arena<uint32_t>(0x204 + 0x40 * s)[0] = r.range(0, 4), arena<uint32_t>(0x20c + 0x40 * s)[0] = (uint32_t)(uintptr_t)arena<void>(0x400 + 0x20 * s);
                                for (int i = 0; i < 4; i++) arena<uint8_t>(0x400 + 0x20 * s)[i] = r.range(0, 5) ? "ab"[r.range(0, 1)] : (uint8_t)r.next();
                            }
                        },
                        [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x104))}; }};
    quick(string_compare);
    // Window::AllowedMoves(this, moves)
    Test window_allowed_moves{0xc5f510,
                              [=](Rng &r) { arena<uint32_t>(0x11c)[0] = (r.next() & ~0x43u) | (r.range(0, 3) ? 3 : r.range(0, 3)) | (r.range(0, 3) ? 0 : 0x40); arena<uint32_t>(0x10)[0] = r.next(); },
                              [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0])}; }};
    quick(window_allowed_moves);
    // SortedIndex::LowerBound2, SortedTable::LowerBound1C, SortedPointers::LowerBound8: 0-8 values sorted low to high
    for (const uint32_t at : {0x80e200u, 0x80e340u, 0x80e1a0u}) {
        Test float_lower_bound{at,
                               [=](Rng &r) {
                                   const int32_t n = r.range(0, 8);
                                   float v = (float)r.range(-5, 5);
                                   for (int i = 0; i < 8; i++) {
                                       arena<uint32_t>(0x400)[i] = (uint32_t)(uintptr_t)arena<void>(0x800 + 0x20 * i);   // objects
                                       arena<int32_t>(0x500)[i] = i;                                                       // the index
                                       arena<float>(0x808 + 0x20 * i)[0] = v, arena<float>(0xc00 + 0x1c * i)[0] = v;
                                       v += (float)r.range(0, 2);
                                   }
                                   if (at == 0x80e200u) arena<uint32_t>(0x10c)[0] = (uint32_t)(uintptr_t)arena<void>(0x500), arena<uint32_t>(0x108)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<int32_t>(0x110)[0] = n;
                                   else if (at == 0x80e340u) arena<uint32_t>(0x140)[0] = (uint32_t)(uintptr_t)arena<void>(0xc00), arena<int32_t>(0x148)[0] = n;
                                   else arena<uint32_t>(0x108)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<int32_t>(0x110)[0] = n;
                                   arena<float>(0x10)[0] = r.range(0, 9) ? (float)r.range(-7, 20) : geo_pick(r);
                               },
                               [at](void *fn) {
                                   if (at == 0x80e340u) return Result{((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x10))};
                                   return Result{((uint32_t(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x10)[0])};
                               }};
        quick(float_lower_bound);
    }
    // Items::SizeAt(this, n): a list of 0-4 nodes, items null or not, either flag
    Test items_size_at{0xc637c0,
                       [=](Rng &r) {
                           const int n = r.range(0, 4);
                           arena<uint32_t>(0x1b0)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<uint32_t>(0x11c)[0] = r.next();
                           for (int i = 0; i <= n; i++) arena<uint32_t>(0x400 + 0x10 * i)[0] = i < n ? (uint32_t)(uintptr_t)arena<void>(0x400 + 0x10 * (i + 1)) : 0;
                           for (int i = 0; i < n; i++) {
                               arena<uint32_t>(0x408 + 0x10 * i)[0] = r.range(0, 5) ? (uint32_t)(uintptr_t)arena<void>(0x1000 + 0x100 * i) : 0;
                               arena<float>(0x10d0 + 0x100 * i)[0] = r.range(0, 9) ? r.real(0, 500) : geo_pick(r), arena<float>(0x10d4 + 0x100 * i)[0] = r.real(0, 500);
                           }
                           arena<int32_t>(0x10)[0] = r.range(0, 5);
                       },
                       [](void *fn) {
                           const float v = (float)((long double(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0]);
                           uint32_t bits;
                           memcpy(&bits, &v, 4);
                           return Result{bits};
                       }};
    quick(items_size_at);
    // SortedPointers::FindFirst and FindFirst2 (this, key, place): 0-8 objects sorted by their first value
    for (const uint32_t at : {0x9734a0u, 0x84f1a0u}) {
        Test find_first{at,
                        [=](Rng &r) {
                            const int32_t n = r.range(0, 8);
                            const bool first = at == 0x9734a0u;
                            arena<uint32_t>(first ? 0x10c : 0x100)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<int32_t>(first ? 0x110 : 0x104)[0] = n;
                            uint32_t v = r.range(0, 1) ? (uint32_t)r.range(-5, 5) : 0x7ffffffeu;
                            for (int i = 0; i < n; i++) arena<uint32_t>(0x400)[i] = (uint32_t)(uintptr_t)arena<void>(0x800 + 16 * i), arena<uint32_t>(0x800 + 16 * i)[0] = v, v += r.range(1, 3);
                            const int pick = r.range(0, n);
                            arena<uint32_t>(0x10)[0] = pick < n ? arena<uint32_t>(0x800 + 16 * pick)[0] : v - r.range(0, 30), arena<uint32_t>(0x20)[0] = r.next();
                        },
                        [](void *fn) { return Result{(uint32_t)((uint8_t(__thiscall *)(void *, uint32_t, void *))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], arena<void>(0x20))}; }};
        quick(find_first);
    }
    // Keys::FindTime(this, t): 0-8 keys with rising times
    Test keys_find_time{0x988f60,
                        [=](Rng &r) {
                            const int32_t n = r.range(0, 8);
                            arena<int32_t>(0x14c)[0] = n, arena<uint32_t>(0x140)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                            float t = r.real(-5, 5);
                            for (int i = 0; i < 8; i++) arena<float>(0x400 + 32 * i)[0] = t, t += r.range(0, 5) ? r.real(0, 3) : 0;
                            arena<float>(0x10)[0] = r.range(0, 2) == 0 ? arena<float>(0x400 + 32 * r.range(0, 7))[0] : r.range(0, 9) ? r.real(-8, 25) : geo_pick(r);
                        },
                        [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x10)[0])}; }};
    quick(keys_find_time);
    // Attraction::SpeedScale(this): kinds round 0x2E-0x31 and 0x44, the game's scale set for the call and put back
    static uint32_t speed_scale_saved;
    Test attraction_speed_scale{0x55fbb0,
                                [=](Rng &r) {
                                    static const uint32_t kinds[] = {0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x44, 0x45};
                                    arena<uint32_t>(0x160)[0] = kinds[r.range(0, 7)], arena<float>(0x284)[0] = r.range(0, 9) ? r.real(0, 3) : geo_pick(r);
                                    arena<float>(0x10)[0] = r.range(0, 9) ? r.real(0, 1) : geo_pick(r);
                                },
                                [](void *fn) {
                                    speed_scale_saved = *(volatile uint32_t *)0x10ee8f4;
                                    *(volatile float *)0x10ee8f4 = arena<float>(0x10)[0];
                                    const float v = (float)((long double(__thiscall *)(void *))fn)(arena<void>(0x100));
                                    *(volatile uint32_t *)0x10ee8f4 = speed_scale_saved;
                                    uint32_t bits;
                                    memcpy(&bits, &v, 4);
                                    return Result{bits};
                                }};
    quick(attraction_speed_scale);
    // Firework::SetFade(this, level): levels in and out of 0-1, signalling NaNs now and then
    Test firework_set_fade{0x9f7110,
                           [=](Rng &r) {
                               arena<float>(0x10)[0] = r.range(0, 9) ? r.real(-0.5f, 1.5f) : geo_pick(r);
                               if (r.range(0, 19) == 0) arena<uint32_t>(0x10)[0] = 0x7f800001u + r.range(0, 99);
                               arena<uint32_t>(0x10c)[0] = r.next();
                           },
                           [](void *fn) { ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x10)[0]); return Result{}; }};
    quick(firework_set_fade);
    // RefList::Find(this, a, b): a circular list of 0-4 nodes, entries matching or not
    Test ref_list_find{0xbbcce0,
                       [=](Rng &r) {
                           const int n = r.range(0, 4);
                           for (int i = 0; i < n; i++) {
                               arena<uint32_t>(0x400 + 0x10 * i)[0] = i + 1 < n ? (uint32_t)(uintptr_t)arena<void>(0x400 + 0x10 * (i + 1)) : (uint32_t)(uintptr_t)arena<void>(0x10c);
                               arena<uint32_t>(0x408 + 0x10 * i)[0] = (uint32_t)(uintptr_t)arena<void>(0x800 + 0x20 * i);
                               arena<uint32_t>(0x800 + 0x20 * i)[0] = r.range(0, 9), arena<uint32_t>(0x804 + 0x20 * i)[0] = r.range(0, 2), arena<uint32_t>(0x808 + 0x20 * i)[0] = r.range(0, 2), arena<uint32_t>(0x80c + 0x20 * i)[0] = r.next();
                           }
                           arena<uint32_t>(0x100)[0] = n ? (uint32_t)(uintptr_t)arena<void>(0x400) : (uint32_t)(uintptr_t)arena<void>(0x10c);
                           arena<uint32_t>(0x10c)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<uint32_t>(0x114)[0] = (uint32_t)(uintptr_t)arena<void>(0x900);
                           arena<uint32_t>(0x10)[0] = r.range(0, 2), arena<uint32_t>(0x14)[0] = r.range(0, 2);
                       },
                       [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], arena<uint32_t>(0x14)[0])}; }};
    quick(ref_list_find);
    // Drag::Offset(this, out): dragging or not
    Test drag_offset{0x682910,
                     [=](Rng &r) { arena<uint8_t>(0x358)[0] = r.range(0, 1) ? 0 : (uint8_t)r.range(1, 255), arena<uint32_t>(0x35c)[0] = r.next(), arena<uint32_t>(0x360)[0] = r.next(); arena<uint32_t>(0x20)[0] = r.next(), arena<uint32_t>(0x24)[0] = r.next(); },
                     [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x20)); return Result{}; }};
    quick(drag_offset);
    // KeyCombination::Set(this, k1, k2, k3, k4): zero keys now and then
    Test key_combination_set{0xbd1940,
                             [=](Rng &r) { for (int i = 0; i < 5; i++) arena<uint32_t>(0x100)[i] = r.next(); for (int i = 0; i < 4; i++) arena<uint32_t>(0x10)[i] = r.range(0, 1) ? 0 : r.next(); },
                             [](void *fn) {
                                 const uint32_t *k = arena<uint32_t>(0x10);
                                 return Result{(uint32_t)(((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), k[0], k[1], k[2], k[3]) == arena<void>(0x100))};
                             }};
    quick(key_combination_set);
    // PhysicsWorld::RemoveBody(this, body): 0-5 bodies with repeats
    Test physics_world_remove_body{0xa6bf80,
                                   [=](Rng &r) {
                                       arena<uint32_t>(0x160)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<int32_t>(0x168)[0] = r.range(0, 5);
                                       for (int i = 0; i < 8; i++) arena<uint32_t>(0x400)[i] = r.range(0, 4);
                                       arena<uint32_t>(0x10)[0] = r.range(0, 4);
                                   },
                                   [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0]); return Result{}; }};
    quick(physics_world_remove_body);
    // Ride::Count585AA0(this, n)
    Test ride_count_585aa0{0x585aa0,
                           [=](Rng &r) {
                               for (const int at : {0x190, 0x174, 0x2f4, 0x2f8}) arena<int32_t>(at)[0] = r.range(0, 3) ? r.range(-1000, 1000) : (int32_t)r.next();
                               arena<int32_t>(0x10)[0] = r.range(0, 3) ? r.range(-100, 100) : (int32_t)r.next();
                           },
                           [](void *fn) { ((void(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0]); return Result{}; }};
    quick(ride_count_585aa0);
    // Values::SumFirst(this, n): 0-11 values
    Test values_sum_first{0x884680,
                          [=](Rng &r) {
                              arena<uint32_t>(0x10c)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                              for (int i = 0; i < 12; i++) arena<float>(0x400)[i] = r.range(0, 19) ? r.real(-1000, 1000) : geo_pick(r);
                              arena<int32_t>(0x10)[0] = r.range(-1, 11);
                          },
                          [](void *fn) {
                              const float v = (float)((long double(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0]);
                              uint32_t bits;
                              memcpy(&bits, &v, 4);
                              return Result{bits};
                          }};
    quick(values_sum_first);
    // Ride::InRange(this, other): values between the limits either way round, ends equal now and then
    Test ride_in_range{0x55f940,
                       [=](Rng &r) {
                           arena<uint8_t>(0x220)[0] = r.range(0, 3) ? 1 : 0, arena<uint8_t>(0x17c)[0] = (uint8_t)r.next();
                           arena<float>(0x218)[0] = (float)r.range(0, 9), arena<float>(0x170)[0] = r.range(0, 9) ? (float)r.range(0, 9) : geo_pick(r);
                           arena<float>(0x224)[0] = r.range(0, 9) ? (float)r.range(-1, 10) : geo_pick(r);
                       },
                       [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200)); return Result{}; }};
    quick(ride_in_range);
    // Cell::StepInside(this, x, y, direction): directions 0-5, cells at the edges (and wrapping bytes)
    Test cell_step_inside{0x71bf70,
                          [=](Rng &r) {
                              arena<uint8_t>(0x15c)[0] = (uint8_t)r.range(0, 9), arena<uint8_t>(0x15d)[0] = (uint8_t)r.range(0, 9);
                              arena<uint8_t>(0x20)[0] = (uint8_t)r.range(0, 10), arena<uint8_t>(0x24)[0] = (uint8_t)r.range(0, 10);
                              arena<uint32_t>(0x10)[0] = r.range(0, 5) | (r.next() & 0xffffff00u);
                          },
                          [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *, void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x20), arena<void>(0x24), arena<uint32_t>(0x10)[0])}; }};
    quick(cell_step_inside);
    // Tree::UpdateUp(node): a chain of 0-3 parents, first or second or third child
    Test tree_update_up{0xbe7a90,
                        [=](Rng &r) {
                            const int depth = r.range(0, 3);
                            for (int i = 0; i < 4; i++) for (int j = 0; j < 10; j++) arena<uint32_t>(0x400 + 0x40 * i)[j] = r.next();
                            for (int i = 0; i < 4; i++) {
                                uint32_t *n = arena<uint32_t>(0x400 + 0x40 * i);
                                n[1] = i < depth ? (uint32_t)(uintptr_t)arena<void>(0x400 + 0x40 * (i + 1)) : 0;
                                if (i < depth) {
                                    uint32_t *p = arena<uint32_t>(0x400 + 0x40 * (i + 1));
                                    const int k = r.range(0, 2);
                                    p[4] = k == 0 ? (uint32_t)(uintptr_t)n : 0, p[7] = k == 1 ? (uint32_t)(uintptr_t)n : 0;
                                }
                            }
                        },
                        [](void *fn) { ((void(__thiscall *)(void *))fn)(arena<void>(0x400)); return Result{}; }};
    quick(tree_update_up);
    // Pairs::InsertionSort(this): 0-7 pairs, values with repeats
    Test pairs_insertion_sort{0xcb2a00,
                              [=](Rng &r) {
                                  arena<uint32_t>(0x100)[0] = r.range(0, 7), arena<uint32_t>(0x10c)[0] = (uint32_t)(uintptr_t)arena<void>(0x400);
                                  for (int i = 0; i < 8; i++) arena<uint32_t>(0x400)[2 * i] = r.next(), arena<uint32_t>(0x404)[2 * i] = r.range(0, 4) | (r.next() & 0xffff0000u);
                              },
                              [](void *fn) { ((void(__thiscall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(pairs_insertion_sort);
    // Lists::ClearMarks(this): 0-4 entries in each list
    Test lists_clear_marks{0x9e2e60,
                           [=](Rng &r) {
                               for (int i = 0; i < 0x14; i++) arena<uint32_t>(0x100)[i] = r.next();
                               arena<uint32_t>(0x120)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<int32_t>(0x128)[0] = r.range(-1, 4);
                               arena<uint32_t>(0x12c)[0] = (uint32_t)(uintptr_t)arena<void>(0x800), arena<int32_t>(0x134)[0] = r.range(-1, 4);
                               for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x400)[i] = r.next(), arena<uint32_t>(0x800)[i] = r.next();
                           },
                           [](void *fn) { ((void(__thiscall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(lists_clear_marks);
    // List::MoveIn(this, entry): an entry linked, half-linked or loose, moved into a list of one node
    Test list_move_in{0x73ee30,
                      [=](Rng &r) {
                          uint32_t *link = arena<uint32_t>(0x20c), *a = arena<uint32_t>(0x300), *b = arena<uint32_t>(0x340);
                          link[0] = r.range(0, 5) ? (uint32_t)(uintptr_t)a : 0, link[1] = r.range(0, 5) ? (uint32_t)(uintptr_t)b : 0;
                          a[1] = r.range(0, 3) ? (uint32_t)(uintptr_t)link : 0, b[0] = r.range(0, 3) ? (uint32_t)(uintptr_t)link : 0;
                          arena<uint32_t>(0x404)[0] = (uint32_t)(uintptr_t)arena<void>(0x500);
                          arena<uint32_t>(0x500)[0] = (uint32_t)(uintptr_t)arena<void>(0x540), arena<uint32_t>(0x500)[1] = (uint32_t)(uintptr_t)arena<void>(0x540);
                          arena<uint32_t>(0x540)[0] = (uint32_t)(uintptr_t)arena<void>(0x500), arena<uint32_t>(0x540)[1] = (uint32_t)(uintptr_t)arena<void>(0x500);
                      },
                      [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200)); return Result{}; }};
    quick(list_move_in);
    // Memory::Move(to, from, n): overlapping either way and apart
    Test memory_move{0xdfe1a0,
                     [=](Rng &r) {
                         for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x400)[i] = r.next();
                         arena<int32_t>(0x10)[0] = r.range(0, 0x60), arena<int32_t>(0x10)[1] = r.range(0, 0x60), arena<uint32_t>(0x10)[2] = r.range(0, 0x50);
                     },
                     [](void *fn) {
                         const int32_t *a = arena<int32_t>(0x10);
                         return Result{(uint32_t)(((void *(__stdcall *)(void *, void *, uint32_t))fn)(arena<uint8_t>(0x400) + a[0], arena<uint8_t>(0x400) + a[1], (uint32_t)a[2]) == arena<uint8_t>(0x400) + a[0])};
                     }};
    quick(memory_move);
    // Ranges::FindFrom(this, n, kind, v): eight entries chained forward, kinds 0-2
    Test ranges_find_from{0x8bff50,
                          [=](Rng &r) {
                              arena<uint32_t>(0x100)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<uint32_t>(0x104)[0] = r.range(0, 8);
                              for (int i = 0; i < 8; i++) {
                                  uint8_t *e = arena<uint8_t>(0x400 + 16 * i);
                                  e[1] = (uint8_t)r.range(0, 2), *(uint32_t *)(e + 4) = r.range(0, 2) ? i + 1 + r.range(0, 1) : 0xffffffffu;
                                  const float lo = (float)r.range(0, 9);
                                  *(float *)(e + 8) = lo, *(float *)(e + 0xc) = lo + (float)r.range(0, 3);
                              }
                              arena<uint32_t>(0x10)[0] = r.range(0, 9), arena<uint32_t>(0x14)[0] = r.range(0, 2) | (r.next() & 0xffffff00u);
                              arena<float>(0x18)[0] = r.range(0, 9) ? (float)r.range(0, 12) : geo_pick(r);
                          },
                          [](void *fn) {
                              return Result{(uint32_t)((uint8_t(__thiscall *)(void *, uint32_t, uint32_t, float))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], arena<uint32_t>(0x14)[0], arena<float>(0x18)[0])};
                          }};
    quick(ranges_find_from);
    // CrashDump::ExceptionName (the code in eax) and Device::ErrorText (in ecx): each code they know, its neighbours, and
    // any other; the text's place is the result
    static const uint32_t exception_codes[] = {0x40010005, 0x40010008, 0x80000002, 0x80000003, 0xc0000005, 0xc0000006, 0xc0000017, 0xc000001d,
                                               0xc0000025, 0xc0000026, 0xc000008c, 0xc000008d, 0xc000008e, 0xc000008f, 0xc0000090, 0xc0000091,
                                               0xc0000092, 0xc0000093, 0xc0000094, 0xc0000095, 0xc0000096, 0xc00000fd, 0xc0000142, 0xe06d7363};
    static const uint32_t sound_codes[] = {0x00000000, 0x0878000a, 0x80004001, 0x80004002, 0x80004003, 0x80004005, 0x80040110, 0x80070005,
                                           0x8007000e, 0x80070057, 0x8878000a, 0x8878001e, 0x88780032, 0x88780046, 0x88780064, 0x88780078,
                                           0x88780082, 0x88780096, 0x887800a0, 0x887800aa, 0x887800b4, 0x887800be, 0x88781161};
    Test crash_exception_name{0x456fd0,
                              [](Rng &r) {
                                  const uint32_t k = exception_codes[r.range(0, 23)];
                                  arena<uint32_t>(0x10)[0] = r.range(0, 3) == 0 ? r.next() : k + (uint32_t)r.range(-2, 2) * (uint32_t)(r.range(0, 1) ? 1 : 0x10);
                              },
                              [](void *fn) {
                                  return Result{(uint32_t)(uintptr_t)((const char *(__attribute__((regparm(1))) *)(uint32_t))fn)(arena<uint32_t>(0x10)[0])};
                              }};
    quick(crash_exception_name);
    Test device_error_text{0xce6090,
                           [](Rng &r) {
                               const uint32_t k = sound_codes[r.range(0, 22)];
                               arena<uint32_t>(0x10)[0] = r.range(0, 3) == 0 ? r.next() : k + (uint32_t)r.range(-2, 2) * (uint32_t)(r.range(0, 1) ? 1 : 0xa);
                           },
                           [](void *fn) { return Result{(uint32_t)(uintptr_t)((const char *(__fastcall *)(uint32_t))fn)(arena<uint32_t>(0x10)[0])}; }};
    quick(device_error_text);
    // QuadTree::CullPlanes and CullBox: the three walks are stand-ins; the tree's box against planes or a box
    auto quad_cull_setup = [=](Rng &r) {
        uint32_t *tree = arena<uint32_t>(0x100);
        tree[0] = r.next(); tree[1] = 21; tree[2] = (uint32_t)(uintptr_t)arena<void>(0x1000);
        for (int i = 3; i < 7; i++) { float f = r.range(0, 5) ? r.real(-20, 20) : geo_pick(r); memcpy(&tree[i], &f, 4); }
        uint32_t *root = arena<uint32_t>(0x1000);
        root[0] = r.range(0, 2) ? 0x10 : r.next() & ~0x10u;
        for (int i = 1; i < 3; i++) { float f = r.range(0, 5) ? r.real(-20, 20) : geo_pick(r); memcpy(&root[i], &f, 4); }
        float *q = arena<float>(0x300);
        for (int i = 0; i < 24; i++) q[i] = r.range(0, 5) ? r.real(-20, 20) : geo_pick(r);
        arena<uint32_t>(CALL_LOG)[0] = 0;
    };
    auto quad_cull_call = [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x300)); return Result{}; };
    auto quad_cull_prepare = [](uint32_t update, uint32_t take_all, uint32_t walk) {
        return [=](const rewrite::Entry *e) {
            auto target = [&](uint32_t o) { return (uintptr_t)(e->at + o + 5 + *(int32_t *)(e->at + o + 1)); };
            stubs_for_now({{target(update), (void *)stub_quad_update}, {target(take_all), (void *)stub_quad_take_all}, {target(walk), (void *)stub_quad_walk}});
        };
    };
    Test quad_cull_planes{0x433c30, quad_cull_setup, quad_cull_call};
    quad_cull_planes.prepare = quad_cull_prepare(0x1a, 0x137, 0x155);
    quad_cull_planes.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(quad_cull_planes);
    Test quad_cull_box{0x48bb30, quad_cull_setup, quad_cull_call};
    quad_cull_box.prepare = quad_cull_prepare(0x19, 0x102, 0x117);
    quad_cull_box.finish = quad_cull_planes.finish;
    quick(quad_cull_box);
    // QuadTree::WalkPlanes: 21 nodes, items with boxes, and planes that often pass through an item's deciding corner,
    // so the order the products are added in decides; the take-all walk and the items' show are stand-ins
    Test quad_walk_planes{0x7b9590,
                          [=](Rng &r) {
                              uint32_t *tree = arena<uint32_t>(0x100);
                              tree[0] = r.next(); tree[1] = 21; tree[2] = (uint32_t)(uintptr_t)arena<void>(0x1000);
                              static void *vtable[9];
                              for (int i = 0; i < 9; i++) vtable[i] = (void *)stub_show;
                              arena<void *>(0x3000)[0] = vtable;
                              for (int i = 0; i < 21; i++) {
                                  uint32_t *node = arena<uint32_t>(0x1000 + i * 0x1c);
                                  node[0] = r.next() & (i < 5 ? 0xffffffffu : 0xfffffff0u);
                                  float lo = r.real(-20, 20), hi = lo + r.real(0, 20);
                                  memcpy(&node[1], &lo, 4); memcpy(&node[2], &hi, 4);
                                  node[3] = r.range(0, 3) ? 0 : 1;
                                  node[4] = r.range(0, 3);
                                  node[6] = (uint32_t)(uintptr_t)arena<void>(0x2000 + i * 0x60);
                                  for (int k = 0; k < 3; k++) {
                                      float *item = arena<float>(0x2000 + i * 0x60 + k * 0x20);
                                      for (int c = 0; c < 3; c++) { item[c] = r.real(-20, 20); item[c + 3] = item[c] + r.real(0, 10); }
                                      ((uint32_t *)item)[6] = (uint32_t)(uintptr_t)arena<void>(0x3000);
                                      ((uint32_t *)item)[7] = r.range(0, 4) ? 1 : 0;
                                  }
                              }
                              float *planes = arena<float>(0x300);
                              for (int p = 0; p < 6; p++) {
                                  float *pl = planes + p * 4;
                                  for (int c = 0; c < 4; c++) pl[c] = r.range(0, 7) ? r.real(-1, 1) : geo_pick(r);
                                  if (!r.range(0, 2)) {   // large values cancelling at a random item's corner: the order of adding decides
                                      int i = r.range(0, 20), k = r.range(0, 2), big = r.range(0, 2);
                                      float m = big == 0 ? 16777216.0f : big == 1 ? 9007199254740992.0f : 18446744073709551616.0f;
                                      float *item = arena<float>(0x2000 + i * 0x60 + k * 0x20);
                                      int order[3] = {0, 1, 2};
                                      for (int c = 2; c > 0; c--) { int j = r.range(0, c); int t = order[c]; order[c] = order[j]; order[j] = t; }
                                      float values[3] = {m, 1.0f, -m};
                                      for (int c = 0; c < 3; c++) { pl[c] = 1.0f; item[order[c]] = values[c]; item[order[c] + 3] = values[c] + 1.0f; }
                                      pl[3] = r.range(0, 1) ? 0.5f : -0.5f;
                                      ((uint32_t *)item)[7] = 1;
                                  } else if (r.range(0, 1)) {   // through the deciding corner of a random item
                                      int i = r.range(0, 20), k = r.range(0, 2);
                                      const float *item = arena<float>(0x2000 + i * 0x60 + k * 0x20);
                                      double d = 0;
                                      for (int c = 0; c < 3; c++) d += (double)pl[c] * (pl[c] > 0 ? item[c] : item[c + 3]);
                                      pl[3] = (float)(-d * (1 + r.range(-2, 2) * 3e-8));
                                  }
                              }
                              float *box = arena<float>(0x400);
                              for (int c = 0; c < 2; c++) { box[c] = r.real(-20, 0); box[c + 2] = r.real(0, 40); }
                              arena<int32_t>(0x10)[0] = r.range(0, 4);
                              arena<uint32_t>(CALL_LOG)[0] = 0;
                          },
                          [](void *fn) {
                              ((void(__thiscall *)(void *, void *, int32_t, void *, void *))fn)(arena<void>(0x100), arena<void>(0x300), arena<int32_t>(0x10)[0], arena<float>(0x400), arena<float>(0x408));
                              return Result{};
                          }};
    quad_walk_planes.prepare = [](const rewrite::Entry *e) {
        const uint8_t *code = (const uint8_t *)e->at;
        std::vector<std::pair<uintptr_t, void *>> stubs;
        for (int k = 0; k < 1240; k++) {
            auto target = [&](int o) { return (uintptr_t)(e->at + o + 5 + *(int32_t *)(code + o + 1)); };
            if (code[k] == 0x57 && code[k + 1] == 0x52 && code[k + 2] == 0xe8) stubs.push_back({target(k + 2), (void *)stub_quad_take_all});
            if (code[k] == 0x8b && code[k + 1] == 0x49 && code[k + 2] == 0x18 && code[k + 3] == 0xe8) stubs.push_back({target(k + 3), (void *)stub_show});
            if (code[k] == 0x8b && code[k + 1] == 0x49 && code[k + 2] == 0x18 && code[k + 3] == 0x6a && code[k + 4] == 0 && code[k + 5] == 0xe8) stubs.push_back({target(k + 5), (void *)stub_show_zero});
        }
        stubs_for_now(stubs);
    };
    quad_walk_planes.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(quad_walk_planes);
    // QuadTree::WalkBox: the same tree; a query box near the items, sometimes touching their sides, and a collector
    // that is sometimes nearly full; the take-all walk and the collector's flush are stand-ins
    Test quad_walk_box{0x7b8920,
                       [=](Rng &r) {
                           quad_walk_planes.setup(r);
                           float *q = arena<float>(0x300);
                           for (int c = 0; c < 3; c++) { q[c] = r.real(-20, 10); q[c + 3] = q[c] + r.real(0, 25); }
                           if (r.range(0, 1)) {   // touching a random item's side
                               int start = arena<int32_t>(0x10)[0];   // mostly an item of the node the walk starts at
                               int i = r.range(0, 2) ? start : r.range(0, 20), k = r.range(0, 2), c = r.range(0, 2);
                               float *item = arena<float>(0x2000 + i * 0x60 + k * 0x20);
                               ((uint32_t *)item)[7] = 1;
                               if (r.range(0, 1)) q[c] = item[c + 3]; else q[c + 3] = item[c];
                           }
                           if (!r.range(0, 9)) q[r.range(0, 5)] = geo_pick(r);
                           static void *vtable[1] = {(void *)stub_flush};
                           uint32_t *collector = arena<uint32_t>(0x3200);
                           collector[0] = (uint32_t)(uintptr_t)vtable;
                           collector[0x104 / 4] = r.range(0, 1) ? r.range(60, 63) : r.range(0, 63);
                           ((uint32_t *)q)[6] = (uint32_t)(uintptr_t)collector;
                       },
                       quad_walk_planes.call};
    quad_walk_box.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{(uintptr_t)(e->at + 0x26f + 5 + *(int32_t *)(e->at + 0x270)), (void *)stub_quad_take_all}});
    };
    quad_walk_box.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(quad_walk_box);
    // QuadTree::TakeAll: the same tree; the items' show is a stand-in
    Test quad_take_all{0x7b85f0, quad_walk_planes.setup,
                       [](void *fn) { ((void(__thiscall *)(void *, int32_t, void *))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<void>(0x300)); return Result{}; }};
    quad_take_all.prepare = [](const rewrite::Entry *e) {
        const uint8_t *code = (const uint8_t *)e->at;
        for (int k = 0; k < 130; k++)
            if (code[k] == 0x6a && code[k + 1] == 0 && code[k + 2] == 0xe8) stubs_for_now({{(uintptr_t)(e->at + k + 2 + 5 + *(int32_t *)(code + k + 3)), (void *)stub_show_zero}});
    };
    quad_take_all.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(quad_take_all);
    // QuadTree::TakeAllRect and WalkBoxRect: each item's object stands somewhere; a rectangle, sometimes on an object's
    // position, and a list that is sometimes full (its grow is a stand-in)
    auto quad_rect_setup = [=](Rng &r) {
        quad_walk_box.setup(r);
        for (int i = 0; i < 21; i++)
            for (int k = 0; k < 3; k++) {
                uint8_t *object = arena<uint8_t>(0x4000 + (i * 3 + k) * 0x100);
                *(float *)(object + 0x80) = r.real(-20, 20);
                *(float *)(object + 0x88) = r.range(0, 30) ? r.real(-20, 20) : geo_pick(r);
                *(void **)(arena<uint8_t>(0x2000 + i * 0x60 + k * 0x20) + 0x18) = object;
            }
        float *rect = arena<float>(0x3400);
        rect[0] = r.real(-20, 5); rect[1] = r.real(-20, 5); rect[2] = rect[0] + r.real(0, 30); rect[3] = rect[1] + r.real(0, 30);
        if (r.range(0, 1)) {   // on the position of an object of the starting node
            const uint8_t *object = arena<uint8_t>(0x4000 + (arena<int32_t>(0x10)[0] * 3 + r.range(0, 2)) * 0x100);
            rect[r.range(0, 1) * 2] = *(const float *)(object + 0x80);
            rect[1 + r.range(0, 1) * 2] = *(const float *)(object + 0x88);
        }
        uint32_t *list = (uint32_t *)(rect + 4);
        list[0] = (uint32_t)(uintptr_t)arena<void>(0x3500);
        list[1] = r.range(0, 8);
        list[2] = r.range(0, 1) ? list[1] : r.range(0, (int32_t)list[1]);
        ((uint32_t *)arena<float>(0x300))[6] = (uint32_t)(uintptr_t)rect;
    };
    auto quad_rect_prepare = [](const rewrite::Entry *e) {
        const uint8_t *code = (const uint8_t *)e->at;
        for (int k = 0; k < 200; k++)   // lea ecx, [esi/edi + 0x10]; call grow
            if (code[k] == 0x8d && (code[k + 1] == 0x4e || code[k + 1] == 0x4f) && code[k + 2] == 0x10 && code[k + 3] == 0xe8)
                stubs_for_now({{(uintptr_t)(e->at + k + 3 + 5 + *(int32_t *)(code + k + 4)), (void *)stub_list_grow}});
    };
    Test quad_take_all_rect{0x7b6d80, quad_rect_setup, quad_take_all.call};
    quad_take_all_rect.prepare = quad_rect_prepare;
    quad_take_all_rect.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(quad_take_all_rect);
    Test quad_walk_box_rect{0x7b82d0, quad_rect_setup, quad_walk_planes.call};
    quad_walk_box_rect.prepare = [=](const rewrite::Entry *e) {
        quad_rect_prepare(e);
        stubs_for_now({{(uintptr_t)0x7b6d80, (void *)stub_quad_take_all}});
    };
    quad_walk_box_rect.finish = quad_take_all_rect.finish;
    quick(quad_walk_box_rect);
    // QuadTree::TakeAllWeighted and WalkBoxWeighted: objects of various kinds and levels, a point among the items, a
    // small bias, a list of centres sometimes full (its grow is a stand-in)
    auto quad_weighted_setup = [=](Rng &r) {
        quad_rect_setup(r);
        for (int i = 0; i < 63; i++) {
            uint8_t *object = arena<uint8_t>(0x4000 + i * 0x100);
            *(int32_t *)(object + 0x190) = r.range(0, 3) ? r.range(0, 7) : 8;
            *(object + 0x120) = (uint8_t)r.range(0, 5);
        }
        float *point = arena<float>(0x3400);
        for (int c = 0; c < 3; c++) point[c] = r.range(0, 9) ? r.real(-20, 20) : geo_pick(r);
        for (int c = 4; c < 7; c++) point[c] = r.real(-5, 5);
        point[0x10] = r.range(0, 3) ? r.real(0, 0.02f) : r.real(-1, 1);
        point[0x11] = r.real(0, 3);
        uint32_t *words = (uint32_t *)point;
        words[0x14] = (uint32_t)(uintptr_t)arena<void>(0x3500);
        words[0x15] = r.range(0, 16);
        words[0x13] = r.range(0, 1) ? words[0x15] : r.range(0, (int32_t)words[0x15]);
    };
    auto quad_weighted_prepare = [](const rewrite::Entry *e) {
        const uint8_t *code = (const uint8_t *)e->at;
        for (int k = 0; k < 400; k++)   // lea ecx, [esi + 0x50]; call grow
            if (code[k] == 0x8d && code[k + 1] == 0x4e && code[k + 2] == 0x50 && code[k + 3] == 0xe8)
                stubs_for_now({{(uintptr_t)(e->at + k + 3 + 5 + *(int32_t *)(code + k + 4)), (void *)stub_list_grow}});
    };
    Test quad_take_all_weighted{0x7b6bd0, quad_weighted_setup, quad_take_all.call};
    quad_take_all_weighted.prepare = quad_weighted_prepare;
    quad_take_all_weighted.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(quad_take_all_weighted);
    Test quad_walk_box_weighted{0x7b7f30, quad_weighted_setup, quad_walk_planes.call};
    quad_walk_box_weighted.prepare = [=](const rewrite::Entry *e) {
        quad_weighted_prepare(e);
        stubs_for_now({{(uintptr_t)0x7b6bd0, (void *)stub_quad_take_all}});
    };
    quad_walk_box_weighted.finish = quad_take_all_weighted.finish;
    quick(quad_walk_box_weighted);
    // QuadTree::TakeAllAimed and WalkBoxAimed: objects of every kind with positions, owners through a stand-in, an aim
    // point and direction among them; the shared index is set in the call (globals aren't put back between runs)
    auto quad_aimed_setup = [=](Rng &r) {
        quad_rect_setup(r);
        static void *vtable[2] = {nullptr, (void *)stub_object_info};
        for (int i = 0; i < 63; i++) {
            uint8_t *object = arena<uint8_t>(0x4000 + i * 0x100);
            *(void **)object = vtable;
            for (int c = 0; c < 3; c++) *(float *)(object + 0x80 + c * 4) = r.range(0, 20) ? r.real(-15, 15) : geo_pick(r);
            *(int32_t *)(object + 0x190) = r.range(0, 1) ? 8 : r.range(0, 9);
            *(object + 0x120) = (uint8_t)r.range(0, 9);
            *(uint32_t *)(object + 0x40) = r.next();
            *(uint32_t *)(object + 0x20) = r.next();
        }
        uint32_t *info = arena<uint32_t>(0x8000);
        info[0x14 / 4] = (uint32_t)(uintptr_t)arena<void>(0x8100);
        info[0x18 / 4] = r.range(0, 4);
        for (int k = 0; k < 4; k++) arena<uint32_t>(0x8100)[k] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x8200 + k * 0x20) : 0;
        for (int k = 0; k < 4; k++) arena<int32_t>(0x8200 + k * 0x20)[4] = r.range(0, 4) ? 0x20 : 0;
        uint32_t *best = arena<uint32_t>(0x3400);
        best[0] = r.range(0, 1) ? (uint32_t)(uintptr_t)arena<void>(0x4000 + r.range(0, 62) * 0x100) : 0;
        float value = r.real(-1, 1); memcpy(&best[1], &value, 4);
        best[2] = r.range(0, 3);
        best[3] = r.range(0, 3) ? 0 : (uint32_t)(uintptr_t)arena<void>(0x4000 + r.range(0, 62) * 0x100);
        best[4] = r.range(0, 1) ? 0 : r.range(1, 3);
        best[5] = (uint32_t)(uintptr_t)arena<void>(0x8300);
        best[6] = (uint32_t)(uintptr_t)arena<void>(0x8310);
        for (int c = 0; c < 3; c++) arena<float>(0x8300)[c] = r.real(-10, 10);
        float dir[3] = {r.real(-1, 1), r.real(-1, 1), r.real(-1, 1)};
        for (int c = 0; c < 3; c++) arena<float>(0x8310)[c] = r.range(0, 9) ? dir[c] : geo_pick(r);
        float furthest = r.range(0, 5) ? r.real(5, 40) : geo_pick(r), least = r.real(-1, 0.8f);
        memcpy(&best[7], &furthest, 4); memcpy(&best[8], &least, 4);
        arena<int32_t>(0x14)[0] = r.range(0, 4);
        // in the starting node: a twin of the first item's object (an exact tie), and an object just over 0.5 away
        // whose distance rounds to 0.5 as a float
        int start = arena<int32_t>(0x10)[0];
        uint8_t *first = arena<uint8_t>(0x4000 + start * 3 * 0x100), *twin = first + 0x100, *close_by = first + 0x200;
        if (r.range(0, 1)) {
            memcpy(twin + 0x80, first + 0x80, 12);
            *(twin + 0x120) = *(first + 0x120);
            *(int32_t *)(twin + 0x190) = *(int32_t *)(first + 0x190);
            *(uint32_t *)(twin + 0x40) = *(uint32_t *)(first + 0x40);
            for (int k = 0; k < 2; k++) arena<uint32_t>(0x2000 + start * 0x60 + k * 0x20)[7] = 1;
        }
        if (r.range(0, 1)) {
            const float *from = arena<float>(0x8300);
            float *at = (float *)(close_by + 0x80);
            at[0] = from[0] + 0.5f; at[1] = from[1] + 0.0001f; at[2] = from[2];
            arena<uint32_t>(0x2000 + start * 0x60 + 0x40)[7] = 1;
        }
    };
    auto quad_aimed_call = [=](void *fn, bool walk) {
        *(int32_t *)0x130654c = arena<int32_t>(0x14)[0];
        if (walk) return quad_walk_planes.call(fn);
        return quad_take_all.call(fn);
    };
    auto quad_aimed_prepare = [](const rewrite::Entry *) { stubs_for_now({{(uintptr_t)0x758b10, (void *)stub_owner_of}}); };
    Test quad_take_all_aimed{0x7b5540, quad_aimed_setup, [=](void *fn) { return quad_aimed_call(fn, false); }};
    quad_take_all_aimed.prepare = quad_aimed_prepare;
    quad_take_all_aimed.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(quad_take_all_aimed);
    Test quad_walk_box_aimed{0x7b6eb0, quad_aimed_setup, [=](void *fn) { return quad_aimed_call(fn, true); }};
    quad_walk_box_aimed.prepare = [=](const rewrite::Entry *e) {
        quad_aimed_prepare(e);
        stubs_for_now({{(uintptr_t)0x7b5540, (void *)stub_quad_take_all}});
    };
    quad_walk_box_aimed.finish = quad_take_all_aimed.finish;
    quick(quad_walk_box_aimed);
    // QuadTree::WalkBoxMatching: objects with bits matched under masks, a buffer sometimes nearly full
    Test quad_walk_box_matching{0x48b0b0,
                                [=](Rng &r) {
                                    quad_rect_setup(r);
                                    for (int i = 0; i < 63; i++) {
                                        uint32_t *object = arena<uint32_t>(0x4000 + i * 0x100);
                                        object[2] = r.next(); object[6] = r.range(0, 3); object[7] = r.range(0, 3);
                                    }
                                    static void *vtable[2] = {nullptr, (void *)stub_flush_count};
                                    uint32_t *q = arena<uint32_t>(0x3400);
                                    q[0] = (uint32_t)(uintptr_t)vtable;
                                    q[2] = r.range(0, 3); q[3] = r.range(0, 3); q[4] = r.range(0, 3); q[5] = r.range(0, 3);
                                    q[7] = r.range(1, 8);
                                    q[6] = r.range(0, 1) ? q[7] - 1 : r.range(0, (int32_t)q[7] - 1);
                                    q[8] = (uint32_t)(uintptr_t)arena<void>(0x3500);
                                },
                                quad_walk_planes.call};
    quad_walk_box_matching.prepare = [](const rewrite::Entry *) { stubs_for_now({{(uintptr_t)0x48a4e0, (void *)stub_quad_take_all}}); };
    quad_walk_box_matching.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(quad_walk_box_matching);
    // QuadTree::WalkPlanesMatching: the plane walk's tree and planes, with WalkBoxMatching's objects and query after them
    Test quad_walk_planes_matching{0x48bdc0,
                                   [=](Rng &r) {
                                       quad_walk_box_matching.setup(r);
                                       float saved[24];
                                       memcpy(saved, arena<float>(0x300), sizeof saved);
                                       quad_walk_planes.setup(r);   // planes, items near them, start node
                                       for (int i = 0; i < 21; i++)
                                           for (int k = 0; k < 3; k++) *(void **)(arena<uint8_t>(0x2000 + i * 0x60 + k * 0x20) + 0x18) = arena<void>(0x4000 + (i * 3 + k) * 0x100);
                                       arena<uint32_t>(0x300)[0x18] = (uint32_t)(uintptr_t)arena<void>(0x3400);
                                   },
                                   quad_walk_planes.call};
    quad_walk_planes_matching.prepare = [](const rewrite::Entry *) { stubs_for_now({{(uintptr_t)0x48b370, (void *)stub_quad_take_all}}); };
    quad_walk_planes_matching.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(quad_walk_planes_matching);
    // QuadTree::TakeAllDrawn and WalkPlanesDrawn: records of 48 bytes at varied distances and facings; the camera and
    // the fixed view are game globals, set in the call (globals aren't put back between runs); the drawer is a stand-in
    auto quad_drawn_setup = [=](Rng &r) {
        quad_walk_planes.setup(r);
        uint32_t *target = arena<uint32_t>(0x3400);
        target[0] = (uint32_t)(uintptr_t)arena<void>(0x4000);
        target[1] = r.next();
        target[2] = r.range(0, 5) ? 1 : 0;
        for (int i = 0; i < 21; i++)
            for (int k = 0; k < 3; k++) arena<uint32_t>(0x2000 + i * 0x60 + k * 0x20)[6] = i * 3 + k;
        float *globals = arena<float>(0x18);   // camera xyz, fixed distance, fixed facing xyz, fixed flag
        for (int c = 0; c < 3; c++) globals[c] = r.real(-10, 10);
        globals[3] = r.real(0, 40);
        for (int c = 0; c < 3; c++) globals[4 + c] = r.real(-1, 1);
        ((uint32_t *)globals)[7] = r.range(0, 3) ? 0 : 1;
        for (int i = 0; i < 63; i++) {
            float *record = arena<float>(0x4000 + i * 0x30);
            for (int c = 0; c < 3; c++) record[c] = r.real(-20, 20);
            for (int c = 0; c < 3; c++) record[4 + c] = r.range(0, 9) ? r.real(-1, 1) : geo_pick(r);
            float dx = record[0] - globals[0], dy = record[1] - globals[1], dz = record[2] - globals[2];
            if (r.range(0, 1)) {   // a normal that leaves the facing test at about 0, so its roundings decide
                double v[3] = {dx, dy, dz}, vv = (double)dx * dx + (double)dy * dy + (double)dz * dz;
                double u[3] = {r.real(-1, 1), r.real(-1, 1), r.real(-1, 1)}, uv = (u[0] * v[0] + u[1] * v[1] + u[2] * v[2]) / vv;
                for (int c = 0; c < 3; c++) u[c] -= uv * v[c];   // across the view
                double a = sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2]), disc = vv * vv - 0.16 * a * a * vv;
                if (vv > 1 && a > 0 && disc > 0) {
                    double b = (-vv + sqrt(disc)) / (0.4 * vv);   // n.v = -0.2 |n|^2 for n = u + b v
                    for (int c = 0; c < 3; c++) record[4 + c] = (float)(u[c] + b * v[c]);
                }
            }
            float distance = sqrtf(dx * dx + dy * dy + dz * dz);
            record[8] = r.range(0, 3) ? distance + r.real(-3, 3) : (r.range(0, 1) ? distance : globals[3]);
            ((uint32_t *)record)[9] = r.next();
        }
        arena<uint32_t>(0x300)[0x18] = (uint32_t)(uintptr_t)target;
    };
    auto quad_drawn_globals = []() {
        const float *globals = arena<float>(0x18);
        for (int c = 0; c < 3; c++) ((float *)0x14002c0)[c] = globals[c];
        *(float *)0x14002f8 = globals[3];
        for (int c = 0; c < 3; c++) ((float *)0x14002a0)[c] = globals[4 + c];
        *(uint8_t *)0x1400306 = (uint8_t)((const uint32_t *)globals)[7];
    };
    Test quad_take_all_drawn{0x432f50, quad_drawn_setup, [=](void *fn) { quad_drawn_globals(); return quad_take_all.call(fn); }};
    quad_take_all_drawn.prepare = [](const rewrite::Entry *) { stubs_for_now({{(uintptr_t)0x430d00, (void *)stub_draw}}); };
    quad_take_all_drawn.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(quad_take_all_drawn);
    Test quad_walk_planes_drawn{0x433600, quad_drawn_setup, [=](void *fn) { quad_drawn_globals(); return quad_walk_planes.call(fn); }};
    quad_walk_planes_drawn.prepare = [](const rewrite::Entry *) { stubs_for_now({{(uintptr_t)0x430d00, (void *)stub_draw}, {(uintptr_t)0x432f50, (void *)stub_quad_take_all}}); };
    quad_walk_planes_drawn.finish = quad_take_all_drawn.finish;
    quick(quad_walk_planes_drawn);
    // CarriedItem::GetOrMake: all five slots of a person filled (the copies use one each): a handle with an object or
    // none, links in a list; the item is made through stand-ins; the object's interface through its slot 1
    Test carried_get_or_make{0xa2c990,
                             [=](Rng &r) {
                                 fresh(r);
                                 *arena<uint32_t>(ALLOC_NEXT) = 0;
                                 static void *slot_table[1] = {(void *)stub_slot_told};
                                 static void *object_table[2] = {nullptr, (void *)stub_object_info};
                                 for (int k = 0; k < 5; k++) {
                                     uint32_t *slot = arena<uint32_t>(0x100 + 0x98 + k * 0x14);
                                     uint32_t *handle = arena<uint32_t>(0x900 + k * 0x20);
                                     slot[0] = (uint32_t)(uintptr_t)slot_table;
                                     slot[1] = (uint32_t)(uintptr_t)handle;
                                     handle[3] = r.range(0, 1) ? (uint32_t)(uintptr_t)arena<void>(0x4000 + k * 0x100) : 0;
                                     // the slot's link in a ring with one other node
                                     uint32_t *other = arena<uint32_t>(0xa00 + k * 0x10);
                                     slot[2] = slot[3] = (uint32_t)(uintptr_t)other;
                                     other[0] = other[1] = (uint32_t)(uintptr_t)&slot[2];
                                     uint8_t *object = arena<uint8_t>(0x4000 + k * 0x100);
                                     *(void **)object = object_table;
                                     *(uint32_t *)(object + 0x40) = r.next();
                                 }
                                 arena<uint32_t>(0x600)[0xbc / 4] = r.next();
                                 uint32_t *new_handle = arena<uint32_t>(0x700);
                                 memset(new_handle, 0, 0x80);   // the new handle's other fields: not what earlier tests left there
                                 new_handle[0x20 / 4] = (uint32_t)(uintptr_t)arena<void>(0x780);
                                 arena<uint32_t>(0x780)[0] = (uint32_t)(uintptr_t)arena<void>(0x790);
                                 arena<uint32_t>(0x790)[1] = (uint32_t)(uintptr_t)arena<void>(0x780);
                                 uint32_t *info = arena<uint32_t>(0x8000);
                                 info[0x14 / 4] = (uint32_t)(uintptr_t)arena<void>(0x8100);
                                 info[0x18 / 4] = r.range(0, 4);
                                 for (int k = 0; k < 4; k++) arena<uint32_t>(0x8100)[k] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x8200 + k * 0x20) : 0;
                                 for (int k = 0; k < 4; k++) arena<int32_t>(0x8200 + k * 0x20)[4] = r.range(0, 3) ? r.range(0, 0x40) : -(int32_t)(uintptr_t)arena<void>(0x4000);
                                 arena<uint32_t>(0x10)[1] = r.range(0, 4);
                                 arena<uint32_t>(0x10)[2] = r.next();
                                 arena<uint32_t>(CALL_LOG)[0] = 0;
                             },
                             [](void *fn) {
                                 *(int32_t *)0x1342768 = arena<int32_t>(0x10)[1];
                                 return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *))fn)(arena<void>(0x100))};
                             }};
    carried_get_or_make.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)stub_plain_alloc}, {0xbcf850, (void *)stub_name_text}, {0xa2c580, (void *)stub_type_by_name}, {0xa29230, (void *)stub_item_make},
                       {0xa297d0, (void *)stub_item_set_type}, {0x805700, (void *)stub_new_handle}});
    };
    carried_get_or_make.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(carried_get_or_make);
    // HandymanTools::Make: the three names, the setup and the exit registration are stand-ins
    Test handyman_tool_make{0xed88b0, [=](Rng &r) { fresh(r); *arena<uint32_t>(ALLOC_NEXT) = 0; },
                            [](void *fn) { ((void(__cdecl *)())fn)(); return Result{}; }};
    handyman_tool_make.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)stub_plain_alloc}, {0xbcf850, (void *)stub_name_text}, {0x753fe0, (void *)stub_tool_setup}});
    };
    handyman_tool_make.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(handyman_tool_make);
    // Ref::PassTo: the handle, the member's add, the release and the set-up are stand-ins
    Test ref_pass_to{0x5d7b10,
                     [=](Rng &r) {
                         fresh(r);
                         uint32_t *handle = arena<uint32_t>(0x700);
                         handle[0x20 / 4] = (uint32_t)(uintptr_t)arena<void>(0x780);
                         arena<uint32_t>(0x780)[0] = (uint32_t)(uintptr_t)arena<void>(0x790);
                         arena<uint32_t>(0x790)[1] = (uint32_t)(uintptr_t)arena<void>(0x780);
                         arena<uint32_t>(CALL_LOG)[0] = 0;
                     },
                     [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x500)); return Result{}; }};
    ref_pass_to.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{0x805700, (void *)stub_new_handle}, {(uintptr_t)(e->at + 0x76 + 5 + *(int32_t *)(e->at + 0x77)), (void *)stub_ref_add},
                       {0x40f730, (void *)stub_ref_release}, {0x800860, (void *)stub_set_up}});
    };
    ref_pass_to.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(ref_pass_to);
    // QuadTree::Move: the item's handle names a node and slot; FindNode (a stand-in) gives the same node or another;
    // corners any floats (signalling NaNs too: the game copies x and y through the FPU)
    Test quad_move{0x431240,
                   [=](Rng &r) {
                       fresh(r);
                       for (int i = 0; i < 16; i++) arena<float>(0x300)[i] = r.range(0, 3) ? r.real(-50, 50) : r.any_float();
                       if (!r.range(0, 3)) { uint32_t nan = 0x7f800001 | (r.next() & 0x3fffff); memcpy(arena<float>(0x300) + r.range(0, 1), &nan, 4); }
                       arena<void *>(0x100)[2] = arena<void>(0x4000);
                       for (int n = 0; n < 21; n++) *(void **)(arena<uint8_t>(0x4000 + n * 0x1c) + 0x18) = arena<void>(0x6000 + n * 0x100);
                       int old = r.range(0, 20), slot = r.range(0, 7);
                       arena<uint32_t>(0x360)[0] = (uint32_t)old << 16 | slot | (r.range(0, 9) ? 0 : 0xffff0000u & (r.next() << 21));
                       *arena<int32_t>(0x20) = r.range(0, 1) && (arena<uint32_t>(0x360)[0] >> 16 & 0xffff) < 21 ? (int32_t)(arena<uint32_t>(0x360)[0] >> 16 & 0xffff) : r.range(0, 20);
                   },
                   [](void *fn) {
                       ((void(__thiscall *)(void *, void *, void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x340), arena<void>(0x360), arena<void>(0x300), arena<void>(0x320));
                       return Result{};
                   }};
    quad_move.prepare = [](const rewrite::Entry *e) {
        auto target = [&](int o) { return (uintptr_t)(e->at + o + 5 + *(int32_t *)(e->at + o + 1)); };
        stubs_for_now({{0xcdb600, (void *)stub_find_node}, {target(0xb1), (void *)stub_mark_changed}, {target(0xcb), (void *)stub_slot_free}, {target(0xed), (void *)stub_node_add}});
    };
    quad_move.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(quad_move);
    // QuadTree::QuadTree: a depth (0-3), two corners (any floats now and then); the allocation and the vector
    // constructor are stand-ins; the node count is the game's own (0xCDB5E0)
    Test quad_construct{0x4301e0,
                        [=](Rng &r) {
                            fresh(r);
                            *arena<uint32_t>(ALLOC_NEXT) = 0;
                            for (int i = 0; i < 4; i++) arena<float>(0x300)[i] = r.range(0, 5) ? r.real(-500, 500) : r.any_float();
                            arena<uint32_t>(0x10)[0] = r.range(0, 3);
                        },
                        [](void *fn) {
                            return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t, void *, void *))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], arena<void>(0x300), arena<void>(0x308))};
                        }};
    quad_construct.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbcdfd0, (void *)stub_alloc_next}, {0xddcf99, (void *)stub_vector_construct}}); };
    quad_construct.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(quad_construct);
    // QuadTree::WalkBoxOverlap and WalkNearPoint: WalkBox's tree and query; for the near-point walk a point among the
    // items (sometimes on a box's side), a radius squared and the collector after it
    auto quad_near_setup = [=](Rng &r) {
        quad_walk_box.setup(r);
        float *q = arena<float>(0x300);
        for (int c = 0; c < 3; c++) q[6 + c] = r.range(0, 9) ? r.real(-15, 15) : geo_pick(r);
        if (r.range(0, 1)) {   // on the side of an item of the starting node
            const float *item = arena<float>(0x2000 + arena<int32_t>(0x10)[0] * 0x60 + r.range(0, 2) * 0x20);
            int c = r.range(0, 2);
            q[6 + c] = item[c + 3 * r.range(0, 1)];
        }
        q[9] = r.range(0, 5) ? r.real(0, 200) : geo_pick(r);
        float *box = arena<float>(0x2000 + arena<int32_t>(0x10)[0] * 0x60);   // the starting node's first item
        ((uint32_t *)box)[7] = 1;
        int kind = r.range(0, 3);
        if (kind == 0) {   // whole numbers: the point outside by (1, 2, 2), the radius squared exactly 9 or not
            for (int c = 0; c < 3; c++) { box[c] = (float)r.range(-10, 10); box[c + 3] = box[c] + (float)r.range(0, 5); }
            q[6] = box[3] + 1; q[7] = box[1] - 2; q[8] = box[5] + 2;
            q[9] = (float)r.range(8, 10);
        } else if (kind == 1) {   // the radius squared at about the distance squared, so roundings decide
            double sum = 0;
            for (int c = 0; c < 3; c++) {
                double d = (double)box[c] - q[6 + c] > 0 ? (double)box[c] - q[6 + c] : (double)q[6 + c] - box[c + 3] > 0 ? (double)q[6 + c] - box[c + 3] : 0;
                sum += d * d;
            }
            q[9] = (float)(sum * (1 + r.range(-2, 2) * 3e-8));
        }
        ((uint32_t *)q)[10] = (uint32_t)(uintptr_t)arena<void>(0x3200);
    };
    Test quad_walk_near{0x7b8680, quad_near_setup, quad_walk_planes.call};
    quad_walk_near.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x25f + 5 + *(int32_t *)(e->at + 0x260)), (void *)stub_quad_take_all}}); };
    quad_walk_near.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(quad_walk_near);
    Test quad_walk_box_overlap{0x7b8bd0,
                               [=](Rng &r) {
                                   quad_walk_box.setup(r);
                                   for (int i = 1; i < 21; i++)   // some children's height not comparable
                                       if (!r.range(0, 5)) arena<uint32_t>(0x1000 + i * 0x1c)[1 + r.range(0, 1)] = 0x7fc00000;
                               },
                               quad_walk_planes.call};
    quad_walk_box_overlap.prepare = [](const rewrite::Entry *e) { stubs_for_now({{(uintptr_t)(e->at + 0x1fb + 5 + *(int32_t *)(e->at + 0x1fc)), (void *)stub_quad_take_all}}); };
    quad_walk_box_overlap.finish = quad_walk_near.finish;
    quick(quad_walk_box_overlap);
    // QuadTree::WalkNearMatching: the near-point walk's query, with WalkBoxMatching's objects and query where items go
    Test quad_walk_near_matching{0x48b6f0,
                                 [=](Rng &r) {
                                     quad_near_setup(r);
                                     for (int i = 0; i < 63; i++) {
                                         uint32_t *object = arena<uint32_t>(0x4000 + i * 0x100);
                                         object[2] = r.next(); object[6] = r.range(0, 3); object[7] = r.range(0, 3);
                                     }
                                     static void *vtable[2] = {nullptr, (void *)stub_flush_count};
                                     uint32_t *q = arena<uint32_t>(0x3400);
                                     q[0] = (uint32_t)(uintptr_t)vtable;
                                     q[2] = r.range(0, 3); q[3] = r.range(0, 3); q[4] = r.range(0, 3); q[5] = r.range(0, 3);
                                     q[7] = r.range(1, 8);
                                     q[6] = r.range(0, 1) ? q[7] - 1 : r.range(0, (int32_t)q[7] - 1);
                                     q[8] = (uint32_t)(uintptr_t)arena<void>(0x3500);
                                     arena<uint32_t>(0x300)[10] = (uint32_t)(uintptr_t)q;
                                 },
                                 quad_walk_planes.call};
    quad_walk_near_matching.prepare = [](const rewrite::Entry *) { stubs_for_now({{(uintptr_t)0x48a6e0, (void *)stub_quad_take_all}}); };
    quad_walk_near_matching.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(quad_walk_near_matching);
    // Lights::SetDirectional: a light index 0-7 and a random description
    Test lights_set_directional{0xc70da0,
                                [](Rng &r) {
                                    for (int i = 0; i < 4; i++) arena<uint32_t>(0x9000)[i] = r.next();
                                    if (r.range(0, 1)) arena<uint8_t>(0x9000)[3] = 0;
                                    arena<uint32_t>(0x10)[0] = r.range(0, 7);
                                },
                                [](void *fn) {
                                    return Result{((uint32_t(__thiscall *)(void *, uint32_t, void *))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], arena<void>(0x9000))};
                                }};
    quick(lights_set_directional);
    // Skin::Positions: 8 bones (aligned 4x4 matrices), a layout of runs (1-3 vertices, 0-3 extra bones) and skips,
    // vertices of 0x20 or 0x24 bytes with bone indices 0-7 and weights
    Test skin_positions{0xbef020,
                        [](Rng &r) {
                            float *bones = arena<float>(0x1000);
                            for (int i = 0; i < 8 * 16; i++) bones[i] = r.range(0, 30) ? r.real(-4, 4) : r.any_float();
                            uint16_t *layout = arena<uint16_t>(0x800);
                            uint32_t size = r.range(0, 1) ? 0x20 : 0x24, vertices = 0;
                            layout[0] = (uint16_t)size; layout[1] = (uint16_t)r.next(); layout[2] = (uint16_t)r.next();
                            int w = 3;
                            for (int k = 0; k < 6; k++) {
                                if (r.range(0, 3)) { int n = r.range(1, 3); layout[w++] = (uint16_t)(n << 2 | r.range(0, 3)); vertices += n; }
                                else { int n = r.range(1, 2); layout[w++] = 0; layout[w++] = (uint16_t)n; vertices += n; }
                            }
                            layout[w++] = 0; layout[w++] = 0;
                            uint8_t *v = arena<uint8_t>(0x2000);
                            for (uint32_t i = 0; i < vertices * size; i++) v[i] = (uint8_t)r.next();
                            for (uint32_t i = 0; i < vertices; i++) {
                                float *at = (float *)(v + i * size);
                                for (int c = 0; c < 3; c++) at[c] = r.range(0, 20) ? r.real(-50, 50) : r.any_float();
                                for (int b = 0; b < 4; b++) v[i * size + 0x18 + b] = (uint8_t)r.range(0, 7);
                            }
                        },
                        [](void *fn) {
                            ((void(__fastcall *)(void *, void *, void *, void *))fn)(arena<void>(0x1000), arena<void>(0x800), arena<void>(0x2000), arena<void>(0x6000));
                            return Result{};
                        }};
    quick(skin_positions);
    // Skin::PositionsX87 (two copies): the same vertices; the result is where they end
    Test skin_positions_x87{0xbeed40, skin_positions.setup, [](void *fn) {
        void *end = ((void *(__fastcall *)(void *, void *, void *, void *))fn)(arena<void>(0x1000), arena<void>(0x800), arena<void>(0x2000), arena<void>(0x6000));
        return Result{(uint32_t)((uintptr_t)end - (uintptr_t)arena<void>(0))};
    }};
    quick(skin_positions_x87);
    // Skin::PositionsNormalsX87 / ...TangentsX87 (four copies): each copy's vertex layout (bones, weights, size, out
    // size) from a table; runs of 1-3 vertices with 0-3 extra bones, skips, 0-8 extra bytes copied; bone bytes 0-7
    // (also where a copy's weights share them)
    struct SkinCopy { uint32_t at, bones, weights, size, out; };
    static const SkinCopy skin_copies[] = {{0xbef5a0, 0x18, 0x1a, 0x1c, 0x18}, {0xbefa90, 0x18, 0x1c, 0x20, 0x18}, {0xbeffa0, 0x28, 0x2a, 0x2c, 0x28}, {0xbf0520, 0x28, 0x2c, 0x30, 0x28}};
    static const SkinCopy *g_skin_copy = &skin_copies[0];
    Test skin_x87{0xbef5a0,
                  [](Rng &r) {
                      const SkinCopy &c = *g_skin_copy;
                      float *bones = arena<float>(0x1000);
                      for (int i = 0; i < 8 * 16; i++) bones[i] = r.range(0, 30) ? r.real(-4, 4) : r.any_float();
                      const uint32_t extra = 4 * r.range(0, 2);
                      uint16_t *layout = arena<uint16_t>(0x800);
                      layout[0] = (uint16_t)(c.size + extra + 4 * r.range(0, 1) * r.range(0, 1));
                      layout[1] = (uint16_t)(c.out + extra + 4 * r.range(0, 1));
                      layout[2] = (uint16_t)extra;
                      int w = 3, vertices = 0;
                      for (int k = 0; k < 6; k++) {
                          if (r.range(0, 3)) { int n = r.range(1, 3); layout[w++] = (uint16_t)(n << 2 | r.range(0, 3)); vertices += n; }
                          else { int n = r.range(1, 2); layout[w++] = 0; layout[w++] = (uint16_t)n; vertices += n; }
                      }
                      layout[w++] = 0; layout[w++] = 0;
                      uint8_t *v = arena<uint8_t>(0x2000);
                      for (int i = 0; i < vertices * layout[0] + 0x40; i++) v[i] = (uint8_t)r.next();
                      for (int i = 0; i < vertices; i++) {
                          uint8_t *at = v + i * layout[0];
                          for (int k = 0; k < 9; k++) ((float *)at)[k] = r.range(0, 20) ? r.real(-50, 50) : r.any_float();
                          for (uint32_t b = c.bones; b < c.weights + 4; b++)
                              if (b < c.bones + 4) at[b] = (uint8_t)r.range(0, 7);
                      }
                  },
                  [](void *fn) {
                      void *end = ((void *(__fastcall *)(void *, void *, void *, void *))fn)(arena<void>(0x1000), arena<void>(0x800), arena<void>(0x2000), arena<void>(0x6000));
                      return Result{(uint32_t)((uintptr_t)end - (uintptr_t)arena<void>(0))};
                  }};
    for (const SkinCopy &c : skin_copies) {
        skin_x87.at = c.at;
        skin_x87.prepare = [&c](const rewrite::Entry *) { g_skin_copy = &c; };
        quick(skin_x87);
    }
    // Skin::PositionsNormals: vertices {position, normal, bones, weights, extra bytes}, packed per copy (2 or 4 bones,
    // read from its code) when called; out strides from the layout
    static uint32_t g_skin_vertex_size;
    Test skin_positions_normals{0xbef8f0,
                                [=](Rng &r) {
                                    skin_positions.setup(r);
                                    for (int i = 0; i < 40 * 12; i++) arena<uint32_t>(0x9000)[i] = r.next();   // raw: 6 floats, 4 bones, 4 weights, 8 extra
                                    for (int i = 0; i < 40; i++) {
                                        uint8_t *at = arena<uint8_t>(0x9000 + i * 0x30);
                                        for (int c = 0; c < 6; c++) ((float *)at)[c] = r.range(0, 20) ? r.real(-50, 50) : r.any_float();
                                        for (int b = 0; b < 4; b++) at[0x18 + b] = (uint8_t)r.range(0, 7);
                                    }
                                    arena<uint32_t>(0x10)[0] = 4 * r.range(0, 2);
                                    arena<uint32_t>(0x10)[1] = 4 * r.range(0, 1);
                                    uint16_t *run = arena<uint16_t>(0x800) + 3;   // up to 1 extra bone
                                    while (run[0] || run[1]) {
                                        if (run[0]) { run[0] = (uint16_t)((run[0] & ~3) | (run[0] & 1)); run++; }
                                        else run += 2;
                                    }
                                },
                                [=](void *fn) {
                                    uint32_t size = g_skin_vertex_size, bones = size == 0x1c ? 2 : 4, extra = arena<uint32_t>(0x10)[0];
                                    uint16_t *layout = arena<uint16_t>(0x800);
                                    layout[0] = (uint16_t)(size + extra);
                                    layout[1] = (uint16_t)(0x18 + extra + arena<uint32_t>(0x10)[1]);
                                    layout[2] = (uint16_t)extra;
                                    for (int i = 0; i < 40; i++) {
                                        const uint8_t *raw = arena<uint8_t>(0x9000 + i * 0x30);
                                        uint8_t *at = arena<uint8_t>(0x2000 + i * layout[0]);
                                        memcpy(at, raw, 0x18 + bones);
                                        memcpy(at + 0x18 + bones, raw + 0x1c, bones);
                                        memcpy(at + size, raw + 0x20, extra);
                                    }
                                    return skin_positions.call(fn);
                                }};
    skin_positions_normals.prepare = [](const rewrite::Entry *e) {
        const uint8_t *code = (const uint8_t *)e->at;
        for (int k = 0x100; k < 0x190; k++)   // add esi, SIZE; add edi, 0x18
            if (code[k] == 0x83 && code[k + 1] == 0xc6 && code[k + 3] == 0x83 && code[k + 4] == 0xc7 && code[k + 5] == 0x18) g_skin_vertex_size = code[k + 2];
    };
    quick(skin_positions_normals);
    // Skin::Full: vertices of 0x40 bytes (out 0x30) so every copy's fields fit; bone bytes 0-7 where any copy keeps its
    // bones; alpha anywhere (NaN, negative, over 1, halfway between whole numbers); a uv scale and offset
    Test skin_full{0xbf0e70,
                   [=](Rng &r) {
                       skin_positions.setup(r);
                       uint16_t *layout = arena<uint16_t>(0x800);
                       layout[0] = 0x40; layout[1] = 0x30; layout[2] = (uint16_t)r.next();
                       uint8_t *v = arena<uint8_t>(0x2000);
                       for (int i = 0; i < 40; i++) {
                           uint8_t *at = v + i * 0x40;
                           for (int c = 0; c < 16; c++) ((float *)at)[c] = r.range(0, 20) ? r.real(-50, 50) : r.any_float();
                           for (int b = 0; b < 4; b++) at[0x18 + b] = at[0x28 + b] = (uint8_t)r.range(0, 7);
                           for (int b = 0; b < 4; b++) at[0x2c + b] = (uint8_t)r.next();
                           at[0x1f] = (uint8_t)r.next(); at[0x2f] = (uint8_t)r.next(); at[0x33] = (uint8_t)r.next();
                       }
                       int a = r.range(0, 5);
                       arena<float>(0x10)[0] = a == 0 ? r.any_float() : a == 1 ? (r.range(0, 510) + 0.5f) / 255.0f * (r.range(0, 1) ? 1 : -1) : r.real(-0.5f, 1.5f);
                       for (int c = 0; c < 4; c++) arena<float>(0x700)[c] = r.range(0, 9) ? r.real(-4, 4) : geo_pick(r);
                   },
                   [](void *fn) {
                       ((void(__fastcall *)(void *, void *, float, void *, void *, void *, void *))fn)(arena<void>(0x1000), arena<void>(0x700), arena<float>(0x10)[0],
                                                                                                       arena<void>(0x708), arena<void>(0x800), arena<void>(0x2000), arena<void>(0x6000));
                       return Result{};
                   }};
    quick(skin_full);
    // Skin::FullX87 (two copies): the same vertices; the result is where they end
    Test skin_full_x87{0xbf0ac0, skin_full.setup, [](void *fn) {
        void *end = ((void *(__fastcall *)(void *, void *, float, void *, void *, void *, void *))fn)(arena<void>(0x1000), arena<void>(0x700), arena<float>(0x10)[0],
                                                                                                      arena<void>(0x708), arena<void>(0x800), arena<void>(0x2000), arena<void>(0x6000));
        return Result{(uint32_t)((uintptr_t)end - (uintptr_t)arena<void>(0))};
    }};
    quick(skin_full_x87);
    // Skin::PositionsNormalsTangents: vertices {position, normal, tangent, word, bones, weights, extra}, packed per copy
    static uint32_t g_skin_tangent_size;
    Test skin_tangents{0xbf0340,
                       [=](Rng &r) {
                           skin_positions_normals.setup(r);
                           for (int i = 0; i < 40; i++) {
                               uint8_t *at = arena<uint8_t>(0x9000 + i * 0x40);
                               for (int c = 0; c < 9; c++) ((float *)at)[c] = r.range(0, 20) ? r.real(-50, 50) : r.any_float();
                               ((uint32_t *)at)[9] = r.next();
                               for (int b = 0; b < 4; b++) { at[0x28 + b] = (uint8_t)r.range(0, 7); at[0x2c + b] = (uint8_t)r.next(); }
                               for (int b = 0; b < 8; b++) at[0x30 + b] = (uint8_t)r.next();
                           }
                       },
                       [=](void *fn) {
                           uint32_t size = g_skin_tangent_size, bones = size == 0x2c ? 2 : 4, extra = arena<uint32_t>(0x10)[0];
                           uint16_t *layout = arena<uint16_t>(0x800);
                           layout[0] = (uint16_t)(size + extra);
                           layout[1] = (uint16_t)(0x28 + extra + arena<uint32_t>(0x10)[1]);
                           layout[2] = (uint16_t)extra;
                           for (int i = 0; i < 40; i++) {
                               const uint8_t *raw = arena<uint8_t>(0x9000 + i * 0x40);
                               uint8_t *at = arena<uint8_t>(0x2000 + i * layout[0]);
                               memcpy(at, raw, 0x28 + bones);
                               memcpy(at + 0x28 + bones, raw + 0x2c, bones);
                               memcpy(at + size, raw + 0x30, extra);
                           }
                           return skin_positions.call(fn);
                       }};
    skin_tangents.prepare = [](const rewrite::Entry *e) {
        const uint8_t *code = (const uint8_t *)e->at;
        for (int k = 0x100; k < 0x1d0; k++)   // add esi, SIZE; add edi, 0x28
            if (code[k] == 0x83 && code[k + 1] == 0xc6 && code[k + 3] == 0x83 && code[k + 4] == 0xc7 && code[k + 5] == 0x28) g_skin_tangent_size = code[k + 2];
    };
    quick(skin_tangents);
    // Window::Construct: a title object (its name or none), a parent or none, a text (a block with references) or none;
    // the wide-string functions, the base window and the members' constructors are stand-ins
    Test window_construct{0x643800,
                          [=](Rng &r) {
                              fresh(r);
                              *arena<uint32_t>(ALLOC_NEXT) = 0;
                              uint32_t *title = arena<uint32_t>(0x8000);
                              title[1] = 0;
                              title[2] = r.range(0, 2) ? (uint32_t)(uintptr_t)"Title" : 0;
                              arena<uint32_t>(0x8800)[0] = 100;
                              uint32_t *text = (uint32_t *)new_wide_block();
                              text[0] = r.range(1, 2);
                              arena<uint32_t>(0x30)[0] = (uint32_t)(uintptr_t)text;
                              arena<uint32_t>(0x10)[0] = r.range(0, 2) ? (uint32_t)(uintptr_t)arena<void>(0x8100) : 0;
                              arena<uint32_t>(0x10)[1] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x30) : 0;
                              arena<uint32_t>(0x10)[2] = r.next();
                              arena<uint32_t>(CALL_LOG)[0] = 0;
                          },
                          [](void *fn) {
                              *(void **)g_window_title = arena<void>(0x8000);
                              uint32_t *a = arena<uint32_t>(0x10);
                              void *made = g_window_args == 3 ? ((void *(__thiscall *)(void *, void *, void *, uint32_t))fn)(arena<void>(0x100), (void *)(uintptr_t)a[0], (void *)(uintptr_t)a[1], a[2])
                                                               : ((void *(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x100), (void *)(uintptr_t)a[0], (void *)(uintptr_t)a[1]);
                              return Result{(uint32_t)((uintptr_t)made - (uintptr_t)arena<void>(0))};
                          }};
    window_construct.prepare = [](const rewrite::Entry *e) {
        const uint8_t *code = (const uint8_t *)e->at;
        for (int k = 0x100; k < 0x400; k++)   // its ret N, soon after the exception frame is put back (mov fs:[0], ecx)
            if (code[k] == 0x64 && code[k + 1] == 0x89 && code[k + 2] == 0x0d && !*(const uint32_t *)(code + k + 3)) {
                for (int m = k + 7; m < k + 24; m++)
                    if (code[m] == 0xc2 && code[m + 2] == 0) { g_window_args = code[m + 1] / 4; break; }
                break;
            }
        for (int k = 0; k < 0x40; k++)
            if (code[k] == 0xa1 && code[k - 1] != 0x64 && *(const uint32_t *)(code + k + 1) >= 0x1000000) { g_window_title = *(const uint32_t *)(code + k + 1); break; }   // mov eax, [title]
        stubs_for_now({{0xbcdfc0, (void *)stub_plain_alloc}, {0xbcf850, (void *)stub_name_text}, {0xbd0490, (void *)stub_wide_empty_shared},
                       {0x403be0, (void *)stub_wide_assign_c}, {0x403bb0, (void *)stub_wide_assign}, {0x41c740, (void *)stub_wide_copy},
                       {0x6af7e0, (void *)stub_window_base}, {0x441030, (void *)stub_member_built}});
    };
    window_construct.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(window_construct);
    // Window::Titled: as Window::Construct, with any base and its values, up to six arguments (the rest random)
    Test titled_construct{0x41ce20,
                          [=](Rng &r) {
                              fresh(r);
                              g_text_ref = nullptr;   // (left from an earlier copy, it could match a temporary's place by chance)
                              *arena<uint32_t>(ALLOC_NEXT) = 0;
                              for (uint32_t k = 0; k < 0x500; k += 4)   // the window's members: each points somewhere (16 places), so writes through them land
                                  arena<uint32_t>(0x100 + k)[0] = (uint32_t)(uintptr_t)arena<void>(0xc000 + k / 4 % 16 * 0x40);
                              for (uint32_t k = 0; k < g_child_array_count; k++)   // (the children already there holding an array of counted texts: one)
                                  if (g_child_arrays[k][2] < 0x500 && g_child_arrays[k][0] < 0x400 && g_child_arrays[k][1] <= 64) {
                                      void **array = (void **)stub_plain_alloc(4 * g_child_arrays[k][1]);
                                      for (uint32_t q = 0; q < g_child_arrays[k][1]; q++) array[q] = new_wide_block();
                                      *(void ***)(arena<uint8_t>(0x100 + g_child_arrays[k][2])[0] ? (uint8_t *)(uintptr_t)arena<uint32_t>(0x100 + g_child_arrays[k][2])[0] + g_child_arrays[k][0] : nullptr) = array;
                                  }
                              uint32_t *title = arena<uint32_t>(0x8000);
                              title[1] = 0;
                              *(uint8_t *)&title[1] = (uint8_t)(r.range(0, 2) == 0);   // (+4: the default named already, or not)
                              arena<uint8_t>(0x8104)[0] = (uint8_t)(r.range(0, 2) == 0);   // (and the description given)
                              title[2] = r.range(0, 2) ? (uint32_t)(uintptr_t)"Title" : 0;
                              arena<uint32_t>(0x8800)[0] = 100;
                              uint32_t *text = (uint32_t *)new_wide_block();
                              text[0] = r.range(1, 2);
                              arena<uint32_t>(0x30)[0] = (uint32_t)(uintptr_t)text;
                              arena<uint32_t>(0x10)[0] = r.range(0, 2) ? (uint32_t)(uintptr_t)arena<void>(0x8100) : 0;
                              arena<uint32_t>(0x10)[1] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x30) : 0;
                              for (int k = 2; k < 8; k++) {   // the other arguments: pointers to filled memory (read through, or kept as values)
                                  arena<uint32_t>(0x10)[k] = (uint32_t)(uintptr_t)arena<void>(0x9000 + k * 0x40);
                                  for (int q = 0; q < 16; q++) arena<uint32_t>(0x9000 + k * 0x40)[q] = r.next();
                                  // (its first dword a shared text's place, for the copies that hold what it holds)
                                  arena<uint32_t>(0x9000 + k * 0x40)[0] = (uint32_t)(uintptr_t)arena<void>(0x9400 + k * 0x10);
                                  arena<uint32_t>(0x9400 + k * 0x10)[0] = r.range(1, 3);
                              }
                              for (int k = 2; k < 8; k++)   // (the arguments cast to an interface: objects whose class info is the case's; argument k + 1 is a[k])
                                  if (g_titled_iface_mask >> (k + 1) & 1) arena<uint32_t>(0x9000 + k * 0x40)[0] = (uint32_t)(uintptr_t)titled_iface_table;
                                  else if (g_titled_vcall_mask >> (k + 1) & 1) arena<uint32_t>(0x9000 + k * 0x40)[0] = (uint32_t)(uintptr_t)titled_child_table;
                              // (and the parent, when it is the one cast or called on: the object at 0x8100, when it is given)
                              if (g_titled_iface_mask >> g_titled_parent & 1) arena<uint32_t>(0x8100)[0] = (uint32_t)(uintptr_t)titled_iface_table;
                              else if (g_titled_vcall_mask >> g_titled_parent & 1) arena<uint32_t>(0x8100)[0] = (uint32_t)(uintptr_t)titled_child_table;
                              {
                                  uint32_t *info = arena<uint32_t>(0x9a00);   // +0x14 the entries, +0x18 their count; each entry's +0x10 an offset
                                  info[15] = r.range(0, 4) != 0, info[5] = (uint32_t)(uintptr_t)arena<void>(0x9a40), info[6] = r.range(0, 4);
                                  for (int q = 0; q < 4; q++) {
                                      arena<uint32_t>(0x9a40)[q] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x9a80 + q * 0x20) : 0;
                                      arena<uint32_t>(0x9a80 + q * 0x20)[4] = 4 * r.range(0, 8);
                                  }
                              }
                              for (uint32_t h : {0x700u, 0x600u}) {   // the empty handle and the one of no object, each with a list
                                  arena<uint32_t>(h)[0x20 / 4] = (uint32_t)(uintptr_t)arena<void>(h + 0x80);
                                  arena<uint32_t>(h + 0x80)[0] = (uint32_t)(uintptr_t)arena<void>(h + 0x90);
                                  arena<uint32_t>(h + 0x90)[1] = (uint32_t)(uintptr_t)arena<void>(h + 0x80);
                              }
                              for (int k = 0; k < 16; k++) {   // the children's sizes
                                  const uint32_t odd[] = {0x7fc00000, 0x7f800000, 0xff800000, 0x7fa00001, 0};
                                  uint32_t bits = r.range(0, 5) ? r.next() >> 3 | (r.range(0, 1) ? 0 : 0x80000000u) : odd[r.range(0, 4)];
                                  if (r.range(0, 1)) {
                                      const float plain = (float)r.range(0, 900) + 0.5f * (float)r.range(0, 1);
                                      memcpy(&bits, &plain, 4);
                                  }
                                  ((volatile uint32_t *)arena<uint32_t>(0x8f00))[k] = bits;   // (as bits: through the FPU a signalling NaN would come out quiet)
                              }
                              {   // the window's rectangle (+0x7C..+0x8C) and some of what members point to: floats, odd ones among them
                                  const uint32_t odd[] = {0x7fa00001, 0xffc00000, 0x7fc00000, 0x7f800000, 0x80000000, 0x00000001, 0x7fe00003, 0xff900005};
                                  auto value = [&]() -> uint32_t {
                                      if (r.range(0, 3) == 0) return odd[r.range(0, 7)];
                                      const float f = (float)r.range(-4000, 4000) / 8.0f;
                                      uint32_t b; memcpy(&b, &f, 4); return b;
                                  };
                                  for (uint32_t off = 0x7c; off <= 0x8c; off += 4) arena<uint32_t>(0x100 + off)[0] = value();
                                  for (uint32_t blk = 0; blk < 16; blk++)
                                      for (uint32_t off = 0x7c; off <= 0x8c; off += 4) arena<uint32_t>(0xc000 + blk * 0x40 + (off & 0x3f))[0] = value();
                                  for (uint32_t g = 0; g < 32; g++)   // (the objects game globals hold: floats too, as their rectangles are read)
                                      for (uint32_t off = 4; off < 0x100; off += 4) {   // (its first members places in the landing blocks, as a built object's)
                                          const uint32_t f_ = value();
                                          arena<uint32_t>(titled_object_place(g) + off)[0] = off < 0x2c ? (uint32_t)(uintptr_t)titled_landing(arena<void>(titled_object_place(g)), off / 4) : f_;
                                      }
                                  for (uint32_t blk = 0; blk < 16; blk++)   // (each what a member points to: an object, with the table of stand-ins)
                                      arena<uint32_t>(0xc000 + blk * 0x40)[0] = (uint32_t)(uintptr_t)titled_child_table;
                              }
                              arena<uint32_t>(0x100)[0xe0 / 4] = (uint32_t)(uintptr_t)arena<void>(0x8c00);   // (the parent the base keeps)
                              for (int k = 0; titled_compared_fields[3 * k]; k++)   // (members compared with a constant: values near it)
                                  if (titled_compared_fields[3 * k] == g_titled_at)
                                      arena<uint32_t>(0x100 + titled_compared_fields[3 * k + 1])[0] = titled_compared_fields[3 * k + 2] + (uint32_t)r.range(0, 4) - 2;
                              for (int k = 0; titled_iface_places[2 * k] != 0xffffffffu && k < 8; k++) {   // (objects inside children cast to an interface)
                                  uint8_t *in = (uint8_t *)(uintptr_t)arena<uint32_t>(0x100 + titled_iface_places[2 * k])[0] + titled_iface_places[2 * k + 1];
                                  arena<uint32_t>(0x9b00 + k * 0x10)[0] = (uint32_t)(uintptr_t)titled_iface_table;
                                  *(uint32_t *)in = (uint32_t)(uintptr_t)arena<void>(0x9b00 + k * 0x10);
                              }
                              for (int k = 0; titled_text_holders[2 * k] != 0xffffffffu && k < 8; k++) {   // (the holders counted texts go into: a text each)
                                  const uint32_t member = titled_text_holders[2 * k], inner = titled_text_holders[2 * k + 1];
                                  uint8_t *holder = member == 0xffff ? arena<uint8_t>(0x100 + inner) : (uint8_t *)(uintptr_t)arena<uint32_t>(0x100 + member)[0] + inner;
                                  uint32_t *held = arena<uint32_t>(0x8e00 + k * 0x20);
                                  held[0] = r.range(1, 2), held[1] = r.next(), held[2] = r.next(), held[3] = (uint32_t)(uintptr_t)arena<void>(0x8e10 + k * 0x20);
                                  *(uint32_t *)holder = (uint32_t)(uintptr_t)held;
                              }
                              arena<uint32_t>(0x4c)[0] = r.next();   // (which of the arguments only a cast reads are 0 this time)
                              arena<uint32_t>(CALL_LOG)[0] = 0;
                          },
                          [](void *fn) {
                              g_text_ref = nullptr;   // (each run from none: the other run's temporary is not this one's)
                              for (int k = 0; titled_class_globals[k] && k < 16; k++) {   // (the class numbers the interface casts look up: small ones)
                                  g_titled_class_saved[k] = *(volatile uint32_t *)(uintptr_t)titled_class_globals[k];
                                  *(volatile int32_t *)(uintptr_t)titled_class_globals[k] = (int32_t)((arena<uint32_t>(0x4c)[0] >> (2 * k + 1)) % 4);
                              }
                              for (int k = 0; titled_read_globals[k] && k < 256; k++) {   // (the globals read as values: values of the case's)
                                  g_titled_read_saved[k] = *(volatile uint32_t *)(uintptr_t)titled_read_globals[k];
                                  const uint32_t v = arena<uint32_t>(0x4c)[0] * 0x9e3779b1u + (uint32_t)k * 0x85ebca6bu;
                                  *(volatile uint32_t *)(uintptr_t)titled_read_globals[k] = v % 5 == 0 ? 0 : v;
                              }
                              for (int k = 0; titled_text_globals[k] && k < 8; k++) {   // (the globals a text is made from: one, or none)
                                  g_titled_text_saved[k] = *(volatile uint32_t *)(uintptr_t)titled_text_globals[k];
                                  uint16_t *wide = arena<uint16_t>(0x7e00 + k * 0x20);
                                  for (int c = 0; c < 6; c++) wide[c] = (uint16_t)('a' + (arena<uint32_t>(0x4c)[0] >> (c + k)) % 26);
                                  wide[6] = 0;
                                  *(volatile uint32_t *)(uintptr_t)titled_text_globals[k] = (arena<uint32_t>(0x4c)[0] >> (12 + k)) % 3 ? (uint32_t)(uintptr_t)wide : 0;
                              }
                              for (int k = 0; titled_written_globals[2 * k] && k < 16; k++) {   // (the globals the steps write: a value of the case's first)
                                  volatile uint8_t *g = (volatile uint8_t *)(uintptr_t)titled_written_globals[2 * k];
                                  g_titled_written_saved[k] = 0;
                                  for (uint32_t b = 0; b < titled_written_globals[2 * k + 1]; b++) {
                                      ((uint8_t *)&g_titled_written_saved[k])[b] = g[b];
                                      g[b] = arena<uint8_t>(0x4c)[b] ^ (uint8_t)k;
                                  }
                              }
                              for (int k = 0; titled_object_globals[k] && k < 32; k++) {   // (the globals called on: objects of the arena's)
                                  g_titled_object_saved[k] = *(volatile uint32_t *)(uintptr_t)titled_object_globals[k];
                                  *(volatile uint32_t *)(uintptr_t)titled_object_globals[k] = (uint32_t)(uintptr_t)arena<void>(titled_object_place(k));
                                  arena<uint32_t>(titled_object_place(k))[0] = (uint32_t)(uintptr_t)titled_child_table;   // (its table: the stand-ins)
                                  for (int q = 0x0b; q < 0x40; q++)   // (its members from +0x2C on point into the filled places, so a chain read through one lands)
                                      arena<uint32_t>(titled_object_place(k))[q] = (uint32_t)(uintptr_t)arena<void>(0xc000 + ((q * 3 + k) % 16) * 0x40);
                              }
                              for (int k = 0; titled_list_globals[k] && k < 8; k++) {   // (the lists a node is put first in: one node in each, or none)
                                  g_titled_list_saved[k] = *(volatile uint32_t *)(uintptr_t)titled_list_globals[k];
                                  uint32_t *head = arena<uint32_t>(0xe000 + k * 0x20), *one = head + 4;
                                  *(volatile uint32_t *)(uintptr_t)titled_list_globals[k] = (uint32_t)(uintptr_t)head;
                                  if (arena<uint32_t>(0x4c)[0] >> (9 + k) & 1)
                                      head[0] = head[1] = (uint32_t)(uintptr_t)one, one[0] = one[1] = (uint32_t)(uintptr_t)head;
                                  else
                                      head[0] = head[1] = (uint32_t)(uintptr_t)head;
                              }
                              if (g_titled_single_global && arena<uint32_t>(0x4c)[0] >> 7 & 1)   // (the singleton not made yet, in some cases)
                                  *(volatile uint32_t *)(uintptr_t)g_titled_single_global = 0;
                              *(void **)g_titled_title = arena<void>(0x8000);
                              g_null_handle = arena<void>(0x700);
                              uint32_t *a = arena<uint32_t>(0x10);
                              if (g_titled_parent != 1 || g_titled_text != 2) {   // (the parent and the text in the copy's own arguments; blocks elsewhere)
                                  const uint32_t parent = a[0], text = a[1], blocks[6] = {a[2], a[3], a[4], a[5], a[6], a[7]};
                                  for (uint32_t k = 1, b = 0; k <= 8; k++)
                                      a[k - 1] = k == g_titled_parent ? parent : k == g_titled_text ? text : blocks[b++ % 6];
                              }
                              for (int k = 2; k <= 8; k++)   // (the flag arguments a byte test reads: 0, 1 or 2)
                                  if (g_titled_byte_flags >> k & 1)   // (the upper bytes whatever the caller left: only the low one is the flag)
                                      a[k - 1] = (arena<uint32_t>(0x4c)[0] >> (2 * k)) % 3 | ((arena<uint32_t>(0x4c)[0] * 0x9e3779b1u + (uint32_t)k * 0x85ebca6bu) & 0xffffff00u);
                              for (int k = 2; k < 8; k++)
                                  if (g_titled_nullable >> k & 1 && (arena<uint32_t>(0x4c)[0] >> (3 * k)) % 3 == 0) a[k - 1] = 0;
                              if (g_titled_at == 0x6e1e70 && a[2] >= (uint32_t)(uintptr_t)(g_arena + 0x100) && a[2] < (uint32_t)(uintptr_t)(g_arena + sizeof g_arena - 0x88))
                                  *(uint32_t *)(uintptr_t)(a[2] + 0x84) = arena<uint32_t>(0x4c)[0] >> 3 & 7;   // (its loop's count, in the source it is given: 0 to 7)
                              for (uint32_t k = 1; k <= 8; k++) {   // (the arguments cast or called on, wherever the moves above put them: their tables)
                                  uint8_t *o = (uint8_t *)(uintptr_t)a[k - 1];
                                  if (o < g_arena + 0x100 || o >= g_arena + sizeof g_arena - 4 || (k == g_titled_text)) continue;
                                  if (g_titled_holder_mask >> k & 1) *(uint32_t *)o = (uint32_t)(uintptr_t)titled_holder_table;
                                  else if (g_titled_iface_mask >> k & 1) *(uint32_t *)o = (uint32_t)(uintptr_t)titled_iface_table;
                                  else if (g_titled_vcall_mask >> k & 1) *(uint32_t *)o = (uint32_t)(uintptr_t)titled_child_table;
                              }
                              void *self = arena<void>(0x100), *made;
                              for (int k = 0, n = 0; titled_window_lists[k]; k += 2)   // (the window's members holding a list a node is put first in)
                                  if (titled_window_lists[k] == g_titled_at && n < 8) {
                                      uint32_t *head = arena<uint32_t>(0xe100 + n++ * 0x20), *one = head + 4;
                                      *(uint32_t *)((uint8_t *)self + titled_window_lists[k + 1]) = (uint32_t)(uintptr_t)head;
                                      if (arena<uint32_t>(0x4c)[0] >> (17 + n) & 1)
                                          head[0] = head[1] = (uint32_t)(uintptr_t)one, one[0] = one[1] = (uint32_t)(uintptr_t)head;
                                      else
                                          head[0] = head[1] = (uint32_t)(uintptr_t)head;
                                  }
                              switch (g_titled_args) {
                              case 2: made = ((void *(__thiscall *)(void *, uint32_t, uint32_t))fn)(self, a[0], a[1]); break;
                              case 3: made = ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))fn)(self, a[0], a[1], a[2]); break;
                              case 4: made = ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))fn)(self, a[0], a[1], a[2], a[3]); break;
                              case 5: made = ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))fn)(self, a[0], a[1], a[2], a[3], a[4]); break;
                              case 6: made = ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))fn)(self, a[0], a[1], a[2], a[3], a[4], a[5]); break;
                              case 7: made = ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))fn)(self, a[0], a[1], a[2], a[3], a[4], a[5], a[6]); break;
                              default: made = ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))fn)(self, a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]); break;
                              }
                              for (int k = 0; titled_object_globals[k] && k < 32; k++) *(volatile uint32_t *)(uintptr_t)titled_object_globals[k] = g_titled_object_saved[k];
                              for (int k = 0; titled_list_globals[k] && k < 8; k++) *(volatile uint32_t *)(uintptr_t)titled_list_globals[k] = g_titled_list_saved[k];
                              for (int k = 0; titled_text_globals[k] && k < 8; k++) *(volatile uint32_t *)(uintptr_t)titled_text_globals[k] = g_titled_text_saved[k];
                              for (int k = 0; titled_read_globals[k] && k < 256; k++) *(volatile uint32_t *)(uintptr_t)titled_read_globals[k] = g_titled_read_saved[k];
                              for (int k = 0; titled_class_globals[k] && k < 16; k++) *(volatile uint32_t *)(uintptr_t)titled_class_globals[k] = g_titled_class_saved[k];
                              for (int k = 0; titled_written_globals[2 * k] && k < 16; k++) {   // (what they hold now: compared; then put back)
                                  volatile uint8_t *g = (volatile uint8_t *)(uintptr_t)titled_written_globals[2 * k];
                                  for (uint32_t b = 0; b < titled_written_globals[2 * k + 1]; b++) {
                                      uint32_t v = g[b];
                                      if (b == 0 && titled_written_globals[2 * k + 1] == 4) {   // (the window's place: as an offset in the arena)
                                          const uint32_t whole = *(volatile uint32_t *)g;
                                          const uint32_t base = (uint32_t)(uintptr_t)arena<void>(0);
                                          if (whole - base < sizeof g_arena) { *(volatile uint32_t *)g = whole - base; v = g[0]; }
                                      }
                                      arena<uint8_t>(0x7f00 + k * 4)[b] = (uint8_t)v;
                                      g[b] = ((uint8_t *)&g_titled_written_saved[k])[b];
                                  }
                              }
                              arena<uint8_t>(0x48)[0] = *(volatile uint8_t *)(uintptr_t)0x13ce4c8;   // (the game globals some copies set: compared)
                              arena<uint8_t>(0x48)[1] = *(volatile uint8_t *)(uintptr_t)0x13ce594;
                              *(volatile uint8_t *)(uintptr_t)0x13ce4c8 = g_titled_flags[0], *(volatile uint8_t *)(uintptr_t)0x13ce594 = g_titled_flags[1];
                              return Result{(uint32_t)((uintptr_t)made - (uintptr_t)arena<void>(0))};
                          }};
    titled_construct.prepare = [](const rewrite::Entry *e) {
        g_titled_flags[0] = *(uint8_t *)(uintptr_t)0x13ce4c8, g_titled_flags[1] = *(uint8_t *)(uintptr_t)0x13ce594;
        titled_tables_ready();
        static void *const bases[7] = {nullptr, nullptr, (void *)stub_titled_base2, (void *)stub_titled_base3, (void *)stub_titled_base4, (void *)stub_titled_base5, (void *)stub_titled_base6};
        static void *const built[9] = {(void *)stub_built0, (void *)stub_built1, (void *)stub_built2, (void *)stub_built3, (void *)stub_built4, (void *)stub_built5,
                                       (void *)stub_built6, (void *)stub_built7, (void *)stub_built8};
        uint32_t base = 0, base_args = 2;
        g_titled_parent = 1, g_titled_text = 2;
        for (int k = 0; titled_info[k]; k += 7)
            if (titled_info[k] == e->at)
                g_titled_title = titled_info[k + 1], g_titled_args = titled_info[k + 2], base = titled_info[k + 3], base_args = titled_info[k + 4],
                g_titled_parent = titled_info[k + 5], g_titled_text = titled_info[k + 6];
        g_titled_nullable = 0, g_titled_byte_flags = 0, g_titled_iface_mask = 0, g_titled_at = e->at;
        g_titled_holder_mask = 0;
        for (int k = 0; titled_holder_args[k]; k += 2)
            if (titled_holder_args[k] == e->at) g_titled_holder_mask = titled_holder_args[k + 1];
        g_child_holder_count = 0;
        for (int k = 0; titled_child_holders[k]; k += 2)
            if (titled_child_holders[k] == e->at && g_child_holder_count < 8) g_child_holders[g_child_holder_count++] = titled_child_holders[k + 1];
        g_child_array_count = 0;
        for (int k = 0; titled_child_arrays[k]; k += 4)
            if (titled_child_arrays[k] == e->at && g_child_array_count < 4)
                g_child_arrays[g_child_array_count][0] = titled_child_arrays[k + 1], g_child_arrays[g_child_array_count][2] = titled_child_arrays[k + 3],
                g_child_arrays[g_child_array_count++][1] = titled_child_arrays[k + 2];
        for (int k = 0; titled_iface_args[k]; k += 2)
            if (titled_iface_args[k] == e->at) g_titled_iface_mask = titled_iface_args[k + 1];
        g_titled_vcall_mask = 0;
        for (int k = 0; titled_vcall_args[k]; k += 2)
            if (titled_vcall_args[k] == e->at) g_titled_vcall_mask = titled_vcall_args[k + 1];
        for (int k = 0; titled_byte_flags[k]; k += 2)
            if (titled_byte_flags[k] == e->at) g_titled_byte_flags = titled_byte_flags[k + 1];
        for (int k = 0; titled_nullable[k]; k += 2)
            if (titled_nullable[k] == e->at) g_titled_nullable = titled_nullable[k + 1];
        std::vector<std::pair<uint32_t, void *>> list = {{0xbcdfc0, (void *)stub_plain_alloc}, {0xbcf850, (void *)stub_name_text}, {0xbd0490, (void *)stub_wide_empty_shared},
                                                         {0x403be0, (void *)stub_wide_assign_c}, {0x403bb0, (void *)stub_wide_assign}, {0x41c740, (void *)stub_wide_copy},
                                                         {0x805750, (void *)stub_null_handle}, {0x805700, (void *)stub_member_handle_of}, {base, bases[base_args]},
                                                         {0x4040f0, (void *)stub_named_text_ref}, {0xbd1880, (void *)stub_text_ref_free}, {0x6a39d0, (void *)stub_text_for_child},
                                                         {0x4b7940, (void *)stub_text_assign2},
                                                         {0x6b1640, (void *)titled_layout}, {0xbeca40, (void *)stub_name_make},
                                                         {0xbecc10, (void *)stub_name_give}, {0xbec5e0, (void *)stub_name_free},
                                                         {0x41cb90, (void *)stub_named_text_ref}, {0xbcf7d0, (void *)stub_class_wide_from},
                                                         {0x431360, (void *)stub_missing_symbol}, {0x49b9f0, (void *)stub_missing_symbol},
                                                         {0x696130, (void *)stub_missing_symbol}, {0x771570, (void *)stub_missing_symbol}};
        static void *const built_rect[5] = {(void *)stub_built0, (void *)stub_built_rect1, (void *)stub_built_rect2, (void *)stub_built_rect3, (void *)stub_built_rect4};
        static void *const built_edx[4] = {(void *)stub_built_edx0, (void *)stub_built_edx1, (void *)stub_built_edx2, (void *)stub_built_edx3};
        g_titled_single_global = g_titled_single_fn = 0;
        for (int q = 0; titled_singletons[q]; q += 3)
            if (titled_singletons[q] == e->at) g_titled_single_global = titled_singletons[q + 1], g_titled_single_fn = titled_singletons[q + 2];
        auto is_holder_callee = [](uint32_t fn) { for (int q = 0; titled_holder_callees[q]; q++) if (titled_holder_callees[q] == fn) return true; return false; };
        static void *const holder_stubs[4] = {(void *)stub_holder0, (void *)stub_holder1, (void *)stub_holder2, (void *)stub_holder3};
        auto is_value_callee = [](uint32_t fn) { for (int q = 0; titled_value_callees[q]; q++) if (titled_value_callees[q] == fn) return true; return false; };
        static void *const fast_stubs[5] = {(void *)stub_fast0, (void *)stub_fast1, (void *)stub_fast2, (void *)stub_fast3, (void *)stub_fast4};
        static void *const plain_stubs[5] = {(void *)stub_plain0, (void *)stub_plain1, (void *)stub_plain2, (void *)stub_plain3, (void *)stub_plain4};
        for (int k = 0; titled_callees[k]; k += 2)   // (never the copy being checked, which other classes make as a member)
            if (titled_callees[k] != e->at)
                list.push_back({titled_callees[k], is_holder_callee(titled_callees[k]) && (titled_callees[k + 1] & 0xf00) == 0 && titled_callees[k + 1] < 4 ?
                                                  holder_stubs[titled_callees[k + 1]] :
                                                  titled_callees[k] == g_titled_single_fn && (titled_callees[k + 1] & 0xf00) == 0 && titled_callees[k + 1] < 4 ?
                                                  (titled_callees[k + 1] == 0 ? (void *)stub_single0 : titled_callees[k + 1] == 1 ? (void *)stub_single1 : titled_callees[k + 1] == 2 ? (void *)stub_single2 : (void *)stub_single3) :
                                                  titled_callees[k + 1] & 0x800 ? fast_stubs[titled_callees[k + 1] & 0xff] : titled_callees[k + 1] & 0x400 ? (is_value_callee(titled_callees[k]) && (titled_callees[k + 1] & 0xff) == 0 ? (void *)stub_plain_value0 : plain_stubs[titled_callees[k + 1] & 0xff]) : titled_callees[k + 1] & 0x200 ? built_rect[titled_callees[k + 1] & 0xff]
                                                  : titled_callees[k + 1] & 0x100 ? built_edx[titled_callees[k + 1] & 0xff] : built[titled_callees[k + 1]]});
        static void *const fast_temp_stubs[4] = {(void *)stub_fast_temp0, (void *)stub_fast_temp1, (void *)stub_fast_temp2, (void *)stub_fast_temp3};
        for (int k = 0; titled_fast_temps[k]; k += 2) list.push_back({titled_fast_temps[k], fast_temp_stubs[titled_fast_temps[k + 1] & 3]});
        static void *const out_stubs[5] = {(void *)stub_text_out1, (void *)stub_text_out2, (void *)stub_text_out3, (void *)stub_text_out4, (void *)stub_text_out5};
        for (int k = 0; titled_out_callees[k]; k += 2)
            if (titled_out_callees[k] != e->at) list.push_back({titled_out_callees[k], out_stubs[titled_out_callees[k + 1] - 1]});
        list.push_back({0xbd01c0, (void *)stub_titled_text_copy});
        for (int k = 0; titled_text_refs_by_text[k]; k++) list.push_back({titled_text_refs_by_text[k], (void *)stub_named_text_ref_text});
        if (e->at == 0x9a0b90)   // (its hand-written loop: the count small, the texts made by the name source new each time)
            for (auto &x : list)
                if (x.first == 0x53c790) x.second = (void *)stub_titled_count8;
        if (e->at == 0x9a0b90) list.push_back({0x53dd80, (void *)stub_titled_text_out});
        if (e->at == 0x6e1e70) list.push_back({0x6e1e40, (void *)stub_titled_text_out});
        if (e->at == 0x61da30)   // (its hand-written loop's array: sized for real)
            for (auto &x : list)
                if (x.first == 0x423de0) x.second = (void *)stub_titled_sized;   // (its hand-written loop's texts: new each time)
        stubs_for_now(list);
        g_built_other = true;
    };
    titled_construct.finish = [](const rewrite::Entry *) { unstub_many(); g_built_other = false; };
    quick(titled_construct);
    // Class::Construct: a description (given or the class's default, a game global set in the call) registered or not,
    // its object info through slot 1 (as the aimed walk's); every function called is a stand-in. Handle::For (0x805700)
    // gives a handle of its own (0x600, with a list), so reference members to an object are told from empty ones.
    Test class_construct{0x549780,
                         [=](Rng &r) {
                             fresh(r);
                             *arena<uint32_t>(ALLOC_NEXT) = 0;
                             for (int k = 0; k < 0x2000 / 4; k++) arena<uint32_t>(0x2000)[k] = r.next();   // (new memory holds what was there)
                             arena<uint32_t>(0x8b48)[0] = r.range(0, 1);   // (the game object of class_singletons there already, or not)
                             arena<uint32_t>(0x8b4c)[0] = r.range(0, 1);   // (and an object in the instance global already, or not)
                             arena<uint32_t>(0x8b50)[0] = r.next();         // (what 0x6C3BE0's list holds)
                             arena<uint32_t>(0x8b54)[0] = r.range(0, 3);    // (how many nodes follow the first in a class_chains chain)
                             arena<uint32_t>(0x8b60)[0] = r.range(0, 1) ? r.range(1, 5) : 0;   // (a class_counts count)
                             arena<uint32_t>(0x8b64)[0] = r.next();   // (the counts of class_text_args texts)
                             arena<uint32_t>(0x8b68)[0] = r.next();   // (0x47D160's part)
                             arena<uint32_t>(0x8b6c)[0] = r.range(0, 6);   // (0x1106D0C's size, and its items: some empty)
                             for (int k = 0; k < 8; k++) arena<uint32_t>(0x10400)[k] = r.range(0, 2) ? r.next() | 1 : 0;
                             arena<uint32_t>(0x8b58)[0] = r.range(0, 20);   // (a class_owner_arrays array's room, and its count: full half the time)
                             arena<uint32_t>(0x8b58)[1] = r.range(0, 1) ? arena<uint32_t>(0x8b58)[0] : r.range(0, arena<uint32_t>(0x8b58)[0]);
                             for (int k = 0; k < 0x80 / 4; k++) arena<uint32_t>(0x9600)[k] = r.next();
                             for (int k = 0; k < 0x100 / 4; k++) arena<uint32_t>(0x9300)[k] = r.next();   // (0x4BF730's items)
                             for (int k = 0; k < 0x100 / 4; k++) arena<uint32_t>(0x9000)[k] = r.next();   // (and its contents when there)
                             static void *vtable[2] = {nullptr, (void *)stub_object_info};
                             uint8_t *description = arena<uint8_t>(0x4000);
                             *(void **)description = vtable;
                             description[4] = (uint8_t)(r.range(0, 2) ? 0 : 1);
                             *(uint32_t *)(description + 0x40) = r.range(0, 3) ? 1 : 0;
                             uint32_t *info = arena<uint32_t>(0x8000);
                             info[0x14 / 4] = (uint32_t)(uintptr_t)arena<void>(0x8100);
                             info[0x18 / 4] = r.range(0, 4);
                             for (int k = 0; k < 4; k++) arena<uint32_t>(0x8100)[k] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x8200 + k * 0x20) : 0;
                             for (int k = 0; k < 4; k++) arena<int32_t>(0x8200 + k * 0x20)[4] = r.range(0, 3) ? 0x20 : -(int32_t)(uintptr_t)description;
                             uint32_t *handle = arena<uint32_t>(0x700);   // the empty handle, with a list
                             handle[0x20 / 4] = (uint32_t)(uintptr_t)arena<void>(0x780);
                             arena<uint32_t>(0x780)[0] = (uint32_t)(uintptr_t)arena<void>(0x790);
                             arena<uint32_t>(0x790)[1] = (uint32_t)(uintptr_t)arena<void>(0x780);
                             uint32_t *object_handle = arena<uint32_t>(0x600);   // the handle Handle::For gives, with a list
                             object_handle[0x20 / 4] = (uint32_t)(uintptr_t)arena<void>(0x680);
                             arena<uint32_t>(0x680)[0] = (uint32_t)(uintptr_t)arena<void>(0x690);
                             arena<uint32_t>(0x690)[1] = (uint32_t)(uintptr_t)arena<void>(0x680);
                             for (int k = 0; k < 0x40; k++) arena<uint32_t>(0x8900)[k] = r.next() | 1;   // the array members' items: an end marked shows
                             arena<uint32_t>(0x8a00)[0x78 / 4] = (uint32_t)(uintptr_t)arena<void>(0x8b00);
                             arena<uint32_t>(0x8b00)[0] = r.next();
                             arena<uint32_t>(0x8b00)[1] = r.next();
                             for (int k = 0; k < 4; k++) arena<uint32_t>(0x8b10)[k] = r.next() & 0x3fffffff;   // the weather defaults, for this case
                             arena<uint32_t>(0x8b20)[0] = r.range(0, 1) ? r.next() : 0;   // the water texts table: there, or not yet
                             arena<uint32_t>(0x8b20)[1] = r.next();                       // and what making it gives
                             arena<uint32_t>(0x8b20)[2] = 0;                              // the tries to make it
                             arena<uint32_t>(0x8b30)[0] = r.next();                       // the lookups' answers
                             arena<uint32_t>(0x8b34)[0] = 0;                              // the water lists made so far
                             arena<float>(0x8b38)[0] = r.range(-4000, 4000) / 7.0f;       // two game values multiplied and rounded (0x10EEACC, 0x10EEAD0)
                             arena<float>(0x8b38)[1] = r.range(-4000, 4000) / 3.0f;
                             for (int k = 0; k < 0x100; k++) arena<uint32_t>(0x200)[k] = r.next();   // the rest of a big object (to 0x600)
                             arena<uint32_t>(0x8800)[0] = 100;
                             arena<uint32_t>(0x10)[0] = r.range(0, 1) ? (uint32_t)(uintptr_t)description : 0;
                             arena<uint32_t>(0x10)[1] = r.next();
                             arena<uint32_t>(0x10)[2] = r.next();
                             arena<uint32_t>(0x10)[3] = r.range(0, 4);
                             for (int k = 0; k < 4; k++) arena<uint32_t>(0x40)[k] = r.next();   // arguments 5 to 7
                             arena<uint32_t>(0x60)[0] = r.next();   // the pair a two-argument constructor's second argument points to
                             arena<uint32_t>(0x60)[1] = r.next();
                             arena<uint32_t>(CALL_LOG)[0] = 0;
                         },
                         [](void *fn) {
                             for (int k = 0; class_read_globals[k] && k < 128; k++) {   // (the globals read as values: the case's, until the settings below)
                                 g_class_read_saved[k] = *(volatile uint32_t *)(uintptr_t)class_read_globals[k];
                                 const uint32_t v = arena<uint32_t>(0x10)[1] * 0x9e3779b1u + (uint32_t)k * 0x85ebca6bu;
                                 *(volatile uint32_t *)(uintptr_t)class_read_globals[k] = v % 5 == 0 ? 0 : v;
                             }
                             *(void **)g_class_description = arena<void>(0x4000);
                             for (int k = 0; k < 4; k++) ((uint32_t *)0x10f1cf8)[k] = arena<uint32_t>(0x8b10)[k];
                             *(uint32_t *)0x133a274 = arena<uint32_t>(0x8b20)[0];
                             *(uint32_t *)0x133a270 = 0;   // the water manager's global (set by its constructor)
                             ((uint32_t *)0x10eeacc)[0] = arena<uint32_t>(0x8b38)[0], ((uint32_t *)0x10eeacc)[1] = arena<uint32_t>(0x8b38)[1];
                             arena<uint32_t>(0x9100)[0] = (uint32_t)(uintptr_t)g_class_old_table;
                             if (g_class_instance) *(uint32_t *)g_class_instance = arena<uint32_t>(0x8b4c)[0] ? (uint32_t)(uintptr_t)arena<void>(0x9100) : 0;
                             // a game list for the end's globals: what they point to (0x7C0) is a list head, and at +0x14 holds another (0x7E0)
                             uint32_t *list = arena<uint32_t>(0x7c0), *inner = arena<uint32_t>(0x7e0);
                             list[0] = (uint32_t)(uintptr_t)arena<void>(0x7b0), arena<uint32_t>(0x7b0)[1] = (uint32_t)(uintptr_t)list;
                             list[1] = (uint32_t)(uintptr_t)arena<void>(0x7b0), arena<uint32_t>(0x7b0)[0] = (uint32_t)(uintptr_t)list;   // (circular both ways)
                             list[5] = (uint32_t)(uintptr_t)inner;
                             list[4] = (uint32_t)(uintptr_t)inner;   // (some classes keep theirs at +0x10)
                             inner[0] = (uint32_t)(uintptr_t)arena<void>(0x7f0), arena<uint32_t>(0x7f0)[1] = (uint32_t)(uintptr_t)inner;
                             inner[1] = (uint32_t)(uintptr_t)arena<void>(0x7f0), arena<uint32_t>(0x7f0)[0] = (uint32_t)(uintptr_t)inner;
                             for (uintptr_t g : g_class_tail_globals)
                                 if (g) *(void **)g = list;
                             *(int32_t *)0x130ac18 = arena<int32_t>(0x10)[3];
                             g_null_handle = arena<void>(0x700);
                             uint32_t *a = arena<uint32_t>(0x10);
                             uint32_t *x = arena<uint32_t>(0x40);
                             // (the object at 0xA000, clear of the handles and lists above: the biggest classes reach +0x1000)
                             for (int k = 0; k < 0x1000 / 4; k++) arena<uint32_t>(0xa000)[k] = arena<uint32_t>(0x100)[k & 0xff];
                             g_class_random_n = 0;
                             for (uint32_t n = 2; n <= 7; n++)
                                 if (g_class_text_args >> n & 1) {
                                     uint32_t *holder = arena<uint32_t>(0x9800 + 0x20 * n), *text = arena<uint32_t>(0x9810 + 0x20 * n);
                                     holder[0] = (uint32_t)(uintptr_t)text;
                                     text[0] = 1 + arena<uint32_t>(0x8b64)[0] % 5;
                                     (n >= 5 ? x[n - 5] : a[n - 1]) = (uint32_t)(uintptr_t)holder;
                                 } else if (g_class_text_args >> (n + 8) & 1) {   // (the argument a text itself)
                                     uint32_t *text = arena<uint32_t>(0x9810 + 0x20 * n);
                                     text[0] = arena<uint32_t>(0x8b64)[0] % 3;   // (0 now and then: raised and let go, it is freed)
                                     text[1] = 0x7000 + n, text[2] = 0x6000 + n, text[3] = 0x5000 + n;   // (its length and characters: told apart)
                                     (n >= 5 ? x[n - 5] : a[n - 1]) = (uint32_t)(uintptr_t)text;
                                 }
                             if (g_class_count) *(volatile uint32_t *)g_class_count = arena<uint32_t>(0x8b60)[0];
                             if (g_class_slot_array) {
                                 ((volatile uint32_t *)0x1106d0c)[0] = (uint32_t)(uintptr_t)arena<void>(0x10400);
                                 ((volatile uint32_t *)0x1106d0c)[1] = arena<uint32_t>(0x8b6c)[0];
                             }
                             if (g_class_owner) {
                                 arena<uint32_t>(0x9500)[0x18 / 4] = arena<uint32_t>(0x8b58)[0];
                                 arena<uint32_t>(0x9500)[0x1c / 4] = arena<uint32_t>(0x8b58)[1];
                                 arena<uint32_t>(0x9500)[0x20 / 4] = (uint32_t)(uintptr_t)arena<void>(0x9600);
                                 *(volatile uint32_t *)g_class_owner = (uint32_t)(uintptr_t)arena<void>(0x9500);
                             }
                             if (g_class_chain) {
                                 const uint32_t more = arena<uint32_t>(0x8b54)[0];
                                 for (uint32_t q = 0; q <= more; q++) arena<uint32_t>(0x9400 + 0x10 * q)[0] = q < more ? (uint32_t)(uintptr_t)arena<void>(0x9410 + 0x10 * q) : 0;
                                 *(volatile uint32_t *)g_class_chain = (uint32_t)(uintptr_t)arena<void>(0x9400);
                                 *(volatile uint32_t *)g_class_chain_flag = 0x5a5a0000 | more;
                             }
                             if (g_class_single) *(volatile uint32_t *)g_class_single = arena<uint32_t>(0x8b48)[0] ? (uint32_t)(uintptr_t)arena<void>(0x9000) : 0;
                             void *self = arena<void>(0xa000), *d = (void *)(uintptr_t)a[0], *made = nullptr;
                             typedef uint32_t U;
                             switch (g_class_args) {
                             case 2: made = ((void *(__thiscall *)(void *, void *, void *))fn)(self, d, arena<void>(0x60)); break;
                             case 3: made = ((void *(__thiscall *)(void *, void *, U, U))fn)(self, d, a[1], a[2]); break;
                             case 4: made = ((void *(__thiscall *)(void *, void *, U, U, U))fn)(self, d, a[1], a[2], a[3]); break;
                             case 5: made = ((void *(__thiscall *)(void *, void *, U, U, U, U))fn)(self, d, a[1], a[2], a[3], x[0]); break;
                             case 6: made = ((void *(__thiscall *)(void *, void *, U, U, U, U, U))fn)(self, d, a[1], a[2], a[3], x[0], x[1]); break;
                             default: made = ((void *(__thiscall *)(void *, void *, U, U, U, U, U, U))fn)(self, d, a[1], a[2], a[3], x[0], x[1], x[2]); break;
                             }
                             if (g_class_chain) arena<uint32_t>(0x84)[0] = *(volatile uint32_t *)g_class_chain_flag;
                             if (g_class_count) arena<uint32_t>(0x88)[0] = *(volatile uint32_t *)g_class_count;
                             if (g_class_slot_array)   // (the array after: its items as an arena offset, and its size)
                                 arena<uint32_t>(0x8c)[0] = ((volatile uint32_t *)0x1106d0c)[0] - (uint32_t)(uintptr_t)arena<void>(0),
                                 arena<uint32_t>(0x90)[0] = ((volatile uint32_t *)0x1106d0c)[1];   // (the flag, so the memory check sees it)
                             if (g_class_instance)   // what the instance global was given, as an arena offset, so the memory check sees it
                                 arena<uint32_t>(0x70)[0] = *(uint32_t *)g_class_instance - (uint32_t)(uintptr_t)arena<void>(0);
                             for (int i = 0; i < 4; i++)   // what the end left in them (a count goes up by one)
                                 if (g_class_tail_globals[i]) arena<uint32_t>(0x74)[i] = *(uint32_t *)g_class_tail_globals[i] - (uint32_t)(uintptr_t)arena<void>(0);
                             for (int k = 0; class_read_globals[k] && k < 128; k++) *(volatile uint32_t *)(uintptr_t)class_read_globals[k] = g_class_read_saved[k];
                             return Result{(uint32_t)((uintptr_t)made - (uintptr_t)arena<void>(0))};
                         }};
    class_construct.prepare = [](const rewrite::Entry *e) {
        // The copy's default description: the game global it loads before building the base (mov r32, [global]); and
        // how many arguments it takes, from its ret N after the exception frame is put back (mov fs:[0], ecx).
        const uint8_t *code = (const uint8_t *)e->at;
        memcpy(g_weather_defaults, (void *)0x10f1cf8, sizeof g_weather_defaults);
        g_water_table = *(uint32_t *)0x133a274;
        g_resource_manager = *(uint32_t *)0x13d0e04;
        g_water_manager = *(uint32_t *)0x133a270;
        memcpy(g_scale_values, (void *)0x10eeacc, sizeof g_scale_values);
        g_class_description = 0;
        uintptr_t base = 0x800a80;   // (Class::Construct, or the parent class's constructor the copy is built on)
        bool base4 = false;
        for (int k = 0; class_bases[k]; k += 3)
            if (class_bases[k] == e->at) base = class_bases[k + 1], base4 = class_bases[k + 2] == 4;
        for (int k = 0; k < 0x80 && !(code[k] == 0xe8 && (uintptr_t)(code + k + 5) + *(const int32_t *)(code + k + 1) == base); k++) {
            uint32_t at = code[k] == 0xa1 ? *(const uint32_t *)(code + k + 1) : code[k] == 0x8b && (code[k + 1] & 0xc7) == 0x05 ? *(const uint32_t *)(code + k + 2) : 0;
            if (at >= 0x1000000 && at < 0x1400000 && code[k - 1] != 0x64) g_class_description = at;
        }
        g_class_args = 3;
        g_class_instance = 0;
        g_class_single = 0;
        g_class_chain = g_class_chain_flag = 0;
        g_class_owner = 0;
        g_class_count = 0;
        g_class_slot_array = e->at == 0x44f240;
        if (g_class_slot_array) g_class_slot_saved[0] = ((volatile uint32_t *)0x1106d0c)[0], g_class_slot_saved[1] = ((volatile uint32_t *)0x1106d0c)[1];
        g_class_text_args = 0;
        for (int k = 0; class_instances[k]; k += 2)
            if (class_instances[k] == e->at) g_class_instance = class_instances[k + 1];
        for (int k = 0; class_text_args[k]; k += 2)
            if (class_text_args[k] == e->at) g_class_text_args = class_text_args[k + 1];
        for (int k = 0; class_counts[k]; k += 2)
            if (class_counts[k] == e->at) g_class_count = class_counts[k + 1], g_class_count_saved = *(volatile uint32_t *)g_class_count;
        for (int k = 0; class_owner_arrays[k]; k += 2)
            if (class_owner_arrays[k] == e->at) g_class_owner = class_owner_arrays[k + 1], g_class_owner_saved = *(volatile uint32_t *)g_class_owner;
        for (int k = 0; class_chains[k]; k += 3)
            if (class_chains[k] == e->at) {
                g_class_chain = class_chains[k + 1], g_class_chain_flag = class_chains[k + 2];
                g_class_chain_saved[0] = *(volatile uint32_t *)g_class_chain, g_class_chain_saved[1] = *(volatile uint32_t *)g_class_chain_flag;
            }
        uint32_t single_fn = 0;
        for (int k = 0; class_singletons[k]; k += 3)
            if (class_singletons[k] == e->at) g_class_single = class_singletons[k + 1], single_fn = class_singletons[k + 2];
        if (g_class_single) g_class_single_saved = *(volatile uint32_t *)g_class_single;
        memset(g_class_tail_globals, 0, sizeof g_class_tail_globals);
        for (int k = 0x80; k < 0x1000; k++)
            if (code[k] == 0x64 && code[k + 1] == 0x89 && code[k + 2] == 0x0d && !*(const uint32_t *)(code + k + 3)) {
                for (int m = k + 7; m < k + 24; m++)
                    if (code[m] == 0xc2 && code[m + 2] == 0) {
                        g_class_args = code[m + 1] / 4;
                        // the instance global: a store of a register to a game global (mov [g], esi/edi/ebp/ebx) just before
                        for (int q = k - 0x20; q < m; q++)
                            if (code[q] == 0x89 && (code[q + 1] & 0xc7) == 0x05 && (code[q + 1] >> 3 & 7) >= 3 && *(const uint32_t *)(code + q + 2) >= 0x1000000 &&
                                *(const uint32_t *)(code + q + 2) < 0x1400000)
                                g_class_instance = *(const uint32_t *)(code + q + 2);
                        // the globals the end reads after the last field is added (mov eax, [g] / mov r32, [g]), the default description aside
                        int last = 0, used = 0;
                        for (int q = 0; q < k; q++)
                            if (code[q] == 0xe8 && (uintptr_t)(code + q + 5) + *(const int32_t *)(code + q + 1) == 0x8087e0) last = q;
                        for (int q = last; q < k && used < 4; q++) {
                            uint32_t g = code[q] == 0xa1 ? *(const uint32_t *)(code + q + 1) : code[q] == 0x8b && (code[q + 1] & 0xc7) == 0x05 ? *(const uint32_t *)(code + q + 2) : 0;
                            if (g >= 0x1000000 && g < 0x1400000 && g != g_class_description && g != g_class_instance && g != 0x133a274 && g != 0x13d0e04)   // (not the
                                g_class_tail_globals[used++] = g;   // water texts table or the resource manager: set up above)
                        }
                        break;
                    }
                break;
            }
        if (!g_class_description) g_class_description = (uintptr_t)arena<void>(0x3ff0);   // not found: the test then fails
        std::vector<std::pair<uintptr_t, void *>> list = {
            {base, base4 ? (void *)stub_class_base4 : (void *)stub_class_base}, {0x475180, (void *)stub_class_random}, {0x8087e0, (void *)stub_field_add}, {0xbcdfc0, (void *)stub_plain_alloc}, {0xbd0490, (void *)stub_wide_empty_shared},
            {0xbeca40, (void *)stub_name_make}, {0xbecc10, (void *)stub_name_give}, {0xbec5e0, (void *)stub_name_free}, {0x8088c0, (void *)stub_built4},
            {0x805700, (void *)stub_member_handle_of}, {0x808d90, (void *)stub_struct_total},
            {0xcd2b30, (void *)stub_list_member}, {0x441030, (void *)stub_member_built}, {0x981ee0, (void *)stub_member_built1},
            {0x84ae30, (void *)stub_member_built}, {0x611a80, (void *)stub_class_array},
            {0x425920, (void *)stub_class_lighting}, {0x9f9da0, (void *)stub_class_find5}, {0x7c8870, (void *)stub_class_info_text},
            {0xbd9ad0, (void *)stub_class_reference}, {0x560d20, (void *)stub_class_named_text}, {0xbd99e0, (void *)stub_class_unreference},
            {0x9e87e0, (void *)stub_class_start}, {0x48c9c0, (void *)stub_class_from_pair}, {0xbd0520, (void *)stub_wide_empty_other},
            {0x707530, (void *)stub_class_after_figures}, {0x6a3b60, (void *)stub_class_record_pairs}, {0x940ea0, (void *)stub_class_water_table},
            {0x403f30, (void *)stub_class_named_in}, {0xbcfb10, (void *)stub_class_text_c}, {0xbdf160, (void *)stub_class_lookup},
            {0xbdf1a0, (void *)stub_class_lookup2}, {0xbdf180, (void *)stub_class_lookup3}, {0xbd14e0, (void *)stub_class_text_assign}, {0xbd1880, (void *)stub_class_text_release},
            {0xddcdb5, (void *)stub_class_sprintf}, {0x9402f0, (void *)stub_class_water_step}, {0x942510, (void *)stub_class_water_list},
            {0x63e1c0, (void *)stub_class_given}, {0x6c3be0, (void *)stub_class_name_list}, {0x4de7d0, (void *)stub_class_member2}, {0x45b600, (void *)stub_class_member2}, {0x991a50, (void *)stub_class_member1}, {0x7541a0, (void *)stub_class_member0}, {0xa0ddf0, (void *)stub_class_member1}, {0xa61160, (void *)stub_class_member2}, {0xa5f3c0, (void *)stub_class_member2},
            {0xa5fa50, (void *)stub_class_member2}, {0xa61b20, (void *)stub_class_member2}, {0x7cd220, (void *)stub_class_member2},
            {0x47d160, (void *)stub_class_part_47d160}, {0xce4770, (void *)stub_class_inner_ce4770}, {0x4bf730, (void *)stub_class_entries_sized}, {0x6f5f30, (void *)stub_class_member1}, {0xa5d540, (void *)stub_class_slots}, {0x4040f0, (void *)stub_class_named_make}, {0xbcf7d0, (void *)stub_class_wide_from}, {0x98a770, (void *)stub_class_given_reference}, {0x40f730, (void *)stub_class_reference_gone}};   // (0x704A50, which only works on the object, runs as it is)
        static void *const by_count[5] = {(void *)stub_class_member0, (void *)stub_class_member1, (void *)stub_class_member2, (void *)stub_class_member3, (void *)stub_class_member4};
        for (const uint32_t *f = class_member_fns; *f; f += 2) {
            bool have = false;
            for (auto &x : list) have |= x.first == f[0];
            if (!have) list.push_back({f[0], by_count[f[1]]});
        }
        for (int k = 0; class_value_calls[k]; k++) list.push_back({class_value_calls[k], (void *)stub_class_value});   // (the functions taking nothing)
        if (single_fn) list.push_back({single_fn, (void *)stub_class_single});
        for (int k = 0; class_ref_setters2[k]; k++) list.push_back({class_ref_setters2[k], (void *)stub_class_named_text2});
        for (int k = 0; class_ref_setters[k]; k++)   // (the member functions given a stack reference: as 0x560D20)
            if (class_ref_setters[k] != 0x560d20) list.push_back({class_ref_setters[k], (void *)stub_class_named_text});
        stubs_for_now(list);
    };
    class_construct.finish = [](const rewrite::Entry *) {
        unstub_many();
        if (g_class_single) *(volatile uint32_t *)g_class_single = g_class_single_saved;
        if (g_class_owner) *(volatile uint32_t *)g_class_owner = g_class_owner_saved;
        if (g_class_count) *(volatile uint32_t *)g_class_count = g_class_count_saved;
        if (g_class_slot_array) ((volatile uint32_t *)0x1106d0c)[0] = g_class_slot_saved[0], ((volatile uint32_t *)0x1106d0c)[1] = g_class_slot_saved[1];
        if (g_class_chain) *(volatile uint32_t *)g_class_chain = g_class_chain_saved[0], *(volatile uint32_t *)g_class_chain_flag = g_class_chain_saved[1];
        memcpy((void *)0x10f1cf8, g_weather_defaults, sizeof g_weather_defaults);
        *(uint32_t *)0x133a274 = g_water_table;
        *(uint32_t *)0x13d0e04 = g_resource_manager;
        *(uint32_t *)0x133a270 = g_water_manager;
        memcpy((void *)0x10eeacc, g_scale_values, sizeof g_scale_values);
    };
    quick(class_construct);
    // Globals::SetFixed: functions that set game globals to fixed values; the globals are compared (data), set to
    // something else first
    t.push_back({0xee6800,
                 [=](Rng &r) { fresh(r); },
                 [](void *fn) {
                     for (uint32_t k = 0; k < 0x240; k++) ((volatile uint8_t *)0x1341a00)[k] = (uint8_t)(k * 7 + 1);
                     return Result{((uint32_t(__cdecl *)())fn)()};
                 }});
    // Model::BonePose / BonePoseOf: an object (0x100) with its matrices of any floats, four bone matrices of any floats
    // (0x2000), the bone number 0-3 (through +0x84, or given); the result written at 0x3000
    auto bone_setup = [=](Rng &r) {
        fresh(r);
        for (int i = 0; i < 0x20; i++) arena<float>(0x100)[i] = r.any_float();
        for (int i = 0; i < 64; i++) arena<float>(0x2000)[i] = r.any_float();
        *(void **)arena<uint8_t>(0x100 + 0x84) = arena<void>(0x600);
        *(void **)arena<uint8_t>(0x100 + 0xb0) = arena<void>(0x700);
        *arena<uint32_t>(0x600) = r.range(0, 3);
        *(void **)arena<uint8_t>(0x700 + 0x18) = arena<void>(0x2000);
        *(void **)arena<uint8_t>(0x700 + 0x20) = arena<void>(0x2000);
        *arena<uint32_t>(0x10) = r.range(0, 3);
    };
    Test bone_pose{0x7bf0c0, bone_setup, [](void *fn) {
        void *made = ((void *(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x3000), arena<void>(0x700));
        return Result{(uint32_t)((uintptr_t)made - (uintptr_t)arena<void>(0))};
    }};
    quick(bone_pose);
    Test bone_pose_of{0xb82090, bone_setup, [](void *fn) {
        void *made = ((void *(__thiscall *)(void *, void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x3000), *arena<uint32_t>(0x10));
        return Result{(uint32_t)((uintptr_t)made - (uintptr_t)arena<void>(0))};
    }};
    quick(bone_pose_of);
    // Matrix::Multiply / MultiplyBy: matrices of any floats (0x100, 0x200), the result at 0x300 (or in place); also
    // with the result being one of the inputs
    Test matrix_multiply{0x42ee50,
                         [=](Rng &r) {
                             fresh(r);
                             for (int i = 0; i < 32; i++) arena<float>(0x100)[i] = r.any_float();
                             for (int i = 0; i < 16; i++) arena<float>(0x200)[i] = r.any_float();
                             *arena<uint32_t>(0x10) = r.range(0, 2);
                         },
                         [](void *fn) {
                             uint32_t w = *arena<uint32_t>(0x10);
                             void *out = w == 0 ? arena<void>(0x300) : w == 1 ? arena<void>(0x100) : arena<void>(0x200);
                             void *made = ((void *(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x100), out, arena<void>(0x200));
                             return Result{(uint32_t)((uintptr_t)made - (uintptr_t)arena<void>(0))};
                         }};
    quick(matrix_multiply);
    Test matrix_multiply_by{0x505c00,
                            [=](Rng &r) {
                                fresh(r);
                                for (int i = 0; i < 16; i++) arena<float>(0x100)[i] = r.any_float();
                                for (int i = 0; i < 16; i++) arena<float>(0x200)[i] = r.any_float();
                                *arena<uint32_t>(0x10) = r.range(0, 1);
                            },
                            [](void *fn) {
                                void *b = *arena<uint32_t>(0x10) ? arena<void>(0x100) : arena<void>(0x200);
                                void *made = ((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), b);
                                return Result{(uint32_t)((uintptr_t)made - (uintptr_t)arena<void>(0))};
                            }};
    quick(matrix_multiply_by);
    // Matrix::Invert4: a matrix of any floats, or (half the cases) of ordinary values in -4..4 so the results are
    // finite and the rounding of each step shows
    Test matrix_invert4{0xc1fc30,
                        [=](Rng &r) {
                            fresh(r);
                            bool plain = r.range(0, 1);
                            for (int i = 0; i < 16; i++) arena<float>(0x100)[i] = plain ? (float)((int32_t)(r.next() % 80000) - 40000) / 10000.0f : r.any_float();
                        },
                        [](void *fn) {
                            void *made = ((void *(__thiscall *)(void *))fn)(arena<void>(0x100));
                            return Result{(uint32_t)((uintptr_t)made - (uintptr_t)arena<void>(0))};
                        }};
    quick(matrix_invert4);
    // Values::Map: the argument at each range's start, one either side, or any value; other registers random
    static uint32_t value_where;
    static const uint32_t *value_starts;
    static uint32_t value_count;
    Test value_map{0x5d1530,
                   [=](Rng &r) {
                       uint32_t pick = value_starts[r.range(0, value_count - 1)] + (uint32_t)r.range(-1, 1);
                       arena<uint32_t>(0x10)[0] = r.range(0, 4) ? pick : r.next();
                       arena<uint32_t>(0x10)[1] = r.next();
                   },
                   [](void *fn) {
                       uint32_t x = arena<uint32_t>(0x10)[0], other = arena<uint32_t>(0x10)[1];
                       switch (value_where) {
                       case 0: return Result{((uint32_t(__fastcall *)(uint32_t, uint32_t))fn)(x, other)};
                       case 1: return Result{((uint32_t(__fastcall *)(uint32_t, uint32_t))fn)(other, x)};
                       case 2: return Result{((uint32_t(__cdecl *)(uint32_t))fn)(x)};
                       default: return Result{((uint32_t(__stdcall *)(uint32_t))fn)(x)};
                       }
                   }};
    value_map.data = false;
    value_map.prepare = [](const rewrite::Entry *e) {
        for (const uint32_t *v = value_info; *v; v += 3 + v[2])
            if (v[0] == e->at) value_where = v[1], value_count = v[2], value_starts = v + 3;
    };
    value_map.iterations = 6000;   // enough to reach every entry of the largest tables (130 values)
    t.push_back(value_map);
    // Record::Fields: its record (0x1000), a second record (0x5000) or values as its arguments; random bytes
    static uint32_t record_layout, record_args;
    Test record_fields{0x84f2f0,
                       [=](Rng &r) {
                           fresh(r);
                           uint32_t mode = r.range(0, 4);   // random bytes, half zeros, small values, 0 or 1, small dwords
                           auto byte = [&]() { return mode == 0 ? r.next() : mode == 1 ? (r.next() & 1 ? r.next() : 0) : mode == 2 ? r.next() & 3 : r.next() & 1; };
                           for (int i = 0; i < 0x8000; i++) arena<uint8_t>(0x1000)[i] = mode == 4 ? (i % 4 ? 0 : r.next() & 3) : (uint8_t)byte();
                           for (int i = 0; i < 3; i++) arena<uint32_t>(0x10)[i] = byte() | (mode == 0 || mode == 1 && arena<uint32_t>(0x10)[i] ? r.next() & ~0xffu : 0);
                       },
                       [](void *fn) {
                           uint8_t *self = arena<uint8_t>(0x1000), *other = arena<uint8_t>(0x5000);
                           uint32_t *v = arena<uint32_t>(0x10), got = 0;
                           uint32_t first = record_layout == 1 ? (uint32_t)(uintptr_t)other : v[0];
                           switch (record_layout * 4 + record_args) {
                           case 0: got = ((uint32_t(__thiscall *)(void *))fn)(self); break;
                           case 1: case 5: got = ((uint32_t(__thiscall *)(void *, uint32_t))fn)(self, first); break;
                           case 2: case 6: got = ((uint32_t(__thiscall *)(void *, uint32_t, uint32_t))fn)(self, first, v[1]); break;
                           case 3: case 7: got = ((uint32_t(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))fn)(self, first, v[1], v[2]); break;
                           case 9: got = ((uint32_t(__stdcall *)(void *))fn)(self); break;
                           case 10: got = ((uint32_t(__stdcall *)(void *, uint32_t))fn)(self, v[1]); break;
                           case 11: got = ((uint32_t(__stdcall *)(void *, uint32_t, uint32_t))fn)(self, v[1], v[2]); break;
                           case 13: got = ((uint32_t(__cdecl *)(void *))fn)(self); break;
                           case 14: got = ((uint32_t(__cdecl *)(void *, uint32_t))fn)(self, v[1]); break;
                           default: got = ((uint32_t(__cdecl *)(void *, uint32_t, uint32_t))fn)(self, v[1], v[2]); break;
                           }
                           return Result{got};
                       }};
    record_fields.prepare = [](const rewrite::Entry *e) {
        for (const uint32_t *v = record_info; *v; v += 3)
            if (v[0] == e->at) record_layout = v[1], record_args = v[2];
    };
    quick(record_fields);
    // Kinds::Code: small values (0-16) and any values for each argument
    t.push_back({0xbca360,
                 [=](Rng &r) {
                     static const int limit[3] = {15, 4, 7};   // a little past what each argument tells apart
                     for (int k = 0; k < 3; k++) arena<uint32_t>(0x10)[k] = r.range(0, 7) ? r.range(0, limit[k]) : r.next();
                 },
                 [](void *fn) {
                     uint32_t *a = arena<uint32_t>(0x10);
                     return Result{((uint32_t(__fastcall *)(uint32_t, uint32_t, uint32_t))fn)(a[0], a[1], a[2])};
                 }});
    t.back().data = false;
    t.back().iterations = 3000;
    // Effect::SetMatrices: an effect (0x100; its limit 0-70 or past 2^31), 256 source matrices (0x4000), numbers (0x9000,
    // any bytes) or none; n 0-70, or past 2^31 when there are numbers
    Test effect_set_matrices{0xc76f70,
                             [=](Rng &r) {
                                 fresh(r);
                                 for (int i = 0; i < 0x1000; i++) arena<uint32_t>(0x4000)[i] = r.next();
                                 for (int i = 0; i < 0x100; i++) arena<uint8_t>(0x9000)[i] = (uint8_t)r.next();
                                 *(uint32_t *)arena<uint8_t>(0x100 + 0x498) = r.range(0, 5) ? r.range(0, 70) : 0x80000000u + r.range(0, 9);
                                 bool numbers = r.range(0, 3) != 0;
                                 arena<uint32_t>(0x10)[0] = numbers;
                                 arena<uint32_t>(0x10)[1] = numbers && !r.range(0, 5) ? 0x80000000u + r.range(0, 9) : r.range(0, 70);
                             },
                             [](void *fn) {
                                 uint32_t *a = arena<uint32_t>(0x10);
                                 return Result{((uint32_t(__thiscall *)(void *, void *, uint32_t, void *))fn)(arena<void>(0x100), arena<void>(0x4000), a[1],
                                                                                                             a[0] ? arena<void>(0x9000) : nullptr)};
                             }};
    quick(effect_set_matrices);
    // Water::EdgeCut: three points, a height, a sum and a lowest value: any floats, or (half the cases) ordinary values
    // around the height so the fractions fall inside and outside 0-1 and the lowest changes or not
    Test water_edge_cut{0x95df40,
                        [=](Rng &r) {
                            fresh(r);
                            bool plain = r.range(0, 1);
                            auto value = [&]() { return plain ? (float)((int32_t)(r.next() % 2000) - 1000) / 100.0f : r.any_float(); };
                            for (int i = 0; i < 12; i++) arena<float>(0x100)[i] = value();
                            for (int i = 0; i < 4; i++) arena<float>(0x130)[i] = value();   // a fourth point
                            *arena<float>(0x140) = value();
                            *arena<float>(0x144) = value();
                            *arena<float>(0x148) = value();
                            *(void **)arena<uint8_t>(0x10) = arena<void>(0x200);
                            *(void **)arena<uint8_t>(0x14) = arena<void>(0x300);
                            *arena<uint32_t>(0x18) = r.next() % 1000;
                        },
                        [](void *fn) {
                            void *got = ((void *(__fastcall *)(void *, void *, void *, float, void *, void *, void *, void *, void *))fn)(
                                arena<void>(0x10), arena<void>(0x14), arena<void>(0x18), *arena<float>(0x148), arena<void>(0x100), arena<void>(0x110),
                                arena<void>(0x120), arena<void>(0x140), arena<void>(0x144));
                            return Result{(uint32_t)((uintptr_t)got - (uintptr_t)arena<void>(0))};
                        }};
    quick(water_edge_cut);
    Test water_edge_cut_quad = water_edge_cut;
    water_edge_cut_quad.at = 0x95e160;
    quick(water_edge_cut_quad);
    Test water_edge_cut_quad2 = water_edge_cut;
    water_edge_cut_quad2.at = 0x95e3d0;
    water_edge_cut_quad2.call = [](void *fn) {
        void *got = ((void *(__fastcall *)(void *, void *, void *, float, void *, void *, void *, void *, void *, void *))fn)(
            arena<void>(0x10), arena<void>(0x14), arena<void>(0x18), *arena<float>(0x148), arena<void>(0x100), arena<void>(0x110), arena<void>(0x120),
            arena<void>(0x130), arena<void>(0x140), arena<void>(0x144));
        return Result{(uint32_t)((uintptr_t)got - (uintptr_t)arena<void>(0))};
    };
    quick(water_edge_cut_quad2);
    Test water_edge_cut_five = water_edge_cut_quad2;
    water_edge_cut_five.at = 0x95e640;
    quick(water_edge_cut_five);
    // Matrix::TransformRows: three points (0x100), an offset (0x200) and a matrix (0x300): any floats
    Test matrix_transform_rows{0xc21770,
                               [=](Rng &r) {
                                   fresh(r);
                                   for (int i = 0; i < 12; i++) arena<float>(0x100)[i] = r.any_float();
                                   for (int i = 0; i < 4; i++) arena<float>(0x200)[i] = r.any_float();
                                   for (int i = 0; i < 16; i++) arena<float>(0x300)[i] = r.any_float();
                               },
                               [](void *fn) {
                                   return Result{((uint32_t(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x200), arena<void>(0x300))};
                               }};
    quick(matrix_transform_rows);
    // Matrix::RotateRows / RotateOut, Guest::Shares: any floats (an object at 0x100, a matrix at 0x300, out at 0x400)
    auto floats_setup = [=](Rng &r) {
        fresh(r);
        for (int i = 0; i < 0x40; i++) arena<float>(0x100)[i] = r.any_float();
        for (int i = 0; i < 16; i++) arena<float>(0x300)[i] = r.any_float();
    };
    Test matrix_rotate_rows{0xcc5900, floats_setup, [](void *fn) {
        void *got = ((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x300));
        return Result{(uint32_t)((uintptr_t)got - (uintptr_t)arena<void>(0))};
    }};
    quick(matrix_rotate_rows);
    Test matrix_rotate_out{0xc21b40, floats_setup, [](void *fn) {
        void *got = ((void *(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x400), arena<void>(0x300));
        return Result{(uint32_t)((uintptr_t)got - (uintptr_t)arena<void>(0))};
    }};
    quick(matrix_rotate_out);
    Test guest_shares{0x7713c0, floats_setup, [](void *fn) {
        ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
        return Result{};
    }};
    quick(guest_shares);
    // vector rotations: a vector (0x100), a matrix (0x300), out (0x400): any floats
    auto rel = [](void *p) { return Result{(uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0))}; };
    Test vector_rotate{0x4812c0, floats_setup, [=](void *fn) { return rel(((void *(__fastcall *)(void *, void *, void *))fn)(arena<void>(0x400), arena<void>(0x100), arena<void>(0x300))); }};
    quick(vector_rotate);
    Test matrix_rotate_vector{0x481220, floats_setup, [=](void *fn) { return rel(((void *(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x300), arena<void>(0x400), arena<void>(0x100))); }};
    quick(matrix_rotate_vector);
    Test vector_rotate_in_place{0x58a620, floats_setup, [=](void *fn) { return rel(((void *(__fastcall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x300))); }};
    quick(vector_rotate_in_place);
    Test vector4_transform{0x505e90, floats_setup, [=](void *fn) { return rel(((void *(__fastcall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x300))); }};
    quick(vector4_transform);
    auto twice_setup = [=](Rng &r) {
        floats_setup(r);
        for (int i = 0; i < 16; i++) arena<float>(0x380)[i] = r.any_float();
    };
    Test vector_transform_twice{0xcb5690, twice_setup, [=](void *fn) { return rel(((void *(__thiscall *)(void *, void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x400), arena<void>(0x380), arena<void>(0x300))); }};
    quick(vector_transform_twice);
    // Matrix::Rotate3 (registers: eax points at 0x100, ecx a matrix at 0x300): the result is edx, its top bit flipped
    // if eax or ecx didn't come back the same
    Test matrix_rotate3{0x5bd3d0, floats_setup, [](void *fn) {
        uint32_t eax = (uint32_t)(uintptr_t)arena<void>(0x100), ecx = (uint32_t)(uintptr_t)arena<void>(0x300), edx = 0;
        __asm__ volatile("call *%[f]" : "+a"(eax), "+c"(ecx), "+d"(edx) : [f] "b"(fn) : "memory", "cc");
        bool kept = eax == (uint32_t)(uintptr_t)arena<void>(0x100) && ecx == (uint32_t)(uintptr_t)arena<void>(0x300);
        return Result{edx ^ (kept ? 0 : 0x80000000u)};
    }};
    quick(matrix_rotate3);
    Test matrix_inverse_transform{0xc21610, twice_setup, [](void *fn) {
        return Result{((uint32_t(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x380), arena<void>(0x300))};
    }};
    quick(matrix_inverse_transform);
    // Water::BoxPlanes: an object (0x100, axes at +0xE0), a box (0x300), planes out (0x400), distances out (0x500)
    Test water_box_planes{0x8e6e10,
                          [=](Rng &r) {
                              fresh(r);
                              for (int i = 0; i < 0x50; i++) arena<float>(0x100)[i] = r.any_float();
                              for (int i = 0x38; i < 0x44; i++)   // signalling NaNs now and then (the copies keep them)
                                  if (r.next() % 8 == 0) arena<uint32_t>(0x100)[i] = 0x7f800001u | (r.next() & 0x803ffffeu);
                              for (int i = 0; i < 16; i++) arena<float>(0x300)[i] = r.any_float();
                              arena<float>(0x10)[0] = r.any_float();
                          },
                          [](void *fn) {
                              void *got = ((void *(__thiscall *)(void *, void *, float, void *, void *))fn)(arena<void>(0x100), arena<void>(0x300), arena<float>(0x10)[0], arena<void>(0x400), arena<void>(0x500));
                              return Result{(uint32_t)((uintptr_t)got - (uintptr_t)arena<void>(0))};
                          }};
    quick(water_box_planes);
    // Water::BoxSpan: an object (0x100, axes at +0xE0: sometimes with zeros, negative zeros or NaNs), a box (0x300,
    // 0x310), out (0x400), the inset flag
    Test water_box_span{0x8e6980,
                        [=](Rng &r) {
                            fresh(r);
                            for (int i = 0; i < 0x50; i++) arena<float>(0x100)[i] = r.range(0, 9) ? r.real(-2, 2) : r.any_float();
                            for (int i = 0x38; i < 0x44; i++) {
                                const int k = r.range(0, 9);
                                if (k == 0) arena<float>(0x100)[i] = 0.0f;
                                if (k == 1) arena<uint32_t>(0x100)[i] = 0x80000000u;
                            }
                            if (r.range(0, 3) == 0)   // a whole axis of zeros (each +0 or -0)
                                for (int i = 0; i < 3; i++) arena<uint32_t>(0x100)[0x38 + 4 * r.range(0, 2) + i] = r.range(0, 1) ? 0x80000000u : 0;
                            for (int i = 0; i < 8; i++) {
                                const int k = r.range(0, 11);
                                arena<float>(0x300)[i] = k == 0 ? r.any_float() : k == 1 ? (r.range(0, 1) ? INFINITY : -INFINITY) : k == 2 ? -0.0f : r.real(-50, 50);
                            }
                            arena<uint32_t>(0x10)[0] = r.range(0, 1);
                        },
                        [](void *fn) {
                            void *got = ((void *(__thiscall *)(void *, void *, void *, void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x300), arena<void>(0x310), arena<void>(0x400), arena<uint32_t>(0x10)[0]);
                            return Result{(uint32_t)((uintptr_t)got - (uintptr_t)arena<void>(0))};
                        }};
    quick(water_box_span);
    // Matrix::TurnBy: a matrix (0x100), a turn (0x300): any floats, sometimes small (turns under 1e-20 or near it)
    Test matrix_turn_by{0xc209e0,
                        [=](Rng &r) {
                            floats_setup(r);
                            const int mode = r.range(0, 5);
                            for (int i = 0; i < 3; i++) {
                                float &t = arena<float>(0x300)[i];
                                if (mode == 1) t = r.real(-1e-20f, 1e-20f);
                                if (mode == 2) t = r.real(-8, 8);
                            }
                        },
                        [](void *fn) {
                            void *got = ((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x300));
                            return Result{(uint32_t)((uintptr_t)got - (uintptr_t)arena<void>(0))};
                        }};
    quick(matrix_turn_by);
    // Triangle::RayHit (registers: edi, esi, edx corners at 0x100, 0x110, 0x120; ecx a direction at 0x130; origin at
    // 0x140, t out at 0x150): rays aimed at a point inside (or near) the triangle, sometimes away, parallel or any
    // floats; the result is al, its next bit set if ecx, edx, esi or edi didn't come back the same
    Test triangle_ray_hit{0xceeb80,
                          [=](Rng &r) {
                              fresh(r);
                              float *f = arena<float>(0x100);
                              for (int i = 0; i < 0x14; i++) f[i] = r.real(-10, 10);
                              const int mode = r.range(0, 10);
                              float u = r.real(-0.2f, 1.1f), v = r.real(-0.2f, 1.1f);
                              const float s = r.real(-3, 3);
                              if (mode >= 2 && mode <= 4) {   // on an edge: u = 0, v = 0 or u + v = 1 (where rounding decides)
                                  const int edge = r.range(0, 2);
                                  if (edge == 0) u = 0;
                                  if (edge == 1) v = 0;
                                  if (edge == 2) v = 1 - u;
                              }
                              if (mode >= 7) {   // small whole-number corners: exact edges
                                  for (int i = 0; i < 0x14; i++) f[i] = (float)r.range(-4, 4);
                                  u = r.range(0, 2) * 0.5f, v = r.range(0, 2) * 0.5f;
                              }
                              for (int i = 0; i < 3; i++) {
                                  const float p = f[8 + i] + u * (f[i] - f[8 + i]) + v * (f[4 + i] - f[8 + i]);
                                  if (mode < 5) f[16 + i] = p - s * f[12 + i];
                              }
                              if (mode == 5) for (int i = 0; i < 0x14; i++) if (r.range(0, 3) == 0) f[i] = r.any_float();
                              if (mode == 6) for (int i = 0; i < 3; i++) f[12 + i] = f[4 + i] - f[i];   // along an edge
                          },
                          [](void *fn) {
                              uint32_t eax, ecx = (uint32_t)(uintptr_t)arena<void>(0x130), edx = (uint32_t)(uintptr_t)arena<void>(0x120),
                                            esi = (uint32_t)(uintptr_t)arena<void>(0x110), edi = (uint32_t)(uintptr_t)arena<void>(0x100);
                              void *o = arena<void>(0x140), *t = arena<void>(0x150);
                              __asm__ volatile("pushl %[t]\n\tpushl %[o]\n\tcall *%[f]"
                                               : "=a"(eax), "+c"(ecx), "+d"(edx), "+S"(esi), "+D"(edi)
                                               : [f] "b"(fn), [o] "m"(o), [t] "m"(t)
                                               : "memory", "cc");
                              bool kept = ecx == (uint32_t)(uintptr_t)arena<void>(0x130) && edx == (uint32_t)(uintptr_t)arena<void>(0x120) &&
                                          esi == (uint32_t)(uintptr_t)arena<void>(0x110) && edi == (uint32_t)(uintptr_t)arena<void>(0x100);
                              return Result{(eax & 0xff) | (kept ? 0 : 0x100u)};
                          }};
    triangle_ray_hit.iterations = 3000;
    quick(triangle_ray_hit);
    // Grid::Sample / SampleIn: a 256 x 256 grid of cells (its own 1 MB, filled once; read only) with its scale and
    // offset; points that land anywhere on it (wrapping), on whole numbers and halves, or any floats
    static uint8_t *g_grid = nullptr;
    auto grid_setup = [](Rng &r) {
        if (!g_grid) {
            g_grid = (uint8_t *)VirtualAlloc(nullptr, 0x100020, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            Rng fill{12345};
            for (int i = 0; i < 0x40000; i++) ((float *)g_grid)[i] = fill.range(0, 50) ? fill.real(-100, 100) : fill.any_float();
        }
        float *g = (float *)(g_grid + 0x100000);
        g[0] = r.real(-2, 2), g[1] = r.real(-2, 2), g[2] = r.real(0.001f, 0.2f), g[3] = r.real(0.001f, 0.2f);
        const int mode = r.range(0, 5);
        float x = r.real(-200, 200), z = r.real(-200, 200);
        if (mode == 1) x = ((float)r.range(-1000, 1000) * 0.5f / 256.0f - g[0]) / g[2], z = ((float)r.range(-1000, 1000) * 0.5f / 256.0f - g[1]) / g[3];
        if (mode == 2) x = r.any_float(), z = r.any_float();
        arena<float>(0x10)[0] = x, arena<float>(0x10)[1] = z;
        float *box = arena<float>(0x100);   // SampleIn: the box and the point
        box[3] = r.real(-50, 0), box[4] = r.real(-50, 0), box[5] = r.real(1, 50), box[6] = r.real(1, 50);
        if (mode == 3) box[5] = box[3];
        *(void **)(arena<uint8_t>(0x100) + 0x1c) = g_grid;
        float *pt = arena<float>(0x200);
        pt[0] = mode == 2 ? r.any_float() : r.real(-60, 60), pt[1] = r.any_float(), pt[2] = r.real(-60, 60);
    };
    auto rel_result = [](void *got) { return Result{(uint32_t)((uintptr_t)got - (uintptr_t)arena<void>(0))}; };
    Test grid_sample{0x4a0090, grid_setup, [=](void *fn) {
        return rel_result(((void *(__thiscall *)(void *, void *, float, float))fn)(g_grid, arena<void>(0x400), arena<float>(0x10)[0], arena<float>(0x10)[1]));
    }};
    quick(grid_sample);
    Test grid_sample_in{0x4a0280, grid_setup, [=](void *fn) {
        return rel_result(((void *(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x400), arena<void>(0x200)));
    }};
    quick(grid_sample_in);
    // SceneryRenderer::QuadVertices: out (0x400), centre, up, side (0x100, 0x110, 0x120), a rectangle (0x130) and a
    // colour (0x140), any floats
    Test quad_vertices{0x42f320, floats_setup, [=](void *fn) {
        return rel_result(((void *(__stdcall *)(void *, void *, void *, void *, void *, void *))fn)(arena<void>(0x400), arena<void>(0x100), arena<void>(0x110),
                                                                                                   arena<void>(0x120), arena<void>(0x130), arena<void>(0x140)));
    }};
    quick(quad_vertices);
    // Firework::Quad: out (0x400), a view (0x300, its rows), a point, a size, a colour (0 to 1 mostly, sometimes any) and
    // a rectangle (0x100 on); the point sometimes near the view so its depth is under 1
    Test firework_quad{0x9f75f0,
                       [=](Rng &r) {
                           floats_setup(r);
                           float *c = arena<float>(0x110);
                           for (int i = 0; i < 4; i++) c[i] = r.range(0, 9) ? r.real(-0.1f, 1.1f) : r.any_float();
                           if (r.range(0, 2) == 0)   // near, along a unit-sized depth row: depths around 0 to 1
                               for (int i = 0; i < 3; i++) arena<float>(0x100)[i] = arena<float>(0x300)[i] + r.real(-1.5f, 1.5f), arena<float>(0x300)[4 + i] = r.real(-1, 1);
                           arena<float>(0x10)[0] = r.range(0, 5) ? r.real(-5, 5) : r.any_float();
                       },
                       [=](void *fn) {
                           return rel_result(((void *(__fastcall *)(void *, void *, void *, float, void *, void *))fn)(arena<void>(0x400), arena<void>(0x300), arena<void>(0x100),
                                                                                                                       arena<float>(0x10)[0], arena<void>(0x110), arena<void>(0x120)));
                       }};
    quick(firework_quad);
    // Matrices::Times: 1-4 matrices at 0x1000 (any floats) times one at 0x300 into 0x2000, or in place; the SSE flag
    // (0x13D2888) on or off for the call and put back after it (0 matrices only without SSE)
    Test matrices_times{0xbee8c0,
                        [=](Rng &r) {
                            floats_setup(r);
                            for (int i = 0; i < 64; i++) arena<float>(0x1000)[i] = r.range(0, 9) ? r.real(-100, 100) : r.any_float();
                            const uint32_t sse = r.range(0, 1);
                            arena<uint32_t>(0x10)[0] = sse;
                            arena<uint32_t>(0x10)[1] = r.range(sse ? 1 : 0, 4);
                            arena<uint32_t>(0x10)[2] = r.range(0, 3) ? 0x2000 : 0x1000;
                        },
                        [=](void *fn) {
                            const uint32_t saved = *(volatile uint32_t *)0x13d2888;
                            *(volatile uint32_t *)0x13d2888 = arena<uint32_t>(0x10)[0];
                            const void *got = ((const void *(__fastcall *)(void *, void *, uint32_t, void *))fn)(arena<void>(arena<uint32_t>(0x10)[2]), arena<void>(0x1000),
                                                                                                                 arena<uint32_t>(0x10)[1], arena<void>(0x300));
                            *(volatile uint32_t *)0x13d2888 = saved;
                            return Result{(uint32_t)((uintptr_t)got - (uintptr_t)arena<void>(0))};
                        }};
    quick(matrices_times);
    // Record::CopyWithEntries: a record of 0x1018 bytes (random) at 0x3000 copied to 0x5000, or to one that overlaps it
    // a little above or below
    Test record_copy_with_entries{0x9648f0,
                                  [=](Rng &r) {
                                      for (int i = 0; i < 0x4000 / 4; i++) arena<uint32_t>(0x2000)[i] = r.next();
                                      const int mode = r.range(0, 3);
                                      arena<uint32_t>(0x10)[0] = mode == 0 ? 0x3000 + 4 * r.range(1, 0x80) : mode == 1 ? 0x3000 - 4 * r.range(1, 0x80) : 0x5000;
                                  },
                                  [=](void *fn) {
                                      return rel_result(((void *(__thiscall *)(void *, void *))fn)(arena<void>(arena<uint32_t>(0x10)[0]), arena<void>(0x3000)));
                                  }};
    quick(record_copy_with_entries);
    // Table::Make (29 copies, tools/table_makes.py): an object (0x800), sixteen arguments each pointing at its own 0x40
    // bytes (0x3000 on), the copy's global with bit 0 either way and its other bits random; Table::Fill16 is a
    // stand-in that records the table and the sixteen values
    struct TableMakeArgs { void *a[16]; };
    static uint32_t table_make_global, table_make_saved;
    Test table_make{0x4164c0,
                    [=](Rng &r) {
                        fresh(r);
                        for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x800)[i] = r.next();
                        for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x3000)[i] = r.next();
                        arena<uint32_t>(0x10)[2] = r.next();
                        arena<uint32_t>(CALL_LOG)[0] = 0;
                    },
                    [](void *fn) {
                        *(uint32_t *)(uintptr_t)table_make_global = arena<uint32_t>(0x10)[2];
                        TableMakeArgs args;
                        for (int i = 0; i < 16; i++) args.a[i] = arena<void>(0x3000 + 0x40 * i);
                        void *back = ((void *(__thiscall *)(void *, TableMakeArgs))fn)(arena<void>(0x800), args);
                        return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ *(uint32_t *)(uintptr_t)table_make_global};
                    }};
    table_make.prepare = [](const rewrite::Entry *e) {
        for (const uint32_t *v = table_make_info; *v; v += 2)
            if (v[0] == (uint32_t)(uintptr_t)e->at) table_make_global = v[1];
        table_make_saved = *(uint32_t *)(uintptr_t)table_make_global;
        stubs_for_now({{0x416110, (void *)w4b_stub_table}});
    };
    table_make.finish = [](const rewrite::Entry *) { *(uint32_t *)(uintptr_t)table_make_global = table_make_saved; unstub_many(); };
    quick(table_make);
    // Array::ResizeRecords (tools/array_resizes.py): an array (0x100) of 0-8 records at 0x1000 (random; sometimes no
    // items at a count of 0), resized to 0-9 (sometimes the same count); the allocator, the free and ArrayConstruct are
    // stand-ins that record their arguments, the new records coming from 0x2000 on (random before)
    Test array_resize{0x417a20,
                      [=](Rng &r) {
                          fresh(r);
                          for (int i = 0; i < 0x400; i++) arena<uint32_t>(0x1000)[i] = r.next();
                          for (int i = 0; i < 0x400; i++) arena<uint32_t>(0x2000)[i] = r.next();
                          const int32_t count = r.range(0, 8);
                          uint32_t *self = arena<uint32_t>(0x100);
                          self[0] = count || r.range(0, 1) ? (uint32_t)(uintptr_t)arena<void>(0x1000) : 0;
                          self[1] = (uint32_t)count;
                          arena<int32_t>(0x10)[0] = r.range(0, 3) ? r.range(0, 9) : count;
                          *arena<uint32_t>(ALLOC_NEXT) = 0;
                          arena<uint32_t>(CALL_LOG)[0] = 0;
                      },
                      [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0])}; }};
    array_resize.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfd0, (void *)stub_alloc_next}, {0x402b90, (void *)stub_vector_construct4}, {0xbce010, (void *)stub_free_aligned}});
    };
    array_resize.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(array_resize);
    // Destructor::Steps: the object at 0x1000 (random; 0x2000 bytes or as far as its steps reach). Each member its steps use
    // gets memory of its own after it: one that is taken (IF) is there three times in four, with a recording table for the arguments
    // its call takes and a small count; a shared text has a count of 1 or 2 and a buffer; one stored through is big
    // enough. The globals of a GCOPY are set (the source sometimes -1) for the call and put back after the test;
    // what they hold after is the result. Every direct call is a numbered stand-in.
    Test dtor_steps_test{0x4482a0,
                         [=](Rng &r) {
                             fresh(r);
                             const uint32_t *st = g_dtor_info + 2;
                             const uint32_t n = g_dtor_info[1];
                             uint32_t top = 0;   // the furthest field the steps use: the object reaches past it, the members' memory comes after
                             for (uint32_t i = 0; i < n; i++) {
                                 const uint32_t *s = st + 5 * i, k = s[0];
                                 uint32_t reach = k == 0 || k == 1 || k == 8 || k == 9 || k == 15 || k == 19 || k == 22 ? s[3] : k == 4 || k == 20 ? s[4] : k == 6 ? s[2]
                                                : k == 13 ? ((s[3] & 0xffff) > (s[4] & 0xffff) ? s[3] & 0xffff : s[4] & 0xffff) : k == 17 && s[1] ? s[3] : 0;
                                 if (reach > top) top = reach;
                             }
                             const uint32_t object_end = 0x1000 + ((top + 0x140) & ~0xffu) < 0x3000 ? 0x3000 : 0x1000 + ((top + 0x140) & ~0xffu);
                             for (uint32_t i = 0; i < (object_end - 0x1000) / 4; i++) arena<uint32_t>(0x1000)[i] = r.next();
                             // what each member needs, over all the steps: a call or count belongs to the member taken last
                             struct Need { uint32_t field, size; int arity; uint32_t count_at; bool text, taken, array; uint32_t items_count_at; bool looped, no_sign;
                                           uint32_t record, place; bool all_there, count32; };
                             std::vector<Need> needs;
                             auto need = [&](uint32_t field) -> Need & {
                                 for (Need &x : needs) if (x.field == field) return x;
                                 needs.push_back({field, 0x40, 0, 0, false, false, false, 0, false, false, 4, 0, false, false});
                                 return needs.back();
                             };
                             uint32_t current = 0;
                             for (uint32_t i = 0; i < n; i++) {
                                 const uint32_t *s = st + 5 * i;
                                 if (s[0] == 1) current = s[3], need(current).taken = true;
                                 if (s[0] == 21) current = 0x80000000u | s[3], need(current).taken = true;   // a global's object: kept under its address
                                 if (s[0] == 13) {   // a loop: what the steps need is each item's; kept under a made-up field past the object
                                     current = 0x10000 + (s[3] & 0xffff);
                                     Need &x = need(current);
                                     x.looped = true, x.items_count_at = s[4] & 0xffff, x.no_sign = s[1] & 2;
                                     x.record = s[4] >> 16 ? s[4] >> 16 : 4, x.place = s[3] >> 16, x.all_there = s[1] & 4;   // (records, the pointer's place)
                                 }
                                 if (s[0] == 2) need(current).arity = (int)s[1];
                                 if (s[0] == 7) need(current).arity = (int)s[1], need(current).count_at = s[2];
                                 if (s[0] == 23) need(current).arity = 1, need(current).count_at = s[2], need(current).count32 = true;   // (a dword count)
                                 if (s[0] == 9) need(s[3]).text = true;
                                 if (s[0] == 12) need(current).array = true;
                                 if (s[0] == 10) { Need &x = need(current); if (x.size < ((s[3] + 0x43) & ~0x3fu)) x.size = (s[3] + 0x43) & ~0x3fu; }
                                 if (s[0] == 6) { Need &x = need(s[2]); if (x.size < ((s[3] + 0x43) & ~0x3fu)) x.size = (s[3] + 0x43) & ~0x3fu; }
                             }
                             uint32_t next = object_end;
                             for (const Need &x : needs) {
                                 if (x.looped) {   // an array of 0-3 pointers (the count sometimes below 0), each to an item or null
                                     const int32_t count = r.range(0, 5) || x.no_sign ? r.range(0, 3) : -r.range(1, 3);   // (a count with no sign: never below 0)
                                     *arena<int32_t>(0x1000 + x.items_count_at) = count;
                                     uint8_t *records = arena<uint8_t>(next);   // (4 records of the loop's size, the rest of each random)
                                     for (uint32_t i = 0; i < x.record; i++) ((uint32_t *)records)[i] = r.next();
                                     next += (4 * x.record + 15) & ~15u;
                                     *arena<uint32_t>(0x1000 + (x.field - 0x10000)) = (uint32_t)(uintptr_t)records;
                                     for (int k = 0; k < 4; k++) {
                                         uint32_t *list = (uint32_t *)(records + k * x.record + x.place) - k;   // (list[k]: the pointer's place)
                                         if (!x.all_there && r.range(0, 3) == 0) { list[k] = 0; continue; }   // (null only where the loop checks)
                                         uint32_t *block = arena<uint32_t>(next);
                                         for (uint32_t i = 0; i < x.size / 4; i++) block[i] = r.next();
                                         block[0] = (uint32_t)(uintptr_t)g_dtor_tables[x.arity];
                                         if (x.count_at && x.count32) *(uint32_t *)((uint8_t *)block + x.count_at) = (uint32_t)r.range(1, 2); else if (x.count_at) *(uint16_t *)((uint8_t *)block + x.count_at) = (uint16_t)r.range(1, 2);
                                         list[k] = (uint32_t)(uintptr_t)block;
                                         next += (x.size + 15) & ~15u;
                                     }
                                     continue;
                                 }
                                 if (x.field & 0x80000000u) continue;   // a global's object: placed below
                                 bool is_list = false;   // the array a loop goes over, also taken (to be freed): it stays the array
                                 for (const Need &y : needs) is_list |= y.looped && y.field == x.field + 0x10000;
                                 if (is_list) continue;
                                 const bool stored_through = x.size > 0x40;
                                 if (x.taken && !x.text && !stored_through && r.range(0, 3) == 0) {   // a member that isn't there
                                     *arena<uint32_t>(0x1000 + x.field) = 0;
                                     continue;
                                 }
                                 uint32_t *block = arena<uint32_t>(next);
                                 for (uint32_t i = 0; i < x.size / 4; i++) block[i] = r.next();
                                 block[0] = x.text ? (uint32_t)r.range(1, 2) : (uint32_t)(uintptr_t)g_dtor_tables[x.arity];
                                 if (x.count_at && x.count32) *(uint32_t *)((uint8_t *)block + x.count_at) = (uint32_t)r.range(1, 2); else if (x.count_at) *(uint16_t *)((uint8_t *)block + x.count_at) = (uint16_t)r.range(1, 2);
                                 *arena<uint32_t>(0x1000 + x.field) = (uint32_t)(uintptr_t)block;
                                 if (x.array) {   // an array of records: its count just before the items
                                     block[3] = (uint32_t)r.range(0, 5);
                                     *arena<uint32_t>(0x1000 + x.field) = (uint32_t)(uintptr_t)(block + 4);
                                 }
                                 next += (x.size + 15) & ~15u;
                             }
                             for (uint32_t i = 0; i < n; i++)   // a list node in the object: a neighbour either side, each pointing back at it
                                 if (st[5 * i] == 19) {
                                     uint32_t *node = arena<uint32_t>(0x1000 + st[5 * i + 3]), *a = arena<uint32_t>(next), *b = arena<uint32_t>(next + 0x10);
                                     for (int k = 0; k < 8; k++) a[k] = r.next();
                                     node[0] = (uint32_t)(uintptr_t)a, node[1] = (uint32_t)(uintptr_t)b;
                                     a[1] = b[0] = (uint32_t)(uintptr_t)node;
                                     if (st[5 * i + 1] == 1 && r.range(0, 2) == 0) {   // (a quiet one: sometimes a node not linked both ways)
                                         const uint32_t how = r.range(0, 3);
                                         if (how == 0) node[0] = 0; else if (how == 1) node[1] = 0; else if (how == 2) a[1] = 0; else b[0] = 0;
                                     }
                                     next += 0x20;
                                 }
                             for (uint32_t i = 0; i < n; i++)   // a field checked against -1: -1 or 0 now and then
                                 if (st[5 * i] == 22 && r.range(0, 2) == 0) *arena<uint32_t>(0x1000 + st[5 * i + 3]) = r.range(0, 1) ? 0xffffffffu : 0;
                             for (uint32_t i = 0; i < n; i++)   // a flag byte: clear, set, or anything
                                 if (st[5 * i] == 15) *arena<uint8_t>(0x1000 + st[5 * i + 3]) = (uint8_t)(r.range(0, 2) ? r.range(0, 1) : r.next());
                             for (size_t i = 0; i < g_dtor_globals.size(); i++) arena<uint32_t>(0x40)[i] = r.range(0, 2) ? r.next() : 0xffffffffu;
                             for (const Need &x : needs)   // an object a global points to: there three times in four
                                 if (x.field & 0x80000000u)
                                     for (size_t i = 0; i < g_dtor_globals.size(); i++)
                                         if (g_dtor_globals[i].first == (x.field & 0x7fffffffu)) {
                                             if (r.range(0, 3) == 0) { arena<uint32_t>(0x40)[i] = 0; continue; }
                                             uint32_t *block = arena<uint32_t>(next);
                                             for (uint32_t k = 0; k < x.size / 4; k++) block[k] = r.next();
                                             block[0] = (uint32_t)(uintptr_t)g_dtor_tables[x.arity];
                                             if (x.count_at && x.count32) *(uint32_t *)((uint8_t *)block + x.count_at) = (uint32_t)r.range(1, 2); else if (x.count_at) *(uint16_t *)((uint8_t *)block + x.count_at) = (uint16_t)r.range(1, 2);
                                             if (x.array) block[3] = (uint32_t)r.range(0, 5);
                                             arena<uint32_t>(0x40)[i] = (uint32_t)(uintptr_t)(x.array ? block + 4 : block);
                                             next += (x.size + 15) & ~15u;
                                         }
                             arena<uint32_t>(CALL_LOG)[0] = 0;
                         },
                         [](void *fn) {
                             for (size_t i = 0; i < g_dtor_globals.size(); i++) *(uint32_t *)(uintptr_t)g_dtor_globals[i].first = arena<uint32_t>(0x40)[i];
                             ((void(__thiscall *)(void *))fn)(arena<void>(0x1000));
                             uint32_t after = 0;
                             for (size_t i = 0; i < g_dtor_globals.size(); i++) after = after * 31 + *(uint32_t *)(uintptr_t)g_dtor_globals[i].first;
                             return Result{after};
                         }};
    dtor_steps_test.prepare = [](const rewrite::Entry *e) {
        g_dtor_info = nullptr;
        for (const uint32_t *v = dtor_steps_info; *v; v += 2 + 5 * v[1])
            if (v[0] == (uint32_t)(uintptr_t)e->at) g_dtor_info = v;
        std::vector<std::pair<uint32_t, void *>> stubs;
        g_dtor_globals.clear();
        auto add = [&](uint32_t target, void **from) {
            for (auto &s : stubs) if (s.first == target) return;
            if (stubs.size() < 48) stubs.push_back({target, from[stubs.size()]});
        };
        const uint32_t *st = g_dtor_info + 2;
        for (uint32_t i = 0; i < g_dtor_info[1]; i++) {
            const uint32_t *s = st + 5 * i;
            if (s[0] == 4 || s[0] == 18 || s[0] == 20) add(s[3], g_dtor_direct[s[1]]);
            if (s[0] == 11) add(s[3], g_dtor_direct[0]);
            if (s[0] == 12) { add(0xbce010, g_dtor_freed); bool have = false; for (auto &x : stubs) have |= x.first == 0xddcf39; if (!have) stubs.push_back({0xddcf39, (void *)dtor_vec_dtor}); }
            if (s[0] == 3) add(s[3], g_dtor_freed);
            if (s[0] == 8) add(s[4], g_dtor_freed);
            if (s[0] == 9) add(0xbce010, g_dtor_freed), add(0xbce000, g_dtor_freed);
            if (s[0] == 5 || s[0] == 14 || s[0] == 16 || s[0] == 18 || s[0] == 21)
                for (uint32_t g : {s[0] == 18 ? s[4] : s[3], s[0] == 5 || s[0] == 16 ? s[4] : s[0] == 18 ? s[4] : s[3]}) {
                    bool have = false;
                    for (auto &x : g_dtor_globals) have |= x.first == g;
                    if (!have) g_dtor_globals.push_back({g, *(uint32_t *)(uintptr_t)g});
                }
        }
        stubs_for_now(stubs);
    };
    dtor_steps_test.finish = [](const rewrite::Entry *) {
        for (auto &g : g_dtor_globals) *(uint32_t *)(uintptr_t)g.first = g.second;
        unstub_many();
    };
    quick(dtor_steps_test);
    // GE_Device::GE_Device: the device at 0x9000 (0x24A0 bytes, random before), four values given. The base, the array
    // constructors, the texts' setup, the debug names, both allocators (the plain one sometimes out of memory), the
    // assembler and its code buffers, the error report, the state record and the two last calls are stand-ins that
    // fold everything they are given into a sum; the small value helpers run as they are. The result is the device
    // given back and that sum.
    Test device_construct{0xc77890,
                          [=](Rng &r) {
                              fresh(r);
                              for (int i = 0; i < 0x928; i++) arena<uint32_t>(0x9000)[i] = r.next();
                              for (int i = 0; i < 0x80; i++) arena<uint32_t>(0x7000)[i] = r.next();
                              for (int i = 0; i < 0x1400; i++) arena<uint32_t>(0x2000)[i] = r.next();
                              for (int i = 0; i < 4; i++) arena<uint32_t>(0x10)[i] = r.next();
                              arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                          },
                          [](void *fn) {
                              const uint32_t *a = arena<uint32_t>(0x10);
                              void *back = ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x9000), a[0], a[1], a[2], a[3]);
                              return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ *arena<uint32_t>(0x20)};
                          }};
    device_construct.iterations = 100;
    device_construct.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xc07350, (void *)dev_base}, {0x402b90, (void *)dev_array}, {0xc77390, (void *)dev_forward}, {0xc09090, (void *)dev_text_init},
                       {0xc73c20, (void *)dev_name}, {0xbcdfc0, (void *)dev_alloc}, {0xbcdfd0, (void *)dev_alloc_aligned}, {0xcf6275, (void *)dev_assemble},
                       {0xdd696e, (void *)dev_error_text}, {0xc34850, (void *)dev_report}, {0xbe2030, (void *)dev_state}, {0xc061d0, (void *)dev_pass_shaders},
                       {0xc75290, (void *)dev_clear}});
    };
    device_construct.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(device_construct);
    // Cheats::Init: the table object at 0x9000; the allocators, the frees, the text made from C text, the text join and
    // the entry's constructor are stand-ins (see above); the result is the object given back and the running sum
    Test cheats_init{0x410ed0,
                     [=](Rng &r) {
                         fresh(r);
                         for (int i = 0; i < 8; i++) arena<uint32_t>(0x9000)[i] = r.next();
                         for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                         arena<uint32_t>(0x10)[0] = r.next();
                         arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                     },
                     [](void *fn) {
                         void *back = ((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x9000), arena<uint32_t>(0x10)[0]);
                         return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ *arena<uint32_t>(0x20)};
                     }};
    cheats_init.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)cheat_alloc}, {0xbcdfd0, (void *)dev_alloc_aligned}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free},
                       {0xbcf850, (void *)cheat_text}, {0x403be0, (void *)cheat_join}, {0x410d50, (void *)cheat_entry}});
    };
    cheats_init.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(cheats_init);
    // RCTDesktop::CreateWindows: the desktop at arena 0x8000, the default description (the global 0x12F7190) at 0x7000
    // (its own name: none, or a C text of the game), a title or none (a holder at 0x7100 of a text at 0x7200), a
    // description or none (0x7300); every function called is a stand-in (see above)
    Test desktop_create{0x6249d0,
                        [=](Rng &r) {
                            fresh(r);
                            for (int i = 0; i < 0x19c0; i++) arena<uint32_t>(0x8000)[i] = r.next();
                            for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x7000)[i] = r.next();
                            for (int i = 0; i < 0x400; i++) arena<uint32_t>(0x2000)[i] = r.next();
                            arena<uint32_t>(0x7000)[2] = r.range(0, 2) ? 0xf2738c : 0;
                            arena<uint32_t>(0x7100)[0] = (uint32_t)(uintptr_t)arena<void>(0x7200);
                            arena<uint32_t>(0x7200)[0] = r.range(1, 3);
                            arena<uint32_t>(0x7180)[0] = (uint32_t)(uintptr_t)arena<void>(0x7280);
                            arena<uint32_t>(0x7280)[0] = r.range(1, 3);
                            arena<uint32_t>(0x10)[0] = r.range(0, 1) ? (uint32_t)(uintptr_t)arena<void>(0x7300) : 0;
                            arena<uint32_t>(0x10)[1] = r.range(0, 2) ? (uint32_t)(uintptr_t)arena<void>(0x7100) : 0;
                            arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                            arena<uint32_t>(0x30)[0] = 0;
                        },
                        [](void *fn) {
                            *(void **)0x12f7190 = arena<void>(0x7000);
                            const uint32_t *a = arena<uint32_t>(0x10);
                            void *back = ((void *(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x8000), a[0], a[1]);
                            return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ *arena<uint32_t>(0x20)};
                        }};
    desktop_create.iterations = 200;
    desktop_create.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)desk_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)cheat_text},
                       {0xbd0490, (void *)desk_empty}, {0x403be0, (void *)cheat_join}, {0x403bb0, (void *)desk_join_wide}, {0x41c740, (void *)desk_copy},
                       {0x681360, (void *)desk_base}, {0xbd37a0, (void *)desk_init}, {0x6b3190, (void *)desk_list}, {0x6b3390, (void *)desk_list_value},
                       {0xbd1940, (void *)desk_key}, {0x6b3570, (void *)desk_list_key}, {0x42abc0, (void *)desk_window}, {0x4840b0, (void *)desk_message},
                       {0xbd31f0, (void *)desk_show}});
    };
    desktop_create.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(desktop_create);
    // DebugEditor::Number (eight copies): the editor at arena 0x8000; a label (C text) or none, a width, the value's place,
    // lowest, highest, step, and whether to show a slider (a byte; the rest of its word set too); the stand-ins above
    Test debug_editor{0xca88d0,
                      [=](Rng &r) {
                          fresh(r);
                          for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x8000)[i] = r.next();
                          for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                          uint32_t *a = arena<uint32_t>(0x40);
                          a[0] = r.range(0, 2) ? 0xf2738c : 0;
                          for (int i = 1; i < 6; i++) a[i] = r.next() ^ (uint32_t)r.range(0, 0xffff) * 0x10001u;   // (low bytes and words of their own)
                          a[6] = (r.next() & ~0xffu) | (r.range(0, 1) ? r.range(1, 255) : 0);
                          arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                          arena<uint32_t>(0x30)[0] = 0;
                      },
                      [](void *fn) {
                          const uint32_t *a = arena<uint32_t>(0x40);
                          typedef void *(__thiscall * Editor)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
                          void *back = ((Editor)fn)(arena<void>(0x8000), a[0], a[1], a[2], a[3], a[4], a[5], a[6]);
                          return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ *arena<uint32_t>(0x20)};
                      }};
    debug_editor.iterations = 300;
    debug_editor.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)ed_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)cheat_text},
                       {0xbd0490, (void *)desk_empty}, {0xbd05b0, (void *)ed_empty_text}, {0xc69eb0, (void *)ed_base}, {0xc69f00, (void *)ed_add},
                       {0xc661d0, (void *)ed_font}, {0xc699c0, (void *)ed_part}, {0xc6b400, (void *)ed_slider}, {0xc686c0, (void *)ed_record}});
    };
    debug_editor.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(debug_editor);
    // Folder::LoadFiles (four copies): the object at arena 0x8000, a walk of 0 to 7 entries; the stand-ins above, with the
    // copy's own clear and load functions
    Test load_files{0x731e10,
                    [=](Rng &r) {
                        fresh(r);
                        for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x8000)[i] = r.next();
                        for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                        arena<uint32_t>(0x9000)[1] = 0, arena<uint32_t>(0x9000)[2] = r.range(0, 7), arena<uint32_t>(0x9000)[3] = r.next();
                        arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                    },
                    [](void *fn) {
                        ((void(__thiscall *)(void *))fn)(arena<void>(0x8000));
                        return Result{*arena<uint32_t>(0x20)};
                    }};
    load_files.iterations = 300;
    load_files.prepare = [](const rewrite::Entry *e) {
        uint32_t clear = 0, load = 0;
        for (auto &row : lf_rows)
            if (row[0] == e->at) clear = row[1], load = row[3];
        stubs_for_now({{0xbcdfc0, (void *)lf_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcfb10, (void *)lf_text_c},
                       {clear, (void *)lf_clear}, {load, (void *)lf_load}, {0xbe5570, (void *)lf_walker}, {0x4042d0, (void *)lf_entry_name},
                       {0xbcf040, (void *)lf_compare}, {0xbcfef0, (void *)lf_copy}, {0xbe47b0, (void *)lf_walker_gone}});
    };
    load_files.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(load_files);
    // Window::MessageNumbered (five copies): where the result goes (arena 0x8000) and a number (mostly 0 to 5, sometimes
    // anything); the copy's six globals set for the case
    Test numbered_message{0x442c60,
                          [=](Rng &r) {
                              fresh(r);
                              for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                              arena<uint32_t>(0x8000)[0] = r.next();
                              arena<uint32_t>(0x40)[0] = r.range(0, 3) ? r.range(0, 5) : r.next();
                              for (int k = 0; k < 6; k++) arena<uint32_t>(0x50)[k] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x7400 + 0x20 * k) : 0;
                              arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                          },
                          [](void *fn) {
                              for (int k = 0; k < 6; k++) *(uint32_t *)(uintptr_t)g_msg_globals[k] = arena<uint32_t>(0x50)[k];
                              void *back = ((void *(__fastcall *)(void *, uint32_t))fn)(arena<void>(0x8000), arena<uint32_t>(0x40)[0]);
                              return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ *arena<uint32_t>(0x20)};
                          }};
    numbered_message.iterations = 300;
    numbered_message.prepare = [](const rewrite::Entry *e) {
        for (auto &row : msg_rows)
            if (row[0] == e->at)
                for (int k = 0; k < 6; k++) g_msg_globals[k] = row[1 + k], g_msg_saved[k] = *(uint32_t *)(uintptr_t)row[1 + k];
        stubs_for_now({{0xbcdfc0, (void *)msg_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbd0490, (void *)desk_empty},
                       {0xbcf7d0, (void *)msg_wide}, {0xbcf850, (void *)msg_text_c}, {0xddcdb5, (void *)msg_sprintf}, {0xcd23a0, (void *)msg_replace}});
    };
    numbered_message.finish = [](const rewrite::Entry *) {
        unstub_many();
        for (int k = 0; k < 6; k++) *(uint32_t *)(uintptr_t)g_msg_globals[k] = g_msg_saved[k];
    };
    quick(numbered_message);
    // Money::FormatNamed (three copies): where the text goes (arena 0x8000) and a number (below 10, or above)
    Test money_named{0x6bd4c0,
                     [=](Rng &r) {
                         fresh(r);
                         for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                         arena<uint32_t>(0x8000)[0] = r.next();
                         // (not negative: the game takes it signed and reads before its table, as the rewrite does)
                         arena<int32_t>(0x40)[0] = r.range(0, 2) ? r.range(0, 25) : (int32_t)(r.next() & 0x7fffffff);
                         arena<uint32_t>(0x44)[0] = r.next();
                         for (int k = 0; k < 11; k++) {   // the named texts: their characters at +8, or none
                             arena<uint32_t>(0x7600 + 0x10 * k)[2] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x7800 + 0x10 * k) : 0;
                         }
                         arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                     },
                     [](void *fn) {
                         for (int k = 0; k < 10; k++) ((uint32_t *)(uintptr_t)g_mf_table)[k] = (uint32_t)(uintptr_t)arena<void>(0x7600 + 0x10 * k);
                         *(uint32_t *)(uintptr_t)g_mf_fallback = (uint32_t)(uintptr_t)arena<void>(0x7600 + 0xa0);
                         void *back = ((void *(__fastcall *)(void *, int32_t))fn)(arena<void>(0x8000), arena<int32_t>(0x40)[0]);
                         return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ *arena<uint32_t>(0x20)};
                     }};
    money_named.iterations = 300;
    money_named.prepare = [](const rewrite::Entry *e) {
        for (auto &row : mf_rows)
            if (row[0] == e->at) g_mf_table = row[1], g_mf_fallback = row[3];
        memcpy(g_mf_saved, (void *)(uintptr_t)g_mf_table, 40);
        g_mf_saved[10] = *(uint32_t *)(uintptr_t)g_mf_fallback;
        stubs_for_now({{0xbcdfc0, (void *)msg_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbd0490, (void *)desk_empty},
                       {0xbcf7d0, (void *)msg_wide}, {0xbcf850, (void *)msg_text_c}, {0x453630, (void *)mf_language}, {0x403bb0, (void *)mf_join},
                       {0xbd01c0, (void *)mf_copy}, {0x410c90, (void *)mf_named}, {0xbd1420, (void *)mf_bit0}, {0xbd1880, (void *)mf_named_gone}});
    };
    money_named.finish = [](const rewrite::Entry *) {
        unstub_many();
        memcpy((void *)(uintptr_t)g_mf_table, g_mf_saved, 40);
        *(uint32_t *)(uintptr_t)g_mf_fallback = g_mf_saved[10];
    };
    quick(money_named);
    // Condition::CurrentText (four copies): the condition at arena 0x8000, where the text goes (0x8100), a part (mostly 0)
    // and the condition's value (any float, rounded by the FPU's mode)
    Test current_text{0x6df560,
                      [=](Rng &r) {
                          fresh(r);
                          for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                          ct_table[0x68 / 4] = (void *)ct_value;
                          arena<uint32_t>(0x8000)[0] = (uint32_t)(uintptr_t)ct_table;
                          arena<uint32_t>(0x8100)[0] = r.next();
                          arena<int32_t>(0x40)[0] = r.range(0, 3) ? 0 : r.range(1, 5);
                          arena<float>(0x48)[0] = r.range(0, 4) ? r.real(-100000, 100000) : r.any_float();
                          arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                      },
                      [](void *fn) {
                          void *back = ((void *(__thiscall *)(void *, void *, int32_t))fn)(arena<void>(0x8000), arena<void>(0x8100), arena<int32_t>(0x40)[0]);
                          return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ *arena<uint32_t>(0x20)};
                      }};
    current_text.iterations = 300;
    current_text.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)msg_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)msg_text_c},
                       {0x4040f0, (void *)stub_class_named_make}, {0x442120, (void *)ct_put_together}, {0xbd1880, (void *)mf_named_gone},
                       {0xddcdb5, (void *)msg_sprintf}, {0xcd23a0, (void *)msg_replace}, {0x6d6ca0, (void *)ct_timing}});
    };
    current_text.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(current_text);
    // Objectives::LabelledNumber (six copies): the objective at arena 0x8000 (its number at +0x70), where the text goes
    Test labelled_number{0x6e4580,
                         [=](Rng &r) {
                             fresh(r);
                             for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                             for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x8000)[i] = r.next();
                             arena<uint32_t>(0x8100)[0] = r.next();
                             arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                         },
                         [](void *fn) {
                             void *back = ((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x8000), arena<void>(0x8100));
                             return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ *arena<uint32_t>(0x20)};
                         }};
    labelled_number.iterations = 300;
    labelled_number.prepare = [](const rewrite::Entry *e) {
        uint32_t label = 0;
        for (auto &row : ot_rows)
            if (row[0] == e->at) label = row[1];
        stubs_for_now({{0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xcd23d0, (void *)ot_decimal}, {0x6be770, (void *)ot_money},
                       {label, (void *)ot_label}, {0x403be0, (void *)cheat_join}, {0x403bb0, (void *)mf_join}});
    };
    labelled_number.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(labelled_number);
    // Text::NumberedName (eight copies): where the text goes (arena 0x8000, in esi) and a number
    Test numbered_name{0x46d730,
                       [=](Rng &r) {
                           fresh(r);
                           for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                           arena<uint32_t>(0x8000)[0] = r.next();
                           arena<int32_t>(0x40)[0] = r.range(0, 3) ? r.range(-2, 120) : (int32_t)r.next();
                           arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                       },
                       [](void *fn) {
                           void *back = call_with_esi(fn, arena<void>(0x8000), arena<int32_t>(0x40)[0]);
                           return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ *arena<uint32_t>(0x20)};
                       }};
    numbered_name.iterations = 300;
    numbered_name.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)msg_alloc}, {0xbcf850, (void *)msg_text_c}, {0xddcdb5, (void *)msg_sprintf}});
    };
    numbered_name.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(numbered_name);
    // Report::MissingResourceSymbol (four copies): the reference at arena 0x8000 points at a resource (0x8100) whose flag
    // is set or not
    Test missing_symbol{0x431360,
                        [=](Rng &r) {
                            fresh(r);
                            for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                            arena<uint32_t>(0x8000)[0] = (uint32_t)(uintptr_t)arena<void>(0x8100);
                            arena<uint32_t>(0x8100)[1] = r.next();
                            arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                        },
                        [](void *fn) {
                            ((void(__thiscall *)(void *))fn)(arena<void>(0x8000));
                            return Result{*arena<uint32_t>(0x20)};
                        }};
    missing_symbol.iterations = 200;
    missing_symbol.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)msg_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)msg_text_c},
                       {0xbd1420, (void *)ms_flag}, {0xbcfef0, (void *)ms_name}, {0x468450, (void *)ms_box}});
    };
    missing_symbol.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(missing_symbol);
    // Animal::Point (four copies): the animal at arena 0x8000 (its span's ends at +0x7C and +0x84: any floats), its two
    // windows; the rectangles given are noted bit for bit
    Test animal_point{0xace100,
                      [=](Rng &r) {
                          fresh(r);
                          for (int i = 0; i < 0x50; i++) arena<uint32_t>(0x8000)[i] = r.next();
                          arena<float>(0x8000)[0x7c / 4] = r.range(0, 4) ? r.real(-500, 500) : r.any_float();
                          arena<float>(0x8000)[0x84 / 4] = r.range(0, 4) ? r.real(-500, 500) : r.any_float();
                          arena<uint32_t>(0x8000)[0x114 / 4] = (uint32_t)(uintptr_t)arena<void>(0x8200);
                          arena<uint32_t>(0x8000)[0x118 / 4] = (uint32_t)(uintptr_t)arena<void>(0x8300);
                          arena<uint32_t>(0x20)[0] = 0;
                      },
                      [](void *fn) {
                          ((void(__thiscall *)(void *))fn)(arena<void>(0x8000));
                          return Result{*arena<uint32_t>(0x20)};
                      }};
    animal_point.iterations = 1000;
    animal_point.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x6afaf0, (void *)ap_rect}}); };
    animal_point.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(animal_point);
    // Text::DecimalWithUnit (four copies): where the text goes (arena 0x8000) and a float (ordinary, or any bit pattern)
    Test decimal_text{0x49d650,
                      [=](Rng &r) {
                          fresh(r);
                          for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                          arena<uint32_t>(0x8000)[0] = r.next();
                          arena<float>(0x40)[0] = r.range(0, 4) ? r.real(-5000, 5000) : r.any_float();
                          arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                      },
                      [](void *fn) {
                          void *back = ((void *(__thiscall *)(void *, float))fn)(arena<void>(0x8000), arena<float>(0x40)[0]);
                          return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ *arena<uint32_t>(0x20)};
                      }};
    decimal_text.iterations = 300;
    decimal_text.prepare = [](const rewrite::Entry *e) {
        uint32_t unit = 0;
        for (auto &row : dt_rows)
            if (row[0] == e->at) unit = row[1];
        stubs_for_now({{0xbcdfc0, (void *)msg_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)msg_text_c},
                       {0xddcdb5, (void *)msg_sprintf}, {0x49d480, (void *)dt_point}, {unit, (void *)ot_label}, {0x403ce0, (void *)dt_from_c_and},
                       {0x403bb0, (void *)mf_join}});
    };
    decimal_text.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(decimal_text);
    // Effect::WithinBox (eight copies): the effect at arena 0x8000 (its part at 0x8100, flagged or not), the box at 0x8200;
    // values near each other so both answers come up, and sometimes any bit pattern (NaNs among them)
    Test within_box{0x48bd10,
                    [=](Rng &r) {
                        fresh(r);
                        auto value = [&]() -> float {   // halves (ties), fine values (sums that round), NaNs and infinities, any bits
                            switch (r.range(0, 9)) {
                            case 0: return std::numeric_limits<float>::quiet_NaN();
                            case 1: return r.range(0, 1) ? std::numeric_limits<float>::infinity() : -std::numeric_limits<float>::infinity();
                            case 2: return r.any_float();
                            case 3: case 4: case 5: return r.real(-3, 3) * (1.0f + 1.0f / 3.0f);
                            default: return (float)r.range(-6, 6) * 0.5f;
                            }
                        };
                        uint32_t *self = arena<uint32_t>(0x8000);
                        self[2] = (uint32_t)(uintptr_t)arena<void>(0x8100);
                        for (int k = 3; k < 7; k++) arena<float>(0x8000)[k] = value();
                        arena<uint32_t>(0x8100)[0] = r.next();
                        arena<float>(0x8100)[1] = value(), arena<float>(0x8100)[2] = value();
                        for (int k = 0; k < 3; k++) arena<float>(0x8200)[k] = value() - 2.0f;
                        for (int k = 3; k < 6; k++) arena<float>(0x8200)[k] = value() + 2.0f;
                        arena<uint32_t>(0x44)[0] = r.next();
                        if (r.range(0, 3) == 0) {   // the box's low edges just at the far ends rounded to floats (rounding them first would differ)
                            const float *e = arena<float>(0x8000);
                            arena<float>(0x8200)[0] = (float)((double)e[3] + e[5]);
                            arena<float>(0x8200)[2] = (float)((double)e[6] + e[4]);
                        }
                        arena<uint32_t>(CALL_LOG)[0] = 0;
                    },
                    [](void *fn) {
                        int32_t got = ((int32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x8000), arena<void>(0x8200));
                        return Result{(uint32_t)got};
                    }};
    within_box.iterations = 3000;
    within_box.prepare = [](const rewrite::Entry *e) {
        for (auto &row : eb_rows)
            if (row[0] == e->at) stubs_for_now({{row[1], (void *)eb_update}, {row[2], (void *)eb_other}, {row[3], (void *)eb_grow}});
    };
    within_box.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(within_box);
    // Effect::RayHit (four copies): the effect at arena 0x8000 (its part at 0x8100, flagged or not; its place and size any
    // floats, signalling NaNs among them), the ray {0x8300, 0x8310} at 0x8200
    Test ray_hit{0x48aff0,
                 [=](Rng &r) {
                     fresh(r);
                     uint32_t *self = arena<uint32_t>(0x8000);
                     self[2] = (uint32_t)(uintptr_t)arena<void>(0x8100);
                     for (int k = 3; k < 7; k++) {
                         float f = r.real(-100, 100);
                         uint32_t bits;
                         memcpy(&bits, &f, 4);
                         arena<uint32_t>(0x8000)[k] = r.range(0, 5) ? bits : (r.range(0, 1) ? 0x7fa00001u : r.next());   // (0x7FA00001: a signalling NaN)
                     }
                     arena<uint32_t>(0x8100)[0] = r.next(), arena<uint32_t>(0x8100)[1] = r.next(), arena<uint32_t>(0x8100)[2] = r.next();
                     arena<uint32_t>(0x8200)[0] = (uint32_t)(uintptr_t)arena<void>(0x8300), arena<uint32_t>(0x8200)[1] = (uint32_t)(uintptr_t)arena<void>(0x8310);
                     arena<uint32_t>(0x44)[0] = r.next();
                     arena<uint32_t>(CALL_LOG)[0] = 0;
                 },
                 [](void *fn) {
                     int32_t got = ((int32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x8000), arena<void>(0x8200));
                     return Result{(uint32_t)got};
                 }};
    ray_hit.iterations = 2000;
    ray_hit.prepare = [](const rewrite::Entry *e) {
        for (auto &row : ef_rows)
            if (row[0] == e->at)
                stubs_for_now({{row[1], (void *)eb_update}, {row[2], (void *)eb_other}, {row[3], (void *)eb_grow}, {0xcda7f0, (void *)ef_slabs}});
    };
    ray_hit.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(ray_hit);
    // Window::TitledFromDescription (three copies): the window at arena 0x8000; its default description (the copy's global)
    // at 0x7000 (its own name: none or a C text), a description or none (0x7300), a title or none (0x7100 holding 0x7200)
    Test titled_window{0x4f0390,
                       [=](Rng &r) {
                           fresh(r);
                           for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x8000)[i] = r.next();
                           for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x7000)[i] = r.next();
                           for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                           arena<uint32_t>(0x7000)[2] = r.range(0, 2) ? 0xf2738c : 0;
                           arena<uint32_t>(0x7100)[0] = (uint32_t)(uintptr_t)arena<void>(0x7200);
                           arena<uint32_t>(0x7200)[0] = r.range(1, 3);
                           arena<uint32_t>(0x7180)[0] = (uint32_t)(uintptr_t)arena<void>(0x7280);
                           arena<uint32_t>(0x7280)[0] = r.range(1, 3);
                           arena<uint32_t>(0x40)[0] = r.range(0, 1) ? (uint32_t)(uintptr_t)arena<void>(0x7300) : 0;
                           arena<uint32_t>(0x40)[1] = r.range(0, 2) ? (uint32_t)(uintptr_t)arena<void>(0x7100) : 0;
                           arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                           arena<uint32_t>(0x30)[0] = 0;
                       },
                       [](void *fn) {
                           *(uint32_t *)(uintptr_t)g_tw_global = (uint32_t)(uintptr_t)arena<void>(0x7000);
                           const uint32_t *a = arena<uint32_t>(0x40);
                           void *back = ((void *(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x8000), a[0], a[1]);
                           return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ *arena<uint32_t>(0x20)};
                       }};
    titled_window.iterations = 300;
    titled_window.prepare = [](const rewrite::Entry *e) {
        for (auto &row : tw_rows)
            if (row[0] == e->at) g_tw_global = row[1], g_tw_saved = *(uint32_t *)(uintptr_t)row[1];
        stubs_for_now({{0xbcdfc0, (void *)desk_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)cheat_text},
                       {0xbd0490, (void *)desk_empty}, {0x403be0, (void *)cheat_join}, {0x403bb0, (void *)desk_join_wide}, {0x41c740, (void *)desk_copy},
                       {0x41cb90, (void *)tw_reference}, {0x69a280, (void *)tw_base}, {0xbd1880, (void *)tw_reference_gone}});
    };
    titled_window.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)(uintptr_t)g_tw_global = g_tw_saved;
    };
    quick(titled_window);
    // Folder::SpecialOrDocuments (three copies): where the path goes (arena 0x8000)
    Test folder_path{0x458170,
                     [=](Rng &r) {
                         fresh(r);
                         for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                         arena<uint32_t>(0x8000)[0] = r.next();
                         arena<uint32_t>(0x44)[0] = r.next();
                         arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                     },
                     [](void *fn) {
                         void *back = ((void *(__thiscall *)(void *))fn)(arena<void>(0x8000));
                         return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ *arena<uint32_t>(0x20)};
                     }};
    folder_path.iterations = 300;
    folder_path.prepare = [](const rewrite::Entry *e) {
        uint32_t fallback = 0;
        for (auto &row : fp_rows)
            if (row[0] == e->at) fallback = row[3];
        std::vector<std::pair<uint32_t, void *>> list = {{0xbcdfc0, (void *)msg_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free},
                                                         {0xbcf850, (void *)msg_text_c}, {0x458010, (void *)fp_special}, {0xbce970, (void *)fp_compare},
                                                         {0x458130, (void *)fp_documents}, {0x403be0, (void *)cheat_join}, {0xbd01c0, (void *)mf_copy},
                                                         {0xbe45e0, (void *)fp_there}};
        if (fallback != 0x458130) list.push_back({fallback, (void *)fp_fallback});
        stubs_for_now(list);
    };
    folder_path.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(folder_path);
    // Designer::DefaultName (two copies): the design at arena 0x8000 (its name at +0x54 and +0x44: one of a few kinds,
    // sometimes shared), where the name goes (0x8100)
    Test default_name{0xad7e10,
                      [=](Rng &r) {
                          fresh(r);
                          for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x8000)[i] = r.next();
                          for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();   // (the texts' memory: what is past their ends shows)
                          arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                          static const char *const names[] = {"%%LaserShow", "##Fountain", "%%", "##", "My Laser's Show", "a b", "%x", "", "Water 'Jet' 2", "%%A"};
                          const char *pick = names[r.range(0, 9)];
                          uint16_t wide[64];
                          uint32_t n = 0;
                          while (pick[n]) wide[n] = (uint8_t)pick[n], n++;
                          DnText *name = dn_make((DnText *)dev_take(0x10), wide, n);
                          name->count = r.range(1, 3);
                          arena<uint32_t>(0x8000)[0x54 / 4] = (uint32_t)(uintptr_t)name;
                          arena<uint32_t>(0x8000)[0x44 / 4] = (uint32_t)(uintptr_t)name;
                          arena<uint32_t>(0x8100)[0] = r.next();
                          arena<uint32_t>(0x44)[0] = r.next(), arena<uint32_t>(0x48)[0] = r.next();
                      },
                      [](void *fn) {
                          void *back = ((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x8000), arena<void>(0x8100));
                          return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ *arena<uint32_t>(0x20)};
                      }};
    default_name.iterations = 400;
    default_name.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)msg_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)dn_text_c},
                       {0xbcf710, (void *)dn_from_n}, {0xbd0810, (void *)dn_part}, {0xbce970, (void *)dn_compare}, {0xbd01c0, (void *)dn_copy},
                       {0xbcffe0, (void *)dn_room}, {0x410c90, (void *)dn_lookup}, {0xbd1420, (void *)dn_found}, {0xbd1880, (void *)dn_lookup_gone},
                       {0xbcf7d0, (void *)dn_wide}, {0xbd0490, (void *)desk_empty}, {0x401310, (void *)dn_check}});
    };
    default_name.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(default_name);
    // Designer::Save (two copies): the designer at arena 0x8000 (its state 0, 1 or another; its path a text; its
    // file writer +0x2F8), a folder and a name (each a holder of a text: names of every kind)
    Test designer_save{0xad0980,
                       [=](Rng &r) {
                           fresh(r);
                           for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x8000)[i] = r.next();
                           for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                           arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                           arena<uint32_t>(0x8000)[0x2fc / 4] = r.range(0, 5) ? 1 : r.range(0, 3);
                           arena<uint32_t>(0x8000)[0x2f8 / 4] = (uint32_t)(uintptr_t)arena<void>(0x8800);
                           static const char *const names[] = {"", ".hidden", " spaced", "Show", "Show.las", "Fountain.wjt", "x"};
                           DnText *path = ds_c("Old\\Path");
                           path->count = r.range(1, 2);
                           arena<uint32_t>(0x8000)[0x398 / 4] = (uint32_t)(uintptr_t)path;
                           arena<uint32_t>(0x40)[0] = (uint32_t)(uintptr_t)ds_c("Designs\\");
                           arena<uint32_t>(0x40)[1] = (uint32_t)(uintptr_t)ds_c(names[r.range(0, 6)]);
                           arena<uint32_t>(0x50)[0] = r.next(), arena<uint32_t>(0x54)[0] = r.next(), arena<uint32_t>(0x58)[0] = r.next();
                       },
                       [](void *fn) {
                           ((void(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x8000), arena<void>(0x40), arena<void>(0x44));   // (holders: the folder's, the name's)
                           return Result{*arena<uint32_t>(0x20)};
                       }};
    designer_save.iterations = 400;
    designer_save.prepare = [](const rewrite::Entry *e) {
        uint32_t write = 0;
        for (auto &row : ds_rows)
            if (row[0] == e->at) write = row[2];
        stubs_for_now({{0xbcdfc0, (void *)msg_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)dn_text_c},
                       {0xbd0490, (void *)ds_empty}, {0x403bb0, (void *)ds_join}, {0x402550, (void *)ds_has_ending}, {0xbcffe0, (void *)dn_room},
                       {0xbd01c0, (void *)dn_copy}, {0xbe45e0, (void *)ds_there}, {0x4040f0, (void *)ds_named}, {0xbcf7d0, (void *)dn_wide},
                       {0xbd1880, (void *)dn_lookup_gone}, {0xcd23a0, (void *)ds_replace}, {0x6a5e20, (void *)ds_window}, {0x6a6ac0, (void *)ds_buttons},
                       {0x800460, (void *)ds_given}, {write, (void *)ds_given}});
    };
    designer_save.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(designer_save);
    // Designer::FileButtonTip (four copies): the designer at arena 0x8000 (its state 0 or not), where the text goes (0x8100)
    Test file_button_tip{0x9bdd70,
                         [=](Rng &r) {
                             fresh(r);
                             for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x8000)[i] = r.next();
                             for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                             arena<uint32_t>(0x8000)[0x2fc / 4] = r.range(0, 1) ? 0 : r.next();
                             arena<uint8_t>(0x8000)[0x20a] = (uint8_t)(r.range(0, 1) ? 0 : r.range(1, 255));   // (the fireworks copy's state)
                             arena<uint32_t>(0x8100)[0] = r.next();
                             arena<uint32_t>(0x58)[0] = r.next();
                             arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                         },
                         [](void *fn) {
                             void *back = ((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x8000), arena<void>(0x8100));
                             return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ *arena<uint32_t>(0x20)};
                         }};
    file_button_tip.iterations = 200;
    file_button_tip.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)msg_alloc}, {0x4040f0, (void *)ds_named}, {0xbcf7d0, (void *)dn_wide}, {0xbd0490, (void *)ds_empty},
                       {0xbd1880, (void *)dn_lookup_gone}});
    };
    file_button_tip.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(file_button_tip);
    // Designer::OpenFiles (two copies): the designer at arena 0x8000 (its folder's name empty or not), a value
    Test open_files{0xad6460,
                    [=](Rng &r) {
                        fresh(r);
                        for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x8000)[i] = r.next();
                        for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                        of_designer_table[0xe4 / 4] = (void *)of_told;
                        arena<uint32_t>(0x8000)[0] = (uint32_t)(uintptr_t)of_designer_table;
                        uint32_t *folder = (uint32_t *)dev_take(0x10);
                        folder[0] = r.range(1, 2), folder[1] = r.range(0, 1) ? 5381 : r.next(), folder[3] = 0x7300;   // (5381: the empty text's hash)
                        arena<uint32_t>(0x8000)[0xec / 4] = (uint32_t)(uintptr_t)folder;
                        arena<uint32_t>(0x8000)[0xe8 / 4] = (uint32_t)(uintptr_t)folder;
                        arena<uint32_t>(0x8300)[0] = r.next(), arena<uint32_t>(0x8300)[1] = r.next();
                        arena<uint32_t>(0x40)[0] = r.next();
                        arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2400;
                    },
                    [](void *fn) {
                        ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x8000), arena<uint32_t>(0x40)[0]);
                        return Result{*arena<uint32_t>(0x20)};
                    }};
    open_files.iterations = 200;
    open_files.prepare = [](const rewrite::Entry *e) {
        for (auto &row : of_rows)
            if (row[0] == e->at)
                stubs_for_now({{0xbcdfc0, (void *)msg_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)msg_text_c},
                               {0xbce970, (void *)fp_compare}, {row[4], (void *)of_make}, {0x6b1640, (void *)of_layout}, {row[5], (void *)of_give},
                               {row[6], (void *)of_build}});
    };
    open_files.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(open_files);
    // Pairs::Insert (two copies): the index at arena 0x8000 (six arrays at 0x8100 on, 64 entries each), 0 to 40 pairs,
    // room for them or a few more (no room: the arrays are made again from the aligned allocation stand-in), pairs of
    // small numbers so finding one is common; the sorted arrays mostly in order. The result and the arena are compared.
    t.push_back({0x85a620,
                 [=](Rng &r) {
                     fresh(r);
                     uint32_t *self = arena<uint32_t>(0x8000);
                     for (int k = 0; k < 6; k++) self[k] = (uint32_t)(uintptr_t)arena<void>(0x8100 + 0x100 * k);
                     int32_t count = r.range(0, 40);
                     self[6] = (uint32_t)count, self[7] = (uint32_t)(count + (r.range(0, 2) ? r.range(1, 4) : 0));
                     int32_t a = 0, b = 0;
                     for (int i = 0; i < 64; i++) {
                         if (r.range(0, 3)) b += r.range(0, 2);
                         else a += 1, b = r.range(0, 5);
                         arena<int32_t>(0x8100)[i] = r.next(), arena<int32_t>(0x8200)[i] = r.range(0, 5), arena<int32_t>(0x8400)[i] = r.range(0, 5);
                         arena<int32_t>(0x8300)[i] = r.range(0, 9) ? a : r.range(0, 9);   // (sorted a, mostly in order)
                         arena<int32_t>(0x8500)[i] = b;
                         arena<int32_t>(0x8600)[i] = r.range(0, 60);
                     }
                     arena<int32_t>(0x40)[0] = r.range(-1, a + 1), arena<int32_t>(0x40)[1] = r.range(-1, 6);
                     *arena<uint32_t>(ALLOC_NEXT) = 0;
                 },
                 [](void *fn) {
                     int32_t got = ((int32_t(__thiscall *)(void *, int32_t, int32_t))fn)(arena<void>(0x8000), arena<int32_t>(0x40)[0], arena<int32_t>(0x40)[1]);
                     return Result{(uint32_t)got};
                 },
                 3000});
    // Colours::Fill / Fill256: both tables (0x13F7188, 2,976 entries in all) filled from a seed before the call; the
    // result is what the call gives back and a sum over the tables after it. The tables are put back after the test.
    static std::vector<uint32_t> colours_saved;
    auto colours_call = [](void *fn) {
        uint32_t *table = (uint32_t *)0x13f7188;
        uint32_t x = *arena<uint32_t>(0x10);
        for (int i = 0; i < 2976; i++) table[i] = x = x * 1103515245u + 12345u;
        uint32_t back = (uint32_t)(uintptr_t)((void *(__cdecl *)())fn)();
        uint32_t sum = back;
        for (int i = 0; i < 2976; i++) sum = (sum ^ table[i]) * 0x01000193u;
        return Result{sum};
    };
    Test colours_fill{0xf05ca0, [=](Rng &r) { *arena<uint32_t>(0x10) = r.next(); }, colours_call};
    colours_fill.iterations = 20;
    colours_fill.prepare = [](const rewrite::Entry *) { colours_saved.assign((uint32_t *)0x13f7188, (uint32_t *)0x13f7188 + 2976); };
    colours_fill.finish = [](const rewrite::Entry *) { memcpy((void *)0x13f7188, colours_saved.data(), 2976 * 4); };
    quick(colours_fill);
    colours_fill.at = 0xf047d0;
    quick(colours_fill);
    // Matrix::LookAlong* (six copies): a matrix (0x100), a direction (0x300) and up (0x380), any floats; sometimes
    // the direction is zero (or tiny), up a multiple of it (the cross product zero), or the direction one of the
    // matrix's own rows (0x10 holds which)
    Test look_along{0xbed970,
                    [=](Rng &r) {
                        fresh(r);
                        for (int i = 0; i < 16; i++) arena<float>(0x100)[i] = r.any_float();
                        for (int i = 0; i < 4; i++) arena<float>(0x300)[i] = r.any_float(), arena<float>(0x380)[i] = r.any_float();
                        const int mode = r.range(0, 7);
                        if (mode == 1) for (int i = 0; i < 3; i++) arena<float>(0x300)[i] = r.next() % 2 ? 0.0f : r.real(-1e-30f, 1e-30f);
                        if (mode == 2) { const float k = r.real(-4, 4); for (int i = 0; i < 3; i++) arena<float>(0x380)[i] = arena<float>(0x300)[i] * k; }
                        arena<uint32_t>(0x10)[0] = mode == 3 ? 0x100 + 0x10 * r.range(0, 2) : 0x300;
                    },
                    [](void *fn) {
                        return Result{((uint32_t(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x100), arena<void>(arena<uint32_t>(0x10)[0]), arena<void>(0x380))};
                    }};
    for (uint32_t at : {0xbed970u, 0xbed820u, 0xbed590u, 0xbed440u, 0xbedac0u, 0xbed6e0u}) {
        look_along.at = at;
        quick(look_along);
    }
    // Skeleton::PartMatrix: a bone (0x100: its matrices any floats), its part (0x800, matrix at +0xC0), out (0x400)
    Test skeleton_part_matrix{0xc22d30,
                              [=](Rng &r) {
                                  fresh(r);
                                  for (int i = 0; i < 0x40; i++) arena<float>(0x100)[i] = r.any_float();
                                  for (int i = 0; i < 16; i++) arena<float>(0x8c0)[i] = r.any_float();
                                  *(void **)arena<uint8_t>(0x100 + 0x54) = arena<void>(0x800);
                                  *(void **)arena<uint8_t>(0x100 + 0x64) = arena<void>(0x400);
                              },
                              [](void *fn) { return Result{((uint32_t(__thiscall *)(void *))fn)(arena<void>(0x100))}; }};
    quick(skeleton_part_matrix);
    // Skeleton::UpdateParts: a skeleton (0x100) -> 0x180 -> object 0x200 whose +0x58 starts a list of up to three bones
    // (0x400, 0x600, 0x800; each one's part the one before, the first's the object), outs at 0xA00 on
    Test skeleton_update_parts{0xc286d0,
                               [=](Rng &r) {
                                   fresh(r);
                                   *(void **)arena<uint8_t>(0x110) = arena<void>(0x180);
                                   *(void **)arena<uint8_t>(0x180) = arena<void>(0x200);
                                   for (int i = 0; i < 16; i++) arena<float>(0x2c0)[i] = r.any_float();
                                   const int bones = r.range(0, 3);
                                   uint32_t prev = 0x200;
                                   for (int k = 0; k < bones; k++) {
                                       const uint32_t at = 0x400 + 0x200 * k;
                                       for (int i = 4; i < 0x40; i++) arena<float>(at)[i] = r.any_float();
                                       *(void **)arena<uint8_t>(prev + 0x58) = arena<void>(at);
                                       *(void **)arena<uint8_t>(at + 0x54) = arena<void>(prev);
                                       *(void **)arena<uint8_t>(at + 0x64) = arena<void>(0xa00 + 0x40 * k);
                                       prev = at;
                                   }
                                   *(void **)arena<uint8_t>(prev + 0x58) = nullptr;
                               },
                               [](void *fn) { return Result{((uint32_t(__thiscall *)(void *))fn)(arena<void>(0x100))}; }};
    quick(skeleton_update_parts);
    // Effect::SetMatrix: an effect (0x100, its matrices any floats) and a matrix of any floats; which 0-5 (4, 5: none)
    Test effect_set_matrix{0xc957e0,
                           [=](Rng &r) {
                               fresh(r);
                               for (uint32_t s : {0x670u, 0x2090u, 0x9f0u})
                                   for (int i = 0; i < 16; i++) *(float *)arena<uint8_t>(0x100 + s + 4 * i) = r.any_float();
                               for (int i = 0; i < 16; i++) arena<float>(0x4000)[i] = r.any_float();
                               *arena<uint32_t>(0x10) = r.range(0, 5);
                           },
                           [](void *fn) {
                               return Result{((uint32_t(__thiscall *)(void *, uint32_t, const void *))fn)(arena<void>(0x100), *arena<uint32_t>(0x10), arena<void>(0x4000))};
                           }};
    quick(effect_set_matrix);
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
#define EQUIV_PART 2
#include "equiv_tests.inc"
#undef EQUIV_PART
    return t;
}

static const char *g_only = getenv("EQUIV_ONLY");   // only the rewrites whose name has one of these (a|b) in it
static bool only_matches(const char *name) {
    for (const char *p = g_only;;) {
        const char *end = strchr(p, '|');
        size_t n = end ? (size_t)(end - p) : strlen(p);
        for (const char *c = name; *c; c++) if (!strncmp(c, p, n)) return true;
        if (!end) return false;
        p = end + 1;
    }
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
        if (e->original_len < 5 || memcmp((void *)e->at, e->original, e->original_len)) {   // as replace_function
            printf("  %-34s 0x%06x  the exe's code there isn't the known version\n", e->name, (unsigned)e->at);
            bad++;
        }
    install_stubs();
    SetUnhandledExceptionFilter(crashed);
    if (getenv("EQUIV_FAULTS"))   // (every access violation as it happens, before the game's own exception frames see it)
        AddVectoredExceptionHandler(1, fault_seen);
    auto all = tests();
    // one line per rewrite; a rewrite of many copies is checked on each copy
    std::vector<const char *> done;
    for (const rewrite::Entry *e = rewrite::list(); e; e = e->next) {
        bool seen = false;
        for (auto n : done) if (!strcmp(n, e->name)) seen = true;
        if (seen) continue;
        done.push_back(e->name);
        if (g_only && !only_matches(e->name)) continue;
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
