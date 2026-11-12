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

// ---- Matrix::TransformRows(offset, m): the three points at this +0, +0x10, +0x20 turned by m's rotation (rows 0-2 of a
// 4x4) and moved by offset, in place. Each new z and y: the three products rounded, added in turn (rounded), then the
// offset (rounded); each new x: the first product rounded, the others added in the FPU's precision. Returns the last
// point's new z (as the game leaves it in eax). ----
static volatile long g_transform_rows;
static uint32_t __thiscall matrix_transform_rows(float *self, const float *offset, const float *m) {
    g_transform_rows++;
    for (int row = 0; row < 3; row++) {
        float *p = self + 4 * row;
        const float x = p[0], y = p[1], z = p[2];
        volatile float z0 = (float)((x87)x * m[2]), z1 = (float)((x87)y * m[6]), z2 = (float)((x87)z * m[10]);
        volatile float zs = (float)((x87)z0 + z1);
        volatile float zt = (float)((x87)zs + z2);
        volatile float x0 = (float)((x87)x * m[0]);
        volatile float y0 = (float)((x87)x * m[1]), y1 = (float)((x87)y * m[5]), y2 = (float)((x87)z * m[9]);
        volatile float ys = (float)((x87)y0 + y1);
        volatile float yt = (float)((x87)ys + y2);
        ((volatile float *)p)[2] = (float)((x87)zt + offset[2]);
        ((volatile float *)p)[0] = (float)((((x87)x0 + (x87)y * m[4]) + (x87)z * m[8]) + offset[0]);
        ((volatile float *)p)[1] = (float)((x87)yt + offset[1]);
    }
    return *(volatile uint32_t *)(self + 10);
}
static const uint8_t matrix_transform_rows_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x4c, 0x8b, 0x45, 0x0c};
REWRITE("Matrix::TransformRows", 0xc21770, matrix_transform_rows_original, &matrix_transform_rows, &g_transform_rows);

// A 3-term dot product as the game adds it: (first + second) + third, each a product of two floats, in the FPU's
// precision; rounded where it's stored.
static inline float dot3(float a0, float b0, float a1, float b1, float a2, float b2) {
    return (float)(((x87)a0 * b0 + (x87)a1 * b1) + (x87)a2 * b2);
}

// ---- Matrix::RotateRows(m): the three points at this +0, +0x10, +0x20 turned by m (3x3 of a 4x4) in place, each
// value's three products added in the game's order (the first point's in its own); returns this ----
static volatile long g_rotate_rows;
static float *__thiscall matrix_rotate_rows(float *self, const float *m) {
    g_rotate_rows++;
    for (int row = 0; row < 3; row++) {
        float *p = self + 4 * row;
        const float x = p[0], y = p[1], z = p[2];
        volatile float *out = p;
        if (row == 0) {
            out[0] = dot3(m[4], y, m[0], x, m[8], z);
            out[2] = dot3(m[6], y, m[10], z, m[2], x);
            out[1] = dot3(m[9], z, m[1], x, y, m[5]);
        } else {
            out[0] = dot3(z, m[8], m[0], x, y, m[4]);
            out[1] = dot3(y, m[5], m[1], x, m[9], z);
            out[2] = dot3(m[2], x, z, m[10], y, m[6]);
        }
    }
    return self;
}
static const uint8_t matrix_rotate_rows_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x30, 0x8b, 0xc1, 0x8b};
REWRITE("Matrix::RotateRows", 0xcc5900, matrix_rotate_rows_original, &matrix_rotate_rows, &g_rotate_rows);

