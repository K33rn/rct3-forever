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
static uint32_t __fastcall w4_stub_kept(void *self) { record(0x4020, self); return 0x4242 + *arena<uint8_t>(0x20); }
static void __stdcall w4_stub_construct(void *items, uint32_t size, int32_t count, uint32_t ctor) {
    record(0x4021, items); record(0x4021, (void *)(uintptr_t)size); record(0x4021, (void *)(uintptr_t)count); record(0x4021, (void *)(uintptr_t)ctor);
}
static void *__cdecl w4_stub_alloc_maybe(uint32_t size) {
    record(0x4022, (void *)(uintptr_t)size);
    return *arena<uint32_t>(0x28) % 4 ? arena<void>(0x3000) : nullptr;
}
// a value at 0x500.. holds its node at +4: deleting it takes the node out of the list
static void *__thiscall w4_stub_delete_value(void **self, uint32_t flags) {
    record(0x4023, self); record(0x4023, (void *)(uintptr_t)flags);
    void **node = (void **)self[1];
    ((void **)node[1])[0] = node[0];
    ((void **)node[0])[1] = node[1];
    node[0] = node[1] = nullptr;   // (the node may have held the walk's cursor, on the stack)
    return self;
}
static void __thiscall w4_stub_owner(void *owner, void *member) { record(0x4024, owner); record(0x4024, member); }
static void __thiscall w4_stub_visit(void *visitor, uint32_t v) { record(0x4025, visitor); record(0x4025, (void *)(uintptr_t)v); }
static void __thiscall w4_stub_four(void *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    record(0x4026, self); record(0x4026, (void *)(uintptr_t)(a + 2 * b + 4 * c + 8 * d));
}
static void __thiscall w4_stub_grow(int32_t *self, int32_t capacity) {
    record(0x4027, self); record(0x4027, (void *)(uintptr_t)capacity);
    self[3] = capacity;
}
static uint8_t *__fastcall w4_stub_record(void *object) { record(0x4028, object); return arena<uint8_t>(0x510); }
static uint32_t __fastcall w4_stub_first(void *self) { record(0x4029, self); return 0x1234; }
static uint32_t __fastcall w4_stub_second(void *self) { record(0x402a, self); return 0x5678; }
static uint32_t __fastcall w4_stub_check(uint32_t a, uint32_t b, uint32_t c) {
    record(0x402b, (void *)(uintptr_t)a); record(0x402b, (void *)(uintptr_t)b); record(0x402b, (void *)(uintptr_t)c);
    return a * 3 + b + c;
}
// room made elsewhere: the array's data moves to 0xa00
static void __thiscall w4_stub_events_room(uint32_t *array, int32_t n) {
    record(0x402c, array); record(0x402c, (void *)(uintptr_t)n);
    array[0] = (uint32_t)(uintptr_t)arena<void>(0xa00);
    array[1] = n;
}
