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

// ---- Six::Free: six aligned buffers, one after the other ------------------------------------------------------------------------------
static volatile long g_w1b_six_frees;
static void __fastcall w1b_six_free(void **self) {
    g_w1b_six_frees++;
    for (int i = 0; i < 6; i++) memory_free_aligned(self[i]);
}
static const uint8_t w1b_six_free_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x06, 0x50};
REWRITE("Six::Free85A5E0", 0x85a5e0, w1b_six_free_original, (&w1b_six_free), &g_w1b_six_frees);
REWRITE("Six::Free85A5E0", 0xceab10, w1b_six_free_original, (&w1b_six_free), &g_w1b_six_frees);

// ---- Parts::Interface: the interface kept in reference number `index` of an array of 28-byte references -----------------------------------
static volatile long g_w1b_part_interfaces;
template <uint32_t ITEMS, uint32_t ID> static void *__thiscall w1b_part_interface(uint8_t *self, int32_t index) {
    g_w1b_part_interfaces++;
    return ref_interface<ID>(*(uint8_t **)(self + ITEMS) + index * 0x1c);
}
template <uint8_t ITEMS> static const uint8_t w1b_part_interface_original[] = {0x8b, 0x51, ITEMS, 0x56, 0x8b, 0x74, 0x24, 0x08};
REWRITE("Parts::Interface81F0B0", 0x81f0b0, w1b_part_interface_original<0x48>, (&w1b_part_interface<0x48, 0x130b540>), &g_w1b_part_interfaces);
REWRITE("Parts::Interface81F0B0", 0xae1130, w1b_part_interface_original<0x40>, (&w1b_part_interface<0x40, 0x13ce570>), &g_w1b_part_interfaces);

// ---- List::DeleteAll: every node taken off the list, and what it held destroyed and freed ---------------------------------------------------
static volatile long g_w1b_delete_alls;
template <uint32_t HEAD, uint32_t DESTROY> static void __fastcall w1b_delete_all(uint8_t *self) {
    g_w1b_delete_alls++;
    for (Node *n = *(Node **)(self + HEAD); n->next; n = *(Node **)(self + HEAD)) {
        unlink(n);
        void *value = n->value;
        if (value) {
            call_plain<DESTROY>(value);
            memory_free(value);
        }
    }
}
static const uint8_t w1b_delete_all_original[] = {0x53, 0x8b, 0xd9, 0x8b, 0x43};
REWRITE("List::DeleteAll419000", 0x419000, w1b_delete_all_original, (&w1b_delete_all<0x20, 0x4188f0>), &g_w1b_delete_alls);
REWRITE("List::DeleteAll419000", 0x82a670, w1b_delete_all_original, (&w1b_delete_all<0x28, 0x827660>), &g_w1b_delete_alls);

// ---- List::CopyFrom: an empty list built in place, then a clone of each item of the other list linked in at the front -----------------------
static volatile long g_w1b_list_copies;
template <uint32_t TABLE, uint32_t CLONE, uint32_t NODE> static void *__thiscall w1b_list_copy(uint32_t *self, uint8_t *other) {
    g_w1b_list_copies++;
    self[0] = TABLE;
    self[1] = (uint32_t)(uintptr_t)&self[4];   // the list: its own node at +4, its end node at +0x10
    self[2] = 0;
    self[3] = 0;
    self[4] = 0;
    self[5] = (uint32_t)(uintptr_t)&self[1];
    self[6] = 0;
    for (Node *n = *(Node **)(other + 4); n->next; n = n->next) {
        uint8_t *clone = ((uint8_t * (__thiscall *)(void *, uint32_t))CLONE)(n->value, 0);
        link_after((Node *)(uintptr_t)self[5], (Node *)(clone + NODE));
    }
    return self;
}
static const uint8_t w1b_list_copy_original[] = {0x56, 0x57, 0x8b, 0xf9, 0xc7, 0x07};
REWRITE("List::CopyFromC106B0", 0xc106b0, w1b_list_copy_original, (&w1b_list_copy<0xfafa9c, 0xc0e4c0, 0x80>), &g_w1b_list_copies);
REWRITE("List::CopyFromC106B0", 0xc10f90, w1b_list_copy_original, (&w1b_list_copy<0xfafaa0, 0xc10ca0, 0xb0>), &g_w1b_list_copies);

// ---- Items::Clear: each 28-byte item ended, then the array sized down to nothing --------------------------------------------------------------
static volatile long g_w1b_item_clears;
template <uint32_t ITEMS, uint32_t END_ITEM, uint32_t RESIZE> static void __fastcall w1b_item_clear(uint8_t *self) {
    g_w1b_item_clears++;
    for (int32_t i = 0; i < *(int32_t *)(self + ITEMS + 4); i++) call_plain<END_ITEM>(*(uint8_t **)(self + ITEMS) + i * 0x1c);
    ((void(__thiscall *)(void *, uint32_t))RESIZE)(self + ITEMS, 0);
}
static const uint8_t w1b_item_clear_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x86};
REWRITE("Items::Clear5DFB60", 0x5dfb60, w1b_item_clear_original, (&w1b_item_clear<0xfc, 0x5aaa50, 0x5aec30>), &g_w1b_item_clears);
REWRITE("Items::Clear5DFB60", 0xb8df10, w1b_item_clear_original, (&w1b_item_clear<0xd8, 0x4ad740, 0xb8cae0>), &g_w1b_item_clears);

// ---- Preset::Apply: row `index` of a table of 20-byte rows given to the target, first three fields of it, then the whole row ---------------------
static volatile long g_w1b_presets;
template <uint32_t TABLE, uint32_t FIELDS, uint32_t ROW> static uint32_t __fastcall w1b_preset(uint32_t index, void *target) {
    g_w1b_presets++;
    uint8_t *row = (uint8_t *)TABLE + index * 20;
    ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))FIELDS)(target, *(uint32_t *)(row + 8), *(uint32_t *)(row + 0xc), *(uint32_t *)(row + 0x10));
    return ((uint32_t(__thiscall *)(void *, void *))ROW)(target, row);
}
static const uint8_t w1b_preset_original[] = {0x56, 0x8d, 0x34, 0x89, 0x8b, 0x04, 0xb5};
REWRITE("Preset::Apply4796B0", 0x4796b0, w1b_preset_original, (&w1b_preset<0x10ed668, 0x6a8350, 0x6a8380>), &g_w1b_presets);
REWRITE("Preset::Apply4796B0", 0x479770, w1b_preset_original, (&w1b_preset<0x10ed860, 0x6a82d0, 0x6a8300>), &g_w1b_presets);

// ---- Spot::Send: three fields of the current object copied to the stack, 16-aligned, and handed on by address ------------------------------------
static volatile long g_w1b_spots;
static uint32_t __fastcall w1b_spot(void *self) {
    g_w1b_spots++;
    uint8_t *current = ((uint8_t * (__fastcall *)(void *))0x833850)(self);
    alignas(16) uint32_t spot[4];
    spot[0] = *(uint32_t *)(current + 0x120);
    spot[1] = *(uint32_t *)(current + 0x124);
    spot[2] = *(uint32_t *)(current + 0x128);
    void *target = ((void *(__cdecl *)())0x40a010)();
    return ((uint32_t(__thiscall *)(void *, void *, uint32_t))0x71b370)(target, spot, 0);
}
static const uint8_t w1b_spot_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x10};
REWRITE("Spot::Send7089E0", 0x7089e0, w1b_spot_original, (&w1b_spot), &g_w1b_spots);
REWRITE("Spot::Send7089E0", 0x786a40, w1b_spot_original, (&w1b_spot), &g_w1b_spots);

// ---- Chain::Run: a value worked out, the current holder fetched, and the value given to the holder ---------------------------------------------
static volatile long g_w1b_chain_runs;
template <uint32_t GIVE> static uint32_t __fastcall w1b_chain_run(void *self) {
    g_w1b_chain_runs++;
    uint32_t value = ((uint32_t(__fastcall *)(void *))0x5ebeb0)(self);
    void *holder = ((void *(__fastcall *)(void *))0x5ec2c0)(self);
    return ((uint32_t(__thiscall *)(void *, uint32_t))GIVE)(holder, value);
}
REWRITE("Chain::Run5EDDF0", 0x5eddf0, (push_call_original<0x5eddf0, 0x5ebeb0>), (&w1b_chain_run<0x5f1a50>), &g_w1b_chain_runs);
REWRITE("Chain::Run5EDDF0", 0x5ede10, (push_call_original<0x5ede10, 0x5ebeb0>), (&w1b_chain_run<0x5f1a80>), &g_w1b_chain_runs);

// ---- Ref::Tell: the interface in a reference inside the object, told about the object ------------------------------------------------------------
static volatile long g_w1b_ref_tells;
template <uint32_t REF, uint32_t ID, uint32_t TELL> static uint32_t __fastcall w1b_ref_tell(uint8_t *self) {
    g_w1b_ref_tells++;
    return ((uint32_t(__thiscall *)(void *, void *))TELL)(ref_interface<ID>(self + REF), self);
}
static const uint8_t w1b_ref_tell_original[] = {0x57, 0x8b, 0xf9, 0x8a, 0x87};
REWRITE("Ref::Tell5329C0", 0x5329c0, w1b_ref_tell_original, (&w1b_ref_tell<0x204, 0x12f5338, 0x55b5f0>), &g_w1b_ref_tells);
REWRITE("Ref::Tell5329C0", 0x564480, w1b_ref_tell_original, (&w1b_ref_tell<0xd8, 0x12f5638, 0x5e02c0>), &g_w1b_ref_tells);

