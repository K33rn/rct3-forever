// Animal::Text (forever/src/rewrite/animal_texts.cpp). The part at arena 0x100: its owner (+0xF8) at 0x400 holds an
// object (0x500) or none at +0xC; the object's class info (slot 1) is 0x600 or none, with 1 to 3 entries (0x640),
// the class number's entry (0x680) or none, its offset 0 or 0x20 (the interface at 0x500 or 0x520, each with a table
// whose slot 0x20 fills the text it is handed, noted, and gives that place or another holding a second text). The kept
// interface and its mark (+0x108, +0x10C) random or none; texts are dn_ texts; memory always given.
#if EQUIV_PART == 1
static uint32_t g_at_saved;
static uint32_t at_off(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0; }
static void *__thiscall at_info(uint8_t *object) { dev_note(730, at_off(object)); return arena<uint32_t>(0x50)[1] ? arena<void>(0x600) : nullptr; }
static void *__thiscall at_fill(uint8_t *face, DnText **place) {
    dev_note(731, at_off(face));
    const uint16_t wide[4] = {'F', 'i', 'l', 'l'};
    *place = dn_make((DnText *)dev_take(0x10), wide, (uint32_t)(arena<uint32_t>(0x50)[2] % 5));
    (*place)->count = 1 + arena<uint32_t>(0x50)[2] % 2;
    if (arena<uint32_t>(0x50)[3]) return (void *)place;
    DnText **other = arena<DnText *>(0x7f0);
    *other = dn_make((DnText *)dev_take(0x10), wide, 2);
    (*other)->count = 2;
    return (void *)other;
}
static void *at_table[9] = {nullptr, (void *)at_info, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, (void *)at_fill};   // (slot 1: the class info; 0x20: fill)
static uint32_t g_at_class_saved;
static void *__cdecl at_alloc(uint32_t size) { dev_note(732, size); return dev_take(0x10); }
static void *__fastcall at_text(DnText *self, void *, const char *c) {
    dev_note(733, (uint32_t)(uintptr_t)c);
    const uint16_t none[1] = {0};
    return dn_make(self, none, 0);
}
#else
    Test animal_text{0xad7110,
                     [=](Rng &r) {
                         fresh(r);
                         arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                         for (int k = 0; k < 0x48; k++) arena<uint32_t>(0x100)[k] = r.next();
                         *arena<void *>(0x100 + 0xf8) = arena<void>(0x400);
                         *arena<void *>(0x400 + 0xc) = r.range(0, 3) ? arena<void>(0x500) : nullptr;
                         *arena<void *>(0x500) = at_table;
                         *arena<void *>(0x520) = at_table;
                         arena<int32_t>(0x600 + 0x18)[0] = r.range(1, 3);
                         *arena<void *>(0x600 + 0x14) = arena<void>(0x640);
                         for (int k = 0; k < 3; k++) arena<uint32_t>(0x640)[k] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x680) : 0;
                         arena<uint32_t>(0x680 + 0x10)[0] = r.range(0, 1) ? 0 : 0x20;
                         arena<int32_t>(0x50)[0] = r.range(0, 3) - 1;   // the class number (-1 at times: the count compared signed)
                         arena<uint32_t>(0x63c)[0] = r.range(0, 1) ? (uint32_t)(uintptr_t)arena<void>(0x680) : 0;   // (the entry before the first)
                         arena<uint32_t>(0x50)[1] = r.range(0, 4) != 0;   // class info there
                         arena<uint32_t>(0x50)[2] = r.next();
                         arena<uint32_t>(0x50)[3] = r.range(0, 2) != 0;   // the place handed back
                         // the kept interface and its mark: looked up already (an interface or none) or not
                         arena<uint8_t>(0x100 + 0x10c)[0] = (uint8_t)r.range(0, 1);
                         *arena<void *>(0x100 + 0x108) = r.range(0, 2) ? arena<void>(r.range(0, 1) ? 0x500 : 0x520) : nullptr;
                     },
                     [](void *fn) {
                         *(int32_t *)(uintptr_t)g_at_saved = arena<int32_t>(0x50)[0];   // (the class number: the case's)
                         void *back = ((void *(__fastcall *)(void *, void *, void *))fn)(arena<void>(0x100), nullptr, arena<void>(0x60));
                         return Result{at_off(back) ^ *arena<uint32_t>(0x20)};
                     }};
    animal_text.iterations = 2000;
    animal_text.prepare = [](const rewrite::Entry *e) {
        const uintptr_t global = e->at == 0xad7110 ? 0x13ce508 : 0x13ce628;
        g_at_saved = (uint32_t)global;
        g_at_class_saved = *(uint32_t *)global;
        stubs_for_now({{0xbcdfc0, (void *)at_alloc}, {0xbcf850, (void *)at_text}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}});
    };
    animal_text.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)(uintptr_t)g_at_saved = g_at_class_saved;
    };
    quick(animal_text);
#endif
