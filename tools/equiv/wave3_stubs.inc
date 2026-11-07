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
