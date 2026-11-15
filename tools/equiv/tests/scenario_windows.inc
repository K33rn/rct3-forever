// Window::BuildCentred (forever/src/rewrite/scenario_windows.cpp). Titled as the ident bars' test (its stand-ins); the
// window at arena 0x8000 and the screen window at 0x7800 with random rectangles (NaN and infinity among them); the
// base, the part's constructor, its placing and Window::UpdateRect are stand-ins noting what they are given. The
// copy's default-description global and the screen global are set in the call and put back after.
#if EQUIV_PART == 1
static const uint32_t sw_rows[][10] = {
#define SCENARIO_WINDOWS(at, description, value, size, t0, t4, td4, te8, tf0, make, ...) {at, description, value, size, t0, t4, td4, te8, tf0, make},
#include "../../../forever/src/rewrite/scenario_windows.inc"
#undef SCENARIO_WINDOWS
};
static uint32_t g_sw_row, g_sw_saved[2];
static uint32_t sw_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0; }
static void *__thiscall sw_base(uint32_t *self, void *description, uint32_t **title, uint32_t value) {
    dev_note(470, sw_at(self)); dev_note(470, sw_at(description)); dev_note(470, sw_at(*title)); dev_note(470, value);
    return self;
}
static void *__thiscall sw_part(void *memory, uint32_t a, uint32_t b) { dev_note(471, sw_at(memory)); dev_note(471, a); dev_note(471, b); return memory; }
static void __thiscall sw_place(void *part, uint32_t relation, void *to) { dev_note(472, sw_at(part)); dev_note(472, relation); dev_note(472, sw_at(to)); }
static void __thiscall sw_rect(void *w, const uint32_t *rect) { dev_note(473, sw_at(w)); for (int k = 0; k < 4; k++) dev_note(473, rect[k]); }
static float sw_value(Rng &r) {
    switch (r.range(0, 11)) {
    case 0: return NAN;
    case 1: return INFINITY;
    case 2: return r.any_float();
    default: return r.real(-1200, 1200);
    }
}
#else
    Test scenario_windows{0x6cf110,
                          [=](Rng &r) {
                              fresh(r);
                              for (int i = 0; i < 0x80; i++) arena<uint32_t>(0x8000)[i] = r.next();
                              for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x7000)[i] = r.next();
                              for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                              for (uint32_t off = 0x7c; off <= 0x88; off += 4) *(float *)(arena<uint8_t>(0x8000) + off) = sw_value(r), *(float *)(arena<uint8_t>(0x7800) + off) = sw_value(r);
                              arena<uint8_t>(0x7000)[4] = (uint8_t)r.range(0, 1);
                              arena<uint32_t>(0x7000)[2] = r.range(0, 2) ? 0xf2738c : 0;
                              arena<uint32_t>(0x7100)[0] = (uint32_t)(uintptr_t)arena<void>(0x7200);
                              arena<uint32_t>(0x7200)[0] = r.range(1, 3);
                              arena<uint32_t>(0x7180)[0] = (uint32_t)(uintptr_t)arena<void>(0x7280);
                              arena<uint32_t>(0x7280)[0] = r.range(1, 3);
                              arena<uint32_t>(0x40)[0] = r.range(0, 1) ? (uint32_t)(uintptr_t)arena<void>(0x7300) : 0;
                              arena<uint32_t>(0x40)[1] = r.range(0, 2) ? (uint32_t)(uintptr_t)arena<void>(0x7100) : 0;
                              arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                              arena<uint32_t>(0x30)[0] = 0;
                          },
                          [](void *fn) {
                              *(uint32_t *)(uintptr_t)sw_rows[g_sw_row][1] = (uint32_t)(uintptr_t)arena<void>(0x7000);
                              *(uint32_t *)(uintptr_t)0x12f83f0 = (uint32_t)(uintptr_t)arena<void>(0x7800);
                              const uint32_t *a = arena<uint32_t>(0x40);
                              void *back = ((void *(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x8000), a[0], a[1]);
                              return Result{sw_at(back) ^ *arena<uint32_t>(0x20)};
                          }};
    scenario_windows.iterations = 1000;
    scenario_windows.prepare = [](const rewrite::Entry *e) {
        for (uint32_t k = 0; k < 2; k++)
            if (sw_rows[k][0] == e->at) g_sw_row = k;
        g_sw_saved[0] = *(uint32_t *)(uintptr_t)sw_rows[g_sw_row][1], g_sw_saved[1] = *(uint32_t *)(uintptr_t)0x12f83f0;
        stubs_for_now({{0xbcdfc0, (void *)desk_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)cheat_text},
                       {0xbd0490, (void *)desk_empty}, {0x403be0, (void *)cheat_join}, {0x403bb0, (void *)desk_join_wide}, {0x41c740, (void *)desk_copy},
                       {0x63a3a0, (void *)sw_base}, {sw_rows[g_sw_row][9], (void *)sw_part}, {0x6b1330, (void *)sw_place}, {0x6afaf0, (void *)sw_rect}});
    };
    scenario_windows.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)(uintptr_t)sw_rows[g_sw_row][1] = g_sw_saved[0], *(uint32_t *)(uintptr_t)0x12f83f0 = g_sw_saved[1];
    };
    quick(scenario_windows);
#endif
