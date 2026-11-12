// Effects: setting one of an effect's matrices by number. Three effect classes have the same setter with their own
// layout: the effect's own matrix (S, 16 floats), the result block (B: the combined matrix, then two more matrices at
// +0x40 and +0x80) and the flags saying which ones changed (F).
#include "wave.h"

typedef long double x87;   // the FPU's own precision: what the game's code keeps on the FPU stack between steps

static inline void copy16(uint8_t *to, const uint32_t *from) {   // an exact copy (never through the FPU)
    const volatile uint32_t *v = from;
    for (int i = 0; i < 16; i++) ((uint32_t *)to)[i] = v[i];
}

// ---- Effect::SetMatrix(which, matrix) ---------------------------------------------------------------------------------
// 0: the effect's own matrix, also the combined one; 1, 2: the two others (2 also sets the byte at +0x1A1); 3: the
// world matrix (kept at +0x10): combined = its rotation part (rows 0-2, columns 0-2) times the effect's matrix, plus
// the effect's last row times the constant at 0xF264E0 (0 in the game); the combined last row is the effect's column
// sums times that constant plus its last row. Each value is worked out in the FPU's precision in the game's order,
// rounded to a float where the game rounds it. Returns what the game leaves in eax.
static volatile long g_effect_set_matrix;
template <uint32_t S, uint32_t B, uint32_t F>
static uint32_t __thiscall effect_set_matrix(uint8_t *self, uint32_t which, const uint32_t *m) {
    g_effect_set_matrix++;
    uint32_t &flags = *(uint32_t *)(self + F);
    switch (which) {
    case 0:
        copy16(self + S, m);
        copy16(self + B, m);
        flags |= 1;
        return ((const volatile uint32_t *)m)[15];
    case 1: {
        copy16(self + B + 0x40, m);
        uint32_t f = flags | 2;
        self[0x1a0] = 0;
        flags = f;
        return f;
    }
    case 2: {
        copy16(self + B + 0x80, m);
        uint32_t f = flags | 4;
        self[0x1a0] = 0;
        self[0x1a1] = 1;
        flags = f;
        return f;
    }
    case 3: {
        copy16(self + 0x10, m);
        const float *a = (const float *)m, *s = (const float *)(self + S);
        const x87 k = *(const float *)0xf264e0;
        volatile float t0 = (float)((x87)s[12] * k), t1 = (float)((x87)s[13] * k);   // rounded, as the game keeps them
        const x87 t2 = (x87)s[14] * k, t3 = (x87)s[15] * k;                         // kept on the FPU stack
        float r[16];
        for (int row = 0; row < 3; row++) {
            const x87 x = a[4 * row], y = a[4 * row + 1], z = a[4 * row + 2];
            r[4 * row] = (float)((z * s[8] + x * s[0]) + y * s[4] + t0);
            r[4 * row + 1] = (float)((y * s[5] + x * s[1]) + z * s[9] + t1);
            r[4 * row + 2] = (float)((y * s[6] + x * s[2]) + z * s[10] + t2);
            r[4 * row + 3] = (float)((z * s[11] + y * s[7]) + x * s[3] + t3);
        }
        r[12] = (float)((((x87)s[4] + s[0]) + s[8]) * k + s[12]);
        r[13] = (float)((((x87)s[5] + s[1]) + s[9]) * k + s[13]);
        r[14] = (float)((((x87)s[10] + s[6]) + s[2]) * k + s[14]);
        r[15] = (float)((((x87)s[3] + s[11]) + s[7]) * k + s[15]);
        volatile float *out = (volatile float *)(self + B);
        for (int i = 0; i < 16; i++) out[i] = r[i];
        flags |= 1;
        return ((volatile uint32_t *)(self + B))[15];
    }
    default:
        return which;
    }
}
static const uint8_t effect_set_matrix_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x81, 0xec, 0x90, 0x00, 0x00, 0x00};
REWRITE("Effect::SetMatrix", 0xc957e0, effect_set_matrix_original, (&effect_set_matrix<0x670, 0x750, 0x1ca4>), &g_effect_set_matrix);
REWRITE("Effect::SetMatrix", 0xc76870, effect_set_matrix_original, (&effect_set_matrix<0x2090, 0x6e0, 0x674>), &g_effect_set_matrix);
REWRITE("Effect::SetMatrix", 0xc71070, effect_set_matrix_original, (&effect_set_matrix<0x9f0, 0x6d0, 0x674>), &g_effect_set_matrix);
