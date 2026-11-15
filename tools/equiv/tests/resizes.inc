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
#endif
