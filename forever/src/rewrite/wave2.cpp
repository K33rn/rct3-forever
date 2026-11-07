// Rewrites of wave 2.
#include "wave.h"

// ---- Stamp::Compare: orders two stamps {unsigned a, unsigned b, three signed ints, signed tail}: 1, 0 or -1 ----------------------
static volatile long g_w2_stamps;
static int32_t __stdcall stamp_compare(uint32_t a0, uint32_t a1, const int32_t *a2, int32_t a3, uint32_t b0, uint32_t b1,
                                       const int32_t *b2, int32_t b3) {
    g_w2_stamps++;
    if (a0 != b0) return a0 > b0 ? 1 : -1;
    if (a1 != b1) return a1 > b1 ? 1 : -1;
    for (int i = 0; i < 3; i++)
        if (a2[i] != b2[i]) return a2[i] > b2[i] ? 1 : -1;
    if (a3 != b3) return a3 > b3 ? 1 : -1;
    return 0;
}
static const uint8_t stamp_compare_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0x4c, 0x24, 0x14};
REWRITE("Stamp::Compare88F750", 0x88f750, stamp_compare_original, stamp_compare, &g_w2_stamps);
REWRITE("Stamp::Compare88F750", 0x973140, stamp_compare_original, stamp_compare, &g_w2_stamps);

// ---- Linked::LeaveBoth: an object on two lists at once, with its nodes at +0 and +0xc, leaves them ----------------------------------
static volatile long g_w2_leave_both;
static void __fastcall linked_leave_both(Node *n) {
    g_w2_leave_both++;
    unlink(n);
    unlink((Node *)((uint8_t *)n + 0xc));
}
static const uint8_t leave_both_original[] = {0x85, 0xc9, 0x74, 0x2f, 0x8b, 0x51, 0x04, 0x85};
REWRITE("Linked::LeaveBoth49B6E0", 0x49b6e0, leave_both_original, linked_leave_both, &g_w2_leave_both);
REWRITE("Linked::LeaveBoth49B6E0", 0x64bf60, leave_both_original, linked_leave_both, &g_w2_leave_both);

// ---- Linked::Leave: a node leaves its list; one that is not properly on a list is left alone ----------------------------------------
static volatile long g_w2_leaves;
static void __fastcall linked_leave(Node *n) {
    g_w2_leaves++;
    if (n && n->prev && n->next && n->next->prev == n && n->prev->next == n) {
        n->next->prev = n->prev;
        n->prev->next = n->next;
        n->next = nullptr;
        n->prev = nullptr;
    }
}
static const uint8_t linked_leave_original[] = {0x85, 0xc9, 0x74, 0x2d, 0x8b, 0x51, 0x04, 0x85};
REWRITE("Linked::Leave593700", 0x593700, linked_leave_original, linked_leave, &g_w2_leaves);
REWRITE("Linked::Leave593700", 0x5c24f0, linked_leave_original, linked_leave, &g_w2_leaves);
REWRITE("Linked::Leave593700", 0xc51760, linked_leave_original, linked_leave, &g_w2_leaves);

// ---- ListPair::Construct: an owner value, then two empty lists of 6 words (head and end nodes that point at each other) -------------
static volatile long g_w2_list_pairs;
static void empty_list(uint32_t *list) {
    list[0] = (uint32_t)(uintptr_t)(list + 3);
    list[1] = 0;
    list[2] = 0;
    list[3] = 0;
    list[4] = (uint32_t)(uintptr_t)list;
    list[5] = 0;
}
static uint32_t *__thiscall list_pair_construct(uint32_t *self, uint32_t owner) {
    g_w2_list_pairs++;
    self[0] = owner;
    empty_list(self + 1);
    empty_list(self + 7);
    return self;
}
static const uint8_t list_pair_original[] = {0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x04, 0x89, 0x08};
REWRITE("ListPair::ConstructBFA820", 0xbfa820, list_pair_original, list_pair_construct, &g_w2_list_pairs);
REWRITE("ListPair::ConstructBFA820", 0xc2f190, list_pair_original, list_pair_construct, &g_w2_list_pairs);

// ---- Event::Send: wraps two values in an event of the given type and hands it to the object's own handler ---------------------------
static volatile long g_w2_events;
template <uint32_t TABLE> static uint32_t __thiscall event_send(void **self, uint32_t a, uint32_t b) {
    g_w2_events++;
    uint32_t event[3] = {TABLE, a, b};
    return ((uint32_t(__thiscall *)(void *, uint32_t *))(*(void ***)self)[0x20])(self, event);
}
static const uint8_t event_send_original[] = {0x83, 0xec, 0x0c, 0x8b, 0x54, 0x24, 0x14, 0x8b};
REWRITE("Event::Send8451D0", 0x8451d0, event_send_original, (&event_send<0xf77ba4>), &g_w2_events);
REWRITE("Event::Send8451D0", 0x845240, event_send_original, (&event_send<0xf77ba8>), &g_w2_events);
REWRITE("Event::Send8451D0", 0x8452b0, event_send_original, (&event_send<0xf77bac>), &g_w2_events);
REWRITE("Event::Send8451D0", 0x845320, event_send_original, (&event_send<0xf77bb0>), &g_w2_events);

// ---- Lock::Release: if held, drops the lock at (+0x18)->+4; the last holder runs the jobs that were waiting on it -------------------
static volatile long g_w2_lock_releases;
static void __fastcall lock_release(uint8_t *self) {
    g_w2_lock_releases++;
    if (!*(uint32_t *)(self + 0x20)) return;
    uint8_t *lock = *(uint8_t **)(*(uint8_t **)(self + 0x18) + 4);
    uint16_t &holders = *(uint16_t *)(lock + 0xc), &waiting = *(uint16_t *)(lock + 0xe);
    if (holders && --holders == 0)
        while (waiting) {
            void **job = *(void ***)(lock + 4);
            ((void(__stdcall *)(void *))(*(void ***)job)[0xc])(job);
            waiting--;
        }
    *(uint32_t *)(self + 0x20) = 0;
}
static const uint8_t lock_release_original[] = {0x57, 0x8b, 0xf9, 0x8b, 0x47, 0x20, 0x85, 0xc0};
REWRITE("Lock::ReleaseBE9BE0", 0xbe9be0, lock_release_original, lock_release, &g_w2_lock_releases);
REWRITE("Lock::ReleaseBE9BE0", 0xbe9dd0, lock_release_original, lock_release, &g_w2_lock_releases);

// ---- Triple::Forward: three values passed on as one record to the real function ---------------------------------------------------
static volatile long g_w2_triples;
template <uint32_t TARGET> static uint32_t __thiscall triple_forward(void *self, uint32_t a, uint32_t b, uint32_t c) {
    g_w2_triples++;
    uint32_t triple[3] = {a, b, c};
    return ((uint32_t(__thiscall *)(void *, uint32_t *))TARGET)(self, triple);
}
static const uint8_t triple_original[] = {0x83, 0xec, 0x0c, 0x8b, 0x54, 0x24, 0x14, 0x8b, 0x44, 0x24, 0x10, 0x89, 0x54, 0x24, 0x04};
REWRITE("Triple::Forward48BB00", 0x48bb00, triple_original, (&triple_forward<0x48aff0>), &g_w2_triples);
REWRITE("Triple::Forward48BB00", 0x7b9070, triple_original, (&triple_forward<0x7b7e70>), &g_w2_triples);
REWRITE("Triple::Forward48BB00", 0xb5eaa0, triple_original, (&triple_forward<0xb5e100>), &g_w2_triples);
REWRITE("Triple::Forward48BB00", 0xb74da0, triple_original, (&triple_forward<0xb74190>), &g_w2_triples);

// ---- Node::Free: a node leaves its list and is freed ------------------------------------------------------------------------------
static volatile long g_w2_node_frees;
static void __stdcall node_free(Node *n) {
    g_w2_node_frees++;
    unlink(n);
    memory_free(n);
}
static const uint8_t node_free_original[] = {0x8b, 0x44, 0x24, 0x04, 0x85, 0xc0, 0x74, 0x39};
REWRITE("Node::Free846450", 0x846450, node_free_original, node_free, &g_w2_node_frees);
REWRITE("Node::Free846450", 0x8464e0, node_free_original, node_free, &g_w2_node_frees);

// ---- Children::Tell: every child of the array at +0x2b4 that exists gets the value through virtual slot SLOT ----------------------
static volatile long g_w2_tells;
template <uint32_t SLOT> static void __thiscall children_tell(uint8_t *self, uint32_t value) {
    g_w2_tells++;
    for (int32_t i = 0; i < *(int32_t *)(self + 0x2b8); i++) {
        void **child = (*(void ****)(self + 0x2b4))[i];
        if (child) ((void(__thiscall *)(void *, uint32_t))(*(void ***)child)[SLOT])(child, value);
    }
}
static const uint8_t children_tell_original[] = {0x56, 0x57, 0x8b, 0xf9, 0x8b, 0x87, 0xb8, 0x02};
REWRITE("Children::TellB6C590", 0xb6c590, children_tell_original, (&children_tell<1>), &g_w2_tells);
REWRITE("Children::TellB6C590", 0xb6c5d0, children_tell_original, (&children_tell<2>), &g_w2_tells);
REWRITE("Children::TellB6C590", 0xb6c610, children_tell_original, (&children_tell<3>), &g_w2_tells);

// ---- ArrayHolder::Clear: deletes the array it holds; the block starts BACK bytes before it, with the count that says it has objects --
static volatile long g_w2_array_clears;
template <uint32_t BACK> static void __fastcall array_clear(uint8_t **holder) {
    g_w2_array_clears++;
    uint8_t *items = *holder;
    if (!items) return;
    if (*(uint32_t *)(items - BACK))
        ((void(__thiscall *)(void *, uint32_t))(*(void ***)items)[0])(items, 3);
    else
        memory_free_aligned(items - BACK);
    *holder = nullptr;
}
static const uint8_t array_clear_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x0e, 0x85, 0xc9, 0x74, 0x27, 0x8b, 0x51};
REWRITE("ArrayHolder::Clear47E380", 0x47e380, array_clear_original, (&array_clear<0x10>), &g_w2_array_clears);
REWRITE("ArrayHolder::Clear47E380", 0xac55a0, array_clear_original, (&array_clear<4>), &g_w2_array_clears);
REWRITE("ArrayHolder::Clear47E380", 0xb75c40, array_clear_original, (&array_clear<4>), &g_w2_array_clears);
REWRITE("ArrayHolder::Clear47E380", 0xb75cd0, array_clear_original, (&array_clear<4>), &g_w2_array_clears);

// ---- Refs::InterfaceAt: the interface of the index'th 20-byte reference at +OFF, by the id in the global ---------------------------
static volatile long g_w2_refs_at;
template <uint32_t OFF> static void *__thiscall refs_interface_at(uint8_t *self, int32_t index) {
    g_w2_refs_at++;
    return find_interface(*(uint8_t **)(self + OFF + index * 20 + 4), (const int32_t *)0x1310f90);
}
static const uint8_t refs_at_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8d, 0x04, 0x80, 0x8d};
REWRITE("Refs::InterfaceAt738270", 0x738270, refs_at_original, (&refs_interface_at<0xd0>), &g_w2_refs_at);
REWRITE("Refs::InterfaceAt738270", 0xac2a60, refs_at_original, (&refs_interface_at<0xc4>), &g_w2_refs_at);

// ---- Record::Assign: a copy that also fixes up the virtual base if this is the most derived type, then the word at +0x18 ------------
static volatile long g_w2_assigns;
template <uint32_t TABLE> static uint32_t *__thiscall record_assign(uint32_t *self, uint32_t *other) {
    g_w2_assigns++;
    if (self[0] == TABLE) {
        // both sides keep the offset of the shared part in their table's second word
        uint32_t *from = (uint32_t *)((uint8_t *)other + ((int32_t *)(uintptr_t)*other)[1]);
        *(uint32_t *)((uint8_t *)self + ((int32_t *)TABLE)[1]) = *from;
    }
    ((void(__thiscall *)(void *, void *))0x5d2970)(self, other);
    self[6] = other[6];
    return self;
}
static const uint8_t record_assign_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x0e, 0x81, 0xf9};
REWRITE("Record::Assign5D3DE0", 0x5d3de0, record_assign_original, (&record_assign<0xf3affc>), &g_w2_assigns);
REWRITE("Record::Assign5D3DE0", 0x5d3f70, record_assign_original, (&record_assign<0xf3f734>), &g_w2_assigns);
REWRITE("Record::Assign5D3DE0", 0x5d3fb0, record_assign_original, (&record_assign<0xf3afc0>), &g_w2_assigns);

// ---- Quad::Construct: a table and four values ---------------------------------------------------------------------------------------
static volatile long g_w2_quads;
template <uint32_t TABLE> static uint32_t *__thiscall quad_construct(uint32_t *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    g_w2_quads++;
    self[0] = TABLE;
    self[1] = a;
    self[2] = b;
    self[3] = c;
    self[4] = d;
    return self;
}
static const uint8_t quad_original[] = {0x8b, 0x54, 0x24, 0x08, 0x8b, 0xc1, 0x8b, 0x4c};
REWRITE("Quad::ConstructCA8000", 0xca8000, quad_original, (&quad_construct<0xfbf76c>), &g_w2_quads);
REWRITE("Quad::ConstructCA8000", 0xca8630, quad_original, (&quad_construct<0xfbf82c>), &g_w2_quads);
REWRITE("Quad::ConstructCA8000", 0xca8660, quad_original, (&quad_construct<0xfbf844>), &g_w2_quads);

// ---- Kind::Map: a kind from 0 to 6 translated by the table, anything else is 7 -----------------------------------------------------
static volatile long g_w2_clamps;
static const uint8_t kind_map_a[7] = {3, 2, 1, 0, 4, 5, 6};
static const uint8_t kind_map_b[7] = {3, 4, 2, 5, 1, 6, 0};
template <const uint8_t *MAP> static uint32_t __fastcall kind_map(uint32_t kind) {
    g_w2_clamps++;
    return kind > 6 ? 7 : MAP[kind];
}
static const uint8_t kind_map_original[] = {0x83, 0xf9, 0x06, 0x77, 0x2e, 0xff, 0x24, 0x8d};
REWRITE("Kind::Map5D1470", 0x5d1470, kind_map_original, (&kind_map<kind_map_a>), &g_w2_clamps);
REWRITE("Kind::Map5D1470", 0x5d14d0, kind_map_original, (&kind_map<kind_map_b>), &g_w2_clamps);

// ---- Words::Copy: a plain copy of WORDS words -----------------------------------------------------------------------------------------
static volatile long g_w2_copies;
template <uint32_t WORDS> static uint32_t *__thiscall words_copy(uint32_t *self, const uint32_t *from) {
    g_w2_copies++;
    for (uint32_t i = 0; i < WORDS; i++) self[i] = from[i];
    return self;
}
static const uint8_t words6_original[] = {0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x04, 0x8b, 0x11, 0x89, 0x10, 0x8b, 0x51, 0x04, 0x89, 0x50, 0x04,
                                          0x8b, 0x51, 0x08, 0x89, 0x50, 0x08, 0x8b, 0x51, 0x0c, 0x89, 0x50, 0x0c, 0x8b, 0x51};
static const uint8_t words4_original[] = {0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x04, 0x8b, 0x11, 0x89, 0x10, 0x8b, 0x51,
                                          0x04, 0x89, 0x50, 0x04, 0x8b, 0x51, 0x08, 0x89, 0x50, 0x08, 0x8b, 0x49};
REWRITE("Words::Copy843950", 0x843950, words6_original, (&words_copy<6>), &g_w2_copies);
REWRITE("Words::Copy843950", 0x8439b0, words6_original, (&words_copy<6>), &g_w2_copies);
REWRITE("Words::Copy47D700", 0x47d700, words4_original, (&words_copy<4>), &g_w2_copies);
REWRITE("Words::Copy47D700", 0xbe7450, words4_original, (&words_copy<4>), &g_w2_copies);

// ---- Items::ValueAt: the word at +0x44 of the index'th item of the array at +4, 0 for an empty place --------------------------------
static volatile long g_w2_values;
static uint32_t __thiscall items_value_at(uint8_t *self, int32_t index) {
    g_w2_values++;
    uint8_t *item = (*(uint8_t ***)(self + 4))[index];
    return item ? *(uint32_t *)(item + 0x44) : 0;
}
static const uint8_t items_value_original[] = {0x8b, 0x41, 0x04, 0x8b, 0x4c, 0x24, 0x04, 0x8b};
REWRITE("Items::ValueAt7578C0", 0x7578c0, items_value_original, items_value_at, &g_w2_values);
REWRITE("Items::ValueAt7578C0", 0x7fa730, items_value_original, items_value_at, &g_w2_values);

// ---- Slots::Reset: sets the flag at +0 and zeroes the array {+4 data, +8 count} -----------------------------------------------------
static volatile long g_w2_resets;
template <uint8_t FLAG> static void __fastcall slots_reset(uint8_t *self) {
    g_w2_resets++;
    self[0] = FLAG;
    for (int32_t i = 0; i < *(int32_t *)(self + 8); i++) (*(uint32_t **)(self + 4))[i] = 0;
}
static const uint8_t slots_reset_original[] = {0x8b, 0x51, 0x08, 0x33, 0xc0, 0x85, 0xd2, 0xc6, 0x01};
REWRITE("Slots::Reset581240", 0x581240, slots_reset_original, (&slots_reset<1>), &g_w2_resets);
REWRITE("Slots::Reset581240", 0x581270, slots_reset_original, (&slots_reset<0>), &g_w2_resets);

// ---- Rows::Resize: throws away the rows and makes room for `count` new ones of 20 bytes ---------------------------------------------
static volatile long g_w2_row_resizes;
static uint32_t __thiscall rows_resize(void **self, uint32_t count) {
    g_w2_row_resizes++;
    if (self[0]) {
        memory_free_aligned(self[0]);
        self[0] = nullptr;
    }
    if (count) self[0] = memory_alloc_aligned(count * 20);
    *(uint32_t *)&self[1] = count;
    return count;
}
static const uint8_t rows_resize_original[] = {0x56, 0x57, 0x8b, 0xf9, 0x8b, 0x07, 0x85, 0xc0};
REWRITE("Rows::Resize45E8C0", 0x45e8c0, rows_resize_original, rows_resize, &g_w2_row_resizes);
REWRITE("Rows::Resize45E8C0", 0xa6bbc0, rows_resize_original, rows_resize, &g_w2_row_resizes);

// ---- Block::Delete: a deleting destructor that frees the block it holds at +0 -------------------------------------------------------
static volatile long g_w2_block_deletes;
static void *__thiscall block_delete(void **self, uint32_t flags) {
    g_w2_block_deletes++;
    memory_free_aligned(self[0]);
    if (flags & 1) memory_free(self);
    return self;
}
static const uint8_t block_delete_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x06, 0x50, 0xe8};
REWRITE("Block::Delete72F200", 0x72f200, block_delete_original, block_delete, &g_w2_block_deletes);
REWRITE("Block::Delete72F200", 0xbb6620, block_delete_original, block_delete, &g_w2_block_deletes);
REWRITE("Block::Delete72F200", 0xc22ad0, block_delete_original, block_delete, &g_w2_block_deletes);

