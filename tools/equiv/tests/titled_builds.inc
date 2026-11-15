// Window::BuildTitled9A3 (forever/src/rewrite/titled_builds.cpp). As the ident bars' test (ident_bars.inc, whose
// stand-ins it uses): the window at arena 0x8000, the copy's default description at 0x7000, a description or none
// (0x7300), a title or none, a value; the base is a stand-in noting what it is given; the member's telling notes the
// member and the window's tables by then.
#if EQUIV_PART == 1
static const uint32_t tb_rows[][8] = {
#define TITLED_BUILDS(at, description, base, member, t0, t4, td4, tm, ...) {at, description, base, member, t0, t4, td4, tm},
#include "../../../forever/src/rewrite/titled_builds.inc"
#undef TITLED_BUILDS
};
static uint32_t g_tb_row, g_tb_saved;
static void __fastcall tb_member(uint32_t *m) {
    const uint32_t *w = arena<uint32_t>(0x8000);
    dev_note(430, m[0]); dev_note(430, m[1]); dev_note(430, m[2]); dev_note(430, m[3]);
    dev_note(430, w[0]); dev_note(430, w[0xd4 / 4]);
}
#else
    Test titled_builds{0x9a31d0,
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
                           arena<uint32_t>(0x40)[0] = r.range(0, 1) ? (uint32_t)(uintptr_t)arena<void>(0x7300) : 0;
                           arena<uint32_t>(0x40)[1] = r.range(0, 2) ? (uint32_t)(uintptr_t)arena<void>(0x7100) : 0;
                           arena<uint32_t>(0x40)[2] = r.next();
                           arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                           arena<uint32_t>(0x30)[0] = 0;
                           arena<uint32_t>(CALL_LOG)[0] = 0;
                       },
                       [](void *fn) {
                           *(uint32_t *)(uintptr_t)tb_rows[g_tb_row][1] = (uint32_t)(uintptr_t)arena<void>(0x7000);
                           const uint32_t *a = arena<uint32_t>(0x40);
                           void *back = ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x8000), a[0], a[1], a[2]);
                           return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ *arena<uint32_t>(0x20)};
                       }};
    titled_builds.iterations = 300;
    titled_builds.prepare = [](const rewrite::Entry *e) {
        for (uint32_t k = 0; k < 3; k++)
            if (tb_rows[k][0] == e->at) g_tb_row = k;
        g_tb_saved = *(uint32_t *)(uintptr_t)tb_rows[g_tb_row][1];
        stubs_for_now({{0xbcdfc0, (void *)desk_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)cheat_text},
                       {0xbd0490, (void *)desk_empty}, {0x403be0, (void *)cheat_join}, {0x403bb0, (void *)desk_join_wide}, {0x41c740, (void *)desk_copy},
                       {tb_rows[g_tb_row][2], (void *)ib_base}, {0x99ff30, (void *)tb_member}, {0xbeca40, (void *)stub_name_make},
                       {0xbecc10, (void *)stub_name_give}, {0xbec5e0, (void *)stub_name_free}});
    };
    titled_builds.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)(uintptr_t)tb_rows[g_tb_row][1] = g_tb_saved;
    };
    quick(titled_builds);
#endif
