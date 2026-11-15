// Window::BuildList (forever/src/rewrite/list_windows.cpp). Titled as the ident bars' test (its stand-ins); the
// window at arena 0x8000 (filled with junk, so every field set shows); the base, the part's build and the list's
// hand-over are stand-ins noting what they are given (the list's links as offsets into the window).
#if EQUIV_PART == 1
static uint32_t g_lw_saved, g_lw_values[4];
static const uint32_t lw_globals[4] = {0x10ec9e8, 0x12fbd28, 0x1302e08, 0x12fa178};   // (read into the window: given values of their own)
static uint32_t lw_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0; }
static void *__thiscall lw_base(uint32_t *self, void *description, uint32_t **title, uint32_t zero, uint32_t c) {
    dev_note(460, lw_at(self)); dev_note(460, lw_at(description)); dev_note(460, lw_at(*title)); dev_note(460, zero); dev_note(460, c);
    return self;
}
static void __thiscall lw_part(void *part, uint32_t g) { dev_note(461, lw_at(part)); dev_note(461, g); }
static void __thiscall lw_list(uint8_t *window, uint32_t *list) {
    dev_note(462, lw_at(window)); dev_note(462, lw_at(list));
    for (int k = 0; k < 8; k++) dev_note(462, k == 1 || k == 5 || k == 7 ? lw_at((void *)(uintptr_t)list[k]) : list[k]);
}
#else
    Test list_windows{0x5996a0,
                      [=](Rng &r) {
                          fresh(r);
                          for (int i = 0; i < 0x80; i++) arena<uint32_t>(0x8000)[i] = r.next();
                          for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x7000)[i] = r.next();
                          for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                          arena<uint8_t>(0x7000)[4] = (uint8_t)r.range(0, 1);
                          arena<uint8_t>(0x7300)[4] = (uint8_t)r.range(0, 1);
                          arena<uint32_t>(0x7000)[2] = r.range(0, 2) ? 0xf2738c : 0;
                          arena<uint32_t>(0x7100)[0] = (uint32_t)(uintptr_t)arena<void>(0x7200);
                          arena<uint32_t>(0x7200)[0] = r.range(1, 3);
                          arena<uint32_t>(0x7180)[0] = (uint32_t)(uintptr_t)arena<void>(0x7280);
                          arena<uint32_t>(0x7280)[0] = r.range(1, 3);
                          arena<uint32_t>(0x40)[0] = r.range(0, 1) ? (uint32_t)(uintptr_t)arena<void>(0x7300) : 0;
                          arena<uint32_t>(0x40)[1] = r.range(0, 2) ? (uint32_t)(uintptr_t)arena<void>(0x7100) : 0;
                          for (int k = 2; k < 5; k++) arena<uint32_t>(0x40)[k] = r.next();
                          for (int k = 0; k < 4; k++) arena<uint32_t>(0x60)[k] = r.next();
                          arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                          arena<uint32_t>(0x30)[0] = 0;
                      },
                      [](void *fn) {
                          *(uint32_t *)(uintptr_t)0x12f54bc = (uint32_t)(uintptr_t)arena<void>(0x7000);
                          for (int k = 0; k < 4; k++) *(uint32_t *)(uintptr_t)lw_globals[k] = arena<uint32_t>(0x60)[k];
                          const uint32_t *a = arena<uint32_t>(0x40);
                          void *back = ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x8000), a[0], a[1], a[2], a[3], a[4]);
                          return Result{lw_at(back) ^ *arena<uint32_t>(0x20)};
                      }};
    list_windows.iterations = 300;
    list_windows.prepare = [](const rewrite::Entry *) {
        g_lw_saved = *(uint32_t *)(uintptr_t)0x12f54bc;
        for (int k = 0; k < 4; k++) g_lw_values[k] = *(uint32_t *)(uintptr_t)lw_globals[k];
        stubs_for_now({{0xbcdfc0, (void *)desk_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)cheat_text},
                       {0xbd0490, (void *)desk_empty}, {0x403be0, (void *)cheat_join}, {0x403bb0, (void *)desk_join_wide}, {0x41c740, (void *)desk_copy},
                       {0x6a2620, (void *)lw_base}, {0xcd2b30, (void *)lw_part}, {0xbeca40, (void *)stub_name_make}, {0xbecc10, (void *)stub_name_give},
                       {0xbec5e0, (void *)stub_name_free}, {0x6a22a0, (void *)lw_list}});
    };
    list_windows.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)(uintptr_t)0x12f54bc = g_lw_saved;
        for (int k = 0; k < 4; k++) *(uint32_t *)(uintptr_t)lw_globals[k] = g_lw_values[k];
    };
    quick(list_windows);
#endif
