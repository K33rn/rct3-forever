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

static uint32_t __fastcall w3b_stub_try(uint32_t kind, uint32_t which, uint32_t a, uint32_t b) {
    record(0x7b20, (void *)(uintptr_t)kind); record(0x7b20, (void *)(uintptr_t)(which & 0xff));
    record(0x7b20, (void *)(uintptr_t)a); record(0x7b20, (void *)(uintptr_t)b);
    return arena<int32_t>(0x28)[0]-- == 0 ? arena<uint32_t>(0x24)[0] : 0;
}
static uint32_t __thiscall w3b_stub_run(void *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f, uint32_t g, uint32_t h, uint32_t i) {
    record(0x7b21, self);
    for (uint32_t v : {a, b, c, d, e, f, g, h, i}) record(0x7b21, (void *)(uintptr_t)v);
    return arena<uint32_t>(0x24)[0];
}
static void *__fastcall w3b_stub_message(uint32_t kind, uint32_t low) {
    record(0x7b22, (void *)(uintptr_t)kind);
    record(0x7b22, (void *)(uintptr_t)(low & 0xff));
    return arena<void>(0x300);
}
static void *__fastcall w3b_stub_queue(void *self) { record(0x7b23, self); return arena<void>(0x340); }
static void *__cdecl w3b_stub_models() { return arena<void>(0x700); }
static void *__thiscall w3b_stub_model(void *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    record(0x7b24, self);
    for (uint32_t v : {a, b, c, d}) record(0x7b24, (void *)(uintptr_t)v);
    return arena<void>(0x720);
}
static void __thiscall w3b_stub_draw(void *self, void *item, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    record(0x7b25, self); record(0x7b25, item);
    for (uint32_t v : {a, b, c, d}) record(0x7b25, (void *)(uintptr_t)v);
}
static void __thiscall w3b_stub_state(void *self, uint32_t a, uint32_t b) { record(0x7b26, self); record(0x7b26, (void *)(uintptr_t)a); record(0x7b26, (void *)(uintptr_t)b); }

static uint32_t __fastcall w3b_stub_row(uint8_t *row) { record(0x7b30, row); return 0xabcd00 | row[0]; }

static void __cdecl w3c_stub_free(void *block) { record(0xbce000, block); }
static void __cdecl w3c_stub_free_aligned(void *block) { record(0xbce010, block); }
static void __thiscall w3c_stub_delete0(void *self, uint32_t flag) { record(0x7c00, self); record(0x7c00, (void *)(uintptr_t)flag); }
static void __thiscall w3c_stub_delete1(void *self, uint32_t flag) { record(0x7c01, self); record(0x7c01, (void *)(uintptr_t)flag); }
static void *w3c_delete_table[2] = {(void *)w3c_stub_delete0, (void *)w3c_stub_delete1};
static void __stdcall w3c_stub_each(void *first, uint32_t size, uint32_t count, void *fn) {
    record(0x7c02, first);
    record(0x7c02, (void *)(uintptr_t)size);
    record(0x7c02, (void *)(uintptr_t)count);
    record(0x7c02, fn);
}
static void __fastcall w3c_stub_base_a(void *self) { record(0x7c03, self); }
static void __fastcall w3c_stub_base_b(void *self) { record(0x7c04, self); }
static void __fastcall w3c_stub_base_c(void *self) { record(0x7c05, self); }
// the base of Selected::Dtor shows what the global held by then
static void __fastcall w3c_stub_base_selected(void *self) { record(0x7c06, self); record(0x7c06, *(void **)0x10eeab0); }
// leaves the list as a real entry's object would
static void __thiscall w3c_stub_entry_gone(void *self, uint32_t flag) {
    record(0x7c07, self);
    record(0x7c07, (void *)(uintptr_t)flag);
    arena<uint32_t>(0x800)[2]--;
}
static void *w3c_gone_table[1] = {(void *)w3c_stub_entry_gone};
static void __thiscall w3c_stub_bind(void *self, uint32_t *key, uint32_t **made) {
    record(0x7fbf50, self);
    record(0x7fbf50, (void *)(uintptr_t)*key);
    record(0x7fbf50, *made);
    record(0x7fbf50, (void *)(uintptr_t)(*made)[0]);
    record(0x7fbf50, (void *)(uintptr_t)(*made)[1]);
}
static void *__thiscall w3c_stub_ctor2(uint8_t *self, uint32_t a, uint32_t b) {
    record(0x7c09, self);
    for (uint32_t v : {a, b}) record(0x7c09, (void *)(uintptr_t)v);
    return self + 2;
}
static void *__thiscall w3c_stub_ctor3(uint8_t *self, uint32_t a, uint32_t b, uint32_t c) {
    record(0x7c0a, self);
    for (uint32_t v : {a, b, c}) record(0x7c0a, (void *)(uintptr_t)v);
    return self + 3;
}
static void *__thiscall w3c_stub_ctor4(uint8_t *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    record(0x7c0b, self);
    for (uint32_t v : {a, b, c, d}) record(0x7c0b, (void *)(uintptr_t)v);
    return self + 4;
}
// Single::Get: registers itself in the copy's global, or sometimes doesn't
static void **w3c_single_global;
static void __thiscall w3c_stub_single(void *self, uint32_t a) {
    record(0x7c0c, self);
    record(0x7c0c, (void *)(uintptr_t)a);
    if (arena<uint32_t>(0x24)[0] & 1) *w3c_single_global = self;
}
static void __stdcall w3c_stub_construct(void *first, uint32_t size, uint32_t count, void *fn) {
    record(0x402b90, first);
    record(0x402b90, (void *)(uintptr_t)size);
    record(0x402b90, (void *)(uintptr_t)count);
    record(0x402b90, fn);
}

