// Pair::Compare (forever/src/rewrite/pair_compares.cpp). Two pairs of counted texts in the arena (counts 0 to 2, so
// letting a copy go sometimes frees it); the compare's stand-in notes the texts it is given, their counts and the
// compare object's byte, and gives 0 or a value by the case (one per call); the frees noted.
#if EQUIV_PART == 1
static uint32_t pc_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0; }
static int __fastcall pc_compare(uint8_t *self, void *, uint32_t **x, uint32_t **y) {
    dev_note(760, *self); dev_note(760, pc_at(*x)); dev_note(760, pc_at(*y)); dev_note(760, (*x)[0]); dev_note(760, (*y)[0]);
    uint32_t &calls = arena<uint32_t>(0x50)[2];
    return arena<int32_t>(0x50)[calls++ & 1];
}
static void __cdecl pc_free_aligned(void *p) { dev_note(761, pc_at(p)); }
static void __cdecl pc_free(void *p) { dev_note(762, pc_at(p)); }
#else
    Test pair_compare{0x4a36e0,
                      [=](Rng &r) {
                          fresh(r);
                          for (uint32_t i = 0; i < 4; i++) {   // the four texts (0x200 + 0x40 i): count, ..., memory
                              uint32_t *t = arena<uint32_t>(0x200 + 0x40 * i);
                              t[0] = r.range(0, 3), t[1] = r.next(), t[2] = r.next(), t[3] = 0x400 + 0x10 * i + (uint32_t)(uintptr_t)arena<void>(0);
                          }
                          uint32_t *a = arena<uint32_t>(0x100), *b = arena<uint32_t>(0x120);
                          a[0] = (uint32_t)(uintptr_t)arena<void>(0x200), a[1] = (uint32_t)(uintptr_t)arena<void>(0x240);
                          b[0] = (uint32_t)(uintptr_t)arena<void>(0x280), b[1] = (uint32_t)(uintptr_t)arena<void>(0x2c0);
                          arena<int32_t>(0x50)[0] = r.range(0, 2) ? 0 : (int32_t)r.next();   // the first compare's result
                          arena<int32_t>(0x50)[1] = r.range(0, 3) ? (int32_t)r.next() : 0;   // the second's
                          arena<uint32_t>(0x50)[2] = 0;
                      },
                      [](void *fn) {
                          int got = ((int(__fastcall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x120));
                          uint32_t counts = 0;
                          for (uint32_t i = 0; i < 4; i++) counts = counts * 7 + arena<uint32_t>(0x200 + 0x40 * i)[0];
                          return Result{(uint32_t)got ^ counts * 3 ^ arena<uint32_t>(0x50)[2] << 20};
                      }};
    pair_compare.iterations = 1000;
    pair_compare.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0xcd5640, (void *)pc_compare}, {0xbce010, (void *)pc_free_aligned}, {0xbce000, (void *)pc_free}});
    };
    pair_compare.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(pair_compare);
#endif
