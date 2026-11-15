// Window::LineWalk (forever/src/rewrite/line_walks.cpp). From and to at arena 0x60 / 0x68: small numbers (whole,
// halves, random fractions, +/-0, denormals; the same point sometimes), so the walk ends (with a NaN or a huge
// distance the game's walk would not); the step's stand-in notes the object and the point (its bits).
#if EQUIV_PART == 1
static void __fastcall lw_step(void *target, void *, const uint32_t *point) {
    dev_note(570, (uint32_t)((uintptr_t)target - (uintptr_t)arena<void>(0)));
    dev_note(570, point[0]);
    dev_note(570, point[1]);
}
static uint32_t lw_value(Rng &r) {
    float f;
    switch (r.range(0, 7)) {
    case 0: return r.range(0, 1) ? 0x80000000u : 0;
    case 1: return r.next() & 0x807fffffu;   // a denormal
    case 2: f = (float)r.range(-30, 30); break;
    case 3: f = (float)r.range(-60, 60) * 0.5f; break;
    default: f = r.real(-30, 30); break;
    }
    uint32_t bits;
    memcpy(&bits, &f, 4);
    return bits;
}
#else
    Test line_walks{0x65ec80,
                    [=](Rng &r) {
                        fresh(r);
                        arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next();
                        for (int k = 0; k < 4; k++) arena<uint32_t>(0x60)[k] = lw_value(r);
                        if (r.range(0, 9) == 0) arena<uint32_t>(0x68)[0] = arena<uint32_t>(0x60)[0], arena<uint32_t>(0x68)[1] = arena<uint32_t>(0x60)[1];
                    },
                    [](void *fn) {
                        ((void(__fastcall *)(void *, void *, void *))fn)(arena<void>(0x60), arena<void>(0x68), arena<void>(0x700));
                        return Result{*arena<uint32_t>(0x20)};
                    }};
    line_walks.iterations = 3000;
    line_walks.prepare = [](const rewrite::Entry *e) { stubs_for_now({{e->at == 0x65ec80 ? 0x65eb50u : 0x714910u, (void *)lw_step}}); };
    line_walks.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(line_walks);
#endif
