// LaserDesigner::BuildIdentBar and WaterJetDesigner::BuildIdentBar: one constructor written out twice. (The compiler
// scheduled the two a little differently, so tools/family.py cannot line them up; their rows are written here, and the
// equivalence test checks each against its own original.) Given a description (or none: the default, a game global)
// and a title (or none), the bar:
//   - titled as the park desktop is (the game's name, " - ", the title, kept in the title; or a copy of the default
//     title), and built (0x9B2340) for the description with that title;
//   - given its three tables, and a reference member (+0xF4) to no object (Handle::For with 0) linked into that
//     handle's list, with its own table, no owner and its flag set; +0x110 cleared;
//   - on the description's first use, the description named (0xBECA40/0xBECC10 with the copy's name global and 0) and
//     the default description marked used when it was the one;
//   - given a reference to its text (+0x110: 0x1C new bytes; "LaserIdentbar" or "WaterJetIdentbar" in the game's text
//     table; none without memory) and the "GUI2_ParticleDesigner_Bar" text (0x6B0230, with 0);
//   - its +0xF0 set (1 for the laser's, 2 for the water jet's).
#include "rewrite.h"

namespace {
struct IdentBar {
    uint32_t at, description, table, second_table, third_table, member_table, name, text, kind;
};
constexpr IdentBar ident_bars[] = {
    {0xad7460, 0x13ce4f0, 0xf951d0, 0xf951b8, 0xf951ac, 0xf952e0, 0x13ce4e0, 0xf952e4, 1},   // the laser's
    {0xaf04a0, 0x13ce5ac, 0xf96508, 0xf964f4, 0xf964e8, 0xf96618, 0x13ce5b0, 0xf9661c, 2},   // the water jet's
};
const uint8_t ident_bars_first[][7] = {{0x6a, 0xff, 0x68, 0xcd, 0xb0, 0xe8, 0x00}, {0x6a, 0xff, 0x68, 0xed, 0xd6, 0xe8, 0x00}};

typedef void *(__cdecl *Alloc)(uint32_t);
typedef void(__cdecl *Free)(void *);
// a shared text {count, ..., buffer at +0xC} let go: the count taken down; at 0 its buffer freed, then it
inline void let_go(int32_t *volatile *held) {
    --**held;
    int32_t *text = *held;   // read again, as the game does
    if (*(volatile int32_t *)text == 0) {
        ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
        ((Free)0xbce000)(text);
    }
}
typedef void *(__thiscall *TextRef)(void *, uintptr_t, uint32_t, uint32_t);

volatile long g_ident_bars;
uint8_t *ident_bar(uint8_t *self, uint8_t *description, int32_t **title, const IdentBar &kind) {
    g_ident_bars++;
    uint8_t *shared = *(uint8_t *volatile *)(uintptr_t)kind.description;   // the default description
    void *named;
    int32_t *volatile copy;
    if (title) {
        const char *own = *(const char *volatile *)(shared + 8);
        int32_t *volatile first;
        if (own) {
            void *memory = ((Alloc)0xbcdfc0)(0x10);
            first = memory ? ((int32_t * (__thiscall *)(void *, const char *))0xbcf850)(memory, own) : nullptr;
        } else {
            first = ((int32_t * (__fastcall *)(uint32_t))0xbd0490)(1);
        }
        int32_t *volatile joined, *volatile whole;
        void *r = ((void *(__thiscall *)(void *, void *, uintptr_t))0x403be0)((void *)&first, (void *)&joined, 0xf2738c);   // " - "
        int32_t **made = (int32_t **)((void *(__thiscall *)(void *, void *, void *))0x403bb0)(r, (void *)&whole, title);
        int32_t *old = *(int32_t *volatile *)title;
        if (*(int32_t *volatile *)made != old) {
            --*(volatile int32_t *)old;
            uintptr_t again = (uintptr_t) * (int32_t *volatile *)title;
            if (*(volatile int32_t *)again == 0 && again) {
                ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)((int32_t *)again)[3]);
                ((Free)0xbce000)((void *)again);
            }
            int32_t *now = *(int32_t *volatile *)made;
            *(int32_t *volatile *)title = now;
            ++*(volatile int32_t *)now;
        }
        let_go(&whole);
        let_go(&joined);
        let_go(&first);
        named = title;
    } else {
        named = ((void *(__thiscall *)(void *, void *))0x41c740)((void *)&copy, shared);
    }
    ((void *(__thiscall *)(void *, void *, void *))0x9b2340)(self, description ? description : *(uint8_t *volatile *)(uintptr_t)kind.description, named);
    if (!title) let_go(&copy);
    uint32_t *member = (uint32_t *)(self + 0xf4);
    *(volatile uint32_t *)(self + 0) = kind.table;
    *(volatile uint32_t *)(self + 4) = kind.second_table;
    *(volatile uint32_t *)(self + 0xd4) = kind.third_table;
    member[0] = 0xf26ae0;
    member[4] = (uint32_t)(uintptr_t)member;
    uint8_t *handle = ((uint8_t * (__fastcall *)(uintptr_t))0x805700)(0);
    member[1] = (uint32_t)(uintptr_t)handle;
    uint32_t *list = *(uint32_t **)(handle + 0x20);
    member[2] = list[0];
    ((uint32_t *)(uintptr_t)list[0])[1] = (uint32_t)(uintptr_t)&member[2];
    member[3] = (uint32_t)(uintptr_t)list;
    list[0] = (uint32_t)(uintptr_t)&member[2];
    member[0] = kind.member_table;
    member[5] = 0;
    *(uint8_t *)&member[6] = 1;
    *(volatile uint32_t *)(self + 0x110) = 0;
    uint8_t *used = description ? description : *(uint8_t *volatile *)(uintptr_t)kind.description;
    if (!used[4]) {
        alignas(16) uint8_t text[0x40];
        void *made = ((void *(__thiscall *)(void *, uintptr_t, uint32_t))0xbeca40)(text, kind.name, 0);
        ((void(__thiscall *)(void *, void *))0xbecc10)(used, made);
        ((void(__thiscall *)(void *))0xbec5e0)(text);
    }
    if (!description) (*(uint8_t *volatile *)(uintptr_t)kind.description)[4] = 1;
    void *memory = ((Alloc)0xbcdfc0)(0x1c);
    void *reference = memory ? ((TextRef)0x41cb90)(memory, kind.text, 0x1303cfc, 1) : nullptr;
    *(void *volatile *)(self + 0x110) = reference;
    alignas(16) uint8_t bar[0x40];
    void *made = ((TextRef)0x41cb90)(bar, 0xf84278, 0x1303cfc, 1);   // "GUI2_ParticleDesigner_Bar"
    ((void(__thiscall *)(void *, void *, uint32_t))0x6b0230)(self, made, 0);
    ((void(__thiscall *)(void *))0xbd1880)(bar);
    *(volatile uint32_t *)(self + 0xf0) = kind.kind;
    return self;
}

template <int K>
uint8_t *__thiscall ident_bar_copy(uint8_t *self, uint8_t *description, int32_t **title) {
    return ident_bar(self, description, title, ident_bars[K]);
}
}   // namespace
REWRITE("Designer::BuildIdentBar", 0xad7460, ident_bars_first[0], (&ident_bar_copy<0>), &g_ident_bars);
REWRITE("Designer::BuildIdentBar", 0xaf04a0, ident_bars_first[1], (&ident_bar_copy<1>), &g_ident_bars);
