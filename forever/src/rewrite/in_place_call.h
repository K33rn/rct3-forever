// Calling a game function whose arguments include an object passed by value that must be made where the callee will
// read it: the game builds such arguments (object references, which keep their own address and are joined into a
// list by it) in the room it has pushed for the call, so a copy made elsewhere and passed by value would not do.
// call_in_place makes room for `bytes` of arguments, has `fill` write them all there (the area's lowest address is
// the first argument), then calls `fn` on `self` (thiscall: the callee takes the arguments off and owns the objects
// given by value). It gives back what the callee leaves in eax.
#pragma once
#include "rewrite.h"

extern "C" uint32_t __cdecl call_in_place(uintptr_t fn, void *self, uint32_t bytes, void(__cdecl *fill)(uint8_t *area, void *context), void *context);

namespace in_place {
// A reference (the reference base class 0xF26AE0) made at `at` to an object: its handle from Handle::For, joined
// first in the handle's list of references, itself as its own owner. 0x14 bytes.
inline void reference_to(uint32_t *at, void *object) {
    at[0] = 0xf26ae0;
    at[4] = (uint32_t)(uintptr_t)at;
    uint8_t *handle = ((uint8_t * (__fastcall *)(void *))0x805700)(object);
    at[1] = (uint32_t)(uintptr_t)handle;
    uint32_t *head = *(uint32_t **)(handle + 0x20);
    at[2] = head[0];
    ((uint32_t *)(uintptr_t)head[0])[1] = (uint32_t)(uintptr_t)&at[2];
    at[3] = (uint32_t)(uintptr_t)head;
    head[0] = (uint32_t)(uintptr_t)&at[2];
}
}   // namespace in_place
