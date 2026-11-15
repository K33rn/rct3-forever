// Name::FindCC9290 and Name::FindCEF210 (one function written out twice; they differ in the table: at +0x30 with 8
// bytes an entry, or at +0x34 with 4): find a name in a table sorted by name (thiscall; where the name's text is held;
// gives its index, or -1).
//   - A binary search over the entries (the count at +0x2C): the middle one's C name made a text (0x10 new bytes,
//     WideString from C; none without memory; the shared empty text when it has no name), it and the name given
//     narrowed in place (as scenery_scans.cpp's narrow_in_place), compared case ignored (0xCD4880, the name given
//     first): after it, the half above; equal, its index; before, the half below. The entry's text let go each time.
#include "rewrite.h"

namespace {
typedef void(__thiscall *Unique)(void *, uint32_t);
typedef void(__cdecl *Free)(void *);
inline void let_go(int32_t *volatile *held) {   // (read again, as the game does)
    --**held;
    int32_t *text = *held;
    if (*(volatile int32_t *)text == 0) {
        ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
        ((Free)0xbce000)(text);
    }
}
const char *narrow_in_place(int32_t *volatile *held) {   // as scenery_scans.cpp's
    int32_t *t = *held;
    uint32_t length = (uint32_t)t[1];
    if (!((uint32_t)t[2] > length && ((uint16_t *)(uintptr_t)(uint32_t)t[3])[length] == 0)) {
        ((Unique)0xbcffe0)((void *)held, length);
        t = *held;
        ((uint16_t *)(uintptr_t)(uint32_t)t[3])[(uint32_t)t[1]] = 0;
    }
    ((Unique)0xbcffe0)((void *)held, ((uint32_t)(*held)[1] + 1) * 3);
    t = *held;
    const uint32_t n = (uint32_t)t[1];
    const uint16_t *wide = (const uint16_t *)(uintptr_t)(uint32_t)t[3];
    char *narrow = (char *)(wide + n + 1);
    uint32_t i = 0;
    for (; i < n && wide[i]; i++) narrow[i] = (char)wide[i];
    narrow[i] = 0;
    t = *held;
    return (const char *)(uintptr_t)(uint32_t)t[3] + (uint32_t)t[1] * 2 + 2;
}

volatile long g_name_finds;
template <uint32_t TABLE, uint32_t STRIDE> int32_t __fastcall name_find(uint8_t *self, void *, int32_t *volatile *name) {
    g_name_finds++;
    int32_t low = 0, high = *(int32_t *)(self + 0x2c) - 1;
    if (high < 0) return -1;
    do {
        const int32_t middle = low + (high - low) / 2;
        const char *c = *(const char **)(*(uint8_t **)(self + TABLE) + (uint32_t)middle * STRIDE);
        int32_t *volatile entry;
        if (c) {
            void *memory = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x10);
            entry = memory ? ((int32_t * (__thiscall *)(void *, const char *))0xbcf850)(memory, c) : nullptr;
        } else {
            entry = ((int32_t * (__fastcall *)(uint32_t))0xbd0490)(1);
        }
        const char *entry_narrow = narrow_in_place(&entry);
        const char *name_narrow = narrow_in_place(name);
        const int32_t order = ((int32_t(__stdcall *)(const char *, const char *))0xcd4880)(name_narrow, entry_narrow);
        if (order > 0) {
            low = middle + 1;
        } else if (order == 0) {
            let_go(&entry);
            return middle;
        } else {
            high = middle - 1;
        }
        let_go(&entry);
    } while (low <= high);
    return -1;
}

const uint8_t name_find_first[][7] = {{0x6a, 0xff, 0x68, 0x73, 0x68, 0xea, 0x00}, {0x6a, 0xff, 0x68, 0x33, 0x78, 0xea, 0x00}};
}   // namespace
REWRITE("Name::FindSorted", 0xcc9290, name_find_first[0], (&name_find<0x30, 8>), &g_name_finds);
REWRITE("Name::FindSorted", 0xcef210, name_find_first[1], (&name_find<0x34, 4>), &g_name_finds);
