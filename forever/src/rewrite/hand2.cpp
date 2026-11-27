// More game functions written by hand, one at a time (each from what it does: see its comment).
#include "wave.h"

// ---- Inflate::BuildHuffmanTable (0xCBB250): zlib's huft_build ---------------------------------------------------------
// The code lengths b in ecx, the most bits a first table may take (m) in eax; then n, s (values from s on are lengths
// or distances), d (their bases), e (their extra bits), t (the first table, given back), hp (the tables' room), hn (how
// much of it is used) and v (a work list of n values); the callee takes the 8 off the stack. As zlib 1.1.4 (inftrees.c):
// codes counted by length, the first and last lengths found (m kept between them), the last length's count topped up to
// make a complete code (-3 when over-full), the values sorted by length into v, then tables of up to m bits filled from
// the shortest codes on, each longer table linked from the one before. Entries are {extra-ops byte, bits byte, base}.
// 0 when complete (or every length 0: t and m given 0), -5 when incomplete with longest length over 1, -4 when the room
// (1440 entries) runs out. Each entry's two bytes after bits are left alone (the game copies stack bytes there).
namespace {
struct Huft {
    uint8_t exop, bits;
    uint16_t pad;
    uint32_t base;
};
}
static volatile long g_huft_builds;
extern "C" int32_t __stdcall forever_huft_build(const uint32_t *b, uint32_t *m, uint32_t n, uint32_t s, const uint32_t *d, const uint32_t *e,
                                                Huft **t, Huft *hp, uint32_t *hn, uint32_t *v) {
    g_huft_builds++;
    uint32_t c[16] = {}, x[16], i, j, a, f, z = 0, mask;
    Huft *u[15], *q = nullptr;
    uint32_t r_word = 0, r_base = 0;   // the entry being built: exop in the low byte, bits in the next
    int32_t k, g, h, l, w, y;
    const uint32_t *p = b;
    i = n;
    do c[*p++]++;
    while (--i);
    if (c[0] == n) {
        *t = nullptr;
        *m = 0;
        return 0;
    }
    l = (int32_t)*m;
    for (j = 1; j <= 15; j++)
        if (c[j]) break;
    k = (int32_t)j;
    if ((uint32_t)l < j) l = (int32_t)j;
    for (i = 15; i; i--)
        if (c[i]) break;
    g = (int32_t)i;
    if ((uint32_t)l > i) l = (int32_t)i;
    *m = (uint32_t)l;
    for (y = 1 << j; j < i; j++, y <<= 1)
        if ((y -= (int32_t)c[j]) < 0) return -3;
    if ((y -= (int32_t)c[i]) < 0) return -3;
    c[i] += (uint32_t)y;
    x[1] = j = 0;
    {
        const uint32_t *pc = c + 1;
        uint32_t *xp = x + 2;
        while (--i) *xp++ = (j += *pc++);
    }
    p = b;
    i = 0;
    do {
        if ((j = *p++) != 0) v[x[j]++] = i;
    } while (++i < n);
    n = x[g];
    x[0] = i = 0;
    p = v;
    h = -1;
    w = -l;
    u[0] = nullptr;
    for (; k <= g; k++) {
        a = c[k];
        while (a--) {
            while (k > w + l) {
                h++;
                w += l;
                z = (uint32_t)(g - w);
                z = z > (uint32_t)l ? (uint32_t)l : z;
                if ((f = 1u << (j = (uint32_t)(k - w))) > a + 1) {
                    f -= a + 1;
                    const uint32_t *xp = c + k;
                    if (j < z)
                        while (++j < z) {
                            if ((f <<= 1) <= *++xp) break;
                            f -= *xp;
                        }
                }
                z = 1u << j;
                if (*hn + z > 1440) return -4;
                u[h] = q = hp + *hn;
                *hn += z;
                if (h) {
                    x[h] = i;
                    r_word = (uint32_t)(uint8_t)j | (uint32_t)(uint8_t)l << 8;
                    j = i >> (w - l);
                    r_base = (uint32_t)(q - u[h - 1]) - j;
                    Huft *to = &u[h - 1][j];
                    to->exop = (uint8_t)r_word, to->bits = (uint8_t)(r_word >> 8), to->base = r_base;
                } else {
                    *t = q;
                }
            }
            uint32_t exop;
            if (p >= v + n) {
                exop = 0xc0;
            } else if (*p < s) {
                exop = *p < 256 ? 0 : 0x60;
                r_base = *p++;
            } else {
                exop = (uint8_t)(e[*p - s] + 0x50);
                r_base = d[*p++ - s];
            }
            r_word = exop | (uint32_t)(uint8_t)(k - w) << 8;
            f = 1u << (k - w);
            for (j = i >> w; j < z; j += f) {
                q[j].exop = (uint8_t)r_word, q[j].bits = (uint8_t)(r_word >> 8), q[j].base = r_base;
            }
            for (j = 1u << (k - 1); i & j; j >>= 1) i ^= j;
            i ^= j;
            mask = (1u << w) - 1;
            while ((i & mask) != x[h]) {
                h--;
                w -= l;
                mask = (1u << w) - 1;
            }
        }
    }
    return y != 0 && g != 1 ? -5 : 0;
}
// b comes in ecx and m in eax: both put in front of the 8 stack values, so the function takes all 10 off
extern "C" __attribute__((naked)) void forever_huft_build_regs() {
    __asm__("movl (%esp), %edx\n\tmovl %eax, (%esp)\n\tpushl %ecx\n\tpushl %edx\n\tjmp _forever_huft_build@40");
}
static const uint8_t huft_build_original[] = {0x81, 0xec, 0xfc, 0x00, 0x00, 0x00, 0x53, 0x55};
REWRITE("Inflate::BuildHuffmanTable", 0xcbb250, huft_build_original, (void *)&forever_huft_build_regs, &g_huft_builds);

