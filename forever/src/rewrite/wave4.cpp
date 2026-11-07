// Rewrites of wave 4.
#include "wave.h"

// ---- Batch::Push: adds a value to a 64-value batch, and hands the full batch to the object's first virtual -------------------
static volatile long g_batch_pushes;
static uint32_t __thiscall batch_push(uint32_t *self, const uint32_t *value, uint32_t, uint32_t, uint32_t) {
    g_batch_pushes++;
    self[1 + self[0x41]] = *value;
    uint32_t count = ++self[0x41];
    if (count != 0x40) return count;
    uint32_t result = ((uint32_t(__thiscall *)(void *, void *, uint32_t))(*(void ***)self)[0])(self, self + 1, count);
    self[0x41] = 0;
    return result;
}
static const uint8_t batch_push_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x86, 0x04, 0x01, 0x00};
REWRITE("Batch::Push7B39C0", 0x7b39c0, batch_push_original, (&batch_push), &g_batch_pushes);
REWRITE("Batch::Push7B39C0", 0xb5bec0, batch_push_original, (&batch_push), &g_batch_pushes);
REWRITE("Batch::Push7B39C0", 0xb6c240, batch_push_original, (&batch_push), &g_batch_pushes);

// ---- Linked::Destroy: a destructor that leaves the list its node at +OFF is in, then runs the base destructor -----------------
static volatile long g_linked_destroys;
template <uint32_t TABLE, uint32_t OFF> static void __fastcall linked_destroy(uint8_t *self) {
    g_linked_destroys++;
    *(uint32_t *)self = TABLE;
    unlink((Node *)(self + OFF));
    ((void(__fastcall *)(void *))0x6a1df0)(self);
}
template <uint32_t TABLE, uint32_t OFF> static const uint8_t linked_destroy_original[] = {
    0x8d, 0x41, (uint8_t)OFF, 0x85, 0xc0, 0x56, 0xc7, 0x01, (uint8_t)TABLE, (uint8_t)(TABLE >> 8), (uint8_t)(TABLE >> 16)};
REWRITE("Linked::Destroy633860", 0x633860, (linked_destroy_original<0xf47f0c, 0x1c>), (&linked_destroy<0xf47f0c, 0x1c>), &g_linked_destroys);
REWRITE("Linked::Destroy633860", 0x633c40, (linked_destroy_original<0xf47f0c, 0x1c>), (&linked_destroy<0xf47f0c, 0x1c>), &g_linked_destroys);
REWRITE("Linked::Destroy633860", 0x65f520, (linked_destroy_original<0xf4de64, 0x20>), (&linked_destroy<0xf4de64, 0x20>), &g_linked_destroys);

// ---- Reference::Make: fills a reference to the handle at +OFF, counted and added first to the handle's list ----------------
static volatile long g_reference_makes;
template <uint32_t OFF> static uint32_t *__thiscall reference_make(uint8_t *self, uint32_t *out) {
    g_reference_makes++;
    out[0] = 0xf26ae0;
    out[4] = (uint32_t)(uintptr_t)out;
    uint8_t *handle = *(uint8_t **)(self + OFF);
    ++*(int32_t *)(handle + 8);
    out[1] = (uint32_t)(uintptr_t)handle;
    Node *head = *(Node **)(handle + 0x20);
    Node *node = (Node *)(out + 2);
    node->next = head->next;
    head->next->prev = node;
    node->prev = head;
    head->next = node;
    return out;
}
static const uint8_t reference_make_original[] = {0x51, 0x8b, 0x44, 0x24, 0x08, 0xc7, 0x00, 0xe0, 0x6a, 0xf2, 0x00};
REWRITE("Reference::Make4FF6F0", 0x4ff6f0, reference_make_original, (&reference_make<0x18>), &g_reference_makes);
REWRITE("Reference::Make4FF6F0", 0x7f4a40, reference_make_original, (&reference_make<0x48>), &g_reference_makes);
REWRITE("Reference::Make4FF6F0", 0x92ecd0, reference_make_original, (&reference_make<0x44>), &g_reference_makes);

// ---- Rows::Copy: copies a 3x3 matrix of rows 16 bytes apart, leaving each row's fourth value alone --------------------------
static volatile long g_rows_copies;
static uint32_t *__thiscall rows_copy(uint32_t *self, const uint32_t *from) {
    g_rows_copies++;
    for (int row = 0; row < 12; row += 4)
        for (int i = 0; i < 3; i++) self[row + i] = from[row + i];
    return self;
}
static const uint8_t rows_copy_original[] = {0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x04, 0x8b, 0x11};
REWRITE("Rows::Copy8E62B0", 0x8e62b0, rows_copy_original, (&rows_copy), &g_rows_copies);
REWRITE("Rows::Copy8E62B0", 0x8e62f0, rows_copy_original, (&rows_copy), &g_rows_copies);

