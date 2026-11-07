// Stand-ins for the wave 4 tests.
static uint32_t __thiscall w4_stub_flush(uint32_t *self, uint32_t *data, uint32_t n) {
    record(0x4001, self); record(0x4001, data); record(0x4001, (void *)(uintptr_t)n);
    record(0x4001, (void *)(uintptr_t)data[0]); record(0x4001, (void *)(uintptr_t)data[n - 1]);
    data[0] = 0x55; self[0x41] = 9;
    return 0xabc0 + n;
}
static uint32_t __thiscall w4_stub_field_size(void *self, uint8_t *data) {
    record(0x4002, self); record(0x4002, data);
    return ((uint32_t)(uintptr_t)self >> 4 & 7) + ((uint32_t)(uintptr_t)data & 15) + 1;
}
// a child passes the check by its first byte; its second lowers the count of children at 0x800 + 0xd0
static uint8_t __fastcall w4_stub_child(uint8_t *child) {
    record(0x4003, child);
    *(int32_t *)arena<uint8_t>(0x8d0) -= child[1];
    return child[0];
}
static uint32_t __fastcall w4_stub_step(void *self) { record(0x4004, self); return 0x7700 + *arena<uint8_t>(0x20); }
template <int N> static void __thiscall w4_stub_slot(void *self, uint32_t a, uint32_t b) {
    record(0x4010 + N, self); record(0x4010 + N, (void *)(uintptr_t)a); record(0x4010 + N, (void *)(uintptr_t)b);
}
static uint32_t __thiscall w4_stub_lookup(void *table, uint32_t *key) {
    record(0x4005, table); record(0x4005, key); record(0x4005, (void *)(uintptr_t)*key);
    return *key ^ 0x5a5a;
}
static void __stdcall w4_stub_pay(void *device) { record(0x4006, device); record(0x4006, (void *)(uintptr_t)arena<uint16_t>(0x80e)[0]); }
// the flag word sits 0xa8 below the interface at 0x900: the part changes it before answering
static uint8_t __fastcall w4_stub_on(uint8_t *part) {
    record(0x4007, part);
    *(uint32_t *)arena<uint8_t>(0x858) ^= *(uint32_t *)(part + 4);
    return part[8];
}
static void __fastcall w4_stub_touch(uint8_t *object) {
    record(0x4008, object);
    // touching the first object can leave the second handle without one
    if (*arena<uint8_t>(0x84)) *(void **)arena<uint8_t>(0x380 + 0xc) = nullptr;
}
static void __thiscall w4_stub_query(void *self, uint32_t *q) {
    record(0x4009, self); record(0x4009, (void *)(uintptr_t)q[0]); record(0x4009, (void *)(uintptr_t)(q[1] & 0xff));
    record(0x4009, (void *)(uintptr_t)q[2]); record(0x4009, (void *)(uintptr_t)q[3]);
    *(uint8_t *)(q + 1) = (uint8_t)(q[3] & 1);
    q[2] = q[3] * 3 + 1;
}
