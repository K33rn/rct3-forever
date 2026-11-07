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

// ---- Array::Push: adds a value to the array {data, capacity, count} at +BASE, growing it first when full -----------
static volatile long g_w3_array_push;
template <uint32_t BASE, int32_t FIRST, uint32_t GROW> static void __thiscall array_push(uint8_t *self, uint32_t value) {
    g_w3_array_push++;
    uint8_t *array = self + BASE;
    int32_t &count = *(int32_t *)(array + 8);
    if (count >= *(int32_t *)(array + 4)) ((void(__thiscall *)(void *, int32_t))GROW)(array, count > 0 ? count * 2 : FIRST);
    (*(uint32_t **)array)[count] = value;
    count++;
}
static const uint8_t array_push_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46};
REWRITE("Array::Push80ED30", 0x80ed30, array_push_original, (&array_push<0x5c, 0x20, 0x80ebb0>), &g_w3_array_push);
REWRITE("Array::Push80ED30", 0x9f9060, array_push_original, (&array_push<0x6c, 0x20, 0x417c20>), &g_w3_array_push);
REWRITE("Array::Push80ED30", 0xa6bcc0, array_push_original, (&array_push<0x40, 0x80, 0xa6b660>), &g_w3_array_push);
REWRITE("Array::Push80ED30", 0xa6bda0, array_push_original, (&array_push<0x54, 0x80, 0xa6b830>), &g_w3_array_push);
REWRITE("Array::Push80ED30", 0xa6bde0, array_push_original, (&array_push<0x60, 0x80, 0xa6b8e0>), &g_w3_array_push);

// ---- Stream::Skip: moves a stream on by a distance; some copies take a third argument they don't use ----------------
static volatile long g_w3_skip;
static uint32_t stream_skip_by(void *stream, int32_t distance) {
    g_w3_skip++;
    int32_t at = ((int32_t(__fastcall *)(void *))0xbe40b0)(stream);
    return ((uint32_t(__thiscall *)(void *, int32_t))0xbe4090)(stream, at + distance);
}
static uint32_t __stdcall stream_skip(void *stream, int32_t distance) { return stream_skip_by(stream, distance); }
static uint32_t __stdcall stream_skip3(void *stream, int32_t distance, uint32_t) { return stream_skip_by(stream, distance); }
static const uint8_t stream_skip_original[] = {0x56, 0x8b, 0x74, 0x24, 0x08, 0x8b, 0xce, 0xe8};
REWRITE("Stream::SkipBBDF90", 0xbbdf90, stream_skip_original, stream_skip, &g_w3_skip);
REWRITE("Stream::SkipBBDF90", 0xbc95f0, stream_skip_original, stream_skip3, &g_w3_skip);
REWRITE("Stream::SkipBBDF90", 0xbc9840, stream_skip_original, stream_skip, &g_w3_skip);
REWRITE("Stream::SkipBBDF90", 0xbccaa0, stream_skip_original, stream_skip, &g_w3_skip);
REWRITE("Stream::SkipBBDF90", 0xce5e80, stream_skip_original, stream_skip3, &g_w3_skip);

// ---- FreeList::Add: puts a block on a global free list and counts it ------------------------------------------------
static volatile long g_w3_free_add;
template <uint32_t HEAD, uint32_t COUNT> static uint32_t __fastcall free_list_add(uint32_t *block) {
    g_w3_free_add++;
    *block = *(uint32_t *)HEAD;
    *(uint32_t *)HEAD = (uint32_t)(uintptr_t)block;
    return ++*(uint32_t *)COUNT;
}
template <uint32_t HEAD> static const uint8_t free_list_original[] = {
    0xa1, (uint8_t)HEAD, (uint8_t)(HEAD >> 8), (uint8_t)(HEAD >> 16), (uint8_t)(HEAD >> 24), 0x89, 0x01, 0xa1};
REWRITE("FreeList::AddBE8940", 0xbe8940, free_list_original<0x13d0eb4>, (&free_list_add<0x13d0eb4, 0x13d0ebc>), &g_w3_free_add);
REWRITE("FreeList::AddBE8940", 0xbe9910, free_list_original<0x13d0ee0>, (&free_list_add<0x13d0ee0, 0x13d0ee8>), &g_w3_free_add);
REWRITE("FreeList::AddBE8940", 0xbe9c70, free_list_original<0x13d0f04>, (&free_list_add<0x13d0f04, 0x13d0f0c>), &g_w3_free_add);
REWRITE("FreeList::AddBE8940", 0xbfd630, free_list_original<0x13d2a30>, (&free_list_add<0x13d2a30, 0x13d2a38>), &g_w3_free_add);
REWRITE("FreeList::AddBE8940", 0xbfd990, free_list_original<0x13d2a54>, (&free_list_add<0x13d2a54, 0x13d2a5c>), &g_w3_free_add);

// ---- Visitor::Run: a small visitor of the given type, holding this object, handed to 0x6ae7c0 -----------------------
static volatile long g_w3_visit;
template <uint32_t TABLE> static uint32_t __thiscall visitor_run(void *self) {
    g_w3_visit++;
    uint32_t visitor[2] = {TABLE, (uint32_t)(uintptr_t)self};
    return ((uint32_t(__thiscall *)(void *, void *))0x6ae7c0)(self, visitor);
}
static const uint8_t visitor_original[] = {0x83, 0xec, 0x08, 0x8d, 0x04, 0x24, 0x50};
REWRITE("Visitor::Run6AF3A0", 0x6af3a0, visitor_original, (&visitor_run<0xf60708>), &g_w3_visit);
REWRITE("Visitor::Run6AF3A0", 0x6af3c0, visitor_original, (&visitor_run<0xf60710>), &g_w3_visit);
REWRITE("Visitor::Run6AF3A0", 0x6af420, visitor_original, (&visitor_run<0xf60718>), &g_w3_visit);
REWRITE("Visitor::Run6AF3A0", 0x6af440, visitor_original, (&visitor_run<0xf60720>), &g_w3_visit);

