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

// the first bytes of a function that starts by calling TO with its object: the call's offset depends on where the copy is
template <uint32_t AT, uint32_t TO> static const uint8_t calls_first_original[] = {
    0x56, 0x8b, 0xf1, 0xe8, (uint8_t)(TO - (AT + 8)), (uint8_t)((TO - (AT + 8)) >> 8), (uint8_t)((TO - (AT + 8)) >> 16), (uint8_t)((TO - (AT + 8)) >> 24)};

// ---- Slot::Reset: no id (-1), everything else zero ----------------------------------------------------------------------------
static volatile long g_slot_resets;
static uint32_t *__fastcall slot_reset(uint32_t *self) {
    g_slot_resets++;
    self[0] = 0xffffffff;
    for (int i = 1; i < 10; i++) self[i] = 0;
    return self;
}
static const uint8_t slot_reset_original[] = {0x8b, 0xc1, 0x33, 0xc9, 0xc7, 0x00, 0xff, 0xff};
REWRITE("Slot::ResetCDF3C0", 0xcdf3c0, slot_reset_original, (&slot_reset), &g_slot_resets);
REWRITE("Slot::ResetCDF3C0", 0xcdf410, slot_reset_original, (&slot_reset), &g_slot_resets);

// ---- Tree::Init: an empty tree: the first entry points at its own end at +0xc, and +0x10 back at the tree -------------------------
static volatile long g_tree_inits;
static uint32_t *__fastcall tree_init(uint32_t *self) {
    g_tree_inits++;
    self[0] = (uint32_t)(uintptr_t)(self + 3);
    self[1] = self[2] = self[3] = self[5] = 0;
    self[4] = (uint32_t)(uintptr_t)self;
    return self;
}
static const uint8_t tree_init_original[] = {0x8b, 0xc1, 0x33, 0xd2, 0x8d, 0x48, 0x0c};
REWRITE("Tree::InitADA8B0", 0xada8b0, tree_init_original, (&tree_init), &g_tree_inits);
REWRITE("Tree::InitADA8B0", 0xaf7570, tree_init_original, (&tree_init), &g_tree_inits);
REWRITE("Tree::InitADA8B0", 0xc77340, tree_init_original, (&tree_init), &g_tree_inits);

// ---- Id::Release: gives an id back to the system in a global, and forgets it ------------------------------------------------------
static volatile long g_id_releases;
static void __fastcall id_release(uint32_t *self) {
    g_id_releases++;
    void *system = *(void **)0x13d0db8;
    ((void(__thiscall *)(void *, uint32_t))(*(void ***)system)[0xc8 / 4])(system, *self);
    *self = 0;
}
static const uint8_t id_release_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x0d, 0xb8, 0x0d, 0x3d, 0x01};
REWRITE("Id::Release910380", 0x910380, id_release_original, (&id_release), &g_id_releases);
REWRITE("Id::Release910380", 0x958940, id_release_original, (&id_release), &g_id_releases);

// ---- Registration::Destroy: sets the table and takes +4 out of the registry at +8, when there is one -----------------------------
static volatile long g_registration_destroys;
static void __fastcall registration_destroy(uint32_t *self) {
    g_registration_destroys++;
    self[0] = 0xf76a98;
    if (self[2]) ((void(__thiscall *)(uint32_t, uint32_t))0x45ee50)(self[2], self[1]);
}
static const uint8_t registration_destroy_original[] = {0x8b, 0xc1, 0x8b, 0x48, 0x08, 0x85, 0xc9, 0xc7, 0x00};
REWRITE("Registration::Destroy8272E0", 0x8272e0, registration_destroy_original, (&registration_destroy), &g_registration_destroys);
REWRITE("Registration::Destroy8272E0", 0x8286e0, registration_destroy_original, (&registration_destroy), &g_registration_destroys);

// ---- Owned::Replace: deletes the object held at +OFF, and holds the new one -----------------------------------------------------
static volatile long g_owned_replaces;
template <uint32_t OFF> static void __thiscall owned_replace(uint8_t *self, void *object) {
    g_owned_replaces++;
    void *old = *(void **)(self + OFF);
    if (old) ((void(__thiscall *)(void *, uint32_t))(*(void ***)old)[0])(old, 1);
    *(void **)(self + OFF) = object;
}
static const uint8_t owned_replace_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x8e};
REWRITE("Owned::Replace7975C0", 0x7975c0, owned_replace_original, (&owned_replace<0x51c>), &g_owned_replaces);
REWRITE("Owned::Replace7975C0", 0xc09470, owned_replace_original, (&owned_replace<0xbc>), &g_owned_replaces);
REWRITE("Owned::Replace7975C0", 0xc094a0, owned_replace_original, (&owned_replace<0xc0>), &g_owned_replaces);

// ---- Members::Each: calls a function on each value of the list at +0x44, with one argument ----------------------------------------
static volatile long g_members_each;
template <uint32_t EACH> static void __thiscall members_each(uint8_t *self, uint32_t argument) {
    g_members_each++;
    for (Node *n = *(Node **)(self + 0x44); n->next; n = n->next) ((void(__thiscall *)(void *, uint32_t))EACH)(n->value, argument);
}
static const uint8_t members_each_original[] = {0x56, 0x8b, 0x71, 0x44, 0x83, 0x3e, 0x00};
REWRITE("Members::Each737210", 0x737210, members_each_original, (&members_each<0x72df40>), &g_members_each);
REWRITE("Members::Each737210", 0x737240, members_each_original, (&members_each<0x729040>), &g_members_each);

// ---- Lookup::Through: asks the interface of the handle at +4 about the value at +FIELD, 0 when there is no interface -----------------
static volatile long g_lookups_through;
template <uint32_t FIELD, uint32_t ASK> static uint32_t __fastcall lookup_through(uint8_t *self) {
    g_lookups_through++;
    void *found = find_interface(*(uint8_t **)(self + 4), (const int32_t *)0x12f51b4);
    if (!found) return 0;
    return ((uint32_t(__thiscall *)(void *, uint32_t))ASK)(found, *(uint32_t *)(self + FIELD));
}
static const uint8_t lookup_through_original[] = {0x56, 0x57, 0x8b, 0xf9, 0x8b, 0x47, 0x04, 0x8b, 0x70, 0x0c};
REWRITE("Lookup::Through7E5280", 0x7e5280, lookup_through_original, (&lookup_through<0x14, 0x531a80>), &g_lookups_through);
REWRITE("Lookup::Through7E5280", 0x7e52d0, lookup_through_original, (&lookup_through<0x18, 0x531ba0>), &g_lookups_through);

// ---- Pair::Release: gives both ids back to the system in a global and returns to the start (no ids, 4.0) ---------------------------
static volatile long g_pair_releases;
static void __fastcall pair_release(uint32_t *self) {
    g_pair_releases++;
    if (self[0] == 0xffffffff) return;
    ((void(__thiscall *)(void *, uint32_t))0x94ef60)(*(void **)0x133a380, self[0]);
    self[0] = 0xffffffff;
    ((void(__thiscall *)(void *, uint32_t))0x94ef60)(*(void **)0x133a380, self[1]);
    self[1] = 0xffffffff;
    self[2] = 0x40800000;
}
static const uint8_t pair_release_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x06, 0x83, 0xf8, 0xff};
REWRITE("Pair::Release50C090", 0x50c090, pair_release_original, (&pair_release), &g_pair_releases);
REWRITE("Pair::Release50C090", 0x50ee20, pair_release_original, (&pair_release), &g_pair_releases);

// ---- Table::Find: what the table at +0x34 gives for a key after mapping it, -1 without a table ---------------------------------
static volatile long g_table_finds;
template <uint32_t FIND> static int32_t __thiscall table_find(uint8_t *self, uint32_t key) {
    g_table_finds++;
    if (!*(void **)(self + 0x34)) return -1;
    uint32_t mapped = ((uint32_t(__thiscall *)(void *, uint32_t))0xcada50)(*(void **)(self + 0x34), key);
    return ((int32_t(__thiscall *)(void *, uint32_t))FIND)(*(void **)(self + 0x34), mapped);
}
static const uint8_t table_find_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x4e, 0x34, 0x85, 0xc9};
REWRITE("Table::FindCAFA00", 0xcafa00, table_find_original, (&table_find<0xcada70>), &g_table_finds);
REWRITE("Table::FindCAFA00", 0xcafa30, table_find_original, (&table_find<0xcaf6e0>), &g_table_finds);
REWRITE("Table::FindCAFA00", 0xcafa90, table_find_original, (&table_find<0xcadc50>), &g_table_finds);

// ---- Level::ValueOrNone: as Level::Value, -1 below the level -----------------------------------------------------------------------
static volatile long g_level_values;
template <uint32_t LEVEL, uint32_t OFF> static int32_t __fastcall level_value_or_none(uint8_t *self) {
    g_level_values++;
    void *a = ((void *(__fastcall *)(void *))0xbb4280)(self);
    uint8_t level = ((uint8_t * (__fastcall *)(void *))0xbb3e70)(a)[1];
    if (!level || level < LEVEL) return -1;
    return *(int32_t *)(self + OFF);
}
REWRITE("Level::ValueOrNone5A11D0", 0x5a11d0, (calls_first_original<0x5a11d0, 0xbb4280>), (&level_value_or_none<2, 0x17c>), &g_level_values);
REWRITE("Level::ValueOrNone5A11D0", 0x5a1280, (calls_first_original<0x5a1280, 0xbb4280>), (&level_value_or_none<3, 0x1b0>), &g_level_values);
REWRITE("Level::ValueOrNone5A11D0", 0x5d26e0, (calls_first_original<0x5d26e0, 0xbb4280>), (&level_value_or_none<2, 0x15c>), &g_level_values);

// ---- Buffers::Clear: frees both aligned buffers and zeroes the bookkeeping ----------------------------------------------------------
static volatile long g_buffer_clears;
static void __fastcall buffers_clear(uint8_t *self) {
    g_buffer_clears++;
    uint32_t *words = (uint32_t *)self;
    memory_free_aligned((void *)(uintptr_t)words[0]);
    memory_free_aligned((void *)(uintptr_t)words[6]);
    for (int i = 0; i < 5; i++) words[i] = 0;
    self[0x14] = 0;
    words[6] = 0;
}
static const uint8_t buffers_clear_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x06, 0x50, 0xe8};
REWRITE("Buffers::Clear94D6D0", 0x94d6d0, buffers_clear_original, (&buffers_clear), &g_buffer_clears);
REWRITE("Buffers::Clear94D6D0", 0x94e060, buffers_clear_original, (&buffers_clear), &g_buffer_clears);

// ---- Level::Allowed: from level 3 whether +OFF is set; below that, always ----------------------------------------------------------
static volatile long g_level_allowed;
template <uint32_t OFF> static uint8_t __fastcall level_allowed(uint8_t *self) {
    g_level_allowed++;
    uint8_t level = ((uint8_t * (__fastcall *)(void *))0xbb3e70)(self)[1];
    if (level < 3) return 1;
    return *(uint32_t *)(self + OFF) != 0;
}
REWRITE("Level::Allowed52FF60", 0x52ff60, (calls_first_original<0x52ff60, 0xbb3e70>), (&level_allowed<0x3c>), &g_level_allowed);
REWRITE("Level::Allowed52FF60", 0x52ff90, (calls_first_original<0x52ff90, 0xbb3e70>), (&level_allowed<0x40>), &g_level_allowed);

