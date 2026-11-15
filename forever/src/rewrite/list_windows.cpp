// Window::Build5996A0 and Window::Build599990 (one function written out twice; tools/family.py: list_windows.inc;
// the second keeps the third value at +0x170 and clears +0x16C, the first the other way round): build a window with
// a list (thiscall: a description or none, a title or none, three values a, b, c; gives the window).
//   - Titled (titled_head.h, default description [0x12F54BC]) and built (0x6A2620: the description, that title, 0, c).
//   - Its part at +0x164 built with the global [0x10EC9E8] (0xCD2B30); b kept at +0x174 and a at the copy's place;
//     the window's tables (+0, +4, +0xD4, +0xDC, the part's +0x164) set and the copy's other place cleared.
//   - An empty list at +0x178 (class 0xF3D87C, its ring of one node pointing at itself, the window at +0x194),
//     +0x198..+0x1A0 cleared.
//   - On the description's first use, named (0xBECA40 with 0x1400128 and 0x164) and the default marked.
//   - The list given to the window (0x6A22A0); three game values copied in (+0x1B0 [0x12FBD28], +0x1AC [0x1302E08],
//     +0x1A8 [0x12FA178]).
#include "titled_head.h"

namespace {
struct ListWindow {
    uint32_t at, kept, cleared;
};
#define LIST_WINDOWS(at, kept, cleared, ...) {at, kept, cleared},
constexpr ListWindow list_windows[] = {
#include "list_windows.inc"
};
#undef LIST_WINDOWS
#define LIST_WINDOWS(at, kept, cleared, ...) {__VA_ARGS__},
const uint8_t list_windows_first[][7] = {
#include "list_windows.inc"
};
#undef LIST_WINDOWS
const uint32_t DESCRIPTION = 0x12f54bc;

volatile long g_list_windows;
uint8_t *list_window(uint8_t *self, uint8_t *description, int32_t **title, uint32_t a, uint32_t b, uint32_t c, const ListWindow &kind) {
    g_list_windows++;
    int32_t *volatile copy;
    void *named = titled::title_for(DESCRIPTION, title, &copy);
    ((void *(__thiscall *)(void *, void *, void *, uint32_t, uint32_t))0x6a2620)(self, titled::description_or_default(description, DESCRIPTION), named, 0, c);
    titled::title_done(title, &copy);
    ((void(__thiscall *)(void *, uint32_t))0xcd2b30)(self + 0x164, *(volatile uint32_t *)(uintptr_t)0x10ec9e8);
    *(volatile uint32_t *)(self + 0x174) = b;
    *(volatile uint32_t *)(self + kind.kept) = a;
    *(volatile uint32_t *)(self + 0) = 0xf3d640;
    *(volatile uint32_t *)(self + 4) = 0xf3d628;
    *(volatile uint32_t *)(self + 0xd4) = 0xf3d614;
    *(volatile uint32_t *)(self + 0xdc) = 0xf3d610;
    *(volatile uint32_t *)(self + 0x164) = 0xf3d608;
    *(volatile uint32_t *)(self + kind.cleared) = 0;
    uint32_t *list = (uint32_t *)(self + 0x178);
    list[3] = 0;                                     // +0x184
    list[6] = 0;                                     // +0x190
    list[2] = 0;                                     // +0x180
    list[1] = (uint32_t)(uintptr_t)&list[4];         // +0x17C: its node (+0x188)
    list[5] = (uint32_t)(uintptr_t)&list[1];         // +0x18C
    list[4] = 0;                                     // +0x188
    list[0] = 0xf3d87c;
    list[7] = (uint32_t)(uintptr_t)self;             // +0x194
    *(volatile uint32_t *)(self + 0x198) = 0;
    *(volatile uint32_t *)(self + 0x19c) = 0;
    *(volatile uint32_t *)(self + 0x1a0) = 0;
    uint8_t *used = titled::description_or_default(description, DESCRIPTION);
    if (!used[4]) {
        alignas(16) uint8_t text[0x40];
        void *made = ((void *(__thiscall *)(void *, uintptr_t, uint32_t))0xbeca40)(text, 0x1400128, 0x164);
        ((void(__thiscall *)(void *, void *))0xbecc10)(used, made);
        ((void(__thiscall *)(void *))0xbec5e0)(text);
    }
    if (!description) (*(uint8_t *volatile *)(uintptr_t)DESCRIPTION)[4] = 1;
    ((void(__thiscall *)(void *, void *))0x6a22a0)(self, list);
    *(volatile uint32_t *)(self + 0x1b0) = *(volatile uint32_t *)(uintptr_t)0x12fbd28;
    *(volatile uint32_t *)(self + 0x1ac) = *(volatile uint32_t *)(uintptr_t)0x1302e08;
    *(volatile uint32_t *)(self + 0x1a8) = *(volatile uint32_t *)(uintptr_t)0x12fa178;
    return self;
}

template <int K> uint8_t *__thiscall list_window_copy(uint8_t *self, uint8_t *description, int32_t **title, uint32_t a, uint32_t b, uint32_t c) {
    return list_window(self, description, title, a, b, c, list_windows[K]);
}
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(list_windows[0].at == 0x5996a0 && list_windows[1].at == 0x599990, "list_windows.inc changed");
REWRITE("Window::BuildList", 0x5996a0, list_windows_first[0], (&list_window_copy<0>), &g_list_windows);
REWRITE("Window::BuildList", 0x599990, list_windows_first[1], (&list_window_copy<1>), &g_list_windows);
