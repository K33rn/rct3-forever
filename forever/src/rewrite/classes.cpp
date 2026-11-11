// Constructors of the game's saved classes: the base (0x800A80) built for the class's description (the one given, or
// the class's own default), the members set up, and on the description's first use the class's saved fields registered:
// each a field record (made by its constructor with its name, type, flags and offset, then given its kind's table) added
// to the class's field list. Each class's own members and fields are lists read from its code.
#include "wave.h"

// A member set-up step: kind 0 stores value at +off, 1 stores a byte, 2 stores a pointer to +value of the object itself,
// 3 calls fn on the member at +off (thiscall) with up to four values, 5 stores the global at value as it is now, 6 gives
// the reference member at +off the empty handle (0x805750) and links it (+8) into that handle's list, 7 stores the shared
// empty wide string (0xBD0490), 8 stores the constructor's argument number value (1-4), 9 is kind 6 with the handle
// Handle::For (0x805700) gives for the object value (0 for none) instead of the empty handle, 10 stores at +off a new
// object: value bytes from 0xBCDFC0 constructed by fn (thiscall) with up to four values, or null when there was no memory,
// 11 calls fn (stdcall) with five values: the compiler's array constructor (0xDDCF99: array, size, count, ctor, dtor).
struct ClassOp {
    uint32_t kind, off, value, fn, count, args[5], globals;   // bit i of globals: args[i] is a global, read as the call is made
    uint32_t selfs;                                           // bit i: args[i] is an offset into the object (a pointer to that member)
};
// A saved field: a record of `size` bytes made by `ctor` with up to four values (name, type, flags, offset...), then
// given its kind's `table` and added to the class's field list (parent -1), or to the list (+4) of the struct field
// made by entry `parent` (the class's list when that one is null). An entry of size 0 closes the struct field made by
// entry `parent` instead: its slot 0 (which totals its fields).
struct ClassField {
    uint32_t size, ctor, table, count, args[4];
    int32_t parent;
};
// A name step: the description is given a text made from `name` and `value` (0 or a member's offset).
struct ClassName {
    uint32_t name, value;
};
// What a constructor does with the object at the end: links its member at +member into a game list (the list the global
// link points to, or the list at +link_at of what it points to), counts it in the global count, gives it to instance.
struct ClassTail {
    uintptr_t instance, link;
    int32_t link_at;
    uint32_t member;
    uintptr_t count;
};
// Links node (next, previous) into a circular list just after its head.
static void link_before(uint32_t *node, uint32_t *head) {
    node[0] = head[0];
    ((uint32_t *)(uintptr_t)head[0])[1] = (uint32_t)(uintptr_t)node;
    node[1] = (uint32_t)(uintptr_t)head;
    head[0] = (uint32_t)(uintptr_t)node;
}
static void *call_with(uintptr_t fn, void *self, uint32_t count, const uint32_t *args) {
    switch (count) {
    case 0: return ((void *(__thiscall *)(void *))fn)(self);
    case 1: return ((void *(__thiscall *)(void *, uint32_t))fn)(self, args[0]);
    case 2: return ((void *(__thiscall *)(void *, uint32_t, uint32_t))fn)(self, args[0], args[1]);
    case 3: return ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))fn)(self, args[0], args[1], args[2]);
    default: return ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))fn)(self, args[0], args[1], args[2], args[3]);
    }
}

