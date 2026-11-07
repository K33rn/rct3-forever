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

// ---- Refs::Sum: the total of a field over the interfaces of the references at +0xcc (count at +0xd0) ------------------
static volatile long g_w3_refs_sum;
template <uint32_t FIELD> static int32_t __fastcall refs_sum(uint8_t *self) {
    g_w3_refs_sum++;
    int32_t total = 0;
    for (int32_t i = 0; i < *(int32_t *)(self + 0xd0); i++) {
        uint8_t *part = (uint8_t *)ref_interface<0x13cde74>(*(uint8_t **)(self + 0xcc) + i * 0x1c);
        if (part) total += *(int32_t *)(part + FIELD);
    }
    return total;
}
static const uint8_t refs_sum_original[] = {0x83, 0xec, 0x08, 0x53, 0x55, 0x8b, 0xe9, 0x8b};
REWRITE("Refs::SumA895B0", 0xa895b0, refs_sum_original, (&refs_sum<0x74>), &g_w3_refs_sum);
REWRITE("Refs::SumA895B0", 0xa89af0, refs_sum_original, (&refs_sum<0x78>), &g_w3_refs_sum);

// ---- Hash::Find: the value of a key: its bucket holds a run of items and of values at the same indices ---------------------
static volatile long g_w3_hash;
static uint32_t *__thiscall hash_find(uint8_t *self, uint32_t key) {
    g_w3_hash++;
    uint32_t hash = ((uint32_t(__thiscall *)(void *, uint32_t))0xbdbf70)(self, key);
    uint32_t bucket = hash % *(uint32_t *)(self + 4);
    uint32_t *starts = *(uint32_t **)(self + 0xc);
    uint32_t end = starts[bucket + 1];
    for (uint32_t i = starts[bucket]; i < end; i++)
        if (((uint8_t(__thiscall *)(void *, uint32_t, uint32_t *))0xbdbfd0)(self + 1, key, *(uint32_t **)(self + 0x10) + i))
            return *(uint32_t **)(self + 0x14) + i;
    return nullptr;
}
static const uint8_t hash_original[] = {0x53, 0x8b, 0x5c, 0x24, 0x08, 0x55, 0x56, 0x57};
REWRITE("Hash::FindBDD210", 0xbdd210, hash_original, hash_find, &g_w3_hash);
REWRITE("Hash::FindBDD210", 0xbdd290, hash_original, hash_find, &g_w3_hash);

// ---- Area::Around: the area at +0x40 grown by MARGIN on each side, kept within the map ---------------------------------
static volatile long g_w3_area;
template <int32_t MARGIN> static void __thiscall area_around(uint8_t *self, int32_t *x0, int32_t *y0, int32_t *x1, int32_t *y1) {
    g_w3_area++;
    uint8_t *map = ((uint8_t * (__cdecl *)())0x40a090)();
    if (!(map[0x2c] & 1)) ((void(__fastcall *)(void *))0x800860)(map);
    uint8_t *size = *(uint8_t **)(map + 0x54);
    int32_t *area = (int32_t *)(self + 0x40);
    int32_t low_x = area[0] - MARGIN, low_y = area[2] - MARGIN, high_x = area[1] + MARGIN, high_y = area[3] + MARGIN;
    int32_t last_x = size[4] - 1, last_y = size[5] - 1;
    *x0 = low_x > 0 ? low_x : 0;
    *y0 = low_y > 0 ? low_y : 0;
    *x1 = last_x > high_x ? high_x : last_x;
    *y1 = last_y > high_y ? high_y : last_y;
}
static const uint8_t area_original[] = {0x83, 0xec, 0x08, 0x56, 0x57, 0x8b, 0xf1};
REWRITE("Area::Around8DCAA0", 0x8dcaa0, area_original, (&area_around<2>), &g_w3_area);
REWRITE("Area::Around8DCAA0", 0x8dcb70, area_original, (&area_around<3>), &g_w3_area);

// ---- Refs::FirstFree: the index of the first item whose target has nothing at +0xc, or -1 --------------------------
static volatile long g_w3b_first_free;
template <uint32_t COUNT, uint32_t ITEMS, uint32_t STRIDE> static int32_t __fastcall w3b_first_free(uint8_t *self) {
    g_w3b_first_free++;
    int32_t count = *(int32_t *)(self + COUNT);
    uint8_t *item = *(uint8_t **)(self + ITEMS);
    for (int32_t i = 0; i < count; i++, item += STRIDE)
        if (!*(uint32_t *)(*(uint8_t **)(item + 4) + 0xc)) return i;
    return -1;
}
static const uint8_t w3b_first_free_a[] = {0x8b, 0x91, 0x84, 0x00, 0x00}, w3b_first_free_b[] = {0x8b, 0x91, 0x5c, 0x01, 0x00};
REWRITE("Refs::FirstFreeA97380", 0xa97380, w3b_first_free_a, (&w3b_first_free<0x84, 0x80, 0x1c>), &g_w3b_first_free);
REWRITE("Refs::FirstFreeA97380", 0xaaea80, w3b_first_free_b, (&w3b_first_free<0x15c, 0x158, 0x18>), &g_w3b_first_free);

// ---- Items::EachCall: one function on every item of a list; the function may shorten the list -----------------------
static volatile long g_w3b_each;
template <uint32_t COUNT, uint32_t ITEMS, uintptr_t CALL> static void __fastcall w3b_each(uint8_t *self) {
    g_w3b_each++;
    for (int32_t i = 0; i < *(int32_t *)(self + COUNT); i++)
        ((void(__fastcall *)(void *))CALL)((*(void ***)(self + ITEMS))[i]);
}
static const uint8_t w3b_each_original[] = {0x56, 0x57, 0x8b, 0xf9, 0x8b, 0x47};
REWRITE("Items::EachCall83FCA0", 0x83fca0, w3b_each_original, (&w3b_each<0x58, 0x54, 0x829b10>), &g_w3b_each);
REWRITE("Items::EachCall83FCA0", 0xce82d0, w3b_each_original, (&w3b_each<0x28, 0x20, 0xce7df0>), &g_w3b_each);

// ---- Hook::Unhook: tells the owner this object is going, by a small {kind, object} notice, then clears the part at +4 ---
static volatile long g_w3b_unhook;
template <uint32_t KIND> static void __fastcall w3b_unhook(uint8_t *self) {
    g_w3b_unhook++;
    struct { uint32_t kind; void *object; } notice = {KIND, self};
    ((void(__thiscall *)(void *, void *))0x6ae7c0)(self, &notice);
    ((void(__fastcall *)(void *))0x6a1e20)(self + 4);
}
static const uint8_t w3b_unhook_original[] = {0x83, 0xec, 0x08, 0x56, 0x8d, 0x44, 0x24, 0x04};
REWRITE("Hook::Unhook6AFD60", 0x6afd60, w3b_unhook_original, (&w3b_unhook<0xf606e8>), &g_w3b_unhook);
REWRITE("Hook::Unhook6AFD60", 0x6afd90, w3b_unhook_original, (&w3b_unhook<0xf606f0>), &g_w3b_unhook);

// ---- Value::InSet: whether a value from 0x11 up belongs to a fixed set ------------------------------------------------
static const uint8_t w3b_set_a[] = {1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,0,1,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1};
static const uint8_t w3b_set_b[] = {1,1,0,0,0,0,0,0,1,0,1,0,0,0,0,1,1,1,0,0,0,0,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,1,1};
static volatile long g_w3b_in_set;
template <const uint8_t *SET, uint32_t COUNT> static bool __fastcall w3b_in_set(uint32_t value) {
    g_w3b_in_set++;
    return value - 0x11 < COUNT && SET[value - 0x11];
}
static const uint8_t w3b_in_set_a_original[] = {0x8d, 0x41, 0xef, 0x3d, 0xee}, w3b_in_set_b_original[] = {0x8d, 0x41, 0xef, 0x3d, 0xa4};
REWRITE("Value::InSet49B190", 0x49b190, w3b_in_set_a_original, (&w3b_in_set<w3b_set_a, 0xef>), &g_w3b_in_set);
REWRITE("Value::InSet49B190", 0x49b3c0, w3b_in_set_b_original, (&w3b_in_set<w3b_set_b, 0xa5>), &g_w3b_in_set);

// ---- Child::Drop: forgets the count before the child, destroys and frees the child, then clears the part at -0x1c ------
static volatile long g_w3b_drop;
template <uint32_t CHILD> static void __thiscall w3b_drop(uint8_t *self, uint32_t) {
    g_w3b_drop++;
    *(uint32_t *)(self + CHILD - 4) = 0;
    void *child = *(void **)(self + CHILD);
    if (child) {
        ((void(__fastcall *)(void *))0x6af760)(child);
        memory_free(child);
    }
    *(void **)(self + CHILD) = nullptr;
    ((void(__fastcall *)(void *))0x6a1e20)(self - 0x1c);
}
static const uint8_t w3b_drop_original[] = {0x56, 0x8b, 0xf1, 0x57, 0x8b, 0x7e};
REWRITE("Child::Drop63B4E0", 0x63b4e0, w3b_drop_original, (&w3b_drop<0x28>), &g_w3b_drop);
REWRITE("Child::Drop63B4E0", 0x6706d0, w3b_drop_original, (&w3b_drop<0x3c>), &g_w3b_drop);
REWRITE("Child::Drop63B4E0", 0x86e810, w3b_drop_original, (&w3b_drop<0x28>), &g_w3b_drop);

// ---- Message::Forward: builds a message from a value and sends it to the target, if there is one ----------------------
static volatile long g_w3b_forward;
template <uint32_t TARGET, uintptr_t BUILD, uintptr_t SEND> static void __thiscall w3b_forward(uint8_t *self, uint32_t value) {
    g_w3b_forward++;
    void *target = *(void **)(self + TARGET);
    if (!target) return;
    uint8_t flag = 0, message[0x64];
    ((void(__thiscall *)(void *, uint32_t, uint8_t *))BUILD)(message, value, &flag);
    ((void(__thiscall *)(void *, void *))SEND)(target, message);
}
static const uint8_t w3b_forward_original[] = {0x83, 0xec, 0x68, 0x56, 0x8b, 0x71};
REWRITE("Message::ForwardA0EB00", 0xa0eb00, w3b_forward_original, (&w3b_forward<0x30, 0xa0bfd0, 0xa0e960>), &g_w3b_forward);
REWRITE("Message::ForwardA0EB00", 0xb5f6f0, w3b_forward_original, (&w3b_forward<0x24, 0xb5b8d0, 0xb5f3b0>), &g_w3b_forward);

// ---- Message::Post: builds a small message from three values and hands it to this object ---------------------------------
static volatile long g_w3b_post;
template <uintptr_t BUILD, uintptr_t SEND> static void __thiscall w3b_post(void *self, uint32_t a, uint32_t b, uint32_t c) {
    g_w3b_post++;
    uint8_t message[0x1c];
    ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))BUILD)(message, a, b, c);
    ((void(__thiscall *)(void *, void *))SEND)(self, message);
}
static const uint8_t w3b_post_original[] = {0x8b, 0x44, 0x24, 0x0c, 0x8b, 0x54, 0x24, 0x04};
REWRITE("Message::Post7B9D30", 0x7b9d30, w3b_post_original, (&w3b_post<0x7b2f30, 0x7b94e0>), &g_w3b_post);
REWRITE("Message::Post7B9D30", 0xb5f370, w3b_post_original, (&w3b_post<0xb5b840, 0xb5ecb0>), &g_w3b_post);
REWRITE("Message::Post7B9D30", 0xb75670, w3b_post_original, (&w3b_post<0xb6bc70, 0xb74fb0>), &g_w3b_post);

