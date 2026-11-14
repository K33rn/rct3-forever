// Destructors as their steps in order. The compiler wrote most destructors out of the same few pieces, in the class's
// order; tools/dtor_ops.py reads each one's steps from the game into dtor_ops_list.inc and dtor_steps runs them:
//   0 SET      a field of the object set to a constant (count: its size in bytes; a: offset; b: value)
//   1 IF       the member at a taken if there is one, else the next b steps are passed over
//   2 VCALL    the taken member's table entry at `small` called (count constant arguments: a, b): usually its
//              deleting destructor, entry 0 with 1
//   3 CDECL    the function a called with the taken member (its memory freed)
//   4 CALL     the function a called on the object + b: a member's or the base's destructor (count arguments, from
//              the ARG steps just before it)
//   5 GCOPY    the global b copied to the global a unless it is -1
//   6 SETM     through the member at `small`: the value b put at its + a
//   7 REL      the taken member's count (a word at its + `small`) taken down; at 0 its table entry at a called (with 1
//              when count is 1)
//   8 FREEM    the function b called with the member at a, whatever it is
//   9 TEXTREL  the shared text at a let go: its count (its first value) taken down; at 0 its buffer (+0xC) freed, then it
//  10 SETP     the value b put in the taken member at + a
//  11 CALLP    the function a called on the taken member (its destructor, called directly)
//  12 VECDEL   the taken member is an array of records of a bytes, its count kept just before it: each record's
//              destructor b run (VecDtor), then the block (from the count on) freed
//  13 LOOP     for each pointer in the array the member at a points to, while the index is under the count at b (read
//              again each time round; nothing when it starts at 0 or less): one that is there is taken and the next
//              `small` steps run on it
//  14 GSET     the global a set to b (count: its size in bytes)
//  15 IFB      the flag (a byte) at a looked at: when it is set (count 1) or clear (count 0) the next b steps are
//              passed over
//  16 GMOVE    the global b copied to the global a
//  17 ARG      an argument for the next call: the constant a (count 0) or the object's value at a (count 1)
//  18 GCALL    the function a called on the object the global b points to (count arguments, as CALL)
// The exception frame the game keeps while it does this isn't rebuilt (as in the other rewrites of destructors).
#include "rewrite.h"

struct DtorStep {
    uint8_t kind, count;
    uint16_t small;
    uint32_t a, b;
};
static volatile long g_dtor_steps;
// the steps from `from` up to `to`, with `member` the one taken so far
__attribute__((noinline)) static void dtor_run_range(uint8_t *self, const DtorStep *STEPS, uint32_t from, uint32_t to, uint8_t *member) {
    uint32_t args[3], nargs = 0;
    for (uint32_t i = from; i < to; i++) {
        const DtorStep &s = STEPS[i];
        switch (s.kind) {
        case 0:
            if (s.count == 4) *(volatile uint32_t *)(self + s.a) = s.b;
            else if (s.count == 2) *(volatile uint16_t *)(self + s.a) = (uint16_t)s.b;
            else *(volatile uint8_t *)(self + s.a) = (uint8_t)s.b;
            break;
        case 1:
            member = *(uint8_t *volatile *)(self + s.a);
            if (!member) i += s.b;
            break;
        case 2: {
            void *fn = (*(void ***)member)[s.small / 4];
            if (s.count == 0) ((void(__thiscall *)(void *))fn)(member);
            else if (s.count == 1) ((void(__thiscall *)(void *, uint32_t))fn)(member, s.a);
            else ((void(__thiscall *)(void *, uint32_t, uint32_t))fn)(member, s.a, s.b);
            break;
        }
        case 3:
            ((void(__cdecl *)(void *))s.a)(member);
            break;
        case 4:
        case 18: {
            void *on = s.kind == 4 ? (void *)(self + s.b) : *(void *volatile *)s.b;
            if (nargs == 0) ((void(__thiscall *)(void *))s.a)(on);
            else if (nargs == 1) ((void(__thiscall *)(void *, uint32_t))s.a)(on, args[0]);
            else if (nargs == 2) ((void(__thiscall *)(void *, uint32_t, uint32_t))s.a)(on, args[0], args[1]);
            else ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))s.a)(on, args[0], args[1], args[2]);
            nargs = 0;
            break;
        }
        case 15:
            if ((*(volatile uint8_t *)(self + s.a) != 0) == (s.count == 1)) i += s.b;
            break;
        case 16:
            *(volatile uint32_t *)s.a = *(volatile uint32_t *)s.b;
            break;
        case 17:
            args[nargs++] = s.count ? *(volatile uint32_t *)(self + s.a) : s.a;
            break;
        case 5:
            if (*(volatile int32_t *)s.b != -1) *(volatile int32_t *)s.a = *(volatile int32_t *)s.b;
            break;
        case 6:
            *(volatile uint32_t *)(*(uint8_t *volatile *)(self + s.small) + s.a) = s.b;
            break;
        case 7:
            if (--*(volatile uint16_t *)(member + s.small) == 0) {
                void *fn = (*(void ***)member)[s.a / 4];
                if (s.count) ((void(__thiscall *)(void *, uint32_t))fn)(member, 1);
                else ((void(__thiscall *)(void *))fn)(member);
            }
            break;
        case 8:
            ((void(__cdecl *)(void *))s.b)(*(void *volatile *)(self + s.a));
            break;
        case 10:
            *(volatile uint32_t *)(member + s.a) = s.b;
            break;
        case 11:
            ((void(__thiscall *)(void *))s.a)(member);
            break;
        case 12:
            ((void(__stdcall *)(void *, uint32_t, int32_t, void *))0xddcf39)(member, s.a, *(volatile int32_t *)(member - 4), (void *)s.b);
            ((void(__cdecl *)(void *))0xbce010)(member - 4);
            break;
        case 13:
            if (*(volatile int32_t *)(self + s.b) > 0) {
                int32_t index = 0;
                do {
                    uint8_t *item = (*(uint8_t **volatile *)(self + s.a))[index];
                    if (item) dtor_run_range(self, STEPS, i + 1, i + 1 + s.small, item);
                    index++;
                } while (index < *(volatile int32_t *)(self + s.b));
            }
            i += s.small;
            break;
        case 14:
            if (s.count == 4) *(volatile uint32_t *)s.a = s.b;
            else if (s.count == 2) *(volatile uint16_t *)s.a = (uint16_t)s.b;
            else *(volatile uint8_t *)s.a = (uint8_t)s.b;
            break;
        case 9: {
            --**(volatile int32_t *volatile *)(self + s.a);
            int32_t *text = *(int32_t *volatile *)(self + s.a);   // read again, as the game does
            if (*(volatile int32_t *)text == 0 && text) {
                ((void(__cdecl *)(void *))0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
                ((void(__cdecl *)(void *))0xbce000)(text);
            }
            break;
        }
        }
    }
}
__attribute__((noinline)) static void dtor_run(uint8_t *self, const DtorStep *STEPS, uint32_t N) {
    g_dtor_steps++;
    dtor_run_range(self, STEPS, 0, N, nullptr);
}
// one small entry for each destructor: the steps are run by the one function above
template <const DtorStep *STEPS, uint32_t N> static void __thiscall dtor_steps(uint8_t *self) { dtor_run(self, STEPS, N); }
#include "dtor_ops_list.inc"
