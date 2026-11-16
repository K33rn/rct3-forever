// GuestGroup::Check (forever/src/rewrite/group_checks.cpp). The group at arena 0x100 (random fields); its references
// at +0x84 / +0x98 linked into the lists of handles 0x400 / 0x500 (each list: a head and one other node); the group's
// handle (+0x50) is 0x600, Handle::For gives 0x700. The shared object is 0x900; the check notes what it is given,
// (each copy's its own stand-in, so the wrong one shows) changes the value and fills the stack place or not (by the case) and passes or fails (by the case). The references'
// tables note the reference and its handle; giving a handle up notes it. The result: pass (al), the fields written,
// the lists, the counts.
#if EQUIV_PART == 1
static uint32_t gc_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0xffffffffu; }
static void *gc_shared() { dev_note(780, 1); return arena<void>(0x900); }
static bool __fastcall gc_check(void *self, void *, void *part, uint32_t value, uint32_t *place, uint32_t *left) {
    dev_note(781, gc_at(self)); dev_note(781, gc_at(part)); dev_note(781, value); dev_note(781, *left == (uint32_t)(uintptr_t)arena<void>(0x100));
    uint32_t mode = arena<uint32_t>(0x50)[0];
    if (mode & 1) *place = arena<uint32_t>(0x50)[1];
    if (mode & 2) *left = arena<uint32_t>(0x50)[2];
    return (mode & 4) ? (uint8_t)(arena<uint32_t>(0x50)[3] | 1) : 0;
}
static bool __fastcall gc_check_b(void *self, void *e, void *part, uint32_t value, uint32_t *place, uint32_t *left) {
    dev_note(785, 2);   // (the other check: noted as such)
    return gc_check(self, e, part, value, place, left);
}
static void __fastcall gc_release(void *handle, void *, uint32_t z) { dev_note(782, gc_at(handle)); dev_note(782, z); }
static void *__fastcall gc_handle_for(uint32_t value) { dev_note(783, value); return arena<void>(0x700); }
static void __fastcall gc_told(uint32_t *r) {
    dev_note(784, gc_at(r)); dev_note(784, gc_at((void *)(uintptr_t)r[1]));
    dev_note(784, gc_at((void *)(uintptr_t)r[2])); dev_note(784, gc_at((void *)(uintptr_t)r[3]));
}
static void *gc_table[1] = {(void *)gc_told};
static void gc_handle(uint32_t at) {   // a handle at `at`: its count (+8), its list (+0x20: a head at +0x80, one node at +0x90)
    uint32_t *h = arena<uint32_t>(at + 0x80), *o = arena<uint32_t>(at + 0x90);
    h[0] = h[1] = (uint32_t)(uintptr_t)o;
    o[0] = o[1] = (uint32_t)(uintptr_t)h;
    arena<uint32_t>(at)[8] = (uint32_t)(uintptr_t)h;
}
static void gc_linked(uint32_t ref, uint32_t handle) {   // a reference linked after the handle's head
    uint32_t *r = arena<uint32_t>(ref), *head = arena<uint32_t>(handle + 0x80);
    r[0] = (uint32_t)(uintptr_t)gc_table, r[1] = (uint32_t)(uintptr_t)arena<void>(handle);
    uint32_t *node = r + 2, *after = (uint32_t *)(uintptr_t)head[1];
    node[0] = (uint32_t)(uintptr_t)head, node[1] = (uint32_t)(uintptr_t)after;
    head[1] = (uint32_t)(uintptr_t)node, after[0] = (uint32_t)(uintptr_t)node;
}
#else
    Test group_check{0x778880,
                     [=](Rng &r) {
                         fresh(r);
                         for (int k = 0; k < 0x30; k++) arena<uint32_t>(0x100)[k] = r.next();
                         for (uint32_t h : {0x400u, 0x500u, 0x600u, 0x700u}) {
                             for (int k = 0; k < 8; k++) arena<uint32_t>(h)[k] = r.next();
                             arena<uint32_t>(h)[2] = r.range(1, 4);
                             gc_handle(h);
                         }
                         arena<uint32_t>(0x100 + 0x50)[0] = (uint32_t)(uintptr_t)arena<void>(0x600);
                         gc_linked(0x100 + 0x84, 0x400), gc_linked(0x100 + 0x98, 0x500);
                         for (int k = 0; k < 4; k++) arena<uint32_t>(0x50)[k] = r.next();
                         arena<uint32_t>(0x60)[0] = r.next();   // the value
                     },
                     [](void *fn) {
                         bool ok = ((bool(__fastcall *)(void *, void *, uint32_t))fn)(arena<void>(0x100), nullptr, arena<uint32_t>(0x60)[0]);
                         uint32_t h = (uint8_t)ok;
                         for (int k = 0; k < 0x30; k++) h = h * 31 + gc_at((void *)(uintptr_t)arena<uint32_t>(0x100)[k]);
                         for (uint32_t a : {0x400u, 0x500u, 0x600u, 0x700u})
                             for (uint32_t k : {2u, 0x20u, 0x21u, 0x24u, 0x25u}) h = h * 31 + gc_at((void *)(uintptr_t)arena<uint32_t>(a)[k]);
                         return Result{h};
                     }};
    group_check.iterations = 1000;
    group_check.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0x757f50, (void *)gc_shared}, {0x76ffa0, (void *)gc_check}, {0x76ff60, (void *)gc_check_b}, {0x8055c0, (void *)gc_release},
                       {0x805700, (void *)gc_handle_for}});
    };
    group_check.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(group_check);
#endif