// ---- Array::GrowTo: raises the count {+4 count, +8 capacity, +0xc growth}, making room when needed; never lowers it --------
static volatile long g_array_grows;
template <uint32_t RESERVE> static void __thiscall array_grow_to(uint8_t *self, int32_t count) {
    g_array_grows++;
    if (count <= *(int32_t *)(self + 4)) return;
    if (count > *(int32_t *)(self + 8)) {
        int32_t growth = *(int32_t *)(self + 0xc);
        ((void(__thiscall *)(void *, int32_t))RESERVE)(self, (growth ? growth : count) + count);
    }
    *(int32_t *)(self + 4) = count;
}
static const uint8_t array_grow_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x04, 0x57, 0x8b, 0x7c, 0x24, 0x0c};
REWRITE("Array::GrowTo6659A0", 0x6659a0, array_grow_original, (&array_grow_to<0x665780>), &g_array_grows);
REWRITE("Array::GrowTo6659A0", 0x6659d0, array_grow_original, (&array_grow_to<0x665850>), &g_array_grows);
REWRITE("Array::GrowTo6659A0", 0x665a00, array_grow_original, (&array_grow_to<0x665920>), &g_array_grows);
REWRITE("Array::GrowTo6659A0", 0xafef30, array_grow_original, (&array_grow_to<0xafeeb0>), &g_array_grows);

// ---- Vectors::Clear: zeroes `count` vectors (16 bytes apart, the fourth value left), given as the last of 4-6 stack arguments --
static volatile long g_vector_clears;
static inline void vectors_clear(int32_t count, uint32_t *vectors) {
    g_vector_clears++;
    for (; count > 0; count--, vectors += 4) vectors[0] = vectors[1] = vectors[2] = 0;
}
static void __fastcall vectors_clear4(int32_t count, void *, uint32_t, uint32_t, uint32_t, uint32_t *vectors) { vectors_clear(count, vectors); }
static void __fastcall vectors_clear5(int32_t count, void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t *vectors) { vectors_clear(count, vectors); }
static void __fastcall vectors_clear6(int32_t count, void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t *vectors) { vectors_clear(count, vectors); }
static const uint8_t vectors_clear_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x0c, 0x8b, 0x55};
REWRITE("Vectors::ClearCCF0D0", 0xccf0d0, vectors_clear_original, (&vectors_clear6), &g_vector_clears);
REWRITE("Vectors::ClearCCF0D0", 0xccf760, vectors_clear_original, (&vectors_clear4), &g_vector_clears);
REWRITE("Vectors::ClearCCF0D0", 0xcd0240, vectors_clear_original, (&vectors_clear5), &g_vector_clears);

// ---- Screen::IsCurrent: whether the current screen (a global) is the part at +0xc of the object in the global at GLOBAL --------
static volatile long g_screen_checks;
template <uint32_t GLOBAL> static uint8_t __cdecl screen_is_current() {
    g_screen_checks++;
    uint32_t object = *(uint32_t *)GLOBAL;
    return *(uint32_t *)0x12f503c == (object ? object + 0xc : 0);
}
template <uint32_t GLOBAL> static const uint8_t screen_current_original[] = {
    0xa1, (uint8_t)GLOBAL, (uint8_t)(GLOBAL >> 8), (uint8_t)(GLOBAL >> 16), (uint8_t)(GLOBAL >> 24)};
REWRITE("Screen::IsCurrent6245E0", 0x6245e0, screen_current_original<0x13ce688>, (&screen_is_current<0x13ce688>), &g_screen_checks);
REWRITE("Screen::IsCurrent6245E0", 0x624610, screen_current_original<0x1306ca4>, (&screen_is_current<0x1306ca4>), &g_screen_checks);

// ---- Fields::SavedSize: 8 bytes of header, then each field's size for each of the rows of a table at +0x28 --------------------
static volatile long g_field_sizes;
static uint32_t __thiscall fields_saved_size(uint8_t *self, int32_t table) {
    g_field_sizes++;
    uint8_t *entry = *(uint8_t **)(self + 0x28) + table;
    int32_t rows = *(int32_t *)(entry + 4);
    uint32_t size = 8;
    for (int32_t row = 0; row < rows; row++) {
        uint8_t *data = *(uint8_t **)entry + row;
        for (int32_t i = 0; i < *(int32_t *)(self + 0x18); i++) {
            void *field = (*(void ***)(self + 0xc))[i];
            size += ((uint32_t(__thiscall *)(void *, void *))(*(void ***)field)[2])(field, data);
        }
    }
    return size;
}
static const uint8_t fields_size_original[] = {0x83, 0xec, 0x08, 0x53, 0x55, 0x8b, 0xd9, 0x8b, 0x4b, 0x28};
REWRITE("Fields::SavedSize5BA7A0", 0x5ba7a0, fields_size_original, (&fields_saved_size), &g_field_sizes);
REWRITE("Fields::SavedSize5BA7A0", 0xa01e00, fields_size_original, (&fields_saved_size), &g_field_sizes);

// ---- Items::At: an item (224 bytes each) of the list in the interface kept by the reference at +0x204 -----------------------
static volatile long g_items_at;
static uint8_t *__thiscall items_at(uint8_t *self, int32_t index) {
    g_items_at++;
    uint8_t *found = (uint8_t *)ref_interface<0x12f5338>(self + 0x204);
    return *(uint8_t **)(found + 0x40) + index * 0xe0;
}
static const uint8_t items_at_original[] = {0x57, 0x8b, 0xf9, 0x8a, 0x87, 0x1c, 0x02, 0x00};
REWRITE("Items::At532DC0", 0x532dc0, items_at_original, (&items_at), &g_items_at);
REWRITE("Items::At532DC0", 0x532e30, items_at_original, (&items_at), &g_items_at);

