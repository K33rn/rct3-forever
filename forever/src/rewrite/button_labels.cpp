// MessageBox::BuildOkCancel and TrackBrowser::BuildButtons (two copies) (one function written out three times;
// tools/family.py: button_labels.inc; they differ in where the two buttons are and their labels' names): the label of
// the button that is lit (thiscall: the place for the text; gives it).
//   - When the first button (+0x114, +0x144, +0x104) has bit 5 of its +0x94 set: its label (the copy's name, e.g.
//     "SMB_Label_OK") looked up (0x4040F0 with the game's text table and 1): a new text of its wide characters (0x10
//     new bytes, WideString::FromWide; none without memory), or the shared empty text when it has none; the lookup
//     destroyed (0xBD1880).
//   - Otherwise the same for the second button ("SMB_Label_Cancel", ...).
//   - Otherwise a new text of "" (0x10 new bytes, WideString from C; none without memory).
#include "rewrite.h"

namespace {
struct ButtonLabel {
    uint32_t at, first, first_name, second, second_name;
};
#define BUTTON_LABELS(at, first, first_name, second, second_name, ...) {at, first, first_name, second, second_name},
constexpr ButtonLabel button_labels[] = {
#include "button_labels.inc"
};
#undef BUTTON_LABELS
#define BUTTON_LABELS(at, first, first_name, second, second_name, ...) {__VA_ARGS__},
const uint8_t button_labels_first[][7] = {
#include "button_labels.inc"
};
#undef BUTTON_LABELS
typedef void *(__cdecl *Alloc)(uint32_t);

void label_of(uintptr_t name, void **out) {   // (the text kept in out before the lookup is destroyed, as the game does)
    alignas(4) uint8_t reference[0x18];
    const uint8_t *found = (const uint8_t *)((void *(__thiscall *)(void *, uintptr_t, uint32_t, uint32_t))0x4040f0)(reference, name, 0x1303cfc, 1);
    const uint16_t *wide = *(const uint16_t *volatile *)(found + 8);
    void *text;
    if (wide) {
        void *memory = ((Alloc)0xbcdfc0)(0x10);
        text = memory ? ((void *(__thiscall *)(void *, const uint16_t *))0xbcf7d0)(memory, wide) : nullptr;
    } else {
        text = ((void *(__fastcall *)(uint32_t))0xbd0490)(1);
    }
    *out = text;
    ((void(__thiscall *)(void *))0xbd1880)(reference);
}
inline bool lit(uint8_t *self, uint32_t at) { return (*(uint32_t *)(*(uint8_t **)(self + at) + 0x94) >> 5) & 1; }

volatile long g_button_labels;
void **button_label(uint8_t *self, void **out, const ButtonLabel &kind) {
    g_button_labels++;
    if (lit(self, kind.first)) {
        label_of(kind.first_name, out);
        return out;
    }
    if (lit(self, kind.second)) {
        label_of(kind.second_name, out);
        return out;
    }
    void *memory = ((Alloc)0xbcdfc0)(0x10);
    *out = memory ? ((void *(__thiscall *)(void *, uintptr_t))0xbcf850)(memory, 0xf264ad) : nullptr;   // ""
    return out;
}

template <int K> void **__thiscall button_label_copy(uint8_t *self, void **out) { return button_label(self, out, button_labels[K]); }
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(button_labels[0].at == 0x65d9b0 && button_labels[1].at == 0x66b660 && button_labels[2].at == 0xa824f0, "button_labels.inc changed");
REWRITE("Buttons::LitLabel", 0x65d9b0, button_labels_first[0], (&button_label_copy<0>), &g_button_labels);
REWRITE("Buttons::LitLabel", 0x66b660, button_labels_first[1], (&button_label_copy<1>), &g_button_labels);
REWRITE("Buttons::LitLabel", 0xa824f0, button_labels_first[2], (&button_label_copy<2>), &g_button_labels);