// ---- Matrix::RotateOut(out, m): out (3x3, rows 0x10 apart) = m's rows against this matrix's rows: out[i][j] =
// m[i] . this[j], each in the game's order; every value read before out is written; returns out ----
static float *__thiscall matrix_rotate_out(const float *t, float *out, const float *m) {
    g_rotate_rows++;
    float r[9];
    r[0] = dot3(t[1], m[1], m[2], t[2], m[0], t[0]);
    r[1] = dot3(t[6], m[2], t[5], m[1], m[0], t[4]);
    r[2] = dot3(t[10], m[2], t[9], m[1], m[0], t[8]);
    r[3] = dot3(m[6], t[2], m[4], t[0], m[5], t[1]);
    r[4] = dot3(t[6], m[6], t[5], m[5], t[4], m[4]);
    r[5] = dot3(t[10], m[6], t[9], m[5], t[8], m[4]);
    r[6] = dot3(m[10], t[2], t[1], m[9], t[0], m[8]);
    r[7] = dot3(t[6], m[10], t[5], m[9], t[4], m[8]);
    r[8] = dot3(t[10], m[10], t[9], m[9], t[8], m[8]);
    volatile float *o = out;
    o[0] = r[0], o[1] = r[1], o[2] = r[2], o[4] = r[3], o[5] = r[4], o[6] = r[5], o[10] = r[8], o[8] = r[6], o[9] = r[7];
    return out;
}
static const uint8_t matrix_rotate_out_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x20, 0x8b, 0x45, 0x0c};
REWRITE("Matrix::RotateOut", 0xc21b40, matrix_rotate_out_original, &matrix_rotate_out, &g_rotate_rows);

// Rounded on the way, as the game keeps them in floats between steps.
static inline float rounded(x87 v) {
    volatile float f = (float)v;
    return f;
}
// The rotation of a point by m (3x3 of a 4x4, rows 0x10 apart) as the game's vector rotations work it out: x from
// y's product and x's rounded one, then z's; y and z from their three products rounded and added in turn (z's last
// product kept in the FPU's precision when LAST_KEPT).
template <bool LAST_KEPT>
static inline void rotate_point(volatile float *out, const float *v, const float *m, int order) {
    const float x = v[0], y = v[1], z = v[2];
    const float rx = (float)(((x87)y * m[4] + (x87)rounded((x87)x * m[0])) + (LAST_KEPT ? (x87)rounded((x87)z * m[8]) : (x87)z * m[8]));
    const float ry = (float)((x87)rounded((x87)rounded((x87)x * m[1]) + rounded((x87)y * m[5])) + rounded((x87)z * m[9]));
    const float rz = (float)((x87)rounded((x87)rounded((x87)x * m[2]) + rounded((x87)y * m[6])) + (LAST_KEPT ? (x87)z * m[10] : (x87)rounded((x87)z * m[10])));
    if (order) {
        out[2] = rz, out[0] = rx, out[1] = ry;
    } else {
        out[0] = rx, out[1] = ry, out[2] = rz;
    }
}
static volatile long g_vector_rotations;

// ---- Vector::Rotate(out, v, m) (fastcall): out = v turned by m; returns out ----
static float *__fastcall vector_rotate(float *out, const float *v, const float *m) {
    g_vector_rotations++;
    rotate_point<false>(out, v, m, 0);
    return out;
}
static const uint8_t vector_rotate_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x4c, 0xd9, 0x42, 0x08};
REWRITE("Vector::Rotate", 0x4812c0, vector_rotate_original, &vector_rotate, &g_vector_rotations);

// ---- Matrix::RotateVector(out, v): out = v turned by this matrix; returns out (two copies) ----
static float *__thiscall matrix_rotate_vector(const float *m, float *out, const float *v) {
    g_vector_rotations++;
    rotate_point<false>(out, v, m, 0);
    return out;
}
static const uint8_t matrix_rotate_vector_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x40, 0x8b, 0x45, 0x0c};
REWRITE("Matrix::RotateVector", 0x481220, matrix_rotate_vector_original, &matrix_rotate_vector, &g_vector_rotations);
REWRITE("Matrix::RotateVector", 0xca35d0, matrix_rotate_vector_original, &matrix_rotate_vector, &g_vector_rotations);

// ---- Vector::RotateInPlace(v, m) (fastcall): v turned by m in place (z's last product kept, x's last rounded);
// returns v ----
static float *__fastcall vector_rotate_in_place(float *v, const float *m) {
    g_vector_rotations++;
    rotate_point<true>(v, v, m, 1);
    return v;
}
static const uint8_t vector_rotate_in_place_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x40, 0x8b, 0xc1, 0xd9};
REWRITE("Vector::RotateInPlace", 0x58a620, vector_rotate_in_place_original, &vector_rotate_in_place, &g_vector_rotations);

