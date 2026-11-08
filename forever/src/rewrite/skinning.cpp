// Skinning with SSE: vertices moved by the bones they hang on. The game picks these when the CPU has SSE; the maths
// is single-precision SSE, so the rewrites use the same instructions (the rest of RCT3 Forever is built for the x87).
#include "rewrite.h"
#include <xmmintrin.h>

#define SSE __attribute__((target("sse")))

// A bone's 4x4 matrix (rows of 16 bytes, 64 bytes a bone) applied to a point: x row0 + y row1 + z row2 + row3. The
// first bone of a vertex adds them in order; the others as (x row0 + y row1) + (z row2 + row3).
SSE static inline __m128 bone_point(const float *bones, uint8_t bone, const float *at) {
    const __m128 *m = (const __m128 *)(bones + bone * 16);
    __m128 v = _mm_mul_ps(_mm_set1_ps(at[0]), m[0]);
    v = _mm_add_ps(v, _mm_mul_ps(_mm_set1_ps(at[1]), m[1]));
    v = _mm_add_ps(v, _mm_mul_ps(_mm_set1_ps(at[2]), m[2]));
    return _mm_add_ps(v, m[3]);
}
SSE static inline __m128 bone_point_more(const float *bones, uint8_t bone, const float *at) {
    const __m128 *m = (const __m128 *)(bones + bone * 16);
    __m128 xy = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(at[0]), m[0]), _mm_mul_ps(_mm_set1_ps(at[1]), m[1]));
    return _mm_add_ps(xy, _mm_add_ps(_mm_mul_ps(_mm_set1_ps(at[2]), m[2]), m[3]));
}
// A weight byte (at +0x1C onward) times 1/255, rounded to float.
static inline float bone_weight(uint8_t weight) {
    uint32_t bits = 0x3b808081;   // 1/255
    float over_255;
    __builtin_memcpy(&over_255, &bits, 4);
    volatile float rounded = (float)((long double)over_255 * weight);
    return rounded;
}

// ---- Skin::Positions(bones; layout; vertices, positions): the layout {vertex size, ?, ?, then runs} gives runs of
// vertices: a word of count << 2 | extra bones (0-3), or a count of 0 then a number of vertices to pass over (0 ends).
// Each vertex (position at 0, bones at +0x18, weights at +0x1C) gets its first bone's point, times its weight when it
// has more bones, plus each further bone's point times its weight (the last bone first); the xyz go out 12 bytes apart --
static volatile long g_skins;
SSE static void __fastcall skin_positions(const float *bones, const uint16_t *layout, const uint8_t *vertex, float *out) {
    g_skins++;
    uint32_t size = layout[0];
    const uint16_t *run = layout + 3;
    for (;;) {
        uint32_t count = *run >> 2;
        if (!count) {
            uint32_t skip = run[1];
            if (!skip) return;
            vertex += skip * size;
            out += skip * 3;
            run += 2;
            continue;
        }
        uint32_t more = *run & 3;
        for (; count; count--) {
            const float *at = (const float *)vertex;
            __m128 v = bone_point(bones, vertex[0x18], at);
            if (more) {
                v = _mm_mul_ps(v, _mm_set1_ps(bone_weight(vertex[0x1c])));
                for (uint32_t k = more; k; k--)
                    v = _mm_add_ps(v, _mm_mul_ps(bone_point_more(bones, vertex[0x18 + k], at), _mm_set1_ps(bone_weight(vertex[0x1c + k]))));
            }
            float lanes[4];
            _mm_storeu_ps(lanes, v);
            out[0] = lanes[0];
            out[1] = lanes[1];
            out[2] = lanes[2];
            vertex += size;
            out += 3;
        }
        run++;
    }
}
static const uint8_t skin_positions_original[] = {0x55, 0x8b, 0xec, 0x83, 0xec, 0x14, 0x53, 0x56};
REWRITE("Skin::Positions", 0xbef020, skin_positions_original, &skin_positions, &g_skins);
REWRITE("Skin::Positions", 0xbef450, skin_positions_original, &skin_positions, &g_skins);
