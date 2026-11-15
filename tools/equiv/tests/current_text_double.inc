// Condition::CurrentTextDouble (forever/src/rewrite/current_texts.cpp's current_text, for the copy writing its value as
// a double with "%.2f"): as Condition::CurrentText's test in equiv.cpp (its stand-ins), the value any float.
#if EQUIV_PART == 2
    Test current_text_double{0x6e5bf0,
                             [=](Rng &r) {
                                 fresh(r);
                                 for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                                 ct_table[0x68 / 4] = (void *)ct_value;
                                 arena<uint32_t>(0x8000)[0] = (uint32_t)(uintptr_t)ct_table;
                                 arena<uint32_t>(0x8100)[0] = r.next();
                                 arena<int32_t>(0x40)[0] = r.range(0, 3) ? 0 : r.range(1, 5);
                                 arena<float>(0x48)[0] = r.range(0, 4) ? r.real(-1000, 1000) : r.any_float();
                                 arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                             },
                             [](void *fn) {
                                 void *back = ((void *(__thiscall *)(void *, void *, int32_t))fn)(arena<void>(0x8000), arena<void>(0x8100), arena<int32_t>(0x40)[0]);
                                 return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ *arena<uint32_t>(0x20)};
                             }};
    current_text_double.iterations = 300;
    current_text_double.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)msg_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)msg_text_c},
                       {0x4040f0, (void *)stub_class_named_make}, {0x442120, (void *)ct_put_together}, {0xbd1880, (void *)mf_named_gone},
                       {0xddcdb5, (void *)msg_sprintf}, {0xcd23a0, (void *)msg_replace}, {0x6d6ca0, (void *)ct_timing}});
    };
    current_text_double.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(current_text_double);
#endif
