// Ride::Window503860, Ride::Window503AB0 and Ride::Window503B80 (the same code three times; only their exception
// tables differ): bring a ride's open window's text up to date (thiscall; no values).
//   - Only when the ride has its window (+0x18): the window told first (0x5036D0); the ride's text asked for (its
//     slot 0x4C, into a text here); the text the window's panel (+0xE0) shows (+0x114) let go of and replaced by it,
//     unless it is that one already; this text let go (each release read again, as the game does); then the window
//     told again (0x501DA0 with 0).
#include "rewrite.h"

namespace {
struct Text {
    int32_t count;
    uint32_t length, room;
    uint16_t *characters;
};
typedef void(__cdecl *Free)(void *);
inline void free_text(Text *text) {
    ((Free)0xbce010)(text->characters);
    ((Free)0xbce000)(text);
}

volatile long g_ride_text_windows;
void __fastcall ride_text_window(uint8_t *self) {
    g_ride_text_windows++;
    if (!*(void **)(self + 0x18)) return;
    ((void(__thiscall *)(void *))0x5036d0)(*(void **)(self + 0x18));
    Text *volatile text;
    ((void(__thiscall *)(void *, void *))(*(void ***)self)[0x4c / 4])(self, (void *)&text);
    uint8_t *panel = *(uint8_t **)(*(uint8_t **)(self + 0x18) + 0xe0);
    Text *old = *(Text **)(panel + 0x114);
    if (text != old) {
        --old->count;
        Text *shown = *(Text *volatile *)(panel + 0x114);
        if (shown->count == 0 && shown) free_text(shown);
        *(Text **)(panel + 0x114) = text;
        ++text->count;
    }
    --text->count;
    Text *mine = text;
    if (*(volatile int32_t *)&mine->count == 0) free_text(mine);
    ((void(__thiscall *)(void *, uint32_t))0x501da0)(*(void **)(self + 0x18), 0);
}

const uint8_t ride_text_window_first[][7] = {{0x6a, 0xff, 0x68, 0xf8, 0x8f, 0xe1, 0x00}, {0x6a, 0xff, 0x68, 0x58, 0x90, 0xe1, 0x00}, {0x6a, 0xff, 0x68, 0x78, 0x90, 0xe1, 0x00}};
}   // namespace
REWRITE("Ride::WindowText", 0x503860, ride_text_window_first[0], &ride_text_window, &g_ride_text_windows);
REWRITE("Ride::WindowText", 0x503ab0, ride_text_window_first[1], &ride_text_window, &g_ride_text_windows);
REWRITE("Ride::WindowText", 0x503b80, ride_text_window_first[2], &ride_text_window, &g_ride_text_windows);
