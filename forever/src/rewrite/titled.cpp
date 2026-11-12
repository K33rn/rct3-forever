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

// A member step: 0 dword store, 1 byte store, 4 word store, 2 store the constructor's argument number value, 3 call fn
// on the member at +off (thiscall) with count values, 5 store the global at value as it is then, 6 store a pointer to
// +value of the object, 7 store the shared empty wide string, 10 store at +off a new object (value bytes made by fn with
// count values, or null when there was no memory; 11 the same storing the memory rather than what fn returns), 12 the mark (no parent: the title object's +4 = 1).
// 8 makes the member at +off a reference to the empty handle (0x805750) linked (+8) first in that handle's list, 9 the
// same with the handle of no object (0x805700 given null).
struct TitledOp {
    uint32_t kind, off, value, fn, count, args[5], globals;   // bit i of globals: args[i] is a global, read when used
    uint32_t given;                                           // bit i: args[i] is the constructor's argument number args[i]
    uint32_t selfs;                                           // bit i: args[i] is an offset into the object (a pointer to it)
    uint32_t temps;                                           // bit i: args[i] is the temporary (what its maker returned)
    uint32_t temp_fn, temp_count, temp_args[3];               // 10/11: a temporary made first (thiscall, constant values)
};
// 15 destroys the temporary (fn, thiscall) when it was made.
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
    alignas(16) uint8_t temp[0x40];   // a temporary on the stack (a text made for a new object's constructor)
    void *temp_made = nullptr;
    bool temp_built = false;
    for (uint32_t k = 0; k < COUNT; k++) {
        const TitledOp &op = OPS[k];
        uint32_t args[5];
        for (uint32_t i = 0; i < 5; i++)
            args[i] = op.globals >> i & 1 ? *(uint32_t *)(uintptr_t)op.args[i]
                      : op.given >> i & 1 ? given[op.args[i]]
                      : op.selfs >> i & 1 ? (uint32_t)(uintptr_t)(self + op.args[i])
                                          : op.args[i];
        switch (op.kind) {
        case 0: *(uint32_t *)(self + op.off) = op.value; break;
        case 1: self[op.off] = (uint8_t)op.value; break;
        case 4: *(uint16_t *)(self + op.off) = (uint16_t)op.value; break;
        case 2: *(uint32_t *)(self + op.off) = given[op.value]; break;
        case 3: titled_call(op.fn, self + op.off, op.count, args); break;
        case 5: *(uint32_t *)(self + op.off) = *(uint32_t *)(uintptr_t)op.value; break;
        case 6: *(uint8_t **)(self + op.off) = self + op.value; break;
        case 7: *(void **)(self + op.off) = ((void *(__fastcall *)(uint32_t))0xbd0490)(1); break;
        case 10:
        case 11: {   // 10 stores what the constructor returns, 11 the memory it was given
            void *memory = memory_alloc(op.value), *object = nullptr;
            if (memory) {
                if (op.temp_fn) {
                    temp_made = titled_call(op.temp_fn, temp, op.temp_count, op.temp_args);
                    temp_built = true;
                    for (uint32_t i = 0; i < 5; i++)
                        if (op.temps >> i & 1) args[i] = (uint32_t)(uintptr_t)temp_made;
                }
                object = titled_call(op.fn, memory, op.count, args);
            }
            *(void **)(self + op.off) = op.kind == 10 ? object : memory;
            break;
        }
        case 8:
        case 9: {
            uint8_t *handle = op.kind == 8 ? ((uint8_t * (__cdecl *)())0x805750)() : ((uint8_t * (__thiscall *)(void *))0x805700)(nullptr);
            *(uint8_t **)(self + op.off + 4) = handle;
            uint32_t *link = (uint32_t *)(self + op.off + 8), *head = *(uint32_t **)(handle + 0x20);
            link[0] = head[0];
            ((uint32_t *)(uintptr_t)head[0])[1] = (uint32_t)(uintptr_t)link;
            link[1] = (uint32_t)(uintptr_t)head;
            head[0] = (uint32_t)(uintptr_t)link;
            break;
        }
        case 15:
            if (temp_built) ((void(__thiscall *)(void *))(uintptr_t)op.fn)(temp);
            temp_built = false;
            break;
        case 12:
            if (!parent) (*(uint8_t **)TITLE)[4] = 1;
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
