// A guest's taste for intense rides. Each guest has a band [Min, Max] of intensity they accept and a preference
// 0-3 that says where in the band their ideal lies. The scenario's IntensityPreference setting can give every new
// guest the same preference and a fixed band.
#include "rewrite.h"

struct GuestIntensity {   // fields of the guest object
    int32_t preference;   // +0x434: 0-3
    float max;            // +0x438
    float min;            // +0x43c
};
static GuestIntensity &intensity(void *guest) { return *(GuestIntensity *)((char *)guest + 0x434); }

static volatile long g_sets, g_ideals;

// SetPreference(preference, set_band): anything outside 0-3 becomes 1. With set_band the band becomes the
// preference's fixed one: 0 -> [0, 5], 1 -> [1, 8], 2 -> [4, 10], 3 -> [6, 12].
static void __fastcall set_preference(void *guest, void *, int32_t preference, uint8_t set_band) {
    g_sets++;
    static const float band_min[4] = {0, 1, 4, 6}, band_max[4] = {5, 8, 10, 12};
    if (preference < 0 || preference > 3) preference = 1;
    GuestIntensity &g = intensity(guest);
    g.preference = preference;
    if (set_band) {
        g.min = band_min[preference];
        g.max = band_max[preference];
    }
}

// Ideal: Min + f x (Max - Min), f = 0.25 / 0.5 / 0.7 / 0.9 for preference 0-3 (0.5 for any other value).
static float __fastcall ideal(void *guest) {
    g_ideals++;
    const GuestIntensity &g = intensity(guest);
    float f;
    switch (g.preference) {
    case 0: f = 0.25f; break;
    case 2: f = 0.7f; break;
    case 3: f = 0.9f; break;
    default: f = 0.5f; break;
    }
    return (g.max - g.min) * f + g.min;
}

static const uint8_t set_original[] = {0x8b, 0x44, 0x24, 0x04, 0x83, 0xec, 0x20, 0x85};
static const uint8_t ideal_original[] = {0x8b, 0x81, 0x34, 0x04, 0x00, 0x00, 0x83, 0xe8};

REWRITE("Guest::SetIntensityPreference", 0x758bb0, set_original, set_preference, &g_sets);
REWRITE("Guest::IdealIntensity", 0x758c30, ideal_original, ideal, &g_ideals);
