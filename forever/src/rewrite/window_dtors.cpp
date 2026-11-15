// Four designer windows' destructors (0xACE700, 0xAEA650, 0xAEB410, 0xAEC120: one function written out four times;
// tools/family.py: window_dtors.inc; they differ in their four tables):
//   - the window's four tables put back (+0, +4, +0xDC, +0xE4);
//   - the part at +0x104 told to finish (its slot 0x100 with 0, 0), when there is one;
//   - the parts at +0x118, +0x114 and +0x11C deleted (their slot 0 with 1), each when there is one;
//   - the reference at +0xE8 destroyed (ObjectRef's 0x40F730), +0xE4's table made the base one (0xF93E50) and the
//     part at +0xDC destroyed (0x6835A0); then the window's own destructor (Window::~, 0x6B0EA0).
#include "rewrite.h"

namespace {
struct WindowDtor {
    uint32_t at, table, second, third, fourth;
};
#define WINDOW_DTORS(at, table, second, third, fourth, ...) {at, table, second, third, fourth},
constexpr WindowDtor window_dtors[] = {
#include "window_dtors.inc"
};
#undef WINDOW_DTORS
#define WINDOW_DTORS(at, table, second, third, fourth, ...) {__VA_ARGS__},
const uint8_t window_dtors_first[][7] = {
#include "window_dtors.inc"
};
#undef WINDOW_DTORS
inline void call(uintptr_t fn, void *self) { ((void(__thiscall *)(void *))fn)(self); }

volatile long g_window_dtors;
void window_dtor(uint8_t *self, const WindowDtor &kind) {
    g_window_dtors++;
    *(volatile uint32_t *)(self + 0) = kind.table;
    *(volatile uint32_t *)(self + 4) = kind.second;
    *(volatile uint32_t *)(self + 0xdc) = kind.third;
    *(volatile uint32_t *)(self + 0xe4) = kind.fourth;
    if (void *part = *(void **)(self + 0x104)) ((void(__thiscall *)(void *, uint32_t, uint32_t))(*(void ***)part)[0x100 / 4])(part, 0, 0);
    static const uint32_t owned[] = {0x118, 0x114, 0x11c};
    for (uint32_t at : owned)
        if (void *part = *(void **)(self + at)) ((void(__thiscall *)(void *, uint32_t))(*(void ***)part)[0])(part, 1);
    call(0x40f730, self + 0xe8);
    *(volatile uint32_t *)(self + 0xe4) = 0xf93e50;
    call(0x6835a0, self + 0xdc);
    call(0x6b0ea0, self);
}

template <int K> void __fastcall window_dtor_copy(uint8_t *self) { window_dtor(self, window_dtors[K]); }
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(window_dtors[0].at == 0xace700 && window_dtors[1].at == 0xaea650 && window_dtors[2].at == 0xaeb410 && window_dtors[3].at == 0xaec120,
              "window_dtors.inc changed");
REWRITE("Designer::Destructor", 0xace700, window_dtors_first[0], (&window_dtor_copy<0>), &g_window_dtors);
REWRITE("Designer::Destructor", 0xaea650, window_dtors_first[1], (&window_dtor_copy<1>), &g_window_dtors);
REWRITE("Designer::Destructor", 0xaeb410, window_dtors_first[2], (&window_dtor_copy<2>), &g_window_dtors);
REWRITE("Designer::Destructor", 0xaec120, window_dtors_first[3], (&window_dtor_copy<3>), &g_window_dtors);
