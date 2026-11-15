// Window::Fill680160 and Window::FillA06FC0 (one function written out twice; they differ only in their exception
// handler): fill a file window's list with the files in its folder.
//   - The window filled (0x687DF0) and its list (+0x168) emptied (0x685AC0).
//   - The folder's path (0x687720) made into a narrow C string in place, after the wide text in the same buffer (as
//     the game's own text code does it, inlined: the wide text ended with a 0, the buffer made big enough for
//     3 bytes a character, the low byte of each character copied up to the first 0), and the folder made
//     (CreateDirectoryA through the game's import, with no security attributes). The path let go.
//   - The file pattern (0x6879A0). The folder (+0x198) searched for it twice, each time with copies of the pattern
//     and the folder (0xBD01C0) given to a search (0xBE5570; the copies let go, the pattern's first): first to count
//     the entries that are not folders (the search's object at +4: slot 4 more?, slot 0x10 a folder?, slot 8 next),
//     the list told the count (0x685A20); then each such entry's name (0x4042D0, copied with 0xBCFEF0) added to the
//     list at its number with the pattern and 1 (0x68C370). Each search ends with 0xBE47B0.
//   - The list redrawn (its slot 0xE4) and the pattern let go.
#include "rewrite.h"

namespace {
typedef void(__cdecl *Free)(void *);
// a shared text {count, length, room, characters} let go: the count taken down (read again, as the game does), at 0
// its characters freed and then it
inline void let_go(int32_t *volatile *held) {
    --**held;
    int32_t *text = *held;
    if (*(volatile int32_t *)text == 0) {
        ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
        ((Free)0xbce000)(text);
    }
}
typedef void(__thiscall *Unique)(void *, uint32_t);   // WideString::MakeUnique, with the room wanted
// The game's narrow copy of a wide text, made just after its characters in the same buffer; gives it.
const char *narrow_in_place(int32_t *volatile *held) {
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

typedef uint8_t(__thiscall *Ask)(void *);
inline uint8_t ask(void *object, uint32_t slot) { return ((Ask)(*(void ***)object)[slot / 4])(object); }
typedef void *(__thiscall *Make)(void *, void *);

volatile long g_file_lists;
void __fastcall file_list(uint8_t *self) {
    g_file_lists++;
    ((void(__thiscall *)(void *))0x687df0)(self);
    ((void(__thiscall *)(void *))0x685ac0)(*(void **)(self + 0x168));
    int32_t *volatile path, *volatile pattern;
    {
        int32_t *volatile *held = (int32_t * volatile *)((Make)0x687720)(self, (void *)&path);
        const char *narrow = narrow_in_place(held);
        typedef int(__stdcall * MakeFolder)(const char *, void *);
        (*(MakeFolder *)(uintptr_t)0xf2615c)(narrow, nullptr);   // CreateDirectoryA
        let_go(&path);
    }
    ((Make)0x6879a0)(self, (void *)&pattern);
    void *folder = self + 0x198;
    for (int round = 0; round < 2; round++) {
        int32_t *volatile folder_copy, *volatile pattern_copy;
        ((Make)0xbd01c0)((void *)&folder_copy, folder);
        ((Make)0xbd01c0)((void *)&pattern_copy, (void *)&pattern);
        void *volatile search[2];
        ((void(__thiscall *)(void *, void *, void *))0xbe5570)((void *)search, (void *)&pattern_copy, (void *)&folder_copy);
        let_go(&pattern_copy);
        let_go(&folder_copy);
        uint32_t count = 0;
        while (ask(search[1], 4)) {
            if (!ask(search[1], 0x10)) {
                if (round == 0) {
                    count++;
                } else {
                    int32_t *volatile name, *volatile copy;
                    void *got = ((Make)0x4042d0)((void *)search, (void *)&name);
                    ((Make)0xbcfef0)((void *)&copy, got);
                    ((void(__thiscall *)(void *, uint32_t, void *, void *, uint32_t))0x68c370)(*(void **)(self + 0x168), count, (void *)&pattern,
                                                                                               (void *)&copy, 1);
                    count++;
                    let_go(&copy);
                    let_go(&name);
                }
            }
            ask(search[1], 8);
        }
        ((void(__thiscall *)(void *))0xbe47b0)((void *)search);
        if (round == 0) ((void(__thiscall *)(void *, uint32_t))0x685a20)(*(void **)(self + 0x168), count);
    }
    void *list = *(void **)(self + 0x168);
    ((void(__thiscall *)(void *))(*(void ***)list)[0xe4 / 4])(list);
    let_go(&pattern);
}

const uint8_t file_lists_first[][7] = {{0x6a, 0xff, 0x68, 0xc0, 0x9b, 0xe3, 0x00}, {0x6a, 0xff, 0x68, 0x60, 0x8d, 0xe7, 0x00}};
}   // namespace
REWRITE("Window::FillFiles", 0x680160, file_lists_first[0], &file_list, &g_file_lists);
REWRITE("Window::FillFiles", 0xa06fc0, file_lists_first[1], &file_list, &g_file_lists);