// ---- Static::Init: builds a global object with its function and arranges its cleanup at exit --------------------------
static volatile long g_w3_static;
template <uint32_t OBJECT, uint32_t FN, uint32_t BUILD, uint32_t CLEANUP> static void __cdecl static_init() {
    g_w3_static++;
    ((void(__thiscall *)(void *, uint32_t, uint32_t))BUILD)((void *)OBJECT, FN, 0);
    ((int(__cdecl *)(uint32_t))0xddc870)(CLEANUP);
}
static const uint8_t static_init_original[] = {0x55, 0x8b, 0xec, 0x6a, 0x00, 0x68};
REWRITE("Static::InitF04320", 0xf04320, static_init_original, (&static_init<0x13f7088, 0xcc1880, 0xcc1ec0, 0xf25630>), &g_w3_static);
REWRITE("Static::InitF04320", 0xf04490, static_init_original, (&static_init<0x13f70e4, 0xcc2d40, 0xcc4950, 0xf25710>), &g_w3_static);
REWRITE("Static::InitF04320", 0xf044c0, static_init_original, (&static_init<0x13f70f0, 0xcc3eb0, 0xcc4990, 0xf25730>), &g_w3_static);
REWRITE("Static::InitF04320", 0xf13820, static_init_original, (&static_init<0x13fa010, 0xcc4f80, 0xcc5690, 0xf257b0>), &g_w3_static);

// ---- Value::Describe: asks the owner's table (at +0x84, then +8) to fill `out` for the value at +OFF ---------------
static volatile long g_w3_describe;
template <uint32_t OFF> static void *__thiscall value_describe(uint8_t *self, void *out) {
    g_w3_describe++;
    uint8_t *owner = ((uint8_t * (__fastcall *)(void *))0x833850)(self);
    uint8_t *table = *(uint8_t **)(owner + 0x84) + 8;
    ((void(__thiscall *)(void *, void *, uint32_t))(*(void ***)table)[2])(table, out, *(uint32_t *)(self + OFF));
    return out;
}
template <uint32_t AT> static const uint8_t describe_original[] = {
    0x56, 0x8b, 0xf1, 0xe8, (uint8_t)(0x833850 - (AT + 8)), (uint8_t)((0x833850 - (AT + 8)) >> 8),
    (uint8_t)((0x833850 - (AT + 8)) >> 16), (uint8_t)((0x833850 - (AT + 8)) >> 24)};
REWRITE("Value::Describe8B5320", 0x8b5320, describe_original<0x8b5320>, (&value_describe<0xf8>), &g_w3_describe);
REWRITE("Value::Describe8B5320", 0x8b5350, describe_original<0x8b5350>, (&value_describe<0x104>), &g_w3_describe);
REWRITE("Value::Describe8B5320", 0x8b5380, describe_original<0x8b5380>, (&value_describe<0xf4>), &g_w3_describe);

// ---- Block::Delete: a deleting destructor that frees the aligned block at +OFF ------------------------------------
static volatile long g_w3_block_delete;
template <uint32_t TABLE, uint32_t OFF> static uint32_t *__thiscall block_delete(uint32_t *self, uint8_t flags) {
    g_w3_block_delete++;
    self[0] = TABLE;
    memory_free_aligned(*(void **)((uint8_t *)self + OFF));
    if (flags & 1) memory_free(self);
    return self;
}
static const uint8_t block_delete_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46};
REWRITE("Block::DeleteBD6FB0", 0xbd6fb0, block_delete_original, (&block_delete<0xfa533c, 4>), &g_w3_block_delete);
REWRITE("Block::DeleteBD6FB0", 0xc015d0, block_delete_original, (&block_delete<0xfa5e00, 4>), &g_w3_block_delete);
REWRITE("Block::DeleteBD6FB0", 0xc73a30, block_delete_original, (&block_delete<0xfba0bc, 0xc>), &g_w3_block_delete);

// ---- Slot::Release: gives the slot number at +OFF back to the global pool, if it has one, then carries on -------------
static volatile long g_w3_slot_release;
template <uint32_t OFF, uint32_t POOL> static uint32_t __fastcall slot_release(uint8_t *self) {
    g_w3_slot_release++;
    int32_t &slot = *(int32_t *)(self + OFF);
    if (slot >= 0) {
        ((void(__thiscall *)(void *, int32_t))0xcdede0)((void *)POOL, slot);
        slot = -1;
    }
    return ((uint32_t(__fastcall *)(void *))0x8338b0)(self);
}
static const uint8_t slot_release_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46};
REWRITE("Slot::Release818F80", 0x818f80, slot_release_original, (&slot_release<0x58, 0x130b420>), &g_w3_slot_release);
REWRITE("Slot::Release818F80", 0x81ca20, slot_release_original, (&slot_release<0x60, 0x130b49c>), &g_w3_slot_release);
REWRITE("Slot::Release818F80", 0x8b44a0, slot_release_original, (&slot_release<0x60, 0x1311290>), &g_w3_slot_release);

// ---- Part::Clear: lets the part at +OFF go, if any, and clears the field at +FIELD (in some copies the part's own field) --
static volatile long g_w3_part_clear;
template <uint32_t OFF, uint32_t GO, uint32_t FIELD> static void __fastcall part_clear(uint8_t *self) {
    g_w3_part_clear++;
    void *part = *(void **)(self + OFF);
    if (part) ((void(__fastcall *)(void *))GO)(part);
    *(uint32_t *)(self + FIELD) = 0;
}
static const uint8_t part_clear_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x4e};
REWRITE("Part::Clear4E2C10", 0x4e2c10, part_clear_original, (&part_clear<0x74, 0x4edbf0, 0x1c>), &g_w3_part_clear);
REWRITE("Part::Clear4E2C10", 0xc3ab60, part_clear_original, (&part_clear<8, 0xc30fc0, 8>), &g_w3_part_clear);
REWRITE("Part::Clear4E2C10", 0xc3b960, part_clear_original, (&part_clear<8, 0xc30fc0, 8>), &g_w3_part_clear);

// ---- Setting::Set: stores one of two settings if it changed, and brings what depends on it up to date ------------------
static volatile long g_w3_setting;
template <uint32_t FIRST, uint32_t SECOND> static void __thiscall setting_set(uint8_t *self, int32_t second, int32_t value) {
    g_w3_setting++;
    int32_t &setting = *(int32_t *)(self + (second ? SECOND : FIRST));
    if (value == setting) return;
    setting = value;
    if (!second) {
        ((void(__fastcall *)(void *))0x8699c0)(self);
        return;
    }
    for (int32_t i = 0; i < *(int32_t *)(self + 0x108); i++) ((void(__thiscall *)(void *, int32_t))0x8625d0)(self, i);
}
static const uint8_t setting_original[] = {0x8b, 0x44, 0x24, 0x04, 0x56, 0x85, 0xc0, 0x8b};
REWRITE("Setting::Set86CFB0", 0x86cfb0, setting_original, (&setting_set<0x30, 0x3c>), &g_w3_setting);
REWRITE("Setting::Set86CFB0", 0x86d000, setting_original, (&setting_set<0x34, 0x40>), &g_w3_setting);
REWRITE("Setting::Set86CFB0", 0x86d050, setting_original, (&setting_set<0x38, 0x44>), &g_w3_setting);