// ---- List::EachVirtual: calls virtual 1 of each value of the list at +OFF, the next node read first as a value may leave --------------
static volatile long g_list_virtuals;
template <uint32_t OFF> static void __fastcall list_each_virtual(uint8_t *self) {
    g_list_virtuals++;
    Node *n = *(Node **)(self + OFF);
    if (!n->next) return;
    while (n) {
        Node *next = n->next;
        Node *following = next->next ? next : nullptr;
        void *value = n->value;
        ((void(__fastcall *)(void *))(*(void ***)value)[1])(value);
        n = following;
    }
}
template <uint32_t OFF> static const uint8_t list_virtual_original[] = {0x8b, 0x41, (uint8_t)OFF, 0x83, 0x38, 0x00, 0x74, 0x23};
REWRITE("List::EachVirtual9817B0", 0x9817b0, list_virtual_original<0x58>, (&list_each_virtual<0x58>), &g_list_virtuals);
REWRITE("List::EachVirtual9817B0", 0xa03e40, list_virtual_original<0x10>, (&list_each_virtual<0x10>), &g_list_virtuals);

// ---- Lookup::Forget: lets the table forget the entry at +0x1c, and releases the object held at +OFF ------------------------------------
static volatile long g_lookup_forgets;
template <uint32_t OFF> static void __fastcall lookup_forget(uint8_t *self) {
    g_lookup_forgets++;
    void *table = *(void **)(*(uint8_t **)(self + 0x14) + 0x470);
    ((void(__thiscall *)(void *, uint32_t))0xc5cb30)(table, *(uint32_t *)(self + 0x1c));
    *(uint32_t *)(self + 0x1c) = 0;
    void *object = *(void **)(self + OFF);
    if (!object) return;
    ((void(__stdcall *)(void *))(*(void ***)object)[2])(object);
    *(void **)(self + OFF) = nullptr;
}
static const uint8_t lookup_forget_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x1c, 0x8b, 0x4e};
REWRITE("Lookup::ForgetBFC400", 0xbfc400, lookup_forget_original, (&lookup_forget<0x3c>), &g_lookup_forgets);
REWRITE("Lookup::ForgetBFC400", 0xbfc5b0, lookup_forget_original, (&lookup_forget<0x40>), &g_lookup_forgets);

// ---- Part::Cycle: runs a part's virtual 2; unless flag 2 is set, notes the reason at +0x14; then its virtual 1 ---------------------------
static volatile long g_part_cycles;
template <uint32_t REASON> static uint32_t __fastcall part_cycle(uint8_t *self) {
    g_part_cycles++;
    void *part = *(void **)(self + 0x28);
    ((void(__fastcall *)(void *))(*(void ***)part)[2])(part);
    if (!(*(uint32_t *)(self + 0x10) & 2)) *(uint32_t *)(self + 0x14) = REASON;
    part = *(void **)(self + 0x28);
    return ((uint32_t(__fastcall *)(void *))(*(void ***)part)[1])(part);
}
static const uint8_t part_cycle_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x4e, 0x28, 0x8b, 0x01};
REWRITE("Part::CycleC501A0", 0xc501a0, part_cycle_original, (&part_cycle<1>), &g_part_cycles);
REWRITE("Part::CycleC501A0", 0xc50220, part_cycle_original, (&part_cycle<0xa>), &g_part_cycles);

// ---- Registry::Join: registers the object with a registry (its id kept at +FIELD), then the base step ----------------------------------
static volatile long g_registry_joins;
template <uint32_t REGISTRY, uint32_t FIELD> static uint32_t __fastcall registry_join(uint8_t *self) {
    g_registry_joins++;
    *(uint32_t *)(self + FIELD) = ((uint32_t(__thiscall *)(void *, void *))0xcdee80)((void *)REGISTRY, self);
    return ((uint32_t(__fastcall *)(void *))0x833890)(self);
}
template <uint32_t REGISTRY> static const uint8_t registry_join_original[] = {
    0x56, 0x8b, 0xf1, 0x56, 0xb9, (uint8_t)REGISTRY, (uint8_t)(REGISTRY >> 8), (uint8_t)(REGISTRY >> 16), (uint8_t)(REGISTRY >> 24)};
REWRITE("Registry::Join786D20", 0x786d20, registry_join_original<0x1306978>, (&registry_join<0x1306978, 0x58>), &g_registry_joins);
REWRITE("Registry::Join786D20", 0x819200, registry_join_original<0x130b420>, (&registry_join<0x130b420, 0x58>), &g_registry_joins);
REWRITE("Registry::Join786D20", 0x8b49d0, registry_join_original<0x1311290>, (&registry_join<0x1311290, 0x60>), &g_registry_joins);

// ---- Status::Busy: whether a check says so or the flag at +0x90 is set ----------------------------------------------------------------
static volatile long g_status_checks;
template <uint32_t CHECK> static uint32_t __fastcall status_busy(uint8_t *self) {
    g_status_checks++;
    return ((uint32_t(__fastcall *)(void *))CHECK)(self) || self[0x90];
}
REWRITE("Status::Busy559BC0", 0x559bc0, (calls_first_original<0x559bc0, 0x558f20>), (&status_busy<0x558f20>), &g_status_checks);
REWRITE("Status::Busy559BC0", 0x559bf0, (calls_first_original<0x559bf0, 0x558fe0>), (&status_busy<0x558fe0>), &g_status_checks);

// ---- Counter::Next: steps a global counter, wrapping to 0 at LIMIT (the value before wrapping is given) --------------------------------
static volatile long g_counter_steps;
template <uint32_t COUNTER, int32_t LIMIT> static int32_t __cdecl counter_next() {
    g_counter_steps++;
    int32_t value = ++*(int32_t *)COUNTER;
    if (value == LIMIT) *(int32_t *)COUNTER = 0;
    return value;
}
template <uint32_t COUNTER> static const uint8_t counter_next_original[] = {
    0xa1, (uint8_t)COUNTER, (uint8_t)(COUNTER >> 8), (uint8_t)(COUNTER >> 16), (uint8_t)(COUNTER >> 24)};
REWRITE("Counter::Next59A170", 0x59a170, counter_next_original<0x12f54c0>, (&counter_next<0x12f54c0, 0xb>), &g_counter_steps);
REWRITE("Counter::Next59A170", 0x59a190, counter_next_original<0x12f54c8>, (&counter_next<0x12f54c8, 3>), &g_counter_steps);
REWRITE("Counter::Next59A170", 0x59a1b0, counter_next_original<0x12f54c4>, (&counter_next<0x12f54c4, 0x1a>), &g_counter_steps);

// ---- Step::Then: the base step, then the step of the part at +OFF when there is one -----------------------------------------------------
static volatile long g_step_thens;
template <uint32_t BASE, uint32_t OFF, uint32_t THEN> static void __fastcall step_then(uint8_t *self) {
    g_step_thens++;
    ((void(__fastcall *)(void *))BASE)(self);
    void *part = *(void **)(self + OFF);
    if (part) ((void(__fastcall *)(void *))THEN)(part);
}
REWRITE("Step::Then67AC00", 0x67ac00, (calls_first_original<0x67ac00, 0x6afd90>), (&step_then<0x6afd90, 0xf8, 0x6b0990>), &g_step_thens);
REWRITE("Step::Then67AC00", 0x7975f0, (calls_first_original<0x7975f0, 0x7cd380>), (&step_then<0x7cd380, 0x51c, 0x752cc0>), &g_step_thens);
REWRITE("Step::Then67AC00", 0x870a40, (calls_first_original<0x870a40, 0x6afd90>), (&step_then<0x6afd90, 0x12c, 0x6b0990>), &g_step_thens);

// ---- Default::Copy: takes +0x48 of the current object in a global (or the default one) into +FIELD ----------------------------------------
static volatile long g_default_copies;
template <uint32_t FIELD> static void __fastcall default_copy(uint8_t *self) {
    g_default_copies++;
    uint8_t *source = *(uint8_t **)0x1106c54;
    if (!source) source = ((uint8_t * (__fastcall *)(void *))0x444090)(self);
    *(uint32_t *)(self + FIELD) = *(uint32_t *)(source + 0x48);
}
static const uint8_t default_copy_original[] = {0xa1, 0x54, 0x6c, 0x10, 0x01, 0x85, 0xc0, 0x56, 0x8b, 0xf1};
REWRITE("Default::Copy4BF080", 0x4bf080, default_copy_original, (&default_copy<0x7c>), &g_default_copies);
REWRITE("Default::Copy4BF080", 0x8b03b0, default_copy_original, (&default_copy<0x1c>), &g_default_copies);

// ---- Rows::Ask: asks the object at +0x54 of a row (0x70 bytes each) ----------------------------------------------------------------------
static volatile long g_rows_asked;
template <uint32_t ASK> static uint32_t __thiscall rows_ask(uint8_t **self, int32_t row, uint32_t what) {
    g_rows_asked++;
    return ((uint32_t(__thiscall *)(void *, uint32_t))ASK)(*(void **)(*self + row * 0x70 + 0x54), what);
}
static const uint8_t rows_ask_original[] = {0x8b, 0x54, 0x24, 0x04, 0x8b, 0x44, 0x24, 0x08, 0x6b, 0xd2, 0x70};
REWRITE("Rows::Ask40BDA0", 0x40bda0, rows_ask_original, (&rows_ask<0xcd8160>), &g_rows_asked);
REWRITE("Rows::Ask40BDA0", 0x40bdc0, rows_ask_original, (&rows_ask<0xcd8170>), &g_rows_asked);

// ---- Entries::Ask: asks an entry (0x1c bytes each, from +0x10) ----------------------------------------------------------------------------
static volatile long g_entries_asked;
template <uint32_t ASK> static uint32_t __thiscall entries_ask(uint8_t *self, int32_t entry, uint32_t what) {
    g_entries_asked++;
    return ((uint32_t(__thiscall *)(void *, uint32_t))ASK)(self + entry * 0x1c + 0x10, what);
}
static const uint8_t entries_ask_original[] = {0x8b, 0x54, 0x24, 0x04, 0x8b, 0x44, 0x24, 0x08, 0x6b, 0xd2, 0x1c};
REWRITE("Entries::Ask8915D0", 0x8915d0, entries_ask_original, (&entries_ask<0x88f830>), &g_entries_asked);
REWRITE("Entries::Ask8915D0", 0x89c760, entries_ask_original, (&entries_ask<0x891680>), &g_entries_asked);

// ---- Triple::Set: stores three values at +OFF and tells the object ---------------------------------------------------------------
static volatile long g_triple_sets;
template <uint32_t OFF, uint32_t CHANGED> static uint32_t __thiscall triple_set(uint8_t *self, uint32_t a, uint32_t b, uint32_t c) {
    g_triple_sets++;
    uint32_t *values = (uint32_t *)(self + OFF);
    values[0] = a;
    values[1] = b;
    values[2] = c;
    return ((uint32_t(__fastcall *)(void *))CHANGED)(self);
}
static const uint8_t triple_set_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0x54, 0x24, 0x08, 0x89, 0x81};
REWRITE("Triple::Set6A4B10", 0x6a4b10, triple_set_original, (&triple_set<0xe0, 0x6a4820>), &g_triple_sets);
REWRITE("Triple::Set6A4B10", 0x6a82d0, triple_set_original, (&triple_set<0xf8, 0x6a8220>), &g_triple_sets);
REWRITE("Triple::Set6A4B10", 0x6a8350, triple_set_original, (&triple_set<0xf8, 0x6a8220>), &g_triple_sets);
REWRITE("Triple::Set6A4B10", 0x83dbc0, triple_set_original, (&triple_set<0xec, 0x83ce70>), &g_triple_sets);

