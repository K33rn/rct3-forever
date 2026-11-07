// Rewrites of wave 1.
#include "wave.h"

// Calling what the game has at a fixed address.
template <uint32_t AT> static inline void call_plain(void *self) { ((void(__fastcall *)(void *))AT)(self); }
// An owned object deleted through its first virtual function.
static inline void delete_object(void *object) { ((void(__thiscall *)(void *, int32_t))(*(void ***)object)[0])(object, 1); }
// Call a virtual function by its byte offset in the table.
template <uint32_t SLOT> static inline void call_virtual(void *object) {
    ((void(__fastcall *)(void *))*(void **)(*(uint8_t **)object + SLOT))(object);
}
// A node put right after a list's own node.
static inline void link_after(Node *list, Node *n) {
    n->next = list->next;
    list->next->prev = n;
    n->prev = list;
    list->next = n;
}
// The first bytes of a function that starts `push esi; mov esi, ecx; call TARGET`: the call's offset differs by copy.
template <uint32_t AT, uint32_t TARGET> static const uint8_t push_call_original[] = {
    0x56, 0x8b, 0xf1, 0xe8, (uint8_t)(TARGET - (AT + 8)), (uint8_t)((TARGET - (AT + 8)) >> 8),
    (uint8_t)((TARGET - (AT + 8)) >> 16), (uint8_t)((TARGET - (AT + 8)) >> 24)};

// ---- BlockArea::Take: the smallest free block that fits `count` items of `align` bytes, aligned. A block with 128 or
// more bytes left over is split, the rest staying free; `largest` is worked out again if its block was the one taken --
struct AreaBlock {
    int32_t index;      // the start, in items
    void *owner;
    int32_t align;      // nonzero: in use
    int32_t offset;
    int32_t size;
    AreaBlock *prev;
    AreaBlock *next;
};
struct BlockArea {
    char unknown[0x1c];
    int32_t largest;    // +0x1c
    AreaBlock *first;   // +0x20
};
static inline int32_t round_up(int32_t value, int32_t align) {
    int32_t last = value + align - 1;
    return last - last % align;
}
static volatile long g_block_takes;
template <uint32_t ALLOC_BLOCK> static AreaBlock *__thiscall block_area_take(BlockArea *self, int32_t count, int32_t align) {
    g_block_takes++;
    int32_t needed = count * align;
    if (needed > self->largest) return nullptr;
    AreaBlock *best = nullptr;
    int32_t best_size = self->largest + 1;
    for (AreaBlock *b = self->first; b; b = b->next) {
        if (b->align || b->size < needed || b->size >= best_size) continue;
        if (b->size >= round_up(b->offset, align) - b->offset + needed) {
            best = b;
            best_size = b->size;
        }
    }
    if (!best) return nullptr;
    int32_t start = round_up(best->offset, align);
    int32_t left = best->offset - start - needed + best_size;
    if (left >= 0x80) {
        AreaBlock *rest = (AreaBlock *)((void *(__fastcall *)(uint32_t))ALLOC_BLOCK)(sizeof(AreaBlock));
        if (rest) *rest = AreaBlock{};
        rest->owner = self;
        rest->align = 0;
        rest->offset = best->offset - left + best_size;
        rest->size = left;
        rest->prev = best;
        rest->next = best->next;
        if (rest->next) rest->next->prev = rest;
        rest->prev->next = rest;
        best->size -= left;
    }
    best->align = align;
    best->index = start / align;
    if (best_size == self->largest) {
        self->largest = 0;
        for (AreaBlock *b = self->first; b; b = b->next)
            if (!b->align && b->size > self->largest) self->largest = b->size;
    }
    return best;
}
static const uint8_t block_take_original[] = {0x83, 0xec, 0x08, 0x8b, 0x41, 0x1c, 0x57, 0x8b, 0x7c, 0x24, 0x10};
REWRITE("BlockArea::TakeBEA3B0", 0xbea3b0, block_take_original, (&block_area_take<0xbea270>), &g_block_takes);
REWRITE("BlockArea::TakeBEA3B0", 0xbfdfd0, block_take_original, (&block_area_take<0xbfde90>), &g_block_takes);

// ---- Record::Copy: a record of vectors and matrices copied field by field; the padding stays as it was ---------------
static volatile long g_record_copies;
static uint32_t *__thiscall record_copy(uint32_t *self, const uint32_t *other) {
    g_record_copies++;
    for (int i = 0; i < 0x1f; i++)
        if (i != 0xe && i != 0xf && i != 0x13 && i != 0x17) self[i] = other[i];
    ((uint8_t *)self)[0x7c] = ((const uint8_t *)other)[0x7c];
    return self;
}
static const uint32_t record_copy_copies[] = {0x436b40, 0x454930};
static const uint8_t record_copy_original[] = {0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x04, 0x8b, 0x11, 0x89, 0x10, 0x8b, 0x51, 0x04, 0x89, 0x50, 0x04, 0x56};
REWRITE_COPIES("Record::Copy436B40", record_copy_copies, record_copy_original, record_copy, &g_record_copies);

// ---- Block76::Copy: 76 plain bytes ------------------------------------------------------------------------------------
static volatile long g_plain_copies;
static uint32_t *__thiscall plain_copy(uint32_t *self, const uint32_t *other) {
    g_plain_copies++;
    for (int i = 0; i < 0x13; i++) self[i] = other[i];
    return self;
}
static const uint32_t plain_copy_copies[] = {0x84dbf0, 0x84dc70};
static const uint8_t plain_copy_original[] = {0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x04, 0x8b, 0x11, 0x89, 0x10, 0x8b, 0x51, 0x04, 0x89, 0x50, 0x04, 0x8b, 0x51, 0x08};
REWRITE_COPIES("Block76::Copy84DBF0", plain_copy_copies, plain_copy_original, plain_copy, &g_plain_copies);

// ---- Transform::SetRows: three rows of three and one more value, kept at +0x70 ---------------------------------------
static volatile long g_row_sets;
static void __thiscall rows_set(uint8_t *self, const uint32_t *rows) {
    g_row_sets++;
    uint32_t *to = (uint32_t *)(self + 0x70);
    for (int i = 0; i <= 12; i++)
        if (i % 4 != 3) to[i] = rows[i];
}
static const uint32_t rows_set_copies[] = {0x988500, 0x9885e0};
static const uint8_t rows_set_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0x10, 0x89, 0x51, 0x70};
REWRITE_COPIES("Transform::SetRows988500", rows_set_copies, rows_set_original, rows_set, &g_row_sets);

// ---- VirtualBase::Assign: an object with virtual bases assigned. Each base's slot is copied only while the object's
// table is that base's own, then the three plain fields -------------------------------------------------------------
static inline void assign_base(uint8_t *self, uint8_t *other, const int32_t *table) {
    if (*(const int32_t **)self != table) return;
    uint8_t *from = other ? other + (*(int32_t **)other)[1] : nullptr;
    *(uint32_t *)(self + table[1]) = *(uint32_t *)from;
}
static volatile long g_base_assigns;
template <uint32_t TABLE1, uint32_t TABLE2> static uint8_t *__thiscall virtual_assign(uint8_t *self, uint8_t *other) {
    g_base_assigns++;
    assign_base(self, other, (const int32_t *)TABLE1);
    assign_base(self, other, (const int32_t *)TABLE2);
    for (int i = 1; i < 4; i++) ((uint32_t *)self)[i] = ((uint32_t *)other)[i];
    return self;
}
static const uint8_t virtual_assign_original[] = {0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x04, 0x56, 0x8b, 0x30, 0x81, 0xfe};
REWRITE("VirtualBase::Assign5D3D40", 0x5d3d40, virtual_assign_original, (&virtual_assign<0xf3f70c, 0xf3af70>), &g_base_assigns);
REWRITE("VirtualBase::Assign5D3D40", 0x5d3e60, virtual_assign_original, (&virtual_assign<0xf3f714, 0xf3af70>), &g_base_assigns);
REWRITE("VirtualBase::Assign5D3D40", 0x5d3ec0, virtual_assign_original, (&virtual_assign<0xf3f71c, 0xf3af68>), &g_base_assigns);

// ---- TaggedValue::Set: a value with its kind in the low byte of a word ------------------------------------------------
static volatile long g_tagged_sets;
template <uint8_t KIND> static uint8_t *__thiscall tagged_set(uint8_t *self, uint32_t value) {
    g_tagged_sets++;
    *(uint16_t *)self = KIND;
    *(uint32_t *)(self + 4) = value;
    return self;
}
static const uint8_t tagged_set_original[] = {0x51, 0x8b, 0x54, 0x24, 0x08, 0x8b, 0xc1, 0xc6, 0x44, 0x24, 0x02};
REWRITE("TaggedValue::Set87B650", 0x87b650, tagged_set_original, (&tagged_set<0x1d>), &g_tagged_sets);
REWRITE("TaggedValue::Set87B650", 0xbdd440, tagged_set_original, (&tagged_set<0x08>), &g_tagged_sets);
REWRITE("TaggedValue::Set87B650", 0xc0a270, tagged_set_original, (&tagged_set<0x21>), &g_tagged_sets);
REWRITE("TaggedValue::Set87B650", 0xc0a290, tagged_set_original, (&tagged_set<0x1f>), &g_tagged_sets);
REWRITE("TaggedValue::Set87B650", 0xc0a2b0, tagged_set_original, (&tagged_set<0x24>), &g_tagged_sets);
REWRITE("TaggedValue::Set87B650", 0xc0a310, tagged_set_original, (&tagged_set<0x25>), &g_tagged_sets);

// ---- Cell::Clear: one pointer set to null (a debug-built constructor) -------------------------------------------------
static volatile long g_cell_clears;
static uint32_t *__fastcall cell_clear(uint32_t *self) {
    g_cell_clears++;
    *self = 0;
    return self;
}
static const uint32_t cell_clear_copies[] = {0xcc2cc0, 0xcc4640};
static const uint8_t cell_clear_original[] = {0x55, 0x8b, 0xec, 0x51, 0xc7, 0x45, 0xfc, 0xcc, 0xcc, 0xcc, 0xcc};
REWRITE_COPIES("Cell::ClearCC2CC0", cell_clear_copies, cell_clear_original, cell_clear, &g_cell_clears);

