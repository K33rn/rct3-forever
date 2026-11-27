// Tests for forever/src/rewrite/hand2.cpp (hand-written rewrites, one at a time).
#if EQUIV_PART == 1
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
#endif
