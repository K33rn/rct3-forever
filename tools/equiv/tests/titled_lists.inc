// Window::BuildWithReference (forever/src/rewrite/titled_lists.cpp). Titled as the ident bars' test (its stand-ins);
// the window at arena 0x8000; the reference given by value names the handle at 0x600 (its count at +8, its list at
// 0x680); the base and ObjectRef's destructor are stand-ins (the destructor notes the reference's fields).
#if EQUIV_PART == 1
static const uint32_t tl_rows[][4] = {
#define TITLED_LISTS(at, description, t0, t4, ...) {at, description, t0, t4},
#include "../../../forever/src/rewrite/titled_lists.inc"
#undef TITLED_LISTS
};
static uint32_t g_tl_row, g_tl_saved;
static uint32_t tl_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0; }
struct TlReference {
    uint32_t w[5];
};
static void *__thiscall tl_base(uint32_t *self, void *description, uint32_t **title, uint32_t y) {
    dev_note(510, tl_at(self)); dev_note(510, tl_at(description)); dev_note(510, tl_at(*title)); dev_note(510, y); dev_note(510, self[0]);
    return self;
}
static void __fastcall tl_reference_gone(uint32_t *r) { dev_note(511, r[0]); dev_note(511, tl_at((void *)(uintptr_t)r[1])); dev_note(511, r[2]); dev_note(511, r[3]); }
#else
    Test titled_lists{0x4d60a0,
                      [=](Rng &r) {
                          fresh(r);
                          for (int i = 0; i < 0x60; i++) arena<uint32_t>(0x8000)[i] = r.next();
                          for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x7000)[i] = r.next();
                          for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                          arena<uint32_t>(0x7000)[2] = r.range(0, 2) ? 0xf2738c : 0;
                          arena<uint32_t>(0x7100)[0] = (uint32_t)(uintptr_t)arena<void>(0x7200);
                          arena<uint32_t>(0x7200)[0] = r.range(1, 3);
                          arena<uint32_t>(0x7180)[0] = (uint32_t)(uintptr_t)arena<void>(0x7280);
                          arena<uint32_t>(0x7280)[0] = r.range(1, 3);
                          uint32_t *handle = arena<uint32_t>(0x600);
                          handle[2] = r.range(0, 5);
                          handle[0x20 / 4] = (uint32_t)(uintptr_t)arena<void>(0x680);
                          arena<uint32_t>(0x680)[0] = (uint32_t)(uintptr_t)arena<void>(0x690);
                          arena<uint32_t>(0x690)[1] = (uint32_t)(uintptr_t)arena<void>(0x680);
                          arena<uint32_t>(0x40)[0] = r.range(0, 1) ? (uint32_t)(uintptr_t)arena<void>(0x7300) : 0;
                          arena<uint32_t>(0x40)[1] = r.range(0, 2) ? (uint32_t)(uintptr_t)arena<void>(0x7100) : 0;
                          arena<uint32_t>(0x40)[2] = r.next(), arena<uint32_t>(0x40)[3] = r.next();
                          arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                          arena<uint32_t>(0x30)[0] = 0;
                      },
                      [](void *fn) {
                          *(uint32_t *)(uintptr_t)tl_rows[g_tl_row][1] = (uint32_t)(uintptr_t)arena<void>(0x7000);
                          const uint32_t *a = arena<uint32_t>(0x40);
                          TlReference given = {{0xf26ae0, (uint32_t)(uintptr_t)arena<void>(0x600), 0x11112222, 0x33334444, 0}};
                          void *back = ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, TlReference, uint32_t))fn)(arena<void>(0x8000), a[0], a[1], a[2], given, a[3]);
                          return Result{tl_at(back) ^ *arena<uint32_t>(0x20)};
                      }};
    titled_lists.iterations = 300;
    titled_lists.prepare = [](const rewrite::Entry *e) {
        for (uint32_t k = 0; k < 2; k++)
            if (tl_rows[k][0] == e->at) g_tl_row = k;
        g_tl_saved = *(uint32_t *)(uintptr_t)tl_rows[g_tl_row][1];
        stubs_for_now({{0xbcdfc0, (void *)desk_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)cheat_text},
                       {0xbd0490, (void *)desk_empty}, {0x403be0, (void *)cheat_join}, {0x403bb0, (void *)desk_join_wide}, {0x41c740, (void *)desk_copy},
                       {0x6aae60, (void *)tl_base}, {0x40f730, (void *)tl_reference_gone}});
    };
    titled_lists.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)(uintptr_t)tl_rows[g_tl_row][1] = g_tl_saved;
    };
    quick(titled_lists);
#endif
