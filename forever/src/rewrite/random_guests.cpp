// Guest::PickRandom79EB70 and Guest::PickRandom7E93D0 (one function written out twice; they differ in the guest's
// kind, 5 or 6, its size and constructor, the counter its number comes from, and the order of two stores): make a
// guest of that kind at a random place (fastcall: a kind wish; gives the guest, or 0 when no place was found).
//   - The terrain manager ([0x1311650]) made when there is none (0x98 bytes, 0x8D63A0 with 0, 0, 0; readied by
//     0x800860 unless marked (+0x2C bit 0); 0x8D41B0 with 90, 90); readied when not marked; its map (+0x54).
//   - A wish outside 0-1 draws a random number that is not used (Random::Unit).
//   - The places: 0x7C2FF0 with the kind and 0 into five; none: 0. The path manager ([0x130625C]) made when there is
//     none (0xD0 bytes, 0x7232C0 with 0, 0, 0; readied unless marked); a spot from it (0x71EF40 with 0, 0, the map's
//     width - 1 and height - 1 as bytes, 1). The two are pushed as whole dwords whose upper three bytes are what the
//     registers held: the width's those of 0x7C2FF0's answer (or, when the path manager was just made, 0 if it was
//     marked, else 0x800860's answer), the height's those of the wish.
//   - The guest (the size's new bytes, the constructor with 0, 0, 0; never without memory): its five references
//     (+0x124, 0x14 apart) let go of what they held (taken out of their handle's list: a broken list stops the game,
//     int3) and pointed at the five places (Handle::For, first in the list; then told, their slot 0).
//   - Its kind (+0x120) and 0 (+0x121); +0x1B4 the game's +0x48 ([0x1106C54], or 0x444090 when there is none); its
//     number: the counter as it was (+0x4FC; the counter one more first); its name "%ME%" + " <number>" (sprintf of
//     " %d" with the counter; the two texts joined by value into 0x7CD6E0; both let go).
//   - With a spot: its place (0x726930) put as the only one in the guest's list of places (+0x198: 0x7E5110 with 1,
//     then the place's three floats copied, the index +0x1A0 made 0).
//   - Random values (Random::Unit u, on the FPU, each rounded where stored): +0x100 = u * 2.5 - 1.25, +0x108 the same;
//     its matrix (+0x50) made the identity; with a spot, its position (+0x80) the spot's place (the list's entry at
//     the index) plus +0x100, +0x104, +0x108; +0x110 = u * 2pi; +0xC0 a random byte's low four bits; +0xC1 - +0xC5
//     made 0; +0x4DC and +0x4D8 = u * 0.8 + 0.1.
//   - Readied when not marked; the guest given.
#include "rewrite.h"

