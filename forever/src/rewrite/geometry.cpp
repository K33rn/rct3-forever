// Geometry: bounds and planes. The game's x87 code keeps some sums in a register (the precision the FPU is set to:
// long double here) and rounds others to float as it stores them; the rewrites do the same in the same order.
#include "rewrite.h"

// ---- Planes::TestBox: a box against six planes {x, y, z, d} (the view's): 0 when it is wholly outside one of them,
// 1 when one cuts it, 2 when it is inside all six. Of each plane, `near` takes the box corner furthest along the
// normal and `far` the nearest. The box: x from low[0] to low[0] + size[0], y from mid[1] to mid[2], z from low[1] to
// low[1] + size[1] ----------------------------------------------------------------------------------------------------
static volatile long g_box_tests;
static int32_t __thiscall planes_test_box(const float *planes, const float *mid, const float *low, const float *size) {
    g_box_tests++;
    volatile float high_x_stored = (float)((long double)low[0] + size[0]);   // volatile: the compiler would keep them unrounded
    volatile float high_z_stored = (float)((long double)size[1] + low[1]);
    float high_x = high_x_stored, high_z = high_z_stored;
    int32_t result = 2;
    for (int plane = 0; plane < 6; plane++) {
        const float *p = planes + plane * 4;
        long double outside = p[3];   // kept in the register
        float inside = p[3];          // stored as a float after each axis
        if (p[0] > 0.0f) {
            outside = outside + (long double)p[0] * low[0];
            inside = (float)((long double)high_x * p[0] + inside);
        } else {
            outside = outside + (long double)high_x * p[0];
            inside = (float)((long double)p[0] * low[0] + inside);
        }
        if (p[1] > 0.0f) {
            outside = outside + (long double)mid[1] * p[1];
            inside = (float)((long double)mid[2] * p[1] + inside);
        } else {
            outside = outside + (long double)mid[2] * p[1];
            inside = (float)((long double)mid[1] * p[1] + inside);
        }
        if (p[2] > 0.0f) {
            outside = outside + (long double)low[1] * p[2];
            inside = (float)((long double)high_z * p[2] + inside);
        } else {
            outside = outside + (long double)high_z * p[2];
            inside = (float)((long double)low[1] * p[2] + inside);
        }
        if (outside > 0.0L) return 0;
        if (inside > 0.0f) result = 1;
    }
    return result;
}
static const uint8_t planes_test_box_original[] = {0x83, 0xec, 0x08, 0x8b, 0x54, 0x24, 0x10, 0x8b};
static const uint32_t planes_test_box_copies[] = {0x42fb10, 0x4493f0, 0xa0c0a0, 0xb5bbe0, 0xb6bd00};
REWRITE_COPIES("Planes::TestBox", planes_test_box_copies, planes_test_box_original, &planes_test_box, &g_box_tests);

// ---- QuadTree::UpdateBounds(n): node n (0x1C bytes: flags, low, high, +0xC, item count, +0x14, items) brought up to
// date when marked changed (flag 0x10). Its children 4n+1 to 4n+4 (those that exist) are updated first, bits 0-3 note
// which of them hold anything, and their ranges are taken in; then its own items' (0x20 bytes: low at +4, high at
// +0x10, in use at +0x1C). The range starts empty (1e20 down to -1e20) ----------------------------------------------------
struct QuadNode {
    uint32_t flags;
    float low, high;
    uint32_t other, item_count, unused;
    uint8_t *items;
};
struct QuadTree {
    uint32_t unused;
    int32_t count;
    QuadNode *nodes;
};
static float float_from_bits(uint32_t bits) {
    float f;
    __builtin_memcpy(&f, &bits, 4);
    return f;
}
static volatile long g_quad_updates;
static void __thiscall quad_update_bounds(QuadTree *tree, int32_t n) {
    g_quad_updates++;
    QuadNode *node = &tree->nodes[n];
    if (!(node->flags & 0x10)) return;
    const float empty_low = float_from_bits(0x60ad78ec), empty_high = float_from_bits(0xe0ad78ec);   // 1e20, -1e20
    float low = empty_low, high = empty_high;
    int32_t first = n * 4 + 1;
    if (first < tree->count) {
        node->flags &= ~0xfu;
        for (int32_t k = 0; k < 4; k++) {
            QuadNode *child = &tree->nodes[first + k];
            if (child->flags & 0x10) quad_update_bounds(tree, first + k);
            if (child->other || child->item_count || (child->flags & 0xf)) node->flags |= 1u << k;
            if (child->low < low) low = child->low;
            if (child->high > high) high = child->high;
        }
    }
    float items_low = empty_low, items_high = empty_high;
    for (int32_t i = 0; i < (int32_t)node->item_count; i++) {
        uint8_t *item = node->items + i * 0x20;
        if (!*(uint32_t *)(item + 0x1c)) continue;
        if (*(float *)(item + 4) < items_low) items_low = *(float *)(item + 4);
        if (items_high < *(float *)(item + 0x10)) items_high = *(float *)(item + 0x10);
    }
    if (items_low < low) low = items_low;
    if (items_high > high) high = items_high;
    node->high = high;
    node->low = low;
    node->flags &= ~0x10u;
}
static const uint8_t quad_update_bounds_original[] = {0x83, 0xec, 0x0c, 0x53, 0x8b, 0xd1, 0x8b, 0x4c};
static const uint32_t quad_update_bounds_copies[] = {0x430770, 0x4891a0, 0x7b41b0, 0xa0c730, 0xb5c2e0, 0xb6d510};
REWRITE_COPIES("QuadTree::UpdateBounds", quad_update_bounds_copies, quad_update_bounds_original, &quad_update_bounds, &g_quad_updates);

