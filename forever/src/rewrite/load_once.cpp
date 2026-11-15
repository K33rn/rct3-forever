// Folder::LoadOnce99F890 and Folder::LoadOnce99FB20 (one function written out twice; tools/family.py: load_once.inc;
// they differ in their flag, folder and window): open a designer's folder window, unless its flag says it is
// open already (static; no values).
//   - The path: the game folder (0x458130) + "\" + the user folder (0x458150) + "\" (0x403BE0 and 0x403BB0 joins);
//     the parts let go (the joins first, newest first, then the two folders).
//   - The window (0x160 new bytes; none without memory): built (the copy's constructor with 0, 0 and the path +
//     the copy's folder, "LaserEffects\" ...); that text let go.
//   - Centred on the screen window [0x12F83F0]: x = (its width - 460) / 2, y = (its height - 360) / 2 (the game's
//     constants), x made 46 when the global object [0x1342708] has its own object marked (+0xA4) and the game value
//     [0x12F4618]'s +4 is below 1000; laid out (0x6B1640, 6, the screen window) at {x, y, x + its width, y + its
//     height}, the sums on the FPU rounded where the game stores them.
//   - The path let go.
#include "rewrite.h"

namespace {
struct LoadOnce {
    uint32_t at, flag, folder, make;
};
#define LOAD_ONCE(at, flag, folder, make, ...) {at, flag, folder, make},
constexpr LoadOnce load_once[] = {
#include "load_once.inc"
};
#undef LOAD_ONCE
#define LOAD_ONCE(at, flag, folder, make, ...) {__VA_ARGS__},
const uint8_t load_once_first[][7] = {
#include "load_once.inc"
};
#undef LOAD_ONCE

typedef void(__cdecl *Free)(void *);
inline void let_go(int32_t *volatile *held) {   // a shared text let go (read again, as the game does)
    --**held;
    int32_t *text = *held;
    if (*(volatile int32_t *)text == 0) {
        ((Free)0xbce010)((void *)(uintptr_t)(uint32_t)text[3]);
        ((Free)0xbce000)(text);
    }
}
typedef long double ext;
inline float rf(ext x) {
    volatile float f = (float)x;
    return f;
}
typedef void *(__thiscall *JoinC)(void *, void *, uintptr_t);
typedef void *(__thiscall *Join)(void *, void *, void *);
inline float at(const uint8_t *w, uint32_t offset) { return *(const float *)(w + offset); }

volatile long g_load_once;
void load_one(const LoadOnce &kind) {
    g_load_once++;
    if (*(volatile uint8_t *)(uintptr_t)kind.flag) return;
    int32_t *volatile user, *volatile game, *volatile a, *volatile b, *volatile path;
    void *u = ((void *(__thiscall *)(void *))0x458150)((void *)&user);
    void *g = ((void *(__thiscall *)(void *))0x458130)((void *)&game);
    void *ja = ((JoinC)0x403be0)(g, (void *)&a, 0xf265e4);   // "\"
    void *jb = ((Join)0x403bb0)(ja, (void *)&b, u);
    ((JoinC)0x403be0)(jb, (void *)&path, 0xf265e4);
    let_go(&b);
    let_go(&a);
    let_go(&game);
    let_go(&user);
    uint8_t *window = nullptr;
    if (void *memory = ((void *(__cdecl *)(uint32_t))0xbcdfc0)(0x160)) {
        int32_t *volatile folder;
        void *t = ((JoinC)0x403be0)((void *)&path, (void *)&folder, kind.folder);
        window = (uint8_t *)((void *(__thiscall *)(void *, uint32_t, uint32_t, void *))kind.make)(memory, 0, 0, t);
        let_go(&folder);
    }
    const uint8_t *screen = *(const uint8_t *volatile *)(uintptr_t)0x12f83f0;
    const ext half = *(const float *)(uintptr_t)0xf2651c;
    const ext y = (((ext)at(screen, 0x88) - at(screen, 0x80)) - *(const float *)(uintptr_t)0xf3c288) * half;   // - 360
    ext x = (((ext)at(screen, 0x84) - at(screen, 0x7c)) - *(const float *)(uintptr_t)0xf27374) * half;         // - 460
    uint8_t **global = *(uint8_t **volatile *)(uintptr_t)0x1342708;
    if (global && *global && (*global)[0xa4] && *(int32_t *)(*(uint8_t *volatile *)(uintptr_t)0x12f4618 + 4) < 1000) x = *(const float *)(uintptr_t)0xf4c1d8;   // 46
    const float height = rf((ext)at(window, 0x88) - at(window, 0x80));
    const ext width = (ext)at(window, 0x84) - at(window, 0x7c);
    volatile float rect[4];
    rect[0] = rf(x);
    rect[1] = rf(y);
    rect[2] = rf(width + x);
    rect[3] = rf((ext)height + y);
    ((void(__thiscall *)(void *, uint32_t, const void *, volatile float *))0x6b1640)(window, 6, screen, rect);
    let_go(&path);
}

template <int K> void __cdecl load_one_copy() { load_one(load_once[K]); }
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(load_once[0].at == 0x99f890 && load_once[1].at == 0x99fb20, "load_once.inc changed");
REWRITE("Folder::LoadOnce", 0x99f890, load_once_first[0], (&load_one_copy<0>), &g_load_once);
REWRITE("Folder::LoadOnce", 0x99fb20, load_once_first[1], (&load_one_copy<1>), &g_load_once);
