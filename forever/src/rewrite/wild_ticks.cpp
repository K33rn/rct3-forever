// WildAnimal::Tick10, Tick5 and Tick4 (one function written out three times; tools/family.py: wild_ticks.inc; they
// differ in their counter, how often they act and the message): every Nth time, tell the animal manager's target
// about the animal (thiscall; gives what the game leaves in eax).
//   - The counter (+0x254, +0x258, +0x25C) counted up; below N (10, 5, 4: signed) nothing more (gives the count).
//   - Counter cleared; the animal manager ([0x1306C68]) made when there is none (0x58 new bytes, 0x7A70A0 with 0,
//     0, 0); its target (0x7A66B0); none: nothing more (gives 0).
//   - The target's slot 0xE0 called with the copy's message, a reference to the animal and a reference to nothing,
//     both passed by value (made where the call reads them, the one to nothing first: in_place_call.h).
#include "in_place_call.h"

namespace {
struct WildTick {
    uint32_t at, handler, counter, every, message;
};
#define WILD_TICKS(at, handler, counter, every, message, ...) {at, handler, counter, every, message},
constexpr WildTick wild_ticks[] = {
#include "wild_ticks.inc"
};
#undef WILD_TICKS
#define WILD_TICKS(at, handler, counter, every, message, ...) {__VA_ARGS__},
const uint8_t wild_ticks_first[][7] = {
#include "wild_ticks.inc"
};
#undef WILD_TICKS

struct TellArguments {
    uint32_t message;
    void *animal;
};
void __cdecl fill_tell(uint8_t *area, void *context) {
    const TellArguments &a = *(const TellArguments *)context;
    in_place::reference_null((uint32_t *)(area + 4 + 0x14));   // (made first, as the game does)
    in_place::reference_to((uint32_t *)(area + 4), a.animal);
    *(uint32_t *)area = a.message;
}

volatile long g_wild_ticks;
uint32_t wild_tick(uint8_t *self, const WildTick &kind) {
    g_wild_ticks++;
    const int32_t count = *(volatile int32_t *)(self + kind.counter) + 1;
    *(volatile int32_t *)(self + kind.counter) = count;
    if (count < (int32_t)kind.every) return (uint32_t)count;
    *(volatile int32_t *)(self + kind.counter) = 0;
    if (!*(void *volatile *)(uintptr_t)0x1306c68) {
        void *memory = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x58);
        if (memory) ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))0x7a70a0)(memory, 0, 0, 0);
    }
    uint8_t *target = ((uint8_t * (__thiscall *)(void *))0x7a66b0)(*(void *volatile *)(uintptr_t)0x1306c68);
    if (!target) return 0;
    TellArguments a = {kind.message, self};
    return call_in_place((uintptr_t)(*(void ***)target)[0xe0 / 4], target, 4 + 2 * 0x14, fill_tell, &a);
}

template <int K> uint32_t __fastcall wild_tick_copy(uint8_t *self) { return wild_tick(self, wild_ticks[K]); }
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(wild_ticks[0].at == 0xb38f30 && wild_ticks[1].at == 0xb39040 && wild_ticks[2].at == 0xb39150, "wild_ticks.inc changed");
REWRITE("WildAnimal::Tick", 0xb38f30, wild_ticks_first[0], (&wild_tick_copy<0>), &g_wild_ticks);
REWRITE("WildAnimal::Tick", 0xb39040, wild_ticks_first[1], (&wild_tick_copy<1>), &g_wild_ticks);
REWRITE("WildAnimal::Tick", 0xb39150, wild_ticks_first[2], (&wild_tick_copy<2>), &g_wild_ticks);
