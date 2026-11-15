// Sound::GUIClick, Sound::GUIBClick and Sound::GUIBRelease (one function written out three times; tools/family.py:
// gui_sounds.inc; they differ in their three sample globals and the sound's name): play an interface sound, when
// sound is on ([0x10EEDD0]). Sound::GUIWarning is the same with two samples: its first for 0 and 2 (row written here).
//   - One of the three samples picked by the C library's rand() % 3 (0: the first, 1: the third, 2: the second).
//   - A sound made for it (0x64 new bytes, 0x47CEE0 with 0, 2, the sample, 0; none without memory): to be played
//     (+0x13, +0x10 set); without a voice (+0x18) also +0x12; with one not yet started (+0x11 clear), its voice
//     (+0xC) started (0xCE4770 with the voice + 0x50), +0x11 set and +0x30 cleared.
//   - Its name (+0x44) made the copy's ("GUIClick", ...: 0x10 new bytes, WideString from C): the old one let go when
//     it is another text, the new one taken, and the made one let go.
#include "rewrite.h"

namespace {
struct GuiSound {
    uint32_t at, first, second, third, name;
};
#define GUI_SOUNDS(at, first, second, third, name, ...) {at, first, second, third, name},
constexpr GuiSound gui_sounds[] = {
#include "gui_sounds.inc"
};
#undef GUI_SOUNDS
#define GUI_SOUNDS(at, first, second, third, name, ...) {__VA_ARGS__},
const uint8_t gui_sounds_first[][7] = {
#include "gui_sounds.inc"
};
#undef GUI_SOUNDS

typedef void *(__cdecl *Alloc)(uint32_t);
typedef void(__cdecl *Free)(void *);
const Alloc alloc = (Alloc)0xbcdfc0;
inline void free_text(int32_t *text) {
    ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
    ((Free)0xbce000)(text);
}

volatile long g_gui_sounds;
void gui_sound(const GuiSound &kind) {
    g_gui_sounds++;
    if (!*(volatile uint8_t *)(uintptr_t)0x10eedd0) return;
    const int pick = ((int(__cdecl *)())0xddedd7)() % 3;
    const uint32_t sample = *(volatile uint32_t *)(uintptr_t)(pick == 1 ? kind.third : pick == 2 ? kind.second : kind.first);
    void *memory = alloc(0x64);
    uint8_t *sound = memory ? (uint8_t *)((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))0x47cee0)(memory, 0, 2, sample, 0) : nullptr;
    const uint32_t voice = *(volatile uint32_t *)(sound + 0x18);
    sound[0x13] = 1;
    sound[0x10] = 1;
    if (voice == 0) {
        sound[0x12] = 1;
    } else if (!sound[0x11]) {
        uint8_t *v = *(uint8_t **)(sound + 0xc);
        ((void(__thiscall *)(void *, void *))0xce4770)(v, v + 0x50);
        sound[0x11] = 1;
        *(volatile uint32_t *)(sound + 0x30) = 0;
    }
    void *block = alloc(0x10);
    int32_t *name = block ? ((int32_t * (__thiscall *)(void *, uintptr_t))0xbcf850)(block, kind.name) : nullptr;
    int32_t *volatile *held = (int32_t * volatile *)(sound + 0x44);
    if (name != *held) {
        --**held;
        int32_t *old = *held;   // read again, as the game does
        if (*(volatile int32_t *)old == 0 && old) free_text(old);
        *held = name;
        ++*name;
    }
    if (--*(volatile int32_t *)name == 0) free_text(name);
}

template <int K> void __cdecl gui_sound_copy() { gui_sound(gui_sounds[K]); }
constexpr GuiSound warning_sound = {0x69aad0, 0x1304244, 0x1304244, 0x12fa680, 0xf53220};   // "GUIWarning"
const uint8_t warning_sound_first[] = {0x6a, 0xff, 0x68, 0x9e, 0xbc, 0xe3, 0x00};
void __cdecl warning_sound_play() { gui_sound(warning_sound); }
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(gui_sounds[0].at == 0x69a6e0 && gui_sounds[1].at == 0x69a830 && gui_sounds[2].at == 0x69a980, "gui_sounds.inc changed");
REWRITE("Sound::GUISound", 0x69a6e0, gui_sounds_first[0], (&gui_sound_copy<0>), &g_gui_sounds);
REWRITE("Sound::GUISound", 0x69a830, gui_sounds_first[1], (&gui_sound_copy<1>), &g_gui_sounds);
REWRITE("Sound::GUISound", 0x69a980, gui_sounds_first[2], (&gui_sound_copy<2>), &g_gui_sounds);
REWRITE("Sound::GUISound", 0x69aad0, warning_sound_first, &warning_sound_play, &g_gui_sounds);
