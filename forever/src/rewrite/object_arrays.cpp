// Array::Grow40C6A0, Grow4D01A0, GrowCE78E0 and GrowCEBE40 (one function written out four times; tools/family.py:
// object_arrays.inc), and Grow735850, Grow87BEE0 and Grow8E9330 (the same, the compiler's order a little different:
// rows written here), and List::Grow9D0F40 (the same with its objects' copy inlined: the fields it takes); they differ in the object's size, the block's header and the object's constructor, destructor
// and copy): grow an array of objects with a map of free places to n (thiscall: n). The array is {objects, map,
// used, count}; the objects' block keeps its count in a header just before them (0x10 or 4 bytes).
//   - Nothing when n is not above the count.
//   - A new block (aligned, n objects and the header; its count set and the objects built by the runtime's vector
//     constructor iterator with the copy's constructor and destructor; none without memory) and a new map (n
//     places).
//   - The old objects copied into the new ones (the copy's copy, new from old), the map's first count - used
//     places copied, and every new place's number (from the old count up to n) put after them.
//   - The old objects destroyed by the runtime's vector destructor iterator (their count from their header) and
//     their block freed; the old map freed; the count made n.
#include "rewrite.h"

namespace {
struct ObjectField {   // a field an object's copy takes (when it has no copy of its own): where, 1 or 4 bytes
    uint16_t at, width;
};
struct ObjectArray {
    uint32_t at, size, header, dtor, ctor, copy, back;
    const ObjectField *fields;
    uint32_t field_count;
};
#define OBJECT_ARRAYS(at, size, header, dtor, ctor, copy, back, ...) {at, size, header, dtor, ctor, copy, back},
constexpr ObjectArray object_arrays[] = {
#include "object_arrays.inc"
};
#undef OBJECT_ARRAYS
#define OBJECT_ARRAYS(at, size, header, dtor, ctor, copy, back, ...) {__VA_ARGS__},
const uint8_t object_arrays_first[][7] = {
#include "object_arrays.inc"
};
#undef OBJECT_ARRAYS
static_assert(object_arrays[0].back == 0u - object_arrays[0].header && object_arrays[1].back == 0u - object_arrays[1].header, "the count is in the header");

typedef void *(__cdecl *Alloc)(uint32_t);
typedef void(__cdecl *Free)(void *);
const Alloc alloc_aligned = (Alloc)0xbcdfd0;
const Free free_aligned = (Free)0xbce010;

volatile long g_object_arrays;
void grow(uint32_t *self, int32_t n, const ObjectArray &kind) {
    g_object_arrays++;
    if (n <= (int32_t)self[3]) return;
    uint8_t *old_objects = (uint8_t *)(uintptr_t)self[0];
    int32_t *old_map = (int32_t *)(uintptr_t)self[1];
    uint8_t *block = (uint8_t *)alloc_aligned((uint32_t)n * kind.size + kind.header);
    uint8_t *objects = nullptr;
    if (block) {
        *(int32_t *)block = n;
        objects = block + kind.header;
        ((void(__stdcall *)(void *, uint32_t, int32_t, uintptr_t, uintptr_t))0xddcf99)(objects, kind.size, n, kind.ctor, kind.dtor);
    }
    self[0] = (uint32_t)(uintptr_t)objects;
    self[1] = (uint32_t)(uintptr_t)alloc_aligned((uint32_t)n * 4);
    for (int32_t i = 0; i < (int32_t)self[3]; i++) {
        uint8_t *to = (uint8_t *)(uintptr_t)self[0] + (uint32_t)i * kind.size, *from = old_objects + (uint32_t)i * kind.size;
        if (kind.copy) {
            ((void(__thiscall *)(void *, void *))kind.copy)(to, from);
        } else {
            for (uint32_t f = 0; f < kind.field_count; f++) {
                if (kind.fields[f].width == 1) to[kind.fields[f].at] = from[kind.fields[f].at];
                else *(uint32_t *)(to + kind.fields[f].at) = *(uint32_t *)(from + kind.fields[f].at);
            }
        }
    }
    const int32_t kept = (int32_t)self[3] - (int32_t)self[2];
    for (int32_t i = 0; i < kept; i++) ((int32_t *)(uintptr_t)self[1])[i] = old_map[i];
    for (int32_t number = (int32_t)self[3]; number < n; number++) ((int32_t *)(uintptr_t)self[1])[kept - (int32_t)self[3] + number] = number;
    if (old_objects) {
        ((void(__stdcall *)(void *, uint32_t, int32_t, uintptr_t))0xddcf39)(old_objects, kind.size, *(int32_t *)(old_objects - kind.header), kind.dtor);
        free_aligned(old_objects - kind.header);
    }
    if (old_map) free_aligned(old_map);
    self[3] = (uint32_t)n;
}

template <int K> void __thiscall grow_copy(uint32_t *self, int32_t n) { grow(self, n, object_arrays[K]); }
constexpr ObjectField fields_9d0f40[] = {{0, 1}, {1, 1}, {4, 4}, {8, 4}, {0x10, 4}, {0x14, 4}, {0x18, 4}, {0x20, 4}, {0x24, 4}, {0x28, 4}, {0x2c, 4},
                                        {0x30, 4}, {0x34, 4}, {0x38, 4}};
constexpr ObjectArray more_arrays[] = {{0x735850, 0x98, 4, 0x7340b0, 0x734250, 0x734160, 0xfffffffc},
                                       {0x87bee0, 0x80, 0x10, 0x87b5e0, 0x87b570, 0x87b700, 0xfffffff0},
                                       {0x8e9330, 0x130, 0x10, 0x8e71c0, 0x8e91d0, 0x8e7e60, 0xfffffff0},
                                       {0x9d0f40, 0x40, 0x10, 0x9cdb00, 0x9cdae0, 0, 0xfffffff0, fields_9d0f40, 14}};   // (its copy inlined: the fields)
const uint8_t more_arrays_first[] = {0x64, 0xa1, 0x00, 0x00, 0x00, 0x00, 0x6a};
template <int K> void __thiscall grow_more(uint32_t *self, int32_t n) { grow(self, n, more_arrays[K]); }
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(object_arrays[0].at == 0x40c6a0 && object_arrays[1].at == 0x4d01a0 && object_arrays[2].at == 0xce78e0 && object_arrays[3].at == 0xcebe40, "object_arrays.inc changed");
REWRITE("ObjectArray::Grow", 0x40c6a0, object_arrays_first[0], (&grow_copy<0>), &g_object_arrays);
REWRITE("ObjectArray::Grow", 0x4d01a0, object_arrays_first[1], (&grow_copy<1>), &g_object_arrays);
REWRITE("ObjectArray::Grow", 0xce78e0, object_arrays_first[2], (&grow_copy<2>), &g_object_arrays);
REWRITE("ObjectArray::Grow", 0xcebe40, object_arrays_first[3], (&grow_copy<3>), &g_object_arrays);
REWRITE("ObjectArray::Grow", 0x735850, more_arrays_first, (&grow_more<0>), &g_object_arrays);
REWRITE("ObjectArray::Grow", 0x87bee0, more_arrays_first, (&grow_more<1>), &g_object_arrays);
REWRITE("ObjectArray::Grow", 0x8e9330, more_arrays_first, (&grow_more<2>), &g_object_arrays);
REWRITE("ObjectArray::Grow", 0x9d0f40, more_arrays_first, (&grow_more<3>), &g_object_arrays);
