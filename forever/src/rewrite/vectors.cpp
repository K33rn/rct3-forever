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
static float *__thiscall vector_rotate_around(float *v, const float *axis, float angle) {   // gives v, as the game's does
    g_vector_rotations++;
    const volatile float s = (float)x87_sin(angle), c = (float)x87_cos(angle);
    rotate_about(v, axis, s, c);
    return v;
}
static float *__thiscall vector_rotate_around_sin_cos(float *v, const float *axis, float sin_f, float cos_f) {   // gives v
    g_vector_rotations++;
    rotate_about(v, axis, sin_f, cos_f);
    return v;
}
static const uint8_t vector_rotate_around_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Vector::RotateAround", 0xbfb0b0, vector_rotate_around_original, &vector_rotate_around, &g_vector_rotations);
REWRITE("Vector::RotateAroundSinCos", 0xbfb1c0, vector_rotate_around_original, &vector_rotate_around_sin_cos, &g_vector_rotations);

// ---- Bezier::Point(out, p0, p1, p2, p3, t) (0xBCBF90): the point at t on a cubic curve --------------------------------
// s^3 p0 + 3 s^2 t p1 + 3 s t^2 p2 + t^3 p3, s = 1 - t, with the game's weights ((s t) t 3, (t (s s)) 3, (s s) s, t t t)
// and its sums: x = (3st^2 p2x + (s^3 p0x + 3s^2t p1x)) + t^3 p3x, y likewise, z = ((s^3 p0z + 3s^2t p1z) + 3st^2 p2z)
// + t^3 p3z; the products and partial sums it stores are rounded there, the rest kept unrounded.
static volatile long g_bezier_points;
static float *__fastcall bezier_point(float *out, const float *p0, const float *p1, const float *p2, const float *p3, float t_f) {
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
    return out;
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
static uint32_t *__thiscall rect_union(float *self, uint32_t *out, const float *other) {
    g_rects++;
    rect_combine(self, self, other, true);
    for (int i = 0; i < 4; i++) out[i] = ((volatile uint32_t *)self)[i];
    return out;
}
static uint32_t *__thiscall rect_intersect(float *self, uint32_t *out, const float *other) {
    g_rects++;
    rect_combine(self, self, other, false);
    for (int i = 0; i < 4; i++) out[i] = ((volatile uint32_t *)self)[i];
    return out;
}
static float *__thiscall rect_union_into(const float *self, float *out, const float *other) {
    g_rects++;
    float r[4];
    rect_combine(r, self, other, true);
    for (int i = 0; i < 4; i++) ((volatile uint32_t *)out)[i] = ((volatile uint32_t *)r)[i];
    return out;
}
static float *__thiscall rect_intersect_into(const float *self, float *out, const float *other) {
    g_rects++;
    float r[4];
    rect_combine(r, self, other, false);
    for (int i = 0; i < 4; i++) ((volatile uint32_t *)out)[i] = ((volatile uint32_t *)r)[i];
    return out;
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
static float *__thiscall vector_side_direction(const uint8_t *self, float *out) {
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
        return out;
    }
    const long double inverse = 1.0L / length;
    out[0] = (float)(a * inverse);
    out[1] = (float)(b * inverse);
    out[2] = (float)(c * inverse);
    return out;
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

// ---- Terrain::Fill8BE510 (0x8BE510) and Terrain::Make8BE610 (0x8BE610): a triangle / a quad of terrain vertices --------
// Each vertex (0x20 bytes, at the buffer *vertices points to): its point (copied as it is), +0xC a, +0x10 0, +0x14 b,
// +0x18 a colour, +0x1C (y - base) scale. Then the indices (16-bit, at *indices) from the running count: a triangle
// takes n, n+1, n+2 (count + 3), a quad n, n+1, n+2, n+2, n+1, n+3 (count + 4); both pointers are moved on.
static volatile long g_terrain_vertices;
static void terrain_vertex(uint32_t *v, const uint32_t *p, uint32_t a, uint32_t b, uint32_t colour, float base, float scale) {
    volatile uint32_t *const o = v;
    o[0] = p[0], o[1] = p[1], o[2] = p[2];
    o[5] = b, o[3] = a, o[6] = colour, o[4] = 0;
    ((volatile float *)o)[7] = (float)(((long double)((const float *)p)[1] - base) * scale);
}
static void terrain_indices(uint16_t **indices, uint16_t *count, const uint8_t *steps, int n, int vertices) {
    for (int i = 0; i < n; i++) ((volatile uint16_t *)*indices)[i] = (uint16_t)(*(volatile uint16_t *)count + steps[i]);
    *indices += n;
    *(volatile uint16_t *)count += (uint16_t)vertices;
}
static void __fastcall terrain_fill(const uint32_t *p0, const uint32_t *p1, const uint32_t *p2, uint32_t c0, uint32_t c1, uint32_t c2, uint32_t a, uint32_t b,
                                    float base, float scale, uint32_t **vertices, uint16_t **indices, uint16_t *count) {
    g_terrain_vertices++;
    terrain_vertex(*vertices, p0, a, b, c0, base, scale);
    terrain_vertex(*vertices + 8, p1, a, b, c1, base, scale);
    terrain_vertex(*vertices + 16, p2, a, b, c2, base, scale);
    *vertices += 24;
    static const uint8_t steps[3] = {0, 1, 2};
    terrain_indices(indices, count, steps, 3, 3);
}
static void __fastcall terrain_make(const uint32_t *p1, const uint32_t *p3, const uint32_t *p0, const uint32_t *p2, uint32_t c01, uint32_t c23, uint32_t a,
                                    uint32_t b, float base, float scale, uint32_t **vertices, uint16_t **indices, uint16_t *count) {
    g_terrain_vertices++;
    terrain_vertex(*vertices, p0, a, b, c01, base, scale);
    terrain_vertex(*vertices + 8, p1, a, b, c01, base, scale);
    terrain_vertex(*vertices + 16, p2, a, b, c23, base, scale);
    terrain_vertex(*vertices + 24, p3, a, b, c23, base, scale);
    *vertices += 32;
    static const uint8_t steps[6] = {0, 1, 2, 2, 1, 3};
    terrain_indices(indices, count, steps, 6, 4);
}
static const uint8_t terrain_fill_original[] = {0x53, 0x8b, 0x5c, 0x24, 0x1c, 0x56, 0x8b, 0x74};
static const uint8_t terrain_make_original[] = {0x53, 0x55, 0x8b, 0x6c, 0x24, 0x20, 0x56, 0x8b};
REWRITE("Terrain::Fill8BE510", 0x8be510, terrain_fill_original, &terrain_fill, &g_terrain_vertices);
REWRITE("Terrain::Make8BE610", 0x8be610, terrain_make_original, &terrain_make, &g_terrain_vertices);

// ---- Vector::StepBCC090(out, p0, p1, p2, p3, t) (0xBCC090): a cubic curve's direction at t -----------------------------
// 3 ((s^2 (p1 - p0) + 2st (p2 - p1)) + t^2 (p3 - p2)), s = 1 - t, with the game's roundings: t t, 2st and s s stored;
// each difference's z stored; of each weighted difference x and z stored (as are the partial sums of x, y and z), y of
// the first two kept unrounded.
static volatile long g_curve_steps;
static float *__fastcall vector_step_bcc090(float *out, const float *p0, const float *p1, const float *p2, const float *p3, float t_f) {
    g_curve_steps++;
    const long double t = t_f, s = (long double)*(const float *)0xf264dc - t, three = *(const float *)0xf2784c;
    const long double d3x = (long double)p3[0] - p2[0], d3y = (long double)p3[1] - p2[1];
    const volatile float d3z = (float)((long double)p3[2] - p2[2]);
    const volatile float tt = (float)(t * t);
    const volatile float a0 = (float)(d3x * tt);
    const long double a1 = d3y * tt;
    const volatile float a2 = (float)((long double)d3z * tt);
    const long double d2x = (long double)p2[0] - p1[0], d2y = (long double)p2[1] - p1[1];
    const volatile float d2z = (float)((long double)p2[2] - p1[2]);
    const long double st = s * t;
    const volatile float w = (float)(st + st);
    const volatile float b0 = (float)(d2x * w);
    const long double b1 = d2y * w, b2 = (long double)d2z * w;
    const long double d1x = (long double)p1[0] - p0[0], d1y = (long double)p1[1] - p0[1];
    const volatile float d1z = (float)((long double)p1[2] - p0[2]);
    const volatile float ss = (float)(s * s);
    const volatile float c0 = (float)(d1x * ss);
    const long double c1 = d1y * ss;
    const volatile float c2 = (float)((long double)d1z * ss);
    const volatile float x1 = (float)((long double)c0 + b0), y1 = (float)(c1 + b1), z1 = (float)((long double)c2 + b2);
    const volatile float x2 = (float)((long double)x1 + a0);
    const long double y2 = (long double)y1 + a1, z2 = (long double)z1 + a2;
    out[0] = (float)((long double)x2 * three);
    out[1] = (float)(y2 * three);
    out[2] = (float)(z2 * three);
    return out;
}
static const uint8_t vector_step_bcc090_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Vector::StepBCC090", 0xbcc090, vector_step_bcc090_original, &vector_step_bcc090, &g_curve_steps);

// ---- Water::Compare95DE00(vertices, indices, count, a, b, c, d, sum, low) (0x95DE00): a water quad, its heights noted --
// The four points go to the vertex buffer (12 bytes each, copied as they are), six 32-bit indices from the count (n,
// n+2, n+1, n+1, n+2, n+3), the pointers and the count (+4) moved on. *sum gains ((b.y + d.y) + a.y) + c.y; *low becomes
// the smallest of c.y, d.y, b.y, a.y and itself, taken in that order by fcomi / fcmov (NaNs as they fall).
static volatile long g_water_quads;
static void __fastcall water_compare(uint32_t **vertices, uint32_t **indices, uint32_t *count, const uint32_t *a, const uint32_t *b, const uint32_t *c,
                                     const uint32_t *d, float *sum, float *low) {
    g_water_quads++;
    volatile uint32_t *const v = *vertices;
    volatile uint32_t *const ix = *indices;
    const uint32_t n = *count;
    *vertices += 12, *indices += 6, *count += 4;
    const uint32_t *const points[4] = {a, b, c, d};
    for (int p = 0; p < 4; p++) v[3 * p] = points[p][0], v[3 * p + 1] = points[p][1], v[3 * p + 2] = points[p][2];
    ix[0] = n, ix[1] = n + 2, ix[4] = n + 2, ix[2] = n + 1, ix[3] = n + 1, ix[5] = n + 3;
    const float *const fa = (const float *)a, *const fb = (const float *)b, *const fc = (const float *)c, *const fd = (const float *)d;
    *(volatile float *)sum = (float)(((((long double)fb[1] + fd[1]) + fa[1]) + fc[1]) + *(volatile float *)sum);
    volatile float m = (float)x87_lesser(x87_load(&fc[1]), x87_load(&fd[1]));
    m = (float)x87_lesser(x87_load(&fb[1]), x87_load((const float *)&m));
    m = (float)x87_lesser(x87_load(&fa[1]), x87_load((const float *)&m));
    *(volatile float *)low = (float)x87_lesser(x87_load(low), x87_load((const float *)&m));
}
static const uint8_t water_compare_original[] = {0x8b, 0x01, 0x53, 0x8b, 0x5c, 0x24, 0x08, 0x55};
REWRITE("Water::Compare95DE00", 0x95de00, water_compare_original, &water_compare, &g_water_quads);

// ---- Text::Layout690F00(out, c0, w0, w1, c1, w2, c2, w3, c3) (0x690F00): four colours mixed and packed ----------------
// Each channel (alpha +0xC, then +0, +4, +8): ((w0 c0 + w1 c1) + w2 c2) + w3 c3, times the game's 255 (0xF264C0),
// rounded by fistp (low 32 bits); packed by shifting the running value left 8 and or-ing the next one in, unmasked.
static volatile long g_text_mixes;
static void __fastcall text_layout_690f00(uint32_t *out, const float *c0, float w0, float w1, const float *c1, float w2, const float *c2, float w3,
                                          const float *c3) {
    g_text_mixes++;
    static const int channels[4] = {3, 0, 1, 2};
    uint32_t packed = 0;
    for (int i = 0; i < 4; i++) {
        const int k = channels[i];
        const long double mixed = (((long double)w0 * c0[k] + (long double)w1 * c1[k]) + (long double)w2 * c2[k]) + (long double)w3 * c3[k];
        const uint32_t v = (uint32_t)x87_whole(mixed * *(const float *)0xf264c0);
        packed = i ? (packed << 8) | v : v;
    }
    *(volatile uint32_t *)out = packed;
}
static const uint8_t text_layout_690f00_original[] = {0x83, 0xec, 0x08, 0xd9, 0x44, 0x24, 0x0c, 0x8b};
REWRITE("Text::Layout690F00", 0x690f00, text_layout_690f00_original, &text_layout_690f00, &g_text_mixes);

// ---- Water::Fill95DD10(vertices, indices, count, a, b, c, sum, low) (0x95DD10): a water triangle, its heights noted -----
// As Water::Compare95DE00 with three points: indices n, n+1, n+2 (count + 3); *sum gains ((b.y + a.y) + c.y); *low
// the smallest of b.y and c.y, then a.y, then itself.
static void __fastcall water_fill(uint32_t **vertices, uint32_t **indices, uint32_t *count, const uint32_t *a, const uint32_t *b, const uint32_t *c,
                                  float *sum, float *low) {
    g_water_quads++;
    volatile uint32_t *const v = *vertices;
    volatile uint32_t *const ix = *indices;
    const uint32_t n = *count;
    *vertices += 9, *indices += 3, *count += 3;
    const uint32_t *const points[3] = {a, b, c};
    for (int p = 0; p < 3; p++) v[3 * p] = points[p][0], v[3 * p + 1] = points[p][1], v[3 * p + 2] = points[p][2];
    ix[0] = n, ix[1] = n + 1, ix[2] = n + 2;
    const float *const fa = (const float *)a, *const fb = (const float *)b, *const fc = (const float *)c;
    *(volatile float *)sum = (float)((((long double)fb[1] + fa[1]) + fc[1]) + *(volatile float *)sum);
    volatile float m = (float)x87_lesser(x87_load(&fb[1]), x87_load(&fc[1]));
    m = (float)x87_lesser(x87_load(&fa[1]), x87_load((const float *)&m));
    *(volatile float *)low = (float)x87_lesser(x87_load(low), x87_load((const float *)&m));
}
static const uint8_t water_fill_original[] = {0x8b, 0x01, 0x53, 0x55, 0x56, 0x8b, 0x74, 0x24};
REWRITE("Water::Fill95DD10", 0x95dd10, water_fill_original, &water_fill, &g_water_quads);

// ---- Fence::QuadA91C80(n, records, indices, colour, 20 values) (0xA91C80): quad n's record (0x60 bytes) and indices ----
// Four vertices of six dwords: (x, y, z, colour, u, v) from the values given in order, copied as they are; six 16-bit
// indices at word 6n: 4n, 4n+1, 4n+2, 4n+2, 4n+1, 4n+3.
static volatile long g_fence_quads;
static void __stdcall fence_quad(int32_t n, uint8_t *records, uint16_t *indices, uint32_t colour, uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3,
                                 uint32_t a4, uint32_t a5, uint32_t a6, uint32_t a7, uint32_t a8, uint32_t a9, uint32_t a10, uint32_t a11, uint32_t a12,
                                 uint32_t a13, uint32_t a14, uint32_t a15, uint32_t a16, uint32_t a17, uint32_t a18, uint32_t a19) {
    g_fence_quads++;
    const uint32_t values[20] = {a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19};
    volatile uint32_t *const r = (volatile uint32_t *)(records + n * 0x60);
    for (int v = 0; v < 4; v++) {
        r[6 * v] = values[5 * v], r[6 * v + 1] = values[5 * v + 1], r[6 * v + 2] = values[5 * v + 2];
        r[6 * v + 3] = colour, r[6 * v + 4] = values[5 * v + 3], r[6 * v + 5] = values[5 * v + 4];
    }
    volatile uint16_t *const ix = indices + n * 6;
    const uint16_t d = (uint16_t)(n * 4);
    ix[0] = d, ix[2] = (uint16_t)(d + 2), ix[3] = (uint16_t)(d + 2), ix[1] = (uint16_t)(d + 1), ix[4] = (uint16_t)(d + 1), ix[5] = (uint16_t)(d + 3);
}
static const uint8_t fence_quad_original[] = {0x8b, 0x4c, 0x24, 0x04, 0x8b, 0x54, 0x24, 0x08};
REWRITE("Fence::QuadA91C80", 0xa91c80, fence_quad_original, &fence_quad, &g_fence_quads);

// ---- Record::Fill9CD960(this, point, rows, value) (0x9CD960): a record from a point and six rows ----------------------
// +0 the point, +0x10 the value (copied as they are); for each of the six rows (32 bytes apart from +0x10 in rows): its
// first value copied as it is, its second and third through the FPU (a signalling NaN comes out quiet), its fifth
// negated; the record's rows 16 bytes apart from +0x20.
static volatile long g_record_fills;
static void __thiscall record_fill(uint32_t *self, const uint32_t *point, const uint8_t *rows, uint32_t value) {
    g_record_fills++;
    volatile uint32_t *const s = self;
    s[0] = point[0], s[1] = point[1], s[2] = point[2];
    s[4] = value;
    for (int k = 0; k < 6; k++) {
        const uint8_t *const row = rows + 0x10 + 0x20 * k;
        const long double e = x87_load((const float *)(row + 0x10)), c = x87_load((const float *)(row + 8)), b = x87_load((const float *)(row + 4));
        s[8 + 4 * k] = *(const uint32_t *)row;
        ((volatile float *)s)[9 + 4 * k] = (float)b;
        ((volatile float *)s)[10 + 4 * k] = (float)c;
        ((volatile float *)s)[11 + 4 * k] = (float)-e;
    }
}
static const uint8_t record_fill_original[] = {0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x04, 0x8b, 0x11};
REWRITE("Record::Fill9CD960", 0x9cd960, record_fill_original, &record_fill, &g_record_fills);

// ---- Animal::ClampA5EDC0(this) (0xA5EDC0): three values kept within -1000 and 1000 ------------------------------------
// Each: the smaller of it and 1000 (by fcomi / fcmov: a NaN gives 1000), then the larger of that and -1000.
static volatile long g_animal_clamps;
static void __thiscall animal_clamp(float *self) {
    g_animal_clamps++;
    static const uint32_t high_bits = 0x447a0000, low_bits = 0xc47a0000;
    for (int k = 0; k < 3; k++) {
        const volatile float t = (float)x87_lesser(x87_load(&self[k]), x87_load((const float *)&high_bits));
        *(volatile float *)&self[k] = (float)x87_greater(x87_load((const float *)&t), x87_load((const float *)&low_bits));
    }
}
static const uint8_t animal_clamp_original[] = {0x83, 0xec, 0x10, 0x8b, 0x01, 0xc7, 0x04, 0x24};
REWRITE("Animal::ClampA5EDC0", 0xa5edc0, animal_clamp_original, &animal_clamp, &g_animal_clamps);

// ---- Waterfall::Set919430(this, a, b, c, d, e) (0x919430): the waterfall's points ------------------------------------
// +0xA0 a, +0xB0 b, +0xE0 e (copied as they are); +0xF0 and +0x100 the vectors at c + 0x10 and d + 0x10 (their x and y
// through the FPU, a signalling NaN coming out quiet, their z copied); +0x300 set; +0x40 gains bits 2 and 3 (bit 2 stored
// first).
static volatile long g_waterfall_sets;
static void __thiscall waterfall_set(uint8_t *self, const uint32_t *a, const uint32_t *b, const uint8_t *c, const uint8_t *d, const uint32_t *e) {
    g_waterfall_sets++;
    volatile uint32_t *const s = (volatile uint32_t *)self;
    s[0xa0 / 4] = a[0], s[0xa4 / 4] = a[1], s[0xa8 / 4] = a[2];
    s[0xb0 / 4] = b[0], s[0xb4 / 4] = b[1], s[0xb8 / 4] = b[2];
    const uint8_t *const from[2] = {c, d};
    for (int k = 0; k < 2; k++) {
        const long double x = x87_load((const float *)(from[k] + 0x10)), y = x87_load((const float *)(from[k] + 0x14));
        s[(0xf8 + 0x10 * k) / 4] = *(const uint32_t *)(from[k] + 0x18);
        ((volatile float *)s)[(0xf0 + 0x10 * k) / 4] = (float)x;
        ((volatile float *)s)[(0xf4 + 0x10 * k) / 4] = (float)y;
    }
    s[0xe0 / 4] = e[0], s[0xe4 / 4] = e[1], s[0xe8 / 4] = e[2];
    *(volatile uint8_t *)(self + 0x300) = 1;
    const uint32_t flags = s[0x40 / 4] | 4;
    s[0x40 / 4] = flags;
    s[0x40 / 4] = flags | 8;
}
static const uint8_t waterfall_set_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Waterfall::Set919430", 0x919430, waterfall_set_original, &waterfall_set, &g_waterfall_sets);

// ---- TileCorners::Pack(this, c1..c8) (0x727890): eight two-bit corner values into +0x30, four looked-up nibbles into
// +0x32 ---------------------------------------------------------------------------------------------------------------
// +0x30 (16 bits): c3 c1 c6 c4 c2 c5 c8 c7, two bits each (shifted in that order, unmasked). +0x32: four entries of the
// table at 0xF6A500, at (c3, c2, c8), (c1, c2, c7), (c6, c5, c8) and (c4, c5, c7) (each index the three two-bit values
// read as 16-bit numbers and shifted together), each entry or-ed in after a shift of 4.
static volatile long g_tile_packs;
static void __thiscall tile_corners_pack(uint8_t *self, uint32_t c1, uint32_t c2, uint32_t c3, uint32_t c4, uint32_t c5, uint32_t c6, uint32_t c7, uint32_t c8) {
    g_tile_packs++;
    uint32_t s = c3 * 4 | c1;
    s = (s << 2) | c6;
    s = (s << 2) | c4;
    s = (s << 2) | c2;
    s = (s << 2) | c5;
    s = (s << 2) | c8;
    s = (s << 2) | c7;
    *(volatile uint16_t *)(self + 0x30) = (uint16_t)s;
    const uint16_t *const table = (const uint16_t *)0xf6a500;
    auto index = [](uint32_t a, uint32_t b, uint32_t c) { return ((((a & 0xffff) << 2) | (b & 0xffff)) << 2) | (c & 0xffff); };
    uint16_t packed = table[index(c3, c2, c8)];
    packed = (uint16_t)((packed << 4) | table[index(c1, c2, c7)]);
    packed = (uint16_t)((packed << 4) | table[index(c6, c5, c8)]);
    packed = (uint16_t)((packed << 4) | table[index(c4, c5, c7)]);
    *(volatile uint16_t *)(self + 0x32) = packed;
}
static const uint8_t tile_corners_pack_original[] = {0x8b, 0x44, 0x24, 0x10, 0x53, 0x8b, 0x5c, 0x24};
REWRITE("TileCorners::Pack", 0x727890, tile_corners_pack_original, &tile_corners_pack, &g_tile_packs);

// ---- Scenery::Normal87A370(out, a, b, t) (0x87A370): two scenery records (nine floats) blended by t ---------------------
// The first three: a + (b - a) t (each difference's x and each step stored first). The next three likewise, kept
// unrounded and made one long (sqrt((z z + y y) + x x)); a length of exactly 0 leaves x and y and makes z 1. +0x18 is
// a's as it is; +0x1C and +0x20 are (b - a) t + a.
static volatile long g_scenery_normals;
static void __fastcall scenery_normal(float *out, const float *a, const float *b, float t_f) {
    g_scenery_normals++;
    const long double t = t_f;
    long double r[6];
    for (int part = 0; part < 2; part++) {
        const float *const pa = a + 3 * part, *const pb = b + 3 * part;
        const volatile float d0 = (float)((long double)pb[0] - pa[0]);
        const long double d1 = (long double)pb[1] - pa[1], d2 = (long double)pb[2] - pa[2];
        const volatile float e0 = (float)((long double)d0 * t), e1 = (float)(d1 * t), e2 = (float)(d2 * t);
        r[3 * part] = (long double)pa[0] + e0;
        r[3 * part + 1] = (long double)e1 + pa[1];
        r[3 * part + 2] = (long double)e2 + pa[2];
    }
    volatile float *const o = out;
    const volatile float z0 = (float)r[2];
    o[2] = z0, o[0] = (float)r[0], o[1] = (float)r[1];
    const long double x = r[3], y = r[4], z = r[5];
    const long double length = x87_sqrt((z * z + y * y) + x * x);
    if (length == 0.0L) {
        *(volatile uint32_t *)&o[5] = 0x3f800000;
        o[3] = (float)x, o[4] = (float)y;
    } else {
        const long double inverse = 1.0L / length;
        const long double nx = x * inverse, ny = y * inverse;
        o[5] = (float)(z * inverse);
        o[3] = (float)nx, o[4] = (float)ny;
    }
    ((volatile uint32_t *)o)[6] = ((const uint32_t *)a)[6];
    o[7] = (float)(((long double)b[7] - a[7]) * t + a[7]);
    o[8] = (float)(((long double)b[8] - a[8]) * t + a[8]);
}
static const uint8_t scenery_normal_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Scenery::Normal87A370", 0x87a370, scenery_normal_original, &scenery_normal, &g_scenery_normals);

// ---- Vector::NormalBEDC00(this) (0xBEDC00): a matrix's three rows (16 bytes apart) made upright again -----------------
// Row 2 is made one long (sqrt((x x + y y) + z z); a length of exactly 0 leaves x and y and makes z 1). Row 0 becomes
// row 1 x row 2 made one long (its y rounded before the length, x and z unrounded; length 0 the same way), and row 1
// becomes row 2 x row 0, all from the stored values.
static volatile long g_matrix_uprights;
static float *__thiscall vector_normal_bedc00(float *m) {
    g_matrix_uprights++;
    volatile float *const r = m;
    {
        const long double x = r[8], y = r[9], z = r[10];
        const long double length = x87_sqrt((x * x + y * y) + z * z);
        if (length == 0.0L) {
            *(volatile uint32_t *)&r[10] = 0x3f800000;
        } else {
            const long double inverse = 1.0L / length;
            r[8] = (float)(inverse * r[8]);
            r[9] = (float)(inverse * r[9]);
            r[10] = (float)(inverse * r[10]);
        }
    }
    const long double c0 = (long double)r[10] * r[5] - (long double)r[6] * r[9];
    const volatile float c1 = (float)((long double)r[6] * r[8] - (long double)r[10] * r[4]);
    r[1] = c1;
    const long double c2 = (long double)r[4] * r[9] - (long double)r[8] * r[5];
    r[0] = (float)c0;
    r[2] = (float)c2;
    const long double length = x87_sqrt((c2 * c2 + (long double)c1 * c1) + c0 * c0);
    if (length == 0.0L) {
        *(volatile uint32_t *)&r[2] = 0x3f800000;
    } else {
        const long double inverse = 1.0L / length;
        r[0] = (float)(inverse * c0);
        r[1] = (float)((long double)c1 * inverse);
        r[2] = (float)(c2 * inverse);
    }
    const long double d0 = (long double)r[2] * r[9] - (long double)r[10] * r[1];
    const long double d1 = (long double)r[10] * r[0] - (long double)r[2] * r[8];
    const volatile float d2 = (float)((long double)r[1] * r[8] - (long double)r[9] * r[0]);
    r[6] = d2;
    r[4] = (float)d0;
    r[5] = (float)d1;
    return m;
}
static const uint8_t vector_normal_bedc00_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Vector::NormalBEDC00", 0xbedc00, vector_normal_bedc00_original, &vector_normal_bedc00, &g_matrix_uprights);

// ---- Particles::TransformC(this) (0x9CDBA0): a particle's point (+0x10) moved by the shared matrix, stored in its slot --
// d = point - the shared origin (0x13415B0; d.y rounded, d.z rounded and kept unrounded), each result
// ((dz m[j][2] + dy m[j][1]) + m[j][0] dx) + m[j][3] (the matrix at 0x1341580, rows of four; row 0 takes dz unrounded),
// into entry n (+0x34) of the list at +0x30 (32 bytes each), the colour (+0x20, four dwords) beside it (+0x10). Then n
// moves on (wrapping to 0 at the count, +4) and the fill count (+0x38) goes up, kept at most the count.
static volatile long g_particle_transforms;
static void __thiscall particles_transform_c(uint8_t *self) {
    g_particle_transforms++;
    const float *const p = (const float *)(self + 0x10), *const origin = (const float *)0x13415b0, *const m = (const float *)0x1341580;
    const long double dx = (long double)p[0] - origin[0];
    const volatile float dy = (float)((long double)p[1] - origin[1]);
    const long double dz = (long double)p[2] - origin[2];
    const volatile float dz_stored = (float)dz;
    const long double x = ((dz * m[2] + (long double)dy * m[1]) + (long double)m[0] * dx) + m[3];
    const long double y = (((long double)dz_stored * m[6] + (long double)dy * m[5]) + (long double)m[4] * dx) + m[7];
    const volatile float z = (float)((((long double)dz_stored * m[10] + (long double)dy * m[9]) + (long double)m[8] * dx) + m[11]);
    volatile uint32_t *const s = (volatile uint32_t *)self;
    float *const out = (float *)(uintptr_t)(s[0x30 / 4] + (s[0x34 / 4] << 5));
    out[2] = z;
    out[0] = (float)x;
    out[1] = (float)y;
    volatile uint32_t *const colour = (volatile uint32_t *)(uintptr_t)(s[0x30 / 4] + (s[0x34 / 4] << 5) + 0x10);
    for (int i = 0; i < 4; i++) colour[i] = s[0x20 / 4 + i];
    const int32_t next = (int32_t)s[0x34 / 4] + 1;
    s[0x34 / 4] = (uint32_t)next;
    const int32_t count = (int32_t)s[1];
    if (next >= count) s[0x34 / 4] = 0;
    const int32_t filled = (int32_t)s[0x38 / 4] + 1;
    s[0x38 / 4] = (uint32_t)filled;
    if (filled > count) s[0x38 / 4] = (uint32_t)count;
}
static const uint8_t particles_transform_c_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Particles::TransformC", 0x9cdba0, particles_transform_c_original, &particles_transform_c, &g_particle_transforms);

// ---- Vector::NormalBEE460(out, v) (0xBEE460): the rotation matrix (rows 16 bytes apart) about v by its length ----------
// When v.v (as (x x + y y) + z z) is under the game's small value (0xF27C70): the identity. Else with a = |v| (rounded),
// c and s its cosine and sine (fsincos, each rounded), k = 1 - c (rounded) and u = v / a (u.x from the unrounded 1 / a and rounded,
// y and z from the rounded one): the usual u u^T k + c I + s [u]x, in the game's order and roundings.
static volatile long g_axis_rotations;
static inline void x87_sincos(long double v, long double *s, long double *c) {
    long double cs, sn;
    __asm__("fsincos" : "=t"(cs), "=u"(sn) : "0"(v));
    *s = sn, *c = cs;
}
static float *__thiscall vector_normal_bee460(float *out, const float *v) {
    g_axis_rotations++;
    const long double q = ((long double)v[0] * v[0] + (long double)v[1] * v[1]) + (long double)v[2] * v[2];
    volatile uint32_t *const o = (volatile uint32_t *)out;
    if (q < (long double)*(const float *)0xf27c70) {
        o[1] = 0, o[2] = 0, o[0] = 0x3f800000, o[4] = 0, o[6] = 0, o[5] = 0x3f800000, o[8] = 0, o[9] = 0, o[10] = 0x3f800000;
        return out;
    }
    const volatile float a = (float)x87_sqrt(q);
    long double sn, cs;
    x87_sincos(a, &sn, &cs);
    const volatile float c = (float)cs, s = (float)sn;
    const long double one = *(const float *)0xf264dc;
    const long double inverse_exact = one / a;
    const volatile float inverse = (float)inverse_exact;
    const volatile float ux = (float)(inverse_exact * v[0]);   // (the reciprocal before it is stored)
    const long double uy = (long double)inverse * v[1], uz = (long double)inverse * v[2];
    const volatile float k = (float)(one - c);
    const long double a1 = (long double)ux * s, b1 = uy * s, c1 = uz * s;
    const long double p = uy * ux * k;
    const volatile float p2 = (float)(uz * ux * k), p3 = (float)(uz * uy * k);
    volatile float *const f = out;
    f[0] = (float)((long double)ux * ux * k + c);
    f[1] = (float)(c1 + p);
    f[2] = (float)((long double)p2 - b1);
    f[4] = (float)(p - c1);
    f[5] = (float)(uy * uy * k + c);
    f[6] = (float)(a1 + p3);
    f[8] = (float)(b1 + p2);
    f[9] = (float)((long double)p3 - a1);
    f[10] = (float)(uz * uz * k + c);
    return out;
}
static const uint8_t vector_normal_bee460_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Vector::NormalBEE460", 0xbee460, vector_normal_bee460_original, &vector_normal_bee460, &g_axis_rotations);

// ---- Grid::CurvedOffsets(grid, x_out, a, b, y_out) (0x7C3370): a place on a grid made two curved offsets ---------------
// x = a / (width - 1) (width the byte at +4), y = b / (height - 1) (+5); each less a half, times -2, then squared
// keeping its sign (times -1 when below 0 or a NaN, else 1), then times 0.95; every step stored in the outputs.
static volatile long g_curved_offsets;
static void __fastcall grid_curved_offsets(const uint8_t *grid, float *x_out, float a, float b, float *y_out) {
    g_curved_offsets++;
    volatile float *const x = x_out, *const y = y_out;
    *x = (float)((long double)a / (long double)(int32_t)(grid[4] - 1));
    *y = (float)((long double)b / (long double)(int32_t)(grid[5] - 1));
    const long double half = *(const float *)0xf2651c, k = *(const float *)0xf2e17c, last = *(const float *)0xf29660;
    *x = (float)(*x - half);
    *y = (float)(*y - half);
    *x = (float)(*x * k);
    *y = (float)(*y * k);
    volatile float *const both[2] = {x, y};
    for (int i = 0; i < 2; i++) {
        const long double v = *both[i];
        const long double sign = !(v >= (long double)*(const float *)0xf264e0) ? *(const float *)0xf268f0 : *(const float *)0xf264dc;
        *both[i] = (float)(v * v * sign);
    }
    *x = (float)(*x * last);
    *y = (float)(*y * last);
}
static const uint8_t grid_curved_offsets_original[] = {0x51, 0x0f, 0xb6, 0x41, 0x04, 0x48, 0x89, 0x04};
REWRITE("Grid::CurvedOffsets", 0x7c3370, grid_curved_offsets_original, &grid_curved_offsets, &g_curved_offsets);

// ---- Wobble::Update(this, -) (0x414270): three wobble values (+0x130 to +0x138) from a phase (+0x150) -------------------
// A phase neither above 2 nor above 0 is set to 5 first. With q = phase - 0.5: +0x130 =
// (sin(q pi/4) + 1) (strength (+0x140) / 2), +0x134 = sin(q 3pi/4) +0x144, +0x138 = sin(q pi/10) +0x148 (fsin; the
// halved strength and q pi/10 stored first).
static volatile long g_wobbles;
static void __thiscall wobble_update(uint8_t *self, uint32_t) {
    g_wobbles++;
    float *const f = (float *)self;
    const long double half = *(const float *)0xf2651c;
    const volatile float h = (float)((long double)f[0x140 / 4] * half);
    const long double p = *(volatile float *)&f[0x150 / 4];
    if (!(p > (long double)*(const float *)0xf2705c) && !(p > (long double)*(const float *)0xf264e0))
        *(volatile uint32_t *)&f[0x150 / 4] = 0x40a00000;   // 5.0
    const long double amplitude = f[0x144 / 4];
    const uint32_t second_bits = *(const uint32_t *)&f[0x148 / 4];
    const long double q = (long double)*(volatile float *)&f[0x150 / 4] - half;
    const long double r = (long double)*(const float *)0xf27058 * q;
    const volatile float t = (float)(q * *(const float *)0xf27054);
    *(volatile float *)&f[0x130 / 4] = (float)((x87_sin(q * *(const float *)0xf27050) + *(const float *)0xf264dc) * h);
    *(volatile float *)&f[0x134 / 4] = (float)(x87_sin(r) * amplitude);
    *(volatile float *)&f[0x138 / 4] = (float)(x87_sin(t) * (long double)*(const float *)&second_bits);
}
static const uint8_t wobble_update_original[] = {0x83, 0xec, 0x0c, 0xd9, 0x81, 0x40, 0x01, 0x00};
REWRITE("Wobble::Update", 0x414270, wobble_update_original, &wobble_update, &g_wobbles);

// ---- DistanceConstraint::DistanceConstraint(this, a, b, k, flag, w) (0xA6B310): a constraint keeping two points apart --
// +0 a, +4 b, +0xC k, +0x10 the flag, +0x14 w; +8 the distance between the points' positions (+0x10 of each: x of the
// difference stored, the length sqrt((z z + y y) + x x)), +0x18 its square (from the unrounded length); +0x1C k over
// the sum of the two points' first values (their weights), or 0 when that sum is exactly 0.
static volatile long g_distance_constraints;
static uint32_t *__thiscall distance_constraint(uint32_t *self, const float *a, const float *b, float k, uint32_t flag, uint32_t w) {
    g_distance_constraints++;
    volatile uint32_t *const s = self;
    s[3] = *(const uint32_t *)&k;
    ((volatile uint8_t *)self)[0x10] = (uint8_t)flag;
    s[0] = (uint32_t)(uintptr_t)a;
    s[5] = w;
    s[1] = (uint32_t)(uintptr_t)b;
    const volatile float dx = (float)((long double)b[4] - a[4]);
    const long double dy = (long double)b[5] - a[5], dz = (long double)b[6] - a[6];
    const long double length = x87_sqrt((dz * dz + dy * dy) + (long double)dx * dx);
    ((volatile float *)self)[2] = (float)length;
    ((volatile float *)self)[6] = (float)(length * length);
    const long double sum = (long double)a[0] + b[0];
    ((volatile float *)self)[7] = (float)sum;
    if (sum == 0.0L)
        s[7] = 0;
    else
        ((volatile float *)self)[7] = (float)((long double)k / sum);
    return self;
}
static const uint8_t distance_constraint_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("DistanceConstraint::DistanceConstraint", 0xa6b310, distance_constraint_original, &distance_constraint, &g_distance_constraints);

// ---- Animation::FrameAt(this, time) (0x911C80): an animation's time moved on, and the frame it is at ------------------
// The time (+0x20) gains the step and wraps once at 2 (above 2: less 2). With the animation's first track (the info at
// (+0x30)'s +8: the first entry of its list at +0x34 when its count +0x30 is above 0) giving the frame count n (+4) and the frames per unit f
// (+8): p = n time / 2 (rounded), k = p rounded down; +0x24 = (k + 0.5) / f, +0x28 = that + 1 / f (unrounded), +0x2C =
// p - k.
static volatile long g_animation_frames;
static void __thiscall animation_frame_at(uint8_t *self, float step) {
    g_animation_frames++;
    const uint8_t *const info = *(const uint8_t *const *)(*(const uint8_t *const *)(self + 0x30) + 8);
    const uint8_t *const track = *(const uint16_t *)(info + 0x30) ? **(const uint8_t *const *const *)(info + 0x34) : nullptr;
    volatile float *const f = (volatile float *)self;
    const long double two = *(const float *)0xf2705c, half = *(const float *)0xf2651c;
    const long double t = (long double)step + f[0x20 / 4];
    f[0x20 / 4] = (float)t;
    if (t > two)
        f[0x20 / 4] = (float)(t - two);
    const volatile float p = (float)((long double)*(const int32_t *)(track + 4) * f[0x20 / 4] * half);
    const long double whole = x87_round(p);
    const volatile float down = (float)(p < whole ? whole - 1.0L : whole);
    const int32_t k = (int32_t)(uint32_t)x87_whole(down);
    const long double inverse = (long double)*(const float *)0xf264dc / *(const float *)(track + 8);
    const long double start = ((long double)k + half) * inverse;
    f[0x24 / 4] = (float)start;
    f[0x28 / 4] = (float)(start + inverse);
    f[0x2c / 4] = (float)((long double)p - (long double)k);
}
static const uint8_t animation_frame_at_original[] = {0x8b, 0x41, 0x30, 0x8b, 0x40, 0x08, 0x83, 0xec};
REWRITE("Animation::FrameAt", 0x911c80, animation_frame_at_original, &animation_frame_at, &g_animation_frames);

// ---- AvatarEditor::Tick(this, time) (0x749120): two looping timers moved on ------------------------------------------
// The first (+0xD0C) gains the time and loses its whole periods (+0xD10): (t / period rounded down) period. Unless
// +0xD6C is set or +0x18 is 0, the second (+0xD18) gains the time times the game's constant (0xF270B0) and loses its
// whole part. Each value stored before it is used again.
static volatile long g_avatar_ticks;
static void __thiscall avatar_editor_tick(uint8_t *self, const float *time) {
    g_avatar_ticks++;
    volatile float *const f = (volatile float *)self;
    const long double a = (long double)*time + f[0xd0c / 4];
    f[0xd0c / 4] = (float)a;
    const volatile float q = (float)(a / f[0xd10 / 4]);
    const long double whole = x87_round(q);
    const volatile float down = (float)(q < whole ? whole - 1.0L : whole);
    f[0xd0c / 4] = (float)((long double)f[0xd0c / 4] - (long double)down * f[0xd10 / 4]);
    if (self[0xd6c] || !*(const uint32_t *)(self + 0x18))
        return;
    const long double b = (long double)*time * *(const float *)0xf270b0 + f[0xd18 / 4];
    f[0xd18 / 4] = (float)b;
    const volatile float bf = (float)b;
    const long double whole2 = x87_round(bf);
    const volatile float down2 = (float)(bf < whole2 ? whole2 - 1.0L : whole2);
    f[0xd18 / 4] = (float)((long double)f[0xd18 / 4] - down2);
}
static const uint8_t avatar_editor_tick_original[] = {0x51, 0x8b, 0x44, 0x24, 0x08, 0xd9, 0x00, 0xd8};
REWRITE("AvatarEditor::Tick", 0x749120, avatar_editor_tick_original, &avatar_editor_tick, &g_avatar_ticks);

// ---- Points::MidpointLength(a, b, n) (0x92DFD0): the length of the point halfway between point n of two lists ---------
// Points of 0x24 bytes (each list's +4). The halves of x and y are rounded (stored), z's kept; the length
// sqrt((z z + y y) + x x) is given back unrounded on the FPU.
static volatile long g_midpoint_lengths;
static long double __stdcall points_midpoint_length(const uint8_t *a, const uint8_t *b, int32_t n) {
    g_midpoint_lengths++;
    const float *const pa = (const float *)(*(const uint8_t *const *)(a + 4) + n * 0x24);
    const float *const pb = (const float *)(*(const uint8_t *const *)(b + 4) + n * 0x24);
    const long double half = *(const float *)0xf2651c;
    const volatile float sx = (float)((long double)pa[0] + pb[0]);
    const long double sy = (long double)pa[1] + pb[1], sz = (long double)pa[2] + pb[2];
    const volatile float hx = (float)((long double)sx * half), hy = (float)(sy * half);
    const long double hz = sz * half;
    return x87_sqrt((hz * hz + (long double)hy * hy) + (long double)hx * hx);
}
static const uint8_t points_midpoint_length_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Points::MidpointLength", 0x92dfd0, points_midpoint_length_original, &points_midpoint_length, &g_midpoint_lengths);

// ---- Frame::Turn(this, angle) (0xBD4C90): a frame's two axis pairs turned by an angle ---------------------------------
// (+0x10, +0x14) and (+0x20, +0x24) each become (c a + s b, c b - a s) with c, s the angle's cosine and sine (fcos,
// fsin, unrounded), from the old values. The game then adds (+0x50 less itself) and (+0x54 less itself) to +0x30 and
// +0x34 (0, or a NaN, which still turns a -0 there into +0) and writes +0x38 back as it was.
static volatile long g_frame_turns;
static void __thiscall frame_turn(uint8_t *self, float angle) {
    g_frame_turns++;
    volatile float *const f = (volatile float *)self;
    const long double p50 = f[0x50 / 4], p54 = f[0x54 / 4];
    const uint32_t r20_bits = *(const volatile uint32_t *)(self + 0x20), r38_bits = *(const volatile uint32_t *)(self + 0x38);
    const long double c = x87_cos(angle), s = x87_sin(angle);
    const long double a10 = f[0x10 / 4];
    f[0x10 / 4] = (float)(c * a10 + s * f[0x14 / 4]);
    f[0x14 / 4] = (float)(c * f[0x14 / 4] - a10 * s);
    f[0x20 / 4] = (float)(c * f[0x20 / 4] + s * f[0x24 / 4]);
    f[0x24 / 4] = (float)(c * f[0x24 / 4] - (long double)*(const float *)&r20_bits * s);
    const uint32_t p54_bits = *(const volatile uint32_t *)(self + 0x54);
    const long double p50_again = f[0x50 / 4];
    *(volatile uint32_t *)(self + 0x38) = r38_bits;
    const volatile float d0 = (float)(p50 - p50_again);
    const long double d1 = p54 - (long double)*(const float *)&p54_bits;
    f[0x30 / 4] = (float)((long double)d0 + f[0x30 / 4]);
    f[0x34 / 4] = (float)(d1 + f[0x34 / 4]);
}
static const uint8_t frame_turn_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Frame::Turn", 0xbd4c90, frame_turn_original, &frame_turn, &g_frame_turns);

// ---- Matrix::InverseTransformPoint(m, out, p) (0x47D7D0): a point taken into a frame -----------------------------------
// d = p - the frame's position (+0x30; d.x and d.y rounded, d.z rounded and kept unrounded); out[j] = ((d.z m[j][2] +
// d.y m[j][1]) + d.x m[j][0]) + m[j][3] with rows of four (out[2] takes d.z unrounded).
static volatile long g_inverse_points;
static void __thiscall matrix_inverse_transform_point(const float *m, float *out, const float *p) {
    g_inverse_points++;
    const volatile float dx = (float)((long double)p[0] - m[12]), dy = (float)((long double)p[1] - m[13]);
    const long double dz = (long double)p[2] - m[14];
    const volatile float dz_stored = (float)dz;
    const long double r2 = ((dz * m[10] + (long double)dy * m[9]) + (long double)dx * m[8]) + m[11];
    const long double r1 = (((long double)dz_stored * m[6] + (long double)dy * m[5]) + (long double)dx * m[4]) + m[7];
    const long double r0 = (((long double)dz_stored * m[2] + (long double)dy * m[1]) + (long double)dx * m[0]) + m[3];
    out[0] = (float)r0;
    out[1] = (float)r1;
    out[2] = (float)r2;
}
static const uint8_t matrix_inverse_transform_point_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Matrix::InverseTransformPoint", 0x47d7d0, matrix_inverse_transform_point_original, &matrix_inverse_transform_point, &g_inverse_points);

// ---- Effect::Set7B2730(this, point, r) (0x7B2730): an effect at a point with a reach ----------------------------------
// +0 the point (copied as it is), +0x10 cleared, +0x20 the point less r, +0x30 r plus the point (each z first, rounded
// as stored), +0x40 1 / (r r), +0x44 cleared.
static volatile long g_effect_sets;
static void __thiscall effect_set(uint32_t *self, const float *point, float r) {
    g_effect_sets++;
    volatile uint32_t *const s = self;
    volatile float *const f = (volatile float *)self;
    s[0] = ((const uint32_t *)point)[0], s[1] = ((const uint32_t *)point)[1], s[2] = ((const uint32_t *)point)[2];
    s[4] = 0, s[5] = 0, s[6] = 0;
    const long double lo0 = (long double)point[0] - r, lo1 = (long double)point[1] - r;
    f[10] = (float)((long double)point[2] - r);
    f[8] = (float)lo0, f[9] = (float)lo1;
    const long double hi0 = (long double)r + point[0], hi1 = (long double)r + point[1];
    f[14] = (float)((long double)r + point[2]);
    f[12] = (float)hi0, f[13] = (float)hi1;
    s[0x44 / 4] = 0, s[0x48 / 4] = 0, s[0x4c / 4] = 0;
    f[0x40 / 4] = (float)((long double)*(const float *)0xf264dc / ((long double)r * r));
}
static const uint8_t effect_set_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Effect::Set7B2730", 0x7b2730, effect_set_original, &effect_set, &g_effect_sets);

// ---- Sound::SetVolumeScale(v) (0x8F8660): the current sound's (0x1339F58) volume scale ---------------------------------
// v kept within 0.2 and 1 (fcomi / fcmov: a NaN gives 1) into +0x14; then its voice (+0xC) gets volume (+0x58) 0 when
// the sound's channel (+0x18) is muted (0x12F0168), else the channel's volume (0x10EDD40) times the scale.
static volatile long g_volume_scales;
static void __stdcall sound_set_volume_scale(float v) {
    g_volume_scales++;
    uint8_t *const sound = *(uint8_t *const volatile *)0x1339f58;
    if (!sound)
        return;
    static const uint32_t one_bits = 0x3f800000, low_bits = 0x3e4ccccd;
    const volatile float t = (float)x87_lesser(x87_load(&v), x87_load((const float *)&one_bits));
    *(volatile float *)(sound + 0x14) = (float)x87_greater(x87_load((const float *)&t), x87_load((const float *)&low_bits));
    const uint32_t channel = *(const uint32_t *)(sound + 0x18);
    volatile float *const voice = (volatile float *)(*(uint8_t *const *)(sound + 0xc) + 0x58);
    if (*(const uint32_t *)(0x12f0168 + channel * 4))
        *voice = *(const float *)0xf264e0;
    else
        *voice = (float)((long double)*(const float *)(0x10edd40 + channel * 4) * *(volatile float *)(sound + 0x14));
}
static const uint8_t sound_set_volume_scale_original[] = {0xa1, 0x58, 0x9f, 0x33, 0x01, 0x83, 0xec, 0x08};
REWRITE("Sound::SetVolumeScale", 0x8f8660, sound_set_volume_scale_original, &sound_set_volume_scale, &g_volume_scales);

// ---- Tabs::Animate(this, time) (0x61C280): the tab offsets (+0xE4, count +0xE8) moved by the time -----------------------
// +0xF8 is set. With d = time times 50 (rounded): the selected tab (+0xE0) loses d; every other gains 2d and is kept at
// most 0 (fcomi / fcmov: a NaN gives 0), each value stored and reloaded on the way.
static volatile long g_tab_animations;
static void __thiscall tabs_animate(uint8_t *self, float time) {
    g_tab_animations++;
    volatile uint32_t *const s = (volatile uint32_t *)self;
    ((volatile uint8_t *)self)[0xf8] = 1;
    if ((int32_t)s[0xe8 / 4] <= 0)
        return;
    const volatile float d = (float)((long double)time * *(const float *)0xf28210);
    static const uint32_t zero_bits = 0;
    for (int32_t i = 0; i < (int32_t)s[0xe8 / 4]; i++) {
        volatile float *const p = (volatile float *)(uintptr_t)s[0xe4 / 4] + i;
        if ((uint32_t)i == s[0xe0 / 4]) {
            *p = (float)((long double)*p - d);
            continue;
        }
        *p = (float)(((long double)d + d) + *p);
        const volatile float v = (float)x87_load((const float *)((volatile float *)(uintptr_t)s[0xe4 / 4] + i));
        const volatile float m = (float)x87_lesser(x87_load((const float *)&v), x87_load((const float *)&zero_bits));
        *((volatile float *)(uintptr_t)s[0xe4 / 4] + i) = (float)x87_load((const float *)&m);
    }
}
static const uint8_t tabs_animate_original[] = {0x8b, 0x91, 0xe8, 0x00, 0x00, 0x00, 0x83, 0xec};
REWRITE("Tabs::Animate", 0x61c280, tabs_animate_original, &tabs_animate, &g_tab_animations);

// ---- Speed::Scale(this, k) (0xA5FD90): a change applied to a speed factor (+0x20) ---------------------------------------
// Nothing when |k - 1| is under the game's small value (0xF27F4C). Otherwise a factor above 1 is multiplied by k, any
// other gains k - 1 (rounded); then it is kept at least 1 and at most 1000 (fcomi / fcmov, NaNs as they fall).
static volatile long g_speed_scales;
static void __thiscall speed_scale(uint8_t *self, float k) {
    g_speed_scales++;
    const long double one = *(const float *)0xf264dc;
    const long double d = (long double)k - one;
    const volatile float d_stored = (float)d;
    if (x87_abs(d) < (long double)*(const float *)0xf27f4c)
        return;
    volatile float *const f = (volatile float *)(self + 0x20);
    if ((long double)*f > one)
        *f = (float)((long double)k * *f);
    else
        *f = (float)((long double)d_stored + *f);
    static const uint32_t one_bits = 0x3f800000, top_bits = 0x447a0000;
    const volatile float t = (float)x87_greater(x87_load((const float *)f), x87_load((const float *)&one_bits));
    *f = (float)x87_lesser(x87_load((const float *)&top_bits), x87_load((const float *)&t));
}
static const uint8_t speed_scale_original[] = {0x83, 0xec, 0x08, 0xd9, 0x44, 0x24, 0x0c, 0xd8};
REWRITE("Speed::Scale", 0xa5fd90, speed_scale_original, &speed_scale, &g_speed_scales);

// ---- Blink::Due(this, once) (0x757EB0): whether a timer (+0xBC) has passed its period ----------------------------------
// The period from the table 0xFC35D8 by +0xB8. Not yet (or a NaN): 0. Passed: with once set, the period is taken off;
// otherwise all whole periods are ((t / period + the game's small value) rounded down, times the period). Gives 1.
static volatile long g_blinks;
static uint32_t __thiscall blink_due(uint8_t *self, uint32_t once) {
    g_blinks++;
    const volatile float period = (float)x87_load((const float *)(0xfc35d8 + *(const uint32_t *)(self + 0xb8) * 4));
    volatile float *const t = (volatile float *)(self + 0xbc);
    if (!((long double)*t >= (long double)period))
        return 0;
    if (once & 0xff) {
        *t = (float)((long double)*t - period);
        return 1;
    }
    const volatile float q = (float)((long double)*t / period + *(const float *)0xf27f4c);
    const long double whole = x87_round(q);
    const volatile float down = (float)(q < whole ? whole - 1.0L : whole);
    *t = (float)((long double)*t - (long double)down * period);
    return 1;
}
static const uint8_t blink_due_original[] = {0x83, 0xec, 0x08, 0x8b, 0x81, 0xb8, 0x00, 0x00};
REWRITE("Blink::Due", 0x757eb0, blink_due_original, &blink_due, &g_blinks);

// ---- Triangle::FromBase(out, base, direction, side) (0xC1C2E0): an arrow head's three points (2D) ---------------------
// p0 = (base + direction / 2) + 2 side (its x rounded first, y not), p1 = base + 1.5 direction, p2 = base - direction / 2.
static volatile long g_triangles;
static void __thiscall triangle_from_base(float *out, const float *base, const float *direction, const float *side) {
    g_triangles++;
    const long double half = *(const float *)0xf2651c, more = *(const float *)0xf26648;
    const long double s2x = (long double)side[0] + side[0], s2y = (long double)side[1] + side[1];
    const volatile float bx = (float)((long double)direction[0] * half + base[0]);
    const long double by = (long double)direction[1] * half + base[1];
    out[0] = (float)((long double)bx + s2x);
    out[1] = (float)(by + s2y);
    const long double p1y = (long double)direction[1] * more + base[1];
    out[2] = (float)((long double)direction[0] * more + base[0]);
    out[3] = (float)p1y;
    const long double hx = (long double)direction[0] * half;
    const long double p2y = (long double)base[1] - (long double)direction[1] * half;
    out[4] = (float)((long double)base[0] - hx);
    out[5] = (float)p2y;
}
static const uint8_t triangle_from_base_original[] = {0x83, 0xec, 0x08, 0x8b, 0x54, 0x24, 0x0c, 0x8b};
REWRITE("Triangle::FromBase", 0xc1c2e0, triangle_from_base_original, &triangle_from_base, &g_triangles);

// ---- Cursor::MoveByScreen(this, delta) (0x502520): a position (+0x20, +0x24) moved by a screen movement ---------------
// On first use (the game's guard bits at 0x12F506C and the byte at 0x12F5068) the scales 2 pi / the screen's width and
// height (0x12F83F0's +0x7C..+0x88) are kept at 0x12F5060 and 0x12F5064. Then x gains +0x30 dx (scale x), y gains
// dy +0x30 (scale y).
static volatile long g_cursor_moves;
static void __thiscall cursor_move_by_screen(uint8_t *self, const float *delta) {
    g_cursor_moves++;
    if (!(*(volatile uint8_t *)0x12f506c & 1))
        *(volatile uint32_t *)0x12f506c |= 1;
    if (!*(volatile uint8_t *)0x12f5068) {
        const float *const screen = *(const float *const volatile *)0x12f83f0;
        const long double w = (long double)screen[0x84 / 4] - screen[0x7c / 4], h = (long double)screen[0x88 / 4] - screen[0x80 / 4];
        *(volatile uint8_t *)0x12f5068 = 1;
        const long double turn = *(const float *)0xf268dc;
        *(volatile float *)0x12f5060 = (float)(turn / w);
        *(volatile float *)0x12f5064 = (float)(turn / h);
    }
    volatile float *const f = (volatile float *)self;
    const long double mx = (long double)f[0x30 / 4] * delta[0] * *(volatile float *)0x12f5060;
    const long double my = (long double)delta[1] * f[0x30 / 4] * *(volatile float *)0x12f5064;
    f[0x20 / 4] = (float)(mx + f[0x20 / 4]);
    f[0x24 / 4] = (float)(my + f[0x24 / 4]);
}
static const uint8_t cursor_move_by_screen_original[] = {0xa0, 0x6c, 0x50, 0x2f, 0x01, 0xba, 0x01, 0x00};
REWRITE("Cursor::MoveByScreen", 0x502520, cursor_move_by_screen_original, &cursor_move_by_screen, &g_cursor_moves);

// ---- Segment::MiddleOfEnds2(this, out) (0x5BCEE0): the point halfway between the last points of two lists -------------
// The lists at +0x2C and +0x28 (count +0, points of 0x24 bytes at +4): out = (a + b) / 2 with the x sum rounded first,
// y and z not.
static volatile long g_segment_middles;
static void __thiscall segment_middle_of_ends2(const uint8_t *self, float *out) {
    g_segment_middles++;
    const uint32_t *const la = *(const uint32_t *const *)(self + 0x2c), *const lb = *(const uint32_t *const *)(self + 0x28);
    const float *const a = (const float *)(uintptr_t)(la[1] + (la[0] * 9 - 9) * 4);
    const float *const b = (const float *)(uintptr_t)(lb[1] + (lb[0] * 9 - 9) * 4);
    const long double half = *(const float *)0xf2651c;
    const volatile float sx = (float)((long double)b[0] + a[0]);
    const long double sy = (long double)b[1] + a[1], sz = (long double)b[2] + a[2];
    out[0] = (float)((long double)sx * half);
    out[1] = (float)(sy * half);
    out[2] = (float)(sz * half);
}
static const uint8_t segment_middle_of_ends2_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Segment::MiddleOfEnds2", 0x5bcee0, segment_middle_of_ends2_original, &segment_middle_of_ends2, &g_segment_middles);

// ---- Frame::Flip(this) (0x50BD80): a frame flipped -----------------------------------------------------------------------
// +0x10 and the vectors at +0xD0 and +0xE0 negated (through the FPU: a signalling NaN comes out quiet; each vector's z
// written first), the flag byte at +0xC toggled (set when it was 0, else cleared).
static volatile long g_frame_flips;
static void __thiscall frame_flip(uint8_t *self) {
    g_frame_flips++;
    volatile float *const f = (volatile float *)self;
    f[0x10 / 4] = (float)-x87_load((const float *)(self + 0x10));
    ((volatile uint8_t *)self)[0xc] = self[0xc] == 0;
    for (int v = 0; v < 2; v++) {
        const int at = (0xd0 + 0x10 * v) / 4;
        const long double x = -x87_load((const float *)&f[at]), y = -x87_load((const float *)&f[at + 1]), z = -x87_load((const float *)&f[at + 2]);
        f[at + 2] = (float)z;
        f[at] = (float)x;
        f[at + 1] = (float)y;
    }
}
static const uint8_t frame_flip_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Frame::Flip", 0x50bd80, frame_flip_original, &frame_flip, &g_frame_flips);

// ---- Terrain::SlopeShade(this, x, y) (0x6B6400): a grey for a terrain cell from its height difference ------------------
// The cell (0x18 bytes each, at +0x24, rows of the width at +4): g = (cell[+0xC] - cell[+0]) k + c (0xF2737C,
// 0xF27844), rounded as stored, then rounded down to a whole number; below 0 gives 0, above 0x7F gives 0x7F7F7F, else
// g in all three channels.
static volatile long g_slope_shades;
static uint32_t __thiscall terrain_slope_shade(const uint8_t *self, uint32_t x, uint32_t y) {
    g_slope_shades++;
    const float *const cell = (const float *)(*(const uint8_t *const *)(self + 0x24) + ((uint32_t)self[4] * (y & 0xff) + (x & 0xff)) * 0x18);
    const volatile float g = (float)(((long double)cell[3] - cell[0]) * *(const float *)0xf2737c + *(const float *)0xf27844);
    const long double whole = x87_round(g);
    const volatile float down = (float)(g < whole ? whole - 1.0L : whole);
    const int32_t n = (int32_t)(uint32_t)x87_whole(down);
    if (n < 0) return 0;
    if (n > 0x7f) return 0x7f7f7f;
    return (uint32_t)((((n << 8) | n) << 8) | n);
}
static const uint8_t terrain_slope_shade_original[] = {0x83, 0xec, 0x08, 0x0f, 0xb6, 0x54, 0x24, 0x10};
REWRITE("Terrain::SlopeShade", 0x6b6400, terrain_slope_shade_original, &terrain_slope_shade, &g_slope_shades);

// ---- CameraAngles::Clamp2(this) (0xA5F530): one tilt (+0x20) kept within +-1.5677, the turn (+0x24) within a turn -----
// As CameraAngles::Clamp with a single tilt.
static void __thiscall camera_angles_clamp2(uint8_t *self) {
    g_camera_clamps++;
    static const uint32_t limit_bits = 0x3fc8a8ea, low_bits = 0xbfc8a8ea;
    float *const a = (float *)(self + 0x20);
    const volatile float t = (float)x87_lesser(x87_load((const float *)&limit_bits), x87_load(a));
    const volatile float c = (float)x87_greater(x87_load((const float *)&t), x87_load((const float *)&low_bits));
    float *const turn = (float *)(self + 0x24);
    const volatile float turns = (float)((long double)*(volatile float *)turn * (long double)*(const float *)0xf36ee4);
    *(volatile float *)a = c;
    const long double whole = x87_round(turns);
    const volatile float down = (float)(turns < whole ? whole - 1.0L : whole);
    *(volatile float *)turn = (float)((long double)*(volatile float *)turn - down * (long double)*(const float *)0xf268dc);
}
static const uint8_t camera_angles_clamp2_original[] = {0x83, 0xec, 0x0c, 0x8b, 0x41, 0x20, 0xc7, 0x44};
REWRITE("CameraAngles::Clamp2", 0xa5f530, camera_angles_clamp2_original, &camera_angles_clamp2, &g_camera_clamps);

// ---- UnitSquare::Rotate(x, y, rotation, x_out, y_out) (0x875160): a place in a unit square turned -----------------------
// 0 keeps (x, y), 1 gives (y, 1 - x), 2 (1 - x, 1 - y), 3 (1 - y, x); the values pass through the FPU (a signalling NaN
// comes out quiet). Any other rotation writes nothing.
static volatile long g_unit_rotations;
static void __stdcall unit_square_rotate(float x, float y, uint32_t rotation, float *x_out, float *y_out) {
    g_unit_rotations++;
    const long double one = *(const float *)0xf264dc, lx = x87_load(&x), ly = x87_load(&y);
    volatile float *const ox = x_out, *const oy = y_out;
    switch (rotation) {
    case 0: *ox = (float)lx, *oy = (float)ly; break;
    case 1: *ox = (float)ly, *oy = (float)(one - lx); break;
    case 2: *ox = (float)(one - lx), *oy = (float)(one - ly); break;
    case 3: *ox = (float)(one - ly), *oy = (float)lx; break;
    default: break;
    }
}
static const uint8_t unit_square_rotate_original[] = {0x8b, 0x44, 0x24, 0x0c, 0x83, 0xf8, 0x03, 0x77};
REWRITE("UnitSquare::Rotate", 0x875160, unit_square_rotate_original, &unit_square_rotate, &g_unit_rotations);

// ---- Window::OutsideEllipse(this, p) (0x63A920): whether a point lies outside the ellipse filling the window -----------
// With the window's half sizes (from +0x7C..+0x88; the half width rounded) and k = 1 - +0xD4: 1 when k k is at most
// (dy / hh)^2 + (dx / hw)^2 (dy rounded first), 0 otherwise (or for a NaN).
static volatile long g_ellipse_tests;
static uint32_t __thiscall window_outside_ellipse(const uint8_t *self, const float *p) {
    g_ellipse_tests++;
    const float *const f = (const float *)self;
    const long double half = *(const float *)0xf2651c;
    const volatile float hw = (float)(((long double)f[0x84 / 4] - f[0x7c / 4]) * half);
    const long double hh = ((long double)f[0x88 / 4] - f[0x80 / 4]) * half;
    const long double dx = (long double)p[0] - hw;
    const volatile float dy = (float)((long double)p[1] - hh);
    const long double nx = dx / hw, ny = (long double)dy / hh;
    const long double k = (long double)*(const float *)0xf264dc - f[0xd4 / 4];
    return k * k <= ny * ny + nx * nx;
}
static const uint8_t window_outside_ellipse_original[] = {0x83, 0xec, 0x10, 0xd9, 0x81, 0x84, 0x00, 0x00};
REWRITE("Window::OutsideEllipse", 0x63a920, window_outside_ellipse_original, &window_outside_ellipse, &g_ellipse_tests);

// ---- Window::ArcA45670(this, out, angle, r, h) (0xA45670): a point on a circle, placed and scaled for the window -------
// x = (cos(angle) r (rounded) + (734 - h / 2)) times (window width (+0x84 - +0x7C) / 1024), y = (sin(angle) r + (527 -
// h / 2 rounded)) times (height (+0x88 - +0x80) / 768, rounded).
static volatile long g_window_arcs;
static float *__thiscall window_arc(const uint8_t *self, float *out, float angle, float r, float h) {
    g_window_arcs++;
    const float *const f = (const float *)self;
    const long double hh = (long double)h * *(const float *)0xf2651c;
    const volatile float hh_stored = (float)hh;
    const long double a = (long double)*(const float *)0xf8db9c - hh;
    const long double b = (long double)*(const float *)0xf8db98 - hh_stored;
    const long double c = x87_cos(angle), s = x87_sin(angle);
    const volatile float x = (float)(c * r);
    const long double y = s * r;
    const long double xa = (long double)x + a;
    const long double sx = ((long double)f[0x84 / 4] - f[0x7c / 4]) * *(const float *)0xf2856c;
    const volatile float sy = (float)(((long double)f[0x88 / 4] - f[0x80 / 4]) * *(const float *)0xf28568);
    out[0] = (float)(sx * xa);
    out[1] = (float)((y + b) * sy);
    return out;
}
static const uint8_t window_arc_original[] = {0x83, 0xec, 0x08, 0xd9, 0x44, 0x24, 0x18, 0x8b};
REWRITE("Window::ArcA45670", 0xa45670, window_arc_original, &window_arc, &g_window_arcs);

// A linear fade kept within 0 and 1 as the game does it: below 0 (ordered) gives 0, above 1 (ordered) gives 1, anything
// else (a NaN too) itself, unrounded
static inline long double fade_kept(long double v) {
    if (v < (long double)*(const float *)0xf264e0) return *(const float *)0xf264e0;
    if (v > (long double)*(const float *)0xf264dc) return *(const float *)0xf264dc;
    return v;
}

// ---- Sound::DistanceFade(this) (0x47BD90): how loud a sound is from its distance to the listener ----------------------
// The sound's place (its voice, +0xC, at +0x60) less the listener's (0x10F4620): d = sqrt((z z + y y) + x x); 1 - (d -
// near) / (far - near) (+0x80, +0x84) kept within 0 and 1, given back unrounded on the FPU.
static volatile long g_distance_fades;
static long double __thiscall sound_distance_fade(const uint8_t *self) {
    g_distance_fades++;
    const float *const v = (const float *)(*(const uint8_t *const *)(self + 0xc) + 0x60), *const listener = (const float *)0x10f4620;
    const long double x = (long double)v[0] - listener[0], y = (long double)v[1] - listener[1], z = (long double)v[2] - listener[2];
    const long double d = x87_sqrt((z * z + y * y) + x * x);
    const float inner = v[0x20 / 4], outer = v[0x24 / 4];
    return fade_kept((long double)*(const float *)0xf264dc - (d - inner) / ((long double)outer - inner));
}
static const uint8_t sound_distance_fade_original[] = {0x8b, 0x41, 0x0c, 0xd9, 0x40, 0x60, 0x83, 0xc0};
REWRITE("Sound::DistanceFade", 0x47bd90, sound_distance_fade_original, &sound_distance_fade, &g_distance_fades);

// ---- Light::FadeAlpha(this, out, x, y) (0x959F30): an alpha for a point from its distance to a light ------------------
// d = sqrt(dy dy + dx dx) from the light's centre (+0x3C, +0x40); (1 - (d - near) / (far - near)) times the strength
// (+0x38), kept within 0 and 1, times 255 (fistp), in the top byte of *out (the rest 0).
static volatile long g_light_fades;
static uint32_t *__thiscall light_fade_alpha(const uint8_t *self, uint32_t *out, float x, float y) {
    g_light_fades++;
    const float *const f = (const float *)self;
    const long double dx = (long double)x - f[0x3c / 4], dy = (long double)y - f[0x40 / 4];
    const long double d = x87_sqrt(dy * dy + dx * dx);
    const long double v = fade_kept(((long double)*(const float *)0xf264dc - (d - f[0x44 / 4]) / ((long double)f[0x48 / 4] - f[0x44 / 4])) * f[0x38 / 4]);
    *(volatile uint32_t *)out = (uint32_t)x87_whole((long double)*(const float *)0xf264c0 * v) << 24;
    return out;
}
static const uint8_t light_fade_alpha_original[] = {0xd9, 0x44, 0x24, 0x08, 0x83, 0xec, 0x08, 0xd8};
REWRITE("Light::FadeAlpha", 0x959f30, light_fade_alpha_original, &light_fade_alpha, &g_light_fades);

// ---- Slider::SetValue(this, fraction) (0xCA6F90): the bound value (*(+4)) from a fraction -----------------------------
// v = (max - min) fraction + min (+0xC, +8; rounded), kept at least min and at most max (fcomi / fcmov: a NaN gives min),
// then rounded down to whole steps (+0x10): (v / step rounded down) step.
static volatile long g_slider_sets;
static void __thiscall slider_set_value(const uint8_t *self, float fraction) {
    g_slider_sets++;
    const float *const f = (const float *)self;
    const volatile float v = (float)(((long double)f[3] - f[2]) * fraction + f[2]);
    const volatile float t = (float)x87_greater(x87_load(&f[2]), x87_load((const float *)&v));
    const volatile float u = (float)x87_lesser(x87_load(&f[3]), x87_load((const float *)&t));
    const volatile float q = (float)((long double)u / f[4]);
    const long double whole = x87_round(q);
    const volatile float down = (float)(q < whole ? whole - 1.0L : whole);
    **(volatile float *const *)(self + 4) = (float)((long double)down * f[4]);
}
static const uint8_t slider_set_value_original[] = {0x83, 0xec, 0x08, 0xd9, 0x41, 0x0c, 0x8b, 0x41};
REWRITE("Slider::SetValue", 0xca6f90, slider_set_value_original, &slider_set_value, &g_slider_sets);

// ---- Matrix::Copy3x4(from, to) (0xB1F750): the first twelve values of a matrix copied ---------------------------------
// Each row's first two go through the FPU (a signalling NaN comes out quiet), the other two are copied as they are.
static volatile long g_matrix_copies;
static void __fastcall matrix_copy3x4(const uint32_t *from, uint32_t *to) {
    g_matrix_copies++;
    for (int row = 0; row < 3; row++) {
        const uint32_t *const r = from + 4 * row;
        const long double a = x87_load((const float *)&r[0]), b = x87_load((const float *)&r[1]);
        const uint32_t c = r[2], d = r[3];
        ((volatile float *)to)[4 * row] = (float)a;
        ((volatile float *)to)[4 * row + 1] = (float)b;
        ((volatile uint32_t *)to)[4 * row + 2] = c;
        ((volatile uint32_t *)to)[4 * row + 3] = d;
    }
}
static const uint8_t matrix_copy3x4_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Matrix::Copy3x4", 0xb1f750, matrix_copy3x4_original, &matrix_copy3x4, &g_matrix_copies);

// log10 as the game takes it: fldlg2, then fyl2x
static inline long double x87_log10(long double v) {
    __asm__("fldlg2\n\tfxch %%st(1)\n\tfyl2x" : "+t"(v));
    return v;
}

// ---- Curve::LogBlend(a, b, x) (0xCE4920): a value between two points' values at x, eased on a log scale ---------------
// Points {value, position}. r = (x - the smaller position (b's unless a's is smaller)) 9 / |b.pos - a.pos|. When a's
// position is the smaller: -log10(1 - r / 10); otherwise 1 - log10(r + 1); either times (b.value - a.value) plus
// a.value, given back unrounded on the FPU.
static volatile long g_log_blends;
static long double __fastcall curve_log_blend(const float *a, const float *b, float x) {
    g_log_blends++;
    const bool a_first = (long double)a[1] < (long double)b[1];
    const long double low = a_first ? a[1] : b[1];
    const long double r = (((long double)x - low) * *(const float *)0xf27848) / x87_abs((long double)b[1] - a[1]);
    const long double one = *(const float *)0xf264dc;
    long double eased;
    if (a_first)
        eased = -x87_log10(one - r * *(const float *)0xf270b0);
    else
        eased = one - x87_log10(r + one);
    return eased * ((long double)b[0] - a[0]) + a[0];
}
static const uint8_t curve_log_blend_original[] = {0xd9, 0x41, 0x04, 0xd8, 0x5a, 0x04, 0xdf, 0xe0};
REWRITE("Curve::LogBlend", 0xce4920, curve_log_blend_original, &curve_log_blend, &g_log_blends);

// ---- Columns::Hit(this, n, p) (0x884890): whether a point falls in column n of a list -----------------------------------
// v = (p.x - 4) / ((+0x84 - +0x7C) - 8) times the scale (+0xDC), w = scale / ((+0x84 - +0x7C) - 8) times 3: 1 when
// v - w <= column n (the floats at +0xFC) <= v + w, else (or for a NaN) 0.
static volatile long g_column_hits;
static uint32_t __thiscall columns_hit(const uint8_t *self, int32_t n, const float *p) {
    g_column_hits++;
    const float *const f = (const float *)self;
    const long double span = ((long double)f[0x84 / 4] - f[0x7c / 4]) - *(const float *)0xf270e8;
    const long double v = ((long double)p[0] - *(const float *)0xf793c8) / span * f[0xdc / 4];
    const long double w = (long double)f[0xdc / 4] / (((long double)f[0x84 / 4] - f[0x7c / 4]) - *(const float *)0xf270e8) * *(const float *)0xf793c4;
    const long double column = (*(const float *const *)(self + 0xfc))[n];
    return v - w <= column && column <= w + v;
}
static const uint8_t columns_hit_original[] = {0x8b, 0x44, 0x24, 0x08, 0xd9, 0x00, 0x8b, 0x54};
REWRITE("Columns::Hit", 0x884890, columns_hit_original, &columns_hit, &g_column_hits);

// ---- Vector::NormaliseFast(v) (0x94CA00): a vector made one long with the fast inverse square root -----------------------
// q = (x x + y y) + z z (rounded); nothing when q is at most the game's small value (0xF3312C) or a NaN; else
// y0 = the float with bits 0x5F3759DF - (q's bits >> 1), r = (1.5 - y0 y0 q 0.5) y0 (one Newton step, unrounded), and
// each part times r.
static volatile long g_fast_normals;
static void __thiscall vector_normalise_fast(float *v) {
    g_fast_normals++;
    const volatile float q = (float)(((long double)v[0] * v[0] + (long double)v[1] * v[1]) + (long double)v[2] * v[2]);
    if (!((long double)q > (long double)*(const float *)0xf3312c))
        return;
    const uint32_t bits = 0x5f3759df - (uint32_t)(*(const volatile int32_t *)&q >> 1);
    const volatile float y0 = *(const float *)&bits;
    const long double r = ((long double)*(const float *)0xf26648 - (long double)y0 * y0 * q * *(const float *)0xf2651c) * y0;
    volatile float *const o = v;
    o[0] = (float)(r * o[0]);
    o[1] = (float)(r * o[1]);
    o[2] = (float)(r * o[2]);
}
static const uint8_t vector_normalise_fast_original[] = {0x83, 0xec, 0x08, 0xd9, 0x41, 0x08, 0xd9, 0x41};
REWRITE("Vector::NormaliseFast", 0x94ca00, vector_normalise_fast_original, &vector_normalise_fast, &g_fast_normals);

// ---- Ease::Out(t, total) (0x5D3970): a time eased over a total ---------------------------------------------------------
// A total under 2 gives t back. Within the last 2 (total - 2 < t): ((total - 1) - (total - t)^2 / 4) / (total - 1)
// total; otherwise t / (total - 1) total; unrounded on the FPU.
static volatile long g_eases;
static long double __stdcall ease_out(float t, float total) {
    g_eases++;
    const long double two = *(const float *)0xf2705c, one = *(const float *)0xf264dc;
    if ((long double)total < two)
        return t;
    if ((long double)total - two < (long double)t) {
        const long double a = (long double)total - t;
        return (((long double)total - one) - a * a * *(const float *)0xf270b8) / ((long double)total - one) * total;
    }
    return (long double)t / ((long double)total - one) * total;
}
static const uint8_t ease_out_original[] = {0xd9, 0x44, 0x24, 0x08, 0xd8, 0x1d, 0x5c, 0x70};
REWRITE("Ease::Out", 0x5d3970, ease_out_original, &ease_out, &g_eases);

// ---- Colour::FromARGB(out, colour) (0x454850 and 0x435A40, the same code): 0xAARRGGBB as red, green, blue, alpha ------
// Each byte times the game's 1/255 (0xF270F0); gives out back.
static volatile long g_colour_unpacks;
static float *__thiscall colour_from_argb(float *out, int32_t colour) {
    g_colour_unpacks++;
    const long double k = *(const float *)0xf270f0;
    out[0] = (float)((long double)((colour >> 16) & 0xff) * k);
    out[1] = (float)((long double)((colour >> 8) & 0xff) * k);
    out[2] = (float)((long double)(colour & 0xff) * k);
    out[3] = (float)((long double)((colour >> 24) & 0xff) * k);
    return out;
}
static const uint8_t colour_from_argb_original[] = {0x8b, 0x54, 0x24, 0x04, 0x8b, 0xc1, 0x8b, 0xca};
REWRITE("Colour::FromARGB", 0x454850, colour_from_argb_original, &colour_from_argb, &g_colour_unpacks);
REWRITE("Colour::FromARGB", 0x435a40, colour_from_argb_original, &colour_from_argb, &g_colour_unpacks);

// ---- Segment::Middle2(this, out) (0x5BCE60): the point halfway between the first points of the parts at +0x2C and +0x28
// (each part's +4); the x sum rounded first, y and z not ----------------------------------------------------------------
static void __thiscall segment_middle2(const uint8_t *self, float *out) {
    g_segment_middles++;
    const float *const a = *(const float *const *)(*(const uint8_t *const *)(self + 0x2c) + 4);
    const float *const b = *(const float *const *)(*(const uint8_t *const *)(self + 0x28) + 4);
    const long double half = *(const float *)0xf2651c;
    const volatile float sx = (float)((long double)b[0] + a[0]);
    const long double sy = (long double)b[1] + a[1], sz = (long double)b[2] + a[2];
    out[0] = (float)((long double)sx * half);
    out[1] = (float)(sy * half);
    out[2] = (float)(sz * half);
}
static const uint8_t segment_middle2_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Segment::Middle2", 0x5bce60, segment_middle2_original, &segment_middle2, &g_segment_middles);

// ---- Colour::Blend(this, t, other) (0x5BC0B0): four whole numbers blended toward another four by t ----------------------
// Each becomes this (1 - t) + other t, rounded by fistp (low 32 bits).
static volatile long g_colour_blends;
static void __thiscall colour_blend(int32_t *self, float t, const int32_t *other) {
    g_colour_blends++;
    const long double s = (long double)*(const float *)0xf264dc - t;
    for (int i = 0; i < 4; i++) self[i] = (int32_t)(uint32_t)x87_whole((long double)self[i] * s + (long double)other[i] * t);
}
static const uint8_t colour_blend_original[] = {0x83, 0xec, 0x08, 0xd9, 0x05, 0xdc, 0x64, 0xf2};
REWRITE("Colour::Blend", 0x5bc0b0, colour_blend_original, &colour_blend, &g_colour_blends);

// ---- Curve::LinearBlend(a, b, x) (0xCE47B0): a value between two points' values at x, in a straight line --------------
// Points {position, value}. f = (x - a.pos) / (b.pos - a.pos) (rounded), or 1 - f when a.value is not below b.value;
// gives |b.value - a.value| f + the smaller value (fcomi / fcmov), unrounded on the FPU.
static volatile long g_linear_blends;
static long double __fastcall curve_linear_blend(const float *a, const float *b, float x) {
    g_linear_blends++;
    const long double f = ((long double)x - a[0]) / ((long double)b[0] - a[0]);
    const volatile float g = (long double)a[1] < (long double)b[1] ? (float)f : (float)((long double)*(const float *)0xf264dc - f);
    const volatile float low = (float)x87_lesser(x87_load(&a[1]), x87_load(&b[1]));
    return x87_abs((long double)b[1] - a[1]) * g + low;
}
static const uint8_t curve_linear_blend_original[] = {0xd9, 0x44, 0x24, 0x04, 0x83, 0xec, 0x14, 0xd8};
REWRITE("Curve::LinearBlend", 0xce47b0, curve_linear_blend_original, &curve_linear_blend, &g_linear_blends);

// ---- Average::AddCell(this, cell) (0x7C32F0): a cell's place (bytes +0x88, +0x89) added to a running average ----------
// With n = count (+0x1D0) + 1 and i = 1 / n (unrounded): each average (+0x1D4, +0x1D8) becomes value i + old count i
// average; the count becomes n.
static volatile long g_average_adds;
static void __thiscall average_add_cell(uint8_t *self, const uint8_t *cell) {
    g_average_adds++;
    volatile float *const f = (volatile float *)self;
    const long double n = (long double)f[0x1d0 / 4] + *(const float *)0xf264dc;
    const long double i = (long double)*(const float *)0xf264dc / n;
    const long double kept = i * f[0x1d0 / 4];
    f[0x1d0 / 4] = (float)n;
    f[0x1d4 / 4] = (float)((long double)cell[0x88] * i + kept * f[0x1d4 / 4]);
    f[0x1d8 / 4] = (float)((long double)cell[0x89] * i + kept * f[0x1d8 / 4]);
}
static const uint8_t average_add_cell_original[] = {0xd9, 0x81, 0xd0, 0x01, 0x00, 0x00, 0x8b, 0x44};
REWRITE("Average::AddCell", 0x7c32f0, average_add_cell_original, &average_add_cell, &g_average_adds);

// ---- IntSlider::SetValue(this, fraction) (0xCA77B0): the bound whole number (*(+4)) from a fraction ---------------------
// v = (max - min, taken unsigned: 2^32 added when it is negative) fraction (fistp, low 32 bits) + min (+8, +0xC); kept
// at least min and at most max, compared unsigned.
static volatile long g_int_slider_sets;
static void __thiscall int_slider_set_value(const uint8_t *self, float fraction) {
    g_int_slider_sets++;
    const uint32_t min = *(const uint32_t *)(self + 8), max = *(const uint32_t *)(self + 0xc);
    const int32_t range = (int32_t)(max - min);
    long double span = (long double)range;
    if (range < 0) span += *(const float *)0xf285f8;
    const uint32_t v = (uint32_t)x87_whole(span * fraction) + min;
    const uint32_t t = min < v ? v : min;
    **(volatile uint32_t *const *)(self + 4) = max > t ? t : max;
}
static const uint8_t int_slider_set_value_original[] = {0x83, 0xec, 0x0c, 0x8b, 0x41, 0x08, 0x8b, 0x51};
REWRITE("IntSlider::SetValue", 0xca77b0, int_slider_set_value_original, &int_slider_set_value, &g_int_slider_sets);

// ---- IntSlider::SetValue2(this, fraction) (0xCA7620): the same, signed ------------------------------------------------
// v = (max - min) fraction + min (fistp, low 32 bits), kept at least min and at most max, compared signed.
static volatile long g_int_slider_sets2;
static void __thiscall int_slider_set_value2(const uint8_t *self, float fraction) {
    g_int_slider_sets2++;
    const int32_t min = *(const int32_t *)(self + 8), max = *(const int32_t *)(self + 0xc);
    const int32_t v = (int32_t)(uint32_t)x87_whole((long double)(int32_t)((uint32_t)max - (uint32_t)min) * fraction + (long double)min);
    const int32_t t = min < v ? v : min;
    **(volatile int32_t *const *)(self + 4) = max > t ? t : max;
}
static const uint8_t int_slider_set_value2_original[] = {0x83, 0xec, 0x0c, 0x8b, 0x41, 0x08, 0x8b, 0x51};
REWRITE("IntSlider::SetValue2", 0xca7620, int_slider_set_value2_original, &int_slider_set_value2, &g_int_slider_sets2);

// A float rounded down as the game does it (frndint, then one less when that went up)
static inline float x87_floor_float(float q) {
    const long double whole = x87_round(q);
    const volatile float down = (float)((long double)q < whole ? whole - 1.0L : whole);
    return down;
}

// ---- Vector2::SnapToGrid(out, p, spacing) (0x617290): a 2D point snapped down to a grid --------------------------------
// i = 1 / spacing (rounded); each coordinate becomes floor(i value (rounded)) spacing; y is worked out first.
static float *__fastcall vector2_snap_to_grid(float *out, const float *p, float spacing) {
    g_grid_snaps++;
    const volatile float inv = (float)((long double)*(const float *)0xf264dc / spacing);
    const volatile float qy = (float)((long double)inv * p[1]);
    const float dy = x87_floor_float(qy);
    const volatile float qx = (float)((long double)inv * p[0]);
    const float dx = x87_floor_float(qx);
    ((volatile float *)out)[0] = (float)((long double)dx * spacing);
    ((volatile float *)out)[1] = (float)((long double)dy * spacing);
    return out;
}
static const uint8_t vector2_snap_to_grid_original[] = {0x83, 0xec, 0x0c, 0xd9, 0x05, 0xdc, 0x64, 0xf2};
REWRITE("Vector2::SnapToGrid", 0x617290, vector2_snap_to_grid_original, &vector2_snap_to_grid, &g_grid_snaps);

// ---- Vector::LerpPoint(out, a, b, t) (0x4259F0): the point between two points by t, with 1 as its fourth value ---------
// Each part (1 - t) a + t b (1 - t unrounded); all three worked out before out is written.
static volatile long g_lerp_points;
static void __fastcall vector_lerp_point(float *out, const float *a, const float *b, float t) {
    g_lerp_points++;
    const long double s = (long double)*(const float *)0xf264dc - t;
    const volatile float x = (float)(s * a[0] + (long double)t * b[0]);
    const volatile float y = (float)(s * a[1] + (long double)t * b[1]);
    const volatile float z = (float)(s * a[2] + (long double)t * b[2]);
    volatile uint32_t *const o = (volatile uint32_t *)out;
    o[0] = *(const volatile uint32_t *)&x;
    o[1] = *(const volatile uint32_t *)&y;
    o[2] = *(const volatile uint32_t *)&z;
    o[3] = 0x3f800000;
}
static const uint8_t vector_lerp_point_original[] = {0x83, 0xec, 0x10, 0xd9, 0x05, 0xdc, 0x64, 0xf2};
REWRITE("Vector::LerpPoint", 0x4259f0, vector_lerp_point_original, &vector_lerp_point, &g_lerp_points);

// ---- Sound::SetLevels(this, a, b) (0x47DD10): two levels (+0x5C, +0x60), each kept between 0 and 1 ---------------------
// fcomi / fcmov: a NaN level stays itself (quiet).
static volatile long g_sound_levels;
static void __thiscall sound_set_levels(uint8_t *self, float a, float b) {
    g_sound_levels++;
    *(volatile float *)(self + 0x5c) = (float)x87_lesser(x87_greater(x87_load(&a), 0.0L), 1.0L);
    *(volatile float *)(self + 0x60) = (float)x87_lesser(x87_greater(x87_load(&b), 0.0L), 1.0L);
}
static const uint8_t sound_set_levels_original[] = {0x83, 0xec, 0x0c, 0xc7, 0x04, 0x24, 0x00, 0x00};
REWRITE("Sound::SetLevels", 0x47dd10, sound_set_levels_original, &sound_set_levels, &g_sound_levels);

// ---- Matrix::ScaleOffset2D(this, origin, scale) (0x4668D0): a matrix scaling x and y and moving by -origin scale --------
// All cleared; +0 and +0x24 the scales as they are; +0x30, +0x34 -(origin scale); +0x3C 1.
static volatile long g_scale_offsets;
static void __fastcall matrix_scale_offset_2d(uint32_t *m, const float *origin, const float *scale) {
    g_scale_offsets++;
    volatile uint32_t *const t = m;
    for (int i = 0; i < 16; i++) t[i] = 0;
    t[0] = *(const uint32_t *)&scale[0];
    t[9] = *(const uint32_t *)&scale[1];
    ((volatile float *)m)[12] = (float)-((long double)origin[0] * scale[0]);
    t[15] = 0x3f800000;
    ((volatile float *)m)[13] = (float)-((long double)origin[1] * scale[1]);
}
static const uint8_t matrix_scale_offset_2d_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Matrix::ScaleOffset2D", 0x4668d0, matrix_scale_offset_2d_original, &matrix_scale_offset_2d, &g_scale_offsets);

// ---- VerletPoint::SetVelocity(this, v) (0xA6B2A0): the previous position (+0x20) set to the position less v dt ---------
// The position (+0x10) is copied first; then each part less v dt (the time step 0xF8F954; x's product rounded, y's and
// z's not).
static volatile long g_verlet_velocity_sets;
static void __thiscall verlet_point_set_velocity(uint8_t *self, const float *v) {
    g_verlet_velocity_sets++;
    volatile uint32_t *const bits = (volatile uint32_t *)self;
    bits[8] = bits[4], bits[9] = bits[5], bits[10] = bits[6];
    const long double dt = *(const float *)0xf8f954;
    const volatile float px = (float)((long double)v[0] * dt);
    const long double py = (long double)v[1] * dt, pz = (long double)v[2] * dt;
    volatile float *const prev = (volatile float *)(self + 0x20);
    prev[0] = (float)((long double)prev[0] - px);
    prev[1] = (float)((long double)prev[1] - py);
    prev[2] = (float)((long double)prev[2] - pz);
}
static const uint8_t verlet_point_set_velocity_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("VerletPoint::SetVelocity", 0xa6b2a0, verlet_point_set_velocity_original, &verlet_point_set_velocity, &g_verlet_velocity_sets);

// ---- Vector::TransformPointInPlace(p, m) (0x4BB400): a point times a 4x4 matrix in place, translation included --------
// x = ((m4 y + m8 z) + x m0) + m12, y = ((m5 y + m1 x) + m9 z) + m13, z = ((m6 y + m2 x) + m10 z) + m14; written
// after all three are worked out.
static volatile long g_in_place_transforms;
static void __fastcall vector_transform_point_in_place(float *p, const float *m) {
    g_in_place_transforms++;
    const long double x = p[0], y = p[1], z = p[2];
    const long double nx = ((m[4] * y + m[8] * z) + x * m[0]) + m[12];
    const long double ny = ((m[5] * y + m[1] * x) + m[9] * z) + m[13];
    const long double nz = ((m[6] * y + m[2] * x) + m[10] * z) + m[14];
    volatile float *const o = p;
    o[2] = (float)nz;
    o[0] = (float)nx;
    o[1] = (float)ny;
}
static const uint8_t vector_transform_point_in_place_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Vector::TransformPointInPlace", 0x4bb400, vector_transform_point_in_place_original, &vector_transform_point_in_place, &g_in_place_transforms);

// ---- Vector::Lerp(out, a, b, t) (0xCE16B0, and Vector::Lerp2 0x466A60, the same code): a + (b - a) t -------------------
// The x difference is rounded, the y and z ones not; the x and y products are rounded, z's not; written z, x, y.
static volatile long g_lerps;
static float *__fastcall vector_lerp(float *out, const float *a, const float *b, float t) {
    g_lerps++;
    const volatile float dx = (float)((long double)b[0] - a[0]);
    const long double dy = (long double)b[1] - a[1], dz = (long double)b[2] - a[2];
    const volatile float px = (float)((long double)dx * t), py = (float)(dy * t);
    const long double pz = dz * t;
    const long double x = (long double)px + a[0], y = (long double)py + a[1], z = pz + a[2];
    volatile float *const o = out;
    o[2] = (float)z;
    o[0] = (float)x;
    o[1] = (float)y;
    return out;
}
static const uint8_t vector_lerp_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Vector::Lerp", 0xce16b0, vector_lerp_original, &vector_lerp, &g_lerps);
REWRITE("Vector::Lerp2", 0x466a60, vector_lerp_original, &vector_lerp, &g_lerps);

// ---- Vector::LerpMarked(out, a, b, t) (0xA67E40): a + (b - a) t, with out's fifth value set to all ones (a NaN) --------
// The z difference is rounded, x and y not; the x and y products are rounded, z's not; written +0x10, z, x, y.
static volatile long g_marked_lerps;
static void __thiscall vector_lerp_marked(float *out, const float *a, const float *b, float t) {
    g_marked_lerps++;
    const long double dx = (long double)b[0] - a[0], dy = (long double)b[1] - a[1];
    const volatile float dz = (float)((long double)b[2] - a[2]);
    const volatile float px = (float)(dx * t), py = (float)(dy * t);
    const long double pz = (long double)dz * t;
    const long double x = (long double)px + a[0], y = (long double)py + a[1], z = pz + a[2];
    volatile float *const o = out;
    ((volatile uint32_t *)out)[4] = 0xffffffff;
    o[2] = (float)z;
    o[0] = (float)x;
    o[1] = (float)y;
}
static const uint8_t vector_lerp_marked_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Vector::LerpMarked", 0xa67e40, vector_lerp_marked_original, &vector_lerp_marked, &g_marked_lerps);

// ---- VerletPoint::Interpolated(this, out) (0xA67EE0): between the previous (+0x20) and current (+0x10) positions --------
// By the global fraction 0x13429E0: previous + (current - previous) f; the z difference rounded, x and y not; the x and
// y products rounded, z's not.
static volatile long g_verlet_interpolations;
static void __thiscall verlet_point_interpolated(const uint8_t *self, float *out) {
    g_verlet_interpolations++;
    const float *const p = (const float *)self;
    const long double f = *(const float *)0x13429e0;
    const long double dx = (long double)p[4] - p[8], dy = (long double)p[5] - p[9];
    const volatile float dz = (float)((long double)p[6] - p[10]);
    const volatile float ax = (float)(f * dx), ay = (float)(f * dy);
    const long double z = f * dz + p[10], y = (long double)ay + p[9], x = (long double)ax + p[8];
    volatile float *const o = out;
    o[0] = (float)x;
    o[1] = (float)y;
    o[2] = (float)z;
}
static const uint8_t verlet_point_interpolated_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("VerletPoint::Interpolated", 0xa67ee0, verlet_point_interpolated_original, &verlet_point_interpolated, &g_verlet_interpolations);

// ---- Motion::DistanceAt(t, v0, v1, T) (0xA88680): the distance covered by whole time t ---------------------------------
// Speeding steadily from v0 to v1 over T, then going on at v1: with s = the lesser of t and T (fcomi / fcmov),
// (v1 - v0) s s / (T + T) + s v0, plus (t - T) v1 when t is past T; unrounded on the FPU.
static volatile long g_motion_distances;
static long double __stdcall motion_distance_at(int32_t t, float v0, float v1, float T) {
    g_motion_distances++;
    const volatile float tf = (float)t;
    const volatile float s = (float)x87_lesser(tf, x87_load(&T));
    long double d = ((long double)v1 - v0) * s * s / ((long double)T + T) + (long double)s * v0;
    if ((long double)tf > (long double)T)
        d = ((long double)tf - T) * v1 + d;
    return d;
}
static const uint8_t motion_distance_at_original[] = {0x83, 0xec, 0x08, 0xdb, 0x44, 0x24, 0x0c, 0xd9};
REWRITE("Motion::DistanceAt", 0xa88680, motion_distance_at_original, &motion_distance_at, &g_motion_distances);

// ---- Terrain::CornerPosition(this, out, x, y, corner) (0x8C0100): the world position of a terrain cell's corner --------
// Bytes x and y, a signed byte corner: out.x = (x + (corner & 1)) width (+0x14) + origin x (+0xC); out.y the height as
// it is, the cell data (+0x24) at ((row length (byte +4) y + x) 6 + corner); out.z = (y + ((corner >> 1) & 1)) depth
// (+0x18) + origin z (+0x10).
static volatile long g_terrain_corners;
static void __thiscall terrain_corner_position(const uint8_t *self, float *out, uint32_t x_arg, uint32_t y_arg, uint32_t corner_arg) {
    g_terrain_corners++;
    const int32_t corner = (int8_t)corner_arg, x = (uint8_t)x_arg, y = (uint8_t)y_arg;
    const float *const f = (const float *)self;
    volatile uint32_t *const o = (volatile uint32_t *)out;
    ((volatile float *)out)[0] = (float)((long double)((corner & 1) + x) * f[0x14 / 4] + f[0xc / 4]);
    o[1] = (*(const uint32_t *const *)(self + 0x24))[(uint32_t)(((int32_t)self[4] * y + x) * 6 + corner)];
    ((volatile float *)out)[2] = (float)((long double)(((corner >> 1) & 1) + y) * f[0x18 / 4] + f[0x10 / 4]);
}
static const uint8_t terrain_corner_position_original[] = {0x0f, 0xbe, 0x44, 0x24, 0x10, 0x53, 0x0f, 0xb6};
REWRITE("Terrain::CornerPosition", 0x8c0100, terrain_corner_position_original, &terrain_corner_position, &g_terrain_corners);

// ---- Ease::In(t, total) (0x5D3900): a time eased over a total --------------------------------------------------------
// A total under 2 gives t back. t under 2: t t / 4 / (total - 1) total; otherwise (t - 1) / (total - 1) total; unrounded.
static long double __stdcall ease_in(float t, float total) {
    g_eases++;
    const long double two = *(const float *)0xf2705c, one = *(const float *)0xf264dc;
    if ((long double)total < two)
        return t;
    if ((long double)t < two)
        return (long double)t * t * *(const float *)0xf270b8 / ((long double)total - one) * total;
    return ((long double)t - one) / ((long double)total - one) * total;
}
static const uint8_t ease_in_original[] = {0xd9, 0x44, 0x24, 0x08, 0xd8, 0x1d, 0x5c, 0x70};
REWRITE("Ease::In", 0x5d3900, ease_in_original, &ease_in, &g_eases);

// ---- Colour::Pack(this, c) (0x9E84E0): a colour of four values from 0 to 1 packed at +8 --------------------------------
// Each times 255 (fistp, low 32 bits, not masked), joined alpha, red, green, blue: ((a << 8 | r) << 8 | g) << 8 | b.
static volatile long g_colour_packs;
static void __thiscall colour_pack(uint8_t *self, const float *c) {
    g_colour_packs++;
    const long double k = *(const float *)0xf264c0;
    uint32_t v = (uint32_t)x87_whole(c[3] * k) << 8;
    v = (v | (uint32_t)x87_whole(c[0] * k)) << 8;
    v = (v | (uint32_t)x87_whole(c[1] * k)) << 8;
    *(volatile uint32_t *)(self + 8) = v | (uint32_t)x87_whole(c[2] * k);
}
static const uint8_t colour_pack_original[] = {0x83, 0xec, 0x08, 0x8b, 0x44, 0x24, 0x0c, 0xd9};
REWRITE("Colour::Pack", 0x9e84e0, colour_pack_original, &colour_pack, &g_colour_packs);

// ---- Tile::EdgePoint(out, x, y, direction) (0xABC300): the middle of a tile's edge, or its centre ----------------------
// (x + 0.5, y + 0.5), moved 0.2 toward the edge: direction 1 adds to x, 2 takes from y, 4 takes from x, 8 adds to y;
// any other direction leaves the centre.
static volatile long g_tile_edge_points;
static void __fastcall tile_edge_point(float *out, int32_t x, int32_t y, uint32_t direction) {
    g_tile_edge_points++;
    const long double half = *(const float *)0xf2651c, step = *(const float *)0xf283d8;
    long double px = (long double)x + half, py = (long double)y + half;
    switch (direction) {
    case 1: px = px + step; break;
    case 2: py = py - step; break;
    case 4: px = px - step; break;
    case 8: py = py + step; break;
    default: break;
    }
    ((volatile float *)out)[0] = (float)px;
    ((volatile float *)out)[1] = (float)py;
}
static const uint8_t tile_edge_point_original[] = {0x51, 0x89, 0x14, 0x24, 0xdb, 0x04, 0x24, 0x8b};
REWRITE("Tile::EdgePoint", 0xabc300, tile_edge_point_original, &tile_edge_point, &g_tile_edge_points);

// ---- WildAnimal::ClampB42390(this) (0xB42390): when flagged (bit 2 of +0xEC), a value at +0x148 from three others -----
// The greatest of +0x1E0, +0x1E4 and +0x1E8 (fcomi / fcmov, in that order: a NaN met first is kept), times 0.3423,
// plus 0.2148.
static volatile long g_wild_clamps;
static void __thiscall wild_animal_clamp_b42390(uint8_t *self) {
    g_wild_clamps++;
    if (!(self[0xec] & 4))
        return;
    const float *const f = (const float *)self;
    const volatile float t = (float)x87_greater(x87_load(&f[0x1e0 / 4]), x87_load(&f[0x1e4 / 4]));
    const volatile float u = (float)x87_greater(x87_load((const float *)&t), x87_load(&f[0x1e8 / 4]));
    *(volatile float *)(self + 0x148) = (float)((long double)u * *(const float *)0xf99824 + *(const float *)0xf99820);
}
static const uint8_t wild_animal_clamp_b42390_original[] = {0x8a, 0x81, 0xec, 0x00, 0x00, 0x00, 0x83, 0xec};
REWRITE("WildAnimal::ClampB42390", 0xb42390, wild_animal_clamp_b42390_original, &wild_animal_clamp_b42390, &g_wild_clamps);

// ---- Counts::AddByKind(this, thing, level) (0x5BC150): one added to the count for a thing's kind (+0x10) ----------------
// Kinds 1 and 2: +0x24; 4: +0x1C; 0x15-0x17: +0x18; 0x23: +0x20; 0x24: +8; 0x25: +0x10; 0x27: +0, only when the level
// is at least 11.1; 0x28: +4; 0x29: +0xC; 0x2A: +0x14; any other kind: nothing.
static volatile long g_kind_counts;
static void __thiscall counts_add_by_kind(uint8_t *self, const uint8_t *thing, float level) {
    g_kind_counts++;
    int at;
    switch (*(const uint32_t *)(thing + 0x10)) {
    case 1: case 2: at = 0x24; break;
    case 4: at = 0x1c; break;
    case 0x15: case 0x16: case 0x17: at = 0x18; break;
    case 0x23: at = 0x20; break;
    case 0x24: at = 8; break;
    case 0x25: at = 0x10; break;
    case 0x27:
        if (!((long double)level >= (long double)*(const float *)0xf3ef80))
            return;
        at = 0;
        break;
    case 0x28: at = 4; break;
    case 0x29: at = 0xc; break;
    case 0x2a: at = 0x14; break;
    default: return;
    }
    ++*(volatile uint32_t *)(self + at);
}
static const uint8_t counts_add_by_kind_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0x40, 0x10, 0x48};
REWRITE("Counts::AddByKind", 0x5bc150, counts_add_by_kind_original, &counts_add_by_kind, &g_kind_counts);

// ---- Guest::EntryRemaining(this, n) (0xB9AB80): the part still left of entry n of the table at +0x74 -------------------
// Entries 184 bytes from +0x20: when in use (byte +0) with a positive total (+0xC): 1 - used (+0x14) / total (rounded),
// at least 0; 1 otherwise.
static volatile long g_entry_remainings;
static long double __thiscall guest_entry_remaining(const uint8_t *self, int32_t n) {
    g_entry_remainings++;
    const uint8_t *const e = *(const uint8_t *const *)(self + 0x74) + n * 0xb8 + 0x20;
    const int32_t total = *(const int32_t *)(e + 0xc);
    if (!e[0] || total <= 0)
        return *(const float *)0xf264dc;
    const volatile float v = (float)((long double)*(const float *)0xf264dc - (long double)*(const int32_t *)(e + 0x14) / (long double)total);
    const volatile float kept = (float)x87_greater(0.0L, v);
    return kept;
}
static const uint8_t guest_entry_remaining_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0x49, 0x74, 0x69};
REWRITE("Guest::EntryRemaining", 0xb9ab80, guest_entry_remaining_original, &guest_entry_remaining, &g_entry_remainings);

