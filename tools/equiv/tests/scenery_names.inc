// Scenery::KindNameText (forever/src/rewrite/scenery_names.cpp). The item at arena 0x100 with its text (+0x84 and
// +0x144) a dn_ text held 1-3 times; the game text by name: the tester's stub_named_text_ref (its wide text at 0x300
// or none, as arena 0x3C says); its release noted with the frees; the rest as info_texts.inc's stand-ins.
#if EQUIV_PART == 1
static void __fastcall scnm_ref_free(void *self) { dev_note(625, self == g_text_ref); }   // (in the same sum as the frees: the order shows)
#else
    Test scenery_names{0x8b1330,
                       [=](Rng &r) {
                           fresh(r);
                           arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                           arena<uint32_t>(0x3c)[0] = r.next();
                           arena<uint32_t>(CALL_LOG)[0] = 0;
                           uint16_t *wide = arena<uint16_t>(0x300);
                           const uint32_t m = r.range(0, 6);
                           for (uint32_t k = 0; k < m; k++) wide[k] = (uint16_t)('A' + r.range(0, 25));
                           wide[m] = 0;
                           for (uint32_t off : {0x84u, 0x144u}) {
                               uint16_t chars[8];
                               const uint32_t n = r.range(0, 7);
                               for (uint32_t k = 0; k < n; k++) chars[k] = (uint16_t)('a' + r.range(0, 25));
                               DnText *text = dn_make((DnText *)dev_take(0x10), chars, n);
                               text->count = r.range(1, 3);
                               *(DnText **)(arena<uint8_t>(0x100) + off) = text;
                           }
                       },
                       [](void *fn) {
                           void *back = ((void *(__fastcall *)(void *, void *, void *))fn)(arena<void>(0x100), nullptr, arena<void>(0x80));
                           return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ *arena<uint32_t>(0x20)};
                       }};
    scenery_names.iterations = 1000;
    scenery_names.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)int_alloc}, {0xbcf7d0, (void *)int_from_wide}, {0xbcf850, (void *)dn_text_c}, {0xbd0490, (void *)ds_empty},
                       {0xcd23a0, (void *)int_replace}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free},
                       {0x4040f0, (void *)stub_named_text_ref}, {0xbd1880, (void *)scnm_ref_free}});
    };
    scenery_names.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(scenery_names);
#endif