// ---- Refs::Selected: the interface of the selected reference of an array, none when nothing is selected ----------------------
static volatile long g_refs_selected;
static void *__fastcall refs_selected(uint8_t *self) {
    g_refs_selected++;
    int32_t selected = *(int32_t *)(self + 0x78);
    if (selected <= -1) return nullptr;
    return ref_interface<0x13d0058>(*(uint8_t **)(self + 0x7c) + selected * 0x1c);
}
static const uint8_t refs_selected_original[] = {0x8b, 0x41, 0x78, 0x83, 0xf8, 0xff, 0x7f, 0x03};
REWRITE("Refs::SelectedB44DA0", 0xb44da0, refs_selected_original, (&refs_selected), &g_refs_selected);
REWRITE("Refs::SelectedB44DA0", 0xb44e10, refs_selected_original, (&refs_selected), &g_refs_selected);

// ---- Children::Any: whether any child passes a check ------------------------------------------------------------------------
static volatile long g_children_any;
template <uint32_t CHECK> static uint8_t __fastcall children_any(uint8_t *self) {
    g_children_any++;
    for (int32_t i = 0; i < *(int32_t *)(self + 0xd0); i++)
        if (((uint8_t(__fastcall *)(void *))CHECK)((*(void ***)(self + 0xcc))[i])) return 1;
    return 0;
}
static const uint8_t children_any_original[] = {0x56, 0x57, 0x8b, 0xf9, 0x8b, 0x87, 0xd0, 0x00};
REWRITE("Children::Any9C6950", 0x9c6950, children_any_original, (&children_any<0x9cbb70>), &g_children_any);
REWRITE("Children::Any9C6950", 0x9c6990, children_any_original, (&children_any<0x9cbb80>), &g_children_any);
REWRITE("Children::Any9C6950", 0x9c6e40, children_any_original, (&children_any<0x9cbb80>), &g_children_any);

// ---- Mode::Step: in one mode, finishes and clears +0x1a4; in any other, the usual step --------------------------------------
static volatile long g_mode_steps;
template <int32_t MODE, uint32_t OTHERWISE> static uint32_t __fastcall mode_step(uint8_t *self) {
    g_mode_steps++;
    if (*(int32_t *)(self + 0x190) != MODE) return ((uint32_t(__fastcall *)(void *))OTHERWISE)(self);
    ((void(__thiscall *)(void *, uint32_t, uint32_t))0x7cd220)(self, 0, 0);
    *(uint32_t *)(self + 0x1a4) = 0;
    return 1;
}
static const uint8_t mode_step_original[] = {0x56, 0x8b, 0xf1, 0x83, 0xbe, 0x90, 0x01, 0x00, 0x00};
REWRITE("Mode::Step799750", 0x799750, mode_step_original, (&mode_step<0xd, 0x7ecd90>), &g_mode_steps);
REWRITE("Mode::Step799750", 0x7e8fc0, mode_step_original, (&mode_step<0xd, 0x7ecd90>), &g_mode_steps);
REWRITE("Mode::Step799750", 0x7f49e0, mode_step_original, (&mode_step<0xd, 0x7ecd90>), &g_mode_steps);
REWRITE("Mode::Step799750", 0x7f8810, mode_step_original, (&mode_step<0x16, 0x761440>), &g_mode_steps);

// ---- Part::Pass: unless the flag at +0x1c0 is set, passes two values on to a virtual of the part at +0x40 ----------------------
static volatile long g_part_passes;
template <uint32_t SLOT> static uint8_t __thiscall part_pass(uint8_t *self, uint32_t a, uint32_t b) {
    g_part_passes++;
    if (self[0x1c0]) return 1;
    void *part = self + 0x40;
    ((void(__thiscall *)(void *, uint32_t, uint32_t))(*(void ***)part)[SLOT / 4])(part, a, b);
    return 0;
}
static const uint8_t part_pass_original[] = {0x8a, 0x81, 0xc0, 0x01, 0x00, 0x00, 0x84, 0xc0};
REWRITE("Part::PassAAE180", 0xaae180, part_pass_original, (&part_pass<0x4>), &g_part_passes);
REWRITE("Part::PassAAE180", 0xaae1b0, part_pass_original, (&part_pass<0x8>), &g_part_passes);
REWRITE("Part::PassAAE180", 0xaae1e0, part_pass_original, (&part_pass<0xc>), &g_part_passes);
REWRITE("Part::PassAAE180", 0xaae210, part_pass_original, (&part_pass<0x10>), &g_part_passes);

// ---- Chain::Free: frees the head of a chain of blocks, the rest of the chain (at +OFF) deleted first ---------------------------
static volatile long g_chain_frees;
template <uint32_t OFF, uint32_t DESTROY> static void __fastcall chain_free(uint8_t **holder) {
    g_chain_frees++;
    uint8_t *head = *holder;
    if (!head) return;
    void *next = *(void **)(head + OFF);
    if (next) ((void *(__thiscall *)(void *, uint32_t))DESTROY)(next, 1);
    memory_free(head);
}
static const uint8_t chain_free_original[] = {0x56, 0x8b, 0x31, 0x85, 0xf6, 0x74, 0x1a, 0x8b, 0x8e};
REWRITE("Chain::Free42C1E0", 0x42c1e0, chain_free_original, (&chain_free<0x504, 0x42bd60>), &g_chain_frees);
REWRITE("Chain::Free42C1E0", 0x42c210, chain_free_original, (&chain_free<0x504, 0x42bd90>), &g_chain_frees);
REWRITE("Chain::Free42C1E0", 0x42c240, chain_free_original, (&chain_free<0x1704, 0x42bdc0>), &g_chain_frees);
REWRITE("Chain::Free42C1E0", 0x84f620, chain_free_original, (&chain_free<0xc04, 0x84db10>), &g_chain_frees);
REWRITE("Chain::Free42C1E0", 0x84f650, chain_free_original, (&chain_free<0x1004, 0x84db40>), &g_chain_frees);