// ---- Path::Case7289D0(this, a_out, b_out) (0x7289D0): two values from +0xA4 by a kind (byte +0x60) ----------------------
// a = v and b = v + 3; kind 1: b + 2; 2: a = v - 2; 4: b + 1; 5: a = v - 1. a_out = a times 1, b_out = b times 1 less 0.1.
static volatile long g_path_cases;
static void __thiscall path_case_7289d0(const uint8_t *self, float *a_out, float *b_out) {
    g_path_cases++;
    const int32_t v = *(const int32_t *)(self + 0xa4);
    int32_t a = v, b = v + 3;
    switch (self[0x60]) {
    case 1: b += 2; break;
    case 2: a = v - 2; break;
    case 4: b += 1; break;
    case 5: a = v - 1; break;
    default: break;
    }
    const long double one = *(const float *)0xf29428;
    *(volatile float *)a_out = (float)((long double)a * one);
    *(volatile float *)b_out = (float)((long double)b * one - *(const float *)0xf270b0);
}
static const uint8_t path_case_7289d0_original[] = {0x83, 0xec, 0x08, 0x8b, 0x81, 0xa4, 0x00, 0x00};
REWRITE("Path::Case7289D0", 0x7289d0, path_case_7289d0_original, &path_case_7289d0, &g_path_cases);

