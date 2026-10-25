// The park's entrances (ParkEntranceBehaviour objects): a global list the guest spawner picks from.
#include "rewrite.h"
#include <math.h>

static int32_t &g_count = *(int32_t *)0x13061a8;
static void **&g_list = *(void ***)0x13061a4;

static float(__cdecl *const random_unit)() = (float(__cdecl *)())0x475180;   // Random::Unit

static volatile long g_counts, g_ats, g_randoms;

static int32_t __cdecl count() {
    g_counts++;
    return g_count;
}

// At: ecx = index (not checked, as in the game)
static void *__fastcall at(int32_t index) {
    g_ats++;
    return g_list[index];
}

// Random: an entrance picked with equal chances, or null when the park has none.
static void *__cdecl random_entrance() {
    g_randoms++;
    if (g_count <= 0) return nullptr;
    float pick = random_unit() * g_count;   // [0, count), rounded to a float first as in the game
    return g_list[(int32_t)floorf(pick)];
}

static const uint8_t count_original[] = {0xa1, 0xa8, 0x61, 0x30, 0x01, 0xc3};
static const uint8_t at_original[] = {0xa1, 0xa4, 0x61, 0x30, 0x01, 0x8b, 0x04, 0x88};
static const uint8_t random_original[] = {0xa1, 0xa8, 0x61, 0x30, 0x01, 0x83, 0xec, 0x0c};

REWRITE("ParkEntrances::Count", 0x708960, count_original, count, &g_counts);
REWRITE("ParkEntrances::At", 0x708970, at_original, at, &g_ats);
REWRITE("ParkEntrances::Random", 0x708980, random_original, random_entrance, &g_randoms);