// ---- Seven::First: the first of seven 20-byte entries whose interface has a state bit set is acted on; false if there is none ---------------------
static volatile long g_w1b_seven_firsts;
template <uint32_t BIT> static uint32_t __fastcall w1b_seven_first(uint8_t *self) {
    g_w1b_seven_firsts++;
    for (int32_t i = 0; i < 7; i++) {
        uint8_t *part = (uint8_t *)find_interface(*(uint8_t **)(self + 0x60 + i * 0x14), (const int32_t *)0x1342768);
        if (!part) continue;
        uint32_t state = *(uint32_t *)(*(uint8_t **)(part + 0xa0) + 0x28);
        if ((state >> BIT) & 1) return ((uint32_t(__thiscall *)(void *, int32_t))0x784320)(self, i);
    }
    return 0;
}
static const uint8_t w1b_seven_first_original[] = {0x53, 0x55, 0x56, 0x8b, 0xe9, 0x57, 0x33, 0xff};
REWRITE("Seven::First7854E0", 0x7854e0, w1b_seven_first_original, (&w1b_seven_first<7>), &g_w1b_seven_firsts);
REWRITE("Seven::First7854E0", 0x785560, w1b_seven_first_original, (&w1b_seven_first<2>), &g_w1b_seven_firsts);

// ---- Ref::Walk: a step given to the interface in a reference, then to each one after it --------------------------------------------------------
static volatile long g_w1b_ref_walks;
template <uint32_t STEP, uint32_t NEXT> static void __thiscall w1b_ref_walk(uint8_t *self, uint32_t value) {
    g_w1b_ref_walks++;
    for (void *item = ref_interface<0x12f5638>(self + 0x6c); item; item = ((void *(__fastcall *)(void *))NEXT)(item))
        ((void(__thiscall *)(void *, uint32_t))STEP)(item, value);
}
static const uint8_t w1b_ref_walk_original[] = {0x56, 0x57, 0x8b, 0xf9, 0x8a, 0x87, 0x84, 0x00, 0x00, 0x00};
REWRITE("Ref::Walk59ABE0", 0x59abe0, w1b_ref_walk_original, (&w1b_ref_walk<0x5d9cf0, 0x5d49d0>), &g_w1b_ref_walks);
REWRITE("Ref::Walk59ABE0", 0x59b0f0, w1b_ref_walk_original, (&w1b_ref_walk<0x5e40d0, 0x5d49d0>), &g_w1b_ref_walks);
REWRITE("Ref::Walk59ABE0", 0x59b950, w1b_ref_walk_original, (&w1b_ref_walk<0x5d9af0, 0x5d4970>), &g_w1b_ref_walks);

// ---- Object::Take: the object recorded, and its interface (or null) given to the owner, 12 bytes before this part ---------------------------------
static volatile long g_w1b_takes;
template <uint32_t ID, uint32_t GIVE> static uint32_t __thiscall w1b_take(uint8_t *self, void *object) {
    g_w1b_takes++;
    ((void(__thiscall *)(void *, void *))0x501f80)(self, object);
    void *part = nullptr;
    if (object) {
        ClassInterfaces *c = ((ClassInterfaces * (__fastcall *)(void *))(*(void ***)object)[1])(object);
        int32_t id = *(const int32_t *)ID;
        if (c && id < c->count && c->by_id[id]) part = (uint8_t *)object + c->by_id[id]->offset;
    }
    ((void(__thiscall *)(void *, void *))GIVE)(self - 0xc, part);
    return 1;
}
static const uint8_t w1b_take_original[] = {0x56, 0x8b, 0x74, 0x24, 0x08, 0x57, 0x56, 0x8b, 0xf9};
REWRITE("Object::Take4B7090", 0x4b7090, w1b_take_original, (&w1b_take<0x12f5544, 0x4b6d10>), &g_w1b_takes);
REWRITE("Object::Take4B7090", 0xa67c00, w1b_take_original, (&w1b_take<0x13429c8, 0xa64760>), &g_w1b_takes);

// ---- Third pass. seh=1 functions start `push -1; push HANDLER`, the handler being different in every copy ------------
template <uint32_t HANDLER> static const uint8_t w1c_seh[] = {0x6a, 0xff, 0x68, (uint8_t)HANDLER, (uint8_t)(HANDLER >> 8), (uint8_t)(HANDLER >> 16), (uint8_t)(HANDLER >> 24)};
static const uint8_t w1c_seh_late[] = {0x64, 0xa1, 0x00, 0x00, 0x00, 0x00, 0x6a, 0xff, 0x68};   // the frame's old value read first

// Tell a registry about a class: its name and the offset of what it registers, through a 20-byte text on the stack.
static inline void w1c_register(void *registry, uint32_t name, uint32_t offset) {
    uint32_t text[5];
    void *made = ((void *(__thiscall *)(void *, uint32_t, uint32_t))0xbeca40)(text, name, offset);
    ((void(__thiscall *)(void *, void *))0xbecc10)(registry, made);
    ((void(__fastcall *)(void *))0xbec5e0)(text);
}

// ---- Registered::Construct: made on the given registry or the class's own, which is marked as filled in afterwards ---
static volatile long g_w1c_registered;
template <uint32_t BASE, uint32_t OWN, uint32_t TABLE, uint32_t TABLE2, uint32_t NAME>
static uint32_t *__thiscall w1c_registered(uint32_t *self, uint8_t *registry, uint32_t a, uint32_t b) {
    g_w1c_registered++;
    bool own = !registry;
    uint8_t *&global = *(uint8_t **)OWN;
    ((void(__thiscall *)(void *, void *, uint32_t, uint32_t))BASE)(self, own ? global : registry, a, b);
    self[0] = TABLE;
    self[1] = TABLE2;
    if (own) registry = global;
    if (!registry[4]) {
        w1c_register(registry, NAME, 0);
        ((void(__fastcall *)(void *))(*(void ***)registry)[1])(registry);
    }
    if (own) global[4] = 1;
    return self;
}
REWRITE("Registered::Construct4D3CB0", 0x4d3cb0, (w1c_seh<0xe14330>), (&w1c_registered<0x800a80, 0x12f4eb4, 0xf335bc, 0xf335b8, 0x12f4ea4>), &g_w1c_registered);
REWRITE("Registered::Construct4D3CB0", 0x72c500, (w1c_seh<0xe4c4f0>), (&w1c_registered<0x72b390, 0x13063a8, 0xf6a6a8, 0xf6a6a0, 0x130636c>), &g_w1c_registered);
REWRITE("Registered::Construct4D3CB0", 0x98a480, (w1c_seh<0xe6dd00>), (&w1c_registered<0x800a80, 0x13ce440, 0xf7fc60, 0xf7fc5c, 0x13ce470>), &g_w1c_registered);
REWRITE("Registered::Construct4D3CB0", 0xa180c0, (w1c_seh<0xe7a0a0>), (&w1c_registered<0x800a80, 0x13426b8, 0xf87df0, 0xf87dec, 0x13426c0>), &g_w1c_registered);
REWRITE("Registered::Construct4D3CB0", 0xad93d0, (w1c_seh<0xe8b400>), (&w1c_registered<0xad8d10, 0x13ce4f4, 0xf95360, 0xf9535c, 0x13ce50c>), &g_w1c_registered);
REWRITE("Registered::Construct4D3CB0", 0xaf5d50, (w1c_seh<0xe8e060>), (&w1c_registered<0xaf58a0, 0x13ce63c, 0xf96924, 0xf96920, 0x13ce640>), &g_w1c_registered);
REWRITE("Registered::Construct4D3CB0", 0xb7d520, (w1c_seh<0xe97ee0>), (&w1c_registered<0xb7c7a0, 0x13d0128, 0xf9ba48, 0xf9ba44, 0x13d0078>), &g_w1c_registered);
REWRITE("Registered::Construct4D3CB0", 0xb7e580, (w1c_seh<0xe98190>), (&w1c_registered<0xb7c7a0, 0x13d00a8, 0xf9bbb8, 0xf9bbb0, 0x13d00e8>), &g_w1c_registered);