// ---- SharedBlock: a count at +0 and data at +0xc; the last to let go frees both --------------------------------------------------
static void shared_block_drop(uint32_t *block) {
    if (--block[0] == 0) {
        memory_free_aligned((void *)(uintptr_t)block[3]);
        memory_free(block);
    }
}
static volatile long g_w2_block_drops;
static void __stdcall shared_block_release(uint32_t *block) {
    g_w2_block_drops++;
    shared_block_drop(block);
}
static const uint8_t block_release_original[] = {0x56, 0x8b, 0x74, 0x24, 0x08, 0xff, 0x0e, 0x75};
REWRITE("SharedBlock::Release75A530", 0x75a530, block_release_original, shared_block_release, &g_w2_block_drops);
REWRITE("SharedBlock::Release75A530", 0x761420, block_release_original, shared_block_release, &g_w2_block_drops);
REWRITE("SharedBlock::Release75A530", 0xb46ed0, block_release_original, shared_block_release, &g_w2_block_drops);

// ---- SharedBlock::Delete: a deleting destructor of a holder of one, at +0 ---------------------------------------------------------
static void *__thiscall shared_block_delete(uint32_t **self, uint32_t flags) {
    g_w2_block_drops++;
    shared_block_drop(self[0]);
    if (flags & 1) memory_free(self);
    return self;
}
static const uint8_t block_holder_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x06, 0xff, 0x08, 0x57};
REWRITE("SharedBlock::Delete438F30", 0x438f30, block_holder_original, shared_block_delete, &g_w2_block_drops);
REWRITE("SharedBlock::Delete438F30", 0x496500, block_holder_original, shared_block_delete, &g_w2_block_drops);

// ---- Owner::Allowed: yes when the global flag at 0x12f552a is set; otherwise by the record flag 0x200, or the state being 12 -----
static volatile long g_w2_allowed;
static uint32_t __fastcall owner_allowed(uint8_t *self) {
    g_w2_allowed++;
    if (*(uint8_t *)0x12f552a) return 1;
    void *owner = ((void *(__fastcall *)(void *))0xbb4280)(*(void **)(self + 0x6f4));
    uint8_t *record = ((uint8_t * (__fastcall *)(void *))0xbb3e70)(owner);
    if (*(uint32_t *)(record + 0x30) & 0x200) return 1;
    return *(int32_t *)(self + 0x38c) == 12;
}
static const uint8_t owner_allowed_original[] = {0xa0, 0x2a, 0x55, 0x2f, 0x01, 0x84, 0xc0, 0x56};
REWRITE("Owner::Allowed5A1760", 0x5a1760, owner_allowed_original, owner_allowed, &g_w2_allowed);
REWRITE("Owner::Allowed5A1760", 0x5a1980, owner_allowed_original, owner_allowed, &g_w2_allowed);

// ---- Field::Is: is the word at +0x18 this value? -----------------------------------------------------------------------------------
static volatile long g_w2_field_is;
static uint8_t __thiscall field_is(uint8_t *self, uint32_t value) {
    g_w2_field_is++;
    return *(uint32_t *)(self + 0x18) == value;
}
static const uint8_t debug_frame_original[] = {0x55, 0x8b, 0xec, 0x51, 0xc7, 0x45, 0xfc, 0xcc};
REWRITE("Field::IsCC1C30", 0xcc1c30, debug_frame_original, field_is, &g_w2_field_is);
REWRITE("Field::IsCC1C30", 0xcc5350, debug_frame_original, field_is, &g_w2_field_is);

// ---- Ref::Copy, Ref::Assign: a counted pointer taken from another, the assignment letting go of the old one first ---------------------
static volatile long g_w2_refs;
static void **__thiscall ref_copy(void **self, void **from) {
    g_w2_refs++;
    *self = *from;
    ((void(__fastcall *)(void *))0xcc2110)(*self);
    return self;
}
static void **__thiscall ref_assign(void **self, void **from) {
    g_w2_refs++;
    ((void(__fastcall *)(void *))0xcc2250)(*self);
    *self = *from;
    ((void(__fastcall *)(void *))0xcc2110)(*self);
    return self;
}
REWRITE("Ref::CopyCC2010", 0xcc2010, debug_frame_original, ref_copy, &g_w2_refs);
REWRITE("Ref::CopyCC2010", 0xcc5860, debug_frame_original, ref_copy, &g_w2_refs);
REWRITE("Ref::AssignCC2050", 0xcc2050, debug_frame_original, ref_assign, &g_w2_refs);
REWRITE("Ref::AssignCC2050", 0xcc58a0, debug_frame_original, ref_assign, &g_w2_refs);

// ---- Text::Store: measures a string and passes it on to the real function with its length -----------------------------------------
static volatile long g_w2_texts;
template <uint32_t TARGET> static void *__thiscall text_store(void *self, void *into, const char *text) {
    g_w2_texts++;
    uint32_t length = 0;
    while (text[length]) length++;
    ((void(__thiscall *)(void *, void *, const char *, uint32_t))TARGET)(self, into, text, length);
    return into;
}
static const uint8_t text_store_original[] = {0x51, 0x8b, 0x54, 0x24, 0x0c, 0x53, 0x80, 0x3a};
REWRITE("Text::Store403BE0", 0x403be0, text_store_original, (&text_store<0xbd0720>), &g_w2_texts);
REWRITE("Text::Store403BE0", 0x403e10, text_store_original, (&text_store<0xbd0ce0>), &g_w2_texts);

// ---- Entries::Visit: passes each active 12-byte entry of the array at +0x54 (the count at +0x60) to the real function ---------------
static volatile long g_w2_visits;
template <uint32_t TARGET> static void __fastcall entries_visit(uint8_t *self) {
    g_w2_visits++;
    for (int32_t i = 0; i < *(int32_t *)(self + 0x60); i++) {
        uint8_t *entry = *(uint8_t **)(self + 0x54) + i * 12;
        if (entry[0]) ((void(__fastcall *)(void *))TARGET)(*(void **)(entry + 4));
    }
}
static const uint8_t entries_visit_original[] = {0x53, 0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x60, 0x33};
REWRITE("Entries::VisitB6C360", 0xb6c360, entries_visit_original, (&entries_visit<0xb39260>), &g_w2_visits);
REWRITE("Entries::VisitB6C360", 0xb6cf00, entries_visit_original, (&entries_visit<0xb36820>), &g_w2_visits);

// ---- Writer::Put: writes a value at the place at +0xc through virtual slot SLOT, then moves on a word; the new place ----------------
static volatile long g_w2_puts;
template <uint32_t SLOT> static uint32_t __thiscall writer_put(uint32_t *self, uint32_t value) {
    g_w2_puts++;
    ((void(__thiscall *)(void *, uint32_t, uint32_t))(*(void ***)self)[SLOT])(self, self[3], value);
    return self[3] += 4;
}
static const uint8_t writer_put_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x4c, 0x24, 0x08, 0x8b};
REWRITE("Writer::PutCA55F0", 0xca55f0, writer_put_original, (&writer_put<3>), &g_w2_puts);
REWRITE("Writer::PutCA55F0", 0xca5610, writer_put_original, (&writer_put<5>), &g_w2_puts);
REWRITE("Writer::PutCA55F0", 0xca5670, writer_put_original, (&writer_put<7>), &g_w2_puts);

// ---- Typed::Forward: passes a value on to the real function with a type marker of this copy ----------------------------------------
static volatile long g_w2_typed;
template <uint32_t TABLE> static uint32_t __thiscall typed_forward(void *self, uint32_t value) {
    g_w2_typed++;
    uint32_t marker = TABLE;
    return ((uint32_t(__thiscall *)(void *, uint32_t *, uint32_t))0x6aed50)(self, &marker, value);
}
static const uint8_t typed_forward_original[] = {0x51, 0x8b, 0x44, 0x24, 0x08, 0x50, 0x8d, 0x54};
REWRITE("Typed::Forward6AF080", 0x6af080, typed_forward_original, (&typed_forward<0xf60660>), &g_w2_typed);
REWRITE("Typed::Forward6AF080", 0x6af0a0, typed_forward_original, (&typed_forward<0xf60670>), &g_w2_typed);
REWRITE("Typed::Forward6AF080", 0x6af0c0, typed_forward_original, (&typed_forward<0xf606a0>), &g_w2_typed);
REWRITE("Typed::Forward6AF080", 0x6af0e0, typed_forward_original, (&typed_forward<0xf606b0>), &g_w2_typed);
REWRITE("Typed::Forward6AF080", 0x6af100, typed_forward_original, (&typed_forward<0xf606d0>), &g_w2_typed);

// ---- Event::Ask: sends an event with an answer word, starting at 0, and gives back what the handler left there ---------------------
static volatile long g_w2_asks;
template <uint32_t TABLE> static uint32_t __fastcall event_ask(void **self) {
    g_w2_asks++;
    uint32_t event[2] = {TABLE, 0};
    ((void(__thiscall *)(void *, uint32_t *))(*(void ***)self)[0x20])(self, event);
    return event[1];
}
static const uint8_t event_ask_original[] = {0x83, 0xec, 0x08, 0x8b, 0x01, 0x8d, 0x14, 0x24};
REWRITE("Event::Ask845AB0", 0x845ab0, event_ask_original, (&event_ask<0xf77bdc>), &g_w2_asks);
REWRITE("Event::Ask845AB0", 0x845bb0, event_ask_original, (&event_ask<0xf77be4>), &g_w2_asks);
REWRITE("Event::Ask845AB0", 0x845dd0, event_ask_original, (&event_ask<0xf77bf4>), &g_w2_asks);

// ---- Event::AskFlag: the same with a one-byte answer that starts at ANSWER ----------------------------------------------------------
template <uint32_t TABLE, uint8_t ANSWER> static uint8_t __fastcall event_ask_flag(void **self) {
    g_w2_asks++;
    struct {
        uint32_t table;
        uint8_t answer;
    } event = {TABLE, ANSWER};
    ((void(__thiscall *)(void *, void *))(*(void ***)self)[0x20])(self, &event);
    return event.answer;
}
REWRITE("Event::AskFlag845A50", 0x845a50, event_ask_original, (&event_ask_flag<0xf77bd8, 0>), &g_w2_asks);
REWRITE("Event::AskFlag845A50", 0x845d20, event_ask_original, (&event_ask_flag<0xf77bec, 1>), &g_w2_asks);
REWRITE("Event::AskFlag845A50", 0x845d50, event_ask_original, (&event_ask_flag<0xf77bf0, 1>), &g_w2_asks);

// ---- Level::Field: a field that only means something from level 2 on, 0 before ------------------------------------------------------
static volatile long g_w2_level_fields;
template <uint32_t OFF> static uint32_t __fastcall level_field(uint8_t *self) {
    g_w2_level_fields++;
    void *owner = ((void *(__fastcall *)(void *))0xbb5130)(self);
    uint8_t *record = ((uint8_t * (__fastcall *)(void *))0xbb3e70)(owner);
    return record[1] >= 2 ? *(uint32_t *)(self + OFF) : 0;
}
// the bytes past the third are the call offset, which differs by copy
template <uint32_t AT> static const uint8_t level_field_original[] = {
    0x56, 0x8b, 0xf1, 0xe8, (uint8_t)(0xbb5130 - (AT + 8)), (uint8_t)((0xbb5130 - (AT + 8)) >> 8),
    (uint8_t)((0xbb5130 - (AT + 8)) >> 16), (uint8_t)((0xbb5130 - (AT + 8)) >> 24)};
REWRITE("Level::Field4A15F0", 0x4a15f0, level_field_original<0x4a15f0>, (&level_field<0x24>), &g_w2_level_fields);
REWRITE("Level::Field4A15F0", 0x4a9070, level_field_original<0x4a9070>, (&level_field<0x1c>), &g_w2_level_fields);

// ---- Holder::Delete: a deleting destructor that tells the object at +4 to go, through its slot 2 --------------------------------------
static volatile long g_w2_holder_deletes;
template <uint32_t TABLE> static void *__thiscall holder_delete(uint32_t *self, uint32_t flags) {
    g_w2_holder_deletes++;
    self[0] = TABLE;
    void **object = (void **)(uintptr_t)self[1];
    ((void(__stdcall *)(void *))(*(void ***)object)[2])(object);
    if (flags & 1) memory_free(self);
    return self;
}
static const uint8_t holder_delete_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x04, 0xc7, 0x06};
REWRITE("Holder::DeleteBEA240", 0xbea240, holder_delete_original, (&holder_delete<0xfa5730>), &g_w2_holder_deletes);
REWRITE("Holder::DeleteBEA240", 0xbfde60, holder_delete_original, (&holder_delete<0xfa5ca4>), &g_w2_holder_deletes);

// ---- Counted::Drop: lets go of a counted object (the count at +4); the last one deletes it -------------------------------------------
static volatile long g_w2_counted;
static void __fastcall counted_drop(uint32_t **self) {
    g_w2_counted++;
    uint32_t *object = *self;
    if (--object[1] == 0) ((void(__thiscall *)(void *, uint32_t))(*(void ***)object)[0])(object, 1);
}
static const uint8_t counted_drop_original[] = {0x8b, 0x01, 0xff, 0x48, 0x04, 0x8b, 0x09, 0x8b};
REWRITE("Counted::DropC321E0", 0xc321e0, counted_drop_original, counted_drop, &g_w2_counted);
REWRITE("Counted::DropC321E0", 0xc425c0, counted_drop_original, counted_drop, &g_w2_counted);

// ---- Items::Pass: passes two values on to the real function of the index'th item of the array at +0x54 -----------------------------
static volatile long g_w2_passes;
template <uint32_t TARGET> static uint32_t __thiscall items_pass(uint8_t *self, int32_t index, uint32_t a, uint32_t b) {
    g_w2_passes++;
    return ((uint32_t(__thiscall *)(void *, uint32_t, uint32_t))TARGET)((*(void ***)(self + 0x54))[index], a, b);
}
static const uint8_t items_pass_original[] = {0x8b, 0x44, 0x24, 0x0c, 0x8b, 0x54, 0x24, 0x08};
REWRITE("Items::Pass8387C0", 0x8387c0, items_pass_original, (&items_pass<0xce23a0>), &g_w2_passes);
REWRITE("Items::Pass8387C0", 0x8387e0, items_pass_original, (&items_pass<0xce23e0>), &g_w2_passes);

// ---- Word::MakeDefault: the word is what the real function gives for 1 ---------------------------------------------------------------
static volatile long g_w2_defaults;
static uint32_t *__fastcall word_make_default(uint32_t *self) {
    g_w2_defaults++;
    *self = ((uint32_t(__fastcall *)(uint8_t))0xbd0490)(1);
    return self;
}
static const uint8_t word_default_original[] = {0x51, 0x56, 0x8b, 0xf1, 0xb1, 0x01, 0xc7, 0x44};
REWRITE("Word::MakeDefault49CD50", 0x49cd50, word_default_original, word_make_default, &g_w2_defaults);
REWRITE("Word::MakeDefault49CD50", 0x49cd70, word_default_original, word_make_default, &g_w2_defaults);

// ---- List::RemoveAll: frees every node holding `value` from the list held at +OFF (an end node stops the walk) ------------------------
static volatile long g_w2_remove_all;
template <uint32_t OFF> static void __thiscall list_remove_all(uint8_t *self, void *value) {
    g_w2_remove_all++;
    Node *n = *(Node **)(self + OFF);
    if (!n->next) return;
    while (n) {
        Node *next = n->next->next ? n->next : nullptr;
        if (n->value == value) {
            unlink(n);
            memory_free(n);
        }
        n = next;
    }
}
template <uint32_t OFF> static const uint8_t list_remove_all_original[] = {
    0x56, 0x8b, 0xb1, (uint8_t)OFF, (uint8_t)(OFF >> 8), (uint8_t)(OFF >> 16), (uint8_t)(OFF >> 24), 0x83, 0x3e, 0x00};
REWRITE("List::RemoveAll59AB70", 0x59ab70, list_remove_all_original<0xb8>, (&list_remove_all<0xb8>), &g_w2_remove_all);
REWRITE("List::RemoveAll59AB70", 0x5c20e0, list_remove_all_original<0x338>, (&list_remove_all<0x338>), &g_w2_remove_all);

// ---- Rows::Total: tells the manager about itself, then adds up a field of each 24-byte row to the base value --------------------------
static volatile long g_w2_totals;
template <uint32_t FIELD, uint32_t BASE> static int32_t __fastcall rows_total(uint8_t *self) {
    g_w2_totals++;
    ((void(__thiscall *)(void *, void *))0xb329f0)(*(void **)0x13ce934, self);
    int32_t total = 0, count = *(int32_t *)(self + 0x70);
    uint8_t *row = *(uint8_t **)(self + 0x6c);
    for (int32_t i = 0; i < count; i++) total += *(int32_t *)(row + i * 0x18 + FIELD);
    return *(int32_t *)(self + BASE) + total;
}
static const uint8_t rows_total_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x0d, 0x34, 0xe9, 0x3c, 0x01};
REWRITE("Rows::Total6BC850", 0x6bc850, rows_total_original, (&rows_total<0x14, 0x68>), &g_w2_totals);
REWRITE("Rows::Total6BC850", 0x6bc880, rows_total_original, (&rows_total<0xc, 0x60>), &g_w2_totals);
REWRITE("Rows::Total6BC850", 0x6bc8f0, rows_total_original, (&rows_total<0x10, 0x64>), &g_w2_totals);

// ---- Interface::Member: the address of a member at +OFF of the interface of the handle at +0x94 ----------------------------------------
static volatile long g_w2_members;
// (added as a number: with no interface the result is the offset alone)
template <uint32_t OFF> static uint32_t __fastcall interface_member(uint8_t *self) {
    g_w2_members++;
    return (uint32_t)(uintptr_t)find_interface(*(uint8_t **)(self + 0x94), (const int32_t *)0x12f51b4) + OFF;
}
static const uint8_t interface_member_original[] = {0x8b, 0x89, 0x94, 0x00, 0x00, 0x00, 0x56, 0x8b};
REWRITE("Interface::Member736010", 0x736010, interface_member_original, (&interface_member<0x1b4>), &g_w2_members);
REWRITE("Interface::Member736010", 0x736060, interface_member_original, (&interface_member<0x1c0>), &g_w2_members);

// ---- Lookup::Then: looks an object up from two values and runs the real function on it ---------------------------------------------
static volatile long g_w2_lookups;
template <uint32_t TARGET> static uint32_t __thiscall lookup_then(void *self, uint32_t a, uint32_t b) {
    g_w2_lookups++;
    void *found = ((void *(__stdcall *)(void *, uint32_t, uint32_t))0x44cc40)(self, a, b);
    return ((uint32_t(__fastcall *)(void *))TARGET)(found);
}
static const uint8_t lookup_then_original[] = {0x8b, 0x44, 0x24, 0x08, 0x8b, 0x54, 0x24, 0x04};
REWRITE("Lookup::Then48CA20", 0x48ca20, lookup_then_original, (&lookup_then<0x48a4b0>), &g_w2_lookups);
REWRITE("Lookup::Then48CA20", 0x48caf0, lookup_then_original, (&lookup_then<0x489a10>), &g_w2_lookups);

// ---- Member::Read: the real function fills `out` from the member at +OFF; the result is `out` --------------------------------------
static volatile long g_w2_reads;
template <uint32_t OFF> static void *__thiscall member_read(uint8_t *self, void *out) {
    g_w2_reads++;
    ((void(__thiscall *)(void *, void *, void *))0x9ea680)(self, out, self + OFF);
    return out;
}
static const uint8_t member_read_original[] = {0x51, 0x56, 0x8b, 0x74, 0x24, 0x0c, 0x8d, 0x41};
REWRITE("Member::Read9EAF10", 0x9eaf10, member_read_original, (&member_read<0x5c>), &g_w2_reads);
REWRITE("Member::Read9EAF10", 0x9eaf30, member_read_original, (&member_read<0x70>), &g_w2_reads);