// ---- WaterDrops::EmitterCount(this, n) (0x94E000): how many drops emitter n gives -----------------------------------------
// Emitters 208 bytes at *(+0x1A4): 0 when the interval (+0xC) is at most 0; otherwise +0x10 / interval (rounded),
// rounded up (frndint, then one more when that went down; NaN stays), as a whole number (fistp).
static volatile long g_emitter_counts;
static uint32_t __thiscall water_drops_emitter_count(const uint8_t *self, int32_t n) {
    g_emitter_counts++;
    const float *const e = (const float *)(*(const uint8_t *const *)(self + 0x1a4) + n * 0xd0);
    const long double interval = e[3];
    if (interval < (long double)*(const float *)0xf264e0 || interval == (long double)*(const float *)0xf264e0)
        return 0;
    const volatile float q = (float)((long double)e[4] / e[3]);
    const long double whole = x87_round(q);
    const volatile float up = (float)(!((long double)q > whole) ? whole : whole + 1.0L);
    return (uint32_t)x87_whole(up);
}
static const uint8_t water_drops_emitter_count_original[] = {0x8b, 0x54, 0x24, 0x04, 0x8b, 0x81, 0xa4, 0x01};
REWRITE("WaterDrops::EmitterCount", 0x94e000, water_drops_emitter_count_original, &water_drops_emitter_count, &g_emitter_counts);

