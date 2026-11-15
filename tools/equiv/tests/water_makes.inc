// Water::MakeNamed (forever/src/rewrite/water_makes.cpp). The object at arena 0x8000 (random); its default description
// (the copy's global) at 0x7000, a description or none (0x7300), each named already (+4) or not; the member set-up's
// stand-in notes its place and what is in the object then; the name steps the class test's.
#if EQUIV_PART == 1
static uint32_t g_wm_global, g_wm_saved;
static void __fastcall wm_member(uint32_t *member) {
    record(0x86, (void *)(uintptr_t)((uintptr_t)member - (uintptr_t)arena<void>(0)));
    record(0x86, (void *)(uintptr_t)member[-1]);
    member[0] ^= 0x5a5a;
}
#else
    Test water_make{0x96be30,
                    [=](Rng &r) {
                        fresh(r);
                        for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x8000)[i] = r.next();
                        for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x7000)[i] = r.next();
                        arena<uint8_t>(0x7000)[4] = (uint8_t)r.range(0, 1);
                        arena<uint8_t>(0x7300)[4] = (uint8_t)r.range(0, 1);
                        arena<uint32_t>(0x40)[0] = r.range(0, 1) ? (uint32_t)(uintptr_t)arena<void>(0x7300) : 0;
                        arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                        arena<uint32_t>(0x30)[0] = 0;
                        arena<uint32_t>(CALL_LOG)[0] = 0;
                    },
                    [](void *fn) {
                        *(uint32_t *)(uintptr_t)g_wm_global = (uint32_t)(uintptr_t)arena<void>(0x7000);
                        void *back = ((void *(__fastcall *)(void *, void *, uint32_t))fn)(arena<void>(0x8000), nullptr, arena<uint32_t>(0x40)[0]);
                        return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ *arena<uint32_t>(0x20)};
                    }};
    water_make.iterations = 400;
    water_make.prepare = [](const rewrite::Entry *e) {
        g_wm_global = e->at == 0x96be30 ? 0x133d0e8 : e->at == 0x96c090 ? 0x133d0f0 : 0x133d0f8;
        g_wm_saved = *(uint32_t *)(uintptr_t)g_wm_global;
        stubs_for_now({{0x9655d0, (void *)wm_member}, {0xbeca40, (void *)stub_name_make}, {0xbecc10, (void *)stub_name_give}, {0xbec5e0, (void *)stub_name_free}});
    };
    water_make.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)(uintptr_t)g_wm_global = g_wm_saved;
    };
    quick(water_make);
#endif
