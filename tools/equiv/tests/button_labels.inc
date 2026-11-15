// Buttons::LitLabel (forever/src/rewrite/button_labels.cpp). The window at arena 0x100 with its two buttons at 0x300
// and 0x400 (bit 5 of their +0x94 lit or not); the label lookups have characters or none; the place for the text at
// 0x40, which the lookup's destruction notes (the text must be kept there first). Memory sometimes none.
#if EQUIV_PART == 1
static uint32_t bl_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0; }
static void *__cdecl bl_alloc(uint32_t size) { dev_note(520, size); return dev_next() % 5 ? dev_take(size) : nullptr; }
static void *__thiscall bl_named(uint32_t *r, uint32_t name, uint32_t table, uint32_t flag) {
    dev_note(521, name); dev_note(521, table); dev_note(521, flag);
    r[0] = name ^ 0x77, r[2] = dev_next() % 3 ? (uint32_t)(uintptr_t)arena<void>(0x7900) : 0;
    return r;
}
static void *__thiscall bl_from_wide(uint32_t *self, const uint16_t *wide) { dev_note(522, bl_at(wide)); self[0] = 1; return self; }
static void *__thiscall bl_text_c(uint32_t *self, const char *c) { dev_note(523, (uint32_t)(uintptr_t)c); self[0] = 1; return self; }
static void __fastcall bl_named_gone(uint32_t *r) { dev_note(524, r[0]); dev_note(524, bl_at(*arena<void *>(0x40))); }
#else
    Test button_labels{0x65d9b0,
                       [=](Rng &r) {
                           fresh(r);
                           arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                           for (uint32_t at : {0x104u, 0x108u, 0x114u, 0x118u, 0x144u, 0x148u}) {
                               uint8_t *button = arena<uint8_t>(at == 0x104 || at == 0x114 || at == 0x144 ? 0x300 : 0x400);
                               *(void **)(arena<uint8_t>(0x100) + at) = button;
                           }
                           *(uint32_t *)(arena<uint8_t>(0x300) + 0x94) = r.next() & ~(r.range(0, 1) ? 0x20u : 0u);
                           *(uint32_t *)(arena<uint8_t>(0x400) + 0x94) = r.next() & ~(r.range(0, 1) ? 0x20u : 0u);
                           arena<uint32_t>(0x40)[0] = r.next();
                       },
                       [](void *fn) {
                           void *back = ((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x40));
                           return Result{bl_at(back) ^ *arena<uint32_t>(0x20)};
                       }};
    button_labels.iterations = 500;
    button_labels.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)bl_alloc}, {0x4040f0, (void *)bl_named}, {0xbcf7d0, (void *)bl_from_wide}, {0xbd0490, (void *)desk_empty},
                       {0xbd1880, (void *)bl_named_gone}, {0xbcf850, (void *)bl_text_c}});
    };
    button_labels.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(button_labels);
#endif
