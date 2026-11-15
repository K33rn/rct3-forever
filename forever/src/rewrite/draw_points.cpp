// Text::FillBF7770, Text::FillBF7F80 and Text::FillBF8540 (one function written out three times; they differ only in
// the kind of primitive drawn: 2, 5 and 4, a line list, a triangle strip and a triangle list): draw coloured
// points with a device (fastcall: the device, the count; on the stack the points, 16 bytes each, and the colour).
//   - The device given an identity matrix (its slot 0xB8, with 0) and the game's two current settings (slot 0xAC,
//     [0x13D28A0] and [0x13D0E34]).
//   - A vertex buffer made for the count (0xBEC340, format 0x201, 3) and locked (its slot 8: 0, the count, 0); each
//     point's x, y and z copied in (through the FPU, as the game does: a signalling NaN comes out quiet), then the
//     colour; unlocked (slot 0xC).
//   - The primitive made from it (0xBE8FE0: the kind, the buffer, then 0, 2.0f and six 0s); the buffer let go (its
//     16-bit count at +4 taken down, at 0 its slot 0 with 1), the primitive drawn (the device's slot 0x44) and let
//     go (at 0 its slot 4).
#include "rewrite.h"

namespace {
inline void through_fpu(float *to, const float *from) { __asm__ volatile("flds %1\n\tfstps %0" : "=m"(*to) : "m"(*from)); }
template <class R = void, class... A> inline R slot(void *object, uint32_t at, A... a) {
    return ((R(__thiscall *)(void *, A...))(*(void ***)object)[at / 4])(object, a...);
}

volatile long g_draw_points;
template <uint32_t KIND> void __fastcall draw_points(uint8_t *device, int32_t count, const float *points, uint32_t colour) {
    g_draw_points++;
    volatile float identity[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    slot(device, 0xb8, 0u, (void *)identity);
    slot(device, 0xac, *(volatile uint32_t *)(uintptr_t)0x13d28a0, *(volatile uint32_t *)(uintptr_t)0x13d0e34);
    uint8_t *buffer = ((uint8_t * (__fastcall *)(int32_t, uint32_t, uint32_t))0xbec340)(count, 0x201, 3);
    float *to = slot<float *>(buffer, 8, 0u, count, 0u);
    for (int32_t i = 0; i < count; i++, to += 4, points += 4) {
        through_fpu(&to[0], &points[0]);
        through_fpu(&to[1], &points[1]);
        through_fpu(&to[2], &points[2]);
        *(volatile uint32_t *)&to[3] = colour;
    }
    slot(buffer, 0xc);
    typedef uint8_t *(__fastcall * MakePrimitive)(uint32_t, void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    uint8_t *primitive = ((MakePrimitive)0xbe8fe0)(KIND, buffer, 0, 0x40000000, 0, 0, 0, 0, 0, 0);
    if (--*(volatile uint16_t *)(buffer + 4) == 0) slot(buffer, 0, 1u);
    slot(device, 0x44, (void *)primitive);
    if (--*(volatile uint16_t *)(primitive + 4) == 0) slot(primitive, 4);
}

const uint8_t draw_points_first[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83};
}   // namespace
REWRITE("Device::DrawPoints", 0xbf7770, draw_points_first, (&draw_points<2>), &g_draw_points);
REWRITE("Device::DrawPoints", 0xbf7f80, draw_points_first, (&draw_points<5>), &g_draw_points);
REWRITE("Device::DrawPoints", 0xbf8540, draw_points_first, (&draw_points<4>), &g_draw_points);
