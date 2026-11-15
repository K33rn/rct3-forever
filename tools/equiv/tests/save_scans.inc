// Save::ScanNumber (forever/src/rewrite/save_scans.cpp). The shared empty text: a dn_ text shared 2-3 times; 0x497300's
// stand-in puts a text of 0-300 characters in its place (shared 1-3 times; the old one let go by one) and gives its
// length, or sometimes a little more (so the text is made longer) or 0 / 256; swscanf's stand-in notes the buffer
// (up to its 0), the format and where to put the number, and puts a number there. Texts are dn_ texts.
#if EQUIV_PART == 1
static DnText *__fastcall ss_empty(uint32_t flag) {
    dev_note(580, flag & 0xff);
    DnText *t = dn_make((DnText *)dev_take(0x10), nullptr, 0);
    t->count = 2 + (int32_t)(dev_next() % 2);
    return t;
}
static int32_t __thiscall ss_text(void *self, DnText **into) {
    dev_note(581, (uint32_t)((uintptr_t)self - (uintptr_t)arena<void>(0)));
    const uint32_t n = arena<uint32_t>(0x50)[0];
    uint16_t wide[300];
    for (uint32_t k = 0; k < n; k++) wide[k] = (uint16_t)(0x20 + (dev_next() % 0x5e));
    --(*into)->count;
    *into = dn_make((DnText *)dev_take(0x10), wide, n);
    (*into)->count = (int32_t)arena<uint32_t>(0x50)[1];
    return (int32_t)arena<uint32_t>(0x50)[2];
}
static int __cdecl ss_scan(const uint16_t *buffer, const void *format, void *out) {
    uint32_t h = 5381, n = 0;
    while (n < 0x100 && buffer[n]) h = h * 33 + buffer[n++];
    dev_note(582, n); dev_note(582, h); dev_note(582, (uint32_t)(uintptr_t)format);
    dev_note(582, (uint32_t)((uintptr_t)out - (uintptr_t)arena<void>(0)));
    *(uint32_t *)out = h;
    return 1;
}
static void *__cdecl ss_alloc(uint32_t size) { dev_note(583, size); return dev_take(size); }
#else
    Test save_scans{0x497a20,
                    [=](Rng &r) {
                        fresh(r);
                        arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                        const uint32_t n = r.range(0, 4) ? r.range(0, 40) : r.range(0, 300);
                        arena<uint32_t>(0x50)[0] = n;
                        arena<uint32_t>(0x50)[1] = r.range(1, 3);
                        uint32_t given = n;
                        switch (r.range(0, 9)) {
                        case 0: given = n + r.range(1, 3); break;
                        case 1: given = 0; break;
                        case 2: given = 0x100 + r.range(0, 2); break;
                        case 3: given = 0xff; break;
                        }
                        if (given > 0x102) given = 0x100;
                        arena<uint32_t>(0x50)[2] = given;
                    },
                    [](void *fn) {
                        const uint8_t back = ((uint8_t(__fastcall *)(void *, void *, void *))fn)(arena<void>(0x100), nullptr, arena<void>(0x60));
                        return Result{back ^ *arena<uint32_t>(0x20) * 3};
                    }};
    save_scans.iterations = 1000;
    save_scans.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xbd0490, (void *)ss_empty}, {0x497300, (void *)ss_text}, {0xdded94, (void *)ss_scan}, {0xbcdfc0, (void *)ss_alloc},
                       {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf710, (void *)dn_from_n}, {0xbcffe0, (void *)dn_room}});
    };
    save_scans.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(save_scans);
#endif
