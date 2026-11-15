// Name::FindADFF60 and Name::FindAF73A0 (one function written out twice; they differ in the class number's global, the
// two objects' sizes and constructors and the call that joins them): make an object for a name (fastcall: where the
// name's text is held; gives the object made, or 0).
//   - The name made lower-case (WideString::ToLower, 0xCD27D0, into a text here).
//   - The name looked up in the game's names ([0x1341408], Name::Find9ACF50; the text given by value, held once
//     more): with what it finds giving the copy's interface (its slot 1's class info at the copy's class number, as
//     ride_checks.cpp's), an object of the first size made from it (none without memory).
//   - When that gave none: the lower-case name loaded (ManagedObject::LoadFile, 0x803D30, into a reference here), the
//     interface of the object its handle (+4) holds (+0xC) taken; the reference destroyed (0x40F730); none: the
//     lower-case text let go and 0.
//   - An object of the second size made (the constructor with 0, 0, 0; none without memory) and given the first (the
//     copy's call; on none too); the lower-case text let go; the second object given.
#include "rewrite.h"

namespace {
struct Text {
    int32_t count;
    uint32_t length, room;
    uint16_t *characters;
};
typedef void *(__cdecl *Alloc)(uint32_t);
typedef void(__cdecl *Free)(void *);
inline void let_go(Text *volatile *held) {
    --(*held)->count;
    Text *text = *held;
    if (*(volatile int32_t *)&text->count == 0) {
        ((Free)0xbce010)(text->characters);
        ((Free)0xbce000)(text);
    }
}
inline uint8_t *interface_of(uint8_t *object, uint32_t class_global) {   // an object's interface by its class info (none: 0)
    if (!object) return nullptr;
    uint8_t *info = ((uint8_t * (__thiscall *)(void *))(*(void ***)object)[1])(object);
    if (!info) return nullptr;
    const int32_t number = *(volatile int32_t *)(uintptr_t)class_global;
    if (number >= *(int32_t *)(info + 0x18)) return nullptr;
    uint8_t *entry = (*(uint8_t ***)(info + 0x14))[number];
    if (!entry) return nullptr;
    return object + *(int32_t *)(entry + 0x10);
}

volatile long g_name_makes;
template <uint32_t CLASS, uint32_t FIRST_SIZE, uintptr_t FIRST, uint32_t SECOND_SIZE, uintptr_t SECOND, uintptr_t JOIN>
void *__fastcall name_make(Text **name) {
    g_name_makes++;
    Text *volatile lower;
    ((void *(__fastcall *)(void *, void *))0xcd27d0)((void *)&lower, name);
    Text *given = *name;
    ++given->count;
    uint8_t *found = (uint8_t *)((void *(__thiscall *)(void *, Text *))0x9acf50)(*(void *volatile *)(uintptr_t)0x1341408, given);
    void *made = nullptr;
    if (uint8_t *face = interface_of(found, CLASS)) {
        if (void *memory = ((Alloc)0xbcdfc0)(FIRST_SIZE)) made = ((void *(__thiscall *)(void *, void *))FIRST)(memory, face);
    }
    if (!made) {
        alignas(4) uint8_t reference[0x20];
        uint8_t *loaded = (uint8_t *)((void *(__fastcall *)(void *, void *))0x803d30)(reference, (void *)&lower);
        made = interface_of(*(uint8_t **)(*(uint8_t **)(loaded + 4) + 0xc), CLASS);
        ((void(__thiscall *)(void *))0x40f730)(reference);
        if (!made) {
            let_go(&lower);
            return nullptr;
        }
    }
    void *second = nullptr;
    if (void *memory = ((Alloc)0xbcdfc0)(SECOND_SIZE)) second = ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))SECOND)(memory, 0, 0, 0);
    ((void(__thiscall *)(void *, void *))JOIN)(second, made);
    let_go(&lower);
    return second;
}

const uint8_t name_make_first[][7] = {{0x6a, 0xff, 0x68, 0x96, 0xb8, 0xe8, 0x00}, {0x6a, 0xff, 0x68, 0xb6, 0xe2, 0xe8, 0x00}};
}   // namespace
REWRITE("Name::MakeObject", 0xadff60, name_make_first[0], (&name_make<0x13ce508, 0xf0, 0xad95a0, 0xc130, 0xadf4e0, 0xadfc50>), &g_name_makes);
REWRITE("Name::MakeObject", 0xaf73a0, name_make_first[1], (&name_make<0x13ce628, 0xc0, 0xaf6480, 0x110, 0xaf4be0, 0xaf5210>), &g_name_makes);