// ---- Actor::Notify: passes a fixed note to the actor, and when the value is this object's own, signals event 10 -----------
static volatile long g_w3b_notify;
template <uint32_t ACTOR, uint32_t OWN, uint32_t LIST, uintptr_t SIGNAL> static void __thiscall w3b_notify(uint8_t *self, uint32_t value) {
    g_w3b_notify++;
    ((void(__thiscall *)(void *, uint32_t))0x4d7e70)(*(void **)(self + ACTOR), 0xf264ad);
    if (value == *(uint32_t *)(self + OWN))
        ((void(__thiscall *)(void *, uint32_t, void *))SIGNAL)(*(void **)(self + LIST), 10, self - 0xd8);
}
static const uint8_t w3b_notify_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x4e};
REWRITE("Actor::Notify710020", 0x710020, w3b_notify_original, (&w3b_notify<0x44, 0x170, 0x20, 0x717840>), &g_w3b_notify);
REWRITE("Actor::Notify710020", 0xaa6b10, w3b_notify_original, (&w3b_notify<0x64, 0x194, 0x30, 0xaa5400>), &g_w3b_notify);

// ---- Registry::Forward: passes the whole object (this is a part OFFSET into it) to the registry's handler -------------------
static volatile long g_w3b_registry;
template <uint32_t OFFSET> static uint32_t __thiscall w3b_registry(uint8_t *self, uint32_t, uint32_t) {
    g_w3b_registry++;
    uint8_t *registry = ((uint8_t * (__cdecl *)())0x42df90)();
    return ((uint32_t(__thiscall *)(void *, void *, uint32_t))0x5f3df0)(*(void **)(registry + 0x30), self - OFFSET, 1);
}
static const uint8_t w3b_registry_a[] = {0x56, 0x8b, 0xf1, 0xe8, 0x78}, w3b_registry_b[] = {0x56, 0x8b, 0xf1, 0xe8, 0x48};
REWRITE("Registry::Forward4A0E10", 0x4a0e10, w3b_registry_a, (&w3b_registry<0xb0>), &g_w3b_registry);
REWRITE("Registry::Forward4A0E10", 0x87ef40, w3b_registry_b, (&w3b_registry<0xb4>), &g_w3b_registry);

// ---- Clock::StreamTime: the source clock's time less the start time; fails without a source ---------------------------------
static volatile long g_w3b_stream_time;
template <uint32_t SOURCE, uint32_t START> static int32_t __thiscall w3b_stream_time(uint8_t *self, int64_t *time) {
    g_w3b_stream_time++;
    void **source = *(void ***)(self + SOURCE);
    if (!source) return (int32_t)0x80040213;
    int32_t result = ((int32_t(__stdcall *)(void *, int64_t *))(*(void ***)source)[3])(source, time);
    if (result < 0) return result;
    *time -= *(int64_t *)(self + START);
    return 0;
}
static const uint8_t w3b_stream_time_original[] = {0x56, 0x57, 0x8b, 0xf9, 0x8b, 0x47};
REWRITE("Clock::StreamTimeDF9030", 0xdf9030, w3b_stream_time_original, (&w3b_stream_time<0x14, 0x18>), &g_w3b_stream_time);
REWRITE("Clock::StreamTimeDF9030", 0xdf9650, w3b_stream_time_original, (&w3b_stream_time<0x18, 0x20>), &g_w3b_stream_time);

// ---- Text::Print: the number in the first byte of the value at +4, as text, cut to fit the buffer ----------------------------
static volatile long g_w3b_print;
static void __thiscall w3b_print(uint8_t *self, char *out, uint32_t size) {
    g_w3b_print++;
    char text[0x64];
    ((int(__cdecl *)(char *, const char *, ...))0xddcdb5)(text, (const char *)0xf28e5c, (uint32_t)**(uint8_t **)(self + 4));
    ((char *(__cdecl *)(char *, const char *, uint32_t))0xddef30)(out, text, size);
    out[size - 1] = 0;
}
static const uint8_t w3b_print_original[] = {0x83, 0xec, 0x68, 0xa1, 0x34, 0xe6, 0x0f, 0x01};
REWRITE("Text::PrintCA71D0", 0xca71d0, w3b_print_original, w3b_print, &g_w3b_print);
REWRITE("Text::PrintCA71D0", 0xca7580, w3b_print_original, w3b_print, &g_w3b_print);

// ---- Child::DropBoth: destroys and frees the child, deletes the owned object, then clears the part at the start ------
static volatile long g_w3b_drop_both;
template <uint32_t CHILD, uint32_t OWNED> static void __thiscall w3b_drop_both(uint8_t *self, uint32_t) {
    g_w3b_drop_both++;
    void *child = *(void **)(self + CHILD);
    if (child) {
        ((void(__fastcall *)(void *))0x6af760)(child);
        memory_free(child);
    }
    void *owned = *(void **)(self + OWNED);
    *(void **)(self + CHILD) = nullptr;
    if (owned) ((void(__thiscall *)(void *, uint32_t))(*(void ***)owned)[0])(owned, 1);
    *(void **)(self + OWNED) = nullptr;
    ((void(__fastcall *)(void *))0x6a1e20)(self);
}
static const uint8_t w3b_drop_both_original[] = {0x56, 0x8b, 0xf1, 0x57, 0x8b, 0x7e};
REWRITE("Child::DropBoth63A180", 0x63a180, w3b_drop_both_original, (&w3b_drop_both<0x3c, 0x40>), &g_w3b_drop_both);
REWRITE("Child::DropBoth63A180", 0x670670, w3b_drop_both_original, (&w3b_drop_both<0x58, 0x54>), &g_w3b_drop_both);

// ---- Task::Release: gives back the handle at +0x1c, tells the worker, and releases the COM object if there is one ---------
static volatile long g_w3b_release;
template <uint32_t WORKER, uint32_t SLOT, uint32_t COM> static void __fastcall w3b_release(uint8_t *self) {
    g_w3b_release++;
    void *pool = *(void **)(*(uint8_t **)(self + 0x14) + 0x470);
    ((void(__thiscall *)(void *, uint32_t))0xc5cb30)(pool, *(uint32_t *)(self + 0x1c));
    *(uint32_t *)(self + 0x1c) = 0;
    void *worker = *(void **)(self + WORKER);
    ((void(__fastcall *)(void *))(*(void ***)worker)[SLOT / 4])(worker);
    void *com = *(void **)(self + COM);
    if (com) {
        ((uint32_t(__stdcall *)(void *))(*(void ***)com)[2])(com);
        *(void **)(self + COM) = nullptr;
    }
}
static const uint8_t w3b_release_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x1c, 0x8b, 0x4e, 0x14};
REWRITE("Task::ReleaseBFC3C0", 0xbfc3c0, w3b_release_original, (&w3b_release<0x38, 0x10, 0x3c>), &g_w3b_release);
REWRITE("Task::ReleaseBFC3C0", 0xbfc570, w3b_release_original, (&w3b_release<0x3c, 0x14, 0x40>), &g_w3b_release);

// ---- Map::Refresh: clears a pending flag and its value, brings both maps up to date, then does the rest --------------------
static volatile long g_w3b_refresh;
static inline void w3b_map_up_to_date(uint8_t *part) {
    uint8_t *map = *(uint8_t **)(part + 0xc);
    if (map && !(map[0x2c] & 1)) ((void(__fastcall *)(void *))0x800860)(map);
}
template <uint32_t BIT, uintptr_t REST> static void __fastcall w3b_refresh(uint8_t *self) {
    g_w3b_refresh++;
    uint32_t *flags = (uint32_t *)(self + 0x40);
    if (*flags >> BIT & 1) {
        *(uint32_t *)(self + 0x74) = 0;
        *flags &= ~(1u << BIT);
    }
    w3b_map_up_to_date(*(uint8_t **)(self + 0x4c));
    w3b_map_up_to_date(*(uint8_t **)(self + 0x60));
    ((void(__fastcall *)(void *))REST)(self);
}
static const uint8_t w3b_refresh_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x40, 0x8b, 0xc8, 0xc1, 0xe9};
REWRITE("Map::Refresh54B740", 0x54b740, w3b_refresh_original, (&w3b_refresh<3, 0x549b10>), &g_w3b_refresh);
REWRITE("Map::Refresh54B740", 0x54bab0, w3b_refresh_original, (&w3b_refresh<2, 0x549ba0>), &g_w3b_refresh);

// ---- Job::Poll: asks whether a waiting job is finished, and moves it on to ready or done -------------------------------------
static volatile long g_w3b_poll;
template <uint32_t WAITING, uint32_t READY, uint32_t DONE> static void __thiscall w3b_poll(void *, uint32_t *job) {
    g_w3b_poll++;
    if (job[1] != WAITING) return;
    uint32_t status = 0;
    int32_t failed = (*(int32_t(__cdecl **)(uint32_t, uint32_t *))0x1106e0c)(job[3], &status);
    if (failed) {
        job[2] = 0;
        job[1] = DONE;
    } else if (status == 1) {
        job[1] = READY;
    } else if (status != 2) {
        job[3] = 0;
        job[1] = DONE;
    }
}
static const uint8_t w3b_poll_original[] = {0x51, 0x56, 0x8b, 0x74, 0x24, 0x0c, 0x83, 0x7e, 0x04};
REWRITE("Job::Poll45E270", 0x45e270, w3b_poll_original, (&w3b_poll<4, 6, 5>), &g_w3b_poll);
REWRITE("Job::Poll45E270", 0x45e320, w3b_poll_original, (&w3b_poll<7, 9, 8>), &g_w3b_poll);

// ---- Choice::Set: stores a new choice of either kind when it changed, and passes it on; kind 0 also resets and announces ---
static volatile long g_w3b_choice;
template <uint32_t MAIN, uint32_t OTHER, uintptr_t PASS> static void __thiscall w3b_choice(uint8_t *self, uint32_t kind, uint32_t value) {
    g_w3b_choice++;
    uint32_t *stored = (uint32_t *)(self + (kind ? OTHER : MAIN));
    if (value == *stored) return;
    if (!kind) ((void(__fastcall *)(void *))0x82b350)(self);
    *stored = value;
    ((void(__thiscall *)(void *, uint32_t, uint32_t))PASS)(*(void **)(self + 0x84), kind, value);
    if (!kind) ((void(__thiscall *)(void *, void *))0x87eee0)(**(void ***)(self + 0x9c), self + 0xcc);
}
static const uint8_t w3b_choice_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x4c, 0x24, 0x08};
REWRITE("Choice::Set82FA30", 0x82fa30, w3b_choice_original, (&w3b_choice<0xd0, 0xdc, 0x86d000>), &g_w3b_choice);
REWRITE("Choice::Set82FA30", 0x82fab0, w3b_choice_original, (&w3b_choice<0xd4, 0xe0, 0x86d050>), &g_w3b_choice);

// ---- Tint::Apply: builds a table of sixteen entries all of one of two stored values, and gives it to the device -----------
static volatile long g_w3b_tint;
template <uint32_t MESH, uint32_t ON, uint32_t OFF> static uint32_t __thiscall w3b_tint(uint8_t *self, uint32_t on) {
    g_w3b_tint++;
    uint32_t value = *(uint32_t *)(self + ((uint8_t)on ? ON : OFF));
    uint32_t *v = &value;
    void **device = *(void ***)0x13d0db8;
    uint8_t table[0x18];
    void *built = ((void *(__thiscall *)(void *, uint32_t *, uint32_t *, uint32_t *, uint32_t *, uint32_t *, uint32_t *, uint32_t *, uint32_t *, uint32_t *,
                                         uint32_t *, uint32_t *, uint32_t *, uint32_t *, uint32_t *, uint32_t *, uint32_t *))0x431070)(
        table, v, v, v, v, v, v, v, v, v, v, v, v, v, v, v, v);
    return ((uint32_t(__thiscall *)(void *, uint32_t, void *))(*(void ***)device)[0xb0 / 4])(device, *(uint32_t *)(self + MESH), built);
}
static const uint8_t w3b_tint_original[] = {0x8a, 0x44, 0x24, 0x04, 0x83, 0xec, 0x18, 0x53, 0x56, 0x57, 0x8b, 0x79};
REWRITE("Tint::Apply9122A0", 0x9122a0, w3b_tint_original, (&w3b_tint<0x34, 0x44, 0x3c>), &g_w3b_tint);
REWRITE("Tint::Apply9122A0", 0x912340, w3b_tint_original, (&w3b_tint<0x38, 0x48, 0x40>), &g_w3b_tint);