// ---- Class::Construct(description, a, b) ------------------------------------------------------------------------------------
static volatile long g_class_constructs;
// given[1..7]: the constructor's arguments, the description first. The base gets the next two, or (PAIR) the two values
// the second one points to. TAIL: what it does with the object at the end, if anything.
template <uintptr_t DESCRIPTION, uintptr_t INDEX, const ClassOp *OPS, uint32_t OP_COUNT, const ClassField *FIELDS, uint32_t FIELD_COUNT, const ClassName *NAMES, uint32_t NAME_COUNT, bool PAIR, const ClassTail *TAIL>
static void *class_construct_body(uint8_t *self, const uint32_t *given) {
    g_class_constructs++;
    uint8_t *description = (uint8_t *)(uintptr_t)given[1];
    uint32_t a = PAIR ? ((const uint32_t *)(uintptr_t)given[2])[0] : given[2], b = PAIR ? ((const uint32_t *)(uintptr_t)given[2])[1] : given[3];
    ((void(__thiscall *)(void *, void *, uint32_t, uint32_t))0x800a80)(self, description ? description : *(uint8_t **)DESCRIPTION, a, b);
    for (uint32_t k = 0; k < OP_COUNT; k++) {
        const ClassOp &op = OPS[k];
        if (op.kind == 0) *(uint32_t *)(self + op.off) = op.value;
        else if (op.kind == 1) self[op.off] = (uint8_t)op.value;
        else if (op.kind == 2) *(uint8_t **)(self + op.off) = self + op.value;
        else if (op.kind == 5) *(uint32_t *)(self + op.off) = *(uint32_t *)(uintptr_t)op.value;
        else if (op.kind == 6 || op.kind == 9) {
            uint8_t *handle = op.kind == 6 ? ((uint8_t * (__cdecl *)())0x805750)() : ((uint8_t * (__fastcall *)(uintptr_t))0x805700)(op.value);
            *(uint8_t **)(self + op.off + 4) = handle;
            link_before((uint32_t *)(self + op.off + 8), *(uint32_t **)(handle + 0x20));
        } else if (op.kind == 7) *(void **)(self + op.off) = ((void *(__fastcall *)(uint32_t))0xbd0490)(1);
        else if (op.kind == 8) *(uint32_t *)(self + op.off) = given[op.value];
        else if (op.kind == 12) self[op.off] = (uint8_t)given[op.value];   // an argument's low byte
        else {
            uint32_t args[5];
            for (uint32_t i = 0; i < 5; i++)
                args[i] = op.globals >> i & 1 ? *(uint32_t *)(uintptr_t)op.args[i] : op.selfs >> i & 1 ? (uint32_t)(uintptr_t)(self + op.args[i]) : op.args[i];
            if (op.kind == 11)   // a stdcall with five values (the compiler's array constructor, 0xDDCF99)
                ((void(__stdcall *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))op.fn)(args[0], args[1], args[2], args[3], args[4]);
            else if (op.kind == 10) {   // new: value bytes, fn constructs them (nothing when there was no memory)
                void *memory = memory_alloc(op.value);
                *(void **)(self + op.off) = memory ? call_with(op.fn, memory, op.count, args) : nullptr;
            } else
                call_with(op.fn, self + op.off, op.count, args);
        }
    }
    uint8_t *used = description ? description : *(uint8_t **)DESCRIPTION;
    if (!used[4]) {
        for (uint32_t k = 0; k < NAME_COUNT; k++) {   // the description named (a text made from a name and a value, given with 0xBECC10, let go)
            alignas(16) uint8_t text[0x40];
            void *made = ((void *(__thiscall *)(void *, uintptr_t, uint32_t))0xbeca40)(text, NAMES[k].name, NAMES[k].value);
            ((void(__thiscall *)(void *, void *))0xbecc10)(used, made);
            ((void(__thiscall *)(void *))0xbec5e0)(text);
        }
        uint8_t *info = ((uint8_t * (__thiscall *)(void *))(*(void ***)used)[1])(used);
        int32_t index = *(int32_t *)INDEX;
        if (info && index < *(int32_t *)(info + 0x18)) {
            uint8_t *part = (*(uint8_t ***)(info + 0x14))[index];
            if (part) {
                uintptr_t fields_at = (uintptr_t)used + *(int32_t *)(part + 0x10);   // integers: the compiler would drop a pointer's null check
                uint8_t *fields_of = (uint8_t *)fields_at;
                if (fields_at) {
                    uint8_t *records[FIELD_COUNT];
                    for (uint32_t k = 0; k < FIELD_COUNT; k++) {
                        const ClassField &field = FIELDS[k];
                        records[k] = nullptr;
                        if (!field.size) {   // a struct field closed
                            uint8_t *record = records[field.parent];
                            ((void(__thiscall *)(void *))(*(void ***)record)[0])(record);
                            continue;
                        }
                        uint8_t *memory = (uint8_t *)memory_alloc(field.size);
                        if (memory) {
                            call_with(field.ctor, memory, field.count, field.args);
                            *(uint32_t *)memory = field.table;
                        }
                        records[k] = memory;
                        uint8_t *list = fields_of + 0x1c;
                        if (field.parent >= 0 && records[field.parent]) list = records[field.parent] + 4;
                        ((void(__thiscall *)(void *, void *))0x8087e0)(list, memory);
                    }
                }
            }
        }
    }
    if (!description) (*(uint8_t **)DESCRIPTION)[4] = 1;
    if (TAIL) {
        if (TAIL->link) {
            uint8_t *list = *(uint8_t **)TAIL->link;
            link_before((uint32_t *)(self + TAIL->member), TAIL->link_at < 0 ? (uint32_t *)list : *(uint32_t **)(list + TAIL->link_at));
        }
        if (TAIL->count) ++*(uint32_t *)TAIL->count;
        if (TAIL->instance) *(uint8_t **)TAIL->instance = self;
    }
    return self;
}
// The constructors take the description and one to six more arguments (thiscall: the callee pops them).
template <uintptr_t DESCRIPTION, uintptr_t INDEX, const ClassOp *OPS, uint32_t OP_COUNT, const ClassField *FIELDS, uint32_t FIELD_COUNT,
          const ClassName *NAMES = nullptr, uint32_t NAME_COUNT = 0, bool PAIR = false, const ClassTail *TAIL = nullptr>
