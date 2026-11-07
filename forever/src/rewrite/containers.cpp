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

// ---- Rows::Save: the opposite of Rows::Load: the size and row count, each added to the stream's checksum, then each
// row's fields by their savers (virtual slot 3 of each, at +0xc, count +0x18). STRIDE is the row's size ---------------
static void stream_write_counted(uint8_t *s, int32_t *value) {
    ((void(__thiscall *)(void *, void *, int32_t))0xbe4060)(s, value, 4);
    for (int i = 0; i < 4; i++) {
        uint16_t check = *(uint16_t *)(s + 0x14);
        uint8_t coded = (uint8_t)(check >> 8) ^ ((uint8_t *)value)[i];
        *(uint16_t *)(s + 0x14) = (uint16_t)(0x58bf - (uint16_t)(coded + check) * 0x3193);
        *(uint32_t *)(s + 0x10) += coded;
    }
}
static volatile long g_rows_saves;
template <uint32_t STRIDE> static void __thiscall rows_save(uint8_t *self, int32_t offset, uint8_t *s) {
    g_rows_saves++;
    uint8_t *store = (uint8_t *)(uintptr_t)(*(uint32_t *)(self + 0x28) + (uint32_t)offset);
    int32_t count = ((int32_t *)store)[1];
    int32_t size = ((int32_t(__thiscall *)(void *, int32_t))(*(void ***)self)[2])(self, offset) - 4;
    stream_write_counted(s, &size);
    stream_write_counted(s, &count);
    for (int32_t row = 0; row < count; row++) {
        uint8_t *data = *(uint8_t **)store + (uint32_t)row * STRIDE;
        for (int32_t i = 0; i < *(int32_t *)(self + 0x18); i++) {
            void *field = (*(void ***)(self + 0xc))[i];
            ((void(__thiscall *)(void *, void *, void *))(*(void ***)field)[3])(field, data, s);
        }
    }
}
static const uint8_t rows_save_original[] = {0x83, 0xec, 0x0c, 0x8b, 0x44, 0x24, 0x10, 0x53};
REWRITE("Rows::Save", 0x533460, rows_save_original, (&rows_save<0x90>), &g_rows_saves);
REWRITE("Rows::Save", 0x55bcd0, rows_save_original, (&rows_save<0xe0>), &g_rows_saves);
REWRITE("Rows::Save", 0x9c8ec0, rows_save_original, (&rows_save<0x90>), &g_rows_saves);
REWRITE("Rows::Save", 0xb9ff30, rows_save_original, (&rows_save<0xb8>), &g_rows_saves);

// ---- Items::AnyMatch: whether any item (+0xe4, count +0xe8), looked up through the interface kept at +0x64 (flag +0x68,
// reference +0x54), passes CHECK ---------------------------------------------------------------------------------------
static volatile long g_any_matches;
template <uint32_t CHECK> static bool __thiscall any_match(uint8_t *self, uint32_t arg) {
    g_any_matches++;
    for (int32_t i = 0; i < *(int32_t *)(self + 0xe8); i++) {
        if (!self[0x68]) {
            *(void **)(self + 0x64) = find_interface(*(uint8_t **)(self + 0x54), (const int32_t *)0x12f54e0);
            self[0x68] = 1;
        }
        void *table = ((void *(__fastcall *)(void *))0x59acc0)(*(void **)(self + 0x64));
        uint32_t item = (*(uint32_t **)(self + 0xe4))[i];
        void *found = ((void *(__thiscall *)(void *, uint32_t, uint32_t))0x5a7300)(table, item, arg);
        if (((uint8_t(__fastcall *)(void *))CHECK)(found)) return true;
    }
    return false;
}
static const uint8_t any_match_original[] = {0x53, 0x55, 0x56, 0x8b, 0xf1, 0x8b, 0x86, 0xe8};
REWRITE("Items::AnyMatch", 0x5d68e0, any_match_original, (&any_match<0x5639c0>), &g_any_matches);
REWRITE("Items::AnyMatch", 0x5d6980, any_match_original, (&any_match<0x563a30>), &g_any_matches);
REWRITE("Items::AnyMatch", 0x5d6a20, any_match_original, (&any_match<0x563b30>), &g_any_matches);
REWRITE("Items::AnyMatch", 0x5d6c00, any_match_original, (&any_match<0x563c50>), &g_any_matches);
REWRITE("Items::AnyMatch", 0x5d6ca0, any_match_original, (&any_match<0x563cc0>), &g_any_matches);
REWRITE("Items::AnyMatch", 0x5d6d40, any_match_original, (&any_match<0x563da0>), &g_any_matches);

// ---- Items::EachInterface: pass two arguments to the interface of each 28-byte reference (+0xc0, count +0xc4; each with
// its interface kept at +0x14, flag +0x18) -----------------------------------------------------------------------------
static volatile long g_each_interfaces;
template <uint32_t CALL> static void __thiscall each_interface(uint8_t *self, uint32_t a, uint32_t b) {
    g_each_interfaces++;
    for (int32_t i = 0; i < *(int32_t *)(self + 0xc4); i++) {
        uint8_t *ref = *(uint8_t **)(self + 0xc0) + i * 0x1c;
        if (!ref[0x18]) {
            *(void **)(ref + 0x14) = find_interface(*(uint8_t **)(ref + 4), (const int32_t *)0x12f50d4);
            ref[0x18] = 1;
        }
        ((void(__thiscall *)(void *, uint32_t, uint32_t))CALL)(*(void **)(ref + 0x14), a, b);
    }
}
static const uint8_t each_interface_original[] = {0x51, 0x53, 0x55, 0x8b, 0xe9, 0x8b, 0x85, 0xc4};
REWRITE("Items::EachInterface", 0x563420, each_interface_original, (&each_interface<0x511d40>), &g_each_interfaces);
REWRITE("Items::EachInterface", 0x5634c0, each_interface_original, (&each_interface<0x511950>), &g_each_interfaces);
REWRITE("Items::EachInterface", 0x563560, each_interface_original, (&each_interface<0x511aa0>), &g_each_interfaces);
REWRITE("Items::EachInterface", 0x563600, each_interface_original, (&each_interface<0x511bf0>), &g_each_interfaces);

// ---- Compare::Mapped: two values (+0xa4), each mapped by a lookup using the object's kind (+0x8d) and a side (the second
// object takes the other side) with a step per copy; true when both map and the results agree ---------------------------
static volatile long g_mapped_compares;
template <int32_t STEP_A, int32_t STEP_B> static uint8_t __stdcall mapped_equal(uint8_t *a, uint8_t *b, uint8_t side) {
    g_mapped_compares++;
    typedef uint8_t(__fastcall * Lookup)(uint8_t, uint8_t, int32_t *, int32_t);
    const Lookup lookup = (Lookup)0x71b0f0;
    int32_t x = *(int32_t *)(a + 0xa4), y = *(int32_t *)(b + 0xa4);
    if (!lookup(a[0x8d], side, &x, STEP_A)) return 0;
    if (!lookup(b[0x8d], side ^ 1, &y, STEP_B)) return 0;
    return x == y;
}
static const uint8_t mapped_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0x88, 0xa4, 0x00};
REWRITE("Compare::Mapped", 0x71d1e0, mapped_original, (&mapped_equal<2, 2>), &g_mapped_compares);
REWRITE("Compare::Mapped", 0x71d250, mapped_original, (&mapped_equal<2, -2>), &g_mapped_compares);
REWRITE("Compare::Mapped", 0x71d2c0, mapped_original, (&mapped_equal<-2, -2>), &g_mapped_compares);
REWRITE("Compare::Mapped", 0x71d520, mapped_original, (&mapped_equal<2, 1>), &g_mapped_compares);
REWRITE("Compare::Mapped", 0x71d590, mapped_original, (&mapped_equal<2, -1>), &g_mapped_compares);
REWRITE("Compare::Mapped", 0x71d600, mapped_original, (&mapped_equal<-2, -1>), &g_mapped_compares);
REWRITE("Compare::Mapped", 0x71d670, mapped_original, (&mapped_equal<-2, 1>), &g_mapped_compares);

// ---- List::FindNamed: the first item of the list at +OFF whose name (+4) matches; null for no name -----------------
static volatile long g_named_finds;
template <uint32_t OFF> static void *__thiscall find_named(uint8_t *self, const char *name) {
    g_named_finds++;
    if (!name || !*name) return nullptr;
    for (Node *n = *(Node **)(self + OFF); n->next; n = n->next) {
        void *item = n->value;
        if (!((int(__cdecl *)(const char *, const char *))0xde22df)(((const char **)item)[1], name)) return item;
    }
    return nullptr;
}
static const uint8_t find_named_original[] = {0x53, 0x8b, 0x5c, 0x24, 0x08, 0x85, 0xdb, 0x74};
REWRITE("List::FindNamed", 0xc0ccf0, find_named_original, (&find_named<0xac>), &g_named_finds);
REWRITE("List::FindNamed", 0xc0cd50, find_named_original, (&find_named<0xc4>), &g_named_finds);
REWRITE("List::FindNamed", 0xc0cdb0, find_named_original, (&find_named<0xdc>), &g_named_finds);
REWRITE("List::FindNamed", 0xc0ce10, find_named_original, (&find_named<0xf4>), &g_named_finds);
REWRITE("List::FindNamed", 0xc0ce70, find_named_original, (&find_named<0x10c>), &g_named_finds);
REWRITE("List::FindNamed", 0xc0ced0, find_named_original, (&find_named<0x124>), &g_named_finds);
REWRITE("List::FindNamed", 0xc0cf30, find_named_original, (&find_named<0x13c>), &g_named_finds);
REWRITE("List::FindNamed", 0xc0cf90, find_named_original, (&find_named<0x154>), &g_named_finds);

// ---- List::LeaveIfLinked: a node in a sound list leaves it through REMOVE and is cleared; otherwise nothing ------------
static volatile long g_leaves;
template <uint32_t REMOVE> static void __fastcall leave_if_linked(Node *n) {
    g_leaves++;
    if (n && n->prev && n->next && n->next->prev == n && n->prev->next == n) {
        ((void(__fastcall *)(Node *))REMOVE)(n);
        n->next = nullptr;
        n->prev = nullptr;
    }
}
static const uint8_t leave_original[] = {0x56, 0x8b, 0xf1, 0x85, 0xf6, 0x74, 0x2a, 0x8b};
REWRITE("List::LeaveIfLinked", 0xc0dd30, leave_original, (&leave_if_linked<0xc0dd70>), &g_leaves);
REWRITE("List::LeaveIfLinked", 0xc0de70, leave_original, (&leave_if_linked<0xc0deb0>), &g_leaves);
REWRITE("List::LeaveIfLinked", 0xc0df90, leave_original, (&leave_if_linked<0xc0dfd0>), &g_leaves);
REWRITE("List::LeaveIfLinked", 0xc0e340, leave_original, (&leave_if_linked<0xc0e380>), &g_leaves);
REWRITE("List::LeaveIfLinked", 0xc0e650, leave_original, (&leave_if_linked<0xc0e690>), &g_leaves);
REWRITE("List::LeaveIfLinked", 0xc0e9a0, leave_original, (&leave_if_linked<0xc0e9e0>), &g_leaves);
REWRITE("List::LeaveIfLinked", 0xc0ed70, leave_original, (&leave_if_linked<0xc0edb0>), &g_leaves);
REWRITE("List::LeaveIfLinked", 0xc0f090, leave_original, (&leave_if_linked<0xc0f0d0>), &g_leaves);
REWRITE("List::LeaveIfLinked", 0xc0f350, leave_original, (&leave_if_linked<0xc0f390>), &g_leaves);
REWRITE("List::LeaveIfLinked", 0xc0f550, leave_original, (&leave_if_linked<0xc0f590>), &g_leaves);
REWRITE("List::LeaveIfLinked", 0xc0f720, leave_original, (&leave_if_linked<0xc0f760>), &g_leaves);

// ---- Shared::Delete: a deleting destructor for shared values, which go back to the pool shared_release uses ---------
static volatile long g_shared_deletes;
template <uint32_t DESTROY> static void *__thiscall shared_delete(void *self, uint32_t flags) {
    g_shared_deletes++;
    ((void(__fastcall *)(void *))DESTROY)(self);
    if ((flags & 1) && self)
        ((void(__thiscall *)(void *, void *))0xca1f80)(*(uint8_t **)0x13e3318 + 4, self);
    return self;
}
// Five known bytes are needed; the fifth on are the destructor call's offset, so each copy has its own.
template <uint32_t AT, uint32_t DESTROY> static const uint8_t shared_delete_original[] = {
    0x56, 0x8b, 0xf1, 0xe8, (uint8_t)(DESTROY - (AT + 8)), (uint8_t)((DESTROY - (AT + 8)) >> 8),
    (uint8_t)((DESTROY - (AT + 8)) >> 16), (uint8_t)((DESTROY - (AT + 8)) >> 24)};