// ---- Vector4::Transform(v, m) (fastcall): the four values of v times m in place, each sum in the game's order;
// returns v ----
static float *__fastcall vector4_transform(float *v, const float *m) {
    g_vector_rotations++;
    const float x = v[0], y = v[1], z = v[2], w = v[3];
    const float rz = (float)((((x87)m[10] * z + (x87)m[6] * y) + (x87)m[2] * x) + (x87)m[14] * w);
    const float rw = (float)((((x87)m[11] * z + (x87)m[7] * y) + (x87)m[3] * x) + (x87)m[15] * w);
    const float rx = (float)((((x87)m[8] * z + (x87)m[4] * y) + (x87)m[12] * w) + (x87)x * m[0]);
    const float ry = (float)((((x87)m[9] * z + (x87)m[5] * y) + (x87)m[1] * x) + (x87)m[13] * w);
    volatile float *o = v;
    o[2] = rz, o[3] = rw, o[0] = rx, o[1] = ry;
    return v;
}
static const uint8_t vector4_transform_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x10, 0xd9, 0x42, 0x20};
REWRITE("Vector4::Transform", 0x505e90, vector4_transform_original, &vector4_transform, &g_vector_rotations);

// ---- Skeleton::PartMatrix(): a bone's matrices from its part's matrix P (at the part +0x54 points to, +0xC0):
// W (this +0xC0) = this +0x10's 4x4 times P, each value's three products in its order plus P's last row (times the
// constant at 0xF264E0, 0 in the game, for the first three rows; rounded first for two of them); then the matrix this
// +0x64 points to = this +0x80's 4x4 times W (rounded), each in its order. Returns the last value written. ----
// How one copy orders its sums: step 1's three products per value, which of the first three rows' last products are
// rounded first, and step 2's.
struct PartOrders {
    uint8_t first[16][3];
    uint8_t last_rounded[12];
    SumOrder second;
};
static float *skeleton_part(uint8_t *bone, const PartOrders &orders) {
    const float *a = (const float *)(bone + 0x10), *p = (const float *)(*(uint8_t **)(bone + 0x54) + 0xc0);
    const x87 zero = *(const float *)0xf264e0;
    float w[16];
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            const uint8_t *o = orders.first[4 * i + j];
            x87 sum = (x87)a[4 * i + o[0]] * p[4 * o[0] + j];
            sum = sum + (x87)a[4 * i + o[1]] * p[4 * o[1] + j];
            sum = sum + (x87)a[4 * i + o[2]] * p[4 * o[2] + j];
            if (i < 3) sum = sum + (orders.last_rounded[4 * i + j] ? (x87)rounded((x87)p[12 + j] * zero) : (x87)p[12 + j] * zero);
            else sum = sum + p[12 + j];
            w[4 * i + j] = rounded(sum);
        }
    for (int i = 0; i < 16; i++) ((volatile float *)(bone + 0xc0))[i] = w[i];
    float *out = *(float **)(bone + 0x64);
    multiply_ordered(out, (const float *)(bone + 0x80), w, orders.second);
    return out;
}
static const PartOrders skeleton_part_orders = {
    {{1, 0, 2}, {2, 1, 0}, {1, 0, 2}, {1, 0, 2}, {1, 2, 0}, {1, 0, 2}, {2, 0, 1}, {1, 2, 0}, {1, 0, 2}, {1, 0, 2}, {2, 0, 1}, {1, 2, 0}, {1, 0, 2}, {1, 0, 2}, {2, 0, 1}, {1, 2, 0}},
    {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0},
    {{3, 0, 2, 1}, {2, 1, 0, 3}, {1, 0, 3, 2}, {1, 3, 0, 2}, {0, 1, 3, 2}, {3, 2, 1, 0}, {3, 2, 0, 1}, {0, 3, 2, 1},
     {0, 1, 3, 2}, {3, 2, 1, 0}, {3, 2, 0, 1}, {0, 3, 2, 1}, {0, 1, 3, 2}, {3, 2, 1, 0}, {3, 0, 2, 1}, {0, 3, 2, 1}}};
