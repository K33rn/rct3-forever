// Designer::BuildPanelWindow (forever/src/rewrite/panel_windows.cpp). Titled as the ident bars' test (its stand-ins);
// the window at arena 0x8000; the base, the panel's constructor and its layout are stand-ins noting what they are
// given. The copy's default-description global is set in the call and put back after.
#if EQUIV_PART == 1
static const uint32_t pw_rows[][7] = {
#define PANEL_WINDOWS(at, description, size, t0, t4, make, side, ...) {at, description, size, t0, t4, make, side},
#include "../../../forever/src/rewrite/panel_windows.inc"
#undef PANEL_WINDOWS
};
static uint32_t g_pw_row, g_pw_saved;
static uint32_t pw_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0; }
static void *__thiscall pw_base(uint32_t *self, void *description, uint32_t **title, uint32_t one) {
    dev_note(480, pw_at(self)); dev_note(480, pw_at(description)); dev_note(480, pw_at(*title)); dev_note(480, one); dev_note(480, self[0]);
    return self;
}
static void *__thiscall pw_panel(void *memory, uint32_t a, uint32_t b, uint32_t value) { dev_note(481, pw_at(memory)); dev_note(481, a); dev_note(481, b); dev_note(481, value); return memory; }
static void __thiscall pw_layout(void *panel, uint32_t relation, void *to, const uint32_t *rect) {
    dev_note(482, pw_at(panel)); dev_note(482, relation); dev_note(482, pw_at(to));
    for (int k = 0; k < 4; k++) dev_note(482, rect[k]);
}
#else
    Test panel_windows{0xae6a10,
                       [=](Rng &r) {
                           fresh(r);
                           for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x8000)[i] = r.next();
                           for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x7000)[i] = r.next();
                           for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
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
                       },
                       [](void *fn) {
                           *(uint32_t *)(uintptr_t)pw_rows[g_pw_row][1] = (uint32_t)(uintptr_t)arena<void>(0x7000);
                           const uint32_t *a = arena<uint32_t>(0x40);
                           void *back = ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x8000), a[0], a[1], a[2]);
                           return Result{pw_at(back) ^ *arena<uint32_t>(0x20)};
                       }};
    panel_windows.iterations = 300;
    panel_windows.prepare = [](const rewrite::Entry *e) {
        for (uint32_t k = 0; k < 2; k++)
            if (pw_rows[k][0] == e->at) g_pw_row = k;
        g_pw_saved = *(uint32_t *)(uintptr_t)pw_rows[g_pw_row][1];
        stubs_for_now({{0xbcdfc0, (void *)desk_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)cheat_text},
                       {0xbd0490, (void *)desk_empty}, {0x403be0, (void *)cheat_join}, {0x403bb0, (void *)desk_join_wide}, {0x41c740, (void *)desk_copy},
                       {0x67dac0, (void *)pw_base}, {pw_rows[g_pw_row][5], (void *)pw_panel}, {0x6b1640, (void *)pw_layout}});
    };
    panel_windows.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)(uintptr_t)pw_rows[g_pw_row][1] = g_pw_saved;
    };
    quick(panel_windows);
#endif