// ---- Member::Into: as Member::Read, with `out` itself the object of the real function -----------------------------------------------
template <uint32_t OFF, uint32_t TARGET> static void *__thiscall member_into(uint8_t *self, void *out) {
    g_w2_reads++;
    ((void(__thiscall *)(void *, void *))TARGET)(out, self + OFF);
    return out;
}
static const uint8_t member_into_original[] = {0x51, 0x56, 0x8b, 0x74, 0x24, 0x0c, 0x83, 0xc1};
REWRITE("Member::Into495BF0", 0x495bf0, member_into_original, (&member_into<0x5c, 0xbd9c10>), &g_w2_reads);
REWRITE("Member::Into495BF0", 0x8fc950, member_into_original, (&member_into<0x1c, 0xbdbe10>), &g_w2_reads);
REWRITE("Member::Into495BF0", 0x9ffb70, member_into_original, (&member_into<0x1c, 0xbdbe10>), &g_w2_reads);

// ---- Base::Delete: a deleting destructor that sets the table and leaves the rest to the real destructor ------------------------------
static volatile long g_w2_base_deletes;
template <uint32_t TABLE, uint32_t DESTROY> static void *__thiscall base_delete(uint32_t *self, uint32_t flags) {
    g_w2_base_deletes++;
    self[0] = TABLE;
    ((void(__fastcall *)(void *))DESTROY)(self);
    if (flags & 1) memory_free(self);
    return self;
}
static const uint8_t base_delete_original[] = {0x56, 0x8b, 0xf1, 0xc7, 0x06};
REWRITE("Base::Delete4029C0", 0x4029c0, base_delete_original, (&base_delete<0xf264b0, 0xbd36c0>), &g_w2_base_deletes);
REWRITE("Base::Delete4029C0", 0x6b26e0, base_delete_original, (&base_delete<0xf60c64, 0xbd3170>), &g_w2_base_deletes);

// ---- Member::Update: runs its own update, then passes the value to the object held at +OFF --------------------------------------------
static volatile long g_w2_updates;
template <uint32_t UPDATE, uint32_t OFF> static uint32_t __thiscall member_update(uint8_t *self, uint32_t value) {
    g_w2_updates++;
    ((void(__fastcall *)(void *))UPDATE)(self);
    return ((uint32_t(__thiscall *)(void *, uint32_t))0x6a0040)(*(void **)(self + OFF), value);
}
// the bytes past the third are the call offset, which differs by copy
template <uint32_t AT, uint32_t UPDATE> static const uint8_t member_update_original[] = {
    0x56, 0x8b, 0xf1, 0xe8, (uint8_t)(UPDATE - (AT + 8)), (uint8_t)((UPDATE - (AT + 8)) >> 8),
    (uint8_t)((UPDATE - (AT + 8)) >> 16), (uint8_t)((UPDATE - (AT + 8)) >> 24)};
REWRITE("Member::Update6C6DB0", 0x6c6db0, (member_update_original<0x6c6db0, 0x6c6c10>), (&member_update<0x6c6c10, 0x11c>), &g_w2_updates);
REWRITE("Member::Update6C6DB0", 0x6c9d30, (member_update_original<0x6c9d30, 0x6c9b80>), (&member_update<0x6c9b80, 0x1a8>), &g_w2_updates);
REWRITE("Member::Update6C6DB0", 0x6ebd20, (member_update_original<0x6ebd20, 0x6eb470>), (&member_update<0x6eb470, 0x158>), &g_w2_updates);

// ---- Refs::TellFlagged: each reference on a list whose interface has flag 1 at +0x40 is told the value through the real function -------
static volatile long g_w2_flagged;
template <uint32_t ID, uint32_t TELL> static void __stdcall refs_tell_flagged(Node **list, uint32_t value) {
    g_w2_flagged++;
    Node *n = *list;
    if (!n->next) return;
    for (;;) {
        uint8_t *interface = (uint8_t *)ref_interface<ID>((uint8_t *)n->value);
        if (interface && (interface[0x40] & 1)) ((void(__thiscall *)(void *, uint32_t))TELL)(interface, value);
        if (!n->next->next) return;
        n = n->next;
    }
}
static const uint8_t refs_flagged_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0x00, 0x83, 0x38};
REWRITE("Refs::TellFlagged54B020", 0x54b020, refs_flagged_original, (&refs_tell_flagged<0x12f52bc, 0x548cf0>), &g_w2_flagged);
REWRITE("Refs::TellFlagged54B020", 0x54b0b0, refs_flagged_original, (&refs_tell_flagged<0x12f5284, 0x549c50>), &g_w2_flagged);
REWRITE("Refs::TellFlagged54B020", 0x550360, refs_flagged_original, (&refs_tell_flagged<0x12f5264, 0x54e720>), &g_w2_flagged);

// ---- Log::Line: writes the prefix of this level ("Trace: ", "Warning: ", "Error: "), the text and a line end to the log held at +0 -----
static volatile long g_w2_log_lines;
static void log_write(void *log, const char *text) {
    uint32_t length = 0;
    while (text[length]) length++;
    ((void(__thiscall *)(void *, const char *, uint32_t))0xbe4060)(log, text, length);
}
template <uint32_t PREFIX> static void __thiscall log_line(void **self, const char *text) {
    g_w2_log_lines++;
    if (!*self || !text) return;
    log_write(*self, (const char *)PREFIX);
    log_write(*self, text);
    log_write(*self, (const char *)0xf27cb4);
}
static const uint8_t log_line_original[] = {0x53, 0x8b, 0xd9, 0x8b, 0x0b, 0x85, 0xc9, 0x74};
REWRITE("Log::LineA3E860", 0xa3e860, log_line_original, (&log_line<0xf8b5b0>), &g_w2_log_lines);
REWRITE("Log::LineA3E860", 0xa3e8e0, log_line_original, (&log_line<0xf8b5b8>), &g_w2_log_lines);
REWRITE("Log::LineA3E860", 0xa3e960, log_line_original, (&log_line<0xf8b5c4>), &g_w2_log_lines);

// ---- Chunks::Take: the next 20-byte slot of a chain of 64-slot chunks {slots, +0x500 used, +0x504 next}; {+0 first, +4 current} --------
static volatile long g_w2_chunk_takes;
template <uint32_t CONSTRUCT> static uint8_t *new_chunk() {
    void *block = memory_alloc(0x508);
    return block ? ((uint8_t * (__thiscall *)(void *, uint32_t))CONSTRUCT)(block, 0) : nullptr;
}
template <uint32_t CONSTRUCT> static uint8_t *__fastcall chunks_take(uint8_t **self) {
    g_w2_chunk_takes++;
    if (!self[0]) self[0] = self[1] = new_chunk<CONSTRUCT>();
    uint8_t *chunk = self[1];
    int32_t &used = *(int32_t *)(chunk + 0x500);
    if (used < 0x40) return chunk + used++ * 20;
    uint8_t *&next = *(uint8_t **)(chunk + 0x504);
    if (!next) next = new_chunk<CONSTRUCT>();
    self[1] = next;
    *(int32_t *)(next + 0x500) = 1;
    return next;
}
static const uint8_t chunks_take_original[] = {0x56, 0x8b, 0xf1, 0x83, 0x3e, 0x00, 0x75, 0x23};
REWRITE("Chunks::Take42C7D0", 0x42c7d0, chunks_take_original, (&chunks_take<0x42c270>), &g_w2_chunk_takes);
REWRITE("Chunks::Take42C7D0", 0x42c870, chunks_take_original, (&chunks_take<0x42c2b0>), &g_w2_chunk_takes);

// ---- Order::MoveBefore, Order::MoveAfter: moves an item next to another on the order list (its node at +0x50), first moving the later
// items it depends on (the edges at +0x20) that this pass (the mark at +0x18) has not reached. MoveBefore fails on reaching the target ----
static volatile long g_w2_order_moves;
template <uint32_t PASS, uint32_t FIRST> static uint8_t __thiscall order_move_before(uint8_t *self, uint8_t *item, uint8_t *target);
template <uint32_t PASS, uint32_t FIRST> static bool order_edges(uint8_t *self, uint8_t *item, uint8_t *target, bool stop_at_target) {
    Node *n = *(Node **)(item + 0x20);
    if (!n->next) return true;
    for (;;) {
        uint8_t *other = (uint8_t *)n->value;
        if (stop_at_target && other == target) return false;
        int32_t pass = *(int32_t *)(self + PASS);
        if (*(int32_t *)(other + 0x1c) > *(int32_t *)(target + 0x1c) && *(int32_t *)(other + 0x18) < pass) {
            *(int32_t *)(other + 0x18) = pass;
            if (!order_move_before<PASS, FIRST>(self, (uint8_t *)n->value, target)) return false;
        }
        n = n->next;
        if (!n->next) return true;
    }
}
template <uint32_t PASS, uint32_t FIRST> static uint8_t __thiscall order_move_before(uint8_t *self, uint8_t *item, uint8_t *target) {
    g_w2_order_moves++;
    if (!order_edges<PASS, FIRST>(self, item, target, true)) return 0;
    Node *n = (Node *)(item + 0x50), *place = (Node *)(target + 0x50);
    unlink(n);
    n->prev = place->prev;
    place->prev->next = n;
    n->next = place;
    place->prev = n;
    if (!*(void **)(self + FIRST)) *(void **)(self + FIRST) = item;
    return 1;
}
template <uint32_t PASS, uint32_t FIRST> static uint8_t __thiscall order_move_after(uint8_t *self, uint8_t *item, uint8_t *target) {
    g_w2_order_moves++;
    if (!order_edges<PASS, FIRST>(self, item, target, false)) return 0;
    Node *n = (Node *)(item + 0x50), *place = (Node *)(target + 0x50);
    unlink(n);
    n->next = place->next;
    place->next->prev = n;
    n->prev = place;
    place->next = n;
    if (!*(void **)(self + FIRST)) *(void **)(self + FIRST) = item;
    return 1;
}
static const uint8_t order_move_original[] = {0x53, 0x8b, 0x5c, 0x24, 0x0c, 0x55, 0x8b, 0x6c};
REWRITE("Order::MoveBefore42C490", 0x42c490, order_move_original, (&order_move_before<0x18, 0x1c>), &g_w2_order_moves);
REWRITE("Order::MoveBefore42C490", 0x965a50, order_move_original, (&order_move_before<0x1c, 0x20>), &g_w2_order_moves);
REWRITE("Order::MoveAfter42C550", 0x42c550, order_move_original, (&order_move_after<0x18, 0x1c>), &g_w2_order_moves);
REWRITE("Order::MoveAfter42C550", 0x965b10, order_move_original, (&order_move_after<0x1c, 0x20>), &g_w2_order_moves);

// ---- List::VisitAt: runs the real function on the value of each node of the list held at +OFF ---------------------------------------
static volatile long g_w2_visit_at;
template <uint32_t OFF, uint32_t TARGET> static void __fastcall list_visit_at(uint8_t *self) {
    g_w2_visit_at++;
    Node *n = *(Node **)(self + OFF);
    if (!n->next) return;
    for (;;) {
        ((void(__fastcall *)(void *))TARGET)(n->value);
        n = n->next;
        if (!n->next) return;
    }
}
template <uint8_t OFF> static const uint8_t list_visit_at_original[] = {0x56, 0x8b, 0x71, OFF, 0x83, 0x3e, 0x00, 0x74, 0x1a};
REWRITE("List::VisitAt6A1E20", 0x6a1e20, list_visit_at_original<0x4>, (&list_visit_at<0x4, 0x6a1e70>), &g_w2_visit_at);
REWRITE("List::VisitAt6A1E20", 0x936f90, list_visit_at_original<0x5c>, (&list_visit_at<0x5c, 0x935a30>), &g_w2_visit_at);

// ---- Owner::Delete: a deleting destructor that deletes the object it owns at +4, if any ----------------------------------------------
static volatile long g_w2_owner_deletes;
template <uint32_t TABLE> static void *__thiscall owner_delete(uint32_t *self, uint32_t flags) {
    g_w2_owner_deletes++;
    void **owned = (void **)(uintptr_t)self[1];
    self[0] = TABLE;
    if (owned) ((void(__thiscall *)(void *, uint32_t))(*(void ***)owned)[0])(owned, 1);
    if (flags & 1) memory_free(self);
    return self;
}
static const uint8_t owner_delete_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x4e, 0x04, 0x85, 0xc9};
REWRITE("Owner::Delete99AF20", 0x99af20, owner_delete_original, (&owner_delete<0xf80c2c>), &g_w2_owner_deletes);
REWRITE("Owner::Delete99AF20", 0xca2230, owner_delete_original, (&owner_delete<0xfbf2b8>), &g_w2_owner_deletes);

// ---- Ref::Field: a field of the interface of the reference held at +REF, which must be there ----------------------------------------
static volatile long g_w2_ref_fields;
template <uint32_t REF, uint32_t ID, uint32_t FIELD> static uint32_t __fastcall ref_field(uint8_t *self) {
    g_w2_ref_fields++;
    return *(uint32_t *)((uint8_t *)ref_interface<ID>(self + REF) + FIELD);
}
static const uint8_t ref_field_original[] = {0x57, 0x8b, 0xf9, 0x8a, 0x87};
REWRITE("Ref::Field532EA0", 0x532ea0, ref_field_original, (&ref_field<0x204, 0x12f5338, 0x44>), &g_w2_ref_fields);
REWRITE("Ref::Field532EA0", 0xab1570, ref_field_original, (&ref_field<0x198, 0x13ce028, 0x64>), &g_w2_ref_fields);
REWRITE("Ref::Field532EA0", 0xab15e0, ref_field_original, (&ref_field<0x198, 0x13ce028, 0x68>), &g_w2_ref_fields);
REWRITE("Ref::Field532EA0", 0xb88850, ref_field_original, (&ref_field<0xe0, 0x13d0188, 0x44>), &g_w2_ref_fields);

// ---- Message::Post: builds a message of SIZE bytes on the stack, posts it to the queue at +OFF, then flushes the sender's pending bytes
// {+4 data, +0x104 count} through its own virtual slot 0 ---------------------------------------------------------------------------
static volatile long g_w2_posts;
template <uint32_t SIZE, uint32_t OFF, uint32_t BUILD, uint32_t POST>
static void __thiscall message_post(uint8_t *self, uint32_t a, uint32_t b, uint8_t *sender) {
    g_w2_posts++;
    alignas(4) uint8_t message[SIZE];
    void *queue = *(void **)(self + OFF);
    ((void(__thiscall *)(void *, uint32_t, uint32_t, void *))BUILD)(message, a, b, sender);
    ((void(__thiscall *)(void *, void *))POST)(queue, message);
    uint32_t &pending = *(uint32_t *)(sender + 0x104);
    if (pending) {
        ((void(__thiscall *)(void *, void *, uint32_t))(*(void ***)sender)[0])(sender, sender + 4, pending);
        pending = 0;
    }
}
static const uint8_t message_post_original[] = {0x8b, 0x44, 0x24, 0x08, 0x83, 0xec};
REWRITE("Message::Post7BB470", 0x7bb470, message_post_original, (&message_post<0x2c, 0x4c, 0x7b2e30, 0x7b9300>), &g_w2_posts);
REWRITE("Message::Post7BB470", 0x7bb4d0, message_post_original, (&message_post<0x1c, 0x4c, 0x7b2ec0, 0x7b93b0>), &g_w2_posts);
REWRITE("Message::Post7BB470", 0xb75820, message_post_original, (&message_post<0x2c, 0x64, 0xb6bb70, 0xb74dd0>), &g_w2_posts);
REWRITE("Message::Post7BB470", 0xb75880, message_post_original, (&message_post<0x1c, 0x64, 0xb6bc00, 0xb74e80>), &g_w2_posts);

// ---- SharedBlock::AssignPair: takes on another pair of shared blocks, letting go of the old ones --------------------------------------
static volatile long g_w2_block_assigns;
static void shared_block_take(uint32_t **slot, uint32_t **from) {
    if (*from == *slot) return;
    shared_block_drop(*slot);
    *slot = *from;
    ++**slot;
}
static uint32_t **__thiscall shared_block_assign_pair(uint32_t **self, uint32_t **other) {
    g_w2_block_assigns++;
    shared_block_take(&self[0], &other[0]);
    shared_block_take(&self[1], &other[1]);
    return self;
}
static const uint8_t block_assign_original[] = {0x53, 0x8b, 0x5c, 0x24, 0x08, 0x56, 0x8b, 0xf1};
REWRITE("SharedBlock::AssignPair4A1670", 0x4a1670, block_assign_original, shared_block_assign_pair, &g_w2_block_assigns);
REWRITE("SharedBlock::AssignPair4A1670", 0xb927b0, block_assign_original, shared_block_assign_pair, &g_w2_block_assigns);

// ---- State::Step: checks with the target if there is one, then in state 2 calls its virtual slot 0x15 with the value, and in state 5
// moves to state 6 and hands the target's data at +0x140 on through the member at +0xe0 ---------------------------------------------
static volatile long g_w2_steps;
static void __thiscall state_step(uint8_t *self, uint32_t value) {
    g_w2_steps++;
    uint8_t *target = ((uint8_t * (__fastcall *)(void *))0x4d18c0)(self);
    if (((uint32_t(__fastcall *)(void *))0x4d1840)(self) &&
        !((uint8_t(__thiscall *)(void *, uint32_t, void *))0x4d2800)(self, 0, target))
        return;
    int32_t &state = *(int32_t *)(self + 0xb0);
    if (state == 2) {
        ((void(__thiscall *)(void *, uint32_t))(*(void ***)self)[0x15])(self, value);
    } else if (state == 5) {
        state = 6;
        uint8_t *member = self + 0xe0;
        void *data = target ? target + 0x140 : nullptr;
        void *result = ((void *(__thiscall *)(void *, void *))(*(void ***)member)[4])(member, data);
        ((void(__fastcall *)(void *))0x7a1840)(result);
    }
}
static const uint8_t state_step_original[] = {0x56, 0x57, 0x8b, 0xf1, 0xe8};
REWRITE("State::Step4E5FB0", 0x4e5fb0, state_step_original, state_step, &g_w2_steps);
REWRITE("State::Step4E5FB0", 0x5eb0e0, state_step_original, state_step, &g_w2_steps);

// ---- Kind::Lookup: a value from a small table, 0 past its end -----------------------------------------------------------------------
static volatile long g_w2_lookup_kinds;
static const uint32_t kind_values_a[4] = {0, 1, 2, 3};
static const uint32_t kind_values_b[4] = {0, 2, 5, 7};
static const uint32_t kind_values_c[5] = {0, 1, 0xa, 6, 9};
static const uint32_t kind_values_d[4] = {0xf37928, 0xf37920, 0xf37918, 0xf37910};
template <const uint32_t *VALUES, uint32_t COUNT> static uint32_t __fastcall kind_lookup(uint32_t kind) {
    g_w2_lookup_kinds++;
    return kind < COUNT ? VALUES[kind] : 0;
}
static const uint8_t kind_lookup3_original[] = {0x83, 0xf9, 0x03, 0x77, 0x19, 0xff, 0x24, 0x8d};
static const uint8_t kind_lookup3b_original[] = {0x83, 0xf9, 0x03, 0x77, 0x1f, 0xff, 0x24, 0x8d};
static const uint8_t kind_lookup4_original[] = {0x83, 0xf9, 0x04, 0x77, 0x1f, 0xff, 0x24, 0x8d};
REWRITE("Kind::Lookup59A2D0", 0x59a2d0, kind_lookup3_original, (&kind_lookup<kind_values_a, 4>), &g_w2_lookup_kinds);
REWRITE("Kind::Lookup59A2D0", 0x5bd920, kind_lookup3_original, (&kind_lookup<kind_values_b, 4>), &g_w2_lookup_kinds);
REWRITE("Kind::Lookup5BD960", 0x5bd960, kind_lookup4_original, (&kind_lookup<kind_values_c, 5>), &g_w2_lookup_kinds);
REWRITE("Kind::Lookup5BD960", 0xbbf9f0, kind_lookup3b_original, (&kind_lookup<kind_values_d, 4>), &g_w2_lookup_kinds);

