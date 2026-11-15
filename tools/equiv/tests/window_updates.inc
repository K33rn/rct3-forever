// Window::PointAt (forever/src/rewrite/window_updates.cpp). As ride_checks.inc: the window at arena 0x100; its holder's
// reference at 0x600 to an object at 0x640 (or none) whose class info gives the interface or not; kept already or not.
// The two interface calls note the interface; the assignment notes the stack reference and sometimes clears the
// holder's flag and points it at an object that resolves (0x700); the reference's destruction leaves its list again.
#if EQUIV_PART == 1
static const uint32_t wu_rows[][8] = {
#define WINDOW_UPDATES(at, flag, holder, cls, let_go, ref, assign, taken, ...) {at, flag, holder, cls, let_go, ref, assign, taken},
#include "../../../forever/src/rewrite/window_updates.inc"
#undef WINDOW_UPDATES
};
static uint32_t g_wu_row, g_wu_saved;
static uint32_t wu_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0; }
static void *__thiscall wu_info(uint8_t *object) { dev_note(560, wu_at(object)); return *(uint8_t **)(object + 4); }
static void *wu_table[2] = {nullptr, (void *)wu_info};
static void __fastcall wu_let_go(void *i) { dev_note(561, wu_at(i)); }
static void __fastcall wu_taken(void *i) { dev_note(562, wu_at(i)); }
static void *__fastcall wu_handle_of(void *object) { dev_note(563, wu_at(object)); return arena<void>(0x7800); }
static void __thiscall wu_assign(uint8_t *holder, uint32_t *ref) {
    dev_note(564, wu_at(holder)); dev_note(564, ref[0]); dev_note(564, wu_at((void *)(uintptr_t)ref[1])); dev_note(564, wu_at((void *)(uintptr_t)ref[5]));
    dev_note(564, (uint8_t)ref[6]); dev_note(564, ref[4] == (uint32_t)(uintptr_t)ref);
    dev_note(564, *(uint32_t *)(uintptr_t)ref[3] == (uint32_t)(uintptr_t)&ref[2]);
    if (dev_next() % 2) {
        holder[0x18] = 0;
        *(void **)(arena<uint8_t>(0x600) + 0xc) = dev_next() % 3 ? arena<void>(0x700) : nullptr;
    }
}
static void __fastcall wu_reference_gone(uint32_t *ref) {
    uint32_t *next = (uint32_t *)(uintptr_t)ref[2], *head = (uint32_t *)(uintptr_t)ref[3];
    head[0] = (uint32_t)(uintptr_t)next, next[1] = (uint32_t)(uintptr_t)head;   // (left again)
    dev_note(565, ref[0]);
}
#else
    Test window_updates{0x9be750,
                        [=](Rng &r) {
                            fresh(r);
                            arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next();
                            const uint32_t *row = wu_rows[g_wu_row];
                            uint8_t *holder = arena<uint8_t>(0x100) + row[2];
                            *(void **)(holder + 4) = arena<void>(0x600);
                            *(void **)(arena<uint8_t>(0x600) + 0xc) = r.range(0, 2) ? arena<void>(0x640) : nullptr;
                            for (uint32_t o : {0x640u, 0x700u}) {
                                uint8_t *object = arena<uint8_t>(o), *info = arena<uint8_t>(o + 0x40);
                                *(void **)object = wu_table, *(void **)(object + 4) = info;
                                *(int32_t *)(info + 0x18) = r.range(0, 4) ? 2 : 1, *(void **)(info + 0x14) = info + 0x20;
                                *(void **)(info + 0x20) = r.range(0, 3) ? info + 0x30 : nullptr;
                                *(void **)(info + 0x24) = r.range(0, 1) ? info + 0x48 : nullptr;
                                *(int32_t *)(info + 0x30 + 0x10) = 0x10, *(int32_t *)(info + 0x48 + 0x10) = 0x20;
                            }
                            holder[0x18] = (uint8_t)(r.range(0, 3) == 0);
                            if (holder[0x18]) *(void **)(holder + 0x14) = r.range(0, 1) ? arena<void>(0x650) : nullptr;
                            arena<uint32_t>(0x7800)[0x20 / 4] = (uint32_t)(uintptr_t)arena<void>(0x7880);
                            arena<uint32_t>(0x7880)[0] = (uint32_t)(uintptr_t)arena<void>(0x7890);
                            arena<uint32_t>(0x7890)[1] = (uint32_t)(uintptr_t)arena<void>(0x7880);
                            arena<uint32_t>(0x50)[0] = r.range(0, 2);   // the class number
                        },
                        [](void *fn) {
                            *(int32_t *)(uintptr_t)wu_rows[g_wu_row][3] = (int32_t)arena<uint32_t>(0x50)[0];
                            ((void(__fastcall *)(void *, void *, void *))fn)(arena<void>(0x100), nullptr, arena<void>(0x7700));
                            return Result{*arena<uint32_t>(0x20)};
                        }};
    window_updates.iterations = 1000;
    window_updates.prepare = [](const rewrite::Entry *e) {
        for (uint32_t k = 0; k < 2; k++)
            if (wu_rows[k][0] == e->at) g_wu_row = k;
        g_wu_saved = *(uint32_t *)(uintptr_t)wu_rows[g_wu_row][3];
        stubs_for_now({{wu_rows[g_wu_row][4], (void *)wu_let_go}, {wu_rows[g_wu_row][7], (void *)wu_taken}, {0x805700, (void *)wu_handle_of},
                       {wu_rows[g_wu_row][6], (void *)wu_assign}, {0x40f730, (void *)wu_reference_gone}});
    };
    window_updates.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)(uintptr_t)wu_rows[g_wu_row][3] = g_wu_saved;
    };
    quick(window_updates);
#endif
