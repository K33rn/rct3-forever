// WildAnimal::Tick (forever/src/rewrite/wild_ticks.cpp). The animal at arena 0x100 with its counters near N; the
// animal manager there or made; its target none or at 0x7700, whose slot 0xE0 (a stand-in) gets the two references by
// value: it notes them (handles and links as arena offsets, own owner where they lie, first in their lists) and
// leaves the lists again, as the real destructors would. Handle::For gives 0x600, the empty handle 0x700.
#if EQUIV_PART == 1
static uint32_t g_wt_saved;
static uint32_t wt_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0; }
struct WtReference {
    uint32_t w[5];
};
static void wt_note_reference(WtReference &ref) {
    dev_note(500, ref.w[0]); dev_note(500, wt_at((void *)(uintptr_t)ref.w[1])); dev_note(500, wt_at((void *)(uintptr_t)ref.w[3]));
    dev_note(500, ref.w[4] == (uint32_t)(uintptr_t)&ref);
    uint32_t *next = (uint32_t *)(uintptr_t)ref.w[2], *head = (uint32_t *)(uintptr_t)ref.w[3];
    dev_note(500, head[0] == (uint32_t)(uintptr_t)&ref.w[2]);
    head[0] = (uint32_t)(uintptr_t)next, next[1] = (uint32_t)(uintptr_t)head;   // (left again)
}
static uint32_t __thiscall wt_tell(void *target, uint32_t message, WtReference animal, WtReference nothing) {
    dev_note(501, wt_at(target)); dev_note(501, message);
    wt_note_reference(animal);
    wt_note_reference(nothing);
    return dev_next();
}
static void *wt_target_table[0xe0 / 4 + 1];
static void *__cdecl wt_alloc(uint32_t size) { dev_note(502, size); return dev_next() % 5 ? dev_take(size) : nullptr; }
static void __thiscall wt_manager(void *m, uint32_t a, uint32_t b, uint32_t c) {
    dev_note(503, wt_at(m)); dev_note(503, a); dev_note(503, b); dev_note(503, c);
    *(void **)(uintptr_t)0x1306c68 = m;
}
static void *__fastcall wt_target(void *manager) { dev_note(504, wt_at(manager)); return dev_next() % 4 ? arena<void>(0x7700) : nullptr; }
static void *__cdecl wt_null_handle() { dev_note(505, 1); return arena<void>(0x700); }
static void *__fastcall wt_handle_of(void *object) { dev_note(506, wt_at(object)); return arena<void>(0x600); }   // (in the same sum: the order of the two shows)
#else
    Test wild_ticks{0xb38f30,
                    [=](Rng &r) {
                        fresh(r);
                        arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                        wt_target_table[0xe0 / 4] = (void *)wt_tell;
                        *(void **)arena<void>(0x7700) = wt_target_table;
                        for (uint32_t off : {0x254u, 0x258u, 0x25cu}) *(int32_t *)(arena<uint8_t>(0x100) + off) = r.range(0, 3) ? r.range(-2, 12) : (int32_t)r.next();
                        for (uint32_t h : {0x600u, 0x700u}) {
                            arena<uint32_t>(h)[0x20 / 4] = (uint32_t)(uintptr_t)arena<void>(h + 0x80);
                            arena<uint32_t>(h + 0x80)[0] = (uint32_t)(uintptr_t)arena<void>(h + 0x90);
                            arena<uint32_t>(h + 0x90)[1] = (uint32_t)(uintptr_t)arena<void>(h + 0x80);
                        }
                        arena<uint32_t>(0x40)[0] = r.range(0, 1);
                    },
                    [](void *fn) {
                        *(void **)(uintptr_t)0x1306c68 = arena<uint32_t>(0x40)[0] ? arena<void>(0x7800) : nullptr;
                        const uint32_t back = ((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100));
                        return Result{back ^ *arena<uint32_t>(0x20)};
                    }};
    wild_ticks.iterations = 1000;
    wild_ticks.prepare = [](const rewrite::Entry *) {
        g_wt_saved = *(uint32_t *)(uintptr_t)0x1306c68;
        stubs_for_now({{0xbcdfc0, (void *)wt_alloc}, {0x7a70a0, (void *)wt_manager}, {0x7a66b0, (void *)wt_target}, {0x805750, (void *)wt_null_handle},
                       {0x805700, (void *)wt_handle_of}});
    };
    wild_ticks.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)(uintptr_t)0x1306c68 = g_wt_saved;
    };
    quick(wild_ticks);
#endif
