// Stand-ins and helpers for wave 1's tests.
static void w1_fill(Rng &r, uint32_t off, uint32_t bytes) { for (uint32_t i = 0; i < bytes / 4; i++) arena<uint32_t>(off)[i] = r.next(); }
// A list of n nodes at `base`, then its end node (no next); value i is the object at 0x1800 + i*0x10, with table `vt`.
// A set bit of `empty` leaves that value null.
static void *w1_chain(uint32_t base, int n, uint32_t vt, uint32_t empty = 0) {
    for (int i = 0; i <= n; i++) {
        uint32_t *node = arena<uint32_t>(base + i * 0x10);
        void *object = arena<void>(0x1800 + i * 0x10);
        *(void **)object = arena<void>(vt);
        node[0] = i < n ? (uint32_t)(uintptr_t)arena<void>(base + (i + 1) * 0x10) : 0;
        node[1] = 0x1111;
        node[2] = (empty >> i) & 1 ? 0 : (uint32_t)(uintptr_t)object;
    }
    return arena<void>(base);
}
static void *w1_object(uint32_t off, uint32_t vt) { *(void **)arena<void>(off) = arena<void>(vt); return arena<void>(off); }
static void *__fastcall w1_stub_pool_alloc(uint32_t size) { record(0x1f0, (void *)(uintptr_t)size); return arena<void>(0x3000); }
static void __fastcall w1_stub_other(void *self) { record(0x1f1, self); }
static uint8_t *__fastcall w1_stub_level(void *self) { record(0x1f2, self); return arena<uint8_t>(0x510); }
static void __stdcall w1_stub_std2(void *device, uint32_t v) { record(0x1f3, device); record(0x1f3, (void *)(uintptr_t)v); }
static void __stdcall w1_stub_std1(void *object) { record(0x1f4, object); }
static void __thiscall w1_stub_this3(void *self, void *out, uint32_t a, uint32_t b) {
    record(0x1f5, self); record(0x1f5, out); record(0x1f5, (void *)(uintptr_t)a); record(0x1f5, (void *)(uintptr_t)b);
}
static uint32_t __thiscall w1_stub_this4(void *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    *(uint8_t *)self = 1;   // the object is on the stack: only written, never recorded
    record(0x1f6, (void *)(uintptr_t)a); record(0x1f6, (void *)(uintptr_t)b); record(0x1f6, (void *)(uintptr_t)c); record(0x1f6, (void *)(uintptr_t)d);
    return a + b;
}
static void __fastcall w1_stub_fast2(void *self, uint32_t k) { record(0x1f7, self); record(0x1f7, (void *)(uintptr_t)k); }
static void *__fastcall w1_stub_maybe(void *self) { record(0x1f8, self); return (*arena<uint32_t>(0x20) & 1) ? arena<void>(0x500) : nullptr; }
static uint32_t __fastcall w1_stub_value(void *self) { record(0x1f9, self); return (uint32_t)(uintptr_t)self ^ 0x1234; }
static uint32_t w1_pop_holder;
static void __thiscall w1_stub_pop(void *object, uint32_t flag) {   // an object taking itself off its list as it goes
    record(0x1fa, object); record(0x1fa, (void *)(uintptr_t)flag);
    uint32_t *holder = arena<uint32_t>(w1_pop_holder);
    *holder = *(uint32_t *)(uintptr_t)*holder;
}
static void __cdecl w1_stub_crt_free(void *p) { record(0x200, p); }
static void __thiscall w1_stub_grow(uint32_t *items, uint32_t n) { record(0x201, items); record(0x201, (void *)(uintptr_t)n); items[1] = n; }
static void *__fastcall w1_stub_next(void *item) { record(0x1fb, item); return *(void **)((uint8_t *)item + 4); }
static void __thiscall w1_stub_two_b(void *self, uint32_t a, uint32_t b) { record(0x1fc, self); record(0x1fc, (void *)(uintptr_t)a); record(0x1fc, (void *)(uintptr_t)b); }
static uint32_t __thiscall w1_stub_load(void *self, void *name) { record(0x1fd, self); record(0x1fd, name); return (*arena<uint32_t>(0x24) & 1) ? (uint32_t)(uintptr_t)arena<void>(0x900) : 0; }
static uint32_t __fastcall w1_stub_willing(void *self) { record(0x1fe, self); return *arena<uint32_t>(0x24) & 0x101; }
static void __thiscall w1_stub_one_b(void *self, uint32_t a) { record(0x1ff, self); record(0x1ff, (void *)(uintptr_t)a); }
static uint32_t __thiscall w1_stub_ask(void *self, uint32_t a) { record(0x202, self); record(0x202, (void *)(uintptr_t)a); return *arena<uint32_t>(0x24) & 0x101; }
static void __thiscall w1_stub_five(void *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    record(0x203, self); record(0x203, (void *)(uintptr_t)a); record(0x203, (void *)(uintptr_t)b); record(0x203, (void *)(uintptr_t)c);
    record(0x203, (void *)(uintptr_t)d); record(0x203, (void *)(uintptr_t)e);
}
static void __thiscall w1_stub_four(void *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    record(0x204, self); record(0x204, (void *)(uintptr_t)a); record(0x204, (void *)(uintptr_t)b); record(0x204, (void *)(uintptr_t)c); record(0x204, (void *)(uintptr_t)d);
}