// ---- Water::Entry94DD40(this, n, level, v, w) (0x94DD40): entry n (208 bytes at *(+0x1A4)) given a new place ----------
// In mode 1 (+0x60) the old place (+0x30) is kept at +0x40 first; then +0xC4 = w, +0x30 = v's three values, and +0x14 =
// the level when it is at least 0.
static volatile long g_water_entries;
static void __thiscall water_entry_94dd40(const uint8_t *self, int32_t n, float level, const uint32_t *v, uint32_t w) {
    g_water_entries++;
    volatile uint32_t *const e = (volatile uint32_t *)(*(uint8_t *const *)(self + 0x1a4) + n * 0xd0);
    if (e[0x60 / 4] == 1)
        e[0x40 / 4] = e[0x30 / 4], e[0x44 / 4] = e[0x34 / 4], e[0x48 / 4] = e[0x38 / 4];
    e[0xc4 / 4] = w;
    e[0x30 / 4] = v[0], e[0x34 / 4] = v[1], e[0x38 / 4] = v[2];
    if (!((long double)level < (long double)*(const float *)0xf264e0) && level == level)
        e[0x14 / 4] = *(const uint32_t *)&level;
}
static const uint8_t water_entry_94dd40_original[] = {0x8b, 0x54, 0x24, 0x04, 0x8b, 0x81, 0xa4, 0x01};
REWRITE("Water::Entry94DD40", 0x94dd40, water_entry_94dd40_original, &water_entry_94dd40, &g_water_entries);

