// The designers' tool windows (0xACD9B0 laser elevation, 0xAE9830 water-jet size, 0xAEA730 laser, 0xAEB4F0 water-jet
// pressure: one function written out four times; tools/family.py: tool_windows.inc; they differ in their default
// description, tables, three numbers for the value control, a count, the unit text and the tooltip's name). Given a
// description (or none: the default), a title (or none), an object and a part, the window:
//   - titled and built as the ident bars are (ident_bars.cpp: the game's name, " - ", the title kept in the title;
//     or a copy of the description's title), with base 0x67DAC0 (the description, that title, 1);
//   - given its tables (+0xDC 0xF270F4 and +0xE0 0 and +0xE4 0xF93E50 first, then the copy's four), and a reference
//     member (+0xE8) to the object (Handle::For's handle, joined first in its list; class 0xF93B58, the object kept,
//     its flag set); the part kept at +0x104;
//   - two controls made (0x118 bytes, 0x6AB2A0 with 0, 0, 0x210, at +0x114; 0x10C bytes, 0x6A8D00 with 0, 0, 0x5800,
//     at +0x118; none without memory); the part told (its slot 0x100: 1 and the window's +0xE4); both placed
//     (0x6B1330, 6, the window); the second given the copy's three numbers (0x6A82D0) and count twice (0x6A7920), and
//     the reference's interface + 0x40 (looked up once, class [0x13CE434]; 0x6A8300); the first given the unit text
//     (0x4D7E70);
//   - its tooltip: the copy's name looked up (0x4040F0 with the game's text table and 1), its wide characters made a
//     text (or the empty text when there are none) given to the window (0x6AE070) and let go;
//   - an OK button (+0x11C; 0x13C bytes; none without memory): "Ok" as a wide text, looked up for the window with
//     the default description (0x6A39D0), the button built with them (0x67BE80: 0, the text, 0x14, 1); then the two
//     texts let go as far as they were made; the button set (0x67A7A0 1; 0x67B4B0 0x1304F94, 0; 0x67B800 the value
//     at [0x13038DC], 1), given its named text ("LaserWritingDesigner_Ok" at 0xF940F0) and laid out (0x6B1640, 6,
//     the window, {K - its width, 5, K - its width + its width again, its height + K2}; K and K2 the game's float
//     constants at 0xF27600 and 0xF28214, the sums on the FPU).
#include "rewrite.h"

