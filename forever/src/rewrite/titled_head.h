// The way the game's windows take their title, inlined at the start of hundreds of window constructors (the ident
// bars, tool windows, species lists, ...). Given the window's default description (a game global) and a title (or
// none):
//   - with a title: the game's name (the default description's C text at +8, as a wide text in 0x10 new bytes,
//     none without memory; or the shared empty text when it has none), " - " and the title joined, and the title made
//     that (the old text let go when it is another; read again, as the game does), the parts let go;
//   - without one: a copy of the default description's title (0x41C740) in `copy`.
// It gives what the base constructor is handed as the title; after the base, title_done lets the copy go.
#pragma once
#include "rewrite.h"

namespace titled {
typedef void(__cdecl *Free)(void *);
inline void free_text(int32_t *text) {
    ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
    ((Free)0xbce000)(text);
}
inline void let_go(int32_t *volatile *held) {   // a shared text let go (read again, as the game does)
    --**held;
    int32_t *text = *held;
    if (*(volatile int32_t *)text == 0) free_text(text);
}
inline void *title_for(uint32_t description_global, int32_t **title, int32_t *volatile *copy) {
    uint8_t *shared = *(uint8_t *volatile *)(uintptr_t)description_global;
    if (!title) return ((void *(__thiscall *)(void *, void *))0x41c740)((void *)copy, shared);
    const char *own = *(const char *volatile *)(shared + 8);
    int32_t *volatile first;
    if (own) {
        void *memory = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x10);
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
        if (*(volatile int32_t *)again == 0 && again) free_text((int32_t *)again);
        int32_t *now = *(int32_t *volatile *)made;
        *(int32_t *volatile *)title = now;
        ++*(volatile int32_t *)now;
    }
    let_go(&whole);
    let_go(&joined);
    let_go(&first);
    return title;
}
inline void title_done(int32_t **title, int32_t *volatile *copy) {
    if (!title) let_go(copy);
}
// the description the window is for: the one given, or the default
inline uint8_t *description_or_default(uint8_t *description, uint32_t description_global) {
    return description ? description : *(uint8_t *volatile *)(uintptr_t)description_global;
}
}   // namespace titled
