// Three window constructors written out from one (0x4F0390, 0x70F2D0, 0xAA6060; tools/family.py: titled_windows.inc, the
// copies differing only in their default description, a game global, and their two class tables). Given a description
// (or none: the default) and a title (or none): with a title, it is made the game's name (the default description's +8,
// as a new text, or the empty one) with " - " and the title after it, kept in the title; with none, a copy of the
// default description's title is used. The base window (0x69A280) is built for the description with that title and a
// reference to the text "BackgroundCurvedMirrored" in the game's text table (let go after), then the copy's tables are set.
#include "rewrite.h"

namespace {
struct TitledWindow {
    uint32_t at, description, table, second_table;
};
#define TITLED_WINDOWS(at, description, table, second_table, ...) {at, description, table, second_table},
constexpr TitledWindow titled_windows[] = {
#include "titled_windows.inc"
};
#undef TITLED_WINDOWS
#define TITLED_WINDOWS(at, description, table, second_table, ...) {__VA_ARGS__},
const uint8_t titled_windows_first[][7] = {
#include "titled_windows.inc"
};
#undef TITLED_WINDOWS

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

volatile long g_titled_windows;
uint8_t *titled_window(uint8_t *self, uint8_t *description, int32_t **title, const TitledWindow &kind) {
    g_titled_windows++;
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
        // kept in the title: the old text let go (unless it is the same one), the new one held
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
        named = ((void *(__thiscall *)(void *, void *))0x41c740)((void *)&copy, shared);   // a copy of the default title
    }
    uint8_t *used = description ? description : *(uint8_t *volatile *)(uintptr_t)kind.description;
    alignas(16) uint8_t reference[0x40];
    void *made = ((void *(__thiscall *)(void *, uintptr_t, uintptr_t, uint32_t))0x41cb90)(reference, 0xf35d2c, 0x1303cfc, 1);
    ((void *(__thiscall *)(void *, void *, void *, void *, uint32_t))0x69a280)(self, used, named, made, 0);
    ((void(__thiscall *)(void *))0xbd1880)(reference);
    if (!title) let_go(&copy);
    *(volatile uint32_t *)self = kind.table;
    *(volatile uint32_t *)(self + 4) = kind.second_table;
    return self;
}

template <int K>
uint8_t *__thiscall titled_window_copy(uint8_t *self, uint8_t *description, int32_t **title) {
    return titled_window(self, description, title, titled_windows[K]);
}
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(titled_windows[0].at == 0x4f0390 && titled_windows[1].at == 0x70f2d0 && titled_windows[2].at == 0xaa6060, "titled_windows.inc changed");
REWRITE("Window::TitledFromDescription", 0x4f0390, titled_windows_first[0], (&titled_window_copy<0>), &g_titled_windows);
REWRITE("Window::TitledFromDescription", 0x70f2d0, titled_windows_first[1], (&titled_window_copy<1>), &g_titled_windows);
REWRITE("Window::TitledFromDescription", 0xaa6060, titled_windows_first[2], (&titled_window_copy<2>), &g_titled_windows);
