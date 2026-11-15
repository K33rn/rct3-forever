// Items::Next (forever/src/rewrite/item_lists.cpp). The object at arena 0x100 with its list {items, used, made} at the
// copy's place: used below, at or (oddly) past what is made; none made yet. Memory of its own from 0x6000 (lists of
// up to 200 items are bigger than dev_take's 0x100 steps), filled with junk; item memory sometimes none.
#if EQUIV_PART == 1
static void *__cdecl il_alloc_aligned(uint32_t size) {
    dev_note(360, size);
    uint32_t &next = *arena<uint32_t>(0x30);
    void *p = arena<void>(next);
    next += (size + 15) & ~15u;
    return p;
}
static void *__cdecl il_alloc(uint32_t size) { dev_note(361, size); return dev_next() % 7 == 0 ? nullptr : dev_take(size); }
#else
    Test item_lists{0x42bf40,
                    [=](Rng &r) {
                        fresh(r);
                        arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                        arena<uint32_t>(0x30)[0] = 0x6000;
                        for (int i = 0; i < 0x3000; i++) arena<uint32_t>(0x2000)[i] = r.next();   // (all new memory holds junk: what is cleared shows)
                        uint32_t *list = (uint32_t *)dev_take(4 * 64);
                        for (int k = 0; k < 64; k++) list[k] = r.next();
                        for (uint32_t used : {0x40u, 0x4cu, 0x1cu, 0x28u}) {
                            uint32_t *at = (uint32_t *)(arena<uint8_t>(0x100) + used);
                            const int32_t made = r.range(0, 3) ? r.range(0, 60) : 0;
                            at[-1] = made ? (uint32_t)(uintptr_t)list : 0;
                            at[0] = (uint32_t)(!made ? 0 : r.range(0, 1) ? made : r.range(0, 3) ? r.range(0, made) : made + r.range(0, 3));   // (none made: none used)
                            at[1] = (uint32_t)made;
                        }
                    },
                    [](void *fn) {
                        const uint32_t back = ((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100));
                        return Result{back ^ *arena<uint32_t>(0x20)};
                    }};
    item_lists.iterations = 1000;
    item_lists.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfd0, (void *)il_alloc_aligned}, {0xbcdfc0, (void *)il_alloc}, {0xbce010, (void *)cheat_free_aligned}});
    };
    item_lists.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(item_lists);
#endif
