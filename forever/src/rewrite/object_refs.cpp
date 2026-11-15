// Object9AF::Name3, Object9AF::Name2 and Object9AF::Name (one function written out three times; tools/family.py:
// object_refs.inc; they differ in the holder's place, the class number, the reference's table and its assignment):
// make one of an object's holders refer to the object given (thiscall: the object).
//   - The holder's interface (+0x14) looked up once and kept (+0x18; as farther_point.cpp's: its reference at +4,
//     the copy's class number); when it already is the object given, nothing more.
//   - Otherwise the name text (+0xE0) let go and made "NULL" (0x10 new bytes, WideString from C; none without
//     memory); the object refreshed (0x9AE960).
//   - A reference to the object given made on the stack (the reference class 0xF26AE0 with Handle::For's handle,
//     joined first in the handle's list, itself as its owner; then the copy's table, the object kept and its flag
//     set), handed to the holder's assignment (0x9AC830, 0x9AC8B0, 0x510890) and destroyed (ObjectRef's 0x40F730).
//   - +0x154 cleared and the object updated (0x9ACB70, then 0x9AC930).
#include "rewrite.h"

namespace {
struct ObjectRefs {
    uint32_t at, handler, looked, holder, number, table, assign;
};
#define OBJECT_REFS(at, handler, looked, holder, number, table, assign, ...) {at, handler, looked, holder, number, table, assign},
constexpr ObjectRefs object_refs[] = {
#include "object_refs.inc"
};
#undef OBJECT_REFS
#define OBJECT_REFS(at, handler, looked, holder, number, table, assign, ...) {__VA_ARGS__},
const uint8_t object_refs_first[][7] = {
#include "object_refs.inc"
};
#undef OBJECT_REFS
static_assert(object_refs[0].looked == object_refs[0].holder + 0x18 && object_refs[1].looked == object_refs[1].holder + 0x18 &&
                  object_refs[2].looked == object_refs[2].holder + 0x18,
              "the flag is the holder's +0x18");

typedef void(__cdecl *Free)(void *);
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
inline void call(uintptr_t fn, void *self) { ((void(__thiscall *)(void *))fn)(self); }

volatile long g_object_refs;
void object_ref(uint8_t *self, uint8_t *object, const ObjectRefs &kind) {
    g_object_refs++;
    uint8_t *holder = self + kind.holder;
    if (interface_of(*(uint8_t **)(holder + 4), kind.number, (uint8_t * volatile *)(holder + 0x14), holder + 0x18) == object) return;
    int32_t *volatile *name = (int32_t * volatile *)(self + 0xe0);
    --**name;
    int32_t *old = *name;   // read again, as the game does
    if (*(volatile int32_t *)old == 0 && old) {
        ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)old[3]);
        ((Free)0xbce000)(old);
    }
    void *memory = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x10);
    *name = memory ? ((int32_t * (__thiscall *)(void *, const char *))0xbcf850)(memory, (const char *)0xf82bfc) : nullptr;   // the game's "NULL"
    call(0x9ae960, self);
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
    reference[0] = kind.table;
    reference[5] = (uint32_t)(uintptr_t)object;
    *(volatile uint8_t *)&reference[6] = 1;
    ((void(__thiscall *)(void *, volatile void *))kind.assign)(holder, reference);
    call(0x40f730, (void *)reference);
    self[0x154] = 0;
    call(0x9acb70, self);
    call(0x9ac930, self);
}

template <int K> void __thiscall object_ref_copy(uint8_t *self, uint8_t *object) { object_ref(self, object, object_refs[K]); }
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(object_refs[0].at == 0x9aeed0 && object_refs[1].at == 0x9af040 && object_refs[2].at == 0x9af1b0, "object_refs.inc changed");
REWRITE("Object9AF::Refer", 0x9aeed0, object_refs_first[0], (&object_ref_copy<0>), &g_object_refs);
REWRITE("Object9AF::Refer", 0x9af040, object_refs_first[1], (&object_ref_copy<1>), &g_object_refs);
REWRITE("Object9AF::Refer", 0x9af1b0, object_refs_first[2], (&object_ref_copy<2>), &g_object_refs);
