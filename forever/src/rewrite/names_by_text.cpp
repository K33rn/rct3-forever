// RideSeating::FromName, OpMode::FromName and RideAnimation::FromName2 (one function written out three times;
// tools/family.py: names_by_text.inc; they differ in the prefix, the names function and the count): the number of
// the kind whose name matches the text given (fastcall: the text's holder), case ignored; the count when none does.
//   - The prefix ("RideSeating_", "OpMode", "Anim": 0x10 new bytes, WideString from C; none without memory) and the
//     text joined (0x403BB0); the prefix let go.
//   - For each number from 0: the joined text made a narrow C string in place (again each time: the game's text
//     code inlined, as file_lists.cpp's) and compared with the kind's name (the copy's names function, fastcall with
//     the number) by the C library's _stricmp; the first that matches is the answer.
//   - The joined text let go.
#include "rewrite.h"

namespace {
struct NamesByText {
    uint32_t at, prefix, names, count;
};
#define NAMES_BY_TEXT(at, prefix, names, count, ...) {at, prefix, names, count},
constexpr NamesByText names_by_text[] = {
#include "names_by_text.inc"
};
#undef NAMES_BY_TEXT
#define NAMES_BY_TEXT(at, prefix, names, count, ...) {__VA_ARGS__},
const uint8_t names_by_text_first[][7] = {
#include "names_by_text.inc"
};
#undef NAMES_BY_TEXT

typedef void(__cdecl *Free)(void *);
inline void let_go(int32_t *volatile *held) {   // a shared text let go (read again, as the game does)
    --**held;
    int32_t *text = *held;
    if (*(volatile int32_t *)text == 0) {
        ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
        ((Free)0xbce000)(text);
    }
}
typedef void(__thiscall *Unique)(void *, uint32_t);   // WideString::MakeUnique, with the room wanted
// The game's narrow copy of a wide text, made just after its characters in the same buffer (as file_lists.cpp's).
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

volatile long g_names_by_text;
int32_t name_number(void *text, const NamesByText &kind) {
    g_names_by_text++;
    int32_t *volatile prefix, *volatile joined;
    void *memory = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x10);
    prefix = memory ? ((int32_t * (__thiscall *)(void *, uintptr_t))0xbcf850)(memory, kind.prefix) : nullptr;
    ((void *(__thiscall *)(void *, void *, void *))0x403bb0)((void *)&prefix, (void *)&joined, text);
    let_go(&prefix);
    for (int32_t i = 0; i < (int32_t)kind.count; i++) {
        const char *narrow = narrow_in_place(&joined);
        const char *name = ((const char *(__fastcall *)(int32_t))kind.names)(i);
        if (((int(__cdecl *)(const char *, const char *))0xde22df)(narrow, name) == 0) {
            let_go(&joined);
            return i;
        }
    }
    let_go(&joined);
    return (int32_t)kind.count;
}

template <int K> int32_t __fastcall name_number_copy(void *text) { return name_number(text, names_by_text[K]); }
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(names_by_text[0].at == 0x5538e0 && names_by_text[1].at == 0xbb42a0 && names_by_text[2].at == 0xbbfc20, "names_by_text.inc changed");
REWRITE("Names::FromText", 0x5538e0, names_by_text_first[0], (&name_number_copy<0>), &g_names_by_text);
REWRITE("Names::FromText", 0xbb42a0, names_by_text_first[1], (&name_number_copy<1>), &g_names_by_text);
REWRITE("Names::FromText", 0xbbfc20, names_by_text_first[2], (&name_number_copy<2>), &g_names_by_text);
