// Designer::BuildIdentBar (forever/src/rewrite/ident_bars.cpp). The example of a test file of its own: see
// tools/equiv/tests/README.md.
#if EQUIV_PART == 1
// Designer::BuildIdentBar: its stand-ins (the titled window test's, the class test's name steps and handle), the base
// bar and the bar text
static uint32_t g_ib_global, g_ib_saved;
static void *__thiscall ib_base(uint32_t *self, void *description, uint32_t **title) {
    dev_note(156, (uint32_t)(uintptr_t)self); dev_note(156, (uint32_t)(uintptr_t)description); dev_note(156, (uint32_t)(uintptr_t)*title); dev_note(156, self[0]);
    return self;
}
static void __thiscall ib_bar_text(uint32_t *self, uint32_t *reference, uint32_t a) { dev_note(157, (uint32_t)(uintptr_t)self); dev_note(157, reference[1]); dev_note(157, a); dev_note(157, self[0x110 / 4]); }
#else
    // Designer::BuildIdentBar (two copies): the bar at arena 0x8000; its default description (the copy's global) at
    // 0x7000 (named or not yet: +4; its own name none or a C text), a description or none (0x7300), a title or none
    Test ident_bar{0xad7460,
                   [=](Rng &r) {
                       fresh(r);
                       for (int i = 0; i < 0x50; i++) arena<uint32_t>(0x8000)[i] = r.next();
                       for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x7000)[i] = r.next();
                       for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                       arena<uint8_t>(0x7000)[4] = (uint8_t)r.range(0, 1);
                       arena<uint8_t>(0x7300)[4] = (uint8_t)r.range(0, 1);
                       arena<uint32_t>(0x7000)[2] = r.range(0, 2) ? 0xf2738c : 0;
                       arena<uint32_t>(0x7100)[0] = (uint32_t)(uintptr_t)arena<void>(0x7200);
                       arena<uint32_t>(0x7200)[0] = r.range(1, 3);
                       arena<uint32_t>(0x7180)[0] = (uint32_t)(uintptr_t)arena<void>(0x7280);
                       arena<uint32_t>(0x7280)[0] = r.range(1, 3);
                       uint32_t *handle = arena<uint32_t>(0x600);
                       handle[0x20 / 4] = (uint32_t)(uintptr_t)arena<void>(0x680);
                       arena<uint32_t>(0x680)[0] = (uint32_t)(uintptr_t)arena<void>(0x690);
                       arena<uint32_t>(0x690)[1] = (uint32_t)(uintptr_t)arena<void>(0x680);
                       arena<uint32_t>(0x40)[0] = r.range(0, 1) ? (uint32_t)(uintptr_t)arena<void>(0x7300) : 0;
                       arena<uint32_t>(0x40)[1] = r.range(0, 2) ? (uint32_t)(uintptr_t)arena<void>(0x7100) : 0;
                       arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                       arena<uint32_t>(0x30)[0] = 0;
                       arena<uint32_t>(CALL_LOG)[0] = 0;
                   },
                   [](void *fn) {
                       *(uint32_t *)(uintptr_t)g_ib_global = (uint32_t)(uintptr_t)arena<void>(0x7000);
                       const uint32_t *a = arena<uint32_t>(0x40);
                       void *back = ((void *(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x8000), a[0], a[1]);
                       return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ *arena<uint32_t>(0x20)};
                   }};
    ident_bar.iterations = 300;
    ident_bar.prepare = [](const rewrite::Entry *e) {
        g_ib_global = e->at == 0xad7460 ? 0x13ce4f0 : 0x13ce5ac;
        g_ib_saved = *(uint32_t *)(uintptr_t)g_ib_global;
        stubs_for_now({{0xbcdfc0, (void *)desk_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)cheat_text},
                       {0xbd0490, (void *)desk_empty}, {0x403be0, (void *)cheat_join}, {0x403bb0, (void *)desk_join_wide}, {0x41c740, (void *)desk_copy},
                       {0x9b2340, (void *)ib_base}, {0x805700, (void *)stub_member_handle_of}, {0xbeca40, (void *)stub_name_make},
                       {0xbecc10, (void *)stub_name_give}, {0xbec5e0, (void *)stub_name_free}, {0x41cb90, (void *)tw_reference},
                       {0x6b0230, (void *)ib_bar_text}, {0xbd1880, (void *)tw_reference_gone}});
    };
    ident_bar.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)(uintptr_t)g_ib_global = g_ib_saved;
    };
    quick(ident_bar);
#endif
