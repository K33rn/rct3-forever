// Animal::Value (forever/src/rewrite/animal_values.cpp). The animal at arena 0x100 (its record at 0x300, items at
// 0x380; its interface kept already or looked up through 0x500..0x5FF); the find (0x591E60) gives one of four found
// records at 0x600.. (kind 0-7, values with NaN and infinity) and sometimes shortens the list, so its count must be
// read again each time.
#if EQUIV_PART == 1
static uint32_t g_av_saved;
static void *__thiscall av_info(uint8_t *object) { dev_note(350, (uint32_t)(uintptr_t)object); return *(uint8_t **)(object + 4); }
static void *av_table[2] = {nullptr, (void *)av_info};
static const void *__thiscall av_find(void *to, uint32_t item) {
    dev_note(351, (uint32_t)(uintptr_t)to); dev_note(351, item);
    if (dev_next() % 9 == 0 && *(int32_t *)(arena<uint8_t>(0x300) + 0x5c) > 0) --*(int32_t *)(arena<uint8_t>(0x300) + 0x5c);
    return arena<uint8_t>(0x600) + (item & 3) * 0x80;
}
static float av_value(Rng &r) {
    switch (r.range(0, 9)) {
    case 0: return NAN;
    case 1: return INFINITY;
    case 2: return r.any_float();
    default: return r.real(-100, 100);
    }
}
#else
    Test animal_values{0xa32200,
                       [=](Rng &r) {
                           fresh(r);
                           arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next();
                           uint8_t *self = arena<uint8_t>(0x100), *rec = arena<uint8_t>(0x300);
                           *(void **)(self + 0x9c) = rec;
                           for (uint32_t off = 0x34; off <= 0x3c; off += 4) *(float *)(rec + off) = av_value(r);
                           if (r.range(0, 9) == 0) *(uint32_t *)(rec + 0x34 + 4 * r.range(0, 2)) = 0x7fa00001;   // (a signalling NaN to start from)
                           *(int32_t *)(rec + 0x5c) = r.range(0, 4) ? r.range(0, 7) : r.range(30, 40);   // (past 32: the bit number wraps)
                           *(void **)(rec + 0x60) = arena<void>(0x380);
                           for (int k = 0; k < 40; k++) arena<uint32_t>(0x380)[k] = r.next();
                           *(uint32_t *)(self + 0x60) = r.next() & r.next();
                           // the interface: kept already, or looked up through a reference (0x500) to an object (0x540) and its info (0x580)
                           *(int32_t *)(uintptr_t)0x12f5494 = 0;
                           uint8_t *ref = arena<uint8_t>(0x500), *object = arena<uint8_t>(0x540), *info = arena<uint8_t>(0x580);
                           *(void **)(self + 0x84) = ref, *(void **)(ref + 0xc) = object, *(void **)object = av_table, *(void **)(object + 4) = info;
                           *(int32_t *)(info + 0x18) = 1, *(void **)(info + 0x14) = info + 0x40, *(void **)(info + 0x40) = info + 0x60, *(int32_t *)(info + 0x70) = 8;
                           self[0x98] = (uint8_t)r.range(0, 1);
                           if (self[0x98]) *(void **)(self + 0x94) = object + 8;
                           for (int k = 0; k < 4; k++) {
                               uint8_t *found = arena<uint8_t>(0x600 + 0x80 * k);
                               *(uint32_t *)(found + 0x54) = r.range(0, 7);
                               *(void **)(found + 0x58) = found + 0x60;
                               for (uint32_t off = 0x10; off <= 0x18; off += 4) *(float *)(found + 0x60 + off) = av_value(r);
                           }
                       },
                       [](void *fn) {
                           Result x;
                           x.f = ((float(__fastcall *)(void *))fn)(arena<void>(0x100));
                           x.i = *arena<uint32_t>(0x20);
                           return x;
                       }};
    animal_values.iterations = 2000;
    animal_values.prepare = [](const rewrite::Entry *) {
        g_av_saved = *(uint32_t *)(uintptr_t)0x12f5494;
        stubs_for_now({{0x591e60, (void *)av_find}});
    };
    animal_values.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)(uintptr_t)0x12f5494 = g_av_saved;
    };
    quick(animal_values);
#endif