// ---- Slots::FirstFlagged: acts on the first of seven slots whose interface's state has a flag; false if none has ----------
static volatile long g_w3b_flagged;
template <uint32_t BIT, uint32_t HOW> static uint8_t __fastcall w3b_flagged(uint8_t *self) {
    g_w3b_flagged++;
    for (int32_t i = 0; i < 7; i++) {
        uint8_t *part = (uint8_t *)find_interface(*(uint8_t **)(self + 0x60 + i * 0x14), (const int32_t *)0x1342768);
        if (part && *(uint32_t *)(*(uint8_t **)(part + 0xa0) + 0x28) >> BIT & 1)
            return ((uint8_t(__thiscall *)(void *, int32_t, uint32_t))0x784030)(self, i, HOW);
    }
    return 0;
}
static const uint8_t w3b_flagged_original[] = {0x53, 0x55, 0x56, 0x8b, 0xe9, 0x57, 0x33, 0xff};
REWRITE("Slots::FirstFlagged7857C0", 0x7857c0, w3b_flagged_original, (&w3b_flagged<0xd, 2>), &g_w3b_flagged);
REWRITE("Slots::FirstFlagged7857C0", 0x785840, w3b_flagged_original, (&w3b_flagged<0xf, 1>), &g_w3b_flagged);

// ---- Child::Refresh: clears the child's flag, has it redo itself, then does the same work for this object --------------------
static volatile long g_w3b_child_refresh;
template <uint32_t CHILD> static void __fastcall w3b_child_refresh(uint8_t *self) {
    g_w3b_child_refresh++;
    uint8_t *child = *(uint8_t **)(self + CHILD);
    child[0x70] = 0;
    ((void(__fastcall *)(void *))(*(void ***)child)[0x80 / 4])(child);
    ((void(__fastcall *)(void *))0xc66cd0)(self);
}
static const uint8_t w3b_child_refresh_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x86};
REWRITE("Child::RefreshC69560", 0xc69560, w3b_child_refresh_original, (&w3b_child_refresh<0xec>), &g_w3b_child_refresh);
REWRITE("Child::RefreshC69560", 0xc697e0, w3b_child_refresh_original, (&w3b_child_refresh<0xe0>), &g_w3b_child_refresh);

// ---- Text::Use: makes sure a growing text ends in a zero, then passes its characters to a C function ------------------------
static volatile long g_w3b_text_use;
template <uintptr_t USE> static uint32_t __thiscall w3b_text_use(void *, uint8_t **holder) {
    g_w3b_text_use++;
    uint8_t *text = *holder;
    uint32_t size = *(uint32_t *)(text + 4);
    if (*(uint32_t *)(text + 8) <= size || (*(uint8_t **)(text + 0xc))[size]) {
        ((void(__thiscall *)(void *, uint32_t))0xbd02b0)(holder, size);
        text = *holder;
        (*(uint8_t **)(text + 0xc))[*(uint32_t *)(text + 4)] = 0;
    }
    return ((uint32_t(__cdecl *)(void *))USE)(*(void **)((*holder) + 0xc));
}
static const uint8_t w3b_text_use_original[] = {0x56, 0x8b, 0x74, 0x24, 0x08, 0x8b, 0x06};
REWRITE("Text::UseC57A80", 0xc57a80, w3b_text_use_original, (&w3b_text_use<0xde0dfd>), &g_w3b_text_use);
REWRITE("Text::UseC57A80", 0xc57ac0, w3b_text_use_original, (&w3b_text_use<0xe04588>), &g_w3b_text_use);

// ---- Try::Both: tries one of two ways and, if it gives nothing, the other; kind says which goes first ---------------------
static volatile long g_w3b_try_both;
template <uintptr_t TRY> static uint32_t __fastcall w3b_try_both(uint32_t kind, uint32_t first, uint32_t second, uint32_t a, uint32_t b) {
    g_w3b_try_both++;
    auto attempt = (uint32_t(__fastcall *)(uint32_t, uint32_t, uint32_t, uint32_t))TRY;
    if (kind > 1) return 0;
    uint32_t found = kind ? attempt(1, (uint8_t)second, a, b) : attempt(0, (uint8_t)first, a, b);
    if (found) return found;
    return kind ? attempt(0, (uint8_t)first, a, b) : attempt(1, (uint8_t)second, a, b);
}
static const uint8_t w3b_try_both_original[] = {0x83, 0xe9, 0x00, 0x53, 0x56, 0x57, 0x8a, 0xda};
REWRITE("Try::Both4CC090", 0x4cc090, w3b_try_both_original, (&w3b_try_both<0x4cbaa0>), &g_w3b_try_both);
REWRITE("Try::Both4CC090", 0x78a210, w3b_try_both_original, (&w3b_try_both<0x7895a0>), &g_w3b_try_both);

// ---- Object::Attach: registers an object, then points the owner at its interface (or at nothing) ----------------------------
static volatile long g_w3b_attach;
template <uintptr_t ID, uint32_t BACK, uintptr_t SET> static uint32_t __thiscall w3b_attach(uint8_t *self, void *object) {
    g_w3b_attach++;
    ((void(__thiscall *)(void *, void *))0x501f80)(self, object);
    return ((uint32_t(__thiscall *)(void *, void *, uint32_t))SET)(self - BACK, w3_interface_of(object, (const int32_t *)ID), 0);
}
static const uint8_t w3b_attach_original[] = {0x56, 0x8b, 0x74, 0x24, 0x08, 0x57, 0x56, 0x8b, 0xf9};
REWRITE("Object::Attach8BA370", 0x8ba370, w3b_attach_original, (&w3b_attach<0x13112f0, 0xc, 0x8b9fe0>), &g_w3b_attach);
REWRITE("Object::Attach8BA370", 0xb74130, w3b_attach_original, (&w3b_attach<0x13ce968, 0x18, 0xb72120>), &g_w3b_attach);

// ---- Stored::Run: runs the worker with the six stored values, the one before them, and the two sides' kinds ------------------
static volatile long g_w3b_run;
template <uint32_t VALUES, uintptr_t RUN> static uint32_t __fastcall w3b_run(uint8_t *self) {
    g_w3b_run++;
    uint32_t *v = (uint32_t *)(self + VALUES);
    uint32_t left = (*(uint8_t **)(self + 0x124))[0x13c], right = (*(uint8_t **)(self + 0x128))[0x13c];
    return ((uint32_t(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))RUN)(
        *(void **)(self + 0x12c), v[0], v[1], v[2], v[3], v[4], v[5], v[-1], left, right);
}
static const uint8_t w3b_run_original[] = {0x8b, 0x81, 0x28, 0x01, 0x00, 0x00, 0x33, 0xd2};
REWRITE("Stored::Run4B76F0", 0x4b76f0, w3b_run_original, (&w3b_run<0x140, 0x4ca6e0>), &g_w3b_run);
REWRITE("Stored::Run4B76F0", 0x7705f0, w3b_run_original, (&w3b_run<0x138, 0x7888d0>), &g_w3b_run);

// ---- Turn::Finish: marks the turn over and, if one was pending, ends it and queues a message; then sets the state -------------
static volatile long g_w3b_finish;
template <uint32_t PENDING, uint32_t KIND, uint32_t STATE> static void __thiscall w3b_finish(uint8_t *self, uint32_t) {
    g_w3b_finish++;
    self[0xe1] = 1;
    if (self[PENDING]) {
        ((void(__fastcall *)(void *))0xb1fb50)(self);
        self[PENDING] = 0;
        void *message = ((void *(__fastcall *)(uint32_t, uint32_t))0xb7e830)(KIND, 0);
        ((void(__thiscall *)(void *, uint32_t))(*(void ***)message)[0x50 / 4])(message, 1);
        void *queue = ((void *(__fastcall *)(void *))0xb7a5a0)(self);
        ((void(__thiscall *)(void *, void *))0xb47380)(queue, message);
    }
    *(uint32_t *)(self + 0x78) = STATE;
}
static const uint8_t w3b_finish_original[] = {0x56, 0x8b, 0xf1, 0x8a, 0x86};
REWRITE("Turn::FinishB1FD30", 0xb1fd30, w3b_finish_original, (&w3b_finish<0xec, 4, 0xd>), &g_w3b_finish);
REWRITE("Turn::FinishB1FD30", 0xb1fe20, w3b_finish_original, (&w3b_finish<0xee, 8, 0xf>), &g_w3b_finish);

// ---- Items::DrawAll: draws every item with its model and fade (a fade of 1.0 when it has none), in a device state for debugging ---
static volatile long g_w3b_draw_all;
static inline void w3b_debug_state(uint32_t value) {
    uint8_t *settings = *(uint8_t **)(*(uint8_t **)0x12f018c + 0x24);
    if (!settings[2]) return;
    void **device = *(void ***)0x13d0db8;
    ((void(__thiscall *)(void *, uint32_t, uint32_t))(*(void ***)device)[0xa4 / 4])(device, 5, value);
}
template <uint32_t TARGET> static void __thiscall w3b_draw_all(uint8_t *self, uint8_t **items, int32_t count) {
    g_w3b_draw_all++;
    w3b_debug_state(2);
    for (int32_t i = 0; i < count; i++) {
        uint8_t *item = items[i];
        void *models = ((void *(__cdecl *)())0xcec500)();
        void *model = ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))0xcec090)(
            models, *(uint32_t *)(item + 0x60), *(uint32_t *)(item + 0x64), 0, 0x10f2fa0);
        uint32_t fades = item[0x89] ? 1 : 0;
        uint32_t fade = fades ? *(uint32_t *)(item + 0x8c) : 0x3f800000;
        ((void(__thiscall *)(void *, void *, uint32_t, uint32_t, uint32_t, uint32_t))0xcf0c00)(
            model, item + 0x20, *(uint32_t *)(item + 0x68), *(uint32_t *)(self + TARGET), fades, fade);
    }
    w3b_debug_state(3);
}
static const uint8_t w3b_draw_all_original[] = {0xa1, 0x8c, 0x01, 0x2f, 0x01, 0x53, 0x8b, 0xd9};
REWRITE("Items::DrawAll974900", 0x974900, w3b_draw_all_original, (&w3b_draw_all<0x54>), &g_w3b_draw_all);
REWRITE("Items::DrawAll974900", 0x9797c0, w3b_draw_all_original, (&w3b_draw_all<0x18>), &g_w3b_draw_all);

// ---- Parts::Tell: does the base work, then tells the interface of each of the two parts, if the part has an object --------
static volatile long g_w3b_tell;
static inline void w3b_tell_part(uint8_t *part, uint32_t value) {
    if (!*(void **)(part + 0xc)) return;
    ((void(__thiscall *)(void *, uint32_t))0x9d4f70)(find_interface(part, (const int32_t *)0x13417cc), value);
}
template <uintptr_t BASE> static void __thiscall w3b_tell(uint8_t *self, uint32_t value) {
    g_w3b_tell++;
    ((void(__thiscall *)(void *, uint32_t))BASE)(self, value);
    w3b_tell_part(*(uint8_t **)(self + 0x4c), value);
    w3b_tell_part(*(uint8_t **)(self + 0x60), value);
}
static const uint8_t w3b_tell_original[] = {0x53, 0x8b, 0x5c, 0x24, 0x08, 0x56, 0x57, 0x53, 0x8b, 0xf9};
REWRITE("Parts::Tell548EA0", 0x548ea0, w3b_tell_original, (&w3b_tell<0x548cf0>), &g_w3b_tell);
REWRITE("Parts::Tell548EA0", 0x549f20, w3b_tell_original, (&w3b_tell<0x549c50>), &g_w3b_tell);