// ---- Lookup::Refresh: when there is something to look up (+0x28) and +0x2c isn't set, looks up the key at +0x20 again -------------
static volatile long g_lookup_refreshes;
static void __fastcall lookup_refresh(uint8_t *self) {
    g_lookup_refreshes++;
    if (!*(uint32_t *)(self + 0x28) || self[0x2c]) return;
    void *table = *(void **)(*(uint8_t **)(self + 0x14) + 0x470);
    *(uint32_t *)(self + 0x1c) = ((uint32_t(__thiscall *)(void *, void *))0xc5c670)(table, self + 0x20);
}
static const uint8_t lookup_refresh_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x28, 0x85, 0xc0};
REWRITE("Lookup::RefreshBFBE30", 0xbfbe30, lookup_refresh_original, (&lookup_refresh), &g_lookup_refreshes);
REWRITE("Lookup::RefreshBFBE30", 0xbfc440, lookup_refresh_original, (&lookup_refresh), &g_lookup_refreshes);
REWRITE("Lookup::RefreshBFBE30", 0xbfc5f0, lookup_refresh_original, (&lookup_refresh), &g_lookup_refreshes);

// ---- Registered::ForEach: runs a function on every value of the list in a global ------------------------------------------
static volatile long g_registered_walks;
template <uint32_t LIST, uint32_t EACH> static void __cdecl registered_for_each() {
    g_registered_walks++;
    for (Node *n = *(Node **)LIST; n->next; n = n->next) ((void(__fastcall *)(void *))EACH)(n->value);
}
template <uint32_t LIST> static const uint8_t registered_original[] = {
    0x56, 0x8b, 0x35, (uint8_t)LIST, (uint8_t)(LIST >> 8), (uint8_t)(LIST >> 16), (uint8_t)(LIST >> 24)};
REWRITE("Registered::ForEach49ED40", 0x49ed40, registered_original<0x10ee250>, (&registered_for_each<0x10ee250, 0x49e670>), &g_registered_walks);
REWRITE("Registered::ForEach49ED40", 0x73d9d0, registered_original<0x10ef794>, (&registered_for_each<0x10ef794, 0x73c240>), &g_registered_walks);
REWRITE("Registered::ForEach49ED40", 0x7ea6d0, registered_original<0x10f0650>, (&registered_for_each<0x10f0650, 0x7ea320>), &g_registered_walks);
REWRITE("Registered::ForEach49ED40", 0xba31a0, registered_original<0x10f2a9c>, (&registered_for_each<0x10f2a9c, 0xb9fbd0>), &g_registered_walks);

// ---- Hold::Release: lets go of a hold {+4 device, +0xc holds, +0xe owed}; the last one pays what is owed to the device -----------
static inline void hold_release(uint8_t *hold) {
    uint16_t &holds = *(uint16_t *)(hold + 0xc), &owed = *(uint16_t *)(hold + 0xe);
    if (!holds || --holds) return;
    for (; owed; owed--) {
        void *device = *(void **)(hold + 4);
        ((void(__stdcall *)(void *))(*(void ***)device)[0x30 / 4])(device);
    }
}
static volatile long g_hold_releases;
static void __fastcall hold_release_call(uint8_t *self) {
    g_hold_releases++;
    hold_release(self);
}
static const uint8_t hold_release_original[] = {0x56, 0x8b, 0xf1, 0x33, 0xc0, 0x66, 0x8b, 0x46, 0x0c};
REWRITE("Hold::ReleaseBE9420", 0xbe9420, hold_release_original, (&hold_release_call), &g_hold_releases);
REWRITE("Hold::ReleaseBE9420", 0xbfd110, hold_release_original, (&hold_release_call), &g_hold_releases);

// ---- Hold::End: ends a hold that is on (+0x24), counting it at +0x18 ---------------------------------------------------------
static volatile long g_hold_ends;
static void __fastcall hold_end(uint8_t *self) {
    g_hold_ends++;
    if (!*(uint32_t *)(self + 0x24)) return;
    hold_release(*(uint8_t **)(*(uint8_t **)(self + 0x20) + 4));
    ++*(int32_t *)(self + 0x18);
    *(uint32_t *)(self + 0x24) = 0;
}
static const uint8_t hold_end_original[] = {0x57, 0x8b, 0xf9, 0x8b, 0x47, 0x24, 0x85, 0xc0};
REWRITE("Hold::EndBFD8F0", 0xbfd8f0, hold_end_original, (&hold_end), &g_hold_ends);
REWRITE("Hold::EndBFD8F0", 0xbfdbc0, hold_end_original, (&hold_end), &g_hold_ends);

