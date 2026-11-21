// Vector maths written by hand, one game function at a time. As geometry.cpp: the game's x87 code keeps sums in a
// register (the precision the FPU is set to: long double here) and rounds to float only where it stores, so these do
// the same, in the same order (each sum's terms as the game adds them).
#include "wave.h"

// x87 instructions the compiler would otherwise call the C library for
static inline long double x87_sqrt(long double v) {
    __asm__("fsqrt" : "+t"(v));
    return v;
}
static inline long double x87_round(long double v) {   // to a whole number, by the rounding the FPU is set to
    __asm__("frndint" : "+t"(v));
    return v;
}

// ---- Vector::AngleBetween(before, a, b, c) (0xC5A5A0): three angles kept within a turn ---------------------------------
// Each of a, b and c (its length L, as a float, worked out first for all three) is measured against the one before it
// (before, then a as it is by then, then b): x = L - dot(before, v) / L, plus pi. When x is outside [0, 2 pi) (a NaN
// is left as it is), v is scaled by (L - whole turns * 2 pi) / L, the whole turns being x / (2 pi) (as a float) rounded
// down. (The game takes the turns off L, not off x: kept so.)
static volatile long g_angle_between;
static inline float length_xy_z(const float *v) {   // ((x x + y y) + z z)
    return (float)x87_sqrt(((long double)v[0] * v[0] + (long double)v[1] * v[1]) + (long double)v[2] * v[2]);
}
static inline float length_zx_y(const float *v) {   // ((z z + x x) + y y)
    return (float)x87_sqrt(((long double)v[2] * v[2] + (long double)v[0] * v[0]) + (long double)v[1] * v[1]);
}
static void angle_kept(float *v, float length, long double dot) {
    if (length == 0.0f)   // (an exact 0 only; a NaN goes on)
        return;
    const volatile float inverse = (float)(1.0L / length);   // (volatile: kept rounded, as stored; the compiler would not)
    const long double x = (length - dot * inverse) + (long double)*(const float *)0xf285f0;   // pi
    const float zero = *(const float *)0xf264e0, turn = *(const float *)0xf268dc;
    if (!(x < zero) && !(x >= turn))   // within [0, 2 pi), or a NaN: left
        return;
    const volatile float turns = (float)(x * (long double)*(const float *)0xf36ee4);   // x / (2 pi)
    const long double whole = x87_round(turns);
    const volatile float down = (float)(turns < whole ? whole - 1.0L : whole);
    const long double scale = (length - down * (long double)turn) * inverse;
    v[0] = (float)(scale * v[0]);
    v[1] = (float)(scale * v[1]);
    v[2] = (float)(scale * v[2]);
}
static void __fastcall vector_angle_between(const float *before, float *a, float *b, float *c) {
    g_angle_between++;
    const volatile float la = length_xy_z(a), lb = length_zx_y(b), lc = length_xy_z(c);
    angle_kept(a, la, ((long double)before[2] * a[2] + (long double)before[1] * a[1]) + (long double)before[0] * a[0]);
    angle_kept(b, lb, ((long double)a[0] * b[0] + (long double)b[2] * a[2]) + (long double)b[1] * a[1]);
    angle_kept(c, lc, ((long double)b[2] * c[2] + (long double)b[1] * c[1]) + (long double)b[0] * c[0]);
}
static const uint8_t vector_angle_between_original[] = {0x83, 0xec, 0x10, 0xd9, 0x42, 0x08, 0x56, 0xd9};
REWRITE("Vector::AngleBetween", 0xc5a5a0, vector_angle_between_original, &vector_angle_between, &g_angle_between);

// ---- Quad::Normal(corners, out0..out4) (0x727EA0): a tile quad's five normals ------------------------------------------
// From the four corner normals (+0, +0x10, +0x20, +0x30) five directions, each the sum of two of them, made one long:
// 0+2, 1+3, 0+1, 2+3 and 1+2. Each sum is stored as floats, its z kept unrounded for the length (the square root of
// z z + y y + x x, in that order, the x and y as stored); a length of exactly 0 leaves x and y and sets z to 1.
static volatile long g_quad_normals;
static void quad_normal_one(float *out, const float *p, const float *q) {
    volatile float *const o = out;   // (every value read back as stored: the compiler would keep it unrounded)
    o[0] = p[0], o[1] = p[1], o[2] = p[2];
    o[0] = (float)((long double)o[0] + q[0]);
    o[1] = (float)((long double)q[1] + o[1]);
    const long double z = (long double)q[2] + o[2];
    o[2] = (float)z;
    const long double y = o[1], x = o[0];
    const long double length = x87_sqrt((z * z + y * y) + x * x);
    if (length == 0.0L) {   // (exactly 0; a NaN is divided by)
        *(volatile uint32_t *)&o[2] = 0x3f800000;   // 1.0
        return;
    }
    const long double inverse = 1.0L / length;
    o[0] = (float)(inverse * o[0]);
    o[1] = (float)(inverse * o[1]);
    o[2] = (float)(inverse * o[2]);
}
static void __fastcall quad_normal(const float *corners, float *out0, float *out1, float *out2, float *out3, float *out4) {
    g_quad_normals++;
    quad_normal_one(out0, corners, corners + 8);
    quad_normal_one(out1, corners + 4, corners + 12);
    quad_normal_one(out2, corners, corners + 4);
    quad_normal_one(out3, corners + 8, corners + 12);
    quad_normal_one(out4, corners + 4, corners + 8);
}
static const uint8_t quad_normal_original[] = {0x8b, 0x01, 0x89, 0x02, 0xd9, 0x02, 0x8b, 0x41};
REWRITE("Quad::Normal", 0x727ea0, quad_normal_original, &quad_normal, &g_quad_normals);

static inline long double x87_abs(long double v) {
    __asm__("fabs" : "+t"(v));
    return v;
}

// ---- Matrix::Invert3(m) (0xBEE0C0): a 3x3 matrix (rows 16 bytes apart) inverted in place ------------------------------
// By its cofactors over the determinant (the first row's cofactors against the first row: t m02 + c01 m01 + c00 m00).
// As the game: c00 stays unrounded, c01 is rounded (stored, then read back), c02 is stored rounded while the
// determinant takes it unrounded; the reciprocal of the determinant is rounded once (stored) and each entry is rounded
// as it is written. All nine values are read before the first is written.
static volatile long g_matrix_inverts;
static void __fastcall matrix_invert3(float *m) {
    g_matrix_inverts++;
    const long double a00 = m[0], a01 = m[1], a02 = m[2], a10 = m[4], a11 = m[5], a12 = m[6], a20 = m[8], a21 = m[9], a22 = m[10];
    const long double c00 = a11 * a22 - a21 * a12;
    const volatile float c01 = (float)-(a22 * a10 - a20 * a12);
    const long double t = a21 * a10 - a20 * a11;
    const volatile float c02 = (float)t;
    const volatile float inverse = (float)(1.0L / ((t * a02 + (long double)c01 * a01) + c00 * a00));
    const long double inv = inverse;
    m[0] = (float)(c00 * inv);
    m[1] = (float)(-(a22 * a01 - a21 * a02) * inv);
    m[2] = (float)((a12 * a01 - a11 * a02) * inv);
    m[4] = (float)((long double)c01 * inv);
    m[5] = (float)((a22 * a00 - a20 * a02) * inv);
    m[6] = (float)(-(a12 * a00 - a10 * a02) * inv);
    m[8] = (float)((long double)c02 * inv);
    m[9] = (float)(-(a21 * a00 - a20 * a01) * inv);
    m[10] = (float)((a11 * a00 - a10 * a01) * inv);
}
static const uint8_t matrix_invert3_original[] = {0x83, 0xec, 0x1c, 0x8b, 0xc1, 0xd9, 0x00, 0x8b};
REWRITE("Matrix::Invert3", 0xbee0c0, matrix_invert3_original, &matrix_invert3, &g_matrix_inverts);