REWRITE("Shared::Delete", 0xc0a5b0, (shared_delete_original<0xc0a5b0, 0xc09bf0>), (&shared_delete<0xc09bf0>), &g_shared_deletes);
REWRITE("Shared::Delete", 0xc0a860, (shared_delete_original<0xc0a860, 0xc0a750>), (&shared_delete<0xc0a750>), &g_shared_deletes);
REWRITE("Shared::Delete", 0xc0dc50, (shared_delete_original<0xc0dc50, 0xc0dc80>), (&shared_delete<0xc0dc80>), &g_shared_deletes);
REWRITE("Shared::Delete", 0xc0ddb0, (shared_delete_original<0xc0ddb0, 0xc0dde0>), (&shared_delete<0xc0dde0>), &g_shared_deletes);
REWRITE("Shared::Delete", 0xc0def0, (shared_delete_original<0xc0def0, 0xc0df20>), (&shared_delete<0xc0df20>), &g_shared_deletes);
REWRITE("Shared::Delete", 0xc0e2a0, (shared_delete_original<0xc0e2a0, 0xc0e2d0>), (&shared_delete<0xc0e2d0>), &g_shared_deletes);
REWRITE("Shared::Delete", 0xc0e5b0, (shared_delete_original<0xc0e5b0, 0xc0e5e0>), (&shared_delete<0xc0e5e0>), &g_shared_deletes);
REWRITE("Shared::Delete", 0xc0e8b0, (shared_delete_original<0xc0e8b0, 0xc0e8e0>), (&shared_delete<0xc0e8e0>), &g_shared_deletes);
REWRITE("Shared::Delete", 0xc0eca0, (shared_delete_original<0xc0eca0, 0xc0ecd0>), (&shared_delete<0xc0ecd0>), &g_shared_deletes);
REWRITE("Shared::Delete", 0xc0efb0, (shared_delete_original<0xc0efb0, 0xc0efe0>), (&shared_delete<0xc0efe0>), &g_shared_deletes);
REWRITE("Shared::Delete", 0xc0f290, (shared_delete_original<0xc0f290, 0xc0f2c0>), (&shared_delete<0xc0f2c0>), &g_shared_deletes);
REWRITE("Shared::Delete", 0xc0f480, (shared_delete_original<0xc0f480, 0xc0f4b0>), (&shared_delete<0xc0f4b0>), &g_shared_deletes);
REWRITE("Shared::Delete", 0xc0f680, (shared_delete_original<0xc0f680, 0xc0f6b0>), (&shared_delete<0xc0f6b0>), &g_shared_deletes);
REWRITE("Shared::Delete", 0xc0f820, (shared_delete_original<0xc0f820, 0xc0d460>), (&shared_delete<0xc0d460>), &g_shared_deletes);

// ---- Items::ByIndex: a per-item value from the 0xe0-byte items at +0x40 of the interface kept at +0x218 (flag +0x21c,
// reference +0x208, id global 0x12f5338); 0 past the end. The class's count is its virtual slot 0x4e. Some copies also
// turn down a negative index -----------------------------------------------------------------------------------------
static volatile long g_by_index;
template <bool CHECK_NEGATIVE, uint32_t GET> static uint32_t __thiscall item_by_index(uint8_t *self, int32_t index) {
    g_by_index++;
    if (CHECK_NEGATIVE && index < 0) return 0;
    if (index >= ((int32_t(__fastcall *)(void *))(*(void ***)self)[0x4e])(self)) return 0;
    if (!self[0x21c]) {
        *(void **)(self + 0x218) = find_interface(*(uint8_t **)(self + 0x208), (const int32_t *)0x12f5338);
        self[0x21c] = 1;
    }
    uint8_t *items = *(uint8_t **)(*(uint8_t **)(self + 0x218) + 0x40);
    return ((uint32_t(__fastcall *)(void *))GET)(items + index * 0xe0);
}
static const uint8_t by_index_original[] = {0x53, 0x8b, 0x5c, 0x24, 0x08, 0x85, 0xdb, 0x57};
static const uint8_t by_index_unchecked_original[] = {0x53, 0x57, 0x8b, 0xf9, 0x8b, 0x07, 0xff, 0x90};
REWRITE("Items::ByIndex", 0x531a80, by_index_original, (&item_by_index<true, 0x5591e0>), &g_by_index);
REWRITE("Items::ByIndex", 0x531b10, by_index_original, (&item_by_index<true, 0x559400>), &g_by_index);
REWRITE("Items::ByIndex", 0x531ba0, by_index_original, (&item_by_index<true, 0x559830>), &g_by_index);
REWRITE("Items::ByIndex", 0x532930, by_index_original, (&item_by_index<true, 0x557b30>), &g_by_index);
REWRITE("Items::ByIndexUnchecked", 0x531580, by_index_unchecked_original, (&item_by_index<false, 0x558f20>), &g_by_index);
REWRITE("Items::ByIndexUnchecked", 0x531610, by_index_unchecked_original, (&item_by_index<false, 0x558fe0>), &g_by_index);
REWRITE("Items::ByIndexUnchecked", 0x5316a0, by_index_unchecked_original, (&item_by_index<false, 0x558f80>), &g_by_index);
REWRITE("Items::ByIndexUnchecked", 0x531730, by_index_unchecked_original, (&item_by_index<false, 0x559040>), &g_by_index);

// ---- Names::Find: binary search of a sorted table of names, by bytes as signed chars. True with the position; false
// with where the name would go ------------------------------------------------------------------------------------------
static int compare_names(const char *a, const char *b) {
    for (; *a && *b; a++, b++)
        if (*a != *b) return *a < *b ? -1 : 1;
    return *a ? 1 : *b ? -1 : 0;
}
static volatile long g_name_finds;
static uint8_t __fastcall names_find(const char **name, const char **table, int32_t count, int32_t *at, uint32_t) {
    g_name_finds++;
    if (count < 1) {
        *at = 0;
        return 0;
    }
    int32_t lo = 0, hi = count - 1;
    for (;;) {
        int32_t mid = (hi + lo) >> 1;
        int order = compare_names(*name, table[mid]);
        if (!order) {
            *at = mid;
            return 1;
        }
        if (order < 0) {
            if (mid <= lo) {
                *at = lo;
                return 0;
            }
            hi = mid - 1;
        } else {
            if (mid >= hi) {
                *at = hi + 1;
                return 0;
            }
            lo = mid + 1;
        }
    }
}
static const uint32_t names_find_copies[] = {0x7fb900, 0x805c10, 0x807970};
static const uint8_t names_find_original[] = {0x51, 0x8b, 0x44, 0x24, 0x08, 0x83, 0xf8, 0x01};
REWRITE_COPIES("Names::Find", names_find_copies, names_find_original, names_find, &g_name_finds);

// ---- Chained::~Chained: leaves the singly linked chain (next at +0x14) that starts at +0x3c of its owner (+0x10) -----
static volatile long g_chained;
static void __fastcall chained_destroy(uint32_t *self) {
    g_chained++;
    self[0] = 0xfbf370;
    uint32_t *link = (uint32_t *)(uintptr_t)(self[4] + 0x3c);
    while (*link != (uint32_t)(uintptr_t)self) link = (uint32_t *)(uintptr_t)(*link + 0x14);
    *link = self[5];
    self[0] = 0xfa589c;
}
static const uint32_t chained_copies[] = {0xca3810, 0xca38a0, 0xca3a20, 0xca3ae0, 0xca3be0, 0xca3da0, 0xca3ea0,
                                          0xca41e0, 0xca4230, 0xca4280, 0xca42d0, 0xca4320, 0xca4370};
static const uint8_t chained_original[] = {0x8b, 0x41, 0x10, 0x83, 0xc0, 0x3c, 0xc7, 0x01};
REWRITE_COPIES("Chained::Destroy", chained_copies, chained_original, chained_destroy, &g_chained);

// ---- Bits::Reset(n): {used, capacity, ?, data}: emptied, with fresh room for n words -----------------------------------
static volatile long g_word_resets;
static void *__thiscall words_reset(uint32_t *self, uint32_t n) {
    g_word_resets++;
    memory_free_aligned((void *)(uintptr_t)self[3]);
    self[0] = 0;
    self[1] = n;
    void *data = n ? memory_alloc_aligned(n * 4) : nullptr;
    self[3] = (uint32_t)(uintptr_t)data;
    return data;
}
static const uint32_t words_reset_copies[] = {0xc479c0, 0xc47a50, 0xc47ae0, 0xc47b70, 0xc47c00, 0xc47c90, 0xcb22a0, 0xcb2330};
static const uint8_t words_reset_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x0c, 0x50, 0xe8};
REWRITE_COPIES("Words::Reset", words_reset_copies, words_reset_original, words_reset, &g_word_resets);

// ---- Words::Resize(n): {data, capacity, count}: keeps the first min(n, capacity) words; both sizes become n ---------
static volatile long g_word_resizes;
static void __thiscall words_resize(uint32_t *self, int32_t n) {
    g_word_resizes++;
    int32_t capacity = (int32_t)self[1];
    if (n == capacity) return;
    uint32_t *fresh = nullptr;
    if (n) {
        fresh = (uint32_t *)memory_alloc_aligned((uint32_t)n * 4);
        capacity = (int32_t)self[1];
        int32_t keep = capacity < n ? capacity : n;
        for (int32_t i = 0; i < keep; i++) fresh[i] = ((uint32_t *)(uintptr_t)self[0])[i];
    }
    memory_free_aligned((void *)(uintptr_t)self[0]);
    self[0] = (uint32_t)(uintptr_t)fresh;
    self[2] = (uint32_t)n;
    self[1] = (uint32_t)n;
}
static const uint32_t words_resize_copies[] = {0x665780, 0x665850, 0x665920, 0xafeeb0};
static const uint8_t words_resize_original[] = {0x83, 0xec, 0x08, 0x53, 0x8b, 0x5c, 0x24, 0x10};
REWRITE_COPIES("Words::Resize", words_resize_copies, words_resize_original, words_resize, &g_word_resizes);

// ---- Slots::Free(i): 32-byte slots at +0x18 (in use flag at +0x1c); +0xc is the lowest free, +0x10 the end, which is
// pulled back over any free slots ---------------------------------------------------------------------------------------
static volatile long g_slot_frees;
static void __thiscall slots_free(uint8_t *self, int32_t index) {
    g_slot_frees++;
    uint8_t *slots = *(uint8_t **)(self + 0x18);
    *(uint32_t *)(slots + (uint32_t)index * 32 + 0x1c) = 0;
    int32_t *lowest = (int32_t *)(self + 0xc), *end = (int32_t *)(self + 0x10);
    if (index < *lowest) *lowest = index;
    if (index + 1 != *end) return;
    --*end;
    while (*end > 0 && !*(uint32_t *)(slots + (uint32_t)*end * 32 - 4)) --*end;
}
static const uint32_t slots_free_copies[] = {0x42f6f0, 0x4883c0, 0x7b2ac0, 0xa0bec0, 0xb5b650, 0xb6b900};
static const uint8_t slots_free_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0xd0, 0xc1, 0xe2};
REWRITE_COPIES("Slots::Free", slots_free_copies, slots_free_original, slots_free, &g_slot_frees);

// ---- Prime::After(n): the smallest prime above n (2 at the least), for hash table sizes ---------------------------------
static volatile long g_primes;
static uint32_t __fastcall prime_after(uint32_t n) {
    g_primes++;
    uint32_t candidate = n + 1 > 2 ? n + 1 : 2;
    for (uint32_t d = 2; d < candidate; d++)
        if (candidate % d == 0) {
            candidate++;
            d = 1;
        }
    return candidate;
}
static const uint32_t prime_copies[] = {0xbd6620, 0xbdd460, 0xbdd4b0, 0xc478d0, 0xc47920, 0xcbc4f0};
static const uint8_t prime_original[] = {0x83, 0xec, 0x0c, 0x41, 0x89, 0x0c, 0x24, 0xb9};
REWRITE_COPIES("Prime::After", prime_copies, prime_original, prime_after, &g_primes);

// ---- Names::Add: a name and its value into parallel arrays {names, capacity, values, ?, growth, count}, the names kept in
// order; full arrays grow by `growth` first. A name already there goes in again, at its place -----------------------------
static volatile long g_name_adds;
template <uint32_t GROW_NAMES, uint32_t GROW_VALUES, uint32_t FIND>
static void __thiscall names_add(uint8_t *self, uint32_t *name, uint32_t *value) {
    g_name_adds++;
    int32_t *count = (int32_t *)(self + 0x14);
    if (*count == *(int32_t *)(self + 4)) {
        int32_t grown = *(int32_t *)(self + 0x10) + *count;
        ((void(__thiscall *)(void *, int32_t))GROW_NAMES)(self, grown);
        ((void(__thiscall *)(void *, int32_t))GROW_VALUES)(self + 8, grown);
    }
    int32_t at = 0;
    ((uint8_t(__fastcall *)(void *, void *, int32_t, int32_t *, int32_t *))FIND)(name, *(void **)self, *count, &at, &at);
    uint32_t *names = *(uint32_t **)self;
    for (int32_t i = *count; i > at; i--) names[i] = names[i - 1];
    names[at] = *name;
    uint32_t *values = *(uint32_t **)(self + 8);
    for (int32_t i = *count; i > at; i--) values[i] = values[i - 1];
    values[at] = *value;
    ++*count;
}
static const uint8_t names_add_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x14, 0x3b, 0x46};
REWRITE("Names::Add", 0x7fbe30, names_add_original, (&names_add<0x7fb570, 0x7fb6f0, 0x7fb900>), &g_name_adds);
REWRITE("Names::Add", 0x7fbf50, names_add_original, (&names_add<0x417c20, 0x7fb7c0, 0x7fb9c0>), &g_name_adds);
REWRITE("Names::Add", 0x802900, names_add_original, (&names_add<0x801370, 0x8012a0, 0x801450>), &g_name_adds);
REWRITE("Names::Add", 0x805ec0, names_add_original, (&names_add<0x7fb570, 0x805b80, 0x805c10>), &g_name_adds);
REWRITE("Names::Add", 0x8085d0, names_add_original, (&names_add<0x7fb570, 0x8078c0, 0x807970>), &g_name_adds);

