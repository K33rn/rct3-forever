// Rewrites of wave 3.
#include "wave.h"

// The interface of an object itself, looked up by the id in the global at `id` (as find_interface, without a handle).
static inline void *w3_interface_of(void *object, const int32_t *id) {
    if (!object) return nullptr;
    ClassInterfaces *c = ((ClassInterfaces * (__fastcall *)(void *))(*(void ***)object)[1])(object);
    if (!c || *id >= c->count || !c->by_id[*id]) return nullptr;
    return (uint8_t *)object + c->by_id[*id]->offset;
}
// The two links every list here starts with.
struct Link {
    Link *next;
    Link *prev;
};
static inline void link_after(Link *head, Link *n) {
    n->next = head->next;
    head->next->prev = n;
    n->prev = head;
    head->next = n;
}

// ---- Record::Assign: copies a 64-byte record field by field, so its two bytes of padding stay as they were -------
static volatile long g_w3_assign;
static uint8_t *__thiscall record_assign(uint8_t *self, const uint8_t *from) {
    g_w3_assign++;
    *(uint32_t *)self = *(const uint32_t *)from;
    self[4] = from[4];
    self[5] = from[5];
    for (int i = 8; i < 0x40; i += 4) *(uint32_t *)(self + i) = *(const uint32_t *)(from + i);
    return self;
}
static const uint8_t record_assign_original[] = {0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x04, 0x8b, 0x11};
REWRITE("Record::Assign47E120", 0x47e120, record_assign_original, record_assign, &g_w3_assign);
REWRITE("Record::Assign47E120", 0x84d220, record_assign_original, record_assign, &g_w3_assign);

// ---- Slots::Reset: COUNT slots of 0x90 bytes from +0x10, each cleared with its second word set to 1 ----------------
// The words at +0x38..+0x3c, +0x4c and +0x5c of a slot are left alone.
static volatile long g_w3_slots;
template <uint32_t COUNT> static uint8_t *__fastcall slots_reset(uint8_t *self) {
    g_w3_slots++;
    for (uint32_t s = 0; s < COUNT; s++) {
        uint8_t *slot = self + 0x10 + s * 0x90;
        uint32_t *word = (uint32_t *)slot;
        word[0] = 0;
        word[1] = 1;
        word[2] = word[3] = 0;
        for (int i = 4; i < 0xe; i++) word[i] = 0;
        word[0x10] = word[0x11] = word[0x12] = 0;
        word[0x14] = word[0x15] = word[0x16] = 0;
        for (int i = 0x18; i < 0x1f; i++) word[i] = 0;
        slot[0x7c] = 0;
    }
    return self;
}
static const uint8_t slots_reset_original[] = {0x53, 0x56, 0x8b, 0xc1, 0x8d, 0x70, 0x10, 0x57};
REWRITE("Slots::ResetC71B30", 0xc71b30, slots_reset_original, (&slots_reset<4>), &g_w3_slots);
REWRITE("Slots::ResetC71B30", 0xc96450, slots_reset_original, (&slots_reset<8>), &g_w3_slots);

// ---- Base::Forward: a debug-built constructor that hands its two arguments to the base's -----------------------
static volatile long g_w3_forward;
static uint8_t *__thiscall base_forward(uint8_t *self, uint32_t a, uint32_t b) {
    g_w3_forward++;
    ((void(__thiscall *)(void *, uint32_t, uint32_t))0xbe79b0)(self, a, b);
    return self;
}
static const uint8_t base_forward_original[] = {0x55, 0x8b, 0xec, 0x51, 0xc7, 0x45, 0xfc, 0xcc};
REWRITE("Base::ForwardCC1EC0", 0xcc1ec0, base_forward_original, base_forward, &g_w3_forward);
REWRITE("Base::ForwardCC1EC0", 0xcc4950, base_forward_original, base_forward, &g_w3_forward);
REWRITE("Base::ForwardCC1EC0", 0xcc4990, base_forward_original, base_forward, &g_w3_forward);
REWRITE("Base::ForwardCC1EC0", 0xcc5690, base_forward_original, base_forward, &g_w3_forward);

