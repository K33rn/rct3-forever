// Ride::Compare55A6A0 and Ride::Compare55B2D0 (one function written out twice; they differ only in where four values
// land: the first swaps +0x80/+0x84 with +0x88/+0x8C): take over another ride part's state (thiscall; the other; gives
// this one).
//   - The references at +0 and +0x1C assigned from the other's (0x4AA7E0), the other's each assigned an empty one (a
//     reference made on the stack: class 0xF26AE0 with Handle::For of none, joined first in its list, itself its owner;
//     then class 0xF30B3C, no object, its flag set; destroyed after, ObjectRef's 0x40F730).
//   - Every node of the other's list (its head at +0x38; up to the one with no next) taken out (a broken list stops the
//     game, int3) and put first in this one's list (its head at [+0x48]).
//   - The reference at +0x50: its link (+0x58) taken out of its list, its handle let go (0x8055C0 with 0), the other's
//     handle (+0x54) taken (its count +8 one more), linked first in that handle's list, told (its slot 0); the other's
//     the same with the empty handle (0x805750; no count).
//   - +0x64..+0x7C copied from the other, then +0x80..+0x8C (as the copy places them); the other's +0x70..+0x8C made -1.
#include "in_place_call.h"

namespace {
inline void take_out(uint32_t *link) {   // a node taken out of its list (the game checks it is whole first)
    uint32_t *next = (uint32_t *)(uintptr_t)link[0], *head = (uint32_t *)(uintptr_t)link[1];
    if (!(head && next && next[1] == (uint32_t)(uintptr_t)link && head[0] == (uint32_t)(uintptr_t)link)) __builtin_trap();
    next[1] = (uint32_t)(uintptr_t)head;
    ((uint32_t *)(uintptr_t)link[1])[0] = link[0];
    link[0] = 0, link[1] = 0;
}
inline void put_first(uint32_t *link, uint32_t *list) {   // a node put first after a list's head
    link[0] = list[0];
    ((uint32_t *)(uintptr_t)list[0])[1] = (uint32_t)(uintptr_t)link;
    link[1] = (uint32_t)(uintptr_t)list;
    list[0] = (uint32_t)(uintptr_t)link;
}
typedef void(__thiscall *Assign)(void *, void *);
inline void assign_empty(uint8_t *to) {   // a reference assigned an empty one made here
    volatile uint32_t empty[7];
    in_place::reference_to((uint32_t *)empty, nullptr);
    empty[0] = 0xf30b3c;
    empty[5] = 0;
    *(volatile uint8_t *)&empty[6] = 1;
    ((Assign)0x4aa7e0)(to, (void *)empty);
    ((void(__thiscall *)(volatile void *))0x40f730)(empty);
}
inline void tell(uint8_t *reference) { ((void(__thiscall *)(void *))(*(void ***)reference)[0])(reference); }

volatile long g_ride_moves;
template <bool SWAPPED> uint8_t *__fastcall ride_move(uint8_t *self, void *, uint8_t *other) {
    g_ride_moves++;
    ((Assign)0x4aa7e0)(self, other);
    assign_empty(other);
    ((Assign)0x4aa7e0)(self + 0x1c, other + 0x1c);
    assign_empty(other + 0x1c);
    for (;;) {
        uint32_t *node = (uint32_t *)(uintptr_t)*(volatile uint32_t *)(other + 0x38);   // (read again each time: the take-out moves it)
        if (!node[0] || !node) break;
        take_out(node);
        put_first(node, *(uint32_t **)(self + 0x48));
    }
    // this one's reference at +0x50 pointed at the other's handle
    take_out((uint32_t *)(self + 0x58));
    ((void(__thiscall *)(void *, uint32_t))0x8055c0)(*(void **)(self + 0x54), 0);
    uint8_t *handle = *(uint8_t **)(other + 0x54);
    ++*(int32_t *)(handle + 8);
    *(uint8_t **)(self + 0x54) = handle;
    put_first((uint32_t *)(self + 0x58), *(uint32_t **)(handle + 0x20));
    tell(self + 0x50);
    // the other's at the empty handle
    take_out((uint32_t *)(other + 0x58));
    ((void(__thiscall *)(void *, uint32_t))0x8055c0)(*(void **)(other + 0x54), 0);
    uint8_t *empty = ((uint8_t * (__cdecl *)())0x805750)();
    *(uint8_t **)(other + 0x54) = empty;
    put_first((uint32_t *)(other + 0x58), *(uint32_t **)(empty + 0x20));
    tell(other + 0x50);
    for (uint32_t off = 0x64; off <= 0x7c; off += 4) *(uint32_t *)(self + off) = *(uint32_t *)(other + off);
    static const uint32_t to_swapped[4] = {0x88, 0x8c, 0x80, 0x84}, to_straight[4] = {0x80, 0x84, 0x88, 0x8c};
    for (int i = 0; i < 4; i++) *(uint32_t *)(self + (SWAPPED ? to_swapped : to_straight)[i]) = *(uint32_t *)(other + 0x80 + 4 * i);
    for (uint32_t off = 0x70; off <= 0x8c; off += 4) *(uint32_t *)(other + off) = 0xffffffff;
    return self;
}

const uint8_t ride_move_first[][7] = {{0x6a, 0xff, 0x68, 0x30, 0xe7, 0xe1, 0x00}, {0x6a, 0xff, 0x68, 0x80, 0xe8, 0xe1, 0x00}};
}   // namespace
REWRITE("RidePart::TakeOver", 0x55a6a0, ride_move_first[0], &ride_move<true>, &g_ride_moves);
REWRITE("RidePart::TakeOver", 0x55b2d0, ride_move_first[1], &ride_move<false>, &g_ride_moves);
