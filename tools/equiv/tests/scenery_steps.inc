// Scenery::SetStep (forever/src/rewrite/scenery_steps.cpp). The scenery at arena 0x100; its reference at 0x300 to an
// object at 0x400 (or none) whose class info at 0x500 has the entry for the class number or not; the interface kept
// already or not. Unlike farther_point's, no interface is fine here.
#if EQUIV_PART == 1
static uint32_t g_ss_saved;
static void *__thiscall ss_info(uint8_t *object) { dev_note(250, (uint32_t)(uintptr_t)object); return *(uint8_t **)(object + 4); }
static void *ss_table[2] = {nullptr, (void *)ss_info};
static uint32_t __thiscall ss_step(void *to, uint32_t which, uint32_t value) {
    dev_note(251, (uint32_t)(uintptr_t)to); dev_note(251, which); dev_note(251, value);
    return dev_next();
}
#else
    Test scenery_steps{0x848840,
                       [=](Rng &r) {
                           fresh(r);
                           arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                           uint8_t *self = arena<uint8_t>(0x100), *ref = arena<uint8_t>(0x300), *object = arena<uint8_t>(0x400), *info = arena<uint8_t>(0x500);
                           const int32_t number = r.range(-1, 3);
                           *(int32_t *)(uintptr_t)0x130b5c8 = number;
                           *(void **)(self + 0x58) = ref;
                           *(void **)(ref + 0xc) = r.range(0, 5) ? object : nullptr;
                           *(void **)object = ss_table;
                           *(void **)(object + 4) = r.range(0, 5) ? info : nullptr;
                           *(int32_t *)(info + 0x18) = r.range(0, 4);
                           *(void **)(info + 0x14) = info + 0x40;
                           for (int k = 0; k < 4; k++) ((void **)(info + 0x40))[k] = r.range(0, 4) ? info + 0x80 + 0x20 * k : nullptr;
                           for (int k = 0; k < 4; k++) *(int32_t *)(info + 0x80 + 0x20 * k + 0x10) = 0x10 * r.range(0, 8);
                           ((void **)(info + 0x40))[-1] = info + 0x60, *(int32_t *)(info + 0x60 + 0x10) = 0x20;   // (number -1 reads the slot before the table)
                           self[0x6c] = (uint8_t)(r.range(0, 3) == 0);
                           if (self[0x6c]) *(void **)(self + 0x68) = r.range(0, 1) ? object + 0x40 : nullptr;
                           arena<uint32_t>(0x40)[0] = r.range(0, 1) ? 0 : r.range(0, 2) ? 1 : r.next();
                           arena<uint32_t>(0x40)[1] = r.next();
                       },
                       [](void *fn) {
                           const uint32_t back = ((uint32_t(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0], arena<uint32_t>(0x40)[1]);
                           return Result{back ^ *arena<uint32_t>(0x20)};
                       }};
    scenery_steps.iterations = 1000;
    scenery_steps.prepare = [](const rewrite::Entry *e) {
        g_ss_saved = *(uint32_t *)(uintptr_t)0x130b5c8;
        stubs_for_now({{e->at == 0x848840 ? 0x82f9c0u : e->at == 0x8489b0 ? 0x82fa30u : 0x82fab0u, (void *)ss_step}});
    };
    scenery_steps.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)(uintptr_t)0x130b5c8 = g_ss_saved;
    };
    quick(scenery_steps);
#endif