// ---- Registered::Create: an object that joins the list its type has in the registry -------------------------------
static volatile long g_w3_registered;
template <uint32_t TABLE, uint32_t OFF> static uint32_t *__fastcall registered_create(uint32_t *self) {
    g_w3_registered++;
    self[0] = TABLE;
    self[3] = (uint32_t)(uintptr_t)self;
    uint8_t *registry = ((uint8_t * (__cdecl *)())0x440fd0)();
    link_after(*(Link **)(registry + OFF + 0x10), (Link *)(self + 1));
    return self;
}
static const uint8_t registered_original[] = {0x56, 0x8b, 0xf1, 0x57, 0xc7, 0x06};
REWRITE("Registered::Create441030", 0x441030, registered_original, (&registered_create<0xf28cc4, 0xc>), &g_w3_registered);
REWRITE("Registered::Create441030", 0x441060, registered_original, (&registered_create<0xf28cc8, 0x54>), &g_w3_registered);
REWRITE("Registered::Create441030", 0x441090, registered_original, (&registered_create<0xf28ccc, 0x24>), &g_w3_registered);
REWRITE("Registered::Create441030", 0x4410c0, registered_original, (&registered_create<0xf28cd4, 0x3c>), &g_w3_registered);

// ---- Tasks::FinishAll: finish every task of the global list that isn't finished, then carry on with the manager ---
static volatile long g_w3_finish;
template <uint32_t LIST, uint32_t MANAGER, uint32_t THEN> static uint32_t __cdecl tasks_finish_all() {
    g_w3_finish++;
    for (Node *n = *(Node **)LIST; n->next && n; n = n->next) {
        uint8_t *task = (uint8_t *)n->value;
        if (!(task[0x2c] & 1)) ((void(__fastcall *)(void *))0x800860)(task);
    }
    void *manager = ((void *(__cdecl *)())MANAGER)();
    return ((uint32_t(__fastcall *)(void *))THEN)(manager);
}
template <uint32_t LIST> static const uint8_t tasks_finish_original[] = {
    0x56, 0x8b, 0x35, (uint8_t)LIST, (uint8_t)(LIST >> 8), (uint8_t)(LIST >> 16), (uint8_t)(LIST >> 24)};
REWRITE("Tasks::FinishAll733270", 0x733270, tasks_finish_original<0x10ef734>, (&tasks_finish_all<0x10ef734, 0x477710, 0x731e10>), &g_w3_finish);
REWRITE("Tasks::FinishAll733270", 0x73b7c0, tasks_finish_original<0x10ef764>, (&tasks_finish_all<0x10ef764, 0x477900, 0x73a660>), &g_w3_finish);
REWRITE("Tasks::FinishAll733270", 0xac4980, tasks_finish_original<0x10f2684>, (&tasks_finish_all<0x10f2684, 0x477870, 0xac3a80>), &g_w3_finish);
REWRITE("Tasks::FinishAll733270", 0xba68d0, tasks_finish_original<0x10f2ab4>, (&tasks_finish_all<0x10f2ab4, 0x477240, 0xba5a90>), &g_w3_finish);

// ---- List::PushFront: a new node holding the value, first in the list kept at +OFF --------------------------------
static volatile long g_w3_push;
template <uint32_t OFF> static Node *__thiscall list_push_front(uint8_t *self, void *value) {
    g_w3_push++;
    Node *n = (Node *)memory_alloc(sizeof(Node));
    if (n) n->value = value;
    link_after(*(Link **)(self + OFF), (Link *)n);
    return n;
}
static const uint8_t list_push_original[] = {0x56, 0x6a, 0x0c, 0x8b, 0xf1, 0xe8};
REWRITE("List::PushFront531060", 0x531060, list_push_original, (&list_push_front<0x2ac>), &g_w3_push);
REWRITE("List::PushFront531060", 0x59ab30, list_push_original, (&list_push_front<0xc8>), &g_w3_push);
REWRITE("List::PushFront531060", 0x5c20a0, list_push_original, (&list_push_front<0x348>), &g_w3_push);
REWRITE("List::PushFront80B670", 0x80b670, list_push_original, (&list_push_front<0x14>), &g_w3_push);
REWRITE("List::PushFront80B670", 0x846410, list_push_original, (&list_push_front<0x10>), &g_w3_push);
REWRITE("List::PushFront80B670", 0x8464a0, list_push_original, (&list_push_front<0x28>), &g_w3_push);

