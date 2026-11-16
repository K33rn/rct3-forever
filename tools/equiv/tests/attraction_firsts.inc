// Attraction::First (forever/src/rewrite/attraction_firsts.cpp). The attraction at arena 0x1000 (random fields); its
// reference (+0x1BC) linked into handle 0x1800's list. A list of 0 to 4 objects (0x1400 + 0x80 i), the list global
// pointing at 0x1380; each object's type (by the case: none, or one at 0x1600 + 0x40 i with 0 to 4 parts, each none or
// an offset) and each copy's global (0 to 4, sometimes -1) set per case. (r.range includes its upper bound.) The checks give 0 to 3 by the case, one value
// per call; the stand-ins note what they are given. Handle::For gives 0x1900. The result: al, the attraction's fields,
// the lists, the calls made.
#if EQUIV_PART == 1
static uint32_t af_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0xffffffffu; }
static uint32_t af_saved[3];
static const uint32_t af_globals[3] = {0x12f4e4c, 0x13cde74, 0x13d0680};
static uint8_t *__fastcall af_next(void *list, void *, uint8_t *after) {
    dev_note(790, af_at(list)); dev_note(790, af_at(after));
    uint32_t i = after ? (af_at(after) - 0x1400) / 0x80 + 1 : 0;
    return i < arena<uint32_t>(0x50)[0] ? arena<uint8_t>(0x1400 + 0x80 * i) : nullptr;
}
static void *__fastcall af_type(uint8_t *object) {
    dev_note(791, af_at(object));
    uint32_t i = (af_at(object) - 0x1400) / 0x80;
    return arena<uint32_t>(0x60)[i] ? arena<void>(0x1600 + 0x40 * i) : nullptr;
}
static void *af_object_table[2] = {nullptr, (void *)af_type};
static uint32_t af_answer() { uint32_t &n = arena<uint32_t>(0x50)[1]; return (arena<uint32_t>(0x70)[n++ & 15]); }
static uint32_t __fastcall af_first(uint8_t *self, void *, uint8_t *object) { dev_note(792, af_at(self)); dev_note(792, af_at(object)); return af_answer(); }
static uint32_t __fastcall af_second(uint8_t *self, void *, uint8_t *part) { dev_note(793, af_at(self)); dev_note(793, af_at(part)); return af_answer(); }
static void __fastcall af_release(void *handle, void *, uint32_t z) { dev_note(794, af_at(handle)); dev_note(794, z); }
static void *__fastcall af_handle_for(uint8_t *object) { dev_note(795, af_at(object)); return arena<void>(0x1900); }
static void __fastcall af_told(uint32_t *r) { dev_note(796, af_at(r)); dev_note(796, af_at((void *)(uintptr_t)r[1])); dev_note(796, af_at((void *)(uintptr_t)r[2])); }
static void *af_ref_table[1] = {(void *)af_told};
static uint32_t __fastcall af_value(uint8_t *object, void *, uint32_t *place, uint32_t flag) {
    dev_note(797, af_at(object)); dev_note(797, place[0]); dev_note(797, place[1]); dev_note(797, place[2]); dev_note(797, flag);
    return arena<uint32_t>(0x50)[2];
}
static void __fastcall af_step(uint8_t *self, void *, uint32_t a, uint32_t b) { dev_note(798, af_at(self)); dev_note(798, a); dev_note(798, b); }
static void af_handle(uint32_t at) {   // a handle: its list (+0x20: a head at +0x80, one node at +0x90)
    uint32_t *h = arena<uint32_t>(at + 0x80), *o = arena<uint32_t>(at + 0x90);
    h[0] = h[1] = (uint32_t)(uintptr_t)o;
    o[0] = o[1] = (uint32_t)(uintptr_t)h;
    arena<uint32_t>(at)[8] = (uint32_t)(uintptr_t)h;
}
#else
    Test attraction_first{0x7ad080,
                          [=](Rng &r) {
                              fresh(r);
                              auto at = [](uint32_t off) { return (uint32_t)(uintptr_t)arena<void>(off); };
                              for (int k = 0; k < 0xc0; k++) arena<uint32_t>(0x1000)[k] = r.next();
                              af_handle(0x1800), af_handle(0x1900);
                              uint32_t *ref = arena<uint32_t>(0x1000 + 0x1bc), *head = arena<uint32_t>(0x1880), *after = arena<uint32_t>(0x1890);
                              ref[0] = (uint32_t)(uintptr_t)af_ref_table, ref[1] = at(0x1800);
                              ref[2] = at(0x1880), ref[3] = at(0x1890);   // (linked between the head and its other node)
                              head[1] = at(0x1000 + 0x1bc + 8), after[0] = at(0x1000 + 0x1bc + 8);
                              arena<uint32_t>(0x50)[0] = r.range(0, 4), arena<uint32_t>(0x50)[1] = 0, arena<uint32_t>(0x50)[2] = r.next();
                              for (uint32_t i = 0; i < 4; i++) {
                                  arena<uint32_t>(0x1400 + 0x80 * i)[0] = (uint32_t)(uintptr_t)af_object_table;
                                  arena<uint32_t>(0x60)[i] = r.range(0, 4) != 0;   // a type or none
                                  uint32_t *type = arena<uint32_t>(0x1600 + 0x40 * i);
                                  type[5] = at(0x1700 + 0x20 * i), type[6] = r.range(0, 4);   // its parts, its count (0 to 4: all four set)
                                  arena<uint32_t>(0x1700 + 0x20 * i - 4)[0] = r.range(0, 1) ? at(0x1a00 + 0x40 * (16 + i)) : 0;   // (the entry before the parts, read with -1: a part or none)
                                  arena<uint32_t>(0x1a00 + 0x40 * (16 + i))[4] = 8;
                                  for (uint32_t k = 0; k < 4; k++) {   // each part: none, or one whose offset is small, or one that makes the sum 0
                                      uint32_t kind = r.range(0, 5);
                                      uint32_t *part = arena<uint32_t>(0x1a00 + 0x40 * (4 * i + k));
                                      part[4] = kind == 4 ? 0u - at(0x1400 + 0x80 * i) : 4 * kind;
                                      arena<uint32_t>(0x1700 + 0x20 * i)[k] = kind == 0 ? 0 : at(0x1a00 + 0x40 * (4 * i + k));
                                  }
                              }
                              for (uint32_t k = 0; k < 16; k++) arena<uint32_t>(0x70)[k] = r.range(0, 5) == 0 ? 0 : r.range(1, 4);
                              *(volatile uint32_t *)(uintptr_t)0x12f4e4c = at(0x1380);
                              for (uint32_t g : {0x13cde74u, 0x13d0680u})   // (each copy's its own value, so the wrong one shows)
                                  *(volatile int32_t *)(uintptr_t)g = r.range(0, 5) == 0 ? -1 : (int32_t)r.range(0, 4);   // (-1 reads the entry before the parts: set below)
                          },
                          [](void *fn) {
                              bool ok = ((bool(__fastcall *)(void *))fn)(arena<void>(0x1000));
                              uint32_t h = (uint8_t)ok;
                              for (int k = 0; k < 0xc0; k++) h = h * 31 + af_at((void *)(uintptr_t)arena<uint32_t>(0x1000)[k]);
                              for (uint32_t a : {0x1880u, 0x1890u, 0x1980u, 0x1990u})
                                  for (uint32_t k = 0; k < 2; k++) h = h * 31 + af_at((void *)(uintptr_t)arena<uint32_t>(a)[k]);
                              return Result{h ^ arena<uint32_t>(0x50)[1] << 24};
                          }};
    attraction_first.iterations = 1000;
    attraction_first.prepare = [](const rewrite::Entry *) {
        for (int k = 0; k < 3; k++) af_saved[k] = *(volatile uint32_t *)(uintptr_t)af_globals[k];
        stubs_for_now({{0x4cce80, (void *)af_next}, {0x7aad50, (void *)af_first}, {0x7abef0, (void *)af_second}, {0x8055c0, (void *)af_release},
                       {0x805700, (void *)af_handle_for}, {0x4bf1f0, (void *)af_value}, {0x7cd220, (void *)af_step}});
    };
    attraction_first.finish = [](const rewrite::Entry *) {
        unstub_many();
        for (int k = 0; k < 3; k++) *(volatile uint32_t *)(uintptr_t)af_globals[k] = af_saved[k];
    };
    quick(attraction_first);
#endif