// ---- Span::Start: where a span goes so that it centres on a stored position, kept on the map --------------------------------
static volatile long g_w3b_span;
template <uint32_t PLACED, uint32_t LOOSE, uint32_t LENGTH, uint32_t TURNED, uint32_t LIMIT> static int32_t __fastcall w3b_span(uint8_t *self) {
    g_w3b_span++;
    uint8_t *shape = *(uint8_t **)(self + 0x16c);
    if (!shape) return self[LOOSE];
    uint32_t kind = *(uint32_t *)(shape + 8) & 0xffff;
    int32_t centre = kind == 1 || kind == 2 || kind == 6 || kind == 8 ? self[PLACED] : self[LOOSE];
    uint8_t *map = ((uint8_t * (__cdecl *)())0x40a090)();
    if (!(map[0x2c] & 1)) ((void(__fastcall *)(void *))0x800860)(map);
    shape = *(uint8_t **)(self + 0x16c);
    int32_t turn = *(int32_t *)(self + 0x17c);
    int32_t length = *(int32_t *)(shape + (turn == 1 || turn == 3 ? TURNED : LENGTH));
    int32_t start = centre - (length >> 1);
    if (start < 0) start = 0;
    int32_t limit = (*(uint8_t **)(map + 0x54))[LIMIT];
    return start + length > limit ? limit - length : start;
}
static const uint8_t w3b_span_original[] = {0x53, 0x56, 0x8b, 0xf1, 0x8b, 0x8e, 0x6c, 0x01};
REWRITE("Span::Start838AC0", 0x838ac0, w3b_span_original, (&w3b_span<0x208, 0x185, 0x10, 0x14, 4>), &g_w3b_span);
REWRITE("Span::Start838AC0", 0x838b60, w3b_span_original, (&w3b_span<0x209, 0x186, 0x14, 0x10, 5>), &g_w3b_span);

// ---- Rows::Count: how many of the rows of the (remembered) interface pass a check ---------------------------------------------
static volatile long g_w3b_rows;
static inline uint8_t *w3b_rows_of(uint8_t *self) {
    if (!self[0x21c]) {
        *(void **)(self + 0x218) = find_interface(*(uint8_t **)(self + 0x208), (const int32_t *)0x12f5338);
        self[0x21c] = 1;
    }
    return *(uint8_t **)(self + 0x218);
}
template <uintptr_t CHECK> static int32_t __fastcall w3b_rows(uint8_t *self) {
    g_w3b_rows++;
    int32_t passed = 0;
    for (int32_t i = 0; i < *(int32_t *)(w3b_rows_of(self) + 0x44); i++)
        if (((uint8_t(__fastcall *)(void *))CHECK)(*(uint8_t **)(w3b_rows_of(self) + 0x40) + i * 0xe0)) passed++;
    return passed;
}
static const uint8_t w3b_rows_original[] = {0x51, 0x53, 0x55, 0x56, 0x33, 0xed, 0x57, 0x8b, 0xf9};
REWRITE("Rows::Count533FF0", 0x533ff0, w3b_rows_original, (&w3b_rows<0x559bc0>), &g_w3b_rows);
REWRITE("Rows::Count533FF0", 0x5340e0, w3b_rows_original, (&w3b_rows<0x559bf0>), &g_w3b_rows);

// ---- Third pass. An original's first eight bytes as one little-endian number: the start of an exception frame holds a handler
// address that differs per copy, so every copy names its own.
template <uint64_t V> static const uint8_t w3c_start[8] = {(uint8_t)V, (uint8_t)(V >> 8), (uint8_t)(V >> 16), (uint8_t)(V >> 24),
                                                           (uint8_t)(V >> 32), (uint8_t)(V >> 40), (uint8_t)(V >> 48), (uint8_t)(V >> 56)};
template <uintptr_t FN> static inline void w3c_run(void *self) { ((void(__fastcall *)(void *))FN)(self); }
// Deletes an object through its virtual destructor (the slot it sits in differs), if there is one.
template <int SLOT = 0> static inline void w3c_delete(void *object) {
    if (object) ((void(__thiscall *)(void *, uint32_t))(*(void ***)object)[SLOT])(object, 1);
}
static inline void w3c_destroy_texts(void *first, uint32_t count) {
    ((void(__stdcall *)(void *, uint32_t, uint32_t, void *))0xddcf39)(first, 0x14, count, (void *)0x40f730);
}

// ---- Owner::Dtor: frees the block it owns and clears the pointer, then the base destructor ---------------------------------
static volatile long g_w3c_owner;
template <uint32_t BLOCK, uintptr_t BASE> static void __fastcall w3c_owner(uint8_t *self) {
    g_w3c_owner++;
    void *&block = *(void **)(self + BLOCK);
    if (block) {
        memory_free_aligned(block);
        block = nullptr;
    }
    w3c_run<BASE>(self);
}
REWRITE("Owner::Dtor5F0520", 0x5f0520, w3c_start<0x6400e2680868ff6a>, (&w3c_owner<0x90, 0x5ef9b0>), &g_w3c_owner);
REWRITE("Owner::Dtor5F0520", 0x5fcb90, w3c_start<0x6400e27e8868ff6a>, (&w3c_owner<0x148, 0x67b9f0>), &g_w3c_owner);
REWRITE("Owner::Dtor5F0520", 0x69fe90, w3c_start<0x6400e3cb4868ff6a>, (&w3c_owner<0x578, 0x69f600>), &g_w3c_owner);
REWRITE("Owner::Dtor5F0520", 0x6f2cd0, w3c_start<0x6400e477c868ff6a>, (&w3c_owner<0x578, 0x69f600>), &g_w3c_owner);
REWRITE("Owner::Dtor5F0520", 0xb7e190, w3c_start<0x6400e9806868ff6a>, (&w3c_owner<0x98, 0xb7b9d0>), &g_w3c_owner);

// ---- The same with the block near the start of the object ------------------------------------------------------------------
static volatile long g_w3c_owner8;
template <uint32_t BLOCK, uintptr_t BASE> static void __fastcall w3c_owner8(uint8_t *self) {
    g_w3c_owner8++;
    void *&block = *(void **)(self + BLOCK);
    if (block) {
        memory_free_aligned(block);
        block = nullptr;
    }
    w3c_run<BASE>(self);
}
REWRITE("Owner::Dtor65B590", 0x65b590, w3c_start<0x6400e3585868ff6a>, (&w3c_owner8<0x24, 0x6a1df0>), &g_w3c_owner8);
REWRITE("Owner::Dtor65B590", 0x6e3a80, w3c_start<0x6400e4522868ff6a>, (&w3c_owner8<0x7c, 0x6d6320>), &g_w3c_owner8);
REWRITE("Owner::Dtor65B590", 0xa00a90, w3c_start<0x6400e784c868ff6a>, (&w3c_owner8<0x68, 0x4882d0>), &g_w3c_owner8);
REWRITE("Owner::Dtor65B590", 0xa06290, w3c_start<0x6400e78c7868ff6a>, (&w3c_owner8<0x5c, 0x4882d0>), &g_w3c_owner8);

// ---- Slots::Dtor: deletes every object of a fixed row of slots, then the base -----------------------------------------------
static volatile long g_w3c_slots_dtor;
template <uint32_t TABLE, uint32_t TABLE2, uint32_t ROW, uint32_t COUNT> static void __fastcall w3c_slots_dtor(uint8_t *self) {
    g_w3c_slots_dtor++;
    *(uint32_t *)self = TABLE;
    *(uint32_t *)(self + 4) = TABLE2;
    for (uint32_t i = 0; i < COUNT; i++) w3c_delete(((void **)(self + ROW))[i]);
    w3c_run<0x6b0ea0>(self);
}
REWRITE("Slots::Dtor666E70", 0x666e70, w3c_start<0x6400e36fd868ff6a>, (&w3c_slots_dtor<0xf4e900, 0xf4e8e8, 0xe0, 0x10>), &g_w3c_slots_dtor);
REWRITE("Slots::Dtor666E70", 0x9962a0, w3c_start<0x6400e6ec7868ff6a>, (&w3c_slots_dtor<0xf803c0, 0xf803a8, 0x100, 0x8>), &g_w3c_slots_dtor);
REWRITE("Slots::Dtor666E70", 0x9c1f40, w3c_start<0x6400e739d868ff6a>, (&w3c_slots_dtor<0xf842e0, 0xf842c8, 0x130, 0xa>), &g_w3c_slots_dtor);
REWRITE("Slots::Dtor666E70", 0xa35300, w3c_start<0x6400e7d61868ff6a>, (&w3c_slots_dtor<0xf89a60, 0xf89a48, 0xd8, 0x6>), &g_w3c_slots_dtor);

// ---- Texts::Dtor: two text members, the later one first, then the base ------------------------------------------------------
static volatile long g_w3c_two_texts;
template <uint32_t LATER, uint32_t EARLIER> static void __fastcall w3c_two_texts(uint8_t *self) {
    g_w3c_two_texts++;
    w3c_run<0x40f730>(self + LATER);
    w3c_run<0x40f730>(self + EARLIER);
    w3c_run<0xb7b9d0>(self);
}
REWRITE("Texts::DtorB1B0E0", 0xb1b0e0, w3c_start<0x6400e9166668ff6a>, (&w3c_two_texts<0xb4, 0x98>), &g_w3c_two_texts);
REWRITE("Texts::DtorB1B0E0", 0xb1f470, w3c_start<0x6400e91ab668ff6a>, (&w3c_two_texts<0x114, 0xa0>), &g_w3c_two_texts);
REWRITE("Texts::DtorB1B0E0", 0xb29470, w3c_start<0x6400e9244668ff6a>, (&w3c_two_texts<0xb4, 0x98>), &g_w3c_two_texts);
REWRITE("Texts::DtorB1B0E0", 0xb7cfa0, w3c_start<0x6400e97e2668ff6a>, (&w3c_two_texts<0xb4, 0x98>), &g_w3c_two_texts);

// ---- Shared::Dtor: gives up one reference to a shared block {count, ..., +0xc data}; the last one out frees it ---------------
static volatile long g_w3c_shared;
template <uint32_t BLOCK, uint32_t TABLE> static void __fastcall w3c_shared(uint8_t *self) {
    g_w3c_shared++;
    int32_t *block = *(int32_t **)(self + BLOCK);
    if (--block[0] == 0) {
        memory_free_aligned(*(void **)(block + 3));
        memory_free(block);
    }
    *(uint32_t *)self = TABLE;
}
REWRITE("Shared::Dtor6F21F0", 0x6f21f0, w3c_start<0x6400e476f868ff6a>, (&w3c_shared<0x8, 0xf328c0>), &g_w3c_shared);
REWRITE("Shared::Dtor6F21F0", 0x6f2640, w3c_start<0x6400e4773868ff6a>, (&w3c_shared<0x4, 0xf328c0>), &g_w3c_shared);
REWRITE("Shared::Dtor6F21F0", 0x7fe8c0, w3c_start<0x6400e5b7c868ff6a>, (&w3c_shared<0x10, 0xf75170>), &g_w3c_shared);
REWRITE("Shared::Dtor6F21F0", 0xca34e0, w3c_start<0x6400ea479868ff6a>, (&w3c_shared<0x8, 0xfa5600>), &g_w3c_shared);

