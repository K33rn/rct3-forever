// AttractionPerson::Check4E84C0 and AttractionPerson::Check5EBF30 (one function written out twice, the same
// instructions): is point q, placed by a matrix, farther from the person's point than point p is? (thiscall, p and
// q are 3-float vectors; gives 1 or 0.)
//   - The person's interface (+0x114) is looked up once and kept (+0x118 set): the object its reference (+0x104)
//     points to (+0xC), asked for its class info (its slot 1), then the info's entry for class number [0x12F5608]
//     (when the number is below the info's count, +0x18; entries at +0x14) gives the interface's offset (+0x10) in
//     the object. None (0) when any step finds nothing.
//   - That interface's member at +0x94 keeps its own interface (+0x14, kept once +0x18 is set) the same way, from its
//     reference at +4 and class number [0x130B5C8]; that one has a matrix at +0xF0 (4 columns of 4 floats, the last
//     being the position).
//   - p and q are each placed by the matrix (x = m0 p0 + m4 p1 + m8 p2 + m12, summed as m4 p1 + m0 p0, then the
//     rest), and their differences from the person's point (+0xA0..0xA8) squared and summed (z, y, then x).
//     q is farther when its sum is greater; NaN or equal is not farther.
// Values are rounded to floats exactly where the game stores them; q's y and z stay on the FPU (the game never
// stores them), and so does q's last difference.
#include "rewrite.h"

namespace {
typedef long double ext;
inline float rf(ext x) {   // stored to memory as a float, and read back
    volatile float f = (float)x;
    return f;
}

// An interface kept by a holder: looked up once (the flag set), then read back.
uint8_t *interface_of(uint8_t *reference, uint32_t class_global, uint8_t *volatile *kept, volatile uint8_t *looked) {
    if (!*looked) {
        uint8_t *object = *(uint8_t **)(reference + 0xc), *found = nullptr;
        if (object) {
            uint8_t *info = ((uint8_t * (__thiscall *)(void *))(*(void ***)object)[1])(object);
            if (info) {
                const int32_t number = *(volatile int32_t *)(uintptr_t)class_global;
                if (number < *(int32_t *)(info + 0x18)) {
                    uint8_t *entry = (*(uint8_t ***)(info + 0x14))[number];
                    if (entry) found = object + *(int32_t *)(entry + 0x10);
                }
            }
        }
        *kept = found;
        *looked = 1;
    }
    return *kept;
}

volatile long g_farther_points;
int __thiscall farther_point(uint8_t *self, const float *p, const float *q) {
    g_farther_points++;
    uint8_t *person = interface_of(*(uint8_t **)(self + 0x104), 0x12f5608, (uint8_t * volatile *)(self + 0x114), self + 0x118);
    uint8_t *holder = person + 0x94;
    const uint8_t *placed = interface_of(*(uint8_t **)(holder + 4), 0x130b5c8, (uint8_t * volatile *)(holder + 0x14), holder + 0x18);
    const float *m = (const float *)(placed + 0xf0);
    const float *at = (const float *)(self + 0xa0);
    // p placed, each coordinate stored; its differences stored
    const float px = rf((((ext)m[4] * p[1] + (ext)m[0] * p[0]) + (ext)m[8] * p[2]) + m[12]);
    const float py = rf((((ext)m[5] * p[1] + (ext)m[1] * p[0]) + (ext)m[9] * p[2]) + m[13]);
    const float pz = rf((((ext)m[6] * p[1] + (ext)m[2] * p[0]) + (ext)m[10] * p[2]) + m[14]);
    const float pdx = rf((ext)px - at[0]), pdy = rf((ext)py - at[1]), pdz = rf((ext)pz - at[2]);
    // q placed: x stored, y and z kept
    const float qx = rf((((ext)m[4] * q[1] + (ext)m[0] * q[0]) + (ext)m[8] * q[2]) + m[12]);
    const ext qy = (((ext)m[5] * q[1] + (ext)m[1] * q[0]) + (ext)m[9] * q[2]) + m[13];
    const ext qz = (((ext)m[6] * q[1] + (ext)m[2] * q[0]) + (ext)m[10] * q[2]) + m[14];
    const float qdx = rf((ext)qx - at[0]), qdy = rf(qy - at[1]);
    const ext qdz = qz - at[2];
    const ext near_sum = ((ext)pdz * pdz + (ext)pdy * pdy) + (ext)pdx * pdx;
    const ext far_sum = (qdz * qdz + (ext)qdy * qdy) + (ext)qdx * qdx;
    return far_sum > near_sum ? 1 : 0;
}

const uint8_t farther_point_first[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83};
}   // namespace
REWRITE("AttractionPerson::FartherPoint", 0x4e84c0, farther_point_first, &farther_point, &g_farther_points);
REWRITE("AttractionPerson::FartherPoint", 0x5ebf30, farther_point_first, &farther_point, &g_farther_points);
