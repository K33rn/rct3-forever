// Constructors of the game's saved classes: the base (0x800A80) built for the class's description (the one given, or
// the class's own default), the members set up, and on the description's first use the class's saved fields registered:
// each a field record (made by its constructor with its name, type, flags and offset, then given its kind's table) added
// to the class's field list. Each class's own members and fields are lists read from its code.
#include "wave.h"

// A member set-up step: kind 0 stores value at +off, 1 stores a byte, 2 stores a pointer to +value of the object itself,
// 3 calls fn on the member at +off (thiscall) with up to four values, 5 stores the global at value as it is now, 6 gives
// the reference member at +off the empty handle (0x805750) and links it (+8) into that handle's list, 7 stores the shared
// empty wide string (0xBD0490), 8 stores the constructor's argument number value (1-4).
struct ClassOp {
    uint32_t kind, off, value, fn, count, args[4], globals;   // bit i of globals: args[i] is a global, read as the call is made
};
// A saved field: a record of `size` bytes made by `ctor` with up to four values (name, type, flags, offset...), then
// given its kind's `table`.
struct ClassField {
    uint32_t size, ctor, table, count, args[4];
};
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
template <uintptr_t DESCRIPTION, uintptr_t INDEX, const ClassOp *OPS, uint32_t OP_COUNT, const ClassField *FIELDS, uint32_t FIELD_COUNT, uintptr_t NAME>
static void *class_construct_body(uint8_t *self, uint8_t *description, uint32_t a, uint32_t b, uint32_t c) {
    g_class_constructs++;
    const uint32_t given[5] = {0, (uint32_t)(uintptr_t)description, a, b, c};
    ((void(__thiscall *)(void *, void *, uint32_t, uint32_t))0x800a80)(self, description ? description : *(uint8_t **)DESCRIPTION, a, b);
    for (uint32_t k = 0; k < OP_COUNT; k++) {
        const ClassOp &op = OPS[k];
        if (op.kind == 0) *(uint32_t *)(self + op.off) = op.value;
        else if (op.kind == 1) self[op.off] = (uint8_t)op.value;
        else if (op.kind == 2) *(uint8_t **)(self + op.off) = self + op.value;
        else if (op.kind == 5) *(uint32_t *)(self + op.off) = *(uint32_t *)(uintptr_t)op.value;
        else if (op.kind == 6) {
            uint8_t *handle = ((uint8_t * (__cdecl *)())0x805750)();
            *(uint8_t **)(self + op.off + 4) = handle;
            uint32_t *link = (uint32_t *)(self + op.off + 8), *head = *(uint32_t **)(handle + 0x20);
            link[0] = head[0];
            ((uint32_t *)(uintptr_t)head[0])[1] = (uint32_t)(uintptr_t)link;
            link[1] = (uint32_t)(uintptr_t)head;
            head[0] = (uint32_t)(uintptr_t)link;
        } else if (op.kind == 7) *(void **)(self + op.off) = ((void *(__fastcall *)(uint32_t))0xbd0490)(1);
        else if (op.kind == 8) *(uint32_t *)(self + op.off) = given[op.value];
        else {
            uint32_t args[4];
            for (uint32_t i = 0; i < 4; i++) args[i] = op.globals >> i & 1 ? *(uint32_t *)(uintptr_t)op.args[i] : op.args[i];
            call_with(op.fn, self + op.off, op.count, args);
        }
    }
    uint8_t *used = description ? description : *(uint8_t **)DESCRIPTION;
    if (!used[4]) {
        if (NAME) {   // the description named (a text made from NAME, given with 0xBECC10, let go)
            alignas(16) uint8_t text[0x40];
            void *made = ((void *(__thiscall *)(void *, uintptr_t, uint32_t))0xbeca40)(text, NAME, 0);
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
                if (fields_at)
                    for (uint32_t k = 0; k < FIELD_COUNT; k++) {
                        const ClassField &field = FIELDS[k];
                        void *memory = memory_alloc(field.size), *made = nullptr;
                        if (memory) {
                            made = call_with(field.ctor, memory, field.count, field.args);
                            *(uint32_t *)memory = field.table;
                            made = memory;
                        }
                        ((void(__thiscall *)(void *, void *))0x8087e0)(fields_of + 0x1c, made);
                    }
            }
        }
    }
    if (!description) (*(uint8_t **)DESCRIPTION)[4] = 1;
    return self;
}
// The constructors take (description, a, b) or (description, a, b, c).
template <uintptr_t DESCRIPTION, uintptr_t INDEX, const ClassOp *OPS, uint32_t OP_COUNT, const ClassField *FIELDS, uint32_t FIELD_COUNT, uintptr_t NAME = 0>
static void *__thiscall class_construct(uint8_t *self, uint8_t *description, uint32_t a, uint32_t b) {
    return class_construct_body<DESCRIPTION, INDEX, OPS, OP_COUNT, FIELDS, FIELD_COUNT, NAME>(self, description, a, b, 0);
}
template <uintptr_t DESCRIPTION, uintptr_t INDEX, const ClassOp *OPS, uint32_t OP_COUNT, const ClassField *FIELDS, uint32_t FIELD_COUNT, uintptr_t NAME = 0>
static void *__thiscall class_construct4(uint8_t *self, uint8_t *description, uint32_t a, uint32_t b, uint32_t c) {
    return class_construct_body<DESCRIPTION, INDEX, OPS, OP_COUNT, FIELDS, FIELD_COUNT, NAME>(self, description, a, b, c);
}
#include "class_list.inc"
