// Ride::Text4F06B0, Ride::Text4F07F0 and Ride::Text4F0930 (one function written out three times; they differ in the
// ending joined on: "On", "Off" and "Dis"): a ride's switch text (thiscall: the ride's name, a C text at +0 or none;
// the text made in the place given, which is returned).
//   - The name as a wide text (0x10 new bytes, WideString from C; none without memory), or the shared empty text
//     when there is no name; the ending joined on (0x403BE0); that copied (0xBD01C0).
//   - The place given made a text reference to the copy, with the game's text table [0x12F4FF8 is its address] and
//     1 (0x41CAB0).
//   - The copy, the joined text and the name let go.
#include "rewrite.h"

namespace {
typedef void(__cdecl *Free)(void *);
inline void let_go(int32_t *volatile *held) {   // a shared text let go (read again, as the game does)
    --**held;
    int32_t *text = *held;
    if (*(volatile int32_t *)text == 0) {
        ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
        ((Free)0xbce000)(text);
    }
}

volatile long g_ride_switch_texts;
template <uintptr_t ENDING> void *__thiscall ride_switch_text(const char **self, void *made) {
    g_ride_switch_texts++;
    int32_t *volatile name, *volatile joined, *volatile copy;
    const char *own = *self;
    if (own) {
        void *memory = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x10);
        name = memory ? ((int32_t * (__thiscall *)(void *, const char *))0xbcf850)(memory, own) : nullptr;
    } else {
        name = ((int32_t * (__fastcall *)(uint32_t))0xbd0490)(1);
    }
    void *j = ((void *(__thiscall *)(void *, void *, uintptr_t))0x403be0)((void *)&name, (void *)&joined, ENDING);
    ((void *(__thiscall *)(void *, void *))0xbd01c0)((void *)&copy, j);
    ((void *(__thiscall *)(void *, void *, uintptr_t, uint32_t))0x41cab0)(made, (void *)&copy, 0x12f4ff8, 1);
    let_go(&copy);
    let_go(&joined);
    let_go(&name);
    return made;
}

const uint8_t ride_switch_texts_first[][7] = {{0x6a, 0xff, 0x68, 0xec, 0x70, 0xe1, 0x00}, {0x6a, 0xff, 0x68, 0x3c, 0x71, 0xe1, 0x00}, {0x6a, 0xff, 0x68, 0x8c, 0x71, 0xe1, 0x00}};
}   // namespace
REWRITE("Ride::SwitchText", 0x4f06b0, ride_switch_texts_first[0], (&ride_switch_text<0xf288e8>), &g_ride_switch_texts);   // "On"
REWRITE("Ride::SwitchText", 0x4f07f0, ride_switch_texts_first[1], (&ride_switch_text<0xf28914>), &g_ride_switch_texts);   // "Off"
REWRITE("Ride::SwitchText", 0x4f0930, ride_switch_texts_first[2], (&ride_switch_text<0xf35d48>), &g_ride_switch_texts);   // "Dis"
