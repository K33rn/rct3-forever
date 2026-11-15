// NumSceneryItemsOfClassificationCondition, DungTargetCondition, ViewingGalleriesCondition, ReleaseAnimalsCondition:
// CurrentText. One function written out four times (tools/family.py checks they differ only in the name of their text,
// current_texts.inc). Given where the text goes and a part number (thiscall; two arguments): for a part other than 0,
// the objective's timing part (0x6D6CA0, for the part before it); for part 0, the condition's "current" text from the
// game's text table, put together with a space (0x442120), with every "#" in it replaced by the condition's current
// value (its slot 0x68, a number on the FPU, rounded to a whole number with its rounding mode) written out with "%d".
// MinAnimalHealthCondition's is the same with the value given as a double, written out with "%.2f" (row written here).
#include "rewrite.h"

namespace {
struct CurrentText {
    uint32_t at, name;
    bool as_double;   // (the value as a double with "%.2f", not rounded with "%d")
};
#define CURRENT_TEXTS(at, name, ...) {at, name},
constexpr CurrentText current_texts[] = {
#include "current_texts.inc"
};
#undef CURRENT_TEXTS
#define CURRENT_TEXTS(at, name, ...) {__VA_ARGS__},
const uint8_t current_texts_first[][7] = {
#include "current_texts.inc"
};
#undef CURRENT_TEXTS

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
inline int32_t *text_c(const char *c_text) {
    void *memory = alloc(0x10);
    return memory ? ((int32_t * (__thiscall *)(void *, const char *))0xbcf850)(memory, c_text) : nullptr;
}

volatile long g_current_texts;
int32_t **current_text(uint8_t *self, int32_t **out, int32_t part, const CurrentText &kind) {
    g_current_texts++;
    if (part) return ((int32_t * *(__thiscall *)(void *, int32_t **, int32_t))0x6d6ca0)(self, out, part - 1);
    int32_t *space_text = text_c((const char *)0xf28e54);   // " "
    int32_t *volatile space = space_text;
    alignas(16) uint8_t named[0x40];
    uint8_t *found = (uint8_t *)((void *(__thiscall *)(void *, uintptr_t, uintptr_t, uint32_t))0x4040f0)(named, kind.name, 0x1303cfc, 1);
    int32_t *volatile text = nullptr;   // (the game makes it in the part number's place, which holds 0 here)
    ((void(__fastcall *)(void *, const void *, void *))0x442120)((void *)&text, *(const void *volatile *)(found + 8), (void *)&space);
    ((void(__thiscall *)(void *))0xbd1880)(named);
    if (--*space_text == 0) free_text(space_text);   // (through the pointer kept, not read again)
    long double value = ((long double(__thiscall *)(void *))(*(void ***)self)[0x68 / 4])(self);
    char written[0x200];   // (room for any double written out)
    if (kind.as_double) {
        volatile double given = (double)value;
        ((int(__cdecl *)(char *, const char *, ...))0xddcdb5)(written, (const char *)0xf30590, given);   // "%.2f"
    } else {
        int64_t whole;
        __asm__("fistpll %0" : "=m"(whole) : "t"(value) : "st");
        ((int(__cdecl *)(char *, const char *, ...))0xddcdb5)(written, (const char *)0xf28e5c, (int32_t)whole);   // "%d"
    }
    int32_t *volatile number = text_c(written);
    int32_t *volatile mark = text_c((const char *)0xf28e58);   // "#"
    ((void(__fastcall *)(void *, void *, void *))0xcd23a0)((void *)&text, (void *)&mark, (void *)&number);   // every "#" replaced
    let_go(&mark);
    let_go(&number);
    *out = text;
    ++*text;
    let_go(&text);
    return out;
}

template <int K>
int32_t **__thiscall current_text_copy(uint8_t *self, int32_t **out, int32_t part) {
    return current_text(self, out, part, current_texts[K]);
}
constexpr CurrentText health_text = {0x6e5bf0, 0xf65bac, true};   // "MinAnimalHealthCondition_Current"
const uint8_t health_text_first[] = {0x6a, 0xff, 0x68, 0xa9, 0x58, 0xe4, 0x00};
int32_t **__thiscall health_current_text(uint8_t *self, int32_t **out, int32_t part) { return current_text(self, out, part, health_text); }
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(current_texts[0].at == 0x6df560 && current_texts[1].at == 0x6e46b0 && current_texts[2].at == 0x6e55d0 && current_texts[3].at == 0x6e6c30,
              "current_texts.inc changed");
REWRITE("Condition::CurrentText", 0x6df560, current_texts_first[0], (&current_text_copy<0>), &g_current_texts);
REWRITE("Condition::CurrentText", 0x6e46b0, current_texts_first[1], (&current_text_copy<1>), &g_current_texts);
REWRITE("Condition::CurrentText", 0x6e55d0, current_texts_first[2], (&current_text_copy<2>), &g_current_texts);
REWRITE("Condition::CurrentText", 0x6e6c30, current_texts_first[3], (&current_text_copy<3>), &g_current_texts);
REWRITE("Condition::CurrentTextDouble", 0x6e5bf0, health_text_first, &health_current_text, &g_current_texts);
