// AttractionPerson::FartherPoint (forever/src/rewrite/farther_point.cpp). The person at arena 0x100; its reference
// at 0x300 to an object at 0x400 whose info (0x500) gives the interface at 0x400 + off (its member +0x94 with a
// reference at 0x700 to an object at 0x800, info 0x900, giving the placed interface with its matrix). Both lookups
// find something (when one finds nothing the original crashes too); each is already kept or not yet.
#if EQUIV_PART == 1
static uint32_t g_fp_saved[2];
static void *__thiscall fp_info(uint8_t *object) {   // slot 1: the object's class info, noted
    dev_note(160, (uint32_t)(uintptr_t)object);
    return *(uint8_t **)(object + 4);
}
static void *fp_table[2] = {nullptr, (void *)fp_info};
static float fp_value(Rng &r) {
    switch (r.range(0, 15)) {
    case 0: return NAN;
    case 1: return 1e30f;
    case 2: return r.any_float();
    default: return r.real(-50, 50);
    }
}
static void fp_object(uint32_t at, uint32_t info, uint32_t number, int32_t off) {   // an object whose entry for number gives off
    uint8_t *o = arena<uint8_t>(at), *i = arena<uint8_t>(info);
    *(void **)o = fp_table;
    *(void **)(o + 4) = i;
    *(int32_t *)(i + 0x18) = number + 1 + (int32_t)(at & 3);
    *(void **)(i + 0x14) = i + 0x40;
    ((uint8_t **)(i + 0x40))[number] = i + 0x80;
    *(int32_t *)(i + 0x80 + 0x10) = off;
}
#else
    Test farther_point{0x4e84c0,
                       [=](Rng &r) {
                           fresh(r);
                           const uint32_t n1 = r.range(0, 3), n2 = r.range(0, 3);
                           *(uint32_t *)(uintptr_t)0x12f5608 = n1, *(uint32_t *)(uintptr_t)0x130b5c8 = n2;
                           uint8_t *self = arena<uint8_t>(0x100);
                           *(void **)(arena<uint8_t>(0x300) + 0xc) = arena<void>(0x400);
                           *(void **)(self + 0x104) = arena<void>(0x300);
                           const int32_t off1 = 0x40 * r.range(0, 4), off2 = 0x10 * r.range(0, 8);
                           fp_object(0x400, 0x500, n1, 0x200 + off1);   // the interface: 0x600 + off1 (room to 0x6FF + 0x94 + 0x1C)
                           uint8_t *person = arena<uint8_t>(0x600 + off1);
                           fp_object(0x800, 0x900, n2, 0x400 + off2);   // placed: 0xC00 + off2, matrix at +0xF0
                           *(void **)(arena<uint8_t>(0x700) + 0xc) = arena<void>(0x800);
                           *(void **)(person + 0x94 + 4) = arena<void>(0x700);
                           if (r.range(0, 2) == 0) self[0x118] = 1, *(void **)(self + 0x114) = person;   // already kept
                           else self[0x118] = 0;
                           if (r.range(0, 2) == 0) person[0x94 + 0x18] = 1, *(void **)(person + 0x94 + 0x14) = arena<void>(0xc00 + off2);
                           else person[0x94 + 0x18] = 0;
                           float *m = (float *)(arena<uint8_t>(0xc00 + off2) + 0xf0);
                           const bool mixed = r.range(0, 1);   // (mixed scales: the sums lose bits, so their order shows in near ties)
                           for (int k = 0; k < 16; k++) m[k] = r.range(0, 3) ? r.real(-2, 2) * (mixed ? ldexpf(1.0f, r.range(-20, 20)) : 1.0f) : fp_value(r);
                           for (int k = 0; k < 3; k++) ((float *)(self + 0xa0))[k] = fp_value(r);
                           float *p = arena<float>(0x40), *q = arena<float>(0x50);
                           for (int k = 0; k < 3; k++) p[k] = fp_value(r), q[k] = r.range(0, 5) ? fp_value(r) : p[k];   // sometimes the same point
                           arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0;
                       },
                       [](void *fn) {
                           uint32_t back = ((uint32_t(__thiscall *)(void *, const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40), arena<void>(0x50));
                           return Result{back ^ *arena<uint32_t>(0x20) << 1};
                       }};
    farther_point.iterations = 3000;
    farther_point.prepare = [](const rewrite::Entry *) { g_fp_saved[0] = *(uint32_t *)(uintptr_t)0x12f5608, g_fp_saved[1] = *(uint32_t *)(uintptr_t)0x130b5c8; };
    farther_point.finish = [](const rewrite::Entry *) { *(uint32_t *)(uintptr_t)0x12f5608 = g_fp_saved[0], *(uint32_t *)(uintptr_t)0x130b5c8 = g_fp_saved[1]; };
    quick(farther_point);
#endif