// ---- Listed::Construct: set the table, point +0xc at itself, and join the list kept in a global ----------------------
static volatile long g_listed_constructs;
template <uint32_t TABLE, uint32_t LIST> static uint32_t *__fastcall listed_construct(uint32_t *self) {
    g_listed_constructs++;
    self[0] = TABLE;
    self[3] = (uint32_t)(uintptr_t)self;
    link_after(*(Node **)LIST, (Node *)(self + 1));
    return self;
}
template <uint32_t TABLE> static const uint8_t listed_construct_original[] = {
    0x8b, 0xc1, 0xc7, 0x00, (uint8_t)TABLE, (uint8_t)(TABLE >> 8), (uint8_t)(TABLE >> 16), (uint8_t)(TABLE >> 24)};
REWRITE("Listed::Construct4535C0", 0x4535c0, listed_construct_original<0xf296f8>, (&listed_construct<0xf296f8, 0x10ec7e4>), &g_listed_constructs);
REWRITE("Listed::Construct4535C0", 0x8e9480, listed_construct_original<0xf7ccbc>, (&listed_construct<0xf7ccbc, 0x10f0b18>), &g_listed_constructs);
REWRITE("Listed::Construct4535C0", 0x946f50, listed_construct_original<0xf7e29c>, (&listed_construct<0xf7e29c, 0x10f0c7c>), &g_listed_constructs);

// ---- Shared::Current: a new reference to the current entry of a table of counted items -------------------------------
static volatile long g_current_refs;
template <uint32_t TABLE> static uint32_t **__thiscall current_reference(uint8_t *self, uint32_t **out) {
    g_current_refs++;
    uint32_t *item = (*(uint32_t ***)(self + TABLE))[*(int32_t *)(self + 0x278)];
    *out = item;
    ++*item;
    return out;
}
static const uint8_t current_ref_original[] = {0x51, 0x8b, 0x81, 0x78, 0x02, 0x00, 0x00};
REWRITE("Shared::Current685B70", 0x685b70, current_ref_original, (&current_reference<0x578>), &g_current_refs);
REWRITE("Shared::Current685B70", 0x685ba0, current_ref_original, (&current_reference<0x580>), &g_current_refs);

// ---- Pair::Construct: a table and two values ---------------------------------------------------------------------------
static volatile long g_pair_constructs;
template <uint32_t TABLE> static uint32_t *__thiscall pair_construct(uint32_t *self, uint32_t a, uint32_t b) {
    g_pair_constructs++;
    self[0] = TABLE;
    self[1] = a;
    self[2] = b;
    return self;
}
static const uint8_t pair_construct_original[] = {0x8b, 0x54, 0x24, 0x08, 0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x04};
REWRITE("Pair::ConstructA5ECE0", 0xa5ece0, pair_construct_original, (&pair_construct<0xf8f6d8>), &g_pair_constructs);
REWRITE("Pair::ConstructA5ECE0", 0xbb2730, pair_construct_original, (&pair_construct<0xf9ebc4>), &g_pair_constructs);

// ---- Kind::IsOneOf: whether a value is one of three ---------------------------------------------------------------------
static volatile long g_kind_checks;
template <int32_t A, int32_t B, int32_t C> static uint32_t __fastcall is_one_of(int32_t kind) {
    g_kind_checks++;
    return kind == A || kind == B || kind == C;
}
template <uint8_t A, uint8_t B> static const uint8_t is_one_of_original[] = {0x83, 0xf9, A, 0x74, 0x0d, 0x83, 0xf9, B};
REWRITE("Kind::IsOneOf5BCFB0", 0x5bcfb0, (is_one_of_original<2, 4>), (&is_one_of<2, 4, 6>), &g_kind_checks);
REWRITE("Kind::IsOneOf5BCFB0", 0xbc4200, (is_one_of_original<1, 2>), (&is_one_of<1, 2, 3>), &g_kind_checks);
REWRITE("Kind::IsOneOf5BCFB0", 0xbc4220, (is_one_of_original<4, 5>), (&is_one_of<4, 5, 6>), &g_kind_checks);

// ---- Screen::Destroy: back to this class's tables, the global that points at the screen cleared, then the base's
// destructor ---------------------------------------------------------------------------------------------------------
static volatile long g_screen_destroys;
template <uint32_t T0, uint32_t T1, uint32_t T2, uint32_t CURRENT> static void __fastcall screen_destroy(uint8_t *self) {
    g_screen_destroys++;
    *(uint32_t *)self = T0;
    *(uint32_t *)(self + 4) = T1;
    *(uint32_t *)(self + 0xd8) = T2;
    *(int32_t *)CURRENT = -1;
    call_plain<0x669660>(self);
}
template <uint32_t T0> static const uint8_t screen_destroy_original[] = {
    0xc7, 0x01, (uint8_t)T0, (uint8_t)(T0 >> 8), (uint8_t)(T0 >> 16), (uint8_t)(T0 >> 24)};
REWRITE("Screen::Destroy631FE0", 0x631fe0, screen_destroy_original<0xf477a8>, (&screen_destroy<0xf477a8, 0xf47794, 0xf47790, 0x10eeab0>), &g_screen_destroys);
REWRITE("Screen::Destroy631FE0", 0x6463a0, screen_destroy_original<0xf4ab18>, (&screen_destroy<0xf4ab18, 0xf4ab00, 0xf4aafc, 0x10eeb74>), &g_screen_destroys);
REWRITE("Screen::Destroy631FE0", 0x78b600, screen_destroy_original<0xf6e100>, (&screen_destroy<0xf6e100, 0xf6e0e8, 0xf6e0e4, 0x10ef98c>), &g_screen_destroys);

// ---- Placed::SetPosition: three values kept in the attached object's data if there is one, otherwise here -------------
static volatile long g_position_sets;
template <uint32_t ATTACHED, uint32_t OWN> static void __thiscall position_set(uint8_t *self, const uint32_t *value) {
    g_position_sets++;
    uint8_t *attached = *(uint8_t **)(self + 4);
    uint32_t *to = attached ? (uint32_t *)(*(uint8_t **)(attached + 0xc) + ATTACHED) : (uint32_t *)(self + OWN);
    for (int i = 0; i < 3; i++) to[i] = value[i];
}
static const uint8_t position_set_original[] = {0x8b, 0x41, 0x04, 0x85, 0xc0, 0x74, 0x1d};
REWRITE("Placed::SetPosition579B90", 0x579b90, position_set_original, (&position_set<0x60, 0x20>), &g_position_sets);
REWRITE("Placed::SetPosition579B90", 0x579bd0, position_set_original, (&position_set<0x70, 0x30>), &g_position_sets);

// ---- State::Enter: a state number and flag, with the time from a global ------------------------------------------------
static volatile long g_state_enters;
template <uint32_t STATE, uint32_t TIME, uint32_t FLAG> static void __fastcall state_enter(uint8_t *self) {
    g_state_enters++;
    *(uint32_t *)(self + 0x190) = STATE;
    *(uint32_t *)(self + 0xb8) = *(uint32_t *)TIME;
    *(uint32_t *)(self + 0x194) = FLAG;
    *(uint32_t *)(self + 0xbc) = 0;
}
static const uint8_t state_enter_original[] = {0xc7, 0x81, 0x90, 0x01, 0x00, 0x00};
REWRITE("State::Enter79DF20", 0x79df20, state_enter_original, (&state_enter<0xe, 0x10efa28, 1>), &g_state_enters);
REWRITE("State::Enter79DF20", 0x79df50, state_enter_original, (&state_enter<0x13, 0x10efa38, 2>), &g_state_enters);

// ---- Updated::SetActive: joining the manager's list when turned on, and asking it to drop this when turned off ----------
static volatile long g_active_sets;
template <uint32_t FLAG, uint32_t LIST, uint32_t REMOVE> static void __thiscall active_set(uint8_t *self, uint32_t on) {
    g_active_sets++;
    uint8_t *manager = *(uint8_t **)0x13d0d84;
    if ((uint8_t)on) {
        if (self[FLAG]) return;
        self[FLAG] = 1;
        link_after(*(Node **)(manager + LIST), (Node *)(self + 4));
    } else if (self[FLAG]) {
        self[FLAG] = 0;
        ((void(__thiscall *)(void *, void *))REMOVE)(manager, self);
    }
}
static const uint8_t active_set_original[] = {0x8a, 0x44, 0x24, 0x04, 0x84, 0xc0, 0x8a, 0x81};
REWRITE("Updated::SetActiveBD31F0", 0xbd31f0, active_set_original, (&active_set<0x98, 0x44, 0xbd2960>), &g_active_sets);
REWRITE("Updated::SetActiveBD31F0", 0xbd3240, active_set_original, (&active_set<0x118, 0x5c, 0xbd29e0>), &g_active_sets);

// ---- Names: the text for a small number, or null ---------------------------------------------------------------------------
template <uint32_t... TEXT> static uint32_t name_of(uint32_t index) {
    static const uint32_t texts[] = {TEXT...};
    return index < sizeof...(TEXT) ? texts[index] : 0;
}
static volatile long g_names8, g_names11;
template <uint32_t... TEXT> static uint32_t __fastcall name8(uint32_t index) { g_names8++; return name_of<TEXT...>(index); }
template <uint32_t... TEXT> static uint32_t __fastcall name11(uint32_t index) { g_names11++; return name_of<TEXT...>(index); }
static const uint8_t name8_original[] = {0x83, 0xf9, 0x07, 0x77, 0x37, 0xff, 0x24, 0x8d};
static const uint8_t name11_original[] = {0x83, 0xf9, 0x0a, 0x77, 0x49, 0xff, 0x24, 0x8d};
REWRITE("Names::Of8BC4150", 0xbc4150, name8_original, (&name8<0xf79228, 0xfa0130, 0xfa0124, 0xfa0118, 0xfa010c, 0xfa0100, 0xfa00f4, 0xf88254>), &g_names8);
REWRITE("Names::Of8BC4150", 0xbc4af0, name8_original, (&name8<0xf7b9e4, 0xfa0c1c, 0xfa0c10, 0xfa0c08, 0xfa0bfc, 0xfa0bf0, 0xfa0be0, 0xfa0bd0>), &g_names8);
REWRITE("Names::Of8BC4150", 0xbc4ba0, name8_original, (&name8<0xf7b9e4, 0xfa0c54, 0xfa0c4c, 0xfa0c40, 0xf3e3ac, 0xfa0c38, 0xfa0c30, 0xfa0c28>), &g_names8);
REWRITE("Names::Of1159A080", 0x59a080, name11_original, (&name11<0xf3d9c4, 0xf3d9b0, 0xf3d99c, 0xf3d980, 0xf3d968, 0xf3d950, 0xf3d93c, 0xf3d928, 0xf3d910, 0xf3d8f4, 0xf3d8cc>), &g_names11);
REWRITE("Names::Of1159A080", 0xbc4da0, name11_original, (&name11<0xfa0de4, 0xfa0dd8, 0xfa0dd0, 0xfa0dc4, 0xfa0db4, 0xfa0da4, 0xfa0d90, 0xfa0d78, 0xfa0d64, 0xfa0d50, 0xfa0d38>), &g_names11);

