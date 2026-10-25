// Growable arrays of 4-byte items (pointers, ids, numbers): {data, capacity}. The compiler made a copy of these
// functions for every item type.
#include "rewrite.h"

struct Array32 {
    uint32_t *data;     // a 16-byte aligned block, or null
    int32_t capacity;   // items
};

static void *(__cdecl *const memory_alloc_aligned)(uint32_t) = (void *(__cdecl *)(uint32_t))0xbcdfd0;
static void(__cdecl *const memory_free_aligned)(void *) = (void(__cdecl *)(void *))0xbce010;

// ---- SetCapacity(n): reallocate to n items, keeping the first min(n, capacity); returns n ------------------

static volatile long g_set_capacity;

static int32_t __fastcall set_capacity(Array32 *a, void *, int32_t n) {
    g_set_capacity++;
    if (n == a->capacity) return n;
    uint32_t *fresh = nullptr;
    if (n) {
        fresh = (uint32_t *)memory_alloc_aligned((uint32_t)n * 4);
        int32_t keep = n < a->capacity ? n : a->capacity;
        for (int32_t i = 0; i < keep; i++) fresh[i] = a->data[i];
    }
    if (a->data) {
        memory_free_aligned(a->data);
        a->data = nullptr;
    }
    a->data = fresh;
    a->capacity = n;
    return n;
}

static const uint32_t set_capacity_copies[] = {
    0x417c20, 0x41b950, 0x41ba40, 0x41bb30, 0x426d40, 0x42a4f0, 0x42f920, 0x449290,
    0x44d040, 0x44da90, 0x45e920, 0x45ea10, 0x46b1c0, 0x46c500, 0x4710b0, 0x495de0,
    0x4a8df0, 0x4c9d80, 0x4cc5f0, 0x5020f0, 0x50cbb0, 0x53c6e0, 0x5902a0, 0x59a430,
    0x5a2510, 0x5d3c50, 0x5f1630, 0x5f1700, 0x5fc2e0, 0x6216f0, 0x65b4f0, 0x6606b0,
    0x66a4c0, 0x6719e0, 0x685630, 0x6856e0, 0x69eff0, 0x6c3a80, 0x6c3b30, 0x6cff20,
    0x6d9780, 0x6f11e0, 0x72af00, 0x745bd0, 0x745c80, 0x750040, 0x751110, 0x7735e0,
    0x787dc0, 0x787e90, 0x788f70, 0x7a40e0, 0x7f0660, 0x7f7200, 0x7fb570, 0x7fb620,
    0x7fb6f0, 0x7fb7c0, 0x7fb870, 0x8011d0, 0x8012a0, 0x801370, 0x805b80, 0x8078c0,
    0x80eb00, 0x80ebb0, 0x814a20, 0x8402b0, 0x8446d0, 0x84e280, 0x84e370, 0x87a640,
    0x8b6670, 0x926200, 0x9262b0, 0x958a00, 0x982b00, 0x982bd0, 0x9847e0, 0x99be90,
    0x9a9e20, 0x9a9f10, 0x9bc0c0, 0x9bc1b0, 0x9c6490, 0x9ce260, 0x9ce310, 0x9ce3c0,
    0x9ce530, 0x9ce5e0, 0x9d65f0, 0x9f8a90, 0xa01c30, 0xa5b4a0, 0xa5bdc0, 0xa6b660,
    0xa6b830, 0xa6b8e0, 0xa6b990, 0xa6bb10, 0xa90840, 0xa9ee30, 0xb10b80, 0xb6ba10,
    0xb826b0, 0xbd1fc0, 0xbdd640, 0xc71a10, 0xc774e0, 0xc96370, 0xcdb8a0, 0xcdeca0,
    0xce2d80, 0xce71d0,
};
static const uint8_t set_capacity_original[] = {0x83, 0xec, 0x08, 0x53, 0x8b, 0x5c, 0x24, 0x10,
                                                0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x04, 0x3b, 0xd8};
REWRITE_COPIES("Array32::SetCapacity", set_capacity_copies, set_capacity_original, set_capacity, &g_set_capacity);

// ---- Allocate(n): drop the contents and make room for n items (not cleared); returns n ---------------------

static volatile long g_allocate;

static int32_t __fastcall allocate(Array32 *a, void *, int32_t n) {
    g_allocate++;
    if (a->data) {
        memory_free_aligned(a->data);
        a->data = nullptr;
    }
    if (n) a->data = (uint32_t *)memory_alloc_aligned((uint32_t)n * 4);
    a->capacity = n;
    return n;
}

static const uint32_t allocate_copies[] = {
    0x41b910, 0x41ba00, 0x41baf0, 0x423da0, 0x42f8e0, 0x45b0a0, 0x45e880, 0x45e9d0,
    0x477f40, 0x48df50, 0x58e7f0, 0x646b00, 0x66a480, 0x671980, 0x67c8e0, 0x6855f0,
    0x6b9f70, 0x6c3a40, 0x6cfee0, 0x733790, 0x733840, 0x7a40a0, 0x7f71c0, 0x84e330,
    0x8fe100, 0x9589c0, 0x994c80, 0x9a9de0, 0x9a9ed0, 0x9bc170, 0x9c2620, 0x9c6450,
    0x9ce4f0, 0xa2d600, 0xa357e0, 0xa40630, 0xa47880, 0xa478e0, 0xa47940, 0xa51760,
    0xa5b460, 0xa5b550, 0xa63cd0, 0xa6b620, 0xa805d0, 0xa90800, 0xa9edf0, 0xac7a00,
    0xaf76b0, 0xb994c0, 0xbd1f80, 0xcb5dc0, 0xcc8450,
};
static const uint8_t allocate_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x06, 0x85, 0xc0, 0x57, 0x74, 0x0f, 0x50};
REWRITE_COPIES("Array32::Allocate", allocate_copies, allocate_original, allocate, &g_allocate);
