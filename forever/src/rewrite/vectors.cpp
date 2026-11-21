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
