// The two colour tables' initialisers (0xF05CA0: 2,720 colours at 0x13F7188; 0xF047D0: 256 at 0x13F9C08). Each is one
// call written out per colour: three values pushed, the entry's address, Colour::Set (0xCC4D60: stores them as blue,
// green, red, 0), and a stack check (the debug runtime's) at the end, which always passes.
//
// The colours themselves are not copied here. They are the pushed values in the game's own code, which stays in memory
// when the function is replaced: this reads each entry's three values and its address from there, as far as the
// closing stack check. Replacing the function overwrites its first five bytes (with the jump to this); those are read
// from the first bytes kept for checking it.
#include "rewrite.h"

static volatile long g_colour_fills;
static void *colours_fill(const uint8_t *code, const uint8_t *first) {
    g_colour_fills++;
    auto byte = [&](uint32_t at) -> uint32_t { return at < 5 ? first[at] : *(const volatile uint8_t *)(code + at); };
    auto value = [&](uint32_t at) -> uint32_t { return byte(at) | byte(at + 1) << 8 | byte(at + 2) << 16 | byte(at + 3) << 24; };
    void *last = nullptr;
    uint32_t at = 3;   // after "push ebp; mov ebp, esp"
    while (byte(at) != 0x3b) {   // until "cmp ebp, esp"
        uint32_t pushed[3];
        for (uint32_t &v : pushed) {
            if (byte(at) == 0x6a) {   // push of one byte, sign-extended
                v = (uint32_t)(int32_t)(int8_t)byte(at + 1);
                at += 2;
            } else if (byte(at) == 0x68) {   // push of four bytes
                v = value(at + 1);
                at += 5;
            } else {
                __asm__ volatile("int3");   // not the code this was written for
                return last;
            }
        }
        if (byte(at) != 0xb9 || byte(at + 5) != 0xe8) {   // mov ecx, entry; call Colour::Set
            __asm__ volatile("int3");
            return last;
        }
        void *entry = (void *)(uintptr_t)value(at + 1);
        at += 10;
        last = ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))0xcc4d60)(entry, pushed[2], pushed[1], pushed[0]);
    }
    return last;
}
static const uint8_t colours_fill_original[] = {0x55, 0x8b, 0xec, 0x68, 0xd7, 0x00, 0x00, 0x00, 0x68, 0xd7};
static void *__cdecl colours_fill_main() { return colours_fill((const uint8_t *)0xf05ca0, colours_fill_original); }
REWRITE("Colours::Fill", 0xf05ca0, colours_fill_original, &colours_fill_main, &g_colour_fills);
static const uint8_t colours_fill256_original[] = {0x55, 0x8b, 0xec, 0x6a, 0x00, 0x68, 0xff, 0x00, 0x00, 0x00};
static void *__cdecl colours_fill_256() { return colours_fill((const uint8_t *)0xf047d0, colours_fill256_original); }
REWRITE("Colours::Fill256", 0xf047d0, colours_fill256_original, &colours_fill_256, &g_colour_fills);
