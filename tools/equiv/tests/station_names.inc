// Names::FromTextInline (forever/src/rewrite/station_names.cpp). As names_by_text.inc (whose stand-ins it uses): the
// text given is one of the copy's names with the prefix taken off and its case mixed, cut short, with a 0 inside, or
// junk; texts are real (dn_); _stricmp a stand-in.
#if EQUIV_PART == 1
static const uint32_t sn2_rows[][8] = {
#define STATION_NAMES(at, prefix, n0, n1, n2, n3, n4, ...) {at, prefix, n0, n1, n2, n3, n4, 5},
#include "../../../forever/src/rewrite/station_names.inc"
#undef STATION_NAMES
    {0xbb45f0, 0xf9f140, 0xf9f124, 0xf9f108, 0xf9f0e0, 0xf9f0bc, 0, 4},
    {0xbbfde0, 0xf9fef8, 0xf9febc, 0xf9fed4, 0, 0, 0, 2},
};
static uint32_t g_sn2_row;
#else
    Test station_names{0xbb3e90,
                       [=](Rng &r) {
                           fresh(r);
                           arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                           const uint32_t *row = sn2_rows[g_sn2_row];
                           const char *name = (const char *)(uintptr_t)row[2 + r.range(0, (int32_t)row[7] - 1)], *prefix = (const char *)(uintptr_t)row[1];
                           size_t skip = 0;
                           while (prefix[skip] && tolower((uint8_t)prefix[skip]) == tolower((uint8_t)name[skip])) skip++;
                           if (prefix[skip]) skip = 0;
                           uint16_t wide[64];
                           uint32_t n = 0;
                           for (const char *p = name + skip; *p && n < 60; p++) wide[n++] = (uint16_t)(r.range(0, 1) ? toupper((uint8_t)*p) : tolower((uint8_t)*p));
                           switch (r.range(0, 5)) {
                           case 0: if (n) n--; break;
                           case 1: n = r.range(0, 6); for (uint32_t k = 0; k < n; k++) wide[k] = (uint16_t)r.range(0x20, 0x17f); break;
                           case 2: if (n) wide[r.range(0, n - 1)] = 0; break;
                           default: break;
                           }
                           *arena<DnText *>(0x40) = dn_make((DnText *)dev_take(0x10), wide, n);
                       },
                       [](void *fn) {
                           const int32_t back = ((int32_t(__fastcall *)(void *))fn)(arena<void>(0x40));
                           return Result{(uint32_t)back ^ *arena<uint32_t>(0x20)};
                       }};
    station_names.iterations = 1000;
    station_names.prepare = [](const rewrite::Entry *e) {
        for (uint32_t k = 0; k < 4; k++)
            if (sn2_rows[k][0] == e->at) g_sn2_row = k;
        stubs_for_now({{0xbcdfc0, (void *)nt_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)dn_text_c},
                       {0x403bb0, (void *)ds_join}, {0xbcffe0, (void *)dn_room}, {0xde22df, (void *)nt_stricmp}});
    };
    station_names.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(station_names);
#endif
