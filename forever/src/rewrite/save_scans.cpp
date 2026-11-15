// Save::Write497A20 and Save::Write497BC0 (one function written out twice; they differ in the format): read a
// number from a text the object gives (thiscall; where to put the number; true when there was one to read).
//   - The text: the shared empty one (0xBD0490), filled by 0x497300, which gives its length; with none, or 256
//     characters or more, it is let go and the answer is false.
//   - Its characters copied one by one into a 256-character buffer, the text made its own first each time (as
//     default_names.cpp's: with others sharing it, one fewer and a new copy, none without memory; made longer when
//     the place is past its end); the buffer ended with a 0.
//   - swscanf (0xDDED94) of the buffer with the copy's format (L"%d" or L"%f") into the place given; the text let
//     go; true.
#include "rewrite.h"

namespace {
struct Text {
    int32_t count;
    uint32_t length, room;
    uint16_t *characters;
};
typedef void(__cdecl *Free)(void *);
inline void let_go(Text *volatile *held) {   // (read again, as the game does)
    --(*held)->count;
    Text *text = *held;
    if (*(volatile int32_t *)&text->count == 0) {
        ((Free)0xbce010)(text->characters);
        ((Free)0xbce000)(text);
    }
}

volatile long g_save_scans;
template <uintptr_t FORMAT> uint8_t __fastcall save_scan(void *self, void *, void *out) {
    g_save_scans++;
    Text *volatile text = ((Text * (__fastcall *)(uint32_t))0xbd0490)(1);
    const int32_t length = ((int32_t(__thiscall *)(void *, void *))0x497300)(self, (void *)&text);
    if (length == 0 || length >= 0x100) {
        let_go(&text);
        return 0;
    }
    uint16_t buffer[0x100];
    for (int32_t i = 0; i < length; i++) {
        Text *t = text;
        if (t->count > 1) {   // made its own
            --t->count;
            void *memory = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x10);
            text = memory ? ((Text * (__thiscall *)(void *, const uint16_t *, uint32_t))0xbcf710)(memory, t->characters, t->length) : nullptr;
        }
        if ((uint32_t)i >= text->length) {
            ((void(__thiscall *)(void *, uint32_t))0xbcffe0)((void *)&text, i + 1);
            text->length = i + 1;
        }
        buffer[i] = text->characters[i];
    }
    buffer[length] = 0;
    ((int(__cdecl *)(const uint16_t *, const void *, ...))0xdded94)(buffer, (const void *)FORMAT, out);
    let_go(&text);
    return 1;
}

const uint8_t save_scan_first[][7] = {{0x6a, 0xff, 0x68, 0xc9, 0xf6, 0xe0, 0x00}, {0x6a, 0xff, 0x68, 0xf9, 0xf6, 0xe0, 0x00}};
}   // namespace
REWRITE("Save::ScanNumber", 0x497a20, save_scan_first[0], (&save_scan<0xf2ea98>), &g_save_scans);   // L"%d"
REWRITE("Save::ScanNumber", 0x497bc0, save_scan_first[1], (&save_scan<0xf2eaa0>), &g_save_scans);   // L"%f"
