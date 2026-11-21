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
static void __fastcall vector_step_bcc090(float *out, const float *p0, const float *p1, const float *p2, const float *p3, float t_f) {
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
static void __thiscall window_arc(const uint8_t *self, float *out, float angle, float r, float h) {
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
}
static const uint8_t window_arc_original[] = {0x83, 0xec, 0x08, 0xd9, 0x44, 0x24, 0x18, 0x8b};
REWRITE("Window::ArcA45670", 0xa45670, window_arc_original, &window_arc, &g_window_arcs);