// ---- Inflate::Fast (0xCC0670): zlib's inflate_fast (1.1.3's inffast.c) ------------------------------------------------
// fastcall: bl and bd (the literal and distance tables' first-level bits), then tl, td, the blocks state s and the stream
// z. While at least 258 bytes of output room and 10 bytes of input are left: a literal/length code (20 bits kept on hand)
// gives a byte, or a length with extra bits and then a distance code (15 bits on hand, then its extra bits); the match is
// copied from the output behind (wrapping round the window, +0x28 to +0x2C). Then the bytes not used are given back to
// the input and the state saved: bits +0x20, count +0x1C, write +0x34; z's next_in +0, avail_in +4, total_in +8. 1 at an
// end-of-block code, -3 for a bad code (z's msg +0x18 set to the game's "invalid distance code" or "invalid
// literal/length code"), else 0. The bit masks are the game's (0x10F4580).
static volatile long g_inflate_fasts;
static int32_t __fastcall inflate_fast(uint32_t bl, uint32_t bd, const Huft *tl, const Huft *td, uint8_t *s, uint8_t *z) {
    g_inflate_fasts++;
    const uint32_t *const mask = (const uint32_t *)0x10f4580;
    const uint8_t *p = *(const uint8_t **)z;
    uint32_t n = *(uint32_t *)(z + 4);
    uint32_t b = *(uint32_t *)(s + 0x20), k = *(uint32_t *)(s + 0x1c);
    uint8_t *q = *(uint8_t **)(s + 0x34);
    uint8_t *const read = *(uint8_t **)(s + 0x30), *const window = *(uint8_t **)(s + 0x28), *const end = *(uint8_t **)(s + 0x2c);
    uint32_t m = (uint32_t)(q < read ? read - q - 1 : end - q);
    const uint32_t ml = mask[bl], md = mask[bd];
    int32_t result = 0;
    auto grab = [&](uint32_t bits) {
        while (k < bits) {
            n--;
            b |= (uint32_t)*p++ << k;
            k += 8;
        }
    };
    auto dump = [&](uint32_t bits) {
        b >>= bits;
        k -= bits;
    };
    do {
        grab(20);
        const Huft *t = tl + (b & ml);
        uint32_t e = t->exop;
        if (e == 0) {
            dump(t->bits);
            *q++ = (uint8_t)t->base;
            m--;
            continue;
        }
        for (;;) {
            dump(t->bits);
            if (e & 16) {
                e &= 15;
                uint32_t c = t->base + (b & mask[e]);
                dump(e);
                grab(15);
                t = td + (b & md);
                e = t->exop;
                for (;;) {
                    dump(t->bits);
                    if (e & 16) {
                        e &= 15;
                        grab(e);
                        const uint32_t d = t->base + (b & mask[e]);
                        dump(e);
                        m -= c;
                        const uint8_t *r;
                        if ((uint32_t)(q - window) >= d) {
                            r = q - d;
                            *q++ = *r++, c--;
                            *q++ = *r++, c--;
                        } else {
                            e = d - (uint32_t)(q - window);
                            r = end - e;
                            if (c > e) {
                                c -= e;
                                do *q++ = *r++;
                                while (--e);
                                r = window;
                            }
                        }
                        do *q++ = *r++;
                        while (--c);
                        break;
                    }
                    if (e & 64) {
                        *(uint32_t *)(z + 0x18) = 0xfc0ff8;   // "invalid distance code"
                        result = -3;
                        goto done;
                    }
                    t += t->base;
                    t += b & mask[e];
                    e = t->exop;
                }
                break;
            }
            if (e & 64) {
                if (e & 32) {
                    result = 1;
                } else {
                    *(uint32_t *)(z + 0x18) = 0xfc1010;   // "invalid literal/length code"
                    result = -3;
                }
                goto done;
            }
            t += t->base;
            t += b & mask[e];
            e = t->exop;
            if (e == 0) {
                dump(t->bits);
                *q++ = (uint8_t)t->base;
                m--;
                break;
            }
        }
    } while (m >= 258 && n >= 10);
done:
    uint32_t c = *(uint32_t *)(z + 4) - n;
    if (k >> 3 < c) c = k >> 3;
    n += c;
    p -= c;
    k -= c << 3;
    *(uint32_t *)(s + 0x20) = b;
    *(uint32_t *)(s + 0x1c) = k;
    *(uint32_t *)(z + 4) = n;
    *(uint32_t *)(z + 8) += (uint32_t)(p - *(const uint8_t **)z);
    *(const uint8_t **)z = p;
    *(uint8_t **)(s + 0x34) = q;
    return result;
}
static const uint8_t inflate_fast_original[] = {0x83, 0xec, 0x18, 0x53, 0x8b, 0x5c, 0x24, 0x28};
REWRITE("Inflate::Fast", 0xcc0670, inflate_fast_original, &inflate_fast, &g_inflate_fasts);