// ---- Marker::Create: keeps its two arguments and flags each of the second's entries of level 2 or more -------------
static volatile long g_w3_marker;
template <uint32_t TABLE> static uint32_t *__thiscall marker_create(uint32_t *self, uint32_t a, uint8_t *b) {
    g_w3_marker++;
    self[1] = a;
    self[2] = (uint32_t)(uintptr_t)b;
    self[0] = TABLE;
    for (int32_t i = 0; i < *(int32_t *)(b + 0x24); i++) {
        uint8_t *entry = (*(uint8_t ***)(b + 0x28))[i];
        if (*(int32_t *)(entry + 0xc) >= 2) *(uint32_t *)(entry + 0x10) |= 1;
    }
    return self;
}
static const uint8_t marker_original[] = {0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x04, 0x56, 0x57};
REWRITE("Marker::CreateCCB3C0", 0xccb3c0, marker_original, (&marker_create<0xfc320c>), &g_w3_marker);
REWRITE("Marker::CreateCCB3C0", 0xcee340, marker_original, (&marker_create<0xfc88b8>), &g_w3_marker);

// ---- Ref::Is: whether a value is the interface kept in the reference at +0x23c --------------------------------------
static volatile long g_w3_ref_is;
static uint32_t __thiscall ref_is(uint8_t *self, void *value) {
    g_w3_ref_is++;
    return ref_interface<0x12f537c>(self + 0x23c) == value;
}
static const uint8_t ref_is_original[] = {0x57, 0x8b, 0xf9, 0x8a, 0x87, 0x54, 0x02, 0x00};
REWRITE("Ref::Is5639C0", 0x5639c0, ref_is_original, ref_is, &g_w3_ref_is);
REWRITE("Ref::Is5639C0", 0x563c50, ref_is_original, ref_is, &g_w3_ref_is);

// ---- Stream::ReadInt: four bytes from a stream, swapped if the stream's order isn't ours -------------------------
static volatile long g_w3_read_int;
static uint32_t __fastcall stream_read_int(void *stream, uint8_t swapped) {
    g_w3_read_int++;
    uint32_t value;
    ((void(__thiscall *)(void *, void *, uint32_t))0xbe5020)(stream, &value, 4);
    return swapped ? __builtin_bswap32(value) : value;
}
static const uint8_t read_int_original[] = {0x51, 0x53, 0x6a, 0x04, 0x8d, 0x44, 0x24, 0x08};
REWRITE("Stream::ReadIntC42410", 0xc42410, read_int_original, stream_read_int, &g_w3_read_int);
REWRITE("Stream::ReadIntC42410", 0xc42500, read_int_original, stream_read_int, &g_w3_read_int);

// ---- Owner::SetObject: takes the object, and tells the outer object (0xc before this part) its interface -----------
static volatile long g_w3_set_object;
template <uint32_t ID, uint32_t TELL> static uint8_t __thiscall owner_set_object(uint8_t *self, void *object) {
    g_w3_set_object++;
    ((void(__thiscall *)(void *, void *))0x501f80)(self, object);
    void *part = w3_interface_of(object, (const int32_t *)ID);
    ((void(__thiscall *)(void *, void *, uint32_t))TELL)(self - 0xc, part, 0);
    return 1;
}
static const uint8_t set_object_original[] = {0x56, 0x8b, 0x74, 0x24, 0x08, 0x57, 0x56, 0x8b};
REWRITE("Owner::SetObject4E0F20", 0x4e0f20, set_object_original, (&owner_set_object<0x12f4b54, 0x4dfb10>), &g_w3_set_object);
REWRITE("Owner::SetObject4E0F20", 0x7b02a0, set_object_original, (&owner_set_object<0x1309020, 0x7afc90>), &g_w3_set_object);
REWRITE("Owner::SetObject4E0F20", 0x9917a0, set_object_original, (&owner_set_object<0x1341354, 0x98e9a0>), &g_w3_set_object);
REWRITE("Owner::SetObject4E0F20", 0xb06f40, set_object_original, (&owner_set_object<0x13ce968, 0xb048a0>), &g_w3_set_object);

