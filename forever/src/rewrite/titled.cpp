// Window classes built from a shared title, any base: windows.cpp's Window::Construct with the base class's own
// constructor (and the values it takes after the parent and the text), then the class's member steps, which here also
// make new objects, call members with values, copy globals and mark the title object where the code does.
// tools/titled_ctors.py writes the list (titled_list.inc).
#include "wave.h"

static void titled_release(void *text) {   // a wide string's shared block: freed when no one has it (no check for none)
    int32_t *block = (int32_t *)text;
    if (--*block) return;
    memory_free_aligned((void *)(uintptr_t)block[3]);
    memory_free(block);
}

static void titled_assign(void **holder, void *text) {   // a counted text put into a holder, as the game writes it inline
    if (*holder == text) return;
    int32_t *old = (int32_t *)*holder;
    --*old;
    old = (int32_t *)*holder;
    if (!*old && old) {
        memory_free_aligned((void *)(uintptr_t)old[3]);
        memory_free(old);
    }
    *holder = text;
    ++*(int32_t *)text;
}

// A member step, in the order the code does them:
//   0 dword store, 1 byte store, 4 word store, 2 store the constructor's argument number value, 5 store the global at
//   value as it is then, 6 store a pointer to +value of the object, 7 store the shared empty wide string, 29 store what
//   the constructor's argument number value points to, 30 copy the member at +value, 26 / 27 and / or the member with
//   value;
//   3 call fn on the member at +off (thiscall), 24 call fn on what the member at +off points to;
//   8 the member at +off a reference to the empty handle (0x805750) linked (+8) first in that handle's list, 9 the same
//   with the handle Handle::For (0x805700) gives for the constructor's argument number value (0: none);
//   12 the mark (no parent: the title object's +4 = 1);
//   59 an x87 run (the game's steps, titled_fpu from value) building locals, then the call given them (kind 9: a local's place);
//   60 a virtual call: slot value, on the window + off or (off bit 24) on what that member points to;
//   61 a call on the constructor's argument off; 62 a call on the object the game global value holds;
//   63 the next off steps skipped when the value given is 0 (fn 0) or is not (fn 1);
//   64 the last call's result kept as scratch value off; 65 memory freed (fn: 0xBCE000 / 0xBCE010) with the value given;
//   67 the window + off kept in the game global fn;
//   68 name off made from the wide text given (a copy), or the empty text when it is none;
//   58 the new object (a text made with 0xBCF850) kept as a name (its place given to calls, kind 5);
//   57 a name let go whether or not it was made (as the game: through none, it fails as the game's does);
//   (3 and 24 with value bit 31: made with edx = the window, as the game makes it);
//   54 a scratch value (off) worked out: fn 0, the value given (args[0]) plus value, or 0 for 0; fn 1, its interface for the
//      class numbered by the global value, or 0; 55 a scratch value (value) stored at off;
//   (20 with value 1: a temporary made whether or not a new object was; values given: kind 7 a scratch value, kind 8 a
//      dword a temporary holds, v & 0xff the temporary and v >> 8 where;)
//   (24: value, the place that far into what the member points to);
//   52 the description (given, or the title object) named on its first use (+4 clear): Name::Assign (a name global, the
//      member's place), NameTable::Set, the name let go; fn 1: a later name of the same run, under the first's decision;
//   a new object: 18 value bytes of memory (0xBCDFC0), then only when there was memory: 21 name number off made from
//   the C text at value (0x10 bytes of memory, WideString::Create; null without memory), 20 temporary number off made
//   on the stack by fn, 10 the object built by fn; 22 stores at +off what fn returned (value 0) or the memory (1), null
//   when there was no memory; 31 / 32 / 33 store a dword / the global at value / a byte into the new object (when
//   there was memory: its own constructor inlined);
//   15 destroy temporary number off with fn, 16 let go of the wide string it holds, 17 let go of name number off: each
//   only when it was made.
//   40 a child laid out: the member at +off asked its size (its slot 0x20), then 0x6B1640 on it with the relation
//   (args[4] >> 8), the member at +value and the rectangle {args[0], args[1] (bits), size w +/- the float at args[2],
//   size h +/- the float at args[3]} (bit 0 / bit 1 of args[4]: subtracted), the sums on the FPU stored as floats;
//   bit 2 / bit 3: w / h only passed through the FPU (a signalling NaN comes out quiet), bit 4: w and h copied as they
//   are, bit 5: the window itself instead of the member; bit 6: the rectangle args[0] - args[3] as it is (not asked
//   the size); bit 7: the new object instead of the member (off 0: what its constructor returned, 1: its memory).
//   44 / 45 the dword at +args[0] of what the member at +off points to set to value / the global at value as it is then.
//   47 / 48 or / and the dword at +args[0] of what the member at +off points to with value.
//   49 the byte at +off set to the constructor's argument number value's low byte.
//   51 a game global (at fn) set to value: a byte (off 1) or a dword.
//   50 one more hold on the shared text the constructor's argument number value holds (its count, at the text's start).
//   46 call fn on the new object: what its constructor returned (off 0) or its memory (1); with value 1 only when there
//   was memory, else always (on null when there was none).
//   41 the dword at +args[1] of the member at +args[0] copied to +value of the member at +off.
//   42 value bytes of aligned memory (0xBCDFD0), 43 the last of them stored at +off.
// The values given to fn are by kind (4 bits each in kinds): 0 the value, 1 the global there, 2 the constructor's
// argument number value, 3 a pointer to +value of the object, 4 what temporary number value's maker returned, 5 where
// name number value is kept, 6 the member at +value.
struct TitledOp {
    uint32_t kind, off, value, fn, count, args[7], kinds;
};
static void fpu_pass(const float *from, volatile uint32_t *to) {   // a float loaded onto the FPU and stored again
    __asm__ volatile("flds %1\n\tfstps %0" : "=m"(*to) : "m"(*from));
}
// A value the base takes after (parent or title, text): a constant, or the constructor's argument number value.
struct TitledExtra {   // is_arg: 0 the value, 1 argument value, 2 a text reference made just before (titled_refs), 3 the start text's place
    uint32_t is_arg, value;
};
extern const uint32_t titled_refs[];

