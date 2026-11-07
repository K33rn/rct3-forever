// Stand-ins for the rewrites of wave 2.
// the handler at slot 0x20: what is recorded is the event words, never where it lives on the stack
static uint32_t __thiscall w2_stub_event(void *self, uint32_t *event) {
    record(0xe201, self);
    for (int i = 0; i < 3; i++) record(0xe201, (void *)(uintptr_t)event[i]);
    return arena<uint32_t>(0x20)[0];
}
static void *w2_event_table[0x40] = {};
static void __stdcall w2_stub_job(void *job) {
    record(0xe202, job);
    record(0xe202, (void *)(uintptr_t)*arena<uint16_t>(0x40e));
}
static void *w2_job_table[0x10] = {};
static uint32_t __thiscall w2_stub_triple(void *self, uint32_t *triple) {
    record(0xe203, self);
    for (int i = 0; i < 3; i++) record(0xe203, (void *)(uintptr_t)triple[i]);
    return arena<uint32_t>(0x20)[1];
}
template <uint32_t SLOT> static void __thiscall w2_stub_tell(void *self, uint32_t value) {
    record(0xe204 + SLOT, self);
    record(0xe204 + SLOT, (void *)(uintptr_t)value);
    if (arena<uint32_t>(0x20)[2] % 5 == 0) (*arena<int32_t>(0x8b8))--;   // a child can leave the array while being told
}
static void *w2_tell_table[4] = {nullptr, (void *)w2_stub_tell<1>, (void *)w2_stub_tell<2>, (void *)w2_stub_tell<3>};
static void __thiscall w2_stub_delete_array(void *self, uint32_t flags) {
    record(0xe20a, self);
    record(0xe20a, (void *)(uintptr_t)flags);
}
static void *w2_array_table[1] = {(void *)w2_stub_delete_array};
static void __fastcall w2_stub_add_ref(void *object) { record(0xe210, object); }
static void __fastcall w2_stub_drop_ref(void *object) { record(0xe211, object); }
static void __thiscall w2_stub_text(void *self, void *into, const char *text, uint32_t length) {
    record(0xe212, self);
    record(0xe212, into);
    record(0xe212, (void *)text);
    record(0xe212, (void *)(uintptr_t)length);
}
static void __fastcall w2_stub_visit(void *value) {
    record(0xe213, value);
    if (arena<uint32_t>(0x20)[0] % 7 == 0) (*arena<int32_t>(0x160))--;   // an entry can go away during the visit
}
template <uint32_t SLOT> static void __thiscall w2_stub_put(uint32_t *self, uint32_t place, uint32_t value) {
    record(0xe220 + SLOT, self);
    record(0xe220 + SLOT, (void *)(uintptr_t)place);
    record(0xe220 + SLOT, (void *)(uintptr_t)value);
    if (arena<uint32_t>(0x20)[1] % 3 == 0) self[3] += 8;   // the place can move during the write
}
static void *w2_put_table[8] = {nullptr, nullptr, nullptr, (void *)w2_stub_put<3>, nullptr, (void *)w2_stub_put<5>, nullptr, (void *)w2_stub_put<7>};
static uint32_t __thiscall w2_stub_typed(void *self, uint32_t *marker, uint32_t value) {
    record(0xe230, self);
    record(0xe230, (void *)(uintptr_t)*marker);
    record(0xe230, (void *)(uintptr_t)value);
    return arena<uint32_t>(0x20)[0];
}
static void __thiscall w2_stub_ask(void *self, uint32_t *event) {
    record(0xe231, self);
    record(0xe231, (void *)(uintptr_t)event[0]);
    record(0xe231, (void *)(uintptr_t)event[1]);
    if (arena<uint32_t>(0x20)[0] & 1) event[1] = arena<uint32_t>(0x20)[1];
}
static void __thiscall w2_stub_ask_flag(void *self, uint32_t *event) {
    record(0xe232, self);
    record(0xe232, (void *)(uintptr_t)event[0]);
    record(0xe232, (void *)(uintptr_t)*(uint8_t *)&event[1]);
    if (arena<uint32_t>(0x20)[0] & 1) *(uint8_t *)&event[1] = (uint8_t)arena<uint32_t>(0x20)[1];
}
static void *w2_ask_table[0x21] = {};
static void *w2_ask_flag_table[0x21] = {};
static void __stdcall w2_stub_go(void *object) { record(0xe233, object); }
static void *w2_go_table[3] = {nullptr, nullptr, (void *)w2_stub_go};
static void __thiscall w2_stub_counted_delete(void *object, uint32_t flags) {
    record(0xe234, object);
    record(0xe234, (void *)(uintptr_t)flags);
}
static void *w2_counted_table[1] = {(void *)w2_stub_counted_delete};
static uint32_t __thiscall w2_stub_pass(void *item, uint32_t a, uint32_t b) {
    record(0xe235, item);
    record(0xe235, (void *)(uintptr_t)a);
    record(0xe235, (void *)(uintptr_t)b);
    return arena<uint32_t>(0x20)[0];
}
static uint32_t __fastcall w2_stub_default(uint8_t kind) {
    record(0xe236, (void *)(uintptr_t)kind);
    return arena<uint32_t>(0x20)[0];
}
static uint32_t __thiscall w2_stub_one(void *self, uint32_t value) {
    record(0xe240, self);
    record(0xe240, (void *)(uintptr_t)value);
    return arena<uint32_t>(0x20)[0];
}
static void *__stdcall w2_stub_lookup(void *self, uint32_t a, uint32_t b) {
    record(0xe241, self);
    record(0xe241, (void *)(uintptr_t)a);
    record(0xe241, (void *)(uintptr_t)b);
    return arena<void>(0x700);
}
static uint32_t __fastcall w2_stub_found(void *found) {
    record(0xe242, found);
    return arena<uint32_t>(0x20)[1];
}
static void __thiscall w2_stub_read(void *self, void *out, void *member) {
    record(0xe243, self);
    record(0xe243, out);
    record(0xe243, member);
}
static void __thiscall w2_stub_into(void *out, void *member) {
    record(0xe244, out);
    record(0xe244, member);
}
static void __fastcall w2_stub_update(void *self) { record(0xe245, self); }
static void __thiscall w2_stub_log(void *log, const char *text, uint32_t length) {
    record(0xe246, log);
    record(0xe246, (void *)text);
    record(0xe246, (void *)(uintptr_t)length);
}
static void *__thiscall w2_stub_chunk(uint8_t *self, uint32_t zero) {
    record(0xe247, self);
    record(0xe247, (void *)(uintptr_t)zero);
    *(uint32_t *)(self + 0x500) = 0;
    *(uint32_t *)(self + 0x504) = 0;
    return self;
}
static uint32_t g_w2_message_size;
static void __thiscall w2_stub_build(uint8_t *message, uint32_t a, uint32_t b, void *sender) {
    record(0xe250, (void *)(uintptr_t)a);
    record(0xe250, (void *)(uintptr_t)b);
    record(0xe250, sender);
    for (uint32_t i = 0; i < g_w2_message_size; i++) message[i] = (uint8_t)(a + i);
}
static void __thiscall w2_stub_post(void *queue, uint8_t *message) {
    record(0xe251, queue);
    for (uint32_t i = 0; i < g_w2_message_size; i += 4) record(0xe251, (void *)(uintptr_t)*(uint32_t *)(message + i));
}
static void __thiscall w2_stub_flush(void *self, void *data, uint32_t count) {
    record(0xe252, self);
    record(0xe252, data);
    record(0xe252, (void *)(uintptr_t)count);
}
static void *w2_flush_table[1] = {(void *)w2_stub_flush};
static void *__fastcall w2_stub_target(void *self) {
    record(0xe253, self);
    return arena<void *>(0x20)[0];
}
static uint32_t __fastcall w2_stub_has_target(void *self) {
    record(0xe254, self);
    return arena<uint32_t>(0x20)[1];
}
static uint8_t __thiscall w2_stub_check(void *self, uint32_t zero, void *target) {
    record(0xe255, self);
    record(0xe255, (void *)(uintptr_t)zero);
    record(0xe255, target);
    return (uint8_t)arena<uint32_t>(0x20)[2];
}
static void __thiscall w2_stub_state2(void *self, uint32_t value) {
    record(0xe256, self);
    record(0xe256, (void *)(uintptr_t)value);
}
static void *__thiscall w2_stub_member(void *member, void *data) {
    record(0xe257, member);
    record(0xe257, data);
    return arena<void>(0x780);
}
static void *w2_state_table[0x16] = {};
static void *w2_member_table[5] = {};
static uint32_t __fastcall w2_stub_plain_update(void *member) {
    record(0xe260, member);
    return arena<uint32_t>(0x20)[0];
}
static uint32_t __fastcall w2_stub_timed_update(void *member) {
    record(0xe261, member);
    return arena<uint32_t>(0x20)[1];
}
static uint32_t __thiscall w2b_stub_walker(void *self, uint32_t first, uint32_t *visitor, uint32_t third, uint32_t fourth) {
    record(0xe270, self);
    record(0xe270, (void *)(uintptr_t)first);
    record(0xe270, (void *)(uintptr_t)visitor[0]);
    record(0xe270, (void *)(uintptr_t)visitor[1]);
    record(0xe270, (void *)(uintptr_t)third);
    record(0xe270, (void *)(uintptr_t)fourth);
    return arena<uint32_t>(0x20)[0];
}
// stops the held object, and on odd seeds clears the owner's hold as it does so
static void __thiscall w2b_stub_stop(void *held, uint32_t how) {
    record(0xe271, held);
    record(0xe271, (void *)(uintptr_t)how);
    if (arena<uint32_t>(0x20)[1] & 1) *(void **)(arena<uint8_t>(0x100) + 0x2c) = nullptr;
}
static void *w2b_delete_table[1] = {(void *)stub_thiscall1};
static void *__cdecl w2b_stub_manager() {
    record(0xe272, nullptr);
    return arena<void>(0x800);
}
static void __cdecl w2b_stub_free_item(void *item) { record(0xe273, item); }
static void __cdecl w2b_stub_free_table(void *table) { record(0xe274, table); }
static void *__fastcall w2b_stub_root(void *self) {
    record(0xe275, self);
    return arena<void>(0x800);
}
static void __thiscall w2b_stub_scene(void *scene, uint32_t kind, void *part) {
    record(0xe276, scene);
    record(0xe276, (void *)(uintptr_t)kind);
    record(0xe276, part);
}
// the visitor is on the caller's stack, so its contents are recorded and not its place
static void __thiscall w2b_stub_visit(void *self, uint32_t *visitor) {
    record(0xe277, self);
    record(0xe277, (void *)(uintptr_t)visitor[0]);
    record(0xe277, (void *)(uintptr_t)visitor[1]);
}
static uint32_t __fastcall w2b_stub_check1(void *self) {
    record(0xe278, self);
    return arena<uint32_t>(0x20)[0];
}
static uint32_t __fastcall w2b_stub_check2(void *self) {
    record(0xe279, self);
    return arena<uint32_t>(0x20)[1];
}
// hands out arena memory, or fails, as the seed says
static void *__cdecl w2b_stub_alloc(uint32_t size) {
    record(0xe27a, (void *)(uintptr_t)size);
    return arena<uint32_t>(0x20)[1] & 1 ? arena<void>(0x900) : nullptr;
}
static void __stdcall w2b_stub_build(void *rows, uint32_t size, uint32_t count, uint32_t init) {
    record(0xe27b, rows);
    record(0xe27b, (void *)(uintptr_t)size);
    record(0xe27b, (void *)(uintptr_t)count);
    record(0xe27b, (void *)(uintptr_t)init);
}
static uint32_t __thiscall w2b_stub_find(void *table, uint32_t key) {
    record(0xe27c, table);
    record(0xe27c, (void *)(uintptr_t)key);
    return arena<uint32_t>(0x20)[0] & 3;
}
static uint32_t __thiscall w2b_stub_verdict(void *self, uint32_t subject) {
    record(0xe27d, self);
    record(0xe27d, (void *)(uintptr_t)subject);
    return arena<uint32_t>(0x20)[1] & 3;
}
// ---- third pass ----
static uint32_t g_w2c_a, g_w2c_b;   // per-copy constants of the test being run
static uint32_t w2c_target(uintptr_t at, uint32_t off) { return (uint32_t)(at + off + 4 + *(int32_t *)(at + off)); }
static uint32_t w2c_word(uintptr_t at, uint32_t off) { return *(uint32_t *)(at + off); }
#define W2C_RET arena<uint32_t>(0x20)[3]
#define W2C_REC(tag, v) record(tag, (void *)(uintptr_t)(v))
template <uint32_t TAG> static uint32_t __fastcall w2c_s0(void *self) { record(TAG, self); return W2C_RET; }
template <uint32_t TAG> static uint32_t __thiscall w2c_s1(void *self, uint32_t a) { record(TAG, self); W2C_REC(TAG, a); return W2C_RET; }
template <uint32_t TAG> static uint32_t __thiscall w2c_s2(void *self, uint32_t a, uint32_t b) { record(TAG, self); W2C_REC(TAG, a); W2C_REC(TAG, b); return W2C_RET; }
template <uint32_t TAG> static uint32_t __thiscall w2c_s3(void *self, uint32_t a, uint32_t b, uint32_t c) { record(TAG, self); W2C_REC(TAG, a); W2C_REC(TAG, b); W2C_REC(TAG, c); return W2C_RET; }
template <uint32_t TAG> static uint32_t __thiscall w2c_s4(void *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d) { record(TAG, self); W2C_REC(TAG, a); W2C_REC(TAG, b); W2C_REC(TAG, c); W2C_REC(TAG, d); return W2C_RET; }
template <uint32_t TAG> static uint32_t __thiscall w2c_s5(void *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e) { record(TAG, self); W2C_REC(TAG, a); W2C_REC(TAG, b); W2C_REC(TAG, c); W2C_REC(TAG, d); W2C_REC(TAG, e); return W2C_RET; }
template <uint32_t TAG> static void __cdecl w2c_free(void *p) { record(TAG, p); }
// memory, or none, as the seed says
static void *__cdecl w2c_alloc(uint32_t size) {
    W2C_REC(0xe2c0, size);
    return arena<uint32_t>(0x20)[4] % 3 ? arena<void>(0x1800) : nullptr;
}
// a different block on every call, whatever the size
static void *__cdecl w2c_alloc_step(uint32_t size) {
    W2C_REC(0xe2c1, size);
    return arena<void>(0x1900 + 0x10 * (*arena<uint32_t>(0x40))++);
}
// a constructor that fills in the singleton's global
static uint32_t __thiscall w2c_ctor_sets(void *self, uint32_t a) {
    record(0xe2c2, self);
    W2C_REC(0xe2c2, a);
    *(void **)(uintptr_t)g_w2c_a = self;
    return 0x7777;
}
static uint32_t __fastcall w2c_flag(uint32_t flag) {
    W2C_REC(0xe2c3, flag & 0xff);
    return W2C_RET + (*arena<uint32_t>(0x40))++;
}
static void *__thiscall w2c_pool(void *self, uint32_t a, uint32_t b) {
    record(0xe2c4, self);
    W2C_REC(0xe2c4, a);
    W2C_REC(0xe2c4, b);
    return arena<uint32_t>(0x20)[4] % 3 ? arena<void>(0x1800) : nullptr;
}
static uint32_t __thiscall w2c_key_find(void *self, uint32_t **key) {
    record(0xe2c5, self);
    for (int i = 0; i < 2; i++) { W2C_REC(0xe2c5, key[i]); W2C_REC(0xe2c5, *key[i]); }
    return W2C_RET;
}
static void __fastcall w2c_key_drop(uint32_t **key) {
    for (int i = 0; i < 2; i++) { W2C_REC(0xe2c6, key[i]); W2C_REC(0xe2c6, *key[i]); }
}
static void __thiscall w2c_join(void *owner, uint32_t *held, uint8_t **me) {
    record(0xe2c7, owner);
    W2C_REC(0xe2c7, *held);
    record(0xe2c7, *me);
    W2C_REC(0xe2c7, *(uint32_t *)*me);   // the class it is at this point
}
static void __thiscall w2c_grid_base(void *self, uint32_t n, void *p, uint32_t *first, uint32_t *second) {
    record(0xe2c8, self);
    W2C_REC(0xe2c8, n);
    record(0xe2c8, p);
    for (int i = 0; i < 2; i++) { W2C_REC(0xe2c8, first[i]); W2C_REC(0xe2c8, second[i]); }
}
static void __stdcall w2c_vector_make(void *at, uint32_t size, uint32_t n, uint32_t make, uint32_t drop) {
    record(0xe2c9, at);
    W2C_REC(0xe2c9, size);
    W2C_REC(0xe2c9, n);
    W2C_REC(0xe2c9, make);
    W2C_REC(0xe2c9, drop);
}
static void *w2c_table[1] = {(void *)w2c_s1<0xe2ca>};
// ---- fourth pass ----
// memory always (for the callers that would fault without it)
static void *__cdecl w2d_alloc_always(uint32_t size) {
    W2C_REC(0xe2d0, size);
    return arena<void>(0x1800);
}
// makes the text in its block with the seeded reference count (sometimes none), and gives the block back
static uint32_t __thiscall w2d_text_make(uint32_t *self, uint32_t source) {
    record(0xe2d1, self);
    W2C_REC(0xe2d1, source);
    self[0] = arena<uint32_t>(0x20)[5] % 3;
    self[3] = arena<uint32_t>(0x20)[2];
    return (uint32_t)(uintptr_t)self;
}
// fills the whole 0x1c-byte temporary, and gives back a value (never the temporary's address)
static uint32_t __thiscall w2d_build(uint32_t *self, uint32_t value, void *part, uint32_t zero) {
    W2C_REC(0xe2d2, value);
    record(0xe2d2, part);
    W2C_REC(0xe2d2, zero);
    for (int i = 0; i < 7; i++) self[i] = value + i * 0x101;
    return W2C_RET;
}
static void __fastcall w2d_build_drop(uint32_t *self) {
    for (int i = 0; i < 7; i++) W2C_REC(0xe2d3, self[i]);
}
// a teardown that also notes which table the object has at that moment
template <uint32_t TAG> static void __fastcall w2d_s_table(uint32_t *self) {
    record(TAG, self);
    W2C_REC(TAG, *self);
}
template <uint32_t TAG> static void __stdcall w2d_release(void *p) { record(TAG, p); }
static void __stdcall w2d_vector_make(void *at, uint32_t size, uint32_t n, uint32_t make) {
    record(0xe2d9, at);
    W2C_REC(0xe2d9, size);
    W2C_REC(0xe2d9, n);
    W2C_REC(0xe2d9, make);
}
static uint8_t *__fastcall w2d_look(void *self) {
    record(0xe2da, self);
    return arena<uint8_t>(0x600);
}
// fills the caller's 16-byte result from the kind and the two bytes, and gives it back
static uint32_t *__fastcall w2d_look_make(uint32_t *made, uint32_t kind, uint32_t first, uint32_t second) {
    W2C_REC(0xe2db, kind);
    W2C_REC(0xe2db, first);
    W2C_REC(0xe2db, second);
    for (int i = 0; i < 4; i++) made[i] = kind * 0x1000 + first * 0x10 + second + i * W2C_RET;
    return made;
}
// hands out the text set aside at 0x60 and points the caller either at it or at the slot at 0x3c0
static int32_t **__thiscall w2d_find_text(void *self, int32_t **out) {
    record(0xe2dc, self);
    *out = arena<int32_t *>(0x60)[0];
    return arena<uint32_t>(0x20)[4] & 1 ? out : arena<int32_t *>(0x3c0);
}
// gives the next entry of either queue of the object at 0x100, as many as the seed says
static void *__fastcall w2d_queue_next(uint8_t *queue) {
    record(0xe2dd, queue);
    int which = queue == arena<uint8_t>(0x100) + 0xc ? 0 : 1;
    uint32_t &taken = arena<uint32_t>(0x40)[which];
    if (taken >= arena<uint32_t>(0x20)[which] % 4) return nullptr;
    return arena<uint8_t>(which ? 0x600 : 0x300) + 0x40 * taken++;
}
