// HashTable::Insert (forever/src/rewrite/hash_inserts.cpp). A table in the arena (1 to 4 buckets, up to 6 entries,
// sorted bucket starts), its keys drawn from eight counted texts (counts 1 or 2, so an assignment sometimes frees one);
// the room-making stand-in notes the table, the hash's gives a value by the case; the frees noted. The result: the
// whole table and every text's count.
#if EQUIV_PART == 1
static uint32_t hi_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0; }
static void __fastcall hi_room(uint8_t *self) { dev_note(770, hi_at(self)); }
static uint32_t __fastcall hi_hash(uint8_t *self, void *, uint32_t *key) { dev_note(771, hi_at(self)); dev_note(771, hi_at((void *)(uintptr_t)*key)); return arena<uint32_t>(0x50)[0]; }
static void __cdecl hi_free_aligned(void *p) { dev_note(772, hi_at(p)); }
static void __cdecl hi_free(void *p) { dev_note(773, hi_at(p)); }
#else
    Test hash_insert{0xbdf1c0,
                     [=](Rng &r) {
                         fresh(r);
                         auto at = [](uint32_t off) { return (uint32_t)(uintptr_t)arena<void>(off); };
                         for (uint32_t i = 0; i < 8; i++) {   // the texts (0x400 + 0x20 i): count, ..., memory
                             uint32_t *t = arena<uint32_t>(0x400 + 0x20 * i);
                             t[0] = r.range(1, 3), t[1] = r.next(), t[2] = r.next(), t[3] = 0x800 + 0x10 * i + at(0);
                         }
                         uint32_t *table = arena<uint32_t>(0x100);
                         uint32_t buckets = r.range(1, 5), count = r.range(0, 7);
                         table[0] = r.next(), table[1] = buckets, table[2] = count;
                         table[3] = at(0x140), table[4] = at(0x180), table[5] = at(0x1c0);
                         uint32_t *starts = arena<uint32_t>(0x140);
                         starts[0] = 0;
                         for (uint32_t b = 1; b < buckets; b++) starts[b] = r.range(starts[b - 1], count + 1);
                         starts[buckets] = count;
                         for (uint32_t i = 0; i < 8; i++) {
                             arena<uint32_t>(0x180)[i] = at(0x400 + 0x20 * r.range(0, 8));
                             arena<uint32_t>(0x1c0)[i] = r.next();
                         }
                         arena<uint32_t>(0x80)[0] = at(0x400 + 0x20 * r.range(0, 8));   // the key
                         arena<uint32_t>(0x84)[0] = r.next();   // the value
                         arena<uint32_t>(0x50)[0] = r.next();   // the hash
                     },
                     [](void *fn) {
                         ((void(__fastcall *)(void *, void *, void *, void *))fn)(arena<void>(0x100), nullptr, arena<void>(0x80), arena<void>(0x84));
                         uint32_t h = 0;
                         for (uint32_t i = 0; i < 3; i++) h = h * 31 + arena<uint32_t>(0x100)[i];
                         for (uint32_t i = 0; i < 6; i++) h = h * 31 + arena<uint32_t>(0x140)[i];
                         for (uint32_t i = 0; i < 8; i++) h = h * 31 + hi_at((void *)(uintptr_t)arena<uint32_t>(0x180)[i]);
                         for (uint32_t i = 0; i < 8; i++) h = h * 31 + arena<uint32_t>(0x1c0)[i];
                         for (uint32_t i = 0; i < 8; i++) h = h * 31 + arena<uint32_t>(0x400 + 0x20 * i)[0];
                         return Result{h};
                     }};
    hash_insert.iterations = 1000;
    hash_insert.prepare = [](const rewrite::Entry *e) {
        stubs_for_now({{e->at == 0xbdf1c0 ? 0xbe0730u : 0xbe0840u, (void *)hi_room}, {0xbdbf70, (void *)hi_hash}, {0xbce010, (void *)hi_free_aligned},
                       {0xbce000, (void *)hi_free}});
    };
    hash_insert.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(hash_insert);
#endif