namespace {
typedef long double ext;
inline float rf(ext x) {
    volatile float f = (float)x;
    return f;
}
struct Text {
    int32_t count;
    uint32_t length, room;
    uint16_t *characters;
};
typedef void *(__cdecl *Alloc)(uint32_t);
typedef void(__cdecl *Free)(void *);
typedef uint32_t(__thiscall *Ready)(void *);
typedef long double(__cdecl *Unit)();
inline uint32_t ready_unless_marked(uint8_t *object, bool *called) {
    if (object[0x2c] & 1) return 0;
    if (called) *called = true;
    return ((Ready)0x800860)(object);
}
inline float game_float(uintptr_t at) { return *(const float *)at; }

volatile long g_random_guests;
template <uint8_t KIND, uint32_t SIZE, uintptr_t MAKE, uintptr_t COUNTER, bool BYTES_LATE> uint8_t *__fastcall random_guest(int32_t wish) {
    g_random_guests++;
    uint8_t *terrain = *(uint8_t *volatile *)(uintptr_t)0x1311650;
    if (!terrain) {
        if (void *memory = ((Alloc)0xbcdfc0)(0x98)) ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))0x8d63a0)(memory, 0, 0, 0);
        ready_unless_marked(*(uint8_t *volatile *)(uintptr_t)0x1311650, nullptr);
        ((void(__thiscall *)(void *, uint32_t, uint32_t))0x8d41b0)(*(void *volatile *)(uintptr_t)0x1311650, 0x5a, 0x5a);
        terrain = *(uint8_t *volatile *)(uintptr_t)0x1311650;
    }
    ready_unless_marked(terrain, nullptr);
    const uint8_t *map = *(const uint8_t **)(terrain + 0x54);
    if (wish < 0 || wish >= 2) (void)((Unit)0x475180)();
    void *places[5];
    const uint32_t found = ((uint32_t(__fastcall *)(uint32_t, uint32_t, void **))0x7c2ff0)(KIND, 0, places);
    if (!(uint8_t)found) return nullptr;
    uint32_t width_bits = found & 0xffffff00u;
    uint8_t *paths = *(uint8_t *volatile *)(uintptr_t)0x130625c;
    if (!paths) {
        void *memory = ((Alloc)0xbcdfc0)(0xd0);
        paths = memory ? (uint8_t *)((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))0x7232c0)(memory, 0, 0, 0) : nullptr;
        *(uint8_t *volatile *)(uintptr_t)0x130625c = paths;
        width_bits = ready_unless_marked(paths, nullptr) & 0xffffff00u;
        paths = *(uint8_t *volatile *)(uintptr_t)0x130625c;
    }
    const uint32_t width = width_bits | (uint8_t)(map[4] - 1), height = ((uint32_t)wish & 0xffffff00u) | (uint8_t)(map[5] - 1);
    void *spot = ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))0x71ef40)(paths, 0, 0, width, height, 1);
    uint8_t *guest = nullptr;
    if (void *memory = ((Alloc)0xbcdfc0)(SIZE)) guest = (uint8_t *)((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))MAKE)(memory, 0, 0, 0);
    for (int i = 0; i < 5; i++) {
        uint32_t *link = (uint32_t *)(guest + 0x12c + 0x14 * i);   // (the reference's list link; its handle just before)
        uint32_t *next = (uint32_t *)(uintptr_t)link[0], *head = (uint32_t *)(uintptr_t)link[1];
        if (!(head && next && next[1] == (uint32_t)(uintptr_t)link && head[0] == (uint32_t)(uintptr_t)link)) __builtin_trap();
        next[1] = (uint32_t)(uintptr_t)head;
        ((uint32_t *)(uintptr_t)link[1])[0] = link[0];
        link[0] = 0, link[1] = 0;
        ((void(__thiscall *)(void *, uint32_t))0x8055c0)((void *)(uintptr_t)link[-1], 0);
        uint8_t *handle = ((uint8_t * (__fastcall *)(void *))0x805700)(places[i]);
        link[-1] = (uint32_t)(uintptr_t)handle;
        uint32_t *list = *(uint32_t **)(handle + 0x20);
        link[0] = list[0];
        ((uint32_t *)(uintptr_t)list[0])[1] = (uint32_t)(uintptr_t)link;
        link[1] = (uint32_t)(uintptr_t)list;
        list[0] = (uint32_t)(uintptr_t)link;
        uint8_t *ref = (uint8_t *)(link - 2);
        ((void(__thiscall *)(void *))(*(void ***)ref)[0])(ref);
    }
    guest[0x120] = KIND;
    guest[0x121] = 0;
    uint8_t *game = *(uint8_t *volatile *)(uintptr_t)0x1106c54;
    if (!game) game = ((uint8_t * (__cdecl *)())0x444090)();
    *(uint32_t *)(guest + 0x1b4) = *(uint32_t *)(game + 0x48);
    const uint32_t number = *(volatile uint32_t *)COUNTER;
    *(volatile uint32_t *)COUNTER = number + 1;
    *(uint32_t *)(guest + 0x4fc) = number;
    char digits[0x18];
    ((int(__cdecl *)(char *, const char *, ...))0xddcdb5)(digits, (const char *)0xf42344, number + 1);   // " %d"
    Text *name = nullptr;
    if (void *memory = ((Alloc)0xbcdfc0)(0x10)) name = ((Text * (__thiscall *)(void *, const char *))0xbcf850)(memory, digits);
    Text *volatile held_name = name;
    Text *volatile me = nullptr;
    if (void *memory = ((Alloc)0xbcdfc0)(0x10)) me = ((Text * (__thiscall *)(void *, const char *))0xbcf850)(memory, (const char *)0xf31f48);   // "%ME%"
    {
        Text *volatile joined;
        ((void *(__thiscall *)(void *, void *, void *))0x403bb0)((void *)&me, (void *)&joined, (void *)&held_name);
        ((void(__thiscall *)(void *, Text *))0x7cd6e0)(guest, joined);   // (given by value: it lets it go)
    }
    --me->count;
    if (*(volatile int32_t *)&me->count == 0) {
        ((Free)0xbce010)(me->characters);
        ((Free)0xbce000)(me);
    }
    if (--name->count == 0) {
        ((Free)0xbce010)(name->characters);
        ((Free)0xbce000)(name);
    }
    if (spot) {
        float place[3];
        const uint32_t *at = (const uint32_t *)((void *(__thiscall *)(void *, void *))0x726930)(spot, place);
        uint8_t *places_list = guest + 0x198;
        ((void(__thiscall *)(void *, uint32_t))0x7e5110)(places_list, 1);
        uint32_t *first = *(uint32_t **)places_list;
        first[0] = at[0], first[1] = at[1], first[2] = at[2];
        *(uint16_t *)(places_list + 8) = 0;
    }
    const float scale = game_float(0xf2666c), shift = game_float(0xf268e0);   // 2.5, 1.25
    *(volatile float *)(guest + 0x100) = rf(((Unit)0x475180)() * scale - shift);
    *(volatile float *)(guest + 0x108) = rf(((Unit)0x475180)() * scale - shift);
    static const uint32_t zeros[] = {0x88, 0x84, 0x80, 0x7c, 0x74, 0x70, 0x6c, 0x68, 0x60, 0x5c, 0x58, 0x54}, ones[] = {0x8c, 0x78, 0x64, 0x50};
    for (uint32_t off : zeros) *(uint32_t *)(guest + off) = 0;
    for (uint32_t off : ones) *(uint32_t *)(guest + off) = 0x3f800000;   // (the matrix made the identity)
    if (spot) {
        const float *entry = (const float *)(*(uint8_t **)(guest + 0x198) + 16 * *(int16_t *)(guest + 0x1a0));
        const float x = rf((ext) * (float *)(guest + 0x100) + entry[0]), y = rf((ext) * (float *)(guest + 0x104) + entry[1]);
        const float z = rf((ext) * (float *)(guest + 0x108) + entry[2]);
        *(volatile float *)(guest + 0x88) = z;
        *(volatile float *)(guest + 0x80) = x;
        *(volatile float *)(guest + 0x84) = y;
    }
    const ext turn = ((Unit)0x475180)() * game_float(0xf268dc);   // 2pi
    if (!BYTES_LATE) guest[0xc1] = 0, guest[0xc2] = 0;
    *(volatile float *)(guest + 0x110) = rf(turn);
    guest[0xc0] = (uint8_t)(((uint32_t(__cdecl *)())0x4751e0)() & 0xf);
    if (BYTES_LATE) guest[0xc1] = 0, guest[0xc2] = 0;
    guest[0xc3] = 0, guest[0xc4] = 0, guest[0xc5] = 0;
    const float tint = game_float(0xf28420), base = game_float(0xf270b0);   // 0.8, 0.1
    *(volatile float *)(guest + 0x4dc) = rf(((Unit)0x475180)() * tint + base);
    *(volatile float *)(guest + 0x4d8) = rf(((Unit)0x475180)() * tint + base);
    ready_unless_marked(guest, nullptr);
    return guest;
}

const uint8_t random_guest_first[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x6a};
}   // namespace
REWRITE("Guest::MakeRandom", 0x79eb70, random_guest_first, (&random_guest<5, 0x560, 0x79e5b0, 0x1306c14, true>), &g_random_guests);
REWRITE("Guest::MakeRandom", 0x7e93d0, random_guest_first, (&random_guest<6, 0x510, 0x7e91e0, 0x130a870, false>), &g_random_guests);