// ---- Refs::DeleteUnused: last to first, deletes each referenced interface whose count at +OFF is zero ---------------
static volatile long g_w3_refs_delete;
template <uint32_t ID, uint32_t OFF> static void __fastcall refs_delete_unused(uint8_t *self) {
    g_w3_refs_delete++;
    for (int32_t i = *(int32_t *)(self + 8) - 1; i >= 0; i--) {
        uint8_t *ref = *(uint8_t **)self + i * 0x14;
        uint8_t *part = (uint8_t *)find_interface(*(uint8_t **)(ref + 4), (const int32_t *)ID);
        if (!*(uint32_t *)(part + OFF)) ((void *(__thiscall *)(void *, uint32_t))(*(void ***)part)[0])(part, 1);
    }
}
static const uint8_t refs_delete_original[] = {0x53, 0x8b, 0xd9, 0x8b, 0x43, 0x08, 0x48, 0x78};
REWRITE("Refs::DeleteUnused730BF0", 0x730bf0, refs_delete_original, (&refs_delete_unused<0x13063f0, 0xcc>), &g_w3_refs_delete);
REWRITE("Refs::DeleteUnused730BF0", 0x738690, refs_delete_original, (&refs_delete_unused<0x130646c, 0xcc>), &g_w3_refs_delete);
REWRITE("Refs::DeleteUnused730BF0", 0xac2d10, refs_delete_original, (&refs_delete_unused<0x13ce3f8, 0xc0>), &g_w3_refs_delete);

// ---- Refs::FindNamed: the first referenced interface whose text at +OFF compares equal, null when none does ----------
static volatile long g_w3_refs_find;
template <uint32_t ID, uint32_t OFF> static void *__thiscall refs_find_named(uint8_t *self, void *name) {
    g_w3_refs_find++;
    for (int32_t i = 0; i < *(int32_t *)(self + 8); i++) {
        uint8_t *ref = *(uint8_t **)self + i * 0x14;
        uint8_t *part = (uint8_t *)find_interface(*(uint8_t **)(ref + 4), (const int32_t *)ID);
        if (!((int32_t(__thiscall *)(void *, void *))0xbce970)((void *)((uintptr_t)part + OFF), name)) return part;   // a null interface still compares, at address OFF
    }
    return nullptr;
}
static const uint8_t refs_find_original[] = {0x51, 0x53, 0x55, 0x8b, 0xd9, 0x8b, 0x43, 0x08};
REWRITE("Refs::FindNamed730AA0", 0x730aa0, refs_find_original, (&refs_find_named<0x13063f0, 0xc0>), &g_w3_refs_find);
REWRITE("Refs::FindNamed730AA0", 0x738540, refs_find_original, (&refs_find_named<0x130646c, 0xc0>), &g_w3_refs_find);
REWRITE("Refs::FindNamed730AA0", 0xac2bc0, refs_find_original, (&refs_find_named<0x13ce3f8, 0xb4>), &g_w3_refs_find);

// ---- Names::AddStem: adds a file name, without its 4-letter extension, to a growing block of terminated names --------
struct NameBlock {
    char unknown[0xc];
    int32_t capacity;   // +0xc
    char *text;         // +0x10
    int32_t size;       // +0x14
};
static volatile long g_w3_add_stem;
static int32_t __thiscall names_add_stem(NameBlock *self, const char *name) {
    g_w3_add_stem++;
    int32_t stem = (int32_t)__builtin_strlen(name) - 4;
    int32_t needed = self->size + stem + 1;
    if (needed >= self->capacity) {
        self->capacity *= 2;
        if (self->capacity <= needed) self->capacity = needed + 1;
        char *grown = (char *)memory_alloc_aligned(self->capacity);
        __builtin_memcpy(grown, self->text, self->size);
        memory_free_aligned(self->text);
        self->text = grown;
    }
    __builtin_memcpy(self->text + self->size, name, stem);
    self->size += stem + 1;
    self->text[self->size - 1] = 0;
    return self->size;
}
static const uint8_t add_stem_original[] = {0x51, 0x8b, 0x44, 0x24, 0x08, 0x53, 0x55, 0x56};
REWRITE("Names::AddStem87ED50", 0x87ed50, add_stem_original, names_add_stem, &g_w3_add_stem);
REWRITE("Names::AddStem87ED50", 0xb32670, add_stem_original, names_add_stem, &g_w3_add_stem);

