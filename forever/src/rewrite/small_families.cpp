// Small functions the game has written out several times (each family checked by tools/family.py; its rows in the .inc
// named after it).
#include "rewrite.h"

namespace {
typedef void *(__cdecl *Alloc)(uint32_t);
typedef void(__cdecl *Free)(void *);
const Alloc alloc = (Alloc)0xbcdfc0;
// a shared text {count, ..., buffer at +0xC} let go: the count taken down; at 0 its buffer freed, then it
inline void let_go(int32_t *volatile *held) {
    --**held;
    int32_t *text = *held;   // read again, as the game does
    if (*(volatile int32_t *)text == 0) {
        ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
        ((Free)0xbce000)(text);
    }
}
}   // namespace

// ---- Text peep%02d, Peep%02d, InspectionPoint%02d, LookAt%02d and their copies (eight): a numbered name. Given where the
// text goes (in esi) and a number (on the stack), the text of the copy's format (numbered_names.inc) with the number + 1
// (the game's sprintf), made on 16 new bytes (none without memory); returns where it went ----
#define NUMBERED_NAMES(at, format, ...) {at, format},
static constexpr uint32_t numbered_names[][2] = {
#include "numbered_names.inc"
};
#undef NUMBERED_NAMES
#define NUMBERED_NAMES(at, format, ...) {__VA_ARGS__},
static const uint8_t numbered_names_first[][7] = {
#include "numbered_names.inc"
};
#undef NUMBERED_NAMES
static volatile long g_numbered_names;
extern "C" int32_t **__stdcall forever_numbered_name(int32_t **out, int32_t number, const char *format) {
    g_numbered_names++;
    char written[0x20];
    ((int(__cdecl *)(char *, const char *, ...))0xddcdb5)(written, format, number + 1);
    void *memory = alloc(0x10);
    *out = memory ? ((int32_t * (__thiscall *)(void *, const char *))0xbcf850)(memory, written) : nullptr;
    return out;
}
// (the register wrappers: the copy's format pushed after the number, esi as where the text goes)
#define NUMBERED_NAME_REGS(k, format)                                                                                       \
    extern "C" __attribute__((naked)) void forever_numbered_name_regs_##k() {                                               \
        __asm__("pushl $" #format "\n\tpushl 8(%esp)\n\tpushl %esi\n\tcall _forever_numbered_name@12\n\tret $4");           \
    }
NUMBERED_NAME_REGS(0, 0xf2cb48)
NUMBERED_NAME_REGS(1, 0xf30aa0)
NUMBERED_NAME_REGS(2, 0xf30aac)
NUMBERED_NAME_REGS(3, 0xf30ac0)
NUMBERED_NAME_REGS(4, 0xf30aac)
NUMBERED_NAME_REGS(5, 0xf30aa0)
NUMBERED_NAME_REGS(6, 0xf9dbc0)
NUMBERED_NAME_REGS(7, 0xf9dbd0)
static_assert(numbered_names[0][1] == 0xf2cb48 && numbered_names[1][1] == 0xf30aa0 && numbered_names[2][1] == 0xf30aac && numbered_names[3][1] == 0xf30ac0 &&
                  numbered_names[4][1] == 0xf30aac && numbered_names[5][1] == 0xf30aa0 && numbered_names[6][1] == 0xf9dbc0 && numbered_names[7][1] == 0xf9dbd0,
              "numbered_names.inc changed: the wrappers' formats");
static_assert(numbered_names[0][0] == 0x46d730 && numbered_names[1][0] == 0x4aabd0 && numbered_names[2][0] == 0x4aac60 && numbered_names[3][0] == 0x4aacf0 &&
                  numbered_names[4][0] == 0x5a9360 && numbered_names[5][0] == 0xa97770 && numbered_names[6][0] == 0xbaddd0 && numbered_names[7][0] == 0xbade60,
              "numbered_names.inc changed: the addresses");
REWRITE("Text::NumberedName", 0x46d730, numbered_names_first[0], (void *)&forever_numbered_name_regs_0, &g_numbered_names);
REWRITE("Text::NumberedName", 0x4aabd0, numbered_names_first[1], (void *)&forever_numbered_name_regs_1, &g_numbered_names);
REWRITE("Text::NumberedName", 0x4aac60, numbered_names_first[2], (void *)&forever_numbered_name_regs_2, &g_numbered_names);
REWRITE("Text::NumberedName", 0x4aacf0, numbered_names_first[3], (void *)&forever_numbered_name_regs_3, &g_numbered_names);
REWRITE("Text::NumberedName", 0x5a9360, numbered_names_first[4], (void *)&forever_numbered_name_regs_4, &g_numbered_names);
REWRITE("Text::NumberedName", 0xa97770, numbered_names_first[5], (void *)&forever_numbered_name_regs_5, &g_numbered_names);
REWRITE("Text::NumberedName", 0xbaddd0, numbered_names_first[6], (void *)&forever_numbered_name_regs_6, &g_numbered_names);
REWRITE("Text::NumberedName", 0xbade60, numbered_names_first[7], (void *)&forever_numbered_name_regs_7, &g_numbered_names);