// ---- Walking a list that ends at a node with no next -------------------------------------------------------------------------
// ValueList::Clear: frees each value (the nodes belong to a pool), and resets to the list's own end node at +0xc.
static volatile long g_value_clears;
static void __fastcall value_list_clear(uint8_t *self) {
    g_value_clears++;
    Node *n = *(Node **)self;
    while (n->next) {
        Node *next = n->next;
        if (n->value) memory_free(n->value);
        n = next;
    }
    *(uint32_t *)(self + 4) = 0;
    *(uint8_t **)self = self + 0xc;
    *(uint8_t **)(self + 0x10) = self;
    *(uint32_t *)(self + 0xc) = 0;
}
static const uint32_t value_clear_copies[] = {0xb781b0, 0xbbcbc0};
static const uint8_t value_clear_original[] = {0x57, 0x8b, 0xf9, 0x8b, 0x07, 0x83, 0x38, 0x00};
REWRITE_COPIES("ValueList::ClearB781B0", value_clear_copies, value_clear_original, value_list_clear, &g_value_clears);

// GlobalList::Each: a virtual function called on everything in a list kept in a global.
static volatile long g_global_eaches;
template <uint32_t LIST, uint32_t SLOT> static void __cdecl global_each(void) {
    g_global_eaches++;
    for (Node *n = *(Node **)LIST; n->next;) {
        Node *next = n->next;
        call_virtual<SLOT>(n->value);
        n = next;
    }
}
template <uint32_t LIST> static const uint8_t global_each_original[] = {
    0xa1, (uint8_t)LIST, (uint8_t)(LIST >> 8), (uint8_t)(LIST >> 16), (uint8_t)(LIST >> 24), 0x83, 0x38, 0x00};
REWRITE("GlobalList::Each452E40", 0x452e40, global_each_original<0x10ec7d4>, (&global_each<0x10ec7d4, 4>), &g_global_eaches);
REWRITE("GlobalList::Each452E40", 0x9420e0, global_each_original<0x10f0c6c>, (&global_each<0x10f0c6c, 4>), &g_global_eaches);
REWRITE("GlobalList::Each452E40", 0x942170, global_each_original<0x10f0c6c>, (&global_each<0x10f0c6c, 0xc>), &g_global_eaches);

// MemberList::Each: the same on a list this object holds; the next node is read after the call.
static volatile long g_member_eaches;
template <uint32_t LIST, uint32_t SLOT> static void __fastcall member_each(uint8_t *self) {
    g_member_eaches++;
    for (Node *n = *(Node **)(self + LIST); n->next; n = n->next) call_virtual<SLOT>(n->value);
}
template <uint8_t LIST> static const uint8_t member_each_original[] = {0x56, 0x8b, 0x71, LIST, 0x83, 0x3e, 0x00};
REWRITE("MemberList::Each440E40", 0x440e40, member_each_original<0x24>, (&member_each<0x24, 4>), &g_member_eaches);
REWRITE("MemberList::Each440E40", 0xbfc780, member_each_original<0x04>, (&member_each<0x04, 0x58>), &g_member_eaches);
REWRITE("MemberList::Each440E40", 0xbfc7b0, member_each_original<0x04>, (&member_each<0x04, 0x5c>), &g_member_eaches);

// OwnedList::DeleteAll: deletes the first value until none is left (each one takes itself off the list).
static volatile long g_owned_clears;
static void __fastcall owned_list_clear(uint8_t *self) {
    g_owned_clears++;
    for (;;) {
        Node *n = *(Node **)(self + 4);
        if (!n->next) return;
        if (n->value) delete_object(n->value);
    }
}
static const uint32_t owned_clear_copies[] = {0xbf53e0, 0xbfc750};
static const uint8_t owned_clear_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x04, 0x83, 0x38, 0x00};
REWRITE_COPIES("OwnedList::ClearBF53E0", owned_clear_copies, owned_clear_original, owned_list_clear, &g_owned_clears);

// OwnedList::DeleteUntilEmpty: the same at another place, stopping at an empty value.
static volatile long g_owned_stops;
template <uint32_t LIST> static void __fastcall owned_list_delete(uint8_t *self) {
    g_owned_stops++;
    for (;;) {
        Node *n = *(Node **)(self + LIST);
        void *value = n->next ? n->value : nullptr;
        if (!value) return;
        delete_object(value);
    }
}
static const uint8_t owned_delete_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46};
REWRITE("OwnedList::Delete4CCDB0", 0x4ccdb0, owned_delete_original, (&owned_list_delete<0x64>), &g_owned_stops);
REWRITE("OwnedList::Delete4CCDB0", 0xb91210, owned_delete_original, (&owned_list_delete<0x1c>), &g_owned_stops);

// ---- Level::Setting: a value kept here once the level's data says the second stage is reached, otherwise a default -------
static volatile long g_level_settings;
template <uint32_t VALUE, int32_t DEFAULT> static int32_t __fastcall level_setting(uint8_t *self) {
    g_level_settings++;
    uint8_t *level = ((uint8_t * (__fastcall *)(void *))0xbb3e70)(self);
    return level[1] > 1 ? *(int32_t *)(self + VALUE) : DEFAULT;
}
REWRITE("Level::Setting4D6650", 0x4d6650, (push_call_original<0x4d6650, 0xbb3e70>), (&level_setting<0x28, 3>), &g_level_settings);
REWRITE("Level::Setting4D6650", 0x4d6680, (push_call_original<0x4d6680, 0xbb3e70>), (&level_setting<0x2c, 3>), &g_level_settings);
REWRITE("Level::Setting4D6650", 0x52fed0, (push_call_original<0x52fed0, 0xbb3e70>), (&level_setting<0x30, -2>), &g_level_settings);
REWRITE("Level::Setting4D6650", 0x52ff00, (push_call_original<0x52ff00, 0xbb3e70>), (&level_setting<0x34, -2>), &g_level_settings);
REWRITE("Level::Setting4D6650", 0x52ff30, (push_call_original<0x52ff30, 0xbb3e70>), (&level_setting<0x38, -2>), &g_level_settings);

// ---- List::PushBack: a new node holding a value at the end of the ring this object points to ---------------------------
static volatile long g_push_backs;
template <uint32_t LIST> static void __thiscall list_push_back(uint8_t *self, void *value) {
    g_push_backs++;
    Node *n = (Node *)memory_alloc(sizeof(Node));
    if (n) n->value = value;
    Node *ring = *(Node **)(self + LIST);
    n->prev = ring->prev;
    ring->prev->next = n;
    n->next = ring;
    ring->prev = n;
}
static const uint8_t push_back_original[] = {0x56, 0x6a, 0x0c, 0x8b, 0xf1, 0xe8};
REWRITE("List::PushBack9F3DC0", 0x9f3dc0, push_back_original, (&list_push_back<0x60>), &g_push_backs);
REWRITE("List::PushBack9F3DC0", 0xa8c1d0, push_back_original, (&list_push_back<0x50>), &g_push_backs);
REWRITE("List::PushBack9F3DC0", 0xa8c210, push_back_original, (&list_push_back<0x68>), &g_push_backs);

// ---- Device::SetCached: a device setting passed on only when it changes -----------------------------------------------------
static volatile long g_cached_sets;
template <uint32_t VALUE, uint32_t SLOT> static void __thiscall cached_set(uint8_t *self, uint32_t value) {
    g_cached_sets++;
    if (*(uint32_t *)(self + VALUE) == value) return;
    void *device = *(void **)(self + 0x24c);
    ((void(__stdcall *)(void *, uint32_t))*(void **)(*(uint8_t **)device + SLOT))(device, value);
    *(uint32_t *)(self + VALUE) = value;
}
static const uint8_t cached_set_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x86};
REWRITE("Device::SetCachedBE8960", 0xbe8960, cached_set_original, (&cached_set<0x3c8, 0x15c>), &g_cached_sets);
REWRITE("Device::SetCachedBE8960", 0xbe8a50, cached_set_original, (&cached_set<0x400, 0x1a0>), &g_cached_sets);
REWRITE("Device::SetCachedBE8960", 0xc738f0, cached_set_original, (&cached_set<0x2490, 0x170>), &g_cached_sets);
REWRITE("Device::SetCachedBE8960", 0xc73920, cached_set_original, (&cached_set<0x2494, 0x1ac>), &g_cached_sets);

// ---- Arrays::Free: three aligned arrays freed --------------------------------------------------------------------------------
static volatile long g_array_frees;
template <uint32_t A, uint32_t B, uint32_t C> static void __fastcall arrays_free(uint8_t *self) {
    g_array_frees++;
    memory_free_aligned(*(void **)(self + A));
    memory_free_aligned(*(void **)(self + B));
    memory_free_aligned(*(void **)(self + C));
}
static const uint8_t arrays_free_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46};
REWRITE("Arrays::Free8E7200", 0x8e7200, arrays_free_original, (&arrays_free<0x10, 0x1c, 0x20>), &g_array_frees);
REWRITE("Arrays::Free8E7200", 0x8eee50, arrays_free_original, (&arrays_free<0x10, 0x28, 0x2c>), &g_array_frees);
REWRITE("Arrays::Free8E7200", 0xbcc1c0, arrays_free_original, (&arrays_free<0x04, 0x14, 0x18>), &g_array_frees);
REWRITE("Arrays::Free8E7200", 0xbd6510, arrays_free_original, (&arrays_free<0x0c, 0x10, 0x14>), &g_array_frees);
REWRITE("Arrays::Free8E7200", 0xc47350, arrays_free_original, (&arrays_free<0x0c, 0x10, 0x14>), &g_array_frees);
REWRITE("Arrays::Free8E7200", 0xc47630, arrays_free_original, (&arrays_free<0x0c, 0x10, 0x14>), &g_array_frees);