static void *titled_call(uintptr_t fn, void *self, uint32_t count, const uint32_t *a) {
    switch (count) {
    case 0: return ((void *(__thiscall *)(void *))fn)(self);
    case 1: return ((void *(__thiscall *)(void *, uint32_t))fn)(self, a[0]);
    case 2: return ((void *(__thiscall *)(void *, uint32_t, uint32_t))fn)(self, a[0], a[1]);
    case 3: return ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))fn)(self, a[0], a[1], a[2]);
    case 4: return ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))fn)(self, a[0], a[1], a[2], a[3]);
    case 5: return ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))fn)(self, a[0], a[1], a[2], a[3], a[4]);
    case 6: return ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))fn)(self, a[0], a[1], a[2], a[3], a[4], a[5]);
    default: return ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))fn)(self, a[0], a[1], a[2], a[3], a[4], a[5], a[6]);
    }
}

// the same, made with edx = the window being made (a call the game makes with it there, the callee reading it)
static void *titled_call_edx(uintptr_t fn, void *self, void *window, uint32_t count, const uint32_t *a) {
    switch (count) {
    case 0: return ((void *(__fastcall *)(void *, void *))fn)(self, window);
    case 1: return ((void *(__fastcall *)(void *, void *, uint32_t))fn)(self, window, a[0]);
    case 2: return ((void *(__fastcall *)(void *, void *, uint32_t, uint32_t))fn)(self, window, a[0], a[1]);
    case 3: return ((void *(__fastcall *)(void *, void *, uint32_t, uint32_t, uint32_t))fn)(self, window, a[0], a[1], a[2]);
    case 4: return ((void *(__fastcall *)(void *, void *, uint32_t, uint32_t, uint32_t, uint32_t))fn)(self, window, a[0], a[1], a[2], a[3]);
    default: return ((void *(__fastcall *)(void *, void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))fn)(self, window, a[0], a[1], a[2], a[3], a[4]);
    }
}