// ---- Linked::Delete: a deleting destructor for a small object that is a list node at +4 ---------------------------------
static volatile long g_linked_deletes;
template <uint32_t TABLE> static void *__thiscall linked_delete(uint32_t *self, uint32_t flags) {
    g_linked_deletes++;
    self[0] = TABLE;
    unlink((Node *)(self + 1));
    if (flags & 1) memory_free(self);
    return self;
}
static const uint8_t linked_delete_original[] = {0x56, 0x8b, 0xf1, 0x8d, 0x46, 0x04, 0x85, 0xc0};
REWRITE("Linked::Delete", 0x453ed0, linked_delete_original, (&linked_delete<0xf296f8>), &g_linked_deletes);
REWRITE("Linked::Delete", 0x7faf30, linked_delete_original, (&linked_delete<0xf75138>), &g_linked_deletes);
REWRITE("Linked::Delete", 0x872120, linked_delete_original, (&linked_delete<0xf78b60>), &g_linked_deletes);
REWRITE("Linked::Delete", 0x8ea2b0, linked_delete_original, (&linked_delete<0xf7ccbc>), &g_linked_deletes);
REWRITE("Linked::Delete", 0x9491b0, linked_delete_original, (&linked_delete<0xf7e29c>), &g_linked_deletes);

// ---- Compare::MappedThen: map the second object's value (the other side), then hand both on, with the first object's two
// bytes at +0x88 and +0x89 in the low bytes of the first and second arguments -----------------------------------------------
static volatile long g_mapped_thens;
template <int32_t STEP> static uint8_t __thiscall mapped_then(void *self, uint32_t a, uint8_t *b, uint32_t side) {
    g_mapped_thens++;
    int32_t value = *(int32_t *)(b + 0xa4);
    if (!((uint8_t(__fastcall *)(uint8_t, uint8_t, int32_t *, int32_t))0x71b0f0)(b[0x8d], (uint8_t)side ^ 1, &value, STEP))
        return 0;
    uint8_t *first = (uint8_t *)(uintptr_t)a;
    return ((uint8_t(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, int32_t))0x71d6e0)(
        self, (a & ~0xffu) | first[0x88], (side & ~0xffu) | first[0x89], side, value);
}
static const uint8_t mapped_then_original[] = {0x8b, 0x44, 0x24, 0x08, 0x53, 0x8b, 0x5c, 0x24};
REWRITE("Compare::MappedThen", 0x71ea70, mapped_then_original, (&mapped_then<2>), &g_mapped_thens);
REWRITE("Compare::MappedThen", 0x71eae0, mapped_then_original, (&mapped_then<-2>), &g_mapped_thens);
REWRITE("Compare::MappedThen", 0x71eb50, mapped_then_original, (&mapped_then<1>), &g_mapped_thens);
REWRITE("Compare::MappedThen", 0x71ebc0, mapped_then_original, (&mapped_then<-1>), &g_mapped_thens);

// ---- Refs::Visit: an array {data, capacity, count} of 20-byte references: the visitor sees each one's interface (or null) -
static volatile long g_ref_visits;
template <uint32_t ID> static void __thiscall refs_visit(uint8_t *self, void *visitor) {
    g_ref_visits++;
    for (int32_t i = 0; i < *(int32_t *)(self + 8); i++) {
        uint8_t *handle = *(uint8_t **)(*(uint8_t **)self + i * 20 + 4);
        void *interface = find_interface(handle, (const int32_t *)ID);
        ((void(__thiscall *)(void *, void *))(*(void ***)visitor)[0])(visitor, interface);
    }
}
static const uint8_t refs_visit_original[] = {0x53, 0x57, 0x8b, 0xf9, 0x8b, 0x47, 0x08, 0x33};
REWRITE("Refs::Visit", 0x730b80, refs_visit_original, (&refs_visit<0x13063f0>), &g_ref_visits);
REWRITE("Refs::Visit", 0x738620, refs_visit_original, (&refs_visit<0x130646c>), &g_ref_visits);
REWRITE("Refs::Visit", 0xac2ca0, refs_visit_original, (&refs_visit<0x13ce3f8>), &g_ref_visits);
REWRITE("Refs::Visit", 0xba4dd0, refs_visit_original, (&refs_visit<0x13d05fc>), &g_ref_visits);

// ---- Cloned::Copy: a copy constructor: an empty list at +4 (first +4, end node +0x10, its own head link at +0x14), filled
// with a clone of each item of the other's list; each clone's list node is at +OFF, and goes at the front -------------------
static volatile long g_cloned;
template <uint32_t TABLE, uint32_t CLONE, uint32_t OFF> static uint32_t *__thiscall cloned_copy(uint32_t *self, uint8_t *other) {
    g_cloned++;
    self[0] = TABLE;
    uint32_t *list = self + 1;
    list[2] = 0;
    list[5] = 0;
    list[1] = 0;
    list[0] = (uint32_t)(uintptr_t)(list + 3);
    list[4] = (uint32_t)(uintptr_t)list;
    list[3] = 0;
    for (Node *n = *(Node **)(other + 4); n->next; n = n->next) {
        uint8_t *clone = ((uint8_t * (__thiscall *)(void *, int32_t))CLONE)(n->value, 0);
        Node *added = (Node *)(clone + OFF), *head = (Node *)(uintptr_t)self[5];
        added->next = head->next;
        head->next->prev = added;
        added->prev = head;
        head->next = added;
    }
    return self;
}
static const uint8_t cloned_original[] = {0x56, 0x57, 0x8b, 0xf9, 0xc7, 0x07};
REWRITE("Cloned::Copy", 0xc0d940, cloned_original, (&cloned_copy<0xfafa94, 0xc0bad0, 4>), &g_cloned);
REWRITE("Cloned::Copy", 0xc10570, cloned_original, (&cloned_copy<0xfafa90, 0xc0e1b0, 0x4c>), &g_cloned);
REWRITE("Cloned::Copy", 0xc105e0, cloned_original, (&cloned_copy<0xfafa98, 0xc0bb70, 4>), &g_cloned);
REWRITE("Cloned::Copy", 0xc124f0, cloned_original, (&cloned_copy<0xfafaa4, 0xc110e0, 0x48>), &g_cloned);

// ---- Checked::ByIndex: an 8-byte table entry (+0x190, count +0x164) passed to GET; a bad index reports at source line LINE
// and stops at a breakpoint when the report asks to ---------------------------------------------------------------------------
static volatile long g_checked;
template <uint32_t LINE, uint32_t GET> static uint32_t __thiscall checked_by_index(uint8_t *self, int32_t index, uint32_t arg) {
    g_checked++;
    if (index >= *(int32_t *)(self + 0x164)) {
        typedef uint8_t(__fastcall * Report)(uint32_t, uint32_t, uint32_t, uint32_t);
        if (((Report)0x68f640)(0xf52e20, LINE, 0xf52e08, 0xf52e60) == 1) __builtin_debugtrap();
    }
    void *item = *(void **)(*(uint8_t **)(self + 0x190) + index * 8);
    return ((uint32_t(__thiscall *)(void *, uint32_t))GET)(item, arg);
}
static const uint8_t checked_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x86, 0x64, 0x01, 0x00};
REWRITE("Checked::ByIndex", 0x68fe00, checked_original, (&checked_by_index<0x1ce, 0x6ae070>), &g_checked);
REWRITE("Checked::ByIndex", 0x68fe50, checked_original, (&checked_by_index<0x1df, 0x67a7a0>), &g_checked);
REWRITE("Checked::ByIndex", 0x68fef0, checked_original, (&checked_by_index<0x203, 0x67bb20>), &g_checked);
REWRITE("Checked::ByIndex", 0x68ff40, checked_original, (&checked_by_index<0x214, 0x67b4f0>), &g_checked);
REWRITE("Checked::ByIndex", 0x68ffe0, checked_original, (&checked_by_index<0x232, 0x67a8c0>), &g_checked);
REWRITE("Checked::ByIndex", 0x690030, checked_original, (&checked_by_index<0x242, 0x67a8a0>), &g_checked);

// ---- Pool::Take: {items, free list, used, capacity}: marks the next free item in use and returns it; a full pool doubles
// (from nothing: 200). Items are 1 << SHIFT bytes, in use flag first ------------------------------------------------------------
static volatile long g_pool_takes;
template <uint32_t GROW, uint32_t SHIFT> static int32_t __fastcall pool_take(uint8_t *self) {
    g_pool_takes++;
    int32_t *used = (int32_t *)(self + 8), *capacity = (int32_t *)(self + 0xc);
    if (*used >= *capacity) ((void(__thiscall *)(void *, int32_t))GROW)(self, *capacity > 0 ? *capacity * 2 : 200);
    int32_t item = (*(int32_t **)(self + 4))[*capacity - *used - 1];
    (*(uint8_t **)self)[(uint32_t)item << SHIFT] = 1;
    ++*used;
    return item;
}
static const uint8_t pool_take_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x0c, 0x39, 0x46};
REWRITE("Pool::Take", 0x42dda0, pool_take_original, (&pool_take<0x42d730, 4>), &g_pool_takes);
REWRITE("Pool::Take", 0x42e950, pool_take_original, (&pool_take<0x42e780, 4>), &g_pool_takes);
REWRITE("Pool::Take", 0x49aae0, pool_take_original, (&pool_take<0x49a7e0, 4>), &g_pool_takes);
REWRITE("Pool::Take", 0x8103a0, pool_take_original, (&pool_take<0x80fa40, 4>), &g_pool_takes);
REWRITE("Pool::Take", 0x87c670, pool_take_original, (&pool_take<0x87bee0, 7>), &g_pool_takes);
REWRITE("Pool::Take", 0x9d29e0, pool_take_original, (&pool_take<0x9d0f40, 6>), &g_pool_takes);
REWRITE("Pool::Take", 0x9f82c0, pool_take_original, (&pool_take<0x9f7fc0, 6>), &g_pool_takes);

// ---- QueueSurface::Activate: a queue's ground surface takes its place in its queue type's database entry, reached
// through the reference at +0x4c (kept at +0x5c, flag +0x60). A park saved with custom queues this install doesn't have
// leaves no entry; the game crashes on that, this does without the surface ------------------------------------------------
static volatile long g_queue_surfaces;
static void __fastcall queue_surface_activate(uint8_t *self) {
    g_queue_surfaces++;
    if (!self[0x60]) {
        *(void **)(self + 0x5c) = find_interface(*(uint8_t **)(self + 0x4c), (const int32_t *)0x130646c);
        self[0x60] = 1;
    }
    uint8_t *entry = *(uint8_t **)(self + 0x5c);
    if (!entry) return;
    if (!(entry[0x2c] & 1)) ((void(__fastcall *)(void *))0x800860)(entry);
    ++*(int32_t *)(entry + 0x16c);
    if (!entry[0x168]) ((void(__fastcall *)(void *))0x7397d0)(entry);
    ((void(__thiscall *)(void *, void *, void *))0xbe8230)(entry + 0x174, self + 0x64, self);
}
static const uint8_t queue_surface_original[] = {0x56, 0x8b, 0xf1, 0x8a, 0x46, 0x60, 0x84, 0xc0};
REWRITE("QueueSurface::Activate", 0x73aa80, queue_surface_original, queue_surface_activate, &g_queue_surfaces);

// ---- Global::Delete: a heap object kept in a global: destroyed, freed and the global cleared ---------------------------
static volatile long g_global_deletes;
template <uint32_t GLOBAL, uint32_t DESTROY> static void __cdecl global_delete() {
    g_global_deletes++;
    void *object = *(void **)GLOBAL;
    if (object) {
        ((void(__fastcall *)(void *))DESTROY)(object);
        memory_free(object);
    }
    *(void **)GLOBAL = nullptr;
}
template <uint32_t GLOBAL> static const uint8_t global_delete_original[] = {
    0x8b, 0x0d, (uint8_t)GLOBAL, (uint8_t)(GLOBAL >> 8), (uint8_t)(GLOBAL >> 16), (uint8_t)(GLOBAL >> 24), 0x85, 0xc9};
