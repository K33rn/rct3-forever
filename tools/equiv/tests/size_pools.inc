// SizePools::Take (forever/src/rewrite/size_pools.cpp). The pools object at arena 0x100 {pools, count}; its pool list
// (20 bytes a pool) and the pools' old blocks and free lists from dev_take, new memory from 0x6000, some pools already holding blocks and free
// items. The copy's own list resize (an Array<20>::SetCapacity) runs as the game's, with the memory functions
// stood in for.
#if EQUIV_PART == 1
static void *__cdecl sp_alloc(uint32_t size) {   // (blocks are up to 4 KB: dev_take moves on at most 0x100, so memory of its own)
    dev_note(300, size);
    uint32_t &next = *arena<uint32_t>(0x30);
    void *p = arena<void>(next);
    next += (size + 15) & ~15u;
    return p;
}
#else
    Test size_pools{0x87bcb0,
                    [=](Rng &r) {
                        fresh(r);
                        arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                        arena<uint32_t>(0x30)[0] = 0x6000;
                        for (int i = 0; i < 0x1800; i++) arena<uint32_t>(0x6000)[i] = r.next();   // (new memory holds junk, so what is cleared shows)
                        uint32_t *self = arena<uint32_t>(0x100);
                        const int32_t count = r.range(0, 6);
                        uint32_t *pools = (uint32_t *)dev_take(20 * 8);
                        self[0] = count ? (uint32_t)(uintptr_t)pools : 0, self[1] = (uint32_t)count;
                        for (int32_t i = 0; i < count; i++) {
                            uint32_t *p = pools + 5 * i;
                            const int32_t blocks = r.range(0, 2), per = r.range(1, 4);
                            uint32_t *list = (uint32_t *)dev_take(4 * 4), *free = (uint32_t *)dev_take(4 * 16);
                            for (int k = 0; k < 4; k++) list[k] = r.next();
                            for (int k = 0; k < 16; k++) free[k] = r.next();
                            p[0] = (uint32_t)per, p[1] = (uint32_t)(uintptr_t)list, p[2] = (uint32_t)blocks, p[3] = (uint32_t)(uintptr_t)free;
                            p[4] = (uint32_t)(blocks ? r.range(-1, per * blocks) : 0);
                        }
                        arena<int32_t>(0x40)[0] = r.range(0, 4) ? r.range(-2, 12) : r.range(1, count + 1);
                    },
                    [](void *fn) {
                        void *back = ((void *(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0]);
                        return Result{(uint32_t)(uintptr_t)back ^ *arena<uint32_t>(0x20)};
                    }};
    size_pools.iterations = 1000;
    size_pools.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfd0, (void *)sp_alloc}, {0xbce010, (void *)cheat_free_aligned}});
    };
    size_pools.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(size_pools);
#endif