// ---- Matrix::Transform(out, v, m) (0x42F0B0): a 4-vector times a 4x4 matrix (rows of four floats) -------------------
// Each of the four results is ((m[3][j] v3 + m[2][j] v2) + m[1][j] v1) + m[0][j] v0, kept unrounded until it is stored.
// Everything is read before the first result is written (out may be v).
static volatile long g_matrix_transforms;
static void __fastcall matrix_transform(float *out, const float *v, const float *m) {
    g_matrix_transforms++;
    const long double v0 = v[0], v1 = v[1], v2 = v[2], v3 = v[3];
    long double r[4];
    for (int j = 0; j < 4; j++)
        r[j] = (((long double)m[12 + j] * v3 + (long double)m[8 + j] * v2) + (long double)m[4 + j] * v1) + (long double)m[j] * v0;
    for (int j = 0; j < 4; j++) out[j] = (float)r[j];
}
static const uint8_t matrix_transform_original[] = {0x83, 0xec, 0x44, 0x8b, 0xc1, 0xd9, 0x02, 0x8b};
REWRITE("Matrix::Transform", 0x42f0b0, matrix_transform_original, &matrix_transform, &g_matrix_transforms);

// ---- Vector::RotateAround(v, axis, angle) (0xBFB0B0) and RotateAroundSinCos(v, axis, sin, cos) (0xBFB1C0) -------------
// v turned about a unit axis u (Rodrigues): its part along u, p = (u.v) u, stays; the rest, w = v - p, becomes
// w cos + (w x u) sin. As the game: u.v is (u1 v1 + u0 v0) + v2 u2; p's x stays unrounded, its y and z are stored; w's z
// is stored too (the cross product takes it rounded in one place); every product with sin or cos is stored, and so are
// the y and z sums before the sine parts are added. The first works out sin and cos of the angle itself (fsin, fcos).
static volatile long g_vector_rotations;
static inline long double x87_sin(long double v) {
    __asm__("fsin" : "+t"(v));
    return v;
}
static inline long double x87_cos(long double v) {
    __asm__("fcos" : "+t"(v));
    return v;
}
static void rotate_about(float *v, const float *u, float sin_f, float cos_f) {
    const long double u0 = u[0], u1 = u[1], u2 = u[2], v0 = v[0], v1 = v[1], v2 = v[2], s = sin_f, c = cos_f;
    const long double d = (u1 * v1 + u0 * v0) + v2 * u2;
    const long double p0 = d * u0;
    const volatile float p1 = (float)(d * u1), p2 = (float)(d * u2);
    const long double w0 = v0 - p0, w1 = v1 - (long double)p1, w2 = v2 - (long double)p2;
    const volatile float w2_stored = (float)w2;
    const long double c0 = w2 * u1 - w1 * u2;
    const long double c1 = w0 * u2 - (long double)w2_stored * u0;
    const volatile float c2 = (float)(w1 * u0 - w0 * u1);
    const volatile float a0 = (float)(c0 * s), a1 = (float)(c1 * s), a2 = (float)((long double)c2 * s);
    const volatile float b0 = (float)(w0 * c), b1 = (float)(w1 * c), b2 = (float)((long double)w2_stored * c);
    const volatile float r1 = (float)((long double)b1 + p1), r2 = (float)((long double)b2 + p2);
    const long double out0 = (p0 + b0) + (long double)a0;
    const long double out1 = (long double)r1 + a1;
    const volatile float out2 = (float)((long double)r2 + a2);
    v[0] = (float)out0;
    v[1] = (float)out1;
    v[2] = out2;
}
static void __thiscall vector_rotate_around(float *v, const float *axis, float angle) {
    g_vector_rotations++;
    const volatile float s = (float)x87_sin(angle), c = (float)x87_cos(angle);
    rotate_about(v, axis, s, c);
}
static void __thiscall vector_rotate_around_sin_cos(float *v, const float *axis, float sin_f, float cos_f) {
    g_vector_rotations++;
    rotate_about(v, axis, sin_f, cos_f);
}
static const uint8_t vector_rotate_around_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Vector::RotateAround", 0xbfb0b0, vector_rotate_around_original, &vector_rotate_around, &g_vector_rotations);
REWRITE("Vector::RotateAroundSinCos", 0xbfb1c0, vector_rotate_around_original, &vector_rotate_around_sin_cos, &g_vector_rotations);

// ---- Bezier::Point(out, p0, p1, p2, p3, t) (0xBCBF90): the point at t on a cubic curve --------------------------------
// s^3 p0 + 3 s^2 t p1 + 3 s t^2 p2 + t^3 p3, s = 1 - t, with the game's weights ((s t) t 3, (t (s s)) 3, (s s) s, t t t)
// and its sums: x = (3st^2 p2x + (s^3 p0x + 3s^2t p1x)) + t^3 p3x, y likewise, z = ((s^3 p0z + 3s^2t p1z) + 3st^2 p2z)
// + t^3 p3z; the products and partial sums it stores are rounded there, the rest kept unrounded.
static volatile long g_bezier_points;
static void __fastcall bezier_point(float *out, const float *p0, const float *p1, const float *p2, const float *p3, float t_f) {
    g_bezier_points++;
    const long double t = t_f, three = *(const float *)0xf2784c;
    const long double s = (long double)*(const float *)0xf264dc - t;
    const long double t3 = t * t * t;
    const volatile float q0 = (float)(t3 * p3[0]), q1 = (float)(t3 * p3[1]);
    const long double q2 = t3 * p3[2];
    const long double k2 = s * t * t * three;
    const long double k2x = k2 * p2[0];
    const volatile float r1 = (float)(k2 * p2[1]), r2 = (float)(k2 * p2[2]);
    const long double ss = s * s;
    const long double k1 = t * ss * three;
    const volatile float s0 = (float)(k1 * p1[0]), s1 = (float)(k1 * p1[1]);
    const long double k1z = k1 * p1[2];
    const long double s3 = ss * s;
    const volatile float t0 = (float)(s3 * p0[0]), t1 = (float)(s3 * p0[1]), t2 = (float)(s3 * p0[2]);
    const volatile float u0 = (float)((long double)t0 + s0), u1 = (float)((long double)t1 + s1), u2 = (float)((long double)t2 + k1z);
    const long double x = k2x + u0;
    const long double y = (long double)u1 + r1;
    const volatile float v2 = (float)((long double)u2 + r2);
    out[0] = (float)(x + q0);
    out[1] = (float)(y + q1);
    out[2] = (float)((long double)v2 + q2);
}
static const uint8_t bezier_point_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Bezier::Point", 0xbcbf90, bezier_point_original, &bezier_point, &g_bezier_points);

