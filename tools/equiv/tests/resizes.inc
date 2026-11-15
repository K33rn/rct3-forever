// Array::ResizeBig (wave2.cpp's w2d_resize for a copy with big records): as the Array::Resize test in wave2_tests.inc
// with counts up to 3 (the records are 0x1FB4 bytes) and the copy's own copier.
#if EQUIV_PART == 1
static uintptr_t rz_copier(const rewrite::Entry *e) { return e->at == 0x445a00 ? 0x445440 : 0; }   // (the copy's copier, as its code calls it)
#else
    w2c_add(0x445a00,
            [=](Rng &r) {
                w2c_seed(r);
                arena<void *>(0x100)[0] = r.range(0, 4) ? arena<void>(0x300) : nullptr;
                arena<int32_t>(0x100)[1] = r.range(0, 3);
                arena<int32_t>(0x20)[0] = r.range(0, 4) ? r.range(0, 3) : arena<int32_t>(0x100)[1];
            },
            w2c_self_arg,
            [](const rewrite::Entry *e) {
                stubs_for_now({{0xbce010, (void *)w2c_free<0xe2d7>}, {0xbcdfd0, (void *)w2d_alloc_always}, {0x402b90, (void *)w2d_vector_make},
                               {rz_copier(e), (void *)w2c_s1<0xe2d5>}});
            });
    // Array::ResizeSizedOther (containers.cpp's solo_resize for a copy whose copy call lies elsewhere): solo_resize's test
    // with this copy's own copier
    Test solo_resize_other{0x82a2a0, solo_resize.setup, solo_resize.call};
    solo_resize_other.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfd0, (void *)stub_alloc_next}, {0x402b90, (void *)stub_vector_construct4}, {0x8283b0, (void *)stub_item_copy}});
    };
    solo_resize_other.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(solo_resize_other);
#endif
