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
