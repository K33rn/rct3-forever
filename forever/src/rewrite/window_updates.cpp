// Window::Update9BE750 and Window::Update9BE910 (one function written out twice; tools/family.py: window_updates.inc;
// they differ in where the window keeps the holder, the interface's class and the four calls): point a window's
// holder at another object (thiscall; the object; no value).
//   - The holder (+0x20 or +4, its flag at holder + 0x18 = +0x38 / +0x1C): its interface looked up once and kept (as
//     ride_checks.cpp's: its reference at +4, the class number in the copy's global); when there is one, told it is
//     let go (the copy's first call, on the interface).
//   - A reference to the object made on the stack (class 0xF26AE0 with Handle::For's handle, joined first in its
//     list, itself its owner; then the copy's class, the object kept and its flag set), given to the holder (the
//     copy's assignment) and destroyed (ObjectRef's 0x40F730).
//   - The interface looked up again; when there is one, told it is taken (the copy's last call).
#include "rewrite.h"

namespace {
struct WindowUpdate {
    uint32_t at, flag, holder, class_global, let_go, reference_class, assign, taken;
};
#define WINDOW_UPDATES(at, flag, holder, cls, let_go, ref, assign, taken, ...) {at, flag, holder, cls, let_go, ref, assign, taken},
constexpr WindowUpdate window_updates[] = {
#include "window_updates.inc"
};
#undef WINDOW_UPDATES
#define WINDOW_UPDATES(at, flag, holder, cls, let_go, ref, assign, taken, ...) {__VA_ARGS__},
const uint8_t window_updates_first[][7] = {
#include "window_updates.inc"
};
#undef WINDOW_UPDATES
static_assert(window_updates[0].flag == window_updates[0].holder + 0x18 && window_updates[1].flag == window_updates[1].holder + 0x18,
              "the flag is the holder's +0x18");

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

typedef void(__thiscall *Tell)(void *);
volatile long g_window_updates;
void window_update(uint8_t *self, void *object, const WindowUpdate &kind) {
    g_window_updates++;
    uint8_t *holder = self + kind.holder;
    if (interface_of(holder, kind.class_global)) ((Tell)kind.let_go)(interface_of(holder, kind.class_global));
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
    reference[0] = kind.reference_class;
    reference[5] = (uint32_t)(uintptr_t)object;
    *(volatile uint8_t *)&reference[6] = 1;
    ((void(__thiscall *)(void *, volatile void *))kind.assign)(holder, reference);
    ((void(__thiscall *)(volatile void *))0x40f730)(reference);
    if (interface_of(holder, kind.class_global)) ((Tell)kind.taken)(interface_of(holder, kind.class_global));
}

template <int K> void __fastcall window_update_copy(uint8_t *self, void *, void *object) { window_update(self, object, window_updates[K]); }
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(window_updates[0].at == 0x9be750 && window_updates[1].at == 0x9be910, "window_updates.inc changed");
REWRITE("Window::PointAt", 0x9be750, window_updates_first[0], (&window_update_copy<0>), &g_window_updates);
REWRITE("Window::PointAt", 0x9be910, window_updates_first[1], (&window_update_copy<1>), &g_window_updates);
