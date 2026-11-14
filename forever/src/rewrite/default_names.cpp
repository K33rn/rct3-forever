// LaserDesigner::DefaultName and WaterJetDesigner::DefaultName: one function written out twice (tools/family.py:
// default_names.inc; they differ in where the design's name is kept, +0x54 or +0x44, and the text name's start,
// "LaserName_" or "WaterJetName_"). Given where the name goes (thiscall, one argument), the name shown for the design:
//   - when 0x401310 says so (given the design's name by value), the design's name itself;
//   - a name beginning "%%" or "##" (and longer than that) names a text: the rest of it looked up in the game's text
//     table; found, its text (a copy, or the shared empty text when it has none); not found, the design's name;
//   - otherwise the start ("LaserName_") with the name's characters after it, spaces and apostrophes left out, looked
//     up the same way; not found, the design's name. (Each character is read with the name made its own first, and the
//     name lengthened when the place is past its end, as the game's indexing does.)
// A text is {count, length, room, characters (two bytes each)}.
#include "rewrite.h"

namespace {
struct DefaultName {
    uint32_t at, name_at, start;
};
#define DEFAULT_NAMES(at, name_at, start, ...) {at, name_at, start},
constexpr DefaultName default_names[] = {
#include "default_names.inc"
};
#undef DEFAULT_NAMES
#define DEFAULT_NAMES(at, name_at, start, ...) {__VA_ARGS__},
const uint8_t default_names_first[][7] = {
#include "default_names.inc"
};
#undef DEFAULT_NAMES

struct Text {
    int32_t count;
    uint32_t length, room;
    uint16_t *characters;
};
typedef void *(__cdecl *Alloc)(uint32_t);
typedef void(__cdecl *Free)(void *);
const Alloc alloc = (Alloc)0xbcdfc0;
// a shared text let go: the count taken down; at 0 its buffer freed, then it
inline void let_go(Text *volatile *held) {
    --(*held)->count;
    Text *text = *held;   // read again, as the game does
    if (*(volatile int32_t *)&text->count == 0) {
        ((Free)0xbce010)(text->characters);
        ((Free)0xbce000)(text);
    }
}
inline Text *text_c(uintptr_t c_text) {
    void *memory = alloc(0x10);
    return memory ? ((Text * (__thiscall *)(void *, uintptr_t))0xbcf850)(memory, c_text) : nullptr;
}
inline void *substring(Text *volatile *of, Text *volatile *into, uint32_t from, uint32_t length) {
    return ((void *(__thiscall *)(void *, void *, uint32_t, uint32_t))0xbd0810)((void *)of, (void *)into, from, length);
}
inline bool same(void *text, uintptr_t c_text) {   // String compare with a text made from C text, which is let go
    Text *volatile other = text_c(c_text);
    const bool equal = ((int32_t(__thiscall *)(void *, void *))0xbce970)(text, (void *)&other) == 0;
    let_go(&other);
    return equal;
}
inline void make_room(Text *volatile *text, uint32_t length) { ((void(__thiscall *)(void *, uint32_t))0xbcffe0)((void *)text, length); }
volatile long g_default_names;
Text **default_name(uint8_t *self, Text **out, const DefaultName &kind) {
    g_default_names++;
    Text *volatile *name = (Text *volatile *)(self + kind.name_at);
    Text *given = *name;
    ++given->count;
    if (((uint8_t(__stdcall *)(Text *))0x401310)(given)) goto own;
    {
        bool text_name = false;
        Text *volatile first, *volatile second;
        int flags = 0;
        if ((*name)->length > 2) {
            void *part = substring(name, &first, 0, 2);
            flags = 2;
            text_name = same(part, 0xf84950);   // "%%"
            if (!text_name) {
                part = substring(name, &second, 0, 2);
                flags = 6;
                text_name = same(part, 0xf8494c);   // "##"
            }
        }
        if (flags & 4) let_go(&second);
        if (flags & 2) let_go(&first);
        alignas(16) uint8_t reference[0x40];
        if (text_name) {
            Text *volatile rest, *volatile key;
            void *part = substring(name, &rest, 2, (*name)->length - 2);
            ((void *(__thiscall *)(void *, void *))0xbd01c0)((void *)&key, part);
            ((void *(__thiscall *)(void *, void *, uint32_t, uint32_t))0x410c90)(reference, (void *)&key, 0x1303cfc, 1);
            let_go(&key);
            let_go(&rest);
            if (((uint8_t(__thiscall *)(void *, uint32_t))0xbd1420)(reference, 0)) {
                const void *characters = *(const void *volatile *)(reference + 8);
                void *text;
                if (characters) {
                    void *memory = alloc(0x10);
                    text = memory ? ((void *(__thiscall *)(void *, const void *))0xbcf7d0)(memory, characters) : nullptr;
                } else {
                    text = ((void *(__fastcall *)(uint32_t))0xbd0490)(1);
                }
                *out = (Text *)text;
                ((void(__thiscall *)(void *))0xbd1880)(reference);
                return out;
            }
            ((void(__thiscall *)(void *))0xbd1880)(reference);
            goto own;
        }
        Text *volatile built = text_c(kind.start);
        for (uint32_t at = 0; at < (*name)->length; at++) {
            Text *text = *name;   // made its own first
            if (text->count > 1) {
                --text->count;
                void *memory = alloc(0x10);
                *name = memory ? ((Text * (__thiscall *)(void *, const uint16_t *, uint32_t))0xbcf710)(memory, (*name)->characters, (*name)->length) : nullptr;
            }
            if (at >= (*name)->length) {
                make_room(name, at + 1);
                (*name)->length = at + 1;
            }
            const uint16_t c = (*name)->characters[at];
            if (c != ' ' && c != '\'') {
                const uint32_t length = built->length;
                make_room(&built, length + 2);
                built->characters[length] = c;
                built->characters[length + 1] = 0;
                built->length = length + 1;
            }
        }
        Text *volatile key;
        ((void *(__thiscall *)(void *, void *))0xbd01c0)((void *)&key, (void *)&built);
        ((void *(__thiscall *)(void *, void *, uint32_t, uint32_t))0x410c90)(reference, (void *)&key, 0x1303cfc, 1);
        let_go(&key);
        if (((uint8_t(__thiscall *)(void *, uint32_t))0xbd1420)(reference, 0)) {
            const void *characters = *(const void *volatile *)(reference + 8);
            void *text;
            if (characters) {
                void *memory = alloc(0x10);
                text = memory ? ((void *(__thiscall *)(void *, const void *))0xbcf7d0)(memory, characters) : nullptr;
            } else {
                text = ((void *(__fastcall *)(uint32_t))0xbd0490)(1);
            }
            *out = (Text *)text;
            ((void(__thiscall *)(void *))0xbd1880)(reference);
            let_go(&built);
            return out;
        }
        ((void(__thiscall *)(void *))0xbd1880)(reference);
        let_go(&built);
    }
own:
    *out = *name;
    ++(*out)->count;
    return out;
}

template <int K>
Text **__thiscall default_name_copy(uint8_t *self, Text **out) {
    return default_name(self, out, default_names[K]);
}
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(default_names[0].at == 0xad7e10 && default_names[1].at == 0xaf4150, "default_names.inc changed");
REWRITE("Designer::DefaultName", 0xad7e10, default_names_first[0], (&default_name_copy<0>), &g_default_names);
REWRITE("Designer::DefaultName", 0xaf4150, default_names_first[1], (&default_name_copy<1>), &g_default_names);
