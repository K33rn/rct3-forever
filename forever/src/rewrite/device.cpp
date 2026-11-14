// The graphics device's constructor (GE_Device, 0xC77890): the base built, the tables set, the members started and
// cleared, four matrices set to the identity, the render defaults, the debug output names, then the 129 vertex shader
// fragments the engine builds its vertex shaders from, each assembled from its source text and kept in its slot, and
// last the lighting defaults, the device's state record and the pass shaders. The game has the fragment code written
// out 129 times (most of the function's 78 KB); here it is one function and the list tools/vs_fragments.py reads from
// the game (slots and the addresses of the texts, which stay in the game).
#include "rewrite.h"

typedef void *(__cdecl *Alloc)(uint32_t);

struct DeviceFragment {
    uint32_t slot, text;
};
static const DeviceFragment device_fragments[] = {
#include "device_fragments.inc"
};

// One fragment: a record {table, text, count, code}. The text is assembled (D3DX's assembler, 0xCF6275); the code it
// gives is kept without its first two values (count = (bytes + 3) / 4 - 2 values, copied from the buffer's + 4), the
// buffer let go. A failed result is reported against GE_Fragment.h, line 0x69. No memory: no fragment.
static uint32_t *device_fragment(const char *text) {
    uint32_t *f = (uint32_t *)((Alloc)0xbcdfc0)(0x10);
    if (!f) return nullptr;
    f[0] = 0xfba0bc;
    f[1] = (uint32_t)(uintptr_t)text;
    f[2] = 0;
    f[3] = 0;
    if (text) {
        void **code = nullptr, **errors = nullptr;   // (the game never asks for the errors: that one stays null)
        const char *end = (const char *)(uintptr_t)f[1];
        while (*end) end++;
        const uint32_t length = (uint32_t)(end - (const char *)(uintptr_t)f[1]);
        const int32_t result = ((int32_t(__stdcall *)(const char *, uint32_t, void *, void *, uint32_t, void ***, void *))0xcf6275)(
            (const char *)(uintptr_t)f[1], length, nullptr, nullptr, 2, &code, nullptr);
        if (code) {
            const uint32_t bytes = ((uint32_t(__stdcall *)(void *))(*(void ***)code)[4])(code);
            f[2] = ((bytes + 3) >> 2) - 2;
            f[3] = (uint32_t)(uintptr_t)((Alloc)0xbcdfd0)(f[2] << 2);
            const uint32_t *from = (const uint32_t *)((uint8_t *(__stdcall *)(void *))(*(void ***)code)[3])(code) + 1;
            for (int32_t i = 0; i < (int32_t)f[2]; i++) ((volatile uint32_t *)(uintptr_t)f[3])[i] = from[i];
            ((uint32_t(__stdcall *)(void *))(*(void ***)code)[2])(code);
        }
        if (errors) ((uint32_t(__stdcall *)(void *))(*(void ***)errors)[2])(errors);
        if (result < 0) {
            const char *what = ((const char *(__stdcall *)(int32_t))0xdd696e)(result);
            ((void(__cdecl *)(const char *, const char *, int32_t, const char *))0xc34850)((const char *)0xfba084, (const char *)0xfba090, 0x69, what);
        }
    }
    return f;
}

static inline void zero_values(uint8_t *at, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) ((volatile uint32_t *)at)[i] = 0;
}
static inline void set4(uint8_t *at, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    volatile uint32_t *v = (volatile uint32_t *)at;
    v[0] = a, v[1] = b, v[2] = c, v[3] = d;
}