// ---- Counted::Assign: a counted pointer (count at +4) takes another's object; the old one is deleted at zero ----------------------
static volatile long g_counted_assigns;
static uint8_t ***__thiscall counted_assign(uint8_t ***self, uint8_t ***other) {
    g_counted_assigns++;
    uint8_t **old = *self;
    if (!--*(int32_t *)(old + 1) && old) ((void(__thiscall *)(void *, uint32_t))(*(void ***)old)[0])(old, 1);
    *self = *other;
    ++*(int32_t *)(*self + 1);
    return self;
}
static const uint8_t counted_assign_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x06, 0xff, 0x48, 0x04};
REWRITE("Counted::AssignC425E0", 0xc425e0, counted_assign_original, (&counted_assign), &g_counted_assigns);
REWRITE("Counted::AssignC425E0", 0xc5d070, counted_assign_original, (&counted_assign), &g_counted_assigns);

// ---- Versioned::Field: the object's +OFF when the check passes and its version (top half of +8) is at least 1, else 0 ----------------
static volatile long g_versioned_fields;
template <uint32_t OFF> static uint32_t __fastcall versioned_field(uint8_t *self) {
    g_versioned_fields++;
    if (!((uint8_t(__thiscall *)(void *, uint32_t))0xbd1420)(self + 0x10, 0)) return 0;
    uint8_t *object = *(uint8_t **)(self + 0x18);
    if ((int32_t)(*(uint32_t *)(object + 8) & 0xffff0000) < 0x10000) return 0;
    return *(uint32_t *)(object + OFF);
}
static const uint8_t versioned_field_original[] = {0x56, 0x8b, 0xf1, 0x6a, 0x00, 0x8d, 0x4e, 0x10};
REWRITE("Versioned::Field87F330", 0x87f330, versioned_field_original, (&versioned_field<0xdc>), &g_versioned_fields);
REWRITE("Versioned::Field87F330", 0x87f370, versioned_field_original, (&versioned_field<0xe0>), &g_versioned_fields);

// ---- Map::Value: what a lookup in the map at +OFF points at, 0 when the key isn't there -----------------------------------------
static volatile long g_map_values;
template <uint32_t OFF, uint32_t FIND> static uint32_t __thiscall map_value(uint8_t *self, uint32_t key) {
    g_map_values++;
    uint32_t *found = ((uint32_t * (__thiscall *)(void *, uint32_t))FIND)(self + OFF, key);
    return found ? *found : 0;
}
static const uint8_t map_value_original[] = {0x8b, 0x44, 0x24, 0x04, 0x50, 0x83, 0xc1};
REWRITE("Map::ValueBDF160", 0xbdf160, map_value_original, (&map_value<0x28, 0xbdd210>), &g_map_values);
REWRITE("Map::ValueBDF160", 0xbdf180, map_value_original, (&map_value<0x28, 0xbdd210>), &g_map_values);
REWRITE("Map::ValueBDF160", 0xbdf1a0, map_value_original, (&map_value<0x40, 0xbdd290>), &g_map_values);

// ---- Queue::DeleteAll: deletes the object at the front of a global queue until the queue is empty (each delete takes itself off) -----
static volatile long g_queue_deletes;
static void __cdecl queue_delete_all() {
    g_queue_deletes++;
    while (*(int32_t *)0x1306bcc != *(int32_t *)0x1306bd8) {
        void *object = (*(void ***)0x1306bc8)[*(int32_t *)0x1306be0];
        if (object) ((void(__thiscall *)(void *, uint32_t))(*(void ***)object)[0])(object, 1);
    }
}
static const uint8_t queue_delete_original[] = {0xa1, 0xcc, 0x6b, 0x30, 0x01, 0x2b, 0x05, 0xd8, 0x6b, 0x30, 0x01};
REWRITE("Queue::DeleteAll79BFD0", 0x79bfd0, queue_delete_original, (&queue_delete_all), &g_queue_deletes);
REWRITE("Queue::DeleteAll79BFD0", 0x79c010, queue_delete_original, (&queue_delete_all), &g_queue_deletes);

// ---- Global::Free: frees the block a global points at, if any, and clears the global ------------------------------------------
static volatile long g_w4b_global_frees;
template <uint32_t GLOBAL> static void __cdecl w4b_global_free() {
    g_w4b_global_frees++;
    void *block = *(void **)GLOBAL;
    if (block) memory_free(block);
    *(void **)GLOBAL = nullptr;
}
static const uint8_t w4b_global_free_a[] = {0xa1, 0x38, 0x18, 0x34, 0x01, 0x85, 0xc0};
static const uint8_t w4b_global_free_b[] = {0xa1, 0x40, 0x18, 0x34, 0x01, 0x85, 0xc0};
REWRITE("Global::Free9DE660", 0x9de660, w4b_global_free_a, (&w4b_global_free<0x1341838>), &g_w4b_global_frees);
REWRITE("Global::Free9DE660", 0x9df9e0, w4b_global_free_b, (&w4b_global_free<0x1341840>), &g_w4b_global_frees);

// ---- Out::Fill: fills the result the caller passed from the value at +OFF and hands the result back ------------------------------
static volatile long g_w4b_out_fills;
template <uint32_t OFF, uint32_t FILL> static void *__thiscall w4b_out_fill(uint8_t *self, void *out) {
    g_w4b_out_fills++;
    ((void(__fastcall *)(void *, uint32_t))FILL)(out, *(uint32_t *)(self + OFF));
    return out;
}
static const uint8_t w4b_out_fill_a[] = {0x51, 0x8b, 0x51, 0x54, 0x56, 0x8b, 0x74, 0x24, 0x0c};
static const uint8_t w4b_out_fill_b[] = {0x51, 0x8b, 0x51, 0x58, 0x56, 0x8b, 0x74, 0x24, 0x0c};
REWRITE("Out::Fill9DA5E0", 0x9da5e0, w4b_out_fill_a, (&w4b_out_fill<0x54, 0x9da260>), &g_w4b_out_fills);
REWRITE("Out::Fill9DA5E0", 0x9da600, w4b_out_fill_b, (&w4b_out_fill<0x58, 0x9da430>), &g_w4b_out_fills);

// ---- Block::Past: the block at +OFF, moved past its 0x38-byte header unless its first word says it has none -------------------
static volatile long g_w4b_block_pasts;
template <uint32_t OFF, int32_t NONE> static uint8_t *__fastcall w4b_block_past(uint8_t *self) {
    g_w4b_block_pasts++;
    uint8_t *block = *(uint8_t **)(self + OFF);
    if (block && *(int32_t *)block != NONE) block += 0x38;
    return block;
}
static const uint8_t w4b_block_past_a[] = {0x56, 0x8b, 0xb1, 0x38, 0x01, 0x00, 0x00};
static const uint8_t w4b_block_past_b[] = {0x56, 0x8b, 0xb1, 0x70, 0x01, 0x00, 0x00};
REWRITE("Block::Past4A1880", 0x4a1880, w4b_block_past_a, (&w4b_block_past<0x138, -1>), &g_w4b_block_pasts);
REWRITE("Block::Past4A1880", 0x4a18b0, w4b_block_past_b, (&w4b_block_past<0x170, 2>), &g_w4b_block_pasts);

// ---- Bar::Add: asks the part at +0x54 to take an amount; if it can, adds it to the byte at +OFF and updates ----------------------
static volatile long g_w4b_bar_adds;
template <uint32_t OFF, uint32_t TAKE> static uint8_t __thiscall w4b_bar_add(uint8_t *self, uint32_t amount) {
    g_w4b_bar_adds++;
    if (!((uint8_t(__thiscall *)(void *, uint32_t))TAKE)(self + 0x54, amount)) return 0;
    self[OFF] += (uint8_t)amount;
    ((void(__fastcall *)(void *))0x723e50)(self);
    return 1;
}
static const uint8_t w4b_bar_add_original[] = {0x53, 0x8b, 0x5c, 0x24, 0x08, 0x56, 0x8b, 0xf1, 0x53, 0x8d, 0x4e, 0x54};
REWRITE("Bar::Add724490", 0x724490, w4b_bar_add_original, (&w4b_bar_add<0x5c, 0x730020>), &g_w4b_bar_adds);
REWRITE("Bar::Add724490", 0x724540, w4b_bar_add_original, (&w4b_bar_add<0x5d, 0x730400>), &g_w4b_bar_adds);

// ---- Owner::Delete: back to the base table, the member at +OFF destroyed, the object freed when asked --------------------------
static volatile long g_w4b_owner_deletes;
template <uint32_t TABLE, uint32_t OFF, uint32_t MEMBER> static void *__thiscall w4b_owner_delete(uint8_t *self, uint32_t flags) {
    g_w4b_owner_deletes++;
    *(uint32_t *)self = TABLE;
    ((void(__fastcall *)(void *))MEMBER)(self + OFF);
    if (flags & 1) memory_free(self);
    return self;
}
static const uint8_t w4b_owner_delete_original[] = {0x56, 0x8b, 0xf1, 0x8d, 0x4e};
REWRITE("Owner::Delete73EE00", 0x73ee00, w4b_owner_delete_original, (&w4b_owner_delete<0xf6ac88, 0xc, 0x5c24f0>), &g_w4b_owner_deletes);
REWRITE("Owner::Delete73EE00", 0xc33fe0, w4b_owner_delete_original, (&w4b_owner_delete<0xfb06bc, 0x1c, 0xc32e80>), &g_w4b_owner_deletes);

// ---- Pair::Delete: frees the two buffers an object holds, then the object when asked ---------------------------------------------
static volatile long g_w4b_pair_deletes;
template <uint32_t FIRST, uint32_t SECOND> static void *__thiscall w4b_pair_delete(uint8_t *self, uint32_t flags) {
    g_w4b_pair_deletes++;
    memory_free_aligned(*(void **)(self + FIRST));
    memory_free_aligned(*(void **)(self + SECOND));
    if (flags & 1) memory_free(self);
    return self;
}
static const uint8_t w4b_pair_delete_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46};
REWRITE("Pair::DeleteBDD150", 0xbdd150, w4b_pair_delete_original, (&w4b_pair_delete<0x4, 0x8>), &g_w4b_pair_deletes);
REWRITE("Pair::DeleteBDD150", 0xca2a90, w4b_pair_delete_original, (&w4b_pair_delete<0x2c, 0x30>), &g_w4b_pair_deletes);

// ---- Vector::FreeBoth: an array of objects (count in a header before it) destroyed and freed, then the plain buffer beside it -----
static volatile long g_w4b_vector_frees;
template <int32_t HEADER, uint32_t SIZE, uint32_t DTOR> static void __fastcall w4b_vector_free_both(uint8_t **self) {
    g_w4b_vector_frees++;
    uint8_t *items = self[0];
    if (items) {
        ((void(__stdcall *)(void *, uint32_t, int32_t, void *))0xddcf39)(items, SIZE, *(int32_t *)(items - HEADER), (void *)DTOR);
        memory_free_aligned(items - HEADER);
    }
    if (self[1]) memory_free_aligned(self[1]);
}
static const uint8_t w4b_vector_free_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x06, 0x85, 0xc0};
REWRITE("Vector::FreeBoth735AE0", 0x735ae0, w4b_vector_free_original, (&w4b_vector_free_both<4, 0x98, 0x7340b0>), &g_w4b_vector_frees);
REWRITE("Vector::FreeBoth735AE0", 0x87bb70, w4b_vector_free_original, (&w4b_vector_free_both<0x10, 0x80, 0x87b5e0>), &g_w4b_vector_frees);
REWRITE("Vector::FreeBoth735AE0", 0x8e92b0, w4b_vector_free_original, (&w4b_vector_free_both<0x10, 0x130, 0x8e71c0>), &g_w4b_vector_frees);