// ---- Flags::Refresh: asks the part at +8 whether it's on, and clears or sets a flag of the object this interface sits in ---------
static volatile long g_flag_refreshes;
static uint32_t __thiscall flags_refresh(uint8_t *self, uint32_t) {
    g_flag_refreshes++;
    void *part = *(void **)(self + 8);
    uint8_t on = ((uint8_t(__fastcall *)(void *))(*(void ***)part)[1])(part);
    uint32_t &flags = *(uint32_t *)(self - 0xa8);
    return flags = on ? flags & ~0x400000u : flags | 0x400000u;
}
static const uint8_t flags_refresh_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x4e, 0x08, 0x8b, 0x01};
REWRITE("Flags::Refresh4EBE70", 0x4ebe70, flags_refresh_original, (&flags_refresh), &g_flag_refreshes);
REWRITE("Flags::Refresh4EBE70", 0x62ac60, flags_refresh_original, (&flags_refresh), &g_flag_refreshes);

// ---- Pair::Touch: touches the objects of the two handles at +0x4c and +0x60, those not already touched -------------------------
static volatile long g_pair_touches;
static inline void touch_handle(uint8_t *handle) {
    uint8_t *object = *(uint8_t **)(handle + 0xc);
    if (object && !(object[0x2c] & 1)) ((void(__fastcall *)(void *))0x800860)(object);
}
static void __fastcall pair_touch(uint8_t *self) {
    g_pair_touches++;
    touch_handle(*(uint8_t **)(self + 0x4c));
    touch_handle(*(uint8_t **)(self + 0x60));
}
static const uint8_t pair_touch_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x4c, 0x8b, 0x48};
REWRITE("Pair::Touch548CA0", 0x548ca0, pair_touch_original, (&pair_touch), &g_pair_touches);
REWRITE("Pair::Touch548CA0", 0x549090, pair_touch_original, (&pair_touch), &g_pair_touches);

// ---- Query::Ask: asks the object's virtual at +0x80 for the answer to a value; the value is replaced when there is an answer ------
struct Query {
    uint32_t table;
    uint8_t found;
    int32_t answer;
    int32_t value;
};
static volatile long g_queries;
static uint8_t __thiscall query_ask(void **self, int32_t *value) {
    g_queries++;
    Query q{0xf30a7c, 1, (int32_t)0x80000000, *value};
    ((void(__thiscall *)(void *, Query *))(*(void ***)self)[0x80 / 4])(self, &q);
    if (!q.found) return 0;
    *value = q.answer;
    return 1;
}
static const uint8_t query_ask_original[] = {0x83, 0xec, 0x10, 0x8b, 0x11, 0x56, 0x8b, 0x74, 0x24, 0x18};
REWRITE("Query::Ask4A9210", 0x4a9210, query_ask_original, (&query_ask), &g_queries);
REWRITE("Query::Ask4A9210", 0x5a2c20, query_ask_original, (&query_ask), &g_queries);

// ---- Kept::Value: the value of the interface kept by a reference at +REF, 0 when there is none -----------------------------------
static volatile long g_kept_values;
template <uint32_t REF> static uint32_t __fastcall kept_value(uint8_t *self) {
    g_kept_values++;
    uint8_t *found = (uint8_t *)ref_interface<0x130bbec>(self + REF);
    if (!found) return 0;
    return ((uint32_t(__fastcall *)(void *))0x846ae0)(found + 0x40);
}
static const uint8_t kept_value_original[] = {0x56, 0x8b, 0xf1, 0x8a, 0x86};
REWRITE("Kept::Value4ABC40", 0x4abc40, kept_value_original, (&kept_value<0x3ac>), &g_kept_values);
REWRITE("Kept::Value4ABC40", 0x586330, kept_value_original, (&kept_value<0x1d4>), &g_kept_values);
REWRITE("Kept::Value4ABC40", 0x591a30, kept_value_original, (&kept_value<0x1ec>), &g_kept_values);
REWRITE("Kept::Value4ABC40", 0xa7bec0, kept_value_original, (&kept_value<0x1ac>), &g_kept_values);

// ---- Refs::AnyHas: whether any reference (up to and including the last index, +0x78) has an object with the interface ID ----------
static inline void *object_interface(void *object, const int32_t *id) {
    if (!object) return nullptr;
    ClassInterfaces *c = ((ClassInterfaces * (__fastcall *)(void *))(*(void ***)object)[1])(object);
    if (!c || *id >= c->count || !c->by_id[*id]) return nullptr;
    return (uint8_t *)object + c->by_id[*id]->offset;
}
static volatile long g_refs_any;
template <uint32_t ID> static uint8_t __fastcall refs_any_has(uint8_t *self) {
    g_refs_any++;
    for (int32_t i = 0; i <= *(int32_t *)(self + 0x78); i++) {
        void *object = ref_interface<0x13d0058>(*(uint8_t **)(self + 0x7c) + i * 0x1c);
        if (object_interface(object, (const int32_t *)ID)) return 1;
    }
    return 0;
}
static const uint8_t refs_any_original[] = {0x51, 0x53, 0x55, 0x8b, 0xe9, 0x8b, 0x45, 0x78, 0x33, 0xdb};
REWRITE("Refs::AnyHasB453A0", 0xb453a0, refs_any_original, (&refs_any_has<0x13ce864>), &g_refs_any);
REWRITE("Refs::AnyHasB453A0", 0xb45460, refs_any_original, (&refs_any_has<0x13ce90c>), &g_refs_any);

