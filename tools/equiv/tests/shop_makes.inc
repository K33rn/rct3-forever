// Shop::MakeItem (forever/src/rewrite/shop_makes.cpp). The shop at arena 0x100 with its list (+0x174) at 0x1000: room
// and count near each other (full sometimes); memory sometimes none; the item maker's stand-in notes the reference it
// gets by value (as wild_ticks.inc's: fields, own owner, first in its list; then leaves the list), the value and the
// two zeros, and gives a number; the list's grower notes the room asked for and moves the list to 0x1800.
#if EQUIV_PART == 1
static uint32_t shm_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0; }
struct ShmReference {
    uint32_t w[5];
};
static void *__cdecl shm_alloc(uint32_t size) { dev_note(590, size); return dev_next() % 5 ? dev_take(size) : nullptr; }
static void *__fastcall shm_handle_of(void *object) { dev_note(591, shm_at(object)); return arena<void>(0x600); }
static uint32_t __thiscall shm_item(void *memory, ShmReference ref, uint32_t value, uint32_t a, uint32_t b) {
    dev_note(592, shm_at(memory)); dev_note(592, value); dev_note(592, a); dev_note(592, b);
    dev_note(592, ref.w[0]); dev_note(592, shm_at((void *)(uintptr_t)ref.w[1])); dev_note(592, shm_at((void *)(uintptr_t)ref.w[3]));
    dev_note(592, ref.w[4] == (uint32_t)(uintptr_t)&ref);
    uint32_t *next = (uint32_t *)(uintptr_t)ref.w[2], *head = (uint32_t *)(uintptr_t)ref.w[3];
    dev_note(592, head[0] == (uint32_t)(uintptr_t)&ref.w[2]);
    head[0] = (uint32_t)(uintptr_t)next, next[1] = (uint32_t)(uintptr_t)head;   // (left again)
    return dev_next();
}
static void __thiscall shm_grow(uint32_t *list, int32_t room) {
    dev_note(593, shm_at(list)); dev_note(593, (uint32_t)room);
    uint32_t *more = arena<uint32_t>(0x1800);
    for (int32_t k = 0; k < (int32_t)list[2] && k < 0x100; k++) more[k] = ((uint32_t *)(uintptr_t)list[0])[k];
    list[0] = (uint32_t)(uintptr_t)more, list[1] = (uint32_t)room;
}
#else
    Test shop_makes{0x4ca440,
                    [=](Rng &r) {
                        fresh(r);
                        arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                        uint32_t *list = (uint32_t *)(arena<uint8_t>(0x100) + 0x174);
                        const int32_t count = r.range(0, 0x40), room = r.range(0, 2) ? count : count + r.range(0, 3) - 1;
                        list[0] = (uint32_t)(uintptr_t)arena<void>(0x1000), list[1] = (uint32_t)room, list[2] = (uint32_t)count;
                        arena<uint32_t>(0x600)[0x20 / 4] = (uint32_t)(uintptr_t)arena<void>(0x680);
                        arena<uint32_t>(0x680)[0] = (uint32_t)(uintptr_t)arena<void>(0x690);
                        arena<uint32_t>(0x690)[1] = (uint32_t)(uintptr_t)arena<void>(0x680);
                        arena<uint32_t>(0x50)[0] = r.next();
                    },
                    [](void *fn) {
                        ((void(__fastcall *)(void *, void *, void *, uint32_t))fn)(arena<void>(0x100), nullptr, arena<void>(0x7700), arena<uint32_t>(0x50)[0]);
                        return Result{*arena<uint32_t>(0x20)};
                    }};
    shop_makes.iterations = 1000;
    shop_makes.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)shm_alloc}, {0x805700, (void *)shm_handle_of}, {0x4caad0, (void *)shm_item}, {0x4c9d80, (void *)shm_grow}});
    };
    shop_makes.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(shop_makes);
#endif