// ---- Handle::Forward: a debug-built wrapper calling a function on the object held ------------------------------------------
static volatile long g_forwards;
template <uint32_t TARGET> static uint32_t __fastcall forward_held(void **self) {
    g_forwards++;
    return ((uint32_t(__fastcall *)(void *))TARGET)(*self);
}
static const uint8_t forward_held_original[] = {0x55, 0x8b, 0xec, 0x51, 0xc7, 0x45, 0xfc, 0xcc, 0xcc, 0xcc, 0xcc, 0x89, 0x4d, 0xfc};
REWRITE("Handle::ForwardCC2090", 0xcc2090, forward_held_original, (&forward_held<0xcc2250>), &g_forwards);
REWRITE("Handle::ForwardCC2090", 0xcc4e70, forward_held_original, (&forward_held<0xcc2250>), &g_forwards);
REWRITE("Handle::ForwardCC2090", 0xcc57d0, forward_held_original, (&forward_held<0xcc2250>), &g_forwards);
REWRITE("Handle::ForwardCC2090", 0xcc5800, forward_held_original, (&forward_held<0xcc4280>), &g_forwards);

// ---- Text::FromRange: the two fields of a text passed on to the function that makes the result --------------------------------
static volatile long g_from_ranges;
template <uint32_t MAKE> static void *__thiscall from_range(void *self, void *out, uint8_t **source) {
    g_from_ranges++;
    uint8_t *text = *source;
    ((void(__thiscall *)(void *, void *, uint32_t, uint32_t))MAKE)(self, out, *(uint32_t *)(text + 0xc), *(uint32_t *)(text + 4));
    return out;
}
static const uint8_t from_range_original[] = {0x51, 0x8b, 0x44, 0x24, 0x0c, 0x8b, 0x00, 0x8b, 0x50, 0x04};
REWRITE("Text::FromRange403BB0", 0x403bb0, from_range_original, (&from_range<0xbd0640>), &g_from_ranges);
REWRITE("Text::FromRange403BB0", 0x7fbff0, from_range_original, (&from_range<0xbd0ce0>), &g_from_ranges);
REWRITE("Text::FromRange403BB0", 0xbd12c0, from_range_original, (&from_range<0xbd0720>), &g_from_ranges);

// ---- Value::Query: a kept value and two globals passed on, with 1.0 as the weight; the helper's own object is a flag on the
// stack --------------------------------------------------------------------------------------------------------------------------
static volatile long g_value_queries;
template <uint32_t VALUE, uint32_t GLOBAL_A, uint32_t GLOBAL_B> static uint32_t __fastcall value_query(uint8_t *self) {
    g_value_queries++;
    uint8_t frame[8];
    uint32_t value = *(uint32_t *)(self + VALUE);
    *(uint32_t *)(frame + 4) = value;
    return ((uint32_t(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))0xbc4ff0)(frame + 3, value, *(uint32_t *)GLOBAL_B, *(uint32_t *)GLOBAL_A, 0x3f800000);
}
static const uint8_t value_query_original[] = {0x83, 0xec, 0x08, 0x8b, 0x41};
REWRITE("Value::QueryBC5420", 0xbc5420, value_query_original, (&value_query<0x10, 0x13d0a80, 0x10f2b30>), &g_value_queries);
REWRITE("Value::QueryBC5420", 0xbc5450, value_query_original, (&value_query<0x18, 0x13d0a88, 0x10f2b38>), &g_value_queries);
REWRITE("Value::QueryBC5420", 0xbc5480, value_query_original, (&value_query<0x14, 0x13d0a84, 0x10f2b34>), &g_value_queries);

// ---- Reference::Construct: the null handle, and joining its list of references --------------------------------------------------
static volatile long g_ref_constructs;
static uint8_t *__fastcall reference_construct(uint8_t *self) {
    g_ref_constructs++;
    *(uint32_t *)self = 0xf26ae0;
    *(uint8_t **)(self + 0x10) = self;
    uint8_t *handle = ((uint8_t * (__cdecl *)())0x805750)();
    *(uint8_t **)(self + 4) = handle;
    link_after(*(Node **)(handle + 0x20), (Node *)(self + 8));
    *(uint32_t *)(self + 0x14) = 0;
    *(uint32_t *)(self + 0x18) = 0;
    *(uint32_t *)(self + 0x20) = 0;
    return self;
}
static const uint32_t ref_construct_copies[] = {0x82a390, 0xb490c0};
static const uint8_t ref_construct_original[] = {0x56, 0x8b, 0xf1, 0xc7, 0x06, 0xe0, 0x6a, 0xf2, 0x00};
REWRITE_COPIES("Reference::Construct82A390", ref_construct_copies, ref_construct_original, reference_construct, &g_ref_constructs);

// ---- Array::Delete: a counted array: items with destructors go through the first's delete function; otherwise the block goes --
static volatile long g_array_deletes;
static void __fastcall counted_array_delete(uint8_t **self) {
    g_array_deletes++;
    uint8_t *items = *self;
    if (!items) return;
    uint32_t *count = (uint32_t *)(items - 4);
    if (*count) ((void(__thiscall *)(void *, int32_t))(*(void ***)items)[1])(items, 3);
    else memory_free_aligned(count);
    *self = nullptr;
}
static const uint32_t array_delete_copies[] = {0x8f8910, 0x940610};
static const uint8_t array_delete_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x0e, 0x85, 0xc9};
REWRITE_COPIES("Array::Delete8F8910", array_delete_copies, array_delete_original, counted_array_delete, &g_array_deletes);

// ---- Pair::Delete: two owned objects, the second first ---------------------------------------------------------------------------
static volatile long g_pair_deletes;
static void __fastcall pair_delete(void **self) {
    g_pair_deletes++;
    if (self[1]) delete_object(self[1]);
    if (self[0]) delete_object(self[0]);
}
static const uint32_t pair_delete_copies[] = {0x86e5c0, 0xa1f7f0};
static const uint8_t pair_delete_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x4e, 0x04, 0x85, 0xc9};
REWRITE_COPIES("Pair::Delete86E5C0", pair_delete_copies, pair_delete_original, pair_delete, &g_pair_deletes);

// ---- Scene::Close: the owned view deleted, then the rest of closing; the argument isn't used --------------------------------------
static volatile long g_scene_closes;
static void __thiscall scene_close(uint8_t *self, uint32_t) {
    g_scene_closes++;
    void *view = *(void **)(self + 0x20);
    if (!view) return;
    *(void **)(self + 0x20) = nullptr;
    delete_object(view);
    call_plain<0x6a1e20>(self);
}
static const uint32_t scene_close_copies[] = {0x63d280, 0x65bd00};
static const uint8_t scene_close_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x4e, 0x20, 0x85, 0xc9};
REWRITE_COPIES("Scene::Close63D280", scene_close_copies, scene_close_original, scene_close, &g_scene_closes);

// ---- Pointers::Construct: {data, count} of `count` pointers ------------------------------------------------------------------------
static volatile long g_pointer_constructs;
static uint32_t *__thiscall pointers_construct(uint32_t *self, uint32_t count) {
    g_pointer_constructs++;
    self[1] = count;
    self[0] = (uint32_t)(uintptr_t)memory_alloc_aligned(count * 4);
    return self;
}
static const uint32_t pointers_construct_copies[] = {0x50cb50, 0x5a24d0};
static const uint8_t pointers_construct_original[] = {0x8b, 0x44, 0x24, 0x04, 0x56, 0x8b, 0xf1};
REWRITE_COPIES("Pointers::Construct50CB50", pointers_construct_copies, pointers_construct_original, pointers_construct, &g_pointer_constructs);

// ---- Found::SetTime: a value given to the object looked up, if there is one --------------------------------------------------------
static volatile long g_found_sets;
static void __thiscall found_set(void *self, uint32_t value) {
    g_found_sets++;
    uint8_t *found = ((uint8_t * (__fastcall *)(void *))0x4d1840)(self);
    if (!found) return;
    *(uint32_t *)(found + 0xb8) = value;
    *(uint32_t *)(found + 0xbc) = 0;
}
template <uint32_t AT> static const uint8_t found_set_original[] = {
    0xe8, (uint8_t)(0x4d1840 - (AT + 5)), (uint8_t)((0x4d1840 - (AT + 5)) >> 8), (uint8_t)((0x4d1840 - (AT + 5)) >> 16),
    (uint8_t)((0x4d1840 - (AT + 5)) >> 24), 0x85, 0xc0, 0x74, 0x14};
REWRITE("Found::Set4E5F00", 0x4e5f00, found_set_original<0x4e5f00>, found_set, &g_found_sets);
REWRITE("Found::Set4E5F00", 0x5eb160, found_set_original<0x5eb160>, found_set, &g_found_sets);

// ---- Shown::Reset: the base's reset, and three flags ----------------------------------------------------------------------------
static volatile long g_shown_resets;
static void __fastcall shown_reset(uint8_t *self) {
    g_shown_resets++;
    call_plain<0xb5f790>(self);
    self[0x51] = 1;
    self[0xbf] = 0;
    self[0xc0] = 1;
}
REWRITE("Shown::ResetB19D40", 0xb19d40, (push_call_original<0xb19d40, 0xb5f790>), shown_reset, &g_shown_resets);
REWRITE("Shown::ResetB19D40", 0xb26f40, (push_call_original<0xb26f40, 0xb5f790>), shown_reset, &g_shown_resets);