// ---- Plane::FromPoints(a, b, c, normal, d) (0xCDCE20): the plane through three points ---------------------------------
// The normal is (b - a) x (c - a) made one long (a length of exactly 0 leaves x and y and sets z to 1), and d is minus
// the normal's dot with a ((az nz + ax nx) + ay ny). As the game: b - a stays unrounded, c - a's x and y are stored;
// the normal's x and y are stored before the length, its z kept unrounded (for the length and the scaling).
static volatile long g_planes_from_points;
static void __stdcall plane_from_points(const float *a, const float *b, const float *c, float *normal, float *d) {
    g_planes_from_points++;
    const long double e1x = (long double)b[0] - a[0], e1y = (long double)b[1] - a[1], e1z = (long double)b[2] - a[2];
    const volatile float e2x = (float)((long double)c[0] - a[0]), e2y = (float)((long double)c[1] - a[1]);
    const long double e2z = (long double)c[2] - a[2];
    const volatile float nx = (float)(e2z * e1y - (long double)e2y * e1z);
    const volatile float ny = (float)(e1z * e2x - e2z * e1x);
    const long double nz = e1x * e2y - e1y * e2x;
    volatile float *const n = normal;
    n[1] = ny;
    n[0] = nx;
    n[2] = (float)nz;
    const long double length = x87_sqrt((nz * nz + (long double)ny * ny) + (long double)nx * nx);
    if (length == 0.0L) {
        *(volatile uint32_t *)&n[2] = 0x3f800000;   // 1.0
    } else {
        const long double inverse = 1.0L / length;
        n[0] = (float)(nx * inverse);
        n[1] = (float)(ny * inverse);
        n[2] = (float)(nz * inverse);
    }
    *d = (float)-(((long double)a[2] * n[2] + (long double)a[0] * n[0]) + (long double)a[1] * n[1]);
}
static const uint8_t plane_from_points_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Plane::FromPoints", 0xcdce20, plane_from_points_original, &plane_from_points, &g_planes_from_points);

// ---- Plane::FromPointsC9B970(plane, a, b, c) (0xC9B970): the same plane, kept as {normal, -, d} -----------------------
// Here b - a and c - a are both stored first, the normal stays unrounded until it is stored (a length of exactly 0
// leaves x and y unscaled and makes z 1), and d (+0x10) is the normal's dot with a, (nz az + nx ax) + ny ay, taken from
// the unrounded normal and not negated.
static void __thiscall plane_from_points_c9b970(float *plane, const float *a, const float *b, const float *c) {
    g_planes_from_points++;
    const volatile float e1x = (float)((long double)b[0] - a[0]), e1y = (float)((long double)b[1] - a[1]), e1z = (float)((long double)b[2] - a[2]);
    const volatile float e2x = (float)((long double)c[0] - a[0]), e2y = (float)((long double)c[1] - a[1]), e2z = (float)((long double)c[2] - a[2]);
    long double nx = (long double)e1y * e2z - (long double)e1z * e2y;
    long double ny = (long double)e1z * e2x - (long double)e2z * e1x;
    long double nz = (long double)e1x * e2y - (long double)e1y * e2x;
    const long double length = x87_sqrt((nz * nz + ny * ny) + nx * nx);
    if (length == 0.0L) {
        nz = *(const float *)0xf264dc;   // 1.0
    } else {
        const long double inverse = 1.0L / length;
        nx = nx * inverse, ny = ny * inverse, nz = nz * inverse;
    }
    plane[0] = (float)nx;
    plane[1] = (float)ny;
    plane[2] = (float)nz;
    plane[4] = (float)((nz * a[2] + nx * a[0]) + ny * a[1]);
}
static const uint8_t plane_from_points_c9b970_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Plane::FromPointsC9B970", 0xc9b970, plane_from_points_c9b970_original, &plane_from_points_c9b970, &g_planes_from_points);

// ---- TrackPiece::Lerp5BC210(a, t, b) (0x5BC210): ten whole numbers blended toward another ten by t --------------------
// Each a[i] becomes a[i] (1 - t) + b[i] t, rounded by the FPU's rounding (fistp of a 64-bit value: its low half kept).
static volatile long g_track_lerps;
static inline int64_t x87_whole(long double v) {
    int64_t q;
    __asm__("fistpll %0" : "=m"(q) : "t"(v) : "st");
    return q;
}
static void __thiscall track_piece_lerp(int32_t *a, float t_f, const int32_t *b) {
    g_track_lerps++;
    const long double t = t_f, s = (long double)*(const float *)0xf264dc - t;
    for (int i = 0; i < 10; i++) {
        const long double mixed = (long double)a[i] * s + (long double)b[i] * t;
        a[i] = (int32_t)(uint32_t)x87_whole(mixed);
    }
}
static const uint8_t track_piece_lerp_original[] = {0x83, 0xec, 0x08, 0x8b, 0x44, 0x24, 0x10, 0xd9};
REWRITE("TrackPiece::Lerp5BC210", 0x5bc210, track_piece_lerp_original, &track_piece_lerp, &g_track_lerps);

// ---- Fence::CrossA905F0(out, p1..p8) (0xA905F0): where two lines cross ------------------------------------------------
// By Cramer's rule over det = p4 p7 - p3 p8: when |det| is at most 1e-9 (or not a number) the answer is (p1, p2);
// else x = X / det and y = Y / det with X and Y the game's sums of triple products (in its order), 1 / det kept
// unrounded for x and rounded (stored) for y, Y stored before. Gives out back.
static volatile long g_fence_crosses;
static float *__thiscall fence_cross(float *out, float p1, float p2, float p3, float p4, float p5, float p6, float p7, float p8) {
    g_fence_crosses++;
    const long double det = (long double)p4 * p7 - (long double)p3 * p8;
    const long double small = *(const float *)0xf918f0;
    if (!(x87_abs(det) > small)) {   // (at most 1e-9, or a NaN)
        out[0] = p1;
        out[1] = p2;
        return out;
    }
    const long double a = (long double)p2 * p3;
    long double x = (long double)p6 * p3 * p7 - a * p7;
    x = x + (long double)p1 * p4 * p7;
    x = x - (long double)p5 * p3 * p8;
    long double y = (long double)p6 * p4 * p7 - a * p8;
    y = y + (long double)p1 * p8 * p4;
    y = y - (long double)p5 * p8 * p4;
    const volatile float y_stored = (float)y;
    const long double inverse = 1.0L / det;
    const volatile float inverse_stored = (float)inverse;
    out[0] = (float)(inverse * x);
    out[1] = (float)((long double)inverse_stored * y_stored);
    return out;
}
static const uint8_t fence_cross_original[] = {0xd9, 0x44, 0x24, 0x10, 0xd8, 0x4c, 0x24, 0x1c};
REWRITE("Fence::CrossA905F0", 0xa905f0, fence_cross_original, &fence_cross, &g_fence_crosses);