static void *__thiscall class_construct2(uint8_t *self, uint32_t d, uint32_t a) {
    const uint32_t given[8] = {0, d, a};
    return class_construct_body<DESCRIPTION, INDEX, OPS, OP_COUNT, FIELDS, FIELD_COUNT, NAMES, NAME_COUNT, PAIR, TAIL>(self, given);
}
template <uintptr_t DESCRIPTION, uintptr_t INDEX, const ClassOp *OPS, uint32_t OP_COUNT, const ClassField *FIELDS, uint32_t FIELD_COUNT,
          const ClassName *NAMES = nullptr, uint32_t NAME_COUNT = 0, bool PAIR = false, const ClassTail *TAIL = nullptr>
static void *__thiscall class_construct(uint8_t *self, uint32_t d, uint32_t a, uint32_t b) {
    const uint32_t given[8] = {0, d, a, b};
    return class_construct_body<DESCRIPTION, INDEX, OPS, OP_COUNT, FIELDS, FIELD_COUNT, NAMES, NAME_COUNT, PAIR, TAIL>(self, given);
}
template <uintptr_t DESCRIPTION, uintptr_t INDEX, const ClassOp *OPS, uint32_t OP_COUNT, const ClassField *FIELDS, uint32_t FIELD_COUNT,
          const ClassName *NAMES = nullptr, uint32_t NAME_COUNT = 0, bool PAIR = false, const ClassTail *TAIL = nullptr>
static void *__thiscall class_construct4(uint8_t *self, uint32_t d, uint32_t a, uint32_t b, uint32_t c) {
    const uint32_t given[8] = {0, d, a, b, c};
    return class_construct_body<DESCRIPTION, INDEX, OPS, OP_COUNT, FIELDS, FIELD_COUNT, NAMES, NAME_COUNT, PAIR, TAIL>(self, given);
}
template <uintptr_t DESCRIPTION, uintptr_t INDEX, const ClassOp *OPS, uint32_t OP_COUNT, const ClassField *FIELDS, uint32_t FIELD_COUNT,
          const ClassName *NAMES = nullptr, uint32_t NAME_COUNT = 0, bool PAIR = false, const ClassTail *TAIL = nullptr>
static void *__thiscall class_construct5(uint8_t *self, uint32_t d, uint32_t a, uint32_t b, uint32_t c, uint32_t e) {
    const uint32_t given[8] = {0, d, a, b, c, e};
    return class_construct_body<DESCRIPTION, INDEX, OPS, OP_COUNT, FIELDS, FIELD_COUNT, NAMES, NAME_COUNT, PAIR, TAIL>(self, given);
}
template <uintptr_t DESCRIPTION, uintptr_t INDEX, const ClassOp *OPS, uint32_t OP_COUNT, const ClassField *FIELDS, uint32_t FIELD_COUNT,
          const ClassName *NAMES = nullptr, uint32_t NAME_COUNT = 0, bool PAIR = false, const ClassTail *TAIL = nullptr>
static void *__thiscall class_construct6(uint8_t *self, uint32_t d, uint32_t a, uint32_t b, uint32_t c, uint32_t e, uint32_t f) {
    const uint32_t given[8] = {0, d, a, b, c, e, f};
    return class_construct_body<DESCRIPTION, INDEX, OPS, OP_COUNT, FIELDS, FIELD_COUNT, NAMES, NAME_COUNT, PAIR, TAIL>(self, given);
}
template <uintptr_t DESCRIPTION, uintptr_t INDEX, const ClassOp *OPS, uint32_t OP_COUNT, const ClassField *FIELDS, uint32_t FIELD_COUNT,
          const ClassName *NAMES = nullptr, uint32_t NAME_COUNT = 0, bool PAIR = false, const ClassTail *TAIL = nullptr>
static void *__thiscall class_construct7(uint8_t *self, uint32_t d, uint32_t a, uint32_t b, uint32_t c, uint32_t e, uint32_t f, uint32_t g) {
    const uint32_t given[8] = {0, d, a, b, c, e, f, g};
    return class_construct_body<DESCRIPTION, INDEX, OPS, OP_COUNT, FIELDS, FIELD_COUNT, NAMES, NAME_COUNT, PAIR, TAIL>(self, given);
}
#include "class_list.inc"
