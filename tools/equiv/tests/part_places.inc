// Window::PlacePart (forever/src/rewrite/part_places.cpp). The window at arena 0x100; its parts at 0x300 (+0xD4),
// 0x400 (+0xF4) and 0x500 (+0x11C; the size call sometimes switches it to 0x600) with random floats, NaN and
// infinity among them; the size at 0x700.
#if EQUIV_PART == 1
static void __fastcall pp_layout(void *self) { dev_note(310, (uint32_t)(uintptr_t)self); }
static void __thiscall pp_rect(void *part, const uint32_t *rect) {
    dev_note(311, (uint32_t)(uintptr_t)part);
    for (int k = 0; k < 4; k++) dev_note(311, rect[k]);
}
static const void *__thiscall pp_size(void *part, void *place) {
    dev_note(312, (uint32_t)(uintptr_t)part); dev_note(312, place != nullptr);   // (the place is on the caller's stack: where differs by frame)
    if (dev_next() % 3 == 0) *(void **)(arena<uint8_t>(0x100) + 0x11c) = arena<void>(0x600), *(void **)(arena<uint8_t>(0x100) + 0xf8) = arena<void>(0x600);
    return arena<void>(0x700);
}
static void *pp_table[0xf4 / 4 + 1];   // (slot 0x20 for 0x6DBC90's part, 0xF4 for the others)
static float pp_value(Rng &r) {
    switch (r.range(0, 9)) {
    case 0: return NAN;
    case 1: return INFINITY;
    case 2: return r.any_float();
    default: return r.real(-600, 600);
    }
}
#else
    Test part_places{0x6dab30,
                     [=](Rng &r) {
                         fresh(r);
                         arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next();
                         pp_table[0xf4 / 4] = (void *)pp_size, pp_table[0x20 / 4] = (void *)pp_size;
                         uint8_t *w = arena<uint8_t>(0x100);
                         *(void **)(w + 0xd4) = arena<void>(0x300), *(void **)(w + 0xf4) = arena<void>(0x400), *(void **)(w + 0x11c) = arena<void>(0x500), *(void **)(w + 0xf8) = arena<void>(0x500);
                         for (uint32_t base : {0x300u, 0x400u, 0x500u, 0x600u}) {
                             *(void **)arena<void>(base) = pp_table;
                             for (uint32_t off = 0x7c; off <= 0x88; off += 4) *(float *)(arena<uint8_t>(base) + off) = pp_value(r);
                         }
                         arena<float>(0x700)[0] = pp_value(r), arena<float>(0x700)[1] = pp_value(r);
                     },
                     [](void *fn) {
                         ((void(__fastcall *)(void *))fn)(arena<void>(0x100));
                         return Result{*arena<uint32_t>(0x20)};
                     }};
    part_places.iterations = 2000;
    part_places.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x6d4870, (void *)pp_layout}, {0x6afaf0, (void *)pp_rect}}); };
    part_places.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(part_places);
#endif
