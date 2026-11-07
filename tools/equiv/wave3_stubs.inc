// Stand-ins for the wave 3 tests.
static void *__cdecl w3_stub_registry() { record(0x440fd0, nullptr); return arena<void>(0x400); }
static void *__cdecl w3_stub_manager() { record(0x477000, nullptr); return arena<void>(0x600); }
static uint32_t __fastcall w3_stub_then(void *self) { record(0x731e10, self); return 0x1234; }
static void __thiscall w3_stub_read(void *self, uint8_t *into, int32_t size) {
    record(0xbe5020, self);
    record(0xbe5020, (void *)(uintptr_t)size);
    for (int32_t i = 0; i < size; i++) into[i] = arena<uint8_t>(0x20)[i];
}
// 0 (a match) when the word there is zero; a null interface's field is a small address, which never matches
static int32_t __thiscall w3_stub_compare(uint32_t *self, void *name) {
    record(0xbce970, self);
    record(0xbce970, name);
    return (uintptr_t)self < 0x1000 ? 1 : (int32_t)*self;
}
static int32_t __fastcall w3_stub_tell(void *self) { record(0xbe40b0, self); return arena<int32_t>(0x20)[0]; }
static uint32_t __thiscall w3_stub_seek(void *self, int32_t at) { record(0xbe4090, self); record(0xbe4090, (void *)(uintptr_t)at); return (uint32_t)at ^ 0x5a5a; }
static uint32_t __thiscall w3_stub_visit(void *self, uint32_t *visitor) {
    record(0x6ae7c0, self);
    record(0x6ae7c0, (void *)(uintptr_t)visitor[0]);
    record(0x6ae7c0, (void *)(uintptr_t)visitor[1]);
    return 0x77;
}
static uint32_t __fastcall w3_stub_eight(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f, uint32_t g, void *h) {
    for (uint32_t v : {a, b, c, d, e, f, g}) record(0x8275d0, (void *)(uintptr_t)v);
    record(0x8275d0, h);
    return a ^ c;
}
// a value that takes its node off its list: {table, the list's field, the next node}
static void *__thiscall w3_stub_drop(uint32_t *self, uint32_t flags) {
    record(0xbfa860, self);
    record(0xbfa860, (void *)(uintptr_t)flags);
    *(uint32_t *)(uintptr_t)self[1] = self[2];
    return self;
}
static void __fastcall w3_stub_each(int32_t i, uint8_t flag) { record(0x47c3f0, (void *)(uintptr_t)i); record(0x47c3f0, (void *)(uintptr_t)flag); }
static int32_t __stdcall w3_com_get(void *self, void **into) { record(0xdff001, self); *into = arena<void>(0x400); return arena<int32_t>(0x20)[0]; }
static int32_t __stdcall w3_com_query(void *self, uint32_t iid, void **into) {
    record(0xdff002, self);
    record(0xdff002, (void *)(uintptr_t)iid);
    *into = arena<void>(0x500);
    return arena<int32_t>(0x20)[1];
}
static uint32_t __stdcall w3_com_release(void *self) { record(0xdff003, self); return 0; }
static void __thiscall w3_stub_key(uint32_t *key, uint32_t id) { record(0xbf47b0, (void *)(uintptr_t)id); key[0] = 0xdead; key[1] = id ^ 0x1111; }
static void *__thiscall w3_stub_lookup(void *self, uint32_t *key) {
    record(0xbf5b60, self);
    record(0xbf5b60, (void *)(uintptr_t)key[0]);
    record(0xbf5b60, (void *)(uintptr_t)key[1]);
    return arena<uint32_t>(0x20)[0] & 3 ? arena<void>(0x360) : nullptr;
}
static void __fastcall w3_stub_report(uint32_t file, uint32_t line) { record(0xbec8a0, (void *)(uintptr_t)file); record(0xbec8a0, (void *)(uintptr_t)line); }
static void *__thiscall w3_stub_item(void *self, uint32_t key, uint32_t *slot) {
    record(0x8fd720, self);
    record(0x8fd720, (void *)(uintptr_t)key);
    record(0x8fd720, (void *)(uintptr_t)*slot);
    return arena<uint32_t>(0x20)[0] & 3 ? arena<void>(0x400) : nullptr;
}
static uint32_t __thiscall w3_stub_four(void *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    record(0x901810, self);
    for (uint32_t v : {a, b, c, d}) record(0x901810, (void *)(uintptr_t)v);
    return a + b;
}
static uint32_t __thiscall w3_stub_four_ref(void *self, uint32_t *a, uint32_t b, uint32_t c, uint32_t d) {
    record(0x7b5420, self);
    for (uint32_t v : {*a, b, c, d}) record(0x7b5420, (void *)(uintptr_t)v);
    return b ^ c;
}
static void *__fastcall w3_stub_next(void **self) { record(0x4bf970, self); return *self; }
static void *__cdecl w3_stub_alloc_maybe(uint32_t size) { record(0xbcdfc0, (void *)(uintptr_t)size); return arena<uint32_t>(0x20)[0] & 3 ? arena<void>(0x2000) : nullptr; }
static void __thiscall w3_stub_three(void *self, uint32_t a, uint32_t b, uint32_t c) {
    record(0x6aae60, self);
    for (uint32_t v : {a, b, c}) record(0x6aae60, (void *)(uintptr_t)v);
}
static uint8_t __thiscall w3_stub_take(void *self, uint32_t amount) {
    record(0x72f820, self);
    record(0x72f820, (void *)(uintptr_t)amount);
    return (uint8_t)(arena<uint32_t>(0x20)[0] & 3 ? 0x7 : 0);
}
static void __fastcall w3_stub_fill(void *out, uint32_t a) { record(0x442570, out); record(0x442570, (void *)(uintptr_t)a); }
static int __cdecl w3_stub_compare_name(void *name, void *other) {
    record(0xde22df, name);
    record(0xde22df, other);
    return arena<int32_t>(0x20)[0]-- == 0 ? 0 : -1;
}
static uint32_t __thiscall w3_stub_hash(void *self, uint32_t key) { record(0xbdbf70, self); record(0xbdbf70, (void *)(uintptr_t)key); return arena<uint32_t>(0x20)[0]; }
static uint8_t __thiscall w3_stub_same(void *self, uint32_t key, uint32_t *item) {
    record(0xbdbfd0, self);
    record(0xbdbfd0, (void *)(uintptr_t)*item);
    return *item == key ? 3 : 0;
}