// ---- Water::Scale8E6C10(this, p, s1, s2, out) (0x8E6C10): a matrix (rows 16 bytes apart) from the water's values -------
// Row 1 is the vector at +0xF0 times s1, row 2 the one at +0x100 times s2 (each rounded as stored); row 0 is row 1
// times the game's constant at 0xF3288C, less the vector at +0xE0, plus p: x with (row1 x K) rounded before the
// subtraction (stored), y unrounded throughout, z with (row1 z K) rounded first. The fifth argument isn't read.
static volatile long g_water_scales;
static void __thiscall water_scale(const uint8_t *self, const float *p, float s1, float s2, float *out, uint32_t) {
    g_water_scales++;
    const float *f = (const float *)(self + 0xf0), *g = (const float *)(self + 0x100), *e = (const float *)(self + 0xe0);
    volatile float *const o = out;
    o[6] = (float)((long double)f[2] * s1);
    o[4] = (float)((long double)f[0] * s1);
    o[5] = (float)((long double)f[1] * s1);
    o[10] = (float)((long double)g[2] * s2);
    o[8] = (float)((long double)g[0] * s2);
    o[9] = (float)((long double)g[1] * s2);
    const long double k = *(const float *)0xf3288c;
    const long double c0 = o[4] * k, c1 = o[5] * k;
    const volatile float c2 = (float)(o[6] * k);
    const volatile float d0 = (float)(c0 - e[0]);
    const long double d1 = c1 - e[1], d2 = (long double)c2 - e[2];
    o[0] = (float)((long double)d0 + p[0]);
    o[1] = (float)(d1 + p[1]);
    o[2] = (float)(d2 + p[2]);
}
static const uint8_t water_scale_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Water::Scale8E6C10", 0x8e6c10, water_scale_original, &water_scale, &g_water_scales);

// ---- Water::Point8E6D20(this, m, h, low, high) (0x8E6D20): the low and high corners of a box ---------------------------
// For each axis: from m's row 0 value, the row 1 and row 2 values and the water's vector at +0xE0 times h are each split
// into a part below 0 (added to the low corner) and a part from 0 up (added to the high one); a NaN goes to the low
// side, a -0 to both. Both sums stay unrounded until stored, the high one first.
static volatile long g_water_points;
static void __thiscall water_point(const uint8_t *self, const float *m, float h, float *low, float *high) {
    g_water_points++;
    const float *e = (const float *)(self + 0xe0);
    const long double zero = 0.0L;
    for (int k = 0; k < 3; k++) {
        long double lo = m[k], hi = m[k];
        const long double terms[3] = {m[4 + k], m[8 + k], (long double)e[k] * h};
        for (int i = 0; i < 3; i++) {
            const long double t = terms[i];
            lo = lo + (t > zero ? zero : t);
            hi = hi + (t >= zero ? t : zero);
        }
        high[k] = (float)hi;
        low[k] = (float)lo;
    }
}
static const uint8_t water_point_original[] = {0x83, 0xec, 0x08, 0x56, 0x81, 0xc1, 0xe0, 0x00};
REWRITE("Water::Point8E6D20", 0x8e6d20, water_point_original, &water_point, &g_water_points);

// ---- Water::Normal8E6150(this, v) (0x8E6150): a direction kept with two at right angles to it ------------------------
// v is copied to +0xE0. Its flat part (x, z) made one long and turned a quarter (z / l, 0, -x / l) goes to +0xF0, and
// the cross product of that with v to +0x100 (each part from the stored values; z rounded as stored before x and y).
// When x x + z z is within the game's small value (0xF3312C) or not a number: (1, 0, 0) and (0, 0, 1).
static volatile long g_water_normals;
static void __thiscall water_normal(uint8_t *self, const uint32_t *v) {
    g_water_normals++;
    volatile uint32_t *const bits = (volatile uint32_t *)(self + 0xe0);
    bits[0] = v[0], bits[1] = v[1], bits[2] = v[2];   // (copied as they are)
    volatile float *const e = (volatile float *)(self + 0xe0), *const f = (volatile float *)(self + 0xf0), *const g = (volatile float *)(self + 0x100);
    const long double e0 = e[0], e2 = e[2];
    const long double q = e0 * e0 + e2 * e2;
    if (!(x87_abs(q) > (long double)*(const float *)0xf3312c)) {
        volatile uint32_t *const fb = (volatile uint32_t *)f, *const gb = (volatile uint32_t *)g;
        fb[0] = 0x3f800000, fb[1] = 0, fb[2] = 0, gb[0] = 0, gb[1] = 0, gb[2] = 0x3f800000;
        return;
    }
    const long double inverse = 1.0L / x87_sqrt(q);
    *(volatile uint32_t *)&f[1] = 0;
    f[0] = (float)(inverse * e[2]);
    f[2] = (float)-(e0 * inverse);
    const long double g0 = (long double)f[2] * e[1] - (long double)f[1] * e[2];
    const long double g1 = (long double)f[0] * e[2] - (long double)f[2] * e[0];
    const volatile float g2 = (float)((long double)e[0] * f[1] - (long double)e[1] * f[0]);
    g[0] = (float)g0;
    g[1] = (float)g1;
    g[2] = g2;
}
static const uint8_t water_normal_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Water::Normal8E6150", 0x8e6150, water_normal_original, &water_normal, &g_water_normals);

// A float loaded onto the FPU as the game does it (fld: a signalling NaN comes back quiet), for the rewrites that only
// move or compare floats (the compiler would copy them as they are)
static inline long double x87_load(const float *p) {
    long double v;
    __asm__("flds %1" : "=t"(v) : "m"(*p));
    return v;
}
// fcomi b, a then fcmovnbe a (the smaller, a NaN keeping b) / fcmovb a (the larger, a NaN taking a)
static inline long double x87_lesser(long double a, long double b) { return b > a ? a : b; }
static inline long double x87_greater(long double a, long double b) { return !(b >= a) ? a : b; }

// ---- Rect::Union / Intersect (0xBE7690 / 0xBE7520) and UnionInto / IntersectInto (0xBE75E0 / 0xBE7470) ----------------
// Rectangles {left, top, right, bottom}. Union takes the smaller left and top and the larger right and bottom of this
// one and another (the second argument; the first is out), Intersect the other way round; the plain ones write the
// answer into this one (each side stored before the next is read) and copy it to out, the Into ones write it into out
// only.
static volatile long g_rects;
static void rect_combine(float *into, const float *self, const float *other, bool unite) {
    for (int i = 0; i < 4; i++) {
        const long double a = x87_load(&self[i]), b = x87_load(&other[i]);
        *(volatile float *)&into[i] = (float)(unite == (i < 2) ? x87_lesser(a, b) : x87_greater(a, b));
    }
}
static void __thiscall rect_union(float *self, uint32_t *out, const float *other) {
    g_rects++;
    rect_combine(self, self, other, true);
    for (int i = 0; i < 4; i++) out[i] = ((volatile uint32_t *)self)[i];
}
static void __thiscall rect_intersect(float *self, uint32_t *out, const float *other) {
    g_rects++;
    rect_combine(self, self, other, false);
    for (int i = 0; i < 4; i++) out[i] = ((volatile uint32_t *)self)[i];
}
static void __thiscall rect_union_into(const float *self, float *out, const float *other) {
    g_rects++;
    float r[4];
    rect_combine(r, self, other, true);
    for (int i = 0; i < 4; i++) ((volatile uint32_t *)out)[i] = ((volatile uint32_t *)r)[i];
}
static void __thiscall rect_intersect_into(const float *self, float *out, const float *other) {
    g_rects++;
    float r[4];
    rect_combine(r, self, other, false);
    for (int i = 0; i < 4; i++) ((volatile uint32_t *)out)[i] = ((volatile uint32_t *)r)[i];
}
static const uint8_t rect_union_original[] = {0x83, 0xec, 0x08, 0x8b, 0x44, 0x24, 0x10, 0x8b};
static const uint8_t rect_union_into_original[] = {0x83, 0xec, 0x14, 0x8b, 0x44, 0x24, 0x1c, 0x8b};
REWRITE("Rect::Union", 0xbe7690, rect_union_original, &rect_union, &g_rects);
REWRITE("Rect::Intersect", 0xbe7520, rect_union_original, &rect_intersect, &g_rects);
REWRITE("Rect::UnionInto", 0xbe75e0, rect_union_into_original, &rect_union_into, &g_rects);
REWRITE("Rect::IntersectInto", 0xbe7470, rect_union_into_original, &rect_intersect_into, &g_rects);