// ---- Timer::Run: without a mode at +MODE runs the plain update of the member at +MEMBER; with one, first starts the timer at +TIMER at
// 20 if it is below 0, copying it to +SHOWN of the object at +0xec, then runs the timed update --------------------------------------
static volatile long g_w2_timers;
template <uint32_t MODE, uint32_t TIMER, uint32_t SHOWN, uint32_t MEMBER> static uint32_t __fastcall timer_run(uint8_t *self) {
    g_w2_timers++;
    void *member = *(void **)(self + MEMBER);
    if (!*(uint32_t *)(self + MODE)) return ((uint32_t(__fastcall *)(void *))0x6b0990)(member);
    if (*(int32_t *)(self + TIMER) < 0) {
        *(int32_t *)(self + TIMER) = 20;
        *(int32_t *)(*(uint8_t **)(self + 0xec) + SHOWN) = 20;
    }
    return ((uint32_t(__fastcall *)(void *))0x6afc20)(member);
}
template <uint32_t MODE> static const uint8_t timer_run_original[] = {
    0x8b, 0x81, (uint8_t)MODE, (uint8_t)(MODE >> 8), 0x00, 0x00, 0x85, 0xc0, 0x74, 0x2c};
REWRITE("Timer::Run543B40", 0x543b40, timer_run_original<0x17c>, (&timer_run<0x17c, 0x188, 0x288, 0x184>), &g_w2_timers);
REWRITE("Timer::Run543B40", 0x543b90, timer_run_original<0x190>, (&timer_run<0x190, 0x19c, 0x28c, 0x198>), &g_w2_timers);

// ---- Scanner::Flush: empties a scanner buffer {+0 file, +4 start, +8 position, +0x10 count, +0x1c at line start, +0x24 status}; the current
// buffer also reloads the scanner state from it -----------------------------------------------------------------------------------
static volatile long g_w2_flushes;
template <uint32_t CURRENT, uint32_t COUNT, uint32_t POSITION, uint32_t FILE_, uint32_t TEXT, uint32_t HELD>
static void __fastcall scanner_flush(uint8_t *self) {
    g_w2_flushes++;
    if (!self) return;
    uint8_t *start = *(uint8_t **)(self + 4);
    *(uint32_t *)(self + 0x10) = 0;
    start[0] = 0;
    start[1] = 0;
    *(uint32_t *)(self + 0x24) = 0;
    *(uint8_t **)(self + 8) = start;
    *(uint32_t *)(self + 0x1c) = 1;
    if (self != *(uint8_t **)CURRENT) return;
    uint8_t *position = *(uint8_t **)(self + 8);
    *(uint32_t *)COUNT = *(uint32_t *)(self + 0x10);
    *(uint8_t **)POSITION = position;
    *(uint32_t *)FILE_ = *(uint32_t *)self;
    *(uint8_t **)TEXT = position;
    *(uint8_t *)HELD = *position;
}
static const uint8_t scanner_flush_original[] = {0x33, 0xc0, 0x3b, 0xc8, 0x74, 0x4e, 0x8b, 0x51};
REWRITE("Scanner::FlushC09740", 0xc09740, scanner_flush_original, (&scanner_flush<0x13e32f8, 0x13e32e8, 0x13e32e4, 0x13e3304, 0x13e32fc, 0x13d32c0>), &g_w2_flushes);
REWRITE("Scanner::FlushC09740", 0xcbbd70, scanner_flush_original, (&scanner_flush<0x13f7014, 0x13f7004, 0x13f6ffc, 0x13f7020, 0x13f7018, 0x13e6fe8>), &g_w2_flushes);

// ---- Pooled::Set: replaces the counted object at +OFF; the last holder of the old one destroys it and gives it back to the pool --------
static volatile long g_w2_pooled;
template <uint32_t OFF> static int32_t *__thiscall pooled_set(uint8_t *self, int32_t *value) {
    g_w2_pooled++;
    int32_t *old = *(int32_t **)(self + OFF);
    if (old && --*old == 0) {
        ((void(__fastcall *)(void *))0xc0a750)(old);
        ((void(__thiscall *)(void *, void *))0xca1f80)(*(uint8_t **)0x13e3318 + 4, old);
    }
    *(int32_t **)(self + OFF) = value;
    if (value) ++*value;
    return value;
}
template <uint8_t OFF> static const uint8_t pooled_set_original[] = {0x56, 0x57, 0x8b, 0xf9, 0x8b, 0x77, OFF};
REWRITE("Pooled::SetC0BC10", 0xc0bc10, pooled_set_original<0x78>, (&pooled_set<0x78>), &g_w2_pooled);
REWRITE("Pooled::SetC0BC10", 0xc0bd40, pooled_set_original<0x24>, (&pooled_set<0x24>), &g_w2_pooled);

// ---- Quad::Write: writes four words to a writer ------------------------------------------------------------------------------------
static volatile long g_w2_quad_writes;
static uint32_t __thiscall quad_write(uint32_t *self, void *writer) {
    g_w2_quad_writes++;
    uint32_t result = 0;
    for (int i = 0; i < 4; i++) result = ((uint32_t(__thiscall *)(void *, uint32_t))0xca5610)(writer, self[i]);
    return result;
}
static const uint8_t quad_write_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x06, 0x57, 0x8b, 0x7c};
REWRITE("Quad::WriteC2F850", 0xc2f850, quad_write_original, quad_write, &g_w2_quad_writes);
REWRITE("Quad::WriteC2F850", 0xca3540, quad_write_original, quad_write, &g_w2_quad_writes);

// ---- Flagged::Copy: a flag byte at +OFF and the record after it, into `out` {flag, record at +4} ---------------------------------------
static volatile long g_w2_flag_copies;
template <uint32_t OFF> static uint32_t __thiscall flagged_copy(uint8_t *self, uint8_t *out) {
    g_w2_flag_copies++;
    *out = self[OFF];
    return ((uint32_t(__thiscall *)(void *, void *))0x423fb0)(out + 4, self + OFF + 4);
}
template <uint32_t OFF> static const uint8_t flagged_copy_original[] = {
    0x8a, 0x91, (uint8_t)OFF, (uint8_t)(OFF >> 8), 0x00, 0x00, 0x8b, 0x44, 0x24, 0x04};
REWRITE("Flagged::Copy581120", 0x581120, flagged_copy_original<0x100>, (&flagged_copy<0x100>), &g_w2_flag_copies);
REWRITE("Flagged::Copy581120", 0x5bf340, flagged_copy_original<0x234>, (&flagged_copy<0x234>), &g_w2_flag_copies);
REWRITE("Flagged::Copy581120", 0x5bf3c0, flagged_copy_original<0x240>, (&flagged_copy<0x240>), &g_w2_flag_copies);

// ---- Slot::Reset: gives row `index` a unit pair and a zero at +OFF, when the index is one the owner's table has -----------------------
static volatile long g_w2b_slot_resets;
template <uint32_t OFF> static void __thiscall w2b_slot_reset(uint8_t *self, int32_t index) {
    g_w2b_slot_resets++;
    if (index < 0 || index >= *(int32_t *)(*(uint8_t **)(self + 0x9c) + 0x70)) return;
    uint32_t *row = (uint32_t *)(*(uint8_t **)(self + 0x140) + index * 0x44 + OFF);
    row[0] = 0x3f800000;   // 1.0f, as bits
    row[1] = 0x3f800000;
    row[2] = 0;
}
static const uint8_t w2b_slot_reset_original[] = {0x8b, 0x44, 0x24, 0x04, 0x85, 0xc0, 0x7c, 0x28};
REWRITE("Slot::Reset8278B0", 0x8278b0, w2b_slot_reset_original, (&w2b_slot_reset<0x10>), &g_w2b_slot_resets);
REWRITE("Slot::Reset8278B0", 0x8278f0, w2b_slot_reset_original, (&w2b_slot_reset<0x1c>), &g_w2b_slot_resets);

// ---- Visitor::Run: builds a small visitor {vtable, value} on the stack and hands it to the walker -------------------------------------
static volatile long g_w2b_visits;
template <uint32_t VTABLE> static uint32_t __thiscall w2b_visit(void *self, uint32_t first, uint32_t value, uint32_t third, uint32_t fourth) {
    g_w2b_visits++;
    uint32_t visitor[2] = {VTABLE, value};
    return ((uint32_t(__thiscall *)(void *, uint32_t, void *, uint32_t, uint32_t))0x76fb20)(self, first, visitor, third, fourth);
}
static const uint8_t w2b_visit_original[] = {0x83, 0xec, 0x08, 0x8b, 0x44, 0x24, 0x10, 0x8b};
REWRITE("Visitor::Run76FF60", 0x76ff60, w2b_visit_original, (&w2b_visit<0xf6ce70>), &g_w2b_visits);
REWRITE("Visitor::Run76FF60", 0x76ffa0, w2b_visit_original, (&w2b_visit<0xf6ce6c>), &g_w2b_visits);

// ---- Trio::Update: updates the three members at +0x128, +0x12c and +0x130 in turn -------------------------------------------------------
static volatile long g_w2b_trios;
template <uint32_t UPDATE> static uint32_t __fastcall w2b_trio(uint8_t *self) {
    g_w2b_trios++;
    auto update = (uint32_t(__fastcall *)(void *))UPDATE;
    update(*(void **)(self + 0x128));
    update(*(void **)(self + 0x12c));
    return update(*(void **)(self + 0x130));
}
static const uint8_t w2b_trio_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x8e, 0x28, 0x01, 0x00, 0x00};
REWRITE("Trio::UpdateB010D0", 0xb010d0, w2b_trio_original, (&w2b_trio<0x6b0990>), &g_w2b_trios);
REWRITE("Trio::UpdateB010D0", 0xb01100, w2b_trio_original, (&w2b_trio<0x6afc20>), &g_w2b_trios);

// ---- Owner::Drop: stops the held object at +0x2c, deletes it if it is still there, and forgets it --------------------------------------
static volatile long g_w2b_drops;
template <uint32_t STOP> static void __thiscall w2b_drop(uint8_t *self) {
    g_w2b_drops++;
    ((void(__thiscall *)(void *, uint32_t))STOP)(*(void **)(self + 0x2c), 0);
    void ***held = *(void ****)(self + 0x2c);   // stopping may have cleared it
    if (held) ((void(__thiscall *)(void *, uint32_t))(*held)[0])(held, 1);
    *(void **)(self + 0x2c) = nullptr;
}
static const uint8_t w2b_drop_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x4e, 0x2c, 0x6a, 0x00};
REWRITE("Owner::Drop40DDC0", 0x40ddc0, w2b_drop_original, (&w2b_drop<0x771f90>), &g_w2b_drops);
REWRITE("Owner::Drop40DDC0", 0x40df10, w2b_drop_original, (&w2b_drop<0x771e50>), &g_w2b_drops);

// ---- Listener::Construct: sets the vtable and links the node at +4 in just after the head of the manager's list ------------------------
static volatile long g_w2b_listeners;
template <uint32_t VTABLE, uint32_t MANAGER, uint32_t LIST> static void *__thiscall w2b_listener(uint32_t *self) {
    g_w2b_listeners++;
    self[0] = VTABLE;
    self[3] = (uint32_t)(uintptr_t)self;
    uint32_t **head = *(uint32_t ***)(((uint8_t *(__cdecl *)())MANAGER)() + LIST);
    uint32_t *node = self + 1;
    node[0] = (uint32_t)(uintptr_t)*head;
    (*head)[1] = (uint32_t)(uintptr_t)node;
    node[1] = (uint32_t)(uintptr_t)head;
    *head = node;
    return self;
}
static const uint8_t w2b_listener_original[] = {0x56, 0x8b, 0xf1, 0x57, 0xc7, 0x06};
REWRITE("Listener::Construct872940", 0x872940, w2b_listener_original, (&w2b_listener<0xf78b60, 0x8839b0, 0x2c>), &g_w2b_listeners);
REWRITE("Listener::Construct872940", 0xa04c20, w2b_listener_original, (&w2b_listener<0xf86ea4, 0xa04bc0, 0x20>), &g_w2b_listeners);

// ---- Owned::Clear: destroys and frees each element of {count, items}, then frees the table itself -------------------------------------
static volatile long g_w2b_clears;
template <uint32_t DESTROY> static void __thiscall w2b_clear(int32_t *self) {
    g_w2b_clears++;
    for (int32_t i = 0; i < self[0]; i++) {
        void *item = (*(void ***)(self + 1))[i];
        if (!item) continue;
        ((void(__fastcall *)(void *))DESTROY)(item);
        ((void(__cdecl *)(void *))0xbce000)(item);
    }
    ((void(__cdecl *)(void *))0xbce010)(*(void **)(self + 1));
}
static const uint8_t w2b_clear_original[] = {0x53, 0x8b, 0xd9, 0x8b, 0x03, 0x57, 0x33, 0xff, 0x85, 0xc0};
REWRITE("Owned::ClearC2B760", 0xc2b760, w2b_clear_original, (&w2b_clear<0xc25690>), &g_w2b_clears);
REWRITE("Owned::ClearC2B760", 0xc43910, w2b_clear_original, (&w2b_clear<0xc437f0>), &g_w2b_clears);

// ---- Part::Attach: readies the part, then tells its scene (at +OFF, when there is one) that a part of KIND has arrived -----------------
static volatile long g_w2b_attaches;
template <uint32_t OFF, uint32_t KIND> static void __fastcall w2b_attach(uint8_t *self) {
    g_w2b_attaches++;
    ((void(__fastcall *)(void *))0x6afd60)(self);
    void *scene = *(void **)(self + OFF);
    if (scene) ((void(__thiscall *)(void *, uint32_t, void *))0x6b1330)(scene, KIND, self);
}
template <uint32_t REL> static const uint8_t w2b_call_first_original[] = {
    0x56, 0x8b, 0xf1, 0xe8, (uint8_t)REL, (uint8_t)(REL >> 8), (uint8_t)(REL >> 16), (uint8_t)(REL >> 24)};
REWRITE("Part::Attach67ABE0", 0x67abe0, w2b_call_first_original<0x6afd60u - 0x67abe8u>, (&w2b_attach<0xf8, 2>), &g_w2b_attaches);
REWRITE("Part::Attach67ABE0", 0x870a20, w2b_call_first_original<0x6afd60u - 0x870a28u>, (&w2b_attach<0x12c, 1>), &g_w2b_attaches);

// ---- Stages::Run: four steps of the same object, one after another -----------------------------------------------------------------------
static volatile long g_w2b_stages;
template <uint32_t A, uint32_t B, uint32_t C, uint32_t D> static uint32_t __fastcall w2b_stages(void *self) {
    g_w2b_stages++;
    ((void(__fastcall *)(void *))A)(self);
    ((void(__fastcall *)(void *))B)(self);
    ((void(__fastcall *)(void *))C)(self);
    return ((uint32_t(__fastcall *)(void *))D)(self);
}
REWRITE("Stages::Run550AF0", 0x550af0, w2b_call_first_original<0x549aa0u - 0x550af8u>, (&w2b_stages<0x549aa0, 0x5498c0, 0x54e360, 0x5499a0>), &g_w2b_stages);
REWRITE("Stages::Run550AF0", 0x8f7730, w2b_call_first_original<0x8f6cb0u - 0x8f7738u>, (&w2b_stages<0x8f6cb0, 0x8f6920, 0x8f33f0, 0x8f3b90>), &g_w2b_stages);

// ---- Group::Update: updates the two fixed members at +0x3c and +0x40, then the {items +0x44, count +0x48} list --------------------------
static volatile long g_w2b_groups;
template <uint32_t UPDATE> static void __fastcall w2b_group(uint8_t *self) {
    g_w2b_groups++;
    auto update = (void(__fastcall *)(void *))UPDATE;
    update(*(void **)(self + 0x3c));
    update(*(void **)(self + 0x40));
    for (int32_t i = 0; i < *(int32_t *)(self + 0x48); i++) update((*(void ***)(self + 0x44))[i]);
}
static const uint8_t w2b_group_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x4e, 0x3c, 0x57};
REWRITE("Group::Update74D800", 0x74d800, w2b_group_original, (&w2b_group<0x6b0990>), &g_w2b_groups);
REWRITE("Group::Update74D800", 0x74d840, w2b_group_original, (&w2b_group<0x6afc20>), &g_w2b_groups);

// ---- Node::Announce: walks a {vtable, self} visitor over the node, then reports it if the root has the reporting flag set ----------------
static volatile long g_w2b_announces;
template <uint32_t VTABLE, uint32_t REPORT> static void __fastcall w2b_announce(void *self) {
    g_w2b_announces++;
    uint32_t visitor[2] = {VTABLE, (uint32_t)(uintptr_t)self};
    ((void(__thiscall *)(void *, void *))0x6ae7c0)(self, visitor);
    uint8_t *root = *(uint8_t **)0x12f83f0;
    if (root && (*(uint32_t *)(root + 0x94) >> 0x17 & 1))
        ((void(__thiscall *)(void *, void *))REPORT)(*(void **)0x1342708, self);
}
static const uint8_t w2b_announce_original[] = {0x83, 0xec, 0x08, 0x56, 0x8d, 0x44, 0x24, 0x04, 0x8b, 0xf1};
REWRITE("Node::Announce6AFDC0", 0x6afdc0, w2b_announce_original, (&w2b_announce<0xf606f8, 0xa1f890>), &g_w2b_announces);
REWRITE("Node::Announce6AFDC0", 0x6afe10, w2b_announce_original, (&w2b_announce<0xf60700, 0xa1f8a0>), &g_w2b_announces);

// ---- Shared::Assign: takes a counted block {count, .., data at +0xc} into +OFF; the last holder of the old one frees it and its data ----
static volatile long g_w2b_assigns;
template <uint32_t OFF> static int32_t *__thiscall w2b_assign(uint8_t *self, int32_t **other) {
    g_w2b_assigns++;
    int32_t *old = *(int32_t **)(self + OFF);
    if (*other == old) return old;
    if (--*old == 0) {
        ((void(__cdecl *)(void *))0xbce010)(*(void **)(old + 3));
        ((void(__cdecl *)(void *))0xbce000)(old);
    }
    int32_t *taken = *other;
    *(int32_t **)(self + OFF) = taken;
    ++*taken;
    return taken;
}
static const uint8_t w2b_assign_original[] = {0x53, 0x8b, 0x5c, 0x24, 0x08, 0x57, 0x8b, 0xf9};
REWRITE("Shared::Assign41BEA0", 0x41bea0, w2b_assign_original, (&w2b_assign<0x114>), &g_w2b_assigns);
REWRITE("Shared::Assign41BEA0", 0x4a90a0, w2b_assign_original, (&w2b_assign<0xb0>), &g_w2b_assigns);

