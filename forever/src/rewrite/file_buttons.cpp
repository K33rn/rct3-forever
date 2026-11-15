// ParticleDesigner::FileButtons and the laser and water jet designers' (three copies; tools/family.py: file_buttons.inc,
// differing only in their two text names). Given where the text goes (thiscall, one argument): the tooltip for the
// designer's file button, by its state (+0x2FC): when 0 the first text ("..._Load"), otherwise the second ("..._Save"),
// from the game's text table, copied (the shared empty text when it has none). FireworkDisplay::LookButtons is the same
// by a byte at +0x20A (row written here).
#include "rewrite.h"

namespace {
struct FileButton {
    uint32_t at, load, save;
    uint32_t state = 0x2fc;
    bool state_byte = false;
};
#define FILE_BUTTONS(at, load, save, ...) {at, load, save},
constexpr FileButton file_buttons[] = {
#include "file_buttons.inc"
};
#undef FILE_BUTTONS
#define FILE_BUTTONS(at, load, save, ...) {__VA_ARGS__},
const uint8_t file_buttons_first[][7] = {
#include "file_buttons.inc"
};
#undef FILE_BUTTONS

volatile long g_file_buttons;
void **file_button(uint8_t *self, void **out, const FileButton &kind) {
    g_file_buttons++;
    const bool set = kind.state_byte ? *(volatile uint8_t *)(self + kind.state) != 0 : *(volatile uint32_t *)(self + kind.state) != 0;
    const uintptr_t name = set ? kind.save : kind.load;
    alignas(16) uint8_t reference[0x40];
    uint8_t *made = (uint8_t *)((void *(__thiscall *)(void *, uintptr_t, uint32_t, uint32_t))0x4040f0)(reference, name, 0x1303cfc, 1);
    const void *characters = *(const void *volatile *)(made + 8);
    void *text;
    if (characters) {
        void *memory = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x10);
        text = memory ? ((void *(__thiscall *)(void *, const void *))0xbcf7d0)(memory, characters) : nullptr;
    } else {
        text = ((void *(__fastcall *)(uint32_t))0xbd0490)(1);
    }
    *out = text;
    ((void(__thiscall *)(void *))0xbd1880)(reference);
    return out;
}

template <int K>
void **__thiscall file_button_copy(uint8_t *self, void **out) {
    return file_button(self, out, file_buttons[K]);
}
constexpr FileButton look_button = {0x98c210, 0xf7febc, 0xf7fed8, 0x20a, true};
const uint8_t look_button_first[] = {0x6a, 0xff, 0x68, 0xbf, 0xde, 0xe6, 0x00};
void **__thiscall look_button_tip(uint8_t *self, void **out) { return file_button(self, out, look_button); }
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(file_buttons[0].at == 0x9bdd70 && file_buttons[1].at == 0xacd350 && file_buttons[2].at == 0xae9330, "file_buttons.inc changed");
REWRITE("Designer::FileButtonTip", 0x9bdd70, file_buttons_first[0], (&file_button_copy<0>), &g_file_buttons);
REWRITE("Designer::FileButtonTip", 0xacd350, file_buttons_first[1], (&file_button_copy<1>), &g_file_buttons);
REWRITE("Designer::FileButtonTip", 0xae9330, file_buttons_first[2], (&file_button_copy<2>), &g_file_buttons);
REWRITE("Designer::FileButtonTip", 0x98c210, look_button_first, &look_button_tip, &g_file_buttons);