// ---- SphereQuery::Set(centre, radius, tag): the box around the sphere (centre - radius to centre + radius), the
// centre, the radius squared and the tag ---------------------------------------------------------------------------------
struct SphereQuery {
    float low[3], high[3], centre[3], radius_squared;
    uint32_t tag;
};
static volatile long g_sphere_sets;
static void __thiscall sphere_query_set(SphereQuery *query, const float *centre, float radius, uint32_t tag) {
    g_sphere_sets++;
    query->radius_squared = radius * radius;
    uint32_t copy[3];
    __builtin_memcpy(copy, centre, 12);
    __builtin_memcpy(query->centre, copy, 12);
    query->tag = tag;
    float x = centre[0], y = centre[1], z = centre[2];
    query->low[0] = x - radius;
    query->low[1] = y - radius;
    query->low[2] = z - radius;
    query->high[0] = radius + centre[0];
    query->high[1] = centre[1] + radius;
    query->high[2] = centre[2] + radius;
}
static const uint8_t sphere_query_set_original[] = {0x83, 0xec, 0x0c, 0xd9, 0x44, 0x24, 0x14, 0x8b};
static const uint32_t sphere_query_set_copies[] = {0x488660, 0x7b2e30, 0xb5b740, 0xb6bb70};
REWRITE_COPIES("SphereQuery::Set", sphere_query_set_copies, sphere_query_set_original, &sphere_query_set, &g_sphere_sets);

// ---- Box::Overlap: a box (x from low[0] to low[0] + size[0], y from mid[1] to mid[2], z from low[1] to low[1] +
// size[1]) against this one {low xyz, high xyz}: 0 when they are apart, 1 when they overlap, 2 when it is inside ------
static volatile long g_box_overlaps;
static int32_t __thiscall box_overlap(const float *box, const float *mid, const float *low, const float *size) {
    g_box_overlaps++;
    volatile float high_x_stored = (float)((long double)low[0] + size[0]);   // volatile: rounded to float, as stored
    volatile float high_z_stored = (float)((long double)size[1] + low[1]);
    float high_x = high_x_stored, high_z = high_z_stored;
    if (low[0] > box[3] || mid[1] > box[4] || low[1] > box[5]) return 0;
    if (high_x < box[0] || mid[2] < box[1] || high_z < box[2]) return 0;
    if (!(low[0] >= box[0]) || !(mid[1] >= box[1]) || !(low[1] >= box[2])) return 1;
    if (!(high_x <= box[3]) || !(mid[2] <= box[4]) || !(high_z <= box[5])) return 1;
    return 2;
}
static const uint8_t box_overlap_original[] = {0x83, 0xec, 0x08, 0x8b, 0x54, 0x24, 0x10, 0x8b};
static const uint32_t box_overlap_copies[] = {0x488710, 0x7b2fc0, 0x7b32c0};
REWRITE_COPIES("Box::Overlap", box_overlap_copies, box_overlap_original, &box_overlap, &g_box_overlaps);
