// TrackedRide::Check5C6E30 and TrackedRide::Check5C7000 (one function written out twice; tools/family.py:
// ride_checks.inc; they differ in where the ride keeps the holder and a value given to the maker): the ride's
// interface, made when there is none (thiscall; gives it).
//   - The holder (+0x2BC or +0x2D8, its flag +0x2D4 / +0x2F0 = holder + 0x18): its interface looked up once and kept
//     (as farther_point.cpp's: its reference at +4, class number [0x12F5608]); when there is one, it is the answer.
//   - Otherwise: a maker built on the stack (0x5BDE60 with 0 or 1) and asked for an object for the ride (0x5BDEC0);
//     a reference to that object made on the stack (class 0xF26AE0 with Handle::For's handle, joined first in its
//     list, itself its owner; then class 0xF35618, the object kept and its flag set), given to the holder (0x4E6410)
//     and destroyed (ObjectRef's 0x40F730); the interface looked up again (once more only when the holder's flag is
//     clear); the maker destroyed (0x4882D0); that interface the answer.
#include "rewrite.h"

namespace {
struct RideCheck {
    uint32_t at, flag, holder, maker_value;
};
#define RIDE_CHECKS(at, flag, holder, value, ...) {at, flag, holder, value},
constexpr RideCheck ride_checks[] = {
#include "ride_checks.inc"
};
#undef RIDE_CHECKS
#define RIDE_CHECKS(at, flag, holder, value, ...) {__VA_ARGS__},
const uint8_t ride_checks_first[][7] = {
#include "ride_checks.inc"
};
#undef RIDE_CHECKS
static_assert(ride_checks[0].flag == ride_checks[0].holder + 0x18 && ride_checks[1].flag == ride_checks[1].holder + 0x18, "the flag is the holder's +0x18");

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

volatile long g_ride_checks;
uint8_t *ride_check(uint8_t *self, const RideCheck &kind) {
    g_ride_checks++;
    uint8_t *holder = self + kind.holder;
    auto lookup = [&] { return interface_of(*(uint8_t **)(holder + 4), 0x12f5608, (uint8_t * volatile *)(holder + 0x14), holder + 0x18); };
    if (lookup()) return *(uint8_t *volatile *)(holder + 0x14);
    alignas(4) uint8_t maker[0x40];
    ((void(__thiscall *)(void *, uint32_t))0x5bde60)(maker, kind.maker_value);
    void *object = ((void *(__thiscall *)(void *, void *))0x5bdec0)(maker, self);
    volatile uint32_t reference[7];
    reference[4] = (uint32_t)(uintptr_t)reference;
    reference[0] = 0xf26ae0;
    uint8_t *handle = ((uint8_t * (__fastcall *)(void *))0x805700)(object);
    reference[1] = (uint32_t)(uintptr_t)handle;
    uint32_t *head = *(uint32_t **)(handle + 0x20);
    reference[2] = head[0];
    ((uint32_t *)(uintptr_t)head[0])[1] = (uint32_t)(uintptr_t)&reference[2];
    reference[3] = (uint32_t)(uintptr_t)head;
    head[0] = (uint32_t)(uintptr_t)&reference[2];
    reference[0] = 0xf35618;
    reference[5] = (uint32_t)(uintptr_t)object;
    *(volatile uint8_t *)&reference[6] = 1;
    ((void(__thiscall *)(void *, volatile void *))0x4e6410)(holder, reference);
    ((void(__thiscall *)(volatile void *))0x40f730)(reference);
    uint8_t *found = lookup();
    ((void(__thiscall *)(void *))0x4882d0)(maker);
    return found;
}

template <int K> uint8_t *__fastcall ride_check_copy(uint8_t *self) { return ride_check(self, ride_checks[K]); }
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(ride_checks[0].at == 0x5c6e30 && ride_checks[1].at == 0x5c7000, "ride_checks.inc changed");
REWRITE("TrackedRide::Interface", 0x5c6e30, ride_checks_first[0], (&ride_check_copy<0>), &g_ride_checks);
REWRITE("TrackedRide::Interface", 0x5c7000, ride_checks_first[1], (&ride_check_copy<1>), &g_ride_checks);