// ---- Effect::Fraction466470(this, near, far, angle, aspect) (0x466470): a perspective matrix ----------------------------
// r = far / (far - near) and c = 1 / tan(angle / 2) (both unrounded); all cleared but +0 = c, +0x14 = c aspect,
// +0x28 = r, +0x2C = 1, +0x38 = -(r near).
static volatile long g_effect_perspectives;
static void __thiscall effect_fraction_466470(uint8_t *self, float n, float f, float angle, float aspect) {
    g_effect_perspectives++;
    const long double r = (long double)f / ((long double)f - n);
    volatile uint32_t *const t = (volatile uint32_t *)self;
    static const int8_t cleared[] = {0xf, 0xd, 0xc, 9, 8, 7, 6, 4, 3, 2, 1};
    for (const int8_t i : cleared) t[i] = 0;
    t[0xb] = 0x3f800000;
    const long double c = (long double)*(const float *)0xf264dc / x87_tan((long double)angle * *(const float *)0xf2651c);
    volatile float *const m = (volatile float *)self;
    m[0] = (float)c;
    m[5] = (float)(c * aspect);
    m[10] = (float)r;
    m[14] = (float)-(r * n);
}
static const uint8_t effect_fraction_466470_original[] = {0xd9, 0x44, 0x24, 0x08, 0x33, 0xc0, 0xd8, 0x64};
REWRITE("Effect::Fraction466470", 0x466470, effect_fraction_466470_original, &effect_fraction_466470, &g_effect_perspectives);

// ---- Animations::Set(this, n, a, b, count, rate) (0x94DEC0): animation n (208 bytes at *(+0x1A4)) set up ---------------
// +0xA4 a, +0xA8 b, +0xAC count, +0xB0 rate, +0x10 the frame time (count - 1) / rate, +0x60 mode 4, +0x30 to +0x38
// cleared.
static volatile long g_animation_sets;
static void __thiscall animations_set(const uint8_t *self, int32_t n, uint32_t a, uint32_t b, int32_t count, float rate) {
    g_animation_sets++;
    volatile uint32_t *const e = (volatile uint32_t *)(*(uint8_t *const *)(self + 0x1a4) + n * 0xd0);
    e[0xa4 / 4] = a;
    e[0xac / 4] = (uint32_t)count;
    e[0xa8 / 4] = b;
    const long double frame = (long double)(int32_t)((uint32_t)count - 1) / rate;
    e[0x60 / 4] = 4;
    e[0xb0 / 4] = *(const uint32_t *)&rate;
    ((volatile float *)e)[0x10 / 4] = (float)frame;
    e[0x30 / 4] = 0, e[0x34 / 4] = 0, e[0x38 / 4] = 0;
}
static const uint8_t animations_set_original[] = {0x8b, 0x91, 0xa4, 0x01, 0x00, 0x00, 0x8b, 0x44};
REWRITE("Animations::Set", 0x94dec0, animations_set_original, &animations_set, &g_animation_sets);

// ---- Tracker::SetVelocity(this, p) (0x8B5230): a velocity (+0xC) from the move from the last position (+0x1C) to p -----
// (p - last) times 2/3 (0xF2A8C4); the y and z differences rounded, x's not; z's product rounded and written first.
// The update count (+0x2C) goes up by one.
static volatile long g_tracker_velocities;
static void __thiscall tracker_set_velocity(uint8_t *self, const float *p) {
    g_tracker_velocities++;
    const float *const f = (const float *)self;
    const long double k = *(const float *)0xf2a8c4;
    const long double dx = (long double)p[0] - f[0x1c / 4];
    const volatile float dy = (float)((long double)p[1] - f[0x20 / 4]), dz = (float)((long double)p[2] - f[0x24 / 4]);
    const long double vx = dx * k, vy = (long double)dy * k;
    const volatile float vz = (float)((long double)dz * k);
    volatile uint32_t *const t = (volatile uint32_t *)self;
    t[0x14 / 4] = *(const volatile uint32_t *)&vz;
    ((volatile float *)self)[0xc / 4] = (float)vx;
    ((volatile float *)self)[0x10 / 4] = (float)vy;
    t[0x2c / 4] = t[0x2c / 4] + 1;
}
static const uint8_t tracker_set_velocity_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Tracker::SetVelocity", 0x8b5230, tracker_set_velocity_original, &tracker_set_velocity, &g_tracker_velocities);

// ---- VerletPoint::Velocity(this, out) (0xA6B240): (position (+0x10) - previous (+0x20)) times 60 (0xF8F980) -------------
// The x and z differences rounded, y's not; x's product rounded, y's and z's not.
static volatile long g_verlet_velocities;
static void __thiscall verlet_point_velocity(const uint8_t *self, float *out) {
    g_verlet_velocities++;
    const float *const f = (const float *)self;
    const long double k = *(const float *)0xf8f980;
    const volatile float dx = (float)((long double)f[4] - f[8]);
    const long double dy = (long double)f[5] - f[9];
    const volatile float dz = (float)((long double)f[6] - f[10]);
    const volatile float vx = (float)((long double)dx * k);
    const long double vy = dy * k, vz = (long double)dz * k;
    volatile float *const o = out;
    o[0] = vx;
    o[1] = (float)vy;
    o[2] = (float)vz;
}
static const uint8_t verlet_point_velocity_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("VerletPoint::Velocity", 0xa6b240, verlet_point_velocity_original, &verlet_point_velocity, &g_verlet_velocities);

// ---- PoolComplex::ValueAAE350(this, flag) (0xAAE350): a quarter-turn angle by a mode (+0x130) ----------------------------
// v = the turn at +0x124 when the flag byte is set, else 2; mode 1 adds 2, mode 2 adds 1, mode 8 adds 3; (v & 3) times
// pi/2 (0xF35D50), unrounded on the FPU.
static volatile long g_pool_values;
static long double __thiscall pool_complex_value_aae350(const uint8_t *self, uint32_t flag) {
    g_pool_values++;
    int32_t v = (uint8_t)flag ? *(const int32_t *)(self + 0x124) : 2;
    switch (*(const uint32_t *)(self + 0x130)) {
    case 1: v += 2; break;
    case 2: v += 1; break;
    case 8: v += 3; break;
    default: break;
    }
    return (long double)(v & 3) * *(const float *)0xf35d50;
}
static const uint8_t pool_complex_value_aae350_original[] = {0x8a, 0x44, 0x24, 0x04, 0x84, 0xc0, 0x74, 0x08};
REWRITE("PoolComplex::ValueAAE350", 0xaae350, pool_complex_value_aae350_original, &pool_complex_value_aae350, &g_pool_values);

// ---- Vector::LerpInto(a, b, t, out) (0xB47A30): a + (b - a) t into out --------------------------------------------------
// The z difference rounded, x and y not; x's and z's products rounded, y's not; x's sum rounded before the writes.
static void __fastcall vector_lerp_into(const float *a, const float *b, float t, float *out) {
    g_lerps++;
    const long double dx = (long double)b[0] - a[0], dy = (long double)b[1] - a[1];
    const volatile float dz = (float)((long double)b[2] - a[2]);
    const volatile float px = (float)(dx * t);
    const long double py = dy * t;
    const volatile float pz = (float)((long double)dz * t);
    const volatile float x = (float)((long double)px + a[0]);
    const long double y = py + a[1], z = (long double)pz + a[2];
    volatile float *const o = out;
    o[0] = x;
    o[1] = (float)y;
    o[2] = (float)z;
}
static const uint8_t vector_lerp_into_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Vector::LerpInto", 0xb47a30, vector_lerp_into_original, &vector_lerp_into, &g_lerps);

// ---- Progress::Advance2(this, dt) (0xB34FA0): a tenth of dt added to a progress value (+0x138), at most 1 ---------------
// Gives 1 when the new value is past the threshold 0xF3B794, else (or for a NaN) 0.
static uint32_t __thiscall progress_advance2(uint8_t *self, float dt) {
    g_progress_advances++;
    const volatile float v = (float)((long double)dt * *(const float *)0xf270b0 + *(const float *)(self + 0x138));
    const volatile float u = (float)x87_lesser(1.0L, x87_load((const float *)&v));
    *(volatile uint32_t *)(self + 0x138) = *(const volatile uint32_t *)&u;
    return (long double)u > (long double)*(const float *)0xf3b794;
}
static const uint8_t progress_advance2_original[] = {0x83, 0xec, 0x08, 0xd9, 0x44, 0x24, 0x0c, 0xc7};
REWRITE("Progress::Advance2", 0xb34fa0, progress_advance2_original, &progress_advance2, &g_progress_advances);

// ---- Vector4::Add(this, v) (0xCC4890): v added to this, part by part; gives this ----------------------------------------
static volatile long g_vector4_adds;
static float *__thiscall vector4_add(float *self, const float *v) {
    g_vector4_adds++;
    volatile float *const t = self;
    for (int i = 0; i < 4; i++) t[i] = (float)((long double)t[i] + v[i]);
    return self;
}
static const uint8_t vector4_add_original[] = {0x55, 0x8b, 0xec, 0x51, 0xc7, 0x45, 0xfc, 0xcc};
REWRITE("Vector4::Add", 0xcc4890, vector4_add_original, &vector4_add, &g_vector4_adds);

// ---- Fireworks::AddTime9E0D70(this, n) (0x9E0D70): 100 n added to a time (+0x3C), kept between 0 and the game's limit ----
// (0x10F0E80, signed); then +0x58 = (+0x28 / time) ((+0x14 / +0x5C) 2).
static volatile long g_firework_times;
static void __thiscall fireworks_add_time_9e0d70(uint8_t *self, int32_t n) {
    g_firework_times++;
    volatile int32_t *const time = (volatile int32_t *)(self + 0x3c);
    const int32_t v = (int32_t)((uint32_t)*time + (uint32_t)n * 100);
    *time = v;
    const int32_t limit = *(const volatile int32_t *)0x10f0e80;
    int32_t c = v < 0 ? 0 : v;
    c = c > limit ? limit : c;
    *time = c;
    const float *const f = (const float *)self;
    const long double q = (long double)f[0x14 / 4] / f[0x5c / 4];
    *(volatile float *)(self + 0x58) = (float)((long double)*(const int32_t *)(self + 0x28) / (long double)c * (q + q));
}
static const uint8_t fireworks_add_time_9e0d70_original[] = {0x83, 0xec, 0x08, 0x8b, 0x44, 0x24, 0x0c, 0x8b};
REWRITE("Fireworks::AddTime9E0D70", 0x9e0d70, fireworks_add_time_9e0d70_original, &fireworks_add_time_9e0d70, &g_firework_times);

// ---- Colour::ToARGB(out, r, g, b, a) (0x402C40): four values from 0 to 1 packed as 0xAARRGGBB; gives out --------------
// Each times 255 (fistp, low 32 bits, not masked): ((a << 8 | r) << 8 | g) << 8 | b.
static uint32_t *__thiscall colour_to_argb(uint32_t *out, float r, float g, float b, float a) {
    g_colour_packs++;
    const long double k = *(const float *)0xf264c0;
    uint32_t v = (uint32_t)x87_whole(a * k) << 8;
    v = (v | (uint32_t)x87_whole(r * k)) << 8;
    v = (v | (uint32_t)x87_whole(g * k)) << 8;
    *(volatile uint32_t *)out = v | (uint32_t)x87_whole(b * k);
    return out;
}
static const uint8_t colour_to_argb_original[] = {0x83, 0xec, 0x08, 0xd9, 0x44, 0x24, 0x18, 0x8b};
REWRITE("Colour::ToARGB", 0x402c40, colour_to_argb_original, &colour_to_argb, &g_colour_packs);

// ---- Screen::ToUnit(this, out, p) (0x483F40): a screen position as -1 to 1 across the GUI area --------------------------
// w = +0x84 - +0x7C and h = +0x88 - +0x80 (rounded): ((2 x - w) (rounded) / w, (2 y - h) / h).
static volatile long g_screen_units;
static void __thiscall screen_to_unit(const uint8_t *self, float *out, const float *p) {
    g_screen_units++;
    const float *const f = (const float *)self;
    const long double x2 = (long double)p[0] + p[0], y2 = (long double)p[1] + p[1];
    const volatile float w = (float)((long double)f[0x84 / 4] - f[0x7c / 4]), h = (float)((long double)f[0x88 / 4] - f[0x80 / 4]);
    const volatile float x = (float)(x2 - w);
    const long double y = y2 - h;
    const volatile float ux = (float)((long double)x / w);
    const long double uy = y / h;
    ((volatile float *)out)[0] = ux;
    ((volatile float *)out)[1] = (float)uy;
}
static const uint8_t screen_to_unit_original[] = {0x83, 0xec, 0x10, 0x8b, 0x44, 0x24, 0x18, 0xd9};
REWRITE("Screen::ToUnit", 0x483f40, screen_to_unit_original, &screen_to_unit, &g_screen_units);

// ---- FloatSlider::SetValue, SetValue4 and SetValue8 (0xCA7A30, 0xCA7C00, 0xCA7DD0): the bound decimal number -------------
// v = (max - min) fraction + min (+0xC, +8; rounded), kept at least min and at most max (fcomi / fcmov: a NaN gives
// min), into the bound value (*(+4)), or the bound object's +4 or +8.
static inline void float_slider_set(const uint8_t *self, float fraction, int at) {
    const float *const f = (const float *)self;
    const volatile float v = (float)(((long double)f[3] - f[2]) * fraction + f[2]);
    const volatile float t = (float)x87_greater(x87_load(&f[2]), x87_load((const float *)&v));
    const volatile float u = (float)x87_lesser(x87_load(&f[3]), x87_load((const float *)&t));
    *(volatile uint32_t *)(*(uint8_t *const *)(self + 4) + at) = *(const volatile uint32_t *)&u;
}
static volatile long g_float_slider_sets;
static void __thiscall float_slider_set_value(const uint8_t *self, float fraction) { g_float_slider_sets++, float_slider_set(self, fraction, 0); }
static void __thiscall float_slider_set_value4(const uint8_t *self, float fraction) { g_float_slider_sets++, float_slider_set(self, fraction, 4); }
static void __thiscall float_slider_set_value8(const uint8_t *self, float fraction) { g_float_slider_sets++, float_slider_set(self, fraction, 8); }
static const uint8_t float_slider_set_original[] = {0x83, 0xec, 0x08, 0xd9, 0x41, 0x0c, 0x8b, 0x41};
REWRITE("FloatSlider::SetValue", 0xca7a30, float_slider_set_original, &float_slider_set_value, &g_float_slider_sets);
REWRITE("FloatSlider::SetValue4", 0xca7c00, float_slider_set_original, &float_slider_set_value4, &g_float_slider_sets);
REWRITE("FloatSlider::SetValue8", 0xca7dd0, float_slider_set_original, &float_slider_set_value8, &g_float_slider_sets);

// ---- Colour::PackArgs(-, r, g, b, a) (0xB96F90): four values from 0 to 1 packed as 0xAARRGGBB, given back -------------
// Each times 255 (fistp, low 32 bits, not masked): ((a << 8 | r) << 8 | g) << 8 | b. (The value is also stored at *ecx.)
static uint32_t __thiscall colour_pack_args(uint32_t *out, float r, float g, float b, float a) {
    g_colour_packs++;
    const long double k = *(const float *)0xf264c0;
    uint32_t v = (uint32_t)x87_whole(a * k) << 8;
    v = (v | (uint32_t)x87_whole(r * k)) << 8;
    v = (v | (uint32_t)x87_whole(g * k)) << 8;
    v |= (uint32_t)x87_whole(b * k);
    *(volatile uint32_t *)out = v;
    return v;
}
static const uint8_t colour_pack_args_original[] = {0x83, 0xec, 0x08, 0xd9, 0x44, 0x24, 0x18, 0xd8};
REWRITE("Colour::PackArgs", 0xb96f90, colour_pack_args_original, &colour_pack_args, &g_colour_packs);

// ---- Colour::PackTo(c, out) (0x425700): a colour of four values from 0 to 1 packed into *out; gives out ------------------
static uint32_t *__thiscall colour_pack_to(const float *c, uint32_t *out) {
    g_colour_packs++;
    const long double k = *(const float *)0xf264c0;
    uint32_t v = (uint32_t)x87_whole(c[3] * k) << 8;
    v = (v | (uint32_t)x87_whole(c[0] * k)) << 8;
    v = (v | (uint32_t)x87_whole(c[1] * k)) << 8;
    *(volatile uint32_t *)out = v | (uint32_t)x87_whole(c[2] * k);
    return out;
}
static const uint8_t colour_pack_to_original[] = {0x83, 0xec, 0x08, 0xd9, 0x41, 0x0c, 0xd8, 0x0d};
REWRITE("Colour::PackTo", 0x425700, colour_pack_to_original, &colour_pack_to, &g_colour_packs);

// ---- Spring::Constants(this, a_out, b_out, c_out) (0x912C20): a spring's constants from its length ----------------------
// L = +4 - +0xC and k = C / L (C at 0xF71CB0; both unrounded): a_out = k, b_out = (D - k L) (2 / (L L)) (D at
// 0xF355E4); when c_out is given, 1 / ((L - E) / 4 + E) (E at 0xF275F4).
static volatile long g_spring_constants;
static void __thiscall spring_constants(const uint8_t *self, float *a_out, float *b_out, float *c_out) {
    g_spring_constants++;
    const float *const f = (const float *)self;
    const long double L = (long double)f[1] - f[3];
    const long double k = (long double)*(const float *)0xf71cb0 / L;
    *(volatile float *)a_out = (float)k;
    *(volatile float *)b_out = (float)(((long double)*(const float *)0xf355e4 - k * L) * ((long double)*(const float *)0xf2705c / (L * L)));
    if (c_out) {
        const long double e = *(const float *)0xf275f4;
        *(volatile float *)c_out = (float)((long double)*(const float *)0xf264dc / ((L - e) * *(const float *)0xf270b8 + e));
    }
}
static const uint8_t spring_constants_original[] = {0xd9, 0x41, 0x04, 0x8b, 0x44, 0x24, 0x04, 0xd8};
REWRITE("Spring::Constants", 0x912c20, spring_constants_original, &spring_constants, &g_spring_constants);

// ---- Vector::Lerp3(out, a, b, -, t) (0xC5A860): a + (b - a) t ------------------------------------------------------------
// The z difference rounded, x and y not; x's and y's products rounded, z's not.
static void __thiscall vector_lerp3(float *out, const float *a, const float *b, uint32_t, float t) {
    g_lerps++;
    const long double dx = (long double)b[0] - a[0], dy = (long double)b[1] - a[1];
    const volatile float dz = (float)((long double)b[2] - a[2]);
    const volatile float px = (float)(dx * t), py = (float)(dy * t);
    const long double z = (long double)dz * t + a[2], y = (long double)py + a[1], x = (long double)px + a[0];
    volatile float *const o = out;
    o[0] = (float)x;
    o[1] = (float)y;
    o[2] = (float)z;
}
static const uint8_t vector_lerp3_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Vector::Lerp3", 0xc5a860, vector_lerp3_original, &vector_lerp3, &g_lerps);

// ---- Particle::Launch(this, p, flag, a, b, h) (0xA17520): a particle started at p raised by h --------------------------
// +0x40 p.x (through the FPU), +0x44 p.y + h, +0x48 p.z as it is; +0x58 a, +0x5C b, +0x60 h; +0x50 1, +0x54 0, byte
// +0x90 the flag.
static volatile long g_particle_launches;
static void __thiscall particle_launch(uint8_t *self, const float *p, uint32_t flag, uint32_t a, uint32_t b, float h) {
    g_particle_launches++;
    volatile uint32_t *const t = (volatile uint32_t *)self;
    const long double x = x87_load(&p[0]);
    const long double y = (long double)h + p[1];
    const uint32_t z = *(const uint32_t *)&p[2];
    ((volatile float *)self)[0x40 / 4] = (float)x;
    t[0x48 / 4] = z;
    ((volatile float *)self)[0x44 / 4] = (float)y;
    t[0x58 / 4] = a;
    t[0x5c / 4] = b;
    t[0x50 / 4] = 1;
    t[0x54 / 4] = 0;
    t[0x60 / 4] = *(const uint32_t *)&h;
    ((volatile uint8_t *)self)[0x90] = (uint8_t)flag;
}
static const uint8_t particle_launch_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Particle::Launch", 0xa17520, particle_launch_original, &particle_launch, &g_particle_launches);

// ---- Contact::Make(out, a, b, v, pair) (0xAF8E30, and Contact::Make2 0xAF75D0, the same code): a contact record --------
// +0 a (through the FPU), +0xC b's first two (through the FPU) and its third as it is, +0x18 v, +0x1C the pair.
static volatile long g_contacts;
static void __stdcall contact_make(uint32_t *out, const float *a, const float *b, uint32_t v, const uint32_t *pair) {
    g_contacts++;
    volatile float *const o = (volatile float *)out;
    const long double a0 = x87_load(&a[0]), a1 = x87_load(&a[1]), a2 = x87_load(&a[2]);
    o[0] = (float)a0, o[1] = (float)a1, o[2] = (float)a2;
    const long double b0 = x87_load(&b[0]), b1 = x87_load(&b[1]);
    const uint32_t b2 = *(const uint32_t *)&b[2];
    o[3] = (float)b0, o[4] = (float)b1;
    volatile uint32_t *const t = out;
    t[5] = b2, t[6] = v, t[7] = pair[0], t[8] = pair[1];
}
static const uint8_t contact_make_original[] = {0x83, 0xec, 0x0c, 0x8b, 0x44, 0x24, 0x14, 0xd9};
REWRITE("Contact::Make", 0xaf8e30, contact_make_original, &contact_make, &g_contacts);
REWRITE("Contact::Make2", 0xaf75d0, contact_make_original, &contact_make, &g_contacts);

// ---- Path::Cell704A50(this) (0x704A50): a cell (+0x1A8, or +0x1B4 when byte +0x1B0 is set) scaled into +0x1BC -----------
// Each whole number times 1/1440 (0xF68B5C).
static volatile long g_path_cells;
static void __thiscall path_cell_704a50(uint8_t *self) {
    g_path_cells++;
    const int32_t *const c = (const int32_t *)(self + (self[0x1b0] ? 0x1b4 : 0x1a8));
    const long double k = *(const float *)0xf68b5c;
    *(volatile float *)(self + 0x1bc) = (float)((long double)c[0] * k);
    *(volatile float *)(self + 0x1c0) = (float)((long double)c[1] * k);
}
static const uint8_t path_cell_704a50_original[] = {0x83, 0xec, 0x08, 0x8b, 0x81, 0xa8, 0x01, 0x00};
REWRITE("Path::Cell704A50", 0x704a50, path_cell_704a50_original, &path_cell_704a50, &g_path_cells);

// ---- Blink::Advance(this, dt) (0x6B60A0): a looping time (+0x6010) moved on by *dt and kept below 1 ---------------------
// The sum is stored; when it (unrounded) is past 1, its whole part (rounded down, as a float) is taken off again.
static void __thiscall blink_advance(uint8_t *self, const float *dt) {
    g_blinks++;
    volatile float *const time = (volatile float *)(self + 0x6010);
    const long double v = (long double)dt[0] + *time;
    *time = (float)v;
    if (!(v > (long double)*(const float *)0xf264dc))
        return;
    const volatile float q = (float)v;
    const long double whole = x87_round(q);
    const volatile float down = (float)((long double)q < whole ? whole - 1.0L : whole);
    *time = (float)((long double)*time - down);
}
static const uint8_t blink_advance_original[] = {0x51, 0x8b, 0x44, 0x24, 0x08, 0xd9, 0x00, 0xd8};
REWRITE("Blink::Advance", 0x6b60a0, blink_advance_original, &blink_advance, &g_blinks);

// ---- Particle::FirstColour / SecondColour(this, out) (0x9C62D0, 0x9C6330): a colour (+0xAC or +0xB8) packed opaque ------
// Each times 255 (fistp, low 32 bits): ((r | 0xFFFFFF00) << 8 | g) << 8 | b into *out; gives out.
static inline uint32_t *particle_colour(const uint8_t *self, int at, uint32_t *out) {
    const float *const c = (const float *)(self + at);
    const long double k = *(const float *)0xf264c0;
    uint32_t v = ((uint32_t)x87_whole(c[0] * k) | 0xffffff00u) << 8;
    v = (v | (uint32_t)x87_whole(c[1] * k)) << 8;
    *(volatile uint32_t *)out = v | (uint32_t)x87_whole(c[2] * k);
    return out;
}
static uint32_t *__thiscall particle_first_colour(const uint8_t *self, uint32_t *out) { g_colour_packs++; return particle_colour(self, 0xac, out); }
static uint32_t *__thiscall particle_second_colour(const uint8_t *self, uint32_t *out) { g_colour_packs++; return particle_colour(self, 0xb8, out); }
static const uint8_t particle_first_colour_original[] = {0x83, 0xec, 0x08, 0xd9, 0x81, 0xac, 0x00, 0x00};
static const uint8_t particle_second_colour_original[] = {0x83, 0xec, 0x08, 0xd9, 0x81, 0xb8, 0x00, 0x00};
REWRITE("Particle::FirstColour", 0x9c62d0, particle_first_colour_original, &particle_first_colour, &g_colour_packs);
REWRITE("Particle::SecondColour", 0x9c6330, particle_second_colour_original, &particle_second_colour, &g_colour_packs);

