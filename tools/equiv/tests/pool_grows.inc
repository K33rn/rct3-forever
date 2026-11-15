// Pool::GrowCopied (forever/src/rewrite/containers.cpp's pool_grow, for the copies whose size is worked out another
// way): as the Pool::Grow tests in equiv.cpp (their setup and call), with the copy's own copier.
#if EQUIV_PART == 1
static uintptr_t pg_copier(const rewrite::Entry *e) {   // (each copy's copier, as its code calls it)
    switch (e->at) {
    case 0x94e250: return 0x94d850;
    case 0x964bc0: return 0x9648f0;
    case 0x9f98a0: return 0x9f89b0;
    }
    return 0;
}
#else
    Test pool_grow_copied{0x94e250, pool_grow_setup, pool_grow_call};
    pool_grow_copied.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{0xbcdfd0, (void *)stub_alloc_next}, {0x402b90, (void *)stub_vector_construct4}, {pg_copier(e), (void *)stub_item_copy}});
    };
    pool_grow_copied.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(pool_grow_copied);
#endif
// Pool::GrowFields (containers.cpp's pool_grow with a list of the fields each item's copy takes): as above, with no
// copier; the old items' memory is junk, so a field taken or left shows.
#if EQUIV_PART == 2
    Test pool_grow_fields{0x9f7fc0, pool_grow_setup, pool_grow_call};
    pool_grow_fields.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbcdfd0, (void *)stub_alloc_next}, {0x402b90, (void *)stub_vector_construct4}}); };
    pool_grow_fields.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(pool_grow_fields);
#endif
