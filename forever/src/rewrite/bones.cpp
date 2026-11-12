// Bones: a model's pose matrix from one of its bones' 64-byte matrices. Each is a 4x4 product whose 16 sums the game
// adds up in its own order (written out per copy below, as the order of k in out[i][j] = sum over k of L[i][k] R[k][j]);
// the sums are worked out in the FPU's precision and rounded to floats, all of them before the result is written.
#include "wave.h"

typedef long double x87;
typedef uint8_t SumOrder[16][4];

static void multiply_ordered(float *out, const float *l, const float *r, const SumOrder &order) {
    float result[16];
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            const uint8_t *o = order[i * 4 + j];
            x87 sum = (x87)l[i * 4 + o[0]] * r[o[0] * 4 + j];
            sum = sum + (x87)l[i * 4 + o[1]] * r[o[1] * 4 + j];
            sum = sum + (x87)l[i * 4 + o[2]] * r[o[2] * 4 + j];
            sum = sum + (x87)l[i * 4 + o[3]] * r[o[3] * 4 + j];
            result[i * 4 + j] = (float)sum;
        }
    volatile float *v = out;
    for (int i = 0; i < 16; i++) v[i] = result[i];
}

static volatile long g_bone_poses;

// ---- Model::BonePose(out, model): the object's matrix (at MATRIX) times the model's bone matrix (list at model +0x18)
// numbered by what +0x84 points to; returns out ----
template <uint32_t MATRIX, const SumOrder &ORDER>
static float *__thiscall bone_pose(uint8_t *self, float *out, uint8_t *model) {
    g_bone_poses++;
    uint32_t bone = **(uint32_t **)(self + 0x84);
    const float *m = (const float *)(*(uint8_t **)(model + 0x18) + bone * 0x40);
    multiply_ordered(out, (const float *)(self + MATRIX), m, ORDER);
    return out;
}
static const SumOrder bone_pose_7bf0c0 = {{3, 2, 1, 0}, {2, 0, 1, 3}, {3, 1, 2, 0}, {2, 0, 3, 1}, {1, 3, 0, 2}, {0, 3, 2, 1}, {1, 0, 3, 2}, {1, 0, 3, 2},
                                          {1, 3, 0, 2}, {0, 3, 2, 1}, {1, 3, 2, 0}, {1, 0, 3, 2}, {1, 3, 0, 2}, {0, 3, 2, 1}, {1, 2, 3, 0}, {0, 1, 3, 2}};
static const SumOrder bone_pose_7bee20 = {{3, 2, 1, 0}, {2, 0, 1, 3}, {1, 3, 2, 0}, {1, 3, 2, 0}, {1, 0, 3, 2}, {3, 2, 1, 0}, {3, 2, 0, 1}, {0, 3, 2, 1},
                                          {1, 0, 3, 2}, {3, 2, 1, 0}, {3, 2, 0, 1}, {0, 3, 2, 1}, {1, 0, 3, 2}, {3, 2, 1, 0}, {3, 2, 0, 1}, {0, 3, 2, 1}};
static const uint8_t bone_pose_7bf0c0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x4c, 0x8b, 0x81, 0x84, 0x00, 0x00, 0x00};
REWRITE("Model::BonePose", 0x7bf0c0, bone_pose_7bf0c0_original, (&bone_pose<0x40, bone_pose_7bf0c0>), &g_bone_poses);
static const uint8_t bone_pose_7bee20_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x4c, 0x8b, 0x81, 0x84, 0x00, 0x00, 0x00};
REWRITE("Model::BonePose", 0x7bee20, bone_pose_7bee20_original, (&bone_pose<0x0, bone_pose_7bee20>), &g_bone_poses);

// ---- Model::BonePoseOf(out, bone): bone matrix number bone (list at +0xB0's +0x20) times the object's matrix at +0x10;
// returns out ----
static const SumOrder bone_pose_b82090 = {{0, 2, 1, 3}, {0, 3, 1, 2}, {2, 0, 3, 1}, {0, 1, 3, 2}, {2, 3, 0, 1}, {1, 3, 2, 0}, {1, 3, 2, 0}, {3, 2, 1, 0},
                                          {2, 3, 0, 1}, {1, 3, 2, 0}, {1, 3, 2, 0}, {3, 2, 1, 0}, {2, 3, 0, 1}, {2, 1, 3, 0}, {1, 3, 0, 2}, {3, 2, 1, 0}};
static float *__thiscall bone_pose_of(uint8_t *self, float *out, uint32_t bone) {
    g_bone_poses++;
    const float *m = (const float *)(*(uint8_t **)(*(uint8_t **)(self + 0xb0) + 0x20) + bone * 0x40);
    multiply_ordered(out, m, (const float *)(self + 0x10), bone_pose_b82090);
    return out;
}
static const uint8_t bone_pose_of_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x4c, 0x8b, 0x91, 0xb0, 0x00, 0x00, 0x00};
REWRITE("Model::BonePoseOf", 0xb82090, bone_pose_of_original, &bone_pose_of, &g_bone_poses);

// ---- Matrix::Multiply(out, b): out = this matrix times b (every value read before out is written); returns out ----
static volatile long g_matrix_multiplies;
static const SumOrder multiply_42ee50 = {{1, 0, 3, 2}, {1, 3, 2, 0}, {0, 3, 2, 1}, {3, 2, 0, 1}, {0, 1, 3, 2}, {1, 3, 2, 0}, {0, 3, 2, 1}, {3, 0, 2, 1},
                                         {0, 1, 3, 2}, {1, 3, 2, 0}, {0, 3, 2, 1}, {3, 2, 0, 1}, {3, 1, 2, 0}, {0, 2, 1, 3}, {1, 2, 3, 0}, {2, 1, 0, 3}};
