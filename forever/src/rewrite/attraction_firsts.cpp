// Attraction::First (two copies; they differ in the global that picks the kind of part looked for): the attraction
// taken to the first object that has that part and passes its two checks (thiscall, nothing else; gives whether none
// failed, in al).
//   - The objects walked in a list (the list at 0x12F4E4C read once; 0x4CCE80 on it gives the one after a given one,
//     the first after none). For each: its type (its table's second entry); when there is one and the global is below
//     its count of parts (signed), the part at that index (its offset, +0x10, added to the object), when there is one
//     and the sum isn't 0:
//       - the first check (0x7AAD50, given the object): 0 gives false at once, 2 goes on to the next object;
//       - else the second (0x7ABEF0, given the part): 0 gives false, 2 goes on to the next; anything else takes it.
//   - Taken: the attraction's reference (+0x1BC: its table, handle, list node) taken out of its handle's list (checked:
//     a break when the list is not sound), the handle given up (0x8055C0 with 0), the object's handle (0x805700) put and
//     the node linked into its list, the reference told (its table's first entry); +0x2B8 = the object asked for a value
//     (0x4BF1F0, given a copy of the attraction's place, +0x80..+0x88, and 1), +0x2DC = 0, a step started (0x7CD220
//     with 11 and 0); true.
//   - No object taken: true.
#include "rewrite.h"

namespace {
volatile long g_attraction_firsts;
struct Node {
    Node *a;   // (the node before: its b is this one)
    Node *b;   // (the node after: its a is this one)
};
struct Reference {
    void **table;
    uint8_t *handle;
    Node node;
};
struct Type {
    uint8_t pad[0x14];
    uint8_t **parts;
    int32_t count;
};

void take(uint8_t *self, uint8_t *object) {
    Reference *r = (Reference *)(self + 0x1bc);
    Node *n = &r->node;
    if (n->b && n->a && n->a->b == n && n->b->a == n) {
        n->a->b = n->b;
        n->b->a = n->a;
        n->a = nullptr;
        n->b = nullptr;
    } else {
        __builtin_debugtrap();
    }
    ((void(__thiscall *)(uint8_t *, uint32_t))0x8055c0)(r->handle, 0);
    uint8_t *handle = ((uint8_t * (__fastcall *)(uint8_t *))0x805700)(object);
    r->handle = handle;
    Node *head = *(Node **)(handle + 0x20);
    n->a = head->a;
    head->a->b = n;
    n->b = head;
    head->a = n;
    ((void(__thiscall *)(Reference *))r->table[0])(r);
    alignas(16) uint32_t place[3] = {*(uint32_t *)(self + 0x80), *(uint32_t *)(self + 0x84), *(uint32_t *)(self + 0x88)};
    *(uint32_t *)(self + 0x2b8) = ((uint32_t(__thiscall *)(uint8_t *, uint32_t *, uint32_t))0x4bf1f0)(object, place, 1);
    *(uint32_t *)(self + 0x2dc) = 0;
    ((void(__thiscall *)(uint8_t *, uint32_t, uint32_t))0x7cd220)(self, 11, 0);
}

template <uint32_t KIND> bool __fastcall attraction_first(uint8_t *self) {
    g_attraction_firsts++;
    void *list = *(void **)0x12f4e4c;
    using Next = uint8_t *(__thiscall *)(void *, uint8_t *);
    for (uint8_t *object = ((Next)0x4cce80)(list, nullptr); object; object = ((Next)0x4cce80)(list, object)) {
        Type *type = ((Type * (__thiscall *)(uint8_t *))(*(void ***)object)[1])(object);
        int32_t kind = *(volatile int32_t *)(uintptr_t)KIND;
        if (!type || kind >= type->count) continue;
        uint8_t *part = type->parts[kind];
        if (!part) continue;
        uintptr_t at = (uintptr_t)object + *(uint32_t *)(part + 0x10);   // (an integer sum: its being 0 is tested)
        if (!at) continue;
        uint32_t first = ((uint32_t(__thiscall *)(uint8_t *, uint8_t *))0x7aad50)(self, object);
        if (first == 0) return false;
        if (first == 2) continue;
        uint32_t second = ((uint32_t(__thiscall *)(uint8_t *, uint8_t *))0x7abef0)(self, (uint8_t *)at);
        if (second == 0) return false;
        if (second == 2) continue;
        take(self, object);
        return true;
    }
    return true;
}

const uint8_t attraction_first_first[][7] = {{0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83}};
}   // namespace
REWRITE("Attraction::First", 0x7ad080, attraction_first_first[0], &attraction_first<0x13cde74>, &g_attraction_firsts);
REWRITE("Attraction::First", 0x7ad1f0, attraction_first_first[0], &attraction_first<0x13d0680>, &g_attraction_firsts);