// ---- Sized::Construct: the shared constructor, given this class's size -----------------------------------------------------------
static volatile long g_sized_constructs;
template <uint32_t SIZE> static void *__fastcall sized_construct(void *self) {
    g_sized_constructs++;
    ((void(__fastcall *)(void *, uint32_t))0x458010)(self, SIZE);
    return self;
}
template <uint8_t SIZE> static const uint8_t sized_construct_original[] = {0x51, 0x56, 0xba, SIZE, 0x00, 0x00, 0x00};
REWRITE("Sized::Construct458130", 0x458130, sized_construct_original<0x05>, (&sized_construct<0x05>), &g_sized_constructs);
REWRITE("Sized::Construct458130", 0x458820, sized_construct_original<0x1a>, (&sized_construct<0x1a>), &g_sized_constructs);

// ---- Com::Release: a held COM object released and forgotten ------------------------------------------------------------------------
static volatile long g_com_releases;
template <uint32_t HELD> static void __fastcall com_release(uint8_t *self) {
    g_com_releases++;
    void *object = *(void **)(self + HELD);
    if (!object) return;
    ((void(__stdcall *)(void *))(*(void ***)object)[2])(object);
    *(void **)(self + HELD) = nullptr;
}
static const uint8_t com_release_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46};
REWRITE("Com::ReleaseC5BE10", 0xc5be10, com_release_original, (&com_release<0x28>), &g_com_releases);
REWRITE("Com::ReleaseC5BE10", 0xc5bea0, com_release_original, (&com_release<0x28>), &g_com_releases);
REWRITE("Com::ReleaseC5BE10", 0xc5c040, com_release_original, (&com_release<0x24>), &g_com_releases);

// ---- Part::Update: the base's update, then the context told about this object and its cached interface -----------------------
static volatile long g_part_updates;
template <uint32_t REF, uint32_t BASE> static void __thiscall part_update(uint8_t *self, void *context) {
    g_part_updates++;
    ((void(__thiscall *)(void *, void *))BASE)(self, context);
    uint8_t *found = (uint8_t *)ref_interface<0x130bbec>(self + REF);
    if (!found) return;
    ((void(__thiscall *)(void *, void *, void *))(*(void ***)context)[0])(context, found + 0x40, self);
}
static const uint8_t part_update_original[] = {0x53, 0x8b, 0x5c, 0x24, 0x08, 0x56, 0x57, 0x53, 0x8b, 0xf1, 0xe8};
REWRITE("Part::Update4ABD00", 0x4abd00, part_update_original, (&part_update<0x36c, 0x534e60>), &g_part_updates);
REWRITE("Part::Update4ABD00", 0x5863f0, part_update_original, (&part_update<0x194, 0x4bee90>), &g_part_updates);
REWRITE("Part::Update4ABD00", 0x591af0, part_update_original, (&part_update<0x1ac, 0x4bee90>), &g_part_updates);
REWRITE("Part::Update4ABD00", 0xa7c190, part_update_original, (&part_update<0x16c, 0x4bee90>), &g_part_updates);

// ---- Enabled::Interface: the cached interface, once this is enabled ---------------------------------------------------------
static volatile long g_enabled_interfaces;
template <uint32_t REF> static void *__fastcall enabled_interface(uint8_t *self) {
    g_enabled_interfaces++;
    return self[0xc0] ? ref_interface<0x12f5608>(self + REF) : nullptr;
}
static const uint8_t enabled_interface_original[] = {0x57, 0x8b, 0xf9, 0x8a, 0x87, 0xc0, 0x00, 0x00, 0x00};
REWRITE("Enabled::Interface512080", 0x512080, enabled_interface_original, (&enabled_interface<0x138>), &g_enabled_interfaces);
REWRITE("Enabled::Interface512080", 0x5120f0, enabled_interface_original, (&enabled_interface<0x138>), &g_enabled_interfaces);
REWRITE("Enabled::Interface512080", 0x512160, enabled_interface_original, (&enabled_interface<0x154>), &g_enabled_interfaces);

// ---- Option::Toggle: in the states that allow it, an option flipped, its bit passed on, and the display refreshed -----------------
static volatile long g_option_toggles;
template <uint32_t OPTION, uint32_t BIT> static void __fastcall option_toggle(uint8_t *self) {
    g_option_toggles++;
    int32_t state = *(int32_t *)(self + 0x144);
    if (state != 1 && state != 3 && state != 4) return;
    *(uint32_t *)(self + OPTION) = !*(uint32_t *)(self + OPTION);
    ((void(__thiscall *)(void *, uint32_t))0x4f1740)(self, BIT);
    ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))0x4f5530)(self, 0, 1, 0, 0);
}
static const uint8_t option_toggle_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x86, 0x44, 0x01, 0x00, 0x00};
REWRITE("Option::Toggle4F6770", 0x4f6770, option_toggle_original, (&option_toggle<0x5f0, 0x08>), &g_option_toggles);
REWRITE("Option::Toggle4F6770", 0x4f67c0, option_toggle_original, (&option_toggle<0x5f4, 0x10>), &g_option_toggles);
REWRITE("Option::Toggle4F6770", 0x4f6810, option_toggle_original, (&option_toggle<0x5f8, 0x20>), &g_option_toggles);
REWRITE("Option::Toggle4F6770", 0x4f6860, option_toggle_original, (&option_toggle<0x5fc, 0x40>), &g_option_toggles);

// ---- Tables::Free: an array, a counted array of items that have destructors, and another array --------------------------------
static volatile long g_tables_frees;
static void __fastcall tables_free(uint8_t *self) {
    g_tables_frees++;
    memory_free_aligned(*(void **)(self + 0xc));
    uint8_t *items = *(uint8_t **)(self + 0x10);
    if (items) {
        int32_t *count = (int32_t *)(items - 4);
        ((void(__stdcall *)(void *, uint32_t, int32_t, void *))0xddcf39)(items, 4, *count, (void *)0x403de0);
        memory_free_aligned(count);
    }
    memory_free_aligned(*(void **)(self + 0x14));
}
static const uint32_t tables_free_copies[] = {0xbe0690, 0xbe06e0};
static const uint8_t tables_free_original[] = {0x57, 0x8b, 0xf9, 0x8b, 0x47, 0x0c, 0x50, 0xe8};
REWRITE_COPIES("Tables::FreeBE0690", tables_free_copies, tables_free_original, tables_free, &g_tables_frees);

// ---- Pair::DeletingDestroy: Pair::Delete, then the object's own memory if asked ---------------------------------------------------
static volatile long g_pair_destroys;
static void *__thiscall pair_destroy(void **self, uint32_t flags) {
    g_pair_destroys++;
    if (self[1]) delete_object(self[1]);
    if (self[0]) delete_object(self[0]);
    if (flags & 1) memory_free(self);
    return self;
}
static const uint32_t pair_destroy_copies[] = {0x86e610, 0xa1f9b0};
static const uint8_t pair_destroy_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x4e, 0x04, 0x85, 0xc9};
REWRITE_COPIES("Pair::Destroy86E610", pair_destroy_copies, pair_destroy_original, pair_destroy, &g_pair_destroys);

// ---- Pairs::Add: an 8-byte item added to {count, ..., data at +0xc}, after making room -------------------------------------------
static volatile long g_pair_adds;
template <uint32_t RESERVE> static void __thiscall pairs_add(uint32_t *self, const uint32_t *item) {
    g_pair_adds++;
    ((void(__thiscall *)(void *, uint32_t))RESERVE)(self, self[0] + 1);
    uint32_t *to = (uint32_t *)(uintptr_t)self[3] + self[0] * 2;
    to[0] = item[0];
    to[1] = item[1];
    self[0]++;
}
static const uint8_t pairs_add_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x06, 0x57, 0x40, 0x50, 0xe8};
REWRITE("Pairs::AddCB2B40", 0xcb2b40, pairs_add_original, (&pairs_add<0xcb29a0>), &g_pair_adds);
REWRITE("Pairs::AddCB2B40", 0xcbd4b0, pairs_add_original, (&pairs_add<0xcbcf50>), &g_pair_adds);

// ---- Wrapped::Construct: the real constructor, called with one argument, returning the object --------------------------------------
static volatile long g_wrapped_constructs;
template <uint32_t CONSTRUCT> static void *__thiscall wrapped_construct(void *self, uint32_t value) {
    g_wrapped_constructs++;
    ((void(__thiscall *)(void *, uint32_t))CONSTRUCT)(self, value);
    return self;
}
static const uint8_t wrapped_construct_original[] = {0x51, 0x8b, 0x44, 0x24, 0x08, 0x56, 0x50, 0x8b, 0xf1};
REWRITE("Wrapped::Construct49D580", 0x49d580, wrapped_construct_original, (&wrapped_construct<0x49cd90>), &g_wrapped_constructs);
REWRITE("Wrapped::Construct49D580", 0x49dd10, wrapped_construct_original, (&wrapped_construct<0x49d5a0>), &g_wrapped_constructs);

// ---- Reference::ConstructWith: Reference::Construct's start, with one field set to this class's start value ------------------------
static volatile long g_ref_constructs_with;
template <uint32_t FIELD, uint32_t VALUE> static uint8_t *__fastcall reference_construct_with(uint8_t *self) {
    g_ref_constructs_with++;
    *(uint32_t *)self = 0xf26ae0;
    *(uint8_t **)(self + 0x10) = self;
    uint8_t *handle = ((uint8_t * (__cdecl *)())0x805750)();
    *(uint8_t **)(self + 4) = handle;
    link_after(*(Node **)(handle + 0x20), (Node *)(self + 8));
    *(uint32_t *)(self + FIELD) = VALUE;
    return self;
}
REWRITE("Reference::ConstructWith9D7050", 0x9d7050, ref_construct_original, (&reference_construct_with<0x24, 0>), &g_ref_constructs_with);
REWRITE("Reference::ConstructWith9D7050", 0xaaf240, ref_construct_original, (&reference_construct_with<0x14, 0xbf800000>), &g_ref_constructs_with);

