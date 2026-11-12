// Water: pieces of the water surface cut at a height, written out as triangles.
#include "rewrite.h"

typedef long double x87;

// The FPU's clamps (fcomi then fcmov): below 0 (or not a number) gives 0, above 1 gives 1.
static inline float clamp01(float t) {
    if (!(t >= 0.0f)) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return t;
}
// the lower of two as the game picks it: b when a is above b, else a (a not a number stays)
static inline float lower(float a, float b) { return a > b ? b : a; }
// where the height cuts the edge from a to b: its fraction (rounded, kept between 0 and 1)
static inline float cut_fraction(float height, const float *a, const float *b) {
    volatile float t = (float)(((x87)height - a[1]) / ((x87)b[1] - a[1]));
    return clamp01(t);
}
// The cut point, as the game works out the first of a piece's two: the z step rounded before it's scaled, the x step
// rounded once scaled, the y step kept in the FPU's precision; each rounded once the start is added.
static inline void cut_first(volatile float *out, float height, const float *a, const float *b) {
    const float u = cut_fraction(height, a, b);
    volatile float dz = (float)((x87)b[2] - a[2]);
    volatile float dxt = (float)(((x87)b[0] - a[0]) * u);
    const x87 dyt = ((x87)b[1] - a[1]) * u, dzt = (x87)dz * u;
    out[0] = (float)((x87)dxt + a[0]);
    out[1] = (float)(dyt + a[1]);
    out[2] = (float)(dzt + a[2]);
}
// ... and the second: all three steps rounded once scaled; the start added in the FPU's precision (rounded where used)
static inline void cut_second(x87 *out, float height, const float *a, const float *b) {
    const float u = cut_fraction(height, a, b);
    volatile float ez = (float)((x87)b[2] - a[2]);
    volatile float ext = (float)(((x87)b[0] - a[0]) * u), eyt = (float)(((x87)b[1] - a[1]) * u), ezt = (float)((x87)ez * u);
    out[0] = (x87)ext + a[0];
    out[1] = (x87)eyt + a[1];
    out[2] = (x87)ezt + a[2];
}
static inline void copy_point(float *to, const float *from) {   // an exact copy (never through the FPU)
    const volatile uint32_t *f = (const volatile uint32_t *)from;
    volatile uint32_t *t = (volatile uint32_t *)to;
    t[0] = f[0], t[1] = f[1], t[2] = f[2];
}
static inline void put_point(float *to, const volatile float *from) {
    for (int i = 0; i < 3; i++) ((volatile float *)to)[i] = from[i];
}
static inline void put_point(float *to, const x87 *from) {
    for (int i = 0; i < 3; i++) ((volatile float *)to)[i] = (float)from[i];
}

// A float negated as the FPU does it (fld then fchs: a signalling NaN comes out quiet, its sign flipped), kept from
// being folded into the products it is used in.
static inline x87 negated(float f) {
    volatile x87 v = f;
    volatile x87 m = -v;
    return m;
}

static volatile long g_water_edges;

// ---- Water::EdgeCut(vertices, indices, count, height, a, b, c, sum, lowest): the triangle a, a-b cut, a-c cut written at
// the vertex and index write pointers, which move on (and the count by 3); sum gains the cut heights and a's, lowest
// keeps the lowest. Returns lowest. ----
static float *__fastcall water_edge_cut(float **vertices, uint32_t **indices, uint32_t *count, float height, const float *a, const float *b,
                                        const float *c, float *sum, float *lowest) {
    g_water_edges++;
    volatile float p1[3];
    x87 q2[3];
    cut_first(p1, height, a, b);
    cut_second(q2, height, a, c);
    uint32_t first = *count;
    uint32_t *index = *indices;
    float *v = *vertices;
    *vertices = v + 9;
    *indices = *indices + 3;
    *count += 3;
    copy_point(v, a);
    put_point(v + 3, p1);
    put_point(v + 6, q2);
    index[0] = first;
    index[2] = first + 2;
    index[1] = first + 1;
    *sum = (float)((((x87)p1[1] + q2[1]) + a[1]) + *sum);
    *lowest = lower(lower(lower((float)q2[1], p1[1]), a[1]), *lowest);
    return lowest;
}
static const uint8_t water_edge_cut_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x44, 0x8b, 0x45, 0x10};
REWRITE("Water::EdgeCut", 0x95df40, water_edge_cut_original, &water_edge_cut, &g_water_edges);