// ---- Watchers::Destroy: empties the list at +8, clearing each value's back reference (+0xc8) -----------------------
static volatile long g_w3_watchers;
template <uint32_t TABLE> static void __fastcall watchers_destroy(uint32_t *self) {
    g_w3_watchers++;
    self[0] = TABLE;
    Node *n = (Node *)(uintptr_t)self[2];
    if (!n->next || !n) return;
    while (n) {
        Node *after = n->next->next ? n->next : nullptr;
        *(uint32_t *)((uint8_t *)n->value + 0xc8) = 0;
        unlink(n);
        memory_free(n);
        n = after;
    }
}
template <uint32_t TABLE> static const uint8_t watchers_original[] = {
    0xc7, 0x01, (uint8_t)TABLE, (uint8_t)(TABLE >> 8), (uint8_t)(TABLE >> 16), (uint8_t)(TABLE >> 24), 0x8b, 0x41};
REWRITE("Watchers::DestroyB53C70", 0xb53c70, watchers_original<0xf9a998>, (&watchers_destroy<0xf9a998>), &g_w3_watchers);
REWRITE("Watchers::DestroyB53C70", 0xb53ea0, watchers_original<0xf9a99c>, (&watchers_destroy<0xf9a99c>), &g_w3_watchers);

// ---- Refs::At: the interface of the reference at an index of the array kept at +OFF --------------------------------
static volatile long g_w3_refs_at;
template <uint32_t ID, uint32_t OFF> static void *__thiscall refs_at(uint8_t *self, int32_t index) {
    g_w3_refs_at++;
    return ref_interface<ID>(*(uint8_t **)(self + OFF) + index * 0x1c);
}
template <uint32_t OFF> static const uint8_t refs_at_original[] = {
    0x8b, 0x91, (uint8_t)OFF, (uint8_t)(OFF >> 8), (uint8_t)(OFF >> 16), (uint8_t)(OFF >> 24), 0x56, 0x8b};
REWRITE("Refs::At510D90", 0x510d90, refs_at_original<0x310>, (&refs_at<0x12f515c, 0x310>), &g_w3_refs_at);
REWRITE("Refs::At510D90", 0xa89450, refs_at_original<0xcc>, (&refs_at<0x13cde74, 0xcc>), &g_w3_refs_at);

// ---- List::FirstInterface: the first interface among the objects of the list at +0x74, whose links are at +4 ---------
struct ChainNode {
    void *unknown;
    ChainNode *next;
    void *object;
};
static volatile long g_w3_first;
template <uint32_t ID> static void *__fastcall list_first_interface(uint8_t *self) {
    g_w3_first++;
    ChainNode *n = *(ChainNode **)(self + 0x74);
    if (!n->next || !n) return nullptr;
    for (;;) {
        if (void *part = w3_interface_of(n->object, (const int32_t *)ID)) return part;
        ChainNode *after = n->next;
        if (!after->next) return nullptr;
        n = after;
    }
}
static const uint8_t first_interface_original[] = {0x56, 0x57, 0x8b, 0x79, 0x74, 0x8b, 0x47, 0x04};
REWRITE("List::FirstInterface4CCF50", 0x4ccf50, first_interface_original, (&list_first_interface<0x12f4b54>), &g_w3_first);
REWRITE("List::FirstInterface4CCF50", 0x4cd030, first_interface_original, (&list_first_interface<0x12f5544>), &g_w3_first);
