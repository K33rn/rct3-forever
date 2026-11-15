// Ride::SwitchText (forever/src/rewrite/ride_switch_texts.cpp). The ride at arena 0x100 (its name: none or a C text);
// the text made at 0x140. Texts are the cheats/desktop stand-ins' (counted blocks from dev_take). Memory never fails:
// with no name text the join and the last let-go would use none, in the game too.
#if EQUIV_PART == 1
static void *__cdecl rs_alloc(uint32_t size) { dev_note(222, size); return dev_take(size); }
static void *__thiscall rs_copy(uint32_t **out, uint32_t **from) {
    dev_note(220, (uint32_t)(uintptr_t)*from);
    uint32_t *made = (uint32_t *)dev_take(0x10);
    made[0] = 1 + (dev_next() & 1), made[3] = 0xe0000000u | (uint32_t)(uintptr_t)made;
    *out = made;
    return out;
}
static void *__thiscall rs_reference(uint32_t *made, uint32_t **text, uint32_t table, uint32_t flag) {
    dev_note(221, (uint32_t)(uintptr_t)made); dev_note(221, (uint32_t)(uintptr_t)*text); dev_note(221, table); dev_note(221, flag);
    made[0] = 0x7e57;
    return made;
}
#else
    Test ride_switch_texts{0x4f06b0,
                           [=](Rng &r) {
                               fresh(r);
                               for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                               *arena<const char *>(0x100) = r.range(0, 2) ? "Ride" : nullptr;
                               arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;
                           },
                           [](void *fn) {
                               void *back = ((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x140));
                               return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ *arena<uint32_t>(0x20)};
                           }};
    ride_switch_texts.iterations = 500;
    ride_switch_texts.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbcdfc0, (void *)rs_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)cheat_text},
                       {0xbd0490, (void *)desk_empty}, {0x403be0, (void *)cheat_join}, {0xbd01c0, (void *)rs_copy}, {0x41cab0, (void *)rs_reference}});
    };
    ride_switch_texts.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(ride_switch_texts);
#endif