// ---- Items::Add: a value added to {data, count} at DATA of this object, after it has grown ----------------------------------------
static volatile long g_item_adds;
template <uint32_t DATA, uint32_t GROW> static void __thiscall items_add(uint8_t *self, uint32_t value) {
    g_item_adds++;
    uint32_t *items = (uint32_t *)(self + DATA);
    ((void(__thiscall *)(void *, uint32_t))GROW)(items, items[1] + 1);
    ((uint32_t *)(uintptr_t)items[0])[items[1] - 1] = value;
}
static const uint8_t items_add_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x86};
REWRITE("Items::Add66A810", 0x66a810, items_add_original, (&items_add<0xdc, 0x66a4c0>), &g_item_adds);
REWRITE("Items::Add66A810", 0x672130, items_add_original, (&items_add<0x148, 0x6719e0>), &g_item_adds);
REWRITE("Items::Add66A810", 0x9d0a90, items_add_original, (&items_add<0x214, 0x9ce530>), &g_item_adds);

// ---- Buffer::Free: a buffer freed with the C runtime, forgotten if it is the current one ---------------------------------------------
static volatile long g_buffer_frees;
template <uint32_t CURRENT> static void __fastcall buffer_free(uint8_t *self) {
    g_buffer_frees++;
    if (!self) return;
    if (self == *(uint8_t **)CURRENT) *(uint8_t **)CURRENT = nullptr;
    void(__cdecl *const crt_free)(void *) = (void(__cdecl *)(void *))0xdddc77;
    if (*(uint32_t *)(self + 0x14)) crt_free(*(void **)(self + 4));
    crt_free(self);
}
static const uint8_t buffer_free_original[] = {0x56, 0x8b, 0xf1, 0x85, 0xf6, 0x74, 0x2e};
REWRITE("Buffer::FreeC0ACF0", 0xc0acf0, buffer_free_original, (&buffer_free<0x13e32f8>), &g_buffer_frees);
REWRITE("Buffer::FreeC0ACF0", 0xcbc7a0, buffer_free_original, (&buffer_free<0x13f7014>), &g_buffer_frees);

// ---- Chain::Broadcast: a value given to each object of a chain; with no key it is also kept here ---------------------------------
static volatile long g_broadcasts;
template <uint32_t KEPT, uint32_t SLOT> static void __thiscall chain_broadcast(uint8_t *self, uint32_t key, uint32_t value) {
    g_broadcasts++;
    if (!key) *(uint32_t *)(self + KEPT) = value;
    for (uint8_t *item = (uint8_t *)ref_interface<0x12f5638>(self + 0x2c); item;
         item = ((uint8_t * (__fastcall *)(void *))0x5d49d0)(item)) {
        void *listener = item + 0x40;
        ((void(__thiscall *)(void *, uint32_t, uint32_t))*(void **)(*(uint8_t **)listener + SLOT))(listener, key, value);
    }
}
static const uint8_t broadcast_original[] = {0x53, 0x8b, 0x5c, 0x24, 0x08, 0x85, 0xdb};
REWRITE("Chain::Broadcast59BB40", 0x59bb40, broadcast_original, (&chain_broadcast<0x64, 4>), &g_broadcasts);
REWRITE("Chain::Broadcast59BB40", 0x59bbc0, broadcast_original, (&chain_broadcast<0x68, 8>), &g_broadcasts);
REWRITE("Chain::Broadcast59BB40", 0x59bc40, broadcast_original, (&chain_broadcast<0x6c, 0xc>), &g_broadcasts);

// ---- Grid::InterfaceAt: the interface of the entry at a row and column of a table of 20-byte entries ---------------------------------
static volatile long g_grid_interfaces;
template <uint32_t TABLE> static void *__thiscall grid_interface(uint8_t *self, int32_t row, int32_t column) {
    g_grid_interfaces++;
    uint8_t *entry = self + TABLE + (column + row * 4) * 20;
    return find_interface(*(uint8_t **)(entry + 4), (const int32_t *)0x1310f90);
}
static const uint8_t grid_interface_original[] = {0x8b, 0x44, 0x24, 0x08, 0x8b, 0x54, 0x24, 0x04, 0x8d, 0x04, 0x90};
REWRITE("Grid::InterfaceAt730720", 0x730720, grid_interface_original, (&grid_interface<0xe4>), &g_grid_interfaces);
REWRITE("Grid::InterfaceAt730720", 0x730770, grid_interface_original, (&grid_interface<0x6d4>), &g_grid_interfaces);

// ---- Sound: a sound {name at NAME, loaded at LOADED, owner at OWNER, stopped flag at FLAG} and the mixer its owner holds at +0x470 -
template <uint32_t OWNER> static inline void *sound_mixer(uint8_t *sound) { return *(void **)(*(uint8_t **)(sound + OWNER) + 0x470); }
template <uint32_t LOAD, uint32_t OWNER, uint32_t NAME> static inline uint32_t sound_load(uint8_t *sound) {
    return ((uint32_t(__thiscall *)(void *, void *))LOAD)(sound_mixer<OWNER>(sound), sound + NAME);
}
static const uint8_t sound_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46};
static const uint8_t sound_list_original[] = {0x57, 0x8b, 0x79, 0x04, 0x83, 0x3f, 0x00};
// Sound::Load: loaded again from its name.
static volatile long g_sound_loads;
template <uint32_t LOAD, uint32_t OWNER, uint32_t NAME, uint32_t LOADED> static void __fastcall sound_reload(uint8_t *self) {
    g_sound_loads++;
    *(uint32_t *)(self + LOADED) = sound_load<LOAD, OWNER, NAME>(self);
}
REWRITE("Sound::LoadBFA560", 0xbfa560, sound_original, (&sound_reload<0xc5c7f0, 0x30, 0x08, 0x28>), &g_sound_loads);
REWRITE("Sound::LoadBFA560", 0xc2f0b0, sound_original, (&sound_reload<0xc5c990, 0x28, 0x14, 0x24>), &g_sound_loads);
// Sounds::LoadAll: the same for each sound of a list.
static volatile long g_sound_load_alls;
template <uint32_t LOAD, uint32_t OWNER, uint32_t NAME, uint32_t LOADED> static void __fastcall sounds_load_all(uint8_t *self) {
    g_sound_load_alls++;
    for (Node *n = *(Node **)(self + 4); n->next; n = n->next) {
        uint8_t *sound = (uint8_t *)n->value;
        *(uint32_t *)(sound + LOADED) = sound_load<LOAD, OWNER, NAME>(sound);
    }
}
REWRITE("Sounds::LoadAllBFA8E0", 0xbfa8e0, sound_list_original, (&sounds_load_all<0xc5c7f0, 0x30, 0x08, 0x28>), &g_sound_load_alls);
REWRITE("Sounds::LoadAllBFA8E0", 0xc2f250, sound_list_original, (&sounds_load_all<0xc5c990, 0x28, 0x14, 0x24>), &g_sound_load_alls);
// Sounds::UnloadAll: each sound given back to its mixer and marked stopped.
static volatile long g_sound_unloads;
template <uint32_t UNLOAD, uint32_t OWNER, uint32_t LOADED, uint32_t FLAG> static void __fastcall sounds_unload_all(uint8_t *self) {
    g_sound_unloads++;
    for (Node *n = *(Node **)(self + 4); n->next; n = n->next) {
        uint8_t *sound = (uint8_t *)n->value;
        ((void(__thiscall *)(void *, uint32_t))UNLOAD)(sound_mixer<OWNER>(sound), *(uint32_t *)(sound + LOADED));
        *(uint32_t *)(sound + LOADED) = 0;
        sound[FLAG] = 1;
    }
}
REWRITE("Sounds::UnloadAllBFA8A0", 0xbfa8a0, sound_list_original, (&sounds_unload_all<0xc5cb90, 0x30, 0x28, 0x34>), &g_sound_unloads);
REWRITE("Sounds::UnloadAllBFA8A0", 0xc2f210, sound_list_original, (&sounds_unload_all<0xc5cc00, 0x28, 0x24, 0x2c>), &g_sound_unloads);
// Sound::Length: loading it if needed; the loaded sound's own value, or 0 if it can't be loaded.
static volatile long g_sound_lengths;
template <uint32_t LOAD, uint32_t OWNER, uint32_t NAME, uint32_t LOADED, uint32_t FLAG> static uint32_t __fastcall sound_length(uint8_t *self) {
    g_sound_lengths++;
    if (!*(uint32_t *)(self + LOADED)) {
        *(uint32_t *)(self + LOADED) = sound_load<LOAD, OWNER, NAME>(self);
        if (!*(uint32_t *)(self + LOADED)) return 0;
    }
    self[FLAG] = 0;
    return *(uint32_t *)(*(uint8_t **)(self + LOADED) + LOADED);
}
REWRITE("Sound::LengthBF9D70", 0xbf9d70, sound_original, (&sound_length<0xc5c7f0, 0x30, 0x08, 0x28, 0x34>), &g_sound_lengths);
REWRITE("Sound::LengthBF9D70", 0xbfb610, sound_original, (&sound_length<0xc5c990, 0x28, 0x14, 0x24, 0x2c>), &g_sound_lengths);

// ---- Control::PassOn: an argument given to the held control, if it is willing ------------------------------------------------------
static volatile long g_pass_ons;
template <uint32_t SLOT> static void __thiscall control_pass_on(uint8_t *self, uint32_t value) {
    g_pass_ons++;
    void *control = *(void **)(self + 0x120);
    if (!control) return;
    if (!(uint8_t)((uint32_t(__fastcall *)(void *))*(void **)(*(uint8_t **)control + 0x110))(control)) return;
    ((void(__thiscall *)(void *, uint32_t))*(void **)(*(uint8_t **)control + SLOT))(control, value);
}
static const uint8_t pass_on_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x86, 0x20, 0x01, 0x00, 0x00};
REWRITE("Control::PassOnB5B1D0", 0xb5b1d0, pass_on_original, (&control_pass_on<0x50>), &g_pass_ons);
REWRITE("Control::PassOnB5B1D0", 0xb5b200, pass_on_original, (&control_pass_on<0x5c>), &g_pass_ons);