REWRITE("Global::Delete", 0x409ec0, global_delete_original<0x10ff9c4>, (&global_delete<0x10ff9c4, 0x409ba0>), &g_global_deletes);
REWRITE("Global::Delete", 0x40d980, global_delete_original<0x10ffa14>, (&global_delete<0x10ffa14, 0x40c980>), &g_global_deletes);
REWRITE("Global::Delete", 0x42dde0, global_delete_original<0x1103be0>, (&global_delete<0x1103be0, 0x42d350>), &g_global_deletes);
REWRITE("Global::Delete", 0x47eb50, global_delete_original<0x12f018c>, (&global_delete<0x12f018c, 0x47e4e0>), &g_global_deletes);
REWRITE("Global::Delete", 0x49abb0, global_delete_original<0x12f46dc>, (&global_delete<0x12f46dc, 0x49a980>), &g_global_deletes);
REWRITE("Global::Delete", 0x6926e0, global_delete_original<0x12f84c8>, (&global_delete<0x12f84c8, 0x6914a0>), &g_global_deletes);
REWRITE("Global::Delete", 0x7260e0, global_delete_original<0x1306344>, (&global_delete<0x1306344, 0x725de0>), &g_global_deletes);
REWRITE("Global::Delete", 0x744f00, global_delete_original<0x13064a4>, (&global_delete<0x13064a4, 0x743f60>), &g_global_deletes);
REWRITE("Global::Delete", 0x757b10, global_delete_original<0x1306530>, (&global_delete<0x1306530, 0x7576d0>), &g_global_deletes);
REWRITE("Global::Delete", 0x7fa7c0, global_delete_original<0x130abd8>, (&global_delete<0x130abd8, 0x7fa630>), &g_global_deletes);
REWRITE("Global::Delete", 0x8545c0, global_delete_original<0x130bbf8>, (&global_delete<0x130bbf8, 0x851a50>), &g_global_deletes);
REWRITE("Global::Delete", 0x8b09b0, global_delete_original<0x1311230>, (&global_delete<0x1311230, 0x8b0400>), &g_global_deletes);
REWRITE("Global::Delete", 0x8f9310, global_delete_original<0x1339fb4>, (&global_delete<0x1339fb4, 0x8f8a00>), &g_global_deletes);
REWRITE("Global::Delete", 0x983700, global_delete_original<0x13412dc>, (&global_delete<0x13412dc, 0x982dc0>), &g_global_deletes);
REWRITE("Global::Delete", 0x409db0, global_delete_original<0x10ff9c8>, (&global_delete<0x10ff9c8, 0x409980>), &g_global_deletes);
REWRITE("Global::Delete", 0x4965e0, global_delete_original<0x12f4694>, (&global_delete<0x12f4694, 0x495e90>), &g_global_deletes);
REWRITE("Global::Delete", 0x496b70, global_delete_original<0x12f469c>, (&global_delete<0x12f469c, 0xc091b0>), &g_global_deletes);
REWRITE("Global::Delete", 0x512480, global_delete_original<0x12f50b4>, (&global_delete<0x12f50b4, 0x50cda0>), &g_global_deletes);
REWRITE("Global::Delete", 0x620d90, global_delete_original<0x12f57d0>, (&global_delete<0x12f57d0, 0x620bd0>), &g_global_deletes);
REWRITE("Global::Delete", 0x85bce0, global_delete_original<0x130bc98>, (&global_delete<0x130bc98, 0x85abe0>), &g_global_deletes);
REWRITE("Global::Delete", 0x884310, global_delete_original<0x1310f74>, (&global_delete<0x1310f74, 0x883020>), &g_global_deletes);
REWRITE("Global::Delete", 0x9655a0, global_delete_original<0x133b4d4>, (&global_delete<0x133b4d4, 0x964fe0>), &g_global_deletes);
REWRITE("Global::Delete", 0x9c6030, global_delete_original<0x13414ec>, (&global_delete<0x13414ec, 0x9c5c90>), &g_global_deletes);
REWRITE("Global::Delete", 0xce92a0, global_delete_original<0x1400b70>, (&global_delete<0x1400b70, 0xce8a10>), &g_global_deletes);
REWRITE("Global::Delete", 0xcee290, global_delete_original<0x1408b80>, (&global_delete<0x1408b80, 0xced240>), &g_global_deletes);
REWRITE("Global::Delete", 0xcefda0, global_delete_original<0x141ab2c>, (&global_delete<0x141ab2c, 0xcef7e0>), &g_global_deletes);

// ---- Field::Construct: a saved field's constructor (0x8088C0) with its class's description and kind, then its own table --
static volatile long g_field_constructs;
template <uint32_t KIND, uint32_t DESCRIPTION, uint32_t TABLE>
static uint32_t *__thiscall field_construct(uint32_t *self, uint32_t a, uint32_t b) {
    g_field_constructs++;
    ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))0x8088c0)(self, a, DESCRIPTION, KIND, b);
    self[0] = TABLE;
    return self;
}
static const uint8_t field_construct_original[] = {0x8b, 0x44, 0x24, 0x08, 0x56, 0x50, 0x6a};
REWRITE("Field::Construct", 0x423b70, field_construct_original, (&field_construct<0x4, 0xf27b18, 0xf27b04>), &g_field_constructs);
REWRITE("Field::Construct", 0x423bb0, field_construct_original, (&field_construct<0x0, 0xf27b34, 0xf27b20>), &g_field_constructs);
REWRITE("Field::Construct", 0x44cac0, field_construct_original, (&field_construct<0x1, 0xf28d50, 0xf2942c>), &g_field_constructs);
REWRITE("Field::Construct", 0x455710, field_construct_original, (&field_construct<0x0, 0xf29794, 0xf29780>), &g_field_constructs);
REWRITE("Field::Construct", 0x4a8bf0, field_construct_original, (&field_construct<0x0, 0xf27b94, 0xf30a40>), &g_field_constructs);
REWRITE("Field::Construct", 0x4e6050, field_construct_original, (&field_construct<0x8, 0xf294a0, 0xf35524>), &g_field_constructs);
REWRITE("Field::Construct", 0x50c680, field_construct_original, (&field_construct<0x8, 0xf294a0, 0xf36e4c>), &g_field_constructs);
REWRITE("Field::Construct", 0x5a22b0, field_construct_original, (&field_construct<0x0, 0xf27b94, 0xf3de30>), &g_field_constructs);
REWRITE("Field::Construct", 0x5a22e0, field_construct_original, (&field_construct<0x0, 0xf27b94, 0xf3de44>), &g_field_constructs);
REWRITE("Field::Construct", 0xb0e270, field_construct_original, (&field_construct<0x0, 0xf97b50, 0xf97b3c>), &g_field_constructs);

// ---- Event::IfAccepted: asks the interface kept at +0x108 (flag +0x10c, reference +0xf8) whether to take a two-argument
// event, and passes it to the object's own handler if so ------------------------------------------------------------------
static volatile long g_if_accepted;
template <uint32_t ASK, uint32_t HANDLE> static void __thiscall event_if_accepted(uint8_t *self, uint32_t a, uint32_t b) {
    g_if_accepted++;
    if (!self[0x10c]) {
        *(void **)(self + 0x108) = find_interface(*(uint8_t **)(self + 0xf8), (const int32_t *)0x13ce3bc);
        self[0x10c] = 1;
    }
    if (((uint8_t(__thiscall *)(void *, uint32_t, uint32_t))ASK)(*(void **)(self + 0x108), a, b))
        ((void(__thiscall *)(void *, uint32_t, uint32_t))HANDLE)(self, a, b);
}
static const uint8_t if_accepted_original[] = {0x53, 0x56, 0x57, 0x8b, 0xf9, 0x8a, 0x87, 0x0c};
REWRITE("Event::IfAccepted", 0xaa90d0, if_accepted_original, (&event_if_accepted<0xaae180, 0x848840>), &g_if_accepted);
REWRITE("Event::IfAccepted", 0xaa9150, if_accepted_original, (&event_if_accepted<0xaae1b0, 0x8489b0>), &g_if_accepted);
REWRITE("Event::IfAccepted", 0xaa91d0, if_accepted_original, (&event_if_accepted<0xaae1e0, 0x848b20>), &g_if_accepted);
REWRITE("Event::IfAccepted", 0xaa9250, if_accepted_original, (&event_if_accepted<0xaae210, 0x848c90>), &g_if_accepted);

// ---- Refs::InterfaceAt(i): the interface of the i-th 28-byte reference (array at +ARRAY, count at +COUNT), or null past
// the end. Each keeps its interface at +0x14, flag +0x18 ----------------------------------------------------------------------
static volatile long g_ref_interfaces;
template <uint32_t COUNT, uint32_t ARRAY, uint32_t ID> static void *__thiscall ref_interface_at(uint8_t *self, int32_t i) {
    g_ref_interfaces++;
    if (i >= *(int32_t *)(self + COUNT)) return nullptr;
    uint8_t *ref = *(uint8_t **)(self + ARRAY) + i * 0x1c;
    if (!ref[0x18]) {
        *(void **)(ref + 0x14) = find_interface(*(uint8_t **)(ref + 4), (const int32_t *)ID);
        ref[0x18] = 1;
    }
    return *(void **)(ref + 0x14);
}
static const uint8_t ref_interface_original[] = {0x8b, 0x44, 0x24, 0x04, 0x3b, 0x81};
REWRITE("Refs::InterfaceAt", 0x561240, ref_interface_original, (&ref_interface_at<0xc4, 0xc0, 0x12f50d4>), &g_ref_interfaces);
REWRITE("Refs::InterfaceAt", 0x5a7300, ref_interface_original, (&ref_interface_at<0x6b0, 0x6ac, 0x12f537c>), &g_ref_interfaces);
REWRITE("Refs::InterfaceAt", 0x5d67e0, ref_interface_original, (&ref_interface_at<0xe0, 0xdc, 0x12f537c>), &g_ref_interfaces);
REWRITE("Refs::InterfaceAt", 0x5d6860, ref_interface_original, (&ref_interface_at<0x100, 0xfc, 0x12f537c>), &g_ref_interfaces);

// ---- Grid::Collect: a quadtree walk from node `index` (28-byte nodes at +8; children 4n+1.. flagged in the low bits of
// their parent). Each entry (32 bytes, at +0x18, count +0x10) whose item matches the query's masks has its value added to
// the query's buffer, which is handed on (virtual slot 1) whenever it fills. The query is at +QUERY of `request` -------------
struct GridQuery {
    void **table;
    uint32_t unknown;
    uint32_t mask[2];    // +8
    uint32_t bits[2];    // +0x10
    int32_t count;       // +0x18
    int32_t capacity;    // +0x1c
    uint32_t *buffer;    // +0x20
};
static volatile long g_grid_collects;
template <uint32_t QUERY> static void __thiscall grid_collect(uint8_t *self, int32_t index, uint8_t *request) {
    g_grid_collects++;
    uint8_t *node = *(uint8_t **)(self + 8) + index * 0x1c;
    if (*(uint32_t *)(node + 0xc) || *(uint32_t *)(node + 0x10)) {
        for (int32_t i = 0; i < *(int32_t *)(node + 0x10); i++) {
            uint8_t *entry = *(uint8_t **)(node + 0x18) + i * 0x20;
            if (!*(uint32_t *)(entry + 0x1c)) continue;
            uint8_t *item = *(uint8_t **)(entry + 0x18);
            GridQuery *q = *(GridQuery **)(request + QUERY);
            if (((*(uint32_t *)(item + 0x18) ^ q->bits[0]) & q->mask[0]) | ((*(uint32_t *)(item + 0x1c) ^ q->bits[1]) & q->mask[1]))
                continue;
            q->buffer[q->count] = *(uint32_t *)(item + 8);
            q->count++;
            if (q->count >= q->capacity && q->count) {
                ((void(__thiscall *)(void *, int32_t))q->table[1])(q, q->count);
                q->count = 0;
            }
        }
    }
    uint32_t children = *(uint32_t *)node;
    if (!(children & 0xf)) return;
    for (int32_t i = 0; i < 4; i++)
        if (*(uint32_t *)node & (1u << i)) grid_collect<QUERY>(self, index * 4 + i + 1, request);
}
static const uint8_t grid_collect_original[] = {0x51, 0x8b, 0x51, 0x08, 0x56, 0x57, 0x8b, 0x7c};
REWRITE("Grid::Collect", 0x48a4e0, grid_collect_original, (&grid_collect<0x18>), &g_grid_collects);
REWRITE("Grid::Collect", 0x48a6e0, grid_collect_original, (&grid_collect<0x28>), &g_grid_collects);
REWRITE("Grid::Collect", 0x48b370, grid_collect_original, (&grid_collect<0x60>), &g_grid_collects);

// ---- Settings::Copy: copies a settings block, leaving out the padding between its groups ---------------------------------
static volatile long g_settings_copies;
static uint8_t *__thiscall settings_copy(uint8_t *self, const uint8_t *other) {
    g_settings_copies++;
    __builtin_memcpy(self, other, 0xc);
    __builtin_memcpy(self + 0x10, other + 0x10, 0xc);
    __builtin_memcpy(self + 0x20, other + 0x20, 0x31);
    __builtin_memcpy(self + 0x60, other + 0x60, 0xc);
    __builtin_memcpy(self + 0x70, other + 0x70, 0x78);
    return self;
}
static const uint32_t settings_copies[] = {0x425b80, 0x4263a0};
static const uint8_t settings_copy_original[] = {0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x04, 0x8b, 0x11};
REWRITE_COPIES("Settings::Copy", settings_copies, settings_copy_original, settings_copy, &g_settings_copies);

// Rows::Save for rows of single bytes.
static const uint8_t rows_save_bytes_original[] = {0x83, 0xec, 0x08, 0x8b, 0x44, 0x24, 0x0c, 0x53};
REWRITE("Rows::SaveBytes", 0x5ba680, rows_save_bytes_original, (&rows_save<1>), &g_rows_saves);
REWRITE("Rows::SaveBytes", 0xa01ce0, rows_save_bytes_original, (&rows_save<1>), &g_rows_saves);

// ---- Cached::Forward: hands on to a method of the interface kept at +CACHE (flag +FLAG, reference +REF) ---------------
static volatile long g_cached_forwards;
template <uint32_t REF, uint32_t FLAG, uint32_t CACHE, uint32_t ID, uint32_t CALL> static uint32_t __fastcall cached_forward(uint8_t *self) {
    g_cached_forwards++;
    if (!self[FLAG]) {
        *(void **)(self + CACHE) = find_interface(*(uint8_t **)(self + REF), (const int32_t *)ID);
        self[FLAG] = 1;
    }
    return ((uint32_t(__fastcall *)(void *))CALL)(*(void **)(self + CACHE));
}
static const uint8_t cached_forward_original[] = {0x57, 0x8b, 0xf9, 0x8a, 0x87};
REWRITE("Cached::Forward", 0x532a20, cached_forward_original, (&cached_forward<0x208, 0x21c, 0x218, 0x12f5338, 0x55b630>), &g_cached_forwards);
REWRITE("Cached::Forward", 0x5ec260, cached_forward_original, (&cached_forward<0x104, 0x118, 0x114, 0x12f5608, 0x5c6c80>), &g_cached_forwards);
REWRITE("Cached::Forward", 0x5ec2c0, cached_forward_original, (&cached_forward<0x104, 0x118, 0x114, 0x12f5608, 0x5c6c80>), &g_cached_forwards);
REWRITE("Cached::Forward", 0x66bb50, cached_forward_original, (&cached_forward<0x168, 0x17c, 0x178, 0x12f4b34, 0x4a1840>), &g_cached_forwards);
REWRITE("Cached::Forward", 0xb54910, cached_forward_original, (&cached_forward<0xb0, 0xc4, 0xc0, 0x13cea88, 0xb53da0>), &g_cached_forwards);

