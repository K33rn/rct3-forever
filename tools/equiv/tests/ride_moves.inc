// RidePart::TakeOver (forever/src/rewrite/ride_moves.cpp). This part at arena 0x100, the other at 0x300 (random
// fields). The reference assignment's stand-in notes what it gets (a reference on the stack by its fields, as
// ride_checks.inc's); Handle::For gives 0x600 and the empty handle is 0x700 (each with a list); the destruction of the
// stack reference takes it out of its list. The +0x50 references: this one's in handle 0x800's list, the other's in
// handle 0x880's (its count at +8); their table's slot 0 notes them. The other's list (head at +0x38): 0-3 nodes at
// 0xA00.. then a last node with no next; this one's list head at [+0x48] = 0xB00 with one node.
#if EQUIV_PART == 1
static uint32_t rm_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0; }
static bool rm_in_arena(const void *p) { return (uintptr_t)p >= (uintptr_t)arena<void>(0) && (uintptr_t)p < (uintptr_t)arena<void>(0x10000); }
static void __thiscall rm_assign(uint8_t *to, uint32_t *from) {
    dev_note(660, rm_at(to));
    if (rm_in_arena(from)) {
        dev_note(660, rm_at(from));
    } else {   // (a reference made on the stack: told by its fields)
        dev_note(661, from[0]); dev_note(661, rm_at((void *)(uintptr_t)from[1])); dev_note(661, from[5]); dev_note(661, (uint8_t)from[6]);
        dev_note(661, from[4] == (uint32_t)(uintptr_t)from);
        dev_note(661, *(uint32_t *)(uintptr_t)from[3] == (uint32_t)(uintptr_t)&from[2]);
    }
}
static void __fastcall rm_reference_gone(uint32_t *ref) {
    uint32_t *next = (uint32_t *)(uintptr_t)ref[2], *head = (uint32_t *)(uintptr_t)ref[3];
    head[0] = (uint32_t)(uintptr_t)next, next[1] = (uint32_t)(uintptr_t)head;   // (left again)
    dev_note(662, ref[0]);
}
static void *__fastcall rm_handle_of(void *object) { dev_note(663, rm_at(object)); return arena<void>(0x600); }
static void *__cdecl rm_empty_handle() { dev_note(664, 1); return arena<void>(0x700); }
static void __thiscall rm_release(void *handle, uint32_t zero) { dev_note(665, rm_at(handle)); dev_note(665, zero); }
static void __fastcall rm_told(void *reference) { dev_note(666, rm_at(reference)); }
static void *rm_table[1] = {(void *)rm_told};
static void rm_list(uint32_t at) {   // a handle's list: its head (at +0x80) with one node
    arena<uint32_t>(at)[0x20 / 4] = (uint32_t)(uintptr_t)arena<void>(at + 0x40);
    arena<uint32_t>(at + 0x40)[0] = (uint32_t)(uintptr_t)arena<void>(at + 0x50);
    arena<uint32_t>(at + 0x50)[1] = (uint32_t)(uintptr_t)arena<void>(at + 0x40);
}
static void rm_link(uint8_t *link, uint32_t handle) {   // a reference's link put first in a handle's list
    uint32_t *l = (uint32_t *)link, *head = (uint32_t *)(uintptr_t)arena<uint32_t>(handle)[0x20 / 4];
    l[0] = head[0], ((uint32_t *)(uintptr_t)head[0])[1] = (uint32_t)(uintptr_t)l, l[1] = (uint32_t)(uintptr_t)head, head[0] = (uint32_t)(uintptr_t)l;
}
#else
    Test ride_moves{0x55a6a0,
                    [=](Rng &r) {
                        fresh(r);
                        arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next();
                        for (uint32_t h : {0x600u, 0x700u, 0x800u, 0x880u}) rm_list(h);
                        arena<uint32_t>(0x880)[2] = r.range(1, 5);
                        for (uint32_t part : {0x100u, 0x300u}) {
                            uint8_t *p = arena<uint8_t>(part);
                            for (uint32_t off = 0x60; off < 0x90; off += 4) *(uint32_t *)(p + off) = r.next();   // (its values: random, so where each lands shows)
                            *(void **)(p + 0x50) = rm_table;
                            *(void **)(p + 0x54) = arena<void>(part == 0x100 ? 0x800 : 0x880);
                            rm_link(p + 0x58, part == 0x100 ? 0x800 : 0x880);
                        }
                        uint8_t *other = arena<uint8_t>(0x300);
                        const int nodes = r.range(0, 3);
                        uint32_t *prev = (uint32_t *)(other + 0x38);
                        for (int i = 0; i <= nodes; i++) {   // (the last one has no next)
                            uint32_t *node = arena<uint32_t>(0xa00 + 0x10 * i);
                            prev[0] = (uint32_t)(uintptr_t)node, node[1] = (uint32_t)(uintptr_t)prev, node[0] = 0;
                            prev = node;
                        }
                        *(void **)(arena<uint8_t>(0x100) + 0x48) = arena<void>(0xb00);
                        arena<uint32_t>(0xb00)[0] = (uint32_t)(uintptr_t)arena<void>(0xb40);
                        arena<uint32_t>(0xb40)[1] = (uint32_t)(uintptr_t)arena<void>(0xb00);
                    },
                    [](void *fn) {
                        void *back = ((void *(__fastcall *)(void *, void *, void *))fn)(arena<void>(0x100), nullptr, arena<void>(0x300));
                        return Result{rm_at(back) ^ *arena<uint32_t>(0x20)};
                    }};
    ride_moves.iterations = 500;
    ride_moves.prepare = [](const rewrite::Entry *) {
        stubs_for_now({{0x4aa7e0, (void *)rm_assign}, {0x40f730, (void *)rm_reference_gone}, {0x805700, (void *)rm_handle_of}, {0x805750, (void *)rm_empty_handle},
                       {0x8055c0, (void *)rm_release}});
    };
    ride_moves.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(ride_moves);
#endif
