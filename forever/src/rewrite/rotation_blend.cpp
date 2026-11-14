// Quaternion::Slerp and Quaternion::Slerp2 (two copies of one function, the same instructions): blend two rotations
// given as rotation vectors (axis times angle) a and b by t, into out (ecx = out, edx = a; b and t on the stack;
// gives out).
//   - When b - a is short (its squared length under 0.01): a straight blend, a + (b - a) t.
//   - Otherwise each rotation becomes a unit quaternion (half the angle: the vector times sin(len/2)/len, or times
//     1/2 when the length is within 2^-23 of 0; w = cos(len/2)); the two are blended along the sphere (the sines of
//     the angle between them, from the game's acos, with b's quaternion turned round when the dot product is
//     negative; a straight blend when they are within 0.99999), and the result turned back into a rotation vector
//     (2 atan2(|v|, w) along v; v as it is when |v| is 0).
// Every value is rounded to a float exactly where the game keeps it in memory, and kept at the FPU's precision where
// the game keeps it on the FPU stack; sin, cos, sqrt and atan2 are the FPU's own instructions.
#include "rewrite.h"

namespace {
typedef long double ext;
inline float rf(ext x) {   // stored to memory as a float, and read back
    volatile float f = (float)x;
    return f;
}
inline float bits(uint32_t b) {
    float f;
    memcpy(&f, &b, 4);
    return f;
}
inline ext fsin(ext x) { __asm__("fsin" : "+t"(x)); return x; }
inline ext fcos(ext x) { __asm__("fcos" : "+t"(x)); return x; }
inline ext fsqrt(ext x) { __asm__("fsqrt" : "+t"(x)); return x; }
inline ext fpatan(ext y, ext x) {   // atan2(y, x)
    ext r;
    __asm__("fpatan" : "=t"(r) : "0"(x), "u"(y) : "st(1)");
    return r;
}
inline ext game_acos(ext x) {   // the game's acos: the value in st(0) and back
    __asm__ volatile("call *%1" : "+t"(x) : "r"((uintptr_t)0xddebc0) : "eax", "ecx", "edx", "memory", "st(1)", "st(2)", "st(3)",
                     "st(4)", "st(5)", "st(6)", "st(7)");
    return x;
}

const float NEAR_BLEND = bits(0x3c23d70a);   // 0.01: squared distance below which the blend is straight
const float TINY = bits(0x34000000);         // 2^-23
const float HALF = 0.5f, ONE = 1.0f, ZERO = 0.0f;
const float SAME = bits(0x3f7fff58);         // 0.99999: quaternions this close blend straight

struct Quat {
    float x, y, z, w;
    ext w_full;   // w as the FPU had it
};
Quat quat_of(const float *v) {
    const ext v0 = v[0], v1 = v[1], v2 = v[2];
    const ext len = fsqrt((v0 * v0 + v1 * v1) + v2 * v2);
    const ext k = len > -TINY && len < TINY ? (ext)HALF : fsin(HALF * len) / len;
    Quat q;
    q.x = rf(k * v[0]);
    q.y = rf(k * v[1]);
    q.z = rf(k * v[2]);
    q.w_full = fcos(len * HALF);
    q.w = rf(q.w_full);
    return q;
}

volatile long g_rotation_blends;
float *__fastcall rotation_blend(float *out, const float *a, const float *b, float t) {
    g_rotation_blends++;
    const ext x0 = (ext)b[0] - a[0];
    const float d1 = rf((ext)b[1] - a[1]);
    const ext x2 = (ext)b[2] - a[2];
    const float d2 = rf(x2);
    if ((x2 * d2 + (ext)d1 * d1) + x0 * x0 < NEAR_BLEND) {
        const float p0 = rf(x0 * t), p1 = rf((ext)d1 * t);
        const ext r2 = (ext)d2 * t + a[2], r1 = (ext)p1 + a[1], r0 = (ext)p0 + a[0];
        out[0] = (float)r0, out[1] = (float)r1, out[2] = (float)r2;
        return out;
    }
    const Quat qa = quat_of(a), qb = quat_of(b);
    ext dot = ((qb.w_full * qa.w + (ext)qb.z * qa.z) + (ext)qb.y * qa.y) + (ext)qb.x * qa.x;
    const bool turned = dot < ZERO;
    if (turned) dot = -dot;
    float dot_f = rf(dot);
    ext s0, s1;
    if (dot > SAME) {
        s0 = (ext)ONE - t;
        s1 = t;
    } else {
        const ext angle = game_acos(dot_f);
        const float scale = rf((ext)ONE / fsqrt((ext)ONE - (ext)dot_f * dot_f));
        const float part = rf(t * angle);
        s0 = fsin(angle - part) * scale;
        s1 = fsin((ext)part) * scale;
    }
    const float bx = rf(qb.x * s1), by = rf(qb.y * s1), bz = rf(qb.z * s1), bw = rf(qb.w * s1);
    const float ax = rf(qa.x * s0), ay = rf(qa.y * s0), az = rf(qa.z * s0);
    const ext aw = s0 * qa.w;
    ext rx;
    float y, z, w;
    if (turned) {
        rx = (ext)ax - bx, y = rf((ext)ay - by), z = rf((ext)az - bz), w = rf(aw - bw);
    } else {
        rx = (ext)ax + bx, y = rf((ext)ay + by), z = rf((ext)az + bz), w = rf(aw + bw);
    }
    const float n = rf(fsqrt(((ext)z * z + (ext)y * y) + rx * rx));
    if (n == ZERO) {
        out[0] = (float)rx, out[1] = y, out[2] = z;
        return out;
    }
    const float angle = rf(fpatan(n, w));
    const ext ox = rx * angle, oy = (ext)y * angle;
    const float oz = rf((ext)z * angle);
    const float ox2 = rf(ox + ox);
    const ext oy2 = oy + oy, oz2 = (ext)oz + oz, inv = (ext)ONE / n;
    out[0] = (float)((ext)ox2 * inv);
    out[1] = (float)(oy2 * inv);
    out[2] = (float)(oz2 * inv);
    return out;
}

const uint8_t rotation_blend_first[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83};
}   // namespace
REWRITE("Quaternion::Slerp", 0xcc5c80, rotation_blend_first, &rotation_blend, &g_rotation_blends);
REWRITE("Quaternion::Slerp", 0xc5a1f0, rotation_blend_first, &rotation_blend, &g_rotation_blends);
