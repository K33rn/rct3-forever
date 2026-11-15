// StaffWindow::BuildTabs (three copies, 0x78D910, 0x78DFC0, 0x78E670; tools/family.py: staff_tabs.inc; they differ
// in their tables, the text table their window uses, a value and the costume's name): build the tabs of a staff
// member's window (thiscall: two values a and b; gives the window).
//   - The window's base built with a and b (0x78D290) and its four tables set (+0, +4, +0xC, +0x24).
//   - Two tabs, "Stats" (+0xDC, its button at +0x12C) and "Tasks" (+0x130, button +0x180): the tab manager made
//     sure of (the global [0x12F8084]: 0x54 new bytes built by 0x659770 with 0 when there is none), the tab built
//     with 0 and the manager's +0x28 (0x63C840), b kept at its +0x4C and its three tables set; the button (0x164 new
//     bytes; none without memory) built (0x6A2620 with 0, the tab's name looked up in the copy's text table, the
//     tab's +4, 0x9165). The name: a wide text of the C text (none without memory), looked up with 0x4B7940; both
//     texts let go after, as far as they were made.
//   - The costume part (+0x184; 0x1B0 new bytes): built (0x7EF3D0) with 0, "Costume" looked up the same way, b and
//     the float the manager's +0x28 gives for 4 (0x63A860).
//   - With a of 0, the copy's text table marked (+4 = 1).
//   - The manager's +0x28 told of the three: 0x63B3D0 with 2, the first button and 0x12FD258; with 3, the second
//     and 0x12FE4D4; 0x63B270 with 4 and the costume part (the manager made sure of before each). The costume
//     part given the copy's value (0x65AA90).
//   - Named texts (0x4040F0: the name, the game's text table, 1; then 0xBD14E0 into the part's +0x9C, and destroyed
//     with 0xBD1880) for the costume part (the copy's name), the second button ("Staff_Tasks") and the first
//     ("Staff_Stats").
//   - Unless the global object [0x1342708] exists with its own object marked (+0xA4): in game mode 2
//     ([0x10EF988]) the first tab (its +4's slot 1) told of its button; in mode 3 the second.
#include "rewrite.h"

