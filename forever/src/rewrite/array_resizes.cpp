// "Resize an array of records": the array is {items, count}. When n differs from the count: n records are allocated
// and constructed (ArrayConstruct with the record's constructor; none for n = 0), the first min(n, count) old records
// are copied a field at a time (only the fields the record's copy takes), the old ones are freed, and the count and
// the new items are kept. Gives n. The functions differ only in the record's size, its constructor and the fields
// copied: read from the game by tools/array_resizes.py into array_resizes_list.inc. (The game's exception frame
// around the construct call isn't rebuilt, as in the other container rewrites.)
#include "rewrite.h"

struct ResizeField {
    uint16_t offset;
    uint8_t bytes;   // 1, 2 or 4
};
static volatile long g_array_resizes;
template <uint32_t SIZE, uint32_t CTOR, const ResizeField *FIELDS, uint32_t N>
static int32_t __thiscall array_resize(uint32_t *self, int32_t n) {
    g_array_resizes++;
    if (n == (int32_t)self[1]) return n;
    uint8_t *fresh = nullptr;
    if (n != 0) {
        fresh = ((uint8_t * (__cdecl *)(uint32_t))0xbcdfd0)((uint32_t)n * SIZE);
        if (fresh) ((void(__stdcall *)(void *, uint32_t, int32_t, void *))0x402b90)(fresh, SIZE, n, (void *)CTOR);
        const int32_t keep = n > (int32_t)self[1] ? (int32_t)self[1] : n;
        for (int32_t i = 0; i < keep; i++) {
            const uint8_t *from = (const uint8_t *)(uintptr_t)self[0] + (uint32_t)i * SIZE;
            uint8_t *to = fresh + (uint32_t)i * SIZE;
            for (uint32_t f = 0; f < N; f++) {
                const uint32_t o = FIELDS[f].offset;
                if (FIELDS[f].bytes == 4) *(volatile uint32_t *)(to + o) = *(const volatile uint32_t *)(from + o);
                else if (FIELDS[f].bytes == 2) *(volatile uint16_t *)(to + o) = *(const volatile uint16_t *)(from + o);
                else *(volatile uint8_t *)(to + o) = *(const volatile uint8_t *)(from + o);
            }
        }
    }
    if (self[0]) {
        ((void(__cdecl *)(void *))0xbce010)((void *)(uintptr_t)self[0]);
        self[0] = 0;
    }
    self[1] = (uint32_t)n;
    self[0] = (uint32_t)(uintptr_t)fresh;
    return n;
}
#include "array_resizes_list.inc"
// The same function where the compiler laid the copy loops out another way (tools/array_resizes.py doesn't follow
// their cursors): rows written here, the fields read from the loop that copies four records at a time.
namespace hand_rows {   // (REWRITE names its entries by line: these would meet the included list's)
static const uint8_t array_resize_hand_original[] = {0x64, 0xa1, 0x00, 0x00, 0x00, 0x00};
static const ResizeField array_resize_94e4b0_fields[] = {{0x0, 4}, {0x4, 4}, {0x8, 4}, {0x10, 4}, {0x14, 4}, {0x18, 4}, {0x1c, 4}, {0x20, 4}};
REWRITE("Array::ResizeRecords", 0x94e4b0, array_resize_hand_original, (&array_resize<0x30, 0x94da50, array_resize_94e4b0_fields, 8>), &g_array_resizes);
static const ResizeField array_resize_a6fd20_fields[] = {{0x0, 4}, {0x4, 4}, {0x8, 4}, {0x10, 4}, {0x14, 4}};
REWRITE("Array::ResizeRecords", 0xa6fd20, array_resize_hand_original, (&array_resize<0x20, 0xa6bc00, array_resize_a6fd20_fields, 5>), &g_array_resizes);
static const ResizeField array_resize_afd3b0_fields[] = {{0x0, 1}, {0x10, 4}, {0x14, 4}, {0x18, 4}, {0x20, 4}, {0x24, 4}, {0x28, 4}};
REWRITE("Array::ResizeRecords", 0xafd3b0, array_resize_hand_original, (&array_resize<0x30, 0xaf9f20, array_resize_afd3b0_fields, 7>), &g_array_resizes);
}   // namespace hand_rows
