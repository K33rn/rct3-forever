// Money::Format, Money::Format (2) and (3): one function written out three times (tools/family.py checks they differ only
// in the three parameters in money_formats.inc). Given a number (ecx: where the text goes, the number in edx):
//   - below 10, the text of the number's entry in the copy's table of named texts (p0) is copied (the shared empty text
//     when it has none);
//   - otherwise a name is made: "Money", then the game's language (0x453630), then the copy's letter (p1: "a", "d" or
//     "b"); when the game's text table (0x1303CFC) has a text by that name (0x410C90; its flag, 0xBD1420), that text is
//     copied; otherwise the text of the copy's default named text (p2) is.
#include "rewrite.h"

namespace {
struct MoneyFormat {
    uint32_t at, table, letter, fallback;
};
#define MONEY_FORMATS(at, table, letter, fallback, ...) {at, table, letter, fallback},
constexpr MoneyFormat money_formats[] = {
#include "money_formats.inc"
};
#undef MONEY_FORMATS
#define MONEY_FORMATS(at, table, letter, fallback, ...) {__VA_ARGS__},
const uint8_t money_formats_first[][7] = {
#include "money_formats.inc"
};
#undef MONEY_FORMATS

typedef void *(__cdecl *Alloc)(uint32_t);
typedef void(__cdecl *Free)(void *);
const Alloc alloc = (Alloc)0xbcdfc0;
inline void free_text(int32_t *text) {
    ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
    ((Free)0xbce000)(text);
}
// a shared text {count, ..., buffer at +0xC} let go: the count taken down; at 0 its buffer freed, then it
inline void let_go(int32_t *volatile *held) {
    --**held;
    int32_t *text = *held;   // read again, as the game does
    if (*(volatile int32_t *)text == 0) free_text(text);
}
inline int32_t *text_c(uintptr_t c_text) {
    void *memory = alloc(0x10);
    return memory ? ((int32_t * (__thiscall *)(void *, uintptr_t))0xbcf850)(memory, c_text) : nullptr;
}
// a copy of a named text's characters (+8), or the shared empty text when it has none
inline int32_t *copy_of(const uint8_t *named) {
    const void *characters = *(const void *volatile *)(named + 8);
    if (!characters) return ((int32_t * (__fastcall *)(uint32_t))0xbd0490)(1);
    void *memory = alloc(0x10);
    return memory ? ((int32_t * (__thiscall *)(void *, const void *))0xbcf7d0)(memory, characters) : nullptr;
}
inline void *named_text(void *into, int32_t *volatile *name) {
    return ((void *(__thiscall *)(void *, void *, uint32_t, uint32_t))0x410c90)(into, (void *)name, 0x1303cfc, 1);
}

volatile long g_money_formats;
int32_t **money_format(int32_t **out, int32_t number, const MoneyFormat &kind) {
    g_money_formats++;
    if (number < 10) {
        *out = copy_of(*(const uint8_t *volatile *)(uintptr_t)(kind.table + 4 * number));
        return out;
    }
    int32_t *letter_text = text_c(kind.letter);
    int32_t *volatile letter = letter_text;
    int32_t *volatile language;
    void *language_made = ((void *(__thiscall *)(void *))0x453630)((void *)&language);
    int32_t *volatile money = text_c(0xf4cd98);   // "Money"
    int32_t *volatile first, *volatile name;
    void *joined = ((void *(__thiscall *)(void *, void *, void *))0x403bb0)((void *)&money, (void *)&first, language_made);
    ((void *(__thiscall *)(void *, void *, void *))0x403bb0)(joined, (void *)&name, (void *)&letter);
    let_go(&first);
    let_go(&money);
    let_go(&language);
    if (--*letter_text == 0) free_text(letter_text);   // (through the pointer kept, not read again)
    int32_t *volatile copy;
    alignas(16) uint8_t found[0x40];
    ((void *(__thiscall *)(void *, void *))0xbd01c0)((void *)&copy, (void *)&name);
    void *made = named_text(found, &copy);
    bool there = ((uint8_t(__thiscall *)(void *, uint32_t))0xbd1420)(made, 0) != 0;
    ((void(__thiscall *)(void *))0xbd1880)(found);
    let_go(&copy);
    if (there) {
        ((void *(__thiscall *)(void *, void *))0xbd01c0)((void *)&copy, (void *)&name);
        made = named_text(found, &copy);
        *out = copy_of((const uint8_t *)made);
        ((void(__thiscall *)(void *))0xbd1880)(found);
        let_go(&copy);
    } else {
        *out = copy_of(*(const uint8_t *volatile *)(uintptr_t)kind.fallback);
    }
    let_go(&name);
    return out;
}

template <int K>
int32_t **__fastcall money_format_copy(int32_t **out, int32_t number) {
    return money_format(out, number, money_formats[K]);
}
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(money_formats[0].at == 0x6bd4c0 && money_formats[1].at == 0x6bd830 && money_formats[2].at == 0x6bdba0, "money_formats.inc changed");
REWRITE("Money::FormatNamed", 0x6bd4c0, money_formats_first[0], (&money_format_copy<0>), &g_money_formats);
REWRITE("Money::FormatNamed", 0x6bd830, money_formats_first[1], (&money_format_copy<1>), &g_money_formats);
REWRITE("Money::FormatNamed", 0x6bdba0, money_formats_first[2], (&money_format_copy<2>), &g_money_formats);
