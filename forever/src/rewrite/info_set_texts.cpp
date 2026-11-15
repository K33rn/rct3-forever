// Info::TextA8AC30 and Info::TextB8BA60 (one function written out twice; they differ in where the info keeps its
// text, +0x68 or +0x5C, and its two own calls): set an info's text (thiscall; where the text is held; no value).
//   - The text copied here (held once more; the game keeps it where the argument was); every "#ME#" in it made
//     "%ME%" (0xCD23A0 with two new texts, none without memory; "#ME#" let go first).
//   - When the text is empty, the info's default (its copy's first call, into a text here) is used instead.
//   - The info's text replaced by that one, unless it is that one already (the old one let go; then the new held).
//   - The copy's second call on the info; the copy let go (each release read again, as the game does).
#include "rewrite.h"

namespace {
struct Text {
    int32_t count;
    uint32_t length, room;
    uint16_t *characters;
};
typedef void(__cdecl *Free)(void *);
typedef void *(__cdecl *Alloc)(uint32_t);
inline void free_text(Text *text) {
    ((Free)0xbce010)(text->characters);
    ((Free)0xbce000)(text);
}
inline void let_go(Text *volatile *held) {
    --(*held)->count;
    Text *text = *held;
    if (*(volatile int32_t *)&text->count == 0) free_text(text);
}
inline Text *text_c(const char *c) {
    void *memory = ((Alloc)0xbcdfc0)(0x10);
    return memory ? ((Text * (__thiscall *)(void *, const char *))0xbcf850)(memory, c) : nullptr;
}
inline void replace_held(uint8_t *self, uint32_t at, Text *with) {   // (the old one let go and freed when no one has it)
    Text *old = *(Text **)(self + at);
    if (with == old) return;
    --old->count;
    Text *shown = *(Text *volatile *)(self + at);
    if (shown->count == 0 && shown) free_text(shown);
    *(Text **)(self + at) = with;
    ++with->count;
}

volatile long g_info_set_texts;
template <uint32_t AT, uintptr_t DEFAULT, uintptr_t AFTER> void __fastcall info_set_text(uint8_t *self, void *, Text *const *source) {
    g_info_set_texts++;
    Text *volatile given = *source;
    ++given->count;
    Text *volatile me = text_c((const char *)0xf31f48);      // "%ME%"
    Text *volatile token = text_c((const char *)0xf31fa8);   // "#ME#"
    ((void(__fastcall *)(void *, void *, void *))0xcd23a0)((void *)&given, (void *)&token, (void *)&me);
    let_go(&token);
    let_go(&me);
    if (given->length == 0) {
        Text *volatile fallback;
        Text **made = ((Text * *(__thiscall *)(void *, void *))DEFAULT)(self, (void *)&fallback);
        replace_held(self, AT, *made);
        let_go(&fallback);
    } else {
        replace_held(self, AT, given);
    }
    ((void(__thiscall *)(void *))AFTER)(self);
    let_go(&given);
}

const uint8_t info_set_text_first[][7] = {{0x6a, 0xff, 0x68, 0x06, 0x51, 0xe8, 0x00}, {0x6a, 0xff, 0x68, 0x46, 0x8e, 0xe9, 0x00}};
}   // namespace
REWRITE("Info::SetText", 0xa8ac30, info_set_text_first[0], (&info_set_text<0x68, 0xa8a040, 0xa89f30>), &g_info_set_texts);
REWRITE("Info::SetText", 0xb8ba60, info_set_text_first[1], (&info_set_text<0x5c, 0xb87cd0, 0xb87b10>), &g_info_set_texts);
