// Text::Decimal49D650, 49D7D0, 49D950, 49DAD0: one function written out four times (tools/family.py checks they differ
// only in the function that makes their unit's text, decimal_texts.inc). Given where the text goes (ecx) and a value (a
// float, on the stack): the value written with "%.2f" (the game's sprintf), its decimal point given the language's own
// (0x49D480), then a space, then the unit's text.
#include "rewrite.h"

namespace {
struct DecimalText {
    uint32_t at, unit;
};
#define DECIMAL_TEXTS(at, unit, ...) {at, unit},
constexpr DecimalText decimal_texts[] = {
#include "decimal_texts.inc"
};
#undef DECIMAL_TEXTS
#define DECIMAL_TEXTS(at, unit, ...) {__VA_ARGS__},
const uint8_t decimal_texts_first[][7] = {
#include "decimal_texts.inc"
};
#undef DECIMAL_TEXTS

typedef void(__cdecl *Free)(void *);
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

volatile long g_decimal_texts;
int32_t **decimal_text(int32_t **out, float value, const DecimalText &kind) {
    g_decimal_texts++;
    char written[0x80];
    ((int(__cdecl *)(char *, const char *, ...))0xddcdb5)(written, (const char *)0xf30590, (double)value);   // "%.2f"
    ((void(__thiscall *)(char *))0x49d480)(written);   // the language's decimal point
    int32_t *volatile unit;
    void *unit_made = ((void *(__thiscall *)(void *))(uintptr_t)kind.unit)((void *)&unit);
    void *memory = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x10);
    int32_t *space_text = memory ? ((int32_t * (__thiscall *)(void *, uintptr_t))0xbcf850)(memory, 0xf28e54) : nullptr;   // " "
    int32_t *volatile space = space_text;
    int32_t *volatile number;
    void *made = ((void *(__fastcall *)(void *, const char *, void *))0x403ce0)((void *)&number, written, (void *)&space);   // the value, then the space
    ((void *(__thiscall *)(void *, void *, void *))0x403bb0)(made, (void *)out, unit_made);
    let_go(&number);
    if (--*space_text == 0) free_text(space_text);   // (through the pointer kept, not read again)
    let_go(&unit);
    return out;
}

template <int K>
int32_t **__thiscall decimal_text_copy(int32_t **out, float value) {
    return decimal_text(out, value, decimal_texts[K]);
}
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(decimal_texts[0].at == 0x49d650 && decimal_texts[1].at == 0x49d7d0 && decimal_texts[2].at == 0x49d950 && decimal_texts[3].at == 0x49dad0,
              "decimal_texts.inc changed");
REWRITE("Text::DecimalWithUnit", 0x49d650, decimal_texts_first[0], (&decimal_text_copy<0>), &g_decimal_texts);
REWRITE("Text::DecimalWithUnit", 0x49d7d0, decimal_texts_first[1], (&decimal_text_copy<1>), &g_decimal_texts);
REWRITE("Text::DecimalWithUnit", 0x49d950, decimal_texts_first[2], (&decimal_text_copy<2>), &g_decimal_texts);
REWRITE("Text::DecimalWithUnit", 0x49dad0, decimal_texts_first[3], (&decimal_text_copy<3>), &g_decimal_texts);