// ---- wave 3, part d ----
static uint8_t __fastcall w3d_stub_assert(uint32_t file, uint32_t line, uint32_t a, uint32_t b) {
    for (uint32_t v : {file, line, a, b}) record(0x68f640, (void *)(uintptr_t)v);
    return 0;
}
static uint32_t __thiscall w3d_stub_handler2(void *self, uint32_t a, uint32_t b) {
    record(0x7d01, self);
    record(0x7d01, (void *)(uintptr_t)a);
    record(0x7d01, (void *)(uintptr_t)b);
    return (uint32_t)(uintptr_t)self ^ a ^ (b << 7);
}
static uint32_t __thiscall w3d_stub_handler3(void *self, void *p, uint32_t a, uint32_t b) {
    record(0x7d02, self);
    record(0x7d02, p);
    record(0x7d02, (void *)(uintptr_t)a);
    record(0x7d02, (void *)(uintptr_t)b);
    return (uint32_t)(uintptr_t)p ^ a ^ (b << 5);
}
// Cached::Find: the object's table (or none)
static void *__thiscall w3d_stub_table(void *self) {
    record(0x7d03, self);
    return arena<uint32_t>(0x24)[0] & 1 ? arena<void>(0x600) : nullptr;
}
static void *w3d_stub_vt[3] = {nullptr, (void *)w3d_stub_table, nullptr};
// Piece::Place: the part found
static void *__thiscall w3d_stub_part(void *self, uint32_t a, uint32_t b, int32_t c, void *d) {
    record(0x89caf0, self);
    for (uint32_t v : {a, b, (uint32_t)c}) record(0x89caf0, (void *)(uintptr_t)v);
    record(0x89caf0, d);
    return arena<void>(0x700);
}
// Keyed::Init: the base set up; records the keys' values (they live on the stack)
static void __thiscall w3d_stub_base(uint8_t *self, uint32_t one, void *field, uint32_t *k2, uint32_t *k0) {
    record(0x488290, self);
    record(0x488290, (void *)(uintptr_t)one);
    record(0x488290, field);
    for (uint32_t v : {k2[0], k2[1], k0[0], k0[1]}) record(0x488290, (void *)(uintptr_t)v);
    *(uint32_t *)(self + 4) = k2[0] ^ k0[0];
}
// Message::Send: builds the whole message; delivery records all of it
static uint32_t w3d_message_size;
static void *__thiscall w3d_stub_build(uint32_t *message, uint32_t a, uint32_t b, void *source) {
    record(0x7d04, (void *)(uintptr_t)a);
    record(0x7d04, (void *)(uintptr_t)b);
    record(0x7d04, source);
    for (uint32_t i = 0; i < w3d_message_size / 4; i++) message[i] = a * (i + 1) ^ b;
    return message;
}
static void __thiscall w3d_stub_deliver(void *target, uint32_t *message) {
    record(0x7d05, target);
    for (uint32_t i = 0; i < w3d_message_size / 4; i++) record(0x7d05, (void *)(uintptr_t)message[i]);
}
static void *__thiscall w3d_stub_ctor2(uint8_t *self, uint32_t a, void *b) {
    record(0x7d06, self);
    record(0x7d06, (void *)(uintptr_t)a);
    record(0x7d06, b);
    return self + 2;
}
// Single::Get (three zeros): registers itself in the copy's global, or sometimes doesn't
static void **w3d_single_global;
static void __thiscall w3d_stub_single3(void *self, uint32_t a, uint32_t b, uint32_t c) {
    record(0x7d07, self);
    for (uint32_t v : {a, b, c}) record(0x7d07, (void *)(uintptr_t)v);
    if (arena<uint32_t>(0x24)[0] & 1) *w3d_single_global = self;
}
static void *__thiscall w3d_stub_ctor1(uint8_t *self, uint32_t a) {
    record(0x7d08, self);
    record(0x7d08, (void *)(uintptr_t)a);
    return self + 3;
}
static void __stdcall w3d_stub_vector(void *first, uint32_t size, uint32_t count, void *ctor, void *dtor) {
    for (void *v : {first, (void *)(uintptr_t)size, (void *)(uintptr_t)count, ctor, dtor}) record(0xddcf99, v);
}
static void *__thiscall w3d_stub_find(void *map, uint32_t key) {
    record(0xbe82b0, map);
    record(0xbe82b0, (void *)(uintptr_t)key);
    return arena<uint32_t>(0x24)[0] & 1 ? arena<void>(0x900) : nullptr;
}
static void *__fastcall w3d_stub_empty(uint8_t flag) {
    record(0xbd0490, (void *)(uintptr_t)flag);
    return arena<void>(0x980);
}
