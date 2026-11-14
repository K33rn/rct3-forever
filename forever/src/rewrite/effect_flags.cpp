// Effect::Flag48AFF0 and its copies (four; tools/family.py: effect_flags.inc, the copies differing only in the three
// functions they call). Given a ray {its origin, its direction} (thiscall, one argument): when the effect is flagged
// (bit 4 of the word its part at +8 points to), it is brought up to date first (p0, with 0). Then the effect's box (low:
// its place x, its part's +4, its place y; high: place x + size x, its part's +8, size y + place y, the sums rounded to
// floats, the others copied as they are) is tried against the ray (0xCDA7F0, Ray::BoxSlabs). Met: the ray is passed on
// (p2, with 0, the effect's place and size) and that answer returned. Missed: -2 (what is left of the game's switch; p1
// is never reached).
#include "rewrite.h"

namespace {
struct EffectFlag {
    uint32_t at, update, other, hit;
};
#define EFFECT_FLAGS(at, update, other, hit, ...) {at, update, other, hit},
constexpr EffectFlag effect_flags[] = {
#include "effect_flags.inc"
};
#undef EFFECT_FLAGS
#define EFFECT_FLAGS(at, update, other, hit, ...) {__VA_ARGS__},
const uint8_t effect_flags_first[][7] = {
#include "effect_flags.inc"
};
#undef EFFECT_FLAGS

volatile long g_effect_flags;
int32_t effect_flag(uint8_t *self, void *const *ray, const EffectFlag &kind) {
    g_effect_flags++;
    if (**(uint32_t *volatile *)(self + 8) >> 4 & 1) ((void(__thiscall *)(void *, uint32_t))kind.update)(self, 0);
    const uint32_t *part = *(const uint32_t *volatile *)(self + 8);
    const float *place = (const float *)(self + 0xc), *size = (const float *)(self + 0x14);
    volatile uint32_t low[3], high[3];
    volatile float far_x = (long double)place[0] + size[0], far_y = (long double)size[1] + place[1];
    high[0] = *(volatile uint32_t *)&far_x, high[1] = *(const volatile uint32_t *)&part[2], high[2] = *(volatile uint32_t *)&far_y;
    // (copied as bits, not loaded as floats: a signalling NaN stays one)
    low[0] = *(const volatile uint32_t *)(self + 0xc), low[1] = *(const volatile uint32_t *)&part[1], low[2] = *(const volatile uint32_t *)(self + 0x10);
    uint32_t found;
    const bool met = ((uint8_t(__fastcall *)(void *, void *, volatile uint32_t *, volatile uint32_t *, uint32_t *))0xcda7f0)(ray[0], ray[1], low, high, &found) != 0;
    if (!met) return -2;
    return ((int32_t(__thiscall *)(void *, void *const *, uint32_t, const float *, const float *))kind.hit)(self, ray, 0, place, size);
}

template <int K>
int32_t __thiscall effect_flag_copy(uint8_t *self, void *const *ray) {
    return effect_flag(self, ray, effect_flags[K]);
}
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(effect_flags[0].at == 0x48aff0 && effect_flags[1].at == 0x7b7e70 && effect_flags[2].at == 0xb5e100 && effect_flags[3].at == 0xb74190,
              "effect_flags.inc changed");
REWRITE("Effect::RayHit", 0x48aff0, effect_flags_first[0], (&effect_flag_copy<0>), &g_effect_flags);
REWRITE("Effect::RayHit", 0x7b7e70, effect_flags_first[1], (&effect_flag_copy<1>), &g_effect_flags);
REWRITE("Effect::RayHit", 0xb5e100, effect_flags_first[2], (&effect_flag_copy<2>), &g_effect_flags);
REWRITE("Effect::RayHit", 0xb74190, effect_flags_first[3], (&effect_flag_copy<3>), &g_effect_flags);
