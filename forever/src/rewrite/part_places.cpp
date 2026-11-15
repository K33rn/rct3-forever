// Window::Place6DAB30, Window::Place6DB500 and Window::Place6DC010 (one function written out three times, the same
// instructions): place a window's part (+0x11C) twice (thiscall).
//   - The window laid out first (0x6D4870).
//   - The part's rectangle (Window::UpdateRect, 0x6AFAF0, on the part) made {x: the +0xF4 part's +0x84 plus 10
//     (the game's constant), y: the +0xD4 part's +0x80, x plus the part's own width (+0x84 - +0x7C), y plus its own
//     height (+0x88 - +0x80)}; the sums on the FPU, each corner stored as a float.
//   - The part asked its size (its slot 0xF4, given a place), then its rectangle made {its x (+0x7C), its y (+0x80),
//     x + the width, y + the height}.
#include "rewrite.h"

namespace {
typedef long double ext;
inline float rf(ext x) {   // stored to memory as a float
    volatile float f = (float)x;
    return f;
}
inline const float *at(uint8_t *object, uint32_t offset) { return (const float *)(object + offset); }

volatile long g_part_places;
void __fastcall part_place(uint8_t *self) {
    g_part_places++;
    ((void(__thiscall *)(void *))0x6d4870)(self);
    uint8_t *a = *(uint8_t **)(self + 0xd4), *b = *(uint8_t **)(self + 0xf4), *part = *(uint8_t **)(self + 0x11c);
    const ext top = *at(a, 0x80);
    const ext left = (ext)*at(b, 0x84) + *(const volatile float *)(uintptr_t)0xf275f4;   // + 10
    const ext width = (ext)*at(part, 0x84) - *at(part, 0x7c);
    volatile float rect[8];
    rect[0] = rf(left);
    rect[1] = rf(top);
    rect[2] = rf(width + left);
    rect[3] = rf(((ext)*at(part, 0x88) - *at(part, 0x80)) + top);
    typedef void(__thiscall * Update)(void *, volatile float *);
    ((Update)0x6afaf0)(part, rect);
    part = *(uint8_t **)(self + 0x11c);
    const float *size = ((const float *(__thiscall *)(void *, volatile float *))(*(void ***)part)[0xf4 / 4])(part, rect);
    part = *(uint8_t **)(self + 0x11c);
    const ext x = *at(part, 0x7c), y = *at(part, 0x80);
    rect[4] = rf(x);
    rect[5] = rf(y);
    rect[6] = rf(x + size[0]);
    rect[7] = rf(y + size[1]);
    ((Update)0x6afaf0)(part, rect + 4);
}

const uint8_t part_places_first[] = {0x83, 0xec, 0x20, 0x56, 0x8b, 0xf1, 0xe8};
}   // namespace
REWRITE("Window::PlacePart", 0x6dab30, part_places_first, &part_place, &g_part_places);
REWRITE("Window::PlacePart", 0x6db500, part_places_first, &part_place, &g_part_places);
REWRITE("Window::PlacePart", 0x6dc010, part_places_first, &part_place, &g_part_places);