static volatile long g_device_constructs;
static uint8_t *__thiscall device_construct(uint8_t *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    g_device_constructs++;
    typedef void *(__thiscall *Self)(void *);
    typedef void(__stdcall *ArrayConstruct)(void *, uint32_t, int32_t, void *);
    auto value = [&](uint32_t offset) -> volatile uint32_t & { return *(volatile uint32_t *)(self + offset); };

    ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))0xc07350)(self, a, b, c, d);   // the base
    value(0) = 0xfbed00;
    value(0x210) = 0xfbecf0;
    ((Self)0x445210)(self + 0x688);
    ((Self)0x445210)(self + 0x688 + 0x44);
    ((ArrayConstruct)0x402b90)(self + 0x6d0 + 0xd0, 0x40, 0x40, (void *)0x4077c0);
    ((ArrayConstruct)0x402b90)(self + 0x17a0 + 0x10, 0x90, 4, (void *)0xc6f3c0);
    value(0x19f0) = 0;
    value(0x19f4) = 0;
    value(0x19f8) = 0;
    ((Self)0x445210)(self + 0x1a00 + 8);
    value(0x1a40) = 0;
    value(0x1a44) = 0;
    ((Self)0xc77340)(self + 0x1a48);
    ((Self)0xc77360)(self + 0x1a60);
    ((ArrayConstruct)0x402b90)(self + 0x1a70, 0x10, 0x61, (void *)0x47d6f0);
    ((void *(__thiscall *)(void *, uint32_t, uint32_t))0xc77390)(self + 0x2480, 0xcb1040, 0);
    value(0x49c) = 4;
    // 10, 10, 0.1, 0.1 (as the bits the game passes)
    ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))0xc09090)((void *)(uintptr_t)(uint32_t)value(0x478), 0x41200000, 0x41200000, 0x3dcccccd, 0x3dcccccd);

    zero_values(self + 0x6d0, 0x434);
    zero_values(self + 0x17a0, 0x94);
    zero_values(self + 0x1a00, 0x10);
    zero_values(self + 0x1a70, 0x184);
    set4(self + 0x20d8, 0, 0, 0, 0);
    value(0x20e8) = 0;
    value(0x20ec) = 0;
    value(0x20f0) = 0;
    value(0x20d4) = 0;
    value(0x20d0) = 0;
    static const uint32_t matrices[4] = {0x2090, 0x6e0, 0x720, 0x760};   // each set to the identity matrix the game keeps at 0x10F2FA0
    for (uint32_t to : matrices)
        for (int i = 0; i < 16; i++) value(to + 4 * i) = ((volatile uint32_t *)0x10f2fa0)[i];
    value(0x6d4) = 1;
    zero_values(self + 0x688, 0x12);

    // the render defaults; three of them go through small helpers and values of their own on the stack, as in the game
    uint32_t zero, kept, tagged[2], one, colour, kept_colour;
    ((void *(__thiscall *)(void *, uint32_t))0x41aac0)(&zero, 0);
    ((void *(__thiscall *)(void *, void *, void *))0x402cb0)(self + 0x688, &kept, &zero);
    value(0x68c) = 0x3f800000;
    value(0x690) = 3;
    one = 0x3f800000;
    void *record = ((void *(__thiscall *)(void *, void *))0xbdd440)(tagged, &one);
    value(0x694) = *((uint32_t * (__thiscall *)(void *))0xc73840)(record);
    value(0x698) = 0;
    value(0x69c) = 0xf;
    value(0x6a0) = 1;
    value(0x6a4) = 0;
    value(0x6a8) = 1;
    value(0x6ac) = 1;
    value(0x6b0) = 1;
    value(0x6b4) = 8;
    value(0x6b8) = 0;
    value(0x6bc) = 0xffffffff;
    value(0x6c0) = 0xffffffff;
    value(0x6c4) = 0;
    value(0x6c8) = 0;
    void *white = ((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))0x402c40)(&colour, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000);
    ((void *(__thiscall *)(void *, void *, void *))0x402cb0)(self + 0x6cc, &kept_colour, white);

    // the debug output names: a table of 0x48 bytes, cleared, then each name's value put in its place
    value(0x248c) = (uint32_t)(uintptr_t)((Alloc)0xbcdfd0)(0x48);
    zero_values((uint8_t *)(uintptr_t)(uint32_t)value(0x248c), 0x12);
    static const uint32_t names[14][2] = {{0x10f3318, 4},    {0x10f331c, 8},    {0x10f3324, 0xc},  {0x10f3328, 0x10}, {0x10f332c, 0x14},
                                          {0x10f3330, 0x18}, {0x10f3334, 0x28}, {0x10f333c, 0x2c}, {0x10f3338, 0x30}, {0x10f3344, 0x34},
                                          {0x10f3348, 0x38}, {0x10f334c, 0x3c}, {0x10f3340, 0x40}, {0x10f3350, 0x44}};
    for (const auto &n : names) {
        const uint32_t made = ((uint32_t(__thiscall *)(void *, uint32_t))0xc73c20)(self, *(volatile uint32_t *)n[0]);
        *(volatile uint32_t *)((uint8_t *)(uintptr_t)(uint32_t)value(0x248c) + n[1]) = made;
    }

    zero_values(self + 0x20f4, 0xe3);
    for (const DeviceFragment &f : device_fragments) value(f.slot) = (uint32_t)(uintptr_t)device_fragment((const char *)(uintptr_t)f.text);

    set4(self + 0x1a90, 0, 0, 0, 0x3f800000);
    set4(self + 0x1a80, 0, 0x3f800000, 0xbf800000, 0x443f4666);
    set4(self + 0x1a70, 0x40008102, 0x40000000, 0xbf800000, 0);
    value(0x498) = 0xf;
    value(0x44c) = 0;
    void *state = ((Alloc)0xbcdfc0)(0x58);
    value(0x448) = (uint32_t)(uintptr_t)(state ? ((void *(__thiscall *)(void *, void *))0xbe2030)(state, self) : nullptr);
    ((Self)0xc061d0)(self);
    ((void(__thiscall *)(void *, uint32_t))0xc75290)(self, 0);
    return self;
}
static const uint8_t device_construct_original[] = {0x55, 0x8b, 0xec, 0x6a, 0xff, 0x68, 0x5b, 0x43, 0xea, 0x00, 0x64, 0xa1};
REWRITE("GE_Device::GE_Device", 0xc77890, device_construct_original, &device_construct, &g_device_constructs);