// ---- Flags::SetVisible: bit 22 of the flags at +0x94 of the object at +OFF is set when hidden ----------------------
static volatile long g_w3_visible;
template <uint32_t OFF> static uint8_t *__thiscall flags_set_visible(uint8_t *self, uint8_t visible) {
    g_w3_visible++;
    uint8_t *object = *(uint8_t **)(self + OFF);
    uint32_t &flags = *(uint32_t *)(object + 0x94);
    flags = visible ? flags & ~0x400000u : flags | 0x400000;
    return object;
}
static const uint8_t visible_original[] = {0x8a, 0x44, 0x24, 0x04, 0x84, 0xc0, 0x8b, 0x81};
REWRITE("Flags::SetVisible6A7C70", 0x6a7c70, visible_original, (&flags_set_visible<0xe0>), &g_w3_visible);
REWRITE("Flags::SetVisible6A7C70", 0x6a7cb0, visible_original, (&flags_set_visible<0xdc>), &g_w3_visible);

// ---- SharedSlots::Copy: copies a set of shared slots, taking a count on each value ---------------------------------
static volatile long g_w3_slots_copy;
template <uint32_t TABLE, uint32_t COUNT> static uint32_t *__thiscall shared_slots_copy(uint32_t *self, const uint32_t *from) {
    g_w3_slots_copy++;
    self[0] = TABLE;
    self[1] = from[1];
    for (uint32_t i = 2; i < 2 + COUNT; i++) {
        uint16_t *value = (uint16_t *)(uintptr_t)from[i];
        self[i] = (uint32_t)(uintptr_t)value;
        if (value) ++*value;
    }
    return self;
}
static const uint8_t slots_copy_original[] = {0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x04, 0x56, 0xc7};
REWRITE("SharedSlots::CopyC09FA0", 0xc09fa0, slots_copy_original, (&shared_slots_copy<0xfaf9b0, 0xb>), &g_w3_slots_copy);
REWRITE("SharedSlots::CopyC09FA0", 0xc0a110, slots_copy_original, (&shared_slots_copy<0xfaf9b8, 0x1d>), &g_w3_slots_copy);

// ---- Kind::Matches: the other's kind is the one wanted, or both are of the paired kind with the other at stage 2 --------
static volatile long g_w3_matches;
template <int32_t WANTED, int32_t MINE> static uint8_t __thiscall kind_matches(uint8_t *self, uint8_t *other) {
    g_w3_matches++;
    int32_t kind = *(int32_t *)(other + 0x190);
    if (kind == WANTED) return 1;
    return *(int32_t *)(self + 0x7c) == MINE && kind == 0x10 && *(int32_t *)(other + 0x194) == 2;
}
static const uint8_t matches_original[] = {0x8b, 0x54, 0x24, 0x04, 0x8b, 0x82, 0x90, 0x01};
REWRITE("Kind::Matches772F10", 0x772f10, matches_original, (&kind_matches<4, 0x10>), &g_w3_matches);
REWRITE("Kind::Matches772F10", 0x772f40, matches_original, (&kind_matches<0xe, 0xe>), &g_w3_matches);

// ---- Pool::Take: marks the next free entry used (its first byte) and gives its index; doubles the pool when full ----
struct IndexPool {
    uint8_t *entries;
    int32_t *free_indices;
    int32_t used;
    int32_t capacity;
};
static volatile long g_w3_take;
template <uint32_t SIZE, uint32_t GROW> static int32_t __fastcall index_pool_take(IndexPool *self) {
    g_w3_take++;
    if (self->used >= self->capacity) ((void(__thiscall *)(void *, int32_t))GROW)(self, self->capacity > 0 ? self->capacity * 2 : 200);
    int32_t index = self->free_indices[self->capacity - self->used - 1];
    self->entries[index * SIZE] = 1;
    self->used++;
    return index;
}
static const uint8_t pool_take_w3_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x0c, 0x39, 0x46};
REWRITE("Pool::Take40D480", 0x40d480, pool_take_w3_original, (&index_pool_take<36, 0x40c7e0>), &g_w3_take);
REWRITE("Pool::Take40D480", 0x9f9b50, pool_take_w3_original, (&index_pool_take<36, 0x9f99d0>), &g_w3_take);
REWRITE("Pool::TakeCEFD20", 0xcefd20, pool_take_w3_original, (&index_pool_take<8, 0xcef970>), &g_w3_take);
REWRITE("Pool::TakeCEFD20", 0xcefd60, pool_take_w3_original, (&index_pool_take<8, 0xcefa90>), &g_w3_take);

// ---- Row::Draw: passes a row of the table at +0x9c, with this object's settings, to 0x8275d0 ----------------------
static volatile long g_w3_row;
template <uint32_t LOW, uint32_t HIGH> static uint32_t __thiscall row_draw(uint8_t *self, int32_t index, uint32_t a, uint32_t b) {
    g_w3_row++;
    uint8_t *table = *(uint8_t **)(self + 0x9c);
    uint8_t *row = *(uint8_t **)(table + 0x38) + index * 0x58;
    return ((uint32_t(__fastcall *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, void *))0x8275d0)(
        *(uint32_t *)(self + 0xa0), *(uint32_t *)(self + 0xa4), *(uint32_t *)(row + LOW), *(uint32_t *)(row + HIGH), a, b,
        *(uint32_t *)(self + 0xac), table);
}
static const uint8_t row_original[] = {0x8b, 0x91, 0x9c, 0x00, 0x00, 0x00, 0x8b, 0x44};
REWRITE("Row::Draw828FC0", 0x828fc0, row_original, (&row_draw<0x1c, 0x20>), &g_w3_row);
REWRITE("Row::Draw828FC0", 0x829010, row_original, (&row_draw<0x34, 0x38>), &g_w3_row);

// ---- Lists::DeleteAll: deletes the values of the lists at +4 and +0x1c; each value takes its node away when it goes ---
static volatile long g_w3_delete_all;
static void delete_list_values(Node *const volatile *list) {
    for (;;) {
        Node *n = *list;
        if (!n->next || !n) return;
        void *value = n->value;
        if (value) ((void *(__thiscall *)(void *, uint32_t))(*(void ***)value)[0])(value, 1);
    }
}
static void __fastcall lists_delete_all(uint8_t *self) {
    g_w3_delete_all++;
    delete_list_values((Node **)(self + 4));
    delete_list_values((Node **)(self + 0x1c));
}
static const uint8_t delete_all_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x04, 0x83, 0x38};
REWRITE("Lists::DeleteAllBFA860", 0xbfa860, delete_all_original, lists_delete_all, &g_w3_delete_all);
REWRITE("Lists::DeleteAllBFA860", 0xc2f1d0, delete_all_original, lists_delete_all, &g_w3_delete_all);