// ---- ResourceFile::Load(this, a, header) (0xCCDD90): a resource file set up over its loaded header -------------------
// this: table 0xFC33E4, a at +4, the header at +8; gives this. When the file is the other byte order (the flag byte
// 0x13D0DF2, read again before each value) the header's values are turned round: +0, +4, +0xC, +0x14 (the count), +0x24,
// +0x28, the count's entries in the list at +0x18, then the record at +0x10: +0x38, +0x40, +0x44, the 16-bit values at
// +0x48 and +0x4A (each written back as 32 bits, so the two bytes after each are cleared, flag or not), +0x4C, +0x50,
// +0x5C to +0x68, and +0 to +0x18 and +0x20 to +0x28.
static volatile long g_resource_loads;
static inline void resource_swap(volatile uint32_t *p) {
    const uint32_t v = *p;
    *p = *(volatile uint8_t *)0x13d0df2 ? __builtin_bswap32(v) : v;
}
static inline void resource_swap16(uint8_t *p) {
    const uint16_t v = *(volatile uint16_t *)p;
    *(volatile uint32_t *)p = *(volatile uint8_t *)0x13d0df2 ? __builtin_bswap16(v) : v;
}
static uint8_t *__thiscall resource_file_load(uint8_t *self, uint32_t a, uint8_t *header) {
    g_resource_loads++;
    volatile uint32_t *const t = (volatile uint32_t *)self;
    t[1] = a, t[2] = (uint32_t)(uintptr_t)header, t[0] = 0xfc33e4;
    if (!*(volatile uint8_t *)0x13d0df2) return self;
    volatile uint32_t *const h = (volatile uint32_t *)header;
    resource_swap(h + 0), resource_swap(h + 1), resource_swap(h + 3), resource_swap(h + 5), resource_swap(h + 9), resource_swap(h + 10);
    if ((int32_t)h[5] > 0)
        for (int32_t i = 0; i < (int32_t)h[5]; i++) resource_swap((volatile uint32_t *)(uintptr_t)h[6] + i);
    auto record = [&]() { return (uint8_t *)(uintptr_t)h[4]; };
    static const uint32_t first[] = {0x38, 0x40, 0x44}, rest[] = {0x4c, 0x50, 0x5c, 0x60, 0x64, 0x68, 0, 4, 8, 0xc, 0x10, 0x14, 0x18, 0x20, 0x24, 0x28};
    for (uint32_t at : first) resource_swap((volatile uint32_t *)(record() + at));
    resource_swap16(record() + 0x48), resource_swap16(record() + 0x4a);
    for (uint32_t at : rest)
        resource_swap((volatile uint32_t *)(record() + at));
    return self;
}
static const uint8_t resource_file_load_original[] = {0x8b, 0x54, 0x24, 0x08, 0x8b, 0xc1, 0x8b, 0x4c};
REWRITE("ResourceFile::Load", 0xccdd90, resource_file_load_original, &resource_file_load, &g_resource_loads);

