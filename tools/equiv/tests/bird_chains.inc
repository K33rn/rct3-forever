// Bird::MarkedAbove (forever/src/rewrite/bird_chains.cpp). A chain of up to five objects at arena 0x1000 + 0x400k
// (each its parent kept already or looked up: its reference at +ref points to the next object's holder, whose class
// info gives the next object's offset 0 or none); some marked. Only al is compared.
#if EQUIV_PART == 1
static const uint32_t bc_rows[][6] = {
#define BIRD_CHAINS(at, flag, ref, number, kept, mark, ...) {at, flag, ref, number, kept, mark},
#include "../../../forever/src/rewrite/bird_chains.inc"
#undef BIRD_CHAINS
};
static uint32_t g_bc_row, g_bc_saved;
static void *__thiscall bc_info(uint8_t *object) { dev_note(370, (uint32_t)(uintptr_t)object); return *(uint8_t **)(object + 4); }
static void *bc_table[2] = {nullptr, (void *)bc_info};
#else
    Test bird_chains{0xa113d0,
                     [=](Rng &r) {
                         fresh(r);
                         arena<uint32_t>(0x20)[0] = 0;
                         const uint32_t *row = bc_rows[g_bc_row];
                         *(int32_t *)(uintptr_t)row[3] = 0;
                         const int length = r.range(0, 5);
                         for (int k = 0; k <= 5; k++) {
                             uint8_t *o = arena<uint8_t>(0x1000 + 0x400 * k), *ref = o + 0x300, *holder = o + 0x380, *info = o + 0x3c0;
                             o[row[5]] = (uint8_t)(r.range(0, 4) == 0);
                             *(void **)(o + row[2]) = ref;
                             const bool next = k < length;
                             *(void **)(ref + 0xc) = next && r.range(0, 5) ? holder : nullptr;   // (sometimes no object, or no entry)
                             *(void **)holder = bc_table, *(void **)(holder + 4) = info;
                             *(int32_t *)(info + 0x18) = 1, *(void **)(info + 0x14) = info + 0x20;
                             *(void **)(info + 0x20) = next && r.range(0, 7) ? info + 0x28 : nullptr;
                             *(int32_t *)(info + 0x28 + 0x10) = 0x80;   // (the next object: this holder + 0x80)
                             o[row[1]] = (uint8_t)(r.range(0, 3) == 0);
                             if (o[row[1]]) *(void **)(o + row[4]) = next ? arena<void>(0x1000 + 0x400 * (k + 1)) : nullptr;
                         }
                     },
                     [](void *fn) {
                         const uint32_t back = ((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x1000));
                         return Result{(back & 0xff) ^ *arena<uint32_t>(0x20)};
                     }};
    bird_chains.iterations = 2000;
    bird_chains.prepare = [](const rewrite::Entry *e) {
        for (uint32_t k = 0; k < 4; k++)
            if (bc_rows[k][0] == e->at) g_bc_row = k;
        g_bc_saved = *(uint32_t *)(uintptr_t)bc_rows[g_bc_row][3];
    };
    bird_chains.finish = [](const rewrite::Entry *) { *(uint32_t *)(uintptr_t)bc_rows[g_bc_row][3] = g_bc_saved; };
    quick(bird_chains);
#endif
