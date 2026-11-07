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