// ---- Quarter::Value: reads the +OFF field of the quarter that the index at +0xa4 picks, from the object the source gives ------------------
static volatile long g_w2b_quarters;
template <uint32_t OFF> static uint32_t __fastcall w2b_quarter(uint8_t *self) {
    g_w2b_quarters++;
    int32_t which = *(int32_t *)(self + 0xa4);
    uint8_t *source = ((uint8_t *(__fastcall *)(void *))0xb44970)(self);
    uint8_t *table = *(uint8_t **)(source + 0x54);
    int slot = which == 0 ? 0x18 : which == 1 ? 0x1c : which == 2 ? 0x20 : 0x24;
    return *(uint32_t *)(*(uint8_t **)(table + slot) + OFF);
}
static const uint8_t w2b_quarter_original[] = {0x56, 0x8b, 0xb1, 0xa4, 0x00, 0x00, 0x00, 0xe8};
REWRITE("Quarter::ValueB35490", 0xb35490, w2b_quarter_original, (&w2b_quarter<0x2c>), &g_w2b_quarters);
REWRITE("Quarter::ValueB35490", 0xb35520, w2b_quarter_original, (&w2b_quarter<0x14>), &g_w2b_quarters);

// ---- Setup::Once: the first time only, does each of the two jobs its check asks for, then remembers it has run --------------------------
static volatile long g_w2b_setups;
template <uint32_t FLAG, uint32_t CHECK1, uint32_t DO1, uint32_t CHECK2, uint32_t DO2> static void __fastcall w2b_setup(uint8_t *self) {
    g_w2b_setups++;
    if (self[FLAG]) return;
    if (((uint8_t(__fastcall *)(void *))CHECK1)(self)) ((void(__fastcall *)(void *))DO1)(self);
    if (((uint8_t(__fastcall *)(void *))CHECK2)(self)) ((void(__fastcall *)(void *))DO2)(self);
    self[FLAG] = 1;
}
template <uint32_t FLAG> static const uint8_t w2b_setup_original[] = {0x56, 0x8b, 0xf1, 0x8a, 0x86, (uint8_t)FLAG, (uint8_t)(FLAG >> 8), 0x00, 0x00, 0x84, 0xc0};
REWRITE("Setup::OnceA12CB0", 0xa12cb0, w2b_setup_original<0x15c>, (&w2b_setup<0x15c, 0xa113d0, 0xa11680, 0xa114a0, 0xa11570>), &g_w2b_setups);
REWRITE("Setup::OnceA12CB0", 0xa161d0, w2b_setup_original<0x14c>, (&w2b_setup<0x14c, 0xa14bf0, 0xa14ef0, 0xa14cc0, 0xa14d90>), &g_w2b_setups);

// ---- Rows::Resize: frees the old {data, count} table and, for a nonzero count, builds a new one of 20-byte rows with INIT ------------------
static volatile long g_w2b_resizes;
template <uint32_t INIT> static uint32_t __thiscall w2b_resize(uint32_t *self, uint32_t count) {
    g_w2b_resizes++;
    if (self[0]) {
        ((void(__cdecl *)(uint32_t))0xbce010)(self[0]);
        self[0] = 0;
    }
    if (count) {
        void *rows = ((void *(__cdecl *)(uint32_t))0xbcdfd0)(count * 20);
        if (rows) ((void(__stdcall *)(void *, uint32_t, uint32_t, uint32_t))0x5d3d00)(rows, 0x14, count, INIT);
        self[0] = (uint32_t)(uintptr_t)rows;
    }
    self[1] = count;
    return count;
}
static const uint8_t w2b_resize_original[] = {0x53, 0x8b, 0xd9, 0x8b, 0x03, 0x85, 0xc0, 0x56};
REWRITE("Rows::Resize5D4350", 0x5d4350, w2b_resize_original, (&w2b_resize<0x5d3da0>), &g_w2b_resizes);
REWRITE("Rows::Resize5D4350", 0x5d4470, w2b_resize_original, (&w2b_resize<0x5d3dc0>), &g_w2b_resizes);

// ---- Weights::Ensure: finds the key's place in the table at +OFF, and gives an empty place a new weight of 1.0 ---------------------------
static volatile long g_w2b_ensures;
template <uint32_t OFF, uint32_t FIND> static uint32_t __thiscall w2b_ensure(uint8_t *self, uint32_t key) {
    g_w2b_ensures++;
    uint32_t ***table = (uint32_t ***)(self + OFF);
    uint32_t place = ((uint32_t(__thiscall *)(void *, uint32_t))FIND)(table, key);
    if (!(*table)[place]) {
        uint32_t *weight = ((uint32_t *(__cdecl *)(uint32_t))0xbcdfc0)(4);
        if (weight) *weight = 0x3f800000;   // 1.0f, as bits
        (*table)[place] = weight;
    }
    return place;
}
static const uint8_t w2b_ensure_original[] = {0x8b, 0x44, 0x24, 0x04, 0x56, 0x57, 0x8d, 0x71};
REWRITE("Weights::Ensure40C160", 0x40c160, w2b_ensure_original, (&w2b_ensure<0x14, 0x40b550>), &g_w2b_ensures);
REWRITE("Weights::Ensure40C160", 0xceb3c0, w2b_ensure_original, (&w2b_ensure<0x28, 0xcea320>), &g_w2b_ensures);

// ---- Quarter::Has: whether the quarter that the index at +0xa4 picks has anything at +OFF -----------------------------------------------
static volatile long g_w2b_quarter_checks;
template <uint32_t OFF> static uint8_t __fastcall w2b_quarter_has(uint8_t *self) {
    g_w2b_quarter_checks++;
    int32_t which = *(int32_t *)(self + 0xa4);
    uint8_t *source = ((uint8_t *(__fastcall *)(void *))0xb44970)(self);
    uint8_t *table = *(uint8_t **)(source + 0x54);
    int slot = which == 0 ? 0x18 : which == 1 ? 0x1c : which == 2 ? 0x20 : 0x24;
    return *(uint32_t *)(*(uint8_t **)(table + slot) + OFF) != 0;
}
REWRITE("Quarter::HasB356B0", 0xb356b0, w2b_quarter_original, (&w2b_quarter_has<0x44>), &g_w2b_quarter_checks);
REWRITE("Quarter::HasB356B0", 0xb35710, w2b_quarter_original, (&w2b_quarter_has<0x48>), &g_w2b_quarter_checks);

// ---- Judge::Act: when ready and allowed by MASK, asks for a verdict: 2 acts with CODE, 1 falls to the plain way, any other does nothing ---
static volatile long g_w2b_judgements;
template <uint32_t MASK, uint32_t CODE> static uint32_t __thiscall w2b_judge(uint8_t *self, uint32_t subject) {
    g_w2b_judgements++;
    if (((uint8_t(__fastcall *)(void *))0x79ff40)(self) && (self[0x408] & MASK)) {
        uint32_t verdict = ((uint32_t(__thiscall *)(void *, uint32_t))0x7c4970)(self, subject);
        if (verdict == 2) return ((uint32_t(__thiscall *)(void *, uint32_t))0x7a0030)(self, CODE);
        if (verdict != 1) return verdict - 2;
    }
    return ((uint32_t(__fastcall *)(void *))0x7a0000)(self);
}
REWRITE("Judge::Act7A0F50", 0x7a0f50, w2b_call_first_original<0x79ff40u - 0x7a0f58u>, (&w2b_judge<2, 0x10>), &g_w2b_judgements);
REWRITE("Judge::Act7A0F50", 0x7a0f90, w2b_call_first_original<0x79ff40u - 0x7a0f98u>, (&w2b_judge<1, 0x15>), &g_w2b_judgements);

// ---- Cached::Pass: looks a value up once and keeps it at +SEEN-4, then passes it to the target's slot 0x48 ------------------------------
static volatile long g_w2b_cached;
template <uint32_t SEEN, uint32_t FETCH> static uint32_t __fastcall w2b_cached(uint8_t *self) {
    g_w2b_cached++;
    uint32_t *cache = (uint32_t *)(self + SEEN - 4);
    if (!self[SEEN]) {
        void *source = *(void **)(*(uint8_t **)(self + SEEN - 0x14) + 0xc);
        *cache = source ? ((uint32_t(__fastcall *)(void *))FETCH)(source) : 0;
        self[SEEN] = 1;
    }
    void ***target = *(void ****)(self + SEEN + 4);
    return ((uint32_t(__thiscall *)(void *, uint32_t))(*target)[0x48])(target, *cache);
}
REWRITE("Cached::PassA80240", 0xa80240, w2b_setup_original<0x154>, (&w2b_cached<0x154, 0x4a8700>), &g_w2b_cached);
REWRITE("Cached::PassA80240", 0xba8db0, w2b_setup_original<0x158>, (&w2b_cached<0x158, 0xb85520>), &g_w2b_cached);

// ======== Third pass of wave 2: destructors, makers and singletons that carry an exception frame (left out here) ========
template <uint32_t HANDLER>
static const uint8_t w2c_seh[] = {0x6a, 0xff, 0x68, (uint8_t)HANDLER, (uint8_t)(HANDLER >> 8), (uint8_t)(HANDLER >> 16), (uint8_t)(HANDLER >> 24)};
static const uint8_t w2c_seh_late[] = {0x64, 0xa1, 0x00, 0x00, 0x00, 0x00, 0x6a, 0xff};
static inline uint32_t &w2c_u32(void *p, uint32_t off) { return *(uint32_t *)((uint8_t *)p + off); }
template <uint32_t FN> static inline void w2c_run(void *self) { ((void(__fastcall *)(void *))FN)(self); }
constexpr uint32_t w2c_text_drop = 0x40f730;   // a member text's destructor

// Reference-counted text buffer {count, .., .., data}: the last owner frees it.
struct W2cShared {
    int32_t count;
    uint32_t unknown[2];
    void *data;
};
static inline void w2c_drop(W2cShared *s) {
    if (--s->count == 0) {
        memory_free_aligned(s->data);
        memory_free(s);
    }
}

// ---- Singleton::Get: the object in GLOBAL; the first call builds it, and its constructor fills GLOBAL in --------------------------------
static volatile long g_w2c_singletons;
template <uint32_t GLOBAL, uint32_t SIZE, uint32_t CTOR> static void *__cdecl w2c_singleton() {
    g_w2c_singletons++;
    void **the_one = (void **)GLOBAL;
    if (!*the_one) {
        void *object = memory_alloc(SIZE);
        if (object) ((void(__thiscall *)(void *, uint32_t))CTOR)(object, 0);
    }
    return *the_one;
}
REWRITE("Singleton::Get40DA80", 0x40da80, w2c_seh<0xe053fb>, (&w2c_singleton<0x1306cd0, 0xe4, 0x7b6180>), &g_w2c_singletons);
REWRITE("Singleton::Get40DA80", 0x41b0b0, w2c_seh<0xe0652b>, (&w2c_singleton<0x12f80fc, 0x84, 0x662750>), &g_w2c_singletons);
REWRITE("Singleton::Get40DA80", 0x440fd0, w2c_seh<0xe094ab>, (&w2c_singleton<0x1106c48, 0x90, 0x4405f0>), &g_w2c_singletons);
REWRITE("Singleton::Get40DA80", 0x6d56d0, w2c_seh<0xe4309b>, (&w2c_singleton<0x13060b8, 0x88, 0x6e9d70>), &g_w2c_singletons);
REWRITE("Singleton::Get40DA80", 0x98e940, w2c_seh<0xe6e1cb>, (&w2c_singleton<0x134131c, 0x270, 0x98e760>), &g_w2c_singletons);
REWRITE("Singleton::Get40DA80", 0xa64700, w2c_seh<0xe82f7b>, (&w2c_singleton<0x13429b8, 0x290, 0xa644a0>), &g_w2c_singletons);
REWRITE("Singleton::Get40DA80", 0xb04840, w2c_seh<0xe8ee1b>, (&w2c_singleton<0x13ce688, 0x350, 0xb04200>), &g_w2c_singletons);

// ---- Singleton::Keep: as above, but the object is stored here and is told its callback -------------------------------------------------
static volatile long g_w2c_kept;
template <uint32_t GLOBAL, uint32_t TOLD> static void *__cdecl w2c_kept() {
    g_w2c_kept++;
    void **the_one = (void **)GLOBAL;
    if (!*the_one) {
        void *object = memory_alloc(0xc);
        if (object) ((void(__thiscall *)(void *, uint32_t, uint32_t))0xbe79b0)(object, TOLD, 0);
        *the_one = object;
    }
    return *the_one;
}
REWRITE("Singleton::Keep4095B0", 0x4095b0, w2c_seh<0xe0511b>, (&w2c_kept<0x10ff9c8, 0x408120>), &g_w2c_kept);
REWRITE("Singleton::Keep4095B0", 0x496040, w2c_seh<0xe0f3db>, (&w2c_kept<0x12f4694, 0x495c10>), &g_w2c_kept);

// ---- Factory::Make: a new 32-byte entry with its two buffers, its type and the function that makes its kind ------------------------------
static volatile long g_w2c_factories;
template <uint32_t TYPE, uint32_t MAKE> static void *__cdecl w2c_factory() {
    g_w2c_factories++;
    uint8_t *entry = (uint8_t *)memory_alloc(0x20);
    if (!entry) return nullptr;
    w2c_run<0x808700>(entry);
    w2c_u32(entry, 0x18) = TYPE;
    w2c_u32(entry, 0x1c) = MAKE;
    return entry;
}
REWRITE("Factory::Make7FF4E0", 0x7ff4e0, w2c_seh<0xe5b9bb>, (&w2c_factory<0xf7526c, 0x7ff460>), &g_w2c_factories);
REWRITE("Factory::Make7FF4E0", 0x7ff5d0, w2c_seh<0xe5b9fb>, (&w2c_factory<0xf75278, 0x7ff550>), &g_w2c_factories);
REWRITE("Factory::Make7FF4E0", 0x7ff6c0, w2c_seh<0xe5ba3b>, (&w2c_factory<0xf75288, 0x7ff640>), &g_w2c_factories);
REWRITE("Factory::Make7FF4E0", 0x7ff7b0, w2c_seh<0xe5ba7b>, (&w2c_factory<0xf75298, 0x7ff730>), &g_w2c_factories);
REWRITE("Factory::Make7FF4E0", 0x7ff8a0, w2c_seh<0xe5babb>, (&w2c_factory<0xf752ac, 0x7ff820>), &g_w2c_factories);

// ---- Shared::Assign: the buffer at +OFF becomes `value`, which arrived as an owned copy that is given up on the way out -------------------
static volatile long g_w2c_assigns;
template <uint32_t OFF> static void __thiscall w2c_assign(uint8_t *self, W2cShared *value) {
    g_w2c_assigns++;
    W2cShared *&slot = *(W2cShared **)(self + OFF);
    if (value != slot) {
        w2c_drop(slot);
        slot = value;
        value->count++;
    }
    w2c_drop(value);
}
REWRITE("Shared::Assign591190", 0x591190, w2c_seh_late, (&w2c_assign<0x64>), &g_w2c_assigns);
REWRITE("Shared::Assign591190", 0x8098f0, w2c_seh_late, (&w2c_assign<0x20>), &g_w2c_assigns);
REWRITE("Shared::Assign591190", 0x872180, w2c_seh_late, (&w2c_assign<0x68>), &g_w2c_assigns);
REWRITE("Shared::Assign591190", 0x9ed9c0, w2c_seh_late, (&w2c_assign<0x40>), &g_w2c_assigns);
REWRITE("Shared::Assign591190", 0xa561d0, w2c_seh_late, (&w2c_assign<0x4>), &g_w2c_assigns);

// ---- Shared::DropTwo: gives up the buffers at +A and +B, then falls back to the base's table ------------------------------------------
static volatile long g_w2c_drop_twos;
template <uint32_t A, uint32_t B> static void __fastcall w2c_drop_two(uint8_t *self) {
    g_w2c_drop_twos++;
    w2c_drop(*(W2cShared **)(self + A));
    w2c_drop(*(W2cShared **)(self + B));
    w2c_u32(self, 0) = 0xf75170;
}
REWRITE("Shared::DropTwo7FD6F0", 0x7fd6f0, w2c_seh<0xe5b4f3>, (&w2c_drop_two<0x14, 0x10>), &g_w2c_drop_twos);
REWRITE("Shared::DropTwo7FD6F0", 0x7fe530, w2c_seh<0xe5b733>, (&w2c_drop_two<0x14, 0x10>), &g_w2c_drop_twos);
REWRITE("Shared::DropTwo7FD6F0", 0x7ff3d0, w2c_seh<0xe5b983>, (&w2c_drop_two<0xc, 0x8>), &g_w2c_drop_twos);

// ---- Dtor::TablesMember: sets its two tables, destroys the member at +OFF, then the base ------------------------------------------------
static volatile long g_w2c_tables_member;
template <uint32_t T0, uint32_t T1, uint32_t OFF, uint32_t MEMBER, uint32_t BASE> static void __fastcall w2c_tables_member(uint8_t *self) {
    g_w2c_tables_member++;
    w2c_u32(self, 0) = T0;
    w2c_u32(self, 4) = T1;
    w2c_run<MEMBER>(self + OFF);
    w2c_run<BASE>(self);
}
REWRITE("Dtor::TablesMember456980", 0x456980, w2c_seh<0xe0ab58>, (&w2c_tables_member<0xf29834, 0xf29830, 0x40, 0x4560b0, 0x800940>), &g_w2c_tables_member);
REWRITE("Dtor::TablesMember456980", 0x5ef9b0, w2c_seh<0xe266a8>, (&w2c_tables_member<0xf3fc54, 0xf3fc50, 0x40, 0x40f730, 0x800940>), &g_w2c_tables_member);
REWRITE("Dtor::TablesMember456980", 0x98d7e0, w2c_seh<0xe6dfc8>, (&w2c_tables_member<0xf7ff38, 0xf7ff34, 0x70, 0x40f730, 0x61bed0>), &g_w2c_tables_member);
REWRITE("Dtor::TablesMember456980", 0xa313d0, w2c_seh<0xe7cfc8>, (&w2c_tables_member<0xf8970c, 0xf89708, 0x40, 0x40f730, 0x800940>), &g_w2c_tables_member);
REWRITE("Dtor::TablesMember456980", 0xb7a4c0, w2c_seh<0xe979e8>, (&w2c_tables_member<0xf9b720, 0xf9b718, 0x44, 0x40f730, 0x800940>), &g_w2c_tables_member);

// ---- Dtor::TwoTexts: destroys the member texts at +A and +B, then the base ---------------------------------------------------------------
static volatile long g_w2c_two_texts;
template <uint32_t A, uint32_t B, uint32_t BASE> static void __fastcall w2c_two_texts(uint8_t *self) {
    g_w2c_two_texts++;
    w2c_run<w2c_text_drop>(self + A);
    w2c_run<w2c_text_drop>(self + B);
    w2c_run<BASE>(self);
}
REWRITE("Dtor::TwoTexts4C68D0", 0x4c68d0, w2c_seh<0xe135a3>, (&w2c_two_texts<0x5c, 0x40, 0x4d3c20>), &g_w2c_two_texts);
REWRITE("Dtor::TwoTexts4C68D0", 0x4d4f10, w2c_seh<0xe144b3>, (&w2c_two_texts<0x5c, 0x40, 0x4d3c20>), &g_w2c_two_texts);
REWRITE("Dtor::TwoTexts4C68D0", 0x533dc0, w2c_seh<0xe1b1e3>, (&w2c_two_texts<0x50, 0x1c, 0x40f730>), &g_w2c_two_texts);
REWRITE("Dtor::TwoTexts4C68D0", 0xabed50, w2c_seh<0xe88573>, (&w2c_two_texts<0x38, 0x1c, 0x40f730>), &g_w2c_two_texts);

