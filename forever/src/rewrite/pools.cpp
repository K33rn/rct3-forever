// Object pools: items handed out from blocks of a fixed number of items, the freed ones kept on a free list. When
// every item of a pool is free again, its blocks go back to memory. A pool set is an array of pools (one per kind
// of item); the compiler made a copy of Free for every item type.
#include "rewrite.h"

struct Pool {
    int32_t items_per_block;
    void **blocks;
    int32_t block_count;
    void **free;          // the free list
    int32_t free_count;
};

struct PoolSet {
    Pool *pools;
};

static void(__cdecl *const memory_free_aligned)(void *) = (void(__cdecl *)(void *))0xbce010;

static volatile long g_frees;

// Free(kind, item)
static void __fastcall pool_free(PoolSet *set, void *, int32_t kind, void *item) {
    g_frees++;
    Pool *p = &set->pools[kind];
    p->free[p->free_count++] = item;
    if (p->free_count < p->items_per_block * p->block_count) return;
    for (int32_t i = 0; i < p->block_count; i++) memory_free_aligned(p->blocks[i]);
    memory_free_aligned(p->blocks);
    memory_free_aligned(p->free);
    p->blocks = nullptr;
    p->block_count = 0;
    p->free = nullptr;
    p->free_count = 0;
}

static const uint32_t free_copies[] = {
    0x8501c0, 0x8503d0, 0x8505e0, 0x8507f0, 0x850a00, 0x850c10, 0x850e20, 0x851030,
    0x851240, 0x851450, 0x8516a0, 0x8518b0, 0x969480, 0x969760, 0x96a070, 0x96a350,
    0x96dcc0, 0x96dfa0, 0x96e280, 0x96e560,
};
static const uint8_t free_original[] = {0x8b, 0x09, 0x8b, 0x44, 0x24, 0x04, 0x53, 0x8d,
                                        0x04, 0x80, 0x8b, 0x54, 0x81, 0x10, 0x56, 0x8d};
REWRITE_COPIES("Pool::Free", free_copies, free_original, pool_free, &g_frees);
