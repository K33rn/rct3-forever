// Water::Make96BE30, Make96C090 and Make96C3B0: one constructor written out three times (they differ in their two
// tables and their default description's global): make a water object (thiscall; a description or none; gives this).
//   - The base table, then its member at +4 set up (Water::Init9655D0); its own two tables (this and the member) and
//     +8 = 1500.
//   - On the description's first use (+4 clear; the default, a game global, when none is given) the description named
//     (Name::Assign with 0x133B4DC and the member's place, 4; NameTable::Set; the name let go), and the default
//     marked used (+4) when it was the one.
#include "rewrite.h"

namespace {
volatile long g_water_makes;
template <uint32_t TABLE, uint32_t MEMBER_TABLE, uint32_t DESCRIPTION> uint8_t *__fastcall water_make(uint8_t *self, void *, uint8_t *description) {
    g_water_makes++;
    *(volatile uint32_t *)self = 0xf2694c;
    ((void(__thiscall *)(void *))0x9655d0)(self + 4);
    *(volatile uint32_t *)self = TABLE;
    *(volatile uint32_t *)(self + 4) = MEMBER_TABLE;
    *(volatile uint32_t *)(self + 8) = 1500;
    const bool by_default = !description;
    uint8_t *used = by_default ? *(uint8_t *volatile *)(uintptr_t)DESCRIPTION : description;
    if (!used[4]) {
        alignas(16) uint8_t name[0x14];
        void *made = ((void *(__thiscall *)(void *, uintptr_t, uint32_t))0xbeca40)(name, 0x133b4dc, 4);
        ((void(__thiscall *)(void *, void *))0xbecc10)(used, made);
        ((void(__thiscall *)(void *))0xbec5e0)(name);
    }
    if (by_default) (*(uint8_t *volatile *)(uintptr_t)DESCRIPTION)[4] = 1;
    return self;
}

const uint8_t water_make_first[][7] = {{0x6a, 0xff, 0x68, 0xeb, 0xbb, 0xe6, 0x00}, {0x6a, 0xff, 0x68, 0x5b, 0xbc, 0xe6, 0x00}, {0x6a, 0xff, 0x68, 0xcb, 0xbc, 0xe6, 0x00}};
}   // namespace
REWRITE("Water::MakeNamed", 0x96be30, water_make_first[0], (&water_make<0xf7e860, 0xf7e84c, 0x133d0e8>), &g_water_makes);
REWRITE("Water::MakeNamed", 0x96c090, water_make_first[1], (&water_make<0xf7e89c, 0xf7e888, 0x133d0f0>), &g_water_makes);
REWRITE("Water::MakeNamed", 0x96c3b0, water_make_first[2], (&water_make<0xf7e8dc, 0xf7e8c8, 0x133d0f8>), &g_water_makes);
