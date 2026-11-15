// Scenery::Step848840, Scenery::Step8489B0 and Scenery::Step848B20 (one function written out three times;
// tools/family.py: scenery_steps.inc; they differ in where the value is kept and the step called): set one of a
// scenery's values and pass it on (thiscall: which, the value; gives what the step gives, 0 without one).
//   - The value kept at the copy's first place (+0x3C, +0x40, +0x44) for which 0, otherwise at its second (+0x48,
//     +0x4C, +0x50).
//   - The scenery's interface (+0x68) looked up once and kept (+0x6C set; as farther_point.cpp's: the object its
//     reference at +0x58 points to (+0xC), its class info (slot 1), the entry for class number [0x130B5C8] giving the
//     offset), and when there is one, the copy's step (0x82F9C0, 0x82FA30, 0x82FAB0) called on it with which and
//     the value.
#include "rewrite.h"

namespace {
struct SceneryStep {
    uint32_t at, first, step, second;
};
#define SCENERY_STEPS(at, first, step, second, ...) {at, first, step, second},
constexpr SceneryStep scenery_steps[] = {
#include "scenery_steps.inc"
};
#undef SCENERY_STEPS
#define SCENERY_STEPS(at, first, step, second, ...) {__VA_ARGS__},
const uint8_t scenery_steps_first[][7] = {
#include "scenery_steps.inc"
};
#undef SCENERY_STEPS

// An interface kept by its holder: looked up once (the flag set), then read back.
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

volatile long g_scenery_steps;
uint32_t scenery_step(uint8_t *self, uint32_t which, uint32_t value, const SceneryStep &kind) {
    g_scenery_steps++;
    *(volatile uint32_t *)(self + (which ? kind.second : kind.first)) = value;
    uint8_t *to = interface_of(*(uint8_t **)(self + 0x58), 0x130b5c8, (uint8_t * volatile *)(self + 0x68), self + 0x6c);
    if (!to) return 0;
    return ((uint32_t(__thiscall *)(void *, uint32_t, uint32_t))kind.step)(to, which, value);
}

template <int K> uint32_t __thiscall scenery_step_copy(uint8_t *self, uint32_t which, uint32_t value) { return scenery_step(self, which, value, scenery_steps[K]); }
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(scenery_steps[0].at == 0x848840 && scenery_steps[1].at == 0x8489b0 && scenery_steps[2].at == 0x848b20, "scenery_steps.inc changed");
REWRITE("Scenery::SetStep", 0x848840, scenery_steps_first[0], (&scenery_step_copy<0>), &g_scenery_steps);
REWRITE("Scenery::SetStep", 0x8489b0, scenery_steps_first[1], (&scenery_step_copy<1>), &g_scenery_steps);
REWRITE("Scenery::SetStep", 0x848b20, scenery_steps_first[2], (&scenery_step_copy<2>), &g_scenery_steps);