// ---- Water::EdgeCutQuad(vertices, indices, count, height, p, q, a, sum, lowest): the quad p, a-p cut, q, a-q cut (two
// triangles: 0 2 1, 1 2 3), as above (count by 4). Returns lowest. ----
static float *__fastcall water_edge_cut_quad(float **vertices, uint32_t **indices, uint32_t *count, float height, const float *p, const float *q,
                                             const float *a, float *sum, float *lowest) {
    g_water_edges++;
    volatile float r1[3];
    x87 r2[3];
    cut_first(r1, height, a, p);
    cut_second(r2, height, a, q);
    uint32_t *index = *indices;
    uint32_t first = *count;
    float *v = *vertices;
    *vertices = v + 12;
    *indices = (uint32_t *)((uint8_t *)*indices + 0x18);
    *count += 4;
    copy_point(v, p);
    put_point(v + 3, r1);
    copy_point(v + 6, q);
    put_point(v + 9, r2);
    index[0] = first, index[1] = first + 2, index[2] = first + 1, index[3] = first + 1, index[4] = first + 2, index[5] = first + 3;
    *sum = (float)(((((x87)q[1] + p[1]) + r2[1]) + r1[1]) + *sum);
    *lowest = lower(lower(lower(lower((float)r2[1], q[1]), r1[1]), p[1]), *lowest);
    return lowest;
}
static const uint8_t water_edge_cut_quad_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x44, 0x8b, 0x45, 0x18};
REWRITE("Water::EdgeCutQuad", 0x95e160, water_edge_cut_quad_original, &water_edge_cut_quad, &g_water_edges);

// ---- Water::EdgeCutQuad2(vertices, indices, count, height, e, a, p, f, sum, lowest): the quad e, a, a-p cut, e-f cut
// (two triangles: 1 2 0, 0 2 3). Returns lowest. ----
static float *__fastcall water_edge_cut_quad2(float **vertices, uint32_t **indices, uint32_t *count, float height, const float *e, const float *a,
                                              const float *p, const float *f, float *sum, float *lowest) {
    g_water_edges++;
    volatile float r1[3];
    x87 r2[3];
    cut_first(r1, height, a, p);
    cut_second(r2, height, e, f);
    uint32_t *index = *indices;
    float *v = *vertices;
    uint32_t first = *count;
    *vertices = v + 12;
    *indices = (uint32_t *)((uint8_t *)*indices + 0x18);
    *count += 4;
    copy_point(v, e);
    copy_point(v + 3, a);
    put_point(v + 6, r1);
    put_point(v + 9, r2);
    index[2] = first, index[3] = first, index[5] = first + 3, index[0] = first + 1, index[1] = first + 2, index[4] = first + 2;
    *sum = (float)(((((x87)r2[1] + e[1]) + r1[1]) + a[1]) + *sum);
    *lowest = lower(lower(lower(lower((float)r2[1], r1[1]), a[1]), e[1]), *lowest);
    return lowest;
}
static const uint8_t water_edge_cut_quad2_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x44, 0xd9, 0x45, 0x0c};
REWRITE("Water::EdgeCutQuad2", 0x95e3d0, water_edge_cut_quad2_original, &water_edge_cut_quad2, &g_water_edges);

// ---- Water::EdgeCutFive(vertices, indices, count, height, e, m, a, p, sum, lowest): five vertices e, m, a, a-p cut, e-p cut
// (three triangles: 2 0 1, 2 4 0, 2 3 4), as above (count by 5). Returns lowest. ----
static float *__fastcall water_edge_cut_five(float **vertices, uint32_t **indices, uint32_t *count, float height, const float *e, const float *m,
                                             const float *a, const float *p, float *sum, float *lowest) {
    g_water_edges++;
    volatile float r1[3];
    x87 r2[3];
    cut_first(r1, height, a, p);
    cut_second(r2, height, e, p);
    uint32_t *index = *indices;
    float *v = *vertices;
    uint32_t first = *count;
    *vertices = v + 15;
    *indices = (uint32_t *)((uint8_t *)*indices + 0x24);
    *count += 5;
    copy_point(v, e);
    copy_point(v + 3, m);
    copy_point(v + 6, a);
    put_point(v + 9, r1);
    put_point(v + 12, r2);
    index[0] = first + 2, index[2] = first + 1, index[3] = first + 2, index[1] = first, index[4] = first + 4, index[5] = first;
    index[7] = first + 3, index[6] = first + 2, index[8] = first + 4;
    *sum = (float)((((((x87)e[1] + m[1]) + r2[1]) + r1[1]) + a[1]) + *sum);
    *lowest = lower(lower(lower(lower(lower((float)r2[1], r1[1]), a[1]), m[1]), e[1]), *lowest);
    return lowest;
}
static const uint8_t water_edge_cut_five_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x44, 0xd9, 0x45, 0x0c, 0x8b, 0x45, 0x1c};
REWRITE("Water::EdgeCutFive", 0x95e640, water_edge_cut_five_original, &water_edge_cut_five, &g_water_edges);