// ---- Interfaced::Construct: a second part at +4 made, then the class told to the registry with that part's offset ----
static volatile long g_w1c_interfaced;
template <uint32_t PART, uint32_t TABLE, uint32_t TABLE2, uint32_t OWN, uint32_t NAME>
static uint32_t *__thiscall w1c_interfaced(uint32_t *self, uint8_t *registry) {
    g_w1c_interfaced++;
    self[0] = 0xf2694c;
    ((void(__fastcall *)(void *))PART)(self + 1);
    self[0] = TABLE;
    self[1] = TABLE2;
    uint8_t *&global = *(uint8_t **)OWN;
    bool own = !registry;
    if (own) registry = global;
    if (!registry[4]) w1c_register(registry, NAME, 4);
    if (own) global[4] = 1;
    return self;
}
REWRITE("Interfaced::Construct79B340", 0x79b340, (w1c_seh<0xe5537b>), (&w1c_interfaced<0x8fb150, 0xf6fd00, 0xf6fcf4, 0x1306bc4, 0x1339fc4>), &g_w1c_interfaced);
REWRITE("Interfaced::Construct79B340", 0x967f20, (w1c_seh<0xe6b7bb>), (&w1c_interfaced<0x9655d0, 0xf7e6b4, 0xf7e6a0, 0x133d08c, 0x133b4dc>), &g_w1c_interfaced);
REWRITE("Interfaced::Construct79B340", 0x96afe0, (w1c_seh<0xe6ba2b>), (&w1c_interfaced<0x9655d0, 0xf7e74c, 0xf7e738, 0x133d0c8, 0x133b4dc>), &g_w1c_interfaced);
REWRITE("Interfaced::Construct79B340", 0x96b1d0, (w1c_seh<0xe6ba9b>), (&w1c_interfaced<0x9655d0, 0xf7e780, 0xf7e76c, 0x133d0d0, 0x133b4dc>), &g_w1c_interfaced);
REWRITE("Interfaced::Construct79B340", 0x96b6d0, (w1c_seh<0xe6bb0b>), (&w1c_interfaced<0x9655d0, 0xf7e7b4, 0xf7e7a0, 0x133d0d8, 0x133b4dc>), &g_w1c_interfaced);
REWRITE("Interfaced::Construct79B340", 0x96ba80, (w1c_seh<0xe6bb7b>), (&w1c_interfaced<0x9655d0, 0xf7e7f8, 0xf7e7e4, 0x133d0e0, 0x133b4dc>), &g_w1c_interfaced);

// ---- Named::Send: refresh, then a named reference on the stack is handed to what the object holds --------------------
static volatile long g_w1c_named_sends;
template <uint32_t NAME, uint32_t HELD> static void __fastcall w1c_named_send(uint8_t *self) {
    g_w1c_named_sends++;
    ((void(__fastcall *)(void *))0x6afd60)(self);
    uint8_t reference[0x1c];
    void *made = ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))0x4040f0)(reference, NAME, 0x1303cfc, 1);
    ((void(__thiscall *)(void *, void *))0xbd14e0)(*(uint8_t **)(self + HELD) + 0x114, made);
    ((void(__fastcall *)(void *))0xbd1880)(reference);
}
REWRITE("Named::Send5FD440", 0x5fd440, (w1c_seh<0xe27f68>), (&w1c_named_send<0xf414fc, 0xdc>), &g_w1c_named_sends);
REWRITE("Named::Send5FD440", 0x5fe210, (w1c_seh<0xe281e8>), (&w1c_named_send<0xf41560, 0xec>), &g_w1c_named_sends);
REWRITE("Named::Send5FD440", 0x601980, (w1c_seh<0xe289e8>), (&w1c_named_send<0xf41864, 0xe4>), &g_w1c_named_sends);
REWRITE("Named::Send5FD440", 0x605810, (w1c_seh<0xe29138>), (&w1c_named_send<0xf41c5c, 0xd4>), &g_w1c_named_sends);
REWRITE("Named::Send5FD440", 0x607500, (w1c_seh<0xe29418>), (&w1c_named_send<0xf41c84, 0xdc>), &g_w1c_named_sends);
REWRITE("Named::Send5FD440", 0x6baae0, (w1c_seh<0xe3f6a8>), (&w1c_named_send<0xf61900, 0xec>), &g_w1c_named_sends);

// ---- Spawn::Start: a new object for the owner 0x50 before this part; its third virtual function is given the value ---
static volatile long g_w1c_spawns;
template <uint32_t SIZE, uint32_t MAKE> static uint32_t __thiscall w1c_spawn(uint8_t *self, uint32_t value, uint32_t) {
    g_w1c_spawns++;
    void *memory = memory_alloc(SIZE);
    void *object = memory ? ((void *(__thiscall *)(void *, uint32_t, void *))MAKE)(memory, 0, self - 0x50) : nullptr;
    return ((uint32_t(__thiscall *)(void *, uint32_t))(*(void ***)object)[2])(object, value);
}
REWRITE("Spawn::Start52F4C0", 0x52f4c0, (w1c_seh<0xe1aecb>), (&w1c_spawn<0x2ec, 0x6377d0>), &g_w1c_spawns);
REWRITE("Spawn::Start52F4C0", 0x58fbe0, (w1c_seh<0xe2150b>), (&w1c_spawn<0x240, 0x635e90>), &g_w1c_spawns);
REWRITE("Spawn::Start52F4C0", 0x5a1530, (w1c_seh<0xe227eb>), (&w1c_spawn<0x344, 0x638790>), &g_w1c_spawns);
REWRITE("Spawn::Start52F4C0", 0xa7b310, (w1c_seh<0xe839cb>), (&w1c_spawn<0x290, 0x6367b0>), &g_w1c_spawns);
REWRITE("Spawn::Start52F4C0", 0xbacb70, (w1c_seh<0xe9be9b>), (&w1c_spawn<0x1e8, 0x637250>), &g_w1c_spawns);

// ---- Singleton::Make: a new object (which sets the global), started if its first flag bit is clear --------------------
static volatile long g_w1c_singletons;
template <uint32_t SIZE, uint32_t MAKE, uint32_t GLOBAL> static uint8_t *__cdecl w1c_singleton() {
    g_w1c_singletons++;
    void *memory = memory_alloc(SIZE);
    if (memory) ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))MAKE)(memory, 0, 0, 0);
    uint8_t *&global = *(uint8_t **)GLOBAL;
    if (!(global[0x2c] & 1)) ((void(__fastcall *)(void *))0x800860)(global);
    return global;
}
REWRITE("Singleton::Make6C1DA0", 0x6c1da0, (w1c_seh<0xe404bb>), (&w1c_singleton<0x1d8, 0x6bfce0, 0x1305dd8>), &g_w1c_singletons);
REWRITE("Singleton::Make6C1DA0", 0x708520, (w1c_seh<0xe49c6b>), (&w1c_singleton<0x1f0, 0x707680, 0x1306194>), &g_w1c_singletons);
REWRITE("Singleton::Make6C1DA0", 0x9ead70, (w1c_seh<0xe76fab>), (&w1c_singleton<0x118, 0x9e8ff0, 0x1341930>), &g_w1c_singletons);
REWRITE("Singleton::Make6C1DA0", 0xa5de40, (w1c_seh<0xe82b2b>), (&w1c_singleton<0x88, 0xa5d6f0, 0x134297c>), &g_w1c_singletons);
REWRITE("Singleton::Make6C1DA0", 0xa63450, (w1c_seh<0xe82e8b>), (&w1c_singleton<0x420, 0xa630e0, 0x13429a8>), &g_w1c_singletons);

// ---- Twice::Add: a temporary's part added, then a bare object of this class on the stack added -----------------------
static volatile long g_w1c_twice;
template <uint32_t TABLE> static uint32_t __thiscall w1c_twice(void *self, uint32_t value) {
    g_w1c_twice++;
    auto add = (uint32_t(__thiscall *)(void *, void *, uint32_t))0x6aed50;
    uint8_t temporary[0x34];
    uint8_t *made = ((uint8_t * (__thiscall *)(void *, uint32_t))0x6b0370)(temporary, 0);
    add(self, made ? made + 4 : nullptr, value);
    ((void(__fastcall *)(void *))0x6b0a90)(temporary);
    uint32_t bare = TABLE;
    return add(self, &bare, value);
}
REWRITE("Twice::Add6B0B50", 0x6b0b50, (w1c_seh<0xe3e308>), (&w1c_twice<0xf60650>), &g_w1c_twice);
REWRITE("Twice::Add6B0B50", 0x6b0bd0, (w1c_seh<0xe3e328>), (&w1c_twice<0xf60680>), &g_w1c_twice);
REWRITE("Twice::Add6B0B50", 0x6b0c50, (w1c_seh<0xe3e348>), (&w1c_twice<0xf60690>), &g_w1c_twice);
REWRITE("Twice::Add6B0B50", 0x6b0cd0, (w1c_seh<0xe3e368>), (&w1c_twice<0xf606c0>), &g_w1c_twice);

// ---- Pool::Make: if the value is accepted, a new object from the pool built on it; null if not or if the pool is full -
static volatile long g_w1c_pool_makes;
template <uint32_t SIZE, uint32_t MAKE> static void *w1c_pool_make_body(void *self, uint32_t value) {
    g_w1c_pool_makes++;
    if (!((uint8_t(__thiscall *)(void *, uint32_t))0xc0d5a0)(self, value)) return nullptr;
    void *memory = ((void *(__thiscall *)(void *, uint32_t, uint32_t))0xca1eb0)(*(uint8_t **)0x13e3318 + 4, 4, SIZE);
    return memory ? ((void *(__thiscall *)(void *, void *, uint32_t))MAKE)(memory, self, value) : nullptr;
}
template <uint32_t SIZE, uint32_t MAKE> static void *__thiscall w1c_pool_make(void *self, uint32_t value) { return w1c_pool_make_body<SIZE, MAKE>(self, value); }
template <uint32_t SIZE, uint32_t MAKE> static void *__thiscall w1c_pool_make2(void *self, uint32_t value, uint32_t) { return w1c_pool_make_body<SIZE, MAKE>(self, value); }
REWRITE("Pool::MakeC0E1B0", 0xc0e1b0, w1c_seh_late, (&w1c_pool_make<0x58, 0xc0e120>), &g_w1c_pool_makes);
REWRITE("Pool::MakeC0E1B0", 0xc0ec20, w1c_seh_late, (&w1c_pool_make<0x44, 0xc0eab0>), &g_w1c_pool_makes);
REWRITE("Pool::MakeC0E1B0", 0xc110e0, w1c_seh_late, (&w1c_pool_make<0x54, 0xc11010>), &g_w1c_pool_makes);
REWRITE("Pool::MakeC12600", 0xc12600, w1c_seh_late, (&w1c_pool_make2<0x44, 0xc12560>), &g_w1c_pool_makes);

