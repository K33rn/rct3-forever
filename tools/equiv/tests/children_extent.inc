// Window::ChildrenExtent (forever/src/rewrite/children_extent.cpp). The window at arena 0x100: its children array at
// 0x400 (0-13 children, or a count below 0), each child at 0x800 + 0x40 * i with its right edge and bottom random
// (NaN, infinity, negative, +/-0, denormal, signalling NaN among them); the result at 0x60.
#if EQUIV_PART == 1
static uint32_t ce_bits(float f) {
    uint32_t bits;
    memcpy(&bits, &f, 4);
    return bits;
}
static uint32_t ce_value(Rng &r) {   // (as bits: a float returned through the FPU would come back with a signalling NaN quiet)
    switch (r.range(0, 10)) {
    case 10: return r.range(0, 1) ? 0x80000000u : 0;
    case 0: return 0x7fc00000u;
    case 1: return r.range(0, 1) ? 0x7f800000u : 0xff800000u;
    case 2: return r.range(0, 1) ? 0x7fa00001u : r.next() & 0x807fffffu;   // a signalling NaN or a denormal
    case 3: return ce_bits(r.any_float());
    default: return ce_bits(r.real(-500, 900));
    }
}
#else
    Test children_extent{0x9c2810,
                         [=](Rng &r) {
                             fresh(r);
                             int32_t count = r.range(0, 13);
                             if (r.range(0, 9) == 0) count = -r.range(1, 5);
                             *(int32_t *)(arena<uint8_t>(0x100) + 0xe4) = count;
                             *(void **)(arena<uint8_t>(0x100) + 0xe0) = arena<void>(0x400);
                             for (int i = 0; i < 14; i++) {
                                 uint8_t *child = arena<uint8_t>(0x800 + 0x40 * i);
                                 arena<void *>(0x400)[i] = child - 0x60;   // (the edges at +0x84 / +0x88 of the child)
                                 const uint32_t right = ce_value(r), bottom = ce_value(r);
                                 memcpy(child + 0x24, &right, 4);
                                 memcpy(child + 0x28, &bottom, 4);
                             }
                         },
                         [](void *fn) {
                             ((void(__fastcall *)(void *, void *, void *))fn)(arena<void>(0x100), nullptr, arena<void>(0x60));
                             return Result{arena<uint32_t>(0x60)[0] ^ arena<uint32_t>(0x60)[1] * 3};
                         }};
    children_extent.iterations = 3000;
    quick(children_extent);
#endif
