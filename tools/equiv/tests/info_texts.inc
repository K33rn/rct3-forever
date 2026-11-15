// Info::NameText (forever/src/rewrite/info_texts.cpp). The info at arena 0x100: its text a dn_ text held 1-3 times;
// its object (+0xA0 and +0x9C) at 0x700 / 0x740 with a wide name (at 0x780 / 0x7c0) or none. The replacer's stand-in
// notes the three texts and puts in the text's place a new text (letting the old go by one) when others hold it, else
// changes it in place; texts from wide and C texts are dn_ texts; the empty text ds_empty's.
#if EQUIV_PART == 1
static void *__thiscall int_from_wide(DnText *self, const uint16_t *wide) {
    uint32_t n = 0;
    while (n < 40 && wide[n]) n++;
    dev_note(620, n);
    return dn_make(self, wide, n);
}
static void __fastcall int_replace(DnText **text, DnText **me, DnText **name) {
    dev_note(621, dn_hash(*text)); dev_note(621, dn_hash(*me)); dev_note(621, dn_hash(*name)); dev_note(621, (uint32_t)(*text)->count);
    DnText *t = *text;
    if (t->count > 1) {
        --t->count;
        *text = dn_make((DnText *)dev_take(0x10), (*name)->characters, (*name)->length);
    } else if (t->length) {
        t->characters[0] = (uint16_t)(t->characters[0] + 1);
    }
}
static void *__cdecl int_alloc(uint32_t size) { dev_note(622, size); return dev_take(size); }
#else
    Test info_texts{0xa290d0,
                    [=](Rng &r) {
                        fresh(r);
                        arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                        uint16_t wide[8];
                        const uint32_t n = r.range(0, 7);
                        for (uint32_t k = 0; k < n; k++) wide[k] = (uint16_t)('a' + r.range(0, 25));
                        DnText *text = dn_make((DnText *)dev_take(0x10), wide, n);
                        text->count = r.range(1, 3);
                        *(DnText **)(arena<uint8_t>(0x100) + 0x64) = text;
                        *(void **)(arena<uint8_t>(0x100) + 0xa0) = arena<void>(0x700);
                        *(void **)(arena<uint8_t>(0x100) + 0x9c) = arena<void>(0x740);
                        for (uint32_t o : {0x700u, 0x740u}) {
                            uint16_t *name = (uint16_t *)arena<uint8_t>(o + 0x80);
                            const uint32_t m = r.range(0, 6);
                            for (uint32_t k = 0; k < m; k++) name[k] = (uint16_t)('A' + r.range(0, 25));
                            name[m] = 0;
                            *(void **)(arena<uint8_t>(o) + 0xc) = r.range(0, 3) ? (void *)name : nullptr;
                        }
                    },
                    [](void *fn) {
                        void *back = ((void *(__fastcall *)(void *, void *, void *))fn)(arena<void>(0x100), nullptr, arena<void>(0x80));
                        return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ *arena<uint32_t>(0x20)};
                    }};
    info_texts.iterations = 1000;
    info_texts.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)int_alloc}, {0xbcf7d0, (void *)int_from_wide}, {0xbcf850, (void *)dn_text_c}, {0xbd0490, (void *)ds_empty},
                       {0xcd23a0, (void *)int_replace}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}});
    };
    info_texts.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(info_texts);
#endif
