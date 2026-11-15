// Database::MissingDependency (forever/src/rewrite/missing_deps.cpp). The entry at arena 0x100 with data or none (at
// +0x80 and +0x8C); sprintf a stand-in (the game's needs its C library set up) noting what it prints; the report notes
// the data and the text; memory never fails (the game lets the text go unchecked).
#if EQUIV_PART == 1
static void *__cdecl md_alloc(uint32_t size) { dev_note(340, size); return dev_take(size); }
static int __cdecl md_sprintf(char *to, const char *format) {
    uint32_t h = 0;
    for (const char *p = format; *p; p++) h = h * 31 + (uint8_t)*p;
    dev_note(341, h);
    return sprintf(to, "%s", format);
}
static void *__thiscall md_text(uint32_t *self, const char *from) {
    uint32_t h = 0;
    for (const char *p = from; *p; p++) h = h * 31 + (uint8_t)*p;
    dev_note(342, h);
    self[0] = 1 + (dev_next() & 1), self[3] = 0xf3000000u | (uint32_t)(uintptr_t)self;
    return self;
}
static void __fastcall md_report(void *data, uint32_t **text, uint32_t level) { dev_note(343, (uint32_t)(uintptr_t)data); dev_note(343, (*text)[0]); dev_note(343, level); }
#else
    Test missing_deps{0x7313c0,
                      [=](Rng &r) {
                          fresh(r);
                          arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                          for (uint32_t at : {0x80u, 0x8cu}) *(uint32_t *)(arena<uint8_t>(0x100) + at) = r.range(0, 2) ? (uint32_t)(uintptr_t)arena<void>(0x300 + at) : 0;
                      },
                      [](void *fn) {
                          ((void(__fastcall *)(void *))fn)(arena<void>(0x100));
                          return Result{*arena<uint32_t>(0x20)};
                      }};
    missing_deps.iterations = 300;
    missing_deps.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)md_alloc}, {0xddcdb5, (void *)md_sprintf}, {0xbcf850, (void *)md_text}, {0xc211c0, (void *)md_report},
                       {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}});
    };
    missing_deps.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(missing_deps);
#endif
