// Helpers shared by the wave*.cpp rewrites: lists, cached interfaces, and the game's memory functions.
#pragma once
#include "rewrite.h"

static void *(__cdecl *const memory_alloc)(uint32_t) = (void *(__cdecl *)(uint32_t))0xbcdfc0;
static void *(__cdecl *const memory_alloc_aligned)(uint32_t) = (void *(__cdecl *)(uint32_t))0xbcdfd0;
static void(__cdecl *const memory_free)(void *) = (void(__cdecl *)(void *))0xbce000;
static void(__cdecl *const memory_free_aligned)(void *) = (void(__cdecl *)(void *))0xbce010;

struct Node {
    Node *next;
    Node *prev;
    void *value;
};

// Take a node out of its list (a broken list stops at a breakpoint, as in the game).
static inline void unlink(Node *n) {
    if (n && n->prev && n->next && n->next->prev == n && n->prev->next == n) {
        n->next->prev = n->prev;
        n->prev->next = n->next;
        n->next = nullptr;
        n->prev = nullptr;
    } else {
        __builtin_debugtrap();
    }
}

struct InterfaceEntry {
    char unknown[0x10];
    int32_t offset;
};
struct ClassInterfaces {
    char unknown[0x14];
    InterfaceEntry **by_id;   // +0x14
    int32_t count;            // +0x18
};
// The interface of a handle's object (+0xc), looked up by the id in the global at `id`.
static inline void *find_interface(uint8_t *handle, const int32_t *id) {
    void *object = *(void **)(handle + 0xc);
    if (!object) return nullptr;
    ClassInterfaces *c = ((ClassInterfaces * (__fastcall *)(void *))(*(void ***)object)[1])(object);
    if (!c || *id >= c->count || !c->by_id[*id]) return nullptr;
    return (uint8_t *)object + c->by_id[*id]->offset;
}
// The interface kept in a 28-byte reference {+4 handle, +0x14 kept, +0x18 flag}, looked up the first time.
template <uint32_t ID> static inline void *ref_interface(uint8_t *ref) {
    if (!ref[0x18]) {
        *(void **)(ref + 0x14) = find_interface(*(uint8_t **)(ref + 4), (const int32_t *)ID);
        ref[0x18] = 1;
    }
    return *(void **)(ref + 0x14);
}
