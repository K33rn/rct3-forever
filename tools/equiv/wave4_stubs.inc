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
static uint32_t __thiscall w4_stub_ask(void *self, uint32_t arg) {
    record(0x4030, self); record(0x4030, (void *)(uintptr_t)arg);
    return arg ^ 0x111;
}
// mapping a key can replace the table at 0x800 + 0x34
static uint32_t __thiscall w4_stub_map(void *self, uint32_t arg) {
    record(0x4031, self); record(0x4031, (void *)(uintptr_t)arg);
    if (*arena<uint8_t>(0x84)) *(void **)arena<uint8_t>(0x834) = arena<void>(0x380);
    return arg + 1;
}
static void *__fastcall w4_stub_a(void *self) { record(0x4032, self); return arena<void>(0x500); }
// a value at 0x500.. holds its node at +4, which it can cut off from the rest
static void __fastcall w4_stub_value_v1(void **self) {
    record(0x4033, self);
    if (*arena<uint8_t>(0x84) == 1) ((void **)self[1])[0] = nullptr;
}
static void __stdcall w4_stub_release2(void *object) { record(0x4034, object); }
// the first virtual of a part at 0x300 can flip the flags at 0x800 + 0x10
static void __fastcall w4_stub_part_first(void *part) {
    record(0x4035, part);
    *arena<uint32_t>(0x810) ^= *arena<uint32_t>(0x84) & 2;
}
static uint32_t __fastcall w4_stub_check_flag(void *self) { record(0x4036, self); return *arena<uint32_t>(0x20); }
static void *__fastcall w4_stub_default(void *self) { record(0x4037, self); return arena<void>(0x700); }
static uint8_t __thiscall w4_stub_flagged(void *self, uint32_t arg) {
    record(0x4040, self); record(0x4040, (void *)(uintptr_t)arg);
    return *arena<uint8_t>(0x20);
}
// finds the key's value at 0x600 when the key is even
static uint32_t *__thiscall w4_stub_find(void *self, uint32_t key) {
    record(0x4041, self); record(0x4041, (void *)(uintptr_t)key);
    return key & 1 ? nullptr : arena<uint32_t>(0x600);
}
// an object of the queue takes itself off the front
static void *__thiscall w4_stub_dequeue(void *self, uint32_t flags) {
    record(0x4042, self); record(0x4042, (void *)(uintptr_t)flags);
    --*(int32_t *)0x1306bcc;
    ++*(int32_t *)0x1306be0;
    return self;
}
static void __fastcall w4b_stub_fill(void *out, uint32_t value) { record(0x4050, out); record(0x4050, (void *)(uintptr_t)value); }
static uint8_t __thiscall w4b_stub_take(void *self, uint32_t amount) {
    record(0x4051, self); record(0x4051, (void *)(uintptr_t)amount);
    return *arena<uint8_t>(0x20);
}
// finds the reference kept at 0x10
static void *__fastcall w4b_stub_get(uint32_t owner) { record(0x4052, (void *)(uintptr_t)owner); return arena<void>(0x10); }
static void *__fastcall w4b_stub_owner(void *self) { record(0x4053, self); return arena<void>(0x300); }
// a matrix from 0x600: written to the space given, or (odd 0x20) handed back from where it is
static void *__thiscall w4b_stub_matrix(void *self, void *space, uint32_t value) {
    record(0x4054, self); record(0x4054, (void *)(uintptr_t)value);
    if (*arena<uint32_t>(0x20) & 1) return arena<void>(0x600);
    memcpy(space, arena<void>(0x600), 0x40);
    return space;
}
static void *__fastcall w4b_stub_of_iface(void *iface) { record(0x4055, iface); return arena<void>(0x480); }
static void *__thiscall w4b_stub_item(void *self, uint32_t which) {
    record(0x4056, self); record(0x4056, (void *)(uintptr_t)which);
    return arena<void>(0x490);
}
// holds on the call the count at 0x20 runs out
static uint8_t __thiscall w4b_stub_check(void *self, uint32_t a, uint32_t b) {
    record(0x4057, self); record(0x4057, (void *)(uintptr_t)a); record(0x4057, (void *)(uintptr_t)b);
    return --*arena<int32_t>(0x20) == 0;
}
// a link holds something when it says so at +0x10
static void *__fastcall w4b_stub_held(uint8_t *link) { record(0x4058, link); return link[0x10] & 1 ? link + 0x20 : nullptr; }
// a join can move the cell at 0x400 (odd 0x20), to show the position is read afresh
static uint32_t __thiscall w4b_stub_join(void *self, void *cell, uint32_t neighbour, uint32_t side) {
    record(0x4059, self); record(0x4059, cell); record(0x4059, (void *)(uintptr_t)neighbour); record(0x4059, (void *)(uintptr_t)side);
    if (*arena<uint32_t>(0x20) & 1) { ((uint8_t *)cell)[0x88] ^= 1; ((uint8_t *)cell)[0x89] ^= 1; }
    return neighbour ^ side;
}
static uint32_t __fastcall w4b_stub_then(void *iface) { record(0x405a, iface); return (uint32_t)(uintptr_t)iface + 3; }
// only the low two bytes of each argument mean anything
static void __thiscall w4b_stub_table(void *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f, uint32_t g, uint32_t h,
                                      uint32_t i, uint32_t j, uint32_t k, uint32_t l, uint32_t m, uint32_t n, uint32_t o, uint32_t p) {
    record(0x405b, self);
    for (uint32_t v : {a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p}) record(0x405b, (void *)(uintptr_t)(v & 0xffff));
}
static void *__fastcall w4b_stub_live(void *self) { record(0x405c, self); return (void *)(uintptr_t)*arena<uint32_t>(0x20); }
static uint32_t __fastcall w4b_stub_path_a(void *self) { record(0x405d, self); return 0xa1; }
static uint32_t __fastcall w4b_stub_path_b(void *self) { record(0x405e, self); return 0xb2; }
static void *__fastcall w4b_stub_view(void *self) { record(0x405f, self); return arena<void>(0x700); }
static void *__fastcall w4b_stub_view_matrix(void *self) { record(0x4060, self); return arena<void>(0x600); }
// ---- third batch ----
static uint32_t w4c_target(const rewrite::Entry *e, uint32_t off) { uint32_t at = (uint32_t)(uintptr_t)e->at + off; return at + 4 + *(int32_t *)(uintptr_t)at; }
// no memory when the word at 0x20 is odd
static void *__cdecl w4c_stub_alloc(uint32_t size) { record(0x4c00, (void *)(uintptr_t)size); return *arena<uint32_t>(0x20) & 1 ? nullptr : arena<void>(0xc00); }
static uint32_t __thiscall w4c_stub_ctor1(void *self, uint32_t a) { record(0x4c01, self); record(0x4c01, (void *)(uintptr_t)a); return (uint32_t)(uintptr_t)self + 3; }
static uint32_t __fastcall w4c_stub_default(uint32_t cl) { record(0x4c02, (void *)(uintptr_t)(cl & 0xff)); return 0x1234; }
// a key object on the stack: filled whole, its address not recorded
static void *__thiscall w4c_stub_key(uint32_t *key, uint32_t a, uint32_t b) {
    record(0x4c03, (void *)(uintptr_t)a); record(0x4c03, (void *)(uintptr_t)b);
    for (int i = 0; i < 5; i++) key[i] = 0x5a5a0000 + i;
    return arena<void>(0xc40);
}
static void __fastcall w4c_stub_key_gone(uint32_t *key) { record(0x4c04, (void *)(uintptr_t)key[0]); record(0x4c04, (void *)(uintptr_t)key[4]); }
static uint32_t __fastcall w4c_stub_amount(void *self) { record(0x4c05, self); return (uint32_t)(uintptr_t)self & 0xfff; }
static uint32_t __fastcall w4c_stub_amount2(void *self) { record(0x4c06, self); return ((uint32_t)(uintptr_t)self & 0xfff) * 3; }
static void __thiscall w4c_stub_place(void *self, uint32_t a, const uint32_t *m) {
    record(0x4c07, self); record(0x4c07, (void *)(uintptr_t)a);
    for (int i = 0; i < 16; i++) record(0x4c07, (void *)(uintptr_t)m[i]);
}
static uint32_t __fastcall w4c_stub_then(void *self, uint32_t a, uint32_t b, uint32_t c) { record(0x4c08, self); record(0x4c08, (void *)(uintptr_t)a); record(0x4c08, (void *)(uintptr_t)b); record(0x4c08, (void *)(uintptr_t)c); return a ^ b ^ c; }
static uint32_t __thiscall w4c_stub_job(uint32_t *job, void *self, uint32_t argument) {
    record(0x4c09, self); record(0x4c09, (void *)(uintptr_t)argument);
    for (int i : {0, 1, 3, 0x1010 / 4}) record(0x4c09, (void *)(uintptr_t)job[i]);
    for (int i = 0x1114 / 4; i <= 0x1138 / 4; i++) record(0x4c09, (void *)(uintptr_t)job[i]);
    return argument + 1;
}
struct W4cWords { uint32_t words[13]; };
// ---- fourth batch ----
typedef uint8_t *W4dP;
static uint32_t __thiscall w4d_stub_pack(uint32_t *packed, uint32_t *value, W4dP u1, W4dP u2, W4dP u3, W4dP u4, W4dP u5, W4dP u6, W4dP u7, W4dP u8, W4dP u9, W4dP u10,
                                         W4dP u11, W4dP u12, W4dP u13, W4dP u14, W4dP u15) {
    record(0x4d00, (void *)(uintptr_t)*value);
    for (W4dP u : {u1, u2, u3, u4, u5, u6, u7, u8, u9, u10, u11, u12, u13, u14, u15}) *u = 0x77;
    for (int i = 0; i < 3; i++) packed[i] = *value ^ (0x1111 * i);
    return *value * 3 + 1;
}
static uint32_t __thiscall w4d_stub_render(void *self, uint32_t key, uint32_t made) { record(0x4d01, self); record(0x4d01, (void *)(uintptr_t)key); record(0x4d01, (void *)(uintptr_t)made); return key ^ made; }
static void *__fastcall w4d_stub_class(void *object) { record(0x4d02, object); return arena<void>(0x2000); }
static uint32_t __fastcall w4d_stub_convert(uint32_t side, uint32_t end, uint32_t *slot, uint32_t mode) {
    side &= 0xff; end &= 0xff;
    record(0x4d03, (void *)(uintptr_t)side); record(0x4d03, (void *)(uintptr_t)end); record(0x4d03, (void *)(uintptr_t)*slot); record(0x4d03, (void *)(uintptr_t)mode);
    *slot = (*slot + (side & 1) + (end & 1)) & 1;
    return ((*arena<uint32_t>(0x20) >> ((side + end) & 7)) & 3) != 0;
}
static void __stdcall w4d_stub_vector_dtor(void *p, uint32_t size, uint32_t n, void *dtor) { record(0x4d04, p); record(0x4d04, (void *)(uintptr_t)size); record(0x4d04, (void *)(uintptr_t)n); record(0x4d04, dtor); }
struct W4dWords14 { uint32_t words[14]; };
static void __thiscall w4d_stub_grow(uint32_t *list, int32_t n) { record(0x4d05, (void *)(uintptr_t)n); record(0x4d05, (void *)(uintptr_t)list[2]); list[0] = (uint32_t)(uintptr_t)arena<void>(0x2400); list[1] = (uint32_t)n; }