// ---- Reference::Assign: leaves the old handle and takes the other reference's, telling the reference (its first virtual) -----------
static inline void reference_assign(uint8_t *ref, uint8_t *other) {
    Node *node = (Node *)(ref + 8);
    unlink(node);
    ((void(__thiscall *)(void *, uint32_t))0x8055c0)(*(void **)(ref + 4), 0);
    uint8_t *handle = *(uint8_t **)(other + 4);
    ++*(int32_t *)(handle + 8);
    *(uint8_t **)(ref + 4) = handle;
    Node *head = *(Node **)(handle + 0x20);
    node->next = head->next;
    head->next->prev = node;
    node->prev = head;
    head->next = node;
    ((void(__fastcall *)(void *))(*(void ***)ref)[0])(ref);
}
static volatile long g_reference_copies;
static uint8_t *__thiscall reference_copy(uint8_t *self, uint8_t *other) {
    g_reference_copies++;
    reference_assign(self, other);
    *(uint32_t *)(self + 0x14) = *(uint32_t *)(other + 0x14);
    *(uint32_t *)(self + 0x18) = *(uint32_t *)(other + 0x18);
    self[0x1c] = other[0x1c];
    *(uint32_t *)(self + 0x20) = *(uint32_t *)(other + 0x20);
    self[0x24] = other[0x24];
    return self;
}
static const uint8_t reference_copy_original[] = {0x53, 0x56, 0x8b, 0xf1, 0x57, 0x8d, 0x7e, 0x08};
REWRITE("Reference::Copy82E860", 0x82e860, reference_copy_original, (&reference_copy), &g_reference_copies);
REWRITE("Reference::Copy82E860", 0xb4b4b0, reference_copy_original, (&reference_copy), &g_reference_copies);

// ---- Reference::AssignAt: assigns the reference held at +OFF ----------------------------------------------------------------
static volatile long g_reference_assigns;
template <uint32_t OFF> static void __thiscall reference_assign_at(uint8_t *self, uint8_t *other) {
    g_reference_assigns++;
    reference_assign(self + OFF, other);
}
template <uint32_t OFF> static const uint8_t reference_assign_original[] = {
    0x56, 0x57, 0x8d, 0xb9, (uint8_t)OFF, (uint8_t)(OFF >> 8), (uint8_t)(OFF >> 16), (uint8_t)(OFF >> 24)};
REWRITE("Reference::AssignAt530F40", 0x530f40, reference_assign_original<0x514>, (&reference_assign_at<0x514>), &g_reference_assigns);
REWRITE("Reference::AssignAt530F40", 0x8268a0, reference_assign_original<0xbc>, (&reference_assign_at<0xbc>), &g_reference_assigns);

// ---- ItemArray::Resize: new aligned items for {data, count}, as many of the old as fit assigned over ----------------------------
static void(__stdcall *const construct_items)(void *, uint32_t, int32_t, uint32_t) = (void(__stdcall *)(void *, uint32_t, int32_t, uint32_t))0x5d3d00;
static volatile long g_item_resizes;
template <uint32_t CONSTRUCT, uint32_t ASSIGN> static int32_t __thiscall item_array_resize(uint32_t *self, int32_t count) {
    g_item_resizes++;
    int32_t old_count = (int32_t)self[1];
    if (count == old_count) return count;
    uint8_t *items = nullptr;
    if (count) {
        items = (uint8_t *)memory_alloc_aligned(count * 0x14);
        if (items) construct_items(items, 0x14, count, CONSTRUCT);
        int32_t keep = count < old_count ? count : old_count;
        for (int32_t i = 0; i < keep; i++)
            ((void(__thiscall *)(void *, void *))ASSIGN)(items + i * 0x14, (uint8_t *)(uintptr_t)self[0] + i * 0x14);
    }
    if (self[0]) {
        memory_free_aligned((void *)(uintptr_t)self[0]);
        self[0] = 0;
    }
    self[0] = (uint32_t)(uintptr_t)items;
    self[1] = (uint32_t)count;
    return count;
}
static const uint8_t item_resize_original[] = {0x83, 0xec, 0x08, 0x55, 0x8b, 0x6c, 0x24, 0x10};
REWRITE("ItemArray::Resize5D42A0", 0x5d42a0, item_resize_original, (&item_array_resize<0x5d3ce0, 0x5d3d40>), &g_item_resizes);
REWRITE("ItemArray::Resize5D42A0", 0x5d43c0, item_resize_original, (&item_array_resize<0x5d3da0, 0x5d3e60>), &g_item_resizes);
REWRITE("ItemArray::Resize5D42A0", 0x5d44e0, item_resize_original, (&item_array_resize<0x5d3dc0, 0x5d3ec0>), &g_item_resizes);

// ---- ItemArray::Reset: drops the items of {data, count} and makes `count` new ones of 32 bytes ----------------------------------
static volatile long g_item_resets;
template <uint32_t CONSTRUCT> static int32_t __thiscall item_array_reset(uint32_t *self, int32_t count) {
    g_item_resets++;
    if (self[0]) {
        memory_free_aligned((void *)(uintptr_t)self[0]);
        self[0] = 0;
    }
    if (count) {
        void *items = memory_alloc_aligned((uint32_t)count << 5);
        if (items) construct_items(items, 0x20, count, CONSTRUCT);
        self[0] = (uint32_t)(uintptr_t)items;
    }
    self[1] = (uint32_t)count;
    return count;
}
static const uint8_t item_reset_original[] = {0x53, 0x8b, 0xd9, 0x8b, 0x03, 0x85, 0xc0, 0x56};
REWRITE("ItemArray::Reset5D7370", 0x5d7370, item_reset_original, (&item_array_reset<0x5d4600>), &g_item_resets);
REWRITE("ItemArray::Reset5D7370", 0x5d74d0, item_reset_original, (&item_array_reset<0x5d4620>), &g_item_resets);

