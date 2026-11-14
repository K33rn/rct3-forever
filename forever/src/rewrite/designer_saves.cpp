// LaserDesigner::Save and WaterJetDesigner::Save: one function written out twice (tools/family.py: designer_saves.inc;
// they differ in the designs' file ending, ".las" or ".wjt", and the function that writes the file). Given a folder and
// a name (thiscall; two text holders), by the designer's state (+0x2FC):
//   0: the content error (0x800460, on +0x2E8) for the folder joined with the name;
//   1: the path (+0x398) made the folder joined with the name, given the ending when it has none (0x402550 says);
//      a name that is empty, starts with "." or with a space gets its message box (SGD_NeedANameTitle,
//      SGD_NameCannotStartWithDot, SGD_NameCannotStartWithSpace); otherwise, when there is no file at the path, it is
//      written (the copy's function, on +0x2F8, given the path), and when there is, the overwrite question
//      (SGD_ConfirmSave, SGD_SaveOverFile with %FILENAME% replaced by the name) is put to the designer's button bar;
//   anything else: nothing.
// Texts given by value are made just before the call that takes them (and lets them go).
#include "rewrite.h"

namespace {
struct DesignerSave {
    uint32_t at, ending, write;
};
#define DESIGNER_SAVES(at, ending, write, ...) {at, ending, write},
constexpr DesignerSave designer_saves[] = {
#include "designer_saves.inc"
};
#undef DESIGNER_SAVES
#define DESIGNER_SAVES(at, ending, write, ...) {__VA_ARGS__},
const uint8_t designer_saves_first[][7] = {
#include "designer_saves.inc"
};
#undef DESIGNER_SAVES

struct Text {
    int32_t count;
    uint32_t length, room;
    uint16_t *characters;
};
typedef void *(__cdecl *Alloc)(uint32_t);
typedef void(__cdecl *Free)(void *);
const Alloc alloc = (Alloc)0xbcdfc0;
inline void free_text(Text *text) {
    ((Free)0xbce010)(text->characters);
    ((Free)0xbce000)(text);
}
// a shared text let go: the count taken down; at 0 its buffer freed, then it
inline void let_go(Text *volatile *held) {
    --(*held)->count;
    Text *text = *held;   // read again, as the game does
    if (*(volatile int32_t *)&text->count == 0) free_text(text);
}
inline Text *text_c(uintptr_t c_text) {
    void *memory = alloc(0x10);
    return memory ? ((Text * (__thiscall *)(void *, uintptr_t))0xbcf850)(memory, c_text) : nullptr;
}
// a text in the game's text table, copied (the shared empty text when it has none); the reference let go after
inline Text *game_text(uintptr_t name) {
    alignas(16) uint8_t reference[0x40];
    uint8_t *made = (uint8_t *)((void *(__thiscall *)(void *, uintptr_t, uint32_t, uint32_t))0x4040f0)(reference, name, 0x1303cfc, 1);
    const void *characters = *(const void *volatile *)(made + 8);
    Text *text;
    if (characters) {
        void *memory = alloc(0x10);
        text = memory ? ((Text * (__thiscall *)(void *, const void *))0xbcf7d0)(memory, characters) : nullptr;
    } else {
        text = ((Text * (__fastcall *)(uint32_t))0xbd0490)(1);
    }
    ((void(__thiscall *)(void *))0xbd1880)(reference);
    return text;
}
typedef void(__thiscall *ButtonBar)(void *, Text *, Text *, uint32_t, uint32_t, Text *, uint32_t);

// a message box: a new button bar window (0x98 bytes, 0x6A5E20; none without memory) given the text and two empty texts
void message(uintptr_t name) {
    void *memory = alloc(0x98);
    void *bar = memory ? ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))0x6a5e20)(memory, 0, 1, 0) : nullptr;
    Text *last = text_c(0xf264ad);   // ""
    Text *second = text_c(0xf264ad);
    Text *first;
    {
        alignas(16) uint8_t reference[0x40];
        uint8_t *made = (uint8_t *)((void *(__thiscall *)(void *, uintptr_t, uint32_t, uint32_t))0x4040f0)(reference, name, 0x1303cfc, 1);
        const void *characters = *(const void *volatile *)(made + 8);
        if (characters) {
            void *m = alloc(0x10);
            first = m ? ((Text * (__thiscall *)(void *, const void *))0xbcf7d0)(m, characters) : nullptr;
        } else {
            first = ((Text * (__fastcall *)(uint32_t))0xbd0490)(1);
        }
        ((ButtonBar)0x6a6ac0)(bar, first, second, 0, 1, last, 0x1e);
        ((void(__thiscall *)(void *))0xbd1880)(reference);
    }
}