// ---- Holder::Dtor: deletes the one object it holds, then the base -----------------------------------------------------------
static volatile long g_w3c_holder;
template <uint32_t TABLE, uint32_t HELD, uintptr_t BASE> static void __fastcall w3c_holder(uint8_t *self) {
    g_w3c_holder++;
    *(uint32_t *)self = TABLE;
    w3c_delete(*(void **)(self + HELD));
    w3c_run<BASE>(self);
}
REWRITE("Holder::Dtor4E4110", 0x4e4110, w3c_start<0x6400e1644868ff6a>, (&w3c_holder<0xf34ea8, 0x8, 0x63d010>), &g_w3c_holder);
REWRITE("Holder::Dtor4E4110", 0x5f5000, w3c_start<0x6400e26da868ff6a>, (&w3c_holder<0xf400d8, 0x1c, 0x6a1df0>), &g_w3c_holder);
REWRITE("Holder::Dtor4E4110", 0x6165f0, w3c_start<0x6400e2afd868ff6a>, (&w3c_holder<0xf42f4c, 0x1c, 0x6a1df0>), &g_w3c_holder);
REWRITE("Holder::Dtor4E4110", 0x616750, w3c_start<0x6400e2b01868ff6a>, (&w3c_holder<0xf42f60, 0x1c, 0x6a1df0>), &g_w3c_holder);

// ---- Selected::Dtor: forgets the global selection, deletes what it holds, then the base -------------------------------------
static volatile long g_w3c_selected;
template <uint32_t TABLE, uint32_t TABLE2, uint32_t HELD> static void __fastcall w3c_selected(uint8_t *self) {
    g_w3c_selected++;
    *(uint32_t *)self = TABLE;
    *(uint32_t *)(self + 4) = TABLE2;
    *(int32_t *)0x10eeab0 = -1;
    w3c_delete(*(void **)(self + HELD));
    w3c_run<0x63a9c0>(self);
}
REWRITE("Selected::Dtor5A0A30", 0x5a0a30, w3c_start<0x6400e2274868ff6a>, (&w3c_selected<0xf3dca0, 0xf3dc88, 0xdc>), &g_w3c_selected);
REWRITE("Selected::Dtor5A0A30", 0x6685a0, w3c_start<0x6400e3721868ff6a>, (&w3c_selected<0xf4ecf8, 0xf4ece0, 0xd8>), &g_w3c_selected);
REWRITE("Selected::Dtor5A0A30", 0x668620, w3c_start<0x6400e3723868ff6a>, (&w3c_selected<0xf4ee00, 0xf4edec, 0xd8>), &g_w3c_selected);

// ---- Panel::Dtor: three tables back to the class's own, deletes the held object, the text member, then the base --------------
static volatile long g_w3c_panel;
template <uint32_t TABLE, uint32_t TABLE2, uint32_t AT3, uint32_t TABLE3, uint32_t HELD, uint32_t TEXT, uintptr_t BASE>
static void __fastcall w3c_panel(uint8_t *self) {
    g_w3c_panel++;
    *(uint32_t *)self = TABLE;
    *(uint32_t *)(self + 4) = TABLE2;
    *(uint32_t *)(self + AT3) = TABLE3;
    w3c_delete<1>(*(void **)(self + HELD));
    w3c_run<0x40f730>(self + TEXT);
    w3c_run<BASE>(self);
}
REWRITE("Panel::Dtor923E10", 0x923e10, w3c_start<0x6400e6997668ff6a>, (&w3c_panel<0xf7d488, 0xf7d470, 0x11c, 0xf7d46c, 0x150, 0x154, 0x5f7b70>), &g_w3c_panel);
REWRITE("Panel::Dtor923E10", 0xad6a10, w3c_start<0x6400e8afe668ff6a>, (&w3c_panel<0xf951d0, 0xf951b8, 0xd4, 0xf951ac, 0x110, 0xf4, 0x9b2640>), &g_w3c_panel);
REWRITE("Panel::Dtor923E10", 0xaefe80, w3c_start<0x6400e8d60668ff6a>, (&w3c_panel<0xf96508, 0xf964f4, 0xd4, 0xf964e8, 0x110, 0xf4, 0x9b2640>), &g_w3c_panel);
REWRITE("Panel::Dtor923E10", 0xaf2010, w3c_start<0x6400e8d8d668ff6a>, (&w3c_panel<0xf966c8, 0xf966b4, 0xd4, 0xf966a8, 0x110, 0xf4, 0x9b2640>), &g_w3c_panel);

// ---- Parts::Dtor: its three parts, the last first, then the base table ------------------------------------------------------
static volatile long g_w3c_parts;
static void __fastcall w3c_parts(uint8_t *self) {
    g_w3c_parts++;
    w3c_run<0x6af760>(self + 0x3c);
    w3c_run<0x6b0150>(self + 0x1c);
    w3c_run<0xcd2b50>(self + 4);
    *(uint32_t *)self = 0xf2694c;
}
REWRITE("Parts::Dtor6B20E0", 0x6b20e0, w3c_start<0x6400e3e5f668ff6a>, (&w3c_parts), &g_w3c_parts);
REWRITE("Parts::Dtor6B20E0", 0x6b2370, w3c_start<0x6400e3e6b668ff6a>, (&w3c_parts), &g_w3c_parts);
REWRITE("Parts::Dtor6B20E0", 0x6b2600, w3c_start<0x6400e3e77668ff6a>, (&w3c_parts), &g_w3c_parts);

// ---- Row::Dtor: a row of 0x40 text members, then the base -------------------------------------------------------------------
static volatile long g_w3c_text_row;
static void __fastcall w3c_text_row(uint8_t *self) {
    g_w3c_text_row++;
    w3c_destroy_texts(self + 0x128, 0x40);
    w3c_run<0x4882d0>(self);
}
REWRITE("Row::Dtor641690", 0x641690, w3c_start<0x6400e31b9868ff6a>, (&w3c_text_row), &g_w3c_text_row);
REWRITE("Row::Dtor641690", 0x6416f0, w3c_start<0x6400e31bb868ff6a>, (&w3c_text_row), &g_w3c_text_row);

// ---- Store::Dtor: empties itself, then frees its block ----------------------------------------------------------------------
static volatile long g_w3c_store;
template <uintptr_t EMPTY> static void __fastcall w3c_store(void **self) {
    g_w3c_store++;
    w3c_run<EMPTY>(self);
    if (*self) {
        memory_free_aligned(*self);
        *self = nullptr;
    }
}
REWRITE("Store::Dtor4A8FD0", 0x4a8fd0, w3c_start<0x6400e1122868ff6a>, (&w3c_store<0x52edd0>), &g_w3c_store);
REWRITE("Store::Dtor4A8FD0", 0x752380, w3c_start<0x6400e4fac868ff6a>, (&w3c_store<0x751830>), &g_w3c_store);

// ---- Named::Dtor: its text at +0x3c, its part at +8, then the base table ----------------------------------------------------
static volatile long g_w3c_named;
template <uint32_t TABLE> static void __fastcall w3c_named(uint8_t *self) {
    g_w3c_named++;
    *(uint32_t *)self = TABLE;
    w3c_run<0xbd1880>(self + 0x3c);
    w3c_run<0xbd99e0>(self + 8);
    *(uint32_t *)self = 0xf306b8;
}
REWRITE("Named::Dtor757670", 0x757670, w3c_start<0x6400e5013368ff6a>, (&w3c_named<0xf6c504>), &g_w3c_named);
REWRITE("Named::Dtor757670", 0x7fa5d0, w3c_start<0x6400e5b04368ff6a>, (&w3c_named<0xf7510c>), &g_w3c_named);

// ---- Entries::Dtor: every entry's object leaves the list by itself when deleted, last first; then the array of 20-byte entries
// goes (its element count sits just before it)
static volatile long g_w3c_entries;
static void __fastcall w3c_entries(uint8_t **self) {
    g_w3c_entries++;
    uint32_t &count = *(uint32_t *)(self + 2);
    while (count) {
        uint8_t *holder = *(uint8_t **)(self[0] + count * 0x14 - 0x10);
        w3c_delete(*(void **)(holder + 0xc));
    }
    if (uint8_t *entries = self[0]) {
        w3c_destroy_texts(entries, *(uint32_t *)(entries - 4));
        memory_free_aligned(entries - 4);
        self[0] = nullptr;
    }
}
REWRITE("Entries::Dtor7321D0", 0x7321d0, w3c_start<0x6400e4cca868ff6a>, (&w3c_entries), &g_w3c_entries);
REWRITE("Entries::Dtor7321D0", 0x73ab90, w3c_start<0x6400e4d94868ff6a>, (&w3c_entries), &g_w3c_entries);
REWRITE("Entries::Dtor7321D0", 0xac3e40, w3c_start<0x6400e88a3868ff6a>, (&w3c_entries), &g_w3c_entries);
REWRITE("Entries::Dtor7321D0", 0xba5e50, w3c_start<0x6400e9b03868ff6a>, (&w3c_entries), &g_w3c_entries);

// ---- Binding::Make: a two-word binding of a value, registered with the target before it takes its final table ---------------
static volatile long g_w3c_binding;
template <uint32_t TABLE> static void *__fastcall w3c_binding(uint32_t value, void *target) {
    g_w3c_binding++;
    uint32_t *binding = (uint32_t *)memory_alloc(8);
    if (!binding) return nullptr;
    binding[0] = 0xf75170;
    binding[1] = value;
    uint32_t *made = binding, key = value;
    ((void(__thiscall *)(void *, void *, void *))0x7fbf50)(target, &key, &made);
    binding[0] = TABLE;
    return binding;
}
REWRITE("Binding::Make7FF460", 0x7ff460, w3c_start<0x6400e5b99b68ff6a>, (&w3c_binding<0xf751e0>), &g_w3c_binding);
REWRITE("Binding::Make7FF460", 0x7ff550, w3c_start<0x6400e5b9db68ff6a>, (&w3c_binding<0xf751e8>), &g_w3c_binding);
REWRITE("Binding::Make7FF460", 0x7ff640, w3c_start<0x6400e5ba1b68ff6a>, (&w3c_binding<0xf751f0>), &g_w3c_binding);
REWRITE("Binding::Make7FF460", 0x7ff730, w3c_start<0x6400e5ba5b68ff6a>, (&w3c_binding<0xf751f8>), &g_w3c_binding);
REWRITE("Binding::Make7FF460", 0x7ff820, w3c_start<0x6400e5ba9b68ff6a>, (&w3c_binding<0xf75200>), &g_w3c_binding);

// ---- Maker::One: a new object built with a zero and the given argument ------------------------------------------------------
static volatile long g_w3c_make1;
template <uint32_t SIZE, uintptr_t CTOR> static void *__stdcall w3c_make1(uint32_t argument) {
    g_w3c_make1++;
    void *made = memory_alloc(SIZE);
    return made ? ((void *(__thiscall *)(void *, uint32_t, uint32_t))CTOR)(made, 0, argument) : nullptr;
}
REWRITE("Maker::OneCB8CC0", 0xcb8cc0, w3c_start<0x6400ea611b68ff6a>, (&w3c_make1<0x14, 0xcb81f0>), &g_w3c_make1);
REWRITE("Maker::OneCB8CC0", 0xcb8e70, w3c_start<0x6400ea615b68ff6a>, (&w3c_make1<0x20, 0xcb8420>), &g_w3c_make1);
REWRITE("Maker::OneCB8CC0", 0xcb8f80, w3c_start<0x6400ea617b68ff6a>, (&w3c_make1<0x20, 0xcb8630>), &g_w3c_make1);
REWRITE("Maker::OneCB8CC0", 0xcb9090, w3c_start<0x6400ea619b68ff6a>, (&w3c_make1<0x18, 0xcb8860>), &g_w3c_make1);