// ---- Water::BoxPlanes(box, extra, planes, distances): a box's six planes from the three axes at this +0xE0 (0x10
// apart): each axis (copied exactly) and its negative (through the FPU) as planes (0x10 apart); their distances the
// axis against the box's centre plus, for the first, extra and for the others the length of the box's half-extent row
// (+0x10, +0x20); the negatives' without. Returns box. ----
static const float *__thiscall water_box_planes(const uint8_t *self, const float *box, float extra, float *planes, float *distances) {
    g_water_edges++;
    const float *axes = (const float *)(self + 0xe0);
    for (int k = 0; k < 3; k++) {
        const float *n = axes + 4 * k;
        float *plane = planes + 8 * k;
        copy_point(plane, n);
        x87 add = extra;
        if (k) {
            const float *e = box + 4 * k;
            add = __builtin_sqrtl(((x87)e[0] * e[0] + (x87)e[1] * e[1]) + (x87)e[2] * e[2]);
        }
        ((volatile float *)distances)[2 * k] = (float)(((((x87)n[2] * box[2] + (x87)n[1] * box[1]) + (x87)box[0] * n[0])) + add);
        const x87 m0 = negated(n[0]), m1 = negated(n[1]), m2 = negated(n[2]);
        volatile float *neg = plane + 4;
        neg[2] = (float)m2, neg[0] = (float)m0, neg[1] = (float)m1;
        ((volatile float *)distances)[2 * k + 1] = (float)((m1 * box[1] + m0 * box[0]) + m2 * box[2]);
    }
    return box;
}
static const uint8_t water_box_planes_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x18, 0x8b, 0x81, 0xe0};
REWRITE("Water::BoxPlanes", 0x8e6e10, water_box_planes_original, &water_box_planes, &g_water_edges);

// ---- Water::BoxSpan(lo, hi, out, grow): a box (corners lo, hi) seen along the three axes at this +0xE0 (0x10 apart):
// for each axis the top and bottom of its range, each a sum from 0 over x, y, z of the axis value times hi or lo, as
// the game's fcomi / fcmov pick them. The top takes hi when the value is above 0, else lo. The bottom takes lo when it
// is 0 or more, else hi. A value that isn't a number gives lo to the top and hi to the bottom. With grow, axis 1's and
// 2's ranges widen by the constant at 0xF7CCB8 (0.2; axis 1's top kept in the FPU's precision). Out: the corner
// bottom0 axis0 + bottom1 axis1 + bottom2 axis2, then the edges (top1 - bottom1) axis1 and (top2 - bottom2) axis2, 0x10
// apart, with the game's roundings kept. Returns out. ----
static inline float rounded(x87 v) {   // rounded to a float, as the game keeps it
    volatile float f = (float)v;
    return f;
}
static inline void box_range(const float *axis, const float *lo, const float *hi, float *top, float *bottom) {
    x87 t = 0.0L, b = 0.0L;
    for (int j = 0; j < 3; j++) {
        const float m = axis[j];
        const x87 by_hi = (x87)m * hi[j], by_lo = (x87)m * lo[j];
        t = t + ((m > 0.0f) ? by_hi : by_lo);
        b = b + ((m >= 0.0f) ? by_lo : by_hi);
    }
    *top = rounded(t), *bottom = rounded(b);
}
static float *__thiscall water_box_span(const uint8_t *self, const float *lo, const float *hi, float *out, uint8_t grow) {
    g_water_edges++;
    const float *r0 = (const float *)(self + 0xe0), *r1 = r0 + 4, *r2 = r0 + 8;
    float top1, bottom1, top2, bottom2, top0, bottom0;
    box_range(r1, lo, hi, &top1, &bottom1);
    box_range(r2, lo, hi, &top2, &bottom2);
    box_range(r0, lo, hi, &top0, &bottom0);   // (its top isn't used)
    x87 t1 = top1;
    if (grow) {
        const x87 e = *(const float *)0xf7ccb8;
        bottom1 = rounded(bottom1 - e);
        t1 = (x87)top1 + e;
        bottom2 = rounded(bottom2 - e);
        top2 = rounded(top2 + e);
    }
    const x87 c0x = (x87)bottom0 * r0[0];
    const float c0y = rounded((x87)bottom0 * r0[1]), c0z = rounded((x87)bottom0 * r0[2]);
    const x87 c2x = (x87)bottom2 * r2[0], c2y = (x87)bottom2 * r2[1];
    const float c2z = rounded((x87)bottom2 * r2[2]);
    const x87 c1x = (x87)bottom1 * r1[0];
    const float c1y = rounded((x87)bottom1 * r1[1]), c1z = rounded((x87)bottom1 * r1[2]);
    const float s0 = rounded(c1x + c2x), s1 = rounded((x87)c1y + c2y), s2 = rounded((x87)c1z + c2z);
    volatile float *o = out;
    o[0] = (float)(c0x + s0), o[1] = (float)((x87)s1 + c0y), o[2] = (float)((x87)s2 + c0z);
    const x87 d1 = t1 - bottom1;
    o[6] = (float)(d1 * r1[2]), o[4] = (float)(d1 * r1[0]), o[5] = (float)(d1 * r1[1]);
    const x87 d2 = (x87)top2 - bottom2;
    o[10] = (float)(d2 * r2[2]), o[8] = (float)(d2 * r2[0]), o[9] = (float)(d2 * r2[1]);
    return out;
}
static const uint8_t water_box_span_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x64, 0x53, 0x8b, 0xd1};
REWRITE("Water::BoxSpan", 0x8e6980, water_box_span_original, &water_box_span, &g_water_edges);
