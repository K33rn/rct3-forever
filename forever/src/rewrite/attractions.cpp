// Attractions other than rides (research notes: they add to the park's IdealGuests next to the rides'
// attractivity).
#include "rewrite.h"

static volatile long g_ideal;

// IdealGuests contribution: three guests for each unit of the value at +0x70.
static int32_t __fastcall ideal_guests(void *attraction) {
    g_ideal++;
    return *(int32_t *)((char *)attraction + 0x70) * 3;
}

static const uint8_t ideal_original[] = {0x8b, 0x41, 0x70, 0x8d, 0x04, 0x40, 0xc3};

REWRITE("Attraction::IdealGuests", 0xa88450, ideal_original, ideal_guests, &g_ideal);