// ---- A cursor node: a place in a list that survives what the walk does to the list ------------------------------------------
static inline void cursor_before(Node *cursor, Node *n) {
    cursor->next = n;
    cursor->prev = n->prev;
    cursor->value = nullptr;
    n->prev->next = cursor;
    n->prev = cursor;
}
static inline void cursor_past(Node *cursor, Node *n) {
    unlink(cursor);
    cursor->next = n->next;
    n->next->prev = cursor;
    cursor->prev = n;
    n->next = cursor;
}

// ---- List::DeleteAll: deletes every value of the list at +OFF (each delete takes its node out), stepping over empty nodes ---------
static volatile long g_list_delete_alls;
template <uint32_t OFF> static void __fastcall list_delete_all(uint8_t *self) {
    g_list_delete_alls++;
    Node cursor;
    cursor_before(&cursor, *(Node **)(self + OFF));
    for (Node *n; (n = cursor.next)->next;) {
        if (n->value) ((void(__thiscall *)(void *, uint32_t))(*(void ***)n->value)[0])(n->value, 1);
        else cursor_past(&cursor, n);
    }
    unlink(&cursor);
}
template <uint32_t OFF> static const uint8_t list_delete_all_original[] = {0x83, 0xec, 0x0c, 0x8b, 0x41, (uint8_t)OFF, 0x56, 0x33, 0xf6};
REWRITE("List::DeleteAll6AF540", 0x6af540, list_delete_all_original<0x64>, (&list_delete_all<0x64>), &g_list_delete_alls);
REWRITE("List::DeleteAll6AF540", 0x6af600, list_delete_all_original<0x4c>, (&list_delete_all<0x4c>), &g_list_delete_alls);

// ---- Members::Notify: after the base step, tells each member's owner (+0x13c) about the member ---------------------------------
static volatile long g_members_notified;
static void __thiscall members_notify(uint8_t *self, uint32_t) {
    g_members_notified++;
    ((void(__fastcall *)(void *))0x69a980)(self);
    Node cursor;
    cursor_before(&cursor, *(Node **)(self + 0xd4));
    for (Node *n; (n = cursor.next)->next;) {
        cursor_past(&cursor, n);
        uint8_t *member = (uint8_t *)n->value;
        if (!member) continue;
        void *owner = *(void **)(member + 0x13c);
        if (owner) ((void(__thiscall *)(void *, void *))(*(void ***)owner)[2])(owner, member);
    }
    unlink(&cursor);
}
static const uint8_t members_notify_original[] = {0x83, 0xec, 0x0c, 0x56, 0x57, 0x8b, 0xf1, 0xe8};
REWRITE("Members::Notify6A20E0", 0x6a20e0, members_notify_original, (&members_notify), &g_members_notified);
REWRITE("Members::Notify6A20E0", 0x6a21c0, members_notify_original, (&members_notify), &g_members_notified);

// ---- List::Visit: hands +0x18 of each value of the list to a visitor's first virtual, skipping empty nodes -----------------------
static volatile long g_list_visits;
static void __thiscall list_visit(Node **list, void *visitor) {
    g_list_visits++;
    Node cursor;
    cursor_before(&cursor, *list);
    for (;;) {
        Node *n = cursor.next;
        while (n->next && !n->value) n = n->next;
        if (!n->next) break;
        cursor_past(&cursor, n);
        ((void(__thiscall *)(void *, uint32_t))(*(void ***)visitor)[0])(visitor, *(uint32_t *)((uint8_t *)n->value + 0x18));
    }
    unlink(&cursor);
}
static const uint8_t list_visit_original[] = {0x83, 0xec, 0x0c, 0x8b, 0x01, 0x56, 0x33, 0xf6};
REWRITE("List::Visit49BAE0", 0x49bae0, list_visit_original, (&list_visit), &g_list_visits);
REWRITE("List::Visit49BAE0", 0x762860, list_visit_original, (&list_visit), &g_list_visits);

// ---- Coaster::SetTarget: in states 1, 3 and 4, moves to a new target (+FIELD) or, when nothing changed, just carries on -------------
static volatile long g_target_sets;
template <uint32_t FIELD, uint32_t BIT> static void __thiscall target_set(uint8_t *self, int32_t target) {
    g_target_sets++;
    int32_t state = *(int32_t *)(self + 0x144);
    if (state != 1 && state != 3 && state != 4) return;
    if (!*(uint32_t *)(self + 0x5dc) && *(int32_t *)(self + FIELD) == target && state != 1) {
        ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))0x4f5950)(self, 0, 0, 0, 0);
        return;
    }
    *(int32_t *)(self + FIELD) = target;
    *(uint32_t *)(self + 0x5dc) = 0;
    ((void(__thiscall *)(void *, uint32_t))0x4f1740)(self, BIT);
    ((void(__fastcall *)(void *))0x4f5060)(self);
    ((void(__thiscall *)(void *, uint32_t))0x4f41c0)(self, 0);
}
static const uint8_t target_set_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x86, 0x44, 0x01, 0x00};
REWRITE("Target::Set4F65F0", 0x4f65f0, target_set_original, (&target_set<0x5e4, 1>), &g_target_sets);
REWRITE("Target::Set4F65F0", 0x4f6670, target_set_original, (&target_set<0x5e8, 2>), &g_target_sets);
REWRITE("Target::Set4F65F0", 0x4f66f0, target_set_original, (&target_set<0x5ec, 4>), &g_target_sets);

