// Ride::Update5365B0 and Ride::Update536880 (one function written out twice; they differ in where the ride keeps the
// holder, +0x1CC or +0x1E8, and its byte flag, +0x2B4 or +0x2B5): point a ride's holder at an object (thiscall; the
// object; no value).
//   - The holder's interface looked up once and kept (as ride_checks.cpp's: its reference at +4, the class number
//     [0x1310F90]); when it is the object already, nothing more.
//   - With an interface and the flag set: the interface told (0x880F60), the flag cleared.
//   - A reference to the object made on the stack (class 0xF26AE0 with Handle::For's handle, joined first in its list,
//     itself its owner; then class 0xF3815C, the object kept, its flag set), given to the holder (0x532A80) and
//     destroyed (ObjectRef's 0x40F730).
//   - With an interface (looked up again) and the flag clear: the interface told (0x8826F0 with 1), the flag set.
//   - For each number below the count (+0x44) of the interface of the ride's second holder (+0x204, class number
//     [0x12F5338]): when the ride has a part for it (0x532930), the part (asked for again) given the ride's value
//     (0x4AA3E0) by 0x737210.
//   - The ride's slot 0x144.
#include "in_place_call.h"

namespace {
uint8_t *interface_of(uint8_t *holder, uint32_t class_global) {   // looked up once and kept (holder +0x14, its flag +0x18)
    if (!*(volatile uint8_t *)(holder + 0x18)) {
        uint8_t *object = *(uint8_t **)(*(uint8_t **)(holder + 4) + 0xc), *found = nullptr;
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
        *(uint8_t *volatile *)(holder + 0x14) = found;
        *(volatile uint8_t *)(holder + 0x18) = 1;
    }
    return *(uint8_t *volatile *)(holder + 0x14);
}

volatile long g_ride_updates;
template <uint32_t HOLDER, uint32_t FLAG> void __fastcall ride_update(uint8_t *self, void *, void *object) {
    g_ride_updates++;
    uint8_t *holder = self + HOLDER;
    if (interface_of(holder, 0x1310f90) == object) return;
    if (interface_of(holder, 0x1310f90) && self[FLAG]) {
        ((void(__thiscall *)(void *))0x880f60)(interface_of(holder, 0x1310f90));
        self[FLAG] = 0;
    }
    volatile uint32_t reference[7];
    in_place::reference_to((uint32_t *)reference, object);
    reference[0] = 0xf3815c;
    reference[5] = (uint32_t)(uintptr_t)object;
    *(volatile uint8_t *)&reference[6] = 1;
    ((void(__thiscall *)(void *, volatile void *))0x532a80)(holder, reference);
    ((void(__thiscall *)(volatile void *))0x40f730)(reference);
    if (interface_of(holder, 0x1310f90) && !self[FLAG]) {
        ((void(__thiscall *)(void *, uint32_t))0x8826f0)(interface_of(holder, 0x1310f90), 1);
        self[FLAG] = 1;
    }
    for (int32_t i = 0;; i++) {
        uint8_t *second = interface_of(self + 0x204, 0x12f5338);
        if (i >= *(int32_t *)(second + 0x44)) break;
        if (((void *(__thiscall *)(void *, int32_t))0x532930)(self, i)) {
            const uint32_t value = ((uint32_t(__thiscall *)(void *))0x4aa3e0)(self);
            void *part = ((void *(__thiscall *)(void *, int32_t))0x532930)(self, i);
            ((void(__thiscall *)(void *, uint32_t))0x737210)(part, value);
        }
    }
    ((void(__thiscall *)(void *))(*(void ***)self)[0x144 / 4])(self);
}

const uint8_t ride_update_first[] = {0x6a, 0xff, 0x64, 0xa1, 0x00, 0x00, 0x00};
}   // namespace
REWRITE("Ride::PointHolderAt", 0x5365b0, ride_update_first, (&ride_update<0x1cc, 0x2b4>), &g_ride_updates);
REWRITE("Ride::PointHolderAt", 0x536880, ride_update_first, (&ride_update<0x1e8, 0x2b5>), &g_ride_updates);
