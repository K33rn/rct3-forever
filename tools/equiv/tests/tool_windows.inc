// Designer::BuildTool (forever/src/rewrite/tool_windows.cpp). As the ident bars' test (ident_bars.inc) for the
// title: the window at arena 0x8000, the default description at 0x7000 (its own name none or a C text), a
// description or none (0x7300), a title or none; the object at 0x7700, the part at 0x7800 (its slot 0x100 a
// stand-in); every function called is a stand-in noting what it is given; memory never fails (the tooltip and the
// OK texts are let go unchecked, in the game too). The copy's default-description global is set in the call and put
// back after.
#if EQUIV_PART == 1
static const uint32_t tw2_rows[][12] = {
#define TOOL_WINDOWS(at, description, t0, t4, tdc, te4, n1, n2, n3, count, unit, tooltip, ...) {at, description, t0, t4, tdc, te4, n1, n2, n3, count, unit, tooltip},
#include "../../../forever/src/rewrite/tool_windows.inc"
#undef TOOL_WINDOWS
};
static uint32_t g_tw2_row, g_tw2_saved;
static uint32_t tw2_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0; }
static void *__cdecl tw2_alloc(uint32_t size) { dev_note(410, size); return dev_take(size); }
static void *__thiscall tw2_base(uint32_t *self, void *description, uint32_t **title, uint32_t one) {
    dev_note(411, tw2_at(self)); dev_note(411, tw2_at(description)); dev_note(411, tw2_at(*title)); dev_note(411, one); dev_note(411, self[0]);
    return self;
}
static void __thiscall tw2_told(void *part, uint32_t one, void *at) { dev_note(412, tw2_at(part)); dev_note(412, one); dev_note(412, tw2_at(at)); }
static void *tw2_part_table[0x100 / 4 + 1];
static void *__thiscall tw2_control(uint8_t *memory, uint32_t a, uint32_t b, uint32_t c) { dev_note(413, tw2_at(memory)); dev_note(413, a); dev_note(413, b); dev_note(413, c); return memory; }
static void __thiscall tw2_place(void *control, uint32_t relation, void *to) { dev_note(414, tw2_at(control)); dev_note(414, relation); dev_note(414, tw2_at(to)); }
static void __thiscall tw2_numbers(void *control, uint32_t a, uint32_t b, uint32_t c) { dev_note(415, tw2_at(control)); dev_note(415, a); dev_note(415, b); dev_note(415, c); }
static void __thiscall tw2_count(void *control, uint32_t a, uint32_t b) { dev_note(416, tw2_at(control)); dev_note(416, a); dev_note(416, b); }
static void __thiscall tw2_target(void *control, void *at) { dev_note(417, tw2_at(control)); dev_note(417, tw2_at(at)); }
static void __thiscall tw2_unit(void *control, uint32_t unit) { dev_note(418, tw2_at(control)); dev_note(418, unit); }
static void *__thiscall tw2_named(uint32_t *r, uint32_t name, uint32_t table, uint32_t flag) {
    dev_note(419, name); dev_note(419, table); dev_note(419, flag);
    r[0] = name ^ 0x5a5a, r[2] = dev_next() % 3 ? (uint32_t)(uintptr_t)arena<void>(0x7900) : 0;
    return r;
}
static void *__thiscall tw2_from_wide(uint32_t *self, const uint16_t *wide) { dev_note(420, tw2_at(wide)); self[0] = 1 + (dev_next() & 1); self[3] = 0xf6000000u | tw2_at(self); return self; }
static void __thiscall tw2_tooltip(void *self, uint32_t **text) { dev_note(421, tw2_at(self)); dev_note(421, (*text)[3]); }
static void __fastcall tw2_named_gone(uint32_t *r) { dev_note(422, r[0]); }
static void *__thiscall tw2_look_up(uint32_t **out, uint32_t **text, void *window, uint32_t description) {
    dev_note(423, (*text)[3]); dev_note(423, tw2_at(window)); dev_note(423, description);
    uint32_t *made = (uint32_t *)dev_take(0x10);
    made[0] = 1 + (dev_next() & 1), made[3] = 0xf7000000u | tw2_at(made);
    *out = made;
    return out;
}
static void *__thiscall tw2_button(uint8_t *memory, uint32_t zero, uint32_t **text, uint32_t a, uint32_t b) {
    dev_note(424, tw2_at(memory)); dev_note(424, zero); dev_note(424, (*text)[3]); dev_note(424, a); dev_note(424, b);
    for (uint32_t off = 0x7c; off <= 0x88; off += 4) *(uint32_t *)(memory + off) = *arena<uint32_t>(0x60 + off - 0x7c);
    return memory;
}
static void __thiscall tw2_b1(void *b, uint32_t v) { dev_note(425, tw2_at(b)); dev_note(425, v); }
static void __thiscall tw2_b2(void *b, uint32_t v, uint32_t w) { dev_note(426, tw2_at(b)); dev_note(426, v); dev_note(426, w); }
static void __thiscall tw2_b3(void *b, uint32_t v, uint32_t w) { dev_note(427, tw2_at(b)); dev_note(427, v); dev_note(427, w); }
static void __thiscall tw2_give(uint8_t *at, uint32_t *r) { dev_note(428, tw2_at(at)); dev_note(428, r[0]); }
static void __thiscall tw2_layout(void *b, uint32_t relation, void *to, const uint32_t *rect) {
    dev_note(429, tw2_at(b)); dev_note(429, relation); dev_note(429, tw2_at(to));
    for (int k = 0; k < 4; k++) dev_note(429, rect[k]);
}
#else
    Test tool_windows{0xacd9b0,
                      [=](Rng &r) {
                          fresh(r);
                          arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                          for (int i = 0; i < 0x60; i++) arena<uint32_t>(0x8000)[i] = r.next();
                          for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x7000)[i] = r.next();
                          for (int i = 0; i < 0x1000; i++) arena<uint32_t>(0x2000)[i] = r.next();
                          arena<uint32_t>(0x7000)[2] = r.range(0, 2) ? 0xf2738c : 0;
                          arena<uint32_t>(0x7100)[0] = (uint32_t)(uintptr_t)arena<void>(0x7200);
                          arena<uint32_t>(0x7200)[0] = r.range(1, 3);
                          arena<uint32_t>(0x7180)[0] = (uint32_t)(uintptr_t)arena<void>(0x7280);   // (the second holder the title stand-ins sometimes give)
                          arena<uint32_t>(0x7280)[0] = r.range(1, 3);
                          uint32_t *handle = arena<uint32_t>(0x600);
                          handle[0x20 / 4] = (uint32_t)(uintptr_t)arena<void>(0x680);
                          arena<uint32_t>(0x680)[0] = (uint32_t)(uintptr_t)arena<void>(0x690);
                          arena<uint32_t>(0x690)[1] = (uint32_t)(uintptr_t)arena<void>(0x680);
                          tw2_part_table[0x100 / 4] = (void *)tw2_told;
                          *(void **)arena<void>(0x7800) = tw2_part_table;
                          arena<uint32_t>(0x40)[0] = r.range(0, 1) ? (uint32_t)(uintptr_t)arena<void>(0x7300) : 0;
                          arena<uint32_t>(0x40)[1] = r.range(0, 2) ? (uint32_t)(uintptr_t)arena<void>(0x7100) : 0;
                          for (int k = 0; k < 4; k++) arena<float>(0x60)[k] = r.range(0, 5) ? r.real(-300, 300) : r.any_float();   // the button's place
                      },
                      [](void *fn) {
                          *(uint32_t *)(uintptr_t)tw2_rows[g_tw2_row][1] = (uint32_t)(uintptr_t)arena<void>(0x7000);
                          const uint32_t *a = arena<uint32_t>(0x40);
                          void *back = ((void *(__thiscall *)(void *, uint32_t, uint32_t, void *, void *))fn)(arena<void>(0x8000), a[0], a[1], arena<void>(0x7700), arena<void>(0x7800));
                          return Result{tw2_at(back) ^ *arena<uint32_t>(0x20)};
                      }};
    tool_windows.iterations = 300;
    tool_windows.prepare = [](const rewrite::Entry *e) {
        for (uint32_t k = 0; k < 4; k++)
            if (tw2_rows[k][0] == e->at) g_tw2_row = k;
        g_tw2_saved = *(uint32_t *)(uintptr_t)tw2_rows[g_tw2_row][1];
        stubs_for_now({{0xbcdfc0, (void *)tw2_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)cheat_text},
                       {0xbd0490, (void *)desk_empty}, {0x403be0, (void *)cheat_join}, {0x403bb0, (void *)desk_join_wide}, {0x41c740, (void *)desk_copy},
                       {0x67dac0, (void *)tw2_base}, {0x805700, (void *)stub_member_handle_of}, {0x6ab2a0, (void *)tw2_control}, {0x6a8d00, (void *)tw2_control},
                       {0x6b1330, (void *)tw2_place}, {0x6a82d0, (void *)tw2_numbers}, {0x6a7920, (void *)tw2_count}, {0x6a8300, (void *)tw2_target},
                       {0x4d7e70, (void *)tw2_unit}, {0x4040f0, (void *)tw2_named}, {0xbcf7d0, (void *)tw2_from_wide}, {0x6ae070, (void *)tw2_tooltip},
                       {0xbd1880, (void *)tw2_named_gone}, {0x6a39d0, (void *)tw2_look_up}, {0x67be80, (void *)tw2_button}, {0x67a7a0, (void *)tw2_b1},
                       {0x67b4b0, (void *)tw2_b2}, {0x67b800, (void *)tw2_b3}, {0xbd14e0, (void *)tw2_give}, {0x6b1640, (void *)tw2_layout}});
    };
    tool_windows.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)(uintptr_t)tw2_rows[g_tw2_row][1] = g_tw2_saved;
    };
    quick(tool_windows);
#endif
