// Small containers the compiler copied for every item type: linked lists, int arrays, recycled blocks, rows read from
// a save stream, and shared slots with defaults.
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
static void unlink(Node *n) {
    if (n && n->prev && n->next && n->next->prev == n && n->prev->next == n) {
        n->next->prev = n->prev;
        n->prev->next = n->next;
        n->next = nullptr;
        n->prev = nullptr;
    } else {
        __builtin_debugtrap();
    }
}

// ---- List::Insert: a new node before `where`; the result is {a, b, the node} -----------------------------------
static volatile long g_list_inserts;
static uint32_t *__thiscall list_insert(void *, uint32_t *result, uint32_t a, uint32_t b, Node *where, void *value) {
    g_list_inserts++;
    Node *n = (Node *)memory_alloc(sizeof(Node));
    if (n) n->value = value;
    n->prev = where->prev;
    where->prev->next = n;
    n->next = where;
    where->prev = n;
    result[0] = a;
    result[1] = b;
    result[2] = (uint32_t)(uintptr_t)((uint32_t)(uintptr_t)n == a ? where : n);
    return result;
}
// Replacing needs at least 5 known bytes, and the fourth on are the allocator call's offset, which differs by copy.
template <uint32_t AT> static const uint8_t list_insert_original[] = {
    0x6a, 0x0c, 0xe8, (uint8_t)(0xbcdfc0 - (AT + 7)), (uint8_t)((0xbcdfc0 - (AT + 7)) >> 8),
    (uint8_t)((0xbcdfc0 - (AT + 7)) >> 16), (uint8_t)((0xbcdfc0 - (AT + 7)) >> 24)};
REWRITE("List::Insert", 0x476830, list_insert_original<0x476830>, list_insert, &g_list_inserts);
REWRITE("List::Insert", 0x47cac0, list_insert_original<0x47cac0>, list_insert, &g_list_inserts);
REWRITE("List::Insert", 0x6a60b0, list_insert_original<0x6a60b0>, list_insert, &g_list_inserts);
REWRITE("List::Insert", 0xa46590, list_insert_original<0xa46590>, list_insert, &g_list_inserts);
REWRITE("List::Insert", 0xacd810, list_insert_original<0xacd810>, list_insert, &g_list_inserts);
REWRITE("List::Insert", 0xb3a990, list_insert_original<0xb3a990>, list_insert, &g_list_inserts);
REWRITE("List::Insert", 0xbbcc40, list_insert_original<0xbbcc40>, list_insert, &g_list_inserts);

// ---- List::Erase: remove and free a node (not the list's own, at +0xc); the result is {a, b, the node after it,
// or the node itself when it is `b`} -----------------------------------------------------------------------------
static volatile long g_list_erases;
static uint32_t *__thiscall list_erase(uint8_t *self, uint32_t *result, uint32_t a, Node *b, Node *n) {
    g_list_erases++;
    Node *after = n;
    if (n != (Node *)(self + 0xc)) {
        if (n != b) after = n->next;
        unlink(n);
        memory_free(n);
    }
    result[0] = a;
    result[1] = (uint32_t)(uintptr_t)b;
    result[2] = (uint32_t)(uintptr_t)after;
    return result;
}
static const uint32_t list_erase_copies[] = {0x476880, 0x47cb10, 0x6a6020, 0xbbcb30};
static const uint8_t list_erase_original[] = {0x56, 0x8b, 0x74, 0x24, 0x14, 0x83, 0xc1, 0x0c};
REWRITE_COPIES("List::Erase", list_erase_copies, list_erase_original, list_erase, &g_list_erases);

// ---- IntArray::Assign: {data, count}, replaced by a copy of another ------------------------------------------
struct IntArray {
    int32_t *data;
    int32_t count;
};
static volatile long g_int_assigns;
static IntArray *__thiscall int_array_assign(IntArray *a, const IntArray *other) {
    g_int_assigns++;
    memory_free_aligned(a->data);
    a->count = other->count;
    a->data = (int32_t *)memory_alloc_aligned((uint32_t)other->count * 4);
    for (int32_t i = 0; i < a->count; i++) a->data[i] = other->data[i];
    return a;
}
static const uint32_t int_assign_copies[] = {0x423fb0, 0x4b4a90, 0x63ee80, 0x6d96f0, 0x7337d0, 0x733880, 0xa5b5e0, 0xb99500};
static const uint8_t int_assign_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x06, 0x57, 0x50, 0xe8};
REWRITE_COPIES("IntArray::Assign", int_assign_copies, int_assign_original, int_array_assign, &g_int_assigns);

