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
#endif
