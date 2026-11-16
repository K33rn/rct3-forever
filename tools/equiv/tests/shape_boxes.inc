// Scenery::ShapeBox (forever/src/rewrite/shape_boxes.cpp). A box (low corner at arena 0x100, high at 0x120) and a
// matrix (0x140) of random floats: coordinates often equal between the corners, the matrix sometimes plain (turning
// nothing) so the swaps and the y limits are met both ways, now and then a NaN, -0 or a tiny value (whose products
// round to 0 as floats but not at the FPU's width); a quarter of the cases use only 0, -0 and +-1e-25, where the
// compares meet -0 against 0 and sums too small for a float. The check's stand-in notes the object and
// the high corner and sometimes moves its y (so the height must be taken after it). The result: the six floats' bits.
#if EQUIV_PART == 1
static void __fastcall sb_check(void *object, void *, float *high) {
    dev_note(800, (uint32_t)(uintptr_t)object); dev_note(800, *(uint32_t *)&high[0]); dev_note(800, *(uint32_t *)&high[1]);
    if (arena<uint32_t>(0x50)[0] & 1) high[1] = arena<float>(0x50)[1];
}
static void __fastcall sb_check_b(void *object, void *e, float *high) { dev_note(801, 2); sb_check(object, e, high); }
static bool g_sb_small;   // (a case of near-zero values only: 0, -0, +-1e-25)
static float sb_float(Rng &r) {
    if (g_sb_small) { const float v[4] = {0.0f, -0.0f, 1e-25f, -1e-25f}; return v[r.range(0, 3)]; }
    switch (r.range(0, 11)) {
    case 0: return 0.0f;
    case 1: return 1.0f;
    case 2: { uint32_t nan = 0x7fc00000u; float f; memcpy(&f, &nan, 4); return f; }
    case 3: return -1.0f;
    case 4: return -0.0f;
    case 5: return r.range(0, 1) ? 1e-25f : -1e-25f;   // (products of two of these: too small for a float, not for the FPU)
    default: return (float)(int32_t)(r.next() % 20001u - 10000) / 64.0f + (float)(r.next() & 0xffff) / 65536.0f;
    }
}
#else
    Test shape_box{0x864010,
                   [=](Rng &r) {
                       fresh(r);
                       g_sb_small = r.range(0, 3) == 0;
                       float *low = arena<float>(0x100), *high = arena<float>(0x120), *m = arena<float>(0x140);
                       for (int k = 0; k < 3; k++) {
                           low[k] = sb_float(r);
                           high[k] = r.range(0, 3) == 0 ? low[k] : sb_float(r);
                       }
                       bool plain = r.range(0, 2) == 0;
                       for (int k = 0; k < 16; k++) m[k] = plain ? ((k % 5 == 0) ? 1.0f : (k >= 12 && r.range(0, 1) ? sb_float(r) : 0.0f)) : sb_float(r);
                       if (r.range(0, 4) == 0) m[r.range(0, 15)] = -m[r.range(0, 15)];
                       arena<uint32_t>(0x50)[0] = r.next(), arena<float>(0x50)[1] = sb_float(r);
                   },
                   [](void *fn) {
                       ((void(__fastcall *)(void *, void *, void *, void *))fn)(arena<void>(0x180), arena<void>(0x100), arena<void>(0x120), arena<void>(0x140));
                       uint32_t h = 0;
                       for (int k = 0; k < 3; k++) h = h * 31 + arena<uint32_t>(0x100)[k], h = h * 31 + arena<uint32_t>(0x120)[k];
                       return Result{h};
                   }};
    shape_box.iterations = 1000;
    shape_box.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x863c30, (void *)sb_check}, {0x864150, (void *)sb_check_b}}); };
    shape_box.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(shape_box);
#endif
