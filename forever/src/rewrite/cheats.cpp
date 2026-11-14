// Cheats::Init (0x410ED0): the constructor of the table of the 32 cheats a guest's name switches on. The table holds
// 32 entries (0x80 bytes). For each cheat: the text "CheatPeepName_" is made, the cheat's own ending added to it (a new
// text: the name's key), the first text let go, and an entry of 0x54 bytes made from the name and the cheat's number
// (none without memory) and put in the table. At the end the 32 names are let go, last to first.
// The game has that written out 32 times; here it is a loop over the endings tools/cheat_names.py reads from the game
// (their addresses: the texts stay in the game).
#include "rewrite.h"

static const uint32_t cheat_endings[32] = {
#include "cheats.inc"
};

// a shared text {count, ..., buffer at +0xC} let go: the count taken down; at 0 its buffer freed, then it
static inline void text_release(int32_t *volatile *held) {
    --**held;
    int32_t *text = *held;   // read again, as the game does
    if (*(volatile int32_t *)text == 0) {
        ((void(__cdecl *)(void *))0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
        ((void(__cdecl *)(void *))0xbce000)(text);
    }
}

static volatile long g_cheat_inits;
static uint32_t *__thiscall cheats_init(uint32_t *self, uint32_t) {
    g_cheat_inits++;
    typedef void *(__cdecl *Alloc)(uint32_t);
    *(volatile uint32_t *)&self[0] = 0xf26b98;
    *(volatile uint32_t *)&self[2] = 0x20;
    *(volatile uint32_t *)&self[1] = (uint32_t)(uintptr_t)((Alloc)0xbcdfd0)(0x80);
    int32_t *volatile names[32];
    for (uint32_t k = 0; k < 32; k++) {
        int32_t *volatile prefix = (int32_t *)((Alloc)0xbcdfc0)(0x10);
        if (prefix) prefix = ((int32_t * (__thiscall *)(void *, const char *))0xbcf850)(prefix, (const char *)0xf26d30);
        ((void *(__thiscall *)(void *, void *, const char *))0x403be0)((void *)&prefix, (void *)&names[k], (const char *)(uintptr_t)cheat_endings[k]);
        text_release(&prefix);
        void *entry = ((Alloc)0xbcdfc0)(0x54);
        if (entry) {
            int32_t *name = names[k];   // given by value: one more holder
            ++*(volatile int32_t *)name;
            entry = ((void *(__thiscall *)(void *, uint32_t, int32_t *, uint32_t))0x410d50)(entry, 0, name, k);
        }
        ((void *volatile *)(uintptr_t)self[1])[k] = entry;
    }
    for (int k = 31; k >= 0; k--) text_release(&names[k]);
    return self;
}
static const uint8_t cheats_init_original[] = {0x6a, 0xff, 0x68, 0x1c, 0x61, 0xe0, 0x00, 0x64, 0xa1};
REWRITE("Cheats::Init", 0x410ed0, cheats_init_original, &cheats_init, &g_cheat_inits);