// ---- Twice::Run: two steps given the same four arguments; the first also takes a zero ----------------------------------------------
static volatile long g_twice_runs;
template <uint32_t SECOND> static void __thiscall twice_run(void *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    g_twice_runs++;
    ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))0xb9dae0)(self, a, b, c, d, 0);
    ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))SECOND)(self, a, b, c, d);
}
static const uint8_t twice_run_original[] = {0x8b, 0x44, 0x24, 0x04, 0x53, 0x8b, 0x5c, 0x24, 0x10};
REWRITE("Twice::RunBA2FF0", 0xba2ff0, twice_run_original, (&twice_run<0xb9de50>), &g_twice_runs);
REWRITE("Twice::RunBA2FF0", 0xba3070, twice_run_original, (&twice_run<0xb9dee0>), &g_twice_runs);

// ---- Ready::Value: a value of the held object, once its reference is ready ---------------------------------------------------------
static volatile long g_ready_values;
template <uint32_t VALUE> static uint32_t __fastcall ready_value(uint8_t *self) {
    g_ready_values++;
    if (!(uint8_t)((uint32_t(__thiscall *)(void *, uint32_t))0xbd1420)(self + 0x84, 0)) return 0;
    return *(uint32_t *)(*(uint8_t **)(self + 0x8c) + VALUE);
}
static const uint8_t ready_value_original[] = {0x56, 0x8b, 0xf1, 0x6a, 0x00, 0x8d, 0x8e};
REWRITE("Ready::Value87F210", 0x87f210, ready_value_original, (&ready_value<0x3c>), &g_ready_values);
REWRITE("Ready::Value87F210", 0x87f240, ready_value_original, (&ready_value<0x40>), &g_ready_values);

// ---- Owned::DeleteHeld: the held object destroyed with its own function, and its memory freed ----------------------------------------
static volatile long g_held_deletes;
template <uint32_t DESTROY> static void __fastcall held_delete(void **self) {
    g_held_deletes++;
    void *held = *self;
    if (!held) return;
    call_plain<DESTROY>(held);
    memory_free(held);
}
static const uint8_t held_delete_original[] = {0x56, 0x8b, 0x31, 0x85, 0xf6, 0x74, 0x10};
REWRITE("Owned::DeleteHeld4085F0", 0x4085f0, held_delete_original, (&held_delete<0xcc2090>), &g_held_deletes);
REWRITE("Owned::DeleteHeld4085F0", 0x809410, held_delete_original, (&held_delete<0xbe4f30>), &g_held_deletes);

// ---- Second wave-1 pass ----------------------------------------------------------------------------------------------------------

// ---- Device::Destroy: the held COM object released, then the memory freed if asked ---------------------------------------------------
static volatile long g_w1b_com_dtors;
template <uint32_t TABLE, uint32_t HELD> static void *__thiscall w1b_com_dtor(uint8_t *self, uint32_t flags) {
    g_w1b_com_dtors++;
    *(uint32_t *)self = TABLE;
    void *held = *(void **)(self + HELD);
    if (held) {
        ((void(__stdcall *)(void *))(*(void ***)held)[2])(held);
        *(void **)(self + HELD) = nullptr;
    }
    if (flags & 1) memory_free(self);
    return self;
}
static const uint8_t w1b_com_dtor_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46};
REWRITE("Device::DestroyC5C5B0", 0xc5c5b0, w1b_com_dtor_original, (&w1b_com_dtor<0xfb7588, 0x28>), &g_w1b_com_dtors);
REWRITE("Device::DestroyC5C5B0", 0xc5c5f0, w1b_com_dtor_original, (&w1b_com_dtor<0xfb758c, 0x28>), &g_w1b_com_dtors);
REWRITE("Device::DestroyC5C5B0", 0xc5c630, w1b_com_dtor_original, (&w1b_com_dtor<0xfb7590, 0x24>), &g_w1b_com_dtors);

// ---- Pair::Construct: the base built from two 8-byte values passed by value, with its storage right after it ---------------------------
struct W1bPair { uint32_t first, second; };
static volatile long g_w1b_pair_ctors;
template <uint32_t TABLE> static void *__thiscall w1b_pair_ctor(uint8_t *self, W1bPair a, W1bPair b) {
    g_w1b_pair_ctors++;
    ((void(__thiscall *)(void *, uint32_t, void *, W1bPair *, W1bPair *))0x488290)(self, 0x40, self + 0x28, &a, &b);
    *(uint32_t *)self = TABLE;
    return self;
}
static const uint8_t w1b_pair_ctor_original[] = {0x56, 0x8b, 0xf1, 0x8d, 0x44, 0x24, 0x10, 0x50};
REWRITE("Pair::Construct6412A0", 0x6412a0, w1b_pair_ctor_original, (&w1b_pair_ctor<0xf4994c>), &g_w1b_pair_ctors);
REWRITE("Pair::Construct6412A0", 0x6412f0, w1b_pair_ctor_original, (&w1b_pair_ctor<0xf49954>), &g_w1b_pair_ctors);
REWRITE("Pair::Construct6412A0", 0x641320, w1b_pair_ctor_original, (&w1b_pair_ctor<0xf4995c>), &g_w1b_pair_ctors);

// ---- Twin::Copy: a copy of an object with two tables, the base given the field at +0x34 of the other ---------------------------------------
static volatile long g_w1b_twin_copies;
template <uint32_t TABLE, uint32_t SECOND> static void *__thiscall w1b_twin_copy(uint8_t *self, uint8_t *other) {
    g_w1b_twin_copies++;
    ((void(__thiscall *)(void *, uint32_t))0xbd9b20)(self, *(uint32_t *)(other + 0x34));
    *(uint32_t *)self = TABLE;
    *(uint32_t *)(self + 0x20) = SECOND;
    *(uint32_t *)(self + 0x34) = *(uint32_t *)(other + 0x34);
    return self;
}
static const uint8_t w1b_twin_copy_original[] = {0x56, 0x57, 0x8b, 0x7c, 0x24, 0x0c, 0x8b, 0x47, 0x34, 0x50};
REWRITE("Twin::CopyBDBE10", 0xbdbe10, w1b_twin_copy_original, (&w1b_twin_copy<0xfa5420, 0xfa5418>), &g_w1b_twin_copies);
REWRITE("Twin::CopyBDBE10", 0xcaf8e0, w1b_twin_copy_original, (&w1b_twin_copy<0xfc091c, 0xfc0914>), &g_w1b_twin_copies);

// ---- Children::Each: virtual +0xe4 of everything on a list that ends at the end node inside the object --------------------------------------
static volatile long g_w1b_eaches;
template <uint32_t FIRST, uint32_t END> static void __fastcall w1b_each(uint8_t *self) {
    g_w1b_eaches++;
    Node *end = (Node *)(self + END);
    for (Node *n = *(Node **)(self + FIRST); n != end; n = n->next) call_virtual<0xe4>(n->value);
}
template <uint8_t FIRST> static const uint8_t w1b_each_original[] = {0x53, 0x56, 0x8b, 0xb1, FIRST, 0x00, 0x00, 0x00};
REWRITE("Children::EachAD1350", 0xad1350, w1b_each_original<0xec>, (&w1b_each<0xec, 0xf8>), &g_w1b_eaches);
REWRITE("Children::EachAD1350", 0xaecbb0, w1b_each_original<0xe8>, (&w1b_each<0xe8, 0xf4>), &g_w1b_eaches);

// ---- Slot::Release: a held slot number given back to its owner; negative means none --------------------------------------------------------
static volatile long g_w1b_slot_releases;
template <uint32_t SLOT, uint32_t OWNER, uint32_t GIVE_BACK> static void __fastcall w1b_slot_release(uint8_t *self) {
    g_w1b_slot_releases++;
    int32_t slot = *(int32_t *)(self + SLOT);
    if (slot < 0) return;
    ((void(__thiscall *)(void *, int32_t))GIVE_BACK)(*(void **)OWNER, slot);
    *(int32_t *)(self + SLOT) = -1;
}
static const uint8_t w1b_slot_release_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x86};
REWRITE("Slot::Release557C20", 0x557c20, w1b_slot_release_original, (&w1b_slot_release<0x8c, 0x12f4e4c, 0x4cde30>), &g_w1b_slot_releases);
REWRITE("Slot::Release557C20", 0xaae260, w1b_slot_release_original, (&w1b_slot_release<0x13c, 0x13cdfe4, 0xa9ef40>), &g_w1b_slot_releases);

// ---- Registered::Destroy: the table set, and the object taken off the global register if it is on it ----------------------------------------
static volatile long g_w1b_unregisters;
template <uint32_t TABLE, uint32_t FLAG, uint32_t REMOVE> static void __fastcall w1b_unregister(uint8_t *self) {
    g_w1b_unregisters++;
    *(uint32_t *)self = TABLE;
    if (!self[FLAG]) return;
    self[FLAG] = 0;
    ((void(__thiscall *)(void *, void *))REMOVE)(*(void **)0x13d0d84, self);
}
template <uint8_t LOW, uint8_t HIGH> static const uint8_t w1b_unregister_original[] = {0x8a, 0x81, LOW, HIGH, 0x00, 0x00};
REWRITE("Registered::DestroyBD37E0", 0xbd37e0, (w1b_unregister_original<0x98, 0x00>), (&w1b_unregister<0xfa1538, 0x98, 0xbd2960>), &g_w1b_unregisters);
REWRITE("Registered::DestroyBD37E0", 0xbd3850, (w1b_unregister_original<0x18, 0x01>), (&w1b_unregister<0xfa153c, 0x118, 0xbd29e0>), &g_w1b_unregisters);

// ---- Attached::Attach: the owner kept, and a new entry of `KIND` added to it, written into a field of the object ------------------------------
static volatile long g_w1b_attaches;
template <uint32_t HELD, uint32_t KIND> static uint32_t __thiscall w1b_attach(uint8_t *self, void *owner) {
    g_w1b_attaches++;
    *(void **)(self + HELD) = owner;
    uint32_t entry = ((uint32_t(__fastcall *)(void *))0xc51360)(owner);
    return ((uint32_t(__thiscall *)(void *, uint32_t, void *, uint32_t, uint32_t, uint32_t, uint32_t))0xc53340)(owner, entry, self + HELD + 4, KIND, 0, 0, 0);
}
static const uint8_t w1b_attach_original[] = {0x56, 0x8b, 0x74, 0x24, 0x08, 0x6a, 0x00, 0x6a, 0x00};
REWRITE("Attached::AttachCB0BB0", 0xcb0bb0, w1b_attach_original, (&w1b_attach<0x30, 0x2e>), &g_w1b_attaches);
REWRITE("Attached::AttachCB0BB0", 0xcb7980, w1b_attach_original, (&w1b_attach<0x1c, 0x06>), &g_w1b_attaches);

