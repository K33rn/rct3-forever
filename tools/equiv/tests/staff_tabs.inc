// StaffWindow::BuildTabs (forever/src/rewrite/staff_tabs.cpp). The window at arena 0x100 (size 0x190); every
// function called is a stand-in that notes what it is given (texts and parts as offsets into the arena); the tab
// manager there already or made (its +0x28 at 0x700); the copy's text table at 0x7400; the global object
// [0x1342708] none, without its own object, or with it marked or not; game modes 0-4. Memory never fails (a missing
// button or part would be used anyway, in the game too). Five game globals are put back after.
#if EQUIV_PART == 1
static const uint32_t st_rows[][14] = {
#define STAFF_TABS(at, t0, t4, tc, t24, s0, s4, s20, texts, k0, k4, k20, value, name, ...) {at, t0, t4, tc, t24, s0, s4, s20, texts, k0, k4, k20, value, name},
#include "../../../forever/src/rewrite/staff_tabs.inc"
#undef STAFF_TABS
};
static uint32_t g_st_row, g_st_saved[5];
static const uint32_t st_globals[5] = {0x12f8084, 0x1342708, 0x10ef988, 0, 0};
static uint32_t st_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0; }
static void *__cdecl st_alloc(uint32_t size) { dev_note(390, size); return dev_take(size); }
static void *__thiscall st_text(uint32_t *self, const char *from) {
    dev_note(391, (uint32_t)(uintptr_t)from);
    self[0] = 1 + (dev_next() & 1), self[3] = 0xf4000000u | st_at(self);
    return self;
}
static void __thiscall st_base(void *self, uint32_t a, uint32_t b) { dev_note(392, st_at(self)); dev_note(392, a); dev_note(392, b); }
static void __thiscall st_manager(void *m, uint32_t zero) {
    dev_note(393, st_at(m)); dev_note(393, zero);
    *(void **)(uintptr_t)0x12f8084 = m;
    *(void **)((uint8_t *)m + 0x28) = arena<void>(0x700);
}
static void st_tab_told(uint8_t *tab, void *button);
static void *st_tab_table[2] = {nullptr, nullptr};
static void __thiscall st_tab(uint8_t *tab, uint32_t zero, void *list) {
    dev_note(394, st_at(tab)); dev_note(394, zero); dev_note(394, st_at(list));
    *(void **)(tab + 4) = st_tab_table;
}
static void __thiscall st_tab_told_stub(uint8_t *tab, void *button) { dev_note(395, st_at(tab)); dev_note(395, st_at(button)); }
static void *__thiscall st_look_up(uint32_t **out, uint32_t **text, uint32_t table) {
    dev_note(396, *text ? (*text)[3] : 0); dev_note(396, table);
    uint32_t *made = (uint32_t *)dev_take(0x10);
    made[0] = 1 + (dev_next() & 1), made[3] = 0xf5000000u | st_at(made);
    *out = made;
    return out;
}
static void *__thiscall st_button(uint8_t *memory, uint32_t zero, uint32_t **found, void *parent, uint32_t id) {
    dev_note(397, st_at(memory)); dev_note(397, zero); dev_note(397, (*found)[3]); dev_note(397, st_at(parent)); dev_note(397, id);
    return memory;
}
static float __thiscall st_size(void *list, uint32_t four) { dev_note(398, st_at(list)); dev_note(398, four); return *arena<float>(0x48); }
static void *__thiscall st_costume(uint8_t *memory, uint32_t zero, uint32_t **found, uint32_t b, float size) {
    uint32_t bits;
    memcpy(&bits, &size, 4);
    dev_note(399, st_at(memory)); dev_note(399, zero); dev_note(399, (*found)[3]); dev_note(399, b); dev_note(399, bits);
    return memory;
}
static void __thiscall st_link3(void *list, uint32_t n, void *part, uint32_t x) { dev_note(400, st_at(list)); dev_note(400, n); dev_note(400, st_at(part)); dev_note(400, x); }
static void __thiscall st_link2(void *list, uint32_t n, void *part) { dev_note(401, st_at(list)); dev_note(401, n); dev_note(401, st_at(part)); }
static void __thiscall st_value(void *part, uint32_t v) { dev_note(402, st_at(part)); dev_note(402, v); }
static void *__thiscall st_named(uint32_t *r, uint32_t name, uint32_t table, uint32_t flag) { dev_note(403, name); dev_note(403, table); dev_note(403, flag); r[0] = name ^ 0x5a5a; return r; }
static void __thiscall st_give(uint8_t *at, uint32_t *r) { dev_note(404, st_at(at)); dev_note(404, r[0]); }
static void __fastcall st_named_gone(uint32_t *r) { dev_note(405, r[0]); }
#else
    Test staff_tabs{0x78d910,
                    [=](Rng &r) {
                        fresh(r);
                        arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                        for (int i = 0; i < 0x1000; i++) arena<uint32_t>(0x2000)[i] = r.next();
                        st_tab_table[1] = (void *)st_tab_told_stub;
                        const uint32_t *row = st_rows[g_st_row];
                        // (the game globals are set in the call: they are not put back between the original's run and the rewrite's)
                        arena<uint8_t>(0x7400)[4] = (uint8_t)r.range(0, 1);
                        arena<uint32_t>(0x50)[0] = r.range(0, 1);   // the manager there already
                        arena<uint32_t>(0x50)[1] = r.range(0, 3);   // the global object: none, empty, or with its own object
                        arena<uint8_t>(0x7600)[0xa4] = (uint8_t)r.range(0, 1);
                        arena<int32_t>(0x50)[2] = r.range(0, 4);    // the game mode
                        arena<uint32_t>(0x40)[0] = r.range(0, 1) ? 0 : r.next(), arena<uint32_t>(0x40)[1] = r.next();
                        const uint32_t odd[] = {0x7fc00000, 0x7f800000, 0x80000000, 0x3f800000};
                        arena<uint32_t>(0x48)[0] = r.range(0, 3) ? r.next() : odd[r.range(0, 3)];
                    },
                    [](void *fn) {
                        *(void **)(uintptr_t)st_rows[g_st_row][8] = arena<void>(0x7400);
                        if (arena<uint32_t>(0x50)[0]) *(void **)(uintptr_t)0x12f8084 = arena<void>(0x6c0), *(void **)(arena<uint8_t>(0x6c0) + 0x28) = arena<void>(0x700);
                        else *(void **)(uintptr_t)0x12f8084 = nullptr;
                        const uint32_t g = arena<uint32_t>(0x50)[1];
                        *(void **)(uintptr_t)0x1342708 = g ? arena<void>(0x7500) : nullptr;
                        *arena<void *>(0x7500) = g >= 2 ? arena<void>(0x7600) : nullptr;
                        *(int32_t *)(uintptr_t)0x10ef988 = arena<int32_t>(0x50)[2];
                        void *back = ((void *(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0], arena<uint32_t>(0x40)[1]);
                        return Result{st_at(back) ^ *arena<uint32_t>(0x20)};
                    }};
    staff_tabs.iterations = 500;
    staff_tabs.prepare = [](const rewrite::Entry *e) {
        for (uint32_t k = 0; k < 3; k++)
            if (st_rows[k][0] == e->at) g_st_row = k;
        g_st_saved[0] = *(uint32_t *)(uintptr_t)0x12f8084, g_st_saved[1] = *(uint32_t *)(uintptr_t)0x1342708, g_st_saved[2] = *(uint32_t *)(uintptr_t)0x10ef988;
        g_st_saved[3] = *(uint32_t *)(uintptr_t)st_rows[g_st_row][8];
        stubs_for_now({{0xbcdfc0, (void *)st_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)st_text},
                       {0x78d290, (void *)st_base}, {0x659770, (void *)st_manager}, {0x63c840, (void *)st_tab}, {0x4b7940, (void *)st_look_up},
                       {0x6a2620, (void *)st_button}, {0x63a860, (void *)st_size}, {0x7ef3d0, (void *)st_costume}, {0x63b3d0, (void *)st_link3},
                       {0x63b270, (void *)st_link2}, {0x65aa90, (void *)st_value}, {0x4040f0, (void *)st_named}, {0xbd14e0, (void *)st_give},
                       {0xbd1880, (void *)st_named_gone},
                       // the tabs' own tables (set over the stand-in's) are the game's: their slot 1 is what the last step calls
                       {*(const uint32_t *)(uintptr_t)(st_rows[g_st_row][6] + 4), (void *)st_tab_told_stub},
                       {*(const uint32_t *)(uintptr_t)(st_rows[g_st_row][10] + 4), (void *)st_tab_told_stub}});
    };
    staff_tabs.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)(uintptr_t)0x12f8084 = g_st_saved[0], *(uint32_t *)(uintptr_t)0x1342708 = g_st_saved[1], *(uint32_t *)(uintptr_t)0x10ef988 = g_st_saved[2];
        *(uint32_t *)(uintptr_t)st_rows[g_st_row][8] = g_st_saved[3];
    };
    quick(staff_tabs);
#endif