// ---- Maker::Blank: a new object built with four zeros -----------------------------------------------------------------------
static volatile long g_w3c_make0;
template <uint32_t SIZE, uintptr_t CTOR> static void *__cdecl w3c_make0() {
    g_w3c_make0++;
    void *made = memory_alloc(SIZE);
    return made ? ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))CTOR)(made, 0, 0, 0, 0) : nullptr;
}
REWRITE("Maker::Blank817E50", 0x817e50, w3c_start<0x6400e5d6fb68ff6a>, (&w3c_make0<0x78, 0x8161a0>), &g_w3c_make0);
REWRITE("Maker::Blank817E50", 0x817f20, w3c_start<0x6400e5d73b68ff6a>, (&w3c_make0<0x78, 0x816a10>), &g_w3c_make0);
REWRITE("Maker::Blank817E50", 0x817f80, w3c_start<0x6400e5d75b68ff6a>, (&w3c_make0<0x78, 0x816a10>), &g_w3c_make0);
REWRITE("Maker::Blank817E50", 0x818130, w3c_start<0x6400e5d7db68ff6a>, (&w3c_make0<0x70, 0x817760>), &g_w3c_make0);

// ---- Maker::Child: a new 0x5c-byte object built with a zero, its parent and the given argument ------------------------------
static volatile long g_w3c_child;
template <uintptr_t CTOR> static void *__thiscall w3c_child(void *parent, uint32_t argument) {
    g_w3c_child++;
    void *made = memory_alloc(0x5c);
    return made ? ((void *(__thiscall *)(void *, uint32_t, void *, uint32_t))CTOR)(made, 0, parent, argument) : nullptr;
}
REWRITE("Maker::Child6B2150", 0x6b2150, w3c_start<0x6400e3e60b68ff6a>, (&w3c_child<0x6b1f20>), &g_w3c_child);
REWRITE("Maker::Child6B2150", 0x6b23e0, w3c_start<0x6400e3e6cb68ff6a>, (&w3c_child<0x6b21b0>), &g_w3c_child);
REWRITE("Maker::Child6B2150", 0x6b2670, w3c_start<0x6400e3e78b68ff6a>, (&w3c_child<0x6b2440>), &g_w3c_child);

// ---- Single::Get: the one instance; building it makes it register itself in the global ---------------------------------------
static volatile long g_w3c_single;
template <uint32_t INSTANCE, uint32_t SIZE, uintptr_t CTOR> static void *__cdecl w3c_single() {
    g_w3c_single++;
    void *&instance = *(void **)INSTANCE;
    if (!instance)
        if (void *made = memory_alloc(SIZE)) ((void(__thiscall *)(void *, uint32_t))CTOR)(made, 0);
    return instance;
}
REWRITE("Single::Get42DF90", 0x42df90, w3c_start<0x6400e07e8b68ff6a>, (&w3c_single<0x12f8084, 0x54, 0x659770>), &g_w3c_single);
REWRITE("Single::Get42DF90", 0x47dbd0, w3c_start<0x6400e0dd0b68ff6a>, (&w3c_single<0x13063f4, 0x1c, 0x735b70>), &g_w3c_single);
REWRITE("Single::Get42DF90", 0x47dcb0, w3c_start<0x6400e0dd4b68ff6a>, (&w3c_single<0x133a25c, 0x38, 0x93c390>), &g_w3c_single);
REWRITE("Single::Get42DF90", 0x6c37d0, w3c_start<0x6400e4072b68ff6a>, (&w3c_single<0x13060fc, 0x48, 0x6ef4f0>), &g_w3c_single);

// ---- Array::Size: drops the old elements and makes `count` new ones, built by the element's constructor ---------------------
static volatile long g_w3c_sized;
template <uint32_t ELEMENT, uintptr_t CTOR> static uint32_t __thiscall w3c_sized(void **self, uint32_t count) {
    g_w3c_sized++;
    if (self[0]) {
        memory_free_aligned(self[0]);
        self[0] = nullptr;
    }
    if (count) {
        void *data = memory_alloc_aligned(count * ELEMENT);
        if (data) ((void(__stdcall *)(void *, uint32_t, uint32_t, void *))0x402b90)(data, ELEMENT, count, (void *)CTOR);
        self[0] = data;
    }
    *(uint32_t *)(self + 1) = count;
    return count;
}
REWRITE("Array::Size9D1120", 0x9d1120, w3c_start<0xff6a00000000a164>, (&w3c_sized<0x44, 0x9ce670>), &g_w3c_sized);
REWRITE("Array::Size9D1120", 0x9e1a20, w3c_start<0xff6a00000000a164>, (&w3c_sized<0x44, 0x9e1330>), &g_w3c_sized);
REWRITE("Array::Size9D1120", 0xaf9d40, w3c_start<0xff6a00000000a164>, (&w3c_sized<0x2c, 0xaf8cf0>), &g_w3c_sized);

// ======== wave 3, part d ========================================================================================================

// ---- Table::Call: checks the index against the count (the game's assert), then calls that entry's handler with the argument
// and the copy's flag ----------------------------------------------------------------------------------------------------------
static volatile long g_w3d_table;
template <uint32_t LINE, uint32_t FLAG, uintptr_t HANDLER> static uint32_t __thiscall w3d_table_call(uint8_t *self, int32_t index, uint32_t argument) {
    g_w3d_table++;
    if (index >= *(int32_t *)(self + 0x164) &&
        ((uint8_t(__fastcall *)(uint32_t, uint32_t, uint32_t, uint32_t))0x68f640)(0xf52e20, LINE, 0xf52e08, 0xf52e60) == 1)
        __builtin_debugtrap();
    void *entry = *(void **)(*(uint8_t **)(self + 0x190) + index * 8);
    return ((uint32_t(__thiscall *)(void *, uint32_t, uint32_t))HANDLER)(entry, argument, FLAG);
}
REWRITE("Table::Call68FCC0", 0x68fcc0, w3c_start<0x000164868bf18b56>, (&w3d_table_call<0x17b, 0, 0x67b4b0>), &g_w3d_table);
REWRITE("Table::Call68FCC0", 0x68fd10, w3c_start<0x000164868bf18b56>, (&w3d_table_call<0x18c, 0, 0x67b3c0>), &g_w3d_table);
REWRITE("Table::Call68FCC0", 0x68fdb0, w3c_start<0x000164868bf18b56>, (&w3d_table_call<0x1ac, 1, 0x67b800>), &g_w3d_table);

// ---- Cached::Find: reports a flag byte; the first time, looks the value up (the owner's object asks its table for the
// current slot's entry: an offset from the object) and keeps it ---------------------------------------------------------------
static volatile long g_w3d_cached;
template <uint32_t FLAG, uint32_t CACHED, uint32_t OWNER, uint32_t VALUE> static uint32_t __thiscall w3d_cached(uint8_t *self, uint8_t *flag) {
    g_w3d_cached++;
    if (flag) *flag = self[FLAG];
    if (!self[CACHED]) {
        uint8_t *object = *(uint8_t **)(*(uint8_t **)(self + OWNER) + 0xc);
        uint32_t found = 0;
        if (object) {
            uint8_t *table = ((uint8_t * (__thiscall *)(void *))(*(void ***)object)[1])(object);
            int32_t slot = *(int32_t *)0x12f5608;
            if (table && slot < *(int32_t *)(table + 0x18))
                if (uint8_t *entry = (*(uint8_t ***)(table + 0x14))[slot]) found = *(uint32_t *)(entry + 0x10) + (uint32_t)(uintptr_t)object;
        }
        *(uint32_t *)(self + VALUE) = found;
        self[CACHED] = 1;
    }
    return *(uint32_t *)(self + VALUE);
}
REWRITE("Cached::Find559FB0", 0x559fb0, w3c_start<0x8b57c0850424448b>, (&w3d_cached<0xd4, 0xb4, 0xa0, 0xb0>), &g_w3d_cached);
REWRITE("Cached::Find559FB0", 0x55a020, w3c_start<0x8b57c0850424448b>, (&w3d_cached<0xd5, 0xd0, 0xbc, 0xcc>), &g_w3d_cached);

// ---- Slots::Detach: takes an item out of the list it is in, marks it out, and gives its packed (group, slot) back to the
// owner if there is one ----------------------------------------------------------------------------------------------------------
static volatile long g_w3d_detach;
template <uint32_t INDEX, uint32_t LIST, uint32_t OWNER, uintptr_t RELEASE_SLOT, uintptr_t RELEASE_GROUP>
static void __thiscall w3d_detach(uint8_t *self, uint8_t *item) {
    g_w3d_detach++;
    int32_t index = *(int32_t *)(item + INDEX);
    if (index < 0) return;
    ((void(__thiscall *)(void *, int32_t))0xcdede0)(self + LIST, index);
    *(int32_t *)(item + INDEX) = -1;
    uint8_t *owner = *(uint8_t **)(self + OWNER);
    if (!owner) return;
    uint32_t packed = *(uint32_t *)(item + INDEX + 4), group = packed >> 16 & 0xffff;
    ((void(__thiscall *)(void *, uint32_t))RELEASE_SLOT)(*(uint8_t **)(owner + 8) + group * 0x1c, packed & 0xffff);
    ((void(__thiscall *)(void *, uint32_t))RELEASE_GROUP)(owner, group);
}
REWRITE("Slots::Detach7B4FA0", 0x7b4fa0, w3c_start<0x88838b08245c8b53>, (&w3d_detach<0x188, 0x1c, 0x4c, 0x7b2ac0, 0x7b2a40>), &g_w3d_detach);
REWRITE("Slots::Detach7B4FA0", 0xa0d600, w3c_start<0xe4838b08245c8b53>, (&w3d_detach<0xe4, 0x10, 0x30, 0xa0bec0, 0xa0be40>), &g_w3d_detach);

// ---- Piece::Place: an entry either lives here (low bit set) or in a part found from its packed fields; places it there ------
static volatile long g_w3d_place;
template <uintptr_t PLACE> static uint32_t __thiscall w3d_place(uint8_t *self, int32_t index, uint32_t a, uint32_t b) {
    g_w3d_place++;
    auto place = (uint32_t(__thiscall *)(void *, void *, uint32_t, uint32_t))PLACE;
    uint32_t flags = (*(uint32_t **)(self + 0x3b0))[index];
    if (flags & 1) return place(self, *(uint8_t **)(self + 0x3c4) + (*(uint32_t **)(self + 0x3b4))[index] * 0x150, a, b);
    void *source = (*(void ***)(self + 0x390))[flags >> 25 & 0xf];
    uint8_t *part = ((uint8_t * (__thiscall *)(void *, uint32_t, uint32_t, int32_t, void *))0x89caf0)(
        source, flags >> 12 & 0xfff, flags >> 1 & 0x7ff, (int32_t)(flags >> 29 & 7) - 1, self);
    return place(*(void **)(part + 0x10), *(uint8_t **)(part + 0x14) + (*(uint32_t **)(self + 0x3b4))[index] * 0x150, a, b);
}
REWRITE("Piece::Place89CF30", 0x89cf30, w3c_start<0x0003b0868bf18b56>, (&w3d_place<0x88d460>), &g_w3d_place);
REWRITE("Piece::Place89CF30", 0x8a0660, w3c_start<0x0003b0868bf18b56>, (&w3d_place<0x89ed50>), &g_w3d_place);

