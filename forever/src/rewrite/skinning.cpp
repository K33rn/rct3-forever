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
// The normal turned by a bone (no move): x row0 + y row1 + z row2.
SSE static inline __m128 bone_normal(const float *bones, uint8_t bone, const float *n) {
    const __m128 *m = (const __m128 *)(bones + bone * 16);
    __m128 v = _mm_mul_ps(_mm_set1_ps(n[0]), m[0]);
    v = _mm_add_ps(v, _mm_mul_ps(_mm_set1_ps(n[1]), m[1]));
    return _mm_add_ps(v, _mm_mul_ps(_mm_set1_ps(n[2]), m[2]));
}
SSE static inline void store3(float *out, __m128 v) {
    float lanes[4];
    _mm_storeu_ps(lanes, v);
    out[0] = lanes[0];
    out[1] = lanes[1];
    out[2] = lanes[2];
}

// ---- Skin::PositionsNormals: as Skin::Positions for vertices {position, normal, (TANGENT: a tangent and a word,)
// bones at +BONES, weights at +WEIGHTS, SIZE bytes in all, then layout[2] bytes copied as they are}: the normal turned by the first bone only; out {position, normal, the
// copied bytes}; vertices passed over move by layout[0] in and layout[1] out --------------------------------------------------
template <uint32_t WEIGHTS, uint32_t SIZE, uint32_t BONES = 0x18, bool TANGENT = false, uint32_t OUT_SIZE = 0x18>
SSE static void __fastcall skin_positions_normals(const float *bones, const uint16_t *layout, const uint8_t *vertex, uint8_t *out) {
    g_skins++;
    uint32_t size_in = layout[0], size_out = layout[1], extra = layout[2];
    const uint16_t *run = layout + 3;
    for (;;) {
        uint32_t count = *run >> 2;
        if (!count) {
            uint32_t skip = run[1];
            if (!skip) return;
            vertex += skip * size_in;
            out += skip * size_out;
            run += 2;
            continue;
        }
        uint32_t more = *run & 3;
        for (; count; count--) {
            const float *at = (const float *)vertex;
            __m128 v = bone_point(bones, vertex[BONES], at), n = bone_normal(bones, vertex[BONES], at + 3), t = TANGENT ? bone_normal(bones, vertex[BONES], at + 6) : n;
            if (more) {
                v = _mm_mul_ps(v, _mm_set1_ps(bone_weight(vertex[WEIGHTS])));
                for (uint32_t k = more; k; k--)
                    v = _mm_add_ps(v, _mm_mul_ps(bone_point_more(bones, vertex[BONES + k], at), _mm_set1_ps(bone_weight(vertex[WEIGHTS + k]))));
            }
            store3((float *)out, v);
            store3((float *)out + 3, n);
            if (TANGENT) {   // with a tangent, and the word after it copied
                store3((float *)out + 6, t);
                *(uint32_t *)(out + 0x24) = *(const uint32_t *)(vertex + 0x24);
            }
            vertex += SIZE;
            out += OUT_SIZE;
            for (uint32_t left = extra; left; left -= 4) {
                *(uint32_t *)out = *(const uint32_t *)vertex;
                vertex += 4;
                out += 4;
            }
        }
        run++;
    }
}
static const uint8_t skin_positions_original[] = {0x55, 0x8b, 0xec, 0x83, 0xec, 0x14, 0x53, 0x56};
REWRITE("Skin::Positions", 0xbef020, skin_positions_original, &skin_positions, &g_skins);
REWRITE("Skin::Positions", 0xbef450, skin_positions_original, &skin_positions, &g_skins);
static const uint8_t skin_positions_normals_original[] = {0x55, 0x8b, 0xec, 0x83, 0xec, 0x18, 0x53, 0x56};
REWRITE("Skin::PositionsNormals", 0xbef8f0, skin_positions_normals_original, (&skin_positions_normals<0x1a, 0x1c>), &g_skins);
REWRITE("Skin::PositionsNormals", 0xbefe00, skin_positions_normals_original, (&skin_positions_normals<0x1c, 0x20>), &g_skins);

