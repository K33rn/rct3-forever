// Sky::Next42BF40, Sky::Next42BFF0, Water::Next965440 and Water::Next9654F0 (one function written out four times;
// tools/family.py: item_lists.inc; they differ in where the list is kept): take the next item from a list of items
// made ahead {items, used, made} (thiscall; gives the item).
//   - When every item made is used: room for twice as many (100 when there were none) in a new aligned block, the
//     items copied over (the used count read again each time), each new place given a new item (12 bytes with its
//     +8 cleared; none without memory), the old block freed and the new count kept.
//   - The item at the used count taken and the count raised.
#include "rewrite.h"

namespace {
struct ItemList {
    uint32_t at, used, made, items;
};
#define ITEM_LISTS(at, used, made, items, ...) {at, used, made, items},
constexpr ItemList item_lists[] = {
#include "item_lists.inc"
};
#undef ITEM_LISTS
#define ITEM_LISTS(at, used, made, items, ...) {__VA_ARGS__},
const uint8_t item_lists_first[][7] = {
#include "item_lists.inc"
};
#undef ITEM_LISTS
static_assert(item_lists[0].made == item_lists[0].used + 4 && item_lists[0].items == item_lists[0].used - 4, "the list is {items, used, made}");

volatile long g_item_lists;
uint32_t next_item(uint8_t *self, const ItemList &kind) {
    g_item_lists++;
    volatile int32_t &used = *(volatile int32_t *)(self + kind.used);
    uint32_t **volatile &items = *(uint32_t **volatile *)(self + kind.items);
    if (used >= *(volatile int32_t *)(self + kind.made)) {
        const int32_t room = used ? used * 2 : 100;
        uint32_t **old = items;
        items = (uint32_t **)((void *(__cdecl *)(uint32_t))0xbcdfd0)((uint32_t)room * 4);
        for (int32_t i = 0; i < used; i++) items[i] = old[i];
        for (int32_t i = used; i < room; i++) {
            uint32_t *item = (uint32_t *)((void *(__cdecl *)(uint32_t))0xbcdfc0)(0xc);
            if (item) item[2] = 0;
            items[i] = item;
        }
        ((void(__cdecl *)(void *))0xbce010)(old);
        *(volatile int32_t *)(self + kind.made) = room;
    }
    const int32_t at = used;
    const uint32_t item = (uint32_t)(uintptr_t)items[at];
    used = at + 1;
    return item;
}

template <int K> uint32_t __fastcall next_item_copy(uint8_t *self) { return next_item(self, item_lists[K]); }
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(item_lists[0].at == 0x42bf40 && item_lists[1].at == 0x42bff0 && item_lists[2].at == 0x965440 && item_lists[3].at == 0x9654f0, "item_lists.inc changed");
REWRITE("Items::Next", 0x42bf40, item_lists_first[0], (&next_item_copy<0>), &g_item_lists);
REWRITE("Items::Next", 0x42bff0, item_lists_first[1], (&next_item_copy<1>), &g_item_lists);
REWRITE("Items::Next", 0x965440, item_lists_first[2], (&next_item_copy<2>), &g_item_lists);
REWRITE("Items::Next", 0x9654f0, item_lists_first[3], (&next_item_copy<3>), &g_item_lists);