// ---- Words::Append: one more word in an exactly sized array {count at +COUNT, data at +DATA}: copied to a new block -----
static volatile long g_word_appends;
template <uint32_t COUNT, uint32_t DATA> static void __thiscall words_append(uint8_t *self, uint32_t word) {
    g_word_appends++;
    int32_t *count = (int32_t *)(self + COUNT);
    uint32_t **data = (uint32_t **)(self + DATA), *old = *data;
    *data = (uint32_t *)memory_alloc_aligned((uint32_t)*count * 4 + 4);
    for (int32_t i = 0; i < *count; i++) (*data)[i] = old[i];
    (*data)[*count] = word;
    ++*count;
    memory_free_aligned(old);
}
static const uint8_t words_append_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46};
REWRITE("Words::Append", 0xcc8320, words_append_original, (&words_append<0x24, 0x28>), &g_word_appends);
REWRITE("Words::Append", 0xce2250, words_append_original, (&words_append<0x14, 0x18>), &g_word_appends);
REWRITE("Words::Append", 0xce22b0, words_append_original, (&words_append<0x3c, 0x40>), &g_word_appends);
REWRITE("Words::Append", 0xce2310, words_append_original, (&words_append<0x3c, 0x44>), &g_word_appends);
REWRITE("Words::Append", 0xceedc0, words_append_original, (&words_append<0x24, 0x28>), &g_word_appends);

// ---- Vector::Free: an array of objects with its count in a header before it: each destroyed (0xDDCF39), the block freed --
typedef void(__stdcall *DestroyEach)(void *, uint32_t, int32_t, void *);
static const DestroyEach destroy_each = (DestroyEach)0xddcf39;
static void vector_free(uint8_t *items, int32_t header, uint32_t size, uint32_t destroy) {
    uint8_t *block = items - header;
    destroy_each(items, size, *(int32_t *)block, (void *)(uintptr_t)destroy);
    memory_free_aligned(block);
}
static volatile long g_vector_frees;
// With a header of 16 or 4 bytes, and a second block at +4.
template <int32_t HEADER, uint32_t SIZE, uint32_t DESTROY> static void __fastcall vector_free_pair(uint8_t **self) {
    g_vector_frees++;
    if (self[0]) vector_free(self[0], HEADER, SIZE, DESTROY);
    if (self[1]) memory_free_aligned(self[1]);
}
static const uint8_t vector_pair_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x06, 0x85, 0xc0, 0x74};
REWRITE("Vector::FreePair", 0x40c550, vector_pair_original, (&vector_free_pair<16, 0x70, 0x40b470>), &g_vector_frees);
REWRITE("Vector::FreePair", 0x40c5c0, vector_pair_original, (&vector_free_pair<4, 0x24, 0x40b4c0>), &g_vector_frees);
REWRITE("Vector::FreePair", 0x4998d0, vector_pair_original, (&vector_free_pair<4, 0x18, 0x4992a0>), &g_vector_frees);
REWRITE("Vector::FreePair", 0x4d0940, vector_pair_original, (&vector_free_pair<4, 0x2c, 0x4cef40>), &g_vector_frees);
REWRITE("Vector::FreePair", 0x9d0d90, vector_pair_original, (&vector_free_pair<16, 0x40, 0x9cdb00>), &g_vector_frees);
REWRITE("Vector::FreePair", 0xce7570, vector_pair_original, (&vector_free_pair<16, 0x70, 0xce6810>), &g_vector_frees);
REWRITE("Vector::FreePair", 0xcebdd0, vector_pair_original, (&vector_free_pair<16, 0x70, 0xce95e0>), &g_vector_frees);

// A vector with a 4-byte header at +OFF, left in place.
template <uint32_t OFF, uint32_t SIZE, uint32_t DESTROY> static void __fastcall vector_free_field(uint8_t *self) {
    g_vector_frees++;
    uint8_t *items = *(uint8_t **)(self + OFF);
    if (items) vector_free(items, 4, SIZE, DESTROY);
}
template <uint32_t OFF> static const uint8_t vector_field_original[] = {0x8b, 0x41, (uint8_t)OFF, 0x85, 0xc0, 0x74};
REWRITE("Vector::FreeField", 0x48a420, vector_field_original<0x8>, (&vector_free_field<0x8, 0x1c, 0x4883b0>), &g_vector_frees);
REWRITE("Vector::FreeField", 0x7b6680, vector_field_original<0x8>, (&vector_free_field<0x8, 0x1c, 0x7b2ab0>), &g_vector_frees);
REWRITE("Vector::FreeField", 0x8fcfe0, vector_field_original<0x8>, (&vector_free_field<0x8, 0x28, 0x8fc8f0>), &g_vector_frees);
REWRITE("Vector::FreeField", 0xa0ddc0, vector_field_original<0x8>, (&vector_free_field<0x8, 0x1c, 0xa0beb0>), &g_vector_frees);
REWRITE("Vector::FreeField", 0xb5d460, vector_field_original<0x8>, (&vector_free_field<0x8, 0x1c, 0xb5b640>), &g_vector_frees);
REWRITE("Vector::FreeField", 0xb71870, vector_field_original<0x8>, (&vector_free_field<0x8, 0x1c, 0xb6b8f0>), &g_vector_frees);
REWRITE("Vector::FreeField", 0xcb4f20, vector_field_original<0xc>, (&vector_free_field<0xc, 0x10, 0xcb2b70>), &g_vector_frees);
REWRITE("Vector::FreeField", 0xcbe140, vector_field_original<0xc>, (&vector_free_field<0xc, 0xc, 0xcbd0e0>), &g_vector_frees);

// ---- Level::Reached: whether a level byte (+1 of a settings record, reached through two lookups) is at least LEVEL, and
// the field at +OFF is set ----------------------------------------------------------------------------------------------------
static volatile long g_level_checks;
template <uint32_t LEVEL, uint32_t OFF> static uint8_t __fastcall level_reached(uint8_t *self) {
    g_level_checks++;
    void *a = ((void *(__fastcall *)(void *))0xbb4280)(self);
    uint8_t *record = ((uint8_t * (__fastcall *)(void *))0xbb3e70)(a);
    uint8_t level = record[1];
    if (!level || level < LEVEL) return 0;
    return *(uint32_t *)(self + OFF) != 0;
}
template <uint32_t AT> static const uint8_t level_original[] = {
    0x56, 0x8b, 0xf1, 0xe8, (uint8_t)(0xbb4280 - (AT + 8)), (uint8_t)((0xbb4280 - (AT + 8)) >> 8),
    (uint8_t)((0xbb4280 - (AT + 8)) >> 16), (uint8_t)((0xbb4280 - (AT + 8)) >> 24)};
REWRITE("Level::Reached", 0x4e28f0, level_original<0x4e28f0>, (&level_reached<3, 0x1a0>), &g_level_checks);
REWRITE("Level::Reached", 0x4ed510, level_original<0x4ed510>, (&level_reached<3, 0x1a4>), &g_level_checks);
REWRITE("Level::Reached", 0x55eea0, level_original<0x55eea0>, (&level_reached<2, 0x178>), &g_level_checks);
REWRITE("Level::Reached", 0x55eee0, level_original<0x55eee0>, (&level_reached<2, 0x184>), &g_level_checks);
REWRITE("Level::Reached", 0x5a1200, level_original<0x5a1200>, (&level_reached<2, 0x180>), &g_level_checks);
REWRITE("Level::Reached", 0x5a12b0, level_original<0x5a12b0>, (&level_reached<3, 0x1b4>), &g_level_checks);
REWRITE("Level::Reached", 0x5d2710, level_original<0x5d2710>, (&level_reached<3, 0x1a8>), &g_level_checks);

// ---- Recycled::Delete: a deleting destructor whose block goes on a free list (head and count in globals) -----------------
static volatile long g_recycled;
template <uint32_t DESTROY, uint32_t HEAD, uint32_t COUNT> static uint32_t *__thiscall recycled_delete(uint32_t *self, uint32_t flags) {
    g_recycled++;
    ((void(__fastcall *)(void *))DESTROY)(self);
    if (flags & 1) {
        self[0] = *(uint32_t *)HEAD;
        *(uint32_t *)HEAD = (uint32_t)(uintptr_t)self;
        ++*(uint32_t *)COUNT;
    }
    return self;
}
template <uint32_t AT, uint32_t DESTROY> static const uint8_t recycled_original[] = {
    0x56, 0x8b, 0xf1, 0xe8, (uint8_t)(DESTROY - (AT + 8)), (uint8_t)((DESTROY - (AT + 8)) >> 8),
    (uint8_t)((DESTROY - (AT + 8)) >> 16), (uint8_t)((DESTROY - (AT + 8)) >> 24)};
REWRITE("Recycled::Delete", 0xbe8fb0, (recycled_original<0xbe8fb0, 0xbe8b10>), (&recycled_delete<0xbe8b10, 0x13d0eb4, 0x13d0ebc>), &g_recycled);
REWRITE("Recycled::Delete", 0xbeb790, (recycled_original<0xbeb790, 0xbeb7c0>), (&recycled_delete<0xbeb7c0, 0x13d0ee0, 0x13d0ee8>), &g_recycled);
REWRITE("Recycled::Delete", 0xbebac0, (recycled_original<0xbebac0, 0xbeba10>), (&recycled_delete<0xbeba10, 0x13d0f28, 0x13d0f30>), &g_recycled);
REWRITE("Recycled::Delete", 0xbebc80, (recycled_original<0xbebc80, 0xbebcb0>), (&recycled_delete<0xbebcb0, 0x13d0f04, 0x13d0f0c>), &g_recycled);
REWRITE("Recycled::Delete", 0xbff0b0, (recycled_original<0xbff0b0, 0xbff0e0>), (&recycled_delete<0xbff0e0, 0x13d2a30, 0x13d2a38>), &g_recycled);
REWRITE("Recycled::Delete", 0xbff3b0, (recycled_original<0xbff3b0, 0xbff310>), (&recycled_delete<0xbff310, 0x13d2a78, 0x13d2a80>), &g_recycled);
REWRITE("Recycled::Delete", 0xbff5e0, (recycled_original<0xbff5e0, 0xbff610>), (&recycled_delete<0xbff610, 0x13d2a54, 0x13d2a5c>), &g_recycled);
REWRITE("Recycled::Delete", 0xc9be00, (recycled_original<0xc9be00, 0xc9bab0>), (&recycled_delete<0xc9bab0, 0x13e6df4, 0x13e6dfc>), &g_recycled);

// ---- OwnerList::~OwnerList: deletes each item of the list at +4 ---------------------------------------------------------
static volatile long g_owner_lists;
template <uint32_t TABLE> static void __fastcall owner_list_destroy(uint32_t *self) {
    g_owner_lists++;
    self[0] = TABLE;
    Node *n = (Node *)(uintptr_t)self[1];
    if (!n->next) return;
    while (n) {
        void *item = n->value;
        Node *next = n->next->next ? n->next : nullptr;
        if (item) ((void(__thiscall *)(void *, int32_t))(*(void ***)item)[0])(item, 1);
        n = next;
    }
}
template <uint32_t TABLE> static const uint8_t owner_list_original[] = {
    0xc7, 0x01, (uint8_t)TABLE, (uint8_t)(TABLE >> 8), (uint8_t)(TABLE >> 16), (uint8_t)(TABLE >> 24), 0x8b, 0x49};
REWRITE("OwnerList::Destroy", 0xc0b4e0, owner_list_original<0xfafa90>, (&owner_list_destroy<0xfafa90>), &g_owner_lists);
REWRITE("OwnerList::Destroy", 0xc0b570, owner_list_original<0xfafa94>, (&owner_list_destroy<0xfafa94>), &g_owner_lists);
REWRITE("OwnerList::Destroy", 0xc0b600, owner_list_original<0xfafa98>, (&owner_list_destroy<0xfafa98>), &g_owner_lists);
REWRITE("OwnerList::Destroy", 0xc0b670, owner_list_original<0xfafa9c>, (&owner_list_destroy<0xfafa9c>), &g_owner_lists);
REWRITE("OwnerList::Destroy", 0xc0b700, owner_list_original<0xfafaa0>, (&owner_list_destroy<0xfafaa0>), &g_owner_lists);
REWRITE("OwnerList::Destroy", 0xc0b790, owner_list_original<0xfafaa4>, (&owner_list_destroy<0xfafaa4>), &g_owner_lists);
REWRITE("OwnerList::Destroy", 0xc0b820, owner_list_original<0xfafaa8>, (&owner_list_destroy<0xfafaa8>), &g_owner_lists);

