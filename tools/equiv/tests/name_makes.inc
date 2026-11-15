// Name::MakeObject (forever/src/rewrite/name_makes.cpp). The name (held at 0x60) a dn_ text; ToLower's stand-in makes a
// lower-case dn_ text; the names lookup's stand-in notes the names object and the text (and lets it go: it takes it by
// value) and finds 0x700 or nothing; 0x700 and the loaded object 0x780 give their interface (+0 or +0x10) or not; the
// load's stand-in puts a handle (0x7C0) holding 0x780 or nothing in the reference; memory sometimes none; the makers
// and the join note what they get. Game globals (the names, the class numbers) set in the call and put back after.
#if EQUIV_PART == 1
static uint32_t g_nm_saved[3];
static uint32_t nm_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0; }
static void *__thiscall nm_info(uint8_t *object) { dev_note(700, nm_at(object)); return *(uint8_t **)(object + 4); }
static void *nm_object_table[2] = {nullptr, (void *)nm_info};
static void *__fastcall nm_lower(DnText **out, DnText **from) {
    uint16_t wide[32];
    const DnText *f = *from;
    uint32_t n = 0;
    for (; n < f->length && n < 31; n++) wide[n] = (uint16_t)(f->characters[n] >= 'A' && f->characters[n] <= 'Z' ? f->characters[n] + 32 : f->characters[n]);
    dev_note(701, dn_hash(f));
    *out = dn_make((DnText *)dev_take(0x10), wide, n);
    return out;
}
static void *__thiscall nm_find(void *names, DnText *given) {   // (takes the text by value: lets it go)
    dev_note(702, nm_at(names)); dev_note(702, dn_hash(given));
    --given->count;
    return arena<uint32_t>(0x50)[0] ? arena<void>(0x700) : nullptr;
}
static void *__cdecl nm_alloc(uint32_t size) { dev_note(703, size); return dev_next() % 5 ? dev_take(size > 0x100 ? 0x100 : size) : nullptr; }
static void *__thiscall nm_first(void *memory, void *face) { dev_note(704, nm_at(face)); return dev_next() % 4 ? memory : nullptr; }
static void *__fastcall nm_load(uint32_t *reference, DnText **path) {
    dev_note(705, dn_hash(*path));
    reference[1] = (uint32_t)(uintptr_t)arena<void>(0x7c0);
    *(void **)(arena<uint8_t>(0x7c0) + 0xc) = arena<uint32_t>(0x50)[1] ? arena<void>(0x780) : nullptr;
    return reference;
}
static void __fastcall nm_reference_gone(uint32_t *reference) { dev_note(706, nm_at((void *)(uintptr_t)reference[1])); }
static void *__thiscall nm_second(void *memory, uint32_t a, uint32_t b, uint32_t c) { dev_note(707, a + b + c); return memory; }
static void __thiscall nm_join(void *second, void *made) { dev_note(708, second != nullptr); dev_note(708, nm_at(made)); }
static void nm_object(uint32_t o, uint32_t entry_off, bool has) {   // two class entries: number 0 and 1 give other places
    uint8_t *object = arena<uint8_t>(o), *info = arena<uint8_t>(o + 0x20);
    *(void **)object = nm_object_table, *(void **)(object + 4) = info;
    *(int32_t *)(info + 0x18) = 2, *(void **)(info + 0x14) = info + 0x1c;
    *(void **)(info + 0x1c) = has ? info + 0x24 : nullptr;
    *(void **)(info + 0x20) = info + 0x38;
    *(int32_t *)(info + 0x24 + 0x10) = (int32_t)entry_off;
    *(int32_t *)(info + 0x38 + 0x10) = (int32_t)(entry_off ^ 0x10);
}
#else
    Test name_makes{0xadff60,
                    [=](Rng &r) {
                        fresh(r);
                        arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                        const uint16_t wide[6] = {'N', 'a', 'M', 'e', 'X', 'y'};
                        DnText *name = dn_make((DnText *)dev_take(0x10), wide, (uint32_t)r.range(0, 6));
                        name->count = r.range(1, 2);
                        *arena<DnText *>(0x60) = name;
                        nm_object(0x700, r.range(0, 1) ? 0 : 0x10, r.range(0, 3) != 0);
                        nm_object(0x780, r.range(0, 1) ? 0 : 0x10, r.range(0, 3) != 0);
                        arena<uint32_t>(0x50)[0] = r.range(0, 3) != 0;   // the names find it
                        arena<uint32_t>(0x50)[1] = r.range(0, 3) != 0;   // the load holds it
                    },
                    [](void *fn) {
                        *(void **)(uintptr_t)0x1341408 = arena<void>(0x900);
                        *(int32_t *)(uintptr_t)0x13ce508 = 0, *(int32_t *)(uintptr_t)0x13ce628 = 1;   // (other numbers: a swap shows)
                        void *back = ((void *(__fastcall *)(void *))fn)(arena<void>(0x60));
                        return Result{nm_at(back) ^ *arena<uint32_t>(0x20)};
                    }};
    name_makes.iterations = 1000;
    name_makes.prepare = [](const rewrite::Entry *e) {
        const bool first = e->at == 0xadff60;
        g_nm_saved[0] = *(uint32_t *)(uintptr_t)0x1341408, g_nm_saved[1] = *(uint32_t *)(uintptr_t)0x13ce508, g_nm_saved[2] = *(uint32_t *)(uintptr_t)0x13ce628;
        stubs_for_now({{0xcd27d0, (void *)nm_lower}, {0x9acf50, (void *)nm_find}, {0xbcdfc0, (void *)nm_alloc}, {first ? 0xad95a0u : 0xaf6480u, (void *)nm_first},
                       {0x803d30, (void *)nm_load}, {0x40f730, (void *)nm_reference_gone}, {first ? 0xadf4e0u : 0xaf4be0u, (void *)nm_second},
                       {first ? 0xadfc50u : 0xaf5210u, (void *)nm_join}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}});
    };
    name_makes.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)(uintptr_t)0x1341408 = g_nm_saved[0], *(uint32_t *)(uintptr_t)0x13ce508 = g_nm_saved[1], *(uint32_t *)(uintptr_t)0x13ce628 = g_nm_saved[2];
    };
    quick(name_makes);
#endif
