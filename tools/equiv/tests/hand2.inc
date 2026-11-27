// Tests for forever/src/rewrite/hand2.cpp (hand-written rewrites, one at a time).
#if EQUIV_PART == 1
// a stand-in for the game's _tolower (0xDDE005), which needs the C library's per-thread data the tester doesn't have
static int __cdecl h2_lower(int c) { return c >= 'A' && c <= 'Z' ? c + 32 : c ^ (c >> 8 & 0x55); }
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
                             [](void *fn) { ((void(__thiscall *)(void *, void *, int32_t))fn)(arena<void>(0x100), arena<void>(0x20), arena<int32_t>(0x10)[0]); return Result{}; }};
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
                            [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x20)); return Result{}; }};
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
#endif