// ---- Dtor::JustTexts: destroys the member texts at +A and +B -----------------------------------------------------------------------------
static volatile long g_w2c_just_texts;
template <uint32_t A, uint32_t B> static void __fastcall w2c_just_texts(uint8_t *self) {
    g_w2c_just_texts++;
    w2c_run<w2c_text_drop>(self + A);
    w2c_run<w2c_text_drop>(self + B);
}
REWRITE("Dtor::JustTexts49BBC0", 0x49bbc0, w2c_seh<0xe0fabb>, (&w2c_just_texts<0x20, 0xc>), &g_w2c_just_texts);
REWRITE("Dtor::JustTexts49BBC0", 0x725de0, w2c_seh<0xe4c16b>, (&w2c_just_texts<0x18, 0x4>), &g_w2c_just_texts);

// ---- Dtor::Pair: runs the object's own cleanup, then its base's ----------------------------------------------------------------------------
static volatile long g_w2c_pairs;
template <uint32_t FIRST, uint32_t BASE> static void __fastcall w2c_pair(void *self) {
    g_w2c_pairs++;
    w2c_run<FIRST>(self);
    w2c_run<BASE>(self);
}
REWRITE("Dtor::Pair496DC0", 0x496dc0, w2c_seh<0xe0f5d8>, (&w2c_pair<0xbe5360, 0xbe4f30>), &g_w2c_pairs);
REWRITE("Dtor::Pair496DC0", 0x496e50, w2c_seh<0xe0f5f8>, (&w2c_pair<0xbe5360, 0xbe4f30>), &g_w2c_pairs);
REWRITE("Dtor::Pair496DC0", 0x594000, w2c_seh<0xe21778>, (&w2c_pair<0x5937e0, 0x40f730>), &g_w2c_pairs);
REWRITE("Dtor::Pair496DC0", 0x594050, w2c_seh<0xe21798>, (&w2c_pair<0x593900, 0x40f730>), &g_w2c_pairs);

// ---- Dtor::TablesPair: sets its two tables (the second at +OFF), then its own cleanup and its base's ---------------------------------------
static volatile long g_w2c_tables_pairs;
template <uint32_t T0, uint32_t OFF, uint32_t T1, uint32_t FIRST, uint32_t BASE> static void __fastcall w2c_tables_pair(uint8_t *self) {
    g_w2c_tables_pairs++;
    w2c_u32(self, 0) = T0;
    w2c_u32(self, OFF) = T1;
    w2c_run<FIRST>(self);
    w2c_run<BASE>(self);
}
REWRITE("Dtor::TablesPair4DD140", 0x4dd140, w2c_seh<0xe15718>, (&w2c_tables_pair<0xf344b8, 4, 0xf344a0, 0x6af540, 0x63a9c0>), &g_w2c_tables_pairs);
REWRITE("Dtor::TablesPair4DD140", 0x7f0550, w2c_seh<0xe5a308>, (&w2c_tables_pair<0xf74508, 4, 0xf744f4, 0x6af540, 0x6b0ea0>), &g_w2c_tables_pairs);
REWRITE("Dtor::TablesPair4DD140", 0xa9cb10, w2c_seh<0xe86648>, (&w2c_tables_pair<0xf91eb8, 4, 0xf91ea0, 0x6af540, 0x63a9c0>), &g_w2c_tables_pairs);
REWRITE("Dtor::TablesPair4DD140", 0xc3a7f0, w2c_seh<0xea10c8>, (&w2c_tables_pair<0xfb0af0, 0xc, 0xfb0aec, 0xc3a4b0, 0xc424f0>), &g_w2c_tables_pairs);

// ---- Dtor::FourTables: sets its four tables, deletes the object it owns at +CHILD, then the base ------------------------------------------
static volatile long g_w2c_four_tables;
template <uint32_t T0, uint32_t T1, uint32_t O2, uint32_t T2, uint32_t O3, uint32_t T3, uint32_t CHILD, uint32_t BASE>
static void __fastcall w2c_four_tables(uint8_t *self) {
    g_w2c_four_tables++;
    w2c_u32(self, 0) = T0;
    w2c_u32(self, 4) = T1;
    w2c_u32(self, O2) = T2;
    w2c_u32(self, O3) = T3;
    void ***child = *(void ****)(self + CHILD);
    if (child) ((void(__thiscall *)(void *, uint32_t))(*child)[0])(child, 1);
    w2c_run<BASE>(self);
}
REWRITE("Dtor::FourTables41B1A0", 0x41b1a0, w2c_seh<0xe06548>, (&w2c_four_tables<0xf27130, 0xf2711c, 0x108, 0xf27118, 0x118, 0xf27110, 0x158, 0x676220>), &g_w2c_four_tables);
REWRITE("Dtor::FourTables41B1A0", 0x99a7f0, w2c_seh<0xe6f3c8>, (&w2c_four_tables<0xf80b18, 0xf80b04, 0xd4, 0xf80af0, 0xec, 0xf80ae8, 0xf4, 0x659b40>), &g_w2c_four_tables);
REWRITE("Dtor::FourTables41B1A0", 0xa3c990, w2c_seh<0xe7e598>, (&w2c_four_tables<0xf8a118, 0xf8a104, 0x108, 0xf8a100, 0x118, 0xf8a0f8, 0x158, 0x676220>), &g_w2c_four_tables);
REWRITE("Dtor::FourTables41B1A0", 0xa40a00, w2c_seh<0xe7e9c8>, (&w2c_four_tables<0xf8d7e8, 0xf8d7d4, 0xd4, 0xf8d7c0, 0xdc, 0xf8d7bc, 0x13c, 0x67b9f0>), &g_w2c_four_tables);

// ---- Buffers::Init: two empty buffers, and the step their size grows by ------------------------------------------------------------------
static volatile long g_w2c_buffers;
template <uint32_t STEP> static uint32_t *__fastcall w2c_buffers(uint32_t *self) {
    g_w2c_buffers++;
    self[1] = 0;
    self[0] = (uint32_t)(uintptr_t)memory_alloc_aligned(0);
    self[3] = 0;
    self[2] = (uint32_t)(uintptr_t)memory_alloc_aligned(0);
    self[5] = 0;
    self[4] = STEP;
    return self;
}
REWRITE("Buffers::Init7FC020", 0x7fc020, w2c_seh<0xe5b238>, (&w2c_buffers<0x80>), &g_w2c_buffers);
REWRITE("Buffers::Init7FC020", 0x7fd440, w2c_seh<0xe5b458>, (&w2c_buffers<0x80>), &g_w2c_buffers);
REWRITE("Buffers::Init7FC020", 0x808700, w2c_seh<0xe5c8f8>, (&w2c_buffers<0x20>), &g_w2c_buffers);

// ---- Buffers::Free: frees the aligned block at +A, and the one at +B if there is one (and forgets it) ------------------------------------
static volatile long g_w2c_frees;
template <uint32_t A, uint32_t B> static void __fastcall w2c_free_two(uint8_t *self) {
    g_w2c_frees++;
    memory_free_aligned(*(void **)(self + A));
    if (void *block = *(void **)(self + B)) {
        memory_free_aligned(block);
        *(void **)(self + B) = nullptr;
    }
}
REWRITE("Buffers::Free84F220", 0x84f220, w2c_seh<0xe6135b>, (&w2c_free_two<0x4, 0x10>), &g_w2c_frees);
REWRITE("Buffers::Free84F220", 0x983870, w2c_seh<0xe6d77b>, (&w2c_free_two<0x18, 0x20>), &g_w2c_frees);

// ---- Make::OnceFor: unless one is already there, makes a helper that knows its owner (who sits 0x40 before this interface) -----------------
static volatile long g_w2c_once;
template <uint32_t SIZE, uint32_t CTOR> static uint8_t __thiscall w2c_once(uint8_t *self, uint32_t) {
    g_w2c_once++;
    if (!w2c_u32(self, 0x4b4)) {
        void *helper = memory_alloc(SIZE);
        if (helper) ((void(__thiscall *)(void *, uint32_t, void *))CTOR)(helper, 0, self - 0x40);
    }
    return 1;
}
REWRITE("Make::OnceFor754DE0", 0x754de0, w2c_seh<0xe4fdcb>, (&w2c_once<0x148, 0x78f490>), &g_w2c_once);
REWRITE("Make::OnceFor754DE0", 0x7971c0, w2c_seh<0xe54d8b>, (&w2c_once<0x188, 0x78dfc0>), &g_w2c_once);
REWRITE("Make::OnceFor754DE0", 0x7a68f0, w2c_seh<0xe5613b>, (&w2c_once<0x184, 0x78f9f0>), &g_w2c_once);
REWRITE("Make::OnceFor754DE0", 0x7e8f50, w2c_seh<0xe596fb>, (&w2c_once<0x134, 0x78ed20>), &g_w2c_once);

// ---- Make::FromField: builds an object from the value at +0x28; the argument is not used ---------------------------------------------------
static volatile long g_w2c_from_field;
template <uint32_t SIZE, uint32_t CTOR> static uint32_t __thiscall w2c_from_field(uint8_t *self, uint32_t) {
    g_w2c_from_field++;
    void *object = memory_alloc(SIZE);
    return object ? ((uint32_t(__thiscall *)(void *, uint32_t, uint32_t))CTOR)(object, 0, w2c_u32(self, 0x28)) : 0;
}
REWRITE("Make::FromField555CB0", 0x555cb0, w2c_seh<0xe1e09b>, (&w2c_from_field<0x170, 0xba78d0>), &g_w2c_from_field);
REWRITE("Make::FromField555CB0", 0x5560c0, w2c_seh<0xe1e0fb>, (&w2c_from_field<0x224, 0xa857e0>), &g_w2c_from_field);

// ---- Make::FromThree: builds an object from the values at +0x40, +0x4c and +0x50 -----------------------------------------------------------
static volatile long g_w2c_from_three;
template <uint32_t SIZE, uint32_t CTOR> static uint32_t __fastcall w2c_from_three(uint8_t *self) {
    g_w2c_from_three++;
    void *object = memory_alloc(SIZE);
    if (!object) return 0;
    return ((uint32_t(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))CTOR)(object, 0, 0, w2c_u32(self, 0x40), w2c_u32(self, 0x4c),
                                                                                                    w2c_u32(self, 0x50));
}
REWRITE("Make::FromThree632790", 0x632790, w2c_seh<0xe2f97b>, (&w2c_from_three<0x198, 0x4d9ef0>), &g_w2c_from_three);
REWRITE("Make::FromThree632790", 0x632870, w2c_seh<0xe2f99b>, (&w2c_from_three<0x148, 0x57e180>), &g_w2c_from_three);

// ---- Make::FromArgs: builds an object from the two arguments, with the rest of the constructor's arguments cleared -------------------------
static volatile long g_w2c_from_args;
template <uint32_t SIZE, uint32_t CTOR> static uint32_t __stdcall w2c_from_args(uint32_t a, uint32_t b) {
    g_w2c_from_args++;
    void *object = memory_alloc(SIZE);
    if (!object) return 0;
    return ((uint32_t(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))CTOR)(object, 0, a, b, 0, 0);
}
REWRITE("Make::FromArgs6D1060", 0x6d1060, w2c_seh<0xe424fb>, (&w2c_from_args<0x80, 0x6cffc0>), &g_w2c_from_args);
REWRITE("Make::FromArgs6D1060", 0xb295f0, w2c_seh<0xe9247b>, (&w2c_from_args<0xd0, 0xb29210>), &g_w2c_from_args);

// ---- Make::WithTail: the same with a fixed last argument --------------------------------------------------------------------------------
static volatile long g_w2c_with_tail;
template <uint32_t SIZE, uint32_t CTOR, int32_t TAIL> static uint32_t __stdcall w2c_with_tail(uint32_t a, uint32_t b) {
    g_w2c_with_tail++;
    void *object = memory_alloc(SIZE);
    if (!object) return 0;
    return ((uint32_t(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, int32_t))CTOR)(object, 0, a, b, 0, TAIL);
}
REWRITE("Make::WithTail6D5BD0", 0x6d5bd0, w2c_seh<0xe4312b>, (&w2c_with_tail<0x50, 0x6d59b0, -1>), &g_w2c_with_tail);
REWRITE("Make::WithTail6D5BD0", 0xb612d0, w2c_seh<0xe96adb>, (&w2c_with_tail<0x70, 0xb606e0, 0>), &g_w2c_with_tail);

// ---- Make::FromPool: if the argument is accepted, builds an object in the pool's memory -----------------------------------------------------
static volatile long g_w2c_from_pool;
template <uint32_t SIZE, uint32_t CTOR> static uint32_t __thiscall w2c_from_pool(void *self, uint32_t arg) {
    g_w2c_from_pool++;
    if (!((uint8_t(__thiscall *)(void *, uint32_t))0xc0d5a0)(self, arg)) return 0;
    uint8_t *pool = *(uint8_t **)0x13e3318;
    void *object = ((void *(__thiscall *)(void *, uint32_t, uint32_t))0xca1eb0)(pool + 4, 4, SIZE);
    return object ? ((uint32_t(__thiscall *)(void *, void *, uint32_t))CTOR)(object, self, arg) : 0;
}
REWRITE("Make::FromPoolC0E4C0", 0xc0e4c0, w2c_seh_late, (&w2c_from_pool<0x8c, 0xc0e420>), &g_w2c_from_pool);
REWRITE("Make::FromPoolC0E4C0", 0xc10940, w2c_seh_late, (&w2c_from_pool<0x94, 0xc10840>), &g_w2c_from_pool);
REWRITE("Make::FromPoolC0E4C0", 0xc10ca0, w2c_seh_late, (&w2c_from_pool<0xc4, 0xc10b30>), &g_w2c_from_pool);

// ---- Make::Choosing: builds an object, passing the source's first word or the one at +ALT, as the source itself says -----------------------
static volatile long g_w2c_choosing;
template <uint32_t OFF, uint32_t CHECK, uint32_t ALT> static uint32_t __fastcall w2c_choosing(uint8_t *self) {
    g_w2c_choosing++;
    void *object = memory_alloc(0xac);
    if (!object) return 0;
    uint8_t *source = *(uint8_t **)(self + OFF);
    uint32_t value = ((uint8_t(__fastcall *)(void *))CHECK)(source) ? w2c_u32(source, 0) : w2c_u32(source, ALT);
    return ((uint32_t(__thiscall *)(void *, uint32_t, void *, uint32_t))0x4e2630)(object, 0, self, value);
}
REWRITE("Make::Choosing4A8910", 0x4a8910, w2c_seh<0xe1120b>, (&w2c_choosing<0x3a8, 0xbb5120, 0x18>), &g_w2c_choosing);
REWRITE("Make::Choosing4A8910", 0x585b20, w2c_seh<0xe20dfb>, (&w2c_choosing<0x1d0, 0xbb5180, 0x8>), &g_w2c_choosing);

// ---- Text::Copy: a new holder made from the other's text, kept in the first word -------------------------------------------------------------
struct W2cTextBody {
    uint32_t unknown;
    uint32_t length;   // +4
    uint32_t unknown2;
    void *data;        // +0xc
};
static volatile long g_w2c_text_copies;
template <uint32_t CTOR> static uint32_t *__thiscall w2c_text_copy(uint32_t *self, W2cTextBody **other) {
    g_w2c_text_copies++;
    void *holder = memory_alloc(0x10);
    *self = holder ? ((uint32_t(__thiscall *)(void *, void *, uint32_t))CTOR)(holder, (*other)->data, (*other)->length) : 0;
    return self;
}
REWRITE("Text::CopyBCFEF0", 0xbcfef0, w2c_seh<0xe9d48b>, (&w2c_text_copy<0xbcf770>), &g_w2c_text_copies);
REWRITE("Text::CopyBCFEF0", 0xbd01c0, w2c_seh<0xe9d50b>, (&w2c_text_copy<0xbcf9e0>), &g_w2c_text_copies);

// ---- Text::CopyLeft: the same, of at most `limit` characters ---------------------------------------------------------------------------------
static volatile long g_w2c_text_lefts;
template <uint32_t CTOR> static uint32_t *__thiscall w2c_text_left(uint32_t *self, W2cTextBody **other, uint32_t limit) {
    g_w2c_text_lefts++;
    void *holder = memory_alloc(0x10);
    uint32_t made = 0;
    if (holder) {
        W2cTextBody *body = *other;
        uint32_t length = limit > body->length ? body->length : limit;
        made = ((uint32_t(__thiscall *)(void *, void *, uint32_t))CTOR)(holder, body->data, length);
    }
    *self = made;
    return self;
}
REWRITE("Text::CopyLeftBCFF60", 0xbcff60, w2c_seh<0xe9d4ab>, (&w2c_text_left<0xbcf770>), &g_w2c_text_lefts);
REWRITE("Text::CopyLeftBCFF60", 0xbd0230, w2c_seh<0xe9d52b>, (&w2c_text_left<0xbcf9e0>), &g_w2c_text_lefts);

// ---- Text::FromField: the out holder gets a copy of the text at +OFF, or a new empty one if there is none --------------------------------------
static volatile long g_w2c_from_text;
template <uint32_t OFF> static uint32_t *__thiscall w2c_from_text(uint8_t *self, uint32_t *out) {
    g_w2c_from_text++;
    uint32_t source = w2c_u32(self, OFF), made;
    if (source) {
        void *holder = memory_alloc(0x10);
        made = holder ? ((uint32_t(__thiscall *)(void *, uint32_t))0xbcf7d0)(holder, source) : 0;
    } else {
        made = ((uint32_t(__fastcall *)(uint8_t))0xbd0490)(1);
    }
    *out = made;
    return out;
}
REWRITE("Text::FromField6AADE0", 0x6aade0, w2c_seh<0xe3ddbb>, (&w2c_from_text<0x11c>), &g_w2c_from_text);
REWRITE("Text::FromField6AADE0", 0xb073b0, w2c_seh<0xe8f4bb>, (&w2c_from_text<0xa4>), &g_w2c_from_text);

// ---- Lookup::ByPair: looks up a key made of two counted references in the table at +4; the key lives only for the call -----------------------
static volatile long g_w2c_by_pair;
template <uint32_t DROP> static uint32_t __thiscall w2c_by_pair(uint8_t *self, int32_t **a, int32_t **b) {
    g_w2c_by_pair++;
    int32_t *key[2] = {*a, *b};
    ++*key[0];
    ++*key[1];
    uint32_t found = ((uint32_t(__thiscall *)(void *, void *))0xbe82b0)(self + 4, key);
    w2c_run<DROP>(key);
    return found;
}
REWRITE("Lookup::ByPair661660", 0x661660, w2c_seh<0xe36498>, (&w2c_by_pair<0x6615d0>), &g_w2c_by_pair);
REWRITE("Lookup::ByPair661660", 0xb33a80, w2c_seh<0xe93208>, (&w2c_by_pair<0xb32d90>), &g_w2c_by_pair);

