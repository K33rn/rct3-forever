// Path::Make731220, Guest::Make739540 and Graph::MakeAC3270 (one function written out three times; they differ in the
// object's size and constructor and where it keeps its name): make a named object (fastcall: where its name's text is
// held, a value; gives the object).
//   - The object (the copy's size in new bytes, its constructor with 0, 0, 0; the game goes on with none, as it does).
//   - Its name (+0xC0, or +0xB4) replaced by the one given unless it is that one already (the old one let go, read
//     again, as the game does; the new held once more); the value kept just after it.
//   - Readied (0x800860) unless marked (+0x2C bit 0).
#include "rewrite.h"

namespace {
struct Text {
    int32_t count;
    uint32_t length, room;
    uint16_t *characters;
};
typedef void(__cdecl *Free)(void *);

volatile long g_path_makes;
template <uint32_t SIZE, uintptr_t MAKE, uint32_t NAME> uint8_t *__fastcall path_make(Text **name, uint32_t value) {
    g_path_makes++;
    uint8_t *object = nullptr;
    if (void *memory = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(SIZE))
        object = (uint8_t *)((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))MAKE)(memory, 0, 0, 0);
    Text *old = *(Text **)(object + NAME);
    if (*name != old) {
        --old->count;
        Text *held = *(Text *volatile *)(object + NAME);
        if (held->count == 0 && held) {
            ((Free)0xbce010)(held->characters);
            ((Free)0xbce000)(held);
        }
        *(Text **)(object + NAME) = *name;
        ++(*name)->count;
    }
    *(uint32_t *)(object + NAME + 4) = value;
    if (!(object[0x2c] & 1)) ((void(__thiscall *)(void *))0x800860)(object);
    return object;
}

const uint8_t path_make_first[][7] = {{0x6a, 0xff, 0x68, 0x8b, 0xca, 0xe4, 0x00}, {0x6a, 0xff, 0x68, 0x7b, 0xd6, 0xe4, 0x00}, {0x6a, 0xff, 0x68, 0x8b, 0x88, 0xe8, 0x00}};
}   // namespace
REWRITE("Object::MakeNamed", 0x731220, path_make_first[0], (&path_make<0xbf8, 0x730db0, 0xc0>), &g_path_makes);
REWRITE("Object::MakeNamed", 0x739540, path_make_first[1], (&path_make<0x180, 0x739100, 0xc0>), &g_path_makes);
REWRITE("Object::MakeNamed", 0xac3270, path_make_first[2], (&path_make<0x7c8, 0xac2ed0, 0xb4>), &g_path_makes);
