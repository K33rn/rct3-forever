// Animal::LinkADFC50 and Animal::LinkAF5210 (one function written out twice; they differ in the reference class's table,
// the member the reference is given to and that member's function, and where the other handle is kept): link an animal
// part to an object (thiscall; the object; no value).
//   - A reference to the object made on the stack: the plain reference table, the object's handle (Handle::For) with
//     the reference joined to its list at the end, then the class's own table, the reference itself, the object and a
//     flag of 1. Given to the member's function (+0xC0 / +0xBC), then destroyed (0x40F730).
//   - The part's own reference (+0x48) moved to the handle kept at +0xC4 / +0xC0: out of its list (a broken list stops
//     at a breakpoint, as in the game, then goes on), its old handle given up (0x8055C0), the new one held once more and
//     joined at its list's end; told it changed (its table's first function).
//   - The part's transform set to the identity (+0x60 to +0xA0: the scale 1 at +0x70, +0x84, +0x98, the rest 0).
#include "rewrite.h"

namespace {
struct ListNode {
    ListNode *prev, *next;
};
struct Handle {
    char unknown[8];
    int32_t references;
    void *object;
    char unknown2[0x10];
    ListNode *refs;
};
struct ObjectRef {
    void **vtable;
    Handle *handle;
    ListNode node;
    void *owner;
    void *object;
    uint8_t flag;
};

volatile long g_animal_links;
template <uint32_t TABLE, uint32_t MEMBER, uintptr_t SET, uint32_t OTHER>
void __fastcall animal_link(uint8_t *self, void *, void *object) {
    g_animal_links++;
    ObjectRef made;
    made.vtable = (void **)0xf26ae0;
    made.owner = &made;
    Handle *h = ((Handle * (__fastcall *)(void *))0x805700)(object);
    made.handle = h;
    ListNode *head = h->refs;
    made.node.prev = head->prev;
    head->prev->next = &made.node;
    made.node.next = head;
    head->prev = &made.node;
    made.vtable = (void **)(uintptr_t)TABLE;
    made.object = object;
    made.flag = 1;
    ((void(__thiscall *)(void *, ObjectRef *))SET)(self + MEMBER, &made);
    ((void(__thiscall *)(ObjectRef *))0x40f730)(&made);

    ObjectRef *own = (ObjectRef *)(self + 0x48);
    ListNode *n = &own->node;
    if (n->next && n->prev && n->prev->next == n && n->next->prev == n) {
        n->prev->next = n->next;
        n->next->prev = n->prev;
        n->prev = nullptr;
        n->next = nullptr;
    } else {
        __builtin_debugtrap();
    }
    ((void(__fastcall *)(Handle *, void *, int32_t))0x8055c0)(own->handle, nullptr, 0);
    Handle *other = *(Handle **)(self + OTHER);
    ++other->references;
    own->handle = other;
    ListNode *oh = other->refs;
    n->prev = oh->prev;
    oh->prev->next = n;
    n->next = oh;
    oh->prev = n;
    ((void(__thiscall *)(ObjectRef *))own->vtable[0])(own);

    uint32_t *m = (uint32_t *)self;
    m[0x60 / 4] = 0, m[0x64 / 4] = 0, m[0x68 / 4] = 0, m[0x74 / 4] = 0, m[0x78 / 4] = 0, m[0x70 / 4] = 0x3f800000;
    m[0x80 / 4] = 0, m[0x88 / 4] = 0, m[0x84 / 4] = 0x3f800000, m[0x90 / 4] = 0, m[0x94 / 4] = 0, m[0x98 / 4] = 0x3f800000;
    m[0xa0 / 4] = 0;
}

const uint8_t animal_link_first[][7] = {{0x6a, 0xff, 0x68, 0xf8, 0xb7, 0xe8, 0x00}, {0x6a, 0xff, 0x68, 0x78, 0xdf, 0xe8, 0x00}};
}   // namespace
REWRITE("Animal::Link", 0xadfc50, animal_link_first[0], (&animal_link<0xf952e0, 0xc0, 0xad6ef0, 0xc4>), &g_animal_links);
REWRITE("Animal::Link", 0xaf5210, animal_link_first[1], (&animal_link<0xf96618, 0xbc, 0xaeff30, 0xc0>), &g_animal_links);
