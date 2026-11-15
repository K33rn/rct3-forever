// Window::BuildAE6A10 and Window::BuildAE7920 (one function written out twice; tools/family.py: panel_windows.inc;
// they differ in their default description, the panel's size and constructor, their two tables and the panel's
// size on screen): a designer window holding one panel (thiscall: a description or none, a title or none, a value;
// gives the window).
//   - Titled (titled_head.h) and built (0x67DAC0: the description, that title, 1); its two tables set.
//   - The panel (+0xDC; the copy's size, built with 0, 0 and the value; none without memory) laid out (0x6B1640, 6,
//     the window) at {0, 0, S, S}: S 320 or 256.
#include "titled_head.h"

namespace {
struct PanelWindow {
    uint32_t at, description, panel_size, table0, table4, panel_make, side;
};
#define PANEL_WINDOWS(at, description, size, t0, t4, make, side, ...) {at, description, size, t0, t4, make, side},
constexpr PanelWindow panel_windows[] = {
#include "panel_windows.inc"
};
#undef PANEL_WINDOWS
#define PANEL_WINDOWS(at, description, size, t0, t4, make, side, ...) {__VA_ARGS__},
const uint8_t panel_windows_first[][7] = {
#include "panel_windows.inc"
};
#undef PANEL_WINDOWS

volatile long g_panel_windows;
uint8_t *panel_window(uint8_t *self, uint8_t *description, int32_t **title, uint32_t value, const PanelWindow &kind) {
    g_panel_windows++;
    int32_t *volatile copy;
    void *named = titled::title_for(kind.description, title, &copy);
    ((void *(__thiscall *)(void *, void *, void *, uint32_t))0x67dac0)(self, titled::description_or_default(description, kind.description), named, 1);
    titled::title_done(title, &copy);
    *(volatile uint32_t *)(self + 0) = kind.table0;
    *(volatile uint32_t *)(self + 4) = kind.table4;
    void *memory = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(kind.panel_size);
    uint8_t *panel = memory ? (uint8_t *)((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))kind.panel_make)(memory, 0, 0, value) : nullptr;
    *(uint8_t *volatile *)(self + 0xdc) = panel;
    volatile uint32_t rect[4] = {0, 0, kind.side, kind.side};
    ((void(__thiscall *)(void *, uint32_t, void *, volatile uint32_t *))0x6b1640)(panel, 6, self, rect);
    return self;
}

template <int K> uint8_t *__thiscall panel_window_copy(uint8_t *self, uint8_t *description, int32_t **title, uint32_t value) {
    return panel_window(self, description, title, value, panel_windows[K]);
}
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(panel_windows[0].at == 0xae6a10 && panel_windows[1].at == 0xae7920, "panel_windows.inc changed");
REWRITE("Designer::BuildPanelWindow", 0xae6a10, panel_windows_first[0], (&panel_window_copy<0>), &g_panel_windows);
REWRITE("Designer::BuildPanelWindow", 0xae7920, panel_windows_first[1], (&panel_window_copy<1>), &g_panel_windows);
