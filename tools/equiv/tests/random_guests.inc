// Guest::MakeRandom (forever/src/rewrite/random_guests.cpp). Game globals set in the call and put back after: the
// terrain manager none or at arena 0x3000 (its map at 0x3100), the path manager none or at 0x3200, the game none or at
// 0x3300, the copy's counter. Memory: blocks over 0x100 bytes at 0x6000 (the guest), the rest from dev_take. The
// guest's constructor links its five references into lists of their own (so they can be taken out) and gives them a
// table whose slot 0 notes them; Handle::For gives 0x600 (with a list). 0x7C2FF0 notes its two byte values (the game's
// callee reads only those) and gives five places or none; 0x71EF40 notes its five values whole and gives a spot or
// none; Random::Unit and the random byte give dev_ numbers; sprintf is a stand-in; texts are dn_ / ds_ texts.
#if EQUIV_PART == 1
static uint32_t rg_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0; }
static uint32_t g_rg_saved[5], g_rg_counter;
static void *__cdecl rg_alloc(uint32_t size) { dev_note(640, size); return size > 0x100 ? arena<void>(0x6000) : dev_take(size); }
static uint32_t __fastcall rg_ready(void *object) { dev_note(641, rg_at(object)); return dev_next(); }
static void *__thiscall rg_terrain(uint8_t *self, uint32_t a, uint32_t b, uint32_t c) {
    dev_note(642, rg_at(self)); dev_note(642, a + b + c);
    self[0x2c] = (uint8_t)(dev_next() % 2);
    *(void **)(self + 0x54) = arena<void>(0x3100);
    *(void **)(uintptr_t)0x1311650 = self;
    return self;
}
static void __thiscall rg_terrain_size(void *self, uint32_t w, uint32_t h) { dev_note(643, rg_at(self)); dev_note(643, w); dev_note(643, h); }
static void *__thiscall rg_paths(uint8_t *self, uint32_t a, uint32_t b, uint32_t c) {
    dev_note(644, rg_at(self)); dev_note(644, a + b + c);
    self[0x2c] = (uint8_t)(dev_next() % 2);
    return self;
}
static uint32_t __fastcall rg_places(uint32_t kind, uint32_t zero, void **places) {
    dev_note(645, kind & 0xff); dev_note(645, zero & 0xff);
    for (int i = 0; i < 5; i++) places[i] = arena<uint8_t>(0x3400) + 0x10 * i;
    return arena<uint32_t>(0x50)[0];
}
static void *__thiscall rg_spot(void *paths, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    dev_note(646, rg_at(paths)); dev_note(646, a); dev_note(646, b); dev_note(646, c); dev_note(646, d); dev_note(646, e);
    return arena<uint32_t>(0x50)[1] ? arena<void>(0x3500) : nullptr;
}
static void __fastcall rg_ref_told(uint8_t *ref) { dev_note(647, rg_at(ref)); }
static void *rg_ref_table[1] = {(void *)rg_ref_told};
static void *__thiscall rg_guest(uint8_t *self, uint32_t a, uint32_t b, uint32_t c) {
    dev_note(648, rg_at(self)); dev_note(648, a + b + c);
    for (int i = 0; i < 5; i++) {
        uint32_t *ref = (uint32_t *)(self + 0x124 + 0x14 * i), *node = arena<uint32_t>(0x5000 + 0x20 * i);
        ref[0] = (uint32_t)(uintptr_t)rg_ref_table;
        ref[1] = (uint32_t)(uintptr_t)arena<void>(0x5200 + 0x10 * i);   // (its handle)
        ref[2] = (uint32_t)(uintptr_t)(node + 4);                       // next
        ref[3] = (uint32_t)(uintptr_t)node;                             // head
        node[0] = (uint32_t)(uintptr_t)&ref[2];
        node[5] = (uint32_t)(uintptr_t)&ref[2];
    }
    self[0x2c] = (uint8_t)(dev_next() % 2);
    *(uint32_t *)(self + 0x104) = dev_next() & 0x43ffffff;
    *(int16_t *)(self + 0x1a0) = 0;
    return self;
}
static void __thiscall rg_release(void *handle, uint32_t zero) { dev_note(649, rg_at(handle)); dev_note(649, zero); }
static void *__fastcall rg_handle_of(void *object) { dev_note(650, rg_at(object)); return arena<void>(0x600); }
static void *__cdecl rg_game() { dev_note(651, 1); return arena<void>(0x3300); }
static int __cdecl rg_sprintf(char *out, const char *format, uint32_t n) {
    dev_note(652, (uint32_t)(uintptr_t)format); dev_note(652, n);
    return snprintf(out, 0x18, " %u", n);
}
static void __thiscall rg_set_name(void *guest, DnText *text) {   // (takes the text by value: lets it go)
    dev_note(653, rg_at(guest)); dev_note(653, dn_hash(text));
    --text->count;
}
static void *__thiscall rg_spot_place(void *spot, uint32_t *out) {
    dev_note(654, rg_at(spot));
    for (int k = 0; k < 3; k++) out[k] = dev_next() & 0x447fffff;
    return out;
}
static void __thiscall rg_places_list(uint32_t *list, uint32_t n) {
    dev_note(655, rg_at(list)); dev_note(655, n);
    list[0] = (uint32_t)(uintptr_t)arena<void>(0x5800);
}
static long double __cdecl rg_unit() { return (long double)(dev_next() % 100000) / 100000.0L; }
static uint32_t __cdecl rg_byte() { return dev_next(); }
#else
    Test random_guests{0x79eb70,
                       [=](Rng &r) {
                           fresh(r);
                           arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                           arena<uint32_t>(0x50)[0] = r.range(0, 5) ? r.next() | 1 : r.next() & 0xffffff00u;   // places found or not
                           arena<uint32_t>(0x50)[1] = r.range(0, 3) != 0;                                       // a spot
                           arena<uint32_t>(0x50)[2] = r.range(0, 3);                                            // terrain: none, marked or not
                           arena<uint32_t>(0x50)[3] = r.range(0, 3);                                            // paths: none, ...
                           arena<uint32_t>(0x50)[4] = r.range(0, 2) != 0;                                       // the game there
                           arena<uint32_t>(0x50)[5] = r.next() % 1000;                                          // the counter
                           const int32_t wishes[] = {0, 1, -1, 2, (int32_t)r.next()};
                           arena<int32_t>(0x70)[0] = wishes[r.range(0, 4)];
                           arena<uint8_t>(0x3000)[0x2c] = (uint8_t)(arena<uint32_t>(0x50)[2] == 1);
                           *(void **)(arena<uint8_t>(0x3000) + 0x54) = arena<void>(0x3100);
                           arena<uint8_t>(0x3100)[4] = (uint8_t)r.range(0, 255), arena<uint8_t>(0x3100)[5] = (uint8_t)r.range(0, 255);
                           arena<uint8_t>(0x3200)[0x2c] = (uint8_t)(arena<uint32_t>(0x50)[3] == 1);
                           arena<uint32_t>(0x3300)[0x48 / 4] = r.next();
                           arena<uint32_t>(0x600)[0x20 / 4] = (uint32_t)(uintptr_t)arena<void>(0x680);
                           arena<uint32_t>(0x680)[0] = (uint32_t)(uintptr_t)arena<void>(0x690);
                           arena<uint32_t>(0x690)[1] = (uint32_t)(uintptr_t)arena<void>(0x680);
                           for (int k = 0; k < 4; k++) arena<uint32_t>(0x5800)[k] = r.next() & 0x447fffff;
                       },
                       [](void *fn) {
                           const uint32_t *s = arena<uint32_t>(0x50);
                           *(void **)(uintptr_t)0x1311650 = s[2] ? arena<void>(0x3000) : nullptr;
                           *(void **)(uintptr_t)0x130625c = s[3] ? arena<void>(0x3200) : nullptr;
                           *(void **)(uintptr_t)0x1106c54 = s[4] ? arena<void>(0x3300) : nullptr;
                           *(uint32_t *)(uintptr_t)g_rg_counter = s[5];
                           void *guest = ((void *(__fastcall *)(int32_t))fn)(arena<int32_t>(0x70)[0]);
                           arena<uint32_t>(0x78)[0] = *(uint32_t *)(uintptr_t)g_rg_counter;
                           arena<uint32_t>(0x78)[1] = rg_at(*(void **)(uintptr_t)0x1311650) ^ rg_at(*(void **)(uintptr_t)0x130625c) * 3;
                           return Result{rg_at(guest) ^ *arena<uint32_t>(0x20)};
                       }};
    random_guests.iterations = 1000;
    random_guests.prepare = [](const rewrite::Entry *e) {
        const bool first = e->at == 0x79eb70;
        g_rg_counter = first ? 0x1306c14 : 0x130a870;
        const uint32_t globals[5] = {0x1311650, 0x130625c, 0x1106c54, g_rg_counter, 0};
        for (int k = 0; k < 4; k++) g_rg_saved[k] = *(uint32_t *)(uintptr_t)globals[k];
        stubs_for_now({{0xbcdfc0, (void *)rg_alloc}, {0x800860, (void *)rg_ready}, {0x8d63a0, (void *)rg_terrain}, {0x8d41b0, (void *)rg_terrain_size},
                       {0x7232c0, (void *)rg_paths}, {0x7c2ff0, (void *)rg_places}, {0x71ef40, (void *)rg_spot}, {first ? 0x79e5b0u : 0x7e91e0u, (void *)rg_guest},
                       {0x8055c0, (void *)rg_release}, {0x805700, (void *)rg_handle_of}, {0x444090, (void *)rg_game}, {0xddcdb5, (void *)rg_sprintf},
                       {0xbcf850, (void *)dn_text_c}, {0x403bb0, (void *)ds_join}, {0x7cd6e0, (void *)rg_set_name}, {0xbce010, (void *)cheat_free_aligned},
                       {0xbce000, (void *)cheat_free}, {0x726930, (void *)rg_spot_place}, {0x7e5110, (void *)rg_places_list}, {0x475180, (void *)rg_unit},
                       {0x4751e0, (void *)rg_byte}});
    };
    random_guests.finish = [](const rewrite::Entry *) {
        unstub_many();
        const uint32_t globals[4] = {0x1311650, 0x130625c, 0x1106c54, g_rg_counter};
        for (int k = 0; k < 4; k++) *(uint32_t *)(uintptr_t)globals[k] = g_rg_saved[k];
    };
    quick(random_guests);
#endif