// ---- Table::Free: back to the base table; the object's array of 0x70-byte items is destroyed, freed and forgotten ---------------
static volatile long g_w4b_table_frees;
template <uint32_t TABLE, uint32_t OFF> static void __fastcall w4b_table_free(uint8_t *self) {
    g_w4b_table_frees++;
    *(uint32_t *)self = TABLE;
    uint8_t *items = *(uint8_t **)(self + OFF);
    if (!items) return;
    ((void(__stdcall *)(void *, uint32_t, int32_t, void *))0xddcf39)(items, 0x70, *(int32_t *)(items - 0x10), (void *)0x73b900);
    memory_free_aligned(items - 0x10);
    *(uint8_t **)(self + OFF) = nullptr;
}
static const uint8_t w4b_table_free_original[] = {0x56, 0x8b, 0xf1, 0xc7, 0x06};
REWRITE("Table::FreeB0E090", 0xb0e090, w4b_table_free_original, (&w4b_table_free<0xf97b38, 0x4>), &g_w4b_table_frees);
REWRITE("Table::FreeB0E090", 0xb790d0, w4b_table_free_original, (&w4b_table_free<0xf9b6d8, 0x10>), &g_w4b_table_frees);

// ---- Shared references: a slot takes the object of another; the old one is released (data at +0xc freed at zero), the new one counted ---
static inline uint32_t *w4b_shared_take(uint32_t **slot, uint32_t **source) {
    if (*source != *slot) {
        uint32_t *old = *slot;
        if (!--*old) {
            memory_free_aligned((void *)(uintptr_t)old[3]);
            memory_free(old);
        }
        *slot = *source;
        ++**source;
    }
    return *source;
}
// Shared::AssignAt: the slot is at +SLOT of the object at +OWNER
static volatile long g_w4b_shared_assigns;
template <uint32_t OWNER, uint32_t SLOT> static uint32_t *__thiscall w4b_shared_assign_at(uint8_t *self, uint32_t **source) {
    g_w4b_shared_assigns++;
    return w4b_shared_take((uint32_t **)(*(uint8_t **)(self + OWNER) + SLOT), source);
}
static const uint8_t w4b_shared_assign_original[] = {0x53, 0x8b, 0x5c, 0x24, 0x08, 0x56, 0x8b, 0xb1};
REWRITE("Shared::AssignAt615D70", 0x615d70, w4b_shared_assign_original, (&w4b_shared_assign_at<0xd8, 0x114>), &g_w4b_shared_assigns);
REWRITE("Shared::AssignAt615D70", 0x6a8180, w4b_shared_assign_original, (&w4b_shared_assign_at<0xdc, 0x124>), &g_w4b_shared_assigns);
// Shared::Refresh: the slot at +SLOT takes what GET finds for the object at +OWNER (the stack argument is unused)
static volatile long g_w4b_shared_refreshes;
template <uint32_t OWNER, uint32_t SLOT, uint32_t GET> static uint32_t *__thiscall w4b_shared_refresh(uint8_t *self, uint32_t) {
    g_w4b_shared_refreshes++;
    uint32_t **found = ((uint32_t **(__fastcall *)(uint32_t))GET)(*(uint32_t *)(self + OWNER));
    return w4b_shared_take((uint32_t **)(self + SLOT), found);
}
static const uint8_t w4b_shared_refresh_original[] = {0x53, 0x56, 0x8b, 0xf1, 0x8b, 0x4e};
REWRITE("Shared::Refresh4D6D80", 0x4d6d80, w4b_shared_refresh_original, (&w4b_shared_refresh<0x34, 0x28, 0x4c0850>), &g_w4b_shared_refreshes);
REWRITE("Shared::Refresh4D6D80", 0x4ec590, w4b_shared_refresh_original, (&w4b_shared_refresh<0x2c, 0x30, 0x4c0850>), &g_w4b_shared_refreshes);
REWRITE("Shared::Refresh4D6D80", 0xb917c0, w4b_shared_refresh_original, (&w4b_shared_refresh<0x28, 0x24, 0xb87790>), &g_w4b_shared_refreshes);

// ---- Owned::FreeAll: destroys and frees every object an array holds (the count is read afresh each time), then the array ---------
static volatile long g_w4b_owned_frees;
template <uint32_t DESTROY> static void __fastcall w4b_owned_free_all(uint32_t *self) {
    g_w4b_owned_frees++;
    for (int32_t i = 0; i < (int32_t)self[1]; i++) {
        void *object = ((void **)(uintptr_t)self[0])[i];
        if (object) {
            ((void(__fastcall *)(void *))DESTROY)(object);
            memory_free(object);
        }
    }
    memory_free_aligned((void *)(uintptr_t)self[0]);
}
static const uint8_t w4b_owned_free_original[] = {0x53, 0x8b, 0xd9, 0x8b, 0x43, 0x04};
REWRITE("Owned::FreeAll811820", 0x811820, w4b_owned_free_original, (&w4b_owned_free_all<0x811760>), &g_w4b_owned_frees);
REWRITE("Owned::FreeAll811820", 0xcdcf10, w4b_owned_free_original, (&w4b_owned_free_all<0xcdbb00>), &g_w4b_owned_frees);

// ---- Owned::DeleteAll: every object an array holds deleted through its table, then the array freed and the count cleared ---------
static volatile long g_w4b_owned_deletes;
template <uint32_t ITEMS, uint32_t COUNT> static void __fastcall w4b_owned_delete_all(uint8_t *self) {
    g_w4b_owned_deletes++;
    void **&items = *(void ***)(self + ITEMS);
    int32_t &count = *(int32_t *)(self + COUNT);
    for (int32_t i = 0; i < count; i++) {
        void *object = items[i];
        if (object) ((void(__thiscall *)(void *, uint32_t))(*(void ***)object)[0])(object, 1);
    }
    if (items) {
        memory_free_aligned(items);
        items = nullptr;
    }
    count = 0;
}
static const uint8_t w4b_owned_delete_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x86};
REWRITE("Owned::DeleteAllA40680", 0xa40680, w4b_owned_delete_original, (&w4b_owned_delete_all<0x240, 0x244>), &g_w4b_owned_deletes);
REWRITE("Owned::DeleteAllA40680", 0xac7d90, w4b_owned_delete_original, (&w4b_owned_delete_all<0xe8, 0xec>), &g_w4b_owned_deletes);

// ---- Matrix::Of: asks the part +8 into the object found for self for the 16-word matrix of the value at +OFF, and copies it to the caller ---
static volatile long g_w4b_matrices;
template <uint32_t OFF> static uint32_t __thiscall w4b_matrix_of(uint8_t *self, uint32_t *out) {
    g_w4b_matrices++;
    uint8_t *owner = ((uint8_t *(__fastcall *)(void *))0xb87f70)(self);
    void *part = owner + 8;
    alignas(16) uint32_t space[16];
    const uint32_t *matrix = ((const uint32_t *(__thiscall *)(void *, void *, uint32_t))(*(void ***)part)[2])(part, space, *(uint32_t *)(self + OFF));
    for (int i = 0; i < 16; i++) out[i] = matrix[i];
    return matrix[15];
}
static const uint8_t w4b_matrix_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x4c, 0x56, 0x8b, 0xf1, 0xe8};
REWRITE("Matrix::OfB88270", 0xb88270, w4b_matrix_original, (&w4b_matrix_of<0x188>), &g_w4b_matrices);
REWRITE("Matrix::OfB88270", 0xb88300, w4b_matrix_original, (&w4b_matrix_of<0x18c>), &g_w4b_matrices);

// ---- Listeners::Notify: every listener of the list at +0x78 whose two keys match (its +0x44 and +0x74) is told -----------------------
static volatile long g_w4b_notifies;
template <uint32_t TELL> static void __thiscall w4b_notify(uint8_t *self, uint32_t first, uint32_t second) {
    g_w4b_notifies++;
    Node *node = *(Node **)(self + 0x78);
    while (node->next) {
        uint8_t *listener = (uint8_t *)ref_interface<0x12f52bc>((uint8_t *)node->value);
        if (listener && *(uint32_t *)(listener + 0x44) == first && *(uint32_t *)(listener + 0x74) == second)
            ((void(__fastcall *)(void *))TELL)(listener);
        node = node->next;
    }
}
static const uint8_t w4b_notify_original[] = {0x8b, 0x41, 0x78, 0x83, 0x38, 0x00, 0x74, 0x7e};
REWRITE("Listeners::Notify54F880", 0x54f880, w4b_notify_original, (&w4b_notify<0x54ef30>), &g_w4b_notifies);
REWRITE("Listeners::Notify54F880", 0x54f910, w4b_notify_original, (&w4b_notify<0x54f0a0>), &g_w4b_notifies);

// ---- Items::Any: whether the check holds, for the two arguments, of any item the object lists (each found through its interface) ------
static volatile long g_w4b_items_any;
template <uint32_t CHECK> static uint8_t __thiscall w4b_items_any(uint8_t *self, uint32_t first, uint32_t second) {
    g_w4b_items_any++;
    for (int32_t i = 0; i < *(int32_t *)(self + 0xe8); i++) {
        void *owner = ((void *(__fastcall *)(void *))0x59acc0)(ref_interface<0x12f54e0>(self + 0x50));
        void *item = ((void *(__thiscall *)(void *, uint32_t))0x5a7300)(owner, (*(uint32_t **)(self + 0xe4))[i]);
        if (((uint8_t(__thiscall *)(void *, uint32_t, uint32_t))CHECK)(item, first, second)) return 1;
    }
    return 0;
}
static const uint8_t w4b_items_any_original[] = {0x53, 0x55, 0x56, 0x8b, 0xf1, 0x8b, 0x86, 0xe8};
REWRITE("Items::Any5D6AC0", 0x5d6ac0, w4b_items_any_original, (&w4b_items_any<0x5738d0>), &g_w4b_items_any);
REWRITE("Items::Any5D6AC0", 0x5d6de0, w4b_items_any_original, (&w4b_items_any<0x56a2e0>), &g_w4b_items_any);

// ---- Chain::Clear: walks a chain from the object at +8 up to the one at +0x14, or until it ends or comes round; what each holds for the key is cleared ---
static volatile long g_w4b_chain_clears;
template <uint32_t CLEAR> static void __thiscall w4b_chain_clear(uint8_t *self, uint32_t key) {
    g_w4b_chain_clears++;
    typedef void *(__fastcall * Held)(void *);
    uint8_t *link = *(uint8_t **)(self + 8);
    while (link) {
        if (((Held)0x5c00c0)(link)) ((void(__thiscall *)(void *, uint32_t))CLEAR)(((Held)0x5c00c0)(link), key);
        if (link == *(uint8_t **)(self + 0x14)) return;
        link = (uint8_t *)ref_interface<0x12f5608>(link + 0x158);
        if (link == *(uint8_t **)(self + 8)) return;
    }
}
static const uint8_t w4b_chain_clear_original[] = {0x55, 0x8b, 0xe9, 0x56, 0x8b, 0x75, 0x08};
REWRITE("Chain::Clear5D55F0", 0x5d55f0, w4b_chain_clear_original, (&w4b_chain_clear<0x5cf670>), &g_w4b_chain_clears);
REWRITE("Chain::Clear5D55F0", 0x5d5690, w4b_chain_clear_original, (&w4b_chain_clear<0x5cf690>), &g_w4b_chain_clears);