// ---- Keyed::Init: base set up with the copy's two keys, then the argument, the vtable and a cleared field ------------------
static volatile long g_w3d_init;
template <uint32_t K0, uint32_t K2, uint32_t K3, uint32_t ARG, uint32_t CLEAR, uint32_t VTABLE>
static void *__thiscall w3d_init(uint8_t *self, uint32_t argument) {
    g_w3d_init++;
    uint32_t keys[4] = {K0, 0, K2, K3};
    ((void(__thiscall *)(void *, uint32_t, void *, void *, void *))0x488290)(self, 1, self + 0x28, &keys[2], &keys[0]);
    *(uint32_t *)(self + ARG) = argument;
    *(uint32_t *)self = VTABLE;
    *(uint32_t *)(self + CLEAR) = 0;
    return self;
}
REWRITE("Keyed::Init506130", 0x506130, w3c_start<0x448df18b5610ec83>, (&w3d_init<0, 0x81fbc001, 0x4780d, 0x30, 0x2c, 0xf36bc0>), &g_w3d_init);
REWRITE("Keyed::Init506130", 0x5bde60, w3c_start<0x448df18b5610ec83>, (&w3d_init<0x80000000, 0x8000000c, 0, 0x34, 0x30, 0xf3f078>), &g_w3d_init);

// ---- Message::Send: if there is a target, builds a message from the arguments and hands it over; then lets go of what the
// source holds -------------------------------------------------------------------------------------------------------------------
static volatile long g_w3d_send;
template <uint32_t SIZE, uintptr_t BUILD, uintptr_t DELIVER> static void __thiscall w3d_send(void **self, uint32_t a, uint32_t b, uint8_t *source) {
    g_w3d_send++;
    void *target = *self;
    if (!target) return;
    alignas(4) uint8_t message[SIZE];
    ((void(__thiscall *)(void *, uint32_t, uint32_t, void *))BUILD)(message, a, b, source);
    ((void(__thiscall *)(void *, void *))DELIVER)(target, message);
    if (uint32_t held = *(uint32_t *)(source + 0x18)) {
        ((void(__thiscall *)(void *, uint32_t))(*(void ***)source)[1])(source, held);
        *(uint32_t *)(source + 0x18) = 0;
    }
}
REWRITE("Message::Send48C5D0", 0x48c5d0, w3c_start<0xff85398b571cec83>, (&w3d_send<0x1c, 0x4884b0, 0x48bb30>), &g_w3d_send);
REWRITE("Message::Send48C5D0", 0x48c8e0, w3c_start<0xff85398b572cec83>, (&w3d_send<0x2c, 0x488660, 0x48bd10>), &g_w3d_send);

// ---- Maker::Blank (another size): a new object built with four zeros -----------------------------------------------------------
REWRITE("Maker::Blank818050", 0x818050, w3c_start<0x6400e5d79b68ff6a>, (&w3c_make0<0xb8, 0x817070>), &g_w3c_make0);
REWRITE("Maker::Blank818050", 0x818e80, w3c_start<0x6400e5d93b68ff6a>, (&w3c_make0<0x80, 0x8185b0>), &g_w3c_make0);

// ---- Child::Attach: makes a new 0xa0-byte child of this object and keeps it at +0x1c (null if out of memory) -----------------
static volatile long g_w3d_attach;
template <uintptr_t CTOR> static void *__thiscall w3d_attach(uint8_t *self, uint32_t) {
    g_w3d_attach++;
    void *made = memory_alloc(0xa0);
    void *child = made ? ((void *(__thiscall *)(void *, uint32_t, void *))CTOR)(made, 0, self) : nullptr;
    *(void **)(self + 0x1c) = child;
    return child;
}
REWRITE("Child::Attach5F4F30", 0x5f4f30, w3c_start<0x6400e26d8b68ff6a>, (&w3d_attach<0x712190>), &g_w3d_attach);
REWRITE("Child::Attach5F4F30", 0x616680, w3c_start<0x6400e2affb68ff6a>, (&w3d_attach<0x8d0bf0>), &g_w3d_attach);

// ---- Range::Add: a new ranged value (signed or unsigned 16-bit limits) from the second argument, added under the first ------
static volatile long g_w3d_range;
template <int32_t LOW, int32_t HIGH, uintptr_t CTOR, uintptr_t ADD> static uint32_t __thiscall w3d_range(void *self, uint32_t key, uint32_t value) {
    g_w3d_range++;
    void *made = memory_alloc(0x14);
    void *item = made ? ((void *(__thiscall *)(void *, uint32_t, int32_t, int32_t, uint32_t))CTOR)(made, value, LOW, HIGH, 1) : nullptr;
    return ((uint32_t(__thiscall *)(void *, uint32_t, void *))ADD)(self, key, item);
}
REWRITE("Range::AddC33DA0", 0xc33da0, w3c_start<0x6400ea049b68ff6a>, (&w3d_range<-0x8000, 0x7fff, 0xca8630, 0xc33c00>), &g_w3d_range);
REWRITE("Range::AddC33DA0", 0xc344e0, w3c_start<0x6400ea05db68ff6a>, (&w3d_range<-0x8000, 0x7fff, 0xca8630, 0xc34340>), &g_w3d_range);
REWRITE("Range::AddC33E20", 0xc33e20, w3c_start<0x6400ea04bb68ff6a>, (&w3d_range<0, 0xffff, 0xca8660, 0xc33c00>), &g_w3d_range);
REWRITE("Range::AddC33E20", 0xc34560, w3c_start<0x6400ea05fb68ff6a>, (&w3d_range<0, 0xffff, 0xca8660, 0xc34340>), &g_w3d_range);

// ---- Single::Get (three zeros): the one instance; building it makes it register itself in the global ---------------------------
static volatile long g_w3d_single;
template <uint32_t INSTANCE, uint32_t SIZE, uintptr_t CTOR> static void *__cdecl w3d_single() {
    g_w3d_single++;
    void *&instance = *(void **)INSTANCE;
    if (!instance)
        if (void *made = memory_alloc(SIZE)) ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))CTOR)(made, 0, 0, 0);
    return instance;
}
REWRITE("Single::Get4779C0", 0x4779c0, w3c_start<0x6400e0d58b68ff6a>, (&w3d_single<0x12f523c, 0xf0, 0x54bc30>), &g_w3d_single);
REWRITE("Single::Get4779C0", 0x7728d0, w3c_start<0x6400e5165b68ff6a>, (&w3d_single<0x1306568, 0xb8, 0x771670>), &g_w3d_single);

// ---- Holder::Init: two vtables, the list member built from the argument, the second base built, then the list told its owner
static volatile long g_w3d_holder;
template <uint32_t VTABLE> static void *__thiscall w3d_holder(uint32_t *self, uint32_t argument) {
    g_w3d_holder++;
    self[1] = 0xf6c524;
    self[0] = VTABLE;
    ((void(__thiscall *)(void *, uint32_t))0xbd9c10)(self + 2, argument);
    ((void(__thiscall *)(void *))0xbd1550)(self + 15);
    self[15] = 0xf6c51c;
    ((void(__thiscall *)(void *, void *))0xbd9a90)(self + 2, self);
    return self;
}
REWRITE("Holder::Init7577E0", 0x7577e0, w3c_start<0x6400e5015e68ff6a>, (&w3d_holder<0xf6c504>), &g_w3d_holder);
REWRITE("Holder::Init7577E0", 0x7fa670, w3c_start<0x6400e5b06e68ff6a>, (&w3d_holder<0xf7510c>), &g_w3d_holder);

// ---- Array::Make: `count` 12-byte elements built in one block that starts with the count (null if out of memory) -------------
static volatile long g_w3d_array;
template <uintptr_t CTOR, uintptr_t DTOR> static void *__thiscall w3d_array(uint32_t *self, uint32_t count) {
    g_w3d_array++;
    self[1] = count;
    uint32_t *block = (uint32_t *)memory_alloc_aligned(count * 12 + 4);
    if (block) {
        block[0] = count;
        ((void(__stdcall *)(void *, uint32_t, uint32_t, void *, void *))0xddcf99)(block + 1, 12, count, (void *)CTOR, (void *)DTOR);
        self[0] = (uint32_t)(uintptr_t)(block + 1);
    } else
        self[0] = 0;
    return self;
}
REWRITE("Array::Make823180", 0x823180, w3c_start<0xff6a00000000a164>, (&w3d_array<0x822c70, 0x822bc0>), &g_w3d_array);
REWRITE("Array::Make823180", 0xa320b0, w3c_start<0xff6a00000000a164>, (&w3d_array<0xa30810, 0xa30820>), &g_w3d_array);

// ---- Map::Get: the item under the key, made and added (under its own key field at +8) when it is missing -------------------
static volatile long g_w3d_get;
template <uint32_t MAP, uintptr_t CTOR> static void *__thiscall w3d_get(uint8_t *self, uint32_t key) {
    g_w3d_get++;
    void *map = self + MAP;
    if (void *found = ((void *(__thiscall *)(void *, uint32_t))0xbe82b0)(map, key)) return found;
    void *made = memory_alloc(0x4c);
    uint8_t *item = made ? ((uint8_t * (__thiscall *)(void *, uint32_t))CTOR)(made, key) : nullptr;
    ((void(__thiscall *)(void *, uintptr_t, void *))0xbe8230)(map, (uintptr_t)item + 8, item);   // +8 even for null
    return item;
}
REWRITE("Map::Get881EE0", 0x881ee0, w3c_start<0xff6a00000000a164>, (&w3d_get<0x34, 0x87f7b0>), &g_w3d_get);
REWRITE("Map::Get881EE0", 0xb33760, w3c_start<0xff6a00000000a164>, (&w3d_get<0x14, 0xb32e20>), &g_w3d_get);

// ---- Text::From: a new text copied from the source's text, or the shared empty one when the source has none, or the copy's
// default text when there is no source; returned through `out` -------------------------------------------------------------------
static volatile long g_w3d_text;
template <uint32_t SOURCE, uint32_t FIELD, uint32_t TEXT> static void **__thiscall w3d_text(uint8_t *self, void **out) {
    g_w3d_text++;
    void *text;
    if (uint8_t *source = *(uint8_t **)(self + SOURCE)) {
        if (void *from = *(void **)(source + FIELD)) {
            void *made = memory_alloc(0x10);
            text = made ? ((void *(__thiscall *)(void *, void *))0xbcf7d0)(made, from) : nullptr;
        } else
            text = ((void *(__fastcall *)(uint8_t))0xbd0490)(1);
    } else {
        void *made = memory_alloc(0x10);
        text = made ? ((void *(__thiscall *)(void *, uint32_t))0xbcf850)(made, TEXT) : nullptr;
    }
    *out = text;
    return out;
}
REWRITE("Text::FromA2C8E0", 0xa2c8e0, w3c_start<0x6400e7c76668ff6a>, (&w3d_text<0x7c, 0xc, 0xf65930>), &g_w3d_text);
REWRITE("Text::FromA2C8E0", 0xba52d0, w3c_start<0x6400e9aec668ff6a>, (&w3d_text<0x50, 0x4, 0xf9d13c>), &g_w3d_text);

// ---- Shared::Own: before changing a shared buffer, a buffer still used elsewhere is let go of and copied (with the argument);
// one used only here is changed in place --------------------------------------------------------------------------------------------
static volatile long g_w3d_own;
template <uintptr_t COPY, uintptr_t CHANGE> static uint32_t __thiscall w3d_own(int32_t **self, uint32_t argument) {
    g_w3d_own++;
    int32_t *shared = *self;
    if (*shared <= 1) return ((uint32_t(__thiscall *)(void *, uint32_t))CHANGE)(shared, argument);
    --*shared;
    void *made = memory_alloc(0x10);
    *self = made ? ((int32_t * (__thiscall *)(void *, void *, uint32_t))COPY)(made, shared, argument) : nullptr;
    return (uint32_t)(uintptr_t)*self;
}
REWRITE("Shared::OwnBCFFE0", 0xbcffe0, w3c_start<0x6400e9d4cb68ff6a>, (&w3d_own<0xbcf8c0, 0xbcf980>), &g_w3d_own);
REWRITE("Shared::OwnBCFFE0", 0xbd02b0, w3c_start<0x6400e9d54b68ff6a>, (&w3d_own<0xbcfb80, 0xbcfc30>), &g_w3d_own);

