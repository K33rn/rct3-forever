// Destructors as their steps in order. The compiler wrote most destructors out of the same few pieces, in the class's
// order; tools/dtor_ops.py reads each one's steps from the game into dtor_ops_list.inc and dtor_steps runs them:
//   0 SET      a field of the object set to a constant (count: its size in bytes; a: offset; b: value)
//   1 IF       the member at a taken if there is one, else the next b steps are passed over
//   2 VCALL    the taken member's table entry at `small` called (count constant arguments: a, b): usually its
//              deleting destructor, entry 0 with 1
//   3 CDECL    the function a called with the taken member (its memory freed)
//   4 CALL     the function a called on the object + b: a member's or the base's destructor
//   5 GCOPY    the global b copied to the global a unless it is -1
//   6 SETM     through the member at `small`: the value b put at its + a
//   7 REL      the taken member's count (a word at its + `small`) taken down; at 0 its table entry at a called (with 1
//              when count is 1)
//   8 FREEM    the function b called with the member at a, whatever it is
//   9 TEXTREL  the shared text at a let go: its count (its first value) taken down; at 0 its buffer (+0xC) freed, then it
// The exception frame the game keeps while it does this isn't rebuilt (as in the other rewrites of destructors).
#include "rewrite.h"

struct DtorStep {
    uint8_t kind, count;
    uint16_t small;
    uint32_t a, b;
};
static volatile long g_dtor_steps;
template <const DtorStep *STEPS, uint32_t N> static void __thiscall dtor_steps(uint8_t *self) {
    g_dtor_steps++;
    uint8_t *member = nullptr;
    for (uint32_t i = 0; i < N; i++) {
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
            ((void(__thiscall *)(void *))s.a)(self + s.b);
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
#include "dtor_ops_list.inc"
