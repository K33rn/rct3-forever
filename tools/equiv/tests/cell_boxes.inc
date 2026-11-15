// Terrain::TellCell (forever/src/rewrite/cell_boxes.cpp). Terrain::Get gives a terrain at arena 0x300 (its grid at
// 0x400: counts up to 255, random cell sizes and origin with NaN and infinity among them); cells inside and outside;
// the one asking at 0x600 with a change waiting or not; the object told at 0x500.
#if EQUIV_PART == 1
static void *__cdecl cb_terrain() { dev_note(320, 1); return arena<void>(0x300); }
static void __fastcall cb_activate(uint8_t *t) { dev_note(321, (uint32_t)(uintptr_t)t); }
static void __thiscall cb_tell(void *object, const uint32_t *low, const uint32_t *high, void *asking) {
    dev_note(322, (uint32_t)(uintptr_t)object); dev_note(322, (uint32_t)(uintptr_t)asking);
    for (int k = 0; k < 3; k++) dev_note(322, low[k]), dev_note(322, high[k]);
}
static void __thiscall cb_change(uint8_t *t, void *at, uint32_t change) { dev_note(323, (uint32_t)(uintptr_t)at - (uint32_t)(uintptr_t)t); dev_note(323, change); }
static void *cb_table[1] = {(void *)cb_change};
static float cb_value(Rng &r) {
    switch (r.range(0, 11)) {
    case 0: return NAN;
    case 1: return -INFINITY;
    case 2: return r.any_float();
    default: return r.real(-4000, 4000);
    }
}
#else
    Test cell_boxes{0x7bb530,
                    [=](Rng &r) {
                        fresh(r);
                        arena<uint32_t>(0x20)[0] = 0;
                        uint8_t *t = arena<uint8_t>(0x300), *g = arena<uint8_t>(0x400);
                        *(void **)t = cb_table;
                        t[0x2c] = (uint8_t)r.next();
                        *(void **)(t + 0x54) = g;
                        *(void **)arena<void>(0x600) = cb_table;
                        *(uint32_t *)(arena<uint8_t>(0x600) + 0x104) = r.range(0, 1) ? 0 : r.next() | 1;
                        g[4] = (uint8_t)r.range(0, 255), g[5] = (uint8_t)r.range(0, 255);
                        for (uint32_t off = 0xc; off <= 0x18; off += 4) *(float *)(g + off) = cb_value(r);
                        for (uint32_t off : {0x24u, 0x4cu, 0x64u}) *(void **)(arena<uint8_t>(0x100) + off) = arena<void>(0x500 + off);
                        const int32_t odd[] = {-1, 0, 255, 256, INT32_MIN, 0x7ffffffe};
                        arena<int32_t>(0x40)[0] = r.range(0, 3) ? r.range(-5, 260) : odd[r.range(0, 5)];
                        arena<int32_t>(0x40)[1] = r.range(0, 3) ? r.range(-5, 260) : odd[r.range(0, 5)];
                        arena<uint32_t>(0x40)[2] = (uint32_t)(uintptr_t)arena<void>(0x600);
                    },
                    [](void *fn) {
                        const int32_t *a = arena<int32_t>(0x40);
                        ((void(__thiscall *)(void *, int32_t, int32_t, int32_t))fn)(arena<void>(0x100), a[0], a[1], a[2]);
                        return Result{*arena<uint32_t>(0x20)};
                    }};
    cell_boxes.iterations = 2000;
    cell_boxes.prepare = [](const rewrite::Entry *e) {
        const uint32_t tell = e->at == 0x7bb530 ? 0x7b9d30 : e->at == 0xb5f5e0 ? 0xb5f370 : 0xb75670;
        stubs_for_now({{0x40a090, (void *)cb_terrain}, {0x800860, (void *)cb_activate}, {tell, (void *)cb_tell}});
    };
    cell_boxes.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(cell_boxes);
#endif
