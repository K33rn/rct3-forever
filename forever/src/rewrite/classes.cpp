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
        ((void(__thiscall *)(void *, void *, uint32_t, uint32_t))BASE)(self, description ? description : *(uint8_t **)DESCRIPTION, a, b);   // (Class::Construct, or a parent class's constructor)
    uint32_t k = 0;
    for (int pass = 0; pass < 2; pass++) {
    for (; k < OP_COUNT; k++) {
        const ClassOp &op = OPS[k];
        if (op.kind == 14) break;
        if (op.kind == 0) *(uint32_t *)(self + op.off) = op.value;
        else if (op.kind == 1) self[op.off] = (uint8_t)op.value;
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
        else if (op.kind == 13) (*(uint32_t **)(self + op.off))[*(uint32_t *)(self + op.off + 4)] = op.value;
        else if (op.kind == 18) *(uint32_t *)(self + op.off) = *(uint32_t *)(self + op.value);
        else if (op.kind == 17) class_hand(op.value, self, given);
        else if (op.kind == 20) class_resource(self + op.off, op.value, op.fn);
        else if (op.kind == 21) class_named_text(self, op.off, op.value, op.fn);
        else if (op.kind == 23) *(uint32_t *)(self + op.off) = ((uint32_t(__cdecl *)())(uintptr_t)op.fn)();   // what a function taking nothing gives
        else if (op.kind == 24) *(void **)(self + op.off) = ((void *(__cdecl *)(uint32_t))(uintptr_t)op.fn)(op.value);   // aligned memory
        else if (op.kind == 25) {   // a reference made on the stack (0xBD9AD0), given with the name value and a flag to fn on the member at off, let go (0xBD99E0)
            alignas(16) uint8_t reference[0x40];
            void *made = ((void *(__thiscall *)(void *))0xbd9ad0)(reference);
            ((void(__thiscall *)(void *, uint32_t, void *, uint32_t))(uintptr_t)op.fn)(self + op.off, op.value, made, op.args[0]);
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
        else {
            uint32_t args[5];
            for (uint32_t i = 0; i < 5; i++)
                args[i] = op.globals >> i & 1 ? *(uint32_t *)(uintptr_t)op.args[i] : op.selfs >> i & 1 ? (uint32_t)(uintptr_t)(self + op.args[i]) : op.args[i];
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
            } else
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
    }
    if constexpr (TAIL != nullptr) {
        if (TAIL->link) {
            uint8_t *list = *(uint8_t **)TAIL->link;
            link_before((uint32_t *)(self + TAIL->member), TAIL->link_at < 0 ? (uint32_t *)list : *(uint32_t **)(list + TAIL->link_at));
        }
        if (TAIL->count) ++*(uint32_t *)TAIL->count;
        if (TAIL->instance) *(uint8_t **)TAIL->instance = self;
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
