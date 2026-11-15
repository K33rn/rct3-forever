// Info::TextA290D0 and Info::TextA30980 (one function written out twice; they differ in where the info keeps its
// object, +0xA0 or +0x9C): the info's text with "%ME%" put as the object's name (thiscall; where to put the text;
// gives that place). Info::Text7C8700 and Info::Text81AFA0 are the same with the text elsewhere (+0x1B0, +0x74) and
// the name from elsewhere: the object at +0x200 (its +8; without an object the text is given as it is), or the
// global wide text [0x12FDEA0].
//   - The info's text (+0x64), held once more; the name: a new text from the object's wide name (+0xC; 0xBCF7D0, none
//     without memory), or the shared empty text when it has none; "%ME%" a new text (none without memory).
//   - Every "%ME%" in the text replaced by the name (0xCD23A0); "%ME%" and the name let go; the place given the text
//     (held once more) and this hold of it let go (each release read again, as the game does).
#include "rewrite.h"

namespace {
struct Text {
    int32_t count;
    uint32_t length, room;
    uint16_t *characters;
};
typedef void(__cdecl *Free)(void *);
typedef void *(__cdecl *Alloc)(uint32_t);
inline void let_go(Text *volatile *held) {
    --(*held)->count;
    Text *text = *held;
    if (*(volatile int32_t *)&text->count == 0) {
        ((Free)0xbce010)(text->characters);
        ((Free)0xbce000)(text);
    }
}

volatile long g_info_texts;
enum NameFrom { OBJECT_C, OBJECT_8_OR_NONE, GLOBAL };
template <uint32_t TEXT, NameFrom FROM, uint32_t WHERE> inline const void *name_of(uint8_t *self) {
    if (FROM == OBJECT_C) return *(const void **)(*(uint8_t **)(self + WHERE) + 0xc);
    if (FROM == GLOBAL) return *(const void *volatile *)(uintptr_t)WHERE;
    const uint8_t *object = *(const uint8_t **)(self + WHERE);
    return object ? *(const void **)(object + 8) : nullptr;
}
template <uint32_t TEXT, NameFrom FROM, uint32_t WHERE> Text **__fastcall info_text(uint8_t *self, void *, Text **out) {
    g_info_texts++;
    Text *volatile text = *(Text **)(self + TEXT);
    ++text->count;
    if (FROM == OBJECT_8_OR_NONE && !*(void **)(self + WHERE)) {   // no object: the text as it is
        *out = text;
        ++text->count;
        let_go(&text);
        return out;
    }
    Text *volatile name;
    if (const void *wide = name_of<TEXT, FROM, WHERE>(self)) {
        void *memory = ((Alloc)0xbcdfc0)(0x10);
        name = memory ? ((Text * (__thiscall *)(void *, const void *))0xbcf7d0)(memory, wide) : nullptr;
    } else {
        name = ((Text * (__fastcall *)(uint32_t))0xbd0490)(1);
    }
    Text *volatile me;
    {
        void *memory = ((Alloc)0xbcdfc0)(0x10);
        me = memory ? ((Text * (__thiscall *)(void *, const char *))0xbcf850)(memory, (const char *)0xf31f48) : nullptr;   // "%ME%"
    }
    ((void(__fastcall *)(void *, void *, void *))0xcd23a0)((void *)&text, (void *)&me, (void *)&name);
    let_go(&me);
    let_go(&name);
    *out = text;
    ++text->count;
    let_go(&text);
    return out;
}

const uint8_t info_text_first[][7] = {{0x6a, 0xff, 0x68, 0x87, 0xbe, 0xe7, 0x00}, {0x6a, 0xff, 0x68, 0x47, 0xce, 0xe7, 0x00},
                                      {0x6a, 0xff, 0x68, 0x37, 0x7c, 0xe5, 0x00}, {0x6a, 0xff, 0x68, 0x17, 0xde, 0xe5, 0x00}};
}   // namespace
REWRITE("Info::NameText", 0xa290d0, info_text_first[0], (&info_text<0x64, OBJECT_C, 0xa0>), &g_info_texts);
REWRITE("Info::NameText", 0xa30980, info_text_first[1], (&info_text<0x64, OBJECT_C, 0x9c>), &g_info_texts);
REWRITE("Info::NameText", 0x7c8700, info_text_first[2], (&info_text<0x1b0, OBJECT_8_OR_NONE, 0x200>), &g_info_texts);
REWRITE("Info::NameText", 0x81afa0, info_text_first[3], (&info_text<0x74, GLOBAL, 0x12fdea0>), &g_info_texts);
