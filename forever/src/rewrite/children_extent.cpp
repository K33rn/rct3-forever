// Window::Point9C2810 and Window::Point9C2AE0 (the same bytes twice): how far a window's children reach (thiscall;
// where to put it; no value).
//   - Over the children (the array at +0xE0, the count at +0xE4; none when the count is below 1), in order: the
//     largest right edge (+0x84) and the largest bottom (+0x88), each starting at 0 and taken only when it is larger
//     (so a NaN never is).
//   - Given as {right, bottom}.
#include "rewrite.h"

namespace {
volatile long g_children_extent;
void __fastcall children_extent(uint8_t *self, void *, float *out) {
    g_children_extent++;
    float right = 0.0f, bottom = *(const float *)(uintptr_t)0xf264e0;   // (the game's 0)
    const int32_t count = *(int32_t *)(self + 0xe4);
    uint8_t *const *children = *(uint8_t *const **)(self + 0xe0);
    for (int32_t i = 0; i < count; i++) {
        const float r = *(const float *)(children[i] + 0x84), b = *(const float *)(children[i] + 0x88);
        if (r > right) right = r;
        if (bottom < b) bottom = b;
    }
    out[0] = right;
    out[1] = bottom;
}

const uint8_t children_extent_first[] = {0x83, 0xec, 0x08, 0xd9, 0x05, 0xe0, 0x64};
}   // namespace
REWRITE("Window::ChildrenExtent", 0x9c2810, children_extent_first, &children_extent, &g_children_extent);
REWRITE("Window::ChildrenExtent", 0x9c2ae0, children_extent_first, &children_extent, &g_children_extent);
