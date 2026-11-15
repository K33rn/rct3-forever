// Folder::LoadOnce (forever/src/rewrite/load_once.cpp). The copy's flag set or not; the two folders and the joins are
// the desktop test's text stand-ins (counted texts; a join sometimes gives the holder at 0x7180); the window's
// constructor gives the window at 0x7a00 (random rectangle); the screen window at 0x7800; the global object and the
// game value at 0x7500/0x7600 and 0x7700. The game globals are set in the call and put back after.
#if EQUIV_PART == 1
static const uint32_t lo_rows[][4] = {
#define LOAD_ONCE(at, flag, folder, make, ...) {at, flag, folder, make},
#include "../../../forever/src/rewrite/load_once.inc"
#undef LOAD_ONCE
};
static uint32_t g_lo_row, g_lo_saved[5];
static const uint32_t lo_globals[4] = {0x12f83f0, 0x1342708, 0x12f4618, 0};
static uint32_t lo_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0; }
static void *__cdecl lo_alloc(uint32_t size) { dev_note(540, size); return dev_take(size); }
static void *__fastcall lo_folder_user(uint32_t **out) { dev_note(541, 1); *out = desk_text(); return out; }
static void *__fastcall lo_folder_game(uint32_t **out) { dev_note(541, 2); *out = desk_text(); return out; }
static void *__thiscall lo_window(void *memory, uint32_t a, uint32_t b, uint32_t **text) { dev_note(542, lo_at(memory)); dev_note(542, a); dev_note(542, b); dev_note(542, lo_at(*text)); return arena<void>(0x7a00); }
static void __thiscall lo_layout(void *w, uint32_t relation, void *to, const uint32_t *rect) {
    dev_note(543, lo_at(w)); dev_note(543, relation); dev_note(543, lo_at(to));
    for (int k = 0; k < 4; k++) dev_note(543, rect[k]);
}
static float lo_value(Rng &r) {
    switch (r.range(0, 11)) {
    case 0: return NAN;
    case 1: return -INFINITY;
    case 2: return r.any_float();
    default: return r.real(-1500, 1500);
    }
}
#else
    Test load_once{0x99f890,
                   [=](Rng &r) {
                       fresh(r);
                       arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                       for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                       arena<uint32_t>(0x7180)[0] = (uint32_t)(uintptr_t)arena<void>(0x7280);
                       arena<uint32_t>(0x7280)[0] = r.range(1, 3);
                       for (uint32_t off = 0x7c; off <= 0x88; off += 4) *(float *)(arena<uint8_t>(0x7800) + off) = lo_value(r), *(float *)(arena<uint8_t>(0x7a00) + off) = lo_value(r);
                       arena<uint8_t>(0x7600)[0xa4] = (uint8_t)r.range(0, 1);
                       arena<int32_t>(0x7700)[1] = r.range(0, 1) ? r.range(990, 1010) : (int32_t)r.next();
                       arena<uint32_t>(0x50)[0] = r.range(0, 4) ? 0 : r.next() & 0xff;   // the flag
                       arena<uint32_t>(0x50)[1] = r.range(0, 2);                         // the global object: none, empty, with its own
                   },
                   [](void *fn) {
                       *(uint8_t *)(uintptr_t)lo_rows[g_lo_row][1] = (uint8_t)arena<uint32_t>(0x50)[0];
                       *(void **)(uintptr_t)0x12f83f0 = arena<void>(0x7800);
                       const uint32_t g = arena<uint32_t>(0x50)[1];
                       *(void **)(uintptr_t)0x1342708 = g ? arena<void>(0x7500) : nullptr;
                       *arena<void *>(0x7500) = g == 2 ? arena<void>(0x7600) : nullptr;
                       *(void **)(uintptr_t)0x12f4618 = arena<void>(0x7700);
                       ((void(__cdecl *)())fn)();
                       return Result{*arena<uint32_t>(0x20)};
                   }};
    load_once.iterations = 1000;
    load_once.prepare = [](const rewrite::Entry *e) {
        for (uint32_t k = 0; k < 2; k++)
            if (lo_rows[k][0] == e->at) g_lo_row = k;
        for (int k = 0; k < 3; k++) g_lo_saved[k] = *(uint32_t *)(uintptr_t)lo_globals[k];
        g_lo_saved[3] = *(uint8_t *)(uintptr_t)lo_rows[g_lo_row][1];
        stubs_for_now({{0xbcdfc0, (void *)lo_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0x458150, (void *)lo_folder_user},
                       {0x458130, (void *)lo_folder_game}, {0x403be0, (void *)cheat_join}, {0x403bb0, (void *)desk_join_wide},
                       {lo_rows[g_lo_row][3], (void *)lo_window}, {0x6b1640, (void *)lo_layout}});
    };
    load_once.finish = [](const rewrite::Entry *) {
        unstub_many();
        for (int k = 0; k < 3; k++) *(uint32_t *)(uintptr_t)lo_globals[k] = g_lo_saved[k];
        *(uint8_t *)(uintptr_t)lo_rows[g_lo_row][1] = (uint8_t)g_lo_saved[3];
    };
    quick(load_once);
#endif
