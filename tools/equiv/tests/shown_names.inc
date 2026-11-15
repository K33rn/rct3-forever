// Window::ShownName (forever/src/rewrite/shown_names.cpp). The window at arena 0x100 with a counted text in each name
// field; the reference holds a text or not (bit 0 of its +4), with characters or none; memory sometimes none.
#if EQUIV_PART == 1
static void *__cdecl sn_alloc(uint32_t size) { dev_note(290, size); return dev_next() % 4 == 0 ? nullptr : dev_take(size); }
static uint32_t *sn_text() {
    uint32_t *t = (uint32_t *)dev_take(0x10);
    t[0] = 1 + (dev_next() & 1), t[3] = 0xf2000000u | (uint32_t)(uintptr_t)t;
    return t;
}
static void *__thiscall sn_copy(uint32_t **out, uint32_t **from) { dev_note(291, (uint32_t)(uintptr_t)*from); *out = sn_text(); return out; }
static void *__thiscall sn_reference(uint32_t *r, uint32_t **text, uint32_t table, uint32_t flag) {
    dev_note(292, (uint32_t)(uintptr_t)*text); dev_note(292, table); dev_note(292, flag);
    r[1] = dev_next(), r[2] = dev_next() % 3 ? (uint32_t)(uintptr_t)arena<void>(0x7000) : 0;
    return r;
}
static uint8_t __thiscall sn_holds(uint32_t *r, uint32_t a) { dev_note(293, a); return (uint8_t)(r[1] & 1); }
static void *__thiscall sn_from_wide(uint32_t *self, const uint16_t *wide) { dev_note(294, (uint32_t)(uintptr_t)wide); self[0] = 1; return self; }
static void __fastcall sn_reference_gone(uint32_t *r) { dev_note(295, r[1]); }
#else
    Test shown_names{0x6c5e50,
                     [=](Rng &r) {
                         fresh(r);
                         arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                         for (int i = 0; i < 0x400; i++) arena<uint32_t>(0x2000)[i] = r.next();
                         for (uint32_t at : {0x80u, 0x84u, 0x88u}) *(uint32_t **)(arena<uint8_t>(0x100) + at) = sn_text();
                     },
                     [](void *fn) {
                         void *back = ((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x40));
                         return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ *arena<uint32_t>(0x20)};
                     }};
    shown_names.iterations = 500;
    shown_names.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)sn_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbd01c0, (void *)sn_copy},
                       {0x410c90, (void *)sn_reference}, {0xbd1420, (void *)sn_holds}, {0xbcf7d0, (void *)sn_from_wide}, {0xbd0490, (void *)desk_empty},
                       {0xbd1880, (void *)sn_reference_gone}});
    };
    shown_names.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(shown_names);
#endif