// ---- Members: objects that join a global list on construction (the list made on first use) and leave it on destruction (freed once empty) ---
struct W4bMembers {
    Node first;   // +0: its next is the first member, or the end
    Node end;     // +0xc: its prev is the last member, or the first node
};
static volatile long g_w4b_joins;
template <uint32_t TABLE, uint32_t LIST> static uint8_t *__fastcall w4b_member_join(uint8_t *self) {
    g_w4b_joins++;
    *(uint32_t *)self = TABLE;
    Node *node = (Node *)(self + 4);
    node->value = self;
    W4bMembers *&list = *(W4bMembers **)LIST;
    if (!list) {
        W4bMembers *made = (W4bMembers *)memory_alloc(sizeof(W4bMembers));
        made->first.value = nullptr;
        made->end.value = nullptr;
        made->first.prev = nullptr;
        made->first.next = &made->end;
        made->end.prev = &made->first;
        made->end.next = nullptr;
        list = made;
    }
    Node *last = list->end.prev;
    node->next = last->next;
    last->next->prev = node;
    node->prev = last;
    last->next = node;
    return self;
}
static const uint8_t w4b_member_join_original[] = {0x56, 0x8b, 0xf1, 0xc7, 0x06};
REWRITE("Members::Join49CC80", 0x49cc80, w4b_member_join_original, (&w4b_member_join<0xf30550, 0x12f4aec>), &g_w4b_joins);
REWRITE("Members::Join49CC80", 0xc07bb0, w4b_member_join_original, (&w4b_member_join<0xfa6270, 0x13d322c>), &g_w4b_joins);
static volatile long g_w4b_leaves;
template <uint32_t TABLE, uint32_t LIST> static void __fastcall w4b_member_leave(uint8_t *self) {
    g_w4b_leaves++;
    *(uint32_t *)self = TABLE;
    unlink((Node *)(self + 4));
    W4bMembers *&list = *(W4bMembers **)LIST;
    if (!list->first.next->next) {
        memory_free(list);
        list = nullptr;
    }
}
static const uint8_t w4b_member_leave_original[] = {0x8d, 0x41, 0x04, 0x85, 0xc0, 0xc7, 0x01};
REWRITE("Members::Leave49CCF0", 0x49ccf0, w4b_member_leave_original, (&w4b_member_leave<0xf30550, 0x12f4aec>), &g_w4b_leaves);
REWRITE("Members::Leave49CCF0", 0xc07c20, w4b_member_leave_original, (&w4b_member_leave<0xfa6270, 0x13d322c>), &g_w4b_leaves);

// ---- Grid::Neighbours: a cell's four links cleared, then joined to each neighbour inside the grid (left, right, up, down) ---------------
static volatile long g_w4b_neighbours;
template <uint32_t JOIN> static void __thiscall w4b_grid_neighbours(uint8_t *self, uint8_t *cell) {
    g_w4b_neighbours++;
    typedef uint32_t(__thiscall * Join)(void *, void *, uint32_t, uint32_t);
    uint32_t *links = (uint32_t *)(cell + 0x64);
    links[0] = links[1] = links[2] = links[3] = 0;
    auto at = [self](uint32_t x, uint32_t y) { return (*(uint32_t **)(self + 0x58))[y * self[0x54] + x]; };
    uint8_t &x = cell[0x88], &y = cell[0x89];
    if (x > 0) links[0] = ((Join)JOIN)(self, cell, at(x - 1, y), 0);
    if (x < self[0x5c]) links[1] = ((Join)JOIN)(self, cell, at(x + 1, y), 1);
    if (y > 0) links[2] = ((Join)JOIN)(self, cell, at(x, y - 1), 2);
    if (y < self[0x5d]) links[3] = ((Join)JOIN)(self, cell, at(x, y + 1), 3);
}
static const uint8_t w4b_grid_neighbours_original[] = {0x53, 0x56, 0x8b, 0x74, 0x24, 0x0c, 0x33, 0xc0, 0x57};
REWRITE("Grid::Neighbours7238A0", 0x7238a0, w4b_grid_neighbours_original, (&w4b_grid_neighbours<0x7221e0>), &g_w4b_neighbours);
REWRITE("Grid::Neighbours7238A0", 0x723990, w4b_grid_neighbours_original, (&w4b_grid_neighbours<0x7223a0>), &g_w4b_neighbours);
REWRITE("Grid::Neighbours7238A0", 0x723a80, w4b_grid_neighbours_original, (&w4b_grid_neighbours<0x7227f0>), &g_w4b_neighbours);

// ---- Kept::Then: the interface kept in the reference at +REF (looked up the first time) handed on to the next function ------------------
static volatile long g_w4b_kept_thens;
template <uint32_t REF, uint32_t ID, uint32_t THEN> static uint32_t __fastcall w4b_kept_then(uint8_t *self) {
    g_w4b_kept_thens++;
    return ((uint32_t(__fastcall *)(void *))THEN)(ref_interface<ID>(self + REF));
}
static const uint8_t w4b_kept_then_original[] = {0x57, 0x8b, 0xf9, 0x8a, 0x47};
REWRITE("Kept::Then4C5890", 0x4c5890, w4b_kept_then_original, (&w4b_kept_then<0x5c, 0x12f5300, 0x4d1840>), &g_w4b_kept_thens);
REWRITE("Kept::Then4C5890", 0x5efd60, w4b_kept_then_original, (&w4b_kept_then<0x40, 0x12f565c, 0x5ec200>), &g_w4b_kept_thens);

// ---- Once::Make: takes two values, makes the shared table the first time (its bit in a global says it's made) and points at it. The table's
// sixteen arguments are two-byte pairs, (FIRST, 0), (SECOND, 0) then all 0xff; the other fourteen words passed here are not used ----------
struct W4bUnusedWords {
    uint32_t words[14];
};
static volatile long g_w4b_once_makes;
template <uint32_t MADE, uint32_t TABLE, uint32_t SECOND, uint32_t FIRST>
static void *__thiscall w4b_once_make(uint32_t *self, uint32_t *a, uint32_t *b, W4bUnusedWords) {
    g_w4b_once_makes++;
    self[1] = *a;
    self[2] = *b;
    uint32_t &made = *(uint32_t *)MADE;
    if (!(made & 1)) {
        made |= 1;
        const uint32_t ff = 0xffff;
        ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
                             uint32_t, uint32_t, uint32_t, uint32_t))0x416110)((void *)TABLE, FIRST, SECOND, ff, ff, ff, ff, ff, ff, ff, ff, ff, ff, ff, ff, ff, ff);
    }
    self[0] = TABLE;
    return self;
}
static const uint8_t w4b_once_make_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0x54, 0x24, 0x08, 0x83, 0xec, 0x38, 0x56, 0x8b, 0xf1};
REWRITE("Once::Make696000", 0x696000, w4b_once_make_original, (&w4b_once_make<0x12f8678, 0x12f8650, 0x1d, 7>), &g_w4b_once_makes);
REWRITE("Once::Make696000", 0x8c8a70, w4b_once_make_original, (&w4b_once_make<0x1311360, 0x1311338, 4, 6>), &g_w4b_once_makes);
REWRITE("Once::Make696000", 0xc73150, w4b_once_make_original, (&w4b_once_make<0x13e6d38, 0x13e6d10, 0x1d, 6>), &g_w4b_once_makes);

// ---- Matrix::Refresh: without a live source (the handle at +0x510) takes the first path; with one, either the second path or, when the bit is set, a fresh
// copy of the 16-word matrix the current view gives (the stack argument is unused) --------------------------------------------------------
static volatile long g_w4b_refreshes;
template <uint32_t BIT> static uint32_t __thiscall w4b_matrix_refresh(uint8_t *self, uint32_t) {
    g_w4b_refreshes++;
    typedef uint32_t(__fastcall * Path)(void *);
    uint8_t *handle = *(uint8_t **)(self + 0x510);
    void *live = nullptr;
    if (*(void **)(handle + 0xc)) {
        void *source = find_interface(handle, (const int32_t *)0x12f4f44);
        live = ((void *(__fastcall *)(void *))(*(void ***)source)[5])(source);
    }
    if (!live) return ((Path)0x7a0000)(self);
    if (!(self[0x408] & BIT)) return ((Path)0x7a0480)(self);
    void *view = ((void *(__fastcall *)(void *))0x79e260)(self);
    const uint32_t *matrix = ((const uint32_t *(__fastcall *)(void *))(*(void ***)view)[7])(view);
    uint32_t *kept = (uint32_t *)(self + 0x50);
    for (int i = 0; i < 16; i++) kept[i] = matrix[i];
    return (uint32_t)(uintptr_t)matrix;
}
static const uint8_t w4b_matrix_refresh_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x86, 0x10, 0x05, 0x00, 0x00};
REWRITE("Matrix::Refresh7A0FD0", 0x7a0fd0, w4b_matrix_refresh_original, (&w4b_matrix_refresh<2>), &g_w4b_refreshes);
REWRITE("Matrix::Refresh7A0FD0", 0x7a10b0, w4b_matrix_refresh_original, (&w4b_matrix_refresh<1>), &g_w4b_refreshes);

// ---- Scanner::Switch: the scanner moves to another input buffer. The current one (if any) gets its position and count back, with the byte held
// out of it restored; the new one's position, count and file become the scanner's and its first byte is held --------------------------------
static volatile long g_w4b_switches;
template <uint32_t CURRENT, uint32_t HOLD, uint32_t POSITION, uint32_t COUNT, uint32_t TEXT, uint32_t FILE_, uint32_t FRESH>
static void __fastcall w4b_scanner_switch(uint32_t *buffer) {
    g_w4b_switches++;
    uint32_t *&current = *(uint32_t **)CURRENT;
    if (current == buffer) return;
    uint8_t *&position = *(uint8_t **)POSITION;
    if (current) {
        *position = *(uint8_t *)HOLD;
        current[2] = (uint32_t)(uintptr_t)position;
        current[4] = *(uint32_t *)COUNT;
    }
    *(uint32_t *)COUNT = buffer[4];
    uint8_t *start = (uint8_t *)(uintptr_t)buffer[2];
    *(uint8_t **)TEXT = start;
    current = buffer;
    *(uint32_t *)FILE_ = buffer[0];
    position = start;
    *(uint8_t *)HOLD = *start;
    *(uint32_t *)FRESH = 1;
}
static const uint8_t w4b_scanner_switch_a[] = {0xa1, 0xf8, 0x32, 0x3e, 0x01, 0x3b, 0xc1};
static const uint8_t w4b_scanner_switch_b[] = {0xa1, 0x14, 0x70, 0x3f, 0x01, 0x3b, 0xc1};
REWRITE("Scanner::SwitchC0AC90", 0xc0ac90, w4b_scanner_switch_a, (&w4b_scanner_switch<0x13e32f8, 0x13d32c0, 0x13e32fc, 0x13e32e8, 0x13e32e4, 0x13e3304, 0x13e32f0>), &g_w4b_switches);
REWRITE("Scanner::SwitchC0AC90", 0xcbc740, w4b_scanner_switch_b, (&w4b_scanner_switch<0x13f7014, 0x13e6fe8, 0x13f7018, 0x13f7004, 0x13f6ffc, 0x13f7020, 0x13f700c>), &g_w4b_switches);