// ---- VertexDeclaration::FillBDD9A0 / FillBDE5E0(this, a, places, out) (0xBDD9A0, 0xBDE5E0): a record's values packed --
// The record is a's table (+0x38) entry this+0xC (16 bits). A reference is 16 bits: the top two pick a base in places
// (+4 on), the low 14 an offset from it. Written from out's +0x10 plus its 16-bit +0xE on:
//  - for each of the record's +0x14 kinds (bytes at its +4 + 1, every second one): kind 1 takes this+8's next reference's
//    32-bit value, at most 0xFF, as a byte (the reference list moves on for every kind);
//  - then its +0x10 parts (pairs of {kind, many} at its +0), each with this+4's next reference: a run of kind 2 packs one
//    bit per value (not 0) into bytes, low bit first, a part byte written after the run when not full; kind 0x25 writes
//    bytes, kind 0x1D 32-bit values after zero bytes up to a 4-byte boundary. A part that is "many" points at {n, list}
//    and gives n + 1 values (one when n is not above 0). Any other kind is not moved past (the game spins on it).
//  - then a zero byte to an even place and a zero 16-bit value (not counted) to a 4-byte one.
// out's +0xC takes this+0xC and its +0xE grows by the record's +0x18 (16 bits); gives out. FillBDE5E0 first copies
// this's 16-bit +0x16, +0x10, +0x12, +0xC and +0x14 to out's +0, +2, +8, +0xC and +4.
static volatile long g_declaration_fills;
static uint8_t *declaration_fill(const uint8_t *self, const uint8_t *a, const uint32_t *places, uint8_t *out) {
    g_declaration_fills++;
    auto at = [&](uint16_t ref) { return (const uint8_t *)(uintptr_t)(places[(ref >> 14) + 1] + (ref & 0x3fff)); };
    const uint8_t *const record = *(const uint8_t *const *)(*(const uint8_t *const *)(a + 0x38) + 4 * *(const uint16_t *)(self + 0xc));
    volatile uint8_t *d = (uint8_t *)(uintptr_t)(*(const uint32_t *)(out + 0x10) + *(const uint16_t *)(out + 0xe));
    {
        const uint16_t *refs = *(const uint16_t *const *)(self + 8);
        const uint8_t *kind = *(const uint8_t *const *)(record + 4) + 1;
        for (uint32_t n = *(const uint32_t *)(record + 0x14); n; n--, refs++, kind += 2)
            if (*kind == 1) {
                const uint32_t v = *(const uint32_t *)at(*refs);
                *d++ = (uint8_t)(v < 0xff ? v : 0xff);
            }
    }
    const uint16_t *refs = *(const uint16_t *const *)(self + 4);
    const uint8_t *part = *(const uint8_t *const *)record;
    uint32_t left = *(const uint32_t *)(record + 0x10);
    while (left) {
        const uint8_t kind = part[0];
        if (kind == 2) {
            uint8_t bits = 0, bit = 1;
            auto put = [&](uint32_t v) {
                if (v) bits |= bit;
                bit <<= 1;
                if (!bit) *d++ = bits, bits = 0, bit = 1;
            };
            for (;;) {
                const uint8_t *p = at(*refs++);
                if (!part[1]) {
                    put(*(const uint32_t *)p);
                } else {
                    int32_t n = *(const int32_t *)p;
                    const uint32_t *list = *(const uint32_t *const *)(p + 4);
                    do put(*list++);
                    while (n-- > 0);
                }
                if (!--left) break;
                part += 2;
                if (part[0] != 2) break;
            }
            if (bit != 1) *d++ = bits;
        } else if (kind == 0x25 || kind == 0x1d) {
            if (kind == 0x1d)
                while ((uintptr_t)d & 3) *d++ = 0;
            for (;;) {
                const uint8_t *p = at(*refs++);
                if (!part[1]) {
                    if (kind == 0x25) *d++ = *p;
                    else *(volatile uint32_t *)d = *(const uint32_t *)p, d += 4;
                } else {
                    int32_t n = *(const int32_t *)p;
                    const uint32_t *list = *(const uint32_t *const *)(p + 4);
                    do {
                        if (kind == 0x25) *d++ = (uint8_t)*list++;
                        else *(volatile uint32_t *)d = *list++, d += 4;
                    } while (n-- > 0);
                }
                if (!--left) goto done;
                part += 2;
                if (part[0] != kind) break;
            }
        }
    }
done:
    if ((uintptr_t)d & 1) *d++ = 0;
    if ((uintptr_t)d & 2) *(volatile uint16_t *)d = 0;
    *(volatile uint16_t *)(out + 0xc) = *(const uint16_t *)(self + 0xc);
    *(volatile uint16_t *)(out + 0xe) += *(const uint16_t *)(record + 0x18);
    return out;
}
static uint8_t *__thiscall declaration_fill_bdd9a0(const uint8_t *self, const uint8_t *a, const uint32_t *places, uint8_t *out) {
    return declaration_fill(self, a, places, out);
}
static uint8_t *__thiscall declaration_fill_bde5e0(const uint8_t *self, const uint8_t *a, const uint32_t *places, uint8_t *out) {
    volatile uint16_t *const o = (volatile uint16_t *)out;
    const uint16_t *const s = (const uint16_t *)self;
    o[0] = s[0xb], o[1] = s[8], o[4] = s[9], o[6] = s[6], o[2] = s[0xa];
    return declaration_fill(self, a, places, out);
}
static const uint8_t declaration_fill_bdd9a0_original[] = {0x83, 0xec, 0x10, 0x53, 0x55, 0x56, 0x57, 0x8b, 0xf9};
REWRITE("VertexDeclaration::FillBDD9A0", 0xbdd9a0, declaration_fill_bdd9a0_original, &declaration_fill_bdd9a0, &g_declaration_fills);
static const uint8_t declaration_fill_bde5e0_original[] = {0x83, 0xec, 0x10, 0x53, 0x55, 0x56, 0x57, 0x8b, 0xd9};
REWRITE("VertexDeclaration::FillBDE5E0", 0xbde5e0, declaration_fill_bde5e0_original, &declaration_fill_bde5e0, &g_declaration_fills);