// ---- BlockPool::Return: an item given back to pool `index` (0 is no pool); once all of a pool's items are back
// its blocks and list are freed -----------------------------------------------------------------------------------
struct BlockPool {
    int32_t per_block;
    void **blocks;
    int32_t block_count;
    void **returned;
    int32_t returned_count;
};
static volatile long g_pool_returns;
static void __thiscall block_pool_return(BlockPool **pools, int32_t index, void *item) {
    g_pool_returns++;
    if (!index) return;
    BlockPool *p = *pools + index;
    p->returned[p->returned_count] = item;
    p->returned_count++;
    if (p->returned_count < (int32_t)((uint32_t)p->per_block * (uint32_t)p->block_count)) return;
    for (int32_t i = 0; i < p->block_count; i++) memory_free_aligned(p->blocks[i]);
    memory_free_aligned(p->blocks);
    memory_free_aligned(p->returned);
    p->blocks = nullptr;
    p->block_count = 0;
    p->returned = nullptr;
    p->returned_count = 0;
}
static const uint32_t pool_return_copies[] = {0x80f990, 0x87be60, 0x909dc0, 0x90a0d0, 0xce7860};
static const uint8_t pool_return_original[] = {0x8b, 0x44, 0x24, 0x04, 0x53, 0x33, 0xdb, 0x3b};
REWRITE_COPIES("BlockPool::Return", pool_return_copies, pool_return_original, block_pool_return, &g_pool_returns);

// ---- SharedSlots::Set: slots of shared values {references, kind}, each with a default in a table. Null restores the
// default; a value of another kind than the default's is converted first (a failed conversion changes nothing) ------
struct Shared {
    uint16_t references;
    uint16_t kind;
};
struct SharedSlots {
    void *unknown;
    Shared **defaults;   // +4
    Shared *slots[1];    // +8
};
static void shared_release(Shared *s) {
    if (!s || --s->references) return;
    ((void(__fastcall *)(Shared *))0xc09bf0)(s);
    uint8_t *pool = *(uint8_t **)0x13e3318 + 4;
    ((void(__thiscall *)(void *, Shared *))0xca1f80)(pool, s);
}
static volatile long g_shared_sets;
static void __thiscall shared_set(SharedSlots *self, uint32_t index, Shared *value) {
    g_shared_sets++;
    if (!value) {
        shared_release(self->slots[index]);
        Shared *fallback = self->defaults[index];
        self->slots[index] = fallback;
        if (fallback) fallback->references++;
        return;
    }
    uint16_t kind = self->defaults[index]->kind;
    Shared *now = value;
    if (kind == value->kind) {
        value->references++;
    } else {
        // the kind goes in the low half of the index's stack slot, which is reused as the argument
        now = ((Shared * (__thiscall *)(Shared *, uint32_t))0xc0cff0)(value, (index & 0xffff0000) | kind);
        if (!now) return;
    }
    shared_release(self->slots[index]);
    self->slots[index] = now;
}
static const uint32_t shared_set_copies[] = {0xc0d780, 0xc0d880, 0xc0da10, 0xc0db10};
static const uint8_t shared_set_original[] = {0x53, 0x56, 0x57, 0x8b, 0xf9, 0x8b, 0x4c, 0x24};
REWRITE_COPIES("SharedSlots::Set", shared_set_copies, shared_set_original, shared_set, &g_shared_sets);

// ---- Rows::Load: rows from a save stream. A size (unused) and a row count, then each row's fields by their loaders
// (virtual slot 1 of each, at +0xc, count +0x10). The row store lives at an offset in the object's record -----------
struct Stream {
    void *vtable;
    int32_t position;   // +4
    int32_t start;      // +8: the buffer holds this much of the stream
    int32_t end;        // +0xc
    char unknown[4];
    uint8_t buffer[1];  // +0x14
};
static void stream_read(Stream *s, int32_t *into) {
    int32_t at = s->position;
    if (at >= s->start && at + 4 <= s->end) {
        *into = *(int32_t *)(s->buffer + (at - s->start));
        s->position = at + 4;
    } else {
        ((void(__thiscall *)(Stream *, int32_t *, int32_t))0x809210)(s, into, 4);
    }
}
static volatile long g_rows_loads;
template <uint32_t RESIZE, uint32_t SHIFT>
static void __thiscall rows_load(uint8_t *self, int32_t offset, Stream *s, uint32_t context) {
    g_rows_loads++;
    uint8_t *store = (uint8_t *)(uintptr_t)(*(uint32_t *)(*(uint8_t **)(self + 4) + 0x28) + (uint32_t)offset);
    int32_t size, count = offset;   // a short read leaves what was there
    stream_read(s, &size);
    stream_read(s, &count);
    ((void(__thiscall *)(void *, int32_t))RESIZE)(store, count);
    for (int32_t row = 0; row < count; row++)
        for (int32_t i = 0; i < *(int32_t *)(self + 0x10); i++) {
            void *field = (*(void ***)(self + 0xc))[i];
            uint8_t *data = *(uint8_t **)store + ((uint32_t)row << SHIFT);
            ((void(__thiscall *)(void *, void *, Stream *, uint32_t))(*(void ***)field)[1])(field, data, s, context);
        }
}
static const uint8_t rows_load_original[] = {0x83, 0xec, 0x08, 0x53, 0x55, 0x56, 0x8b, 0xd9};
REWRITE("Rows::Load", 0x44f150, rows_load_original, (&rows_load<0x44e080, 5>), &g_rows_loads);
REWRITE("Rows::Load", 0x54ad80, rows_load_original, (&rows_load<0x417a20, 4>), &g_rows_loads);
REWRITE("Rows::Load", 0x76b790, rows_load_original, (&rows_load<0x76ad90, 5>), &g_rows_loads);
REWRITE("Rows::Load", 0x9935e0, rows_load_original, (&rows_load<0x991910, 5>), &g_rows_loads);

