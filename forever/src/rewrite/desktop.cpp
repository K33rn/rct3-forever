// RCTDesktop::CreateWindows (0x6249D0): the park desktop's constructor. It titles the desktop (when given a title: the
// game's own name, if it has one, joined with " - " and the title given, the result kept in the title; otherwise a copy
// of the default title), builds the base desktop (0x681360) for the description given or the default one, sets its
// tables, and then makes its window lists one after another: 152 calls, each making a window list member of the desktop
// with the file's name, a line and a global of the game it registers in (or a value, or a key combination). After them it
// makes two windows with new, two more window lists and a few plain members, marks the default description used, shows
// the desktop and sets its flag.
// The window lists are read from the game by tools/desktop_lists.py (which checks the stretch holds nothing else).
#include "rewrite.h"

namespace {
struct DesktopList {
    uint32_t kind, member, line, value;   // kind 0: 0x6B3190 with a global; 1: 0x6B3390 with a value; 2: 0x6B3570 with a key
};
const DesktopList desktop_lists[] = {
#include "desktop_lists.inc"
};

typedef void *(__cdecl *Alloc)(uint32_t);
typedef void(__cdecl *Free)(void *);
const uintptr_t FILE_NAME = 0xf445b8;   // the source file's name the window lists are given

// a shared text {count, ..., buffer at +0xC} let go: the count taken down; at 0 its buffer freed, then it
inline void text_let_go(int32_t *volatile *held) {
    --**held;
    int32_t *text = *held;   // read again, as the game does
    if (*(volatile int32_t *)text == 0) {
        ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
        ((Free)0xbce000)(text);
    }
}

void make_list(uint8_t *self, const DesktopList &l) {
    uint8_t *member = self + l.member;
    if (l.kind == 0) {
        ((void *(__thiscall *)(void *, uintptr_t, uint32_t, uint32_t, void *, uint32_t))0x6b3190)(member, FILE_NAME, l.line, 0, self, l.value);
    } else if (l.kind == 1) {
        ((void *(__thiscall *)(void *, uint32_t, void *, uint32_t))0x6b3390)(member, 0, self, l.value);
    } else {
        alignas(4) uint8_t key[0x20];   // the key combination, made on the stack from the one key
        ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))0xbd1940)(key, l.value, 0, 0, 0);
        ((void *(__thiscall *)(void *, uintptr_t, uint32_t, uint32_t, void *, void *))0x6b3570)(member, FILE_NAME, l.line, 0, self, key);
    }
}

volatile long g_desktop_creates;
uint8_t *__thiscall desktop_create_windows(uint8_t *self, uint8_t *description, int32_t **title) {
    g_desktop_creates++;
    uint8_t *shared = *(uint8_t *volatile *)0x12f7190;   // the default description
    void *named;                                         // the title the base desktop is given
    int32_t *volatile copy;
    if (title) {
        // the game's own name (a new text, or the empty one), joined with " - ", then with the title given
        const char *own = *(const char *volatile *)(shared + 8);
        int32_t *volatile first;
        if (own) {
            void *memory = ((Alloc)0xbcdfc0)(0x10);
            first = memory ? ((int32_t * (__thiscall *)(void *, const char *))0xbcf850)(memory, own) : nullptr;
        } else {
            first = ((int32_t * (__fastcall *)(uint32_t))0xbd0490)(1);
        }
        int32_t *volatile joined, *volatile whole;
        void *r = ((void *(__thiscall *)(void *, void *, uintptr_t))0x403be0)((void *)&first, (void *)&joined, 0xf2738c);
        int32_t **made = (int32_t **)((void *(__thiscall *)(void *, void *, void *))0x403bb0)(r, (void *)&whole, title);
        // kept in the title: the old text let go (unless it is the same one), the new one held
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
        text_let_go(&whole);
        text_let_go(&joined);
        text_let_go(&first);
        named = title;
    } else {
        named = ((void *(__thiscall *)(void *, void *))0x41c740)((void *)&copy, shared);   // a copy of the default title
    }
    ((void *(__thiscall *)(void *, void *, void *))0x681360)(self, description ? description : *(uint8_t *volatile *)0x12f7190, named);
    if (!title) text_let_go(&copy);
    *(volatile uint32_t *)(self + 0) = 0xf445f8;
    *(volatile uint32_t *)(self + 4) = 0xf445e4;
    *(volatile uint32_t *)(self + 0xd4) = 0xf445dc;
    ((void(__thiscall *)(void *))0xbd37a0)(self + 0x378);
    *(volatile uint32_t *)(self + 0x414) = 0;
    for (const DesktopList &l : desktop_lists) make_list(self, l);
    // two windows made with new (none without memory), the window lists after them and a few plain members
    void *memory = ((Alloc)0xbcdfc0)(0x150);
    *(void *volatile *)(self + 0x654c) = memory ? ((void *(__thiscall *)(void *, uint32_t, uint32_t))0x42abc0)(memory, 0, 0) : nullptr;
    *(volatile uint32_t *)(self + 0x6550) = 0;
    *(volatile uint32_t *)(self + 0x6554) = 0;
    *(volatile uint32_t *)(self + 0x6558) = 0;
    *(volatile uint32_t *)(self + 0x6560) = *(volatile uint32_t *)(self + 0xcbc);
    memory = ((Alloc)0xbcdfc0)(0x104);
    void *message = memory ? ((void *(__thiscall *)(void *, void *))0x4840b0)(memory, self) : nullptr;
    *(void *volatile *)(self + 0x6564) = message;
    *(volatile uint32_t *)(self + 0x6570) = 0;
    *(volatile uint32_t *)(self + 0x6574) = 0;
    *(volatile uint32_t *)(self + 0x6578) = 0;
    make_list(self, {0, 0x6580, 0x29a, 0x12f7d20});
    make_list(self, {0, 0x6624, 0x29b, 0x12f7760});
    *(volatile uint32_t *)(self + 0x66c8) = 0;
    *(volatile uint32_t *)(self + 0x66d0) = *(volatile uint32_t *)0x13d29c0;
    *(volatile uint32_t *)(self + 0x66d4) = *(volatile uint32_t *)0x13d29c4;
    *(volatile uint32_t *)(self + 0x66d8) = *(volatile uint32_t *)0x13d29c8;
    *(volatile uint32_t *)(self + 0x66e0) = 0xbf800000;   // -1.0f
    *(volatile uint32_t *)(self + 0x66e4) = 0;
    memory = ((Alloc)0xbcdfc0)(0x10);
    *(void *volatile *)(self + 0x66e8) = memory ? ((void *(__thiscall *)(void *, uintptr_t))0xbcf850)(memory, 0xf264ad) : nullptr;   // an empty text
    if (!description) (*(uint8_t *volatile *)0x12f7190)[4] = 1;   // the default description used
    ((void(__thiscall *)(void *, uint32_t))0xbd31f0)(self + 0x378, 1);
    *(volatile uint32_t *)(self + 0x94) |= 0x200000;
    return self;
}
const uint8_t desktop_create_windows_original[] = {0x6a, 0xff, 0x68, 0xdd, 0xd8, 0xe2, 0x00, 0x64, 0xa1};
}   // namespace
REWRITE("RCTDesktop::CreateWindows", 0x6249d0, desktop_create_windows_original, &desktop_create_windows, &g_desktop_creates);