// ---- Object::Position(this, out) (0x548B40): the position (+0x90) of the object's part (+0x41C, else +0x418) ------------
// x copied as it is, y and z through the FPU.
static volatile long g_object_positions;
static void __thiscall object_position(const uint8_t *self, float *out) {
    g_object_positions++;
    const uint8_t *part = *(const uint8_t *const *)(self + 0x41c);
    if (!part)
        part = *(const uint8_t *const *)(self + 0x418);
    const float *const p = (const float *)(part + 0x90);
    const long double z = x87_load(&p[2]);
    const uint32_t x = *(const uint32_t *)&p[0];
    const long double y = x87_load(&p[1]);
    ((volatile float *)out)[1] = (float)y;
    ((volatile uint32_t *)out)[0] = x;
    ((volatile float *)out)[2] = (float)z;
}
static const uint8_t object_position_original[] = {0x8b, 0x81, 0x1c, 0x04, 0x00, 0x00, 0x85, 0xc0};
REWRITE("Object::Position", 0x548b40, object_position_original, &object_position, &g_object_positions);

// ---- ByteSlider::SetValue / CharSlider::SetValue(this, fraction) (0xCA81B0, 0xCA80A0): the bound byte ---------------------
// v = the low byte of (max - min) fraction + min (fistp; min +8, max +9, bytes); kept at least min and at most max,
// compared unsigned (ByteSlider) or signed (CharSlider).
static volatile long g_byte_slider_sets;
static void __thiscall byte_slider_set_value(const uint8_t *self, float fraction) {
    g_byte_slider_sets++;
    const uint8_t min = self[8], max = self[9];
    const uint8_t v = (uint8_t)x87_whole((long double)(int32_t)(max - min) * fraction + (long double)(int32_t)min);
    const uint8_t t = min > v ? min : v;
    **(volatile uint8_t *const *)(self + 4) = max >= t ? t : max;
}
static void __thiscall char_slider_set_value(const uint8_t *self, float fraction) {
    g_byte_slider_sets++;
    const int8_t min = (int8_t)self[8], max = (int8_t)self[9];
    const int8_t v = (int8_t)(uint8_t)x87_whole((long double)(int32_t)(max - min) * fraction + (long double)(int32_t)min);
    const int8_t t = min > v ? min : v;
    **(volatile int8_t *const *)(self + 4) = max >= t ? t : max;
}
static const uint8_t byte_slider_set_value_original[] = {0x83, 0xec, 0x0c, 0x8a, 0x41, 0x08, 0x8a, 0x51};
REWRITE("ByteSlider::SetValue", 0xca81b0, byte_slider_set_value_original, &byte_slider_set_value, &g_byte_slider_sets);
REWRITE("CharSlider::SetValue", 0xca80a0, byte_slider_set_value_original, &char_slider_set_value, &g_byte_slider_sets);

// ---- Window::FadeC5E410(this, v) (0xC5E410): +0x9C cleared, +0xE0 set from v -----------------------------------------
// r = 1 - +0xDC (unrounded): r when v is past it; otherwise 0 when v is below 0, else v as it is.
static volatile long g_window_fades;
static void __thiscall window_fade_c5e410(uint8_t *self, float v) {
    g_window_fades++;
    const long double r = (long double)*(const float *)0xf264dc - *(const float *)(self + 0xdc);
    ((volatile uint8_t *)self)[0x9c] = 0;
    if ((long double)v > r)
        *(volatile float *)(self + 0xe0) = (float)r;
    else if ((long double)v < (long double)*(const float *)0xf264e0)
        *(volatile uint32_t *)(self + 0xe0) = 0;
    else
        *(volatile uint32_t *)(self + 0xe0) = *(const uint32_t *)&v;
}
static const uint8_t window_fade_c5e410_original[] = {0xd9, 0x05, 0xdc, 0x64, 0xf2, 0x00, 0x33, 0xd2};
REWRITE("Window::FadeC5E410", 0xc5e410, window_fade_c5e410_original, &window_fade_c5e410, &g_window_fades);

// ---- Bar::Fill(this) (0x436D70): a bar's value (+0x2E8) over the game's maximum (0x10EDA7C), kept between 0 and 1 --------
// fcomi / fcmov: a NaN gives 0.
static volatile long g_bar_fills;
static long double __thiscall bar_fill(const uint8_t *self) {
    g_bar_fills++;
    const volatile float q = (float)((long double)*(const float *)(self + 0x2e8) / *(const volatile float *)0x10eda7c);
    const volatile float t = (float)x87_greater(0.0L, x87_load((const float *)&q));
    const volatile float u = (float)x87_lesser(x87_load((const float *)&t), 1.0L);
    return u;
}
static const uint8_t bar_fill_original[] = {0x83, 0xec, 0x0c, 0xd9, 0x81, 0xe8, 0x02, 0x00};
REWRITE("Bar::Fill", 0x436d70, bar_fill_original, &bar_fill, &g_bar_fills);

// ---- Angle::Sector(angle) (0xBC4F90): which sector an angle falls in ------------------------------------------------------
// v = angle + the offset 0xFA0DEC (unrounded), less a turn (0xF268DC) when not below it; v times the scale 0xF9B168
// (rounded), rounded down, as a whole number (fistp).
static volatile long g_angle_sectors;
static uint32_t __stdcall angle_sector(float angle) {
    g_angle_sectors++;
    long double v = (long double)angle + *(const float *)0xfa0dec;
    if (!(v < (long double)*(const float *)0xf268dc) && v == v)
        v = v - *(const float *)0xf268dc;
    const volatile float q = (float)(v * *(const float *)0xf9b168);
    const long double whole = x87_round(q);
    const volatile float down = (float)((long double)q < whole ? whole - 1.0L : whole);
    return (uint32_t)x87_whole(down);
}
static const uint8_t angle_sector_original[] = {0xd9, 0x44, 0x24, 0x04, 0x83, 0xec, 0x08, 0xd8};
REWRITE("Angle::Sector", 0xbc4f90, angle_sector_original, &angle_sector, &g_angle_sectors);

// ---- Counter::AddFraction(this, rate) (0xCDB720): a rate kept (+0) and added, scaled, to a running fraction ------------
// s = rate K (0xF27370) + the rest (+8), rounded; its whole part (rounded down) as a whole number at +4, the rest
// s - whole at +8.
static volatile long g_counter_fractions;
static void __thiscall counter_add_fraction(uint8_t *self, float rate) {
    g_counter_fractions++;
    const volatile float s = (float)((long double)rate * *(const float *)0xf27370 + *(const float *)(self + 8));
    const long double whole = x87_round(s);
    const volatile float down = (float)((long double)s < whole ? whole - 1.0L : whole);
    const uint32_t n = (uint32_t)x87_whole(down);
    *(volatile uint32_t *)self = *(const uint32_t *)&rate;
    *(volatile uint32_t *)(self + 4) = n;
    *(volatile float *)(self + 8) = (float)((long double)s - down);
}
static const uint8_t counter_add_fraction_original[] = {0x83, 0xec, 0x10, 0xd9, 0x44, 0x24, 0x14, 0xd8};
REWRITE("Counter::AddFraction", 0xcdb720, counter_add_fraction_original, &counter_add_fraction, &g_counter_fractions);

// ---- Vector4::Scale(this, k) (0xCC48F0): each of four values times k; gives this -----------------------------------------
static float *__thiscall vector4_scale(float *self, float k) {
    g_vector4_adds++;
    volatile float *const t = self;
    for (int i = 0; i < 4; i++) t[i] = (float)((long double)k * t[i]);
    return self;
}
static const uint8_t vector4_scale_original[] = {0x55, 0x8b, 0xec, 0x51, 0xc7, 0x45, 0xfc, 0xcc};
REWRITE("Vector4::Scale", 0xcc48f0, vector4_scale_original, &vector4_scale, &g_vector4_adds);

// ---- WAS_PlayWithItem::AddProgress(this, d) (0xB23990): d added to the play progress (+0x130), kept between 0 and 1 ------
// The sum is stored first, then kept (fcomi / fcmov: a NaN stays).
static void __thiscall was_play_with_item_add_progress(uint8_t *self, float d) {
    g_progress_advances++;
    volatile float *const p = (volatile float *)(self + 0x130);
    const volatile float v = (float)((long double)d + *p);
    *p = v;
    const long double t = x87_greater(v, 0.0L);
    *p = (float)x87_lesser(t, 1.0L);
}
static const uint8_t was_play_with_item_add_progress_original[] = {0x83, 0xec, 0x0c, 0xd9, 0x44, 0x24, 0x10, 0xc7};
REWRITE("WAS_PlayWithItem::AddProgress", 0xb23990, was_play_with_item_add_progress_original, &was_play_with_item_add_progress, &g_progress_advances);

// ---- ScrollBar::SetPageSize(this, size) (0xC5E3C0): a page size (+0xDC, at most 1), the position (+0xE0) kept in ---------
// When page + position is past 1, the position becomes 1 - page.
static volatile long g_page_sizes;
static void __thiscall scroll_bar_set_page_size(uint8_t *self, float size) {
    g_page_sizes++;
    const volatile float page = (float)x87_lesser(1.0L, x87_load(&size));
    const long double sum = (long double)page + *(const float *)(self + 0xe0);
    *(volatile uint32_t *)(self + 0xdc) = *(const volatile uint32_t *)&page;
    if (sum > (long double)*(const float *)0xf264dc)
        *(volatile float *)(self + 0xe0) = (float)((long double)*(const float *)0xf264dc - page);
}
static const uint8_t scroll_bar_set_page_size_original[] = {0x83, 0xec, 0x08, 0xc7, 0x44, 0x24, 0x04, 0x00};
REWRITE("ScrollBar::SetPageSize", 0xc5e3c0, scroll_bar_set_page_size_original, &scroll_bar_set_page_size, &g_page_sizes);

// ---- Points::Within(a, b, d) (0x925D20): whether two points are closer than d ---------------------------------------------
// |sqrt((dz dz + dx dx) + dy dy)| < d (unrounded), as a byte.
static volatile long g_points_within;
static uint8_t __stdcall points_within(const float *a, const float *b, float d) {
    g_points_within++;
    const long double dx = (long double)a[0] - b[0], dy = (long double)a[1] - b[1], dz = (long double)a[2] - b[2];
    return x87_abs(x87_sqrt((dz * dz + dx * dx) + dy * dy)) < (long double)d;
}
static const uint8_t points_within_original[] = {0x55, 0x8b, 0xec, 0x8b, 0x45, 0x08, 0x8b, 0x4d};
REWRITE("Points::Within", 0x925d20, points_within_original, &points_within, &g_points_within);

// ---- Angle::Octant(this) (0x4412C0): which of eight directions an angle (+0x48 less +0x54) points in ---------------------
// q = (+0x48 - +0x54) K (0xF28CF8, rounded), rounded down, as a whole number (fistp); (q / 4) % 8, signed as C divides.
static uint32_t __thiscall angle_octant(const uint8_t *self) {
    g_angle_sectors++;
    const float *const f = (const float *)self;
    const volatile float q = (float)(((long double)f[0x48 / 4] - f[0x54 / 4]) * *(const float *)0xf28cf8);
    const long double whole = x87_round(q);
    const volatile float down = (float)((long double)q < whole ? whole - 1.0L : whole);
    const int32_t n = (int32_t)(uint32_t)x87_whole(down);
    return (uint32_t)((n / 4) % 8);
}
static const uint8_t angle_octant_original[] = {0x83, 0xec, 0x0c, 0xd9, 0x41, 0x48, 0xd8, 0x61};
REWRITE("Angle::Octant", 0x4412c0, angle_octant_original, &angle_octant, &g_angle_sectors);

// ---- Window::CaseA45110(n) (0xA45110): one of six of the game's numbers by case, or 0 --------------------------------------
static volatile long g_window_cases;
static long double __stdcall window_case_a45110(uint32_t n) {
    g_window_cases++;
    static const uint32_t at[6] = {0xa450c4, 0xf27380, 0xf8db68, 0xf8db64, 0xf79714, 0xf4c75c};
    return *(const float *)(uintptr_t)(n <= 5 ? at[n] : 0xf264e0);
}
static const uint8_t window_case_a45110_original[] = {0x8b, 0x44, 0x24, 0x04, 0x83, 0xf8, 0x05, 0x77};
REWRITE("Window::CaseA45110", 0xa45110, window_case_a45110_original, &window_case_a45110, &g_window_cases);

// ---- Graph::PointFor(this, out, v, below) (0x99D300): a point of a graph from a value pair --------------------------------
// y = |v.y| (negated when the byte below is set); out.y = (y + 1) (+0x88 - +0x80) / 2; with s = the scale record at
// +0xDC: out.x = s's +0xDC v.x + s's +0x8C.
static volatile long g_graph_points;
static float *__thiscall graph_point_for(const uint8_t *self, float *out, const float *v, uint32_t below) {
    g_graph_points++;
    long double y = x87_abs(v[1]);
    if ((uint8_t)below)
        y = -y;
    const float *const f = (const float *)self;
    const long double h = (long double)f[0x88 / 4] - f[0x80 / 4];
    const float *const s = *(const float *const *)(self + 0xdc);
    const long double py = h * (y + *(const float *)0xf264dc) * *(const float *)0xf2651c;
    const long double px = (long double)s[0xdc / 4] * v[0] + s[0x8c / 4];
    ((volatile float *)out)[0] = (float)px;
    ((volatile float *)out)[1] = (float)py;
    return out;
}
static const uint8_t graph_point_for_original[] = {0x8b, 0x44, 0x24, 0x08, 0xd9, 0x40, 0x04, 0x8a};
REWRITE("Graph::PointFor", 0x99d300, graph_point_for_original, &graph_point_for, &g_graph_points);

// ---- Ride::Flip548A90(this) (0x548A90): six values negated through the FPU ------------------------------------------------
// +0x20 to +0x28 (written +0x28, +0x20, +0x24), then +0 to +8 (written +8, +0, +4).
static volatile long g_ride_flips;
static void __thiscall ride_flip_548a90(float *self) {
    g_ride_flips++;
    volatile float *const t = self;
    static const int8_t bases[] = {8, 0};
    for (const int8_t base : bases) {
        const long double a = -x87_load(&self[base]), b = -x87_load(&self[base + 1]), c = -x87_load(&self[base + 2]);
        t[base + 2] = (float)c;
        t[base] = (float)a;
        t[base + 1] = (float)b;
    }
}
static const uint8_t ride_flip_548a90_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Ride::Flip548A90", 0x548a90, ride_flip_548a90_original, &ride_flip_548a90, &g_ride_flips);

// ---- Colour::ToRGB(out, r, g, b) (0x402BF0): three values from 0 to 1 packed as an opaque 0xFFRRGGBB; gives out ----------
static uint32_t *__thiscall colour_to_rgb(uint32_t *out, float r, float g, float b) {
    g_colour_packs++;
    const long double k = *(const float *)0xf264c0;
    uint32_t v = ((uint32_t)x87_whole(r * k) | 0xffffff00u) << 8;
    v = (v | (uint32_t)x87_whole(g * k)) << 8;
    *(volatile uint32_t *)out = v | (uint32_t)x87_whole(b * k);
    return out;
}
static const uint8_t colour_to_rgb_original[] = {0x83, 0xec, 0x08, 0xd9, 0x44, 0x24, 0x0c, 0x8b};
REWRITE("Colour::ToRGB", 0x402bf0, colour_to_rgb_original, &colour_to_rgb, &g_colour_packs);

// ---- Park::Copy9E0DD0(this, from) (0x9E0DD0): a point copied, and its offset from the game's origin (0x14002C0) ----------
// +0 and +4 copied as they are, +8 through the FPU; +0x10 the point less the origin (z's rounded and written first).
static volatile long g_park_copies;
static void __thiscall park_copy_9e0dd0(uint8_t *self, const uint32_t *from) {
    g_park_copies++;
    volatile uint32_t *const t = (volatile uint32_t *)self;
    t[0] = from[0], t[1] = from[1];
    const long double z = x87_load((const float *)&from[2]);
    ((volatile float *)self)[2] = (float)z;
    const float *const origin = (const float *)0x14002c0;
    const long double dx = (long double)((const volatile float *)self)[0] - origin[0];
    const long double dy = (long double)((const volatile float *)self)[1] - origin[1];
    const volatile float dz = (float)(z - origin[2]);
    t[6] = *(const volatile uint32_t *)&dz;
    ((volatile float *)self)[4] = (float)dx;
    ((volatile float *)self)[5] = (float)dy;
}
static const uint8_t park_copy_9e0dd0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Park::Copy9E0DD0", 0x9e0dd0, park_copy_9e0dd0_original, &park_copy_9e0dd0, &g_park_copies);

// ---- Value::MoveToward(value, rate, target, dt) (0x9E85F0): a value moved toward a target by rate dt ------------------
// Gives (in the low byte) 1 straight away when it already equals the target. Nothing (0) when the distance is at most
// 1e-8 (or a NaN). Otherwise the step is added (target above) or taken off (not above) and stored; when that passes the
// target (unrounded) the value becomes the target and 1, else 0.
static volatile long g_move_towards;
static uint8_t __thiscall value_move_toward(float *value, float rate, float target, float dt) {
    g_move_towards++;
    volatile float *const v = value;
    if ((long double)*v == (long double)target)
        return 1;
    const long double step = (long double)rate * dt;
    const long double d = (long double)target - *v;
    if (!(x87_abs(d) > (long double)*(const float *)0xf2a9bc))
        return 0;
    if (d > (long double)*(const float *)0xf264e0) {
        const long double n = step + *v;
        *v = (float)n;
        if (!(n > (long double)target))
            return 0;
    } else {
        const long double n = (long double)*v - step;
        *v = (float)n;
        if (!(n < (long double)target))
            return 0;
    }
    *(volatile uint32_t *)value = *(const uint32_t *)&target;
    return 1;
}
static const uint8_t value_move_toward_original[] = {0xd9, 0x44, 0x24, 0x08, 0xd9, 0x01, 0xda, 0xe9};
REWRITE("Value::MoveToward", 0x9e85f0, value_move_toward_original, &value_move_toward, &g_move_towards);

// ---- Keys::Find / Find2(this, t) (0xE02670, 0xE025F0): the key a time falls in ------------------------------------------
// Keys of 16 bytes, time first (at +0x10, count +0xC; Find2: +8, +4): 0 unless t is past the first key's time; the last
// when t is at least the last's; otherwise by halving: lo = 0, hi = last; while lo < hi - 1 the middle (lo + (hi - lo)
// / 2) becomes hi when t is at most its time (or a NaN), else lo; gives lo.
static inline uint32_t keys_find(const float *keys, int32_t count, float t) {
    if (!((long double)t > (long double)keys[0]) && (long double)t == (long double)t)
        return 0;
    int32_t lo = 0, hi = count - 1;
    if (!((long double)t < (long double)keys[4 * (count - 1)]) && (long double)t == (long double)t)
        return (uint32_t)hi;
    while (lo < hi - 1) {
        const int32_t mid = (hi - lo) / 2 + lo;
        if ((long double)t > (long double)keys[4 * mid])
            lo = mid;
        else
            hi = mid;
    }
    return (uint32_t)lo;
}
static volatile long g_key_finds;
static uint32_t __thiscall keys_find_10(const uint8_t *self, float t) {
    g_key_finds++;
    return keys_find(*(const float *const *)(self + 0x10), *(const int32_t *)(self + 0xc), t);
}
static uint32_t __thiscall keys_find_8(const uint8_t *self, float t) {
    g_key_finds++;
    return keys_find(*(const float *const *)(self + 8), *(const int32_t *)(self + 4), t);
}
static const uint8_t keys_find_10_original[] = {0xd9, 0x44, 0x24, 0x04, 0x8b, 0x51, 0x0c, 0x53};
static const uint8_t keys_find_8_original[] = {0xd9, 0x44, 0x24, 0x04, 0x8b, 0x51, 0x04, 0x53};
REWRITE("Keys::Find", 0xe02670, keys_find_10_original, &keys_find_10, &g_key_finds);
REWRITE("Keys::Find2", 0xe025f0, keys_find_8_original, &keys_find_8, &g_key_finds);

// ---- Value::Band2(v) (0x7C3080): which band a value falls in, by fixed limits ------------------------------------------
// 1 when K1 <= v <= K2, 2 when K2 <= v <= K3, 0 when K3 <= v <= K4 (0xF27050, 0xF27058, 0xF71CAC, 0xF71CA8), else 3 (a
// NaN too); in the low byte.
static volatile long g_value_bands;
static uint8_t __stdcall value_band2(float v) {
    g_value_bands++;
    const long double x = v;
    const float *const k1 = (const float *)0xf27050, *const k2 = (const float *)0xf27058, *const k3 = (const float *)0xf71cac, *const k4 = (const float *)0xf71ca8;
    if (x >= (long double)*k1 && x <= (long double)*k2) return 1;
    if (x >= (long double)*k2 && x <= (long double)*k3) return 2;
    if (x >= (long double)*k3 && x <= (long double)*k4) return 0;
    return 3;
}
static const uint8_t value_band2_original[] = {0xd9, 0x44, 0x24, 0x04, 0xd8, 0x1d, 0x50, 0x70};
REWRITE("Value::Band2", 0x7c3080, value_band2_original, &value_band2, &g_value_bands);

// ---- Entries::FindMatching(object, list, out) (0xB78630): the entry whose +0xC and +0x10 match the object's ---------------
// Entries of 0x70 bytes (*list, count list[2]); the first match's first value goes to *out (through the FPU) and 1 is
// given (in the low byte), else 0.
static volatile long g_matching_finds;
static uint8_t __stdcall entries_find_matching(const uint8_t *object, const uint32_t *list, float *out) {
    g_matching_finds++;
    const uint8_t *const base = (const uint8_t *)(uintptr_t)list[0];
    const int32_t count = (int32_t)list[2];
    for (int32_t i = 0; i < count; i++) {
        const uint8_t *const e = base + i * 0x70;
        if (*(const uint32_t *)(object + 0x10) == *(const uint32_t *)(e + 0x10) && *(const uint32_t *)(object + 0xc) == *(const uint32_t *)(e + 0xc)) {
            *(volatile float *)out = (float)x87_load((const float *)e);
            return 1;
        }
    }
    return 0;
}
static const uint8_t entries_find_matching_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Entries::FindMatching", 0xb78630, entries_find_matching_original, &entries_find_matching, &g_matching_finds);

// ---- Sizes::Pick(base, scale) (0x9D9C40): a size in pixels: one of three bases times one of three scales -----------------
// Bases 0, 1, 2: 0xF6C844, 0xF851C4, 0xF851C8 (others 0); scales 1: 0.5, 2: 0xF28420, others 1; fistp, low 32 bits.
static volatile long g_size_picks;
static uint32_t __fastcall sizes_pick(uint32_t base, uint32_t scale) {
    g_size_picks++;
    const long double b = base == 0 ? *(const float *)0xf6c844 : base == 1 ? *(const float *)0xf851c4 : base == 2 ? *(const float *)0xf851c8 : *(const float *)0xf264e0;
    const long double s = scale == 1 ? *(const float *)0xf2651c : scale == 2 ? *(const float *)0xf28420 : *(const float *)0xf264dc;
    return (uint32_t)x87_whole(s * b);
}
static const uint8_t sizes_pick_original[] = {0xd9, 0x05, 0xe0, 0x64, 0xf2, 0x00, 0x8b, 0xc1};
REWRITE("Sizes::Pick", 0x9d9c40, sizes_pick_original, &sizes_pick, &g_size_picks);

// ---- Guest::AddValue759690(this, a, b) (0x759690): a added to +0x40C and b to +0x41C, each kept between 0 and 1 ----------
// The sum is stored; past 1 (unrounded) it becomes 1, below 0 it becomes 0 (a NaN stays).
static volatile long g_guest_values;
static inline void guest_add_kept(volatile float *v, float d) {
    const long double s = (long double)d + *v;
    *v = (float)s;
    if (s > (long double)*(const float *)0xf264dc)
        *(volatile uint32_t *)v = 0x3f800000;
    else if (s < (long double)*(const float *)0xf264e0)
        *(volatile uint32_t *)v = 0;
}
static void __thiscall guest_add_value_759690(uint8_t *self, float a, float b) {
    g_guest_values++;
    guest_add_kept((volatile float *)(self + 0x40c), a);
    guest_add_kept((volatile float *)(self + 0x41c), b);
}
static const uint8_t guest_add_value_759690_original[] = {0xd9, 0x44, 0x24, 0x04, 0x33, 0xd2, 0xd8, 0x81};
REWRITE("Guest::AddValue759690", 0x759690, guest_add_value_759690_original, &guest_add_value_759690, &g_guest_values);

// ---- Vertices::Gather / Gather2(positions, normals, n, out, source, offset) (0xCCF9F0, 0xCCF970): n vertices of 32 bytes --
// For each source entry (12 bytes: two 16-bit indices, u, v): the position at positions + 16 i0 and the normal at
// normals + 16 i1 (three values each, as they are), then u + offset[0] and v + offset[1]. (n is at least 1.)
static volatile long g_vertex_gathers;
static void __fastcall vertices_gather(const uint8_t *positions, const uint8_t *normals, uint32_t n, uint32_t *out, const uint8_t *source, const float *offset) {
    g_vertex_gathers++;
    const long double ou = x87_load(&offset[0]), ov = x87_load(&offset[1]);
    do {
        volatile uint32_t *const o = out;
        const uint32_t *const p = (const uint32_t *)(positions + *(const uint16_t *)source * 16);
        o[0] = p[0], o[1] = p[1], o[2] = p[2];
        const uint32_t *const q = (const uint32_t *)(normals + *(const uint16_t *)(source + 2) * 16);
        o[3] = q[0], o[4] = q[1], o[5] = q[2];
        const long double u = (long double)*(const float *)(source + 4) + ou;
        const long double v = (long double)*(const float *)(source + 8) + ov;
        ((volatile float *)out)[6] = (float)u;
        ((volatile float *)out)[7] = (float)v;
        source += 12, out += 8;
    } while (--n);
}
static const uint8_t vertices_gather_original[] = {0x55, 0x8b, 0xec, 0x83, 0xec, 0x08, 0x53, 0x56};
REWRITE("Vertices::Gather", 0xccf9f0, vertices_gather_original, &vertices_gather, &g_vertex_gathers);
REWRITE("Vertices::Gather2", 0xccf970, vertices_gather_original, &vertices_gather, &g_vertex_gathers);

// ---- Curve::Falloff(kind, -, v, limit) (0x8AE2F0): a value past a limit, eased back by its kind -------------------------
// d = v - limit. Kind 0: v when below the limit, else limit - d / 2; 1: v when below, else limit - d; 2: limit - d d;
// 3: limit - d (d / limit); others 0. Unrounded on the FPU.
static volatile long g_falloffs;
static long double __fastcall curve_falloff(uint32_t kind, uint32_t, float v, float limit) {
    g_falloffs++;
    const long double d = (long double)v - limit;
    switch (kind) {
    case 0: return (long double)v < (long double)limit ? (long double)v : (long double)limit - d * *(const float *)0xf2651c;
    case 1: return (long double)v < (long double)limit ? (long double)v : (long double)limit - d;
    case 2: return (long double)limit - d * d;
    case 3: return (long double)limit - d * (d / limit);
    default: return *(const float *)0xf264e0;
    }
}
static const uint8_t curve_falloff_original[] = {0x83, 0xf9, 0x03, 0x77, 0x7a, 0xff, 0x24, 0x8d};
REWRITE("Curve::Falloff", 0x8ae2f0, curve_falloff_original, &curve_falloff, &g_falloffs);

