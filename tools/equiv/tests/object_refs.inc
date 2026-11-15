// Object9AF::Refer (forever/src/rewrite/object_refs.cpp). The object at arena 0x100 (size 0x160); its holder's
// reference at 0x300 to an object at 0x400 (or none) whose class info (0x500) gives the interface or not; kept
// already or not; the object given sometimes is the kept interface. Handle::For gives the class test's handle (0x600,
// its list at 0x680); every function called is a stand-in; memory never fails.
#if EQUIV_PART == 1
static const uint32_t or_rows[][7] = {
#define OBJECT_REFS(at, handler, looked, holder, number, table, assign, ...) {at, handler, looked, holder, number, table, assign},
#include "../../../forever/src/rewrite/object_refs.inc"
#undef OBJECT_REFS
};
static uint32_t g_or_row, g_or_saved;
static void *__cdecl or_alloc(uint32_t size) { dev_note(260, size); return dev_take(size); }
static void *__thiscall or_info(uint8_t *object) { dev_note(261, (uint32_t)(uintptr_t)object); return *(uint8_t **)(object + 4); }
static void *or_table[2] = {nullptr, (void *)or_info};
static void __fastcall or_note_self(uint8_t *self) {   // the object's refresh and updates: what it has by then
    dev_note(262, (uint32_t)(uintptr_t)self); dev_note(262, self[0x154]); dev_note(262, *(uint32_t *)(self + 0xe0));
}
static void __fastcall or_update_a(uint8_t *self) { dev_note(265, 1); or_note_self(self); }   // (each update told apart)
static void __fastcall or_update_b(uint8_t *self) { dev_note(265, 2); or_note_self(self); }
static void __thiscall or_assign(uint8_t *holder, const uint32_t *reference) {
    dev_note(263, (uint32_t)(uintptr_t)holder);
    for (int k = 0; k < 6; k++) dev_note(263, reference[k] - (k == 4 ? (uint32_t)(uintptr_t)reference : 0));
    dev_note(263, (uint8_t)reference[6]);
}
static void __fastcall or_gone(uint32_t *reference) {   // (leaves the handle's list, as ObjectRef's destructor does: no stack address stays behind)
    dev_note(264, reference[0]); dev_note(264, reference[5]); dev_note(264, reference[3]);
    uint32_t *next = (uint32_t *)(uintptr_t)reference[2], *prev = (uint32_t *)(uintptr_t)reference[3];
    prev[0] = (uint32_t)(uintptr_t)next, next[1] = (uint32_t)(uintptr_t)prev;
}
#else
    Test object_refs{0x9aeed0,
                     [=](Rng &r) {
                         fresh(r);
                         arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                         const uint32_t *row = or_rows[g_or_row];
                         uint8_t *self = arena<uint8_t>(0x100), *holder = self + row[3], *ref = arena<uint8_t>(0x300), *object = arena<uint8_t>(0x400),
                                 *info = arena<uint8_t>(0x500);
                         *(int32_t *)(uintptr_t)row[4] = r.range(0, 3);
                         *(void **)(holder + 4) = ref;
                         *(void **)(ref + 0xc) = r.range(0, 5) ? object : nullptr;
                         *(void **)object = or_table;
                         *(void **)(object + 4) = r.range(0, 5) ? info : nullptr;
                         *(int32_t *)(info + 0x18) = r.range(0, 4);
                         *(void **)(info + 0x14) = info + 0x40;
                         for (int k = 0; k < 4; k++) ((void **)(info + 0x40))[k] = r.range(0, 4) ? info + 0x80 : nullptr;
                         *(int32_t *)(info + 0x80 + 0x10) = 0x40;
                         holder[0x18] = (uint8_t)(r.range(0, 2) == 0);
                         if (holder[0x18]) *(void **)(holder + 0x14) = r.range(0, 1) ? object + 0x40 : nullptr;
                         uint32_t *text = (uint32_t *)dev_take(0x10);
                         text[0] = r.range(1, 2), text[3] = 0xf1000000u | (uint32_t)(uintptr_t)text;
                         *(uint32_t **)(self + 0xe0) = text;
                         self[0x154] = (uint8_t)r.next();
                         uint32_t *handle = arena<uint32_t>(0x600);
                         handle[0x20 / 4] = (uint32_t)(uintptr_t)arena<void>(0x680);
                         arena<uint32_t>(0x680)[0] = (uint32_t)(uintptr_t)arena<void>(0x690);
                         arena<uint32_t>(0x690)[1] = (uint32_t)(uintptr_t)arena<void>(0x680);
                         arena<uint32_t>(0x40)[0] = (uint32_t)(uintptr_t)(r.range(0, 2) ? object + 0x40 : arena<uint8_t>(0x700));
                     },
                     [](void *fn) {
                         ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0]);
                         return Result{*arena<uint32_t>(0x20)};
                     }};
    object_refs.iterations = 1000;
    object_refs.prepare = [](const rewrite::Entry *e) {
        for (uint32_t k = 0; k < 3; k++)
            if (or_rows[k][0] == e->at) g_or_row = k;
        g_or_saved = *(uint32_t *)(uintptr_t)or_rows[g_or_row][4];
        stubs_for_now({{0xbcdfc0, (void *)or_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)cheat_text},
                       {0x9ae960, (void *)or_note_self}, {0x805700, (void *)stub_member_handle_of}, {or_rows[g_or_row][6], (void *)or_assign},
                       {0x40f730, (void *)or_gone}, {0x9acb70, (void *)or_update_a}, {0x9ac930, (void *)or_update_b}});
    };
    object_refs.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)(uintptr_t)or_rows[g_or_row][4] = g_or_saved;
    };
    quick(object_refs);
#endif