// ---- second pass ----
static int32_t w3b_count_at;
static void __fastcall w3b_stub_each(void *item) {
    record(0x7b01, item);
    if (arena<int32_t>(0x20)[0]-- == 0) *(int32_t *)(arena<uint8_t>(0x1000) + w3b_count_at) -= 1;
}
static void __thiscall w3b_stub_notice(void *self, uint32_t *notice) { record(0x7b02, self); record(0x7b02, (void *)(uintptr_t)notice[0]); record(0x7b02, (void *)(uintptr_t)notice[1]); }
static void __thiscall w3b_stub_build2(uint8_t *message, uint32_t value, uint8_t *flag) {
    record(0x7b03, (void *)(uintptr_t)value);
    record(0x7b03, (void *)(uintptr_t)*flag);
    for (int i = 0; i < 0x64; i++) message[i] = (uint8_t)(value * 7 + i * 13);
}
static void w3b_note_bytes(uint32_t id, void *self, const uint8_t *message, int size) {
    uint32_t sum = 0;
    for (int i = 0; i < size; i++) sum = sum * 31 + message[i];
    record(id, self);
    record(id, (void *)(uintptr_t)sum);
}
static void __thiscall w3b_stub_send64(void *self, uint8_t *message) { w3b_note_bytes(0x7b04, self, message, 0x64); }
static void __thiscall w3b_stub_build3(uint8_t *message, uint32_t a, uint32_t b, uint32_t c) {
    for (int i = 0; i < 0x1c; i++) message[i] = (uint8_t)(a * 3 + b * 5 + c * 11 + i * 13 + (a >> 8) + (b >> 16));
}
static void __thiscall w3b_stub_send1c(void *self, uint8_t *message) { w3b_note_bytes(0x7b05, self, message, 0x1c); }
static void *__cdecl w3b_stub_registry() { return arena<void>(0x400); }
static uint32_t __thiscall w3b_stub_handler(void *self, void *object, uint32_t flag) {
    record(0x7b06, self); record(0x7b06, object); record(0x7b06, (void *)(uintptr_t)flag);
    return arena<uint32_t>(0x24)[0];
}
static int32_t __stdcall w3b_stub_time(void *self, uint32_t *time) {
    record(0x7b07, self);
    time[0] = arena<uint32_t>(0x28)[0];
    time[1] = arena<uint32_t>(0x2c)[0];
    return arena<int32_t>(0x24)[0];
}
static int __cdecl w3b_stub_sprintf(char *out, const char *format, uint32_t value) {
    record(0x7b08, (void *)format);
    record(0x7b08, (void *)(uintptr_t)value);
    int size = (int)(value % 30);
    for (int i = 0; i < size; i++) out[i] = (char)('a' + (value + i) % 26);
    out[size] = 0;
    return size;
}
static char *__cdecl w3b_stub_strncpy(char *out, const char *text, uint32_t size) {
    record(0x7b09, out);
    record(0x7b09, (void *)(uintptr_t)size);
    uint32_t i = 0;
    for (; i < size && text[i]; i++) out[i] = text[i];
    for (; i < size; i++) out[i] = 0;
    return out;
}