// ---- Managed::Destroy: the class's global cleared, a member destroyed, then the base -----------------------------------
static volatile long g_w1c_managed;
template <uint32_t TABLE, uint32_t TABLE2, uint32_t MEMBER, uint32_t GLOBAL, uint32_t END> static void __fastcall w1c_managed(uint32_t *self) {
    g_w1c_managed++;
    self[0] = TABLE;
    self[1] = TABLE2;
    *(uint32_t *)GLOBAL = 0;
    ((void(__fastcall *)(void *))END)((uint8_t *)self + MEMBER);
    ((void(__fastcall *)(void *))0x800940)(self);
}
REWRITE("Managed::Destroy46D230", 0x46d230, (w1c_seh<0xe0c948>), (&w1c_managed<0xf2cabc, 0xf2cab8, 0x44, 0x12f0028, 0x40f730>), &g_w1c_managed);
REWRITE("Managed::Destroy46D230", 0x63f6b0, (w1c_seh<0xe317f8>), (&w1c_managed<0xf49814, 0xf49810, 0x40, 0x12f7eb4, 0x40f730>), &g_w1c_managed);
REWRITE("Managed::Destroy46D230", 0x7a7200, (w1c_seh<0xe561a8>), (&w1c_managed<0xf70910, 0xf7090c, 0x40, 0x1306c68, 0x40f730>), &g_w1c_managed);
REWRITE("Managed::Destroy46D230", 0x7bb910, (w1c_seh<0xe57208>), (&w1c_managed<0xf71978, 0xf71974, 0x40, 0x13070c8, 0x4951b0>), &g_w1c_managed);

// ---- Listener::Make: a 16-byte listener on `owner`, entered into `list` as a pair, then given its final table ----------
static volatile long g_w1c_listeners;
template <uint32_t TABLE> static uint32_t *__fastcall w1c_listener(uint32_t owner, void *list) {
    g_w1c_listeners++;
    uint32_t *made = (uint32_t *)memory_alloc(0x10);
    if (!made) return nullptr;
    made[0] = 0xf75170;
    made[1] = owner;
    uint32_t pair[2] = {(uint32_t)(uintptr_t)made, owner};
    ((void(__thiscall *)(void *, void *, void *))0x7fbf50)(list, &pair[1], &pair[0]);
    made[0] = TABLE;
    made[2] = 0xffffffff;
    made[3] = (uint32_t)(uintptr_t)list;
    return made;
}
REWRITE("Listener::Make7FD780", 0x7fd780, (w1c_seh<0xe5b50b>), (&w1c_listener<0xf75188>), &g_w1c_listeners);
REWRITE("Listener::Make7FD780", 0x7fe5c0, (w1c_seh<0xe5b74b>), (&w1c_listener<0xf7519c>), &g_w1c_listeners);
REWRITE("Listener::Make7FD780", 0x7fe920, (w1c_seh<0xe5b7eb>), (&w1c_listener<0xf751c8>), &g_w1c_listeners);

// ---- Items::Resize: the old storage dropped and `count` new 16-byte items built --------------------------------------
static volatile long g_w1c_resizes;
template <uint32_t BUILD> static uint32_t __thiscall w1c_resize(uint32_t *self, uint32_t count) {
    g_w1c_resizes++;
    if (self[0]) {
        memory_free_aligned((void *)(uintptr_t)self[0]);
        self[0] = 0;
    }
    if (count) {
        void *items = memory_alloc_aligned(count << 4);
        if (items) ((void(__stdcall *)(void *, uint32_t, uint32_t, uint32_t))0x402b90)(items, 0x10, count, BUILD);
        self[0] = (uint32_t)(uintptr_t)items;
    }
    self[1] = count;
    return count;
}
REWRITE("Items::Resize7E5350", 0x7e5350, w1c_seh_late, (&w1c_resize<0x407490>), &g_w1c_resizes);
REWRITE("Items::Resize7E5350", 0x980400, w1c_seh_late, (&w1c_resize<0x97ffb0>), &g_w1c_resizes);
REWRITE("Items::Resize7E5350", 0xabc440, w1c_seh_late, (&w1c_resize<0xabc130>), &g_w1c_resizes);

// ---- Holder::Construct: a holder of `value` with its counts cleared and an empty aligned buffer ------------------------
static volatile long g_w1d_holders;
static uint32_t *__thiscall w1d_holder(uint32_t *self, uint32_t value) {
    g_w1d_holders++;
    self[0] = value;
    self[1] = 0;
    self[2] = 0;
    self[5] = 0;
    self[4] = (uint32_t)(uintptr_t)memory_alloc_aligned(0);
    self[6] = 0;
    return self;
}
REWRITE("Holder::Construct7E6D70", 0x7e6d70, (w1c_seh<0xe595fb>), (&w1d_holder), &g_w1d_holders);
REWRITE("Holder::Construct7E6D70", 0xaaaf70, (w1c_seh<0xe87c9b>), (&w1d_holder), &g_w1d_holders);

// ---- Watched::Destroy: the inner part's table restored, its member at +0x24 destroyed, then the base table --------------
static volatile long g_w1d_watched;
static void __fastcall w1d_watched(uint32_t *self) {
    g_w1d_watched++;
    self[8] = 0xf70de0;
    ((void(__fastcall *)(void *))0x40f730)(self + 9);
    self[0] = 0xf708c8;
}
REWRITE("Watched::Destroy7CD180", 0x7cd180, (w1c_seh<0xe58128>), (&w1d_watched), &g_w1d_watched);
REWRITE("Watched::Destroy7CD180", 0x7cd1d0, (w1c_seh<0xe58148>), (&w1d_watched), &g_w1d_watched);

// ---- TextPair::Construct: two empty texts ------------------------------------------------------------------------------
static volatile long g_w1d_text_pairs;
static uint32_t *__fastcall w1d_text_pair(uint32_t *self) {
    g_w1d_text_pairs++;
    self[0] = ((uint32_t(__fastcall *)(uint8_t))0xbd0490)(1);
    self[1] = ((uint32_t(__fastcall *)(uint8_t))0xbd0490)(1);
    return self;
}
REWRITE("TextPair::Construct87F560", 0x87f560, (w1c_seh<0xe63958>), (&w1d_text_pair), &g_w1d_text_pairs);
REWRITE("TextPair::Construct87F560", 0xb32d20, (w1c_seh<0xe92f88>), (&w1d_text_pair), &g_w1d_text_pairs);

// ---- Handled::Forward: the handle's interface (by the id in a global, null if it has none) asked to act for us --------
static volatile long g_w1d_forwards;
template <uint32_t ID, uint32_t ACT> static uint32_t __thiscall w1d_forward(uint8_t *self, uint32_t value) {
    g_w1d_forwards++;
    void *iface = find_interface(*(uint8_t **)(self + 0x44), (const int32_t *)ID);
    return ((uint32_t(__thiscall *)(void *, void *, uint32_t))ACT)(iface, self, value);
}
static const uint8_t w1d_forward_original[] = {0x56, 0x57, 0x8b, 0xf9, 0x8b, 0x47, 0x44, 0x8b, 0x70, 0x0c};
REWRITE("Handled::Forward588AE0", 0x588ae0, w1d_forward_original, (&w1d_forward<0x12f53f0, 0x588260>), &g_w1d_forwards);
REWRITE("Handled::Forward588AE0", 0x5936a0, w1d_forward_original, (&w1d_forward<0x12f5494, 0x592240>), &g_w1d_forwards);

// ---- Linked::Destroy: the held object released, our list node taken out, the owner's back pointer cleared ------------
static volatile long g_w1d_linked;
template <uint32_t TABLE> static void __fastcall w1d_linked(uint32_t *self) {
    g_w1d_linked++;
    self[0] = TABLE;
    if (void *held = (void *)(uintptr_t)self[5]) ((void(__stdcall *)(void *))(*(void ***)held)[2])(held);
    unlink((Node *)(self + 1));
    *(uint32_t *)(uintptr_t)self[4] = 0;
    self[0] = 0xfb951c;
}
REWRITE("Linked::DestroyC72810", 0xc72810, (w1c_seh<0xea3bb8>), (&w1d_linked<0xfb9660>), &g_w1d_linked);
REWRITE("Linked::DestroyC72810", 0xc8b760, (w1c_seh<0xea4398>), (&w1d_linked<0xfbee2c>), &g_w1d_linked);

