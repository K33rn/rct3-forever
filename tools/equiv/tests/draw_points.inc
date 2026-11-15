// Device::DrawPoints (forever/src/rewrite/draw_points.cpp). The device at arena 0x100; the vertex buffer (0x600,
// locked memory at 0x3000) and the primitive (0x700) are the stand-ins'; points at 0x4000.
#if EQUIV_PART == 1
static uint32_t g_dp_saved[2];
static void __thiscall dp_matrix(void *device, uint32_t zero, const uint32_t *m) {
    dev_note(200, (uint32_t)(uintptr_t)device); dev_note(200, zero);
    for (int k = 0; k < 16; k++) dev_note(200, m[k]);
}
static void __thiscall dp_settings(void *device, uint32_t a, uint32_t b) { dev_note(201, (uint32_t)(uintptr_t)device); dev_note(201, a); dev_note(201, b); }
static void __thiscall dp_draw(void *device, uint8_t *primitive) { dev_note(202, (uint32_t)(uintptr_t)primitive); dev_note(202, *(uint16_t *)(primitive + 4)); }
static void *dp_device_table[0xb8 / 4 + 1];
static void __thiscall dp_buffer_gone(uint8_t *buffer, uint32_t flag) { dev_note(203, flag); }
static void *__thiscall dp_lock(uint8_t *buffer, uint32_t a, int32_t count, uint32_t b) { dev_note(204, a); dev_note(204, (uint32_t)count); dev_note(204, b); return arena<void>(0x3000); }
static void __thiscall dp_unlock(uint8_t *buffer) { dev_note(205, *(uint16_t *)(buffer + 4)); }
static void *dp_buffer_table[4] = {(void *)dp_buffer_gone, nullptr, (void *)dp_lock, (void *)dp_unlock};
static void *__fastcall dp_buffer(int32_t count, uint32_t format, uint32_t three) {
    dev_note(206, (uint32_t)count); dev_note(206, format); dev_note(206, three);
    *(void **)arena<void>(0x600) = dp_buffer_table;
    return arena<void>(0x600);
}
static void __thiscall dp_primitive_gone(uint8_t *primitive) { dev_note(207, (uint32_t)(uintptr_t)primitive); }
static void *dp_primitive_table[2] = {nullptr, (void *)dp_primitive_gone};
static void *__fastcall dp_primitive(uint32_t kind, uint8_t *buffer, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f, uint32_t g, uint32_t h) {
    dev_note(208, kind); dev_note(208, (uint32_t)(uintptr_t)buffer);
    for (uint32_t v : {a, b, c, d, e, f, g, h}) dev_note(208, v);
    *(void **)arena<void>(0x700) = dp_primitive_table;
    return arena<void>(0x700);
}
#else
    Test draw_points{0xbf7770,
                     [=](Rng &r) {
                         fresh(r);
                         dp_device_table[0xb8 / 4] = (void *)dp_matrix, dp_device_table[0xac / 4] = (void *)dp_settings, dp_device_table[0x44 / 4] = (void *)dp_draw;
                         *(void **)arena<void>(0x100) = dp_device_table;
                         *(uint16_t *)(arena<uint8_t>(0x600) + 4) = (uint16_t)r.range(1, 2);
                         *(uint16_t *)(arena<uint8_t>(0x700) + 4) = (uint16_t)r.range(1, 2);
                         for (int k = 0; k < 0x200; k++) {
                             const uint32_t odd[] = {0x7fa00001, 0xff800001, 0x7fc00000, 0x80000000, 0x00000001};
                             arena<uint32_t>(0x4000)[k] = r.range(0, 5) ? r.next() : odd[r.range(0, 4)];
                             arena<uint32_t>(0x3000)[k] = r.next();
                         }
                         arena<int32_t>(0x40)[0] = r.range(0, 4) ? r.range(0, 30) : r.range(-3, 0);
                         arena<uint32_t>(0x40)[1] = r.next();
                         *(uint32_t *)(uintptr_t)0x13d28a0 = r.next(), *(uint32_t *)(uintptr_t)0x13d0e34 = r.next();
                         arena<uint32_t>(0x20)[0] = 0;
                     },
                     [](void *fn) {
                         ((void(__fastcall *)(void *, int32_t, const void *, uint32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<void>(0x4000),
                                                                                           arena<uint32_t>(0x40)[1]);
                         return Result{*arena<uint32_t>(0x20)};
                     }};
    draw_points.iterations = 1000;
    draw_points.prepare = [](const rewrite::Entry *) {
        g_dp_saved[0] = *(uint32_t *)(uintptr_t)0x13d28a0, g_dp_saved[1] = *(uint32_t *)(uintptr_t)0x13d0e34;
        stubs_for_now({{0xbec340, (void *)dp_buffer}, {0xbe8fe0, (void *)dp_primitive}});
    };
    draw_points.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)(uintptr_t)0x13d28a0 = g_dp_saved[0], *(uint32_t *)(uintptr_t)0x13d0e34 = g_dp_saved[1];
    };
    quick(draw_points);
#endif
