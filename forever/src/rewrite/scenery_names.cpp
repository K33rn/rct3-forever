// VistaPoint::Name and WaterCannon::Name (one function written out twice; they differ in where the item keeps its
// text, +0x84 or +0x144, and the game text named): the item's text with "%ME%" put as its kind's name (thiscall; where
// to put the text; gives that place). As info_texts.cpp's, with the name from the game's texts:
//   - The item's text, held once more; the game text by name ("SceneryItem_Vistapoint" / "SceneryItem_WaterCannon",
//     0x4040F0 into a temporary on the stack); the name a new text from its wide text (+8; 0xBCF7D0, none without
//     memory) or the shared empty text when it has none; "%ME%" a new text.
//   - Every "%ME%" replaced by the name (0xCD23A0); "%ME%" and the name let go; the temporary destroyed (0xBD1880);
//     the place given the text (held once more) and this hold of it let go.
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

volatile long g_scenery_names;
template <uint32_t TEXT, uintptr_t NAME> Text **__fastcall scenery_name(uint8_t *self, void *, Text **out) {
    g_scenery_names++;
    Text *volatile text = *(Text **)(self + TEXT);
    ++text->count;
    alignas(4) uint8_t game_text[0x1c];
    uint8_t *ref = (uint8_t *)((void *(__thiscall *)(void *, const char *, uintptr_t, uint32_t))0x4040f0)(game_text, (const char *)NAME, 0x1303cfc, 1);
    Text *volatile name;
    if (const void *wide = *(const void **)(ref + 8)) {
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
    ((void(__thiscall *)(void *))0xbd1880)(game_text);
    *out = text;
    ++text->count;
    let_go(&text);
    return out;
}

const uint8_t scenery_name_first[][7] = {{0x6a, 0xff, 0x68, 0x0f, 0x53, 0xe6, 0x00}, {0x6a, 0xff, 0x68, 0x5f, 0x5a, 0xe6, 0x00}};
}   // namespace
REWRITE("Scenery::KindNameText", 0x8b1330, scenery_name_first[0], (&scenery_name<0x84, 0xf7a4b0>), &g_scenery_names);    // VistaPoint
REWRITE("Scenery::KindNameText", 0x8b6900, scenery_name_first[1], (&scenery_name<0x144, 0xf7a894>), &g_scenery_names);   // WaterCannon
