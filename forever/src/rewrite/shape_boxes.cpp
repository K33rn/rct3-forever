// Scenery::Shapes (two copies; they differ in the check run first): a box (its low and high corners) turned by a
// matrix (rows of four floats: x, y, z axes and the place) and put back in order (fastcall: the object, the low corner;
// then the high corner and the matrix on the stack; ret 8).
//   - The copy's check run on the high corner (given the object as it came, in ecx).
//   - The box's height (high y - low y, kept as a float).
//   - Both corners turned: each new coordinate the corner's three coordinates times the matrix's column, plus the place,
//     added in the order the game adds them (it differs between the corners).
//   - x: when the new high x (before it is rounded to a float) is below the low x, the two swapped. z: when the low z is
//     above the high z, swapped.
//   - y: the high y less the height (before rounding), when below the low y, becomes the low y; the low y plus the
//     height (rounded to a float), when above the high y, becomes the high y.
// The sums are worked at the FPU's width (long double: the same x87 steps at the same precision) and rounded to floats
// where the game stores them; the compares see what the game's compares see (a value too small for a float is not yet
// 0 there).
#include "rewrite.h"

namespace {
volatile long g_shape_boxes;
using W = long double;
// a value rounded to a float, as the game's float stores do (through memory: the compiler may otherwise keep a float at
// the FPU's width)
float rounded(W v) {
    volatile float f = (float)v;
    return f;
}
template <uint32_t CHECK> void __fastcall shape_box(void *object, float *low, float *high, const float *m) {
    g_shape_boxes++;
    ((void(__thiscall *)(void *, float *))CHECK)(object, high);
    float height = rounded((W)high[1] - low[1]);
    W lx = low[0], ly = low[1], lz = low[2];
    W z = ((m[10] * lz + m[2] * lx) + ly * m[6]) + m[14];
    W x = ((lz * m[8] + lx * m[0]) + m[4] * ly) + m[12];
    W y = ((lz * m[9] + m[5] * ly) + lx * m[1]) + m[13];
    low[2] = (float)z, low[0] = (float)x, low[1] = (float)y;
    W hx = high[0], hy = high[1], hz = high[2];
    W nx = ((m[8] * hz + m[4] * hy) + hx * m[0]) + m[12];
    W ny = ((m[9] * hz + m[5] * hy) + m[1] * hx) + m[13];
    W nz = ((m[10] * hz + m[6] * hy) + m[2] * hx) + m[14];
    high[0] = (float)nx, high[1] = (float)ny, high[2] = (float)nz;
    if (nx < low[0]) {
        float t = low[0];
        low[0] = (float)nx, high[0] = t;
    }
    if (low[2] > high[2]) {
        float t = high[2];
        high[2] = low[2], low[2] = t;
    }
    W below = (W)high[1] - height;
    float above = rounded(height + (W)low[1]);
    if (below < low[1]) low[1] = (float)below;
    if (above > high[1]) high[1] = above;
}

const uint8_t shape_box_first[][7] = {{0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83}};
}   // namespace
REWRITE("Scenery::ShapeBox", 0x864010, shape_box_first[0], &shape_box<0x863c30>, &g_shape_boxes);
REWRITE("Scenery::ShapeBox", 0x864620, shape_box_first[0], &shape_box<0x864150>, &g_shape_boxes);
