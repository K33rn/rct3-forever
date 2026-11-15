// Window::Build9A31D0, Build9A3440 and Build9A36B0 (one function written out three times; tools/family.py:
// titled_builds.inc; they differ in their default description, base, member's place and tables): build a window
// from a description (or none: the copy's default), a title (or none) and a value (thiscall; gives the window).
//   - Titled and built as the ident bars are (ident_bars.cpp), with the copy's base (0x9C1AF0, or the ident bars
//     themselves: 0xAD7460, 0xAF04A0) given the description and that title.
//   - A member (+0x12C or +0x114): the value at its +0xC, class 0xF80C2C, +4 cleared, the window at +8; told
//     (0x99FF30). The window's three tables and the member's own set.
//   - On the description's first use, the description named (0xBECA40 with the name global 0x13413DC and the member's
//     place; 0xBECC10; the name destroyed with 0xBEC5E0), and the default description marked used when it was the
//     one. The member told again.
#include "rewrite.h"

namespace {
struct TitledBuild {
    uint32_t at, description, base, member, table0, table4, table_d4, member_table;
};
#define TITLED_BUILDS(at, description, base, member, t0, t4, td4, tm, ...) {at, description, base, member, t0, t4, td4, tm},
constexpr TitledBuild titled_builds[] = {
#include "titled_builds.inc"
};
#undef TITLED_BUILDS
#define TITLED_BUILDS(at, description, base, member, t0, t4, td4, tm, ...) {__VA_ARGS__},
const uint8_t titled_builds_first[][7] = {
#include "titled_builds.inc"
};
#undef TITLED_BUILDS

typedef void *(__cdecl *Alloc)(uint32_t);
typedef void(__cdecl *Free)(void *);
inline void let_go(int32_t *volatile *held) {   // a shared text let go (read again, as the game does)
    --**held;
    int32_t *text = *held;
    if (*(volatile int32_t *)text == 0) {
        ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
        ((Free)0xbce000)(text);
    }
}

volatile long g_titled_builds;
uint8_t *titled_build(uint8_t *self, uint8_t *description, int32_t **title, uint32_t value, const TitledBuild &kind) {
    g_titled_builds++;
    uint8_t *shared = *(uint8_t *volatile *)(uintptr_t)kind.description;
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
    ((void *(__thiscall *)(void *, void *, void *))kind.base)(self, description ? description : *(uint8_t *volatile *)(uintptr_t)kind.description, named);
    if (!title) let_go(&copy);
    uint32_t *member = (uint32_t *)(self + kind.member);
    member[3] = value;
    member[0] = 0xf80c2c;
    member[1] = 0;
    member[2] = (uint32_t)(uintptr_t)self;
    ((void(__thiscall *)(void *))0x99ff30)(member);
    *(volatile uint32_t *)(self + 0) = kind.table0;
    *(volatile uint32_t *)(self + 4) = kind.table4;
    *(volatile uint32_t *)(self + 0xd4) = kind.table_d4;
    *(volatile uint32_t *)member = kind.member_table;
    uint8_t *used = description ? description : *(uint8_t *volatile *)(uintptr_t)kind.description;
    if (!used[4]) {
        alignas(16) uint8_t text[0x40];
        void *made = ((void *(__thiscall *)(void *, uintptr_t, uint32_t))0xbeca40)(text, 0x13413dc, kind.member);
        ((void(__thiscall *)(void *, void *))0xbecc10)(used, made);
        ((void(__thiscall *)(void *))0xbec5e0)(text);
    }
    if (!description) (*(uint8_t *volatile *)(uintptr_t)kind.description)[4] = 1;
    ((void(__thiscall *)(void *))0x99ff30)(member);
    return self;
}

template <int K> uint8_t *__thiscall titled_build_copy(uint8_t *self, uint8_t *description, int32_t **title, uint32_t value) {
    return titled_build(self, description, title, value, titled_builds[K]);
}
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(titled_builds[0].at == 0x9a31d0 && titled_builds[1].at == 0x9a3440 && titled_builds[2].at == 0x9a36b0, "titled_builds.inc changed");
REWRITE("Window::BuildTitled9A3", 0x9a31d0, titled_builds_first[0], (&titled_build_copy<0>), &g_titled_builds);
REWRITE("Window::BuildTitled9A3", 0x9a3440, titled_builds_first[1], (&titled_build_copy<1>), &g_titled_builds);
REWRITE("Window::BuildTitled9A3", 0x9a36b0, titled_builds_first[2], (&titled_build_copy<2>), &g_titled_builds);
