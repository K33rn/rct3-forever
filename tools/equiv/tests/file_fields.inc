// Database::FileFields (forever/src/rewrite/file_fields.cpp). Memory from dev_take, sometimes none (each step handles
// none); every function called is a stand-in that notes what it is given and what the objects hold by then.
#if EQUIV_PART == 1
static void *__cdecl ff_alloc(uint32_t size) { dev_note(280, size); return dev_next() % 5 == 0 ? nullptr : dev_take(size); }
static void __fastcall ff_link(uint32_t *link) { dev_note(281, (uint32_t)(uintptr_t)link); link[0] = 0x11a7; }
static void __thiscall ff_field(uint32_t *field, uint32_t name, uint32_t kind, uint32_t a, uint32_t b) {
    dev_note(282, (uint32_t)(uintptr_t)field); dev_note(282, name); dev_note(282, kind); dev_note(282, a); dev_note(282, b);
    field[0] = 0xf1e1d;
}
static void __thiscall ff_give(uint32_t *link, uint32_t *field) {
    dev_note(283, (uint32_t)(uintptr_t)link); dev_note(283, (uint32_t)(uintptr_t)field);
    if (link) { dev_note(283, link[6]); dev_note(283, link[7]); }
    if (field) dev_note(283, field[0]);
}
#else
    Test file_fields{0x7fd800,
                     [=](Rng &r) {
                         fresh(r);
                         arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                         for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x2000)[i] = r.next();
                     },
                     [](void *fn) {
                         void *back = ((void *(__cdecl *)())fn)();
                         return Result{(uint32_t)(uintptr_t)back ^ *arena<uint32_t>(0x20)};
                     }};
    file_fields.iterations = 300;
    file_fields.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)ff_alloc}, {0x808700, (void *)ff_link}, {0x8088c0, (void *)ff_field}, {0x8087e0, (void *)ff_give}});
    };
    file_fields.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(file_fields);
#endif