// ---- Owned::Construct: owner and size kept, a fresh 28-byte record (from ALLOC) that points back at us --------------
static volatile long g_w1d_owned;
template <uint32_t TABLE, uint32_t ALLOC> static uint32_t *__thiscall w1d_owned(uint32_t *self, uint32_t owner, uint32_t size) {
    g_w1d_owned++;
    self[1] = owner;
    self[2] = size;
    ((uint16_t *)self)[6] = 0;
    ((uint16_t *)self)[7] = 0;
    self[0] = TABLE;
    self[6] = (uint32_t)(uintptr_t)self;
    self[7] = size;
    uint32_t *record = ((uint32_t * (__cdecl *)())ALLOC)();   // a 28-byte record from its pool
    if (record)
        for (int i = 0; i < 7; i++) record[i] = 0;
    self[8] = (uint32_t)(uintptr_t)record;
    self[0xb] = 0;
    record[1] = (uint32_t)(uintptr_t)self;
    record[4] = size;
    return self;
}
REWRITE("Owned::ConstructBEA2D0", 0xbea2d0, (w1c_seh<0xe9e4b8>), (&w1d_owned<0xfa5734, 0xbea270>), &g_w1d_owned);
REWRITE("Owned::ConstructBEA2D0", 0xbfdef0, (w1c_seh<0xe9e968>), (&w1d_owned<0xfa5ca8, 0xbfde90>), &g_w1d_owned);

// ---- Shared::Destroy: our buffer freed, a member destroyed, and the shared block let go (freed with its data when its
// count reaches zero) ------------------------------------------------------------------------------------------------------
static volatile long g_w1d_shared;
template <uint32_t TABLE> static void __fastcall w1d_shared(uint32_t *self) {
    g_w1d_shared++;
    self[0] = TABLE;
    memory_free_aligned((void *)(uintptr_t)self[4]);
    ((void(__fastcall *)(void *))0xbd99e0)(self + 6);
    int32_t *shared = (int32_t *)(uintptr_t)self[2];
    if (--shared[0] == 0 && shared) {
        memory_free_aligned((void *)(uintptr_t)shared[3]);
        memory_free(shared);
    }
    self[0] = 0xf306b8;
}
REWRITE("Shared::Destroy87F8C0", 0x87f8c0, (w1c_seh<0xe639fe>), (&w1d_shared<0xf78f88>), &g_w1d_shared);
REWRITE("Shared::Destroy87F8C0", 0xb32f30, (w1c_seh<0xe9301e>), (&w1d_shared<0xf990b8>), &g_w1d_shared);

// ---- Maker::Make: when we are active, a new SIZE-byte object built for us from the two values; null otherwise --------
static volatile long g_w1d_makers;
template <uint32_t SIZE, uint32_t BUILD> static void *__thiscall w1d_maker(uint32_t *self, uint32_t a, uint32_t b) {
    g_w1d_makers++;
    if (!self[8]) return nullptr;
    void *made = memory_alloc(SIZE);
    return made ? ((void *(__thiscall *)(void *, void *, uint32_t, uint32_t))BUILD)(made, self, a, b) : nullptr;
}
REWRITE("Maker::MakeC59F00", 0xc59f00, (w1c_seh<0xea269b>), (&w1d_maker<0x18, 0xc59ab0>), &g_w1d_makers);
REWRITE("Maker::MakeC59F00", 0xca3440, (w1c_seh<0xea477b>), (&w1d_maker<0x14, 0xca2f10>), &g_w1d_makers);

// ---- Derived::Construct: the base built from four of the five values, our table, then the third value applied to the
// fourth --------------------------------------------------------------------------------------------------------------------
static volatile long g_w1d_derived;
template <uint32_t TABLE> static uint32_t *__thiscall w1d_derived(uint32_t *self, uint32_t a, uint32_t b, uint32_t c, void *d, uint32_t e) {
    g_w1d_derived++;
    ((void(__thiscall *)(void *, uint32_t, uint32_t, void *, uint32_t))0xca3750)(self, a, b, d, e);
    self[0] = TABLE;
    ((void(__thiscall *)(void *, uint32_t))0xcb8360)(d, c);
    return self;
}
REWRITE("Derived::ConstructCA3D30", 0xca3d30, (w1c_seh<0xea4818>), (&w1d_derived<0xfbf438>), &g_w1d_derived);
REWRITE("Derived::ConstructCA3D30", 0xca3e30, (w1c_seh<0xea4838>), (&w1d_derived<0xfbf460>), &g_w1d_derived);

// ---- Fresh::Make: a new SIZE-byte object built from the value (passed by address); null if out of memory ----------------
static volatile long g_w1d_fresh;
template <uint32_t SIZE, uint32_t BUILD> static void *__stdcall w1d_fresh(uint32_t value, uint32_t) {
    g_w1d_fresh++;
    void *made = memory_alloc(SIZE);
    return made ? ((void *(__thiscall *)(void *, uint32_t, uint32_t *))BUILD)(made, 0, &value) : nullptr;
}
REWRITE("Fresh::Make77A5B0", 0x77a5b0, (w1c_seh<0xe5194b>), (&w1d_fresh<0xf0, 0x7753d0>), &g_w1d_fresh);
REWRITE("Fresh::Make77A5B0", 0xa03b40, (w1c_seh<0xe7877b>), (&w1d_fresh<0x90, 0xa02e50>), &g_w1d_fresh);

// ---- Buffered::Destroy: the member at +0x10 destroyed, then the aligned buffer in SLOT freed and cleared ------------------
static volatile long g_w1d_buffered;
template <uint32_t END, uint32_t SLOT> static void __fastcall w1d_buffered(uint8_t *self) {
    g_w1d_buffered++;
    ((void(__fastcall *)(void *))END)(self + 0x10);
    void **buffer = (void **)(self + SLOT);
    if (*buffer) {
        memory_free_aligned(*buffer);
        *buffer = nullptr;
    }
}
REWRITE("Buffered::Destroy90B3B0", 0x90b3b0, (w1c_seh<0xe68fcb>), (&w1d_buffered<0xbd1880, 4>), &g_w1d_buffered);
REWRITE("Buffered::Destroy90B3B0", 0xaa0b80, (w1c_seh<0xe86b9b>), (&w1d_buffered<0x40f730, 8>), &g_w1d_buffered);

// ---- Pooled::Destroy: the pooled pointer dropped, the aligned buffer freed, then the base ---------------------------------
static volatile long g_w1d_pooled;
template <uint32_t TABLE, uint32_t HELD, uint32_t BUFFER, uint32_t BASE> static void __fastcall w1d_pooled(uint8_t *self) {
    g_w1d_pooled++;
    *(uint32_t *)self = TABLE;
    if (*(uint32_t *)(self + HELD)) *(uint32_t *)(self + HELD) = 0;
    memory_free_aligned(*(void **)(self + BUFFER));
    ((void(__fastcall *)(void *))BASE)(self);
}
REWRITE("Pooled::DestroyBEBCB0", 0xbebcb0, (w1c_seh<0xe9e598>), (&w1d_pooled<0xfa57b4, 0x20, 0x30, 0xbeba10>), &g_w1d_pooled);
REWRITE("Pooled::DestroyBEBCB0", 0xbff610, (w1c_seh<0xe9ea48>), (&w1d_pooled<0xfa5d0c, 0x24, 0x34, 0xbff310>), &g_w1d_pooled);

// ---- Paired::Destroy: both tables restored, the object kept at PART told to finish, then the base ---------------------
static volatile long g_w1d_paired;
template <uint32_t TABLE, uint32_t TABLE2, uint32_t PART, uint32_t FINISH, uint32_t BASE> static void __fastcall w1d_paired(uint32_t *self) {
    g_w1d_paired++;
    self[0] = TABLE;
    self[1] = TABLE2;
    ((void(__fastcall *)(void *))FINISH)(*(void **)((uint8_t *)self + PART));
    ((void(__fastcall *)(void *))BASE)(self);
}
REWRITE("Paired::Destroy8CDCD0", 0x8cdcd0, (w1c_seh<0xe66858>), (&w1d_paired<0xf7b468, 0xf7b454, 0xd8, 0x6b0990, 0x63a9c0>), &g_w1d_paired);
REWRITE("Paired::Destroy8CDCD0", 0xb19110, (w1c_seh<0xe91438>), (&w1d_paired<0xf98308, 0xf98304, 0x90, 0xb0e0d0, 0xb7a4c0>), &g_w1d_paired);

// ---- Built::Make: a new SIZE-byte object (null if out of memory) stored in `out`, then set up and started --------------
static volatile long g_w1d_built;
template <uint32_t SIZE, uint32_t BUILD> static void __fastcall w1d_built(void **out, uint32_t value, uint32_t extra) {
    g_w1d_built++;
    void *made = memory_alloc(SIZE);
    if (made) made = ((void *(__thiscall *)(void *, uint32_t, uint32_t))BUILD)(made, value, extra);
    *out = made;
    ((void(__fastcall *)(void *))0xc51900)(made);
    ((void(__fastcall *)(void *))0xc501a0)(*out);
}
REWRITE("Built::MakeC595A0", 0xc595a0, (w1c_seh<0xea259b>), (&w1d_built<0x154, 0xc58f40>), &g_w1d_built);
REWRITE("Built::MakeC595A0", 0xc59620, (w1c_seh<0xea25bb>), (&w1d_built<0x18c, 0xc59040>), &g_w1d_built);

// ---- Single::Get: the one instance, built on first use (its constructor records itself in the global) -----------------
static volatile long g_w1d_singles;
template <uint32_t GLOBAL, uint32_t SIZE, uint32_t BUILD> static uint32_t __cdecl w1d_single() {
    g_w1d_singles++;
    if (*(uint32_t *)GLOBAL) return *(uint32_t *)GLOBAL;
    if (void *made = memory_alloc(SIZE)) ((void(__fastcall *)(void *))BUILD)(made);
    return *(uint32_t *)GLOBAL;
}
REWRITE("Single::Get402F70", 0x402f70, (w1c_seh<0xe048ab>), (&w1d_single<0x12f84c8, 0x38, 0x699ea0>), &g_w1d_singles);
REWRITE("Single::Get402F70", 0x741e60, (w1c_seh<0xe4df0b>), (&w1d_single<0x13070cc, 0x6c, 0x7be2f0>), &g_w1d_singles);
REWRITE("Single::Get402F70", 0xa04bc0, (w1c_seh<0xe7897b>), (&w1d_single<0x1341dcc, 0x28, 0xa048f0>), &g_w1d_singles);