// ---- Guest::Band(v) (0xA883A0): which band a value falls in once divided by the game's unit (0x13CDF20) -----------------
// q = v / unit: below 1 (unrounded) 0; then, rounded, below 2: 1, below 3: 2, below 4: 3, below 5: 4; otherwise (a NaN
// too) 5.
static volatile long g_guest_bands;
static uint32_t __stdcall guest_band(float v) {
    g_guest_bands++;
    const long double q = (long double)v / *(const volatile float *)0x13cdf20;
    if (q < (long double)*(const float *)0xf264dc) return 0;
    const volatile float qf = (float)q;
    if ((long double)qf < (long double)*(const float *)0xf2705c) return 1;
    if ((long double)qf < (long double)*(const float *)0xf2784c) return 2;
    if ((long double)qf < (long double)*(const float *)0xf2669c) return 3;
    if ((long double)qf < (long double)*(const float *)0xf28214) return 4;
    return 5;
}
static const uint8_t guest_band_original[] = {0xd9, 0x44, 0x24, 0x04, 0xd8, 0x35, 0x20, 0xdf};
REWRITE("Guest::Band", 0xa883a0, guest_band_original, &guest_band, &g_guest_bands);

// ---- Bird::ClampA0A340(this) (0xA0A340): three values (+0xB0 to +0xB8) cleared when outside fixed bounds ----------------
// Cleared when below 0xF6FD64 (or a NaN) or above 0xF6FD60.
static volatile long g_bird_clamps;
static void __thiscall bird_clamp_a0a340(uint8_t *self) {
    g_bird_clamps++;
    for (int i = 0; i < 3; i++) {
        volatile float *const v = (volatile float *)(self + 0xb0) + i;
        if (!((long double)*v >= (long double)*(const float *)0xf6fd64) || (long double)*v > (long double)*(const float *)0xf6fd60)
            *(volatile uint32_t *)v = 0;
    }
}
static const uint8_t bird_clamp_a0a340_original[] = {0xd9, 0x81, 0xb0, 0x00, 0x00, 0x00, 0x33, 0xd2};
REWRITE("Bird::ClampA0A340", 0xa0a340, bird_clamp_a0a340_original, &bird_clamp_a0a340, &g_bird_clamps);

// ---- Car::Angle5294A0(a, b, c, out) (0x5294A0): two angle differences from c, compared ------------------------------------
// d1 = a - c and d2 = b - c, each less a turn when past half a turn (unrounded); *out = d1. Gives (in the low byte) 1 when
// |d1| is below 0xF37A78, else whether d1 and d2 differ in being at least 0 (a NaN counts as not).
static volatile long g_car_angles;
static uint8_t __stdcall car_angle_5294a0(float a, float b, float c, float *out) {
    g_car_angles++;
    const long double half = *(const float *)0xf285f0, turn = *(const float *)0xf268dc;
    long double d1 = (long double)a - c;
    if (d1 > half) d1 = d1 - turn;
    *(volatile float *)out = (float)d1;
    if (x87_abs(d1) < (long double)*(const float *)0xf37a78)
        return 1;
    long double d2 = (long double)b - c;
    if (d2 > half) d2 = d2 - turn;
    const bool s2 = d2 >= (long double)*(const float *)0xf264e0, s1 = d1 >= (long double)*(const float *)0xf264e0;
    return s1 != s2;
}
static const uint8_t car_angle_5294a0_original[] = {0xd9, 0x44, 0x24, 0x04, 0xd8, 0x64, 0x24, 0x0c};
REWRITE("Car::Angle5294A0", 0x5294a0, car_angle_5294a0_original, &car_angle_5294a0, &g_car_angles);

// ---- Pairs::SortOnce(this) (0x940AE0): a list of pairs (8 bytes, at +0, count +8) sorted by their second value, once -----
// Marked sorted at byte +0xC first. For each place i, each later pair whose value is below i's (ordered) is swapped in.
static volatile long g_pair_sorts;
static void __thiscall pairs_sort_once(uint8_t *self) {
    g_pair_sorts++;
    if (self[0xc])
        return;
    ((volatile uint8_t *)self)[0xc] = 1;
    for (int32_t i = 0; i < *(const volatile int32_t *)(self + 8); i++)
        for (int32_t j = i + 1; j < *(const volatile int32_t *)(self + 8); j++) {
            volatile uint32_t *const p = *(volatile uint32_t *const *)self;
            if ((long double)((const volatile float *)p)[2 * i + 1] > (long double)((const volatile float *)p)[2 * j + 1]) {
                const uint32_t a = p[2 * i], v = p[2 * i + 1];
                p[2 * i] = p[2 * j];
                p[2 * i + 1] = p[2 * j + 1];
                p[2 * j] = a;
                p[2 * j + 1] = v;
            }
        }
}
static const uint8_t pairs_sort_once_original[] = {0x51, 0x8a, 0x41, 0x0c, 0x84, 0xc0, 0x75, 0x79};
REWRITE("Pairs::SortOnce", 0x940ae0, pairs_sort_once_original, &pairs_sort_once, &g_pair_sorts);

// ---- FrameTime::Smooth(this, dt) (0x4141E0): the frame time *dt smoothed ---------------------------------------------------
// With a cap (+0x11C not 0): *dt doubled, and the cap when past it. Without: once the average is set (byte +0x110), *dt
// becomes it (+0x118); before that the frames are counted (+0x10C) and added up (+0x114), and past 100 the average is
// the total times 0xF2704C and marked set.
static volatile long g_frame_smooths;
static void __thiscall frame_time_smooth(uint8_t *self, float *dt) {
    g_frame_smooths++;
    const float cap = *(const float *)(self + 0x11c);
    if (!((long double)*(const float *)0xf264e0 == (long double)cap)) {
        const long double v = (long double)*dt + *dt;
        *(volatile float *)dt = (float)v;
        if (v > (long double)cap)
            *(volatile uint32_t *)dt = *(const uint32_t *)&cap;
        return;
    }
    if (self[0x110]) {
        *(volatile uint32_t *)dt = *(const uint32_t *)(self + 0x118);
        return;
    }
    volatile int32_t *const frames = (volatile int32_t *)(self + 0x10c);
    const int32_t n = *frames + 1;
    *frames = n;
    volatile float *const total = (volatile float *)(self + 0x114);
    *total = (float)((long double)*dt + *total);
    if (n > 100) {
        ((volatile uint8_t *)self)[0x110] = 1;
        *(volatile float *)(self + 0x118) = (float)((long double)*total * *(const float *)0xf2704c);
    }
}
static const uint8_t frame_time_smooth_original[] = {0xd9, 0x05, 0xe0, 0x64, 0xf2, 0x00, 0xd9, 0x81};
REWRITE("FrameTime::Smooth", 0x4141e0, frame_time_smooth_original, &frame_time_smooth, &g_frame_smooths);

// ---- Grid::Contains(this, p) (0x493930): whether a point falls in a cell of a grid ----------------------------------------
// Column floor((x - +0xC) / +0x14) and row floor((y - +0x10) / +0x18) (each quotient rounded, rounded down, made whole
// by fistp and cut to its low byte): 1 when below the width (byte +4) and height (byte +5).
static volatile long g_grid_contains;
static uint32_t __thiscall grid_contains(const uint8_t *self, const float *p) {
    g_grid_contains++;
    const float *const f = (const float *)self;
    const volatile float qx = (float)(((long double)p[0] - f[3]) / f[5]);
    long double whole = x87_round(qx);
    const volatile float fx = (float)((long double)qx < whole ? whole - 1.0L : whole);
    const volatile float qy = (float)(((long double)p[1] - f[4]) / f[6]);
    whole = x87_round(qy);
    const volatile float fy = (float)((long double)qy < whole ? whole - 1.0L : whole);
    const uint32_t row = (uint8_t)x87_whole(fy), column = (uint8_t)x87_whole(fx);
    return column < self[4] && row < self[5];
}
static const uint8_t grid_contains_original[] = {0x83, 0xec, 0x10, 0x8b, 0x44, 0x24, 0x14, 0xd9};
REWRITE("Grid::Contains", 0x493930, grid_contains_original, &grid_contains, &g_grid_contains);

// ---- Terrain::LodDistance(this) (0x8F35B0): a terrain patch's distance for detail -----------------------------------------
// With the patch (*this): a = width (byte +4) times the cell width (+0x14), b = depth (byte +5) times the cell depth
// (+0x18), both rounded; m = the smaller cell side (the depth unless the width is below it) times (1 << level (+4) - 1),
// at most 0xF27370; gives the smaller of a and b (b unless a is below it) halved, plus m, unrounded.
static volatile long g_lod_distances;
static long double __thiscall terrain_lod_distance(const uint32_t *self) {
    g_lod_distances++;
    const uint8_t *const patch = (const uint8_t *)(uintptr_t)self[0];
    const float cw = *(const float *)(patch + 0x14), cd = *(const float *)(patch + 0x18);
    const volatile float a = (float)((long double)patch[4] * cw), b = (float)((long double)patch[5] * cd);
    long double m = (long double)cw < (long double)cd ? (long double)cw : (long double)cd;
    m = m * (long double)(int32_t)((1u << (self[1] & 31)) - 1);
    if (m > (long double)*(const float *)0xf27370)
        m = *(const float *)0xf27370;
    const long double half = *(const float *)0xf2651c;
    return (long double)a < (long double)b ? m + (long double)a * half : m + (long double)b * half;
}
static const uint8_t terrain_lod_distance_original[] = {0x83, 0xec, 0x14, 0x8b, 0x01, 0x8b, 0x50, 0x18};
REWRITE("Terrain::LodDistance", 0x8f35b0, terrain_lod_distance_original, &terrain_lod_distance, &g_lod_distances);

// ---- Condition::Test(this, v) (0xBB5880): a value tested against a condition's number (+8) by its operator (+0) --------
// 2: equal, 3: not equal, 4: less, 5: at most, 6: more, 7: at least (a NaN passes only "not equal"); 1 or 0. Any other
// operator gives the operator less 2 with its low byte cleared.
static volatile long g_condition_tests;
static uint32_t __thiscall condition_test(const uint8_t *self, float v) {
    g_condition_tests++;
    const uint32_t op = *(const uint32_t *)self - 2;
    const long double x = v, n = *(const float *)(self + 8);
    switch (op) {
    case 0: return x == n;
    case 1: return !(x == n);
    case 2: return x < n;
    case 3: return x <= n;
    case 4: return x > n;
    case 5: return x >= n;
    default: return op & 0xffffff00u;
    }
}
static const uint8_t condition_test_original[] = {0x8b, 0x01, 0x83, 0xc0, 0xfe, 0x83, 0xf8, 0x05};
REWRITE("Condition::Test", 0xbb5880, condition_test_original, &condition_test, &g_condition_tests);

// ---- PhysicsWorld::AtRest(this) (0xA6EF50): whether every particle of a body is at rest --------------------------------
// For each particle (pointers at +0xC, count +0x10): (position (+0x10) - previous (+0x20)) times 60 (0xF8F980): the x
// difference rounded, y's and z's not; x's and y's products rounded, z's not; vz vz + vx vx + vy vy at most the game's
// limit (0x13C2A04), or a NaN. Gives the answer in the low byte.
static volatile long g_rest_checks;
static uint8_t __thiscall physics_world_at_rest(const uint8_t *self) {
    g_rest_checks++;
    const int32_t n = *(const int32_t *)(self + 0x10);
    const float *const *const parts = *(const float *const *const *)(self + 0xc);
    const long double k = *(const float *)0xf8f980;
    for (int32_t i = 0; i < n; i++) {
        const float *const p = parts[i];
        const volatile float dx = (float)((long double)p[4] - p[8]);
        const long double dy = (long double)p[5] - p[9], dz = (long double)p[6] - p[10];
        const volatile float vx = (float)((long double)dx * k), vy = (float)(dy * k);
        const long double vz = dz * k;
        const long double s = (vz * vz + (long double)vx * vx) + (long double)vy * vy;
        if (s > (long double)*(const volatile float *)0x13c2a04)
            return 0;
    }
    return 1;
}
static const uint8_t physics_world_at_rest_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("PhysicsWorld::AtRest", 0xa6ef50, physics_world_at_rest_original, &physics_world_at_rest, &g_rest_checks);

// ---- Terrain::CellColourAt(this, x, y, h, out) (0xAFA0E0): the colour for a terrain cell at a height -------------------
// *out = 0xFF333333 first. Cells of the grid (width +0x30, depth +0x2C) are layers of 0x2C bytes at +0x14; when the
// cell's layer is flagged (bit 0 of +0x10), the chain (next at +0xC, -1 ends) is walked: the first layer whose bottom
// (+0) is at most h and top (+4) at least h gives its colour (+0x24). Gives (in the low byte) bit 1 of the last layer
// looked at, or 0 outside the grid or for an unflagged cell.
static volatile long g_cell_colours;
static uint8_t __thiscall terrain_cell_colour_at(const uint8_t *self, int32_t x, int32_t y, float h, uint32_t *out) {
    g_cell_colours++;
    *(volatile uint32_t *)out = 0xff333333;
    const int32_t width = *(const int32_t *)(self + 0x30);
    if (x < 0 || y < 0 || x >= width || y >= *(const int32_t *)(self + 0x2c))
        return 0;
    const uint8_t *const layers = *(const uint8_t *const *)(self + 0x14);
    int32_t i = width * y + x;
    if (!(layers[i * 0x2c + 0x10] & 1))
        return 0;
    uint8_t bit = 0;
    while (i != -1) {
        const uint8_t *const e = layers + i * 0x2c;
        bit = (*(const uint32_t *)(e + 0x10) >> 1) & 1;
        if ((long double)*(const float *)e <= (long double)h && (long double)*(const float *)(e + 4) >= (long double)h) {
            *(volatile uint32_t *)out = *(const uint32_t *)(e + 0x24);
            return bit;
        }
        i = *(const int32_t *)(e + 0xc);
    }
    return bit;
}
static const uint8_t terrain_cell_colour_at_original[] = {0x8b, 0x44, 0x24, 0x04, 0x85, 0xc0, 0x53, 0x8b};
REWRITE("Terrain::CellColourAt", 0xafa0e0, terrain_cell_colour_at_original, &terrain_cell_colour_at, &g_cell_colours);

// ---- Segment::Middle(this, out) (0xBC6140): the point halfway between an object's two end points --------------------------
// With the object flagged 0x400000 (+0x18) and at least two stations (+0xE4): the first points of the ends at +0x110 and
// +0x10C (each's +4); the x sum rounded first, y and z not.
static void __thiscall segment_middle(const uint8_t *self, float *out) {
    g_segment_middles++;
    const bool has = (*(const uint32_t *)(self + 0x18) & 0x400000) && *(const int32_t *)(self + 0xe4) >= 2;
    const uint8_t *const end_a = has ? *(const uint8_t *const *)(self + 0x110) : nullptr;
    const float *const a = *(const float *const *)(end_a + 4);
    const uint8_t *const end_b = has ? *(const uint8_t *const *)(self + 0x10c) : nullptr;
    const float *const b = *(const float *const *)(end_b + 4);
    const long double half = *(const float *)0xf2651c;
    const volatile float sx = (float)((long double)b[0] + a[0]);
    const long double sy = (long double)b[1] + a[1], sz = (long double)b[2] + a[2];
    out[0] = (float)((long double)sx * half);
    out[1] = (float)(sy * half);
    out[2] = (float)(sz * half);
}
static const uint8_t segment_middle_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Segment::Middle", 0xbc6140, segment_middle_original, &segment_middle, &g_segment_middles);

// ---- Screen::AreaScale(out) (0x6ACB30): the GUI area's (0x12F83F0) width and height over the screen's; gives out ---------
// (1, 1) without an area or when either side (+0x84 - +0x7C, +0x88 - +0x80, rounded) is not above 0. The screen's
// (*0x12F4618) width (+4) and height (+8) are taken unsigned.
static volatile long g_area_scales;
static float *__fastcall screen_area_scale(float *out) {
    g_area_scales++;
    const float *const area = *(const float *const *)0x12f83f0;
    if (area) {
        const volatile float w = (float)((long double)area[0x84 / 4] - area[0x7c / 4]), h = (float)((long double)area[0x88 / 4] - area[0x80 / 4]);
        if ((long double)w > (long double)*(const float *)0xf264e0 && (long double)h > (long double)*(const float *)0xf264e0) {
            const uint8_t *const screen = *(const uint8_t *const *)0x12f4618;
            const int32_t sw = *(const int32_t *)(screen + 4), sh = *(const int32_t *)(screen + 8);
            long double a = sw;
            if (sw < 0) a += *(const float *)0xf285f8;
            ((volatile float *)out)[0] = (float)((long double)w / a);
            long double b = sh;
            if (sh < 0) b += *(const float *)0xf285f8;
            ((volatile float *)out)[1] = (float)((long double)h / b);
            return out;
        }
    }
    ((volatile uint32_t *)out)[0] = 0x3f800000;
    ((volatile uint32_t *)out)[1] = 0x3f800000;
    return out;
}
static const uint8_t screen_area_scale_original[] = {0xa1, 0xf0, 0x83, 0x2f, 0x01, 0x83, 0xec, 0x0c};
REWRITE("Screen::AreaScale", 0x6acb30, screen_area_scale_original, &screen_area_scale, &g_area_scales);

// ---- Particles::SetCount(this, f) (0x9CBBB0): the number of particles to draw (+0xBC) and their spacing (+0xC0) ----------
// n = +0x64, times the detail setting (the float at *(+0xA4) + 0x90; fistp) when +0x68 is set. With a fade g (+0xB8,
// at most 1) above 0: n ((1 - g) + g f) (fistp), but n itself when n is at most 1 and that came out below it; otherwise
// n. The spacing is +0xA8 over the count.
static volatile long g_particle_counts;
static void __thiscall particles_set_count(uint8_t *self, float f) {
    g_particle_counts++;
    int32_t n = *(const int32_t *)(self + 0x64);
    if (*(const uint32_t *)(self + 0x68))
        n = (int32_t)(uint32_t)x87_whole((long double)n * *(const float *)(*(const uint8_t *const *)(self + 0xa4) + 0x90));
    volatile int32_t *const count = (volatile int32_t *)(self + 0xbc);
    const float fade = *(const float *)(self + 0xb8);
    if ((long double)fade > (long double)*(const float *)0xf264e0) {
        const long double one = *(const float *)0xf264dc;
        const long double g = (long double)fade > one ? one : (long double)fade;
        const int32_t m = (int32_t)(uint32_t)x87_whole(((one - g) + g * f) * (long double)n);
        *count = m;
        if (!(n > 1) && m < n)
            *count = n;
    } else
        *count = n;
    *(volatile float *)(self + 0xc0) = (float)((long double)*(const float *)(self + 0xa8) / (long double)*count);
}
static const uint8_t particles_set_count_original[] = {0x83, 0xec, 0x08, 0x8b, 0x41, 0x68, 0x85, 0xc0};
REWRITE("Particles::SetCount", 0x9cbbb0, particles_set_count_original, &particles_set_count, &g_particle_counts);

// ---- Ease::InOut(t, total) (0x5D3850): a time eased over a total, slow at both ends ------------------------------------------
// A total under 2 gives t back. t under 2: t t / 4 / (total - 2) total; within the last 2 (t past total - 2):
// ((total - 2) - (total - t)^2 / 4) / (total - 2) total; otherwise (t - 1) / (total - 2) total; unrounded.
static long double __stdcall ease_in_out(float t, float total) {
    g_eases++;
    const long double two = *(const float *)0xf2705c, one = *(const float *)0xf264dc, quarter = *(const float *)0xf270b8;
    if ((long double)total < two)
        return t;
    if ((long double)t < two)
        return (long double)t * t * quarter / ((long double)total - two) * total;
    const long double e = (long double)total - two;
    if ((long double)t > e) {
        const long double a = (long double)total - t;
        return (e - a * a * quarter) / ((long double)total - two) * total;
    }
    return ((long double)t - one) / ((long double)total - two) * total;
}
static const uint8_t ease_in_out_original[] = {0xd9, 0x44, 0x24, 0x08, 0xd8, 0x1d, 0x5c, 0x70};
REWRITE("Ease::InOut", 0x5d3850, ease_in_out_original, &ease_in_out, &g_eases);

// ---- SinCos::Lookup(table, angle, a, b) (0x7C4690): sine and cosine from a quarter-turn table of 256 values --------------
// i = angle K (0xF6D450; fistp) & 0x3FF, lo = i & 0xFF, hi = 255 - lo. Quadrant 0: (t[lo], t[hi]); 1: (t[hi], -t[lo]);
// 2: (-t[lo], -t[hi]); 3: (-t[hi], t[lo]). Values copied as they are, negated ones through the FPU.
static volatile long g_sin_cos;
static void __thiscall sin_cos_lookup(const uint32_t *table, float angle, uint32_t *a, uint32_t *b) {
    g_sin_cos++;
    const uint32_t i = (uint32_t)x87_whole((long double)angle * *(const float *)0xf6d450) & 0x3ff;
    const uint32_t lo = i & 0xff, hi = 0xff - lo;
    switch (i & 0x300) {
    case 0: *(volatile uint32_t *)a = table[lo], *(volatile uint32_t *)b = table[hi]; break;
    case 0x100:
        *(volatile uint32_t *)a = table[hi];
        *(volatile float *)b = (float)-x87_load((const float *)&table[lo]);
        break;
    case 0x200:
        *(volatile float *)a = (float)-x87_load((const float *)&table[lo]);
        *(volatile float *)b = (float)-x87_load((const float *)&table[hi]);
        break;
    default:
        *(volatile float *)a = (float)-x87_load((const float *)&table[hi]);
        *(volatile uint32_t *)b = table[lo];
        break;
    }
}
static const uint8_t sin_cos_lookup_original[] = {0x83, 0xec, 0x08, 0xd9, 0x44, 0x24, 0x0c, 0x56};
REWRITE("SinCos::Lookup", 0x7c4690, sin_cos_lookup_original, &sin_cos_lookup, &g_sin_cos);

// ---- Entries::CountNegative(this) (0x46C6C0): how many entries have a value below 0 at +0x40 -----------------------------
// Entries of 0x50 bytes at +0x6C, count +0x70 (a NaN does not count).
static volatile long g_negative_counts;
static uint32_t __thiscall entries_count_negative(const uint8_t *self) {
    g_negative_counts++;
    const int32_t n = *(const int32_t *)(self + 0x70);
    const uint8_t *const e = *(const uint8_t *const *)(self + 0x6c);
    uint32_t count = 0;
    for (int32_t i = 0; i < n; i++)
        if ((long double)*(const float *)(e + i * 0x50 + 0x40) < (long double)*(const float *)0xf264e0)
            count++;
    return count;
}
static const uint8_t entries_count_negative_original[] = {0x53, 0x55, 0x57, 0x8b, 0x79, 0x70, 0x33, 0xed};
REWRITE("Entries::CountNegative", 0x46c6c0, entries_count_negative_original, &entries_count_negative, &g_negative_counts);

// ---- Box::ClosestPoint(out, min, max, p) (0x88CFD0): the point of a box nearest p; gives out ---------------------------------
// Each coordinate (z, then y, then x): max's when p is above it, else min's when p is below it, else p's (through the FPU).
static volatile long g_closest_points;
static float *__fastcall box_closest_point(float *out, const float *min, const float *max, const float *p) {
    g_closest_points++;
    long double c[3];
    for (int i = 2; i >= 0; i--)
        c[i] = (long double)p[i] > (long double)max[i] ? x87_load(&max[i]) : (long double)p[i] < (long double)min[i] ? x87_load(&min[i]) : x87_load(&p[i]);
    ((volatile float *)out)[0] = (float)c[0];
    ((volatile float *)out)[1] = (float)c[1];
    ((volatile float *)out)[2] = (float)c[2];
    return out;
}
static const uint8_t box_closest_point_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Box::ClosestPoint", 0x88cfd0, box_closest_point_original, &box_closest_point, &g_closest_points);

// ---- Angle::TurnToward(this, target, step, tolerance) (0x7C28A0): an angle (+0x110) turned toward the target ------------
// d = target - angle (unrounded), brought into -pi to pi (a turn added below 0xF37CFC, taken off above pi). Within the
// tolerance: nothing, 0 (in the low byte). Past +step: the angle moves by +step; below -step: by -step; then 1 unless d
// is exactly 0. Otherwise the angle takes d and 0.
static volatile long g_turn_towards;
static uint8_t __thiscall angle_turn_toward(uint8_t *self, float target, float step, float tolerance) {
    g_turn_towards++;
    volatile float *const angle = (volatile float *)(self + 0x110);
    long double d = (long double)target - *angle;
    if (d < (long double)*(const float *)0xf37cfc)
        d = d + *(const float *)0xf268dc;
    else if (d > (long double)*(const float *)0xf285f0)
        d = d - *(const float *)0xf268dc;
    if (!(x87_abs(d) > (long double)tolerance) && tolerance == tolerance && d == d)
        return 0;
    if (d > (long double)step)
        *angle = (float)((long double)step + *angle);
    else if (d < -(long double)step)
        *angle = (float)((long double)*angle - step);
    else {
        *angle = (float)(d + *angle);
        return 0;
    }
    return !((long double)*(const float *)0xf264e0 == d);
}
static const uint8_t angle_turn_toward_original[] = {0xd9, 0x44, 0x24, 0x04, 0xd8, 0xa1, 0x10, 0x01};
REWRITE("Angle::TurnToward", 0x7c28a0, angle_turn_toward_original, &angle_turn_toward, &g_turn_towards);

// ---- Box::Overshoot2D(out, p, min, max) (0x9F8340): how far a point lies outside a box ------------------------------------
// Each part: p - min when below the minimum, p - max when above the maximum, else 0 (x and y rounded, z not).
static volatile long g_overshoots;
static void __stdcall box_overshoot_2d(float *out, const float *p, const float *min, const float *max) {
    g_overshoots++;
    long double o[3];
    for (int i = 0; i < 3; i++) {
        if ((long double)p[i] < (long double)min[i]) o[i] = (long double)p[i] - min[i];
        else if ((long double)p[i] > (long double)max[i]) o[i] = (long double)p[i] - max[i];
        else o[i] = *(const float *)0xf264e0;
    }
    const volatile float ox = (float)o[0], oy = (float)o[1];
    ((volatile float *)out)[0] = ox;
    ((volatile uint32_t *)out)[1] = *(const volatile uint32_t *)&oy;
    ((volatile float *)out)[2] = (float)o[2];
}
static const uint8_t box_overshoot_2d_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Box::Overshoot2D", 0x9f8340, box_overshoot_2d_original, &box_overshoot_2d, &g_overshoots);