// ---- Ctor::Registered: joins `owner`, then settles as the final class with two empty counted texts -------------------------------------------
static volatile long g_w2c_registered;
template <uint32_t T0, uint32_t T1> static uint8_t *__thiscall w2c_registered(uint8_t *self, uint32_t value, void *owner) {
    g_w2c_registered++;
    w2c_u32(self, 4) = value;
    w2c_u32(self, 0) = 0xf75170;
    uint32_t held = value;
    uint8_t *me = self;
    ((void(__thiscall *)(void *, uint32_t *, uint8_t **))0x7fbf50)(owner, &held, &me);
    w2c_u32(self, 8) = 0xf75178;
    self[0xc] = 0;
    self[0xd] = 0;
    w2c_u32(self, 0) = T0;
    w2c_u32(self, 8) = T1;
    w2c_u32(self, 0x10) = ((uint32_t(__fastcall *)(uint8_t))0xbd0490)(1);
    w2c_u32(self, 0x14) = ((uint32_t(__fastcall *)(uint8_t))0xbd0490)(1);
    return self;
}
REWRITE("Ctor::Registered7FC400", 0x7fc400, w2c_seh<0xe5b293>, (&w2c_registered<0xf75180, 0xf7517c>), &g_w2c_registered);
REWRITE("Ctor::Registered7FC400", 0x7fc730, w2c_seh<0xe5b303>, (&w2c_registered<0xf75194, 0xf75190>), &g_w2c_registered);

// ---- Ctor::Grid: the base from two pairs of values, then 64 texts of 20 bytes from +0x128 --------------------------------------------------------
static volatile long g_w2c_grids;
template <uint32_t TABLE> static uint8_t *__thiscall w2c_grid(uint8_t *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    g_w2c_grids++;
    uint32_t first[2] = {a, b}, second[2] = {c, d};
    ((void(__thiscall *)(void *, uint32_t, void *, void *, void *))0x488290)(self, 0x40, self + 0x28, first, second);
    ((void(__stdcall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))0xddcf99)(self + 0x128, 0x14, 0x40, 0x42e030, 0x40f730);
    w2c_u32(self, 0) = TABLE;
    return self;
}
REWRITE("Ctor::Grid641750", 0x641750, w2c_seh<0xe31bd8>, (&w2c_grid<0xf49aa0>), &g_w2c_grids);
REWRITE("Ctor::Grid641750", 0x641950, w2c_seh<0xe31bf8>, (&w2c_grid<0xf49aa8>), &g_w2c_grids);

// ======== Fourth pass of wave 2: more counted-text makers behind an exception frame (left out here) ========
// A counted text made by MAKE from `source` in a fresh 16-byte block, or none when memory runs out.
template <uint32_t MAKE> static inline uint32_t w2d_text_from(uint32_t source) {
    void *holder = memory_alloc(0x10);
    return holder ? ((uint32_t(__thiscall *)(void *, uint32_t))MAKE)(holder, source) : 0;
}

// ---- Text::FromField: the text made from the source held at OFF, or the empty text when nothing is held ---------------------------------------
static volatile long g_w2d_from_field;
template <uint32_t OFF, uint32_t MAKE, uint32_t EMPTY> static uint32_t *__thiscall w2d_from_field(uint8_t *self, uint32_t *out) {
    g_w2d_from_field++;
    uint32_t source = w2c_u32(self, OFF);
    *out = source ? w2d_text_from<MAKE>(source) : ((uint32_t(__fastcall *)(uint8_t))EMPTY)(1);
    return out;
}
REWRITE("Text::FromField61CB70", 0x61cb70, w2c_seh<0xe2c02b>, (&w2d_from_field<0x48, 0xbcf7d0, 0xbd0490>), &g_w2d_from_field);
REWRITE("Text::FromField61CB70", 0xb0abc0, w2c_seh<0xe8fe4b>, (&w2d_from_field<0x30, 0xbcf850, 0xbd0490>), &g_w2d_from_field);

// ---- Text::Ctor: a text from a string, or the empty text for no string --------------------------------------------------------------------------
static volatile long g_w2d_text_ctor;
template <uint32_t MAKE, uint32_t EMPTY> static uint32_t *__thiscall w2d_text_ctor(uint32_t *self, uint32_t string) {
    g_w2d_text_ctor++;
    *self = string ? w2d_text_from<MAKE>(string) : ((uint32_t(__fastcall *)(uint8_t))EMPTY)(1);
    return self;
}
REWRITE("Text::Ctor401080", 0x401080, w2c_seh_late, (&w2d_text_ctor<0xbcf850, 0xbd0490>), &g_w2d_text_ctor);
REWRITE("Text::Ctor401080", 0x4028c0, w2c_seh_late, (&w2d_text_ctor<0xbcfb10, 0xbd0520>), &g_w2d_text_ctor);

// ---- Text::FromOwner: the owner at +0x8c gives its source at INNER; with no owner or no source, a fixed default string ----------------------------
static volatile long g_w2d_from_owner;
template <uint32_t INNER, uint32_t DEFAULT> static uint32_t *__thiscall w2d_from_owner(uint8_t *self, uint32_t *out) {
    g_w2d_from_owner++;
    uint8_t *owner = (uint8_t *)(uintptr_t)w2c_u32(self, 0x8c);
    uint32_t source = owner ? w2c_u32(owner, INNER) : 0;
    *out = source ? w2d_text_from<0xbcf7d0>(source) : w2d_text_from<0xbcf850>(DEFAULT);
    return out;
}
REWRITE("Text::FromOwner7312D0", 0x7312d0, w2c_seh<0xe4cab6>, (&w2d_from_owner<0x8, 0xf6a8e4>), &g_w2d_from_owner);
REWRITE("Text::FromOwner7312D0", 0x7395f0, w2c_seh<0xe4d6a6>, (&w2d_from_owner<0x4, 0xf6ab84>), &g_w2d_from_owner);

// ---- Text::NameOr: a new reference to the name the object at OFF keeps at +0xf0; with no object, a copy of a fixed string (made and let go) --------
static volatile long g_w2d_name_or;
template <uint32_t OFF> static int32_t **__thiscall w2d_name_or(uint8_t *self, int32_t **out, uint32_t) {
    g_w2d_name_or++;
    uint8_t *object = (uint8_t *)(uintptr_t)w2c_u32(self, OFF);
    if (object) {
        int32_t *name = *(int32_t **)(object + 0xf0);
        *out = name;
        ++*name;
        return out;
    }
    int32_t *made = (int32_t *)(uintptr_t)w2d_text_from<0xbcf850>(0xf264ad);
    *out = made;
    ++*made;
    if (--*made == 0) {   // the temporary's reference
        memory_free_aligned(*(void **)(made + 3));
        memory_free(made);
    }
    return out;
}
REWRITE("Text::NameOr99AD00", 0x99ad00, w2c_seh<0xe6f474>, (&w2d_name_or<0x4>), &g_w2d_name_or);
REWRITE("Text::NameOr99AD00", 0x99fdb0, w2c_seh<0xe6faf4>, (&w2d_name_or<0x28>), &g_w2d_name_or);

// ---- Reset::WithName: clears +0x14, then hands the outer object (0x40 before) a text of a fixed string ------------------------------------------
static volatile long g_w2d_reset_named;
template <uint32_t STRING> static void __fastcall w2d_reset_named(uint8_t *self) {
    g_w2d_reset_named++;
    w2c_u32(self, 0x14) = 0;
    uint32_t text = w2d_text_from<0xbcf850>(STRING);
    ((void(__thiscall *)(void *, uint32_t))0xba9700)(self - 0x40, text);
}
REWRITE("Reset::WithNameBAA100", 0xbaa100, w2c_seh<0xe9ba9b>, (&w2d_reset_named<0xf9d868>), &g_w2d_reset_named);
REWRITE("Reset::WithNameBAA100", 0xbaa180, w2c_seh<0xe9babb>, (&w2d_reset_named<0xf9d884>), &g_w2d_reset_named);

// ---- Add::Made: builds a 20-byte part from `what` (with 0, 1.0 and 0.1) and adds it under `key` --------------------------------------------------
static volatile long g_w2d_add_made;
template <uint32_t ADD> static uint32_t __thiscall w2d_add_made(void *self, uint32_t key, uint32_t what) {
    g_w2d_add_made++;
    void *holder = memory_alloc(0x14);
    uint32_t part = holder ? ((uint32_t(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))0xca8000)(holder, what, 0, 0x3f800000, 0x3dcccccd) : 0;
    return ((uint32_t(__thiscall *)(void *, uint32_t, uint32_t))ADD)(self, key, part);
}
REWRITE("Add::MadeC33D20", 0xc33d20, w2c_seh<0xea047b>, (&w2d_add_made<0xc33c00>), &g_w2d_add_made);
REWRITE("Add::MadeC33D20", 0xc34460, w2c_seh<0xea05bb>, (&w2d_add_made<0xc34340>), &g_w2d_add_made);

// ---- Text::SetBuilt: builds a temporary from `value` and the part at +8, stores its text at +0x3c, then lets the temporary go ----------------------
static volatile long g_w2d_set_built;
static void __thiscall w2d_set_built(uint8_t *self, uint32_t value) {
    g_w2d_set_built++;
    uint32_t temporary[7];   // 0x1c bytes, as the original's frame
    uint32_t text = ((uint32_t(__thiscall *)(void *, uint32_t, void *, uint32_t))0x7578e0)(temporary, value, self + 8, 0);
    ((void(__thiscall *)(void *, uint32_t))0xbd14e0)(self + 0x3c, text);
    ((void(__fastcall *)(void *))0xbd1880)(temporary);
}
REWRITE("Text::SetBuilt757AA0", 0x757aa0, w2c_seh<0xe501f8>, (&w2d_set_built), &g_w2d_set_built);
REWRITE("Text::SetBuilt757AA0", 0x7fa750, w2c_seh<0xe5b088>, (&w2d_set_built), &g_w2d_set_built);

// ---- Dtor::TwoTexts: drops the member texts at A and B (by the copy's own text destructor), then the base when there is one ------------------------
static volatile long g_w2d_two_texts;
template <uint32_t A, uint32_t B, uint32_t DROP, uint32_t BASE> static void __fastcall w2d_two_texts(uint8_t *self) {
    g_w2d_two_texts++;
    w2c_run<DROP>(self + A);
    w2c_run<DROP>(self + B);
    if (BASE) ((void(__fastcall *)(void *))BASE)(self);
}
REWRITE("Dtor::TwoTexts6E8280", 0x6e8280, w2c_seh<0xe45ed3>, (&w2d_two_texts<0x90, 0x7c, w2c_text_drop, 0x6d6320>), &g_w2d_two_texts);
REWRITE("Dtor::TwoTexts6E8280", 0xb22ff0, w2c_seh<0xe91db3>, (&w2d_two_texts<0x98, 0x7c, w2c_text_drop, 0xb7a4c0>), &g_w2d_two_texts);
REWRITE("Dtor::TwoTexts510710", 0x510710, w2c_seh<0xe1989e>, (&w2d_two_texts<0x154, 0x138, w2c_text_drop, 0>), &g_w2d_two_texts);
REWRITE("Dtor::TwoTexts510710", 0xc25690, w2c_seh<0xe9febe>, (&w2d_two_texts<0x140, 0x100, 0xc21320, 0>), &g_w2d_two_texts);

// ---- Dtor::SharedText: lets go of the counted text at OFF (freeing it with the last reference), then the base --------------------------------------
static volatile long g_w2d_shared_text;
template <uint32_t OFF, uint32_t BASE> static void __fastcall w2d_shared_text(uint8_t *self) {
    g_w2d_shared_text++;
    int32_t *text = *(int32_t **)(self + OFF);
    --*text;
    if (*text == 0 && text) {
        memory_free_aligned(*(void **)(text + 3));
        memory_free(text);
    }
    w2c_run<BASE>(self);
}
REWRITE("Dtor::SharedText478630", 0x478630, w2c_seh<0xe0d748>, (&w2d_shared_text<0x20, 0x478360>), &g_w2d_shared_text);
REWRITE("Dtor::SharedText478630", 0x63fea0, w2c_seh<0xe31858>, (&w2d_shared_text<0x14, 0x45b240>), &g_w2d_shared_text);

// ---- Dtor::ThreeFaces: back to this class's three tables (0, 4, 0x11c), its own teardown, the part at 0x11c, then the base ------------------------
static volatile long g_w2d_three_faces;
template <uint32_t T0, uint32_t T1, uint32_t T2, uint32_t TEARDOWN> static void __fastcall w2d_three_faces(uint8_t *self) {
    g_w2d_three_faces++;
    w2c_u32(self, 0) = T0;
    w2c_u32(self, 4) = T1;
    w2c_u32(self, 0x11c) = T2;
    w2c_run<TEARDOWN>(self);
    w2c_run<0x6835a0>(self + 0x11c);
    w2c_run<0x5f7b70>(self);
}
REWRITE("Dtor::ThreeFaces8E5D50", 0x8e5d50, w2c_seh<0xe68681>, (&w2d_three_faces<0xf7cb90, 0xf7cb78, 0xf7cb64, 0x8e5910>), &g_w2d_three_faces);
REWRITE("Dtor::ThreeFaces8E5D50", 0x9efac0, w2c_seh<0xe776d1>, (&w2d_three_faces<0xf864d8, 0xf864c0, 0xf864ac, 0x9eedb0>), &g_w2d_three_faces);

// ---- Dtor::Parts: the part at A, then the one at B (seen from a possibly-null object), then the base ------------------------------------------------
static volatile long g_w2d_parts;
template <uint32_t A, uint32_t FA, uint32_t B, uint32_t FB, uint32_t BASE> static void __fastcall w2d_parts(uint8_t *self) {
    g_w2d_parts++;
    w2c_run<FA>(self + A);
    w2c_run<FB>(self ? self + B : nullptr);
    w2c_run<BASE>(self);
}
REWRITE("Dtor::Parts6B1EB0", 0x6b1eb0, w2c_seh<0xe3e531>, (&w2d_parts<0xf4, 0x6af760, 0xd4, 0x6b0150, 0x6b0ea0>), &g_w2d_parts);
REWRITE("Dtor::Parts6B1EB0", 0x7ef990, w2c_seh<0xe5a161>, (&w2d_parts<0x174, 0x6a1df0, 0x164, 0xcd2b50, 0x6a28b0>), &g_w2d_parts);

// ---- Dtor::Handles: this class's table; tells the first handle to stop (its own slot), releases the second and forgets it; then the base ---------------
static volatile long g_w2d_handles;
template <uint32_t TABLE, uint32_t FIRST, uint32_t SLOT, uint32_t SECOND> static void __fastcall w2d_handles(uint8_t *self) {
    g_w2d_handles++;
    w2c_u32(self, 0) = TABLE;
    if (void **first = *(void ***)(self + FIRST)) ((void(__thiscall *)(void *))(*(void ***)first)[SLOT / 4])(first);
    if (void **second = *(void ***)(self + SECOND)) {
        ((void(__stdcall *)(void *))(*(void ***)second)[2])(second);
        w2c_u32(self, SECOND) = 0;
    }
    w2c_run<0xbfc840>(self);
}
REWRITE("Dtor::HandlesBFC920", 0xbfc920, w2c_seh<0xe9e848>, (&w2d_handles<0xfa5b68, 0x38, 0x4c, 0x3c>), &g_w2d_handles);
REWRITE("Dtor::HandlesBFC920", 0xbfc9c0, w2c_seh<0xe9e868>, (&w2d_handles<0xfa5bc8, 0x3c, 0x38, 0x40>), &g_w2d_handles);

// ---- Dtor::Owned: three tables, deletes the object owned at +0x44, the parts at 0x20 and 4, and ends as the shared base's table ------------------------
static volatile long g_w2d_owned;
template <uint32_t T0, uint32_t T1, uint32_t T2> static void __fastcall w2d_owned(uint8_t *self) {
    g_w2d_owned++;
    w2c_u32(self, 0) = T0;
    w2c_u32(self, 4) = T1;
    w2c_u32(self, 0x20) = T2;
    if (void **owned = *(void ***)(self + 0x44)) ((void(__thiscall *)(void *, uint32_t))(*(void ***)owned)[0])(owned, 1);
    w2c_run<0x6b0150>(self + 0x20);
    w2c_run<0x6a1df0>(self + 4);
    w2c_u32(self, 0) = 0xf2694c;
}
REWRITE("Dtor::Owned63AB90", 0x63ab90, w2c_seh<0xe30f46>, (&w2d_owned<0xf49148, 0xf49134, 0xf49108>), &g_w2d_owned);
REWRITE("Dtor::Owned63AB90", 0x86e450, w2c_seh<0xe624c6>, (&w2d_owned<0xf78500, 0xf784ec, 0xf784c0>), &g_w2d_owned);

// ---- Array::Remake: frees the old elements, makes `count` new ones of SIZE bytes with MAKE (none for zero or no memory), keeps the count -----------
static volatile long g_w2d_remake;
template <uint32_t SIZE, uint32_t MAKE> static uint32_t __thiscall w2d_remake(uint32_t *self, uint32_t count) {
    g_w2d_remake++;
    if (self[0]) {
        memory_free_aligned((void *)(uintptr_t)self[0]);
        self[0] = 0;
    }
    if (count) {
        void *block = memory_alloc_aligned(count * SIZE);
        if (block) ((void(__stdcall *)(void *, uint32_t, uint32_t, uint32_t))0x402b90)(block, SIZE, count, MAKE);
        self[0] = (uint32_t)(uintptr_t)block;
    }
    self[1] = count;
    return count;
}
REWRITE("Array::Remake967360", 0x967360, w2c_seh_late, (&w2d_remake<0x18, 0x966ea0>), &g_w2d_remake);
REWRITE("Array::Remake967360", 0xafd320, w2c_seh_late, (&w2d_remake<0x30, 0xaf9f20>), &g_w2d_remake);

// ---- Release::Five: lets go of the five parts held at O1, 0xf0, 0xf4, O4 and O5 (the first three by FA, the last two by FB) -----------------------
static volatile long g_w2d_release_five;
static const uint8_t w2d_release_five_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x8e};
template <uint32_t O1, uint32_t O4, uint32_t O5, uint32_t FA, uint32_t FB> static void __fastcall w2d_release_five(uint8_t *self) {
    g_w2d_release_five++;
    w2c_run<FA>((void *)(uintptr_t)w2c_u32(self, O1));
    w2c_run<FA>((void *)(uintptr_t)w2c_u32(self, 0xf0));
    w2c_run<FA>((void *)(uintptr_t)w2c_u32(self, 0xf4));
    w2c_run<FB>((void *)(uintptr_t)w2c_u32(self, O4));
    w2c_run<FB>((void *)(uintptr_t)w2c_u32(self, O5));
}
REWRITE("Release::Five501E90", 0x501e90, w2d_release_five_original, (&w2d_release_five<0xe8, 0xfc, 0xf8, 0x6b0990, 0x6b0990>), &g_w2d_release_five);
REWRITE("Release::Five501E90", 0x501ed0, w2d_release_five_original, (&w2d_release_five<0xe8, 0xfc, 0xf8, 0x6afc20, 0x6afc20>), &g_w2d_release_five);
REWRITE("Release::Five501E90", 0x63a8a0, w2d_release_five_original, (&w2d_release_five<0xec, 0xe4, 0xe8, 0x6b0990, 0x6afc20>), &g_w2d_release_five);