// ======== third batch ========
// Functions with an exception frame start the same way, apart from the handler they name.
template <uint32_t HANDLER> static const uint8_t w4c_seh[] = {0x6a, 0xff, 0x68, (uint8_t)HANDLER, (uint8_t)(HANDLER >> 8), (uint8_t)(HANDLER >> 16), 0x00, 0x64, 0xa1, 0, 0, 0, 0};
static inline void w4c_destroy_owned(void *owned) {
    if (owned) ((void(__thiscall *)(void *, uint32_t))(*(void ***)owned)[0])(owned, 1);
}

// ---- Part::Destroy: destroys the part at +4, then falls back to the plain table --------------------------------------------------
static volatile long g_w4c_part_destroys;
template <uint32_t PART> static void __fastcall w4c_part_destroy(uint32_t *self) {
    g_w4c_part_destroys++;
    ((void(__fastcall *)(void *))PART)(self + 1);
    self[0] = 0xf2694c;
}
REWRITE("Part::Destroy6A5A80", 0x6a5a80, (w4c_seh<0xe3d4e8>), (&w4c_part_destroy<0xcd2b50>), &g_w4c_part_destroys);
REWRITE("Part::Destroy6A5A80", 0x79b440, (w4c_seh<0xe55398>), (&w4c_part_destroy<0x8fb110>), &g_w4c_part_destroys);
REWRITE("Part::Destroy6A5A80", 0x9e4740, (w4c_seh<0xe76558>), (&w4c_part_destroy<0xcd2b50>), &g_w4c_part_destroys);
REWRITE("Part::Destroy6A5A80", 0xb830f0, (w4c_seh<0xe98828>), (&w4c_part_destroy<0xbc2cb0>), &g_w4c_part_destroys);

// ---- Handle::Named: fills a handle to a fresh 16-byte reference to a fixed name (none without memory) ---------------------------------
static volatile long g_w4c_nameds;
template <uint32_t NAME> static uint32_t *__thiscall w4c_handle_named(uint32_t *out) {
    g_w4c_nameds++;
    void *memory = memory_alloc(0x10);
    out[0] = memory ? ((uint32_t(__thiscall *)(void *, uint32_t))0xbcf850)(memory, NAME) : 0;
    return out;
}
REWRITE("Handle::Named453340", 0x453340, (w4c_seh<0xe0a71b>), (&w4c_handle_named<0xf28e94>), &g_w4c_nameds);
REWRITE("Handle::Named453340", 0x49d370, (w4c_seh<0xe0fd3b>), (&w4c_handle_named<0xf3058c>), &g_w4c_nameds);
REWRITE("Handle::Named453340", 0x6248d0, (w4c_seh<0xe2cfdb>), (&w4c_handle_named<0x10ec6b0>), &g_w4c_nameds);
REWRITE("Handle::Named453340", 0xa52ac0, (w4c_seh<0xe813eb>), (&w4c_handle_named<0xf8ea44>), &g_w4c_nameds);

// ---- Five-table destructor: sets its five tables, destroys what it owns at +0x154, then runs the base destructor ---------------
static volatile long g_w4c_five_destroys;
template <uint32_t T0, uint32_t T1, uint32_t T2, uint32_t T3, uint32_t T4> static void __fastcall w4c_five_destroy(uint8_t *self) {
    g_w4c_five_destroys++;
    *(uint32_t *)self = T0;
    *(uint32_t *)(self + 4) = T1;
    *(uint32_t *)(self + 0xd4) = T2;
    *(uint32_t *)(self + 0xe8) = T3;
    *(uint32_t *)(self + 0xf0) = T4;
    w4c_destroy_owned(*(void **)(self + 0x154));
    ((void(__fastcall *)(void *))0x63a030)(self);
}
REWRITE("Five::Destroy6CC840", 0x6cc840, (w4c_seh<0xe41d08>), (&w4c_five_destroy<0xf63348, 0xf63330, 0xf6332c, 0xf63318, 0xf632ec>), &g_w4c_five_destroys);
REWRITE("Five::Destroy6CC840", 0x6e9270, (w4c_seh<0xe46468>), (&w4c_five_destroy<0xf661f8, 0xf661e0, 0xf661dc, 0xf661c8, 0xf6619c>), &g_w4c_five_destroys);
REWRITE("Five::Destroy6CC840", 0x7eec40, (w4c_seh<0xe59f98>), (&w4c_five_destroy<0xf73ec8, 0xf73eb0, 0xf73eac, 0xf73e98, 0xf73e6c>), &g_w4c_five_destroys);
REWRITE("Five::Destroy6CC840", 0x97c770, (w4c_seh<0xe6c958>), (&w4c_five_destroy<0xf7ee10, 0xf7edfc, 0xf7edf8, 0xf7ede4, 0xf7edb8>), &g_w4c_five_destroys);

// ---- Three-table destructor: destroys what it owns at +OWNED, the member at +0xd4, then the base --------------------------------------------
static volatile long g_w4c_three_destroys;
template <uint32_t T0, uint32_t T1, uint32_t T2, uint32_t OWNED, uint32_t MEMBER> static void __fastcall w4c_three_destroy(uint8_t *self) {
    g_w4c_three_destroys++;
    *(uint32_t *)self = T0;
    *(uint32_t *)(self + 4) = T1;
    *(uint32_t *)(self + 0xd4) = T2;
    w4c_destroy_owned(*(void **)(self + OWNED));
    ((void(__fastcall *)(void *))MEMBER)(self + 0xd4);
    ((void(__fastcall *)(void *))0x6b0ea0)(self);
}
REWRITE("Three::Destroy623BC0", 0x623bc0, (w4c_seh<0xe2cea1>), (&w4c_three_destroy<0xf44490, 0xf44478, 0xf44464, 0xf0, 0x6835a0>), &g_w4c_three_destroys);
REWRITE("Three::Destroy623BC0", 0x6ea7b0, (w4c_seh<0xe46781>), (&w4c_three_destroy<0xf66728, 0xf66710, 0xf6670c, 0xe4, 0x440ed0>), &g_w4c_three_destroys);
REWRITE("Three::Destroy623BC0", 0xa21410, (w4c_seh<0xe7ad11>), (&w4c_three_destroy<0xf885c0, 0xf885a8, 0xf8859c, 0xf4, 0x67d000>), &g_w4c_three_destroys);
REWRITE("Three::Destroy623BC0", 0xa21ab0, (w4c_seh<0xe7ae01>), (&w4c_three_destroy<0xf887e8, 0xf887d0, 0xf887c4, 0xf4, 0x67d000>), &g_w4c_three_destroys);

// ---- Four-table destructor: destroys the member at +MEMBER, then the base ------------------------------------------------------------
static volatile long g_w4c_four_destroys;
template <uint32_t T0, uint32_t T1, uint32_t T2, uint32_t T3, uint32_t MEMBER> static void __fastcall w4c_four_destroy(uint8_t *self) {
    g_w4c_four_destroys++;
    *(uint32_t *)self = T0;
    *(uint32_t *)(self + 4) = T1;
    *(uint32_t *)(self + 0xd4) = T2;
    *(uint32_t *)(self + 0xdc) = T3;
    ((void(__fastcall *)(void *))0x40f730)(self + MEMBER);
    ((void(__fastcall *)(void *))0x67b9f0)(self);
}
REWRITE("Four::DestroyA80350", 0xa80350, (w4c_seh<0xe83ea8>), (&w4c_four_destroy<0xf900c8, 0xf900b4, 0xf900a0, 0xf9009c, 0x13c>), &g_w4c_four_destroys);
REWRITE("Four::DestroyA80350", 0xa85640, (w4c_seh<0xe84888>), (&w4c_four_destroy<0xf910c0, 0xf910a8, 0xf91094, 0xf91090, 0x144>), &g_w4c_four_destroys);
REWRITE("Four::DestroyA80350", 0xac5300, (w4c_seh<0xe88cb8>), (&w4c_four_destroy<0xf936b0, 0xf9369c, 0xf93688, 0xf93684, 0x140>), &g_w4c_four_destroys);
REWRITE("Four::DestroyA80350", 0xb0f7d0, (w4c_seh<0xe90598>), (&w4c_four_destroy<0xf97cb0, 0xf97c98, 0xf97c84, 0xf97c80, 0x13c>), &g_w4c_four_destroys);

// ---- Holder destructor: the sub-object at +SUB goes back to its base table, what it owned at the next word is destroyed and the base runs ---
static volatile long g_w4c_holder_destroys;
template <uint32_t T0, uint32_t T1, uint32_t T2, uint32_t SUB, uint32_t BASE> static void __fastcall w4c_holder_destroy(uint8_t *self) {
    g_w4c_holder_destroys++;
    *(uint32_t *)self = T0;
    *(uint32_t *)(self + 4) = T1;
    *(uint32_t *)(self + 0xd4) = T2;
    *(uint32_t *)(self + SUB) = 0xf80c2c;
    w4c_destroy_owned(*(void **)(self + SUB + 4));
    ((void(__fastcall *)(void *))BASE)(self);
}
REWRITE("Holder::Destroy99B1F0", 0x99b1f0, (w4c_seh<0xe6f4f8>), (&w4c_holder_destroy<0xf80d88, 0xf80d74, 0xf80d68, 0x12c, 0x9c14e0>), &g_w4c_holder_destroys);
REWRITE("Holder::Destroy99B1F0", 0x99b320, (w4c_seh<0xe6f518>), (&w4c_holder_destroy<0xf80ec0, 0xf80ea8, 0xf80e9c, 0x114, 0xad6a10>), &g_w4c_holder_destroys);
REWRITE("Holder::Destroy99B1F0", 0x99b3d0, (w4c_seh<0xe6f538>), (&w4c_holder_destroy<0xf80ff8, 0xf80fe0, 0xf80fd4, 0x114, 0xaefe80>), &g_w4c_holder_destroys);
REWRITE("Holder::Destroy99B1F0", 0x9a3bf0, (w4c_seh<0xe70598>), (&w4c_holder_destroy<0xf81c30, 0xf81c1c, 0xf81c10, 0x114, 0xaf2010>), &g_w4c_holder_destroys);

// ---- Mode::Handle: in the one mode (+0x190 is MODE) gives a handle to the global name, or the default handle without one; otherwise asks FALLBACK --
static volatile long g_w4c_mode_handles;
template <uint32_t MODE, uint32_t FALLBACK, uint32_t NAME> static uint32_t *__thiscall w4c_mode_handle(uint8_t *self, uint32_t *out) {
    g_w4c_mode_handles++;
    if (*(uint32_t *)(self + 0x190) != MODE) {
        ((void(__thiscall *)(void *, void *))FALLBACK)(self, out);
        return out;
    }
    uint32_t name = *(uint32_t *)NAME;
    if (!name) {
        out[0] = ((uint32_t(__fastcall *)(uint8_t))0xbd0490)(1);
        return out;
    }
    void *memory = memory_alloc(0x10);
    out[0] = memory ? ((uint32_t(__thiscall *)(void *, uint32_t))0xbcf7d0)(memory, name) : 0;
    return out;
}
REWRITE("Mode::Handle756B00", 0x756b00, (w4c_seh<0xe4ff5b>), (&w4c_mode_handle<0xd, 0x7c8b10, 0x12fb234>), &g_w4c_mode_handles);
REWRITE("Mode::Handle756B00", 0x799ff0, (w4c_seh<0xe551ab>), (&w4c_mode_handle<0xd, 0x7c8b10, 0x12f8fc0>), &g_w4c_mode_handles);
REWRITE("Mode::Handle756B00", 0x7e98a0, (w4c_seh<0xe5979b>), (&w4c_mode_handle<0xd, 0x7c8b10, 0x12f8fc0>), &g_w4c_mode_handles);
REWRITE("Mode::Handle756B00", 0x7f8ef0, (w4c_seh<0xe5af2b>), (&w4c_mode_handle<0x16, 0x75cdf0, 0x12f98b8>), &g_w4c_mode_handles);