// ---- SharedBlock: {references, ..., +0xc data}. Release drops a reference held at +OFF, freeing at none ---------------------
static volatile long g_block_releases;
template <uint32_t OFF> static void __fastcall shared_block_release(uint8_t *self) {
    g_block_releases++;
    int32_t *block = *(int32_t **)(self + OFF);
    if (--*block || !block) return;
    memory_free_aligned((void *)(uintptr_t)block[3]);
    memory_free(block);
}
template <uint32_t OFF> static const uint8_t block_release_original[] = {0x8b, 0x41, (uint8_t)OFF, 0xff, 0x08, 0x56, 0x8b, 0x71};
REWRITE("SharedBlock::Release", 0x44a0f0, block_release_original<0x4>, (&shared_block_release<0x4>), &g_block_releases);
REWRITE("SharedBlock::Release", 0x4c0c60, block_release_original<0x4>, (&shared_block_release<0x4>), &g_block_releases);
REWRITE("SharedBlock::Release", 0x66b330, block_release_original<0x8>, (&shared_block_release<0x8>), &g_block_releases);
REWRITE("SharedBlock::Release", 0x66c6f0, block_release_original<0x8>, (&shared_block_release<0x8>), &g_block_releases);
REWRITE("SharedBlock::Release", 0x746500, block_release_original<0x4>, (&shared_block_release<0x4>), &g_block_releases);
REWRITE("SharedBlock::Release", 0x7512b0, block_release_original<0x4>, (&shared_block_release<0x4>), &g_block_releases);
REWRITE("SharedBlock::Release", 0x7512e0, block_release_original<0x10>, (&shared_block_release<0x10>), &g_block_releases);
REWRITE("SharedBlock::Release", 0xa1d4b0, block_release_original<0x4>, (&shared_block_release<0x4>), &g_block_releases);
REWRITE("SharedBlock::Release", 0xbdf6a0, block_release_original<0x4>, (&shared_block_release<0x4>), &g_block_releases);

// A deleting destructor for an object owning a block at +OFF.
static volatile long g_field_deletes;
template <uint32_t OFF> static void *__thiscall field_delete(uint8_t *self, uint32_t flags) {
    g_field_deletes++;
    memory_free_aligned(*(void **)(self + OFF));
    if (flags & 1) memory_free(self);
    return self;
}
template <uint32_t OFF> static const uint8_t field_delete_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46, (uint8_t)OFF, 0x50, 0xe8};
REWRITE("Field::Delete", 0x401200, field_delete_original<0xc>, (&field_delete<0xc>), &g_field_deletes);
REWRITE("Field::Delete", 0x4035b0, field_delete_original<0xc>, (&field_delete<0xc>), &g_field_deletes);
REWRITE("Field::Delete", 0x59ea40, field_delete_original<0x4>, (&field_delete<0x4>), &g_field_deletes);
REWRITE("Field::Delete", 0xbcba60, field_delete_original<0x4>, (&field_delete<0x4>), &g_field_deletes);
REWRITE("Field::Delete", 0xc32450, field_delete_original<0xc>, (&field_delete<0xc>), &g_field_deletes);
REWRITE("Field::Delete", 0xc32480, field_delete_original<0x4>, (&field_delete<0x4>), &g_field_deletes);

// ---- Debug builds of small wrappers (these copies keep the compiler's stack checks, which do nothing here) --------------
static volatile long g_debug_wrappers;
template <uint32_t CALL> static uint32_t __fastcall debug_forward(void *self) {
    g_debug_wrappers++;
    return ((uint32_t(__fastcall *)(void *))CALL)(self);
}
template <uint32_t DESTROY> static void *__thiscall debug_delete(void *self, uint32_t flags) {
    g_debug_wrappers++;
    ((void(__fastcall *)(void *))DESTROY)(self);
    if (flags & 1) memory_free(self);
    return self;
}
static const uint8_t debug_original[] = {0x55, 0x8b, 0xec, 0x51, 0xc7, 0x45, 0xfc, 0xcc};
REWRITE("Debug::Forward", 0xcc1d70, debug_original, (&debug_forward<0xbe86e0>), &g_debug_wrappers);
REWRITE("Debug::Forward", 0xcc1f00, debug_original, (&debug_forward<0xbe88b0>), &g_debug_wrappers);
REWRITE("Debug::Forward", 0xcc2630, debug_original, (&debug_forward<0xcc25d0>), &g_debug_wrappers);
REWRITE("Debug::Forward", 0xcc4520, debug_original, (&debug_forward<0xbe86e0>), &g_debug_wrappers);
REWRITE("Debug::Forward", 0xcc45b0, debug_original, (&debug_forward<0xbe86e0>), &g_debug_wrappers);
REWRITE("Debug::Forward", 0xcc49d0, debug_original, (&debug_forward<0xbe88b0>), &g_debug_wrappers);
REWRITE("Debug::Forward", 0xcc4a00, debug_original, (&debug_forward<0xbe88b0>), &g_debug_wrappers);
REWRITE("Debug::Forward", 0xcc5610, debug_original, (&debug_forward<0xbe86e0>), &g_debug_wrappers);
REWRITE("Debug::Forward", 0xcc56d0, debug_original, (&debug_forward<0xbe88b0>), &g_debug_wrappers);
REWRITE("Debug::Delete", 0xcc1cd0, debug_original, (&debug_delete<0xcc1b90>), &g_debug_wrappers);
REWRITE("Debug::Delete", 0xcc4480, debug_original, (&debug_delete<0xcc4140>), &g_debug_wrappers);
REWRITE("Debug::Delete", 0xcc4700, debug_original, (&debug_delete<0xcc2ed0>), &g_debug_wrappers);
REWRITE("Debug::Delete", 0xcc5530, debug_original, (&debug_delete<0xcc5240>), &g_debug_wrappers);
REWRITE("Debug::Delete", 0xcc5570, debug_original, (&debug_delete<0xcc57d0>), &g_debug_wrappers);

// ---- Grid::MarkUp: flags a quadtree node and its parents (bit 4), stopping at one already flagged ------------------------
static volatile long g_grid_marks;
static void __thiscall grid_mark_up(uint8_t *self, int32_t index) {
    g_grid_marks++;
    for (;;) {
        uint32_t *node = (uint32_t *)(*(uint8_t **)(self + 8) + index * 0x1c);
        if (*node & 0x10) return;
        *node |= 0x10;
        if (!index) return;
        index = (index - 1) >> 2;
        if (index < 0) return;
    }
}
static const uint32_t grid_mark_copies[] = {0x42f670, 0x488320, 0x7b2a40, 0xa0be40, 0xb5b5d0, 0xb6b880};
static const uint8_t grid_mark_original[] = {0x56, 0x8b, 0x74, 0x24, 0x08, 0x57, 0x8b, 0xf9};
REWRITE_COPIES("Grid::MarkUp", grid_mark_copies, grid_mark_original, grid_mark_up, &g_grid_marks);

// ---- Names::Value: where a name's value is kept, or null when the name isn't there (see Names::Add) ------------------------
static volatile long g_name_values;
template <uint32_t FIND> static uint32_t *__thiscall names_value(uint8_t *self, void *name) {
    g_name_values++;
    int32_t at = (int32_t)(uintptr_t)name;   // the argument's slot is reused
    uint8_t found = ((uint8_t(__fastcall *)(void *, void *, int32_t, int32_t *, int32_t *))FIND)(
        name, *(void **)self, *(int32_t *)(self + 0x14), &at, &at);
    return found ? *(uint32_t **)(self + 8) + at : nullptr;
}
static const uint8_t names_value_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x56, 0x14, 0x8d, 0x44};
REWRITE("Names::Value", 0x7fbdf0, names_value_original, (&names_value<0x7fb900>), &g_name_values);
REWRITE("Names::Value", 0x7fd920, names_value_original, (&names_value<0x7fb9c0>), &g_name_values);
REWRITE("Names::Value", 0x8028c0, names_value_original, (&names_value<0x801450>), &g_name_values);
REWRITE("Names::Value", 0x805e80, names_value_original, (&names_value<0x805c10>), &g_name_values);
REWRITE("Names::Value", 0x808590, names_value_original, (&names_value<0x807970>), &g_name_values);
REWRITE("Names::Value", 0xb11c30, names_value_original, (&names_value<0xb117e0>), &g_name_values);

// ---- Pool::TakeSized: as Pool::Take, for items of a size that isn't a power of two -------------------------------------------
template <uint32_t GROW, uint32_t SIZE> static int32_t __fastcall pool_take_sized(uint8_t *self) {
    g_pool_takes++;
    int32_t *used = (int32_t *)(self + 8), *capacity = (int32_t *)(self + 0xc);
    if (*used >= *capacity) ((void(__thiscall *)(void *, int32_t))GROW)(self, *capacity > 0 ? *capacity * 2 : 200);
    int32_t item = (*(int32_t **)(self + 4))[*capacity - *used - 1];
    (*(uint8_t **)self)[(uint32_t)item * SIZE] = 1;
    ++*used;
    return item;
}
REWRITE("Pool::TakeSized", 0x409b60, pool_take_original, (&pool_take_sized<0x409660, 0x70>), &g_pool_takes);
REWRITE("Pool::TakeSized", 0x40d440, pool_take_original, (&pool_take_sized<0x40c6a0, 0x70>), &g_pool_takes);
REWRITE("Pool::TakeSized", 0x4d0980, pool_take_original, (&pool_take_sized<0x4d01a0, 0x2c>), &g_pool_takes);
REWRITE("Pool::TakeSized", 0xce7c80, pool_take_original, (&pool_take_sized<0xce78e0, 0x70>), &g_pool_takes);
REWRITE("Pool::TakeSized", 0xcec4c0, pool_take_original, (&pool_take_sized<0xcebe40, 0x70>), &g_pool_takes);

// ---- Array::NextIndex: the index of a new item, the array grown by 16 first when full -------------------------------------
static volatile long g_next_indexes;
template <uint32_t GROW> static int32_t __fastcall array_next_index(uint8_t *self) {
    g_next_indexes++;
    int32_t *count = (int32_t *)(self + 8);
    if (*count >= *(int32_t *)(self + 4)) ((void(__thiscall *)(void *, int32_t))GROW)(self, *count + 0x10);
    return (*count)++;
}
static const uint8_t next_index_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x08, 0x3b, 0x46};
REWRITE("Array::NextIndex", 0x7c0a60, next_index_original, (&array_next_index<0x7c06e0>), &g_next_indexes);
REWRITE("Array::NextIndex", 0x7c0ab0, next_index_original, (&array_next_index<0x7c0780>), &g_next_indexes);
REWRITE("Array::NextIndex", 0x7c0b00, next_index_original, (&array_next_index<0x7c0830>), &g_next_indexes);
REWRITE("Array::NextIndex", 0xa0ee90, next_index_original, (&array_next_index<0xa0ecb0>), &g_next_indexes);
REWRITE("Array::NextIndex", 0xa0eee0, next_index_original, (&array_next_index<0xa0ed40>), &g_next_indexes);
REWRITE("Array::NextIndex", 0xa0ef30, next_index_original, (&array_next_index<0xa0edf0>), &g_next_indexes);
REWRITE("Array::NextIndex", 0xb75db0, next_index_original, (&array_next_index<0xb75ba0>), &g_next_indexes);
REWRITE("Array::NextIndex", 0xb76260, next_index_original, (&array_next_index<0xb76170>), &g_next_indexes);

// The interface kept in a 28-byte reference, looked up the first time.
template <uint32_t ID> static void *ref_interface(uint8_t *ref) {
    if (!ref[0x18]) {
        *(void **)(ref + 0x14) = find_interface(*(uint8_t **)(ref + 4), (const int32_t *)ID);
        ref[0x18] = 1;
    }
    return *(void **)(ref + 0x14);
}
typedef void(__thiscall *Event2)(void *, uint32_t, uint32_t);

// ---- TrackedRide::PassOn: a two-argument event to the base class, then to the interfaces of the three lists of 28-byte
// references (+0xac, +0xb4, +0xbc; counts 4 further on) ------------------------------------------------------------------------
static volatile long g_ride_events;
template <uint32_t BASE, uint32_t EACH> static void __thiscall tracked_ride_pass(uint8_t *self, uint32_t a, uint32_t b) {
    g_ride_events++;
    ((Event2)BASE)(self, a, b);
    for (uint32_t list = 0xac; list <= 0xbc; list += 8)
        for (int32_t i = 0; i < *(int32_t *)(self + list + 4); i++) {
            uint8_t *ref = *(uint8_t **)(self + list) + i * 0x1c;
            ((Event2)EACH)(ref_interface<0x12f537c>(ref), a, b);
        }
}
static const uint8_t ride_pass_original[] = {0x51, 0x8b, 0x44, 0x24, 0x0c, 0x53, 0x55, 0x56};
REWRITE("TrackedRide::PassOn", 0x5d5cb0, ride_pass_original, (&tracked_ride_pass<0x8451d0, 0x5634c0>), &g_ride_events);
REWRITE("TrackedRide::PassOn", 0x5d5e80, ride_pass_original, (&tracked_ride_pass<0x845240, 0x563560>), &g_ride_events);
REWRITE("TrackedRide::PassOn", 0x5d6050, ride_pass_original, (&tracked_ride_pass<0x845240, 0x563600>), &g_ride_events);
REWRITE("TrackedRide::PassOn", 0x5d6220, ride_pass_original, (&tracked_ride_pass<0x845320, 0x563420>), &g_ride_events);

