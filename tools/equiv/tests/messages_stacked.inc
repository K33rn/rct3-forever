// Window::MessageNumberedStacked (forever/src/rewrite/messages.cpp's message, for the copy taking its values on the
// stack): as Window::MessageNumbered's test in equiv.cpp (its stand-ins), with this copy's six globals.
#if EQUIV_PART == 1
static const uint32_t ms_globals[6] = {0x12fb7b0, 0x12fe290, 0x12fb218, 0x12ff314, 0x1303444, 0x1304768};
#else
    Test stacked_message{0x48d240,
                         [=](Rng &r) {
                             fresh(r);
                             for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                             arena<uint32_t>(0x8000)[0] = r.next();
                             arena<uint32_t>(0x40)[0] = r.range(0, 3) ? r.range(0, 5) : r.next();
                             for (int k = 0; k < 6; k++) arena<uint32_t>(0x50)[k] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x7400 + 0x20 * k) : 0;
                             arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                         },
                         [](void *fn) {
                             for (int k = 0; k < 6; k++) *(uint32_t *)(uintptr_t)ms_globals[k] = arena<uint32_t>(0x50)[k];
                             void *back = ((void *(__stdcall *)(void *, uint32_t))fn)(arena<void>(0x8000), arena<uint32_t>(0x40)[0]);
                             return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ *arena<uint32_t>(0x20)};
                         }};
    stacked_message.iterations = 300;
    stacked_message.prepare = [](const rewrite::Entry *) {
        for (int k = 0; k < 6; k++) g_msg_saved[k] = *(uint32_t *)(uintptr_t)ms_globals[k];
        stubs_for_now({{0xbcdfc0, (void *)msg_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbd0490, (void *)desk_empty},
                       {0xbcf7d0, (void *)msg_wide}, {0xbcf850, (void *)msg_text_c}, {0xddcdb5, (void *)msg_sprintf}, {0xcd23a0, (void *)msg_replace}});
    };
    stacked_message.finish = [](const rewrite::Entry *) {
        unstub_many();
        for (int k = 0; k < 6; k++) *(uint32_t *)(uintptr_t)ms_globals[k] = g_msg_saved[k];
    };
    quick(stacked_message);
#endif
