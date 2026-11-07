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
