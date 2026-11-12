// Window classes built from a shared title, any base: windows.cpp's Window::Construct with the base class's own
// constructor (and the values it takes after the parent and the text), then the class's member steps, which here also
// make new objects, call members with values, copy globals and mark the title object where the code does.
// tools/titled_ctors.py writes the list (titled_list.inc).
#include "wave.h"

static void titled_release(void *text) {   // a wide string's shared block: freed when no one has it (no check for none)
    int32_t *block = (int32_t *)text;
    if (--*block) return;
    memory_free_aligned((void *)(uintptr_t)block[3]);
    memory_free(block);
}

// A member step, in the order the code does them:
//   0 dword store, 1 byte store, 4 word store, 2 store the constructor's argument number value, 5 store the global at
//   value as it is then, 6 store a pointer to +value of the object, 7 store the shared empty wide string, 29 store what
//   the constructor's argument number value points to, 30 copy the member at +value, 26 / 27 and / or the member with
//   value;
//   3 call fn on the member at +off (thiscall), 24 call fn on what the member at +off points to;
//   8 the member at +off a reference to the empty handle (0x805750) linked (+8) first in that handle's list, 9 the same
//   with the handle Handle::For (0x805700) gives for the constructor's argument number value (0: none);
//   12 the mark (no parent: the title object's +4 = 1);
//   a new object: 18 value bytes of memory (0xBCDFC0), then only when there was memory: 21 name number off made from
//   the C text at value (0x10 bytes of memory, WideString::Create; null without memory), 20 temporary number off made
//   on the stack by fn, 10 the object built by fn; 22 stores at +off what fn returned (value 0) or the memory (1), null
//   when there was no memory; 31 / 32 / 33 store a dword / the global at value / a byte into the new object (when
//   there was memory: its own constructor inlined);
//   15 destroy temporary number off with fn, 16 let go of the wide string it holds, 17 let go of name number off: each
//   only when it was made.
// The values given to fn are by kind (4 bits each in kinds): 0 the value, 1 the global there, 2 the constructor's
// argument number value, 3 a pointer to +value of the object, 4 what temporary number value's maker returned, 5 where
// name number value is kept, 6 the member at +value.
struct TitledOp {
    uint32_t kind, off, value, fn, count, args[5], kinds;
};
// A value the base takes after (parent or title, text): a constant, or the constructor's argument number value.
struct TitledExtra {
    uint32_t is_arg, value;
};

static void *titled_call(uintptr_t fn, void *self, uint32_t count, const uint32_t *a) {
    switch (count) {
    case 0: return ((void *(__thiscall *)(void *))fn)(self);
    case 1: return ((void *(__thiscall *)(void *, uint32_t))fn)(self, a[0]);
    case 2: return ((void *(__thiscall *)(void *, uint32_t, uint32_t))fn)(self, a[0], a[1]);
    case 3: return ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))fn)(self, a[0], a[1], a[2]);
    case 4: return ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))fn)(self, a[0], a[1], a[2], a[3]);
    case 5: return ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))fn)(self, a[0], a[1], a[2], a[3], a[4]);
    default: return ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))fn)(self, a[0], a[1], a[2], a[3], a[4], a[5]);
    }
}

