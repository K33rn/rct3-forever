// Scenery::ScanNumber (forever/src/rewrite/scenery_scans.cpp). 0x834260's stand-in gives 0 or not and puts in the
// text's place a dn_ text of 0-40 characters (a 0 inside sometimes; held 1-2 times; its end marked with a 0 or not,
// its room just its length sometimes; the old one let go by one); sscanf's stand-in notes the narrowed text, the format
// and the place, and puts a number there. The empty text is ds_empty's; the room maker dn_room.
#if EQUIV_PART == 1
static uint32_t scn_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0; }
static int32_t __thiscall scn_text(void *self, DnText **into) {
    dev_note(610, scn_at(self));
    const uint32_t n = arena<uint32_t>(0x50)[0];
    uint16_t wide[48];
    for (uint32_t k = 0; k < n; k++) wide[k] = (uint16_t)(dev_next() % 7 == 0 ? 0x100 + dev_next() % 0x300 : '0' + dev_next() % 10);
    if (n && arena<uint32_t>(0x50)[3]) wide[dev_next() % n] = 0;
    --(*into)->count;
    *into = dn_make((DnText *)dev_take(0x10), wide, n);
    (*into)->count = (int32_t)arena<uint32_t>(0x50)[1];
    switch (arena<uint32_t>(0x50)[4]) {
    case 1: (*into)->characters[n] = 'z'; break;   // (its end not marked)
    case 2: (*into)->room = n; break;              // (no room past its end)
    }
    return (int32_t)arena<uint32_t>(0x50)[2];
}
static int __cdecl scn_scan(const char *narrow, const char *format, uint32_t *out) {
    uint32_t h = 5381, n = 0;
    while (n < 0x100 && narrow[n]) h = h * 33 + (uint8_t)narrow[n++];
    dev_note(611, n); dev_note(611, h); dev_note(611, (uint32_t)(uintptr_t)format); dev_note(611, scn_at(out));
    *out = h;
    return 1;
}
#else
    Test scenery_scans{0x8345a0,
                       [=](Rng &r) {
                           fresh(r);
                           arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                           arena<uint32_t>(0x50)[0] = r.range(0, 40);
                           arena<uint32_t>(0x50)[1] = r.range(1, 2);
                           arena<uint32_t>(0x50)[2] = r.range(0, 3) ? 0 : r.next() | 1;
                           arena<uint32_t>(0x50)[3] = r.range(0, 5) == 0;
                           arena<uint32_t>(0x50)[4] = r.range(0, 2);
                           arena<uint32_t>(0x80)[0] = r.next();
                       },
                       [](void *fn) {
                           const uint32_t back = ((uint32_t(__fastcall *)(void *, void *, void *))fn)(arena<void>(0x100), nullptr, arena<void>(0x80));
                           return Result{back ^ *arena<uint32_t>(0x20) * 3};
                       }};
    scenery_scans.iterations = 1000;
    scenery_scans.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbd0490, (void *)ds_empty}, {0x834260, (void *)scn_text}, {0xddd661, (void *)scn_scan}, {0xbcffe0, (void *)dn_room},
                       {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}});
    };
    scenery_scans.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(scenery_scans);
#endif
