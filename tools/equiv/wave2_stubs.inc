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