// ---- Named::Value: something named looked up through a reference on the stack, its value kept past the end of the reference --------------------
static volatile long g_w1b_named_values;
template <uint32_t NAME> static uint32_t __cdecl w1b_named_value() {
    g_w1b_named_values++;
    uint8_t reference[0x1c];
    uint8_t *found = ((uint8_t * (__thiscall *)(void *, uint32_t, uint32_t, uint32_t))0x41cb90)(reference, NAME, 0x1303cfc, 1);
    uint32_t value = *(uint32_t *)(found + 8);
    ((void(__fastcall *)(void *))0xbd1880)(reference);
    return value;
}
static const uint8_t w1b_named_value_original[] = {0x83, 0xec, 0x1c, 0x56, 0x6a, 0x01, 0x68, 0xfc, 0x3c, 0x30, 0x01};
REWRITE("Named::ValueB70080", 0xb70080, w1b_named_value_original, (&w1b_named_value<0xf9b4e4>), &g_w1b_named_values);
REWRITE("Named::ValueB70080", 0xb700b0, w1b_named_value_original, (&w1b_named_value<0xf9b4f0>), &g_w1b_named_values);
REWRITE("Named::ValueB70080", 0xb700e0, w1b_named_value_original, (&w1b_named_value<0xf9b4fc>), &g_w1b_named_values);
REWRITE("Named::ValueB70080", 0xb70110, w1b_named_value_original, (&w1b_named_value<0xf9b50c>), &g_w1b_named_values);

// ---- Buffers::Destroy: two aligned buffers freed, then the memory if asked ---------------------------------------------------------------------
static volatile long g_w1b_buffer_dtors;
template <uint32_t TABLE, uint32_t FIRST, uint32_t SECOND> static void *__thiscall w1b_buffer_dtor(uint8_t *self, uint32_t flags) {
    g_w1b_buffer_dtors++;
    *(uint32_t *)self = TABLE;
    memory_free_aligned(*(void **)(self + FIRST));
    memory_free_aligned(*(void **)(self + SECOND));
    if (flags & 1) memory_free(self);
    return self;
}
static const uint8_t w1b_buffer_dtor_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46};
REWRITE("Buffers::Destroy891760", 0x891760, w1b_buffer_dtor_original, (&w1b_buffer_dtor<0xf7a1ec, 0x14, 0x50>), &g_w1b_buffer_dtors);
REWRITE("Buffers::Destroy891760", 0xbdd7e0, w1b_buffer_dtor_original, (&w1b_buffer_dtor<0xfa55dc, 0x04, 0x08>), &g_w1b_buffer_dtors);
REWRITE("Buffers::Destroy891760", 0xc50d60, w1b_buffer_dtor_original, (&w1b_buffer_dtor<0xfb7364, 0x08, 0x0c>), &g_w1b_buffer_dtors);

// ---- Twice::Whole: the same two steps as Twice::Run, over the whole of the area from (0,0) ---------------------------------------------------
static volatile long g_w1b_wholes;
template <uint32_t SECOND> static void __fastcall w1b_whole(uint8_t *self) {
    g_w1b_wholes++;
    ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))0xb9dae0)(
        self, 0, 0, *(uint32_t *)(self + 0x64) - 1, *(uint32_t *)(self + 0x68) - 1, 1);
    ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))SECOND)(self, 0, 0, *(uint32_t *)(self + 0x64) - 1, *(uint32_t *)(self + 0x68) - 1);
}
static const uint8_t w1b_whole_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x68, 0x8b, 0x4e, 0x64};
REWRITE("Twice::WholeBA2FB0", 0xba2fb0, w1b_whole_original, (&w1b_whole<0xb9de50>), &g_w1b_wholes);
REWRITE("Twice::WholeBA2FB0", 0xba3030, w1b_whole_original, (&w1b_whole<0xb9dee0>), &g_w1b_wholes);

// ---- Array::Delete: a counted array destroyed item by item, freed from its count, and forgotten ---------------------------------------------
static volatile long g_w1b_array_deletes;
template <uint32_t HELD, uint32_t SIZE, uint32_t ITEM_DTOR> static void __fastcall w1b_array_delete(uint8_t *self) {
    g_w1b_array_deletes++;
    uint8_t *items = *(uint8_t **)(self + HELD);
    if (!items) return;
    uint32_t *block = (uint32_t *)items - 1;
    ((void(__stdcall *)(void *, uint32_t, uint32_t, uint32_t))0xddcf39)(items, SIZE, *block, ITEM_DTOR);
    memory_free_aligned(block);
    *(void **)(self + HELD) = nullptr;
}
static const uint8_t w1b_array_delete_original[] = {0x57, 0x8b, 0xf9, 0x8b, 0x47};
REWRITE("Array::Delete421120", 0x421120, w1b_array_delete_original, (&w1b_array_delete<0x20, 0x14, 0x41bc10>), &g_w1b_array_deletes);
REWRITE("Array::Delete421120", 0x43ef00, w1b_array_delete_original, (&w1b_array_delete<0x10, 0x04, 0x4012e0>), &g_w1b_array_deletes);
REWRITE("Array::Delete421120", 0x519eb0, w1b_array_delete_original, (&w1b_array_delete<0x08, 0x10, 0x5106c0>), &g_w1b_array_deletes);

// ---- Pending::Finish: the common step, then a pending number (0 to 3) handed to the manager and cleared ------------------------------------------
static volatile long g_w1b_finishes;
template <uint32_t STEP, uint32_t PENDING> static void __fastcall w1b_finish(uint8_t *self) {
    g_w1b_finishes++;
    call_plain<STEP>(self);
    int32_t pending = *(int32_t *)(self + PENDING);
    if (pending < 0 || pending > 3) return;
    *(int32_t *)(self + PENDING) = -1;
    void *manager = ((void *(__cdecl *)())0x42df90)();
    ((void(__thiscall *)(void *, int32_t))0x6595d0)(manager, pending);
}
REWRITE("Pending::Finish65EA60", 0x65ea60, (push_call_original<0x65ea60, 0x5f7710>), (&w1b_finish<0x5f7710, 0x134>), &g_w1b_finishes);
REWRITE("Pending::Finish65EA60", 0xa04d70, (push_call_original<0xa04d70, 0x8e58f0>), (&w1b_finish<0x8e58f0, 0x130>), &g_w1b_finishes);
REWRITE("Pending::Finish65EA60", 0xa086d0, (push_call_original<0xa086d0, 0x5f7710>), (&w1b_finish<0x5f7710, 0x1d4>), &g_w1b_finishes);

// ---- Held::Past: the address of the held object, moved past two fixed sizes if it is in use --------------------------------------------------
static volatile long g_w1b_pasts;
template <uint32_t HELD, uint32_t IN_USE> static uint32_t __fastcall w1b_past(uint8_t *self) {
    g_w1b_pasts++;
    uint8_t *held = *(uint8_t **)(self + HELD);
    if (!held) return 0;
    if (!(uint8_t)((uint32_t(__fastcall *)(void *))IN_USE)(held)) return (uint32_t)(uintptr_t)held;
    uint32_t first = ((uint32_t(__cdecl *)())0xbb3d70)();
    return ((uint32_t(__cdecl *)())0xbb51b0)() + (uint32_t)(uintptr_t)held + first;
}
template <uint8_t LOW, uint8_t HIGH> static const uint8_t w1b_past_original[] = {0x56, 0x8b, 0xb1, LOW, HIGH, 0x00, 0x00};
REWRITE("Held::Past4A1800", 0x4a1800, (w1b_past_original<0xc8, 0x00>), (&w1b_past<0xc8, 0xbb5120>), &g_w1b_pasts);
REWRITE("Held::Past4A1800", 0x4a1840, (w1b_past_original<0x00, 0x01>), (&w1b_past<0x100, 0xbb4230>), &g_w1b_pasts);

// ---- Panel::Sync: the held panel told yes or no, from what the current state answers ------------------------------------------------------------
static volatile long g_w1b_syncs;
static void __fastcall w1b_sync(uint8_t *self) {
    g_w1b_syncs++;
    void *state = ((void *(__fastcall *)(void *))0x40a090)(self);
    bool yes = (uint8_t)((uint32_t(__fastcall *)(void *))0x8d30d0)(state);
    ((void(__thiscall *)(void *, uint32_t))0x67a580)(*(void **)(self + 0x13c), yes ? 1 : 0);
}
REWRITE("Panel::Sync62E790", 0x62e790, (push_call_original<0x62e790, 0x40a090>), (&w1b_sync), &g_w1b_syncs);
REWRITE("Panel::Sync62E790", 0x62e810, (push_call_original<0x62e810, 0x40a090>), (&w1b_sync), &g_w1b_syncs);

// ---- Sample::Drop: the held sample given back to the system that made it, and the object marked -----------------------------------------------------
static volatile long g_w1b_drops;
template <uint32_t SAMPLE, uint32_t OWNER, uint32_t GIVE_BACK, uint32_t MARK> static void __fastcall w1b_drop(uint8_t *self) {
    g_w1b_drops++;
    void *system = *(void **)(*(uint8_t **)(self + OWNER) + 0x470);
    ((void(__thiscall *)(void *, uint32_t))GIVE_BACK)(system, *(uint32_t *)(self + SAMPLE));
    *(uint32_t *)(self + SAMPLE) = 0;
    self[MARK] = 1;
}
static const uint8_t w1b_drop_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46};
REWRITE("Sample::DropBFA530", 0xbfa530, w1b_drop_original, (&w1b_drop<0x28, 0x30, 0xc5cb90, 0x34>), &g_w1b_drops);
REWRITE("Sample::DropBFA530", 0xc2f080, w1b_drop_original, (&w1b_drop<0x24, 0x28, 0xc5cc00, 0x2c>), &g_w1b_drops);