// ---- Registered::Make: builds the object; with no owner given uses the shared one. An owner not yet ready registers a key made from the
// offset of the part at +4; the shared owner is then marked ready ---------------------------------------------------------------------
static volatile long g_w4c_registereds;
template <uint32_t T0, uint32_t T1, uint32_t SHARED> static uint32_t *__thiscall w4c_registered_make(uint32_t *self, uint8_t *owner) {
    g_w4c_registereds++;
    self[0] = 0xf2694c;
    ((void(__thiscall *)(void *, void *))0x967c10)(self + 1, self);
    self[0] = T0;
    self[1] = T1;
    bool shared = !owner;
    if (shared) owner = *(uint8_t **)SHARED;
    if (!owner[4]) {
        uint32_t key[5];
        void *made = ((void *(__thiscall *)(void *, uint32_t, uint32_t))0xbeca40)(key, 0x133d070, 4);
        ((void(__thiscall *)(void *, void *))0xbecc10)(owner, made);
        ((void(__fastcall *)(void *))0xbec5e0)(key);
    }
    if (shared) (*(uint8_t **)SHARED)[4] = 1;
    return self;
}
REWRITE("Registered::Make968000", 0x968000, (w4c_seh<0xe6b7eb>), (&w4c_registered_make<0xf7e6d0, 0xf7e6bc, 0x133d090>), &g_w4c_registereds);
REWRITE("Registered::Make968000", 0x976ec0, (w4c_seh<0xe6c3db>), (&w4c_registered_make<0xf7eb00, 0xf7eaec, 0x133f6dc>), &g_w4c_registereds);
REWRITE("Registered::Make968000", 0x977210, (w4c_seh<0xe6c46b>), (&w4c_registered_make<0xf7eb44, 0xf7eb30, 0x133f6e4>), &g_w4c_registereds);
REWRITE("Registered::Make968000", 0x977e40, (w4c_seh<0xe6c56b>), (&w4c_registered_make<0xf7ebb0, 0xf7eb9c, 0x133f6f4>), &g_w4c_registereds);
REWRITE("Registered::Make968000", 0x9782b0, (w4c_seh<0xe6c5db>), (&w4c_registered_make<0xf7ebe8, 0xf7ebd4, 0x133f6fc>), &g_w4c_registereds);
REWRITE("Registered::Make968000", 0x97a430, (w4c_seh<0xe6c7cb>), (&w4c_registered_make<0xf7ecd8, 0xf7ecc4, 0x134014c>), &g_w4c_registereds);

// ---- Refs::Total: adds up what one virtual of each live reference in the list at +0xcc (count +0xd0) says -----------------------------------
static volatile long g_w4c_totals;
template <uint32_t SLOT> static uint32_t __fastcall w4c_refs_total(uint8_t *self) {
    g_w4c_totals++;
    uint32_t total = 0;
    for (int32_t i = 0; i < *(int32_t *)(self + 0xd0); i++) {
        void *live = ref_interface<0x13cde74>(*(uint8_t **)(self + 0xcc) + i * 0x1c);
        if (live) total += ((uint32_t(__fastcall *)(void *))(*(void ***)live)[SLOT])(live);
    }
    return total;
}
static const uint8_t w4c_refs_total_original[] = {0x83, 0xec, 0x08, 0x53, 0x55, 0x8b, 0xe9, 0x8b, 0x8d, 0xd0};
REWRITE("Refs::TotalA898F0", 0xa898f0, w4c_refs_total_original, (&w4c_refs_total<0xe4 / 4>), &g_w4c_totals);
REWRITE("Refs::TotalA898F0", 0xa899f0, w4c_refs_total_original, (&w4c_refs_total<0xe8 / 4>), &g_w4c_totals);

// ---- Once::Make3: as Once::Make, with three values kept; the table's first three pairs are (0, 0), (SECOND, 0), (FIRST, 0), then all 0xff ------
struct W4cUnusedWords {
    uint32_t words[13];
};
static volatile long g_w4c_once_makes;
template <uint32_t MADE, uint32_t TABLE, uint32_t FIRST, uint32_t SECOND>
static void *__thiscall w4c_once_make(uint32_t *self, uint32_t *a, uint32_t *b, uint32_t *c, W4cUnusedWords) {
    g_w4c_once_makes++;
    self[1] = *a;
    self[2] = *b;
    self[3] = *c;
    uint32_t &made = *(uint32_t *)MADE;
    if (!(made & 1)) {
        made |= 1;
        const uint32_t ff = 0xffff;
        ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
                             uint32_t, uint32_t, uint32_t, uint32_t))0x416110)((void *)TABLE, 0, SECOND, FIRST, ff, ff, ff, ff, ff, ff, ff, ff, ff, ff, ff, ff, ff);
    }
    self[0] = TABLE;
    return self;
}
static const uint8_t w4c_once_make_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0x54, 0x24, 0x08, 0x83, 0xec, 0x34, 0x56, 0x8b, 0xf1};
REWRITE("Once::Make71F340", 0x71f340, w4c_once_make_original, (&w4c_once_make<0x130632c, 0x1306304, 4, 2>), &g_w4c_once_makes);
REWRITE("Once::Make71F340", 0x9e6500, w4c_once_make_original, (&w4c_once_make<0x134192c, 0x1341904, 2, 6>), &g_w4c_once_makes);

// ---- Place::At: places the object at a position (an identity matrix moved there), sets the two global sizes, then carries on with THEN ---------
static volatile long g_w4c_places;
template <uint32_t THEN> static uint32_t __fastcall w4c_place_at(void *self, const uint32_t *position, uint32_t a, uint32_t b, uint32_t c) {
    g_w4c_places++;
    const uint32_t one = 0x3f800000;   // 1.0f
    alignas(16) uint32_t matrix[16] = {one, 0, 0, 0, 0, one, 0, 0, 0, 0, one, 0, position[0], position[1], position[2], one};
    ((void(__thiscall *)(void *, uint32_t, void *))(*(void ***)self)[0xb8 / 4])(self, 0, matrix);
    ((void(__thiscall *)(void *, uint32_t, uint32_t))(*(void ***)self)[0xac / 4])(self, *(uint32_t *)0x13d28a0, *(uint32_t *)0x13d0e34);
    return ((uint32_t(__fastcall *)(void *, uint32_t, uint32_t, uint32_t))THEN)(self, a, b, c);
}
static const uint8_t w4c_place_at_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x4c};
REWRITE("Place::AtBF7130", 0xbf7130, w4c_place_at_original, (&w4c_place_at<0xbf6ff0>), &g_w4c_places);
REWRITE("Place::AtBF7130", 0xbf8980, w4c_place_at_original, (&w4c_place_at<0xbf8850>), &g_w4c_places);

// ---- Big::Run: runs WORK on a large cleared job held on the stack -------------------------------------------------------------------------------
static volatile long g_w4c_big_runs;
template <uint32_t WORK> static uint32_t __thiscall w4c_big_run(void *self, uint32_t argument) {
    g_w4c_big_runs++;
    uint32_t job[0x113c / 4];
    job[0] = 0;
    job[1] = 0;
    job[3] = 0xf264ad;
    job[0x1010 / 4] = 0;
    for (uint32_t i = 0x1114 / 4; i <= 0x1138 / 4; i++) job[i] = 0;
    return ((uint32_t(__thiscall *)(void *, void *, uint32_t))WORK)(job, self, argument);
}
static const uint8_t w4c_big_run_original[] = {0xb8, 0x40, 0x11, 0x00, 0x00, 0xe8};
REWRITE("Big::RunC161E0", 0xc161e0, w4c_big_run_original, (&w4c_big_run<0xc160b0>), &g_w4c_big_runs);
REWRITE("Big::RunC161E0", 0xc16280, w4c_big_run_original, (&w4c_big_run<0xc16180>), &g_w4c_big_runs);

// ======== fourth batch ========
// ---- Pack::Send: packs the value at OFF (the packer's fifteen other outputs are not wanted) and hands it, with the word at +0xdc, to the renderer
static volatile long g_w4d_sends;
typedef uint8_t W4dB;
template <uint32_t OFF> static uint32_t __fastcall w4d_pack_send(uint8_t *self) {
    g_w4d_sends++;
    uint32_t value = *(uint32_t *)(self + OFF);
    uint32_t key = *(uint32_t *)(self + 0xdc);
    void *renderer = *(void **)0x13d0db8;
    void *slot = (*(void ***)renderer)[0xb0 / 4];
    uint8_t unused;
    uint32_t packed[3];
    W4dB *u = &unused;
    uint32_t made = ((uint32_t(__thiscall *)(void *, uint32_t *, W4dB *, W4dB *, W4dB *, W4dB *, W4dB *, W4dB *, W4dB *, W4dB *, W4dB *, W4dB *, W4dB *, W4dB *, W4dB *, W4dB *,
                                             W4dB *))0x431070)(packed, &value, u, u, u, u, u, u, u, u, u, u, u, u, u, u, u);
    return ((uint32_t(__thiscall *)(void *, uint32_t, uint32_t))slot)(renderer, key, made);
}
static const uint8_t w4d_pack_send_original[] = {0x83, 0xec, 0x20, 0x8b, 0x81};
REWRITE("Pack::Send949090", 0x949090, w4d_pack_send_original, (&w4d_pack_send<0xe0>), &g_w4d_sends);
REWRITE("Pack::Send949090", 0x949120, w4d_pack_send_original, (&w4d_pack_send<0xe4>), &g_w4d_sends);

// ---- Parts::FindKind: the first part (list at +0x60, count +0x48, 20 bytes each) whose interface 0x130654c has kind KIND at +0x190 -----------
// (a part with no such interface is read at address 0x190, as the original does)
static volatile long g_w4d_finds;
template <int32_t KIND> static uint8_t *__fastcall w4d_find_kind(uint8_t *self) {
    g_w4d_finds++;
    for (int32_t i = 0; i < *(int32_t *)(self + 0x48); i++) {
        uint8_t *part = (uint8_t *)find_interface(*(uint8_t **)(*(uint8_t **)(self + 0x60) + i * 0x14 + 4), (const int32_t *)0x130654c);
        if (*(int32_t *)(part + 0x190) == KIND) return part;
    }
    return nullptr;
}
static const uint8_t w4d_find_kind_original[] = {0x53, 0x55, 0x56, 0x57, 0x8b, 0xf9, 0x8b, 0x47, 0x48};
REWRITE("Parts::FindKind7743D0", 0x7743d0, w4d_find_kind_original, (&w4d_find_kind<8>), &g_w4d_finds);
REWRITE("Parts::FindKind7743D0", 0x774440, w4d_find_kind_original, (&w4d_find_kind<0xc>), &g_w4d_finds);

