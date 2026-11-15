// Ride::WindowText (forever/src/rewrite/ride_text_windows.cpp). The ride at arena 0x100 (its table at 0x7000), its
// window none or at 0x700 with its panel at 0x800; the panel's text a dn_ text held 1-3 times; the ride's slot 0x4C
// gives that same text (held once more) or a new one held 1-2 times; the frees note what they free.
#if EQUIV_PART == 1
static uint32_t rw_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0; }
static void __fastcall rw_first(void *window) { dev_note(600, rw_at(window)); }
static void __thiscall rw_again(void *window, uint32_t value) { dev_note(601, rw_at(window)); dev_note(601, value); }
static void *__thiscall rw_text(void *ride, DnText **into) {
    dev_note(602, rw_at(ride));
    DnText *shown = *(DnText **)(arena<uint8_t>(0x800) + 0x114);
    if (arena<uint32_t>(0x50)[0]) {
        *into = shown;
        shown->count++;
    } else {
        const uint16_t wide[3] = {'a', 'b', 'c'};
        *into = dn_make((DnText *)dev_take(0x10), wide, 3);
        (*into)->count = (int32_t)arena<uint32_t>(0x50)[1];
    }
    return into;
}
static void *rw_table[0x4c / 4 + 1];
#else
    Test ride_text_windows{0x503860,
                           [=](Rng &r) {
                               fresh(r);
                               arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                               rw_table[0x4c / 4] = (void *)rw_text;
                               *(void **)arena<void>(0x100) = rw_table;
                               *(void **)(arena<uint8_t>(0x100) + 0x18) = r.range(0, 5) ? arena<void>(0x700) : nullptr;
                               *(void **)(arena<uint8_t>(0x700) + 0xe0) = arena<void>(0x800);
                               const uint16_t wide[2] = {'x', 'y'};
                               DnText *shown = dn_make((DnText *)dev_take(0x10), wide, 2);
                               shown->count = r.range(1, 3);
                               *(DnText **)(arena<uint8_t>(0x800) + 0x114) = shown;
                               arena<uint32_t>(0x50)[0] = r.range(0, 2) == 0;
                               arena<uint32_t>(0x50)[1] = r.range(1, 2);
                           },
                           [](void *fn) {
                               ((void(__fastcall *)(void *))fn)(arena<void>(0x100));
                               return Result{*arena<uint32_t>(0x20)};
                           }};
    ride_text_windows.iterations = 1000;
    ride_text_windows.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0x5036d0, (void *)rw_first}, {0x501da0, (void *)rw_again}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}});
    };
    ride_text_windows.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(ride_text_windows);
#endif