// ---- Started::Get: the one instance, built on first use and started unless its flag says it already is --------------
static volatile long g_w1d_started;
template <uint32_t GLOBAL, uint32_t SIZE, uint32_t BUILD> static uint8_t *__cdecl w1d_started_get() {
    g_w1d_started++;
    uint8_t **instance = (uint8_t **)GLOBAL;
    if (*instance) return *instance;
    uint8_t *made = (uint8_t *)memory_alloc(SIZE);
    *instance = made ? ((uint8_t * (__thiscall *)(void *, uint32_t, uint32_t, uint32_t))BUILD)(made, 0, 0, 0) : nullptr;
    if (!((*instance)[0x2c] & 1)) ((void(__fastcall *)(void *))0x800860)(*instance);
    return *instance;
}
REWRITE("Started::Get416F10", 0x416f10, (w1c_seh<0xe0633b>), (&w1d_started_get<0x1103c1c, 0x58, 0x430010>), &g_w1d_started);
REWRITE("Started::Get416F10", 0x87ecd0, (w1c_seh<0xe638fb>), (&w1d_started_get<0x130ac4c, 0x48, 0x80d930>), &g_w1d_started);

// ---- Parts::Drop: three owned parts of the model deleted and cleared, then the rest finished -----------------------------
static volatile long g_w1d_parts;
template <uint32_t FIRST> static void __thiscall w1d_parts(uint32_t *self, uint32_t) {
    g_w1d_parts++;
    for (uint32_t at = FIRST; at < FIRST + 12; at += 4) {
        void **part = (void **)((uint8_t *)(uintptr_t)self[7] + at);
        if (*part) delete_object(*part);
        *part = nullptr;
    }
    ((void(__fastcall *)(void *))0x6a1e20)(self);
}
static const uint8_t w1d_parts_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x1c, 0x8b, 0x88};
REWRITE("Parts::Drop598620", 0x598620, w1d_parts_original, (&w1d_parts<0x198>), &g_w1d_parts);
REWRITE("Parts::Drop598620", 0x630d80, w1d_parts_original, (&w1d_parts<0x190>), &g_w1d_parts);

// ---- Pairs::Resize: the old storage dropped and `count` new 8-byte items built ---------------------------------------
static volatile long g_w1d_pair_resizes;
template <uint32_t BUILD> static uint32_t __thiscall w1d_pair_resize(uint32_t *self, uint32_t count) {
    g_w1d_pair_resizes++;
    if (self[0]) {
        memory_free_aligned((void *)(uintptr_t)self[0]);
        self[0] = 0;
    }
    if (count) {
        void *items = memory_alloc_aligned(count * 8);
        if (items) ((void(__stdcall *)(void *, uint32_t, uint32_t, uint32_t))0x402b90)(items, 8, count, BUILD);
        self[0] = (uint32_t)(uintptr_t)items;
    }
    self[1] = count;
    return count;
}
REWRITE("Pairs::Resize733560", 0x733560, w1c_seh_late, (&w1d_pair_resize<0x7332e0>), &g_w1d_pair_resizes);
REWRITE("Pairs::Resize733560", 0x99bde0, w1c_seh_late, (&w1d_pair_resize<0x402ae0>), &g_w1d_pair_resizes);

// ---- Mode::Allows: whether `value` is allowed (no owner, or it is the owner) and our mode is one of the four; two of
// those modes first switch us on with CODE ----------------------------------------------------------------------------
static volatile long g_w1d_allows;
template <uint32_t ID, int32_t PLAIN1, int32_t PLAIN2, int32_t SWITCH1, int32_t SWITCH2, uint32_t CODE>
static bool __thiscall w1d_allows(uint8_t *self, uint32_t value) {
    g_w1d_allows++;
    void *owner_of = ref_interface<ID>(self + 0xd8);
    uint32_t owner = ((uint32_t(__thiscall *)(void *, void *))0x5d4e20)(owner_of, self);
    if (owner && owner != value) return false;
    int32_t mode = *(int32_t *)(self + 0x60);
    if (mode == PLAIN1 || mode == PLAIN2) return true;
    if (mode != SWITCH1 && mode != SWITCH2) return false;
    ((void(__thiscall *)(void *, uint32_t))(*(void ***)self)[13])(self, CODE);
    return true;
}
static const uint8_t w1d_allows_original[] = {0x56, 0x8b, 0xf1, 0x8a, 0x86, 0xf0, 0x00};
REWRITE("Mode::Allows563920", 0x563920, w1d_allows_original, (&w1d_allows<0x12f5638, 0x40, 0x23, 0x21, 0x24, 0x22>), &g_w1d_allows);
REWRITE("Mode::Allows563920", 0x563bb0, w1d_allows_original, (&w1d_allows<0x12f5638, 0x42, 0x3c, 0x3a, 0x3d, 0x3b>), &g_w1d_allows);

// ---- Placed::Matrix: the 64-byte transform of our placed part (through its cached interface) copied out; without a
// part, worked out the slow way --------------------------------------------------------------------------------------------
static volatile long g_w1d_matrices;
template <uint32_t PART> static uint32_t *__thiscall w1d_matrix(void *self, uint32_t *out, uint32_t extra) {
    g_w1d_matrices++;
    if (!((void *(__fastcall *)(void *))PART)(self)) {
        ((void(__thiscall *)(void *, uint32_t *, uint32_t))0x557ee0)(self, out, extra);
        return out;
    }
    uint8_t *part = (uint8_t *)((void *(__fastcall *)(void *))PART)(self);
    uint32_t *matrix = (uint32_t *)((uint8_t *)ref_interface<0x130b5c8>(part + 0x94) + 0xf0);
    for (int i = 0; i < 16; i++) out[i] = matrix[i];
    return out;
}
template <uint32_t AT, uint32_t TARGET> static const uint8_t w1d_matrix_original[] = {
    0x56, 0x57, 0x8b, 0xf1, 0xe8, (uint8_t)(TARGET - (AT + 9)), (uint8_t)((TARGET - (AT + 9)) >> 8),
    (uint8_t)((TARGET - (AT + 9)) >> 16), (uint8_t)((TARGET - (AT + 9)) >> 24)};
REWRITE("Placed::Matrix559C20", 0x559c20, (w1d_matrix_original<0x559c20, 0x558f20>), (&w1d_matrix<0x558f20>), &g_w1d_matrices);
REWRITE("Placed::Matrix559C20", 0x559d10, (w1d_matrix_original<0x559d10, 0x558fe0>), (&w1d_matrix<0x558fe0>), &g_w1d_matrices);

// ---- Ride::Pass: the event handed to our helper, then to the vehicle (mode 4) or, in modes 2 and 3, to the car found by
// the right lookup when that car is active; SLOT picks the handler ---------------------------------------------------------
static volatile long g_w1d_passes;
template <uint32_t HELPER, uint32_t SLOT> static void __thiscall w1d_pass(uint8_t *self, uint32_t a, uint32_t b) {
    g_w1d_passes++;
    ((void(__thiscall *)(void *, uint32_t, uint32_t))HELPER)(*(void **)(self + 0x148), a, b);
    auto hand = [&](uint8_t *to) {
        uint8_t *part = to + 0x40;
        ((void(__thiscall *)(void *, uint32_t, uint32_t))(*(void ***)part)[SLOT])(part, a, b);
    };
    int32_t mode = *(int32_t *)(self + 0x144);
    if (mode == 4 && *(uint8_t **)(self + 0x660)) return hand(*(uint8_t **)(self + 0x660));
    if (mode != 3 && mode != 2) return;
    void *cars = *(void **)(self + 0x5d4);
    uint8_t *car = (uint8_t *)((void *(__fastcall *)(void *))(self[0x668] ? 0x5d5030 : 0x5d4fd0))(cars);
    if (car && car[0xd4]) hand(car);
}
static const uint8_t w1d_pass_original[] = {0x53, 0x8b, 0x5c, 0x24, 0x08, 0x56, 0x57};
REWRITE("Ride::Pass4EE6B0", 0x4ee6b0, w1d_pass_original, (&w1d_pass<0x4bbd60, 1>), &g_w1d_passes);
REWRITE("Ride::Pass4EE6B0", 0x4ee760, w1d_pass_original, (&w1d_pass<0x4bbd80, 2>), &g_w1d_passes);
REWRITE("Ride::Pass4EE6B0", 0x4ee810, w1d_pass_original, (&w1d_pass<0x4bbda0, 3>), &g_w1d_passes);

