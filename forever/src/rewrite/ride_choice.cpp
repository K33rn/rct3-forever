// How guests choose rides: what the game keeps on the ride about the choices made.
#include "rewrite.h"

static volatile long g_logs;

// LogScorePart(part, value): each part of a guest's score for this ride is kept as a running average over about
// six choices: log = (5 x log + value) / 6. The log is an array at +0x12c of the ride.
static void __fastcall log_score_part(void *ride, void *, int32_t part, float value) {
    g_logs++;
    float *log = *(float **)((char *)ride + 0x12c);
    log[part] = (log[part] * 5.0f + value) * (1.0f / 6.0f);
}

static const uint8_t log_original[] = {0x8b, 0x81, 0x2c, 0x01, 0x00, 0x00, 0x8b, 0x4c};

REWRITE("Ride::LogScorePart", 0x4bfc00, log_original, log_score_part, &g_logs);
