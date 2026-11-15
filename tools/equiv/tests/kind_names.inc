// Condition::KindName (forever/src/rewrite/kind_names.cpp). Kinds -1 to 4; the text reference's stand-in notes the key
// and holds a text or none (by the case); memory sometimes none; the copies, the empty text and the literal text noted.
#if EQUIV_PART == 1
static bool g_kn_stack;   // (the copy takes its values on the stack)
static uint32_t kn_at(const void *p) { return p ? (uint32_t)((uintptr_t)p - (uintptr_t)arena<void>(0)) : 0; }
static void *__fastcall kn_reference(uint8_t *self, void *, uint32_t key, uint32_t table, uint32_t flag) {
    dev_note(750, key); dev_note(750, table); dev_note(750, flag);
    *(void **)(self + 8) = arena<uint32_t>(0x50)[1] ? arena<void>(0x300) : nullptr;
    return self;
}
static void __fastcall kn_reference_gone(uint8_t *self) { dev_note(751, kn_at(*(void **)(self + 8))); }
static void *__cdecl kn_alloc(uint32_t size) { dev_note(752, size); return dev_next() % 4 ? dev_take(0x10) : nullptr; }
static void *__fastcall kn_copy(uint32_t *self, void *, void *from) { dev_note(753, kn_at(from)); self[0] = 0xc0b1; return self; }
static void *__fastcall kn_empty(uint32_t flag) { dev_note(754, flag); return arena<void>(0x380); }
static void *__fastcall kn_literal(uint32_t *self, void *, const char *c) { dev_note(755, (uint32_t)(uintptr_t)c); self[0] = 0x117e; return self; }
#else
    Test kind_name{0x6df770,
                   [=](Rng &r) {
                       fresh(r);
                       arena<uint32_t>(0x20)[0] = 0, arena<uint32_t>(0x24)[0] = 0, arena<uint32_t>(0x28)[0] = r.next(), arena<uint32_t>(0x2c)[0] = 0x2000;
                       arena<int32_t>(0x50)[0] = r.range(0, 11) - 1;   // the kind (-1 to 10: past every copy's count)
                       arena<uint32_t>(0x50)[1] = r.range(0, 2) != 0;   // the key holds a text
                       arena<uint32_t>(0x60)[0] = r.next();
                   },
                   [](void *fn) {
                       void *back = g_kn_stack ? ((void *(__stdcall *)(void *, uint32_t))fn)(arena<void>(0x60), arena<uint32_t>(0x50)[0])
                                               : ((void *(__fastcall *)(void *, uint32_t))fn)(arena<void>(0x60), arena<uint32_t>(0x50)[0]);
                       return Result{kn_at(back) ^ *arena<uint32_t>(0x20) ^ kn_at(*arena<void *>(0x60)) * 3};
                   }};
    kind_name.iterations = 1000;
    kind_name.prepare = [](const rewrite::Entry *e) {
        g_kn_stack = e->at == 0xb6fe70;
        stubs_for_now({{0x4040f0, (void *)kn_reference}, {0xbd1880, (void *)kn_reference_gone}, {0xbcdfc0, (void *)kn_alloc}, {0xbcf7d0, (void *)kn_copy},
                       {0xbd0490, (void *)kn_empty}, {0xbcf850, (void *)kn_literal}});
    };
    kind_name.finish = [](const rewrite::Entry *) { unstub_many(); };
    quick(kind_name);
#endif