// ---- Cached interfaces -------------------------------------------------------------------------------------------
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
static void *find_interface(uint8_t *handle, const int32_t *id) {
    void *object = *(void **)(handle + 0xc);
    if (!object) return nullptr;
    ClassInterfaces *c = ((ClassInterfaces * (__fastcall *)(void *))(*(void ***)object)[1])(object);
    if (!c || *id >= c->count || !c->by_id[*id]) return nullptr;
    return (uint8_t *)object + c->by_id[*id]->offset;
}

// ---- List::DrainOwners: empty a list of owned records. Each holds a reference at +4 whose interface (kept at +0x14,
// flag +0x18) is destroyed, then the record itself ------------------------------------------------------------------
static volatile long g_list_drains;
template <uint32_t ID, uint32_t DESTROY> static void __thiscall list_drain_owners(void *, Node **list) {
    g_list_drains++;
    for (;;) {
        Node *n = *list;
        if (!n->next) return;
        unlink(n);
        uint8_t *record = (uint8_t *)n->value;
        if (!record[0x18]) {
            *(void **)(record + 0x14) = find_interface(*(uint8_t **)(record + 4), (const int32_t *)ID);
            record[0x18] = 1;
        }
        void *interface = *(void **)(record + 0x14);
        if (interface) ((void(__thiscall *)(void *, int32_t))(*(void ***)interface)[0])(interface, 1);
        void *owned = n->value;
        if (owned) {
            ((void(__fastcall *)(void *))DESTROY)(owned);
            memory_free(owned);
        }
        memory_free(n);
    }
}
static const uint8_t list_drain_original[] = {0x55, 0x56, 0x57, 0x8b, 0x44, 0x24, 0x10, 0x8b};
REWRITE("List::DrainOwners", 0x551390, list_drain_original, (&list_drain_owners<0x12f5264, 0x54e110>), &g_list_drains);
REWRITE("List::DrainOwners", 0x551460, list_drain_original, (&list_drain_owners<0x12f52bc, 0x54e120>), &g_list_drains);
REWRITE("List::DrainOwners", 0x551530, list_drain_original, (&list_drain_owners<0x12f5284, 0x54e130>), &g_list_drains);
REWRITE("List::DrainOwners", 0x551600, list_drain_original, (&list_drain_owners<0x12f5294, 0x54e140>), &g_list_drains);

// ---- Notify::Pass: pass a two-argument event on: the object's own handler, the interface part at +0x40 (reached
// through the reference at +0x160, kept at +0x170, flag +0x174) and the two listeners at +0x2b4 and +0x2b8 -------------
static volatile long g_notifies;
template <uint32_t OWN, uint32_t SLOT, uint32_t LISTENER>
static void __thiscall notify_pass(uint8_t *self, uint32_t a, uint32_t b) {
    g_notifies++;
    typedef void(__thiscall * Event)(void *, uint32_t, uint32_t);
    ((Event)OWN)(self, a, b);
    if (!self[0x174]) {
        *(void **)(self + 0x170) = find_interface(*(uint8_t **)(self + 0x160), (const int32_t *)0x12f561c);
        self[0x174] = 1;
    }
    uint8_t *interface = *(uint8_t **)(self + 0x170);
    if (interface) {
        void *part = interface + 0x40;
        ((Event)(*(void ***)part)[SLOT])(part, a, b);
    }
    void *listener = *(void **)(self + 0x2b4);
    if (listener) ((Event)LISTENER)(listener, a, b);
    listener = *(void **)(self + 0x2b8);
    if (listener) ((Event)LISTENER)(listener, a, b);
}
static const uint8_t notify_original[] = {0x53, 0x8b, 0x5c, 0x24, 0x08, 0x55, 0x8b, 0x6c};
REWRITE("Notify::Pass", 0x5c0a30, notify_original, (&notify_pass<0x848840, 1, 0x86cfb0>), &g_notifies);
REWRITE("Notify::Pass", 0x5c0b30, notify_original, (&notify_pass<0x8489b0, 2, 0x86d000>), &g_notifies);
REWRITE("Notify::Pass", 0x5c0c30, notify_original, (&notify_pass<0x848b20, 3, 0x86d050>), &g_notifies);
REWRITE("Notify::Pass", 0x5c0d30, notify_original, (&notify_pass<0x848c90, 4, 0x86d0a0>), &g_notifies);