// ---- Matrix::Transpose / Transpose2 (0xC73730 / 0xC73680): a 4x4 matrix's transpose written into another ------------
// Everything is read first. Seven of the entries go through the FPU on the way (out 9 to 15: a signalling NaN comes out
// quiet), the rest are copied as they are.
static volatile long g_matrix_transposes;
static void __thiscall matrix_transpose(const uint32_t *in, uint32_t *out) {
    g_matrix_transposes++;
    uint32_t t[16];
    long double f[16];
    for (int i = 0; i < 16; i++) t[i] = in[(i & 3) * 4 + (i >> 2)];
    for (int i = 9; i < 16; i++) f[i] = x87_load((const float *)&in[(i & 3) * 4 + (i >> 2)]);
    for (int i = 0; i < 9; i++) ((volatile uint32_t *)out)[i] = t[i];
    for (int i = 9; i < 16; i++) ((volatile float *)out)[i] = (float)f[i];
}
static const uint8_t matrix_transpose_original[] = {0x83, 0xec, 0x20, 0x8b, 0x41, 0x08, 0xd9, 0x41};
REWRITE("Matrix::Transpose", 0xc73730, matrix_transpose_original, &matrix_transpose, &g_matrix_transposes);
REWRITE("Matrix::Transpose2", 0xc73680, matrix_transpose_original, &matrix_transpose, &g_matrix_transposes);

// ---- Matrix::From4x3(in, out) (0x856550): four rows of three made a 4x4 matrix, the last column 0, 0, 0, 1 ------------
// The first row's three values go through the FPU (a signalling NaN comes out quiet); the rest are copied as they are.
static volatile long g_matrix_expands;
static void __thiscall matrix_from_4x3(const uint32_t *in, uint32_t *out) {
    g_matrix_expands++;
    uint32_t t[12];
    for (int i = 0; i < 12; i++) t[i] = in[i];
    const long double a = x87_load((const float *)&in[0]), b = x87_load((const float *)&in[1]), c = x87_load((const float *)&in[2]);
    volatile uint32_t *const o = out;
    ((volatile float *)o)[0] = (float)a, ((volatile float *)o)[1] = (float)b, ((volatile float *)o)[2] = (float)c;
    for (int row = 1; row < 4; row++)
        for (int k = 0; k < 3; k++) o[row * 4 + k] = t[row * 3 + k];
    o[3] = 0, o[7] = 0, o[11] = 0, o[15] = 0x3f800000;
}
static const uint8_t matrix_from_4x3_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Matrix::From4x3", 0x856550, matrix_from_4x3_original, &matrix_from_4x3, &g_matrix_expands);

// ---- Quaternion::ToMatrix(q, out) (0xB00C00): a rotation quaternion (x, y, z, w) as a matrix's rotation rows -----------
// With x2 = x + x and so on: 2xx, 2yx and 2zw stay unrounded, the other products (2xz, 2xw, 2yy, 2yw, 2zy, 2zz) are
// stored first; rows at 0, 4 and 8 (their fourth values left alone).
static volatile long g_quaternion_matrices;
static void __thiscall quaternion_to_matrix(const float *q, float *out) {
    g_quaternion_matrices++;
    const long double x = q[0], y = q[1], z = q[2], w = q[3], one = *(const float *)0xf264dc;
    const long double x2 = x + x, y2 = y + y, z2 = z + z;
    const long double xx = x2 * x;
    const volatile float xz = (float)(x2 * z), xw = (float)(x2 * w);
    const long double yx = y2 * x;
    const volatile float yy = (float)(y2 * y), yw = (float)(y2 * w);
    const volatile float zy = (float)(z2 * y), zz = (float)(z2 * z);
    const long double zw = z2 * w;
    out[0] = (float)((one - yy) - zz);
    out[1] = (float)(zw + yx);
    out[2] = (float)((long double)xz - yw);
    out[4] = (float)(yx - zw);
    out[5] = (float)(((one - xx) - zz));
    out[6] = (float)((long double)zy + xw);
    out[8] = (float)((long double)yw + xz);
    out[9] = (float)((long double)zy - xw);
    out[10] = (float)((one - xx) - yy);
}
static const uint8_t quaternion_to_matrix_original[] = {0x83, 0xec, 0x18, 0xd9, 0x01, 0x8b, 0x44, 0x24};
REWRITE("Quaternion::ToMatrix", 0xb00c00, quaternion_to_matrix_original, &quaternion_to_matrix, &g_quaternion_matrices);

// ---- CameraAngles::Clamp(this) (0xA61B90): the two tilts (+0x30, +0x34) kept within +-1.5677, the turn (+0x38) within
// a whole turn ------------------------------------------------------------------------------------------------------------
// Each tilt: the smaller of it and 1.5677 (a NaN kept), then the larger of that and -1.5677 (a NaN kept). The turn has
// its whole turns taken off: t / (2 pi) (as a float) rounded down, times 2 pi, taken from t.
static volatile long g_camera_clamps;
static void __thiscall camera_angles_clamp(uint8_t *self) {
    g_camera_clamps++;
    static const uint32_t limit_bits = 0x3fc8a8ea, low_bits = 0xbfc8a8ea;
    const long double limit = x87_load((const float *)&limit_bits), low = x87_load((const float *)&low_bits);
    for (int k = 0; k < 2; k++) {
        float *const a = (float *)(self + 0x30 + 4 * k);
        const volatile float t = (float)x87_lesser(limit, x87_load(a));
        *(volatile float *)a = (float)x87_greater(x87_load((const float *)&t), low);
    }
    float *const turn = (float *)(self + 0x38);
    const volatile float turns = (float)((long double)*(volatile float *)turn * (long double)*(const float *)0xf36ee4);
    const long double whole = x87_round(turns);
    const volatile float down = (float)(turns < whole ? whole - 1.0L : whole);
    *(volatile float *)turn = (float)((long double)*(volatile float *)turn - down * (long double)*(const float *)0xf268dc);
}
static const uint8_t camera_angles_clamp_original[] = {0x83, 0xec, 0x10, 0x8b, 0x41, 0x30, 0xc7, 0x04};
REWRITE("CameraAngles::Clamp", 0xa61b90, camera_angles_clamp_original, &camera_angles_clamp, &g_camera_clamps);