// ---- Report::MissingResourceSymbol and its copies (four, identical): when the resource reference's flag (bit 0, 0xBD1420)
// is clear, a message box (0x468450) titled "Missing Resource Symbol" with the reference's name (+0x18, copied) ----
#define MISSING_SYMBOLS(at, ...) {__VA_ARGS__},
static const uint8_t missing_symbols_first[][7] = {
#include "missing_symbols.inc"
};
#undef MISSING_SYMBOLS
static volatile long g_missing_symbols;
static void __thiscall missing_symbol(uint8_t **self) {
    g_missing_symbols++;
    if (((uint8_t(__thiscall *)(void *, uint32_t))0xbd1420)(*self, 0)) return;
    void *memory = alloc(0x10);
    int32_t *volatile title = memory ? ((int32_t * (__thiscall *)(void *, uintptr_t))0xbcf850)(memory, 0xf28408) : nullptr;   // "Missing Resource Symbol"
    int32_t *volatile name;
    ((void *(__thiscall *)(void *, void *))0xbcfef0)((void *)&name, *(uint8_t *volatile *)self + 0x18);
    ((void(__fastcall *)(void *, void *))0x468450)((void *)&name, (void *)&title);
    let_go(&name);
    let_go(&title);
}
REWRITE("Report::MissingResourceSymbol", 0x431360, missing_symbols_first[0], &missing_symbol, &g_missing_symbols);
REWRITE("Report::MissingResourceSymbol", 0x49b9f0, missing_symbols_first[1], &missing_symbol, &g_missing_symbols);
REWRITE("Report::MissingResourceSymbol", 0x696130, missing_symbols_first[2], &missing_symbol, &g_missing_symbols);
REWRITE("Report::MissingResourceSymbol", 0x771570, missing_symbols_first[3], &missing_symbol, &g_missing_symbols);

// ---- Animal::Point and its copies (four, identical): two windows (+0x114, +0x118) given rectangles (0x6AFAF0) from the
// animal's span (+0x84 - +0x7C) on the FPU: from 15 + span x 0.5 across span x 0.5, and from 15 + span x 0.05 across
// span x 0.4375 (the start kept unrounded for the end, as the game keeps it on the FPU), 2 to 18 down ----
#define ANIMAL_POINTS(at, ...) {__VA_ARGS__},
static const uint8_t animal_points_first[][7] = {
#include "animal_points.inc"
};
#undef ANIMAL_POINTS
static volatile long g_animal_points;
static void animal_rect(uint8_t *self, uint32_t window, uintptr_t scale_at, uintptr_t width_at) {
    typedef long double x87;
    const x87 start = ((x87) * (volatile float *)(self + 0x84) - *(volatile float *)(self + 0x7c)) * *(volatile float *)scale_at + *(volatile float *)0xf29638;
    volatile float rect[4];
    rect[1] = 2.0f, rect[3] = 18.0f;
    rect[0] = (float)start;
    rect[2] = (float)(((x87) * (volatile float *)(self + 0x84) - *(volatile float *)(self + 0x7c)) * *(volatile float *)width_at + start);
    ((void(__thiscall *)(void *, volatile float *))0x6afaf0)(*(void *volatile *)(self + window), rect);
}
static void __thiscall animal_points(uint8_t *self) {
    g_animal_points++;
    animal_rect(self, 0x114, 0xf2651c, 0xf2651c);   // 0.5, 0.5
    animal_rect(self, 0x118, 0xf270b4, 0xf94ad4);   // 0.05, 0.4375
}
REWRITE("Animal::Point", 0xace100, animal_points_first[0], &animal_points, &g_animal_points);
REWRITE("Animal::Point", 0xae9f80, animal_points_first[1], &animal_points, &g_animal_points);
REWRITE("Animal::Point", 0xaeae80, animal_points_first[2], &animal_points, &g_animal_points);
REWRITE("Animal::Point", 0xaebc40, animal_points_first[3], &animal_points, &g_animal_points);
