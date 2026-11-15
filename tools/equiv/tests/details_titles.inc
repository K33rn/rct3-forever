// DetailsWindow::Build (forever/src/rewrite/details_titles.cpp). Titled as the ident bars' test (its stand-ins); the
// object at arena 0x7700 (Handle::For gives the class test's handle at 0x600, its list at 0x680). The background's
// stand-in gets the reference by value, as the game's does: it notes it (its handle, list links as arena offsets,
// whether it is its own owner where it lies) and leaves the list again, as the real one's destructor would (no stack
// address is left behind). The copy's globals are set in the call and put back after.
#if EQUIV_PART == 1
static const uint32_t dtt_rows[][8] = {
#define DETAILS_TITLES(at, description, part, t0, t4, td8, global, value, ...) {at, description, part, t0, t4, td8, global, value},
#include "../../../forever/src/rewrite/details_titles.inc"
#undef DETAILS_TITLES
};
static uint32_t g_dtt_row, g_dtt_saved[2];
static uint32_t dtt_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0; }
struct DttReference {
    uint32_t w[5];
};
static void *__thiscall dtt_background(uint32_t *self, uint32_t description, uint32_t named, uint32_t value, uint32_t part, DttReference ref) {
    dev_note(490, dtt_at(self)); dev_note(490, dtt_at((void *)(uintptr_t)description)); dev_note(490, dtt_at(*(void **)(uintptr_t)named));
    dev_note(490, value); dev_note(490, dtt_at((void *)(uintptr_t)part));
    dev_note(490, ref.w[0]); dev_note(490, dtt_at((void *)(uintptr_t)ref.w[1])); dev_note(490, dtt_at((void *)(uintptr_t)ref.w[2]));
    dev_note(490, dtt_at((void *)(uintptr_t)ref.w[3])); dev_note(490, ref.w[4] == (uint32_t)(uintptr_t)&ref);
    uint32_t *next = (uint32_t *)(uintptr_t)ref.w[2], *head = (uint32_t *)(uintptr_t)ref.w[3];
    dev_note(490, head[0] == (uint32_t)(uintptr_t)&ref.w[2]);   // (joined first in the list)
    dev_note(490, next[1] == (uint32_t)(uintptr_t)&ref.w[2]);
    head[0] = (uint32_t)(uintptr_t)next, next[1] = (uint32_t)(uintptr_t)head;   // (left again)
    dev_note(490, (uint32_t)(*(uint32_t *)(uintptr_t)ref.w[3] == ref.w[2]));
    return self;
}
#else
    Test details_titles{0x634820,
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
                            uint32_t *handle = arena<uint32_t>(0x600);
                            handle[0x20 / 4] = (uint32_t)(uintptr_t)arena<void>(0x680);
                            arena<uint32_t>(0x680)[0] = (uint32_t)(uintptr_t)arena<void>(0x690);
                            arena<uint32_t>(0x690)[1] = (uint32_t)(uintptr_t)arena<void>(0x680);
                            arena<uint32_t>(0x40)[0] = r.range(0, 1) ? (uint32_t)(uintptr_t)arena<void>(0x7300) : 0;
                            arena<uint32_t>(0x40)[1] = r.range(0, 2) ? (uint32_t)(uintptr_t)arena<void>(0x7100) : 0;
                            arena<uint32_t>(0x40)[2] = r.next();
                            arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                            arena<uint32_t>(0x30)[0] = 0;
                        },
                        [](void *fn) {
                            *(uint32_t *)(uintptr_t)dtt_rows[g_dtt_row][1] = (uint32_t)(uintptr_t)arena<void>(0x7000);
                            *(uint32_t *)(uintptr_t)dtt_rows[g_dtt_row][6] = 0x5eed;
                            const uint32_t *a = arena<uint32_t>(0x40);
                            void *back = ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, void *))fn)(arena<void>(0x8000), a[0], a[1], a[2], arena<void>(0x7700));
                            return Result{dtt_at(back) ^ *arena<uint32_t>(0x20) ^ *(uint32_t *)(uintptr_t)dtt_rows[g_dtt_row][6] << 8};
                        }};
    details_titles.iterations = 300;
    details_titles.prepare = [](const rewrite::Entry *e) {
        for (uint32_t k = 0; k < 2; k++)
            if (dtt_rows[k][0] == e->at) g_dtt_row = k;
        g_dtt_saved[0] = *(uint32_t *)(uintptr_t)dtt_rows[g_dtt_row][1], g_dtt_saved[1] = *(uint32_t *)(uintptr_t)dtt_rows[g_dtt_row][6];
        stubs_for_now({{0xbcdfc0, (void *)desk_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)cheat_text},
                       {0xbd0490, (void *)desk_empty}, {0x403be0, (void *)cheat_join}, {0x403bb0, (void *)desk_join_wide}, {0x41c740, (void *)desk_copy},
                       {0x805700, (void *)stub_member_handle_of}, {0x669750, (void *)dtt_background}});
    };
    details_titles.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)(uintptr_t)dtt_rows[g_dtt_row][1] = g_dtt_saved[0], *(uint32_t *)(uintptr_t)dtt_rows[g_dtt_row][6] = g_dtt_saved[1];
    };
    quick(details_titles);
#endif