// ---- Vector::TransformPoint(out, p, m) (0x481360) and TransformPoint2(m, out, p) (0xBB6050): a point times a 4x4 ------
// matrix with its translation: out[j] = ((m[8 + j] p2 + m[4 + j] p1) + m[j] p0) + m[12 + j], all read first.
static volatile long g_point_transforms;
static void transform_point(float *out, const float *p, const float *m) {
    const long double p0 = p[0], p1 = p[1], p2 = p[2];
    long double r[3];
    for (int j = 0; j < 3; j++) r[j] = (((long double)m[8 + j] * p2 + (long double)m[4 + j] * p1) + (long double)m[j] * p0) + m[12 + j];
    for (int j = 0; j < 3; j++) out[j] = (float)r[j];
}
static void __fastcall vector_transform_point(float *out, const float *p, const float *m) {
    g_point_transforms++;
    transform_point(out, p, m);
}
static void __thiscall vector_transform_point2(const float *m, float *out, const float *p) {
    g_point_transforms++;
    transform_point(out, p, m);
}
static const uint8_t vector_transform_point_original[] = {0x83, 0xec, 0x1c, 0xd9, 0x02, 0x56, 0x8b, 0x74};
static const uint8_t vector_transform_point2_original[] = {0x83, 0xec, 0x1c, 0x8b, 0x44, 0x24, 0x24, 0xd9};
REWRITE("Vector::TransformPoint", 0x481360, vector_transform_point_original, &vector_transform_point, &g_point_transforms);
REWRITE("Vector::TransformPoint2", 0xbb6050, vector_transform_point2_original, &vector_transform_point2, &g_point_transforms);

// ---- Quad::EdgeMiddles(quad, out0..out4) (0x727DE0): five points from a quad's corners (16 bytes apart) ----------------
// With A = (x0 + x1) / 2 and B = (z0 + z2) / 2 (each rounded once) and the y's halved sums of two corners: out0 = (x0,
// (y2 + y0) / 2, B), out1 = (x1, (y3 + y1) / 2, B), out2 = (A, (y1 + y0) / 2, z0), out3 = (A, (y3 + y2) / 2, z2), out4 =
// (A, (y2 + y1) / 2, B). x0 and the last B are copied as they are, x1, z0 and z2 go through the FPU (quieting a
// signalling NaN).
static volatile long g_quad_middles;
static void __fastcall quad_edge_middles(const float *q, float *out0, float *out1, float *out2, float *out3, float *out4) {
    g_quad_middles++;
    const long double half = *(const float *)0xf2651c;
    const long double x1 = x87_load(&q[4]), z0 = x87_load(&q[2]), z2 = x87_load(&q[10]);
    const uint32_t x0_bits = *(const uint32_t *)&q[0];
    const volatile float a = (float)(((long double)q[0] + q[4]) * half);
    const volatile float b = (float)(((long double)q[2] + q[10]) * half);
    *(volatile uint32_t *)&out0[0] = x0_bits;
    out0[1] = (float)(((long double)q[9] + q[1]) * half);
    out0[2] = b;
    out1[0] = (float)x1;
    out1[1] = (float)(((long double)q[13] + q[5]) * half);
    out1[2] = b;
    out2[0] = a;
    out2[1] = (float)(((long double)q[5] + q[1]) * half);
    out2[2] = (float)z0;
    out3[0] = a;
    out3[1] = (float)(((long double)q[13] + q[9]) * half);
    out3[2] = (float)z2;
    out4[0] = a;
    *(volatile uint32_t *)&out4[2] = *(const volatile uint32_t *)&b;
    out4[1] = (float)(((long double)q[9] + q[5]) * half);
}
static const uint8_t quad_edge_middles_original[] = {0x83, 0xec, 0x08, 0xd9, 0x41, 0x10, 0x8b, 0x01};
REWRITE("Quad::EdgeMiddles", 0x727de0, quad_edge_middles_original, &quad_edge_middles, &g_quad_middles);

// ---- Spline::SegmentPoints(this, n, p0, p1, p2, p3) (0x4240B0): the four control points of segment n ------------------
// The spline's points (0x24 bytes each, at the pointer at +0x44): p0 is point n, p3 point n + 1 (both through the FPU:
// a signalling NaN comes out quiet), p1 = p0 + point n's out-handle (+0x18), p2 = p3 + point n + 1's in-handle (+0xC),
// each sum from the copies just written.
static volatile long g_spline_segments;
static void __thiscall spline_segment_points(const uint8_t *self, int32_t n, float *p0, float *p1, float *p2, float *p3) {
    g_spline_segments++;
    const float *const a = (const float *)(*(uint8_t *const volatile *)(self + 0x44) + n * 0x24);
    for (int i = 0; i < 3; i++) *(volatile float *)&p0[i] = (float)x87_load(&a[i]);
    const float *const b = (const float *)(*(uint8_t *const volatile *)(self + 0x44) + n * 0x24 + 0x24);
    for (int i = 0; i < 3; i++) *(volatile float *)&p3[i] = (float)x87_load(&b[i]);
    const float *const out_handle = (const float *)(*(uint8_t *const volatile *)(self + 0x44) + n * 0x24 + 0x18);
    long double s[3];
    for (int i = 0; i < 3; i++) s[i] = (long double)out_handle[i] + *(volatile float *)&p0[i];
    for (int i = 0; i < 3; i++) p1[i] = (float)s[i];
    const float *const in_handle = (const float *)(*(uint8_t *const volatile *)(self + 0x44) + n * 0x24 + 0x30);
    for (int i = 0; i < 3; i++) s[i] = (long double)in_handle[i] + *(volatile float *)&p3[i];
    for (int i = 0; i < 3; i++) p2[i] = (float)s[i];
}
static const uint8_t spline_segment_points_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Spline::SegmentPoints", 0x4240b0, spline_segment_points_original, &spline_segment_points, &g_spline_segments);

// ---- Points::UpdateDepth(this, n) (0x42FF50): point n's depth and its moved place --------------------------------------
// Of point n (0x70 bytes each, in the list this points to), with u its first vector (+0) and v its second (+0x10):
// its depth (+0x24) is ((u + v).z m38 + (u + v).y m28) + (u + v).x m18, plus m48 (this's view values, the sums kept
// unrounded); its place at +0x40 is u + v k (k the game's constant at 0xF283D8, each v k rounded first); +0x28 is set.
static volatile long g_point_depths;
static void __thiscall points_update_depth(const uint8_t *self, int32_t n) {
    g_point_depths++;
    uint8_t *const p = *(uint8_t *const volatile *)self + n * 0x70;
    const float *const f = (const float *)p, *const m = (const float *)self;
    const long double s0 = (long double)f[4] + f[0], s1 = (long double)f[5] + f[1], s2 = (long double)f[6] + f[2];
    *(volatile float *)(p + 0x24) = (float)(((s2 * m[0x38 / 4] + s1 * m[0x28 / 4]) + s0 * m[0x18 / 4]) + m[0x48 / 4]);
    const long double k = *(const float *)0xf283d8;
    const volatile float t0 = (float)((long double)f[4] * k), t1 = (float)((long double)f[5] * k), t2 = (float)((long double)f[6] * k);
    const long double r0 = (long double)f[0] + t0, r1 = (long double)t1 + f[1];
    const volatile float r2 = (float)((long double)t2 + f[2]);
    float *const out = (float *)(p + 0x40);
    out[2] = r2;
    out[0] = (float)r0;
    out[1] = (float)r1;
    *(volatile uint8_t *)(*(uint8_t *const volatile *)self + n * 0x70 + 0x28) = 1;
}
static const uint8_t points_update_depth_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Points::UpdateDepth", 0x42ff50, points_update_depth_original, &points_update_depth, &g_point_depths);

