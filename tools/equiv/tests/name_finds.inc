// Name::FindSorted (forever/src/rewrite/name_finds.cpp). The object at arena 0x100: a table of 0-12 names in order
// (case ignored; one missing sometimes) at +0x30 (8 bytes an entry) and +0x34 (4 bytes), the count at +0x2C; the name
// given (held at 0x60) a dn_ text of one of them in mixed case or a name between them. The compare is a stand-in that
// notes both texts (case ignored, as the game's); texts are dn_ / ds_ texts; dn_room the room maker.
#if EQUIV_PART == 1
static const char *const nf_words[] = {"alpha", "Bravo", "charlie", "Delta", "echo", "foxtrot", "Golf", "hotel", "India", "juliet", "kilo", "Lima", "mike"};
static int nf_lower(int c) { return c >= 'A' && c <= 'Z' ? c + 32 : c; }
static int32_t __stdcall nf_compare(const char *a, const char *b) {
    uint32_t ha = 5381, hb = 5381;
    for (const char *p = a; *p; p++) ha = ha * 33 + (uint8_t)nf_lower(*p);
    for (const char *p = b; *p; p++) hb = hb * 33 + (uint8_t)nf_lower(*p);
    dev_note(690, ha); dev_note(690, hb);
    for (;; a++, b++) {
        const int x = nf_lower((uint8_t)*a), y = nf_lower((uint8_t)*b);
        if (x != y) return x - y;
        if (!x) return 0;
    }
}
static void *__cdecl nf_alloc(uint32_t size) { dev_note(691, size); return dev_take(size); }
#else
    Test name_finds{0xcc9290,
                    [=](Rng &r) {
                        fresh(r);
                        arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                        const int32_t count = r.range(0, 12);
                        *(int32_t *)(arena<uint8_t>(0x100) + 0x2c) = r.range(0, 9) ? count : -r.range(1, 3);
                        *(void **)(arena<uint8_t>(0x100) + 0x30) = arena<void>(0x400);
                        *(void **)(arena<uint8_t>(0x100) + 0x34) = arena<void>(0x500);
                        const int32_t missing = r.range(0, 3) ? -1 : r.range(0, 12);
                        for (int32_t i = 0; i < 13; i++) {
                            const char *word = i == missing ? nullptr : nf_words[i];
                            arena<const char *>(0x400)[2 * i] = word, arena<uint32_t>(0x400)[2 * i + 1] = r.next();
                            arena<const char *>(0x500)[i] = word;
                        }
                        char key[16];
                        const char *pick = nf_words[r.range(0, 12)];
                        if (r.range(0, 3) == 0) snprintf(key, sizeof key, "%sz", pick);   // (between two names)
                        else snprintf(key, sizeof key, "%s", pick);
                        uint16_t wide[16];
                        uint32_t n = 0;
                        for (; key[n]; n++) wide[n] = (uint16_t)(r.range(0, 1) ? key[n] : nf_lower(key[n]) == key[n] ? key[n] - 32 * (key[n] >= 'a' && key[n] <= 'z') : key[n]);
                        DnText *name = dn_make((DnText *)dev_take(0x10), wide, n);
                        name->count = r.range(1, 2);
                        *arena<DnText *>(0x60) = name;
                    },
                    [](void *fn) {
                        const int32_t back = ((int32_t(__fastcall *)(void *, void *, void *))fn)(arena<void>(0x100), nullptr, arena<void>(0x60));
                        return Result{(uint32_t)back ^ *arena<uint32_t>(0x20)};
                    }};
    name_finds.iterations = 1000;
    name_finds.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)nf_alloc}, {0xbcf850, (void *)dn_text_c}, {0xbd0490, (void *)ds_empty}, {0xbcffe0, (void *)dn_room},
                       {0xcd4880, (void *)nf_compare}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}});
    };
    name_finds.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(name_finds);
#endif