// ---- Named::Create: a debug-built constructor: two fields, then the name at +8 ------------------------------------
static volatile long g_w3_named;
static uint32_t *__thiscall named_create(uint32_t *self, uint32_t a, uint32_t b, uint32_t name) {
    g_w3_named++;
    self[0] = a;
    self[1] = b;
    ((void(__thiscall *)(void *, uint32_t))0x407350)(self + 2, name);
    return self;
}
static const uint8_t named_original[] = {0x55, 0x8b, 0xec, 0x51, 0xc7, 0x45, 0xfc, 0xcc};
REWRITE("Named::CreateCC3E60", 0xcc3e60, named_original, named_create, &g_w3_named);
REWRITE("Named::CreateCC3E60", 0xcc4f30, named_original, named_create, &g_w3_named);

// ---- Reader::Create: a reader of a range; the two copies keep start and end the other way round ---------------------
static volatile long g_w3_reader;
template <uint32_t KEPT, uint32_t CLEARED> static uint8_t *__thiscall reader_create(uint8_t *self, uint32_t a, uint32_t b, uint8_t flag) {
    g_w3_reader++;
    *(uint32_t *)(self + KEPT) = a;
    *(uint32_t *)(self + 0x14) = b;
    *(uint32_t *)(self + 4) = 0;
    *(uint32_t *)(self + CLEARED) = 0;
    self[0x18] = flag;
    *(uint32_t *)(self + 0x1c) = 0;
    return self;
}
static const uint8_t reader_original[] = {0x8b, 0x54, 0x24, 0x04, 0x8b, 0xc1, 0x89, 0x50};
REWRITE("Reader::CreateC30260", 0xc30260, reader_original, (&reader_create<0xc, 0x10>), &g_w3_reader);
REWRITE("Reader::CreateC30260", 0xc30290, reader_original, (&reader_create<0x10, 0xc>), &g_w3_reader);

// ---- Pick::Value: a field of one of two records by flag bit 0, passed through a conversion when bit 1 is set -----------
static volatile long g_w3_pick;
template <uint32_t OFF, uint32_t CONVERT> static uint32_t __fastcall pick_value(uint32_t *self) {
    g_w3_pick++;
    uint8_t flags = *(uint8_t *)(self + 1);
    uint8_t *record = (uint8_t *)(uintptr_t)self[0] + (flags & 1 ? 0x38 : 0x54);
    uint32_t value = *(uint32_t *)(record + OFF);
    return flags & 2 ? ((uint32_t(__fastcall *)(uint32_t))CONVERT)(value) : value;
}
static const uint8_t pick_original[] = {0xf6, 0x41, 0x04, 0x01, 0x8b, 0x01, 0x74, 0x05};
REWRITE("Pick::ValueBC54B0", 0xbc54b0, pick_original, (&pick_value<0x10, 0xbc4b50>), &g_w3_pick);
REWRITE("Pick::ValueBC54B0", 0xbc54e0, pick_original, (&pick_value<0x14, 0xbc4c00>), &g_w3_pick);

// ---- Three::Each: runs a function for each of the three items, with a flag -----------------------------------------
static volatile long g_w3_three;
template <uint32_t EACH> static void __fastcall three_each(uint8_t flag) {
    g_w3_three++;
    for (int32_t i = 0; i < 3; i++) ((void(__fastcall *)(int32_t, uint8_t))EACH)(i, flag);
}
static const uint8_t three_original[] = {0x53, 0x56, 0x8a, 0xd9, 0x33, 0xf6, 0x8a, 0xd3};
REWRITE("Three::Each47CA60", 0x47ca60, three_original, (&three_each<0x47c3f0>), &g_w3_three);
REWRITE("Three::Each47CA60", 0x47ca80, three_original, (&three_each<0x47c470>), &g_w3_three);

// ---- Reference::CreateAt: an empty reference at +OFF of a new object, in the null handle's list ------------------------
static volatile long g_w3_ref_create;
template <uint32_t OFF> static uint8_t *__fastcall reference_create_at(uint8_t *self) {
    g_w3_ref_create++;
    uint32_t *ref = (uint32_t *)(self + OFF);
    ref[0] = 0xf26ae0;
    ref[4] = (uint32_t)(uintptr_t)ref;
    uint8_t *handle = ((uint8_t * (__cdecl *)())0x805750)();
    ref[1] = (uint32_t)(uintptr_t)handle;
    link_after(*(Link **)(handle + 0x20), (Link *)(ref + 2));
    return self;
}
static const uint8_t ref_create_original[] = {0x56, 0x57, 0x8b, 0xf9, 0x8d, 0x77};
REWRITE("Reference::CreateAt7142F0", 0x7142f0, ref_create_original, (&reference_create_at<0x10>), &g_w3_ref_create);
REWRITE("Reference::CreateAt7142F0", 0x81da70, ref_create_original, (&reference_create_at<0x40>), &g_w3_ref_create);

// ---- Reference::Assign: this reference moves to the other's handle, then takes its three values -----------------------
static volatile long g_w3_ref_assign;
static uint8_t *__thiscall reference_assign(uint8_t *self, uint8_t *from) {
    g_w3_ref_assign++;
    unlink((Node *)(self + 8));
    ((void(__thiscall *)(void *, int32_t))0x8055c0)(*(void **)(self + 4), 0);
    uint8_t *handle = *(uint8_t **)(from + 4);
    ++*(int32_t *)(handle + 8);
    *(uint8_t **)(self + 4) = handle;
    link_after(*(Link **)(handle + 0x20), (Link *)(self + 8));
    ((void(__fastcall *)(void *))(*(void ***)self)[0])(self);
    for (int i = 0x14; i < 0x20; i += 4) *(uint32_t *)(self + i) = *(uint32_t *)(from + i);
    return self;
}
static const uint8_t ref_assign_original[] = {0x53, 0x56, 0x57, 0x8b, 0xf9, 0x8d, 0x77, 0x08};
REWRITE("Reference::Assign71F220", 0x71f220, ref_assign_original, reference_assign, &g_w3_ref_assign);
REWRITE("Reference::Assign71F220", 0x762980, ref_assign_original, reference_assign, &g_w3_ref_assign);