// ---- Bits::PackScaled(v) (0xC197D0): v times the game's constant (0xFAFE84, as a float) rounded down, packed -----------
// The whole number w (its low 32 bits) goes into three fields: bits 20-29 hold w's low 10 bits, 10-18 and 0-8 its low 9.
static volatile long g_bit_packs;
static uint32_t __stdcall bits_pack_scaled(int32_t v) {
    g_bit_packs++;
    const volatile float f = (float)((long double)v * *(const float *)0xfafe84);
    const long double whole = x87_round(f);
    const volatile float down = (float)(f < whole ? whole - 1.0L : whole);
    const uint32_t w = (uint32_t)x87_whole(down);
    return ((((w & 0x3ff) << 10) | (w & 0x1ff)) << 10) | (w & 0x1ff);
}
static const uint8_t bits_pack_scaled_original[] = {0x83, 0xec, 0x14, 0xdb, 0x44, 0x24, 0x18, 0xd8};
REWRITE("Bits::PackScaled", 0xc197d0, bits_pack_scaled_original, &bits_pack_scaled, &g_bit_packs);

// ---- Vector::SideDirection(this, out) (0x4262F0): the ground-plane direction across the vector at +0x3F0 ---------------
// With t = y 0 (rounded): (z - t, x 0 - z 0, t - x), made one long; when its length is exactly 0 the first two are
// given unscaled and the last is 1.
static volatile long g_side_directions;
static void __thiscall vector_side_direction(const uint8_t *self, float *out) {
    g_side_directions++;
    const float *const v = (const float *)(self + 0x3f0);
    const long double zero = *(const float *)0xf264e0;
    const volatile float t = (float)((long double)v[1] * zero);
    const long double a = (long double)v[2] - t;
    const long double b = (long double)v[0] * zero - (long double)v[2] * zero;
    const long double c = (long double)t - v[0];
    const long double length = x87_sqrt((c * c + b * b) + a * a);
    if (length == 0.0L) {
        out[0] = (float)a;
        out[1] = (float)b;
        out[2] = *(const float *)0xf264dc;
        return;
    }
    const long double inverse = 1.0L / length;
    out[0] = (float)(a * inverse);
    out[1] = (float)(b * inverse);
    out[2] = (float)(c * inverse);
}
static const uint8_t vector_side_direction_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Vector::SideDirection", 0x4262f0, vector_side_direction_original, &vector_side_direction, &g_side_directions);

// ---- Segment::Direction(this, out) (0x5BE7F0): the unit direction from one part's point to another's -----------------
// From the point of the part at +0x2C to the point of the part at +0x28 (each part's +4): d = b - a, its x rounded
// (stored), made one long (the length sqrt((z z + y y) + x x)); when the length is exactly 0, (0, 0, 1).
static volatile long g_segment_directions;
static void __thiscall segment_direction(const uint8_t *self, float *out) {
    g_segment_directions++;
    const float *const a = *(const float *const *)(*(const uint8_t *const *)(self + 0x2c) + 4);
    const float *const b = *(const float *const *)(*(const uint8_t *const *)(self + 0x28) + 4);
    const volatile float dx = (float)((long double)b[0] - a[0]);
    const long double dy = (long double)b[1] - a[1], dz = (long double)b[2] - a[2];
    const long double length = x87_sqrt((dz * dz + dy * dy) + (long double)dx * dx);
    if (length == 0.0L) {
        volatile uint32_t *const o = (volatile uint32_t *)out;
        o[0] = 0, o[1] = 0, o[2] = 0x3f800000;
        return;
    }
    const long double inverse = 1.0L / length;
    out[0] = (float)(dx * inverse);
    out[1] = (float)(dy * inverse);
    out[2] = (float)(dz * inverse);
}
static const uint8_t segment_direction_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Segment::Direction", 0x5be7f0, segment_direction_original, &segment_direction, &g_segment_directions);

// ---- Lerp::Current(this) (0xC219D0): the point now along a blend, into this (+0), given back ---------------------------
// From the start (+0x20) to the end (+0x10) by the amount t (+0x30): once t is at least 1, the end as it is; else
// start (1 - t) + end t, each product rounded (stored) except end.z t.
static volatile long g_lerp_currents;
static float *__thiscall lerp_current(float *self) {
    g_lerp_currents++;
    const long double t = self[0x30 / 4];
    if (!(t < (long double)*(const float *)0xf264dc) && t == t) {   // (at least 1; a NaN blends)
        volatile uint32_t *const s = (volatile uint32_t *)self;
        s[0] = s[4], s[1] = s[5], s[2] = s[6];
        return self;
    }
    const volatile float ex = (float)(t * self[4]), ey = (float)(t * self[5]);
    const long double ez = t * self[6];
    const long double s = (long double)*(const float *)0xf264dc - self[0x30 / 4];
    const volatile float sx = (float)(s * self[8]), sy = (float)(s * self[9]), sz = (float)(s * self[10]);
    const long double x = (long double)sx + ex, y = (long double)sy + ey;
    const volatile float z = (float)((long double)sz + ez);
    self[2] = z;
    self[0] = (float)x;
    self[1] = (float)y;
    return self;
}
static const uint8_t lerp_current_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Lerp::Current", 0xc219d0, lerp_current_original, &lerp_current, &g_lerp_currents);

// ---- Math::Round(this, a, b) (0xCDB7B0): two values scaled, added to and split into whole and fraction -----------------
// For a (and b likewise into +0xC..+0x14): f = a k + this[2] (k the game's constant at 0xF27370, f rounded as stored),
// w = f rounded down; this[0] = a as given, this[1] = w as a whole number (its low 32 bits), this[2] = f - w.
static volatile long g_math_rounds;
static void __thiscall math_round(uint32_t *self, float a, float b) {
    g_math_rounds++;
    const float args[2] = {a, b};
    for (int k = 0; k < 2; k++) {
        uint32_t *const s = self + 3 * k;
        const volatile float f = (float)((long double)args[k] * *(const float *)0xf27370 + *(volatile float *)&s[2]);
        const long double whole = x87_round(f);
        const volatile float down = (float)(f < whole ? whole - 1.0L : whole);
        const uint32_t w = (uint32_t)x87_whole(down);
        ((volatile uint32_t *)s)[0] = *(const uint32_t *)&args[k];
        ((volatile uint32_t *)s)[1] = w;
        *(volatile float *)&s[2] = (float)((long double)f - down);
    }
}
static const uint8_t math_round_original[] = {0x83, 0xec, 0x10, 0xd9, 0x44, 0x24, 0x14, 0xd8};
REWRITE("Math::Round", 0xcdb7b0, math_round_original, &math_round, &g_math_rounds);

