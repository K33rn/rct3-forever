// Window::Mouse65EC80 and Path::MouseStep (one function written out twice; they differ only in the step function):
// walk from one point to another a whole unit at a time, calling a step for each point on the way (fastcall: from,
// to; then what the step is called on; no value).
//   - The distance d = to - from in x and y (each rounded to a float), the direction in each +1 or -1: when d.x is
//     below 0 it is made its size; when d.y is, it is made minus d.x (the game's slip: not its own size); the error
//     e = d.x / 2 - d.y / 2 (halves on the FPU, the difference rounded).
//   - From the start, again and again: the step (thiscall on the given object with the point); then when e < 0 the
//     point moves one in y (by the y direction) and e += d.x, else one in x and e -= d.y (each rounded where the game
//     stores it); it stops once the point is past the end in x or in y (beyond it in the walk's direction, ordered
//     compares: never with a NaN).
#include "rewrite.h"

namespace {
typedef long double ext;
inline float rf(ext x) {
    volatile float f = (float)x;
    return f;
}

volatile long g_line_walks;
template <uintptr_t STEP> void __fastcall line_walk(const float *from, const float *to, void *target) {
    g_line_walks++;
    const float zero = *(const float *)(uintptr_t)0xf264e0, half = *(const float *)(uintptr_t)0xf2651c;
    float dx = rf((ext)to[0] - from[0]), dy = rf((ext)to[1] - from[1]);
    float sx = 1.0f, sy = 1.0f;
    if (dx < zero) sx = -1.0f, dx = -dx;
    if (dy < zero) sy = -1.0f, dy = -dx;   // (as the game does: d.y becomes minus d.x, not its own size)
    float e = rf((ext)dx * half - (ext)dy * half);
    volatile float point[2];
    ((volatile uint32_t *)point)[0] = ((const uint32_t *)from)[0];   // (copied as it is)
    ((volatile uint32_t *)point)[1] = ((const uint32_t *)from)[1];
    for (;;) {
        ((void(__thiscall *)(void *, volatile float *))STEP)(target, point);
        if (e < zero) {
            point[1] = rf((ext)point[1] + sy);
            e = rf((ext)dx + e);
        } else {
            point[0] = rf((ext)point[0] + sx);
            e = rf((ext)e - dy);
        }
        if (sx < zero ? point[0] < to[0] : point[0] > to[0]) return;
        if (sy < zero ? point[1] < to[1] : point[1] > to[1]) return;
    }
}

const uint8_t line_walk_first[] = {0x83, 0xec, 0x1c, 0x56, 0x8b, 0xf2, 0xd9};
}   // namespace
REWRITE("Window::LineWalk", 0x65ec80, line_walk_first, (&line_walk<0x65eb50>), &g_line_walks);
REWRITE("Window::LineWalk", 0x715b40, line_walk_first, (&line_walk<0x714910>), &g_line_walks);