// ---- Reference::ResetAt: the reference at +OFF goes back to the null handle ----------------------------------------
static volatile long g_w3_ref_reset;
template <uint32_t OFF> static uint32_t __fastcall reference_reset_at(uint8_t *self) {
    g_w3_ref_reset++;
    uint8_t *ref = self + OFF;
    unlink((Node *)(ref + 8));
    ((void(__thiscall *)(void *, int32_t))0x8055c0)(*(void **)(ref + 4), 0);
    uint8_t *handle = ((uint8_t * (__cdecl *)())0x805750)();
    *(uint8_t **)(ref + 4) = handle;
    link_after(*(Link **)(handle + 0x20), (Link *)(ref + 8));
    return ((uint32_t(__fastcall *)(void *))(*(void ***)ref)[0])(ref);
}
template <uint8_t OFF> static const uint8_t ref_reset_original[] = {0x56, 0x57, 0x8d, 0x79, OFF, 0x8d, 0x77, 0x08};
REWRITE("Reference::ResetAt742710", 0x742710, ref_reset_original<0x48>, (&reference_reset_at<0x48>), &g_w3_ref_reset);
REWRITE("Reference::ResetAt742710", 0xb7bbd0, ref_reset_original<0x78>, (&reference_reset_at<0x78>), &g_w3_ref_reset);

// ---- Objects::Clear: deletes every object of the array at +0x214, frees it, and optionally finishes the main task ---------
static volatile long g_w3_objects;
static void __thiscall objects_clear(uint8_t *self, uint8_t finish) {
    g_w3_objects++;
    void **&objects = *(void ***)(self + 0x214);
    int32_t &count = *(int32_t *)(self + 0x218);
    for (int32_t i = 0; i < count; i++)
        if (void *object = objects[i]) ((void *(__thiscall *)(void *, uint32_t))(*(void ***)object)[0])(object, 1);
    if (objects) {
        memory_free_aligned(objects);
        objects = nullptr;
    }
    count = 0;
    if (finish) {
        uint8_t *task = ((uint8_t * (__cdecl *)())0x40a090)();
        if (!(task[0x2c] & 1)) ((void(__fastcall *)(void *))0x800860)(task);
    }
}
static const uint8_t objects_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x86, 0x18, 0x02, 0x00};
REWRITE("Objects::Clear9D0AC0", 0x9d0ac0, objects_original, objects_clear, &g_w3_objects);
REWRITE("Objects::Clear9D0AC0", 0x9d0b40, objects_original, objects_clear, &g_w3_objects);

// ---- Tree::Walk: visits a node's marked items, then its children (four to a node, picked by the low bits of its mask) ---
static volatile long g_w3_tree;
template <uint32_t VISIT> static void __thiscall tree_walk(uint8_t *self, int32_t index, uint32_t passed) {
    g_w3_tree++;
    uint8_t *node = *(uint8_t **)(self + 8) + index * 0x1c;
    for (int32_t i = 0; i < *(int32_t *)(node + 0x10); i++) {
        uint8_t *item = *(uint8_t **)(node + 0x18) + i * 0x20;
        if (*(uint32_t *)(item + 0x1c)) ((void(__fastcall *)(void *))VISIT)(*(void **)(item + 0x18));
    }
    for (int32_t child = 0; child < 4; child++)
        if (*(uint32_t *)node & (1u << child)) tree_walk<VISIT>(self, index * 4 + child + 1, passed);
}
static const uint8_t tree_original[] = {0x53, 0x55, 0x56, 0x8b, 0x74, 0x24, 0x10, 0x6b};
REWRITE("Tree::WalkA0E240", 0xa0e240, tree_original, (&tree_walk<0xa0a330>), &g_w3_tree);
REWRITE("Tree::WalkA0E240", 0xb74250, tree_original, (&tree_walk<0xb44a30>), &g_w3_tree);

// ---- Com::Query: gets the COM object of the source at +0x18 and asks it for an interface; E_NOTIMPL on any failure ------
static volatile long g_w3_com;
template <uint32_t IID> static uint32_t __thiscall com_query(uint8_t *self, void **out) {
    g_w3_com++;
    *out = nullptr;
    void *source = *(void **)(self + 0x18);
    void *object;
    if (((int32_t(__stdcall *)(void *, void **))(*(void ***)source)[6])(source, &object) < 0) return 0x80004001;
    void *found;
    int32_t result = ((int32_t(__stdcall *)(void *, uint32_t, void **))(*(void ***)object)[0])(object, IID, &found);
    ((uint32_t(__stdcall *)(void *))(*(void ***)object)[2])(object);
    if (result < 0) return 0x80004001;
    *out = found;
    return 0;
}
static const uint8_t com_original[] = {0x51, 0x57, 0x8b, 0x7c, 0x24, 0x0c, 0x8d, 0x54};
REWRITE("Com::QueryDFF1B0", 0xdff1b0, com_original, (&com_query<0x10451b4>), &g_w3_com);
REWRITE("Com::QueryDFF1B0", 0xdff220, com_original, (&com_query<0x1044b64>), &g_w3_com);

// ---- Registry::Find: the interface of the object registered under a key; a found object without it is reported ---------
static volatile long g_w3_find;
template <uint32_t KEY, uint32_t TABLE, uint32_t ID, uint32_t LINE> static void *__fastcall registry_find(void *self) {
    g_w3_find++;
    uint32_t key[2];
    ((void(__thiscall *)(void *, uint32_t))0xbf47b0)(key, *(uint32_t *)KEY);
    key[0] = TABLE;
    void *object = ((void *(__thiscall *)(void *, void *))0xbf5b60)(self, key);
    if (!object) return nullptr;
    if (void *part = w3_interface_of(object, (const int32_t *)ID)) return part;
    ((void(__fastcall *)(uint32_t, uint32_t))0xbec8a0)(0xfc0e04, LINE);
    return nullptr;
}
template <uint32_t KEY> static const uint8_t registry_find_original[] = {
    0x83, 0xec, 0x08, 0xa1, (uint8_t)KEY, (uint8_t)(KEY >> 8), (uint8_t)(KEY >> 16), (uint8_t)(KEY >> 24)};
REWRITE("Registry::FindCB8DE0", 0xcb8de0, registry_find_original<0x13e6fa0>, (&registry_find<0x13e6fa0, 0xfc0d14, 0x13e6fdc, 0xc9>), &g_w3_find);
REWRITE("Registry::FindCB8DE0", 0xcb8f10, registry_find_original<0x13e6fa4>, (&registry_find<0x13e6fa4, 0xfc0d60, 0x13e6fcc, 0x163>), &g_w3_find);
REWRITE("Registry::FindCB8DE0", 0xcb9020, registry_find_original<0x13e6f98>, (&registry_find<0x13e6f98, 0xfc0d88, 0x13e6fb8, 0x1dc>), &g_w3_find);
REWRITE("Registry::FindCB8DE0", 0xcb9130, registry_find_original<0x13e6f9c>, (&registry_find<0x13e6f9c, 0xfc0db0, 0x13e6f80, 0x24f>), &g_w3_find);