// ---- Ticker::Update(this) (0x7EA210): a tick counted (+0x410) and a timer (+0x4D8) run down ------------------------------
// Above 0.5 the timer drops by 0.5; below 0.25, +0x4DC drops by a quarter of a step by kind (+0x414): 1: 0.9, 2: 0.7,
// 3: 0.3, others 1.
static volatile long g_ticker_updates;
static void __thiscall ticker_update(uint8_t *self) {
    g_ticker_updates++;
    const long double t = *(const float *)(self + 0x4d8);
    *(volatile int32_t *)(self + 0x410) = *(const int32_t *)(self + 0x410) + 1;
    const long double half = *(const float *)0xf2651c, quarter = *(const float *)0xf270b8;
    if (t > half) {
        *(volatile float *)(self + 0x4d8) = (float)(t - half);
        return;
    }
    if (!(t < quarter))
        return;
    uint32_t at;
    switch (*(const uint32_t *)(self + 0x414)) {
    case 1: at = 0xf29640; break;
    case 2: at = 0xf27ca4; break;
    case 3: at = 0xf27ca8; break;
    default: at = 0xf264dc; break;
    }
    volatile float *const v = (volatile float *)(self + 0x4dc);
    *v = (float)((long double)*v - (long double)*(const float *)(uintptr_t)at * quarter);
}
static const uint8_t ticker_update_original[] = {0xd9, 0x81, 0xd8, 0x04, 0x00, 0x00, 0x8b, 0x81};
REWRITE("Ticker::Update", 0x7ea210, ticker_update_original, &ticker_update, &g_ticker_updates);

// ---- Polygon::Area(this) (0x90C170): the area of a polygon by the shoelace sum -------------------------------------------
// Points of 8 bytes (x, y) at +0x24, count +0x28: the sum over i of ((y[i-1] + y[i]) / 2) (x[i] - x[i-1]), made positive.
static volatile long g_polygon_areas;
static long double __thiscall polygon_area(const uint8_t *self) {
    g_polygon_areas++;
    const int32_t n = *(const int32_t *)(self + 0x28);
    const float *const p = *(const float *const *)(self + 0x24);
    long double sum = *(const float *)0xf264e0;
    for (int32_t i = 1; i < n; i++)
        sum = sum + ((long double)p[2 * i - 1] + p[2 * i + 1]) * *(const float *)0xf2651c * ((long double)p[2 * i] - p[2 * i - 2]);
    return x87_abs(sum);
}
static const uint8_t polygon_area_original[] = {0xd9, 0x05, 0xe0, 0x64, 0xf2, 0x00, 0x56, 0x8b};
REWRITE("Polygon::Area", 0x90c170, polygon_area_original, &polygon_area, &g_polygon_areas);

// ---- Entries::Highest(this) (0xC35E50): the entry whose second value is the largest, or null --------------------------------
// Entries of 12 bytes at +0xA0 (count +0xB4); the first of equal ones is kept, a NaN never wins after the first.
static volatile long g_highest;
static const uint8_t *__thiscall entries_highest(const uint8_t *self) {
    g_highest++;
    const int32_t n = *(const int32_t *)(self + 0xb4);
    const uint8_t *const e = *(const uint8_t *const *)(self + 0xa0);
    const uint8_t *best = nullptr;
    for (int32_t i = 0; i < n; i++)
        if (!best || (long double)*(const float *)(e + i * 12 + 4) > (long double)*(const float *)(best + 4))
            best = e + i * 12;
    return best;
}
static const uint8_t entries_highest_original[] = {0x53, 0x8b, 0x99, 0xb4, 0x00, 0x00, 0x00, 0x55};
REWRITE("Entries::Highest", 0xc35e50, entries_highest_original, &entries_highest, &g_highest);

// ---- VerletPoints::AllSettled(this, speed) (0xA6EFF0): whether every point of a set is nearly still ----------------------
// As PhysicsWorld::AtRest (the squared speed vz vz + vx vx + vy vy against the game's limit 0x13C2A04), but the first
// point past the limit has its squared speed put in *speed (0 in the low byte); 1 when none is.
static uint8_t __thiscall verlet_points_all_settled(const uint8_t *self, float *speed) {
    g_rest_checks++;
    const int32_t n = *(const int32_t *)(self + 0x10);
    const float *const *const parts = *(const float *const *const *)(self + 0xc);
    const long double k = *(const float *)0xf8f980;
    for (int32_t i = 0; i < n; i++) {
        const float *const p = parts[i];
        const volatile float dx = (float)((long double)p[4] - p[8]);
        const long double dy = (long double)p[5] - p[9], dz = (long double)p[6] - p[10];
        const volatile float vx = (float)((long double)dx * k), vy = (float)(dy * k);
        const long double vz = dz * k;
        const long double s = (vz * vz + (long double)vx * vx) + (long double)vy * vy;
        if (s > (long double)*(const volatile float *)0x13c2a04) {
            *(volatile float *)speed = (float)s;
            return 0;
        }
    }
    return 1;
}
static const uint8_t verlet_points_all_settled_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("VerletPoints::AllSettled", 0xa6eff0, verlet_points_all_settled_original, &verlet_points_all_settled, &g_rest_checks);

// ---- Objects::AnyNear(p) (0x7A2180): whether any object of the game's list (0x10EFB90) is closer than 0xF28218 to p ------
// Nodes (next +0, object +8) from the list's first, while the node's next is not null; the object's place at +0x80:
// sqrt((dz dz + dy dy) + dx dx) with the x difference rounded. Gives the answer in the low byte.
static volatile long g_any_nears;
static uint8_t __fastcall objects_any_near(const float *p) {
    g_any_nears++;
    const uint32_t *node = *(const uint32_t *const *)0x10efb90;
    if (!*node || !node)
        return 0;
    for (;;) {
        const float *const o = (const float *)((const uint8_t *)(uintptr_t)node[2] + 0x80);
        const volatile float dx = (float)((long double)o[0] - p[0]);
        const long double dy = (long double)o[1] - p[1], dz = (long double)o[2] - p[2];
        if (x87_sqrt((dz * dz + dy * dy) + (long double)dx * dx) < (long double)*(const float *)0xf28218)
            return 1;
        node = (const uint32_t *)(uintptr_t)*node;
        if (!*node || !node)
            return 0;
    }
}
static const uint8_t objects_any_near_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Objects::AnyNear", 0x7a2180, objects_any_near_original, &objects_any_near, &g_any_nears);

// ---- Box::ThinnestAxis(min, max) (0x80E090): which axis a box is thinnest along, as the game numbers them -------------------
// dx (rounded), dy (unrounded) and dz (rounded) the sides: 2 when dx is below both others, 1 when dy is, else 0.
static volatile long g_thinnest_axes;
static uint32_t __fastcall box_thinnest_axis(const float *min, const float *max) {
    g_thinnest_axes++;
    const volatile float dx = (float)((long double)max[0] - min[0]);
    const long double dy = (long double)max[1] - min[1];
    const volatile float dz = (float)((long double)max[2] - min[2]);
    if ((long double)dx < dy && (long double)dx < (long double)dz) return 2;
    if (dy < (long double)dx && dy < (long double)dz) return 1;
    return 0;
}
static const uint8_t box_thinnest_axis_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("Box::ThinnestAxis", 0x80e090, box_thinnest_axis_original, &box_thinnest_axis, &g_thinnest_axes);

// ---- Vector::Compare(a, b) (0xBFB2B0): a sort comparison of two vectors, x then y then z: -1, 0 or 1 ---------------------------
// Below gives -1; not equal (a NaN too) 1.
static volatile long g_vector_compares;
static int32_t __thiscall vector_compare(const float *a, const float *b) {
    g_vector_compares++;
    for (int i = 0; i < 3; i++) {
        if ((long double)a[i] < (long double)b[i]) return -1;
        if (!((long double)a[i] == (long double)b[i])) return 1;
    }
    return 0;
}
static const uint8_t vector_compare_original[] = {0xd9, 0x01, 0x8b, 0x54, 0x24, 0x04, 0xd9, 0x02};
REWRITE("Vector::Compare", 0xbfb2b0, vector_compare_original, &vector_compare, &g_vector_compares);

// ---- SortedIndex::LowerBound(this, entry) (0x80E3A0): the place for an entry's +8 in an index sorted high to low ------------
// Indices at +0x44 (count +0x48) of 0x1C-byte entries at +0x40: halving; a value above the key moves lo past the middle, a
// value below moves hi before it, an equal value (or a NaN) ends the search; gives lo.
static volatile long g_lower_bounds;
static int32_t __thiscall sorted_index_lower_bound(const uint8_t *self, const float *entry) {
    g_lower_bounds++;
    const int32_t *const index = *(const int32_t *const *)(self + 0x44);
    const uint8_t *const entries = *(const uint8_t *const *)(self + 0x40);
    int32_t lo = 0, hi = *(const int32_t *)(self + 0x48) - 1;
    while (lo <= hi) {
        const int32_t mid = (hi - lo) / 2 + lo;
        const long double v = *(const float *)(entries + index[mid] * 0x1c + 8), k = entry[2];
        if (k < v)
            lo = mid + 1;
        else if (k > v)
            hi = mid - 1;
        else
            break;
    }
    return lo;
}
static const uint8_t sorted_index_lower_bound_original[] = {0x51, 0x56, 0x57, 0x8b, 0x79, 0x48, 0x33, 0xf6};
REWRITE("SortedIndex::LowerBound", 0x80e3a0, sorted_index_lower_bound_original, &sorted_index_lower_bound, &g_lower_bounds);

// ---- Terrain::CellHeightRange(this, x, y, low, high) (0x8BCF80): the lowest and highest corner heights of a cell -----------
// Cells of 24 bytes at +0x24, the grid's width (byte +4); the four heights from the cell's +0, through the FPU.
static volatile long g_height_ranges;
static void __thiscall terrain_cell_height_range(const uint8_t *self, uint32_t x, uint32_t y, float *low, float *high) {
    g_height_ranges++;
    const float *const h = (const float *)(*(const uint8_t *const *)(self + 0x24) + ((uint32_t)self[4] * (uint8_t)y + (uint8_t)x) * 24);
    volatile float lo = (float)x87_load(&h[0]);
    long double hi = x87_load(&h[0]);
    for (int i = 1; i < 4; i++) {
        const long double v = x87_load(&h[i]);
        const volatile float vf = (float)v;
        if (v < (long double)lo)
            lo = (float)v;
        if (v > hi)
            hi = vf;
    }
    *(volatile float *)low = lo;
    *(volatile float *)high = (float)hi;
}
static const uint8_t terrain_cell_height_range_original[] = {0x0f, 0xb6, 0x41, 0x04, 0x0f, 0xb6, 0x54, 0x24};
REWRITE("Terrain::CellHeightRange", 0x8bcf80, terrain_cell_height_range_original, &terrain_cell_height_range, &g_height_ranges);

// ---- SinTable::Sin(table, angle) (0xCD0660): the sine of an angle from a quarter-turn table (256 entries) -------------------
// i = angle K (0xF6D450; fistp) & 0x3FF, lo = i & 0xFF: quadrant 0 t[lo], 1 t[255 - lo], 2 -t[lo], 3 -t[255 - lo].
static long double __thiscall sin_table_sin(const float *table, float angle) {
    g_sin_cos++;
    const uint32_t i = (uint32_t)x87_whole((long double)angle * *(const float *)0xf6d450) & 0x3ff;
    const uint32_t lo = i & 0xff;
    switch (i & 0x300) {
    case 0: return x87_load(&table[lo]);
    case 0x100: return x87_load(&table[0xff - lo]);
    case 0x200: return -x87_load(&table[lo]);
    default: return -x87_load(&table[0xff - lo]);
    }
}
static const uint8_t sin_table_sin_original[] = {0x83, 0xec, 0x08, 0xd9, 0x44, 0x24, 0x0c, 0xd8};
REWRITE("SinTable::Sin", 0xcd0660, sin_table_sin_original, &sin_table_sin, &g_sin_cos);

// ---- Matrix::Ortho(this, a, b, w, h) (0x4664E0): an orthographic projection -------------------------------------------------
// +0 2 / w, +0x14 2 / h, r = 1 / (b - a): +0x28 r and +0x38 r a (r unrounded); +0x3C 1; the rest cleared.
static volatile long g_orthos;
static void __thiscall matrix_ortho(uint8_t *self, float a, float b, float w, float h) {
    g_orthos++;
    const long double two = *(const float *)0xf2705c;
    const long double sx = two / w;
    volatile uint32_t *const t = (volatile uint32_t *)self;
    static const int8_t cleared[] = {0xd, 0xc, 0xb, 9, 8, 7, 6, 4, 3, 2, 1};
    for (const int8_t i : cleared) t[i] = 0;
    t[0xf] = 0x3f800000;
    volatile float *const m = (volatile float *)self;
    m[0] = (float)sx;
    m[5] = (float)(two / h);
    const long double r = (long double)*(const float *)0xf264dc / ((long double)b - a);
    m[10] = (float)r;
    m[14] = (float)(r * a);
}
static const uint8_t matrix_ortho_original[] = {0xd9, 0x05, 0x5c, 0x70, 0xf2, 0x00, 0x33, 0xc0};
REWRITE("Matrix::Ortho", 0x4664e0, matrix_ortho_original, &matrix_ortho, &g_orthos);

// ---- ProgramInfo::MinSize(this, out) (0x621370): the program info window's size (+0xF8, +0xFC), at least 530 by 200 --------
// fcomi / fcmov: a NaN gives the minimum. The width goes out through the FPU, the height as it is.
static volatile long g_min_sizes;
static void __thiscall program_info_min_size(const uint8_t *self, float *out) {
    g_min_sizes++;
    const volatile float w = (float)x87_greater(530.0L, x87_load((const float *)(self + 0xf8)));
    const volatile float h = (float)x87_greater(200.0L, x87_load((const float *)(self + 0xfc)));
    ((volatile float *)out)[0] = (float)x87_load((const float *)&w);
    ((volatile uint32_t *)out)[1] = *(const volatile uint32_t *)&h;
}
static const uint8_t program_info_min_size_original[] = {0x83, 0xec, 0x10, 0x8b, 0x81, 0xf8, 0x00, 0x00};
REWRITE("ProgramInfo::MinSize", 0x621370, program_info_min_size_original, &program_info_min_size, &g_min_sizes);

// ---- Damping::For(speed) (0x5BADA0): a damping amount for a speed -----------------------------------------------------------
// s = speed times the game's scale (0x12F55CC): at least 0xF275F4 (unrounded) gives speed / 2; above 0 (rounded):
// (0.9 - s 0xF3EF64) speed; otherwise speed 0xF3EF60. Unrounded on the FPU.
static volatile long g_dampings;
static long double __stdcall damping_for(float speed) {
    g_dampings++;
    const long double s = (long double)*(const volatile float *)0x12f55cc * speed;
    const volatile float sf = (float)s;
    if (s >= (long double)*(const float *)0xf275f4)
        return (long double)speed * *(const float *)0xf2651c;
    if ((long double)sf > (long double)*(const float *)0xf264e0)
        return ((long double)*(const float *)0xf29640 - (long double)sf * *(const float *)0xf3ef64) * speed;
    return (long double)speed * *(const float *)0xf3ef60;
}
static const uint8_t damping_for_original[] = {0x51, 0xd9, 0x05, 0xcc, 0x55, 0x2f, 0x01, 0xd8};
REWRITE("Damping::For", 0x5bada0, damping_for_original, &damping_for, &g_dampings);

// ---- Counts::AddByLevel(this, v) (0x5BC050): one added to the count of the highest of four levels a value reaches ----------
// At least 0xF3EF7C: +0xC; 0xF3EF78: +8; 0xF3EF74: +4; 0xF3EF70: +0; below all (or a NaN) nothing.
static void __thiscall counts_add_by_level(uint8_t *self, float v) {
    g_kind_counts++;
    static const uint32_t levels[4] = {0xf3ef7c, 0xf3ef78, 0xf3ef74, 0xf3ef70};
    for (int i = 0; i < 4; i++)
        if ((long double)v >= (long double)*(const float *)(uintptr_t)levels[i]) {
            volatile int32_t *const c = (volatile int32_t *)(self + 4 * (3 - i));
            *c = *c + 1;
            return;
        }
}
static const uint8_t counts_add_by_level_original[] = {0xd9, 0x44, 0x24, 0x04, 0xd8, 0x1d, 0x7c, 0xef};
REWRITE("Counts::AddByLevel", 0x5bc050, counts_add_by_level_original, &counts_add_by_level, &g_kind_counts);

// ---- Lower-bound searches over float keys sorted low to high --------------------------------------------------------------
// Halving: a value below the key moves lo past the middle, one above moves hi before it, an equal value (or a NaN) ends
// the search; gives lo. Value(i) reads the i-th value.
template <class Value> static int32_t float_lower_bound(int32_t count, float key, Value value) {
    int32_t lo = 0, hi = count - 1;
    while (lo <= hi) {
        const int32_t mid = (hi - lo) / 2 + lo;
        const long double v = value(mid);
        if ((long double)key > v)
            lo = mid + 1;
        else if ((long double)key < v)
            hi = mid - 1;
        else
            break;
    }
    return lo;
}
// SortedIndex::LowerBound2(this, key) (0x80E200): indices at +0xC into objects at +8 (count +0x10), by their +8
static int32_t __thiscall sorted_index_lower_bound2(const uint8_t *self, float key) {
    g_lower_bounds++;
    const int32_t *const index = *(const int32_t *const *)(self + 0xc);
    const uint8_t *const *const objects = *(const uint8_t *const *const *)(self + 8);
    return float_lower_bound(*(const int32_t *)(self + 0x10), key, [&](int32_t i) { return (long double)*(const float *)(objects[index[i]] + 8); });
}
// SortedTable::LowerBound1C(this, entry) (0x80E340): entries of 0x1C bytes at +0x40 (count +0x48) by their first value
static int32_t __thiscall sorted_table_lower_bound1c(const uint8_t *self, const float *entry) {
    g_lower_bounds++;
    const uint8_t *const table = *(const uint8_t *const *)(self + 0x40);
    return float_lower_bound(*(const int32_t *)(self + 0x48), entry[0], [&](int32_t i) { return (long double)*(const float *)(table + i * 0x1c); });
}
// SortedPointers::LowerBound8(this, key) (0x80E1A0): objects at +8 (count +0x10) by their +8
static int32_t __thiscall sorted_pointers_lower_bound8(const uint8_t *self, float key) {
    g_lower_bounds++;
    const uint8_t *const *const objects = *(const uint8_t *const *const *)(self + 8);
    return float_lower_bound(*(const int32_t *)(self + 0x10), key, [&](int32_t i) { return (long double)*(const float *)(objects[i] + 8); });
}
static const uint8_t sorted_index_lower_bound2_original[] = {0x56, 0x57, 0x8b, 0x79, 0x10, 0x33, 0xf6, 0x4f};
static const uint8_t sorted_table_lower_bound1c_original[] = {0x56, 0x57, 0x8b, 0x79, 0x48, 0x33, 0xf6, 0x4f};
REWRITE("SortedIndex::LowerBound2", 0x80e200, sorted_index_lower_bound2_original, &sorted_index_lower_bound2, &g_lower_bounds);
REWRITE("SortedTable::LowerBound1C", 0x80e340, sorted_table_lower_bound1c_original, &sorted_table_lower_bound1c, &g_lower_bounds);
REWRITE("SortedPointers::LowerBound8", 0x80e1a0, sorted_index_lower_bound2_original, &sorted_pointers_lower_bound8, &g_lower_bounds);

// ---- Items::SizeAt(this, n) (0xC637C0): the width or height of item n of the list at +0xB0, or 0 ------------------------------
// Nodes (next +0, item +8, until a node whose next is null); +0xD0 when bit 2 of +0x1C is set, else +0xD4.
static volatile long g_item_sizes;
static long double __thiscall items_size_at(const uint8_t *self, int32_t n) {
    g_item_sizes++;
    const uint32_t *node = *(const uint32_t *const *)(self + 0xb0);
    if (!*node || !node)
        return *(const float *)0xf264e0;
    while (n) {
        node = (const uint32_t *)(uintptr_t)*node;
        n--;
        if (!*node || !node)
            return *(const float *)0xf264e0;
    }
    const uint8_t *const item = (const uint8_t *)(uintptr_t)node[2];
    if (!item)
        return *(const float *)0xf264e0;
    return x87_load((const float *)(item + (self[0x1c] & 4 ? 0xd0 : 0xd4)));
}
static const uint8_t items_size_at_original[] = {0x8b, 0x81, 0xb0, 0x00, 0x00, 0x00, 0x8b, 0x54};
REWRITE("Items::SizeAt", 0xc637c0, items_size_at_original, &items_size_at, &g_item_sizes);

// ---- Keys::FindTime(this, t) (0x988F60): the key a time falls in, by halving; -1 when there are none ------------------------
// Keys of 32 bytes, time first, at +0x40 (count +0x4C): lo = 0, hi = count; while they are more than 1 apart the middle
// ((hi + lo) / 2) becomes lo when t is past its time, else hi (a NaN too); gives lo.
static int32_t __thiscall keys_find_time(const uint8_t *self, float t) {
    g_key_finds++;
    int32_t hi = *(const int32_t *)(self + 0x4c);
    if (!hi)
        return -1;
    const uint8_t *const keys = *(const uint8_t *const *)(self + 0x40);
    int32_t lo = 0;
    while ((hi - lo < 0 ? lo - hi : hi - lo) > 1) {
        const int32_t mid = (hi + lo) / 2;
        if ((long double)t > (long double)*(const float *)(keys + mid * 32))
            lo = mid;
        else
            hi = mid;
    }
    return lo;
}
static const uint8_t keys_find_time_original[] = {0x56, 0x8b, 0x71, 0x4c, 0x85, 0xf6, 0x75, 0x07};
REWRITE("Keys::FindTime", 0x988f60, keys_find_time_original, &keys_find_time, &g_key_finds);

// ---- Attraction::SpeedScale(this) (0x55FBB0): a speed scale for some attraction kinds (+0x60) ---------------------------------
// Kinds 0x2E to 0x31 and 0x44: the game's scale (0x10EE8F4) times +0x184 (unrounded), at least 0xF268F0 and at most 1;
// other kinds 0.
static volatile long g_attraction_speeds;
static long double __thiscall attraction_speed_scale(const uint8_t *self) {
    g_attraction_speeds++;
    const uint32_t kind = *(const uint32_t *)(self + 0x60);
    if (!((kind >= 0x2e && kind <= 0x31) || kind == 0x44))
        return *(const float *)0xf264e0;
    const long double v = (long double)*(const volatile float *)0x10ee8f4 * *(const float *)(self + 0x184);
    if (v < (long double)*(const float *)0xf268f0) return *(const float *)0xf268f0;
    if (v > (long double)*(const float *)0xf264dc) return *(const float *)0xf264dc;
    return v;
}
static const uint8_t attraction_speed_scale_original[] = {0x8b, 0x41, 0x60, 0x83, 0xf8, 0x2e, 0x74, 0x1b};
REWRITE("Attraction::SpeedScale", 0x55fbb0, attraction_speed_scale_original, &attraction_speed_scale, &g_attraction_speeds);

// ---- Firework::SetFade(this, level) (0x9F7110): a fade value (+0xC) from a level kept between 0 and 1 ------------------------
// u = the level kept (fcomi / fcmov); (u K + 0.5) u + D (K 0xF86BDC, D 0xF355E4).
static volatile long g_firework_fades;
static void __thiscall firework_set_fade(uint8_t *self, float level) {
    g_firework_fades++;
    const volatile float t = (float)x87_greater(0.0L, x87_load(&level));
    const volatile float u = (float)x87_lesser(1.0L, x87_load((const float *)&t));
    *(volatile float *)(self + 0xc) = (float)(((long double)u * *(const float *)0xf86bdc + *(const float *)0xf2651c) * u + *(const float *)0xf355e4);
}
static const uint8_t firework_set_fade_original[] = {0x83, 0xec, 0x08, 0xc7, 0x04, 0x24, 0x00, 0x00};
REWRITE("Firework::SetFade", 0x9f7110, firework_set_fade_original, &firework_set_fade, &g_firework_fades);

// ---- Values::SumFirst(this, n) (0x884680): the sum of the first n values of the list at +0xC, unrounded ---------------------
static volatile long g_value_sums;
static long double __thiscall values_sum_first(const uint8_t *self, int32_t n) {
    g_value_sums++;
    long double sum = *(const float *)0xf264e0;
    const float *const v = *(const float *const *)(self + 0xc);
    for (int32_t i = 0; i < n; i++) sum = sum + v[i];
    return sum;
}
static const uint8_t values_sum_first_original[] = {0xd9, 0x05, 0xe0, 0x64, 0xf2, 0x00, 0x56, 0x8b};
REWRITE("Values::SumFirst", 0x884680, values_sum_first_original, &values_sum_first, &g_value_sums);

// ---- Ride::InRange(this, other) (0x55F940): a flag (byte +0x7C) for a value between two limits ------------------------------
// Without other's flag (byte +0x20) the flag is cleared. Otherwise it is set when other's value (+0x24) lies between
// other's +0x18 and this's +0x70 (either way round, ends included); else left as it is.
static volatile long g_in_ranges;
static void __thiscall ride_in_range(uint8_t *self, const uint8_t *other) {
    g_in_ranges++;
    if (!other[0x20]) {
        ((volatile uint8_t *)self)[0x7c] = 0;
        return;
    }
    const long double v = *(const float *)(other + 0x24), a = *(const float *)(other + 0x18), b = *(const float *)(self + 0x70);
    if ((v >= a && v <= b) || (v >= b && v <= a))
        ((volatile uint8_t *)self)[0x7c] = 1;
}
static const uint8_t ride_in_range_original[] = {0x8b, 0x54, 0x24, 0x04, 0x8a, 0x42, 0x20, 0x84};
REWRITE("Ride::InRange", 0x55f940, ride_in_range_original, &ride_in_range, &g_in_ranges);

// ---- Ranges::FindFrom(this, n, kind, v) (0x8BFF50): from entry n along the chain, an entry of a kind whose range holds v ----
// Entries of 16 bytes at +0 (count +4): kind byte +1, next +4, range +8 to +0xC (ends included). Gives 1 in the low byte
// when found, else 0.
static volatile long g_range_finds;
static uint8_t __thiscall ranges_find_from(const uint32_t *self, uint32_t n, uint32_t kind, float v) {
    g_range_finds++;
    const uint32_t count = self[1];
    const uint8_t *const entries = (const uint8_t *)(uintptr_t)self[0];
    for (uint32_t i = n; i < count;) {
        const uint8_t *const e = entries + i * 16;
        if (e[1] == (uint8_t)kind && (long double)v >= (long double)*(const float *)(e + 8) && (long double)v <= (long double)*(const float *)(e + 0xc))
            return 1;
        i = *(const uint32_t *)(e + 4);
    }
    return 0;
}
static const uint8_t ranges_find_from_original[] = {0x8b, 0x44, 0x24, 0x04, 0x53, 0x56, 0x8b, 0x71};
REWRITE("Ranges::FindFrom", 0x8bff50, ranges_find_from_original, &ranges_find_from, &g_range_finds);