static volatile long g_titled_constructs;
// given[1..]: the constructor's arguments, (parent, text, ...)
template <uintptr_t TITLE, uintptr_t BASE, const TitledExtra *EXTRAS, uint32_t EXTRA_COUNT, const TitledOp *OPS, uint32_t COUNT>
static void *titled_body(uint8_t *self, const uint32_t *given) {
    g_titled_constructs++;
    void *parent = (void *)(uintptr_t)given[1];
    void **text = (void **)(uintptr_t)given[2];
    uint8_t *title = *(uint8_t **)TITLE;
    void *made = nullptr;   // the copy of the title when there's no text
    bool copied = false;
    void *used;
    if (text) {
        void *name;
        const char *title_name = *(const char **)(title + 8);
        if (title_name) {
            void *memory = memory_alloc(0x10);
            name = memory ? ((void *(__thiscall *)(void *, const char *))0xbcf850)(memory, title_name) : nullptr;
        } else {
            name = ((void *(__fastcall *)(uint32_t))0xbd0490)(1);
        }
        void *with_separator = nullptr, *joined = nullptr;
        void *first = ((void *(__thiscall *)(void *, void **, const char *))0x403be0)(&name, &with_separator, (const char *)0xf2738c);
        void **result = (void **)((void *(__thiscall *)(void *, void **, void **))0x403bb0)(first, &joined, text);
        if (*result != *text) {
            int32_t *old = (int32_t *)*text;
            --*old;
            old = (int32_t *)*text;
            if (!*old && old) {
                memory_free_aligned((void *)(uintptr_t)old[3]);
                memory_free(old);
            }
            *text = *result;
            ++*(int32_t *)*result;
        }
        titled_release(joined);
        titled_release(with_separator);
        titled_release(name);
        used = text;
    } else {
        used = ((void *(__thiscall *)(void **, void *))0x41c740)(&made, title);
        copied = true;
    }
    uint32_t base_args[6] = {(uint32_t)(uintptr_t)(parent ? parent : *(void **)TITLE), (uint32_t)(uintptr_t)used};
    for (uint32_t i = 0; i < EXTRA_COUNT; i++) base_args[2 + i] = EXTRAS[i].is_arg ? given[EXTRAS[i].value] : EXTRAS[i].value;
    titled_call(BASE, self, 2 + EXTRA_COUNT, base_args);
    if (copied) titled_release(made);
    alignas(16) uint8_t temps[3][0x40];   // temporaries on the stack (texts made for a new object's constructor)
    void *temp_made[3] = {}, *names[3] = {};
    bool temp_built[3] = {}, name_built[3] = {};
    uint8_t *memory = nullptr;
    void *object = nullptr;
    for (uint32_t k = 0; k < COUNT; k++) {
        const TitledOp &op = OPS[k];
        uint32_t args[5];
        for (uint32_t i = 0; i < op.count; i++) {
            uint32_t v = op.args[i];
            switch (op.kinds >> (4 * i) & 15) {
            case 0: args[i] = v; break;
            case 1: args[i] = *(uint32_t *)(uintptr_t)v; break;
            case 2: args[i] = given[v]; break;
            case 3: args[i] = (uint32_t)(uintptr_t)(self + v); break;
            case 4: args[i] = (uint32_t)(uintptr_t)temp_made[v]; break;
            case 5: args[i] = (uint32_t)(uintptr_t)&names[v]; break;
            default: args[i] = *(uint32_t *)(self + v); break;
            }
        }
        switch (op.kind) {
        case 0: *(uint32_t *)(self + op.off) = op.value; break;
        case 1: self[op.off] = (uint8_t)op.value; break;
        case 4: *(uint16_t *)(self + op.off) = (uint16_t)op.value; break;
        case 2: *(uint32_t *)(self + op.off) = given[op.value]; break;
        case 5: *(uint32_t *)(self + op.off) = *(uint32_t *)(uintptr_t)op.value; break;
        case 6: *(uint8_t **)(self + op.off) = self + op.value; break;
        case 7: *(void **)(self + op.off) = ((void *(__fastcall *)(uint32_t))0xbd0490)(1); break;
        case 29: *(uint32_t *)(self + op.off) = *(uint32_t *)(uintptr_t)given[op.value]; break;
        case 30: *(uint32_t *)(self + op.off) = *(uint32_t *)(self + op.value); break;
        case 26: *(uint32_t *)(self + op.off) &= op.value; break;
        case 27: *(uint32_t *)(self + op.off) |= op.value; break;
        case 3: titled_call(op.fn, self + op.off, op.count, args); break;
        case 24: titled_call(op.fn, *(void **)(self + op.off), op.count, args); break;
        case 8:
        case 9: {
            uint8_t *handle = op.kind == 8 ? ((uint8_t * (__cdecl *)())0x805750)()
                                           : ((uint8_t * (__thiscall *)(void *))0x805700)(op.value ? (void *)(uintptr_t)given[op.value] : nullptr);
            *(uint8_t **)(self + op.off + 4) = handle;
            uint32_t *link = (uint32_t *)(self + op.off + 8), *head = *(uint32_t **)(handle + 0x20);
            link[0] = head[0];
            ((uint32_t *)(uintptr_t)head[0])[1] = (uint32_t)(uintptr_t)link;
            link[1] = (uint32_t)(uintptr_t)head;
            head[0] = (uint32_t)(uintptr_t)link;
            break;
        }
        case 12:
            if (!parent) (*(uint8_t **)TITLE)[4] = 1;
            break;
        case 18:
            memory = (uint8_t *)memory_alloc(op.value);
            object = nullptr;
            break;
        case 21:
            if (memory) {
                void *block = memory_alloc(0x10);
                names[op.off] = block ? ((void *(__thiscall *)(void *, const char *))0xbcf850)(block, (const char *)(uintptr_t)op.value) : nullptr;
                name_built[op.off] = true;
            }
            break;
        case 20:
            if (memory) {
                temp_made[op.off] = titled_call(op.fn, temps[op.off], op.count, args);
                temp_built[op.off] = true;
            }
            break;
        case 10:
            if (memory) object = titled_call(op.fn, memory, op.count, args);
            break;
        case 22: *(void **)(self + op.off) = op.value ? (void *)memory : object; break;
        case 31: if (memory) *(uint32_t *)(memory + op.off) = op.value; break;
        case 32: if (memory) *(uint32_t *)(memory + op.off) = *(uint32_t *)(uintptr_t)op.value; break;
        case 33: if (memory) memory[op.off] = (uint8_t)op.value; break;
        case 15:
            if (temp_built[op.off]) ((void(__thiscall *)(void *))(uintptr_t)op.fn)(temps[op.off]);
            temp_built[op.off] = false;
            break;
        case 16:
            if (temp_built[op.off]) titled_release(*(void **)temps[op.off]);
            temp_built[op.off] = false;
            break;
        case 17:
            if (name_built[op.off]) titled_release(names[op.off]);
            name_built[op.off] = false;
            break;
        }
    }
    return self;
}
// The constructors take (parent, text) and up to four more values.
#define TITLED_ENTRY(N, PARAMS, GIVEN)                                                                                        \
    template <uintptr_t TITLE, uintptr_t BASE, const TitledExtra *EXTRAS, uint32_t EXTRA_COUNT, const TitledOp *OPS, uint32_t COUNT> \
    static void *__thiscall titled_construct##N PARAMS {                                                                       \
        const uint32_t given[] = GIVEN;                                                                                        \
        return titled_body<TITLE, BASE, EXTRAS, EXTRA_COUNT, OPS, COUNT>(self, given);                                         \
    }
#define COMMA ,
TITLED_ENTRY(2, (uint8_t * self, uint32_t a, uint32_t b), {0 COMMA a COMMA b})
TITLED_ENTRY(3, (uint8_t * self, uint32_t a, uint32_t b, uint32_t c), {0 COMMA a COMMA b COMMA c})
TITLED_ENTRY(4, (uint8_t * self, uint32_t a, uint32_t b, uint32_t c, uint32_t d), {0 COMMA a COMMA b COMMA c COMMA d})
TITLED_ENTRY(5, (uint8_t * self, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e), {0 COMMA a COMMA b COMMA c COMMA d COMMA e})
TITLED_ENTRY(6, (uint8_t * self, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f), {0 COMMA a COMMA b COMMA c COMMA d COMMA e COMMA f})
#undef COMMA
#undef TITLED_ENTRY
#include "titled_list.inc"
