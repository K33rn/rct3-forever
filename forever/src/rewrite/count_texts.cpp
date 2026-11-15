// Objectives::CountText6DD800, 6DDB30 and 6DDE60 (one function written out three times; tools/family.py:
// count_texts.inc; they differ in the text they start from): an objective's text with its count (thiscall: the place
// for the text; gives it).
//   - The copy's text (0x6DD5F0, 0x6DD920, 0x6DDC50: made in a text of its own).
//   - The count (+0x70) printed with ": %d" (the game's sprintf, into 32 bytes) and added on as the game's text code
//     does it inline: room for the length + the digits + 1 (WideString::MakeUnique), each byte widened up to the
//     first 0 or the digits' count, a 0 after, the length raised by the digits' count.
//   - The text taken into the place given and its own let go.
// (The game's stack cookie check guards only its own frame.)
#include "rewrite.h"

namespace {
struct CountText {
    uint32_t at, base;
};
#define COUNT_TEXTS(at, base, ...) {at, base},
constexpr CountText count_texts[] = {
#include "count_texts.inc"
};
#undef COUNT_TEXTS
#define COUNT_TEXTS(at, base, ...) {__VA_ARGS__},
const uint8_t count_texts_first[][7] = {
#include "count_texts.inc"
};
#undef COUNT_TEXTS
typedef void(__cdecl *Free)(void *);

volatile long g_count_texts;
int32_t **count_text(uint8_t *self, int32_t **out, const CountText &kind) {
    g_count_texts++;
    int32_t *volatile text;
    ((void *(__thiscall *)(void *))kind.base)((void *)&text);
    char digits[32];
    ((int(__cdecl *)(char *, const char *, ...))0xddcdb5)(digits, (const char *)0xf65674, *(volatile int32_t *)(self + 0x70));   // ": %d"
    uint32_t n = 0;
    while (digits[n]) n++;
    ((void(__thiscall *)(void *, uint32_t))0xbcffe0)((void *)&text, (uint32_t)text[1] + n + 1);
    uint16_t *end = (uint16_t *)(uintptr_t)(uint32_t)text[3] + (uint32_t)text[1];
    uint32_t i = 0;
    for (; i < n && digits[i]; i++) end[i] = (uint8_t)digits[i];
    end[i] = 0;
    text[1] += (int32_t)n;
    int32_t *made = text;
    *out = made;
    ++*made;
    --*text;
    int32_t *again = text;   // read again, as the game does
    if (*(volatile int32_t *)again == 0) {
        ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)again[3]);
        ((Free)0xbce000)(again);
    }
    return out;
}

template <int K> int32_t **__thiscall count_text_copy(uint8_t *self, int32_t **out) { return count_text(self, out, count_texts[K]); }
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(count_texts[0].at == 0x6dd800 && count_texts[1].at == 0x6ddb30 && count_texts[2].at == 0x6dde60, "count_texts.inc changed");
REWRITE("Objectives::CountText", 0x6dd800, count_texts_first[0], (&count_text_copy<0>), &g_count_texts);
REWRITE("Objectives::CountText", 0x6ddb30, count_texts_first[1], (&count_text_copy<1>), &g_count_texts);
REWRITE("Objectives::CountText", 0x6dde60, count_texts_first[2], (&count_text_copy<2>), &g_count_texts);