static float *__thiscall matrix_multiply(const float *self, float *out, const float *b) {
    g_matrix_multiplies++;
    multiply_ordered(out, self, b, multiply_42ee50);
    return out;
}
static const uint8_t matrix_multiply_original[] = {0x83, 0xec, 0x20, 0x8b, 0x44, 0x24, 0x28, 0xd9, 0x40, 0x2c};
REWRITE("Matrix::Multiply", 0x42ee50, matrix_multiply_original, &matrix_multiply, &g_matrix_multiplies);

// ---- Matrix::MultiplyBy(b): this matrix = itself times b; returns this ----
static const SumOrder multiply_by_505c00 = {{2, 1, 0, 3}, {3, 1, 2, 0}, {0, 3, 1, 2}, {2, 0, 3, 1}, {2, 3, 0, 1}, {0, 1, 3, 2}, {0, 3, 2, 1}, {3, 2, 1, 0},
                                            {2, 3, 0, 1}, {1, 0, 3, 2}, {3, 2, 0, 1}, {3, 2, 1, 0}, {2, 3, 0, 1}, {1, 0, 3, 2}, {2, 3, 0, 1}, {3, 2, 1, 0}};
static float *__thiscall matrix_multiply_by(float *self, const float *b) {
    g_matrix_multiplies++;
    multiply_ordered(self, self, b, multiply_by_505c00);
    return self;
}
static const uint8_t matrix_multiply_by_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x40, 0x8b};
REWRITE("Matrix::MultiplyBy", 0x505c00, matrix_multiply_by_original, &matrix_multiply_by, &g_matrix_multiplies);

// ---- Matrix::Invert4(): this 4x4 matrix inverted in place, from its cofactors. Each cofactor (out element by out
// element: the adjugate) is ((a*b - c*d) * e - (f*g - h*i) * j) + (k*l - m*n) * o of the matrix's elements numbered
// below, negated where marked, in the FPU's precision and rounded to a float; the determinant is the first row times
// the cofactors of out elements 0, 4, 8, 12 added in that order; each element is its cofactor times (the constant at
// 0xF264DC, 1) over the determinant. Returns this. ----
struct Cofactor {
    uint8_t negate, e[15];
};
static const Cofactor invert4_cofactors[16] = {
    {0, {15, 10, 14, 11, 5, 15, 6, 14, 7, 9, 11, 6, 10, 7, 13}}, {1, {15, 10, 14, 11, 1, 15, 2, 14, 3, 9, 11, 2, 10, 3, 13}},
    {0, {15, 6, 14, 7, 1, 15, 2, 14, 3, 5, 7, 2, 6, 3, 13}},     {1, {11, 6, 10, 7, 1, 11, 2, 10, 3, 5, 7, 2, 6, 3, 9}},
    {1, {15, 10, 14, 11, 4, 15, 6, 14, 7, 8, 11, 6, 10, 7, 12}}, {0, {15, 10, 14, 11, 0, 15, 2, 14, 3, 8, 11, 2, 10, 3, 12}},
    {1, {15, 6, 14, 7, 0, 15, 2, 14, 3, 4, 7, 2, 6, 3, 12}},     {0, {11, 6, 10, 7, 0, 11, 2, 10, 3, 4, 7, 2, 6, 3, 8}},
    {0, {15, 9, 13, 11, 4, 15, 5, 13, 7, 8, 11, 5, 9, 7, 12}},   {1, {15, 9, 13, 11, 0, 15, 1, 13, 3, 8, 11, 1, 9, 3, 12}},
    {0, {15, 5, 13, 7, 0, 15, 1, 13, 3, 4, 7, 1, 5, 3, 12}},     {1, {11, 5, 9, 7, 0, 11, 1, 9, 3, 4, 7, 1, 5, 3, 8}},
    {1, {14, 9, 13, 10, 4, 14, 5, 13, 6, 8, 10, 5, 9, 6, 12}},   {0, {14, 9, 13, 10, 0, 14, 1, 13, 2, 8, 10, 1, 9, 2, 12}},
    {1, {14, 5, 13, 6, 0, 14, 1, 13, 2, 4, 6, 1, 5, 2, 12}},     {0, {5, 10, 6, 9, 0, 1, 10, 2, 9, 4, 1, 6, 2, 5, 8}}};
static volatile long g_matrix_inverts;
static float *__thiscall matrix_invert4(float *m) {
    g_matrix_inverts++;
    volatile float c[16];   // the cofactors, rounded as the game keeps them
    for (int k = 0; k < 16; k++) {
        const uint8_t *i = invert4_cofactors[k].e;
        x87 v = (((x87)m[i[0]] * m[i[1]] - (x87)m[i[2]] * m[i[3]]) * m[i[4]] - ((x87)m[i[5]] * m[i[6]] - (x87)m[i[7]] * m[i[8]]) * m[i[9]]) +
                ((x87)m[i[10]] * m[i[11]] - (x87)m[i[12]] * m[i[13]]) * m[i[14]];
        if (invert4_cofactors[k].negate) v = -v;
        c[k] = (float)v;
    }
    const x87 det = (((x87)c[0] * m[0] + (x87)c[4] * m[1]) + (x87)c[8] * m[2]) + (x87)c[12] * m[3];
    const x87 inverse = (x87) * (const float *)0xf264dc / det;
    for (int k = 0; k < 16; k++) ((volatile float *)m)[k] = (float)((x87)c[k] * inverse);
    return m;
}
static const uint8_t matrix_invert4_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x81, 0xec, 0xb0, 0x01, 0x00, 0x00};
REWRITE("Matrix::Invert4", 0xc1fc30, matrix_invert4_original, &matrix_invert4, &g_matrix_inverts);
