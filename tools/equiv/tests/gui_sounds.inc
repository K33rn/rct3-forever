// Sound::GUISound (forever/src/rewrite/gui_sounds.cpp). Sound on or off; rand() a stand-in; the sound made is at
// arena 0x600 (its voice at 0x700, its old name a counted text); memory never fails (without a sound or a name the
// game would use none). The copy's three sample globals get values of their own and are put back.
#if EQUIV_PART == 1
static const uint32_t gs_rows[][5] = {
#define GUI_SOUNDS(at, first, second, third, name, ...) {at, first, second, third, name},
#include "../../../forever/src/rewrite/gui_sounds.inc"
#undef GUI_SOUNDS
};
static uint32_t g_gs_saved[4], g_gs_row;
static void *__cdecl gs_alloc(uint32_t size) { dev_note(230, size); return dev_take(size); }
static int __cdecl gs_rand() { const int v = (int)(dev_next() & 0x7fff); dev_note(231, (uint32_t)v); return v; }
static void *__thiscall gs_sound(uint8_t *memory, uint32_t a, uint32_t b, uint32_t sample, uint32_t c) {
    dev_note(232, (uint32_t)(uintptr_t)memory); dev_note(232, a); dev_note(232, b); dev_note(232, sample); dev_note(232, c);
    return arena<void>(0x600);
}
static void __thiscall gs_start(uint8_t *voice, uint8_t *at) { dev_note(233, (uint32_t)(uintptr_t)voice); dev_note(233, (uint32_t)(uintptr_t)at); }
#else
    Test gui_sounds{0x69a6e0,
                    [=](Rng &r) {
                        fresh(r);
                        for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                        arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0;   // (before any dev_take: fresh() leaves its pointer random)
                        *(uint8_t *)(uintptr_t)0x10eedd0 = (uint8_t)(r.range(0, 4) ? 1 + (r.next() & 0xfe) : 0);
                        for (int k = 1; k < 4; k++) *(uint32_t *)(uintptr_t)gs_rows[g_gs_row][k] = r.next();
                        uint8_t *sound = arena<uint8_t>(0x600);
                        for (int k = 0; k < 0x50; k++) sound[k] = (uint8_t)r.next();
                        *(uint32_t *)(sound + 0x18) = r.range(0, 1) ? 0 : r.next() | 1;
                        sound[0x11] = (uint8_t)r.range(0, 1);
                        *(void **)(sound + 0xc) = arena<void>(0x700);
                        uint32_t *old = (uint32_t *)dev_take(0x10);
                        old[0] = r.range(1, 2), old[3] = 0xf0000000u | (uint32_t)(uintptr_t)old;
                        *(uint32_t **)(sound + 0x44) = old;
                    },
                    [](void *fn) {
                        ((void(__cdecl *)())fn)();
                        return Result{*arena<uint32_t>(0x20)};
                    }};
    gui_sounds.iterations = 1000;
    gui_sounds.prepare = [](const rewrite::Entry *e) {
        for (uint32_t k = 0; k < 3; k++)
            if (gs_rows[k][0] == e->at) g_gs_row = k;
        g_gs_saved[0] = *(uint8_t *)(uintptr_t)0x10eedd0;
        for (int k = 1; k < 4; k++) g_gs_saved[k] = *(uint32_t *)(uintptr_t)gs_rows[g_gs_row][k];
        stubs_for_now({{0xbcdfc0, (void *)gs_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)cheat_text},
                       {0xddedd7, (void *)gs_rand}, {0x47cee0, (void *)gs_sound}, {0xce4770, (void *)gs_start}});
    };
    gui_sounds.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint8_t *)(uintptr_t)0x10eedd0 = (uint8_t)g_gs_saved[0];
        for (int k = 1; k < 4; k++) *(uint32_t *)(uintptr_t)gs_rows[g_gs_row][k] = g_gs_saved[k];
    };
    quick(gui_sounds);
#endif
