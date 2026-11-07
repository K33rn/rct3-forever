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