// ---- SlotPool::Take: the next free slot of {items, free indices, used, capacity}, marked in use; a full pool doubles ---------------
static volatile long g_slot_takes;
template <uint32_t GROW, uint32_t SIZE> static int32_t __fastcall slot_pool_take(int32_t *self) {
    g_slot_takes++;
    if (self[2] >= self[3]) ((void(__thiscall *)(void *, int32_t))GROW)(self, self[3] > 0 ? self[3] * 2 : 200);
    int32_t slot = ((int32_t *)(uintptr_t)self[1])[self[3] - self[2] - 1];
    ((uint8_t *)(uintptr_t)self[0])[slot * SIZE] = 1;
    self[2]++;
    return slot;
}
static const uint8_t slot_take_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x0c, 0x39, 0x46, 0x08};
REWRITE("SlotPool::Take735B20", 0x735b20, slot_take_original, (&slot_pool_take<0x735850, 0x98>), &g_slot_takes);
REWRITE("SlotPool::Take735B20", 0x8ea260, slot_take_original, (&slot_pool_take<0x8e9330, 0x130>), &g_slot_takes);
REWRITE("SlotPool::Take735B20", 0x964d20, slot_take_original, (&slot_pool_take<0x964bc0, 0x1018>), &g_slot_takes);

// ---- Limit::Check: the object at +0x294's limit (3 at low levels) against two counts of this object ---------------------------------
static volatile long g_limit_checks;
template <uint32_t FIELD, uint32_t COUNT, uint32_t CHECK> static uint32_t __fastcall limit_check(uint8_t *self) {
    g_limit_checks++;
    uint8_t *object = *(uint8_t **)(self + 0x294);
    uint8_t *record = ((uint8_t * (__fastcall *)(void *))0xbb3e70)(object);
    uint32_t limit = record[1] > 1 ? *(uint32_t *)(object + FIELD) : 3;
    uint32_t first = ((uint32_t(__fastcall *)(void *))0x532ea0)(self);
    uint32_t second = ((uint32_t(__fastcall *)(void *))COUNT)(self);
    return ((uint32_t(__fastcall *)(uint32_t, uint32_t, uint32_t))CHECK)(limit, second, first);
}
static const uint8_t limit_check_original[] = {0x56, 0x8b, 0xf1, 0x57, 0x8b, 0xbe, 0x94, 0x02, 0x00, 0x00};
REWRITE("Limit::Check536B50", 0x536b50, limit_check_original, (&limit_check<0x28, 0x533ff0, 0xbb3de0>), &g_limit_checks);
REWRITE("Limit::Check536B50", 0x536ba0, limit_check_original, (&limit_check<0x2c, 0x5340e0, 0xbb3de0>), &g_limit_checks);
REWRITE("Limit::Check536B50", 0x536bf0, limit_check_original, (&limit_check<0x28, 0x533ff0, 0xbb3e20>), &g_limit_checks);
REWRITE("Limit::Check536B50", 0x536c40, limit_check_original, (&limit_check<0x2c, 0x5340e0, 0xbb3e20>), &g_limit_checks);

// ---- Events::Add: adds {kind, flag} to the array at +0x7c, room made ten at a time ------------------------------------------------
static volatile long g_events_added;
template <uint32_t KIND> static void __thiscall events_add(uint8_t *self, uint32_t flag) {
    g_events_added++;
    int32_t &count = *(int32_t *)(self + 0x84);
    if (*(int32_t *)(self + 0x80) <= count) ((void(__thiscall *)(void *, int32_t))0x475a30)(self + 0x7c, count + 10);
    uint32_t *event = *(uint32_t **)(self + 0x7c) + count * 2;
    event[0] = KIND;
    event[1] = (uint8_t)flag != 0;
    count++;
}
static const uint8_t events_add_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x86, 0x84, 0x00, 0x00};
REWRITE("Events::Add475CB0", 0x475cb0, events_add_original, (&events_add<1>), &g_events_added);
REWRITE("Events::Add475CB0", 0x475d00, events_add_original, (&events_add<3>), &g_events_added);

// ---- Tree::Update: updates the object's parts (list at +0x130), then each child's the same way (list at +0x148) ----------------------
static volatile long g_tree_updates;
template <uint32_t EACH> static void __fastcall tree_update(uint8_t *self) {
    g_tree_updates++;
    for (Node *n = *(Node **)(self + 0x130); n->next; n = n->next) ((void(__fastcall *)(void *))EACH)(n->value);
    for (Node *n = *(Node **)(self + 0x148); n->next; n = n->next) tree_update<EACH>(*(uint8_t **)n->value);
}
static const uint8_t tree_update_original[] = {0x56, 0x57, 0x8b, 0xf9, 0x8b, 0xb7, 0x30, 0x01, 0x00, 0x00};
REWRITE("Tree::Update86ED80", 0x86ed80, tree_update_original, (&tree_update<0x836bd0>), &g_tree_updates);
REWRITE("Tree::Update86ED80", 0x86ede0, tree_update_original, (&tree_update<0x836d00>), &g_tree_updates);
