// Names::FromText (forever/src/rewrite/names_by_text.cpp). The text given is a real name of the copy's kinds (the
// game's own names function, run as it is) with the prefix taken off and its letters' case mixed, or one cut short, or
// junk; texts are real (dn_). _stricmp is a stand-in (the C library's needs its locale set up).
#if EQUIV_PART == 1
static const uint32_t nt_rows[][4] = {
#define NAMES_BY_TEXT(at, prefix, names, count, ...) {at, prefix, names, count},
#include "../../../forever/src/rewrite/names_by_text.inc"
#undef NAMES_BY_TEXT
};
static uint32_t g_nt_row;
static void *__cdecl nt_alloc(uint32_t size) { dev_note(240, size); return dev_take(size); }
static int __cdecl nt_stricmp(const char *a, const char *b) {
    uint32_t h = 0;
    for (const char *p = a; *p; p++) h = h * 31 + (uint8_t)*p;
    dev_note(241, h); dev_note(241, (uint32_t)(uintptr_t)b);
    for (;; a++, b++) {
        const int x = tolower((uint8_t)*a), y = tolower((uint8_t)*b);
        if (x != y) return x - y;
        if (!x) return 0;
    }
}
#else
    Test names_by_text{0x5538e0,
                       [=](Rng &r) {
                           fresh(r);
                           arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                           const uint32_t *row = nt_rows[g_nt_row];
                           const int32_t i = r.range(0, (int32_t)row[3] - 1);
                           const char *name = ((const char *(__fastcall *)(int32_t))(uintptr_t)row[2])(i), *prefix = (const char *)(uintptr_t)row[1];
                           size_t skip = 0;
                           while (prefix[skip] && tolower((uint8_t)prefix[skip]) == tolower((uint8_t)name[skip])) skip++;
                           if (prefix[skip]) skip = 0;   // (a name without the prefix: kept whole)
                           uint16_t wide[64];
                           uint32_t n = 0;
                           for (const char *p = name + skip; *p && n < 60; p++) wide[n++] = (uint16_t)(r.range(0, 1) ? toupper((uint8_t)*p) : tolower((uint8_t)*p));
                           switch (r.range(0, 5)) {
                           case 0: if (n) n--; break;                                  // cut short
                           case 1: n = r.range(0, 6); for (uint32_t k = 0; k < n; k++) wide[k] = (uint16_t)r.range(0x20, 0x17f); break;   // junk
                           case 2: if (n) wide[r.range(0, n - 1)] = 0; break;          // a 0 inside
                           default: break;
                           }
                           *arena<DnText *>(0x40) = dn_make((DnText *)dev_take(0x10), wide, n);
                       },
                       [](void *fn) {
                           const int32_t back = ((int32_t(__fastcall *)(void *))fn)(arena<void>(0x40));
                           return Result{(uint32_t)back ^ *arena<uint32_t>(0x20)};
                       }};
    names_by_text.iterations = 1000;
    names_by_text.prepare = [](const rewrite::Entry *e) {
        for (uint32_t k = 0; k < 3; k++)
            if (nt_rows[k][0] == e->at) g_nt_row = k;
        stubs_for_now({{0xbcdfc0, (void *)nt_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)dn_text_c},
                       {0x403bb0, (void *)ds_join}, {0xbcffe0, (void *)dn_room}, {0xde22df, (void *)nt_stricmp}});
    };
    names_by_text.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(names_by_text);
#endif
