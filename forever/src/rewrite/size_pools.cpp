// List::Find87BCB0 and List::Find909C10 (one function written out twice; tools/family.py: size_pools.inc; they
// differ in the pool list's resize, an Array<20>::SetCapacity of their own): take an item of n dwords from pools
// kept by size (thiscall: n; gives the item, or 0 for n of 0 or less). The object is {pools, count}; a pool is
// {items a block, blocks, block count, free items, free count}.
//   - When there is no pool for n yet: the list made n + 1 long; when it was empty, pool 0 cleared and counted;
//     each new pool given 0x1000 / (n * 4) items a block (unsigned; at least 1; the same n for every new pool, as
//     the game has it) and the rest cleared.
//   - When pool n has no free item: one more block. The block list made one longer (aligned memory, the old entries
//     copied, the old list freed), the free list freed and made again for every block's items; the new block (items
//     a block * n dwords) allocated and its items put on the free list from the last to the first.
//   - The last free item taken.
#include "rewrite.h"

namespace {
struct SizePools {
    uint32_t at, resize;
    uint32_t unit = 4;   // (an item is n of these bytes)
};
#define SIZE_POOLS(at, resize, ...) {at, resize},
constexpr SizePools size_pools[] = {
#include "size_pools.inc"
};
#undef SIZE_POOLS
#define SIZE_POOLS(at, resize, ...) {__VA_ARGS__},
const uint8_t size_pools_first[][7] = {
#include "size_pools.inc"
};
#undef SIZE_POOLS

struct Pool {
    int32_t per_block;
    uint8_t **blocks;
    int32_t block_count;
    uint8_t **free;
    int32_t free_count;
};
typedef void *(__cdecl *Alloc)(uint32_t);
typedef void(__cdecl *Free)(void *);
const Alloc alloc_aligned = (Alloc)0xbcdfd0;
const Free free_aligned = (Free)0xbce010;

volatile long g_size_pools;
uint8_t *take(uint32_t *self, int32_t n, const SizePools &kind) {
    g_size_pools++;
    int32_t have = (int32_t)self[1];
    if (n >= have) {
        ((void(__thiscall *)(void *, int32_t))kind.resize)(self, n + 1);
        if (have == 0) {
            Pool *first = *(Pool **)self;
            first->per_block = 0, first->blocks = nullptr, first->block_count = 0, first->free = nullptr, first->free_count = 0;
            have = 1;
        }
        if (have <= n) {
            const int32_t per = (int32_t)(0x1000u / ((uint32_t)n * kind.unit));
            for (int32_t i = have; i <= n; i++) {
                Pool *p = *(Pool **)self + i;
                p->per_block = per;
                if (p->per_block < 1) p->per_block = 1;
                p->blocks = nullptr, p->block_count = 0, p->free = nullptr, p->free_count = 0;
            }
        }
    }
    if (n <= 0) return nullptr;
    Pool *p = *(Pool **)self + n;
    if (p->free_count <= 0) {
        const int32_t blocks = p->block_count + 1;
        uint8_t **old = p->blocks;
        p->blocks = (uint8_t **)alloc_aligned((uint32_t)blocks * 4);
        for (int32_t i = 0; i < p->block_count; i++) p->blocks[i] = old[i];
        free_aligned(old);
        free_aligned(p->free);
        p->free = (uint8_t **)alloc_aligned((uint32_t)p->per_block * (uint32_t)blocks * 4);   // (wrapping as the game's does)
        for (int32_t b = blocks - 1; b >= p->block_count; b--) {
            p->blocks[b] = (uint8_t *)alloc_aligned((uint32_t)p->per_block * (uint32_t)n * kind.unit);
            for (int32_t k = p->per_block - 1; k >= 0; k--) p->free[p->free_count++] = p->blocks[b] + (uint32_t)k * (uint32_t)n * kind.unit;
        }
        p->block_count = blocks;
    }
    return p->free[--p->free_count];
}

template <int K> uint8_t *__thiscall take_copy(uint32_t *self, int32_t n) { return take(self, n, size_pools[K]); }
constexpr SizePools pools_16 = {0xce76b0, 0xce7040, 16};   // List::FindCE76B0: items of n 16-byte units (row written here)
const uint8_t pools_16_first[] = {0x83, 0xec, 0x08, 0x53, 0x8b, 0x5c, 0x24};
uint8_t *__thiscall take_16(uint32_t *self, int32_t n) { return take(self, n, pools_16); }
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(size_pools[0].at == 0x87bcb0 && size_pools[1].at == 0x909c10, "size_pools.inc changed");
REWRITE("SizePools::Take", 0x87bcb0, size_pools_first[0], (&take_copy<0>), &g_size_pools);
REWRITE("SizePools::Take", 0x909c10, size_pools_first[1], (&take_copy<1>), &g_size_pools);
REWRITE("SizePools::Take", 0xce76b0, pools_16_first, &take_16, &g_size_pools);
