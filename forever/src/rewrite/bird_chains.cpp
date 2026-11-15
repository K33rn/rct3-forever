// Bird::CheckA113D0, A114A0, A14BF0 and A14CC0 (one function written out four times; tools/family.py:
// bird_chains.inc; they differ in the class's offsets and its class number): is any object up the chain marked
// (thiscall; gives al: 1 or 0)?
//   - The object's parent interface (+kept, looked up once through the reference at +ref with the class number and
//     kept, +flag set; as farther_point.cpp's), then that one's, and so on: the first with its mark byte set
//     (+0x15C, or +0x14C) gives 1; the end of the chain gives 0. The object itself is not asked.
#include "rewrite.h"

namespace {
struct BirdChain {
    uint32_t at, flag, ref, number, kept, mark;
};
#define BIRD_CHAINS(at, flag, ref, number, kept, mark, ...) {at, flag, ref, number, kept, mark},
constexpr BirdChain bird_chains[] = {
#include "bird_chains.inc"
};
#undef BIRD_CHAINS
#define BIRD_CHAINS(at, flag, ref, number, kept, mark, ...) {__VA_ARGS__},
const uint8_t bird_chains_first[][7] = {
#include "bird_chains.inc"
};
#undef BIRD_CHAINS

// An interface kept by its holder: looked up once (the flag set), then read back (as farther_point.cpp's).
uint8_t *interface_of(uint8_t *reference, uint32_t class_global, uint8_t *volatile *kept, volatile uint8_t *looked) {
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
inline uint8_t *parent(uint8_t *object, const BirdChain &kind) {
    return interface_of(*(uint8_t **)(object + kind.ref), kind.number, (uint8_t * volatile *)(object + kind.kept), object + kind.flag);
}

volatile long g_bird_chains;
bool bird_chain(uint8_t *self, const BirdChain &kind) {
    g_bird_chains++;
    for (uint8_t *up = parent(self, kind); up; up = parent(up, kind))
        if (up[kind.mark]) return true;
    return false;
}

template <int K> bool __fastcall bird_chain_copy(uint8_t *self) { return bird_chain(self, bird_chains[K]); }
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(bird_chains[0].at == 0xa113d0 && bird_chains[1].at == 0xa114a0 && bird_chains[2].at == 0xa14bf0 && bird_chains[3].at == 0xa14cc0, "bird_chains.inc changed");
REWRITE("Bird::MarkedAbove", 0xa113d0, bird_chains_first[0], (&bird_chain_copy<0>), &g_bird_chains);
REWRITE("Bird::MarkedAbove", 0xa114a0, bird_chains_first[1], (&bird_chain_copy<1>), &g_bird_chains);
REWRITE("Bird::MarkedAbove", 0xa14bf0, bird_chains_first[2], (&bird_chain_copy<2>), &g_bird_chains);
REWRITE("Bird::MarkedAbove", 0xa14cc0, bird_chains_first[3], (&bird_chain_copy<3>), &g_bird_chains);
