// Animal::Link (forever/src/rewrite/animal_links.cpp). The part at arena 0x100 (random); its reference (+0x48) with a
// table (slot 0 noted) linked into a first handle's list (0x400, the list head at 0x480, one other node at 0x490); the
// other handle (+0xC4 / +0xC0) at 0x500 with its list (0x580, one node 0x590); Handle::For gives a third (0x600, list
// 0x680). The member's function notes the reference it is given (its table, handle, owner as an offset, object, flag)
// and the list it is in; the reference's destroying notes its handle; giving a handle up notes it.
#if EQUIV_PART == 1
static uint32_t al_off(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0xffffffffu; }
static void *__fastcall al_handle_for(void *object) { dev_note(740, al_off(object)); return arena<void>(0x600); }
static void __fastcall al_set(void *member, void *, uint32_t *r) {
    dev_note(741, al_off(member));
    dev_note(741, r[0]); dev_note(741, al_off((void *)(uintptr_t)r[1])); dev_note(741, r[4] == (uint32_t)(uintptr_t)r);
    dev_note(741, al_off((void *)(uintptr_t)r[5])); dev_note(741, r[6] & 0xff);
    dev_note(741, al_off((void *)(uintptr_t)r[3])); dev_note(741, al_off(*(void **)(uintptr_t)r[2]) == al_off(&r[2]) ? 1 : 2);
}
static void __fastcall al_destroy(uint32_t *r) {   // (out of its list, as the real one: no place on the stack left in it)
    dev_note(742, al_off((void *)(uintptr_t)r[1]));
    uint32_t *prev = (uint32_t *)(uintptr_t)r[2], *next = (uint32_t *)(uintptr_t)r[3];
    prev[1] = r[3], next[0] = r[2];
}
static void __fastcall al_release(void *handle, void *, int32_t z) { dev_note(743, al_off(handle)); dev_note(743, (uint32_t)z); }
static void __fastcall al_changed(uint32_t *r) { dev_note(744, al_off(r)); dev_note(744, al_off((void *)(uintptr_t)r[1])); }
static void *al_table[1] = {(void *)al_changed};
static void al_list(uint32_t head, uint32_t other) {   // a list head and one other node, linked both ways
    uint32_t *h = arena<uint32_t>(head), *o = arena<uint32_t>(other);
    h[0] = h[1] = (uint32_t)(uintptr_t)o;
    o[0] = o[1] = (uint32_t)(uintptr_t)h;
}
#else
    Test animal_link{0xadfc50,
                     [=](Rng &r) {
                         fresh(r);
                         arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                         for (int k = 0; k < 0x40; k++) arena<uint32_t>(0x100)[k] = r.next();
                         al_list(0x480, 0x490), al_list(0x580, 0x590), al_list(0x680, 0x690);
                         arena<uint32_t>(0x400)[8] = (uint32_t)(uintptr_t)arena<void>(0x480), arena<uint32_t>(0x400)[2] = r.range(1, 3);
                         arena<uint32_t>(0x500)[8] = (uint32_t)(uintptr_t)arena<void>(0x580), arena<uint32_t>(0x500)[2] = r.range(1, 3);
                         arena<uint32_t>(0x600)[8] = (uint32_t)(uintptr_t)arena<void>(0x680), arena<uint32_t>(0x600)[2] = r.range(1, 3);
                         // the part's reference: its table, its handle (0x400), its node linked into that list after the head
                         uint32_t *own = arena<uint32_t>(0x100 + 0x48), *head = arena<uint32_t>(0x480), *other = arena<uint32_t>(0x490);
                         own[0] = (uint32_t)(uintptr_t)al_table, own[1] = (uint32_t)(uintptr_t)arena<void>(0x400);
                         own[2] = (uint32_t)(uintptr_t)head, own[3] = (uint32_t)(uintptr_t)other;   // (prev the head, next the other)
                         head[1] = (uint32_t)(uintptr_t)&own[2], other[0] = (uint32_t)(uintptr_t)&own[2];
                         arena<uint32_t>(0x100)[0xc4 / 4] = (uint32_t)(uintptr_t)arena<void>(0x500);
                         arena<uint32_t>(0x100)[0xc0 / 4] = (uint32_t)(uintptr_t)arena<void>(0x500);
                         arena<uint32_t>(0x50)[0] = (uint32_t)(uintptr_t)arena<void>(0x700 + r.range(0, 3) * 0x10);
                     },
                     [](void *fn) {
                         ((void(__fastcall *)(void *, void *, uint32_t))fn)(arena<void>(0x100), nullptr, arena<uint32_t>(0x50)[0]);
                         return Result{*arena<uint32_t>(0x20)};
                     }};
    animal_link.iterations = 1000;
    animal_link.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{0x805700, (void *)al_handle_for}, {e->at == 0xadfc50 ? 0xad6ef0u : 0xaeff30u, (void *)al_set}, {0x40f730, (void *)al_destroy},
                       {0x8055c0, (void *)al_release}});
    };
    animal_link.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(animal_link);
#endif