// ---- Ride::PassOn: the same kind of event for a ride: its listener (+0x418), a part (+0x404, when `a` is 0), each entry
// (references at +0x310) whose interface isn't flagged at +0x1c1, and six more listeners -------------------------------------------
static volatile long g_ride_passes;
template <uint32_t LISTEN, uint32_t PART, uint32_t ENTRY> static void __thiscall ride_pass(uint8_t *self, uint32_t a, uint32_t b) {
    g_ride_passes++;
    void *listener = *(void **)(self + 0x418);
    if (listener) ((Event2)LISTEN)(listener, a, b);
    void *part = *(void **)(self + 0x404);
    if (part && !a) ((void(__thiscall *)(void *, uint32_t))PART)(part, b);
    for (int32_t i = 0; i < *(int32_t *)(self + 0x314); i++) {
        uint8_t *ref = *(uint8_t **)(self + 0x310) + i * 0x1c;
        uint8_t *interface = (uint8_t *)ref_interface<0x12f515c>(ref);
        if (interface[0x1c1]) continue;
        void *entry = ((void *(__thiscall *)(void *, int32_t, uint32_t, uint32_t))0x510d90)(self, i, a, b);
        ((void(__fastcall *)(void *))ENTRY)(entry);
    }
    for (uint32_t k = 0; k < 4; k++) {
        listener = *(void **)(self + 0x438 + k * 0x90);
        if (listener) ((Event2)LISTEN)(listener, a, b);
    }
    for (uint32_t k = 0; k < 2; k++) {
        listener = *(void **)(self + 0x674 + k * 0x50);
        if (listener) ((Event2)LISTEN)(listener, a, b);
    }
}
static const uint8_t ride_pass2_original[] = {0x51, 0x53, 0x55, 0x8b, 0xe9, 0x8b, 0x8d, 0x18};
REWRITE("Ride::PassOn", 0x511950, ride_pass2_original, (&ride_pass<0x86cfb0, 0xb83320, 0x52b9e0>), &g_ride_passes);
REWRITE("Ride::PassOn", 0x511aa0, ride_pass2_original, (&ride_pass<0x86d000, 0xb83330, 0x52b9f0>), &g_ride_passes);
REWRITE("Ride::PassOn", 0x511bf0, ride_pass2_original, (&ride_pass<0x86d050, 0xb83340, 0x52ba00>), &g_ride_passes);
REWRITE("Ride::PassOn", 0x511d40, ride_pass2_original, (&ride_pass<0x86d0a0, 0xb83350, 0x52ba10>), &g_ride_passes);

// ---- List::RemoveValue: takes the first node holding `value` out of the list (an end node stops the search); the node, or null
static volatile long g_list_removes;
static Node *__thiscall list_remove_value(Node **list, void *value) {
    g_list_removes++;
    for (Node *n = *list; n->next; n = n->next)
        if (n->value == value) {
            unlink(n);
            return n;
        }
    return nullptr;
}
static const uint32_t list_remove_copies[] = {0x435730, 0x475f10, 0x49ec50, 0xacd780};
static const uint8_t list_remove_original[] = {0x8b, 0x01, 0x83, 0x38, 0x00, 0x74, 0x19, 0x85};
REWRITE_COPIES("List::RemoveValue", list_remove_copies, list_remove_original, list_remove_value, &g_list_removes);

// ---- Linked::DeleteAt: as Linked::Delete with the node at +OFF, and a broken list simply left -----------------------------------
static volatile long g_linked_at;
template <uint32_t OFF, uint32_t TABLE> static void *__thiscall linked_delete_at(uint32_t *self, uint32_t flags) {
    g_linked_at++;
    self[0] = TABLE;
    Node *n = (Node *)((uint8_t *)self + OFF);
    if (n->prev && n->next && n->next->prev == n && n->prev->next == n) {
        n->next->prev = n->prev;
        n->prev->next = n->next;
        n->next = nullptr;
        n->prev = nullptr;
    }
    if (flags & 1) memory_free(self);
    return self;
}
template <uint32_t OFF> static const uint8_t linked_at_original[] = {0x56, 0x8b, 0xf1, 0x8d, 0x46, (uint8_t)OFF, 0x85, 0xc0};
REWRITE("Linked::DeleteAt", 0x413810, linked_at_original<0x8>, (&linked_delete_at<0x8, 0xf26e00>), &g_linked_at);
REWRITE("Linked::DeleteAt", 0x593fa0, linked_at_original<0x4>, (&linked_delete_at<0x4, 0xf3cf10>), &g_linked_at);
REWRITE("Linked::DeleteAt", 0x7545f0, linked_at_original<0x70>, (&linked_delete_at<0x70, 0xf6c0bc>), &g_linked_at);
REWRITE("Linked::DeleteAt", 0xc1b2d0, linked_at_original<0x4>, (&linked_delete_at<0x4, 0xfaff14>), &g_linked_at);

// ---- List::DeleteValues: destroys and frees each node's value (nodes stay), stopping at the end or a broken link ------------------
static volatile long g_list_deletes;
template <uint32_t DESTROY> static void __fastcall list_delete_values(Node **list) {
    g_list_deletes++;
    Node *n = *list;
    if (!n->next) n = nullptr;
    while (n && n->prev && n->next && n->next->prev == n && n->prev->next == n) {
        void *value = n->value;
        Node *next = n->next;
        n = next->next ? next : nullptr;
        if (value) {
            ((void(__fastcall *)(void *))DESTROY)(value);
            memory_free(value);
        }
    }
}
static const uint8_t list_delete_original[] = {0x8b, 0x01, 0x56, 0x8b, 0x30, 0xf7, 0xde, 0x1b};
REWRITE("List::DeleteValues", 0x49bc60, list_delete_original, (&list_delete_values<0x49b6e0>), &g_list_deletes);
REWRITE("List::DeleteValues", 0x654280, list_delete_original, (&list_delete_values<0x64bf60>), &g_list_deletes);
REWRITE("List::DeleteValues", 0x6695b0, list_delete_original, (&list_delete_values<0x49b6e0>), &g_list_deletes);
REWRITE("List::DeleteValues", 0x7d4140, list_delete_original, (&list_delete_values<0x64bf60>), &g_list_deletes);

// ---- Compare::MappedOne: the second object's value mapped (the other side) against the first's as it is -----------------------
static volatile long g_mapped_ones;
template <int32_t STEP> static uint8_t __stdcall mapped_one(uint8_t *a, uint8_t *b, uint8_t side) {
    g_mapped_ones++;
    int32_t value = *(int32_t *)(b + 0xa4);
    if (!((uint8_t(__fastcall *)(uint8_t, uint8_t, int32_t *, int32_t))0x71b0f0)(b[0x8d], side ^ 1, &value, STEP)) return 0;
    return *(int32_t *)(a + 0xa4) == value;
}
static const uint8_t mapped_one_original[] = {0x8b, 0x44, 0x24, 0x08, 0x8b, 0x88, 0xa4, 0x00};
REWRITE("Compare::MappedOne", 0x71d0a0, mapped_one_original, (&mapped_one<2>), &g_mapped_ones);
REWRITE("Compare::MappedOne", 0x71d0f0, mapped_one_original, (&mapped_one<-2>), &g_mapped_ones);
REWRITE("Compare::MappedOne", 0x71d140, mapped_one_original, (&mapped_one<1>), &g_mapped_ones);
REWRITE("Compare::MappedOne", 0x71d190, mapped_one_original, (&mapped_one<-1>), &g_mapped_ones);

// ---- Refs::First: the interface of the first of an array of references, null when there are none ---------------------------------
static volatile long g_ref_firsts;
template <uint32_t ID> static void *__fastcall refs_first(uint8_t *self) {
    g_ref_firsts++;
    if (!*(uint32_t *)(self + 8)) return nullptr;
    return find_interface(*(uint8_t **)(*(uint8_t **)self + 4), (const int32_t *)ID);
}
static const uint8_t refs_first_original[] = {0x8b, 0x41, 0x08, 0x85, 0xc0, 0x74, 0x38, 0x8b};
REWRITE("Refs::First", 0x730b30, refs_first_original, (&refs_first<0x13063f0>), &g_ref_firsts);
REWRITE("Refs::First", 0x7385d0, refs_first_original, (&refs_first<0x130646c>), &g_ref_firsts);
REWRITE("Refs::First", 0xac2c50, refs_first_original, (&refs_first<0x13ce3f8>), &g_ref_firsts);
REWRITE("Refs::First", 0xba4d80, refs_first_original, (&refs_first<0x13d05fc>), &g_ref_firsts);

// ---- Bounds::Reset: two empty vectors and boxes: the box corners at +0x30 and +0x40 start at +-1e9 ---------------------------------
static volatile long g_bounds;
template <uint32_t TABLE> static uint32_t *__fastcall bounds_reset(uint32_t *self) {
    g_bounds++;
    self[5] = self[6] = self[7] = self[8] = 0;
    self[0x14] = self[0x15] = self[0x16] = self[0x17] = 0;
    self[0xc] = self[0xd] = self[0xe] = 0x4e6e6b28;
    self[0x10] = self[0x11] = self[0x12] = 0xce6e6b28;
    self[0] = TABLE;
    return self;
}
static const uint8_t bounds_original[] = {0x8b, 0xc1, 0x33, 0xc9, 0x89, 0x48, 0x14, 0x89};
REWRITE("Bounds::Reset", 0x891920, bounds_original, (&bounds_reset<0xf7a1f4>), &g_bounds);
REWRITE("Bounds::Reset", 0x891960, bounds_original, (&bounds_reset<0xf7a1fc>), &g_bounds);
REWRITE("Bounds::Reset", 0x8919a0, bounds_original, (&bounds_reset<0xf7a204>), &g_bounds);
REWRITE("Bounds::Reset", 0x8919e0, bounds_original, (&bounds_reset<0xf7a20c>), &g_bounds);

// ---- SharedSlots::~SharedSlots: lets go of each slot's value ----------------------------------------------------------------
static volatile long g_slots_destroys;
template <uint32_t TABLE, uint32_t COUNT> static void __fastcall shared_slots_destroy(uint32_t *self) {
    g_slots_destroys++;
    self[0] = TABLE;
    for (uint32_t i = 0; i < COUNT; i++) shared_release((Shared *)(uintptr_t)self[2 + i]);
}
template <uint32_t TABLE> static const uint8_t slots_destroy_original[] = {
    0x53, 0x56, 0x57, 0xc7, 0x01, (uint8_t)TABLE, (uint8_t)(TABLE >> 8), (uint8_t)(TABLE >> 16)};
REWRITE("SharedSlots::Destroy", 0xc0d740, slots_destroy_original<0xfaf9b0>, (&shared_slots_destroy<0xfaf9b0, 11>), &g_slots_destroys);
REWRITE("SharedSlots::Destroy", 0xc0d840, slots_destroy_original<0xfaf9b4>, (&shared_slots_destroy<0xfaf9b4, 24>), &g_slots_destroys);
REWRITE("SharedSlots::Destroy", 0xc0d9d0, slots_destroy_original<0xfaf9b8>, (&shared_slots_destroy<0xfaf9b8, 29>), &g_slots_destroys);
REWRITE("SharedSlots::Destroy", 0xc0dad0, slots_destroy_original<0xfaf9bc>, (&shared_slots_destroy<0xfaf9bc, 3>), &g_slots_destroys);
REWRITE("Pool::TakeSized", 0x49ec10, pool_take_original, (&pool_take_sized<0x49e3b0, 0x30>), &g_pool_takes);
REWRITE("Pool::TakeSized", 0x901d60, pool_take_original, (&pool_take_sized<0x8ffa60, 0x60>), &g_pool_takes);
REWRITE("Pool::TakeSized", 0x9506b0, pool_take_original, (&pool_take_sized<0x94e380, 0x60>), &g_pool_takes);
REWRITE("Pool::TakeSized", 0x9fe560, pool_take_original, (&pool_take_sized<0x9fe100, 0x30>), &g_pool_takes);

// ---- Chain::Delete: a deleting destructor that also deletes the next object of its chain, held at +OFF ------------------------
static volatile long g_chain_deletes;
template <uint32_t OFF> static void *__thiscall chain_delete(uint8_t *self, uint32_t flags) {
    g_chain_deletes++;
    void *next = *(void **)(self + OFF);
    if (next) chain_delete<OFF>((uint8_t *)next, 1);
    if (flags & 1) memory_free(self);
    return self;
}
template <uint32_t OFF> static const uint8_t chain_delete_original[] = {
    0x56, 0x8b, 0xf1, 0x8b, 0x8e, (uint8_t)OFF, (uint8_t)(OFF >> 8), (uint8_t)(OFF >> 16)};
REWRITE("Chain::Delete", 0x42bd60, chain_delete_original<0x504>, (&chain_delete<0x504>), &g_chain_deletes);
REWRITE("Chain::Delete", 0x42bd90, chain_delete_original<0x504>, (&chain_delete<0x504>), &g_chain_deletes);
REWRITE("Chain::Delete", 0x42bdc0, chain_delete_original<0x1704>, (&chain_delete<0x1704>), &g_chain_deletes);
REWRITE("Chain::Delete", 0x84db10, chain_delete_original<0xc04>, (&chain_delete<0xc04>), &g_chain_deletes);
REWRITE("Chain::Delete", 0x84db40, chain_delete_original<0x1004>, (&chain_delete<0x1004>), &g_chain_deletes);

// ---- Level::Value: as Level::Reached, giving the field itself ---------------------------------------------------------------
template <uint32_t LEVEL, uint32_t OFF> static uint32_t __fastcall level_value(uint8_t *self) {
    g_level_checks++;
    void *a = ((void *(__fastcall *)(void *))0xbb4280)(self);
    uint8_t *record = ((uint8_t * (__fastcall *)(void *))0xbb3e70)(a);
    uint8_t level = record[1];
    if (!level || level < LEVEL) return 0;
    return *(uint32_t *)(self + OFF);
}
REWRITE("Level::Value", 0x4d4430, level_original<0x4d4430>, (&level_value<2, 0x158>), &g_level_checks);
REWRITE("Level::Value", 0x5a12f0, level_original<0x5a12f0>, (&level_value<3, 0x1b8>), &g_level_checks);
REWRITE("Level::Value", 0x5bd150, level_original<0x5bd150>, (&level_value<3, 0x19c>), &g_level_checks);
REWRITE("Level::Value", 0x5d26a0, level_original<0x5d26a0>, (&level_value<2, 0x148>), &g_level_checks);
REWRITE("Level::Value", 0x846740, level_original<0x846740>, (&level_value<2, 0x150>), &g_level_checks);

