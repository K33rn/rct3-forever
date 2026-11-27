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
                         [](void *fn) { ((void(__thiscall *)(void *, void *))fn)(arena<void>(0x100), arena<void>(0x20)); return Result{}; }};
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
                               ((void(__thiscall *)(void *, void *, int32_t, int32_t, uint32_t))fn)(arena<void>(0x100), arena<void>(0x20), a[0], a[1], (uint32_t)a[2]);
                               return Result{};
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
                              ((void(__stdcall *)(void *, const void *, const void *))fn)(arena<void>(0x400), arena<void>(0x1000), arena<void>(0x40));
                              return Result{0};
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
#endif
