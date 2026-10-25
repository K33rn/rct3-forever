// The game's random numbers: one generator for the whole simulation (guests, rides, weather, spawning...).
// It is the "minimal standard" Lehmer generator MINSTD: state' = state x 48271 mod (2^31 - 1). The state is kept
// XORed with a fixed mask; saving and restoring it (the save file, replays) uses that stored form.
#include "rewrite.h"

static const uint32_t STATE_MASK = 0x075bd924;
static const int32_t MODULUS = 0x7fffffff;   // 2^31 - 1
static const int32_t MULTIPLIER = 48271;

static uint32_t &g_stored = *(uint32_t *)0x10ec9e0;   // the state XOR STATE_MASK

static volatile long g_set, g_get, g_next, g_unit;

// One step of the generator, by Schrage's method: with q = s / 44488 (44488 = modulus / 48271),
// s x 48271 - q x modulus, in 32-bit arithmetic, is s x 48271 mod (2^31 - 1), give or take one modulus. The
// division is a multiplication by 2^46 / 44488. The same arithmetic as the game is used so that any stored state,
// also one written by a mod or an old save, steps the same way.
static int32_t step(int32_t s) {
    int64_t wide = (int64_t)s * 0x5e4789c9;                  // s x 2^46 / 44488
    int32_t q = (int32_t)(wide >> 32) >> 14;                 // s / 44488, rounded down...
    q += (uint32_t)q >> 31;                                  // ...then towards zero
    int32_t r = (int32_t)((uint32_t)s * MULTIPLIER - (uint32_t)q * MODULUS);
    if (r < 0) r += MODULUS;
    return r;
}

// SetState: ecx = a state in its stored form (as returned by GetState).
static void __fastcall set_state(uint32_t stored) {
    g_set++;
    g_stored = stored;
}

static uint32_t __cdecl get_state() {
    g_get++;
    return g_stored;
}

// Advance: one step; returns the new state in its stored form.
static uint32_t __cdecl advance() {
    g_next++;
    g_stored = (uint32_t)step((int32_t)(g_stored ^ STATE_MASK)) ^ STATE_MASK;
    return g_stored;
}

// Unit: one step, and a number in [0, 1) from the low 23 bits of the new state.
static float __cdecl unit() {
    g_unit++;
    int32_t s = step((int32_t)(g_stored ^ STATE_MASK));
    g_stored = (uint32_t)s ^ STATE_MASK;
    return (float)(s & 0x7fffff) * (1.0f / 8388608.0f);
}

static const uint8_t set_state_original[] = {0x89, 0x0d, 0xe0, 0xc9, 0x0e, 0x01, 0xc3};
static const uint8_t get_state_original[] = {0xa1, 0xe0, 0xc9, 0x0e, 0x01, 0xc3};
static const uint8_t advance_original[] = {0x8b, 0x0d, 0xe0, 0xc9, 0x0e, 0x01, 0x81, 0xf1};
static const uint8_t unit_original[] = {0x51, 0x8b, 0x0d, 0xe0, 0xc9, 0x0e, 0x01, 0x81};

REWRITE("Random::SetState", 0x475170, set_state_original, set_state, &g_set);
REWRITE("Random::GetState", 0x475220, get_state_original, get_state, &g_get);
REWRITE("Random::Advance", 0x4751e0, advance_original, advance, &g_next);
REWRITE("Random::Unit", 0x475180, unit_original, unit, &g_unit);