// An x87 run (op 59): the game's own steps, replayed one by one, so every value (and which NaN comes out of two) is the
// game's. The FPU stack is carried from one step to the next (nothing between them uses it: the loop is integers only).
// Each step {code, kind, value, local}; an operand: kind 1 a game global, 2 the window's (self + value), 3 in what a member
// points to (*(self + value & 0xffff) + (value >> 16)), 4 a local (loc[value]), 5 the constructor's argument.
extern const uint32_t titled_fpu[];
#define X87(text) __asm__ volatile(".intel_syntax noprefix\n\t" text "\n\t.att_syntax prefix" : : "a"(p) : "memory")
#define X87I(text) __asm__ volatile(".intel_syntax noprefix\n\t" text "\n\t.att_syntax prefix" : : : "memory")
__attribute__((noinline)) static void titled_x87(const uint32_t *step, uint32_t count, uint8_t *self, const uint32_t *given, uint32_t *loc) {
    uint8_t *size = nullptr;   // what a size query (code 25) gave: operand kind 6 reads through it
    for (uint32_t k = 0; k < count; k++, step += 4) {
        const uint32_t code = step[0], kind = step[1], v = step[2];
        if (code == 25) {   // a child's size asked for: its table's slot 0x20 (the window + v, or what that member points to), a local to fill
            uint8_t *child = kind ? *(uint8_t **)(self + v) : self + v;
            size = ((uint8_t * (__thiscall *)(void *, void *))(*(void ***)child)[0x20 / 4])(child, &loc[step[3]]);
            continue;
        }
        uint32_t *p = kind == 1 ? (uint32_t *)(uintptr_t)v
                      : kind == 2 ? (uint32_t *)(self + v)
                      : kind == 3 ? (uint32_t *)(*(uint8_t **)(self + (v & 0xffff)) + (v >> 16))
                      : kind == 4 ? &loc[v]
                      : kind == 5 ? (uint32_t *)&given[v]
                      : kind == 6 ? (uint32_t *)(size + v) : nullptr;
        switch (code) {
        case 1: X87("fld dword ptr [eax]"); break;
        case 21: X87("fild dword ptr [eax]"); break;
        case 3: X87("fst dword ptr [eax]"); break;
        case 4: X87("fstp dword ptr [eax]"); break;
        case 5: X87("fadd dword ptr [eax]"); break;
        case 6: X87("fsub dword ptr [eax]"); break;
        case 7: X87("fsubr dword ptr [eax]"); break;
        case 8: X87("fmul dword ptr [eax]"); break;
        case 9: X87("fdiv dword ptr [eax]"); break;
        case 10: X87("fdivr dword ptr [eax]"); break;
        case 11: X87I("faddp st(1), st"); break;
        case 12: X87I("fsubp st(1), st"); break;
        case 13: X87I("fsubrp st(1), st"); break;
        case 14: X87I("fmulp st(1), st"); break;
        case 15: X87I("fdivp st(1), st"); break;
        case 16: X87I("fdivrp st(1), st"); break;
        case 18: X87I("fchs"); break;
        case 22: X87I("fstp st(0)"); break;
        case 23: X87I("fld1"); break;
        case 24: X87I("fldz"); break;
        case 19: loc[step[3]] = *p; break;   // an integer copy into a local
        case 20: loc[step[3]] = v; break;    // a constant into a local
        case 2:
            switch (v) {
            case 0: X87I("fld st(0)"); break;
            case 1: X87I("fld st(1)"); break;
            case 2: X87I("fld st(2)"); break;
            default: X87I("fld st(3)"); break;
            }
            break;
        case 17:
            switch (v) {
            case 1: X87I("fxch st(1)"); break;
            case 2: X87I("fxch st(2)"); break;
            default: X87I("fxch st(3)"); break;
            }
            break;
        default: {   // 30..35: st(0) = st(0) op st(i); 40..45: st(i) = st(i) op st(0) (add, sub, subr, mul, div, divr), i 1..3
            const uint32_t which = code % 10 * 4 + (v - 1) + (code >= 40 ? 24 : 0);
            switch (which) {
#define ROW(n, text) case n: X87I(text); break;
                ROW(0, "fadd st(0), st(1)") ROW(1, "fadd st(0), st(2)") ROW(2, "fadd st(0), st(3)")
                ROW(4, "fsub st(0), st(1)") ROW(5, "fsub st(0), st(2)") ROW(6, "fsub st(0), st(3)")
                ROW(8, "fsubr st(0), st(1)") ROW(9, "fsubr st(0), st(2)") ROW(10, "fsubr st(0), st(3)")
                ROW(12, "fmul st(0), st(1)") ROW(13, "fmul st(0), st(2)") ROW(14, "fmul st(0), st(3)")
                ROW(16, "fdiv st(0), st(1)") ROW(17, "fdiv st(0), st(2)") ROW(18, "fdiv st(0), st(3)")
                ROW(20, "fdivr st(0), st(1)") ROW(21, "fdivr st(0), st(2)") ROW(22, "fdivr st(0), st(3)")
                ROW(24, "fadd st(1), st(0)") ROW(25, "fadd st(2), st(0)") ROW(26, "fadd st(3), st(0)")
                ROW(28, "fsub st(1), st(0)") ROW(29, "fsub st(2), st(0)") ROW(30, "fsub st(3), st(0)")
                ROW(32, "fsubr st(1), st(0)") ROW(33, "fsubr st(2), st(0)") ROW(34, "fsubr st(3), st(0)")
                ROW(36, "fmul st(1), st(0)") ROW(37, "fmul st(2), st(0)") ROW(38, "fmul st(3), st(0)")
                ROW(40, "fdiv st(1), st(0)") ROW(41, "fdiv st(2), st(0)") ROW(42, "fdiv st(3), st(0)")
                ROW(44, "fdivr st(1), st(0)") ROW(45, "fdivr st(2), st(0)") ROW(46, "fdivr st(3), st(0)")
#undef ROW
            }
            break;
        }
        }
    }
}
#undef X87
#undef X87I

