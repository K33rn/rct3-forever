// Object::MakeNamed (forever/src/rewrite/path_makes.cpp). The name given (held at 0x60) a dn_ text; the constructor's
// stand-in notes its arguments, gives the object a name (the given one, or another held once or more) and a drawn
// mark byte at +0x2C; the readying's stand-in notes the object. (Memory is always given: with none the game reads
// through a null object, as the rewrite does.)
#if EQUIV_PART == 1
static uint32_t g_pm_name_at;
static uint32_t pm_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0; }
static void *__cdecl pm_alloc(uint32_t size) { dev_note(720, size); return dev_take(0x100); }
static void *__thiscall pm_make(uint8_t *object, uint32_t a, uint32_t b, uint32_t c) {
    dev_note(721, pm_at(object)); dev_note(721, a ^ b * 3 ^ c * 7);
    for (int k = 0; k < 0x40; k++) ((volatile uint32_t *)object)[k] = dev_next();
    *(DnText **)(object + g_pm_name_at) = arena<uint32_t>(0x50)[0] ? *arena<DnText *>(0x60) : *arena<DnText *>(0x64);
    object[0x2c] = (uint8_t)arena<uint32_t>(0x50)[2];   // (drawn in the setup: dev_next runs the same in every case)
    return object;
}
static void __fastcall pm_ready(uint8_t *object) { dev_note(722, pm_at(object)); dev_note(722, *(uint32_t *)(object + g_pm_name_at + 4)); }
#else
    Test path_makes{0x731220,
                    [=](Rng &r) {
                        fresh(r);
                        arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x2c)[0] = 0x2000;
                        const uint16_t wide[6] = {'P', 'a', 't', 'h', 'X', 'y'};
                        DnText *name = dn_make((DnText *)dev_take(0x10), wide, (uint32_t)r.range(0, 6));
                        name->count = r.range(1, 3);
                        DnText *other = dn_make((DnText *)dev_take(0x10), wide + 2, (uint32_t)r.range(0, 4));
                        other->count = r.range(1, 2);
                        *arena<DnText *>(0x60) = name, *arena<DnText *>(0x64) = other;
                        arena<uint32_t>(0x50)[0] = r.range(0, 3) == 0;   // the object named so already
                        arena<uint32_t>(0x50)[1] = r.next();             // the value
                        arena<uint32_t>(0x50)[2] = r.next() >> 13;       // the mark byte
                    },
                    [](void *fn) {
                        void *back = ((void *(__fastcall *)(void *, uint32_t))fn)(arena<void>(0x60), arena<uint32_t>(0x50)[1]);
                        return Result{pm_at(back)};
                    }};
    path_makes.iterations = 1500;
    path_makes.prepare = [](const rewrite::Entry *e) {
        g_pm_name_at = e->at == 0xac3270 ? 0xb4 : 0xc0;
        const uintptr_t make = e->at == 0x731220 ? 0x730db0 : e->at == 0x739540 ? 0x739100 : 0xac2ed0;
        stubs_for_now({{0xbcdfc0, (void *)pm_alloc}, {make, (void *)pm_make}, {0x800860, (void *)pm_ready},
                       {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}});
    };
    path_makes.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(path_makes);
#endif