namespace {
struct StaffTabs {
    uint32_t at, table0, table4, table_c, table24, stats0, stats4, stats20, texts, tasks0, tasks4, tasks20, costume_value, costume_name;
};
#define STAFF_TABS(at, t0, t4, tc, t24, s0, s4, s20, texts, k0, k4, k20, value, name, ...) {at, t0, t4, tc, t24, s0, s4, s20, texts, k0, k4, k20, value, name},
constexpr StaffTabs staff_tabs[] = {
#include "staff_tabs.inc"
};
#undef STAFF_TABS
#define STAFF_TABS(at, t0, t4, tc, t24, s0, s4, s20, texts, k0, k4, k20, value, name, ...) {__VA_ARGS__},
const uint8_t staff_tabs_first[][7] = {
#include "staff_tabs.inc"
};
#undef STAFF_TABS

typedef void *(__cdecl *Alloc)(uint32_t);
typedef void(__cdecl *Free)(void *);
const Alloc alloc = (Alloc)0xbcdfc0;
inline void let_go(int32_t *volatile *held) {   // a shared text let go (read again, as the game does)
    --**held;
    int32_t *text = *held;
    if (*(volatile int32_t *)text == 0) {
        ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
        ((Free)0xbce000)(text);
    }
}
inline uint8_t *manager() {   // the tab manager, made the first time
    if (!*(void *volatile *)(uintptr_t)0x12f8084) {
        void *memory = alloc(0x54);
        if (memory) ((void(__thiscall *)(void *, uint32_t))0x659770)(memory, 0);
    }
    return *(uint8_t *volatile *)(uintptr_t)0x12f8084;
}
inline void *manager_list() { return *(void **)(manager() + 0x28); }

// A name looked up in a text table: made (a wide text of the C text, then the lookup), used, then both let go as far
// as they were made (the game's flags: only once the memory for what uses them was there).
struct Looked {
    int32_t *volatile text, *volatile found;
};
inline void *look_up(Looked &l, const char *name, uint32_t texts) {
    void *memory = alloc(0x10);
    l.text = memory ? ((int32_t * (__thiscall *)(void *, const char *))0xbcf850)(memory, name) : nullptr;
    return ((void *(__thiscall *)(void *, void *, uint32_t))0x4b7940)((void *)&l.found, (void *)&l.text, *(volatile uint32_t *)(uintptr_t)texts);
}

uint8_t *build_tab(uint8_t *self, uint32_t at, uint32_t b, uint32_t t0, uint32_t t4, uint32_t t20, const char *name, uint32_t texts) {
    uint8_t *tab = self + at;
    ((void(__thiscall *)(void *, uint32_t, void *))0x63c840)(tab, 0, manager_list());
    *(volatile uint32_t *)(tab + 0x4c) = b;
    *(volatile uint32_t *)(tab + 0) = t0;
    *(volatile uint32_t *)(tab + 4) = t4;
    *(volatile uint32_t *)(tab + 0x20) = t20;
    void *memory = alloc(0x164);
    if (!memory) return nullptr;
    Looked l;
    void *found = look_up(l, name, texts);
    uint8_t *button = (uint8_t *)((void *(__thiscall *)(void *, uint32_t, void *, void *, uint32_t))0x6a2620)(memory, 0, found, tab + 4, 0x9165);
    let_go(&l.found);
    let_go(&l.text);
    return button;
}
void named_text(uint8_t *part, uintptr_t name) {
    alignas(4) uint8_t reference[0x18];
    void *made = ((void *(__thiscall *)(void *, uintptr_t, uint32_t, uint32_t))0x4040f0)(reference, name, 0x1303cfc, 1);
    ((void(__thiscall *)(void *, void *))0xbd14e0)(part + 0x9c, made);
    ((void(__thiscall *)(void *))0xbd1880)(reference);
}

volatile long g_staff_tabs;
uint8_t *staff_tab(uint8_t *self, uint32_t a, uint32_t b, const StaffTabs &kind) {
    g_staff_tabs++;
    ((void(__thiscall *)(void *, uint32_t, uint32_t))0x78d290)(self, a, b);
    *(volatile uint32_t *)(self + 0) = kind.table0;
    *(volatile uint32_t *)(self + 4) = kind.table4;
    *(volatile uint32_t *)(self + 0xc) = kind.table_c;
    *(volatile uint32_t *)(self + 0x24) = kind.table24;
    *(uint8_t *volatile *)(self + 0x12c) = build_tab(self, 0xdc, b, kind.stats0, kind.stats4, kind.stats20, (const char *)0xf3766c, kind.texts);   // "Stats"
    *(uint8_t *volatile *)(self + 0x180) = build_tab(self, 0x130, b, kind.tasks0, kind.tasks4, kind.tasks20, (const char *)0xf6f2bc, kind.texts);   // "Tasks"
    uint8_t *costume = nullptr;
    if (void *memory = alloc(0x1b0)) {
        Looked l;
        void *found = look_up(l, (const char *)0xf6c4d8, kind.texts);   // "Costume"
        const float size = ((float(__thiscall *)(void *, uint32_t))0x63a860)(manager_list(), 4);
        costume = (uint8_t *)((void *(__thiscall *)(void *, uint32_t, void *, uint32_t, float))0x7ef3d0)(memory, 0, found, b, size);
        *(uint8_t *volatile *)(self + 0x184) = costume;
        let_go(&l.found);
        let_go(&l.text);
    } else {
        *(uint8_t *volatile *)(self + 0x184) = nullptr;
    }
    if (!a) (*(uint8_t *volatile *)(uintptr_t)kind.texts)[4] = 1;
    ((void(__thiscall *)(void *, uint32_t, void *, uint32_t))0x63b3d0)(manager_list(), 2, *(void **)(self + 0x12c), 0x12fd258);
    ((void(__thiscall *)(void *, uint32_t, void *, uint32_t))0x63b3d0)(manager_list(), 3, *(void **)(self + 0x180), 0x12fe4d4);
    ((void(__thiscall *)(void *, uint32_t, void *))0x63b270)(manager_list(), 4, *(void **)(self + 0x184));
    ((void(__thiscall *)(void *, uint32_t))0x65aa90)(*(void **)(self + 0x184), kind.costume_value);
    named_text(*(uint8_t **)(self + 0x184), kind.costume_name);
    named_text(*(uint8_t **)(self + 0x180), 0xf6f298);   // "Staff_Tasks"
    named_text(*(uint8_t **)(self + 0x12c), 0xf59be8);   // "Staff_Stats"
    uint8_t **global = *(uint8_t **volatile *)(uintptr_t)0x1342708;
    if (!(global && *global && (*global)[0xa4])) {
        const int32_t mode = *(volatile int32_t *)(uintptr_t)0x10ef988;
        if (mode == 2 || mode == 3) {
            uint8_t *tab = self + (mode == 2 ? 0xe0 : 0x134);
            void *button = *(void **)(self + (mode == 2 ? 0x12c : 0x180));
            ((void(__thiscall *)(void *, void *))(*(void ***)tab)[1])(tab, button);
        }
    }
    return self;
}

template <int K> uint8_t *__thiscall staff_tab_copy(uint8_t *self, uint32_t a, uint32_t b) { return staff_tab(self, a, b, staff_tabs[K]); }
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(staff_tabs[0].at == 0x78d910 && staff_tabs[1].at == 0x78dfc0 && staff_tabs[2].at == 0x78e670, "staff_tabs.inc changed");
REWRITE("StaffWindow::BuildTabs", 0x78d910, staff_tabs_first[0], (&staff_tab_copy<0>), &g_staff_tabs);
REWRITE("StaffWindow::BuildTabs", 0x78dfc0, staff_tabs_first[1], (&staff_tab_copy<1>), &g_staff_tabs);
REWRITE("StaffWindow::BuildTabs", 0x78e670, staff_tabs_first[2], (&staff_tab_copy<2>), &g_staff_tabs);