static void __fastcall w3b_stub_slot_a(void *self) { record(0x7b10, self); }
static void __fastcall w3b_stub_slot_b(void *self) { record(0x7b11, self); }
static uint32_t __stdcall w3b_stub_com_release(void *self) { record(0x7b12, self); return 0; }
static int32_t __cdecl w3b_stub_query(uint32_t handle, uint32_t *status) {
    record(0x7b13, (void *)(uintptr_t)handle);
    record(0x7b13, (void *)(uintptr_t)*status);
    *status = arena<uint32_t>(0x28)[0];
    return arena<int32_t>(0x24)[0];
}
static void *__thiscall w3b_stub_table(void *self, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d, uint32_t *e, uint32_t *f, uint32_t *g, uint32_t *h,
                                       uint32_t *i, uint32_t *j, uint32_t *k, uint32_t *l, uint32_t *m, uint32_t *n, uint32_t *o, uint32_t *p) {
    uint32_t sum = 0;
    for (uint32_t *v : {a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p}) sum = sum * 31 + *v;
    record(0x7b14, (void *)(uintptr_t)sum);
    memset(self, 0x5a, 0x18);
    return arena<void>(0x500);
}
static uint32_t __thiscall w3b_stub_device(void *self, uint32_t mesh, void *built) {
    record(0x7b15, self); record(0x7b15, (void *)(uintptr_t)mesh); record(0x7b15, built);
    return arena<uint32_t>(0x24)[0];
}
static uint32_t __thiscall w3b_stub_act(void *self, int32_t slot, uint32_t how) {
    record(0x7b16, self); record(0x7b16, (void *)(uintptr_t)slot); record(0x7b16, (void *)(uintptr_t)how);
    return arena<uint32_t>(0x24)[0];
}
static void __thiscall w3b_stub_grow(void **holder, uint32_t size) {
    record(0x7b17, holder);
    record(0x7b17, (void *)(uintptr_t)size);
    if (arena<uint32_t>(0x28)[0] & 1) *holder = arena<void>(0x500);
}
static uint32_t __cdecl w3b_stub_use(uint8_t *text) {
    record(0x7b18, text);
    record(0x7b18, (void *)(uintptr_t)text[0]);
    return arena<uint32_t>(0x24)[0];
}
