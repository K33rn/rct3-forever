// Effect::Bounds48BD10 and Effect::Bounds48BC60, each written out four times (tools/family.py: bounds_a.inc, bounds_b.inc;
// the copies differ only in the three functions they call). Given a box {low x, y, z, high x, y, z} (thiscall, one
// argument): when the effect is flagged (bit 4 of the word its part at +8 points to), it is brought up to date first
// (p0, with 0). Then whether the effect lies within the box on all six sides: its place (+0xC, +0x10) and size (+0x14,
// +0x18) and its part's +4 and +8 against the box, the far ends added on the FPU. Within: the box is grown to it (p2,
// with 0, its place and its size), and that answer returned. Not within: -2 (what is left of the game's switch; p1 is
// never reached). The two families differ only in a comparison that cannot be made (a NaN): BD10's sides then fail,
// BC60's pass.
#include "rewrite.h"

namespace {
struct Bounds {
    uint32_t at, update, other, grow;
};
#define BOUNDS_A(at, update, other, grow, ...) {at, update, other, grow},
#define BOUNDS_B(at, update, other, grow, ...) {at, update, other, grow},
constexpr Bounds bounds_a[] = {
#include "bounds_a.inc"
};
constexpr Bounds bounds_b[] = {
#include "bounds_b.inc"
};
#undef BOUNDS_A
#undef BOUNDS_B
#define BOUNDS_A(at, update, other, grow, ...) {__VA_ARGS__},
#define BOUNDS_B(at, update, other, grow, ...) {__VA_ARGS__},
const uint8_t bounds_a_first[][7] = {
#include "bounds_a.inc"
};
const uint8_t bounds_b_first[][7] = {
#include "bounds_b.inc"
};
#undef BOUNDS_A
#undef BOUNDS_B

typedef long double x87;
// a <= b, and a >= b: when they cannot be compared, false (BD10) or true (BC60)
template <bool NAN_PASSES>
inline bool at_most(x87 a, x87 b) {
    return NAN_PASSES ? !(a > b) : a <= b;
}
template <bool NAN_PASSES>
inline bool at_least(x87 a, x87 b) {
    return NAN_PASSES ? !(a < b) : a >= b;
}

volatile long g_effect_bounds;
template <bool NAN_PASSES>
int32_t bounds(uint8_t *self, const float *box, const Bounds &kind) {
    g_effect_bounds++;
    if (**(uint32_t *volatile *)(self + 8) >> 4 & 1) ((void(__thiscall *)(void *, uint32_t))kind.update)(self, 0);
    const float *place = (const float *)(self + 0xc), *size = (const float *)(self + 0x14);
    const float *part = *(const float *volatile *)(self + 8);
    const bool within = at_most<NAN_PASSES>(place[0], box[3]) && at_most<NAN_PASSES>(part[1], box[4]) && at_most<NAN_PASSES>(place[1], box[5]) &&
                        at_least<NAN_PASSES>((x87)place[0] + size[0], box[0]) && at_least<NAN_PASSES>(part[2], box[1]) &&
                        at_least<NAN_PASSES>((x87)size[1] + place[1], box[2]);
    if (!within) return -2;
    return ((int32_t(__thiscall *)(void *, const float *, uint32_t, const float *, const float *))kind.grow)(self, box, 0, place, size);
}

template <int K>
int32_t __thiscall bounds_a_copy(uint8_t *self, const float *box) {
    return bounds<false>(self, box, bounds_a[K]);
}
template <int K>
int32_t __thiscall bounds_b_copy(uint8_t *self, const float *box) {
    return bounds<true>(self, box, bounds_b[K]);
}
}   // namespace
// (the addresses again, as numbers, checked against the tables)
static_assert(bounds_a[0].at == 0x48bd10 && bounds_a[1].at == 0x7b9300 && bounds_a[2].at == 0xb5ead0 && bounds_a[3].at == 0xb74dd0, "bounds_a.inc changed");
static_assert(bounds_b[0].at == 0x48bc60 && bounds_b[1].at == 0x7b94e0 && bounds_b[2].at == 0xb5ecb0 && bounds_b[3].at == 0xb74fb0, "bounds_b.inc changed");
REWRITE("Effect::WithinBox", 0x48bd10, bounds_a_first[0], (&bounds_a_copy<0>), &g_effect_bounds);
REWRITE("Effect::WithinBox", 0x7b9300, bounds_a_first[1], (&bounds_a_copy<1>), &g_effect_bounds);
REWRITE("Effect::WithinBox", 0xb5ead0, bounds_a_first[2], (&bounds_a_copy<2>), &g_effect_bounds);
REWRITE("Effect::WithinBox", 0xb74dd0, bounds_a_first[3], (&bounds_a_copy<3>), &g_effect_bounds);
REWRITE("Effect::WithinBox", 0x48bc60, bounds_b_first[0], (&bounds_b_copy<0>), &g_effect_bounds);
REWRITE("Effect::WithinBox", 0x7b94e0, bounds_b_first[1], (&bounds_b_copy<1>), &g_effect_bounds);
REWRITE("Effect::WithinBox", 0xb5ecb0, bounds_b_first[2], (&bounds_b_copy<2>), &g_effect_bounds);
REWRITE("Effect::WithinBox", 0xb74fb0, bounds_b_first[3], (&bounds_b_copy<3>), &g_effect_bounds);
