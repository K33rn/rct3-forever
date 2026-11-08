// Geometry: bounds and planes. The game's x87 code keeps some sums in a register (the precision the FPU is set to:
// long double here) and rounds others to float as it stores them; the rewrites do the same in the same order.
#include "rewrite.h"

// ---- Planes::TestBox: a box against six planes {x, y, z, d} (the view's): 0 when it is wholly outside one of them,
// 1 when one cuts it, 2 when it is inside all six. Of each plane, `near` takes the box corner furthest along the
// normal and `far` the nearest. The box: x from low[0] to low[0] + size[0], y from mid[1] to mid[2], z from low[1] to
// low[1] + size[1] ----------------------------------------------------------------------------------------------------
static volatile long g_box_tests;
static int32_t planes_box_bounds(const float *planes, float low_x, float high_x, float low_y, float high_y, float low_z, float high_z) {
    const float mid[3] = {0, low_y, high_y}, low[2] = {low_x, low_z};
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
static int32_t planes_box(const float *planes, const float *mid, const float *low, const float *size) {
    volatile float high_x_stored = (float)((long double)low[0] + size[0]);   // volatile: the compiler would keep them unrounded
    volatile float high_z_stored = (float)((long double)size[1] + low[1]);
    return planes_box_bounds(planes, low[0], high_x_stored, mid[1], mid[2], low[1], high_z_stored);
}
static int32_t __thiscall planes_test_box(const float *planes, const float *mid, const float *low, const float *size) {
    g_box_tests++;
    return planes_box(planes, mid, low, size);
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
static int32_t box_overlaps_bounds(const float *box, float low_x, float high_x, float low_y, float high_y, float low_z, float high_z) {
    const float mid[3] = {0, low_y, high_y}, low[2] = {low_x, low_z};
    if (low[0] > box[3] || mid[1] > box[4] || low[1] > box[5]) return 0;
    if (high_x < box[0] || mid[2] < box[1] || high_z < box[2]) return 0;
    if (!(low[0] >= box[0]) || !(mid[1] >= box[1]) || !(low[1] >= box[2])) return 1;
    if (!(high_x <= box[3]) || !(mid[2] <= box[4]) || !(high_z <= box[5])) return 1;
    return 2;
}
static int32_t box_overlaps(const float *box, const float *mid, const float *low, const float *size) {
    volatile float high_x_stored = (float)((long double)low[0] + size[0]);   // volatile: rounded to float, as stored
    volatile float high_z_stored = (float)((long double)size[1] + low[1]);
    return box_overlaps_bounds(box, low[0], high_x_stored, mid[1], mid[2], low[1], high_z_stored);
}
static int32_t __thiscall box_overlap(const float *box, const float *mid, const float *low, const float *size) {
    g_box_overlaps++;
    return box_overlaps(box, mid, low, size);
}
static const uint8_t box_overlap_original[] = {0x83, 0xec, 0x08, 0x8b, 0x54, 0x24, 0x10, 0x8b};
static const uint32_t box_overlap_copies[] = {0x488710, 0x7b2fc0, 0x7b32c0};
REWRITE_COPIES("Box::Overlap", box_overlap_copies, box_overlap_original, &box_overlap, &g_box_overlaps);

// ---- A heap of 24-byte entries keyed by the float at their start (largest first): {entries, ?, count} -----------------
struct HeapEntry {
    float key;
    uint32_t rest[5];
};
struct Heap {
    HeapEntry *entries;
    uint32_t unused;
    int32_t count;
};
// The entry at i moved down to its place among the first n: the larger child (the right one when they're equal or not
// comparable) moves up while its key is greater.
static void heap_sift_down(Heap *heap, int32_t i, int32_t n) {
    HeapEntry moving = heap->entries[i];
    int32_t left = i * 2 + 1;
    for (; left + 1 < n; left = i * 2 + 1) {
        int32_t larger = heap->entries[left].key > heap->entries[left + 1].key ? left : left + 1;
        if (!(heap->entries[larger].key > moving.key)) {
            heap->entries[i] = moving;
            return;
        }
        heap->entries[i] = heap->entries[larger];
        i = larger;
    }
    if (left < n && heap->entries[left].key > moving.key) {
        heap->entries[i] = heap->entries[left];
        i = left;
    }
    heap->entries[i] = moving;
}
static volatile long g_heap_calls;
// Heap::SiftDown(i, n, unused)
static void __thiscall heap_sift_down_call(Heap *heap, int32_t i, int32_t n, uint32_t) {
    g_heap_calls++;
    heap_sift_down(heap, i, n);
}
// Heap::Build(unused): every entry sifted down, from the last to the first
static void __thiscall heap_build(Heap *heap, uint32_t) {
    g_heap_calls++;
    for (int32_t i = heap->count - 1; i >= 0; i--) heap_sift_down(heap, i, heap->count);
}
// Heap::Sort(unused): the largest swapped to the end, the rest made a heap again, until one is left (smallest first)
static void __thiscall heap_sort(Heap *heap, uint32_t) {
    g_heap_calls++;
    for (int32_t end = heap->count - 1; end > 0; end--) {
        HeapEntry largest = heap->entries[0];
        heap->entries[0] = heap->entries[end];
        heap->entries[end] = largest;
        heap_sift_down(heap, 0, end);
    }
}
static const uint8_t heap_sift_down_original[] = {0x83, 0xec, 0x1c, 0x8b, 0x11, 0x53, 0x8b, 0x5c};
static const uint8_t heap_build_original[] = {0x83, 0xec, 0x2c, 0x57, 0x8b, 0x79, 0x08, 0x4f};
static const uint8_t heap_sort_original[] = {0x83, 0xec, 0x3c, 0x53, 0x8b, 0x59, 0x08, 0x4b};
REWRITE("Heap::SiftDown", 0x80c530, heap_sift_down_original, &heap_sift_down_call, &g_heap_calls);
REWRITE("Heap::SiftDown", 0x96ee00, heap_sift_down_original, &heap_sift_down_call, &g_heap_calls);
REWRITE("Heap::Build", 0x80c9c0, heap_build_original, &heap_build, &g_heap_calls);
REWRITE("Heap::Build", 0x96f900, heap_build_original, &heap_build, &g_heap_calls);
REWRITE("Heap::Sort", 0x80cba0, heap_sort_original, &heap_sort, &g_heap_calls);
REWRITE("Heap::Sort", 0x96fae0, heap_sort_original, &heap_sort, &g_heap_calls);

// ---- Box::CornerOffsets(direction): for each axis, the byte offsets of the near and far corner coordinates in a box
// {low xyz, high xyz}: the low one is near when the direction is positive on that axis --------------------------------
static volatile long g_corner_offsets;
static void __thiscall box_corner_offsets(uint8_t *offsets, const float *direction) {
    g_corner_offsets++;
    for (int axis = 0; axis < 3; axis++) {
        bool positive = direction[axis] > 0.0f;
        offsets[axis] = (uint8_t)(axis * 4 + (positive ? 0 : 0x10));
        offsets[axis + 3] = (uint8_t)(axis * 4 + (positive ? 0x10 : 0));
    }
}
static const uint8_t box_corner_offsets_original[] = {0x8b, 0x54, 0x24, 0x04, 0xd9, 0x02, 0xd8, 0x1d};
REWRITE("Box::CornerOffsets", 0x80c340, box_corner_offsets_original, &box_corner_offsets, &g_corner_offsets);
REWRITE("Box::CornerOffsets", 0x96ea70, box_corner_offsets_original, &box_corner_offsets, &g_corner_offsets);

// ---- QuadTree::Cull(query): the tree's bounds brought up to date (UPDATE) when the root is marked changed, then its
// box (x and z from the tree, y from the root node) tested against the query: six planes (Planes::TestBox) or a box
// (Box::Overlap). Cut: the partial walk WALK(query, 0, low, size); inside: the take-all walk TAKE_ALL(0, query) ----------
struct QuadTreeBox {
    uint32_t unused;
    int32_t count;
    QuadNode *nodes;
    float low[2], size[2];
};
static volatile long g_quad_culls;
template <bool BY_PLANES, uintptr_t UPDATE, uintptr_t TAKE_ALL, uintptr_t WALK> static void __thiscall quad_cull(QuadTreeBox *tree, const float *query) {
    g_quad_culls++;
    if (tree->nodes[0].flags & 0x10) ((void(__thiscall *)(void *, int32_t))UPDATE)(tree, 0);
    const float *root = (const float *)tree->nodes;
    int32_t seen = BY_PLANES ? planes_box(query, root, tree->low, tree->size) : box_overlaps(query, root, tree->low, tree->size);
    if (seen == 2) ((void(__thiscall *)(void *, int32_t, const float *))TAKE_ALL)(tree, 0, query);
    else if (seen == 1) ((void(__thiscall *)(void *, const float *, int32_t, float *, float *))WALK)(tree, query, 0, tree->low, tree->size);
}
static const uint8_t quad_cull_planes_original[] = {0x83, 0xec, 0x10, 0x53, 0x55, 0x56, 0x8b, 0xf1};
static const uint8_t quad_cull_box_original[] = {0x83, 0xec, 0x08, 0x53, 0x56, 0x8b, 0xf1, 0x8b};
REWRITE("QuadTree::CullPlanes", 0x433c30, quad_cull_planes_original, (&quad_cull<true, 0x430770, 0x432f50, 0x433600>), &g_quad_culls);
REWRITE("QuadTree::CullPlanes", 0x44c450, quad_cull_planes_original, (&quad_cull<true, 0x430770, 0x44ba10, 0x44bd20>), &g_quad_culls);
REWRITE("QuadTree::CullPlanes", 0x48c460, quad_cull_planes_original, (&quad_cull<true, 0x4891a0, 0x48b370, 0x48bdc0>), &g_quad_culls);
REWRITE("QuadTree::CullPlanes", 0x7b9d70, quad_cull_planes_original, (&quad_cull<true, 0x7b41b0, 0x7b85f0, 0x7b9590>), &g_quad_culls);
REWRITE("QuadTree::CullPlanes", 0xa0e960, quad_cull_planes_original, (&quad_cull<true, 0xa0c730, 0xa0e240, 0xa0e450>), &g_quad_culls);
REWRITE("QuadTree::CullPlanes", 0xb5f3b0, quad_cull_planes_original, (&quad_cull<true, 0xb5c2e0, 0xb5e950, 0xb5ed60>), &g_quad_culls);
REWRITE("QuadTree::CullPlanes", 0xb756b0, quad_cull_planes_original, (&quad_cull<true, 0xb6d510, 0xb74250, 0xb75060>), &g_quad_culls);
REWRITE("QuadTree::CullBox", 0x48bb30, quad_cull_box_original, (&quad_cull<false, 0x4891a0, 0x48a4e0, 0x48b0b0>), &g_quad_culls);
REWRITE("QuadTree::CullBox", 0x7b7d40, quad_cull_box_original, (&quad_cull<false, 0x7b41b0, 0x7b5540, 0x7b6eb0>), &g_quad_culls);
REWRITE("QuadTree::CullBox", 0x7b90a0, quad_cull_box_original, (&quad_cull<false, 0x7b41b0, 0x7b6bd0, 0x7b7f30>), &g_quad_culls);
REWRITE("QuadTree::CullBox", 0x7b91d0, quad_cull_box_original, (&quad_cull<false, 0x7b41b0, 0x7b6d80, 0x7b82d0>), &g_quad_culls);
REWRITE("QuadTree::CullBox", 0x7b93b0, quad_cull_box_original, (&quad_cull<false, 0x7b41b0, 0x7b7390, 0x7b8920>), &g_quad_culls);
REWRITE("QuadTree::CullBox", 0xb5eb80, quad_cull_box_original, (&quad_cull<false, 0xb5c2e0, 0xb5d780, 0xb5e460>), &g_quad_culls);
REWRITE("QuadTree::CullBox", 0xb74e80, quad_cull_box_original, (&quad_cull<false, 0xb6d510, 0xb732b0, 0xb74580>), &g_quad_culls);

// ---- QuadTree::WalkPlanes(planes, n, low, size): node n is cut by the view. Each of its items in use (box low xyz,
// high xyz) that no plane has wholly outside is taken (Take, with what the planes point to at +0x60); the plane's
// distance is summed from the box corner nearest against the normal, in an order that differs per copy (bits 2p of
// LAST: which of the x, y and z products is added last). Then each child it marks gets its box (a half-size quarter, at 0.25 or 0.75
// of the parent's half) and is taken whole (TAKE_ALL) when inside or walked again when cut --------------------------------
static volatile long g_quad_walks;
// Show an item's object (+0x18): its virtual slot 8, or a fixed function (given a 0 when it takes one).
struct ShowVirtual {
    static void take(void *const *, uint8_t *item) {
        void *object = *(void **)(item + 0x18);
        ((void(__thiscall *)(void *))(*(void ***)object)[8])(object);
    }
};
template <uintptr_t SHOW> struct ShowCall {
    static void take(void *const *, uint8_t *item) { ((void(__thiscall *)(void *))SHOW)(*(void **)(item + 0x18)); }
};
template <uintptr_t SHOW> struct ShowCallZero {
    static void take(void *const *, uint8_t *item) { ((void(__thiscall *)(void *, int32_t))SHOW)(*(void **)(item + 0x18), 0); }
};
// Child k's box in x and z: half the parent's size, at 0.25 or 0.75 of that half from the parent's low corner (bit 0:
// x, bit 1: z); each value rounded to float as the game stores it.
static void quad_child_box(const float *low, const float *size, int32_t k, volatile float *half, volatile float *child_low, volatile float *high_x, volatile float *high_z) {
    half[0] = (float)((long double)size[0] * 0.5f);
    half[1] = (float)((long double)size[1] * 0.5f);
    long double along_z = k & 2 ? 0.75f : 0.25f, along_x = k & 1 ? 0.75f : 0.25f;
    child_low[0] = (float)(along_x * half[0] + low[0]);
    child_low[1] = (float)(along_z * half[1] + low[1]);
    *high_x = (float)((long double)child_low[0] + half[0]);
    *high_z = (float)((long double)child_low[1] + half[1]);
}
template <uint32_t LAST, uintptr_t TAKE_ALL, class Take> static void __thiscall quad_walk_planes(QuadTreeBox *tree, const float *planes, int32_t n, const float *low, const float *size) {
    g_quad_walks++;
    QuadNode *node = &tree->nodes[n];
    if (node->other || node->item_count)
        for (int32_t i = 0; i < (int32_t)node->item_count; i++) {
            const float *box = (const float *)(node->items + i * 0x20);
            if (!*(uint32_t *)(node->items + i * 0x20 + 0x1c)) continue;
            bool shown = true;
            for (int p = 0; p < 6 && shown; p++) {
                const float *plane = planes + p * 4;
                long double x = plane[0] > 0.0f ? box[0] : box[3], y = plane[1] > 0.0f ? box[1] : box[4], z = plane[2] > 0.0f ? box[2] : box[5];
                long double along[3] = {x * plane[0], y * plane[1], z * plane[2]};
                int last = LAST >> p * 2 & 3;   // the product added last; the other two first
                long double distance = (along[last == 0 ? 1 : 0] + along[last == 2 ? 1 : 2]) + along[last];
                distance = distance + plane[3];
                if (!(distance <= 0.0L)) shown = false;
            }
            if (shown) Take::take((void *const *)(planes + 0x18), node->items + i * 0x20);
        }
    if (!(node->flags & 0xf)) return;
    int32_t first = n * 4 + 1;
    for (int32_t k = 0; k < 4; k++) {
        if (!(node->flags & (1u << k))) continue;
        volatile float half[2], child_low[2], high_x, high_z;
        quad_child_box(low, size, k, half, child_low, &high_x, &high_z);
        QuadNode *child = &tree->nodes[first + k];
        int32_t seen = planes_box_bounds(planes, child_low[0], high_x, child->low, child->high, child_low[1], high_z);
        if (seen == 2) ((void(__thiscall *)(void *, int32_t, const float *))TAKE_ALL)(tree, first + k, planes);
        else if (seen == 1) quad_walk_planes<LAST, TAKE_ALL, Take>(tree, planes, first + k, (const float *)child_low, (const float *)half);
    }
}
static const uint8_t quad_walk_planes_original[] = {0x83, 0xec, 0x24, 0x8b, 0x51, 0x08, 0x53, 0x55};
REWRITE("QuadTree::WalkPlanes", 0x7b9590, quad_walk_planes_original, (&quad_walk_planes<0x454, 0x7b85f0, ShowVirtual>), &g_quad_walks);
REWRITE("QuadTree::WalkPlanes", 0xa0e450, quad_walk_planes_original, (&quad_walk_planes<0x52, 0xa0e240, ShowCall<0xa0a330>>), &g_quad_walks);
REWRITE("QuadTree::WalkPlanes", 0xb5ed60, quad_walk_planes_original, (&quad_walk_planes<0x52, 0xb5e950, ShowCallZero<0xb563b0>>), &g_quad_walks);
REWRITE("QuadTree::WalkPlanes", 0xb75060, quad_walk_planes_original, (&quad_walk_planes<0x52, 0xb74250, ShowCall<0xb44a30>>), &g_quad_walks);

// ---- What the box walks do with an item they take; query+0x18 is where it goes ---------------------------------------------
// Into a collector's list of 64 (at +4, count at +0x104), handed on whole to the collector's virtual slot 0 (list, 64)
// when full.
struct TakeInto64 {
    static void take(void *const *slot, uint8_t *item) {
        uint8_t *collector = (uint8_t *)*slot;
        int32_t &count = *(int32_t *)(collector + 0x104);
        ((uint32_t *)(collector + 4))[count] = *(uint32_t *)(item + 0x18);
        if (++count == 0x40) {
            ((void(__thiscall *)(void *, void *, int32_t))(*(void ***)collector)[0])(collector, collector + 4, 0x40);
            count = 0;
        }
    }
};
// When the item's object stands (x at +0x80, z at +0x88) within a rectangle {low x, low z, high x, high z}, onto the list
// after it {objects, capacity, count}, grown by GROW(capacity + 16) when full.
template <uintptr_t GROW> struct TakeInRect {
    static void take(void *const *slot, uint8_t *item) {
        uint8_t *object = *(uint8_t **)(item + 0x18);
        float *rect = (float *)*slot;
        float x = *(float *)(object + 0x80), z = *(float *)(object + 0x88);
        if (!(x >= rect[0]) || !(x <= rect[2]) || !(z >= rect[1]) || !(z <= rect[3])) return;
        uint32_t *list = (uint32_t *)(rect + 4);
        if ((int32_t)list[2] >= (int32_t)list[1]) ((void(__thiscall *)(void *, int32_t))GROW)(list, (int32_t)list[1] + 0x10);
        ((void **)(uintptr_t)list[0])[list[2]++] = object;
    }
};
// Weighted towards a point {x, y, z at 0; sums at +0x10; weight sum at +0x44; bias at +0x40; centres {count +0x4C,
// entries +0x50, capacity +0x54} of 16 bytes}: an item whose object isn't of kind 8 (+0x190) and whose level (+0x120)
// is under 4 has the centre of its box weighed by 1 / (squared distance to the point) - bias; when that's above 0, the
// weighted centre and the weight are added to the sums and the centre to the list (grown by 256 with GROW when full).
template <uintptr_t GROW> struct TakeWeighted {
    static void take(void *const *slot, uint8_t *item) {
        float *point = (float *)*slot;
        uint8_t *object = *(uint8_t **)(item + 0x18);
        if (*(int32_t *)(object + 0x190) == 8 || *(uint8_t *)(object + 0x120) >= 4) return;
        const float *box = (const float *)item;
        volatile float centre[3];
        for (int k = 0; k < 3; k++) centre[k] = (float)(((long double)box[k + 3] + box[k]) * 0.5f);
        long double dx = (long double)centre[0] - point[0], dy = (long double)centre[1] - point[1], dz = (long double)centre[2] - point[2];
        long double weight = 1.0L / ((dx * dx + dz * dz) + dy * dy) - point[0x10];
        if (!(weight > 0.0L)) return;
        volatile float weighted_z = (float)(weight * centre[2]);
        point[4] = (float)(centre[0] * weight + point[4]);
        point[5] = (float)(weight * centre[1] + point[5]);
        point[6] = (float)((long double)weighted_z + point[6]);
        int32_t &count = *(int32_t *)(point + 0x13), &capacity = *(int32_t *)(point + 0x15);
        point[0x11] = (float)(weight + point[0x11]);
        if (count == capacity) ((void(__thiscall *)(void *, int32_t))GROW)(point + 0x14, capacity + 0x100);
        uint32_t *entry = (uint32_t *)(*(uint8_t **)(point + 0x14) + count++ * 16);
        __builtin_memcpy(entry, (const void *)centre, 12);
    }
};
// The x87 square root itself (a library sqrtl might not round the same way).
static long double x87_sqrt(long double value) {
    long double root;
    __asm__("fsqrt" : "=t"(root) : "0"(value));
    return root;
}
// The best aim among the items' objects, kept in {best object, its value, its rank; +0xC the one to leave out; +0x10
// an owner to leave out; +0x14 the point aimed from; +0x18 the direction; +0x1C the furthest; +0x20 the least value}.
// An object owned (through its virtual slot 1 and the shared index at 0x130654C) by the owner left out isn't taken. Its
// position (+0x80) must be over 0.5 and at most the furthest away, and its value (the direction's dot product with the
// way to it, over the distance) above the least. Rank by its kind (+0x120): 0-2 1 when +0x190 is 8, 3 3, 7 2, others 0;
// a higher rank wins, then a lower value. Z_FIRST: the squared distance is summed z, y, x, with z squared from its value
// before and after rounding to float (as one copy has it); else x, z, y from the rounded values.
template <uintptr_t OWNER_OF, bool Z_FIRST> struct TakeAimed {
    static void take(void *const *slot, uint8_t *item) {
        uint32_t *best = (uint32_t *)*slot;
        uint8_t *object = *(uint8_t **)(item + 0x18);
        if ((uintptr_t)object == best[3]) return;
        if (best[4] && object) {
            uint8_t *info = ((uint8_t * (__thiscall *)(void *))(*(void ***)object)[1])(object);
            int32_t index = *(int32_t *)0x130654c;
            if (info && index < *(int32_t *)(info + 0x18)) {
                uint8_t *part = (*(uint8_t ***)(info + 0x14))[index];
                if (part) {
                    uint8_t *owned = object + *(int32_t *)(part + 0x10);
                    if (owned && ((uint32_t(__thiscall *)(void *))OWNER_OF)(owned) == best[4]) return;
                }
            }
        }
        const float *from = *(const float **)(best + 5), *direction = *(const float **)(best + 6);
        const float *position = (const float *)(object + 0x80);
        volatile float way[3];
        for (int k = 0; k < 2; k++) way[k] = (float)((long double)position[k] - from[k]);
        long double z = (long double)position[2] - from[2];
        way[2] = (float)z;
        long double distance = Z_FIRST ? x87_sqrt((z * way[2] + (long double)way[1] * way[1]) + (long double)way[0] * way[0])
                                       : x87_sqrt(((long double)way[0] * way[0] + (long double)way[2] * way[2]) + (long double)way[1] * way[1]);
        volatile float distance_stored = (float)distance;
        if (!(distance > 0.5L)) return;
        if (!(distance_stored <= *(float *)(best + 7))) return;
        long double value = (((long double)way[2] * direction[2] + (long double)way[1] * direction[1]) + (long double)way[0] * direction[0]) / distance_stored;
        volatile float value_stored = (float)value;
        if (!(value > *(float *)(best + 8))) return;
        uint8_t kind = *(object + 0x120);
        int32_t rank = kind <= 2 ? *(int32_t *)(object + 0x190) == 8 : kind == 3 ? 3 : kind == 7 ? 2 : 0;
        if (best[0]) {
            if (rank < (int32_t)best[2]) return;
            if (rank == (int32_t)best[2] && !(value_stored < *(float *)(best + 1))) return;
        }
        best[0] = (uint32_t)(uintptr_t)object;
        *(float *)(best + 1) = value_stored;
        best[2] = (uint32_t)rank;
    }
};
// Objects whose bits (+0x18, +0x1C) match a query's {table, ?, masks, bits, count, capacity, buffer} under its masks: the
// object's id (+8) goes into the buffer, handed on whole to the query's virtual slot 1 (count) when full (as Grid::Collect).
struct TakeMatching {
    static void take(void *const *slot, uint8_t *item) {
        uint8_t *object = *(uint8_t **)(item + 0x18);
        uint32_t *q = (uint32_t *)*slot;
        if (((*(uint32_t *)(object + 0x18) ^ q[4]) & q[2]) | ((*(uint32_t *)(object + 0x1c) ^ q[5]) & q[3])) return;
        ((uint32_t *)(uintptr_t)q[8])[q[6]] = *(uint32_t *)(object + 8);
        int32_t count = (int32_t)++q[6];
        if (count >= (int32_t)q[7] && count) {
            ((void(__thiscall *)(void *, int32_t))(*(void ***)q)[1])(q, count);
            q[6] = 0;
        }
    }
};

// ---- QuadTree::WalkBox(query, n, low, size): as WalkPlanes for a query box {low xyz, high xyz, +0x18 where items go}:
// the items overlapping it are taken (Take); the children are taken whole or walked again -----------------------------------
template <class Take, uintptr_t TAKE_ALL> static void __thiscall quad_walk_box(QuadTreeBox *tree, const float *query, int32_t n, const float *low, const float *size) {
    g_quad_walks++;
    QuadNode *node = &tree->nodes[n];
    if (node->other || node->item_count)
        for (int32_t i = 0; i < (int32_t)node->item_count; i++) {
            const float *box = (const float *)(node->items + i * 0x20);
            if (!*(uint32_t *)(node->items + i * 0x20 + 0x1c)) continue;
            if (!(box[0] <= query[3]) || !(box[3] >= query[0]) || !(box[1] <= query[4]) || !(box[4] >= query[1]) || !(box[2] <= query[5]) ||
                !(box[5] >= query[2]))
                continue;
            Take::take((void *const *)(query + 6), node->items + i * 0x20);
        }
    if (!(node->flags & 0xf)) return;
    int32_t first = n * 4 + 1;
    for (int32_t k = 0; k < 4; k++) {
        if (!(node->flags & (1u << k))) continue;
        volatile float half[2], child_low[2], high_x, high_z;
        quad_child_box(low, size, k, half, child_low, &high_x, &high_z);
        QuadNode *child = &tree->nodes[first + k];
        int32_t seen = box_overlaps_bounds(query, child_low[0], high_x, child->low, child->high, child_low[1], high_z);
        if (seen == 2) ((void(__thiscall *)(void *, int32_t, const float *))TAKE_ALL)(tree, first + k, query);
        else if (seen == 1) quad_walk_box<Take, TAKE_ALL>(tree, query, first + k, (const float *)child_low, (const float *)half);
    }
}
static const uint8_t quad_walk_box_original[] = {0x83, 0xec, 0x20, 0x8b, 0x54, 0x24, 0x28, 0x6b};
REWRITE("QuadTree::WalkBox", 0x7b8920, quad_walk_box_original, (&quad_walk_box<TakeInto64, 0x7b7390>), &g_quad_walks);
REWRITE("QuadTree::WalkBox", 0xb5e460, quad_walk_box_original, (&quad_walk_box<TakeInto64, 0xb5d780>), &g_quad_walks);
REWRITE("QuadTree::WalkBox", 0xb74580, quad_walk_box_original, (&quad_walk_box<TakeInto64, 0xb732b0>), &g_quad_walks);

// ---- QuadTree::TakeAll(n, query): node n is wholly inside the view: every item in use is shown (SHOW, with a 0 when
// SHOW_TAKES_ZERO, or virtual slot 8 of its object) and the children it marks are taken the same way -----------------------
template <uintptr_t SHOW, bool SHOW_TAKES_ZERO> static void __thiscall quad_take_all(QuadTreeBox *tree, int32_t n, const float *query) {
    g_quad_walks++;
    QuadNode *node = &tree->nodes[n];
    if (node->other || node->item_count)
        for (int32_t i = 0; i < (int32_t)node->item_count; i++) {
            if (!*(uint32_t *)(node->items + i * 0x20 + 0x1c)) continue;
            void *object = *(void **)(node->items + i * 0x20 + 0x18);
            if (SHOW_TAKES_ZERO) ((void(__thiscall *)(void *, int32_t))SHOW)(object, 0);
            else ((void(__thiscall *)(void *))(*(void ***)object)[8])(object);
        }
    if (!(node->flags & 0xf)) return;
    for (int32_t k = 0; k < 4; k++)
        if (node->flags & (1u << k)) quad_take_all<SHOW, SHOW_TAKES_ZERO>(tree, n * 4 + 1 + k, query);
}
static const uint8_t quad_take_all_original[] = {0x53, 0x55, 0x56, 0x8b, 0x74, 0x24, 0x10, 0x6b};
REWRITE("QuadTree::TakeAll", 0x7b85f0, quad_take_all_original, (&quad_take_all<0, false>), &g_quad_walks);
REWRITE("QuadTree::TakeAll", 0xb5e950, quad_take_all_original, (&quad_take_all<0xb563b0, true>), &g_quad_walks);

// ---- QuadTree::TakeAllBox(n, query): node n is wholly inside the query box: every item in use is taken (Take), and the
// marked children the same way ------------------------------------------------------------------------------------------------
template <class Take> static void __thiscall quad_take_all_box(QuadTreeBox *tree, int32_t n, const float *query) {
    g_quad_walks++;
    QuadNode *node = &tree->nodes[n];
    if (node->other || node->item_count)
        for (int32_t i = 0; i < (int32_t)node->item_count; i++)
            if (*(uint32_t *)(node->items + i * 0x20 + 0x1c)) Take::take((void *const *)(query + 6), node->items + i * 0x20);
    if (!(node->flags & 0xf)) return;
    for (int32_t k = 0; k < 4; k++)
        if (node->flags & (1u << k)) quad_take_all_box<Take>(tree, n * 4 + 1 + k, query);
}
static const uint8_t quad_take_all_rect_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
static const uint8_t quad_walk_box_rect_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec};
REWRITE("QuadTree::TakeAllRect", 0x7b6d80, quad_take_all_rect_original, (&quad_take_all_box<TakeInRect<0x72af00>>), &g_quad_walks);
REWRITE("QuadTree::WalkBoxRect", 0x7b82d0, quad_walk_box_rect_original, (&quad_walk_box<TakeInRect<0x72af00>, 0x7b6d80>), &g_quad_walks);
REWRITE("QuadTree::TakeAllWeighted", 0x7b6bd0, quad_take_all_rect_original, (&quad_take_all_box<TakeWeighted<0x417a20>>), &g_quad_walks);
REWRITE("QuadTree::WalkBoxWeighted", 0x7b7f30, quad_walk_box_rect_original, (&quad_walk_box<TakeWeighted<0x417a20>, 0x7b6bd0>), &g_quad_walks);
REWRITE("QuadTree::TakeAllAimed", 0x7b5540, quad_take_all_rect_original, (&quad_take_all_box<TakeAimed<0x758b10, false>>), &g_quad_walks);
REWRITE("QuadTree::WalkBoxAimed", 0x7b6eb0, quad_walk_box_rect_original, (&quad_walk_box<TakeAimed<0x758b10, true>, 0x7b5540>), &g_quad_walks);
static const uint8_t quad_walk_box_matching_original[] = {0x83, 0xec, 0x20, 0x8b, 0x54, 0x24, 0x28, 0x6b};
REWRITE("QuadTree::WalkBoxMatching", 0x48b0b0, quad_walk_box_matching_original, (&quad_walk_box<TakeMatching, 0x48a4e0>), &g_quad_walks);
static const uint8_t quad_walk_planes_matching_original[] = {0x83, 0xec, 0x24, 0x8b, 0x51, 0x08, 0x53, 0x55};
REWRITE("QuadTree::WalkPlanesMatching", 0x48bdc0, quad_walk_planes_matching_original, (&quad_walk_planes<0x52, 0x48b370, TakeMatching>), &g_quad_walks);
