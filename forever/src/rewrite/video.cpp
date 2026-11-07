// Video colour conversion (YUV to RGB) tables, filled in at start-up: 256 entries each, entry i the colour factor times i,
// rounded down, kept to MASK and moved up by SHIFT (fixed point). The product is taken in single precision.
#include "rewrite.h"

static volatile long g_yuv_tables;
template <uint32_t FACTOR_BITS, uint32_t MASK, uint32_t SHIFT, uintptr_t TABLE> static void __cdecl yuv_table() {
    g_yuv_tables++;
    float factor;
    uint32_t bits = FACTOR_BITS;
    __builtin_memcpy(&factor, &bits, 4);
    for (int32_t i = 0; i < 256; i++) {
        volatile float product = factor * (float)i;   // rounded to single precision, as the game's own values are
        float value = product;
        int64_t whole = (int64_t)value;
        if ((float)whole > value) whole--;   // rounded down
        ((uint32_t *)TABLE)[i] = ((uint32_t)whole & MASK) << SHIFT;
    }
}
static const uint8_t yuv_table_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf8, 0x83, 0xec};
REWRITE("Video::YuvTable", 0xef42a0, yuv_table_original, (&yuv_table<0x3fcc49ba, 0x1ff, 0, 0x13e43c0>), &g_yuv_tables);   // 1.596: V in red
REWRITE("Video::YuvTable", 0xef7c10, yuv_table_original, (&yuv_table<0x3f5020c5, 0x1ff, 10, 0x13e3fc0>), &g_yuv_tables);  // 0.813: V in green
REWRITE("Video::YuvTable", 0xefb880, yuv_table_original, (&yuv_table<0x400126e9, 0x3ff, 20, 0x13e37c0>), &g_yuv_tables);  // 2.018: U in blue
REWRITE("Video::YuvTable", 0xeff4f0, yuv_table_original, (&yuv_table<0x3ec83127, 0x1ff, 10, 0x13e3bc0>), &g_yuv_tables);  // 0.391: U in green