volatile long g_designer_saves;
void designer_save(uint8_t *self, Text *volatile *folder, Text *volatile *name, const DesignerSave &kind) {
    g_designer_saves++;
    const uint32_t state = *(volatile uint32_t *)(self + 0x2fc);
    if (state == 0) {
        Text *joined;
        ((void *(__thiscall *)(void *, void *, void *))0x403bb0)((void *)folder, (void *)&joined, (void *)name);
        ((void(__thiscall *)(void *, Text *))0x800460)(self + 0x2e8, joined);
        return;
    }
    if (state != 1) return;
    Text *volatile *path = (Text *volatile *)(self + 0x398);
    {
        Text *volatile joined;
        Text **made = (Text **)((void *(__thiscall *)(void *, void *, void *))0x403bb0)((void *)folder, (void *)&joined, (void *)name);
        Text *old = *path;
        if (*(Text *volatile *)made != old) {
            --old->count;
            uintptr_t again = (uintptr_t)*path;
            if (*(volatile int32_t *)again == 0 && again) free_text((Text *)again);
            Text *now = *(Text *volatile *)made;
            *path = now;
            ++now->count;
        }
        let_go(&joined);
    }
    Text *ending = text_c(kind.ending);   // (given by value)
    if (!((uint8_t(__thiscall *)(void *, Text *))0x402550)((void *)path, ending)) {
        const char *c = (const char *)(uintptr_t)kind.ending;
        uint32_t length = 0;
        while (c[length]) length++;
        ((void(__thiscall *)(void *, uint32_t))0xbcffe0)((void *)path, (*path)->length + length + 1);
        uint16_t *to = (*path)->characters + (*path)->length;
        uint32_t k = 0;
        for (; k < length && c[k]; k++) to[k] = (uint8_t)c[k];
        to[k] = 0;
        (*path)->length += length;
    }
    const Text *given = *name;
    if (given->length == 0) return message(0xf466b8);              // SGD_NeedANameTitle
    if (given->characters[0] == '.') return message(0xf4668c);     // SGD_NameCannotStartWithDot
    if (given->characters[0] == ' ') return message(0xf4666c);     // SGD_NameCannotStartWithSpace
    Text *volatile copy;
    ((void *(__thiscall *)(void *, void *))0xbd01c0)((void *)&copy, (void *)path);
    const bool there = ((uint8_t(__thiscall *)(void *))0xbe45e0)((void *)&copy) != 0;
    let_go(&copy);
    if (!there) {
        Text *whole = *path;
        ++whole->count;
        ((void(__thiscall *)(void *, Text *))kind.write)(*(void *volatile *)(self + 0x2f8), whole);
        return;
    }
    Text *question = game_text(0xf4665c);             // SGD_ConfirmSave
    Text *volatile asked = game_text(0xf823f4);       // SGD_SaveOverFile
    Text *volatile mark = text_c(0xf33f28);           // "%FILENAME%"
    ((void(__fastcall *)(void *, void *, void *))0xcd23a0)((void *)&asked, (void *)&mark, (void *)name);
    let_go(&mark);
    Text *empty = text_c(0xf264ad);
    ++question->count;
    Text *asked_given = asked;
    ++asked_given->count;
    ((ButtonBar)0x6a6ac0)(self + 0x300, asked_given, question, (uint32_t)(uintptr_t)(self + 0x154), 0xc, empty, 0x1e);
    let_go(&asked);
    if (--question->count == 0) free_text(question);   // (through the pointer kept, not read again)
}

template <int K>
void __thiscall designer_save_copy(uint8_t *self, Text *volatile *folder, Text *volatile *name) {
    designer_save(self, folder, name, designer_saves[K]);
}
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(designer_saves[0].at == 0xad0980 && designer_saves[1].at == 0xaec200, "designer_saves.inc changed");
REWRITE("Designer::Save", 0xad0980, designer_saves_first[0], (&designer_save_copy<0>), &g_designer_saves);
REWRITE("Designer::Save", 0xaec200, designer_saves_first[1], (&designer_save_copy<1>), &g_designer_saves);