// ---- Slots::Match: do two pieces' slots (+0xa4) meet? Same side and slot with the side bit 2 set agrees at once unless forced; otherwise both
// slots are converted (the second from the other end) and must come out equal -------------------------------------------------------------------
static volatile long g_w4d_matches;
template <uint32_t MODE> static bool __stdcall w4d_slots_match(uint8_t *a, uint8_t *b, uint32_t end_word, uint32_t forced_word) {
    uint8_t end = (uint8_t)end_word, forced = (uint8_t)forced_word;   // bytes passed in words
    g_w4d_matches++;
    typedef bool(__fastcall * Convert)(uint8_t, uint8_t, uint32_t *, uint32_t);
    uint32_t slot_a = *(uint32_t *)(a + 0xa4), slot_b = *(uint32_t *)(b + 0xa4);
    if (!forced && a[0x8d] == b[0x8d] && slot_a == slot_b && ((a[0x8d] ^ end) & 2)) return true;
    if (!((Convert)0x71b0f0)(a[0x8d], end, &slot_a, MODE)) return false;
    if (!((Convert)0x71b0f0)(b[0x8d], (uint8_t)(end ^ 1), &slot_b, MODE)) return false;
    return slot_a == slot_b;
}
static const uint8_t w4d_slots_match_original[] = {0x8b, 0x4c, 0x24, 0x04, 0x8a, 0x44, 0x24, 0x10};
REWRITE("Slots::Match71D330", 0x71d330, w4d_slots_match_original, (&w4d_slots_match<1>), &g_w4d_matches);
REWRITE("Slots::Match71D330", 0x71d480, w4d_slots_match_original, (&w4d_slots_match<0xffffffff>), &g_w4d_matches);

// ---- Batches::Add: queues a value in batch INDEX of the table; once the batch holds per * blocks values, its blocks (arrays of SIZE-byte
// objects ended with DTOR) and both lists are freed and the batch starts empty. Index 0 is no batch. ----------------------------------------------
struct W4dBatch {
    int32_t per;
    uint8_t **blocks;
    int32_t blocks_n;
    uint32_t *items;
    int32_t count;
};
static volatile long g_w4d_adds;
template <uint32_t SIZE, uint32_t DTOR> static void __thiscall w4d_batch_add(W4dBatch **self, uint32_t index, uint32_t value) {
    g_w4d_adds++;
    if (!index) return;
    W4dBatch &b = (*self)[index];
    b.items[b.count++] = value;
    if (b.count < b.per * b.blocks_n) return;
    for (int32_t i = 0; i < b.blocks_n; i++) {
        uint8_t *block = b.blocks[i];
        if (!block) continue;
        ((void(__stdcall *)(void *, uint32_t, uint32_t, void *))0xddcf39)(block, SIZE, *(uint32_t *)(block - 4), (void *)DTOR);
        memory_free_aligned(block - 4);
    }
    memory_free_aligned(b.blocks);
    memory_free_aligned(b.items);
    b.blocks = nullptr;
    b.blocks_n = 0;
    b.items = nullptr;
    b.count = 0;
}
static const uint8_t w4d_batch_add_original[] = {0x8b, 0x44, 0x24, 0x04, 0x55, 0x33, 0xed, 0x3b, 0xc5};
REWRITE("Batches::Add8552B0", 0x8552b0, w4d_batch_add_original, (&w4d_batch_add<0x30, 0x8535c0>), &g_w4d_adds);
REWRITE("Batches::Add8552B0", 0x8ff9c0, w4d_batch_add_original, (&w4d_batch_add<0x5c, 0x90a150>), &g_w4d_adds);

// ---- Once::Make2: keeps two values; the shared table is set up once (first pairs (0, 0), (SECOND, 0), then all 0xff) and becomes the vtable --
struct W4dUnusedWords {
    uint32_t words[14];
};
static volatile long g_w4d_once_makes;
template <uint32_t MADE, uint32_t TABLE, uint32_t SECOND> static void *__thiscall w4d_once_make2(uint32_t *self, uint32_t *a, uint32_t *b, W4dUnusedWords) {
    g_w4d_once_makes++;
    self[1] = *a;
    self[2] = *b;
    uint32_t &made = *(uint32_t *)MADE;
    if (!(made & 1)) {
        made |= 1;
        const uint32_t ff = 0xffff;
        ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
                             uint32_t, uint32_t, uint32_t, uint32_t))0x416110)((void *)TABLE, 0, SECOND, ff, ff, ff, ff, ff, ff, ff, ff, ff, ff, ff, ff, ff, ff);
    }
    self[0] = TABLE;
    return self;
}
static const uint8_t w4d_once_make2_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0x54, 0x24, 0x08, 0x83, 0xec, 0x38, 0x56, 0x8b, 0xf1};
REWRITE("Once::Make285B290", 0x85b290, w4d_once_make2_original, (&w4d_once_make2<0x1310dd8, 0x1310db0, 6>), &g_w4d_once_makes);
REWRITE("Once::Make285B290", 0x87c800, w4d_once_make2_original, (&w4d_once_make2<0x1310f70, 0x1310f48, 2>), &g_w4d_once_makes);
REWRITE("Once::Make285B290", 0xc1cc20, w4d_once_make2_original, (&w4d_once_make2<0x13e484c, 0x13e4824, 4>), &g_w4d_once_makes);

// ---- Grid::CellRef: the interface (id 0x13ce3bc) kept in the reference at REF of the cell (0x58 bytes) at (x, y), if inside the bounds --------
static volatile long g_w4d_cell_refs;
template <uint32_t REF> static void *__thiscall w4d_cell_ref(uint8_t *self, int32_t x, int32_t y) {
    g_w4d_cell_refs++;
    int32_t x0 = *(int32_t *)(self + 0x4c), y0 = *(int32_t *)(self + 0x50), x1 = *(int32_t *)(self + 0x54), y1 = *(int32_t *)(self + 0x58);
    if (x < x0 || x > x1 || y < y0 || y > y1) return nullptr;
    uint8_t *cell = *(uint8_t **)(self + 0x5c) + ((x1 - x0 + 1) * (y - y0) + x - x0) * 0x58;
    if (!cell) return nullptr;
    return ref_interface<0x13ce3bc>(cell + REF);
}
static const uint8_t w4d_cell_ref_original[] = {0x8b, 0x41, 0x4c, 0x53, 0x8b, 0x5c, 0x24, 0x08};
REWRITE("Grid::CellRefABD360", 0xabd360, w4d_cell_ref_original, (&w4d_cell_ref<0>), &g_w4d_cell_refs);
REWRITE("Grid::CellRefABD360", 0xabd410, w4d_cell_ref_original, (&w4d_cell_ref<0x1c>), &g_w4d_cell_refs);

// ---- Words::Push: appends a word to the list {data, capacity, count} at LIST, doubling it (FIRST at first) through GROW when full -----------------
static volatile long g_w4d_pushes;
template <uint32_t LIST, uint32_t GROW, int32_t FIRST> static void __thiscall w4d_words_push(uint8_t *self, uint32_t value) {
    g_w4d_pushes++;
    uint32_t *list = (uint32_t *)(self + LIST);
    int32_t count = (int32_t)list[2];
    if (count >= (int32_t)list[1]) ((void(__thiscall *)(void *, int32_t))GROW)(list, count > 0 ? count * 2 : FIRST);
    ((uint32_t *)(uintptr_t)list[0])[list[2]] = value;
    list[2]++;
}
static const uint8_t w4d_words_push_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x86};
REWRITE("Words::Push427790", 0x427790, w4d_words_push_original, (&w4d_words_push<0x678, 0x426d40, 4>), &g_w4d_pushes);
REWRITE("Words::Push427790", 0x80ee30, w4d_words_push_original, (&w4d_words_push<0x8c, 0x80ebb0, 0x20>), &g_w4d_pushes);

// ---- Owned::Destroy: destructor that frees the aligned block at OWNED, passing through its own vtable and ending at the base's -------------------
static volatile long g_w4d_owned_destroys;
template <uint32_t VTABLE, uint32_t OWNED, uint32_t BASE_VTABLE> static void __fastcall w4d_owned_destroy(uint32_t *self) {
    g_w4d_owned_destroys++;
    self[0] = VTABLE;
    memory_free_aligned(*(void **)((uint8_t *)self + OWNED));
    self[0] = BASE_VTABLE;
}
REWRITE("Owned::DestroyC65790", 0xc65790, (w4c_seh<0xea2738>), (&w4d_owned_destroy<0xfb7614, 0xc, 0xfb0350>), &g_w4d_owned_destroys);
REWRITE("Owned::DestroyC65790", 0xca57b0, (w4c_seh<0xea48b8>), (&w4d_owned_destroy<0xfbf5f0, 0x24, 0xfbf5b0>), &g_w4d_owned_destroys);

// ---- Single::Get: the single instance in the global INSTANCE, made the first time (SIZE bytes, built by MAKE, which registers itself) ----------
static volatile long g_w4d_singles;
template <uint32_t INSTANCE, uint32_t SIZE, uint32_t MAKE> static void *__cdecl w4d_single_get() {
    g_w4d_singles++;
    if (!*(void **)INSTANCE) {
        void *made = memory_alloc(SIZE);
        if (made) ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))MAKE)(made, 0, 0, 0);
    }
    return *(void **)INSTANCE;
}
REWRITE("Single::Get436350", 0x436350, (w4c_seh<0xe0828b>), (&w4d_single_get<0x1306c68, 0x58, 0x7a70a0>), &g_w4d_singles);
REWRITE("Single::Get436350", 0x7bbce0, (w4c_seh<0xe5728b>), (&w4d_single_get<0x13070c8, 0x68, 0x7bb7d0>), &g_w4d_singles);

// ---- Holder::Destroy: destructor: the inner part's vtable at INNER is reset, the member at MEMBER is destroyed, then the base destructor ---------
static volatile long g_w4d_holder_destroys;
template <uint32_t INNER, uint32_t MEMBER, uint32_t BASE> static void __fastcall w4d_holder_destroy(uint8_t *self) {
    g_w4d_holder_destroys++;
    *(uint32_t *)(self + INNER) = 0xf2694c;
    ((void(__fastcall *)(void *))0xcd2b50)(self ? self + MEMBER : nullptr);
    ((void(__fastcall *)(void *))BASE)(self);
}
REWRITE("Holder::Destroy42BAE0", 0x42bae0, (w4c_seh<0xe07d98>), (&w4d_holder_destroy<0x18, 0x10, 0x440ed0>), &g_w4d_holder_destroys);
REWRITE("Holder::Destroy42BAE0", 0x7726c0, (w4c_seh<0xe51618>), (&w4d_holder_destroy<0x20, 0x18, 0x772410>), &g_w4d_holder_destroys);

// ---- Thing::New: allocates SIZE bytes and builds them with MAKE(0, a, b, -1); null when out of memory --------------------------------------------
static volatile long g_w4d_news;
template <uint32_t SIZE, uint32_t MAKE> static void *__stdcall w4d_thing_new(uint32_t a, uint32_t b) {
    g_w4d_news++;
    void *made = memory_alloc(SIZE);
    if (!made) return nullptr;
    return ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, int32_t))MAKE)(made, 0, a, b, -1);
}
REWRITE("Thing::New6C6150", 0x6c6150, (w4c_seh<0xe40dab>), (&w4d_thing_new<0x90, 0x6c3f30>), &g_w4d_news);
REWRITE("Thing::New6C6150", 0xbaa270, (w4c_seh<0xe9bafb>), (&w4d_thing_new<0xd0, 0xba9840>), &g_w4d_news);
