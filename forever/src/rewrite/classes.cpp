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
// 11 calls fn (stdcall) with five values: the compiler's array constructor (0xDDCF99: array, size, count, ctor, dtor),
// 13 marks the end of the member array at +off ({items, count}: items[count] = value), 14 is where the registration and the
// mark happen (the steps after it run after them; without one, they come after every step), 15 is kind 10 for an
// object then given a table of its own (the last value): the member gets the memory, not what the constructor gave back,
// 16 is kind 9 for the object the constructor's argument number value gives, 17 runs the stretch written by hand number
// value (below), 18 copies the member at +value to +off, 19 is kind 9 for the object itself (+value), 20 stores at +off
// the resource the game's resource manager (0x13D0E04) gives for the name at value (fn: 0xBDF160 or 0xBDF1A0, given the
// place of a text made from the name, then let go), 21 sets the member's wide text at +off from the named text value
// of the texts table fn (see class_named_text). Kind 7 calls fn instead of 0xBD0490 when fn is set (0xBD0520, the
// other shared empty string).
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
// link points to, or the list at +link_at of what it points to), counts it in the global count, gives it to instance,
// then runs after (a few last lines written by hand, below), if any.
struct ClassTail {
    uintptr_t instance, link;
    int32_t link_at;
    uint32_t member;
    uintptr_t count;
    void (*after)(uint8_t *self);
};
// Links node (next, previous) into a circular list just after its head.
static void link_before(uint32_t *node, uint32_t *head) {
    node[0] = head[0];
    ((uint32_t *)(uintptr_t)head[0])[1] = (uint32_t)(uintptr_t)node;
    node[1] = (uint32_t)(uintptr_t)head;
    head[0] = (uint32_t)(uintptr_t)node;
}
static void class_hand(uint32_t number, uint8_t *self, const uint32_t *given);   // class_list.inc
// The resource the game's resource manager (0x13D0E04) gives for a name, stored into a member: a text made from the name
// (on 16 new bytes, or none without memory) is given by its place to lookup (0xBDF160 or 0xBDF1A0), and let go after.
static void class_resource(uint8_t *into, uintptr_t name, uintptr_t lookup) {
    void *memory = memory_alloc(0x10);
    int32_t *volatile text = memory ? ((int32_t * (__thiscall *)(void *, uintptr_t))0xbcfb10)(memory, name) : nullptr;
    *(volatile uint32_t *)into = ((uint32_t(__thiscall *)(void *, void *))lookup)(*(void *volatile *)0x13d0e04, (void *)&text);
    --*text;
    int32_t *again = text;   // read again, as the game does
    if (*(volatile int32_t *)again == 0) {
        ((void(__cdecl *)(void *))0xbce010)((void *)(uintptr_t)(uint32_t)again[3]);
        ((void(__cdecl *)(void *))0xbce000)(again);
    }
}
// A member's wide text set from a named text: the named text made in a temporary (0x4040F0: the name, the texts table,
// 1); the member's old text let go (its count taken down; at 0, and when there is one, its buffer freed, then it); the
// member given a new wide text copied from the named text's characters (+8; 0xBCF7D0 on 16 new bytes, none without
// memory), or the shared empty one (0xBD0490) when it has none; the temporary let go (0xBD1880).
static void class_named_text(uint8_t *self, uint32_t off, uintptr_t name, uintptr_t table) {
    typedef void(__cdecl * Free)(void *);
    alignas(16) uint8_t temporary[0x40];
    uint8_t *made = (uint8_t *)((void *(__thiscall *)(void *, uintptr_t, uintptr_t, uint32_t))0x4040f0)(temporary, name, table, 1);
    const void *characters = *(const void *volatile *)(made + 8);
    int32_t *volatile *member = (int32_t *volatile *)(self + off);
    --**member;
    uintptr_t old = (uintptr_t)*member;
    if (*(volatile int32_t *)old == 0 && old) {
        ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)((int32_t *)old)[3]);
        ((Free)0xbce000)((void *)old);
    }
    void *text;
    if (characters) {
        void *memory = memory_alloc(0x10);
        text = memory ? ((void *(__thiscall *)(void *, const void *))0xbcf7d0)(memory, characters) : nullptr;
    } else {
        text = ((void *(__fastcall *)(uint32_t))0xbd0490)(1);
    }
    *(void *volatile *)member = text;
    ((void(__thiscall *)(void *))0xbd1880)(temporary);
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
template <uintptr_t DESCRIPTION, uintptr_t INDEX, const ClassOp *OPS, uint32_t OP_COUNT, const ClassField *FIELDS, uint32_t FIELD_COUNT, const ClassName *NAMES, uint32_t NAME_COUNT, bool PAIR, const ClassTail *TAIL, uintptr_t BASE, uint32_t BASE_N, uint32_t BASE_X>
static void *class_construct_body(uint8_t *self, const uint32_t *given) {
    g_class_constructs++;
    uint8_t *description = (uint8_t *)(uintptr_t)given[1];
    uint32_t a = PAIR ? ((const uint32_t *)(uintptr_t)given[2])[0] : given[2], b = PAIR ? ((const uint32_t *)(uintptr_t)given[2])[1] : given[3];
    if (BASE_N == 4)   // (a parent class's constructor taking one more value, a constant)
        ((void(__thiscall *)(void *, void *, uint32_t, uint32_t, uint32_t))BASE)(self, description ? description : *(uint8_t **)DESCRIPTION, a, b, BASE_X >> 31 ? given[BASE_X & 0xff] : BASE_X);
    else
        ((void(__thiscall *)(void *, void *, uint32_t, uint32_t))BASE)(self, BASE_N & 0x100 || description ? description : *(uint8_t **)DESCRIPTION, a, b);   // (BASE_N bit 8: as given)   // (Class::Construct, or a parent class's constructor)
    uint32_t k = 0;
    for (int pass = 0; pass < 2; pass++) {
    for (; k < OP_COUNT; k++) {
        const ClassOp &op = OPS[k];
        if (op.kind == 14) break;
        if (op.kind == 0) *(uint32_t *)(self + op.off) = op.value;
        else if (op.kind == 1) self[op.off] = (uint8_t)op.value;
        else if (op.kind == 26) {   // a loop counted by a constant: the next value steps (plain stores), off times, fn bytes further on each pass
            for (uint32_t p = 0; p < op.off; p++)
                for (uint32_t j = 1; j <= op.value; j++) {
                    const ClassOp &b = OPS[k + j];
                    if (b.kind == 0) *(uint32_t *)(self + b.off + p * op.fn) = b.value;
                    else self[b.off + p * op.fn] = (uint8_t)b.value;
                }
            k += op.value;
        }
        else if (op.kind == 2) *(uint8_t **)(self + op.off) = self + op.value;
        else if (op.kind == 5) *(uint32_t *)(self + op.off) = *(uint32_t *)(uintptr_t)op.value;
        else if (op.kind == 6 || op.kind == 9 || op.kind == 16 || op.kind == 19) {
            uintptr_t of = op.kind == 16 ? given[op.value] : op.kind == 19 ? (uintptr_t)(self + op.value) : op.value;
            uint8_t *handle = op.kind == 6 ? ((uint8_t * (__cdecl *)())0x805750)() : ((uint8_t * (__fastcall *)(uintptr_t))0x805700)(of);
            *(uint8_t **)(self + op.off + 4) = handle;
            link_before((uint32_t *)(self + op.off + 8), *(uint32_t **)(handle + 0x20));
        } else if (op.kind == 7) *(void **)(self + op.off) = ((void *(__fastcall *)(uint32_t))(op.fn ? op.fn : 0xbd0490))(1);
        else if (op.kind == 8) *(uint32_t *)(self + op.off) = given[op.value];
        else if (op.kind == 12) self[op.off] = (uint8_t)given[op.value];   // an argument's low byte
        else if (op.kind == 33) self[op.off] = (uint8_t)given[op.value] != 0;   // whether it is not 0
        else if (op.kind == 38) *(uint32_t *)(self + op.off) = (uint8_t)given[op.value];   // or a dword given it, zero-extended
        else if (op.kind == 37) *(volatile uint32_t *)(self + op.off) |= op.value;   // a member with bits set
        else if (op.kind == 42) {   // the counted text argument value is, let go: its count down by one; at none, its characters and itself freed
            uint32_t *const text = (uint32_t *)(uintptr_t)given[op.value];
            const uint32_t count = *(volatile uint32_t *)text - 1;
            *(volatile uint32_t *)text = count;
            if (!count) {
                ((void(__cdecl *)(void *))0xbce010)((void *)(uintptr_t)text[3]);
                ((void(__cdecl *)(void *))0xbce000)(text);
            }
        } else if (op.kind == 41) ++*(volatile uint32_t *)(uintptr_t)given[op.value];   // the count of the counted text argument value is, raised
        else if (op.kind == 40) {   // the counted text argument value points to put in the member, its count raised
            uint32_t *const text = *(uint32_t *volatile *)(uintptr_t)given[op.value];
            *(uint32_t *volatile *)(self + op.off) = text;
            ++*(volatile uint32_t *)text;
        }
        else if (op.kind == 39) self[op.off] = *(volatile uint32_t *)(uintptr_t)op.value != 0;   // whether a global is not 0
        else if (op.kind == 13) (*(uint32_t **)(self + op.off))[*(uint32_t *)(self + op.off + 4)] = op.value;
        else if (op.kind == 18) *(uint32_t *)(self + op.off) = *(uint32_t *)(self + op.value);
        else if (op.kind == 17) class_hand(op.value, self, given);
        else if (op.kind == 20) class_resource(self + op.off, op.value, op.fn);
        else if (op.kind == 21) class_named_text(self, op.off, op.value, op.fn);
        else if (op.kind == 23) *(uint32_t *)(self + op.off) = ((uint32_t(__cdecl *)())(uintptr_t)op.fn)();   // what a function taking nothing gives
        else if (op.kind == 32)   // a constant into what the member at off points to, at fn
            *(volatile uint32_t *)(uintptr_t)(*(volatile uint32_t *)(self + op.off) + op.fn) = op.value;
        else if (op.kind == 36)   // what a fastcall gives for value in ecx (it never reads edx)
            *(volatile uint32_t *)(self + op.off) = ((uint32_t(__fastcall *)(uint32_t))(uintptr_t)op.fn)(op.value);
        else if (op.kind == 31)   // and its signed remainder by value (the sign of what it gave)
            *(int32_t *)(self + op.off) = (int32_t)((uint32_t(__cdecl *)())(uintptr_t)op.fn)() % (int32_t)op.value;
        else if (op.kind == 24) *(void **)(self + op.off) = ((void *(__cdecl *)(uint32_t))(uintptr_t)op.fn)(op.value);   // aligned memory
        else if (op.kind == 25) {   // a reference made on the stack (0xBD9AD0), given with the name value and a flag to fn on the member at off, let go (0xBD99E0)
            alignas(16) uint8_t reference[0x40];
            void *made = ((void *(__thiscall *)(void *))0xbd9ad0)(reference);
            if (op.count)
                ((void(__thiscall *)(void *, uint32_t, void *, uint32_t))(uintptr_t)op.fn)(self + op.off, op.value, made, op.args[0]);
            else   // (given only the name value and the reference)
                ((void(__thiscall *)(void *, uint32_t, void *))(uintptr_t)op.fn)(self + op.off, op.value, made);
            ((void(__thiscall *)(void *))0xbd99e0)(reference);
        } else if (op.kind == 22) {   // Random::Ran0's value (left on the FPU), times the game constant at value (if any), stored at off
            uintptr_t fn = op.fn;
            float *to = (float *)(self + op.off);
            const float *by = (const float *)(uintptr_t)op.value;
            if (by)
                __asm__ volatile("call *%%eax\n\tfmuls (%%ebx)\n\tfstps (%%esi)" : "+a"(fn) : "b"(by), "S"(to) : "ecx", "edx", "memory", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
            else
                __asm__ volatile("call *%%eax\n\tfstps (%%esi)" : "+a"(fn) : "S"(to) : "ecx", "edx", "memory", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
        }
        else if (op.kind == 27)   // a stdcall given two plain values whose float result (on the FPU) is stored at off
            *(float *)(self + op.off) = ((float(__stdcall *)(uint32_t, uint32_t))(uintptr_t)op.fn)(op.args[0], op.args[1]);
        else if (op.kind == 28) {   // a member's memory let go (fn, cdecl) if there is any, and the member cleared
            const uint32_t memory = *(volatile uint32_t *)(self + op.off);
            if (memory) {
                ((void(__cdecl *)(uint32_t))(uintptr_t)op.fn)(memory);
                *(volatile uint32_t *)(self + op.off) = 0;
            }
        } else if (op.kind == 30) {   // a game object made if its global (value) holds none: off bytes, fn run on them with args[0]
            if (!*(volatile uint32_t *)(uintptr_t)op.value) {
                void *memory = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(op.off);
                if (memory) ((void *(__thiscall *)(void *, uint32_t))(uintptr_t)op.fn)(memory, op.args[0]);
            }
        } else if (op.kind == 29)   // an array member {items, count} given value up to its count, both read again on each pass
            for (int32_t i = 0; i < *(volatile int32_t *)(self + op.off + 4); i++) (*(uint32_t *volatile *)(self + op.off))[i] = op.value;
        else {
            uint32_t args[5];
            for (uint32_t i = 0; i < 5; i++)
                args[i] = op.globals >> i & op.selfs >> i & 1 ?   // (both: a dword inside a global's object, {global | offset << 25})
                          *(volatile uint32_t *)(uintptr_t)(*(volatile uint32_t *)(uintptr_t)(op.args[i] & 0x1ffffff) + (op.args[i] >> 25)) :
                          op.globals >> i & 1 ? *(uint32_t *)(uintptr_t)op.args[i] : op.selfs >> i & 1 ? (op.args[i] >> 31 ? given[op.args[i] & 0xff] : (uint32_t)(uintptr_t)(self + op.args[i])) : op.args[i];   // (bit 31: an argument's value)
            if (op.kind == 11)   // a stdcall with five values (the compiler's array constructor, 0xDDCF99)
                ((void(__stdcall *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))op.fn)(args[0], args[1], args[2], args[3], args[4]);
            else if (op.kind == 10) {   // new: value bytes, fn constructs them (nothing when there was no memory)
                void *memory = memory_alloc(op.value);
                *(void **)(self + op.off) = memory ? call_with(op.fn, memory, op.count, args) : nullptr;
            } else if (op.kind == 15) {   // new, constructed, then given its own table
                void *memory = memory_alloc(op.value);
                if (memory) {
                    call_with(op.fn, memory, op.count - 1, args);
                    *(uint32_t *)memory = op.args[op.count - 1];
                }
                *(void **)(self + op.off) = memory;
            } else if (op.kind == 34)   // a member set to what fn gives, called on the object the global value holds
                *(void **)(self + op.off) = call_with(op.fn, *(void *volatile *)(uintptr_t)op.value, op.count, args);
            else
                call_with(op.fn, self + op.off, op.count, args);
        }
    }
    if (pass) break;
    k++;   // past the marker, if there is one
    uint8_t *used = description ? description : *(uint8_t **)DESCRIPTION;
    if (!used[4]) {
        for (uint32_t k = 0; k < NAME_COUNT; k++) {   // the description named (a text made from a name and a value, given with 0xBECC10, let go)
            alignas(16) uint8_t text[0x40];
            void *made = ((void *(__thiscall *)(void *, uintptr_t, uint32_t))0xbeca40)(text, NAMES[k].name, NAMES[k].value);
            ((void(__thiscall *)(void *, void *))0xbecc10)(used, made);
            ((void(__thiscall *)(void *))0xbec5e0)(text);
        }
        uint8_t *info = ((uint8_t * (__thiscall *)(void *))(*(void ***)used)[1])(used);
        int32_t index = FIELD_COUNT ? *(int32_t *)INDEX : 0;   // (a class registering no fields only casts: nothing else read)
        if (FIELD_COUNT && info && index < *(int32_t *)(info + 0x18)) {
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
                            if (field.table) *(uint32_t *)memory = field.table;   // (none when the record's constructor sets its own)
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
    }
    if constexpr (TAIL != nullptr) {
        if (TAIL->link) {   // (bit 0: linked in at the list's end, just before its head)
            uint8_t *list = *(uint8_t **)(TAIL->link & ~(uintptr_t)1);
            uint32_t *const node = (uint32_t *)(self + TAIL->member), *const head = TAIL->link_at < 0 ? (uint32_t *)list : *(uint32_t **)(list + TAIL->link_at);
            if (TAIL->link & 1) {
                node[1] = head[1];
                ((uint32_t *)(uintptr_t)head[1])[0] = (uint32_t)(uintptr_t)node;
                node[0] = (uint32_t)(uintptr_t)head;
                head[1] = (uint32_t)(uintptr_t)node;
            } else
                link_before(node, head);
        }
        if (TAIL->count) ++*(uint32_t *)TAIL->count;
        if (TAIL->instance) {   // (bit 0: the object there before, if any, deleted first: its table's slot 0, given 1)
            uint8_t *volatile *const at = (uint8_t * volatile *)(TAIL->instance & ~(uintptr_t)1);
            if (TAIL->instance & 1)
                if (void *before = *at) ((void(__thiscall *)(void *, uint32_t))(**(void ***)before))(before, 1);
            *at = self;
        }
        if (TAIL->after) TAIL->after(self);
    }
    return self;
}
// The constructors take the description and one to six more arguments (thiscall: the callee pops them).
template <uintptr_t DESCRIPTION, uintptr_t INDEX, const ClassOp *OPS, uint32_t OP_COUNT, const ClassField *FIELDS, uint32_t FIELD_COUNT,
          const ClassName *NAMES = nullptr, uint32_t NAME_COUNT = 0, bool PAIR = false, const ClassTail *TAIL = nullptr, uintptr_t BASE = 0x800a80, uint32_t BASE_N = 3, uint32_t BASE_X = 0>
static void *__thiscall class_construct2(uint8_t *self, uint32_t d, uint32_t a) {
    const uint32_t given[8] = {0, d, a};
    return class_construct_body<DESCRIPTION, INDEX, OPS, OP_COUNT, FIELDS, FIELD_COUNT, NAMES, NAME_COUNT, PAIR, TAIL, BASE, BASE_N, BASE_X>(self, given);
}
template <uintptr_t DESCRIPTION, uintptr_t INDEX, const ClassOp *OPS, uint32_t OP_COUNT, const ClassField *FIELDS, uint32_t FIELD_COUNT,
          const ClassName *NAMES = nullptr, uint32_t NAME_COUNT = 0, bool PAIR = false, const ClassTail *TAIL = nullptr, uintptr_t BASE = 0x800a80, uint32_t BASE_N = 3, uint32_t BASE_X = 0>
static void *__thiscall class_construct(uint8_t *self, uint32_t d, uint32_t a, uint32_t b) {
    const uint32_t given[8] = {0, d, a, b};
    return class_construct_body<DESCRIPTION, INDEX, OPS, OP_COUNT, FIELDS, FIELD_COUNT, NAMES, NAME_COUNT, PAIR, TAIL, BASE, BASE_N, BASE_X>(self, given);
}
template <uintptr_t DESCRIPTION, uintptr_t INDEX, const ClassOp *OPS, uint32_t OP_COUNT, const ClassField *FIELDS, uint32_t FIELD_COUNT,
          const ClassName *NAMES = nullptr, uint32_t NAME_COUNT = 0, bool PAIR = false, const ClassTail *TAIL = nullptr, uintptr_t BASE = 0x800a80, uint32_t BASE_N = 3, uint32_t BASE_X = 0>
static void *__thiscall class_construct4(uint8_t *self, uint32_t d, uint32_t a, uint32_t b, uint32_t c) {
    const uint32_t given[8] = {0, d, a, b, c};
    return class_construct_body<DESCRIPTION, INDEX, OPS, OP_COUNT, FIELDS, FIELD_COUNT, NAMES, NAME_COUNT, PAIR, TAIL, BASE, BASE_N, BASE_X>(self, given);
}
template <uintptr_t DESCRIPTION, uintptr_t INDEX, const ClassOp *OPS, uint32_t OP_COUNT, const ClassField *FIELDS, uint32_t FIELD_COUNT,
          const ClassName *NAMES = nullptr, uint32_t NAME_COUNT = 0, bool PAIR = false, const ClassTail *TAIL = nullptr, uintptr_t BASE = 0x800a80, uint32_t BASE_N = 3, uint32_t BASE_X = 0>
static void *__thiscall class_construct5(uint8_t *self, uint32_t d, uint32_t a, uint32_t b, uint32_t c, uint32_t e) {
    const uint32_t given[8] = {0, d, a, b, c, e};
    return class_construct_body<DESCRIPTION, INDEX, OPS, OP_COUNT, FIELDS, FIELD_COUNT, NAMES, NAME_COUNT, PAIR, TAIL, BASE, BASE_N, BASE_X>(self, given);
}
template <uintptr_t DESCRIPTION, uintptr_t INDEX, const ClassOp *OPS, uint32_t OP_COUNT, const ClassField *FIELDS, uint32_t FIELD_COUNT,
          const ClassName *NAMES = nullptr, uint32_t NAME_COUNT = 0, bool PAIR = false, const ClassTail *TAIL = nullptr, uintptr_t BASE = 0x800a80, uint32_t BASE_N = 3, uint32_t BASE_X = 0>
static void *__thiscall class_construct6(uint8_t *self, uint32_t d, uint32_t a, uint32_t b, uint32_t c, uint32_t e, uint32_t f) {
    const uint32_t given[8] = {0, d, a, b, c, e, f};
    return class_construct_body<DESCRIPTION, INDEX, OPS, OP_COUNT, FIELDS, FIELD_COUNT, NAMES, NAME_COUNT, PAIR, TAIL, BASE, BASE_N, BASE_X>(self, given);
}
template <uintptr_t DESCRIPTION, uintptr_t INDEX, const ClassOp *OPS, uint32_t OP_COUNT, const ClassField *FIELDS, uint32_t FIELD_COUNT,
          const ClassName *NAMES = nullptr, uint32_t NAME_COUNT = 0, bool PAIR = false, const ClassTail *TAIL = nullptr, uintptr_t BASE = 0x800a80, uint32_t BASE_N = 3, uint32_t BASE_X = 0>
static void *__thiscall class_construct7(uint8_t *self, uint32_t d, uint32_t a, uint32_t b, uint32_t c, uint32_t e, uint32_t f, uint32_t g) {
    const uint32_t given[8] = {0, d, a, b, c, e, f, g};
    return class_construct_body<DESCRIPTION, INDEX, OPS, OP_COUNT, FIELDS, FIELD_COUNT, NAMES, NAME_COUNT, PAIR, TAIL, BASE, BASE_N, BASE_X>(self, given);
}
// ---- the last lines of a few constructors, by hand ---------------------------------------------------------------------
// Person::Person (0x7DF320): the person's firework entry found in the shared lighting object's list (0x425920 makes the
// object on first use), a flag set, its info text made, its state bits put to their start (bits cleared, others set),
// two values cleared, two bytes put to 0xFF and twelve counters to 1.
static void person_after(uint8_t *self) {
    uint8_t *lighting = ((uint8_t * (__cdecl *)())0x425920)();
    void *list = *(void **)(lighting + 0x78);
    uint32_t found = ((uint32_t(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))0x9f9da0)(list, 0, 4, 3, 1, 0);
    *(volatile uint32_t *)(self + 0xc8) = found;
    self[0x2b0] = 1;
    ((void(__thiscall *)(void *))0x7c8870)(self);
    *(volatile uint32_t *)(self + 0x378) = (*(volatile uint32_t *)(self + 0x378) & 0xfffec688) | 0x2c688;
    *(volatile uint32_t *)(self + 0x3c8) = 0;
    *(volatile uint32_t *)(self + 0x3cc) = 0;
    self[0xc3] = 0xff;
    self[0xc4] = 0xff;
    for (uint32_t k = 0; k < 12; k++) *(volatile uint32_t *)(self + 0x240 + 4 * k) = 1;
}

// RideTrainInstance::RideTrainInstance (0x56BF80), in its member set-up: its status members cleared, and its status text
// (+0x294) made from the empty name and an empty resource reference made for it on the stack (then let go).
static void ride_train_status_text(uint8_t *self, const uint32_t *) {
    for (uint32_t at = 0x258; at <= 0x274; at += 4) *(volatile uint32_t *)(self + at) = 0;
    self[0x278] = 0;
    *(volatile uint32_t *)(self + 0x280) = 0xffffffff;
    self[0x284] = 0;
    *(volatile uint32_t *)(self + 0x288) = 0;
    *(volatile uint32_t *)(self + 0x28c) = 0;
    *(volatile uint32_t *)(self + 0x290) = 0;
    alignas(16) uint8_t reference[0x40];
    void *made = ((void *(__thiscall *)(void *))0xbd9ad0)(reference);
    ((void *(__thiscall *)(void *, uintptr_t, void *, uint32_t))0x560d20)(self + 0x294, 0xf264ad, made, 1);
    ((void(__thiscall *)(void *))0xbd99e0)(reference);
}

// Weather::Weather (0x9E8FF0), in its member set-up: its first wind value the base value (0x10F1CF8) plus the one for its
// kind (+0x58, through the table of pointers at 0x10F1CD0), added on the FPU and stored as a float; three more values
// from the game's defaults, one cleared, and then the weather set going (0x9E87E0 with 0).
static void weather_start(uint8_t *self, const uint32_t *) {
    const float *by_kind = *(const float *volatile *)(0x10f1cd0 + 4 * *(volatile int32_t *)(self + 0x58));
    long double sum = (long double)*(volatile float *)0x10f1cf8 + (long double)*(volatile const float *)by_kind;
    *(volatile float *)(self + 0x5c) = (float)sum;
    *(volatile uint32_t *)(self + 0x60) = *(volatile uint32_t *)0x10f1d00;
    *(volatile uint32_t *)(self + 0x64) = *(volatile uint32_t *)0x10f1cfc;
    *(volatile uint32_t *)(self + 0x68) = *(volatile uint32_t *)0x10f1d04;
    *(volatile uint32_t *)(self + 0x6c) = 0;
    ((void(__thiscall *)(void *, uint32_t))0x9e87e0)(self, 0);
}
// 0xBA9840, in its member set-up: the member at +0x40 given (0x4DE7D0) the value 0x4C3610 gives for kind 2 (a game
// constant, left on the FPU and stored as a float) and 10.0.
static void member_float_ba9840(uint8_t *self, const uint32_t *) {
    const long double value = ((long double(__fastcall *)(uint32_t))0x4c3610)(2);
    ((void(__thiscall *)(void *, float, uint32_t))0x4de7d0)(self + 0x40, (float)value, 0x41200000);
}
// 0xA0A650, in its member set-up: the member at +0x48 made (0x48C9C0) for the object with a pair {4, 0} made on the stack.
static void member_from_pair_a0a650(uint8_t *self, const uint32_t *) {
    uint32_t pair[4];
    pair[1] = 0, pair[0] = 4;
    ((void *(__thiscall *)(void *, void *, void *))0x48c9c0)(self + 0x48, self, pair);
}
// 0x9288F0, in its member set-up: parts made in place on 0x40 new bytes each (none without memory): their first three
// dwords cleared, 1.0 at +0x10, +0x14 and +0x18 cleared, the member at +0x20 given 0x3FFFFFFF (0x435A40), 10000.0 at
// +0x30 and +0x34 (+0xC and the rest left as they were); seven into +0x334..+0x34C, then two into +0x3C0 and +0x3C4.
static void *part_9288f0() {
    uint32_t *const part = (uint32_t *)memory_alloc(0x40);
    if (!part) return nullptr;
    part[0] = 0, part[1] = 0, part[2] = 0;
    part[4] = 0x3f800000, part[5] = 0, part[6] = 0;
    ((void(__thiscall *)(void *, uint32_t))0x435a40)(part + 8, 0x3fffffff);
    part[12] = 0x461c4000, part[13] = 0x461c4000;
    return part;
}
static void parts_9288f0(uint8_t *self, const uint32_t *) {
    for (uint32_t at = 0x334; at <= 0x34c; at += 4) *(void *volatile *)(self + at) = part_9288f0();
}
static void parts_9288f0b(uint8_t *self, const uint32_t *) {
    for (uint32_t at = 0x3c0; at <= 0x3c4; at += 4) *(void *volatile *)(self + at) = part_9288f0();
}
// 0x4A6300, in its member set-up: two texts of 0xF264AD made (16 new bytes each, none without memory), the second put in
// +0xB4 and the first in +0xB8, each count raised; then each let go once (the second first). (A text made without memory
// is used all the same, as the game does.)
static uint32_t *text_of_4a6300() {
    void *const memory = memory_alloc(0x10);
    return memory ? (uint32_t *)((void *(__thiscall *)(void *, uintptr_t))0xbcf850)(memory, 0xf264ad) : nullptr;
}
static void text_let_go_4a6300(uint32_t *text) {
    if (!--*(volatile uint32_t *)text) {
        ((void(__cdecl *)(void *))0xbce010)((void *)(uintptr_t)text[3]);
        ((void(__cdecl *)(void *))0xbce000)(text);
    }
}
static void texts_4a6300(uint8_t *self, const uint32_t *) {
    uint32_t *const first = text_of_4a6300();
    uint32_t *const second = text_of_4a6300();
    *(uint32_t *volatile *)(self + 0xb4) = second;
    ++*(volatile uint32_t *)second;
    *(uint32_t *volatile *)(self + 0xb8) = first;
    ++*(volatile uint32_t *)first;
    text_let_go_4a6300(second);
    text_let_go_4a6300(first);
}
// 0x830740, in its member set-up: the member at +0x40 made for the object with a pair of zeros made on the stack.
static void member_from_pair_830740(uint8_t *self, const uint32_t *) {
    uint32_t pair[4];
    pair[0] = 0, pair[1] = 0;
    ((void *(__thiscall *)(void *, void *, void *))0x48c9c0)(self + 0x40, self, pair);
}
// Park::Park (0x707680), in its member set-up: its figures worked out from two of its values (0x704A50), then 0x707530
// on it.
static void park_figures(uint8_t *self, const uint32_t *) {
    ((void(__thiscall *)(void *))0x704a50)(self);
    ((void(__thiscall *)(void *))0x707530)(self);
}
// Sky::Sky (0x9E51D0), in its member set-up: its colour values (twelve cleared, four set to 1.0), then its colour
// window record made with new (0x24 bytes, by 0x6A3B60 from a pair of zeros, a pair of 1.0s, both on the stack, and -1),
// or none without memory.
static void sky_colours(uint8_t *self, const uint32_t *) {
    static const uint16_t cleared[] = {0x3d8, 0x3d4, 0x3d0, 0x3cc, 0x3c4, 0x3c0, 0x3bc, 0x3b8, 0x3b0, 0x3ac, 0x3a8, 0x3a4};
    for (uint16_t at : cleared) *(volatile uint32_t *)(self + at) = 0;
    static const uint16_t ones[] = {0x3dc, 0x3c8, 0x3b4, 0x3a0};
    for (uint16_t at : ones) *(volatile uint32_t *)(self + at) = 0x3f800000;   // 1.0f
    void *memory = memory_alloc(0x24);
    void *made = nullptr;
    if (memory) {
        uint32_t zeros[2] = {0, 0}, ones[2] = {0x3f800000, 0x3f800000};
        made = ((void *(__thiscall *)(void *, const uint32_t *, const uint32_t *, int32_t))0x6a3b60)(memory, zeros, ones, -1);
    }
    *(void *volatile *)(self + 0x3e0) = made;
}
// WaterManager::WaterManager (0x947720), in its member set-up: the water's texts table made if there is none yet
// (0x940EA0), then the manager's name (+0x84) found in it.
static void water_manager_name(uint8_t *self, const uint32_t *) {
    if (!*(volatile uint32_t *)0x133a274) ((void(__cdecl *)())0x940ea0)();
    ((void *(__thiscall *)(void *, uintptr_t, uint32_t, uint32_t))0x403f30)(self + 0x84, 0xf7e3b8, *(volatile uint32_t *)0x133a274, 1);
}
// WaterManager::WaterManager (0x947720), its end: the water's shared step (0x9402F0 on 0x133A2BC); the manager kept in
// its global (0x133A270); its three tables of named textures filled (each made to hold a number of entries by 0x942510,
// then entry i given the text found for the name made from a format and i: "...0%d" for the first ten, "...%d" after),
// four resources looked up by name, and two more named texts.
static uint32_t water_table() {
    if (!*(volatile uint32_t *)0x133a274) ((void(__cdecl *)())0x940ea0)();
    return *(volatile uint32_t *)0x133a274;
}
static void water_entry(uint8_t *list, uint32_t i, uint32_t at, uintptr_t format, uintptr_t format_low) {
    char name[0x80];
    ((int(__cdecl *)(char *, const char *, ...))0xddcdb5)(name, (const char *)(at > 0xfc ? format : format_low), i);
    uint32_t table = water_table();
    alignas(16) uint8_t text[0x40];
    ((void *(__thiscall *)(void *, const char *, uint32_t, uint32_t))0x403f30)(text, name, table, 1);
    ((void(__thiscall *)(void *, void *))0xbd14e0)(*(uint8_t *volatile *)list + at, text);
    ((void(__thiscall *)(void *))0xbd1880)(text);
}
static void water_manager_after(uint8_t *self) {
    ((void(__thiscall *)(void *))0x9402f0)((void *)0x133a2bc);
    *(uint8_t *volatile *)0x133a270 = self;
    ((void(__thiscall *)(void *, uint32_t))0x942510)(self + 0xc8, 0x20);
    for (uint32_t i = 0, at = 0; at < 0x380; i++, at += 0x1c) water_entry(self + 0xc8, i, at, 0xf7e340, 0xf7e330);   // SimpleWater
    class_resource(self + 0xd8, 0xf7e31c, 0xbdf1a0);
    ((void(__thiscall *)(void *, uint32_t))0x942510)(self + 0xf4, 0x20);
    for (int32_t i = 0, at = 0; i < *(volatile int32_t *)(self + 0xf8); i++, at += 0x1c) water_entry(self + 0xf4, i, at, 0xf7e310, 0xf7e304);   // Caustics
    class_resource(self + 0x104, 0xf7e2f0, 0xbdf1a0);
    class_resource(self + 0x108, 0xf7e2e0, 0xbdf1a0);
    class_resource(self + 0x10c, 0xf7e2d0, 0xbdf1a0);
    alignas(16) uint8_t first[0x40], second[0x40];
    ((void *(__thiscall *)(void *, uintptr_t, uint32_t, uint32_t))0x403f30)(first, 0xf7aa64, water_table(), 1);
    ((void(__thiscall *)(void *, void *))0xbd14e0)(self + 0x118, first);
    ((void *(__thiscall *)(void *, uintptr_t, uint32_t, uint32_t))0x403f30)(second, 0xf7ded0, water_table(), 1);
    ((void(__thiscall *)(void *, void *))0xbd14e0)(self + 0x134, second);
    ((void(__thiscall *)(void *, uint32_t))0x942510)(self + 0x110, 8);
    for (int32_t i = 0, at = 0; i < *(volatile int32_t *)(self + 0x114); i++, at += 0x1c) water_entry(self + 0x110, i, at, 0xf7e2c0, 0xf7e2b0);   // WaterNormal
    ((void(__thiscall *)(void *))0xbd1880)(second);
    ((void(__thiscall *)(void *))0xbd1880)(first);
}
// 0xB4B7A0, its end: its list member (+0x148) linked in last in the game's list the global 0x10F28E8 points to (just
// before the list's head).
static void link_last_b4b7a0(uint8_t *self) {
    uint32_t *head = *(uint32_t *volatile *)0x10f28e8, *node = (uint32_t *)(self + 0x148);
    uint32_t last = head[1];
    node[1] = last;
    *(uint32_t **)(uintptr_t)head[1] = node;
    node[0] = (uint32_t)(uintptr_t)head;
    head[1] = (uint32_t)(uintptr_t)node;
}
// 0x76B880, its end: values drawn from Random::Ran0 (0x475180, a value from 0 to 1 left on the FPU), worked with the
// game's constants (read where the game keeps them; on the FPU, in its precision):
//   +0x408 = Ran0 * 0.3 + 0.35, with -1.0 at +0x40C and +0x414, 0 at +0x410; then +0x418..+0x424 = {+0x408, -1, 0, -1}
//   and -1.0 at +0x428, +0x42C, +0x430;
//   +0x4B0 = Ran0 * 0.2 + 0.9; if a new Ran0 is below 0.4, Ran0 * 0.6 is added to it; if another is below 0.05, a Ran0 is added too
//   (a draw that is not a number is never below: the game's test passes over it then too).
static long double ran0() { return ((long double(__cdecl *)())0x475180)(); }
static long double game_float(uintptr_t at) { return (long double)*(volatile const float *)at; }
static void random_values_76b880(uint8_t *self) {
    const uint32_t minus_one = 0xbf800000;
    long double x = ran0() * game_float(0xf27ca8);
    *(volatile uint32_t *)(self + 0x40c) = minus_one;
    x = x + game_float(0xf3d890);
    *(volatile uint32_t *)(self + 0x410) = 0;
    *(volatile uint32_t *)(self + 0x414) = minus_one;
    *(volatile float *)(self + 0x408) = (float)x;
    *(volatile uint32_t *)(self + 0x418) = *(volatile uint32_t *)(self + 0x408);
    *(volatile uint32_t *)(self + 0x41c) = minus_one;
    *(volatile uint32_t *)(self + 0x420) = 0;
    *(volatile uint32_t *)(self + 0x424) = minus_one;
    *(volatile uint32_t *)(self + 0x428) = minus_one;
    *(volatile uint32_t *)(self + 0x42c) = minus_one;
    *(volatile uint32_t *)(self + 0x430) = minus_one;
    *(volatile float *)(self + 0x4b0) = (float)(ran0() * game_float(0xf283d8) + game_float(0xf29640));
    if (ran0() < game_float(0xf355e4))
        *(volatile float *)(self + 0x4b0) = (float)(ran0() * game_float(0xf29658) + game_float((uintptr_t)(self + 0x4b0)));
    if (ran0() < game_float(0xf270b4))
        *(volatile float *)(self + 0x4b0) = (float)(ran0() + game_float((uintptr_t)(self + 0x4b0)));
}
// 0x6F5FD0, its end: a list made on the stack (0x6C3BE0: {table, entries, room, count}, filled by the game). When it has
// room, the counted text at +0x11C is let go (its count down by one; at none, its characters and itself freed) and
// replaced by a text of the first entry's name (+4; 16 new bytes made from it, none without memory), or the shared empty
// text when the name is none. Then the object array at +0x114 resized (0x6F5F30, given 1), and the list's entries freed.
static void list_name_6f5fd0(uint8_t *self) {
    uint32_t list[4];
    ((void *(__thiscall *)(void *))0x6c3be0)(list);
    if (*(volatile uint32_t *)&list[2]) {
        const uintptr_t name = *(volatile uint32_t *)(uintptr_t)(**(volatile uint32_t *volatile *)&list[1] + 4);
        int32_t *volatile *const member = (int32_t * volatile *)(self + 0x11c);
        --**member;
        int32_t *text = *member;   // (read again, as the game does)
        if (*(volatile int32_t *)text == 0 && text) {
            ((void(__cdecl *)(void *))0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
            ((void(__cdecl *)(void *))0xbce000)(text);
        }
        void *made;
        if (name) {
            void *memory = memory_alloc(0x10);
            made = memory ? ((void *(__thiscall *)(void *, uintptr_t))0xbcf850)(memory, name) : nullptr;
        } else
            made = ((void *(__fastcall *)(uint32_t))0xbd0490)(1);
        *(void *volatile *)(self + 0x11c) = made;
    }
    ((void(__thiscall *)(void *, uint32_t))0x6f5f30)(self + 0x114, 1);
    if (const uint32_t entries = *(volatile uint32_t *)&list[1]) ((void(__cdecl *)(void *))0xbce010)((void *)(uintptr_t)entries);
}
// 0x4ADEF0, its end: its two arrays {items, count} at +0x36C and +0x374 emptied, each only when it has a count: the items
// freed (aligned) if any, then the count and the items cleared. (The constructor cleared both earlier, so in practice there
// is nothing to do; done as the game does it all the same.)
static void array_emptied(uint8_t *at) {
    volatile uint32_t *const array = (volatile uint32_t *)at;
    if (array[1]) {
        if (const uint32_t items = array[0]) {
            ((void(__cdecl *)(void *))0xbce010)((void *)(uintptr_t)items);
            array[0] = 0;
        }
        array[1] = 0;
        array[0] = 0;
    }
}
static void arrays_emptied_4adef0(uint8_t *self) {
    array_emptied(self + 0x36c);
    array_emptied(self + 0x374);
}
// 0x5940A0, its end: the same for its one array, at +0x1B8.
static void array_emptied_5940a0(uint8_t *self) { array_emptied(self + 0x1b8); }
// 0xB6A000, its end: the reference member at +0x188 pointed at nothing again. Its link (+8) is taken out of its handle's
// list: the game checks both neighbours point back at it and stops (int3) when not. Its handle (+4) is let go (0x8055C0,
// given 0), the empty handle (0x805750) taken and the link put in that one's list (just after its head), and the
// reference told through its table's slot 0.
static void reference_emptied(uint8_t *const reference) {
    volatile uint32_t *const link = (volatile uint32_t *)(reference + 8);
    {
        const uint32_t previous = link[1], next = link[0];
        if (!previous || !next || ((volatile uint32_t *)(uintptr_t)next)[1] != (uint32_t)(uintptr_t)link ||
            ((volatile uint32_t *)(uintptr_t)previous)[0] != (uint32_t)(uintptr_t)link)
            __asm__ volatile("int3");
        ((volatile uint32_t *)(uintptr_t)next)[1] = previous;
        ((volatile uint32_t *)(uintptr_t)link[1])[0] = link[0];
        link[0] = 0;
        link[1] = 0;
    }
    ((void(__thiscall *)(void *, uint32_t))0x8055c0)(*(void *volatile *)(reference + 4), 0);
    uint8_t *const handle = ((uint8_t * (__cdecl *)())0x805750)();
    *(uint8_t *volatile *)(reference + 4) = handle;
    link_before((uint32_t *)link, *(uint32_t **)(handle + 0x20));
    (**(void(__thiscall *const *const *)(void *))reference)(reference);
}
static void reference_emptied_b6a000(uint8_t *self) { reference_emptied(self + 0x188); }
// 0xB17C70, its end: the same for its three reference members, at +0x110, +0xB4 and +0xD4 in that order.
static void references_emptied_b17c70(uint8_t *self) {
    reference_emptied(self + 0x110);
    reference_emptied(self + 0xb4);
    reference_emptied(self + 0xd4);
}
// 0x992B60, its end: the member at +0x40 given 0x14 (0x991A50).
static void member_992b60(uint8_t *self) { ((void(__thiscall *)(void *, uint32_t))0x991a50)(self + 0x40, 0x14); }
// 0xB7D6A0, its end: the same for its reference member at +0xF0.
static void reference_emptied_b7d6a0(uint8_t *self) { reference_emptied(self + 0xf0); }
// 0x4C29F0, its end: the array at +0x9C sized for 10 entries (0x4BF730), its ten 12-byte entries cleared (its items read
// again for each entry, as the game does), then the members at +0x90, +0x94 and +0x98 cleared.
static void entries_cleared_4c29f0(uint8_t *self) {
    ((void(__thiscall *)(void *, uint32_t))0x4bf730)(self + 0x9c, 10);
    for (uint32_t entry = 0; entry < 10; entry++)
        for (uint32_t k = 0; k < 3; k++) ((volatile uint32_t *)(uintptr_t)*(volatile uint32_t *)(self + 0x9c))[3 * entry + k] = 0;
    *(volatile uint32_t *)(self + 0x90) = 0;
    *(volatile uint32_t *)(self + 0x94) = 0;
    *(volatile uint32_t *)(self + 0x98) = 0;
}
// 0xA11C30, its end: Random::Advance (0x4751E0) drawn; 2.0 put at +0xD0; then +0xC0 given the draw's signed remainder by
// 3, plus 1.
static void random_kind_a11c30(uint8_t *self) {
    const int32_t drawn = (int32_t)((uint32_t(__cdecl *)())0x4751e0)();
    *(volatile uint32_t *)(self + 0xd0) = 0x40000000;
    *(volatile int32_t *)(self + 0xc0) = drawn % 3 + 1;
}
// 0x72D930, its end: the byte at +0x8B set to 2; the member at +0xE8 linked into the list the global 0x10EF72C points to,
// just after its head; the members +0xF4..+0x100 set to -1.
static void listed_72d930(uint8_t *self) {
    self[0x8b] = 2;
    link_before((uint32_t *)(self + 0xe8), *(uint32_t *volatile *)0x10ef72c);
    for (uint32_t at = 0xf4; at <= 0x100; at += 4) *(volatile uint32_t *)(self + at) = 0xffffffff;
}
// 0xAE0480, its end: +0x50 cleared and +0x6C set to 1, then the member at +0x58 given 5 and -1 (0x45B600).
static void member_ae0480(uint8_t *self) {
    *(volatile uint32_t *)(self + 0x50) = 0;
    *(volatile uint32_t *)(self + 0x6c) = 1;
    ((void(__thiscall *)(void *, uint32_t, uint32_t))0x45b600)(self + 0x58, 5, 0xffffffff);
}
// 0x586FA0, its end: four 8-byte entries from +0x1A8, each {0, its byte at +4 set to 1}; then +0x1FC..+0x208 set to -1.
static void entries_586fa0(uint8_t *self) {
    for (uint32_t k = 0; k < 4; k++) {
        *(volatile uint32_t *)(self + 0x1a8 + 8 * k) = 0;
        self[0x1ac + 8 * k] = 1;
    }
    for (uint32_t at = 0x1fc; at <= 0x208; at += 4) *(volatile uint32_t *)(self + at) = 0xffffffff;
}
// A chain of nodes a global points to (each node's first dword the next, none at the end) counted as the game does:
// with no first next, or none at all, a flag global is cleared. (Each node is read before it is checked, as the game's
// loop does.)
static void cleared_if_chain_empty(uintptr_t chain, uintptr_t flag) {
    const uint32_t *node = *(const uint32_t *volatile *)chain;
    uint32_t next = *(const volatile uint32_t *)node;
    int32_t count = 0;
    if (next && node)
        do {
            node = (const uint32_t *)(uintptr_t)*(const volatile uint32_t *)node;
            next = *(const volatile uint32_t *)node;
            count++;
        } while (next && node);
    if (count <= 0) *(volatile uint32_t *)flag = 0;
}
// 0x7557D0, its end: 0x1306508 cleared when the chain 0x10EF7F8 points to is empty; +0x200 set to 0x12FF8A0; the member at
// +0x530 linked into the list 0x10EF808 points to, just after its head.
static void chained_7557d0(uint8_t *self) {
    cleared_if_chain_empty(0x10ef7f8, 0x1306508);
    *(volatile uint32_t *)(self + 0x200) = 0x12ff8a0;
    link_before((uint32_t *)(self + 0x530), *(uint32_t *volatile *)0x10ef808);
}
// 0x79E5B0, its end: 0x1306C14 cleared when the chain 0x10EFB90 points to is empty; +0x194 cleared, +0x408 set to 3 and
// +0x200 to 0x12FCF10; the member at +0x534 linked into the list 0x10EFBA0 points to, just after its head.
static void chained_79e5b0(uint8_t *self) {
    cleared_if_chain_empty(0x10efb90, 0x1306c14);
    *(volatile uint32_t *)(self + 0x194) = 0;
    *(volatile uint32_t *)(self + 0x408) = 3;
    *(volatile uint32_t *)(self + 0x200) = 0x12fcf10;
    link_before((uint32_t *)(self + 0x534), *(uint32_t *volatile *)0x10efba0);
}
// 0xA2B130, its end: the object added to the array of the game object 0x1342788 points to ({+0x18 room, +0x1C count, +0x20
// items}). When the array is full, it is grown first: room doubled (at least 16), new aligned memory, the items copied
// over (the object, its count and its items read again for each), the old items freed. Then +0xB8 gets the count, the
// count goes up by one, and the object goes in at its index. Every read is through the global, as the game does.
static void appended(uint8_t *self, uintptr_t global, uint32_t index_at) {
    uint8_t *volatile *const owner = (uint8_t * volatile *)global;
    const int32_t room = *(volatile int32_t *)(*owner + 0x18);
    if (*(volatile int32_t *)(*owner + 0x1c) >= room) {
        int32_t grown = room + room;
        if (grown < 0x10) grown = 0x10;
        uint32_t *const items = (uint32_t *)((void *(__cdecl *)(uint32_t))0xbcdfd0)((uint32_t)grown * 4);
        for (int32_t k = 0; k < *(volatile int32_t *)(*owner + 0x1c); k++)
            items[k] = ((volatile uint32_t *)(uintptr_t)*(volatile uint32_t *)(*owner + 0x20))[k];
        ((void(__cdecl *)(void *))0xbce010)((void *)(uintptr_t)*(volatile uint32_t *)(*owner + 0x20));
        *(volatile uint32_t *)(*owner + 0x20) = (uint32_t)(uintptr_t)items;
        *(volatile int32_t *)(*owner + 0x18) = grown;
    }
    *(volatile uint32_t *)(self + index_at) = *(volatile uint32_t *)(*owner + 0x1c);
    ++*(volatile uint32_t *)(*owner + 0x1c);
    ((volatile uint32_t *)(uintptr_t)*(volatile uint32_t *)(*owner + 0x20))[*(volatile uint32_t *)(self + index_at)] = (uint32_t)(uintptr_t)self;
}
static void appended_a2b130(uint8_t *self) { appended(self, 0x1342788, 0xb8); }
// 0xA2A600, its end: the same into the array of the game object 0x134276C points to, its index at +0x44.
static void appended_a2a600(uint8_t *self) { appended(self, 0x134276c, 0x44); }
// 0xA189C0, its end: +0xC0 given Random::Advance's signed remainder by 4; +0x6C and +0x70 cleared, +0xD0 set to 2.0 and
// +0x68 to 0x200.
static void random_kind_a189c0(uint8_t *self) {
    *(volatile int32_t *)(self + 0xc0) = (int32_t)((uint32_t(__cdecl *)())0x4751e0)() % 4;
    *(volatile uint32_t *)(self + 0x6c) = 0;
    *(volatile uint32_t *)(self + 0x70) = 0;
    *(volatile uint32_t *)(self + 0xd0) = 0x40000000;
    *(volatile uint32_t *)(self + 0x68) = 0x200;
}
// 0x46D2B0, its end: a colour from three Random::Ran0 draws (the first two stored as floats first, the third used as it
// is on the FPU), each times the game's constant at 0xF264C0 and rounded to a whole number on the FPU (its rounding mode;
// the low half of the 64-bit result): {0xFF, third, second, first} from the top byte down, at +0x74. Then the member at
// +0x60 linked in at the end of the list 0x10EC998 points to, just before its head.
static uint32_t rounded_scaled(long double value) {
    int64_t whole;
    __asm__("fistpll %0" : "=m"(whole) : "t"(value * (long double)*(volatile const float *)0xf264c0) : "st");
    return (uint32_t)whole;
}
static void colour_listed_46d2b0(uint8_t *self) {
    const float first = (float)ran0();
    const float second = (float)ran0();
    uint32_t colour = (rounded_scaled(ran0()) | 0xffffff00) << 8;
    colour = (colour | rounded_scaled(second)) << 8;
    colour |= rounded_scaled(first);
    *(volatile uint32_t *)(self + 0x74) = colour;
    uint32_t *const head = *(uint32_t *volatile *)0x10ec998, *const node = (uint32_t *)(self + 0x60);
    node[1] = head[1];
    ((uint32_t *)(uintptr_t)head[1])[0] = (uint32_t)(uintptr_t)node;
    node[0] = (uint32_t)(uintptr_t)head;
    head[1] = (uint32_t)(uintptr_t)node;
}
// 0x6E6530, its end: +0x90 cleared when +0x88 is not 0.
static void cleared_6e6530(uint8_t *self) {
    if (*(volatile uint32_t *)(self + 0x88)) *(volatile uint32_t *)(self + 0x90) = 0;
}
// 0x7977F0, its end: 0x1306AF8 cleared when the chain 0x10EF9D0 points to is empty; +0x200 set to 0x12F8C38; the first
// time (the count 0x1306AFC still 0) the game objects at 0x1306B1C, 0x1306B64 and 0x1306B4C set up (0x7541A0 on each),
// and the count raised; then the member at +0x510 linked into the list 0x10EF9E0 points to, just after its head.
static void chained_7977f0(uint8_t *self) {
    cleared_if_chain_empty(0x10ef9d0, 0x1306af8);
    *(volatile uint32_t *)(self + 0x200) = 0x12f8c38;
    if (!*(volatile uint32_t *)0x1306afc) {
        ((void(__thiscall *)(void *))0x7541a0)((void *)0x1306b1c);
        ((void(__thiscall *)(void *))0x7541a0)((void *)0x1306b64);
        ((void(__thiscall *)(void *))0x7541a0)((void *)0x1306b4c);
    }
    ++*(volatile uint32_t *)0x1306afc;
    link_before((uint32_t *)(self + 0x510), *(uint32_t *volatile *)0x10ef9e0);
}
// 0xA0A650, its end: twelve members cleared and four set to 1.0 (in the game's order), then the object given to 0xA0DDF0,
// called on the game object 0x13422C0 holds.
static void registered_a0a650(uint8_t *self) {
    static const uint16_t zeros[12] = {0xb8, 0xb4, 0xb0, 0xac, 0xa4, 0xa0, 0x9c, 0x98, 0x90, 0x8c, 0x88, 0x84};
    static const uint16_t ones[4] = {0xbc, 0xa8, 0x94, 0x80};
    for (const uint16_t at : zeros) *(volatile uint32_t *)(self + at) = 0;
    for (const uint16_t at : ones) *(volatile uint32_t *)(self + at) = 0x3f800000;
    ((void(__thiscall *)(void *, void *))0xa0ddf0)(*(void *volatile *)0x13422c0, self);
}
// 0xAF19C0, its end: the member at +0xB0 linked in at the end of the list whose head the game object 0x13CE5C0 holds
// points to (just before that head).
static void listed_af19c0(uint8_t *self) {
    uint32_t *const head = **(uint32_t **volatile *)0x13ce5c0, *const node = (uint32_t *)(self + 0xb0);
    node[1] = head[1];
    ((uint32_t *)(uintptr_t)head[1])[0] = (uint32_t)(uintptr_t)node;
    node[0] = (uint32_t)(uintptr_t)head;
    head[1] = (uint32_t)(uintptr_t)node;
}
// 0xA630E0, its end: the object put in the global 0x13429A8; then four parts made, each on new memory (none without it)
// given the members at +0x200 and +0x3B0, into +0x3B8 (0xE0 bytes, 0xA61160), +0x3BC (0x30, 0xA5F3C0), +0x3C0 (0x50,
// 0xA5FA50) and +0x3C4 (0x50, 0xA61B20).
static void parts_a630e0(uint8_t *self) {
    static const uint32_t parts[4][3] = {{0xe0, 0xa61160, 0x3b8}, {0x30, 0xa5f3c0, 0x3bc}, {0x50, 0xa5fa50, 0x3c0}, {0x50, 0xa61b20, 0x3c4}};
    *(uint8_t *volatile *)0x13429a8 = self;
    for (const auto &part : parts) {
        void *const memory = memory_alloc(part[0]);
        *(void *volatile *)(self + part[2]) = memory ? ((void *(__thiscall *)(void *, void *, void *))(uintptr_t)part[1])(memory, self + 0x200, self + 0x3b0) : nullptr;
    }
}
// 0x79A440, its end: +0x200 set to 0x12FA250; the member at +0x520 linked into the list 0x10EFA10 points to, just after
// its head; +0xB8 set to 0x147 and +0xBC cleared; then the object given 0xD and 0 (0x7CD220).
static void listed_79a440(uint8_t *self) {
    *(volatile uint32_t *)(self + 0x200) = 0x12fa250;
    link_before((uint32_t *)(self + 0x520), *(uint32_t *volatile *)0x10efa10);
    *(volatile uint32_t *)(self + 0xb8) = 0x147;
    *(volatile uint32_t *)(self + 0xbc) = 0;
    ((void(__thiscall *)(void *, uint32_t, uint32_t))0x7cd220)(self, 0xd, 0);
}
// 0xADF4E0, its end: the member at +0xDC linked into the list the game object 0x13CE524 holds keeps at +0x10, just after
// its head; then three named parts (0x1C bytes each, 0x403F30 given the name, 0x1303CFC and 1; none without memory) into
// +0xF0, +0xF4 and +0xF8.
static void named_parts_adf4e0(uint8_t *self) {
    link_before((uint32_t *)(self + 0xdc), *(uint32_t **)(*(uint8_t *volatile *)0x13ce524 + 0x10));
    static const uint32_t parts[3][2] = {{0xf954d0, 0xf0}, {0xf954c4, 0xf4}, {0xf954b8, 0xf8}};
    for (const auto &part : parts) {
        void *const memory = memory_alloc(0x1c);
        *(void *volatile *)(self + part[1]) =
            memory ? ((void *(__thiscall *)(void *, uintptr_t, uintptr_t, uint32_t))0x403f30)(memory, part[0], 0x1303cfc, 1) : nullptr;
    }
}
// 0x44F240, its end:
// - the object put in the first empty slot of the game's array 0x1106D0C {items, size} (or past its end; the array made
//   one bigger first, 0x44D040, when the slot isn't inside it);
// - a part made on 0x64 new bytes (0x47D160, given 0, 0, the game value 0x1302858, +0xA0, +0x130 and 0; none without
//   memory) into +0x1A8;
// - that part's counted text at +0x44 replaced by a text of 0xF29590 (16 new bytes; the old one let go, the new one's
//   count raised), the text made then let go once (the part keeps its count);
// - the part's inner object (+0xC) given 40.0 at +0x80 and 500.0 at +0x84;
// - the part's byte +0x10 set; then, when its +0x18 is 0, its byte +0x12 set; else, when its byte +0x11 is 0, 0xCE4770
//   run on the inner object (given its +0x50), the byte +0x11 set and +0x30 cleared.
static void counted_text_let_go(uint32_t *text) {
    if (*(volatile uint32_t *)text == 0 && text) {
        ((void(__cdecl *)(void *))0xbce010)((void *)(uintptr_t)text[3]);
        ((void(__cdecl *)(void *))0xbce000)(text);
    }
}
static void slot_part_44f240(uint8_t *self) {
    volatile uint32_t *const array = (volatile uint32_t *)0x1106d0c;
    const int32_t size = (int32_t)array[1];
    int32_t slot = size;
    for (int32_t k = 0; k < size; k++)
        if (!((volatile uint32_t *)(uintptr_t)array[0])[k]) {
            slot = k;
            break;
        }
    if (slot + 1 >= size) ((void(__thiscall *)(void *, uint32_t))0x44d040)((void *)array, (uint32_t)(slot + 1));
    ((volatile uint32_t *)(uintptr_t)array[0])[slot] = (uint32_t)(uintptr_t)self;
    void *const memory = memory_alloc(0x64);
    *(void *volatile *)(self + 0x1a8) = memory ?
        ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, void *, void *, uint32_t))0x47d160)(memory, 0, 0, *(volatile uint32_t *)0x1302858, self + 0xa0, self + 0x130, 0) : nullptr;
    void *const text_memory = memory_alloc(0x10);
    uint32_t *const text = text_memory ? (uint32_t *)((void *(__thiscall *)(void *, uintptr_t))0xbcf850)(text_memory, 0xf29590) : nullptr;
    uint8_t *const part = *(uint8_t *volatile *)(self + 0x1a8);
    if ((uint32_t)(uintptr_t)text != *(volatile uint32_t *)(part + 0x44)) {
        --**(volatile uint32_t *volatile *)(part + 0x44);
        counted_text_let_go(*(uint32_t *volatile *)(part + 0x44));
        *(uint32_t *volatile *)(part + 0x44) = text;
        ++*(volatile uint32_t *)text;
    }
    --*(volatile uint32_t *)text;
    if (!*(volatile uint32_t *)text) {
        ((void(__cdecl *)(void *))0xbce010)((void *)(uintptr_t)text[3]);
        ((void(__cdecl *)(void *))0xbce000)(text);
    }
    *(volatile uint32_t *)(*(uint8_t *volatile *)(*(uint8_t *volatile *)(self + 0x1a8) + 0xc) + 0x80) = 0x42200000;
    *(volatile uint32_t *)(*(uint8_t *volatile *)(*(uint8_t *volatile *)(self + 0x1a8) + 0xc) + 0x84) = 0x43fa0000;
    uint8_t *const again = *(uint8_t *volatile *)(self + 0x1a8);
    const uint32_t there = *(volatile uint32_t *)(again + 0x18);
    again[0x10] = 1;
    if (!there)
        again[0x12] = 1;
    else if (!again[0x11]) {
        uint8_t *const inner = *(uint8_t *volatile *)(again + 0xc);
        ((void(__thiscall *)(void *, void *))0xce4770)(inner, inner + 0x50);
        again[0x11] = 1;
        *(volatile uint32_t *)(again + 0x30) = 0;
    }
}
// 0x8DD190, its end: the game flags 0x1313C64 given bit 0 and their bits 1-3 cleared; then the object put in the global
// 0x1313C60, the one there before (if any) deleted first (its table's slot 0, given 1).
static void flagged_instance_8dd190(uint8_t *self) {
    *(volatile uint32_t *)0x1313c64 = (*(volatile uint32_t *)0x1313c64 & 0xfffffff1) | 1;
    if (void *before = *(void *volatile *)0x1313c60) ((void(__thiscall *)(void *, uint32_t))(**(void ***)before))(before, 1);
    *(uint8_t *volatile *)0x1313c60 = self;
}
// 0x6404D0, its end: the member at +0x5C given (0x63E1C0) the product of two game values (0x10EEAD0 by 0x10EEACC),
// rounded to a whole number on the FPU (its rounding mode; the low half of the 64-bit result).
static void scaled_value_6404d0(uint8_t *self) {
    long double product = (long double)*(volatile float *)0x10eead0 * (long double)*(volatile float *)0x10eeacc;
    int64_t whole;
    __asm__("fistpll %0" : "=m"(whole) : "t"(product) : "st");
    ((void(__thiscall *)(void *, uint32_t))0x63e1c0)(self + 0x5c, (uint32_t)whole);
}
// 0x9D91F0, its end: the member at +0x50 given (0x98A770) a reference made on the stack to nothing: its table, the
// handle of no object (0x805700 with 0), linked (+8) into that handle's list, no owner and its flag set; then that
// reference let go (0x40F730, which unlinks it).
static void empty_reference_9d91f0(uint8_t *self) {
    uint32_t reference[8];
    reference[4] = (uint32_t)(uintptr_t)reference;
    reference[0] = 0xf26ae0;
    uint8_t *handle = ((uint8_t * (__fastcall *)(uintptr_t))0x805700)(0);
    reference[1] = (uint32_t)(uintptr_t)handle;
    uint32_t *list = *(uint32_t **)(handle + 0x20);
    reference[2] = list[0];
    ((uint32_t *)(uintptr_t)list[0])[1] = (uint32_t)(uintptr_t)&reference[2];
    reference[3] = (uint32_t)(uintptr_t)list;
    list[0] = (uint32_t)(uintptr_t)&reference[2];
    reference[0] = 0xf7ff1c;
    reference[5] = 0;
    *(uint8_t *)&reference[6] = 1;
    ((void(__thiscall *)(void *, void *))0x98a770)(self + 0x50, reference);
    ((void(__thiscall *)(void *))0x40f730)(reference);
}
// 0xA5D6F0, its end: the object kept in its global (0x134297C); its six slots (+0x60, 0xA5D540 with 6) emptied (each
// slot's aligned block freed and cleared, its second word cleared); its table of 31 words (+0x6C) freed if it has one,
// then made again (aligned, 0x7C bytes, count +0x70 = 31) and cleared.
static void slots_a5d6f0(uint8_t *self) {
    typedef void(__cdecl * Free)(void *);
    *(uint8_t *volatile *)0x134297c = self;
    ((void(__thiscall *)(void *, uint32_t))0xa5d540)(self + 0x60, 6);
    for (uint32_t at = 0; at < 0x30; at += 8) {
        uint32_t *slot = (uint32_t *)(*(uint8_t *volatile *)(self + 0x60) + at);
        if (slot[0]) {
            ((Free)0xbce010)((void *)(uintptr_t)slot[0]);
            slot[0] = 0;
        }
        slot[1] = 0;
    }
    if (uint32_t old = *(volatile uint32_t *)(self + 0x6c)) {
        ((Free)0xbce010)((void *)(uintptr_t)old);
        *(volatile uint32_t *)(self + 0x6c) = 0;
    }
    *(void *volatile *)(self + 0x6c) = memory_alloc_aligned(0x7c);
    *(volatile uint32_t *)(self + 0x70) = 0x1f;
    for (uint32_t at = 0; at < 0x7c; at += 4) *(volatile uint32_t *)(*(uint8_t *volatile *)(self + 0x6c) + at) = 0;
}

#include "class_list.inc"
