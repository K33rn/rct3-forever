// SpeciesCondition::Build (forever/src/rewrite/species_builds.cpp). Titled as the ident bars' test (whose stand-ins it
// uses); the condition at arena 0x7700 with a count up to 7; the box (the stand-in's) at the memory given, its list's
// texts (+0x104 + 0x1C8) at 0x7a00, each a counted text; the condition's text function sometimes gives the text that
// is in the list already. Memory never fails (the box's texts are let go unchecked, in the game too).
#if EQUIV_PART == 1
static const uint32_t sb_rows[][9] = {
#define SPECIES_BUILDS(at, description, t0, t4, tf8, t118, count, text, selected, ...) {at, description, t0, t4, tf8, t118, count, text, selected},
#include "../../../forever/src/rewrite/species_builds.inc"
#undef SPECIES_BUILDS
};
static uint32_t g_sb_row, g_sb_saved;
static uint32_t sb_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0; }
static void *__cdecl sb_alloc(uint32_t size) { dev_note(440, size); return dev_take(size); }
static void *__thiscall sb_base(uint32_t *self, void *description, uint32_t **title, void *condition) {
    dev_note(441, sb_at(self)); dev_note(441, sb_at(description)); dev_note(441, sb_at(*title)); dev_note(441, sb_at(condition));
    return self;
}
static void __thiscall sb_member(void *m, void *window) { dev_note(442, sb_at(m)); dev_note(442, sb_at(window)); }
static void *__thiscall sb_look_up(uint32_t **out, uint32_t **text, void *window, uint32_t description) {
    dev_note(443, *text ? (*text)[3] : 0); dev_note(443, sb_at(window)); dev_note(443, description);
    uint32_t *made = (uint32_t *)dev_take(0x10);
    made[0] = 1 + (dev_next() & 1), made[3] = 0xf8000000u | sb_at(made);
    *out = made;
    return out;
}
static void *__thiscall sb_box(uint8_t *memory, uint32_t zero, uint32_t **looked) {
    dev_note(444, sb_at(memory)); dev_note(444, zero); dev_note(444, (*looked)[3]);
    *(void **)(memory + 0x104 + 0x1c8) = arena<void>(0x7a00);
    return memory;
}
static void __thiscall sb_link(void *at, void *list, void *member) { dev_note(445, sb_at(at)); dev_note(445, sb_at(list)); dev_note(445, sb_at(member)); }
static void __thiscall sb_room(void *list, int32_t n) { dev_note(446, sb_at(list)); dev_note(446, (uint32_t)n); }
static void *__thiscall sb_text(void *condition, uint32_t **out, int32_t i) {
    dev_note(447, sb_at(condition)); dev_note(447, (uint32_t)i);
    if (dev_next() % 4 == 0) *out = arena<uint32_t *>(0x7a00)[i], ++**out;   // the text already there
    else {
        uint32_t *made = (uint32_t *)dev_take(0x10);
        made[0] = 1, made[3] = 0xf9000000u | sb_at(made);
        *out = made;
    }
    return out;
}
static void __fastcall sb_told(void *list) { dev_note(448, sb_at(list)); }
static void __thiscall sb_select(void *list, uint32_t v) { dev_note(449, sb_at(list)); dev_note(449, v); }
static void __thiscall sb_place(void *box, uint32_t relation, void *to) { dev_note(450, sb_at(box)); dev_note(450, relation); dev_note(450, sb_at(to)); }
#else
    Test species_builds{0x6e3400,
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
                            arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                            uint8_t *condition = arena<uint8_t>(0x7700);
                            for (uint32_t off : {0x80u, 0x88u}) *(int32_t *)(condition + off) = r.range(0, 7);
                            for (uint32_t off : {0x84u, 0x90u}) *(uint32_t *)(condition + off) = r.next();
                            for (int k = 0; k < 8; k++) {
                                uint32_t *t = arena<uint32_t>(0x7b00 + 0x10 * k);
                                t[0] = r.range(1, 2), t[3] = 0xfa000000u | (0x7b00 + 0x10 * k);
                                arena<uint32_t *>(0x7a00)[k] = t;
                            }
                        },
                        [](void *fn) {
                            *(uint32_t *)(uintptr_t)sb_rows[g_sb_row][1] = (uint32_t)(uintptr_t)arena<void>(0x7000);
                            const uint32_t *a = arena<uint32_t>(0x40);
                            void *back = ((void *(__thiscall *)(void *, uint32_t, uint32_t, void *))fn)(arena<void>(0x8000), a[0], a[1], arena<void>(0x7700));
                            return Result{sb_at(back) ^ *arena<uint32_t>(0x20)};
                        }};
    species_builds.iterations = 300;
    species_builds.prepare = [](const rewrite::Entry *e) {
        for (uint32_t k = 0; k < 2; k++)
            if (sb_rows[k][0] == e->at) g_sb_row = k;
        g_sb_saved = *(uint32_t *)(uintptr_t)sb_rows[g_sb_row][1];
        stubs_for_now({{0xbcdfc0, (void *)sb_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)cheat_text},
                       {0xbd0490, (void *)desk_empty}, {0x403be0, (void *)cheat_join}, {0x403bb0, (void *)desk_join_wide}, {0x41c740, (void *)desk_copy},
                       {0x6d5380, (void *)sb_base}, {0x6af6c0, (void *)sb_member}, {0x6a39d0, (void *)sb_look_up}, {0x69f710, (void *)sb_box},
                       {0x6af710, (void *)sb_link}, {0xbeca40, (void *)stub_name_make}, {0xbecc10, (void *)stub_name_give}, {0xbec5e0, (void *)stub_name_free},
                       {0x6a0b20, (void *)sb_room}, {sb_rows[g_sb_row][7], (void *)sb_text}, {0x6a0560, (void *)sb_told}, {0x6a0040, (void *)sb_select},
                       {0x6b1330, (void *)sb_place}});
    };
    species_builds.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)(uintptr_t)sb_rows[g_sb_row][1] = g_sb_saved;
    };
    quick(species_builds);
#endif
