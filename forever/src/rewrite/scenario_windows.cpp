// Window::Build6CF110 and Window::Build6E93E0 (one function written out twice; tools/family.py: scenario_windows.inc;
// they differ in their default description, the base's last value, the part's size and constructor and the tables):
// build a window and centre it on the screen window (thiscall: a description or none, a title or none; gives it).
//   - Titled (titled_head.h) and built (0x63A3A0: the description, that title, the copy's value); five tables set.
//   - Its part (+0x154; the copy's size, built with 0, 0; none without memory); with no description given, the
//     default marked used; the part placed (0x6B1330, 6).
//   - Centred on the window [0x12F83F0]: x = (its width - ours, stored) / 2, y = (its height, stored, - ours) / 2
//     (stored); the rectangle {x, y, x + our width, y + our height} given to Window::UpdateRect, the sums on the FPU
//     rounded where the game stores them.
#include "titled_head.h"

namespace {
struct ScenarioWindow {
    uint32_t at, description, base_value, part_size, table0, table4, table_d4, table_e8, table_f0, part_make;
};
#define SCENARIO_WINDOWS(at, description, value, size, t0, t4, td4, te8, tf0, make, ...) {at, description, value, size, t0, t4, td4, te8, tf0, make},
constexpr ScenarioWindow scenario_windows[] = {
#include "scenario_windows.inc"
};
#undef SCENARIO_WINDOWS
#define SCENARIO_WINDOWS(at, description, value, size, t0, t4, td4, te8, tf0, make, ...) {__VA_ARGS__},
const uint8_t scenario_windows_first[][7] = {
#include "scenario_windows.inc"
};
#undef SCENARIO_WINDOWS
typedef long double ext;
inline float rf(ext x) {
    volatile float f = (float)x;
    return f;
}
inline float at(const uint8_t *w, uint32_t offset) { return *(const float *)(w + offset); }

volatile long g_scenario_windows;
uint8_t *scenario_window(uint8_t *self, uint8_t *description, int32_t **title, const ScenarioWindow &kind) {
    g_scenario_windows++;
    int32_t *volatile copy;
    void *named = titled::title_for(kind.description, title, &copy);
    ((void *(__thiscall *)(void *, void *, void *, uint32_t))0x63a3a0)(self, titled::description_or_default(description, kind.description), named, kind.base_value);
    titled::title_done(title, &copy);
    *(volatile uint32_t *)(self + 0) = kind.table0;
    *(volatile uint32_t *)(self + 4) = kind.table4;
    *(volatile uint32_t *)(self + 0xd4) = kind.table_d4;
    *(volatile uint32_t *)(self + 0xe8) = kind.table_e8;
    *(volatile uint32_t *)(self + 0xf0) = kind.table_f0;
    void *memory = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(kind.part_size);
    *(void *volatile *)(self + 0x154) = memory ? ((void *(__thiscall *)(void *, uint32_t, uint32_t))kind.part_make)(memory, 0, 0) : nullptr;
    if (!description) (*(uint8_t *volatile *)(uintptr_t)kind.description)[4] = 1;
    ((void(__thiscall *)(void *, uint32_t, void *))0x6b1330)(*(void **)(self + 0x154), 6, self);
    const uint8_t *screen = *(const uint8_t *volatile *)(uintptr_t)0x12f83f0;
    const ext half = *(const float *)(uintptr_t)0xf2651c;   // 0.5
    const ext width = (ext)at(self, 0x84) - at(self, 0x7c), height = (ext)at(self, 0x88) - at(self, 0x80);
    const ext screen_width = (ext)at(screen, 0x84) - at(screen, 0x7c), screen_height = (ext)at(screen, 0x88) - at(screen, 0x80);
    volatile float rect[4];
    const float room = rf(screen_width - width);
    const float screen_height_f = rf(screen_height);
    const float y = rf(((ext)screen_height_f - height) * half);
    const ext x = (ext)room * half;
    rect[1] = y;
    rect[0] = rf(x);
    rect[2] = rf(((ext)at(self, 0x84) - at(self, 0x7c)) + x);
    rect[3] = rf(((ext)at(self, 0x88) - at(self, 0x80)) + y);
    ((void(__thiscall *)(void *, volatile float *))0x6afaf0)(self, rect);
    return self;
}

template <int K> uint8_t *__thiscall scenario_window_copy(uint8_t *self, uint8_t *description, int32_t **title) {
    return scenario_window(self, description, title, scenario_windows[K]);
}
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(scenario_windows[0].at == 0x6cf110 && scenario_windows[1].at == 0x6e93e0, "scenario_windows.inc changed");
REWRITE("Window::BuildCentred", 0x6cf110, scenario_windows_first[0], (&scenario_window_copy<0>), &g_scenario_windows);
REWRITE("Window::BuildCentred", 0x6e93e0, scenario_windows_first[1], (&scenario_window_copy<1>), &g_scenario_windows);