static volatile long g_titled_constructs;
// given[1..]: the constructor's arguments, (parent, text, ...)
template <uintptr_t TITLE, uintptr_t BASE, const TitledExtra *EXTRAS, uint32_t EXTRA_COUNT, const TitledOp *OPS, uint32_t COUNT, uint32_t PARENT, uint32_t TEXT>
static void *titled_body(uint8_t *self, const uint32_t *given) {
    g_titled_constructs++;
    void *parent = (void *)(uintptr_t)given[PARENT];   // (the parent's and the text's arguments: the first two, or others)
    void **text = (void **)(uintptr_t)given[TEXT];
    uint8_t *title = *(uint8_t **)TITLE;
    void *made = nullptr;   // the copy of the title when there's no text
    bool copied = false;
    void *used;
    void *start_name = nullptr;   // a text made from a literal first of all (its place given to the base; name 0 after)
    bool has_start_name = false;
    for (uint32_t i = 0; i < EXTRA_COUNT; i++)
        if (EXTRAS[i].is_arg == 3) {
            void *memory = memory_alloc(0x10);
            start_name = memory ? ((void *(__thiscall *)(void *, const char *))0xbcf850)(memory, (const char *)(uintptr_t)EXTRAS[i].value) : nullptr;
            has_start_name = true;
        }
    if (text) {
        void *name;
        const char *title_name = *(const char **)(title + 8);
        if (title_name) {
            void *memory = memory_alloc(0x10);
            name = memory ? ((void *(__thiscall *)(void *, const char *))0xbcf850)(memory, title_name) : nullptr;
        } else {
            name = ((void *(__fastcall *)(uint32_t))0xbd0490)(1);
        }
        void *with_separator = nullptr, *joined = nullptr;
        void *first = ((void *(__thiscall *)(void *, void **, const char *))0x403be0)(&name, &with_separator, (const char *)0xf2738c);
        void **result = (void **)((void *(__thiscall *)(void *, void **, void **))0x403bb0)(first, &joined, text);
        if (*result != *text) {
            int32_t *old = (int32_t *)*text;
            --*old;
            old = (int32_t *)*text;
            if (!*old && old) {
                memory_free_aligned((void *)(uintptr_t)old[3]);
                memory_free(old);
            }
            *text = *result;
            ++*(int32_t *)*result;
        }
        titled_release(joined);
        titled_release(with_separator);
        titled_release(name);
        used = text;
    } else {
        used = ((void *(__thiscall *)(void **, void *))0x41c740)(&made, title);
        copied = true;
    }
    uint32_t base_args[6] = {(uint32_t)(uintptr_t)(parent ? parent : *(void **)TITLE), (uint32_t)(uintptr_t)used};
    alignas(16) uint8_t reference[0x40];   // a text reference made just before the base (is_arg 2), destroyed just after
    void *reference_made = nullptr;
    for (uint32_t i = 0; i < EXTRA_COUNT; i++)
        if (EXTRAS[i].is_arg == 2) {
            const uint32_t *r = &titled_refs[3 * EXTRAS[i].value];
            reference_made = ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))0x41cb90)(reference, r[0], r[1], r[2]);
        }
    for (uint32_t i = 0; i < EXTRA_COUNT; i++)
        base_args[2 + i] = EXTRAS[i].is_arg == 1 ? given[EXTRAS[i].value]
                           : EXTRAS[i].is_arg == 2 ? (uint32_t)(uintptr_t)reference_made
                           : EXTRAS[i].is_arg == 3 ? (uint32_t)(uintptr_t)&start_name : EXTRAS[i].value;
    titled_call(BASE, self, 2 + EXTRA_COUNT, base_args);
    if (reference_made) ((void(__thiscall *)(void *))0xbd1880)(reference);
    if (copied) titled_release(made);
    alignas(16) uint8_t temps[16][0x40];   // temporaries on the stack (texts made for a new object's constructor; titled_ctors.py keeps to 16)
    void *temp_made[16] = {}, *names[16] = {};
    bool temp_built[16] = {}, name_built[16] = {};
    names[0] = start_name, name_built[0] = has_start_name;   // (the start text: name 0, let go by the steps)
    uint8_t *memory = nullptr;
    bool naming = false;
    uint32_t scratch[16] = {};
    void *last = nullptr;   // what the last call (3, 24, 60, 61, 62) gave back
    uint32_t loc[0x44] = {};   // the x87 runs' locals (by place below the constructor's entry, as dwords)
    void *object = nullptr, *aligned = nullptr;
    for (uint32_t k = 0; k < COUNT; k++) {
        const TitledOp &op = OPS[k];
        uint32_t args[7];
        for (uint32_t i = 0; i < op.count; i++) {
            uint32_t v = op.args[i];
            switch (op.kinds >> (4 * i) & 15) {
            case 0: args[i] = v; break;
            case 1: args[i] = *(uint32_t *)(uintptr_t)v; break;
            case 2: args[i] = given[v]; break;
            case 3: args[i] = (uint32_t)(uintptr_t)(self + v); break;
            case 4: args[i] = (uint32_t)(uintptr_t)temp_made[v]; break;
            case 5: args[i] = (uint32_t)(uintptr_t)&names[v]; break;
            case 7: args[i] = scratch[v]; break;
            case 8: args[i] = *(uint32_t *)((uint8_t *)temp_made[v & 0xff] + (v >> 8)); break;
            case 9: args[i] = (uint32_t)(uintptr_t)&loc[v]; break;
            case 10: args[i] = *(uint32_t *)(self + (v & 0xffff)) + (v >> 16); break;   // a place inside what a member points to
            case 11: args[i] = *(uint32_t *)(*(uint8_t **)(self + (v & 0xffff)) + (v >> 16)); break;   // a dword inside what a member points to
            case 12: args[i] = *(uint32_t *)((uint8_t *)(uintptr_t)given[v & 0xff] + (v >> 8)); break;   // a dword inside what an argument points to
            case 13: args[i] = v ? (uint32_t)(uintptr_t)memory : (uint32_t)(uintptr_t)object; break;   // the new object (as returned / its memory)
            default: args[i] = *(uint32_t *)(self + v); break;
            }
        }
        switch (op.kind) {
        case 0: *(uint32_t *)(self + op.off) = op.value; break;
        case 1: self[op.off] = (uint8_t)op.value; break;
        case 4: *(uint16_t *)(self + op.off) = (uint16_t)op.value; break;
        case 2: *(uint32_t *)(self + op.off) = given[op.value]; break;
        case 5: *(uint32_t *)(self + op.off) = *(uint32_t *)(uintptr_t)op.value; break;
        case 6: *(uint8_t **)(self + op.off) = self + op.value; break;
        case 7: *(void **)(self + op.off) = ((void *(__fastcall *)(uint32_t))0xbd0490)(1); break;
        case 29: *(uint32_t *)(self + op.off) = *(uint32_t *)(uintptr_t)given[op.value]; break;
        case 30: *(uint32_t *)(self + op.off) = *(uint32_t *)(self + op.value); break;
        case 41: *(uint32_t *)(*(uint8_t **)(self + op.off) + op.value) = *(uint32_t *)(*(uint8_t **)(self + op.args[0]) + op.args[1]); break;
        case 40: {
            const uint32_t how_ = op.args[4];
            uint8_t *child = how_ & 128 ? (op.off ? memory : (uint8_t *)object) : *(uint8_t **)(self + op.off);
            volatile uint32_t rect[4];
            const uint32_t how = op.args[4];
            rect[0] = op.args[0], rect[1] = op.args[1];
            uint8_t scratch[0x10];
            const float *size = how & 64 ? nullptr : (const float *)((void *(__thiscall *)(void *, void *))(*(void ***)child)[0x20 / 4])(child, scratch);
            for (int i = 0; i < 2; i++) {
                if (how & 64) {
                    rect[2 + i] = op.args[2 + i];
                } else if (how & 16) {
                    rect[2 + i] = ((const uint32_t *)size)[i];
                } else if (how & (4 << i)) {
                    fpu_pass(&size[i], &rect[2 + i]);
                } else {
                    const long double v = (long double)size[i], k = *(const float *)(uintptr_t)op.args[2 + i];
                    volatile float sum = how & (1 << i) ? v - k : v + k;
                    rect[2 + i] = *(volatile uint32_t *)&sum;
                }
            }
            ((void(__thiscall *)(void *, uint32_t, void *, volatile uint32_t *))0x6b1640)(child, how >> 8, how & 32 ? (void *)self : *(void **)(self + op.value), rect);
            break;
        }
        case 47: *(uint32_t *)(*(uint8_t **)(self + op.off) + op.args[0]) |= op.value; break;
        case 48: *(uint32_t *)(*(uint8_t **)(self + op.off) + op.args[0]) &= op.value; break;
        case 49: self[op.off] = (uint8_t)given[op.value]; break;
        case 50: ++**(int32_t **)(uintptr_t)given[op.value]; break;
        case 51:
            if (op.off == 1) *(volatile uint8_t *)(uintptr_t)op.fn = (uint8_t)op.value;
            else *(volatile uint32_t *)(uintptr_t)op.fn = op.value;
            break;
        case 46:
            if (memory || !op.value) titled_call(op.fn, op.off ? (void *)memory : object, op.count, args);
            break;
        case 44: *(uint32_t *)(*(uint8_t **)(self + op.off) + op.args[0]) = op.value; break;
        case 45: *(uint32_t *)(*(uint8_t **)(self + op.off) + op.args[0]) = *(uint32_t *)(uintptr_t)op.value; break;
        case 26: *(uint32_t *)(self + op.off) &= op.value; break;
        case 27: *(uint32_t *)(self + op.off) |= op.value; break;
        case 3:
            if (op.value >> 31) last = titled_call_edx(op.fn, self + op.off, self, op.count, args);
            else last = titled_call(op.fn, self + op.off, op.count, args);
            break;
        case 24:
            if (op.value >> 31) last = titled_call_edx(op.fn, *(uint8_t **)(self + op.off) + (op.value & 0x7fffffff), self, op.count, args);
            else last = titled_call(op.fn, *(uint8_t **)(self + op.off) + op.value, op.count, args);
            break;
        case 64: scratch[op.off] = (uint32_t)(uintptr_t)last; break;   // the last call's result kept
        case 71: *(uint32_t *)(self + op.off) = args[0]; break;   // a dword read inside what an argument points to (kind 12) stored
        case 67: *(uint8_t *volatile *)(uintptr_t)op.fn = self + op.off; break;   // the window's place kept in a game global
        case 65: ((void(__cdecl *)(void *))(uintptr_t)op.fn)((void *)(uintptr_t)args[0]); break;   // memory freed
        case 59: {   // an x87 run (titled_fpu from value), then the call given what it built; on the window + off, or (off bit 24)
                     // what that member points to
            titled_x87(&titled_fpu[op.value + 4], titled_fpu[op.value], self, given, loc);
            uint8_t *on = op.off >> 24 ? *(uint8_t **)(self + (op.off & 0xffffff)) : self + op.off;
            titled_call(op.fn, on, op.count, args);
            break;
        }
        case 60: {   // a virtual call (slot value) on the window + off, or (off bit 24) on what that member points to
            uint8_t *on = op.off >> 24 ? *(uint8_t **)(self + (op.off & 0xffffff)) : self + op.off;
            last = titled_call((uintptr_t)(*(void ***)on)[op.value / 4], on, op.count, args);
            break;
        }
        case 61: last = titled_call(op.fn, (void *)(uintptr_t)given[op.off], op.count, args); break;              // on an argument
        case 62: last = titled_call(op.fn, *(void *volatile *)(uintptr_t)op.value, op.count, args); break;   // on a global's object
        case 83: {   // a virtual call (slot value) on the object argument number off is
            uint8_t *on = (uint8_t *)(uintptr_t)given[op.off];
            last = titled_call((uintptr_t)(*(void ***)on)[op.value / 4], on, op.count, args);
            break;
        }
        case 82: {   // a virtual call (slot value) on the object the global at off holds
            uint8_t *on = *(uint8_t *volatile *)(uintptr_t)op.off;
            last = titled_call((uintptr_t)(*(void ***)on)[op.value / 4], on, op.count, args);
            break;
        }
        case 63: {   // a block of off steps skipped when the value is 0 (fn 0) or when it isn't (fn 1); fn 2: when its low byte is 0;
                     // fn 0x10 | n: when the value compares with the constant (value) as the n-th of ja jae jb jbe jg jge jl jle je jne
            if (op.fn >= 0x10) {
                const uint32_t a = args[0], b = op.value;
                const int32_t sa = (int32_t)a, sb = (int32_t)b;
                const bool skip[10] = {a > b, a >= b, a < b, a <= b, sa > sb, sa >= sb, sa < sb, sa <= sb, a == b, a != b};
                if (skip[op.fn & 0xf]) k += op.off;
                break;
            }
            const bool zero = op.fn >= 2 ? (args[0] & 0xff) == 0 : args[0] == 0;   // (fn 3: skipped when the low byte isn't 0)
            if (zero != (op.fn == 1 || op.fn == 3)) k += op.off;
            break;
        }
        case 68: {   // a text copied from the wide text given (0xBCF7D0; none without memory), or the empty text when there is none: name off
            void *from = (void *)(uintptr_t)args[0];
            if (from) {
                void *block = memory_alloc(0x10);
                names[op.off] = block ? ((void *(__thiscall *)(void *, void *))0xbcf7d0)(block, from) : nullptr;
            } else {
                names[op.off] = ((void *(__fastcall *)(uint32_t))0xbd0490)(1);
            }
            name_built[op.off] = true;
            break;
        }
        case 57: titled_release(names[op.off]); break;   // (made or not: the game reads through it either way)
        case 58:   // the new object (a text, or none without memory) kept as a name
            names[op.off] = object;
            name_built[op.off] = true;
            break;
        case 8:
        case 9: {
            uint8_t *handle = op.kind == 8 ? ((uint8_t * (__cdecl *)())0x805750)()
                                           : ((uint8_t * (__thiscall *)(void *))0x805700)(op.value ? (void *)(uintptr_t)given[op.value] : nullptr);
            *(uint8_t **)(self + op.off + 4) = handle;
            uint32_t *link = (uint32_t *)(self + op.off + 8), *head = *(uint32_t **)(handle + 0x20);
            link[0] = head[0];
            ((uint32_t *)(uintptr_t)head[0])[1] = (uint32_t)(uintptr_t)link;
            link[1] = (uint32_t)(uintptr_t)head;
            head[0] = (uint32_t)(uintptr_t)link;
            break;
        }
        case 12:
            if (!parent) (*(uint8_t **)TITLE)[4] = 1;
            break;
        case 54: {   // a value worked out: fn 0, the base plus value (none for none); fn 1, the base's interface for the class
                     // whose number is in the global value (none for none, for no class info, a number past its count or no entry)
            uint8_t *base = (uint8_t *)(uintptr_t)args[0];
            uint32_t got = 0;
            if (base && !op.fn) got = args[0] + op.value;
            else if (base) {
                uint8_t *info = ((uint8_t * (__thiscall *)(void *))(*(void ***)base)[1])(base);
                if (info) {
                    const int32_t number = *(volatile int32_t *)(uintptr_t)op.value;
                    if (number < *(int32_t *)(info + 0x18)) {
                        uint8_t *entry = (*(uint8_t ***)(info + 0x14))[number];
                        if (entry) got = args[0] + *(uint32_t *)(entry + 0x10);
                    }
                }
            }
            scratch[op.off] = got;
            break;
        }
        case 55: *(uint32_t *)(self + op.off) = scratch[op.value]; break;
        case 52: {   // the description (given, or the default) named on its first use: a name (Name::Assign with the global and the
                     // member's place) set in its table (NameTable::Set), then let go; the first of a run decides for all
            uint8_t *described = parent ? (uint8_t *)parent : *(uint8_t **)TITLE;
            if (!op.fn) naming = !described[4];
            if (naming) {
                alignas(16) uint8_t name[0x40];
                void *made = ((void *(__thiscall *)(void *, uintptr_t, uint32_t))0xbeca40)(name, op.value, op.off);
                ((void(__thiscall *)(void *, void *))0xbecc10)(described, made);
                ((void(__thiscall *)(void *))0xbec5e0)(name);
            }
            break;
        }
        case 18:
            memory = (uint8_t *)memory_alloc(op.value);
            object = nullptr;
            break;
        case 21:
            if (memory) {
                void *block = memory_alloc(0x10);
                names[op.off] = block ? ((void *(__thiscall *)(void *, const char *))0xbcf850)(block, (const char *)(uintptr_t)op.value) : nullptr;
                name_built[op.off] = true;
            }
            break;
        case 20:
            if (memory || op.value) {
                temp_made[op.off] = titled_call(op.fn, temps[op.off], op.count, args);
                temp_built[op.off] = true;
            }
            break;
        case 10:
            if (memory) object = titled_call(op.fn, memory, op.count, args);
            break;
        case 22: *(void **)(self + op.off) = op.value ? (void *)memory : object; break;
        case 42: aligned = memory_alloc_aligned(op.value); break;
        case 43: *(void **)(self + op.off) = aligned; break;
        case 31: if (memory) *(uint32_t *)(memory + op.off) = op.value; break;
        case 32: if (memory) *(uint32_t *)(memory + op.off) = *(uint32_t *)(uintptr_t)op.value; break;
        case 33: if (memory) memory[op.off] = (uint8_t)op.value; break;
        case 72: if (memory) *(uint32_t *)(memory + op.off) = args[0]; break;
        case 78: scratch[op.off] = *(uint32_t *)(uintptr_t)(args[0] + op.value); break;   // a dword inside a scratch value's object
        case 79: scratch[op.off] = args[0] + op.value; break;   // a scratch value plus a constant
        case 80: last = titled_call(op.fn, (void *)(uintptr_t)args[0], op.count - 1, args + 1); break;   // a call on a scratch value
        case 76: last = titled_call(op.fn, nullptr, op.count, args); break;
        case 81: titled_release((void *)(uintptr_t)args[0]); break;   // a counted text let go (from a member, a stack slot, a register)   // a plain function (it never reads ecx), what it gives kept
        case 77: {   // a shared object: the one the global args[0] holds, else made by fn (taking nothing); args[1]: cast to its part at
                     // +value (none for none)
            uint32_t got = *(volatile uint32_t *)(uintptr_t)args[0];
            if (!got) got = (uint32_t)(uintptr_t)titled_call(op.fn, nullptr, 0, args);
            scratch[op.off] = args[1] ? (got ? got + op.value : 0) : got;
            break;
        }
        case 75: titled_assign((void **)(uintptr_t)args[0], op.fn ? names[op.value] : (void *)(uintptr_t)args[1]); break;   // a counted text into a holder
        case 74: *(uint32_t *)(*(uint8_t **)(self + op.off) + op.value) = args[0]; break;   // a value into what a member points to (a child)
        case 73:   // a constant into the object a game global points to (args[0]: its size, | 0x100 only when there was memory)
            if (!(args[0] & 0x100) || memory) {
                uint8_t *o = *(uint8_t **)(uintptr_t)op.fn;
                if ((args[0] & 0xff) == 1) o[op.off] = (uint8_t)op.value;
                else *(uint32_t *)(o + op.off) = op.value;
            }
            break;   // a value (the window, an argument, a member) into the new object
        case 15:
            if (temp_built[op.off]) ((void(__thiscall *)(void *))(uintptr_t)op.fn)(temps[op.off]);
            temp_built[op.off] = false;
            break;
        case 16:
            if (temp_built[op.off]) titled_release(*(void **)temps[op.off]);
            temp_built[op.off] = false;
            break;
        case 17:
            if (name_built[op.off]) titled_release(names[op.off]);
            name_built[op.off] = false;
            break;
        }
    }
    return self;
}
// The constructors take (parent, text) and up to five more values.
#define TITLED_ENTRY(N, PARAMS, GIVEN)                                                                                        \
    template <uintptr_t TITLE, uintptr_t BASE, const TitledExtra *EXTRAS, uint32_t EXTRA_COUNT, const TitledOp *OPS, uint32_t COUNT, uint32_t PARENT = 1, uint32_t TEXT = 2> \
    static void *__thiscall titled_construct##N PARAMS {                                                                       \
        const uint32_t given[] = GIVEN;                                                                                        \
        return titled_body<TITLE, BASE, EXTRAS, EXTRA_COUNT, OPS, COUNT, PARENT, TEXT>(self, given);                           \
    }
#define COMMA ,
TITLED_ENTRY(2, (uint8_t * self, uint32_t a, uint32_t b), {0 COMMA a COMMA b})
TITLED_ENTRY(3, (uint8_t * self, uint32_t a, uint32_t b, uint32_t c), {0 COMMA a COMMA b COMMA c})
TITLED_ENTRY(4, (uint8_t * self, uint32_t a, uint32_t b, uint32_t c, uint32_t d), {0 COMMA a COMMA b COMMA c COMMA d})
TITLED_ENTRY(5, (uint8_t * self, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e), {0 COMMA a COMMA b COMMA c COMMA d COMMA e})
TITLED_ENTRY(6, (uint8_t * self, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f), {0 COMMA a COMMA b COMMA c COMMA d COMMA e COMMA f})
TITLED_ENTRY(7, (uint8_t * self, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f, uint32_t g),
             {0 COMMA a COMMA b COMMA c COMMA d COMMA e COMMA f COMMA g})
#undef COMMA
#undef TITLED_ENTRY
#include "titled_list.inc"