namespace {
struct ToolWindow {
    uint32_t at, description, table0, table4, table_dc, table_e4, n1, n2, n3, count, unit, tooltip;
};
#define TOOL_WINDOWS(at, description, t0, t4, tdc, te4, n1, n2, n3, count, unit, tooltip, ...) {at, description, t0, t4, tdc, te4, n1, n2, n3, count, unit, tooltip},
constexpr ToolWindow tool_windows[] = {
#include "tool_windows.inc"
};
#undef TOOL_WINDOWS
#define TOOL_WINDOWS(at, description, t0, t4, tdc, te4, n1, n2, n3, count, unit, tooltip, ...) {__VA_ARGS__},
const uint8_t tool_windows_first[][7] = {
#include "tool_windows.inc"
};
#undef TOOL_WINDOWS

typedef void *(__cdecl *Alloc)(uint32_t);
typedef void(__cdecl *Free)(void *);
const Alloc alloc = (Alloc)0xbcdfc0;
typedef long double ext;
inline float rf(ext x) {
    volatile float f = (float)x;
    return f;
}
inline void let_go(int32_t *volatile *held) {   // a shared text let go (read again, as the game does)
    --**held;
    int32_t *text = *held;
    if (*(volatile int32_t *)text == 0) {
        ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
        ((Free)0xbce000)(text);
    }
}
uint8_t *interface_of(uint8_t *reference, uint32_t class_global, uint8_t *volatile *kept, volatile uint8_t *looked) {   // as farther_point.cpp's
    if (!*looked) {
        uint8_t *object = *(uint8_t **)(reference + 0xc), *found = nullptr;
        if (object) {
            uint8_t *info = ((uint8_t * (__thiscall *)(void *))(*(void ***)object)[1])(object);
            if (info) {
                const int32_t number = *(volatile int32_t *)(uintptr_t)class_global;
                if (number < *(int32_t *)(info + 0x18)) {
                    uint8_t *entry = (*(uint8_t ***)(info + 0x14))[number];
                    if (entry) found = object + *(int32_t *)(entry + 0x10);
                }
            }
        }
        *kept = found;
        *looked = 1;
    }
    return *kept;
}
typedef void *(__thiscall *TextRef)(void *, uintptr_t, uint32_t, uint32_t);

volatile long g_tool_windows;
uint8_t *tool_window(uint8_t *self, uint8_t *description, int32_t **title, void *object, uint8_t *part, const ToolWindow &kind) {
    g_tool_windows++;
    // the title, as the ident bars' (and the desktop's)
    uint8_t *shared = *(uint8_t *volatile *)(uintptr_t)kind.description;
    void *named;
    int32_t *volatile copy;
    if (title) {
        const char *own = *(const char *volatile *)(shared + 8);
        int32_t *volatile first;
        if (own) {
            void *memory = alloc(0x10);
            first = memory ? ((int32_t * (__thiscall *)(void *, const char *))0xbcf850)(memory, own) : nullptr;
        } else {
            first = ((int32_t * (__fastcall *)(uint32_t))0xbd0490)(1);
        }
        int32_t *volatile joined, *volatile whole;
        void *r = ((void *(__thiscall *)(void *, void *, uintptr_t))0x403be0)((void *)&first, (void *)&joined, 0xf2738c);   // " - "
        int32_t **made = (int32_t **)((void *(__thiscall *)(void *, void *, void *))0x403bb0)(r, (void *)&whole, title);
        int32_t *old = *(int32_t *volatile *)title;
        if (*(int32_t *volatile *)made != old) {
            --*(volatile int32_t *)old;
            uintptr_t again = (uintptr_t) * (int32_t *volatile *)title;
            if (*(volatile int32_t *)again == 0 && again) {
                ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)((int32_t *)again)[3]);
                ((Free)0xbce000)((void *)again);
            }
            int32_t *now = *(int32_t *volatile *)made;
            *(int32_t *volatile *)title = now;
            ++*(volatile int32_t *)now;
        }
        let_go(&whole);
        let_go(&joined);
        let_go(&first);
        named = title;
    } else {
        named = ((void *(__thiscall *)(void *, void *))0x41c740)((void *)&copy, shared);
    }
    ((void *(__thiscall *)(void *, void *, void *, uint32_t))0x67dac0)(self, description ? description : *(uint8_t *volatile *)(uintptr_t)kind.description, named, 1);
    if (!title) let_go(&copy);
    // tables and the reference to the object
    *(volatile uint32_t *)(self + 0xdc) = 0xf270f4;
    *(volatile uint32_t *)(self + 0xe0) = 0;
    *(volatile uint32_t *)(self + 0xe4) = 0xf93e50;
    *(volatile uint32_t *)(self + 0) = kind.table0;
    *(volatile uint32_t *)(self + 4) = kind.table4;
    *(volatile uint32_t *)(self + 0xdc) = kind.table_dc;
    *(volatile uint32_t *)(self + 0xe4) = kind.table_e4;
    uint32_t *member = (uint32_t *)(self + 0xe8);
    member[0] = 0xf26ae0;
    member[4] = (uint32_t)(uintptr_t)member;
    uint8_t *handle = ((uint8_t * (__fastcall *)(void *))0x805700)(object);
    member[1] = (uint32_t)(uintptr_t)handle;
    uint32_t *list = *(uint32_t **)(handle + 0x20);
    member[2] = list[0];
    ((uint32_t *)(uintptr_t)list[0])[1] = (uint32_t)(uintptr_t)&member[2];
    member[3] = (uint32_t)(uintptr_t)list;
    list[0] = (uint32_t)(uintptr_t)&member[2];
    member[0] = 0xf93b58;
    member[5] = (uint32_t)(uintptr_t)object;
    *(volatile uint8_t *)&member[6] = 1;
    *(uint8_t *volatile *)(self + 0x104) = part;
    // the two controls
    void *memory = alloc(0x118);
    *(void *volatile *)(self + 0x114) = memory ? ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))0x6ab2a0)(memory, 0, 0, 0x210) : nullptr;
    memory = alloc(0x10c);
    *(void *volatile *)(self + 0x118) = memory ? ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))0x6a8d00)(memory, 0, 0, 0x5800) : nullptr;
    uint8_t *told = *(uint8_t **)(self + 0x104);
    ((void(__thiscall *)(void *, uint32_t, void *))(*(void ***)told)[0x100 / 4])(told, 1, self + 0xe4);
    typedef void(__thiscall * Place)(void *, uint32_t, void *);
    ((Place)0x6b1330)(*(void **)(self + 0x114), 6, self);
    ((Place)0x6b1330)(*(void **)(self + 0x118), 6, self);
    ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))0x6a82d0)(*(void **)(self + 0x118), kind.n3, kind.n2, kind.n1);
    ((void(__thiscall *)(void *, uint32_t, uint32_t))0x6a7920)(*(void **)(self + 0x118), kind.count, kind.count);
    uint8_t *to = interface_of((uint8_t *)(uintptr_t)member[1], 0x13ce434, (uint8_t * volatile *)&member[5], (volatile uint8_t *)&member[6]);
    ((void(__thiscall *)(void *, void *))0x6a8300)(*(void **)(self + 0x118), to + 0x40);
    ((void(__thiscall *)(void *, uintptr_t))0x4d7e70)(*(void **)(self + 0x114), kind.unit);
    // the tooltip
    {
        alignas(4) uint8_t reference[0x18];
        const uint8_t *found = (const uint8_t *)((TextRef)0x4040f0)(reference, kind.tooltip, 0x1303cfc, 1);
        const uint16_t *wide = *(const uint16_t *volatile *)(found + 8);
        int32_t *volatile tip;
        if (wide) {
            void *m = alloc(0x10);
            tip = m ? ((int32_t * (__thiscall *)(void *, const uint16_t *))0xbcf7d0)(m, wide) : nullptr;
        } else {
            tip = ((int32_t * (__fastcall *)(uint32_t))0xbd0490)(1);
        }
        ((void(__thiscall *)(void *, void *))0x6ae070)(self, (void *)&tip);
        let_go(&tip);
        ((void(__thiscall *)(void *))0xbd1880)(reference);
    }
    // the OK button
    uint8_t *button = nullptr;
    if (void *b = alloc(0x13c)) {
        void *m = alloc(0x10);
        int32_t *volatile ok = m ? ((int32_t * (__thiscall *)(void *, uintptr_t))0xbcf850)(m, 0xf94108) : nullptr;   // "Ok"
        int32_t *volatile looked;
        void *r = ((void *(__thiscall *)(void *, void *, void *, uint32_t))0x6a39d0)((void *)&looked, (void *)&ok, self,
                                                                                  *(volatile uint32_t *)(uintptr_t)kind.description);
        button = (uint8_t *)((void *(__thiscall *)(void *, uint32_t, void *, uint32_t, uint32_t))0x67be80)(b, 0, r, 0x14, 1);
        *(uint8_t *volatile *)(self + 0x11c) = button;
        let_go(&looked);
        let_go(&ok);
    } else {
        *(uint8_t *volatile *)(self + 0x11c) = nullptr;
    }
    ((void(__thiscall *)(void *, uint32_t))0x67a7a0)(*(void **)(self + 0x11c), 1);
    ((void(__thiscall *)(void *, uint32_t, uint32_t))0x67b4b0)(*(void **)(self + 0x11c), 0x1304f94, 0);
    ((void(__thiscall *)(void *, uint32_t, uint32_t))0x67b800)(*(void **)(self + 0x11c), *(volatile uint32_t *)(uintptr_t)0x13038dc, 1);
    {
        alignas(4) uint8_t reference[0x18];
        void *made = ((TextRef)0x4040f0)(reference, 0xf940f0, 0x1303cfc, 1);   // "LaserWritingDesigner_Ok"
        ((void(__thiscall *)(void *, void *))0xbd14e0)(*(uint8_t **)(self + 0x11c) + 0x9c, made);
        ((void(__thiscall *)(void *))0xbd1880)(reference);
    }
    const float *b = (const float *)*(uint8_t **)(self + 0x11c);
    const ext k = *(const float *)(uintptr_t)0xf27600, k2 = *(const float *)(uintptr_t)0xf28214;
    const ext left = k - ((ext)b[0x84 / 4] - b[0x7c / 4]);
    const ext width = (ext)b[0x84 / 4] - b[0x7c / 4];
    volatile float rect[4];
    rect[0] = rf(left);
    rect[1] = 5.0f;
    rect[2] = rf(width + left);
    rect[3] = rf(((ext)b[0x88 / 4] - b[0x80 / 4]) + k2);
    ((void(__thiscall *)(void *, uint32_t, void *, volatile float *))0x6b1640)(*(void **)(self + 0x11c), 6, self, rect);
    return self;
}

template <int K> uint8_t *__thiscall tool_window_copy(uint8_t *self, uint8_t *description, int32_t **title, void *object, uint8_t *part) {
    return tool_window(self, description, title, object, part, tool_windows[K]);
}
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(tool_windows[0].at == 0xacd9b0 && tool_windows[1].at == 0xae9830 && tool_windows[2].at == 0xaea730 && tool_windows[3].at == 0xaeb4f0, "tool_windows.inc changed");
REWRITE("Designer::BuildTool", 0xacd9b0, tool_windows_first[0], (&tool_window_copy<0>), &g_tool_windows);
REWRITE("Designer::BuildTool", 0xae9830, tool_windows_first[1], (&tool_window_copy<1>), &g_tool_windows);
REWRITE("Designer::BuildTool", 0xaea730, tool_windows_first[2], (&tool_window_copy<2>), &g_tool_windows);
REWRITE("Designer::BuildTool", 0xaeb4f0, tool_windows_first[3], (&tool_window_copy<3>), &g_tool_windows);
