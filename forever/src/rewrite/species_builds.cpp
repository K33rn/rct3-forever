// SpeciesCondition::Build and its second copy (0x6E5F30, 0x6E3400; tools/family.py: species_builds.inc; they differ
// in their default description, tables, the condition's count and selection fields and its text function): build
// the scenario editor's species list for an animal condition (thiscall: a description or none, a title or none, the
// condition; gives the window).
//   - Titled and built as the ident bars are (ident_bars.cpp), the base 0x6D5380 given the description, that title and
//     the condition.
//   - Its member at +0xF8 built (0x6AF6C0 with the window); +0x118's base class 0xF31470, then the window's four
//     tables.
//   - The list box (+0x11C; 0x578 new bytes, none without memory): "SpeciesListBox" (0xF659EC) as a wide text looked
//     up for the window with the default description (0x6A39D0), the box built with 0 and it (0x69F710); the two
//     texts let go as far as they were made. The box's list (+0x104) linked to the member (0x6AF710 on +0x120).
//   - The condition kept (+0x140). On the description's first use, the description named (0xBECA40 with the name
//     global 0x1305C0C and 0x118) and the default marked used when it was the one.
//   - The list given room for the condition's count (+0x80 or +0x88; 0x6A0B20); for each, the condition's text for it
//     (the copy's function, into a text of its own) put at that place of the list's texts (+0x1C8: the old one let go
//     when it is another, the new one taken), the list told (0x6A0560), the text let go.
//   - The list's selection set from the condition (+0x84 or +0x90; 0x6A0040) and the box placed (0x6B1330, 6).
#include "rewrite.h"