// ---- List::PushCounted: as List::PushFront, for a list {head, ?, count} at +OFF -----------------------------------
template <uint32_t OFF> static Node *__thiscall list_push_counted(uint8_t *self, void *value) {
    Node *n = list_push_front<OFF>(self, value);
    ++*(int32_t *)(self + OFF + 8);
    return n;
}
REWRITE("List::PushCounted4CCDE0", 0x4ccde0, list_push_original, (&list_push_counted<0x74>), &g_w3_push);
REWRITE("List::PushCounted4CCDE0", 0xb91240, list_push_original, (&list_push_counted<0x2c>), &g_w3_push);

// ---- Array::Claim: the index of a new entry at the end, making room for `more` when the array is full ------------------
static volatile long g_w3_claim;
template <uint32_t GROW> static int32_t __thiscall array_claim(int32_t *self, int32_t more) {
    g_w3_claim++;
    int32_t count = self[2], capacity = self[1];
    if (count >= capacity && count + more > capacity) ((void(__thiscall *)(void *, int32_t))GROW)(self, count + more);
    return self[2]++;
}
static const uint8_t claim_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x08, 0x8b, 0x4e};
REWRITE("Array::Claim7E5EE0", 0x7e5ee0, claim_original, (&array_claim<0x7e5400>), &g_w3_claim);
REWRITE("Array::Claim7E5EE0", 0xaaad60, claim_original, (&array_claim<0xaaabf0>), &g_w3_claim);

// ---- Items::Forward: finds an item by key and passes the other four arguments on to it ------------------------------
static volatile long g_w3_items;
template <uint32_t CALL> static uint32_t __thiscall items_forward(void *self, uint32_t key, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    g_w3_items++;
    void *item = ((void *(__thiscall *)(void *, uint32_t, uint32_t *))0x8fd720)(self, key, &key);
    if (!item) return 0;
    return ((uint32_t(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))CALL)(item, a, b, c, d);
}
static const uint8_t items_original[] = {0x8b, 0x54, 0x24, 0x04, 0x8d, 0x44, 0x24, 0x04};
REWRITE("Items::Forward9024F0", 0x9024f0, items_original, (&items_forward<0x901810>), &g_w3_items);
REWRITE("Items::Forward9024F0", 0x902530, items_original, (&items_forward<0x901a00>), &g_w3_items);

// ---- Chain::Run: from the first value of the list at +OFF, runs each object and moves to the one it gives next ----------
static volatile long g_w3_chain;
template <uint32_t OFF, uint32_t RUN, uint32_t NEXT> static void __fastcall chain_run(uint8_t *self) {
    g_w3_chain++;
    Node *list = *(Node **)(self + OFF);
    void *object = list->next && list ? list->value : nullptr;
    while (object) {
        ((void(__fastcall *)(void *))RUN)(object);
        object = ((void *(__fastcall *)(void *))NEXT)(object);
    }
}
template <uint8_t OFF> static const uint8_t chain_run_original[] = {0x8b, 0x41, OFF, 0x83, 0x38, 0x00, 0x74, 0x13};
REWRITE("Chain::Run4CDB90", 0x4cdb90, chain_run_original<0x64>, (&chain_run<0x64, 0x4bf080, 0x4bf970>), &g_w3_chain);
REWRITE("Chain::Run4CDB90", 0xb91450, chain_run_original<0x1c>, (&chain_run<0x1c, 0xb8a970, 0xb85be0>), &g_w3_chain);

// ---- Global::CreateBlock: a new zeroed block of six words kept in a global (null if none could be had) -------------------
static volatile long g_w3_global_block;
template <uint32_t GLOBAL> static uint32_t *__cdecl global_create_block() {
    g_w3_global_block++;
    uint32_t *block = (uint32_t *)memory_alloc(0x18);
    if (block) for (int i = 0; i < 6; i++) block[i] = 0;
    *(uint32_t **)GLOBAL = block;
    return block;
}
template <uint32_t AT> static const uint8_t global_block_original[] = {
    0x6a, 0x18, 0xe8, (uint8_t)(0xbcdfc0 - (AT + 7)), (uint8_t)((0xbcdfc0 - (AT + 7)) >> 8),
    (uint8_t)((0xbcdfc0 - (AT + 7)) >> 16), (uint8_t)((0xbcdfc0 - (AT + 7)) >> 24)};
REWRITE("Global::CreateBlock4097E0", 0x4097e0, global_block_original<0x4097e0>, (&global_create_block<0x10ff9c4>), &g_w3_global_block);
REWRITE("Global::CreateBlock4097E0", 0x42d860, global_block_original<0x42d860>, (&global_create_block<0x1103be0>), &g_w3_global_block);

// ---- Member::Delete: a deleting destructor that leaves the manager (global 0x13d0d84) if the object is still in it ---------
static volatile long g_w3_member;
template <uint32_t TABLE, uint32_t JOINED, uint32_t LEAVE> static uint32_t *__thiscall member_delete(uint32_t *self, uint8_t flags) {
    g_w3_member++;
    self[0] = TABLE;
    uint8_t &joined = *((uint8_t *)self + JOINED);
    if (joined) {
        joined = 0;
        ((void(__thiscall *)(void *, void *))LEAVE)(*(void **)0x13d0d84, self);
    }
    if (flags & 1) memory_free(self);
    return self;
}
static const uint8_t member_delete_original[] = {0x56, 0x8b, 0xf1, 0x8a, 0x86};
REWRITE("Member::DeleteBD3880", 0xbd3880, member_delete_original, (&member_delete<0xfa1538, 0x98, 0xbd2960>), &g_w3_member);
REWRITE("Member::DeleteBD3880", 0xbd38c0, member_delete_original, (&member_delete<0xfa153c, 0x118, 0xbd29e0>), &g_w3_member);

// ---- Member::Create: joins the list the manager (global 0x13d0d84) keeps at +LIST --------------------------------------
template <uint32_t TABLE, uint32_t JOINED, uint32_t LIST> static uint32_t *__fastcall member_create(uint32_t *self) {
    g_w3_member++;
    self[0] = TABLE;
    self[3] = (uint32_t)(uintptr_t)self;
    uint8_t *bytes = (uint8_t *)self;
    bytes[JOINED] = 1;
    *(uint32_t *)(bytes + JOINED - 8) = 0;
    *(uint32_t *)(bytes + JOINED - 4) = 0;
    link_after(*(Link **)(*(uint8_t **)0x13d0d84 + LIST), (Link *)(self + 1));
    return self;
}
template <uint32_t TABLE> static const uint8_t member_create_original[] = {
    0x8b, 0xc1, 0xc7, 0x00, (uint8_t)TABLE, (uint8_t)(TABLE >> 8), (uint8_t)(TABLE >> 16), (uint8_t)(TABLE >> 24)};