// ---- Placed::Update: the handle's interface asked about the three values, then its 64-byte transform copied into us ----
static volatile long g_w1d_updates;
template <uint32_t ASK> static bool __thiscall w1d_update(uint8_t *self, uint32_t a, uint32_t b, uint32_t c) {
    g_w1d_updates++;
    uint8_t *handle = *(uint8_t **)(self + 0x58);
    bool answer = ((bool(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))ASK)(find_interface(handle, (const int32_t *)0x1309020), a, b, c);
    uint32_t *from = (uint32_t *)((uint8_t *)find_interface(handle, (const int32_t *)0x1309020) + 0x50);
    for (int i = 0; i < 16; i++) ((uint32_t *)(self + 0x70))[i] = from[i];
    return answer;
}
static const uint8_t w1d_update_original[] = {0x53, 0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x58, 0x57};
REWRITE("Placed::Update4D1E80", 0x4d1e80, w1d_update_original, (&w1d_update<0x7c2970>), &g_w1d_updates);
REWRITE("Placed::Update4D1E80", 0x4d1fa0, w1d_update_original, (&w1d_update<0x7c6280>), &g_w1d_updates);

// ---- Placed::Owner: through two cached interfaces (ours at REF, then the part's), the value the owner keeps at +0x84 -----
static volatile long g_w1d_owners;
template <uint32_t REF> static uint32_t __fastcall w1d_owner(uint8_t *self) {
    g_w1d_owners++;
    uint8_t *part = (uint8_t *)ref_interface<0x130bbec>(self + REF);
    uint8_t *owner = (uint8_t *)ref_interface<0x130b5c8>(part + 0x94);
    return *(uint32_t *)(owner + 0x84);
}
static const uint8_t w1d_owner_original[] = {0x56, 0x57, 0x8b, 0xf9, 0x8a, 0x87};
REWRITE("Placed::OwnerB87EB0", 0xb87eb0, w1d_owner_original, (&w1d_owner<0xa0>), &g_w1d_owners);
REWRITE("Placed::OwnerB87EB0", 0xb87f70, w1d_owner_original, (&w1d_owner<0xbc>), &g_w1d_owners);

// ---- Quad::Destroy: the four tables restored, the owned object deleted, then the members and bases in reverse ----------
static volatile long g_w1d_quads;
template <uint32_t T0, uint32_t T1, uint32_t T3, uint32_t T5> static void __fastcall w1d_quad(uint32_t *self) {
    g_w1d_quads++;
    self[0] = T0;
    self[1] = T1;
    self[3] = T3;
    self[5] = T5;
    if (self[9]) delete_object((void *)(uintptr_t)self[9]);
    self[0x18] = 0xf284e0;
    ((void(__fastcall *)(void *))0x40f730)(self + 0x10);
    ((void(__fastcall *)(void *))0x4b48a0)(self + 3);
    ((void(__fastcall *)(void *))0xcd2b50)(self + 1);
    self[0] = 0xf2694c;
}
REWRITE("Quad::Destroy4DEF00", 0x4def00, (w1c_seh<0xe159fc>), (&w1d_quad<0xf348b0, 0xf348a8, 0xf34828, 0xf34820>), &g_w1d_quads);
REWRITE("Quad::Destroy4DEF00", 0x7aefd0, (w1c_seh<0xe56adc>), (&w1d_quad<0xf71700, 0xf716f8, 0xf71678, 0xf71674>), &g_w1d_quads);
REWRITE("Quad::Destroy4DEF00", 0x8b7640, (w1c_seh<0xe65b1c>), (&w1d_quad<0xf7a938, 0xf7a930, 0xf7a8b0, 0xf7a8ac>), &g_w1d_quads);

// ---- Scene::Destroy: four tables restored, the owned object deleted, two members finished, then the base -------------
static volatile long g_w1d_scenes;
template <uint32_t T0, uint32_t T1, uint32_t T2, uint32_t T3> static void __fastcall w1d_scene(uint8_t *self) {
    g_w1d_scenes++;
    *(uint32_t *)self = T0;
    *(uint32_t *)(self + 4) = T1;
    *(uint32_t *)(self + 0xf8) = T2;
    *(uint32_t *)(self + 0x118) = T3;
    if (void *owned = *(void **)(self + 0x11c)) delete_object(owned);
    ((void(__fastcall *)(void *))0x6af760)(self + 0x120);
    ((void(__fastcall *)(void *))0x6b0150)(self + 0xf8);
    ((void(__fastcall *)(void *))0x6d4520)(self);
}
REWRITE("Scene::Destroy6D85A0", 0x6d85a0, (w1c_seh<0xe4362f>), (&w1d_scene<0xf647f8, 0xf647e0, 0xf647b4, 0xf647a8>), &g_w1d_scenes);
REWRITE("Scene::Destroy6D85A0", 0x6d8b60, (w1c_seh<0xe436ef>), (&w1d_scene<0xf64b50, 0xf64b38, 0xf64b0c, 0xf64b00>), &g_w1d_scenes);
REWRITE("Scene::Destroy6D85A0", 0x6d9480, (w1c_seh<0xe437bf>), (&w1d_scene<0xf64fd0, 0xf64fb8, 0xf64f8c, 0xf64f80>), &g_w1d_scenes);

// ---- Named::Destroy: the shared name let go (freed with its text at zero), two members finished, then the base -------
static volatile long g_w1d_named;
static void __fastcall w1d_named(uint8_t *self) {
    g_w1d_named++;
    int32_t *shared = *(int32_t **)(self + 0x398);
    if (--shared[0] == 0 && shared) {
        memory_free_aligned((void *)(uintptr_t)shared[3]);
        memory_free(shared);
    }
    ((void(__fastcall *)(void *))0x6a69d0)(self + 0x300);
    ((void(__fastcall *)(void *))0x8002c0)(self + 0x2e8);
    ((void(__fastcall *)(void *))0x68a3d0)(self);
}
REWRITE("Named::DestroyAD1220", 0xad1220, (w1c_seh<0xe8a2ef>), (&w1d_named), &g_w1d_named);
REWRITE("Named::DestroyAD1220", 0xaecaa0, (w1c_seh<0xe8d09f>), (&w1d_named), &g_w1d_named);

// ---- Labelled::Refresh: our current label fetched (virtual 19) and given to the scene object, which lets its old one go;
// then our reference to the fetched label is dropped ----------------------------------------------------------------------
static volatile long g_w1d_labels;
static inline void w1d_release_text(int32_t *text) {
    if (--text[0] == 0 && text) {
        memory_free_aligned((void *)(uintptr_t)text[3]);
        memory_free(text);
    }
}
static void __fastcall w1d_label(uint32_t *self) {
    g_w1d_labels++;
    if (!self[6]) return;
    int32_t *label;
    ((void(__thiscall *)(void *, int32_t **))(*(void ***)self)[19])(self, &label);
    uint8_t *scene = *(uint8_t **)((uint8_t *)(uintptr_t)self[6] + 0xe0);
    int32_t **held = (int32_t **)(scene + 0x114);
    if (label != *held) {
        w1d_release_text(*held);
        *held = label;
        ++label[0];
    }
    if (--label[0] == 0) {
        memory_free_aligned((void *)(uintptr_t)label[3]);
        memory_free(label);
    }
}
REWRITE("Labelled::Refresh503930", 0x503930, (w1c_seh<0xe19018>), (&w1d_label), &g_w1d_labels);
REWRITE("Labelled::Refresh503930", 0x5039f0, (w1c_seh<0xe19038>), (&w1d_label), &g_w1d_labels);

// ---- Records::Resize: `count` new 24-byte records built, as many old ones as fit copied over, the old storage freed ----
static volatile long g_w1d_record_resizes;
template <uint32_t BUILD> static int32_t __thiscall w1d_record_resize(uint32_t *self, int32_t count) {
    g_w1d_record_resizes++;
    if (count == (int32_t)self[1]) return count;
    uint32_t *items = nullptr;
    if (count) {
        items = (uint32_t *)memory_alloc_aligned(count * 24);
        if (items) ((void(__stdcall *)(void *, uint32_t, uint32_t, uint32_t))0x402b90)(items, 0x18, count, BUILD);
    }
    int32_t keep = count < (int32_t)self[1] ? count : (int32_t)self[1];
    uint32_t *old = (uint32_t *)(uintptr_t)self[0];
    for (int32_t i = 0; i < keep * 6; i++) items[i] = old[i];
    if (self[0]) {
        memory_free_aligned(old);
        self[0] = 0;
    }
    self[0] = (uint32_t)(uintptr_t)items;
    self[1] = count;
    return count;
}
REWRITE("Records::Resize80C8B0", 0x80c8b0, w1c_seh_late, (&w1d_record_resize<0x80c520>), &g_w1d_record_resizes);
REWRITE("Records::Resize80C8B0", 0x96f7f0, w1c_seh_late, (&w1d_record_resize<0x96edf0>), &g_w1d_record_resizes);

// Arrays made with new[]: the count is kept HEADER bytes before the items, which are built and destroyed by the runtime's
// vector iterators.
static void(__stdcall *const w1d_build_items)(void *, uint32_t, uint32_t, uint32_t, uint32_t) = (void(__stdcall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))0xddcf99;
static void(__stdcall *const w1d_destroy_items)(void *, uint32_t, uint32_t, uint32_t) = (void(__stdcall *)(void *, uint32_t, uint32_t, uint32_t))0xddcf39;
template <uint32_t ITEM, uint32_t HEADER, uint32_t BUILD, uint32_t END> static uint8_t *w1d_new_items(uint32_t count) {
    uint8_t *block = (uint8_t *)memory_alloc_aligned(count * ITEM + HEADER);
    if (!block) return nullptr;
    *(uint32_t *)block = count;
    w1d_build_items(block + HEADER, ITEM, count, BUILD, END);
    return block + HEADER;
}
template <uint32_t ITEM, uint32_t HEADER, uint32_t END> static void w1d_delete_items(uint32_t *self) {
    uint8_t *items = (uint8_t *)(uintptr_t)self[0];
    if (!items) return;
    w1d_destroy_items(items, ITEM, *(uint32_t *)(items - HEADER), END);
    memory_free_aligned(items - HEADER);
    self[0] = 0;
}

