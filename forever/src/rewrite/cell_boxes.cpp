// Person::Terrain7BB530, Animal::TerrainB5F5E0 and Animal::TerrainB758E0 (one function written out three times;
// tools/family.py: cell_boxes.inc; they differ in the object they tell and how): tell an object about one terrain
// cell (thiscall: the cell's x and y, and the one asking: an object with a change that may be waiting).
//   - The park's terrain (Terrain::Get) made sure of (ManagedObject::Activate unless its flags at +0x2C have bit 0).
//   - Nothing more for a cell outside its grid (+0x54: the counts are bytes at +4 and +5; signed compares).
//   - The cell's box {x * the cell width (+0x14) + the grid's x (+0xC), -32768, y * the cell depth (+0x18) + the
//     grid's z (+0x10)} to {the same for x + 1, 32767, y + 1}, each corner a float, given with the value to the
//     copy's function (0x7B9D30, 0xB5F370, 0xB75670) on the object at +0x4C, +0x24 or +0x64, with the one asking.
//   - When the one asking has a change waiting (+0x104): its slot 0 with its +4 and the change, and the change
//     cleared.
#include "rewrite.h"

namespace {
struct CellBox {
    uint32_t at, object, tell;
};
#define CELL_BOXES(at, object, tell, ...) {at, object, tell},
constexpr CellBox cell_boxes[] = {
#include "cell_boxes.inc"
};
#undef CELL_BOXES
#define CELL_BOXES(at, object, tell, ...) {__VA_ARGS__},
const uint8_t cell_boxes_first[][7] = {
#include "cell_boxes.inc"
};
#undef CELL_BOXES
typedef long double ext;
inline float rf(ext x) {   // stored to memory as a float
    volatile float f = (float)x;
    return f;
}

volatile long g_cell_boxes;
void cell_box(uint8_t *self, int32_t x, int32_t y, uint8_t *asking, const CellBox &kind) {
    g_cell_boxes++;
    uint8_t *terrain = ((uint8_t * (__cdecl *)())0x40a090)();
    if (!(terrain[0x2c] & 1)) ((void(__thiscall *)(void *))0x800860)(terrain);
    const uint8_t *grid = *(uint8_t **)(terrain + 0x54);
    if (x < 0 || x >= (int32_t)grid[4] || y < 0 || y >= (int32_t)grid[5]) return;
    const float width = *(const float *)(grid + 0x14), depth = *(const float *)(grid + 0x18), left = *(const float *)(grid + 0xc),
                front = *(const float *)(grid + 0x10);
    volatile float low[3], high[3];
    low[0] = rf((ext)x * width + left);
    low[1] = -32768.0f;
    low[2] = rf((ext)y * depth + front);
    high[0] = rf((ext)(x + 1) * width + left);
    high[1] = 32767.0f;
    high[2] = rf((ext)(y + 1) * depth + front);
    ((void(__thiscall *)(void *, volatile float *, volatile float *, void *))kind.tell)(*(void **)(self + kind.object), low, high, asking);
    if (uint32_t change = *(volatile uint32_t *)(asking + 0x104)) {
        ((void(__thiscall *)(void *, void *, uint32_t))(*(void ***)asking)[0])(asking, asking + 4, change);
        *(volatile uint32_t *)(asking + 0x104) = 0;
    }
}

template <int K> void __thiscall cell_box_copy(uint8_t *self, int32_t x, int32_t y, uint8_t *asking) { cell_box(self, x, y, asking, cell_boxes[K]); }
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(cell_boxes[0].at == 0x7bb530 && cell_boxes[1].at == 0xb5f5e0 && cell_boxes[2].at == 0xb758e0, "cell_boxes.inc changed");
REWRITE("Terrain::TellCell", 0x7bb530, cell_boxes_first[0], (&cell_box_copy<0>), &g_cell_boxes);
REWRITE("Terrain::TellCell", 0xb5f5e0, cell_boxes_first[1], (&cell_box_copy<1>), &g_cell_boxes);
REWRITE("Terrain::TellCell", 0xb758e0, cell_boxes_first[2], (&cell_box_copy<2>), &g_cell_boxes);
