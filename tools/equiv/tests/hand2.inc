// Tests for forever/src/rewrite/hand2.cpp (hand-written rewrites, one at a time).
#if EQUIV_PART == 1
// a stand-in for the game's _tolower (0xDDE005), which needs the C library's per-thread data the tester doesn't have
static int __cdecl h2_lower(int c) { return c >= 'A' && c <= 'Z' ? c + 32 : c ^ (c >> 8 & 0x55); }
// a stand-in for Window::UpdateRect (0x6AFAF0): notes the part and the rectangle at arena 0x40
static uint32_t __thiscall h2_update_rect(void *part, const float *rect) {
    uint32_t *log = arena<uint32_t>(0x40);
    log[1] = (uint32_t)(uintptr_t)part;
    for (int k = 0; k < 4; k++) memcpy(&log[2 + k], &rect[k], 4);
    log[0]++;
    return log[0] * 7 + 3;
}
// a stand-in for the collision call 0xCDA7F0: notes its two objects and both points at arena 0x60, answers by them
static uint8_t __fastcall h2_hit(uint32_t a, uint32_t b, const float *from, const float *to, float *out) {
    uint32_t *log = arena<uint32_t>(0x60);
    log[0] = a, log[1] = b;
    for (int k = 0; k < 3; k++) memcpy(&log[2 + k], &from[k], 4), memcpy(&log[5 + k], &to[k], 4);
    uint32_t h = a ^ b;
    for (int k = 2; k < 8; k++) h = h * 31 + log[k];
    *out = 1.0f;
    return (uint8_t)(h >> 7);
}
// a stand-in for the animal interface getter 0xB44970: the object at arena 0x600
static void *__thiscall h2_animal_iface(void *) { return arena<void>(0x600); }
// a stand-in virtual for the GUI root: notes the point at arena 0x70 and answers by it
static uint32_t __thiscall h2_root_slot(void *root, const float *p) {
    uint32_t *log = arena<uint32_t>(0x70);
    memcpy(&log[0], &p[0], 4), memcpy(&log[1], &p[1], 4);
    log[2] = (uint32_t)(uintptr_t)root;
    return log[0] ^ (log[1] * 3);
}
// a stand-in for Effect::Value7B05E0 (a, b in ecx, edx, c on the stack): notes them at arena 0x80, answers by them
static long double __fastcall h2_effect_value(uint32_t a, uint32_t b, uint32_t c) {
    uint32_t *log = arena<uint32_t>(0x80);
    log[0] = a, log[1] = b, log[2] = c;
    return (long double)(int32_t)((a ^ (b * 7)) & 0xffff) / 4096.0L - 4.0L;
}
// a stand-in for QuadTree::Move (0x431240): notes the tree, the value its item points at, the handle and both boxes at
// arena 0x80
static void __thiscall h2_quad_move(void *tree, const int32_t *item, void *handle, const float *a, const float *b) {
    uint32_t *log = arena<uint32_t>(0x80);
    log[0] = (uint32_t)(uintptr_t)tree, log[1] = (uint32_t)*item, log[2] = (uint32_t)(uintptr_t)handle;
    for (int k = 0; k < 3; k++) memcpy(&log[3 + k], &a[k], 4), memcpy(&log[6 + k], &b[k], 4);
    log[9]++;
}
// stand-ins for Water::BoxSpan (0x8E6980) and Water::Point8E6D20 (0x8E6D20): note what they are given at arena 0xA0 and
// 0xC0 and write a mark into their outputs
static float *__thiscall h2_box_span(const void *self, const float *lo, const float *hi, float *out, uint8_t grow) {
    uint32_t *log = arena<uint32_t>(0xa0);
    log[0] = (uint32_t)(uintptr_t)self, log[1] = (uint32_t)(uintptr_t)lo, log[2] = (uint32_t)(uintptr_t)hi, log[3] = (uint32_t)(uintptr_t)out, log[4] = grow;
    log[5]++;
    out[0] = lo[0] + hi[1];
    return out;
}
static void __thiscall h2_water_point(const void *self, const float *m, float h, float *low, float *high) {
    uint32_t *log = arena<uint32_t>(0xc0);
    log[0] = (uint32_t)(uintptr_t)self, log[1] = (uint32_t)(uintptr_t)m, log[3] = (uint32_t)(uintptr_t)low, log[4] = (uint32_t)(uintptr_t)high;
    memcpy(&log[2], &h, 4);
    log[5]++;
    low[0] = m[0] + h, high[0] = m[0] - h;
}
// a stand-in for WildAnimal::InterfaceB35450 (0xB35450): the animal's +0x140 on the FPU
static long double __thiscall h2_animal_class_value(const uint8_t *animal) { return *(const float *)(animal + 0x140); }
// a stand-in for Triple::Set (0x6A4B10): notes its part and three values at arena 0xE0 (each call after the last) and
// gives the call count
static uint32_t __thiscall h2_triple_set(void *part, uint32_t a, uint32_t b, uint32_t c) {
    uint32_t *log = arena<uint32_t>(0xe0);
    const uint32_t n = log[0]++;
    log[1 + n * 4] = (uint32_t)(uintptr_t)part, log[2 + n * 4] = a, log[3 + n * 4] = b, log[4 + n * 4] = c;
    return n;
}
// a stand-in for the trail's grow (0x913CA0, this = the array {entries, room}): notes n at arena 0x90 and sets the room
// (the arena already has the space)
static void __thiscall h2_trail_grow(uint32_t *array, int32_t n) {
    arena<uint32_t>(0x90)[0]++, arena<int32_t>(0x90)[1] = n;
    array[1] = (uint32_t)n;
}
// a stand-in for _stricmp (0xDE22DF): ASCII letters alike in either case
static int __cdecl h2_stricmp(const char *a, const char *b) {
    for (;; a++, b++) {
        int x = (uint8_t)*a, y = (uint8_t)*b;
        if (x >= 'A' && x <= 'Z') x += 32;
        if (y >= 'A' && y <= 'Z') y += 32;
        if (x != y || !x) return x - y;
    }
}
// a stand-in for a list's grow (0xCE7110, this = {entries, room}): notes n at arena 0x90 and sets the room (the arena
// already has the space)
static void __thiscall h2_list_grow(uint32_t *array, int32_t n) {
    arena<uint32_t>(0x90)[0]++, arena<int32_t>(0x90)[1] = n;
    array[1] = (uint32_t)n;
}
// a stand-in for Pool::Grow42D730 ({items, free list, used, room}): notes n at arena 0x98, sets the room and fills the
// free list's new end with item numbers 0-7 (the arena already has the space)
static void __thiscall h2_pool_grow(uint32_t *pool, int32_t n) {
    arena<uint32_t>(0x98)[0]++, arena<int32_t>(0x98)[1] = n;
    uint32_t *free_list = (uint32_t *)(uintptr_t)pool[1];
    for (int32_t k = (int32_t)pool[3] - (int32_t)pool[2]; k < n - (int32_t)pool[2]; k++) free_list[k] = (uint32_t)(k * 5 + n) & 7;
    pool[3] = (uint32_t)n;
}
// a stand-in for Shared::Own (0xBCFFE0, this = the text's holder): notes each n at arena 0xA0 and sets the room to n when
// above it (the arena already has the space)
static void __thiscall h2_text_own(uint8_t **holder, uint32_t n) {
    uint32_t *log = arena<uint32_t>(0xa0);
    log[1 + (log[0]++ & 3)] = n;
    if (n > *(uint32_t *)(*holder + 8)) *(uint32_t *)(*holder + 8) = n;
}
// stand-ins for Slots::Free (0x42F6F0) and Grid::MarkUp (0x42F670): each notes its this and argument at arena 0xB0
static void __thiscall h2_slot_free(void *self, uint32_t n) {
    uint32_t *log = arena<uint32_t>(0xb0);
    log[1] = (uint32_t)(uintptr_t)self, log[2] = n, log[0]++;
}
static void __thiscall h2_block_mark(void *self, uint32_t n) {
    uint32_t *log = arena<uint32_t>(0xb0);
    log[3] = (uint32_t)(uintptr_t)self, log[4] = n, log[0] += 0x100;
}
// stand-ins for Grid::GrowM4 (0x850030: notes n at arena 0xC0, gives arena 0x600) and Pool::Free (0x8501C0: notes its
// arguments at arena 0xC8)
static void *__thiscall h2_grid_take(void *, int32_t n) {
    arena<uint32_t>(0xc0)[0]++, arena<int32_t>(0xc0)[1] = n;
    return arena<void>(0x600);
}
static void __thiscall h2_grid_give(void *grid, int32_t n, void *block) {
    uint32_t *log = arena<uint32_t>(0xc8);
    log[0] = (uint32_t)(uintptr_t)grid, log[1] = (uint32_t)n, log[2] = (uint32_t)(uintptr_t)block;
}
// a stand-in for Water::Remove95A570 (0x95A570): notes its this and corners at arena 0xD0 and gives arena 0xD0's first dword
static uint32_t __thiscall h2_edge_find(void *self, uint32_t a, uint32_t b) {
    uint32_t *log = arena<uint32_t>(0xd0);
    log[1] = (uint32_t)(uintptr_t)self, log[2] = a, log[3] = b;
    return log[0];
}
// a stand-in for Water::Entry94EF60 (0x94EF60): notes each this and n at arena 0xE0 (up to 8 calls)
static void __thiscall h2_water_release(void *self, int32_t n) {
    uint32_t *log = arena<uint32_t>(0xe0);
    const uint32_t k = log[0]++ & 7;
    log[1 + k * 2] = (uint32_t)(uintptr_t)self, log[2 + k * 2] = (uint32_t)n;
}
// a stand-in for the animal's interface (0xADB340): notes its this at arena 0xF8 and gives arena 0x180
static void *__thiscall h2_animal_interface(void *self) {
    arena<uint32_t>(0xf8)[0] = (uint32_t)(uintptr_t)self;
    return arena<void>(0x180);
}
// a stand-in for Animals::PassB6C900 (0xB6C900): notes its this at arena 0xFC and gives arena 0x1F0's first dword
static uint32_t __thiscall h2_animals_pass(void *self) {
    arena<uint32_t>(0xfc)[0] = (uint32_t)(uintptr_t)self;
    return arena<uint32_t>(0x1f0)[0];
}
// a stand-in for Track::RoundBB69E0 (0xBB69E0): notes its keys and n at arena 0x70 and gives the key and fraction kept at
// arena 0x80 (the key 0-2, then a float)
static void __thiscall h2_track_round(void *, const uint16_t *list, uint32_t n, int32_t *place, float *part) {
    uint32_t *log = arena<uint32_t>(0x70);
    log[0]++, log[1] = (uint32_t)(uintptr_t)list, log[2] = n;
    *place = arena<int32_t>(0x80)[0], *part = arena<float>(0x80)[1];
}
// a stand-in for QuadTree::Insert (0x4311C0): notes its tree, the key's and box's three dwords at arena 0x140 and gives
// arena 0x15C's dword
static uint32_t __thiscall h2_tree_insert(void *tree, const uint32_t *key, const uint32_t *lo, const uint32_t *hi) {
    uint32_t *log = arena<uint32_t>(0x140);
    log[0] = (uint32_t)(uintptr_t)tree;
    for (int k = 0; k < 3; k++) log[1 + k] = key[k], log[4 + k] = lo[k] | hi[k] << 1;
    return log[7];
}
// a stand-in for Names::Find (0x805C10): notes its name, table and count at arena 0x160 and gives the found flag and place
// kept at arena 0x170 (both outs set)
static uint8_t __fastcall h2_names_find(void *name, void *table, int32_t count, int32_t *at, int32_t *at2) {
    uint32_t *log = arena<uint32_t>(0x160);
    log[0] = (uint32_t)(uintptr_t)name, log[1] = (uint32_t)(uintptr_t)table, log[2] = (uint32_t)count, log[3] = (uint32_t)(at == at2);
    *at = arena<int32_t>(0x170)[1];
    return (uint8_t)arena<uint32_t>(0x170)[0];
}
// a stand-in for an object's interface (0x4712B0): the object itself
static void *__thiscall h2_same_object(void *self) { return self; }
#else
    // Inflate::BuildHuffmanTable: complete, incomplete, all-zero and random code lengths for bit-length codes (s 19),
    // literals (s 257) and distances (s 0); sometimes little room left
    Test huft_build{0xcbb250,
                    [=](Rng &r) {
                        uint32_t *A = arena<uint32_t>(0x20);
                        const int kind = r.range(0, 2);
                        const uint32_t n = kind == 0 ? 19 : kind == 1 ? (uint32_t)r.range(257, 288) : (uint32_t)r.range(1, 30);
                        A[0] = n, A[1] = kind == 0 ? 19 : kind == 1 ? 257 : 0;
                        A[2] = (uint32_t)(uintptr_t)arena<void>(0x2000), A[3] = (uint32_t)(uintptr_t)arena<void>(0x2100);
                        A[4] = (uint32_t)(uintptr_t)arena<void>(0x10), A[5] = (uint32_t)(uintptr_t)arena<void>(0x4000);
                        A[6] = (uint32_t)(uintptr_t)arena<void>(0x18), A[7] = (uint32_t)(uintptr_t)arena<void>(0x1800);
                        for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x2000)[i] = r.next(), arena<uint32_t>(0x2100)[i] = r.range(0, 13);
                        for (int i = 0; i < 0x5a0 * 2; i++) arena<uint32_t>(0x4000)[i] = r.next();
                        for (int i = 0; i < 0x120; i++) arena<uint32_t>(0x1800)[i] = r.next();
                        arena<uint32_t>(0x10)[0] = r.next(), arena<uint32_t>(0x14)[0] = r.range(1, 15);
                        arena<uint32_t>(0x18)[0] = r.range(0, 7) == 0 ? r.range(1300, 1440) : r.range(0, 60);
                        uint32_t *b = arena<uint32_t>(0x1000);
                        for (uint32_t i = 0; i < n; i++) b[i] = 0;
                        const int mode = r.range(0, 9);
                        if (mode == 0) {
                            for (uint32_t i = 0; i < n; i++) b[i] = r.range(0, 15);
                        } else if (mode == 1) {
                            if (r.range(0, 1)) b[r.range(0, n - 1)] = r.range(1, 15);
                        } else {
                            // a complete code: leaves split at random up to the count wanted, then one taken off (incomplete) at times
                            uint32_t depth[300];
                            uint32_t leaves = 2, want = (uint32_t)r.range(2, (int)n < 2 ? 2 : (int)n);
                            depth[0] = depth[1] = 1;
                            for (int tries = 0; leaves < want && tries < 2000; tries++) {
                                const uint32_t at = r.range(0, leaves - 1);
                                if (depth[at] >= (uint32_t)r.range(4, 15)) continue;
                                depth[at]++, depth[leaves++] = depth[at];
                            }
                            if (mode == 2 && leaves > 1) leaves--;
                            if (leaves > n) leaves = n;
                            for (uint32_t i = 0; i < leaves; i++) {
                                uint32_t at = r.range(0, n - 1);
                                while (b[at]) at = (at + 1) % n;
                                b[at] = depth[i];
                            }
                        }
                    },
                    [](void *fn) {
                        uint32_t eax = (uint32_t)(uintptr_t)arena<void>(0x14);
                        uint32_t *A = arena<uint32_t>(0x20);
                        void *b = arena<void>(0x1000);
                        __asm__ volatile("pushl 28(%%esi)\n\tpushl 24(%%esi)\n\tpushl 20(%%esi)\n\tpushl 16(%%esi)\n\tpushl 12(%%esi)\n\tpushl 8(%%esi)\n\tpushl 4(%%esi)\n\tpushl (%%esi)\n\tcall *%%ebx"
                                         : "+a"(eax), "+c"(b), "+S"(A)
                                         : "b"(fn)
                                         : "edx", "edi", "memory", "cc");
                        for (int i = 0; i < 0x5a0; i++) arena<uint16_t>(0x4002)[4 * i] = 0;   // the stack bytes the game copies
                        return Result{eax};
                    }};
    quick(huft_build);
    // Inflate::Fast: random input decoded with zlib's fixed tables (the game's, 0x10F3480 and 0x10F4480) into a 32 KB
    // window, written at any point with room for at least 258; bits on hand 0-16
    Test inflate_fast{0xcc0670,
                      [=](Rng &r) {
                          uint8_t *z = arena<uint8_t>(0x100), *s = arena<uint8_t>(0x200);
                          for (int i = 0; i < 0x800; i++) arena<uint8_t>(0x1000)[i] = (uint8_t)r.next();
                          for (int i = 0; i < 0x2000; i++) arena<uint32_t>(0x8000)[i] = r.next();
                          *(uint32_t *)z = (uint32_t)(uintptr_t)arena<void>(0x1000), *(uint32_t *)(z + 4) = r.range(10, 0x7f0);
                          *(uint32_t *)(z + 8) = r.next(), *(uint32_t *)(z + 0x18) = r.next();
                          const uint32_t k = r.range(0, 16);
                          *(uint32_t *)(s + 0x1c) = k, *(uint32_t *)(s + 0x20) = r.next() & ((1u << k) - 1);
                          uint8_t *window = arena<uint8_t>(0x8000), *end = window + 0x8000;
                          *(uint32_t *)(s + 0x28) = (uint32_t)(uintptr_t)window, *(uint32_t *)(s + 0x2c) = (uint32_t)(uintptr_t)end;
                          uint8_t *q, *rd;
                          if (r.range(0, 1)) {
                              q = window + r.range(0, 0x8000 - 0x103), rd = window + r.range(0, (int)(q - window));
                          } else {
                              q = window + r.range(0, 0x7000), rd = q + r.range(0x103, (int)(end - q));
                          }
                          *(uint32_t *)(s + 0x30) = (uint32_t)(uintptr_t)rd, *(uint32_t *)(s + 0x34) = (uint32_t)(uintptr_t)q;
                      },
                      [](void *fn) {
                          return Result{(uint32_t)((int32_t(__fastcall *)(uint32_t, uint32_t, const void *, const void *, void *, void *))fn)(
                              9, 5, (const void *)0x10f3480, (const void *)0x10f4480, arena<void>(0x200), arena<void>(0x100))};
                      }};
    quick(inflate_fast);
    // ResourceFile::Load(this, a, header): either byte order (the flag in the game's .data), 0-5 entries (or a negative
    // count), the record apart from the header
    Test resource_file_load{0xccdd90,
                            [=](Rng &r) {
                                for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x400)[i] = r.next(), arena<uint32_t>(0x600)[i] = r.next(), arena<uint32_t>(0x800)[i] = r.next();
                                arena<uint32_t>(0x400)[4] = (uint32_t)(uintptr_t)arena<void>(0x600), arena<uint32_t>(0x400)[6] = (uint32_t)(uintptr_t)arena<void>(0x800);
                                const int32_t n = r.range(-1, 5);
                                const uint8_t flag = (uint8_t)(r.range(0, 2) ? r.range(1, 255) : 0);
                                arena<uint32_t>(0x400)[5] = flag ? __builtin_bswap32((uint32_t)n) : (uint32_t)n;
                                *(volatile uint8_t *)0x13d0df2 = flag;
                                arena<uint32_t>(0x10)[0] = r.next();
                            },
                            [](void *fn) {
                                return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t, void *))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], arena<void>(0x400))};
                            }};
    t.push_back(resource_file_load);
    // VertexDeclaration::FillBDD9A0 / FillBDE5E0: 0-8 kinds, 0-9 parts in runs of kinds 2, 0x25 and 0x1D, single values
    // (places 1 and 2) or lists of 0-4 (places 3 and 4, n from -1), output at any alignment
    auto declaration_setup = [=](Rng &r) {
        for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x100)[i] = r.next();
        uint8_t *self = arena<uint8_t>(0x100);
        *(uint32_t *)(self + 4) = (uint32_t)(uintptr_t)arena<void>(0x200), *(uint32_t *)(self + 8) = (uint32_t)(uintptr_t)arena<void>(0x280);
        *(uint16_t *)(self + 0xc) = (uint16_t)r.range(0, 3);
        *(uint32_t *)arena<uint8_t>(0x338) = (uint32_t)(uintptr_t)arena<void>(0x340);
        for (int i = 0; i < 4; i++) arena<uint32_t>(0x340)[i] = (uint32_t)(uintptr_t)arena<void>(0x400);
        uint8_t *rec = arena<uint8_t>(0x400);
        for (int i = 0; i < 8; i++) arena<uint32_t>(0x400)[i] = r.next();
        *(uint32_t *)rec = (uint32_t)(uintptr_t)arena<void>(0x480), *(uint32_t *)(rec + 4) = (uint32_t)(uintptr_t)arena<void>(0x500);
        const int parts = r.range(0, 9), kinds = r.range(0, 8);
        *(uint32_t *)(rec + 0x10) = parts, *(uint32_t *)(rec + 0x14) = kinds;
        for (int i = 0; i < 0x20; i++) arena<uint8_t>(0x500)[i] = (uint8_t)(r.range(0, 2) ? 1 : r.next());
        static const uint8_t kind_of[] = {2, 0x25, 0x1d};
        uint8_t kind = kind_of[r.range(0, 2)];
        for (int i = 0; i < 10; i++) {
            if (r.range(0, 3) == 0) kind = kind_of[r.range(0, 2)];
            const bool many = r.range(0, 2) == 0;
            arena<uint8_t>(0x480)[2 * i] = kind, arena<uint8_t>(0x480)[2 * i + 1] = many ? (uint8_t)r.range(1, 255) : 0;
            arena<uint16_t>(0x200)[i] = (uint16_t)((many ? r.range(2, 3) : r.range(0, 1)) << 14 | 8 * r.range(0, 7));
        }
        for (int i = 0; i < 10; i++) arena<uint16_t>(0x280)[i] = (uint16_t)(r.range(0, 1) << 14 | 8 * r.range(0, 7));
        for (int i = 0; i < 4; i++) arena<uint32_t>(0x604)[i] = (uint32_t)(uintptr_t)arena<void>(0x1000 + 0x100 * i);
        for (int i = 0; i < 0x80; i++) arena<uint32_t>(0x1000)[i] = r.range(0, 1) ? r.next() : r.range(0, 0x120) * r.range(0, 1);
        for (int i = 0; i < 0x80; i++) {
            arena<int32_t>(0x1200)[i] = (i & 1) ? (int32_t)(uintptr_t)arena<void>(0x2000 + 0x20 * (i & 7)) : r.range(-1, 4);
        }
        for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x2000)[i] = r.range(0, 1) ? r.next() : 0;
        for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x3000)[i] = r.next();
        *(uint32_t *)arena<uint8_t>(0x710) = (uint32_t)(uintptr_t)arena<void>(0x3000), *(uint16_t *)arena<uint8_t>(0x70e) = (uint16_t)r.range(0, 9);
    };
    Test declaration_fill_bdd9a0{0xbdd9a0, declaration_setup, [](void *fn) {
                                     return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x300), arena<void>(0x600), arena<void>(0x700))};
                                 }};
    quick(declaration_fill_bdd9a0);
    Test declaration_fill_bde5e0{0xbde5e0, declaration_setup, [](void *fn) {
                                     return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x300), arena<void>(0x600), arena<void>(0x700))};
                                 }};
    quick(declaration_fill_bde5e0);
    // Terrain::Indices8F0D30: ring sizes -1 to 6, the same or not, any first point
    Test terrain_indices{0x8f0d30,
                         [=](Rng &r) {
                             int32_t *t = arena<int32_t>(0x100);
                             t[0] = r.range(-1, 6), t[1] = r.range(-1, 6);
                             if (r.range(0, 1)) t[2] = t[0], t[3] = t[1];
                             else t[2] = r.range(-1, 6), t[3] = r.range(-1, 6);
                             *(uint32_t *)arena<uint8_t>(0x120) = (uint32_t)(uintptr_t)arena<void>(0x1000);
                             *(uint32_t *)arena<uint8_t>(0x124) = r.next();
                             for (int i = 0; i < 0x200; i++) arena<uint32_t>(0x1000)[i] = r.next();
                         },
                         [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    quick(terrain_indices);
    // Path::Update71C230: on or off, the shared count above 0 or not, boxes inside an 8x8 grid of cells in use or not
    Test path_update_71c230{0x71c230,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                self[0xa0] = (uint8_t)(r.range(0, 5) ? r.range(1, 255) : 0);
                                *(volatile int32_t *)0x130634c = r.range(-1, 3);
                                self[0x54] = 8, *(uint32_t *)(self + 0x58) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                const int density = r.range(0, 4);
                                for (int i = 0; i < 64; i++) {
                                    uint8_t *c = arena<uint8_t>(0x800 + 0x10 * i);
                                    for (int j = 0; j < 4; j++) ((uint32_t *)c)[j] = r.next();
                                    if (r.range(0, 9) >= density) {
                                        *(uint16_t *)(c + 0xa) = 0, c[6] = 0, *(uint32_t *)(c + 0xc) = 0;
                                        if (r.range(0, 3) == 0) *(uint16_t *)(c + 0xa) = 1;
                                        else if (r.range(0, 3) == 0) c[6] = 1;
                                        else if (r.range(0, 3) == 0) *(uint32_t *)(c + 0xc) = 1;
                                    }
                                    arena<uint32_t>(0x400)[i] = r.range(0, 4) ? (uint32_t)(uintptr_t)c : 0;
                                }
                                self[0x9c] = (uint8_t)r.range(0, 7), self[0x9d] = (uint8_t)r.range(0, 7);
                                self[0x9e] = (uint8_t)r.range(0, 7), self[0x9f] = (uint8_t)r.range(0, 7);
                            },
                            [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    t.push_back(path_update_71c230);
    // Text::FindBDEB50: two objects with or without descriptions, 0-5 values of each size kind, places in a, b, c or the
    // objects (themselves or their parts)
    Test text_find_bdeb50{0xbdeb50,
                          [=](Rng &r) {
                              for (int i = 0; i < 0x400; i++) arena<uint32_t>(0x4000)[i] = r.next();
                              for (int i = 0; i < 0x400; i++) arena<uint32_t>(0x2000)[i] = r.next();
                              for (int k = 0; k < 2; k++) {
                                  uint8_t *p = arena<uint8_t>(0x600 + 0x100 * k), *q = arena<uint8_t>(0x800 + 0x40 * k);
                                  arena<uint32_t>(0x500)[k] = (uint32_t)(uintptr_t)p, *(uint32_t *)p = (uint32_t)(uintptr_t)q;
                                  *(uint32_t *)(q + 0x24) = r.range(0, 0x40), *(uint32_t *)(q + 0x20) = r.range(0, 3);
                                  uint8_t *d = arena<uint8_t>(0x900 + 0x100 * k);
                                  arena<uint32_t>(0x510)[k] = r.range(0, 4) ? (uint32_t)(uintptr_t)d : 0;
                                  const int count = r.range(0, 5);
                                  *(uint16_t *)d = (uint16_t)count, *(uint16_t *)(d + 2) = (uint16_t)r.range(count, count + 3);
                                  *(uint32_t *)(d + 4) = (uint32_t)(uintptr_t)(d + 0x10), *(uint32_t *)(d + 8) = (uint32_t)(uintptr_t)(d + 0x40);
                                  for (int i = 0; i < 8; i++) {
                                      static const uint8_t hi[] = {0, 0, 0, 1, 2, 0x80, 0x81, 0x83};
                                      ((uint16_t *)(d + 0x10))[i] = (uint16_t)(hi[r.range(0, 7)] << 8 | r.range(0, 37));
                                  }
                                  for (int i = 0; i < 16; i++) {
                                      const int top = r.range(0, 3);
                                      ((uint16_t *)(d + 0x40))[i] = (uint16_t)(top < 3 ? top << 14 | r.range(0, 0x80) : 0xc000 | r.range(0, 1) << 13 | r.range(0, 0x60));
                                  }
                              }
                              for (int i = 0; i < 3; i++) arena<uint32_t>(0x20)[i] = (uint32_t)(uintptr_t)arena<void>(0x2000 + 0x200 * i);
                              arena<uint32_t>(0x4014)[0] = (uint32_t)(uintptr_t)arena<void>(0x3000), arena<uint32_t>(0x4018)[0] = (uint32_t)(uintptr_t)arena<void>(0x3400);
                          },
                          [](void *fn) {
                              return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *, void *, uint32_t, uint32_t, uint32_t))fn)(
                                  arena<void>(0x4000), arena<void>(0x500), arena<void>(0x510), arena<uint32_t>(0x20)[0], arena<uint32_t>(0x20)[1], arena<uint32_t>(0x20)[2])};
                          }};
    quick(text_find_bdeb50);
    // Stream::ReadC51E00: requests before, inside, across and past the stream's range, with or without buffers
    Test stream_read_c51e00{0xc51e00,
                            [=](Rng &r) {
                                for (int i = 0; i < 0x400; i++) arena<uint32_t>(0x1000)[i] = r.next(), arena<uint32_t>(0x2000)[i] = r.next();
                                int32_t *s = arena<int32_t>(0x100), *q = arena<int32_t>(0x200);
                                for (int i = 0; i < 8; i++) s[i] = (int32_t)r.next(), q[i] = (int32_t)r.next();
                                const int32_t start = r.range(0, 0x100);
                                s[5] = start, s[6] = r.range(0, 3) ? start + r.range(0, 0x200) : start;
                                s[4] = r.range(0, 0x100);
                                const int which = r.range(0, 3);
                                s[1] = which < 3 ? (int32_t)(uintptr_t)arena<void>(0x1000) - (which == 2 ? s[4] : 0) : 0;
                                if (which == 2) s[1] = -s[4];
                                s[2] = which == 3 ? (int32_t)(uintptr_t)arena<void>(0x1000) : which == 2 ? (int32_t)(uintptr_t)arena<void>(0x1400) : 0;
                                q[1] = r.range(0, 5) ? (int32_t)(uintptr_t)arena<void>(0x2000) : 0;
                                q[2] = r.range(0, 5) ? 0 : (int32_t)(uintptr_t)arena<void>(0x2000);
                                q[6] = which == 2 ? start : start + r.range(-0x20, 0x220), q[5] = r.range(0, 0x40), q[3] = q[6] - q[5] + r.range(-0x10, 0x200);
                                q[4] = r.range(0, 0x100) + q[5] - q[6];
                                q[7] = r.range(0, 5) ? r.range(0, 9) : -1;
                            },
                            [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200))}; }};
    quick(stream_read_c51e00);
    // Options::ApplyKeys: the commands' options chosen at random (0-40)
    Test options_apply_keys{0x860a20,
                            [=](Rng &r) {
                                for (int i = 0; i < 47; i++) ((volatile uint32_t *)0x10f0940)[i] = r.range(0, 40);
                            },
                            [](void *fn) { return Result{((uint32_t(*)())fn)()}; }};
    t.push_back(options_apply_keys);
    // Water::Loop8E8D80: heaps of 0-12 of 8 records (in use or not, keys with repeats), any start
    Test water_sift_8e8d80{0x8e8d80,
                           [=](Rng &r) {
                               *(uint32_t *)arena<uint8_t>(0x218) = (uint32_t)(uintptr_t)arena<void>(0x1000);
                               for (int k = 0; k < 8; k++) {
                                   uint8_t *rec = arena<uint8_t>(0x1000 + 0x130 * k);
                                   rec[0] = (uint8_t)(r.range(0, 4) ? r.range(1, 255) : 0);
                                   *(uint32_t *)(rec + 0xa0) = (uint32_t)(uintptr_t)arena<void>(0x800 + 4 * k);
                                   arena<uint32_t>(0x800)[k] = r.range(0, 1) ? r.range(0, 5) : r.next();
                               }
                               const int n = r.range(0, 12);
                               arena<int32_t>(0x10)[0] = r.range(0, n > 0 ? n - 1 : 0), arena<int32_t>(0x10)[1] = n;
                               for (int k = 0; k < 16; k++) arena<int32_t>(0x400)[k] = r.range(0, 7);
                           },
                           [](void *fn) {
                               ((void(__thiscall *)(void *, int32_t, int32_t, void *))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<int32_t>(0x10)[1], arena<void>(0x400));
                               return Result{};
                           }};
    quick(water_sift_8e8d80);
    // Terrain::Range7AE6F0: grids of 1-8 by 1-8 cells anywhere in byte space, few or many set, points inside and out
    Test terrain_range_7ae6f0{0x7ae6f0,
                              [=](Rng &r) {
                                  uint8_t *self = arena<uint8_t>(0x100);
                                  self[4] = (uint8_t)r.range(0, 250), self[5] = (uint8_t)r.range(0, 250);
                                  self[6] = (uint8_t)r.range(1, 8), self[7] = (uint8_t)r.range(1, 8);
                                  *(uint32_t *)(self + 8) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                  const int density = r.range(0, 30);
                                  for (int i = 0; i < 64; i++) arena<uint8_t>(0x400)[i] = (uint8_t)(r.range(0, 99) < density ? r.range(1, 255) : 0);
                                  arena<uint32_t>(0x10)[0] = (uint32_t)(self[4] + r.range(-12, 20)) & 0xff | (r.next() & 0xffffff00u);
                                  arena<uint32_t>(0x10)[1] = (uint32_t)(self[5] + r.range(-12, 20)) & 0xff | (r.next() & 0xffffff00u);
                                  arena<uint8_t>(0x20)[0] = (uint8_t)r.next(), arena<uint8_t>(0x24)[0] = (uint8_t)r.next();
                              },
                              [](void *fn) {
                                  return Result{((uint8_t(__thiscall *)(void *, uint32_t, uint32_t, void *, void *))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], arena<uint32_t>(0x10)[1], arena<void>(0x20), arena<void>(0x24))};
                              }};
    quick(terrain_range_7ae6f0);
    // PathGrid::InsideB9B090: grids of 1-6 by 1-6 with 3 owners, cells inside, on the edge and outside
    Test path_grid_inside{0xb9b090,
                          [=](Rng &r) {
                              uint8_t *self = arena<uint8_t>(0x100);
                              *(int32_t *)(self + 0x64) = r.range(1, 6), *(int32_t *)(self + 0x68) = r.range(1, 6);
                              *(uint32_t *)(self + 0x6c) = (uint32_t)(uintptr_t)arena<void>(0x400);
                              for (int i = 0; i < 36; i++) arena<uint32_t>(0x400)[6 * i] = r.range(0, 2) == 2 ? 0xffffffffu : r.range(0, 1);
                              arena<uint32_t>(0x10)[0] = r.range(0, 7) | (r.next() & 0xffffff00u), arena<uint32_t>(0x10)[1] = r.range(0, 7) | (r.next() & 0xffffff00u);
                          },
                          [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], arena<uint32_t>(0x10)[1])}; }};
    quick(path_grid_inside);
    // Terrain::Loop8D67A0: cells of two 4-wide grids (or the same grid), bits already set or not
    Test terrain_cell_copy{0x8d67a0,
                           [=](Rng &r) {
                               uint8_t *a = arena<uint8_t>(0x100), *b = arena<uint8_t>(0x180);
                               a[4] = 4, b[4] = (uint8_t)r.range(1, 4);
                               *(uint32_t *)(a + 0x24) = (uint32_t)(uintptr_t)arena<void>(0x400);
                               *(uint32_t *)(b + 0x24) = (uint32_t)(uintptr_t)arena<void>(r.range(0, 3) ? 0x800 : 0x400);
                               for (int i = 0; i < 0x200; i++) arena<uint32_t>(0x400)[i] = r.next();
                               for (int i = 0; i < 4; i++) arena<uint32_t>(0x10)[i] = r.range(0, 3) | (r.next() & 0xffffff00u);
                               arena<uint32_t>(0x20)[0] = r.range(0, 3) ? 0x180 : 0x100;
                           },
                           [](void *fn) {
                               uint32_t *a = arena<uint32_t>(0x10);
                               void *from = arena<void>(0x100), *to = arena<void>(arena<uint32_t>(0x20)[0]);
                               uint32_t eax;
                               __asm__ volatile("pushl 12(%%ecx)\n\tpushl 8(%%ecx)\n\tpushl 4(%%ecx)\n\tpushl (%%ecx)\n\tcall *%%ebx"
                                                : "=a"(eax), "+S"(from), "+d"(to), "+c"(a)
                                                : "b"(fn)
                                                : "edi", "memory", "cc");
                               return Result{};
                           }};
    quick(terrain_cell_copy);
    // Table::FreeCA6900: eight slots with links, in-use bits, ring places and the free list all at random (in range)
    Test table_free_ca6900{0xca6900,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               *(uint32_t *)self = (uint32_t)(uintptr_t)arena<void>(0x400), *(uint32_t *)(self + 4) = (uint32_t)(uintptr_t)arena<void>(0x800);
                               *(uint16_t *)(self + 8) = (uint16_t)r.next(), *(uint16_t *)(self + 0xa) = (uint16_t)r.range(0, 7);
                               for (int k = 0; k < 8; k++) {
                                   uint8_t *s = arena<uint8_t>(0x400 + 8 * k);
                                   *(uint32_t *)s = 4 * r.range(0, 15) | r.range(0, 1);
                                   *(uint16_t *)(s + 4) = (uint16_t)r.range(0, 7), *(uint16_t *)(s + 6) = (uint16_t)r.range(0, 7);
                               }
                               for (int i = 0; i < 0x20; i++) arena<uint16_t>(0x800)[i] = (uint16_t)r.range(0, 7);
                               arena<uint32_t>(0x10)[0] = r.range(0, 7) | (r.next() & 0xfff80000u);
                           },
                           [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0])}; }};
    quick(table_free_ca6900);
    // Device::VersionC00C80: every major and minor (and others), the device's values at random
    Test device_version{0xc00c80,
                        [=](Rng &r) {
                            *(volatile uint32_t *)0x13d0d84 = (uint32_t)(uintptr_t)arena<void>(0x100);
                            for (int i = 0; i < 0x60; i++) arena<uint32_t>(0x100)[i] = r.next();
                            arena<uint32_t>(0x10)[0] = (r.next() & 0xff00ffffu) | (uint32_t)r.range(0, 9) << 20 | (uint32_t)r.range(0, 5) << 16;
                        },
                        [](void *fn) { return Result{((uint32_t(__stdcall *)(uint32_t))fn)(arena<uint32_t>(0x10)[0])}; }};
    t.push_back(device_version);
    // Model::FindCC7200: 0-5 entries (keys with repeats), a sorted list of 0-5 pairs that may hold the value, on or off
    Test model_find_cc7200{0xcc7200,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100), *model = arena<uint8_t>(0x200);
                               for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next();
                               *(uint32_t *)self = (uint32_t)(uintptr_t)model;
                               *(uint32_t *)(self + 0x18) = (uint32_t)(uintptr_t)arena<void>(0x300);
                               *(uint32_t *)(self + 0x1c) = (uint32_t)(uintptr_t)arena<void>(0x400);
                               const int pairs = r.range(0, 5);
                               *(int32_t *)(self + 0x20) = pairs;
                               int32_t v = r.range(-4, 2);
                               for (int i = 0; i < 8; i++) v += r.range(0, 2), arena<int32_t>(0x400)[2 * i] = v, arena<int32_t>(0x404)[2 * i] = r.range(0, 9);
                               *(int32_t *)(model + 0x1c) = r.range(0, 5);
                               *(uint32_t *)(model + 0x20) = (uint32_t)(uintptr_t)arena<void>(0x500);
                               *(uint32_t *)(model + 0x10) = (uint32_t)(uintptr_t)arena<void>(0x1000);
                               for (int i = 0; i < 8; i++) arena<uint32_t>(0x500)[i] = r.range(0, 5);
                               for (int i = 0; i < 6; i++) *(int32_t *)arena<uint8_t>(0x1058 + 0x60 * i) = r.range(-4, 12);
                               for (int i = 0; i < 0x30; i++) arena<uint32_t>(0x300)[i] = r.next();
                               arena<uint32_t>(0x10)[0] = r.range(0, 6), arena<uint32_t>(0x10)[1] = r.range(0, 1) ? r.next() : 0;
                               arena<uint32_t>(0x10)[2] = r.next(), arena<uint32_t>(0x10)[3] = r.next(), arena<uint32_t>(0x10)[4] = r.next();
                           },
                           [](void *fn) {
                               const uint32_t *a = arena<uint32_t>(0x10);
                               return Result{((uint8_t(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), a[0], a[1], a[2], a[3], a[4])};
                           }};
    quick(model_find_cc7200);
    // Water::Alpha97C630: no holder, no current item, or each pair's item the current one or not
    Test water_alpha_97c630{0x97c630,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                *(uint32_t *)(self + 0xd8) = r.range(0, 5) ? (uint32_t)(uintptr_t)arena<void>(0x300) : 0;
                                *(uint32_t *)arena<uint8_t>(0x3f0) = r.range(0, 5) ? r.range(1, 3) : 0;
                                for (uint32_t at : {0xe0u, 0xf4u, 0x108u}) *(uint32_t *)(self + at) = r.range(0, 3);
                                const uint32_t parts[] = {0xe4, 0xe8, 0xf8, 0xfc, 0x10c, 0x110};
                                for (int i = 0; i < 6; i++) *(uint32_t *)(self + parts[i]) = (uint32_t)(uintptr_t)arena<void>(0x400 + 0x100 * i), *(uint32_t *)arena<uint8_t>(0x4c4 + 0x100 * i) = r.next();
                            },
                            [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(water_alpha_97c630);
    // Shader::SetCB5BE0: 0-4 percentages (some above 100 or negative), levels and targets near each other or not
    Test shader_set_cb5be0{0xcb5be0,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               *(int32_t *)(self + 0x4c) = r.range(0, 4);
                               *(uint32_t *)(self + 0x54) = (uint32_t)(uintptr_t)arena<void>(0x400);
                               for (int i = 0; i < 8; i++) arena<int16_t>(0x400)[2 * i] = (int16_t)(r.range(0, 5) ? r.range(10, 99) : r.range(-200, 300));
                               *(int32_t *)(self + 0x60) = r.range(0, 3) ? r.range(0, 2000) : (int32_t)r.next();
                               arena<int32_t>(0x10)[0] = r.range(0, 5) ? r.range(-5, 600) : *(int32_t *)(self + 0x60);
                               // the game never ends a one-percentage run whose steps stop taking the level down: such cases start at the target
                               if (*(int32_t *)(self + 0x4c) <= 1) {
                                   const int32_t first = arena<int16_t>(0x400)[0], target = arena<int32_t>(0x10)[0];
                                   int32_t level = *(int32_t *)(self + 0x60), p = first, up = 1, steps = 0;
                                   int32_t drop = (int32_t)((uint32_t)level * (uint32_t)p) / 100;
                                   while (level > target && steps < 5000) {
                                       level -= drop, steps++;
                                       if (level == 1) break;
                                       const int32_t c = (int32_t)((uint32_t)(100 - p) * (uint32_t)first);
                                       p += up > 0 ? c / 100 : c / -100, up = up == 0;
                                       drop = (int32_t)((uint32_t)level * (uint32_t)p) / 100;
                                   }
                                   if (steps >= 5000) arena<int32_t>(0x10)[0] = *(int32_t *)(self + 0x60);
                               }
                           },
                           [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0])}; }};
    quick(shader_set_cb5be0);
    // Scenery::Pop85A9C0: 0-9 sorted pairs (repeats in a), pairs looked for that are there, between or outside
    Test scenery_pop_85a9c0{0x85a9c0,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                const int n = r.range(0, 9);
                                *(int32_t *)(self + 0x18) = n;
                                *(uint32_t *)(self + 8) = (uint32_t)(uintptr_t)arena<void>(0x400), *(uint32_t *)(self + 0x10) = (uint32_t)(uintptr_t)arena<void>(0x500);
                                *(uint32_t *)(self + 0x14) = (uint32_t)(uintptr_t)arena<void>(0x600);
                                int32_t a = r.range(-3, 3), b = r.range(-3, 3);
                                for (int i = 0; i < 10; i++) {
                                    if (r.range(0, 2) == 0) a += r.range(1, 2), b = r.range(-3, 3);
                                    else b += r.range(1, 2);
                                    arena<int32_t>(0x400)[i] = a, arena<int32_t>(0x500)[i] = b, arena<uint32_t>(0x600)[i] = r.next();
                                }
                                if (n && r.range(0, 1)) {
                                    const int i = r.range(0, n - 1);
                                    arena<int32_t>(0x10)[0] = arena<int32_t>(0x400)[i], arena<int32_t>(0x10)[1] = arena<int32_t>(0x500)[i];
                                } else {
                                    arena<int32_t>(0x10)[0] = r.range(-4, 14), arena<int32_t>(0x10)[1] = r.range(-4, 14);
                                }
                            },
                            [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, int32_t, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<int32_t>(0x10)[1])}; }};
    quick(scenery_pop_85a9c0);
    // Text::SumBD70B0: 0-4 of 6 entries with 0-4 parts each (seconds and thirds with repeats), out or not
    Test text_sum_bd70b0{0xbd70b0,
                         [=](Rng &r) {
                             uint8_t *self = arena<uint8_t>(0x100);
                             *(uint32_t *)(self + 0x10) = (uint32_t)(uintptr_t)arena<void>(0x200);
                             for (int k = 0; k < 6; k++) {
                                 uint8_t *e = arena<uint8_t>(0x300 + 0x20 * k);
                                 arena<uint32_t>(0x200)[k] = (uint32_t)(uintptr_t)e;
                                 *(uint16_t *)(e + 8) = (uint16_t)r.range(0, 4), *(uint32_t *)(e + 0xc) = (uint32_t)(uintptr_t)arena<void>(0x400 + 0x20 * k);
                             }
                             for (int i = 0; i < 0x30; i++) {
                                 uint8_t *p = arena<uint8_t>(0x400 + 4 * i);
                                 p[0] = (uint8_t)r.next(), p[1] = (uint8_t)r.range(0, 2), p[2] = (uint8_t)r.range(0, 3), p[3] = (uint8_t)r.next();
                             }
                             for (int i = 0; i < 6; i++) arena<uint32_t>(0x10)[i] = r.range(0, 5);
                             arena<uint32_t>(0x30)[0] = r.range(0, 4), arena<uint32_t>(0x30)[1] = r.range(0, 2) ? 0x800 : 0;
                             for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x800)[i] = r.next();
                         },
                         [](void *fn) {
                             void *out = arena<uint32_t>(0x30)[1] ? arena<void>(0x800) : nullptr;
                             return Result{((uint32_t(__thiscall *)(void *, void *, uint32_t, void *))fn)(arena<void>(0x100), arena<void>(0x10), arena<uint32_t>(0x30)[0], out)};
                         }};
    quick(text_sum_bd70b0);
    // ShaderFile::FindC47500: 1-4 buckets of entries, keys that match one fully, in part, or not at all
    Test shader_file_find{0xc47500,
                          [=](Rng &r) {
                              uint8_t *self = arena<uint8_t>(0x100);
                              const uint32_t buckets = r.range(1, 4);
                              *(uint32_t *)(self + 4) = buckets;
                              *(uint32_t *)(self + 0xc) = (uint32_t)(uintptr_t)arena<void>(0x200);
                              *(uint32_t *)(self + 0x10) = (uint32_t)(uintptr_t)arena<void>(0x400);
                              *(uint32_t *)(self + 0x14) = (uint32_t)(uintptr_t)arena<void>(0x800);
                              uint32_t at = 0;
                              for (uint32_t b = 0; b <= buckets; b++) arena<uint32_t>(0x200)[b] = at, at += r.range(0, 3);
                              // entries: four values from a small set, data of 0-3 values from a small set
                              for (int i = 0; i < 16; i++) {
                                  uint32_t *e = arena<uint32_t>(0x400 + 20 * i);
                                  const uint32_t words = r.range(0, 3);
                                  e[0] = r.range(0, 1), e[1] = r.range(0, 1), e[2] = 7, e[3] = words << 18 | r.range(0, 1);
                                  e[4] = (uint32_t)(uintptr_t)arena<void>(0xa00 + 0x10 * i);
                                  for (int k = 0; k < 4; k++) arena<uint32_t>(0xa00 + 0x10 * i)[k] = r.range(0, 1);
                              }
                              uint32_t *key = arena<uint32_t>(0x20);
                              const int pick = r.range(0, 15);
                              for (int k = 0; k < 5; k++) key[k] = arena<uint32_t>(0x400 + 20 * pick)[k];
                              key[4] = (uint32_t)(uintptr_t)arena<void>(0x40);
                              for (int k = 0; k < 4; k++) arena<uint32_t>(0x40)[k] = arena<uint32_t>(0xa00 + 0x10 * pick)[k] ^ (r.range(0, 5) ? 0 : 1);
                              if (r.range(0, 3) == 0) key[r.range(0, 3)] ^= 1;
                          },
                          [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x20))}; }};
    quick(shader_file_find);
    // Stream::StepC50390: with or without a part, the 32 streams on it or not, the count about to wrap or not
    Test stream_step_c50390{0xc50390,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x100)[i] = r.next();
                                *(uint32_t *)(self + 0xf0) = r.range(0, 4) ? r.range(1, 3) : 0;
                                *(uint32_t *)(self + 0x68) = r.range(0, 4) ? r.next() : 0xffffffffu;
                                for (int d = 0; d < 32; d++) {
                                    ((volatile uint32_t *)0x13e6bb8)[d] = (uint32_t)(uintptr_t)arena<void>(0x1000 + 0x100 * d);
                                    arena<uint32_t>(0x1000 + 0x100 * d)[4] = r.next(), arena<uint32_t>(0x10ec + 0x100 * d)[0] = r.range(0, 3);
                                }
                            },
                            [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    t.push_back(stream_step_c50390);
    // OverlayFile::FindStreamedItem: 0-12 entries sorted by id then value (0xFFFF among them), ids and values that are
    // there or not, values with high bits
    Test overlay_find_streamed{0xc3b2c0,
                               [=](Rng &r) {
                                   uint8_t *self = arena<uint8_t>(0x100);
                                   const int n = r.range(0, 12);
                                   *(uint32_t *)(self + 0x48) = n, *(uint32_t *)(self + 0x4c) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                   int32_t id = r.range(-3, 2);
                                   int32_t v = -1;
                                   for (int i = 0; i < 12; i++) {
                                       uint8_t *e = arena<uint8_t>(0x400 + 20 * i);
                                       if (r.range(0, 2) == 0) id += r.range(1, 2), v = r.range(-1, 2);
                                       else v += r.range(1, 2);
                                       *(int32_t *)e = id, *(uint16_t *)(e + 4) = (uint16_t)v, *(uint16_t *)(e + 6) = (uint16_t)r.next();
                                   }
                                   arena<int32_t>(0x10)[0] = r.range(-4, 14);
                                   const int32_t value = r.range(-2, 12);
                                   arena<int32_t>(0x10)[1] = r.range(0, 4) ? value : (int32_t)((uint32_t)value | 0x10000u);
                               },
                               [](void *fn) {
                                   return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, int32_t, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<int32_t>(0x10)[1])};
                               }};
    quick(overlay_find_streamed);
    // Window::Flags6F0D80: windows with random flags, each listed flag (and others) at positions 0-12
    Test window_flags_6f0d80{0x6f0d80,
                             [=](Rng &r) {
                                 static const uint32_t flags[] = {0x80, 4, 8, 0x10, 1, 0x20, 0x40, 2, 0x100, 0x200, 0x400, 0x800, 0x18};
                                 *(uint32_t *)arena<uint8_t>(0x1d0) = r.next() & 0xfff;
                                 arena<uint32_t>(0x10)[0] = flags[r.range(0, 12)], arena<int32_t>(0x10)[1] = r.range(0, 12);
                             },
                             [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t, int32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], arena<int32_t>(0x10)[1])}; }};
    quick(window_flags_6f0d80);
    // Toolbar::Next79CB60: the two shared values equal or not, indexes in and out of range, items marked or not, shown in
    // windows of kind 0xE or 0xD (lists of 0-3 windows) or not
    Test toolbar_next_79cb60{0x79cb60,
                             [=](Rng &r) {
                                 *(volatile int32_t *)0x1306bcc = r.range(0, 1), *(volatile int32_t *)0x1306bd8 = r.range(0, 4) ? 0 : 1;
                                 const int32_t begin = r.range(0, 3);
                                 *(volatile int32_t *)0x1306be0 = begin, *(volatile int32_t *)0x1306be4 = begin + r.range(0, 5);
                                 *(volatile int32_t *)0x1306bb8 = r.range(-1, 6);
                                 *(volatile uint32_t *)0x1306bc8 = (uint32_t)(uintptr_t)arena<void>(0x100);
                                 for (int i = 0; i < 12; i++) {
                                     arena<uint32_t>(0x100)[i] = r.range(0, 5) ? (uint32_t)(uintptr_t)arena<void>(0x400 + 0x100 * i) : 0;
                                     arena<uint32_t>(0x4a8 + 0x100 * i)[0] = r.range(0, 3) ? 1 : r.range(0, 2);
                                 }
                                 // two lists of nodes {next, -, window}, ending at a node whose first value is 0
                                 for (int l = 0; l < 2; l++) {
                                     const int n = r.range(0, 3);
                                     const uint32_t list = l ? 0x10ef794 : 0x10ef9d0;
                                     *(volatile uint32_t *)list = (uint32_t)(uintptr_t)arena<void>(0x1800 + 0x100 * l);
                                     for (int k = 0; k <= n; k++) {
                                         uint32_t *node = arena<uint32_t>(0x1800 + 0x100 * l + 0x10 * k);
                                         node[0] = k < n ? (uint32_t)(uintptr_t)(node + 4) : 0;
                                         uint8_t *w = arena<uint8_t>(0x2000 + 0x400 * (2 * l + (k & 1)));
                                         node[2] = r.range(0, 4) ? (uint32_t)(uintptr_t)w : 0;
                                         *(uint32_t *)(w + 0x190) = r.range(0, 1) ? (l ? 0xd : 0xe) : 0xc;
                                         *(uint32_t *)(w + 0x1c0) = (uint32_t)(uintptr_t)(w + 0x200);
                                         *(uint32_t *)(w + 0x20c) = arena<uint32_t>(0x100)[r.range(0, 11)];
                                     }
                                 }
                             },
                             [](void *fn) { ((void (*)())fn)(); return Result{}; }};
    t.push_back(toolbar_next_79cb60);
    // List::Pass42D400: 0-4 boxes to move among up to 10 entries (some not in use), moves of -40 to 40
    Test list_pass_42d400{0x42d400,
                          [=](Rng &r) {
                              uint8_t *self = arena<uint8_t>(0x100);
                              *(uint32_t *)self = (uint32_t)(uintptr_t)arena<void>(0x400);
                              int in_use = 0;
                              for (int i = 0; i < 12; i++) {
                                  uint8_t *e = arena<uint8_t>(0x400 + 0x10 * i);
                                  for (int k = 0; k < 16; k++) e[k] = (uint8_t)r.next();
                                  e[0] = (uint8_t)(r.range(0, 2) ? r.range(1, 255) : 0);
                                  if (e[0]) in_use++;
                              }
                              arena<uint8_t>(0x4b0)[0] = 1;
                              *(int32_t *)(self + 8) = r.range(-1, in_use < 4 ? in_use : 4);
                              arena<int32_t>(0x10)[0] = r.range(-40, 40), arena<int32_t>(0x10)[1] = r.range(-40, 40);
                          },
                          [](void *fn) {
                              ((void(__thiscall *)(void *, int32_t, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<int32_t>(0x10)[1]);
                              return Result{};
                          }};
    quick(list_pass_42d400);
    // Ride::Part4FE690: modes 0-3, flag or not, values near each other
    Test ride_part_4fe690{0x4fe690,
                          [=](Rng &r) {
                              for (int i = 0; i < 0x60; i++) arena<uint32_t>(0x100)[i] = r.next();
                              uint8_t *self = arena<uint8_t>(0x100);
                              *(int32_t *)(self + 0x100) = r.range(0, 3);
                              *(int32_t *)(self + 0xd8) = r.range(-2, 3), *(int32_t *)(self + 0xd4) = r.range(-2, 3);
                              arena<uint32_t>(0x10)[0] = r.range(0, 1) * r.range(1, 255) | (r.next() & 0xffffff00u);
                              arena<int32_t>(0x10)[1] = r.range(0, 9) ? r.range(-3, 5) : (int32_t)r.next(), arena<int32_t>(0x10)[2] = r.range(-3, 5);
                          },
                          [](void *fn) {
                              ((void(__thiscall *)(void *, uint32_t, int32_t, int32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], arena<int32_t>(0x10)[1], arena<int32_t>(0x10)[2]);
                              return Result{};
                          }};
    quick(ride_part_4fe690);
    // Water::Step973040: keys equal up to a random part, then differing there (each way), flags with high bits
    Test water_key_compare{0x973040,
                           [=](Rng &r) {
                               uint32_t *x = arena<uint32_t>(0x100), *y = arena<uint32_t>(0x120);
                               for (int i = 0; i < 8; i++) x[i] = y[i] = r.next();
                               x[2] = (uint32_t)(uintptr_t)arena<void>(0x200), y[2] = (uint32_t)(uintptr_t)arena<void>(0x210);
                               for (int i = 0; i < 3; i++) arena<uint32_t>(0x200)[i] = arena<uint32_t>(0x210)[i] = r.next();
                               x[4] = r.range(0, 1) * 0x10 | (r.next() & 0xffffff00u), y[4] = (x[4] & 0xff) | (r.next() & 0xffffff00u);
                               x[7] = r.range(0, 1) * 0x10 | (r.next() & 0xffffff00u), y[7] = (x[7] & 0xff) | (r.next() & 0xffffff00u);
                               const int part = r.range(0, 10);
                               uint32_t *slot = part < 2 ? x + part : part < 5 ? arena<uint32_t>(0x200) + (part - 2) : part == 5 ? x + 3 : part == 6 ? x + 4 : part < 9 ? x + part - 2 : part == 9 ? x + 7 : nullptr;
                               if (slot) {
                                   if (part == 6 || part == 9) *slot ^= 0x10;
                                   else *slot += r.range(0, 1) ? 1 : 0x80000000u;
                               }
                           },
                           [](void *fn) {
                               uint32_t *x = arena<uint32_t>(0x100);
                               uint32_t eax;
                               __asm__ volatile("movl $16, %%ecx\n1:\n\tpushl 60(%%esi)\n\tsubl $4, %%esi\n\tdecl %%ecx\n\tjnz 1b\n\tcall *%%ebx"
                                                : "=a"(eax), "+S"(x)
                                                : "b"(fn)
                                                : "ecx", "edx", "edi", "memory", "cc");
                               return Result{eax};
                           }};
    quick(water_key_compare);
    // Water::Cell93DBF0: cells inside and outside a grid of up to 4 by 4, halves with the same object or not, flags 0-7
    Test water_cell_93dbf0{0x93dbf0,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               self[0x58] = (uint8_t)r.range(1, 4), self[0x59] = (uint8_t)r.range(1, 4);
                               *(uint32_t *)(self + 0x5c) = (uint32_t)(uintptr_t)arena<void>(0x400);
                               for (int i = 0; i < 16; i++) {
                                   uint32_t *a = arena<uint32_t>(0x400 + 0x30 * i), *b = a + 6;
                                   const uint32_t obj = (uint32_t)(uintptr_t)arena<void>(0x1000 + 0x40 * r.range(0, 1));
                                   a[1] = r.range(0, 5) ? obj : 0, b[1] = r.range(0, 3) ? a[1] : r.range(0, 1) ? 0 : (uint32_t)(uintptr_t)arena<void>(0x1080);
                                   a[5] = r.range(0, 2) ? 7 | (r.next() & ~7u) : r.next(), b[5] = r.range(0, 2) ? 7 | (r.next() & ~7u) : r.next();
                               }
                               for (int i = 0; i < 0x30; i++) arena<uint32_t>(0x1000)[i] = r.next();
                               arena<int32_t>(0x10)[0] = r.range(-1, 4), arena<int32_t>(0x10)[1] = r.range(-1, 4);
                               arena<uint32_t>(0x20)[0] = r.next(), arena<uint32_t>(0x24)[0] = r.next();
                           },
                           [](void *fn) {
                               return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, int32_t, int32_t, void *, void *))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<int32_t>(0x10)[1], arena<void>(0x20), arena<void>(0x24))};
                           }};
    quick(water_cell_93dbf0);
    // Shader::ScanCC4C50: 0-40 bytes from one, two or all three ranges (and zeros)
    Test shader_scan_cc4c50{0xcc4c50,
                            [=](Rng &r) {
                                const int which = r.range(0, 7);
                                for (int i = 0; i < 0x40; i++) {
                                    const int k = r.range(0, 3);
                                    arena<uint8_t>(0x400)[i] = (uint8_t)(k == 3 || !(which & (1 << k)) ? 0 : k == 0 ? r.range(1, 0x55) : k == 1 ? r.range(0x56, 0xaa) : r.range(0xab, 0xff));
                                }
                                arena<uint32_t>(0x10)[0] = r.range(0, 8), arena<uint32_t>(0x10)[1] = r.range(0, 5);
                            },
                            [](void *fn) { return Result{((uint32_t(__fastcall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x400), arena<uint32_t>(0x10)[0], arena<uint32_t>(0x10)[1])}; }};
    quick(shader_scan_cc4c50);
    // Stream::StepC52120: ranges before, across, inside and past what the stream holds
    Test stream_step_c52120{0xc52120,
                            [=](Rng &r) {
                                for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next(), arena<uint32_t>(0x200)[i] = r.next();
                                uint32_t *self = arena<uint32_t>(0x100);
                                self[4] = r.range(0, 0x40), self[5] = self[4] + r.range(0, 0x40), self[6] = r.range(0, 0x100);
                                arena<int32_t>(0x20)[0] = r.range(-0x20, 0x100), arena<int32_t>(0x20)[1] = r.range(-0x20, 0x120);
                            },
                            [](void *fn) {
                                const uint32_t out = (uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x200), arena<void>(0x20));
                                for (int i = 1; i < 4; i++) arena<uint8_t>(0x200)[i] = 0;   // the stack bytes the game copies
                                return Result{out};
                            }};
    quick(stream_step_c52120);
    // Animal::EntryA6FAF0: 0-6 triangles over points 0-4, edges looked for from any triangle on, either way round
    Test animal_entry_a6faf0{0xa6faf0,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100);
                                 *(int32_t *)(self + 0x4c) = r.range(0, 20), *(uint32_t *)(self + 0x48) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                 for (int i = 0; i < 24; i++) arena<uint32_t>(0x400)[i] = r.range(0, 4);
                                 arena<uint32_t>(0x10)[0] = r.range(0, 4), arena<uint32_t>(0x10)[1] = r.range(0, 4), arena<int32_t>(0x10)[2] = r.range(-1, 6);
                             },
                             [](void *fn) {
                                 return Result{(uint32_t)((int32_t(__thiscall *)(void *, uint32_t, uint32_t, int32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0], arena<uint32_t>(0x10)[1], arena<int32_t>(0x10)[2])};
                             }};
    quick(animal_entry_a6faf0);
    // Key::MatchesPattern: each part of the pattern the wildcard, the key's own, or one value off; byte runs of 0-5 equal
    // or one byte off
    Test key_matches_pattern{0xbb2220,
                             [=](Rng &r) {
                                 uint8_t *key = arena<uint8_t>(0x100), *pat = arena<uint8_t>(0x200);
                                 for (int i = 0; i < 0x14; i++) arena<uint32_t>(0x100)[i] = r.next();
                                 for (uint32_t at = 0; at <= 0x2c; at += at ? 0x1c : 0x10) {
                                     const int how = r.range(0, 3);
                                     for (int i = 0; i < 16; i++) pat[at + i] = how == 0 ? ((const uint8_t *)0xf9ebb4)[i] : key[at + i];
                                     if (how == 3) pat[at + r.range(0, 15)] ^= 1;
                                 }
                                 const uint32_t n = r.range(0, 5);
                                 *(uint32_t *)(key + 0x40) = n, *(uint32_t *)(pat + 0x40) = r.range(0, 4) ? n : r.range(0, 5);
                                 *(uint32_t *)(key + 0x44) = (uint32_t)(uintptr_t)arena<void>(0x300), *(uint32_t *)(pat + 0x44) = (uint32_t)(uintptr_t)arena<void>(0x310);
                                 for (int i = 0; i < 8; i++) arena<uint8_t>(0x300)[i] = arena<uint8_t>(0x310)[i] = (uint8_t)r.next();
                                 if (r.range(0, 2) == 0) arena<uint8_t>(0x310)[r.range(0, 5)] ^= 1;
                             },
                             [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200))}; }};
    quick(key_matches_pattern);
    // GroupedList::MoveAcross: an object once or twice in a list of 0-10 among six groups (starts and ends at random),
    // in or out
    Test grouped_move_across{0xbf4910,
                             [=](Rng &r) {
                                 uint8_t *owner = arena<uint8_t>(0x200);
                                 *(uint32_t *)arena<uint8_t>(0x104) = (uint32_t)(uintptr_t)owner;
                                 const int n = r.range(0, 10);
                                 *(int32_t *)(owner + 0x8c) = n, *(uint32_t *)(owner + 0x94) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                 for (int i = 0; i < 12; i++) arena<uint32_t>(0x400)[i] = r.range(0, 3) ? r.next() : (uint32_t)(uintptr_t)arena<void>(0x100);
                                 int32_t edge = 0;
                                 for (int g = 0; g < 6; g++) {
                                     const int32_t s = edge + r.range(0, 2), e = s + r.range(0, 3);
                                     *(int32_t *)(owner + 0x4c + 12 * g) = s, *(int32_t *)(owner + 0x50 + 12 * g) = e, edge = e;
                                 }
                                 arena<uint32_t>(0x10)[0] = r.range(0, 1) * r.range(1, 255) | (r.next() & 0xffffff00u);
                             },
                             [](void *fn) { ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0]); return Result{}; }};
    quick(grouped_move_across);
    // Resource::BinarySearch: nine sorted ranges with 0-4 sorted parts each (gaps between), places inside, between and
    // outside, parts with and without a base
    Test resource_binary_search{0xc3b130,
                                [=](Rng &r) {
                                    *(uint32_t *)arena<uint8_t>(0x16c) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                    uint32_t at = r.range(0, 0x20);
                                    for (int k = 0; k < 9; k++) {
                                        uint32_t *e = arena<uint32_t>(0x400 + 0x18 * k);
                                        const uint32_t start = at, size = r.range(1, 0x40);
                                        e[4] = start, e[5] = size, at = start + size + r.range(0, 0x10);
                                        const uint32_t parts = r.range(0, 4);
                                        e[0] = parts, e[1] = r.next(), e[2] = r.next(), e[3] = (uint32_t)(uintptr_t)arena<void>(0x800 + 0x40 * k);
                                        uint32_t p = start;
                                        for (uint32_t j = 0; j < 4; j++) {
                                            uint32_t *f = arena<uint32_t>(0x800 + 0x40 * k + 0x10 * j);
                                            p += r.range(0, 4);
                                            f[1] = p, f[0] = r.range(1, 8), f[2] = r.range(0, 4) ? r.next() : 0, f[3] = r.next();
                                            p += f[0];
                                        }
                                    }
                                    arena<uint32_t>(0x10)[0] = r.range(0, (int)at + 0x10);
                                },
                                [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0])}; }};
    quick(resource_binary_search);
    // SortedTable::Find: 0-12 sorted entries over small values, keys that are there or fall between
    Test sorted_table_find{0xe03cd0,
                           [=](Rng &r) {
                               const int n = r.range(0, 12);
                               *(volatile int32_t *)0x141dcdc = n, *(volatile uint32_t *)0x141dce4 = (uint32_t)(uintptr_t)arena<void>(0x400);
                               uint32_t k[6] = {0, 0, (uint32_t)-1, (uint32_t)-1, (uint32_t)-1, (uint32_t)-1};
                               for (int i = 0; i < 12; i++) {
                                   const int part = r.range(0, 5);
                                   k[part] += r.range(1, 2);
                                   for (int j = part + 1; j < 6; j++) k[j] = j < 2 ? r.range(0, 2) : (uint32_t)r.range(-2, 2);
                                   for (int j = 0; j < 6; j++) arena<uint32_t>(0x400 + 32 * i)[j] = k[j];
                                   arena<uint32_t>(0x400 + 32 * i)[6] = r.next(), arena<uint32_t>(0x400 + 32 * i)[7] = r.next();
                               }
                               uint32_t *key = arena<uint32_t>(0x20);
                               if (n && r.range(0, 1)) {
                                   for (int j = 0; j < 6; j++) key[j] = arena<uint32_t>(0x400 + 32 * r.range(0, n - 1))[j];
                               } else {
                                   for (int j = 0; j < 6; j++) key[j] = j < 2 ? r.range(0, 12) : (uint32_t)r.range(-3, 3);
                               }
                               arena<uint32_t>(0x40)[0] = r.next();
                           },
                           [](void *fn) {
                               const uint32_t *k = arena<uint32_t>(0x20);
                               return Result{((uint8_t(__fastcall *)(uint32_t, uint32_t, const void *, int32_t, void *))fn)(k[0], k[1], k + 2, (int32_t)k[5], arena<void>(0x40))};
                           }};
    t.push_back(sorted_table_find);
    // Entries::UpdateMatching: 0-5 entries (a negative count too), keys matching the record's or not
    Test entries_update_matching{0xb786b0,
                                 [=](Rng &r) {
                                     for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x100)[i] = r.next();
                                     for (int i = 0; i < 0xc0; i++) arena<uint32_t>(0x400)[i] = r.next();
                                     arena<uint32_t>(0x100)[3] = r.range(0, 1), arena<uint32_t>(0x100)[4] = r.range(0, 1);
                                     for (int i = 0; i < 6; i++) arena<uint32_t>(0x40c + 0x70 * i)[0] = r.range(0, 1), arena<uint32_t>(0x410 + 0x70 * i)[0] = r.range(0, 1);
                                     arena<uint32_t>(0x200)[0] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<int32_t>(0x200)[2] = r.range(-1, 5);
                                 },
                                 [](void *fn) { return Result{(uint32_t)((int32_t(__stdcall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200))}; }};
    quick(entries_update_matching);
    // Span::Clip: ranges before, across, inside and past this one
    Test span_clip{0xc52060,
                   [=](Rng &r) {
                       for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next(), arena<uint32_t>(0x200)[i] = r.next();
                       uint32_t *self = arena<uint32_t>(0x100);
                       self[4] = r.range(0, 0x40), self[5] = self[4] + r.range(0, 0x40), self[3] = r.range(0, 0x100);
                       arena<int32_t>(0x20)[0] = r.range(-0x20, 0x100), arena<int32_t>(0x20)[1] = r.range(-0x20, 0x120);
                   },
                   [](void *fn) {
                       const uint32_t out = (uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x200), arena<void>(0x20));
                       for (int i = 1; i < 4; i++) arena<uint8_t>(0x200)[i] = 0;   // the stack bytes the game copies
                       return Result{out};
                   }};
    quick(span_clip);
    // PairIds::Get: pairs of 0-3 either way round, rows holding 0-5 pairs already (the pair among them or not)
    Test pair_ids_get{0x466970,
                      [=](Rng &r) {
                          for (int row = 0; row < 4; row++) {
                              volatile int32_t *p = (volatile int32_t *)(0x111efe0 + 0x14c * row);
                              p[0] = r.range(0, 5);
                              for (int i = 0; i < 40; i++) p[1 + i] = r.range(0, 6), p[0x29 + i] = (int32_t)r.next();
                          }
                          *(volatile int32_t *)0x12dffe8 = r.range(0, 20);
                          for (int i = 0; i < 0x60; i++) ((volatile uint32_t *)0x1106e90)[i] = r.next();
                          arena<int32_t>(0x10)[0] = r.range(0, 3), arena<int32_t>(0x10)[1] = r.range(0, 3);
                      },
                      [](void *fn) { return Result{((uint32_t(__fastcall *)(int32_t, int32_t))fn)(arena<int32_t>(0x10)[0], arena<int32_t>(0x10)[1])}; }};
    t.push_back(pair_ids_get);
    // Grid::QueueCell: cells inside and outside a 3 by 3 grid, 0-4 objects flagged or not, linked, half-linked or loose
    Test grid_queue_cell{0xba30b0,
                         [=](Rng &r) {
                             uint8_t *self = arena<uint8_t>(0x100);
                             *(int32_t *)(self + 0x64) = 3, *(int32_t *)(self + 0x68) = 3;
                             *(uint32_t *)(self + 0x6c) = (uint32_t)(uintptr_t)arena<void>(0x200);
                             for (int c = 0; c < 9; c++) {
                                 uint32_t *cell = arena<uint32_t>(0x200 + 0x18 * c);
                                 cell[0] = r.range(0, 4) ? r.range(0, 3) : (uint32_t)-1;
                                 cell[3] = (uint32_t)(uintptr_t)arena<void>(0x400), cell[4] = r.range(0, 4);
                             }
                             for (int i = 0; i < 4; i++) {
                                 uint8_t *obj = arena<uint8_t>(0x800 + 0x100 * i);
                                 arena<uint32_t>(0x400)[i] = (uint32_t)(uintptr_t)obj;
                                 obj[0x60] = (uint8_t)(r.range(0, 2) ? r.range(1, 255) : 0);
                                 *(uint32_t *)(obj + 0x58) = r.next();
                                 uint32_t *link = (uint32_t *)(obj + 0x4c), *a = arena<uint32_t>(0xd00 + 0x10 * i), *b = arena<uint32_t>(0xd80 + 0x10 * i);
                                 link[0] = r.range(0, 5) ? (uint32_t)(uintptr_t)a : 0, link[1] = r.range(0, 5) ? (uint32_t)(uintptr_t)b : 0;
                                 a[1] = r.range(0, 3) ? (uint32_t)(uintptr_t)link : 0, b[0] = r.range(0, 3) ? (uint32_t)(uintptr_t)link : 0;
                             }
                             *(uint32_t *)(self + 0x8c) = (uint32_t)(uintptr_t)arena<void>(0xe00);
                             arena<uint32_t>(0xe00)[0] = arena<uint32_t>(0xe00)[1] = (uint32_t)(uintptr_t)arena<void>(0xe00);
                             arena<int32_t>(0x10)[0] = r.range(-1, 3), arena<int32_t>(0x10)[1] = r.range(-1, 3);
                         },
                         [](void *fn) {
                             ((void(__thiscall *)(void *, int32_t, int32_t, uint32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<int32_t>(0x10)[1], 0);
                             return Result{};
                         }};
    quick(grid_queue_cell);
    // Grid::MarkRectForOwner: 0-3 layers of two owners, rectangles inside a 6-wide grid of cells
    Test grid_mark_rect{0x71cb80,
                        [=](Rng &r) {
                            uint8_t *self = arena<uint8_t>(0x100);
                            *(int32_t *)(self + 0x70) = r.range(-1, 3), *(int32_t *)(self + 0x60) = 6;
                            *(uint32_t *)(self + 0x68) = (uint32_t)(uintptr_t)arena<void>(0x200);
                            for (int i = 0; i < 3; i++) {
                                uint32_t *layer = arena<uint32_t>(0x200 + 32 * i);
                                layer[1] = (uint32_t)(uintptr_t)arena<void>(0x300 + 0x10 * i), arena<uint32_t>(0x30c + 0x10 * i)[0] = r.range(1, 2);
                                layer[5] = (uint32_t)(uintptr_t)arena<void>(0x1000 + 0x1000 * i);
                            }
                            for (int i = 0; i < 0xc00; i++) arena<uint32_t>(0x1000)[i] = r.next();
                            arena<uint32_t>(0x10)[0] = r.range(1, 2);
                            for (int k = 1; k < 5; k++) arena<uint32_t>(0x10)[k] = (uint32_t)r.range(0, 5) << 4 | r.range(0, 15) | (r.next() & 0xffffff00u);
                        },
                        [](void *fn) {
                            const uint32_t *a = arena<uint32_t>(0x10);
                            ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), a[0], a[1], a[2], a[3], a[4]);
                            return Result{};
                        }};
    quick(grid_mark_rect);
    // Ride::Clamp4944D0: grids of 0-6 by 0-6, rectangles partly or wholly outside, reversed
    Test ride_clamp_4944d0{0x4944d0,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               *(int32_t *)(self + 8) = r.range(0, 6), *(int32_t *)(self + 0xc) = r.range(0, 6);
                               *(uint32_t *)(self + 0x10) = (uint32_t)(uintptr_t)arena<void>(0x400);
                               for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x400)[i] = r.next();
                               for (int k = 0; k < 4; k++) arena<int32_t>(0x10)[k] = r.range(-3, 8);
                           },
                           [](void *fn) {
                               const int32_t *a = arena<int32_t>(0x10);
                               ((void(__thiscall *)(void *, int32_t, int32_t, int32_t, int32_t))fn)(arena<void>(0x100), a[0], a[1], a[2], a[3]);
                               return Result{};
                           }};
    quick(ride_clamp_4944d0);
    // Mask::ToPixels: rows 0-5 (or none), columns within 0-40, bits at random, image strides 40-48
    Test mask_to_pixels{0x7aeb50,
                        [=](Rng &r) {
                            uint8_t *self = arena<uint8_t>(0x100);
                            self[0x14] = (uint8_t)r.range(0, 4), self[0x15] = (uint8_t)r.range(0, 5);
                            for (int row = 0; row < 6; row++) {
                                self[0x16 + row] = (uint8_t)r.range(0, 40), self[0x116 + row] = (uint8_t)r.range(0, 40);
                                for (int k = 0; k < 8; k++) *(uint32_t *)(self + 0x228 + 32 * row + 4 * k) = r.next();
                            }
                            arena<int32_t>(0x10)[0] = r.range(40, 48);
                            for (int i = 0; i < 0x200; i++) arena<uint32_t>(0x1000)[i] = r.next();
                        },
                        [](void *fn) { ((void(__thiscall *)(void *, void *, int32_t))fn)(arena<void>(0x100), arena<void>(0x1000), arena<int32_t>(0x10)[0]); return Result{}; }};
    quick(mask_to_pixels);
    // Description::Differs: kinds 0-3, missing first, each compared value the same or not
    Test description_differs{0x970c80,
                             [=](Rng &r) {
                                 uint32_t *a = arena<uint32_t>(0x100), *b = arena<uint32_t>(0x200);
                                 for (int i = 0; i < 0x40; i++) a[i] = b[i] = r.next();
                                 a[0] = b[0] = r.range(0, 3);
                                 static const uint32_t at[] = {0, 0xb8, 0xbc, 0xc0, 8, 0xc, 0x54, 0x60};
                                 if (r.range(0, 1)) ((uint8_t *)b)[at[r.range(0, 7)]] ^= 1;
                                 arena<uint32_t>(0x10)[0] = r.range(0, 7) ? (uint32_t)(uintptr_t)a : 0;
                             },
                             [](void *fn) { return Result{((uint8_t(__stdcall *)(void *, void *))fn)((void *)(uintptr_t)arena<uint32_t>(0x10)[0], arena<void>(0x200))}; }};
    quick(description_differs);
    // Text::Find* (ten copies): sides of the copy's kinds (0 text of bytes, 1 text of 16-bit, 2 bytes to a 0, 3 16-bit to
    // a 0), from a few letters in either case (and high values), 0-4 long, places 0-5, none at times
    auto text_places_test = [&](uintptr_t at, int ka, int kb) {
        Test x{at,
               [=](Rng &r) {
                   static const uint16_t letters[] = {'a', 'A', 'b', 'B', 'z', 0x80, 0xc1, 0x141};
                   const int kinds[2] = {ka, kb};
                   for (int s = 0; s < 2; s++) {
                       uint32_t *it = arena<uint32_t>(0x100 + 0x10 * s);
                       uint8_t *text = arena<uint8_t>(0x200 + 0x40 * s), *chars = arena<uint8_t>(0x300 + 0x40 * s);
                       const int len = r.range(0, 4);
                       // the two sides share their first letters often, so runs of equal letters happen
                       for (int i = 0; i < 8; i++) {
                           const uint16_t c = i < len ? letters[r.range(0, 7)] : 0;
                           if (kinds[s] == 0 || kinds[s] == 2) chars[i] = (uint8_t)c;
                           else ((uint16_t *)chars)[i] = c;
                       }
                       if (s == 1 && r.range(0, 1)) memcpy(chars, arena<uint8_t>(0x300), 16);
                       if (kinds[s] < 2) {
                           *(uint32_t *)(text + 4) = len, *(uint32_t *)(text + 0xc) = (uint32_t)(uintptr_t)chars;
                           arena<uint32_t>(0x280 + 4 * s)[0] = (uint32_t)(uintptr_t)text;
                           it[0] = r.range(0, 7) ? (uint32_t)(uintptr_t)arena<void>(0x280 + 4 * s) : 0, it[1] = r.range(0, 5);
                       } else {
                           it[0] = r.range(0, 7) ? (uint32_t)(uintptr_t)chars : 0, it[1] = r.next();
                       }
                   }
               },
               [](void *fn) { return Result{(uint32_t)((int32_t(__fastcall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x110))}; }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xdde005, (void *)h2_lower}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    };
    text_places_test(0xcd3700, 0, 0), text_places_test(0xcd3800, 0, 1), text_places_test(0xcd3ab0, 1, 0), text_places_test(0xcd3bb0, 1, 1);
    text_places_test(0xcd3070, 2, 0), text_places_test(0xcd3150, 2, 1), text_places_test(0xcd3390, 3, 0), text_places_test(0xcd3470, 3, 1);
    text_places_test(0xcd3620, 0, 3), text_places_test(0xcd39d0, 1, 3);
    // Text::CompareCD5150 / CD5260 / CD5640 / CD5370: the same sides, given as the holder (or characters) itself
    auto text_whole_test = [&](uintptr_t at, int ka, int kb) {
        text_places_test(at, ka, kb);
        t.back().call = [](void *fn) {
            return Result{(uint32_t)((int32_t(__stdcall *)(uint32_t, uint32_t))fn)(arena<uint32_t>(0x100)[0], arena<uint32_t>(0x110)[0])};
        };
    };
    text_whole_test(0xcd5150, 0, 0), text_whole_test(0xcd5260, 0, 1), text_whole_test(0xcd5640, 1, 1), text_whole_test(0xcd5370, 1, 2);
    // Water::Range93DCD0: grids of up to 6 by 6 cells (few or many fully watered), points inside and out, reach -1 to 5
    Test water_range_93dcd0{0x93dcd0,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                self[0x58] = (uint8_t)r.range(1, 6), self[0x59] = (uint8_t)r.range(1, 6);
                                *(uint32_t *)(self + 0x5c) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                const int density = r.range(0, 9);
                                for (int i = 0; i < 36; i++) {
                                    uint32_t *a = arena<uint32_t>(0x400 + 0x30 * i), *b = a + 6;
                                    const bool full = r.range(0, 9) < density;
                                    a[1] = b[1] = (uint32_t)(uintptr_t)arena<void>(0x1000);
                                    if (!full && r.range(0, 1)) b[1] = 0;
                                    a[5] = full || r.range(0, 1) ? 7 : r.range(0, 6), b[5] = full ? 7 : r.range(0, 7);
                                }
                                arena<int32_t>(0x10)[0] = r.range(-2, 7), arena<int32_t>(0x14)[0] = r.range(-2, 7), arena<int32_t>(0x18)[0] = r.range(-1, 5);
                            },
                            [](void *fn) {
                                return Result{((uint8_t(__thiscall *)(void *, void *, void *, int32_t))fn)(arena<void>(0x100), arena<void>(0x10), arena<void>(0x14), arena<int32_t>(0x18)[0])};
                            }};
    quick(water_range_93dcd0);
    // Water::Find90D380: 0-6 sorted entries with 0-3 records and indexes each, laid out one after another (with gaps at
    // times), the key there or not
    Test water_find_90d380{0x90d380,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               const int n = r.range(0, 6);
                               *(uint32_t *)(self + 0x78) = (uint32_t)(uintptr_t)arena<void>(0x200), *(int32_t *)(self + 0x80) = n;
                               *(uint32_t *)(self + 0x84) = (uint32_t)(uintptr_t)arena<void>(0x400), *(uint32_t *)(self + 0x90) = (uint32_t)(uintptr_t)arena<void>(0xa00);
                               *(uint32_t *)(self + 0x8c) = r.next(), *(uint32_t *)(self + 0x98) = r.next();
                               uint32_t key = r.range(0, 3), rec = r.range(0, 2), idx = r.range(0, 2);
                               for (int i = 0; i < 8; i++) {
                                   uint32_t *e = arena<uint32_t>(0x200 + 20 * i);
                                   e[0] = key, key += r.range(1, 3);
                                   e[1] = rec, e[2] = r.range(0, 3), rec += e[2] + (r.range(0, 4) ? 0 : 1);
                                   e[3] = idx, e[4] = r.range(0, 3), idx += e[4] + (r.range(0, 4) ? 0 : 1);
                               }
                               for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x400)[i] = r.next();
                               for (int i = 0; i < 0x40; i++) arena<uint16_t>(0xa00)[i] = (uint16_t)r.next();
                               arena<uint32_t>(0x10)[0] = r.range(0, 20);
                           },
                           [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x10)[0])}; }};
    quick(water_find_90d380);
    // Animal::AverageAE0890: 0-6 points, values either side of 0, equal ones, and a quiet NaN at times
    Test animal_bounds{0xae0890,
                       [=](Rng &r) {
                           uint8_t *self = arena<uint8_t>(0x100);
                           *(int32_t *)(self + 0x50) = r.range(-1, 6), *(uint32_t *)(self + 0x40) = (uint32_t)(uintptr_t)arena<void>(0x400);
                           for (int i = 0; i < 12; i++) {
                               float v = r.range(0, 4) ? (float)r.range(-3, 3) : r.real(-20, 20);
                               if (r.range(0, 15) == 0) { const uint32_t q = 0x7fc00000u; memcpy(&v, &q, 4); }
                               arena<float>(0x400)[i] = v;
                           }
                           for (int i = 0; i < 4; i++) arena<uint32_t>(0x200)[i] = r.next();
                       },
                       [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200)); return Result{}; }};
    quick(animal_bounds);
    // PhysicsWorld::Integrate: 0-3 particles (one listed twice at times) with values from geo_pick (no signalling NaNs)
    Test physics_integrate{0xa6c3a0,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               const int n = r.range(0, 3);
                               *(int32_t *)(self + 0x48) = n, *(uint32_t *)(self + 0x40) = (uint32_t)(uintptr_t)arena<void>(0x200);
                               for (int i = 0; i < 3; i++) {
                                   arena<uint32_t>(0x200)[i] = (uint32_t)(uintptr_t)arena<void>(0x400 + 0x80 * (r.range(0, 5) ? i : 0));
                                   for (int k = 0; k < 0x14; k++) {
                                       float v = k == 12 ? r.real(0, 1) : r.range(0, 9) ? r.real(-20, 20) : (float)r.range(-1, 1);
                                       arena<float>(0x400 + 0x80 * i)[k] = v;
                                   }
                               }
                               *(volatile float *)0x13429f0 = r.range(0, 3) ? 0.001f * r.range(1, 40) : r.real(-2, 2);
                           },
                           [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    t.push_back(physics_integrate);
    // Cell::Quad: cells 0-3 by 0-3 of a 4-wide terrain, sizes and origins from geo_pick
    Test cell_quad{0x729530,
                   [=](Rng &r) {
                       uint8_t *cell = arena<uint8_t>(0x100), *terrain = arena<uint8_t>(0x200);
                       cell[0x88] = (uint8_t)r.range(0, 3), cell[0x89] = (uint8_t)r.range(0, 3);
                       terrain[4] = 4, *(uint32_t *)(terrain + 0x24) = (uint32_t)(uintptr_t)arena<void>(0x400);
                       for (int i = 0; i < 4; i++) *(float *)(terrain + 0xc + 4 * i) = r.range(0, 3) ? r.real(-50, 50) : geo_pick(r);
                       for (int i = 0; i < 0x60; i++) arena<uint32_t>(0x400)[i] = r.next();
                       for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x800)[i] = r.next();
                   },
                   [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x800), arena<void>(0x200))}; }};
    quick(cell_quad);
    // Model::BlendCCF530: 1-4 colours, fractions in and out of 0-1, a matrix from geo_pick
    Test model_blend_ccf530{0xccf530,
                            [=](Rng &r) {
                                arena<int32_t>(0x10)[0] = r.range(1, 4), arena<float>(0x10)[1] = r.range(0, 3) ? r.real(0, 1) : geo_pick(r);
                                for (int i = 0; i < 12; i++) arena<float>(0x100)[i] = r.range(0, 4) ? r.real(-2, 2) : geo_pick(r);
                                for (int i = 0; i < 12; i++) arena<uint8_t>(0x200)[i] = (uint8_t)r.next(), arena<uint8_t>(0x220)[i] = (uint8_t)r.next();
                                for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x400)[i] = r.next();
                            },
                            [](void *fn) {
                                return Result{((uint32_t(__fastcall *)(int32_t, void *, float, void *, void *, void *))fn)(arena<int32_t>(0x10)[0], arena<void>(0x100), arena<float>(0x10)[1], arena<void>(0x200), arena<void>(0x220), arena<void>(0x400))};
                            }};
    quick(model_blend_ccf530);
    // Water::Segment925AC0: points before, beside and past segments (some of no length), fractions and -0 at times
    Test water_segment{0x925ac0,
                       [=](Rng &r) {
                           float *a = arena<float>(0x10);
                           for (int i = 0; i < 6; i++) a[i] = r.range(0, 5) ? (float)r.range(-4, 4) + 0.25f * r.range(0, 3) : r.range(0, 3) ? r.real(-20, 20) : -0.0f;
                           if (r.range(0, 6) == 0) a[4] = a[2], a[5] = a[3];
                           arena<uint32_t>(0x40)[0] = r.next(), arena<uint32_t>(0x40)[1] = r.next();
                       },
                       [](void *fn) {
                           const float *a = arena<float>(0x10);
                           Result x;
                           x.f = ((long double(__stdcall *)(float, float, float, float, float, float, void *))fn)(a[0], a[1], a[2], a[3], a[4], a[5], arena<void>(0x40));
                           return x;
                       }};
    quick(water_segment);
    // WildAnimal::NearB1CED0: points within and beyond radii 0-6, directions of any length (and none)
    Test wild_animal_near{0xb1ced0,
                          [=](Rng &r) {
                              for (int i = 0; i < 0x30; i++) arena<uint32_t>(0x100)[i] = r.next();
                              *(float *)arena<uint8_t>(0x180) = r.real(-8, 8), *(float *)arena<uint8_t>(0x188) = r.real(-8, 8);
                              for (int i = 0; i < 3; i++) arena<float>(0x200)[i] = r.real(-8, 8), arena<float>(0x210)[i] = r.range(0, 6) ? r.real(-3, 3) : 0.0f;
                              arena<float>(0x10)[0] = r.range(0, 5) ? r.real(0, 6) : (float)r.range(0, 1);
                              for (int i = 0; i < 3; i++) arena<uint32_t>(0x300)[i] = r.next();
                          },
                          [](void *fn) {
                              ((void(__stdcall *)(void *, void *, void *, float, void *))fn)(arena<void>(0x100), arena<void>(0x200), arena<void>(0x210), arena<float>(0x10)[0], arena<void>(0x300));
                              return Result{};
                          }};
    quick(wild_animal_near);
    // Ride::Last5BB3A0: last points of two lists (1-3 entries), opposite at times so the sum has no length
    Test ride_last_5bb3a0{0x5bb3a0,
                          [=](Rng &r) {
                              uint8_t *self = arena<uint8_t>(0x100);
                              *(uint32_t *)(self + 0x2c) = (uint32_t)(uintptr_t)arena<void>(0x200), *(uint32_t *)(self + 0x28) = (uint32_t)(uintptr_t)arena<void>(0x210);
                              arena<uint32_t>(0x200)[0] = r.range(1, 3), arena<uint32_t>(0x200)[1] = (uint32_t)(uintptr_t)arena<void>(0x400);
                              arena<uint32_t>(0x210)[0] = r.range(1, 3), arena<uint32_t>(0x210)[1] = (uint32_t)(uintptr_t)arena<void>(0x500);
                              for (int i = 0; i < 0x1b; i++) arena<float>(0x400)[i] = r.range(0, 3) ? r.real(-5, 5) : (float)r.range(-1, 1), arena<float>(0x500)[i] = r.real(-5, 5);
                              if (r.range(0, 3) == 0) {
                                  const float *e = (const float *)(uintptr_t)(arena<uint32_t>(0x200)[1] + 0x24 * arena<uint32_t>(0x200)[0] - 0x18);
                                  float *f = (float *)(uintptr_t)(arena<uint32_t>(0x210)[1] + 0x24 * arena<uint32_t>(0x210)[0] - 0x18);
                                  for (int k = 0; k < 3; k++) f[k] = -e[k];
                              }
                              for (int i = 0; i < 3; i++) arena<uint32_t>(0x300)[i] = r.next();
                          },
                          [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x300)); return Result{}; }};
    quick(ride_last_5bb3a0);
    // TrackPiece::Length5BB2B0 and TrackPiece::Last5BE8B0: the same lists, points opposite or equal at times
    auto track_points_test = [&](uintptr_t at, int same) {
        Test x{at,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   *(uint32_t *)(self + 0x2c) = (uint32_t)(uintptr_t)arena<void>(0x200), *(uint32_t *)(self + 0x28) = (uint32_t)(uintptr_t)arena<void>(0x210);
                   arena<uint32_t>(0x200)[0] = r.range(1, 3), arena<uint32_t>(0x200)[1] = (uint32_t)(uintptr_t)arena<void>(0x400);
                   arena<uint32_t>(0x210)[0] = r.range(1, 3), arena<uint32_t>(0x210)[1] = (uint32_t)(uintptr_t)arena<void>(0x500);
                   for (int i = 0; i < 0x1b; i++) arena<float>(0x400)[i] = r.range(0, 3) ? r.real(-5, 5) : (float)r.range(-1, 1), arena<float>(0x500)[i] = r.real(-5, 5);
                   if (r.range(0, 3) == 0)
                       for (int i = 0; i < 0x1b; i++) arena<float>(0x500)[i] = same ? arena<float>(0x400)[i] : -arena<float>(0x400)[i];
                   if (r.range(0, 3) == 0) arena<uint32_t>(0x210)[0] = arena<uint32_t>(0x200)[0];
                   for (int i = 0; i < 3; i++) arena<uint32_t>(0x300)[i] = r.next();
               },
               [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x300)); return Result{}; }};
        quick(x);
    };
    track_points_test(0x5bb2b0, 0), track_points_test(0x5be8b0, 1);
    // Ride::Blend55FA00: angles within and past a few turns, steps either way, periods near 1
    Test ride_blend_55fa00{0x55fa00,
                           [=](Rng &r) {
                               for (int i = 0; i < 0x120; i++) arena<uint32_t>(0x100)[i] = r.next();
                               uint8_t *self = arena<uint8_t>(0x100);
                               *(float *)(self + 0x70) = r.real(-3, 3);
                               for (int j = 0; j < 3; j++) *(float *)(self + 0x3d0 + 4 * j) = r.real(0.5f, 4), *(float *)(self + 0x3dc + 4 * j) = r.range(0, 4) ? r.real(-20, 20) : (float)r.range(-2, 2);
                               arena<float>(0x10)[0] = r.range(0, 4) ? r.real(-2, 2) : 0.0f;
                           },
                           [](void *fn) { ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x10)[0]); return Result{}; }};
    quick(ride_blend_55fa00);
    // Scenery::Points856900 and Water::Points97ACA0: 0-3 vertices moved by a random matrix, apart or in place
    auto vertices_setup = [=](Rng &r) {
        for (int i = 0; i < 16; i++) arena<float>(0x100)[i] = r.range(0, 4) ? r.real(-3, 3) : (float)r.range(-1, 1);
        for (int i = 0; i < 0x1b; i++) arena<float>(0x200)[i] = r.real(-10, 10), arena<uint32_t>(0x300)[i] = r.next();
        arena<int32_t>(0x10)[0] = r.range(0, 3), arena<uint32_t>(0x10)[1] = r.range(0, 3) ? 0x300 : 0x200, arena<uint32_t>(0x10)[2] = r.next();
    };
    Test scenery_points{0x856900, vertices_setup, [](void *fn) {
                            const uint32_t *a = arena<uint32_t>(0x10);
                            uint32_t n = a[0];
                            void *m = arena<void>(0x100), *to = arena<void>(a[1]), *from = arena<void>(0x200);
                            uint32_t args[4] = {a[2], (uint32_t)(uintptr_t)from, (uint32_t)(uintptr_t)to, (uint32_t)(uintptr_t)fn};
                            uint32_t *pa = args;
                            __asm__ volatile("pushl (%%esi)\n\tpushl 4(%%esi)\n\tpushl 8(%%esi)\n\tcall *12(%%esi)"
                                             : "+a"(n), "+c"(m), "+S"(pa)
                                             :
                                             : "edx", "memory", "cc");
                            return Result{};
                        }};
    quick(scenery_points);
    Test water_points{0x97aca0, vertices_setup, [](void *fn) {
                          const uint32_t *a = arena<uint32_t>(0x10);
                          uint32_t n = a[0];
                          void *m = arena<void>(0x100), *to = arena<void>(a[1]), *from = arena<void>(0x200);
                          uint32_t args[3] = {a[2], (uint32_t)(uintptr_t)to, (uint32_t)(uintptr_t)fn};
                          uint32_t *pa = args;
                          __asm__ volatile("pushl (%%esi)\n\tpushl 4(%%esi)\n\tcall *8(%%esi)"
                                           : "+a"(n), "+c"(m), "+d"(from), "+S"(pa)
                                           :
                                           : "memory", "cc");
                          return Result{};
                      }};
    quick(water_points);
    // Animal::CheckAE09F0: 0-5 points, lengths and t that land inside, on the end, past it and before the start
    Test animal_check_ae09f0{0xae09f0,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100);
                                 *(int32_t *)(self + 0x50) = r.range(0, 5), *(float *)(self + 0x54) = r.real(0.5f, 6);
                                 *(uint32_t *)(self + 0x40) = (uint32_t)(uintptr_t)arena<void>(0x400), *(uint32_t *)(self + 0x48) = (uint32_t)(uintptr_t)arena<void>(0x500);
                                 for (int i = 0; i < 12; i++) arena<float>(0x400)[i] = r.real(-10, 10), arena<uint8_t>(0x500)[i] = (uint8_t)r.next();
                                 arena<float>(0x10)[0] = r.range(0, 5) ? r.real(0, 6) : r.real(-1, 0);
                                 arena<uint32_t>(0x20)[0] = r.next(), arena<uint32_t>(0x20)[1] = r.next();
                             },
                             [](void *fn) { return Result{((uint8_t(__thiscall *)(void *, float, void *))fn)(arena<void>(0x100), arena<float>(0x10)[0], arena<void>(0x20))}; }};
    quick(animal_check_ae09f0);
    // Effects::Point498630: with or without a part, values whole, fractional, negative and huge
    Test effects_point_498630{0x498630,
                              [=](Rng &r) {
                                  uint8_t *self = arena<uint8_t>(0x100);
                                  *(uint32_t *)(self + 0x24) = r.range(0, 5) ? (uint32_t)(uintptr_t)arena<void>(0x200) : 0;
                                  for (int k = 0; k < 3; k++) {
                                      const int how = r.range(0, 5);
                                      const float v = how == 0 ? (float)r.range(-3, 3) : how == 1 ? r.real(-1e12f, 1e12f) : r.real(-20, 20);
                                      *(float *)(k == 0 ? self + 0x40 : arena<uint8_t>(0x200) + 4 * k) = v;
                                  }
                                  for (int i = 0; i < 5; i++) arena<uint32_t>(0x20)[i] = r.next();
                              },
                              [](void *fn) {
                                  return Result{((uint8_t(__thiscall *)(void *, void *, void *, void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x20), arena<void>(0x24), arena<void>(0x28), arena<void>(0x2c), arena<void>(0x30))};
                              }};
    quick(effects_point_498630);
    // Scenery::Round85D200: scales from just below 0 to past the table's end (whole steps at times), blends 0-1
    Test scenery_round_85d200{0x85d200,
                              [=](Rng &r) {
                                  arena<float>(0x10)[0] = r.range(0, 4) ? r.real(-0.2f, 1.2f) : (float)r.range(0, 7) / 6.0f;
                                  arena<float>(0x10)[1] = r.range(0, 4) ? r.real(0, 1) : (float)r.range(0, 1), arena<float>(0x10)[2] = r.real(0, 2);
                                  for (int i = 0; i < 4; i++) arena<uint32_t>(0x100)[i] = r.next();
                              },
                              [](void *fn) {
                                  const float *a = arena<float>(0x10);
                                  ((void(__thiscall *)(void *, float, float, float))fn)(arena<void>(0x100), a[0], a[1], a[2]);
                                  return Result{};
                              }};
    quick(scenery_round_85d200);
    // Grid::Round4414F0: angles and heights across several turns either way, with or without the reference
    Test grid_round_4414f0{0x4414f0,
                           [=](Rng &r) {
                               *(volatile uint32_t *)0x1106c54 = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x200) : 0;
                               *(float *)arena<uint8_t>(0x254) = r.real(-3000, 3000);
                               *(float *)arena<uint8_t>(0x148) = r.range(0, 4) ? r.real(-12000, 12000) : (float)(90 * r.range(-100, 100));
                               *(float *)arena<uint8_t>(0x154) = r.range(0, 4) ? r.real(-3000, 3000) : 0.0f;
                           },
                           [](void *fn) { return Result{(uint32_t)((int32_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
    t.push_back(grid_round_4414f0);
    // Scenery::Part862DF0: parts 0-3 of a count of 0-3, placed or not, a random matrix
    Test scenery_part_862df0{0x862df0,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100);
                                 for (int i = 0; i < 16; i++) arena<float>(0x160)[i] = r.range(0, 4) ? r.real(-3, 3) : (float)r.range(-1, 1);
                                 *(int32_t *)(self + 0x108) = r.range(0, 3), *(uint32_t *)(self + 0x104) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                 for (int i = 0; i < 4; i++) {
                                     uint8_t *e = arena<uint8_t>(0x400 + 0x48 * i);
                                     *(uint32_t *)e = (uint32_t)(uintptr_t)arena<void>(0x800 + 0x40 * i), e[4] = (uint8_t)(r.range(0, 2) ? 0 : r.range(1, 255));
                                     for (int k = 0; k < 3; k++) arena<float>(0x830 + 0x40 * i)[k] = r.real(-10, 10);
                                 }
                                 arena<int32_t>(0x10)[0] = r.range(0, 3);
                                 for (int i = 0; i < 3; i++) arena<uint32_t>(0x20)[i] = r.next();
                             },
                             [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *, int32_t))fn)(arena<void>(0x100), arena<void>(0x20), arena<int32_t>(0x10)[0])}; }};
    quick(scenery_part_862df0);
    // Camera::PassCD8060: with or without a target, a random matrix
    Test camera_pass_cd8060{0xcd8060,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                for (int i = 0; i < 16; i++) arena<float>(0x160)[i] = r.range(0, 4) ? r.real(-3, 3) : (float)r.range(-1, 1);
                                *(uint32_t *)(self + 8) = r.range(0, 4) ? (uint32_t)(uintptr_t)arena<void>(0x300) : 0;
                                *(uint32_t *)arena<uint8_t>(0x310) = (uint32_t)(uintptr_t)arena<void>(0x380);
                                for (int k = 0; k < 3; k++) arena<float>(0x380)[k] = r.real(-10, 10);
                                for (int i = 0; i < 3; i++) arena<uint32_t>(0x20)[i] = r.next();
                            },
                            [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x20))}; }};
    quick(camera_pass_cd8060);
    // Track::PointBB7030: t 0 or not, lengths at, below and above 0, a random step
    Test track_point_bb7030{0xbb7030,
                            [=](Rng &r) {
                                for (int i = 0; i < 3; i++) arena<float>(0x20)[i] = r.real(-10, 10), arena<float>(0x30)[i] = r.real(-10, 10), arena<float>(0x40)[i] = r.real(-10, 10);
                                arena<float>(0x50)[0] = r.range(0, 4) ? r.real(-1, 6) : (float)r.range(-1, 1);
                                *(float *)arena<uint8_t>(0x130) = r.real(0.1f, 3);
                                arena<float>(0x10)[0] = r.range(0, 3) ? r.real(0, 1) : 0.0f;
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(void *, void *, void *, void *, void *, float))fn)(arena<void>(0x100), arena<void>(0x20), arena<void>(0x50), arena<void>(0x30), arena<void>(0x40), arena<float>(0x10)[0]);
                                return Result{};
                            }};
    quick(track_point_bb7030);
    // Ride::Set53CAD0: random scales and values, the two shared parts the same one at times
    Test ride_set_53cad0{0x53cad0,
                         [=](Rng &r) {
                             for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next();
                             arena<uint32_t>(0x200)[0] = (uint32_t)(uintptr_t)arena<void>(0x300), arena<uint32_t>(0x204)[0] = (uint32_t)(uintptr_t)arena<void>(r.range(0, 2) ? 0x310 : 0x300);
                             arena<uint32_t>(0x300)[0] = r.range(0, 9), arena<uint32_t>(0x310)[0] = r.range(0, 9);
                             for (int i = 0; i < 11; i++) arena<float>(0x400)[i] = r.real(-100, 100);
                             arena<float>(0x10)[0] = r.real(-100, 100);
                         },
                         [](void *fn) {
                             return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *, void *, float, void *))fn)(arena<void>(0x100), arena<void>(0x200), arena<void>(0x204), arena<float>(0x10)[0], arena<void>(0x400))};
                         }};
    quick(ride_set_53cad0);
    // List::StepBCBE20: pieces 0-4 of lists of 1-4 entries (the last wrapping to the first)
    Test list_step_bcbe20{0xbcbe20,
                          [=](Rng &r) {
                              arena<uint32_t>(0x100)[0] = r.range(1, 4), arena<uint32_t>(0x100)[1] = (uint32_t)(uintptr_t)arena<void>(0x400);
                              for (int i = 0; i < 0x2d; i++) arena<float>(0x400)[i] = r.real(-10, 10);
                              for (int i = 0; i < 12; i++) arena<uint32_t>(0x200)[i] = r.next();
                              arena<int32_t>(0x10)[0] = r.range(0, 4);
                          },
                          [](void *fn) {
                              ((void(__thiscall *)(void *, int32_t, void *, void *, void *, void *))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0], arena<void>(0x200), arena<void>(0x20c), arena<void>(0x218), arena<void>(0x224));
                              return Result{};
                          }};
    quick(list_step_bcbe20);
    // Entries::SortB78EA0: heaps of 0-4 entries with random keys
    Test entries_sort_b78ea0{0xb78ea0,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100);
                                 *(uint32_t *)self = (uint32_t)(uintptr_t)arena<void>(0x400), *(uint32_t *)(self + 8) = r.range(0, 4);
                                 for (int i = 0; i < 0x70; i++) arena<uint32_t>(0x400)[i] = r.range(0, 1) ? r.next() : r.range(0, 9);
                                 for (int i = 0; i < 0x1c; i++) arena<uint32_t>(0x200)[i] = r.next();
                                 arena<uint32_t>(0x10)[0] = r.next();
                             },
                             [](void *fn) { return Result{((uint8_t(__thiscall *)(void *, void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x200), arena<uint32_t>(0x10)[0])}; }};
    quick(entries_sort_b78ea0);
    // Memory::CopyCA6A50: no buffer; a first buffer of 0x30-0x80 bytes; a table of 1-3 entries moved to a buffer smaller,
    // the same size or larger
    Test memory_copy_ca6a50{0xca6a50,
                            [=](Rng &r) {
                                uint32_t *self = arena<uint32_t>(0x100);
                                for (int i = 0; i < 0x400; i++) arena<uint32_t>(0x1000)[i] = r.next();
                                const int how = r.range(0, 5);
                                uint32_t buffer = (uint32_t)(uintptr_t)arena<void>(0x2000), size = r.range(0x30, 0x80) & ~3u;
                                if (how == 0) {
                                    buffer = 0;
                                    self[0] = r.next(), self[1] = r.next(), self[2] = r.next();
                                } else if (how == 1) {
                                    self[0] = r.next(), self[1] = 0, self[2] = r.next();
                                } else {
                                    const uint32_t old = (uint32_t)(uintptr_t)arena<void>(0x1000), used = 4 * r.range(8, 24), n = r.range(1, 3);
                                    self[1] = old, self[0] = old + used, self[2] = n | (r.next() & 0xffff0000u);
                                    for (uint32_t k = 0; k < n; k++) arena<uint32_t>(0x1000 + used + 8 * k)[0] = 4 * r.range(0, 6) | r.range(0, 1);
                                    size = used + 8 * n + 4 * r.range(-2, 12);
                                }
                                arena<uint32_t>(0x10)[0] = buffer, arena<uint32_t>(0x10)[1] = size;
                            },
                            [](void *fn) {
                                return Result{((uint8_t(__thiscall *)(void *, void *, uint32_t))fn)(arena<void>(0x100), (void *)(uintptr_t)arena<uint32_t>(0x10)[0], arena<uint32_t>(0x10)[1])};
                            }};
    quick(memory_copy_ca6a50);
    // List::PassC25A10: 0-3 objects (a slot empty at times) with flags 8, 0x100, both or neither, directions of any size
    Test list_pass_c25a10{0xc25a10,
                          [=](Rng &r) {
                              uint8_t *self = arena<uint8_t>(0x100);
                              *(int32_t *)(self + 0xc) = r.range(0, 3), *(uint32_t *)(self + 0x10) = (uint32_t)(uintptr_t)arena<void>(0x200);
                              for (int i = 0; i < 3; i++) {
                                  arena<uint32_t>(0x200)[i] = r.range(0, 5) ? (uint32_t)(uintptr_t)arena<void>(0x400 + 0x200 * i) : 0;
                                  for (int k = 0; k < 0x60; k++) arena<uint32_t>(0x400 + 0x200 * i)[k] = r.next();
                                  for (int k = 0; k < 3; k++) arena<float>(0x470 + 0x200 * i)[k] = r.range(0, 5) ? r.real(-3, 3) : 0.0f;
                                  arena<uint32_t>(0x450 + 0x200 * i)[0] = (r.next() & ~0x108u) | (r.range(0, 1) ? 8 : 0) | (r.range(0, 1) ? 0x100 : 0);
                              }
                          },
                          [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(list_pass_c25a10);
    // Loop::StepCDF4C0: heaps of 0-9 triangles over 8 points, any start, a random direction
    Test loop_step_cdf4c0{0xcdf4c0,
                          [=](Rng &r) {
                              for (int i = 0; i < 10; i++) {
                                  uint32_t *e = arena<uint32_t>(0x400 + 32 * i);
                                  for (int k = 0; k < 8; k++) e[k] = r.next();
                                  e[0] = r.range(0, 7), e[1] = r.range(0, 7), e[2] = r.range(0, 7);
                              }
                              for (int i = 0; i < 32; i++) arena<float>(0x200)[i] = r.real(-5, 5);
                              for (int k = 0; k < 3; k++) arena<float>(0x300)[k] = r.real(-1, 1);
                              const int n = r.range(0, 9);
                              arena<int32_t>(0x10)[0] = r.range(0, n > 0 ? n - 1 : 0), arena<int32_t>(0x10)[1] = n;
                          },
                          [](void *fn) {
                              return Result{((uint32_t(__fastcall *)(int32_t, int32_t, void *, uint32_t, uint32_t))fn)(arena<int32_t>(0x10)[0], arena<int32_t>(0x10)[1], arena<void>(0x400),
                                                                                                                      (uint32_t)(uintptr_t)arena<void>(0x200), (uint32_t)(uintptr_t)arena<void>(0x300))};
                          }};
    quick(loop_step_cdf4c0);
    // Labels::Screen4368B0: with or without the GUI root, screen sizes from 0 to 2000 (equal corners at times)
    Test labels_screen{0x4368b0,
                       [=](Rng &r) {
                           *(volatile uint32_t *)0x12f83f0 = r.range(0, 5) ? (uint32_t)(uintptr_t)arena<void>(0x200) : 0;
                           for (int k = 0; k < 4; k++) *(float *)arena<uint8_t>(0x27c + 4 * k) = r.range(0, 5) ? (float)r.range(0, 2000) : 0.0f;
                           *(uint32_t *)arena<uint8_t>(0x11c) = (uint32_t)(uintptr_t)arena<void>(0x400), *(uint32_t *)arena<uint8_t>(0x120) = (uint32_t)(uintptr_t)arena<void>(0x440);
                           for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x400)[i] = r.next();
                       },
                       [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    t.push_back(labels_screen);
    // Vector::StepBB7800: SSE flag on or off, random matrices, out apart from a and b or the same as either
    Test matrix_product_bb7800{0xbb7800,
                               [=](Rng &r) {
                                   for (int i = 0; i < 16; i++) arena<float>(0x100)[i] = r.real(-4, 4), arena<float>(0x140)[i] = r.real(-4, 4), arena<uint32_t>(0x180)[i] = r.next();
                                   *(volatile uint8_t *)0x13d07fc = (uint8_t)(r.range(0, 1) ? r.range(1, 255) : 0);
                                   const int where = r.range(0, 3);
                                   arena<uint32_t>(0x10)[0] = where == 1 ? 0x100 : where == 2 ? 0x140 : 0x180;
                               },
                               [](void *fn) {
                                   ((void(__fastcall *)(void *, void *, void *))fn)(arena<void>(arena<uint32_t>(0x10)[0]), arena<void>(0x100), arena<void>(0x140));
                                   return Result{};
                               }};
    t.push_back(matrix_product_bb7800);
    // Terrain::Length8BFB10: directions of any length (straight up or none at times), piece sizes 0.5-4
    Test terrain_length_8bfb10{0x8bfb10,
                               [=](Rng &r) {
                                   for (int i = 0; i < 0x34; i++) arena<uint32_t>(0x100)[i] = r.next();
                                   const int how = r.range(0, 5);
                                   for (int k = 0; k < 3; k++) arena<float>(0x190)[k] = how == 0 ? (k == 1 ? r.real(0.1f, 3) : 0.0f) : how == 1 ? 0.0f : r.real(-3, 3);
                                   arena<float>(0x114)[0] = r.real(0.5f, 4), arena<float>(0x118)[0] = r.real(0.5f, 4);
                               },
                               [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(terrain_length_8bfb10);
    // Camera::ScreenRay: flat or not, fields of view 0.2-2.5, screen points across the view (and the centre)
    Test camera_screen_ray{0xcd79c0,
                           [=](Rng &r) {
                               for (int i = 0; i < 0x70; i++) arena<uint32_t>(0x100)[i] = r.next();
                               uint8_t *self = arena<uint8_t>(0x100);
                               for (uint32_t at = 0x120; at < 0x16c; at += 4) *(float *)(self + at) = r.real(-2, 2);
                               *(float *)(self + 0x190) = r.real(0.2f, 2.5f), *(float *)(self + 0x194) = r.real(0.5f, 2), *(float *)(self + 0x198) = r.real(1, 50);
                               self[0x1a6] = (uint8_t)(r.range(0, 2) ? 0 : r.range(1, 255));
                               if (r.range(0, 5) == 0) for (uint32_t at = 0x120; at < 0x14c; at += 4) *(float *)(self + at) = 0.0f;
                               arena<float>(0x10)[0] = r.range(0, 5) ? r.real(-1, 1) : 0.0f, arena<float>(0x10)[1] = r.range(0, 5) ? r.real(-1, 1) : 0.0f;
                               for (int i = 0; i < 6; i++) arena<uint32_t>(0x20)[i] = r.next();
                           },
                           [](void *fn) {
                               ((void(__thiscall *)(void *, float, float, void *, void *))fn)(arena<void>(0x100), arena<float>(0x10)[0], arena<float>(0x10)[1], arena<void>(0x20), arena<void>(0x2c));
                               return Result{};
                           }};
    quick(camera_screen_ray);
    // Vector::DistanceToLine: points before, beside and past segments (of no length at times)
    Test vector_distance_to_line{0xcdb390,
                                 [=](Rng &r) {
                                     for (int k = 0; k < 3; k++) arena<float>(0x100)[k] = r.real(-6, 6), arena<float>(0x110)[k] = r.real(-4, 4), arena<float>(0x120)[k] = r.real(-4, 4);
                                     if (r.range(0, 5) == 0) for (int k = 0; k < 3; k++) arena<float>(0x120)[k] = arena<float>(0x110)[k];
                                     if (r.range(0, 5) == 0) for (int k = 0; k < 3; k++) arena<float>(0x100)[k] = arena<float>(0x110)[k];
                                 },
                                 [](void *fn) {
                                     Result x;
                                     x.f = ((long double(__fastcall *)(void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x110), arena<void>(0x120));
                                     return x;
                                 }};
    quick(vector_distance_to_line);
    // Box::Transform: boxes the right way round, flat or turned over, random matrices (zeros and negatives)
    Test box_transform{0x85d2f0,
                       [=](Rng &r) {
                           for (int k = 0; k < 3; k++) {
                               const float a = r.real(-5, 5), b = r.real(-5, 5);
                               arena<float>(0x100)[k] = a < b ? a : b, arena<float>(0x110)[k] = a < b ? b : a;
                           }
                           if (r.range(0, 5) == 0) { const float t = arena<float>(0x104)[0]; arena<float>(0x104)[0] = arena<float>(0x114)[0], arena<float>(0x114)[0] = t; }
                           if (r.range(0, 5) == 0) arena<float>(0x114)[0] = arena<float>(0x104)[0];
                           for (int i = 0; i < 16; i++) arena<float>(0x200)[i] = r.range(0, 4) ? r.real(-2, 2) : (float)r.range(-1, 1);
                           for (int i = 0; i < 6; i++) arena<uint32_t>(0x300)[i] = r.next();
                       },
                       [](void *fn) {
                           ((void(__fastcall *)(void *, void *, void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x110), arena<void>(0x200), arena<void>(0x300), arena<void>(0x30c));
                           return Result{};
                       }};
    quick(box_transform);
    // Mesh::ComputeNormals: 0-6 vertices, 0-5 triangles over them (flat or repeated corners at times)
    Test mesh_compute_normals{0x95faa0,
                              [=](Rng &r) {
                                  uint8_t *self = arena<uint8_t>(0x100);
                                  const int n = r.range(0, 6);
                                  *(int32_t *)self = n, *(int32_t *)(self + 0x10) = n ? r.range(0, 5) : 0;
                                  *(uint32_t *)(self + 4) = (uint32_t)(uintptr_t)arena<void>(0x200), *(uint32_t *)(self + 8) = (uint32_t)(uintptr_t)arena<void>(0x300);
                                  *(uint32_t *)(self + 0xc) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                  for (int i = 0; i < 24; i++) arena<float>(0x200)[i] = r.range(0, 5) ? r.real(-3, 3) : 0.0f, arena<uint32_t>(0x300)[i] = r.next();
                                  for (int i = 0; i < 15; i++) arena<uint32_t>(0x400)[i] = n ? r.range(0, n - 1) : 0;
                              },
                              [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(mesh_compute_normals);
    // Vector::LengthBED120 / BED2B0: axes along each part (either sign), random, tied parts and zero
    auto frame_about_test = [&](uintptr_t at) {
        Test x{at,
               [=](Rng &r) {
                   for (int i = 0; i < 12; i++) arena<uint32_t>(0x100)[i] = r.next();
                   const int how = r.range(0, 5);
                   for (int k = 0; k < 3; k++) arena<float>(0x200)[k] = how == 0 ? (float)r.range(-1, 1) : how == 1 ? (k == r.range(0, 2) ? r.real(-3, 3) : r.real(-0.1f, 0.1f)) : r.real(-3, 3);
                   if (r.range(0, 7) == 0) arena<float>(0x200)[r.range(0, 2)] = arena<float>(0x200)[r.range(0, 2)];
               },
               [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200)); return Result{}; }};
        quick(x);
    };
    frame_about_test(0xbed120), frame_about_test(0xbed2b0);
    // RideCarSpinningPart::TrackMotion: states 0-3, frame times at, below and above 0, random positions
    Test spinning_part_track_motion{0x52bae0,
                                    [=](Rng &r) {
                                        for (int i = 0; i < 0x80; i++) arena<uint32_t>(0x100)[i] = r.next();
                                        uint8_t *self = arena<uint8_t>(0x100);
                                        for (uint32_t at = 0x1d0; at < 0x1fc; at += 4) *(float *)(self + at) = r.real(-10, 10);
                                        *(uint32_t *)(self + 0x1c8) = r.range(0, 3), *(uint32_t *)(self + 0xf4) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                        for (int k = 0; k < 3; k++) arena<float>(0x490)[k] = r.real(-10, 10);
                                        arena<float>(0x10)[0] = r.range(0, 5) ? r.real(0.001f, 0.1f) : (float)r.range(-1, 0);
                                    },
                                    [](void *fn) { ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x10)[0]); return Result{}; }};
    quick(spinning_part_track_motion);
    // Plane::BelowCDB0E0 and Water::Below93D140: random triangles and rays, half aimed inside the triangle (some at its
    // edges or away), flat triangles at times
    auto ray_triangle_setup = [=](Rng &r) {
        float *v0 = arena<float>(0x100), *e1 = arena<float>(0x110), *e2 = arena<float>(0x120), *o = arena<float>(0x130), *d = arena<float>(0x140);
        for (int k = 0; k < 3; k++) v0[k] = r.real(-3, 3), e1[k] = r.real(-3, 3), e2[k] = r.real(-3, 3), o[k] = r.real(-6, 6);
        if (r.range(0, 7) == 0) for (int k = 0; k < 3; k++) e2[k] = 2 * e1[k];
        const int how = r.range(0, 3);
        if (how == 0) {
            for (int k = 0; k < 3; k++) d[k] = r.real(-3, 3);
        } else {
            const float a = how == 1 ? r.real(0, 0.5f) : (float)r.range(0, 1), b = how == 1 ? r.real(0, 0.5f) : r.real(-0.2f, 1.2f);
            for (int k = 0; k < 3; k++) d[k] = (v0[k] + a * e1[k] + b * e2[k] - o[k]) * (r.range(0, 4) ? 1.0f : -1.0f);
        }
        arena<uint32_t>(0x150)[0] = r.next();
    };
    Test plane_below_cdb0e0{0xcdb0e0, ray_triangle_setup, [](void *fn) {
                                return Result{((uint8_t(__fastcall *)(void *, void *, void *, void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x110), arena<void>(0x120), arena<void>(0x130), arena<void>(0x140), arena<void>(0x150))};
                            }};
    quick(plane_below_cdb0e0);
    Test water_below_93d140{0x93d140, ray_triangle_setup, [](void *fn) {
                                uint32_t stack[2] = {(uint32_t)(uintptr_t)arena<void>(0x110), (uint32_t)(uintptr_t)arena<void>(0x150)};
                                uint32_t *ps = stack;
                                uint32_t eax = (uint32_t)(uintptr_t)arena<void>(0x130);
                                void *v0 = arena<void>(0x100), *e2 = arena<void>(0x120), *d = arena<void>(0x140);
                                __asm__ volatile("pushl 4(%%edx)\n\tpushl (%%edx)\n\tcall *%%ebx"
                                                 : "+a"(eax), "+c"(e2), "+d"(ps), "+D"(v0), "+S"(d)
                                                 : "b"(fn)
                                                 : "memory", "cc");
                                return Result{(uint32_t)(uint8_t)eax};
                            }};
    quick(water_below_93d140);
    // Terrain::Below8BBB60: the same rays and triangles, the normal written out too
    Test terrain_below_8bbb60{0x8bbb60, ray_triangle_setup, [](void *fn) {
                                  return Result{((uint8_t(__fastcall *)(void *, void *, void *, void *, void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x110), arena<void>(0x120), arena<void>(0x130), arena<void>(0x140), arena<void>(0x160), arena<void>(0x150))};
                              }};
    quick(terrain_below_8bbb60);
    // Terrain::Below8BBD40: the same rays and triangles with the normal given (the true one, or off at times)
    Test terrain_below_8bbd40{0x8bbd40,
                              [=](Rng &r) {
                                  ray_triangle_setup(r);
                                  const float *e1 = arena<float>(0x110), *e2 = arena<float>(0x120);
                                  float *n = arena<float>(0x160);
                                  n[0] = e1[1] * e2[2] - e2[1] * e1[2], n[1] = e2[0] * e1[2] - e1[0] * e2[2], n[2] = e2[1] * e1[0] - e1[1] * e2[0];
                                  if (r.range(0, 4) == 0) n[r.range(0, 2)] += r.real(-1, 1);
                              },
                              [](void *fn) {
                                  return Result{((uint8_t(__fastcall *)(void *, void *, void *, void *, void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x110), arena<void>(0x120), arena<void>(0x130), arena<void>(0x140), arena<void>(0x160), arena<void>(0x150))};
                              }};
    quick(terrain_below_8bbd40);
    // Path::Below71C470: the same rays and triangles, the nearest t so far large, small or between
    Test path_below_71c470{0x71c470,
                           [=](Rng &r) {
                               ray_triangle_setup(r);
                               arena<float>(0x150)[0] = r.range(0, 2) ? 1e30f : r.real(0, 2);
                           },
                           [](void *fn) {
                               return Result{((uint8_t(__fastcall *)(void *, void *, void *, void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x110), arena<void>(0x120), arena<void>(0x130), arena<void>(0x140), arena<void>(0x150))};
                           }};
    quick(path_below_71c470);
    // Water::Length94CA80: unit normals either way, directions of any size, ratios near 1 (so both branches happen)
    Test water_bend{0x94ca80,
                    [=](Rng &r) {
                        float *n = arena<float>(0x100), *v = arena<float>(0x110);
                        for (int k = 0; k < 3; k++) n[k] = r.real(-1, 1), v[k] = r.real(-2, 2);
                        if (r.range(0, 6) == 0) for (int k = 0; k < 3; k++) v[k] = n[k] * r.real(-2, 2);
                        arena<float>(0x10)[0] = r.range(0, 3) ? r.real(0.5f, 1.5f) : r.real(0.05f, 0.5f);
                        for (int i = 0; i < 3; i++) arena<uint32_t>(0x120)[i] = r.next();
                    },
                    [](void *fn) {
                        uint32_t eax;
                        void *n = arena<void>(0x100), *v = arena<void>(0x110), *out = arena<void>(0x120);
                        const uint32_t ratio = arena<uint32_t>(0x10)[0];
                        __asm__ volatile("pushl %[r]\n\tcall *%[f]"
                                         : "=a"(eax), "+c"(n), "+d"(v), "+S"(out)
                                         : [r] "m"(ratio), [f] "b"(fn)
                                         : "memory", "cc");
                        return Result{eax};
                    }};
    quick(water_bend);
    // Sound::Direction: listeners before, beside and past segments (of no length at times)
    Test sound_direction{0xce55d0,
                         [=](Rng &r) {
                             for (int i = 0; i < 0x30; i++) arena<uint32_t>(0x100)[i] = r.next();
                             for (int k = 0; k < 3; k++) arena<float>(0x190)[k] = r.real(-4, 4), arena<float>(0x1a0)[k] = r.real(-4, 4), ((volatile float *)0x10f4620)[k] = r.real(-6, 6);
                             if (r.range(0, 5) == 0) for (int k = 0; k < 3; k++) arena<float>(0x1a0)[k] = arena<float>(0x190)[k];
                             for (int i = 0; i < 3; i++) arena<uint32_t>(0x20)[i] = r.next();
                         },
                         [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x20))}; }};
    t.push_back(sound_direction);
    // Vector::SameC5D1C0: pairs close or not in each part (exact, tiny and large differences), either order
    Test vector_same_c5d1c0{0xc5d1c0,
                            [=](Rng &r) {
                                for (int k = 0; k < 4; k++) {
                                    const float v = r.real(-5, 5);
                                    const int how = r.range(0, 2);
                                    arena<float>(0x100)[k] = v, arena<float>(0x110)[k] = how == 0 ? v : how == 1 ? v + r.real(-1e-4f, 1e-4f) : r.real(-5, 5);
                                }
                                for (int i = 0; i < 4; i++) arena<uint32_t>(0x120)[i] = r.next();
                            },
                            [](void *fn) {
                                return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *, void *, void *))fn)(arena<void>(0x120), arena<void>(0x100), arena<void>(0x110))};
                            }};
    quick(vector_same_c5d1c0);
    // Window::PointToScreen and its copies: windows with no parent or a chain of one or two (scroll flag on or off)
    auto point_to_screen_test = [&](uintptr_t at, int d) {
        Test x{at,
               [=](Rng &r) {
                   for (int i = 0; i < 0x180; i++) arena<float>(0x200)[i] = r.real(-500, 500);
                   uint8_t *window = arena<uint8_t>(0x400 + d);
                   const int parents = r.range(0, 2);
                   *(uint32_t *)(window + 0x3c) = parents ? (uint32_t)(uintptr_t)arena<void>(0x800) : 0;
                   *(uint32_t *)arena<uint8_t>(0x83c) = parents > 1 ? (uint32_t)(uintptr_t)arena<void>(0x900) : 0;
                   *(uint32_t *)arena<uint8_t>(0x93c) = 0;
                   *(uint32_t *)(window + 0x94) = r.next(), *(uint32_t *)arena<uint8_t>(0x894) = r.next();
                   for (uint32_t at2 : {0x7cu, 0x80u, 0x8cu, 0x90u}) *(float *)arena<uint8_t>(0x800 + at2) = r.real(-500, 500), *(float *)arena<uint8_t>(0x900 + at2) = r.real(-500, 500);
                   arena<float>(0x10)[0] = r.real(-100, 100), arena<float>(0x10)[1] = r.real(-100, 100);
                   arena<uint32_t>(0x20)[0] = r.next(), arena<uint32_t>(0x20)[1] = r.next();
               },
               [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x400), arena<void>(0x20), arena<void>(0x10))}; }};
        quick(x);
    };
    for (uintptr_t at : {0xac7fb0u, 0x99b2b0u, 0x9b9cd0u, 0xa213a0u, 0xa21a40u, 0xaccb40u}) point_to_screen_test(at, -0xd4);
    point_to_screen_test(0x67d320, -0xdc), point_to_screen_test(0xae2090, -0xdc), point_to_screen_test(0x9973d0, -0xec);
    point_to_screen_test(0x67f300, -0x114), point_to_screen_test(0x996680, -0x13c);
    // Math::ScaleC198C0 / C19910 / C19960: n from -2000 to 2000 (and wider at times)
    for (uintptr_t at : {0xc198c0u, 0xc19910u, 0xc19960u}) {
        Test x{at, [=](Rng &r) { arena<int32_t>(0x10)[0] = r.range(0, 5) ? r.range(-2000, 2000) : (int32_t)r.next(); },
               [](void *fn) { return Result{((uint32_t(__stdcall *)(int32_t))fn)(arena<int32_t>(0x10)[0])}; }};
        quick(x);
    }
    // Window::RectFromSize and its copies: random sizes; Window::UpdateRect stood in for (it notes the part and the rectangle)
    for (uintptr_t at : {0x490000u, 0x448380u, 0xa1faf0u, 0xa1fb70u}) {
        Test x{at,
               [=](Rng &r) {
                   for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x100)[i] = r.next();
                   for (int k = 0; k < 4; k++) arena<float>(0x17c)[k] = r.real(-500, 500);
                   arena<uint32_t>(0x40)[0] = 0;
               },
               [](void *fn) { return Result{((uint32_t(__fastcall *)(void *))fn)(arena<void>(0x100))}; }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x6afaf0, (void *)h2_update_rect}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Effect::Hit488A00 and its copies: random points; the collision call stood in for
    for (uintptr_t at : {0x488a00u, 0x7b3580u, 0xb5bb70u, 0xb6bff0u}) {
        Test x{at,
               [=](Rng &r) {
                   arena<uint32_t>(0x100)[0] = r.next(), arena<uint32_t>(0x100)[1] = r.next();
                   for (int k = 0; k < 3; k++) arena<float>(0x110)[k] = r.real(-50, 50), arena<float>(0x120)[k] = r.real(-50, 50), arena<float>(0x130)[k] = r.real(-50, 50);
               },
               [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x110), arena<void>(0x120), arena<void>(0x130))}; }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xcda7f0, (void *)h2_hit}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // WildAnimal::InterfaceB35450 and its copies: classes -1 to 4; the interface getter stood in for
    for (uintptr_t at : {0xb35450u, 0xb355b0u, 0xb355f0u, 0xb35630u, 0xb35670u}) {
        Test x{at,
               [=](Rng &r) {
                   *(int32_t *)arena<uint8_t>(0x1a4) = r.range(-1, 4);
                   *(uint32_t *)arena<uint8_t>(0x654) = (uint32_t)(uintptr_t)arena<void>(0x700);
                   for (int k = 0; k < 4; k++) arena<uint32_t>(0x718)[k] = (uint32_t)(uintptr_t)arena<void>(0x800 + 0x80 * k);
                   for (int i = 0; i < 0x80; i++) arena<float>(0x800)[i] = r.real(-100, 100);
               },
               [](void *fn) { Result x; x.f = ((long double(__thiscall *)(void *))fn)(arena<void>(0x100)); return x; }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xb44970, (void *)h2_animal_iface}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // AttractionPersonStrategy::PointSlot35 and its copies: random window places and points; the root's slots stood in for
    for (uintptr_t at : {0x5f7750u, 0x5f77b0u, 0x5f7810u, 0x5f7890u, 0x9ee850u}) {
        Test x{at,
               [=](Rng &r) {
                   for (int k = 0; k < 6; k++) arena<float>(0x17c)[k] = r.real(-500, 500);
                   arena<float>(0x10)[0] = r.real(-100, 100), arena<float>(0x10)[1] = r.real(-100, 100);
                   *(volatile uint32_t *)0x12f83f0 = (uint32_t)(uintptr_t)arena<void>(0x600);
                   arena<uint32_t>(0x600)[0] = (uint32_t)(uintptr_t)arena<void>(0x700);
                   for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x700)[i] = (uint32_t)(uintptr_t)&h2_root_slot;
               },
               [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x10))}; }};
        t.push_back(x);
    }
    // Guest::Shared7594E0 and its copies: the park object there, park values in and out of 0-1 (the game's own random numbers)
    for (uintptr_t at : {0x7594e0u, 0x759570u, 0x7595f0u}) {
        Test x{at,
               [=](Rng &r) {
                   *(volatile uint32_t *)0x1306194 = (uint32_t)(uintptr_t)arena<void>(0x400);
                   for (int k = 0; k < 4; k++) arena<float>(0x568)[k] = r.range(0, 3) ? r.real(-0.5f, 1.5f) : (float)r.range(0, 1);
                   for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x100 + 0x400)[i] = r.next();
               },
               [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
        t.push_back(x);
    }
    // Colour::Make6B5D10: hues across every sector (and below 0), saturations and lightnesses in and past 0-1
    Test colour_make_6b5d10{0x6b5d10,
                            [=](Rng &r) {
                                arena<float>(0x10)[0] = r.range(0, 7) ? r.real(-0.05f, 1.05f) : (float)r.range(0, 6) / 6.0f;
                                arena<float>(0x10)[1] = r.range(0, 5) ? r.real(0, 1) : r.real(-0.5f, 1.5f);
                                arena<float>(0x10)[2] = r.range(0, 5) ? r.real(0, 1) : r.real(-0.5f, 1.5f);
                                arena<uint32_t>(0x20)[0] = r.next();
                            },
                            [](void *fn) {
                                const float *a = arena<float>(0x10);
                                return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, float, float, float))fn)(arena<void>(0x20), a[0], a[1], a[2])};
                            }};
    quick(colour_make_6b5d10);
    // Water::Fade919610: edge or not, flag 0x40 or not, cells inside and past the fade width, levels across the limits
    Test water_fade_919610{0x919610,
                           [=](Rng &r) {
                               for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x100)[i] = r.next();
                               uint8_t *self = arena<uint8_t>(0x100);
                               *(volatile int32_t *)0x10f0bec = r.range(1, 8);
                               *(int32_t *)(self + 0x154) = r.range(20, 40), *(int32_t *)(self + 0x158) = r.range(2, 30);
                               *(float *)(self + 0x31c) = r.real(-1, 3), *(float *)(self + 0x320) = r.real(-1, 3);
                               arena<int32_t>(0x10)[0] = r.range(0, 40), arena<int32_t>(0x10)[1] = r.range(0, 30);
                               arena<uint32_t>(0x10)[2] = r.range(0, 1) * r.range(1, 255) | (r.next() & 0xffffff00u);
                               arena<uint32_t>(0x20)[0] = r.next(), arena<uint32_t>(0x20)[1] = r.next();
                           },
                           [](void *fn) {
                               const int32_t *a = arena<int32_t>(0x10);
                               return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *, int32_t, int32_t, uint32_t))fn)(arena<void>(0x100), arena<void>(0x20), a[0], a[1], (uint32_t)a[2])};
                           }};
    t.push_back(water_fade_919610);
    // Vector::ScaleBEE2C0: random matrices and turns (traces either side of 0, each diagonal the largest), zeros at times
    Test matrix_to_rotation{0xbee2c0,
                            [=](Rng &r) {
                                for (int i = 0; i < 12; i++) arena<float>(0x100)[i] = r.range(0, 5) ? r.real(-1, 1) : (float)r.range(-1, 1);
                                if (r.range(0, 4) == 0) arena<float>(0x100)[5 * r.range(0, 2)] = 2.0f;
                                for (int i = 0; i < 4; i++) arena<uint32_t>(0x200)[i] = r.next();
                            },
                            [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x200)); return Result{}; }};
    quick(matrix_to_rotation);
    // Animal::ModeA6F930: modes 1-3, four random points (the fourth either side of the first three's plane, or on it)
    Test animal_mode_a6f930{0xa6f930,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                *(int32_t *)self = r.range(0, 3) ? 2 : r.range(1, 3);
                                *(uint32_t *)(self + 0xc) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                for (int i = 0; i < 4; i++) {
                                    arena<uint32_t>(0x200)[i] = (uint32_t)(uintptr_t)arena<void>(0x400 + 0x40 * i);
                                    for (int k = 0; k < 16; k++) arena<float>(0x400 + 0x40 * i)[k] = r.real(-5, 5);
                                }
                                if (r.range(0, 5) == 0) for (int k = 4; k < 7; k++) arena<float>(0x4c0)[k] = arena<float>(0x400)[k];
                            },
                            [](void *fn) { ((void(__fastcall *)(void *))fn)(arena<void>(0x100)); return Result{}; }};
    quick(animal_mode_a6f930);
    // Animal::StepBB6B10: add or not, t 0 or between, weights below, at and above 1
    Test animal_step_bb6b10{0xbb6b10,
                            [=](Rng &r) {
                                for (int k = 0; k < 3; k++) arena<float>(0x100)[k] = r.real(-10, 10), arena<float>(0x110)[k] = r.real(-10, 10), arena<float>(0x120)[k] = r.real(-10, 10);
                                arena<float>(0x10)[0] = r.range(0, 3) ? r.real(0, 2) : 1.0f, arena<float>(0x10)[1] = r.range(0, 3) ? r.real(0, 1) : 0.0f;
                                arena<uint32_t>(0x10)[2] = r.range(0, 1) * r.range(1, 255) | (r.next() & 0xffffff00u);
                            },
                            [](void *fn) {
                                const float *a = arena<float>(0x10);
                                ((void(__fastcall *)(void *, void *, float, void *, float, uint32_t))fn)(arena<void>(0x120), arena<void>(0x100), a[0], arena<void>(0x110), a[1], arena<uint32_t>(0x10)[2]);
                                return Result{};
                            }};
    quick(animal_step_bb6b10);
    // Graph::RangeACA200: random colours, greys, and colours with two equal parts
    Test graph_range_aca200{0xaca200,
                            [=](Rng &r) {
                                uint32_t c = r.next();
                                const int how = r.range(0, 4);
                                if (how == 0) c = (c & 0xff) * 0x10101;
                                else if (how == 1) c = (c & 0xffff00ffu) | (c & 0xff) << 8;
                                else if (how == 2) c = (c & 0xff00ffffu) | (c & 0xff) << 16;
                                arena<uint32_t>(0x10)[0] = c;
                                for (int i = 0; i < 3; i++) arena<uint32_t>(0x20)[i] = r.next();
                            },
                            [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__stdcall *)(void *, uint32_t))fn)(arena<void>(0x20), arena<uint32_t>(0x10)[0])}; }};
    quick(graph_range_aca200);
    // Water::Edge95F1E0: random triangles and rays (half aimed inside), the registers set as the game passes them
    Test water_edge_95f1e0{0x95f1e0,
                           [=](Rng &r) {
                               ray_triangle_setup(r);
                               for (int i = 0; i < 4; i++) arena<uint32_t>(0x150)[i] = r.next();
                           },
                           [](void *fn) {
                               uint32_t stack[4] = {(uint32_t)(uintptr_t)arena<void>(0x130), (uint32_t)(uintptr_t)arena<void>(0x150), (uint32_t)(uintptr_t)arena<void>(0x154), (uint32_t)(uintptr_t)arena<void>(0x158)};
                               uint32_t *ps = stack;
                               void *p = arena<void>(0x100), *a = arena<void>(0x110), *b = arena<void>(0x120), *d = arena<void>(0x140);
                               __asm__ volatile("pushl 12(%%eax)\n\tpushl 8(%%eax)\n\tpushl 4(%%eax)\n\tpushl (%%eax)\n\tcall *%%ebx"
                                                : "+a"(ps), "+D"(p), "+S"(a), "+d"(b), "+c"(d)
                                                : "b"(fn)
                                                : "memory", "cc");
                               return Result{(uint32_t)(uintptr_t)ps};
                           }};
    quick(water_edge_95f1e0);
    // Camera::InverseView: random camera frames and places (singular ones at times)
    Test camera_inverse_view{0xcd78a0,
                             [=](Rng &r) {
                                 for (int i = 0; i < 0x60; i++) arena<float>(0x100)[i] = r.real(-3, 3);
                                 if (r.range(0, 7) == 0) for (int c = 0; c < 3; c++) arena<float>(0x230)[c] = arena<float>(0x220)[c];
                                 for (int i = 0; i < 16; i++) arena<uint32_t>(0x400)[i] = r.next();
                             },
                             [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x400))}; }};
    quick(camera_inverse_view);
    // Text::ReadNumberCBA820: signs, 0-22 digits (past both limits at times), stops on letters and high bytes; never a
    // first tab or space (the game spins on those)
    Test text_read_number{0xcba820,
                          [=](Rng &r) {
                              char *s = arena<char>(0x200);
                              int n = 0;
                              const int sign = r.range(0, 3);
                              if (sign == 1) s[n++] = '-';
                              else if (sign == 2) s[n++] = '+';
                              const int digits = r.range(0, 4) ? r.range(0, 12) : r.range(17, 22);
                              for (int i = 0; i < digits; i++) s[n++] = (char)(r.range(0, 9) ? '0' + r.range(0, 9) : '9');
                              static const char ends[] = {0, 'x', ' ', '.', (char)0x80, '-'};
                              s[n++] = ends[r.range(0, 5)];
                              s[n] = 0;
                              if (s[0] == ' ' || s[0] == 9) s[0] = 'x';
                              if (r.range(0, 9) == 0) { const char *lim = sign == 1 ? "-9223372036854775808" : "9223372036854775807"; memcpy(s, lim, strlen(lim) + 1); }
                              arena<uint32_t>(0x300)[0] = r.next(), arena<uint32_t>(0x300)[1] = r.next();
                          },
                          [](void *fn) {
                              const uint32_t p = ((uint32_t(__fastcall *)(void *, void *))fn)(arena<void>(0x200), arena<void>(0x300));
                              return Result{p ? p - (uint32_t)(uintptr_t)arena<void>(0) : 0xffffffffu};
                          }};
    quick(text_read_number);
    // Math::Split9F8680: part lists from small values (zeros for "none"), a taken from b at times
    Test math_split_9f8680{0x9f8680,
                           [=](Rng &r) {
                               uint32_t *v = arena<uint32_t>(0x10);
                               for (int i = 0; i < 4; i++) v[i] = (uint32_t)r.range(0, 4) | (uint32_t)r.range(0, 4) << 16;
                               if (r.range(0, 2) == 0) {
                                   uint16_t *bp = (uint16_t *)&v[2], *ap = (uint16_t *)&v[0];
                                   int j = 0;
                                   for (int k = 0; k < 4; k++) if (r.range(0, 1)) ap[j++] = bp[k];
                                   while (j < 4) ap[j++] = 0;
                               }
                               if (r.range(0, 7) == 0) v[r.range(0, 3)] = r.next();
                           },
                           [](void *fn) {
                               const uint32_t *v = arena<uint32_t>(0x10);
                               return Result{((uint8_t(__stdcall *)(uint32_t, uint32_t, uint32_t, uint32_t))fn)(v[0], v[1], v[2], v[3])};
                           }};
    quick(math_split_9f8680);
    // Water::Entry94EF60: entries of kind 4 or not in each band, lists of 0-5 records some matching, used or free
    Test water_entry_94ef60{0x94ef60,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                for (int i = 0; i < 0x80; i++) arena<uint32_t>(0x100)[i] = r.next();
                                *(uint32_t *)(self + 0x1a4) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                for (int i = 0; i < 0xd0; i++) arena<uint32_t>(0x400)[i] = r.next();
                                const int n = r.range(0, 2);
                                uint8_t *e = arena<uint8_t>(0x400 + 0xd0 * n);
                                *(int32_t *)(e + 0x60) = r.range(0, 3) ? 4 : 3, *(int32_t *)(e + 4) = r.range(8, 16);
                                *(uint32_t *)(e + 0xa4) = r.range(0, 2), *(uint32_t *)(e + 0xa8) = r.range(0, 2), e[0] = (uint8_t)r.range(0, 1);
                                for (int b = 0; b < 3; b++) {
                                    uint32_t *list = arena<uint32_t>(0x100 + 0x1c * (b + 12));
                                    list[0] = (uint32_t)(uintptr_t)arena<void>(0x1000 + 0x400 * b), list[1] = r.range(0, 5);
                                    for (int i = 0; i < 0x100; i++) arena<uint32_t>(0x1000 + 0x400 * b)[i] = r.next();
                                    for (int i = 0; i < 6; i++) arena<uint32_t>(0x1058 + 0x400 * b + 0x80 * i)[0] = r.range(0, 4), arena<uint32_t>(0x105c + 0x400 * b + 0x80 * i)[0] = r.range(0, 4);
                                }
                                *(uint32_t *)(self + 0x1a8) = (uint32_t)(uintptr_t)arena<void>(0x2000);
                                *(int32_t *)(self + 0x1b0) = 8, *(int32_t *)(self + 0x1ac) = r.range(1, 8);
                                arena<int32_t>(0x10)[0] = n;
                            },
                            [](void *fn) { ((void(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x10)[0]); return Result{}; }};
    quick(water_entry_94ef60);
    // Grid::CellAt: grids of 1-20 by 1-20 cells, positions inside, on edges and outside either way
    Test grid_cell_at{0x8bcc90,
                      [=](Rng &r) {
                          uint8_t *self = arena<uint8_t>(0x100);
                          self[4] = (uint8_t)r.range(1, 20), self[5] = (uint8_t)r.range(1, 20);
                          *(float *)(self + 0xc) = r.real(-50, 50), *(float *)(self + 0x10) = r.real(-50, 50);
                          *(float *)(self + 0x14) = r.range(0, 3) ? 4.0f : r.real(0.5f, 8), *(float *)(self + 0x18) = r.range(0, 3) ? 4.0f : r.real(0.5f, 8);
                          arena<float>(0x10)[0] = *(float *)(self + 0xc) + (r.range(0, 4) ? r.real(-10, 100) : 4.0f * r.range(-2, 22));
                          arena<float>(0x10)[1] = *(float *)(self + 0x10) + (r.range(0, 4) ? r.real(-10, 100) : 4.0f * r.range(-2, 22));
                          arena<uint32_t>(0x20)[0] = r.next();
                      },
                      [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, float, float, void *, void *))fn)(arena<void>(0x100), arena<float>(0x10)[0], arena<float>(0x10)[1], arena<void>(0x20), arena<void>(0x21))}; }};
    quick(grid_cell_at);
    // Matrix::NearestAxis: random and axis-aligned vectors and matrices (ties at times)
    Test matrix_nearest_axis{0x8a08e0,
                             [=](Rng &r) {
                                 for (int i = 0; i < 12; i++) arena<float>(0x100)[i] = r.range(0, 3) ? r.real(-1, 1) : (float)r.range(-1, 1);
                                 for (int k = 0; k < 3; k++) arena<float>(0x200)[k] = r.range(0, 3) ? r.real(-2, 2) : (float)r.range(-1, 1);
                             },
                             [](void *fn) { return Result{((uint32_t(__fastcall *)(void *, void *))fn)(arena<void>(0x200), arena<void>(0x100))}; }};
    quick(matrix_nearest_axis);
    // Vector::OntoC5A0E0: random points and directions (none at times), places that wrap either way or not
    Test vector_onto_c5a0e0{0xc5a0e0,
                            [=](Rng &r) {
                                for (int k = 0; k < 3; k++) arena<float>(0x100)[k] = r.real(-50, 50), arena<float>(0x110)[k] = r.range(0, 9) ? r.real(-3, 3) : 0.0f;
                                for (int i = 0; i < 3; i++) arena<uint32_t>(0x120)[i] = r.next();
                            },
                            [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *, void *, void *))fn)(arena<void>(0x120), arena<void>(0x100), arena<void>(0x110))}; }};
    quick(vector_onto_c5a0e0);
    // Water::Cell907BB0: grids of 2-6 samples a side, places inside, on edges and outside
    Test water_cell_907bb0{0x907bb0,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               const int n = r.range(2, 6);
                               *(int32_t *)self = n;
                               *(float *)(self + 8) = r.real(-20, 0), *(float *)(self + 0xc) = r.real(-20, 0), *(float *)(self + 0x10) = r.real(1, 20), *(float *)(self + 0x14) = r.real(1, 20);
                               *(float *)(self + 0x18) = r.real(-5, 5), *(float *)(self + 0x1c) = r.real(-5, 5), *(float *)(self + 0x2c) = r.real(-2, 2);
                               *(uint32_t *)(self + 0x40) = (uint32_t)(uintptr_t)arena<void>(0x400);
                               for (int i = 0; i < 36; i++) arena<float>(0x400)[i] = r.real(-3, 3);
                               arena<float>(0x10)[0] = r.real(-25, 25), arena<float>(0x10)[1] = r.real(-25, 25);
                           },
                           [](void *fn) { Result x; x.f = ((long double(__thiscall *)(void *, float, float))fn)(arena<void>(0x100), arena<float>(0x10)[0], arena<float>(0x10)[1]); return x; }};
    quick(water_cell_907bb0);
    // Vector::ConeCD9730: random lines, cone points and axes (k of 0 at times, axes along a or b at times), the registers
    // set as the game passes them; only the low byte of the result is the game's
    Test vector_cone_cd9730{0xcd9730,
                            [=](Rng &r) {
                                float *v = arena<float>(0x100);   // a 0x100, b 0x110, c 0x120, d 0x130
                                for (int i = 0; i < 16; i++) v[i] = r.real(-4, 4);
                                const int shape = r.range(0, 9);
                                if (shape == 0) for (int c = 0; c < 3; c++) v[4 + c] = v[c] * 2;      // b along a: n = 0
                                if (shape == 1) for (int c = 0; c < 3; c++) v[12 + c] = v[8 + c];     // d = c
                                if (shape == 2) for (int c = 0; c < 3; c++) v[12 + c] = -v[8 + c];    // d = -c
                                arena<float>(0x140)[0] = r.range(0, 5) == 0 ? 0.0f : r.real(0, r.range(0, 1) ? 0.05f : 20.0f);
                                arena<uint32_t>(0x150)[0] = r.next();
                            },
                            [](void *fn) {
                                uint32_t stack[2] = {(uint32_t)(uintptr_t)arena<void>(0x120), arena<uint32_t>(0x140)[0]};
                                uint32_t *ps = stack;
                                void *b = arena<void>(0x110), *d = arena<void>(0x130), *o = arena<void>(0x150);
                                uint32_t a = (uint32_t)(uintptr_t)arena<void>(0x100);
                                __asm__ volatile("pushl 4(%%edx)\n\tpushl (%%edx)\n\tcall *%%ebx"
                                                 : "+a"(a), "+c"(b), "+S"(d), "+D"(o), "+d"(ps)
                                                 : "b"(fn)
                                                 : "memory", "cc");
                                return Result{a & 0xff};
                            }};
    quick(vector_cone_cd9730);
    // Fence::DistanceA90D00: outlines of 2-8 points, every edge (the last wrapping), lines through, along and past the
    // edge, and lines of no length at times
    Test fence_distance_a90d00{0xa90d00,
                               [=](Rng &r) {
                                   const int n = r.range(2, 8);
                                   uint32_t *list = arena<uint32_t>(0x100);
                                   list[0] = (uint32_t)(uintptr_t)arena<void>(0x200), list[1] = (uint32_t)n;
                                   float *pt = arena<float>(0x200);
                                   for (int k = 0; k < 2 * n; k++) pt[k] = r.real(-10, 10);
                                   const int i = r.range(0, n - 1), j = i + 1 < n ? i + 1 : 0;
                                   float *a = arena<float>(0x40);   // px py qx qy, then i
                                   for (int k = 0; k < 4; k++) a[k] = r.real(-12, 12);
                                   const int shape = r.range(0, 7);
                                   if (shape == 0) a[2] = a[0], a[3] = a[1];                                            // q = p
                                   if (shape == 1) a[2] = a[0] + (pt[2 * j] - pt[2 * i]), a[3] = a[1] + (pt[2 * j + 1] - pt[2 * i + 1]);   // along
                                   if (shape == 2) a[0] = pt[2 * i], a[1] = pt[2 * i + 1];                              // from the edge's start
                                   if (shape == 3) a[2] = pt[2 * j], a[3] = pt[2 * j + 1];                              // to the edge's end
                                   arena<int32_t>(0x50)[0] = i;
                               },
                               [](void *fn) {
                                   const float *a = arena<float>(0x40);
                                   Result x;
                                   x.f = ((long double(__stdcall *)(void *, float, float, float, float, int32_t))fn)(arena<void>(0x100), a[0], a[1], a[2], a[3], arena<int32_t>(0x50)[0]);
                                   return x;
                               }};
    quick(fence_distance_a90d00);
    // Person::SwimToward: targets near and far (on the person at times, so the facing is used), steps and stops either
    // side of the distance
    Test person_swim_toward{0x7c2970,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                for (int i = 0; i < 0x30; i++) arena<uint32_t>(0x100)[i] = r.next();
                                float *pos = (float *)(self + 0x80), *face = (float *)(self + 0x70), *t = arena<float>(0x40);
                                for (int c = 0; c < 3; c++) pos[c] = r.real(-30, 30), face[c] = r.real(-1, 1);
                                const int shape = r.range(0, 5);
                                const float sc = shape == 0 ? 0.0f : shape == 1 ? 0.001f : r.real(0.1f, 20);
                                for (int c = 0; c < 3; c++) t[c] = pos[c] + r.real(-1, 1) * sc;
                                t[3] = r.real(0, r.range(0, 1) ? 0.5f : 10);   // step
                                t[4] = r.range(0, 3) == 0 ? 0.0f : r.real(-2, 12);   // stop
                            },
                            [](void *fn) {
                                const float *t = arena<float>(0x40);
                                return Result{((uint32_t(__thiscall *)(void *, const void *, float, float))fn)(arena<void>(0x100), t, t[3], t[4])};
                            }};
    quick(person_swim_toward);
    // Curves::StepC213B0: counts below, at and past the stored one, weights below and past 1, w of 0 or less at times
    Test curves_step_c213b0{0xc213b0,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next();
                                float *f = (float *)self;
                                for (int i = 4; i <= 10; i++) f[i] = r.real(-50, 50);
                                f[12] = r.range(0, 4) == 0 ? r.real(1, 3) : r.real(0, 1);
                                *(int32_t *)(self + 0x34) = r.range(-3, 6);
                                for (int c = 0; c < 3; c++) arena<float>(0x40)[c] = r.real(-50, 50);
                                arena<int32_t>(0x50)[0] = r.range(-3, 6);
                                arena<float>(0x50)[1] = r.range(0, 4) == 0 ? -r.real(0, 1) * r.range(0, 1) : r.real(0, 3);
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(void *, const void *, int32_t, float))fn)(arena<void>(0x100), arena<void>(0x40), arena<int32_t>(0x50)[0], arena<float>(0x50)[1]);
                                return Result{0};
                            }};
    quick(curves_step_c213b0);
    // Effect::Length414630: 0-5 triangles from a pool of 6 points (repeated points at times: normals of no length)
    Test effect_length_414630{0x414630,
                              [=](Rng &r) {
                                  uint8_t *self = arena<uint8_t>(0x100);
                                  const int n = r.range(0, 5);
                                  *(uint32_t *)(self + 0x94) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                  *(int32_t *)(self + 0x98) = r.range(0, 9) ? n : -r.range(0, 3);
                                  float *pts = arena<float>(0x400);
                                  for (int i = 0; i < 18; i++) pts[i] = r.real(-10, 10);
                                  for (int i = 0; i < n; i++) {
                                      uint32_t *e = arena<uint32_t>(0x200 + i * 0x20);
                                      for (int k = 0; k < 3; k++) e[k] = (uint32_t)(uintptr_t)(pts + 3 * r.range(0, r.range(0, 3) ? 5 : 1));
                                      for (int k = 3; k < 8; k++) e[k] = r.next();
                                  }
                              },
                              [](void *fn) {
                                  ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                                  return Result{0};
                              }};
    quick(effect_length_414630);
    // Effect::Pass414320: 0-6 points near p (on it at times), k either side of their distances
    Test effect_pass_414320{0x414320,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                const int n = r.range(0, 6);
                                *(uint32_t *)(self + 0x14) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                *(int32_t *)(self + 0x20) = r.range(0, 9) ? n : -r.range(0, 3);
                                float *p = arena<float>(0x40);
                                for (int c = 0; c < 3; c++) p[c] = r.real(-20, 20);
                                for (int i = 0; i < n; i++) {
                                    float *e = arena<float>(0x200 + i * 0x50);
                                    for (int k = 0; k < 20; k++) arena<uint32_t>(0x200 + i * 0x50)[k] = r.next();
                                    const float s = r.range(0, 6) == 0 ? 0.0f : r.real(0, 8);
                                    for (int c = 0; c < 3; c++) e[c] = p[c] + r.real(-1, 1) * s;
                                }
                                p[3] = r.real(0, 10);
                            },
                            [](void *fn) {
                                const float *p = arena<float>(0x40);
                                ((void(__thiscall *)(void *, uint32_t, const void *, float))fn)(arena<void>(0x100), 0, p, p[3]);
                                return Result{0};
                            }};
    quick(effect_pass_414320);
    // Fence::NearA90E70: outlines of 2-8 points (an edge of no length at times), every edge, points beside, before and past it
    Test fence_near_a90e70{0xa90e70,
                           [=](Rng &r) {
                               const int n = r.range(2, 8);
                               uint32_t *list = arena<uint32_t>(0x100);
                               list[0] = (uint32_t)(uintptr_t)arena<void>(0x200), list[1] = (uint32_t)n;
                               float *pt = arena<float>(0x200);
                               for (int k = 0; k < 2 * n; k++) pt[k] = r.real(-10, 10);
                               const int i = r.range(0, n - 1), j = i + 1 < n ? i + 1 : 0;
                               if (r.range(0, 6) == 0) pt[2 * j] = pt[2 * i], pt[2 * j + 1] = pt[2 * i + 1] + (r.range(0, 1) ? 0.0f : 1e-5f);
                               float *a = arena<float>(0x40);
                               const float t = r.real(-0.5f, 1.5f);
                               a[0] = pt[2 * i] + t * (pt[2 * j] - pt[2 * i]) + r.real(-3, 3);
                               a[1] = pt[2 * i + 1] + t * (pt[2 * j + 1] - pt[2 * i + 1]) + r.real(-3, 3);
                               arena<int32_t>(0x50)[0] = i;
                               arena<uint32_t>(0x60)[0] = r.next(), arena<uint32_t>(0x60)[1] = r.next();
                           },
                           [](void *fn) {
                               const float *a = arena<float>(0x40);
                               return Result{(uint32_t)(uintptr_t)((void *(__stdcall *)(void *, void *, float, float, int32_t))fn)(arena<void>(0x60), arena<void>(0x100), a[0], a[1], arena<int32_t>(0x50)[0])};
                           }};
    quick(fence_near_a90e70);
    // Terrain::Near8F2380: grids of 0-12 entries around p, inner and outer radii (equal or crossed at times)
    Test terrain_near_8f2380{0x8f2380,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100);
                                 for (int i = 0; i < 8; i++) arena<uint32_t>(0x100)[i] = r.next();
                                 *(int32_t *)(self + 8) = r.range(-1, 3), *(int32_t *)(self + 0xc) = r.range(0, 3);
                                 *(uint32_t *)(self + 0x14) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                 *(uint32_t *)(self + 0x1c) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                 float *p = arena<float>(0x40);
                                 p[0] = r.real(-20, 20), p[1] = r.real(-20, 20);
                                 for (int k = 0; k < 12; k++) {
                                     float *e = arena<float>(0x200 + k * 0x10);
                                     for (int c = 0; c < 4; c++) arena<uint32_t>(0x200 + k * 0x10)[c] = r.next();
                                     e[0] = p[0] + r.real(-12, 12), e[2] = p[1] + r.real(-12, 12);
                                     arena<uint32_t>(0x400)[k] = r.next();
                                 }
                                 p[2] = r.real(0, 6);
                                 const int shape = r.range(0, 7);
                                 p[3] = shape == 0 ? p[2] : shape == 1 ? r.real(0, p[2]) : p[2] + r.real(0, 10);
                             },
                             [](void *fn) {
                                 const float *p = arena<float>(0x40);
                                 ((void(__thiscall *)(void *, const void *, float, float))fn)(arena<void>(0x100), p, p[2], p[3]);
                                 return Result{0};
                             }};
    quick(terrain_near_8f2380);
    // Vector::NearCDB2A0: rays from inside, outside and past spheres (aimed at the centre at times), the registers set as
    // the game passes them; only the low byte of the result is the game's
    Test vector_near_cdb2a0{0xcdb2a0,
                            [=](Rng &r) {
                                float *v = arena<float>(0x100);   // a 0x100, d 0x110, c 0x120, r 0x130, out 0x140
                                for (int i = 0; i < 12; i++) v[i] = r.real(-10, 10);

                                for (int c = 0; c < 3; c++) v[4 + c] = r.real(-1, 1);
                                if (r.range(0, 3) == 0)
                                    for (int c = 0; c < 3; c++) v[4 + c] = v[8 + c] - v[c];
                                v[12] = r.real(0, 12);
                                arena<uint32_t>(0x140)[0] = r.next();
                            },
                            [](void *fn) {
                                uint32_t stack[3] = {(uint32_t)(uintptr_t)arena<void>(0x120), arena<uint32_t>(0x130)[0], (uint32_t)(uintptr_t)arena<void>(0x140)};
                                uint32_t *ps = stack;
                                void *a = arena<void>(0x100), *d = arena<void>(0x110);
                                __asm__ volatile("pushl 8(%%eax)\n\tpushl 4(%%eax)\n\tpushl (%%eax)\n\tcall *%%ebx"
                                                 : "+a"(ps), "+c"(a), "+d"(d)
                                                 : "b"(fn)
                                                 : "memory", "cc");
                                return Result{(uint32_t)(uintptr_t)ps & 0xff};
                            }};
    quick(vector_near_cdb2a0);
    // Animal::DirectionA60E40: points near and far apart (the same at times), steps either side of the distance
    Test animal_direction_a60e40{0xa60e40,
                                 [=](Rng &r) {
                                     float *v = arena<float>(0x100);   // out 0x100, to 0x110, from 0x120, a k 0x130
                                     for (int i = 0; i < 4; i++) arena<uint32_t>(0x100)[i] = r.next();
                                     for (int c = 0; c < 3; c++) v[8 + c] = r.real(-20, 20);
                                     const float s = r.range(0, 6) == 0 ? 0.0f : r.real(0, r.range(0, 1) ? 1.0f : 30.0f);
                                     for (int c = 0; c < 3; c++) v[4 + c] = v[8 + c] + r.real(-1, 1) * s;
                                     v[12] = r.real(0, 2), v[13] = r.range(0, 5) == 0 ? -r.real(0, 1) : r.real(0, 3);
                                 },
                                 [](void *fn) {
                                     uint32_t stack[3] = {(uint32_t)(uintptr_t)arena<void>(0x120), arena<uint32_t>(0x130)[0], arena<uint32_t>(0x130)[1]};
                                     uint32_t *ps = stack;
                                     void *o = arena<void>(0x100), *t = arena<void>(0x110);
                                     __asm__ volatile("pushl 8(%%eax)\n\tpushl 4(%%eax)\n\tpushl (%%eax)\n\tcall *%%ebx"
                                                      : "+a"(ps), "+c"(o), "+d"(t)
                                                      : "b"(fn)
                                                      : "memory", "cc");
                                     return Result{0};
                                 }};
    quick(animal_direction_a60e40);
    // Effect::Normalise7B3B20: weights either side of 0, 0-6 points around the base and centre
    Test effect_normalise_7b3b20{0x7b3b20,
                                 [=](Rng &r) {
                                     uint8_t *self = arena<uint8_t>(0x100);
                                     float *f = (float *)self;
                                     for (int i = 0; i < 0x18; i++) arena<uint32_t>(0x100)[i] = r.next();
                                     for (int i = 0; i < 7; i++) f[i] = r.real(-10, 10);
                                     f[0x44 / 4] = r.range(0, 5) == 0 ? -r.real(0, 2) * r.range(0, 1) : r.real(0.1f, 5);
                                     f[0x48 / 4] = r.real(0, 3);
                                     const int n = r.range(0, 6);
                                     *(int32_t *)(self + 0x4c) = r.range(0, 9) ? n : -1;
                                     *(uint32_t *)(self + 0x50) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                     for (int i = 0; i < n * 4; i++) arena<float>(0x200)[i] = r.real(-12, 12);
                                 },
                                 [](void *fn) {
                                     ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                                     return Result{0};
                                 }};
    quick(effect_normalise_7b3b20);
    // Grid::SnapToRow: row heights of 0.5-4 (set in the game's global, put back after), heights below 0, inside and
    // past the last row, first values either side of the minimum
    {
        static uint32_t saved_row_height;
        Test x{0x99c2a0,
               [=](Rng &r) {
                   *(float *)0x13413b0 = r.real(0.5f, 4);
                   uint8_t *self = arena<uint8_t>(0x100);
                   *(int32_t *)(self + 0x108) = r.range(0, 40);
                   float *p = arena<float>(0x40);
                   p[0] = r.real(-60, 60), p[1] = r.real(-20, 150);
                   arena<uint32_t>(0x60)[0] = r.next(), arena<uint32_t>(0x60)[1] = r.next();
               },
               [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *, const void *))fn)(arena<void>(0x100), arena<void>(0x60), arena<void>(0x40))}; }};
        x.prepare = [](const rewrite::Entry *) { saved_row_height = *(uint32_t *)0x13413b0; };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x13413b0 = saved_row_height; };
        quick(x);
    }
    // Segment::MiddleOfEnds: two lists of 1-4 points each (their last points used)
    Test segment_middle_of_ends{0xbc61e0,
                                [=](Rng &r) {
                                    uint8_t *self = arena<uint8_t>(0x100);
                                    *(uint32_t *)(self + 0x18) = r.next() | 0x400000;
                                    *(int32_t *)(self + 0xe4) = r.range(2, 5);
                                    uint32_t *l1 = arena<uint32_t>(0x40), *l2 = arena<uint32_t>(0x50);
                                    *(uint32_t *)(self + 0x10c) = (uint32_t)(uintptr_t)l1, *(uint32_t *)(self + 0x110) = (uint32_t)(uintptr_t)l2;
                                    l1[0] = r.range(1, 4), l1[1] = (uint32_t)(uintptr_t)arena<void>(0x400);
                                    l2[0] = r.range(1, 4), l2[1] = (uint32_t)(uintptr_t)arena<void>(0x600);
                                    for (int i = 0; i < 36; i++) arena<float>(0x400)[i] = r.real(-50, 50), arena<float>(0x600)[i] = r.real(-50, 50);
                                    arena<uint32_t>(0x60)[0] = r.next();
                                },
                                [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x60))}; }};
    quick(segment_middle_of_ends);
    // Mover::StepBack: the game's time scale and node list set (and put back after), nodes in and out of range (a list
    // at 0 at times), heights either side of the node's
    {
        static uint32_t saved_scale, saved_nodes;
        Test x{0x913620,
               [=](Rng &r) {
                   *(float *)0x10f0b88 = r.real(0.5f, 4);
                   uint8_t *nodes = arena<uint8_t>(0x40);
                   *(uint32_t *)0x133a0f4 = (uint32_t)(uintptr_t)nodes;
                   *(int32_t *)(nodes + 0x14) = r.range(0, 4);
                   const bool none = r.range(0, 9) == 0;
                   *(uint32_t *)(nodes + 0xc) = none ? 0 : (uint32_t)(uintptr_t)arena<void>(0x400);
                   for (int i = 0; i < 4 * 0x98 / 4; i++) arena<float>(0x400)[i] = r.real(-20, 20);
                   uint8_t *self = arena<uint8_t>(0x100);
                   for (int i = 0; i < 0x1c; i++) arena<uint32_t>(0x100)[i] = r.next();
                   float *f = (float *)self;
                   f[1] = r.real(-20, 20), f[2] = r.real(-20, 20);
                   f[0x58 / 4] = r.real(-5, 5), f[0x64 / 4] = r.real(-3, 3), f[0x68 / 4] = r.real(-3, 3);
                   *(int32_t *)(self + 0x60) = none ? 0 : r.range(-1, 4);
                   arena<float>(0x60)[0] = r.real(-2, 2);
               },
               [](void *fn) {
                   ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x60)[0]);
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { saved_scale = *(uint32_t *)0x10f0b88, saved_nodes = *(uint32_t *)0x133a0f4; };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x10f0b88 = saved_scale, *(uint32_t *)0x133a0f4 = saved_nodes; };
        quick(x);
    }
    // Window::ScreenPosition: chains of 0-4 parents, scroll flags on some children
    Test window_screen_position{0x6ad560,
                                [=](Rng &r) {
                                    const int n = r.range(0, 4);
                                    for (int k = 0; k <= n; k++) {
                                        uint8_t *w = arena<uint8_t>(0x200 + k * 0xa0);
                                        for (int i = 0; i < 0x28; i++) arena<uint32_t>(0x200 + k * 0xa0)[i] = r.next();
                                        for (uint32_t o : {0x7cu, 0x80u, 0x8cu, 0x90u}) *(float *)(w + o) = r.real(-500, 500);
                                        *(uint32_t *)(w + 0x3c) = k < n ? (uint32_t)(uintptr_t)arena<void>(0x200 + (k + 1) * 0xa0) : 0;
                                    }
                                    arena<uint32_t>(0x60)[0] = r.next(), arena<uint32_t>(0x60)[1] = r.next();
                                },
                                [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *))fn)(arena<void>(0x200), arena<void>(0x60))}; }};
    quick(window_screen_position);
    // Fog::Setup: distances either side of the small limit, no object, objects with and without the single value
    Test fog_setup{0x7c3d50,
                   [=](Rng &r) {
                       uint8_t *o = arena<uint8_t>(0x200);
                       for (int i = 0; i < 0x70; i++) arena<uint32_t>(0x200)[i] = r.next();
                       o[0x1a6] = r.range(0, 1) ? 0 : (uint8_t)r.range(1, 255);
                       arena<uint32_t>(0x40)[0] = r.range(0, 3) ? (uint32_t)(uintptr_t)o : 0;
                       arena<float>(0x40)[1] = r.range(0, 4) == 0 ? r.real(-1e-6f, 2e-6f) : r.real(-5, 500);
                   },
                   [](void *fn) {
                       ((void(__thiscall *)(void *, float))fn)((void *)(uintptr_t)arena<uint32_t>(0x40)[0], arena<float>(0x40)[1]);
                       return Result{0};
                   }};
    t.push_back(fog_setup);   // (its writes are game globals: the data section is compared)
    // PathGrid::SetEdges / ClearEdges: grids of 3-6 cells a side, inner cells (every neighbour there), any bits
    for (uintptr_t at : {(uintptr_t)0xb9a810, (uintptr_t)0xb9a8c0}) {
        Test x{at,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   const int w = r.range(3, 6), h = r.range(3, 6);
                   *(int32_t *)(self + 0x64) = w;
                   *(uint32_t *)(self + 0x6c) = (uint32_t)(uintptr_t)arena<void>(0x400);
                   for (int i = 0; i < 36 * 6; i++) arena<uint32_t>(0x400)[i] = r.next();
                   arena<int32_t>(0x40)[0] = r.range(1, w - 2), arena<int32_t>(0x40)[1] = r.range(1, h - 2);
                   arena<uint32_t>(0x40)[2] = r.next();
               },
               [](void *fn) {
                   const int32_t *a = arena<int32_t>(0x40);
                   ((void(__thiscall *)(void *, int32_t, int32_t, uint32_t))fn)(arena<void>(0x100), a[0], a[1], (uint32_t)a[2]);
                   return Result{0};
               }};
        quick(x);
    }
    // KeyMap::Find: 1-4 buckets over 0-8 entries, keys copied from an entry (one value changed at times) or random
    Test keymap_find{0xc477d0,
                     [=](Rng &r) {
                         uint8_t *self = arena<uint8_t>(0x100);
                         const uint32_t nb = r.range(1, 4), n = r.range(0, 8);
                         *(uint32_t *)(self + 4) = nb;
                         uint32_t *buckets = arena<uint32_t>(0x140);
                         *(uint32_t *)(self + 0xc) = (uint32_t)(uintptr_t)buckets;
                         *(uint32_t *)(self + 0x10) = (uint32_t)(uintptr_t)arena<void>(0x400);
                         *(uint32_t *)(self + 0x14) = (uint32_t)(uintptr_t)arena<void>(0x180);
                         buckets[0] = 0;
                         for (uint32_t b = 1; b <= nb; b++) buckets[b] = b == nb ? n : r.range(buckets[b - 1], n);
                         uint32_t *e = arena<uint32_t>(0x400);
                         for (uint32_t i = 0; i < 12 * n; i++) e[i] = r.range(0, 3) ? r.range(0, 3) : r.next();
                         uint32_t *key = arena<uint32_t>(0x200);
                         if (n && r.range(0, 3)) {
                             const uint32_t pick = r.range(0, n - 1);
                             for (int k = 0; k < 12; k++) key[k] = e[pick * 12 + k];
                             if (r.range(0, 3) == 0) key[r.range(0, 11)] ^= 1u << r.range(0, 31);
                         } else
                             for (int k = 0; k < 12; k++) key[k] = r.range(0, 3);
                     },
                     [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, const void *))fn)(arena<void>(0x100), arena<void>(0x200))}; }};
    quick(keymap_find);
    // Fireworks::Step99C660 / Step99C730: 0-12 signed values (word or byte), v anywhere along the table, entries random
    for (uintptr_t at : {(uintptr_t)0x99c660, (uintptr_t)0x99c730}) {
        Test x{at,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100), *o = arena<uint8_t>(0x200);
                   *(uint32_t *)(self + 0x134) = (uint32_t)(uintptr_t)o;
                   *(uint32_t *)(o + 0xd4) = (uint32_t)(uintptr_t)arena<void>(0x4000);
                   *(float *)(o + 0xe0) = r.real(0.5f, 8);
                   float *e = arena<float>(0x4000);
                   for (int i = 0; i < 0x9c48 / 4; i++) e[i] = r.range(0, 3) ? r.real(-1, 1) : 0.0f;
                   for (int i = 0; i < 24; i++) arena<uint8_t>(0x40)[i] = (uint8_t)r.next();
                   arena<int32_t>(0x60)[0] = r.range(0, 9) ? r.range(0, 12) : -1;
                   arena<float>(0x60)[1] = r.real(0, *(float *)(o + 0xe0));
               },
               [](void *fn) {
                   return Result{((uint32_t(__thiscall *)(void *, const void *, int32_t, uint32_t, uint32_t, float))fn)(arena<void>(0x100), arena<void>(0x40), arena<int32_t>(0x60)[0], 0x1234, 0x5678, arena<float>(0x60)[1])};
               }};
        quick(x);
    }
    // Fence::CellA906C0: points around and past the 4 x 4 square (cells of a quarter), random bit grids
    Test fence_cell_a906c0{0xa906c0,
                           [=](Rng &r) {
                               for (int i = 0; i < 10; i++) arena<uint32_t>(0x100)[i] = r.range(0, 3) ? r.next() : 0;
                               float *p = arena<float>(0x40);
                               for (int c = 0; c < 3; c++) p[c] = r.real(-2.6f, 2.6f);
                               if (r.range(0, 5) == 0) p[0] = (float)r.range(-10, 10) / 4, p[2] = (float)r.range(-10, 10) / 4;
                           },
                           [](void *fn) { return Result{((uint32_t(__stdcall *)(const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40)) & 0xff}; }};
    quick(fence_cell_a906c0);
    // Animal::SumB81E50: the game's two scales set (and put back after), sums and x around every level's edge, types 0-5
    {
        static uint32_t saved_a, saved_b;
        Test x{0xb81e50,
               [=](Rng &r) {
                   *(float *)0x10ed8c4 = r.real(0, 2), *(float *)0x10ed8d8 = r.real(0, 2);
                   uint8_t *self = arena<uint8_t>(0x100), *rec = arena<uint8_t>(0x200), *type = arena<uint8_t>(0x240);
                   *(uint32_t *)(self + 0xa4) = (uint32_t)(uintptr_t)rec;
                   *(uint32_t *)rec = (uint32_t)(uintptr_t)type;
                   type[4] = (uint8_t)r.range(0, 5);
                   for (uint32_t o : {8u, 0xcu, 0x10u}) *(float *)(rec + o) = r.real(-1, 3);
                   arena<float>(0x40)[0] = r.range(0, 3) ? r.real(0, 400) : r.real(-50, 50);
               },
               [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x40)[0])}; }};
        x.prepare = [](const rewrite::Entry *) { saved_a = *(uint32_t *)0x10ed8c4, saved_b = *(uint32_t *)0x10ed8d8; };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x10ed8c4 = saved_a, *(uint32_t *)0x10ed8d8 = saved_b; };
        quick(x);
    }
    // Animal::AddTimeB828D0: 0-6 entries of four kinds, times below 0, inside and past their periods
    Test animal_add_time_b828d0{0xb828d0,
                                [=](Rng &r) {
                                    uint8_t *self = arena<uint8_t>(0x100), *rec = arena<uint8_t>(0x200), *kinds = arena<uint8_t>(0x240);
                                    *(uint32_t *)(self + 0xa4) = (uint32_t)(uintptr_t)rec;
                                    *(uint32_t *)(rec + 4) = (uint32_t)(uintptr_t)kinds;
                                    *(uint32_t *)(kinds + 0x10) = (uint32_t)(uintptr_t)arena<void>(0x280);
                                    for (int k = 0; k < 4; k++) arena<float>(0x280)[k] = r.real(0.2f, 5);
                                    const int n = r.range(0, 6);
                                    *(int32_t *)(self + 8) = r.range(0, 9) ? n : -1;
                                    *(uint32_t *)(self + 4) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                    for (int i = 0; i < n; i++) {
                                        uint32_t *e = arena<uint32_t>(0x400 + i * 0x14);
                                        for (int k = 0; k < 5; k++) e[k] = r.next();
                                        e[0] = r.range(0, 3);
                                        *(float *)(e + 1) = r.real(-12, 12);
                                    }
                                    arena<float>(0x40)[0] = r.real(-3, 3);
                                },
                                [](void *fn) {
                                    ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x40)[0]);
                                    return Result{0};
                                }};
    quick(animal_add_time_b828d0);
    // WildAnimal::PointB18E70: 0-8 points (some off), near and far from p
    Test wild_animal_point_b18e70{0xb18e70,
                                  [=](Rng &r) {
                                      uint8_t *self = arena<uint8_t>(0x1000);
                                      const int n = r.range(0, 8);
                                      *(int32_t *)(self + 0x108) = r.range(0, 9) ? n : -1;
                                      for (int i = 0; i < 8; i++) {
                                          float *q = (float *)(self + 0x110 + i * 0x10);
                                          for (int c = 0; c < 4; c++) q[c] = r.real(-300, 300) * (r.range(0, 2) ? 0.02f : 1);
                                          *(uint32_t *)(self + 0x440 + i * 4) = r.range(0, 3) ? r.next() | 1 : 0;
                                      }
                                      float *p = arena<float>(0x40);
                                      for (int c = 0; c < 3; c++) p[c] = r.real(-6, 6);
                                      for (int c = 0; c < 3; c++) arena<uint32_t>(0x60)[c] = r.next();
                                  },
                                  [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *, const void *))fn)(arena<void>(0x1000), arena<void>(0x60), arena<void>(0x40))}; }};
    quick(wild_animal_point_b18e70);
    // Vector::LengthBECFA0: random directions (along an axis, or of no length, at times) with x largest or not
    Test vector_length_becfa0{0xbecfa0,
                              [=](Rng &r) {
                                  for (int i = 0; i < 12; i++) arena<uint32_t>(0x100)[i] = r.next();
                                  float *v = arena<float>(0x40);
                                  for (int c = 0; c < 3; c++) v[c] = r.real(-5, 5);
                                  const int shape = r.range(0, 7);
                                  if (shape == 0) v[0] = v[1] = v[2] = 0;
                                  if (shape == 1) v[r.range(0, 2)] = 0, v[r.range(0, 2)] = 0;
                                  if (shape == 2) v[1] = v[0];
                                  if (shape == 3) v[0] = 1, v[1] = v[2] = 0;
                              },
                              [](void *fn) {
                                  ((void(__thiscall *)(void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40));
                                  return Result{0};
                              }};
    quick(vector_length_becfa0);
    // Water::Range907440: four grids of sizes 1-8 (equal, multiples and not), random samples, heights and spreads
    Test water_range_907440{0x907440,
                            [=](Rng &r) {
                                for (int g = 0; g < 4; g++) {
                                    uint8_t *grid = arena<uint8_t>(0x100 + g * 0x60);
                                    for (int i = 0; i < 0x18; i++) arena<uint32_t>(0x100 + g * 0x60)[i] = r.next();
                                    const int n = r.range(0, 2) ? 1 << r.range(0, 3) : r.range(1, 8);
                                    *(int32_t *)grid = n, *(int32_t *)(grid + 4) = r.range(0, 7);
                                    *(float *)(grid + 0x34) = r.real(-2, 2);
                                    const uintptr_t base = 0x1000 + g * 0x800;
                                    *(uint32_t *)(grid + 0x40) = (uint32_t)(base);
                                    *(uint32_t *)(grid + 0x4c) = (uint32_t)(base + 0x200);
                                    *(uint32_t *)(grid + 0x50) = (uint32_t)(base + 0x400);
                                    for (int i = 0; i < 0x600 / 4; i++) arena<float>(base)[i] = r.real(-10, 10);
                                }
                                for (int g = 0; g < 4; g++) {   // the arrays as game addresses
                                    uint8_t *grid = arena<uint8_t>(0x100 + g * 0x60);
                                    for (uint32_t o : {0x40u, 0x4cu, 0x50u}) *(uint32_t *)(grid + o) = (uint32_t)(uintptr_t)arena<void>(*(uint32_t *)(grid + o));
                                }
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(void *, void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x160), arena<void>(0x1c0), arena<void>(0x220));
                                return Result{0};
                            }};
    quick(water_range_907440);
    // List::PassC236C0: a chain of 8 nodes of random words (signalling NaNs among them), 0-3 skipped, 0-4 copied
    Test list_pass_c236c0{0xc236c0,
                          [=](Rng &r) {
                              for (int k = 0; k < 8; k++) {
                                  uint32_t *n = arena<uint32_t>(0x1000 + k * 0x100);
                                  for (int i = 0; i < 0x40; i++) n[i] = r.range(0, 7) ? r.next() : 0x7f800001u + (r.next() & 0x3fffff);
                                  n[0x54 / 4] = (uint32_t)(uintptr_t)arena<void>(0x1000 + (k + 1) * 0x100);
                              }
                              for (int i = 0; i < 0x380 / 4; i++) arena<uint32_t>(0x400)[i] = r.next();
                              arena<int32_t>(0x40)[0] = r.range(-1, 3), arena<int32_t>(0x40)[1] = r.range(0, 4);
                          },
                          [](void *fn) {
                              return Result{(uint32_t)(uintptr_t)((void *(__stdcall *)(void *, const void *, const void *))fn)(arena<void>(0x400), arena<void>(0x1000), arena<void>(0x40))};
                          }};
    quick(list_pass_c236c0);
    // Animal::LoopB788A0: lists of 1-6 nodes over random records, always one value under 1000 (ties at times); the game
    // copies out stack bytes it never set when none is
    Test animal_loop_b788a0{0xb788a0,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                const int n = r.range(1, 6);
                                const float tie = r.real(0, 900);
                                for (int k = 0; k < n; k++) {
                                    uint32_t *node = arena<uint32_t>(0x200 + k * 0x10), *rec = arena<uint32_t>(0x400 + k * 0x60);
                                    node[0] = k + 1 < n ? (uint32_t)(uintptr_t)arena<void>(0x200 + (k + 1) * 0x10) : (uint32_t)(uintptr_t)(self + 0x28);
                                    node[2] = (uint32_t)(uintptr_t)rec;
                                    for (int i = 0; i < 0x18; i++) rec[i] = r.next();
                                    *(float *)&rec[0] = r.real(-50, 50);
                                    *(float *)&rec[2] = r.range(0, 3) == 0 ? tie : r.real(-100, 1500);
                                }
                                *(float *)arena<uint8_t>(0x400 + r.range(0, n - 1) * 0x60 + 8) = r.real(0, 999);
                                *(uint32_t *)(self + 0x1c) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                for (int i = 0; i < 0x18; i++) arena<uint32_t>(0x800)[i] = r.next();
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x800));
                                return Result{0};
                            }};
    quick(animal_loop_b788a0);
    // Renderer::SetMaterial: the same material or a new one, no texture set at times, 0-6 textures (more would run the
    // count into its own bytes)
    Test renderer_set_material{0xc946d0,
                               [=](Rng &r) {
                                   uint8_t *self = arena<uint8_t>(0x1000), *m = arena<uint8_t>(0x100), *set = arena<uint8_t>(0x200);
                                   for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x1000 + 0x1cb0)[i] = r.next(), arena<uint32_t>(0x1000 + 0x2140)[i] = r.next();
                                   for (int i = 0; i < 0x50; i++) arena<uint32_t>(0x200)[i] = r.next();
                                   *(uint32_t *)(m + 0x28) = r.range(0, 5) ? (uint32_t)(uintptr_t)set : 0;
                                   *(uint16_t *)(set + 0xca) = (uint16_t)r.range(0, 6);   // (from 7 on the game overwrites the count with 0xFFFF)
                                   if (r.range(0, 4) == 0) *(uint32_t *)(self + 0x2140) = (uint32_t)(uintptr_t)m;
                               },
                               [](void *fn) {
                                   ((void(__thiscall *)(void *, const void *))fn)(arena<void>(0x1000), arena<void>(0x100));
                                   return Result{0};
                               }};
    quick(renderer_set_material);
    // Track::InitBB83A0: random sources (byte 1 set or not, +0x14 tiny, negative or larger), an owner place or none
    Test track_init_bb83a0{0xbb83a0,
                           [=](Rng &r) {
                               for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next(), arena<uint32_t>(0x200)[i] = r.next();
                               arena<uint8_t>(0x200)[1] = r.range(0, 1) ? 0 : (uint8_t)r.next();
                               const int shape = r.range(0, 3);
                               *(float *)arena<uint8_t>(0x214) = shape == 0 ? 1e-39f : shape == 1 ? -r.real(0, 5) : r.real(0, 5);
                               arena<uint32_t>(0x40)[0] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x60) : 0;
                               arena<uint32_t>(0x40)[1] = r.next(), arena<uint32_t>(0x40)[2] = r.next();
                               arena<uint32_t>(0x60)[0] = r.next();
                           },
                           [](void *fn) {
                               const uint32_t *a = arena<uint32_t>(0x40);
                               return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *, uint32_t, const void *, uint32_t))fn)(arena<void>(0x100), (void *)(uintptr_t)a[0], a[1], arena<void>(0x200), a[2])};
                           }};
    quick(track_init_bb83a0);
    // Effects::Entry455040: entries of kinds 0-4, points near and far
    Test effects_entry_455040{0x455040,
                              [=](Rng &r) {
                                  *arena<uint32_t>(0x100) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                  for (int i = 0; i < 4 * 0x90 / 4; i++) arena<float>(0x200)[i] = r.real(-20, 20);
                                  for (int k = 0; k < 4; k++) arena<int32_t>(0x200 + k * 0x90)[0] = r.range(0, 4);
                                  arena<int32_t>(0x40)[0] = r.range(0, 3);
                                  for (int c = 0; c < 3; c++) arena<float>(0x50)[c] = r.real(-20, 20);
                              },
                              [](void *fn) {
                                  Result x;
                                  x.f = ((long double(__thiscall *)(void *, int32_t, const void *, uint32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<void>(0x50), 7);
                                  return x;
                              }};
    quick(effects_entry_455040);
    // KeyMap::Clear / Table::Clear: 0-6 entries, 0-8 buckets, random contents
    for (uintptr_t at : {(uintptr_t)0xc47750, (uintptr_t)0xc47490}) {
        Test x{at,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   *(uint32_t *)(self + 8) = r.range(0, 6), *(uint32_t *)(self + 4) = r.range(0, 8);
                   *(uint32_t *)(self + 0xc) = (uint32_t)(uintptr_t)arena<void>(0x200);
                   *(uint32_t *)(self + 0x10) = (uint32_t)(uintptr_t)arena<void>(0x400);
                   *(uint32_t *)(self + 0x14) = (uint32_t)(uintptr_t)arena<void>(0x300);
                   for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x200)[i] = r.next();
                   for (int i = 0; i < 0x50; i++) arena<uint32_t>(0x400)[i] = r.next();
               },
               [](void *fn) {
                   ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                   return Result{0};
               }};
        quick(x);
    }
    // Entries::Remove: 0-6 entries with ids from a few (repeats), ids there (first, middle, last) or not
    Test entries_remove{0x940a20,
                        [=](Rng &r) {
                            uint8_t *self = arena<uint8_t>(0x100);
                            const int n = r.range(0, 6);
                            *(int32_t *)(self + 8) = r.range(0, 9) ? n : -1;
                            *(uint32_t *)self = (uint32_t)(uintptr_t)arena<void>(0x200);
                            for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x200)[i] = r.next();
                            for (int i = 0; i < n; i++) arena<uint32_t>(0x200)[i * 4] = r.range(0, 5);
                            arena<uint32_t>(0x40)[0] = r.range(0, 6);
                        },
                        [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0]) & 0xff}; }};
    quick(entries_remove);
    // Effects::Value4377E0: types of every size rule (kinds 0-31), random sources, places in the record
    Test effects_value_4377e0{0x4377e0,
                              [=](Rng &r) {
                                  uint8_t *self = arena<uint8_t>(0x100);
                                  *(uint32_t *)self = (uint32_t)(uintptr_t)arena<void>(0x140);
                                  *(uint32_t *)arena<uint8_t>(0x154) = (uint32_t)(uintptr_t)arena<void>(0x1000);
                                  self[8] = (uint8_t)r.range(0, 31);
                                  const int h = r.range(0, 5);
                                  self[9] = h == 0 ? 0 : h == 1 ? 1 : h == 2 ? (uint8_t)r.range(2, 0x7f) : (uint8_t)r.range(0x80, 0x84);
                                  *(uint16_t *)(self + 0xa) = (uint16_t)r.range(0, 0x200);
                                  for (int i = 0; i < 0x200; i++) arena<uint32_t>(0x1000)[i] = r.next();
                                  for (int i = 0; i < 0x60; i++) arena<uint32_t>(0x400)[i] = r.next();
                              },
                              [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t, const void *))fn)(arena<void>(0x100), 9, arena<void>(0x400))}; }};
    quick(effects_value_4377e0);
    // UIntSlider::SetValue: bounds anywhere (past 2^31, crossed at times), fractions below 0, inside and past 1
    Test uint_slider_set_value{0xca8560,
                               [=](Rng &r) {
                                   uint8_t *self = arena<uint8_t>(0x100);
                                   *(uint32_t *)(self + 4) = (uint32_t)(uintptr_t)arena<void>(0x140);
                                   const uint32_t a = r.range(0, 2) ? r.range(0, 1000) : r.next(), b = r.range(0, 2) ? r.range(0, 1000) : r.next();
                                   *(uint32_t *)(self + 8) = r.range(0, 4) ? (a < b ? a : b) : a;
                                   *(uint32_t *)(self + 0xc) = r.range(0, 4) ? (a < b ? b : a) : b;
                                   arena<uint32_t>(0x140)[0] = r.next();
                                   arena<float>(0x40)[0] = r.real(-0.3f, 1.3f);
                               },
                               [](void *fn) {
                                   ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x40)[0]);
                                   return Result{0};
                               }};
    quick(uint_slider_set_value);
    // WideString::FindLast: texts of 0-12 characters from a few, characters there or not, from -1, inside or past the end
    Test wide_string_find_last{0xbcef20,
                               [=](Rng &r) {
                                   uint32_t *s = arena<uint32_t>(0x140);
                                   arena<uint32_t>(0x100)[0] = (uint32_t)(uintptr_t)s;
                                   s[1] = r.range(0, 12), s[3] = (uint32_t)(uintptr_t)arena<void>(0x200);
                                   for (int i = 0; i < 16; i++) arena<uint16_t>(0x200)[i] = (uint16_t)(r.range(0, 5) ? 0x41 + r.range(0, 3) : r.next());
                                   arena<uint32_t>(0x40)[0] = (r.next() & ~0xffu) | (uint32_t)(0x41 + r.range(0, 4));
                                   const int f = r.range(0, 3);
                                   arena<uint32_t>(0x40)[1] = f == 0 ? 0xffffffffu : f == 1 ? r.range(0, 20) : r.range(0, 11);
                               },
                               [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(const void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0], arena<uint32_t>(0x40)[1])}; }};
    quick(wide_string_find_last);
    // TypeList::Copy: 0-4 random types (and a negative count at times) over a random list
    Test type_list_copy{0xbd2170,
                        [=](Rng &r) {
                            for (int i = 0; i < 0x14; i++) arena<uint32_t>(0x100)[i] = r.next();
                            for (int i = 0; i < 5; i++) arena<uint32_t>(0x200)[i] = r.next();
                            arena<int32_t>(0x200)[4] = r.range(0, 9) ? r.range(0, 4) : -r.range(1, 3);
                        },
                        [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, const void *))fn)(arena<void>(0x100), arena<void>(0x200))}; }};
    quick(type_list_copy);
    // TrackDesigner::CarSettingMode: the mode 0 or not, no car, values below, at and above 0
    Test track_designer_car_setting_mode{0x4ef080,
                                         [=](Rng &r) {
                                             uint8_t *self = arena<uint8_t>(0x1000);
                                             *(uint32_t *)(self + 0x810) = r.range(0, 4) ? r.next() | 1 : 0;
                                             const int v = r.range(0, 3);
                                             *(float *)arena<uint8_t>(0x100 + 0x1bc) = v == 0 ? 0.0f : v == 1 ? -0.0f : r.real(-5, 5);
                                             arena<uint32_t>(0x40)[0] = r.range(0, 5) ? (uint32_t)(uintptr_t)arena<void>(0x100) : 0;
                                         },
                                         [](void *fn) {
                                             ((void(__thiscall *)(void *, const void *))fn)(arena<void>(0x1000), (const void *)(uintptr_t)arena<uint32_t>(0x40)[0]);
                                             return Result{0};
                                         }};
    quick(track_designer_car_setting_mode);
    // Section::Section: a random section's room, a name with any count, any kind
    Test section_section{0x46fad0,
                         [=](Rng &r) {
                             for (int i = 0; i < 0x370 / 4; i++) arena<uint32_t>(0x1000)[i] = r.next();
                             arena<uint32_t>(0x40)[0] = (uint32_t)(uintptr_t)arena<void>(0x60);
                             arena<uint32_t>(0x60)[0] = r.next(), arena<uint32_t>(0x60)[1] = r.next();
                             arena<uint32_t>(0x40)[1] = r.next();
                         },
                         [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, const void *, uint32_t))fn)(arena<void>(0x1000), arena<void>(0x40), arena<uint32_t>(0x40)[1])}; }};
    quick(section_section);
    // ShortSlider::SetValue / UShortSlider::SetValue: any 16-bit bounds (crossed at times), fractions past both ends
    for (uintptr_t at : {(uintptr_t)0xca82c0, (uintptr_t)0xca83d0}) {
        Test x{at,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   *(uint32_t *)(self + 4) = (uint32_t)(uintptr_t)arena<void>(0x140);
                   const uint16_t a = (uint16_t)(r.range(0, 2) ? r.range(0, 500) : r.next()), b = (uint16_t)(r.range(0, 2) ? r.range(0, 500) : r.next());
                   *(uint16_t *)(self + 8) = a, *(uint16_t *)(self + 0xa) = b;
                   arena<uint32_t>(0x140)[0] = r.next();
                   arena<float>(0x40)[0] = r.real(-0.3f, 1.3f);
               },
               [](void *fn) {
                   ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x40)[0]);
                   return Result{0};
               }};
        quick(x);
    }
    // String::FindLastBCF540: texts of 0-12 bytes from a few, characters there or not, from -1, inside, at or past the end
    Test string_find_last_bcf540{0xbcf540,
                          [=](Rng &r) {
                              uint32_t *s = arena<uint32_t>(0x140);
                              arena<uint32_t>(0x100)[0] = (uint32_t)(uintptr_t)s;
                              s[1] = r.range(0, 12), s[3] = (uint32_t)(uintptr_t)arena<void>(0x200);
                              for (int i = 0; i < 24; i++) arena<uint8_t>(0x200)[i] = (uint8_t)(r.range(0, 5) ? 0x41 + r.range(0, 3) : r.next());
                              arena<uint32_t>(0x40)[0] = (r.next() & ~0xffu) | (uint32_t)(0x41 + r.range(0, 4));
                              const int f = r.range(0, 3);
                              arena<uint32_t>(0x40)[1] = f == 0 ? 0xffffffffu : f == 1 ? r.range(0, 20) : s[1];
                          },
                          [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(const void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0], arena<uint32_t>(0x40)[1])}; }};
    quick(string_find_last_bcf540);
    // Sound::VolumeToHundredthsDb / Db2: volumes below the floor, at it, between, at 1 and past it (and 0 or less)
    for (uintptr_t at : {(uintptr_t)0xce45f0, (uintptr_t)0xc193e0}) {
        Test x{at,
               [=](Rng &r) {
                   const int k = r.range(0, 6);
                   arena<float>(0x40)[0] = k == 0 ? 0.001f : k == 1 ? 1.0f : k == 2 ? r.real(-1, 0.001f) : k == 3 ? r.real(1, 3) : r.real(0.001f, 1);
               },
               [](void *fn) { return Result{(uint32_t)((int32_t(__stdcall *)(float))fn)(arena<float>(0x40)[0])}; }};
        quick(x);
    }
    // Fireworks::Size9FBD30: the game's halving flag set or not (put back after), random sizes and vectors
    {
        static uint32_t saved_flag;
        Test x{0x9fbd30,
               [=](Rng &r) {
                   *(uint32_t *)0x10edb90 = r.range(0, 1) ? r.next() | 1 : 0;
                   *(float *)arena<uint8_t>(0x174) = r.real(-5, 5);
                   for (int c = 0; c < 4; c++) arena<float>(0x40)[c] = r.real(-20, 20);
                   for (int c = 0; c < 4; c++) arena<uint32_t>(0x60)[c] = r.next();
               },
               [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, void *, const void *))fn)(arena<void>(0x100), arena<void>(0x60), arena<void>(0x40))}; }};
        x.prepare = [](const rewrite::Entry *) { saved_flag = *(uint32_t *)0x10edb90; };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x10edb90 = saved_flag; };
        quick(x);
    }
    // Water::Entry8FE240: entries 0-3 with the byte set or not, t within and past 0..1
    Test water_entry_8fe240{0x8fe240,
                            [=](Rng &r) {
                                *(uint32_t *)arena<uint8_t>(0x1a8) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                for (int i = 0; i < 4 * 0x18; i++) arena<float>(0x400)[i] = r.real(-30, 30);
                                for (int k = 0; k < 4; k++) {
                                    uint8_t *e = arena<uint8_t>(0x400 + k * 0x60);
                                    *(uint32_t *)(e + 0x38) = r.range(0, 1) ? r.next() & 0xff : 0;
                                    *(uint32_t *)(e + 0x4c) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                }
                                *(uint32_t *)arena<uint8_t>(0x20c) = (uint32_t)(uintptr_t)arena<void>(0x240);
                                *(float *)arena<uint8_t>(0x258) = r.real(-5, 5), *(float *)arena<uint8_t>(0x25c) = r.real(-5, 5);
                                arena<int32_t>(0x40)[0] = r.range(0, 3), arena<float>(0x40)[1] = r.real(-0.5f, 1.5f);
                                for (int c = 0; c < 3; c++) arena<uint32_t>(0x60)[c] = r.next();
                            },
                            [](void *fn) {
                                return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, void *, int32_t, float))fn)(arena<void>(0x100), arena<void>(0x60), arena<int32_t>(0x40)[0], arena<float>(0x40)[1])};
                            }};
    quick(water_entry_8fe240);
    // Shop::Swing4C4BA0: the game's limit set (put back after), values inside and past either side, p.x infinite at times
    {
        static uint32_t saved_limit;
        Test x{0x4c4ba0,
               [=](Rng &r) {
                   *(float *)0x10ee2a4 = r.real(0, 10);
                   *(float *)arena<uint8_t>(0x190) = r.real(-15, 15);
                   const int k = r.range(0, 9);
                   arena<float>(0x40)[0] = k == 0 ? __builtin_inff() : k == 1 ? -0.0f : r.real(-100, 100);
               },
               [](void *fn) {
                   ((void(__thiscall *)(void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40));
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { saved_limit = *(uint32_t *)0x10ee2a4; };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x10ee2a4 = saved_limit; };
        quick(x);
    }
    // PackedRef::Resolve: every top-bit pattern, both blocks, offsets below and past the block's limit
    Test packed_ref_resolve{0xbdd0d0,
                            [=](Rng &r) {
                                uint32_t *self = arena<uint32_t>(0x100);
                                for (int i = 0; i < 8; i++) self[i] = r.next();
                                self[0] = (uint32_t)(uintptr_t)arena<void>(0x140);
                                for (int b = 0; b < 2; b++) {
                                    arena<uint32_t>(0x140)[b] = (uint32_t)(uintptr_t)arena<void>(0x180 + b * 0x10);
                                    arena<uint32_t>(0x180 + b * 0x10)[0] = (uint32_t)(uintptr_t)arena<void>(0x200 + b * 0x40);
                                    arena<uint32_t>(0x200 + b * 0x40)[9] = r.range(0, 0x2000);
                                }
                                arena<uint32_t>(0x40)[0] = r.next();
                            },
                            [](void *fn) { return Result{(uint32_t)((uintptr_t(__thiscall *)(const void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0])}; }};
    quick(packed_ref_resolve);
    // Vector::Quadrant: random flat vectors (equal sizes, zeros and signed zeros at times), passed in eax as the game does
    Test vector_quadrant{0x55fc10,
                         [=](Rng &r) {
                             float *v = arena<float>(0x40);
                             for (int c = 0; c < 3; c++) v[c] = r.real(-5, 5);
                             const int k = r.range(0, 5);
                             if (k == 0) v[2] = r.range(0, 1) ? v[0] : -v[0];
                             if (k == 1) v[0] = r.range(0, 1) ? 0.0f : -0.0f;
                             if (k == 2) v[2] = r.range(0, 1) ? 0.0f : -0.0f;
                         },
                         [](void *fn) {
                             uint32_t a = (uint32_t)(uintptr_t)arena<void>(0x40);
                             __asm__ volatile("call *%%ebx" : "+a"(a) : "b"(fn) : "ecx", "edx", "memory", "cc");
                             return Result{a};
                         }};
    quick(vector_quadrant);
    // ActiveList::Remove: entries 0-5 active or not, active lists of 0-6 ids from a few (n there or not)
    Test active_list_remove{0x9fca90,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                *(uint32_t *)(self + 0x54) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                for (int i = 0; i < 6 * 0xc; i++) arena<uint32_t>(0x400)[i] = r.range(0, 2) ? r.next() : 0;
                                *(uint32_t *)(self + 0x64) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                const int n = r.range(0, 6);
                                *(int32_t *)(self + 0x6c) = r.range(0, 9) ? n : -1;
                                for (int i = 0; i < 8; i++) arena<int32_t>(0x200)[i] = r.range(0, 5);
                                arena<int32_t>(0x40)[0] = r.range(0, 5);
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0]);
                                return Result{0};
                            }};
    quick(active_list_remove);
    // List::TakeAll: a random target and a source list of 0-5 nodes
    Test list_take_all{0x966e40,
                       [=](Rng &r) {
                           for (int i = 0; i < 8; i++) arena<uint32_t>(0x100)[i] = r.next();
                           const int n = r.range(0, 5);
                           uint32_t *end = arena<uint32_t>(0x300);
                           end[0] = 0, end[1] = r.next();
                           for (int k = 0; k < n; k++) {
                               uint32_t *node = arena<uint32_t>(0x200 + k * 0x10);
                               node[0] = (uint32_t)(uintptr_t)(k + 1 < n ? arena<void>(0x200 + (k + 1) * 0x10) : (void *)end);
                               node[1] = r.next(), node[2] = r.next();
                           }
                           arena<uint32_t>(0x40)[0] = (uint32_t)(uintptr_t)(n ? arena<void>(0x200) : (void *)end);
                       },
                       [](void *fn) {
                           ((void(__stdcall *)(void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40));
                           return Result{0};
                       }};
    quick(list_take_all);
    // MeshPart::CopyFrom: sources with 0-5 buffers and a +0x1C buffer or none
    Test mesh_part_copy_from{0xbe8aa0,
                             [=](Rng &r) {
                                 for (int i = 0; i < 0x14; i++) arena<uint32_t>(0x100)[i] = r.next(), arena<uint32_t>(0x200)[i] = r.next();
                                 uint32_t *from = arena<uint32_t>(0x200);
                                 const uint32_t n = r.range(0, 5);
                                 from[0x13] = n;
                                 for (uint32_t i = 0; i < n; i++) from[0xc + i] = (uint32_t)(uintptr_t)arena<void>(0x400 + i * 0x10);
                                 from[7] = r.range(0, 2) ? (uint32_t)(uintptr_t)arena<void>(0x480) : 0;
                                 for (int i = 0; i < 0x24; i++) arena<uint32_t>(0x400)[i] = r.next();
                             },
                             [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, const void *))fn)(arena<void>(0x100), arena<void>(0x200))}; }};
    quick(mesh_part_copy_from);
    // Table::SumBDC880: 0-128 entries of random bytes (0xFF and 0xFE among the second ones), a length not a multiple of 4
    // at times
    Test table_sum_bdc880{0xbdc880,
                          [=](Rng &r) {
                              uint8_t *self = arena<uint8_t>(0x1000);
                              for (int i = 0; i < 0x80; i++) {
                                  arena<uint32_t>(0x100c)[i] = r.next();
                                  const int k = r.range(0, 5);
                                  if (k < 2) self[0xc + i * 4 + 1] = k ? 0xff : 0xfe;
                              }
                              *(uint32_t *)(self + 0x20c) = r.range(0, 2) ? 4 * r.range(0, 0x80) : r.range(0, 0x200);
                          },
                          [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *))fn)(arena<void>(0x1000))}; }};
    quick(table_sum_bdc880);
    // Car::SpeedFactor: the game's speed and flag set (put back after), parts flagged or not
    {
        static uint32_t saved_speed, saved_flag;
        Test x{0x4e4a00,
               [=](Rng &r) {
                   *(float *)0x10ee2fc = r.real(-3, 3), *(uint32_t *)0x10ee2f8 = r.range(0, 1) ? r.next() | 1 : 0;
                   uint8_t *self = arena<uint8_t>(0x1000), *part = arena<uint8_t>(0x200);
                   for (int k = 0; k < 4; k++) *(uint32_t *)(self + 0x154 + k * 4) = (uint32_t)(uintptr_t)part;
                   part[0x148] = r.range(0, 1) ? (uint8_t)r.range(1, 255) : 0;
                   *(uint32_t *)(part + 0x144) = r.range(0, 1) ? r.next() | 1 : 0;
                   arena<int32_t>(0x40)[0] = r.range(0, 3);
               },
               [](void *fn) {
                   Result x;
                   x.f = ((long double(__thiscall *)(const void *, int32_t))fn)(arena<void>(0x1000), arena<int32_t>(0x40)[0]);
                   return x;
               }};
        x.prepare = [](const rewrite::Entry *) { saved_speed = *(uint32_t *)0x10ee2fc, saved_flag = *(uint32_t *)0x10ee2f8; };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x10ee2fc = saved_speed, *(uint32_t *)0x10ee2f8 = saved_flag; };
        quick(x);
    }
    // Park::Copy8EECF0 / 8EED40 / 8EED90 / 8EEDE0: random points and scales
    for (uintptr_t at : {(uintptr_t)0x8eecf0, (uintptr_t)0x8eed40, (uintptr_t)0x8eed90, (uintptr_t)0x8eede0}) {
        Test x{at,
               [=](Rng &r) {
                   for (int i = 0; i < 8; i++) arena<uint32_t>(0x100)[i] = r.next();
                   for (int i = 0; i < 12; i++) arena<float>(0x40)[i] = r.real(-50, 50);
               },
               [](void *fn) {
                   ((void(__thiscall *)(void *, const void *, const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40), arena<void>(0x50), arena<void>(0x60));
                   return Result{0};
               }};
        quick(x);
    }
    // Options::SetFloat: options 0-7, values below, inside and past their bounds (the game's table compared)
    Test options_set_float{0x4796f0,
                           [=](Rng &r) {
                               arena<uint32_t>(0x40)[0] = r.range(0, 7);
                               arena<float>(0x40)[1] = r.range(0, 2) ? r.real(-1, 4) : r.real(-200, 200);
                           },
                           [](void *fn) {
                               uint32_t n = arena<uint32_t>(0x40)[0];
                               ((void(__thiscall *)(uintptr_t, float))fn)(n, arena<float>(0x40)[1]);
                               return Result{0};
                           }};
    t.push_back(options_set_float);   // (its writes are game globals: the data section is compared)
    // Options::SetInt: options 0-7, values below, inside and past their bounds (the game's table compared)
    Test options_set_int{0x479630,
                         [=](Rng &r) {
                             arena<uint32_t>(0x40)[0] = r.range(0, 7);
                             arena<int32_t>(0x40)[1] = r.range(0, 2) ? r.range(-5, 5000) : (int32_t)r.next();
                         },
                         [](void *fn) {
                             ((void(__fastcall *)(uintptr_t, int32_t))fn)(arena<uint32_t>(0x40)[0], arena<int32_t>(0x40)[1]);
                             return Result{0};
                         }};
    t.push_back(options_set_int);
    // Effect::Entry408700 / 408750: groups 0-2, entries 0-3, random sizes
    for (uintptr_t at : {(uintptr_t)0x408700, (uintptr_t)0x408750}) {
        Test x{at,
               [=](Rng &r) {
                   *arena<uint32_t>(0x100) = (uint32_t)(uintptr_t)arena<void>(0x140);
                   for (int g = 0; g < 3; g++) {
                       arena<uint32_t>(0x140)[g] = (uint32_t)(uintptr_t)arena<void>(0x180 + g * 0x10);
                       arena<uint32_t>(0x180 + g * 0x10)[3] = (uint32_t)(uintptr_t)arena<void>(0x400 + g * 0x200);
                   }
                   for (int i = 0; i < 0x180; i++) arena<uint32_t>(0x400)[i] = r.next();
                   arena<int32_t>(0x40)[0] = r.range(0, 2) << 16 | r.range(0, 3);
                   arena<float>(0x40)[1] = r.real(-30, 30), arena<float>(0x40)[2] = r.real(-30, 30);
               },
               [](void *fn) {
                   return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, int32_t, float, float))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<float>(0x40)[1], arena<float>(0x40)[2])};
               }};
        quick(x);
    }
    // Effect::Round441270 / Ride::Round5F97A0: differences of both signs (whole steps at times)
    for (uintptr_t at : {(uintptr_t)0x441270, (uintptr_t)0x5f97a0}) {
        Test x{at,
               [=](Rng &r) {
                   float *f = arena<float>(0x100);
                   f[0x48 / 4] = r.real(-20000, 20000), f[0x54 / 4] = r.range(0, 3) ? r.real(-20000, 20000) : f[0x48 / 4] - 90.0f * r.range(-50, 50);
               },
               [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(const void *))fn)(arena<void>(0x100))}; }};
        quick(x);
    }
    // Window::ScaleA45620 / ScaleA455D0: random window sizes and points
    Test window_scale_a45620{0xa45620,
                             [=](Rng &r) {
                                 float *f = arena<float>(0x100);
                                 for (int k = 0x7c / 4; k <= 0x88 / 4; k++) f[k] = r.real(-800, 800);
                                 arena<float>(0x40)[0] = r.real(-1000, 1000), arena<float>(0x40)[1] = r.real(-1000, 1000);
                             },
                             [](void *fn) {
                                 ((void(__thiscall *)(const void *, void *))fn)(arena<void>(0x100), arena<void>(0x40));
                                 return Result{0};
                             }};
    quick(window_scale_a45620);
    Test window_scale_a455d0{0xa455d0,
                             [=](Rng &r) {
                                 float *f = arena<float>(0x100);
                                 for (int k = 0x7c / 4; k <= 0x88 / 4; k++) f[k] = r.real(-800, 800);
                                 arena<float>(0x40)[0] = r.real(-1000, 1000), arena<float>(0x40)[1] = r.real(-1000, 1000);
                                 arena<uint32_t>(0x60)[0] = r.next(), arena<uint32_t>(0x60)[1] = r.next();
                             },
                             [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, void *, const void *))fn)(arena<void>(0x100), arena<void>(0x60), arena<void>(0x40))}; }};
    quick(window_scale_a455d0);
    // Waterfall::Line917540: random planes and points
    Test waterfall_line_917540{0x917540,
                               [=](Rng &r) {
                                   for (int i = 0; i < 7; i++) arena<float>(0x100)[i] = r.real(-20, 20);
                                   for (int c = 0; c < 3; c++) arena<float>(0x40)[c] = r.real(-20, 20);
                               },
                               [](void *fn) {
                                   Result x;
                                   x.f = ((long double(__thiscall *)(const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40));
                                   return x;
                               }};
    quick(waterfall_line_917540);
    // Terrain::Untotal8FDA10: random counts and sizes (the game's totals compared)
    Test terrain_untotal_8fda10{0x8fda10,
                                [=](Rng &r) {
                                    uint8_t *self = arena<uint8_t>(0x100);
                                    *(uint32_t *)(self + 0x14) = r.range(0, 500), *(uint32_t *)(self + 0x18) = r.range(0, 500);
                                    *(uint32_t *)(self + 0xc) = (uint32_t)(uintptr_t)arena<void>(0x140);
                                    *(uint32_t *)arena<uint8_t>(0x14c) = (uint32_t)(uintptr_t)arena<void>(0x180);
                                    *arena<int32_t>(0x180) = r.range(0, 300);
                                },
                                [](void *fn) {
                                    ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                                    return Result{0};
                                }};
    t.push_back(terrain_untotal_8fda10);   // (its writes are game globals: the data section is compared)
    // WildAnimal::SetPointB2F0C0: points all round (on an axis at times)
    Test wild_animal_set_point_b2f0c0{0xb2f0c0,
                                      [=](Rng &r) {
                                          for (int i = 0; i < 8; i++) arena<uint32_t>(0x180)[i] = r.next();
                                          float *p = arena<float>(0x40);
                                          for (int c = 0; c < 3; c++) p[c] = r.real(-10, 10);
                                          const int k = r.range(0, 5);
                                          if (k == 0) p[0] = 0;
                                          if (k == 1) p[2] = r.range(0, 1) ? 0.0f : -0.0f;
                                      },
                                      [](void *fn) {
                                          ((void(__thiscall *)(void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40));
                                          return Result{0};
                                      }};
    quick(wild_animal_set_point_b2f0c0);
    // Window::SetPointC5DED0: sizes of either sign (0 at times), random points
    Test window_set_point_c5ded0{0xc5ded0,
                                 [=](Rng &r) {
                                     for (int i = 0; i < 0x34; i++) arena<uint32_t>(0x1000 + 0xd0)[i] = r.next();
                                     arena<int32_t>(0x40)[0] = r.range(-5, 2000), arena<int32_t>(0x40)[1] = r.range(-5, 2000);
                                     arena<float>(0x50)[0] = r.real(-3000, 3000), arena<float>(0x50)[1] = r.real(-3000, 3000);
                                 },
                                 [](void *fn) {
                                     ((void(__thiscall *)(void *, int32_t, int32_t, const void *))fn)(arena<void>(0x1000), arena<int32_t>(0x40)[0], arena<int32_t>(0x40)[1], arena<void>(0x50));
                                     return Result{0};
                                 }};
    quick(window_set_point_c5ded0);
    // Lighting::DownwardDirection: y either side of 0 (signed zeros at times)
    Test lighting_downward_direction{0x4261f0,
                                     [=](Rng &r) {
                                         float *v = arena<float>(0x220);
                                         for (int c = 0; c < 3; c++) v[c] = r.real(-1, 1);
                                         if (r.range(0, 5) == 0) v[1] = r.range(0, 1) ? 0.0f : -0.0f;
                                         for (int c = 0; c < 3; c++) arena<uint32_t>(0x40)[c] = r.next();
                                     },
                                     [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, void *))fn)(arena<void>(0x100), arena<void>(0x40))}; }};
    quick(lighting_downward_direction);
    // CustomPersistingSpline::EndDirection: 1-6 points of random values
    Test spline_end_direction{0x424d50,
                              [=](Rng &r) {
                                  uint8_t *self = arena<uint8_t>(0x100);
                                  *(uint32_t *)(self + 4) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                  *(int32_t *)(self + 8) = r.range(1, 6);
                                  for (int i = 0; i < 6 * 9; i++) arena<float>(0x200)[i] = r.real(-5, 5);
                                  for (int c = 0; c < 3; c++) arena<uint32_t>(0x40)[c] = r.next();
                              },
                              [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x40), 3)}; }};
    quick(spline_end_direction);
    // Rect::Contains: random rects (crossed at times), points inside, on the edges and outside
    Test rect_contains{0xbe7780,
                       [=](Rng &r) {
                           float *q = arena<float>(0x100), *p = arena<float>(0x40);
                           for (int c = 0; c < 4; c++) q[c] = (float)r.range(-6, 6);
                           p[0] = r.range(0, 1) ? (float)r.range(-7, 7) : r.real(-7, 7);
                           p[1] = r.range(0, 1) ? (float)r.range(-7, 7) : r.real(-7, 7);
                       },
                       [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40))}; }};
    quick(rect_contains);
    // HashTable::FindBD65D0: 1-4 buckets over 0-10 keys from a few, keys there or not
    Test hash_table_find_bd65d0{0xbd65d0,
                                [=](Rng &r) {
                                    uint8_t *self = arena<uint8_t>(0x100);
                                    const uint32_t nb = r.range(1, 4), n = r.range(0, 10);
                                    *(uint32_t *)(self + 4) = nb;
                                    uint32_t *buckets = arena<uint32_t>(0x140);
                                    *(uint32_t *)(self + 0xc) = (uint32_t)(uintptr_t)buckets;
                                    *(uint32_t *)(self + 0x10) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                    *(uint32_t *)(self + 0x14) = (uint32_t)(uintptr_t)arena<void>(0x180);
                                    buckets[0] = 0;
                                    for (uint32_t b = 1; b <= nb; b++) buckets[b] = b == nb ? n : r.range(buckets[b - 1], n);
                                    for (uint32_t i = 0; i < n; i++) arena<uint32_t>(0x200)[i] = r.range(0, 12);
                                    arena<uint32_t>(0x40)[0] = r.range(0, 12);
                                },
                                [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40))}; }};
    quick(hash_table_find_bd65d0);
    // PointerArray::RemoveSwap2: 0-6 values from a few (repeats), the value there or not
    Test pointer_array_remove_swap2{0x80ed70,
                                    [=](Rng &r) {
                                        uint8_t *self = arena<uint8_t>(0x100);
                                        const int n = r.range(0, 6);
                                        *(int32_t *)(self + 0x64) = r.range(0, 9) ? n : -1;
                                        *(uint32_t *)(self + 0x5c) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                        for (int i = 0; i < 8; i++) arena<uint32_t>(0x200)[i] = r.range(0, 5);
                                        arena<uint32_t>(0x40)[0] = r.range(0, 6);
                                    },
                                    [](void *fn) {
                                        ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0]);
                                        return Result{0};
                                    }};
    quick(pointer_array_remove_swap2);
    // Record::InitC212D0: a point or none
    Test record_init_c212d0{0xc212d0,
                            [=](Rng &r) {
                                for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next();
                                for (int c = 0; c < 3; c++) arena<uint32_t>(0x50)[c] = r.next();
                                arena<uint32_t>(0x40)[0] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x50) : 0;
                                arena<uint32_t>(0x40)[1] = r.next();
                            },
                            [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, const void *, uint32_t))fn)(arena<void>(0x100), (const void *)(uintptr_t)arena<uint32_t>(0x40)[0], arena<uint32_t>(0x40)[1])}; }};
    quick(record_init_c212d0);
    // Animal::ScaleB16D60: values that scale below 0, inside and past 1.5
    Test animal_scale_b16d60{0xb16d60,
                             [=](Rng &r) { *(float *)arena<uint8_t>(0x110) = r.range(0, 5) == 0 ? -0.0f : r.real(-3, 8); },
                             [](void *fn) {
                                 Result x;
                                 x.f = ((long double(__thiscall *)(const void *))fn)(arena<void>(0x100));
                                 return x;
                             }};
    quick(animal_scale_b16d60);
    // Window::Width670620: widths either side of 290
    Test window_width_670620{0x670620,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100);
                                 *(uint32_t *)(self + 0x134) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                 *(uint32_t *)(self + 0x108) = (uint32_t)(uintptr_t)arena<void>(0x500);
                                 *(float *)arena<uint8_t>(0x47c) = r.real(-100, 300), *(float *)arena<uint8_t>(0x484) = r.real(0, 400);
                                 *(float *)arena<uint8_t>(0x584) = r.real(-50, 200);
                             },
                             [](void *fn) {
                                 Result x;
                                 x.f = ((long double(__thiscall *)(const void *))fn)(arena<void>(0x100));
                                 return x;
                             }};
    quick(window_width_670620);
    // Shader::TakeCC2C30: any count and out value
    Test shader_take_cc2c30{0xcc2c30,
                            [=](Rng &r) { *(uint32_t *)arena<uint8_t>(0x1100) = r.next(), arena<uint32_t>(0x40)[0] = r.next(); },
                            [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x40), 5)}; }};
    quick(shader_take_cc2c30);
    // Bits::ResetE04110: versions below, at and past 0x40000000, 0-8 words (a negative count at times)
    Test bits_reset_e04110{0xe04110,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               for (int i = 0; i < 6; i++) arena<uint32_t>(0x100)[i] = r.next();
                               *(int32_t *)self = r.range(0, 9) ? r.range(0, 8) : -1;
                               const int k = r.range(0, 3);
                               *(int32_t *)(self + 8) = k == 0 ? 0x40000000 : k == 1 ? 0x40000001 : (int32_t)r.next();
                               *(uint32_t *)(self + 0xc) = (uint32_t)(uintptr_t)arena<void>(0x200);
                               for (int i = 0; i < 8; i++) arena<uint32_t>(0x200)[i] = r.next();
                           },
                           [](void *fn) {
                               ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                               return Result{0};
                           }};
    quick(bits_reset_e04110);
    // Bits::TestC165F0: bits -70..200 over 0-6 words of random bits
    Test bits_test_c165f0{0xc165f0,
                          [=](Rng &r) {
                              uint8_t *self = arena<uint8_t>(0x100);
                              *(int32_t *)(self + 0x10) = r.range(0, 6), *(uint32_t *)(self + 0x14) = r.next();
                              *(uint32_t *)(self + 0xc) = (uint32_t)(uintptr_t)arena<void>(0x200);
                              for (int i = 0; i < 8; i++) arena<uint32_t>(0x200)[i] = r.next() & r.next();
                              arena<int32_t>(0x40)[0] = r.range(-70, 200);
                          },
                          [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0])}; }};
    quick(bits_test_c165f0);
    // Bits::SizeBB65A0: 0-200 bits into random words
    Test bits_size_bb65a0{0xbb65a0,
                          [=](Rng &r) {
                              for (int i = 0; i < 8; i++) arena<uint32_t>(0x200)[i] = r.next() & r.next();
                              arena<uint32_t>(0x40)[0] = r.range(0, 3) ? r.range(0, 200) : 32 * r.range(0, 6);
                          },
                          [](void *fn) { return Result{((uint32_t(__fastcall *)(void *, uint32_t))fn)(arena<void>(0x200), arena<uint32_t>(0x40)[0])}; }};
    quick(bits_size_bb65a0);
    // Text::CopyBFC290: the game's byte set or not (put back after), values 0 or not, the +0x18 byte set or not
    {
        static uint8_t saved_byte;
        Test x{0xbfc290,
               [=](Rng &r) {
                   *(uint8_t *)0x13d3175 = r.range(0, 1) ? (uint8_t)r.range(1, 255) : 0;
                   for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x100)[i] = r.next();
                   *(uint32_t *)arena<uint8_t>(0x210) = r.range(0, 3) ? r.next() : 0;
                   *arena<uint8_t>(0x218) = r.range(0, 1) ? (uint8_t)r.range(1, 255) : 0;
               },
               [](void *fn) { return Result{(uint32_t)(uintptr_t)((const void *(__thiscall *)(void *, const void *))fn)(arena<void>(0x100), arena<void>(0x200))}; }};
        x.prepare = [](const rewrite::Entry *) { saved_byte = *(uint8_t *)0x13d3175; };
        x.finish = [](const rewrite::Entry *) { *(uint8_t *)0x13d3175 = saved_byte; };
        quick(x);
    }
    // Window::Init6AF710: a window joining two lists (empty ones end at their own head)
    Test window_init_6af710{0x6af710,
                            [=](Rng &r) {
                                for (int i = 0; i < 8; i++) arena<uint32_t>(0x100)[i] = r.next();
                                uint32_t *owner = arena<uint32_t>(0x200), *list = arena<uint32_t>(0x280);
                                owner[0x34 / 4] = (uint32_t)(uintptr_t)arena<void>(0x300);
                                list[0x14 / 4] = (uint32_t)(uintptr_t)arena<void>(0x320);
                                for (uintptr_t h : {(uintptr_t)0x300, (uintptr_t)0x320}) {
                                    uint32_t *head = arena<uint32_t>(h), *first = arena<uint32_t>(h + 0x40);
                                    head[0] = r.range(0, 1) ? (uint32_t)(uintptr_t)first : (uint32_t)(uintptr_t)head;
                                    head[1] = r.next(), first[0] = r.next(), first[1] = r.next();
                                }
                            },
                            [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x200), arena<void>(0x280))}; }};
    quick(window_init_6af710);
    // TrackImporter::OppositeDirection: directions 0-7 in eax as the game passes them
    Test opposite_direction{0xa3fa60,
                            [=](Rng &r) { arena<uint32_t>(0x40)[0] = r.range(0, 7); },
                            [](void *fn) {
                                uint32_t a = arena<uint32_t>(0x40)[0];
                                __asm__ volatile("call *%%ebx" : "+a"(a) : "b"(fn) : "ecx", "edx", "memory", "cc");
                                return Result{a};
                            }};
    quick(opposite_direction);
    // Ride::Near58CD40: points near and far, squared distances either side (the points in ecx and eax)
    Test ride_near_58cd40{0x58cd40,
                          [=](Rng &r) {
                              float *a = arena<float>(0x100), *b = arena<float>(0x110);
                              for (int c = 0; c < 3; c++) a[c] = r.real(-20, 20), b[c] = a[c] + r.real(-3, 3);
                              arena<float>(0x40)[0] = r.real(0, 30);
                          },
                          [](void *fn) {
                              uint32_t stack[1] = {arena<uint32_t>(0x40)[0]};
                              uint32_t *ps = stack;
                              void *a = arena<void>(0x100);
                              uint32_t b = (uint32_t)(uintptr_t)arena<void>(0x110);
                              __asm__ volatile("pushl (%%edx)\n\tcall *%%ebx" : "+a"(b), "+c"(a), "+d"(ps) : "b"(fn) : "memory", "cc");
                              return Result{b};
                          }};
    quick(ride_near_58cd40);
    // Vector::StepC1E790: random planes (tiny normals at times)
    Test vector_step_c1e790{0xc1e790,
                            [=](Rng &r) {
                                for (int i = 0; i < 6; i++) arena<uint32_t>(0x100)[i] = r.next();
                                float *p = arena<float>(0x40);
                                const float s = r.range(0, 5) ? 1.0f : 1e-3f;
                                for (int c = 0; c < 3; c++) p[c] = r.real(-5, 5) * s;
                                p[3] = r.real(-50, 50);
                            },
                            [](void *fn) { return Result{(uint32_t)(uintptr_t)((const void *(__thiscall *)(void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40))}; }};
    quick(vector_step_c1e790);
    // Ride::Value576B00: values either side of a half, modes 0-3
    Test ride_value_576b00{0x576b00,
                           [=](Rng &r) {
                               *(int32_t *)arena<uint8_t>(0x104) = r.range(0, 3);
                               *(float *)arena<uint8_t>(0x108) = r.range(0, 5) == 0 ? 0.5f : r.real(-0.5f, 1.5f);
                           },
                           [](void *fn) {
                               Result x;
                               x.f = ((long double(__thiscall *)(const void *))fn)(arena<void>(0x100));
                               return x;
                           }};
    quick(ride_value_576b00);
    // Window::Fraction660630: random values and steps (whole quotients at times)
    Test window_fraction_660630{0x660630,
                                [=](Rng &r) {
                                    float *f = arena<float>(0x100);
                                    f[0x158 / 4] = r.real(-100, 100), f[0x160 / 4] = r.real(-100, 400);
                                    f[0x16c / 4] = r.range(0, 4) ? r.real(1, 40) : -r.real(1, 40);
                                    arena<float>(0x40)[1] = r.range(0, 3) ? r.real(-100, 300) : f[0x160 / 4] - f[0x158 / 4] - f[0x16c / 4] * r.range(-5, 5);
                                },
                                [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40))}; }};
    quick(window_fraction_660630);
    // Effect::Colour413C70: parts within and past 0..1
    Test effect_colour_413c70{0x413c70,
                              [=](Rng &r) {
                                  arena<uint32_t>(0x100)[0] = r.next();
                                  for (int c = 0; c < 3; c++) arena<float>(0x40)[c] = r.range(0, 3) ? r.real(0, 1) : r.real(-0.5f, 1.5f);
                              },
                              [](void *fn) {
                                  const float *c = arena<float>(0x40);
                                  return Result{((uint32_t(__thiscall *)(void *, float, float, float))fn)(arena<void>(0x100), c[0], c[1], c[2])};
                              }};
    quick(effect_colour_413c70);
    // Track::MoveBC53D0: the game's step sizes set (put back after), steps of either sign
    {
        static uint32_t saved_steps[3];
        Test x{0xbc53d0,
               [=](Rng &r) {
                   for (int k = 0; k < 3; k++) ((float *)0x10f2b30)[k] = r.real(-3, 3);
                   for (int k = 0; k < 3; k++) arena<float>(0x110)[k] = r.real(-100, 100);
                   for (int k = 0; k < 3; k++) arena<int32_t>(0x40)[k] = r.range(-40, 40);
               },
               [](void *fn) {
                   const int32_t *a = arena<int32_t>(0x40);
                   ((void(__thiscall *)(void *, int32_t, int32_t, int32_t))fn)(arena<void>(0x100), a[0], a[1], a[2]);
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { memcpy(saved_steps, (void *)0x10f2b30, 12); };
        x.finish = [](const rewrite::Entry *) { memcpy((void *)0x10f2b30, saved_steps, 12); };
        quick(x);
    }
    // WildAnimal::AddB23940: sums below 0, inside and past 1
    Test wild_animal_add_b23940{0xb23940,
                                [=](Rng &r) { arena<float>(0x100)[0] = r.real(-1, 2), arena<float>(0x40)[0] = r.real(-1, 1); },
                                [](void *fn) {
                                    ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x40)[0]);
                                    return Result{0};
                                }};
    quick(wild_animal_add_b23940);
    // Track::ValueBC5E20: one part, the other or both (equal values at times)
    Test track_value_bc5e20{0xbc5e20,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                const int k = r.range(0, 3);
                                *(uint32_t *)(self + 0x20) = k == 1 ? 0 : (uint32_t)(uintptr_t)arena<void>(0x200);
                                *(uint32_t *)(self + 0x24) = k == 2 ? 0 : (uint32_t)(uintptr_t)arena<void>(0x240);
                                *(float *)arena<uint8_t>(0x21c) = r.real(-5, 5);
                                *(float *)arena<uint8_t>(0x25c) = r.range(0, 4) ? r.real(-5, 5) : *(float *)arena<uint8_t>(0x21c);
                            },
                            [](void *fn) {
                                Result x;
                                x.f = ((long double(__thiscall *)(const void *))fn)(arena<void>(0x100));
                                return x;
                            }};
    quick(track_value_bc5e20);
    // Ride::Point4E5D40: 1-6 points of random values
    Test ride_point_4e5d40{0x4e5d40,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               *(int32_t *)self = r.range(1, 6);
                               *(uint32_t *)(self + 4) = (uint32_t)(uintptr_t)arena<void>(0x200);
                               for (int i = 0; i < 6 * 9; i++) arena<float>(0x200)[i] = r.real(-5, 5);
                               for (int c = 0; c < 3; c++) arena<uint32_t>(0x40)[c] = r.next();
                           },
                           [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, void *))fn)(arena<void>(0x100), arena<void>(0x40))}; }};
    quick(ride_point_4e5d40);
    // Ride::Point4BB4C0: random offsets and points
    Test ride_point_4bb4c0{0x4bb4c0,
                           [=](Rng &r) {
                               for (int k = 0x7c / 4; k <= 0x90 / 4; k++) arena<float>(0x100)[k] = r.real(-500, 500);
                               arena<float>(0x40)[0] = r.real(-500, 500), arena<float>(0x40)[1] = r.real(-500, 500);
                               arena<uint32_t>(0x60)[0] = r.next(), arena<uint32_t>(0x60)[1] = r.next();
                           },
                           [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, void *, const void *))fn)(arena<void>(0x100), arena<void>(0x60), arena<void>(0x40))}; }};
    quick(ride_point_4bb4c0);
    // Text::Value49E0B0: random points, phases and sizes, with out or none
    Test text_value_49e0b0{0x49e0b0,
                           [=](Rng &r) {
                               arena<float>(0x100)[3] = r.real(-10, 10), arena<float>(0x100)[4] = r.real(-3, 3);
                               for (int c = 0; c < 3; c++) arena<float>(0x40)[c] = r.real(-200, 200);
                               arena<uint32_t>(0x60)[0] = r.next(), arena<uint32_t>(0x60)[1] = r.next();
                               arena<uint32_t>(0x50)[0] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x60) : 0;
                           },
                           [](void *fn) {
                               Result x;
                               x.f = ((long double(__thiscall *)(const void *, const void *, void *))fn)(arena<void>(0x100), arena<void>(0x40), (void *)(uintptr_t)arena<uint32_t>(0x50)[0]);
                               return x;
                           }};
    quick(text_value_49e0b0);
    // Track::ScaleBC7F90: the game's step sizes set (put back after), modes 0-4
    {
        static uint32_t saved_steps[3];
        Test x{0xbc7f90,
               [=](Rng &r) {
                   for (int k = 0; k < 3; k++) ((float *)0x10f2b30)[k] = r.real(-3, 3);
                   *(uint32_t *)arena<uint8_t>(0x104) = (uint32_t)(uintptr_t)arena<void>(0x200);
                   arena<int32_t>(0x200)[4] = r.range(-40, 40), arena<int32_t>(0x200)[5] = r.range(-40, 40);
                   arena<int32_t>(0x40)[0] = r.range(0, 4);
                   arena<uint32_t>(0x60)[0] = r.next(), arena<uint32_t>(0x60)[1] = r.next();
               },
               [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, void *, int32_t))fn)(arena<void>(0x100), arena<void>(0x60), arena<int32_t>(0x40)[0])}; }};
        x.prepare = [](const rewrite::Entry *) { memcpy(saved_steps, (void *)0x10f2b30, 12); };
        x.finish = [](const rewrite::Entry *) { memcpy((void *)0x10f2b30, saved_steps, 12); };
        quick(x);
    }
    // Fireworks::Entry9FC910: entries 0-3, values of either sign
    Test fireworks_entry_9fc910{0x9fc910,
                                [=](Rng &r) {
                                    *(uint32_t *)arena<uint8_t>(0x154) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                    for (int k = 0; k < 4; k++) arena<uint32_t>(0x200 + k * 0x30)[2] = (uint32_t)(uintptr_t)arena<void>(0x400 + k * 0x80);
                                    for (int i = 0; i < 0x80; i++) arena<uint32_t>(0x400)[i] = r.next();
                                    arena<int32_t>(0x40)[0] = r.range(0, 3), arena<float>(0x40)[1] = r.range(0, 1) ? r.real(0.1f, 20) : -r.real(0.1f, 20);
                                },
                                [](void *fn) {
                                    ((void(__thiscall *)(const void *, int32_t, float))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<float>(0x40)[1]);
                                    return Result{0};
                                }};
    quick(fireworks_entry_9fc910);
    // Math::ScaleC19880: whole numbers of either sign (large ones at times)
    Test math_scale_c19880{0xc19880,
                           [=](Rng &r) { arena<int32_t>(0x40)[0] = r.range(0, 3) ? r.range(-2000, 2000) : (int32_t)r.next() >> 4; },
                           [](void *fn) { return Result{((uint32_t(__stdcall *)(int32_t))fn)(arena<int32_t>(0x40)[0])}; }};
    quick(math_scale_c19880);
    // Animal::ValueA5ED70: values below 0, between 0 and 1 and past it
    Test animal_value_a5ed70{0xa5ed70,
                             [=](Rng &r) {
                                 *(uint32_t *)arena<uint8_t>(0x108) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                 arena<float>(0x200)[0] = r.range(0, 5) == 0 ? 0.0f : r.real(-1, 4);
                             },
                             [](void *fn) {
                                 Result x;
                                 x.f = ((long double(__thiscall *)(const void *))fn)(arena<void>(0x100));
                                 return x;
                             }};
    quick(animal_value_a5ed70);
    // Animal::FractionA887F0: counts giving fractions either side of a half, sums either side of 0
    Test animal_fraction_a887f0{0xa887f0,
                                [=](Rng &r) {
                                    uint8_t *self = arena<uint8_t>(0x100);
                                    *(int32_t *)(self + 0x6c) = r.range(1, 40), *(int32_t *)(self + 0xa8) = r.range(-5, 8);
                                    *(float *)(self + 0x50) = r.real(-3, 3);
                                },
                                [](void *fn) {
                                    ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                                    return Result{0};
                                }};
    quick(animal_fraction_a887f0);
    // Water::Value92E870: the flag clear (a list or none) or set, points 1-4, random steps
    Test water_value_92e870{0x92e870,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                *(uint32_t *)(self + 0x20) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                *(float *)(self + 0x54) = r.real(-10, 10);
                                *(uint32_t *)arena<uint8_t>(0x21c) = r.range(0, 3) ? r.next() | 1 : 0;
                                *(uint32_t *)arena<uint8_t>(0x218) = (uint32_t)(uintptr_t)arena<void>(0x300);
                                for (int i = 0; i < 10; i++) arena<float>(0x300)[i] = r.real(-10, 10);
                                arena<int32_t>(0x40)[0] = r.range(1, 4);
                                arena<uint32_t>(0x40)[1] = r.range(0, 1) ? (r.next() & ~0xffu) : r.next() | 1;
                                arena<float>(0x40)[2] = r.real(-5, 5);
                            },
                            [](void *fn) {
                                Result x;
                                x.f = ((long double(__thiscall *)(const void *, int32_t, uint32_t, float))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<uint32_t>(0x40)[1], arena<float>(0x40)[2]);
                                return x;
                            }};
    quick(water_value_92e870);
    // Record::SameBD19C0: counts 0-5 equal or not, dwords from a few
    Test record_same_bd19c0{0xbd19c0,
                            [=](Rng &r) {
                                int32_t *a = arena<int32_t>(0x100), *b = arena<int32_t>(0x140);
                                for (int i = 0; i < 4; i++) a[i] = r.range(0, 2), b[i] = r.range(0, 3) ? a[i] : r.range(0, 2);
                                a[4] = r.range(-1, 4), b[4] = r.range(0, 3) ? a[4] : r.range(-1, 4);
                            },
                            [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x140)) & 0xff}; }};
    quick(record_same_bd19c0);
    // Track::NearBC3F90: points near and far, radii either side (the points in ecx and eax)
    Test track_near_bc3f90{0xbc3f90,
                           [=](Rng &r) {
                               float *a = arena<float>(0x100), *b = arena<float>(0x110);
                               for (int c = 0; c < 3; c++) a[c] = r.real(-20, 20), b[c] = a[c] + r.real(-3, 3);
                               arena<float>(0x40)[0] = r.real(-1, 6);
                           },
                           [](void *fn) {
                               uint32_t stack[1] = {arena<uint32_t>(0x40)[0]};
                               uint32_t *ps = stack;
                               void *a = arena<void>(0x100);
                               uint32_t b = (uint32_t)(uintptr_t)arena<void>(0x110);
                               __asm__ volatile("pushl (%%edx)\n\tcall *%%ebx" : "+a"(b), "+c"(a), "+d"(ps) : "b"(fn) : "memory", "cc");
                               return Result{b};
                           }};
    quick(track_near_bc3f90);
    // WideString::CompareNarrow / String::CompareWide: texts of 0-8 characters alike or not (high bytes at times)
    for (uintptr_t at : {(uintptr_t)0xbcea50, (uintptr_t)0xbcf120}) {
        Test x{at,
               [=](Rng &r) {
                   const bool wide_first = at == 0xbcea50;
                   uint32_t *ta = arena<uint32_t>(0x140), *tb = arena<uint32_t>(0x180);
                   arena<uint32_t>(0x100)[0] = (uint32_t)(uintptr_t)ta, arena<uint32_t>(0x104)[0] = (uint32_t)(uintptr_t)tb;
                   ta[1] = r.range(0, 8), tb[1] = r.range(0, 3) ? ta[1] : r.range(0, 8);
                   ta[3] = (uint32_t)(uintptr_t)arena<void>(0x200), tb[3] = (uint32_t)(uintptr_t)arena<void>(0x300);
                   for (int i = 0; i < 8; i++) {
                       const uint8_t c = (uint8_t)(r.range(0, 7) ? 0x41 + r.range(0, 3) : r.next());
                       const uint8_t d = r.range(0, 4) ? c : (uint8_t)(0x41 + r.range(0, 3));
                       if (wide_first) arena<uint16_t>(0x200)[i] = r.range(0, 9) ? c : (uint16_t)r.next(), arena<uint8_t>(0x300)[i] = d;
                       else arena<uint8_t>(0x200)[i] = c, arena<uint16_t>(0x300)[i] = r.range(0, 9) ? d : (uint16_t)r.next();
                   }
               },
               [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x104))}; }};
        quick(x);
    }
    // Scenery::WrapAngle839120: angles from -5 turns to 5, whole turns and 0 at times
    Test scenery_wrap_angle_839120{0x839120,
                                   [=](Rng &r) {
                                       const int k = r.range(0, 5);
                                       arena<float>(0x40)[0] = k == 0 ? 6.2831854820251465f * r.range(-3, 3) : k == 1 ? -0.0f : r.real(-32, 32);
                                   },
                                   [](void *fn) {
                                       ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x40)[0]);
                                       return Result{0};
                                   }};
    quick(scenery_wrap_angle_839120);
    // Record::CompareC21270: pairs from a few values (ties on either part)
    Test record_compare_c21270{0xc21270,
                               [=](Rng &r) {
                                   for (int i = 0; i < 2; i++) arena<float>(0x100)[i] = (float)r.range(-2, 2), arena<float>(0x140)[i] = (float)r.range(-2, 2);
                                   if (r.range(0, 4) == 0) arena<float>(0x100)[r.range(0, 1)] = r.real(-2, 2);
                               },
                               [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x140))}; }};
    quick(record_compare_c21270);
    // Fireworks::Point9E8790: shapes 0-3 of the game's table, points 0-3, rows 0-3
    Test fireworks_point_9e8790{0x9e8790,
                                [=](Rng &r) {
                                    *(int32_t *)arena<uint8_t>(0x158) = r.range(0, 3);
                                    for (int i = 0; i < 6; i++) arena<uint32_t>(0x40)[i] = r.next();
                                    arena<int32_t>(0x60)[0] = r.range(0, 3), arena<int32_t>(0x60)[1] = r.range(0, 3);
                                },
                                [](void *fn) {
                                    return Result{((uint32_t(__thiscall *)(const void *, void *, int32_t, int32_t))fn)(arena<void>(0x100), arena<void>(0x40), arena<int32_t>(0x60)[0], arena<int32_t>(0x60)[1])};
                                }};
    quick(fireworks_point_9e8790);
    // Scenery::Value8605F0: 1-4 entries, the last or not, kinds 0-7 of the game's tables
    Test scenery_value_8605f0{0x8605f0,
                              [=](Rng &r) {
                                  uint8_t *self = arena<uint8_t>(0x100), *part = arena<uint8_t>(0x200);
                                  *(uint32_t *)(self + 0xc) = (uint32_t)(uintptr_t)part;
                                  *(int32_t *)(self + 0x10) = r.range(0, 7);
                                  const int n = r.range(1, 4);
                                  *(int32_t *)(part + 0x14) = n;
                                  *(uint32_t *)(part + 0x18) = (uint32_t)(uintptr_t)arena<void>(0x240);
                                  for (int k = 0; k < 4; k++) {
                                      arena<uint32_t>(0x240)[k] = (uint32_t)(uintptr_t)arena<void>(0x300 + k * 0x40);
                                      *(float *)arena<uint8_t>(0x338 + k * 0x40) = r.real(-5, 5);
                                  }
                                  arena<int32_t>(0x40)[0] = r.range(0, n - 1);
                              },
                              [](void *fn) {
                                  Result x;
                                  x.f = ((long double(__thiscall *)(const void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0]);
                                  return x;
                              }};
    quick(scenery_value_8605f0);
    // Record::Make7CD120 / SceneryBehaviour::Init820B50: from records over an object whose list is empty or not
    for (uintptr_t at : {(uintptr_t)0x7cd120, (uintptr_t)0x820b50}) {
        Test x{at,
               [=](Rng &r) {
                   for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next(), arena<uint32_t>(0x200)[i] = r.next();
                   uint32_t *object = arena<uint32_t>(0x300), *head = arena<uint32_t>(0x380), *first = arena<uint32_t>(0x3c0);
                   for (int i = 0; i < 0x10; i++) object[i] = r.next();
                   object[8] = (uint32_t)(uintptr_t)head;
                   head[0] = r.range(0, 1) ? (uint32_t)(uintptr_t)first : (uint32_t)(uintptr_t)head;
                   head[1] = r.next(), first[0] = r.next(), first[1] = r.next();
                   arena<uint32_t>(0x200)[at == 0x7cd120 ? 2 : 1] = (uint32_t)(uintptr_t)object;
               },
               [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, const void *))fn)(arena<void>(0x100), arena<void>(0x200))}; }};
        quick(x);
    }
    // Effect::Init488290 / Window::Init6A3B60: random values
    Test effect_init_488290{0x488290,
                            [=](Rng &r) {
                                for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next();
                                for (int i = 0; i < 8; i++) arena<uint32_t>(0x40)[i] = r.next();
                            },
                            [](void *fn) {
                                const uint32_t *a = arena<uint32_t>(0x40);
                                return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t, uint32_t, const void *, const void *))fn)(arena<void>(0x100), a[0], a[1], a + 2, a + 4)};
                            }};
    quick(effect_init_488290);
    Test window_init_6a3b60{0x6a3b60,
                            [=](Rng &r) {
                                for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x100)[i] = r.next();
                                for (int i = 0; i < 8; i++) arena<uint32_t>(0x40)[i] = r.next();
                            },
                            [](void *fn) {
                                const uint32_t *a = arena<uint32_t>(0x40);
                                return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, const void *, const void *, uint32_t))fn)(arena<void>(0x100), a, a + 2, a[4])};
                            }};
    quick(window_init_6a3b60);
    // Window::Ring680AB0: rings of room 1-6, any front, counts up to full
    Test window_ring_680ab0{0x680ab0,
                            [=](Rng &r) {
                                uint32_t *self = arena<uint32_t>(0x100);
                                self[0] = (uint32_t)(uintptr_t)arena<void>(0x200);
                                self[1] = r.range(1, 6), self[2] = r.range(0, 20), self[3] = r.range(0, self[1]);
                                for (int i = 0; i < 18; i++) arena<uint32_t>(0x200)[i] = r.next();
                                for (int i = 0; i < 3; i++) arena<uint32_t>(0x40)[i] = r.next();
                            },
                            [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40)) & 0xff}; }};
    quick(window_ring_680ab0);
    // Water::Entry94DC80: entries 0-3, random values
    Test water_entry_94dc80{0x94dc80,
                            [=](Rng &r) {
                                *(uint32_t *)arena<uint8_t>(0x2b4) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                for (int i = 0; i < 0x60; i++) arena<uint32_t>(0x400)[i] = r.next();
                                for (int i = 0; i < 6; i++) arena<uint32_t>(0x40)[i] = r.next();
                                arena<int32_t>(0x60)[0] = r.range(0, 3);
                            },
                            [](void *fn) {
                                return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, int32_t, const void *, const void *))fn)(arena<void>(0x100), arena<int32_t>(0x60)[0], arena<void>(0x40), arena<void>(0x4c))};
                            }};
    quick(water_entry_94dc80);
    // Terrain::Clear8FFFE0: entries 0-3 on or off, free lists of 8
    Test terrain_clear_8fffe0{0x8fffe0,
                              [=](Rng &r) {
                                  uint8_t *self = arena<uint8_t>(0x100);
                                  *(uint32_t *)(self + 0xa8) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                  for (int i = 0; i < 0x60; i++) arena<uint32_t>(0x400)[i] = r.next();
                                  for (int k = 0; k < 4; k++) arena<uint8_t>(0x400 + k * 0x60)[0] = r.range(0, 1) ? (uint8_t)r.range(1, 255) : 0;
                                  *(uint32_t *)(self + 0xac) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                  *(int32_t *)(self + 0xb4) = 8, *(int32_t *)(self + 0xb0) = r.range(1, 8);
                                  for (int i = 0; i < 9; i++) arena<uint32_t>(0x200)[i] = r.next();
                                  arena<int32_t>(0x40)[0] = r.range(0, 3);
                              },
                              [](void *fn) {
                                  ((void(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0]);
                                  return Result{0};
                              }};
    quick(terrain_clear_8fffe0);
    // Terrain::Entry8FE1E0: entries 0-3, random places
    Test terrain_entry_8fe1e0{0x8fe1e0,
                              [=](Rng &r) {
                                  *(uint32_t *)arena<uint8_t>(0x1a8) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                  for (int i = 0; i < 0x60; i++) arena<float>(0x400)[i] = r.real(-100, 100);
                                  arena<int32_t>(0x40)[0] = r.range(0, 3), arena<float>(0x40)[1] = r.real(-100, 100), arena<float>(0x40)[2] = r.real(-100, 100);
                              },
                              [](void *fn) {
                                  return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, int32_t, float, float))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<float>(0x40)[1], arena<float>(0x40)[2])};
                              }};
    quick(terrain_entry_8fe1e0);
    // Labels::Value47DDC0: values below 0, inside and past 1
    Test labels_value_47ddc0{0x47ddc0,
                             [=](Rng &r) { arena<float>(0x40)[0] = r.range(0, 5) == 0 ? -0.0f : r.real(-1, 2); },
                             [](void *fn) {
                                 ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x40)[0]);
                                 return Result{0};
                             }};
    quick(labels_value_47ddc0);
    // Ride::Clear4BF580: a random matrix's room
    Test ride_clear_4bf580{0x4bf580,
                           [=](Rng &r) { for (int i = 0; i < 16; i++) arena<uint32_t>(0x200)[i] = r.next(); },
                           [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, uint32_t, void *))fn)(arena<void>(0x100), 7, arena<void>(0x200))}; }};
    quick(ride_clear_4bf580);
    // Camera::PartCD7F60: a part or none
    Test camera_part_cd7f60{0xcd7f60,
                            [=](Rng &r) {
                                *(uint32_t *)arena<uint8_t>(0x108) = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x200) : 0;
                                *(uint32_t *)arena<uint8_t>(0x210) = (uint32_t)(uintptr_t)arena<void>(0x240);
                                for (int c = 0; c < 3; c++) arena<float>(0x240)[c] = r.real(-50, 50), arena<uint32_t>(0x40)[c] = r.next();
                            },
                            [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, void *))fn)(arena<void>(0x100), arena<void>(0x40))}; }};
    quick(camera_part_cd7f60);
    // Car::Time502070: times that end below 0, inside and past 4 pi
    Test car_time_502070{0x502070,
                         [=](Rng &r) { *(float *)arena<uint8_t>(0x128) = r.real(-1, 14), arena<float>(0x40)[0] = r.real(-3000, 3000); },
                         [](void *fn) {
                             ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x40)[0]);
                             return Result{0};
                         }};
    quick(car_time_502070);
    // Path::Entry7338D0: entries -1..3, on or off
    Test path_entry_7338d0{0x7338d0,
                           [=](Rng &r) {
                               *(uint32_t *)arena<uint8_t>(0x10c) = (uint32_t)(uintptr_t)arena<void>(0x400);
                               for (int i = 0; i < 4 * 0x98 / 4; i++) arena<uint32_t>(0x400)[i] = r.next();
                               for (int k = 0; k < 4; k++) arena<uint8_t>(0x400 + k * 0x98)[0] = r.range(0, 1) ? (uint8_t)r.range(1, 255) : 0;
                               arena<int32_t>(0x40)[0] = r.range(-1, 3);
                               arena<uint32_t>(0x60)[0] = r.next(), arena<uint32_t>(0x60)[1] = r.next();
                           },
                           [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, void *, int32_t))fn)(arena<void>(0x100), arena<void>(0x60), arena<int32_t>(0x40)[0])}; }};
    quick(path_entry_7338d0);
    // Resource::EntryC30890: chains of 1-4 entries through 8 (n -1 at times)
    Test resource_entry_c30890{0xc30890,
                               [=](Rng &r) {
                                   uint8_t *self = arena<uint8_t>(0x100);
                                   *(uint32_t *)(self + 0x20) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                   *(int32_t *)(self + 0x24) = r.range(-1, 7), *(int32_t *)(self + 0x2c) = r.range(0, 20);
                                   for (int i = 0; i < 32; i++) arena<uint32_t>(0x200)[i] = r.next();
                                   int order[8] = {0, 1, 2, 3, 4, 5, 6, 7};
                                   for (int i = 7; i > 0; i--) { const int j = r.range(0, i), t = order[i]; order[i] = order[j], order[j] = t; }
                                   const int len = r.range(1, 4);
                                   for (int i = 0; i < len; i++) arena<int32_t>(0x200 + order[i] * 16)[3] = i + 1 < len ? order[i + 1] : -r.range(1, 5);
                                   arena<int32_t>(0x40)[0] = r.range(0, 7) ? order[0] : -1;
                               },
                               [](void *fn) {
                                   ((void(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0]);
                                   return Result{0};
                               }};
    quick(resource_entry_c30890);
    // Window::Step6F2490: random values and a reference with any count
    Test window_step_6f2490{0x6f2490,
                            [=](Rng &r) {
                                for (int i = 0; i < 12; i++) arena<uint32_t>(0x100)[i] = r.next();
                                for (int c = 0; c < 3; c++) arena<uint32_t>(0x40)[c] = r.next();
                                arena<uint32_t>(0x50)[0] = (uint32_t)(uintptr_t)arena<void>(0x60);
                                arena<uint32_t>(0x60)[0] = r.next();
                                arena<float>(0x70)[0] = r.real(-30, 30), arena<uint32_t>(0x70)[1] = r.next(), arena<uint32_t>(0x70)[2] = r.next();
                            },
                            [](void *fn) {
                                return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, float, const void *, const void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<float>(0x70)[0], arena<void>(0x40), arena<void>(0x50), arena<uint32_t>(0x70)[1], arena<uint32_t>(0x70)[2])};
                            }};
    quick(window_step_6f2490);
    // Animal::SetB9A1C0: slots -1..3
    Test animal_set_b9a1c0{0xb9a1c0,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               for (int i = 0; i < 0x2c; i++) arena<uint32_t>(0x100)[i] = r.next();
                               *(int32_t *)(self + 0xac) = r.range(-1, 3);
                               *(uint32_t *)(self + 0x74) = (uint32_t)(uintptr_t)arena<void>(0x400);
                               for (int i = 0; i < 4 * 0xb8 / 4; i++) arena<uint32_t>(0x400)[i] = r.next();
                           },
                           [](void *fn) {
                               ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                               return Result{0};
                           }};
    quick(animal_set_b9a1c0);
    // AttractionInstance::RecentTotal: counts of either sign (large ones at times)
    Test attraction_recent_total{0x4bfa20,
                                 [=](Rng &r) {
                                     *(uint32_t *)arena<uint8_t>(0x19c) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                     for (int i = 0; i < 30; i++) arena<int32_t>(0x200)[i] = r.range(0, 3) ? r.range(-50, 400) : (int32_t)r.next();
                                 },
                                 [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(const void *))fn)(arena<void>(0x100))}; }};
    quick(attraction_recent_total);
    // Guest::Init756BC0: a guest whose list is empty (its own back) or not, or none
    Test guest_init_756bc0{0x756bc0,
                           [=](Rng &r) {
                               for (int i = 0; i < 8; i++) arena<uint32_t>(0x100)[i] = r.next();
                               uint32_t *list = arena<uint32_t>(0x300), *last = arena<uint32_t>(0x340);
                               *(uint32_t *)arena<uint8_t>(0x200 + 0x518) = (uint32_t)(uintptr_t)list;
                               list[0] = r.next(), list[1] = r.range(0, 1) ? (uint32_t)(uintptr_t)last : (uint32_t)(uintptr_t)list;
                               last[0] = r.next(), last[1] = r.next();
                               arena<uint32_t>(0x40)[0] = r.range(0, 4) ? (uint32_t)(uintptr_t)arena<void>(0x200) : 0;
                           },
                           [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, const void *))fn)(arena<void>(0x100), (const void *)(uintptr_t)arena<uint32_t>(0x40)[0])}; }};
    quick(guest_init_756bc0);
    // TrackPiece::Index5D1670: small and random values
    Test track_piece_index_5d1670{0x5d1670,
                                  [=](Rng &r) { for (int i = 0; i < 10; i++) arena<int32_t>(0x100)[i] = r.range(0, 3) ? r.range(0, 6) : (int32_t)r.next(); },
                                  [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(const void *))fn)(arena<void>(0x100))}; }};
    quick(track_piece_index_5d1670);
    // Guest::Check73BAB0: modes 0-3, values alike or not
    Test guest_check_73bab0{0x73bab0,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                *(uint32_t *)(self + 0x51c) = r.range(0, 2), *(int32_t *)(self + 0x520) = r.range(0, 3);
                                arena<uint32_t>(0x40)[0] = r.range(0, 2), arena<uint32_t>(0x40)[1] = r.next();
                            },
                            [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0], arena<uint32_t>(0x40)[1])}; }};
    quick(guest_check_73bab0);
    // Person::Time7F8620: times at or below 0, and sums inside and past 1
    Test person_time_7f8620{0x7f8620,
                            [=](Rng &r) {
                                *(float *)arena<uint8_t>(0x100 + 0x5b0) = r.range(0, 4) ? r.real(-0.5f, 1.2f) : 0.0f;
                                arena<float>(0x40)[0] = r.real(-0.5f, 0.8f);
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x40)[0]);
                                return Result{0};
                            }};
    quick(person_time_7f8620);
    // Animal::SetA5FE20: random positions and points
    Test animal_set_a5fe20{0xa5fe20,
                           [=](Rng &r) {
                               for (int i = 0; i < 0x14; i++) arena<float>(0x100)[i] = r.real(-100, 100);
                               for (int c = 0; c < 3; c++) arena<float>(0x40)[c] = r.real(-100, 100);
                           },
                           [](void *fn) {
                               ((void(__thiscall *)(void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40));
                               return Result{0};
                           }};
    quick(animal_set_a5fe20);
    // D3DResource::GetDesc: a buffer or none
    Test d3d_resource_get_desc{0x41fa20,
                               [=](Rng &r) {
                                   for (int i = 0; i < 12; i++) arena<uint32_t>(0x100)[i] = r.next(), arena<uint32_t>(0x200)[i] = r.next();
                                   arena<uint32_t>(0x40)[0] = r.range(0, 4) ? (uint32_t)(uintptr_t)arena<void>(0x200) : 0;
                               },
                               [](void *fn) { return Result{((uint32_t(__stdcall *)(const void *, void *))fn)(arena<void>(0x100), (void *)(uintptr_t)arena<uint32_t>(0x40)[0])}; }};
    quick(d3d_resource_get_desc);
    // WildAnimal::CountB33390: counts reaching 0 or not, a part with +0x14 set or not, or none; the game's list set (and
    // its value), put back after
    {
        static uint32_t saved_value, saved_list;
        Test x{0xb33390,
               [=](Rng &r) {
                   *(uint32_t *)0x10f2780 = r.next();
                   uint32_t *head = arena<uint32_t>(0x300), *first = arena<uint32_t>(0x340);
                   *(uint32_t *)0x10f2794 = (uint32_t)(uintptr_t)head;
                   head[0] = r.range(0, 1) ? (uint32_t)(uintptr_t)first : (uint32_t)(uintptr_t)head;
                   head[1] = r.next(), first[0] = r.next(), first[1] = r.next();
                   uint8_t *self = arena<uint8_t>(0x100);
                   for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x100)[i] = r.next();
                   *(int32_t *)(self + 0x68) = r.range(0, 2);
                   *(uint32_t *)(self + 0x54) = r.range(0, 4) ? (uint32_t)(uintptr_t)arena<void>(0x200) : 0;
                   *(uint32_t *)arena<uint8_t>(0x214) = r.range(0, 3) ? r.next() | 1 : 0;
               },
               [](void *fn) {
                   ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { saved_value = *(uint32_t *)0x10f2780, saved_list = *(uint32_t *)0x10f2794; };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x10f2780 = saved_value, *(uint32_t *)0x10f2794 = saved_list; };
        quick(x);
    }
    // Animals::PassB83A20: 0-6 entries with keys from a few, the key there or not
    Test animals_pass_b83a20{0xb83a20,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100);
                                 *(uint32_t *)(self + 4) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                 *(int32_t *)(self + 8) = r.range(0, 9) ? r.range(0, 6) : -1;
                                 for (int i = 0; i < 6; i++) arena<uint32_t>(0x200 + i * 0x14)[0] = r.range(0, 5), arena<float>(0x200 + i * 0x14)[1] = r.real(-5, 5);
                                 arena<uint32_t>(0x40)[0] = r.range(0, 6);
                             },
                             [](void *fn) {
                                 Result x;
                                 x.f = ((long double(__thiscall *)(const void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0]);
                                 return x;
                             }};
    quick(animals_pass_b83a20);
    // Car::Check50B970: on or off, values either side of both limits
    Test car_check_50b970{0x50b970,
                          [=](Rng &r) {
                              uint8_t *self = arena<uint8_t>(0x100);
                              self[0x2c0] = r.range(0, 3) ? (uint8_t)r.range(1, 255) : 0;
                              *(float *)(self + 0x2a4) = r.range(0, 5) == 0 ? 10.0f : r.real(5, 15);
                              arena<float>(0x40)[0] = r.range(0, 5) == 0 ? 12.0f : r.real(6, 18);
                          },
                          [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, float))fn)(arena<void>(0x100), arena<float>(0x40)[0]) & 0xff}; }};
    quick(car_check_50b970);
    // Park::Pass91C1D0: 0-6 entries of pairs from a few
    Test park_pass_91c1d0{0x91c1d0,
                          [=](Rng &r) {
                              uint8_t *self = arena<uint8_t>(0x100);
                              *(uint32_t *)(self + 0x110) = (uint32_t)(uintptr_t)arena<void>(0x400);
                              *(int32_t *)(self + 0x118) = r.range(0, 9) ? r.range(0, 6) : -1;
                              for (int i = 0; i < 6 * 7; i++) arena<uint32_t>(0x400)[i] = r.range(0, 2);
                              arena<uint32_t>(0x40)[0] = r.range(0, 2), arena<uint32_t>(0x40)[1] = r.range(0, 2);
                          },
                          [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0], arena<uint32_t>(0x40)[1]) & 0xff}; }};
    quick(park_pass_91c1d0);
    // Camera::SharedCE1670: the game's two point lists and count set (put back after), 0-6 points
    {
        static uint32_t saved[4];
        Test x{0xce1670,
               [=](Rng &r) {
                   *(uint32_t *)0x1400810 = (uint32_t)(uintptr_t)arena<void>(0x200);
                   *(uint32_t *)0x1400818 = (uint32_t)(uintptr_t)arena<void>(0x400);
                   *(int32_t *)0x140081c = r.range(0, 9) ? r.range(0, 6) : -1;
                   for (int i = 0; i < 24; i++) arena<uint32_t>(0x200)[i] = r.next(), arena<uint32_t>(0x400)[i] = r.next();
               },
               [](void *fn) {
                   return Result{(uint32_t)(uintptr_t)((void *(__stdcall *)())fn)()};
               }};
        x.prepare = [](const rewrite::Entry *) { memcpy(saved, (void *)0x1400810, 16); };
        x.finish = [](const rewrite::Entry *) { memcpy((void *)0x1400810, saved, 16); };
        quick(x);
    }
    // Park::Pass884B50 / Pass8852D0: 0-6 values from a few, v among them or not
    for (uintptr_t at : {(uintptr_t)0x884b50, (uintptr_t)0x8852d0}) {
        Test x{at,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   const int32_t n = r.range(0, 9) ? r.range(0, 6) : -1;
                   *(int32_t *)(self + 0xf4) = n, *(int32_t *)(self + 0xf8) = n;
                   *(uint32_t *)(self + 0xfc) = (uint32_t)(uintptr_t)arena<void>(0x400);
                   *(uint32_t *)(self + 0x100) = (uint32_t)(uintptr_t)arena<void>(0x400);
                   *(float *)(self + 0xdc) = r.real(-5, 5);
                   for (int i = 0; i < 12; i++) arena<float>(0x400)[i] = (float)r.range(0, 4);
                   arena<float>(0x40)[0] = r.range(0, 3) ? (float)r.range(-1, 4) : r.real(-1, 4);
               },
               [](void *fn) {
                   Result x;
                   x.f = ((long double(__thiscall *)(const void *, float))fn)(arena<void>(0x100), arena<float>(0x40)[0]);
                   return x;
               }};
        quick(x);
    }
    // Person::Pass7F0300: twelve values from a few, v there (first, middle, last) or not
    Test person_pass_7f0300{0x7f0300,
                            [=](Rng &r) {
                                for (int i = 0; i < 12; i++) arena<uint32_t>(0x100 + 0x17c)[i] = r.range(0, 15);
                                arena<uint32_t>(0x40)[0] = r.range(0, 15);
                            },
                            [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0])}; }};
    quick(person_pass_7f0300);
    // Engine::Pass40C490: versions below, at and past 0x40000000, 0-5 entries set or not
    Test engine_pass_40c490{0x40c490,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                const int k = r.range(0, 3);
                                *(int32_t *)(self + 0x3c) = k == 0 ? 0x40000000 : k == 1 ? 0x40000001 : (int32_t)r.next();
                                *(int32_t *)(self + 0x38) = r.range(0, 9) ? r.range(0, 5) : -1;
                                *(uint32_t *)(self + 0x2c) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                for (int i = 0; i < 5 * 9; i++) arena<uint32_t>(0x400)[i] = r.next();
                                for (int e = 0; e < 5; e++) arena<uint8_t>(0x400 + e * 0x24)[0] = r.range(0, 1) ? (uint8_t)r.range(1, 255) : 0;
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                                return Result{0};
                            }};
    quick(engine_pass_40c490);
    // Window::FindA5B920: 0-4 groups of 0-4 items, n from -1 to past the end
    Test window_find_a5b920{0xa5b920,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                const int groups = r.range(0, 4);
                                *(int32_t *)(self + 0x64) = groups;
                                *(uint32_t *)(self + 0x60) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                int total = 0;
                                for (int g = 0; g < 4; g++) {
                                    const int c = r.range(0, 4);
                                    if (g < groups) total += c;
                                    arena<uint32_t>(0x200)[g * 2] = (uint32_t)(uintptr_t)arena<void>(0x300 + g * 0x20);
                                    arena<uint32_t>(0x200)[g * 2 + 1] = c;
                                }
                                for (int i = 0; i < 32; i++) arena<uint32_t>(0x300)[i] = r.next();
                                arena<int32_t>(0x40)[0] = r.range(0, total + 1);
                            },
                            [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0])}; }};
    quick(window_find_a5b920);
    // Resource::FindC35F10: 0-6 entries of pairs from a few
    Test resource_find_c35f10{0xc35f10,
                              [=](Rng &r) {
                                  uint8_t *self = arena<uint8_t>(0x100);
                                  *(uint32_t *)(self + 0xb4) = r.range(0, 6);
                                  *(uint32_t *)(self + 0xa0) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                  for (int i = 0; i < 18; i++) arena<uint32_t>(0x400)[i] = r.range(0, 2);
                                  arena<uint32_t>(0x40)[0] = r.range(0, 2), arena<uint32_t>(0x40)[1] = r.range(0, 2);
                              },
                              [](void *fn) { return Result{(uint32_t)(uintptr_t)((const void *(__thiscall *)(const void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0], arena<uint32_t>(0x40)[1])}; }};
    quick(resource_find_c35f10);
    // Labels::Part475610: a part or none, each place given or not
    Test labels_part_475610{0x475610,
                            [=](Rng &r) {
                                *(uint32_t *)arena<uint8_t>(0x144) = r.range(0, 4) ? (uint32_t)(uintptr_t)arena<void>(0x200) : 0;
                                for (int i = 0; i < 4; i++) arena<uint32_t>(0x200)[i] = r.next(), arena<uint32_t>(0x60)[i] = r.next();
                                for (int i = 0; i < 3; i++) arena<uint32_t>(0x40)[i] = r.range(0, 2) ? (uint32_t)(uintptr_t)arena<void>(0x60 + i * 4) : 0;
                            },
                            [](void *fn) {
                                const uint32_t *a = arena<uint32_t>(0x40);
                                return Result{((uint32_t(__thiscall *)(const void *, void *, void *, void *))fn)(arena<void>(0x100), (void *)(uintptr_t)a[0], (void *)(uintptr_t)a[1], (void *)(uintptr_t)a[2]) & 0xff};
                            }};
    quick(labels_part_475610);
    // Regions::Merge417010: random bounds either side
    Test regions_merge_417010{0x417010,
                              [=](Rng &r) {
                                  for (int i = 0; i < 0x12; i++) arena<int32_t>(0x100)[i] = r.range(-20, 20), arena<int32_t>(0x200)[i] = r.range(-20, 20);
                              },
                              [](void *fn) {
                                  ((void(__thiscall *)(void *, const void *))fn)(arena<void>(0x100), arena<void>(0x200));
                                  return Result{0};
                              }};
    quick(regions_merge_417010);
    // Static::Reset4984C0: four tables of 0-4 entries set or not
    Test static_reset_4984c0{0x4984c0,
                             [=](Rng &r) {
                                 for (int k = 0; k < 4; k++) {
                                     uint8_t *t = arena<uint8_t>(0x200 + k * 0x40);
                                     *(uint32_t *)arena<uint8_t>(0x160 + k * 4) = (uint32_t)(uintptr_t)t;
                                     *(int32_t *)(t + 0x24) = r.range(0, 9) ? r.range(0, 4) : -1;
                                     *(uint32_t *)(t + 0x18) = (uint32_t)(uintptr_t)arena<void>(0x400 + k * 0x80);
                                     for (int i = 0; i < 0x18; i++) arena<uint32_t>(0x400 + k * 0x80)[i] = r.range(0, 1) ? r.next() : 0;
                                 }
                                 arena<uint8_t>(0x170)[0] = (uint8_t)r.next();
                             },
                             [](void *fn) {
                                 ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                                 return Result{0};
                             }};
    quick(static_reset_4984c0);
    // Window::Count6C80C0: rows 0-2 of eight slots, some used
    Test window_count_6c80c0{0x6c80c0,
                             [=](Rng &r) {
                                 for (int i = 0; i < 24; i++) arena<uint32_t>(0x130)[i] = r.range(0, 1) ? (uint32_t)(uintptr_t)arena<void>(0x400 + i * 0x60) : 0;
                                 for (int i = 0; i < 24 * 0x18; i++) arena<uint32_t>(0x400)[i] = r.next();
                                 arena<int32_t>(0x40)[0] = r.range(0, 2);
                             },
                             [](void *fn) {
                                 return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0])};
                             }};
    quick(window_count_6c80c0);
    // Fireworks::Time9E0E20: the game's flag and offset set (put back after)
    {
        static uint8_t saved_flag;
        static uint32_t saved_offset[3];
        Test x{0x9e0e20,
               [=](Rng &r) {
                   *(uint8_t *)0x10f0e89 = r.range(0, 3) ? (uint8_t)r.range(1, 255) : 0;
                   for (int c = 0; c < 3; c++) ((float *)0x14002c0)[c] = r.real(-50, 50);
                   for (int i = 0; i < 8; i++) arena<float>(0x100)[i] = r.real(-50, 50);
               },
               [](void *fn) {
                   ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { saved_flag = *(uint8_t *)0x10f0e89, memcpy(saved_offset, (void *)0x14002c0, 12); };
        x.finish = [](const rewrite::Entry *) { *(uint8_t *)0x10f0e89 = saved_flag, memcpy((void *)0x14002c0, saved_offset, 12); };
        quick(x);
    }
    // Animal::SetAD78F0: on or off, 0-5 entries, random scales
    Test animal_set_ad78f0{0xad78f0,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               *(float *)(self + 0x48) = r.real(0.2f, 5);
                               self[0x50] = r.range(0, 3) ? (uint8_t)r.range(1, 255) : 0;
                               *(int32_t *)(self + 0x4c) = r.range(0, 9) ? r.range(0, 5) : -1;
                               *(uint32_t *)(self + 0x40) = (uint32_t)(uintptr_t)arena<void>(0x400);
                               for (int i = 0; i < 40; i++) arena<float>(0x400)[i] = r.real(-10, 10);
                               arena<float>(0x40)[0] = r.real(-5, 5);
                           },
                           [](void *fn) {
                               ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x40)[0]);
                               return Result{0};
                           }};
    quick(animal_set_ad78f0);
    // Scenery::Time8B0470: angles that end below, inside and past the range
    Test scenery_time_8b0470{0x8b0470,
                             [=](Rng &r) { *(float *)arena<uint8_t>(0x114) = r.real(0, 3), arena<float>(0x40)[0] = r.real(-2000, 2000); },
                             [](void *fn) {
                                 ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x40)[0]);
                                 return Result{0};
                             }};
    quick(scenery_time_8b0470);
    // Water::Entry94DCC0: flags the same or new, 0-4 children
    Test water_entry_94dcc0{0x94dcc0,
                            [=](Rng &r) {
                                *(uint32_t *)arena<uint8_t>(0x2b4) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                for (int i = 0; i < 0x60; i++) arena<uint32_t>(0x400)[i] = r.next();
                                for (int i = 0; i < 0x60; i++) arena<uint32_t>(0x800)[i] = r.next();
                                const uint8_t f = (uint8_t)r.range(0, 2);
                                for (int k = 0; k < 4; k++) {
                                    uint8_t *e = arena<uint8_t>(0x400 + k * 0x60);
                                    e[0x54] = r.range(0, 1) ? f : (uint8_t)r.range(0, 2);
                                    *(uint32_t *)(e + 0x38) = (uint32_t)(uintptr_t)arena<void>(0x800);
                                    *(int32_t *)(e + 0x3c) = r.range(0, 9) ? r.range(0, 4) : -1;
                                }
                                arena<int32_t>(0x40)[0] = r.range(0, 3), arena<uint32_t>(0x40)[1] = (r.next() & ~0xffu) | f;
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(const void *, int32_t, uint32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<uint32_t>(0x40)[1]);
                                return Result{0};
                            }};
    quick(water_entry_94dcc0);
    // Fence::StepABC2B0: random boxes either side
    Test fence_step_abc2b0{0xabc2b0,
                           [=](Rng &r) {
                               for (int i = 0; i < 4; i++) arena<int32_t>(0x164)[i] = r.range(-10, 10), arena<int32_t>(0x40)[i] = r.range(-10, 10);
                           },
                           [](void *fn) {
                               const int32_t *a = arena<int32_t>(0x40);
                               ((void(__thiscall *)(void *, int32_t, int32_t, int32_t, int32_t))fn)(arena<void>(0x100), a[0], a[1], a[2], a[3]);
                               return Result{0};
                           }};
    quick(fence_step_abc2b0);
    // Shader::EntryCC4840: indexes within and past the count
    Test shader_entry_cc4840{0xcc4840,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100);
                                 *(uint32_t *)(self + 0x14) = r.range(0, 6);
                                 *(uint32_t *)(self + 0x18) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                 *(uint32_t *)(self + 0x20) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                 for (int i = 0; i < 8; i++) arena<uint32_t>(0x200)[i] = r.range(0, 20);
                                 arena<uint32_t>(0x40)[0] = r.range(0, 7);
                             },
                             [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0])}; }};
    quick(shader_entry_cc4840);
    // Labels::Set475720: on or off, weights at or below 0 and above
    Test labels_set_475720{0x475720,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               self[0x6d] = r.range(0, 3) ? 0 : (uint8_t)r.range(1, 255);
                               *(float *)(self + 0x1c) = r.real(0.5f, 10), *(float *)(self + 0x20) = r.real(-10, 10);
                               *(float *)(self + 0x70) = r.range(0, 4) ? r.real(0, 8) : -r.real(0, 2);
                               arena<float>(0x40)[0] = r.real(-10, 10);
                           },
                           [](void *fn) {
                               ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x40)[0]);
                               return Result{0};
                           }};
    quick(labels_set_475720);
    // Labels::Count475DA0: on or off (cl), counts from -2 up (the game's count compared), a list of 0-4 labels set and put
    // back after
    {
        static uint32_t saved_list, saved_count;
        Test x{0x475da0,
               [=](Rng &r) {
                   *(int32_t *)0x12f0140 = r.range(-2, 5);
                   const int n = r.range(0, 4);
                   uint32_t *end = arena<uint32_t>(0x300);
                   end[0] = 0, end[2] = (uint32_t)(uintptr_t)arena<void>(0x500);
                   for (int k = 0; k < n; k++) {
                       uint32_t *node = arena<uint32_t>(0x200 + k * 0x10);
                       node[0] = (uint32_t)(uintptr_t)(k + 1 < n ? arena<void>(0x200 + (k + 1) * 0x10) : (void *)end);
                       node[2] = (uint32_t)(uintptr_t)arena<void>(0x400 + k * 0x60);
                   }
                   for (int i = 0; i < 0x80; i++) arena<uint32_t>(0x400)[i] = r.next();
                   *(uint32_t *)0x10eca14 = (uint32_t)(uintptr_t)(n ? arena<void>(0x200) : (void *)end);
                   arena<uint32_t>(0x40)[0] = (r.next() & ~0xffu) | (r.range(0, 1) ? (uint32_t)r.range(1, 255) : 0);
               },
               [](void *fn) {
                   ((void(__fastcall *)(uint32_t))fn)(arena<uint32_t>(0x40)[0]);
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { saved_list = *(uint32_t *)0x10eca14, saved_count = *(uint32_t *)0x12f0140; };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x10eca14 = saved_list, *(uint32_t *)0x12f0140 = saved_count; };
        t.push_back(x);   // (its writes are game globals: the data section is compared)
    }
    // Rides::Pass50EDB0: 0-5 entries, the part's byte +9 0, 1 or more
    Test rides_pass_50edb0{0x50edb0,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x1000);
                               *(int32_t *)(self + 0x8c0) = r.range(0, 9) ? r.range(0, 5) : -1;
                               *(uint32_t *)(self + 0x8c4) = (uint32_t)(uintptr_t)arena<void>(0x400);
                               *(uint32_t *)(self + 0x414) = (uint32_t)(uintptr_t)arena<void>(0x200);
                               for (int i = 0; i < 0x20; i++) arena<uint32_t>(0x400)[i] = r.next();
                               arena<uint8_t>(0x200)[9] = (uint8_t)r.range(0, 3);
                               *(uint32_t *)arena<uint8_t>(0x30c) = r.next();
                           },
                           [](void *fn) {
                               ((void(__thiscall *)(const void *))fn)(arena<void>(0x1000));
                               return Result{0};
                           }};
    quick(rides_pass_50edb0);
    // Park::Pass7575E0: 0-5 entries with keys from a few (counts 1-4), n of either sign
    Test park_pass_7575e0{0x7575e0,
                          [=](Rng &r) {
                              uint8_t *self = arena<uint8_t>(0x100);
                              *(int32_t *)(self + 0xa4) = r.range(0, 9) ? r.range(0, 5) : -1;
                              *(uint32_t *)(self + 0xa8) = (uint32_t)(uintptr_t)arena<void>(0x200);
                              for (int k = 0; k < 5; k++) {
                                  uint32_t *e = arena<uint32_t>(0x200 + k * 12);
                                  e[0] = r.range(0, 4), e[1] = r.range(1, 4), e[2] = (uint32_t)(uintptr_t)arena<void>(0x300 + k * 0x10);
                              }
                              for (int i = 0; i < 20; i++) arena<uint32_t>(0x300)[i] = r.next();
                              arena<uint32_t>(0x40)[0] = r.range(0, 5), arena<int32_t>(0x40)[1] = r.range(-20, 20);
                          },
                          [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, uint32_t, int32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0], arena<int32_t>(0x40)[1])}; }};
    quick(park_pass_7575e0);
    // Rides::Pass4CC830: 0-4 entries in use among 8 (the count never more than are in use)
    Test rides_pass_4cc830{0x4cc830,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               *(uint32_t *)(self + 0x94) = (uint32_t)(uintptr_t)arena<void>(0x400);
                               int used = 0;
                               for (int i = 0; i < 8 * 11; i++) arena<uint32_t>(0x400)[i] = r.next();
                               for (int k = 0; k < 8; k++) {
                                   const bool on = r.range(0, 1);
                                   arena<uint8_t>(0x400 + k * 0x2c)[0] = on ? (uint8_t)r.range(1, 255) : 0;
                                   used += on;
                               }
                               *(int32_t *)(self + 0x9c) = r.range(0, 9) ? r.range(0, used) : -1;
                               arena<int32_t>(0x40)[0] = r.range(-50, 50), arena<int32_t>(0x40)[1] = r.range(-50, 50);
                           },
                           [](void *fn) {
                               ((void(__thiscall *)(const void *, int32_t, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<int32_t>(0x40)[1]);
                               return Result{0};
                           }};
    quick(rides_pass_4cc830);
    // Table::StepCA84A0: signed bounds (crossed at times), fractions past both ends
    Test table_step_ca84a0{0xca84a0,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               *(uint32_t *)(self + 4) = (uint32_t)(uintptr_t)arena<void>(0x140);
                               *(int32_t *)(self + 8) = r.range(0, 2) ? r.range(-500, 500) : (int32_t)r.next() >> 2;
                               *(int32_t *)(self + 0xc) = r.range(0, 2) ? r.range(-500, 500) : (int32_t)r.next() >> 2;
                               arena<uint32_t>(0x140)[0] = r.next();
                               arena<float>(0x40)[0] = r.real(-0.3f, 1.3f);
                           },
                           [](void *fn) {
                               ((void(__thiscall *)(const void *, float))fn)(arena<void>(0x100), arena<float>(0x40)[0]);
                               return Result{0};
                           }};
    quick(table_step_ca84a0);
    // Effect::Pass412EA0: 32 slots (some empty) with keys from a few, the key there or not
    Test effect_pass_412ea0{0x412ea0,
                            [=](Rng &r) {
                                *(uint32_t *)arena<uint8_t>(0x104) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                for (int k = 0; k < 32; k++) {
                                    arena<uint32_t>(0x200)[k] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x400 + k * 0x60) : 0;
                                    uint8_t *e = arena<uint8_t>(0x400 + k * 0x60);
                                    *(uint32_t *)(e + 0x50) = r.range(0, 40);
                                    *(uint32_t *)(e + 0x30) = (uint32_t)(uintptr_t)arena<void>(0x1400 + k * 0x20);
                                    uint32_t *head = arena<uint32_t>(0x1400 + k * 0x20);
                                    head[0] = r.range(0, 1) ? (uint32_t)(uintptr_t)(head + 4) : (uint32_t)(uintptr_t)head;
                                    head[1] = r.next(), head[4] = r.next(), head[5] = r.next();
                                }
                                for (int i = 0; i < 4; i++) arena<uint32_t>(0x60)[i] = r.next();
                                arena<uint32_t>(0x40)[0] = r.range(0, 40);
                            },
                            [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x60), arena<uint32_t>(0x40)[0])}; }};
    quick(effect_pass_412ea0);
    // Animals::PassA5B970: 0-4 groups of 0-4 items (bits 0-40, past 31 at times)
    Test animals_pass_a5b970{0xa5b970,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100);
                                 *(int32_t *)(self + 0x64) = r.range(0, 9) ? r.range(0, 4) : -1;
                                 *(uint32_t *)(self + 0x68) = r.next();
                                 *(uint32_t *)(self + 0x60) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                 for (int g = 0; g < 4; g++) {
                                     arena<uint32_t>(0x200)[g * 2] = (uint32_t)(uintptr_t)arena<void>(0x300 + g * 0x10);
                                     arena<uint32_t>(0x200)[g * 2 + 1] = r.range(0, 4);
                                 }
                                 for (int i = 0; i < 16; i++) arena<uint32_t>(0x300)[i] = r.range(0, 40);
                             },
                             [](void *fn) {
                                 ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                                 return Result{0};
                             }};
    quick(animals_pass_a5b970);
    // Scenery::Count880F60: counts reaching 0 or not, a part with +0x64 set or not, or none; the game's list set (and its
    // value), put back after
    {
        static uint32_t saved_value, saved_list;
        Test x{0x880f60,
               [=](Rng &r) {
                   *(uint32_t *)0x10f0a28 = r.next();
                   uint32_t *head = arena<uint32_t>(0x300), *first = arena<uint32_t>(0x340);
                   *(uint32_t *)0x10f0a44 = (uint32_t)(uintptr_t)head;
                   head[0] = r.range(0, 1) ? (uint32_t)(uintptr_t)first : (uint32_t)(uintptr_t)head;
                   head[1] = r.next(), first[0] = r.next(), first[1] = r.next();
                   uint8_t *self = arena<uint8_t>(0x1000);
                   for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x1000 + 0xc0)[i] = r.next();
                   *(int32_t *)(self + 0xe0) = r.range(0, 2);
                   *(uint32_t *)(self + 0xcc) = r.range(0, 4) ? (uint32_t)(uintptr_t)arena<void>(0x200) : 0;
                   *(uint32_t *)arena<uint8_t>(0x264) = r.range(0, 3) ? r.next() | 1 : 0;
               },
               [](void *fn) {
                   ((void(__thiscall *)(void *))fn)(arena<void>(0x1000));
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { saved_value = *(uint32_t *)0x10f0a28, saved_list = *(uint32_t *)0x10f0a44; };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x10f0a28 = saved_value, *(uint32_t *)0x10f0a44 = saved_list; };
        quick(x);
    }
    // Resource::CheckC3ACE0: n from -3 to past the count, random bits, the byte 0 or not
    Test resource_check_c3ace0{0xc3ace0,
                               [=](Rng &r) {
                                   uint8_t *self = arena<uint8_t>(0x100);
                                   *(int32_t *)(self + 0x2c) = r.range(0, 70);
                                   *(uint32_t *)(self + 0x88) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                   self[0xa0] = r.range(0, 1) ? 0 : (uint8_t)r.range(1, 255);
                                   for (int i = 0; i < 3; i++) arena<uint32_t>(0x200)[i] = r.next();
                                   arena<int32_t>(0x40)[0] = r.range(-3, 75);
                               },
                               [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0]) & 0xff}; }};
    quick(resource_check_c3ace0);
    // Car::Mode50AF10: modes 2-4
    Test car_mode_50af10{0x50af10,
                         [=](Rng &r) {
                             *(int32_t *)arena<uint8_t>(0x1c8 + 0x100) = r.range(2, 4);
                             for (int i = 0; i < 3; i++) arena<uint32_t>(0x1f0 + 0x100)[i] = r.next(), arena<uint32_t>(0x40)[i] = r.next();
                         },
                         [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, void *))fn)(arena<void>(0x100), arena<void>(0x40))}; }};
    quick(car_mode_50af10);
    // Window::Flag6809F0: values either side, modes 0-4
    Test window_flag_6809f0{0x6809f0,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x1000);
                                *(float *)(self + 0x358) = r.real(-5, 5), *(int32_t *)(self + 0x354) = r.range(0, 4);
                                *(uint32_t *)arena<uint8_t>(0x294) = r.next();
                                arena<float>(0x40)[0] = r.real(-5, 5);
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(void *, void *, float))fn)(arena<void>(0x1000), arena<void>(0x200), arena<float>(0x40)[0]);
                                return Result{0};
                            }};
    quick(window_flag_6809f0);
    // Device::SharedC08890: the game's once bit set or not (the shared list compared, all put back after)
    {
        static uint32_t saved[7];
        Test x{0xc08890,
               [=](Rng &r) { for (int i = 0; i < 7; i++) ((uint32_t *)0x13d3280)[i] = r.next(); },
               [](void *fn) { return Result{((uint32_t(__stdcall *)())fn)()}; }};
        x.prepare = [](const rewrite::Entry *) { memcpy(saved, (void *)0x13d3280, 28); };
        x.finish = [](const rewrite::Entry *) { memcpy((void *)0x13d3280, saved, 28); };
        t.push_back(x);
    }
    // Text::ParseC099A0: texts of 0-40 bytes appended at any place
    Test text_parse_c099a0{0xc099a0,
                           [=](Rng &r) {
                               *(uint32_t *)arena<uint8_t>(0x1000 + 0x1010) = r.range(0, 0x100);
                               for (int i = 0; i < 0x50; i++) arena<uint32_t>(0x1010)[i] = r.next();
                               const int n = r.range(0, 40);
                               char *s = arena<char>(0x200);
                               for (int i = 0; i < n; i++) s[i] = (char)r.range(1, 255);
                               s[n] = 0;
                           },
                           [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, const char *))fn)(arena<void>(0x1000), arena<char>(0x200))}; }};
    quick(text_parse_c099a0);
    // TrackPiece::Index5D1BD0: small and random values
    Test track_piece_index_5d1bd0{0x5d1bd0,
                                  [=](Rng &r) {
                                      for (int i = 0; i < 10; i++) arena<int32_t>(0x100)[i] = r.range(0, 3) ? r.range(0, 2) : (int32_t)r.next();
                                      arena<int32_t>(0x40)[0] = r.range(0, 200), arena<int32_t>(0x40)[1] = r.range(0, 3) ? r.range(0, 3) : (int32_t)r.next();
                                  },
                                  [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(const void *, int32_t, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<int32_t>(0x40)[1])}; }};
    quick(track_piece_index_5d1bd0);
    // VertexDeclaration::AddBDF790: 0-16 entries with keys from a few, high bits set at times
    Test vertex_declaration_add_bdf790{0xbdf790,
                                       [=](Rng &r) {
                                           uint8_t *self = arena<uint8_t>(0x1000);
                                           *(uint32_t *)self = (uint32_t)(uintptr_t)arena<void>(0x200);
                                           arena<uint32_t>(0x200)[9] = r.next();
                                           for (int i = 0; i < 16; i++) {
                                               uint16_t *e = arena<uint16_t>(0x100c + i * 4);
                                               e[0] = (uint16_t)(r.range(0, 2) << 8 | r.range(0, 2));
                                               e[1] = (uint16_t)(r.range(0, 3) ? r.next() & 0x3fff : r.next());
                                           }
                                           *(uint32_t *)(self + 0x20c) = 4 * r.range(0, 16);
                                           arena<uint32_t>(0x40)[0] = (r.next() & ~0xffu) | r.range(0, 2);
                                           arena<uint32_t>(0x40)[1] = r.range(0, 2);
                                       },
                                       [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, uint32_t, uint32_t))fn)(arena<void>(0x1000), arena<uint32_t>(0x40)[0], arena<uint32_t>(0x40)[1])}; }};
    quick(vertex_declaration_add_bdf790);
    // Person::Reset801C90: 0-6 items set or not
    Test person_reset_801c90{0x801c90,
                             [=](Rng &r) {
                                 int32_t *self = arena<int32_t>(0x100);
                                 self[0] = r.range(0, 9) ? r.range(0, 6) : -1, self[1] = (int32_t)r.next();
                                 self[2] = (int32_t)(uintptr_t)arena<void>(0x200);
                                 for (int k = 0; k < 6; k++) {
                                     arena<uint32_t>(0x200)[k] = (uint32_t)(uintptr_t)arena<void>(0x300 + k * 0x10);
                                     for (int i = 0; i < 4; i++) arena<uint32_t>(0x300 + k * 0x10)[i] = r.next();
                                     arena<uint8_t>(0x300 + k * 0x10)[8] = r.range(0, 1) ? (uint8_t)r.range(1, 255) : 0;
                                 }
                             },
                             [](void *fn) {
                                 ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                                 return Result{0};
                             }};
    quick(person_reset_801c90);
    // Park::Pass774010: 0-5 entries with keys from a few
    Test park_pass_774010{0x774010,
                          [=](Rng &r) {
                              uint8_t *self = arena<uint8_t>(0x1000);
                              *(int32_t *)(self + 0x48) = r.range(0, 9) ? r.range(0, 5) : -1;
                              *(uint32_t *)(self + 0x60) = (uint32_t)(uintptr_t)arena<void>(0x200);
                              *(uint32_t *)(self + 0x13c) = (uint32_t)(uintptr_t)arena<void>(0x400);
                              for (int k = 0; k < 5; k++) {
                                  arena<uint32_t>(0x200 + k * 0x14)[1] = (uint32_t)(uintptr_t)arena<void>(0x300 + k * 0x10);
                                  arena<uint32_t>(0x300 + k * 0x10)[3] = r.range(0, 4);
                              }
                              for (int i = 0; i < 20; i++) arena<uint32_t>(0x400)[i] = r.next();
                              for (int i = 0; i < 3; i++) arena<uint32_t>(0x60)[i] = r.next();
                              arena<uint32_t>(0x40)[0] = r.range(0, 5);
                          },
                          [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, uint32_t, void *))fn)(arena<void>(0x1000), arena<uint32_t>(0x40)[0], arena<void>(0x60))}; }};
    quick(park_pass_774010);
    // Window::Slot6E99B0: rows a + b of 0-2, eight slots some used
    Test window_slot_6e99b0{0x6e99b0,
                            [=](Rng &r) {
                                for (int i = 0; i < 24; i++) arena<uint32_t>(0x128)[i] = r.range(0, 1) ? (uint32_t)(uintptr_t)arena<void>(0x400 + i * 0x60) : 0;
                                for (int i = 0; i < 24 * 0x18; i++) arena<uint32_t>(0x400)[i] = r.next();
                                const int a = r.range(0, 2);
                                arena<int32_t>(0x40)[0] = a, arena<int32_t>(0x40)[1] = r.range(0, 2 - a);
                            },
                            [](void *fn) {
                                return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, int32_t, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<int32_t>(0x40)[1])};
                            }};
    quick(window_slot_6e99b0);
    // Ride::List53DDB0: sections 0-1 with lists of 1-4 nodes, steps 0-3 (never past the end)
    Test ride_list_53ddb0{0x53ddb0,
                          [=](Rng &r) {
                              *(uint32_t *)arena<uint8_t>(0x130) = (uint32_t)(uintptr_t)arena<void>(0x200);
                              int len[2];
                              for (int s = 0; s < 2; s++) {
                                  arena<uint32_t>(0x200)[s] = (uint32_t)(uintptr_t)arena<void>(0x240 + s * 0x10);
                                  len[s] = r.range(1, 4);
                                  uint32_t *end = arena<uint32_t>(0x700 + s * 0x10);
                                  end[0] = 0;
                                  for (int k = 0; k < len[s]; k++) {
                                      uint32_t *node = arena<uint32_t>(0x300 + s * 0x100 + k * 0x20);
                                      node[0] = (uint32_t)(uintptr_t)(k + 1 < len[s] ? arena<void>(0x300 + s * 0x100 + (k + 1) * 0x20) : (void *)end);
                                      node[2] = (uint32_t)(uintptr_t)arena<void>(0x800 + s * 0x40 + k * 8);
                                      arena<uint32_t>(0x800 + s * 0x40 + k * 8)[0] = (uint32_t)(uintptr_t)arena<void>(0x900 + s * 0x40 + k * 8);
                                      arena<uint32_t>(0x900 + s * 0x40 + k * 8)[0] = r.range(0, 100);
                                  }
                                  arena<uint32_t>(0x240 + s * 0x10)[1] = (uint32_t)(uintptr_t)arena<void>(0x300 + s * 0x100);
                              }
                              const int s = r.range(0, 1);
                              arena<int32_t>(0x40)[0] = s, arena<int32_t>(0x40)[1] = r.range(0, len[s] - 1);
                              arena<uint32_t>(0x60)[0] = r.next();
                          },
                          [](void *fn) {
                              return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, void *, int32_t, int32_t))fn)(arena<void>(0x100), arena<void>(0x60), arena<int32_t>(0x40)[0], arena<int32_t>(0x40)[1])};
                          }};
    quick(ride_list_53ddb0);
    // Resource::ValueC307D0: chains of 1-4 entries through 8 (none at times), sizes small or past 2^31, any total
    Test resource_value_c307d0{0xc307d0,
                               [=](Rng &r) {
                                   uint8_t *self = arena<uint8_t>(0x100);
                                   *(uint32_t *)(self + 0x20) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                   *(uint32_t *)(self + 0x18) = r.range(0, 2) ? r.range(1, 5000) : r.next();
                                   for (int i = 0; i < 32; i++) arena<uint32_t>(0x200)[i] = r.range(0, 2) ? r.range(0, 3000) : r.next();
                                   int order[8] = {0, 1, 2, 3, 4, 5, 6, 7};
                                   for (int i = 7; i > 0; i--) { const int j = r.range(0, i), t = order[i]; order[i] = order[j], order[j] = t; }
                                   const int len = r.range(1, 4);
                                   for (int i = 0; i < len; i++) arena<int32_t>(0x200 + order[i] * 16)[3] = i + 1 < len ? order[i + 1] : -r.range(1, 5);
                                   *(int32_t *)(self + 0x28) = r.range(0, 7) ? order[0] : -1;
                               },
                               [](void *fn) {
                                   Result x;
                                   x.f = ((long double(__thiscall *)(const void *))fn)(arena<void>(0x100));
                                   return x;
                               }};
    quick(resource_value_c307d0);
    // List::Clear: lists of 0-4 nodes, the game's free list and count set (compared, put back after)
    {
        static uint32_t saved_head, saved_count;
        Test x{0xca06a0,
               [=](Rng &r) {
                   *(uint32_t *)0x13e6e28 = r.next(), *(uint32_t *)0x13e6e30 = r.range(0, 100);
                   const int n = r.range(0, 4);
                   uint32_t *self = arena<uint32_t>(0x100), *end = arena<uint32_t>(0x300);
                   for (int i = 0; i < 6; i++) self[i] = r.next();
                   end[0] = 0;
                   for (int k = 0; k < n; k++) {
                       uint32_t *node = arena<uint32_t>(0x200 + k * 0x10);
                       node[0] = (uint32_t)(uintptr_t)(k + 1 < n ? arena<void>(0x200 + (k + 1) * 0x10) : (void *)end);
                       node[2] = (uint32_t)(uintptr_t)arena<void>(0x400 + k * 0x10);
                       arena<uint32_t>(0x400 + k * 0x10)[0] = r.next();
                   }
                   self[0] = (uint32_t)(uintptr_t)(n ? arena<void>(0x200) : (void *)end);
               },
               [](void *fn) {
                   ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { saved_head = *(uint32_t *)0x13e6e28, saved_count = *(uint32_t *)0x13e6e30; };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x13e6e28 = saved_head, *(uint32_t *)0x13e6e30 = saved_count; };
        t.push_back(x);
    }
    // Object3AFD0::CopyFrom: whole or not, a source with its base at any of a few offsets
    Test object3afd0_copy_from{0x560460,
                               [=](Rng &r) {
                                   for (int i = 0; i < 12; i++) arena<uint32_t>(0x100)[i] = r.next(), arena<uint32_t>(0x200)[i] = r.next();
                                   arena<uint32_t>(0x200)[0] = (uint32_t)(uintptr_t)arena<void>(0x300);
                                   arena<int32_t>(0x300)[1] = 4 * r.range(1, 10);
                                   arena<uint32_t>(0x40)[0] = r.range(0, 1) ? r.next() | 1 : 0;
                               },
                               [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, const void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x200), arena<uint32_t>(0x40)[0])}; }};
    quick(object3afd0_copy_from);
    // A node list of n nodes at arena `at` (0x20 apart) ending at an end node, items from `items` (0x40 apart); gives the
    // first node (the end node when n is 0).
    auto node_list_h2 = [](Rng &r, uint32_t at, int n, uint32_t items) {
        uint32_t *end = arena<uint32_t>(at + 0x100);
        end[0] = 0, end[1] = r.next(), end[2] = r.next();
        for (int k = 0; k < n; k++) {
            uint32_t *node = arena<uint32_t>(at + k * 0x20);
            node[0] = (uint32_t)(uintptr_t)(k + 1 < n ? arena<void>(at + (k + 1) * 0x20) : (void *)end);
            node[1] = r.next();
            node[2] = (uint32_t)(uintptr_t)arena<void>(items + k * 0x40);
        }
        return (uint32_t)(uintptr_t)(n ? arena<void>(at) : (void *)end);
    };
    // Ride::Find5947B0: lists of 0-5 nodes, n from -1 past the end
    Test ride_find_5947b0{0x5947b0,
                          [=](Rng &r) {
                              *(uint32_t *)arena<uint8_t>(0x1208) = node_list_h2(r, 0x200, r.range(0, 5), 0x400);
                              arena<int32_t>(0x40)[0] = r.range(-1, 6);
                          },
                          [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, int32_t))fn)(arena<void>(0x1000), arena<int32_t>(0x40)[0])}; }};
    quick(ride_find_5947b0);
    // Ride::Set59FFA0: an object whose list of 0-5 items has this ride first in one or none
    Test ride_set_59ffa0{0x59ffa0,
                         [=](Rng &r) {
                             const int n = r.range(0, 5);
                             *(uint32_t *)arena<uint8_t>(0x874) = node_list_h2(r, 0x200, n, 0x400);
                             for (int k = 0; k < 5; k++) arena<uint32_t>(0x400 + k * 0x40)[0] = r.range(0, 3) ? r.next() : (uint32_t)(uintptr_t)arena<void>(0x1000);
                             *(uint32_t *)arena<uint8_t>(0x108c) = r.next();
                         },
                         [](void *fn) {
                             ((void(__thiscall *)(void *, const void *))fn)(arena<void>(0x1000), arena<void>(0x800));
                             return Result{0};
                         }};
    quick(ride_set_59ffa0);
    // Window::Count61CEF0: counts -1..3, each part's value 0 or not
    Test window_count_61cef0{0x61cef0,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100);
                                 *(int32_t *)(self + 0x8c) = r.range(-1, 3);
                                 *(uint32_t *)(self + 0x94) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                 *(uint32_t *)(self + 0x74) = (uint32_t)(uintptr_t)arena<void>(0x220);
                                 *(uint32_t *)(self + 0x88) = (uint32_t)(uintptr_t)arena<void>(0x240);
                                 *(uint32_t *)arena<uint8_t>(0x20c) = r.range(0, 1) ? r.next() | 1 : 0;
                                 *(uint32_t *)arena<uint8_t>(0x22c) = r.range(0, 1) ? r.next() | 1 : 0;
                                 *(uint32_t *)arena<uint8_t>(0x244) = r.range(0, 1) ? r.next() | 1 : 0;
                             },
                             [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *))fn)(arena<void>(0x100)) & 0xff}; }};
    quick(window_count_61cef0);
    // Window::SlotC63730: lists of 0-5 objects, the object in it or not, owned by this window or not
    Test window_slot_c63730{0xc63730,
                            [=](Rng &r) {
                                const int n = r.range(0, 5);
                                *(uint32_t *)arena<uint8_t>(0x10b0) = node_list_h2(r, 0x200, n, 0x400);
                                const int pick = r.range(0, 5);
                                const uint32_t obj = (uint32_t)(uintptr_t)arena<void>(0x400 + pick * 0x40);
                                *(uint32_t *)arena<uint8_t>(0x400 + pick * 0x40 + 0x18) = r.range(0, 4) ? (uint32_t)(uintptr_t)arena<void>(0x1000) : r.next();
                                arena<uint32_t>(0x40)[0] = obj;
                            },
                            [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(const void *, const void *))fn)(arena<void>(0x1000), (const void *)(uintptr_t)arena<uint32_t>(0x40)[0])}; }};
    quick(window_slot_c63730);
    // Grid::Slot6FDE90: lists of 0-5 nodes (an empty item at times), slots -1 past the end
    Test grid_slot_6fde90{0x6fde90,
                          [=](Rng &r) {
                              uint8_t *self = arena<uint8_t>(0x1000);
                              *(uint32_t *)(self + 0x4c) = node_list_h2(r, 0x200, r.range(0, 5), 0x400);
                              *(int32_t *)(self + 0x40) = r.range(-1, 6);
                              for (int k = 0; k < 5; k++) {
                                  arena<uint32_t>(0x400 + k * 0x40)[1] = (uint32_t)(uintptr_t)arena<void>(0x800 + k * 0x20);
                                  arena<uint32_t>(0x800 + k * 0x20)[3] = r.next();
                                  if (r.range(0, 5) == 0) arena<uint32_t>(0x200 + k * 0x20)[2] = 0;
                              }
                          },
                          [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *))fn)(arena<void>(0x1000))}; }};
    quick(grid_slot_6fde90);
    // PoolComplex::KindAADE10: kinds 3-11
    Test pool_complex_kind_aade10{0xaade10,
                                  [=](Rng &r) { *(int32_t *)arena<uint8_t>(0x220) = r.range(3, 11); },
                                  [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, uint32_t))fn)(arena<void>(0x100), 9)}; }};
    quick(pool_complex_kind_aade10);
    // Objectives::Find: places in and out of range, empty or not
    Test objectives_find{0x6e9a40,
                         [=](Rng &r) {
                             for (int i = 0; i < 24; i++) arena<uint32_t>(0x128)[i] = r.range(0, 3) ? r.next() : 0;
                             arena<int32_t>(0x40)[0] = r.range(-1, 3), arena<int32_t>(0x40)[1] = r.range(-1, 1), arena<int32_t>(0x40)[2] = r.range(-1, 8);
                         },
                         [](void *fn) {
                             const int32_t *a = arena<int32_t>(0x40);
                             return Result{((uint32_t(__thiscall *)(void *, int32_t, int32_t, int32_t))fn)(arena<void>(0x100), a[0], a[1], a[2])};
                         }};
    quick(objectives_find);
    // Window::Slot6E9960: objects whose place holds them or another, in and out of range
    Test window_slot_6e9960{0x6e9960,
                            [=](Rng &r) {
                                for (int i = 0; i < 24; i++) arena<uint32_t>(0x128)[i] = r.next();
                                uint8_t *o = arena<uint8_t>(0x400);
                                const int32_t g = r.range(-1, 3), m = r.range(0, 4) ? 0 : r.range(-1, 1), s = r.range(-1, 8);
                                *(int32_t *)(o + 0x44) = g, *(int32_t *)(o + 0x48) = m, *(int32_t *)(o + 0x50) = s;
                                if (g >= 0 && g < 3 && m == 0 && s >= 0 && s < 8 && r.range(0, 3)) arena<uint32_t>(0x128)[s + g * 8] = (uint32_t)(uintptr_t)o;
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(void *, const void *))fn)(arena<void>(0x100), arena<void>(0x400));
                                return Result{0};
                            }};
    quick(window_slot_6e9960);
    // Text::FindCharBCF3E0: texts of 0-12 bytes from a few (with their 0), from inside, at and past the end
    Test text_find_char_bcf3e0{0xbcf3e0,
                               [=](Rng &r) {
                                   uint32_t *t = arena<uint32_t>(0x140);
                                   arena<uint32_t>(0x100)[0] = (uint32_t)(uintptr_t)t;
                                   const int n = r.range(0, 12);
                                   t[1] = n, t[3] = (uint32_t)(uintptr_t)arena<void>(0x200);
                                   for (int i = 0; i < n; i++) arena<uint8_t>(0x200)[i] = (uint8_t)(0x41 + r.range(0, 3));
                                   arena<uint8_t>(0x200)[n] = 0;
                                   arena<uint32_t>(0x40)[0] = (r.next() & ~0xffu) | (uint32_t)(r.range(0, 6) ? 0x41 + r.range(0, 4) : 0);
                                   arena<uint32_t>(0x40)[1] = r.range(0, n + 2);
                               },
                               [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(const void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0], arena<uint32_t>(0x40)[1])}; }};
    quick(text_find_char_bcf3e0);
    // Animals::PassB6C900: 0-6 entries in use or not over animals with the flag and byte set or not
    Test animals_pass_b6c900{0xb6c900,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100);
                                 *(int32_t *)(self + 0x60) = r.range(0, 9) ? r.range(0, 6) : -1;
                                 *(uint32_t *)(self + 0x54) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                 for (int k = 0; k < 6; k++) {
                                     uint32_t *e = arena<uint32_t>(0x200 + k * 12);
                                     e[0] = r.range(0, 2) ? r.next() | 1 : 0;
                                     e[1] = (uint32_t)(uintptr_t)arena<void>(0x400 + k * 0xc0);
                                     arena<uint8_t>(0x400 + k * 0xc0)[0xa8] = (uint8_t)r.next();
                                     arena<uint8_t>(0x400 + k * 0xc0)[0xac] = r.range(0, 1) ? 0 : (uint8_t)r.range(1, 255);
                                 }
                             },
                             [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *))fn)(arena<void>(0x100))}; }};
    quick(animals_pass_b6c900);
    // Engine::ListC50B20: on or off, the value at +0x48 or in a list of 0-5 items, or nowhere
    Test engine_list_c50b20{0xc50b20,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x1000);
                                self[0x6c] = r.range(0, 1) ? (uint8_t)r.range(1, 255) : 0;
                                *(uint32_t *)(self + 0x48) = r.range(0, 5);
                                *(uint32_t *)(self + 0xcc) = node_list_h2(r, 0x200, r.range(0, 5), 0x400);
                                for (int k = 0; k < 5; k++) arena<uint32_t>(0x400 + k * 0x40)[3] = r.range(0, 5);
                                arena<uint32_t>(0x40)[0] = r.range(0, 5);
                            },
                            [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, uint32_t))fn)(arena<void>(0x1000), arena<uint32_t>(0x40)[0]) & 0xff}; }};
    quick(engine_list_c50b20);
    // TrackPiece::Check5D1370: values from 0-8 (each test value at times)
    Test track_piece_check_5d1370{0x5d1370,
                                  [=](Rng &r) {
                                      for (int i = 0; i < 10; i++) arena<int32_t>(0x100)[i] = r.range(0, 9) ? 0 : r.range(0, 8);
                                  },
                                  [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *))fn)(arena<void>(0x100))}; }};
    quick(track_piece_check_5d1370);
    // Park::Shared774520: the game's list of 0-5 items (on or off, places alike or not) set and put back after
    {
        static uint32_t saved_list;
        Test x{0x774520,
               [=](Rng &r) {
                   *(uint32_t *)0x10ef8bc = node_list_h2(r, 0x200, r.range(0, 5), 0x1000);   // (the items are set again below)
                   arena<uint32_t>(0x120)[0] = r.range(0, 2), arena<uint32_t>(0x124)[0] = r.range(0, 2);
                   for (int k = 0; k < 5; k++) {
                       arena<uint32_t>(0x200 + k * 0x20)[2] = (uint32_t)(uintptr_t)arena<void>(0x1000 + k * 0x200);
                       *(uint32_t *)arena<uint8_t>(0x1000 + k * 0x200 + 0xe4) = r.range(0, 3) ? r.next() | 1 : 0;
                       *(uint32_t *)arena<uint8_t>(0x1000 + k * 0x200 + 0x104) = (uint32_t)(uintptr_t)arena<void>(0x2000 + k * 8);
                       arena<uint32_t>(0x2000 + k * 8)[0] = r.range(0, 2), arena<uint32_t>(0x2000 + k * 8)[1] = r.range(0, 2);
                   }
               },
               [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *))fn)(arena<void>(0x100)) & 0xff}; }};
        x.prepare = [](const rewrite::Entry *) { saved_list = *(uint32_t *)0x10ef8bc; };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x10ef8bc = saved_list; };
        quick(x);
    }
    // TrackPiece::Replace5D0D10: values from a few in the piece and a list of 0-5 items
    Test track_piece_replace_5d0d10{0x5d0d10,
                                    [=](Rng &r) {
                                        uint32_t *self = arena<uint32_t>(0x1000);
                                        self[0] = node_list_h2(r, 0x200, r.range(0, 5), 0x400);
                                        self[7] = r.range(0, 3), self[8] = r.range(0, 3);
                                        for (int k = 0; k < 5; k++) arena<uint32_t>(0x400 + k * 0x40)[0x34 / 4] = r.range(0, 3), arena<uint32_t>(0x400 + k * 0x40)[0x3c / 4] = r.range(0, 3);
                                        arena<uint32_t>(0x40)[0] = r.range(0, 3), arena<uint32_t>(0x40)[1] = r.next();
                                    },
                                    [](void *fn) {
                                        ((void(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x1000), arena<uint32_t>(0x40)[0], arena<uint32_t>(0x40)[1]);
                                        return Result{0};
                                    }};
    quick(track_piece_replace_5d0d10);
    // Window::Mode6A7DA0: modes 0-4, a bound value or none
    Test window_mode_6a7da0{0x6a7da0,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x1f0)[i] = r.next();
                                *(int32_t *)(self + 0xf0) = r.range(0, 4);
                                *(uint32_t *)(self + 0xf4) = r.range(0, 4) ? (uint32_t)(uintptr_t)arena<void>(0x40) : 0;
                                arena<uint32_t>(0x40)[0] = r.next();
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                                return Result{0};
                            }};
    quick(window_mode_6a7da0);
    // Scenery::Part827D00: a part or none, 0-20 pairs, sparse masks
    Test scenery_part_827d00{0x827d00,
                             [=](Rng &r) {
                                 *(uint32_t *)arena<uint8_t>(0x19c) = r.range(0, 5) ? (uint32_t)(uintptr_t)arena<void>(0x400) : 0;
                                 *(int32_t *)arena<uint8_t>(0x434) = r.range(-1, 20);
                                 *(uint32_t *)arena<uint8_t>(0x234) = r.range(0, 2) ? 1u << r.range(0, 31) : 0;
                             },
                             [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *))fn)(arena<void>(0x100)) & 0xff}; }};
    quick(scenery_part_827d00);
    // Text::CompareC46A80: texts alike or not (in the first word, the length or a byte), missing ones
    Test text_compare_c46a80{0xc46a80,
                             [=](Rng &r) {
                                 uint16_t *a = arena<uint16_t>(0x200), *b = arena<uint16_t>(0x300);
                                 a[0] = (uint16_t)r.range(0, 2), a[1] = (uint16_t)r.range(0, 12);
                                 for (int i = 0; i < 12; i++) arena<uint8_t>(0x204)[i] = (uint8_t)r.range(0, 2);
                                 for (int i = 0; i < 16; i++) arena<uint8_t>(0x300)[i] = arena<uint8_t>(0x200)[i];
                                 const int k = r.range(0, 4);
                                 if (k == 0) b[0] ^= 1;
                                 if (k == 1) b[1] = (uint16_t)r.range(0, 12);
                                 if (k == 2) arena<uint8_t>(0x304)[r.range(0, 11)] ^= 1;
                                 arena<uint32_t>(0x40)[0] = r.range(0, 6) ? (uint32_t)(uintptr_t)a : 0;
                                 arena<uint32_t>(0x40)[1] = r.range(0, 6) ? (uint32_t)(uintptr_t)b : 0;
                             },
                             [](void *fn) {
                                 return Result{((uint32_t(__thiscall *)(const void *, const void *))fn)((const void *)(uintptr_t)arena<uint32_t>(0x40)[0], (const void *)(uintptr_t)arena<uint32_t>(0x40)[1])};
                             }};
    quick(text_compare_c46a80);
    // PointerArray::Remove: 0-6 values from a few, v there or not
    Test pointer_array_remove{0xcdb9e0,
                              [=](Rng &r) {
                                  uint32_t *self = arena<uint32_t>(0x100);
                                  self[0] = (uint32_t)(uintptr_t)arena<void>(0x200);
                                  self[2] = r.range(0, 9) ? r.range(0, 6) : (uint32_t)-1;
                                  for (int i = 0; i < 8; i++) arena<uint32_t>(0x200)[i] = r.range(0, 5);
                                  arena<uint32_t>(0x40)[0] = r.range(0, 6);
                              },
                              [](void *fn) {
                                  ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0]);
                                  return Result{0};
                              }};
    quick(pointer_array_remove);
    // Table::Entry45EF10: rows of 0-4, columns -1..7, the second list (from 0x10000) of 0-4
    Test table_entry_45ef10{0x45ef10,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                *(int32_t *)(self + 8) = r.range(0, 4), *(int32_t *)(self + 0x10) = r.range(-1, 4);
                                *(uint32_t *)(self + 4) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                *(uint32_t *)(self + 0xc) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                for (int i = 0; i < 40; i++) arena<uint32_t>(0x200)[i] = r.next();
                                for (int i = 0; i < 5; i++) arena<uint32_t>(0x400)[i] = r.next();
                                arena<int32_t>(0x40)[0] = r.range(0, 1) ? r.range(-1, 5) : 0x10000 + r.range(-1, 5);
                                arena<int32_t>(0x40)[1] = r.range(-1, 7);
                            },
                            [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, int32_t, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<int32_t>(0x40)[1])}; }};
    quick(table_entry_45ef10);
    // Toolbar::Find79C050: the game's toolbar (put back after): ranges of 0-8 items (some empty), flags from a few
    {
        static uint32_t saved[8];
        Test x{0x79c050,
               [=](Rng &r) {
                   uint32_t *g = (uint32_t *)0x1306bc8;
                   g[0] = (uint32_t)(uintptr_t)arena<void>(0x200);
                   g[1] = r.range(0, 1), g[4] = r.range(0, 1);
                   g[6] = r.range(0, 4), g[7] = g[6] + r.range(0, 4);
                   for (int k = 0; k < 8; k++) {
                       arena<uint32_t>(0x200)[k] = r.range(0, 4) ? (uint32_t)(uintptr_t)arena<void>(0x400 + k * 0xc0) : 0;
                       *(uint32_t *)arena<uint8_t>(0x400 + k * 0xc0 + 0xb4) = 1u << r.range(0, 3);
                   }
                   arena<uint32_t>(0x40)[0] = r.range(0, 15);
               },
               [](void *fn) { return Result{((uint32_t(__fastcall *)(uint32_t))fn)(arena<uint32_t>(0x40)[0])}; }};
        x.prepare = [](const rewrite::Entry *) { memcpy(saved, (void *)0x1306bc8, 32); };
        x.finish = [](const rewrite::Entry *) { memcpy((void *)0x1306bc8, saved, 32); };
        quick(x);
    }
    // Bird::ClearA1CDA0: the node alone or in a ring of 2-4, or already out (next 0)
    Test bird_clear_a1cda0{0xa1cda0,
                           [=](Rng &r) {
                               const int n = r.range(0, 4);
                               *(uint32_t *)arena<uint8_t>(0x188) = (uint32_t)(uintptr_t)arena<void>(0x200);
                               for (int k = 0; k < 4; k++) arena<uint32_t>(0x200 + k * 0x10)[2] = r.next();
                               if (n == 0) {
                                   arena<uint32_t>(0x200)[0] = 0, arena<uint32_t>(0x200)[1] = r.next();
                                   return;
                               }
                               for (int k = 0; k < n; k++) {
                                   arena<uint32_t>(0x200 + k * 0x10)[0] = (uint32_t)(uintptr_t)arena<void>(0x200 + (k + 1) % n * 0x10);
                                   arena<uint32_t>(0x200 + k * 0x10)[1] = (uint32_t)(uintptr_t)arena<void>(0x200 + (k + n - 1) % n * 0x10);
                               }
                           },
                           [](void *fn) {
                               ((void(__thiscall *)(const void *))fn)(arena<void>(0x100));
                               return Result{0};
                           }};
    quick(bird_clear_a1cda0);
    // Scenery::Min8439E0: random boxes either side
    Test scenery_min_8439e0{0x8439e0,
                            [=](Rng &r) { for (int i = 0; i < 6; i++) arena<int32_t>(0x100)[i] = r.range(-10, 10), arena<int32_t>(0x40)[i] = r.range(-10, 10); },
                            [](void *fn) {
                                ((void(__thiscall *)(void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40));
                                return Result{0};
                            }};
    quick(scenery_min_8439e0);
    // WideString::FindChar: texts of 0-12 wide characters from a few (with their 0), from inside, at and past the end
    Test wide_string_find_char{0xbced20,
                               [=](Rng &r) {
                                   uint32_t *t = arena<uint32_t>(0x140);
                                   arena<uint32_t>(0x100)[0] = (uint32_t)(uintptr_t)t;
                                   const int n = r.range(0, 12);
                                   t[1] = n, t[3] = (uint32_t)(uintptr_t)arena<void>(0x200);
                                   for (int i = 0; i < n; i++) arena<uint16_t>(0x200)[i] = (uint16_t)(r.range(0, 5) ? 0x41 + r.range(0, 3) : 0x141);
                                   arena<uint16_t>(0x200)[n] = 0;
                                   arena<uint32_t>(0x40)[0] = (r.next() & ~0xffffu) | (uint32_t)(r.range(0, 6) ? 0x41 + r.range(0, 3) : 0x141);
                                   arena<uint32_t>(0x40)[1] = r.range(0, n + 2);
                               },
                               [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(const void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0], arena<uint32_t>(0x40)[1])}; }};
    quick(wide_string_find_char);
    // Loop::PartCDEF80: no table, or tables of 0-10 places (some empty), places below 0 and inside
    Test loop_part_cdef80{0xcdef80,
                          [=](Rng &r) {
                              int32_t *self = arena<int32_t>(0x100);
                              uint8_t *t = arena<uint8_t>(0x200);
                              self[0] = r.range(0, 6) ? (int32_t)(uintptr_t)t : 0, self[1] = (int32_t)r.next();
                              self[2] = r.range(0, 2) ? -1 : r.range(0, 10);
                              *(uint32_t *)t = (uint32_t)(uintptr_t)arena<void>(0x300);
                              *(int32_t *)(t + 0x18) = r.range(0, 3), *(int32_t *)(t + 0x1c) = r.range(0, 10);
                              for (int i = 0; i < 12; i++) arena<uint32_t>(0x300)[i] = r.range(0, 2) ? 0 : r.next();
                          },
                          [](void *fn) {
                              ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                              return Result{0};
                          }};
    quick(loop_part_cdef80);
    // Graph::FindAC5A00: 0-6 entries from a few values, o matching one or none
    Test graph_find_ac5a00{0xac5a00,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               *(int32_t *)(self + 0x44) = r.range(0, 9) ? r.range(0, 6) : -1;
                               *(uint32_t *)(self + 0x40) = (uint32_t)(uintptr_t)arena<void>(0x200);
                               for (int i = 0; i < 6 * 9; i++) arena<uint32_t>(0x200)[i] = r.next();
                               for (int k = 0; k < 6; k++) {
                                   uint8_t *e = arena<uint8_t>(0x200 + k * 0x24 + 8);
                                   *(uint32_t *)e = r.range(0, 1), e[0x18] = (uint8_t)r.range(0, 1), e[0x19] = (uint8_t)r.range(0, 1);
                               }
                               uint8_t *o = arena<uint8_t>(0x400);
                               *(uint32_t *)(o + 8) = r.range(0, 1), o[0x20] = (uint8_t)r.range(0, 1), o[0x21] = (uint8_t)r.range(0, 1);
                           },
                           [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x400)) & 0xff}; }};
    quick(graph_find_ac5a00);
    // WideString::FindBCED70: texts of 0-12 wide characters (high ones at times), from inside, at and past the end
    Test wide_string_find_bced70{0xbced70,
                                 [=](Rng &r) {
                                     uint32_t *t = arena<uint32_t>(0x140);
                                     arena<uint32_t>(0x100)[0] = (uint32_t)(uintptr_t)t;
                                     const int n = r.range(0, 12);
                                     t[1] = n, t[3] = (uint32_t)(uintptr_t)arena<void>(0x200);
                                     for (int i = 0; i < n; i++) arena<uint16_t>(0x200)[i] = (uint16_t)(r.range(0, 5) ? 0x41 + r.range(0, 3) : 0x141);
                                     arena<uint16_t>(0x200)[n] = 0;
                                     arena<uint32_t>(0x40)[0] = (r.next() & ~0xffu) | (uint32_t)(0x41 + r.range(0, 3));
                                     arena<uint32_t>(0x40)[1] = r.range(0, n + 2);
                                 },
                                 [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(const void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0], arena<uint32_t>(0x40)[1])}; }};
    quick(wide_string_find_bced70);
    // Labels::Shared475C10: the game's label list (0-4 labels, turns alike or not) and turn set; the game's data compared
    // (all put back after)
    {
        static uint32_t saved_value, saved_list, saved_turn;
        Test x{0x475c10,
               [=](Rng &r) {
                   *(uint32_t *)0x10eca14 = node_list_h2(r, 0x200, r.range(0, 4), 0x400);
                   *(uint32_t *)0x12f0144 = r.range(0, 1);
                   for (int k = 0; k < 4; k++) {
                       uint8_t *l = arena<uint8_t>(0x400 + k * 0x40);
                       for (int i = 0; i < 16; i++) arena<uint32_t>(0x400 + k * 0x40)[i] = r.next();
                       *(uint32_t *)(l + 0x64) = r.range(0, 1);
                       *(float *)(l + 0x1c) = r.real(-50, 50);
                       arena<uint32_t>(0x200 + k * 0x20)[2] = (uint32_t)(uintptr_t)arena<void>(0x400 + k * 0x40);
                   }
                   arena<uint32_t>(0x40)[0] = r.next();
               },
               [](void *fn) {
                   ((void(__thiscall *)(const void *, uint32_t))fn)(nullptr, arena<uint32_t>(0x40)[0]);
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { saved_value = *(uint32_t *)0x10eca08, saved_list = *(uint32_t *)0x10eca14, saved_turn = *(uint32_t *)0x12f0144; };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x10eca08 = saved_value, *(uint32_t *)0x10eca14 = saved_list, *(uint32_t *)0x12f0144 = saved_turn; };
        t.push_back(x);
    }
    // Window::Flag683770: flagged or not, texts of 0-8 wide characters, c with its high bit at times
    Test window_flag_683770{0x683770,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                self[0x97] = (uint8_t)r.next();
                                *(uint32_t *)(self + 0x130) = (uint32_t)(uintptr_t)arena<void>(0x300);
                                *(uint32_t *)arena<uint8_t>(0x304) = r.range(0, 8);
                                *(uint32_t *)arena<uint8_t>(0x30c) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                for (int i = 0; i < 8; i++) arena<uint16_t>(0x400)[i] = (uint16_t)(r.range(0, 4) ? 0x41 + r.range(0, 3) : 0xffc1);
                                arena<uint32_t>(0x40)[0] = (r.next() & ~0xffu) | (uint32_t)(r.range(0, 4) ? 0x41 + r.range(0, 3) : 0xc1);
                            },
                            [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0]) & 0xff}; }};
    quick(window_flag_683770);
    // Zip::CheckEndCB0B60: good end records and ones off in each field
    Test zip_check_end_cb0b60{0xcb0b60,
                              [=](Rng &r) {
                                  uint8_t *z = arena<uint8_t>(0x100);
                                  *(uint32_t *)z = 0x6054b50;
                                  *(uint16_t *)(z + 4) = 0, *(uint16_t *)(z + 6) = 0;
                                  *(uint16_t *)(z + 8) = *(uint16_t *)(z + 0xa) = (uint16_t)r.range(0, 50);
                                  *(uint16_t *)(z + 0x14) = 0;
                                  *(uint32_t *)(z + 0x18) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                  const uint32_t end = r.range(0, 3) ? r.range(100, 100000) : 0xffffffff;
                                  *(uint32_t *)arena<uint8_t>(0x244) = end;
                                  *(uint32_t *)(z + 0x10) = r.range(0, 90);
                                  *(uint32_t *)(z + 0xc) = (end == 0xffffffff ? 0 : end) - *(uint32_t *)(z + 0x10) - 0x16;
                                  switch (r.range(0, 9)) {
                                  case 0: *(uint32_t *)z ^= 1; break;
                                  case 1: *(uint16_t *)(z + 4) = 1; break;
                                  case 2: *(uint16_t *)(z + 6) = 1; break;
                                  case 3: *(uint16_t *)(z + 0xa) += 1; break;
                                  case 4: *(uint16_t *)(z + 0x14) = 1; break;
                                  case 5: *(uint32_t *)(z + 0xc) += 1; break;
                                  default: break;
                                  }
                              },
                              [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *))fn)(arena<void>(0x100))}; }};
    quick(zip_check_end_cb0b60);
    // Person::Shared7E9130: the game's list of 0-5 people of kinds 0-4 set (put back after)
    {
        static uint32_t saved_list;
        Test x{0x7e9130,
               [=](Rng &r) {
                   *(uint32_t *)0x10f0620 = node_list_h2(r, 0x200, r.range(0, 5), 0x400);
                   for (int k = 0; k < 5; k++) {
                       arena<uint32_t>(0x200 + k * 0x20)[2] = (uint32_t)(uintptr_t)arena<void>(0x1000 + k * 0x500);
                       *(int32_t *)arena<uint8_t>(0x1000 + k * 0x500 + 0x414) = r.range(0, 4);
                   }
               },
               [](void *fn) {
                   Result x;
                   x.f = ((long double(__stdcall *)())fn)();
                   return x;
               }};
        x.prepare = [](const rewrite::Entry *) { saved_list = *(uint32_t *)0x10f0620; };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x10f0620 = saved_list; };
        quick(x);
    }
    // Water::Remove95A570: chains of 0-5 edges among points 0-3 (each edge's next links in order)
    Test water_remove_95a570{0x95a570,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100);
                                 *(uint32_t *)(self + 0x20) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                 *(uint32_t *)(self + 8) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                 const uint16_t a = (uint16_t)r.range(0, 3);
                                 const int n = r.range(0, 5);
                                 for (int p = 0; p < 4; p++) arena<uint16_t>(0x200)[p] = 0xffff;
                                 if (n) arena<uint16_t>(0x200)[a] = 0;
                                 for (int k = 0; k < n; k++) {
                                     uint16_t *e = arena<uint16_t>(0x400 + k * 16);
                                     for (int i = 0; i < 8; i++) e[i] = (uint16_t)r.next();
                                     const bool from = r.range(0, 1);
                                     e[0] = from ? a : (uint16_t)((a + r.range(1, 3)) % 4);   // (not a: those follow the other link)
                                     e[1] = from ? (uint16_t)r.range(0, 3) : a;
                                     e[from ? 4 : 5] = (uint16_t)(k + 1 < n ? k + 1 : 0xffff);
                                 }
                                 arena<uint32_t>(0x40)[0] = (r.next() & ~0xffffu) | a, arena<uint32_t>(0x40)[1] = (r.next() & ~0xffffu) | r.range(0, 3);
                             },
                             [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0], arena<uint32_t>(0x40)[1])}; }};
    quick(water_remove_95a570);
    // GridArea::ValueAt / PartAt / ValueAt2: areas of 1-4 a side, places inside and past every edge
    for (uintptr_t at : {(uintptr_t)0xabc6d0, (uintptr_t)0xabc730, (uintptr_t)0xabc790}) {
        Test x{at,
               [=](Rng &r) {
                   int32_t *b = arena<int32_t>(0x14c);
                   b[0] = r.range(-3, 3), b[1] = r.range(-3, 3), b[2] = b[0] + r.range(0, 3), b[3] = b[1] + r.range(0, 3);
                   *(uint32_t *)arena<uint8_t>(0x15c) = (uint32_t)(uintptr_t)arena<void>(0x400);
                   for (int c = 0; c < 16; c++) {
                       uint8_t *cell = arena<uint8_t>(0x400 + c * 0x58);
                       *(uint32_t *)(cell + 0x3c) = (uint32_t)(uintptr_t)arena<void>(0x1000 + c * 0x10);
                       *(uint32_t *)arena<uint8_t>(0x1000 + c * 0x10 + 0xc) = r.range(0, 1) ? r.next() | 1 : 0;
                       *(uint32_t *)(cell + 0x4c) = r.next(), *(uint32_t *)(cell + 0x50) = r.next();
                   }
                   arena<int32_t>(0x40)[0] = r.range(b[0] - 1, b[2] + 1), arena<int32_t>(0x40)[1] = r.range(b[1] - 1, b[3] + 1);
               },
               [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, int32_t, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<int32_t>(0x40)[1])}; }};
        quick(x);
    }
    // List::Count6C3CC0: lists of 0-3 sub-lists of 0-4 values from a few (the list in eax)
    Test list_count_6c3cc0{0x6c3cc0,
                           [=](Rng &r) {
                               uint32_t *list = arena<uint32_t>(0x100);
                               list[0] = (uint32_t)(uintptr_t)arena<void>(0x140), list[1] = r.range(0, 9) ? r.range(0, 3) : (uint32_t)-1;
                               for (int s = 0; s < 3; s++) {
                                   uint32_t *sub = arena<uint32_t>(0x200 + s * 0x10);
                                   arena<uint32_t>(0x140)[s] = (uint32_t)(uintptr_t)sub;
                                   sub[0] = (uint32_t)(uintptr_t)arena<void>(0x300 + s * 0x20), sub[1] = r.range(0, 4);
                                   for (int i = 0; i < 4; i++) arena<uint32_t>(0x300 + s * 0x20)[i] = r.range(0, 2);
                               }
                               arena<uint32_t>(0x40)[0] = r.range(0, 2);
                           },
                           [](void *fn) {
                               uint32_t stack[1] = {arena<uint32_t>(0x40)[0]};
                               uint32_t *ps = stack;
                               uint32_t a = (uint32_t)(uintptr_t)arena<void>(0x100);
                               __asm__ volatile("pushl (%%edx)\n\tcall *%%ebx" : "+a"(a), "+d"(ps) : "b"(fn) : "ecx", "memory", "cc");
                               return Result{a & 0xff};
                           }};
    quick(list_count_6c3cc0);
    // Values4::Snap: four values near each other or not, tolerances small and large
    Test values4_snap{0x8e0c30,
                      [=](Rng &r) {
                          const float base = r.real(-10, 10);
                          for (int i = 0; i < 4; i++) arena<float>(0x100)[i] = base + (r.range(0, 1) ? r.real(-0.1f, 0.1f) : r.real(-5, 5));
                          arena<float>(0x40)[0] = r.range(0, 1) ? r.real(0, 0.3f) : r.real(0, 6);
                      },
                      [](void *fn) {
                          ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x40)[0]);
                          return Result{0};
                      }};
    quick(values4_snap);
    // Animal::EntryA6BED0: lists 1-3 of 0-6 values, o in its list (once or twice) or not
    Test animal_entry_a6bed0{0xa6bed0,
                             [=](Rng &r) {
                                 *(uint32_t *)arena<uint8_t>(0x14c) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                 const uint32_t o = (uint32_t)(uintptr_t)arena<void>(0x600);
                                 *(int32_t *)arena<uint8_t>(0x614) = r.range(1, 3);
                                 for (int l = 0; l < 3; l++) {
                                     uint32_t *list = arena<uint32_t>(0x200 + l * 12);
                                     list[0] = (uint32_t)(uintptr_t)arena<void>(0x300 + l * 0x40), list[1] = r.range(0, 6), list[2] = r.next();
                                     for (int i = 0; i < 8; i++) arena<uint32_t>(0x300 + l * 0x40)[i] = r.range(0, 3) ? r.next() : o;
                                 }
                             },
                             [](void *fn) {
                                 ((void(__thiscall *)(const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x600));
                                 return Result{0};
                             }};
    quick(animal_entry_a6bed0);
    // Table::RemoveCE3100 / RemoveCE3250: chains of 0-5 edges at a (each touching a), b on one of them or none
    for (uintptr_t at : {(uintptr_t)0xce3100, (uintptr_t)0xce3250}) {
        Test x{at,
               [=](Rng &r) {
                   const bool big = at == 0xce3100;
                   const uint32_t size = big ? 16 : 12;
                   uint8_t *self = arena<uint8_t>(0x100);
                   *(uint32_t *)(self + (big ? 8 : 0xc)) = (uint32_t)(uintptr_t)arena<void>(0x200);
                   *(uint32_t *)(self + (big ? 0x14 : 0x38)) = (uint32_t)(uintptr_t)arena<void>(0x400);
                   const uint16_t a = (uint16_t)r.range(0, 3), b = (uint16_t)((a + r.range(1, 3)) % 4);
                   const int n = r.range(0, 5);
                   for (int p = 0; p < 4; p++) arena<uint16_t>(0x200)[p] = 0xffff;
                   if (n) arena<uint16_t>(0x200)[a] = 0;
                   for (int k = 0; k < n; k++) {
                       uint16_t *e = (uint16_t *)arena<uint8_t>(0x400 + k * size);
                       for (uint32_t i = 0; i < size / 2; i++) e[i] = (uint16_t)r.next();
                       const bool first = r.range(0, 1);
                       const uint16_t other = r.range(0, 5) == 0 ? b : (uint16_t)(4 + r.range(0, 3));
                       e[0] = first ? a : other, e[1] = first ? other : a;
                       e[first ? 4 : 5] = (uint16_t)(k + 1 < n ? k + 1 : 0xffff);
                   }
                   arena<uint32_t>(0x40)[0] = (r.next() & ~0xffffu) | a, arena<uint32_t>(0x40)[1] = (r.next() & ~0xffffu) | b;
               },
               [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0], arena<uint32_t>(0x40)[1])}; }};
        quick(x);
    }
    // TypeLists::Contains: lists of 0-4 type lists of counts 0-3 from a few values, t like one of them or not
    Test type_lists_contains{0xbd2780,
                             [=](Rng &r) {
                                 *(uint32_t *)arena<uint8_t>(0x114) = node_list_h2(r, 0x200, r.range(0, 4), 0x400);
                                 for (int k = 0; k < 4; k++) {
                                     uint32_t *item = arena<uint32_t>(0x400 + k * 0x40);
                                     for (int i = 0; i < 4; i++) item[i] = r.range(0, 1);
                                     item[4] = r.range(0, 3);
                                 }
                                 uint32_t *t = arena<uint32_t>(0x600);
                                 for (int i = 0; i < 4; i++) t[i] = r.range(0, 1);
                                 t[4] = r.range(0, 3);
                             },
                             [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x600)) & 0xff}; }};
    quick(type_lists_contains);
    // Car::Kind50E6D0: counts either side of 0, the flag set or not, values either side of 0
    Test car_kind_50e6d0{0x50e6d0,
                         [=](Rng &r) {
                             uint8_t *self = arena<uint8_t>(0x1000);
                             *(int32_t *)(self + 0x7b4) = r.range(-1, 2), *(uint32_t *)(self + 0x39c) = r.next();
                             *(int32_t *)(self + 0x8c0) = r.range(-1, 2), *(int32_t *)(self + 0x72c) = r.range(-1, 2), *(int32_t *)(self + 0x730) = r.range(-1, 2);
                         },
                         [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *))fn)(arena<void>(0x1000))}; }};
    quick(car_kind_50e6d0);
    // Slots::ValueFor: ids matching one slot or none
    Test slots_value_for{0x772990,
                         [=](Rng &r) {
                             uint8_t *self = arena<uint8_t>(0x100);
                             for (int k = 0; k < 3; k++) {
                                 *(uint32_t *)(self + 0xac + k * 0x14) = (uint32_t)(uintptr_t)arena<void>(0x300 + k * 8);
                                 arena<uint32_t>(0x300 + k * 8)[0] = r.range(0, 1), arena<uint32_t>(0x300 + k * 8)[1] = r.range(0, 1);
                                 *(float *)(self + 0x9c + k * 4) = r.real(-5, 5);
                             }
                             *(float *)(self + 0x78) = r.real(-5, 5);
                             *(uint32_t *)arena<uint8_t>(0x420) = r.range(0, 1), *(uint32_t *)arena<uint8_t>(0x424) = r.range(0, 1);
                         },
                         [](void *fn) {
                             Result x;
                             x.f = ((long double(__thiscall *)(const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x400));
                             return x;
                         }};
    quick(slots_value_for);
    // Effect::Next7B5070: mode 1 or not, ranges of 0-6 places (some empty), frames near the wrap
    Test effect_next_7b5070{0x7b5070,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                *(int32_t *)(self + 0x3c) = r.range(0, 3) ? 1 : r.range(0, 2);
                                *(uint32_t *)(self + 0x20) = r.range(0, 1), *(uint32_t *)(self + 0x2c) = r.range(0, 1);
                                *(uint32_t *)(self + 0x1c) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                *(int32_t *)(self + 0x34) = r.range(0, 3), *(int32_t *)(self + 0x38) = r.range(0, 6);
                                for (int k = 0; k < 6; k++) {
                                    arena<uint32_t>(0x200)[k] = r.range(0, 4) ? (uint32_t)(uintptr_t)arena<void>(0x400 + k * 0xc0) : 0;
                                    *(int32_t *)arena<uint8_t>(0x400 + k * 0xc0 + 0xb8) = r.range(0, 1) ? r.range(340, 347) : r.range(-5, 400);
                                    *(uint32_t *)arena<uint8_t>(0x400 + k * 0xc0 + 0xbc) = r.next();
                                }
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(const void *))fn)(arena<void>(0x100));
                                return Result{0};
                            }};
    quick(effect_next_7b5070);
    // List::Find45EEA0: 0-3 rows and 0-4 extras of values from a few
    Test list_find_45eea0{0x45eea0,
                          [=](Rng &r) {
                              uint8_t *self = arena<uint8_t>(0x100);
                              *(int32_t *)(self + 8) = r.range(0, 9) ? r.range(0, 3) : -1, *(int32_t *)(self + 0x10) = r.range(0, 4);
                              *(uint32_t *)(self + 4) = (uint32_t)(uintptr_t)arena<void>(0x200);
                              *(uint32_t *)(self + 0xc) = (uint32_t)(uintptr_t)arena<void>(0x300);
                              for (int i = 0; i < 24; i++) arena<uint32_t>(0x200)[i] = r.range(0, 40);
                              for (int i = 0; i < 4; i++) arena<uint32_t>(0x300)[i] = r.range(0, 40);
                              arena<uint32_t>(0x40)[0] = r.range(0, 40);
                          },
                          [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(const void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0])}; }};
    quick(list_find_45eea0);
    // Track::CheckBC6D00: rides with each condition met or not, flag set or not (the ride in eax, the other in esi)
    Test track_check_bc6d00{0xbc6d00,
                            [=](Rng &r) {
                                uint8_t *ride = arena<uint8_t>(0x100), *other = arena<uint8_t>(0x300);
                                *(uint32_t *)(ride + 0x18) = r.next() | (r.range(0, 4) ? 0x400000 : 0);
                                *(int32_t *)(ride + 0xe4) = r.range(0, 4) ? r.range(2, 5) : r.range(0, 1);
                                *(uint32_t *)(ride + 0x10c) = r.range(0, 5) ? r.next() | 1 : 0, *(uint32_t *)(ride + 0x110) = r.range(0, 5) ? r.next() | 1 : 0;
                                *(uint32_t *)(other + 0x18) = r.next();
                                arena<uint32_t>(0x40)[0] = (r.next() & ~0xffu) | (r.range(0, 1) ? 1u : 0u);
                            },
                            [](void *fn) {
                                uint32_t stack[1] = {arena<uint32_t>(0x40)[0]};
                                uint32_t *ps = stack;
                                uint32_t a = (uint32_t)(uintptr_t)arena<void>(0x100);
                                void *o = arena<void>(0x300);
                                __asm__ volatile("pushl (%%edx)\n\tcall *%%ebx" : "+a"(a), "+d"(ps), "+S"(o) : "b"(fn) : "ecx", "memory", "cc");
                                return Result{a & 0xff};
                            }};
    quick(track_check_bc6d00);
    // Items::SetValueAt: lists of 1-5 items, n within the list
    Test items_set_value_at{0xc64ce0,
                            [=](Rng &r) {
                                const int n = r.range(1, 5);
                                *(uint32_t *)arena<uint8_t>(0x1f0) = (uint32_t)(uintptr_t)arena<void>(0x600);
                                *(uint32_t *)arena<uint8_t>(0x6b0) = node_list_h2(r, 0x200, n, 0x1000);
                                for (int k = 0; k < 5; k++) {
                                    arena<uint32_t>(0x200 + k * 0x20)[2] = (uint32_t)(uintptr_t)arena<void>(0x1000 + k * 0x100);
                                    for (int i = 0; i < 0x40; i++) arena<uint32_t>(0x1000 + k * 0x100)[i] = r.next();
                                }
                                arena<uint8_t>(0x19c)[0] = (uint8_t)r.next();
                                arena<int32_t>(0x40)[0] = r.range(0, n - 1), arena<uint32_t>(0x40)[1] = r.next();
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(void *, int32_t, uint32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<uint32_t>(0x40)[1]);
                                return Result{0};
                            }};
    quick(items_set_value_at);
    // SortedPointers::Find64: 0-8 sorted pairs (repeats in the first value), pairs there and between
    Test sorted_pointers_find64{0xcdc420,
                                [=](Rng &r) {
                                    uint32_t *self = arena<uint32_t>(0x100);
                                    const int n = r.range(0, 8);
                                    self[0] = (uint32_t)(uintptr_t)arena<void>(0x200), self[1] = n;
                                    uint32_t x = 0, y = 0;
                                    for (int k = 0; k < n; k++) {
                                        if (r.range(0, 1)) x += r.range(1, 3), y = r.range(0, 2);
                                        else y += r.range(1, 3);
                                        arena<uint32_t>(0x200)[k] = (uint32_t)(uintptr_t)arena<void>(0x300 + k * 8);
                                        arena<uint32_t>(0x300 + k * 8)[0] = x, arena<uint32_t>(0x300 + k * 8)[1] = y;
                                    }
                                    arena<uint32_t>(0x40)[0] = r.range(0, 20), arena<uint32_t>(0x40)[1] = r.range(0, 10), arena<uint32_t>(0x40)[2] = r.next();
                                },
                                [](void *fn) {
                                    return Result{((uint32_t(__thiscall *)(const void *, uint32_t, uint32_t, void *))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0], arena<uint32_t>(0x40)[1], arena<void>(0x48)) & 0xff};
                                }};
    quick(sorted_pointers_find64);
    // Entries::EarlierSame: a part or none, n 0-5 among entries of ids from a few, masks sparse
    Test entries_earlier_same{0x827e00,
                              [=](Rng &r) {
                                  *(uint32_t *)arena<uint8_t>(0x19c) = r.range(0, 5) ? (uint32_t)(uintptr_t)arena<void>(0x300) : 0;
                                  *(uint32_t *)arena<uint8_t>(0x338) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                  for (int k = 0; k < 6; k++) *(uint32_t *)arena<uint8_t>(0x400 + k * 0x58 + 0x54) = r.range(0, 2);
                                  *(uint32_t *)arena<uint8_t>(0x234) = r.next() & r.next();
                                  arena<int32_t>(0x40)[0] = r.range(0, 5);
                              },
                              [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0]) & 0xff}; }};
    quick(entries_earlier_same);
    // NamedList::Find2: 0-4 entries with names from a few, the name there or not
    Test named_list_find2{0xbd5110,
                          [=](Rng &r) {
                              static const char *const names[] = {"", "a", "ab", "abc", "b"};
                              char *pool = arena<char>(0x800);
                              for (int i = 0; i < 5; i++) {
                                  const char *s = names[i];
                                  int k = 0;
                                  for (; s[k]; k++) pool[i * 8 + k] = s[k];
                                  pool[i * 8 + k] = 0;
                              }
                              const int n = r.range(0, 4);
                              *(uint32_t *)arena<uint8_t>(0x304) = n ? (uint32_t)(uintptr_t)arena<void>(0x400) : 0;
                              for (int k = 0; k < n; k++) {
                                  uint8_t *e = arena<uint8_t>(0x400 + k * 0x40);
                                  *(uint32_t *)(e + 0x24) = (uint32_t)(uintptr_t)(pool + r.range(0, 4) * 8);
                                  *(uint32_t *)(e + 0x30) = k + 1 < n ? (uint32_t)(uintptr_t)arena<void>(0x400 + (k + 1) * 0x40) : 0;
                              }
                              arena<uint32_t>(0x40)[0] = (uint32_t)(uintptr_t)(pool + r.range(0, 4) * 8);
                          },
                          [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, const char *))fn)(arena<void>(0x100), (const char *)(uintptr_t)arena<uint32_t>(0x40)[0])}; }};
    quick(named_list_find2);
    // SortedTable::LowerBoundKind / FindKind: 0-8 entries sorted by kind then number, kinds 0, 1 and 2 asked
    for (uintptr_t at : {(uintptr_t)0x88f9a0, (uintptr_t)0x88fa10}) {
        Test x{at,
               [=](Rng &r) {
                   const bool big = at == 0x88f9a0;
                   uint8_t *self = arena<uint8_t>(0x100);
                   const int n = r.range(0, 8);
                   *(int32_t *)(self + (big ? 0x18 : 0x54)) = n;
                   *(uint32_t *)(self + (big ? 0x14 : 0x50)) = (uint32_t)(uintptr_t)arena<void>(0x400);
                   const int ones = r.range(0, n);
                   uint32_t v = 0;
                   for (int k = 0; k < n; k++) {
                       if (k == n - ones) v = 0;
                       v += r.range(0, 2);
                       uint8_t *e = arena<uint8_t>(0x400 + k * (big ? 0x150 : 12));
                       if (big) e[0] = (uint8_t)((r.next() & 0xfe) | (k >= n - ones)), *(uint32_t *)(e + 0xc) = v;
                       else e[4] = (uint8_t)(k >= n - ones), *(uint32_t *)e = v;
                   }
                   arena<uint32_t>(0x40)[0] = r.range(0, 10), arena<uint32_t>(0x40)[1] = (r.next() & ~0xffu) | r.range(0, 2), arena<uint32_t>(0x40)[2] = r.next();
               },
               [=](void *fn) {
                   const uint32_t *a = arena<uint32_t>(0x40);
                   if (at == 0x88f9a0) return Result{((uint32_t(__thiscall *)(const void *, uint32_t, uint32_t))fn)(arena<void>(0x100), a[0], a[1])};
                   return Result{((uint32_t(__thiscall *)(const void *, uint32_t, uint32_t, void *))fn)(arena<void>(0x100), a[0], a[1], arena<void>(0x48)) & 0xff};
               }};
        quick(x);
    }
    // Entries::Differs: 0-5 entries from a few values, c 0 or not
    Test entries_differs{0x557e70,
                         [=](Rng &r) {
                             uint8_t *self = arena<uint8_t>(0x100);
                             *(int32_t *)(self + 0x98) = r.range(0, 9) ? r.range(0, 5) : -1;
                             *(uint32_t *)(self + 0x94) = (uint32_t)(uintptr_t)arena<void>(0x200);
                             for (int i = 0; i < 20; i++) arena<uint32_t>(0x200)[i] = r.range(0, 1);
                             for (int i = 0; i < 4; i++) arena<uint32_t>(0x40)[i] = r.range(0, 1);
                             if (r.range(0, 1)) arena<uint32_t>(0x40)[2] = r.next();
                         },
                         [](void *fn) {
                             const uint32_t *a = arena<uint32_t>(0x40);
                             return Result{((uint32_t(__thiscall *)(const void *, uint32_t, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), a[0], a[1], a[2], a[3])};
                         }};
    quick(entries_differs);
    // List::LoopA1CE30: lists of 0-4 items whose ring nodes are alone, in a ring of 2-3 or broken
    Test list_loop_a1ce30{0xa1ce30,
                          [=](Rng &r) {
                              arena<uint32_t>(0x40)[0] = node_list_h2(r, 0x200, r.range(0, 4), 0x400);
                              for (int k = 0; k < 4; k++) {
                                  uint32_t *n = arena<uint32_t>(0x400 + k * 0x40 + 8);
                                  const int shape = r.range(0, 3);
                                  uint32_t *o = arena<uint32_t>(0x800 + k * 0x20), *q = arena<uint32_t>(0x810 + k * 0x20);
                                  if (shape == 0) n[0] = n[1] = (uint32_t)(uintptr_t)n;
                                  if (shape == 1) n[0] = n[1] = (uint32_t)(uintptr_t)o, o[0] = o[1] = (uint32_t)(uintptr_t)n;
                                  if (shape == 2) n[0] = (uint32_t)(uintptr_t)o, n[1] = (uint32_t)(uintptr_t)q, o[1] = (uint32_t)(uintptr_t)n, q[0] = (uint32_t)(uintptr_t)n, o[0] = (uint32_t)(uintptr_t)q, q[1] = (uint32_t)(uintptr_t)o;
                                  if (shape == 3) n[0] = r.range(0, 1) ? 0 : (uint32_t)(uintptr_t)o, n[1] = (uint32_t)(uintptr_t)q, o[1] = r.next(), q[0] = r.next();
                              }
                          },
                          [](void *fn) {
                              ((void(__stdcall *)(const void *))fn)(arena<void>(0x40));
                              return Result{0};
                          }};
    quick(list_loop_a1ce30);
    // Effects::Set47C4E0: levels 0-3, lists of 0-3 effects of kinds 0-3, kinds muted or not (the game's tables compared,
    // all put back after)
    {
        static uint32_t saved_levels[4], saved_lists[24], saved_muted[4];
        Test x{0x47c4e0,
               [=](Rng &r) {
                   for (int k = 0; k < 4; k++) {
                       ((float *)0x10edd40)[k] = r.real(0, 2), ((uint32_t *)0x12f0168)[k] = r.range(0, 2) ? 0 : 1;
                       *(uint32_t *)(0x10edd60 + k * 24) = node_list_h2(r, 0x200 + k * 0x200, r.range(0, 3), 0x1000 + k * 0x200);
                       for (int j = 0; j < 3; j++) {
                           uint8_t *e = arena<uint8_t>(0x1000 + k * 0x200 + j * 0x40);
                           *(int32_t *)(e + 0x18) = r.range(0, 3), *(float *)(e + 0x14) = r.real(-2, 2);
                           *(uint32_t *)(e + 0xc) = (uint32_t)(uintptr_t)arena<void>(0x2000 + (k * 3 + j) * 0x60);
                       }
                   }
                   arena<uint32_t>(0x40)[0] = r.range(0, 3), arena<float>(0x40)[1] = r.real(0, 3);
               },
               [](void *fn) {
                   ((void(__thiscall *)(uintptr_t, float))fn)(arena<uint32_t>(0x40)[0], arena<float>(0x40)[1]);
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { memcpy(saved_levels, (void *)0x10edd40, 16), memcpy(saved_lists, (void *)0x10edd60, 96), memcpy(saved_muted, (void *)0x12f0168, 16); };
        x.finish = [](const rewrite::Entry *) { memcpy((void *)0x10edd40, saved_levels, 16), memcpy((void *)0x10edd60, saved_lists, 96), memcpy((void *)0x12f0168, saved_muted, 16); };
        t.push_back(x);
    }
    // List678::Remove: 0-6 values from a few, v there (once or twice) or not
    Test list678_remove{0x4277e0,
                        [=](Rng &r) {
                            uint8_t *self = arena<uint8_t>(0x100);
                            *(uint32_t *)(self + 0x678) = (uint32_t)(uintptr_t)arena<void>(0x800);
                            *(int32_t *)(self + 0x680) = r.range(0, 9) ? r.range(0, 6) : -1;
                            for (int i = 0; i < 8; i++) arena<uint32_t>(0x800)[i] = r.range(0, 4);
                            arena<uint32_t>(0x40)[0] = r.range(0, 5);
                        },
                        [](void *fn) {
                            ((void(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0]);
                            return Result{0};
                        }};
    quick(list678_remove);
    // Bits::SetRange: block sizes 1-8, ranges of either sign over 0-6 words
    Test bits_set_range{0xc166e0,
                        [=](Rng &r) {
                            uint8_t *self = arena<uint8_t>(0x100);
                            *(int32_t *)(self + 4) = r.range(1, 8), *(int32_t *)(self + 0x10) = r.range(0, 6), *(uint32_t *)(self + 0x14) = r.next();
                            *(uint32_t *)(self + 0xc) = (uint32_t)(uintptr_t)arena<void>(0x200);
                            for (int i = 0; i < 8; i++) arena<uint32_t>(0x200)[i] = r.next() & r.next();
                            const int a = r.range(-40, 300);
                            arena<int32_t>(0x40)[0] = a, arena<int32_t>(0x40)[1] = a + r.range(-5, 60);
                        },
                        [](void *fn) {
                            ((void(__thiscall *)(void *, int32_t, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<int32_t>(0x40)[1]);
                            return Result{0};
                        }};
    quick(bits_set_range);
    // Object::Scale2 / Scale: flags, end counts and values either side of 0
    for (uintptr_t at : {(uintptr_t)0xbc3eb0, (uintptr_t)0xbc3f20}) {
        Test x{at,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   *(uint32_t *)(self + 0x18) = r.next();
                   *(int32_t *)(self + 0xe4) = r.range(0, 3);
                   *(float *)(self + 0xfc) = r.range(0, 3) ? r.real(-3, 3) : 0.0f, *(float *)(self + 0x100) = r.range(0, 3) ? r.real(-3, 3) : 0.0f;
               },
               [](void *fn) {
                   Result x;
                   x.f = ((long double(__thiscall *)(const void *))fn)(arena<void>(0x100));
                   return x;
               }};
        quick(x);
    }
    // SortedPointers::FindFlagged: 0-8 sorted items (both flags for a number at times), numbers there and between
    Test sorted_pointers_find_flagged{0xce7260,
                                      [=](Rng &r) {
                                          uint8_t *self = arena<uint8_t>(0x100);
                                          const int n = r.range(0, 8);
                                          *(int32_t *)(self + 0x1c) = n, *(uint32_t *)(self + 0x18) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                          uint32_t v = 0;
                                          bool f = true;
                                          for (int k = 0; k < n; k++) {
                                              if (f || r.range(0, 1)) v += r.range(1, 2), f = r.range(0, 1);
                                              else f = true;
                                              arena<uint32_t>(0x200)[k] = (uint32_t)(uintptr_t)arena<void>(0x300 + k * 8);
                                              arena<uint32_t>(0x300 + k * 8)[0] = v, arena<uint32_t>(0x300 + k * 8)[1] = f ? (r.next() | 1) & 0xff : 0;
                                          }
                                          arena<uint32_t>(0x40)[0] = r.range(0, 18), arena<uint32_t>(0x40)[1] = (r.next() & ~0xffu) | (r.range(0, 1) ? r.range(1, 255) : 0), arena<uint32_t>(0x40)[2] = r.next();
                                      },
                                      [](void *fn) {
                                          return Result{((uint32_t(__thiscall *)(const void *, uint32_t, uint32_t, void *))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0], arena<uint32_t>(0x40)[1], arena<void>(0x48)) & 0xff};
                                      }};
    quick(sorted_pointers_find_flagged);
    // List::SortBy34: 0-7 objects with values from a few (ties at times)
    Test list_sort_by34{0x80edc0,
                        [=](Rng &r) {
                            uint8_t *self = arena<uint8_t>(0x100);
                            const int n = r.range(0, 7);
                            *(int32_t *)(self + 0x64) = n, *(uint32_t *)(self + 0x5c) = (uint32_t)(uintptr_t)arena<void>(0x200);
                            for (int k = 0; k < 7; k++) {
                                arena<uint32_t>(0x200)[k] = (uint32_t)(uintptr_t)arena<void>(0x300 + k * 0x40);
                                *(float *)arena<uint8_t>(0x334 + k * 0x40) = r.range(0, 1) ? (float)r.range(0, 3) : r.real(-5, 5);
                            }
                        },
                        [](void *fn) {
                            ((void(__thiscall *)(const void *))fn)(arena<void>(0x100));
                            return Result{0};
                        }};
    quick(list_sort_by34);
    // Grid::Range: grids of 0-4 by 0-4 cells of random values
    Test grid_range{0x8386a0,
                    [=](Rng &r) {
                        uint8_t *self = arena<uint8_t>(0x100);
                        *(int32_t *)(self + 0x10) = r.range(0, 4), *(int32_t *)(self + 0x14) = r.range(0, 4);
                        *(uint32_t *)(self + 0x18) = (uint32_t)(uintptr_t)arena<void>(0x400);
                        for (int i = 0; i < 16 * 5; i++) arena<int32_t>(0x400)[i] = r.range(-100, 100);
                        arena<int32_t>(0x40)[0] = r.range(-100, 100), arena<int32_t>(0x40)[1] = r.range(-100, 100);
                    },
                    [](void *fn) {
                        ((void(__thiscall *)(const void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x40), arena<void>(0x44));
                        return Result{0};
                    }};
    quick(grid_range);
    // SortedPointers::FindXY3: 0-8 entries sorted by y then x, places there and between, out given or not
    Test sorted_pointers_find_xy3{0x8ff150,
                                  [=](Rng &r) {
                                      uint8_t *self = arena<uint8_t>(0x100);
                                      const int n = r.range(0, 8);
                                      *(int32_t *)(self + 0x40) = n, *(uint32_t *)(self + 0x3c) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                      int32_t x = -3, y = -3;
                                      for (int k = 0; k < n; k++) {
                                          if (r.range(0, 1)) y += r.range(1, 2), x = r.range(-3, 0);
                                          else x += r.range(1, 2);
                                          arena<uint32_t>(0x200)[k] = (uint32_t)(uintptr_t)arena<void>(0x300 + k * 8);
                                          arena<int32_t>(0x300 + k * 8)[0] = x, arena<int32_t>(0x300 + k * 8)[1] = y;
                                      }
                                      arena<int32_t>(0x40)[0] = r.range(-3, 8), arena<int32_t>(0x40)[1] = r.range(-3, 10);
                                      arena<uint32_t>(0x40)[2] = r.range(0, 4) ? (uint32_t)(uintptr_t)arena<void>(0x60) : 0;
                                      arena<uint32_t>(0x60)[0] = r.next();
                                  },
                                  [](void *fn) {
                                      return Result{(uint32_t)(uintptr_t)((const void *(__thiscall *)(const void *, int32_t, int32_t, void *))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<int32_t>(0x40)[1], (void *)(uintptr_t)arena<uint32_t>(0x40)[2])};
                                  }};
    quick(sorted_pointers_find_xy3);
    // Vector::DeadZone: parts within and past the limits (and infinities)
    Test vector_dead_zone{0xa17470,
                          [=](Rng &r) {
                              for (int c = 0; c < 3; c++) {
                                  const int k = r.range(0, 5);
                                  arena<float>(0x100)[c] = k == 0 ? 2e6f : k == 1 ? -2e6f : k == 2 ? 999999.875f : k == 3 ? __builtin_inff() : r.real(-1000, 1000);
                              }
                          },
                          [](void *fn) {
                              ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                              return Result{0};
                          }};
    quick(vector_dead_zone);
    // VertexDeclaration::FindBDD180: two slots (or none) and texts that fit or not (length or bytes off)
    Test vertex_declaration_find_bdd180{0xbdd180,
                                        [=](Rng &r) {
                                            for (int k = 0; k < 2; k++) {
                                                uint8_t *slot = arena<uint8_t>(0x200 + k * 0x20), *text = arena<uint8_t>(0x400 + k * 0x40);
                                                *(uint32_t *)arena<uint8_t>(0x118 + k * 4) = r.range(0, 5) ? (uint32_t)(uintptr_t)slot : 0;
                                                const uint16_t a = (uint16_t)r.range(0, 8);
                                                *(uint16_t *)slot = a, *(uint16_t *)(slot + 2) = (uint16_t)r.range(0, a);
                                                *(uint32_t *)(slot + 4) = (uint32_t)(uintptr_t)arena<void>(0x600 + k * 0x20);
                                                for (int i = 0; i < 16; i++) arena<uint8_t>(0x600 + k * 0x20)[i] = text[i] = (uint8_t)r.range(0, 1);
                                                if (r.range(0, 4) == 0) text[r.range(0, 7)] ^= 1;
                                                *(uint32_t *)(text + 0x20) = r.range(0, 9);
                                                arena<uint32_t>(0x300)[k] = (uint32_t)(uintptr_t)arena<void>(0x380 + k * 4);
                                                arena<uint32_t>(0x380)[k] = (uint32_t)(uintptr_t)text;
                                            }
                                        },
                                        [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x300)) & 0xff}; }};
    quick(vertex_declaration_find_bdd180);
    // TrackPiece::Compare5BF4A0: ranges either way round, lists of 0-4 items on or off with values in and out
    Test track_piece_compare_5bf4a0{0x5bf4a0,
                                    [=](Rng &r) {
                                        *(uint32_t *)arena<uint8_t>(0x404) = node_list_h2(r, 0x200, r.range(0, 4), 0x600);
                                        for (int k = 0; k < 4; k++) {
                                            uint8_t *item = arena<uint8_t>(0x600 + k * 0x40);
                                            *(float *)(item + 4) = (float)r.range(0, 6), item[8] = r.range(0, 3) ? (uint8_t)r.range(1, 255) : 0;
                                        }
                                        arena<float>(0x40)[0] = (float)r.range(0, 6), arena<float>(0x40)[1] = r.range(0, 1) ? (float)r.range(0, 6) : r.real(0, 6);
                                    },
                                    [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, float, float))fn)(arena<void>(0x100), arena<float>(0x40)[0], arena<float>(0x40)[1]) & 0xff}; }};
    quick(track_piece_compare_5bf4a0);
    // Terrain::Inside7AE680: areas of 1-6 by 1-6 cells anywhere (wrapping past 255 at times), cells in and around them
    Test terrain_inside_7ae680{0x7ae680,
                               [=](Rng &r) {
                                   uint8_t *self = arena<uint8_t>(0x100);
                                   self[4] = (uint8_t)(r.range(0, 4) ? r.range(0, 20) : r.range(250, 255)), self[5] = (uint8_t)r.range(0, 20);
                                   self[6] = (uint8_t)r.range(1, 6), self[7] = (uint8_t)r.range(1, 6);
                                   *(uint32_t *)(self + 8) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                   for (int i = 0; i < 0x40; i++) arena<uint8_t>(0x400)[i] = r.range(0, 1) ? (uint8_t)r.next() : 0;
                                   arena<uint32_t>(0x40)[0] = (r.next() & ~0xffu) | (uint8_t)(self[4] + r.range(-1, 6));
                                   arena<uint32_t>(0x40)[1] = (r.next() & ~0xffu) | (uint8_t)(self[5] + r.range(-1, 6));
                               },
                               [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0], arena<uint32_t>(0x40)[1]) & 0xff}; }};
    quick(terrain_inside_7ae680);
    // SortedShorts::Find: sorted lists of 2-20 words (repeats at times), keys below the last, guesses anywhere in the list
    Test sorted_shorts_find{0xbb6970,
                            [=](Rng &r) {
                                const int n = r.range(2, 20);
                                uint16_t v = (uint16_t)r.range(0, 3);
                                for (int i = 0; i < n; i++) arena<uint16_t>(0x400)[i] = v, v = (uint16_t)(v + r.range(0, 3));
                                const uint16_t last = arena<uint16_t>(0x400)[n - 1];
                                *(uint16_t *)arena<uint8_t>(0x138) = last ? (uint16_t)r.range(0, last - 1) : 0;
                                if (!last) arena<uint16_t>(0x400)[n - 1] = 1;
                                *(float *)arena<uint8_t>(0x12c) = r.real(0, (n - 0.6f) / n);   // (a guess rounding to n would read past the list)
                                arena<uint32_t>(0x40)[0] = n;
                            },
                            [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, const void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x400), arena<uint32_t>(0x40)[0])}; }};
    quick(sorted_shorts_find);
    // Sound::MuteKind / UnmuteKind: kinds 0-3 (in ecx), flag or not (in dl), mute counts from -1, lists of 0-3 sounds (the
    // game's tables compared, all put back after)
    for (uintptr_t at : {(uintptr_t)0x47c3f0, (uintptr_t)0x47c470}) {
        static uint32_t saved_levels[4], saved_lists[24], saved_muted[4];
        Test x{at,
               [=](Rng &r) {
                   for (int k = 0; k < 4; k++) {
                       ((float *)0x10edd40)[k] = r.real(0, 2), ((int32_t *)0x12f0168)[k] = r.range(-1, 2);
                       *(uint32_t *)(0x10edd60 + k * 24) = node_list_h2(r, 0x200 + k * 0x200, r.range(0, 3), 0x1000 + k * 0x200);
                       for (int j = 0; j < 3; j++) {
                           uint8_t *e = arena<uint8_t>(0x1000 + k * 0x200 + j * 0x40);
                           *(int32_t *)(e + 0x18) = r.range(0, 3), *(float *)(e + 0x14) = r.real(-2, 2);
                           *(uint32_t *)(e + 0xc) = (uint32_t)(uintptr_t)arena<void>(0x2000 + (k * 3 + j) * 0x60);
                       }
                   }
                   arena<uint32_t>(0x40)[0] = r.range(0, 3), arena<uint32_t>(0x40)[1] = (r.next() & ~0xffu) | (r.range(0, 2) ? 0u : 1u);
               },
               [](void *fn) {
                   ((void(__fastcall *)(uintptr_t, uint32_t))fn)(arena<uint32_t>(0x40)[0], arena<uint32_t>(0x40)[1]);
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { memcpy(saved_levels, (void *)0x10edd40, 16), memcpy(saved_lists, (void *)0x10edd60, 96), memcpy(saved_muted, (void *)0x12f0168, 16); };
        x.finish = [](const rewrite::Entry *) { memcpy((void *)0x10edd40, saved_levels, 16), memcpy((void *)0x10edd60, saved_lists, 96), memcpy((void *)0x12f0168, saved_muted, 16); };
        t.push_back(x);
    }
    // Materials::CompareOpacity: parts over 4 materials with and without +0x58, opacity set or not
    Test materials_compare_opacity{0xceb590,
                                   [=](Rng &r) {
                                       uint8_t *self = arena<uint8_t>(0x100);
                                       *(uint32_t *)self = (uint32_t)(uintptr_t)arena<void>(0x400);
                                       *(uint32_t *)(self + 0x7c) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                       for (int p = 0; p < 4; p++) arena<uint32_t>(0x200)[p * 3] = r.range(0, 3);
                                       for (int m = 0; m < 4; m++) {
                                           uint8_t *mat = arena<uint8_t>(0x400 + m * 0x70);
                                           mat[0x58] = r.range(0, 4) ? 1 : 0, mat[0x50] = r.range(0, 1) ? 1 : 0;
                                           *(float *)(mat + 0x54) = r.range(0, 1) ? 1.0f : r.real(0, 1.5f);
                                       }
                                       arena<int32_t>(0x40)[0] = r.range(0, 3), arena<int32_t>(0x40)[1] = r.range(0, 3);
                                   },
                                   [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, int32_t, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<int32_t>(0x40)[1]) & 0xff}; }};
    quick(materials_compare_opacity);
    // Tile::QuarterCorners: the game's tile size and base set (put back after), all four bit pairs (out in ecx, bits in
    // dl)
    {
        static uint32_t saved_size, saved_base;
        Test x{0x727cc0,
               [=](Rng &r) {
                   *(float *)0x1306390 = r.real(0.5f, 8), *(float *)0x13063a4 = r.range(0, 4) ? r.real(-500, 500) : -0.0f;
                   for (int i = 0; i < 8; i++) arena<uint32_t>(0x100)[i] = r.next();
                   arena<uint32_t>(0x40)[0] = r.next();
               },
               [](void *fn) {
                   ((void(__fastcall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0]);
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { saved_size = *(uint32_t *)0x1306390, saved_base = *(uint32_t *)0x13063a4; };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x1306390 = saved_size, *(uint32_t *)0x13063a4 = saved_base; };
        quick(x);
    }
    // Shape::SameAs: shapes of 0-4 points copied from each other, then one part changed at times
    Test shape_same_as{0xae0980,
                       [=](Rng &r) {
                           uint8_t *a = arena<uint8_t>(0x100), *b = arena<uint8_t>(0x200);
                           const int n = r.range(0, 4);
                           *(int32_t *)(a + 0x50) = n, *(int32_t *)(b + 0x50) = n;
                           *(float *)(a + 0x54) = *(float *)(b + 0x54) = r.real(-5, 5);
                           *(uint32_t *)(a + 0x40) = (uint32_t)(uintptr_t)arena<void>(0x300), *(uint32_t *)(b + 0x40) = (uint32_t)(uintptr_t)arena<void>(0x380);
                           for (int i = 0; i < 8; i++) arena<float>(0x300)[i] = arena<float>(0x380)[i] = (float)r.range(-3, 3);
                           switch (r.range(0, 5)) {
                           case 0: *(int32_t *)(b + 0x50) = r.range(0, 4); break;
                           case 1: *(float *)(b + 0x54) += 1; break;
                           case 2: arena<float>(0x380)[r.range(0, 7)] += 1; break;
                           case 3: *(float *)(a + 0x54) = -0.0f, *(float *)(b + 0x54) = 0.0f; break;
                           default: break;
                           }
                       },
                       [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x200)) & 0xff}; }};
    quick(shape_same_as);
    // List::MoveAllTo: lists of 0-4 items whose ring nodes are alone, in rings or broken, moved to a ring of 1-2 nodes
    Test list_move_all_to{0xa1ce90,
                          [=](Rng &r) {
                              arena<uint32_t>(0x40)[0] = node_list_h2(r, 0x200, r.range(0, 4), 0x400);
                              uint32_t *head = arena<uint32_t>(0xa00), *other = arena<uint32_t>(0xa10);
                              *(uint32_t *)arena<uint8_t>(0x188) = (uint32_t)(uintptr_t)head;
                              if (r.range(0, 1)) head[0] = head[1] = (uint32_t)(uintptr_t)head;
                              else head[0] = head[1] = (uint32_t)(uintptr_t)other, other[0] = other[1] = (uint32_t)(uintptr_t)head;
                              for (int k = 0; k < 4; k++) {
                                  uint32_t *n = arena<uint32_t>(0x400 + k * 0x40 + 8);
                                  uint32_t *o = arena<uint32_t>(0x800 + k * 0x20), *q = arena<uint32_t>(0x810 + k * 0x20);
                                  const int shape = r.range(0, 2);
                                  if (shape == 0) n[0] = n[1] = (uint32_t)(uintptr_t)n;
                                  if (shape == 1) n[0] = n[1] = (uint32_t)(uintptr_t)o, o[0] = o[1] = (uint32_t)(uintptr_t)n;
                                  if (shape == 2) n[0] = 0, n[1] = (uint32_t)(uintptr_t)q, q[0] = r.next();
                              }
                          },
                          [](void *fn) {
                              ((void(__thiscall *)(const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40));
                              return Result{0};
                          }};
    quick(list_move_all_to);
    // Pools::ResetCounts: two lists of 0-3 pools with uses either side of 3/4 of their sizes
    Test pools_reset_counts{0xbfef90,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                for (int i = 0; i < 0x28; i++) arena<uint32_t>(0x100)[i] = r.next();
                                *(uint32_t *)(self + 0x24) = node_list_h2(r, 0x200, r.range(0, 3), 0x400);
                                *(uint32_t *)(self + 0x84) = node_list_h2(r, 0x600, r.range(0, 3), 0x800);
                                for (int k = 0; k < 3; k++) {
                                    int32_t *a = arena<int32_t>(0x400 + k * 0x40), *b = arena<int32_t>(0x800 + k * 0x40);
                                    for (int i = 0; i < 10; i++) a[i] = (int32_t)r.next(), b[i] = (int32_t)r.next();
                                    a[2] = r.range(1, 20), a[7] = r.range(0, 25);
                                }
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                                return Result{0};
                            }};
    quick(pools_reset_counts);
    // Scenery::Count8606E0: 0-4 entries of kinds 0-4 over parts of 0-3 pieces with flags from a few
    Test scenery_count_8606e0{0x8606e0,
                              [=](Rng &r) {
                                  *(uint32_t *)arena<uint8_t>(0x10c) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                  *(int32_t *)arena<uint8_t>(0x214) = r.range(0, 4);
                                  *(uint32_t *)arena<uint8_t>(0x218) = (uint32_t)(uintptr_t)arena<void>(0x240);
                                  for (int k = 0; k < 4; k++) {
                                      arena<uint32_t>(0x240)[k] = (uint32_t)(uintptr_t)arena<void>(0x300 + k * 0x20);
                                      uint32_t *e = arena<uint32_t>(0x300 + k * 0x20);
                                      e[0] = r.range(0, 4), e[2] = e[4] = (uint32_t)(uintptr_t)arena<void>(0x400 + k * 0x40);
                                      *(int32_t *)arena<uint8_t>(0x424 + k * 0x40) = r.range(0, 3);
                                      *(uint32_t *)arena<uint8_t>(0x428 + k * 0x40) = (uint32_t)(uintptr_t)arena<void>(0x600 + k * 0x10);
                                      for (int j = 0; j < 3; j++) {
                                          arena<uint32_t>(0x600 + k * 0x10)[j] = (uint32_t)(uintptr_t)arena<void>(0x800 + (k * 3 + j) * 0x20);
                                          arena<uint8_t>(0x810 + (k * 3 + j) * 0x20)[0] = r.range(0, 2) ? (uint8_t)(r.next() & 0xf5) : (uint8_t)r.next();
                                      }
                                  }
                              },
                              [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *))fn)(arena<void>(0x100)) & 0xff}; }};
    quick(scenery_count_8606e0);
    // NamedList::Find: lists of 0-4 items with names from a few, the name there, not there or none
    Test named_list_find{0xc32c60,
                         [=](Rng &r) {
                             static const char *const names[] = {"", "a", "ab", "abc", "b"};
                             char *pool = arena<char>(0x800);
                             for (int i = 0; i < 5; i++) {
                                 int k = 0;
                                 for (; names[i][k]; k++) pool[i * 8 + k] = names[i][k];
                                 pool[i * 8 + k] = 0;
                             }
                             *(uint32_t *)arena<uint8_t>(0x104) = node_list_h2(r, 0x200, r.range(0, 4), 0x400);
                             for (int k = 0; k < 4; k++) arena<uint32_t>(0x400 + k * 0x40)[1] = (uint32_t)(uintptr_t)(pool + r.range(0, 4) * 8);
                             arena<uint32_t>(0x40)[0] = r.range(0, 5) ? (uint32_t)(uintptr_t)(pool + r.range(0, 4) * 8) : 0;
                         },
                         [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, const char *))fn)(arena<void>(0x100), (const char *)(uintptr_t)arena<uint32_t>(0x40)[0])}; }};
    quick(named_list_find);
    // Window::SumA51870: 0-8 entries of kinds 2-6 (a count of 0 or below at times)
    Test window_sum_a51870{0xa51870,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               *(int32_t *)(self + 0x10) = r.range(0, 9) ? r.range(0, 8) : -1;
                               *(uint32_t *)(self + 0xc) = (uint32_t)(uintptr_t)arena<void>(0x200);
                               for (int i = 0; i < 8; i++) arena<int32_t>(0x200)[i * 2] = r.range(2, 6), arena<uint32_t>(0x200)[i * 2 + 1] = r.next();
                           },
                           [](void *fn) {
                               Result x;
                               x.f = ((long double(__thiscall *)(const void *))fn)(arena<void>(0x100));
                               return x;
                           }};
    quick(window_sum_a51870);
    // SceneryRenderer::SiftDown: heaps of 1-12 items with values from a few (ties), any start
    Test scenery_renderer_sift_down{0x42fe40,
                                    [=](Rng &r) {
                                        uint8_t *self = arena<uint8_t>(0x100);
                                        const int n = r.range(1, 12);
                                        *(int32_t *)(self + 0x6c) = n, *(uint32_t *)(self + 0x64) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                        for (int k = 0; k < 12; k++) {
                                            arena<uint32_t>(0x200)[k] = (uint32_t)(uintptr_t)arena<void>(0x400 + k * 0x40);
                                            *(float *)arena<uint8_t>(0x424 + k * 0x40) = (float)r.range(0, 5);
                                        }
                                        arena<int32_t>(0x40)[0] = r.range(0, n - 1);
                                    },
                                    [](void *fn) {
                                        ((void(__thiscall *)(const void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0]);
                                        return Result{0};
                                    }};
    quick(scenery_renderer_sift_down);
    // Key::CompareMasked: masks 0-7 (in ecx and edx), parts from a few
    Test key_compare_masked{0xcc4aa0,
                            [=](Rng &r) {
                                arena<uint32_t>(0x40)[0] = r.range(0, 7), arena<uint32_t>(0x40)[1] = r.range(0, 7);
                                for (int i = 0; i < 6; i++) arena<int32_t>(0x50)[i] = r.range(-1, 1);
                            },
                            [](void *fn) {
                                const int32_t *a = arena<int32_t>(0x50);
                                return Result{(uint32_t)((int32_t(__fastcall *)(uint32_t, uint32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t))fn)(arena<uint32_t>(0x40)[0], arena<uint32_t>(0x40)[1], a[0], a[1], a[2], a[3], a[4], a[5])};
                            }};
    quick(key_compare_masked);
    // Value::Band: values around each of the six limits (on them at times)
    Test value_band{0x4ff3a0,
                    [=](Rng &r) {
                        static const uintptr_t limits[6] = {0xf36898, 0xf36894, 0xf36890, 0xf3688c, 0xf36888, 0xf28218};
                        const float l = *(const float *)limits[r.range(0, 5)];
                        arena<float>(0x40)[0] = r.range(0, 3) ? l + r.real(-2, 2) * (l ? l : 1) * 0.1f : l;
                    },
                    [](void *fn) { return Result{((uint32_t(__stdcall *)(float))fn)(arena<float>(0x40)[0])}; }};
    quick(value_band);
    // Bits::MergeLowest: costs below, equal to and above the best (signed words), fields of any bits (cost in edi, new cost
    // in si, fields in ecx)
    Test bits_merge_lowest{0x8e30c0,
                           [=](Rng &r) {
                               *arena<int16_t>(0x100) = (int16_t)r.range(-3, 3);
                               arena<uint8_t>(0x110)[0] = (uint8_t)r.next();
                               arena<uint32_t>(0x40)[0] = (r.next() & ~0xffffu) | (uint16_t)(int16_t)r.range(-3, 3);
                               arena<uint32_t>(0x40)[1] = r.next();
                           },
                           [](void *fn) {
                               uint32_t stack[1] = {arena<uint32_t>(0x40)[1]};
                               uint32_t *ps = stack;
                               void *cost = arena<void>(0x100), *fields = arena<void>(0x110);
                               uint32_t a = 0, si = arena<uint32_t>(0x40)[0];
                               __asm__ volatile("pushl (%%edx)\n\tcall *%%ebx" : "+a"(a), "+d"(ps), "+D"(cost), "+S"(si), "+c"(fields) : "b"(fn) : "memory", "cc");
                               return Result{a & 0xff};
                           }};
    quick(bits_merge_lowest);
    // TileQuarter::Corners: the game's tile size and base set (put back after), quarters 0-3 of any packed bits (out in
    // ecx, packed in dx)
    {
        static uint32_t saved_size, saved_base;
        Test x{0x727d30,
               [=](Rng &r) {
                   *(float *)0x1306390 = r.real(0.5f, 8), *(float *)0x13063a4 = r.range(0, 4) ? r.real(-500, 500) : -0.0f;
                   for (int i = 0; i < 8; i++) arena<uint32_t>(0x100)[i] = r.next();
                   arena<uint32_t>(0x40)[0] = r.next(), arena<uint32_t>(0x40)[1] = (r.next() & ~0xffu) | r.range(0, 3);
               },
               [](void *fn) {
                   ((void(__fastcall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0], arena<uint32_t>(0x40)[1]);
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { saved_size = *(uint32_t *)0x1306390, saved_base = *(uint32_t *)0x13063a4; };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x1306390 = saved_size, *(uint32_t *)0x13063a4 = saved_base; };
        quick(x);
    }
    // HeightPatch::Flatten: rectangles of 0-4 by 0-4 cells (empty ones at times), steps 0-6, heights of either sign
    Test height_patch_flatten{0x8db7a0,
                              [=](Rng &r) {
                                  *(int32_t *)arena<uint8_t>(0x108) = r.range(0, 2) ? r.range(1, 6) : r.range(-1, 0);
                                  uint8_t *p = arena<uint8_t>(0x200);
                                  p[4] = (uint8_t)r.range(1, 10), p[5] = (uint8_t)r.range(1, 10);   // (from 1: an end one below a start of 0 would wrap to 255)
                                  p[6] = (uint8_t)(p[4] + r.range(-1, 3)), p[7] = (uint8_t)(p[5] + r.range(-1, 3));
                                  *(uint32_t *)(p + 8) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                  *(uint32_t *)(p + 0x10) = (uint32_t)(uintptr_t)arena<void>(0x300);
                                  arena<float>(0x300)[1] = r.real(-40, 40);
                                  for (int i = 0; i < 64; i++) arena<uint32_t>(0x400)[i] = r.next();
                              },
                              [](void *fn) {
                                  ((void(__thiscall *)(const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x200));
                                  return Result{0};
                              }};
    quick(height_patch_flatten);
    // HeightPatch::Average: rectangles of 0-4 by 0-4 cells over a terrain 8 cells wide of random heights
    Test height_patch_average{0x8dc0c0,
                              [=](Rng &r) {
                                  uint8_t *p = arena<uint8_t>(0x200), *terrain = arena<uint8_t>(0x300);
                                  *(uint32_t *)p = (uint32_t)(uintptr_t)terrain;
                                  terrain[4] = 8;
                                  *(uint32_t *)(terrain + 0x24) = (uint32_t)(uintptr_t)arena<void>(0x1000);
                                  for (int i = 0; i < 8 * 8 * 6; i++) arena<float>(0x1000)[i] = r.real(-30, 30);
                                  p[4] = (uint8_t)r.range(1, 4), p[5] = (uint8_t)r.range(1, 4);   // (from 1, as above)
                                  p[6] = (uint8_t)(p[4] + r.range(-1, 3)), p[7] = (uint8_t)(p[5] + r.range(-1, 3));
                                  *(uint32_t *)(p + 8) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                  for (int i = 0; i < 64; i++) arena<uint32_t>(0x400)[i] = r.next();
                              },
                              [](void *fn) {
                                  ((void(__thiscall *)(const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x200));
                                  return Result{0};
                              }};
    quick(height_patch_average);
    // Timers::Advance: 0-6 timers near their ends and a second list of 0-3
    Test timers_advance{0x90c710,
                        [=](Rng &r) {
                            uint8_t *self = arena<uint8_t>(0x100);
                            *(int32_t *)(self + 0x48) = r.range(0, 9) ? r.range(0, 6) : -1, *(int32_t *)(self + 0x54) = r.range(0, 9) ? r.range(0, 3) : -1;
                            *(uint32_t *)(self + 0x40) = (uint32_t)(uintptr_t)arena<void>(0x200);
                            *(uint32_t *)(self + 0x4c) = (uint32_t)(uintptr_t)arena<void>(0x400);
                            for (int i = 0; i < 6 * 8; i++) arena<uint32_t>(0x200)[i] = r.next();
                            for (int k = 0; k < 6; k++) arena<float>(0x200 + k * 32)[4] = r.real(0, 5), arena<float>(0x200 + k * 32)[6] = r.real(0, 5);
                            for (int k = 0; k < 3; k++) arena<float>(0x400 + k * 32)[6] = r.real(-50, 50);
                            arena<float>(0x40)[0] = r.real(0, 2);
                        },
                        [](void *fn) {
                            ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x40)[0]);
                            return Result{0};
                        }};
    quick(timers_advance);
    // Keys::FindFrom / FindFrom2: 1-12 keys of rising times (equal ones at times), guesses anywhere, times around the keys
    for (uintptr_t at : {(uintptr_t)0xe02530, (uintptr_t)0xe02470}) {
        Test x{at,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   const int n = r.range(1, 12);
                   const bool first = at == 0xe02530;
                   *(int32_t *)(self + (first ? 0xc : 4)) = n;
                   *(uint32_t *)(self + (first ? 0x10 : 8)) = (uint32_t)(uintptr_t)arena<void>(0x200);
                   float v = r.real(-2, 2);
                   for (int k = 0; k < 12; k++) arena<float>(0x200 + k * 16)[0] = v, v += r.range(0, 4) ? r.real(0.1f, 2) : 0.0f;
                   arena<int32_t>(0x40)[0] = r.range(0, n + 1);
                   arena<float>(0x40)[1] = r.range(0, 3) ? r.real(-3, 25) : arena<float>(0x200 + r.range(0, n - 1) * 16)[0];
               },
               [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(const void *, float, int32_t))fn)(arena<void>(0x100), arena<float>(0x40)[1], arena<int32_t>(0x40)[0])}; }};
        quick(x);
    }
    // Plane::BoxCornerDistance: random planes, boxes and directions (along the plane at times)
    Test plane_box_corner_distance{0xc1e7e0,
                                   [=](Rng &r) {
                                       float *p = arena<float>(0x100);
                                       for (int c = 0; c < 3; c++) p[c] = r.range(0, 5) ? r.real(-1, 1) : 0.0f;
                                       p[4] = r.real(-10, 10);
                                       for (int c = 0; c < 3; c++) arena<float>(0x200)[c] = r.real(-10, 0), arena<float>(0x210)[c] = r.real(0, 10), arena<float>(0x220)[c] = r.real(-1, 1);
                                       if (r.range(0, 5) == 0) arena<float>(0x220)[0] = p[1], arena<float>(0x220)[1] = -p[0], arena<float>(0x220)[2] = 0;
                                       arena<uint32_t>(0x240)[0] = r.next();
                                   },
                                   [](void *fn) {
                                       return Result{(uint32_t)((int32_t(__thiscall *)(const void *, const void *, const void *, const void *, void *))fn)(arena<void>(0x100), arena<void>(0x200), arena<void>(0x210), arena<void>(0x220), arena<void>(0x240))};
                                   }};
    quick(plane_box_corner_distance);
    // Groups::Totals: 0-3 groups of 0-3 members with random counts, flags and values
    Test groups_totals{0xbd2ca0,
                       [=](Rng &r) {
                           *(uint32_t *)arena<uint8_t>(0x11c) = node_list_h2(r, 0x200, r.range(0, 3), 0x400);
                           for (int g = 0; g < 3; g++) {
                               uint8_t *grp = arena<uint8_t>(0x400 + g * 0x40);
                               for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x400 + g * 0x40)[i] = r.next();
                               *(uint32_t *)(grp + 0x14) = node_list_h2(r, 0x600 + g * 0x200, r.range(0, 3), 0x1000 + g * 0x100);
                               for (int m = 0; m < 3; m++) {
                                   uint8_t *mem = arena<uint8_t>(0x1000 + g * 0x100 + m * 0x40);
                                   for (int i = 0; i < 0x10; i++) arena<uint32_t>(0x1000 + g * 0x100 + m * 0x40)[i] = r.next();
                                   *(float *)(mem + 0x44) = r.real(-5, 5), *(float *)(mem + 0x48) = r.real(-5, 5);
                               }
                           }
                       },
                       [](void *fn) {
                           ((void(__thiscall *)(const void *))fn)(arena<void>(0x100));
                           return Result{0};
                       }};
    quick(groups_totals);
    // Water::Entry970F20: heaps of 1-12 entries with keys from a few (ties), any start
    Test water_entry_970f20{0x970f20,
                            [=](Rng &r) {
                                *(uint32_t *)arena<uint8_t>(0x190) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                const int n = r.range(1, 12);
                                for (int k = 0; k < 12; k++) arena<uint32_t>(0x200)[k * 2] = r.next(), arena<float>(0x200)[k * 2 + 1] = (float)r.range(0, 5);
                                arena<int32_t>(0x40)[0] = r.range(0, n - 1), arena<int32_t>(0x40)[1] = n;
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(const void *, int32_t, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<int32_t>(0x40)[1]);
                                return Result{0};
                            }};
    quick(water_entry_970f20);
    // Water::Set93CC50: water or none, turned or not, levels around the cell's heights
    Test water_set_93cc50{0x93cc50,
                          [=](Rng &r) {
                              uint8_t *self = arena<uint8_t>(0x100), *terrain = arena<uint8_t>(0x200);
                              *(uint32_t *)(self + 4) = r.range(0, 5) ? (uint32_t)(uintptr_t)arena<void>(0x300) : 0;
                              *(uint32_t *)(self + 0x14) = r.next();
                              self[0] = (uint8_t)r.range(0, 3), self[1] = (uint8_t)r.range(0, 3), self[2] = r.range(0, 1) ? (uint8_t)r.range(1, 255) : 0;
                              terrain[4] = 4;
                              *(uint32_t *)(terrain + 0x24) = (uint32_t)(uintptr_t)arena<void>(0x1000);
                              for (int i = 0; i < 16 * 6; i++) arena<float>(0x1000)[i] = (float)r.range(0, 4);
                              *(float *)arena<uint8_t>(0x330) = r.range(0, 1) ? (float)r.range(0, 4) : r.real(0, 4);
                          },
                          [](void *fn) {
                              ((void(__thiscall *)(void *, const void *))fn)(arena<void>(0x100), arena<void>(0x200));
                              return Result{0};
                          }};
    quick(water_set_93cc50);
    // Water::Value9060F0: grids 8 wide of random samples, rows and columns inside, runs of -1..7
    Test water_value_9060f0{0x9060f0,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                *(int32_t *)self = 8, *(int32_t *)(self + 4) = 8;
                                *(uint32_t *)(self + 0x44) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                *(uint32_t *)(self + 0x4c) = (uint32_t)(uintptr_t)arena<void>(0x600);
                                *(uint32_t *)(self + 0x50) = (uint32_t)(uintptr_t)arena<void>(0x800);
                                for (int i = 0; i < 0x180; i++) arena<float>(0x400)[i] = r.real(-10, 10);
                                arena<int32_t>(0x40)[0] = r.range(0, 1), arena<int32_t>(0x40)[1] = r.range(0, 6), arena<int32_t>(0x40)[2] = r.range(-1, 7);
                            },
                            [](void *fn) {
                                const int32_t *a = arena<int32_t>(0x40);
                                Result x;
                                x.f = ((long double(__thiscall *)(const void *, int32_t, int32_t, int32_t))fn)(arena<void>(0x100), a[0], a[1], a[2]);
                                return x;
                            }};
    quick(water_value_9060f0);
    // Fireworks::Find989300: 0-8 items whose parts have values either side of 0 (ties at times)
    Test fireworks_find_989300{0x989300,
                               [=](Rng &r) {
                                   uint8_t *o = arena<uint8_t>(0x100);
                                   *(int32_t *)(o + 0x58) = r.range(0, 9) ? r.range(0, 8) : -1;
                                   *(uint32_t *)(o + 0xcc) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                   for (int k = 0; k < 8; k++) {
                                       arena<uint32_t>(0x200)[k] = (uint32_t)(uintptr_t)arena<void>(0x400 + k * 0xc0);
                                       *(uint32_t *)arena<uint8_t>(0x4a0 + k * 0xc0) = (uint32_t)(uintptr_t)arena<void>(0x1000 + k * 0x60);
                                       *(float *)arena<uint8_t>(0x104c + k * 0x60) = r.range(0, 1) ? (float)r.range(-2, 3) : r.real(-3, 3);
                                   }
                               },
                               [](void *fn) { return Result{((uint32_t(__stdcall *)(const void *))fn)(arena<void>(0x100))}; }};
    quick(fireworks_find_989300);
    // Scenery::Slot8847A0: ranges and scales, 0-8 sorted stops (ties), values anywhere
    Test scenery_slot_8847a0{0x8847a0,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100);
                                 *(float *)(self + 0x7c) = r.real(-50, 50), *(float *)(self + 0x84) = *(float *)(self + 0x7c) + r.real(10, 200);
                                 *(float *)(self + 0xdc) = r.real(1, 10);
                                 *(int32_t *)(self + 0xf4) = r.range(0, 9) ? r.range(0, 8) : -1;
                                 *(uint32_t *)(self + 0xfc) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                 float s = r.real(-5, 0);
                                 for (int i = 0; i < 8; i++) arena<float>(0x400)[i] = s, s += r.range(0, 3) ? r.real(0.2f, 3) : 0.0f;
                                 arena<float>(0x40)[0] = r.real(-20, 220);
                             },
                             [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(const void *, float))fn)(arena<void>(0x100), arena<float>(0x40)[0])}; }};
    quick(scenery_slot_8847a0);
    // Track::LengthBC64E0: two end lists' first points (the same at times)
    Test track_length_bc64e0{0xbc64e0,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100);
                                 *(uint32_t *)(self + 0x18) = r.next() | 0x400000, *(int32_t *)(self + 0xe4) = r.range(2, 4);
                                 *(uint32_t *)(self + 0x110) = (uint32_t)(uintptr_t)arena<void>(0x300);
                                 *(uint32_t *)(self + 0x10c) = (uint32_t)(uintptr_t)arena<void>(0x320);
                                 arena<uint32_t>(0x300)[1] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<uint32_t>(0x320)[1] = (uint32_t)(uintptr_t)arena<void>(0x420);
                                 for (int c = 0; c < 3; c++) arena<float>(0x400)[c] = r.real(-30, 30), arena<float>(0x420)[c] = r.range(0, 5) ? r.real(-30, 30) : arena<float>(0x400)[c];
                                 for (int c = 0; c < 3; c++) arena<uint32_t>(0x60)[c] = r.next();
                             },
                             [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, void *))fn)(arena<void>(0x100), arena<void>(0x60))}; }};
    quick(track_length_bc64e0);
    // Scenery::Cell88F3F0: three levels of grids (8, 4, 2 wide), cells in use or not, any start level, bounds either side
    Test scenery_cell_88f3f0{0x88f3f0,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100);
                                 *(uint32_t *)(self + 0x390) = (uint32_t)(uintptr_t)arena<void>(0x500);
                                 *(int32_t *)(self + 0x394) = 3;
                                 for (int l = 0; l < 3; l++) {
                                     uint32_t *g = arena<uint32_t>(0x540 + l * 0x10);
                                     arena<uint32_t>(0x500)[l] = (uint32_t)(uintptr_t)g;
                                     g[0] = 8 >> l, g[2] = (uint32_t)(uintptr_t)arena<void>(0x1000 + l * 0x1000);
                                     for (int c = 0; c < 64 >> (2 * l); c++) {
                                         float *cell = arena<float>(0x1000 + l * 0x1000 + c * 64);
                                         for (int k = 0; k < 12; k++) cell[k] = r.real(-10, 10);
                                         *(int32_t *)(cell + 12) = r.range(-1, 3);
                                     }
                                 }
                                 for (int k = 0; k < 3; k++) arena<float>(0x40)[k] = r.real(-12, 12), arena<float>(0x50)[k] = r.real(-12, 12);
                                 arena<int32_t>(0x60)[0] = r.range(0, 2), arena<int32_t>(0x60)[1] = r.range(0, 7) >> arena<int32_t>(0x60)[0], arena<int32_t>(0x60)[2] = r.range(0, 7) >> arena<int32_t>(0x60)[0];
                             },
                             [](void *fn) {
                                 const int32_t *a = arena<int32_t>(0x60);
                                 ((void(__thiscall *)(const void *, int32_t, int32_t, int32_t, const void *, const void *))fn)(arena<void>(0x100), a[0], a[1], a[2], arena<void>(0x40), arena<void>(0x50));
                                 return Result{0};
                             }};
    quick(scenery_cell_88f3f0);
    // Particles::Pass9E16E0: the game's pull and lifetime set (put back after), on or off, 0-6 particles of any age
    {
        static uint32_t saved_pull, saved_life;
        Test x{0x9e16e0,
               [=](Rng &r) {
                   *(float *)0x10f0e8c = r.real(-10, 0), *(float *)0x10f0e90 = r.real(1, 3);
                   uint8_t *self = arena<uint8_t>(0x100);
                   self[0x10] = r.range(0, 4) ? 1 : 0;
                   *(int32_t *)(self + 0x1c) = r.range(0, 9) ? r.range(0, 6) : -1;
                   *(uint32_t *)(self + 0x14) = (uint32_t)(uintptr_t)arena<void>(0x400);
                   for (int i = 0; i < 6 * 12; i++) arena<float>(0x400)[i] = r.real(-5, 5);
                   for (int k = 0; k < 6; k++) arena<float>(0x400 + k * 0x30)[8] = r.real(0, 3);
                   arena<float>(0x40)[0] = r.real(0, 0.5f);
               },
               [](void *fn) {
                   ((void(__thiscall *)(void *, float))fn)(arena<void>(0x100), arena<float>(0x40)[0]);
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { saved_pull = *(uint32_t *)0x10f0e8c, saved_life = *(uint32_t *)0x10f0e90; };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x10f0e8c = saved_pull, *(uint32_t *)0x10f0e90 = saved_life; };
        quick(x);
    }
    // Toolbar::Find79C0A0: the game's toolbar (put back after): 0-8 items near and far, kinds 0-5, counts given or not
    // (point in ecx, counts in edx)
    {
        static uint32_t saved[8];
        Test x{0x79c0a0,
               [=](Rng &r) {
                   uint32_t *g = (uint32_t *)0x1306bc8;
                   g[0] = (uint32_t)(uintptr_t)arena<void>(0x200);
                   g[1] = r.range(0, 4) ? 1 : 0, g[4] = 0;
                   g[6] = r.range(0, 3), g[7] = g[6] + r.range(0, 5);
                   for (int k = 0; k < 8; k++) {
                       arena<uint32_t>(0x200)[k] = r.range(0, 4) ? (uint32_t)(uintptr_t)arena<void>(0x400 + k * 0xc0) : 0;
                       uint8_t *item = arena<uint8_t>(0x400 + k * 0xc0);
                       *(int32_t *)(item + 0xa4) = r.range(0, 5);
                       *(uint32_t *)(item + 0xac) = (uint32_t)(uintptr_t)arena<void>(0x1000 + k * 0x40);
                       *(uint32_t *)arena<uint8_t>(0x1030 + k * 0x40) = (uint32_t)(uintptr_t)arena<void>(0x1400 + k * 0x100);
                       for (int c = 0; c < 3; c++) arena<float>(0x1490 + k * 0x100)[c] = r.real(-10, 10);
                   }
                   for (int c = 0; c < 3; c++) arena<float>(0x40)[c] = r.real(-10, 10);
                   for (int i = 0; i < 6; i++) arena<uint32_t>(0x60)[i] = r.next();
                   arena<uint32_t>(0x50)[0] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x60) : 0;
                   arena<float>(0x50)[1] = r.real(0, 15);
               },
               [](void *fn) { return Result{((uint32_t(__fastcall *)(const void *, void *, float))fn)(arena<void>(0x40), (void *)(uintptr_t)arena<uint32_t>(0x50)[0], arena<float>(0x50)[1])}; }};
        x.prepare = [](const rewrite::Entry *) { memcpy(saved, (void *)0x1306bc8, 32); };
        x.finish = [](const rewrite::Entry *) { memcpy((void *)0x1306bc8, saved, 32); };
        quick(x);
    }
    // QuadTree::FindNode: square roots of 64-1024 and boxes of every size within them (start in ecx, limit in edx)
    Test quad_tree_find_node{0xcdb600,
                             [=](Rng &r) {
                                 const float side = (float)(64 << r.range(0, 4));
                                 float *a = arena<float>(0x100);
                                 a[0] = r.real(-500, 0), a[1] = r.real(-500, 0), a[2] = side, a[3] = side;   // corner, size
                                 const float s = side / (float)(1 << r.range(0, 7)) * r.real(0.3f, 1.2f);
                                 a[6] = s, a[7] = r.range(0, 1) ? s : s * r.real(0.3f, 2);   // box size
                                 a[4] = a[0] + r.real(0, side - s), a[5] = a[1] + r.real(0, side - a[7]);   // box corner
                                 arena<int32_t>(0x40)[0] = 0, arena<int32_t>(0x40)[1] = r.range(0, 3) ? 1365 : r.range(1, 100);
                             },
                             [](void *fn) {
                                 const float *a = arena<float>(0x100);
                                 return Result{(uint32_t)((int32_t(__fastcall *)(int32_t, int32_t, const void *, const void *, const void *, const void *))fn)(arena<int32_t>(0x40)[0], arena<int32_t>(0x40)[1], a, a + 2, a + 4, a + 6)};
                             }};
    quick(quad_tree_find_node);
    // Water::Time8E6880: the game's day pointer set (put back after) to times before, in and past the day, modes 0-3
    {
        static uint32_t saved_day;
        Test x{0x8e6880,
               [=](Rng &r) {
                   *(uint32_t *)0x1103b6c = (uint32_t)(uintptr_t)arena<void>(0x1000);
                   float *d = arena<float>(0x1000);
                   d[0x20 / 4] = r.real(0, 100), d[0x200 / 4] = d[0x20 / 4] + r.real(1, 500);
                   d[0x110 / 4] = d[0x20 / 4] + r.real(-50, 600);
                   *(int32_t *)arena<uint8_t>(0x280) = r.range(0, 3), *(int32_t *)arena<uint8_t>(0x284) = r.range(0, 3);
                   arena<uint32_t>(0x40)[0] = r.next();
               },
               [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, void *))fn)(arena<void>(0x100), arena<void>(0x40))}; }};
        x.prepare = [](const rewrite::Entry *) { saved_day = *(uint32_t *)0x1103b6c; };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x1103b6c = saved_day; };
        quick(x);
    }
    // Track::LastBC65D0: two end lists of 1-4 points, their last points (the same at times)
    Test track_last_bc65d0{0xbc65d0,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               *(uint32_t *)(self + 0x18) = r.next() | 0x400000, *(int32_t *)(self + 0xe4) = r.range(2, 4);
                               *(uint32_t *)(self + 0x10c) = (uint32_t)(uintptr_t)arena<void>(0x300);
                               *(uint32_t *)(self + 0x110) = (uint32_t)(uintptr_t)arena<void>(0x320);
                               arena<uint32_t>(0x300)[0] = r.range(1, 4), arena<uint32_t>(0x300)[1] = (uint32_t)(uintptr_t)arena<void>(0x400);
                               arena<uint32_t>(0x320)[0] = r.range(1, 4), arena<uint32_t>(0x320)[1] = (uint32_t)(uintptr_t)arena<void>(0x600);
                               for (int i = 0; i < 36; i++) arena<float>(0x400)[i] = r.real(-30, 30), arena<float>(0x600)[i] = r.range(0, 5) ? r.real(-30, 30) : arena<float>(0x400)[i];
                               if (r.range(0, 5) == 0)
                                   for (int c = 0; c < 3; c++) arena<float>(0x600 + (arena<uint32_t>(0x320)[0] - 1) * 36)[c] = arena<float>(0x400 + (arena<uint32_t>(0x300)[0] - 1) * 36)[c];
                               for (int c = 0; c < 3; c++) arena<uint32_t>(0x60)[c] = r.next();
                           },
                           [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, void *))fn)(arena<void>(0x100), arena<void>(0x60))}; }};
    quick(track_last_bc65d0);
    // Track::LengthBC62A0: two end lists' first points with random vectors (opposite at times)
    Test track_length_bc62a0{0xbc62a0,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100);
                                 *(uint32_t *)(self + 0x18) = r.next() | 0x400000, *(int32_t *)(self + 0xe4) = r.range(2, 4);
                                 *(uint32_t *)(self + 0x110) = (uint32_t)(uintptr_t)arena<void>(0x300);
                                 *(uint32_t *)(self + 0x10c) = (uint32_t)(uintptr_t)arena<void>(0x320);
                                 arena<uint32_t>(0x300)[1] = (uint32_t)(uintptr_t)arena<void>(0x400), arena<uint32_t>(0x320)[1] = (uint32_t)(uintptr_t)arena<void>(0x500);
                                 for (int c = 0; c < 3; c++) arena<float>(0x418)[c] = r.real(-3, 3), arena<float>(0x518)[c] = r.range(0, 5) ? r.real(-3, 3) : -arena<float>(0x418)[c];
                                 for (int c = 0; c < 3; c++) arena<uint32_t>(0x60)[c] = r.next();
                             },
                             [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, void *))fn)(arena<void>(0x100), arena<void>(0x60))}; }};
    quick(track_length_bc62a0);
    // Track::PassBB7470: 1-6 pieces with runs of 2-6 sorted keys each (the key below each run's last; guesses inside), or no
    // runs, flags set or not
    Test track_pass_bb7470{0xbb7470,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100), *t = arena<uint8_t>(0x200);
                               *(uint32_t *)(self + 0x10) = (uint32_t)(uintptr_t)t;
                               const int n = r.range(1, 6);
                               *(uint32_t *)(t + 0x20) = n;
                               *(uint32_t *)(self + 0x3c) = (uint32_t)(uintptr_t)arena<void>(0x2c0);
                               *(uint32_t *)arena<uint8_t>(0x2c8) = (uint32_t)(uintptr_t)arena<void>(0x2e0);
                               for (int i = 0; i < 6; i++) arena<uint16_t>(0x2e0)[i] = (uint16_t)r.range(0, 15);
                               *(uint32_t *)(t + 0x30) = (uint32_t)(uintptr_t)arena<void>(0x400);
                               for (int i = 0; i < 0x40; i++) arena<uint8_t>(0x400)[i] = r.range(0, 1) ? (uint8_t)r.next() : 0;
                               const uint16_t key = (uint16_t)r.range(0, 8);
                               *(uint16_t *)(self + 0x38) = key, *(uint16_t *)(self + 0x3a) = (uint16_t)r.range(0, 3);
                               *(float *)(self + 0x2c) = r.real(0, 0.5f);
                               const bool runs = r.range(0, 4) != 0;
                               *(uint32_t *)(t + 0x40) = runs ? (uint32_t)(uintptr_t)arena<void>(0x500) : 0;
                               *(uint32_t *)(t + 0x50) = (uint32_t)(uintptr_t)arena<void>(0x600);
                               for (int i = 0; i < n; i++) {
                                   const int cnt = r.range(2, 6);
                                   arena<uint16_t>(0x500)[i * 2] = (uint16_t)cnt, arena<uint16_t>(0x500)[i * 2 + 1] = (uint16_t)(i * 6);
                                   uint16_t v = (uint16_t)r.range(0, 2);
                                   for (int k = 0; k < cnt; k++) arena<uint16_t>(0x600)[i * 6 + k] = v, v = (uint16_t)(v + r.range(0, 3));
                                   if (arena<uint16_t>(0x600)[i * 6 + cnt - 1] <= key) arena<uint16_t>(0x600)[i * 6 + cnt - 1] = (uint16_t)(key + 1);
                               }
                               for (int i = 0; i < 16; i++) arena<uint8_t>(0x700)[i] = (uint8_t)r.next();
                           },
                           [](void *fn) {
                               ((void(__thiscall *)(const void *, void *))fn)(arena<void>(0x100), arena<void>(0x700));
                               return Result{0};
                           }};
    quick(track_pass_bb7470);
    // Water::Cell93DA00: grids of 1-4 by 1-4 cells, places in and out, cells with both, one or no parts (levels tied at
    // times)
    Test water_cell_93da00{0x93da00,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               self[0x58] = (uint8_t)r.range(1, 4), self[0x59] = (uint8_t)r.range(1, 4);
                               *(uint32_t *)(self + 0x5c) = (uint32_t)(uintptr_t)arena<void>(0x400);
                               for (int c = 0; c < 16; c++) {
                                   uint32_t *cell = arena<uint32_t>(0x400 + c * 0x30);
                                   cell[1] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x1000 + c * 0x80) : 0;
                                   cell[7] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x1040 + c * 0x80) : 0;
                                   *(float *)arena<uint8_t>(0x1030 + c * 0x80) = (float)r.range(0, 3), *(float *)arena<uint8_t>(0x1070 + c * 0x80) = (float)r.range(0, 3);
                               }
                               arena<int32_t>(0x40)[0] = r.range(-1, 4), arena<int32_t>(0x40)[1] = r.range(-1, 4);
                               arena<uint32_t>(0x50)[0] = r.next(), arena<uint32_t>(0x50)[1] = r.next();
                           },
                           [](void *fn) {
                               return Result{((uint32_t(__thiscall *)(const void *, int32_t, int32_t, void *, void *))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<int32_t>(0x40)[1], arena<void>(0x50), arena<void>(0x54)) & 0xff};
                           }};
    quick(water_cell_93da00);
    // Grid::Round435EA0: grids of 1-6 by 1-6 cells of random corner heights, points in every cell and outside
    Test grid_round_435ea0{0x435ea0,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               self[4] = (uint8_t)r.range(1, 6), self[5] = (uint8_t)r.range(1, 6);
                               *(float *)(self + 0xc) = r.real(-50, 50), *(float *)(self + 0x10) = r.real(-50, 50);
                               *(float *)(self + 0x1c) = r.real(0.2f, 2), *(float *)(self + 0x20) = r.real(0.2f, 2);
                               *(uint32_t *)(self + 0x24) = (uint32_t)(uintptr_t)arena<void>(0x400);
                               for (int i = 0; i < 36 * 6; i++) arena<float>(0x400)[i] = r.real(-20, 20);
                               arena<float>(0x40)[0] = *(float *)(self + 0xc) + r.real(-1, 7) / *(float *)(self + 0x1c);
                               arena<float>(0x40)[1] = *(float *)(self + 0x10) + r.real(-1, 7) / *(float *)(self + 0x20);
                           },
                           [](void *fn) {
                               Result x;
                               x.f = ((long double(__thiscall *)(const void *, float, float))fn)(arena<void>(0x100), arena<float>(0x40)[0], arena<float>(0x40)[1]);
                               return x;
                           }};
    quick(grid_round_435ea0);
    // Track::LengthBC63B0: two end lists of 1-4 points with random last vectors (opposite at times)
    Test track_length_bc63b0{0xbc63b0,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100);
                                 *(uint32_t *)(self + 0x18) = r.next() | 0x400000, *(int32_t *)(self + 0xe4) = r.range(2, 4);
                                 *(uint32_t *)(self + 0x110) = (uint32_t)(uintptr_t)arena<void>(0x300);
                                 *(uint32_t *)(self + 0x10c) = (uint32_t)(uintptr_t)arena<void>(0x320);
                                 arena<uint32_t>(0x300)[0] = r.range(1, 4), arena<uint32_t>(0x300)[1] = (uint32_t)(uintptr_t)arena<void>(0x400);
                                 arena<uint32_t>(0x320)[0] = r.range(1, 4), arena<uint32_t>(0x320)[1] = (uint32_t)(uintptr_t)arena<void>(0x600);
                                 for (int i = 0; i < 36; i++) arena<float>(0x400)[i] = r.real(-3, 3), arena<float>(0x600)[i] = r.real(-3, 3);
                                 if (r.range(0, 4) == 0)
                                     for (int c = 0; c < 3; c++) arena<float>(0x60c + (arena<uint32_t>(0x320)[0] - 1) * 36)[c] = -arena<float>(0x40c + (arena<uint32_t>(0x300)[0] - 1) * 36)[c];
                                 for (int c = 0; c < 3; c++) arena<uint32_t>(0x60)[c] = r.next();
                             },
                             [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, void *))fn)(arena<void>(0x100), arena<void>(0x60))}; }};
    quick(track_length_bc63b0);
    // Track::RoundBB69E0: 2-8 sorted keys (the key below the last; ties and keys below the first), scales 0-1 against
    // counts 2-20
    Test track_round_bb69e0{0xbb69e0,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                *(uint32_t *)(self + 0x10) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                const int m = r.range(2, 20);
                                arena<int32_t>(0x200)[1] = m;
                                const int n = r.range(2, 8);
                                uint16_t v = (uint16_t)r.range(0, 3);
                                for (int k = 0; k < n; k++) arena<uint16_t>(0x300)[k] = v, v = (uint16_t)(v + r.range(0, 4));
                                uint16_t key = (uint16_t)r.range(0, arena<uint16_t>(0x300)[n - 1] + 1);
                                if (arena<uint16_t>(0x300)[n - 1] <= key) arena<uint16_t>(0x300)[n - 1] = (uint16_t)(key + 1 + r.range(0, 2));
                                *(uint16_t *)(self + 0x38) = key;
                                *(float *)(self + 0x2c) = r.range(0, 3) ? (key + r.real(0, 1.2f)) / (float)(m - 1) : r.real(0, 0.9f);
                                if (*(float *)(self + 0x2c) * n >= n - 0.6f) *(float *)(self + 0x2c) = r.real(0, (n - 0.6f) / n);
                                arena<uint32_t>(0x40)[0] = n;
                                arena<uint32_t>(0x50)[0] = r.next(), arena<uint32_t>(0x50)[1] = r.next();
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(const void *, const void *, uint32_t, void *, void *))fn)(arena<void>(0x100), arena<void>(0x300), arena<uint32_t>(0x40)[0], arena<void>(0x50), arena<void>(0x54));
                                return Result{0};
                            }};
    quick(track_round_bb69e0);
    // Camera::TimeCD0510: tracks of 1-8 rising key times, t below, inside (on keys at times) and past the end, with or
    // without wrap
    Test camera_time_cd0510{0xcd0510,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                *(uint32_t *)(self + 0x24) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                const int k = r.range(0, 2), count = r.range(0, 5) ? r.range(2, 8) : 1;
                                uint8_t *e = arena<uint8_t>(0x200 + k * 64);
                                *(int32_t *)(e + 0x24) = count, *(uint32_t *)(e + 0x28) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                float tm = r.real(0.1f, 2);
                                for (int j = 0; j < count; j++) arena<float>(0x400)[j] = tm, tm += r.real(0.1f, 2);
                                const float last = arena<float>(0x400)[count - 1];
                                float t;
                                switch (r.range(0, 4)) {
                                case 0: t = r.real(-2, 0); break;
                                case 1: t = arena<float>(0x400)[r.range(0, count - 1)]; break;
                                case 2: t = r.real(last, last * 4); break;
                                default: t = r.real(0, last); break;
                                }
                                arena<int32_t>(0x40)[0] = k, arena<float>(0x40)[1] = t, arena<uint32_t>(0x40)[2] = r.range(0, 1) | (r.next() & 0xff00);
                                for (int j = 0; j < 3; j++) arena<uint32_t>(0x60)[j] = r.next();
                            },
                            [](void *fn) {
                                const uint32_t *a = arena<uint32_t>(0x40);
                                ((void(__thiscall *)(const void *, int32_t, float, uint32_t, void *, void *, void *))fn)(arena<void>(0x100), (int32_t)a[0], arena<float>(0x40)[1], a[2], arena<void>(0x60), arena<void>(0x64), arena<void>(0x68));
                                return Result{0};
                            }};
    quick(camera_time_cd0510);
    // PoolComplex::LoopAAA110: heaps of 1-12 node numbers over 12 nodes with small whole keys (ties), any place i
    Test pool_complex_loop_aaa110{0xaaa110,
                                  [=](Rng &r) {
                                      *arena<uint32_t>(0x100) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                      *arena<uint32_t>(0x110) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                      const int n = r.range(1, 12);
                                      for (int j = 0; j < 12; j++) {
                                          arena<uint32_t>(0x200)[j] = r.range(0, 11);
                                          float *node = arena<float>(0x400 + j * 0x38);
                                          node[0x28 / 4] = (float)r.range(0, 4), node[0x2c / 4] = r.range(0, 1) ? (float)r.range(0, 4) : r.real(0, 4);
                                      }
                                      arena<int32_t>(0x40)[0] = r.range(0, n - 1), arena<int32_t>(0x40)[1] = n;
                                  },
                                  [](void *fn) {
                                      ((void(__thiscall *)(const void *, int32_t, int32_t, const void *))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<int32_t>(0x40)[1], arena<void>(0x110));
                                      return Result{0};
                                  }};
    quick(pool_complex_loop_aaa110);
    // PoolComplex::PointsAAA300: heaps of 1-12 node numbers over 12 nodes with small whole keys (ties)
    Test pool_complex_points_aaa300{0xaaa300,
                                    [=](Rng &r) {
                                        uint32_t *self = arena<uint32_t>(0x100);
                                        self[0] = (uint32_t)(uintptr_t)arena<void>(0x200), self[2] = r.range(1, 12);
                                        *arena<uint32_t>(0x110) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                        for (int j = 0; j < 12; j++) {
                                            arena<uint32_t>(0x200)[j] = r.range(0, 11);
                                            float *node = arena<float>(0x400 + j * 0x38);
                                            node[0x28 / 4] = (float)r.range(0, 4), node[0x2c / 4] = r.range(0, 1) ? (float)r.range(0, 4) : r.real(0, 4);
                                        }
                                    },
                                    [](void *fn) {
                                        ((void(__thiscall *)(void *, const void *))fn)(arena<void>(0x100), arena<void>(0x110));
                                        return Result{0};
                                    }};
    quick(pool_complex_points_aaa300);
    // Person::Loop7E6750: heaps of 1-12 item numbers over 12 items with small whole keys (ties), any place i
    Test person_loop_7e6750{0x7e6750,
                            [=](Rng &r) {
                                *arena<uint32_t>(0x100) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                *arena<uint32_t>(0x110) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                const int n = r.range(1, 12);
                                for (int j = 0; j < 12; j++) {
                                    arena<uint32_t>(0x200)[j] = r.range(0, 11);
                                    arena<float>(0x408 + j * 20)[0] = r.range(0, 1) ? (float)r.range(0, 4) : r.real(0, 4);
                                }
                                arena<int32_t>(0x40)[0] = r.range(0, n - 1), arena<int32_t>(0x40)[1] = n;
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(const void *, int32_t, int32_t, const void *))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<int32_t>(0x40)[1], arena<void>(0x110));
                                return Result{0};
                            }};
    quick(person_loop_7e6750);
    // Camera::RayCD7DC0: random camera frames, fields of view 0.3-2, flat or not, points ahead, behind and to the sides
    Test camera_ray_cd7dc0{0xcd7dc0,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               for (int k = 0; k < 12; k++) *(float *)(self + 0x120 + k * 4) = r.real(-1, 1);
                               for (int k = 0; k < 3; k++) *(float *)(self + 0x160 + k * 4) = r.real(-20, 20);
                               *(float *)(self + 0x188) = r.real(0, 2), *(float *)(self + 0x190) = r.real(0.3f, 2);
                               *(float *)(self + 0x194) = r.real(0.5f, 2), *(float *)(self + 0x198) = r.real(1, 30);
                               self[0x1a6] = r.range(0, 2) == 0;
                               for (int k = 0; k < 3; k++) arena<float>(0x40)[k] = *(float *)(self + 0x160 + k * 4) + r.real(-30, 30);
                               arena<float>(0x50)[0] = r.real(100, 1000), arena<float>(0x50)[1] = r.real(100, 1000);
                               arena<uint32_t>(0x60)[0] = r.next(), arena<uint32_t>(0x60)[1] = r.next();
                           },
                           [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, void *, const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x60), arena<void>(0x40), arena<void>(0x50))}; }};
    quick(camera_ray_cd7dc0);
    // Vector::CompareCDF200: two triangles over 8 points of small whole or random values (ties), random directions (a in
    // ecx, b in edx)
    Test vector_compare_cdf200{0xcdf200,
                               [=](Rng &r) {
                                   const bool whole = r.range(0, 1);
                                   for (int k = 0; k < 32; k++) arena<float>(0x200)[k] = whole ? (float)r.range(-3, 3) : r.real(-10, 10);
                                   for (int k = 0; k < 3; k++) arena<float>(0x40)[k] = whole ? (float)r.range(-2, 2) : r.real(-1, 1);
                                   for (int k = 0; k < 6; k++) arena<uint32_t>(0x60)[k] = r.range(0, 7);
                               },
                               [](void *fn) { return Result{(uint32_t)((int32_t(__fastcall *)(const void *, const void *, const void *, const void *))fn)(arena<void>(0x60), arena<void>(0x6c), arena<void>(0x200), arena<void>(0x40))}; }};
    quick(vector_compare_cdf200);
    // Person::Points7E6C20: heaps of 1-12 item numbers over 12 items with small whole keys (ties)
    Test person_points_7e6c20{0x7e6c20,
                              [=](Rng &r) {
                                  uint32_t *self = arena<uint32_t>(0x100);
                                  self[0] = (uint32_t)(uintptr_t)arena<void>(0x200), self[2] = r.range(1, 12);
                                  *arena<uint32_t>(0x110) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                  for (int j = 0; j < 12; j++) {
                                      arena<uint32_t>(0x200)[j] = r.range(0, 11);
                                      arena<float>(0x408 + j * 20)[0] = r.range(0, 1) ? (float)r.range(0, 4) : r.real(0, 4);
                                  }
                              },
                              [](void *fn) {
                                  ((void(__thiscall *)(void *, const void *))fn)(arena<void>(0x100), arena<void>(0x110));
                                  return Result{0};
                              }};
    quick(person_points_7e6c20);
    // Terrain::Cell8BCE50: grids of 1-5 by 1-5 cells, points on every edge and inside, all four corners
    Test terrain_cell_8bce50{0x8bce50,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100);
                                 self[4] = (uint8_t)r.range(1, 5), self[5] = (uint8_t)r.range(1, 5);
                                 for (int k = 0; k < 4; k++) *(float *)(self + 0xc + k * 4) = r.real(-50, 50);
                                 *(float *)(self + 0x14) = r.real(0.5f, 8), *(float *)(self + 0x18) = r.real(0.5f, 8);
                                 *(uint32_t *)(self + 0x24) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                 for (int k = 0; k < 25 * 6; k++) arena<uint32_t>(0x400)[k] = r.next();
                                 for (int k = 0; k < 16; k++) arena<uint32_t>(0x200)[k] = r.next();
                                 arena<uint32_t>(0x40)[0] = r.range(0, self[4] + 1), arena<uint32_t>(0x40)[1] = r.range(0, self[5] + 1);
                                 arena<uint32_t>(0x40)[2] = r.range(0, 3) | (r.next() & 0xfc);
                             },
                             [](void *fn) {
                                 const uint32_t *a = arena<uint32_t>(0x40);
                                 return Result{(uint32_t)((int32_t(__thiscall *)(const void *, void *, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<void>(0x200), a[0], a[1], a[2])};
                             }};
    quick(terrain_cell_8bce50);
    // Water::Value9061C0: grids 4-8 wide of random values, columns of 0-6 rows from any place
    Test water_value_9061c0{0x9061c0,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                const int w = r.range(4, 8);
                                *(int32_t *)(self + 4) = w;
                                *(uint32_t *)(self + 0x48) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                *(uint32_t *)(self + 0x4c) = (uint32_t)(uintptr_t)arena<void>(0x600);
                                *(uint32_t *)(self + 0x50) = (uint32_t)(uintptr_t)arena<void>(0x800);
                                for (int k = 0; k < 0x80; k++) arena<float>(0x400)[k] = r.real(-10, 10);
                                arena<int32_t>(0x40)[0] = r.range(0, w - 1), arena<int32_t>(0x40)[1] = r.range(0, 2), arena<int32_t>(0x40)[2] = r.range(-1, 6);
                            },
                            [](void *fn) {
                                const int32_t *a = arena<int32_t>(0x40);
                                Result x;
                                x.f = ((long double(__thiscall *)(const void *, int32_t, int32_t, int32_t))fn)(arena<void>(0x100), a[0], a[1], a[2]);
                                return x;
                            }};
    quick(water_value_9061c0);
    // Water::Range95F910: boxes of 0-7 by 0-7 pixels (empty or backwards at times) in a 10-wide image of random pixels
    // (pixels in ecx, width in edx)
    Test water_range_95f910{0x95f910,
                            [=](Rng &r) {
                                for (int k = 0; k < 100; k++) arena<uint32_t>(0x400)[k] = r.next();
                                int32_t *a = arena<int32_t>(0x40);
                                a[0] = r.range(0, 3), a[1] = a[0] + r.range(-1, 7), a[2] = r.range(0, 2), a[3] = a[2] + r.range(-1, 7);
                            },
                            [](void *fn) {
                                const int32_t *a = arena<int32_t>(0x40);
                                ((void(__fastcall *)(void *, int32_t, int32_t, int32_t, int32_t, int32_t))fn)(arena<void>(0x400), 10, a[0], a[1], a[2], a[3]);
                                return Result{0};
                            }};
    quick(water_range_95f910);
    // Colour::Value6B5B90: random colours, greys and colours with two parts the same
    Test colour_value_6b5b90{0x6b5b90,
                             [=](Rng &r) {
                                 uint32_t c = r.next();
                                 const int kind = r.range(0, 5);
                                 const uint32_t v = c & 0xff;
                                 if (kind == 0) c = (c & 0xff000000) | v << 16 | v << 8 | v;
                                 else if (kind == 1) c = (c & 0xff00ffff) | v << 16;
                                 else if (kind == 2) c = (c & 0xffff00ff) | v << 8;
                                 else if (kind == 3) c = (c & 0xff00ffff) | (c & 0xff00) << 8;
                                 arena<uint32_t>(0x40)[0] = c;
                             },
                             [](void *fn) {
                                 Result x;
                                 x.f = ((long double(__stdcall *)(uint32_t))fn)(arena<uint32_t>(0x40)[0]);
                                 return x;
                             }};
    quick(colour_value_6b5b90);
    // Window::Mode6A7DF0: modes 0-4, values below, at and past the ends by small steps, value missing at times
    Test window_mode_6a7df0{0x6a7df0,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                *(int32_t *)(self + 0xf0) = r.range(0, 4);
                                *(uint32_t *)(self + 0xf4) = r.range(0, 7) ? (uint32_t)(uintptr_t)arena<void>(0x40) : 0;
                                *(uint32_t *)(self + 0xdc) = (uint32_t)(uintptr_t)arena<void>(0x300);
                                *(uint32_t *)(self + 0xe0) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                *(uint32_t *)arena<uint8_t>(0x394) = r.next(), *(uint32_t *)arena<uint8_t>(0x494) = r.next();
                                const int lo = r.range(0, 10), hi = lo + r.range(0, 20), step = r.range(0, 4), v = r.range(lo - 3, hi + 3);
                                if (*(int32_t *)(self + 0xf0) == 1) {
                                    *(float *)(self + 0xf8) = (float)lo, *(float *)(self + 0xfc) = (float)hi;
                                    *(float *)(self + 0x100) = r.range(0, 1) ? (float)step : r.real(0, 4);
                                    arena<float>(0x40)[0] = r.range(0, 1) ? (float)v : r.real(lo - 3, hi + 3);
                                } else {
                                    *(int32_t *)(self + 0xf8) = lo, *(int32_t *)(self + 0xfc) = hi, *(int32_t *)(self + 0x100) = step;
                                    if (*(int32_t *)(self + 0xf0) == 3) self[0xf9] = (uint8_t)hi, self[0xfa] = (uint8_t)step;
                                    arena<int32_t>(0x40)[0] = v < 0 ? v + 3 : v;
                                }
                            },
                            [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *))fn)(arena<void>(0x100))}; }};
    quick(window_mode_6a7df0);
    // Vector::AxisBFB340: random vectors, vectors with ties in size and zero parts, the zero vector
    Test vector_axis_bfb340{0xbfb340,
                            [=](Rng &r) {
                                float *v = arena<float>(0x40);
                                const bool whole = r.range(0, 2) == 0;
                                for (int k = 0; k < 3; k++) v[k] = whole ? (float)r.range(-2, 2) : r.real(-10, 10);
                                for (int k = 0; k < 3; k++) arena<uint32_t>(0x60)[k] = r.next();
                            },
                            [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, void *))fn)(arena<void>(0x40), arena<void>(0x60))}; }};
    quick(vector_axis_bfb340);
    // Shop::Round4BB620: grids of 1-6 by 1-6 cells, some with their own heights, points in every cell and outside
    Test shop_round_4bb620{0x4bb620,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               self[4] = (uint8_t)r.range(1, 6), self[5] = (uint8_t)r.range(1, 6);
                               *(float *)(self + 0xc) = r.real(-50, 50), *(float *)(self + 0x10) = r.real(-50, 50);
                               *(float *)(self + 0x1c) = r.real(0.2f, 2), *(float *)(self + 0x20) = r.real(0.2f, 2);
                               *(uint32_t *)(self + 0x24) = (uint32_t)(uintptr_t)arena<void>(0x400);
                               for (int c = 0; c < 36; c++) {
                                   float *cell = arena<float>(0x400 + c * 24);
                                   for (int k = 0; k < 4; k++) cell[k] = r.real(-20, 20);
                                   *(uint32_t *)(cell + 5) = r.range(0, 2) ? 0 : (uint32_t)(uintptr_t)arena<void>(0x800 + c * 16);
                                   for (int k = 0; k < 4; k++) arena<float>(0x800 + c * 16)[k] = r.real(-20, 20);
                               }
                               arena<float>(0x40)[0] = *(float *)(self + 0xc) + r.real(-1, 7) / *(float *)(self + 0x1c);
                               arena<float>(0x40)[1] = *(float *)(self + 0x10) + r.real(-1, 7) / *(float *)(self + 0x20);
                           },
                           [](void *fn) {
                               Result x;
                               x.f = ((long double(__thiscall *)(const void *, float, float))fn)(arena<void>(0x100), arena<float>(0x40)[0], arena<float>(0x40)[1]);
                               return x;
                           }};
    quick(shop_round_4bb620);
    // Person::Values4FEEB0: with and without the record (and its +0x66C), values either side of the 1000 and 2000 caps,
    // counts below and above 11, flags either way
    Test person_values_4feeb0{0x4feeb0,
                              [=](Rng &r) {
                                  uint8_t *self = arena<uint8_t>(0x100), *p = arena<uint8_t>(0x400);
                                  *(uint32_t *)(self + 0xb8) = r.range(0, 7) ? (uint32_t)(uintptr_t)p : 0;
                                  *(uint32_t *)(p + 0x66c) = r.range(0, 7) ? r.next() | 1 : 0;
                                  *(float *)(p + 0x600) = r.range(0, 4) ? r.real(0, 3000) : (float)(1000 * r.range(1, 2));
                                  *(float *)(p + 0x604) = r.real(-50, 50);
                                  *(int32_t *)(p + 0x608) = r.range(-2, 15), *(int32_t *)(p + 0x60c) = r.range(-1, 1), *(int32_t *)(p + 0x610) = r.range(-1, 1);
                                  for (int k = 0; k < 3; k++) arena<uint32_t>(0x60)[k] = r.next();
                              },
                              [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, void *))fn)(arena<void>(0x100), arena<void>(0x60)) & 0xff}; }};
    quick(person_values_4feeb0);
    // Person::Values4FE8F0: the game's 0x12F5034 set (put back after); with and without the record, values either side
    // of every threshold
    {
        static uint32_t saved_scale;
        Test x{0x4fe8f0,
               [=](Rng &r) {
                   *(float *)0x12f5034 = r.range(0, 3) ? r.real(0.5f, 2) : 1.0f;
                   uint8_t *self = arena<uint8_t>(0x100), *p = arena<uint8_t>(0x400);
                   *(uint32_t *)(self + 0xb8) = r.range(0, 7) ? (uint32_t)(uintptr_t)p : 0;
                   *(uint32_t *)(p + 0x66c) = r.range(0, 7) ? r.next() | 1 : 0;
                   *(float *)(p + 0x5e0) = r.real(-20, 20);
                   *(float *)(p + 0x5e4) = r.range(0, 4) ? r.real(-30, 30) : -2.5f / 0.3f;
                   *(float *)(p + 0x5e8) = r.range(0, 4) ? r.real(-2, 12) : (float)r.range(3, 8);
                   for (int k = 0; k < 3; k++) arena<uint32_t>(0x60)[k] = r.next();
               },
               [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, void *))fn)(arena<void>(0x100), arena<void>(0x60)) & 0xff}; }};
        x.prepare = [](const rewrite::Entry *) { saved_scale = *(uint32_t *)0x12f5034; };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x12f5034 = saved_scale; };
        quick(x);
    }
    // Terrain::Scale8F2480: grids of 0-5 by 0-5 steps, random origins and sizes
    Test terrain_scale_8f2480{0x8f2480,
                              [=](Rng &r) {
                                  uint8_t *self = arena<uint8_t>(0x100);
                                  *(int32_t *)(self + 8) = r.range(0, 5), *(int32_t *)(self + 0xc) = r.range(0, 5);
                                  *(uint32_t *)(self + 0x14) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                  for (int k = 0; k < 80; k++) arena<uint32_t>(0x400)[k] = r.next();
                                  arena<float>(0x40)[0] = r.real(-100, 100), arena<float>(0x40)[1] = r.real(-100, 100);
                                  arena<float>(0x48)[0] = r.real(1, 300), arena<float>(0x48)[1] = r.real(1, 300);
                              },
                              [](void *fn) {
                                  ((void(__thiscall *)(const void *, const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40), arena<void>(0x48));
                                  return Result{0};
                              }};
    quick(terrain_scale_8f2480);
    // Person::Values4FEDF0: with and without the record (and its +0x66C), counts either side of 9 and 12
    Test person_values_4fedf0{0x4fedf0,
                              [=](Rng &r) {
                                  uint8_t *self = arena<uint8_t>(0x100), *p = arena<uint8_t>(0x400);
                                  *(uint32_t *)(self + 0xb8) = r.range(0, 7) ? (uint32_t)(uintptr_t)p : 0;
                                  *(uint32_t *)(p + 0x66c) = r.range(0, 7) ? r.next() | 1 : 0;
                                  *(int32_t *)(p + 0x5f0) = r.range(-2, 16), *(float *)(p + 0x5f4) = r.real(-20, 20);
                                  for (int k = 0; k < 3; k++) arena<uint32_t>(0x60)[k] = r.next();
                              },
                              [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, void *))fn)(arena<void>(0x100), arena<void>(0x60)) & 0xff}; }};
    quick(person_values_4fedf0);
    // Person::Values500780: with and without +0xB8, lists of 0-5 entries with counts 0-5 and f either side of 20 and far
    // past it
    Test person_values_500780{0x500780,
                              [=](Rng &r) {
                                  uint8_t *self = arena<uint8_t>(0x100);
                                  *(uint32_t *)(self + 0xb8) = r.range(0, 7) ? r.next() | 1 : 0;
                                  const int n = r.range(0, 5);
                                  for (int k = 0; k < 5; k++) {
                                      uint8_t *item = arena<uint8_t>(0x600 + k * 0x40);
                                      *(uint32_t *)item = (uint32_t)(uintptr_t)arena<void>(0x800 + k * 0x40);
                                      *(int32_t *)arena<uint8_t>(0x83c + k * 0x40) = r.range(-5, 500);
                                      *(int32_t *)(item + 4) = r.range(0, 5);
                                      *(float *)(item + 8) = r.range(0, 3) ? r.real(0, 60) : 20.0f;
                                  }
                                  *(uint32_t *)(self + 0x180) = node_list_h2(r, 0x400, n, 0x600);
                                  for (int k = 0; k < 3; k++) arena<uint32_t>(0x60)[k] = r.next();
                              },
                              [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, void *))fn)(arena<void>(0x100), arena<void>(0x60)) & 0xff}; }};
    quick(person_values_500780);
    // Person::Scale4FF2F0: y below, on and between every level (10, 11, 12, 13.2, 14.5, 20)
    Test person_scale_4ff2f0{0x4ff2f0,
                             [=](Rng &r) {
                                 static const float levels[] = {10, 11, 12, 13.2f, 14.5f, 20};
                                 arena<float>(0x40)[0] = r.real(-100, 100);
                                 arena<float>(0x40)[1] = r.range(0, 2) ? r.real(5, 25) : levels[r.range(0, 5)];
                             },
                             [](void *fn) {
                                 Result x;
                                 x.f = ((long double(__stdcall *)(float, float))fn)(arena<float>(0x40)[0], arena<float>(0x40)[1]);
                                 return x;
                             }};
    quick(person_scale_4ff2f0);
    // Ride::Compare59A1E0: small whole values (every tie and order), c below, at and above 0
    Test ride_compare_59a1e0{0x59a1e0,
                             [=](Rng &r) {
                                 for (int k = 0; k < 4; k++) arena<float>(0x40)[k] = (float)r.range(-2, 2);
                             },
                             [](void *fn) {
                                 const float *a = arena<float>(0x40);
                                 return Result{((uint32_t(__stdcall *)(float, float, float, float))fn)(a[0], a[1], a[2], a[3]) & 0xff};
                             }};
    quick(ride_compare_59a1e0);
    // Terrain::Range8F47F0: grids of 1-5 by 1-5 cells of random corner heights
    Test terrain_range_8f47f0{0x8f47f0,
                              [=](Rng &r) {
                                  *(uint32_t *)arena<uint8_t>(0x110) = (uint32_t)(uintptr_t)arena<void>(0x1000);
                                  uint8_t *g = arena<uint8_t>(0x200);
                                  g[4] = (uint8_t)r.range(1, 5), g[5] = (uint8_t)r.range(1, 5);
                                  *(uint32_t *)(g + 0x24) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                  for (int k = 0; k < 25 * 6; k++) arena<float>(0x400)[k] = r.real(-20, 20);
                                  for (int k = 0; k < 24; k++) arena<uint32_t>(0x1000)[k] = r.next();
                              },
                              [](void *fn) {
                                  ((void(__thiscall *)(const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x200));
                                  return Result{0};
                              }};
    quick(terrain_range_8f47f0);
    // Fireworks::Count99C360: the game's slot length set (put back after); 0-4 other shows in slot n or not, of other
    // groups, the same group or unset, overlapping or not, going either way
    {
        static uint32_t saved_slot;
        Test x{0x99c360,
               [=](Rng &r) {
                   *(float *)0x13413b0 = (float)r.range(5, 20);
                   uint8_t *self = arena<uint8_t>(0x100);
                   const int m = r.range(0, 4);
                   *(int32_t *)(self + 0xec) = m, *(uint32_t *)(self + 0xe8) = (uint32_t)(uintptr_t)arena<void>(0x180);
                   const int n = r.range(0, 3);
                   for (int k = 0; k < 5; k++) {
                       uint8_t *entry = arena<uint8_t>(0x200 + k * 0x10), *obj = arena<uint8_t>(0x400 + k * 0x200), *tm = arena<uint8_t>(0x1600 + k * 0x100);
                       arena<uint32_t>(0x180)[k] = (uint32_t)(uintptr_t)entry;
                       *(uint32_t *)(entry + 4) = (uint32_t)(uintptr_t)obj;
                       *(uint32_t *)(obj + 0x18c) = (uint32_t)(uintptr_t)tm;
                       *(int32_t *)(obj + 0x194) = r.range(-1, 3);
                       const float start = (float)r.range(0, 60), len = (float)r.range(1, 20);
                       *(float *)(tm + 0x7c) = start, *(float *)(tm + 0x84) = start + len;
                       const int slot = r.range(0, 3) ? n : r.range(0, 3);
                       *(float *)(tm + 0x80) = (slot + r.real(0, 0.99f)) * *(float *)0x13413b0;
                   }
                   // the show: the last record (+4 its object)
                   *(uint32_t *)arena<uint8_t>(0x284) = (uint32_t)(uintptr_t)arena<void>(0xe00);
                   *(uint32_t *)arena<uint8_t>(0xf8c) = (uint32_t)(uintptr_t)arena<void>(0x1b00);
                   *(int32_t *)arena<uint8_t>(0xf94) = r.range(0, 3);
                   const float start = (float)r.range(0, 60);
                   *(float *)arena<uint8_t>(0x1b7c) = start, *(float *)arena<uint8_t>(0x1b84) = start + (float)r.range(1, 20);
                   arena<int32_t>(0x40)[0] = n, arena<float>(0x40)[1] = r.range(0, 1) ? (float)r.range(0, 70) : r.real(0, 70);
                   arena<uint32_t>(0x60)[0] = r.next();
               },
               [](void *fn) {
                   return Result{((uint32_t(__thiscall *)(const void *, int32_t, float, const void *, void *))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<float>(0x40)[1], arena<void>(0x280), arena<void>(0x60)) & 0xff};
               }};
        x.prepare = [](const rewrite::Entry *) { saved_slot = *(uint32_t *)0x13413b0; };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x13413b0 = saved_slot; };
        quick(x);
    }
    // Fireworks::Height9E0B90: the game's box and margin set (put back after); points inside, past each side and on it,
    // height kept or not
    {
        static uint32_t saved_margin, saved_box[3];
        Test x{0x9e0b90,
               [=](Rng &r) {
                   *(float *)0x10f0e94 = r.real(0, 5);
                   for (int k = 0; k < 3; k++) ((float *)0x14002c0)[k] = r.real(-50, 50);
                   uint8_t *self = arena<uint8_t>(0x100);
                   for (int k = 0; k < 3; k++) *(float *)(self + 0x10 + k * 4) = r.real(5, 40);
                   *(int32_t *)(self + 0x44) = r.range(0, 9);
                   for (int k = 0; k < 3; k++) {
                       const float mid = ((float *)0x14002c0)[k], h = *(float *)(self + 0x10 + k * 4) + (k == 1 ? 0 : *(float *)0x10f0e94);
                       switch (r.range(0, 3)) {
                       case 0: arena<float>(0x40)[k] = mid + r.real(-h, h); break;
                       case 1: arena<float>(0x40)[k] = mid + h + r.real(0, 10); break;
                       case 2: arena<float>(0x40)[k] = mid - h - r.real(0, 10); break;
                       default: arena<float>(0x40)[k] = mid + (r.range(0, 1) ? h : -h); break;
                       }
                   }
                   arena<uint32_t>(0x50)[0] = r.range(0, 1) | (r.next() & 0xff00);
               },
               [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x40), arena<uint32_t>(0x50)[0]) & 0xff}; }};
        x.prepare = [](const rewrite::Entry *) { saved_margin = *(uint32_t *)0x10f0e94, memcpy(saved_box, (void *)0x14002c0, 12); };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x10f0e94 = saved_margin, memcpy((void *)0x14002c0, saved_box, 12); };
        quick(x);
    }
    // Water::Update8FEAE0: on or off, modes 3-9, boxes of 0.3-40 wide and deep, 2-6 points around the middle
    Test water_update_8feae0{0x8feae0,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100);
                                 float *f = (float *)self;
                                 self[0x2c] = r.range(0, 5) != 0;
                                 *(int32_t *)(self + 0x34) = r.range(3, 9);
                                 const int n = r.range(2, 6);
                                 *(int32_t *)(self + 8) = n;
                                 *(uint32_t *)self = (uint32_t)(uintptr_t)arena<void>(0x400);
                                 f[4] = r.real(-50, 50), f[5] = r.real(-50, 50);
                                 f[6] = f[4] + (r.range(0, 1) ? r.real(0.3f, 2) : r.real(2, 40)), f[7] = f[5] + (r.range(0, 1) ? r.real(0.3f, 2) : r.real(2, 40));
                                 f[8] = r.real(0, 500), f[3] = r.real(0, 30);
                                 for (int k = 0; k < 6 * 3; k++) arena<float>(0x400)[k] = r.real(-60, 60);
                                 for (int k = 0x44; k < 0x54; k += 4) *(uint32_t *)(self + k) = r.next();
                             },
                             [](void *fn) {
                                 ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                                 return Result{0};
                             }};
    quick(water_update_8feae0);
    // Animal::Pass414470: 0-6 links between 5 points (shared points, some fixed), lengths shorter and longer than the gaps
    Test animal_pass_414470{0x414470,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                const int n = r.range(0, 6);
                                *(int32_t *)(self + 0x84) = n, *(uint32_t *)(self + 0x80) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                for (int k = 0; k < 5; k++) {
                                    float *p = arena<float>(0x400 + k * 0x50);
                                    for (int c = 0; c < 3; c++) p[c] = r.real(-10, 10);
                                    p[0x40 / 4] = r.range(0, 3) ? r.real(0.1f, 2) : 0.0f;
                                }
                                for (int i = 0; i < n; i++) {
                                    uint32_t *e = arena<uint32_t>(0x200 + i * 12);
                                    const int a = r.range(0, 4), b = (a + r.range(1, 4)) % 5;
                                    e[0] = (uint32_t)(uintptr_t)arena<void>(0x400 + a * 0x50), e[1] = (uint32_t)(uintptr_t)arena<void>(0x400 + b * 0x50);
                                    ((float *)e)[2] = r.real(0.5f, 20);
                                }
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(const void *))fn)(arena<void>(0x100));
                                return Result{0};
                            }};
    quick(animal_pass_414470);
    // Scenery::Flat8B5F60: targets near and far (straight up at times), gravities, launch values, limits, time wanted or
    // not (target in ecx, out in edx)
    Test scenery_flat_8b5f60{0x8b5f60,
                             [=](Rng &r) {
                                 float *t = arena<float>(0x40);
                                 t[0] = r.range(0, 7) ? r.real(-20, 20) : 0.0f, t[1] = r.real(-10, 10), t[2] = r.range(0, 7) ? r.real(-20, 20) : 0.0f;
                                 float *p = arena<float>(0x80);
                                 p[0] = r.real(0.5f, 10), p[1] = r.real(-1, 1), p[2] = r.real(10, 300), p[3] = r.real(0, 30);
                                 arena<uint32_t>(0x90)[0] = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x70) : 0;
                                 for (int k = 0; k < 4; k++) arena<uint32_t>(0x60)[k] = r.next();
                             },
                             [](void *fn) {
                                 const float *p = arena<float>(0x80);
                                 return Result{((uint32_t(__fastcall *)(const void *, void *, float, float, float, float, void *, uint32_t))fn)(arena<void>(0x40), arena<void>(0x60), p[0], p[1], p[2], p[3], (void *)(uintptr_t)arena<uint32_t>(0x90)[0], 0) & 0xff};
                             }};
    quick(scenery_flat_8b5f60);
    // Animal::ValueA88470: the game's 0x13CDF20 set (put back after); counts 0-12 (none at times), sizes 1-6, values from
    // small to past every cap
    {
        static uint32_t saved_scale;
        Test x{0xa88470,
               [=](Rng &r) {
                   *(float *)0x13cdf20 = r.real(0.01f, 2);
                   uint8_t *self = arena<uint8_t>(0x100);
                   auto set = [&](uint32_t at, int v) { *(int32_t *)(self + at) = v; };
                   set(0x28, r.range(1, 6));
                   for (uint32_t at = 0x2c; at <= 0x3c; at += 4) set(at, r.range(0, 4) ? r.range(0, 12) : 0);
                   if (*(int32_t *)(self + 0x2c) + *(int32_t *)(self + 0x30) == 0) set(0x30, 1);
                   for (uint32_t at : {0x60u, 0x6cu, 0x70u, 0x78u, 0x7cu}) set(at, r.range(0, 20));
                   const int p = r.range(0, 10), m = *(int32_t *)(self + 0x28);
                   set(0x80, p), set(0x84, (p * p) / m + r.range(0, 30));
                   *(float *)(self + 0xc) = r.real(0, 30);
               },
               [](void *fn) {
                   Result x;
                   x.f = ((long double(__thiscall *)(const void *))fn)(arena<void>(0x100));
                   return x;
               }};
        x.prepare = [](const rewrite::Entry *) { saved_scale = *(uint32_t *)0x13cdf20; };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x13cdf20 = saved_scale; };
        quick(x);
    }
    // Water::Range941EF0: 4 x 4 grids of small whole or random heights, neighbouring cells each way (and the same cell),
    // levels across the heights
    Test water_range_941ef0{0x941ef0,
                            [=](Rng &r) {
                                uint8_t *g = arena<uint8_t>(0x100);
                                g[4] = 4, *(uint32_t *)(g + 0x24) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                const bool whole = r.range(0, 1);
                                for (int k = 0; k < 16 * 6; k++) arena<float>(0x400)[k] = whole ? (float)r.range(0, 4) : r.real(0, 4);
                                uint8_t *a = arena<uint8_t>(0x40), *b = arena<uint8_t>(0x44);
                                a[0] = (uint8_t)r.range(1, 2), a[1] = (uint8_t)r.range(1, 2);
                                b[0] = a[0], b[1] = a[1];
                                switch (r.range(0, 4)) {
                                case 0: b[0]++; break;
                                case 1: b[0]--; break;
                                case 2: b[1]++; break;
                                case 3: b[1]--; break;
                                }
                                arena<float>(0x48)[0] = whole ? (float)r.range(0, 4) + (r.range(0, 1) ? 0.5f : 0.0f) : r.real(-0.5f, 4.5f);
                            },
                            [](void *fn) { return Result{((uint32_t(__stdcall *)(float, const void *, const void *, const void *))fn)(arena<float>(0x48)[0], arena<void>(0x40), arena<void>(0x44), arena<void>(0x100)) & 0xff}; }};
    quick(water_range_941ef0);
    // Animal::PointA6ED40: 1-13 points of random values (every unrolled count and tail)
    Test animal_point_a6ed40{0xa6ed40,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100);
                                 const int n = r.range(1, 13);
                                 *(int32_t *)(self + 0x10) = n, *(uint32_t *)(self + 0xc) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                 for (int k = 0; k < 13; k++) {
                                     arena<uint32_t>(0x200)[k] = (uint32_t)(uintptr_t)arena<void>(0x400 + k * 0x30);
                                     for (int c = 4; c < 11; c++) arena<float>(0x400 + k * 0x30)[c] = r.real(-100, 100);
                                 }
                                 for (int k = 0; k < 3; k++) arena<uint32_t>(0x60)[k] = r.next();
                             },
                             [](void *fn) {
                                 return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, void *))fn)(arena<void>(0x100), arena<void>(0x60))};
                             }};
    quick(animal_point_a6ed40);
    // Model::PartsCC66B0: done or not, models of 0-5 parts with random matrices
    Test model_parts_cc66b0{0xcc66b0,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100), *model = arena<uint8_t>(0x180);
                                *(uint32_t *)self = (uint32_t)(uintptr_t)model, *(uint32_t *)(self + 4) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                self[8] = r.range(0, 4) == 0;
                                *(int32_t *)(model + 0x24) = r.range(0, 5), *(uint32_t *)(model + 0x34) = (uint32_t)(uintptr_t)arena<void>(0x800);
                                for (int k = 0; k < 5 * 16; k++) arena<float>(0x400)[k] = r.real(-3, 3), arena<float>(0x800)[k] = r.real(-3, 3);
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                                return Result{0};
                            }};
    quick(model_parts_cc66b0);
    // Model::PartsCC6420: done or not, models of 0-5 parts with random matrices
    Test model_parts_cc6420{0xcc6420,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100), *model = arena<uint8_t>(0x180);
                                *(uint32_t *)self = (uint32_t)(uintptr_t)model, *(uint32_t *)(self + 4) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                self[8] = r.range(0, 3) == 0;
                                *(int32_t *)(model + 0x24) = r.range(0, 5), *(uint32_t *)(model + 0x34) = (uint32_t)(uintptr_t)arena<void>(0x800);
                                for (int k = 0; k < 5 * 16; k++) arena<float>(0x400)[k] = r.real(-3, 3), arena<float>(0x800)[k] = r.real(-3, 3), arena<uint32_t>(0xc00)[k] = r.next();
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(const void *, void *))fn)(arena<void>(0x100), arena<void>(0xc00));
                                return Result{0};
                            }};
    quick(model_parts_cc6420);
    // Terrain::Height8BC4A0: grids of 1-6 by 1-6 cells of random or flat corners, points in every cell and outside
    Test terrain_height_8bc4a0{0x8bc4a0,
                               [=](Rng &r) {
                                   uint8_t *self = arena<uint8_t>(0x100);
                                   self[4] = (uint8_t)r.range(1, 6), self[5] = (uint8_t)r.range(1, 6);
                                   *(float *)(self + 0xc) = r.real(-50, 50), *(float *)(self + 0x10) = r.real(-50, 50);
                                   *(float *)(self + 0x14) = r.real(0.5f, 8), *(float *)(self + 0x18) = r.real(0.5f, 8);
                                   *(uint32_t *)(self + 0x24) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                   const bool flat = r.range(0, 5) == 0;
                                   for (int k = 0; k < 36 * 6; k++) arena<float>(0x400)[k] = flat ? 2.0f : r.real(-20, 20);
                                   arena<float>(0x40)[0] = *(float *)(self + 0xc) + r.real(-1, 7) * *(float *)(self + 0x14);
                                   arena<float>(0x40)[1] = *(float *)(self + 0x10) + r.real(-1, 7) * *(float *)(self + 0x18);
                                   for (int k = 0; k < 3; k++) arena<uint32_t>(0x60)[k] = r.next();
                               },
                               [](void *fn) {
                                   ((void(__thiscall *)(const void *, float, float, void *))fn)(arena<void>(0x100), arena<float>(0x40)[0], arena<float>(0x40)[1], arena<void>(0x60));
                                   return Result{0};
                               }};
    quick(terrain_height_8bc4a0);
    // Vector::Toward466660: random eyes and targets (the same point at times), random ups (along the view at times) (out in
    // ecx, eye in edx)
    Test vector_toward_466660{0x466660,
                              [=](Rng &r) {
                                  float *eye = arena<float>(0x40), *target = arena<float>(0x50), *up = arena<float>(0x60);
                                  for (int k = 0; k < 3; k++) eye[k] = r.real(-50, 50);
                                  for (int k = 0; k < 3; k++) target[k] = r.range(0, 7) ? r.real(-50, 50) : eye[k];
                                  for (int k = 0; k < 3; k++) up[k] = r.real(-1, 1);
                                  if (r.range(0, 7) == 0)
                                      for (int k = 0; k < 3; k++) up[k] = target[k] - eye[k];
                                  for (int k = 0; k < 16; k++) arena<uint32_t>(0x100)[k] = r.next();
                              },
                              [](void *fn) {
                                  ((void(__fastcall *)(void *, const void *, const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40), arena<void>(0x50), arena<void>(0x60));
                                  return Result{0};
                              }};
    quick(vector_toward_466660);
    // Particles::TimeCE1720: the game's two shape lists filled (all put back after) with 1-8 points, either list first;
    // planes through, beside and clear of the shape (data compared)
    {
        static uint8_t saved[0x2a0];
        Test x{0xce1720,
               [=](Rng &r) {
                   uint32_t *g = (uint32_t *)0x1400810;
                   const bool first = r.range(0, 1);
                   g[0] = first ? 0x1400830 : 0x1400970, g[1] = first ? 0x1400970 : 0x1400830;
                   const int n = r.range(1, 8);
                   g[3] = n;
                   for (int k = 0; k < 0x140 / 4; k++) ((uint32_t *)0x1400830)[k] = r.next(), ((uint32_t *)0x1400970)[k] = r.next();
                   for (int k = 0; k < n; k++)
                       for (int c = 0; c < 3; c++) ((float *)(uintptr_t)g[0])[k * 4 + c] = r.range(0, 3) ? r.real(-10, 10) : (float)r.range(-2, 2);
                   float *pl = arena<float>(0x40);
                   for (int c = 0; c < 3; c++) pl[c] = r.real(-1, 1);
                   arena<float>(0x50)[0] = r.real(-12, 12);
               },
               [](void *fn) {
                   ((void(__thiscall *)(const void *, float))fn)(arena<void>(0x40), arena<float>(0x50)[0]);
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { memcpy(saved, (void *)0x1400810, sizeof saved); };
        x.finish = [](const rewrite::Entry *) { memcpy((void *)0x1400810, saved, sizeof saved); };
        t.push_back(x);
    }
    // Model::BuildBC9C70: the game's SSE switch set either way (put back after); 1-3 groups of 0-4 vertices with random
    // matrices, vertices and uv offsets
    {
        static uint8_t saved_switch;
        Test x{0xbc9c70,
               [=](Rng &r) {
                   *(uint8_t *)0x13d0b14 = r.range(0, 1);
                   uint8_t *mesh = arena<uint8_t>(0x100);
                   const int groups = r.range(1, 3);
                   *(int32_t *)mesh = groups;
                   *(uint32_t *)(mesh + 8) = (uint32_t)(uintptr_t)arena<void>(0x180);
                   *(uint32_t *)(mesh + 0x18) = (uint32_t)(uintptr_t)arena<void>(0x400);
                   for (int g = 0; g < 3; g++) arena<int32_t>(0x180)[g] = r.range(0, 4);
                   for (int k = 0; k < 12 * 8; k++) arena<float>(0x400)[k] = r.real(-5, 5), arena<uint32_t>(0x800)[k] = r.next();
                   for (int k = 0; k < 3 * 16; k++) arena<float>(0xc00)[k] = r.real(-3, 3);
                   arena<float>(0x40)[0] = r.real(-1, 1), arena<float>(0x40)[1] = r.real(-1, 1);
               },
               [](void *fn) {
                   ((void (*)(const void *, void *, const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x800), arena<void>(0xc00), arena<void>(0x40));
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { saved_switch = *(uint8_t *)0x13d0b14; };
        x.finish = [](const rewrite::Entry *) { *(uint8_t *)0x13d0b14 = saved_switch; };
        quick(x);
    }
    // Water::Colours908940: grids of 0-5 points a side over random boxes, random or flat heights (flat with a zero scale)
    Test water_colours_908940{0x908940,
                              [=](Rng &r) {
                                  uint8_t *self = arena<uint8_t>(0x100);
                                  float *f = (float *)self;
                                  *(int32_t *)self = r.range(0, 5);
                                  f[2] = r.real(-50, 0), f[3] = r.real(-50, 0), f[4] = f[2] + r.real(1, 40), f[5] = f[3] + r.real(1, 40);
                                  f[6] = r.real(-5, 5), f[9] = r.real(0, 2);
                                  const bool flat = r.range(0, 5) == 0;
                                  f[11] = flat ? 0.0f : r.real(0.2f, 3);
                                  *(uint32_t *)(self + 0x40) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                  *(uint32_t *)(self + 0x58) = (uint32_t)(uintptr_t)arena<void>(0x380);
                                  for (int k = 0; k < 25; k++) arena<float>(0x400)[k] = flat ? 1.0f : r.real(-1, 3);
                                  for (int k = 0; k < 9; k++) arena<uint32_t>(0x380)[k] = r.next();
                                  for (int k = 0; k < 25 * 9; k++) arena<uint32_t>(0x600)[k] = r.next();
                              },
                              [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, void *))fn)(arena<void>(0x100), arena<void>(0x600))}; }};
    quick(water_colours_908940);
    // Grid::GrowChanged: grids of 0-12, changed areas any way round, rectangles inside, across the edges and empty
    Test grid_grow_changed{0x703170,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               *(int32_t *)(self + 0x6c) = r.range(0, 12), *(int32_t *)(self + 0x70) = r.range(0, 12);
                               for (int k = 0; k < 4; k++) *(int32_t *)(self + 0x74 + k * 4) = r.range(-2, 14);
                               for (int k = 0; k < 4; k++) arena<int32_t>(0x40)[k] = r.range(-3, 15);
                           },
                           [](void *fn) {
                               const int32_t *a = arena<int32_t>(0x40);
                               ((void(__thiscall *)(void *, int32_t, int32_t, int32_t, int32_t))fn)(arena<void>(0x100), a[0], a[1], a[2], a[3]);
                               return Result{0};
                           }};
    quick(grid_grow_changed);
    // Key::Matches: entries of every kind, keys of kinds 0-5 with the bytes and dword the same or not, +0xA either sign
    Test key_matches{0x7e5050,
                     [=](Rng &r) {
                         uint8_t *e = arena<uint8_t>(0x100), *k = arena<uint8_t>(0x140);
                         for (int i = 0; i < 16; i++) e[i] = (uint8_t)r.range(0, 3), k[i] = (uint8_t)r.range(0, 3);
                         e[0xf] = (uint8_t)r.next();
                         k[8] = (uint8_t)r.range(0, 5);
                         if (r.range(0, 1)) k[4] = e[0], k[5] = e[1], k[6] = e[2], k[7] = e[3];
                         k[0xa] = r.range(0, 3) ? e[0xe] : (uint8_t)(r.range(0, 1) ? 0x80 | r.next() : r.range(0, 3));
                     },
                     [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x140)) & 0xff}; }};
    quick(key_matches);
    // Window::Match9DAC40: four parts or missing ones with values 0-3, a and b 0-3 (3 matching any)
    Test window_match_9dac40{0x9dac40,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100);
                                 for (int k = 0; k < 4; k++) {
                                     *(uint32_t *)(self + 0x18 + k * 4) = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x200 + k * 0x60) : 0;
                                     *(int32_t *)arena<uint8_t>(0x254 + k * 0x60) = r.range(0, 3), *(int32_t *)arena<uint8_t>(0x258 + k * 0x60) = r.range(0, 3);
                                 }
                                 arena<int32_t>(0x40)[0] = r.range(0, 3), arena<int32_t>(0x40)[1] = r.range(0, 3);
                             },
                             [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, int32_t, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<int32_t>(0x40)[1]) & 0xff}; }};
    quick(window_match_9dac40);
    // Window::Value6F13C0: every flag mix, options on and off, counts with more or fewer on than counted
    Test window_value_6f13c0{0x6f13c0,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100);
                                 *(uint32_t *)(self + 0xd0) = r.next() & 0x69c;
                                 self[0x10c] = r.range(0, 1) ? (uint8_t)r.next() : 0, self[0xe4] = r.range(0, 1) ? (uint8_t)r.next() : 0, self[0xf0] = r.range(0, 1) ? (uint8_t)r.next() : 0;
                                 *(int32_t *)(self + 0xf4) = r.range(0, 5), *(int32_t *)(self + 0x100) = r.range(0, 7);
                                 *(int32_t *)(self + 0x120) = r.range(0, 5), *(int32_t *)(self + 0x12c) = r.range(0, 7);
                                 const int n = r.range(1, 3);
                                 *(int32_t *)(self + 0x118) = n, *(uint32_t *)(self + 0x114) = (uint32_t)(uintptr_t)arena<void>(0x300);
                                 *(uint32_t *)arena<uint8_t>(0x300 + n * 20 - 0x10) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                 *(uint32_t *)arena<uint8_t>(0x40c) = r.range(0, 1) ? r.next() : 0;
                             },
                             [](void *fn) {
                                 Result x;
                                 x.f = ((long double(__thiscall *)(const void *))fn)(arena<void>(0x100));
                                 return x;
                             }};
    quick(window_value_6f13c0);
    // Fireworks::List9F4C80: lists of 0-5 pieces with random places and directions
    Test fireworks_list_9f4c80{0x9f4c80,
                               [=](Rng &r) {
                                   for (int k = 0; k < 5; k++)
                                       for (int c = 4; c < 15; c++) arena<float>(0x400 + k * 0x40)[c] = r.real(-10, 10);
                                   *(uint32_t *)arena<uint8_t>(0x14c) = node_list_h2(r, 0x200, r.range(0, 5), 0x400);
                               },
                               [](void *fn) {
                                   ((void(__thiscall *)(const void *))fn)(arena<void>(0x100));
                                   return Result{0};
                               }};
    quick(fireworks_list_9f4c80);
    // Window::Value6CC710: 0-3 rows with their parts there or not, random sizes
    Test window_value_6cc710{0x6cc710,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100);
                                 const int n = r.range(0, 3);
                                 *(int32_t *)(self + 0xe4) = r.range(0, 7) ? n : -1;
                                 *(uint32_t *)(self + 0xe0) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                 for (int k = 0; k < 18; k++) {
                                     float *p = arena<float>(0x400 + k * 0x90);
                                     p[0x80 / 4] = r.real(0, 20), p[0x88 / 4] = r.real(0, 40);
                                 }
                                 for (int i = 0; i < 3; i++)
                                     for (int k = 0; k < 20; k++) arena<uint32_t>(0x200 + i * 0x50)[k] = (k < 3 || r.range(0, 2)) ? (uint32_t)(uintptr_t)arena<void>(0x400 + r.range(0, 17) * 0x90) : 0;
                             },
                             [](void *fn) {
                                 Result x;
                                 x.f = ((long double(__thiscall *)(const void *))fn)(arena<void>(0x100));
                                 return x;
                             }};
    quick(window_value_6cc710);
    // Track::StepBC5090: both kinds of ride, 0-5 entries of small whole ranges (t, u on the edges at times), e below 0 or
    // not
    Test track_step_bc5090{0xbc5090,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               *(uint32_t *)(self + 0x18) = r.range(0, 1) ? 0x400000 : 0, *(int32_t *)(self + 0xe4) = r.range(0, 3);
                               *(int32_t *)(self + 0x70) = r.range(0, 5), *(uint32_t *)(self + 0x74) = (uint32_t)(uintptr_t)arena<void>(0x200);
                               for (int k = 0; k < 25; k++) arena<float>(0x200)[k] = (float)r.range(-1, 6);
                               arena<float>(0x40)[0] = r.range(0, 1) ? (float)r.range(0, 6) : r.real(0, 6), arena<float>(0x40)[1] = (float)r.range(-1, 6);
                               for (int k = 0; k < 3; k++) arena<uint32_t>(0x60)[k] = r.next();
                           },
                           [](void *fn) {
                               return Result{((uint32_t(__thiscall *)(const void *, float, float, void *, void *, void *))fn)(arena<void>(0x100), arena<float>(0x40)[0], arena<float>(0x40)[1], arena<void>(0x60), arena<void>(0x64), arena<void>(0x68)) & 0xff};
                           }};
    quick(track_step_bc5090);
    // Window::Count6CF4B0: rings of 1-12 slots, any start, 0-12 values (wrapping), small whole values (ties)
    Test window_count_6cf4b0{0x6cf4b0,
                             [=](Rng &r) {
                                 int32_t *ring = arena<int32_t>(0x100);
                                 const int size = r.range(1, 12);
                                 ring[0] = (int32_t)(uintptr_t)arena<void>(0x200), ring[1] = r.range(0, size - 1), ring[2] = size, ring[3] = r.range(0, size);
                                 for (int k = 0; k < 12; k++) arena<float>(0x200)[k] = r.range(0, 1) ? (float)r.range(-3, 3) : r.real(-2e6f, 2e6f);
                                 arena<uint32_t>(0x40)[0] = r.next(), arena<uint32_t>(0x40)[1] = r.next();
                             },
                             [](void *fn) {
                                 ((void(__stdcall *)(const void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x40), arena<void>(0x44));
                                 return Result{0};
                             }};
    quick(window_count_6cf4b0);
    // Fence::CountA90B50: shapes of 0-7 corners on small whole places (corners on the point's line often), points anywhere
    Test fence_count_a90b50{0xa90b50,
                            [=](Rng &r) {
                                const int n = r.range(0, 7);
                                arena<uint32_t>(0x100)[0] = (uint32_t)(uintptr_t)arena<void>(0x200), arena<uint32_t>(0x100)[1] = n;
                                const bool whole = r.range(0, 2) != 0;
                                for (int k = 0; k < 16; k++) arena<float>(0x200)[k] = whole ? (float)r.range(-3, 3) : r.real(-3, 3);
                                arena<float>(0x40)[0] = whole ? (float)r.range(-3, 3) + (r.range(0, 1) ? 0.5f : 0.0f) : r.real(-3, 3);
                                arena<float>(0x40)[1] = r.real(-3, 3);
                                arena<float>(0x40)[2] = whole ? (float)r.range(-3, 3) : r.real(-3, 3);
                            },
                            [](void *fn) { return Result{((uint32_t(__stdcall *)(const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40))}; }};
    quick(fence_count_a90b50);
    // Window::Value67CDD0: 0-7 rising key times in 0..1 (whole steps at times), looping or not, t before, on, between and
    // past the keys (and outside 0..1)
    Test window_value_67cdd0{0x67cdd0,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100);
                                 const int n = r.range(0, 7);
                                 *(int32_t *)(self + 0xc) = n, self[0x10] = r.range(0, 1) ? (uint8_t)r.range(1, 255) : 0;
                                 *(uint32_t *)self = (uint32_t)(uintptr_t)arena<void>(0x200), *(uint32_t *)(self + 8) = (uint32_t)(uintptr_t)arena<void>(0x300);
                                 float tm = r.real(0, 0.2f);
                                 for (int i = 0; i < 8; i++) arena<float>(0x200)[i] = tm, tm += r.real(0.01f, 0.15f), arena<float>(0x300)[i] = r.real(-10, 10);
                                 arena<float>(0x40)[0] = (n && r.range(0, 3) == 0) ? arena<float>(0x200)[r.range(0, n - 1)] : r.real(-1.5f, 2.5f);
                             },
                             [](void *fn) {
                                 Result x;
                                 x.f = ((long double(__thiscall *)(const void *, float))fn)(arena<void>(0x100), arena<float>(0x40)[0]);
                                 return x;
                             }};
    quick(window_value_67cdd0);
    // Water::Cell908060: grids 1-6 wide (rows to width + 1), every flag mix, rows -2..width + 1, column ranges across the
    // edges
    Test water_cell_908060{0x908060,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               const int w = r.range(1, 6);
                               *(int32_t *)self = w, *(uint32_t *)(self + 0x20) = r.next() & 0xc, *(float *)(self + 0x38) = r.real(0, 2);
                               *(uint32_t *)(self + 0x44) = (uint32_t)(uintptr_t)arena<void>(0x400);
                               for (int k = 0; k < 8 * 8; k++) arena<float>(0x400)[k] = r.real(-10, 10);
                               int32_t *a = arena<int32_t>(0x40);
                               a[0] = r.range(-2, w + 1), a[1] = a[0] + r.range(-1, 5), a[2] = r.range(-2, w + 1);
                               arena<float>(0x40)[3] = r.real(-3, 3);
                           },
                           [](void *fn) {
                               const int32_t *a = arena<int32_t>(0x40);
                               ((void(__thiscall *)(const void *, int32_t, int32_t, int32_t, float))fn)(arena<void>(0x100), a[0], a[1], a[2], arena<float>(0x40)[3]);
                               return Result{0};
                           }};
    quick(water_cell_908060);
    // Window::Scroll9C1FD0: ten parts of small whole or random sizes, negative at times
    Test window_scroll_9c1fd0{0x9c1fd0,
                              [=](Rng &r) {
                                  uint8_t *self = arena<uint8_t>(0x100);
                                  for (int k = 0; k < 10; k++) {
                                      *(uint32_t *)(self + 0x130 + k * 4) = (uint32_t)(uintptr_t)arena<void>(0x400 + k * 0x90);
                                      float *p = arena<float>(0x400 + k * 0x90);
                                      p[0x84 / 4] = r.range(0, 1) ? (float)r.range(-3, 5) : r.real(-5, 50), p[0x88 / 4] = r.range(0, 1) ? (float)r.range(-3, 5) : r.real(-5, 50);
                                  }
                                  arena<uint32_t>(0x40)[0] = r.next(), arena<uint32_t>(0x40)[1] = r.next();
                              },
                              [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, void *))fn)(arena<void>(0x100), arena<void>(0x40))}; }};
    quick(window_scroll_9c1fd0);
    // Scenery::Point884D20: frames of random sizes, 0-9 points around the box (on its edges at times), points anywhere
    Test scenery_point_884d20{0x884d20,
                              [=](Rng &r) {
                                  uint8_t *self = arena<uint8_t>(0x100);
                                  float *f = (float *)self;
                                  f[0x7c / 4] = r.real(-10, 10), f[0x84 / 4] = f[0x7c / 4] + r.real(20, 60);
                                  f[0x80 / 4] = r.real(-10, 10), f[0x88 / 4] = f[0x80 / 4] + r.real(20, 60);
                                  f[0xdc / 4] = r.real(5, 30), f[0xf0 / 4] = r.real(5, 30);
                                  const int n = r.range(-1, 9);
                                  *(int32_t *)(self + 0xf8) = n, *(uint32_t *)(self + 0x100) = (uint32_t)(uintptr_t)arena<void>(0x300);
                                  arena<float>(0x40)[0] = r.real(0, 40), arena<float>(0x40)[1] = r.real(0, 40);
                                  const float u = (arena<float>(0x40)[0] - 4) / ((f[0x84 / 4] - f[0x7c / 4]) - 8) * f[0xdc / 4];
                                  const float v = (arena<float>(0x40)[1] - 4) / ((f[0x88 / 4] - f[0x80 / 4]) - 8) * f[0xf0 / 4];
                                  for (int k = 0; k < 9; k++) arena<float>(0x300)[2 * k] = u + r.real(-4, 4), arena<float>(0x300)[2 * k + 1] = v + r.real(-4, 4);
                              },
                              [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40))}; }};
    quick(scenery_point_884d20);
    // Ranking::Order7C34F0: six small whole (ties) or random values, random bits around the order
    Test ranking_order_7c34f0{0x7c34f0,
                              [=](Rng &r) {
                                  *(uint32_t *)arena<uint8_t>(0x478) = r.next();
                                  const bool whole = r.range(0, 1);
                                  for (int k = 0; k < 6; k++) arena<float>(0x40)[k] = whole ? (float)r.range(-2, 2) : r.real(-10, 10);
                              },
                              [](void *fn) {
                                  ((void(__thiscall *)(void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40));
                                  return Result{0};
                              }};
    quick(ranking_order_7c34f0);
    // Camera::Projection: the game's 0x1400150 set either way (put back after); zoom 0 or not, flat or not, clip plane on
    // or off with planes in front of and behind the eye
    {
        static uint32_t saved_switch;
        Test x{0xcd75c0,
               [=](Rng &r) {
                   *(uint32_t *)0x1400150 = r.range(0, 1) ? r.next() | 1 : 0;
                   uint8_t *self = arena<uint8_t>(0x100);
                   float *f = (float *)self;
                   for (uint32_t at = 0xe0; at < 0x1a0; at += 4) f[at / 4] = r.real(-1, 1);
                   for (int k = 0; k < 3; k++) f[0x160 / 4 + k] = r.real(-20, 20);
                   f[0x188 / 4] = r.real(0.1f, 5), f[0x18c / 4] = f[0x188 / 4] + r.real(10, 500);
                   f[0x190 / 4] = r.real(0.3f, 2), f[0x194 / 4] = r.real(0.5f, 2), f[0x198 / 4] = r.real(1, 30);
                   f[0x19c / 4] = r.range(0, 2) ? 0.0f : r.real(0, 3), f[0x1a0 / 4] = r.real(0.5f, 2);
                   self[0x1a4] = (uint8_t)r.next(), self[0x1a6] = r.range(0, 1) ? 1 : 0, self[0x1a7] = r.range(0, 1) ? 1 : 0;
                   f[0x17c / 4] = r.real(-30, 30);
               },
               [](void *fn) {
                   ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { saved_switch = *(uint32_t *)0x1400150; };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x1400150 = saved_switch; };
        quick(x);
    }
    // Person::Values4FEA70: with and without the record (and its +0x66C), every flag either way, counts around each cap,
    // whole numbers of any size
    Test person_values_4fea70{0x4fea70,
                              [=](Rng &r) {
                                  uint8_t *self = arena<uint8_t>(0x100), *p = arena<uint8_t>(0x400);
                                  *(uint32_t *)(self + 0xb8) = r.range(0, 7) ? (uint32_t)(uintptr_t)p : 0;
                                  *(uint32_t *)(p + 0x66c) = r.range(0, 7) ? r.next() | 1 : 0;
                                  for (uint32_t at : {0x64cu, 0x654u, 0x644u, 0x648u, 0x650u}) *(int32_t *)(p + at) = r.range(-1, 2);
                                  for (uint32_t at : {0x65cu, 0x640u, 0x63cu, 0x638u, 0x634u, 0x5f8u}) *(int32_t *)(p + at) = r.range(-2, 16);
                                  for (uint32_t at = 0x614; at <= 0x62c; at += 4) *(int32_t *)(p + at) = r.range(0, 1) ? r.range(-50, 500) : (int32_t)r.next();
                                  for (int k = 0; k < 3; k++) arena<uint32_t>(0x60)[k] = r.next();
                              },
                              [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, void *))fn)(arena<void>(0x100), arena<void>(0x60)) & 0xff}; }};
    quick(person_values_4fea70);
    // Track::SpeedBBD650: on or off, random frames, directions inside and past the limit, pushes, places and velocities,
    // steps of 0.01-0.2
    Test track_speed_bbd650{0xbbd650,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                float *f = (float *)self;
                                *(uint32_t *)(self + 0x14) = r.range(0, 5) ? r.next() | 1 : 0;
                                f[7] = r.real(0.1f, 2), f[8] = r.real(0.5f, 20);
                                for (int k = 12; k < 15; k++) f[k] = r.real(-20, 20);
                                for (int k = 16; k < 19; k++) f[k] = r.range(0, 3) ? r.real(-10, 10) : r.real(-200, 200);
                                for (int k = 0; k < 12; k++) arena<float>(0x200)[k] = r.real(-1, 1);
                                for (int k = 0; k < 3; k++) arena<float>(0x240)[k] = r.real(-3, 3), arena<float>(0x250)[k] = r.real(-3, 3);
                                arena<float>(0x40)[0] = r.real(0.01f, 0.2f);
                                if (r.range(0, 2) == 0) {   // the place right by the target (the eased step overshoots)
                                    const float *a = arena<float>(0x200), *b = arena<float>(0x240), *c = arena<float>(0x250);
                                    const float sv = a[8] * b[0] + a[9] * b[1] + a[10] * b[2];
                                    const float g = sv > f[7] ? 1 : -f[7] < sv ? (sv + f[7]) / (2 * f[7]) : 0;
                                    const float t = (f[8] * c[0] * a[4] + f[8] * c[1] * a[5] + f[8] * c[2] * a[6]) * g;
                                    for (int k = 0; k < 3; k++) f[12 + k] = t * a[4 + k] + r.real(-0.004f, 0.004f);
                                }
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(void *, float, const void *, const void *, const void *))fn)(arena<void>(0x100), arena<float>(0x40)[0], arena<void>(0x200), arena<void>(0x240), arena<void>(0x250));
                                return Result{0};
                            }};
    quick(track_speed_bbd650);
    // Scenery::Near86F890: grids of 1-4 place cells, 2-6 by 2-6 cells, items in and near the corners (negative offsets at
    // times), random weights
    Test scenery_near_86f890{0x86f890,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100), *item = arena<uint8_t>(0x200), *a = arena<uint8_t>(0x300);
                                 const int cw = r.range(1, 4), rw = r.range(2, 6), rh = r.range(2, 6);
                                 *(int32_t *)(self + 4) = cw, *(int32_t *)(self + 8) = rw, *(int32_t *)(self + 0xc) = rh;
                                 *(uint32_t *)(self + 0x10) = (uint32_t)(uintptr_t)arena<void>(0x400);
                                 for (int k = 0; k < 36; k++) arena<float>(0x400)[k] = r.real(-5, 5);
                                 *(uint32_t *)(item + 0x9c) = (uint32_t)(uintptr_t)a;
                                 *(int32_t *)(a + 0x10) = r.range(-6, 6), *(int32_t *)(a + 0x14) = r.range(-6, 6);
                                 *(int32_t *)(item + 0xa0) = r.range(-4, cw * rw + 4), *(int32_t *)(item + 0xa4) = r.range(-4, cw * rh + 4);
                                 arena<float>(0x40)[0] = r.real(-3, 3);
                             },
                             [](void *fn) {
                                 ((void(__thiscall *)(const void *, const void *, float))fn)(arena<void>(0x100), arena<void>(0x200), arena<float>(0x40)[0]);
                                 return Result{0};
                             }};
    quick(scenery_near_86f890);
    // Points::StepA6F0A0: 1-13 points of random values (every unrolled count and tail), steps small and large
    Test points_step_a6f0a0{0xa6f0a0,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                const int n = r.range(1, 13);
                                *(int32_t *)(self + 0x10) = n, *(uint32_t *)(self + 0xc) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                for (int k = 0; k < 13; k++) {
                                    arena<uint32_t>(0x200)[k] = (uint32_t)(uintptr_t)arena<void>(0x400 + k * 0x30);
                                    for (int c = 4; c < 11; c++) arena<float>(0x400 + k * 0x30)[c] = r.real(-20, 20);
                                }
                                arena<float>(0x40)[0] = r.range(0, 1) ? r.real(0, 0.5f) : r.real(0, 2000);
                            },
                            [](void *fn) {
                                Result x;
                                x.f = ((long double(__thiscall *)(const void *, float))fn)(arena<void>(0x100), arena<float>(0x40)[0]);
                                return x;
                            }};
    quick(points_step_a6f0a0);
    // Track::Turn85CB30: quarter-turn counts -6..6 (out in ecx, n in edx)
    Test track_turn_85cb30{0x85cb30,
                           [=](Rng &r) {
                               for (int k = 0; k < 16; k++) arena<uint32_t>(0x100)[k] = r.next();
                               arena<int32_t>(0x40)[0] = r.range(-6, 6);
                           },
                           [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0])}; }};
    quick(track_turn_85cb30);
    // Window::SetPoint680B50: random rectangles and points; Window::UpdateRect stood in for (it notes the part and the
    // rectangle at arena 0x40)
    {
        Test x{0x680b50,
               [=](Rng &r) {
                   for (int k = 0; k < 4; k++) arena<float>(0x17c)[k] = r.real(-500, 500);
                   arena<float>(0x80)[0] = r.real(-500, 500), arena<float>(0x80)[1] = r.real(-500, 500);
                   arena<uint32_t>(0x1d4)[0] = r.next(), arena<uint32_t>(0x1d4)[1] = r.next();
                   arena<uint32_t>(0x40)[0] = 0;
               },
               [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, const void *))fn)(arena<void>(0x100), arena<void>(0x80))}; }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x6afaf0, (void *)h2_update_rect}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Window::Scale6ACBD0: the game's GUI area and screen set (put back after) or no area; random points (out in ecx, p in
    // edx)
    {
        static uint32_t saved_area, saved_screen;
        Test x{0x6acbd0,
               [=](Rng &r) {
                   uint8_t *area = arena<uint8_t>(0x300), *screen = arena<uint8_t>(0x400);
                   *(uint32_t *)0x12f83f0 = r.range(0, 4) ? (uint32_t)(uintptr_t)area : 0;
                   *(uint32_t *)0x12f4618 = (uint32_t)(uintptr_t)screen;
                   *(float *)(area + 0x7c) = r.real(0, 100), *(float *)(area + 0x84) = *(float *)(area + 0x7c) + r.real(100, 2000);
                   *(float *)(area + 0x80) = r.real(0, 100), *(float *)(area + 0x88) = *(float *)(area + 0x80) + r.real(100, 2000);
                   *(int32_t *)(screen + 4) = r.range(320, 2560), *(int32_t *)(screen + 8) = r.range(200, 1600);
                   arena<float>(0x40)[0] = r.real(-3000, 3000), arena<float>(0x40)[1] = r.real(-3000, 3000);
                   arena<uint32_t>(0x60)[0] = r.next(), arena<uint32_t>(0x60)[1] = r.next();
               },
               [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *, const void *))fn)(arena<void>(0x60), arena<void>(0x40))}; }};
        x.prepare = [](const rewrite::Entry *) { saved_area = *(uint32_t *)0x12f83f0, saved_screen = *(uint32_t *)0x12f4618; };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x12f83f0 = saved_area, *(uint32_t *)0x12f4618 = saved_screen; };
        quick(x);
    }
    // Car::Dot50BEA0: unit directions at every angle, the same and opposite ones (a in ecx, b in edx)
    Test car_dot_50bea0{0x50bea0,
                        [=](Rng &r) {
                            float *a = arena<float>(0x40), *b = arena<float>(0x50);
                            for (int k = 0; k < 3; k++) a[k] = r.real(-1, 1);
                            const float la = sqrtf(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
                            for (int k = 0; k < 3; k++) a[k] /= la;
                            const int kind = r.range(0, 4);
                            for (int k = 0; k < 3; k++) b[k] = kind == 0 ? a[k] : kind == 1 ? -a[k] : r.real(-1, 1);
                            const float lb = sqrtf(b[0] * b[0] + b[1] * b[1] + b[2] * b[2]);
                            for (int k = 0; k < 3; k++) b[k] /= lb;
                        },
                        [](void *fn) {
                            Result x;
                            x.f = ((long double(__fastcall *)(const void *, const void *))fn)(arena<void>(0x40), arena<void>(0x50));
                            return x;
                        }};
    quick(car_dot_50bea0);
    // Vector::RotateByAxis988070: random vectors and axes (tiny and zero ones at times)
    Test vector_rotate_by_axis_988070{0x988070,
                                      [=](Rng &r) {
                                          for (int k = 0; k < 3; k++) arena<float>(0x100)[k] = r.real(-30, 30);
                                          const int kind = r.range(0, 6);
                                          for (int k = 0; k < 3; k++) arena<float>(0x40)[k] = kind == 0 ? 0.0f : kind == 1 ? r.real(-1e-21f, 1e-21f) : r.real(-4, 4);
                                      },
                                      [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40))}; }};
    quick(vector_rotate_by_axis_988070);
    // Fireworks::Point9A9FD0: random part rectangles, offsets and objects; Window::UpdateRect stood in for (it notes the part
    // and the rectangle at arena 0x40)
    {
        Test x{0x9a9fd0,
               [=](Rng &r) {
                   for (int k = 0; k < 4; k++) arena<float>(0x17c)[k] = r.real(-500, 500);
                   arena<float>(0x214)[0] = r.real(-500, 500), arena<float>(0x214)[1] = r.real(-500, 500);
                   for (uint32_t at : {0xe8u, 0xecu, 0xf8u, 0xfcu}) *(float *)arena<uint8_t>(0x400 + at) = r.real(-500, 500);
                   arena<uint32_t>(0x40)[0] = 0;
               },
               [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, const void *))fn)(arena<uint8_t>(0x100) + 0xdc, arena<void>(0x400))}; }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x6afaf0, (void *)h2_update_rect}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Animal::RayA5EF50: random cameras (flat or not), random distances
    Test animal_ray_a5ef50{0xa5ef50,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100), *cam = arena<uint8_t>(0x200);
                               *(uint32_t *)(self + 4) = (uint32_t)(uintptr_t)cam, *(uint32_t *)(self + 8) = (uint32_t)(uintptr_t)arena<void>(0x180);
                               for (uint32_t at = 0x120; at < 0x1a0; at += 4) *(float *)(cam + at) = r.real(-1, 1);
                               for (int k = 0; k < 3; k++) *(float *)(cam + 0x160 + k * 4) = r.real(-50, 50);
                               *(float *)(cam + 0x190) = r.real(0.3f, 2), *(float *)(cam + 0x194) = r.real(0.5f, 2), *(float *)(cam + 0x198) = r.real(1, 30);
                               cam[0x1a6] = r.range(0, 1) ? 1 : 0;
                               arena<float>(0x180)[0] = r.real(-100, 100);
                               for (int k = 0; k < 3; k++) arena<uint32_t>(0x40)[k] = r.next();
                           },
                           [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, void *))fn)(arena<void>(0x100), arena<void>(0x40))}; }};
    quick(animal_ray_a5ef50);
    // Effect::Value7B06C0: random effects and arguments; Effect::Value7B05E0 stood in for (effect in esi)
    {
        Test x{0x7b06c0,
               [=](Rng &r) {
                   for (int k = 0; k < 0x10; k++) arena<float>(0x100)[k] = r.real(-20, 20);
                   arena<uint32_t>(0x40)[0] = r.next(), arena<uint32_t>(0x40)[1] = r.next();
               },
               [](void *fn) {
                   void *e = arena<void>(0x100);
                   const uint32_t a = arena<uint32_t>(0x40)[0], b = arena<uint32_t>(0x40)[1];
                   __asm__ volatile("pushl %[b]\n\tpushl %[a]\n\tcall *%[f]"
                                    : "+S"(e)
                                    : [a] "m"(a), [b] "m"(b), [f] "r"(fn)
                                    : "eax", "ecx", "edx", "memory", "cc");
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x7b05e0, (void *)h2_effect_value}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Track::TurnBC5D50: quarter-turn counts -6..6 (out in ecx, n in edx)
    Test track_turn_bc5d50{0xbc5d50,
                           [=](Rng &r) {
                               for (int k = 0; k < 12; k++) arena<uint32_t>(0x100)[k] = r.next();
                               arena<int32_t>(0x40)[0] = r.range(-6, 6);
                           },
                           [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0])}; }};
    quick(track_turn_bc5d50);
    // Window::Place6ADB60: no parent, or a chain of one to three parents, scroll flags either way, random places
    Test window_place_6adb60{0x6adb60,
                             [=](Rng &r) {
                                 const int depth = r.range(0, 3);
                                 for (int w = 0; w < 4; w++) {
                                     uint8_t *win = arena<uint8_t>(0x200 + w * 0x100);
                                     for (uint32_t at = 0x7c; at <= 0x90; at += 4) *(float *)(win + at) = r.real(-500, 500);
                                     *(uint32_t *)(win + 0x94) = r.next();
                                     *(uint32_t *)(win + 0x3c) = w < depth ? (uint32_t)(uintptr_t)arena<void>(0x300 + w * 0x100) : 0;
                                 }
                                 for (int k = 0; k < 4; k++) arena<uint32_t>(0x40)[k] = r.next();
                             },
                             [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, void *))fn)(arena<void>(0x200), arena<void>(0x40))}; }};
    quick(window_place_6adb60);
    // Water::Cell90B150: grids 1-6 wide over random boxes, places in and around them, random amounts (the move itself real)
    Test water_cell_90b150{0x90b150,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               float *f = (float *)self;
                               const int w = r.range(1, 6);
                               *(int32_t *)self = w, *(uint32_t *)(self + 0x20) = r.next() & 0xc, f[0x38 / 4] = r.real(0, 2);
                               f[2] = r.real(-50, 50), f[3] = r.real(-50, 50), f[4] = f[2] + r.real(5, 60), f[5] = f[3] + r.real(5, 60);
                               *(uint32_t *)(self + 0x44) = (uint32_t)(uintptr_t)arena<void>(0x400);
                               for (int k = 0; k < 8 * 8; k++) arena<float>(0x400)[k] = r.real(-10, 10);
                               float *a = arena<float>(0x40);
                               a[0] = f[2] + r.real(-0.2f, 1.2f) * (f[4] - f[2]), a[1] = f[2] + r.real(-0.2f, 1.2f) * (f[4] - f[2]);
                               a[2] = f[3] + r.real(-0.3f, 1.3f) * (f[5] - f[3]), a[3] = r.real(-3, 3);
                           },
                           [](void *fn) {
                               const float *a = arena<float>(0x40);
                               ((void(__thiscall *)(void *, float, float, float, float))fn)(arena<void>(0x100), a[0], a[1], a[2], a[3]);
                               return Result{0};
                           }};
    quick(water_cell_90b150);
    // Animal::StepA91180: shapes of 0-7 corners within -2..2 (the real inside test), random bits before
    Test animal_step_a91180{0xa91180,
                            [=](Rng &r) {
                                uint32_t *shape = arena<uint32_t>(0x100);
                                shape[0] = (uint32_t)(uintptr_t)arena<void>(0x200), shape[1] = r.range(0, 7);
                                for (int k = 0; k < 8; k++) shape[2 + k] = r.next();
                                for (int k = 0; k < 16; k++) arena<float>(0x200)[k] = r.range(0, 2) ? r.real(-2.2f, 2.2f) : (float)r.range(-8, 8) * 0.25f;
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x300), arena<void>(0x100));
                                return Result{0};
                            }};
    quick(animal_step_a91180);
    // Terrain::Corner8F3650: 0-3 rings of 0-4 by 0-4 steps (the real edge fill), random grids
    Test terrain_corner_8f3650{0x8f3650,
                               [=](Rng &r) {
                                   uint8_t *self = arena<uint8_t>(0x100), *g = arena<uint8_t>(0x180);
                                   *(uint32_t *)self = (uint32_t)(uintptr_t)g, *(int32_t *)(self + 4) = r.range(0, 3);
                                   *(uint32_t *)(self + 8) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                   g[4] = (uint8_t)r.range(1, 40), g[5] = (uint8_t)r.range(1, 40);
                                   *(float *)(g + 0xc) = r.real(-100, 100), *(float *)(g + 0x10) = r.real(-100, 100);
                                   *(float *)(g + 0x14) = r.real(0.5f, 8), *(float *)(g + 0x18) = r.real(0.5f, 8);
                                   for (int i = 0; i < 3; i++) {
                                       uint8_t *ring = arena<uint8_t>(0x200 + i * 0x30);
                                       *(int32_t *)(ring + 8) = r.range(0, 4), *(int32_t *)(ring + 0xc) = r.range(0, 4);
                                       *(uint32_t *)(ring + 0x14) = (uint32_t)(uintptr_t)arena<void>(0x400 + i * 0x200);
                                   }
                                   for (int k = 0; k < 3 * 0x80; k++) arena<uint32_t>(0x400)[k] = r.next();
                               },
                               [](void *fn) {
                                   ((void(__thiscall *)(const void *))fn)(arena<void>(0x100));
                                   return Result{0};
                               }};
    quick(terrain_corner_8f3650);
    // Window::Hit655360: rectangles and points in and out (by the real Rect::Contains), with or without a child, points
    // near its rounded corner
    Test window_hit_655360{0x655360,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100), *k = arena<uint8_t>(0x300);
                               float *f = (float *)self;
                               f[0x7c / 4] = r.real(0, 100), f[0x80 / 4] = r.real(0, 100);
                               f[0x84 / 4] = f[0x7c / 4] + r.real(40, 200), f[0x88 / 4] = f[0x80 / 4] + r.real(10, 60);
                               *(uint32_t *)(self + 0x100) = r.range(0, 3) ? (uint32_t)(uintptr_t)k : 0;
                               *(float *)(k + 0x7c) = r.real(0, 30), *(float *)(k + 0x84) = *(float *)(k + 0x7c) + r.real(10, 120);
                               float *p = arena<float>(0x40);
                               p[0] = f[0x7c / 4] + r.real(-10, f[0x84 / 4] - f[0x7c / 4] + 10);
                               p[1] = r.range(0, 1) ? f[0x80 / 4] + r.real(-2, 25) : f[0x80 / 4] + r.real(-10, f[0x88 / 4] - f[0x80 / 4] + 10);
                           },
                           [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40)) & 0xff}; }};
    quick(window_hit_655360);
    // Loop::StepCDF760 and Loop::StepCDF690: 0-8 triangles over 8 points of small whole or random values (ties), random
    // directions and other fields (n in ecx, list in edx; the real compares and sifts)
    for (uintptr_t at : {0xcdf760u, 0xcdf690u}) {
        Test x{at,
               [=](Rng &r) {
                   const bool whole = r.range(0, 1);
                   for (int k = 0; k < 32; k++) arena<float>(0x200)[k] = whole ? (float)r.range(-3, 3) : r.real(-10, 10);
                   for (int k = 0; k < 3; k++) arena<float>(0x40)[k] = whole ? (float)r.range(-2, 2) : r.real(-1, 1);
                   for (int t = 0; t < 8; t++) {
                       uint32_t *rec = arena<uint32_t>(0x400 + t * 0x20);
                       for (int k = 0; k < 8; k++) rec[k] = r.next();
                       for (int k = 0; k < 3; k++) rec[k] = r.range(0, 7);
                       for (int k = 4; k < 6; k++) ((float *)rec)[k] = r.real(-5, 5);
                   }
                   arena<int32_t>(0x60)[0] = r.range(0, 8);
               },
               [](void *fn) {
                   ((void(__fastcall *)(int32_t, void *, const void *, const void *))fn)(arena<int32_t>(0x60)[0], arena<void>(0x400), arena<void>(0x200), arena<void>(0x40));
                   return Result{0};
               }};
        quick(x);
    }
    // Table::StepCE2BB0: meshes of 0-6 triangles over 8 points around the point (the real ray test) (mesh in ecx, p in edx)
    Test table_step_ce2bb0{0xce2bb0,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               *(uint32_t *)(self + 0x24) = r.range(0, 6);
                               *(uint32_t *)(self + 0x28) = (uint32_t)(uintptr_t)arena<void>(0x200);
                               *(uint32_t *)(self + 0x2c) = (uint32_t)(uintptr_t)arena<void>(0x300);
                               for (int k = 0; k < 8; k++)
                                   for (int c = 0; c < 4; c++) arena<float>(0x200 + k * 16)[c] = r.real(-5, 5);
                               for (int k = 0; k < 18; k++) arena<uint16_t>(0x300)[k] = (uint16_t)r.range(0, 7);
                               for (int c = 0; c < 3; c++) arena<float>(0x40)[c] = r.real(-3, 3);
                           },
                           [](void *fn) { return Result{((uint32_t(__fastcall *)(const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40)) & 0xff}; }};
    quick(table_step_ce2bb0);
    // Water::Step91C0E0: 0-5 triangles around rays from random places in random directions (the real ray test)
    Test water_step_91c0e0{0x91c0e0,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               *(int32_t *)(self + 0x318) = r.range(0, 5), *(uint32_t *)(self + 0x310) = (uint32_t)(uintptr_t)arena<void>(0x500);
                               for (int k = 0; k < 5 * 12; k++) arena<float>(0x500)[k] = r.real(-5, 5);
                               for (int c = 0; c < 3; c++) arena<float>(0x40)[c] = r.real(-3, 3), arena<float>(0x50)[c] = r.real(-1, 1);
                               arena<uint32_t>(0x60)[0] = r.next();
                           },
                           [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *, const void *, const void *, void *))fn)(arena<void>(0x100), arena<void>(0x40), arena<void>(0x50), arena<void>(0x60)) & 0xff}; }};
    quick(water_step_91c0e0);
    // Text::FindBCEBA0 / BCEC90 / BCF260: texts and keys of 0-6 letters from a small set in both cases (equal at times,
    // one a start of the other), bytes past 0x7F now and then; tolower stood in for (the game's needs its CRT set up)
    for (uintptr_t at : {0xbceba0u, 0xbcec90u, 0xbcf260u}) {
        Test x{at,
               [=](Rng &r) {
                   static const char set[] = "aAbBzZ_1";
                   uint8_t *s = arena<uint8_t>(0x100);
                   *(uint32_t *)arena<uint8_t>(0x40) = (uint32_t)(uintptr_t)s;
                   const int tl = r.range(0, 6), kl = r.range(0, 1) ? tl - r.range(0, tl) : r.range(0, 6);
                   *(uint32_t *)(s + 4) = tl, *(uint32_t *)(s + 0xc) = (uint32_t)(uintptr_t)arena<void>(0x200);
                   uint8_t text[8];
                   for (int i = 0; i < 8; i++) text[i] = r.range(0, 9) ? (uint8_t)set[r.range(0, 7)] : (uint8_t)r.range(0x80, 0xff);
                   for (int i = 0; i < 8; i++) {
                       const uint8_t c = i < kl ? (r.range(0, 2) ? text[i] : (uint8_t)set[r.range(0, 7)]) : 0;
                       const uint8_t t = i < tl ? text[i] : (uint8_t)r.next();
                       const uint8_t kc = (c >= 'a' && c <= 'z' && r.range(0, 1)) ? (uint8_t)(c - 32) : c;
                       if (at == 0xbcf260u) arena<uint8_t>(0x200)[i] = t;
                       else arena<uint16_t>(0x200)[i] = t;
                       if (at == 0xbceba0u) arena<uint16_t>(0x300)[i] = kc;
                       else arena<uint8_t>(0x300)[i] = kc;
                   }
               },
               [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(const void *, const void *))fn)(arena<void>(0x40), arena<void>(0x300))}; }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xdde005, (void *)h2_lower}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Matrix::StepBEE1E0 and Vector::StepBEE5B0: rotation matrices from random turns (the identity at times) and random
    // matrices (the real turns and rotation reads)
    for (uintptr_t at : {0xbee1e0u, 0xbee5b0u}) {
        Test x{at,
               [=](Rng &r) {
                   float *m = arena<float>(0x100);
                   for (int k = 0; k < 16; k++) m[k] = r.real(-1, 1);
                   const int kind = r.range(0, 4);
                   if (kind < 3) {
                       const float a = kind ? r.real(-3.1f, 3.1f) : 0.0f, b = kind ? r.real(-1.5f, 1.5f) : 0.0f;
                       const float ca = cosf(a), sa = sinf(a), cb = cosf(b), sb = sinf(b);
                       const float rm[9] = {ca, sa * sb, -sa * cb, 0, cb, sb, sa, -ca * sb, ca * cb};
                       for (int i = 0; i < 3; i++)
                           for (int j = 0; j < 3; j++) m[i * 4 + j] = rm[i * 3 + j];
                   }
                   for (int k = 0; k < 4; k++) arena<uint32_t>(0x40)[k] = r.next();
               },
               [at](void *fn) {
                   if (at == 0xbee1e0u) {
                       ((void(__thiscall *)(const void *, void *, void *, void *))fn)(arena<void>(0x100), arena<void>(0x40), arena<void>(0x44), arena<void>(0x48));
                       return Result{0};
                   }
                   return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, void *))fn)(arena<void>(0x100), arena<void>(0x40))};
               }};
        quick(x);
    }
    // Sky::Point4266A0: random unit axes, angles either sign (0 and tiny ones at times; the real turn)
    Test sky_point_4266a0{0x4266a0,
                          [=](Rng &r) {
                              float a[3], len = 0;
                              for (int k = 0; k < 3; k++) a[k] = r.real(-1, 1), len += a[k] * a[k];
                              for (int k = 0; k < 3; k++) *(float *)arena<uint8_t>(0x100 + 0x3f0 + k * 4) = a[k] / sqrtf(len);
                              const int kind = r.range(0, 6);
                              arena<float>(0x40)[0] = kind == 0 ? 0.0f : kind == 1 ? r.real(-1e-21f, 1e-21f) : r.real(-7, 7);
                              for (int k = 0; k < 3; k++) arena<uint32_t>(0x60)[k] = r.next();
                          },
                          [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(const void *, void *, float))fn)(arena<void>(0x100), arena<void>(0x60), arena<float>(0x40)[0])}; }};
    quick(sky_point_4266a0);
    // Waterfall::Copy9199B0: random points, i from 0 to n (the middle giving no turn), n 1-12 (the real turn)
    Test waterfall_copy_9199b0{0x9199b0,
                               [=](Rng &r) {
                                   for (int k = 0; k < 3; k++) arena<float>(0x40)[k] = r.real(-20, 20);
                                   const int n = r.range(1, 12);
                                   arena<int32_t>(0x50)[0] = r.range(0, 4) ? r.range(0, n) : n / 2, arena<int32_t>(0x50)[1] = n;
                                   for (int k = 0; k < 3; k++) arena<uint32_t>(0x60)[k] = r.next();
                               },
                               [](void *fn) {
                                   return Result{(uint32_t)(uintptr_t)((void *(__stdcall *)(void *, const void *, int32_t, int32_t))fn)(arena<void>(0x60), arena<void>(0x40), arena<int32_t>(0x50)[0], arena<int32_t>(0x50)[1])};
                               }};
    quick(waterfall_copy_9199b0);
    // Scenery::Point8B5510: random directions, nearly the same and nearly opposite ones (exactly so can put the ratio past
    // 1, where the game's acos needs its CRT set up) (a in ecx, b in edx)
    Test scenery_point_8b5510{0x8b5510,
                              [=](Rng &r) {
                                  float *a = arena<float>(0x40), *b = arena<float>(0x50);
                                  for (int k = 0; k < 3; k++) a[k] = r.real(-5, 5);
                                  const int kind = r.range(0, 4);
                                  for (int k = 0; k < 3; k++) b[k] = kind == 0 ? a[k] * 2 + r.real(-0.05f, 0.05f) : kind == 1 ? -a[k] + r.real(-0.05f, 0.05f) : r.real(-5, 5);
                              },
                              [](void *fn) {
                                  Result x;
                                  x.f = ((long double(__fastcall *)(const void *, const void *))fn)(arena<void>(0x40), arena<void>(0x50));
                                  return x;
                              }};
    quick(scenery_point_8b5510);
    // Scenery::Turn847040: footprints of 1-4 by 1-4 cells, every turn, places in and out (the real turn)
    Test scenery_turn_847040{0x847040,
                             [=](Rng &r) {
                                 uint8_t *self = arena<uint8_t>(0x100), *f = arena<uint8_t>(0x200);
                                 int32_t *t = (int32_t *)(self + 0x20);
                                 t[0] = (int32_t)(uintptr_t)f, t[1] = r.range(-3, 3), t[2] = r.range(-3, 3), t[3] = r.range(-50, 50), t[4] = r.range(0, 4);
                                 *(int32_t *)(f + 0x10) = r.range(1, 4), *(int32_t *)(f + 0x14) = r.range(1, 4);
                                 *(uint32_t *)(f + 0x18) = (uint32_t)(uintptr_t)arena<void>(0x300);
                                 for (int k = 0; k < 16 * 5; k++) arena<uint32_t>(0x300)[k] = r.range(0, 2) ? r.next() : (uint32_t)r.range(-100, 100);
                                 arena<int32_t>(0x40)[0] = t[1] + r.range(-2, 5), arena<int32_t>(0x40)[1] = t[2] + r.range(-2, 5);
                                 arena<uint32_t>(0x50)[0] = r.next(), arena<uint32_t>(0x50)[1] = r.next();
                             },
                             [](void *fn) {
                                 return Result{((uint32_t(__thiscall *)(void *, int32_t, int32_t, void *, void *))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<int32_t>(0x40)[1], arena<void>(0x50), arena<void>(0x54)) & 0xff};
                             }};
    quick(scenery_turn_847040);
    // Sky::Entry431F00 and Effect::Entry44B2A0: n from -1 past the count, random places and values; QuadTree::Move stood
    // in for (it notes what it is given)
    for (uintptr_t at : {0x431f00u, 0x44b2a0u}) {
        Test x{at,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   const int count = r.range(0, 4);
                   *(int32_t *)(self + 0x4c) = count, *(int32_t *)(self + 0x28) = count;
                   *(uint32_t *)(self + 0x40) = (uint32_t)(uintptr_t)arena<void>(0x400), *(uint32_t *)(self + 0x1c) = (uint32_t)(uintptr_t)arena<void>(0x400);
                   *(uint32_t *)(self + 0x50) = (uint32_t)(uintptr_t)arena<void>(0x300), *(uint32_t *)(self + 0x2c) = (uint32_t)(uintptr_t)arena<void>(0x300);
                   for (int k = 0; k < 0x40; k++) arena<uint32_t>(0x400)[k] = r.next();
                   for (int k = 0; k < 10; k++) arena<uint32_t>(0x80)[k] = 0;
                   arena<int32_t>(0x40)[0] = r.range(-1, 5), arena<float>(0x40)[1] = r.real(-10, 10);
                   for (int k = 0; k < 3; k++) arena<float>(0x50)[k] = r.real(-100, 100);
               },
               [at](void *fn) {
                   if (at == 0x431f00u) ((void(__thiscall *)(void *, int32_t, const void *))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<void>(0x50));
                   else ((void(__thiscall *)(void *, int32_t, float, const void *))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<float>(0x40)[1], arena<void>(0x50));
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x431240, (void *)h2_quad_move}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Terrain::Entry8E8140: entries on or off, growing or not, random boxes; Water::BoxSpan and Water::Point8E6D20 stood in
    // for (they note what they are given)
    {
        Test x{0x8e8140,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   *(uint32_t *)(self + 0x118) = (uint32_t)(uintptr_t)arena<void>(0x400);
                   for (int k = 0; k < 3 * 0x130 / 4; k++) arena<uint32_t>(0x400)[k] = r.next();
                   for (int e = 0; e < 3; e++) {
                       uint8_t *en = arena<uint8_t>(0x400 + e * 0x130);
                       en[3] = r.range(0, 1) ? (uint8_t)r.range(1, 255) : 0, en[1] = r.range(0, 1) ? (uint8_t)r.range(1, 255) : 0;
                       *(float *)(en + 0x10) = r.real(-5, 5), *(float *)(en + 0x50) = r.real(-5, 5);
                   }
                   for (int k = 0; k < 3; k++) arena<float>(0x40)[k] = r.real(-50, 50), arena<float>(0x50)[k] = r.real(-50, 50);
                   for (int k = 0; k < 12; k++) arena<uint32_t>(0xa0)[k] = 0;
                   arena<int32_t>(0x60)[0] = r.range(0, 2);
               },
               [](void *fn) {
                   ((void(__thiscall *)(const void *, int32_t, const void *, const void *))fn)(arena<void>(0x100), arena<int32_t>(0x60)[0], arena<void>(0x40), arena<void>(0x50));
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x8e6980, (void *)h2_box_span}, {0x8e6d20, (void *)h2_water_point}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Guest::Part753180: with or without a part, 0-3 entries already, random sizes and places (the real copies)
    Test guest_part_753180{0x753180,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100), *part = arena<uint8_t>(0x200);
                               *(uint32_t *)(self + 0x14) = r.range(0, 5) ? (uint32_t)(uintptr_t)part : 0;
                               *(float *)(self + 0xc) = r.real(-5, 5), *(float *)(self + 0x10) = r.real(-5, 5);
                               *(uint32_t *)(part + 0x40) = (uint32_t)(uintptr_t)arena<void>(0x400), *(int32_t *)(part + 0x4c) = r.range(0, 3);
                               for (int k = 0; k < 0x50; k++) arena<uint32_t>(0x400)[k] = r.next();
                               for (int k = 0; k < 3; k++) arena<float>(0x40)[k] = r.real(-50, 50);
                           },
                           [](void *fn) {
                               ((void(__thiscall *)(const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40));
                               return Result{0};
                           }};
    quick(guest_part_753180);
    // Animal::BitsB6CD90: 0-6 entries on or off, animals flagged or not, sizes either side of 0.5; the class value stood in
    // for
    {
        Test x{0xb6cd90,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   *(int32_t *)(self + 0x60) = r.range(0, 6), *(uint32_t *)(self + 0x54) = (uint32_t)(uintptr_t)arena<void>(0x200);
                   for (int i = 0; i < 6; i++) {
                       uint8_t *e = arena<uint8_t>(0x200 + i * 12), *a = arena<uint8_t>(0x400 + i * 0x180);
                       e[0] = r.range(0, 3) ? 1 : 0, *(uint32_t *)(e + 4) = (uint32_t)(uintptr_t)a;
                       *(uint32_t *)(a + 0xa8) = r.next(), *(float *)(a + 0x130) = r.range(0, 5) ? r.real(0, 1) : 0.5f, *(float *)(a + 0x140) = r.real(0, 3);
                   }
               },
               [](void *fn) { return Result{((uint32_t(__thiscall *)(const void *))fn)(arena<void>(0x100)) & 0xff}; }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xb35450, (void *)h2_animal_class_value}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Static::Step4987C0 / Step498840: tables 0-1 by id, 1-16 frames, entries of any height, frame counts from the real
    // Effects::Point498630 (no part at times)
    for (uintptr_t at : {0x4987c0u, 0x498840u}) {
        Test x{at,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   for (int t = 0; t < 2; t++) {
                       uint8_t *tab = arena<uint8_t>(0x200 + t * 0x40);
                       *(uint32_t *)(self + 0x54 + t * 4) = (uint32_t)(uintptr_t)tab;
                       *(int32_t *)(tab + 4) = r.range(1, 16), *(uint32_t *)(tab + 0xc) = r.next();
                       *(uint32_t *)(tab + 0x18) = (uint32_t)(uintptr_t)arena<void>(0x400 + t * 0x100);
                   }
                   for (int k = 0; k < 0x80; k++) arena<int32_t>(0x400)[k] = r.range(-5, 40);
                   *(uint32_t *)(self + 0x24) = r.range(0, 5) ? (uint32_t)(uintptr_t)arena<void>(0x380) : 0;
                   *(float *)(self + 0x40) = r.real(0, 3), *(float *)arena<uint8_t>(0x384) = r.real(0, 3), *(float *)arena<uint8_t>(0x388) = r.real(0, 2);
                   arena<uint32_t>(0x40)[0] = (uint32_t)r.range(0, 1) << 24 | (uint32_t)r.range(0, 9);
                   for (int k = 0; k < 4; k++) arena<uint32_t>(0x60)[k] = r.next();
               },
               [](void *fn) {
                   ((void(__thiscall *)(const void *, uint32_t, void *, void *))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0], arena<void>(0x60), arena<void>(0x68));
                   return Result{0};
               }};
        quick(x);
    }
    // Screen::Scale6ACC60 and Camera::PickA62D90: the game's GUI root and screen set (put back after); random rectangles
    // (out in ecx, r in edx), random cameras (own or at +0x48), planes and screen points
    for (uintptr_t at : {0x6acc60u, 0xa62d90u}) {
        static uint32_t saved_area, saved_screen;
        Test x{at,
               [=](Rng &r) {
                   uint8_t *area = arena<uint8_t>(0x700), *screen = arena<uint8_t>(0x800);
                   *(uint32_t *)0x12f83f0 = (at == 0xa62d90u || r.range(0, 4)) ? (uint32_t)(uintptr_t)area : 0;
                   *(uint32_t *)0x12f4618 = (uint32_t)(uintptr_t)screen;
                   *(float *)(area + 0x7c) = r.real(0, 100), *(float *)(area + 0x84) = *(float *)(area + 0x7c) + r.real(100, 2000);
                   *(float *)(area + 0x80) = r.real(0, 100), *(float *)(area + 0x88) = *(float *)(area + 0x80) + r.real(100, 2000);
                   *(int32_t *)(screen + 4) = r.range(320, 2560), *(int32_t *)(screen + 8) = r.range(200, 1600);
                   for (int k = 0; k < 4; k++) arena<float>(0x40)[k] = r.real(-3000, 3000);
                   uint8_t *self = arena<uint8_t>(0x100), *cam = r.range(0, 1) ? self + 0x200 : arena<uint8_t>(0x900);
                   *(uint32_t *)(self + 0x48) = cam == self + 0x200 ? 0 : (uint32_t)(uintptr_t)cam;
                   for (uint32_t o = 0x120; o < 0x1a0; o += 4) *(float *)(cam + o) = r.real(-1, 1);
                   for (int k = 0; k < 3; k++) *(float *)(cam + 0x160 + k * 4) = r.real(-50, 50);
                   *(float *)(cam + 0x190) = r.real(0.3f, 2), *(float *)(cam + 0x194) = r.real(0.5f, 2), *(float *)(cam + 0x198) = r.real(1, 30);
                   cam[0x1a6] = r.range(0, 1) ? 1 : 0;
                   for (int k = 0; k < 3; k++) arena<float>(0x60)[k] = r.real(-1, 1);
                   arena<float>(0x6c)[0] = r.real(-20, 20), arena<float>(0x70)[0] = r.real(0, 2000), arena<float>(0x74)[0] = r.real(0, 2000);
                   for (int k = 0; k < 4; k++) arena<uint32_t>(0x80)[k] = r.next();
               },
               [at](void *fn) {
                   if (at == 0x6acc60u) return Result{(uint32_t)(uintptr_t)((void *(__fastcall *)(void *, const void *))fn)(arena<void>(0x80), arena<void>(0x40))};
                   return Result{((uint32_t(__thiscall *)(void *, const void *, float, float, float, void *))fn)(arena<void>(0x100), arena<void>(0x60), arena<float>(0x6c)[0], arena<float>(0x70)[0], arena<float>(0x74)[0], arena<void>(0x80))};
               }};
        x.prepare = [](const rewrite::Entry *) { saved_area = *(uint32_t *)0x12f83f0, saved_screen = *(uint32_t *)0x12f4618; };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x12f83f0 = saved_area, *(uint32_t *)0x12f4618 = saved_screen; };
        quick(x);
    }
    // Window::Fits99C1A0: the game's 0x13413B0 set (put back after); rows and sizes either side of both limits;
    // Triple::Set stood in for (it notes each call)
    {
        static uint32_t saved_height;
        Test x{0x99c1a0,
               [=](Rng &r) {
                   *(float *)0x13413b0 = r.real(10, 30);
                   uint8_t *self = arena<uint8_t>(0x100), *inner = arena<uint8_t>(0x300), *part = arena<uint8_t>(0x500);
                   float *f = (float *)self;
                   f[0x7c / 4] = r.real(0, 50), f[0x84 / 4] = f[0x7c / 4] + r.real(50, 400);
                   f[0x80 / 4] = r.real(0, 50), f[0x88 / 4] = f[0x80 / 4] + r.real(50, 400);
                   f[0x8c / 4] = r.real(-100, 100), f[0x90 / 4] = r.real(-100, 100), f[0x114 / 4] = r.real(0, 500);
                   *(int32_t *)(self + 0x108) = r.range(0, 25);
                   *(uint32_t *)(self + 0xe0) = (uint32_t)(uintptr_t)inner;
                   *(uint32_t *)(inner + 0x158) = (uint32_t)(uintptr_t)part, *(uint32_t *)(inner + 0x164) = r.next(), *(uint32_t *)(inner + 0x168) = r.next();
                   *(float *)(part + 0x7c) = r.real(0, 50), *(float *)(part + 0x84) = r.real(50, 400);
                   for (int k = 0; k < 9; k++) arena<uint32_t>(0xe0)[k] = 0;
               },
               [](void *fn) {
                   ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { saved_height = *(uint32_t *)0x13413b0, stubs_for_now({{0x6a4b10, (void *)h2_triple_set}}); };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x13413b0 = saved_height, unstub_many(); };
        quick(x);
    }
    // Terrain::Curve8DF080: curves with random weights, t before, inside and past the curve's span, random seeds (the game's
    // random numbers really used: data compared)
    Test terrain_curve_8df080{0x8df080,
                              [=](Rng &r) {
                                  float *c = arena<float>(0x100);
                                  for (int k = 0; k < 8; k++) c[k] = r.real(-2, 2);
                                  c[2] = r.real(0, 0.3f), c[3] = r.real(0, 0.3f);
                                  arena<float>(0x40)[0] = r.real(-0.2f, 1.2f), arena<uint32_t>(0x40)[1] = r.next();
                              },
                              [](void *fn) {
                                  Result x;
                                  x.f = ((long double(__stdcall *)(float, const void *, uint32_t))fn)(arena<float>(0x40)[0], arena<void>(0x100), arena<uint32_t>(0x40)[1]);
                                  return x;
                              }};
    t.push_back(terrain_curve_8df080);
    // Fireworks::Entry9E87E0: the game's flag and table pointers set (put back after); entries of random weights (zero
    // ones at times), the game's random numbers really used (data compared)
    {
        static uint32_t saved_flag, saved_tables[4];
        Test x{0x9e87e0,
               [=](Rng &r) {
                   *(uint32_t *)0x10edae0 = r.range(0, 5) ? 0 : 1;
                   for (int k = 0; k < 4; k++) ((uint32_t *)0x10f1cd0)[k] = (uint32_t)(uintptr_t)arena<void>(0x400 + k * 0x100);
                   for (int k = 0; k < 0x100; k++) arena<float>(0x400)[k] = r.range(0, 3) ? r.real(0, 5) : 0.0f;
                   uint8_t *self = arena<uint8_t>(0x100);
                   *(int32_t *)(self + 0x58) = r.range(0, 3);
                   for (int k = 0; k < 5; k++) *(uint32_t *)(self + 0x70 + k * 4) = r.next();
                   arena<int32_t>(0x40)[0] = r.range(0, 7);
               },
               [](void *fn) {
                   ((void(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0]);
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { saved_flag = *(uint32_t *)0x10edae0, memcpy(saved_tables, (void *)0x10f1cd0, 16); };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x10edae0 = saved_flag, memcpy((void *)0x10f1cd0, saved_tables, 16); };
        t.push_back(x);
    }
    // Water::Shared9142C0: the game's shared object set (put back after), flagged or not; trails of 0-50 entries, full or
    // not, places moved or not; the trail's grow stood in for
    {
        static uint32_t saved_shared;
        Test x{0x9142c0,
               [=](Rng &r) {
                   *(uint32_t *)0x133a0f4 = (uint32_t)(uintptr_t)arena<void>(0x80);
                   arena<uint32_t>(0x80)[2] = r.range(0, 3) ? 0x10 | r.next() : r.next() & ~0x10u;
                   uint8_t *self = arena<uint8_t>(0x100);
                   for (int k = 1; k < 5; k++) *(float *)(self + k * 4) = r.real(-5, 5);
                   if (r.range(0, 3) == 0) *(uint32_t *)(self + 0xc) = *(uint32_t *)(self + 4), *(uint32_t *)(self + 0x10) = *(uint32_t *)(self + 8);
                   *(int32_t *)(self + 0x34) = r.range(0, 2);
                   *(uint32_t *)(self + 0x40) = r.next(), *(uint32_t *)(self + 0x44) = r.next();
                   const int c = r.range(0, 4) ? r.range(0, 8) : r.range(48, 51);
                   *(int32_t *)(self + 0x84) = c, *(int32_t *)(self + 0x80) = r.range(0, 1) ? c : c + r.range(1, 3);
                   *(uint32_t *)(self + 0x7c) = (uint32_t)(uintptr_t)arena<void>(0x400);
                   arena<uint32_t>(0x90)[0] = 0, arena<uint32_t>(0x90)[1] = 0;
               },
               [](void *fn) {
                   ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { saved_shared = *(uint32_t *)0x133a0f4, stubs_for_now({{0x913ca0, (void *)h2_trail_grow}}); };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x133a0f4 = saved_shared, unstub_many(); };
        quick(x);
    }
    // Text::FindBCEAA0 / BCE9D0 / BCF0A0 / BCF170: two texts (bytes or two-byte letters, a few in either case and high
    // values, 0-5 long, sharing their start often); tolower stood in for
    auto text_order_test = [&](uintptr_t at, bool wide_a, bool wide_b) {
        Test x{at,
               [=](Rng &r) {
                   static const uint16_t letters[] = {'a', 'A', 'b', 'B', 'z', 0x80, 0xc1, 0x141};
                   const bool wide[2] = {wide_a, wide_b};
                   for (int s = 0; s < 2; s++) {
                       uint8_t *text = arena<uint8_t>(0x200 + 0x40 * s), *chars = arena<uint8_t>(0x300 + 0x40 * s);
                       for (int i = 0; i < 8; i++) {
                           const uint16_t c = letters[r.range(0, 7)];
                           if (wide[s]) ((uint16_t *)chars)[i] = c;
                           else chars[i] = (uint8_t)c;
                       }
                       if (s == 1 && r.range(0, 1)) {
                           for (int i = 0; i < 8; i++) {
                               const uint16_t c = wide[0] ? ((uint16_t *)arena<uint8_t>(0x300))[i] : arena<uint8_t>(0x300)[i];
                               if (wide[1]) ((uint16_t *)chars)[i] = r.range(0, 3) ? c : c ^ 0x20;
                               else chars[i] = (uint8_t)(r.range(0, 3) ? c : c ^ 0x20);
                           }
                       }
                       *(uint32_t *)(text + 4) = r.range(0, 5), *(uint32_t *)(text + 0xc) = (uint32_t)(uintptr_t)chars;
                       arena<uint32_t>(0x100 + 4 * s)[0] = (uint32_t)(uintptr_t)text;
                   }
               },
               [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x104))}; }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xdde005, (void *)h2_lower}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    };
    text_order_test(0xbceaa0, true, false), text_order_test(0xbce9d0, true, true), text_order_test(0xbcf0a0, false, false), text_order_test(0xbcf170, false, true);
    // WildAnimal::RandomB34740: n from -3 to 24 (the game's random numbers really used: data compared)
    Test wildanimal_random_b34740{0xb34740, [=](Rng &r) { arena<int32_t>(0x40)[0] = r.range(-3, 24); },
                                  [](void *fn) { return Result{(uint32_t)((int32_t(__fastcall *)(int32_t))fn)(arena<int32_t>(0x40)[0])}; }};
    t.push_back(wildanimal_random_b34740);
    // Water::Step90F610: 0-8 entries (empty, marked or not), a start past the end at times, +4 and +0x10 alike at times
    // (the game's random numbers really used: data compared)
    Test water_step_90f610{0x90f610,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               uint32_t *list = arena<uint32_t>(0x200);
                               for (int k = 0; k < 8; k++) {
                                   uint8_t *p = arena<uint8_t>(0x300 + k * 0x20);
                                   for (int j = 0; j < 0x20; j += 4) *(uint32_t *)(p + j) = r.next();
                                   if (r.range(0, 2)) *(uint32_t *)(p + 0x14) |= 7;
                                   list[k] = r.range(0, 5) ? (uint32_t)(uintptr_t)p : 0;
                               }
                               *(uint32_t *)self = (uint32_t)(uintptr_t)list;
                               *(uint32_t *)(self + 4) = r.range(0, 2), *(uint32_t *)(self + 0x10) = r.range(0, 2);
                               *(uint32_t *)(self + 0x18) = r.range(0, 3), *(uint32_t *)(self + 0x1c) = r.range(0, 8);
                               arena<uint32_t>(0x40)[0] = r.next(), arena<uint32_t>(0x40)[1] = r.next();
                           },
                           [](void *fn) {
                               return Result{((uint32_t(__thiscall *)(void *, void *, void *))fn)(arena<void>(0x100), arena<uint32_t>(0x40), arena<uint32_t>(0x40) + 1)};
                           }};
    t.push_back(water_step_90f610);
    // Table::FindC2FA90: tables of up to 6 named records in 1-4 buckets (chains), names looked up in either case, missing
    // or empty; _stricmp stood in for
    {
        Test x{0xc2fa90,
               [=](Rng &r) {
                   static const char *const names[] = {"ab", "AB", "c", "Cd", "e", "abc", "zz", ""};
                   uint8_t *self = arena<uint8_t>(0x100), *entries = arena<uint8_t>(0x200), *heap = arena<uint8_t>(0x400);
                   const uint32_t extra = 0x10, nb = r.range(1, 4);
                   *(uint32_t *)self = (uint32_t)(uintptr_t)entries, *(uint32_t *)(self + 4) = extra;
                   *(uint32_t *)(self + 0x14) = nb, *(uint16_t *)(self + 0x10) = 7;
                   uint16_t *buckets = (uint16_t *)(heap + 0x300);
                   for (uint32_t k = 0; k < 4; k++) buckets[k] = 0;
                   *(uint32_t *)(entries + 7 * 8) = (uint32_t)(uintptr_t)heap + 0x300 - extra + r.range(0, 1);
                   for (int i = 1; i <= 6; i++) {
                       uint8_t *rec = heap + i * 0x40;
                       *(uint32_t *)(entries + i * 8) = (uint32_t)(uintptr_t)rec - extra + r.range(0, 1);
                       strcpy((char *)rec + 4, names[r.range(0, 6)]);
                       const uint32_t b = r.range(0, nb - 1);
                       *(uint16_t *)rec = buckets[b], buckets[b] = (uint16_t)i;
                   }
                   strcpy(arena<char>(0x40), names[r.range(0, 7)]);
                   arena<uint32_t>(0x60)[0] = r.next();
               },
               [](void *fn) { return Result{((uint16_t(__thiscall *)(void *, const char *, uint32_t))fn)(arena<void>(0x100), arena<char>(0x40), arena<uint32_t>(0x60)[0])}; }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xde22df, (void *)h2_stricmp}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Camera::AddCE7370: lists of 0-5 records, full or not, the low mark above or below; the list's grow stood in for
    {
        Test x{0xce7370,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   for (int k = 0; k < 0x24; k += 4) *(uint32_t *)(self + k) = r.next();
                   const int32_t c = r.range(0, 5);
                   *(uint32_t *)(self + 0x14) = (uint32_t)(uintptr_t)arena<void>(0x400);
                   *(int32_t *)(self + 0x1c) = c, *(int32_t *)(self + 0x18) = r.range(0, 1) ? c : c + r.range(1, 3);
                   *(int32_t *)(self + 0x20) = r.range(0, 7);
                   arena<uint32_t>(0x90)[0] = arena<uint32_t>(0x90)[1] = 0;
                   for (int k = 0; k < 3; k++) arena<uint32_t>(0x40)[k] = r.next();
               },
               [](void *fn) {
                   uint32_t *v = arena<uint32_t>(0x40);
                   return Result{(uint32_t)((int32_t(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), v[0], v[1], v[2])};
               }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xce7110, (void *)h2_list_grow}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // List::Add42DE10: pools of room 0-6 (empty ones grow to 200), full or not, free lists of item numbers 0-7, flags with
    // either bit; Pool::Grow stood in for
    {
        Test x{0x42de10,
               [=](Rng &r) {
                   uint32_t *pool = arena<uint32_t>(0x100), *free_list = arena<uint32_t>(0x800);
                   const int32_t room = r.range(0, 6), used = r.range(0, 3) ? r.range(0, room) : room;
                   pool[0] = (uint32_t)(uintptr_t)arena<void>(0x400), pool[1] = (uint32_t)(uintptr_t)free_list, pool[2] = used, pool[3] = room;
                   for (int k = 0; k < 8; k++) free_list[k] = r.range(0, 7);
                   for (int k = 0; k < 0x20; k++) arena<uint32_t>(0x400)[k] = r.next();
                   arena<uint32_t>(0x98)[0] = arena<uint32_t>(0x98)[1] = 0;
                   arena<uint32_t>(0x40)[0] = r.next(), arena<uint32_t>(0x40)[1] = r.next(), arena<uint32_t>(0x40)[2] = r.next();
               },
               [](void *fn) {
                   uint32_t *v = arena<uint32_t>(0x40);
                   return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), v[0], v[1], v[2])};
               }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x42d730, (void *)h2_pool_grow}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Model::SetPartCC6280: parts with 0-3 tracks, each with 1-4 keys of rising times at both offsets, t before, among and
    // past them (the real Keys::Find and Find2)
    Test model_set_part_cc6280{0xcc6280,
                               [=](Rng &r) {
                                   uint8_t *self = arena<uint8_t>(0x100), *model = arena<uint8_t>(0x180);
                                   uint32_t *lists = arena<uint32_t>(0x1c0);
                                   *(uint32_t *)self = (uint32_t)(uintptr_t)model, *(uint32_t *)(model + 8) = (uint32_t)(uintptr_t)lists;
                                   *(uint32_t *)(self + 0xc) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                   *(uint32_t *)(self + 0x10) = (uint32_t)(uintptr_t)arena<void>(0x240);
                                   *(uint32_t *)(self + 0x14) = (uint32_t)(uintptr_t)arena<void>(0x280);
                                   for (int k = 0; k < 0x30; k++) arena<uint32_t>(0x200)[k] = r.next();
                                   const int32_t n = r.range(0, 3);
                                   uint8_t *list = arena<uint8_t>(0x300);
                                   lists[n] = (uint32_t)(uintptr_t)list;
                                   const int32_t tracks = r.range(0, 3);
                                   *(int32_t *)(list + 4) = tracks, *(uint32_t *)(list + 0xc) = (uint32_t)(uintptr_t)arena<void>(0x340);
                                   for (int i = 0; i < tracks; i++) {
                                       uint8_t *tr = arena<uint8_t>(0x340 + i * 0x14);
                                       *(uint32_t *)tr = r.range(0, 15);
                                       for (int s = 0; s < 2; s++) {
                                           float *keys = arena<float>(0x500 + (i * 2 + s) * 0x40);
                                           const int32_t kc = r.range(1, 4);
                                           float time = r.real(-1, 0);
                                           for (int k = 0; k < kc; k++) keys[k * 4] = time, time += r.real(0.1f, 1);
                                           *(int32_t *)(tr + 4 + s * 8) = kc, *(uint32_t *)(tr + 8 + s * 8) = (uint32_t)(uintptr_t)keys;
                                       }
                                   }
                                   arena<int32_t>(0x40)[0] = n, arena<float>(0x40)[1] = r.real(-1.5f, 4);
                               },
                               [](void *fn) {
                                   return Result{(uint32_t)((int32_t(__thiscall *)(void *, int32_t, float))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<float>(0x40)[1])};
                               }};
    quick(model_set_part_cc6280);
    // List::Add42A2B0, List::Add430D00 and Water::Add90F6F0: lists of 0-4 records, full or not (the empty water list grows to
    // 64), random points and values; the grows stood in for (they note the new room)
    for (uintptr_t at : {0x42a2b0u, 0x430d00u, 0x90f6f0u}) {
        Test x{at,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   for (int k = 0; k < 0x50; k += 4) *(uint32_t *)(self + k) = r.next();
                   uint8_t *list = at == 0x90f6f0u ? self + 0x40 : self;
                   const int32_t c = r.range(0, 4);
                   *(uint32_t *)list = (uint32_t)(uintptr_t)arena<void>(0x400);
                   *(int32_t *)(list + 8) = c, *(int32_t *)(list + 4) = r.range(0, 1) ? c : c + r.range(1, 3);
                   for (int k = 0; k < 0x100; k++) arena<uint32_t>(0x400)[k] = r.next();
                   for (int k = 0; k < 3; k++) arena<float>(0x40)[k] = r.real(-100, 100), arena<float>(0x50)[k] = r.real(-100, 100);
                   arena<float>(0x60)[0] = r.real(-100, 100), arena<float>(0x60)[1] = r.real(-100, 100);
                   arena<uint32_t>(0x90)[0] = arena<uint32_t>(0x90)[1] = 0;
               },
               [at](void *fn) {
                   if (at == 0x90f6f0u)
                       return Result{((uint32_t(__thiscall *)(void *, const void *, float, float))fn)(arena<void>(0x100), arena<void>(0x40), arena<float>(0x60)[0], arena<float>(0x60)[1])};
                   return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, const void *, const void *, uint32_t))fn)(arena<void>(0x100), arena<void>(0x40), arena<void>(0x50), arena<uint32_t>(0x60)[0])};
               }};
        x.prepare = [](const rewrite::Entry *e) {
            stubs_for_now({{e->at == 0x42a2b0u ? 0x42a050u : e->at == 0x430d00u ? 0x430690u : 0x90d7a0u, (void *)h2_list_grow}});
        };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Water::Grow942320, Park::Pass939E40 and Animal::FindB829C0: lists of 0-5 entries, full or not, n or keys within and
    // past them; the grows stood in for (they note and set the new size)
    for (uintptr_t at : {0x942320u, 0x939e40u, 0xb829c0u}) {
        Test x{at,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   for (int k = 0; k < 0x170; k += 4) *(uint32_t *)(self + k) = r.next();
                   for (int k = 0; k < 0x100; k++) arena<uint32_t>(0x400)[k] = r.range(0, 3) ? r.range(0, 7) : r.next();
                   arena<uint32_t>(0x90)[0] = arena<uint32_t>(0x90)[1] = 0;
                   const int32_t c = r.range(0, 5);
                   if (at == 0x942320u) {
                       *(uint32_t *)(self + 0x160) = (uint32_t)(uintptr_t)arena<void>(0x400), *(int32_t *)(self + 0x164) = c;
                       arena<int32_t>(0x40)[0] = r.range(0, 9);
                   } else if (at == 0x939e40u) {
                       *(uint32_t *)(self + 0xc) = (uint32_t)(uintptr_t)arena<void>(0x400);
                       *(int32_t *)(self + 0x14) = c, *(int32_t *)(self + 0x10) = r.range(0, 1) ? c : c + r.range(1, 3);
                       *(int32_t *)(self + 0x18) = r.range(0, 9), *(int32_t *)(self + 0x34) = r.range(0, 2) ? r.range(1, 9) : 0;
                   } else {
                       *(uint32_t *)(self + 4) = (uint32_t)(uintptr_t)arena<void>(0x400), *(int32_t *)(self + 8) = c;
                       arena<uint32_t>(0x40)[0] = r.range(0, 7);
                   }
               },
               [at](void *fn) {
                   if (at == 0x939e40u) return Result{((uint32_t(__thiscall *)(void *))fn)(arena<void>(0x100))};
                   return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0])};
               }};
        x.prepare = [](const rewrite::Entry *e) {
            stubs_for_now({{e->at == 0x942320u ? 0x940670u : e->at == 0x939e40u ? 0x939430u : 0xb82520u, (void *)h2_list_grow}});
        };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // VertexDeclaration::MoveBDC800: 0-6 ordered entries of few kinds and values, new ones before, among and after them
    // (the game's memmove really used)
    Test vertexdeclaration_move_bdc800{0xbdc800,
                                       [=](Rng &r) {
                                           uint8_t *self = arena<uint8_t>(0x1000);
                                           const int32_t c = r.range(0, 6);
                                           uint16_t *e = (uint16_t *)(self + 0xc);
                                           uint32_t key = 0;
                                           for (int i = 0; i < 0x20; i++) e[i] = (uint16_t)r.next();
                                           for (int i = 0; i < c; i++) {
                                               key += r.range(0, 3);
                                               e[i * 2] = (uint16_t)(key & 3), e[i * 2 + 1] = (uint16_t)((key >> 2 & 3) << 14 | r.range(0, 0x3fff));
                                           }
                                           *(int32_t *)(self + 0x20c) = c * 4;
                                           arena<uint32_t>(0x40)[0] = r.next() & 0xffff0000 | r.range(0, 4);
                                           arena<uint32_t>(0x40)[1] = r.next() & 0xffff0000 | r.range(0, 3) << 14 | r.range(0, 0x3fff);
                                       },
                                       [](void *fn) {
                                           return Result{(uint32_t)((int32_t(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x1000), arena<uint32_t>(0x40)[0], arena<uint32_t>(0x40)[1])};
                                       }};
    quick(vertexdeclaration_move_bdc800);
    // Guest::Next749B60: texts of 0-6 two-byte letters (a 0 inside at times), already ended or not, with room or not;
    // Shared::Own stood in for
    {
        Test x{0x749b60,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100), *text = arena<uint8_t>(0x200);
                   uint16_t *letters = arena<uint16_t>(0x300);
                   *(uint32_t *)(self + 0x40) = (uint32_t)(uintptr_t)text;
                   const uint32_t n = r.range(0, 6);
                   for (int i = 0; i < 0x40; i++) letters[i] = (uint16_t)(r.range(0, 9) ? r.range(0x20, 0x17f) : 0);
                   if (r.range(0, 1)) letters[n] = 0;
                   *(uint32_t *)(text + 4) = n, *(uint32_t *)(text + 8) = n + r.range(0, 2), *(uint32_t *)(text + 0xc) = (uint32_t)(uintptr_t)letters;
                   for (int k = 0; k < 5; k++) arena<uint32_t>(0xa0)[k] = 0;
               },
               [](void *fn) { return Result{(uint32_t)(uintptr_t)((char *(__thiscall *)(void *))fn)(arena<void>(0x100))}; }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbcffe0, (void *)h2_text_own}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Effect::Entry44A690: n from -2 past the count, entries used or not, slots in blocks 0-0xFFFF (high bit at times); the
    // slot free and block mark stood in for
    {
        Test x{0x44a690,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100), *pool = arena<uint8_t>(0x180);
                   const int32_t count = r.range(1, 6), used = r.range(0, count);
                   *(uint32_t *)(self + 0x1c) = (uint32_t)(uintptr_t)arena<void>(0x400);
                   *(uint32_t *)(self + 0x20) = (uint32_t)(uintptr_t)arena<void>(0x200);
                   *(int32_t *)(self + 0x24) = used, *(int32_t *)(self + 0x28) = count;
                   *(uint32_t *)(self + 0x2c) = (uint32_t)(uintptr_t)pool, *(uint32_t *)(pool + 8) = r.next();
                   for (int k = 0; k < 0x60; k++) arena<uint32_t>(0x400)[k] = r.next();
                   for (int k = 0; k < 8; k++) arena<uint32_t>(0x200)[k] = r.next(), arena<uint8_t>(0x420)[k * 0x40] = (uint8_t)r.range(0, 1);
                   for (int k = 0; k < 5; k++) arena<uint32_t>(0xb0)[k] = 0;
                   arena<int32_t>(0x40)[0] = r.range(-2, count + 1);
               },
               [](void *fn) {
                   ((void(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0]);
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x42f6f0, (void *)h2_slot_free}, {0x42f670, (void *)h2_block_mark}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Fireworks::Slot9F9B90: lists of no block or 0-3 bits, full or not, least bits 0-4; the grid's take and give stood in for
    {
        Test x{0x9f9b90,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   const int32_t bits = r.range(-1, 3), room = bits >= 0 ? 1 << bits : 0;
                   *(uint32_t *)self = r.next(), *(int32_t *)(self + 4) = r.range(0, 4);
                   *(uint32_t *)(self + 8) = (uint32_t)(uintptr_t)arena<void>(0x400), *(int32_t *)(self + 0xc) = bits;
                   *(int32_t *)(self + 0x10) = r.range(0, 1) ? room : r.range(0, room);
                   for (int k = 0; k < 0x20; k++) arena<uint32_t>(0x400)[k] = r.next(), arena<uint32_t>(0x600)[k] = r.next();
                   for (int k = 0; k < 2; k++) arena<uint32_t>(0xc0)[k] = 0;
                   for (int k = 0; k < 3; k++) arena<uint32_t>(0xc8)[k] = 0;
                   arena<uint32_t>(0x40)[0] = r.next();
               },
               [](void *fn) { return Result{((uint32_t(__thiscall *)(void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40))}; }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x850030, (void *)h2_grid_take}, {0x8501c0, (void *)h2_grid_give}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Record::Pass9838D0: 0-5 records, a free one at times; the list's grow stood in for
    {
        Test x{0x9838d0,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   const int32_t c = r.range(0, 5);
                   *(uint32_t *)(self + 0x20) = (uint32_t)(uintptr_t)arena<void>(0x400), *(int32_t *)(self + 0x24) = c;
                   for (int k = 0; k < 0x60; k++) arena<uint32_t>(0x400)[k] = (k & 1) || r.range(0, 4) ? r.next() : 0;
                   arena<uint32_t>(0x90)[0] = arena<uint32_t>(0x90)[1] = 0;
                   arena<uint32_t>(0x40)[0] = r.next(), arena<float>(0x40)[1] = r.real(-100, 100);
               },
               [](void *fn) {
                   return Result{(uint32_t)((int32_t(__thiscall *)(void *, uint32_t, float))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0], arena<float>(0x40)[1])};
               }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x9837e0, (void *)h2_list_grow}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Effect::InRange7B3A50: rectangles and objects inside, on the edges and outside either way (NaNs at times), lists full
    // or not; the list's grow stood in for
    {
        Test x{0x7b3a50,
               [=](Rng &r) {
                   float *self = arena<float>(0x100);
                   uint8_t *obj = arena<uint8_t>(0x200);
                   for (int k = 0; k < 2; k++) self[k] = r.real(-10, 0), self[k + 2] = r.real(0, 10);
                   auto pick = [&](int k) {
                       const int c = r.range(0, 9);
                       return c == 0 ? self[k] : c == 1 ? self[k + 2] : c == 2 ? NAN : r.real(-12, 12);
                   };
                   *(float *)(obj + 0x80) = pick(0), *(float *)(obj + 0x88) = pick(1);
                   arena<uint32_t>(0x40)[0] = (uint32_t)(uintptr_t)obj;
                   const int32_t c = r.range(0, 4);
                   *(uint32_t *)(self + 4) = (uint32_t)(uintptr_t)arena<void>(0x400);
                   *(int32_t *)(self + 6) = c, *(int32_t *)(self + 5) = r.range(0, 1) ? c : c + r.range(1, 3);
                   for (int k = 0; k < 0x20; k++) arena<uint32_t>(0x400)[k] = r.next();
                   arena<uint32_t>(0x90)[0] = arena<uint32_t>(0x90)[1] = 0;
               },
               [](void *fn) {
                   ((void(__thiscall *)(void *, const void *, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<void>(0x40), 1, 2, 3);
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x72af00, (void *)h2_list_grow}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Scenery::Step8915F0 and Water::Step973500: sorted lists of 0-7 stamps from few values, the stamp sought one of them or
    // between (the real Stamp::Compare)
    for (uintptr_t at : {0x8915f0u, 0x973500u}) {
        Test x{at,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   const int32_t c = r.range(0, 7);
                   uint32_t *stamps = arena<uint32_t>(0x800), *ptrs = arena<uint32_t>(0x700);
                   uint32_t key = r.range(0, 3);
                   for (int i = 0; i < 8; i++) {
                       uint32_t *s = stamps + i * 8;
                       for (int k = 0; k < 8; k++) s[k] = r.next();
                       s[0] = key >> 3, s[1] = key >> 2 & 1, s[2] = key >> 1 & 1, s[3] = 0, s[4] = 0, s[5] = (int32_t)(key & 1) - 1;
                       key += r.range(1, 3);
                       ptrs[i] = (uint32_t)(uintptr_t)s;
                   }
                   if (at == 0x8915f0u) *(uint32_t *)(self + 0x3d4) = (uint32_t)(uintptr_t)stamps, *(int32_t *)(self + 0x3d8) = c;
                   else *(uint32_t *)(self + 0x18) = (uint32_t)(uintptr_t)ptrs, *(int32_t *)(self + 0x1c) = c;
                   const uint32_t k = r.range(0, 24);
                   uint32_t *q = arena<uint32_t>(0x40);
                   q[0] = k >> 3, q[1] = k >> 2 & 1, q[2] = k >> 1 & 1, q[3] = 0, q[4] = 0, q[5] = (int32_t)(k & 1) - 1;
                   arena<uint32_t>(0x60)[0] = r.next();
               },
               [](void *fn) {
                   uint32_t *q = arena<uint32_t>(0x40);
                   const uint8_t found = ((uint8_t(__thiscall *)(void *, uint32_t, uint32_t, const void *, int32_t, void *))fn)(arena<void>(0x100), q[0], q[1], q + 2, (int32_t)q[5], arena<void>(0x60));
                   return Result{found};
               }};
        quick(x);
    }
    // Water::Step973580: sorted lists of 0-7 keys from few values, the key sought one of them or between (the real
    // Key::Compare)
    Test water_step_973580{0x973580,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               uint32_t *keys = arena<uint32_t>(0x800), *ptrs = arena<uint32_t>(0x700);
                               uint32_t key = r.range(0, 3);
                               auto fill = [](uint32_t *s, uint32_t k) {
                                   s[0] = k >> 4, s[1] = k >> 3 & 1, s[2] = k >> 2 & 1, s[3] = 0, s[4] = 0, s[5] = k >> 1 & 1, s[6] = 0, s[7] = (int32_t)(k & 1) - 1;
                               };
                               for (int i = 0; i < 8; i++) {
                                   uint32_t *s = keys + i * 8;
                                   fill(s, key);
                                   key += r.range(1, 3);
                                   ptrs[i] = (uint32_t)(uintptr_t)s;
                               }
                               *(uint32_t *)(self + 0x24) = (uint32_t)(uintptr_t)ptrs, *(int32_t *)(self + 0x28) = r.range(0, 7);
                               fill(arena<uint32_t>(0x40), r.range(0, 24));
                               arena<uint32_t>(0x60)[0] = r.next();
                           },
                           [](void *fn) {
                               uint32_t *q = arena<uint32_t>(0x40);
                               const uint8_t found = ((uint8_t(__thiscall *)(void *, uint32_t, uint32_t, const void *, uint32_t, uint32_t, uint32_t, void *))fn)(
                                   arena<void>(0x100), q[0], q[1], q + 2, q[5], q[6], q[7], arena<void>(0x60));
                               return Result{found};
                           }};
    quick(water_step_973580);
    // Water::Pass942280 and Car::Step5124B0: lists of 0-4 entries, keys found or not, full or not; the grows stood in for
    for (uintptr_t at : {0x942280u, 0x5124b0u}) {
        Test x{at,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   for (int k = 0; k < 0x180; k += 4) *(uint32_t *)(self + k) = r.next();
                   uint8_t *list = at == 0x942280u ? self + 0x16c : self;
                   const int32_t c = r.range(0, 4);
                   *(uint32_t *)list = (uint32_t)(uintptr_t)arena<void>(0x400);
                   *(int32_t *)(list + 8) = c, *(int32_t *)(list + 4) = r.range(0, 1) ? c : c + r.range(1, 3);
                   for (int k = 0; k < 0x80; k++) arena<uint32_t>(0x400)[k] = r.range(0, 7);
                   for (int k = 0; k < 8; k++) arena<uint32_t>(0x40)[k] = r.next(), arena<uint32_t>(0x60)[k] = r.next();
                   arena<uint32_t>(0x60)[0] = r.range(0, 7);
                   arena<uint32_t>(0x90)[0] = arena<uint32_t>(0x90)[1] = 0;
               },
               [at](void *fn) {
                   uint32_t *v = arena<uint32_t>(0x60);
                   if (at == 0x942280u) return Result{((uint32_t(__thiscall *)(void *, uint32_t, const void *))fn)(arena<void>(0x100), v[0], arena<void>(0x40)) & 0xff};
                   return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, const void *, const void *, uint32_t, uint32_t, uint32_t, uint32_t))fn)(
                       arena<void>(0x100), arena<void>(0x40), arena<void>(0x50), v[1], v[2], v[3], v[4])};
               }};
        x.prepare = [](const rewrite::Entry *e) { stubs_for_now({{e->at == 0x942280u ? 0x940730u : 0x5101a0u, (void *)h2_list_grow}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Water::Pass91C040, Record::Kind984870 and Sky::Pass4251C0: lists of 0-4 entries (triples found or not, full or not),
    // 0-12 values with +0xA4 either side of 0 (NaNs at times), random points; the grows stood in for
    for (uintptr_t at : {0x91c040u, 0x984870u, 0x4251c0u}) {
        Test x{at,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   for (int k = 0; k < 0x140; k += 4) *(uint32_t *)(self + k) = r.next();
                   for (int k = 0; k < 0x80; k++) arena<uint32_t>(0x400)[k] = r.range(0, 2);
                   for (int k = 0; k < 6; k++) arena<float>(0x40)[k] = r.real(-100, 100);
                   for (int k = 0; k < 3; k++) arena<uint32_t>(0x60)[k] = r.range(0, 2);
                   arena<uint32_t>(0x90)[0] = arena<uint32_t>(0x90)[1] = 0;
                   const int32_t c = r.range(0, 4);
                   if (at == 0x91c040u) {
                       *(uint32_t *)(self + 0x130) = (uint32_t)(uintptr_t)arena<void>(0x400);
                       *(int32_t *)(self + 0x138) = c, *(int32_t *)(self + 0x134) = r.range(0, 1) ? c : c + r.range(1, 3);
                   } else if (at == 0x984870u) {
                       *(uint32_t *)(self + 0x9c) = (uint32_t)(uintptr_t)arena<void>(0x400);
                       *(int32_t *)(self + 0xa0) = r.range(0, 3) ? r.range(0, 12) : 12;
                       *(float *)(self + 0xa4) = r.range(0, 9) ? r.real(-5, 5) : r.range(0, 1) ? NAN : -0.0f;
                   } else {
                       *(uint32_t *)(self + 0x44) = (uint32_t)(uintptr_t)arena<void>(0x400), *(int32_t *)(self + 0x48) = c;
                   }
               },
               [at](void *fn) {
                   uint32_t *v = arena<uint32_t>(0x60);
                   if (at == 0x91c040u) return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), v[0], v[1], v[2])};
                   if (at == 0x984870u) return ((void(__thiscall *)(void *))fn)(arena<void>(0x100)), Result{0};
                   return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, const void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40), arena<float>(0x40) + 3)};
               }};
        x.prepare = [](const rewrite::Entry *e) {
            stubs_for_now({{e->at == 0x91c040u ? 0x91a0b0u : e->at == 0x984870u ? 0x9847e0u : 0x424290u, (void *)h2_list_grow}});
        };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Water::Remove95A660: the find giving numbers under, at and past the count (high halves at times), edges lists full or
    // not, corners 0-7 with their last edges; the find and the list's grow stood in for
    {
        Test x{0x95a660,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   for (int k = 0; k < 0x24; k += 4) *(uint32_t *)(self + k) = r.next();
                   const int32_t c = r.range(0, 4);
                   *(uint32_t *)(self + 8) = (uint32_t)(uintptr_t)arena<void>(0x400);
                   *(int32_t *)(self + 0x10) = c, *(int32_t *)(self + 0xc) = r.range(0, 1) ? c : c + r.range(1, 3);
                   *(uint32_t *)(self + 0x20) = (uint32_t)(uintptr_t)arena<void>(0x600);
                   for (int k = 0; k < 0x40; k++) arena<uint32_t>(0x400)[k] = r.next(), arena<uint32_t>(0x600)[k] = r.next();
                   arena<uint32_t>(0xd0)[0] = (r.range(0, 3) ? 0 : r.next() & 0xffff0000) | (r.range(0, 1) ? r.range(0, 6) : 0xffff);
                   for (int k = 1; k < 4; k++) arena<uint32_t>(0xd0)[k] = 0;
                   arena<uint32_t>(0x90)[0] = arena<uint32_t>(0x90)[1] = 0;
                   arena<uint32_t>(0x40)[0] = r.next() & 0xffff0000 | r.range(0, 7), arena<uint32_t>(0x40)[1] = r.next() & 0xffff0000 | r.range(0, 7);
               },
               [](void *fn) {
                   return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0], arena<uint32_t>(0x40)[1])};
               }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x95a570, (void *)h2_edge_find}, {0x95a090, (void *)h2_list_grow}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Car::Pass50E630: the game's water pointer set (put back after); 0-4 records holding entries or -1s; the release stood
    // in for
    {
        static uint32_t saved_water;
        Test x{0x50e630,
               [=](Rng &r) {
                   *(uint32_t *)0x133a380 = (uint32_t)(uintptr_t)arena<void>(0x180);
                   uint32_t *list = arena<uint32_t>(0x100);
                   list[0] = (uint32_t)(uintptr_t)arena<void>(0x400), list[1] = r.range(0, 4);
                   for (int k = 0; k < 0x20; k++) arena<uint32_t>(0x400)[k] = r.range(0, 2) ? r.range(0, 99) : (uint32_t)-(int32_t)r.range(1, 3);
                   for (int k = 0; k < 17; k++) arena<uint32_t>(0xe0)[k] = 0;
               },
               [](void *fn) { return Result{(uint32_t)((int32_t(__stdcall *)(const void *))fn)(arena<void>(0x100))}; }};
        x.prepare = [](const rewrite::Entry *) { saved_water = *(uint32_t *)0x133a380, stubs_for_now({{0x94ef60, (void *)h2_water_release}}); };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x133a380 = saved_water, unstub_many(); };
        quick(x);
    }
    // Labels::Step470D70: random directions (unit and not), flags either way (the real Matrix::LookAlongZSide)
    Test labels_step_470d70{0x470d70,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x1000);
                                for (int k = 0x120; k < 0x1b0; k += 4) *(uint32_t *)(self + k) = r.next();
                                self[0x1a7] = (uint8_t)r.range(0, 1);
                                for (int k = 0; k < 3; k++) arena<float>(0x40)[k] = r.real(-1, 1);
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(void *, const void *))fn)(arena<void>(0x1000), arena<void>(0x40));
                                return Result{0};
                            }};
    quick(labels_step_470d70);
    // Animal::CheckADBDF0: on or not, out given or not, interface states 1-3, rings empty or not with ends 0-1023 (the
    // record before 0 is the last); the interface stood in for
    {
        Test x{0xadbdf0,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x1000);
                   self[0xb0] = (uint8_t)(r.range(0, 5) ? 1 : 0);
                   const int32_t end = r.range(0, 3) ? r.range(0, 1023) : 0, start = r.range(0, 3) ? r.range(0, 1023) : end;
                   *(int32_t *)(self + 0xc124) = end, *(int32_t *)(self + 0xc120) = start;
                   const uint32_t *slot = (uint32_t *)(self + ((end + 0x3ff) % 0x400 + 6) * 48);
                   for (int k = 0; k < 3; k++) ((uint32_t *)slot)[k] = r.next();
                   *(int32_t *)(arena<uint8_t>(0x180) + 0x50) = r.range(1, 3);
                   for (int k = 0; k < 3; k++) arena<uint32_t>(0x40)[k] = r.next();
                   arena<uint32_t>(0x60)[0] = r.range(0, 5) ? (uint32_t)(uintptr_t)arena<void>(0x40) : 0;
                   arena<uint32_t>(0xf8)[0] = 0;
               },
               [](void *fn) {
                   return Result{((uint8_t(__thiscall *)(void *, void *))fn)(arena<void>(0x1000), (void *)(uintptr_t)arena<uint32_t>(0x60)[0])};
               }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xadb340, (void *)h2_animal_interface}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Animal::StepB6CF60: the game's animals set (put back after); rising or falling, values and steps near both ends (and
    // exactly on them), the find giving 0 or not; the find stood in for
    {
        static uint32_t saved_animals;
        Test x{0xb6cf60,
               [=](Rng &r) {
                   *(uint32_t *)0x13ceee8 = r.next();
                   uint8_t *self = arena<uint8_t>(0x200);
                   *(uint32_t *)(self - 0xa8) = r.next();
                   self[8] = (uint8_t)r.range(0, 1);
                   *(float *)(self - 0x78) = r.range(0, 5) ? r.real(0, 1.1f) : r.range(0, 1) ? 1.0f : 0.1f;
                   arena<float>(0x40)[0] = r.range(0, 5) ? r.real(0, 0.2f) : 0.0f;
                   arena<uint32_t>(0x1f0)[0] = r.range(0, 1) ? r.next() : 0;
                   arena<uint32_t>(0xfc)[0] = 0;
               },
               [](void *fn) {
                   ((void(__thiscall *)(void *, const void *))fn)(arena<void>(0x200), arena<void>(0x40));
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { saved_animals = *(uint32_t *)0x13ceee8, stubs_for_now({{0xb6c900, (void *)h2_animals_pass}}); };
        x.finish = [](const rewrite::Entry *) { *(uint32_t *)0x13ceee8 = saved_animals, unstub_many(); };
        quick(x);
    }
    // Shader::ParseCBBE20: texts of 0-6 letters and strings of 0-6; the own stood in for
    {
        Test x{0xcbbe20,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100), *text = arena<uint8_t>(0x200);
                   *(uint32_t *)(self + 0x20) = (uint32_t)(uintptr_t)text;
                   const uint32_t len = r.range(0, 6);
                   *(uint32_t *)(text + 4) = len, *(uint32_t *)(text + 8) = len + r.range(0, 3), *(uint32_t *)(text + 0xc) = (uint32_t)(uintptr_t)arena<void>(0x300);
                   for (int k = 0; k < 0x10; k++) arena<uint32_t>(0x300)[k] = r.next();
                   char *s = arena<char>(0x40);
                   const int n = r.range(0, 6);
                   for (int k = 0; k < n; k++) s[k] = (char)r.range('a', 'z');
                   s[n] = 0;
                   for (int k = 0; k < 5; k++) arena<uint32_t>(0xa0)[k] = 0;
               },
               [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(void *, const char *))fn)(arena<void>(0x100), arena<char>(0x40))}; }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbd02b0, (void *)h2_text_own}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Water::Add91D8C0, Scenery::Copy863B80 and Ride::Pass557DC0: lists of 0-4 entries (full or not, found or not), random
    // fields and records; the grows and resize stood in for
    for (uintptr_t at : {0x91d8c0u, 0x863b80u, 0x557dc0u}) {
        Test x{at,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100), *from = arena<uint8_t>(0x300);
                   for (int k = 0; k < 0x120; k += 4) *(uint32_t *)(self + k) = r.next(), *(uint32_t *)(from + k) = r.next();
                   for (int k = 0; k < 0x40; k++) arena<uint32_t>(0x400)[k] = r.range(0, 2), arena<uint32_t>(0x600)[k] = r.next();
                   for (int k = 0; k < 4; k++) arena<uint32_t>(0x40)[k] = r.range(0, 2), arena<uint32_t>(0x60)[k] = r.next();
                   arena<uint32_t>(0x90)[0] = arena<uint32_t>(0x90)[1] = 0;
                   const int32_t c = r.range(0, 4);
                   if (at == 0x91d8c0u) {
                       *(uint32_t *)(self + 0x110) = (uint32_t)(uintptr_t)arena<void>(0x400);
                       *(int32_t *)(self + 0x118) = c, *(int32_t *)(self + 0x114) = r.range(0, 1) ? c : c + r.range(1, 3);
                   } else if (at == 0x863b80u) {
                       *(uint32_t *)(self + 0x28) = (uint32_t)(uintptr_t)arena<void>(0x400), *(int32_t *)(self + 0x2c) = r.range(0, 4);
                       *(uint32_t *)(from + 0x28) = (uint32_t)(uintptr_t)arena<void>(0x600), *(int32_t *)(from + 0x2c) = c;
                   } else {
                       *(uint32_t *)(self + 0x94) = (uint32_t)(uintptr_t)arena<void>(0x400), *(int32_t *)(self + 0x98) = c;
                   }
               },
               [at](void *fn) {
                   uint32_t *v = arena<uint32_t>(0x40);
                   if (at == 0x91d8c0u) return Result{(uint32_t)((int32_t(__thiscall *)(void *, uint32_t, uint32_t, const void *))fn)(arena<void>(0x100), v[0], v[1], arena<void>(0x60))};
                   if (at == 0x863b80u) return Result{((uint32_t(__thiscall *)(void *, const void *))fn)(arena<void>(0x100), arena<void>(0x300))};
                   return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), v[0], v[1], arena<uint32_t>(0x60)[0], v[3])};
               }};
        x.prepare = [](const rewrite::Entry *e) {
            stubs_for_now({{e->at == 0x91d8c0u ? 0x91c630u : e->at == 0x863b80u ? 0x863a60u : 0x557d20u, (void *)h2_list_grow}});
        };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Water::Find938670: 0-4 entries with keys 0-3 (high halves random), keys found or not, full or not; the grow stood in for
    {
        Test x{0x938670,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   for (int k = 0; k < 0x48; k += 4) *(uint32_t *)(self + k) = r.next();
                   const int32_t c = r.range(0, 4);
                   *(uint32_t *)(self + 0x40) = (uint32_t)(uintptr_t)arena<void>(0x400);
                   *(int32_t *)(self + 0x3c) = c, *(int32_t *)(self + 0x44) = r.range(0, 1) ? c : c + r.range(1, 3);
                   for (int k = 0; k < 0x40; k++) arena<uint32_t>(0x400)[k] = r.next();
                   for (int k = 0; k < 8; k++) arena<uint16_t>(0x404)[k * 8] = (uint16_t)r.range(0, 3), arena<float>(0x408)[k * 4] = r.real(-9, 9), arena<float>(0x40c)[k * 4] = r.real(-9, 9);
                   arena<uint32_t>(0x40)[0] = r.next() & 0xffff0000 | r.range(0, 4);
                   arena<float>(0x40)[1] = r.real(-5, 5), arena<float>(0x40)[2] = r.real(-5, 5);
                   arena<uint32_t>(0x90)[0] = arena<uint32_t>(0x90)[1] = 0;
               },
               [](void *fn) {
                   return Result{((uint32_t(__thiscall *)(void *, uint32_t, float, float))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0], arena<float>(0x40)[1], arena<float>(0x40)[2])};
               }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x9377d0, (void *)h2_list_grow}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Table::SetCE3160: the find giving a slot or 0xFFFF (high halves random), lists full or not, a 0-7; the find and grow
    // stood in for
    {
        Test x{0xce3160,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   for (int k = 0; k < 0x20; k += 4) *(uint32_t *)(self + k) = r.next();
                   const int32_t c = r.range(0, 4);
                   *(uint32_t *)(self + 8) = (uint32_t)(uintptr_t)arena<void>(0x600), *(uint32_t *)(self + 0x10) = (uint32_t)(uintptr_t)arena<void>(0x700);
                   *(uint32_t *)(self + 0x14) = (uint32_t)(uintptr_t)arena<void>(0x400);
                   *(int32_t *)(self + 0x1c) = c, *(int32_t *)(self + 0x18) = r.range(0, 1) ? c : c + r.range(1, 3);
                   for (int k = 0; k < 0x40; k++) arena<uint32_t>(0x400)[k] = r.next(), arena<uint32_t>(0x600)[k] = r.next(), arena<uint32_t>(0x700)[k] = r.next();
                   arena<uint32_t>(0xd0)[0] = (r.next() & 0xffff0000) | (r.range(0, 1) ? r.range(0, 4) : 0xffff);
                   for (int k = 1; k < 4; k++) arena<uint32_t>(0xd0)[k] = 0;
                   arena<uint32_t>(0x90)[0] = arena<uint32_t>(0x90)[1] = 0;
                   arena<uint32_t>(0x40)[0] = r.next() & 0xffff0000 | r.range(0, 7), arena<uint32_t>(0x40)[1] = r.next();
               },
               [](void *fn) {
                   return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0], arena<uint32_t>(0x40)[1])};
               }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xce3100, (void *)h2_edge_find}, {0xce2e30, (void *)h2_list_grow}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Animal::StepA704F0 and Animal::StepAFDCB0: lists of 0-4 records, full or not (the empty one grows to 16), free lists
    // of 0-3 slots, n 0-4; the grows stood in for
    for (uintptr_t at : {0xa704f0u, 0xafdcb0u}) {
        Test x{at,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x1000);
                   const int32_t c = r.range(0, 4);
                   for (int k = 0; k < 0x80; k++) arena<uint32_t>(0x400)[k] = r.next();
                   for (int k = 0; k < 4; k++) arena<uint32_t>(0x40)[k] = r.next(), arena<uint32_t>(0x60)[k] = r.next();
                   arena<uint32_t>(0x90)[0] = arena<uint32_t>(0x90)[1] = 0;
                   if (at == 0xa704f0u) {
                       *(uint32_t *)(self + 0x50b4) = (uint32_t)(uintptr_t)arena<void>(0x400);
                       *(int32_t *)(self + 0x50bc) = c, *(int32_t *)(self + 0x50b8) = r.range(0, 1) ? c : c + r.range(1, 3);
                   } else {
                       *(uint32_t *)(self + 0x107c) = (uint32_t)(uintptr_t)arena<void>(0x400);
                       *(int32_t *)(self + 0x1084) = c, *(int32_t *)(self + 0x1080) = c + r.range(0, 3);
                       *(uint32_t *)(self + 0x1088) = (uint32_t)(uintptr_t)arena<void>(0x300);
                       for (int k = 0; k < 4; k++) arena<int32_t>(0x300)[k] = r.range(0, 4);
                       *(int32_t *)(self + 0x1090) = r.range(0, 3);
                       arena<int32_t>(0x60)[0] = r.range(0, 2) ? 0 : r.range(1, 5);
                   }
               },
               [at](void *fn) {
                   uint32_t *v = arena<uint32_t>(0x60);
                   if (at == 0xa704f0u)
                       return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, const void *, const void *, uint32_t, uint32_t))fn)(arena<void>(0x1000), arena<void>(0x40), arena<void>(0x50), v[1], v[2])};
                   return Result{(uint32_t)((int32_t(__thiscall *)(void *, const void *, const void *, int32_t))fn)(arena<void>(0x1000), arena<void>(0x40), arena<void>(0x50), (int32_t)v[0])};
               }};
        x.prepare = [](const rewrite::Entry *e) { stubs_for_now({{e->at == 0xa704f0u ? 0xa6ff20u : 0xafd3b0u, (void *)h2_list_grow}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Car::Count4FF580: lists of 0-9 slots with room up to 16, n from -2 to 20; the grow stood in for
    {
        Test x{0x4ff580,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   const int32_t c = r.range(0, 9);
                   *(uint32_t *)(self + 0x198) = (uint32_t)(uintptr_t)arena<void>(0x400);
                   *(int32_t *)(self + 0x1a0) = c, *(int32_t *)(self + 0x19c) = c + r.range(0, 7);
                   for (int k = 0; k < 0x20; k++) arena<float>(0x400)[k] = r.real(-9, 9);
                   arena<int32_t>(0x40)[0] = r.range(-2, 20), arena<float>(0x40)[1] = r.real(-5, 5);
                   arena<uint32_t>(0x90)[0] = arena<uint32_t>(0x90)[1] = 0;
               },
               [](void *fn) {
                   return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, int32_t, float))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0], arena<float>(0x40)[1])};
               }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x423da0, (void *)h2_list_grow}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Model::Part2CC6310: times before and up to four lengths past the first track's length, 0-3 tracks each with 1-4 keys
    // (the real Keys::Find and Find2)
    Test model_part2_cc6310{0xcc6310,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100), *model = arena<uint8_t>(0x180);
                                uint32_t *lists = arena<uint32_t>(0x1c0);
                                *(uint32_t *)self = (uint32_t)(uintptr_t)model, *(uint32_t *)(model + 8) = (uint32_t)(uintptr_t)lists;
                                *(uint32_t *)(self + 0xc) = (uint32_t)(uintptr_t)arena<void>(0x200);
                                *(uint32_t *)(self + 0x10) = (uint32_t)(uintptr_t)arena<void>(0x240);
                                *(uint32_t *)(self + 0x14) = (uint32_t)(uintptr_t)arena<void>(0x280);
                                for (int k = 0; k < 0x30; k++) arena<uint32_t>(0x200)[k] = r.next();
                                const int32_t n = r.range(0, 3);
                                uint8_t *list = arena<uint8_t>(0x300);
                                lists[n] = (uint32_t)(uintptr_t)list;
                                const float len = r.real(0.5f, 3);
                                *(uint32_t *)list = (uint32_t)(uintptr_t)arena<void>(0x4f0), arena<float>(0x4f8)[0] = len;
                                const int32_t tracks = r.range(0, 3);
                                *(int32_t *)(list + 4) = tracks, *(uint32_t *)(list + 0xc) = (uint32_t)(uintptr_t)arena<void>(0x340);
                                for (int i = 0; i < tracks; i++) {
                                    uint8_t *tr = arena<uint8_t>(0x340 + i * 0x14);
                                    *(uint32_t *)tr = r.range(0, 15);
                                    for (int s = 0; s < 2; s++) {
                                        float *keys = arena<float>(0x500 + (i * 2 + s) * 0x40);
                                        const int32_t kc = r.range(1, 4);
                                        float time = r.real(-0.5f, 0);
                                        for (int k = 0; k < kc; k++) keys[k * 4] = time, time += r.real(0.1f, 1);
                                        *(int32_t *)(tr + 4 + s * 8) = kc, *(uint32_t *)(tr + 8 + s * 8) = (uint32_t)(uintptr_t)keys;
                                    }
                                }
                                arena<float>(0x280 + n * 8 + 4)[0] = r.range(0, 4) ? r.real(0, len * 4) : len;
                                arena<int32_t>(0x40)[0] = n;
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0]);
                                return Result{0};
                            }};
    quick(model_part2_cc6310);
    // Water::Cell938CD0: 4-wide grids with cells made or not (0xFFFF), random steps, points lists full or not; the grow
    // stood in for
    {
        Test x{0x938cd0,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   for (int k = 0; k < 0x3c; k += 4) *(uint32_t *)(self + k) = r.next();
                   *(uint32_t *)(self + 4) = (uint32_t)(uintptr_t)arena<void>(0x180);
                   for (int k = 0; k < 8; k++) arena<float>(0x180)[k] = r.real(-50, 50);
                   *(int32_t *)(self + 0x30) = 4, *(uint32_t *)(self + 0x38) = (uint32_t)(uintptr_t)arena<void>(0x200);
                   for (int k = 0; k < 16; k++) arena<uint16_t>(0x200)[k] = r.range(0, 1) ? 0xffff : (uint16_t)r.range(0, 9);
                   const int32_t c = r.range(0, 4);
                   *(uint32_t *)(self + 0x10) = (uint32_t)(uintptr_t)arena<void>(0x400);
                   *(int32_t *)(self + 0xc) = c, *(int32_t *)(self + 0x14) = r.range(0, 1) ? c : c + r.range(1, 3), *(int32_t *)(self + 8) = r.range(1, 8);
                   for (int k = 0; k < 0x20; k++) arena<uint32_t>(0x400)[k] = r.next();
                   arena<uint32_t>(0x40)[0] = r.next() & ~0xffu | r.range(0, 3), arena<uint32_t>(0x40)[1] = r.next() & ~0xffu | r.range(0, 3);
                   arena<uint32_t>(0x90)[0] = arena<uint32_t>(0x90)[1] = 0;
               },
               [](void *fn) {
                   return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t))fn)(arena<void>(0x100), arena<uint32_t>(0x40)[0], arena<uint32_t>(0x40)[1])};
               }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x417a20, (void *)h2_list_grow}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Track::PassBB7580: parts of 0-4 entries with a key table or not, slots 0-7, random values; the round stood in for
    {
        Test x{0xbb7580,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100), *part = arena<uint8_t>(0x180), *slots = arena<uint8_t>(0x1c0);
                   *(uint32_t *)(self + 0x10) = (uint32_t)(uintptr_t)part, *(uint32_t *)(self + 0x3c) = (uint32_t)(uintptr_t)slots;
                   *(uint32_t *)(slots + 0xc) = (uint32_t)(uintptr_t)arena<void>(0x1e0);
                   for (int k = 0; k < 8; k++) arena<uint16_t>(0x1e0)[k] = (uint16_t)r.range(0, 7);
                   *(uint16_t *)(self + 0x3a) = (uint16_t)r.range(0, 2), *(float *)(self + 0x34) = r.real(0, 1);
                   const uint32_t n = r.range(0, 4);
                   *(uint32_t *)(part + 0x24) = n;
                   *(uint32_t *)(part + 0x34) = (uint32_t)(uintptr_t)arena<void>(0x400);
                   for (int k = 0; k < 0x40; k++) arena<float>(0x400)[k] = r.real(-20, 20);
                   *(uint32_t *)(part + 0x44) = r.range(0, 1) ? (uint32_t)(uintptr_t)arena<void>(0x500) : 0;
                   for (int k = 0; k < 8; k++) arena<uint16_t>(0x500)[k * 2] = (uint16_t)r.range(0, 9), arena<uint16_t>(0x500)[k * 2 + 1] = (uint16_t)r.range(0, 6);
                   *(uint32_t *)(part + 0x54) = (uint32_t)(uintptr_t)arena<void>(0x580);
                   arena<int32_t>(0x80)[0] = r.range(0, 2), arena<float>(0x80)[1] = r.real(0, 1);
                   for (int k = 0; k < 3; k++) arena<uint32_t>(0x70)[k] = 0;
                   for (int k = 0; k < 8; k++) arena<uint32_t>(0x40)[k] = r.next();
               },
               [](void *fn) {
                   ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x40));
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0xbb69e0, (void *)h2_track_round}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Water::Step9733F0: sorted lists of 0-7 keys (rising first values, the rest alike, junk past the flag bytes), the key
    // sought one of them or between (the real Water::Step973040)
    Test water_step_9733f0{0x9733f0,
                           [=](Rng &r) {
                               uint8_t *self = arena<uint8_t>(0x100);
                               uint32_t *ptrs = arena<uint32_t>(0x700);
                               for (int k = 0; k < 4; k++) arena<int32_t>(0x780)[k] = 0;
                               uint32_t a = r.range(0, 3);
                               for (int i = 0; i < 8; i++) {
                                   uint8_t *e = arena<uint8_t>(0x800 + i * 0x40);
                                   for (int k = 0; k < 0x28; k += 4) *(uint32_t *)(e + k) = 0;
                                   *(uint32_t *)e = a, e[0x19] = (uint8_t)r.next(), e[0x25] = (uint8_t)r.next();
                                   a += r.range(1, 3);
                                   ptrs[i] = (uint32_t)(uintptr_t)e;
                               }
                               *(uint32_t *)self = (uint32_t)(uintptr_t)ptrs, *(int32_t *)(self + 4) = r.range(0, 7);
                               uint32_t *q = arena<uint32_t>(0x40);
                               q[0] = r.range(0, 24), q[1] = 0, q[2] = (uint32_t)(uintptr_t)arena<void>(0x780), q[3] = 0;
                               q[4] = r.next() & ~0xffu, q[5] = 0, q[6] = 0, q[7] = r.next() & ~0xffu;
                               arena<uint32_t>(0x60)[0] = r.next();
                           },
                           [](void *fn) {
                               struct Key8 {
                                   uint32_t v[8];
                               } k;
                               memcpy(&k, arena<void>(0x40), sizeof k);
                               return Result{((uint8_t(__thiscall *)(void *, Key8, void *))fn)(arena<void>(0x100), k, arena<void>(0x60))};
                           }};
    quick(water_step_9733f0);
    // Draw::Step470EB0: random objects, places, directions and values (the real Matrix::LookAlongZSide)
    Test draw_step_470eb0{0x470eb0,
                          [=](Rng &r) {
                              uint8_t *obj = arena<uint8_t>(0x1000);
                              for (int k = 0x130; k < 0x1c0; k += 4) *(uint32_t *)(obj + k) = r.next();
                              for (int k = 0; k < 3; k++) arena<uint32_t>(0x40)[k] = r.next(), arena<float>(0x50)[k] = r.real(-1, 1);
                              arena<uint32_t>(0x60)[0] = r.next(), arena<uint32_t>(0x60)[1] = r.next();
                          },
                          [](void *fn) {
                              return Result{((uint32_t(__stdcall *)(void *, const void *, const void *, uint32_t, uint32_t))fn)(
                                  arena<void>(0x1000), arena<void>(0x40), arena<void>(0x50), arena<uint32_t>(0x60)[0], arena<uint32_t>(0x60)[1])};
                          }};
    quick(draw_step_470eb0);
    // Person::Add7E6DD0: 0-6 records on a valid heap (built by pushing in turn), full or not, with growth steps 0-3; the new
    // record's key below, among or above theirs; the grows stood in for
    {
        Test x{0x7e6dd0,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   const int32_t c = r.range(0, 6);
                   *(int32_t *)self = r.range(0, 3);
                   *(uint32_t *)(self + 4) = (uint32_t)(uintptr_t)arena<void>(0x400);
                   *(int32_t *)(self + 0xc) = c, *(int32_t *)(self + 8) = r.range(0, 1) ? c : c + r.range(1, 3);
                   *(uint32_t *)(self + 0x10) = (uint32_t)(uintptr_t)arena<void>(0x300);
                   *(int32_t *)(self + 0x18) = c, *(int32_t *)(self + 0x14) = r.range(0, 1) ? c : c + r.range(1, 3);
                   for (int k = 0; k < 0x40; k++) arena<uint32_t>(0x400)[k] = r.next();
                   int32_t *heap = arena<int32_t>(0x300);
                   for (int n = 0; n < c; n++) {
                       const float key = (float)r.range(0, 8);
                       arena<float>(0x400 + n * 20 + 8)[0] = key;
                       int32_t i = n;
                       while (i > 0 && key < arena<float>(0x400 + heap[(i - 1) >> 1] * 20 + 8)[0]) heap[i] = heap[(i - 1) >> 1], i = (i - 1) >> 1;
                       heap[i] = n;
                   }
                   for (int k = 0; k < 5; k++) arena<uint32_t>(0x40)[k] = r.next();
                   arena<float>(0x40)[2] = (float)r.range(0, 8);
                   arena<uint32_t>(0x90)[0] = arena<uint32_t>(0x90)[1] = 0;
               },
               [](void *fn) { return Result{(uint32_t)((int32_t(__thiscall *)(void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40))}; }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x7e5400, (void *)h2_list_grow}, {0x417c20, (void *)h2_list_grow}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Fireworks::Find9F9DA0: pools of room 0-6 (empty ones grow to 200), full or not, free lists of item numbers 0-7, c with
    // bit 2 or not and its list full or not; the pool's and list's grows stood in for
    {
        Test x{0x9f9da0,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   uint32_t *pool = (uint32_t *)(self + 0x10), *free_list = arena<uint32_t>(0x1000);
                   const int32_t room = r.range(0, 6), used = r.range(0, 3) ? r.range(0, room) : room;
                   pool[0] = (uint32_t)(uintptr_t)arena<void>(0x2000), pool[1] = (uint32_t)(uintptr_t)free_list, pool[2] = used, pool[3] = room;
                   for (int k = 0; k < 8; k++) free_list[k] = r.range(0, 7);
                   for (int k = 0; k < 0x120; k++) arena<uint32_t>(0x2000)[k] = r.next();
                   const int32_t c = r.range(0, 3);
                   *(uint32_t *)(self + 0x24) = (uint32_t)(uintptr_t)arena<void>(0x300);
                   *(int32_t *)(self + 0x2c) = c, *(int32_t *)(self + 0x28) = r.range(0, 1) ? c : c + r.range(1, 3);
                   arena<uint32_t>(0x98)[0] = arena<uint32_t>(0x98)[1] = 0, arena<uint32_t>(0x90)[0] = arena<uint32_t>(0x90)[1] = 0;
                   for (int k = 0; k < 5; k++) arena<uint32_t>(0x40)[k] = r.next();
               },
               [](void *fn) {
                   uint32_t *v = arena<uint32_t>(0x40);
                   return Result{((uint32_t(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))fn)(arena<void>(0x100), v[0], v[1], v[2], v[3], v[4])};
               }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x9f98a0, (void *)h2_pool_grow}, {0x417c20, (void *)h2_list_grow}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Sky::Find431E30: pools of room 0-6 (empty ones grow to 200), full or not, free lists of item numbers 0-7; the pool's
    // grow and the tree's insert stood in for
    {
        Test x{0x431e30,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x200);
                   uint32_t *pool = (uint32_t *)(self + 0x40), *free_list = arena<uint32_t>(0x1000);
                   const int32_t room = r.range(0, 6), used = r.range(0, 3) ? r.range(0, room) : room;
                   pool[0] = (uint32_t)(uintptr_t)arena<void>(0x2000), pool[1] = (uint32_t)(uintptr_t)free_list, pool[2] = used, pool[3] = room;
                   for (int k = 0; k < 8; k++) free_list[k] = r.range(0, 7);
                   for (int k = 0; k < 0x60; k++) arena<uint32_t>(0x2000)[k] = r.next();
                   *(uint32_t *)(self + 0x50) = r.next();
                   for (int k = 0; k < 7; k++) arena<uint32_t>(0x140)[k] = 0;
                   arena<uint32_t>(0x140)[7] = r.next();
                   arena<uint32_t>(0x98)[0] = arena<uint32_t>(0x98)[1] = 0;
               },
               [](void *fn) { return Result{((uint32_t(__thiscall *)(void *))fn)(arena<void>(0x200))}; }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x4303c0, (void *)h2_pool_grow}, {0x4311c0, (void *)h2_tree_insert}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Module::CheckBE40D0: on or not, parts with flags 2 and 0x2000 in all four ways (the game's debug place put back after;
    // the game's _strncpy really used)
    {
        static uint8_t saved_place[0x90];
        Test x{0xbe40d0,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100), *holder = arena<uint8_t>(0x140), *part = arena<uint8_t>(0x200);
                   *(uint32_t *)self = (uint32_t)(uintptr_t)holder, *(uint32_t *)(holder + 8) = (uint32_t)(uintptr_t)part;
                   self[0xc] = (uint8_t)(r.range(0, 4) ? 1 : 0), *(uint32_t *)(self + 8) = r.next();
                   *(uint32_t *)(part + 0x10) = r.next() & ~0x2002u | (r.range(0, 1) ? 2 : 0) | (r.range(0, 1) ? 0x2000 : 0);
                   *(uint32_t *)(part + 0xc4) = r.next();
                   *(uint32_t *)0x13d28c0 = 0, *(uint32_t *)0x10f2e60 = 0, *(uint8_t *)0x13d28c8 = 0, *(uint8_t *)0x13d2947 = 1;
               },
               [](void *fn) {
                   const uint8_t on = ((uint8_t(__thiscall *)(void *))fn)(arena<void>(0x100));
                   Result x{on};
                   x.f = *(uint32_t *)0x13d28c0 + *(uint32_t *)0x10f2e60 * 3 + *(uint8_t *)0x13d28c8 * 7 + *(uint8_t *)0x13d2947;
                   return x;
               }};
        x.prepare = [](const rewrite::Entry *) {
            memcpy(saved_place, (void *)0x13d28c0, 0x88), memcpy(saved_place + 0x88, (void *)0x10f2e60, 4);
        };
        x.finish = [](const rewrite::Entry *) { memcpy((void *)0x13d28c0, saved_place, 0x88), memcpy((void *)0x10f2e60, saved_place + 0x88, 4); };
        quick(x);
    }
    // Regions::Find805F60: maps of 0-6 names, found or not at each place; Names::Find stood in for
    {
        Test x{0x805f60,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   const int32_t c = r.range(0, 6);
                   *(uint32_t *)self = (uint32_t)(uintptr_t)arena<void>(0x400), *(uint32_t *)(self + 8) = (uint32_t)(uintptr_t)arena<void>(0x500);
                   *(int32_t *)(self + 0x14) = c;
                   for (int k = 0; k < 8; k++) arena<uint32_t>(0x400)[k] = r.next(), arena<uint32_t>(0x500)[k] = r.next();
                   arena<uint32_t>(0x170)[0] = r.range(0, 1) && c ? 1 : 0, arena<int32_t>(0x170)[1] = c ? r.range(0, c - 1) : 0;
                   for (int k = 0; k < 4; k++) arena<uint32_t>(0x160)[k] = 0;
                   arena<uint32_t>(0x40)[0] = r.next();
               },
               [](void *fn) {
                   ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), (void *)(uintptr_t)arena<uint32_t>(0x40)[0]);
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x805c10, (void *)h2_names_find}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Guest::Grow799A40: lists full or not, 0-5 objects (none at times) either side of the plane (on it at times); the
    // interface (the object itself) and grow stood in for
    {
        Test x{0x799a40,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x1000);
                   const int32_t c = r.range(0, 4);
                   *(uint32_t *)(self + 0x108) = (uint32_t)(uintptr_t)arena<void>(0x400);
                   *(int32_t *)(self + 0x110) = c, *(int32_t *)(self + 0x10c) = c + r.range(0, 6);
                   for (int k = 0; k < 0x10; k++) arena<uint32_t>(0x400)[k] = r.next();
                   for (int k = 0; k < 3; k++) *(float *)(self + 0x120 + k * 4) = r.real(-5, 5), *(float *)(self + 0x130 + k * 4) = r.real(-1, 1);
                   const int32_t n = r.range(0, 5);
                   for (int i = 0; i < 6; i++) {
                       uint8_t *obj = arena<uint8_t>(0x2000 + i * 0x100);
                       for (int k = 0; k < 3; k++) *(float *)(obj + 0x80 + k * 4) = r.range(0, 7) ? r.real(-10, 10) : *(float *)(self + 0x120 + k * 4);
                       arena<uint32_t>(0x300)[i] = r.range(0, 5) ? (uint32_t)(uintptr_t)obj : 0;
                   }
                   arena<int32_t>(0x40)[0] = n;
                   arena<uint32_t>(0x90)[0] = arena<uint32_t>(0x90)[1] = 0;
               },
               [](void *fn) {
                   ((void(__thiscall *)(void *, const void *, int32_t))fn)(arena<void>(0x1000), arena<void>(0x300), arena<int32_t>(0x40)[0]);
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x4712b0, (void *)h2_same_object}, {0x7735e0, (void *)h2_list_grow}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Curves::StepC21520: new steps and the same step, weights either side of 1 (exactly 1 at times), t either side of 0,
    // small random rotations (the real Quaternion::Slerp)
    Test curves_step_c21520{0xc21520,
                            [=](Rng &r) {
                                uint8_t *self = arena<uint8_t>(0x100);
                                for (int k = 0; k < 3; k++) *(float *)(self + 0x10 + k * 4) = r.real(-1, 1), *(float *)(self + 0x20 + k * 4) = r.real(-1, 1);
                                for (int k = 0; k < 3; k++) arena<float>(0x40)[k] = r.real(-1, 1);
                                *(float *)(self + 0x30) = r.range(0, 5) ? r.real(0, 2) : 1.0f;
                                *(int32_t *)(self + 0x34) = r.range(0, 3);
                                arena<int32_t>(0x60)[0] = r.range(0, 4), arena<float>(0x60)[1] = r.range(0, 5) ? r.real(-0.5f, 1) : 0.0f;
                            },
                            [](void *fn) {
                                ((void(__thiscall *)(void *, const void *, int32_t, float))fn)(arena<void>(0x100), arena<void>(0x40), arena<int32_t>(0x60)[0], arena<float>(0x60)[1]);
                                return Result{0};
                            }};
    quick(curves_step_c21520);
    // SceneryBehaviour::Kind81A760: kinds 0-6 (the game's tables), with an effect or none; the reference stood in for
    {
        Test x{0x81a760,
               [=](Rng &r) {
                   uint8_t *self = arena<uint8_t>(0x100);
                   for (int k = 0x60; k < 0x74; k += 4) *(uint32_t *)(self + k) = r.next();
                   *(uint32_t *)(self + 0x64) = r.range(0, 3) ? (uint32_t)(uintptr_t)arena<void>(0x200) : 0;
                   for (int k = 0; k < 0x14; k++) arena<uint32_t>(0x200)[k] = r.next();
                   for (int k = 0; k < 3; k++) *(float *)(arena<uint8_t>(0x180) + 0x120 + k * 4) = r.real(-500, 500);
                   arena<uint32_t>(0xf8)[0] = 0;
                   arena<int32_t>(0x40)[0] = r.range(0, 6);
               },
               [](void *fn) {
                   return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, int32_t))fn)(arena<void>(0x100), arena<int32_t>(0x40)[0])};
               }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x833850, (void *)h2_animal_interface}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Skeleton::StepC1F960: 0-5 random planes, boxes and directions, spans of random ends (the real
    // Plane::BoxCornerDistance)
    Test skeleton_step_c1f960{0xc1f960,
                              [=](Rng &r) {
                                  uint8_t *self = arena<uint8_t>(0x100);
                                  *(int32_t *)(self + 0xd0) = r.range(0, 5);
                                  for (int i = 0; i < 5; i++) {
                                      float *p = (float *)(self + 0x10 + i * 32);
                                      for (int k = 0; k < 3; k++) p[k] = r.real(-1, 1);
                                      p[4] = r.real(-5, 5);
                                  }
                                  for (int k = 0; k < 3; k++) arena<float>(0x40)[k] = r.real(-3, 0), arena<float>(0x50)[k] = r.real(0, 3), arena<float>(0x60)[k] = r.real(-1, 1);
                                  arena<float>(0x70)[0] = r.real(-20, 5), arena<float>(0x70)[1] = r.real(-5, 20);
                              },
                              [](void *fn) {
                                  ((void(__thiscall *)(void *, const void *, const void *, const void *, float *, float *))fn)(
                                      arena<void>(0x100), arena<void>(0x40), arena<void>(0x50), arena<void>(0x60), arena<float>(0x70), arena<float>(0x70) + 1);
                                  return Result{0};
                              }};
    quick(skeleton_step_c1f960);
    // Category::Clear: random categories; the text assign and the list resize stood in for (they note their this and
    // argument)
    {
        Test x{0x6bf800,
               [=](Rng &r) {
                   for (int k = 0; k < 0x20; k++) arena<uint32_t>(0x100)[k] = r.next();
                   for (int k = 0; k < 5; k++) arena<uint32_t>(0xb0)[k] = 0;
               },
               [](void *fn) {
                   ((void(__thiscall *)(void *))fn)(arena<void>(0x100));
                   return Result{0};
               }};
        x.prepare = [](const rewrite::Entry *) { stubs_for_now({{0x40a6e0, (void *)h2_slot_free}, {0x6bf490, (void *)h2_block_mark}}); };
        x.finish = [](const rewrite::Entry *) { unstub_many(); };
        quick(x);
    }
    // Math::CosineRuleC23650: triangles of random sides, flat ones (cosines of exactly -1 and 1) and impossible ones past
    // both ends (the game's acos only ever sees -1..1)
    Test math_cosinerule_c23650{0xc23650,
                                [=](Rng &r) {
                                    float *s = arena<float>(0x40);
                                    s[1] = r.real(0.5f, 10), s[2] = r.real(0.5f, 10);
                                    const int k = r.range(0, 5);
                                    s[0] = k == 0 ? s[1] + s[2] : k == 1 ? fabsf(s[1] - s[2]) : k == 2 ? s[1] + s[2] + r.real(0.1f, 5) : r.real(fabsf(s[1] - s[2]), s[1] + s[2]);
                                    if (k == 3 && r.range(0, 1)) s[0] = r.real(0, 0.2f);
                                },
                                [](void *fn) {
                                    Result x;
                                    float *s = arena<float>(0x40);
                                    x.f = ((long double(__stdcall *)(float, float, float))fn)(s[0], s[1], s[2]);
                                    return x;
                                }};
    quick(math_cosinerule_c23650);
    // Effect::Init40BB90: random points, ups and directions (the real Matrix::LookAlongZSide)
    Test effect_init_40bb90{0x40bb90,
                            [=](Rng &r) {
                                for (int k = 0; k < 0x10; k++) arena<uint32_t>(0x100)[k] = r.next();
                                for (int k = 0; k < 12; k++) arena<float>(0x40)[k] = r.real(-1, 1);
                            },
                            [](void *fn) { return Result{(uint32_t)(uintptr_t)((void *(__thiscall *)(void *, const void *))fn)(arena<void>(0x100), arena<void>(0x40))}; }};
    quick(effect_init_40bb90);
#endif
