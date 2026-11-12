// Guests: values worked out from a guest's own numbers.
#include "rewrite.h"

typedef long double x87;

// ---- Guest::Shares(): clears the flag at +0xB0; three ratios a / (b + a) (+0x58 from +0x40 and +0x44, +0x5C from +0x48
// and +0x4C, +0x60 from +0x50 and +0x54); then the eight values at +0x70 as running shares of their total (+0x90 on:
// the running sum times 1 over the total, the constant at 0xF264DC being 1), all in the FPU's precision ----
static volatile long g_guest_shares;
static void __thiscall guest_shares(uint8_t *self) {
    g_guest_shares++;
    float *f = (float *)self;
    self[0xb0] = 0;
    for (int k = 0; k < 3; k++) {
        const float a = f[0x40 / 4 + 2 * k], b = f[0x44 / 4 + 2 * k];
        ((volatile float *)f)[0x58 / 4 + k] = (float)((x87)a / ((x87)b + a));
    }
    x87 total = f[0x70 / 4];
    for (int k = 1; k < 8; k++) total = total + f[0x70 / 4 + k];
    const x87 inverse = (x87) * (const float *)0xf264dc / total;
    x87 running = f[0x70 / 4];
    for (int k = 0; k < 8; k++) {
        if (k) running = running + f[0x70 / 4 + k];
        ((volatile float *)f)[0x90 / 4 + k] = (float)(running * inverse);
    }
}
static const uint8_t guest_shares_original[] = {0xc6, 0x81, 0xb0, 0x00, 0x00, 0x00, 0x00, 0xd9, 0x41, 0x44, 0xd8, 0x41};
REWRITE("Guest::Shares", 0x7713c0, guest_shares_original, &guest_shares, &g_guest_shares);
