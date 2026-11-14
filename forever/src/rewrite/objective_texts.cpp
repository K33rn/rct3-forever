// Objectives::NumberText (three copies: 0x6E4580, 0x6E4BA0, EnclosureVisitorsText) and Objectives::MoneyText (three
// copies): one function in six, the two families checked by tools/family.py (number_texts.inc, money_texts.inc: each copy
// differs only in the function that makes its label). Given where the text goes (thiscall, one argument), it writes the
// objective's label (the copy's own function: a copy of its wide text), ": ", then the objective's number (+0x70)
// written out: in decimal (0xCD23D0) for NumberText, as an amount of money (0x6BE770) for MoneyText.
#include "rewrite.h"

namespace {
struct ObjectiveText {
    uint32_t at, label;
};
#define NUMBER_TEXTS(at, label, ...) {at, label},
#define MONEY_TEXTS(at, label, ...) {at, label},
constexpr ObjectiveText number_texts[] = {
#include "number_texts.inc"
};
constexpr ObjectiveText money_texts[] = {
#include "money_texts.inc"
};
#undef NUMBER_TEXTS
#undef MONEY_TEXTS
#define NUMBER_TEXTS(at, label, ...) {__VA_ARGS__},
#define MONEY_TEXTS(at, label, ...) {__VA_ARGS__},
const uint8_t number_texts_first[][7] = {
#include "number_texts.inc"
};
const uint8_t money_texts_first[][7] = {
#include "money_texts.inc"
};
#undef NUMBER_TEXTS
#undef MONEY_TEXTS

typedef void(__cdecl *Free)(void *);
// a shared text {count, ..., buffer at +0xC} let go: the count taken down; at 0 its buffer freed, then it
inline void let_go(int32_t *volatile *held) {
    --**held;
    int32_t *text = *held;   // read again, as the game does
    if (*(volatile int32_t *)text == 0) {
        ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
        ((Free)0xbce000)(text);
    }
}

volatile long g_objective_texts;
int32_t **objective_text(uint8_t *self, int32_t **out, const ObjectiveText &kind, bool money) {
    g_objective_texts++;
    int32_t *volatile number, *volatile label, *volatile labelled, *volatile whole;
    int32_t value = *(volatile int32_t *)(self + 0x70);
    void *written = money ? ((void *(__fastcall *)(void *, int32_t, uint32_t))0x6be770)((void *)&number, value, 0)
                          : ((void *(__fastcall *)(void *, int32_t))0xcd23d0)((void *)&number, value);
    void *made = ((void *(__thiscall *)(void *))(uintptr_t)kind.label)((void *)&label);
    void *joined = ((void *(__thiscall *)(void *, void *, uintptr_t))0x403be0)(made, (void *)&labelled, 0xf5e1bc);   // ": "
    ((void *(__thiscall *)(void *, void *, void *))0x403bb0)(joined, (void *)&whole, written);
    let_go(&labelled);
    let_go(&label);
    let_go(&number);
    *out = whole;
    ++*whole;
    let_go(&whole);
    return out;
}

template <int K>
int32_t **__thiscall number_text_copy(uint8_t *self, int32_t **out) {
    return objective_text(self, out, number_texts[K], false);
}
template <int K>
int32_t **__thiscall money_text_copy(uint8_t *self, int32_t **out) {
    return objective_text(self, out, money_texts[K], true);
}
}   // namespace
// (the addresses again, as numbers, checked against the tables)
static_assert(number_texts[0].at == 0x6e4580 && number_texts[1].at == 0x6e4ba0 && number_texts[2].at == 0x6e7060, "number_texts.inc changed");
static_assert(money_texts[0].at == 0x6dd000 && money_texts[1].at == 0x6dd4b0 && money_texts[2].at == 0x6e2fb0, "money_texts.inc changed");
REWRITE("Objectives::LabelledNumber", 0x6e4580, number_texts_first[0], (&number_text_copy<0>), &g_objective_texts);
REWRITE("Objectives::LabelledNumber", 0x6e4ba0, number_texts_first[1], (&number_text_copy<1>), &g_objective_texts);
REWRITE("Objectives::LabelledNumber", 0x6e7060, number_texts_first[2], (&number_text_copy<2>), &g_objective_texts);
REWRITE("Objectives::LabelledNumber", 0x6dd000, money_texts_first[0], (&money_text_copy<0>), &g_objective_texts);
REWRITE("Objectives::LabelledNumber", 0x6dd4b0, money_texts_first[1], (&money_text_copy<1>), &g_objective_texts);
REWRITE("Objectives::LabelledNumber", 0x6e2fb0, money_texts_first[2], (&money_text_copy<2>), &g_objective_texts);