namespace {
struct SpeciesBuild {
    uint32_t at, description, table0, table4, table_f8, table_118, count, text, selected;
};
#define SPECIES_BUILDS(at, description, t0, t4, tf8, t118, count, text, selected, ...) {at, description, t0, t4, tf8, t118, count, text, selected},
constexpr SpeciesBuild species_builds[] = {
#include "species_builds.inc"
};
#undef SPECIES_BUILDS
#define SPECIES_BUILDS(at, description, t0, t4, tf8, t118, count, text, selected, ...) {__VA_ARGS__},
const uint8_t species_builds_first[][7] = {
#include "species_builds.inc"
};
#undef SPECIES_BUILDS

typedef void *(__cdecl *Alloc)(uint32_t);
typedef void(__cdecl *Free)(void *);
const Alloc alloc = (Alloc)0xbcdfc0;
inline void free_text(int32_t *text) {
    ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
    ((Free)0xbce000)(text);
}
inline void let_go(int32_t *volatile *held) {   // a shared text let go (read again, as the game does)
    --**held;
    int32_t *text = *held;
    if (*(volatile int32_t *)text == 0) free_text(text);
}

volatile long g_species_builds;
uint8_t *species_build(uint8_t *self, uint8_t *description, int32_t **title, uint8_t *condition, const SpeciesBuild &kind) {
    g_species_builds++;
    uint8_t *shared = *(uint8_t *volatile *)(uintptr_t)kind.description;
    void *named;
    int32_t *volatile copy;
    if (title) {
        const char *own = *(const char *volatile *)(shared + 8);
        int32_t *volatile first;
        if (own) {
            void *memory = alloc(0x10);
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
            if (*(volatile int32_t *)again == 0 && again) free_text((int32_t *)again);
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
    ((void *(__thiscall *)(void *, void *, void *, void *))0x6d5380)(self, description ? description : *(uint8_t *volatile *)(uintptr_t)kind.description, named,
                                                                     condition);
    if (!title) let_go(&copy);
    ((void(__thiscall *)(void *, void *))0x6af6c0)(self + 0xf8, self);
    *(volatile uint32_t *)(self + 0x118) = 0xf31470;
    *(volatile uint32_t *)(self + 0) = kind.table0;
    *(volatile uint32_t *)(self + 4) = kind.table4;
    *(volatile uint32_t *)(self + 0xf8) = kind.table_f8;
    *(volatile uint32_t *)(self + 0x118) = kind.table_118;
    uint8_t *box = nullptr;
    if (void *memory = alloc(0x578)) {
        void *m = alloc(0x10);
        int32_t *volatile name = m ? ((int32_t * (__thiscall *)(void *, uintptr_t))0xbcf850)(m, 0xf659ec) : nullptr;   // "SpeciesListBox"
        int32_t *volatile looked;
        void *r = ((void *(__thiscall *)(void *, void *, void *, uint32_t))0x6a39d0)((void *)&looked, (void *)&name, self,
                                                                                  *(volatile uint32_t *)(uintptr_t)kind.description);
        box = (uint8_t *)((void *(__thiscall *)(void *, uint32_t, void *))0x69f710)(memory, 0, r);
        *(uint8_t *volatile *)(self + 0x11c) = box;
        let_go(&looked);
        let_go(&name);
    } else {
        *(uint8_t *volatile *)(self + 0x11c) = nullptr;
    }
    ((void(__thiscall *)(void *, void *, void *))0x6af710)(self + 0x120, *(uint8_t **)(self + 0x11c) + 0x104, self + 0xf8);
    *(uint8_t *volatile *)(self + 0x140) = condition;
    uint8_t *used = description ? description : *(uint8_t *volatile *)(uintptr_t)kind.description;
    if (!used[4]) {
        alignas(16) uint8_t text[0x40];
        void *made = ((void *(__thiscall *)(void *, uintptr_t, uint32_t))0xbeca40)(text, 0x1305c0c, 0x118);
        ((void(__thiscall *)(void *, void *))0xbecc10)(used, made);
        ((void(__thiscall *)(void *))0xbec5e0)(text);
    }
    if (!description) (*(uint8_t *volatile *)(uintptr_t)kind.description)[4] = 1;
    uint8_t *list = *(uint8_t **)(self + 0x11c) + 0x104;
    const int32_t count = *(int32_t *)(condition + kind.count);
    ((void(__thiscall *)(void *, int32_t))0x6a0b20)(list, count);
    for (int32_t i = 0; i < count; i++) {
        int32_t *volatile text;
        int32_t **got = (int32_t **)((void *(__thiscall *)(void *, void *, int32_t))kind.text)(*(void **)(self + 0x140), (void *)&text, i);
        int32_t *volatile *slot = (int32_t * volatile *)(*(uint8_t **)(list + 0x1c8) + (uint32_t)i * 4);
        if (*got != *slot) {
            --**slot;
            int32_t *old = *slot;   // read again, as the game does
            if (*(volatile int32_t *)old == 0 && old) free_text(old);
            int32_t *now = *got;
            *slot = now;
            ++*now;
        }
        ((void(__thiscall *)(void *))0x6a0560)(list);
        let_go(&text);
    }
    ((void(__thiscall *)(void *, uint32_t))0x6a0040)(list, *(uint32_t *)(*(uint8_t **)(self + 0x140) + kind.selected));
    ((void(__thiscall *)(void *, uint32_t, void *))0x6b1330)(*(void **)(self + 0x11c), 6, self);
    return self;
}

template <int K> uint8_t *__thiscall species_build_copy(uint8_t *self, uint8_t *description, int32_t **title, uint8_t *condition) {
    return species_build(self, description, title, condition, species_builds[K]);
}
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(species_builds[0].at == 0x6e3400 && species_builds[1].at == 0x6e5f30, "species_builds.inc changed");
REWRITE("SpeciesCondition::Build", 0x6e3400, species_builds_first[0], (&species_build_copy<0>), &g_species_builds);
REWRITE("SpeciesCondition::Build", 0x6e5f30, species_builds_first[1], (&species_build_copy<1>), &g_species_builds);