// ---- Skin::Full(bones, uv scale; alpha, uv offset, layout, vertices, out): as Skin::PositionsNormals with a tangent
// (TANGENT: turned by the first bone like the normal), a word copied over (WORD_FROM to WORD_TO, after the tangent; -1:
// none), a colour (COLOUR_IN: rgb kept, alpha its alpha byte + 1 times the alpha given, rounded by the FPU to a whole
// number and kept to 0-255; -1: none) and texture coordinates (UV_IN, two floats, times the scale plus the offset, on the
// FPU). Vertices and out move by the layout's two sizes; bones at +BONES, weights at +WEIGHTS ---------------------------------
static int32_t fpu_round_low(long double value) {   // fistp to 64 bits, as the game rounds (the FPU's rounding mode)
    int64_t whole;
    __asm__("fistpll %0" : "=m"(whole) : "t"(value) : "st");
    return (int32_t)whole;
}
template <bool TANGENT, uint32_t BONES, uint32_t WEIGHTS, int WORD_FROM, uint32_t WORD_TO, int COLOUR_IN, uint32_t COLOUR_OUT, uint32_t UV_IN, uint32_t UV_OUT>
SSE static void __fastcall skin_full(const float *bones, const float *uv_scale, float alpha, const float *uv_offset, const uint16_t *layout, const uint8_t *vertex,
                                     uint8_t *out) {
    g_skins++;
    int32_t alpha_whole = fpu_round_low((long double)alpha * 255.0f);
    if (alpha_whole < 0) alpha_whole = 0;
    if (alpha_whole > 0xff) alpha_whole = 0xff;
    uint32_t size_in = layout[0], size_out = layout[1];
    const uint16_t *run = layout + 3;
    for (;;) {
        uint32_t count = *run >> 2;
        if (!count) {
            uint32_t skip = run[1];
            if (!skip) return;
            vertex += skip * size_in;
            out += skip * size_out;
            run += 2;
            continue;
        }
        uint32_t more = *run & 3;
        for (; count; count--) {
            const float *at = (const float *)vertex;
            uint8_t bone = vertex[BONES];
            __m128 v = bone_point(bones, bone, at), n = bone_normal(bones, bone, at + 3), t = TANGENT ? bone_normal(bones, bone, at + 6) : n;
            if (more) {
                v = _mm_mul_ps(v, _mm_set1_ps(bone_weight(vertex[WEIGHTS])));
                for (uint32_t k = more; k; k--)
                    v = _mm_add_ps(v, _mm_mul_ps(bone_point_more(bones, vertex[BONES + k], at), _mm_set1_ps(bone_weight(vertex[WEIGHTS + k]))));
            }
            store3((float *)out, v);
            store3((float *)out + 3, n);
            if (TANGENT) store3((float *)out + 6, t);
            if (WORD_FROM >= 0) *(uint32_t *)(out + WORD_TO) = *(const uint32_t *)(vertex + WORD_FROM);
            if (COLOUR_IN >= 0) {
                uint32_t scaled = ((uint32_t)vertex[COLOUR_IN + 3] + 1) * (uint32_t)alpha_whole;
                *(uint32_t *)(out + COLOUR_OUT) = (scaled << 16 & 0xff000000) | (*(const uint32_t *)(vertex + COLOUR_IN) & 0xffffff);
            }
            const float *uv = (const float *)(vertex + UV_IN);
            ((float *)(out + UV_OUT))[0] = (float)((long double)uv[0] * uv_scale[0] + uv_offset[0]);
            ((float *)(out + UV_OUT))[1] = (float)((long double)uv[1] * uv_scale[1] + uv_offset[1]);
            vertex += size_in;
            out += size_out;
        }
        run++;
    }
}
static const uint8_t skin_full_bf0e70_original[] = {0x55, 0x8b, 0xec, 0x83, 0xec, 0x18, 0xd9, 0x45};
REWRITE("Skin::Full", 0xbf0e70, skin_full_bf0e70_original, (&skin_full<false, 0x18, 0x1a, -1, 0, -1, 0, 0x1c, 0x18>), &g_skins);
static const uint8_t skin_full_bf1410_original[] = {0x55, 0x8b, 0xec, 0x83, 0xec, 0x18, 0xd9, 0x45};
REWRITE("Skin::Full", 0xbf1410, skin_full_bf1410_original, (&skin_full<false, 0x18, 0x1c, -1, 0, -1, 0, 0x20, 0x18>), &g_skins);
static const uint8_t skin_full_bf1a10_original[] = {0x55, 0x8b, 0xec, 0x83, 0xec, 0x18, 0xd9, 0x45};
REWRITE("Skin::Full", 0xbf1a10, skin_full_bf1a10_original, (&skin_full<true, 0x28, 0x2a, 36, 0x20, -1, 0, 0x2c, 0x24>), &g_skins);
static const uint8_t skin_full_bf2060_original[] = {0x55, 0x8b, 0xec, 0x83, 0xec, 0x18, 0xd9, 0x45};
REWRITE("Skin::Full", 0xbf2060, skin_full_bf2060_original, (&skin_full<true, 0x28, 0x2c, 36, 0x20, -1, 0, 0x30, 0x24>), &g_skins);
static const uint8_t skin_full_bf2670_original[] = {0x55, 0x8b, 0xec, 0x83, 0xec, 0x1c, 0xd9, 0x45};
REWRITE("Skin::Full", 0xbf2670, skin_full_bf2670_original, (&skin_full<false, 0x18, 0x1a, -1, 0, 28, 0x18, 0x20, 0x1c>), &g_skins);
static const uint8_t skin_full_bf2c60_original[] = {0x55, 0x8b, 0xec, 0x83, 0xec, 0x1c, 0xd9, 0x45};
REWRITE("Skin::Full", 0xbf2c60, skin_full_bf2c60_original, (&skin_full<false, 0x18, 0x1c, -1, 0, 32, 0x18, 0x24, 0x1c>), &g_skins);
static const uint8_t skin_full_bf32b0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xec, 0x1c, 0xd9, 0x45};
REWRITE("Skin::Full", 0xbf32b0, skin_full_bf32b0_original, (&skin_full<true, 0x28, 0x2a, 36, 0x20, 44, 0x24, 0x30, 0x28>), &g_skins);
static const uint8_t skin_full_bf3950_original[] = {0x55, 0x8b, 0xec, 0x83, 0xec, 0x1c, 0xd9, 0x45};
REWRITE("Skin::Full", 0xbf3950, skin_full_bf3950_original, (&skin_full<true, 0x28, 0x2c, 36, 0x20, 48, 0x24, 0x34, 0x28>), &g_skins);
REWRITE("Skin::PositionsNormalsTangents", 0xbf0340, skin_positions_normals_original, (&skin_positions_normals<0x2a, 0x2c, 0x28, true, 0x28>), &g_skins);
REWRITE("Skin::PositionsNormalsTangents", 0xbf08e0, skin_positions_normals_original, (&skin_positions_normals<0x2c, 0x30, 0x28, true, 0x28>), &g_skins);