static volatile long g_skeleton_parts;
static uint32_t __thiscall skeleton_part_matrix(uint8_t *self) {
    g_skeleton_parts++;
    return ((volatile uint32_t *)skeleton_part(self, skeleton_part_orders))[15];
}
static const uint8_t skeleton_part_matrix_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x81, 0xec, 0x90, 0x00, 0x00, 0x00, 0x8b, 0x41, 0x10};
REWRITE("Skeleton::PartMatrix", 0xc22d30, skeleton_part_matrix_original, &skeleton_part_matrix, &g_skeleton_parts);

// ---- Skeleton::UpdateParts(): every bone on the skeleton's list (the object this +0x10 points to points to; the
// first at its +0x58, each next at the bone's +0x58) given its matrices as Skeleton::PartMatrix does, in this copy's
// orders; returns 0 (the list's end) ----
static const PartOrders skeleton_update_parts_orders = {
    {{1, 2, 0}, {2, 0, 1}, {2, 1, 0}, {1, 2, 0}, {2, 0, 1}, {0, 2, 1}, {1, 0, 2}, {2, 0, 1}, {2, 0, 1}, {0, 2, 1}, {1, 0, 2}, {2, 0, 1}, {2, 0, 1}, {0, 2, 1}, {1, 0, 2}, {2, 0, 1}},
    {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0},
    {{3, 2, 1, 0}, {0, 2, 1, 3}, {3, 1, 0, 2}, {2, 3, 1, 0}, {1, 3, 0, 2}, {0, 3, 2, 1}, {1, 0, 3, 2}, {1, 0, 3, 2},
     {1, 3, 0, 2}, {0, 3, 2, 1}, {1, 3, 2, 0}, {1, 0, 3, 2}, {1, 3, 0, 2}, {0, 3, 2, 1}, {1, 2, 3, 0}, {0, 1, 3, 2}}};
static uint32_t __thiscall skeleton_update_parts(uint8_t *self) {
    g_skeleton_parts++;
    for (uint8_t *bone = *(uint8_t **)(**(uint8_t ***)(self + 0x10) + 0x58); bone; bone = *(uint8_t **)(bone + 0x58))
        skeleton_part(bone, skeleton_update_parts_orders);
    return 0;
}
static const uint8_t skeleton_update_parts_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x81, 0xec, 0xd0, 0x00, 0x00, 0x00, 0x8b, 0x41, 0x10, 0x8b, 0x08, 0x8b};
REWRITE("Skeleton::UpdateParts", 0xc286d0, skeleton_update_parts_original, &skeleton_update_parts, &g_skeleton_parts);

// ---- Vector::TransformTwice(out, second, first): the point at this through the first 4x4 (with its translation),
// then the result through the second, into out; the first result's z rounded where x and y use it but kept in the
// FPU's precision for z; returns out ----
static float *__thiscall vector_transform_twice(const float *v, float *out, const float *m2, const float *m1) {
    g_vector_rotations++;
    const float x = v[0], y = v[1], z = v[2];
    const float tx = (float)((((x87)z * m1[8] + (x87)y * m1[4]) + (x87)x * m1[0]) + m1[12]);
    const float ty = (float)((((x87)x * m1[1] + (x87)z * m1[9]) + (x87)y * m1[5]) + m1[13]);
    const x87 tz = (((x87)x * m1[2] + (x87)z * m1[10]) + (x87)y * m1[6]) + m1[14];
    const float tzr = rounded(tz);
    volatile float *o = out;
    o[0] = (float)((((x87)tzr * m2[8] + (x87)ty * m2[4]) + (x87)tx * m2[0]) + m2[12]);
    o[1] = (float)((((x87)tzr * m2[9] + (x87)ty * m2[5]) + (x87)tx * m2[1]) + m2[13]);
    o[2] = (float)(((tz * m2[10] + (x87)ty * m2[6]) + (x87)tx * m2[2]) + m2[14]);
    return out;
}
static const uint8_t vector_transform_twice_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x10, 0x8b, 0x45, 0x10, 0xd9, 0x01, 0xd9};
REWRITE("Vector::TransformTwice", 0xcb5690, vector_transform_twice_original, &vector_transform_twice, &g_vector_rotations);