REWRITE("Member::CreateBD37A0", 0xbd37a0, member_create_original<0xfa1538>, (&member_create<0xfa1538, 0x98, 0x44>), &g_w3_member);
REWRITE("Member::CreateBD37A0", 0xbd3810, member_create_original<0xfa153c>, (&member_create<0xfa153c, 0x118, 0x5c>), &g_w3_member);

// ---- Buffers::Destroy: frees the two aligned blocks and lets the part at +8 go -----------------------------------------
static volatile long g_w3_buffers;
static void __fastcall buffers_destroy(uint32_t *self) {
    g_w3_buffers++;
    self[0] = 0xfb7158;
    memory_free_aligned((void *)(uintptr_t)self[5]);
    memory_free_aligned((void *)(uintptr_t)self[8]);
    ((void(__fastcall *)(uint32_t))0xbce060)(self[2]);
}
static const uint8_t buffers_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x14, 0x50, 0xc7};
REWRITE("Buffers::DestroyC46CB0", 0xc46cb0, buffers_original, buffers_destroy, &g_w3_buffers);
REWRITE("Buffers::DestroyC46CB0", 0xc481f0, buffers_original, buffers_destroy, &g_w3_buffers);

// ---- Control::Create: the base built with fixed arguments; only the third argument is kept -------------------------------
static volatile long g_w3_control;
template <uint32_t TABLE, uint32_t SECOND> static uint32_t *__thiscall control_create(uint32_t *self, uint32_t, uint32_t, uint32_t kept) {
    g_w3_control++;
    ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))0x6aae60)(self, 0, 0, 0x10);
    *(uint32_t *)((uint8_t *)self + 0x114) = kept;
    self[0] = TABLE;
    self[1] = SECOND;
    return self;
}
static const uint8_t control_original[] = {0x56, 0x6a, 0x10, 0x6a, 0x00, 0x6a, 0x00, 0x8b};
REWRITE("Control::Create7E2810", 0x7e2810, control_original, (&control_create<0xf729d8, 0xf729c0>), &g_w3_control);
REWRITE("Control::Create7E2810", 0x7e29b0, control_original, (&control_create<0xf72b08, 0xf72af0>), &g_w3_control);

// ---- List::DeleteAll: deletes the values of the list the object points to ---------------------------------------------
static void __fastcall list_delete_all(uint8_t *self) {
    g_w3_delete_all++;
    delete_list_values((Node **)self);
}
static const uint8_t list_delete_all_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x06, 0x83, 0x38, 0x00};
REWRITE("List::DeleteAll673AC0", 0x673ac0, list_delete_all_original, list_delete_all, &g_w3_delete_all);
REWRITE("List::DeleteAll673AC0", 0x675550, list_delete_all_original, list_delete_all, &g_w3_delete_all);

// ---- Stock::Take: takes some from the store at +0x54 if it can, lowers the byte at +OFF and updates ---------------------
static volatile long g_w3_stock;
template <uint32_t TAKE, uint32_t OFF> static uint8_t __thiscall stock_take(uint8_t *self, uint32_t amount) {
    g_w3_stock++;
    if (!((uint8_t(__thiscall *)(void *, uint32_t))TAKE)(self + 0x54, amount)) return 0;
    self[OFF] -= (uint8_t)amount;
    ((void(__fastcall *)(void *))0x723e50)(self);
    return 1;
}
static const uint8_t stock_original[] = {0x53, 0x8b, 0x5c, 0x24, 0x08, 0x56, 0x8b, 0xf1};
REWRITE("Stock::Take7242C0", 0x7242c0, stock_original, (&stock_take<0x72f820, 0x5c>), &g_w3_stock);
REWRITE("Stock::Take7242C0", 0x7243e0, stock_original, (&stock_take<0x72fc30, 0x5d>), &g_w3_stock);

// ---- Block::Clear: frees the aligned block at +BLOCK, if any, and zeroes it and its size at +BLOCK + 4 --------------------
static volatile long g_w3_block_clear;
template <uint32_t BLOCK> static void __fastcall block_clear(uint8_t *self) {
    g_w3_block_clear++;
    void *&block = *(void **)(self + BLOCK);
    if (block) {
        memory_free_aligned(block);
        block = nullptr;
    }
    *(uint32_t *)(self + BLOCK + 4) = 0;
}
static const uint8_t block_clear_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x86};
REWRITE("Block::Clear66A7E0", 0x66a7e0, block_clear_original, (&block_clear<0xdc>), &g_w3_block_clear);
REWRITE("Block::Clear66A7E0", 0xb42cb0, block_clear_original, (&block_clear<0x270>), &g_w3_block_clear);

// ---- Resource::Release: gives the handle at +0x1c back to the owner's store, and forgets it ------------------------------
static volatile long g_w3_resource;
static void __fastcall resource_release(uint32_t *self) {
    g_w3_resource++;
    void *store = *(void **)((uint8_t *)(uintptr_t)self[5] + 0x470);
    ((void(__thiscall *)(void *, uint32_t))0xc5cb30)(store, self[7]);
    self[7] = 0;
}
static const uint8_t resource_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x1c, 0x8b, 0x4e};
REWRITE("Resource::ReleaseBFBE10", 0xbfbe10, resource_original, resource_release, &g_w3_resource);
REWRITE("Resource::ReleaseBFBE10", 0xbfbe60, resource_original, resource_release, &g_w3_resource);

// ---- Slot::Return: gives the slot number at +0x28 back to the manager, if it has one --------------------------------------
static volatile long g_w3_slot_return;
static void __fastcall slot_return(int32_t *self) {
    g_w3_slot_return++;
    if (self[0xa] < 0) return;
    void *manager = ((void *(__cdecl *)())0x425920)();
    ((void(__thiscall *)(void *, int32_t))0x9fe2a0)(manager, self[0xa]);
    self[0xa] = -1;
}
static const uint8_t slot_return_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x28, 0x85, 0xc0};
REWRITE("Slot::Return5D37F0", 0x5d37f0, slot_return_original, slot_return, &g_w3_slot_return);
REWRITE("Slot::Return5D37F0", 0x5d40f0, slot_return_original, slot_return, &g_w3_slot_return);

