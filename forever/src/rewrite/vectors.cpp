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