// ---- Slots::Resize: `count` new 8-byte slots, as many old ones as fit assigned over, the old array deleted -------------
static volatile long g_w1d_slot_resizes;
template <uint32_t END, uint32_t BUILD, uint32_t ASSIGN> static int32_t __thiscall w1d_slot_resize(uint32_t *self, int32_t count) {
    g_w1d_slot_resizes++;
    if (count == (int32_t)self[1]) return count;
    uint8_t *items = count ? w1d_new_items<8, 4, BUILD, END>(count) : nullptr;
    if (count) {
        int32_t keep = count < (int32_t)self[1] ? count : (int32_t)self[1];
        for (int32_t i = 0; i < keep; i++)
            ((void(__thiscall *)(void *, void *))ASSIGN)(items + i * 8, (uint8_t *)(uintptr_t)self[0] + i * 8);
    }
    w1d_delete_items<8, 4, END>(self);
    self[1] = count;
    self[0] = (uint32_t)(uintptr_t)items;
    return count;
}
REWRITE("Slots::Resize424540", 0x424540, w1c_seh_late, (&w1d_slot_resize<0x423d80, 0x423ce0, 0x423fb0>), &g_w1d_slot_resizes);
REWRITE("Slots::Resize424540", 0x464510, w1c_seh_late, (&w1d_slot_resize<0x43e700, 0x45e600, 0x461080>), &g_w1d_slot_resizes);
REWRITE("Slots::Resize424540", 0xa5d5e0, w1c_seh_late, (&w1d_slot_resize<0xa5bfe0, 0xa5bfd0, 0xa5b5e0>), &g_w1d_slot_resizes);

// ---- Blocks::Reset: the old array of 32-byte blocks deleted and `count` new ones made ----------------------------------
static volatile long g_w1d_block_resets;
template <uint32_t HEADER, uint32_t END, uint32_t BUILD> static uint32_t __thiscall w1d_block_reset(uint32_t *self, uint32_t count) {
    g_w1d_block_resets++;
    w1d_delete_items<0x20, HEADER, END>(self);
    if (count) self[0] = (uint32_t)(uintptr_t)w1d_new_items<0x20, HEADER, BUILD, END>(count);
    self[1] = count;
    return count;
}
REWRITE("Blocks::Reset462FC0", 0x462fc0, w1c_seh_late, (&w1d_block_reset<4, 0x41f860, 0x45f980>), &g_w1d_block_resets);
REWRITE("Blocks::Reset462FC0", 0x991f50, w1c_seh_late, (&w1d_block_reset<4, 0x438c00, 0x98ffa0>), &g_w1d_block_resets);
REWRITE("Blocks::Reset462FC0", 0x9e1ec0, w1c_seh_late, (&w1d_block_reset<0x10, 0x9e1310, 0x9e1670>), &g_w1d_block_resets);

// ---- Texts::Resize: `count` new shared texts, as many old ones as fit assigned over (the replaced one let go, the new
// one held), the old array deleted -------------------------------------------------------------------------------------------
static volatile long g_w1d_text_resizes;
template <uint32_t END, uint32_t BUILD> static int32_t __thiscall w1d_text_resize(uint32_t *self, int32_t count) {
    g_w1d_text_resizes++;
    if (count == (int32_t)self[1]) return count;
    int32_t **items = count ? (int32_t **)w1d_new_items<4, 4, BUILD, END>(count) : nullptr;
    if (count) {
        int32_t keep = count < (int32_t)self[1] ? count : (int32_t)self[1];
        for (int32_t i = 0; i < keep; i++) {
            int32_t *from = ((int32_t **)(uintptr_t)self[0])[i];
            if (from == items[i]) continue;
            w1d_release_text(items[i]);
            items[i] = from;
            ++from[0];
        }
    }
    w1d_delete_items<4, 4, END>(self);
    self[1] = count;
    self[0] = (uint32_t)(uintptr_t)items;
    return count;
}
REWRITE("Texts::Resize460F20", 0x460f20, (w1c_seh<0xe0ba1b>), (&w1d_text_resize<0x4012e0, 0x4027f0>), &g_w1d_text_resizes);
REWRITE("Texts::Resize460F20", 0xb11d40, (w1c_seh<0xe90a3b>), (&w1d_text_resize<0x403de0, 0x448170>), &g_w1d_text_resizes);

// ---- Plain::Resize: `count` new ITEM-byte records built, as many old ones as fit copied over, the old storage freed -----
static volatile long g_w1d_plain_resizes;
template <uint32_t ITEM, uint32_t BUILD> static int32_t __thiscall w1d_plain_resize(uint32_t *self, int32_t count) {
    g_w1d_plain_resizes++;
    if (count == (int32_t)self[1]) return count;
    uint32_t *items = nullptr;
    if (count) {
        items = (uint32_t *)memory_alloc_aligned(count * ITEM);
        if (items) ((void(__stdcall *)(void *, uint32_t, uint32_t, uint32_t))0x402b90)(items, ITEM, count, BUILD);
    }
    int32_t keep = count < (int32_t)self[1] ? count : (int32_t)self[1];
    uint32_t *old = (uint32_t *)(uintptr_t)self[0];
    for (int32_t i = 0; i < keep * (int32_t)(ITEM / 4); i++) items[i] = old[i];
    if (self[0]) {
        memory_free_aligned(old);
        self[0] = 0;
    }
    self[1] = count;
    self[0] = (uint32_t)(uintptr_t)items;
    return count;
}
REWRITE("Plain::Resize863A60", 0x863a60, w1c_seh_late, (&w1d_plain_resize<0x1c, 0x860f50>), &g_w1d_plain_resizes);
REWRITE("Plain::Resize863A60", 0x91a0b0, w1c_seh_late, (&w1d_plain_resize<0x2c, 0x9176a0>), &g_w1d_plain_resizes);
REWRITE("Plain::Resize863A60", 0x91c630, w1c_seh_late, (&w1d_plain_resize<0x1c, 0x91a270>), &g_w1d_plain_resizes);

// ---- Panel::Destroy: tables restored, the shared frame counter caught up once, two owned parts deleted, a member
// finished, then the base ------------------------------------------------------------------------------------------------------
static volatile long g_w1d_panels;
template <uint32_t T0, uint32_t T1, uint32_t T3, uint32_t T9, uint32_t FIRST, uint32_t SECOND, uint32_t MEMBER, uint32_t BASE>
static void __fastcall w1d_panel(uint32_t *self) {
    g_w1d_panels++;
    self[0] = T0;
    self[1] = T1;
    self[3] = T3;
    self[9] = T9;
    uint8_t *synced = (uint8_t *)self + 0x30;
    if (!*synced) {
        *(uint32_t *)0x10eeaac = *(uint32_t *)0x10eeab0;
        *synced = 1;
    }
    if (void *part = *(void **)((uint8_t *)self + FIRST)) delete_object(part);
    if (void *part = *(void **)((uint8_t *)self + SECOND)) delete_object(part);
    ((void(__fastcall *)(void *))0x63ab90)((uint8_t *)self + MEMBER);
    ((void(__fastcall *)(void *))BASE)(self);
}
REWRITE("Panel::Destroy632DA0", 0x632da0, (w1c_seh<0xe2fa36>), (&w1d_panel<0xf47ccc, 0xf47cc8, 0xf47cc0, 0xf47cb8, 0x1e0, 0x1e4, 0x190, 0x631ad0>), &g_w1d_panels);
REWRITE("Panel::Destroy632DA0", 0x632fc0, (w1c_seh<0xe2fa56>), (&w1d_panel<0xf47cec, 0xf47ce8, 0xf47ce0, 0xf47cd8, 0x2ec, 0x340, 0x2f0, 0x631ea0>), &g_w1d_panels);

// ---- Named::Construct: the base built with a fresh text of our class name, that text let go, then our table ----------
static volatile long g_w1d_named_makes;
template <uint32_t NAME, uint32_t MAKE_TEXT, uint32_t BASE, uint32_t TABLE> static uint32_t *__fastcall w1d_named_make(uint32_t *self) {
    g_w1d_named_makes++;
    void *memory = memory_alloc(0x10);
    int32_t *text = memory ? ((int32_t * (__thiscall *)(void *, uint32_t))MAKE_TEXT)(memory, NAME) : nullptr;
    ((void(__thiscall *)(void *, int32_t **))BASE)(self, &text);
    if (--text[0] == 0) {
        memory_free_aligned((void *)(uintptr_t)text[3]);
        memory_free(text);
    }
    self[0] = TABLE;
    return self;
}
REWRITE("Named::ConstructBE4940", 0xbe4940, (w1c_seh<0xe9e10b>), (&w1d_named_make<0xfa56a8, 0xbcfb10, 0xc183f0, 0xfa56a4>), &g_w1d_named_makes);
REWRITE("Named::ConstructBE4940", 0xc38520, (w1c_seh<0xea0dcb>), (&w1d_named_make<0xfb0adc, 0xbcf850, 0xc43450, 0xfb0ad0>), &g_w1d_named_makes);