// ---- Result::Make: fills `out` through a function taking it and one argument, and gives `out` back -----------------------
static volatile long g_w3_make;
template <uint32_t FILL> static void *__stdcall result_make(void *out, uint32_t a) {
    g_w3_make++;
    ((void(__fastcall *)(void *, uint32_t))FILL)(out, a);
    return out;
}
static const uint8_t make_original[] = {0x51, 0x8b, 0x54, 0x24, 0x0c, 0x56, 0x8b, 0x74};
REWRITE("Result::Make48D5B0", 0x48d5b0, make_original, (&result_make<0x442570>), &g_w3_make);
REWRITE("Result::Make48D5B0", 0x48d650, make_original, (&result_make<0x442af0>), &g_w3_make);

// ---- Inner::Delete: a deleting destructor called on a part 8 bytes into the object -------------------------------------
static volatile long g_w3_inner;
template <uint32_t DESTROY> static uint8_t *__thiscall inner_delete(uint8_t *self, uint8_t flags) {
    g_w3_inner++;
    ((void(__fastcall *)(void *))DESTROY)(self);
    if (flags & 1) memory_free(self - 8);
    return self - 8;
}
template <uint32_t AT, uint32_t TO> static const uint8_t call_first_original[] = {
    0x56, 0x8b, 0xf1, 0xe8, (uint8_t)(TO - (AT + 8)), (uint8_t)((TO - (AT + 8)) >> 8),
    (uint8_t)((TO - (AT + 8)) >> 16), (uint8_t)((TO - (AT + 8)) >> 24)};
REWRITE("Inner::DeleteDFF0C0", 0xdff0c0, (call_first_original<0xdff0c0, 0xdff0f0>), (&inner_delete<0xdff0f0>), &g_w3_inner);
REWRITE("Inner::DeleteDFF0C0", 0xdffa50, (call_first_original<0xdffa50, 0xdffa80>), (&inner_delete<0xdffa80>), &g_w3_inner);

// ---- Owned::Clear: deletes the object at +OBJECT, if any, and zeroes it and the field at +OTHER --------------------------
static volatile long g_w3_owned;
template <uint32_t OBJECT, uint32_t OTHER> static void __fastcall owned_clear(uint8_t *self) {
    g_w3_owned++;
    void *object = *(void **)(self + OBJECT);
    if (object) ((void *(__thiscall *)(void *, uint32_t))(*(void ***)object)[0])(object, 1);
    *(void **)(self + OBJECT) = nullptr;
    *(uint32_t *)(self + OTHER) = 0;
}
static const uint8_t owned_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x8e};
REWRITE("Owned::Clear713D40", 0x713d40, owned_original, (&owned_clear<0x224, 0x234>), &g_w3_owned);
REWRITE("Owned::Clear713D40", 0x8245d0, owned_original, (&owned_clear<0x140, 0x13c>), &g_w3_owned);

// ---- Part::Offset: asks the part at +PART, with a position and the position moved by ADD ---------------------------------
static volatile long g_w3_offset;
template <uint32_t PART, uint32_t ADD, uint32_t CALL> static uint32_t __thiscall part_offset(uint8_t *self, uint32_t at, uint32_t a, uint32_t b) {
    g_w3_offset++;
    return ((uint32_t(__thiscall *)(void *, uint32_t *, uint32_t, uint32_t, uint32_t))CALL)(*(void **)(self + PART), &at, at + ADD, a, b);
}
static const uint8_t offset_original[] = {0x8b, 0x44, 0x24, 0x0c, 0x8b, 0x54, 0x24, 0x08};
REWRITE("Part::Offset7B6760", 0x7b6760, offset_original, (&part_offset<0x4c, 0x18c, 0x7b5420>), &g_w3_offset);
REWRITE("Part::Offset7B6760", 0xa0dea0, offset_original, (&part_offset<0x30, 0xe8, 0xa0d750>), &g_w3_offset);

// ---- State::Reset: resets the state, clears some of its flags, and marks it as having no target --------------------------
static volatile long g_w3_state;
template <uint32_t RESET, uint32_t KEEP> static void __fastcall state_reset(uint32_t *self) {
    g_w3_state++;
    ((void(__fastcall *)(void *))RESET)(self);
    self[0x10] &= KEEP;
    self[0x1d] = 0xffffffff;
    self[0x11] = 0;
}
REWRITE("State::Reset548CD0", 0x548cd0, (call_first_original<0x548cd0, 0x548c40>), (&state_reset<0x548c40, 0xfffffffc>), &g_w3_state);
REWRITE("State::Reset548CD0", 0x5490c0, (call_first_original<0x5490c0, 0x549000>), (&state_reset<0x549000, 0xfffffffe>), &g_w3_state);

// ---- Names::Index: the index, from 1, of a name in a global table, 0 when it isn't there ----------------------------------
static volatile long g_w3_name_index;
template <uint32_t TABLE, uint32_t COUNT> static uint32_t __fastcall names_index(const char *name) {
    g_w3_name_index++;
    for (uint32_t i = 1; i < COUNT; i++)
        if (!((int(__cdecl *)(const char *, const char *))0xde22df)(name, ((const char **)TABLE)[i])) return i;
    return 0;
}
static const uint8_t name_index_original[] = {0x56, 0x57, 0x8b, 0xf9, 0xbe, 0x01, 0x00, 0x00};
REWRITE("Names::IndexC47120", 0xc47120, name_index_original, (&names_index<0x10f306c, 9>), &g_w3_name_index);
REWRITE("Names::IndexC47120", 0xc47160, name_index_original, (&names_index<0x10f3090, 6>), &g_w3_name_index);

// ---- Value::Map: maps the values 1.. through a fixed table; anything else stays as it is --------------------------------
static volatile long g_w3_map;
template <const uint8_t *MAP, uint32_t COUNT> static uint32_t __fastcall value_map(uint32_t value) {
    g_w3_map++;
    return value - 1 < COUNT ? MAP[value - 1] : value;
}
static const uint8_t map_rotate[] = {4, 5, 6, 1, 2, 3}, map_pairs[] = {2, 1, 4, 3, 6, 5}, map_reverse[] = {7, 6, 5, 4, 3, 2, 1};
static const uint8_t map_original[] = {0x8b, 0xc1, 0x8d, 0x50, 0xff, 0x83, 0xfa};
REWRITE("Value::MapBC41B0", 0xbc41b0, map_original, (&value_map<map_rotate, 6>), &g_w3_map);
REWRITE("Value::MapBC41B0", 0xbc4b50, map_original, (&value_map<map_pairs, 6>), &g_w3_map);
REWRITE("Value::MapBC41B0", 0xbc4c00, map_original, (&value_map<map_reverse, 7>), &g_w3_map);
