// ShapeMaterial::FromName (forever/src/rewrite/shape_materials.cpp). The shape at arena 0x100 (count +0x24, list
// +0x28 at 0x300); material k at 0x400 + 0x40k, its name object at 0x800 + 0x40k with the name's C text at +8. The
// texts are real (the dn_ stand-ins: made, cut, compared, joined, given room); found or not by a hash of the key.
#if EQUIV_PART == 1
static uint32_t g_sm_saved[2];
static void *__cdecl sm_alloc(uint32_t size) { dev_note(190, size); return dev_take(size); }   // (never none: "Mask" is not checked)
static void *__thiscall sm_name(DnText **out, const char *c) { dev_note(191, (uint32_t)(uintptr_t)c); *out = (DnText *)dn_text_c((DnText *)dev_take(0x10), c); return out; }
static int32_t __thiscall sm_find(DnText **in, const char *what, uint32_t from) {
    const DnText *t = *in;
    uint32_t n = 0;
    while (what[n]) n++;
    dev_note(192, dn_hash(t)); dev_note(192, n); dev_note(192, from);
    for (uint32_t q = 0; q < n; q++) dev_note(192, (uint8_t)what[q]);   // (what is looked for: "Alpha" and "Glass" are the same length)
    for (uint32_t k = from; k + n <= t->length; k++) {
        uint32_t q = 0;
        while (q < n && t->characters[k + q] == (uint8_t)what[q]) q++;
        if (q == n) return (int32_t)k;
    }
    return -1;
}
static void *__thiscall sm_text_c(DnText *self, const char *c) { return dn_text_c(self, c); }
static void *__thiscall sm_narrow_c(DnText *self, const char *c) { dev_note(194, 1); return dn_text_c(self, c); }   // (told from the wide one)
static uint32_t __thiscall sm_look_up(void *manager, DnText **key) {
    const uint32_t h = dn_hash(*key);
    dev_note(193, (uint32_t)(uintptr_t)manager); dev_note(193, h);
    return (h ^ *arena<uint32_t>(0x44)) % 3 ? h | 1 : 0;   // (found or not differs from case to case, even for one key)
}
static const char *const sm_names[] = {"SIGlass", "SIAlphaText", "AlphaMask_x", "WoodAlpha", "Alpha", "BigAlphaTree", "Glass", "FillZ",
                                       "CausticsA", "Water2", "Plain", "sialphatext", "Rock", "xAlphaAlpha", "SIGlas", "AlphaMaskAlpha"};
#else
    Test shape_materials{0xccb610,
                         [=](Rng &r) {
                             fresh(r);
                             for (int i = 0; i < 0x1000; i++) arena<uint32_t>(0x2000)[i] = r.next();
                             uint8_t *shape = arena<uint8_t>(0x100);
                             const int32_t n = r.range(0, 5);
                             *(int32_t *)(shape + 0x24) = n;
                             *(void **)(shape + 0x28) = arena<void>(0x300);
                             for (int k = 0; k < 6; k++) {
                                 uint8_t *m = arena<uint8_t>(0x400 + 0x40 * k), *name = arena<uint8_t>(0x800 + 0x40 * k);
                                 arena<void *>(0x300)[k] = m;
                                 *(void **)(m + 8) = name;
                                 *(uint32_t *)(m + 0x10) = r.next() & (r.range(0, 1) ? 0xfffffff5u : 0xffffffffu);
                                 const char *s = sm_names[r.range(0, 15)];
                                 for (int q = 0;; q++) { name[8 + q] = (uint8_t)s[q]; if (!s[q]) break; }
                             }
                             *(int32_t *)(uintptr_t)0x12f014c = r.range(0, 3) ? 1 : r.range(0, 2);
                             *(void **)(uintptr_t)0x13d0e04 = arena<void>(0x1f00);
                             *arena<uint32_t>(0x44) = r.next();
                             arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                         },
                         [](void *fn) {
                             ((void(__stdcall *)(void *))fn)(arena<void>(0x100));
                             return Result{*arena<uint32_t>(0x20)};
                         }};
    shape_materials.iterations = 500;
    shape_materials.prepare = [](const rewrite::Entry *) {
        g_sm_saved[0] = *(uint32_t *)(uintptr_t)0x12f014c, g_sm_saved[1] = *(uint32_t *)(uintptr_t)0x13d0e04;
        stubs_for_now({{0xbcdfc0, (void *)sm_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcfef0, (void *)sm_name},
                       {0xbd0960, (void *)sm_find}, {0xbcf850, (void *)sm_text_c}, {0xbcfb10, (void *)sm_narrow_c}, {0xbce970, (void *)dn_compare},
                       {0xbdf160, (void *)sm_look_up}, {0xbd0810, (void *)dn_part}, {0x403bb0, (void *)ds_join}, {0xbcffe0, (void *)dn_room}});
    };
    shape_materials.finish = [](const rewrite::Entry *) {
        unstub_many();
        *(uint32_t *)(uintptr_t)0x12f014c = g_sm_saved[0], *(uint32_t *)(uintptr_t)0x13d0e04 = g_sm_saved[1];
    };
    quick(shape_materials);
#endif
