// Window::Name6C5D50, Window::Name6C5E50, Window::Name6C5F50 and Window::Name6C6050 (one function written out four
// times; tools/family.py: shown_names.inc; they differ in which name field they use, +0x7C to +0x88): the name to
// show for a field (thiscall: the place for the text; gives it).
//   - A text reference looked up by a copy of the field's text (NamedTextRef's constructor 0x410C90 with the game's
//     text table and 1; the copy let go).
//   - When the reference holds a text (0xBD1420 with 0): a new text from its wide characters (0x10 new bytes,
//     WideString::FromWide; none without memory), or the shared empty text when it has no characters.
//   - Otherwise the field's own text, taken.
//   - The reference destroyed (0xBD1880).
#include "rewrite.h"

namespace {
struct ShownName {
    uint32_t at, field;
};
#define SHOWN_NAMES(at, field, ...) {at, field},
constexpr ShownName shown_names[] = {
#include "shown_names.inc"
};
#undef SHOWN_NAMES
#define SHOWN_NAMES(at, field, ...) {__VA_ARGS__},
const uint8_t shown_names_first[][7] = {
#include "shown_names.inc"
};
#undef SHOWN_NAMES
typedef void(__cdecl *Free)(void *);
inline void let_go(int32_t *volatile *held) {   // a shared text let go (read again, as the game does)
    --**held;
    int32_t *text = *held;
    if (*(volatile int32_t *)text == 0) {
        ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
        ((Free)0xbce000)(text);
    }
}

volatile long g_shown_names;
int32_t **shown_name(uint8_t *self, int32_t **out, const ShownName &kind) {
    g_shown_names++;
    int32_t **field = (int32_t **)(self + kind.field);
    int32_t *volatile copy;
    ((void *(__thiscall *)(void *, void *))0xbd01c0)((void *)&copy, field);
    alignas(4) uint8_t reference[0x18];
    ((void *(__thiscall *)(void *, void *, uintptr_t, uint32_t))0x410c90)(reference, (void *)&copy, 0x1303cfc, 1);
    let_go(&copy);
    if (((uint8_t(__thiscall *)(void *, uint32_t))0xbd1420)(reference, 0)) {
        const uint16_t *wide = *(const uint16_t *volatile *)(reference + 8);
        if (wide) {
            void *memory = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x10);
            *out = memory ? ((int32_t * (__thiscall *)(void *, const uint16_t *))0xbcf7d0)(memory, wide) : nullptr;
        } else {
            *out = ((int32_t * (__fastcall *)(uint32_t))0xbd0490)(1);
        }
    } else {
        int32_t *own = *field;
        *out = own;
        ++*own;
    }
    ((void(__thiscall *)(void *))0xbd1880)(reference);
    return out;
}

template <int K> int32_t **__thiscall shown_name_copy(uint8_t *self, int32_t **out) { return shown_name(self, out, shown_names[K]); }
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(shown_names[0].at == 0x6c5e50 && shown_names[1].at == 0x6c5f50 && shown_names[2].at == 0x6c6050 && shown_names[3].at == 0x6c5d50,
              "shown_names.inc changed");
REWRITE("Window::ShownName", 0x6c5e50, shown_names_first[0], (&shown_name_copy<0>), &g_shown_names);
REWRITE("Window::ShownName", 0x6c5f50, shown_names_first[1], (&shown_name_copy<1>), &g_shown_names);
REWRITE("Window::ShownName", 0x6c6050, shown_names_first[2], (&shown_name_copy<2>), &g_shown_names);
REWRITE("Window::ShownName", 0x6c5d50, shown_names_first[3], (&shown_name_copy<3>), &g_shown_names);
