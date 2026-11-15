// Shop::Make4CA440, Shop::Make4CA520 and Shop::Make4CA600 (the same code three times; only their exception tables
// differ): make a shop's new item for an object and add it to the shop's list (thiscall; the object, a value; no
// value).
//   - 0x104 new bytes; with memory, the item is built there (0x4CAAD0) from a reference to the object given by
//     value (made in place: in_place_call.h), the value, 0 and 0; none without memory.
//   - Added at the end of the list at +0x174 (+0x178 its room, +0x17C its count; made room for 0x20 more past the
//     count by 0x4C9D80 when it is full).
#include "in_place_call.h"

namespace {
struct ShopMake {
    void *object;
    uint32_t value;
};
void __cdecl fill_make(uint8_t *area, void *context) {
    const ShopMake &m = *(const ShopMake *)context;
    in_place::reference_to((uint32_t *)area, m.object);
    ((uint32_t *)area)[5] = m.value;
    ((uint32_t *)area)[6] = 0;
    ((uint32_t *)area)[7] = 0;
}

volatile long g_shop_makes;
void __fastcall shop_make(uint8_t *self, void *, void *object, uint32_t value) {
    g_shop_makes++;
    void *memory = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x104);
    uint32_t item = 0;
    if (memory) {
        ShopMake m{object, value};
        item = call_in_place(0x4caad0, memory, 0x20, fill_make, &m);
    }
    const int32_t count = *(int32_t *)(self + 0x17c);
    if (*(int32_t *)(self + 0x178) < count + 1) ((void(__thiscall *)(void *, int32_t))0x4c9d80)(self + 0x174, count + 0x20);
    const int32_t at = *(volatile int32_t *)(self + 0x17c);
    *(int32_t *)(self + 0x17c) = at + 1;
    (*(uint32_t **)(self + 0x174))[at] = item;
}

const uint8_t shop_make_first[][7] = {{0x6a, 0xff, 0x68, 0x1b, 0x3a, 0xe1, 0x00}, {0x6a, 0xff, 0x68, 0x3b, 0x3a, 0xe1, 0x00}, {0x6a, 0xff, 0x68, 0x5b, 0x3a, 0xe1, 0x00}};
}   // namespace
REWRITE("Shop::MakeItem", 0x4ca440, shop_make_first[0], &shop_make, &g_shop_makes);
REWRITE("Shop::MakeItem", 0x4ca520, shop_make_first[1], &shop_make, &g_shop_makes);
REWRITE("Shop::MakeItem", 0x4ca600, shop_make_first[2], &shop_make, &g_shop_makes);