// ---- VerletPoint::Init(this, position, velocity, owner) (0xA6B190): a moving point for Verlet steps --------------------
// +0 the owner given, +0x10 the position, +0x20 the previous position (the position less the velocity times the time
// step 0xF8F980; the x product rounded first), +0x2C the largest float, +0x30 a damping of 0.99, +0x34 a weight of 1,
// +0x40 no force.
static volatile long g_verlet_inits;
static void __thiscall verlet_point_init(uint32_t *self, const uint32_t *position, const float *velocity, uint32_t owner) {
    g_verlet_inits++;
    volatile uint32_t *const s = self;
    s[0] = owner;
    s[4] = position[0], s[5] = position[1], s[6] = position[2];
    s[0x2c / 4] = 0x7f7fffff, s[0x30 / 4] = 0x3f7d70a4, s[0x34 / 4] = 0x3f800000;
    s[0x40 / 4] = 0, s[0x44 / 4] = 0, s[0x48 / 4] = 0;
    const long double step = *(const float *)0xf8f980;
    s[8] = s[4], s[9] = s[5], s[10] = s[6];
    const volatile float vx = (float)((long double)velocity[0] * step);
    const long double vy = (long double)velocity[1] * step, vz = (long double)velocity[2] * step;
    volatile float *const f = (volatile float *)self;
    f[8] = (float)((long double)f[8] - vx);
    f[9] = (float)((long double)f[9] - vy);
    f[10] = (float)((long double)f[10] - vz);
}
static const uint8_t verlet_point_init_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("VerletPoint::Init", 0xa6b190, verlet_point_init_original, &verlet_point_init, &g_verlet_inits);

// ---- Particle::SetColours(this, first, second) (0x9C6210): two 0xRRGGBB colours as parts from 0 to 1 -----------------
// Each byte times the game's 1/255 (0xF270F0): first's red, green, blue at +0xAC, +0xB0, +0xB4, second's at +0xB8 on.
static volatile long g_particle_colours;
static void __thiscall particle_set_colours(uint8_t *self, int32_t first, int32_t second) {
    g_particle_colours++;
    const long double k = *(const float *)0xf270f0;
    const int32_t colours[2] = {first, second};
    for (int c = 0; c < 2; c++)
        for (int part = 0; part < 3; part++)
            *(volatile float *)(self + 0xac + c * 12 + part * 4) = (float)((long double)((colours[c] >> (16 - 8 * part)) & 0xff) * k);
}
static const uint8_t particle_set_colours_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0xd0, 0xc1, 0xfa};
REWRITE("Particle::SetColours", 0x9c6210, particle_set_colours_original, &particle_set_colours, &g_particle_colours);

// ---- Grid::SnapIndex(flag, v, size, scale, tolerance) (0xBC4FF0): the nearest whole step for v ------------------------
// n = ((size / 2 + v) scale) (as a float) rounded down; when |v - n size| is within the tolerance, *flag = 0 and n is
// given; otherwise (or for a NaN) *flag = 1 and the step v lies in, (v scale) rounded down. (Low 32 bits of each.)
static volatile long g_grid_snaps;
static int32_t __thiscall grid_snap_index(uint8_t *flag, float v, float size, float scale, float tolerance) {
    g_grid_snaps++;
    const volatile float f = (float)(((long double)size * *(const float *)0xf2651c + v) * scale);
    const long double whole = x87_round(f);
    const volatile float down = (float)(f < whole ? whole - 1.0L : whole);
    const int32_t n = (int32_t)(uint32_t)x87_whole(down);
    if (x87_abs((long double)v - (long double)n * size) <= tolerance) {
        *(volatile uint8_t *)flag = 0;
        return n;
    }
    *(volatile uint8_t *)flag = 1;
    const volatile float g = (float)((long double)v * scale);
    const long double whole2 = x87_round(g);
    const volatile float down2 = (float)(g < whole2 ? whole2 - 1.0L : whole2);
    return (int32_t)(uint32_t)x87_whole(down2);
}
static const uint8_t grid_snap_index_original[] = {0x83, 0xec, 0x0c, 0xd9, 0x44, 0x24, 0x14, 0xd8};
REWRITE("Grid::SnapIndex", 0xbc4ff0, grid_snap_index_original, &grid_snap_index, &g_grid_snaps);

// ---- Progress::Advance(this, time) (0xB34EF0): a progress value moved on, with a second one following it --------------
// a = (time k + this[0x134]) (k at 0xF270B4) kept at most 1; the second value (+0x150) gains (a - the old value) times
// the count at +0xD8 times k, kept at most 1. Gives whether a is past the threshold at 0xF3B794 (a NaN: no).
static volatile long g_progress_advances;
static int32_t __thiscall progress_advance(uint8_t *self, float time) {
    g_progress_advances++;
    static const uint32_t one_bits = 0x3f800000;
    const long double one = x87_load((const float *)&one_bits), k = *(const float *)0xf270b4;
    volatile float *const p = (volatile float *)(self + 0x134), *const q = (volatile float *)(self + 0x150);
    const volatile float sum = (float)((long double)time * k + *p);
    const volatile float a = (float)x87_lesser(one, x87_load((const float *)&sum));
    const long double old = *p;
    *(volatile uint32_t *)p = *(const volatile uint32_t *)&a;
    const volatile float s = (float)((long double)*(const int32_t *)(self + 0xd8) * k * ((long double)a - old) + *q);
    *q = s;
    *q = (float)x87_lesser(one, x87_load((const float *)&s));
    return (long double)a > (long double)*(const float *)0xf3b794;
}
static const uint8_t progress_advance_original[] = {0x83, 0xec, 0x0c, 0xd9, 0x44, 0x24, 0x10, 0xc7};
REWRITE("Progress::Advance", 0xb34ef0, progress_advance_original, &progress_advance, &g_progress_advances);

// ---- Camera::SetFromHeight(this, height, lift) (0x7455F0): the camera's offset from a height and its field of view -----
// d = -(height / 2 + c) / tan(fov / 2) (c at 0xF270B0, fov at +0x290; tan by fptan), d rounded as stored and kept
// unrounded too: the offset (+0x260) is (d dx, D dy + lift + c, D dz) with D the rounded d and (dx, dy, dz) the
// direction at +0x240 (D dy rounded first); then marked changed (+0x2A4 bit 1, and bit 0 when +0x2A7 is set).
static volatile long g_camera_heights;
static inline long double x87_tan(long double v) {
    __asm__("fptan\n\tfstp %%st(0)" : "+t"(v));
    return v;
}
static void __thiscall camera_set_from_height(uint8_t *self, float height, float lift) {
    g_camera_heights++;
    const long double half = *(const float *)0xf2651c, c = *(const float *)0xf270b0;
    const float *const dir = (const float *)(self + 0x240);
    const long double q = (long double)height * half + c;
    const long double t = x87_tan((long double)*(const float *)(self + 0x290) * half);
    const long double d = -(q / t);
    const volatile float d_stored = (float)d;
    const long double x = d * dir[0];
    const volatile float y = (float)((long double)d_stored * dir[1]);
    const long double z = (long double)d_stored * dir[2];
    float *const offset = (float *)(self + 0x260);
    offset[0] = (float)x;
    offset[1] = (float)(((long double)y + lift) + c);
    offset[2] = (float)z;
    *(volatile uint8_t *)(self + 0x2a4) |= *(const volatile uint8_t *)(self + 0x2a7) ? 3 : 2;
}
static const uint8_t camera_set_from_height_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Camera::SetFromHeight", 0x7455f0, camera_set_from_height_original, &camera_set_from_height, &g_camera_heights);
