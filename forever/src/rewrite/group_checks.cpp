// GuestGroup::Check (two copies; they differ in the check they run): a check run on the park's shared object, and when
// it passes, the group's two references pointed at new handles (thiscall: one value; ret 4; gives whether it passed,
// in al).
//   - The shared object fetched (0x757F50, made the first time); its check (the copy's) given the group's part at +0x40
//     (none for no group), the value, the value's place and a place on the stack (which starts out holding the group's
//     own address: the slot the compiler reserved with push ecx). It failing: false.
//   - +0x7C = 11, +0x80 = 1.
//   - The reference at +0x84: taken out of its handle's list (checked: a break when the list is not sound), its handle
//     given up (0x8055C0 with 0), the group's handle at +0x50 taken (its count at +8 raised), linked into that handle's
//     list, and told (its table's first entry).
//   - The reference at +0x98 the same way, with the handle for the value as it is now (0x805700; the check may have
//     changed it).
//   - +0xAC = what the check left in the place on the stack; true.
#include "rewrite.h"

namespace {
volatile long g_group_checks;
struct Node {
    Node *a;   // (the node before: its b is this one)
    Node *b;   // (the node after: its a is this one)
};
struct Reference {
    void **table;
    uint8_t *handle;
    Node node;
};
void unlink(Node *n) {
    if (n->b && n->a && n->a->b == n && n->b->a == n) {
        n->a->b = n->b;
        n->b->a = n->a;
        n->a = nullptr;
        n->b = nullptr;
    } else {
        __builtin_debugtrap();
    }
}
void link(Reference *r, uint8_t *handle) {
    r->handle = handle;
    Node *head = *(Node **)(handle + 0x20);
    r->node.a = head->a;
    head->a->b = &r->node;
    r->node.b = head;
    head->a = &r->node;
    ((void(__thiscall *)(Reference *))r->table[0])(r);
}
void give_up(Reference *r) {
    unlink(&r->node);
    ((void(__thiscall *)(uint8_t *, uint32_t))0x8055c0)(r->handle, 0);
}

template <uint32_t CHECK> bool __fastcall group_check(uint8_t *self, void *, uint32_t value) {
    g_group_checks++;
    uint32_t left = (uint32_t)(uintptr_t)self;
    void *shared = ((void *(*)())0x757f50)();
    using Check = bool(__thiscall *)(void *, void *, uint32_t, uint32_t *, uint32_t *);
    if (!((Check)CHECK)(shared, self ? self + 0x40 : nullptr, value, &value, &left)) return false;
    *(uint32_t *)(self + 0x7c) = 11;
    *(uint32_t *)(self + 0x80) = 1;
    Reference *first = (Reference *)(self + 0x84);
    give_up(first);
    uint8_t *handle = *(uint8_t **)(self + 0x50);
    ++*(int32_t *)(handle + 8);
    link(first, handle);
    Reference *second = (Reference *)(self + 0x98);
    give_up(second);
    link(second, ((uint8_t * (__fastcall *)(uint32_t))0x805700)(*(volatile uint32_t *)&value));
    *(uint32_t *)(self + 0xac) = left;
    return true;
}

const uint8_t group_check_first[][7] = {{0x51, 0x53, 0x8b, 0xd9, 0x85, 0xdb, 0x74}};
}   // namespace
REWRITE("GuestGroup::Check", 0x778880, group_check_first[0], &group_check<0x76ffa0>, &g_group_checks);
REWRITE("GuestGroup::Check", 0x7789c0, group_check_first[0], &group_check<0x76ff60>, &g_group_checks);
