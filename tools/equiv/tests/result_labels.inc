// Dialog::Label (forever/src/rewrite/result_labels.cpp). The name is a real text (dn_) at arena 0x40; the copy's
// statics (flags, the " - " text, two colours) set in the call from choices kept in the arena and put back after;
// atexit, the lookup and the label's constructor are stand-ins; the label's memory sometimes none.
#if EQUIV_PART == 1
static const uint32_t rl_rows[][9] = {
#define RESULT_LABELS(at, flags, cleanup, dash, colour1, colour2, prefix, table, size, ...) {at, flags, cleanup, dash, colour1, colour2, prefix, table, size},
#include "../../../forever/src/rewrite/result_labels.inc"
#undef RESULT_LABELS
};
static uint32_t g_rl_row, g_rl_saved[4];
static void *__cdecl rl_alloc(uint32_t size) { dev_note(530, size); return size == 0xe4 && dev_next() % 5 == 0 ? nullptr : dev_take(size); }
static int __cdecl rl_atexit(uint32_t fn) { dev_note(531, fn); return 0; }
static void *__thiscall rl_lookup(uint32_t *r, DnText **text, uint32_t table, uint32_t flag) { dev_note(532, dn_hash(*text)); dev_note(532, table); dev_note(532, flag); r[0] = dn_hash(*text); return r; }
static void *__thiscall rl_label(void *memory, uint32_t *r, uint32_t dash, uint32_t size, uint32_t c1, uint32_t c2, uint32_t a, uint32_t b, uint32_t c) {
    dev_note(533, (uint32_t)((uintptr_t)memory - (uintptr_t)arena<void>(0))); dev_note(533, r[0]); dev_note(533, dash); dev_note(533, size);
    dev_note(533, c1); dev_note(533, c2); dev_note(533, a); dev_note(533, b); dev_note(533, c);
    dev_note(533, *(uint32_t *)(uintptr_t)c1); dev_note(533, *(uint32_t *)(uintptr_t)c2);
    return dev_next() % 4 ? memory : arena<void>(0x7f00);
}
static void __fastcall rl_lookup_gone(uint32_t *r) { dev_note(534, r[0]); }
#else
    Test result_labels{0x55d300,
                       [=](Rng &r) {
                           fresh(r);
                           arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                           for (int i = 0; i < 0x800; i++) arena<uint32_t>(0x2000)[i] = r.next();
                           uint16_t name[] = {'R', 'i', 'd', 'e'};
                           *arena<DnText *>(0x40) = dn_make((DnText *)dev_take(0x10), name, (uint32_t)r.range(0, 4));
                           arena<uint32_t>(0x50)[0] = r.next() & 7 | (r.next() & 0xfff0);   // the statics' flags
                           arena<uint32_t>(0x50)[1] = r.next(), arena<uint32_t>(0x50)[2] = r.next(), arena<uint32_t>(0x50)[3] = r.next();
                       },
                       [](void *fn) {
                           const uint32_t *row = rl_rows[g_rl_row];
                           *(uint32_t *)(uintptr_t)row[1] = arena<uint32_t>(0x50)[0];
                           *(uint32_t *)(uintptr_t)row[3] = arena<uint32_t>(0x50)[1];
                           *(uint32_t *)(uintptr_t)row[4] = arena<uint32_t>(0x50)[2];
                           *(uint32_t *)(uintptr_t)row[5] = arena<uint32_t>(0x50)[3];
                           void *back = ((void *(__stdcall *)(void *))fn)(arena<void>(0x40));
                           uint32_t h = *arena<uint32_t>(0x20);
                           for (int k = 1; k < 6; k++) h = h * 31 + *(uint32_t *)(uintptr_t)row[k] * (k == 3 ? 0 : 1);   // (the static text's address differs: its contents are noted)
                           return Result{(uint32_t)((uintptr_t)back - (uintptr_t)arena<void>(0)) ^ h};
                       }};
    result_labels.iterations = 500;
    result_labels.prepare = [](const rewrite::Entry *e) {
        for (uint32_t k = 0; k < 3; k++)
            if (rl_rows[k][0] == e->at) g_rl_row = k;
        for (int k = 0; k < 4; k++) g_rl_saved[k] = *(uint32_t *)(uintptr_t)rl_rows[g_rl_row][k == 0 ? 1 : k + 2];
        stubs_for_now({{0xbcdfc0, (void *)rl_alloc}, {0xbce010, (void *)cheat_free_aligned}, {0xbce000, (void *)cheat_free}, {0xbcf850, (void *)dn_text_c},
                       {0xddc870, (void *)rl_atexit}, {0x403bb0, (void *)ds_join}, {0xbd01c0, (void *)dn_copy}, {0x410c90, (void *)rl_lookup},
                       {0x615ee0, (void *)rl_label}, {0xbd1880, (void *)rl_lookup_gone}});
    };
    result_labels.finish = [](const rewrite::Entry *) {
        unstub_many();
        for (int k = 0; k < 4; k++) *(uint32_t *)(uintptr_t)rl_rows[g_rl_row][k == 0 ? 1 : k + 2] = g_rl_saved[k];
    };
    quick(result_labels);
#endif
