// Objectives::CountText (forever/src/rewrite/count_texts.cpp). The objective at arena 0x100 with a count at +0x70
// (negative and extreme ones too); the copy's text a real one (dn_), shared or not; making room (dn_room) moves a
// shared text, so the text must be read again after it. sprintf a stand-in (the game's needs its C library set up).
#if EQUIV_PART == 1
static void *__fastcall ct_base(DnText **out) {
    uint16_t w[12];
    const uint32_t n = dev_next() % 11;
    for (uint32_t k = 0; k < n; k++) w[k] = (uint16_t)(0x41 + (dev_next() % 0x300));
    *out = dn_make((DnText *)dev_take(0x10), w, n);
    (*out)->count = 1 + (int32_t)(dev_next() & 1);
    dev_note(330, n);
    return out;
}
static int __cdecl ct_sprintf(char *to, const char *format, int value) {
    dev_note(331, (uint32_t)(uintptr_t)format); dev_note(331, (uint32_t)value);
    return sprintf(to, format, value);
}
#else
    Test count_texts{0x6dd800,
                     [=](Rng &r) {
                         fresh(r);
                         arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                         for (int i = 0; i < 0x1000; i++) arena<uint32_t>(0x2000)[i] = r.next();   // (text memory holds junk: a missing end 0 shows)
                         const int32_t odd[] = {0, -1, INT32_MIN, INT32_MAX, 10, -100000};
                         *(int32_t *)(arena<uint8_t>(0x100) + 0x70) = r.range(0, 2) ? r.range(-500, 5000) : odd[r.range(0, 5)];
                     },
                     [](void *fn) {
                         void *back = ((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x40));
                         const DnText *t = *arena<DnText *>(0x40);
                         return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ *arena<uint32_t>(0x20) ^ dn_hash(t) ^ t->length << 20};
                     }};
    count_texts.iterations = 1000;
    count_texts.prepare = [](const rewrite::Entry *e) {
        const uint32_t base = e->at == 0x6dd800 ? 0x6dd5f0 : e->at == 0x6ddb30 ? 0x6dd920 : 0x6ddc50;
        stubs_for_now({{base, (void *)ct_base}, {0xddcdb5, (void *)ct_sprintf}, {0xbcffe0, (void *)dn_room}, {0xbce010, (void *)cheat_free_aligned},
                       {0xbce000, (void *)cheat_free}});
    };
    count_texts.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(count_texts);
#endif