// ---- Dtor::TextAndPart: lets go of the counted text at TEXT, the part at PART (seen from a possibly-null object), then the base ------------------------
static volatile long g_w2d_text_part;
template <uint32_t TEXT, uint32_t PART, uint32_t FPART, uint32_t BASE> static void __fastcall w2d_text_part(uint8_t *self) {
    g_w2d_text_part++;
    int32_t *text = *(int32_t **)(self + TEXT);
    --*text;
    if (*text == 0 && text) {
        memory_free_aligned(*(void **)(text + 3));
        memory_free(text);
    }
    w2c_run<FPART>(self ? self + PART : nullptr);
    w2c_run<BASE>(self);
}
REWRITE("Dtor::TextAndPart45C1C0", 0x45c1c0, w2c_seh<0xe0b4a1>, (&w2d_text_part<0x2f8, 0x2e8, 0x8002c0, 0x68a3d0>), &g_w2d_text_part);
REWRITE("Dtor::TextAndPart45C1C0", 0x634f10, w2c_seh<0xe2feb1>, (&w2d_text_part<0x150, 0x13c, 0x4535f0, 0x67b9f0>), &g_w2d_text_part);

// ---- Text::ItemOr: a new reference to item `index` of the list (count at COUNT, items at ITEMS); past the end, a copy of a fixed string ---------------
static volatile long g_w2d_item_or;
template <uint32_t COUNT, uint32_t ITEMS> static int32_t **__thiscall w2d_item_or(uint8_t *self, int32_t **out, uint32_t index) {
    g_w2d_item_or++;
    if (index < w2c_u32(self, COUNT)) {
        int32_t *item = (*(int32_t ***)(self + ITEMS))[index];
        *out = item;
        ++*item;
        return out;
    }
    int32_t *made = (int32_t *)(uintptr_t)w2d_text_from<0xbcfb10>(0xf264ad);
    *out = made;
    ++*made;
    if (--*made == 0) {   // the temporary's reference
        memory_free_aligned(*(void **)(made + 3));
        memory_free(made);
    }
    return out;
}
REWRITE("Text::ItemOrC3C610", 0xc3c610, w2c_seh<0xea1234>, (&w2d_item_or<0x58, 0x5c>), &g_w2d_item_or);
REWRITE("Text::ItemOrC3C610", 0xc3c6d0, w2c_seh<0xea1264>, (&w2d_item_or<0x50, 0x54>), &g_w2d_item_or);

// ---- Look::Pair: the two bytes the look gives (0x120, 0x121) turned into three values, of kind ONE when +0xc0 is 1 and OTHER otherwise --------------
static volatile long g_w2d_look_pair;
template <uint32_t ONE, uint32_t OTHER> static uint32_t *__thiscall w2d_look_pair(uint8_t *self, uint32_t *out) {
    g_w2d_look_pair++;
    auto look = (uint8_t *(__fastcall *)(void *))0x4c5890;
    uint32_t kind = w2c_u32(self, 0xc0) == 1 ? ONE : OTHER;
    uint8_t second = look(self)[0x121];
    uint8_t first = look(self)[0x120];
    uint32_t made[4];
    uint32_t *value = ((uint32_t * (__fastcall *)(void *, uint32_t, uint32_t, uint32_t))0xa87e00)(made, kind, first, second);
    out[0] = value[0];
    out[1] = value[1];
    out[2] = value[2];
    return out;
}
static const uint8_t w2d_look_pair_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x28};
REWRITE("Look::Pair4C5E10", 0x4c5e10, w2d_look_pair_original, (&w2d_look_pair<9, 3>), &g_w2d_look_pair);
REWRITE("Look::Pair4C5E10", 0x4c5ea0, w2d_look_pair_original, (&w2d_look_pair<8, 4>), &g_w2d_look_pair);

// ---- Dtor::OwnedPart: three tables, deletes the part at PART and the object owned at OWNED (forgetting both), the parts at 0x20 and 4, then the shared base's table
static volatile long g_w2d_owned_part;
template <uint32_t T0, uint32_t T1, uint32_t T2, uint32_t PART, uint32_t OWNED> static void __fastcall w2d_owned_part(uint8_t *self) {
    g_w2d_owned_part++;
    w2c_u32(self, 0) = T0;
    w2c_u32(self, 4) = T1;
    w2c_u32(self, 0x20) = T2;
    if (void *part = *(void **)(self + PART)) {
        w2c_run<0x6af760>(part);
        memory_free(part);
    }
    w2c_u32(self, PART) = 0;
    if (void **owned = *(void ***)(self + OWNED)) ((void(__thiscall *)(void *, uint32_t))(*(void ***)owned)[0])(owned, 1);
    w2c_u32(self, OWNED) = 0;
    w2c_run<0x6b0150>(self + 0x20);
    w2c_run<0x6a1df0>(self + 4);
    w2c_u32(self, 0) = 0xf2694c;
}
REWRITE("Dtor::OwnedPart63A2A0", 0x63a2a0, w2c_seh<0xe30de6>, (&w2d_owned_part<0xf48e7c, 0xf48e68, 0xf48e3c, 0x40, 0x44>), &g_w2d_owned_part);
REWRITE("Dtor::OwnedPart63A2A0", 0x7462b0, w2c_seh<0xe4e646>, (&w2d_owned_part<0xf6b230, 0xf6b21c, 0xf6b1f0, 0x44, 0x40>), &g_w2d_owned_part);

// ---- Grid::Mark: sets the flag byte FLAG in every 0x34-byte cell of the clamped rectangle [x0,x1) x [y0,y1), and widens the box kept at BOX ----------------
static volatile long g_w2d_grid_mark;
static const uint8_t w2d_grid_mark_original[] = {0x55, 0x8b, 0x6c, 0x24, 0x08, 0x85, 0xed};
template <uint32_t FLAG, uint32_t BOX> static void __thiscall w2d_grid_mark(uint8_t *self, int32_t x0, int32_t y0, int32_t x1, int32_t y1) {
    g_w2d_grid_mark++;
    int32_t width = (int32_t)w2c_u32(self, 0x48);
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > width) x1 = width;
    if (y1 > (int32_t)w2c_u32(self, 0x4c)) y1 = (int32_t)w2c_u32(self, 0x4c);
    if (x0 >= x1 || y0 >= y1) return;
    for (int32_t y = y0; y < y1; y++)
        for (int32_t x = x0; x < x1; x++) {
            uint8_t *cells = (uint8_t *)(uintptr_t)w2c_u32(self, 0x50);
            cells[((int32_t)w2c_u32(self, 0x48) * y + x) * 0x34 + FLAG] = 1;
        }
    int32_t *box = (int32_t *)(self + BOX);
    if (x0 < box[0]) box[0] = x0;
    if (y0 < box[1]) box[1] = y0;
    if (x1 > box[2]) box[2] = x1;
    if (y1 > box[3]) box[3] = y1;
}
REWRITE("Grid::Mark874D90", 0x874d90, w2d_grid_mark_original, (&w2d_grid_mark<2, 0x64>), &g_w2d_grid_mark);
REWRITE("Grid::Mark874D90", 0x874e40, w2d_grid_mark_original, (&w2d_grid_mark<1, 0x54>), &g_w2d_grid_mark);

// ---- Stat::Get: statistic `which`: five plain fields (0 and 3 the same one), or entry `which - 6` of the table when it is in use and in range; else 0 -----
static volatile long g_w2d_stat_get;
static const uint8_t w2d_stat_get_original[] = {0x8b, 0x44, 0x24, 0x04, 0x83, 0xf8, 0x05, 0x77, 0x2b};
template <uint32_t A, uint32_t B, uint32_t C, uint32_t D, uint32_t E, uint32_t ENTRY> static uint32_t __thiscall w2d_stat_get(uint8_t *self, int32_t which) {
    g_w2d_stat_get++;
    switch (which) {
    case 0:
    case 3: return w2c_u32(self, A);
    case 1: return w2c_u32(self, B);
    case 2: return w2c_u32(self, C);
    case 4: return w2c_u32(self, D);
    case 5: return w2c_u32(self, E);
    }
    if (which < 6 || which > 0x19) return 0;
    int32_t index = which - 6;
    int32_t used = (w2c_u32(self, 0x18) & 0x400000) && (int32_t)w2c_u32(self, 0xe4) >= 2 ? (int32_t)w2c_u32(self, 0x11c) : 0;
    if (index >= used) return 0;
    return w2c_u32((void *)(uintptr_t)w2c_u32(self, 0x120), index * 12 + ENTRY);
}
REWRITE("Stat::GetBC5AB0", 0xbc5ab0, w2d_stat_get_original, (&w2d_stat_get<0x20, 0x28, 0x30, 0xd8, 0x10c, 4>), &g_w2d_stat_get);
REWRITE("Stat::GetBC5AB0", 0xbc5b40, w2d_stat_get_original, (&w2d_stat_get<0x24, 0x2c, 0x34, 0xdc, 0x110, 8>), &g_w2d_stat_get);

// ---- Pool::Release: marks the block free and merges it with a free neighbour on each side; merged-away headers go on the spare list (FREE, COUNT) --------
// Block: +0 spare link, +8 in use, +0x10 size, +0x14 previous, +0x18 next. The pool keeps its largest free size at +0x1c.
static volatile long g_w2d_pool_release;
static const uint8_t w2d_pool_release_original[] = {0x8b, 0x44, 0x24, 0x04, 0x56, 0x8b, 0x70, 0x14};
template <uint32_t FREE, uint32_t COUNT> static void __thiscall w2d_pool_release(uint8_t *self, uint8_t *block) {
    g_w2d_pool_release++;
    auto field = [](uint8_t *b, uint32_t off) -> uint8_t *& { return *(uint8_t **)(b + off); };
    auto spare = [](uint8_t *b) {
        *(uint32_t *)b = *(uint32_t *)(uintptr_t)FREE;
        *(uint8_t **)(uintptr_t)FREE = b;
        ++*(uint32_t *)(uintptr_t)COUNT;
    };
    w2c_u32(block, 8) = 0;
    uint8_t *kept = block;
    uint8_t *before = field(block, 0x14);
    if (before && !w2c_u32(before, 8)) {
        w2c_u32(before, 0x10) += w2c_u32(block, 0x10);
        field(before, 0x18) = field(block, 0x18);
        if (uint8_t *after = field(block, 0x18)) field(after, 0x14) = before;
        kept = before;
        spare(block);
    }
    uint8_t *after = field(kept, 0x18);
    if (after && !w2c_u32(after, 8)) {
        w2c_u32(kept, 0x10) += w2c_u32(after, 0x10);
        field(kept, 0x18) = field(after, 0x18);
        spare(after);
        if (uint8_t *next = field(kept, 0x18)) field(next, 0x14) = kept;
    }
    if ((int32_t)w2c_u32(kept, 0x10) > (int32_t)w2c_u32(self, 0x1c)) w2c_u32(self, 0x1c) = w2c_u32(kept, 0x10);
}
REWRITE("Pool::ReleaseBE9590", 0xbe9590, w2d_pool_release_original, (&w2d_pool_release<0x13d0f4c, 0x13d0f54>), &g_w2d_pool_release);
REWRITE("Pool::ReleaseBE9590", 0xbfd270, w2d_pool_release_original, (&w2d_pool_release<0x13d2a0c, 0x13d2a14>), &g_w2d_pool_release);

// Takes a link {previous, next} out of its ring; a link not properly in a ring stops at a breakpoint, as the original does.
static inline void w2d_unlink(uint32_t *link) {
    if (!link) __builtin_debugtrap();
    uint32_t *previous = (uint32_t *)(uintptr_t)link[0], *next = (uint32_t *)(uintptr_t)link[1];
    if (!next || !previous || previous[1] != (uint32_t)(uintptr_t)link || next[0] != (uint32_t)(uintptr_t)link) __builtin_debugtrap();
    previous[1] = link[1];
    next[0] = link[0];
    link[0] = 0;
    link[1] = 0;
}

// ---- Dtor::Linked: this class's table, out of its ring (link at LINK), frees the shared block at SHARED once it is empty, lets go of the text at TEXT --------
static volatile long g_w2d_linked;
template <uint32_t TABLE, uint32_t LINK, uint32_t SHARED, uint32_t TEXT> static void __fastcall w2d_linked(uint8_t *self) {
    g_w2d_linked++;
    w2c_u32(self, 0) = TABLE;
    w2d_unlink((uint32_t *)(self + LINK));
    uint32_t **shared = (uint32_t **)(uintptr_t)SHARED;
    if (**(uint32_t **)*shared == 0) {
        memory_free(*shared);
        *shared = nullptr;
    }
    int32_t *text = *(int32_t **)(self + TEXT);
    --*text;
    if (*text == 0 && text) {
        memory_free_aligned(*(void **)(text + 3));
        memory_free(text);
    }
}
REWRITE("Dtor::LinkedC18490", 0xc18490, w2c_seh<0xe9fa3b>, (&w2d_linked<0xfafc88, 8, 0x13e33a8, 4>), &g_w2d_linked);
REWRITE("Dtor::LinkedC18490", 0xc18640, w2c_seh<0xe9fa7b>, (&w2d_linked<0xfafc8c, 8, 0x13e33a4, 4>), &g_w2d_linked);
REWRITE("Dtor::LinkedC18490", 0xc434f0, w2c_seh<0xea16fb>, (&w2d_linked<0xfb12a4, 4, 0x13e6b50, 0x10>), &g_w2d_linked);

// ---- Dtor::Bound: this class's table; settles first when ACTIVE is set; deletes the bound object (out of its ring first), leaves its own ring, base table ---
static volatile long g_w2d_bound;
template <uint32_t T0, uint32_t ACTIVE, uint32_t SETTLE, uint32_t HOLDER, uint32_t LINK, uint32_t T1> static void __fastcall w2d_bound(uint8_t *self) {
    g_w2d_bound++;
    w2c_u32(self, 0) = T0;
    if (w2c_u32(self, ACTIVE)) w2c_run<SETTLE>(self);
    if (uint8_t *holder = (uint8_t *)(uintptr_t)w2c_u32(self, HOLDER)) {
        uint8_t *bound = (uint8_t *)(uintptr_t)w2c_u32(holder, 4);
        w2d_unlink(bound ? (uint32_t *)(bound + 0x10) : nullptr);
        if (bound) ((void(__thiscall *)(void *, uint32_t))(*(void ***)bound)[0])(bound, 1);
    }
    w2d_unlink((uint32_t *)(self + LINK));
    w2c_u32(self, 0) = T1;
}
REWRITE("Dtor::BoundBEBF70", 0xbebf70, w2c_seh<0xe9e5b8>, (&w2d_bound<0xfa57d0, 0x20, 0xbe9dd0, 0x18, 0x24, 0xfa5718>), &g_w2d_bound);
REWRITE("Dtor::BoundBEBF70", 0xbff950, w2c_seh<0xe9ea68>, (&w2d_bound<0xfa5d28, 0x24, 0xbfdbc0, 0x20, 0x28, 0xfa5c8c>), &g_w2d_bound);

// Lets go of one reference to a counted text, freeing it with the last (the copy at hand checks for none first).
template <bool CHECK> static inline void w2d_text_release(int32_t *text) {
    --*text;
    if (*text == 0 && (!CHECK || text)) {
        memory_free_aligned(*(void **)(text + 3));
        memory_free(text);
    }
}

// ---- Text::Refresh: asks the source at HOLDER (by FIND) for its current text and keeps it at FIELD when it changed; the asked-for reference is let go -----
static volatile long g_w2d_refresh;
template <uint32_t HOLDER, uint32_t FIND, uint32_t FIELD> static void __thiscall w2d_refresh(uint8_t *self, uint32_t) {
    g_w2d_refresh++;
    int32_t *asked;
    int32_t **current = ((int32_t * *(__thiscall *)(void *, int32_t **))FIND)((void *)(uintptr_t)w2c_u32(self, HOLDER), &asked);
    int32_t *&kept = *(int32_t **)(self + FIELD);
    if (*current != kept) {
        w2d_text_release<true>(kept);
        kept = *current;
        ++*kept;
    }
    w2d_text_release<false>(asked);
}
REWRITE("Text::Refresh64A320", 0x64a320, w2c_seh<0xe32cf8>, (&w2d_refresh<0x30, 0x7c8700, 0x28>), &g_w2d_refresh);
REWRITE("Text::Refresh64A320", 0x7ee540, w2c_seh<0xe59de8>, (&w2d_refresh<0x38, 0x7c8700, 0x30>), &g_w2d_refresh);
REWRITE("Text::Refresh64A320", 0xa35ee0, w2c_seh<0xe7d698>, (&w2d_refresh<0x2c, 0xa30980, 0x4c>), &g_w2d_refresh);

// ---- Dtor::Queues: deletes everything left in the first queue (+0xc); takes each entry out of the second (+0x1c) and frees it; the shared text at +0x18 -----
static volatile long g_w2d_queues;
template <uint32_t T0, uint32_t T1, uint32_t DROP> static void __fastcall w2d_queues(uint8_t *self) {
    g_w2d_queues++;
    auto next = (uint8_t * (__fastcall *)(void *))0xbe8400;
    w2c_u32(self, 0) = T0;
    w2c_u32(self, 8) = T1;
    while (void **object = (void **)next(self + 0xc)) ((void(__thiscall *)(void *, uint32_t))(*(void ***)object)[0])(object, 1);
    while (uint8_t *entry = next(self + 0x1c)) {
        ((void(__thiscall *)(void *, void *, uint32_t))0xbe8230)(self + 0x1c, entry + 0xc, 0);
        w2c_run<DROP>(entry + 0xc);
        memory_free(entry);
    }
    w2c_run<0xbe88b0>(self + 0x1c);
    w2d_text_release<true>(*(int32_t **)(self + 0x18));
    w2c_run<0xbe88b0>(self + 0xc);
    w2c_u32(self, 0) = 0xf306b8;
}
REWRITE("Dtor::Queues4A71F0", 0x4a71f0, w2c_seh<0xe11069>, (&w2d_queues<0xf30900, 0xf308fc, 0x4a35b0>), &g_w2d_queues);
REWRITE("Dtor::Queues4A71F0", 0xb943f0, w2c_seh<0xe99e49>, (&w2d_queues<0xf9c7dc, 0xf9c7d8, 0xb92bb0>), &g_w2d_queues);

// ---- Array::Resize: a new block of `count` SIZE-byte elements made by MAKE, the first ones copied over by COPY, the old block freed (not destroyed) -------
static volatile long g_w2d_resize;
template <uint32_t SIZE, uint32_t MAKE, uint32_t COPY> static uint32_t __thiscall w2d_resize(uint8_t **self, int32_t count) {
    g_w2d_resize++;
    if ((uint32_t)count == (uint32_t)(uintptr_t)self[1]) return (uint32_t)count;
    uint8_t *block = nullptr;
    if (count) {
        block = (uint8_t *)memory_alloc_aligned(count * SIZE);
        if (block) ((void(__stdcall *)(void *, uint32_t, uint32_t, uint32_t))0x402b90)(block, SIZE, count, MAKE);
        int32_t old = (int32_t)(uintptr_t)self[1];
        int32_t keep = count > old ? old : count;
        for (int32_t i = 0; i < keep; i++) ((void(__thiscall *)(void *, void *))COPY)(block + i * SIZE, self[0] + i * SIZE);
    }
    if (self[0]) {
        memory_free_aligned(self[0]);
        self[0] = nullptr;
    }
    self[0] = block;
    self[1] = (uint8_t *)(uintptr_t)count;
    return (uint32_t)count;
}
REWRITE("Array::Resize430690", 0x430690, w2c_seh_late, (&w2d_resize<0x70, 0x42f2f0, 0x42f9b0>), &g_w2d_resize);
REWRITE("Array::Resize430690", 0x9e1ab0, w2c_seh_late, (&w2d_resize<0x44, 0x9e1330, 0x9e1340>), &g_w2d_resize);