// ---- Path::Tail: the text after the last separator (a backslash for the name, '.' for the extension) in the path at +0x448 --
static volatile long g_w3d_tail;
template <char SEPARATOR> static void **__thiscall w3d_tail(uint8_t *self, void **out) {
    g_w3d_tail++;
    const char *path = (const char *)self + 0x448;
    uintptr_t at = (uintptr_t)path + __builtin_strlen(path) - 1;
    while (at >= (uintptr_t)path && *(const char *)at != SEPARATOR) at--;
    uintptr_t tail = at + 1;
    void *text;
    if (tail) {
        void *made = memory_alloc(0x10);
        text = made ? ((void *(__thiscall *)(void *, uintptr_t))0xbcf850)(made, tail) : nullptr;
    } else
        text = ((void *(__fastcall *)(uint8_t))0xbd0490)(1);
    *out = text;
    return out;
}
REWRITE("Path::TailCA64C0", 0xca64c0, w3c_start<0x6400ea494b68ff6a>, (&w3d_tail<0x5c>), &g_w3d_tail);
REWRITE("Path::TailCA64C0", 0xca6620, w3c_start<0x6400ea498b68ff6a>, (&w3d_tail<0x2e>), &g_w3d_tail);

// ---- Hash::Init: a table sized for the wanted count: a cleared bucket array, the built slot array (count first) and a third
// array ------------------------------------------------------------------------------------------------------------------------------
static volatile long g_w3d_hash;
template <uintptr_t SIZE_FOR> static void *__thiscall w3d_hash(uint32_t *self, uint32_t wanted) {
    g_w3d_hash++;
    self[1] = ((uint32_t(__fastcall *)(uint32_t))SIZE_FOR)(wanted);
    self[2] = 0;
    self[3] = (uint32_t)(uintptr_t)memory_alloc_aligned(self[1] * 4 + 4);
    uint32_t count = self[1];
    uint32_t *block = (uint32_t *)memory_alloc_aligned(count * 4 + 4);
    uint32_t *slots = nullptr;
    if (block) {
        block[0] = count;
        ((void(__stdcall *)(void *, uint32_t, uint32_t, void *, void *))0xddcf99)(block + 1, 4, count, (void *)0x448170, (void *)0x403de0);
        slots = block + 1;
    }
    self[4] = (uint32_t)(uintptr_t)slots;
    self[5] = (uint32_t)(uintptr_t)memory_alloc_aligned(self[1] * 4);
    __builtin_memset((void *)(uintptr_t)self[3], 0, self[1] * 4 + 4);
    return self;
}
REWRITE("Hash::InitBDFAE0", 0xbdfae0, w3c_start<0xff6a00000000a164>, (&w3d_hash<0xbdd460>), &g_w3d_hash);
REWRITE("Hash::InitBDFAE0", 0xbdfbb0, w3c_start<0xff6a00000000a164>, (&w3d_hash<0xbdd4b0>), &g_w3d_hash);

// ---- Text::Pass: the C string (or the shared empty text) as a text, handed to the object's virtual (slot 0x68) with the other
// arguments, then let go of --------------------------------------------------------------------------------------------------------
static volatile long g_w3d_pass;
template <uintptr_t FROM>
static void __thiscall w3d_pass(void **self, uint32_t a, const char *string, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    g_w3d_pass++;
    int32_t *text;
    if (string) {
        void *made = memory_alloc(0x10);
        text = made ? ((int32_t * (__thiscall *)(void *, const char *))FROM)(made, string) : nullptr;
    } else
        text = ((int32_t * (__fastcall *)(uint8_t))0xbd0490)(1);
    ((void(__thiscall *)(void *, uint32_t, int32_t **, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))(*(void ***)self)[0x68 / 4])(
        self, a, &text, (uint32_t)text[1], b, c, d, e);
    if (--*text == 0) {
        memory_free_aligned((void *)(uintptr_t)text[3]);
        memory_free(text);
    }
}
REWRITE("Text::Pass4042F0", 0x4042f0, w3c_start<0x6400e04be368ff6a>, (&w3d_pass<0xbcf7d0>), &g_w3d_pass);
REWRITE("Text::Pass4042F0", 0xbf92c0, w3c_start<0x6400e9e75368ff6a>, (&w3d_pass<0xbcf850>), &g_w3d_pass);

// ---- Array::Resize: new 32-byte elements (built by the copy's constructor), the kept ones assigned over from the old array
// (an element of the exact class also takes its virtual base's first field), then the old array freed ---------------------------
static volatile long g_w3d_resize;
template <uintptr_t CTOR, uint32_t VBTABLE> static int32_t __thiscall w3d_resize(uint8_t **self, int32_t count) {
    g_w3d_resize++;
    int32_t old_count = *(int32_t *)(self + 1);
    if (count == old_count) return count;
    uint8_t *data = nullptr;
    if (count) {
        data = (uint8_t *)memory_alloc_aligned((uint32_t)count << 5);
        if (data) ((void(__stdcall *)(void *, uint32_t, int32_t, void *))0x5d3d00)(data, 0x20, count, (void *)CTOR);
        int32_t kept = count < old_count ? count : old_count;
        for (int32_t i = 0; i < kept; i++) {
            uint8_t *to = data + i * 0x20;
            uintptr_t from = (uintptr_t)self[0] + i * 0x20;
            uint32_t *table = *(uint32_t **)to;
            if ((uint32_t)(uintptr_t)table == VBTABLE) {
                uintptr_t base = from ? from + (*(uint32_t **)from)[1] : 0;
                *(uint32_t *)(to + table[1]) = *(uint32_t *)base;
            }
            ((void(__thiscall *)(void *, uintptr_t))0x5d2970)(to, from);
            *(uint32_t *)(to + 0x18) = *(uint32_t *)(from + 0x18);
        }
    }
    if (self[0]) {
        memory_free_aligned(self[0]);
        self[0] = nullptr;
    }
    self[0] = data;
    *(int32_t *)(self + 1) = count;
    return count;
}
REWRITE("Array::Resize5D73D0", 0x5d73d0, w3c_start<0x458be98b550cec83>, (&w3d_resize<0x5d4600, 0xf3f734>), &g_w3d_resize);
REWRITE("Array::Resize5D73D0", 0x5d7530, w3c_start<0x458be98b550cec83>, (&w3d_resize<0x5d4620, 0xf3afc0>), &g_w3d_resize);
REWRITE("Array::Resize5D73D0", 0x5d7630, w3c_start<0x458be98b550cec83>, (&w3d_resize<0x5d4640, 0xf3affc>), &g_w3d_resize);

// ---- Entries::UpdateAll: every 28-byte entry whose (lazily looked up, as Cached::Find does) target is there gets updated -----
static inline uint32_t w3d_entry_target(uint8_t *entry) {
    if (!entry[0x18]) {
        uint8_t *object = *(uint8_t **)(*(uint8_t **)(entry + 4) + 0xc);
        uint32_t found = 0;
        if (object) {
            uint8_t *table = ((uint8_t * (__thiscall *)(void *))(*(void ***)object)[1])(object);
            int32_t slot = *(int32_t *)0x13ce3d8;
            if (table && slot < *(int32_t *)(table + 0x18))
                if (uint8_t *found_entry = (*(uint8_t ***)(table + 0x14))[slot]) found = *(uint32_t *)(found_entry + 0x10) + (uint32_t)(uintptr_t)object;
        }
        *(uint32_t *)(entry + 0x14) = found;
        entry[0x18] = 1;
    }
    return *(uint32_t *)(entry + 0x14);
}
static volatile long g_w3d_update;
template <uintptr_t UPDATE> static int32_t __fastcall w3d_update_all(uint8_t *self) {
    g_w3d_update++;
    int32_t count = *(int32_t *)(self + 0x28);
    if (count <= 0) return count;
    int32_t i = 0;
    do {
        if (w3d_entry_target(*(uint8_t **)(self + 0x24) + i * 0x1c))
            ((void(__thiscall *)(uint32_t))UPDATE)(w3d_entry_target(*(uint8_t **)(self + 0x24) + i * 0x1c));
    } while (++i < *(int32_t *)(self + 0x28));
    return i;
}
REWRITE("Entries::UpdateAllA9F7E0", 0xa9f7e0, w3c_start<0x28458be98b555351>, (&w3d_update_all<0xabd040>), &g_w3d_update);
REWRITE("Entries::UpdateAllA9F7E0", 0xaa0680, w3c_start<0x28458be98b555351>, (&w3d_update_all<0xabfe20>), &g_w3d_update);

// ---- Item::Enlist: a new item for this owner (set up unless it already is), handed to the one registry (made, and set up,
// when there is none yet) ----------------------------------------------------------------------------------------------------------
static volatile long g_w3d_enlist;
template <uint32_t SIZE, uintptr_t CTOR> static uint32_t __fastcall w3d_enlist(void *owner) {
    g_w3d_enlist++;
    void *made = memory_alloc(SIZE);
    uint8_t *item = made ? ((uint8_t * (__thiscall *)(void *, uint32_t, uint32_t, uint32_t, void *))CTOR)(made, 0, 0, 0, owner) : nullptr;
    auto set_up = (void(__thiscall *)(void *))0x800860;
    if (!(item[0x2c] & 1)) set_up(item);
    uint8_t *&registry = *(uint8_t **)0x12f5798;
    if (!registry)
        if (void *made_registry = memory_alloc(0x58)) ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))0x61e450)(made_registry, 0, 0, 0);
    if (!(registry[0x2c] & 1)) set_up(registry);
    return ((uint32_t(__thiscall *)(void *, void *))0x61ffe0)(registry, item);
}
REWRITE("Item::Enlist6203A0", 0x6203a0, w3c_start<0x6400e2c68668ff6a>, (&w3d_enlist<0x70, 0x61c9b0>), &g_w3d_enlist);
REWRITE("Item::Enlist6203A0", 0x9daf50, w3c_start<0x6400e757c668ff6a>, (&w3d_enlist<0x78, 0x9dad90>), &g_w3d_enlist);
REWRITE("Item::Enlist6203A0", 0xa5b830, w3c_start<0x6400e8268668ff6a>, (&w3d_enlist<0x78, 0xa5b630>), &g_w3d_enlist);

// ---- Array::Resize (plain elements): new elements built by the copy's constructor, the kept ones assigned over, the old array
// freed ------------------------------------------------------------------------------------------------------------------------------
static volatile long g_w3d_resize2;
template <uint32_t SHIFT, uintptr_t CTOR, uintptr_t ASSIGN> static int32_t __thiscall w3d_resize2(uint8_t **self, int32_t count) {
    g_w3d_resize2++;
    int32_t old_count = *(int32_t *)(self + 1);
    if (count == old_count) return count;
    uint8_t *data = nullptr;
    if (count) {
        data = (uint8_t *)memory_alloc_aligned((uint32_t)count << SHIFT);
        if (data) ((void(__stdcall *)(void *, uint32_t, int32_t, void *))0x402b90)(data, 1u << SHIFT, count, (void *)CTOR);
        int32_t kept = count < old_count ? count : old_count;
        for (int32_t i = 0; i < kept; i++)
            ((void(__thiscall *)(uintptr_t, uintptr_t))ASSIGN)((uintptr_t)data + (i << SHIFT), (uintptr_t)self[0] + (i << SHIFT));
    }
    if (self[0]) {
        memory_free_aligned(self[0]);
        self[0] = nullptr;
    }
    self[0] = data;
    *(int32_t *)(self + 1) = count;
    return count;
}
REWRITE("Array::Resize61C320", 0x61c320, w3c_start<0xff6a00000000a164>, (&w3d_resize2<4, 0x4073a0, 0xbe7450>), &g_w3d_resize2);
REWRITE("Array::Resize61C320", 0x693300, w3c_start<0xff6a00000000a164>, (&w3d_resize2<6, 0x691b00, 0x691b10>), &g_w3d_resize2);
