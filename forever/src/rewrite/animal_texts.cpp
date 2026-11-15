// Animal::TextAD7110 and Animal::TextAF0150 (one function written out twice; they differ in the class number's global):
// an animal part's text (thiscall; where to put it; gives that place).
//   - The part's interface for the class (its own +0x108, looked up once and kept, +0x10C marking it looked up): the
//     object its owner (+0xF8) holds at +0xC, through its class info (slot 1): the entry for the class number (a game
//     global) if the info has that many, the object plus the entry's offset; none at any miss.
//   - With the interface: the place its slot 0x20 gives (handed a text to fill); without: a new empty text ("" made
//     with 0xBCF850, none without memory).
//   - The text there given out, held once more; then the empty text let go (when made), then the filled one (when
//     there was an interface): a let-go is a count taken down, at 0 its buffer and it freed.
#include "rewrite.h"

namespace {
struct Text {
    int32_t count;
    uint32_t length, room;
    uint16_t *characters;
};
typedef void(__cdecl *Free)(void *);

template <uintptr_t CLASS> uint8_t *interface_of_owner(uint8_t *self) {
    uint8_t *object = *(uint8_t **)(*(uint8_t **)(self + 0xf8) + 0xc);
    if (!object) return nullptr;
    uint8_t *info = ((uint8_t * (__thiscall *)(void *))(*(void ***)object)[1])(object);
    if (!info) return nullptr;
    const int32_t number = *(volatile int32_t *)CLASS;
    if (number >= *(int32_t *)(info + 0x18)) return nullptr;
    uint8_t *entry = (*(uint8_t ***)(info + 0x14))[number];
    if (!entry) return nullptr;
    return object + *(uint32_t *)(entry + 0x10);
}

volatile long g_animal_texts;
template <uintptr_t CLASS> Text **__fastcall animal_text(uint8_t *self, void *, Text **out) {
    g_animal_texts++;
    if (!self[0x10c]) {
        *(uint8_t **)(self + 0x108) = interface_of_owner<CLASS>(self);
        self[0x10c] = 1;
    }
    Text *filled = nullptr, *empty = nullptr;
    Text **from;
    bool from_interface;
    if (*(uint8_t *volatile *)(self + 0x108)) {
        if (!*(volatile uint8_t *)(self + 0x10c)) {   // (looked at again, as the game does: it is set by now)
            *(uint8_t **)(self + 0x108) = interface_of_owner<CLASS>(self);
            self[0x10c] = 1;
        }
        uint8_t *face = *(uint8_t **)(self + 0x108);
        from = ((Text * *(__thiscall *)(void *, Text **))(*(void ***)face)[0x20 / 4])(face, &filled);
        from_interface = true;
    } else {
        void *memory = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x10);
        empty = memory ? ((Text * (__thiscall *)(void *, const char *))0xbcf850)(memory, (const char *)0xf264ad) : nullptr;
        from = &empty;
        from_interface = false;
    }
    Text *given = *from;
    *out = given;
    ++given->count;
    if (!from_interface) {   // the empty text let go (the game reads through it as it is)
        if (--empty->count == 0) {
            ((Free)0xbce010)(empty->characters);
            ((Free)0xbce000)(empty);
        }
    } else {   // the filled one let go
        --filled->count;
        Text *again = *(Text *volatile *)&filled;
        if (again->count == 0) {
            ((Free)0xbce010)(again->characters);
            ((Free)0xbce000)(again);
        }
    }
    return out;
}

const uint8_t animal_text_first[][7] = {{0x6a, 0xff, 0x68, 0x4d, 0xb0, 0xe8, 0x00}, {0x6a, 0xff, 0x68, 0x6d, 0xd6, 0xe8, 0x00}};
}   // namespace
REWRITE("Animal::Text", 0xad7110, animal_text_first[0], (&animal_text<0x13ce508>), &g_animal_texts);
REWRITE("Animal::Text", 0xaf0150, animal_text_first[1], (&animal_text<0x13ce628>), &g_animal_texts);