// ---- Pool::Grow(n): {items, free list, used, capacity} grown to n: new items made (each constructed by CTOR), the old
// ones copied over (as plain data, or through COPY(dst, src) when given), the free list kept with the new items added to its
// end, the old blocks freed (COPY is __thiscall, the source on the stack). Nothing happens unless n is more than the capacity ------------------------------------------
static volatile long g_pool_grows;
template <uint32_t SIZE, uint32_t CTOR, uint32_t COPY> static void __thiscall pool_grow(uint8_t *self, int32_t n) {
    g_pool_grows++;
    int32_t *capacity = (int32_t *)(self + 0xc);
    if (n <= *capacity) return;
    uint8_t *old_items = *(uint8_t **)self;
    int32_t *old_free = *(int32_t **)(self + 4);
    uint8_t *items = (uint8_t *)memory_alloc_aligned((uint32_t)n * SIZE);
    if (items) ((void(__stdcall *)(void *, uint32_t, int32_t, void *))0x402b90)(items, SIZE, n, (void *)CTOR);
    *(uint8_t **)self = items;
    int32_t *free_list = (int32_t *)memory_alloc_aligned((uint32_t)n * 4);
    *(int32_t **)(self + 4) = free_list;
    for (int32_t i = 0; i < *capacity; i++) {
        uint8_t *to = *(uint8_t **)self + (uint32_t)i * SIZE, *from = old_items + (uint32_t)i * SIZE;
        if (COPY) ((void(__thiscall *)(void *, void *))COPY)(to, from);
        else for (uint32_t k = 0; k < SIZE; k += 4) *(uint32_t *)(to + k) = *(uint32_t *)(from + k);
    }
    int32_t free_count = *capacity - *(int32_t *)(self + 8);
    for (int32_t i = 0; i < free_count; i++) (*(int32_t **)(self + 4))[i] = old_free[i];
    for (int32_t i = *capacity; i < n; i++) (*(int32_t **)(self + 4))[free_count - *capacity + i] = i;
    if (old_items) memory_free_aligned(old_items);
    if (old_free) memory_free_aligned(old_free);
    *capacity = n;
}
static const uint8_t pool_grow_original[] = {0x64, 0xa1, 0x00, 0x00, 0x00, 0x00, 0x6a, 0xff};
template <uint32_t HANDLER> static const uint8_t pool_grow_late_original[] = {
    0x6a, 0xff, 0x68, (uint8_t)HANDLER, (uint8_t)(HANDLER >> 8), (uint8_t)(HANDLER >> 16), (uint8_t)(HANDLER >> 24)};
REWRITE("Pool::Grow42D730", 0x42d730, pool_grow_original, (&pool_grow<16, 0x42d190, 0>), &g_pool_grows);
REWRITE("Pool::Grow42D730", 0x42e780, pool_grow_original, (&pool_grow<16, 0x42e420, 0>), &g_pool_grows);
REWRITE("Pool::Grow42D730", 0x49a7e0, pool_grow_original, (&pool_grow<16, 0x49a010, 0>), &g_pool_grows);
REWRITE("Pool::Grow42D730", 0x80fa40, pool_grow_original, (&pool_grow<16, 0x80dd90, 0>), &g_pool_grows);
REWRITE("Pool::GrowCEF970", 0xcef970, pool_grow_late_original<0xea78db>, (&pool_grow<8, 0xcef670, 0>), &g_pool_grows);
REWRITE("Pool::GrowCEF970", 0xcefa90, pool_grow_late_original<0xea78fb>, (&pool_grow<8, 0xcef680, 0>), &g_pool_grows);
REWRITE("Pool::Grow8FFA60", 0x8ffa60, pool_grow_original, (&pool_grow<0x60, 0x8fd120, 0x8fde70>), &g_pool_grows);
REWRITE("Pool::Grow8FFA60", 0x94e380, pool_grow_original, (&pool_grow<0x60, 0x94c9e0, 0x94d9c0>), &g_pool_grows);

// ---- Small destructors (their exception frames only clean up after C++ exceptions, so they are left out) -----------------
static volatile long g_solo_dtors;
// Lets go of a shared block {references, ..., +0xc data}: freed with its data at none.
static void solo_block_release(int32_t *block) {
    --*block;
    if (*block || !block) return;
    memory_free_aligned((void *)(uintptr_t)block[3]);
    memory_free(block);
}
// Pair::Destroy: the shared blocks at +SECOND, then +FIRST.
template <uint32_t FIRST, uint32_t SECOND> static void __fastcall solo_pair_destroy(uint8_t *self) {
    g_solo_dtors++;
    solo_block_release(*(int32_t **)(self + SECOND));
    solo_block_release(*(int32_t **)(self + FIRST));
}
// Counted::Destroy: drops a counted member (+4 count; deleted at none), then the base destructor.
template <uint32_t OFF, uint32_t BASE> static void __fastcall solo_counted_destroy(uint8_t *self) {
    g_solo_dtors++;
    uint8_t *member = *(uint8_t **)(self + OFF);
    --*(int32_t *)(member + 4);
    member = *(uint8_t **)(self + OFF);
    if (!*(int32_t *)(member + 4) && member) ((void(__thiscall *)(void *, int32_t))(*(void ***)member)[0])(member, 1);
    ((void(__fastcall *)(void *))BASE)(self);
}
// Three tables, a member at +0x108 destroyed, then the base (0xAC7EB0).
template <uint32_t T0, uint32_t T1, uint32_t T2> static void __fastcall solo_tables_member(uint32_t *self) {
    g_solo_dtors++;
    self[0] = T0;
    self[1] = T1;
    self[0xd4 / 4] = T2;
    ((void(__fastcall *)(void *))0x40f730)((uint8_t *)self + 0x108);
    ((void(__fastcall *)(void *))0xac7eb0)(self);
}
// Three tables (+0, +0x14, +0x18), the owned object at +0x2c deleted, then the base (0x40DB80).
template <uint32_t T0, uint32_t T1, uint32_t T2> static void __fastcall solo_tables_owned(uint32_t *self) {
    g_solo_dtors++;
    self[0] = T0;
    self[5] = T1;
    self[6] = T2;
    void *owned = (void *)(uintptr_t)self[0xb];
    if (owned) ((void(__thiscall *)(void *, int32_t))(*(void ***)owned)[0])(owned, 1);
    ((void(__fastcall *)(void *))0x40db80)(self);
}
// The table, the block at +OFF freed and cleared, then the base table (0xF2694C).
template <uint32_t TABLE, uint32_t OFF> static void __fastcall solo_table_block(uint32_t *self) {
    g_solo_dtors++;
    self[0] = TABLE;
    void *&block = *(void **)((uint8_t *)self + OFF);
    if (block) {
        memory_free_aligned(block);
        block = nullptr;
    }
    self[0] = 0xf2694c;
}
// The table, the member at +0x1c destroyed (MEMBER), then the base (0x6A1DF0).
template <uint32_t TABLE, uint32_t MEMBER> static void __fastcall solo_table_member(uint32_t *self) {
    g_solo_dtors++;
    self[0] = TABLE;
    ((void(__fastcall *)(void *))MEMBER)((uint8_t *)self + 0x1c);
    ((void(__fastcall *)(void *))0x6a1df0)(self);
}

// ---- Array::ResizeSized: as Array::Resize61C320 for items of SIZE bytes (not a power of two) --------------------------------
static volatile long g_solo_resizes;
template <uint32_t SIZE, uintptr_t CTOR, uintptr_t ASSIGN> static int32_t __thiscall solo_resize(uint8_t **self, int32_t count) {
    g_solo_resizes++;
    int32_t old_count = *(int32_t *)(self + 1);
    if (count == old_count) return count;
    uint8_t *data = nullptr;
    if (count) {
        data = (uint8_t *)memory_alloc_aligned((uint32_t)count * SIZE);
        if (data) ((void(__stdcall *)(void *, uint32_t, int32_t, void *))0x402b90)(data, SIZE, count, (void *)CTOR);
        int32_t kept = count < old_count ? count : old_count;
        for (int32_t i = 0; i < kept; i++)
            ((void(__thiscall *)(uintptr_t, uintptr_t))ASSIGN)((uintptr_t)data + (uint32_t)i * SIZE, (uintptr_t)self[0] + (uint32_t)i * SIZE);
    }
    if (self[0]) {
        memory_free_aligned(self[0]);
        self[0] = nullptr;
    }
    self[0] = data;
    *(int32_t *)(self + 1) = count;
    return count;
}
static const uint8_t solo_o_53cbd0[] = {0x6a, 0xff, 0x68, 0xeb, 0xbc, 0xe1, 0x00, 0x64};
REWRITE("Pair::Destroy53CBD0", 0x53cbd0, solo_o_53cbd0, (&solo_pair_destroy<0x34, 0x38>), &g_solo_dtors);
static const uint8_t solo_o_7b4e80[] = {0x6a, 0xff, 0x68, 0x2b, 0x6e, 0xe5, 0x00, 0x64};
REWRITE("Pair::Destroy53CBD0", 0x7b4e80, solo_o_7b4e80, (&solo_pair_destroy<0x4, 0x8>), &g_solo_dtors);
static const uint8_t solo_o_c6c210[] = {0x6a, 0xff, 0x68, 0xb8, 0x32, 0xea, 0x00, 0x64};
REWRITE("Counted::DestroyC6C210", 0xc6c210, solo_o_c6c210, (&solo_counted_destroy<0xf4, 0xc69a30>), &g_solo_dtors);
static const uint8_t solo_o_c6e9d0[] = {0x6a, 0xff, 0x68, 0x18, 0x3a, 0xea, 0x00, 0x64};
REWRITE("Counted::DestroyC6C210", 0xc6e9d0, solo_o_c6e9d0, (&solo_counted_destroy<0x100, 0xc69a30>), &g_solo_dtors);
static const uint8_t solo_o_c6ee50[] = {0x6a, 0xff, 0x68, 0x08, 0x3b, 0xea, 0x00, 0x64};
REWRITE("Counted::DestroyC6C210", 0xc6ee50, solo_o_c6ee50, (&solo_counted_destroy<0xe8, 0xc67520>), &g_solo_dtors);
static const uint8_t solo_o_ac9810[] = {0x6a, 0xff, 0x68, 0xf8, 0x92, 0xe8, 0x00, 0x64};
REWRITE("Tables::DestroyAC9810", 0xac9810, solo_o_ac9810, (&solo_tables_member<0xf93b80, 0xf93b68, 0xf93b5c>), &g_solo_dtors);
static const uint8_t solo_o_ac9cf0[] = {0x6a, 0xff, 0x68, 0xb8, 0x93, 0xe8, 0x00, 0x64};
REWRITE("Tables::DestroyAC9810", 0xac9cf0, solo_o_ac9cf0, (&solo_tables_member<0xf93ca8, 0xf93c90, 0xf93c84>), &g_solo_dtors);
static const uint8_t solo_o_40dca0[] = {0x6a, 0xff, 0x68, 0x58, 0x54, 0xe0, 0x00, 0x64};
REWRITE("Tables::Destroy40DCA0", 0x40dca0, solo_o_40dca0, (&solo_tables_owned<0xf26988, 0xf26980, 0xf26978>), &g_solo_dtors);
static const uint8_t solo_o_40ddf0[] = {0x6a, 0xff, 0x68, 0x98, 0x54, 0xe0, 0x00, 0x64};
REWRITE("Tables::Destroy40DCA0", 0x40ddf0, solo_o_40ddf0, (&solo_tables_owned<0xf269ac, 0xf269a4, 0xf2699c>), &g_solo_dtors);
static const uint8_t solo_o_86f7b0[] = {0x6a, 0xff, 0x68, 0xd8, 0x26, 0xe6, 0x00, 0x64};
REWRITE("Block::Destroy86F7B0", 0x86f7b0, solo_o_86f7b0, (&solo_table_block<0xf7868c, 0x10>), &g_solo_dtors);
static const uint8_t solo_o_8e31e0[] = {0x6a, 0xff, 0x68, 0x48, 0x84, 0xe6, 0x00, 0x64};
REWRITE("Block::Destroy86F7B0", 0x8e31e0, solo_o_8e31e0, (&solo_table_block<0xf7c9dc, 0x14>), &g_solo_dtors);
static const uint8_t solo_o_555d70[] = {0x6a, 0xff, 0x68, 0xb8, 0xe0, 0xe1, 0x00, 0x64};
REWRITE("Member::Destroy555D70", 0x555d70, solo_o_555d70, (&solo_table_member<0xf3a090, 0x555dd0>), &g_solo_dtors);
static const uint8_t solo_o_556180[] = {0x6a, 0xff, 0x68, 0x18, 0xe1, 0xe1, 0x00, 0x64};
REWRITE("Member::Destroy555D70", 0x556180, solo_o_556180, (&solo_table_member<0xf3a1e4, 0x5561e0>), &g_solo_dtors);
static const uint8_t solo_o_46cfb0[] = {0x64, 0xa1, 0x00, 0x00, 0x00, 0x00, 0x6a, 0xff};
REWRITE("Array::ResizeSized46CFB0", 0x46cfb0, solo_o_46cfb0, (&solo_resize<0x50, 0x46c470, 0x46c480>), &g_solo_resizes);
static const uint8_t solo_o_939ac0[] = {0x64, 0xa1, 0x00, 0x00, 0x00, 0x00, 0x6a, 0xff};
REWRITE("Array::ResizeSized46CFB0", 0x939ac0, solo_o_939ac0, (&solo_resize<0x50, 0x939310, 0x939550>), &g_solo_resizes);
