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
// Second pass.
static uint32_t w1b_at1, w1b_at2;
static void __thiscall w1b_stub_pair_base(void *self, uint32_t size, void *storage, uint32_t *a, uint32_t *b) {
    record(0x210, self); record(0x210, (void *)(uintptr_t)size); record(0x210, storage);
    record(0x210, (void *)(uintptr_t)a[0]); record(0x210, (void *)(uintptr_t)a[1]); record(0x210, (void *)(uintptr_t)b[0]); record(0x210, (void *)(uintptr_t)b[1]);
}
static uint32_t __fastcall w1b_stub_entry(void *owner) { record(0x211, owner); return *arena<uint32_t>(0x24); }
static uint32_t __thiscall w1b_stub_add(void *owner, uint32_t entry, void *field, uint32_t kind, uint32_t a, uint32_t b, uint32_t c) {
    record(0x212, owner); record(0x212, (void *)(uintptr_t)entry); record(0x212, field); record(0x212, (void *)(uintptr_t)kind);
    record(0x212, (void *)(uintptr_t)a); record(0x212, (void *)(uintptr_t)b); record(0x212, (void *)(uintptr_t)c);
    return entry ^ 0x5555;
}
// The reference lives on the stack: filled whole and returned; the end records what it holds, then wipes it.
static void *__thiscall w1b_stub_ref(uint32_t *self, uint32_t name, uint32_t where, uint32_t flag) {
    record(0x213, (void *)(uintptr_t)name); record(0x213, (void *)(uintptr_t)where); record(0x213, (void *)(uintptr_t)flag);
    for (uint32_t i = 0; i < 7; i++) self[i] = name * 3 + i * 0x101;
    return self;
}
static void __fastcall w1b_stub_ref_end(uint32_t *self) {
    for (uint32_t i = 0; i < 7; i++) { record(0x214, (void *)(uintptr_t)self[i]); self[i] = 0; }
}
static void __stdcall w1b_stub_vector_dtor(void *items, uint32_t size, uint32_t count, uint32_t dtor) {
    record(0x215, items); record(0x215, (void *)(uintptr_t)size); record(0x215, (void *)(uintptr_t)count); record(0x215, (void *)(uintptr_t)dtor);
}
static void *__cdecl w1b_stub_manager() { record(0x216, nullptr); return arena<void>(0x510); }
static uint32_t w1b_clones;
static void *__thiscall w1b_stub_clone(void *value, uint32_t flag) {
    record(0x217, value); record(0x217, (void *)(uintptr_t)flag);
    return arena<void>(0x2000 + w1b_clones++ * 0x100);
}
static void __thiscall w1b_stub_spot(void *self, uint32_t *spot, uint32_t flag) {   // the spot is on the stack: its values are recorded
    record(0x218, self); record(0x218, (void *)(uintptr_t)flag);
    record(0x218, (void *)(uintptr_t)((uintptr_t)spot & 0xf));
    for (int i = 0; i < 3; i++) record(0x218, (void *)(uintptr_t)spot[i]);
}
// Third pass.
static uint32_t *w1c_global;   // the global of the copy under test
static uint32_t w1c_at(const rewrite::Entry *e, uint32_t operand) { return e->at + operand + 4 + *(int32_t *)(e->at + operand); }   // a call's target
static uint32_t w1c_word(const rewrite::Entry *e, uint32_t operand) { return *(uint32_t *)(e->at + operand); }
static void *__thiscall w1c_stub_text(uint32_t *self, uint32_t name, uint32_t offset) {   // the text is on the stack
    record(0x220, (void *)(uintptr_t)name); record(0x220, (void *)(uintptr_t)offset);
    for (uint32_t i = 0; i < 5; i++) self[i] = name * 5 + offset + i * 0x11;
    return arena<void>(0x580);
}
static void __fastcall w1c_stub_text_end(uint32_t *self) { for (uint32_t i = 0; i < 5; i++) { record(0x221, (void *)(uintptr_t)self[i]); self[i] = 0; } }
static void __thiscall w1c_stub_take_ref(void *self, uint32_t *ref) { record(0x222, self); for (int i = 0; i < 7; i++) record(0x222, (void *)(uintptr_t)ref[i]); }
static uint32_t __thiscall w1c_stub_virtual(void *self, uint32_t a) { record(0x223, self); record(0x223, (void *)(uintptr_t)a); return a ^ 0x5a5a; }
static void *__thiscall w1c_stub_spawned(uint32_t *self, uint32_t a, void *b) {
    record(0x224, self); record(0x224, (void *)(uintptr_t)a); record(0x224, b);
    self[0] = arena<uint32_t>(0x640)[0];
    return self;
}
static void *__cdecl w1c_stub_alloc_maybe(uint32_t size) { record(0x225, (void *)(uintptr_t)size); return arena<uint32_t>(0x20)[0] & 3 ? arena<void>(0x2000) : nullptr; }
static void __fastcall w1c_stub_swap(void *self) { record(0x226, self); *w1c_global = (uint32_t)(uintptr_t)arena<void>(0x6a0); }
static void *__thiscall w1c_stub_mid(uint32_t *self, uint32_t flag) {
    record(0x227, (void *)(uintptr_t)flag);
    for (uint32_t i = 0; i < 13; i++) self[i] = 0x7000 + i * 3;
    return arena<uint32_t>(0x20)[0] & 1 ? self : nullptr;
}
static void __fastcall w1c_stub_mid_end(uint32_t *self) { for (uint32_t i = 0; i < 13; i++) { record(0x228, (void *)(uintptr_t)self[i]); self[i] = 0; } }
static uint32_t __thiscall w1c_stub_add(void *self, uint32_t *what, uint32_t value) {
    record(0x229, self); record(0x229, (void *)(uintptr_t)(what ? *what : 0)); record(0x229, (void *)(uintptr_t)value);
    return value + 7;
}
static void *__thiscall w1c_stub_pool(void *self, uint32_t a, uint32_t size) {
    record(0x22a, self); record(0x22a, (void *)(uintptr_t)a); record(0x22a, (void *)(uintptr_t)size);
    return arena<uint32_t>(0x20)[0] & 3 ? arena<void>(0x2000) : nullptr;
}
static uint32_t __thiscall w1c_stub_built(void *self, void *a, uint32_t b) { record(0x22b, self); record(0x22b, a); record(0x22b, (void *)(uintptr_t)b); return b ^ 0x3333; }
static void __fastcall w1c_stub_see_global(void *self) { record(0x22c, self); record(0x22c, (void *)(uintptr_t)*w1c_global); }
static void __thiscall w1c_stub_pair(void *self, uint32_t *owner, uint32_t *made) {
    record(0x22d, self); record(0x22d, (void *)(uintptr_t)*owner); record(0x22d, (void *)(uintptr_t)*made);
    record(0x22d, (void *)(uintptr_t)((uint8_t *)owner - (uint8_t *)made));
    uint32_t *object = (uint32_t *)(uintptr_t)*made;
    record(0x22d, (void *)(uintptr_t)object[0]); record(0x22d, (void *)(uintptr_t)object[1]);
}
static void *__cdecl w1d_stub_aligned(uint32_t size) { record(0x240, (void *)(uintptr_t)size); return arena<uint32_t>(0x20)[0] & 1 ? arena<void>(0x2000) : nullptr; }
static void __fastcall w1d_stub_member_end(uint32_t *self) {
    record(0x241, self); record(0x241, (void *)(uintptr_t)self[-1]); record(0x241, (void *)(uintptr_t)self[-9]);
    self[0] = 0x4444;
}
static uint32_t __fastcall w1d_stub_empty_text(uint8_t flag) { record(0x242, (void *)(uintptr_t)flag); return ++arena<uint32_t>(0x30)[0]; }
static uint32_t __thiscall w1d_stub_act(void *self, void *who, uint32_t value) { record(0x243, self); record(0x243, who); record(0x243, (void *)(uintptr_t)value); return value * 3 + 1; }
static void __stdcall w1d_stub_release(uint32_t *self) { record(0x244, self); record(0x244, (void *)(uintptr_t)arena<uint32_t>(0x800)[0]); self[1] = 0x5151; }
static void *__cdecl w1d_stub_record() { record(0x245, (void *)(uintptr_t)arena<uint32_t>(0x800)[8]); return arena<void>(0x2000); }
static void __cdecl w1d_stub_free(void *p) { record(0x246, p); }
static void __fastcall w1d_stub_end_seen(void *self) { record(0x247, self); record(0x247, (void *)(uintptr_t)arena<uint32_t>(0x800)[0]); record(0x247, (void *)(uintptr_t)arena<uint32_t>(0x800)[2]); }
static void *__thiscall w1d_stub_build3(void *self, void *a, uint32_t b, uint32_t c) {
    record(0x248, self); record(0x248, a); record(0x248, (void *)(uintptr_t)b); record(0x248, (void *)(uintptr_t)c);
    return (uint8_t *)self + 4;
}
static void __thiscall w1d_stub_base4(uint32_t *self, uint32_t a, uint32_t b, void *d, uint32_t e) {
    record(0x249, self); record(0x249, (void *)(uintptr_t)a); record(0x249, (void *)(uintptr_t)b); record(0x249, d); record(0x249, (void *)(uintptr_t)e);
    self[0] = 0x1111; self[1] = a ^ e;
}
static void __thiscall w1d_stub_apply(void *self, uint32_t c) { record(0x24a, self); record(0x24a, (void *)(uintptr_t)c); record(0x24a, (void *)(uintptr_t)arena<uint32_t>(0x800)[0]); }
static void *__thiscall w1d_stub_build_ref(void *self, uint32_t a, uint32_t *p) {
    record(0x24b, self); record(0x24b, (void *)(uintptr_t)a); record(0x24b, (void *)(uintptr_t)*p);
    return (uint8_t *)self + 8;
}
static void __fastcall w1d_stub_first(void *self) { record(0x24c, self); record(0x24c, (void *)(uintptr_t)arena<uint32_t>(0x800)[0]); }
static void *__cdecl w1d_stub_alloc_always(uint32_t size) { record(0x24d, (void *)(uintptr_t)size); return arena<void>(0x2000); }
static void __fastcall w1d_stub_registers(void *self) { record(0x24e, self); if (arena<uint32_t>(0x20)[1] & 1) *w1c_global = (uint32_t)(uintptr_t)self; }
static void *__thiscall w1d_stub_build_flags(uint8_t *self, uint32_t a, uint32_t b, uint32_t c) {
    record(0x24f, self); record(0x24f, (void *)(uintptr_t)(a + b + c));
    self[0x2c] = (uint8_t)arena<uint32_t>(0x20)[1];
    return self;
}
static void __thiscall w1d_stub_delete(void *self, uint32_t flag) { record(0x250, self); record(0x250, (void *)(uintptr_t)flag); }
static uint32_t w1d_modes[4];   // the copy's four modes
static uint32_t __thiscall w1d_stub_owner(void *self, void *who) {
    record(0x251, self); record(0x251, who);
    uint32_t pick = arena<uint32_t>(0x20)[2] % 3;
    return pick == 0 ? 0 : pick == 1 ? arena<uint32_t>(0x20)[1] : arena<uint32_t>(0x20)[3];
}
static void __thiscall w1d_stub_switch(void *self, uint32_t code) { record(0x252, self); record(0x252, (void *)(uintptr_t)code); }
static void *__fastcall w1d_stub_part(void *self) { record(0x253, self); return arena<uint32_t>(0x20)[0] & 1 ? arena<void>(0xa00) : nullptr; }
static void __thiscall w1d_stub_slow(void *self, uint32_t *out, uint32_t extra) { record(0x254, self); record(0x254, (void *)(uintptr_t)extra); out[3] = extra; }
static void __thiscall w1d_stub_helper(void *self, uint32_t a, uint32_t b) { record(0x255, self); record(0x255, (void *)(uintptr_t)(a ^ b)); }
template <int N> static void __thiscall w1d_stub_slot(void *self, uint32_t a, uint32_t b) { record(0x256 + N, self); record(0x256 + N, (void *)(uintptr_t)(a - b)); }
static void *__fastcall w1d_stub_car_a(void *self) { record(0x25a, self); return arena<uint32_t>(0x20)[0] & 1 ? arena<void>(0xb00) : nullptr; }
static void *__fastcall w1d_stub_car_b(void *self) { record(0x25b, self); return arena<uint32_t>(0x20)[0] & 2 ? arena<void>(0xb00) : nullptr; }
static bool __thiscall w1d_stub_ask(void *self, uint32_t a, uint32_t b, uint32_t c) { record(0x25c, self); record(0x25c, (void *)(uintptr_t)(a + b * 3 + c * 7)); return (a ^ c) & 1; }
template <int N> static void __fastcall w1d_stub_end_n(uint32_t *self) {
    record(0x260 + N, self);
    for (int i = 0; i < 2; i++) record(0x260 + N, (void *)(uintptr_t)arena<uint32_t>(0x800)[i]);
    self[0] = 0x9000 + N;
}
static void __thiscall w1d_stub_label(void *self, uint32_t *out) {
    record(0x270, self);
    *out = (uint32_t)(uintptr_t)(arena<uint32_t>(0x20)[0] & 1 ? arena<void>(0xc00) : *(void **)arena<uint8_t>(0xb00 + 0x114));
    ++*(int32_t *)(uintptr_t)*out;
}
static void __stdcall w1d_stub_build_items(uint32_t *items, uint32_t size, uint32_t count, uint32_t build, uint32_t end) {
    record(0x271, items); record(0x271, (void *)(uintptr_t)size); record(0x271, (void *)(uintptr_t)count);
    record(0x271, (void *)(uintptr_t)build); record(0x271, (void *)(uintptr_t)end);
    for (uint32_t i = 0; i < count * size / 4; i++) items[i] = 0x7700 + i;
}
static void __stdcall w1d_stub_destroy_items(void *items, uint32_t size, uint32_t count, uint32_t end) {
    record(0x272, items); record(0x272, (void *)(uintptr_t)size); record(0x272, (void *)(uintptr_t)count); record(0x272, (void *)(uintptr_t)end);
}
static void __thiscall w1d_stub_assign(uint32_t *self, uint32_t *from) { record(0x273, self); record(0x273, (void *)(uintptr_t)from[0]); self[0] = from[0]; self[1] = from[1] ^ 1; }
// new shared texts: each item points at one of four text blocks at 0xe00.. (a count, then the text pointer at +0xc)
static void __stdcall w1d_stub_build_texts(uint32_t *items, uint32_t size, uint32_t count, uint32_t build, uint32_t end) {
    record(0x274, items); record(0x274, (void *)(uintptr_t)size); record(0x274, (void *)(uintptr_t)count);
    record(0x274, (void *)(uintptr_t)build); record(0x274, (void *)(uintptr_t)end);
    for (uint32_t i = 0; i < count; i++) {
        uint32_t *text = arena<uint32_t>(0xe00 + ((i + arena<uint32_t>(0x20)[2]) & 3) * 0x20);
        items[i] = (uint32_t)(uintptr_t)text;
        text[0]++;
    }
}
static int32_t *__thiscall w1d_stub_make_text(int32_t *self, uint32_t name) { record(0x275, self); record(0x275, (void *)(uintptr_t)name); self[0] = arena<int32_t>(0x20)[1] & 1; self[3] = 0x4321; return self; }
static void __thiscall w1d_stub_base_text(uint32_t *self, int32_t **text) { record(0x276, self); record(0x276, *text); (**text)++; self[0] = 0x5555; }
static uint32_t __thiscall w1d_stub_find_named(void *self, uint32_t name) { record(0x277, self); record(0x277, (void *)(uintptr_t)name); return arena<uint32_t>(0x20)[0] & 1 ? (uint32_t)(uintptr_t)arena<void>(0xa00) : 0; }
static void *__thiscall w1d_stub_copy_named(void *self, uint32_t flag) { record(0x278, self); record(0x278, (void *)(uintptr_t)flag); return arena<uint32_t>(0x20)[0] & 2 ? arena<void>(0xb00) : nullptr; }
static void __thiscall w1d_stub_set_value(void *self, uint32_t value) { record(0x279, self); record(0x279, (void *)(uintptr_t)value); }
static uint32_t __thiscall w1d_stub_build1(void *self, uint32_t value) { record(0x27a, self); record(0x27a, (void *)(uintptr_t)value); return value ^ 0x2222; }
static void *__thiscall w1d_stub_temp_text(uint32_t *self, uint32_t name, uint32_t table, uint32_t flag) {
    record(0x27b, (void *)(uintptr_t)name); record(0x27b, (void *)(uintptr_t)table); record(0x27b, (void *)(uintptr_t)flag);
    for (int i = 0; i < 7; i++) self[i] = name + i;
    return self;
}
static void __thiscall w1d_stub_use_text(void *self, uint32_t *text, uint32_t flag) { record(0x27c, self); record(0x27c, (void *)(uintptr_t)text[0]); record(0x27c, (void *)(uintptr_t)flag); }
static void __fastcall w1d_stub_temp_end(uint32_t *self) { for (int i = 0; i < 7; i++) { record(0x27d, (void *)(uintptr_t)self[i]); self[i] = 0; } }
