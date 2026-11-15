// Designer::Destructor (forever/src/rewrite/window_dtors.cpp). The window at arena 0x100; its parts (+0x104, +0x114,
// +0x118, +0x11C) at 0x300.. or none; the stand-ins note what the window's tables are when they are called.
#if EQUIV_PART == 1
static void wd_tables(uint32_t tag) {
    const uint32_t *w = arena<uint32_t>(0x100);
    dev_note(tag, w[0]); dev_note(tag, w[1]); dev_note(tag, w[0xdc / 4]); dev_note(tag, w[0xe4 / 4]);
}
static void __thiscall wd_finish(void *part, uint32_t a, uint32_t b) { dev_note(270, (uint32_t)(uintptr_t)part); dev_note(270, a); dev_note(270, b); wd_tables(270); }
static void __thiscall wd_delete(void *part, uint32_t flag) { dev_note(271, (uint32_t)(uintptr_t)part); dev_note(271, flag); wd_tables(271); }
static void *wd_part_table[0x100 / 4 + 1];
static void __fastcall wd_reference_gone(void *r) { dev_note(272, (uint32_t)(uintptr_t)r); wd_tables(272); }
static void __fastcall wd_part_gone(void *p) { dev_note(273, (uint32_t)(uintptr_t)p); wd_tables(273); }
static void __fastcall wd_window_gone(void *w) { dev_note(274, (uint32_t)(uintptr_t)w); wd_tables(274); }
#else
    Test window_dtors{0xace700,
                      [=](Rng &r) {
                          fresh(r);
                          arena<uint32_t>(0x20)[0] = 0;
                          wd_part_table[0] = (void *)wd_delete, wd_part_table[0x100 / 4] = (void *)wd_finish;
                          uint8_t *w = arena<uint8_t>(0x100);
                          for (uint32_t at : {0x104u, 0x114u, 0x118u, 0x11cu}) {
                              uint8_t *part = arena<uint8_t>(0x300 + at);
                              *(void **)part = wd_part_table;
                              *(void **)(w + at) = r.range(0, 3) ? part : nullptr;
                          }
                      },
                      [](void *fn) {
                          ((void(__fastcall *)(void *))fn)(arena<void>(0x100));
                          return Result{*arena<uint32_t>(0x20)};
                      }};
    window_dtors.iterations = 300;
    window_dtors.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0x40f730, (void *)wd_reference_gone}, {0x6835a0, (void *)wd_part_gone}, {0x6b0ea0, (void *)wd_window_gone}});
    };
    window_dtors.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(window_dtors);
#endif
