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

// ---- Terrain::Indices8F0D30(this) (0x8F0D30): the triangles joining a patch's outer ring of points to its inner one ---
// this: the outer ring's width +0 and height +4, the inner one's +8 and +0xC, the index list +0x20 (16-bit), the first
// inner point +0x24. Indices are 16-bit. When both rings are the same size (above 1 each way): the n = w + h points of
// the outer ring start 4n before the inner one, and each side (width, height, width, then height less one) gives two
// triangles per step, {o, i + 1, i} and {o, o + 1, i + 1} with o going up by 2 and i by 1; then one closing pair back
// to the first inner point. Otherwise the outer ring starts 2n before; the outer side steps (width, height, width,
// height less one) give {o, o + 1, i} with i going up every second step, then a closing triangle; then the inner side
// steps (w1, h1, w1, h1 less one) give {i, o + 1, i + 1} with o going up by 2, less 1 after each side when the outer
// side before it was odd, and the closing triangle. Gives the place of the last triangle (left in eax).
static volatile long g_terrain_indices;
static uint16_t *__fastcall terrain_indices(const uint8_t *self) {
    g_terrain_indices++;
    const volatile int32_t *const t = (const volatile int32_t *)self;
    volatile uint16_t *o = *(uint16_t *const *)(self + 0x20);
    const volatile uint16_t *const base = (const volatile uint16_t *)(self + 0x24);
    if (t[0] == t[2] && t[1] == t[3] && t[0] > 1 && t[1] > 1) {
        uint16_t s = *base, d = (uint16_t)(s - 4 * (uint32_t)(uint16_t)(t[0] + t[1]));
        auto pair = [&]() {
            o[0] = d, o[1] = (uint16_t)(s + 1), o[2] = s, o[3] = d, o[5] = (uint16_t)(s + 1);
            o += 6;
            o[-2] = (uint16_t)(d + 1);
            d = (uint16_t)(d + 2), s++;
        };
        for (int32_t i = 0; i < t[0]; i++) pair();
        for (int32_t i = 0; i < t[1]; i++) pair();
        for (int32_t i = 0; i < t[0]; i++) pair();
        for (int32_t i = 1; i < t[1]; i++) pair();
        o[0] = d, o[1] = *base, o[3] = d, o[2] = s, o[4] = (uint16_t)(d + 1), o[5] = *base;
        return (uint16_t *)o;
    }
    const uint32_t ring = 2 * (uint32_t)(uint16_t)(t[0] + t[1]);
    uint16_t in = *base, s = (uint16_t)(in - ring);
    auto outer = [&](int32_t j) {
        o[0] = s, s++, o[1] = s, o[2] = in;
        if (!(j & 1)) in++;
        o += 3;
    };
    for (int32_t j = 0; j < t[0]; j++) outer(j);
    for (int32_t j = 0; j < t[1]; j++) outer(j);
    for (int32_t j = 0; j < t[0]; j++) outer(j);
    for (int32_t j = 0; j < t[1] - 1; j++) outer(j);
    o[0] = s, o[1] = (uint16_t)(*base - ring);
    o[2] = (t[1] & 1) ? in : *base;
    o += 3;
    uint16_t d = *base;
    s = (uint16_t)(d - ring);
    auto inner = [&]() {
        o[0] = d, o[1] = (uint16_t)(s + 1), d++, o[2] = d;
        s = (uint16_t)(s + 2);
        o += 3;
    };
    for (int32_t j = 0; j < t[2]; j++) inner();
    if (t[0] & 1) s--;
    for (int32_t j = 0; j < t[3]; j++) inner();
    if (t[1] & 1) s--;
    for (int32_t j = 0; j < t[2]; j++) inner();
    if (t[0] & 1) s--;
    for (int32_t j = 0; j < t[3] - 1; j++) inner();
    o[0] = d;
    if (t[1] & 1) o[1] = (uint16_t)(*base - ring), o[2] = *base;
    else o[1] = (uint16_t)(s + 1), o[2] = *base;
    return (uint16_t *)o;
}
static const uint8_t terrain_indices_original[] = {0x8b, 0x51, 0x08, 0x8b, 0x41, 0x20, 0x53, 0x8b};
REWRITE("Terrain::Indices8F0D30", 0x8f0d30, terrain_indices_original, &terrain_indices, &g_terrain_indices);

// ---- Path::Update71C230(this) (0x71C230): a grid's marked box shrunk to the cells in use, when it is to be (+0xA0) ----
// The box is bytes x0 +0x9C, y0 +0x9D, x1 +0x9E, y1 +0x9F (ends included) over cells (+0x58, row width +0x54 byte). A
// cell is empty when it is null or its +0xA (16 bits), +6 (byte) and +0xC are all 0. When the shared count (0x130634C)
// is not above 0 the box is made empty (0xFF, 0xFF, 0, 0) and nothing else; otherwise the left column, top row, right
// column and bottom row are moved in, each while all its cells (between the other two sides) are empty, and +0xA0 is
// cleared.
static volatile long g_path_updates;
static void __fastcall path_update_71c230(uint8_t *self) {
    g_path_updates++;
    volatile uint8_t *const b = self + 0x9c;
    if (!self[0xa0]) return;
    if (*(volatile int32_t *)0x130634c <= 0) {
        b[0] = 0xff, b[1] = 0xff, b[2] = 0, b[3] = 0;
        return;
    }
    auto used = [&](uint32_t x, uint32_t y) {
        const uint8_t *c = (*(uint8_t *const *const *)(self + 0x58))[y * self[0x54] + x];
        return c && (*(const uint16_t *)(c + 0xa) || c[6] || *(const uint32_t *)(c + 0xc));
    };
    // a line of cells from `from` while not above *to: the first in use found? (bytes, as the game counts)
    auto any = [&](uint8_t from, const volatile uint8_t *to, bool column, uint8_t at) {
        for (uint8_t i = from; i <= *to; i++)
            if (column ? used(at, i) : used(i, at)) return true;
        return false;
    };
    for (; b[0] <= b[2]; b[0]++)
        if (any(b[1], b + 3, true, b[0])) break;
    for (; b[1] <= b[3]; b[1]++)
        if (any(b[0], b + 2, false, b[1])) break;
    for (; b[2] >= b[0]; b[2]--)
        if (any(b[1], b + 3, true, b[2])) break;
    for (; b[3] >= b[1]; b[3]--)
        if (any(b[0], b + 2, false, b[3])) break;
    self[0xa0] = 0;
}
static const uint8_t path_update_71c230_original[] = {0x8a, 0x81, 0xa0, 0x00, 0x00, 0x00, 0x84, 0xc0};
REWRITE("Path::Update71C230", 0x71c230, path_update_71c230_original, &path_update_71c230, &g_path_updates);

// ---- Text::FindBDEB50(this, objects, descriptions, a, b, c) (0xBDEB50): two objects' values gathered into this ---------
// this takes objects +0, descriptions +4, a +8, c +0xC, b +0x10 and gives this. For each of the two objects (k): its
// part pointer +0x14 + 4k is set to this + 0x1C + (the bytes the last described object wrote) less the object's start
// (+0x24 of *objects[k]); then, when it has a description {count, first, kinds, places}, each value from the object's
// +0x20 up to count is copied there, one after another from that start: a kind's size by its high byte (0: the size
// table 0x10F2BA8 by the low byte, 1: 16, above 0x7F: (high - 0x7F) sizes of the low byte's plus 4, else 0); its place
// (entry first - count0 + i... of places, 16 bits) is in a, c or b by its top two bits (plus the low 14), or else (top
// bits 3) in object (bit 13) at the low 13 bits: in that object itself when below its start, else in its part.
static volatile long g_text_finds;
static uint8_t *__thiscall text_find_bdeb50(uint8_t *self, uint32_t *objects, uint32_t *descriptions, uint32_t a, uint32_t b, uint32_t c) {
    g_text_finds++;
    volatile uint32_t *const t = (volatile uint32_t *)self;
    t[2] = a, t[3] = c, t[0] = (uint32_t)(uintptr_t)objects, t[4] = b, t[1] = (uint32_t)(uintptr_t)descriptions;
    const uint32_t *const sizes = (const uint32_t *)0x10f2ba8;
    uint32_t written = 0;
    for (int k = 0; k < 2; k++) {
        auto start = [&](int n) { return *(const volatile uint32_t *)(*(uint8_t *const *)(uintptr_t)objects[n] + 0x24); };
        uint32_t pos = start(k);
        t[5 + k] = written - pos + (uint32_t)(uintptr_t)self + 0x1c;
        const uint8_t *desc = (const uint8_t *)(uintptr_t)descriptions[k];
        if (!desc) continue;
        for (uint32_t i = *(const volatile uint32_t *)(*(uint8_t *const *)(uintptr_t)objects[k] + 0x20); i < *(const volatile uint16_t *)desc; i++) {
            const uint16_t kind = (*(const uint16_t *const *)(desc + 4))[i];
            const uint8_t lo = (uint8_t)kind, hi = (uint8_t)(kind >> 8);
            const uint32_t size = hi == 0 ? sizes[lo] : hi == 1 ? 0x10 : hi > 0x7f ? (hi - 0x7fu) * sizes[lo] + 4 : 0;
            uint8_t *to = (uint8_t *)(uintptr_t)(t[5 + k] + pos);
            const uint32_t ref = (*(const uint16_t *const *)(desc + 8))[*(const uint16_t *)(desc + 2) - *(const uint16_t *)desc + i];
            const uint8_t *from;
            if ((ref >> 14) < 3) {
                from = (const uint8_t *)(uintptr_t)(t[2 + (ref >> 14)] + (ref & 0x3fff));
            } else {
                const uint32_t which = ref >> 13 & 1, low = ref & 0x1fff;
                const uint32_t object = ((const uint32_t *)(uintptr_t)t[0])[which];
                from = (const uint8_t *)(uintptr_t)(low < *(const uint32_t *)(*(uint8_t *const *)(uintptr_t)object + 0x24) ? object + low : t[5 + which] + low);
            }
            volatile uint32_t *td = (volatile uint32_t *)to;
            const uint32_t *fd = (const uint32_t *)from;
            for (uint32_t n = size >> 2; n; n--) *td++ = *fd++;
            volatile uint8_t *tb = (volatile uint8_t *)td;
            const uint8_t *fb = (const uint8_t *)fd;
            for (uint32_t n = size & 3; n; n--) *tb++ = *fb++;
            pos += size;
        }
        written = pos - start(k);
    }
    return self;
}
static const uint8_t text_find_bdeb50_original[] = {0x83, 0xec, 0x08, 0x8b, 0x54, 0x24, 0x10, 0x8b};
REWRITE("Text::FindBDEB50", 0xbdeb50, text_find_bdeb50_original, &text_find_bdeb50, &g_text_finds);

// ---- Stream::ReadC51E00(this, request) (0xC51E00): the part of a request the stream holds, copied over ---------------
// The stream holds positions +0x14 up to +0x18 (signed), its data at +4 (else +8) plus +0x10. A request with a buffer at
// +4 (and none at +8), from position +0x18 (not before the stream's start) up to +0xC + +0x14, takes the bytes from the
// later start to the earlier end: the stream first moves its start up to the request's (+0x10 with it, the start kept
// at most its end), the bytes are copied to the request's buffer +0x10 less +0x14 at that position, then the stream's
// +0x10 and start move on by the count (start at most its end), the request's position too, its +0 byte set to 1 and its
// +0x1C counted up (1 after wrapping to 0). Gives the count (0 when there is nothing to take). Data at +4 whose sum
// with +0x10 is 0 counts as none.
static volatile long g_stream_reads;
static uint32_t __thiscall stream_read_c51e00(uint8_t *self, uint8_t *request) {
    g_stream_reads++;
    volatile int32_t *const s = (volatile int32_t *)self, *const q = (volatile int32_t *)request;
    if (s[6] == s[5]) return 0;
    if (!q[1] && !q[2]) return 0;
    if (q[2]) return 0;
    if (q[6] < s[5]) return 0;
    const int32_t from = s[5] < q[6] ? q[6] : s[5];
    const int32_t end = (int32_t)((uint32_t)q[3] + (uint32_t)q[5]);
    const int32_t to = s[6] > end ? end : s[6];
    const uint32_t n = (uint32_t)((to > from ? to : from) - from);
    if (!n) return 0;
    if (s[5] < from) {
        const uint32_t d = (uint32_t)(from - s[5]);
        s[4] = (int32_t)((uint32_t)s[4] + d);
        const int32_t moved = (int32_t)((uint32_t)s[5] + d), limit = s[6];
        s[5] = moved > limit ? limit : moved;
    }
    uint32_t data = 0;
    if (s[1] && (data = (uint32_t)s[1] + (uint32_t)s[4])) {
    } else if (s[2]) {
        data = (uint32_t)s[4] + (uint32_t)s[2];
    } else {
        data = 0;
    }
    const uint8_t *src = (const uint8_t *)(uintptr_t)(data - (uint32_t)s[5] + (uint32_t)from);
    uint8_t *dst = (uint8_t *)(uintptr_t)((q[1] ? (uint32_t)q[4] + (uint32_t)q[1] : 0) - (uint32_t)q[5] + (uint32_t)from);
    volatile uint32_t *td = (volatile uint32_t *)dst;
    const uint32_t *fd = (const uint32_t *)src;
    for (uint32_t i = n >> 2; i; i--) *td++ = *fd++;
    volatile uint8_t *tb = (volatile uint8_t *)td;
    const uint8_t *fb = (const uint8_t *)fd;
    for (uint32_t i = n & 3; i; i--) *tb++ = *fb++;
    s[4] = (int32_t)((uint32_t)s[4] + n);
    const int32_t moved = (int32_t)((uint32_t)s[5] + n), limit = s[6];
    s[5] = moved > limit ? limit : moved;
    q[6] = (int32_t)((uint32_t)q[6] + n);
    const uint32_t count = (uint32_t)q[7] + 1;
    *(volatile uint8_t *)request = 1;
    q[7] = (int32_t)count;
    if (!count) q[7] = 1;
    return n;
}
static const uint8_t stream_read_c51e00_original[] = {0x8b, 0xd1, 0x8b, 0x42, 0x18, 0x8b, 0x4a, 0x14};
REWRITE("Stream::ReadC51E00", 0xc51e00, stream_read_c51e00_original, &stream_read_c51e00, &g_stream_reads);

// ---- Options::ApplyKeys() (0x860A20): each of the 47 key commands given its chosen option's two values ---------------
// The option chosen for command i (0x10F0940) picks a 20-byte entry of the options table (0x10ED860); its first value
// goes to 0x10F07C0[i] and the next entry's first (+0x14) to 0x10F087C[i]. eax is left 0xBC.
static volatile long g_key_applies;
static uint32_t options_apply_keys() {
    g_key_applies++;
    for (int i = 0; i < 47; i++) {
        const uint32_t *const e = (const uint32_t *)(0x10ed860 + 20 * ((const volatile uint32_t *)0x10f0940)[i]);
        const uint32_t a = e[0], b = e[5];
        ((volatile uint32_t *)0x10f07c0)[i] = a;
        ((volatile uint32_t *)0x10f087c)[i] = b;
    }
    return 0xbc;
}
static const uint8_t options_apply_keys_original[] = {0x33, 0xc0, 0x8b, 0x0c, 0x85, 0x40, 0x09, 0x0f, 0x01};
REWRITE("Options::ApplyKeys", 0x860a20, options_apply_keys_original, &options_apply_keys, &g_key_applies);

// ---- Water::Loop8E8D80(this, i, n, heap) (0x8E8D80): entry i of a heap of n record numbers moved down into place -----
// Records are 0x130 bytes at this+0x118: a byte that says it is in use (+0) and a pointer to its key (+0xA0, 32 bits).
// A record goes above another when it is not in use, or both are and its key is larger. Each step picks the left child
// unless it goes above the right one, and stops (the entry put at i) when that child goes above the moving entry;
// otherwise the child moves up into i and i becomes the child.
static volatile long g_water_sifts;
static void __thiscall water_sift_8e8d80(const uint8_t *self, int32_t i, int32_t n, volatile int32_t *heap) {
    g_water_sifts++;
    auto record = [&](int32_t k) { return *(uint8_t *const *)(self + 0x118) + 0x130 * k; };
    auto above = [&](int32_t a, int32_t b) {   // record a goes above record b
        const uint8_t *x = record(a), *y = record(b);
        return !x[0] || (y[0] && **(const uint32_t *const *)(x + 0xa0) > **(const uint32_t *const *)(y + 0xa0));
    };
    const int32_t v = heap[i];
    int32_t c = 2 * i + 1;
    while (c + 1 < n) {
        const int32_t pick = above(heap[c], heap[c + 1]) ? c + 1 : c;
        if (above(heap[pick], v)) {
            heap[i] = v;
            return;
        }
        heap[i] = heap[pick];
        i = pick;
        c = 2 * i + 1;
    }
    if (c >= n || above(heap[c], v)) {
        heap[i] = v;
        return;
    }
    heap[i] = heap[c];
    heap[c] = v;
}
static const uint8_t water_sift_8e8d80_original[] = {0x51, 0x8b, 0x44, 0x24, 0x0c, 0x53, 0x55, 0x8b};
REWRITE("Water::Loop8E8D80", 0x8e8d80, water_sift_8e8d80_original, &water_sift_8e8d80, &g_water_sifts);

// ---- Terrain::Range7AE6F0(this, x, y, out x, out y) (0x7AE6F0): the nearest set cell to a point, by a square spiral ----
// The grid: first cell x0 +4 and y0 +5, width +6 and height +7 (bytes), a byte per cell at +8 with rows from the top
// (y0 + height - 1) down. The point is first kept inside the grid; then a spiral of steps round it (the game's own order,
// widening until its reach is 10) looks for a cell not 0 inside the grid: its place (bytes) is written out and 1 given;
// else 0.
static volatile long g_terrain_ranges;
static uint8_t __thiscall terrain_range_7ae6f0(const uint8_t *self, uint32_t x_in, uint32_t y_in, uint8_t *out_x, uint8_t *out_y) {
    g_terrain_ranges++;
    const uint8_t x0 = self[4], y0 = self[5];
    const uint8_t x = (uint8_t)x_in, y = (uint8_t)y_in;
    const uint8_t X = x < x0 ? x0 : (int32_t)x > (int32_t)self[6] + x0 - 1 ? (uint8_t)(self[6] + x0 - 1) : x;
    const uint8_t Y = y < y0 ? y0 : (int32_t)y > (int32_t)self[7] + y0 - 1 ? (uint8_t)(self[7] + y0 - 1) : y;
    const uint8_t top = (uint8_t)(self[7] + y0 - 1);
    int32_t dx = 0, dy = 0, reach = 0, low = 0, a = 0, b = 0, e;
    do {
        const uint8_t cy = (uint8_t)(dy + Y), cx = (uint8_t)(dx + X);
        if (cx >= x0 && cx <= (uint8_t)(self[6] + x0 - 1) && cy >= y0 && cy <= top &&
            *(const uint8_t *)(uintptr_t)(((uint32_t)top - cy) * self[6] + *(const uint32_t *)(self + 8) - x0 + cx)) {
            *out_x = (uint8_t)(dx + X);
            *out_y = (uint8_t)(dy + Y);
            return 1;
        }
        e = reach;
        if (dy >= reach) {
            dx--, a++, b--;
            if (dx < low) reach = a, low = b, e = a;
        } else if (dx >= reach) {
            dy++;
        } else if (dy > low) {
            dy--;
        } else {
            dx++, a--, b++;
        }
    } while (e < 10);
    return 0;
}
static const uint8_t terrain_range_7ae6f0_original[] = {0x83, 0xec, 0x14, 0x53, 0x55, 0x56, 0x8b, 0xf1};
REWRITE("Terrain::Range7AE6F0", 0x7ae6f0, terrain_range_7ae6f0_original, &terrain_range_7ae6f0, &g_terrain_ranges);

// ---- PathGrid::InsideB9B090(this, x, y) (0xB9B090): which of a cell's four neighbours belong to something else -------
// A cell's owner is the first value of its 0x18 bytes (cells +0x6C, width +0x64, height +0x68, signed); outside the grid
// it is -1. Bit 1: the cell to the left differs from this one's, 2: the one below (y + 1), 4: the right, 8: above.
static volatile long g_path_insides;
static uint32_t __thiscall path_grid_inside(const uint8_t *self, uint32_t x_in, uint32_t y_in) {
    g_path_insides++;
    const int32_t x = (uint8_t)x_in, y = (uint8_t)y_in;
    auto owner = [&](int32_t cx, int32_t cy) -> uint32_t {
        if (cx < 0 || cy < 0 || cx >= *(const int32_t *)(self + 0x64) || cy >= *(const int32_t *)(self + 0x68)) return 0xffffffffu;
        return *(const uint32_t *)(*(const uint8_t *const *)(self + 0x6c) + 0x18 * (*(const int32_t *)(self + 0x64) * cy + cx));
    };
    const uint32_t c = owner(x, y);
    uint32_t r = 0;
    if (owner(x - 1, y) != c) r = 1;
    if (owner(x, y + 1) != c) r |= 2;
    if (owner(x + 1, y) != c) r |= 4;
    if (owner(x, y - 1) != c) r |= 8;
    return r;
}
static const uint8_t path_grid_inside_original[] = {0x53, 0x0f, 0xb6, 0x5c, 0x24, 0x0c, 0x55, 0x56};
REWRITE("PathGrid::InsideB9B090", 0xb9b090, path_grid_inside_original, &path_grid_inside, &g_path_insides);

// ---- Terrain::Loop8D67A0(from in esi, to in edx, x, y, to x, to y) (0x8D67A0): one terrain cell copied to another -----
// Grids: width +4 (byte), cells of 0x18 bytes at +0x24. The cell's first four values and its bytes +0x10 and +0x11 are
// copied, and bits 0-6 of its byte +0x12 set in the other's (bits already set there stay). Places are bytes.
static volatile long g_terrain_cell_copies;
extern "C" uint32_t __stdcall forever_terrain_cell_copy(const uint8_t *from, const uint8_t *to, uint32_t x, uint32_t y, uint32_t tx, uint32_t ty) {
    g_terrain_cell_copies++;
    auto cell = [](const uint8_t *grid, uint32_t cx, uint32_t cy) {
        return *(uint8_t *const *)(grid + 0x24) + 0x18 * ((uint32_t)grid[4] * (uint8_t)cy + (uint8_t)cx);
    };
    for (int k = 0; k < 4; k++) ((volatile uint32_t *)cell(to, tx, ty))[k] = ((const uint32_t *)cell(from, x, y))[k];
    *(volatile uint8_t *)(cell(to, tx, ty) + 0x10) = cell(from, x, y)[0x10];
    *(volatile uint8_t *)(cell(to, tx, ty) + 0x11) = cell(from, x, y)[0x11];
    for (int b = 0; b < 7; b++)
        if (*(volatile uint8_t *)(cell(from, x, y) + 0x12) & (1 << b)) *(volatile uint8_t *)(cell(to, tx, ty) + 0x12) |= (uint8_t)(1 << b);
    return 0;
}
// the two grids come in esi and edx: put in front of the four stack values, the function takes all six off
extern "C" __attribute__((naked)) void forever_terrain_cell_copy_regs() {
    __asm__("popl %eax\n\tpushl %edx\n\tpushl %esi\n\tpushl %eax\n\tjmp _forever_terrain_cell_copy@24");
}
static const uint8_t terrain_cell_copy_original[] = {0x83, 0xec, 0x0c, 0x53, 0x55, 0x33, 0xc9, 0xc7};
REWRITE("Terrain::Loop8D67A0", 0x8d67a0, terrain_cell_copy_original, (void *)&forever_terrain_cell_copy_regs, &g_terrain_cell_copies);

// ---- Table::FreeCA6900(this, n) (0xCA6900): slot n given back, joined with free neighbours ---------------------------
// Slots of 8 bytes at +0: a value whose bit 0 says in use (the rest a place in the area at +4), then 16-bit numbers of
// the slots before (+4) and after (+6). Free slots form a list from +0xA (through +6). In the area, each place holds a
// ring link pair {16-bit, 16-bit} of slot numbers, slot 0's place being the ring's head. When the slot before n is free,
// n is taken out of the slot chain (the free list gets n) and that one is the slot to keep; otherwise n's place goes
// into the ring just after the head, its first link cleared. Then when the slot after the kept one is free it is taken
// out of the chain and the ring and goes to the free list. The kept slot is marked free (bit 0 cleared); gives its value.
static volatile long g_table_frees;
static uint32_t __thiscall table_free_ca6900(uint8_t *self, uint32_t n_in) {
    g_table_frees++;
    volatile uint32_t *const t = (volatile uint32_t *)self;
    volatile uint16_t *const head = (volatile uint16_t *)(self + 0xa);
    auto value = [&](uint32_t k) -> volatile uint32_t & { return *(volatile uint32_t *)(uintptr_t)(t[0] + 8 * k); };
    auto before = [&](uint32_t k) -> volatile uint16_t & { return *(volatile uint16_t *)(uintptr_t)(t[0] + 8 * k + 4); };
    auto after = [&](uint32_t k) -> volatile uint16_t & { return *(volatile uint16_t *)(uintptr_t)(t[0] + 8 * k + 6); };
    auto link = [&](uint32_t place, uint32_t at) -> volatile uint16_t & { return *(volatile uint16_t *)(uintptr_t)(place + t[1] + at); };
    const uint16_t n = (uint16_t)n_in;
    uint16_t keep;
    const uint16_t prev = before(n);
    if (!(value(prev) & 1)) {
        const uint16_t next = after(n);
        before(next) = prev;
        after(prev) = next;
        after(n) = *head;
        *head = n;
        keep = prev;
    } else {
        link(value(n) & ~1u, 0) = 0;
        link(value(n) & ~1u, 2) = (uint16_t)link(value(0) & ~1u, 2);
        const uint16_t k = link(value(0) & ~1u, 2);
        link(value(k) & ~1u, 0) = n;
        link(value(0) & ~1u, 2) = n;
        keep = n;
    }
    const uint16_t nx = after(keep);
    if (!(value(nx) & 1)) {
        const uint16_t a = after(nx), b = before(nx);
        before(a) = b;
        after(b) = a;
        const uint32_t place = value(nx) & ~1u;
        const uint16_t l0 = link(place, 0), l2 = link(place, 2);
        link(value(l2) & ~1u, 0) = l0;
        link(value(l0) & ~1u, 2) = l2;
        after(nx) = *head;
        *head = nx;
    }
    const uint32_t v = value(keep) & ~1u;
    value(keep) = v;
    return v;
}
static const uint8_t table_free_ca6900_original[] = {0x51, 0x8b, 0x54, 0x24, 0x08, 0x53, 0x55, 0x0f};
REWRITE("Table::FreeCA6900", 0xca6900, table_free_ca6900_original, &table_free_ca6900, &g_table_frees);

// ---- Device::VersionC00C80(version) (0xC00C80): the device's value for a shader version, 0 when it has none ---------
// The major number (bits 16-19: 1 to 4, else 0) and the minor (bits 20-23) pick one of the device's (0x13D0D84) values:
// minors 1-4 (+0x12C, +0x130, +0x134, +0x138) for majors up to 1, minors 6, 7, 8 (+0x140, +0x13C, +0x144) for major 0.
static volatile long g_device_versions;
static uint32_t __stdcall device_version(uint32_t version) {
    g_device_versions++;
    const uint32_t m = version & 0xf0000;
    const int32_t major = m == 0x10000 ? 1 : m == 0x20000 ? 2 : m == 0x30000 ? 3 : m == 0x40000 ? 4 : 0;
    const uint8_t *const device = *(const uint8_t *const volatile *)0x13d0d84;
    switch (version & 0xf00000) {
    case 0x100000: return major <= 1 ? *(const uint32_t *)(device + 0x12c) : 0;
    case 0x200000: return major <= 1 ? *(const uint32_t *)(device + 0x130) : 0;
    case 0x300000: return major <= 1 ? *(const uint32_t *)(device + 0x134) : 0;
    case 0x400000: return major <= 1 ? *(const uint32_t *)(device + 0x138) : 0;
    case 0x600000: return major == 0 ? *(const uint32_t *)(device + 0x140) : 0;
    case 0x700000: return major == 0 ? *(const uint32_t *)(device + 0x13c) : 0;
    case 0x800000: return major == 0 ? *(const uint32_t *)(device + 0x144) : 0;
    default: return 0;
    }
}
static const uint8_t device_version_original[] = {0x8b, 0x54, 0x24, 0x04, 0x8b, 0xc2, 0x25, 0x00, 0x00, 0x0f, 0x00};
REWRITE("Device::VersionC00C80", 0xc00c80, device_version_original, &device_version, &g_device_versions);

// ---- Model::FindCC7200(this, key, on, a, b, flag) (0xCC7200): an entry's settings, and its key kept in a sorted list --
// The model (+0) lists its entries' keys (+0x20, count +0x1C). For the entry i holding key: its 16-byte record at
// +0x18 takes {on, a, b, flag byte}; its part's (model +0x10, 0x60 bytes each, by the entry's key) value +0x58 is looked
// for among the {value, entry} pairs at +0x1C (count +0x20, sorted by value, signed). Missing, it is put in before the
// first larger value; there and `on` is 0, it is taken out. Gives 1, or 0 when no entry holds key.
static volatile long g_model_finds;
static uint8_t __thiscall model_find_cc7200(uint8_t *self, uint32_t key, uint32_t on, uint32_t a, uint32_t b, uint32_t flag) {
    g_model_finds++;
    const uint8_t *const model = *(const uint8_t *const *)self;
    volatile int32_t *const count = (volatile int32_t *)(self + 0x20);
    int32_t i = 0;
    for (;; i++) {
        if (i >= *(const int32_t *)(*(const uint8_t *const *)self + 0x1c)) return 0;
        if ((*(const uint32_t *const *)(model + 0x20))[i] == key) break;
    }
    volatile uint32_t *const rec = (volatile uint32_t *)(*(uint8_t *const *)(self + 0x18) + 16 * i);
    rec[1] = a, rec[2] = b, *(volatile uint8_t *)(rec + 3) = (uint8_t)flag, rec[0] = on;
    const uint8_t *const m = *(const uint8_t *const *)self;
    const int32_t value = *(const int32_t *)(*(const uint8_t *const *)(m + 0x10) + 0x60 * (*(const uint32_t *const *)(m + 0x20))[i] + 0x58);
    const int32_t n = *count;
    auto pairs = [&]() { return (volatile int32_t *)*(int32_t *const *)(self + 0x1c); };
    int32_t j = 0;
    if (n > 0)
        for (; j < *count; j++)
            if (pairs()[2 * j] == value) break;
    if (j == n) {
        int32_t p = 0;
        if (n > 0)
            for (; p < *count; p++)
                if (pairs()[2 * p] > value) break;
        for (int32_t s = n; s > p; s--) {
            volatile int32_t *const l = pairs();
            l[2 * s] = l[2 * s - 2];
            l[2 * s + 1] = l[2 * s - 1];
        }
        pairs()[2 * p] = value, pairs()[2 * p + 1] = i;
        *count = *count + 1;
        return 1;
    }
    if (on) return 1;
    for (int32_t k = j + 1; k < *count; k++) {
        volatile int32_t *const l = pairs();
        l[2 * k - 2] = l[2 * k];
        l[2 * k - 1] = l[2 * k + 1];
    }
    *count = *count - 1;
    return 1;
}
static const uint8_t model_find_cc7200_original[] = {0x8b, 0x01, 0x8b, 0x50, 0x1c, 0x53, 0x56, 0x57};
REWRITE("Model::FindCC7200", 0xcc7200, model_find_cc7200_original, &model_find_cc7200, &g_model_finds);

// ---- Water::Alpha97C630(this) (0x97C630): the three pairs of parts see-through (0.3) or not (1.0) --------------------
// With no current item (+0xD8's +0xF0, or no +0xD8) all six parts (+0xE8, +0xFC, +0x110, +0xE4, +0xF8, +0x10C) get 0.3 as
// their alpha (+0xC4); otherwise each pair ({+0xE8, +0xE4} by +0xE0, {+0xFC, +0xF8} by +0xF4, {+0x110, +0x10C} by +0x108)
// gets 0.3 when its item is the current one, else 1.0.
static volatile long g_water_alphas;
static void __fastcall water_alpha_97c630(const uint8_t *self) {
    g_water_alphas++;
    auto set = [&](uint32_t at, uint32_t bits) { *(volatile uint32_t *)(*(uint8_t *const *)(self + at) + 0xc4) = bits; };
    const uint8_t *const holder = *(const uint8_t *const *)(self + 0xd8);
    const uint32_t current = holder ? *(const uint32_t *)(holder + 0xf0) : 0;
    if (!current) {
        static const uint32_t all[] = {0xe8, 0xfc, 0x110, 0xe4, 0xf8, 0x10c};
        for (uint32_t at : all) set(at, 0x3e99999a);
        return;
    }
    static const uint32_t pairs[3][3] = {{0xe0, 0xe8, 0xe4}, {0xf4, 0xfc, 0xf8}, {0x108, 0x110, 0x10c}};
    for (const auto &p : pairs) {
        const uint32_t bits = *(const uint32_t *)(self + p[0]) == current ? 0x3e99999a : 0x3f800000;
        set(p[1], bits), set(p[2], bits);
    }
}
static const uint8_t water_alpha_97c630_original[] = {0x8b, 0x81, 0xd8, 0x00, 0x00, 0x00, 0x85, 0xc0};
REWRITE("Water::Alpha97C630", 0x97c630, water_alpha_97c630_original, &water_alpha_97c630, &g_water_alphas);

// ---- Shader::SetCB5BE0(this, target) (0xCB5BE0): how many steps down from the current level (+0x60) to the target ----
// 0 when already there. With more than one step size (+0x4C of the 16-bit percentages at +0x54, every 4 bytes): the
// number of percentages of the current level that stay above the target, less one. With one: the level is taken down
// by its percentage each step, the percentage moving toward 100 by (100 - it) times the first percentage / 100, then
// away by as much, by turns; the steps until the level is not above the target (or reaches 1), less one. Divisions by
// 100 cut toward 0, products in 32 bits.
static volatile long g_shader_steps;
static int32_t __thiscall shader_set_cb5be0(const uint8_t *self, int32_t target) {
    g_shader_steps++;
    const int32_t n = *(const int32_t *)(self + 0x4c);
    const int32_t cur = *(const int32_t *)(self + 0x60);
    const int16_t *const pc = *(const int16_t *const *)(self + 0x54);
    if (target == cur) return 0;
    if (n > 1) {
        int32_t steps = 0, level = cur;
        for (int32_t i = 0; i < n && level > target; i++, steps++) level = (int32_t)((uint32_t)(int32_t)pc[2 * i] * (uint32_t)cur) / 100;
        return steps - 1;
    }
    const int32_t first = pc[0];
    int32_t level = cur, p = first, count = 0, up = 1;
    int32_t drop = (int32_t)((uint32_t)level * (uint32_t)p) / 100;
    while (level > target) {
        level -= drop;
        count++;
        if (level == 1) break;
        const int32_t c = (int32_t)((uint32_t)(100 - p) * (uint32_t)first);
        p += up > 0 ? c / 100 : c / -100;
        up = up == 0;
        drop = (int32_t)((uint32_t)level * (uint32_t)p) / 100;
    }
    return count - 1;
}
static const uint8_t shader_set_cb5be0_original[] = {0x51, 0x53, 0x8b, 0x59, 0x4c, 0x83, 0xfb, 0x01};
REWRITE("Shader::SetCB5BE0", 0xcb5be0, shader_set_cb5be0_original, &shader_set_cb5be0, &g_shader_steps);

// ---- Scenery::Pop85A9C0(this, a, b) (0x85A9C0): the value kept for a pair of keys, by a binary search ---------------
// The pairs are sorted (by a, then b; signed) in two lists (+8 and +0x10) with their values at +0x14 (count +0x18). The
// first and last entries are checked first; -1 when the pair is not there.
static volatile long g_scenery_pops;
static uint32_t __thiscall scenery_pop_85a9c0(const uint8_t *self, int32_t a, int32_t b) {
    g_scenery_pops++;
    const int32_t n = *(const int32_t *)(self + 0x18);
    if (n <= 0) return 0xffffffffu;
    const int32_t *const k1 = *(const int32_t *const *)(self + 8), *const k2 = *(const int32_t *const *)(self + 0x10);
    auto value = [&](int32_t i) { return (*(const uint32_t *const *)(self + 0x14))[i]; };
    int32_t hi = n - 1, lo = 0;
    if (k1[0] == a && k2[0] == b) return value(0);
    if (k1[hi] == a && k2[hi] == b) return value(hi);
    if (k1[0] > a || (k1[0] == a && k2[0] > b)) return 0xffffffffu;
    if (k1[hi] < a || (k1[hi] == a && k2[hi] < b)) return 0xffffffffu;
    if (hi <= 1) return 0xffffffffu;
    do {
        const int32_t mid = lo + (hi - lo) / 2;
        const int32_t x = k1[mid], y = k2[mid];
        if (x == a && y == b) return value(mid);
        if (x < a || (x == a && y < b)) lo = mid;
        else hi = mid;
    } while (hi > lo + 1);
    return 0xffffffffu;
}
static const uint8_t scenery_pop_85a9c0_original[] = {0x83, 0xec, 0x0c, 0x8b, 0x41, 0x18, 0x85, 0xc0};
REWRITE("Scenery::Pop85A9C0", 0x85a9c0, scenery_pop_85a9c0_original, &scenery_pop_85a9c0, &g_scenery_pops);

// ---- Text::SumBD70B0(this, list, n, out) (0xBD70B0): the n entries' 16-bit counts added up, or their parts listed ----
// Entries are this+0x10's, numbered by list. Without out: the sum of their counts (+8). With out: each entry's parts
// (count +8, 4 bytes each at +0xC) written to out as {first, second, third} bytes (the fourth left), the third raised past
// any earlier part with the same second and third (after a raise the look starts again from the second part written);
// gives how many were written.
static volatile long g_text_sums;
static uint32_t __thiscall text_sum_bd70b0(const uint8_t *self, const uint32_t *list, uint32_t n, uint8_t *out) {
    g_text_sums++;
    auto entry = [&](uint32_t i) { return (*(const uint8_t *const *const *)(self + 0x10))[list[i]]; };
    if (!out) {
        uint32_t sum = 0;
        for (uint32_t i = 0; i < n; i++) sum += *(const uint16_t *)(entry(i) + 8);
        return sum;
    }
    uint32_t written = 0;
    for (uint32_t i = 0; i < n; i++) {
        const uint8_t *const e = entry(i);
        for (uint32_t s = 0; s < *(const volatile uint16_t *)(e + 8); s++) {
            const uint8_t *const part = *(const uint8_t *const *)(e + 0xc) + 4 * s;
            const uint8_t first = part[0], second = part[1];
            uint8_t third = part[2];
            for (uint32_t d = 0; d < written; d++)
                if (out[4 * d + 1] == second && out[4 * d + 2] == third) third++, d = 0;
            volatile uint8_t *const o = out + 4 * written;
            o[0] = first, o[1] = second, o[2] = third;
            written++;
        }
    }
    return written;
}
static const uint8_t text_sum_bd70b0_original[] = {0x83, 0xec, 0x10, 0x53, 0x8b, 0xd9, 0x8b, 0x4c};
REWRITE("Text::SumBD70B0", 0xbd70b0, text_sum_bd70b0_original, &text_sum_bd70b0, &g_text_sums);

// ---- ShaderFile::FindC47500(this, key) (0xC47500): an entry found by a hash of its key ------------------------------
// A key is four values (+0 to +0xC; the last holds the data's byte count in its high 16 bits) and data (+0x10) of that
// many bytes / 4 values. The hash, all of them xored, picks a bucket (modulo +4) whose entries run from +0xC's [bucket] up
// to [bucket + 1]; entries are 20 bytes at +0x10: the four values and a pointer to their data. Gives the matching entry's
// 16-bit slot in +0x14, or null.
static volatile long g_shader_file_finds;
static uint8_t *__thiscall shader_file_find(const uint8_t *self, const uint32_t *key) {
    g_shader_file_finds++;
    const uint16_t words = (uint16_t)(*(const uint16_t *)((const uint8_t *)key + 0xe) >> 2);
    const uint32_t *const data = (const uint32_t *)(uintptr_t)key[4];
    uint32_t hash = key[0] ^ key[1] ^ key[2] ^ key[3];
    for (uint32_t i = 0; i < words; i++) hash ^= data[i];
    const uint32_t *const bucket = *(const uint32_t *const *)(self + 0xc) + hash % *(const uint32_t *)(self + 4);
    const uint32_t end = bucket[1];
    for (uint32_t i = bucket[0]; i < end; i++) {
        const uint32_t *const e = (const uint32_t *)(*(const uint8_t *const *)(self + 0x10) + 20 * i);
        if (e[0] != key[0] || e[1] != key[1] || e[2] != key[2] || e[3] != key[3]) continue;
        const uint32_t *const other = (const uint32_t *)(uintptr_t)e[4];
        uint16_t k = 0;
        while (k < words && data[k] == other[k]) k++;
        if (k == words) return *(uint8_t *const *)(self + 0x14) + 2 * i;
    }
    return nullptr;
}
static const uint8_t shader_file_find_original[] = {0x83, 0xec, 0x18, 0x8b, 0x54, 0x24, 0x1c, 0x8b};
REWRITE("ShaderFile::FindC47500", 0xc47500, shader_file_find_original, &shader_file_find, &g_shader_file_finds);

// ---- Stream::StepC50390(this) (0xC50390): a stream started over ------------------------------------------------------
// When it has a part (+0xF0), each of the 32 streams (0x13E6BB8) on the same part gets bit 30 of its +0x10 set. Then bits
// 11 and 12 of +0x10 are cleared, +0x64 takes +0x60, +0x5C and the byte +0x4C are cleared and +0x68 is counted up (1
// after wrapping to 0; the count before that fix is given back).
static volatile long g_stream_steps;
static uint32_t __fastcall stream_step_c50390(uint8_t *self) {
    g_stream_steps++;
    volatile uint32_t *const t = (volatile uint32_t *)self;
    if (t[0xf0 / 4])
        for (uint32_t d = 0; d < 32; d++) {
            volatile uint32_t *const s = (volatile uint32_t *)(uintptr_t)((const volatile uint32_t *)0x13e6bb8)[d];
            if (s[0xec / 4] == t[0xf0 / 4]) s[4] |= 0x40000000;
        }
    t[4] &= 0xffffe7ffu;
    t[0x64 / 4] = t[0x60 / 4];
    const uint32_t count = t[0x68 / 4] + 1;
    t[0x5c / 4] = 0;
    self[0x4c] = 0;
    t[0x68 / 4] = count;
    if (!count) t[0x68 / 4] = 1;
    return count;
}
static const uint8_t stream_step_c50390_original[] = {0x8b, 0x81, 0xf0, 0x00, 0x00, 0x00, 0x85, 0xc0};
REWRITE("Stream::StepC50390", 0xc50390, stream_step_c50390_original, &stream_step_c50390, &g_stream_steps);

// ---- OverlayFile::FindStreamedItem(this, id, value) (0xC3B2C0): a streamed item by id and 16-bit value --------------
// The table (+0x4C, count +0x48) holds 20-byte entries sorted by id (signed, +0) and then by value (+4, 16 bits). A binary
// search finds an entry with the id; if its value's low 16 bits are the ones given it is the one; otherwise the run of
// entries with that id is searched by value (0xFFFF counting as -1, compared with the whole value given). Null when
// there is none.
static volatile long g_streamed_finds;
static uint8_t *__thiscall overlay_find_streamed(const uint8_t *self, int32_t id, int32_t value) {
    g_streamed_finds++;
    const uint32_t count = *(const uint32_t *)(self + 0x48);
    if (!count) return nullptr;
    uint8_t *const table = *(uint8_t *const *)(self + 0x4c);
    auto id_of = [&](uint32_t i) { return *(const int32_t *)(table + 20 * i); };
    uint32_t lo = 0, hi = count - 1, mid;
    for (;;) {
        mid = (hi + lo) >> 1;
        const int32_t k = id_of(mid);
        if (id < k) {
            if (lo >= mid) return nullptr;
            hi = mid - 1;
        } else if (id > k) {
            if (hi <= mid) return nullptr;
            lo = mid + 1;
        } else {
            break;
        }
    }
    if (*(const uint16_t *)(table + 20 * mid + 4) == (uint16_t)value) return table + 20 * mid;
    int32_t first = (int32_t)mid, last = (int32_t)mid;
    if ((int32_t)mid - 1 >= 0)
        do {
            if (id_of((uint32_t)first - 1) != id) break;
            first--;
        } while (first >= 1);
    for (uint32_t i = mid + 1; i < count && id_of(i) == id; i++) last++;
    for (;;) {
        const int32_t m = (last + first) / 2;
        const uint16_t v = *(const uint16_t *)(table + 20 * m + 4);
        const int32_t c = v == 0xffff ? -1 : (int32_t)v;
        if (value < c) {
            if ((uint32_t)first >= (uint32_t)m) return nullptr;
            last = m - 1;
        } else if (value > c) {
            if ((uint32_t)last <= (uint32_t)m) return nullptr;
            first = m + 1;
        } else {
            return table + 20 * m;
        }
    }
}
static const uint8_t overlay_find_streamed_original[] = {0x51, 0x55, 0x8b, 0x69, 0x48, 0x85, 0xed, 0x75};
REWRITE("OverlayFile::FindStreamedItem", 0xc3b2c0, overlay_find_streamed_original, &overlay_find_streamed, &g_streamed_finds);

// ---- Window::Flags6F0D80(this, flag, n) (0x6F0D80): whether a flag is the window's n-th one, in the game's order -----
// 0 unless the window (+0xD0) has the flag. The flags are counted in the order 0x80, 4, 8, 0x10, (again 0x10), 1, 0x20,
// 0x40, 2, 0x100, 0x200, 0x400, counting only those the window has (from 1): when the flag is met as number n it gives
// 1, or 2 when met at the repeat; 0 when the list ends.
static volatile long g_window_flag_orders;
static uint32_t __thiscall window_flags_6f0d80(const uint8_t *self, uint32_t flag, int32_t n) {
    g_window_flag_orders++;
    static const uint32_t order[] = {0x80, 4, 8, 0x10, 0xffffffffu, 1, 0x20, 0x40, 2, 0x100, 0x200, 0x400, 0};
    const uint32_t has = *(const uint32_t *)(self + 0xd0);
    if (!(flag & has)) return 0;
    int32_t number = 1;
    for (const uint32_t *p = order; *p; p++) {
        const uint32_t c = *p == 0xffffffffu ? p[-1] : *p;
        if (flag == c && number == n) return *p == 0xffffffffu ? 2 : 1;
        if (c & has) number++;
    }
    return 0;
}
static const uint8_t window_flags_6f0d80_original[] = {0x83, 0xec, 0x34, 0x53, 0x8b, 0x99, 0xd0, 0x00};
REWRITE("Window::Flags6F0D80", 0x6f0d80, window_flags_6f0d80_original, &window_flags_6f0d80, &g_window_flag_orders);

// ---- Toolbar::Next79CB60() (0x79CB60): the next shared item looked at, its "new" mark cleared when no window shows it --
// Only when 0x1306BCC and 0x1306BD8 differ. The index 0x1306BB8 is kept within the list (entries 0x1306BE0 up to
// 0x1306BE4 of 0x1306BC8; else 0); an item there marked 1 at +0xA8 has the mark cleared unless an open window of kind 0xE
// (list 0x10EF9D0) or 0xD (list 0x10EF794) shows it (window +0x190 the kind, its +0x1C0's +0xC the item); then the index
// moves on by one.
static volatile long g_toolbar_nexts;
static void toolbar_next_79cb60() {
    g_toolbar_nexts++;
    volatile int32_t *const index = (volatile int32_t *)0x1306bb8;
    if (*(volatile int32_t *)0x1306bcc == *(volatile int32_t *)0x1306bd8) return;
    const int32_t begin = *(volatile int32_t *)0x1306be0;
    int32_t i = *index;
    if (i < 0 || i >= (int32_t)((uint32_t)(*(volatile int32_t *)0x1306be4 - begin) << 2) >> 2) {
        i = 0;
        *index = 0;
    }
    uint8_t *const item = (*(uint8_t *const *const volatile *)0x1306bc8)[begin + i];
    if (item && *(const uint32_t *)(item + 0xa8) == 1) {
        auto shown = [&](uintptr_t list, uint32_t kind) {
            const uint32_t *node = *(const uint32_t *const volatile *)list;
            if (!*node || !node) return false;
            for (;;) {
                const uint8_t *const w = (const uint8_t *)(uintptr_t)node[2];
                if (w && *(const uint32_t *)(w + 0x190) == kind && *(const uint32_t *)(*(const uint8_t *const *)(w + 0x1c0) + 0xc) == (uint32_t)(uintptr_t)item) return true;
                node = (const uint32_t *)(uintptr_t)*node;
                if (!*node || !node) return false;
            }
        };
        if (shown(0x10ef9d0, 0xe) || shown(0x10ef794, 0xd)) {
            *index = i + 1;
            return;
        }
        *(volatile uint32_t *)(item + 0xa8) = 0;
        i = *index;
    }
    *index = i + 1;
}
static const uint8_t toolbar_next_79cb60_original[] = {0xa1, 0xcc, 0x6b, 0x30, 0x01, 0x2b, 0x05, 0xd8};
REWRITE("Toolbar::Next79CB60", 0x79cb60, toolbar_next_79cb60_original, &toolbar_next_79cb60, &g_toolbar_nexts);

// ---- List::Pass42D400(this, dx, dy) (0x42D400): the boxes of the entries in use moved, kept within 0 to 255 -----------
// Entries are 16 bytes at +0, in use when their first byte is not 0; a box is the bytes +4 to +7 (left, top, right,
// bottom): left and top moved and kept at least 0, right and bottom moved and kept at most 255 (the bytes written are the
// low ones). It goes on until it has moved as many boxes as the count (+8), however many entries that passes.
static volatile long g_list_passes;
static void __thiscall list_pass_42d400(const uint8_t *self, int32_t dx, int32_t dy) {
    g_list_passes++;
    int32_t moved = 0;
    if (*(const volatile int32_t *)(self + 8) <= 0) return;
    for (uint32_t at = 0;; at += 0x10) {
        volatile uint8_t *const e = *(uint8_t *const *)self + at;
        if (e[0]) {
            const int32_t l = e[4] + dx, t = e[5] + dy, r = e[6] + dx, b = e[7] + dy;
            moved++;
            e[4] = (uint8_t)(l > 0 ? l : 0);
            e[5] = (uint8_t)(t > 0 ? t : 0);
            e[6] = (uint8_t)(r < 0xff ? r : 0xff);
            e[7] = (uint8_t)(b < 0xff ? b : 0xff);
        }
        if (moved >= *(const volatile int32_t *)(self + 8)) return;
    }
}
static const uint8_t list_pass_42d400_original[] = {0x8b, 0x41, 0x08, 0x83, 0xec, 0x18, 0x53, 0x33};
REWRITE("List::Pass42D400", 0x42d400, list_pass_42d400_original, &list_pass_42d400, &g_list_passes);

// ---- Ride::Part4FE690(this, flag, a, b) (0x4FE690): a ride's tallies counted by its mode (+0x100) --------------------
// Mode 0: without the flag, +0x128 when +0xD8 is b, else +0x12C when a is +0xD4; with the flag, +0x11C when +0xD8 is
// above b, +0x120 when it is b, else +0x124 when a is +0xD4. Mode 1 (a not above +0xD4): +0x16C, and +0x164 when a is
// within 1 of +0xD8. Mode 2: +0x158 when a is 0 to 2 above +0xD8. Signed.
static volatile long g_ride_tallies;
static void __thiscall ride_part_4fe690(uint8_t *self, uint32_t flag, int32_t a, int32_t b) {
    g_ride_tallies++;
    volatile int32_t *const t = (volatile int32_t *)self;
    const int32_t mode = t[0x100 / 4];
    if (mode == 0) {
        const int32_t e = t[0xd8 / 4];
        if (!(uint8_t)flag) {
            if (e == b) t[0x128 / 4]++;
            else if (a == t[0xd4 / 4]) t[0x12c / 4]++;
            return;
        }
        if (e > b) t[0x11c / 4]++;
        if (e == b) t[0x120 / 4]++;
        else if (a == t[0xd4 / 4]) t[0x124 / 4]++;
    } else if (mode == 1) {
        if (a > t[0xd4 / 4]) return;
        const int32_t d = (int32_t)((uint32_t)a - (uint32_t)t[0xd8 / 4]);
        t[0x16c / 4] = t[0x16c / 4] + 1;
        if ((d < 0 ? (int32_t)(0u - (uint32_t)d) : d) <= 1) t[0x164 / 4]++;
    } else if (mode == 2) {
        const int32_t d = (int32_t)((uint32_t)a - (uint32_t)t[0xd8 / 4]);
        if (d >= 0 && d <= 2) t[0x158 / 4]++;
    }
}
static const uint8_t ride_part_4fe690_original[] = {0x8b, 0x81, 0x00, 0x01, 0x00, 0x00, 0x85, 0xc0};
REWRITE("Ride::Part4FE690", 0x4fe690, ride_part_4fe690_original, &ride_part_4fe690, &g_ride_tallies);

// ---- Water::Step973040(a, b) (0x973040): two keys given by value compared: 1, -1 or 0 --------------------------------
// A key is 32 bytes: two unsigned values, a pointer to three signed ones, a signed value, a flag byte, two signed values
// and a flag byte, compared in that order (a set flag above a clear one).
namespace {
struct WaterKey {
    uint32_t a, b;
    const int32_t *three;
    int32_t c;
    uint32_t flag1;
    int32_t d, e;
    uint32_t flag2;
};
}
static volatile long g_water_key_compares;
static int32_t __stdcall water_key_compare(WaterKey x, WaterKey y) {
    g_water_key_compares++;
    if (x.a != y.a) return x.a > y.a ? 1 : -1;
    if (x.b != y.b) return x.b > y.b ? 1 : -1;
    for (int i = 0; i < 3; i++)
        if (x.three[i] != y.three[i]) return x.three[i] > y.three[i] ? 1 : -1;
    if (x.c != y.c) return x.c > y.c ? 1 : -1;
    const bool f1 = (uint8_t)x.flag1, g1 = (uint8_t)y.flag1;
    if (f1 != g1) return f1 ? 1 : -1;
    if (x.d != y.d) return x.d > y.d ? 1 : -1;
    if (x.e != y.e) return x.e > y.e ? 1 : -1;
    const bool f2 = (uint8_t)x.flag2, g2 = (uint8_t)y.flag2;
    if (f2 != g2) return f2 ? 1 : -1;
    return 0;
}
static const uint8_t water_key_compare_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0x4c, 0x24, 0x24};
REWRITE("Water::Step973040", 0x973040, water_key_compare_original, &water_key_compare, &g_water_key_compares);

// ---- Water::Cell93DBF0(this, x, y, out value, out both) (0x93DBF0): the object filling both halves of a water cell ---
// Cells of 0x30 bytes (+0x5C; width +0x58, height +0x59, bytes; signed checks), each two halves of 0x18 bytes with an
// object (+4) and flags (+0x14, low 3 bits). When both halves hold the same object and both have flags: it is given,
// with its +0x30 written out and "both" set when both halves have all three flags (else cleared). Otherwise 0.
static volatile long g_water_cells;
static uint8_t *__thiscall water_cell_93dbf0(const uint8_t *self, int32_t x, int32_t y, uint32_t *value, uint8_t *both) {
    g_water_cells++;
    if (y < 0 || y >= self[0x59] || x < 0 || x >= self[0x58]) return nullptr;
    const uint8_t *const a = *(const uint8_t *const *)(self + 0x5c) + 0x30 * ((uint32_t)(uint8_t)y * self[0x58] + (uint8_t)x), *const b = a + 0x18;
    uint8_t *const object = *(uint8_t *const *)(a + 4);
    if (!object || !*(const uint32_t *)(b + 4) || object != *(uint8_t *const *)(b + 4)) return nullptr;
    const uint32_t fa = *(const uint32_t *)(a + 0x14) & 7, fb = *(const uint32_t *)(b + 0x14) & 7;
    if (fa == 7 && fb == 7) {
        *(volatile uint8_t *)both = 1;
    } else {
        if (!fa || !fb) return nullptr;
        *(volatile uint8_t *)both = 0;
    }
    *(volatile uint32_t *)value = *(const uint32_t *)(object + 0x30);
    return object;
}
static const uint8_t water_cell_93dbf0_original[] = {0x8b, 0x54, 0x24, 0x08, 0x85, 0xd2, 0x57, 0x0f};
REWRITE("Water::Cell93DBF0", 0x93dbf0, water_cell_93dbf0_original, &water_cell_93dbf0, &g_water_cells);

// ---- Shader::ScanCC4C50(bytes, size, count) (0xCC4C50): which of three byte ranges a block's bytes fall in ----------
// Over size * count bytes (stopping once all three are seen): bit 1 for a byte 1 to 0x55, 2 for 0x56 to 0xAA, 4 for 0xAB
// and up (0 counts for none).
static volatile long g_shader_scans;
static uint32_t __fastcall shader_scan_cc4c50(const uint8_t *bytes, uint32_t size, uint32_t count) {
    g_shader_scans++;
    const uint32_t n = size * count;
    uint32_t seen = 0;
    for (uint32_t i = 0; i < n && seen != 7; i++) {
        const uint8_t c = *bytes++;
        if (c >= 1 && c < 0x56) seen |= 1;
        if (c >= 0x56 && c < 0xab) seen |= 2;
        if (c >= 0xab) seen |= 4;
    }
    return seen;
}
static const uint8_t shader_scan_cc4c50_original[] = {0x55, 0x8b, 0xec, 0x83, 0xec, 0x18, 0xb8, 0xcc};
REWRITE("Shader::ScanCC4C50", 0xcc4c50, shader_scan_cc4c50_original, &shader_scan_cc4c50, &g_shader_scans);

// ---- Stream::StepC52120(this, out, range) (0xC52120): a request record for the part of a range the stream holds -----
// The stream holds +0x14 less +0x10 (its start) up to +0x18; the range is {from, to} (signed). out gets 8 values: a
// zero byte (the next three bytes are left: the game copies stack bytes there), +4, 0, 0, then the later start less
// the stream's, the later start less the range's from, the later of that start and the earlier end less the range's
// from, and +0x1C. Gives out.
static volatile long g_stream_requests;
static uint32_t *__thiscall stream_step_c52120(const uint8_t *self, uint32_t *out, const int32_t *range) {
    g_stream_requests++;
    const int32_t s = (int32_t)(*(const uint32_t *)(self + 0x14) - *(const uint32_t *)(self + 0x10));
    const int32_t e = *(const int32_t *)(self + 0x18);
    const int32_t lo = s < range[0] ? range[0] : s;
    const int32_t hi0 = e > range[1] ? range[1] : e;
    const int32_t hi = lo < hi0 ? hi0 : lo;
    volatile uint32_t *const o = out;
    *(volatile uint8_t *)out = 0;
    o[1] = *(const uint32_t *)(self + 4), o[2] = 0, o[3] = 0;
    o[4] = (uint32_t)(lo - s), o[5] = (uint32_t)(lo - range[0]), o[6] = (uint32_t)(hi - range[0]);
    o[7] = *(const uint32_t *)(self + 0x1c);
    return out;
}
static const uint8_t stream_step_c52120_original[] = {0x83, 0xec, 0x34, 0x33, 0xc0, 0x88, 0x44, 0x24};
REWRITE("Stream::StepC52120", 0xc52120, stream_step_c52120_original, &stream_step_c52120, &g_stream_requests);

// ---- Animal::EntryA6FAF0(this, a, b, i) (0xA6FAF0): the first triangle edge from triangle i on joining a and b -------
// Triangles are three points each at +0x48 (count of points +0x4C, so a third as many triangles). Edge 3t is the
// triangle's second and third points, 3t + 1 its third and first, 3t + 2 its first and second, either way round. -1
// when none.
static volatile long g_animal_edges;
static int32_t __thiscall animal_entry_a6faf0(const uint8_t *self, uint32_t a, uint32_t b, int32_t i) {
    g_animal_edges++;
    const int32_t triangles = *(const int32_t *)(self + 0x4c) / 3;
    for (; i < triangles; i++) {
        const uint32_t *const t = *(const uint32_t *const *)(self + 0x48) + 3 * i;
        const uint32_t p0 = t[0], p1 = t[1], p2 = t[2];
        if ((p1 == a && p2 == b) || (p1 == b && p2 == a)) return 3 * i;
        if ((p2 == a && p0 == b) || (p2 == b && p0 == a)) return 3 * i + 1;
        if ((p0 == a && p1 == b) || (p0 == b && p1 == a)) return 3 * i + 2;
    }
    return -1;
}
static const uint8_t animal_entry_a6faf0_original[] = {0x51, 0x8b, 0x51, 0x4c, 0xb8, 0x56, 0x55, 0x55};
REWRITE("Animal::EntryA6FAF0", 0xa6faf0, animal_entry_a6faf0_original, &animal_entry_a6faf0, &g_animal_edges);

// ---- Key::MatchesPattern(this, pattern) (0xBB2220): whether a key matches a pattern ----------------------------------
// Three 16-byte parts (+0, +0x10, +0x2C) must each be equal unless the pattern's is the wildcard (0xF9EBB4); a wildcard
// third part matches the rest too, otherwise the byte counts (+0x40) must be equal and their bytes (+0x44) too.
static volatile long g_key_matches;
static uint32_t __thiscall key_matches_pattern(const uint8_t *self, const uint8_t *pattern) {
    g_key_matches++;
    const void *const wildcard = (const void *)0xf9ebb4;
    auto same = [](const void *a, const void *b) {
        for (int i = 0; i < 4; i++)
            if (((const uint32_t *)a)[i] != ((const uint32_t *)b)[i]) return false;
        return true;
    };
    for (uint32_t at = 0; at <= 0x10; at += 0x10)
        if (!same(pattern + at, wildcard) && !same(self + at, pattern + at)) return 0;
    if (same(pattern + 0x2c, wildcard)) return 1;
    if (!same(self + 0x2c, pattern + 0x2c)) return 0;
    const uint32_t n = *(const uint32_t *)(self + 0x40);
    if (n != *(const uint32_t *)(pattern + 0x40)) return 0;
    if (!n) return 1;
    const uint8_t *const x = *(const uint8_t *const *)(self + 0x44), *const y = *(const uint8_t *const *)(pattern + 0x44);
    for (uint32_t i = 0; i < n; i++)
        if (x[i] != y[i]) return 0;
    return 1;
}
static const uint8_t key_matches_pattern_original[] = {0x8b, 0x44, 0x24, 0x04, 0x53, 0x55, 0x56, 0x57};
REWRITE("Key::MatchesPattern", 0xbb2220, key_matches_pattern_original, &key_matches_pattern, &g_key_matches);

// ---- GroupedList::MoveAcross(this, in) (0xBF4910): an object moved across the start of its group ---------------------
// The owner (+4) lists objects (+0x94, count +0x8C) in six groups, each with a start (+0x4C) and end (+0x50), 12 bytes
// apart. Found from the end of the list, in the first group whose end it is before: when it is before the group's start
// and not `in`, the start moves back by one and the object swaps places with the one there; when it is at or after the
// start and `in`, it swaps places with the one at the start and the start moves on by one. An object in no group is
// looked for further back.
static volatile long g_group_moves;
static void __thiscall grouped_move_across(uint8_t *self, uint32_t in) {
    g_group_moves++;
    auto owner = [&]() { return *(uint8_t *const *)(self + 4); };
    auto list = [&]() { return *(uint32_t *const *)(owner() + 0x94); };
    auto start = [&](int32_t g) -> volatile int32_t & { return *(volatile int32_t *)(owner() + 0x4c + 12 * g); };
    for (int32_t i = *(const int32_t *)(owner() + 0x8c) - 1; i >= 0; i--) {
        if (list()[i] != (uint32_t)(uintptr_t)self) continue;
        int32_t g = 0;
        while (g < 6 && !(i < *(const int32_t *)(owner() + 0x50 + 12 * g))) g++;
        if (g == 6) continue;
        if (i < start(g)) {
            if ((uint8_t)in) return;
            start(g) = start(g) - 1;
            volatile uint32_t *const p = list() + start(g);
            const uint32_t other = *p;
            *p = (uint32_t)(uintptr_t)self;
            ((volatile uint32_t *)list())[i] = other;
        } else {
            if (!(uint8_t)in) return;
            volatile uint32_t *const p = list() + start(g);
            const uint32_t other = *p;
            *p = (uint32_t)(uintptr_t)self;
            ((volatile uint32_t *)list())[i] = other;
            start(g) = start(g) + 1;
        }
        return;
    }
}
static const uint8_t grouped_move_across_original[] = {0x57, 0x8b, 0x79, 0x04, 0x8b, 0x97, 0x8c, 0x00};
REWRITE("GroupedList::MoveAcross", 0xbf4910, grouped_move_across_original, &grouped_move_across, &g_group_moves);

// ---- Resource::BinarySearch(this, at) (0xC3B130): where a place falls, by two binary searches ------------------------
// Nine 0x18-byte ranges at +0x6C ({count, -, -, parts, start +0x10, size +0x14}, sorted) and, in the one holding at,
// its 16-byte parts ({size, start, base, -}, sorted): the part's base plus at less its start, or 0 when at falls in
// none (or the part has no base). Unsigned.
static volatile long g_resource_searches;
static uint32_t __thiscall resource_binary_search(const uint8_t *self, uint32_t at) {
    g_resource_searches++;
    const uint8_t *const table = *(const uint8_t *const *)(self + 0x6c);
    uint32_t lo = 0, hi = 8, mid;
    for (;;) {
        mid = (hi + lo) >> 1;
        const uint32_t s = *(const uint32_t *)(table + 0x18 * mid + 0x10);
        if (at < s) {
            if (lo >= mid) return 0;
            hi = mid - 1;
        } else if (at >= *(const uint32_t *)(table + 0x18 * mid + 0x14) + s) {
            if (mid >= hi) return 0;
            lo = mid + 1;
        } else {
            break;
        }
    }
    const uint8_t *const range = *(const uint8_t *const *)(self + 0x6c) + 0x18 * mid;
    const uint32_t count = *(const uint32_t *)range;
    if (!count) return 0;
    const uint8_t *const parts = *(const uint8_t *const *)(range + 0xc);
    lo = 0, hi = count - 1;
    for (;;) {
        mid = (hi + lo) >> 1;
        const uint32_t s = *(const uint32_t *)(parts + 16 * mid + 4);
        if (at < s) {
            if (lo >= mid) return 0;
            hi = mid - 1;
        } else if (at >= *(const uint32_t *)(parts + 16 * mid) + s) {
            if (mid >= hi) return 0;
            lo = mid + 1;
        } else {
            break;
        }
    }
    const uint8_t *const part = *(const uint8_t *const *)(range + 0xc) + 16 * mid;
    const uint32_t base = *(const uint32_t *)(part + 8);
    if (!base) return 0;
    return base - *(const uint32_t *)(part + 4) + at;
}
static const uint8_t resource_binary_search_original[] = {0x51, 0x53, 0x55, 0x8b, 0x6c, 0x24, 0x10, 0x56};
REWRITE("Resource::BinarySearch", 0xc3b130, resource_binary_search_original, &resource_binary_search, &g_resource_searches);

// ---- SortedTable::Find(a, b, three, c, out) (0xE03CD0): a six-part key looked for in the sorted table ----------------
// The table (0x141DCE4, count 0x141DCDC) has 32-byte entries sorted by a and b (unsigned), then the three values given
// by pointer and c (signed). Gives 1 with the entry's place, or 0 with the place it would go.
static volatile long g_sorted_finds;
static uint8_t __fastcall sorted_table_find(uint32_t a, uint32_t b, const int32_t *three, int32_t c, int32_t *out) {
    g_sorted_finds++;
    int32_t lo = 0, hi = *(const volatile int32_t *)0x141dcdc - 1;
    while (lo <= hi) {
        const int32_t mid = lo + (hi - lo) / 2;
        const uint32_t *const e = (const uint32_t *)(*(const uint8_t *const volatile *)0x141dce4 + 32 * mid);
        int order;
        if (a != e[0]) order = a > e[0] ? 1 : -1;
        else if (b != e[1]) order = b > e[1] ? 1 : -1;
        else if (three[0] != (int32_t)e[2]) order = three[0] > (int32_t)e[2] ? 1 : -1;
        else if (three[1] != (int32_t)e[3]) order = three[1] > (int32_t)e[3] ? 1 : -1;
        else if (three[2] != (int32_t)e[4]) order = three[2] > (int32_t)e[4] ? 1 : -1;
        else if (c != (int32_t)e[5]) order = c > (int32_t)e[5] ? 1 : -1;
        else {
            *out = mid;
            return 1;
        }
        if (order > 0) lo = mid + 1;
        else hi = mid - 1;
    }
    *out = lo;
    return 0;
}
static const uint8_t sorted_table_find_original[] = {0x83, 0xec, 0x0c, 0x53, 0x55, 0x8b, 0xe9, 0x8b};
REWRITE("SortedTable::Find", 0xe03cd0, sorted_table_find_original, &sorted_table_find, &g_sorted_finds);

// ---- Entries::UpdateMatching(record, list) (0xB786B0): a record copied over each list entry with the same keys -------
// The list {entries, -, count} holds 0x70-byte entries; one whose +0x10 and +0xC equal the record's takes its values
// +0 to +0x1C, the byte +0x20, +0x24 to +0x34, +0x40, +0x44, +0x48, +0x50, +0x54 and the byte +0x58. Gives the count.
static volatile long g_entry_updates;
static int32_t __stdcall entries_update_matching(const uint8_t *record, const uint8_t *list) {
    g_entry_updates++;
    int32_t n = *(const volatile int32_t *)(list + 8);
    for (int32_t i = 0; i < n; i++, n = *(const volatile int32_t *)(list + 8)) {
        uint8_t *const e = *(uint8_t *const *)list + 0x70 * i;
        if (*(const uint32_t *)(record + 0x10) != *(const uint32_t *)(e + 0x10) || *(const uint32_t *)(record + 0xc) != *(const uint32_t *)(e + 0xc)) continue;
        auto copy = [&](uint32_t at) { *(volatile uint32_t *)(e + at) = *(const uint32_t *)(record + at); };
        for (uint32_t at = 0; at <= 0x1c; at += 4) copy(at);
        *(volatile uint8_t *)(e + 0x20) = record[0x20];
        for (uint32_t at = 0x24; at <= 0x34; at += 4) copy(at);
        copy(0x40), copy(0x44), copy(0x48), copy(0x50), copy(0x54);
        *(volatile uint8_t *)(e + 0x58) = record[0x58];
    }
    return n;
}
static const uint8_t entries_update_matching_original[] = {0x55, 0x57, 0x8b, 0x7c, 0x24, 0x10, 0x8b, 0x47};
REWRITE("Entries::UpdateMatching", 0xb786b0, entries_update_matching_original, &entries_update_matching, &g_entry_updates);

// ---- Span::Clip(this, out, range) (0xC52060): a span record for the part of this range inside the range given --------
// This range runs from +0x14 less +0x10 for +0xC; the range given is {from, to} (signed). out gets 8 values: a zero byte
// (the next three bytes are left: the game copies stack bytes there), +4, 0, 0, the later start less this start, the
// later start, the later of that start and the earlier end, and +0x1C. Gives out.
static volatile long g_span_clips;
static uint32_t *__thiscall span_clip(const uint8_t *self, uint32_t *out, const int32_t *range) {
    g_span_clips++;
    const int32_t s = (int32_t)(*(const uint32_t *)(self + 0x14) - *(const uint32_t *)(self + 0x10));
    const int32_t e = (int32_t)(*(const uint32_t *)(self + 0xc) + (uint32_t)s);
    const int32_t lo = s < range[0] ? range[0] : s;
    const int32_t hi0 = e > range[1] ? range[1] : e;
    const int32_t hi = lo < hi0 ? hi0 : lo;
    volatile uint32_t *const o = out;
    *(volatile uint8_t *)out = 0;
    o[1] = *(const uint32_t *)(self + 4), o[2] = 0, o[3] = 0;
    o[4] = (uint32_t)(lo - s), o[5] = (uint32_t)lo, o[6] = (uint32_t)hi, o[7] = *(const uint32_t *)(self + 0x1c);
    return out;
}
static const uint8_t span_clip_original[] = {0x83, 0xec, 0x34, 0x8b, 0x51, 0x14, 0x53, 0x8b};
REWRITE("Span::Clip", 0xc52060, span_clip_original, &span_clip, &g_span_clips);

// ---- PairIds::Get(a, b) (0x466970): the number of an unordered pair of numbers --------------------------------------
// Rows of 0x14C bytes at 0x111EFE0, one per smaller number: a count, then up to 40 larger numbers (+4) and their pair
// numbers (+0xA4). A pair not there yet gets the next number (0x12DFFE8, counted up) and a 16-byte record at 0x1106E90
// {smaller, larger, -1, 0 byte}, and is added to the row. Signed.
static volatile long g_pair_ids;
static uint32_t __fastcall pair_ids_get(int32_t a, int32_t b) {
    g_pair_ids++;
    const int32_t lo = a > b ? b : a, hi = a < b ? b : a;
    volatile int32_t *const row = (volatile int32_t *)(0x111efe0 + 0x14c * lo);
    const int32_t n = row[0];
    for (int32_t i = 0; i < row[0]; i++)
        if (row[1 + i] == hi) return (uint32_t)row[0x29 + i];
    volatile int32_t *const count = (volatile int32_t *)0x12dffe8;
    volatile int32_t *const rec = (volatile int32_t *)(0x1106e90 + 16 * *count);
    rec[0] = lo, rec[1] = hi;
    *(volatile uint8_t *)(rec + 3) = 0;
    rec[2] = -1;
    row[1 + n] = hi;
    row[0x29 + row[0]] = *count;
    row[0] = row[0] + 1;
    const int32_t number = *count;
    *count = number + 1;
    return (uint32_t)number;
}
static const uint8_t pair_ids_get_original[] = {0x83, 0xec, 0x10, 0x53, 0x55, 0x56, 0x57, 0x89};
REWRITE("PairIds::Get", 0x466970, pair_ids_get_original, &pair_ids_get, &g_pair_ids);

// ---- Grid::QueueCell(this, x, y, -) (0xBA30B0): a grid cell's flagged objects moved to the queue -------------------------
// Cells of 0x18 bytes (+0x6C, width +0x64, height +0x68; skipped outside or with a negative first value) list objects
// (+0xC, count +0x10). Each with its byte +0x60 set has its link (+0x4C) taken out of its list (when properly linked) and
// put last in the list at +0x8C, and its +0x58 set to 1.0.
static volatile long g_cell_queues;
static void __thiscall grid_queue_cell(const uint8_t *self, int32_t x, int32_t y, uint32_t) {
    g_cell_queues++;
    if (x < 0 || y < 0 || x >= *(const int32_t *)(self + 0x64) || y >= *(const int32_t *)(self + 0x68)) return;
    const uint8_t *const cell = *(const uint8_t *const *)(self + 0x6c) + 0x18 * (*(const int32_t *)(self + 0x64) * y + x);
    if (*(const int32_t *)cell < 0) return;
    for (int32_t i = 0; i < *(const volatile int32_t *)(cell + 0x10); i++) {
        uint8_t *const obj = (*(uint8_t *const *const *)(cell + 0xc))[i];
        if (!obj[0x60]) continue;
        volatile uint32_t *const link = (volatile uint32_t *)(obj + 0x4c);
        if (link) {
            const uint32_t next = link[1], prev = link[0];
            if (next && prev && ((volatile uint32_t *)(uintptr_t)prev)[1] == (uint32_t)(uintptr_t)link && *(volatile uint32_t *)(uintptr_t)next == (uint32_t)(uintptr_t)link) {
                ((volatile uint32_t *)(uintptr_t)prev)[1] = next;
                *(volatile uint32_t *)(uintptr_t)link[1] = link[0];
                link[0] = 0;
                link[1] = 0;
            }
        }
        volatile uint32_t *const head = *(volatile uint32_t *const *)(self + 0x8c);
        link[1] = head[1];
        *(volatile uint32_t *)(uintptr_t)head[1] = (uint32_t)(uintptr_t)link;
        link[0] = (uint32_t)(uintptr_t)head;
        head[1] = (uint32_t)(uintptr_t)link;
        *(volatile uint32_t *)((*(uint8_t *const *const *)(cell + 0xc))[i] + 0x58) = 0x3f800000;
    }
}
static const uint8_t grid_queue_cell_original[] = {0x8b, 0x54, 0x24, 0x04, 0x55, 0x8b, 0xe9, 0x33};
REWRITE("Grid::QueueCell", 0xba30b0, grid_queue_cell_original, &grid_queue_cell, &g_cell_queues);

// ---- Grid::MarkRectForOwner(this, owner, x0, y0, x1, y1) (0x71CB80): a rectangle marked in an owner's layers --------
// The corners are bytes, divided by 16. Each layer (32 bytes at +0x68, count +0x70) whose +4's +0xC is the owner has
// byte +0x18 of every cell (0x4C bytes at the layer's +0x14, rows of +0x60) in the rectangle set to 1.
static volatile long g_rect_marks;
static void __thiscall grid_mark_rect(const uint8_t *self, uint32_t owner, uint32_t x0_in, uint32_t y0_in, uint32_t x1_in, uint32_t y1_in) {
    g_rect_marks++;
    const int32_t x0 = (uint8_t)x0_in >> 4, y0 = (uint8_t)y0_in >> 4, x1 = (uint8_t)x1_in >> 4, y1 = (uint8_t)y1_in >> 4;
    if (*(const int32_t *)(self + 0x70) <= 0) return;
    for (int32_t i = 0; i < *(const volatile int32_t *)(self + 0x70); i++) {
        const uint8_t *const layer = *(const uint8_t *const *)(self + 0x68) + 32 * i;
        if (*(const uint32_t *)(*(const uint8_t *const *)(layer + 4) + 0xc) != owner) continue;
        for (int32_t y = y0; y <= y1; y++) {
            volatile uint8_t *p = *(uint8_t *const *)(layer + 0x14) + 0x4c * (*(const int32_t *)(self + 0x60) * y + x0) + 0x18;
            for (int32_t x = x0; x <= x1; x++, p += 0x4c) *p = 1;
        }
    }
}
static const uint8_t grid_mark_rect_original[] = {0x0f, 0xb6, 0x44, 0x24, 0x0c, 0x0f, 0xb6, 0x54};
REWRITE("Grid::MarkRectForOwner", 0x71cb80, grid_mark_rect_original, &grid_mark_rect, &g_rect_marks);

// ---- Ride::Clamp4944D0(this, x0, y0, x1, y1) (0x4944D0): a rectangle of a grid's 16-bit cells cleared ----------------
// The grid: width +8, height +0xC, cells +0x10. The rectangle (ends included, signed) is first kept to the grid.
static volatile long g_ride_clears;
static void __thiscall ride_clamp_4944d0(const uint8_t *self, int32_t x0, int32_t y0, int32_t x1, int32_t y1) {
    g_ride_clears++;
    const int32_t w = *(const int32_t *)(self + 8), h = *(const int32_t *)(self + 0xc);
    const int32_t a = x0 > 0 ? x0 : 0, b = y0 > 0 ? y0 : 0;
    const int32_t c = w - 1 > x1 ? x1 : w - 1, d = h - 1 > y1 ? y1 : h - 1;
    for (int32_t y = b; y <= d; y++)
        for (int32_t x = a; x <= c; x++)
            if (x >= 0 && x < *(const int32_t *)(self + 8) && y >= 0 && y < *(const int32_t *)(self + 0xc))
                *(volatile uint16_t *)(*(uint8_t *const *)(self + 0x10) + 2 * (*(const int32_t *)(self + 8) * y + x)) = 0;
}
static const uint8_t ride_clamp_4944d0_original[] = {0x83, 0xec, 0x0c, 0x56, 0xb8, 0x00, 0x00, 0x00};
REWRITE("Ride::Clamp4944D0", 0x4944d0, ride_clamp_4944d0_original, &ride_clamp_4944d0, &g_ride_clears);

// ---- Mask::ToPixels(this, image, stride) (0x7AEB50): a bit mask's rows written into an image ------------------------
// Rows from the byte +0x14 to +0x15, each with its first and last column (bytes +0x16 + row and +0x116 + row) and 256
// bits at +0x228 + 32 * row: a pixel (4 bytes, rows of stride pixels) is 0x7FFFFFFF where the bit is set, else 0xFFFFFF.
static volatile long g_mask_pixels;
static void __thiscall mask_to_pixels(const uint8_t *self, uint32_t *image, int32_t stride) {
    g_mask_pixels++;
    for (int32_t row = self[0x14]; row <= *(const volatile uint8_t *)(self + 0x15); row++) {
        const uint32_t *const bits = (const uint32_t *)(self + 0x228 + 32 * row);
        const int32_t last = self[0x116 + row];
        for (int32_t x = self[0x16 + row]; x <= last; x++)
            ((volatile uint32_t *)image)[row * stride + x] = (bits[x >> 5] & (1u << (x & 31))) ? 0x7fffffffu : 0xffffffu;
    }
}
static const uint8_t mask_to_pixels_original[] = {0x83, 0xec, 0x08, 0x0f, 0xb6, 0x41, 0x15, 0x57};
REWRITE("Mask::ToPixels", 0x7aeb50, mask_to_pixels_original, &mask_to_pixels, &g_mask_pixels);

// ---- Description::Differs(a, b) (0x970C80): whether two descriptions differ --------------------------------------------
// 1 when a is missing, or their kinds (+0) or +0xB8 differ; kinds 1 and 2 then compare +0xBC, +0xC0, +8 and +0xC, and
// kind 1 also +0x54 and the byte +0x60.
static volatile long g_description_differs;
static uint8_t __stdcall description_differs(const uint8_t *a, const uint8_t *b) {
    g_description_differs++;
    if (!a) return 1;
    auto v = [](const uint8_t *p, uint32_t at) { return *(const uint32_t *)(p + at); };
    if (v(b, 0) != v(a, 0) || v(b, 0xb8) != v(a, 0xb8)) return 1;
    const uint32_t kind = v(a, 0);
    if (kind != 1 && kind != 2) return 0;
    if (v(b, 0xbc) != v(a, 0xbc) || v(b, 0xc0) != v(a, 0xc0) || v(b, 8) != v(a, 8) || v(b, 0xc) != v(a, 0xc)) return 1;
    if (kind == 2) return 0;
    return v(b, 0x54) != v(a, 0x54) || b[0x60] != a[0x60];
}
static const uint8_t description_differs_original[] = {0x8b, 0x44, 0x24, 0x04, 0x85, 0xc0, 0x0f, 0x84};
REWRITE("Description::Differs", 0x970c80, description_differs_original, &description_differs, &g_description_differs);

// ---- Text::Find* (0xCD3070 to 0xCD3BB0, ten copies): two places in texts compared without case, from there on ------
// fastcall (a, b). Each side is either a text place {text holder (text: length +4, characters +0xC), position} or a
// pointer to characters ending at 0, and its characters are bytes (sign-extended) or 16-bit; the copies differ only in
// that (TEXT8 etc. below). A side is at its end when it has no text (or pointer) or its position is past the length
// (or its character is 0). While neither is at its end, the characters (0 past a text's length) go through the game's
// _tolower (0xDDE005) and the first difference gives -1 or 1 (signed); both sides move on. Then: 1 when a has more,
// else -1 when b has more, else 0.
namespace {
enum { TEXT8, TEXT16, CHARS8, CHARS16 };
template <int K> struct TextSide {
    uint32_t *at;
    bool none() const { return !at[0]; }
    bool end() const {
        if (K == TEXT8 || K == TEXT16) return !at[0] || at[1] >= *(const uint32_t *)(**(uint8_t *const *const *)at + 4);
        return !at[0] || (K == CHARS8 ? !*(const uint8_t *)(uintptr_t)at[0] : !*(const uint16_t *)(uintptr_t)at[0]);
    }
    int32_t get() const {
        if (K == CHARS8) return *(const int8_t *)(uintptr_t)at[0];
        if (K == CHARS16) return *(const uint16_t *)(uintptr_t)at[0];
        const uint8_t *const text = **(uint8_t *const *const *)at;
        if (at[1] >= *(const uint32_t *)(text + 4)) return 0;
        const uint8_t *const c = *(const uint8_t *const *)(text + 0xc);
        return K == TEXT8 ? (int32_t)((const int8_t *)c)[at[1]] : (int32_t)((const uint16_t *)c)[at[1]];
    }
    void next() {
        if (K == TEXT8 || K == TEXT16) ((volatile uint32_t *)at)[1]++;
        else *(volatile uint32_t *)at += K == CHARS8 ? 1 : 2;
    }
};
}
static volatile long g_text_place_compares;
template <int A, int B> static int32_t __fastcall text_places_compare(uint32_t *a_at, uint32_t *b_at) {
    g_text_place_compares++;
    TextSide<A> a{a_at};
    TextSide<B> b{b_at};
    if (a.none() || b.none()) {
        if (!a.end()) return 1;
        return b.end() ? 0 : -1;
    }
    int (__cdecl *const lower)(int) = (int(__cdecl *)(int))0xdde005;
    while (!a.end() && !b.end()) {
        const int32_t x = lower(a.get());
        const int32_t y = lower(b.get());
        if (x < y) return -1;
        if (x > y) return 1;
        a.next(), b.next();
    }
    if (!a.end()) return 1;
    return b.end() ? 0 : -1;
}
static const uint8_t text_places_original[] = {0x56, 0x57, 0x8b, 0xf9, 0x8b, 0x07, 0x85, 0xc0};
static const uint8_t text_places_620_original[] = {0x53, 0x56, 0x8b, 0xf1, 0x8b, 0x06, 0x85, 0xc0};
REWRITE("Text::FindCD3700", 0xcd3700, text_places_original, (&text_places_compare<TEXT8, TEXT8>), &g_text_place_compares);
REWRITE("Text::FindCD3800", 0xcd3800, text_places_original, (&text_places_compare<TEXT8, TEXT16>), &g_text_place_compares);
REWRITE("Text::FindCD3AB0", 0xcd3ab0, text_places_original, (&text_places_compare<TEXT16, TEXT8>), &g_text_place_compares);
REWRITE("Text::FindCD3BB0", 0xcd3bb0, text_places_original, (&text_places_compare<TEXT16, TEXT16>), &g_text_place_compares);
REWRITE("Text::FindCD3070", 0xcd3070, text_places_original, (&text_places_compare<CHARS8, TEXT8>), &g_text_place_compares);
REWRITE("Text::FindCD3150", 0xcd3150, text_places_original, (&text_places_compare<CHARS8, TEXT16>), &g_text_place_compares);
REWRITE("Text::FindCD3390", 0xcd3390, text_places_original, (&text_places_compare<CHARS16, TEXT8>), &g_text_place_compares);
REWRITE("Text::FindCD3470", 0xcd3470, text_places_original, (&text_places_compare<CHARS16, TEXT16>), &g_text_place_compares);
REWRITE("Text::FindCD3620", 0xcd3620, text_places_620_original, (&text_places_compare<TEXT8, CHARS16>), &g_text_place_compares);
REWRITE("Text::FindCD39D0", 0xcd39d0, text_places_620_original, (&text_places_compare<TEXT16, CHARS16>), &g_text_place_compares);

// ---- Text::CompareCD5150 / CD5260 / CD5640 / CD5370(a, b) (stdcall): two whole texts compared without case ---------
// The same comparison from the start of each: a and b are text holders (or, for CD5370, b points to bytes ending at 0);
// characters bytes or 16-bit as in each copy's name below. The places are its own, not written back.
template <int A, int B> static int32_t __stdcall text_whole_compare(uint32_t a, uint32_t b) {
    uint32_t pa[2] = {a, 0}, pb[2] = {b, 0};
    return text_places_compare<A, B>(pa, pb);
}
static const uint8_t text_whole_original[] = {0x8b, 0x54, 0x24, 0x08, 0x55, 0x8b, 0x6c, 0x24};
static const uint8_t text_whole_370_original[] = {0x53, 0x8b, 0x5c, 0x24, 0x0c, 0x55, 0x8b, 0x6c};
REWRITE("Text::CompareCD5150", 0xcd5150, text_whole_original, (&text_whole_compare<TEXT8, TEXT8>), &g_text_place_compares);
REWRITE("Text::CompareCD5260", 0xcd5260, text_whole_original, (&text_whole_compare<TEXT8, TEXT16>), &g_text_place_compares);
REWRITE("Text::CompareCD5640", 0xcd5640, text_whole_original, (&text_whole_compare<TEXT16, TEXT16>), &g_text_place_compares);
REWRITE("Text::CompareCD5370", 0xcd5370, text_whole_370_original, (&text_whole_compare<TEXT16, CHARS8>), &g_text_place_compares);

// ---- Water::Range93DCD0(this, x, y, reach) (0x93DCD0): the nearest fully watered cell, ring by ring ------------------
// x and y are given by pointer. For each ring r = 0 up to reach (none when reach is negative) the cells on its square
// are tried with Water::Cell93DBF0: first the left and right columns (x - r and x + r) from y - r down to y + r, then
// the top and bottom rows (y - r and y + r) between them; a cell whose two halves are both full counts, and the
// nearest (squared distance, ties to the first) wins. At the end of the first ring with one, its place is written back
// and 1 given; else 0. The two pointers are read again for each ring's bounds.
static volatile long g_water_ranges;
static uint8_t __thiscall water_range_93dcd0(uint8_t *self, int32_t *px, int32_t *py, int32_t reach) {
    g_water_ranges++;
    typedef void *(__thiscall *Cell)(void *, int32_t, int32_t, uint32_t *, uint8_t *);
    const Cell cell = (Cell)0x93dbf0;
    if (reach < 0) return 0;
    const int32_t x = *px, y = *py;
    int32_t best = 100000000, bx = 0, by = 0;
    bool found = false;
    auto look = [&](int32_t cx, int32_t cy, int32_t d1, int32_t d2) {
        uint32_t value;
        volatile uint8_t both = 0;
        cell(self, cx, cy, &value, (uint8_t *)&both);
        if (!both) return;
        const int32_t d = (int32_t)((uint32_t)d1 * (uint32_t)d1 + (uint32_t)d2 * (uint32_t)d2);
        if (d < best) best = d, by = cy, bx = cx, found = true;
    };
    for (int32_t r = 0; r <= reach; r++) {
        int32_t k = r;
        for (int32_t cy = y - r; cy < *py + r + 1; cy++, k--) {
            look(x - r, cy, r, k);
            look(x + r, cy, -r, k);
        }
        k = r - 1;
        for (int32_t cx = x + 1 - r; cx < *px + r && x + 1 - r < x + r; cx++, k--) {
            look(cx, y - r, r, k);
            look(cx, y + r, -r, k);
        }
        if (found) {
            *py = by;
            *px = bx;
            return 1;
        }
    }
    return 0;
}
static const uint8_t water_range_93dcd0_original[] = {0x83, 0xec, 0x48, 0x8b, 0x44, 0x24, 0x54, 0x53};
REWRITE("Water::Range93DCD0", 0x93dcd0, water_range_93dcd0_original, &water_range_93dcd0, &g_water_ranges);

// ---- Water::Find90D380(this, key) (0x90D380): an entry taken out of a sorted table, its parts packed up behind it -----
// The table (+0x78, count +0x80) has 20-byte entries {key, first record, record count, first index, index count}; the
// place is SortedTable::Find20's (found or not). Each later entry moves down one, its records (16 bytes at +0x84, the
// first 12 copied) and 16-bit indexes (+0x90, each less the distance its records moved) moved down to follow the ones
// before; the next free record and index (+0x8C, +0x98) are where they end, and the count goes down by one (given).
static volatile long g_water_finds;
static int32_t __thiscall water_find_90d380(uint8_t *self, uint32_t key) {
    g_water_finds++;
    typedef uint8_t(__fastcall *Find)(const uint32_t *, const uint8_t *, int32_t, int32_t *, int32_t *);
    uint32_t k[5] = {key};
    int32_t place, unused;
    ((Find)0x90c5c0)(k, *(const uint8_t *const *)(self + 0x78), *(const int32_t *)(self + 0x80), &place, &unused);
    auto entry = [&](int32_t i) { return (volatile uint32_t *)(*(uint8_t *const *)(self + 0x78) + 20 * i); };
    uint32_t records = entry(place)[1], indexes = entry(place)[3];
    for (int32_t i = place; i < *(const volatile int32_t *)(self + 0x80) - 1; i++) {
        volatile uint32_t *const e = entry(i);
        for (int w = 0; w < 5; w++) e[w] = e[5 + w];
        const uint32_t moved = e[1] - records;
        for (int32_t j = 0; j < (int32_t)e[2]; j++) {
            volatile uint32_t *const to = (volatile uint32_t *)(*(uint8_t *const *)(self + 0x84) + 16 * (records + j));
            const uint32_t *const from = (const uint32_t *)(*(uint8_t *const *)(self + 0x84) + 16 * (e[1] + j));
            to[0] = from[0], to[1] = from[1], to[2] = from[2];
        }
        for (int32_t j = 0; j < (int32_t)e[4]; j++) {
            volatile uint16_t *const list = *(uint16_t *const *)(self + 0x90);
            list[indexes + j] = (uint16_t)(list[e[3] + j] - (uint16_t)moved);
        }
        const uint32_t count = e[2], index_count = e[4];
        e[1] = records, records += count;
        e[3] = indexes, indexes += index_count;
    }
    const int32_t n = *(const volatile int32_t *)(self + 0x80) - 1;
    *(volatile uint32_t *)(self + 0x8c) = records;
    *(volatile uint32_t *)(self + 0x98) = indexes;
    *(volatile int32_t *)(self + 0x80) = n;
    return n;
}
static const uint8_t water_find_90d380_original[] = {0x83, 0xec, 0x24, 0x8b, 0x44, 0x24, 0x28, 0x53};
REWRITE("Water::Find90D380", 0x90d380, water_find_90d380_original, &water_find_90d380, &g_water_finds);

// ---- Animal::AverageAE0890(this, out) (0xAE0890): the bounds of a list of points ------------------------------------
// Points are {x, y} floats at +0x40 (count +0x50). out gets {least x, least y, most x, most y}, each starting at 0: a
// point's value replaces the least unless it is above it, and the most unless it is below it (so a NaN always replaces
// the least and never the most).
static volatile long g_animal_bounds;
static void __thiscall animal_bounds(const uint8_t *self, float *out) {
    g_animal_bounds++;
    volatile float lx = 0.0f, ly = 0.0f, mx = 0.0f, my = 0.0f;
    for (int32_t i = 0; i < *(const volatile int32_t *)(self + 0x50); i++) {
        const float *const p = *(const float *const *)(self + 0x40) + 2 * i;
        const float x = p[0], y = p[1];
        lx = x > lx ? (float)lx : x;
        ly = y > ly ? (float)ly : y;
        mx = !(x >= mx) ? (float)mx : x;
        my = !(y >= my) ? (float)my : y;
    }
    volatile float *const o = out;
    o[0] = lx, o[1] = ly, o[2] = mx, o[3] = my;
}
static const uint8_t animal_bounds_original[] = {0x83, 0xec, 0x24, 0x8b, 0x51, 0x50, 0x33, 0xc0};
REWRITE("Animal::AverageAE0890", 0xae0890, animal_bounds_original, &animal_bounds, &g_animal_bounds);

// ---- PhysicsWorld::Integrate(this) (0xA6C3A0): one Verlet step for each particle (+0x40, count +0x48) -------------
// position (+0x10) += damping (+0x30) x (position - previous (+0x20)) + force (+0x40) x the particle's +0 x the step
// factor (0x13429F0); the old position becomes the previous one. As the game rounds: d px, d qx, d qz, the factor k m,
// k m fx, fy (k m), fz (k m), the x and y differences and the three added steps go through floats; d py, d pz and d qy,
// the z difference and the x step sum stay on the FPU.
static volatile long g_physics_steps;
static void __fastcall physics_integrate(const uint8_t *self) {
    g_physics_steps++;
    for (int32_t i = 0; i < *(const volatile int32_t *)(self + 0x48); i++) {
        float *const p = (*(float *const *const *)(self + 0x40))[i];
        const float px = p[4], py = p[5], pz = p[6], qx = p[8], qy = p[9], qz = p[10], d = p[12];
        volatile float t10 = (float)((long double)d * px);
        const long double e = (long double)py * d, f = (long double)pz * d;
        volatile float t30 = (float)((long double)d * qx);
        const long double h = (long double)qy * d;
        volatile float t38 = (float)((long double)qz * d);
        const long double k = (long double)*(const volatile float *)0x13429f0 * p[0];
        volatile float tc = (float)k;
        volatile float t20 = (float)(k * p[16]);
        volatile float t24 = (float)((long double)p[17] * tc);
        volatile float t28 = (float)((long double)p[18] * tc);
        t10 = (float)((long double)t10 - t30);
        volatile float t14 = (float)(e - h);
        const long double l = f - t38;
        const long double m = (long double)t20 + t10;
        t14 = (float)((long double)t24 + t14);
        volatile float t18 = (float)((long double)t28 + l);
        volatile float *const v = p;
        v[4] = (float)(m + px);
        v[5] = (float)((long double)t14 + v[5]);
        v[6] = (float)((long double)t18 + v[6]);
        *(volatile uint32_t *)(p + 10) = *(const uint32_t *)&pz;
        v[8] = px;
        v[9] = py;
    }
}
static const uint8_t physics_integrate_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x4c, 0x8b, 0x41, 0x48};
REWRITE("PhysicsWorld::Integrate", 0xa6c3a0, physics_integrate_original, &physics_integrate, &g_physics_steps);

// ---- Cell::Quad(this, out, terrain) (0x729530): the four corner points of a terrain cell -----------------------------
// The cell's place is the bytes +0x88 (x) and +0x89 (y); the terrain gives the cell size (+0x14, +0x18), its origin
// (+0xC, +0x10), its width (+4 byte) and its cells (+0x24, 0x18 bytes each, the four corner heights first). out gets four
// 16-byte points {x, height, y}: (x0, y0), (x1, y0), (x0, y1), (x1, y1), x1 and y1 one size on. x0, x1 and y0 stay on the
// FPU until stored; y1 is rounded once and copied. Gives the last height's bits.
static volatile long g_cell_quads;
static uint32_t __thiscall cell_quad(const uint8_t *self, uint8_t *out, const uint8_t *terrain) {
    g_cell_quads++;
    const float sx = *(const float *)(terrain + 0x14), sy = *(const float *)(terrain + 0x18);
    const long double x0 = (long double)(int32_t)self[0x88] * sx + *(const float *)(terrain + 0xc);
    const long double x1 = x0 + sx;
    const long double y0 = (long double)(int32_t)self[0x89] * sy + *(const float *)(terrain + 0x10);
    volatile float y1 = (float)(y0 + sy);
    volatile float *const o = (volatile float *)out;
    o[0] = (float)x0;
    *(volatile uint32_t *)(out + 0x28) = *(volatile uint32_t *)&y1;
    *(volatile uint32_t *)(out + 0x38) = *(volatile uint32_t *)&y1;
    o[2] = (float)y0;
    o[4] = (float)x1;
    o[6] = (float)y0;
    o[8] = (float)x0;
    o[12] = (float)x1;
    uint32_t h = 0;
    for (int k = 0; k < 4; k++) {
        const uint32_t i = (uint32_t)terrain[4] * self[0x89] + self[0x88];
        h = *(const uint32_t *)(*(const uint8_t *const *)(terrain + 0x24) + 0x18 * i + 4 * k);
        *(volatile uint32_t *)(out + 4 + 0x10 * k) = h;
    }
    return h;
}
static const uint8_t cell_quad_original[] = {0x0f, 0xb6, 0x81, 0x88, 0x00, 0x00, 0x00, 0x8b};
REWRITE("Cell::Quad", 0x729530, cell_quad_original, &cell_quad, &g_cell_quads);

// ---- Model::BlendCCF530(n, m, t, from, to, out) (0xCCF530): n byte colours blended and turned by a 3x3 matrix -------
// fastcall n and m. a = t K and b = (1 - t) K (floats; K 0xFC344C, the 1 at 0xF264DC); each of the n three-byte
// colours is {to a + from b + c} per part (c 0xFC3450; kept on the FPU), and out gets (16 bytes apart) its product with
// the matrix's columns (rows 16 bytes apart): x m0 + y m4 + z m8, and so on.
static volatile long g_model_blends;
static uint32_t __fastcall model_blend_ccf530(int32_t n, const float *m, float t, const uint8_t *from, const uint8_t *to, float *out) {
    g_model_blends++;
    const float k = *(const float *)0xfc344c;
    volatile float a = (float)((long double)t * k);
    volatile float b = (float)(((long double)*(const float *)0xf264dc - t) * k);
    const long double c = *(const float *)0xfc3450;
    const long double la = a, lb = b;
    do {
        const long double z = ((long double)(int32_t)to[2] * la + (long double)(int32_t)from[2] * lb) + c;
        const long double y = ((long double)(int32_t)to[1] * la + (long double)(int32_t)from[1] * lb) + c;
        const long double x = ((long double)(int32_t)to[0] * la + (long double)(int32_t)from[0] * lb) + c;
        volatile float *const o = out;
        o[0] = (float)((x * m[0] + y * m[4]) + z * m[8]);
        o[1] = (float)((x * m[1] + y * m[5]) + z * m[9]);
        o[2] = (float)((x * m[2] + y * m[6]) + z * m[10]);
        from += 3, to += 3, out += 4;
    } while (--n);
    return 0;
}
static const uint8_t model_blend_ccf530_original[] = {0x55, 0x8b, 0xec, 0x83, 0xec, 0x14, 0xd9, 0x45, 0x08};
REWRITE("Model::BlendCCF530", 0xccf530, model_blend_ccf530_original, &model_blend_ccf530, &g_model_blends);

static inline long double x87_sqrt_h2(long double v) {
    __asm__("fsqrt" : "+t"(v));
    return v;
}

// ---- Water::Segment925AC0(px, py, ax, ay, bx, by, out) (0x925AC0): the nearest point of a segment and how far ---------
// d = b - a (floats); t = (p - a) . d. When t (unrounded) is not above the game's 0 (0xF264E0) the nearest point is a;
// when |d|^2 is not above t (as a float) it is b; both give the game's constant 0xF268F0. Otherwise it is a + d t / |d|^2
// (each part rounded to a float, written to out) and the distance to it is given on the FPU, unrounded. A NaN goes on.
static volatile long g_water_segments;
static long double __stdcall water_segment(float px, float py, float ax, float ay, float bx, float by, float *out) {
    g_water_segments++;
    volatile float dx = (float)((long double)bx - ax), dy = (float)((long double)by - ay);
    const long double tl = ((long double)py - ay) * dy + ((long double)px - ax) * dx;
    volatile float t = (float)tl;
    const float zero = *(const float *)0xf264e0;
    if (!(tl > zero) && !(tl != tl || zero != zero)) {
        out[0] = ax, out[1] = ay;
        return *(const float *)0xf268f0;
    }
    const long double l = (long double)dy * dy + (long double)dx * dx;
    const float tf = t;
    if (!(l > tf) && !(l != l || tf != tf)) {
        out[0] = bx, out[1] = by;
        return *(const float *)0xf268f0;
    }
    const long double s = (long double)tf / l;
    volatile float sy = (float)((long double)dy * s);
    volatile float qx = (float)((long double)dx * s + ax);
    *(volatile float *)out = qx;
    volatile float qy = (float)((long double)sy + ay);
    *(volatile float *)(out + 1) = qy;
    const long double ex = (long double)px - out[0], ey = (long double)py - out[1];
    return x87_sqrt_h2(ey * ey + ex * ex);
}
static const uint8_t water_segment_original[] = {0x83, 0xec, 0x0c, 0xd9, 0x44, 0x24, 0x20, 0xd8};
REWRITE("Water::Segment925AC0", 0x925ac0, water_segment_original, &water_segment, &g_water_segments);

// ---- WildAnimal::NearB1CED0(animal, p, v, r, out) (0xB1CED0): a point pulled back along a direction when too far ------
// D = the flat distance from the animal (+0x80, +0x88) to p (x rounded to a float, z not). Within r, out = p. Beyond:
// S = (D - r) (C2 / (C1 + D)) / (|v| + C3) (constants 0xF2669C, 0xF2784C, 0xF26E2C) and out = p - S v (the y and z
// products rounded to floats, the x one not).
static volatile long g_animal_nears;
static void __stdcall wild_animal_near(const uint8_t *animal, const float *p, const float *v, float r, float *out) {
    g_animal_nears++;
    const float vx = v[0], vy = v[1], vz = v[2];
    volatile float dx = (float)((long double)*(const float *)(animal + 0x80) - p[0]);
    const long double dz = (long double)*(const float *)(animal + 0x88) - p[2];
    const long double d = x87_sqrt_h2(dz * dz + (long double)dx * dx);
    if (!(d > r) && !(d != d || r != r)) {
        volatile uint32_t *const o = (volatile uint32_t *)out;
        const uint32_t *const q = (const uint32_t *)p;
        o[0] = q[0], o[1] = q[1], o[2] = q[2];
        return;
    }
    const long double q = (long double)*(const float *)0xf2784c / ((long double)*(const float *)0xf2669c + d);
    const long double w = (d - r) * q;
    const long double n = x87_sqrt_h2(((long double)vz * vz + (long double)vy * vy) + (long double)vx * vx);
    const long double s = w / (n + *(const float *)0xf26e2c);
    const long double ax = (long double)vx * s;
    volatile float sy = (float)(s * vy);
    volatile float sz = (float)((long double)vz * s);
    const long double x = (long double)p[0] - ax;
    const long double y = (long double)p[1] - sy;
    const long double z = (long double)p[2] - sz;
    volatile float *const o = out;
    o[0] = (float)x, o[1] = (float)y, o[2] = (float)z;
}
static const uint8_t wild_animal_near_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x20, 0x8b, 0x45, 0x10};
REWRITE("WildAnimal::NearB1CED0", 0xb1ced0, wild_animal_near_original, &wild_animal_near, &g_animal_nears);

// A unit direction written out, or (0, 0, 1) when its length is exactly the game's 0 (0xF264E0); x and y come as
// floats, z unrounded; scaled by C1 / length (C1 0xF264DC), each part rounded once.
static void unit_or_up(float x, float y, long double z, float *out) {
    const long double len = x87_sqrt_h2((z * z + (long double)y * y) + (long double)x * x);
    if (len == *(const float *)0xf264e0) {
        volatile uint32_t *const o = (volatile uint32_t *)out;
        o[0] = 0, o[1] = 0, o[2] = 0x3f800000;
        return;
    }
    const long double inv = (long double)*(const float *)0xf264dc / len;
    volatile float *const o = out;
    o[0] = (float)((long double)x * inv);
    o[1] = (float)((long double)y * inv);
    o[2] = (float)(z * inv);
}
// the sum of two points negated (x and y rounded to floats, z not), as a unit direction
static void back_from(const float *e, const float *f, float *out) {
    volatile float sx = (float)(-(long double)f[0] + -(long double)e[0]);
    volatile float sy = (float)(-(long double)f[1] + -(long double)e[1]);
    unit_or_up(sx, sy, -(long double)f[2] + -(long double)e[2], out);
}

// ---- Ride::Last5BB3A0(this, out) (0x5BB3A0) and TrackPiece::Length5BB2B0 (0x5BB2B0): the way back from two points ---
// Lists at +0x2C and +0x28 hold {count, 0x24-byte entries}. Ride::Last takes each list's last entry's point (+0xC),
// TrackPiece::Length each first entry's (+0x18): back_from of the two.
static volatile long g_ride_lasts;
static void __thiscall ride_last_5bb3a0(const uint8_t *self, float *out) {
    g_ride_lasts++;
    auto last = [&](uint32_t at) {
        const uint32_t *const list = *(const uint32_t *const *)(self + at);
        return (const float *)(uintptr_t)(list[1] + 0x24 * list[0] - 0x18);
    };
    back_from(last(0x2c), last(0x28), out);
}
static void __thiscall track_piece_length(const uint8_t *self, float *out) {
    g_ride_lasts++;
    auto first = [&](uint32_t at) { return (const float *)(uintptr_t)((*(const uint32_t *const *)(self + at))[1] + 0x18); };
    back_from(first(0x2c), first(0x28), out);
}
static const uint8_t ride_last_5bb3a0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x30, 0x8b, 0x41, 0x2c};
REWRITE("Ride::Last5BB3A0", 0x5bb3a0, ride_last_5bb3a0_original, &ride_last_5bb3a0, &g_ride_lasts);
REWRITE("TrackPiece::Length5BB2B0", 0x5bb2b0, ride_last_5bb3a0_original, &track_piece_length, &g_ride_lasts);

// ---- TrackPiece::Last5BE8B0(this, out) (0x5BE8B0): the unit direction from one list's last point to the other's -----
// The last entries' points (+0) of the lists at +0x28 (from) and +0x2C (to): x as a float, y and z unrounded.
static void __thiscall track_piece_last(const uint8_t *self, float *out) {
    g_ride_lasts++;
    auto last = [&](uint32_t at) {
        const uint32_t *const list = *(const uint32_t *const *)(self + at);
        return (const float *)(uintptr_t)(list[1] + 0x24 * list[0] - 0x24);
    };
    const float *const e = last(0x28), *const f = last(0x2c);
    const long double dy = (long double)f[1] - e[1];
    volatile float dx = (float)((long double)f[0] - e[0]);
    const long double dz = (long double)f[2] - e[2];
    const long double len = x87_sqrt_h2((dz * dz + dy * dy) + (long double)dx * dx);
    if (len == *(const float *)0xf264e0) {
        volatile uint32_t *const o = (volatile uint32_t *)out;
        o[0] = 0, o[1] = 0, o[2] = 0x3f800000;
        return;
    }
    const long double inv = (long double)*(const float *)0xf264dc / len;
    volatile float *const o = out;
    o[0] = (float)((long double)dx * inv);
    o[1] = (float)(dy * inv);
    o[2] = (float)(dz * inv);
}
static const uint8_t track_piece_last_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x30, 0x8b, 0x41, 0x28};
REWRITE("TrackPiece::Last5BE8B0", 0x5be8b0, track_piece_last_original, &track_piece_last, &g_ride_lasts);

static inline long double x87_round_h2(long double v) {
    __asm__("frndint" : "+t"(v));
    return v;
}
static inline long double x87_sin_h2(long double v) {
    __asm__("fsin" : "+t"(v));
    return v;
}
static inline long double x87_cos_h2(long double v) {
    __asm__("fcos" : "+t"(v));
    return v;
}

// ---- Ride::Blend55FA00(this, t) (0x55FA00): three spinning parts moved on and their turn matrices made -----------------
// For each part j: its angle (+0x3DC) grows by t times the speed (+0x70) over its period (+0x3D0); the whole turns
// (angle x C (0xF36EE4), rounded down as a float) times a turn (0xF268DC) are taken off (the angle kept as a float).
// Its matrix (0x40 bytes from +0x310) gets cos and sin of the angle (floats) as a turn about z: {c, s} in the first row,
// {-s, c} in the second, and in the third and fourth {k1 - c k1 + s k2, k2 - s k1 - c k2} (k1 0xF3AF7C, k2 0xF3AF78;
// each rounded once and written to both).
static volatile long g_ride_spins;
static void __thiscall ride_blend_55fa00(uint8_t *self, float t) {
    g_ride_spins++;
    const float k1 = *(const float *)0xf3af7c, k2 = *(const float *)0xf3af78;
    for (int j = 0; j < 3; j++) {
        volatile float *const angle = (volatile float *)(self + 0x3dc + 4 * j);
        const long double v = (long double)t * *(const float *)(self + 0x70) / *(const float *)(self + 0x3d0 + 4 * j) + *angle;
        *angle = (float)v;
        volatile float u = (float)(v * *(const float *)0xf36ee4);
        const long double r = x87_round_h2(u);
        volatile float whole = (float)((long double)u >= r ? r : r - 1.0L);
        const long double f = (long double)*angle - (long double)whole * *(const float *)0xf268dc;
        *angle = (float)f;
        volatile float s = (float)x87_sin_h2(f);
        volatile float c = (float)x87_cos_h2(f);
        uint8_t *const m = self + 0x310 + 0x40 * j;
        *(volatile uint32_t *)m = *(volatile uint32_t *)&c;
        *(volatile float *)(m + 0x10) = (float)-(long double)s;
        *(volatile uint32_t *)(m + 4) = *(volatile uint32_t *)&s;
        *(volatile uint32_t *)(m + 0x14) = *(volatile uint32_t *)&c;
        const long double x = ((long double)k1 - (long double)c * k1) + (long double)s * k2;
        *(volatile float *)(m + 0x20) = (float)x;
        const long double y = ((long double)k2 - (long double)s * k1) - (long double)c * k2;
        *(volatile float *)(m + 0x24) = (float)y;
        *(volatile float *)(m + 0x30) = (float)x;
        *(volatile float *)(m + 0x34) = (float)y;
    }
}
static const uint8_t ride_blend_55fa00_original[] = {0x83, 0xec, 0x0c, 0x56, 0x57, 0x8d, 0x91, 0xdc, 0x03, 0x00, 0x00};
REWRITE("Ride::Blend55FA00", 0x55fa00, ride_blend_55fa00_original, &ride_blend_55fa00, &g_ride_spins);

// ---- Scenery::Points856900 (0x856900) and Water::Points97ACA0 (0x97ACA0): n vertices moved by a matrix ---------------
// Vertices of 0x24 bytes: position, normal, then two values. Each written one gets the position times the 4x4 matrix
// (rows 16 bytes apart; with its last row added) and the normal times its 3x3, each sum added in the game's order and
// rounded once, then value at +0x18 and the source's +0x1C and +0x20. The source is read again after each write (as
// the game does, so a vertex list moved in place gives what the game gives). Registers: n in eax, the matrix in ecx;
// Scenery's then takes (to, from, value), Water's has from in edx and takes (to, value).
static volatile long g_vertex_moves;
extern "C" uint32_t __stdcall forever_vertices_move(int32_t n, const float *m, const uint8_t *from, uint8_t *to, uint32_t value) {
    g_vertex_moves++;
    for (; n > 0; n--, from += 0x24, to += 0x24) {
        const volatile float *const s = (const volatile float *)from;
        volatile float *const d = (volatile float *)to;
        d[0] = (float)((((long double)s[2] * m[8] + (long double)m[4] * s[1]) + (long double)s[0] * m[0]) + m[12]);
        d[1] = (float)((((long double)m[5] * s[1] + (long double)m[1] * s[0]) + (long double)s[2] * m[9]) + m[13]);
        d[2] = (float)((((long double)s[2] * m[10] + (long double)m[6] * s[1]) + (long double)s[0] * m[2]) + m[14]);
        d[3] = (float)(((long double)s[5] * m[8] + (long double)s[4] * m[4]) + (long double)s[3] * m[0]);
        d[4] = (float)(((long double)m[5] * s[4] + (long double)m[1] * s[3]) + (long double)s[5] * m[9]);
        const long double nz = ((long double)s[5] * m[10] + (long double)m[6] * s[4]) + (long double)s[3] * m[2];
        *(volatile uint32_t *)(to + 0x18) = value;
        d[5] = (float)nz;
        *(volatile uint32_t *)(to + 0x1c) = *(const volatile uint32_t *)(from + 0x1c);
        *(volatile uint32_t *)(to + 0x20) = *(const volatile uint32_t *)(from + 0x20);
    }
    return 0;
}
extern "C" __attribute__((naked)) void forever_scenery_points_regs() {   // (to, from, value) on the stack
    __asm__("pushl 12(%esp)\n\tpushl 8(%esp)\n\tpushl 16(%esp)\n\tpushl %ecx\n\tpushl %eax\n\tcalll _forever_vertices_move@20\n\tret $12");
}
extern "C" __attribute__((naked)) void forever_water_points_regs() {   // from in edx; (to, value) on the stack
    __asm__("pushl 8(%esp)\n\tpushl 8(%esp)\n\tpushl %edx\n\tpushl %ecx\n\tpushl %eax\n\tcalll _forever_vertices_move@20\n\tret $8");
}
static const uint8_t scenery_points_original[] = {0x53, 0x8b, 0x5c, 0x24, 0x0c, 0x55, 0x8b, 0x6c, 0x24, 0x14};
static const uint8_t water_points_original[] = {0x53, 0x55, 0x8b, 0x6c, 0x24, 0x10, 0x56, 0x8b, 0x74, 0x24, 0x10};
REWRITE("Scenery::Points856900", 0x856900, scenery_points_original, (void *)&forever_scenery_points_regs, &g_vertex_moves);
REWRITE("Water::Points97ACA0", 0x97aca0, water_points_original, (void *)&forever_water_points_regs, &g_vertex_moves);

static inline int64_t x87_fistp64_h2(long double v) {   // to a whole number by the FPU's rounding (the "indefinite" value when out of range)
    int64_t r;
    __asm__("fistpll %0" : "=m"(r) : "t"(v) : "st");
    return r;
}

// ---- Animal::CheckAE09F0(this, t, out) (0xAE09F0): a point along a path of points, and the next point's flag -------
// With no points (+0x50) 0. f = n / length (+0x54) x t; t is kept as a float, f rounded to a whole i. From the last point
// on, out is the last point and 0 is given; otherwise out is the blend of points i and i + 1 ({x, y} at +0x40) by the
// part of t past its whole (floats as the game rounds them: the second point's y and both of the first's products),
// and the byte for point i + 1 (+0x48) is given.
static volatile long g_animal_checks;
static uint8_t __thiscall animal_check_ae09f0(const uint8_t *self, float t, float *out) {
    g_animal_checks++;
    const int32_t n = *(const int32_t *)(self + 0x50);
    if (!n) return 0;
    const long double f = (long double)n / *(const float *)(self + 0x54) * t;
    volatile float tf = (float)f;
    const int32_t i = (int32_t)x87_fistp64_h2(f);
    if (i >= n - 1) {
        const uint32_t *const p = (const uint32_t *)(*(const uint8_t *const *)(self + 0x40) + 8 * (n - 1));
        volatile uint32_t *const o = (volatile uint32_t *)out;
        o[0] = p[0], o[1] = p[1];
        return 0;
    }
    const long double r = x87_round_h2(tf);
    volatile float whole = (float)((long double)tf >= r ? r : r - 1.0L);
    const long double frac = (long double)tf - whole;
    const float *const p = (const float *)(*(const uint8_t *const *)(self + 0x40) + 8 * i);
    const long double a = frac * p[2];
    volatile float b = (float)(frac * p[3]);
    const long double g = (long double)*(const float *)0xf264dc - frac;
    volatile float c = (float)(g * p[0]);
    volatile float d = (float)(g * p[1]);
    volatile float *const o = out;
    o[0] = (float)((long double)c + a);
    o[1] = (float)((long double)b + d);
    return (*(const uint8_t *const *)(self + 0x48))[i + 1];
}
static const uint8_t animal_check_ae09f0_original[] = {0x8b, 0x41, 0x50, 0x83, 0xec, 0x18, 0x85, 0xc0};
REWRITE("Animal::CheckAE09F0", 0xae09f0, animal_check_ae09f0_original, &animal_check_ae09f0, &g_animal_checks);

// ---- Effects::Point498630(this, a, b, c, d, e) (0x498630): an effect's place and sizes, and two counts rounded up -----
// Without a part (+0x24) all five are 0 and 0 is given. Otherwise a = +0x40, b = the part's +4, c = twice its +8 (a
// float), d = a rounded up and e = (b + c) a (a float) rounded up, both as whole numbers; gives 1.
static volatile long g_effect_points;
static inline long double ceil_h2(float v) {   // rounded up as the game does: frndint, and one more when that was below
    const long double r = x87_round_h2(v);
    return (long double)v <= r || v != v ? r : r + 1.0L;
}
static uint8_t __thiscall effects_point_498630(const uint8_t *self, float *a, float *b, float *c, int32_t *d, int32_t *e) {
    g_effect_points++;
    const uint8_t *const part = *(const uint8_t *const *)(self + 0x24);
    if (!part) {
        *(volatile uint32_t *)a = 0, *(volatile uint32_t *)b = 0, *(volatile uint32_t *)c = 0, *(volatile int32_t *)d = 0, *(volatile int32_t *)e = 0;
        return 0;
    }
    *(volatile uint32_t *)a = *(const uint32_t *)(self + 0x40);
    *(volatile uint32_t *)b = *(const uint32_t *)(part + 4);
    *(volatile float *)c = (float)((long double)*(const float *)(part + 8) + *(const float *)(part + 8));
    volatile float up = (float)ceil_h2(*(volatile float *)a);
    *(volatile int32_t *)d = (int32_t)x87_fistp64_h2(up);
    volatile float w = (float)(((long double)*(volatile float *)b + *(volatile float *)c) * *(volatile float *)a);
    volatile float up2 = (float)ceil_h2(w);
    *(volatile int32_t *)e = (int32_t)x87_fistp64_h2(up2);
    return 1;
}
static const uint8_t effects_point_498630_original[] = {0x8b, 0x41, 0x24, 0x83, 0xec, 0x08, 0x85, 0xc0};
REWRITE("Effects::Point498630", 0x498630, effects_point_498630_original, &effects_point_498630, &g_effect_points);

static inline long double floor_h2(float v) {   // rounded down as the game does: frndint, and one less when that was above
    const long double r = x87_round_h2(v);
    return (long double)v >= r ? r : r - 1.0L;
}

// ---- Scenery::Round85D200(out, s, w, m) (0x85D200): a colour from a table by a scale, blended toward white -----------
// u = s K (0xF26610, a float); i = u rounded down (at most 5, then the part past it counts from 0xF28214 and the next
// row is row 0). The three tables (0xF780C0, 0xF780A8, 0xF78090) give rows i and i + 1, blended by the part f: f next +
// (1 - f) this (1 at 0xF264DC); out[3] = 1. Then each part p becomes ((1 - w) p + w) m.
static volatile long g_scenery_rounds;
static void __thiscall scenery_round_85d200(float *out, float s, float w, float m) {
    g_scenery_rounds++;
    volatile float u = (float)((long double)s * *(const float *)0xf26610);
    volatile float fl = (float)floor_h2(u);
    int32_t i = (int32_t)x87_fistp64_h2(fl), j;
    long double base;
    if (i >= 5) i = 5, j = 0, base = *(const float *)0xf28214;
    else j = i + 1, base = fl;
    const long double f = (long double)u - base;
    const long double g = (long double)*(const float *)0xf264dc - f;
    const float *const t0 = (const float *)0xf780c0, *const t1 = (const float *)0xf780a8, *const t2 = (const float *)0xf78090;
    volatile float *const o = out;
    o[0] = (float)(f * t0[j] + g * t0[i]);
    o[1] = (float)(f * t1[j] + g * t1[i]);
    *(volatile uint32_t *)(out + 3) = 0x3f800000;
    o[2] = (float)(f * t2[j] + g * t2[i]);
    const long double h = (long double)*(const float *)0xf264dc - w;
    o[0] = (float)((h * o[0] + w) * m);
    o[1] = (float)((h * o[1] + w) * m);
    o[2] = (float)((h * o[2] + w) * m);
}
static const uint8_t scenery_round_85d200_original[] = {0x83, 0xec, 0x0c, 0xd9, 0x44, 0x24, 0x10, 0xd8, 0x0d};
REWRITE("Scenery::Round85D200", 0x85d200, scenery_round_85d200_original, &scenery_round_85d200, &g_scenery_rounds);

// ---- Grid::Round4414F0(this) (0x4414F0): a slot number from an angle and a height -----------------------------------
// a = +0x48 less the reference (0x1106C54's +0x54, or the game's 0 without one); whole turns of 2880 taken off (by a K1,
// 0xF28D28, rounded down); s = that rest x K2 (0xF28CE8) rounded down, and 360 s taken off the rest (as a float); a
// band from (+0x48 - +0x54) K3 (0xF28CF8) rounded down, quartered (toward 0) and taken modulo 8 (signed), picks a whole
// number from the table 0x10EC748; the rest times it times K2, rounded down, plus 1 is given. Each rounding down is the
// game's (frndint and one less when above), of a float; the whole numbers by fistp.
static volatile long g_grid_rounds;
static int32_t __fastcall grid_round_4414f0(const uint8_t *self) {
    g_grid_rounds++;
    const uint8_t *const ref = *(const uint8_t *const volatile *)0x1106c54;
    const float base = ref ? *(const float *)(ref + 0x54) : *(const float *)0xf264e0;
    volatile float a = (float)((long double)*(const float *)(self + 0x48) - base);
    volatile float u1 = (float)((long double)a * *(const float *)0xf28d28);
    volatile float f1 = (float)floor_h2(u1);
    const int32_t i1 = (int32_t)x87_fistp64_h2(f1);
    const long double rest = (long double)a - (long double)(int32_t)((uint32_t)i1 * 0xb40u);
    a = (float)rest;
    volatile float u2 = (float)(rest * *(const float *)0xf28ce8);
    volatile float f2 = (float)floor_h2(u2);
    volatile float u3 = (float)(((long double)*(const float *)(self + 0x48) - *(const float *)(self + 0x54)) * *(const float *)0xf28cf8);
    volatile float f3 = (float)floor_h2(u3);
    const int32_t i2 = (int32_t)x87_fistp64_h2(f2);
    const long double c = (long double)a - (long double)(int32_t)((uint32_t)i2 * 0x168u);
    const int32_t i3 = (int32_t)x87_fistp64_h2(f3);
    const int32_t k = (i3 / 4) % 8;
    volatile float u4 = (float)(c * (long double)((const int32_t *)0x10ec748)[k] * *(const float *)0xf28ce8);
    volatile float f4 = (float)floor_h2(u4);
    return (int32_t)x87_fistp64_h2(f4) + 1;
}
static const uint8_t grid_round_4414f0_original[] = {0xa1, 0x54, 0x6c, 0x10, 0x01, 0xd9, 0x41, 0x48};
REWRITE("Grid::Round4414F0", 0x4414f0, grid_round_4414f0_original, &grid_round_4414f0, &g_grid_rounds);

// ---- Scenery::Part862DF0(this, out, n) (0x862DF0): where part n is ---------------------------------------------------
// Parts are 0x48 bytes at +0x104 (count +0x108): a pointer to the part (its point at +0x30) and a byte (+4) saying the
// point is already placed. Past the count: (0, the game's 0 (0xF264E0), 0). Placed: the point. Otherwise the point
// moved by this's matrix (rows +0x60, +0x70, +0x80, then +0x90 added), each part's sum in the game's order.
static volatile long g_scenery_parts;
static float *__thiscall scenery_part_862df0(const uint8_t *self, float *out, int32_t n) {
    g_scenery_parts++;
    volatile float *const o = out;
    if (n >= *(const int32_t *)(self + 0x108)) {
        o[0] = 0.0f;
        *(volatile uint32_t *)(out + 2) = 0;
        o[1] = *(const float *)0xf264e0;
        return out;
    }
    const uint8_t *const e = *(const uint8_t *const *)(self + 0x104) + 0x48 * n;
    const float *const p = (const float *)(*(const uint8_t *const *)e + 0x30);
    const float x = p[0], y = p[1];
    const uint32_t zbits = *(const uint32_t *)(p + 2);
    if (e[4]) {
        o[0] = x;
        *(volatile uint32_t *)(out + 2) = zbits;
        o[1] = y;
        return out;
    }
    const float z = p[2];
    const float *const m = (const float *)(self + 0x60);
    const long double tx = (((long double)z * m[8] + (long double)y * m[4]) + (long double)x * m[0]) + m[12];
    volatile float ty = (float)((((long double)z * m[9] + (long double)y * m[5]) + (long double)x * m[1]) + m[13]);
    volatile float tz = (float)((((long double)z * m[10] + (long double)y * m[6]) + (long double)x * m[2]) + m[14]);
    volatile float fx = (float)tx;
    o[0] = fx;
    *(volatile uint32_t *)(out + 2) = *(volatile uint32_t *)&tz;
    o[1] = ty;
    return out;
}
static const uint8_t scenery_part_862df0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x3c, 0x8b, 0x45, 0x0c};
REWRITE("Scenery::Part862DF0", 0x862df0, scenery_part_862df0_original, &scenery_part_862df0, &g_scenery_parts);

// ---- Camera::PassCD8060(this, out) (0xCD8060): the camera's target point (+8's +0x10, or 0s) moved by its matrix ------
// Rows at +0x60 (added), +0x70, +0x80, +0x90; the products and partial sums rounded to floats where the game rounds
// them: x = ((a r70 + (b r80)) + (c r90)) + r60 and y likewise, z = ((b r88 + (a r78)) + (c r98)) + r68 (the
// parenthesised parts floats, a r70, a r74 and b r88 not).
static volatile long g_camera_passes;
static float *__thiscall camera_pass_cd8060(const uint8_t *self, float *out) {
    g_camera_passes++;
    float a, b, c;
    const uint8_t *const part = *(const uint8_t *const *)(self + 8);
    if (part) {
        const float *const q = *(const float *const *)(part + 0x10);
        a = q[0], b = q[1], c = q[2];
    } else {
        a = b = c = *(const float *)0xf264e0;
    }
    const float *const m = (const float *)(self + 0x60);
    volatile float c0 = (float)((long double)c * m[12]), c1 = (float)((long double)c * m[13]), c2 = (float)((long double)c * m[14]);
    volatile float b0 = (float)((long double)b * m[8]), b1 = (float)((long double)b * m[9]);
    const long double b2 = (long double)b * m[10];
    const long double a0 = (long double)a * m[4], a1 = (long double)a * m[5];
    volatile float a2 = (float)((long double)a * m[6]);
    volatile float s0 = (float)(a0 + b0);
    volatile float s1 = (float)(a1 + b1);
    const long double zz = b2 + a2;
    volatile float t0 = (float)((long double)s0 + c0);
    volatile float t1 = (float)((long double)s1 + c1);
    const long double z = (zz + c2) + m[2];
    const long double y = (long double)t1 + m[1];
    const long double x = (long double)t0 + m[0];
    volatile float *const o = out;
    o[0] = (float)x, o[1] = (float)y, o[2] = (float)z;
    return out;
}
static const uint8_t camera_pass_cd8060_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x40, 0x8b, 0x41, 0x08};
REWRITE("Camera::PassCD8060", 0xcd8060, camera_pass_cd8060_original, &camera_pass_cd8060, &g_camera_passes);

// ---- Track::PointBB7030(this, out, length, p, q, t) (0xBB7030): a point added into a running average along a track ---
// The point is p, or p + (q - p) t when t is not exactly the game's 0 (0xF264E0) (x and z rounded to floats, y not).
// When *length is above 0 it grows by this's step (+0x30) and out moves toward the point by step / length (x and y
// differences rounded where the game rounds them); otherwise *length becomes the step and out the point.
static volatile long g_track_points;
static void __thiscall track_point_bb7030(const uint8_t *self, float *out, float *length, const float *p, const float *q, float t) {
    g_track_points++;
    const float k0 = *(const float *)0xf264e0;
    volatile float px = p[0], pz = p[2];
    long double py = p[1];
    if (!(t == k0)) {
        volatile float d0 = (float)((long double)q[0] - px);
        const long double d1 = (long double)q[1] - py;
        const long double d2 = (long double)q[2] - pz;
        volatile float e0 = (float)((long double)d0 * t);
        volatile float e1 = (float)(d1 * t);
        const long double e2 = d2 * t;
        px = (float)((long double)e0 + px);
        py = py + e1;
        pz = (float)(e2 + pz);
    }
    const float step = *(const float *)(self + 0x30);
    if (!(*length > k0)) {
        *(volatile uint32_t *)length = *(const uint32_t *)(self + 0x30);
        volatile float *const o = out;
        o[0] = px;
        *(volatile uint32_t *)(out + 2) = *(volatile uint32_t *)&pz;
        o[1] = (float)py;
        return;
    }
    volatile float l = (float)((long double)*length + step);
    *(volatile float *)length = l;
    volatile float *const o = out;
    volatile float g0 = (float)((long double)px - o[0]);
    const long double g1 = py - o[1];
    const long double g2 = (long double)pz - o[2];
    const long double r = (long double)step / l;
    volatile float h0 = (float)((long double)g0 * r);
    volatile float h1 = (float)(g1 * r);
    const long double h2 = g2 * r;
    o[0] = (float)((long double)h0 + o[0]);
    o[1] = (float)((long double)h1 + o[1]);
    o[2] = (float)(h2 + o[2]);
}
static const uint8_t track_point_bb7030_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x40, 0x8b, 0x45, 0x10};
REWRITE("Track::PointBB7030", 0xbb7030, track_point_bb7030_original, &track_point_bb7030, &g_track_points);

// ---- Ride::Set53CAD0(this, a, b, s, list) (0x53CAD0): a ride part set up from shared parts and scaled values ---------
// +4 = s K (K 0xF2704C, a float); +0x34 and +0x38 take *a and *b and count each up (their first value); +8 to +0x30
// take the eleven values of list times K. Gives this.
static volatile long g_ride_sets;
static uint8_t *__thiscall ride_set_53cad0(uint8_t *self, uint32_t **a, uint32_t **b, float s, const float *list) {
    g_ride_sets++;
    const float k = *(const float *)0xf2704c;
    volatile float *const f = (volatile float *)self;
    f[1] = (float)((long double)s * k);
    volatile uint32_t *const x = *a;
    *(volatile uint32_t *)(self + 0x34) = (uint32_t)(uintptr_t)x;
    x[0]++;
    volatile uint32_t *const y = *b;
    *(volatile uint32_t *)(self + 0x38) = (uint32_t)(uintptr_t)y;
    y[0]++;
    for (int i = 0; i < 11; i++) f[2 + i] = (float)((long double)list[i] * k);
    return self;
}
static const uint8_t ride_set_53cad0_original[] = {0xd9, 0x44, 0x24, 0x0c, 0x8b, 0x54, 0x24, 0x08};
REWRITE("Ride::Set53CAD0", 0x53cad0, ride_set_53cad0_original, &ride_set_53cad0, &g_ride_sets);

// ---- List::StepBCBE20(this, n, a, b, c, d) (0xBCBE20): the four control points of curve piece n ----------------------
// Entries of 0x24 bytes ({count, entries} at this) hold three points: P (+0), Q (+0xC) and R (+0x18). With m the next
// entry (entry 0 after the last): a = n's P, d = m's P, b = n's R + a, c = m's Q + d (each part one float sum).
static volatile long g_curve_steps;
static void __thiscall list_step_bcbe20(const uint32_t *self, int32_t n, float *a, float *b, float *c, float *d) {
    g_curve_steps++;
    const int32_t m = n < (int32_t)self[0] - 1 ? n + 1 : 0;
    auto entry = [&](int32_t i) { return (const float *)(uintptr_t)(self[1] + 0x24 * i); };
    volatile float *const va = a, *const vb = b, *const vc = c, *const vd = d;
    for (int k = 0; k < 3; k++) va[k] = entry(n)[k];
    for (int k = 0; k < 3; k++) vd[k] = entry(m)[k];
    const float *const r = entry(n) + 6;
    const float b0 = (float)((long double)r[0] + va[0]), b1 = (float)((long double)r[1] + va[1]), b2 = (float)((long double)r[2] + va[2]);
    vb[0] = b0, vb[1] = b1, vb[2] = b2;
    const float *const q = entry(m) + 3;
    const float c0 = (float)((long double)q[0] + vd[0]), c1 = (float)((long double)q[1] + vd[1]), c2 = (float)((long double)q[2] + vd[2]);
    vc[0] = c0, vc[1] = c1, vc[2] = c2;
}
static const uint8_t list_step_bcbe20_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x14, 0x8b, 0x11, 0x8b};
REWRITE("List::StepBCBE20", 0xbcbe20, list_step_bcbe20_original, &list_step_bcbe20, &g_curve_steps);

// The fields of a 0x70-byte entry that the game copies (as Entries::UpdateMatching): +0 to +0x1C, the byte +0x20, +0x24
// to +0x34, +0x40, +0x44, +0x48, +0x50, +0x54 and the byte +0x58.
static void entry_fields_copy(uint8_t *to, const uint8_t *from) {
    auto copy = [&](uint32_t at) { *(volatile uint32_t *)(to + at) = *(const volatile uint32_t *)(from + at); };
    for (uint32_t at = 0; at <= 0x1c; at += 4) copy(at);
    *(volatile uint8_t *)(to + 0x20) = *(const volatile uint8_t *)(from + 0x20);
    for (uint32_t at = 0x24; at <= 0x34; at += 4) copy(at);
    copy(0x40), copy(0x44), copy(0x48), copy(0x50), copy(0x54);
    *(volatile uint8_t *)(to + 0x58) = *(const volatile uint8_t *)(from + 0x58);
}

// ---- Entries::SortB78EA0(this, out, extra) (0xB78EA0): the first entry of a heap taken off -----------------------------
// Entries of 0x70 bytes at +0 (count +8). 0 when empty; otherwise out gets the first entry's fields, the count goes
// down, and when entries are left the last one's fields move to the front and Entries::SortB782A0 (0, count, extra)
// puts it in place. Gives 1.
static volatile long g_entry_pops;
static uint8_t __thiscall entries_sort_b78ea0(uint8_t *self, uint8_t *out, uint32_t extra) {
    g_entry_pops++;
    if (!*(const uint32_t *)(self + 8)) return 0;
    uint8_t *const first = *(uint8_t *const *)self;
    entry_fields_copy(out, first);
    const uint32_t n = *(const uint32_t *)(self + 8) - 1;
    *(volatile uint32_t *)(self + 8) = n;
    if (n) {
        entry_fields_copy(*(uint8_t *const *)self, *(uint8_t *const *)self + 0x70 * n);
        ((void(__thiscall *)(uint8_t *, uint32_t, uint32_t, uint32_t))0xb782a0)(self, 0, n, extra);
    }
    return 1;
}
static const uint8_t entries_sort_b78ea0_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x08, 0x85, 0xc0};
REWRITE("Entries::SortB78EA0", 0xb78ea0, entries_sort_b78ea0_original, &entries_sort_b78ea0, &g_entry_pops);

// ---- Memory::CopyCA6A50(this, buffer, size) (0xCA6A50): a block table set up in a buffer, or moved to a bigger one -------
// this: the table's head (+0, at the buffer's end), the buffer (+4), the table's count (+8, 16 bits; 8 bytes each). No
// buffer: all cleared, 1. A first buffer (at least 0x40 bytes) gets a two-entry table at its end: entry 0 in use over
// the whole buffer (its size less 0x14), entry 1 free, with their ring links in the buffer set (1, 1 and 0, 0); 1 (0
// when too small). Otherwise the old buffer's contents (up to the table's end) are copied over (the game's memcpy), the
// table is moved by the room gained (when any) and entry 0's size grows by it, its ring links moved with it; 1 (0 when
// the new buffer is smaller).
static volatile long g_table_moves;
static uint8_t __thiscall memory_copy_ca6a50(uint8_t *self, uint8_t *buffer, uint32_t size) {
    g_table_moves++;
    typedef void *(__cdecl *Copy)(void *, const void *, uint32_t);
    const Copy copy = (Copy)0xddf260;
    volatile uint32_t *const t = (volatile uint32_t *)self;
    volatile uint16_t *const count = (volatile uint16_t *)(self + 8);
    if (!buffer) {
        t[1] = 0, t[0] = 0, count[0] = 0, count[1] = 0;
        return 1;
    }
    auto link = [&](uint32_t place, uint32_t at) -> volatile uint16_t & { return *(volatile uint16_t *)(uintptr_t)((place & ~1u) + t[1] + at); };
    const uint32_t old = t[1];
    if (!old) {
        if (size < 0x40) return 0;
        volatile uint32_t *const h = (volatile uint32_t *)(buffer + size - 0x10);
        t[0] = (uint32_t)(uintptr_t)h, t[1] = (uint32_t)(uintptr_t)buffer, count[0] = 2;
        h[0] = size - 0x14;
        auto head = [&]() { return (volatile uint32_t *)(uintptr_t)t[0]; };
        head()[0] |= 1;
        ((volatile uint16_t *)head())[2] = 1;
        ((volatile uint16_t *)head())[3] = 1;
        link(head()[0], 2) = 1;
        link(head()[0], 0) = 1;
        head()[2] = 0;
        ((volatile uint16_t *)head())[6] = 0;
        ((volatile uint16_t *)head())[7] = 0;
        link(head()[2], 2) = 0;
        link(head()[2], 0) = 0;
        return 1;
    }
    const uint32_t entries = (uint32_t)count[0] * 8;
    const int32_t gained = (int32_t)(old - entries - t[0] + size);
    if (gained < 0) return 0;
    copy(buffer, (const void *)(uintptr_t)old, entries - old + t[0]);
    t[0] = t[0] + ((uint32_t)(uintptr_t)buffer - t[1]);
    t[1] = (uint32_t)(uintptr_t)buffer;
    if (gained) {
        uint8_t *const moved = (uint8_t *)(uintptr_t)t[0] + gained;
        copy(moved, (const void *)(uintptr_t)t[0], (uint32_t)count[0] * 8);
        t[0] = (uint32_t)(uintptr_t)moved;
        volatile uint32_t *const h = (volatile uint32_t *)moved;
        const uint32_t place = h[0];
        const uint16_t l0 = link(place, 0), l2 = link(place, 2);
        h[0] = place + (uint32_t)gained;
        link(((volatile uint32_t *)(uintptr_t)t[0])[0], 0) = l0;
        link(((volatile uint32_t *)(uintptr_t)t[0])[0], 2) = l2;
    }
    return 1;
}
static const uint8_t memory_copy_ca6a50_original[] = {0x56, 0x57, 0x8b, 0x7c, 0x24, 0x0c, 0x33, 0xd2};
REWRITE("Memory::CopyCA6A50", 0xca6a50, memory_copy_ca6a50_original, &memory_copy_ca6a50, &g_table_moves);

// ---- List::PassC25A10(this) (0xC25A10): each listed object's cached frame brought up to date ------------------------
// Objects at +0x10 (count +0xC; empty slots skipped), flags at +0x50. Flag 8: the direction (+0x70) and place (+0) are
// copied to +0x110 and +0x150, with 1 and 0 at +0x130/+0x134 and +0x170/+0x174, and flag 0x100 set. Else flag 0x100:
// a frame is made from the direction by Vector::NormalBEE460 (into a 4x4 whose last column starts at FLT_MAX), its
// three rows copied to +0x10, +0x20 and +0x30, the place to +0x40, and flag 0x100 cleared.
static volatile long g_frame_passes;
static void __fastcall list_pass_c25a10(const uint8_t *self) {
    g_frame_passes++;
    for (int32_t i = 0; i < *(const volatile int32_t *)(self + 0xc); i++) {
        if ((uint32_t)i >= *(const volatile uint32_t *)(self + 0xc)) continue;
        uint8_t *const o = (*(uint8_t *const *const *)(self + 0x10))[i];
        if (!o) continue;
        volatile uint32_t *const v = (volatile uint32_t *)o;
        const uint32_t flags = v[0x50 / 4];
        if (flags & 8) {
            v[0x110 / 4] = v[0x70 / 4], v[0x114 / 4] = v[0x74 / 4], v[0x118 / 4] = v[0x78 / 4];
            v[0x134 / 4] = 0, v[0x130 / 4] = 0x3f800000;
            v[0x150 / 4] = v[0], v[0x154 / 4] = v[1], v[0x158 / 4] = v[2];
            v[0x170 / 4] = 0x3f800000, v[0x174 / 4] = 0;
            v[0x50 / 4] = v[0x50 / 4] | 0x100;
        } else if (flags & 0x100) {
            alignas(16) uint32_t m[16];
            m[3] = m[7] = m[11] = 0x7f7fffff;
            ((void(__thiscall *)(uint32_t *, const void *))0xbee460)(m, o + 0x70);
            v[4] = m[0], v[6] = m[2], v[5] = m[1];
            v[8] = m[4], v[10] = m[6], v[9] = m[5];
            v[12] = m[8], v[14] = m[10], v[13] = m[9];
            v[0x10] = v[0], v[0x11] = v[1], v[0x12] = v[2];
            v[0x50 / 4] = v[0x50 / 4] & 0xfffffeffu;
        }
    }
}
static const uint8_t list_pass_c25a10_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x34, 0x53, 0x8b, 0xd9};
REWRITE("List::PassC25A10", 0xc25a10, list_pass_c25a10_original, &list_pass_c25a10, &g_frame_passes);

// ---- Loop::StepCDF4C0(i, n, list, points, direction) (0xCDF4C0): entry i of a heap of n moved down into place --------
// fastcall i and n. Entries are 32 bytes (six values used: +0 to +8 and +0x10 to +0x18), ordered by
// Vector::CompareCDF200 (a, b, points, direction): a child goes up when the comparison says 1. Each step takes the left
// child when it compares 1 against the right one (else the right), and moves it up while it compares 1 against the
// moving entry; the moving entry goes where it stops.
static volatile long g_triangle_sifts;
static uint32_t __fastcall loop_step_cdf4c0(int32_t i, int32_t n, uint8_t *list, uint32_t points, uint32_t direction) {
    g_triangle_sifts++;
    typedef int32_t(__fastcall *Compare)(const void *, const void *, uint32_t, uint32_t);
    const Compare compare = (Compare)0xcdf200;
    uint32_t v[8];
    auto fields = [](volatile uint32_t *to, const uint32_t *from) {
        to[0] = from[0], to[1] = from[1], to[2] = from[2], to[4] = from[4], to[5] = from[5], to[6] = from[6];
    };
    fields(v, (const uint32_t *)(list + 32 * i));
    auto at = [&](int32_t k) { return (uint32_t *)(list + 32 * k); };
    int32_t c = 2 * i + 1;
    for (; c + 1 < n; c = 2 * i + 1) {
        const int32_t pick = compare(at(c), at(c + 1), points, direction) == 1 ? c : c + 1;
        if (compare(at(pick), v, points, direction) != 1) {
            fields(at(i), v);
            return v[5];
        }
        fields(at(i), at(pick));
        i = pick;
    }
    if (c < n && compare(at(c), v, points, direction) == 1) {
        fields(at(i), at(c));
        i = c;
    }
    fields(at(i), v);
    return v[5];
}
static const uint8_t loop_step_cdf4c0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x34, 0x53, 0x8b, 0xd9};
REWRITE("Loop::StepCDF4C0", 0xcdf4c0, loop_step_cdf4c0_original, &loop_step_cdf4c0, &g_triangle_sifts);

// ---- Labels::Screen4368B0(this) (0x4368B0): two view records sized from the GUI root -------------------------------
// With the root (0x12F83F0): w and h its width and height (+0x84 - +0x7C, +0x88 - +0x80; floats). P = w K1, Q = h K2
// (K1 0xF2856C, K2 0xF28568) and R = P K3 (0xF28564) stay on the FPU; x = R (a float) and y = (1 - (w - R) / w) (Q / P) h.
// Both records (+0x1C, +0x20) get {x, y, w, h, 0, 1, 1, 0}.
static volatile long g_label_screens;
static void __fastcall labels_screen(const uint8_t *self) {
    g_label_screens++;
    const uint8_t *const root = *(const uint8_t *const volatile *)0x12f83f0;
    if (!root) return;
    auto f = [&](uint32_t at) { return *(const float *)(root + at); };
    volatile float w = (float)((long double)f(0x84) - f(0x7c));
    volatile float h = (float)((long double)f(0x88) - f(0x80));
    volatile uint32_t *const a = *(volatile uint32_t *const *)(self + 0x1c);
    a[2] = *(volatile uint32_t *)&w, a[3] = *(volatile uint32_t *)&h;
    const long double p = (long double)w * *(const float *)0xf2856c;
    const long double q = (long double)h * *(const float *)0xf28568;
    const long double r = p * *(const float *)0xf28564;
    volatile float x = (float)r;
    a[0] = *(volatile uint32_t *)&x;
    const long double s = (long double)*(const float *)0xf264dc - ((long double)w - r) / w;
    volatile float y = (float)(s * (q / p) * h);
    a[1] = *(volatile uint32_t *)&y;
    volatile uint32_t *const a2 = *(volatile uint32_t *const *)(self + 0x1c);
    a2[4] = 0, a2[5] = 0x3f800000, a2[6] = 0x3f800000, a2[7] = 0;
    volatile uint32_t *const b = *(volatile uint32_t *const *)(self + 0x20);
    b[0] = *(volatile uint32_t *)&x, b[1] = *(volatile uint32_t *)&y, b[2] = *(volatile uint32_t *)&w, b[3] = *(volatile uint32_t *)&h;
    b[4] = 0, b[5] = 0x3f800000, b[6] = 0x3f800000, b[7] = 0;
}
static const uint8_t labels_screen_original[] = {0xa1, 0xf0, 0x83, 0x2f, 0x01, 0x83, 0xec, 0x10};
REWRITE("Labels::Screen4368B0", 0x4368b0, labels_screen_original, &labels_screen, &g_label_screens);

#include <xmmintrin.h>

// ---- Vector::StepBB7800(out, a, b) (0xBB7800): out = a times b (4x4, rows 16 bytes; a's last column taken as 0,0,0,1) --
// fastcall out and a. With the SSE flag (0x13D07FC) each output row is ((a0 b0 + a1 b1) + a2 b2) in four lanes, plus b's
// last row for the last one, all rows worked out before any is written (rows aligned to 16). Without it, on the FPU,
// row by row (each written before the next is read): the same sums for three columns, unrounded until stored, with 0 in
// the fourth (1 in the last row's).
static volatile long g_matrix_products;
__attribute__((target("sse"))) static void matrix_product_sse(float *out, const float *a, const float *b) {
    const __m128 b0 = _mm_load_ps(b), b1 = _mm_load_ps(b + 4), b2 = _mm_load_ps(b + 8), b3 = _mm_load_ps(b + 12);
    __m128 r[4];
    for (int i = 0; i < 4; i++) {
        __m128 x = _mm_mul_ps(_mm_set1_ps(a[4 * i]), b0);
        x = _mm_add_ps(x, _mm_mul_ps(_mm_set1_ps(a[4 * i + 1]), b1));
        x = _mm_add_ps(x, _mm_mul_ps(_mm_set1_ps(a[4 * i + 2]), b2));
        r[i] = x;
    }
    r[3] = _mm_add_ps(r[3], b3);
    for (int i = 0; i < 4; i++) _mm_store_ps(out + 4 * i, r[i]);
}
static void __fastcall matrix_product_bb7800(float *out, const float *a, const float *b) {
    g_matrix_products++;
    if (*(const volatile uint8_t *)0x13d07fc) {
        matrix_product_sse(out, a, b);
        return;
    }
    const volatile float *const va = a, *const vb = b;
    volatile float *const o = out;
    for (int i = 0; i < 4; i++) {
        const float x = va[4 * i], y = va[4 * i + 1], z = va[4 * i + 2];
        long double s[3];
        for (int j = 0; j < 3; j++) s[j] = ((long double)x * vb[j] + (long double)y * vb[4 + j]) + (long double)z * vb[8 + j];
        if (i < 3) {
            o[4 * i] = (float)s[0], o[4 * i + 1] = (float)s[1], o[4 * i + 2] = (float)s[2];
            *(volatile uint32_t *)(out + 4 * i + 3) = 0;
        } else {
            const long double t0 = s[0] + vb[12], t1 = s[1] + vb[13], t2 = s[2] + vb[14];
            o[12] = (float)t0, o[13] = (float)t1, o[14] = (float)t2, o[15] = 1.0f;
        }
    }
}
static const uint8_t matrix_product_bb7800_original[] = {0x55, 0x8b, 0xec, 0x83, 0xec, 0x08, 0xa0, 0xfc, 0x07, 0x3d, 0x01};
REWRITE("Vector::StepBB7800", 0xbb7800, matrix_product_bb7800_original, &matrix_product_bb7800, &g_matrix_products);

// Three floats in place scaled to length 1 (by C1 / length, C1 0xF264DC; the length's sum ((x x + y y) + z z) on the
// FPU); when the length is exactly the game's 0 (0xF264E0) only z is set, to 1.
static void unit_in_place_h2(float *v) {
    volatile float *const p = v;
    const float x = p[0], y = p[1], z = p[2];
    const long double len = x87_sqrt_h2(((long double)x * x + (long double)y * y) + (long double)z * z);
    if (len == *(const float *)0xf264e0) {
        p[2] = 1.0f;
        return;
    }
    const long double inv = (long double)*(const float *)0xf264dc / len;
    p[0] = (float)(inv * p[0]);
    p[1] = (float)(inv * p[1]);
    p[2] = (float)(inv * p[2]);
}

// ---- Terrain::Length8BFB10(this) (0x8BFB10): a terrain piece's side direction and its scaled version --------------
// up (+0xA0) = (0, 1, 0); +0xB0 = up x the direction (+0x90), made unit length; +0xC0 = that over the piece's size
// (+0x14 for x, +0x18 for z; y 0), made unit length.
static volatile long g_terrain_sides;
static void __fastcall terrain_length_8bfb10(uint8_t *self) {
    g_terrain_sides++;
    volatile float *const f = (volatile float *)self;
    f[0xa4 / 4] = 1.0f, *(volatile uint32_t *)(self + 0xa0) = 0, *(volatile uint32_t *)(self + 0xa8) = 0;
    const long double ux = f[0xa0 / 4], uy = f[0xa4 / 4], uz = f[0xa8 / 4];
    const long double vx = f[0x90 / 4], vy = f[0x94 / 4], vz = f[0x98 / 4];
    const long double x = uy * vz - vy * uz;
    const long double y = vx * uz - ux * vz;
    volatile float z = (float)(vy * ux - uy * vx);
    *(volatile uint32_t *)(self + 0xb8) = *(volatile uint32_t *)&z;
    f[0xb0 / 4] = (float)x;
    f[0xb4 / 4] = (float)y;
    unit_in_place_h2((float *)(self + 0xb0));
    *(volatile uint32_t *)(self + 0xc4) = 0;
    f[0xc0 / 4] = (float)((long double)f[0xb0 / 4] / f[0x14 / 4]);
    f[0xc8 / 4] = (float)((long double)f[0xb8 / 4] / f[0x18 / 4]);
    unit_in_place_h2((float *)(self + 0xc0));
}
static const uint8_t terrain_length_8bfb10_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x10, 0xc7, 0x81, 0xa4};
REWRITE("Terrain::Length8BFB10", 0x8bfb10, terrain_length_8bfb10_original, &terrain_length_8bfb10, &g_terrain_sides);

static inline long double x87_tan_h2(long double v) {
    __asm__("fptan\n\tfstp %%st(0)" : "+t"(v));
    return v;
}

// ---- Camera::ScreenRay(this, x, y, origin, direction) (0xCD79C0): a ray through a screen point ------------------------
// t = tan(field of view (+0x190) K) (K 0xF2651C, fptan). The camera's right (+0x120), up (+0x130), forward (+0x140) and
// place (+0x160). Flat (+0x1A6 set): a = t (+0x198) and b = a / (+0x194) y; origin = place + a x right + b up, direction
// = forward. Otherwise b = t / (+0x194) y and c = t x: origin = place, direction = forward + c right + b up made unit
// length ((d.z d.z + d.y d.y) + d.x d.x; a zero length leaves (d.x, d.y, 1)). Roundings as the game's.
static volatile long g_screen_rays;
static void __thiscall camera_screen_ray(const uint8_t *self, float x, float y, float *origin, float *direction) {
    g_screen_rays++;
    auto m = [&](uint32_t at) { return *(const float *)(self + at); };
    const long double t = x87_tan_h2((long double)m(0x190) * *(const float *)0xf2651c);
    volatile float *const o = origin, *const d = direction;
    if (self[0x1a6]) {
        const long double a = t * m(0x198);
        const long double b = a / m(0x194) * y;
        volatile float u0 = (float)(b * m(0x130)), u1 = (float)(b * m(0x134)), u2 = (float)(b * m(0x138));
        const long double c = a * x;
        const long double r0 = c * m(0x120);
        volatile float r1 = (float)(c * m(0x124)), r2 = (float)(c * m(0x128));
        const long double q0 = r0 + m(0x160);
        volatile float q1 = (float)((long double)r1 + m(0x164)), q2 = (float)((long double)r2 + m(0x168));
        o[0] = (float)(q0 + u0);
        o[1] = (float)((long double)q1 + u1);
        o[2] = (float)((long double)q2 + u2);
        volatile uint32_t *const di = (volatile uint32_t *)direction;
        const uint32_t *const f = (const uint32_t *)(self + 0x140);
        di[0] = f[0], di[1] = f[1], di[2] = f[2];
        return;
    }
    {
        volatile uint32_t *const oi = (volatile uint32_t *)origin;
        const uint32_t *const p = (const uint32_t *)(self + 0x160);
        oi[0] = p[0], oi[1] = p[1], oi[2] = p[2];
    }
    const long double b = t / m(0x194) * y;
    volatile float u0 = (float)(b * m(0x130)), u1 = (float)(b * m(0x134)), u2 = (float)(b * m(0x138));
    const long double c = t * x;
    const long double r0 = c * m(0x120);
    volatile float r1 = (float)(c * m(0x124)), r2 = (float)(c * m(0x128));
    const long double q0 = r0 + m(0x140);
    volatile float q1 = (float)((long double)r1 + m(0x144)), q2 = (float)((long double)r2 + m(0x148));
    const long double d0 = q0 + u0, d1 = (long double)q1 + u1, d2 = (long double)q2 + u2;
    const long double len = x87_sqrt_h2((d2 * d2 + d1 * d1) + d0 * d0);
    if (len == *(const float *)0xf264e0) {
        d[0] = (float)d0, d[1] = (float)d1, d[2] = *(const float *)0xf264dc;
        return;
    }
    const long double inv = (long double)*(const float *)0xf264dc / len;
    d[0] = (float)(d0 * inv), d[1] = (float)(d1 * inv), d[2] = (float)(d2 * inv);
}
static const uint8_t camera_screen_ray_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x30, 0xd9, 0x81, 0x90};
REWRITE("Camera::ScreenRay", 0xcd79c0, camera_screen_ray_original, &camera_screen_ray, &g_screen_rays);

// ---- Vector::DistanceToLine(p, a, b) (0xCDB390): a point's distance from the segment a-b, on the FPU ----------------
// fastcall p and a. d = b - a (floats); L = |d| (its z square taken as the unrounded z times the float); n = d / L by
// the float 1 / L (or (0, 0, 1) when L is exactly the game's 0). Before a (n . (p - a) < 0, summed in the game's
// order): |a - p|. Past b (n . (b - p) < 0): |b - p|. Otherwise sqrt(h^2 - (h . d)^2 / L^2) with h = |p - a| rounded to a
// float. NaNs go on to the last case.
static volatile long g_line_distances;
static long double __fastcall vector_distance_to_line(const float *p, const float *a, const float *b) {
    g_line_distances++;
    const float a0 = a[0], a1 = a[1], a2 = a[2], b0 = b[0], b1 = b[1], b2 = b[2];
    volatile float d0 = (float)((long double)b0 - a0), d1 = (float)((long double)b1 - a1);
    const long double d2l = (long double)b2 - a2;
    volatile float d2 = (float)d2l;
    const long double len = x87_sqrt_h2((d2l * d2 + (long double)d1 * d1) + (long double)d0 * d0);
    volatile float lf = (float)len;
    long double n0, n1, n2;
    if (len == *(const float *)0xf264e0) {
        n0 = *(const float *)0xf264e0, n1 = *(const float *)0xf264e0, n2 = *(const float *)0xf264dc;
    } else {
        volatile float inv = (float)((long double)*(const float *)0xf264dc / len);
        n0 = (long double)inv * d0, n1 = (long double)d1 * inv, n2 = (long double)d2 * inv;
    }
    const float p0 = p[0], p1 = p[1], p2 = p[2];
    volatile float m1 = (float)-n1, m2 = (float)-n2;
    const long double s = ((((p2 * n2 + p1 * n1) + (long double)m2 * a2) + -n1 * a1) + n0 * p0) + -n0 * a0;
    if (s < 0) {
        const long double e0 = (long double)a0 - p0, e1 = (long double)a1 - p1, e2 = (long double)a2 - p2;
        return x87_sqrt_h2((e2 * e2 + e1 * e1) + e0 * e0);
    }
    const long double t = (((((long double)p2 * m2 + (long double)m1 * p1) + n2 * b2) + n1 * b1) + -n0 * p0) + n0 * b0;
    if (t < 0) {
        const long double g0 = (long double)b0 - p0, g1 = (long double)b1 - p1, g2 = (long double)b2 - p2;
        return x87_sqrt_h2((g2 * g2 + g1 * g1) + g0 * g0);
    }
    const long double h0 = (long double)p0 - a0, h1 = (long double)p1 - a1, h2 = (long double)p2 - a2;
    volatile float hf = (float)x87_sqrt_h2((h2 * h2 + h1 * h1) + h0 * h0);
    const long double dot = (h2 * d2 + h1 * d1) + h0 * d0;
    const long double q = (long double)hf * hf;
    return x87_sqrt_h2(q - dot * dot / ((long double)lf * lf));
}
static const uint8_t vector_distance_to_line_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x60, 0x8b, 0x02, 0x89};
REWRITE("Vector::DistanceToLine", 0xcdb390, vector_distance_to_line_original, &vector_distance_to_line, &g_line_distances);

// ---- Box::Transform(lo, hi, m, out lo, out hi) (0x85D2F0): the box around a box moved by a matrix ---------------------
// fastcall lo and hi. Both outputs start at m's translation (+0x30); nothing more when hi.y is below lo.y. Then for each
// output part j and input part k: a = lo[k] m[k][j] (kept on the FPU) and b = hi[k] m[k][j] (a float); the smaller goes
// on the low corner and the larger on the high one (a NaN or a tie: b low, a high), each sum rounded to a float.
static volatile long g_box_transforms;
static void __fastcall box_transform(const float *lo, const float *hi, const float *m, float *out_lo, float *out_hi) {
    g_box_transforms++;
    volatile float *const ol = out_lo, *const oh = out_hi;
    for (int j = 0; j < 3; j++) {
        *(volatile uint32_t *)(out_lo + j) = *(const uint32_t *)(m + 12 + j);
        *(volatile uint32_t *)(out_hi + j) = *(const uint32_t *)(m + 12 + j);
    }
    if (hi[1] < lo[1]) return;
    for (int j = 0; j < 3; j++)
        for (int k = 0; k < 3; k++) {
            const float e = m[4 * k + j];
            const long double a = (long double)lo[k] * e;
            volatile float b = (float)((long double)hi[k] * e);
            if (a < b) {
                ol[j] = (float)(a + ol[j]);
                oh[j] = (float)((long double)b + oh[j]);
            } else {
                ol[j] = (float)((long double)b + ol[j]);
                oh[j] = (float)(a + oh[j]);
            }
        }
}
static const uint8_t box_transform_original[] = {0x53, 0x8b, 0x5c, 0x24, 0x08, 0x8b, 0x43, 0x30};
REWRITE("Box::Transform", 0x85d2f0, box_transform_original, &box_transform, &g_box_transforms);

// ---- Mesh::ComputeNormals(this) (0x95FAA0): a mesh's vertex normals from its triangles -------------------------------
// this: vertex count +0, positions +4 and normals +8 (16 bytes each), triangles +0xC (three numbers) and their count
// +0x10. The normals are cleared; each triangle's normal ((p1 - p0) x (p2 - p0), heights scaled by K 0xF355E4, the
// differences floats) made unit length (its x and y kept on the FPU, z rounded; a zero length gives (x, y, 1)) is added
// to its three corners; then each normal is made unit length (unit_in_place_h2).
static volatile long g_mesh_normals;
static void __fastcall mesh_compute_normals(const uint8_t *self) {
    g_mesh_normals++;
    const int32_t n = *(const int32_t *)self;
    for (int32_t i = 0; i < n && i < *(const volatile int32_t *)self; i++) {
        volatile uint32_t *const v = (volatile uint32_t *)(*(uint8_t *const *)(self + 8) + 16 * i);
        v[0] = 0, v[1] = 0, v[2] = 0;
    }
    const float k = *(const float *)0xf355e4;
    for (int32_t t = 0; t < *(const volatile int32_t *)(self + 0x10); t++) {
        const uint32_t *const tri = (const uint32_t *)(*(const uint8_t *const *)(self + 0xc) + 12 * t);
        const uint32_t i0 = tri[0], i1 = tri[1], i2 = tri[2];
        const float *const pos = *(const float *const *)(self + 4);
        const float *const p0 = pos + 4 * i0, *const p1 = pos + 4 * i1, *const p2 = pos + 4 * i2;
        volatile float z0 = (float)((long double)p0[2] * k), z1 = (float)((long double)p1[2] * k), z2 = (float)((long double)p2[2] * k);
        volatile float v0 = (float)((long double)p2[0] - p0[0]), v1 = (float)((long double)p2[1] - p0[1]), v2 = (float)((long double)z2 - z0);
        volatile float u0 = (float)((long double)p1[0] - p0[0]), u1 = (float)((long double)p1[1] - p0[1]), u2 = (float)((long double)z1 - z0);
        long double nx = (long double)u1 * v2 - (long double)u2 * v1;
        long double ny = (long double)u2 * v0 - (long double)v2 * u0;
        const long double nz = (long double)v1 * u0 - (long double)u1 * v0;
        const long double len = x87_sqrt_h2((nz * nz + ny * ny) + nx * nx);
        volatile float z;
        if (len == *(const float *)0xf264e0) {
            z = 1.0f;
        } else {
            const long double inv = (long double)*(const float *)0xf264dc / len;
            nx = nx * inv, ny = ny * inv;
            z = (float)(nz * inv);
        }
        const uint32_t corner[3] = {i0, i1, i2};
        for (int c = 0; c < 3; c++) {
            volatile float *const m = (volatile float *)(*(uint8_t *const *)(self + 8) + 16 * corner[c]);
            m[0] = (float)(nx + m[0]);
            m[1] = (float)(ny + m[1]);
            m[2] = (float)((long double)z + m[2]);
        }
    }
    for (int32_t i = 0; i < *(const volatile int32_t *)self; i++) unit_in_place_h2((float *)(*(uint8_t *const *)(self + 8) + 16 * i));
}
static const uint8_t mesh_compute_normals_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x64, 0x8b, 0x01, 0x53};
REWRITE("Mesh::ComputeNormals", 0x95faa0, mesh_compute_normals_original, &mesh_compute_normals, &g_mesh_normals);

// ---- Vector::LengthBED120 / BED2B0(this, v) (0xBED120, 0xBED2B0): a 3x3 frame (rows 16 bytes) around a given axis ----
// Row A takes v made unit length (unit_in_place_h2). Its main part M is compared (by size) with part F, then part S:
// when bigger than both, k is K1, else K2. Row B = e_k - A_k A (each part a float: 1 - A_k^2 or -(A_k A_j)), made unit
// length; row C = B x A (each part one float). LengthBED120: A row 1, B row 2, C row 0, M = y, F = z, S = x, K1 = z,
// K2 = y. LengthBED2B0: A row 2, B row 0, C row 1, M = z, F = x, S = y, K1 = x, K2 = z.
static volatile long g_frames_about;
template <int AR, int BR, int CR, int M, int F, int S, int K1, int K2> static void __thiscall frame_about(uint8_t *self, const uint32_t *v) {
    g_frames_about++;
    float *const a = (float *)(self + 16 * AR), *const b = (float *)(self + 16 * BR), *const c = (float *)(self + 16 * CR);
    volatile uint32_t *const ai = (volatile uint32_t *)a;
    ai[0] = v[0], ai[1] = v[1], ai[2] = v[2];
    unit_in_place_h2(a);
    const volatile float *const va = a;
    const float am = va[M] < 0 ? -va[M] : va[M], af = va[F] < 0 ? -va[F] : va[F], as = va[S] < 0 ? -va[S] : va[S];
    const int k = (am > af && am > as) ? K1 : K2;
    volatile float *const vb = b;
    const long double ak = va[k];
    for (int j = 0; j < 3; j++) vb[j] = j == k ? (float)((long double)*(const float *)0xf264dc - ak * ak) : (float)-(ak * va[j]);
    unit_in_place_h2(b);
    const long double ax = va[0], ay = va[1], az = va[2], bx = vb[0], by = vb[1], bz = vb[2];
    volatile float cz = (float)(bx * ay - by * ax);
    volatile float *const vc = c;
    *(volatile uint32_t *)(c + 2) = *(volatile uint32_t *)&cz;
    vc[0] = (float)(by * az - bz * ay);
    vc[1] = (float)(bz * ax - bx * az);
}
static const uint8_t frame_about_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x10, 0x8b, 0x45, 0x08};
REWRITE("Vector::LengthBED120", 0xbed120, frame_about_original, (&frame_about<1, 2, 0, 1, 2, 0, 2, 1>), &g_frames_about);
REWRITE("Vector::LengthBED2B0", 0xbed2b0, frame_about_original, (&frame_about<2, 0, 1, 2, 0, 1, 0, 2>), &g_frames_about);

// ---- RideCarSpinningPart::TrackMotion(this, dt) (0x52BAE0): a part's velocity and acceleration from its position ----
// Nothing unless dt is above the game's 0 (a NaN goes on). i = 1 / dt (a float). The position is the part's (+0xF4)
// +0x90. First call (+0x1C8 = 0): it is kept at +0x1D0. Second: the velocity (position - kept) i goes to +0x1E0 (x kept
// on the FPU until stored, y and z differences floats). Later: also the acceleration (velocity - kept velocity) i to
// +0x1F0, x on the FPU, y and z differences floats. The state counts up to 3.
static volatile long g_spin_motions;
static void __thiscall spinning_part_track_motion(uint8_t *self, float dt) {
    g_spin_motions++;
    if (dt <= *(const float *)0xf264e0) return;
    volatile float inv = (float)((long double)*(const float *)0xf264dc / dt);
    const uint8_t *const part = *(const uint8_t *const *)(self + 0xf4);
    volatile float px = *(const float *)(part + 0x90);
    const uint32_t py = *(const uint32_t *)(part + 0x94), pz = *(const uint32_t *)(part + 0x98);
    volatile uint32_t *const t = (volatile uint32_t *)self;
    volatile float *const f = (volatile float *)self;
    if (!t[0x1c8 / 4]) {
        f[0x1d0 / 4] = px;
        t[0x1d4 / 4] = py, t[0x1d8 / 4] = pz;
        t[0x1c8 / 4] = 1;
        return;
    }
    auto as_float = [](uint32_t bits) {
        float v;
        __builtin_memcpy(&v, &bits, 4);
        return v;
    };
    const long double dx = (long double)px - f[0x1d0 / 4];
    volatile float dy = (float)((long double)as_float(py) - f[0x1d4 / 4]);
    volatile float dz = (float)((long double)as_float(pz) - f[0x1d8 / 4]);
    t[0x1d0 / 4] = *(volatile uint32_t *)&px, t[0x1d4 / 4] = py, t[0x1d8 / 4] = pz;
    const uint32_t state = t[0x1c8 / 4];
    const long double vx = dx * inv;
    volatile float vy = (float)((long double)dy * inv), vz = (float)((long double)dz * inv);
    if (state == 1) {
        f[0x1e0 / 4] = (float)vx;
        t[0x1e4 / 4] = *(volatile uint32_t *)&vy, t[0x1e8 / 4] = *(volatile uint32_t *)&vz;
        t[0x1c8 / 4] = 2;
        return;
    }
    const long double ax = vx - f[0x1e0 / 4];
    volatile float ay = (float)((long double)vy - f[0x1e4 / 4]), az = (float)((long double)vz - f[0x1e8 / 4]);
    const long double ax2 = ax * inv, ay2 = (long double)ay * inv;
    volatile float az2 = (float)((long double)az * inv);
    t[0x1f8 / 4] = *(volatile uint32_t *)&az2;
    f[0x1f0 / 4] = (float)ax2;
    f[0x1f4 / 4] = (float)ay2;
    t[0x1e4 / 4] = *(volatile uint32_t *)&vy, t[0x1e8 / 4] = *(volatile uint32_t *)&vz;
    f[0x1e0 / 4] = (float)vx;
    t[0x1c8 / 4] = 3;
}
static const uint8_t spinning_part_track_motion_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x40, 0xd9, 0x45, 0x08};
REWRITE("RideCarSpinningPart::TrackMotion", 0x52bae0, spinning_part_track_motion_original, &spinning_part_track_motion, &g_spin_motions);

// ---- Plane::BelowCDB0E0(v0, e1, e2, origin, direction, t out) (0xCDB0E0): whether a ray meets a triangle ----------------
// fastcall v0 and e1. w = v0 - origin (y and z floats); n = e1 x e2; t = (w . n) / (n . direction), written out (a
// float); none when t is below the game's 0. The point's offset r = w - t direction (t dy, t dz floats); with a1 = r . e1
// (a float), a2 = r . e2 and the Gram terms e11, e12 and e22 (a float), u = (e12 a2 - e22 a1) / det and v = (e12 a1 -
// e11 a2) / det, det = e22 e11 - e12^2: it meets when u and v are above the game's epsilon (0xF6A450) and u + v is below
// 0xF6A44C. Sums in the game's orders.
static volatile long g_ray_triangles;
extern "C" uint8_t __stdcall forever_ray_triangle(const float *v0, const float *e1, const float *e2, const float *origin, const float *d, float *t_out) {
    g_ray_triangles++;
    const long double w0 = (long double)v0[0] - origin[0];
    volatile float w1 = (float)((long double)v0[1] - origin[1]), w2 = (float)((long double)v0[2] - origin[2]);
    const long double c0 = (long double)e1[1] * e2[2] - (long double)e2[1] * e1[2];
    const long double c1 = (long double)e1[2] * e2[0] - (long double)e1[0] * e2[2];
    const long double c2 = (long double)e2[1] * e1[0] - (long double)e1[1] * e2[0];
    const long double num = ((long double)w2 * c2 + c1 * w1) + c0 * w0;
    const long double den = (c2 * d[2] + c1 * d[1]) + c0 * d[0];
    const long double t = num / den;
    *(volatile float *)t_out = (float)t;
    if (t < *(const float *)0xf264e0 || t != t) return 0;
    const long double q0 = t * d[0];
    volatile float q1 = (float)(t * d[1]), q2 = (float)(t * d[2]);
    const long double r0 = w0 - q0, r1 = (long double)w1 - q1, r2 = (long double)w2 - q2;
    volatile float a1 = (float)((r1 * e1[1] + r2 * e1[2]) + r0 * e1[0]);
    const long double a2 = (r1 * e2[1] + r0 * e2[0]) + r2 * e2[2];
    const long double e11 = ((long double)e1[1] * e1[1] + (long double)e1[2] * e1[2]) + (long double)e1[0] * e1[0];
    const long double e12 = ((long double)e1[0] * e2[0] + (long double)e1[2] * e2[2]) + (long double)e1[1] * e2[1];
    volatile float e22 = (float)(((long double)e2[1] * e2[1] + (long double)e2[2] * e2[2]) + (long double)e2[0] * e2[0]);
    const long double det = (long double)e22 * e11 - e12 * e12;
    const long double inv = (long double)*(const float *)0xf264dc / det;
    volatile float u = (float)((e12 * a2 - (long double)e22 * a1) * inv);
    volatile float v = (float)((e12 * a1 - e11 * a2) * inv);
    const float eps = *(const float *)0xf6a450;
    if (!(u > eps)) return 0;
    if (!(v > eps)) return 0;
    return (long double)v + u < *(const float *)0xf6a44c;
}
static uint8_t __fastcall plane_below_cdb0e0(const float *v0, const float *e1, const float *e2, const float *origin, const float *d, float *t_out) {
    return forever_ray_triangle(v0, e1, e2, origin, d, t_out);
}
// Water::Below93D140 (0x93D140) is the same test with v0 in edi, the origin in eax, e2 in ecx and the direction in esi;
// e1 and t out on the stack
extern "C" __attribute__((naked)) void forever_water_below_regs() {
    __asm__("pushl 8(%esp)\n\tpushl %esi\n\tpushl %eax\n\tpushl %ecx\n\tpushl 20(%esp)\n\tpushl %edi\n\tcalll _forever_ray_triangle@24\n\tret $8");
}
static const uint8_t water_below_93d140_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x30, 0xd9, 0x07, 0x8b};
REWRITE("Water::Below93D140", 0x93d140, water_below_93d140_original, (void *)&forever_water_below_regs, &g_ray_triangles);
static const uint8_t plane_below_cdb0e0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x2c, 0x8b, 0x45, 0x0c};
REWRITE("Plane::BelowCDB0E0", 0xcdb0e0, plane_below_cdb0e0_original, &plane_below_cdb0e0, &g_ray_triangles);

// ---- Terrain::Below8BBB60(v0, e1, e2, origin, direction, n out, t out) (0x8BBB60): the ray-triangle test, terrain's copy --
// As forever_ray_triangle, with these differences: the normal e1 x e2 is written out (floats); a NaN t goes on (only a t
// below 0 fails); a1 = (r0 e1x + r1 e1y) + r2 e1z; e12 = (e1z e2z + e1y e2y) + e1x e2x; the result is the whole eax.
static uint32_t __fastcall terrain_below_8bbb60(const float *v0, const float *e1, const float *e2, const float *origin, const float *d, float *n_out, float *t_out) {
    g_ray_triangles++;
    const long double w0 = (long double)v0[0] - origin[0];
    volatile float w1 = (float)((long double)v0[1] - origin[1]), w2 = (float)((long double)v0[2] - origin[2]);
    const long double c0 = (long double)e1[1] * e2[2] - (long double)e2[1] * e1[2];
    const long double c1 = (long double)e2[0] * e1[2] - (long double)e1[0] * e2[2];
    const long double c2 = (long double)e2[1] * e1[0] - (long double)e1[1] * e2[0];
    volatile float *const n = n_out;
    n[0] = (float)c0, n[1] = (float)c1, n[2] = (float)c2;
    const long double num = ((long double)w2 * c2 + c1 * w1) + c0 * w0;
    const long double den = (c2 * d[2] + c1 * d[1]) + c0 * d[0];
    const long double t = num / den;
    *(volatile float *)t_out = (float)t;
    if (t < *(const float *)0xf264e0) return 0;
    const long double q0 = t * d[0];
    volatile float q1 = (float)(t * d[1]), q2 = (float)(t * d[2]);
    const long double r0 = w0 - q0, r1 = (long double)w1 - q1, r2 = (long double)w2 - q2;
    volatile float a1 = (float)((r0 * e1[0] + r1 * e1[1]) + r2 * e1[2]);
    const long double a2 = (r1 * e2[1] + r0 * e2[0]) + r2 * e2[2];
    const long double e11 = ((long double)e1[1] * e1[1] + (long double)e1[2] * e1[2]) + (long double)e1[0] * e1[0];
    const long double e12 = ((long double)e1[2] * e2[2] + (long double)e1[1] * e2[1]) + (long double)e1[0] * e2[0];
    volatile float e22 = (float)(((long double)e2[1] * e2[1] + (long double)e2[2] * e2[2]) + (long double)e2[0] * e2[0]);
    const long double det = (long double)e22 * e11 - e12 * e12;
    const long double inv = (long double)*(const float *)0xf264dc / det;
    volatile float u = (float)((e12 * a2 - (long double)e22 * a1) * inv);
    volatile float v = (float)((e12 * a1 - e11 * a2) * inv);
    const float eps = *(const float *)0xf6a450;
    if (!(u > eps)) return 0;
    if (!(v > eps)) return 0;
    return (long double)v + u < *(const float *)0xf6a44c;
}
static const uint8_t terrain_below_8bbb60_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x2c, 0xd9, 0x01, 0x8b};
REWRITE("Terrain::Below8BBB60", 0x8bbb60, terrain_below_8bbb60_original, &terrain_below_8bbb60, &g_ray_triangles);

// ---- Terrain::Below8BBD40(v0, e1, e2, origin, direction, n, t out) (0x8BBD40): the ray-triangle test with a given normal --
// As Terrain::Below8BBB60 but the normal n is given: t = ((w2 n2 + w1 n1) + w0 n0) / ((d0 n0 + n2 d2) + n1 d1);
// a1 = (r0 e1x + r2 e1z) + r1 e1y; a2 = (r0 e2x + r2 e2z) + r1 e2y; e11 and e22 = (x x + y y) + z z; e12 = (e2z e1z +
// e2y e1y) + e1x e2x.
static uint32_t __fastcall terrain_below_8bbd40(const float *v0, const float *e1, const float *e2, const float *origin, const float *d, const float *n, float *t_out) {
    g_ray_triangles++;
    const long double w0 = (long double)v0[0] - origin[0];
    volatile float w1 = (float)((long double)v0[1] - origin[1]), w2 = (float)((long double)v0[2] - origin[2]);
    const long double num = ((long double)w2 * n[2] + (long double)w1 * n[1]) + w0 * n[0];
    const long double den = ((long double)d[0] * n[0] + (long double)n[2] * d[2]) + (long double)n[1] * d[1];
    const long double t = num / den;
    *(volatile float *)t_out = (float)t;
    if (t < *(const float *)0xf264e0) return 0;
    const long double q0 = t * d[0];
    volatile float q1 = (float)(t * d[1]), q2 = (float)(t * d[2]);
    const long double r0 = w0 - q0, r1 = (long double)w1 - q1, r2 = (long double)w2 - q2;
    volatile float a1 = (float)((r0 * e1[0] + r2 * e1[2]) + r1 * e1[1]);
    const long double a2 = (r0 * e2[0] + r2 * e2[2]) + r1 * e2[1];
    const long double e11 = ((long double)e1[0] * e1[0] + (long double)e1[1] * e1[1]) + (long double)e1[2] * e1[2];
    const long double e12 = ((long double)e2[2] * e1[2] + (long double)e2[1] * e1[1]) + (long double)e1[0] * e2[0];
    volatile float e22 = (float)(((long double)e2[0] * e2[0] + (long double)e2[1] * e2[1]) + (long double)e2[2] * e2[2]);
    const long double det = (long double)e22 * e11 - e12 * e12;
    const long double inv = (long double)*(const float *)0xf264dc / det;
    volatile float u = (float)((e12 * a2 - (long double)e22 * a1) * inv);
    volatile float v = (float)((e12 * a1 - e11 * a2) * inv);
    const float eps = *(const float *)0xf6a450;
    if (!(u > eps)) return 0;
    if (!(v > eps)) return 0;
    return (long double)v + u < *(const float *)0xf6a44c;
}
static const uint8_t terrain_below_8bbd40_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x30, 0x8b, 0x45, 0x0c};
REWRITE("Terrain::Below8BBD40", 0x8bbd40, terrain_below_8bbd40_original, &terrain_below_8bbd40, &g_ray_triangles);

// ---- Path::Below71C470(v0, e1, e2, origin, direction, nearest) (0x71C470): the ray-triangle test, keeping the nearest t --
// As forever_ray_triangle but w is all floats; t (rounded to a float) must not be below 0 (unrounded; a NaN fails) and
// must be below *nearest; the point's offset uses the float t (r0 a float, r2 from a float t dz); a1 = (r1 e1y + r0 e1x)
// + r2 e1z, a2 = (r1 e2y + r0 e2x) + r2 e2z. On a hit *nearest becomes t; gives 1 (low byte), else 0.
static uint8_t __fastcall path_below_71c470(const float *v0, const float *e1, const float *e2, const float *origin, const float *d, float *nearest) {
    g_ray_triangles++;
    volatile float w0 = (float)((long double)v0[0] - origin[0]), w1 = (float)((long double)v0[1] - origin[1]), w2 = (float)((long double)v0[2] - origin[2]);
    const long double c0 = (long double)e1[1] * e2[2] - (long double)e2[1] * e1[2];
    const long double c1 = (long double)e1[2] * e2[0] - (long double)e1[0] * e2[2];
    const long double c2 = (long double)e2[1] * e1[0] - (long double)e1[1] * e2[0];
    const long double num = ((long double)w2 * c2 + c1 * w1) + c0 * w0;
    const long double den = (c2 * d[2] + c1 * d[1]) + c0 * d[0];
    const long double t = num / den;
    volatile float tf = (float)t;
    if (t < *(const float *)0xf264e0 || t != t) return 0;
    if (!(tf < *nearest)) return 0;
    const long double q0 = (long double)tf * d[0], q1 = (long double)tf * d[1];
    volatile float q2 = (float)((long double)tf * d[2]);
    volatile float r0 = (float)((long double)w0 - q0);
    const long double r1 = (long double)w1 - q1;
    const long double r2 = (long double)w2 - q2;
    volatile float a1 = (float)((r1 * e1[1] + (long double)r0 * e1[0]) + r2 * e1[2]);
    const long double a2 = (r1 * e2[1] + (long double)r0 * e2[0]) + r2 * e2[2];
    const long double e11 = ((long double)e1[1] * e1[1] + (long double)e1[2] * e1[2]) + (long double)e1[0] * e1[0];
    const long double e12 = ((long double)e1[0] * e2[0] + (long double)e1[2] * e2[2]) + (long double)e1[1] * e2[1];
    volatile float e22 = (float)(((long double)e2[1] * e2[1] + (long double)e2[2] * e2[2]) + (long double)e2[0] * e2[0]);
    const long double det = (long double)e22 * e11 - e12 * e12;
    const long double inv = (long double)*(const float *)0xf264dc / det;
    volatile float u = (float)((e12 * a2 - (long double)e22 * a1) * inv);
    volatile float v = (float)((e12 * a1 - e11 * a2) * inv);
    const float eps = *(const float *)0xf6a450;
    if (!(u > eps)) return 0;
    if (!(v > eps)) return 0;
    if (!((long double)v + u < *(const float *)0xf6a44c)) return 0;
    *(volatile uint32_t *)nearest = *(volatile uint32_t *)&tf;
    return 1;
}
static const uint8_t path_below_71c470_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x38, 0x8b, 0x45, 0x0c};
REWRITE("Path::Below71C470", 0x71c470, path_below_71c470_original, &path_below_71c470, &g_ray_triangles);

// ---- Water::Length94CA80(n in ecx, v in edx, out in esi, ratio) (0x94CA80): a direction bent through a water surface ---
// S = v . n ((v1 n1 + n2 v2) + (v0 n0 as a float)); t = v - S n (S n as floats; t1 a float) made unit length (a zero
// length leaves t0 and t1 and sets t2 to 1; t0 stays on the FPU). s = (t . v) / ratio (a float; the test of it unrounded). Below 1: c = sqrt(1 -
// s^2), negated (times 0xF268F0) when S is below 0, and out = c n + s t; otherwise out = n times the game's 0 plus t. Then
// out is made unit length (a zero length sets z to 1). Floats where the game rounds; gives out.
static volatile long g_water_bends;
extern "C" float *__stdcall forever_water_bend(const float *n, const float *v, float *out, float ratio) {
    g_water_bends++;
    const float one = *(const float *)0xf264dc, zero = *(const float *)0xf264e0;
    volatile float d1 = (float)((long double)v[0] * n[0]);
    const long double s0 = ((long double)v[1] * n[1] + (long double)n[2] * v[2]) + d1;
    volatile float p0 = (float)(s0 * n[0]), p1 = (float)(s0 * n[1]), p2 = (float)(s0 * n[2]);
    long double t0 = (long double)v[0] - p0;
    volatile float t1 = (float)((long double)v[1] - p1);
    const long double t2l = (long double)v[2] - p2;
    volatile float t2;
    const long double len = x87_sqrt_h2((t2l * t2l + (long double)t1 * t1) + t0 * t0);
    if (len == zero) {
        t2 = 1.0f;
    } else {
        const long double inv = (long double)one / len;
        t0 = t0 * inv;
        t1 = (float)((long double)t1 * inv);
        t2 = (float)(t2l * inv);
    }
    const long double sl = ((t0 * v[0] + (long double)t2 * v[2]) + (long double)t1 * v[1]) / ratio;
    volatile float s = (float)sl;
    long double r0;
    volatile float q0, q1, q2, r1, r2;
    if (sl < one) {
        long double c = x87_sqrt_h2((long double)one - (long double)s * s);
        if (((long double)v[1] * n[1] + (long double)n[2] * v[2]) + d1 < zero) c = c * *(const float *)0xf268f0;
        q0 = (float)(c * n[0]), q1 = (float)(c * n[1]), q2 = (float)(c * n[2]);
        r0 = t0 * s;
        r1 = (float)((long double)t1 * s), r2 = (float)((long double)t2 * s);
    } else {
        q0 = (float)((long double)n[0] * zero);
        r1 = t1;
        q1 = (float)((long double)n[1] * zero);
        q2 = (float)((long double)n[2] * zero);
        r2 = t2;
        r0 = t0;
    }
    const long double o0 = r0 + q0;
    volatile float o1 = (float)((long double)r1 + q1), o2 = (float)((long double)r2 + q2);
    const long double len2 = x87_sqrt_h2(((long double)o2 * o2 + (long double)o1 * o1) + o0 * o0);
    volatile float *const o = out;
    if (len2 == zero) {
        o[2] = 1.0f;
        o[0] = (float)o0;
        o[1] = o1;
        return out;
    }
    const long double inv = (long double)one / len2;
    o[2] = (float)((long double)o2 * inv);
    o[0] = (float)(o0 * inv);
    o[1] = (float)((long double)o1 * inv);
    return out;
}
extern "C" __attribute__((naked)) void forever_water_bend_regs() {   // n in ecx, v in edx, out in esi; ratio on the stack
    __asm__("pushl 4(%esp)\n\tpushl %esi\n\tpushl %edx\n\tpushl %ecx\n\tcalll _forever_water_bend@16\n\tret $4");
}
static const uint8_t water_bend_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x40, 0xd9, 0x02, 0xd8, 0x09};
REWRITE("Water::Length94CA80", 0x94ca80, water_bend_original, (void *)&forever_water_bend_regs, &g_water_bends);

// ---- Sound::Direction(this, out) (0xCE55D0): the point of a sound's segment nearest the listener ------------------------
// The segment runs from a (+0x90) to b (+0xA0); the listener is at 0x10F4620. d = b - a (floats; L = |d| with the z
// square as the unrounded z times the float); n = d / L (x, y floats, z on the FPU; (0, 0, 1) when L is exactly 0).
// Before a (n . (p - a) < 0, summed in the game's order): a. Past b (n . (b - p) < 0): b. Otherwise a + n s, s =
// sqrt(|p - a|^2 - perp^2) (|p - a|^2 as a float there) and perp^2 from (h^2 - (h . d)^2 / L^2) with h = |p - a| as a
// float. NaNs go on.
static volatile long g_sound_directions;
static float *__thiscall sound_direction(const uint8_t *self, float *out) {
    g_sound_directions++;
    const float *const a = (const float *)(self + 0x90), *const b = (const float *)(self + 0xa0), *const p = (const float *)0x10f4620;
    const float a0 = a[0], a1 = a[1], a2 = a[2], b0 = b[0], b1 = b[1], b2 = b[2];
    const float p0 = p[0], p1 = p[1], p2 = p[2];
    volatile float d0 = (float)((long double)b0 - a0), d1 = (float)((long double)b1 - a1);
    const long double d2l = (long double)b2 - a2;
    volatile float d2 = (float)d2l;
    const long double len = x87_sqrt_h2((d2l * d2 + (long double)d1 * d1) + (long double)d0 * d0);
    volatile float lf = (float)len;
    volatile float n0, n1;
    long double n2;
    if (len == *(const float *)0xf264e0) {
        n0 = 0.0f, n1 = 0.0f, n2 = *(const float *)0xf264dc;
    } else {
        const long double inv = (long double)*(const float *)0xf264dc / len;
        n0 = (float)((long double)d0 * inv), n1 = (float)((long double)d1 * inv), n2 = inv * d2;
    }
    const long double m0 = -(long double)n0, m1 = -(long double)n1;
    volatile float mf1 = (float)m1, mf2 = (float)-n2;
    volatile float *const o = out;
    const long double s1 = ((((long double)p2 * n2 + (long double)p1 * n1) + -n2 * a2) + m1 * a1) + (long double)n0 * p0;
    if (s1 + m0 * a0 < 0) {
        *(volatile uint32_t *)(out + 2) = *(const uint32_t *)&a2;
        o[0] = a0, o[1] = a1;
        return out;
    }
    const long double t1 = (((long double)p2 * mf2 + (long double)mf1 * p1) + n2 * b2) + (long double)n1 * b1;
    if ((t1 + m0 * p0) + (long double)n0 * b0 < 0) {
        o[0] = b0;
        *(volatile uint32_t *)(out + 1) = *(const uint32_t *)&b1, *(volatile uint32_t *)(out + 2) = *(const uint32_t *)&b2;
        return out;
    }
    const long double h0 = (long double)p0 - a0, h1 = (long double)p1 - a1, h2 = (long double)p2 - a2;
    const long double q = (h2 * h2 + h1 * h1) + h0 * h0;
    volatile float qf = (float)q;
    volatile float hf = (float)x87_sqrt_h2(q);
    const long double dot = (h2 * d2 + h1 * d1) + h0 * d0;
    const long double perp = x87_sqrt_h2((long double)hf * hf - dot * dot / ((long double)lf * lf));
    const long double along = x87_sqrt_h2((long double)qf - perp * perp);
    const long double x = (long double)n0 * along;
    volatile float y = (float)((long double)n1 * along), z = (float)(n2 * along);
    o[0] = (float)(x + a0);
    o[1] = (float)((long double)y + a1);
    o[2] = (float)((long double)z + a2);
    return out;
}
static const uint8_t sound_direction_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x60, 0x8b, 0x81, 0x90};
REWRITE("Sound::Direction", 0xce55d0, sound_direction_original, &sound_direction, &g_sound_directions);

// ---- Vector::SameC5D1C0(out, p, q) (0xC5D1C0): two near-equal segments joined into one -------------------------------
// fastcall out and p (four floats each, q given). "Close" is a difference below the game's epsilon (0xF27F4C; a NaN is
// not close). x and z close: y close gives {p0, lo(p3, q3), p2, hi(p3, q3)}, else {p0, lo(p1, q1), p2, hi(p1, q1)};
// only x close: {lo(p2, q2), p1, hi(p2, q2), p3}; x not close: {lo(p0, q0), p1, hi(p0, q0), p3}. hi(b, a) is b when a is
// below b (or they are unordered), else a; lo(b, a) is b when a is above b, else a. Gives out.
static volatile long g_segment_joins;
static float *__fastcall vector_same_c5d1c0(float *out, const float *p, const float *q) {
    g_segment_joins++;
    const float eps = *(const float *)0xf27f4c;
    auto close = [&](int k) {
        const long double d = (long double)p[k] - q[k];
        return (d < 0 ? -d : d) < eps;
    };
    auto hi = [](float b, float a) { return a < b || a != a || b != b ? b : a; };
    auto lo = [](float b, float a) { return a > b ? b : a; };
    volatile uint32_t *const o = (volatile uint32_t *)out;
    volatile float *const of = out;
    const uint32_t *const pi = (const uint32_t *)p;
    int pick, keep0, keep1;   // the pair joined (into out[a] and out[b]) and the two parts kept from p
    if (close(0) && close(2)) {
        pick = close(1) ? 3 : 1;
        const float h = hi(p[pick], q[pick]), l = lo(p[pick], q[pick]);
        of[1] = l, o[0] = pi[0], of[3] = h, o[2] = pi[2];
        return out;
    }
    pick = close(0) ? 2 : 0;
    keep0 = 1, keep1 = 3;
    const float h = hi(p[pick], q[pick]), l = lo(p[pick], q[pick]);
    of[0] = l, o[keep0] = pi[keep0], of[2] = h, o[keep1] = pi[keep1];
    return out;
}
static const uint8_t vector_same_c5d1c0_original[] = {0xd9, 0x02, 0x83, 0xec, 0x14, 0x56, 0x8b, 0x74, 0x24, 0x1c};
REWRITE("Vector::SameC5D1C0", 0xc5d1c0, vector_same_c5d1c0_original, &vector_same_c5d1c0, &g_segment_joins);

// ---- Window::PointToScreen and its copies (0xAC7FB0 and ten more): a point in a window turned into a screen place -----
// The window is this interface + D (each copy's own D). out = (window +0x8C + p.x, window +0x90 + p.y) (floats) plus
// Window::ScreenPosition (0x6AD560) of the window, each sum rounded to a float; gives out.
static volatile long g_points_to_screen;
template <int D> static float *__thiscall window_point_to_screen(uint8_t *self, float *out, const float *p) {
    g_points_to_screen++;
    uint8_t *const window = self + D;
    volatile float x = (float)((long double)*(const float *)(window + 0x8c) + p[0]);
    volatile float y = (float)((long double)*(const float *)(window + 0x90) + p[1]);
    float place[2];
    const float *const r = ((const float *(__thiscall *)(uint8_t *, float *))0x6ad560)(window, place);
    const long double sy = (long double)y + r[1];
    const long double sx = (long double)x + r[0];
    volatile float *const o = out;
    o[0] = (float)sx;
    o[1] = (float)sy;
    return out;
}
static const uint8_t window_point_to_screen_d4_original[] = {0x83, 0xec, 0x10, 0x8b, 0x44, 0x24, 0x18, 0xd9, 0x41, 0xb8};
static const uint8_t window_point_to_screen_dc_original[] = {0x83, 0xec, 0x10, 0x8b, 0x44, 0x24, 0x18, 0xd9, 0x41, 0xb0};
static const uint8_t window_point_to_screen_ec_original[] = {0x83, 0xec, 0x10, 0x8b, 0x44, 0x24, 0x18, 0xd9, 0x41, 0xa0};
static const uint8_t window_point_to_screen_114_original[] = {0x83, 0xec, 0x10, 0x8b, 0x44, 0x24, 0x18, 0xd9, 0x81, 0x78};
static const uint8_t window_point_to_screen_13c_original[] = {0x83, 0xec, 0x10, 0x8b, 0x44, 0x24, 0x18, 0xd9, 0x81, 0x50};
REWRITE("Window::PointToScreen", 0xac7fb0, window_point_to_screen_d4_original, &window_point_to_screen<-0xd4>, &g_points_to_screen);
REWRITE("Window::PointToScreen2", 0x99b2b0, window_point_to_screen_d4_original, &window_point_to_screen<-0xd4>, &g_points_to_screen);
REWRITE("Window::PointToScreenB8", 0x9b9cd0, window_point_to_screen_d4_original, &window_point_to_screen<-0xd4>, &g_points_to_screen);
REWRITE("Window::PointToScreenB8", 0xa213a0, window_point_to_screen_d4_original, &window_point_to_screen<-0xd4>, &g_points_to_screen);
REWRITE("Window::PointToScreenB8", 0xa21a40, window_point_to_screen_d4_original, &window_point_to_screen<-0xd4>, &g_points_to_screen);
REWRITE("Window::PointToScreenB8", 0xaccb40, window_point_to_screen_d4_original, &window_point_to_screen<-0xd4>, &g_points_to_screen);
REWRITE("Window::PointToScreenB0", 0x67d320, window_point_to_screen_dc_original, &window_point_to_screen<-0xdc>, &g_points_to_screen);
REWRITE("Window::PointToScreenB0", 0xae2090, window_point_to_screen_dc_original, &window_point_to_screen<-0xdc>, &g_points_to_screen);
REWRITE("Window::PointToScreenA0", 0x9973d0, window_point_to_screen_ec_original, &window_point_to_screen<-0xec>, &g_points_to_screen);
REWRITE("Window::PointToScreen78", 0x67f300, window_point_to_screen_114_original, &window_point_to_screen<-0x114>, &g_points_to_screen);
REWRITE("Window::PointToScreen50", 0x996680, window_point_to_screen_13c_original, &window_point_to_screen<-0x13c>, &g_points_to_screen);

// ---- Math::ScaleC198C0 / C19910 / C19960(n) (stdcall): a bit field from n times a scale --------------------------------
// n (whole) times the copy's scale K, as a float, rounded down (the game's frndint floor) to a whole number by fistp;
// its low bits (mask M) shifted up by S. C198C0: K 0xFAFE8C, M 0x1FF, S 10; C19910: K 0xFAFE90, M 0x3FF, S 20;
// C19960: K 0xFAFE94, M 0x1FF, S 10.
static volatile long g_scale_fields;
template <uint32_t K, uint32_t M, int S> static uint32_t __stdcall math_scale_field(int32_t n) {
    g_scale_fields++;
    volatile float v = (float)((long double)n * *(const float *)(uintptr_t)K);
    volatile float f = (float)floor_h2(v);
    return ((uint32_t)x87_fistp64_h2(f) & M) << S;
}
static const uint8_t math_scale_field_original[] = {0x83, 0xec, 0x08, 0xdb, 0x44, 0x24, 0x0c, 0xd8, 0x0d};
REWRITE("Math::ScaleC198C0", 0xc198c0, math_scale_field_original, (&math_scale_field<0xfafe8c, 0x1ff, 10>), &g_scale_fields);
REWRITE("Math::ScaleC19910", 0xc19910, math_scale_field_original, (&math_scale_field<0xfafe90, 0x3ff, 20>), &g_scale_fields);
REWRITE("Math::ScaleC19960", 0xc19960, math_scale_field_original, (&math_scale_field<0xfafe94, 0x1ff, 10>), &g_scale_fields);

// ---- Window::RectFromSize and its copies (0x490000, 0x448380, 0xA1FAF0, 0xA1FB70): a part sized like this window ------
// The rectangle {0, 0, width (+0x84 - +0x7C), height (+0x88 - +0x80)} (floats) given to Window::UpdateRect (0x6AFAF0)
// of the part at +P (0xD4 or 0xF4); gives what that gives.
static volatile long g_rects_from_size;
template <uint32_t P> static uint32_t __fastcall window_rect_from_size(const uint8_t *self) {
    g_rects_from_size++;
    float rect[4];
    *(volatile uint32_t *)&rect[0] = 0, *(volatile uint32_t *)&rect[1] = 0;
    *(volatile float *)&rect[2] = (float)((long double)*(const float *)(self + 0x84) - *(const float *)(self + 0x7c));
    *(volatile float *)&rect[3] = (float)((long double)*(const float *)(self + 0x88) - *(const float *)(self + 0x80));
    return ((uint32_t(__thiscall *)(void *, float *))0x6afaf0)(*(void *const *)(self + P), rect);
}
static const uint8_t window_rect_from_size_original[] = {0x83, 0xec, 0x10, 0xd9, 0x81, 0x84, 0x00, 0x00, 0x00};
REWRITE("Window::RectFromSize", 0x490000, window_rect_from_size_original, &window_rect_from_size<0xd4>, &g_rects_from_size);
REWRITE("Effect::Width448380", 0x448380, window_rect_from_size_original, &window_rect_from_size<0xf4>, &g_rects_from_size);
REWRITE("Window::WidthA1FAF0", 0xa1faf0, window_rect_from_size_original, &window_rect_from_size<0xf4>, &g_rects_from_size);
REWRITE("Window::WidthA1FB70", 0xa1fb70, window_rect_from_size_original, &window_rect_from_size<0xf4>, &g_rects_from_size);

// ---- Effect::Hit488A00 and its copies (0x488A00, 0x7B3580, 0xB5BB70, 0xB6BFF0): whether a short ray hits ------------
// this, a, b, c: the ray runs from (a.x, c.y, a.y) to (a.x + b.x, c.z, a.y + b.y) (the sums floats) and is given, with a
// place for the result, to 0xCDA7F0 (fastcall this +0 and +4); 1 when that says so (its low byte), else 0.
static volatile long g_short_ray_hits;
static uint32_t __thiscall effect_hit_488a00(const uint32_t *self, const float *c, const float *a, const float *b) {
    g_short_ray_hits++;
    float from[3], to[3], result;
    volatile float *const f = from, *const t = to;
    t[0] = (float)((long double)a[0] + b[0]);
    t[2] = (float)((long double)b[1] + a[1]);
    *(volatile uint32_t *)&from[0] = *(const uint32_t *)&a[0];
    *(volatile uint32_t *)&from[1] = *(const uint32_t *)&c[1];
    *(volatile uint32_t *)&from[2] = *(const uint32_t *)&a[1];
    *(volatile uint32_t *)&to[1] = *(const uint32_t *)&c[2];
    (void)f;
    typedef uint8_t(__fastcall *Hit)(uint32_t, uint32_t, float *, float *, float *);
    return ((Hit)0xcda7f0)(self[0], self[1], from, to, &result) != 0;
}
static const uint8_t effect_hit_488a00_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x28, 0x8b, 0x45, 0x0c};
REWRITE("Effect::Hit488A00", 0x488a00, effect_hit_488a00_original, &effect_hit_488a00, &g_short_ray_hits);
REWRITE("Effect::Hit7B3580", 0x7b3580, effect_hit_488a00_original, &effect_hit_488a00, &g_short_ray_hits);
REWRITE("Effect::HitB5BB70", 0xb5bb70, effect_hit_488a00_original, &effect_hit_488a00, &g_short_ray_hits);
REWRITE("Effect::HitB6BFF0", 0xb6bff0, effect_hit_488a00_original, &effect_hit_488a00, &g_short_ray_hits);

// ---- WildAnimal::InterfaceB35450 and its copies (0xB35450 to 0xB35670): a value of the animal's current size class ---
// The animal's interface (0xB44970 on this) has a record (+0x54) holding four entries (+0x18 to +0x24); the one for the
// animal's class (+0xA4: 0, 1, 2, anything else) gives its float at +F (each copy's own F), on the FPU.
static volatile long g_animal_class_values;
template <uint32_t F> static long double __thiscall wild_animal_class_value(uint8_t *self) {
    g_animal_class_values++;
    const int32_t kind = *(const int32_t *)(self + 0xa4);
    const uint8_t *const iface = ((const uint8_t *(__thiscall *)(uint8_t *))0xb44970)(self);
    const uint8_t *const rec = *(const uint8_t *const *)(iface + 0x54);
    const uint32_t at = kind == 0 ? 0x18 : kind == 1 ? 0x1c : kind == 2 ? 0x20 : 0x24;
    return *(const float *)(*(const uint8_t *const *)(rec + at) + F);
}
static const uint8_t wild_animal_class_value_original[] = {0x56, 0x8b, 0xb1, 0xa4, 0x00, 0x00, 0x00, 0xe8};
REWRITE("WildAnimal::InterfaceB35450", 0xb35450, wild_animal_class_value_original, &wild_animal_class_value<0x18>, &g_animal_class_values);
REWRITE("WildAnimal::InterfaceB355B0", 0xb355b0, wild_animal_class_value_original, &wild_animal_class_value<0x34>, &g_animal_class_values);
REWRITE("WildAnimal::InterfaceB355F0", 0xb355f0, wild_animal_class_value_original, &wild_animal_class_value<0x38>, &g_animal_class_values);
REWRITE("WildAnimal::InterfaceB35630", 0xb35630, wild_animal_class_value_original, &wild_animal_class_value<0x3c>, &g_animal_class_values);
REWRITE("WildAnimal::InterfaceB35670", 0xb35670, wild_animal_class_value_original, &wild_animal_class_value<0x40>, &g_animal_class_values);

// ---- AttractionPersonStrategy::PointSlot35 and its copies (0x5F7750, 0x5F77B0, 0x5F7810, 0x5F7890, 0x9EE850) ---------
// A point in this window, (+0x7C + (+0x8C + p.x), +0x80 + (+0x90 + p.y)) (the inner sums on the FPU, the outer rounded
// to floats), handed to slot S of the GUI root (0x12F83F0); gives what that gives.
static volatile long g_root_points;
template <uint32_t S> static uint32_t __thiscall window_point_to_root(const uint8_t *self, const float *p) {
    g_root_points++;
    auto f = [&](uint32_t at) { return *(const float *)(self + at); };
    const long double a = (long double)f(0x8c) + p[0], b = (long double)f(0x90) + p[1];
    float point[2];
    *(volatile float *)&point[0] = (float)((long double)f(0x7c) + a);
    *(volatile float *)&point[1] = (float)((long double)f(0x80) + b);
    uint8_t *const root = *(uint8_t *const volatile *)0x12f83f0;
    typedef uint32_t(__thiscall *Slot)(uint8_t *, float *);
    return (*(Slot const *)(*(const uint8_t *const *)root + S))(root, point);
}
static const uint8_t window_point_to_root_original[] = {0x83, 0xec, 0x10, 0x8b, 0x54, 0x24, 0x14, 0x8b, 0xc1};
REWRITE("AttractionPersonStrategy::PointSlot35", 0x5f7750, window_point_to_root_original, &window_point_to_root<0x8c>, &g_root_points);
REWRITE("AttractionPersonStrategy::PointSlot38", 0x5f77b0, window_point_to_root_original, &window_point_to_root<0x98>, &g_root_points);
REWRITE("AttractionPersonStrategy::PointSlot41", 0x5f7810, window_point_to_root_original, &window_point_to_root<0xa4>, &g_root_points);
REWRITE("AttractionPersonStrategy::PointSlot20", 0x5f7890, window_point_to_root_original, &window_point_to_root<0x50>, &g_root_points);
REWRITE("Window::PointSlot35", 0x9ee850, window_point_to_root_original, &window_point_to_root<0x8c>, &g_root_points);

// ---- Guest::Shared7594E0 and its copies (0x7594E0, 0x759570, 0x7595F0): a guest value set near the park's, at random --
// v = the shared park object's (0x1306194, made by 0x708520 when there is none) +SRC; the guest's +DST becomes
// v + Random::Ran0 x K1 + K2 (kept as a float), then kept between 0 and 1 (a NaN gives 1).
static volatile long g_guest_shared;
template <uint32_t SRC, uint32_t DST, uint32_t K1, uint32_t K2> static void __fastcall guest_shared(uint8_t *self) {
    g_guest_shared++;
    const uint8_t *park = *(const uint8_t *const volatile *)0x1306194;
    if (!park) park = ((const uint8_t *(*)())0x708520)();
    volatile uint32_t *const dst = (volatile uint32_t *)(self + DST);
    *dst = *(const uint32_t *)(park + SRC);
    const long double r = ((long double(__cdecl *)())0x475180)();
    volatile float t = (float)((r * *(const float *)(uintptr_t)K1 + *(volatile float *)dst) + *(const float *)(uintptr_t)K2);
    *(volatile float *)dst = t;
    const float lo = 0.0f < t || t != t ? (float)t : 0.0f;
    const float v = 1.0f > lo ? lo : 1.0f;
    volatile float out = v;
    *dst = *(volatile uint32_t *)&out;
}
static const uint8_t guest_shared_original[] = {0xa1, 0x94, 0x61, 0x30, 0x01, 0x83, 0xec, 0x10};
REWRITE("Guest::Shared7594E0", 0x7594e0, guest_shared_original, (&guest_shared<0x168, 0x40c, 0xf283d8, 0xf270b0>), &g_guest_shared);
REWRITE("Guest::Shared759570", 0x759570, guest_shared_original, (&guest_shared<0x16c, 0x428, 0xf27ca8, 0xf33750>), &g_guest_shared);
REWRITE("Guest::Shared7595F0", 0x7595f0, guest_shared_original, (&guest_shared<0x170, 0x42c, 0xf27ca8, 0xf33750>), &g_guest_shared);

// ---- Colour::Make6B5D10(out, h, s, l) (0x6B5D10): a colour (0xAARRGGBB, alpha 0xFF) from hue, saturation and lightness ---
// H = h K6 (0xF2706C; the first test unrounded, the rest on the float). By sector it picks (a, b, c) for red, green and
// blue (a and b floats, c kept on the FPU): H not below 0 and below K1: (1, H - 1, Q); below K2: (K3 - H, 1, Q); below K4:
// (-1, 1, H - K5); below K6b: (-1, K7 - H, 1); below K8: (H - K9, -1, 1); else (1, -1, K10 - H). m = s K11 (1 - l when
// l + m is above 1), then l when l - m is below 0. Each part p gives the whole
// number p m 255 + l 255 (fistp); they are packed as ((r | 0xFFFFFF00) << 8 | g) << 8 | b in 32 bits. Gives out.
static volatile long g_colour_makes;
static uint32_t *__thiscall colour_make_6b5d10(uint32_t *out, float h, float s, float l) {
    g_colour_makes++;
    auto k = [](uintptr_t at) { return *(const float *)at; };
    const long double hl = (long double)h * k(0xf2706c);
    volatile float hf = (float)hl;
    const float one = k(0xf264dc);
    volatile float a = -1.0f, b = -1.0f;
    long double c;
    const float H = hf;
    if (!(hl < k(0xf264e0)) && hl == hl && H < k(0xf2705c)) {
        b = (float)((long double)H - one), a = 1.0f, c = k(0xf268f0);
    } else if (H < k(0xf2669c)) {
        b = 1.0f, a = (float)((long double)k(0xf2784c) - H), c = k(0xf268f0);
    } else if (H < k(0xf26610)) {
        c = (long double)H - k(0xf28214), b = 1.0f;
    } else if (H < k(0xf270e8)) {
        c = one, b = (float)((long double)k(0xf360ac) - H);
    } else if (H < k(0xf275f4)) {
        c = one, a = (float)((long double)H - k(0xf27848));
    } else {
        c = (long double)k(0xf31988) - H, a = 1.0f;
    }
    long double m = (long double)s * k(0xf270b8);
    const long double sum = (long double)l + m;
    if (sum > one) m = (long double)one - l;
    if ((long double)l - m < k(0xf264e0)) m = l;
    const float full = k(0xf264c0);
    const long double base = (long double)l * full;
    const uint32_t r = (uint32_t)x87_fistp64_h2((long double)a * m * full + base);
    uint32_t v = (r | 0xffffff00u) << 8;
    const uint32_t g = (uint32_t)x87_fistp64_h2((long double)b * m * full + base);
    v = (v | g) << 8;
    const uint32_t bl = (uint32_t)x87_fistp64_h2(c * m * full + base);
    *(volatile uint32_t *)out = v | bl;
    return out;
}
static const uint8_t colour_make_6b5d10_original[] = {0x83, 0xec, 0x10, 0xd9, 0x05, 0xf0, 0x68, 0xf2, 0x00};
REWRITE("Colour::Make6B5D10", 0x6b5d10, colour_make_6b5d10_original, &colour_make_6b5d10, &g_colour_makes);

// ---- Water::Fade919610(this, out, x, y, edge) (0x919610): how strongly a water cell is drawn across and along ---------
// N is the shared fade width (0x10F0BEC). With edge set: x not past N gives 1 - (x / N) K, else (w - x - 1) / N K (w the
// width +0x154), less from 1 when flag 0x40 of +0x40 is set. Without: 1 - (x - N) / (w - 2N - 1), times 1, K2 or K3 by
// the level +0x31C (below 0xF28210, below 0xF360E8, else). out[0] is that; out[1] = (1 - y / (h - 1)) (h +0x158) times 1,
// K2, K3 or K4 by +0x320 (below 0xF28210, 0xF27604, 0xF27618, else). K 0xF270B8, K2 0xF2705C, K3 0xF2784C, K4 0xF2669C.
static volatile long g_water_fades;
static float *__thiscall water_fade_919610(const uint8_t *self, float *out, int32_t x, int32_t y, uint32_t edge) {
    g_water_fades++;
    auto k = [](uintptr_t at) { return *(const float *)at; };
    const float one = k(0xf264dc);
    const int32_t n = *(const volatile int32_t *)0x10f0bec, w = *(const int32_t *)(self + 0x154);
    long double u;
    if ((uint8_t)edge) {
        if (x <= n) {
            u = (long double)one - (long double)x / n * k(0xf270b8);
        } else {
            const long double part = (long double)(int32_t)((uint32_t)w - (uint32_t)x - 1) / n * k(0xf270b8);
            u = (self[0x40] & 0x40) ? (long double)one - part : part;
        }
    } else {
        const int32_t d = (int32_t)((uint32_t)x - (uint32_t)n), den = (int32_t)((uint32_t)w - 2 * (uint32_t)n - 1);
        u = (long double)one - (long double)d / den;
        const float lv = *(const float *)(self + 0x31c);
        u = u * (lv < k(0xf28210) ? one : lv < k(0xf360e8) ? k(0xf2705c) : k(0xf2784c));
    }
    const int32_t h1 = (int32_t)(*(const uint32_t *)(self + 0x158) - 1);
    const long double v = (long double)one - (long double)y / h1;
    const float lv2 = *(const float *)(self + 0x320);
    const float g = lv2 < k(0xf28210) ? one : lv2 < k(0xf27604) ? k(0xf2705c) : lv2 < k(0xf27618) ? k(0xf2784c) : k(0xf2669c);
    volatile float *const o = out;
    o[0] = (float)u;
    o[1] = (float)((long double)g * v);
    return out;
}
static const uint8_t water_fade_919610_original[] = {0x8a, 0x44, 0x24, 0x10, 0x84, 0xc0, 0x74, 0x59};
REWRITE("Water::Fade919610", 0x919610, water_fade_919610_original, &water_fade_919610, &g_water_fades);

// ---- Vector::ScaleBEE2C0(this, out) (0xBEE2C0): the rotation (quaternion x, y, z, w) of a 3x3 matrix (rows 16 bytes) ----
// t = (m22 + m11) + m00. Above 0: s = sqrt(t + 1), out = ((m12 - m21), (m20 - m02), (m01 - m10)) K / s and w = s K
// (K 0xF2651C). Otherwise i is the largest diagonal (1 when m11 is above m00, then 2 when m22 is above that), j and k the
// next two (1, 2, 0); s = sqrt((mii - (mkk + mjj)) + 1), q[i] = K s, s' = K / s (0 stays 0), w = (mjk - mkj) s',
// q[j] = (mji + mij) s', q[k] = (mki + mik) s'. Floats where stored.
static volatile long g_matrix_rotations;
static void __thiscall matrix_to_rotation(const float *self, float *out) {
    g_matrix_rotations++;
    auto m = [&](int r, int c) { return self[4 * r + c]; };
    const float half = *(const float *)0xf2651c, one = *(const float *)0xf264dc;
    const long double t = ((long double)m(2, 2) + m(1, 1)) + m(0, 0);
    volatile float *const o = out;
    if (t > *(const float *)0xf264e0) {
        const long double s = x87_sqrt_h2(t + one);
        const long double inv = (long double)half / s;
        o[0] = (float)(((long double)m(1, 2) - m(2, 1)) * inv);
        o[1] = (float)(((long double)m(2, 0) - m(0, 2)) * inv);
        o[2] = (float)(((long double)m(0, 1) - m(1, 0)) * inv);
        o[3] = (float)(s * half);
        return;
    }
    static const int next[3] = {1, 2, 0};
    int i = m(1, 1) > m(0, 0) ? 1 : 0;
    if (m(2, 2) > m(i, i)) i = 2;
    const int j = next[i], k = next[j];
    long double s = x87_sqrt_h2(((long double)m(i, i) - ((long double)m(k, k) + m(j, j))) + one);
    volatile float q[4];
    q[i] = (float)((long double)half * s);
    if (!(s == *(const float *)0xf264e0)) s = (long double)half / s;
    q[3] = (float)(((long double)m(j, k) - m(k, j)) * s);
    q[j] = (float)(((long double)m(j, i) + m(i, j)) * s);
    q[k] = (float)(((long double)m(k, i) + m(i, k)) * s);
    *(volatile uint32_t *)(out + 1) = *(volatile uint32_t *)&q[1];
    o[0] = q[0];
    *(volatile uint32_t *)(out + 2) = *(volatile uint32_t *)&q[2];
    *(volatile uint32_t *)(out + 3) = *(volatile uint32_t *)&q[3];
}
static const uint8_t matrix_to_rotation_original[] = {0x83, 0xec, 0x50, 0x8b, 0x01, 0x8b, 0x51, 0x04};
REWRITE("Vector::ScaleBEE2C0", 0xbee2c0, matrix_to_rotation_original, &matrix_to_rotation, &g_matrix_rotations);

// ---- Animal::ModeA6F930(this) (0xA6F930): four points put in turning order, in mode 2 (+0 = 2) --------------------------
// The four records (+0xC's pointers; a point at +0x10 and a second vector at +0x20). n = (p2 - p0) x (p1 - p0) as the
// game works it (u = p1 - p0 with x a float, v = p2 - p0 with x and y floats; n's parts floats); when (p3 - p0) . n is
// not above 0, records 1 and 2 swap both vectors.
static volatile long g_animal_orders;
static void __fastcall animal_mode_a6f930(const uint8_t *self) {
    g_animal_orders++;
    if (*(const int32_t *)self != 2) return;
    auto rec = [&](int i) { return (uint8_t *)(uintptr_t)((*(const uint32_t *const *)(self + 0xc))[i]); };
    const float *const p0 = (const float *)(rec(0) + 0x10), *const p1 = (const float *)(rec(1) + 0x10);
    const float *const p2 = (const float *)(rec(2) + 0x10), *const p3 = (const float *)(rec(3) + 0x10);
    const long double a0 = p0[0], a1 = p0[1];
    volatile float u0 = (float)((long double)p1[0] - a0);
    const long double u1 = (long double)p1[1] - a1, u2 = (long double)p1[2] - p0[2];
    volatile float v0 = (float)((long double)p2[0] - a0), v1 = (float)((long double)p2[1] - a1);
    const long double v2 = (long double)p2[2] - p0[2];
    volatile float n0 = (float)(v2 * u1 - (long double)v1 * u2);
    volatile float n1 = (float)(u2 * v0 - v2 * u0);
    volatile float n2 = (float)((long double)v1 * u0 - u1 * v0);
    volatile float w0 = (float)((long double)p3[0] - a0);
    const long double w1 = (long double)p3[1] - a1, w2 = (long double)p3[2] - p0[2];
    const long double d = ((long double)n2 * w2 + w1 * n1) + (long double)w0 * n0;
    if (d > *(const float *)0xf264e0 || d != d) return;
    uint8_t *const r1 = rec(1);
    volatile float sx = *(const float *)(r1 + 0x10), sy = *(const float *)(r1 + 0x14);
    const uint32_t sz = *(const uint32_t *)(r1 + 0x18);
    const uint32_t s20 = *(const uint32_t *)(r1 + 0x20), s24 = *(const uint32_t *)(r1 + 0x24), s28 = *(const uint32_t *)(r1 + 0x28);
    for (uint32_t at = 0x10; at <= 0x28; at += 4) {
        if (at == 0x1c) continue;
        *(volatile uint32_t *)(rec(1) + at) = *(const volatile uint32_t *)(rec(2) + at);
    }
    uint8_t *const r2 = rec(2);
    *(volatile float *)(r2 + 0x10) = sx;
    *(volatile uint32_t *)(r2 + 0x18) = sz;
    *(volatile float *)(r2 + 0x14) = sy;
    *(volatile uint32_t *)(r2 + 0x20) = s20, *(volatile uint32_t *)(r2 + 0x24) = s24, *(volatile uint32_t *)(r2 + 0x28) = s28;
}
static const uint8_t animal_mode_a6f930_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x58, 0x8b, 0x01, 0x83};
REWRITE("Animal::ModeA6F930", 0xa6f930, animal_mode_a6f930_original, &animal_mode_a6f930, &g_animal_orders);

// ---- Animal::StepBB6B10(out, a, w, b, t, add) (0xBB6B10): a point between a and b, weighted into out ------------------
// fastcall out and a. p = a + (b - a) t (x parts floats, y on the FPU, z rounded once), or a when t is exactly the game's
// 0 (a NaN t blends). With add: out += p w (the x product a float, y and z on the FPU). Without: out = p, then out *= w
// when w is below 1.
static volatile long g_animal_blends;
static void __fastcall animal_step_bb6b10(float *out, const float *a, float w, const float *b, float t, uint32_t add) {
    g_animal_blends++;
    volatile float *const o = out;
    const bool blend = !(t == *(const float *)0xf264e0);
    const long double a0 = a[0], a1 = a[1];
    if ((uint8_t)add) {
        if (blend) {
            volatile float d0 = (float)((long double)b[0] - a0);
            const long double d1 = (long double)b[1] - a1, d2 = (long double)b[2] - a[2];
            volatile float e0 = (float)((long double)d0 * t), e1 = (float)(d1 * t);
            const long double e2 = d2 * t;
            volatile float p0 = (float)((long double)e0 + a[0]);
            const long double p1 = (long double)a[1] + e1;
            volatile float p2 = (float)((long double)a[2] + e2);
            volatile float q0 = (float)((long double)p0 * w);
            const long double q1 = p1 * w, q2 = (long double)p2 * w;
            o[0] = (float)((long double)q0 + o[0]);
            o[1] = (float)(q1 + o[1]);
            o[2] = (float)(q2 + o[2]);
        } else {
            volatile float q0 = (float)(a0 * w);
            const long double q1 = a1 * w, q2 = (long double)a[2] * w;
            o[0] = (float)((long double)q0 + o[0]);
            o[1] = (float)(q1 + o[1]);
            o[2] = (float)(q2 + o[2]);
        }
        return;
    }
    uint32_t zbits;
    if (blend) {
        volatile float d0 = (float)((long double)b[0] - a0);
        const long double d1 = (long double)b[1] - a1, d2 = (long double)b[2] - a[2];
        volatile float e0 = (float)((long double)d0 * t), e1 = (float)(d1 * t);
        const long double e2 = d2 * t;
        const long double p0 = a0 + e0, p1 = (long double)a[1] + e1;
        volatile float p2 = (float)((long double)a[2] + e2);
        zbits = *(volatile uint32_t *)&p2;
        o[0] = (float)p0;
        o[1] = (float)p1;
    } else {
        o[0] = (float)a0;
        zbits = *(const uint32_t *)(a + 2);
        o[1] = (float)a1;
    }
    *(volatile uint32_t *)(out + 2) = zbits;
    if (w < *(const float *)0xf264dc) {
        o[0] = (float)((long double)w * o[0]);
        o[1] = (float)((long double)w * o[1]);
        o[2] = (float)((long double)w * o[2]);
    }
}
static const uint8_t animal_step_bb6b10_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x3c, 0xd9, 0x05, 0xe0};
REWRITE("Animal::StepBB6B10", 0xbb6b10, animal_step_bb6b10_original, &animal_step_bb6b10, &g_animal_blends);

// ---- Graph::RangeACA200(out, colour) (0xACA200): hue, saturation and lightness of a colour (0x..RRGGBB) ---------------
// R, G, B = the bytes times K (0xF270F0) as floats. lo = min(min(B, G), R); hi = max(min(B, G), R) (the game takes the
// smaller of B and G again here); d = hi - lo and L = (hi + lo) / 2 (floats). S = d / (2L, or K2 - 2L from L = 0.5 on)
// when L is between 0 and 1 (else 0). H = 0 unless d is above 0: by which part hi equals, (G - B) / d, + (B - R) / d + K2
// and + (R - G) / d + K3 (each when the part is hi and the next is not), times K4 (0xF30500). out = {H, S, L}; gives out.
// min(x, y) is y when x is above y, else x; max(x, y) is y when x is below y or they are unordered, else x.
static volatile long g_colour_ranges;
static float *__stdcall graph_range_aca200(float *out, uint32_t colour) {
    g_colour_ranges++;
    auto k = [](uintptr_t at) { return *(const float *)at; };
    const float kk = k(0xf270f0), zero = k(0xf264e0), one = k(0xf264dc), half = k(0xf2651c), two = k(0xf2705c);
    volatile float b = (float)((long double)(int32_t)(colour & 0xff) * kk);
    volatile float g = (float)((long double)(int32_t)(colour >> 8 & 0xff) * kk);
    volatile float r = (float)((long double)(int32_t)(colour >> 16 & 0xff) * kk);
    auto mn = [](float x, float y) { return x > y ? y : x; };
    auto mx = [](float x, float y) { return x < y || x != x || y != y ? y : x; };
    const float bg = mn(b, g);
    const float lo = mn(bg, r);
    const float hi = mx(mn(b, g), r);
    volatile float d = (float)((long double)hi - lo);
    volatile float l = (float)(((long double)hi + lo) * half);
    float s = 0.0f;
    if (l > zero && l < one) {
        const long double twice = (long double)l + l;
        const long double den = l < half ? twice : (long double)two - twice;
        s = (float)((long double)d / den);
    }
    volatile float h = zero;
    if (d > zero) {
        long double acc = zero;
        if (r == hi && g != hi) acc = ((long double)g - b) / d;
        if (g == hi && b != hi) acc = acc + (((long double)b - r) / d + two);
        if (b == hi && r != hi) acc = acc + (((long double)r - g) / d + k(0xf2669c));
        h = (float)(acc * k(0xf30500));
    }
    volatile float *const o = out;
    o[0] = h, o[1] = s, o[2] = l;
    return out;
}
static const uint8_t graph_range_aca200_original[] = {0x83, 0xec, 0x24, 0x8b, 0x44, 0x24, 0x2c, 0x8b, 0xc8};
REWRITE("Graph::RangeACA200", 0xaca200, graph_range_aca200_original, &graph_range_aca200, &g_colour_ranges);

// ---- Water::Edge95F1E0 (0x95F1E0): a ray against a water triangle, with its place on the triangle written out -----------
// p in edi, a in esi, b in edx, the direction d in ecx; o, t out, u out and v out on the stack. e1 = p - o (on the FPU),
// e2 = a - o (floats, z also kept), w = o - b (floats, z also kept); n = e2 x e1 (n2 a float); t = (w . n) / (n . d)
// (written out); r = w - t d (t dy, t dz floats); a1 = r . e1 and a2 = r . e2 (floats); with E11 (a float), E12 and E22
// and det's inverse a float, u = (E12 a2 - E22 a1) / det, v = (E12 a1 - E11 a2) / det (floats, written out). 1 when
// u and v are not below 0 and u + v is not above 1, else 0. Sums in the game's orders.
static volatile long g_water_edges;
extern "C" uint32_t __stdcall forever_water_edge(const float *p, const float *a, const float *b, const float *d, const float *o, float *t_out, float *u_out, float *v_out) {
    g_water_edges++;
    const long double e10 = (long double)p[0] - o[0], e11 = (long double)p[1] - o[1], e12 = (long double)p[2] - o[2];
    volatile float e20 = (float)((long double)a[0] - o[0]), e21 = (float)((long double)a[1] - o[1]);
    const long double e22l = (long double)a[2] - o[2];
    volatile float e22 = (float)e22l;
    const long double n0 = e22l * e11 - (long double)e21 * e12;
    const long double n1 = e12 * e20 - (long double)e22 * e10;
    volatile float n2 = (float)((long double)e21 * e10 - (long double)e20 * e11);
    volatile float w0 = (float)((long double)o[0] - b[0]), w1 = (float)((long double)o[1] - b[1]);
    const long double w2l = (long double)o[2] - b[2];
    volatile float w2 = (float)w2l;
    const long double num = (w2l * n2 + (long double)w1 * n1) + (long double)w0 * n0;
    const long double den = ((long double)n2 * d[2] + n1 * d[1]) + n0 * d[0];
    const long double t = num / den;
    *(volatile float *)t_out = (float)t;
    const long double q0 = t * d[0];
    volatile float q1 = (float)(t * d[1]), q2 = (float)(t * d[2]);
    const long double r0 = (long double)w0 - q0, r1 = (long double)w1 - q1, r2 = (long double)w2 - q2;
    volatile float a1 = (float)((r2 * e12 + r1 * e11) + r0 * e10);
    volatile float a2 = (float)((r2 * e22 + r1 * e21) + r0 * e20);
    volatile float ee11 = (float)((e12 * e12 + e10 * e10) + e11 * e11);
    const long double ee12 = ((long double)e22 * e12 + (long double)e21 * e11) + (long double)e20 * e10;
    const long double ee22 = ((long double)e22 * e22 + (long double)e21 * e21) + (long double)e20 * e20;
    volatile float inv = (float)((long double)*(const float *)0xf264dc / ((long double)ee11 * ee22 - ee12 * ee12));
    volatile float u = (float)((ee12 * a2 - ee22 * a1) * inv);
    *(volatile uint32_t *)u_out = *(volatile uint32_t *)&u;
    volatile float v = (float)((ee12 * a1 - (long double)ee11 * a2) * inv);
    *(volatile uint32_t *)v_out = *(volatile uint32_t *)&v;
    const float zero = *(const float *)0xf264e0;
    if (u < zero || u != u) return 0;
    if (v < zero || v != v) return 0;
    const long double s = (long double)v + u;
    return s < *(const float *)0xf264dc || s == *(const float *)0xf264dc;
}
extern "C" __attribute__((naked)) void forever_water_edge_regs() {   // p edi, a esi, b edx, d ecx; o, t, u, v on the stack
    __asm__("pushl 16(%esp)\n\tpushl 16(%esp)\n\tpushl 16(%esp)\n\tpushl 16(%esp)\n\tpushl %ecx\n\tpushl %edx\n\tpushl %esi\n\tpushl %edi\n\tcalll _forever_water_edge@32\n\tret $16");
}
static const uint8_t water_edge_95f1e0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x50, 0x8b, 0x45, 0x08};
REWRITE("Water::Edge95F1E0", 0x95f1e0, water_edge_95f1e0_original, (void *)&forever_water_edge_regs, &g_water_edges);

// ---- Camera::InverseView(this, out) (0xCD78A0): the inverse of the camera's view matrix -------------------------------
// The 4x4 {right (+0x120), up (+0x130), forward (+0x140), each with 0, and the place (+0x160) with 1} inverted by
// Matrix::Invert4 (0xC1FC30); out takes the 16 values it gives back. Gives out.
static volatile long g_inverse_views;
static uint32_t *__thiscall camera_inverse_view(const uint8_t *self, uint32_t *out) {
    g_inverse_views++;
    alignas(16) uint32_t m[16];
    const uint32_t *const s = (const uint32_t *)self;
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) m[4 * r + c] = s[(0x120 + 16 * r) / 4 + c];
        m[4 * r + 3] = 0;
    }
    m[12] = s[0x160 / 4], m[13] = s[0x164 / 4], m[14] = s[0x168 / 4], m[15] = 0x3f800000;
    const uint32_t *const inv = ((const uint32_t *(__thiscall *)(uint32_t *))0xc1fc30)(m);
    volatile uint32_t *const o = out;
    for (int i = 0; i < 16; i++) o[i] = inv[i];
    return out;
}
static const uint8_t camera_inverse_view_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x40, 0x8b, 0x81, 0x20};
REWRITE("Camera::InverseView", 0xcd78a0, camera_inverse_view_original, &camera_inverse_view, &g_inverse_views);

// ---- Text::ReadNumberCBA820(text, out) (0xCBA820): a whole number (64 bits) read from the start of a C string ----------
// fastcall. (A first tab or space makes the game spin there for ever: its skipping loop never moves on; kept so.) An
// optional sign, then digits (bytes compared signed): each step stops before going past 0x0CCCCCCCCCCCCCCC, and after
// multiplying by 10 stops when the digit would pass the largest value (or the smallest, negated) - the value then keeps
// that multiplication. No digits: 0. Otherwise out takes the value (negated for '-') and the place after the digits is
// given.
static volatile long g_number_reads;
static uint32_t __fastcall text_read_number(const char *s, int64_t *out) {
    g_number_reads++;
    const char c = *s;
    if (c == 9 || c == ' ')
        for (;;) (void)*(const volatile char *)s;
    bool negative = false;
    if (c == '-') negative = true, s++;
    else if (c == '+') s++;
    const char *const start = s;
    uint64_t v = 0;
    if ((signed char)*s < '0') return 0;
    for (;;) {
        const signed char d = (signed char)*s;
        if (d > '9' || v > 0x0cccccccccccccccull) break;
        v *= 10;
        const uint64_t digit = (uint64_t)(int64_t)(d - '0');
        if (v > (negative ? 0x8000000000000000ull : 0x7fffffffffffffffull) - digit) break;
        v += digit;
        s++;
        if ((signed char)*s < '0') break;
    }
    if (s == start) return 0;
    if (negative) v = 0 - v;
    *(volatile int64_t *)out = (int64_t)v;
    return (uint32_t)(uintptr_t)s;
}
static const uint8_t text_read_number_original[] = {0x83, 0xec, 0x10, 0x8b, 0xc1, 0x8a, 0x08, 0x89, 0x54, 0x24, 0x0c};
REWRITE("Text::ReadNumberCBA820", 0xcba820, text_read_number_original, &text_read_number, &g_number_reads);

// ---- Math::Split9F8680(a, b) (0x9F8680): whether one 64-bit list of four 16-bit parts is found in order in another ---
// Each 64-bit value (low part first) splits into four 16-bit parts, each less 1 (so 0 becomes -1, "none"). a's parts up to
// the first none must each be met in b, in order, going forward: b's parts are passed while below the one looked for;
// a none in b, a larger part or the end means 0. Gives 1 (low byte) when all are met.
static volatile long g_part_lists;
static uint8_t __stdcall math_split_9f8680(uint32_t a_lo, uint32_t a_hi, uint32_t b_lo, uint32_t b_hi) {
    g_part_lists++;
    const int32_t a[4] = {(int32_t)(a_lo & 0xffff) - 1, (int32_t)(a_lo >> 16) - 1, (int32_t)(a_hi & 0xffff) - 1, (int32_t)(a_hi >> 16) - 1};
    const int32_t b[4] = {(int32_t)(b_lo & 0xffff) - 1, (int32_t)(b_lo >> 16) - 1, (int32_t)(b_hi & 0xffff) - 1, (int32_t)(b_hi >> 16) - 1};
    int at = 0;
    for (int i = 0; i < 4; i++) {
        if (a[i] < 0) return 1;
        if (at >= 4) return 0;
        int k = at;
        for (;; k++) {
            if (k >= 4 || b[k] < 0 || b[k] > a[i]) return 0;
            if (b[k] == a[i]) break;
        }
        at = k + 1;
    }
    return 1;
}
static const uint8_t math_split_9f8680_original[] = {0x83, 0xec, 0x20, 0x8b, 0x44, 0x24, 0x24, 0x8b, 0xc8};
REWRITE("Math::Split9F8680", 0x9f8680, math_split_9f8680_original, &math_split_9f8680, &g_part_lists);

// ---- Water::Entry94EF60(this, n) (0x94EF60): water entry n taken out of its band's list and its slot freed -------------
// Entries are 0xD0 bytes at +0x1A4. For one of kind 4 (+0x60): its band by its count (+4: below 0xB, below 0xE, else)
// picks a list {records (0x80 bytes), count, -, a, b} at this + 0x1C (band + 12); each record whose +0x58 matches the
// entry's +0xA4 or whose +0x5C matches its +0xA8 is replaced by the last (Record::CopyFields, 0x94CCD0), the count going
// down (and a by 4, b by 6), and looked at again. Then, when the entry is in use (byte +0), it is marked free and n goes
// back on the free list (+0x1A8, top +0x1B0 less +0x1AC, which goes down).
static volatile long g_water_entries;
static void __thiscall water_entry_94ef60(uint8_t *self, int32_t n) {
    g_water_entries++;
    const uint8_t *const e = *(const uint8_t *const *)(self + 0x1a4) + 0xd0 * n;
    if (*(const int32_t *)(e + 0x60) == 4) {
        const int32_t c = *(const int32_t *)(e + 4);
        const int32_t band = c >= 0xe ? 2 : c >= 0xb ? 1 : 0;
        volatile int32_t *const list = (volatile int32_t *)(self + 0x1c * (band + 12));
        for (int32_t i = 0; i < list[1]; i++) {
            uint8_t *const records = (uint8_t *)(uintptr_t)list[0];
            uint8_t *const r = records + 0x80 * i;
            if (*(const uint32_t *)(r + 0x58) != *(const uint32_t *)(e + 0xa4) && *(const uint32_t *)(r + 0x5c) != *(const uint32_t *)(e + 0xa8)) continue;
            list[3] = list[3] - 4, list[4] = list[4] - 6;
            ((void(__thiscall *)(uint8_t *, const uint8_t *))0x94ccd0)(r, records + 0x80 * list[1] - 0x80);
            list[1] = list[1] - 1;
            i--;
        }
    }
    volatile uint8_t *const used = *(uint8_t *const *)(self + 0x1a4) + 0xd0 * n;
    if (*used) {
        *used = 0;
        volatile int32_t *const t = (volatile int32_t *)self;
        ((volatile int32_t *)(uintptr_t)t[0x1a8 / 4])[t[0x1b0 / 4] - t[0x1ac / 4]] = n;
        t[0x1ac / 4] = t[0x1ac / 4] - 1;
    }
}
static const uint8_t water_entry_94ef60_original[] = {0x83, 0xec, 0x08, 0x8b, 0x44, 0x24, 0x0c, 0x69, 0xc0, 0xd0};
REWRITE("Water::Entry94EF60", 0x94ef60, water_entry_94ef60_original, &water_entry_94ef60, &g_water_entries);

// ---- Grid::CellAt(this, x, z, column out, row out) (0x8BCC90): the grid cell (bytes) a position is in, kept inside --------
// Each part (x - origin) / size (origin +0xC/+0x10, size +0x14/+0x18), as a float, rounded down (the game's floor); below
// 0 it is 0, above the last cell (width +4 byte - 1, height +5 byte - 1) it is that; each written as the low byte of its
// fistp. Gives the column's place (left in eax).
static volatile long g_cells_at;
static uint8_t *__thiscall grid_cell_at(const uint8_t *self, float x, float z, uint8_t *column, uint8_t *row) {
    g_cells_at++;
    auto f = [&](uint32_t at) { return *(const float *)(self + at); };
    volatile float cx = (float)(((long double)x - f(0xc)) / f(0x14));
    volatile float fx = (float)floor_h2(cx);
    volatile float cz = (float)(((long double)z - f(0x10)) / f(0x18));
    volatile float fz = (float)floor_h2(cz);
    volatile float col = fx;
    const float zero = *(const float *)0xf264e0;
    if (fx < zero) {
        *(volatile uint32_t *)&col = 0;
    } else {
        const long double w = (long double)(int32_t)(self[4] - 1);
        if ((long double)fx > w) col = (float)w;
    }
    long double rw;
    if (fz < zero) {
        rw = zero;
    } else {
        volatile float h = (float)(int32_t)(self[5] - 1);
        rw = fz > h ? (long double)h : (long double)fz;
    }
    *(volatile uint8_t *)column = (uint8_t)x87_fistp64_h2(col);
    *(volatile uint8_t *)row = (uint8_t)x87_fistp64_h2(rw);
    return column;
}
static const uint8_t grid_cell_at_original[] = {0x83, 0xec, 0x0c, 0xd9, 0x44, 0x24, 0x10, 0x8b, 0x41, 0x10};
REWRITE("Grid::CellAt", 0x8bcc90, grid_cell_at_original, &grid_cell_at, &g_cells_at);

// ---- Matrix::NearestAxis(v, m) (0x8A08E0): which axis of a matrix (rows 16 bytes) a vector lines up with best ----------
// fastcall. d0 = v . row 0 (on the FPU), d1 = (v . row 1) / 2 and d2 = v . row 2 (floats), their sizes as floats. The
// first not smaller than both others wins (ties to the earlier): 3, 4 or 5 when its d is not below 0, else 0, 1 or 2
// (a NaN counts as below).
static volatile long g_nearest_axes;
static uint32_t __fastcall matrix_nearest_axis(const float *v, const float *m) {
    g_nearest_axes++;
    const long double d0 = ((long double)m[2] * v[2] + (long double)m[1] * v[1]) + (long double)v[0] * m[0];
    volatile float d2 = (float)(((long double)m[8] * v[0] + (long double)m[10] * v[2]) + (long double)m[9] * v[1]);
    volatile float d1 = (float)((((long double)m[4] * v[0] + (long double)m[6] * v[2]) + (long double)m[5] * v[1]) * *(const float *)0xf2651c);
    volatile float a0 = (float)(d0 < 0 ? -d0 : d0);
    const float g1 = d1, g2 = d2;
    volatile float a1 = g1 < 0 ? -g1 : g1, a2 = g2 < 0 ? -g2 : g2;
    const float zero = *(const float *)0xf264e0;
    auto below = [](long double a, float b) { return a < b || a != a || b != b; };
    if (!below(a0, a1) && !below(a0, a2)) return below(d0, zero) ? 0 : 3;
    if (!below(a1, a0) && !below(a1, a2)) return below(d1, zero) ? 1 : 4;
    return below(d2, zero) ? 2 : 5;
}
static const uint8_t matrix_nearest_axis_original[] = {0x83, 0xec, 0x14, 0xd9, 0x42, 0x08, 0xd8, 0x49, 0x08};
REWRITE("Matrix::NearestAxis", 0x8a08e0, matrix_nearest_axis_original, &matrix_nearest_axis, &g_nearest_axes);

// ---- Vector::OntoC5A0E0(out, p, v) (0xC5A0E0): v rescaled so a point's place along it wraps within a range ------------
// fastcall out and p. L = |v| ((x x + y y) + z z, a float); out = v when L is exactly 0. i = 1 / L (a float); t = (L -
// (p . v) i) + K (0xF285F0). When t is from 0 up to below R (0xF268DC) out = v; otherwise (below 0 or from R on) w =
// (L - floor(t K3) R) i (K3 0xF36EE4; the floor the game's, of a float) and out = v w (x and y by w as a float, z by w
// unrounded). Gives out.
static volatile long g_wraps_onto;
static float *__fastcall vector_onto_c5a0e0(float *out, const float *p, const float *v) {
    g_wraps_onto++;
    volatile float l = (float)x87_sqrt_h2(((long double)v[0] * v[0] + (long double)v[1] * v[1]) + (long double)v[2] * v[2]);
    volatile uint32_t *const oi = (volatile uint32_t *)out;
    const uint32_t *const vi = (const uint32_t *)v;
    if (l == *(const float *)0xf264e0) {
        oi[0] = vi[0], oi[1] = vi[1], oi[2] = vi[2];
        return out;
    }
    volatile float inv = (float)((long double)*(const float *)0xf264dc / l);
    const long double s = (((long double)p[2] * v[2] + (long double)p[1] * v[1]) + (long double)p[0] * v[0]) * inv;
    const long double t = ((long double)l - s) + *(const float *)0xf285f0;
    const float range = *(const float *)0xf268dc;
    if (!(t < *(const float *)0xf264e0) && (t < range || t != t)) {
        oi[0] = vi[0], oi[1] = vi[1], oi[2] = vi[2];
        return out;
    }
    volatile float u = (float)(t * *(const float *)0xf36ee4);
    volatile float fl = (float)floor_h2(u);
    const long double w = ((long double)l - (long double)fl * range) * inv;
    volatile float wf = (float)w;
    volatile float *const o = out;
    o[0] = (float)((long double)wf * v[0]);
    o[1] = (float)((long double)wf * v[1]);
    o[2] = (float)(w * v[2]);
    return out;
}
static const uint8_t vector_onto_c5a0e0_original[] = {0x83, 0xec, 0x0c, 0x56, 0x8b, 0x74, 0x24, 0x14, 0xd9, 0x46, 0x08};
REWRITE("Vector::OntoC5A0E0", 0xc5a0e0, vector_onto_c5a0e0_original, &vector_onto_c5a0e0, &g_wraps_onto);

// ---- Water::Cell907BB0(this, x, z) (0x907BB0): the water height at a place, blended from the four nearest samples ------
// The grid covers +8..+0x10 by +0xC..+0x14 with n (+0) samples a side (heights +0x40). The place scaled to the samples
// (floats) is rounded down; outside 0..n-2 the height is +0x1C + +0x18. Otherwise the four samples are blended by the
// parts past the whole (x first along each row, then the rows by z), times +0x2C plus +0x18; given on the FPU.
static volatile long g_water_heights;
static long double __thiscall water_cell_907bb0(const uint8_t *self, float x, float z) {
    g_water_heights++;
    auto f = [&](uint32_t at) { return *(const float *)(self + at); };
    const int32_t n = *(const int32_t *)self;
    const long double scale = (long double)(n - 1);
    volatile float sx = (float)(((long double)x - f(8)) / ((long double)f(0x10) - f(8)) * scale);
    volatile float sz = (float)(((long double)z - f(0xc)) / ((long double)f(0x14) - f(0xc)) * scale);
    volatile float fx = (float)floor_h2(sx), fz = (float)floor_h2(sz);
    const int32_t ix = (int32_t)x87_fistp64_h2(fx), iz = (int32_t)x87_fistp64_h2(fz);
    if (ix < 0 || ix >= n - 1 || iz < 0 || iz >= n - 1) return (long double)f(0x1c) + f(0x18);
    const float *const h = *(const float *const *)(self + 0x40);
    const int32_t i = n * iz + ix;
    const long double a = (long double)sx - fx, b = (long double)*(const float *)0xf264dc - a, c = (long double)sz - fz;
    const long double r0 = a * h[i + 1] + b * h[i];
    const long double p = r0 * ((long double)*(const float *)0xf264dc - c);
    const long double r1 = a * h[i + n + 1] + b * h[i + n];
    return (p + r1 * c) * f(0x2c) + f(0x18);
}
static const uint8_t water_cell_907bb0_original[] = {0x83, 0xec, 0x10, 0x8b, 0x01, 0x48, 0x89, 0x44, 0x24, 0x04};
REWRITE("Water::Cell907BB0", 0x907bb0, water_cell_907bb0_original, &water_cell_907bb0, &g_water_heights);

// ---- Vector::ConeCD9730 (0xCD9730): where a line meets a cone (or cylinder) around an axis -----------------------------
// a in eax, b in ecx, d in esi, out in edi; c and k on the stack. n = a x b the game's way (on the FPU); nc = n . c and
// q = |n|^2 k (floats). When nc nc is below q: out = 0 and 1. Otherwise with e = n . d (on the FPU): when nc e is not
// below 0, 0; else out = (sqrt(q) s - nc) / e (s 1, or K (0xF268F0) when nc is below 0) and 1 when out is below 1, else
// 0. (Only the low byte of the result is set on the first two ways out, as in the game.)
static volatile long g_cones;
extern "C" uint32_t __stdcall forever_cone(const float *a, const float *b, const float *c, float k, const float *d, float *out) {
    g_cones++;
    const long double x = (long double)a[2] * b[1] - (long double)b[2] * a[1];
    const long double y = (long double)b[2] * a[0] - (long double)b[0] * a[2];
    const long double z = (long double)b[0] * a[1] - (long double)a[0] * b[1];
    volatile float nc = (float)((z * c[2] + y * c[1]) + x * c[0]);
    volatile float q = (float)(((x * x + z * z) + y * y) * k);
    if ((long double)nc * nc < q) {
        *(volatile uint32_t *)out = 0;
        return 1;
    }
    const long double e = (z * d[2] + y * d[1]) + x * d[0];
    const long double ne = (long double)nc * e;
    if (!(ne < *(const float *)0xf264e0 || ne != ne)) return 0;
    const long double s = nc < *(const float *)0xf264e0 ? *(const float *)0xf268f0 : *(const float *)0xf264dc;
    const long double r = (x87_sqrt_h2(q) * s - nc) / e;
    volatile float *const o = out;
    *o = (float)r;
    return r < *(const float *)0xf264dc ? 1 : 0;
}
extern "C" __attribute__((naked)) void forever_cone_regs() {   // a eax, b ecx, d esi, out edi; c, k on the stack
    __asm__("pushl %edi\n\tpushl %esi\n\tpushl 16(%esp)\n\tpushl 16(%esp)\n\tpushl %ecx\n\tpushl %eax\n\tcalll _forever_cone@24\n\tret $8");
}
static const uint8_t vector_cone_cd9730_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x10, 0xd9, 0x40, 0x08};
REWRITE("Vector::ConeCD9730", 0xcd9730, vector_cone_cd9730_original, (void *)&forever_cone_regs, &g_cones);

// ---- Fence::DistanceA90D00(list, px, py, qx, qy, i) (0xA90D00): where a line crosses one edge of a closed outline ----
// The edge runs from point i of the list {points (x, y floats), count} to the next (wrapping). E = edge (x a float, y on
// the FPU), D = q - p (floats); den = Ex Dy - Dx Ey, 0 giving K (0xF268F0). s = ((px - x0) Dy - (py - y0) Dx) / den and
// t = ((y0 - py) Ex - (x0 - px) Ey) / den (floats, by den's inverse). t when s and t are both within 0..1, else K.
static volatile long g_edge_crossings;
static long double __stdcall fence_distance_a90d00(const uint32_t *list, float px, float py, float qx, float qy, int32_t i) {
    g_edge_crossings++;
    const float *const pts = (const float *)(uintptr_t)list[0];
    const int32_t count = (int32_t)list[1];
    const int32_t j = i + 1 < count ? i + 1 : i - count + 1;
    const long double x0 = pts[2 * i], y0 = pts[2 * i + 1];
    volatile float dy = (float)((long double)qy - py);
    volatile float ex = (float)((long double)pts[2 * j] - x0);
    const long double ey = (long double)pts[2 * j + 1] - y0;
    volatile float dx = (float)((long double)qx - px);
    const long double den = (long double)ex * dy - (long double)dx * ey;
    const float none = *(const float *)0xf268f0;
    if (den == *(const float *)0xf264e0) return none;
    const long double inv = (long double)*(const float *)0xf264dc / den;
    volatile float s = (float)((((long double)px - x0) * dy - ((long double)py - y0) * dx) * inv);
    volatile float t = (float)(((y0 - py) * ex - (x0 - px) * ey) * inv);
    const float zero = *(const float *)0xf264e0, one = *(const float *)0xf264dc;
    if (s < zero || s != s || s > one) return none;
    if (t < zero || t != t || t > one) return none;
    return t;
}
static const uint8_t fence_distance_a90d00_original[] = {0x51, 0x8b, 0x54, 0x24, 0x08, 0x8b, 0x44, 0x24, 0x1c, 0x8b, 0x0a};
REWRITE("Fence::DistanceA90D00", 0xa90d00, fence_distance_a90d00_original, &fence_distance_a90d00, &g_edge_crossings);

// ---- Person::SwimToward(target, step, stop) (0x7C2970) ----------------------------------------------------------------
// D = target - position (+0x80; floats, z kept unrounded for its square too), d = |D| (z^2 + y^2 + x^2). When d is above
// E (0xF3312C) the direction is D / d (floats), else the facing (+0x70). v = d - stop, kept within -step..step (a NaN
// kept); the position moves by direction v (x unrounded, y and z as floats). 1 when |v| is not below step (or NaN), else 0.
static volatile long g_swims;
static uint32_t __thiscall person_swim_toward(uint8_t *self, const float *target, float step, float stop) {
    g_swims++;
    volatile float *const pos = (volatile float *)(self + 0x80);
    const float px = pos[0], py = pos[1], pz = pos[2];
    volatile float dx = (float)((long double)target[0] - px);
    volatile float dy = (float)((long double)target[1] - py);
    const long double dzx = (long double)target[2] - pz;
    volatile float dz = (float)dzx;
    const long double d = x87_sqrt_h2((dzx * dz + (long double)dy * dy) + (long double)dx * dx);
    if (d > *(const float *)0xf3312c) {
        const long double inv = (long double)*(const float *)0xf264dc / d;
        dx = (float)(dx * inv);
        dy = (float)(dy * inv);
        dz = (float)(dz * inv);
    } else {
        const volatile float *const facing = (const volatile float *)(self + 0x70);
        dx = facing[0], dy = facing[1], dz = facing[2];
    }
    long double v = d - stop;
    if (v > step) v = step;
    else {
        volatile float neg = (float)-(long double)step;
        if (v < neg) v = neg;
    }
    const long double mx = dx * v;
    volatile float my = (float)(dy * v);
    volatile float mz = (float)(dz * v);
    pos[2] = (float)((long double)mz + pz);
    pos[0] = (float)(mx + px);
    pos[1] = (float)((long double)my + py);
    const long double a = __builtin_fabsl(v);
    return a < step || a != a ? 0 : 1;
}
static const uint8_t person_swim_toward_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x40, 0x8b, 0x81, 0x80, 0x00, 0x00, 0x00};
REWRITE("Person::SwimToward", 0x7c2970, person_swim_toward_original, &person_swim_toward, &g_swims);

// ---- Curves::StepC213B0(point, n, w) (0xC213B0): a running blend of points ------------------------------------------
// Self: newest point +0x10, older +0x20, weight +0x30, count +0x34. When n is past the count: the older point becomes
// the newest blended in by the weight (each part w a + (1 - w) b, z keeping w a unrounded; a plain copy when the weight is
// not below 1), then the point and w become the newest and the weight. Otherwise, when w is above 0: the weight grows by
// w, r = w / weight (unrounded), and the newest becomes r point + (1 - r) newest (z keeping (1 - r) newest unrounded).
// The count becomes n either way.
static volatile long g_point_blends;
static void __thiscall curves_step_c213b0(uint8_t *self, const uint32_t *point, int32_t n, float w) {
    g_point_blends++;
    volatile float *const f = (volatile float *)self;   // f[4..6] newest, f[8..10] older, f[12] weight
    const float one = *(const float *)0xf264dc;
    if (n > *(int32_t *)(self + 0x34)) {
        if (f[12] < one || f[12] != f[12]) {
            const long double k = f[12];
            volatile float a = (float)(k * f[4]);
            volatile float b = (float)(k * f[5]);
            const long double c = k * f[6];
            const long double om = (long double)one - f[12];
            volatile float e = (float)(om * f[8]);
            volatile float g = (float)(om * f[9]);
            volatile float h = (float)(om * f[10]);
            const long double x = (long double)e + a, y = (long double)g + b;
            volatile float z = (float)((long double)h + c);
            f[8] = (float)x;
            f[9] = (float)y;
            f[10] = z;
        } else {
            uint32_t *const u = (uint32_t *)self;
            u[8] = u[4], u[9] = u[5], u[10] = u[6];
        }
        *(volatile float *)(self + 0x30) = w;
        uint32_t *const u = (uint32_t *)self;
        u[4] = point[0], u[5] = point[1], u[6] = point[2];
    } else if (w > *(const float *)0xf264e0) {
        const float *const p = (const float *)point;
        const long double s = (long double)w + f[12];
        f[12] = (float)s;
        const long double r = w / s;
        volatile float a = (float)(r * p[0]);
        volatile float b = (float)(r * p[1]);
        volatile float c = (float)(r * p[2]);
        const long double om = (long double)one - r;
        volatile float e = (float)(om * f[4]);
        volatile float g = (float)(om * f[5]);
        const long double h = om * f[6];
        const long double x = (long double)e + a, y = (long double)g + b;
        volatile float z = (float)(h + c);
        f[4] = (float)x;
        f[5] = (float)y;
        f[6] = z;
    }
    *(int32_t *)(self + 0x34) = n;
}
static const uint8_t curves_step_c213b0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x2c, 0x8b, 0x55, 0x0c, 0x3b, 0x51, 0x34};
REWRITE("Curves::StepC213B0", 0xc213b0, curves_step_c213b0_original, &curves_step_c213b0, &g_point_blends);

// ---- Effect::Length414630 (0x414630): each triangle's normal, made of length 1 ---------------------------------------
// Entries of 0x20 bytes (+0x94, count +0x98, read again each time): points a, b, c (pointers) and the normal at +0x10.
// U = b - a (x unrounded), V = c - a (floats); the normal is V x U the game's way, stored, then each part negated, then
// divided by its length (z^2 + x^2 + y^2); a length of 0 sets z to 1 and leaves x and y.
static volatile long g_effect_normals;
static void __thiscall effect_length_414630(uint8_t *self) {
    g_effect_normals++;
    for (int32_t i = 0; i < *(volatile int32_t *)(self + 0x98); i++) {
        uint8_t *const e = *(uint8_t *const volatile *)(self + 0x94) + i * 0x20;
        const float *const a = *(const float *const *)e, *const b = *(const float *const *)(e + 4), *const c = *(const float *const *)(e + 8);
        const long double ux = (long double)b[0] - a[0];
        volatile float uy = (float)((long double)b[1] - a[1]);
        volatile float uz = (float)((long double)b[2] - a[2]);
        volatile float vx = (float)((long double)c[0] - a[0]);
        volatile float vy = (float)((long double)c[1] - a[1]);
        volatile float vz = (float)((long double)c[2] - a[2]);
        const long double nx = (long double)vy * uz - (long double)vz * uy;
        const long double ny = (long double)vz * ux - (long double)uz * vx;
        const long double nz = (long double)uy * vx - (long double)vy * ux;
        volatile float *const n = (volatile float *)(e + 0x10);
        n[2] = (float)nz;
        n[0] = (float)nx;
        n[1] = (float)ny;
        for (int k = 0; k < 3; k++) n[k] = (float)-(long double)n[k];
        const long double z = n[2], y = n[1], x = n[0];
        const long double len = x87_sqrt_h2((z * z + x * x) + y * y);
        if (len == *(const float *)0xf264e0) {
            *(volatile uint32_t *)(e + 0x18) = 0x3f800000;
            continue;
        }
        const long double inv = (long double)*(const float *)0xf264dc / len;
        n[0] = (float)(inv * n[0]);
        n[1] = (float)(inv * n[1]);
        n[2] = (float)(inv * n[2]);
    }
}
static const uint8_t effect_length_414630_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x34, 0x8b, 0x81, 0x98, 0x00, 0x00, 0x00};
REWRITE("Effect::Length414630", 0x414630, effect_length_414630_original, &effect_length_414630, &g_effect_normals);

// ---- Effect::Pass414320(unused, p, k) (0x414320): pushes points nearer p than k out to k -------------------------------
// Entries of 0x50 bytes (+0x14, count +0x20, read again each time), a point first. D = point - p (x and z unrounded, y a
// float), L = |D| (z^2 + y^2 + x^2). The direction is D / L (x and y floats, z unrounded), or (x, y, 1) when L is 0.
// When L (as a float) is below k the point becomes p + k direction (z unrounded).
static volatile long g_effect_pushes;
static void __thiscall effect_pass_414320(uint8_t *self, uint32_t, const float *p, float k) {
    g_effect_pushes++;
    for (int32_t i = 0; i < *(volatile int32_t *)(self + 0x20); i++) {
        volatile float *const e = (volatile float *)(*(uint8_t *const volatile *)(self + 0x14) + i * 0x50);
        const long double dx = (long double)e[0] - p[0];
        volatile float dy = (float)((long double)e[1] - p[1]);
        const long double dz = (long double)e[2] - p[2];
        volatile float x = (float)dx, y = dy;
        const long double len = x87_sqrt_h2((dz * dz + (long double)dy * dy) + dx * dx);
        volatile float lf = (float)len;
        long double z;
        if (len == *(const float *)0xf264e0) z = *(const float *)0xf264dc;
        else {
            const long double inv = (long double)*(const float *)0xf264dc / len;
            x = (float)(inv * dx);
            y = (float)(dy * inv);
            z = dz * inv;
        }
        if (!(lf < k)) continue;
        volatile float xk = (float)(x * (long double)k);
        volatile float yk = (float)(y * (long double)k);
        const long double zk = z * k;
        e[2] = (float)(zk + p[2]);
        e[0] = (float)((long double)xk + p[0]);
        e[1] = (float)((long double)yk + p[1]);
    }
}
static const uint8_t effect_pass_414320_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x44, 0x8b, 0x41, 0x20, 0x53, 0x56, 0x33, 0xdb};
REWRITE("Effect::Pass414320", 0x414320, effect_pass_414320_original, &effect_pass_414320, &g_effect_pushes);

// ---- Fence::NearA90E70(out, list, px, py, i) (0xA90E70): the nearest point to p on one edge of an outline -------------
// The edge from point i of the list {points (x, y floats), count} to the next (wrapping). A = x0 - x1, B = y0 - y1
// (unrounded). When A^2 + B^2 is not above E (0xF2CF34) out is point i. Otherwise t = ((y0 - py) B + (x0 - px) A) /
// (B^2 + A^2) (B as a float, t rounded to one): above 1 out is point i + 1, below 0 point i, else (with a NaN too) out is
// x1 t + x0 (1 - t), y1 t + y0 (1 - t) (the x product and both t products floats). Gives out.
static volatile long g_fence_nears;
static float *__stdcall fence_near_a90e70(float *out, const uint32_t *list, float px, float py, int32_t i) {
    g_fence_nears++;
    const float *const pts = (const float *)(uintptr_t)list[0];
    const int32_t count = (int32_t)list[1];
    const int32_t j = i + 1 < count ? i + 1 : i - count + 1;
    const float x0 = pts[2 * i], y0 = pts[2 * i + 1], x1 = pts[2 * j], y1 = pts[2 * j + 1];
    const long double a = (long double)x0 - x1;
    const long double b = (long double)y0 - y1;
    volatile float bf = (float)b;
    if (!(a * a + b * b > *(const float *)0xf2cf34)) {
        volatile float *const o = out;
        o[0] = x0, o[1] = y0;
        return out;
    }
    const long double num = ((long double)y0 - py) * bf + ((long double)x0 - px) * a;
    volatile float t = (float)(num / ((long double)bf * bf + a * a));
    volatile float *const o = out;
    if (t > *(const float *)0xf264dc) {
        o[0] = x1, o[1] = y1;
        return out;
    }
    if (t < *(const float *)0xf264e0) {
        o[0] = x0, o[1] = y0;
        return out;
    }
    volatile float tx = (float)((long double)x1 * t);
    volatile float ty = (float)((long double)y1 * t);
    const long double om = (long double)*(const float *)0xf264dc - t;
    volatile float sx = (float)(x0 * om);
    const long double sy = y0 * om;
    o[0] = (float)((long double)sx + tx);
    o[1] = (float)(sy + ty);
    return out;
}
static const uint8_t fence_near_a90e70_original[] = {0x83, 0xec, 0x10, 0x8b, 0x54, 0x24, 0x18, 0x8b, 0x0a, 0x8b, 0x44, 0x24, 0x24};
REWRITE("Fence::NearA90E70", 0xa90e70, fence_near_a90e70_original, &fence_near_a90e70, &g_fence_nears);

// ---- Terrain::Near8F2380(p, r0, r1) (0x8F2380): a fade by distance into each grid entry's top byte --------------------
// (+8 + +0xC) x 2 entries of 0x10 bytes (+0x14; x at +0, z at +8) and as many words (+0x1C). d = (x - px)^2 + (z - pz)^2
// (a float). Within r0^2 (a float) the top byte is 0xFF; from r1^2 (a float) on (or NaN) 0; between, 0xFF less
// floor((d - r0^2) 255 / (r1^2 - r0^2) + 0.5) (the scale a float, r1^2 unrounded under it).
static volatile long g_terrain_fades;
static void __thiscall terrain_near_8f2380(const uint8_t *self, const float *p, float r0, float r1) {
    g_terrain_fades++;
    volatile float in2 = (float)((long double)r0 * r0);
    const long double out2x = (long double)r1 * r1;
    volatile float out2 = (float)out2x;
    volatile float scale = (float)(*(const float *)0xf264c0 / (out2x - in2));
    const int32_t n = (*(const int32_t *)(self + 0xc) + *(const int32_t *)(self + 8)) * 2;
    if (n <= 0) return;
    const float *e = *(const float *const *)(self + 0x14);
    uint32_t *w = *(uint32_t *const *)(self + 0x1c);
    for (int32_t k = 0; k < n; k++, e += 4, w++) {
        const long double dx = (long double)e[0] - p[0];
        const long double dz = (long double)e[2] - p[1];
        volatile float d = (float)(dz * dz + dx * dx);
        uint32_t v;
        if (d <= in2) v = 0xff;
        else if (!(d < out2 || d != d)) v = 0;
        else {
            volatile float u = (float)(((long double)d - in2) * scale + *(const float *)0xf2651c);
            volatile float fl = (float)floor_h2(u);
            v = 0xff - (uint32_t)x87_fistp64_h2(fl);
        }
        *w = (*w & 0xffffff) | v << 24;
    }
}
static const uint8_t terrain_near_8f2380_original[] = {0x83, 0xec, 0x10, 0xd9, 0x44, 0x24, 0x18, 0x8b, 0x41, 0x0c, 0xd8, 0x4c, 0x24, 0x18};
REWRITE("Terrain::Near8F2380", 0x8f2380, terrain_near_8f2380_original, &terrain_near_8f2380, &g_terrain_fades);

// ---- Vector::NearCDB2A0 (0xCDB2A0): where a ray from a first meets a sphere ---------------------------------------
// The start a in ecx, the direction d in edx; the centre c, the radius r and out on the stack. P = a - c (x a float, y
// and z unrounded); b = 2 (P . d) (z, y, x order), disc = b b' - 4 ((z^2 + x^2 + y^2) - r^2) (b' = b as a float). With
// disc below 0 (or NaN) 0. Else out = (-b' - sqrt disc) / 2, and 1 when that is not below 0; else out = (sqrt disc -
// b') / 2 and 1 when that is not below 0, else 0. (Only the low byte is set on the first two ways out, as in the game.)
static volatile long g_sphere_hits;
extern "C" uint32_t __stdcall forever_sphere_hit(const float *a, const float *d, const float *c, float r, float *out) {
    g_sphere_hits++;
    volatile float px = (float)((long double)a[0] - c[0]);
    const long double py = (long double)a[1] - c[1];
    const long double pz = (long double)a[2] - c[2];
    const long double dot = (pz * d[2] + py * d[1]) + (long double)px * d[0];
    const long double b = dot + dot;
    volatile float bf = (float)b;
    const long double s2 = (pz * pz + (long double)px * px) + py * py;
    const long double disc = b * bf - (s2 - (long double)r * r) * *(const float *)0xf2669c;
    if (disc < *(const float *)0xf264e0 || disc != disc) return 0;
    const long double s = x87_sqrt_h2(disc);
    const long double nb = -(long double)bf;
    const float half = *(const float *)0xf2651c;
    volatile float *const o = out;
    const long double t1 = (nb - s) * half;
    *o = (float)t1;
    if (!(t1 < *(const float *)0xf264e0 || t1 != t1)) return 1;
    const long double t2 = (s + nb) * half;
    *o = (float)t2;
    return t2 < *(const float *)0xf264e0 || t2 != t2 ? 0 : 1;
}
extern "C" __attribute__((naked)) void forever_sphere_hit_regs() {   // a ecx, d edx; c, r, out on the stack
    __asm__("pushl 12(%esp)\n\tpushl 12(%esp)\n\tpushl 12(%esp)\n\tpushl %edx\n\tpushl %ecx\n\tcalll _forever_sphere_hit@20\n\tret $12");
}
static const uint8_t vector_near_cdb2a0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x20, 0x8b, 0x41, 0x08, 0xd9, 0x01};
REWRITE("Vector::NearCDB2A0", 0xcdb2a0, vector_near_cdb2a0_original, (void *)&forever_sphere_hit_regs, &g_sphere_hits);

// ---- Animal::DirectionA60E40 (0xA60E40): a step from one point toward another, by a square-root rule -----------------
// out in ecx, to in edx; from, a and k on the stack. D = to - from (floats; z kept unrounded for its square), L = |D|
// (z^2 + x^2 + y^2), also kept as a float L'. The step m = sqrt(L') a k; when L is above 0, m is below L' and q = m / L'
// is below Q (0xF37CF8), out = from + q D (the x product unrounded, y and z products floats).
// Otherwise out is to (copied).
static volatile long g_animal_steps;
extern "C" void __stdcall forever_animal_direction(float *out, const float *to, const float *from, float a, float k) {
    g_animal_steps++;
    volatile float dx = (float)((long double)to[0] - from[0]);
    volatile float dy = (float)((long double)to[1] - from[1]);
    const long double dzx = (long double)to[2] - from[2];
    volatile float dz = (float)dzx;
    const long double len = x87_sqrt_h2((dzx * dz + (long double)dx * dx) + (long double)dy * dy);
    volatile float lf = (float)len;
    if (len > *(const float *)0xf264e0) {
        const long double m = x87_sqrt_h2(lf) * a * k;
        if (m < lf) {
            const long double q = m / lf;
            if (q < *(const float *)0xf37cf8) {
                const long double mx = dx * q;
                volatile float my = (float)(dy * q);
                volatile float mz = (float)(dz * q);
                volatile float *const o = out;
                o[2] = (float)((long double)mz + from[2]);
                o[0] = (float)(mx + from[0]);
                o[1] = (float)((long double)my + from[1]);
                return;
            }
        }
    }
    uint32_t *const o = (uint32_t *)out;
    const uint32_t *const t = (const uint32_t *)to;
    o[0] = t[0], o[1] = t[1], o[2] = t[2];
}
extern "C" __attribute__((naked)) void forever_animal_direction_regs() {   // out ecx, to edx; from, a, k on the stack
    __asm__("pushl 12(%esp)\n\tpushl 12(%esp)\n\tpushl 12(%esp)\n\tpushl %edx\n\tpushl %ecx\n\tcalll _forever_animal_direction@20\n\tret $12");
}
static const uint8_t animal_direction_a60e40_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x2c, 0xd9, 0x02, 0x56};
REWRITE("Animal::DirectionA60E40", 0xa60e40, animal_direction_a60e40_original, (void *)&forever_animal_direction_regs, &g_animal_steps);

// ---- Effect::Normalise7B3B20 (0x7B3B20): scales a centre by one over a weight, and a spread from it -----------------
// When the weight (+0x44) is above 0: i = 1 / weight; the centre (+0x10) is scaled by i (x by i unrounded, y and z by i
// as a float). For each of the points (+0x50, 0x10 bytes each, count +0x4C read again each time), with A = point - base
// (+0) and B = point - centre (unrounded but B's z, a float), +0x48 += (B.z^2 + B.y^2 + B.x^2) / (A.z^2 + A.x^2 + A.y^2).
// Then +0x48 = sqrt(i +0x48).
static volatile long g_effect_spreads;
static void __thiscall effect_normalise_7b3b20(uint8_t *self) {
    g_effect_spreads++;
    volatile float *const f = (volatile float *)self;
    if (!(f[0x44 / 4] > *(const float *)0xf264e0)) return;
    const long double inv = (long double)*(const float *)0xf264dc / f[0x44 / 4];
    volatile float invf = (float)inv;
    f[4] = (float)(inv * f[4]);
    f[5] = (float)((long double)invf * f[5]);
    f[6] = (float)((long double)invf * f[6]);
    for (int32_t i = 0; i < *(volatile int32_t *)(self + 0x4c); i++) {
        const float *const p = *(const float *const *)(self + 0x50) + i * 4;
        const long double ax = (long double)p[0] - f[0], ay = (long double)p[1] - f[1], az = (long double)p[2] - f[2];
        const long double bx = (long double)p[0] - f[4], by = (long double)p[1] - f[5];
        volatile float bz = (float)((long double)p[2] - f[6]);
        const long double w = (long double)*(const float *)0xf264dc / ((az * az + ax * ax) + ay * ay);
        const long double t = ((long double)bz * bz + by * by) + bx * bx;
        f[0x48 / 4] = (float)(w * t + f[0x48 / 4]);
    }
    f[0x48 / 4] = (float)x87_sqrt_h2((long double)invf * f[0x48 / 4]);
}
static const uint8_t effect_normalise_7b3b20_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x1c, 0xd9, 0x41, 0x44, 0x56};
REWRITE("Effect::Normalise7B3B20", 0x7b3b20, effect_normalise_7b3b20_original, &effect_normalise_7b3b20, &g_effect_spreads);

// ---- Grid::SnapToRow(out, p) (0x99C2A0) ----------------------------------------------------------------------------
// H = the row height (0x13413B0). y = floor(p.y / H) H (the quotient a float, the product as a float y'): below 0 it is
// 0; above the last row ((+0x108 - 1) H, unrounded) it is that; else y'. out = {p.x but at least M (0xF2E17C), y}.
static volatile long g_row_snaps;
static float *__thiscall grid_snap_to_row(const uint8_t *self, float *out, const float *p) {
    g_row_snaps++;
    const volatile float *const h = (const volatile float *)0x13413b0;
    volatile float q = (float)((long double)p[1] / *h);
    volatile float fl = (float)floor_h2(q);
    const int32_t i = (int32_t)x87_fistp64_h2(fl);
    volatile float yf = (float)((long double)i * *h);
    long double y;
    if (yf < *(const float *)0xf264e0) y = *(const float *)0xf264e0;
    else {
        const long double top = (long double)(*(const int32_t *)(self + 0x108) - 1) * *h;
        y = yf > top ? top : (long double)yf;
    }
    volatile float *const o = out;
    const float m = *(const float *)0xf2e17c;
    o[0] = p[0] < m ? m : p[0];
    o[1] = (float)y;
    return out;
}
static const uint8_t grid_snap_to_row_original[] = {0x83, 0xec, 0x08, 0x8b, 0x54, 0x24, 0x10, 0xd9, 0x42, 0x04, 0xd8, 0x35, 0xb0, 0x13, 0x34, 0x01};
REWRITE("Grid::SnapToRow", 0x99c2a0, grid_snap_to_row_original, &grid_snap_to_row, &g_row_snaps);

// ---- Segment::MiddleOfEnds(out) (0xBC61E0) -------------------------------------------------------------------------
// The lists (+0x10C, +0x110: {count, points of 0x24 bytes}) are only there with flag 0x400000 (+0x18) and at least two
// ends (+0xE4): the game reads through 0 otherwise. out = (last of +0x10C + last of +0x110) / 2, x's sum as a float.
static volatile long g_segment_middles;
static float *__thiscall segment_middle_of_ends(const uint8_t *self, float *out) {
    g_segment_middles++;
    const bool there = (*(const uint32_t *)(self + 0x18) & 0x400000) && *(const int32_t *)(self + 0xe4) >= 2;
    const uint32_t *const volatile l2 = there ? *(const uint32_t *const *)(self + 0x110) : nullptr;
    const uint32_t *const volatile l1 = there ? *(const uint32_t *const *)(self + 0x10c) : nullptr;
    const float *const b = (const float *)((const uint8_t *)(uintptr_t)l2[1] + l2[0] * 0x24 - 0x24);
    const float *const a = (const float *)((const uint8_t *)(uintptr_t)l1[1] + l1[0] * 0x24 - 0x24);
    volatile float sx = (float)((long double)a[0] + b[0]);
    const long double sy = (long double)a[1] + b[1];
    const long double sz = (long double)a[2] + b[2];
    const float half = *(const float *)0xf2651c;
    volatile float *const o = out;
    o[0] = (float)((long double)sx * half);
    o[1] = (float)(sy * half);
    o[2] = (float)(sz * half);
    return out;
}
static const uint8_t segment_middle_of_ends_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x3c, 0x56, 0x8b, 0x71, 0x18};
REWRITE("Segment::MiddleOfEnds", 0xbc61e0, segment_middle_of_ends_original, &segment_middle_of_ends, &g_segment_middles);

// ---- Mover::StepBack(dt) (0x913620) --------------------------------------------------------------------------------
// time (+0x58) -= dt; r = time / T (0x10F0B88; the time unrounded under it) is kept at +0x50. The node +0x60 of the list
// at 0x133A0F4 ({.., nodes +0xC, count +0x14}, 0x98 bytes each): when it is out of range (or its place is 0) bit 1 of
// +0 is set. Otherwise x (+4) = node.x - r dx (+0x64; unrounded) and y (+8) = node.y - r dy (+0x68; a float), y going
// back to what it was when that was below node.y.
static volatile long g_mover_steps;
static void __thiscall mover_step_back(uint8_t *self, float dt) {
    g_mover_steps++;
    volatile float *const f = (volatile float *)self;
    const long double t = (long double)f[0x58 / 4] - dt;
    f[0x58 / 4] = (float)t;
    volatile float r = (float)(t / *(const volatile float *)0x10f0b88);
    f[0x50 / 4] = r;
    const uint8_t *const nodes = *(const uint8_t *const volatile *)0x133a0f4;
    const int32_t i = *(const int32_t *)(self + 0x60);
    const float *const node = i < *(const int32_t *)(nodes + 0x14) && i >= 0 ? (const float *)(*(const uint8_t *const *)(nodes + 0xc) + i * 0x98) : nullptr;
    if (!node) {
        *(volatile uint32_t *)self |= 1;
        return;
    }
    const float y = f[2];
    const long double a = (long double)r * f[0x64 / 4];
    volatile float b = (float)((long double)r * f[0x68 / 4]);
    const float ny = node[2];
    f[1] = (float)((long double)node[1] - a);
    f[2] = (float)((long double)ny - b);
    if (y < node[2]) f[2] = y;
}
static const uint8_t mover_step_back_original[] = {0x83, 0xec, 0x18, 0xd9, 0x41, 0x58, 0x56, 0xd8, 0x64, 0x24, 0x20};
REWRITE("Mover::StepBack", 0x913620, mover_step_back_original, &mover_step_back, &g_mover_steps);

// ---- Window::ScreenPosition(out) (0x6AD560) ------------------------------------------------------------------------
// The window's position (+0x7C) plus, up the parents (+0x3C), each parent's position, and before it the parent's scroll
// (+0x8C) when the child below it has flag 0x1000 (+0x94). Each sum a float. Gives out.
static volatile long g_screen_positions;
static float *__thiscall window_screen_position(const uint8_t *self, float *out) {
    g_screen_positions++;
    auto at = [](const uint8_t *w, uint32_t o) { return *(const float *)(w + o); };
    volatile float x = at(self, 0x7c), y = at(self, 0x80);
    const uint8_t *child = self;
    for (const uint8_t *p = *(const uint8_t *const *)(self + 0x3c); p; child = p, p = *(const uint8_t *const *)(p + 0x3c)) {
        if (*(const uint32_t *)(child + 0x94) >> 12 & 1) {
            x = (float)((long double)x + at(p, 0x8c));
            y = (float)((long double)y + at(p, 0x90));
        }
        x = (float)((long double)at(p, 0x7c) + x);
        y = (float)((long double)y + at(p, 0x80));
    }
    volatile float *const o = out;
    o[0] = x, o[1] = y;
    return out;
}
static const uint8_t window_screen_position_original[] = {0x83, 0xec, 0x18, 0x8b, 0x41, 0x7c, 0x8b, 0x91, 0x80, 0x00, 0x00, 0x00};
REWRITE("Window::ScreenPosition", 0x6ad560, window_screen_position_original, &window_screen_position, &g_screen_positions);

// ---- Fog::Setup(object, distance) (0x7C3D50): the global fog values -------------------------------------------------
// 0x1307EE4 = 1 / distance when that is above E (0xF3312C), else -1. 0x10EFC80 = -1 and the colour (0x1307EF0..F8) 0;
// then from the object (ecx, when there): its single value (+0x198) into 0x10EFC80 when +0x1A6 is set, else its colour
// (+0x160..+0x168).
static volatile long g_fog_setups;
static void __thiscall fog_setup(const uint8_t *object, float distance) {
    g_fog_setups++;
    volatile uint32_t *const g = (volatile uint32_t *)0x1307ee4;
    if (distance > *(const float *)0xf3312c) *(volatile float *)g = (float)((long double)*(const float *)0xf264dc / distance);
    else *g = 0xbf800000;
    volatile uint32_t *const single = (volatile uint32_t *)0x10efc80, *const colour = (volatile uint32_t *)0x1307ef0;
    *single = 0xbf800000;
    colour[0] = 0, colour[1] = 0, colour[2] = 0;
    if (!object) return;
    const uint32_t *const o = (const uint32_t *)object;
    if (object[0x1a6]) *single = o[0x198 / 4];
    else colour[0] = o[0x160 / 4], colour[1] = o[0x164 / 4], colour[2] = o[0x168 / 4];
}
static const uint8_t fog_setup_original[] = {0xd9, 0x44, 0x24, 0x04, 0xd8, 0x1d, 0x2c, 0x31, 0xf3, 0x00};
REWRITE("Fog::Setup", 0x7c3d50, fog_setup_original, &fog_setup, &g_fog_setups);

// ---- PathGrid::SetEdges / ClearEdges(x, y, bits) (0xB9A810, 0xB9A8C0) --------------------------------------------------
// Cells of 0x18 bytes (+0x6C), width +0x64 (read again each time), edge bits at +0x14 of each. The cell's bits are set
// (or cleared); then for bit 1 the cell to its left gets 0x40, for 4 the one to its right 0x10, for 8 the one above
// (y - 1) 0x20 and for 2 the one below 0x80 (in that order; only the low byte of bits counts).
static volatile long g_path_edges;
template <bool Set> static void __thiscall path_grid_edges(const uint8_t *self, int32_t x, int32_t y, uint32_t bits_in) {
    g_path_edges++;
    const uint8_t b = (uint8_t)bits_in;
    auto cell = [&](int32_t cx, int32_t cy) -> volatile uint8_t & {
        const uint32_t i = (uint32_t)*(const volatile int32_t *)(self + 0x64) * (uint32_t)cy + (uint32_t)cx;
        return *(volatile uint8_t *)(*(uint8_t *const volatile *)(self + 0x6c) + i * 0x18 + 0x14);
    };
    auto put = [](volatile uint8_t &c, uint8_t m) {
        if (Set) c = c | m;
        else c = c & (uint8_t)~m;
    };
    put(cell(x, y), b);
    if (b & 1) put(cell(x - 1, y), 0x40);
    if (b & 4) put(cell(x + 1, y), 0x10);
    if (b & 8) put(cell(x, y - 1), 0x20);
    if (b & 2) put(cell(x, y + 1), 0x80);
}
static const uint8_t path_grid_set_edges_original[] = {0x8b, 0x41, 0x64, 0x8b, 0x54, 0x24, 0x08, 0x0f, 0xaf, 0xc2, 0x53, 0x8a, 0x5c, 0x24, 0x10};
REWRITE("PathGrid::SetEdges", 0xb9a810, path_grid_set_edges_original, &path_grid_edges<true>, &g_path_edges);
static const uint8_t path_grid_clear_edges_original[] = {0x8b, 0x41, 0x64, 0x8b, 0x54, 0x24, 0x08, 0x0f, 0xaf, 0xc2, 0x53, 0x8a, 0x5c, 0x24, 0x10};
REWRITE("PathGrid::ClearEdges", 0xb9a8c0, path_grid_clear_edges_original, &path_grid_edges<false>, &g_path_edges);

// ---- KeyMap::Find(key) (0xC477D0) ----------------------------------------------------------------------------------
// The key is 12 dwords; its hash is them all xored, its bucket hash % +4 (unsigned). The bucket's entries run from
// buckets[b] to buckets[b + 1] (+0xC); entry i (0x30 bytes at +0x10) equal to the key gives +0x14 + 2 i, none 0.
static volatile long g_keymap_finds;
static uint16_t *__thiscall keymap_find(const uint8_t *self, const uint32_t *key) {
    g_keymap_finds++;
    uint32_t h = 0;
    for (int k = 0; k < 12; k++) h ^= key[k];
    const uint32_t *const bucket = *(const uint32_t *const *)(self + 0xc) + h % *(const uint32_t *)(self + 4);
    const uint32_t end = bucket[1];
    const uint32_t *const entries = *(const uint32_t *const *)(self + 0x10);
    for (uint32_t i = bucket[0]; i < end; i++) {
        const uint32_t *const e = entries + i * 12;
        int k = 0;
        while (k < 12 && e[k] == key[k]) k++;
        if (k == 12) return (uint16_t *)(*(uint8_t *const *)(self + 0x14) + i * 2);
    }
    return nullptr;
}
static const uint8_t keymap_find_original[] = {0x51, 0x53, 0x55, 0x56, 0x57, 0x89, 0x4c, 0x24, 0x10, 0x8b, 0x5c, 0x24, 0x18};
REWRITE("KeyMap::Find", 0xc477d0, keymap_find_original, &keymap_find, &g_keymap_finds);

// ---- Fireworks::Step99C660 / Step99C730(values, n, -, -, v) (0x99C660 16-bit values, 0x99C730 8-bit) ----------------
// The table of the object +0x134: {.., entries of 8 bytes +0xD4, length +0xE0}. k = v / length 5000 rounded by the FPU
// (one entry for every value). For each signed value s: a = |s K| (K 0xF36BBC for words, 0xF7D05C for bytes); when the
// entry's second float is smaller in size, the entry becomes {v, a} (-a when k is even). Then the table's dwords
// +0x9C38/+0x9C3C become 0x71C9F2CA and 0. Gives 0.
static volatile long g_firework_steps;
template <class V, uintptr_t K> static uint32_t __thiscall fireworks_step(const uint8_t *self, const V *values, int32_t n, uint32_t, uint32_t, float v) {
    g_firework_steps++;
    for (int32_t i = 0; i < n; i++) {
        const uint8_t *const o = *(const uint8_t *const volatile *)(self + 0x134);
        const long double a = __builtin_fabsl((long double)(int32_t)values[i] * *(const float *)K);
        const int32_t k = (int32_t)x87_fistp64_h2((long double)v / *(const float *)(o + 0xe0) * *(const float *)0xf6c844);
        volatile float *const e = (volatile float *)(*(uint8_t *const *)(o + 0xd4) + k * 8);
        const long double b = __builtin_fabsl((long double)e[1]);
        if (b < a) {
            *(volatile uint32_t *)e = *(const uint32_t *)&v;
            e[1] = (float)(k & 1 ? a : -a);
        }
    }
    uint8_t *const t = *(uint8_t *const *)(*(const uint8_t *const *)(self + 0x134) + 0xd4);
    *(volatile uint32_t *)(t + 0x9c38) = 0x71c9f2ca;
    *(volatile uint32_t *)(t + 0x9c3c) = 0;
    return 0;
}
static const uint8_t fireworks_step_99c660_original[] = {0x83, 0xec, 0x10, 0x55, 0x8b, 0x6c, 0x24, 0x1c, 0x57, 0x33, 0xff, 0x85, 0xed};
REWRITE("Fireworks::Step99C660", 0x99c660, fireworks_step_99c660_original, (&fireworks_step<int16_t, 0xf36bbc>), &g_firework_steps);
static const uint8_t fireworks_step_99c730_original[] = {0x83, 0xec, 0x10, 0x55, 0x8b, 0x6c, 0x24, 0x1c, 0x57, 0x33, 0xff, 0x85, 0xed};
REWRITE("Fireworks::Step99C730", 0x99c730, fireworks_step_99c730_original, (&fireworks_step<int8_t, 0xf7d05c>), &g_firework_steps);

// ---- Fence::CellA906C0(grid, p) (0xA906C0): whether a point's cell is marked in a 16 x 16 bit grid ------------------
// i = floor((p.x + 2) 4) and j = floor((p.z + 2) 4) (each a float first); outside 0..15 0. Rows pair up in dwords from
// +8: row j's bits are the dword j / 2's low half (even j) or high half; the bit for i is tested (only al is set).
static volatile long g_fence_cells;
static uint8_t __stdcall fence_cell_a906c0(const uint8_t *grid, const float *p) {
    g_fence_cells++;
    const float two = *(const float *)0xf2705c, four = *(const float *)0xf2669c;
    volatile float u = (float)(((long double)p[0] + two) * four);
    volatile float fu = (float)floor_h2(u);
    const int32_t i = (int32_t)x87_fistp64_h2(fu);
    volatile float w = (float)(((long double)p[2] + two) * four);
    volatile float fw = (float)floor_h2(w);
    const int32_t j = (int32_t)x87_fistp64_h2(fw);
    if (i < 0 || i > 15 || j < 0 || j > 15) return 0;
    const uint32_t mask = (j & 1 ? 0x10000u : 1u) << i;
    return (*(const uint32_t *)(grid + 8 + (j >> 1) * 4) & mask) != 0;
}
static const uint8_t fence_cell_a906c0_original[] = {0x83, 0xec, 0x08, 0x8b, 0x44, 0x24, 0x10, 0xd9, 0x00, 0x56};
REWRITE("Fence::CellA906C0", 0xa906c0, fence_cell_a906c0_original, &fence_cell_a906c0, &g_fence_cells);

// ---- Animal::SumB81E50(x) (0xB81E50): a level 0-3 from a sum of three values against x ---------------------------
// s = (+0xC + +0x10) + +8 of the record +0xA4 (unrounded). When B s 300 (B the game's 0x10ED8D8) is not above x, -1.
// Else with v = A s as a float (A 0x10ED8C4): 0 when 10 v is above x, 1 for 30 v, 2 for 100 v, else 3. At most the
// record's type (+0: its byte +4) less one (signed).
static volatile long g_animal_levels;
static int32_t __thiscall animal_sum_b81e50(const uint8_t *self, float x) {
    g_animal_levels++;
    const uint8_t *const rec = *(const uint8_t *const *)(self + 0xa4);
    auto at = [&](uint32_t o) { return *(const float *)(rec + o); };
    const long double s = ((long double)at(0xc) + at(0x10)) + at(8);
    int32_t level = -1;
    if ((long double)*(const volatile float *)0x10ed8d8 * s * *(const float *)0xf28958 > x) {
        volatile float v = (float)((long double)*(const volatile float *)0x10ed8c4 * s);
        if ((long double)v * *(const float *)0xf275f4 > x) level = 0;
        else if ((long double)v * *(const float *)0xf28218 > x) level = 1;
        else if ((long double)v * *(const float *)0xf27604 > x) level = 2;
        else level = 3;
    }
    const int32_t top = (int32_t)(*(const uint8_t *const *)rec)[4] - 1;
    return level > top ? top : level;
}
static const uint8_t animal_sum_b81e50_original[] = {0x83, 0xec, 0x08, 0x8b, 0x81, 0xa4, 0x00, 0x00, 0x00, 0xd9, 0x40, 0x08};
REWRITE("Animal::SumB81E50", 0xb81e50, animal_sum_b81e50_original, &animal_sum_b81e50, &g_animal_levels);

// ---- Animal::AddTimeB828D0(dt) (0xB828D0): times that wrap around their periods -----------------------------------
// Entries of 0x14 bytes (+4, count +8 read again each time): {kind, time, ..}. time += dt (a float); with T the period
// for the kind (the float table at +0x10 of the record +4 of +0xA4), a time not below T, or below 0, becomes time -
// floor(time / T) T (the quotient a float).
static volatile long g_animal_times;
static void __thiscall animal_add_time_b828d0(const uint8_t *self, float dt) {
    g_animal_times++;
    for (int32_t i = 0; i < *(const volatile int32_t *)(self + 8); i++) {
        uint8_t *const e = *(uint8_t *const volatile *)(self + 4) + i * 0x14;
        volatile float *const t = (volatile float *)(e + 4);
        *t = (float)((long double)dt + *t);
        const float *const table = *(const float *const *)(*(const uint8_t *const *)(*(const uint8_t *const *)(self + 0xa4) + 4) + 0x10);
        volatile float period = table[*(const int32_t *)e];
        const bool wrap = !(*t < period || *t != *t || period != period) || *t < *(const float *)0xf264e0;
        if (!wrap) continue;
        volatile float q = (float)((long double)*t / period);
        volatile float fl = (float)floor_h2(q);
        *t = (float)((long double)*t - (long double)fl * period);
    }
}
static const uint8_t animal_add_time_b828d0_original[] = {0x8b, 0x41, 0x08, 0x83, 0xec, 0x0c, 0x57, 0x33, 0xff, 0x85, 0xc0};
REWRITE("Animal::AddTimeB828D0", 0xb828d0, animal_add_time_b828d0_original, &animal_add_time_b828d0, &g_animal_times);

// ---- WildAnimal::PointB18E70(out, p) (0xB18E70): the nearest of the animal's points that are on --------------------
// Points (+0x110, 0x10 bytes each; count +0x108) with on flags (+0x440, dwords). The best starts as {0, 0, 0} with an
// offset of (10000, 10000); a point on wins when its distance from p across x and z (dz unrounded for its own square)
// is below the best offset's length, which is then its {dz, dx} as floats. Gives out.
static volatile long g_animal_nearest;
static float *__thiscall wild_animal_point_b18e70(const uint8_t *self, float *out, const float *p) {
    g_animal_nearest++;
    long double a = *(const float *)0xf37b34, b = *(const float *)0xf37b34, c = *(const float *)0xf264e0;
    uint32_t bx = 0, bz = 0;
    const int32_t n = *(const int32_t *)(self + 0x108);
    for (int32_t i = 0; i < n; i++) {
        if (!*(const uint32_t *)(self + 0x440 + i * 4)) continue;
        const float *const q = (const float *)(self + 0x110 + i * 0x10);
        volatile float dx = (float)((long double)q[0] - p[0]);
        const long double dzx = (long double)q[2] - p[2];
        volatile float dz = (float)dzx;
        const long double d = x87_sqrt_h2(dzx * dz + (long double)dx * dx);
        const long double m = x87_sqrt_h2(a * a + b * b);
        if (!(m > d)) continue;
        bx = *(const uint32_t *)&q[0], bz = *(const uint32_t *)&q[2];
        c = q[1], b = dx, a = dz;
    }
    volatile float *const o = out;
    o[0] = *(const float *)&bx;
    *(volatile uint32_t *)(o + 2) = bz;
    o[1] = (float)c;
    return out;
}
static const uint8_t wild_animal_point_b18e70_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x28, 0x8b, 0x81, 0x08, 0x01, 0x00, 0x00};
REWRITE("WildAnimal::PointB18E70", 0xb18e70, wild_animal_point_b18e70_original, &wild_animal_point_b18e70, &g_animal_nearest);

// ---- Vector::LengthBECFA0(v) (0xBECFA0): a frame of three axes from a direction -------------------------------------
// Axis 0 (+0) = v made of length 1 ((x^2 + y^2) + z^2; a length of 0 sets z to 1 and leaves x and y). Axis 1 (+0x10):
// when |x| is above both |y| and |z|, {-(x y), 1 - y^2, -(y z)}, else {1 - x^2, -(x y), -(x z)} (products as floats),
// made of length 1 the same way. Axis 2 (+0x20) = axis 1 x axis 0 as floats.
static volatile long g_frames_from;
static inline void unit_becfa0(volatile float *p) {
    const long double x = p[0], y = p[1], z = p[2];
    const long double len = x87_sqrt_h2((x * x + y * y) + z * z);
    if (len == *(const float *)0xf264e0) {
        *(volatile uint32_t *)(p + 2) = 0x3f800000;
        return;
    }
    const long double inv = (long double)*(const float *)0xf264dc / len;
    p[0] = (float)(inv * p[0]);
    p[1] = (float)(inv * p[1]);
    p[2] = (float)(inv * p[2]);
}
static void __thiscall vector_length_becfa0(uint8_t *self, const float *v) {
    g_frames_from++;
    volatile float *const f = (volatile float *)self;
    *(volatile uint32_t *)self = *(const uint32_t *)v;
    *(volatile uint32_t *)(self + 4) = *(const uint32_t *)(v + 1);
    f[2] = v[2];
    unit_becfa0(f);
    const float one = *(const float *)0xf264dc;
    const long double ax = __builtin_fabsl((long double)f[0]), ay = __builtin_fabsl((long double)f[1]);
    if (ax > ay && ax > __builtin_fabsl((long double)f[2])) {
        const long double y = f[1];
        f[5] = (float)(-(y * y) + one);
        f[6] = (float)-((long double)f[2] * f[1]);
        f[4] = (float)-((long double)f[1] * f[0]);
    } else {
        f[5] = (float)-((long double)f[1] * f[0]);
        f[6] = (float)-((long double)f[2] * f[0]);
        const long double x = f[0];
        f[4] = (float)(-(x * x) + one);
    }
    unit_becfa0(f + 4);
    const long double a = (long double)f[5] * f[2] - (long double)f[6] * f[1];
    const long double b = (long double)f[6] * f[0] - (long double)f[2] * f[4];
    const long double c = (long double)f[1] * f[4] - (long double)f[5] * f[0];
    f[10] = (float)c;
    f[8] = (float)a;
    f[9] = (float)b;
}
static const uint8_t vector_length_becfa0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x10, 0x8b, 0x45, 0x08, 0x8b, 0x10};
REWRITE("Vector::LengthBECFA0", 0xbecfa0, vector_length_becfa0_original, &vector_length_becfa0, &g_frames_from);

// ---- Water::Range907440(a, b, c) (0x907440): evens out the heights where four water grids meet -----------------------
// Each grid: size n (+0), +4, spread +0x34, samples +0x40, edge heights +0x4C and +0x50 (float arrays). First a with b:
// the corner samples a1 = a.samples[(n_a - 1) n_a] and b1 = b.samples[n_b - 1] (floats); the larger grid's size over the
// smaller's, q, scales the larger's sample by q^2 (a float), the spread m is the smaller's, and the smaller's share is
// 1 / q (a float; 1 for the larger). H = ((a.h4C[+4 n_a] + b.h4C[n_b]) / 2 + (a1 - b1) m) K (K 0xF7D038); b's height
// becomes its share H, a's H its share. Then the grid itself with c the same way with s = samples[n n - 1] and c0 =
// c.samples[0] (unrounded, the larger's scaled by q^2), heights h50[(+4 + 1) n] and c.h50[0].
static volatile long g_water_meets;
static void __thiscall water_range_907440(const uint8_t *self, const uint8_t *a, const uint8_t *b, const uint8_t *c) {
    g_water_meets++;
    auto num = [](const uint8_t *g, uint32_t o) { return *(const int32_t *)(g + o); };
    auto arr = [](const uint8_t *g, uint32_t o) { return *(volatile float *const *)(g + o); };
    auto val = [](const uint8_t *g, uint32_t o) { return *(const float *)(g + o); };
    const float half = *(const float *)0xf2651c, k = *(const float *)0xf7d038, one = *(const float *)0xf264dc;
    const int32_t n = num(self, 0);
    const long double s0 = arr(self, 0x40)[n * n - 1];
    const int32_t na = num(a, 0), nb = num(b, 0);
    volatile float share_b = 1.0f, share_a = 1.0f;
    volatile float a1 = arr(a, 0x40)[(na - 1) * na];
    volatile float b1 = arr(b, 0x40)[nb - 1];
    const long double c0 = arr(c, 0x40)[0];
    long double m;
    if (na > nb) {
        const int32_t q = na / nb;
        a1 = (float)((long double)(q * q) * a1);
        m = val(b, 0x34);
        share_a = (float)(one / (long double)q);
    } else {
        const int32_t q = nb / na;
        b1 = (float)((long double)(q * q) * b1);
        m = val(a, 0x34);
        share_b = (float)(one / (long double)q);
    }
    volatile float *const bh = arr(b, 0x4c) + nb;
    const long double h1 = ((long double)arr(a, 0x4c)[num(a, 4) * na] + *bh) * half;
    const long double big = (h1 + ((long double)a1 - b1) * m) * k;
    *bh = (float)((long double)share_b * big);
    arr(a, 0x4c)[num(a, 0) * num(a, 4)] = (float)(big * share_a);

    const int32_t nc = num(c, 0);
    long double s = s0, c1 = c0, m2;
    volatile float share_c = 1.0f, share_s = 1.0f;
    if (n > nc) {
        const int32_t q = n / nc;
        s = s0 * (long double)(q * q);
        m2 = val(c, 0x34);
        share_s = (float)(one / (long double)q);
    } else {
        const int32_t q = nc / n;
        c1 = c0 * (long double)(q * q);
        m2 = val(self, 0x34);
        share_c = (float)(one / (long double)q);
    }
    volatile float *const ch = arr(c, 0x50);
    const long double h2 = ((long double)arr(self, 0x50)[(num(self, 4) + 1) * n] + *ch) * half;
    const long double big2 = (h2 + (s - c1) * m2) * k;
    *ch = (float)((long double)share_c * big2);
    arr(self, 0x50)[(num(self, 4) + 1) * num(self, 0)] = (float)(big2 * share_s);
}
static const uint8_t water_range_907440_original[] = {0x83, 0xec, 0x0c, 0x8b, 0x01, 0x8b, 0xd0, 0x0f, 0xaf, 0xd0, 0x8b, 0x41, 0x40};
REWRITE("Water::Range907440", 0x907440, water_range_907440_original, &water_range_907440, &g_water_meets);

// ---- List::PassC236C0(out, node, range) (0xC236C0): copies a run of linked nodes into records, last first ----------------
// Skips range[0] nodes (next at +0x54), then for range[1] nodes fills records of 0xB0 bytes from the last one down: the
// node at +0, the next node at +4, the node's +0x10..+0x48 dwords (but +0x1C, +0x2C, +0x3C) at the same places and its
// +0xC0..+0xF8 at +0x50..+0x88 (also skipping each fourth; the first two of +0xC0 and +0xF0 go through the FPU). Then
// the first record's +0x50.. part is copied onto its own +0x10.. part.
static volatile long g_node_runs;
static inline void fpu_copy_h2(void *to, const void *from) { __asm__ volatile("flds (%1)\n\tfstps (%0)" : : "r"(to), "r"(from) : "memory"); }
static const uint8_t *__stdcall list_pass_c236c0(uint8_t *out, const uint8_t *node, const int32_t *range) {
    g_node_runs++;
    auto next = [](const uint8_t *p) { return *(const uint8_t *const *)(p + 0x54); };
    const uint8_t *prev = node;   // the last node skipped (the game gives it back)
    for (int32_t s = range[0]; s > 0; s--) prev = node, node = next(node);
    for (int32_t i = range[1] - 1; i >= 0; i--) {
        uint32_t *const e = (uint32_t *)(out + i * 0xb0);
        const uint32_t *const n = (const uint32_t *)node;
        e[0] = (uint32_t)(uintptr_t)node;
        for (uint32_t k = 4; k < 0x13; k++)
            if ((k & 3) != 3) e[k] = n[k];
        fpu_copy_h2(e + 0x14, n + 0x30);
        fpu_copy_h2(e + 0x15, n + 0x31);
        e[0x16] = n[0x32];
        for (uint32_t k = 0x18; k < 0x1f; k++)
            if ((k & 3) != 3) e[k] = n[k + 0x1c];
        e[0x22] = n[0x3e];
        fpu_copy_h2(e + 0x20, n + 0x3c);
        fpu_copy_h2(e + 0x21, n + 0x3d);
        node = next(node);
        e[1] = (uint32_t)(uintptr_t)node;
    }
    uint32_t *const o = (uint32_t *)out;
    for (uint32_t k = 4; k < 0x13; k++)
        if ((k & 3) != 3) o[k] = o[k + 0x10];
    return prev;
}
static const uint8_t list_pass_c236c0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x44, 0x8b, 0x4d, 0x0c, 0x53};
REWRITE("List::PassC236C0", 0xc236c0, list_pass_c236c0_original, &list_pass_c236c0, &g_node_runs);

// ---- Animal::LoopB788A0(out) (0xB788A0): the record with the lowest value over a list ------------------------------
// The list runs from +0x1C to its head at +0x28 (next at +0); each node points to a record (+8) whose float +8 is
// compared: the last one not above the best so far (from 1000) is kept and copied out (dwords +0..+0x1C (the first
// through the FPU), +0x24..+0x34, +0x40..+0x48, +0x50, +0x54 and the bytes +0x20 and +0x58). With no record kept the
// game copies out stack bytes it never set; this gives zeros there instead.
static volatile long g_animal_lowest;
static void __thiscall animal_loop_b788a0(const uint8_t *self, uint8_t *out) {
    g_animal_lowest++;
    uint32_t best[0x5c / 4 + 1] = {};
    *(float *)&best[2] = 1000.0f;
    for (const uint32_t *p = *(const uint32_t *const *)(self + 0x1c); p != (const uint32_t *)(self + 0x28); p = (const uint32_t *)(uintptr_t)*p) {
        const uint32_t *const rec = (const uint32_t *)(uintptr_t)p[2];
        if (!(*(const float *)&rec[2] <= *(const float *)&best[2])) continue;
        for (uint32_t k = 0; k <= 0x16; k++) best[k] = rec[k];
    }
    uint32_t *const o = (uint32_t *)out;
    fpu_copy_h2(o, &best[0]);
    for (uint32_t k = 1; k <= 7; k++) o[k] = best[k];
    out[0x20] = (uint8_t)best[8];
    for (uint32_t k = 9; k <= 0xd; k++) o[k] = best[k];
    o[0x10] = best[0x10], o[0x11] = best[0x11], o[0x12] = best[0x12], o[0x14] = best[0x14], o[0x15] = best[0x15];
    out[0x58] = (uint8_t)best[0x16];
}
static const uint8_t animal_loop_b788a0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x74, 0x8b, 0x51, 0x1c, 0x53};
REWRITE("Animal::LoopB788A0", 0xb788a0, animal_loop_b788a0_original, &animal_loop_b788a0, &g_animal_lowest);

// ---- Renderer::SetMaterial(material) (0xC946D0) --------------------------------------------------------------------
// Nothing when it is the current one (+0x2140). Else it is kept, +0x1CB8 becomes -1 and its texture set (+0x28) is kept
// at +0x2144; with no set +0x1CBC is 0, else -1 and for each texture (count: the set's word +0xCA, read again each time)
// slot i (+0x214C) points to the set's entry i (0x14 bytes from +0xCC) and the set's byte +0xC4 + i becomes 0xFF (past
// the sixth these bytes run over the count and the entries, as in the game).
static volatile long g_material_sets;
static void __thiscall renderer_set_material(uint8_t *self, const uint8_t *material) {
    g_material_sets++;
    if (*(const uint8_t *const *)(self + 0x2140) == material) return;
    *(const uint8_t **)(self + 0x2140) = material;
    *(volatile int32_t *)(self + 0x1cb8) = -1;
    uint8_t *const set = *(uint8_t *const *)(material + 0x28);
    *(uint8_t **)(self + 0x2144) = set;
    if (!set) {
        *(volatile int32_t *)(self + 0x1cbc) = 0;
        return;
    }
    *(volatile int32_t *)(self + 0x1cbc) = -1;
    for (uint32_t i = 0; i < *(volatile uint16_t *)(set + 0xca); i++) {
        *(volatile uint32_t *)(self + 0x214c + i * 4) = (uint32_t)(uintptr_t)(set + 0xcc + i * 0x14);
        *(volatile uint8_t *)(set + 0xc4 + i) = 0xff;
    }
}
static const uint8_t renderer_set_material_original[] = {0x8b, 0x44, 0x24, 0x04, 0x39, 0x81, 0x40, 0x21, 0x00, 0x00};
REWRITE("Renderer::SetMaterial", 0xc946d0, renderer_set_material_original, &renderer_set_material, &g_material_sets);

// ---- Track::InitBB83A0(owner out, b, from, d) (0xBB83A0): fills a record from its arguments --------------------------
// +8 = itself, +0xC = the owner place, +0x10 = b, +0x3C = d; from a source: its byte 0 at +0x14, +0x15 = 1, +0x18 = 1
// when its byte 1 is set else -1, its dwords +8..+0x14 at +0x1C..+0x28 and +4 at +0x2C. +0x30, +0x34 and the words
// +0x38, +0x3A are 0, then +0x30 = +0x24 when +0x28 is below the smallest float (0xF26E2C). The owner place, when given,
// is pointed at the record. Gives the record.
static volatile long g_track_inits;
static uint8_t *__thiscall track_init_bb83a0(uint8_t *self, uint8_t **owner, uint32_t b, const uint8_t *from, uint32_t d) {
    g_track_inits++;
    uint32_t *const u = (uint32_t *)self;
    const uint32_t *const s = (const uint32_t *)from;
    u[4] = b;
    u[2] = (uint32_t)(uintptr_t)self;
    u[3] = (uint32_t)(uintptr_t)owner;
    self[0x14] = from[0];
    self[0x15] = 1;
    u[6] = from[1] ? 1 : (uint32_t)-1;
    u[7] = s[2], u[8] = s[3], u[9] = s[4], u[10] = s[5];
    u[11] = s[1];
    u[15] = d;
    u[12] = 0, u[13] = 0;
    *(uint16_t *)(self + 0x38) = 0, *(uint16_t *)(self + 0x3a) = 0;
    if (*(const float *)(self + 0x28) < *(const float *)0xf26e2c) u[12] = u[9];
    if (owner) *owner = self;
    return self;
}
static const uint8_t track_init_bb83a0_original[] = {0x8b, 0x44, 0x24, 0x08, 0x89, 0x41, 0x10, 0x8b, 0x44, 0x24, 0x0c, 0x53};
REWRITE("Track::InitBB83A0", 0xbb83a0, track_init_bb83a0_original, &track_init_bb83a0, &g_track_inits);

// ---- Effects::Entry455040(n, p, -) (0x455040): a value from entry n (0x90 bytes, kind first) -------------------------
// Kind 1: the distance from p to the entry's point (+0x40; (z^2 + x^2) + y^2) over its +0x60. Kind 3: the mean of its
// +8, +0xC and +0x10 ((+0x10 + +0xC) + +8, times 1/3 as a float). Otherwise 0.
static volatile long g_effect_values;
static long double __thiscall effects_entry_455040(const uint8_t *self, int32_t n, const float *p, uint32_t) {
    g_effect_values++;
    const uint8_t *const e = *(const uint8_t *const *)self + n * 0x90;
    auto at = [&](uint32_t o) { return *(const float *)(e + o); };
    switch (*(const int32_t *)e) {
    case 1: {
        const long double dx = (long double)at(0x40) - p[0], dy = (long double)at(0x44) - p[1], dz = (long double)at(0x48) - p[2];
        return x87_sqrt_h2((dz * dz + dx * dx) + dy * dy) / at(0x60);
    }
    case 3: return (((long double)at(0x10) + at(0xc)) + at(8)) * *(const float *)0xf27060;
    default: return *(const float *)0xf264e0;
    }
}
static const uint8_t effects_entry_455040_original[] = {0x55, 0x8b, 0xec, 0x8b, 0x45, 0x08, 0x8b, 0x11, 0x8d, 0x04, 0xc0};
REWRITE("Effects::Entry455040", 0x455040, effects_entry_455040_original, &effects_entry_455040, &g_effect_values);


// ---- KeyMap::Clear, Table::Clear (0xC47750 entries of 48 bytes, 0xC47490 of 20) -------------------------------------
// Each entry (count +8, read again each time; at +0x10) zeroed and its mark (words at +0x14) 0; then the count 0 and
// the buckets (+0xC, +4 + 1 dwords) zeroed.
static volatile long g_table_clears;
template <uint32_t Size> static void __thiscall table_clear(uint8_t *self) {
    g_table_clears++;
    for (uint32_t i = 0; i < *(volatile uint32_t *)(self + 8); i++) {
        volatile uint32_t *const e = (volatile uint32_t *)(*(uint8_t *const *)(self + 0x10) + i * Size);
        for (uint32_t k = 0; k < Size / 4; k++) e[k] = 0;
        (*(volatile uint16_t *const *)(self + 0x14))[i] = 0;
    }
    const uint32_t n = *(const uint32_t *)(self + 4) * 4 + 4;
    volatile uint8_t *const b = *(volatile uint8_t *const *)(self + 0xc);
    *(volatile uint32_t *)(self + 8) = 0;
    for (uint32_t i = 0; i < n; i++) b[i] = 0;
}
static const uint8_t keymap_clear_original[] = {0x83, 0xec, 0x30, 0x8b, 0xd1, 0x8b, 0x42, 0x08, 0x53, 0x33, 0xdb};
REWRITE("KeyMap::Clear", 0xc47750, keymap_clear_original, &table_clear<0x30>, &g_table_clears);
static const uint8_t table_clear_original[] = {0x8b, 0xd1, 0x8b, 0x42, 0x08, 0x33, 0xc9, 0x85, 0xc0, 0x57};
REWRITE("Table::Clear", 0xc47490, table_clear_original, &table_clear<0x14>, &g_table_clears);

// ---- Entries::Remove(id) (0x940A20): takes out the entry (16 bytes, id first) with an id --------------------------
// The first entry with the id (count +8, signed) gets the last one's id and +0xC, and swaps its +4 and +8 with the
// last's (nothing when it is the last); the count goes down by one; 1. 0 (only al) when there is none.
static volatile long g_entry_removes;
static uint8_t __thiscall entries_remove(uint8_t *self, uint32_t id) {
    g_entry_removes++;
    const int32_t n = *(const int32_t *)(self + 8);
    uint32_t *const list = *(uint32_t *const *)self;
    int32_t i = 0;
    while (i < n && list[i * 4] != id) i++;
    if (i >= n) return 0;
    if (i != n - 1) {
        uint32_t *const e = list + i * 4, *const l = list + (n - 1) * 4;
        e[0] = l[0];
        e[3] = l[3];
        const uint32_t a = e[1], b = e[2];
        e[1] = l[1], e[2] = l[2];
        l[1] = a, l[2] = b;
    }
    *(int32_t *)(self + 8) -= 1;
    return 1;
}
static const uint8_t entries_remove_original[] = {0x8b, 0x51, 0x08, 0x53, 0x55, 0x56, 0x33, 0xf6, 0x85, 0xd2, 0x57};
REWRITE("Entries::Remove", 0x940a20, entries_remove_original, &entries_remove, &g_entry_removes);

// ---- Effects::Value4377E0(-, from) (0x4377E0): copies a value of a type into its record ----------------------------
// The type word (+8): low byte k, high byte h. Its size: h 0 the table at 0x10F2BA8 entry k, h 1 16, h past 0x7F
// (h - 0x7F) times that entry plus 4, else 0. Copied from from to the record (+0: its +0x14) at the word +0xA. Gives
// the size.
static volatile long g_effect_copies;
static uint32_t __thiscall effects_value_4377e0(const uint8_t *self, uint32_t, const uint8_t *from) {
    g_effect_copies++;
    const uint8_t k = self[8], h = self[9];
    const uint32_t *const sizes = (const uint32_t *)0x10f2ba8;
    uint32_t n;
    if (h == 0) n = sizes[k];
    else if (h == 1) n = 0x10;
    else if (h > 0x7f) n = (uint32_t)(h - 0x7f) * sizes[k] + 4;
    else n = 0;
    volatile uint8_t *const to = *(uint8_t *const *)(*(const uint8_t *const *)self + 0x14) + *(const uint16_t *)(self + 0xa);
    for (uint32_t i = 0; i < n; i++) to[i] = from[i];
    return n;
}
static const uint8_t effects_value_4377e0_original[] = {0x8b, 0xd1, 0x66, 0x8b, 0x42, 0x08, 0x84, 0xe4, 0x56, 0x57};
REWRITE("Effects::Value4377E0", 0x4377e0, effects_value_4377e0_original, &effects_value_4377e0, &g_effect_copies);

// ---- UIntSlider::SetValue(f) (0xCA8560) ----------------------------------------------------------------------------
// v = (max - min) f + min (both unsigned, made so by adding 2^32 to negative ones; rounded by the FPU, low 32 bits),
// kept within min..max (unsigned), into the bound number (+4).
static volatile long g_uint_sliders;
static void __thiscall uint_slider_set_value(const uint8_t *self, float f) {
    g_uint_sliders++;
    const uint32_t lo = *(const uint32_t *)(self + 8), hi = *(const uint32_t *)(self + 0xc);
    const float big = *(const float *)0xf285f8;
    auto load = [&](uint32_t u) {   // fild, then 2^32 added (a rounded add) when negative, as the game does
        const int32_t i = (int32_t)u;
        long double r;
        __asm__("fildl %1" : "=t"(r) : "m"(i));
        return i < 0 ? r + big : r;
    };
    const long double scaled = load(hi - lo) * f;
    const long double base = load(lo);
    uint32_t v = (uint32_t)x87_fistp64_h2(scaled + base);
    if (lo > v) v = lo;
    if (hi < v) v = hi;
    **(uint32_t *const *)(self + 4) = v;
}
static const uint8_t uint_slider_set_value_original[] = {0x83, 0xec, 0x08, 0x8b, 0x51, 0x0c, 0x8b, 0x41, 0x08, 0x56};
REWRITE("UIntSlider::SetValue", 0xca8560, uint_slider_set_value_original, &uint_slider_set_value, &g_uint_sliders);

// ---- WideString::FindLast(ch, from) (0xBCEF20) ---------------------------------------------------------------------
// The text (+0: characters +0xC, length +4); from the last character (from -1) or from min(length - 1, from)
// (unsigned) back to the first, the place of the first wide character equal to ch's low byte, or -1.
static volatile long g_wide_find_lasts;
static int32_t __thiscall wide_string_find_last(const uint8_t *self, uint32_t ch, uint32_t from) {
    g_wide_find_lasts++;
    const uint8_t *const s = *(const uint8_t *const *)self;
    const uintptr_t buf = *(const uint32_t *)(s + 0xc);
    const uint32_t last = *(const uint32_t *)(s + 4) - 1;
    const uint16_t c = (uint8_t)ch;
    uintptr_t p = buf + (uintptr_t)((from == 0xffffffffu ? last : last > from ? from : last) * 2);
    for (; p >= buf; p -= 2)
        if (*(const uint16_t *)p == c) return (int32_t)(p - buf) >> 1;
    return -1;
}
static const uint8_t wide_string_find_last_original[] = {0x51, 0x8b, 0x01, 0x8b, 0x48, 0x0c, 0x8b, 0x40, 0x04, 0x33, 0xd2};
REWRITE("WideString::FindLast", 0xbcef20, wide_string_find_last_original, &wide_string_find_last, &g_wide_find_lasts);

// ---- TypeList::Copy(from) (0xBD2170) -------------------------------------------------------------------------------
// The four types and their count (+0x10) copied; +0x1C and +0x28 point at the list, the byte +0x3C and +0x40..+0x48
// are 0, and each type's size (the table at 0xFA14D8 by its top four bits) goes to +0x2C on. Gives the list.
static volatile long g_type_list_copies;
static uint32_t *__thiscall type_list_copy(uint32_t *self, const uint32_t *from) {
    g_type_list_copies++;
    for (int k = 0; k < 5; k++) self[k] = from[k];
    self[7] = (uint32_t)(uintptr_t)self;
    self[10] = (uint32_t)(uintptr_t)self;
    *(uint8_t *)(self + 15) = 0;
    self[16] = 0, self[17] = 0, self[18] = 0;
    for (int32_t i = 0; i < *(volatile int32_t *)(self + 4); i++) self[11 + i] = ((const uint32_t *)0xfa14d8)[self[i] >> 28];
    return self;
}
static const uint8_t type_list_copy_original[] = {0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x04, 0x56, 0x8b, 0x31};
REWRITE("TypeList::Copy", 0xbd2170, type_list_copy_original, &type_list_copy, &g_type_list_copies);

// ---- TrackDesigner::CarSettingMode(car) (0x4EF080) -----------------------------------------------------------------
// Left alone while +0x810 is 0. Else 1 with no car, 3 when the car's +0x1BC is below 0, 1 when it is 0, else 2.
static volatile long g_car_setting_modes;
static void __thiscall track_designer_car_setting_mode(uint8_t *self, const uint8_t *car) {
    g_car_setting_modes++;
    int32_t *const mode = (int32_t *)(self + 0x810);
    if (!*mode) return;
    if (!car) {
        *mode = 1;
        return;
    }
    const float v = *(const float *)(car + 0x1bc), zero = *(const float *)0xf264e0;
    *mode = v < zero ? 3 : v == zero ? 1 : 2;
}
static const uint8_t track_designer_car_setting_mode_original[] = {0x8b, 0x81, 0x10, 0x08, 0x00, 0x00, 0x85, 0xc0, 0x74, 0x52};
REWRITE("TrackDesigner::CarSettingMode", 0x4ef080, track_designer_car_setting_mode_original, &track_designer_car_setting_mode, &g_car_setting_modes);

// ---- Section::Section(name, kind) (0x46FAD0) -----------------------------------------------------------------------
// Takes a reference to the name (its count +0 goes up), keeps the kind (+4), zeroes +8..+0x324 and +0x328, +0x340,
// +0x34C, +0x350; the child list at +0x338 ({first +0x344, .., end +0x348 = the list}, +0x33C and +0x344 0); +0x354 = 8
// and +0x360 = itself. Gives the section.
static volatile long g_sections;
static uint8_t *__thiscall section_section(uint8_t *self, uint32_t *const *name, uint32_t kind) {
    g_sections++;
    uint32_t *const u = (uint32_t *)self;
    uint32_t *const n = *name;
    u[0] = (uint32_t)(uintptr_t)n;
    n[0]++;
    u[0x328 / 4] = 0;
    u[1] = kind;
    u[0x340 / 4] = 0, u[0x34c / 4] = 0;
    u[0x33c / 4] = 0, u[0x344 / 4] = 0;
    u[0x348 / 4] = (uint32_t)(uintptr_t)(self + 0x338);
    u[0x338 / 4] = (uint32_t)(uintptr_t)(self + 0x344);
    u[0x350 / 4] = 0, u[0x354 / 4] = 8;
    u[0x360 / 4] = (uint32_t)(uintptr_t)self;
    for (uint32_t k = 2; k < 0x328 / 4; k++) u[k] = 0;
    return self;
}
static const uint8_t section_section_original[] = {0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x04, 0x8b, 0x09, 0x89, 0x08};
REWRITE("Section::Section", 0x46fad0, section_section_original, &section_section, &g_sections);

// ---- ShortSlider::SetValue, UShortSlider::SetValue(f) (0xCA82C0 signed, 0xCA83D0 unsigned) ----------------------------
// As UIntSlider::SetValue for 16-bit bounds (+8, +0xA): v = (max - min) f + min (rounded by the FPU, low 16 bits),
// kept within min..max, into the bound number (+4).
static volatile long g_short_sliders;
template <class T> static void __thiscall short_slider_set_value(const uint8_t *self, float f) {
    g_short_sliders++;
    const T lo = *(const T *)(self + 8), hi = *(const T *)(self + 0xa);
    T v = (T)x87_fistp64_h2((long double)((int32_t)hi - (int32_t)lo) * f + (long double)(int32_t)lo);
    if (lo > v) v = lo;
    **(T *const *)(self + 4) = hi < v ? hi : v;
}
static const uint8_t short_slider_set_value_original[] = {0x83, 0xec, 0x0c, 0x66, 0x8b, 0x41, 0x08, 0x66, 0x8b, 0x51, 0x0a, 0x56, 0x57, 0x0f, 0xbf};
REWRITE("ShortSlider::SetValue", 0xca82c0, short_slider_set_value_original, &short_slider_set_value<int16_t>, &g_short_sliders);
static const uint8_t ushort_slider_set_value_original[] = {0x83, 0xec, 0x0c, 0x66, 0x8b, 0x41, 0x08, 0x66, 0x8b, 0x51, 0x0a, 0x56, 0x57, 0x0f, 0xb7};
REWRITE("UShortSlider::SetValue", 0xca83d0, ushort_slider_set_value_original, &short_slider_set_value<uint16_t>, &g_short_sliders);

// ---- String::FindLastBCF540(ch, from) (0xBCF540) -------------------------------------------------------------------------
// The text (+0: characters +0xC, length +4); from the last character (from -1) or from min(length, from) (unsigned:
// the end itself when from is past it) back to the first, the place of the first byte equal to ch, or -1.
static volatile long g_find_lasts;
static int32_t __thiscall string_find_last_bcf540(const uint8_t *self, uint32_t ch, uint32_t from) {
    g_find_lasts++;
    const uint8_t *const s = *(const uint8_t *const *)self;
    const uintptr_t buf = *(const uint32_t *)(s + 0xc);
    const uint32_t len = *(const uint32_t *)(s + 4);
    uintptr_t p = from == 0xffffffffu ? buf + len - 1 : buf + (len > from ? from : len);
    for (; p >= buf; p--)
        if (*(const uint8_t *)p == (uint8_t)ch) return (int32_t)(p - buf);
    return -1;
}
static const uint8_t string_find_last_bcf540_original[] = {0x8b, 0x54, 0x24, 0x08, 0x8b, 0x01, 0x8b, 0x48, 0x0c, 0x83, 0xec, 0x08};
REWRITE("String::FindLastBCF540", 0xbcf540, string_find_last_bcf540_original, &string_find_last_bcf540, &g_find_lasts);

// ---- Sound::VolumeToHundredthsDb, VolumeToHundredthsDb2(v) (0xCE45F0, 0xC193E0) ---------------------------------------
// -10000 when v is not above F (0xF2CF10); 0 from 1 on; else log10(v) 20 100 rounded by the FPU (low 32 bits).
static volatile long g_volume_dbs;
static inline long double x87_log10_h2(long double v) {
    long double r;
    __asm__("fldlg2\n\tfxch %%st(1)\n\tfyl2x" : "=t"(r) : "0"(v));
    return r;
}
static int32_t __stdcall sound_volume_to_db(float v) {
    g_volume_dbs++;
    if (!(v > *(const float *)0xf2cf10 || v != v)) return -10000;
    if (!(v < *(const float *)0xf264dc || v != v)) return 0;
    return (int32_t)x87_fistp64_h2(x87_log10_h2(v) * *(const float *)0xf27630 * *(const float *)0xf27604);
}
static const uint8_t sound_volume_to_db_original[] = {0xd9, 0x44, 0x24, 0x04, 0x83, 0xec, 0x08, 0xd8, 0x1d, 0x10, 0xcf, 0xf2, 0x00};
REWRITE("Sound::VolumeToHundredthsDb", 0xce45f0, sound_volume_to_db_original, &sound_volume_to_db, &g_volume_dbs);
REWRITE("Sound::VolumeToHundredthsDb2", 0xc193e0, sound_volume_to_db_original, &sound_volume_to_db, &g_volume_dbs);

// ---- Fireworks::Size9FBD30(out, v) (0x9FBD30) ----------------------------------------------------------------------
// s = the size (+0x74), halved when the game's 0x10EDB90 is set; out = {s v.x, s v.y, s v.z (floats), v.w}. Gives out.
static volatile long g_firework_sizes;
static uint32_t *__thiscall fireworks_size_9fbd30(const uint8_t *self, uint32_t *out, const float *v) {
    g_firework_sizes++;
    long double s = *(const float *)(self + 0x74);
    if (*(const volatile uint32_t *)0x10edb90) s *= *(const float *)0xf2651c;
    volatile float x = (float)(s * v[0]), y = (float)(s * v[1]), z = (float)(s * v[2]);
    const uint32_t w = ((const uint32_t *)v)[3];
    volatile uint32_t *const o = out;
    o[0] = *(volatile uint32_t *)&x, o[1] = *(volatile uint32_t *)&y, o[2] = *(volatile uint32_t *)&z, o[3] = w;
    return out;
}
static const uint8_t fireworks_size_9fbd30_original[] = {0xa1, 0x90, 0xdb, 0x0e, 0x01, 0xd9, 0x41, 0x74, 0x83, 0xec, 0x10};
REWRITE("Fireworks::Size9FBD30", 0x9fbd30, fireworks_size_9fbd30_original, &fireworks_size_9fbd30, &g_firework_sizes);

// ---- Water::Entry8FE240(out, n, t) (0x8FE240): a point from entry n (0x60 bytes at +0xA8) ----------------------------
// The entry's point (+0x10); when its byte +0x38 is set its y becomes y t + (h1 + h0) (1 - t), the heights +0x1C and
// +0x18 of the record +0xC of its part +0x4C. Gives out.
static volatile long g_water_points;
static float *__thiscall water_entry_8fe240(const uint8_t *self, float *out, int32_t n, float t) {
    g_water_points++;
    const uint8_t *const e = *(const uint8_t *const *)(self + 0xa8) + n * 0x60;
    const float x = *(const float *)(e + 0x10), z = *(const float *)(e + 0x18);
    long double y = *(const float *)(e + 0x14);
    if (e[0x38]) {
        const uint8_t *const r = *(const uint8_t *const *)(*(const uint8_t *const *)(e + 0x4c) + 0xc);
        const long double a = ((long double)*(const float *)(r + 0x1c) + *(const float *)(r + 0x18)) * ((long double)*(const float *)0xf264dc - t);
        y = a + y * t;
    }
    volatile float *const o = out;
    o[0] = x, o[1] = (float)y, o[2] = z;
    return out;
}
static const uint8_t water_entry_8fe240_original[] = {0x55, 0x8b, 0xec, 0x8b, 0x45, 0x0c, 0x8b, 0x91, 0xa8, 0x00, 0x00, 0x00};
REWRITE("Water::Entry8FE240", 0x8fe240, water_entry_8fe240_original, &water_entry_8fe240, &g_water_points);

// ---- Shop::Swing4C4BA0(p) (0x4C4BA0) -------------------------------------------------------------------------------
// +0x90 += p.x 0 (a float: only a NaN or an infinity there moves it), then kept within -L..L (L the game's 0x10EE2A4,
// -L as a float; the unrounded sum is what is compared).
static volatile long g_shop_swings;
static void __thiscall shop_swing_4c4ba0(uint8_t *self, const float *p) {
    g_shop_swings++;
    volatile float *const v = (volatile float *)(self + 0x90);
    const long double s = (long double)p[0] * *(const float *)0xf264e0 + *v;
    *v = (float)s;
    const volatile float *const limit = (const volatile float *)0x10ee2a4;
    volatile float neg = (float)-(long double)*limit;
    if (s < neg) *(volatile uint32_t *)v = *(volatile uint32_t *)&neg;
    else if (s > *limit) *(volatile uint32_t *)v = *(const volatile uint32_t *)limit;
}
static const uint8_t shop_swing_4c4ba0_original[] = {0x8b, 0x44, 0x24, 0x04, 0xd9, 0x00, 0xd8, 0x0d, 0xe0, 0x64, 0xf2, 0x00};
REWRITE("Shop::Swing4C4BA0", 0x4c4ba0, shop_swing_4c4ba0_original, &shop_swing_4c4ba0, &g_shop_swings);

// ---- PackedRef::Resolve(ref) (0xBDD0D0) ----------------------------------------------------------------------------
// The top two bits of the low half pick base 0-2 (+8 on) for the low 14 bits; with both set, bit 13 picks a block b:
// the low 13 bits o are an offset from the block's own place (+0: a list of places) while o is below its first dword's
// +0x24, else from base +0x14 + 4 b.
static volatile long g_packed_refs;
static uintptr_t __thiscall packed_ref_resolve(const uint32_t *self, uint32_t ref) {
    g_packed_refs++;
    const uint32_t top = (ref & 0xffff) >> 14;
    if (top < 3) return self[2 + top] + (ref & 0x3fff);
    const uint32_t b = ref >> 13 & 1, o = ref & 0x1fff;
    const uint32_t place = ((const uint32_t *)(uintptr_t)self[0])[b];
    const uint32_t limit = *(const uint32_t *)(uintptr_t)(*(const uint32_t *)(uintptr_t)place + 0x24);
    return (o < limit ? place : self[5 + b]) + o;
}
static const uint8_t packed_ref_resolve_original[] = {0x8b, 0x44, 0x24, 0x04, 0x66, 0x8b, 0xd0, 0x66, 0xc1, 0xea, 0x0e};
REWRITE("PackedRef::Resolve", 0xbdd0d0, packed_ref_resolve_original, &packed_ref_resolve, &g_packed_refs);

// ---- Vector::Quadrant (0x55FC10): which of four ways a flat vector (in eax) mostly points -----------------------------
// When |x| is above |z|: 1 for x above 0, else 0. Otherwise 3 for z above 0, else 2 (a NaN counting as neither).
static volatile long g_quadrants;
extern "C" uint32_t __stdcall forever_quadrant(const float *v) {
    g_quadrants++;
    const float x = v[0], z = v[2], zero = *(const float *)0xf264e0;
    const long double ax = __builtin_fabsl((long double)x), az = __builtin_fabsl((long double)z);
    if (az < ax) return x > zero ? 1 : 0;
    return z > zero ? 3 : 2;
}
extern "C" __attribute__((naked)) void forever_quadrant_regs() {   // v in eax
    __asm__("pushl %eax\n\tcalll _forever_quadrant@4\n\tret");
}
static const uint8_t vector_quadrant_original[] = {0x51, 0xd9, 0x00, 0x8b, 0x40, 0x08, 0xd9, 0xc0, 0x89, 0x04, 0x24};
REWRITE("Vector::Quadrant", 0x55fc10, vector_quadrant_original, (void *)&forever_quadrant_regs, &g_quadrants);

// ---- ActiveList::Remove(n) (0x9FCA90) ------------------------------------------------------------------------------
// When entry n's active byte (0x30 bytes each at +0x54, +0xC) is set: it is cleared and n's place in the active list
// (+0x64, count +0x6C) gets the last one (by the count as it was first read); the count goes down by one.
static volatile long g_active_removes;
static void __thiscall active_list_remove(uint8_t *self, int32_t n) {
    g_active_removes++;
    uint8_t *const flag = *(uint8_t *const *)(self + 0x54) + n * 0x30 + 0xc;
    if (!*flag) return;
    *flag = 0;
    const int32_t count = *(const int32_t *)(self + 0x6c);
    int32_t *const list = *(int32_t *const *)(self + 0x64);
    for (int32_t i = 0; i < *(volatile int32_t *)(self + 0x6c); i++)
        if (list[i] == n) {
            list[i] = list[count - 1];
            *(int32_t *)(self + 0x6c) -= 1;
            return;
        }
}
static const uint8_t active_list_remove_original[] = {0x8b, 0x51, 0x54, 0x53, 0x8b, 0x5c, 0x24, 0x08, 0x8d, 0x04, 0x5b};
REWRITE("ActiveList::Remove", 0x9fca90, active_list_remove_original, &active_list_remove, &g_active_removes);

// ---- List::TakeAll(to, from) (0x966E40) ----------------------------------------------------------------------------
// to becomes empty ({first = its end node +0xC, 0, .., end.next 0, place = to}); then each node of from (from its first
// until the end node, whose next is 0) is put in after to's place (+0x10): nodes {next, back}.
static volatile long g_list_take_alls;
static void __stdcall list_take_all(uint32_t *to, uint32_t *const *from) {
    g_list_take_alls++;
    to[0] = (uint32_t)(uintptr_t)(to + 3);
    to[3] = 0;
    to[1] = 0;
    to[4] = (uint32_t)(uintptr_t)to;
    uint32_t *n = *from;
    if (!n[0] || !n) return;
    while (n) {
        uint32_t *const nx = (uint32_t *)(uintptr_t)n[0];
        uint32_t *const after = nx[0] ? nx : nullptr;
        uint32_t *const p = (uint32_t *)(uintptr_t)to[4];
        n[0] = p[0];
        ((uint32_t *)(uintptr_t)p[0])[1] = (uint32_t)(uintptr_t)n;
        n[1] = (uint32_t)(uintptr_t)p;
        p[0] = (uint32_t)(uintptr_t)n;
        n = after;
    }
}
static const uint8_t list_take_all_original[] = {0x56, 0x8b, 0x74, 0x24, 0x08, 0x8d, 0x46, 0x0c, 0x89, 0x06};
REWRITE("List::TakeAll", 0x966e40, list_take_all_original, &list_take_all, &g_list_take_alls);

// ---- MeshPart::CopyFrom(from) (0xBE8AA0) ---------------------------------------------------------------------------
// Table 0xFA56F8; the 19 dwords from +4 copied, the use count (word +4) 1; each buffer (+0x30 on, count +0x4C read again
// each time) and the one at +0x1C (when there) count one more use (their word +4). Gives the part.
static volatile long g_mesh_part_copies;
static uint32_t *__thiscall mesh_part_copy_from(uint32_t *self, const uint32_t *from) {
    g_mesh_part_copies++;
    self[0] = 0xfa56f8;
    for (int k = 1; k <= 0x13; k++) self[k] = from[k];
    *(uint16_t *)(self + 1) = 1;
    for (uint32_t i = 0; i < *(volatile uint32_t *)(self + 0x13); i++) (*(uint16_t *)(uintptr_t)(self[0xc + i] + 4))++;
    if (self[7]) (*(uint16_t *)(uintptr_t)(self[7] + 4))++;
    return self;
}
static const uint8_t mesh_part_copy_from_original[] = {0x8b, 0xc1, 0x56, 0x8b, 0x74, 0x24, 0x08, 0x8d, 0x50, 0x04};
REWRITE("MeshPart::CopyFrom", 0xbe8aa0, mesh_part_copy_from_original, &mesh_part_copy_from, &g_mesh_part_copies);

// ---- Table::SumBDC880 (0xBDC880): one past the highest second byte of the table's entries ---------------------------
// Entries of 4 bytes from +0xC (+0x20C bytes of them); a second byte of 0xFF is skipped. 0 when there are none.
static volatile long g_table_highs;
static uint32_t __thiscall table_sum_bdc880(const uint8_t *self) {
    g_table_highs++;
    uint32_t m = 0;
    const uintptr_t end = (uintptr_t)(self + 0xc) + *(const uint32_t *)(self + 0x20c);
    for (uintptr_t p = (uintptr_t)(self + 0xc); p < end; p += 4) {
        const uint32_t b = ((const uint8_t *)p)[1];
        if (b != 0xff && m < b + 1) m = b + 1;
    }
    return m;
}
static const uint8_t table_sum_bdc880_original[] = {0x83, 0xec, 0x0c, 0x56, 0x8b, 0xb1, 0x0c, 0x02, 0x00, 0x00};
REWRITE("Table::SumBDC880", 0xbdc880, table_sum_bdc880_original, &table_sum_bdc880, &g_table_highs);

// ---- Car::SpeedFactor(n) (0x4E4A00) --------------------------------------------------------------------------------
// The game's speed (0x10EE2FC) times 4 when part n (+0x154) is flagged (byte +0x148) and 0x10EE2F8 is set, 1 when its
// +0x144 is set, else -1.
static volatile long g_car_speed_factors;
static long double __thiscall car_speed_factor(const uint8_t *self, int32_t n) {
    g_car_speed_factors++;
    const uint8_t *const part = *(const uint8_t *const *)(self + 0x154 + n * 4);
    const float k = part[0x148] && *(const volatile uint32_t *)0x10ee2f8 ? *(const float *)0xf2669c
                    : *(const uint32_t *)(part + 0x144)                 ? *(const float *)0xf264dc
                                                                        : *(const float *)0xf268f0;
    return (long double)k * *(const volatile float *)0x10ee2fc;
}
static const uint8_t car_speed_factor_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0x84, 0x81, 0x54, 0x01, 0x00, 0x00};
REWRITE("Car::SpeedFactor", 0x4e4a00, car_speed_factor_original, &car_speed_factor, &g_car_speed_factors);

// ---- Park::Copy8EECF0 / 8EED40 / 8EED90 / 8EEDE0(a, b, c) (0x8EECF0 +z, 0x8EED40 -z, 0x8EED90 +x, 0x8EEDE0 -x) ---------
// A record facing one of four ways: +0 = a's first three dwords; the direction (+0xC..+0x14) is the axis (z first, x
// last) with its sign; +0x18 = c.x times the distance along it (+z: a.z - b.y, -z: b.y - a.z, +x: b.x - a.x, -x:
// a.x - b.x) and +0x1C = c.y a.y (floats).
static volatile long g_park_facings;
template <bool X, bool Neg> static void __thiscall park_copy_facing(uint32_t *self, const float *a, const float *b, const float *c) {
    g_park_facings++;
    const uint32_t *const au = (const uint32_t *)a;
    self[0] = au[0], self[1] = au[1], self[2] = au[2];
    const uint32_t one = Neg ? 0xbf800000u : 0x3f800000u;
    self[3] = X ? 0 : one, self[4] = 0, self[5] = X ? one : 0;
    const long double d = X ? (Neg ? (long double)a[0] - b[0] : (long double)b[0] - a[0]) : (Neg ? (long double)b[1] - a[2] : (long double)a[2] - b[1]);
    volatile float *const f = (volatile float *)self;
    f[6] = (float)(d * c[0]);
    f[7] = (float)((long double)c[1] * a[1]);
}
static const uint8_t park_copy_8eecf0_original[] = {0x55, 0x8b, 0xec, 0x8b, 0x45, 0x08, 0x8b, 0x10, 0x89, 0x11, 0x8b, 0x50, 0x04};
REWRITE("Park::Copy8EECF0", 0x8eecf0, park_copy_8eecf0_original, (&park_copy_facing<false, false>), &g_park_facings);
REWRITE("Park::Copy8EED40", 0x8eed40, park_copy_8eecf0_original, (&park_copy_facing<false, true>), &g_park_facings);
REWRITE("Park::Copy8EED90", 0x8eed90, park_copy_8eecf0_original, (&park_copy_facing<true, false>), &g_park_facings);
REWRITE("Park::Copy8EEDE0", 0x8eede0, park_copy_8eecf0_original, (&park_copy_facing<true, true>), &g_park_facings);

// ---- Options::SetFloat(n in ecx, v) (0x4796F0) ---------------------------------------------------------------------
// Number option n (0x10ED860, 20 bytes each: value, .., min +8, max +0xC) = v kept within min..max: v when min is
// below it (or either is a NaN), else min; then that when max is above it, else max.
static volatile long g_option_floats;
static void __thiscall options_set_float(uintptr_t n, float v) {
    g_option_floats++;
    uint8_t *const e = (uint8_t *)0x10ed860 + n * 20;
    const long double lo = *(const float *)(e + 8), hi = *(const float *)(e + 0xc);
    const long double t = lo < v || lo != lo || v != v ? (long double)v : lo;
    *(volatile float *)e = (float)(hi > t ? t : hi);
}
static const uint8_t options_set_float_original[] = {0x83, 0xec, 0x0c, 0x8d, 0x04, 0x89, 0x8b, 0x0c, 0x85, 0x6c, 0xd8, 0x0e, 0x01};
REWRITE("Options::SetFloat", 0x4796f0, options_set_float_original, &options_set_float, &g_option_floats);

// ---- Options::SetInt(n in ecx, v in edx) (0x479630) ----------------------------------------------------------------
// Whole-number option n (0x10ED668, 20 bytes each: value, .., min +8, max +0xC) = v kept within min..max (signed).
static volatile long g_option_ints;
static void __fastcall options_set_int(uintptr_t n, int32_t v) {
    g_option_ints++;
    int32_t *const e = (int32_t *)((uint8_t *)0x10ed668 + n * 20);
    if (v < e[2]) v = e[2];
    if (v > e[3]) v = e[3];
    e[0] = v;
}
static const uint8_t options_set_int_original[] = {0x83, 0xec, 0x10, 0x8d, 0x0c, 0x89, 0x8b, 0x04, 0x8d, 0x74, 0xd6, 0x0e, 0x01};
REWRITE("Options::SetInt", 0x479630, options_set_int_original, &options_set_int, &g_option_ints);

// ---- Effect::Entry408700(n, w, h), Entry408750(n, w, v) (0x408700, 0x408750): a centred rectangle into an entry ---------
// The entry: group n >> 16 (signed) of the list at +0 (each a record whose +0xC is its entries), entry n & 0xFFFF (0x70
// bytes), +0x20. 408700: {-w/2, -h/2, w/2, h/2}; 408750: {-w/2, 0, w/2, v}. Both give the place.
static volatile long g_effect_rects;
static inline float *effect_rect_place(const uint8_t *self, int32_t n) {
    const uint8_t *const group = (*(const uint8_t *const *const *)self)[n >> 16];
    return (float *)(*(uint8_t *const *)(group + 0xc) + (n & 0xffff) * 0x70 + 0x20);
}
static float *__thiscall effect_entry_408700(const uint8_t *self, int32_t n, float w, float h) {
    g_effect_rects++;
    const float half = *(const float *)0xf2651c;
    const long double hw = (long double)w * half, hh = (long double)h * half;
    volatile float *const p = effect_rect_place(self, n);
    p[0] = (float)-hw, p[1] = (float)-hh, p[2] = (float)hw, p[3] = (float)hh;
    return (float *)p;
}
static float *__thiscall effect_entry_408750(const uint8_t *self, int32_t n, float w, uint32_t v) {
    g_effect_rects++;
    const long double hw = (long double)w * *(const float *)0xf2651c;
    volatile float *const p = effect_rect_place(self, n);
    ((volatile uint32_t *)p)[1] = 0, ((volatile uint32_t *)p)[3] = v;
    p[0] = (float)-hw, p[2] = (float)hw;
    return (float *)p;
}
static const uint8_t effect_entry_408700_original[] = {0x8b, 0x44, 0x24, 0x04, 0xd9, 0x44, 0x24, 0x08, 0xd8, 0x0d, 0x1c, 0x65, 0xf2, 0x00};
REWRITE("Effect::Entry408700", 0x408700, effect_entry_408700_original, &effect_entry_408700, &g_effect_rects);
static const uint8_t effect_entry_408750_original[] = {0x8b, 0x44, 0x24, 0x04, 0xd9, 0x44, 0x24, 0x08, 0x8b, 0x09};
REWRITE("Effect::Entry408750", 0x408750, effect_entry_408750_original, &effect_entry_408750, &g_effect_rects);

// ---- Effect::Round441270, Ride::Round5F97A0 (0x441270 by 4, 0x5F97A0 by 32) -----------------------------------------
// floor((+0x48 - +0x54) K) (K 0xF28CF8; a float first), divided by 4 or 32 (as C divides).
static volatile long g_round_steps;
template <int32_t Div> static int32_t __thiscall round_step(const uint8_t *self) {
    g_round_steps++;
    volatile float d = (float)(((long double)*(const float *)(self + 0x48) - *(const float *)(self + 0x54)) * *(const float *)0xf28cf8);
    volatile float fl = (float)floor_h2(d);
    return (int32_t)x87_fistp64_h2(fl) / Div;
}
static const uint8_t round_step_original[] = {0x83, 0xec, 0x0c, 0xd9, 0x41, 0x48, 0xd8, 0x61, 0x54, 0xd8, 0x0d, 0xf8, 0x8c, 0xf2, 0x00};
REWRITE("Effect::Round441270", 0x441270, round_step_original, &round_step<4>, &g_round_steps);
REWRITE("Ride::Round5F97A0", 0x5f97a0, round_step_original, &round_step<32>, &g_round_steps);

// ---- Window::ScaleA45620(p), ScaleA455D0(out, p) (0xA45620 in place, 0xA455D0 into out) ------------------------------
// x by the width ((+0x84 - +0x7C) / 1024) and y by the height ((+0x88 - +0x80) K, K 0xF28568; a float in A455D0).
static volatile long g_window_scales;
static void __thiscall window_scale_a45620(const uint8_t *self, float *p) {
    g_window_scales++;
    auto at = [&](uint32_t o) { return *(const float *)(self + o); };
    volatile float x = (float)(((long double)at(0x84) - at(0x7c)) * *(const float *)0xf2856c * p[0]);
    volatile float y = (float)(((long double)at(0x88) - at(0x80)) * *(const float *)0xf28568 * p[1]);
    volatile float *const o = p;
    o[0] = x;
    o[1] = y;
}
static float *__thiscall window_scale_a455d0(const uint8_t *self, float *out, const float *p) {
    g_window_scales++;
    auto at = [&](uint32_t o) { return *(const float *)(self + o); };
    const long double y = p[1];
    const long double w = ((long double)at(0x84) - at(0x7c)) * *(const float *)0xf2856c;
    volatile float h = (float)(((long double)at(0x88) - at(0x80)) * *(const float *)0xf28568);
    volatile float *const o = out;
    o[0] = (float)(w * p[0]);
    o[1] = (float)((long double)h * y);
    return out;
}
static const uint8_t window_scale_a45620_original[] = {0x83, 0xec, 0x08, 0xd9, 0x81, 0x84, 0x00, 0x00, 0x00, 0x8b, 0x44, 0x24, 0x0c};
REWRITE("Window::ScaleA45620", 0xa45620, window_scale_a45620_original, &window_scale_a45620, &g_window_scales);
static const uint8_t window_scale_a455d0_original[] = {0x8b, 0x44, 0x24, 0x08, 0xd9, 0x40, 0x04, 0xd9, 0x81, 0x84, 0x00, 0x00, 0x00};
REWRITE("Window::ScaleA455D0", 0xa455d0, window_scale_a455d0_original, &window_scale_a455d0, &g_window_scales);

// ---- Waterfall::Line917540(p) (0x917540): a plane's value at a point --------------------------------------------------
// With the normal n (+0x10) and base b (+0): ((((n.z p.z + n.y p.y) - n.z b.z) - n.y b.y) - n.x b.x) + n.x p.x.
static volatile long g_waterfall_lines;
static long double __thiscall waterfall_line_917540(const float *self, const float *p) {
    g_waterfall_lines++;
    const long double a = self[4], b = self[5], c = self[6];
    long double r = (long double)self[6] * p[2] + (long double)self[5] * p[1];
    r = r + -c * self[2];
    r = r + -b * self[1];
    r = r + -a * self[0];
    return r + (long double)self[4] * p[0];
}
static const uint8_t waterfall_line_917540_original[] = {0x55, 0x8b, 0xec, 0xd9, 0x41, 0x10, 0x8b, 0x45, 0x08, 0xd9, 0xe0};
REWRITE("Waterfall::Line917540", 0x917540, waterfall_line_917540_original, &waterfall_line_917540, &g_waterfall_lines);

// ---- Terrain::Untotal8FDA10 (0x8FDA10) -----------------------------------------------------------------------------
// Takes its two counts (+0x14, +0x18) out of the game's totals (0x133A028, 0x133A02C), sets them from the size n of its
// part (+0xC: +0xC: +0): 2 n and 6 n - 6, and adds them back in.
static volatile long g_terrain_untotals;
static void __thiscall terrain_untotal_8fda10(uint8_t *self) {
    g_terrain_untotals++;
    volatile int32_t *const total = (volatile int32_t *)0x133a028;
    int32_t *const c = (int32_t *)(self + 0x14);
    total[0] -= c[0];
    total[1] -= c[1];
    const int32_t n = **(const int32_t *const *)(*(const uint8_t *const *)(self + 0xc) + 0xc);
    c[1] = 6 * n - 6;
    c[0] = 2 * n;
    total[0] += c[0];
    total[1] += c[1];
}
static const uint8_t terrain_untotal_8fda10_original[] = {0x8b, 0x41, 0x14, 0x8b, 0x15, 0x28, 0xa0, 0x33, 0x01};
REWRITE("Terrain::Untotal8FDA10", 0x8fda10, terrain_untotal_8fda10_original, &terrain_untotal_8fda10, &g_terrain_untotals);

// ---- WildAnimal::SetPointB2F0C0(p) (0xB2F0C0) ----------------------------------------------------------------------
// The point goes to +0x90; the heading (+0x80) = atan2(x, z) (fpatan), 2 pi (a float) added when below 0.
static volatile long g_animal_headings;
static inline long double x87_atan2_h2(long double y, long double x) {
    long double r;
    __asm__("fpatan" : "=t"(r) : "0"(x), "u"(y) : "st(1)");
    return r;
}
static void __thiscall wild_animal_set_point_b2f0c0(uint8_t *self, const uint32_t *p) {
    g_animal_headings++;
    uint32_t *const u = (uint32_t *)(self + 0x90);
    u[0] = p[0], u[1] = p[1], u[2] = p[2];
    long double a = x87_atan2_h2(*(const float *)(self + 0x90), *(const float *)(self + 0x98));
    if (a < *(const float *)0xf264e0) a = a + *(const float *)0xf268dc;
    *(volatile float *)(self + 0x80) = (float)a;
}
static const uint8_t wild_animal_set_point_b2f0c0_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0x10, 0x89, 0x91, 0x90, 0x00, 0x00, 0x00};
REWRITE("WildAnimal::SetPointB2F0C0", 0xb2f0c0, wild_animal_set_point_b2f0c0_original, &wild_animal_set_point_b2f0c0, &g_animal_headings);

// ---- Window::SetPointC5DED0(w, h, p) (0xC5DED0) --------------------------------------------------------------------
// The point goes to +0xD0, and its parts over w and h (whole numbers) to +0x194, +0x198.
static volatile long g_window_points;
static void __thiscall window_set_point_c5ded0(uint8_t *self, int32_t w, int32_t h, const float *p) {
    g_window_points++;
    uint32_t *const u = (uint32_t *)(self + 0xd0);
    u[0] = ((const uint32_t *)p)[0], u[1] = ((const uint32_t *)p)[1];
    *(volatile float *)(self + 0x194) = (float)(p[0] / (long double)w);
    *(volatile float *)(self + 0x198) = (float)(p[1] / (long double)h);
}
static const uint8_t window_set_point_c5ded0_original[] = {0x83, 0xec, 0x08, 0x8b, 0x44, 0x24, 0x14, 0xdb, 0x44, 0x24, 0x0c};
REWRITE("Window::SetPointC5DED0", 0xc5ded0, window_set_point_c5ded0_original, &window_set_point_c5ded0, &g_window_points);

// ---- Lighting::DownwardDirection(out) (0x4261F0) -------------------------------------------------------------------
// The vector at +0x120 with its y negated when it is above 0. Gives out.
static volatile long g_light_downs;
static float *__thiscall lighting_downward_direction(const uint8_t *self, float *out) {
    g_light_downs++;
    const float *const v = (const float *)(self + 0x120);
    long double y = v[1];
    if (y > *(const float *)0xf264e0) y = -y;
    volatile float *const o = out;
    o[0] = v[0], o[1] = (float)y, o[2] = v[2];
    return out;
}
static const uint8_t lighting_downward_direction_original[] = {0x55, 0x8b, 0xec, 0xd9, 0x81, 0x20, 0x01, 0x00, 0x00};
REWRITE("Lighting::DownwardDirection", 0x4261f0, lighting_downward_direction_original, &lighting_downward_direction, &g_light_downs);

// ---- CustomPersistingSpline::EndDirection(out, -) (0x424D50) -------------------------------------------------------
// The last point's (36 bytes each at +4, count +8) vector at +0xC, reversed. Gives out.
static volatile long g_spline_ends;
static float *__thiscall spline_end_direction(const uint8_t *self, float *out, uint32_t) {
    g_spline_ends++;
    const float *const v = (const float *)(*(const uint8_t *const *)(self + 4) + *(const int32_t *)(self + 8) * 36 - 0x18);
    volatile float *const o = out;
    o[0] = (float)-(long double)v[0], o[1] = (float)-(long double)v[1], o[2] = (float)-(long double)v[2];
    return out;
}
static const uint8_t spline_end_direction_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x10, 0x8b, 0x41, 0x08};
REWRITE("CustomPersistingSpline::EndDirection", 0x424d50, spline_end_direction_original, &spline_end_direction, &g_spline_ends);

// ---- Rect::Contains(p) (0xBE7780) ----------------------------------------------------------------------------------
// Whether left <= x <= right and top <= y <= bottom (the rect {left, top, right, bottom}; a NaN gives 0).
static volatile long g_rect_contains;
static uint32_t __thiscall rect_contains(const float *self, const float *p) {
    g_rect_contains++;
    return self[0] <= p[0] && self[2] >= p[0] && self[1] <= p[1] && self[3] >= p[1] ? 1 : 0;
}
static const uint8_t rect_contains_original[] = {0xd9, 0x01, 0x8b, 0x54, 0x24, 0x04, 0xd8, 0x1a, 0xdf, 0xe0};
REWRITE("Rect::Contains", 0xbe7780, rect_contains_original, &rect_contains, &g_rect_contains);

// ---- HashTable::FindBD65D0(key) (0xBD65D0) -------------------------------------------------------------------------
// The key's first dword k; its bucket k % +4 (unsigned) runs from buckets[b] to buckets[b + 1] (+0xC); entry i (dwords
// at +0x10) equal to k gives +0x14 + 2 i, none 0.
static volatile long g_hash_finds;
static uint16_t *__thiscall hash_table_find_bd65d0(const uint8_t *self, const uint32_t *key) {
    g_hash_finds++;
    const uint32_t k = key[0];
    const uint32_t *const bucket = *(const uint32_t *const *)(self + 0xc) + k % *(const uint32_t *)(self + 4);
    const uint32_t *const keys = *(const uint32_t *const *)(self + 0x10);
    for (uint32_t i = bucket[0]; i < bucket[1]; i++)
        if (keys[i] == k) return (uint16_t *)(*(uint8_t *const *)(self + 0x14) + i * 2);
    return nullptr;
}
static const uint8_t hash_table_find_bd65d0_original[] = {0x8b, 0x44, 0x24, 0x04, 0x56, 0x57, 0x8b, 0x38, 0x33, 0xd2};
REWRITE("HashTable::FindBD65D0", 0xbd65d0, hash_table_find_bd65d0_original, &hash_table_find_bd65d0, &g_hash_finds);

// ---- PointerArray::RemoveSwap2(v) (0x80ED70) -----------------------------------------------------------------------
// The first entry equal to v (array +0x5C, count +0x64 read again each time) gets the last one (by the count as first
// read), and the count goes down by one.
static volatile long g_pointer_removes;
static void __thiscall pointer_array_remove_swap2(uint8_t *self, uint32_t v) {
    g_pointer_removes++;
    const int32_t count = *(const int32_t *)(self + 0x64);
    uint32_t *const a = *(uint32_t *const *)(self + 0x5c);
    for (int32_t i = 0; i < *(volatile int32_t *)(self + 0x64); i++)
        if (a[i] == v) {
            a[i] = a[count - 1];
            *(int32_t *)(self + 0x64) -= 1;
            return;
        }
}
static const uint8_t pointer_array_remove_swap2_original[] = {0x57, 0x8b, 0x79, 0x64, 0x33, 0xc0, 0x85, 0xff};
REWRITE("PointerArray::RemoveSwap2", 0x80ed70, pointer_array_remove_swap2_original, &pointer_array_remove_swap2, &g_pointer_removes);

// ---- Record::InitC212D0(p, v) (0xC212D0) ---------------------------------------------------------------------------
// +0xC, +0x1C, +0x2C = the largest float, +0x38 = v, +0x3C = 0, +0 = p's three dwords (zeros with no p). Gives it.
static volatile long g_record_inits;
static uint32_t *__thiscall record_init_c212d0(uint32_t *self, const uint32_t *p, uint32_t v) {
    g_record_inits++;
    self[3] = self[7] = self[11] = 0x7f7fffff;
    self[14] = v;
    self[15] = 0;
    if (p) self[0] = p[0], self[1] = p[1], self[2] = p[2];
    else self[0] = self[1] = self[2] = 0;
    return self;
}
static const uint8_t record_init_c212d0_original[] = {0x8b, 0xc1, 0xb9, 0xff, 0xff, 0x7f, 0x7f, 0x89, 0x48, 0x0c};
REWRITE("Record::InitC212D0", 0xc212d0, record_init_c212d0_original, &record_init_c212d0, &g_record_inits);

// ---- Animal::ScaleB16D60 (0xB16D60) --------------------------------------------------------------------------------
// t = +0x10 K (K 0xF85E78, a float) kept within 0..1.5 the FPU's way: t when 0 is below it (or t is a NaN), else 0;
// then that when 1.5 is above it, else 1.5 (so a NaN gives 1.5).
static volatile long g_animal_scales;
static long double __thiscall animal_scale_b16d60(const uint8_t *self) {
    g_animal_scales++;
    volatile float t = (float)((long double)*(const float *)(self + 0x10) * *(const float *)0xf85e78);
    const long double lo = 0.0L, hi = 1.5L;
    const long double m = lo < t || t != t ? (long double)t : lo;
    volatile float r = (float)(hi > m ? m : hi);
    return r;
}
static const uint8_t animal_scale_b16d60_original[] = {0x83, 0xec, 0x10, 0xd9, 0x41, 0x10, 0xc7, 0x04, 0x24, 0x00, 0x00, 0xc0, 0x3f};
REWRITE("Animal::ScaleB16D60", 0xb16d60, animal_scale_b16d60_original, &animal_scale_b16d60, &g_animal_scales);

// ---- Window::Width670620 (0x670620) --------------------------------------------------------------------------------
// The part's width ((+0x134: +0x84 - +0x7C) plus the +0x108 part's +0x84, a float), at least 290 (290 for a NaN).
static volatile long g_window_widths;
static long double __thiscall window_width_670620(const uint8_t *self) {
    g_window_widths++;
    const uint8_t *const p = *(const uint8_t *const *)(self + 0x134), *const q = *(const uint8_t *const *)(self + 0x108);
    volatile float w = (float)(((long double)*(const float *)(p + 0x84) - *(const float *)(p + 0x7c)) + *(const float *)(q + 0x84));
    volatile float r = w < 290.0f || w != w ? 290.0f : (float)w;
    return r;
}
static const uint8_t window_width_670620_original[] = {0x83, 0xec, 0x0c, 0x8b, 0x81, 0x34, 0x01, 0x00, 0x00, 0xd9, 0x80, 0x84, 0x00, 0x00, 0x00};
REWRITE("Window::Width670620", 0x670620, window_width_670620_original, &window_width_670620, &g_window_widths);

// ---- Shader::TakeCC2C30(out, -) (0xCC2C30): one more taken (+0x1000), out cleared. Gives the shader. -----------------
static volatile long g_shader_takes;
static uint8_t *__thiscall shader_take_cc2c30(uint8_t *self, uint32_t *out, uint32_t) {
    g_shader_takes++;
    *(int32_t *)(self + 0x1000) += 1;
    *out = 0;
    return self;
}
static const uint8_t shader_take_cc2c30_original[] = {0x55, 0x8b, 0xec, 0x83, 0xec, 0x08, 0xc7, 0x45, 0xf8, 0xcc, 0xcc, 0xcc, 0xcc};
REWRITE("Shader::TakeCC2C30", 0xcc2c30, shader_take_cc2c30_original, &shader_take_cc2c30, &g_shader_takes);

// ---- Bits::ResetE04110 (0xE04110): a new version of a bit set --------------------------------------------------------
// The version (+8) goes up by one; past 0x40000000 it starts again at 1 and the words (+0xC, count +0 read again each
// time) are cleared. +0x14 = 0 either way.
static volatile long g_bit_resets;
static void __thiscall bits_reset_e04110(uint8_t *self) {
    g_bit_resets++;
    int32_t *const version = (int32_t *)(self + 8);
    if (*version > 0x40000000) {
        *version = 1;
        for (int32_t i = 0; i < *(volatile int32_t *)self; i++) (*(uint32_t *const *)(self + 0xc))[i] = 0;
    } else
        *version += 1;
    *(uint32_t *)(self + 0x14) = 0;
}
static const uint8_t bits_reset_e04110_original[] = {0x8b, 0x41, 0x08, 0x3d, 0x00, 0x00, 0x00, 0x40, 0x7e, 0x28};
REWRITE("Bits::ResetE04110", 0xe04110, bits_reset_e04110_original, &bits_reset_e04110, &g_bit_resets);

// ---- Bits::TestC165F0(n) (0xC165F0): sets bit n, counting it (+0x14) when it was clear --------------------------------
// Word n / 32 and bit n % 32 (as C divides); nothing outside the words (+0xC, count +0x10). Gives the word index when
// outside, else the bit's mask.
static volatile long g_bit_sets;
static uint32_t __thiscall bits_test_c165f0(uint8_t *self, int32_t n) {
    g_bit_sets++;
    const int32_t w = n / 32, b = n % 32;
    if (w < 0 || w >= *(const int32_t *)(self + 0x10)) return (uint32_t)w;
    uint32_t *const word = *(uint32_t *const *)(self + 0xc) + w;
    const uint32_t mask = 1u << (b & 31);
    if (!(*word & mask)) *(int32_t *)(self + 0x14) += 1;
    *word |= mask;
    return mask;
}
static const uint8_t bits_test_c165f0_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x4c, 0x24, 0x08, 0x8b, 0xc1, 0x99};
REWRITE("Bits::TestC165F0", 0xc165f0, bits_test_c165f0_original, &bits_test_c165f0, &g_bit_sets);

// ---- Bits::SizeBB65A0(words in ecx, n in edx) (0xBB65A0): the first n bits set ----------------------------------------
// The whole words to 0xFFFFFFFF, then the bits of the last part one by one. Gives n.
static volatile long g_bit_fills;
static uint32_t __fastcall bits_size_bb65a0(uint32_t *words, uint32_t n) {
    g_bit_fills++;
    const uint32_t whole = n >> 5;
    for (uint32_t i = 0; i < whole; i++) words[i] = 0xffffffff;
    for (uint32_t i = whole << 5; i < n; i++) words[i >> 5] |= 1u << (i & 31);
    return n;
}
static const uint8_t bits_size_bb65a0_original[] = {0x53, 0x56, 0x8b, 0xd9, 0x8b, 0xf2, 0xc1, 0xee, 0x05, 0x57};
REWRITE("Bits::SizeBB65A0", 0xbb65a0, bits_size_bb65a0_original, &bits_size_bb65a0, &g_bit_fills);

// ---- Text::CopyBFC290(from) (0xBFC290) -----------------------------------------------------------------------------
// from's +0x10 into +0x5C and +0x74; the mode (+0x7C) 0 when it is 0, else 6 (7 when the game's byte 0x13D3175 is
// clear), with 8 added when from's byte +0x18 is set. Gives from.
static volatile long g_text_copies;
static const uint8_t *__thiscall text_copy_bfc290(uint8_t *self, const uint8_t *from) {
    g_text_copies++;
    uint32_t *const u = (uint32_t *)self;
    const uint32_t v = *(const uint32_t *)(from + 0x10);
    u[0x5c / 4] = v, u[0x74 / 4] = v;
    if (!v) {
        u[0x7c / 4] = 0;
        return from;
    }
    u[0x7c / 4] = *(const volatile uint8_t *)0x13d3175 ? 6 : 7;
    if (from[0x18]) u[0x7c / 4] |= 8;
    return from;
}
static const uint8_t text_copy_bfc290_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0x50, 0x10, 0x89, 0x51, 0x5c};
REWRITE("Text::CopyBFC290", 0xbfc290, text_copy_bfc290_original, &text_copy_bfc290, &g_text_copies);

// ---- Window::Init6AF710(owner, list) (0x6AF710) --------------------------------------------------------------------
// +8 and +0x14 point at itself, +0x18 = owner, +0x1C = list; its node at +0xC goes in at the front of the list's
// (+0x14) nodes, and itself at the front of the owner's (+0x34) (nodes {next, back}). Gives it.
static volatile long g_window_inits;
static uint32_t *__thiscall window_init_6af710(uint32_t *self, const uint8_t *owner, const uint8_t *list) {
    g_window_inits++;
    self[2] = (uint32_t)(uintptr_t)self;
    self[5] = (uint32_t)(uintptr_t)self;
    self[6] = (uint32_t)(uintptr_t)owner;
    self[7] = (uint32_t)(uintptr_t)list;
    auto push_front = [](uint32_t *head, uint32_t *node) {
        node[0] = head[0];
        ((uint32_t *)(uintptr_t)head[0])[1] = (uint32_t)(uintptr_t)node;
        node[1] = (uint32_t)(uintptr_t)head;
        head[0] = (uint32_t)(uintptr_t)node;
    };
    push_front(*(uint32_t *const *)(list + 0x14), self + 3);
    push_front(*(uint32_t *const *)((const uint8_t *)(uintptr_t)self[6] + 0x34), self);
    return self;
}
static const uint8_t window_init_6af710_original[] = {0x8b, 0x54, 0x24, 0x08, 0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x04};
REWRITE("Window::Init6AF710", 0x6af710, window_init_6af710_original, &window_init_6af710, &g_window_inits);

// ---- TrackImporter::OppositeDirection (0xA3FA60): of eight directions (in eax), the opposite --------------------------
// From a table on the stack {4, 5, 6, 7, 0, 1, 2, 3} (the game reads past it for others).
static volatile long g_opposite_directions;
extern "C" uint32_t __stdcall forever_opposite_direction(uint32_t d) {
    g_opposite_directions++;
    static const uint32_t table[8] = {4, 5, 6, 7, 0, 1, 2, 3};
    return table[d & 7];
}
extern "C" __attribute__((naked)) void forever_opposite_direction_regs() {   // d in eax
    __asm__("pushl %eax\n\tcalll _forever_opposite_direction@4\n\tret");
}
static const uint8_t opposite_direction_original[] = {0x83, 0xec, 0x20, 0xc7, 0x04, 0x24, 0x04, 0x00, 0x00, 0x00};
REWRITE("TrackImporter::OppositeDirection", 0xa3fa60, opposite_direction_original, (void *)&forever_opposite_direction_regs, &g_opposite_directions);

// ---- Ride::Near58CD40(r2) (0x58CD40): whether two points (ecx, eax) are within a squared distance ----------------------
// (dx^2 + dz^2) + dy^2 not above r2 (a NaN gives 0).
static volatile long g_ride_nears;
extern "C" uint32_t __stdcall forever_ride_near(const float *a, const float *b, float r2) {
    g_ride_nears++;
    const long double dx = (long double)a[0] - b[0], dy = (long double)a[1] - b[1], dz = (long double)a[2] - b[2];
    return (dx * dx + dz * dz) + dy * dy <= r2 ? 1 : 0;
}
extern "C" __attribute__((naked)) void forever_ride_near_regs() {   // a ecx, b eax; r2 on the stack
    __asm__("pushl 4(%esp)\n\tpushl %eax\n\tpushl %ecx\n\tcalll _forever_ride_near@12\n\tret $4");
}
static const uint8_t ride_near_58cd40_original[] = {0x55, 0x8b, 0xec, 0xd9, 0x01, 0x83, 0xe4, 0xf0, 0xd8, 0x20};
REWRITE("Ride::Near58CD40", 0x58cd40, ride_near_58cd40_original, (void *)&forever_ride_near_regs, &g_ride_nears);

// ---- Vector::StepC1E790(p) (0xC1E790): a plane made of length 1 ------------------------------------------------------
// i = 1 / |p.xyz| ((x^2 + y^2) + z^2) as a float; the record's +0 = p.xyz i and +0x10 = i p.w. Gives p.
static volatile long g_plane_units;
static const float *__thiscall vector_step_c1e790(uint8_t *self, const float *p) {
    g_plane_units++;
    const long double x = p[0], y = p[1], z = p[2];
    volatile float inv = (float)((long double)*(const float *)0xf264dc / x87_sqrt_h2((x * x + y * y) + z * z));
    volatile float *const f = (volatile float *)self;
    f[0] = (float)(x * inv);
    f[1] = (float)(y * inv);
    f[2] = (float)(z * inv);
    f[4] = (float)((long double)inv * p[3]);
    return p;
}
static const uint8_t vector_step_c1e790_original[] = {0x8b, 0x44, 0x24, 0x04, 0xd9, 0x40, 0x08, 0xd9, 0x40, 0x04, 0xd9, 0x00};
REWRITE("Vector::StepC1E790", 0xc1e790, vector_step_c1e790_original, &vector_step_c1e790, &g_plane_units);

// ---- Ride::Value576B00 (0x576B00) ----------------------------------------------------------------------------------
// t = +8 when below a half, else 1 - +8; t (1, or -1 for modes 0 and 2 at +4) K (K 0xF3B7FC).
static volatile long g_ride_values;
static long double __thiscall ride_value_576b00(const uint8_t *self) {
    g_ride_values++;
    const float v = *(const float *)(self + 8);
    const long double t = v < *(const float *)0xf2651c ? (long double)v : (long double)*(const float *)0xf264dc - v;
    const int32_t mode = *(const int32_t *)(self + 4);
    const float s = mode == 2 || mode == 0 ? *(const float *)0xf268f0 : *(const float *)0xf264dc;
    return (long double)s * t * *(const float *)0xf3b7fc;
}
static const uint8_t ride_value_576b00_original[] = {0xd9, 0x41, 0x08, 0xd8, 0x1d, 0x1c, 0x65, 0xf2, 0x00};
REWRITE("Ride::Value576B00", 0x576b00, ride_value_576b00_original, &ride_value_576b00, &g_ride_values);

// ---- Window::Fraction660630(p) (0x660630) --------------------------------------------------------------------------
// floor(((+0x160 - +0x158) - p.y) / +0x16C) (a float first) as a whole number.
static volatile long g_window_fractions;
static int32_t __thiscall window_fraction_660630(const uint8_t *self, const float *p) {
    g_window_fractions++;
    auto at = [&](uint32_t o) { return *(const float *)(self + o); };
    volatile float q = (float)((((long double)at(0x160) - at(0x158)) - p[1]) / at(0x16c));
    volatile float fl = (float)floor_h2(q);
    return (int32_t)x87_fistp64_h2(fl);
}
static const uint8_t window_fraction_660630_original[] = {0x83, 0xec, 0x08, 0xd9, 0x81, 0x60, 0x01, 0x00, 0x00};
REWRITE("Window::Fraction660630", 0x660630, window_fraction_660630_original, &window_fraction_660630, &g_window_fractions);

// ---- Effect::Colour413C70(r, g, b) (0x413C70) ----------------------------------------------------------------------
// Each part times 255 rounded by the FPU (low 32 bits), packed as ((((r | 0xFFFFFF00) << 8) | g) << 8) | b into +0
// (whole words ored in, as the game does). Gives it.
static volatile long g_effect_colours;
static uint32_t __thiscall effect_colour_413c70(uint32_t *self, float r, float g, float b) {
    g_effect_colours++;
    const float k = *(const float *)0xf264c0;
    uint32_t c = ((uint32_t)x87_fistp64_h2((long double)r * k) | 0xffffff00u) << 8;
    c = (c | (uint32_t)x87_fistp64_h2((long double)g * k)) << 8;
    c |= (uint32_t)x87_fistp64_h2((long double)b * k);
    *self = c;
    return c;
}
static const uint8_t effect_colour_413c70_original[] = {0x83, 0xec, 0x08, 0xd9, 0x44, 0x24, 0x0c, 0xd8, 0x0d, 0xc0, 0x64, 0xf2, 0x00};
REWRITE("Effect::Colour413C70", 0x413c70, effect_colour_413c70_original, &effect_colour_413c70, &g_effect_colours);

// ---- Track::MoveBC53D0(a, b, c) (0xBC53D0): moves a position by whole steps --------------------------------------------
// The position (+0x10) += {a X, c Y, b Z} with the game's step sizes X, Y, Z (0x10F2B30.., the z product a float).
static volatile long g_track_moves;
static void __thiscall track_move_bc53d0(uint8_t *self, int32_t a, int32_t b, int32_t c) {
    g_track_moves++;
    const volatile float *const step = (const volatile float *)0x10f2b30;
    const long double x = (long double)a * step[0], y = (long double)c * step[1];
    volatile float z = (float)((long double)b * step[2]);
    volatile float *const p = (volatile float *)(self + 0x10);
    p[0] = (float)(x + p[0]);
    p[1] = (float)(y + p[1]);
    p[2] = (float)((long double)z + p[2]);
}
static const uint8_t track_move_bc53d0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x10, 0xdb, 0x45, 0x08};
REWRITE("Track::MoveBC53D0", 0xbc53d0, track_move_bc53d0_original, &track_move_bc53d0, &g_track_moves);

// ---- WildAnimal::AddB23940(n) (0xB23940): +0 += n, kept within 0..1 the FPU's way (a NaN gives 1) ----------------------
static volatile long g_animal_adds;
static void __thiscall wild_animal_add_b23940(float *self, float n) {
    g_animal_adds++;
    volatile float t = (float)((long double)n + self[0]);
    const long double m = 0.0L < t || t != t ? (long double)t : 0.0L;
    *(volatile float *)self = (float)(1.0L > m ? m : 1.0L);
}
static const uint8_t wild_animal_add_b23940_original[] = {0x83, 0xec, 0x0c, 0xd9, 0x44, 0x24, 0x10, 0xc7, 0x44, 0x24, 0x10, 0x00, 0x00, 0x80, 0x3f};
REWRITE("WildAnimal::AddB23940", 0xb23940, wild_animal_add_b23940_original, &wild_animal_add_b23940, &g_animal_adds);

// ---- Track::ValueBC5E20 (0xBC5E20): the larger of two parts' values -------------------------------------------------
// The parts +0x20 and +0x24 (their +0x1C): with only one there, its value; with both the FPU's max (the first's for a
// NaN). (With neither the game reads through 0.)
static volatile long g_track_values;
static long double __thiscall track_value_bc5e20(const uint8_t *self) {
    g_track_values++;
    const uint8_t *const a = *(const uint8_t *const *)(self + 0x20), *const b = *(const uint8_t *const *)(self + 0x24);
    if (!a) return *(const float *)(b + 0x1c);
    if (!b) return *(const float *)(a + 0x1c);
    const float y = *(const float *)(a + 0x1c), x = *(const float *)(b + 0x1c);
    volatile float r = x < y || x != x || y != y ? y : x;
    return r;
}
static const uint8_t track_value_bc5e20_original[] = {0x8b, 0x41, 0x20, 0x8b, 0x49, 0x24, 0x83, 0xec, 0x0c, 0x85, 0xc0};
REWRITE("Track::ValueBC5E20", 0xbc5e20, track_value_bc5e20_original, &track_value_bc5e20, &g_track_values);

// ---- Ride::Point4E5D40(out) (0x4E5D40): the last point's (36 bytes each at +4, count +0) vector +0xC, reversed ----------
static volatile long g_ride_ends;
static float *__thiscall ride_point_4e5d40(const uint8_t *self, float *out) {
    g_ride_ends++;
    const float *const v = (const float *)(*(const uint8_t *const *)(self + 4) + *(const int32_t *)self * 36 - 0x18);
    volatile float *const o = out;
    o[0] = (float)-(long double)v[0], o[1] = (float)-(long double)v[1], o[2] = (float)-(long double)v[2];
    return out;
}
static const uint8_t ride_point_4e5d40_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x10, 0x8b, 0x01, 0x8b, 0x49, 0x04};
REWRITE("Ride::Point4E5D40", 0x4e5d40, ride_point_4e5d40_original, &ride_point_4e5d40, &g_ride_ends);

// ---- Ride::Point4BB4C0(out, p) (0x4BB4C0): out = +0x7C + (p + +0x8C) (each sum unrounded first). Gives out. -----------
static volatile long g_ride_offsets;
static float *__thiscall ride_point_4bb4c0(const uint8_t *self, float *out, const float *p) {
    g_ride_offsets++;
    auto at = [&](uint32_t o) { return *(const float *)(self + o); };
    const long double a = (long double)p[0] + at(0x8c), b = (long double)at(0x90) + p[1];
    volatile float *const o = out;
    o[0] = (float)(at(0x7c) + a);
    o[1] = (float)(at(0x80) + b);
    return out;
}
static const uint8_t ride_point_4bb4c0_original[] = {0x83, 0xec, 0x08, 0x8b, 0x44, 0x24, 0x10, 0xd9, 0x00, 0xd8, 0x81, 0x8c, 0x00, 0x00, 0x00};
REWRITE("Ride::Point4BB4C0", 0x4bb4c0, ride_point_4bb4c0_original, &ride_point_4bb4c0, &g_ride_offsets);

// ---- Text::Value49E0B0(p, out) (0x49E0B0): a swing from two points' sum --------------------------------------------
// t = ((p.z + p.x) A + +0xC) B (A 0xF305AC, B 0xF305A8); s = sin t (unrounded t) +0x10; with out, out = {cos t (t as a
// float) +0x10, s}. Gives s C (C 0xF2A8C4).
static volatile long g_text_swings;
static long double __thiscall text_value_49e0b0(const uint8_t *self, const float *p, float *out) {
    g_text_swings++;
    const float amp = *(const float *)(self + 0x10);
    const long double t = (((long double)p[2] + p[0]) * *(const float *)0xf305ac + *(const float *)(self + 0xc)) * *(const float *)0xf305a8;
    volatile float tf = (float)t;
    const long double s = x87_sin_h2(t) * amp;
    if (out) {
        volatile float *const o = out;
        o[0] = (float)(x87_cos_h2(tf) * amp);
        o[1] = (float)s;
    }
    return s * *(const float *)0xf2a8c4;
}
static const uint8_t text_value_49e0b0_original[] = {0x8b, 0x44, 0x24, 0x04, 0xd9, 0x40, 0x08, 0xd8, 0x00, 0x8b, 0x44, 0x24, 0x08};
REWRITE("Text::Value49E0B0", 0x49e0b0, text_value_49e0b0_original, &text_value_49e0b0, &g_text_swings);

// ---- Track::ScaleBC7F90(out, mode) (0xBC7F90): a part's size in world units -----------------------------------------
// The part (+4): a = +0x10 X and b = +0x14 Z (whole numbers, the game's step sizes 0x10F2B30 and 0x10F2B38; floats).
// out = {a, b}, or {b, a} for modes 1 and 3. Gives out.
static volatile long g_track_scales;
static float *__thiscall track_scale_bc7f90(const uint8_t *self, float *out, int32_t mode) {
    g_track_scales++;
    const uint8_t *const p = *(const uint8_t *const *)(self + 4);
    volatile float a = (float)((long double)*(const int32_t *)(p + 0x10) * *(const volatile float *)0x10f2b30);
    volatile float b = (float)((long double)*(const int32_t *)(p + 0x14) * *(const volatile float *)0x10f2b38);
    volatile float *const o = out;
    if (mode == 1 || mode == 3) o[0] = b, o[1] = a;
    else o[0] = a, o[1] = b;
    return out;
}
static const uint8_t track_scale_bc7f90_original[] = {0x83, 0xec, 0x08, 0x8b, 0x41, 0x04, 0xdb, 0x40, 0x10, 0xd8, 0x0d, 0x30, 0x2b, 0x0f, 0x01};
REWRITE("Track::ScaleBC7F90", 0xbc7f90, track_scale_bc7f90_original, &track_scale_bc7f90, &g_track_scales);

// ---- Fireworks::Entry9FC910(n, v) (0x9FC910) -----------------------------------------------------------------------
// Entry n's part (0x30 bytes at +0x54, its +8): +0x60 = v, +0x68 = the game's 0x10F1F34, +0x6C = A / v, +0x70 = B / v^2
// (A, B the game's 0x10F1F38, 0x10F1F3C).
static volatile long g_firework_entries;
static void __thiscall fireworks_entry_9fc910(const uint8_t *self, int32_t n, float v) {
    g_firework_entries++;
    uint8_t *const e = *(uint8_t *const *)(*(const uint8_t *const *)(self + 0x54) + n * 0x30 + 8);
    *(float *)(e + 0x60) = v;
    *(uint32_t *)(e + 0x68) = *(const volatile uint32_t *)0x10f1f34;
    *(volatile float *)(e + 0x6c) = (float)(*(const volatile float *)0x10f1f38 / (long double)v);
    *(volatile float *)(e + 0x70) = (float)(*(const volatile float *)0x10f1f3c / ((long double)v * v));
}
static const uint8_t fireworks_entry_9fc910_original[] = {0x8b, 0x49, 0x54, 0x8b, 0x44, 0x24, 0x04, 0x8b, 0x54, 0x24, 0x08};
REWRITE("Fireworks::Entry9FC910", 0x9fc910, fireworks_entry_9fc910_original, &fireworks_entry_9fc910, &g_firework_entries);

// ---- Math::ScaleC19880(n) (0xC19880): floor(n K) (K 0xFAFE88, a float first) & 0x1FF ---------------------------------
static volatile long g_math_scales;
static uint32_t __stdcall math_scale_c19880(int32_t n) {
    g_math_scales++;
    volatile float q = (float)((long double)n * *(const float *)0xfafe88);
    volatile float fl = (float)floor_h2(q);
    return (uint32_t)x87_fistp64_h2(fl) & 0x1ff;
}
static const uint8_t math_scale_c19880_original[] = {0x83, 0xec, 0x08, 0xdb, 0x44, 0x24, 0x0c, 0xd8, 0x0d, 0x88, 0xfe, 0xfa, 0x00};
REWRITE("Math::ScaleC19880", 0xc19880, math_scale_c19880_original, &math_scale_c19880, &g_math_scales);

// ---- Animal::ValueA5ED70 (0xA5ED70): half the part's (+8) first value, at least 1 first; 0 when that is not above 0 --
static volatile long g_animal_halves;
static long double __thiscall animal_value_a5ed70(const uint8_t *self) {
    g_animal_halves++;
    const float v = **(const float *const *)(self + 8);
    if (!(v > *(const float *)0xf264e0)) return *(const float *)0xf264e0;
    volatile float m = v < 1.0f ? 1.0f : v;
    return (long double)m * *(const float *)0xf2651c;
}
static const uint8_t animal_value_a5ed70_original[] = {0x8b, 0x49, 0x08, 0xd9, 0x01, 0x83, 0xec, 0x0c, 0xd8, 0x1d, 0xe0, 0x64, 0xf2, 0x00};
REWRITE("Animal::ValueA5ED70", 0xa5ed70, animal_value_a5ed70_original, &animal_value_a5ed70, &g_animal_halves);

// ---- Animal::FractionA887F0 (0xA887F0) -----------------------------------------------------------------------------
// f = 1 - +0xA8 10 / +0x6C (whole numbers), at least a half; +0x50 += f S (S the game's 0x10F24D0), 0 when that sum
// (unrounded) is below 0.
static volatile long g_animal_fractions;
static void __thiscall animal_fraction_a887f0(uint8_t *self) {
    g_animal_fractions++;
    long double f = (long double)*(const float *)0xf264dc - (long double)*(const int32_t *)(self + 0xa8) * *(const float *)0xf275f4 / (long double)*(const int32_t *)(self + 0x6c);
    if (f < *(const float *)0xf2651c) f = *(const float *)0xf2651c;
    volatile float *const v = (volatile float *)(self + 0x50);
    const long double s = f * *(const volatile float *)0x10f24d0 + *v;
    *v = (float)s;
    if (s < *(const float *)0xf264e0) *(volatile uint32_t *)v = 0;
}
static const uint8_t animal_fraction_a887f0_original[] = {0xdb, 0x81, 0xa8, 0x00, 0x00, 0x00, 0xd8, 0x0d, 0xf4, 0x75, 0xf2, 0x00};
REWRITE("Animal::FractionA887F0", 0xa887f0, animal_fraction_a887f0_original, &animal_fraction_a887f0, &g_animal_fractions);

// ---- Water::Value92E870(n, flag, step) (0x92E870) ------------------------------------------------------------------
// With flag (a byte) clear: the height of point n (the +0x20 record's list +0x18, pairs of floats; its second), or 0
// with no list count (+0x1C). Set: (n - 1) step + +0x54.
static volatile long g_water_values;
static long double __thiscall water_value_92e870(const uint8_t *self, int32_t n, uint32_t flag, float step) {
    g_water_values++;
    if (!(uint8_t)flag) {
        const uint8_t *const r = *(const uint8_t *const *)(self + 0x20);
        if (!*(const uint32_t *)(r + 0x1c)) return *(const float *)0xf264e0;
        return (*(const float *const *)(r + 0x18))[n * 2 - 1];
    }
    return (long double)(n - 1) * step + *(const float *)(self + 0x54);
}
static const uint8_t water_value_92e870_original[] = {0x8a, 0x44, 0x24, 0x08, 0x84, 0xc0, 0x75, 0x21};
REWRITE("Water::Value92E870", 0x92e870, water_value_92e870_original, &water_value_92e870, &g_water_values);

// ---- Record::SameBD19C0(other) (0xBD19C0): 1 when the counts (+0x10) match and the first count dwords do, else 0 -------
// (only al is set)
static volatile long g_record_sames;
static uint8_t __thiscall record_same_bd19c0(const int32_t *self, const int32_t *other) {
    g_record_sames++;
    const int32_t n = self[4];
    if (n != other[4]) return 0;
    for (int32_t i = 0; i < n; i++)
        if (self[i] != other[i]) return 0;
    return 1;
}
static const uint8_t record_same_bd19c0_original[] = {0x56, 0x8b, 0x74, 0x24, 0x08, 0x8b, 0x56, 0x10, 0x8b, 0xc1};
REWRITE("Record::SameBD19C0", 0xbd19c0, record_same_bd19c0_original, &record_same_bd19c0, &g_record_sames);

// ---- Track::NearBC3F90(r) (0xBC3F90): whether two points (ecx, eax) are within r -------------------------------------
// (dx^2 + dz^2) + dy^2 not above r r (unrounded; a NaN gives 0).
static volatile long g_track_nears;
extern "C" uint32_t __stdcall forever_track_near(const float *a, const float *b, float r) {
    g_track_nears++;
    const long double dx = (long double)a[0] - b[0], dy = (long double)a[1] - b[1], dz = (long double)a[2] - b[2];
    return (dx * dx + dz * dz) + dy * dy <= (long double)r * r ? 1 : 0;
}
extern "C" __attribute__((naked)) void forever_track_near_regs() {   // a ecx, b eax; r on the stack
    __asm__("pushl 4(%esp)\n\tpushl %eax\n\tpushl %ecx\n\tcalll _forever_track_near@12\n\tret $4");
}
static const uint8_t track_near_bc3f90_original[] = {0x55, 0x8b, 0xec, 0xd9, 0x01, 0x83, 0xe4, 0xf0, 0xd8, 0x20};
REWRITE("Track::NearBC3F90", 0xbc3f90, track_near_bc3f90_original, (void *)&forever_track_near_regs, &g_track_nears);

// ---- WideString::CompareNarrow(other), String::CompareWide(other) (0xBCEA50, 0xBCF120) -------------------------------
// Texts {.., length +4, characters +0xC} (behind +0): over the shorter length (unsigned), the first difference of
// this text's character less the other's (wide characters unsigned, narrow ones signed); else the difference in length.
static volatile long g_mixed_compares;
template <class A, class B> static int32_t __thiscall mixed_compare(const uint8_t *const *self, const uint8_t *const *other) {
    g_mixed_compares++;
    const uint8_t *const a = *self, *const b = *other;
    const uint32_t la = *(const uint32_t *)(a + 4), lb = *(const uint32_t *)(b + 4);
    const A *const ca = *(const A *const *)(a + 0xc);
    const B *const cb = *(const B *const *)(b + 0xc);
    const uint32_t n = lb < la ? lb : la;
    for (uint32_t i = 0; i < n; i++) {
        const int32_t d = (int32_t)ca[i] - (int32_t)cb[i];
        if (d) return d;
    }
    return (int32_t)(la - lb);
}
static const uint8_t wide_compare_narrow_original[] = {0x8b, 0x01, 0x8b, 0x4c, 0x24, 0x04, 0x8b, 0x09, 0x53, 0x8b, 0x59, 0x0c};
REWRITE("WideString::CompareNarrow", 0xbcea50, wide_compare_narrow_original, (&mixed_compare<uint16_t, int8_t>), &g_mixed_compares);
REWRITE("String::CompareWide", 0xbcf120, wide_compare_narrow_original, (&mixed_compare<int8_t, uint16_t>), &g_mixed_compares);

// ---- Scenery::WrapAngle839120(a) (0x839120): +0xF8 = a wrapped into 0..2 pi by whole turns (unrounded steps) ----------
static volatile long g_scenery_wraps;
static void __thiscall scenery_wrap_angle_839120(uint8_t *self, float a) {
    g_scenery_wraps++;
    const float turn = *(const float *)0xf268dc;
    long double x = a;
    while (!(x < turn || x != x)) x = x - turn;
    while (x < *(const float *)0xf264e0) x = x + turn;
    *(volatile float *)(self + 0xf8) = (float)x;
}
static const uint8_t scenery_wrap_angle_839120_original[] = {0xd9, 0x44, 0x24, 0x04, 0xd8, 0x15, 0xdc, 0x68, 0xf2, 0x00};
REWRITE("Scenery::WrapAngle839120", 0x839120, scenery_wrap_angle_839120_original, &scenery_wrap_angle_839120, &g_scenery_wraps);

// ---- Record::CompareC21270(other) (0xC21270): -1, 0 or 1 by the first float, then the second ------------------------
// A NaN in either compares as greater (1).
static volatile long g_record_compares;
static int32_t __thiscall record_compare_c21270(const float *self, const float *other) {
    g_record_compares++;
    if (self[0] < other[0]) return -1;
    if (!(self[0] == other[0])) return 1;
    if (self[1] < other[1]) return -1;
    return self[1] == other[1] ? 0 : 1;
}
static const uint8_t record_compare_c21270_original[] = {0xd9, 0x01, 0x8b, 0x54, 0x24, 0x04, 0xd8, 0x1a, 0xdf, 0xe0};
REWRITE("Record::CompareC21270", 0xc21270, record_compare_c21270_original, &record_compare_c21270, &g_record_compares);

// ---- Fireworks::Point9E8790(out, n, k) (0x9E8790) ------------------------------------------------------------------
// Row n of the game's table at 0x10F1CF8 (20 bytes: x, z, y, w): out = {shape[k].x + x, y, z, w, n}, the shape being
// the +0x58'th of the list at 0x10F1CD0 (0x20 bytes a point). Gives the row's w.
static volatile long g_firework_points;
static uint32_t __thiscall fireworks_point_9e8790(const uint8_t *self, uint32_t *out, int32_t n, int32_t k) {
    g_firework_points++;
    const float *const shape = ((const float *const *)0x10f1cd0)[*(const int32_t *)(self + 0x58)];
    const uint32_t *const row = (const uint32_t *)(0x10f1cf8 + n * 20);
    *(volatile float *)out = (float)((long double)*(const float *)((const uint8_t *)shape + k * 0x20) + *(const float *)row);
    out[1] = row[2], out[2] = row[1], out[3] = row[3], out[4] = (uint32_t)n;
    return row[3];
}
static const uint8_t fireworks_point_9e8790_original[] = {0x8b, 0x49, 0x58, 0x8b, 0x0c, 0x8d, 0xd0, 0x1c, 0x0f, 0x01};
REWRITE("Fireworks::Point9E8790", 0x9e8790, fireworks_point_9e8790_original, &fireworks_point_9e8790, &g_firework_points);

// ---- Scenery::Value8605F0(i) (0x8605F0) ----------------------------------------------------------------------------
// The part (+0xC) list's (+0x18) entry i, its +0x38 v; k = +0x10. a = v A[k] S (A at 0x10F087C, S 0x10F093C). For the
// last entry (+0x14 count) a; else b = v B[k] T (B at 0x10F07C0, T 0x10F0938) as a float when below a, else a.
static volatile long g_scenery_values;
static long double __thiscall scenery_value_8605f0(const uint8_t *self, int32_t i) {
    g_scenery_values++;
    const uint8_t *const part = *(const uint8_t *const *)(self + 0xc);
    const int32_t k = *(const int32_t *)(self + 0x10);
    const float v = *(const float *)((*(const uint8_t *const *const *)(part + 0x18))[i] + 0x38);
    const long double a = (long double)v * ((const float *)0x10f087c)[k] * *(const volatile float *)0x10f093c;
    if (i == *(const int32_t *)(part + 0x14) - 1) return a;
    const long double bx = (long double)v * ((const float *)0x10f07c0)[k] * *(const volatile float *)0x10f0938;
    volatile float b = (float)bx;
    return bx < a ? (long double)b : a;   // (compared unrounded, given as a float)
}
static const uint8_t scenery_value_8605f0_original[] = {0x8b, 0x51, 0x0c, 0x8b, 0x41, 0x10, 0x56, 0x8b, 0x74, 0x24, 0x08};
REWRITE("Scenery::Value8605F0", 0x8605f0, scenery_value_8605f0_original, &scenery_value_8605f0, &g_scenery_values);

// ---- Record::Make7CD120(from), SceneryBehaviour::Init820B50(from) (0x7CD120, 0x820B50): handle copies ----------------
// A handle (table 0xF26AE0, then its own) taking a reference to from's object (its count +8 goes up) and joining the
// object's list (+0x20) at the front (nodes {next, back}); its +0x10 points at itself; then a few of from's values.
static volatile long g_handle_copies;
static inline void handle_join(uint32_t *h, uint32_t *object) {
    object[2]++;
    h[1] = (uint32_t)(uintptr_t)object;
    uint32_t *const head = (uint32_t *)(uintptr_t)object[8], *const node = h + 2;
    node[0] = head[0];
    ((uint32_t *)(uintptr_t)head[0])[1] = (uint32_t)(uintptr_t)node;
    node[1] = (uint32_t)(uintptr_t)head;
    head[0] = (uint32_t)(uintptr_t)node;
}
static uint32_t *__thiscall record_make_7cd120(uint32_t *self, const uint8_t *from) {
    g_handle_copies++;
    self[0] = 0xf70de0;
    uint32_t *const h = self + 1;
    h[0] = 0xf26ae0;
    h[4] = (uint32_t)(uintptr_t)h;
    handle_join(h, *(uint32_t *const *)(from + 8));
    h[0] = 0xf72040;
    h[5] = *(const uint32_t *)(from + 0x18);
    *(uint8_t *)(h + 6) = from[0x1c];
    ((uint8_t *)self)[0x20] = from[0x20];
    ((uint8_t *)self)[0x21] = from[0x21];
    return self;
}
static const uint8_t record_make_7cd120_original[] = {0x8b, 0xc1, 0xc7, 0x00, 0xe0, 0x0d, 0xf7, 0x00};
REWRITE("Record::Make7CD120", 0x7cd120, record_make_7cd120_original, &record_make_7cd120, &g_handle_copies);
static uint32_t *__thiscall scenery_behaviour_init_820b50(uint32_t *self, const uint8_t *from) {
    g_handle_copies++;
    self[0] = 0xf26ae0;
    self[4] = (uint32_t)(uintptr_t)self;
    handle_join(self, *(uint32_t *const *)(from + 4));
    self[0] = 0xf75b08;
    self[5] = *(const uint32_t *)(from + 0x14);
    *(uint8_t *)(self + 6) = from[0x18];
    self[7] = *(const uint32_t *)(from + 0x1c);
    return self;
}
static const uint8_t scenery_behaviour_init_820b50_original[] = {0x8b, 0xc1, 0xc7, 0x00, 0xe0, 0x6a, 0xf2, 0x00};
REWRITE("SceneryBehaviour::Init820B50", 0x820b50, scenery_behaviour_init_820b50_original, &scenery_behaviour_init_820b50, &g_handle_copies);

// ---- Effect::Init488290(a, b, p, q) (0x488290): {table 0xF2E308, -, p (two dwords), q (two), 0, a, b}. Gives it. ------
static volatile long g_effect_inits;
static uint32_t *__thiscall effect_init_488290(uint32_t *self, uint32_t a, uint32_t b, const uint32_t *p, const uint32_t *q) {
    g_effect_inits++;
    self[0] = 0xf2e308;
    self[2] = p[0], self[3] = p[1], self[4] = q[0], self[5] = q[1];
    self[6] = 0, self[7] = a, self[8] = b;
    return self;
}
static const uint8_t effect_init_488290_original[] = {0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x0c, 0xc7, 0x00, 0x08, 0xe3, 0xf2, 0x00};
REWRITE("Effect::Init488290", 0x488290, effect_init_488290_original, &effect_init_488290, &g_effect_inits);

// ---- Window::Init6A3B60(p, q, v) (0x6A3B60): {p (two dwords), q (two), 0, 0, 1, 1, v}. Gives it. -----------------------
static volatile long g_window_rect_inits;
static uint32_t *__thiscall window_init_6a3b60(uint32_t *self, const uint32_t *p, const uint32_t *q, uint32_t v) {
    g_window_rect_inits++;
    self[0] = p[0], self[1] = p[1], self[2] = q[0], self[3] = q[1];
    self[4] = 0, self[5] = 0, self[6] = 0x3f800000, self[7] = 0x3f800000, self[8] = v;
    return self;
}
static const uint8_t window_init_6a3b60_original[] = {0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x04, 0x8b, 0x11, 0x89, 0x10};
REWRITE("Window::Init6A3B60", 0x6a3b60, window_init_6a3b60_original, &window_init_6a3b60, &g_window_rect_inits);

// ---- Window::Ring680AB0(item) (0x680AB0): puts a 12-byte item at the back of a ring --------------------------------------
// Entries +0, room +4, front +8, count +0xC (unsigned): 0 when full, else the item goes to (front + count) % room, the
// count goes up and 1 (only al).
static volatile long g_ring_pushes;
static uint8_t __thiscall window_ring_680ab0(uint32_t *self, const uint32_t *item) {
    g_ring_pushes++;
    if (self[3] >= self[1]) return 0;
    uint32_t *const e = (uint32_t *)(uintptr_t)self[0] + (self[2] + self[3]) % self[1] * 3;
    e[0] = item[0], e[1] = item[1], e[2] = item[2];
    self[3]++;
    return 1;
}
static const uint8_t window_ring_680ab0_original[] = {0x8b, 0x51, 0x0c, 0x56, 0x8b, 0x71, 0x04, 0x3b, 0xd6};
REWRITE("Window::Ring680AB0", 0x680ab0, window_ring_680ab0_original, &window_ring_680ab0, &g_ring_pushes);

// ---- Water::Entry94DC80(n, p, q) (0x94DC80): entry n (0x60 bytes at +0x1B4) +0x10 = p and +0x20 = q. Gives it. --------
static volatile long g_water_entry_sets;
static uint32_t *__thiscall water_entry_94dc80(const uint8_t *self, int32_t n, const uint32_t *p, const uint32_t *q) {
    g_water_entry_sets++;
    uint32_t *const e = (uint32_t *)(*(uint8_t *const *)(self + 0x1b4) + n * 0x60);
    e[4] = p[0], e[5] = p[1], e[6] = p[2];
    e[8] = q[0], e[9] = q[1], e[10] = q[2];
    return e;
}
static const uint8_t water_entry_94dc80_original[] = {0x8b, 0x91, 0xb4, 0x01, 0x00, 0x00, 0x8b, 0x44, 0x24, 0x04};
REWRITE("Water::Entry94DC80", 0x94dc80, water_entry_94dc80_original, &water_entry_94dc80, &g_water_entry_sets);

// ---- Terrain::Clear8FFFE0(n) (0x8FFFE0): frees entry n (0x60 bytes at +0xA8) when its byte +0 is on --------------------
// The byte is cleared and n goes on the free list: slot (+0xB4 - +0xB0) of +0xAC, then +0xB0 goes down by one.
static volatile long g_terrain_frees;
static void __thiscall terrain_clear_8fffe0(uint8_t *self, int32_t n) {
    g_terrain_frees++;
    uint8_t *const e = *(uint8_t *const *)(self + 0xa8) + n * 0x60;
    if (!*e) return;
    *e = 0;
    int32_t *const used = (int32_t *)(self + 0xb0);
    (*(int32_t *const *)(self + 0xac))[*(const int32_t *)(self + 0xb4) - *used] = n;
    *used -= 1;
}
static const uint8_t terrain_clear_8fffe0_original[] = {0x8b, 0x44, 0x24, 0x04, 0x53, 0x8d, 0x14, 0x40, 0x56};
REWRITE("Terrain::Clear8FFFE0", 0x8fffe0, terrain_clear_8fffe0_original, &terrain_clear_8fffe0, &g_terrain_frees);

// ---- Terrain::Entry8FE1E0(n, x, z) (0x8FE1E0): entry n moves to x, z, keeping how far it moved ------------------------
// Entry n (0x60 bytes at +0xA8): +0x30 = x - +0x10 and +0x34 = z - +0x18 (floats), then +0x10 = x, +0x18 = z. Gives it.
static volatile long g_terrain_moves;
static uint8_t *__thiscall terrain_entry_8fe1e0(const uint8_t *self, int32_t n, float x, float z) {
    g_terrain_moves++;
    uint8_t *const e = *(uint8_t *const *)(self + 0xa8) + n * 0x60;
    const long double oz = *(const float *)(e + 0x18);
    *(volatile float *)(e + 0x30) = (float)((long double)x - *(const float *)(e + 0x10));
    *(volatile float *)(e + 0x34) = (float)((long double)z - oz);
    *(float *)(e + 0x10) = x;
    *(float *)(e + 0x18) = z;
    return e;
}
static const uint8_t terrain_entry_8fe1e0_original[] = {0x8b, 0x91, 0xa8, 0x00, 0x00, 0x00, 0x8b, 0x44, 0x24, 0x04};
REWRITE("Terrain::Entry8FE1E0", 0x8fe1e0, terrain_entry_8fe1e0_original, &terrain_entry_8fe1e0, &g_terrain_moves);

// ---- Labels::Value47DDC0(v) (0x47DDC0): +0x80 = v kept within 0..1 the FPU's way (a NaN gives 1) ---------------------
static volatile long g_label_values;
static void __thiscall labels_value_47ddc0(uint8_t *self, float v) {
    g_label_values++;
    const long double m = 0.0L < v || v != v ? (long double)v : 0.0L;
    *(volatile float *)(self + 0x80) = (float)(1.0L > m ? m : 1.0L);
}
static const uint8_t labels_value_47ddc0_original[] = {0x83, 0xec, 0x0c, 0xc7, 0x04, 0x24, 0x00, 0x00, 0x80, 0x3f};
REWRITE("Labels::Value47DDC0", 0x47ddc0, labels_value_47ddc0_original, &labels_value_47ddc0, &g_label_values);

// ---- Ride::Clear4BF580(-, m) (0x4BF580): m becomes the 4 x 4 identity. Gives m. ------------------------------------
static volatile long g_ride_identities;
static uint32_t *__thiscall ride_clear_4bf580(const void *, uint32_t, uint32_t *m) {
    g_ride_identities++;
    for (int i = 0; i < 16; i++) m[i] = i % 5 ? 0 : 0x3f800000;
    return m;
}
static const uint8_t ride_clear_4bf580_original[] = {0x8b, 0x44, 0x24, 0x08, 0x33, 0xc9, 0x89, 0x48, 0x38};
REWRITE("Ride::Clear4BF580", 0x4bf580, ride_clear_4bf580_original, &ride_clear_4bf580, &g_ride_identities);

// ---- Camera::PartCD7F60(out) (0xCD7F60): the part's (+8) point (its +0x10), or zeros with no part. Gives out. ---------
static volatile long g_camera_parts;
static float *__thiscall camera_part_cd7f60(const uint8_t *self, float *out) {
    g_camera_parts++;
    const uint8_t *const part = *(const uint8_t *const *)(self + 8);
    volatile float *const o = out;
    if (!part) {
        ((volatile uint32_t *)o)[0] = 0, ((volatile uint32_t *)o)[1] = 0, ((volatile uint32_t *)o)[2] = 0;
        return out;
    }
    const float *const v = *(const float *const *)(part + 0x10);
    o[0] = v[0], o[1] = v[1], o[2] = v[2];
    return out;
}
static const uint8_t camera_part_cd7f60_original[] = {0x55, 0x8b, 0xec, 0x8b, 0x41, 0x08, 0x33, 0xc9, 0x83, 0xe4, 0xf0};
REWRITE("Camera::PartCD7F60", 0xcd7f60, camera_part_cd7f60_original, &camera_part_cd7f60, &g_camera_parts);

// ---- Car::Time502070(dt) (0x502070): +0x28 -= dt K (K 0xF368C8), 0 when below 0 and 4 pi when past it (unrounded) ----
static volatile long g_car_times;
static void __thiscall car_time_502070(uint8_t *self, float dt) {
    g_car_times++;
    volatile float *const t = (volatile float *)(self + 0x28);
    const long double v = *t - (long double)dt * *(const float *)0xf368c8;
    *t = (float)v;
    if (v < *(const float *)0xf368c4) *(volatile uint32_t *)t = 0;
    else if (v > *(const float *)0xf368c0) *(volatile uint32_t *)t = 0x41490fdb;
}
static const uint8_t car_time_502070_original[] = {0xd9, 0x44, 0x24, 0x04, 0xd8, 0x0d, 0xc8, 0x68, 0xf3, 0x00};
REWRITE("Car::Time502070", 0x502070, car_time_502070_original, &car_time_502070, &g_car_times);

// ---- Path::Entry7338D0(out, n) (0x7338D0): entry n's (0x98 bytes at +0xC) two dwords +0x58, or -1s ---------------------
// (-1s too for n below 0 or an entry whose byte +0 is clear). Gives out.
static volatile long g_path_entries;
static uint32_t *__thiscall path_entry_7338d0(const uint8_t *self, uint32_t *out, int32_t n) {
    g_path_entries++;
    if (n >= 0) {
        const uint8_t *const e = *(const uint8_t *const *)(self + 0xc) + n * 0x98;
        if (*e) {
            out[0] = *(const uint32_t *)(e + 0x58), out[1] = *(const uint32_t *)(e + 0x5c);
            return out;
        }
    }
    out[0] = 0xffffffff, out[1] = 0xffffffff;
    return out;
}
static const uint8_t path_entry_7338d0_original[] = {0x8b, 0x44, 0x24, 0x08, 0x85, 0xc0, 0x7c, 0x26};
REWRITE("Path::Entry7338D0", 0x7338d0, path_entry_7338d0_original, &path_entry_7338d0, &g_path_entries);

// ---- Resource::EntryC30890(n) (0xC30890): frees a chain of entries -------------------------------------------------
// From n (nothing for n below 0) along the next links (+0xC; 16-byte entries at +0x20) to the last (link below 0): each
// one's +4 cleared and the count +0x2C down by one; the last links to the free list (+0x24), whose head becomes n.
static volatile long g_resource_frees;
static void __thiscall resource_entry_c30890(uint8_t *self, int32_t n) {
    g_resource_frees++;
    if (n < 0) return;
    int32_t i = n, last;
    do {
        uint8_t *const e = *(uint8_t *const *)(self + 0x20) + i * 16;
        *(uint32_t *)(e + 4) = 0;
        *(int32_t *)(self + 0x2c) -= 1;
        last = i;
        i = *(const int32_t *)(*(uint8_t *const *)(self + 0x20) + i * 16 + 0xc);
    } while (i >= 0);
    *(int32_t *)(*(uint8_t *const *)(self + 0x20) + last * 16 + 0xc) = *(const int32_t *)(self + 0x24);
    *(int32_t *)(self + 0x24) = n;
}
static const uint8_t resource_entry_c30890_original[] = {0x53, 0x8b, 0x5c, 0x24, 0x08, 0x85, 0xdb, 0x7c, 0x35};
REWRITE("Resource::EntryC30890", 0xc30890, resource_entry_c30890_original, &resource_entry_c30890, &g_resource_frees);

// ---- Window::Step6F2490(k, p, ref, a, b) (0x6F2490) ----------------------------------------------------------------
// {table 0xF67034, a reference to *ref (its count +0 up), .., p at +0x10, k^2 (a float) at +0x20, a, b}. Gives it.
static volatile long g_window_steps;
static uint32_t *__thiscall window_step_6f2490(uint32_t *self, float k, const uint32_t *p, uint32_t *const *ref, uint32_t a, uint32_t b) {
    g_window_steps++;
    self[0] = 0xf67034;
    uint32_t *const r = *ref;
    self[1] = (uint32_t)(uintptr_t)r;
    r[0]++;
    self[4] = p[0], self[5] = p[1], self[6] = p[2];
    *(volatile float *)(self + 8) = (float)((long double)k * k);
    self[9] = a, self[10] = b;
    return self;
}
static const uint8_t window_step_6f2490_original[] = {0xd9, 0x44, 0x24, 0x04, 0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x0c};
REWRITE("Window::Step6F2490", 0x6f2490, window_step_6f2490_original, &window_step_6f2490, &g_window_steps);

// ---- Animal::SetB9A1C0 (0xB9A1C0): lets go of its slot ---------------------------------------------------------------
// With a slot (+0xAC, not below 0): that entry's (0xB8 bytes at +0x74) byte +0x79 = 1; +0x7C, +0x80 = 0; the slot -1;
// +0x84 = +0x64 - 1 and +0x88 = +0x68 - 1.
static volatile long g_animal_slot_frees;
static void __thiscall animal_set_b9a1c0(uint8_t *self) {
    g_animal_slot_frees++;
    int32_t *const u = (int32_t *)self;
    const int32_t s = u[0xac / 4];
    if (s < 0) return;
    (*(uint8_t *const *)(self + 0x74))[s * 0xb8 + 0x79] = 1;
    u[0x7c / 4] = 0, u[0x80 / 4] = 0;
    u[0xac / 4] = -1;
    u[0x84 / 4] = u[0x64 / 4] - 1;
    u[0x88 / 4] = u[0x68 / 4] - 1;
}
static const uint8_t animal_set_b9a1c0_original[] = {0x8b, 0x81, 0xac, 0x00, 0x00, 0x00, 0x33, 0xd2, 0x3b, 0xc2};
REWRITE("Animal::SetB9A1C0", 0xb9a1c0, animal_set_b9a1c0_original, &animal_set_b9a1c0, &g_animal_slot_frees);

// ---- AttractionInstance::RecentTotal (0x4BFA20): the history's ten counts (12 bytes apart at +0x9C) times 90 / 300 ------
static volatile long g_recent_totals;
static int32_t __thiscall attraction_recent_total(const uint8_t *self) {
    g_recent_totals++;
    const int32_t *const h = *(const int32_t *const *)(self + 0x9c);
    uint32_t sum = 0;
    for (int k = 0; k < 10; k++) sum += (uint32_t)h[k * 3];
    return (int32_t)(sum * 90) / 300;
}
static const uint8_t attraction_recent_total_original[] = {0x8b, 0x89, 0x9c, 0x00, 0x00, 0x00, 0x8b, 0x51, 0x60};
REWRITE("AttractionInstance::RecentTotal", 0x4bfa20, attraction_recent_total_original, &attraction_recent_total, &g_recent_totals);

// ---- Guest::Init756BC0(guest) (0x756BC0) ---------------------------------------------------------------------------
// Table 0xF6C374, +0xC pointing at itself; its node (+4: {next, back}) goes at the back of the guest's list (+0x518),
// or is {0, 0} with no guest. Gives it.
static volatile long g_guest_links;
static uint32_t *__thiscall guest_init_756bc0(uint32_t *self, const uint8_t *guest) {
    g_guest_links++;
    self[0] = 0xf6c374;
    self[3] = (uint32_t)(uintptr_t)self;
    if (!guest) {
        self[1] = 0, self[2] = 0;
        return self;
    }
    uint32_t *const list = *(uint32_t *const *)(guest + 0x518), *const node = self + 1;
    node[1] = list[1];
    *(uint32_t *)(uintptr_t)list[1] = (uint32_t)(uintptr_t)node;
    node[0] = (uint32_t)(uintptr_t)list;
    list[1] = (uint32_t)(uintptr_t)node;
    return self;
}
static const uint8_t guest_init_756bc0_original[] = {0x8b, 0x54, 0x24, 0x04, 0x8b, 0xc1, 0x33, 0xc9, 0x3b, 0xd1};
REWRITE("Guest::Init756BC0", 0x756bc0, guest_init_756bc0_original, &guest_init_756bc0, &g_guest_links);

// ---- TrackPiece::Index5D1670 (0x5D1670): its ten values as a mixed-base number -------------------------------------
// Bits from +0x24 down to +0xC (base 2), then +8 (base 8), +4 and +0 (base 7).
static volatile long g_piece_indexes;
static int32_t __thiscall track_piece_index_5d1670(const int32_t *self) {
    g_piece_indexes++;
    uint32_t h = (uint32_t)self[9];
    for (int k = 8; k >= 3; k--) h = (uint32_t)self[k] + h * 2;
    h = (uint32_t)self[2] + h * 8;
    h = h * 7 + (uint32_t)self[1];
    return (int32_t)(h * 7 + (uint32_t)self[0]);
}
static const uint8_t track_piece_index_5d1670_original[] = {0x8b, 0x51, 0x20, 0x8b, 0x41, 0x24, 0x8d, 0x04, 0x42};
REWRITE("TrackPiece::Index5D1670", 0x5d1670, track_piece_index_5d1670_original, &track_piece_index_5d1670, &g_piece_indexes);

// ---- Guest::Check73BAB0(a, b) (0x73BAB0) ---------------------------------------------------------------------------
// By the mode (+0x520): 0 gives 3 when +0x51C differs from a, else 4; 1 gives 0 when it differs, else b; others 0.
static volatile long g_guest_checks;
static uint32_t __thiscall guest_check_73bab0(const uint8_t *self, uint32_t a, uint32_t b) {
    g_guest_checks++;
    const bool differs = *(const uint32_t *)(self + 0x51c) != a;
    switch (*(const int32_t *)(self + 0x520)) {
    case 0: return differs ? 3 : 4;
    case 1: return differs ? 0 : b;
    default: return 0;
    }
}
static const uint8_t guest_check_73bab0_original[] = {0x8b, 0x81, 0x1c, 0x05, 0x00, 0x00, 0x8b, 0x54, 0x24, 0x04};
REWRITE("Guest::Check73BAB0", 0x73bab0, guest_check_73bab0_original, &guest_check_73bab0, &g_guest_checks);

// ---- Person::Time7F8620(dt) (0x7F8620): a running time (+0x5B0, when above 0) += dt, at most 1 (unrounded check) ------
static volatile long g_person_times;
static void __thiscall person_time_7f8620(uint8_t *self, float dt) {
    g_person_times++;
    volatile float *const t = (volatile float *)(self + 0x5b0);
    if (!(*t > *(const float *)0xf264e0)) return;
    const long double v = (long double)dt + *t;
    *t = (float)v;
    if (v > *(const float *)0xf264dc) *(volatile uint32_t *)t = 0x3f800000;
}
static const uint8_t person_time_7f8620_original[] = {0xd9, 0x81, 0xb0, 0x05, 0x00, 0x00, 0xd8, 0x1d, 0xe0, 0x64, 0xf2, 0x00};
REWRITE("Person::Time7F8620", 0x7f8620, person_time_7f8620_original, &person_time_7f8620, &g_person_times);

// ---- Animal::SetA5FE20(p) (0xA5FE20): +0x40 = 1 and +0x30 = p - the position (+0x10), as floats -----------------------
static volatile long g_animal_aims;
static void __thiscall animal_set_a5fe20(uint8_t *self, const float *p) {
    g_animal_aims++;
    volatile float *const f = (volatile float *)self;
    *(volatile uint32_t *)(self + 0x40) = 0x3f800000;
    const long double dx = (long double)p[0] - f[4], dy = (long double)p[1] - f[5];
    f[14] = (float)((long double)p[2] - f[6]);
    f[12] = (float)dx;
    f[13] = (float)dy;
}
static const uint8_t animal_set_a5fe20_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x10, 0x8b, 0x45, 0x08};
REWRITE("Animal::SetA5FE20", 0xa5fe20, animal_set_a5fe20_original, &animal_set_a5fe20, &g_animal_aims);

// ---- D3DResource::GetDesc(this, out) (0x41FA20): the 32-byte description (+8) into out; D3DERR_INVALIDCALL with no out --
// (The game reads from address 0 when this is 0.)
static volatile long g_resource_descs;
static uint32_t __stdcall d3d_resource_get_desc(const uint8_t *self, uint32_t *out) {
    g_resource_descs++;
    if (!out) return 0x8876086c;
    const uint32_t *const from = (const uint32_t *)(self + 8);
    for (int k = 0; k < 8; k++) out[k] = from[k];
    return 0;
}
static const uint8_t d3d_resource_get_desc_original[] = {0x57, 0x8b, 0x7c, 0x24, 0x0c, 0x85, 0xff, 0x75, 0x09};
REWRITE("D3DResource::GetDesc", 0x41fa20, d3d_resource_get_desc_original, &d3d_resource_get_desc, &g_resource_descs);

// ---- WildAnimal::CountB33390 (0xB33390) ----------------------------------------------------------------------------
// The count (+0x68) goes down by one; when it reaches 0 with a part (+0x54) whose +0x14 is set: +0x6C = the game's
// 0x10F2780 and its node (+0x70: {next, back}) goes in at the front of the game's list (0x10F2794).
static volatile long g_animal_counts;
static void __thiscall wild_animal_count_b33390(uint8_t *self) {
    g_animal_counts++;
    const int32_t c = --*(int32_t *)(self + 0x68);
    const uint8_t *const part = *(const uint8_t *const *)(self + 0x54);
    if (!part || !*(const uint32_t *)(part + 0x14) || c) return;
    *(uint32_t *)(self + 0x6c) = *(const volatile uint32_t *)0x10f2780;
    uint32_t *const node = (uint32_t *)(self + 0x70), *const head = *(uint32_t *const volatile *)0x10f2794;
    node[0] = head[0];
    ((uint32_t *)(uintptr_t)head[0])[1] = (uint32_t)(uintptr_t)node;
    node[1] = (uint32_t)(uintptr_t)head;
    head[0] = (uint32_t)(uintptr_t)node;
}
static const uint8_t wild_animal_count_b33390_original[] = {0x8b, 0x41, 0x68, 0x48, 0x89, 0x41, 0x68, 0x8b, 0xd0};
REWRITE("WildAnimal::CountB33390", 0xb33390, wild_animal_count_b33390_original, &wild_animal_count_b33390, &g_animal_counts);

// ---- Animals::PassB83A20(key) (0xB83A20): the float +4 of the first entry (0x14 bytes at +4, count +8) with key, else 0 -
static volatile long g_animal_finds;
static long double __thiscall animals_pass_b83a20(const uint8_t *self, uint32_t key) {
    g_animal_finds++;
    const uint8_t *const list = *(const uint8_t *const *)(self + 4);
    const int32_t n = *(const int32_t *)(self + 8);
    for (int32_t i = 0; i < n; i++) {
        const uint8_t *const e = list + i * 0x14;
        if (*(const uint32_t *)e != key) continue;
        if (!e) break;
        return *(const float *)(e + 4);
    }
    return *(const float *)0xf264e0;
}
static const uint8_t animals_pass_b83a20_original[] = {0x8b, 0x51, 0x08, 0x56, 0x33, 0xc0, 0x85, 0xd2, 0x57};
REWRITE("Animals::PassB83A20", 0xb83a20, animals_pass_b83a20_original, &animals_pass_b83a20, &g_animal_finds);

// ---- Car::Check50B970(x) (0x50B970): 1 when on (byte +0x2C0), +0x2A4 is above 10 and x above 12, else 0 (only al) ------
static volatile long g_car_checks;
static uint8_t __thiscall car_check_50b970(const uint8_t *self, float x) {
    g_car_checks++;
    if (!self[0x2c0]) return 0;
    return *(const float *)(self + 0x2a4) > *(const float *)0xf275f4 && x > *(const float *)0xf2706c ? 1 : 0;
}
static const uint8_t car_check_50b970_original[] = {0x8a, 0x81, 0xc0, 0x02, 0x00, 0x00, 0x84, 0xc0, 0x74, 0x30};
REWRITE("Car::Check50B970", 0x50b970, car_check_50b970_original, &car_check_50b970, &g_car_checks);

// ---- Park::Pass91C1D0(a, b) (0x91C1D0): 1 when an entry (0x1C bytes at +0x110, count +0x118) has +4 = a, +8 = b ------
// (only al)
static volatile long g_park_pairs;
static uint8_t __thiscall park_pass_91c1d0(const uint8_t *self, uint32_t a, uint32_t b) {
    g_park_pairs++;
    const int32_t n = *(const int32_t *)(self + 0x118);
    const uint8_t *const list = *(const uint8_t *const *)(self + 0x110);
    for (int32_t i = 0; i < n; i++) {
        const uint32_t *const e = (const uint32_t *)(list + i * 0x1c);
        if (e[1] == a && e[2] == b) return 1;
    }
    return 0;
}
static const uint8_t park_pass_91c1d0_original[] = {0x56, 0x8b, 0xb1, 0x18, 0x01, 0x00, 0x00, 0x33, 0xd2};
REWRITE("Park::Pass91C1D0", 0x91c1d0, park_pass_91c1d0_original, &park_pass_91c1d0, &g_park_pairs);

// ---- Camera::SharedCE1670 (0xCE1670): copies the game's points (0x1400810, 16 bytes each) to 0x1400818 ------------------
// The first three dwords of each, the count 0x140081C read again each time.
static volatile long g_camera_copies;
static int32_t __stdcall camera_shared_ce1670() {
    g_camera_copies++;
    for (int32_t i = 0; i < *(const volatile int32_t *)0x140081c; i++) {
        const uint32_t *const from = (const uint32_t *)(*(const volatile uint32_t *)0x1400810 + i * 16);
        uint32_t *const to = (uint32_t *)(*(const volatile uint32_t *)0x1400818 + i * 16);
        to[0] = from[0], to[1] = from[1], to[2] = from[2];
    }
    return *(const volatile int32_t *)0x140081c;
}
static const uint8_t camera_shared_ce1670_original[] = {0xa1, 0x1c, 0x08, 0x40, 0x01, 0x56, 0x33, 0xf6};
REWRITE("Camera::SharedCE1670", 0xce1670, camera_shared_ce1670_original, &camera_shared_ce1670, &g_camera_copies);

// ---- Park::Pass884B50, Pass8852D0(v) (0x884B50: floats at +0xFC, count +0xF4; 0x8852D0: pairs at +0x100, count +0xF8) --
// The first value above v, or +0xDC when there is none.
static volatile long g_park_steps;
template <uint32_t List, uint32_t Count, uint32_t Stride> static long double __thiscall park_first_above(const uint8_t *self, float v) {
    g_park_steps++;
    const int32_t n = *(const int32_t *)(self + Count);
    const uint8_t *const list = *(const uint8_t *const *)(self + List);
    for (int32_t i = 0; i < n; i++) {
        const float e = *(const float *)(list + i * Stride);
        if (e > v) return e;
    }
    return *(const float *)(self + 0xdc);
}
static const uint8_t park_pass_884b50_original[] = {0x53, 0x56, 0x8b, 0xb1, 0xf4, 0x00, 0x00, 0x00, 0x33, 0xd2};
REWRITE("Park::Pass884B50", 0x884b50, park_pass_884b50_original, (&park_first_above<0xfc, 0xf4, 4>), &g_park_steps);
static const uint8_t park_pass_8852d0_original[] = {0x53, 0x56, 0x8b, 0xb1, 0xf8, 0x00, 0x00, 0x00, 0x33, 0xd2};
REWRITE("Park::Pass8852D0", 0x8852d0, park_pass_8852d0_original, (&park_first_above<0x100, 0xf8, 8>), &g_park_steps);

// ---- Person::Pass7F0300(v) (0x7F0300): moves v to the front of its 12 recent values (+0x17C) ---------------------------
// When v is there at i, the ones before it move up by one and v goes first; 0 then, 12 when it is not there.
static volatile long g_person_recents;
static uint32_t __thiscall person_pass_7f0300(uint8_t *self, uint32_t v) {
    g_person_recents++;
    uint32_t *const a = (uint32_t *)(self + 0x17c);
    int32_t i = 0;
    while (i < 12 && a[i] != v) i++;
    if (i == 12) return 12;
    for (; i > 0; i--) a[i] = a[i - 1];
    a[0] = v;
    return 0;
}
static const uint8_t person_pass_7f0300_original[] = {0x56, 0x8d, 0xb1, 0x7c, 0x01, 0x00, 0x00, 0x57};
REWRITE("Person::Pass7F0300", 0x7f0300, person_pass_7f0300_original, &person_pass_7f0300, &g_person_recents);

// ---- Engine::Pass40C490 (0x40C490): a new version (+0x3C) -----------------------------------------------------------
// Up by one; past 0x40000000 it starts again at 1 and every entry (0x24 bytes at +0x2C, count +0x38 read again each
// time) whose byte +0 is set gets +0x18 = 0.
static volatile long g_engine_versions;
static void __thiscall engine_pass_40c490(uint8_t *self) {
    g_engine_versions++;
    int32_t *const version = (int32_t *)(self + 0x3c);
    if (*version <= 0x40000000) {
        *version += 1;
        return;
    }
    *version = 1;
    for (int32_t i = 0; i < *(volatile int32_t *)(self + 0x38); i++) {
        uint8_t *const e = *(uint8_t *const *)(self + 0x2c) + i * 0x24;
        if (*e) *(uint32_t *)(e + 0x18) = 0;
    }
}
static const uint8_t engine_pass_40c490_original[] = {0x8b, 0x41, 0x3c, 0x3d, 0x00, 0x00, 0x00, 0x40, 0x7e, 0x34};
REWRITE("Engine::Pass40C490", 0x40c490, engine_pass_40c490_original, &engine_pass_40c490, &g_engine_versions);

// ---- Window::FindA5B920(n) (0xA5B920): item n over groups {items, count} (+0x60, count +0x64); 0x1F past the end ------
// (signed: a group is passed while n is not below its count)
static volatile long g_window_finds;
static uint32_t __thiscall window_find_a5b920(const uint8_t *self, int32_t n) {
    g_window_finds++;
    const int32_t groups = *(const int32_t *)(self + 0x64);
    const uint32_t *const g = *(const uint32_t *const *)(self + 0x60);
    int32_t i = 0;
    for (; i < groups; i++) {
        if (n < (int32_t)g[i * 2 + 1]) break;
        n -= (int32_t)g[i * 2 + 1];
    }
    if (i >= groups) return 0x1f;
    return ((const uint32_t *)(uintptr_t)g[i * 2])[n];
}
static const uint8_t window_find_a5b920_original[] = {0x53, 0x8b, 0x59, 0x64, 0x33, 0xc0, 0x85, 0xdb, 0x56};
REWRITE("Window::FindA5B920", 0xa5b920, window_find_a5b920_original, &window_find_a5b920, &g_window_finds);

// ---- Resource::FindC35F10(a, b) (0xC35F10): the first entry (12 bytes at +0xA0, count +0xB4) with +0 = a, +8 = b, or 0 -
static volatile long g_resource_finds;
static const uint32_t *__thiscall resource_find_c35f10(const uint8_t *self, uint32_t a, uint32_t b) {
    g_resource_finds++;
    const uint32_t n = *(const uint32_t *)(self + 0xb4);
    const uint32_t *const list = *(const uint32_t *const *)(self + 0xa0);
    for (uint32_t i = 0; i < n; i++)
        if (list[i * 3] == a && list[i * 3 + 2] == b) return list + i * 3;
    return nullptr;
}
static const uint8_t resource_find_c35f10_original[] = {0x53, 0x56, 0x8b, 0xb1, 0xb4, 0x00, 0x00, 0x00, 0x33, 0xd2};
REWRITE("Resource::FindC35F10", 0xc35f10, resource_find_c35f10_original, &resource_find_c35f10, &g_resource_finds);

// ---- Labels::Part475610(a, b, c) (0x475610): the part's (+0x44) word +2, dword +4 and word +0xE into those given -------
// 0 with no part, else 1 (only al).
static volatile long g_label_parts;
static uint8_t __thiscall labels_part_475610(const uint8_t *self, uint32_t *a, uint32_t *b, uint32_t *c) {
    g_label_parts++;
    const uint8_t *const p = *(const uint8_t *const *)(self + 0x44);
    if (!p) return 0;
    if (a) *a = *(const uint16_t *)(p + 2);
    if (b) *b = *(const uint32_t *)(p + 4);
    if (c) *c = *(const uint16_t *)(p + 0xe);
    return 1;
}
static const uint8_t labels_part_475610_original[] = {0x8b, 0x41, 0x44, 0x85, 0xc0, 0x74, 0x34};
REWRITE("Labels::Part475610", 0x475610, labels_part_475610_original, &labels_part_475610, &g_label_parts);

// ---- Regions::Merge417010(other) (0x417010) ------------------------------------------------------------------------
// +0xC += other's - 1; the bounds +0x38, +0x40 take the smaller, +0x3C, +0x44 the larger (signed).
static volatile long g_region_merges;
static void __thiscall regions_merge_417010(int32_t *self, const int32_t *other) {
    g_region_merges++;
    self[3] += other[3] - 1;
    if (other[14] < self[14]) self[14] = other[14];
    if (other[16] < self[16]) self[16] = other[16];
    if (other[15] > self[15]) self[15] = other[15];
    if (other[17] > self[17]) self[17] = other[17];
}
static const uint8_t regions_merge_417010_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0x50, 0x0c, 0x56, 0x8b, 0x71, 0x0c};
REWRITE("Regions::Merge417010", 0x417010, regions_merge_417010_original, &regions_merge_417010, &g_region_merges);

// ---- Static::Reset4984C0 (0x4984C0): the byte +0x70 = 1, and in each of its four tables (+0x60..) every entry (0x18
// bytes at +0x18, count +0x24 read again each time) whose byte +0 is set gets its byte +8 = 1 ----------------------------
static volatile long g_static_resets;
static void __thiscall static_reset_4984c0(uint8_t *self) {
    g_static_resets++;
    self[0x70] = 1;
    for (int k = 0; k < 4; k++) {
        const uint8_t *const t = *(const uint8_t *const *)(self + 0x60 + k * 4);
        for (int32_t i = 0; i < *(const volatile int32_t *)(t + 0x24); i++) {
            uint8_t *const e = *(uint8_t *const *)(t + 0x18) + i * 0x18;
            if (*e) e[8] = 1;
        }
    }
}
static const uint8_t static_reset_4984c0_original[] = {0x53, 0x55, 0x56, 0xb3, 0x01, 0x57, 0x88, 0x59, 0x70};
REWRITE("Static::Reset4984C0", 0x4984c0, static_reset_4984c0_original, &static_reset_4984c0, &g_static_resets);

// ---- Window::Count6C80C0(n) (0x6C80C0): packs row n's eight slots (+0x30 + 0x20 n) to the front ----------------------
// Each slot in use that moves down to place j is cleared where it was and gets its row (+0x4C) and place (+0x50).
static volatile long g_slot_packs;
static int32_t __thiscall window_count_6c80c0(uint8_t *self, int32_t n) {
    g_slot_packs++;
    uint32_t *const slot = (uint32_t *)(self + 0x30 + n * 0x20);
    int32_t j = 0;
    for (int32_t i = 0; i < 8; i++) {
        const uint32_t v = slot[i];
        if (!v) continue;
        if (j != i) {
            slot[j] = v;
            slot[i] = 0;
            uint8_t *const w = (uint8_t *)(uintptr_t)slot[j];
            *(int32_t *)(w + 0x4c) = n;
            *(int32_t *)(w + 0x50) = j;
        }
        j++;
    }
    return j;
}
static const uint8_t window_count_6c80c0_original[] = {0x53, 0x8b, 0x5c, 0x24, 0x08, 0x56, 0x57, 0x8b, 0xd3};
REWRITE("Window::Count6C80C0", 0x6c80c0, window_count_6c80c0_original, &window_count_6c80c0, &g_slot_packs);

// ---- Fireworks::Time9E0E20 (0x9E0E20): with the game's flag (byte 0x10F0E89) set, +0 = the game's 0x14002C0 + +0x10 --
static volatile long g_firework_offsets;
static void __thiscall fireworks_time_9e0e20(uint8_t *self) {
    g_firework_offsets++;
    if (!*(const volatile uint8_t *)0x10f0e89) return;
    const volatile float *const g = (const volatile float *)0x14002c0;
    volatile float *const f = (volatile float *)self;
    const long double x = (long double)g[0] + f[4], y = (long double)g[1] + f[5];
    f[2] = (float)((long double)g[2] + f[6]);
    f[0] = (float)x;
    f[1] = (float)y;
}
static const uint8_t fireworks_time_9e0e20_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x10, 0xa0, 0x89, 0x0e, 0x0f, 0x01};
REWRITE("Fireworks::Time9E0E20", 0x9e0e20, fireworks_time_9e0e20_original, &fireworks_time_9e0e20, &g_firework_offsets);

// ---- Animal::SetAD78F0(v) (0xAD78F0): a new scale (+0x48); when on (byte +0x50) each entry's (0x20 bytes at +0x40,
// count +0x4C read again each time) first float is scaled by new / old (a float) ---------------------------------------
static volatile long g_animal_rescales;
static void __thiscall animal_set_ad78f0(uint8_t *self, float v) {
    g_animal_rescales++;
    volatile float r = (float)((long double)v / *(const float *)(self + 0x48));
    *(float *)(self + 0x48) = v;
    if (!self[0x50]) return;
    for (int32_t i = 0; i < *(const volatile int32_t *)(self + 0x4c); i++) {
        volatile float *const e = (volatile float *)(*(uint8_t *const *)(self + 0x40) + i * 0x20);
        *e = (float)((long double)r * *e);
    }
}
static const uint8_t animal_set_ad78f0_original[] = {0x51, 0xd9, 0x44, 0x24, 0x08, 0x8b, 0x44, 0x24, 0x08, 0xd8, 0x71, 0x48};
REWRITE("Animal::SetAD78F0", 0xad78f0, animal_set_ad78f0_original, &animal_set_ad78f0, &g_animal_rescales);

// ---- Scenery::Time8B0470(dt) (0x8B0470): an angle (+0x14) -= dt K1 K2, kept within K2..K3 (unrounded checks) ----------
// (K1 0xF2CF10, K2 0xF3B7FC, K3 0xF7A3D4)
static volatile long g_scenery_times;
static void __thiscall scenery_time_8b0470(uint8_t *self, float dt) {
    g_scenery_times++;
    volatile float *const a = (volatile float *)(self + 0x14);
    const long double v = *a - (long double)dt * *(const float *)0xf2cf10 * *(const float *)0xf3b7fc;
    *a = (float)v;
    if (v > *(const float *)0xf7a3d4) *(volatile uint32_t *)a = 0x400cbe4c;
    else if (v <= *(const float *)0xf3b7fc) *(volatile uint32_t *)a = 0x3f20d97c;
}
static const uint8_t scenery_time_8b0470_original[] = {0xd9, 0x44, 0x24, 0x04, 0xd8, 0x0d, 0x10, 0xcf, 0xf2, 0x00};
REWRITE("Scenery::Time8B0470", 0x8b0470, scenery_time_8b0470_original, &scenery_time_8b0470, &g_scenery_times);

// ---- Water::Entry94DCC0(n, flag) (0x94DCC0): entry n's (0x60 bytes at +0x1B4) byte +0x54 = flag; when it changed, each
// of its children (+0x38, count +0x3C read again each time) gets it too -------------------------------------------------
static volatile long g_water_flags;
static void __thiscall water_entry_94dcc0(const uint8_t *self, int32_t n, uint32_t flag) {
    g_water_flags++;
    uint8_t *const e = *(uint8_t *const *)(self + 0x1b4) + n * 0x60;
    const uint8_t f = (uint8_t)flag, old = e[0x54];
    e[0x54] = f;
    if (old == f) return;
    for (int32_t i = 0; i < *(const volatile int32_t *)(e + 0x3c); i++) (*(uint8_t *const *)(e + 0x38))[i * 0x60 + 0x54] = f;
}
static const uint8_t water_entry_94dcc0_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8a, 0x54, 0x24, 0x08, 0x8d, 0x04, 0x40};
REWRITE("Water::Entry94DCC0", 0x94dcc0, water_entry_94dcc0_original, &water_entry_94dcc0, &g_water_flags);

// ---- Fence::StepABC2B0(x0, y0, x1, y1) (0xABC2B0): grows a box (+0x64..+0x70) to take in another (signed) ------------
static volatile long g_fence_boxes;
static void __thiscall fence_step_abc2b0(int32_t *self, int32_t x0, int32_t y0, int32_t x1, int32_t y1) {
    g_fence_boxes++;
    int32_t *const b = self + 0x64 / 4;
    if (x0 < b[0]) b[0] = x0;
    if (y0 < b[1]) b[1] = y0;
    if (x1 > b[2]) b[2] = x1;
    if (y1 > b[3]) b[3] = y1;
}
static const uint8_t fence_step_abc2b0_original[] = {0x8b, 0x41, 0x64, 0x8b, 0x54, 0x24, 0x04, 0x3b, 0xd0};
REWRITE("Fence::StepABC2B0", 0xabc2b0, fence_step_abc2b0_original, &fence_step_abc2b0, &g_fence_boxes);

// ---- Shader::EntryCC4840(n) (0xCC4840): entry (indexes +0x18)[n] of the 0x1C-byte records at +0x20; 0 past +0x14 -----
static volatile long g_shader_entries;
static uint8_t *__thiscall shader_entry_cc4840(const uint8_t *self, uint32_t n) {
    g_shader_entries++;
    if (n >= *(const uint32_t *)(self + 0x14)) return nullptr;
    return *(uint8_t *const *)(self + 0x20) + (*(const uint32_t *const *)(self + 0x18))[n] * 0x1c;
}
static const uint8_t shader_entry_cc4840_original[] = {0x55, 0x8b, 0xec, 0x83, 0xec, 0x08, 0xc7, 0x45, 0xf8, 0xcc, 0xcc, 0xcc, 0xcc};
REWRITE("Shader::EntryCC4840", 0xcc4840, shader_entry_cc4840_original, &shader_entry_cc4840, &g_shader_entries);

// ---- Labels::Set475720(v) (0x475720): +0x20 = v, or (when off and the weight +0x70 is above 0) the blend
// ((+0x1C - w) v + +0x20 w) / +0x1C, the weight becoming +0x1C ---------------------------------------------------------
static volatile long g_label_sets;
static void __thiscall labels_set_475720(uint8_t *self, float v) {
    g_label_sets++;
    float *const f = (float *)self;
    if (self[0x6d] || !(f[0x70 / 4] > *(const float *)0xf264e0)) {
        f[0x20 / 4] = v;
        return;
    }
    const long double a = ((long double)f[0x1c / 4] - f[0x70 / 4]) * v;
    const long double b = (long double)f[0x20 / 4] * f[0x70 / 4];
    *(uint32_t *)(self + 0x70) = *(const uint32_t *)(self + 0x1c);
    *(volatile float *)(self + 0x20) = (float)((a + b) / f[0x1c / 4]);
}
static const uint8_t labels_set_475720_original[] = {0x8a, 0x41, 0x6d, 0x84, 0xc0, 0x75, 0x31};
REWRITE("Labels::Set475720", 0x475720, labels_set_475720_original, &labels_set_475720, &g_label_sets);

// ---- Labels::Count475DA0(on in cl) (0x475DA0): the game's count (0x12F0140) up or down by one, not below 0; then every
// label on the game's list (0x10Eca14; {next, .., label +8}, up to the end node whose next is 0) gets byte +0x50 = on ------
static volatile long g_label_counts;
static void __fastcall labels_count_475da0(uint32_t on) {
    g_label_counts++;
    const uint8_t b = (uint8_t)on;
    volatile int32_t *const count = (volatile int32_t *)0x12f0140;
    const int32_t c = *count + (b ? 1 : -1);
    *count = c;
    if (c < 0) *count = 0;
    const uint32_t *node = *(const uint32_t *const volatile *)0x10eca14;
    if (!node[0] || !node) return;
    while (node) {
        const uint32_t *const next = (const uint32_t *)(uintptr_t)node[0];
        ((uint8_t *)(uintptr_t)node[2])[0x50] = b;
        node = next[0] ? next : nullptr;
    }
}
static const uint8_t labels_count_475da0_original[] = {0x8b, 0x15, 0x40, 0x01, 0x2f, 0x01, 0x33, 0xc0, 0x84, 0xc9};
REWRITE("Labels::Count475DA0", 0x475da0, labels_count_475da0_original, &labels_count_475da0, &g_label_counts);

// ---- Rides::Pass50EDB0 (0x50EDB0): each entry's (0x14 bytes at +0x8C4, count +0x8C0 read again each time) +0x10 = the
// part's (+0x414) +0x10C, or 0 when its byte +9 is 0 or 1 -----------------------------------------------------------------
static volatile long g_ride_fills;
static void __thiscall rides_pass_50edb0(const uint8_t *self) {
    g_ride_fills++;
    for (int32_t i = 0; i < *(const volatile int32_t *)(self + 0x8c0); i++) {
        const uint8_t *const p = *(const uint8_t *const *)(self + 0x414);
        const uint32_t v = p[9] <= 1 ? 0 : *(const uint32_t *)(p + 0x10c);
        *(uint32_t *)(*(uint8_t *const *)(self + 0x8c4) + i * 0x14 + 0x10) = v;
    }
}
static const uint8_t rides_pass_50edb0_original[] = {0x8b, 0x81, 0xc0, 0x08, 0x00, 0x00, 0x56, 0x33, 0xf6};
REWRITE("Rides::Pass50EDB0", 0x50edb0, rides_pass_50edb0_original, &rides_pass_50edb0, &g_ride_fills);

// ---- Park::Pass7575E0(key, n) (0x7575E0): the entry (12 bytes at +0xA8, count +0xA4) with key: its list's (+8) item
// n % its count (+4; signed), or 0 when there is none -------------------------------------------------------------------
static volatile long g_park_cycles;
static uint32_t __thiscall park_pass_7575e0(const uint8_t *self, uint32_t key, int32_t n) {
    g_park_cycles++;
    const int32_t count = *(const int32_t *)(self + 0xa4);
    const uint32_t *const list = *(const uint32_t *const *)(self + 0xa8);
    for (int32_t i = 0; i < count; i++) {
        const uint32_t *const e = list + i * 3;
        if (e[0] == key) return ((const uint32_t *)(uintptr_t)e[2])[n % (int32_t)e[1]];
    }
    return 0;
}
static const uint8_t park_pass_7575e0_original[] = {0x56, 0x8b, 0xb1, 0xa4, 0x00, 0x00, 0x00, 0x33, 0xc0};
REWRITE("Park::Pass7575E0", 0x7575e0, park_pass_7575e0_original, &park_pass_7575e0, &g_park_cycles);

// ---- Rides::Pass4CC830(a, b) (0x4CC830): moves the entries in use by a, b ---------------------------------------------
// Entries of 0x2C bytes (+0x94), used when their byte +0 is set; +0x9C (read again each time) of them get +8 += a and
// +0xC += b (the game goes on until it has found that many).
static volatile long g_ride_shifts;
static void __thiscall rides_pass_4cc830(const uint8_t *self, int32_t a, int32_t b) {
    g_ride_shifts++;
    if (*(const int32_t *)(self + 0x9c) <= 0) return;
    int32_t found = 0;
    for (uint32_t off = 0; found < *(const volatile int32_t *)(self + 0x9c); off += 0x2c) {
        uint8_t *const e = *(uint8_t *const *)(self + 0x94) + off;
        if (!*e) continue;
        *(int32_t *)(e + 8) += a;
        *(int32_t *)(e + 0xc) += b;
        found++;
    }
}
static const uint8_t rides_pass_4cc830_original[] = {0x8b, 0x81, 0x9c, 0x00, 0x00, 0x00, 0x56, 0x33, 0xf6};
REWRITE("Rides::Pass4CC830", 0x4cc830, rides_pass_4cc830_original, &rides_pass_4cc830, &g_ride_shifts);

// ---- Table::StepCA84A0(f) (0xCA84A0): as the slider setters for a signed whole number: (max - min) f + min rounded by
// the FPU, kept within min (+8)..max (+0xC), into +4's number ------------------------------------------------------------
static volatile long g_int_sliders;
static void __thiscall table_step_ca84a0(const uint8_t *self, float f) {
    g_int_sliders++;
    const int32_t lo = *(const int32_t *)(self + 8), hi = *(const int32_t *)(self + 0xc);
    int32_t v = (int32_t)x87_fistp64_h2((long double)(int32_t)((uint32_t)hi - (uint32_t)lo) * f + (long double)lo);
    if (lo > v) v = lo;
    **(int32_t *const *)(self + 4) = hi < v ? hi : v;
}
static const uint8_t table_step_ca84a0_original[] = {0x83, 0xec, 0x0c, 0x8b, 0x51, 0x0c, 0x8b, 0x41, 0x08, 0x56};
REWRITE("Table::StepCA84A0", 0xca84a0, table_step_ca84a0_original, &table_step_ca84a0, &g_int_sliders);

// ---- Effect::Pass412EA0(to, key) (0x412EA0): ties to to the first of 32 effects (+4) whose +0x50 is key -----------------
// to's node (+8) goes at the front of the effect's list (+0x30) and to's +4 = the effect; gives the node, or 32 when
// none has the key.
static volatile long g_effect_ties;
static uint32_t __thiscall effect_pass_412ea0(const uint8_t *self, uint32_t *to, uint32_t key) {
    g_effect_ties++;
    const uint32_t *const slots = *(const uint32_t *const *)(self + 4);
    int32_t i = 0;
    for (; i < 32; i++)
        if (slots[i] && *(const uint32_t *)(uintptr_t)(slots[i] + 0x50) == key) break;
    if (i == 32) return 32;
    const uint32_t e = slots[i];
    uint32_t *const head = *(uint32_t *const *)(uintptr_t)(e + 0x30), *const node = to + 2;
    node[0] = head[0];
    ((uint32_t *)(uintptr_t)head[0])[1] = (uint32_t)(uintptr_t)node;
    node[1] = (uint32_t)(uintptr_t)head;
    head[0] = (uint32_t)(uintptr_t)node;
    to[1] = e;
    return (uint32_t)(uintptr_t)node;
}
static const uint8_t effect_pass_412ea0_original[] = {0x56, 0x8b, 0x71, 0x04, 0x57, 0x8b, 0x7c, 0x24, 0x10};
REWRITE("Effect::Pass412EA0", 0x412ea0, effect_pass_412ea0_original, &effect_pass_412ea0, &g_effect_ties);

// ---- Animals::PassA5B970 (0xA5B970): +0x68 = the bits (1 << each) of every item of every group {items, count} (+0x60,
// count +0x64; counts read again each time) ------------------------------------------------------------------------------
static volatile long g_animal_masks;
static void __thiscall animals_pass_a5b970(uint8_t *self) {
    g_animal_masks++;
    volatile uint32_t *const mask = (volatile uint32_t *)(self + 0x68);
    *mask = 0;
    for (int32_t i = 0; i < *(const volatile int32_t *)(self + 0x64); i++)
        for (int32_t j = 0; j < (int32_t)(*(const uint32_t *const volatile *)(self + 0x60))[i * 2 + 1]; j++)
            *mask |= 1u << (((const uint32_t *)(uintptr_t)(*(const uint32_t *const *)(self + 0x60))[i * 2])[j] & 31);
}
static const uint8_t animals_pass_a5b970_original[] = {0x8b, 0xc1, 0x8b, 0x48, 0x64, 0x56, 0x33, 0xf6, 0x3b, 0xce};
REWRITE("Animals::PassA5B970", 0xa5b970, animals_pass_a5b970_original, &animals_pass_a5b970, &g_animal_masks);

// ---- Scenery::Count880F60 (0x880F60): as WildAnimal::CountB33390 with the count +0xE0, part +0xCC (its +0x64), the
// game's 0x10F0A28 into +0xE4 and the node +0xE8 put at the front of the game's list 0x10F0A44 ---------------------------
static volatile long g_scenery_counts;
static void __thiscall scenery_count_880f60(uint8_t *self) {
    g_scenery_counts++;
    const int32_t c = --*(int32_t *)(self + 0xe0);
    const uint8_t *const part = *(const uint8_t *const *)(self + 0xcc);
    if (!part || !*(const uint32_t *)(part + 0x64) || c) return;
    *(uint32_t *)(self + 0xe4) = *(const volatile uint32_t *)0x10f0a28;
    uint32_t *const node = (uint32_t *)(self + 0xe8), *const head = *(uint32_t *const volatile *)0x10f0a44;
    node[0] = head[0];
    ((uint32_t *)(uintptr_t)head[0])[1] = (uint32_t)(uintptr_t)node;
    node[1] = (uint32_t)(uintptr_t)head;
    head[0] = (uint32_t)(uintptr_t)node;
}
static const uint8_t scenery_count_880f60_original[] = {0x8b, 0x81, 0xe0, 0x00, 0x00, 0x00, 0x48, 0x89, 0x81, 0xe0, 0x00, 0x00, 0x00};
REWRITE("Scenery::Count880F60", 0x880f60, scenery_count_880f60_original, &scenery_count_880f60, &g_scenery_counts);

// ---- Resource::CheckC3ACE0(n) (0xC3ACE0) ---------------------------------------------------------------------------
// 0 (only al) for -1; for 0..+0x2C - 1 bit n of the words at +0x88 (as 0 or 1); otherwise 1 when byte +0xA0 is 0
// (only al).
static volatile long g_resource_checks;
static uint32_t __thiscall resource_check_c3ace0(const uint8_t *self, int32_t n) {
    g_resource_checks++;
    if (n == -1) return 0;
    if (n > -1 && n < *(const int32_t *)(self + 0x2c))
        return ((*(const uint32_t *const *)(self + 0x88))[n >> 5] >> (n & 31)) & 1;
    return self[0xa0] == 0;
}
static const uint8_t resource_check_c3ace0_original[] = {0x8b, 0x44, 0x24, 0x04, 0x83, 0xf8, 0xff, 0x75, 0x05};
REWRITE("Resource::CheckC3ACE0", 0xc3ace0, resource_check_c3ace0_original, &resource_check_c3ace0, &g_resource_checks);

// ---- Car::Mode50AF10(out) (0x50AF10): +0x1F0's three dwords in mode 3 (+0x1C8), else zeros. Gives out. ----------------
static volatile long g_car_modes;
static uint32_t *__thiscall car_mode_50af10(const uint8_t *self, uint32_t *out) {
    g_car_modes++;
    if (*(const int32_t *)(self + 0x1c8) == 3) {
        const uint32_t *const v = (const uint32_t *)(self + 0x1f0);
        out[0] = v[0], out[1] = v[1], out[2] = v[2];
    } else
        out[0] = out[1] = out[2] = 0;
    return out;
}
static const uint8_t car_mode_50af10_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x10, 0x83, 0xb9, 0xc8, 0x01, 0x00, 0x00, 0x03};
REWRITE("Car::Mode50AF10", 0x50af10, car_mode_50af10_original, &car_mode_50af10, &g_car_modes);

// ---- Window::Flag6809F0(part, v) (0x6809F0): part's +0x94 gets 0x80000; +0x358 = v when below it; a mode (+0x354) of
// 0 or 3 becomes 1 ---------------------------------------------------------------------------------------------------
static volatile long g_window_flags;
static void __thiscall window_flag_6809f0(uint8_t *self, uint8_t *part, float v) {
    g_window_flags++;
    *(uint32_t *)(part + 0x94) |= 0x80000;
    if (*(const float *)(self + 0x358) < v) *(float *)(self + 0x358) = v;
    int32_t *const mode = (int32_t *)(self + 0x354);
    if (*mode == 0 || *mode == 3) *mode = 1;
}
static const uint8_t window_flag_6809f0_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0x90, 0x94, 0x00, 0x00, 0x00};
REWRITE("Window::Flag6809F0", 0x6809f0, window_flag_6809f0_original, &window_flag_6809f0, &g_window_flags);

// ---- Device::SharedC08890 (0xC08890): the game's shared list at 0x13D3280, set up once (bit 0 of 0x13D3298) ------------
// {first = its end node 0x13D328C, 0, .., end {0, ..}, place = the list, 0}. Gives the list.
static volatile long g_device_shareds;
static uint32_t __stdcall device_shared_c08890() {
    g_device_shareds++;
    volatile uint32_t *const g = (volatile uint32_t *)0x13d3280;   // g[6] is 0x13D3298
    if (!(g[6] & 1)) {
        g[6] |= 1;
        g[2] = 0, g[5] = 0, g[1] = 0;
        g[0] = 0x13d328c;
        g[4] = 0x13d3280;
        g[3] = 0;
    }
    return 0x13d3280;
}
static const uint8_t device_shared_c08890_original[] = {0x8a, 0x0d, 0x98, 0x32, 0x3d, 0x01, 0xb8, 0x01, 0x00, 0x00, 0x00};
REWRITE("Device::SharedC08890", 0xc08890, device_shared_c08890_original, &device_shared_c08890, &g_device_shareds);

// ---- Text::ParseC099A0(s) (0xC099A0): appends s with its 0 to the buffer (+0x10, used +0x1010). Gives where it went. ---
static volatile long g_text_appends;
static uint8_t *__thiscall text_parse_c099a0(uint8_t *self, const char *s) {
    g_text_appends++;
    uint32_t n = 0;
    while (s[n]) n++;
    n++;
    uint8_t *const to = self + 0x10 + *(const uint32_t *)(self + 0x1010);
    for (uint32_t i = 0; i < n; i++) to[i] = (uint8_t)s[i];
    *(uint32_t *)(self + 0x1010) += n;
    return to;
}
static const uint8_t text_parse_c099a0_original[] = {0x53, 0x55, 0x56, 0x8b, 0x74, 0x24, 0x10, 0x8b, 0xc6, 0x57};
REWRITE("Text::ParseC099A0", 0xc099a0, text_parse_c099a0_original, &text_parse_c099a0, &g_text_appends);

// ---- TrackPiece::Index5D1BD0(a, b) (0x5D1BD0): the bits +0x24 down to +0xC as a number c, then (b + 3 c) 0x81 + a ----
static volatile long g_piece_indexes2;
static int32_t __thiscall track_piece_index_5d1bd0(const int32_t *self, int32_t a, int32_t b) {
    g_piece_indexes2++;
    uint32_t h = (uint32_t)self[9];
    for (int k = 8; k >= 3; k--) h = (uint32_t)self[k] + h * 2;
    return (int32_t)(((uint32_t)b + h * 3) * 0x81 + (uint32_t)a);
}
static const uint8_t track_piece_index_5d1bd0_original[] = {0x8b, 0x51, 0x20, 0x8b, 0x41, 0x24, 0x8d, 0x04, 0x42, 0x8b, 0x51, 0x1c};
REWRITE("TrackPiece::Index5D1BD0", 0x5d1bd0, track_piece_index_5d1bd0_original, &track_piece_index_5d1bd0, &g_piece_indexes2);

// ---- VertexDeclaration::AddBDF790(a, b) (0xBDF790): finds a two-byte key in the storage -------------------------------
// Key (a << 8 | b, low 16 bits); entries of 4 bytes from +0xC (+0x20C bytes); for the first match, its second word v:
// 0 when v has either top bit, else (v & 0x3FFF) + the object's (+0) +0x24. 0 when none matches.
static volatile long g_declaration_finds;
static uint32_t __thiscall vertex_declaration_add_bdf790(const uint8_t *self, uint32_t a, uint32_t b) {
    g_declaration_finds++;
    const uint16_t key = (uint16_t)(((a & 0xff) << 8) | b);
    const uintptr_t end = (uintptr_t)(self + 0xc) + *(const uint32_t *)(self + 0x20c);
    for (uintptr_t p = (uintptr_t)(self + 0xc); p < end; p += 4) {
        if (*(const uint16_t *)p != key) continue;
        const uint16_t v = *(const uint16_t *)(p + 2);
        if (v & 0xc000) return 0;
        return (v & 0x3fff) + *(const uint32_t *)(*(const uint8_t *const *)self + 0x24);
    }
    return 0;
}
static const uint8_t vertex_declaration_add_bdf790_original[] = {0x56, 0x8b, 0xf1, 0x8b, 0x96, 0x0c, 0x02, 0x00, 0x00};
REWRITE("VertexDeclaration::AddBDF790", 0xbdf790, vertex_declaration_add_bdf790_original, &vertex_declaration_add_bdf790, &g_declaration_finds);

// ---- Person::Reset801C90 (0x801C90): sorts the items (+8) so those whose byte +8 is set come first ----------------
// The count of those (+4) starts at all (+0); each clear one is swapped with the last still counted (their +4 values
// swapped too) and the count goes down.
static volatile long g_person_sorts;
static void __thiscall person_reset_801c90(int32_t *self) {
    g_person_sorts++;
    self[1] = self[0];
    if (self[0] <= 0) return;
    int32_t i = 0;
    while (i < self[1]) {
        uint32_t *const list = (uint32_t *)(uintptr_t)self[2];
        uint8_t *const e = (uint8_t *)(uintptr_t)list[i];
        if (e[8]) {
            i++;
            continue;
        }
        self[1]--;
        uint8_t *const d = (uint8_t *)(uintptr_t)list[self[1]];
        const uint32_t t = *(const uint32_t *)(d + 4);
        *(uint32_t *)(d + 4) = *(const uint32_t *)(e + 4);
        *(uint32_t *)(e + 4) = t;
        ((uint32_t *)(uintptr_t)self[2])[i] = (uint32_t)(uintptr_t)d;
        ((uint32_t *)(uintptr_t)self[2])[self[1]] = (uint32_t)(uintptr_t)e;
    }
}
static const uint8_t person_reset_801c90_original[] = {0x8b, 0x01, 0x56, 0x33, 0xf6, 0x85, 0xc0, 0x89, 0x41, 0x04};
REWRITE("Person::Reset801C90", 0x801c90, person_reset_801c90_original, &person_reset_801c90, &g_person_sorts);

// ---- Park::Pass774010(key, out) (0x774010): for the entry (0x14 bytes at +0x60, count +0x48) whose part (+4) has +0xC
// = key, out = its point (16 bytes each at +0x13C); gives the point's third dword, or the count when there is none -----
static volatile long g_park_points;
static uint32_t __thiscall park_pass_774010(const uint8_t *self, uint32_t key, uint32_t *out) {
    g_park_points++;
    const int32_t n = *(const int32_t *)(self + 0x48);
    const uint8_t *const list = *(const uint8_t *const *)(self + 0x60);
    for (int32_t i = 0; i < n; i++) {
        if (*(const uint32_t *)(*(const uint8_t *const *)(list + i * 0x14 + 4) + 0xc) != key) continue;
        const uint32_t *const p = (const uint32_t *)(*(const uint8_t *const *)(self + 0x13c) + i * 16);
        out[0] = p[0], out[1] = p[1], out[2] = p[2];
        return p[2];
    }
    return n > 0 ? (uint32_t)n : 0;
}
static const uint8_t park_pass_774010_original[] = {0x56, 0x8b, 0x71, 0x48, 0x33, 0xc0, 0x85, 0xf6};
REWRITE("Park::Pass774010", 0x774010, park_pass_774010_original, &park_pass_774010, &g_park_points);

// ---- Window::Slot6E99B0(a, b) (0x6E99B0): as Window::Count6C80C0 for row a + b of the slots at +0x28, the moved ones
// getting +0x44 = a, +0x48 = b and +0x50 = their place ----------------------------------------------------------------
static volatile long g_slot_packs2;
static int32_t __thiscall window_slot_6e99b0(uint8_t *self, int32_t a, int32_t b) {
    g_slot_packs2++;
    uint32_t *const slot = (uint32_t *)(self + 0x28 + (a + b) * 0x20);
    int32_t j = 0;
    for (int32_t i = 0; i < 8; i++) {
        const uint32_t v = slot[i];
        if (!v) continue;
        if (j != i) {
            slot[j] = v;
            slot[i] = 0;
            uint8_t *const w = (uint8_t *)(uintptr_t)slot[j];
            *(int32_t *)(w + 0x44) = a;
            *(int32_t *)(w + 0x48) = b;
            *(int32_t *)(w + 0x50) = j;
        }
        j++;
    }
    return j;
}
static const uint8_t window_slot_6e99b0_original[] = {0x53, 0x8b, 0x5c, 0x24, 0x08, 0x55, 0x8b, 0x6c, 0x24, 0x10};
REWRITE("Window::Slot6E99B0", 0x6e99b0, window_slot_6e99b0_original, &window_slot_6e99b0, &g_slot_packs2);

// ---- Ride::List53DDB0(out, n, k) (0x53DDB0): a reference to the object of node k of section n's list ----------------
// Section n (+0x30); its list's first node (+4), then k steps along next (+0), stopping at the end node (next 0). out =
// the node's +8's first dword, whose count (+0) goes up. Gives out. (The game reads through 0 past the end.)
static volatile long g_ride_refs;
static uint32_t *__thiscall ride_list_53ddb0(const uint8_t *self, uint32_t *out, int32_t n, int32_t k) {
    g_ride_refs++;
    const uint32_t *const first = (const uint32_t *)(uintptr_t)(*(const uint32_t *const *)(*(const uint8_t *const *)(self + 0x30) + n * 4))[1];
    const uint32_t *p = first[0] ? first : nullptr;
    for (int32_t i = 0; i < k; i++) {
        if (!p) break;
        p = (const uint32_t *)(uintptr_t)p[0];
        if (!p[0]) p = nullptr;
    }
    uint32_t *const object = *(uint32_t *const *)(uintptr_t)p[2];
    *out = (uint32_t)(uintptr_t)object;
    object[0]++;
    return out;
}
static const uint8_t ride_list_53ddb0_original[] = {0x51, 0x8b, 0x41, 0x30, 0x8b, 0x4c, 0x24, 0x0c, 0x8b, 0x04, 0x88};
REWRITE("Ride::List53DDB0", 0x53ddb0, ride_list_53ddb0_original, &ride_list_53ddb0, &g_ride_refs);

// ---- Resource::ValueC307D0 (0xC307D0): the sizes along a chain over a total -------------------------------------------
// From entry +0x28 (0 when below 0) along the next links (+0xC; 16-byte entries at +0x20) to the last, the sum of their
// +4; then sum / +0x18, both unsigned (loaded the game's way: fild, then 2^32 added when negative).
static volatile long g_resource_shares;
static inline long double fild_u32_h2(uint32_t u) {   // fild, then 2^32 added (a rounded add) when negative, as the game does
    const int32_t i = (int32_t)u;
    long double r;
    __asm__("fildl %1" : "=t"(r) : "m"(i));
    return i < 0 ? r + *(const float *)0xf285f8 : r;
}
static long double __thiscall resource_value_c307d0(const uint8_t *self) {
    g_resource_shares++;
    int32_t i = *(const int32_t *)(self + 0x28);
    if (i < 0) return *(const float *)0xf264e0;
    uint32_t sum = 0;
    do {
        const uint32_t *const e = (const uint32_t *)(*(const uint8_t *const *)(self + 0x20) + i * 16);
        sum += e[1];
        i = (int32_t)e[3];
    } while (i >= 0);
    const long double a = fild_u32_h2(sum);
    return a / fild_u32_h2(*(const uint32_t *)(self + 0x18));
}
static const uint8_t resource_value_c307d0_original[] = {0x51, 0x8b, 0x41, 0x28, 0x85, 0xc0, 0x7c, 0x42};
REWRITE("Resource::ValueC307D0", 0xc307d0, resource_value_c307d0_original, &resource_value_c307d0, &g_resource_shares);

// ---- List::Clear (0xCA06A0): gives every node's block (+8) back to the game's free list (0x13E6E28, count 0x13E6E30) --
// From the first node up to the end node (next 0); then the list is empty: {first = its end +0xC, 0, .., end.next 0,
// place = the list}.
static volatile long g_list_clears;
static void __thiscall list_clear(uint32_t *self) {
    g_list_clears++;
    const uint32_t *n = (const uint32_t *)(uintptr_t)self[0];
    if (n[0] && n) {
        volatile uint32_t *const free_head = (volatile uint32_t *)0x13e6e28, *const free_count = (volatile uint32_t *)0x13e6e30;
        while (n) {
            const uint32_t *const next = (const uint32_t *)(uintptr_t)n[0];
            uint32_t *const block = (uint32_t *)(uintptr_t)n[2];
            block[0] = *free_head;
            *free_count = *free_count + 1;
            *free_head = (uint32_t)(uintptr_t)block;
            n = next[0] ? next : nullptr;
        }
    }
    self[1] = 0;
    self[0] = (uint32_t)(uintptr_t)(self + 3);
    self[4] = (uint32_t)(uintptr_t)self;
    self[3] = 0;
}
static const uint8_t list_clear_original[] = {0x8b, 0x11, 0x83, 0x3a, 0x00, 0x74, 0x38};
REWRITE("List::Clear", 0xca06a0, list_clear_original, &list_clear, &g_list_clears);

// ---- Object3AFD0::CopyFrom(from, whole) (0x560460) -----------------------------------------------------------------
// With whole set: tables 0xF3AFD0 (+0) and 0xF3AFC8 (+0xC), and +0x20 = the dword at from's own base (from + (from's
// first table)+4). Then from's +4, +8 and +0x10..+0x1C. Gives it.
static volatile long g_layered_copies;
static uint32_t *__thiscall object3afd0_copy_from(uint32_t *self, const uint8_t *from, uint32_t whole) {
    g_layered_copies++;
    const uint32_t *const f = (const uint32_t *)from;
    if (whole) {
        self[0] = 0xf3afd0;
        self[3] = 0xf3afc8;
        const uint32_t *const base = from ? (const uint32_t *)(from + *(const int32_t *)(uintptr_t)(f[0] + 4)) : nullptr;
        self[8] = *base;
    }
    self[1] = f[1], self[2] = f[2];
    self[4] = f[4], self[5] = f[5], self[6] = f[6], self[7] = f[7];
    return self;
}
static const uint8_t object3afd0_copy_from_original[] = {0x8b, 0xc1, 0x8b, 0x4c, 0x24, 0x08, 0x85, 0xc9, 0x8b, 0x4c, 0x24, 0x04, 0x74, 0x21};
REWRITE("Object3AFD0::CopyFrom", 0x560460, object3afd0_copy_from_original, &object3afd0_copy_from, &g_layered_copies);

// Node lists {next, back, item}: a node is the end when its next is 0; stepping off a node whose next is the end gives 0.
static inline const uint32_t *list_step_h2(const uint32_t *p) {
    const uint32_t *const n = (const uint32_t *)(uintptr_t)p[0];
    return n[0] ? n : nullptr;
}

// ---- Ride::Find5947B0(n) (0x5947B0): item n of the list at +0x208 (0 for n below 0, an empty list or past the end) --
static volatile long g_ride_items;
static uint32_t __thiscall ride_find_5947b0(const uint8_t *self, int32_t n) {
    g_ride_items++;
    if (n < 0) return 0;
    const uint32_t *p = *(const uint32_t *const *)(self + 0x208);
    if (!p[0] || !p) return 0;
    for (; n > 0; n--) {
        p = list_step_h2(p);
        if (!p) return 0;
    }
    return p[2];
}
static const uint8_t ride_find_5947b0_original[] = {0x8b, 0x54, 0x24, 0x04, 0x85, 0xd2, 0x7c, 0x29};
REWRITE("Ride::Find5947B0", 0x5947b0, ride_find_5947b0_original, &ride_find_5947b0, &g_ride_items);

// ---- Ride::Set59FFA0(object) (0x59FFA0): +0x58 = object; +0x8C = the first item of the object's list (+0x74) whose
// first dword is this ride, or 0 --------------------------------------------------------------------------------------
static volatile long g_ride_links;
static void __thiscall ride_set_59ffa0(uint8_t *self, const uint8_t *object) {
    g_ride_links++;
    *(const uint8_t **)(self + 0x58) = object;
    uint32_t found = 0;
    const uint32_t *p = *(const uint32_t *const *)(object + 0x74);
    if (p[0] && p)
        for (;;) {
            if (*(const uint32_t *)(uintptr_t)p[2] == (uint32_t)(uintptr_t)self) {
                found = p[2];
                break;
            }
            p = (const uint32_t *)(uintptr_t)p[0];
            if (!p[0] || !p) break;
        }
    *(uint32_t *)(self + 0x8c) = found;
}
static const uint8_t ride_set_59ffa0_original[] = {0x8b, 0x44, 0x24, 0x04, 0x89, 0x41, 0x58, 0x8b, 0x40, 0x74};
REWRITE("Ride::Set59FFA0", 0x59ffa0, ride_set_59ffa0_original, &ride_set_59ffa0, &g_ride_links);

// ---- Window::Count61CEF0 (0x61CEF0) --------------------------------------------------------------------------------
// 0 (only al) when the count (+0x8C) is at least 2 and the +0x94 part's +0xC is 0; else 1 for a count below 1, or
// when the +0x74 part's +0xC or the +0x88 part's +4 is set; else 0.
static volatile long g_window_counts;
static uint8_t __thiscall window_count_61cef0(const uint8_t *self) {
    g_window_counts++;
    auto part = [&](uint32_t o, uint32_t f) { return *(const uint32_t *)(*(const uint8_t *const *)(self + o) + f); };
    const int32_t c = *(const int32_t *)(self + 0x8c);
    if (c >= 2 && !part(0x94, 0xc)) return 0;
    if (c < 1) return 1;
    return part(0x74, 0xc) || part(0x88, 4) ? 1 : 0;
}
static const uint8_t window_count_61cef0_original[] = {0x8b, 0x81, 0x8c, 0x00, 0x00, 0x00, 0x83, 0xf8, 0x02, 0x7c, 0x0f};
REWRITE("Window::Count61CEF0", 0x61cef0, window_count_61cef0_original, &window_count_61cef0, &g_window_counts);

// ---- Window::SlotC63730(object) (0xC63730): the object's place in the list at +0xB0, or -1 (also when its owner +0x18
// is another) ----------------------------------------------------------------------------------------------------------
static volatile long g_window_places;
static int32_t __thiscall window_slot_c63730(const uint8_t *self, const uint8_t *object) {
    g_window_places++;
    if (*(const uint8_t *const *)(object + 0x18) != self) return -1;
    const uint32_t *p = *(const uint32_t *const *)(self + 0xb0);
    if (!p[0] || !p) return -1;
    for (int32_t i = 0; p; i++) {
        if (p[2] == (uint32_t)(uintptr_t)object) return i;
        p = list_step_h2(p);
    }
    return -1;
}
static const uint8_t window_slot_c63730_original[] = {0x8b, 0x54, 0x24, 0x04, 0x39, 0x4a, 0x18, 0x74, 0x06};
REWRITE("Window::SlotC63730", 0xc63730, window_slot_c63730_original, &window_slot_c63730, &g_window_places);

// ---- Grid::Slot6FDE90 (0x6FDE90): item (+0x40) of the list at +0x4C: its +4's +0xC, or 0 (no slot, past the end, or no
// item there) ----------------------------------------------------------------------------------------------------------
static volatile long g_grid_slots;
static uint32_t __thiscall grid_slot_6fde90(const uint8_t *self) {
    g_grid_slots++;
    int32_t n = *(const int32_t *)(self + 0x40);
    if (n < 0) return 0;
    const uint32_t *const l = *(const uint32_t *const *)(self + 0x4c);
    const uint32_t *p = l[0] ? l : nullptr;
    for (; n > 0; n--) {
        if (!p) return 0;
        p = list_step_h2(p);
    }
    if (!p || !p[2]) return 0;
    return *(const uint32_t *)(uintptr_t)(((const uint32_t *)(uintptr_t)p[2])[1] + 0xc);
}
static const uint8_t grid_slot_6fde90_original[] = {0x8b, 0x51, 0x40, 0x85, 0xd2, 0x7c, 0x34};
REWRITE("Grid::Slot6FDE90", 0x6fde90, grid_slot_6fde90_original, &grid_slot_6fde90, &g_grid_slots);

// ---- PoolComplex::KindAADE10(-) (0xAADE10): 0 for kinds 5-9 (+0x120), else 1 ----------------------------------------
static volatile long g_pool_kinds;
static uint32_t __thiscall pool_complex_kind_aade10(const uint8_t *self, uint32_t) {
    g_pool_kinds++;
    const int32_t k = *(const int32_t *)(self + 0x120);
    return k >= 5 && k <= 9 ? 0 : 1;
}
static const uint8_t pool_complex_kind_aade10_original[] = {0x8b, 0x81, 0x20, 0x01, 0x00, 0x00, 0x83, 0xf8, 0x05};
REWRITE("PoolComplex::KindAADE10", 0xaade10, pool_complex_kind_aade10_original, &pool_complex_kind_aade10, &g_pool_kinds);

// ---- Objectives::Find(group, middle, slot) (0x6E9A40) and Window::Slot6E9960(o) (0x6E9960) -------------------------------
// The table of objectives at +0x28: entry slot + (group + middle) 8, with slot 0-7, middle 0 and group 0-2. Find gives
// the objective there (0 out of range); Slot6E9960 clears the place of o (its group +0x44, middle +0x48, slot +0x50)
// when o is there.
static volatile long g_objective_places;
static inline uint32_t *objective_place_h2(uint8_t *self, int32_t group, int32_t middle, int32_t slot) {
    if (slot < 0 || slot >= 8 || middle < 0 || middle >= 1 || group < 0 || group >= 3) return nullptr;
    return (uint32_t *)(self + 0x28) + slot + (group + middle) * 8;
}
static uint32_t __thiscall objectives_find(uint8_t *self, int32_t group, int32_t middle, int32_t slot) {
    g_objective_places++;
    const uint32_t *const p = objective_place_h2(self, group, middle, slot);
    return p ? *p : 0;
}
static const uint8_t objectives_find_original[] = {0x56, 0x8b, 0x74, 0x24, 0x10, 0x33, 0xc0, 0x85, 0xf6, 0x7c, 0x30};
REWRITE("Objectives::Find", 0x6e9a40, objectives_find_original, &objectives_find, &g_objective_places);
static void __thiscall window_slot_6e9960(uint8_t *self, const uint8_t *o) {
    g_objective_places++;
    uint32_t *const p = objective_place_h2(self, *(const int32_t *)(o + 0x44), *(const int32_t *)(o + 0x48), *(const int32_t *)(o + 0x50));
    if (p && *p == (uint32_t)(uintptr_t)o) *p = 0;
}
static const uint8_t window_slot_6e9960_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0x50, 0x50, 0x85, 0xd2, 0x56};
REWRITE("Window::Slot6E9960", 0x6e9960, window_slot_6e9960_original, &window_slot_6e9960, &g_objective_places);

// ---- Text::FindCharBCF3E0(ch, from) (0xBCF3E0): the place of ch's low byte from from on (up to the text's 0), or -1 ----
// (-1 also when from is past the length +4, unsigned)
static volatile long g_text_char_finds;
static int32_t __thiscall text_find_char_bcf3e0(const uint8_t *const *self, uint32_t ch, uint32_t from) {
    g_text_char_finds++;
    const uint8_t *const t = *self;
    if (from > *(const uint32_t *)(t + 4)) return -1;
    const uint8_t *const buf = *(const uint8_t *const *)(t + 0xc);
    for (const uint8_t *p = buf + from; *p; p++)
        if (*p == (uint8_t)ch) return (int32_t)(p - buf);
    return -1;
}
static const uint8_t text_find_char_bcf3e0_original[] = {0x8b, 0x01, 0x8b, 0x4c, 0x24, 0x08, 0x3b, 0x48, 0x04, 0x76, 0x06};
REWRITE("Text::FindCharBCF3E0", 0xbcf3e0, text_find_char_bcf3e0_original, &text_find_char_bcf3e0, &g_text_char_finds);

// ---- Animals::PassB6C900 (0xB6C900): the first animal of the entries (12 bytes at +0x54, count +0x60) that is in use
// (byte +0), has flag 4 (+0xA8) and whose byte +0xAC is 0; else 0 ------------------------------------------------------
static volatile long g_animal_picks;
static uint32_t __thiscall animals_pass_b6c900(const uint8_t *self) {
    g_animal_picks++;
    const int32_t n = *(const int32_t *)(self + 0x60);
    const uint8_t *const list = *(const uint8_t *const *)(self + 0x54);
    for (int32_t i = 0; i < n; i++) {
        const uint8_t *const e = list + i * 12;
        if (!*e) continue;
        const uint8_t *const a = *(const uint8_t *const *)(e + 4);
        if ((a[0xa8] & 4) && !a[0xac]) return (uint32_t)(uintptr_t)a;
    }
    return 0;
}
static const uint8_t animals_pass_b6c900_original[] = {0x56, 0x8b, 0x71, 0x60, 0x33, 0xd2, 0x85, 0xf6, 0x57};
REWRITE("Animals::PassB6C900", 0xb6c900, animals_pass_b6c900_original, &animals_pass_b6c900, &g_animal_picks);

// ---- Engine::ListC50B20(v) (0xC50B20): 1 (only al) when on (byte +0x6C) with +0x48 = v, or when an item of the list at
// +0xCC has +0xC = v; else 0 ------------------------------------------------------------------------------------------
static volatile long g_engine_has;
static uint8_t __thiscall engine_list_c50b20(const uint8_t *self, uint32_t v) {
    g_engine_has++;
    if (self[0x6c] && *(const uint32_t *)(self + 0x48) == v) return 1;
    const uint32_t *p = *(const uint32_t *const *)(self + 0xcc);
    if (!p[0] || !p) return 0;
    for (;;) {
        if (*(const uint32_t *)(uintptr_t)(p[2] + 0xc) == v) return 1;
        p = (const uint32_t *)(uintptr_t)p[0];
        if (!p[0] || !p) return 0;
    }
}
static const uint8_t engine_list_c50b20_original[] = {0x8a, 0x41, 0x6c, 0x84, 0xc0, 0x8b, 0x54, 0x24, 0x04};
REWRITE("Engine::ListC50B20", 0xc50b20, engine_list_c50b20_original, &engine_list_c50b20, &g_engine_has);

// ---- TrackPiece::Check5D1370 (0x5D1370): 0 when +0 or +4 is 7, +8 is 8, any of +0xC..+0x20 is 2 or +0x24 is 3; else 1 -
static volatile long g_piece_checks;
static uint32_t __thiscall track_piece_check_5d1370(const int32_t *self) {
    g_piece_checks++;
    if (self[0] == 7 || self[1] == 7 || self[2] == 8) return 0;
    for (int k = 3; k <= 8; k++)
        if (self[k] == 2) return 0;
    return self[9] == 3 ? 0 : 1;
}
static const uint8_t track_piece_check_5d1370_original[] = {0x83, 0x39, 0x07, 0x74, 0x3d, 0x83, 0x79, 0x04, 0x07};
REWRITE("TrackPiece::Check5D1370", 0x5d1370, track_piece_check_5d1370_original, &track_piece_check_5d1370, &g_piece_checks);

// ---- Park::Shared774520 (0x774520): 1 (only al) when an item of the game's list (0x10EF8BC) that is on (+0xE4) has a
// place (+0x104: two dwords) equal to this one's +0x20, +0x24; else 0 -------------------------------------------------
static volatile long g_park_place_finds;
static uint8_t __thiscall park_shared_774520(const uint8_t *self) {
    g_park_place_finds++;
    const uint32_t *p = *(const uint32_t *const volatile *)0x10ef8bc;
    if (!p[0] || !p) return 0;
    for (;;) {
        const uint8_t *const item = (const uint8_t *)(uintptr_t)p[2];
        if (*(const uint32_t *)(item + 0xe4)) {
            const uint32_t *const q = *(const uint32_t *const *)(item + 0x104);
            if (q[0] == *(const uint32_t *)(self + 0x20) && q[1] == *(const uint32_t *)(self + 0x24)) return 1;
        }
        p = (const uint32_t *)(uintptr_t)p[0];
        if (!p[0] || !p) return 0;
    }
}
static const uint8_t park_shared_774520_original[] = {0xa1, 0xbc, 0xf8, 0x0e, 0x01, 0x83, 0x38, 0x00, 0x56};
REWRITE("Park::Shared774520", 0x774520, park_shared_774520_original, &park_shared_774520, &g_park_place_finds);

// ---- TrackPiece::Replace5D0D10(old, now) (0x5D0D10): every old in +0x1C, +0x20 and its list's items' +0x34, +0x3C becomes
// now -----------------------------------------------------------------------------------------------------------------
static volatile long g_piece_replaces;
static void __thiscall track_piece_replace_5d0d10(uint8_t *self, uint32_t old, uint32_t now) {
    g_piece_replaces++;
    uint32_t *const u = (uint32_t *)self;
    if (u[7] == old) u[7] = now;
    if (u[8] == old) u[8] = now;
    const uint32_t *p = (const uint32_t *)(uintptr_t)u[0];
    if (!p[0] || !p) return;
    for (;;) {
        uint32_t *const item = (uint32_t *)(uintptr_t)p[2];
        if (item[0x34 / 4] == old) item[0x34 / 4] = now;
        if (item[0x3c / 4] == old) item[0x3c / 4] = now;
        p = (const uint32_t *)(uintptr_t)p[0];
        if (!p[0] || !p) return;
    }
}
static const uint8_t track_piece_replace_5d0d10_original[] = {0x8b, 0x54, 0x24, 0x04, 0x3b, 0x51, 0x1c, 0x56};
REWRITE("TrackPiece::Replace5D0D10", 0x5d0d10, track_piece_replace_5d0d10_original, &track_piece_replace_5d0d10, &g_piece_replaces);

// ---- Window::Mode6A7DA0 (0x6A7DA0): takes the bound value (+0xF4, when there) by the mode (+0xF0): modes 1 and 2 its
// dword into +0x104, mode 3 its byte into +0xFB ---------------------------------------------------------------------------
static volatile long g_window_modes;
static void __thiscall window_mode_6a7da0(uint8_t *self) {
    g_window_modes++;
    const uint8_t *const v = *(const uint8_t *const *)(self + 0xf4);
    const int32_t mode = *(const int32_t *)(self + 0xf0);
    if (!v || mode < 1 || mode > 3) return;
    if (mode == 3) self[0xfb] = *v;
    else *(uint32_t *)(self + 0x104) = *(const uint32_t *)v;
}
static const uint8_t window_mode_6a7da0_original[] = {0x8b, 0x81, 0xf0, 0x00, 0x00, 0x00, 0x48, 0x74, 0x2c};
REWRITE("Window::Mode6A7DA0", 0x6a7da0, window_mode_6a7da0_original, &window_mode_6a7da0, &g_window_modes);

// ---- Scenery::Part827D00 (0x827D00): 1 (only al) when any of the part's (+0x9C) +0x34 pairs of bits in +0x134 is set ---
static volatile long g_scenery_bits;
static uint8_t __thiscall scenery_part_827d00(const uint8_t *self) {
    g_scenery_bits++;
    const uint8_t *const p = *(const uint8_t *const *)(self + 0x9c);
    if (!p) return 0;
    const int32_t n = *(const int32_t *)(p + 0x34);
    const uint32_t mask = *(const uint32_t *)(self + 0x134);
    for (int32_t i = 0; i < n; i++)
        if (mask & (1u << (2 * i & 31)) || mask & (2u << (2 * i & 31))) return 1;
    return 0;
}
static const uint8_t scenery_part_827d00_original[] = {0x8b, 0x91, 0x9c, 0x00, 0x00, 0x00, 0x85, 0xd2, 0x56, 0x57};
REWRITE("Scenery::Part827D00", 0x827d00, scenery_part_827d00_original, &scenery_part_827d00, &g_scenery_bits);

// ---- Text::CompareC46A80(other) (0xC46A80): 1 when two counted texts {word, length word, bytes} are alike ------------
// (both missing counts as alike, one missing as not)
static volatile long g_text_alikes;
static uint32_t __thiscall text_compare_c46a80(const uint16_t *self, const uint16_t *other) {
    g_text_alikes++;
    if (!self) return other ? 0 : 1;
    if (!other || self[0] != other[0] || self[1] != other[1]) return 0;
    const uint8_t *const a = (const uint8_t *)(self + 2), *const b = (const uint8_t *)(other + 2);
    for (uint32_t i = 0; i < self[1]; i++)
        if (a[i] != b[i]) return 0;
    return 1;
}
static const uint8_t text_compare_c46a80_original[] = {0x8b, 0xc1, 0x85, 0xc0, 0x74, 0x2d, 0x8b, 0x54, 0x24, 0x04};
REWRITE("Text::CompareC46A80", 0xc46a80, text_compare_c46a80_original, &text_compare_c46a80, &g_text_alikes);

// ---- PointerArray::Remove(v) (0xCDB9E0): the first v in the array (+0, count +8) gets the last one (or 0 when it was
// the last) and the count goes down by one -----------------------------------------------------------------------------
static volatile long g_pointer_removes2;
static void __thiscall pointer_array_remove(uint32_t *self, uint32_t v) {
    g_pointer_removes2++;
    const int32_t n = (int32_t)self[2];
    uint32_t *const a = (uint32_t *)(uintptr_t)self[0];
    int32_t i = 0;
    while (i < (int32_t)self[2] && a[i] != v) i++;
    if (i >= n) return;
    self[2] = (uint32_t)(n - 1);
    a[i] = i < n - 1 ? a[n - 1] : 0;
}
static const uint8_t pointer_array_remove_original[] = {0x56, 0x8b, 0x71, 0x08, 0x33, 0xc0, 0x85, 0xf6};
REWRITE("PointerArray::Remove", 0xcdb9e0, pointer_array_remove_original, &pointer_array_remove, &g_pointer_removes2);

// ---- Table::Entry45EF10(n, m) (0x45EF10) ---------------------------------------------------------------------------
// From 0x10000 on, n - 0x10000 indexes the list at +0xC (count +0x10); below it, row n (count +8) column m (0-6) of the
// rows of 8 at +4 (column 0 their second dword). -1 out of range.
static volatile long g_table_entries;
static uint32_t __thiscall table_entry_45ef10(const uint8_t *self, int32_t n, int32_t m) {
    g_table_entries++;
    if (n >= 0x10000) {
        const int32_t k = n - 0x10000;
        return *(const int32_t *)(self + 0x10) > k ? (*(const uint32_t *const *)(self + 0xc))[k] : 0xffffffff;
    }
    if (m >= 7 || m < 0 || n < 0 || n >= *(const int32_t *)(self + 8)) return 0xffffffff;
    return (*(const uint32_t *const *)(self + 4))[m + n * 8 + 1];
}
static const uint8_t table_entry_45ef10_original[] = {0x8b, 0x44, 0x24, 0x04, 0x3d, 0x00, 0x00, 0x01, 0x00, 0x7d, 0x23};
REWRITE("Table::Entry45EF10", 0x45ef10, table_entry_45ef10_original, &table_entry_45ef10, &g_table_entries);

// ---- Toolbar::Find79C050(mask in ecx) (0x79C050): how many of the game's toolbar items have a flag of mask (+0xB4) ----
// The items 0x1306BC8[0x1306BE0 .. 0x1306BE4) (empty places skipped); 0 when 0x1306BCC equals 0x1306BD8.
static volatile long g_toolbar_counts;
static uint32_t __fastcall toolbar_find_79c050(uint32_t mask) {
    g_toolbar_counts++;
    const volatile uint32_t *const g = (const volatile uint32_t *)0x1306bc8;   // g[1] 0x1306BCC, g[4] 0x1306BD8
    if (g[1] == g[4]) return 0;
    uint32_t n = 0;
    const uint32_t *p = (const uint32_t *)(uintptr_t)(g[0] + g[6] * 4), *const end = (const uint32_t *)(uintptr_t)(g[0] + g[7] * 4);
    for (; p < end; p++)
        if (*p && *(const uint32_t *)(uintptr_t)(*p + 0xb4) & mask) n++;
    return n;
}
static const uint8_t toolbar_find_79c050_original[] = {0x8b, 0x15, 0xcc, 0x6b, 0x30, 0x01, 0x56};
REWRITE("Toolbar::Find79C050", 0x79c050, toolbar_find_79c050_original, &toolbar_find_79c050, &g_toolbar_counts);

// ---- Bird::ClearA1CDA0 (0xA1CDA0): takes the node at +0x88 out of its ring ({next, back}) ----------------------------
// Only when its neighbours point back at it (the game spins otherwise); then its links are cleared.
static volatile long g_bird_unlinks;
static void __thiscall bird_clear_a1cda0(const uint8_t *self) {
    g_bird_unlinks++;
    for (;;) {
        __asm__ volatile("" ::: "memory");   // (the game's loop; it may spin, so it must not be assumed to end)
        uint32_t *const l = *(uint32_t *const *)(self + 0x88);
        if (!l[0] || !l) return;
        uint32_t *const next = (uint32_t *)(uintptr_t)l[0], *const back = (uint32_t *)(uintptr_t)l[1];
        if (!next || !l || !back || !next || next[1] != (uint32_t)(uintptr_t)l || back[0] != (uint32_t)(uintptr_t)l) continue;
        next[1] = (uint32_t)(uintptr_t)back;
        ((uint32_t *)(uintptr_t)l[1])[0] = l[0];
        l[0] = 0, l[1] = 0;
    }
}
static const uint8_t bird_clear_a1cda0_original[] = {0x56, 0x8b, 0x81, 0x88, 0x00, 0x00, 0x00, 0x83, 0x38, 0x00};
REWRITE("Bird::ClearA1CDA0", 0xa1cda0, bird_clear_a1cda0_original, &bird_clear_a1cda0, &g_bird_unlinks);

// ---- Scenery::Min8439E0(o) (0x8439E0): grows a box {x0, y0, x1, y1, z0, z1} to take in o's (signed) -----------------
static volatile long g_scenery_boxes;
static void __thiscall scenery_min_8439e0(int32_t *self, const int32_t *o) {
    g_scenery_boxes++;
    if (o[0] < self[0]) self[0] = o[0];
    if (o[1] < self[1]) self[1] = o[1];
    if (o[2] > self[2]) self[2] = o[2];
    if (o[3] > self[3]) self[3] = o[3];
    if (o[4] < self[4]) self[4] = o[4];
    if (o[5] > self[5]) self[5] = o[5];
}
static const uint8_t scenery_min_8439e0_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0x10, 0x56, 0x3b, 0x11};
REWRITE("Scenery::Min8439E0", 0x8439e0, scenery_min_8439e0_original, &scenery_min_8439e0, &g_scenery_boxes);

// ---- WideString::FindChar(ch, from) (0xBCED20): the place of the wide character ch from from on, or -1 ----------------
// (-1 also when from is past the length +4, unsigned)
static volatile long g_wide_char_finds;
static int32_t __thiscall wide_string_find_char(const uint8_t *const *self, uint32_t ch, uint32_t from) {
    g_wide_char_finds++;
    const uint8_t *const t = *self;
    if (from > *(const uint32_t *)(t + 4)) return -1;
    const uint16_t *const buf = *(const uint16_t *const *)(t + 0xc);
    for (const uint16_t *p = buf + from; *p; p++)
        if (*p == (uint16_t)ch) return (int32_t)((uintptr_t)p - (uintptr_t)buf) >> 1;
    return -1;
}
static const uint8_t wide_string_find_char_original[] = {0x8b, 0x01, 0x8b, 0x4c, 0x24, 0x08, 0x3b, 0x48, 0x04, 0x76, 0x06};
REWRITE("WideString::FindChar", 0xbced20, wide_string_find_char_original, &wide_string_find_char, &g_wide_char_finds);

// ---- Loop::PartCDEF80 (0xCDEF80): steps to the next used place of a table ----------------------------------------------
// The table (+0): places +0, from +0x18 (where a place below 0 at +8 starts) up to +0x1C. The place (+8) moves on by
// one, then past empty places; +4 = the item found there, or 0.
static volatile long g_loop_steps;
static void __thiscall loop_part_cdef80(int32_t *self) {
    g_loop_steps++;
    const uint8_t *const t = (const uint8_t *)(uintptr_t)self[0];
    if (!t) return;
    self[1] = 0;
    self[2] = self[2] < 0 ? *(const int32_t *)(t + 0x18) : self[2] + 1;
    while (self[2] < *(const int32_t *)((const uint8_t *)(uintptr_t)self[0] + 0x1c)) {
        const uint32_t v = (*(const uint32_t *const *)t)[self[2]];
        if (v) {
            self[1] = (int32_t)v;
            return;
        }
        self[2]++;
    }
}
static const uint8_t loop_part_cdef80_original[] = {0x8b, 0x11, 0x85, 0xd2, 0x74, 0x44, 0x8b, 0x41, 0x08};
REWRITE("Loop::PartCDEF80", 0xcdef80, loop_part_cdef80_original, &loop_part_cdef80, &g_loop_steps);

// ---- Graph::FindAC5A00(o) (0xAC5A00): 1 (only al) when an entry (0x24 bytes at +0x40, count +0x44; from the last back)
// matches o: its +8 dword is o's +8, its byte +0x20 o's +0x20 and its byte +0x21 o's +0x21; else 0 --------------------
static volatile long g_graph_finds;
static uint8_t __thiscall graph_find_ac5a00(const uint8_t *self, const uint8_t *o) {
    g_graph_finds++;
    const uint8_t *const list = *(const uint8_t *const *)(self + 0x40);
    for (int32_t i = *(const int32_t *)(self + 0x44) - 1; i >= 0; i--) {
        const uint8_t *const e = list + i * 0x24 + 8;
        if (e[0x19] == o[0x21] && *(const uint32_t *)e == *(const uint32_t *)(o + 8) && e[0x18] == o[0x20]) return 1;
    }
    return 0;
}
static const uint8_t graph_find_ac5a00_original[] = {0x8b, 0x41, 0x44, 0x48, 0x53, 0x56, 0x57, 0x78, 0x33};
REWRITE("Graph::FindAC5A00", 0xac5a00, graph_find_ac5a00_original, &graph_find_ac5a00, &g_graph_finds);

// ---- WideString::FindBCED70(ch, from) (0xBCED70): as WideString::FindChar for ch's low byte -----------------------------
static volatile long g_wide_byte_finds;
static int32_t __thiscall wide_string_find_bced70(const uint8_t *const *self, uint32_t ch, uint32_t from) {
    g_wide_byte_finds++;
    const uint8_t *const t = *self;
    if (from > *(const uint32_t *)(t + 4)) return -1;
    const uint16_t *const buf = *(const uint16_t *const *)(t + 0xc);
    for (const uint16_t *p = buf + from; *p; p++)
        if (*p == (uint8_t)ch) return (int32_t)((uintptr_t)p - (uintptr_t)buf) >> 1;
    return -1;
}
static const uint8_t wide_string_find_bced70_original[] = {0x33, 0xc0, 0x8a, 0x44, 0x24, 0x04, 0x56, 0x25, 0xff, 0x00, 0x00, 0x00};
REWRITE("WideString::FindBCED70", 0xbced70, wide_string_find_bced70_original, &wide_string_find_bced70, &g_wide_byte_finds);

// ---- Labels::Shared475C10(v) (0x475C10) ----------------------------------------------------------------------------
// The game's 0x10ECA08 = v; then for each label on the game's list (0x10ECA14): a label whose turn (+0x64) is not the
// game's 0x12F0144 takes it and has +0x1C and +0x70 cleared; +0x1C = old +0x1C + +0x1C (a float: doubled when it kept
// its turn).
static volatile long g_label_turns;
static void __thiscall labels_shared_475c10(const void *, uint32_t v) {
    g_label_turns++;
    *(volatile uint32_t *)0x10eca08 = v;
    const uint32_t *p = *(const uint32_t *const volatile *)0x10eca14;
    if (!p[0] || !p) return;
    for (;;) {
        uint8_t *const label = (uint8_t *)(uintptr_t)p[2];
        const uint32_t turn = *(const volatile uint32_t *)0x12f0144;
        const long double old = *(const float *)(label + 0x1c);
        if (*(const uint32_t *)(label + 0x64) != turn) {
            *(uint32_t *)(label + 0x64) = turn;
            *(uint32_t *)(label + 0x1c) = 0;
            *(uint32_t *)(label + 0x70) = 0;
        }
        *(volatile float *)(label + 0x1c) = (float)(old + *(const float *)(label + 0x1c));
        p = (const uint32_t *)(uintptr_t)p[0];
        if (!p[0] || !p) return;
    }
}
static const uint8_t labels_shared_475c10_original[] = {0x8b, 0x44, 0x24, 0x04, 0x8b, 0x0d, 0x14, 0xca, 0x0e, 0x01};
REWRITE("Labels::Shared475C10", 0x475c10, labels_shared_475c10_original, &labels_shared_475c10, &g_label_turns);

// ---- Window::Flag683770(c) (0x683770): 1 (only al) when flagged (bit 0 of byte +0x97) and the part's (+0x130) wide text
// (count +4, characters +0xC) holds c (signed byte); else 0 ------------------------------------------------------------
static volatile long g_window_text_has;
static uint8_t __thiscall window_flag_683770(const uint8_t *self, uint32_t c) {
    g_window_text_has++;
    if (!(self[0x97] & 1)) return 0;
    const uint8_t *const p = *(const uint8_t *const *)(self + 0x130);
    const uint32_t n = *(const uint32_t *)(p + 4);
    const int32_t want = (int8_t)c;
    for (uint32_t i = 0; i < n; i++)
        if ((int32_t)(*(const uint16_t *const *)(p + 0xc))[i] == want) return 1;
    return 0;
}
static const uint8_t window_flag_683770_original[] = {0xf6, 0x81, 0x97, 0x00, 0x00, 0x00, 0x01, 0x53, 0x56, 0x57};
REWRITE("Window::Flag683770", 0x683770, window_flag_683770_original, &window_flag_683770, &g_window_text_has);

// ---- Zip::CheckEndCB0B60 (0xCB0B60): 1 for a zip end record that fits ------------------------------------------------
// Signature 0x06054B50, disk 0 (+4) and directory disk 0 (+6), entries on this disk (+8) = all entries (+0xA), no comment
// (+0x14), and the directory offset (+0xC) = the file's end place (+0x18's +0x44, -1 counting as 0) - its size (+0x10) -
// 22.
static volatile long g_zip_checks;
static uint32_t __thiscall zip_check_end_cb0b60(const uint8_t *self) {
    g_zip_checks++;
    if (*(const uint32_t *)self != 0x6054b50 || *(const uint16_t *)(self + 4) || *(const uint16_t *)(self + 8) != *(const uint16_t *)(self + 0xa) ||
        *(const uint16_t *)(self + 6) || *(const uint16_t *)(self + 0x14))
        return 0;
    uint32_t end = *(const uint32_t *)(*(const uint8_t *const *)(self + 0x18) + 0x44);
    if (end == 0xffffffff) end = 0;
    return *(const uint32_t *)(self + 0xc) == end - *(const uint32_t *)(self + 0x10) - 0x16 ? 1 : 0;
}
static const uint8_t zip_check_end_cb0b60_original[] = {0x81, 0x39, 0x50, 0x4b, 0x05, 0x06, 0x75, 0x43};
REWRITE("Zip::CheckEndCB0B60", 0xcb0b60, zip_check_end_cb0b60_original, &zip_check_end_cb0b60, &g_zip_checks);

// ---- Person::Shared7E9130 (0x7E9130): a weight over the game's list (0x10F0620) of people -----------------------------
// Each adds by its kind (+0x414): 1 adds 1.5, 2 adds 2, 3 adds 3, others 1 (summed unrounded from 0).
static volatile long g_person_weights;
static long double __stdcall person_shared_7e9130() {
    g_person_weights++;
    long double sum = *(const float *)0xf264e0;
    const uint32_t *p = *(const uint32_t *const volatile *)0x10f0620;
    if (!p[0] || !p) return sum;
    for (;;) {
        switch (*(const int32_t *)(p[2] + 0x414)) {
        case 1: sum = sum + *(const float *)0xf26648; break;
        case 2: sum = sum + *(const float *)0xf2705c; break;
        case 3: sum = sum + *(const float *)0xf2784c; break;
        default: sum = sum + *(const float *)0xf264dc; break;
        }
        p = (const uint32_t *)(uintptr_t)p[0];
        if (!p[0] || !p) return sum;
    }
}
static const uint8_t person_shared_7e9130_original[] = {0xa1, 0x20, 0x06, 0x0f, 0x01, 0xd9, 0x05, 0xe0, 0x64, 0xf2, 0x00};
REWRITE("Person::Shared7E9130", 0x7e9130, person_shared_7e9130_original, &person_shared_7e9130, &g_person_weights);

// ---- Water::Remove95A570(a, b) (0x95A570): the edge between a and b in a's chain -------------------------------------
// From index (+0x20)[a] along 16-byte edges (+8) {from, to, .., next for from +8, next for to +0xA} (words): the first
// edge from a to b or from b; gives its index (a word), 0xFFFF at the chain's end.
static volatile long g_water_edge_finds;
static uint32_t __thiscall water_remove_95a570(const uint8_t *self, uint32_t a_in, uint32_t b_in) {
    g_water_edge_finds++;
    const uint16_t a = (uint16_t)a_in, b = (uint16_t)b_in;
    uint16_t i = (*(const uint16_t *const *)(self + 0x20))[a];
    while (i != 0xffff) {
        const uint16_t *const e = (const uint16_t *)(*(const uint8_t *const *)(self + 8) + i * 16);
        if (e[0] == a) {
            if (e[1] == b) break;
            i = e[4];
        } else {
            if (e[0] == b) break;
            i = e[5];
        }
    }
    return i;
}
static const uint8_t water_remove_95a570_original[] = {0x8b, 0x51, 0x20, 0x56, 0x66, 0x8b, 0x74, 0x24, 0x08};
REWRITE("Water::Remove95A570", 0x95a570, water_remove_95a570_original, &water_remove_95a570, &g_water_edge_finds);

// ---- GridArea::ValueAt, PartAt, ValueAt2(x, y) (0xABC6D0, 0xABC730, 0xABC790) -------------------------------------------
// The cell at x, y inside the area {x0 +0x4C, y0 +0x50, x1 +0x54, y1 +0x58} (cells of 0x58 bytes at +0x5C, rows x1 - x0 +
// 1 wide): ValueAt gives its +0x4C when its part's (+0x3C) +0xC is set, else 0; PartAt that +0xC (0 outside);
// ValueAt2 its +0x50 (-1 outside).
static volatile long g_grid_cells;
template <int Kind> static uint32_t __thiscall grid_area_at(const uint8_t *self, int32_t x, int32_t y) {
    g_grid_cells++;
    const int32_t *const b = (const int32_t *)(self + 0x4c);
    const uint32_t outside = Kind == 2 ? 0xffffffff : 0;
    if (x < b[0] || x > b[2] || y < b[1] || y > b[3]) return outside;
    const uint32_t i = (uint32_t)(b[2] - b[0] + 1) * (uint32_t)(y - b[1]) - (uint32_t)b[0] + (uint32_t)x;
    const uint8_t *const cell = *(const uint8_t *const *)(self + 0x5c) + i * 0x58;
    if (!cell) return outside;
    if (Kind == 2) return *(const uint32_t *)(cell + 0x50);
    const uint32_t part = *(const uint32_t *)(*(const uint8_t *const *)(cell + 0x3c) + 0xc);
    if (Kind == 1) return part;
    return part ? *(const uint32_t *)(cell + 0x4c) : 0;
}
static const uint8_t grid_area_at_original[] = {0x8b, 0x41, 0x4c, 0x53, 0x8b, 0x5c, 0x24, 0x08, 0x3b, 0xd8, 0x56, 0x57};
REWRITE("GridArea::ValueAt", 0xabc6d0, grid_area_at_original, &grid_area_at<0>, &g_grid_cells);
REWRITE("GridArea::PartAt", 0xabc730, grid_area_at_original, &grid_area_at<1>, &g_grid_cells);
REWRITE("GridArea::ValueAt2", 0xabc790, grid_area_at_original, &grid_area_at<2>, &g_grid_cells);

// ---- List::Count6C3CC0(v) (0x6C3CC0): whether every sub-list of a list (in eax: {sub-lists, count}) holds v ------------
// Each sub-list {values, count}; 0 (only al) for an empty list or when one lacks v, else 1.
static volatile long g_list_all_have;
extern "C" uint8_t __stdcall forever_list_all_have(const uint32_t *list, uint32_t v) {
    g_list_all_have++;
    const int32_t n = (int32_t)list[1];
    if (!n) return 0;
    for (int32_t i = 0; i < n; i++) {
        const uint32_t *const sub = (const uint32_t *)(uintptr_t)((const uint32_t *)(uintptr_t)list[0])[i];
        const int32_t m = (int32_t)sub[1];
        int32_t j = 0;
        while (j < m && ((const uint32_t *)(uintptr_t)sub[0])[j] != v) j++;
        if (j >= m) return 0;
    }
    return 1;
}
extern "C" __attribute__((naked)) void forever_list_all_have_regs() {   // list in eax; v on the stack
    __asm__("pushl 4(%esp)\n\tpushl %eax\n\tcalll _forever_list_all_have@8\n\tret $4");
}
static const uint8_t list_count_6c3cc0_original[] = {0x53, 0x8b, 0x58, 0x04, 0x85, 0xdb, 0x55, 0x8b, 0x6c, 0x24, 0x0c};
REWRITE("List::Count6C3CC0", 0x6c3cc0, list_count_6c3cc0_original, (void *)&forever_list_all_have_regs, &g_list_all_have);

// ---- Values4::Snap(tolerance) (0x8E0C30): makes near-equal values of four equal -------------------------------------
// Each value after the first becomes the first earlier kept value it is within tolerance of (|difference| below it);
// one that matches none is kept for the later ones.
static volatile long g_value_snaps;
static void __thiscall values4_snap(uint32_t *self, float tolerance) {
    g_value_snaps++;
    uint32_t kept[4];
    kept[0] = self[0];
    int32_t n = 1;
    for (int j = 1; j < 4; j++) {
        int32_t i = 0;
        for (; i < n; i++)
            if (__builtin_fabsl((long double)*(const float *)&self[j] - *(const float *)&kept[i]) < tolerance) break;
        if (i < n) self[j] = kept[i];
        else kept[n++] = self[j];
    }
}
static const uint8_t values4_snap_original[] = {0x83, 0xec, 0x10, 0x8b, 0x01, 0x56, 0x57, 0x89, 0x44, 0x24, 0x08};
REWRITE("Values4::Snap", 0x8e0c30, values4_snap_original, &values4_snap, &g_value_snaps);

// ---- Animal::EntryA6BED0(o) (0xA6BED0): takes o out of its list --------------------------------------------------------
// The list is o's +0x14'th (counting from 1) of the 12-byte {values, count} lists at +0x4C; the first o found is removed,
// the later values moving down by one.
static volatile long g_animal_list_removes;
static void __thiscall animal_entry_a6bed0(const uint8_t *self, const uint8_t *o) {
    g_animal_list_removes++;
    uint32_t *const list = (uint32_t *)(*(uint8_t *const *)(self + 0x4c) + *(const int32_t *)(o + 0x14) * 12 - 0xc);
    const int32_t n = (int32_t)list[1];
    int32_t i = 0;
    if (n > 0)
        while (i < (int32_t)list[1] && ((const uint32_t *)(uintptr_t)list[0])[i] != (uint32_t)(uintptr_t)o) i++;
    if (i >= n) return;
    if (i < n - 1)
        for (; i < (int32_t)list[1] - 1; i++) ((uint32_t *)(uintptr_t)list[0])[i] = ((uint32_t *)(uintptr_t)list[0])[i + 1];
    list[1]--;
}
static const uint8_t animal_entry_a6bed0_original[] = {0x8b, 0x49, 0x4c, 0x56, 0x57, 0x8b, 0x7c, 0x24, 0x0c};
REWRITE("Animal::EntryA6BED0", 0xa6bed0, animal_entry_a6bed0_original, &animal_entry_a6bed0, &g_animal_list_removes);

// ---- Table::RemoveCE3100, RemoveCE3250(a, b) (0xCE3100: index +8, edges of 16 at +0x14; 0xCE3250: index +0xC, edges of
// 12 at +0x38): the first edge on a's chain that touches b ------------------------------------------------------------
// From index[a] along edges {p, q, .., next for p +8, next for q +0xA} (words): stops at an edge with p or q = b and
// gives its index; otherwise goes on by the link of the end that is a. 0xFFFF at the chain's end.
static volatile long g_table_edges;
template <uint32_t Index, uint32_t Edges, uint32_t Size> static uint32_t __thiscall table_edge_find(const uint8_t *self, uint32_t a_in, uint32_t b_in) {
    g_table_edges++;
    const uint16_t a = (uint16_t)a_in, b = (uint16_t)b_in;
    uint16_t i = (*(const uint16_t *const *)(self + Index))[a];
    while (i != 0xffff) {
        __asm__ volatile("" ::: "memory");   // (the game spins on an edge touching neither a nor b)
        const uint16_t *const e = (const uint16_t *)(*(const uint8_t *const *)(self + Edges) + i * Size);
        if (e[0] == b || e[1] == b) break;
        if (e[0] == a) i = e[4];
        else if (e[1] == a) i = e[5];
    }
    return i;
}
static const uint8_t table_remove_ce3100_original[] = {0x8b, 0x51, 0x08, 0x55, 0x66, 0x8b, 0x6c, 0x24, 0x08};
REWRITE("Table::RemoveCE3100", 0xce3100, table_remove_ce3100_original, (&table_edge_find<8, 0x14, 16>), &g_table_edges);
static const uint8_t table_remove_ce3250_original[] = {0x8b, 0x51, 0x0c, 0x55, 0x66, 0x8b, 0x6c, 0x24, 0x08};
REWRITE("Table::RemoveCE3250", 0xce3250, table_remove_ce3250_original, (&table_edge_find<0xc, 0x38, 12>), &g_table_edges);

// ---- TypeLists::Contains(t) (0xBD2780): 1 (only al) when an item of the list at +0x14 has t's count (+0x10) and values ---
static volatile long g_type_list_has;
static uint8_t __thiscall type_lists_contains(const uint8_t *self, const uint32_t *t) {
    g_type_list_has++;
    const uint32_t *p = *(const uint32_t *const *)(self + 0x14);
    if (!p[0] || !p) return 0;
    for (;;) {
        const uint32_t *const item = (const uint32_t *)(uintptr_t)p[2];
        const int32_t n = (int32_t)item[4];
        if ((uint32_t)n == t[4]) {
            int32_t k = 0;
            while (k < n && item[k] == t[k]) k++;
            if (k >= n) return 1;
        }
        p = (const uint32_t *)(uintptr_t)p[0];
        if (!p[0] || !p) return 0;
    }
}
static const uint8_t type_lists_contains_original[] = {0x53, 0x55, 0x56, 0x57, 0x8b, 0x79, 0x14, 0x83, 0x3f, 0x00};
REWRITE("TypeLists::Contains", 0xbd2780, type_lists_contains_original, &type_lists_contains, &g_type_list_has);

// ---- Car::Kind50E6D0 (0x50E6D0) --------------------------------------------------------------------------------------
// With a count at +0x7B4: 3 (4 when +0x8C0 is above 0) when flag bit 3 of +0x39C is set, else 2. Without: with the flag
// 1 when +0x72C or +0x730 is below 0, else 3; without the flag 0.
static volatile long g_car_kinds;
static uint32_t __thiscall car_kind_50e6d0(const uint8_t *self) {
    g_car_kinds++;
    const bool flag = *(const uint32_t *)(self + 0x39c) >> 3 & 1;
    if (*(const int32_t *)(self + 0x7b4) > 0) return flag ? 3 + (*(const int32_t *)(self + 0x8c0) > 0) : 2;
    if (!flag) return 0;
    return *(const int32_t *)(self + 0x72c) < 0 || *(const int32_t *)(self + 0x730) < 0 ? 1 : 3;
}
static const uint8_t car_kind_50e6d0_original[] = {0x8b, 0x81, 0xb4, 0x07, 0x00, 0x00, 0x85, 0xc0, 0x7f, 0x30};
REWRITE("Car::Kind50E6D0", 0x50e6d0, car_kind_50e6d0_original, &car_kind_50e6d0, &g_car_kinds);

// ---- Slots::ValueFor(o) (0x772990): the value for o's id (+0x20, +0x24) among three slots (+0xAC, +0xC0, +0xD4: each an
// id pair; values +0x9C, +0xA0, +0xA4), or the default (+0x78) ---------------------------------------------------------
static volatile long g_slot_values;
static long double __thiscall slots_value_for(const uint8_t *self, const uint8_t *o) {
    g_slot_values++;
    const uint32_t a = *(const uint32_t *)(o + 0x20), b = *(const uint32_t *)(o + 0x24);
    for (int k = 0; k < 3; k++) {
        const uint32_t *const id = *(const uint32_t *const *)(self + 0xac + k * 0x14);
        if (id[0] == a && id[1] == b) return *(const float *)(self + 0x9c + k * 4);
    }
    return *(const float *)(self + 0x78);
}
static const uint8_t slots_value_for_original[] = {0x8b, 0x54, 0x24, 0x04, 0xd9, 0x41, 0x78, 0x8b, 0x42, 0x20};
REWRITE("Slots::ValueFor", 0x772990, slots_value_for_original, &slots_value_for, &g_slot_values);

// ---- Effect::Next7B5070 (0x7B5070): in mode 1 (+0x3C) and with +0x20 not +0x2C, each item in places +0x34 ..
// +0x38 of the list at +0x1C (read again each time) gets +0xBC = 0 and its frame (+0xB8) one on, from 0 to 347 ----------
static volatile long g_effect_frames;
static void __thiscall effect_next_7b5070(const uint8_t *self) {
    g_effect_frames++;
    if (*(const int32_t *)(self + 0x3c) != 1 || *(const uint32_t *)(self + 0x20) == *(const uint32_t *)(self + 0x2c)) return;
    for (uintptr_t p = *(const uint32_t *)(self + 0x1c) + *(const int32_t *)(self + 0x34) * 4;
         p < *(const volatile uint32_t *)(self + 0x1c) + *(const volatile int32_t *)(self + 0x38) * 4; p += 4) {
        uint8_t *const e = *(uint8_t *const *)p;
        if (!e) continue;
        *(uint32_t *)(e + 0xbc) = 0;
        *(int32_t *)(e + 0xb8) = (*(const int32_t *)(e + 0xb8) + 1) % 0x15c;
    }
}
static const uint8_t effect_next_7b5070_original[] = {0x83, 0x79, 0x3c, 0x01, 0x75, 0x58, 0x8b, 0x41, 0x20};
REWRITE("Effect::Next7B5070", 0x7b5070, effect_next_7b5070_original, &effect_next_7b5070, &g_effect_frames);

// ---- List::Find45EEA0(v) (0x45EEA0): where v is: the row i (rows of 8 at +4, count +8; v among a row's 7 values after
// its first) or 0x10000 + k for place k of the list at +0xC (count +0x10); -1 when nowhere -----------------------------
static volatile long g_list_finds2;
static int32_t __thiscall list_find_45eea0(const uint8_t *self, uint32_t v) {
    g_list_finds2++;
    const int32_t rows = *(const int32_t *)(self + 8);
    const uint32_t *const r = *(const uint32_t *const *)(self + 4);
    for (int32_t i = 0; i < rows; i++)
        for (int j = 0; j < 7; j++)
            if (r[i * 8 + 1 + j] == v) return i;
    const int32_t n = *(const int32_t *)(self + 0x10);
    const uint32_t *const l = *(const uint32_t *const *)(self + 0xc);
    for (int32_t k = 0; k < n; k++)
        if (l[k] == v) return k + 0x10000;
    return -1;
}
static const uint8_t list_find_45eea0_original[] = {0x53, 0x8b, 0x59, 0x08, 0x55, 0x8b, 0x6c, 0x24, 0x0c, 0x56};
REWRITE("List::Find45EEA0", 0x45eea0, list_find_45eea0_original, &list_find_45eea0, &g_list_finds2);

// ---- Track::CheckBC6D00(flag) (0xBC6D00): the ride in eax, another in esi -------------------------------------------
// 0 (only al) unless the ride has flag 0x400000 (+0x18), at least two ends (+0xE4) and both end lists (+0x10C, +0x110).
// Then with flag (a byte): bit 29 of the other's +0x18 when the ride has 0x10000000, else 1; without: bit 27 when it has
// 0x4000000, else 1.
static volatile long g_track_checks;
extern "C" uint8_t __stdcall forever_track_check(const uint8_t *ride, const uint8_t *other, uint32_t flag) {
    g_track_checks++;
    const uint32_t f = *(const uint32_t *)(ride + 0x18);
    if (!(f & 0x400000) || *(const int32_t *)(ride + 0xe4) < 2 || !*(const uint32_t *)(ride + 0x10c) || !*(const uint32_t *)(ride + 0x110)) return 0;
    if ((uint8_t)flag) return f & 0x10000000 ? *(const uint32_t *)(other + 0x18) >> 29 & 1 : 1;
    return f & 0x4000000 ? *(const uint32_t *)(other + 0x18) >> 27 & 1 : 1;
}
extern "C" __attribute__((naked)) void forever_track_check_regs() {   // ride eax, other esi; flag on the stack
    __asm__("pushl 4(%esp)\n\tpushl %esi\n\tpushl %eax\n\tcalll _forever_track_check@12\n\tret $4");
}
static const uint8_t track_check_bc6d00_original[] = {0x8b, 0x50, 0x18, 0xf7, 0xc2, 0x00, 0x00, 0x40, 0x00};
REWRITE("Track::CheckBC6D00", 0xbc6d00, track_check_bc6d00_original, (void *)&forever_track_check_regs, &g_track_checks);

// ---- Items::SetValueAt(n, v) (0xC64CE0): item n of the owner's (+0xF0) list (+0xB0) gets +0xD0 = v; +0x9C = 0 --------
// (The game writes through 0 past the list's end.)
static volatile long g_item_sets;
static void __thiscall items_set_value_at(uint8_t *self, int32_t n, uint32_t v) {
    g_item_sets++;
    self[0x9c] = 0;
    const uint32_t *p = *(const uint32_t *const *)(*(const uint8_t *const *)(self + 0xf0) + 0xb0);
    uint8_t *item = nullptr;
    if (p[0] && p) {
        for (;;) {
            if (!n) {
                item = (uint8_t *)(uintptr_t)p[2];
                break;
            }
            p = (const uint32_t *)(uintptr_t)p[0];
            n--;
            if (!p[0] || !p) break;
        }
    }
    *(volatile uint32_t *)(item + 0xd0) = v;
}
static const uint8_t items_set_value_at_original[] = {0x8b, 0x81, 0xf0, 0x00, 0x00, 0x00, 0x8b, 0x54, 0x24, 0x04};
REWRITE("Items::SetValueAt", 0xc64ce0, items_set_value_at_original, &items_set_value_at, &g_item_sets);

// ---- SortedPointers::Find64(a, b, out) (0xCDC420): binary search of objects (+0, count +4) sorted by their first two
// dwords (unsigned): 1 (only al) and out = the place when found, else 0 and out = where it would go -----------------------
static volatile long g_sorted_pair_finds;
static uint8_t __thiscall sorted_pointers_find64(const uint32_t *self, uint32_t a, uint32_t b, int32_t *out) {
    g_sorted_pair_finds++;
    int32_t lo = 0, hi = (int32_t)self[1] - 1;
    const uint32_t *const *const list = (const uint32_t *const *)(uintptr_t)self[0];
    while (lo <= hi) {
        const int32_t mid = (hi - lo) / 2 + lo;
        const uint32_t *const e = list[mid];
        if (a > e[0] || (a == e[0] && b > e[1])) lo = mid + 1;
        else if (a < e[0] || b < e[1]) hi = mid - 1;
        else {
            *out = mid;
            return 1;
        }
    }
    *out = lo;
    return 0;
}
static const uint8_t sorted_pointers_find64_original[] = {0x53, 0x55, 0x56, 0x57, 0x8b, 0x79, 0x04, 0x33, 0xf6};
REWRITE("SortedPointers::Find64", 0xcdc420, sorted_pointers_find64_original, &sorted_pointers_find64, &g_sorted_pair_finds);

// ---- Entries::EarlierSame(n) (0x827E00): 1 (only al) when an entry before n (0x58 bytes at the +0x9C part's +0x38) has
// n's id (+0x54) and either of its two bits set in +0x134; 0 otherwise or with no part --------------------------------
static volatile long g_earlier_sames;
static uint8_t __thiscall entries_earlier_same(const uint8_t *self, int32_t n) {
    g_earlier_sames++;
    const uint8_t *const p = *(const uint8_t *const *)(self + 0x9c);
    if (!p) return 0;
    const uint8_t *const list = *(const uint8_t *const *)(p + 0x38);
    const uint32_t id = *(const uint32_t *)(list + n * 0x58 + 0x54), mask = *(const uint32_t *)(self + 0x134);
    for (int32_t i = 0; i < n; i++)
        if (*(const uint32_t *)(list + i * 0x58 + 0x54) == id && mask & (3u << (2 * i & 31))) return 1;
    return 0;
}
static const uint8_t entries_earlier_same_original[] = {0x53, 0x8b, 0xd9, 0x8b, 0x83, 0x9c, 0x00, 0x00, 0x00};
REWRITE("Entries::EarlierSame", 0x827e00, entries_earlier_same_original, &entries_earlier_same, &g_earlier_sames);

// ---- NamedList::Find2(name) (0xBD5110): the first entry (from +0x204, next +0x30) whose name (+0x24) is name, or 0 ------
static volatile long g_named_finds;
static uint8_t *__thiscall named_list_find2(const uint8_t *self, const char *name) {
    g_named_finds++;
    for (uint8_t *e = *(uint8_t *const *)(self + 0x204); e; e = *(uint8_t *const *)(e + 0x30)) {
        const char *a = name, *b = *(const char *const *)(e + 0x24);
        while (*a == *b && *a) a++, b++;
        if (*a == *b) return e;
    }
    return nullptr;
}
static const uint8_t named_list_find2_original[] = {0x53, 0x55, 0x56, 0x57, 0x8b, 0xb9, 0x04, 0x02, 0x00, 0x00};
REWRITE("NamedList::Find2", 0xbd5110, named_list_find2_original, &named_list_find2, &g_named_finds);

// ---- SortedTable::LowerBoundKind(n, kind) (0x88F9A0), SortedTable::FindKind(n, kind, out) (0x88FA10) --------------------
// Binary searches of tables sorted by kind (0 first), then number (unsigned). LowerBoundKind: entries of 0x150 bytes at
// +0x14 (count +0x18; kind the low bit of byte +0, number +0xC); gives the place after the last not above. FindKind:
// entries of 12 at +0x50 (count +0x54; kind byte +4, number +0); 1 (only al) and out = the place when found, else 0
// and out = where it would go. The kind given is compared as a whole byte; a nonzero one goes up when it differs.
static volatile long g_kind_searches;
static int32_t __thiscall sorted_table_lower_bound_kind(const uint8_t *self, uint32_t n, uint32_t kind_in) {
    g_kind_searches++;
    const uint8_t kind = (uint8_t)kind_in;
    int32_t lo = 0, hi = *(const int32_t *)(self + 0x18) - 1;
    const uint8_t *const t = *(const uint8_t *const *)(self + 0x14);
    while (lo <= hi) {
        const int32_t mid = (hi - lo) / 2 + lo;
        const uint8_t *const e = t + mid * 0x150;
        const bool up = kind == (e[0] & 1) ? !(n < *(const uint32_t *)(e + 0xc)) : kind != 0;
        if (up) lo = mid + 1;
        else hi = mid - 1;
    }
    return lo;
}
static const uint8_t sorted_table_lower_bound_kind_original[] = {0x51, 0x56, 0x57, 0x8b, 0x79, 0x18, 0x33, 0xf6, 0x4f};
REWRITE("SortedTable::LowerBoundKind", 0x88f9a0, sorted_table_lower_bound_kind_original, &sorted_table_lower_bound_kind, &g_kind_searches);
static uint8_t __thiscall sorted_table_find_kind(const uint8_t *self, uint32_t n, uint32_t kind_in, int32_t *out) {
    g_kind_searches++;
    const uint8_t kind = (uint8_t)kind_in;
    int32_t lo = 0, hi = *(const int32_t *)(self + 0x54) - 1;
    const uint8_t *const t = *(const uint8_t *const *)(self + 0x50);
    while (lo <= hi) {
        const int32_t mid = (hi - lo) / 2 + lo;
        const uint8_t *const e = t + mid * 12;
        bool up;
        if (kind == e[4]) {
            const uint32_t v = *(const uint32_t *)e;
            if (n == v) {
                *out = mid;
                return 1;
            }
            up = n > v;
        } else
            up = kind != 0;
        if (up) lo = mid + 1;
        else hi = mid - 1;
    }
    *out = lo;
    return 0;
}
static const uint8_t sorted_table_find_kind_original[] = {0x53, 0x55, 0x56, 0x57, 0x8b, 0x79, 0x54, 0x33, 0xf6, 0x4f};
REWRITE("SortedTable::FindKind", 0x88fa10, sorted_table_find_kind_original, &sorted_table_find_kind, &g_kind_searches);

// ---- Entries::Differs(a, b, c, d) (0x557E70): for the entry (16 bytes at +0x94, count +0x98) {a, b, x, d}, whether x
// is not c (0 or 1); with none, c with its low byte made c != 0 (as the game leaves it) ------------------------------------
static volatile long g_entry_differs;
static uint32_t __thiscall entries_differs(const uint8_t *self, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    g_entry_differs++;
    const int32_t n = *(const int32_t *)(self + 0x98);
    const uint32_t *const list = *(const uint32_t *const *)(self + 0x94);
    for (int32_t i = 0; i < n; i++) {
        const uint32_t *const e = list + i * 4;
        if (e[0] == a && e[1] == b && e[3] == d) return c != e[2];
    }
    return (c & ~0xffu) | (c != 0);
}
static const uint8_t entries_differs_original[] = {0x53, 0x55, 0x56, 0x8b, 0xb1, 0x98, 0x00, 0x00, 0x00};
REWRITE("Entries::Differs", 0x557e70, entries_differs_original, &entries_differs, &g_entry_differs);

// ---- List::LoopA1CE30(list) (0xA1CE30): takes each item's own node (+8 in the item) out of its ring -------------------
// For each node of the list (to the end node, next 0), the item's ring node {next, back} is unlinked when both its
// neighbours point back at it, and cleared.
static volatile long g_ring_unlinks;
static void __stdcall list_loop_a1ce30(uint32_t *const *list) {
    g_ring_unlinks++;
    const uint32_t *p = *list;
    if (!p[0] || !p) return;
    for (;;) {
        uint32_t *const n = (uint32_t *)(uintptr_t)(p[2] + 8);
        if (n) {
            uint32_t *const back = (uint32_t *)(uintptr_t)n[1], *const next = (uint32_t *)(uintptr_t)n[0];
            if (back && next && next[1] == (uint32_t)(uintptr_t)n && back[0] == (uint32_t)(uintptr_t)n) {
                next[1] = (uint32_t)(uintptr_t)back;
                ((uint32_t *)(uintptr_t)n[1])[0] = n[0];
                n[0] = 0, n[1] = 0;
            }
        }
        p = (const uint32_t *)(uintptr_t)p[0];
        if (!p[0] || !p) return;
    }
}
static const uint8_t list_loop_a1ce30_original[] = {0x8b, 0x44, 0x24, 0x04, 0x56, 0x8b, 0x30, 0x83, 0x3e, 0x00};
REWRITE("List::LoopA1CE30", 0xa1ce30, list_loop_a1ce30_original, &list_loop_a1ce30, &g_ring_unlinks);

// ---- Effects::Set47C4E0(k in ecx, v) (0x47C4E0): the game's level k (floats at 0x10EDD40) = v, then each effect on list
// k (0x10EDD60, 24 bytes each) gets its sound's +0x58 = its volume (+0x14) times its kind's (+0x18) level, or 0 when that
// kind is muted (0x12F0168) -------------------------------------------------------------------------------------------
static volatile long g_effect_levels;
static void __thiscall effects_set_47c4e0(uintptr_t k, float v) {
    g_effect_levels++;
    volatile float *const level = (volatile float *)0x10edd40;
    level[k] = v;
    const uint32_t *p = *(const uint32_t *const volatile *)(0x10edd60 + k * 24);
    if (!p[0] || !p) return;
    for (;;) {
        uint8_t *const e = (uint8_t *)(uintptr_t)p[2];
        const int32_t kind = *(const int32_t *)(e + 0x18);
        fpu_copy_h2(e + 0x14, e + 0x14);
        const long double r = ((const volatile uint32_t *)0x12f0168)[kind] ? (long double)*(const float *)0xf264e0 : (long double)*(const float *)(e + 0x14) * level[kind];
        *(volatile float *)(*(uint8_t *const *)(e + 0xc) + 0x58) = (float)r;
        p = (const uint32_t *)(uintptr_t)p[0];
        if (!p[0] || !p) return;
    }
}
static const uint8_t effects_set_47c4e0_original[] = {0xd9, 0x44, 0x24, 0x04, 0x8d, 0x04, 0x49, 0x8b, 0x14, 0xc5, 0x60, 0xdd, 0x0e, 0x01};
REWRITE("Effects::Set47C4E0", 0x47c4e0, effects_set_47c4e0_original, &effects_set_47c4e0, &g_effect_levels);

// ---- List678::Remove(v) (0x4277E0): takes the first v out of the list at +0x678 (count +0x680), the later ones moving down
static volatile long g_list678_removes;
static void __thiscall list678_remove(uint8_t *self, uint32_t v) {
    g_list678_removes++;
    const int32_t n = *(const int32_t *)(self + 0x680);
    int32_t i = 0;
    if (n > 0)
        while (i < *(const volatile int32_t *)(self + 0x680) && (*(const uint32_t *const *)(self + 0x678))[i] != v) i++;
    if (i >= n) return;
    if (i < n - 1)
        for (; i < *(const volatile int32_t *)(self + 0x680) - 1; i++) (*(uint32_t *const *)(self + 0x678))[i] = (*(uint32_t *const *)(self + 0x678))[i + 1];
    *(int32_t *)(self + 0x680) -= 1;
}
static const uint8_t list678_remove_original[] = {0x56, 0x8b, 0xb1, 0x80, 0x06, 0x00, 0x00, 0x33, 0xc0};
REWRITE("List678::Remove", 0x4277e0, list678_remove_original, &list678_remove, &g_list678_removes);

// ---- Bits::SetRange(from, to) (0xC166E0): sets the bit of every block (size +4) from from / size to (to + size - 1) /
// size (as C divides) as Bits::TestC165F0 does --------------------------------------------------------------------------
static volatile long g_bit_ranges;
static void __thiscall bits_set_range(uint8_t *self, int32_t from, int32_t to) {
    g_bit_ranges++;
    const int32_t size = *(const int32_t *)(self + 4);
    const int32_t hi = (to + size - 1) / size;
    for (int32_t i = from / size; i < hi; i++) {
        const int32_t w = i / 32;
        if (w < 0 || w >= *(const int32_t *)(self + 0x10)) continue;
        uint32_t *const word = *(uint32_t *const *)(self + 0xc) + w;
        const uint32_t mask = 1u << (i % 32 & 31);
        if (!(*word & mask)) *(int32_t *)(self + 0x14) += 1;
        *word |= mask;
    }
}
static const uint8_t bits_set_range_original[] = {0x8b, 0x44, 0x24, 0x08, 0x53, 0x56, 0x57, 0x8b, 0xf9};
REWRITE("Bits::SetRange", 0xc166e0, bits_set_range_original, &bits_set_range, &g_bit_ranges);

// ---- Object::Scale2, Object::Scale (0xBC3EB0: +0xFC, defaults 0x10F2B20 / 0x10F2B24; 0xBC3F20: +0x100, 0x10F2B28 /
// 0x10F2B2C) ---------------------------------------------------------------------------------------------------------
// The default is the first with flag 0x400000 (+0x18) and at least two ends (+0xE4), else the second. With the flag and
// more than one end a nonzero value is given as is when above 0, else as -(default value); otherwise the default.
static volatile long g_object_scales;
template <uint32_t Value, uintptr_t Defaults> static long double __thiscall object_scale(const uint8_t *self) {
    g_object_scales++;
    const bool flag = *(const uint32_t *)(self + 0x18) & 0x400000;
    const int32_t ends = *(const int32_t *)(self + 0xe4);
    const long double d = ((const volatile float *)Defaults)[flag && ends >= 2 ? 0 : 1];
    if (!flag || ends <= 1) return d;
    const float v = *(const float *)(self + Value);
    if (v == *(const float *)0xf264e0) return d;
    if (v > *(const float *)0xf264e0) return v;
    return -(d * v);
}
static const uint8_t object_scale2_original[] = {0x8b, 0x41, 0x18, 0x25, 0x00, 0x00, 0x40, 0x00, 0x74, 0x09};
REWRITE("Object::Scale2", 0xbc3eb0, object_scale2_original, (&object_scale<0xfc, 0x10f2b20>), &g_object_scales);
REWRITE("Object::Scale", 0xbc3f20, object_scale2_original, (&object_scale<0x100, 0x10f2b28>), &g_object_scales);

// ---- SortedPointers::FindFlagged(n, flagged, out) (0xCE7260): binary search of items {number, flag byte} (+0x18, count
// +0x1C) sorted by number, then unflagged first: 1 (only al) and out = the place when found, else 0 and out = where it
// would go ---------------------------------------------------------------------------------------------------------------
static volatile long g_flagged_finds;
static uint8_t __thiscall sorted_pointers_find_flagged(const uint8_t *self, uint32_t n, uint32_t flagged, int32_t *out) {
    g_flagged_finds++;
    int32_t lo = 0, hi = *(const int32_t *)(self + 0x1c) - 1;
    const uint32_t *const *const list = *(const uint32_t *const *const *)(self + 0x18);
    while (lo <= hi) {
        const int32_t mid = (hi - lo) / 2 + lo;
        const uint32_t *const e = list[mid];
        const bool f = *(const uint8_t *)(e + 1) != 0;
        bool up;
        if (n != e[0]) up = n > e[0];
        else if ((uint8_t)flagged ? f : !f) {
            *out = mid;
            return 1;
        } else
            up = (uint8_t)flagged != 0;
        if (up) lo = mid + 1;
        else hi = mid - 1;
    }
    *out = lo;
    return 0;
}
static const uint8_t sorted_pointers_find_flagged_original[] = {0x53, 0x55, 0x56, 0x57, 0x8b, 0x79, 0x1c, 0x33, 0xf6};
REWRITE("SortedPointers::FindFlagged", 0xce7260, sorted_pointers_find_flagged_original, &sorted_pointers_find_flagged, &g_flagged_finds);

// ---- List::SortBy34 (0x80EDC0): bubble-sorts the objects (+0x5C, count +0x64 read again each time) by +0x34, largest
// first (a pair swaps only when the first is below the second) ------------------------------------------------------------
static volatile long g_list_sorts;
static void __thiscall list_sort_by34(const uint8_t *self) {
    g_list_sorts++;
    for (int32_t pass = 0; pass < *(const volatile int32_t *)(self + 0x64) - 1; pass++)
        for (int32_t j = 0; j < *(const volatile int32_t *)(self + 0x64) - 1; j++) {
            uint32_t *const l = *(uint32_t *const *)(self + 0x5c);
            const float a = *(const float *)(uintptr_t)(l[j] + 0x34), b = *(const float *)(uintptr_t)(l[j + 1] + 0x34);
            if (a < b) {
                const uint32_t t = l[j];
                l[j] = l[j + 1];
                l[j + 1] = t;
            }
        }
}
static const uint8_t list_sort_by34_original[] = {0x8b, 0x41, 0x64, 0x53, 0x33, 0xdb, 0x48, 0x85, 0xc0};
REWRITE("List::SortBy34", 0x80edc0, list_sort_by34_original, &list_sort_by34, &g_list_sorts);

// ---- Grid::Range(min, max) (0x8386A0): the smallest first value (+4) and largest second value (+8) of the cells (0x14
// bytes at +0x18) of a +0x10 by +0x14 grid (signed; both counts read again each time) -----------------------------------
static volatile long g_grid_ranges;
static void __thiscall grid_range(const uint8_t *self, int32_t *min, int32_t *max) {
    g_grid_ranges++;
    for (int32_t i = 0; i < *(const volatile int32_t *)(self + 0x10); i++)
        for (int32_t j = 0; j < *(const volatile int32_t *)(self + 0x14); j++) {
            const int32_t *const c = (const int32_t *)(*(const uint8_t *const *)(self + 0x18) + (*(const int32_t *)(self + 0x10) * j + i) * 0x14);
            if (!i && !j) {
                *min = c[1];
                *max = c[2];
                continue;
            }
            if (c[1] < *min) *min = c[1];
            if (c[2] > *max) *max = c[2];
        }
}
static const uint8_t grid_range_original[] = {0x8b, 0x41, 0x10, 0x56, 0x33, 0xf6, 0x85, 0xc0, 0x7e, 0x5b};
REWRITE("Grid::Range", 0x8386a0, grid_range_original, &grid_range, &g_grid_ranges);

// ---- SortedPointers::FindXY3(x, y, out) (0x8FF150): binary search of entries (+0x3C, count +0x40) sorted by y then x
// (signed): the entry, with out (when given) = its place; 0 when none ----------------------------------------------------
static volatile long g_xy_finds;
static const int32_t *__thiscall sorted_pointers_find_xy3(const uint8_t *self, int32_t x, int32_t y, int32_t *out) {
    g_xy_finds++;
    int32_t lo = 0, hi = *(const int32_t *)(self + 0x40) - 1;
    const int32_t *const *const list = *(const int32_t *const *const *)(self + 0x3c);
    while (lo <= hi) {
        const int32_t mid = (hi - lo) / 2 + lo;
        const int32_t *const e = list[mid];
        if (y < e[1] || (y == e[1] && x < e[0])) hi = mid - 1;
        else if (y > e[1] || x > e[0]) lo = mid + 1;
        else {
            if (out) *out = mid;
            return e;
        }
    }
    return nullptr;
}
static const uint8_t sorted_pointers_find_xy3_original[] = {0x51, 0x53, 0x55, 0x56, 0x57, 0x8b, 0x79, 0x40, 0x33, 0xf6};
REWRITE("SortedPointers::FindXY3", 0x8ff150, sorted_pointers_find_xy3_original, &sorted_pointers_find_xy3, &g_xy_finds);

// ---- Vector::DeadZone (0xA17470): clears each part not within -999999.875..999999.875 (0xF6FD64, 0xF6FD60; NaNs too) ---
static volatile long g_dead_zones;
static void __thiscall vector_dead_zone(float *self) {
    g_dead_zones++;
    const float lo = *(const float *)0xf6fd64, hi = *(const float *)0xf6fd60;
    for (int k = 0; k < 3; k++)
        if (!(self[k] >= lo && self[k] <= hi)) *(volatile uint32_t *)&self[k] = 0;
}
static const uint8_t vector_dead_zone_original[] = {0xd9, 0x01, 0x33, 0xd2, 0xd8, 0x1d, 0x64, 0xfd, 0xf6, 0x00};
REWRITE("Vector::DeadZone", 0xa17470, vector_dead_zone_original, &vector_dead_zone, &g_dead_zones);

// ---- VertexDeclaration::FindBDD180(texts) (0xBDD180): 1 (only al) when both of two texts fit their slots ------------------
// Slot k (+0x18 + 4k: {word a, word b, bytes}) and text k (*texts[k]: bytes with its length at +0x20): with no slot the
// text must be empty; with one its length must be within a - b .. a and its bytes the slot's.
static volatile long g_declaration_fits;
static uint8_t __thiscall vertex_declaration_find_bdd180(const uint8_t *self, const uint8_t *const *const *texts) {
    g_declaration_fits++;
    for (int k = 0; k < 2; k++) {
        const uint8_t *const slot = *(const uint8_t *const *)(self + 0x18 + k * 4);
        const uint8_t *const text = *texts[k];
        const uint32_t len = *(const uint32_t *)(text + 0x20);
        if (!slot) {
            if (len) return 0;
            continue;
        }
        const uint16_t a = *(const uint16_t *)slot;
        if (len > a || len < (uint16_t)(a - *(const uint16_t *)(slot + 2))) return 0;
        const uint8_t *const bytes = *(const uint8_t *const *)(slot + 4);
        for (uint32_t i = 0; i < len; i++)
            if (text[i] != bytes[i]) return 0;
    }
    return 1;
}
static const uint8_t vertex_declaration_find_bdd180_original[] = {0x53, 0x8b, 0x5c, 0x24, 0x08, 0x55, 0x56, 0x57, 0x33, 0xd2};
REWRITE("VertexDeclaration::FindBDD180", 0xbdd180, vertex_declaration_find_bdd180_original, &vertex_declaration_find_bdd180, &g_declaration_fits);

// ---- TrackPiece::Compare5BF4A0(a, b) (0x5BF4A0): 1 (only al) when an item of the list at +0x304 that is on (byte +8)
// has its value (+4) within a..b (put in order first); else 0 -----------------------------------------------------------
static volatile long g_piece_ranges;
static uint8_t __thiscall track_piece_compare_5bf4a0(const uint8_t *self, float a, float b) {
    g_piece_ranges++;
    if (a > b) {
        const float t = b;
        volatile float s = a;
        b = s, a = t;
    }
    const uint32_t *p = *(const uint32_t *const *)(self + 0x304);
    if (!p[0] || !p) return 0;
    for (;;) {
        const uint8_t *const item = (const uint8_t *)(uintptr_t)p[2];
        const float v = *(const float *)(item + 4);
        if (item[8] && v >= a && v <= b) return 1;
        p = (const uint32_t *)(uintptr_t)p[0];
        if (!p[0] || !p) return 0;
    }
}
static const uint8_t track_piece_compare_5bf4a0_original[] = {0xd9, 0x44, 0x24, 0x04, 0xd8, 0x5c, 0x24, 0x08, 0xdf, 0xe0};
REWRITE("TrackPiece::Compare5BF4A0", 0x5bf4a0, track_piece_compare_5bf4a0_original, &track_piece_compare_5bf4a0, &g_piece_ranges);

// ---- Terrain::Inside7AE680(x, y) (0x7AE680): 1 (only al) when the byte cell x, y lies in the area {x0 +4, y0 +5, w +6,
// h +7} (byte sums) and its byte in the rows at +8 (the last row first) is set; else 0 --------------------------------------
static volatile long g_terrain_insides;
static uint8_t __thiscall terrain_inside_7ae680(const uint8_t *self, uint32_t x_in, uint32_t y_in) {
    g_terrain_insides++;
    const uint8_t x = (uint8_t)x_in, y = (uint8_t)y_in, x0 = self[4], y0 = self[5], w = self[6];
    const uint8_t ymax = (uint8_t)(self[7] + y0 - 1), xmax = (uint8_t)(w + x0 - 1);
    if (x < x0 || x > xmax || y < y0 || y > ymax) return 0;
    const uint8_t *const rows = *(const uint8_t *const *)(self + 8);
    return rows[(uint32_t)(ymax - y) * w - x0 + x] ? 1 : 0;
}
static const uint8_t terrain_inside_7ae680_original[] = {0x51, 0x8a, 0x41, 0x07, 0x8a, 0x51, 0x04, 0x53, 0x8a, 0x59, 0x05};
REWRITE("Terrain::Inside7AE680", 0x7ae680, terrain_inside_7ae680_original, &terrain_inside_7ae680, &g_terrain_insides);

// ---- SortedShorts::Find(list, n) (0xBB6970): the place after the last number not above the key (+0x38) in a sorted list
// of n words, searched from a guess (n times +0x2C, rounded by the FPU); a number equal to the key gives the place after
// it (the game spins when the key is not below the last number) ---------------------------------------------------------
static volatile long g_short_finds;
static uint32_t __thiscall sorted_shorts_find(const uint8_t *self, const uint16_t *list, uint32_t n) {
    g_short_finds++;
    const uint16_t key = *(const uint16_t *)(self + 0x38);
    uint32_t lo = 0, hi = n;
    uint32_t i = (uint32_t)x87_fistp64_h2(fild_u32_h2(n) * *(const float *)(self + 0x2c));
    for (;;) {
        __asm__ volatile("" ::: "memory");
        const uint16_t d = list[i];
        if (d > key) {
            if (!i || list[i - 1] <= key) return i;
            hi = i;
        } else if (d == key)
            return i + 1;
        else
            lo = i;
        i = (lo + hi) >> 1;
    }
}
static const uint8_t sorted_shorts_find_original[] = {0x83, 0xec, 0x08, 0x8b, 0x44, 0x24, 0x10, 0x53, 0x56, 0x57};
REWRITE("SortedShorts::Find", 0xbb6970, sorted_shorts_find_original, &sorted_shorts_find, &g_short_finds);

// ---- Sound::MuteKind, Sound::UnmuteKind(kind in ecx, flag in dl) (0x47C3F0, 0x47C470) -----------------------------------
// Mute counts one more mute on the kind (0x12F0168), 10000 with flag; Unmute one less, 0 with flag or below 0. Then each
// sound on the kind's list (0x10EDD60) gets its volume again as Effects::Set47C4E0 gives it.
static volatile long g_sound_mutes;
static void sound_volumes_h2(uintptr_t k) {
    const volatile float *const level = (const volatile float *)0x10edd40;
    const uint32_t *p = *(const uint32_t *const volatile *)(0x10edd60 + k * 24);
    if (!p[0] || !p) return;
    for (;;) {
        uint8_t *const e = (uint8_t *)(uintptr_t)p[2];
        const int32_t kind = *(const int32_t *)(e + 0x18);
        fpu_copy_h2(e + 0x14, e + 0x14);
        const long double r = ((const volatile uint32_t *)0x12f0168)[kind] ? (long double)*(const float *)0xf264e0 : (long double)*(const float *)(e + 0x14) * level[kind];
        *(volatile float *)(*(uint8_t *const *)(e + 0xc) + 0x58) = (float)r;
        p = (const uint32_t *)(uintptr_t)p[0];
        if (!p[0] || !p) return;
    }
}
static void __fastcall sound_mute_kind(uintptr_t k, uint32_t flag) {
    g_sound_mutes++;
    volatile int32_t *const muted = (volatile int32_t *)0x12f0168;
    muted[k] = muted[k] + 1;
    if ((uint8_t)flag) muted[k] = 10000;
    sound_volumes_h2(k);
}
static void __fastcall sound_unmute_kind(uintptr_t k, uint32_t flag) {
    g_sound_mutes++;
    volatile int32_t *const muted = (volatile int32_t *)0x12f0168;
    const int32_t m = muted[k] - 1;
    muted[k] = m;
    if (m < 0 || (uint8_t)flag) muted[k] = 0;
    sound_volumes_h2(k);
}
static const uint8_t sound_mute_kind_original[] = {0x8b, 0x04, 0x8d, 0x68, 0x01, 0x2f, 0x01, 0x40, 0x84, 0xd2};
REWRITE("Sound::MuteKind", 0x47c3f0, sound_mute_kind_original, &sound_mute_kind, &g_sound_mutes);
static const uint8_t sound_unmute_kind_original[] = {0xff, 0x0c, 0x8d, 0x68, 0x01, 0x2f, 0x01, 0x78, 0x04};
REWRITE("Sound::UnmuteKind", 0x47c470, sound_unmute_kind_original, &sound_unmute_kind, &g_sound_mutes);

// ---- Materials::CompareOpacity(i, j) (0xCEB590): 1 (only al) when part i's material is more opaque than part j's ---------
// Parts i, j index entries (12 bytes at +0x7C) whose first dword picks a 0x70-byte material at +0; a material without
// +0x58 gives 0 for i, 1 for j; opacity is +0x54 when byte +0x50 is set, else 1.
static volatile long g_opacity_compares;
static uint8_t __thiscall materials_compare_opacity(const uint8_t *self, int32_t i, int32_t j) {
    g_opacity_compares++;
    const uint32_t *const parts = *(const uint32_t *const *)(self + 0x7c);
    const uint8_t *const base = *(const uint8_t *const *)self;
    const uint8_t *const a = base + parts[i * 3] * 0x70, *const b = base + parts[j * 3] * 0x70;
    if (!a[0x58]) return 0;
    if (!b[0x58]) return 1;
    const float one = *(const float *)0xf264dc;
    const float oa = a[0x50] ? *(const float *)(a + 0x54) : one, ob = b[0x50] ? *(const float *)(b + 0x54) : one;
    return oa > ob ? 1 : 0;
}
static const uint8_t materials_compare_opacity_original[] = {0x8b, 0x51, 0x7c, 0x8b, 0x44, 0x24, 0x04, 0x56, 0x8b, 0x31};
REWRITE("Materials::CompareOpacity", 0xceb590, materials_compare_opacity_original, &materials_compare_opacity, &g_opacity_compares);

// ---- Tile::QuarterCorners(out in ecx, bits in dl) (0x727CC0): the four corners of a tile half -------------------------
// With the game's tile size S (0x1306390) and base B (0x13063A4): x = (bit 1 ? 2 S : 0) + B, y = (bit 2 ? 0 : 2 S) + B
// (unrounded); corner i is {x (+ 2 S for i bit 1), y (+ 2 S for i bit 2)} as floats.
static volatile long g_quarter_corners;
static void __fastcall tile_quarter_corners(float *out, uint32_t bits) {
    g_quarter_corners++;
    const volatile float *const g = (const volatile float *)0x1306390;
    const float b = *(const volatile float *)0x13063a4, zero = *(const float *)0xf264e0;
    auto two_s = [&] {
        const long double s = g[0];
        return s + s;
    };
    const long double x = (bits & 1 ? two_s() : (long double)zero) + b;
    const long double y = (bits & 2 ? (long double)zero : two_s()) + b;
    volatile float *const o = out;
    for (int i = 0; i < 4; i++) {
        o[2 * i] = (float)(i & 1 ? two_s() + x : x);
        o[2 * i + 1] = (float)(i & 2 ? two_s() + y : y);
    }
}
static const uint8_t tile_quarter_corners_original[] = {0xf6, 0xc2, 0x01, 0x74, 0x0a, 0xd9, 0x05, 0x90, 0x63, 0x30, 0x01};
REWRITE("Tile::QuarterCorners", 0x727cc0, tile_quarter_corners_original, &tile_quarter_corners, &g_quarter_corners);

// ---- Shape::SameAs(other) (0xAE0980): 1 (only al) when the point counts (+0x50), the values +0x54 and every point (+0x40,
// x and y floats) are equal (a NaN never is) ----------------------------------------------------------------------------
static volatile long g_shape_sames;
static uint8_t __thiscall shape_same_as(const uint8_t *self, const uint8_t *other) {
    g_shape_sames++;
    const int32_t n = *(const int32_t *)(self + 0x50);
    if (*(const int32_t *)(other + 0x50) != n || !(*(const float *)(self + 0x54) == *(const float *)(other + 0x54))) return 0;
    const float *const a = *(const float *const *)(self + 0x40), *const b = *(const float *const *)(other + 0x40);
    for (int32_t i = 0; i < n; i++)
        if (!(a[2 * i] == b[2 * i]) || !(a[2 * i + 1] == b[2 * i + 1])) return 0;
    return 1;
}
static const uint8_t shape_same_as_original[] = {0x8b, 0x54, 0x24, 0x04, 0x8b, 0x42, 0x50, 0x57, 0x8b, 0x79, 0x50};
REWRITE("Shape::SameAs", 0xae0980, shape_same_as_original, &shape_same_as, &g_shape_sames);

// ---- List::MoveAllTo(list) (0xA1CE90): every item of the list has its ring node (+8 in the item) taken out of its ring
// (as List::LoopA1CE30) and put at the back of the ring at +0x88 ----------------------------------------------------------
static volatile long g_ring_moves;
static void __thiscall list_move_all_to(const uint8_t *self, uint32_t *const *list) {
    g_ring_moves++;
    const uint32_t *p = *list;
    if (!p[0] || !p) return;
    for (;;) {
        uint32_t *const n = (uint32_t *)(uintptr_t)(p[2] + 8);
        if (n) {
            uint32_t *const back = (uint32_t *)(uintptr_t)n[1], *const next = (uint32_t *)(uintptr_t)n[0];
            if (back && next && next[1] == (uint32_t)(uintptr_t)n && back[0] == (uint32_t)(uintptr_t)n) {
                next[1] = (uint32_t)(uintptr_t)back;
                ((uint32_t *)(uintptr_t)n[1])[0] = n[0];
                n[0] = 0, n[1] = 0;
            }
        }
        uint32_t *const head = *(uint32_t *const *)(self + 0x88);
        n[1] = head[1];
        ((uint32_t *)(uintptr_t)head[1])[0] = (uint32_t)(uintptr_t)n;
        n[0] = (uint32_t)(uintptr_t)head;
        head[1] = (uint32_t)(uintptr_t)n;
        p = (const uint32_t *)(uintptr_t)p[0];
        if (!p[0] || !p) return;
    }
}
static const uint8_t list_move_all_to_original[] = {0x8b, 0x44, 0x24, 0x04, 0x56, 0x8b, 0x30, 0x83, 0x3e, 0x00};
REWRITE("List::MoveAllTo", 0xa1ce90, list_move_all_to_original, &list_move_all_to, &g_ring_moves);

// ---- Pools::ResetCounts (0xBFEF90) -------------------------------------------------------------------------------------
// Each pool on the list at +0x24 whose use (+0x1C) over its size (+8) is above 3/4 (0xFA5C88) gets +0x1C and +0x20 back to
// its size; every one gets +0x24 = 0. Each pool on the list at +0x84 gets +0x20 = 0. Then +0x14..+0x20 are 0.
static volatile long g_pool_resets;
static void __thiscall pools_reset_counts(uint8_t *self) {
    g_pool_resets++;
    const uint32_t *p = *(const uint32_t *const *)(self + 0x24);
    if (p[0] && p)
        for (;;) {
            int32_t *const pool = (int32_t *)(uintptr_t)p[2];
            if ((long double)pool[7] / (long double)pool[2] > *(const float *)0xfa5c88) pool[7] = pool[2], pool[8] = pool[2];
            pool[9] = 0;
            p = (const uint32_t *)(uintptr_t)p[0];
            if (!p[0] || !p) break;
        }
    p = *(const uint32_t *const *)(self + 0x84);
    if (p[0] && p)
        for (;;) {
            ((int32_t *)(uintptr_t)p[2])[8] = 0;
            p = (const uint32_t *)(uintptr_t)p[0];
            if (!p[0] || !p) break;
        }
    uint32_t *const u = (uint32_t *)self;
    u[5] = 0, u[7] = 0, u[8] = 0, u[6] = 0;
}
static const uint8_t pools_reset_counts_original[] = {0x51, 0x53, 0x56, 0x8b, 0x71, 0x24, 0x8b, 0x06, 0x33, 0xdb};
REWRITE("Pools::ResetCounts", 0xbfef90, pools_reset_counts_original, &pools_reset_counts, &g_pool_resets);

// ---- Scenery::Count8606E0 (0x8606E0): 1 (only al) when any entry (the +0xC part's list +0x18, count +0x14) of kind 0
// (its part +8) or 3 (its part +0x10) has a piece (the part's list +0x28, count +0x24) with flag 2 or 8 (+0x10); else 0 --
static volatile long g_scenery_flagged;
static uint8_t __thiscall scenery_count_8606e0(const uint8_t *self) {
    g_scenery_flagged++;
    const uint8_t *const part = *(const uint8_t *const *)(self + 0xc);
    const int32_t n = *(const int32_t *)(part + 0x14);
    for (int32_t i = 0; i < n; i++) {
        const uint32_t *const e = (*(const uint32_t *const *const *)(part + 0x18))[i];
        const uint8_t *sub;
        if (e[0] == 0) sub = (const uint8_t *)(uintptr_t)e[2];
        else if (e[0] == 3) sub = (const uint8_t *)(uintptr_t)e[4];
        else continue;
        const int32_t m = *(const int32_t *)(sub + 0x24);
        for (int32_t j = 0; j < m; j++)
            if ((*(const uint8_t *const *const *)(sub + 0x28))[j][0x10] & 0xa) return 1;
    }
    return 0;
}
static const uint8_t scenery_count_8606e0_original[] = {0x8b, 0x41, 0x0c, 0x53, 0x8b, 0x58, 0x14, 0x55, 0x56, 0x57};
REWRITE("Scenery::Count8606E0", 0x8606e0, scenery_count_8606e0_original, &scenery_count_8606e0, &g_scenery_flagged);

// ---- NamedList::Find(name) (0xC32C60): the first item of the list at +4 whose name (+4) is name ("" for none), or 0 -----
static volatile long g_named_finds2;
static uint32_t __thiscall named_list_find(const uint8_t *self, const char *name) {
    g_named_finds2++;
    if (!name) name = (const char *)0xf264ad;
    const uint32_t *p = *(const uint32_t *const *)(self + 4);
    if (!p[0] || !p) return 0;
    for (;;) {
        const uint32_t *const item = (const uint32_t *)(uintptr_t)p[2];
        const char *a = name, *b = (const char *)(uintptr_t)item[1];
        while (*a == *b && *a) a++, b++;
        if (*a == *b) return p[2];
        p = (const uint32_t *)(uintptr_t)p[0];
        if (!p[0] || !p) return 0;
    }
}
static const uint8_t named_list_find_original[] = {0x53, 0x55, 0x8b, 0x6c, 0x24, 0x0c, 0x85, 0xed, 0x56, 0x57};
REWRITE("NamedList::Find", 0xc32c60, named_list_find_original, &named_list_find, &g_named_finds2);

// ---- Window::SumA51870 (0xA51870): a weighted count of the entries (8 bytes at +0xC, count +0x10) of kinds 3, 4 and 5:
// (n3 + 3 n5 + 2 n4) / (count 3) 100 (whole numbers loaded as such) ---------------------------------------------------
static volatile long g_window_sums;
static long double __thiscall window_sum_a51870(const uint8_t *self) {
    g_window_sums++;
    const int32_t n = *(const int32_t *)(self + 0x10);
    const int32_t *const e = *(const int32_t *const *)(self + 0xc);
    int32_t c3 = 0, c4 = 0, c5 = 0;
    for (int32_t i = 0; i < n; i++) c3 += e[i * 2] == 3;
    for (int32_t i = 0; i < n; i++) c4 += e[i * 2] == 4;
    for (int32_t i = 0; i < n; i++) c5 += e[i * 2] == 5;
    const int32_t v = c5 + c3 + c5 * 2 + c4 * 2;
    return (long double)v / ((long double)n * *(const float *)0xf2784c) * *(const float *)0xf27604;
}
static const uint8_t window_sum_a51870_original[] = {0x83, 0xec, 0x08, 0x8b, 0x41, 0x10, 0x53, 0x56, 0x33, 0xdb};
REWRITE("Window::SumA51870", 0xa51870, window_sum_a51870_original, &window_sum_a51870, &g_window_sums);

// ---- SceneryRenderer::SiftDown(i) (0x42FE40): moves item i down the max-heap (+0x64, count +0x6C read again) by +0x24 ---
// The larger child (the right one only when the left is below it) moves up while the item is below it.
static volatile long g_heap_sifts;
static void __thiscall scenery_renderer_sift_down(const uint8_t *self, int32_t i) {
    g_heap_sifts++;
    uint32_t *const h = *(uint32_t *const *)(self + 0x64);
    const uint32_t v = h[i];
    auto key = [](uint32_t item) { return *(const float *)(uintptr_t)(item + 0x24); };
    while (i < *(const volatile int32_t *)(self + 0x6c) / 2) {
        const int32_t l = 2 * i + 1;
        int32_t c = l + 1;
        if (c >= *(const volatile int32_t *)(self + 0x6c) || !(key(h[l]) < key(h[c]))) c = l;
        if (!(key(v) < key(h[c]))) break;
        h[i] = h[c];
        i = c;
    }
    h[i] = v;
}
static const uint8_t scenery_renderer_sift_down_original[] = {0x8b, 0x41, 0x64, 0x53, 0x8b, 0x5c, 0x24, 0x08, 0x55};
REWRITE("SceneryRenderer::SiftDown", 0x42fe40, scenery_renderer_sift_down_original, &scenery_renderer_sift_down, &g_heap_sifts);

// ---- Key::CompareMasked(mask a in ecx, mask b in edx, a0, a1, a2, b0, b1, b2) (0xCC4AA0): -1, 0 or 1 over the parts both
// masks have (bits 1, 2, 4 for parts 0, 1, 2), the first that differs deciding (signed) ----------------------------------
static volatile long g_key_compares;
static int32_t __fastcall key_compare_masked(uint32_t ma, uint32_t mb, int32_t a0, int32_t a1, int32_t a2, int32_t b0, int32_t b1, int32_t b2) {
    g_key_compares++;
    const uint32_t m = ma & mb;
    const int32_t a[3] = {a0, a1, a2}, b[3] = {b0, b1, b2};
    for (int k = 0; k < 3; k++)
        if (m & (1u << k)) {
            if (a[k] < b[k]) return -1;
            if (a[k] > b[k]) return 1;
        }
    return 0;
}
static const uint8_t key_compare_masked_original[] = {0x55, 0x8b, 0xec, 0x83, 0xec, 0x08, 0xc7, 0x45, 0xf8, 0xcc, 0xcc, 0xcc, 0xcc};
REWRITE("Key::CompareMasked", 0xcc4aa0, key_compare_masked_original, &key_compare_masked, &g_key_compares);

// ---- Value::Band(v) (0x4FF3A0): which of seven bands v is in: the first of the six limits (0xF36898, 0xF36894, 0xF36890,
// 0xF3688C, 0xF36888, 0xF28218) it is below, else 6 (a NaN is below none) ------------------------------------------------
static volatile long g_value_bands;
static uint32_t __stdcall value_band(float v) {
    g_value_bands++;
    static const uintptr_t limits[6] = {0xf36898, 0xf36894, 0xf36890, 0xf3688c, 0xf36888, 0xf28218};
    for (uint32_t k = 0; k < 6; k++)
        if (v < *(const float *)limits[k]) return k;
    return 6;
}
static const uint8_t value_band_original[] = {0xd9, 0x44, 0x24, 0x04, 0xd8, 0x1d, 0x98, 0x68, 0xf3, 0x00};
REWRITE("Value::Band", 0x4ff3a0, value_band_original, &value_band, &g_value_bands);

// ---- Bits::MergeLowest(cost in edi, new cost in si, fields in ecx, new fields) (0x8E30C0) ------------------------------
// Below the best cost (a signed word) 1. Above it: the cost and the fields byte are replaced, 0. Equal: each two-bit field
// takes the new one unless the old is lower; 1 when any old one was lower (the last one ends it at once), else 0. Only al.
static volatile long g_cost_merges;
extern "C" uint8_t __stdcall forever_merge_lowest(int16_t *cost, uint32_t new_in, uint8_t *fields, uint32_t new_fields) {
    g_cost_merges++;
    const int16_t c = (int16_t)new_in;
    const uint8_t nf = (uint8_t)new_fields;
    if (*cost < c) return 1;
    if (*cost > c) {
        *cost = c;
        *fields = nf;
        return 0;
    }
    uint8_t kept = 0;
    for (int k = 0; k < 3; k++) {
        const uint8_t m = (uint8_t)(3 << (2 * k));
        if ((*fields & m) < (nf & m)) kept = 1;
        else *fields = (uint8_t)((*fields & ~m) | (nf & m));
    }
    if ((*fields & 0xc0) < (nf & 0xc0)) return 1;
    *fields = (uint8_t)((*fields & 0x3f) | (nf & 0xc0));
    return kept;
}
extern "C" __attribute__((naked)) void forever_merge_lowest_regs() {   // cost edi, new cost si, fields ecx; new fields on the stack
    __asm__("pushl 4(%esp)\n\tpushl %ecx\n\tpushl %esi\n\tpushl %edi\n\tcalll _forever_merge_lowest@16\n\tret $4");
}
static const uint8_t bits_merge_lowest_original[] = {0x51, 0x66, 0x8b, 0x17, 0x66, 0x3b, 0xd6, 0x8a, 0x44, 0x24, 0x08};
REWRITE("Bits::MergeLowest", 0x8e30c0, bits_merge_lowest_original, (void *)&forever_merge_lowest_regs, &g_cost_merges);

// ---- TileQuarter::Corners(out in ecx, packed in dx, q) (0x727D30): the four corners of quarter q of a tile --------------
// With the game's tile size S (0x1306390) and base B (0x13063A4) and the four bits n of quarter q (packed >> 4 q):
// x = ((n bit 1 ? 2 S : 0) + (q bit 1 ? S : 0)) + B, y = ((n bit 2 ? 0 : 2 S) + (q bit 2 ? S : 0)) + B (unrounded); corner i
// is {x (+ S for i bit 1), y (+ S for i bit 2)} as floats.
static volatile long g_quarter_corners2;
static void __fastcall tile_quarter_corners_727d30(float *out, uint32_t packed, uint32_t q_in) {
    g_quarter_corners2++;
    const uint8_t q = (uint8_t)q_in;
    const uint32_t n = ((packed & 0xffff) >> ((q << 2) & 31)) & 0xf;
    const volatile float *const s = (const volatile float *)0x1306390;
    const float b = *(const volatile float *)0x13063a4, zero = *(const float *)0xf264e0;
    auto two_s = [&] {
        const long double v = s[0];
        return v + v;
    };
    const long double x = ((n & 1 ? two_s() : (long double)zero) + (q & 1 ? (long double)s[0] : (long double)zero)) + b;
    const long double y = ((n & 2 ? (long double)zero : two_s()) + (q & 2 ? (long double)s[0] : (long double)zero)) + b;
    volatile float *const o = out;
    for (int i = 0; i < 4; i++) {
        o[2 * i] = (float)(i & 1 ? (long double)s[0] + x : x);
        o[2 * i + 1] = (float)(i & 2 ? (long double)s[0] + y : y);
    }
}
static const uint8_t tile_quarter_corners_727d30_original[] = {0x8a, 0x44, 0x24, 0x04, 0x56, 0x8b, 0xf1, 0x8a, 0xc8};
REWRITE("TileQuarter::Corners", 0x727d30, tile_quarter_corners_727d30_original, &tile_quarter_corners_727d30, &g_quarter_corners2);

// ---- HeightPatch::Flatten(r) (0x8DB7A0): one height at all four corners of each cell of r's rectangle --------------------
// r: {.., x0 +4, y0 +5, x1 +6, y1 +7 (bytes), cells +8 (16 bytes each, row by row), source +0x10 (its height +4)}. With a
// step count (+8, above 0) the height is rounded to steps: k = steps K (K 0xF29428), floor(h / k + 0.5) k (floats).
static volatile long g_patch_flattens;
static void __thiscall height_patch_flatten(const uint8_t *self, const uint8_t *r) {
    g_patch_flattens++;
    uint32_t h = *(const uint32_t *)(*(const uint8_t *const *)(r + 0x10) + 4);
    const int32_t steps = *(const int32_t *)(self + 8);
    if (steps > 0) {
        volatile float k = (float)((long double)steps * *(const float *)0xf29428);
        volatile float q = (float)((long double)*(const float *)&h / k + *(const float *)0xf2651c);
        volatile float fl = (float)floor_h2(q);
        volatile float v = (float)((long double)fl * k);
        h = *(volatile uint32_t *)&v;
    }
    uint32_t *out = *(uint32_t *const *)(r + 8);
    for (uint8_t y = r[5]; y <= r[7]; y++)
        if (r[4] <= r[6])
            for (uint8_t x = r[4]; x <= r[6]; x++, out += 4) out[0] = out[1] = out[2] = out[3] = h;
}
static const uint8_t height_patch_flatten_original[] = {0x83, 0xec, 0x08, 0x8b, 0x54, 0x24, 0x0c, 0x8b, 0x42, 0x10};
REWRITE("HeightPatch::Flatten", 0x8db7a0, height_patch_flatten_original, &height_patch_flatten, &g_patch_flattens);

// ---- HeightPatch::Average(r) (0x8DC0C0): each cell of r's rectangle gets the average of the terrain's four heights there -
// The terrain (r +0: width byte +4, heights +0x24, six per cell); each height times 1/4 (0xF270B8) summed from 0 in order.
static volatile long g_patch_averages;
static void __thiscall height_patch_average(const void *, const uint8_t *r) {
    g_patch_averages++;
    const uint8_t *const terrain = *(const uint8_t *const *)r;
    uint32_t *out = *(uint32_t *const *)(r + 8);
    for (uint8_t y = r[5]; y <= r[7]; y++)
        if (r[4] <= r[6])
            for (uint8_t x = r[4]; x <= r[6]; x++, out += 4) {
                const float *const hs = *(const float *const *)(terrain + 0x24) + ((uint32_t)terrain[4] * y + x) * 6;
                long double sum = *(const float *)0xf264e0;
                for (int j = 0; j < 4; j++) sum = sum + (long double)hs[j] * *(const float *)0xf270b8;
                volatile float a = (float)sum;
                out[0] = out[1] = out[2] = out[3] = *(volatile uint32_t *)&a;
            }
}
static const uint8_t height_patch_average_original[] = {0x83, 0xec, 0x0c, 0x57, 0x8b, 0x7c, 0x24, 0x14, 0x8b, 0x47, 0x08};
REWRITE("HeightPatch::Average", 0x8dc0c0, height_patch_average_original, &height_patch_average, &g_patch_averages);

// ---- Timers::Advance(dt) (0x90C710) --------------------------------------------------------------------------------
// Each timer (32 bytes at +0x40, count +0x48 read again each time) gets +0x18 += dt (a float); one not below its end
// (+0x10) takes the last one's +0..+8 and +0x10..+0x18 and the count goes down (the moved one is done again). Then each
// entry of the second list (+0x4C, count +0x54) gets +0x18 += dt 30 (unrounded).
static volatile long g_timer_advances;
static void __thiscall timers_advance(uint8_t *self, float dt) {
    g_timer_advances++;
    for (int32_t i = 0; i < *(const volatile int32_t *)(self + 0x48); i++) {
        float *const e = (float *)(*(uint8_t *const *)(self + 0x40) + i * 32);
        *(volatile float *)(e + 6) = (float)((long double)dt + e[6]);
        if (e[6] < e[4] || e[6] != e[6] || e[4] != e[4]) continue;
        const uint32_t *const last = (const uint32_t *)(*(uint8_t *const *)(self + 0x40) + *(const int32_t *)(self + 0x48) * 32 - 32);
        uint32_t *const u = (uint32_t *)e;
        u[0] = last[0], u[1] = last[1], u[2] = last[2], u[4] = last[4], u[5] = last[5], u[6] = last[6];
        *(int32_t *)(self + 0x48) -= 1;
        i--;
    }
    const int32_t n = *(const int32_t *)(self + 0x54);
    if (n <= 0) return;
    const long double d = (long double)dt * *(const float *)0xf28218;
    for (int32_t j = 0; j < *(const volatile int32_t *)(self + 0x54); j++) {
        volatile float *const t = (volatile float *)(*(uint8_t *const *)(self + 0x4c) + j * 32 + 0x18);
        *t = (float)(d + *t);
    }
}
static const uint8_t timers_advance_original[] = {0x8b, 0x41, 0x48, 0x56, 0x57, 0x33, 0xff, 0x85, 0xc0};
REWRITE("Timers::Advance", 0x90c710, timers_advance_original, &timers_advance, &g_timer_advances);

// ---- Keys::FindFrom, FindFrom2(t, guess) (0xE02530: keys +0x10, count +0xC; 0xE02470: keys +8, count +4) -----------------
// Keys of 16 bytes, time first. From the guess: past the last key the last; the guess when t is below the next key's time,
// the one after when below the one after that (or past the end); the last when t is not below the last key's time; else a
// binary search between them for the key with the last time below t.
static volatile long g_key_finds;
template <uint32_t Keys, uint32_t Count> static int32_t __thiscall keys_find_from(const uint8_t *self, float t, int32_t g) {
    g_key_finds++;
    const int32_t n = *(const int32_t *)(self + Count), last = n - 1;
    if (g >= last) return last;
    const uint8_t *const k = *(const uint8_t *const *)(self + Keys);
    auto time = [&](int32_t i) { return *(const float *)(k + i * 16); };
    if (t < time(g + 1)) return g;
    int32_t lo = g + 2;
    if (lo >= n || t < time(lo)) return g + 1;
    int32_t hi = *(const int32_t *)(self + Count) - 1;
    if (!(t < time(hi) || t != t)) return last;
    while (lo < hi - 1) {
        const int32_t mid = (hi - lo) / 2 + lo;
        if (t > time(mid)) lo = mid;
        else hi = mid;
    }
    return lo;
}
static const uint8_t keys_find_from_original[] = {0x8b, 0x54, 0x24, 0x08, 0x55, 0x57, 0x8b, 0x79, 0x0c, 0x8d, 0x6f, 0xff};
REWRITE("Keys::FindFrom", 0xe02530, keys_find_from_original, (&keys_find_from<0x10, 0xc>), &g_key_finds);
static const uint8_t keys_find_from2_original[] = {0x8b, 0x54, 0x24, 0x08, 0x55, 0x57, 0x8b, 0x79, 0x04, 0x8d, 0x6f, 0xff};
REWRITE("Keys::FindFrom2", 0xe02470, keys_find_from2_original, (&keys_find_from<8, 4>), &g_key_finds);

// ---- Plane::BoxCornerDistance(a, b, d, out) (0xC1E7E0) ---------------------------------------------------------------
// The plane: normal n (+0), distance +0x10. With s = d . n (z, y, x order): 0 when |s| is below E (0xF3312C). Else the
// corner c takes each part from a where n's is above 0, else from b; out = ((c.z n.z + c.x n.x) + c.y n.y - dist)
// (-1 / s), and 1 when s is above 0, else -1.
static volatile long g_box_corners;
static int32_t __thiscall plane_box_corner_distance(const float *self, const float *a, const float *b, const float *d, float *out) {
    g_box_corners++;
    const long double s = ((long double)d[2] * self[2] + (long double)d[1] * self[1]) + (long double)d[0] * self[0];
    if (__builtin_fabsl(s) < *(const float *)0xf3312c) return 0;
    const float zero = *(const float *)0xf264e0;
    const float cx = self[0] > zero ? a[0] : b[0], cy = self[1] > zero ? a[1] : b[1], cz = self[2] > zero ? a[2] : b[2];
    const long double dist = ((long double)cz * self[2] + (long double)cx * self[0]) + (long double)cy * self[1] - self[4];
    *(volatile float *)out = (float)(dist * ((long double)*(const float *)0xf268f0 / s));
    return s > zero ? 1 : -1;
}
static const uint8_t plane_box_corner_distance_original[] = {0x8b, 0x44, 0x24, 0x0c, 0xd9, 0x40, 0x08, 0xd8, 0x49, 0x08};
REWRITE("Plane::BoxCornerDistance", 0xc1e7e0, plane_box_corner_distance_original, &plane_box_corner_distance, &g_box_corners);

// ---- Groups::Totals (0xBD2CA0): for each group on the list at +0x1C, totals over its members (list +0x14) ---------------
// The group's +0x38, +0x30, +0x34 and byte +0x40 start at 0; each member adds its +0x40 to +0x38 and +0x3C, ors its byte
// +0x3C into +0x40, and +0x30, +0x34 keep the larger of theirs and the member's +0x44, +0x48 (the FPU's way).
static volatile long g_group_totals;
static void __thiscall groups_totals(const uint8_t *self) {
    g_group_totals++;
    const uint32_t *p = *(const uint32_t *const *)(self + 0x1c);
    if (!p[0] || !p) return;
    for (;;) {
        uint8_t *const g = (uint8_t *)(uintptr_t)p[2];
        *(uint32_t *)(g + 0x38) = 0, *(uint32_t *)(g + 0x30) = 0, *(uint32_t *)(g + 0x34) = 0;
        g[0x40] = 0;
        const uint32_t *q = *(const uint32_t *const *)(g + 0x14);
        if (q[0] && q)
            for (;;) {
                const uint8_t *const m = (const uint8_t *)(uintptr_t)q[2];
                *(int32_t *)(g + 0x38) += *(const int32_t *)(m + 0x40);
                *(int32_t *)(g + 0x3c) += *(const int32_t *)(m + 0x40);
                g[0x40] |= m[0x3c];
                for (int k = 0; k < 2; k++) {
                    const float cur = *(const float *)(g + 0x30 + k * 4), v = *(const float *)(m + 0x44 + k * 4);
                    volatile float r = v < cur || v != v || cur != cur ? cur : v;
                    *(volatile float *)(g + 0x30 + k * 4) = r;
                }
                q = (const uint32_t *)(uintptr_t)q[0];
                if (!q[0] || !q) break;
            }
        p = (const uint32_t *)(uintptr_t)p[0];
        if (!p[0] || !p) return;
    }
}
static const uint8_t groups_totals_original[] = {0x83, 0xec, 0x18, 0x57, 0x8b, 0x79, 0x1c, 0x8b, 0x07, 0x33, 0xd2};
REWRITE("Groups::Totals", 0xbd2ca0, groups_totals_original, &groups_totals, &g_group_totals);

// ---- Water::Entry970F20(i, n) (0x970F20): sifts entry i down the min-heap of n {id, key float} entries at +0x90 --------
// The smaller child (the right one unless the left is below it) moves up while the entry's key is above it.
static volatile long g_water_heap_sifts;
static void __thiscall water_entry_970f20(const uint8_t *self, int32_t i, int32_t n) {
    g_water_heap_sifts++;
    uint32_t *const h = *(uint32_t *const *)(self + 0x90);
    const uint32_t id = h[i * 2];
    const float k = *(const float *)&h[i * 2 + 1];
    auto key = [&](int32_t j) { return *(const float *)&h[j * 2 + 1]; };
    int32_t l = 2 * i + 1;
    for (; l + 1 < n; l = 2 * i + 1) {
        const int32_t c = key(l) < key(l + 1) ? l : l + 1;
        if (!(k > key(c))) goto done;
        h[i * 2] = h[c * 2], h[i * 2 + 1] = h[c * 2 + 1];
        i = c;
    }
    if (l < n && k > key(l)) {
        h[i * 2] = h[l * 2], h[i * 2 + 1] = h[l * 2 + 1];
        i = l;
    }
done:
    h[i * 2] = id;
    fpu_copy_h2(&h[i * 2 + 1], &k);
}
static const uint8_t water_entry_970f20_original[] = {0x8b, 0x81, 0x90, 0x00, 0x00, 0x00, 0x53, 0x8b, 0x5c, 0x24, 0x0c};
REWRITE("Water::Entry970F20", 0x970f20, water_entry_970f20_original, &water_entry_970f20, &g_water_heap_sifts);

// ---- Water::Set93CC50(terrain) (0x93CC50): which of a cell's three heights the water's level is above ----------------
// With no water (+4) +0x14 = 0. Else the level (water +0x30) against the cell (x byte +0, y byte +1) of the terrain (width
// byte +4, cells of 24 bytes at +0x24): bits 1, 2, 4 for its +0, +4, +8 (or +0xC, +8, +4 when byte +2 is set).
static volatile long g_water_levels;
static void __thiscall water_set_93cc50(uint8_t *self, const uint8_t *terrain) {
    g_water_levels++;
    const uint8_t *const w = *(const uint8_t *const *)(self + 4);
    if (!w) {
        *(uint32_t *)(self + 0x14) = 0;
        return;
    }
    const float h = *(const float *)(w + 0x30);
    const float *const c = (const float *)(*(const uint8_t *const *)(terrain + 0x24) + ((uint32_t)terrain[4] * self[1] + self[0]) * 24);
    const bool turned = self[2] != 0;
    const float h1 = turned ? c[3] : c[0], h2 = turned ? c[2] : c[1], h4 = turned ? c[1] : c[2];
    *(uint32_t *)(self + 0x14) = (h > h4 ? 4u : 0u) | (h > h2 ? 2u : 0u) | (h > h1 ? 1u : 0u);
}
static const uint8_t water_set_93cc50_original[] = {0x8b, 0x41, 0x04, 0x85, 0xc0, 0x0f, 0x84, 0xba, 0x00, 0x00, 0x00};
REWRITE("Water::Set93CC50", 0x93cc50, water_set_93cc50_original, &water_set_93cc50, &g_water_levels);

// ---- Water::Value9060F0(x, y, n) (0x9060F0): a sum along a row of the water grids ------------------------------------
// From 0: the n samples of +0x44 from row y (width +0) column x, then for the n - 1 after column x of row y (width +4)
// each one of +0x4C followed by the same one of +0x50.
static volatile long g_water_row_sums;
static long double __thiscall water_value_9060f0(const uint8_t *self, int32_t x, int32_t y, int32_t n) {
    g_water_row_sums++;
    long double sum = *(const float *)0xf264e0;
    const float *const a = *(const float *const *)(self + 0x44) + (*(const int32_t *)self * y + x);
    for (int32_t i = 0; i < n; i++) sum = sum + a[i];
    if (n > 1) {
        const int32_t at = *(const int32_t *)(self + 4) * y + x + 1;
        const float *const b = *(const float *const *)(self + 0x4c) + at, *const c = *(const float *const *)(self + 0x50) + at;
        for (int32_t i = 0; i < n - 1; i++) {
            sum = sum + b[i];
            sum = sum + c[i];
        }
    }
    return sum;
}
static const uint8_t water_value_9060f0_original[] = {0x8b, 0x01, 0xd9, 0x05, 0xe0, 0x64, 0xf2, 0x00, 0x8b, 0x51, 0x44};
REWRITE("Water::Value9060F0", 0x9060f0, water_value_9060f0_original, &water_value_9060f0, &g_water_row_sums);

// ---- Fireworks::Find989300(o) (0x989300): of o's items (+0xCC, count +0x58; from the last back), the part (+0xA0) with
// the largest value (+0x4C) above 0, the first found keeping ties; 0 when none is above 0 --------------------------------
static volatile long g_firework_bests;
static uint32_t __stdcall fireworks_find_989300(const uint8_t *o) {
    g_firework_bests++;
    float best = *(const float *)0xf264e0;
    uint32_t found = 0;
    const uint32_t *const items = *(const uint32_t *const *)(o + 0xcc);
    for (int32_t i = *(const int32_t *)(o + 0x58) - 1; i >= 0; i--) {
        const uint32_t part = *(const uint32_t *)(uintptr_t)(items[i] + 0xa0);
        const float v = *(const float *)(uintptr_t)(part + 0x4c);
        if (best < v) best = v, found = part;
    }
    return found;
}
static const uint8_t fireworks_find_989300_original[] = {0xd9, 0x05, 0xe0, 0x64, 0xf2, 0x00, 0x55, 0x8b, 0x6c, 0x24, 0x08};
REWRITE("Fireworks::Find989300", 0x989300, fireworks_find_989300_original, &fireworks_find_989300, &g_firework_bests);

// ---- Scenery::Slot8847A0(v) (0x8847A0): the slot of a value in the sorted stops at +0xFC (count +0xF4) -----------------
// With W = (+0x84 - +0x7C) - 8 and s = +0xDC: t = (v - 4) / W s - s / W 3 (a float); the first stop above t, or the count.
static volatile long g_scenery_slots;
static int32_t __thiscall scenery_slot_8847a0(const uint8_t *self, float v) {
    g_scenery_slots++;
    auto at = [&](uint32_t o) { return *(const float *)(self + o); };
    const float k1 = *(const float *)0xf793c8, k2 = *(const float *)0xf270e8, k3 = *(const float *)0xf793c4;
    const long double a = ((long double)v - k1) / (((long double)at(0x84) - at(0x7c)) - k2) * at(0xdc);
    const long double b = (long double)at(0xdc) / (((long double)at(0x84) - at(0x7c)) - k2) * k3;
    volatile float t = (float)(a - b);
    const int32_t n = *(const int32_t *)(self + 0xf4);
    const float *const stops = *(const float *const *)(self + 0xfc);
    int32_t i = 0;
    for (; i < n; i++)
        if (stops[i] > t) return i;
    return i;
}
static const uint8_t scenery_slot_8847a0_original[] = {0xd9, 0x44, 0x24, 0x04, 0x56, 0xd8, 0x25, 0xc8, 0x93, 0xf7, 0x00};
REWRITE("Scenery::Slot8847A0", 0x8847a0, scenery_slot_8847a0_original, &scenery_slot_8847a0, &g_scenery_slots);

// ---- Track::LengthBC64E0(out) (0xBC64E0): the direction from the first point of one end list to the first of the other --
// The lists (+0x110, +0x10C; points at their +4) are there with flag 0x400000 (+0x18) and at least two ends (+0xE4); the
// game reads through 0 otherwise. D = q - p (x and y floats, z unrounded) made of length 1 ((z^2 + y^2) + x^2); a length of
// 0 gives {0, 0, 1}. Gives out.
static volatile long g_track_directions;
static float *__thiscall track_length_bc64e0(const uint8_t *self, float *out) {
    g_track_directions++;
    const bool there = (*(const uint32_t *)(self + 0x18) & 0x400000) && *(const int32_t *)(self + 0xe4) >= 2;
    const uint32_t *const volatile la = there ? *(const uint32_t *const *)(self + 0x110) : nullptr;
    const float *const p = (const float *)(uintptr_t)la[1];
    const uint32_t *const volatile lb = there ? *(const uint32_t *const *)(self + 0x10c) : nullptr;
    const float *const q = (const float *)(uintptr_t)lb[1];
    volatile float dx = (float)((long double)q[0] - p[0]);
    volatile float dy = (float)((long double)q[1] - p[1]);
    const long double dz = (long double)q[2] - p[2];
    const long double len = x87_sqrt_h2((dz * dz + (long double)dy * dy) + (long double)dx * dx);
    volatile float *const o = out;
    if (len == *(const float *)0xf264e0) {
        ((volatile uint32_t *)o)[0] = 0, ((volatile uint32_t *)o)[1] = 0, ((volatile uint32_t *)o)[2] = 0x3f800000;
        return out;
    }
    const long double inv = (long double)*(const float *)0xf264dc / len;
    o[0] = (float)((long double)dx * inv);
    o[1] = (float)((long double)dy * inv);
    o[2] = (float)(dz * inv);
    return out;
}
static const uint8_t track_length_bc64e0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x2c, 0x8b, 0x51, 0x18};
REWRITE("Track::LengthBC64E0", 0xbc64e0, track_length_bc64e0_original, &track_length_bc64e0, &g_track_directions);

// ---- Scenery::Cell88F3F0(level, x, y, lo, hi) (0x88F3F0): grows the bounds of a cell on every level from level up -------
// Grids (+0x390, count +0x394: {width, .., cells of 64 bytes +8}); x and y halve (signed) from level to level. A cell in
// use (count +0x30 above 0) counts one more and takes the smaller lo (+0x10..) and larger hi (+0x20..) parts; else it
// starts with count 1 and lo, hi.
static volatile long g_scenery_cells;
static void __thiscall scenery_cell_88f3f0(const uint8_t *self, int32_t level, int32_t x, int32_t y, const float *lo, const float *hi) {
    g_scenery_cells++;
    for (;;) {
        const uint32_t *const grid = (*(const uint32_t *const *const *)(self + 0x390))[level];
        uint8_t *const cell = (uint8_t *)(uintptr_t)grid[2] + ((int32_t)grid[0] * y + x) * 64;
        float *const c = (float *)cell;
        int32_t *const count = (int32_t *)(cell + 0x30);
        if (*count > 0) {
            *count += 1;
            for (int k = 0; k < 3; k++)
                if (lo[k] < c[4 + k]) *(uint32_t *)&c[4 + k] = *(const uint32_t *)&lo[k];
            for (int k = 0; k < 3; k++)
                if (hi[k] > c[8 + k]) *(uint32_t *)&c[8 + k] = *(const uint32_t *)&hi[k];
        } else {
            *count = 1;
            for (int k = 0; k < 3; k++) *(uint32_t *)&c[4 + k] = *(const uint32_t *)&lo[k];
            for (int k = 0; k < 3; k++) *(uint32_t *)&c[8 + k] = *(const uint32_t *)&hi[k];
        }
        if (level + 1 >= *(const int32_t *)(self + 0x394)) return;
        y >>= 1, x >>= 1;
        level++;
    }
}
static const uint8_t scenery_cell_88f3f0_original[] = {0x53, 0x8b, 0x5c, 0x24, 0x08, 0x55, 0x8b, 0x6c, 0x24, 0x14, 0x56};
REWRITE("Scenery::Cell88F3F0", 0x88f3f0, scenery_cell_88f3f0_original, &scenery_cell_88f3f0, &g_scenery_cells);

// ---- Particles::Pass9E16E0(dt) (0x9E16E0): moves the particles (0x30 bytes at +0x14, count +0x1C) when on (+0x10) -------
// Each: age (+0x20) += dt; velocity y (+0x14) += G dt (G the game's 0x10F0E8C, unrounded); position (+0) += velocity
// (+0x10) dt (x, y unrounded, z a float). One past the game's lifetime (0x10F0E90) takes the last one's values and the
// count goes down (the moved one is done again). With none left the particles are off.
static volatile long g_particle_steps;
static void __thiscall particles_pass_9e16e0(uint8_t *self, float dt) {
    g_particle_steps++;
    if (!self[0x10]) return;
    int32_t *const count = (int32_t *)(self + 0x1c);
    if (*count > 0) {
        const long double g = (long double)*(const volatile float *)0x10f0e8c * dt;
        for (int32_t i = 0; i < *count; i++) {
            volatile float *const e = (volatile float *)(*(uint8_t *const *)(self + 0x14) + i * 0x30);
            e[8] = (float)((long double)dt + e[8]);
            e[5] = (float)(g + e[5]);
            const long double a = (long double)dt * e[4], b = (long double)dt * e[5];
            volatile float c = (float)((long double)dt * e[6]);
            e[0] = (float)(a + e[0]);
            e[1] = (float)(b + e[1]);
            e[2] = (float)((long double)c + e[2]);
            if (!(e[8] > *(const volatile float *)0x10f0e90)) continue;
            *count -= 1;
            uint32_t *const u = (uint32_t *)e;
            const uint32_t *const last = (const uint32_t *)(*(uint8_t *const *)(self + 0x14) + *count * 0x30);
            u[0] = last[0], u[1] = last[1], u[2] = last[2], u[4] = last[4], u[5] = last[5], u[6] = last[6], u[8] = last[8];
            i--;
        }
    }
    if (!*count) self[0x10] = 0;
}
static const uint8_t particles_pass_9e16e0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x14, 0x8a, 0x41, 0x10};
REWRITE("Particles::Pass9E16E0", 0x9e16e0, particles_pass_9e16e0_original, &particles_pass_9e16e0, &g_particle_steps);

// ---- Toolbar::Find79C0A0(point in ecx, counts in edx, r) (0x79C0A0): how many of the game's toolbar items lie within r --
// counts (when given) is cleared (6 dwords) and gets one more for each such item's kind (+0xA4). The items as
// Toolbar::Find79C050 walks them (none when 0x1306BCC equals 0x1306BD8); an item's place is its +0xAC's +0x30's +0x90;
// d = (dz^2 + dy^2) + dx^2 (dx a float) against r^2 (a float).
static volatile long g_toolbar_nears;
static uint32_t __fastcall toolbar_find_79c0a0(const float *point, uint32_t *counts, float r) {
    g_toolbar_nears++;
    volatile float r2 = (float)((long double)r * r);
    uint32_t n = 0;
    if (counts)
        for (int k = 0; k < 6; k++) counts[k] = 0;
    const volatile uint32_t *const g = (const volatile uint32_t *)0x1306bc8;
    if (g[1] == g[4]) return 0;
    for (const uint32_t *p = (const uint32_t *)(uintptr_t)(g[0] + g[6] * 4); p < (const uint32_t *)(uintptr_t)(g[0] + g[7] * 4); p++) {
        if (!*p) continue;
        const uint8_t *const item = (const uint8_t *)(uintptr_t)*p;
        const float *const pos = (const float *)(*(const uint8_t *const *)(*(const uint8_t *const *)(item + 0xac) + 0x30) + 0x90);
        volatile float dx = (float)((long double)pos[0] - point[0]);
        const long double dy = (long double)pos[1] - point[1], dz = (long double)pos[2] - point[2];
        if (!((dz * dz + dy * dy) + (long double)dx * dx <= r2)) continue;
        n++;
        if (counts) counts[*(const int32_t *)(item + 0xa4)]++;
    }
    return n;
}
static const uint8_t toolbar_find_79c0a0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x34, 0xd9, 0x45, 0x08};
REWRITE("Toolbar::Find79C0A0", 0x79c0a0, toolbar_find_79c0a0_original, &toolbar_find_79c0a0, &g_toolbar_nears);

// ---- QuadTree::FindNode(start in ecx, limit in edx, corner, size, box corner, box size) (0xCDB600) ----------------------
// From node start (whose corner and size are given), while the box is no bigger than a quarter of the node in x and
// then y, steps to child 4 i + 1 (+1 for the upper x half, +2 for the upper y half, by where the box's middle (a float)
// lies against the child's corner plus its half size) and moves the corner by 3/4 or 1/4 of the halved size; stops when
// the child number reaches the limit. Gives the node.
static volatile long g_quad_finds;
static int32_t __fastcall quad_tree_find_node(int32_t node, int32_t limit, const float *corner, const float *size, const float *box, const float *box_size) {
    g_quad_finds++;
    const float half = *(const float *)0xf2651c, quarter = *(const float *)0xf270b8, three = *(const float *)0xf27c74;
    long double a0 = corner[0], a1 = corner[1], b0 = size[0], b1 = size[1];
    const float s0 = box_size[0];
    volatile float m0 = (float)((long double)box_size[0] * half + box[0]);
    volatile float m1 = (float)((long double)box_size[1] * half + box[1]);
    if (!(b0 * quarter >= s0)) return node;
    for (;;) {
        if (!((long double)quarter * b1 >= box_size[1])) return node;
        const int32_t c = node * 4 + 1;
        if (c >= limit) return node;
        node = c;
        b0 = b0 * half;
        b1 = b1 * half;
        if (a0 + b0 < m0) {
            a0 = a0 + b0 * three;
            node = c + 1;
        } else
            a0 = a0 + b0 * quarter;
        if (a1 + b1 < m1) {
            a1 = a1 + (long double)three * b1;
            node += 2;
        } else
            a1 = a1 + (long double)quarter * b1;
        if (!(b0 * quarter >= s0)) return node;
    }
}
static const uint8_t quad_tree_find_node_original[] = {0x83, 0xec, 0x08, 0x8b, 0x44, 0x24, 0x0c, 0xd9, 0x00, 0x57};
REWRITE("QuadTree::FindNode", 0xcdb600, quad_tree_find_node_original, &quad_tree_find_node, &g_quad_finds);

// ---- Water::Time8E6880(out) (0x8E6880): a colour for the time of day --------------------------------------------------
// f = (now - start) / (end - start) of the game's day (0x1103B6C: +0x110, +0x20, +0x200), kept within 0..1; k = f 128
// rounded by the FPU, at most 128 (signed). The colour starts 0x7F7F7F7F with 255 - k in byte 2, 1 or 0 by mode +0x180
// (0, 1, 2; other modes none), then k + 127 in byte 2, 1 or 0 by mode +0x184. Gives out.
static volatile long g_water_day_colours;
static uint32_t *__thiscall water_time_8e6880(const uint8_t *self, uint32_t *out) {
    g_water_day_colours++;
    const float *const day = *(const float *const volatile *)0x1103b6c;
    long double f = ((long double)day[0x110 / 4] - day[0x20 / 4]) / ((long double)day[0x200 / 4] - day[0x20 / 4]);
    if (f > *(const float *)0xf264dc) f = *(const float *)0xf264dc;
    else if (f < *(const float *)0xf264e0) f = *(const float *)0xf264e0;
    int32_t k = (int32_t)x87_fistp64_h2((long double)*(const float *)0xf61410 * f);
    if (k > 0x80) k = 0x80;
    const uint32_t a = (uint32_t)(0x80 - k) + 0x7f, b = (uint32_t)k + 0x7f;
    uint32_t c = 0x7f7f7f7f;
    switch (*(const int32_t *)(self + 0x180)) {
    case 0: c = a << 16 | 0x7f007f7f; break;
    case 1: c = a << 8 | 0x7f7f007f; break;
    case 2: c = a | 0x7f7f7f00; break;
    }
    switch (*(const int32_t *)(self + 0x184)) {
    case 0: c = (c & 0xff00ffff) | b << 16; break;
    case 1: c = (c & 0xffff00ff) | b << 8; break;
    case 2: c = (c & 0xffffff00) | b; break;
    }
    *out = c;
    return out;
}
static const uint8_t water_time_8e6880_original[] = {0xa1, 0x6c, 0x3b, 0x10, 0x01, 0xd9, 0x80, 0x10, 0x01, 0x00, 0x00};
REWRITE("Water::Time8E6880", 0x8e6880, water_time_8e6880_original, &water_time_8e6880, &g_water_day_colours);

// ---- Track::LastBC65D0(out) (0xBC65D0): the direction from the last point of one end list to the last of the other ----
// As Track::LengthBC64E0 with each list's last point (36 bytes each) and from +0x10C's to +0x110's. Gives out.
static volatile long g_track_end_directions;
static float *__thiscall track_last_bc65d0(const uint8_t *self, float *out) {
    g_track_end_directions++;
    const bool there = (*(const uint32_t *)(self + 0x18) & 0x400000) && *(const int32_t *)(self + 0xe4) >= 2;
    const uint32_t *const volatile la = there ? *(const uint32_t *const *)(self + 0x10c) : nullptr;
    const float *const p = (const float *)((const uint8_t *)(uintptr_t)la[1] + la[0] * 36 - 36);
    const uint32_t *const volatile lb = there ? *(const uint32_t *const *)(self + 0x110) : nullptr;
    const float *const q = (const float *)((const uint8_t *)(uintptr_t)lb[1] + lb[0] * 36 - 36);
    volatile float dx = (float)((long double)q[0] - p[0]);
    volatile float dy = (float)((long double)q[1] - p[1]);
    const long double dz = (long double)q[2] - p[2];
    const long double len = x87_sqrt_h2((dz * dz + (long double)dy * dy) + (long double)dx * dx);
    volatile float *const o = out;
    if (len == *(const float *)0xf264e0) {
        ((volatile uint32_t *)o)[0] = 0, ((volatile uint32_t *)o)[1] = 0, ((volatile uint32_t *)o)[2] = 0x3f800000;
        return out;
    }
    const long double inv = (long double)*(const float *)0xf264dc / len;
    o[0] = (float)((long double)dx * inv);
    o[1] = (float)((long double)dy * inv);
    o[2] = (float)(dz * inv);
    return out;
}
static const uint8_t track_last_bc65d0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x2c, 0x56, 0x8b, 0x71, 0x18};
REWRITE("Track::LastBC65D0", 0xbc65d0, track_last_bc65d0_original, &track_last_bc65d0, &g_track_end_directions);

// ---- Track::LengthBC62A0(out) (0xBC62A0): the reversed sum of the two end lists' first directions, made of length 1 ------
// The first points (+0x110's, +0x10C's; lists as Track::LengthBC64E0 finds them) and their vectors at +0x18: s = -q - p
// (x and y floats, z unrounded), made of length 1 ((z^2 + y^2) + x^2); a length of 0 gives {0, 0, 1}. Gives out.
static volatile long g_track_turns;
static float *__thiscall track_length_bc62a0(const uint8_t *self, float *out) {
    g_track_turns++;
    const bool there = (*(const uint32_t *)(self + 0x18) & 0x400000) && *(const int32_t *)(self + 0xe4) >= 2;
    const uint32_t *const volatile la = there ? *(const uint32_t *const *)(self + 0x110) : nullptr;
    const float *const p = (const float *)((const uint8_t *)(uintptr_t)la[1] + 0x18);
    volatile float px = (float)-(long double)p[0], py = (float)-(long double)p[1];
    const long double pz = -(long double)p[2];
    const uint32_t *const volatile lb = there ? *(const uint32_t *const *)(self + 0x10c) : nullptr;
    const float *const q = (const float *)((const uint8_t *)(uintptr_t)lb[1] + 0x18);
    volatile float qx = (float)-(long double)q[0];
    const long double qy = -(long double)q[1], qz = -(long double)q[2];
    volatile float sx = (float)((long double)qx + px);
    volatile float sy = (float)(qy + py);
    const long double sz = qz + pz;
    const long double len = x87_sqrt_h2((sz * sz + (long double)sy * sy) + (long double)sx * sx);
    volatile float *const o = out;
    if (len == *(const float *)0xf264e0) {
        ((volatile uint32_t *)o)[0] = 0, ((volatile uint32_t *)o)[1] = 0, ((volatile uint32_t *)o)[2] = 0x3f800000;
        return out;
    }
    const long double inv = (long double)*(const float *)0xf264dc / len;
    o[0] = (float)((long double)sx * inv);
    o[1] = (float)((long double)sy * inv);
    o[2] = (float)(sz * inv);
    return out;
}
static const uint8_t track_length_bc62a0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x30, 0x8b, 0x51, 0x18};
REWRITE("Track::LengthBC62A0", 0xbc62a0, track_length_bc62a0_original, &track_length_bc62a0, &g_track_turns);

// ---- Track::PassBB7470(out) (0xBB7470): one byte per piece, picked by the piece's key -----------------------------------
// The table (+0x10): count +0x20, flags +0x30, runs +0x40 ({count, offset} words per piece), keys +0x50. For piece i
// (written at out[the word (+0x3C's +8)[i]]): with runs, the place of the key (+0x38) among its sorted keys as
// SortedShorts::Find finds it (the guess count times +0x2C), and the flag at offset + place; without, the flag at
// +0x3A count + i. Each byte written is 1 when the flag is set, else 0.
static volatile long g_track_piece_flags;
static void __thiscall track_pass_bb7470(const uint8_t *self, uint8_t *out) {
    g_track_piece_flags++;
    const uint8_t *const t = *(const uint8_t *const *)(self + 0x10);
    const uint32_t n = *(const uint32_t *)(t + 0x20);
    for (uint32_t i = 0; i < n; i++) {
        const uint16_t slot = (*(const uint16_t *const *)(*(const uint8_t *const *)(self + 0x3c) + 8))[i];
        const uint8_t *const tt = *(const uint8_t *const *)(self + 0x10);
        const uint16_t *const runs = *(const uint16_t *const *)(tt + 0x40);
        uint8_t flag;
        if (runs) {
            const uint32_t cnt = runs[i * 2], off = runs[i * 2 + 1];
            const uint16_t *const list = *(const uint16_t *const *)(tt + 0x50) + off;
            const uint16_t key = *(const uint16_t *)(self + 0x38);
            uint32_t lo = 0, hi = cnt;
            uint32_t j = (uint32_t)x87_fistp64_h2((long double)(int32_t)cnt * *(const float *)(self + 0x2c));
            for (;;) {
                __asm__ volatile("" ::: "memory");
                const uint16_t d = list[j];
                if (d > key) {
                    if (!j || list[j - 1] <= key) break;
                    hi = j;
                } else if (d == key) {
                    j++;
                    break;
                } else
                    lo = j;
                j = (lo + hi) >> 1;
            }
            flag = (*(const uint8_t *const *)(tt + 0x30) + off)[j];
        } else
            flag = (*(const uint8_t *const *)(tt + 0x30) + *(const uint16_t *)(self + 0x3a) * *(const uint32_t *)(tt + 0x20))[i];
        out[slot] = flag != 0;
    }
}
static const uint8_t track_pass_bb7470_original[] = {0x83, 0xec, 0x18, 0x8b, 0x41, 0x10, 0x57, 0x8b, 0x78, 0x20};
REWRITE("Track::PassBB7470", 0xbb7470, track_pass_bb7470_original, &track_pass_bb7470, &g_track_piece_flags);

// ---- Water::Cell93DA00(x, y, low, high) (0x93DA00): a cell's two water levels, the smaller first ------------------------
// Inside the grid (width byte +0x58, height byte +0x59; cells of 0x30 bytes at +0x5C, by the low bytes of x and y): its
// two parts (+4, +0x1C) and their levels (+0x30). With both: low = the first's when below the second's, else the
// second's, and high the other; with one: both its level; with none 0 (only al), else 1.
static volatile long g_water_cell_levels;
static uint8_t __thiscall water_cell_93da00(const uint8_t *self, int32_t x, int32_t y, uint32_t *low, uint32_t *high) {
    g_water_cell_levels++;
    if (y < 0 || y >= self[0x59] || x < 0 || x >= self[0x58]) return 0;
    const uint8_t *const c = *(const uint8_t *const *)(self + 0x5c) + ((uint32_t)(uint8_t)y * self[0x58] + (uint8_t)x) * 0x30;
    const uint8_t *const a = *(const uint8_t *const *)(c + 4), *const b = *(const uint8_t *const *)(c + 0x1c);
    auto level = [](const uint8_t *p) { return *(const uint32_t *)(p + 0x30); };
    if (a && b) {
        if (*(const float *)(a + 0x30) < *(const float *)(b + 0x30)) *low = level(a), *high = level(b);
        else *low = level(b), *high = level(a);
        return 1;
    }
    if (a) return *low = level(a), *high = level(a), 1;
    if (b) return *low = level(b), *high = level(b), 1;
    return 0;
}
static const uint8_t water_cell_93da00_original[] = {0x53, 0x57, 0x8b, 0xf9, 0x8b, 0x4c, 0x24, 0x10, 0x85, 0xc9};
REWRITE("Water::Cell93DA00", 0x93da00, water_cell_93da00_original, &water_cell_93da00, &g_water_cell_levels);

// ---- Grid::Round435EA0(x, y) (0x435EA0): the height at a point from its cell's four corners ---------------------------
// u = (x - +0xC) +0x1C, v = (y - +0x10) +0x20 (floats), the cell their floors (i, j; -32768 outside 0..+4 byte, 0..+5
// byte). With du, dv the parts past the floors and s = du + dv: above 1 the far triangle (1 - du) h2 + (1 - dv) h1 + (s -
// 1) h3, else the near one (1 - s) h0 + du h1 + dv h2 (cells of 24 bytes at +0x24, row by row).
static volatile long g_grid_heights;
static long double __thiscall grid_round_435ea0(const uint8_t *self, float x, float y) {
    g_grid_heights++;
    auto at = [&](uint32_t o) { return *(const float *)(self + o); };
    volatile float u = (float)(((long double)x - at(0xc)) * at(0x1c));
    volatile float v = (float)(((long double)y - at(0x10)) * at(0x20));
    volatile float fu = (float)floor_h2(u), fv = (float)floor_h2(v);
    const int32_t i = (int32_t)x87_fistp64_h2(fu), j = (int32_t)x87_fistp64_h2(fv);
    if (i < 0 || j < 0 || i >= self[4] || j >= self[5]) return *(const float *)0xf28530;
    const float *const h = (const float *)(*(const uint8_t *const *)(self + 0x24) + ((uint32_t)self[4] * j + i) * 24);
    const long double dv = (long double)v - fv, du = (long double)u - fu, s = du + dv;
    const long double one = *(const float *)0xf264dc;
    if (s > one) return ((one - du) * h[2] + (one - dv) * h[1]) + (s - one) * h[3];
    return ((one - s) * h[0] + du * h[1]) + dv * h[2];
}
static const uint8_t grid_round_435ea0_original[] = {0x83, 0xec, 0x10, 0xd9, 0x44, 0x24, 0x14, 0x56, 0xd8, 0x61, 0x0c};
REWRITE("Grid::Round435EA0", 0x435ea0, grid_round_435ea0_original, &grid_round_435ea0, &g_grid_heights);

// ---- Track::LengthBC63B0(out) (0xBC63B0): the reversed sum of the two end lists' last directions, made of length 1 ------
// As Track::LengthBC62A0 with each list's last point (36 bytes each) and its vector at +0xC.
static volatile long g_track_end_turns;
static float *__thiscall track_length_bc63b0(const uint8_t *self, float *out) {
    g_track_end_turns++;
    const bool there = (*(const uint32_t *)(self + 0x18) & 0x400000) && *(const int32_t *)(self + 0xe4) >= 2;
    const uint32_t *const volatile la = there ? *(const uint32_t *const *)(self + 0x110) : nullptr;
    const float *const p = (const float *)((const uint8_t *)(uintptr_t)la[1] + la[0] * 36 - 0x18);
    volatile float px = (float)-(long double)p[0], py = (float)-(long double)p[1];
    const long double pz = -(long double)p[2];
    const uint32_t *const volatile lb = there ? *(const uint32_t *const *)(self + 0x10c) : nullptr;
    const float *const q = (const float *)((const uint8_t *)(uintptr_t)lb[1] + lb[0] * 36 - 0x18);
    volatile float qx = (float)-(long double)q[0];
    const long double qy = -(long double)q[1], qz = -(long double)q[2];
    volatile float sx = (float)((long double)qx + px);
    volatile float sy = (float)(qy + py);
    const long double sz = qz + pz;
    const long double len = x87_sqrt_h2((sz * sz + (long double)sy * sy) + (long double)sx * sx);
    volatile float *const o = out;
    if (len == *(const float *)0xf264e0) {
        ((volatile uint32_t *)o)[0] = 0, ((volatile uint32_t *)o)[1] = 0, ((volatile uint32_t *)o)[2] = 0x3f800000;
        return out;
    }
    const long double inv = (long double)*(const float *)0xf264dc / len;
    o[0] = (float)((long double)sx * inv);
    o[1] = (float)((long double)sy * inv);
    o[2] = (float)(sz * inv);
    return out;
}
static const uint8_t track_length_bc63b0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x3c, 0x56, 0x8b, 0x71, 0x18};
REWRITE("Track::LengthBC63B0", 0xbc63b0, track_length_bc63b0_original, &track_length_bc63b0, &g_track_end_turns);

// ---- Track::RoundBB69E0(keys, n, place, part) (0xBB69E0): where the key (+0x38) falls among n sorted keys, and how far --
// The place j as SortedShorts::Find finds it (the guess n (unsigned) times +0x2C); 0 gives 0 and 0. Else with m = the
// table's (+0x10) +4 count - 1 and a, b the keys j - 1, j over m: t = (+0x2C - a) / (b - a) (a float). Above 0.95 gives
// j and 0; else j - 1 with t, or 0 when t is below 0.05.
static volatile long g_track_key_places;
static void __thiscall track_round_bb69e0(const uint8_t *self, const uint16_t *list, uint32_t n, int32_t *place, float *part) {
    g_track_key_places++;
    const uint16_t key = *(const uint16_t *)(self + 0x38);
    uint32_t lo = 0, hi = n;
    uint32_t j = (uint32_t)x87_fistp64_h2(fild_u32_h2(n) * *(const float *)(self + 0x2c));
    for (;;) {
        __asm__ volatile("" ::: "memory");
        const uint16_t d = list[j];
        if (d > key) {
            if (!j) break;
            if (list[j - 1] <= key) break;
            hi = j;
        } else if (d == key) {
            j++;
            break;
        } else
            lo = j;
        j = (lo + hi) >> 1;
    }
    if (!j) {
        *place = 0, *(uint32_t *)part = 0;
        return;
    }
    const long double inv = (long double)*(const float *)0xf264dc / (long double)(*(const int32_t *)(*(const uint8_t *const *)(self + 0x10) + 4) - 1);
    const long double a = (long double)list[j - 1] * inv;
    volatile float t = (float)(((long double)*(const float *)(self + 0x2c) - a) / ((long double)list[j] * inv - a));
    if (t > *(const float *)0xf29660) {
        *place = (int32_t)j, *(uint32_t *)part = 0;
        return;
    }
    *place = (int32_t)j - 1;
    if (t < *(const float *)0xf270b4) *(uint32_t *)part = 0;
    else *(volatile float *)part = t;
}
static const uint8_t track_round_bb69e0_original[] = {0x83, 0xec, 0x08, 0x8b, 0x44, 0x24, 0x10, 0x53, 0x55, 0x56};
REWRITE("Track::RoundBB69E0", 0xbb69e0, track_round_bb69e0_original, &track_round_bb69e0, &g_track_key_places);

// ---- Camera::TimeCD0510(track, t, wrap, part, from, to) (0xCD0510): which two keys of a track t lies between -----------
// Track k (64 bytes at +0x24: key count +0x24, key times +0x28). t below 0 or a single key gives 0, 0, 0. t at or past
// the last time: without wrap gives the last key twice and 0; with wrap t - floor(t / last + 0.00001 (a float)) last.
// Then from = the first key whose time is above t (a binary search), to = the next (0 after the last), and part = (t -
// the time before from (0 for the first)) / (from's time - that).
static volatile long g_camera_key_finds;
static void __thiscall camera_time_cd0510(const uint8_t *self, int32_t k, float t, uint8_t wrap, float *part, int32_t *from, int32_t *to) {
    g_camera_key_finds++;
    const uint8_t *const e = *(const uint8_t *const *)(self + 0x24) + k * 64;
    const int32_t count = *(const int32_t *)(e + 0x24);
    const float *const times = *(const float *const *)(e + 0x28);
    const float last = times[count - 1];
    if (t < *(const float *)0xf264e0 || count == 1) {
        *from = 0, *to = 0, *(uint32_t *)part = 0;
        return;
    }
    long double v = t;
    if (!(t < last || __builtin_isunordered(t, last))) {
        if (!wrap) {
            *from = count - 1, *to = count - 1, *(uint32_t *)part = 0;
            return;
        }
        volatile float x = (float)((long double)t / last + *(const float *)0xf2e9f0);
        volatile float f = (float)floor_h2(x);
        v = (long double)t - (long double)f * last;
    }
    int32_t lo = 0, hi = count - 1;
    while (lo != hi) {
        const int32_t mid = (lo + hi) >> 1;
        if (v < times[mid]) hi = mid;
        else lo = mid + 1;
    }
    *from = lo;
    *to = lo + 1 >= count ? 0 : lo + 1;
    const long double prev = lo ? (long double)times[lo - 1] : (long double)*(const float *)0xf264e0;
    *(volatile float *)part = (float)((v - prev) / ((long double)times[lo] - prev));
}
static const uint8_t camera_time_cd0510_original[] = {0x51, 0x8b, 0x54, 0x24, 0x08, 0x8b, 0x41, 0x24, 0xc1, 0xe2, 0x06};
REWRITE("Camera::TimeCD0510", 0xcd0510, camera_time_cd0510_original, &camera_time_cd0510, &g_camera_key_finds);

// ---- PoolComplex::LoopAAA110(i, n, nodes) (0xAAA110): sifts heap place i down among n (smallest key on top) ------------
// The heap (*self) holds node numbers; a node's key is its +0x2C + +0x28 (nodes of 0x38 bytes at *nodes). The smaller
// child (the second on ties) moves up while its key is below the moved node's.
static volatile long g_pool_heap_sifts;
static void __thiscall pool_complex_loop_aaa110(const uint32_t *const *self, int32_t i, int32_t n, const uint8_t *const *nodes) {
    g_pool_heap_sifts++;
    auto key = [&](uint32_t node) {
        const float *const p = (const float *)(*nodes + node * 0x38);
        return (long double)p[0x2c / 4] + p[0x28 / 4];
    };
    uint32_t *const heap = (uint32_t *)*self;
    const uint32_t v = heap[i];
    int32_t hole = i, c = 2 * i + 1;
    for (; c + 1 < n; c = 2 * hole + 1) {
        const int32_t pick = key(heap[c + 1]) > key(heap[c]) ? c : c + 1;
        if (!(key(v) > key(heap[pick]))) goto done;
        heap[hole] = heap[pick];
        hole = pick;
    }
    if (c < n && key(v) > key(heap[c])) {
        heap[hole] = heap[c];
        hole = c;
    }
done:
    heap[hole] = v;
}
static const uint8_t pool_complex_loop_aaa110_original[] = {0x83, 0xec, 0x08, 0x8b, 0x44, 0x24, 0x0c, 0x53, 0x55, 0x56};
REWRITE("PoolComplex::LoopAAA110", 0xaaa110, pool_complex_loop_aaa110_original, &pool_complex_loop_aaa110, &g_pool_heap_sifts);

// ---- PoolComplex::PointsAAA300(nodes) (0xAAA300): takes the top off the pool's heap ------------------------------------
// The count (+8) goes down; with any left, the last takes the top place and sifts down as PoolComplex::LoopAAA110 does.
static volatile long g_pool_heap_pops;
static void __thiscall pool_complex_points_aaa300(uint32_t *self, const uint8_t *const *nodes) {
    g_pool_heap_pops++;
    const int32_t n = (int32_t)--self[2];
    if (!n) return;
    uint32_t *const heap = (uint32_t *)(uintptr_t)self[0];
    heap[0] = heap[n];
    auto key = [&](uint32_t node) {
        const float *const p = (const float *)(*nodes + node * 0x38);
        return (long double)p[0x2c / 4] + p[0x28 / 4];
    };
    const uint32_t v = heap[0];
    int32_t hole = 0, c = 1;
    for (; c + 1 < n; c = 2 * hole + 1) {
        const int32_t pick = key(heap[c + 1]) > key(heap[c]) ? c : c + 1;
        if (!(key(v) > key(heap[pick]))) goto done;
        heap[hole] = heap[pick];
        hole = pick;
    }
    if (c < n && key(v) > key(heap[c])) {
        heap[hole] = heap[c];
        hole = c;
    }
done:
    heap[hole] = v;
}
static const uint8_t pool_complex_points_aaa300_original[] = {0x83, 0xec, 0x10, 0x56, 0x8b, 0xf1, 0x8b, 0x46, 0x08, 0x48};
REWRITE("PoolComplex::PointsAAA300", 0xaaa300, pool_complex_points_aaa300_original, &pool_complex_points_aaa300, &g_pool_heap_pops);

// ---- Person::Loop7E6750(i, n, items) (0x7E6750): sifts heap place i down among n (smallest key on top) ----------------
// The heap (*self) holds item numbers; an item's key is the float at +8 (items of 20 bytes at *items). The smaller child
// (the second on ties) moves up while its key is below the moved item's.
static volatile long g_person_heap_sifts;
static void __thiscall person_loop_7e6750(const uint32_t *const *self, int32_t i, int32_t n, const uint8_t *const *items) {
    g_person_heap_sifts++;
    auto key = [&](uint32_t item) { return *(const float *)(*items + item * 20 + 8); };
    uint32_t *const heap = (uint32_t *)*self;
    const uint32_t v = heap[i];
    int32_t hole = i, c = 2 * i + 1;
    for (; c + 1 < n; c = 2 * hole + 1) {
        const int32_t pick = key(heap[c]) < key(heap[c + 1]) ? c : c + 1;
        if (!(key(heap[pick]) < key(v))) goto done;
        heap[hole] = heap[pick];
        hole = pick;
    }
    if (c < n && key(heap[c]) < key(v)) {
        heap[hole] = heap[c];
        hole = c;
    }
done:
    heap[hole] = v;
}
static const uint8_t person_loop_7e6750_original[] = {0x83, 0xec, 0x10, 0x8b, 0x44, 0x24, 0x14, 0x53, 0x55, 0x8b, 0xd1};
REWRITE("Person::Loop7E6750", 0x7e6750, person_loop_7e6750_original, &person_loop_7e6750, &g_person_heap_sifts);

// ---- Camera::RayCD7DC0(out, point, size) (0xCD7DC0): where a point lands on a screen of size, and whether it is on it ----
// d = point - place (+0x160); c = d against the camera's right (+0x120), up (+0x130), forward (+0x140) ((z + y) + x; the
// first two floats). w = (flat (+0x1A6): +0x198, else c.z) tan(field of view (+0x190) / 2). out = ((cx / w + 1) size.x,
// (cy / (w / +0x194) + 1) size.y) / 2 (floats). 1 when c.z is past the near distance (+0x188) and out lies within
// 0..size, else 0.
static volatile long g_camera_projections;
static uint32_t __thiscall camera_ray_cd7dc0(const uint8_t *self, float *out, const float *point, const float *size) {
    g_camera_projections++;
    auto m = [&](uint32_t at) { return *(const float *)(self + at); };
    const long double dx = (long double)point[0] - m(0x160), dy = (long double)point[1] - m(0x164), dz = (long double)point[2] - m(0x168);
    volatile float cx = (float)((dz * m(0x128) + dy * m(0x124)) + dx * m(0x120));
    volatile float cy = (float)((dz * m(0x138) + dy * m(0x134)) + dx * m(0x130));
    const long double cz = (dz * m(0x148) + dy * m(0x144)) + dx * m(0x140);
    const long double h = self[0x1a6] ? (long double)m(0x198) : cz;
    const long double half = *(const float *)0xf2651c, one = *(const float *)0xf264dc;
    const long double w = h * x87_tan_h2((long double)m(0x190) * half);
    const long double wa = w / m(0x194);
    volatile float *const o = out;
    o[0] = (float)(((long double)cx / w + one) * size[0] * half);
    volatile float y = (float)(((long double)cy / wa + one) * size[1] * half);
    o[1] = y;
    if (!(cz > m(0x188))) return 0;
    if (!(o[0] >= *(const float *)0xf264e0) || !(o[0] <= size[0])) return 0;
    if (!(y >= *(const float *)0xf264e0) || !(y <= size[1])) return 0;
    return 1;
}
static const uint8_t camera_ray_cd7dc0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x0c, 0x8b, 0x45, 0x0c};
REWRITE("Camera::RayCD7DC0", 0xcd7dc0, camera_ray_cd7dc0_original, &camera_ray_cd7dc0, &g_camera_projections);

// ---- Vector::CompareCDF200(a in ecx, b in edx, points, dir) (0xCDF200): which of two triangles reaches less far along dir --
// Each triangle's least (y dy + z dz) + x dx over its three point numbers (points of 16 bytes; the second and third
// as floats, the first unrounded, ties keeping the earlier). 1 when a's is above b's, -1 when below, else 0.
static volatile long g_triangle_compares;
static long double triangle_least_h2(const uint32_t *t, const float *points, const float *d) {
    auto dot = [&](uint32_t i) {
        const float *const v = points + i * 4;
        return ((long double)v[1] * d[1] + (long double)v[2] * d[2]) + (long double)v[0] * d[0];
    };
    volatile float f1 = (float)dot(t[1]), f2 = (float)dot(t[2]);
    long double m = dot(t[0]);
    if (f1 < m) m = f1;
    if (f2 < m) m = f2;
    return m;
}
static int32_t __fastcall vector_compare_cdf200(const uint32_t *a, const uint32_t *b, const float *points, const float *d) {
    g_triangle_compares++;
    const long double ma = triangle_least_h2(a, points, d), mb = triangle_least_h2(b, points, d);
    if (ma > mb) return 1;
    if (ma < mb) return -1;
    return 0;
}
static const uint8_t vector_compare_cdf200_original[] = {0x8b, 0x01, 0x53, 0x8b, 0x5c, 0x24, 0x08, 0x56, 0x8b, 0x74, 0x24, 0x10};
REWRITE("Vector::CompareCDF200", 0xcdf200, vector_compare_cdf200_original, &vector_compare_cdf200, &g_triangle_compares);

// ---- Person::Points7E6C20(items) (0x7E6C20): takes the top off the heap ------------------------------------------------
// The count (+8) goes down; with any left, the last takes the top place and sifts down as Person::Loop7E6750 does.
static volatile long g_person_heap_pops;
static void __thiscall person_points_7e6c20(uint32_t *self, const uint8_t *const *items) {
    g_person_heap_pops++;
    const int32_t n = (int32_t)--self[2];
    if (!n) return;
    uint32_t *const heap = (uint32_t *)(uintptr_t)self[0];
    heap[0] = heap[n];
    auto key = [&](uint32_t item) { return *(const float *)(*items + item * 20 + 8); };
    const uint32_t v = heap[0];
    int32_t hole = 0, c = 1;
    for (; c + 1 < n; c = 2 * hole + 1) {
        const int32_t pick = key(heap[c]) < key(heap[c + 1]) ? c : c + 1;
        if (!(key(heap[pick]) < key(v))) goto done;
        heap[hole] = heap[pick];
        hole = pick;
    }
    if (c < n && key(heap[c]) < key(v)) {
        heap[hole] = heap[c];
        hole = c;
    }
done:
    heap[hole] = v;
}
static const uint8_t person_points_7e6c20_original[] = {0x83, 0xec, 0x18, 0x8b, 0xd1, 0x8b, 0x42, 0x08, 0x48};
REWRITE("Person::Points7E6C20", 0x7e6c20, person_points_7e6c20_original, &person_points_7e6c20, &g_person_heap_pops);

// ---- Terrain::Cell8BCE50(out, x, y, corner) (0x8BCE50): the heights of the cells meeting at a grid point ---------------
// The point: x +0x14 + +0xC (unrounded; one step more with corner bit 0) and y +0x18 + +0x10 (a float; one step more
// with bit 1, the sum a float). The cells x - (bit 0 clear), +1 and y - (bit 1 clear), +1 that lie in the grid (bytes
// +4, +5; cells of 24 bytes at +0x24) each add {x, height, y} (16 bytes) to out, height the cell's corner 3 - 2 k - j
// (j, k the steps in x, y). Gives how many.
static volatile long g_terrain_point_cells;
static int32_t __thiscall terrain_cell_8bce50(const uint8_t *self, float *out, uint8_t x, uint8_t y, uint8_t corner) {
    g_terrain_point_cells++;
    auto m = [&](uint32_t at) { return *(const float *)(self + at); };
    long double px = (long double)x * m(0x14) + m(0xc);
    volatile float fy = (float)((long double)y * m(0x18) + m(0x10));
    const int32_t x0 = (int32_t)x - (~corner & 1), y0 = (int32_t)y - (~(corner >> 1) & 1);
    if (corner & 1) px = px + m(0x14);
    const long double py = (corner & 2) ? (long double)fy + m(0x18) : (long double)fy;
    int32_t count = 0;
    for (int32_t j = 0; j < 2; j++) {
        const int32_t xi = x0 + j;
        if (xi < 0 || xi >= self[4]) continue;
        for (int32_t k = 0; k < 2; k++) {
            const int32_t yi = y0 + k;
            if (yi < 0 || yi >= self[5]) continue;
            const uint32_t *const cell = (const uint32_t *)(*(const uint8_t *const *)(self + 0x24) + (self[4] * yi + xi) * 24);
            volatile float *const e = out + count * 4;
            e[0] = (float)px;
            e[2] = (float)py;
            ((volatile uint32_t *)e)[1] = cell[(int8_t)(3 - 2 * k - j)];
            count++;
        }
    }
    return count;
}
static const uint8_t terrain_cell_8bce50_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x24, 0x0f, 0xb6, 0x45, 0x0c};
REWRITE("Terrain::Cell8BCE50", 0x8bce50, terrain_cell_8bce50_original, &terrain_cell_8bce50, &g_terrain_point_cells);

// ---- Water::Value9061C0(x, y, n) (0x9061C0): a column's total over n rows ----------------------------------------------
// Rows of width +4. The sum of n values of +0x48 down from (x, y), then for the n - 1 rows from y + 1 each +0x50 value
// in and +0x4C value out (the sub first), all unrounded.
static volatile long g_water_column_sums;
static long double __thiscall water_value_9061c0(const uint8_t *self, int32_t x, int32_t y, int32_t n) {
    g_water_column_sums++;
    const int32_t w = *(const int32_t *)(self + 4);
    const float *const a = *(const float *const *)(self + 0x48), *const b = *(const float *const *)(self + 0x4c), *const c = *(const float *const *)(self + 0x50);
    long double s = *(const float *)0xf264e0;
    for (int32_t k = 0, i = w * y + x; k < n; k++, i += w) s = s + a[i];
    if (n > 1)
        for (int32_t k = 0, i = (y + 1) * w + x; k < n - 1; k++, i += w) {
            s = s - b[i];
            s = s + c[i];
        }
    return s;
}
static const uint8_t water_value_9061c0_original[] = {0x51, 0x8b, 0x41, 0x04, 0xd9, 0x05, 0xe0, 0x64, 0xf2, 0x00};
REWRITE("Water::Value9061C0", 0x9061c0, water_value_9061c0_original, &water_value_9061c0, &g_water_column_sums);

// ---- Water::Range95F910(pixels in ecx, width in edx, x0, x1, y0, y1) (0x95F910): fades a box of pixels to its edges ---
// hx = (x1 - x0) / 2, hy = (y1 - y0) / 2 (floats). For pixel (i, j) of the box: dx = (i + 0.5 - hx) / hx (by 1 / hx), dy
// = (j + 0.5 - hy) / hy; f = 1 - (dx^2 + dy^2), at least 0; the pixel's top byte becomes its value times f, rounded by the
// FPU.
static volatile long g_water_box_fades;
static void __fastcall water_range_95f910(uint32_t *pixels, int32_t width, int32_t x0, int32_t x1, int32_t y0, int32_t y1) {
    g_water_box_fades++;
    const long double half = *(const float *)0xf2651c, one = *(const float *)0xf264dc, zero = *(const float *)0xf264e0;
    volatile float hx = (float)((long double)(x1 - x0) * half);
    volatile float hy = (float)((long double)(y1 - y0) * half);
    if (y0 >= y1) return;
    uint32_t *row = pixels + (width * y0 + x0);
    for (int32_t j = 0, rows = y1 - y0; rows; j++, rows--, row += width) {
        if (x0 >= x1) continue;
        const long double inv = one / hx;
        const long double dy = (((long double)j + half) - hy) / hy;
        const long double dy2 = dy * dy;
        uint32_t *p = row;
        for (int32_t i = 0, cols = x1 - x0; cols; i++, cols--, p++) {
            const long double dx = (((long double)i + half) - hx) * inv;
            long double f = one - (dx * dx + dy2);
            if (f < zero) f = zero;
            const uint32_t v = *p;
            *p = (uint32_t)x87_fistp64_h2((long double)(int32_t)(v >> 24) * f) << 24 | (v & 0xffffff);
        }
    }
}
static const uint8_t water_range_95f910_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf8, 0x83, 0xec, 0x24, 0x8b, 0x45, 0x08};
REWRITE("Water::Range95F910", 0x95f910, water_range_95f910_original, &water_range_95f910, &g_water_box_fades);

// ---- Colour::Value6B5B90(colour) (0x6B5B90): a colour's hue, 0 to 1 -----------------------------------------------------
// r, g, b = the bytes 2, 1, 0 over 255 (unrounded, and as floats). By which is largest, middle and least (the game's
// compares and ties) it takes a sixth k (1, 3, 5, 7, 9, 11) and t = 2 (mid - least) / (largest - least) - 1 (negated in
// the odd sixths); (k + t) / 12, or k / 12 when all match.
static volatile long g_colour_hues;
static long double __stdcall colour_value_6b5b90(uint32_t c) {
    g_colour_hues++;
    const long double s = *(const float *)0xf270f0;
    const long double R = (long double)(int32_t)((c >> 16) & 0xff) * s;
    volatile float rf = (float)R;
    volatile float gf = (float)((long double)(int32_t)((c >> 8) & 0xff) * s);
    const long double B = (long double)(int32_t)(c & 0xff) * s;
    volatile float bf = (float)B;
    long double top, mid, least, k;
    bool flip;
    if (R > gf) {
        if (B > R) top = bf, mid = rf, least = gf, k = 9, flip = false;
        else if (B < gf) top = R, mid = gf, least = bf, k = 1, flip = false;
        else top = R, mid = bf, least = gf, k = 11, flip = true;
    } else if (B < R) top = gf, mid = rf, least = bf, k = 3, flip = true;
    else if (bf > gf) top = bf, mid = gf, least = rf, k = 7, flip = true;
    else top = gf, mid = bf, least = rf, k = 5, flip = false;
    volatile float kf = (float)k;
    const long double d = top - least;
    if (!(d > *(const float *)0xf264e0)) return ((long double)*(const float *)0xf264e0 + kf) * *(const float *)0xf3b798;
    long double t = (mid - least) / d;
    t = t + t;
    t = t - *(const float *)0xf264dc;
    if (flip) t = -t;
    return (t + kf) * *(const float *)0xf3b798;
}
static const uint8_t colour_value_6b5b90_original[] = {0x83, 0xec, 0x0c, 0x8b, 0x44, 0x24, 0x10, 0x8b, 0xc8, 0xc1, 0xf9, 0x10};
REWRITE("Colour::Value6B5B90", 0x6b5b90, colour_value_6b5b90_original, &colour_value_6b5b90, &g_colour_hues);

// ---- Window::Mode6A7DF0() (0x6A7DF0): greys out a spinner's down and up buttons by its value -----------------------------
// The value (+0xF4) by mode (+0xF0): 1 a float, 2 a whole number, 3 a byte (with byte step, least, most at +0xFA, +0xF8,
// +0xF9; else +0x100, +0xF8, +0xFC). The up button (+0xDC) gets flag 0x400000 (+0x94) when value + step is past the
// most, else loses it; the down button (+0xE0) gets it when value - step is below the least (a float compare in mode
// 1), else loses it. No value or another mode clears both. Gives the down button's new flags.
static volatile long g_spinner_greys;
static uint32_t __thiscall window_mode_6a7df0(const uint8_t *self) {
    g_spinner_greys++;
    bool down_ok = true, up_off = false;
    const void *const p = *(const void *const *)(self + 0xf4);
    switch (*(const int32_t *)(self + 0xf0)) {
    case 1:
        if (!p) break;
        if ((long double)*(const float *)p - *(const float *)(self + 0x100) < *(const float *)(self + 0xf8)) down_ok = false;
        up_off = (long double)*(const float *)(self + 0x100) + *(const float *)p > *(const float *)(self + 0xfc);
        break;
    case 2: {
        if (!p) break;
        const int32_t v = *(const int32_t *)p, step = *(const int32_t *)(self + 0x100);
        if (v - step < *(const int32_t *)(self + 0xf8)) down_ok = false;
        up_off = v + step > *(const int32_t *)(self + 0xfc);
        break;
    }
    case 3: {
        if (!p) break;
        const int32_t v = *(const uint8_t *)p, step = self[0xfa];
        if (v - step < self[0xf8]) down_ok = false;
        up_off = v + step > self[0xf9];
        break;
    }
    }
    uint32_t *const a = (uint32_t *)(*(uint8_t *const *)(self + 0xdc) + 0x94);
    *a = up_off ? *a | 0x400000 : *a & 0xffbfffff;
    uint32_t *const b = (uint32_t *)(*(uint8_t *const *)(self + 0xe0) + 0x94);
    *b = down_ok ? *b & 0xffbfffff : *b | 0x400000;
    return *b;
}
static const uint8_t window_mode_6a7df0_original[] = {0x8b, 0x81, 0xf0, 0x00, 0x00, 0x00, 0x48, 0x53, 0x55, 0x56};
REWRITE("Window::Mode6A7DF0", 0x6a7df0, window_mode_6a7df0_original, &window_mode_6a7df0, &g_spinner_greys);

// ---- Vector::AxisBFB340(out) (0xBFB340): a direction of length 1 at right angles to the vector --------------------------
// By the largest size (|x| unrounded against |y|, |z| as floats; ties go to x, then y): x gives (-z, 0, x), y gives (y,
// -x, 0), z gives (0, z, -y), each over its length (sqrt of the two squares); a length of 0 gives {0, 0, 1}. Gives out.
static volatile long g_vector_normals;
static float *__thiscall vector_axis_bfb340(const float *v, float *out) {
    g_vector_normals++;
    const long double zero = *(const float *)0xf264e0, one = *(const float *)0xf264dc;
    const long double ax = __builtin_fabsl((long double)v[0]);
    volatile float ay = (float)__builtin_fabsl((long double)v[1]), az = (float)__builtin_fabsl((long double)v[2]);
    volatile float *const o = out;
    auto none = [&] { ((volatile uint32_t *)o)[0] = 0, ((volatile uint32_t *)o)[1] = 0, ((volatile uint32_t *)o)[2] = 0x3f800000; };
    if (!(ax < ay) && !(ax < az)) {
        const long double a = -(long double)v[2], b = v[0];
        const long double len = x87_sqrt_h2(b * b + a * a);
        if (len == zero) return none(), out;
        const long double inv = one / len;
        o[0] = (float)(inv * a);
        o[1] = (float)(zero * inv);
        o[2] = (float)(b * inv);
        return out;
    }
    if (!(ay < az)) {
        const long double b = v[1];
        volatile float a = (float)-(long double)v[0];
        const long double len = x87_sqrt_h2((long double)a * a + b * b);
        if (len == zero) return none(), out;
        const long double inv = one / len;
        o[0] = (float)(inv * b);
        o[1] = (float)((long double)a * inv);
        o[2] = (float)(inv * zero);
        return out;
    }
    const long double b = v[2], a = -(long double)v[1];
    const long double len = x87_sqrt_h2(a * a + b * b);
    if (len == zero) return none(), out;
    const long double inv = one / len;
    o[0] = (float)(zero * inv);
    o[1] = (float)(b * inv);
    o[2] = (float)(a * inv);
    return out;
}
static const uint8_t vector_axis_bfb340_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x20, 0xd9, 0x01, 0xd9, 0xe1};
REWRITE("Vector::AxisBFB340", 0xbfb340, vector_axis_bfb340_original, &vector_axis_bfb340, &g_vector_normals);

// ---- Shop::Round4BB620(x, y) (0x4BB620): the height at a point from its cell's four corners -----------------------------
// As Grid::Round435EA0, with the corners read from the cell's +0x14 heights when it has them.
static volatile long g_shop_heights;
static long double __thiscall shop_round_4bb620(const uint8_t *self, float x, float y) {
    g_shop_heights++;
    auto at = [&](uint32_t o) { return *(const float *)(self + o); };
    volatile float u = (float)(((long double)x - at(0xc)) * at(0x1c));
    volatile float v = (float)(((long double)y - at(0x10)) * at(0x20));
    volatile float fu = (float)floor_h2(u), fv = (float)floor_h2(v);
    const int32_t i = (int32_t)x87_fistp64_h2(fu), j = (int32_t)x87_fistp64_h2(fv);
    if (i < 0 || j < 0 || i >= self[4] || j >= self[5]) return *(const float *)0xf28530;
    const uint8_t *const cell = *(const uint8_t *const *)(self + 0x24) + ((uint32_t)self[4] * j + i) * 24;
    const float *const h = *(const float *const *)(cell + 0x14) ? *(const float *const *)(cell + 0x14) : (const float *)cell;
    const long double dv = (long double)v - fv, du = (long double)u - fu, s = du + dv;
    const long double one = *(const float *)0xf264dc;
    if (s > one) return ((one - du) * h[2] + (one - dv) * h[1]) + (s - one) * h[3];
    return ((one - s) * h[0] + du * h[1]) + dv * h[2];
}
static const uint8_t shop_round_4bb620_original[] = {0x83, 0xec, 0x10, 0xd9, 0x44, 0x24, 0x14, 0x56, 0xd8, 0x61, 0x0c};
REWRITE("Shop::Round4BB620", 0x4bb620, shop_round_4bb620_original, &shop_round_4bb620, &g_shop_heights);

// ---- Person::Values4FEEB0(out) (0x4FEEB0): three values from the +0xB8 record's +0x600 group ----------------------------
// None (0, only al) without the record or its +0x66C. With v = +0x600: s = min(v, C) K1 (a float) + +0x604 K4, b = min(v,
// C2) K2 and c = min(v, C) K3; each of +0x60C, +0x610 above 0 adds E1 to s and E2 to c. out = ((min(+0x608, 11) K5 + s)
// K6, b K7, c K8) (the game's constants at 0xF27370, 0xF36810-0xF36824, 0xF348BC, 0xF283D8, 0xF33750, 0xF27060).
static volatile long g_person_values_a;
static uint8_t __thiscall person_values_4feeb0(const uint8_t *self, float *out) {
    g_person_values_a++;
    const uint8_t *const p = *(const uint8_t *const *)(self + 0xb8);
    if (!p || !*(const uint32_t *)(p + 0x66c)) return 0;
    auto k = [](uint32_t at) { return (long double)*(const float *)at; };
    auto m = [&](uint32_t at) { return (long double)*(const float *)(p + at); };
    auto least = [&](uint32_t c) { return m(0x600) < k(c) ? m(0x600) : k(c); };
    volatile float s0 = (float)(least(0xf27370) * k(0xf36824));
    const long double b = least(0xf36820) * k(0xf3681c);
    long double c = least(0xf27370) * k(0xf348bc);
    long double s = m(0x604) * k(0xf36818) + s0;
    if (*(const int32_t *)(p + 0x60c) > 0) s = s + k(0xf283d8), c = c + k(0xf33750);
    if (*(const int32_t *)(p + 0x610) > 0) s = s + k(0xf283d8), c = c + k(0xf33750);
    int32_t n = *(const int32_t *)(p + 0x608);
    if (n >= 11) n = 11;
    volatile float *const o = out;
    o[0] = (float)(((long double)n * k(0xf36814) + s) * k(0xf36810));
    o[1] = (float)(b * k(0xf27060));
    o[2] = (float)(c * k(0xf3680c));
    return 1;
}
static const uint8_t person_values_4feeb0_original[] = {0x51, 0x8b, 0x89, 0xb8, 0x00, 0x00, 0x00, 0x85, 0xc9};
REWRITE("Person::Values4FEEB0", 0x4feeb0, person_values_4feeb0_original, &person_values_4feeb0, &g_person_values_a);

// ---- Person::Values4FE8F0(out) (0x4FE8F0): three values from the +0xB8 record's +0x5E0 group ----------------------------
// None (0, only al) without the record or its +0x66C. With g the game's 0x12F5034: a = g +0x5E0 / 2, b = g +0x5E4 K (a
// float), c = g +0x5E8 / 2 (a float). x0 = K1 a - max(b, K4) K5 (a float) + min(c, K7) K8; x1 = K2 a (a float) - (b -
// 1) K2; x2 = a K3 (a float) - (b - 1, a float) K6. c above K9 adds K10 to x1 and K11 to x2; above K12 also K13 and K12,
// and halves x0. out = (x0 K14, (c + x1) K15, (c K16 + x2) K17) (the game's constants at 0xF36770-0xF36790 and others).
static volatile long g_person_values_b;
static uint8_t __thiscall person_values_4fe8f0(const uint8_t *self, float *out) {
    g_person_values_b++;
    const uint8_t *const p = *(const uint8_t *const *)(self + 0xb8);
    if (!p || !*(const uint32_t *)(p + 0x66c)) return 0;
    auto k = [](uint32_t at) { return (long double)*(const float *)at; };
    auto m = [&](uint32_t at) { return (long double)*(const float *)(p + at); };
    const long double g = *(const volatile float *)0x12f5034;
    const long double a = g * m(0x5e0) * k(0xf2651c);
    volatile float bf = (float)(g * m(0x5e4) * k(0xf27ca8));
    volatile float cf = (float)(g * m(0x5e8) * k(0xf2651c));
    volatile float s8 = (float)(k(0xf36790) * a), sc = (float)(k(0xf28420) * a), s10 = (float)(a * k(0xf3678c));
    const long double t = bf > k(0xf36788) ? (long double)bf : k(0xf36788);
    s8 = (float)((long double)s8 - t * k(0xf36784));
    const long double bm1 = (long double)bf - k(0xf264dc);
    bf = (float)bm1;
    long double x1 = (long double)sc - bm1 * k(0xf28420);
    long double x2 = (long double)s10 - (long double)bf * k(0xf36780);
    const long double u = cf < k(0xf26648) ? (long double)cf : k(0xf26648);
    long double x0 = u * k(0xf355e4) + s8;
    if (cf > k(0xf26680)) x1 = x1 + k(0xf30600), x2 = x2 + k(0xf2705c);
    if (cf > k(0xf2669c)) {
        x1 = x1 + k(0xf3677c), x2 = x2 + k(0xf2669c);
        x0 = x0 * k(0xf2651c);
    }
    volatile float *const o = out;
    o[0] = (float)(x0 * k(0xf36778));
    o[1] = (float)(((long double)cf + x1) * k(0xf36774));
    o[2] = (float)(((long double)cf * k(0xf27060) + x2) * k(0xf36770));
    return 1;
}
static const uint8_t person_values_4fe8f0_original[] = {0x8b, 0x81, 0xb8, 0x00, 0x00, 0x00, 0x83, 0xec, 0x14, 0x85, 0xc0};
REWRITE("Person::Values4FE8F0", 0x4fe8f0, person_values_4fe8f0_original, &person_values_4fe8f0, &g_person_values_b);

// ---- Terrain::Scale8F2480(origin, size) (0x8F2480): the points around a grid's edge -----------------------------------
// With nx, ny = +8, +0xC and steps sx = size.x / nx, sy = size.y / ny (floats), the far corner e = origin + size (floats):
// points (16 bytes at +0x14; x and z written) go along the near x edge (origin.x + i sx, origin.y), up the far side (e.x,
// origin.y + i sy), back along the far edge (e.x - i sx, e.y) and down the near side (origin.x, e.y - i sy).
static volatile long g_terrain_edges;
static void __thiscall terrain_scale_8f2480(const uint8_t *self, const float *origin, const float *size) {
    g_terrain_edges++;
    const int32_t nx = *(const int32_t *)(self + 8), ny = *(const int32_t *)(self + 0xc);
    volatile float sx = (float)((long double)size[0] / nx), sy = (float)((long double)size[1] / ny);
    volatile float ex = (float)((long double)origin[0] + size[0]), ey = (float)((long double)origin[1] + size[1]);
    volatile float *p = *(float *const *)(self + 0x14);
    for (int32_t i = 0; i < nx; i++, p += 4) {
        p[0] = (float)((long double)i * sx + origin[0]);
        ((volatile uint32_t *)p)[2] = *(const uint32_t *)&origin[1];
    }
    for (int32_t i = 0; i < ny; i++, p += 4) {
        ((volatile uint32_t *)p)[0] = *(const volatile uint32_t *)&ex;
        p[2] = (float)((long double)i * sy + origin[1]);
    }
    for (int32_t i = 0; i < nx; i++, p += 4) {
        ((volatile uint32_t *)p)[2] = *(const volatile uint32_t *)&ey;
        p[0] = (float)((long double)ex - (long double)i * sx);
    }
    for (int32_t i = 0; i < ny; i++, p += 4) {
        ((volatile uint32_t *)p)[0] = *(const uint32_t *)&origin[0];
        p[2] = (float)((long double)ey - (long double)i * sy);
    }
}
static const uint8_t terrain_scale_8f2480_original[] = {0x83, 0xec, 0x10, 0x56, 0x8b, 0x74, 0x24, 0x1c, 0x57, 0x8b, 0x79, 0x08};
REWRITE("Terrain::Scale8F2480", 0x8f2480, terrain_scale_8f2480_original, &terrain_scale_8f2480, &g_terrain_edges);

// ---- Person::Values4FEDF0(out) (0x4FEDF0): three values from the +0xB8 record's +0x5F0 group ----------------------------
// None (0, only al) without the record or its +0x66C. With n = +0x5F0 and w = +0x5F4: out = ((w K7 + min(n, 9) K1) K8,
// (min(n, 12) K2 + w K3) K4, (min(n, 12) K5 + w K6) / 2) (the game's constants at 0xF367F0-0xF36808 and 0xF270B0).
static volatile long g_person_values_c;
static uint8_t __thiscall person_values_4fedf0(const uint8_t *self, float *out) {
    g_person_values_c++;
    const uint8_t *const p = *(const uint8_t *const *)(self + 0xb8);
    if (!p || !*(const uint32_t *)(p + 0x66c)) return 0;
    auto k = [](uint32_t at) { return (long double)*(const float *)at; };
    const int32_t n = *(const int32_t *)(p + 0x5f0);
    const long double w = *(const float *)(p + 0x5f4);
    const long double a = (long double)(n >= 9 ? 9 : n) * k(0xf36808);
    const long double b = ((long double)(n >= 12 ? 12 : n) * k(0xf36804) + w * k(0xf36800)) * k(0xf367fc);
    const long double c = ((long double)(n >= 12 ? 12 : n) * k(0xf270b0) + w * k(0xf367f8)) * k(0xf2651c);
    volatile float *const o = out;
    o[0] = (float)((w * k(0xf367f4) + a) * k(0xf367f0));
    o[1] = (float)b;
    o[2] = (float)c;
    return 1;
}
static const uint8_t person_values_4fedf0_original[] = {0x51, 0x8b, 0x89, 0xb8, 0x00, 0x00, 0x00, 0x85, 0xc9};
REWRITE("Person::Values4FEDF0", 0x4fedf0, person_values_4fedf0_original, &person_values_4fedf0, &g_person_values_c);

// ---- Person::Values500780(out) (0x500780): a total over the +0x180 list -----------------------------------------------
// None (0) without +0xB8. For each entry (item +8: {kind, count +4, f +8}): t = kind's +0x3C K1 K2 / 2 times g = max(1 -
// (max(f, F) - F) 0.05, 0); times K4 with a count above 1, K5 above 2, and K6 instead above 3 (the game's constants at
// 0xF27090, 0xF2CF10, 0xF27630, 0xF27068, 0xF29658, 0xF32894). out = (the total, 0, 0); gives 1.
static volatile long g_person_values_d;
static uint8_t __thiscall person_values_500780(const uint8_t *self, float *out) {
    g_person_values_d++;
    if (!*(const uint32_t *)(self + 0xb8)) return 0;
    auto k = [](uint32_t at) { return (long double)*(const float *)at; };
    long double s = k(0xf264e0);
    for (const uint32_t *p = *(const uint32_t *const *)(self + 0x180); p[0] && p; p = (const uint32_t *)(uintptr_t)p[0]) {
        __asm__ volatile("" ::: "memory");
        const uint8_t *const item = (const uint8_t *)(uintptr_t)p[2];
        const uint8_t *const kind = *(const uint8_t *const *)item;
        long double t = (long double)*(const int32_t *)(kind + 0x3c) * k(0xf27090);
        t = t * k(0xf2cf10);
        t = t * k(0xf2651c);
        const float f = *(const float *)(item + 8);
        const long double fm = f > *(const float *)0xf27630 ? (long double)f : k(0xf27630);
        long double g = k(0xf264dc) - (fm - k(0xf27630)) * k(0xf270b4);
        if (k(0xf264e0) > g) g = k(0xf264e0);
        long double v = t * g;
        const int32_t c = *(const int32_t *)(item + 4);
        if (c > 1) v = v * k(0xf27068);
        if (c > 2) v = v * k(0xf29658);
        if (c > 3) v = k(0xf32894);
        s = s + v;
    }
    volatile float *const o = out;
    o[0] = (float)s;
    ((volatile uint32_t *)o)[1] = 0, ((volatile uint32_t *)o)[2] = 0;
    return 1;
}
static const uint8_t person_values_500780_original[] = {0x8b, 0x81, 0xb8, 0x00, 0x00, 0x00, 0xd9, 0x05, 0xe0, 0x64, 0xf2, 0x00};
REWRITE("Person::Values500780", 0x500780, person_values_500780_original, &person_values_500780, &g_person_values_d);

// ---- Person::Scale4FF2F0(x, y) (0x4FF2F0): x halved for each of the game's levels y reaches -----------------------------
// Five levels (doubles at 0xF36880, 0xF36878, 0xF36870, 0xF36868, 0xF36860): each one y is at or past takes v - v / 2;
// at or past 0xF27630 (a float) v is then scaled by 0xF270B0.
static volatile long g_person_scales;
static long double __stdcall person_scale_4ff2f0(float x, float y) {
    g_person_scales++;
    const long double h = *(const float *)0xf2651c;
    long double v = x;
    if (y >= *(const double *)0xf36880) v = (long double)x - (long double)x * h;
    if (y >= *(const double *)0xf36878) v = v - h * v;
    if (y >= *(const double *)0xf36870) v = v - h * v;
    if (y >= *(const double *)0xf36868) v = v - h * v;
    if (y >= *(const double *)0xf36860) v = v - h * v;
    if (y >= *(const float *)0xf27630) v = v * *(const float *)0xf270b0;
    return v;
}
static const uint8_t person_scale_4ff2f0_original[] = {0xd9, 0x44, 0x24, 0x08, 0xdc, 0x1d, 0x80, 0x68, 0xf3, 0x00};
REWRITE("Person::Scale4FF2F0", 0x4ff2f0, person_scale_4ff2f0_original, &person_scale_4ff2f0, &g_person_scales);

// ---- Ride::Compare59A1E0(a, b, c, d) (0x59A1E0): whether d lies on the way from a to b, going c's way --------------------
// c above 0: with b at or past a, a <= d <= b; else a <= d or d <= b. c below 0: with b at or below a, b <= d <= a; else
// b <= d or d <= a. c 0 gives 0 (only al).
static volatile long g_ride_arcs;
static uint8_t __stdcall ride_compare_59a1e0(float a, float b, float c, float d) {
    g_ride_arcs++;
    if (c > *(const float *)0xf264e0) {
        if (!(b < a || __builtin_isunordered(b, a))) return a <= d && d <= b;
        return a <= d || d <= b;
    }
    if (!(c < *(const float *)0xf264e0)) return 0;
    if (b <= a) return b <= d && d <= a;
    return b <= d || d <= a;
}
static const uint8_t ride_compare_59a1e0_original[] = {0xd9, 0x44, 0x24, 0x0c, 0xd8, 0x1d, 0xe0, 0x64, 0xf2, 0x00};
REWRITE("Ride::Compare59A1E0", 0x59a1e0, ride_compare_59a1e0_original, &ride_compare_59a1e0, &g_ride_arcs);

// ---- Terrain::Range8F47F0(grid) (0x8F47F0): the heights around a grid's edge -------------------------------------------
// Into +0x10, for a grid of w x h cells (bytes +4, +5; cells of 24 bytes at +0x24 with corner heights 0-3) and W = w - 1,
// H = h - 1 (bytes): the first cell's corner 0, then along the near row (c(i)[0] + c(i - 1)[1]) / 2 for i = 1..W and the
// last cell's corner 1; up the far column (c(j, W)[1] + c(j - 1, W)[3]) / 2 for j = 1..H and its corner 3; back along
// the far row (c(H, i)[3] + c(H, i + 1)[2]) / 2 down to i = 0 and its corner 2; down the near column (c(j, 0)[2] + c(j +
// 1, 0)[0]) / 2 down to j = 0.
static volatile long g_terrain_edge_heights;
static void __thiscall terrain_range_8f47f0(const uint8_t *self, const uint8_t *grid) {
    g_terrain_edge_heights++;
    const long double half = *(const float *)0xf2651c;
    volatile float *o = *(float *const *)(self + 0x10);
    const uint8_t W = (uint8_t)(grid[4] - 1), H = (uint8_t)(grid[5] - 1);
    auto c = [&](uint32_t cell) { return (const float *)(*(const uint8_t *const *)(grid + 0x24) + cell * 24); };
    auto raw = [&](const float *f) { *(volatile uint32_t *)o++ = *(const uint32_t *)f; };
    raw(c(0));
    for (uint32_t i = 1; i <= W; i++) *o++ = (float)(((long double)c(i)[0] + c(i - 1)[1]) * half);
    raw(c(W) + 1);
    for (uint32_t j = 1; j <= H; j++) *o++ = (float)(((long double)c(j * grid[4] + W)[1] + c((j - 1) * grid[4] + W)[3]) * half);
    raw(c(H * grid[4] + W) + 3);
    for (uint32_t i = W; i-- > 0;) *o++ = (float)(((long double)c(H * grid[4] + i)[3] + c(H * grid[4] + i + 1)[2]) * half);
    raw(c(H * grid[4]) + 2);
    for (uint32_t j = H; j-- > 0;) *o++ = (float)(((long double)c(j * grid[4])[2] + c((j + 1) * grid[4])[0]) * half);
}
static const uint8_t terrain_range_8f47f0_original[] = {0x83, 0xec, 0x08, 0x8b, 0x41, 0x10, 0x53, 0x55, 0x56};
REWRITE("Terrain::Range8F47F0", 0x8f47f0, terrain_range_8f47f0_original, &terrain_range_8f47f0, &g_terrain_edge_heights);

// ---- Fireworks::Count99C360(n, t, show, out) (0x99C360): moves a show's start time clear of the others in slot n --------
// out = t. The show's timing (show +4's +0x18C: start +0x7C, end +0x84); going up when its start is past t. While out
// stays at or below the show's start going up (at or above going down), each other show (+0xE8 list of +0xEC, each's
// +4) not unset (-1) or of the same group (+0x194), in slot floor(+0x80 / the game's 0x13413B0) = n and overlapping
// ((length + out) - 2 past its start + 2, out + 2 before its end - 2) moves out (to its end - 4 going up, else its
// start - length + 4) and the search starts over.
// Gives 1 when nothing was moved, else 0 (only al).
static volatile long g_firework_slots;
static uint8_t __thiscall fireworks_count_99c360(const uint8_t *self, int32_t n, float t, const uint8_t *show, float *out) {
    g_firework_slots++;
    auto k = [](uint32_t at) { return (long double)*(const float *)at; };
    *out = t;
    const uint8_t *const s0 = *(const uint8_t *const *)(*(const uint8_t *const *)(show + 4) + 0x18c);
    const bool up = *(const float *)(s0 + 0x7c) > t;
    uint8_t moved_none = 1;
    for (int32_t i = 0; i < *(const int32_t *)(self + 0xec); i++) {
        __asm__ volatile("" ::: "memory");
        const uint8_t *const sp = *(const uint8_t *const *)(show + 4);
        const uint8_t *const s = *(const uint8_t *const *)(sp + 0x18c);
        const float v = *out;
        if (up ? !(v <= *(const float *)(s + 0x7c)) : !(v >= *(const float *)(s + 0x7c))) break;
        const uint8_t *const other = *(const uint8_t *const *)(*(const uint8_t *const *)(*(const uint8_t *const *)(self + 0xe8) + i * 4) + 4);
        const int32_t group = *(const int32_t *)(other + 0x194);
        if (group == -1 || group == *(const int32_t *)(sp + 0x194)) continue;
        const uint8_t *const o = *(const uint8_t *const *)(other + 0x18c);
        volatile float q = (float)((long double)*(const float *)(o + 0x80) / *(const volatile float *)0x13413b0);
        volatile float f = (float)floor_h2(q);
        if ((int32_t)x87_fistp64_h2(f) != n) continue;
        const long double e = k(0xf2705c);
        const long double len = (long double)*(const float *)(s + 0x84) - *(const float *)(s + 0x7c);
        if (!((len + *out) - e > (long double)*(const float *)(o + 0x7c) + e)) continue;
        if (!((long double)*out + e < (long double)*(const float *)(o + 0x84) - e)) continue;
        moved_none = 0;
        i = -1;
        if (up) *(volatile float *)out = (float)((long double)*(const float *)(o + 0x84) - k(0xf2669c));
        else *(volatile float *)out = (float)(((long double)*(const float *)(o + 0x7c) - len) + k(0xf2669c));
    }
    return moved_none;
}
static const uint8_t fireworks_count_99c360_original[] = {0x83, 0xec, 0x0c, 0xd9, 0x44, 0x24, 0x14, 0x53, 0x8b, 0x5c, 0x24, 0x20};
REWRITE("Fireworks::Count99C360", 0x99c360, fireworks_count_99c360_original, &fireworks_count_99c360, &g_firework_slots);

// ---- Fireworks::Height9E0B90(point, keep_height) (0x9E0B90): wraps a point back into the game's box ----------------------
// Box middle (0x14002C0..C8) and half sizes (+0x10 and +0x18 plus the game's margin 0x10F0E94, and +0x14). Past one side
// a part comes back in from the other by as much (x with the high side and the overshoot floats, z with the high side a
// float, y (only without keep_height) with both sides floats and the result unrounded). When anything changed the point
// takes the new values, +0x44 counts one more and gives 1; else 0 (only al).
static volatile long g_firework_wraps;
static uint8_t __thiscall fireworks_height_9e0b90(uint8_t *self, float *point, uint8_t keep_height) {
    g_firework_wraps++;
    auto m = [&](uint32_t at) { return (long double)*(const float *)(self + at); };
    auto g = [](uint32_t at) { return (long double)*(const volatile float *)at; };
    volatile float px = point[0], pz = point[2];
    long double py = point[1];
    const long double a = g(0x10f0e94) + m(0x10);
    volatile float bz = (float)(g(0x10f0e94) + m(0x18));
    {
        volatile float hi = (float)(a + g(0x14002c0));
        volatile float d1 = (float)((long double)px - hi);
        const long double lo = g(0x14002c0) - a;
        volatile float d2 = (float)((long double)px - lo);
        if (d1 > *(const float *)0xf264e0) px = (float)(lo + d1);
        else if (d2 < *(const float *)0xf264e0) px = (float)((long double)hi + d2);
    }
    {
        volatile float hi = (float)(g(0x14002c8) + bz);
        const long double d1 = (long double)pz - hi;
        const long double lo = g(0x14002c8) - bz;
        volatile float d2 = (float)((long double)pz - lo);
        if (d1 > *(const float *)0xf264e0) pz = (float)(lo + d1);
        else if (d2 < *(const float *)0xf264e0) pz = (float)((long double)hi + d2);
    }
    {
        const long double hi = g(0x14002c4) + m(0x14);
        volatile float hif = (float)hi;
        volatile float d1 = (float)(py - hi);
        const long double lo = g(0x14002c4) - m(0x14);
        volatile float lof = (float)lo;
        volatile float d2 = (float)(py - lo);
        if (!keep_height) {
            if (d1 > *(const float *)0xf264e0) py = (long double)lof + d1;
            else if (d2 < *(const float *)0xf264e0) py = (long double)hif + d2;
        }
    }
    if (px == point[0] && py == point[1] && pz == point[2]) return 0;
    *(volatile uint32_t *)&point[0] = *(const volatile uint32_t *)&px;
    *(volatile float *)&point[1] = (float)py;
    *(volatile uint32_t *)&point[2] = *(const volatile uint32_t *)&pz;
    (*(int32_t *)(self + 0x44))++;
    return 1;
}
static const uint8_t fireworks_height_9e0b90_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x40, 0x8b, 0x55, 0x08};
REWRITE("Fireworks::Height9E0B90", 0x9e0b90, fireworks_height_9e0b90_original, &fireworks_height_9e0b90, &g_firework_wraps);

// ---- Water::Update8FEAE0() (0x8FEAE0): the flow of a water feature when on (+0x2C) -------------------------------------
// The box (+0x10..+0x1C): width, depth and middle as floats; k = +0x20 / (count +8 - 1). Modes (+0x34) 4-7: with the
// width (4, 5) or depth s: +0x44 = k s / 1000 (at most 0.1 by the unrounded value), +0x48 = pi / s 15 (above 8.976 set
// to 8.976, below 4.189 set to 4.189), +0x4C = 0, +0x44 times 2 - (+0x48 - 4.189) 0.2089, +0x50 = ceil(+0xC 5 +0x48 /
// pi) pi / +0x48. Mode 8: +0x44 = k 0.0075, +0x4C = the angle (0 to 2 pi) from the middle to the last point (12 bytes
// at +0), +0x50 = +0xC 5. (The game's constants at 0xF285F0, 0xF2CF10, 0xF29638, 0xF7CFAC-0xF7CFB8 and more.)
static volatile long g_water_flows;
static void __thiscall water_update_8feae0(uint8_t *self) {
    g_water_flows++;
    if (!self[0x2c]) return;
    auto k = [](uint32_t at) { return (long double)*(const float *)at; };
    float *const f = (float *)self;
    volatile float *const vf = (volatile float *)self;
    volatile float w = (float)((long double)f[6] - f[4]), d = (float)((long double)f[7] - f[5]);
    volatile float cx = (float)(((long double)f[4] + f[6]) * k(0xf2651c)), cz = (float)(((long double)f[7] + f[5]) * k(0xf2651c));
    const int32_t n = *(const int32_t *)(self + 8);
    const long double per = (long double)f[8] / (long double)(n - 1);
    const int32_t mode = *(const int32_t *)(self + 0x34);
    if (mode >= 4 && mode <= 7) {
        const long double s = (mode == 4 || mode == 5) ? (long double)w : (long double)d;
        vf[0x44 / 4] = (float)(per * s);
        vf[0x48 / 4] = (float)(k(0xf285f0) / s);
        const long double a = (long double)vf[0x44 / 4] * k(0xf2cf10);
        vf[0x44 / 4] = (float)a;
        volatile float b = (float)((long double)vf[0x48 / 4] * k(0xf29638));
        vf[0x48 / 4] = b;
        if (a > k(0xf270b0)) *(volatile uint32_t *)(self + 0x44) = 0x3dcccccd;
        if (b > *(const float *)0xf7cfb8) *(volatile uint32_t *)(self + 0x48) = 0x410f9d9d;
        if (vf[0x48 / 4] < *(const float *)0xf7cfb4) *(volatile uint32_t *)(self + 0x48) = 0x40860a92;
        *(volatile uint32_t *)(self + 0x4c) = 0;
        vf[0x44 / 4] = (float)(((k(0xf264dc) - ((long double)vf[0x48 / 4] - k(0xf7cfb4)) * k(0xf7cfb0)) + k(0xf264dc)) * vf[0x44 / 4]);
        const long double v = (long double)f[3] * k(0xf28214);
        vf[0x50 / 4] = (float)v;
        volatile float t = (float)(v * vf[0x48 / 4] * k(0xf7cfac));
        volatile float c = (float)ceil_h2(t);
        vf[0x50 / 4] = (float)((long double)c * k(0xf285f0) / vf[0x48 / 4]);
        return;
    }
    if (mode != 8) return;
    const float *const last = *(const float *const *)self + n * 3 - 3;
    long double ang = x87_atan2_h2((long double)last[1] - cz, (long double)last[0] - cx);
    if (ang < *(const float *)0xf264e0) ang = ang + k(0xf268dc);
    vf[0x44 / 4] = (float)(per * k(0xf367c4));
    vf[0x4c / 4] = (float)ang;
    vf[0x50 / 4] = (float)((long double)f[3] * k(0xf28214));
}
static const uint8_t water_update_8feae0_original[] = {0x8a, 0x41, 0x2c, 0x83, 0xec, 0x14, 0x84, 0xc0};
REWRITE("Water::Update8FEAE0", 0x8feae0, water_update_8feae0_original, &water_update_8feae0, &g_water_flows);

// ---- Animal::Pass414470() (0x414470): pulls each linked pair of points back to its length -----------------------------
// Links (12 bytes at +0x80, count +0x84: {a, b, length}). d = b - a (x unrounded, y and z floats), L = sqrt((z z + y y)
// + x x) and f = (L - length) / L (a float). With both points free (+0x40 not 0) each moves half of d f toward the other;
// with a fixed (its +0x40 is 0) b moves all of it back, else a moves all of it. Roundings as the game's.
static volatile long g_animal_links;
static void __thiscall animal_pass_414470(const uint8_t *self) {
    g_animal_links++;
    const long double half = *(const float *)0xf2651c, zero = *(const float *)0xf264e0;
    for (int32_t i = 0; i < *(const int32_t *)(self + 0x84); i++) {
        const uint32_t *const e = (const uint32_t *)(*(const uint8_t *const *)(self + 0x80) + i * 12);
        volatile float *const a = (float *)(uintptr_t)e[0], *const b = (float *)(uintptr_t)e[1];
        const long double dx = (long double)b[0] - a[0];
        volatile float dy = (float)((long double)b[1] - a[1]);
        const long double dzu = (long double)b[2] - a[2];
        volatile float dz = (float)dzu;
        const long double len = x87_sqrt_h2((dzu * dz + (long double)dy * dy) + dx * dx);
        volatile float f = (float)((len - *(const float *)&e[2]) / len);
        const bool afree = !(a[0x40 / 4] == zero), bfree = !(b[0x40 / 4] == zero);
        if (afree && bfree) {
            const long double hx = dx * half;
            volatile float hxf = (float)hx;
            const long double hy = (long double)dy * half, hz = (long double)dz * half;
            volatile float hzf = (float)hz;
            volatile float mx = (float)((long double)hxf * f), my = (float)(hy * f);
            const long double mz = hz * f;
            a[0] = (float)((long double)mx + a[0]);
            a[1] = (float)((long double)my + a[1]);
            a[2] = (float)(mz + a[2]);
            volatile float hx2 = (float)hx;
            volatile float nx = (float)((long double)hx2 * f), ny = (float)(hy * f);
            const long double nz = (long double)hzf * f;
            b[0] = (float)((long double)b[0] - nx);
            b[1] = (float)((long double)b[1] - ny);
            b[2] = (float)((long double)b[2] - nz);
            continue;
        }
        const long double mx = dx * f, my = (long double)dy * f;
        volatile float mz = (float)((long double)dz * f);
        if (!afree) {
            b[0] = (float)((long double)b[0] - mx);
            b[1] = (float)((long double)b[1] - my);
            b[2] = (float)((long double)b[2] - mz);
        } else {
            a[0] = (float)(mx + a[0]);
            a[1] = (float)(my + a[1]);
            a[2] = (float)((long double)mz + a[2]);
        }
    }
}
static const uint8_t animal_pass_414470_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x81, 0xec, 0x84, 0x00, 0x00, 0x00};
REWRITE("Animal::Pass414470", 0x414470, animal_pass_414470_original, &animal_pass_414470, &g_animal_links);

// ---- Scenery::Flat8B5F60(target in ecx, out in edx, g, a, b, most, time, -) (0x8B5F60): how to throw to a target -------
// D = the flat (x, z) distance (a float), c = b - y a and T = sqrt((c - sqrt(c c - (D D + y y) g)) / (g / 2)) (a float).
// None (0, only al) when T is past most or not above 0. Else the start velocity: across D / T, up y / T + T a / 2 (a
// float), along the flat direction (x, z made of length 1; (x, 0, 1) at no distance), made of length 1 ((x, y, 1) at
// none) into out; time (when given) gets T; gives 1. Roundings as the game's.
static volatile long g_throw_solves;
static uint8_t __fastcall scenery_flat_8b5f60(const float *t, float *out, float g, float a, float b, float most, float *time, uint32_t) {
    g_throw_solves++;
    const long double zero = *(const float *)0xf264e0, one = *(const float *)0xf264dc, half = *(const float *)0xf7a7e4;
    const long double x = t[0], z = t[2];
    volatile float yf = t[1];
    volatile float df = (float)x87_sqrt_h2(z * z + x * x);
    const long double c = (long double)b - (long double)yf * a;
    const long double disc = c * c - ((long double)df * df + (long double)yf * yf) * g;
    const long double tt = x87_sqrt_h2((c - x87_sqrt_h2(disc)) / ((long double)g * half));
    volatile float tf = (float)tt;
    if (tt > most) return 0;
    if (tf < zero || tf == zero) return 0;
    const long double inv = one / tf;
    volatile float across = (float)((long double)df * inv);
    volatile float up = (float)(inv * yf + (long double)tf * a * half);
    const long double d2 = x87_sqrt_h2(z * z + x * x);
    long double nx, nz;
    volatile float ny = 0.0f;
    if (d2 == zero) nz = one, nx = x;
    else {
        const long double inv2 = one / d2;
        nx = x * inv2;
        ny = (float)(zero * inv2);
        nz = z * inv2;
    }
    volatile float vx = (float)((long double)across * nx);
    volatile float vy = (float)((long double)(float)((long double)ny * across) + up);
    volatile float vz = (float)(nz * across);
    volatile float len = (float)x87_sqrt_h2(((long double)vz * vz + (long double)vy * vy) + (long double)vx * vx);
    volatile float *const o = out;
    if (len == zero) {
        *(volatile uint32_t *)&o[0] = *(const volatile uint32_t *)&vx;
        *(volatile uint32_t *)&o[2] = 0x3f800000;
        *(volatile uint32_t *)&o[1] = *(const volatile uint32_t *)&vy;
    } else {
        const long double invl = one / len;
        volatile float invf = (float)invl;
        const long double ox = invl * vx;
        volatile float oy = (float)((long double)vy * invf);
        o[0] = (float)ox;
        o[2] = (float)((long double)vz * invf);
        *(volatile uint32_t *)&o[1] = *(const volatile uint32_t *)&oy;
    }
    if (time) *(volatile float *)time = tf;
    return 1;
}
static const uint8_t scenery_flat_8b5f60_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x40, 0xd9, 0x01};
REWRITE("Scenery::Flat8B5F60", 0x8b5f60, scenery_flat_8b5f60_original, &scenery_flat_8b5f60, &g_throw_solves);

// ---- Animal::ValueA88470() (0xA88470): an enclosure's score ------------------------------------------------------------
// Each part is capped at 1 (by the unrounded value) and weighted: n (+0x2C) K1 by K2; +0x7C 5 / t and +0x78 5 / t by
// K3 (t the total of +0x2C..+0x3C, a float); sqrt(+0x84 / m - (+0x80^2) / (m^2)) / 10 by K3 (m = +0x28); +0x70 K5 / t by
// 15; with n above 0 also +0x60 K5 / n by 5 and +0x6C 20 / n by K5. The sum times (1 - h) / 90 (75 with no n), where h =
// 1 / the game's 0x13CDF20 / m +0xC / 10, or 0 when h is past 1. (K1 0xF348BC, K2 0xF315C4, K3 0xF2706C, K5 0xF275F4.)
static volatile long g_animal_scores;
static long double __thiscall animal_value_a88470(const uint8_t *self) {
    g_animal_scores++;
    auto k = [](uint32_t at) { return (long double)*(const float *)at; };
    auto i = [&](uint32_t at) { return *(const int32_t *)(self + at); };
    const long double one = k(0xf264dc);
    auto cap = [&](long double v) { return v > one ? one : v; };
    const int32_t n = i(0x2c), m = i(0x28);
    volatile float nf = (float)n;
    volatile float tf = (float)(i(0x3c) + i(0x38) + i(0x34) + i(0x30) + n);
    long double acc = cap((long double)n * k(0xf348bc)) * k(0xf315c4);
    acc = acc + cap((long double)i(0x7c) * k(0xf28214) / tf) * k(0xf2706c);
    acc = acc + cap((long double)i(0x78) * k(0xf28214) / tf) * k(0xf2706c);
    const int32_t p = i(0x80);
    const long double spread = (long double)i(0x84) / (long double)m - (long double)(int32_t)((uint32_t)p * (uint32_t)p) / (long double)(int32_t)((uint32_t)m * (uint32_t)m);
    acc = acc + cap(x87_sqrt_h2(spread) * k(0xf270b0)) * k(0xf2706c);
    acc = acc + cap((long double)i(0x70) * k(0xf275f4) / tf) * k(0xf29638);
    long double base = 75.0L;
    if (n > 0) {
        acc = acc + cap((long double)i(0x60) * k(0xf275f4) / nf) * k(0xf28214);
        acc = acc + cap((long double)i(0x6c) * k(0xf27630) / nf) * k(0xf275f4);
        base = 90.0L;
    }
    long double h = one / *(const volatile float *)0x13cdf20;
    h = h / (long double)m;
    h = h * *(const float *)(self + 0xc);
    h = h * k(0xf270b0);
    if (h > one) return k(0xf264e0) / base * acc;
    return (one - h) / base * acc;
}
static const uint8_t animal_value_a88470_original[] = {0x83, 0xec, 0x0c, 0x8b, 0x41, 0x3c, 0x8b, 0x51, 0x34, 0x56};
REWRITE("Animal::ValueA88470", 0xa88470, animal_value_a88470_original, &animal_value_a88470, &g_animal_scores);

// ---- Water::Range941EF0(level, a, b, grid) (0x941EF0): whether water at level gets from cell a to cell b ---------------
// a, b are byte (x, y) places in the grid (width byte +4; cells of 24 bytes at +0x24, corner heights 0-3). The two
// corners of each cell on their shared side (by which way b lies: x first, then y; the same cell its 2, 1) are p0, p1
// and q0, q1. 1 when p0 and q0, or p1 and q1, are both below the level; else when the side crosses (p0 and q1, or p1 and
// q0, below) and d = ((q1 - q0) - p0) + p1 is larger than 0xF3312C, whether p0 + (p1 - p0) (p0 - q0) / d is below; else
// 0 (only al).
static volatile long g_water_crossings;
static uint8_t __stdcall water_range_941ef0(float level, const uint8_t *a, const uint8_t *b, const uint8_t *grid) {
    g_water_crossings++;
    const uint32_t w = grid[4];
    auto cell = [&](const uint8_t *p) { return (const float *)(*(const uint8_t *const *)(grid + 0x24) + (p[1] * w + p[0]) * 24); };
    const float *const A = cell(a), *const B = cell(b);
    int ia0, ia1, ib0, ib1;
    if (a[0] > b[0]) ia0 = 0, ia1 = 2, ib0 = 1, ib1 = 3;
    else if (a[0] < b[0]) ia0 = 1, ia1 = 3, ib0 = 0, ib1 = 2;
    else if (a[1] > b[1]) ia0 = 0, ia1 = 1, ib0 = 2, ib1 = 3;
    else if (a[1] < b[1]) ia0 = 2, ia1 = 3, ib0 = 0, ib1 = 1;
    else ia0 = 2, ia1 = 1, ib0 = 2, ib1 = 1;
    const long double p0 = A[ia0], p1 = A[ia1], q0 = B[ib0], q1 = B[ib1], L = level;
    if ((p0 < L && q0 < L) || (p1 < L && q1 < L)) return 1;
    if (!((p0 < L && q1 < L) || (p1 < L && q0 < L))) return 0;
    const long double d = ((q1 - q0) - p0) + p1;
    if (!(__builtin_fabsl(d) > *(const float *)0xf3312c)) return 0;
    const long double t = (p0 - q0) / d;
    return (p1 - p0) * t + p0 < L;
}
static const uint8_t water_range_941ef0_original[] = {0x8b, 0x44, 0x24, 0x08, 0x8a, 0x08, 0x55, 0x8b, 0x6c, 0x24, 0x10};
REWRITE("Water::Range941EF0", 0x941ef0, water_range_941ef0_original, &water_range_941ef0, &g_water_crossings);

// ---- Animal::PointA6ED40(out) (0xA6ED40): the mean offset of a shape's points -------------------------------------------
// Over the n (+0x10) points (pointers at +0xC): out = (sum of +0x10..+0x18 - sum of +0x20..+0x28) / (n K) (K the game's
// 0xF8F954; the scale a float). The sums go four points at a time and then one at a time, some kept unrounded between
// steps exactly as the game's are.
static volatile long g_animal_means;
static float *__thiscall animal_point_a6ed40(const uint8_t *self, float *out) {
    g_animal_means++;
    const int32_t n = *(const int32_t *)(self + 0x10);
    const float *const *const list = *(const float *const *const *)(self + 0xc);
    long double x = *(const float *)0xf264e0, ureg = *(const float *)0xf264e0;
    volatile float y = 0, z = 0, u = 0, v = 0, w = 0, xf;
    auto uvw = [&](const float *e) {
        u = (float)((long double)u + e[8]);
        v = (float)((long double)v + e[9]);
        w = (float)((long double)w + e[10]);
    };
    int32_t i = 0;
    if (n >= 4) {
        for (uint32_t m = (uint32_t)(n - 4) / 4 + 1; m; m--, i += 4) {
            const float *const e0 = list[i], *const e1 = list[i + 1], *const e2 = list[i + 2], *const e3 = list[i + 3];
            x = x + e0[4];
            long double yt = (long double)y + e0[5];
            z = (float)((long double)z + e0[6]);
            uvw(e0);
            xf = (float)(x + e1[4]);
            yt = yt + e1[5];
            const long double zt = (long double)z + e1[6];
            uvw(e1);
            xf = (float)((long double)xf + e2[4]);
            y = (float)(yt + e2[5]);
            z = (float)(zt + e2[6]);
            uvw(e2);
            x = (long double)xf + e3[4];
            y = (float)((long double)y + e3[5]);
            z = (float)((long double)z + e3[6]);
            ureg = (long double)u + e3[8];
            u = (float)ureg;
            v = (float)((long double)v + e3[9]);
            w = (float)((long double)w + e3[10]);
        }
    }
    for (; i < n; i++) {
        const float *const e = list[i];
        x = x + e[4];
        y = (float)((long double)y + e[5]);
        z = (float)((long double)z + e[6]);
        ureg = ureg + e[8];
        v = (float)((long double)v + e[9]);
        w = (float)((long double)w + e[10]);
    }
    const long double dx = x - ureg;
    volatile float dy = (float)((long double)y - v), dz = (float)((long double)z - w);
    volatile float s = (float)((long double)*(const float *)0xf264dc / ((long double)n * *(const float *)0xf8f954));
    volatile float *const o = out;
    o[0] = (float)(dx * s);
    o[1] = (float)((long double)dy * s);
    o[2] = (float)((long double)dz * s);
    return out;
}
static const uint8_t animal_point_a6ed40_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x34, 0xd9, 0x05, 0xe0, 0x64, 0xf2, 0x00};
REWRITE("Animal::PointA6ED40", 0xa6ed40, animal_point_a6ed40_original, &animal_point_a6ed40, &g_animal_means);

// ---- Model::PartsCC66B0() (0xCC66B0): puts each part's local matrix in front of its matrix, once (+8) -------------------
// For parts 1.. of the model (*self: count +0x24, local matrices +0x34) the part's matrix (64 bytes at +4) becomes local
// x matrix (affine: the last column 0, 0, 0, 1), each entry summed in the game's order. Then +8 is set.
static volatile long g_model_part_matrices;
static void __thiscall model_parts_cc66b0(uint8_t *self) {
    g_model_part_matrices++;
    if (self[8]) return;
    const uint8_t *const model = *(const uint8_t *const *)self;
    for (int32_t i = 1; i < *(const int32_t *)(model + 0x24); i++) {
        float *const P = (float *)(*(uint8_t *const *)(self + 4) + i * 0x40);
        const float *const L = (const float *)(*(const uint8_t *const *)(model + 0x34) + i * 0x40);
        auto p = [&](int r, int c) { return (long double)P[r * 4 + c]; };
        auto l = [&](int r, int c) { return (long double)L[r * 4 + c]; };
        volatile float r00 = (float)((l(0, 1) * p(1, 0) + p(2, 0) * l(0, 2)) + l(0, 0) * p(0, 0));
        volatile float r01 = (float)((l(0, 0) * p(0, 1) + l(0, 2) * p(2, 1)) + p(1, 1) * l(0, 1));
        volatile float r02 = (float)((l(0, 0) * p(0, 2) + l(0, 2) * p(2, 2)) + p(1, 2) * l(0, 1));
        volatile float r10 = (float)((p(0, 0) * l(1, 0) + p(2, 0) * l(1, 2)) + l(1, 1) * p(1, 0));
        volatile float r11 = (float)((l(1, 2) * p(2, 1) + l(1, 0) * p(0, 1)) + l(1, 1) * p(1, 1));
        volatile float r12 = (float)((p(2, 2) * l(1, 2) + l(1, 0) * p(0, 2)) + l(1, 1) * p(1, 2));
        volatile float r20 = (float)((p(0, 0) * l(2, 0) + p(2, 0) * l(2, 2)) + l(2, 1) * p(1, 0));
        volatile float r21 = (float)((l(2, 2) * p(2, 1) + l(2, 0) * p(0, 1)) + l(2, 1) * p(1, 1));
        volatile float r22 = (float)((l(2, 2) * p(2, 2) + l(2, 0) * p(0, 2)) + l(2, 1) * p(1, 2));
        volatile float r30 = (float)(((p(0, 0) * l(3, 0) + p(2, 0) * l(3, 2)) + l(3, 1) * p(1, 0)) + p(3, 0));
        volatile float r31 = (float)(((l(3, 2) * p(2, 1) + l(3, 0) * p(0, 1)) + l(3, 1) * p(1, 1)) + p(3, 1));
        volatile float r32 = (float)(((l(3, 2) * p(2, 2) + l(3, 0) * p(0, 2)) + l(3, 1) * p(1, 2)) + p(3, 2));
        volatile float *const o = P;
        o[0] = r00, o[1] = r01, o[2] = r02, ((volatile uint32_t *)o)[3] = 0;
        o[4] = r10, o[5] = r11, o[6] = r12, ((volatile uint32_t *)o)[7] = 0;
        o[8] = r20, o[9] = r21, o[10] = r22, ((volatile uint32_t *)o)[11] = 0;
        o[12] = r30, o[13] = r31, o[14] = r32, ((volatile uint32_t *)o)[15] = 0x3f800000;
    }
    self[8] = 1;
}
static const uint8_t model_parts_cc66b0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x44, 0x8a, 0x41, 0x08};
REWRITE("Model::PartsCC66B0", 0xcc66b0, model_parts_cc66b0_original, &model_parts_cc66b0, &g_model_part_matrices);

// ---- Model::PartsCC6420(out) (0xCC6420): the parts' matrices with their local matrices in front, into out -------------
// out[0] = part 0's matrix (64 bytes at +4). Once done (+8): the other parts' matrices copied; else for parts 1.. of the
// model (*self: count +0x24, local matrices +0x34) out[i] = local x matrix (affine: last column 0, 0, 0, 1), each entry
// summed in the game's order.
static volatile long g_model_part_copies;
static void __thiscall model_parts_cc6420(const uint8_t *self, float *out) {
    g_model_part_copies++;
    const float *const mats = *(const float *const *)(self + 4);
    for (int k = 0; k < 16; k++) ((volatile uint32_t *)out)[k] = ((const uint32_t *)mats)[k];
    const uint8_t *const model = *(const uint8_t *const *)self;
    if (self[8]) {
        for (int32_t i = 1; i < *(const int32_t *)(model + 0x24); i++)
            for (int k = 0; k < 16; k++) ((volatile uint32_t *)out)[i * 16 + k] = ((const uint32_t *)(*(const float *const *)(self + 4)))[i * 16 + k];
        return;
    }
    for (int32_t i = 1; i < *(const int32_t *)(model + 0x24); i++) {
        volatile float *const o = out + i * 16;
        ((volatile uint32_t *)o)[15] = 0x3f800000;
        ((volatile uint32_t *)o)[3] = 0, ((volatile uint32_t *)o)[7] = 0, ((volatile uint32_t *)o)[11] = 0;
        const float *const L = (const float *)(*(const uint8_t *const *)(model + 0x34) + i * 0x40);
        const float *const P = *(const float *const *)(self + 4) + i * 16;
        auto p = [&](int r, int c) { return (long double)P[r * 4 + c]; };
        auto l = [&](int r, int c) { return (long double)L[r * 4 + c]; };
        o[0] = (float)((l(0, 2) * p(2, 0) + p(0, 0) * l(0, 0)) + l(0, 1) * p(1, 0));
        o[1] = (float)((p(1, 1) * l(0, 1) + l(0, 2) * p(2, 1)) + p(0, 1) * l(0, 0));
        o[2] = (float)((p(2, 2) * l(0, 2) + p(1, 2) * l(0, 1)) + l(0, 0) * p(0, 2));
        o[4] = (float)((p(0, 0) * l(1, 0) + l(1, 1) * p(1, 0)) + p(2, 0) * l(1, 2));
        o[5] = (float)((l(1, 2) * p(2, 1) + l(1, 0) * p(0, 1)) + l(1, 1) * p(1, 1));
        o[6] = (float)((l(1, 0) * p(0, 2) + p(2, 2) * l(1, 2)) + l(1, 1) * p(1, 2));
        o[8] = (float)((p(0, 0) * l(2, 0) + l(2, 2) * p(2, 0)) + l(2, 1) * p(1, 0));
        o[9] = (float)((p(0, 1) * l(2, 0) + l(2, 2) * p(2, 1)) + p(1, 1) * l(2, 1));
        o[10] = (float)((l(2, 1) * p(1, 2) + l(2, 2) * p(2, 2)) + l(2, 0) * p(0, 2));
        o[12] = (float)(((p(0, 0) * l(3, 0) + l(3, 1) * p(1, 0)) + p(2, 0) * l(3, 2)) + p(3, 0));
        o[13] = (float)(((l(3, 2) * p(2, 1) + l(3, 0) * p(0, 1)) + l(3, 1) * p(1, 1)) + p(3, 1));
        o[14] = (float)(((l(3, 0) * p(0, 2) + p(2, 2) * l(3, 2)) + l(3, 1) * p(1, 2)) + p(3, 2));
    }
}
static const uint8_t model_parts_cc6420_original[] = {0x53, 0x56, 0x57, 0x8b, 0x51, 0x04, 0x8b, 0x44, 0x24, 0x10, 0x8b, 0x32};
REWRITE("Model::PartsCC6420", 0xcc6420, model_parts_cc6420_original, &model_parts_cc6420, &g_model_part_copies);

// ---- Terrain::Height8BC4A0(x, z, out) (0x8BC4A0): the ground's up direction under a point --------------------------------
// u = (x - +0xC) / +0x14, v = (z - +0x10) / +0x18 (floats) and their floors; outside the grid (bytes +4, +5) gives (0, 1,
// 0). In the cell (24 bytes at +0x24, corners h0-h3): past the diagonal (du + dv above 1) the far triangle with a = h2 -
// h3, b = h1 - h3, X = -+0x14, Z = -+0x18: n = (b 0 - Z a, Z X, a 0 - b X); else a = h2 - h0, b = h1 - h0, X = +0x14, Z =
// +0x18: n = (a 0 - Z b, Z X, b 0 - a X). n (x, y as floats) made of length 1 ((z z + y y) + x x); a length of 0 leaves
// (x, y, 1). Roundings as the game's.
static volatile long g_terrain_normals;
static void __thiscall terrain_height_8bc4a0(const uint8_t *self, float x, float z, float *out) {
    g_terrain_normals++;
    auto m = [&](uint32_t at) { return (long double)*(const float *)(self + at); };
    const long double zero = *(const float *)0xf264e0, one = *(const float *)0xf264dc;
    volatile float u = (float)(((long double)x - m(0xc)) / m(0x14));
    volatile float v = (float)(((long double)z - m(0x10)) / m(0x18));
    volatile float fu = (float)floor_h2(u), fv = (float)floor_h2(v);
    volatile float *const o = out;
    if (fu < zero || (long double)self[4] <= fu || fv < zero || (long double)self[5] <= fv) {
        ((volatile uint32_t *)o)[0] = 0, ((volatile uint32_t *)o)[1] = 0x3f800000, ((volatile uint32_t *)o)[2] = 0;
        return;
    }
    const int32_t j = (int32_t)x87_fistp64_h2(fv), i = (int32_t)x87_fistp64_h2(fu);
    const float *const h = (const float *)(*(const uint8_t *const *)(self + 0x24) + (j * (int32_t)self[4] + i) * 24);
    const long double du = (long double)u - fu, dv = (long double)v - fv;
    long double nz;
    volatile float nx, ny;
    if (dv + du > one) {
        const long double X = -m(0x14), a = (long double)h[2] - h[3], b = (long double)h[1] - h[3], Z = -m(0x18);
        nx = (float)(b * zero - Z * a);
        ny = (float)(Z * X);
        nz = a * zero - b * X;
    } else {
        const long double X = m(0x14), b = (long double)h[1] - h[0], a = (long double)h[2] - h[0], Z = m(0x18);
        nx = (float)(a * zero - Z * b);
        ny = (float)(Z * X);
        nz = b * zero - a * X;
    }
    o[1] = ny;
    o[0] = nx;
    o[2] = (float)nz;
    const long double len = x87_sqrt_h2((nz * nz + (long double)ny * ny) + (long double)nx * nx);
    if (len == zero) {
        ((volatile uint32_t *)o)[2] = 0x3f800000;
        return;
    }
    const long double inv = one / len;
    o[0] = (float)((long double)nx * inv);
    o[1] = (float)((long double)ny * inv);
    o[2] = (float)(nz * inv);
}
static const uint8_t terrain_height_8bc4a0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x20, 0xd9, 0x45, 0x08};
REWRITE("Terrain::Height8BC4A0", 0x8bc4a0, terrain_height_8bc4a0_original, &terrain_height_8bc4a0, &g_terrain_normals);

// ---- Vector::Toward466660(out in ecx, eye in edx, target, up) (0x466660): a view matrix looking from eye to target -----
// f = target - eye (floats) made of length 1 (x, y floats; z unrounded); s = f x up made of length 1 (y a float); u = s x
// f made of length 1 (its length a float). Too short an f or s (squared length below 1e-8) gives the identity. Columns
// (s, u, f) in the first three rows and -(eye . s), -(eye . u), -(eye . f), 1 in the last. Roundings as the game's.
static volatile long g_view_matrices;
static void __fastcall vector_toward_466660(float *out, const float *eye, const float *target, const float *up) {
    g_view_matrices++;
    const long double zero = *(const float *)0xf264e0, one = *(const float *)0xf264dc, tiny = *(const float *)0xf2a9bc;
    volatile float *const o = out;
    auto identity = [&] {
        for (int k = 0; k < 16; k++) ((volatile uint32_t *)o)[k] = (k % 5 == 0) ? 0x3f800000 : 0;
    };
    volatile float f0 = (float)((long double)target[0] - eye[0]), f1 = (float)((long double)target[1] - eye[1]);
    const long double f2u = (long double)target[2] - eye[2];
    volatile float f2 = (float)f2u;
    const long double d2 = (f2u * f2 + (long double)f1 * f1) + (long double)f0 * f0;
    if (d2 < tiny) return identity();
    const long double len = x87_sqrt_h2(d2);
    long double fz;
    if (len == zero) fz = one;
    else {
        const long double inv = one / len;
        f0 = (float)((long double)f0 * inv);
        f1 = (float)((long double)f1 * inv);
        fz = inv * f2;
    }
    const long double s0 = fz * up[1] - (long double)f1 * up[2];
    volatile float m1 = (float)((long double)f0 * up[2] - fz * up[0]);
    const long double s2u = (long double)f1 * up[0] - (long double)f0 * up[1];
    volatile float m2 = (float)s2u;
    const long double q = (s2u * m2 + (long double)m1 * m1) + s0 * s0;
    if (!(q >= tiny || __builtin_isunordered(q, tiny))) return identity();
    const long double sl = x87_sqrt_h2(q);
    long double S0, S2;
    if (sl == zero) S2 = one, S0 = s0;
    else {
        const long double inv2 = one / sl;
        S0 = s0 * inv2;
        m1 = (float)((long double)m1 * inv2);
        S2 = inv2 * m2;
    }
    long double U0 = (long double)f1 * S2 - (long double)m1 * fz;
    long double U1 = fz * S0 - S2 * f0;
    long double U2 = (long double)m1 * f0 - (long double)f1 * S0;
    volatile float ulen = (float)x87_sqrt_h2((U2 * U2 + U1 * U1) + U0 * U0);
    if (ulen == zero) U2 = one;
    else {
        const long double iu = one / ulen;
        U0 = U0 * iu;
        U1 = U1 * iu;
        U2 = U2 * iu;
    }
    o[0] = (float)S0, o[1] = (float)U0, o[2] = f0, ((volatile uint32_t *)o)[3] = 0;
    o[4] = m1, o[5] = (float)U1, o[6] = f1, ((volatile uint32_t *)o)[7] = 0;
    o[8] = (float)S2, o[9] = (float)U2, o[10] = (float)fz, ((volatile uint32_t *)o)[11] = 0;
    o[12] = (float)-((S0 * eye[0] + (long double)m1 * eye[1]) + S2 * eye[2]);
    o[13] = (float)-((U0 * eye[0] + U1 * eye[1]) + U2 * eye[2]);
    o[14] = (float)-(((long double)f0 * eye[0] + (long double)f1 * eye[1]) + fz * eye[2]);
    ((volatile uint32_t *)o)[15] = 0x3f800000;
}
static const uint8_t vector_toward_466660_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x2c, 0x8b, 0x45, 0x08};
REWRITE("Vector::Toward466660", 0x466660, vector_toward_466660_original, &vector_toward_466660, &g_view_matrices);

// ---- Particles::TimeCE1720(d) (0xCE1720): clips the game's shape to the side of a plane --------------------------------
// The shape: points (16 bytes) at 0x1400810, count 0x140081C, written to 0x1400814. Keeps what has n . p ((z + y) + x;
// n is self) below -d: inside points are kept, and where an edge crosses, the point at t = (dp - D) / (dp - dc) (or (D -
// dp) / (dc - dp) going out; dp the last value as a float) along it (x, y as floats). Then the count is the new one, the
// written list becomes the shape and the other of the two lists (0x1400830, 0x1400970) the next to write.
static volatile long g_shape_clips;
static void __thiscall particles_time_ce1720(const float *n, float d) {
    g_shape_clips++;
    volatile uint32_t *const g = (volatile uint32_t *)0x1400810;
    volatile float big_d = (float)((long double)d * *(const float *)0xf268f0);
    auto dot = [&](const float *p) { return ((long double)p[2] * n[2] + (long double)p[1] * n[1]) + (long double)p[0] * n[0]; };
    const float *const last = (const float *)(uintptr_t)g[0] + ((int32_t)g[3] - 1) * 4;
    volatile float px = last[0], py = last[1], pz = last[2];
    const long double dp0 = dot(last);
    volatile float dpf = (float)dp0;
    bool prev_in = dp0 < big_d;
    int32_t out = 0;
    auto cut = [&](const float *c, float *o, long double t) {
        volatile float dx = (float)((long double)c[0] - px);
        const long double dy = (long double)c[1] - py;
        volatile float dz = (float)((long double)c[2] - pz);
        volatile float mx = (float)((long double)dx * t), my = (float)(dy * t);
        const long double mz = t * dz;
        volatile float *const v = o;
        v[2] = (float)(mz + pz);
        v[0] = (float)((long double)mx + px);
        v[1] = (float)((long double)my + py);
    };
    auto copy = [&](const float *c, float *o) {
        for (int k = 0; k < 3; k++) ((volatile uint32_t *)o)[k] = ((const uint32_t *)c)[k];
    };
    for (int32_t i = 0; i < (int32_t)g[3]; i++) {
        const float *const c = (const float *)(uintptr_t)g[0] + i * 4;
        const long double dc = dot(c);
        const bool cur_in = dc < big_d;
        if (cur_in) {
            if (prev_in) copy(c, (float *)(uintptr_t)g[1] + out * 4), out++;
            else {
                cut(c, (float *)(uintptr_t)g[1] + out * 4, ((long double)dpf - big_d) / ((long double)dpf - dc));
                out++;
                copy(c, (float *)(uintptr_t)g[1] + out * 4), out++;
            }
        } else if (prev_in) {
            cut(c, (float *)(uintptr_t)g[1] + out * 4, ((long double)big_d - dpf) / (dc - dpf));
            out++;
        }
        px = c[0], py = c[1], pz = c[2];
        prev_in = cur_in;
        dpf = (float)dc;
    }
    const uint32_t written = g[1];
    g[3] = (uint32_t)out;
    g[0] = written;
    g[1] = written == 0x1400830 ? 0x1400970 : 0x1400830;
}
static const uint8_t particles_time_ce1720_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x44, 0x53, 0x56, 0x57};
REWRITE("Particles::TimeCE1720", 0xce1720, particles_time_ce1720_original, &particles_time_ce1720, &g_shape_clips);

// ---- Model::BuildBC9C70(mesh, out, matrices, uv) (0xBC9C70, cdecl): moves a mesh's vertices by their groups' matrices --
// mesh: group count +0, vertex counts per group +8, vertices (32 bytes: place, normal, uv) +0x18. Each group's vertices
// go through its matrix (64 bytes): place with the last row added, normal without; uv + the uv offset. With the game's
// SSE switch (0x13D0B14) in four-wide single sums ((x m0 + y m1) + z m2) + m3; else on the FPU in the game's order
// (normals adding the last row times 0).
static volatile long g_mesh_builds;
__attribute__((target("sse"))) static void mesh_build_sse_h2(const float *v, float *o, int32_t count, const float *m) {
    const __m128 r0 = _mm_set_ps(0, m[2], m[1], m[0]), r1 = _mm_set_ps(0, m[6], m[5], m[4]);
    const __m128 r2 = _mm_set_ps(0, m[10], m[9], m[8]), r3 = _mm_set_ps(0, m[14], m[13], m[12]);
    alignas(16) float p[4], n[4];
    _mm_store_ps(p, _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_set1_ps(v[0]), r0), _mm_mul_ps(_mm_set1_ps(v[1]), r1)), _mm_mul_ps(_mm_set1_ps(v[2]), r2)), r3));
    _mm_store_ps(n, _mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_set1_ps(v[3]), r0), _mm_mul_ps(_mm_set1_ps(v[4]), r1)), _mm_mul_ps(_mm_set1_ps(v[5]), r2)));
    for (int k = 0; k < 3; k++) ((volatile uint32_t *)o)[k] = ((const uint32_t *)p)[k], ((volatile uint32_t *)o)[3 + k] = ((const uint32_t *)n)[k];
    (void)count;
}
static void mesh_build_bc9c70(const uint8_t *mesh, float *out, const float *matrices, const float *uv) {
    g_mesh_builds++;
    const int32_t groups = *(const int32_t *)mesh;
    const int32_t *const counts = *(const int32_t *const *)(mesh + 8);
    const float *const src = *(const float *const *)(mesh + 0x18);
    if (*(const volatile uint8_t *)0x13d0b14) {
        const float *v = src;
        float *o = out;
        int32_t g = 0;
        do {
            for (uint32_t c = (uint32_t)counts[g]; c; c--, v += 8, o += 8) {
                mesh_build_sse_h2(v, o, 0, matrices + g * 16);
                volatile float *const vo = o;
                vo[7] = (float)((long double)v[7] + uv[1]);
                vo[6] = (float)((long double)v[6] + uv[0]);
            }
            g++;
        } while (g != groups);
        return;
    }
    const long double zero = *(const float *)0xf264e0;
    int32_t vi = 0;
    for (int32_t g = 0; g < *(const int32_t *)mesh; g++) {
        const int32_t c = (*(const int32_t *const *)(mesh + 8))[g];
        if (c <= 0) continue;
        const float *const m = matrices + g * 16;
        auto mm = [&](int r, int k) { return (long double)m[r * 4 + k]; };
        for (int32_t j = 0; j < c; j++, vi++) {
            const float *const v = src + vi * 8;
            volatile float *const o = out + vi * 8;
            const long double p0 = v[0], p1 = v[1], p2 = v[2];
            const long double x = ((p2 * mm(2, 0) + p0 * mm(0, 0)) + p1 * mm(1, 0)) + mm(3, 0);
            const long double y = ((p2 * mm(2, 1) + p0 * mm(0, 1)) + p1 * mm(1, 1)) + mm(3, 1);
            volatile float z = (float)(((p1 * mm(1, 2) + p2 * mm(2, 2)) + p0 * mm(0, 2)) + mm(3, 2));
            o[0] = (float)x, o[1] = (float)y, o[2] = z;
            const long double n0 = v[3], n1 = v[4], n2 = v[5];
            const long double nx = ((n0 * mm(0, 0) + n2 * mm(2, 0)) + n1 * mm(1, 0)) + mm(3, 0) * zero;
            const long double ny = ((n2 * mm(2, 1) + n0 * mm(0, 1)) + n1 * mm(1, 1)) + mm(3, 1) * zero;
            volatile float nz = (float)(((n1 * mm(1, 2) + n2 * mm(2, 2)) + n0 * mm(0, 2)) + mm(3, 2) * zero);
            o[3] = (float)nx, o[4] = (float)ny, *(volatile uint32_t *)&o[5] = *(const volatile uint32_t *)&nz;
            o[6] = (float)((long double)v[6] + uv[0]);
            o[7] = (float)((long double)v[7] + uv[1]);
        }
    }
}
static const uint8_t mesh_build_bc9c70_original[] = {0x53, 0x8b, 0xdc, 0x83, 0xec, 0x08, 0x83, 0xe4, 0xf0, 0x83, 0xc4, 0x04};
REWRITE("Model::BuildBC9C70", 0xbc9c70, mesh_build_bc9c70_original, &mesh_build_bc9c70, &g_mesh_builds);

// ---- Water::Colours908940(out) (0x908940): the water surface's vertices (36 bytes: place, normal, colour, uv) -----------
// n = +0 points a side over the box (+8, +0xC)-(+0x10, +0x14), heights +0x40 (n x n) times +0x2C (at least 0.5) plus
// +0x18. n of 1 or less: one vertex at the middle with normal (0, 1, 0), made in the template at +0x58 and copied. Else
// with q = (+0x10 - +0x8) / n K / +0x2C, m = +0x24 / 2, the box shrunk by m (corners and sizes floats): vertex (i, j) at
// (A + i / (n - 1) Wx, height, B + j / (n - 1) Wz) with normal (dx, q, dz) made of length 1 (dx, dz the height
// differences of the neighbours; (dx, q, 1) when all are 0), colour 0xFF0C0C0C, uv = (x, z). Gives the end of out.
static volatile long g_water_surfaces;
static float *__thiscall water_colours_908940(const uint8_t *self, float *out) {
    g_water_surfaces++;
    auto m = [&](uint32_t at) { return (long double)*(const float *)(self + at); };
    const long double half = *(const float *)0xf2651c, one = *(const float *)0xf264dc, zero = *(const float *)0xf264e0;
    const float *const h = *(const float *const *)(self + 0x40);
    auto height = [&](int32_t at) {
        long double y = (long double)h[at] * m(0x2c);
        if (y < half) y = half;
        return y + m(0x18);
    };
    const int32_t n = *(const int32_t *)self;
    if (n <= 1) {
        const long double cx = (m(0x10) + m(0x8)) * half, cz = (m(0x14) + m(0xc)) * half;
        const long double y = height(0);
        volatile float *const t = *(float *const *)(self + 0x58);
        ((volatile uint32_t *)t)[6] = 0xff0c0c0c, ((volatile uint32_t *)t)[4] = 0x3f800000;
        t[0] = (float)cx, ((volatile uint32_t *)t)[3] = 0, t[1] = (float)y, ((volatile uint32_t *)t)[5] = 0;
        t[2] = (float)cz, t[7] = (float)cx, t[8] = (float)cz;
        volatile float *const o = out;
        o[0] = (float)cx;
        for (int k = 1; k < 9; k++) ((volatile uint32_t *)o)[k] = ((volatile uint32_t *)t)[k];
        return out + 9;
    }
    volatile float q = (float)((m(0x10) - m(0x8)) / (long double)n * *(const float *)0xf26610 / m(0x2c));
    const long double mg = m(0x24) * half;
    volatile float a = (float)(mg + m(0x8)), wx = (float)((m(0x10) - mg) - a);
    volatile float b = (float)(mg + m(0xc)), wz = (float)((m(0x14) - mg) - b);
    float *o = out;
    for (int32_t j = 0; j < *(const int32_t *)self; j++) {
        const int32_t last = *(const int32_t *)self - 1;
        const int32_t jm = j > 0 ? j - 1 : 0, jp = j < last ? j + 1 : last;
        volatile float z = (float)((long double)j / (long double)last * wz + b);
        const long double q2 = (long double)q * q;
        for (int32_t i = 0; i < *(const int32_t *)self; i++, o += 9) {
            const int32_t w = *(const int32_t *)self, lst = w - 1;
            const int32_t im = i > 0 ? i - 1 : 0, ip = i < lst ? i + 1 : lst;
            const long double dx = (long double)h[w * j + im] - h[w * j + ip];
            const long double dz = (long double)h[w * jm + i] - h[w * jp + i];
            const long double len = x87_sqrt_h2((dz * dz + dx * dx) + q2);
            long double nx, nz;
            volatile float qn = q;
            if (len == zero) nz = one, nx = dx;
            else {
                const long double inv = one / len;
                nx = dx * inv;
                qn = (float)((long double)q * inv);
                nz = dz * inv;
            }
            const long double y = height(w * j + i);
            volatile float x = (float)((long double)i / (long double)lst * wx + a);
            volatile float *const v = o;
            *(volatile uint32_t *)&v[2] = *(const volatile uint32_t *)&z;
            *(volatile uint32_t *)&v[0] = *(const volatile uint32_t *)&x;
            v[1] = (float)y;
            *(volatile uint32_t *)&v[4] = *(const volatile uint32_t *)&qn;
            v[3] = (float)nx;
            v[5] = (float)nz;
            *(volatile uint32_t *)&v[7] = *(const volatile uint32_t *)&x;
            ((volatile uint32_t *)v)[6] = 0xff0c0c0c;
            *(volatile uint32_t *)&v[8] = *(const volatile uint32_t *)&z;
        }
    }
    return o;
}
static const uint8_t water_colours_908940_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x44, 0x8b, 0x01, 0xd9, 0x41, 0x10};
REWRITE("Water::Colours908940", 0x908940, water_colours_908940_original, &water_colours_908940, &g_water_surfaces);

// ---- Grid::GrowChanged(x0, y0, x1, y1) (0x703170): grows the changed area (+0x74..+0x80) to take in a rectangle ----------
// The rectangle is kept inside the grid (0 to +0x6C, +0x70); an empty one changes nothing.
static volatile long g_grid_changes;
static void __thiscall grid_grow_changed(uint8_t *self, int32_t x0, int32_t y0, int32_t x1, int32_t y1) {
    g_grid_changes++;
    int32_t *const c = (int32_t *)(self + 0x74);
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > *(const int32_t *)(self + 0x6c)) x1 = *(const int32_t *)(self + 0x6c);
    if (y1 > *(const int32_t *)(self + 0x70)) y1 = *(const int32_t *)(self + 0x70);
    if (x0 >= x1 || y0 >= y1) return;
    if (x0 < c[0]) c[0] = x0;
    if (y0 < c[1]) c[1] = y0;
    if (x1 > c[2]) c[2] = x1;
    if (y1 > c[3]) c[3] = y1;
}
static const uint8_t grid_grow_changed_original[] = {0x53, 0x56, 0x57, 0x8b, 0x7c, 0x24, 0x10, 0x85, 0xff};
REWRITE("Grid::GrowChanged", 0x703170, grid_grow_changed_original, &grid_grow_changed, &g_grid_changes);

// ---- Key::Matches(key) (0x7E5050): whether an entry matches a search key ----------------------------------------------
// By the entry's kind bits (+0xF & 6): 4 and 6 never (only al); 0 matches keys of kind (+8) 1, 3 or 4 by its two bytes
// (key +4, +5) and, when the key's +0xA is not negative, its +0xE; 2 matches keys of other kinds by its whole first
// dword (key +4).
static volatile long g_search_key_matches;
static uint8_t __thiscall key_matches(const uint8_t *e, const uint8_t *key) {
    g_search_key_matches++;
    const uint8_t kind = e[0xf] & 6;
    if (kind == 4 || kind == 6) return 0;
    const uint8_t t = key[8];
    const bool named = t == 1 || t == 3 || t == 4;
    if (!kind) return named && e[0] == key[4] && e[1] == key[5] && ((int8_t)key[0xa] < 0 || e[0xe] == key[0xa]);
    return !named && *(const uint32_t *)e == *(const uint32_t *)(key + 4);
}
static const uint8_t key_matches_original[] = {0x8a, 0x41, 0x0f, 0x24, 0x06, 0x3c, 0x04, 0x74, 0x04};
REWRITE("Key::Matches", 0x7e5050, key_matches_original, &key_matches, &g_search_key_matches);

// ---- Window::Match9DAC40(a, b) (0x9DAC40): whether any of the four parts (+0x18..+0x24) matches a and b -------------------
// A part matches when a is 3 or its +0x54 is a, and b is 3 or its +0x58 is b. 1 or 0 (only al).
static volatile long g_window_part_matches;
static uint8_t __thiscall window_match_9dac40(const uint8_t *self, int32_t a, int32_t b) {
    g_window_part_matches++;
    uint8_t found = 0;
    for (int k = 0; k < 4; k++) {
        const uint8_t *const p = *(const uint8_t *const *)(self + 0x18 + k * 4);
        if (p && (a == 3 || *(const int32_t *)(p + 0x54) == a) && (b == 3 || *(const int32_t *)(p + 0x58) == b)) found = 1;
    }
    return found;
}
static const uint8_t window_match_9dac40_original[] = {0x8b, 0x51, 0x18, 0x56, 0x8b, 0x74, 0x24, 0x08, 0x32, 0xc0};
REWRITE("Window::Match9DAC40", 0x9dac40, window_match_9dac40_original, &window_match_9dac40, &g_window_part_matches);

// ---- Window::Value6F13C0() (0x6F13C0): how much of a window's chosen options are on, 0 to 1 -------------------------------
// By the flags (+0xD0): 0x80 counts +0x10C, 4 counts +0xE4, 8 counts +0xF0 (bytes, on when set); 0x200 counts the last
// entry's (+0x114, count +0x118, 20 bytes each; its -0x10's +0xC) value; 0x10 counts +0xF4 with min(+0x100, it) on; 0x400
// counts +0x120 with min(+0x12C, it) on. 1 when all are on (or none counted), else on / counted.
static volatile long g_window_shares;
static long double __thiscall window_value_6f13c0(const uint8_t *self) {
    g_window_shares++;
    const uint32_t f = *(const uint32_t *)(self + 0xd0);
    int32_t total = 0, on = 0;
    if (f & 0x80) total = 1, on = self[0x10c] ? 1 : 0;
    if (f & 4) total++, on += self[0xe4] ? 1 : 0;
    if (f & 8) total++, on += self[0xf0] ? 1 : 0;
    if (f & 0x200) {
        total++;
        const uint8_t *const e = *(const uint8_t *const *)(*(const uint8_t *const *)(self + 0x114) + *(const int32_t *)(self + 0x118) * 20 - 0x10);
        if (*(const uint32_t *)(e + 0xc)) on++;
    }
    auto part = [&](uint32_t all, uint32_t some) {
        const int32_t a = *(const int32_t *)(self + all), s = *(const int32_t *)(self + some);
        total += a, on += s <= a ? s : a;
    };
    if (f & 0x10) part(0xf4, 0x100);
    if (f & 0x400) part(0x120, 0x12c);
    if (on == total) return *(const float *)0xf264dc;
    return (long double)on / (long double)total;
}
static const uint8_t window_value_6f13c0_original[] = {0x83, 0xec, 0x08, 0x53, 0x8b, 0x99, 0xd0, 0x00, 0x00, 0x00};
REWRITE("Window::Value6F13C0", 0x6f13c0, window_value_6f13c0_original, &window_value_6f13c0, &g_window_shares);

// ---- Fireworks::List9F4C80() (0x9F4C80): stretches each piece's direction to reach the next piece -----------------------
// For each piece of the +0x4C list (node list, item +8) and the one after it: d = the gap between their places (+0x10;
// length (z z + y y) + x x); the direction (+0x30) times d over 3 times its own length ((x x + y y) + z z), x by the
// unrounded scale, y and z by it as a float.
static volatile long g_firework_stretches;
static void __thiscall fireworks_list_9f4c80(const uint8_t *self) {
    g_firework_stretches++;
    const uint32_t *p = *(const uint32_t *const *)(self + 0x4c);
    if (!p[0] || !p) return;
    for (;;) {
        __asm__ volatile("" ::: "memory");
        const uint32_t *const q = (const uint32_t *)(uintptr_t)p[0];
        if (!q[0] || !q) return;
        volatile float *const a = (float *)(uintptr_t)p[2];
        const float *const b = (const float *)(uintptr_t)q[2];
        const long double dx = (long double)b[4] - a[4], dy = (long double)b[5] - a[5], dz = (long double)b[6] - a[6];
        const long double len = x87_sqrt_h2((dz * dz + dy * dy) + dx * dx);
        volatile float v0 = (float)(len * a[12]), v1 = (float)(len * a[13]);
        const long double v2 = len * a[14];
        const long double d0 = a[12], d1 = a[13], d2 = a[14];
        const long double own = x87_sqrt_h2((d0 * d0 + d1 * d1) + d2 * d2);
        const long double s = (long double)*(const float *)0xf264dc / (own * *(const float *)0xf2784c);
        volatile float sf = (float)s;
        const long double x = s * v0;
        volatile float y = (float)((long double)v1 * sf);
        a[14] = (float)(v2 * sf);
        a[12] = (float)x;
        a[13] = y;
        p = q;
    }
}
static const uint8_t fireworks_list_9f4c80_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x30, 0x8b, 0x41, 0x4c};
REWRITE("Fireworks::List9F4C80", 0x9f4c80, fireworks_list_9f4c80_original, &fireworks_list_9f4c80, &g_firework_stretches);

// ---- Window::Value6CC710() (0x6CC710): the total height of a window's rows ---------------------------------------------
// 0 with no rows (+0xE4). Else 1, then for each row (0x50 bytes at +0xE0): its first part's size (+0x88 - +0x80; not for
// row 0), its +8 part's size, 3, and the sizes of the parts at +0xC..+0x48 that are there; then 1 more.
static volatile long g_window_heights;
static long double __thiscall window_value_6cc710(const uint8_t *self) {
    g_window_heights++;
    const int32_t n = *(const int32_t *)(self + 0xe4);
    if (n <= 0) return *(const float *)0xf264e0;
    auto size = [](const uint8_t *p) { return (long double)*(const float *)(p + 0x88) - *(const float *)(p + 0x80); };
    long double acc = *(const float *)0xf264dc;
    for (int32_t i = 0; i < n; i++) {
        const uint8_t *const row = *(const uint8_t *const *)(self + 0xe0) + i * 0x50;
        if (i > 0) acc = acc + size(*(const uint8_t *const *)row);
        acc = acc + size(*(const uint8_t *const *)(row + 8));
        acc = acc + *(const float *)0xf2784c;
        for (int k = 0; k < 16; k++) {
            const uint8_t *const p = *(const uint8_t *const *)(row + 0xc + k * 4);
            if (p) acc = acc + size(p);
        }
    }
    return acc + *(const float *)0xf264dc;
}
static const uint8_t window_value_6cc710_original[] = {0xd9, 0x05, 0xe0, 0x64, 0xf2, 0x00, 0x55, 0x8b, 0xa9, 0xe4, 0x00, 0x00, 0x00};
REWRITE("Window::Value6CC710", 0x6cc710, window_value_6cc710_original, &window_value_6cc710, &g_window_heights);

// ---- Track::StepBC5090(t, u, c, a, b) (0xBC5090): finds the entry whose range holds t -----------------------------------
// Rides not flagged 0x400000 or with fewer than two stations (+0xE4): entries {a, b, c} (12 bytes at +0x74, count +0x70)
// with a <= t <= b. Others: entries {a, b, c, d, e} (20 bytes) with a <= t <= b, u >= d and e below 0 or u below e. The
// last such entry's c, a, b are written out; gives whether any was found (only al).
static volatile long g_track_range_finds;
static uint8_t __thiscall track_step_bc5090(const uint8_t *self, float t, float u, uint32_t *c, uint32_t *a, uint32_t *b) {
    g_track_range_finds++;
    uint8_t found = 0;
    const bool plain = !(*(const uint32_t *)(self + 0x18) & 0x400000) || *(const int32_t *)(self + 0xe4) < 2;
    for (int32_t i = 0; i < *(const int32_t *)(self + 0x70); i++) {
        const float *const e = (const float *)(*(const uint8_t *const *)(self + 0x74) + i * (plain ? 12 : 20));
        if (!(t >= e[0]) || !(t <= e[1])) continue;
        if (!plain) {
            if (!(u >= e[3])) continue;
            if (!(e[4] < *(const float *)0xf264e0) && !(u < e[4])) continue;
        }
        *c = ((const uint32_t *)e)[2], *a = ((const uint32_t *)e)[0], *b = ((const uint32_t *)e)[1];
        found = 1;
    }
    return found;
}
static const uint8_t track_step_bc5090_original[] = {0x51, 0x8b, 0x41, 0x18, 0x53, 0x55, 0x32, 0xdb};
REWRITE("Track::StepBC5090", 0xbc5090, track_step_bc5090_original, &track_step_bc5090, &g_track_range_finds);

// ---- Window::Count6CF4B0(ring, least, most) (0x6CF4B0): the smallest and largest values in a ring of floats ---------------
// least starts at 1e6 and most at -1e6. The ring (values +0, start +4, size +8, count +0xC) is gone over from the newest
// (index (start + i) % size, i from count - 1 down to 0); each value larger than most or smaller than least takes its
// place.
static volatile long g_ring_ranges;
static void __stdcall window_count_6cf4b0(const uint8_t *ring, uint32_t *least, uint32_t *most) {
    g_ring_ranges++;
    *least = 0x49742400, *most = 0xc9742400;
    const int32_t *const r = (const int32_t *)ring;
    for (int32_t i = r[3] - 1; i >= 0; i--) {
        const uint32_t *const v = (const uint32_t *)(uintptr_t)r[0] + (r[1] + i) % r[2];
        if (*(const float *)v > *(const float *)most) *most = *v;
        if (*(const float *)v < *(const float *)least) *least = *v;
    }
}
static const uint8_t window_count_6cf4b0_original[] = {0x8b, 0x4c, 0x24, 0x04, 0x53, 0x8b, 0x5c, 0x24, 0x10, 0x56, 0x57};
REWRITE("Window::Count6CF4B0", 0x6cf4b0, window_count_6cf4b0_original, &window_count_6cf4b0, &g_ring_ranges);

// ---- Fence::CountA90B50(shape, point) (0xA90B50): whether a point (x, z at +0, +8) is inside a flat shape ---------------
// The shape: n (+4) corners {x, z} (8 bytes at +0). Counts the sides crossing the point's line to its right: a corner on
// the line and right of the point counts when its neighbours lie on opposite sides; a side from one side to the other
// counts when it meets the line at ((cx - nx) pz - nz cx + nx cz) / (cz - nz) past the point. Gives the count's lowest
// bit.
static volatile long g_fence_insides;
static uint32_t __stdcall fence_count_a90b50(const uint32_t *shape, const float *p) {
    g_fence_insides++;
    const int32_t n = (int32_t)shape[1];
    const float *const pts = (const float *)(uintptr_t)shape[0];
    const float px = p[0], pz = p[2];
    uint32_t count = 0;
    for (int32_t i = 0; i < n; i++) {
        const float *const c = pts + 2 * i, *const nx = pts + 2 * (i + 1 < n ? i + 1 : 0), *const pv = pts + 2 * (i > 0 ? i - 1 : n - 1);
        const float cx = c[0], cz = c[1], ncx = nx[0], ncz = nx[1], pcz = pv[1];
        if (cz == pz) {
            if (!(cx > px)) continue;
            if (pcz < cz && ncz > cz) {
                count++;
                continue;
            }
            if (pcz > cz && ncz < cz) count++;
            continue;
        }
        if (!((cz < pz && ncz > pz) || (cz > pz && ncz < pz))) continue;
        const long double x = ((((long double)cx - ncx) * pz - (long double)ncz * cx) + (long double)ncx * cz) / ((long double)cz - ncz);
        if (x > px) count++;
    }
    return count & 1;
}
static const uint8_t fence_count_a90b50_original[] = {0x83, 0xec, 0x1c, 0x8b, 0x44, 0x24, 0x20, 0x53, 0x8b, 0x58, 0x04};
REWRITE("Fence::CountA90B50", 0xa90b50, fence_count_a90b50_original, &fence_count_a90b50, &g_fence_insides);

// ---- Window::Value67CDD0(t) (0x67CDD0): a curve's value at t (key times +0, values +8, count +0xC, looping +0x10) -------
// No keys gives 0, one gives its value. Not looping: before or at the first time the first value, at or past the last
// the last; else the two keys around t (a binary search) and (1 - f) a + f b with f = (t - ta) / (tb - ta). Looping: t
// brought into 0..1 (by one step), then outside the times the last key and the first (its time + 1, t + 1 when before
// the first) are the pair.
static volatile long g_curve_values;
static long double __thiscall window_value_67cdd0(const uint8_t *self, float tf) {
    g_curve_values++;
    const int32_t n = *(const int32_t *)(self + 0xc);
    const float *const k = *(const float *const *)self, *const v = *(const float *const *)(self + 8);
    const long double zero = *(const float *)0xf264e0, one = *(const float *)0xf264dc;
    if (n <= 0) return zero;
    if (n == 1) return v[0];
    auto search = [&](long double t) {
        int32_t lo = 0, hi = n - 1, at = 0;
        while (hi > lo + 1) {
            const int32_t mid = lo + (hi - lo) / 2;
            if (!(t >= k[mid])) hi = mid;
            else lo = mid, at = mid;
        }
        return at;
    };
    auto mix = [&](long double f, int32_t a, int32_t b) { return (one - f) * v[a] + f * v[b]; };
    if (!self[0x10]) {
        const long double t = tf;
        if (t <= k[0]) return v[0];
        if (t >= k[n - 1]) return v[n - 1];
        const int32_t a = search(t);
        return mix((t - k[a]) / ((long double)k[a + 1] - k[a]), a, a + 1);
    }
    long double t = tf;
    if (t < zero) t = t + one;
    else if (t >= one) t = t - one;
    const int32_t a = (t < k[0] || t > k[n - 1]) ? n - 1 : search(t);
    if (a + 1 < n) return mix((t - k[a]) / ((long double)k[a + 1] - k[a]), a, a + 1);
    const long double next = (long double)k[0] + one;
    if (t < k[a]) t = t + one;
    return mix((t - k[a]) / (next - k[a]), a, 0);
}
static const uint8_t window_value_67cdd0_original[] = {0x51, 0x8b, 0xd1, 0x8b, 0x4a, 0x0c, 0x85, 0xc9, 0x89, 0x14, 0x24};
REWRITE("Window::Value67CDD0", 0x67cdd0, window_value_67cdd0_original, &window_value_67cdd0, &g_curve_values);

// ---- Water::Cell908060(x0, x1, j, amount) (0x908060): moves water from row j to row j + 1 over columns x0..x1 ------------
// Grid width +0, values +0x44. Nothing for j outside -1..width or columns outside the grid; the columns kept inside. d =
// amount +0x38 (a float). Row j loses d when j lies in (flag 8 clear (+0x20): 1, else 0) .. (flag 4: width, else width -
// 1); row j + 1 gains d when j lies in (flag 8: -1, else 0) .. (flag 4: width - 1, else width - 2).
static volatile long g_water_row_moves;
static void __thiscall water_cell_908060(const uint8_t *self, int32_t x0, int32_t x1, int32_t j, float amount) {
    g_water_row_moves++;
    const int32_t w = *(const int32_t *)self;
    if (j < -1 || j > w || x1 < 0 || x0 >= w) return;
    if (x0 < 0) x0 = 0;
    if (x1 >= w) x1 = w - 1;
    volatile float d = (float)((long double)amount * *(const float *)(self + 0x38));
    const uint32_t f = *(const uint32_t *)(self + 0x20);
    const bool b2 = (f >> 2) & 1, b3 = (f >> 3) & 1;
    auto at = [&](int32_t row, int32_t i) { return *(float *const *)(self + 0x44) + *(const int32_t *)self * row + i; };
    if (j >= (b3 ? 0 : 1) && j <= (b2 ? w : w - 1))
        for (int32_t i = x0; i <= x1; i++) *at(j, i) = (float)((long double)*at(j, i) - d);
    if (j >= (b3 ? -1 : 0) && j <= (b2 ? w - 1 : w - 2))
        for (int32_t i = x0; i <= x1; i++) *at(j + 1, i) = (float)((long double)d + *at(j + 1, i));
}
static const uint8_t water_cell_908060_original[] = {0x56, 0x8b, 0x74, 0x24, 0x10, 0x83, 0xfe, 0xff, 0x0f, 0x8c};
REWRITE("Water::Cell908060", 0x908060, water_cell_908060_original, &water_cell_908060, &g_water_row_moves);

// ---- Window::Scroll9C1FD0(out) (0x9C1FD0): the largest width and height of the ten parts (+0x130..+0x154) -------------
// Each starts at 0 and takes a part's +0x84 (width) or +0x88 (height) when that is larger. Gives out.
static volatile long g_window_part_sizes;
static uint32_t *__thiscall window_scroll_9c1fd0(const uint8_t *self, uint32_t *out) {
    g_window_part_sizes++;
    uint32_t w = 0, h = 0;
    for (int k = 0; k < 10; k++) {
        const uint8_t *const p = *(const uint8_t *const *)(self + 0x130 + k * 4);
        if (*(const float *)(p + 0x84) > *(const float *)&w) w = *(const uint32_t *)(p + 0x84);
        if (*(const float *)&h < *(const float *)(p + 0x88)) h = *(const uint32_t *)(p + 0x88);
    }
    out[0] = w, out[1] = h;
    return out;
}
static const uint8_t window_scroll_9c1fd0_original[] = {0x83, 0xec, 0x08, 0x8b, 0x91, 0x30, 0x01, 0x00, 0x00};
REWRITE("Window::Scroll9C1FD0", 0x9c1fd0, window_scroll_9c1fd0_original, &window_scroll_9c1fd0, &g_window_part_sizes);

// ---- Scenery::Point884D20(p) (0x884D20): the first of the shape's points (+0x100, count +0xF8) near p --------------------
// p is taken into the shape's frame: u = (p.x - 4) / ((+0x84 - +0x7C) - 8) +0xDC with half width r = +0xDC / (that size)
// 3 (likewise y with +0x88, +0x80 and +0xF0); the box u - r .. u + r by v - r2 .. v + r2 (floats). Gives the first point
// {x, y} (8 bytes) inside it, or the count when none is.
static volatile long g_scenery_point_finds;
static int32_t __thiscall scenery_point_884d20(const uint8_t *self, const float *p) {
    g_scenery_point_finds++;
    auto m = [&](uint32_t at) { return (long double)*(const float *)(self + at); };
    const long double four = *(const float *)0xf793c8, eight = *(const float *)0xf270e8, three = *(const float *)0xf793c4;
    const long double u = ((long double)p[0] - four) / ((m(0x84) - m(0x7c)) - eight) * m(0xdc);
    const long double ru = m(0xdc) / ((m(0x84) - m(0x7c)) - eight) * three;
    volatile float x0 = (float)(u - ru), x1 = (float)(ru + u);
    const long double v = ((long double)p[1] - four) / ((m(0x88) - m(0x80)) - eight) * m(0xf0);
    const long double rv = m(0xf0) / ((m(0x88) - m(0x80)) - eight) * three;
    volatile float y0 = (float)(v - rv), y1 = (float)(rv + v);
    const int32_t n = *(const int32_t *)(self + 0xf8);
    const float *const pts = *(const float *const *)(self + 0x100);
    int32_t i = 0;
    for (; i < n; i++)
        if (x0 <= pts[2 * i] && pts[2 * i] <= x1 && y0 <= pts[2 * i + 1] && pts[2 * i + 1] <= y1) return i;
    return i;
}
static const uint8_t scenery_point_884d20_original[] = {0x83, 0xec, 0x0c, 0x8b, 0x44, 0x24, 0x10, 0xd9, 0x00, 0x56};
REWRITE("Scenery::Point884D20", 0x884d20, scenery_point_884d20_original, &scenery_point_884d20, &g_scenery_point_finds);

// ---- Ranking::Order7C34F0(values) (0x7C34F0): the order of six values, largest first, into +0x378 -----------------------
// Six rounds each pick the largest value not yet picked (the first on ties) and put its index (3 bits) at bits 3 r of
// +0x378, the other bits kept.
static volatile long g_value_orders;
static void __thiscall ranking_order_7c34f0(uint8_t *self, const float *v) {
    g_value_orders++;
    uint32_t used = 0;
    for (uint32_t shift = 0; shift < 18; shift += 3) {
        int32_t best = -1;
        float top = 0;
        for (int32_t k = 0; k < 6; k++) {
            if (used & (1u << k)) continue;
            if (best < 0 || v[k] > top) top = v[k], best = k;
        }
        uint32_t *const bits = (uint32_t *)(self + 0x378);
        *bits = (*bits & ~(7u << shift)) | ((uint32_t)best << shift);
        used |= 1u << best;
    }
}
static const uint8_t ranking_order_7c34f0_original[] = {0x51, 0x53, 0x55, 0x56, 0x8b, 0xe9, 0x57, 0x8b, 0x7c, 0x24, 0x18};
REWRITE("Ranking::Order7C34F0", 0x7c34f0, ranking_order_7c34f0_original, &ranking_order_7c34f0, &g_value_orders);

// ---- Camera::Projection() (0xCD75C0): the camera's projection matrix (+0xE0) --------------------------------------------
// n = near (+0x188); with a zoom (+0x19C) not 0: n + (far - near) zoom (4 flat, else 1) / 1000 when the game's 0x1400150 is
// set, else n (zoom + 1). A = +0x1A0 / tan(field of view (+0x190) / 2), B = A +0x194. The matrix is cleared, then A, B,
// the shift (+0x180, +0x184), q = far / (far - n) (flat (+0x1A6): +0x198 / (far - n) and +0x198 at +0x11C; else 1 at
// +0x10C) and -q n. Bit 0 of +0x1A4 is cleared when the clip plane (+0x1A7) is on; that plane (+0x170: n, d) in view
// space (rows +0x120..+0x148, the point -d n less the eye +0x160) replaces the third column when its distance is below
// -0.001.
static volatile long g_camera_projections_b;
static void __thiscall camera_projection(uint8_t *self) {
    g_camera_projections_b++;
    auto m = [&](uint32_t at) { return (long double)*(const float *)(self + at); };
    volatile float *const f = (volatile float *)self;
    volatile uint32_t *const u = (volatile uint32_t *)self;
    long double n = m(0x188);
    if (!(m(0x19c) == *(const float *)0xf264e0)) {
        if (*(const volatile uint32_t *)0x1400150) {
            const long double k = self[0x1a6] ? (long double)*(const float *)0xf2669c : (long double)*(const float *)0xf264dc;
            n = n + (m(0x18c) - m(0x188)) * m(0x19c) * k * *(const float *)0xf2cf10;
        } else
            n = n * (m(0x19c) + *(const float *)0xf264dc);
    }
    const long double a = m(0x1a0) / x87_tan_h2(m(0x190) * *(const float *)0xf2651c);
    const long double b = a * m(0x194);
    for (uint32_t at = 0xe0; at < 0x120; at += 4) u[at / 4] = 0;
    f[0xe0 / 4] = (float)a;
    u[0x100 / 4] = *(const uint32_t *)(self + 0x180);
    f[0xf4 / 4] = (float)b;
    u[0x104 / 4] = *(const uint32_t *)(self + 0x184);
    const long double span = m(0x18c) - n;
    long double q;
    if (self[0x1a6]) {
        u[0x11c / 4] = *(const uint32_t *)(self + 0x198);
        q = m(0x198) / span;
    } else {
        q = m(0x18c) / span;
        u[0x10c / 4] = 0x3f800000;
    }
    f[0x108 / 4] = (float)q;
    f[0x118 / 4] = (float)-(q * n);
    self[0x1a4] &= 0xfe;
    if (!self[0x1a7]) return;
    const long double pa = m(0x170), pb = m(0x174), pc = m(0x178), nd = -m(0x17c);
    volatile float p0 = (float)(nd * pa), p1 = (float)(pb * nd), p2 = (float)(pc * nd);
    volatile float n0 = (float)((pc * m(0x128) + pb * m(0x124)) + pa * m(0x120));
    volatile float n1 = (float)((pc * m(0x138) + pb * m(0x134)) + pa * m(0x130));
    const long double n2 = (pc * m(0x148) + pb * m(0x144)) + pa * m(0x140);
    const long double e0 = (long double)p0 - m(0x160), e1 = (long double)p1 - m(0x164), e2 = (long double)p2 - m(0x168);
    volatile float w0 = (float)((e2 * m(0x128) + e1 * m(0x124)) + e0 * m(0x120));
    volatile float w1 = (float)((e2 * m(0x138) + e1 * m(0x134)) + e0 * m(0x130));
    const long double w2 = (e2 * m(0x148) + e1 * m(0x144)) + e0 * m(0x140);
    const long double dist = -((w2 * n2 + (long double)w1 * n1) + (long double)w0 * n0);
    volatile float distf = (float)dist;
    if (!(dist < *(const float *)0xf37b2c)) return;
    f[0x108 / 4] = (float)n2;
    u[0xe8 / 4] = *(const volatile uint32_t *)&n0;
    u[0xf8 / 4] = *(const volatile uint32_t *)&n1;
    u[0x118 / 4] = *(const volatile uint32_t *)&distf;
}
static const uint8_t camera_projection_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x30, 0xd9, 0x81, 0x88, 0x01, 0x00, 0x00};
REWRITE("Camera::Projection", 0xcd75c0, camera_projection_original, &camera_projection, &g_camera_projections_b);

// ---- Person::Values4FEA70(out) (0x4FEA70): three values from the +0xB8 record's +0x5F8..+0x65C group --------------------
// None (0, only al) without the record or its +0x66C. Three sums built from the record's flags (+0x64C, +0x654, +0x644 /
// +0x648, +0x650), its count +0x65C (capped at 9 and 11, and +0x65C - 5 kept in 0..10), its six whole numbers +0x614..
// +0x62C (as floats) and the capped +0x640, +0x63C, +0x638, +0x634, +0x5F8, each with the game's weights
// (0xF36794..0xF367EC and others), in the game's order and roundings.
static volatile long g_person_values_e;
static uint8_t __thiscall person_values_4fea70(const uint8_t *self, float *out) {
    g_person_values_e++;
    const uint8_t *const p = *(const uint8_t *const *)(self + 0xb8);
    if (!p || !*(const uint32_t *)(p + 0x66c)) return 0;
    auto k = [](uint32_t at) { return (long double)*(const float *)at; };
    auto i = [&](uint32_t at) { return *(const int32_t *)(p + at); };
    auto cap = [](int32_t v, int32_t most) { return (long double)(v < most ? v : most); };
    long double x = *(const float *)0xf264e0;
    volatile float l0 = 0.0f, l1 = 0.0f;
    if (i(0x64c) > 0) {
        *(volatile uint32_t *)&l1 = 0x3ecccccd;
        x = k(0xf270b8);
        *(volatile uint32_t *)&l0 = 0x3f0ccccd;
    }
    long double y = l1;
    if (i(0x654) > 0) {
        y = y + k(0xf367ec), x = x + k(0xf367e8);
        l0 = (float)((long double)l0 + k(0xf367e4));
    }
    if (i(0x644) > 0 || i(0x648) > 0) {
        y = y + k(0xf2651c), x = x + k(0xf27ca8);
        l0 = (float)((long double)l0 + k(0xf283d8));
    }
    if (i(0x650) > 0) y = y + k(0xf367e8), x = x + k(0xf27ca8);
    const int32_t v = i(0x65c);
    if (v > 0) {
        y = y + cap(v, 9) * k(0xf367e0);
        x = x + cap(v, 11) * k(0xf367dc);
        int32_t c = v - 5;
        c = c < 0 ? 0 : c > 10 ? 10 : c;
        l0 = (float)((long double)c * k(0xf283d8) + l0);
    }
    volatile float f2 = (float)i(0x61c), f3 = (float)i(0x618), f4 = (float)i(0x614), f5 = (float)i(0x62c), f6 = (float)i(0x628), f7 = (float)i(0x624);
    l1 = (float)((((((long double)f2 * k(0xf29304) + y) + (long double)f3 * k(0xf367d8)) + (long double)f4 * k(0xf367d4)) + (long double)f5 * k(0xf367d0) + (long double)f6 * k(0xf367d0)) + (long double)f7 * k(0xf367cc));
    x = (((((x + (long double)f2 * k(0xf367c8)) + (long double)f3 * k(0xf367c4)) + (long double)f4 * k(0xf367c0)) + (long double)f5 * k(0xf367c8)) + (long double)f6 * k(0xf367c4)) + (long double)f7 * k(0xf367c0);
    long double z = (((((long double)f2 * k(0xf270b4) + l0) + (long double)f3 * k(0xf367bc)) + (long double)f4 * k(0xf367b8)) + (long double)f5 * k(0xf367b4) + (long double)f6 * k(0xf367bc)) + (long double)f7 * k(0xf367b0);
    const int32_t w = i(0x640);
    l1 = (float)(cap(w, 4) * k(0xf29320) + l1);
    z = z + cap(w, 8) * k(0xf29320);
    long double d = cap(i(0x63c), 6) * k(0xf270e4) + l1;
    d = d + cap(i(0x638), 6) * k(0xf367ac);
    d = d + cap(i(0x634), 7) * k(0xf367a8);
    const int32_t f = i(0x5f8);
    if (f > 0) {
        d = d + cap(f, 6) * k(0xf367a4);
        x = x + cap(f, 6) * k(0xf2651c);
        z = z + cap(f, 6) * k(0xf367a0);
    }
    volatile float *const o = out;
    o[0] = (float)(d * k(0xf3679c));
    o[1] = (float)(x * k(0xf36798));
    o[2] = (float)(z * k(0xf36794));
    return 1;
}
static const uint8_t person_values_4fea70_original[] = {0x8b, 0x81, 0xb8, 0x00, 0x00, 0x00, 0x83, 0xec, 0x20, 0x85, 0xc0};
REWRITE("Person::Values4FEA70", 0x4fea70, person_values_4fea70_original, &person_values_4fea70, &g_person_values_e);

// ---- Track::SpeedBBD650(dt, frame, dir, push) (0xBBD650): moves a body along a track frame's axis -----------------------
// When on (+0x14): s = frame's third row (+0x20) . dir; g = 1 past the limit (+0x1C), (s + lim) / 2 lim inside it, else 0.
// The target f = ((g k push) . the second row (+0x10)) times that row (k +0x20). The velocity wanted toward f from the
// place (+0x30) over dt is eased (sqrt of its length times K1 over (length + K2)); if that would carry past f it heads
// straight for f instead. The change from the velocity (+0x40), capped at length K3, is added over dt; then the place
// moves by the velocity dt. (K1 0xF355E4, K2 0xF26E2C, K3 0xF28218; roundings as the game's.)
static volatile long g_track_speeds;
static void __thiscall track_speed_bbd650(uint8_t *self, float dt, const float *a, const float *b, const float *c) {
    g_track_speeds++;
    if (!*(const uint32_t *)(self + 0x14)) return;
    volatile float *const s = (volatile float *)self;
    auto k = [](uint32_t at) { return (long double)*(const float *)at; };
    const long double one = k(0xf264dc), zero = k(0xf264e0), lim = s[7];
    const long double sv = ((long double)a[10] * b[2] + (long double)a[9] * b[1]) + (long double)b[0] * a[8];
    volatile float sf = (float)sv;
    long double g;
    if (sv > lim) g = one;
    else if (-lim < sf) g = ((long double)sf + lim) / (lim + lim);
    else g = zero;
    const long double kk = s[8];
    const long double a0 = kk * c[0] * g;
    volatile float kc1 = (float)(kk * c[1]), kc2 = (float)(kk * c[2]);
    const long double a1 = (long double)kc1 * g, a2 = g * kc2;
    const long double f = (a2 * a[6] + a1 * a[5]) + a0 * a[4];
    volatile float f0 = (float)(f * a[4]), f1 = (float)(f * a[5]), f2 = (float)(f * a[6]);
    volatile float d0 = (float)((long double)f0 - s[12]), d1 = (float)((long double)f1 - s[13]), d2 = (float)((long double)f2 - s[14]);
    const long double r = one / dt;
    const long double e0 = (long double)d0 * r;
    volatile float e1 = (float)((long double)d1 * r);
    const long double e2 = (long double)d2 * r;
    volatile float e2f = (float)e2;
    const long double len = x87_sqrt_h2((e2 * e2f + (long double)e1 * e1) + e0 * e0);
    const long double h = x87_sqrt_h2(len) * k(0xf355e4) / (len + k(0xf26e2c));
    volatile float hf = (float)h;
    volatile float he0 = (float)(h * e0), he2 = (float)((long double)e2f * hf);
    long double he1 = (long double)e1 * hf;
    const long double g0 = (long double)s[12] - f0, g1 = (long double)s[13] - f1, g2 = (long double)s[14] - f2;
    volatile float n0 = (float)((long double)he0 * dt + s[12]), n1 = (float)(he1 * dt + s[13]);
    volatile float he2dt = (float)((long double)he2 * dt);
    const long double n2 = (long double)he2dt + s[14];
    volatile float m0 = (float)((long double)n0 - f0), m1 = (float)((long double)n1 - f1);
    const long double m2 = n2 - f2;
    if ((m2 * g2 + (long double)m1 * g1) + (long double)m0 * g0 < zero) {
        const long double x0 = (long double)f0 - s[12];
        volatile float x1 = (float)((long double)f1 - s[13]), x2 = (float)((long double)f2 - s[14]);
        volatile float rf = (float)r;
        he0 = (float)(x0 * r);
        he1 = (long double)x1 * rf;
        he2 = (float)((long double)x2 * rf);
    }
    volatile float w0 = (float)((long double)he0 - s[16]), w1 = (float)(he1 - s[17]), w2 = (float)((long double)he2 - s[18]);
    volatile float rf2 = (float)r;
    long double z0 = r * w0, z1 = (long double)w1 * rf2;
    const long double z2 = (long double)w2 * rf2;
    volatile float z2f = (float)z2;
    const long double zl = x87_sqrt_h2((z2 * z2f + z1 * z1) + z0 * z0);
    if (zl > k(0xf28218)) {
        const long double sc = k(0xf28218) / (zl + k(0xf26e2c));
        z0 = z0 * sc;
        z1 = z1 * sc;
        z2f = (float)((long double)z2f * sc);
    }
    volatile float q0 = (float)(z0 * dt), q1 = (float)(z1 * dt);
    const long double q2 = (long double)z2f * dt;
    s[18] = (float)(q2 + s[18]);
    s[16] = (float)((long double)q0 + s[16]);
    s[17] = (float)((long double)q1 + s[17]);
    const long double p0 = (long double)dt * s[16];
    volatile float p1 = (float)((long double)dt * s[17]), p2 = (float)((long double)dt * s[18]);
    s[14] = (float)((long double)p2 + s[14]);
    s[12] = (float)(p0 + s[12]);
    s[13] = (float)((long double)p1 + s[13]);
}
static const uint8_t track_speed_bbd650_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x50, 0x8b, 0x41, 0x14};
REWRITE("Track::SpeedBBD650", 0xbbd650, track_speed_bbd650_original, &track_speed_bbd650, &g_track_speeds);

// ---- Scenery::Near86F890(item, w) (0x86F890): spreads w over the cells near an item ------------------------------------
// The item's middle: (+0xA0, +0xA4) plus (its +0x9C's +0x10 + +0x14) / 4. Over the places 7 before to 7 after it in x
// and y (kept within 0..+4 +8 and 0..+0xC +4): within a squared distance of 26 the place's cell (cells of +4 places,
// rows of +8, floats at +0x10) gains w, within 50 it gains w / 2. In the game's order (x outside, y inside).
static volatile long g_scenery_spreads;
static void __thiscall scenery_near_86f890(const uint8_t *self, const uint8_t *item, float w) {
    g_scenery_spreads++;
    const uint8_t *const a = *(const uint8_t *const *)(item + 0x9c);
    const int32_t c = (*(const int32_t *)(a + 0x10) + *(const int32_t *)(a + 0x14)) / 4;
    const int32_t cx = *(const int32_t *)(item + 0xa0) + c, cy = *(const int32_t *)(item + 0xa4) + c;
    const int32_t cw = *(const int32_t *)(self + 4), rw = *(const int32_t *)(self + 8), rh = *(const int32_t *)(self + 0xc);
    const int32_t x0 = cx - 7 < 0 ? 0 : cx - 7, x1 = cx + 8 > cw * rw ? cw * rw : cx + 8;
    const int32_t y0 = cy - 7 < 0 ? 0 : cy - 7, y1 = cy + 8 > rh * cw ? rh * cw : cy + 8;
    for (int32_t x = x0; x < x1; x++)
        for (int32_t y = y0; y < y1; y++) {
            const int32_t d = (y - cy) * (y - cy) + (x - cx) * (x - cx);
            if (d > 50) continue;
            const long double add = d > 26 ? (long double)w * *(const float *)0xf2651c : (long double)w;
            float *const cell = *(float *const *)(self + 0x10) + (y / *(const int32_t *)(self + 4)) * *(const int32_t *)(self + 8) + x / *(const int32_t *)(self + 4);
            *(volatile float *)cell = (float)(add + *cell);
        }
}
static const uint8_t scenery_near_86f890_original[] = {0x83, 0xec, 0x1c, 0x55, 0x56, 0x8b, 0x74, 0x24, 0x28};
REWRITE("Scenery::Near86F890", 0x86f890, scenery_near_86f890_original, &scenery_near_86f890, &g_scenery_spreads);

// ---- Points::StepA6F0A0(dt) (0xA6F0A0): pulls a shape's points so their mean offset moves at most dt K -------------------
// The sums as Animal::PointA6ED40 takes them (four points at a time, the same roundings; here the y difference is left
// unrounded); m = (sum of +0x10.. - sum of +0x20..) / n (1 / n a float; m a float each). Gives L = |m| K2 (K2 0xF8F980).
// With m.y below 0 taken as 0: when |m| is past dt K (K 0xF8F954, a float), every point's +0x10.. moves back by m (1 -
// dt K / |m|).
static volatile long g_point_pulls;
static long double __thiscall points_step_a6f0a0(const uint8_t *self, float dt) {
    g_point_pulls++;
    volatile float most = (float)((long double)dt * *(const float *)0xf8f954);
    const int32_t n = *(const int32_t *)(self + 0x10);
    const float *const *const list = *(const float *const *const *)(self + 0xc);
    long double x = *(const float *)0xf264e0, ureg = *(const float *)0xf264e0;
    volatile float y = 0, z = 0, u = 0, v = 0, w = 0, xf;
    auto uvw = [&](const float *e) {
        u = (float)((long double)u + e[8]);
        v = (float)((long double)v + e[9]);
        w = (float)((long double)w + e[10]);
    };
    int32_t i = 0;
    if (n >= 4) {
        for (uint32_t m = (uint32_t)(n - 4) / 4 + 1; m; m--, i += 4) {
            const float *const e0 = list[i], *const e1 = list[i + 1], *const e2 = list[i + 2], *const e3 = list[i + 3];
            x = x + e0[4];
            long double yt = (long double)y + e0[5];
            z = (float)((long double)z + e0[6]);
            uvw(e0);
            xf = (float)(x + e1[4]);
            yt = yt + e1[5];
            const long double zt = (long double)z + e1[6];
            uvw(e1);
            xf = (float)((long double)xf + e2[4]);
            y = (float)(yt + e2[5]);
            z = (float)(zt + e2[6]);
            uvw(e2);
            x = (long double)xf + e3[4];
            y = (float)((long double)y + e3[5]);
            z = (float)((long double)z + e3[6]);
            ureg = (long double)u + e3[8];
            u = (float)ureg;
            v = (float)((long double)v + e3[9]);
            w = (float)((long double)w + e3[10]);
        }
    }
    for (; i < n; i++) {
        const float *const e = list[i];
        x = x + e[4];
        y = (float)((long double)y + e[5]);
        z = (float)((long double)z + e[6]);
        ureg = ureg + e[8];
        v = (float)((long double)v + e[9]);
        w = (float)((long double)w + e[10]);
    }
    const long double dx = x - ureg, dy = (long double)y - v;
    volatile float dz = (float)((long double)z - w);
    volatile float s = (float)((long double)*(const float *)0xf264dc / (long double)n);
    volatile float mx = (float)(dx * s), my = (float)(dy * s);
    const long double mzu = (long double)dz * s;
    volatile float mz = (float)mzu;
    volatile float a = (float)(mzu * mz), b = (float)((long double)mx * mx);
    const long double len = x87_sqrt_h2(((long double)my * my + b) + a) * *(const float *)0xf8f980;
    if (my < *(const float *)0xf264e0) *(volatile uint32_t *)&my = 0;
    const long double l2 = x87_sqrt_h2(((long double)my * my + b) + a);
    if (!(l2 > most)) return len;
    const long double q = (long double)most / l2;
    volatile float t1 = (float)((long double)my * q), t2 = (float)((long double)mz * q);
    volatile float e0 = (float)((long double)mx - (long double)mx * q);
    volatile float e1 = (float)((long double)my - t1), e2 = (float)((long double)mz - t2);
    for (int32_t k = 0; k < *(const int32_t *)(self + 0x10); k++) {
        volatile float *const p = (float *)list[k] + 4;
        p[0] = (float)((long double)p[0] - e0);
        p[1] = (float)((long double)p[1] - e1);
        p[2] = (float)((long double)p[2] - e2);
    }
    return len;
}
static const uint8_t points_step_a6f0a0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x44, 0xd9, 0x45, 0x08};
REWRITE("Points::StepA6F0A0", 0xa6f0a0, points_step_a6f0a0_original, &points_step_a6f0a0, &g_point_pulls);

// ---- Track::Turn85CB30(out in ecx, n in edx) (0x85CB30): the matrix turned about y by (n + 2) quarter turns back -------
// An identity matrix (16-byte aligned) turned by Matrix::TurnPairs (0xC208C0) with (n + 2) times -pi/2 (a float), copied
// into out. Gives out.
static volatile long g_track_quarter_turns;
static float *__fastcall track_turn_85cb30(float *out, int32_t n) {
    g_track_quarter_turns++;
    alignas(16) float m[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    volatile float angle = (float)((long double)(n + 2) * *(const float *)0xf7808c);
    ((void(__thiscall *)(float *, float))0xc208c0)(m, angle);
    for (int k = 0; k < 16; k++) ((volatile uint32_t *)out)[k] = ((const uint32_t *)m)[k];
    return out;
}
static const uint8_t track_turn_85cb30_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x4c, 0x83, 0xc2, 0x02};
REWRITE("Track::Turn85CB30", 0x85cb30, track_turn_85cb30_original, &track_turn_85cb30, &g_track_quarter_turns);

// ---- Window::SetPoint680B50(p) (0x680B50): moves a window part to a point --------------------------------------------
// +0xD4, +0xD8 = p. The part's rectangle (+0x7C..+0x88) moved so its corner sits at p: x0 = (x0 - p.x) + p.x, y0 = (y0 -
// p.y) + p.y, x1 = (x1 - x0) + that x0, y1 = (y1 - y0) + the new y0 (the game's roundings), handed to Window::UpdateRect
// (0x6AFAF0). Gives what that gives.
static volatile long g_window_part_moves;
static uint32_t __thiscall window_set_point_680b50(uint8_t *self, const uint32_t *p) {
    g_window_part_moves++;
    *(uint32_t *)(self + 0xd4) = p[0], *(uint32_t *)(self + 0xd8) = p[1];
    auto m = [&](uint32_t at) { return (long double)*(const float *)(self + at); };
    volatile float t = (float)(m(0x7c) - m(0xd4));
    const long double x0 = (long double)t + m(0xd4);
    float r[4];
    volatile float *const rv = r;
    rv[1] = (float)((m(0x80) - m(0xd8)) + m(0xd8));
    rv[0] = (float)x0;
    rv[2] = (float)((m(0x84) - m(0x7c)) + x0);
    rv[3] = (float)((m(0x88) - m(0x80)) + rv[1]);
    return ((uint32_t(__thiscall *)(void *, float *))0x6afaf0)(self, r);
}
static const uint8_t window_set_point_680b50_original[] = {0x83, 0xec, 0x18, 0x8b, 0x44, 0x24, 0x1c, 0x8b, 0x10};
REWRITE("Window::SetPoint680B50", 0x680b50, window_set_point_680b50_original, &window_set_point_680b50, &g_window_part_moves);

// ---- Window::Scale6ACBD0(out in ecx, p in edx) (0x6ACBD0): a point snapped to whole steps of the screen's scale -------
// With (sx, sy) from Screen::AreaScale (0x6ACB30): out = (floor(p.x / sx + 0.5) sx, sy floor(p.y / sy + 0.5)) (the
// floors of floats). Gives out.
static volatile long g_screen_snaps;
static float *__fastcall window_scale_6acbd0(float *out, const float *p) {
    g_screen_snaps++;
    float sc[2];
    ((void(__thiscall *)(float *))0x6acb30)(sc);
    const long double half = *(const float *)0xf2651c;
    volatile float a = (float)((long double)p[1] / sc[1] + half);
    volatile float qy = (float)floor_h2(a);
    volatile float b = (float)((long double)p[0] / sc[0] + half);
    volatile float qx = (float)floor_h2(b);
    volatile float *const o = out;
    o[0] = (float)((long double)qx * sc[0]);
    o[1] = (float)((long double)sc[1] * qy);
    return out;
}
static const uint8_t window_scale_6acbd0_original[] = {0x83, 0xec, 0x14, 0x56, 0x57, 0x8b, 0xf1, 0x8d, 0x4c, 0x24, 0x14};
REWRITE("Window::Scale6ACBD0", 0x6acbd0, window_scale_6acbd0_original, &window_scale_6acbd0, &g_screen_snaps);

// ---- Car::Dot50BEA0(a in ecx, b in edx) (0x50BEA0): the signed angle between two directions ---------------------------
// d = (a.y b.y + a.x b.x) + b.z a.z (a float): 0 at or past 0.9999, pi below -0.9999, else the game's acos(d) (0xDDEBC0),
// negated when b.x a.z - b.z a.x is above 0.
static volatile long g_car_angles;
static long double car_acos_h2(long double x) {
    __asm__ volatile("call *%1" : "+t"(x) : "r"((uintptr_t)0xddebc0) : "eax", "ecx", "edx", "memory", "st(1)", "st(2)", "st(3)",
                     "st(4)", "st(5)", "st(6)", "st(7)");
    return x;
}
static long double __fastcall car_dot_50bea0(const float *a, const float *b) {
    g_car_angles++;
    const long double d = ((long double)a[1] * b[1] + (long double)a[0] * b[0]) + (long double)b[2] * a[2];
    volatile float df = (float)d;
    if (!(d < *(const float *)0xf36e48)) return *(const float *)0xf264e0;
    if (df < *(const float *)0xf36e44) return *(const float *)0xf285f0;
    long double r = car_acos_h2(df);
    if ((long double)b[0] * a[2] - (long double)b[2] * a[0] > *(const float *)0xf264e0) r = -r;
    return r;
}
static const uint8_t car_dot_50bea0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x08, 0x56, 0x57};
REWRITE("Car::Dot50BEA0", 0x50bea0, car_dot_50bea0_original, &car_dot_50bea0, &g_car_angles);

// ---- Vector::RotateByAxis988070(axis) (0x988070): turns the vector about axis's direction by axis's length --------------
// L = sqrt((x x + y y) + z z) (a float): below 1e-20 nothing changes and gives the vector; else Vector::RotateAround
// (0xBFB0B0) with axis / L (16-byte aligned floats) and L, giving what that gives (the vector).
static volatile long g_axis_rotations;
static float *__thiscall vector_rotate_by_axis_988070(float *v, const float *axis) {
    g_axis_rotations++;
    const long double x = axis[0], y = axis[1], z = axis[2];
    const long double len = x87_sqrt_h2((x * x + y * y) + z * z);
    volatile float lf = (float)len;
    if (len < *(const float *)0xf27c70) return v;
    const long double inv = (long double)*(const float *)0xf264dc / lf;
    alignas(16) float d[3];
    volatile float *const dv = d;
    dv[0] = (float)(inv * axis[0]), dv[1] = (float)(inv * axis[1]), dv[2] = (float)(inv * axis[2]);
    return ((float *(__thiscall *)(float *, const float *, float))0xbfb0b0)(v, d, lf);
}
static const uint8_t vector_rotate_by_axis_988070_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x20, 0x8b, 0x55, 0x08};
REWRITE("Vector::RotateByAxis988070", 0x988070, vector_rotate_by_axis_988070_original, &vector_rotate_by_axis_988070, &g_axis_rotations);

// ---- Fireworks::Point9A9FD0(object) (0x9A9FD0): places a window part (self is its +0xDC) at an object's offset ---------
// With the part w = self - 0xDC: x0 = (o.+0xF8 - o.+0xE8) + w.+0x114, y0 = (o.+0xFC - o.+0xEC) + w.+0x118, x1 = (w.+0x84 -
// w.+0x7C) + x0, y1 = (w.+0x88 - w.+0x80) + y0 (the game's roundings), handed to Window::UpdateRect (0x6AFAF0) for w.
// Gives what that gives.
static volatile long g_firework_part_places;
static uint32_t __thiscall fireworks_point_9a9fd0(uint8_t *self, const uint8_t *o) {
    g_firework_part_places++;
    uint8_t *const w = self - 0xdc;
    auto f = [](const uint8_t *p, uint32_t at) { return (long double)*(const float *)(p + at); };
    float r[4];
    volatile float *const rv = r;
    rv[0] = (float)((f(o, 0xf8) - f(o, 0xe8)) + f(w, 0x114));
    const long double y0 = (f(o, 0xfc) - f(o, 0xec)) + f(w, 0x118);
    rv[1] = (float)y0;
    rv[2] = (float)((f(w, 0x84) - f(w, 0x7c)) + rv[0]);
    rv[3] = (float)((f(w, 0x88) - f(w, 0x80)) + y0);
    return ((uint32_t(__thiscall *)(void *, float *))0x6afaf0)(w, r);
}
static const uint8_t fireworks_point_9a9fd0_original[] = {0x83, 0xec, 0x18, 0x8b, 0x44, 0x24, 0x1c, 0xd9, 0x80, 0xf8, 0x00, 0x00, 0x00};
REWRITE("Fireworks::Point9A9FD0", 0x9a9fd0, fireworks_point_9a9fd0_original, &fireworks_point_9a9fd0, &g_firework_part_places);

// ---- Animal::RayA5EF50(out) (0xA5EF50): the point a distance along the camera's middle ray ------------------------------
// Camera::ScreenRay (0xCD79C0) of the camera (+4) at (0, 0) gives origin o and direction d; with s = *(+8): out = (o.x +
// d.x s, o.y + d.y s, o.z + d.z s) (the y and z steps floats). Gives out.
static volatile long g_animal_camera_points;
static float *__thiscall animal_ray_a5ef50(const uint8_t *self, float *out) {
    g_animal_camera_points++;
    float o[4], d[4];
    o[3] = d[3] = 3.40282347e38f;
    ((void(__thiscall *)(const void *, float, float, float *, float *))0xcd79c0)(*(const void *const *)(self + 4), 0.0f, 0.0f, o, d);
    const long double s = **(const float *const *)(self + 8);
    volatile float ty = (float)((long double)d[1] * s), tz = (float)((long double)d[2] * s);
    volatile float *const v = out;
    v[0] = (float)((long double)o[0] + (long double)d[0] * s);
    v[1] = (float)((long double)o[1] + ty);
    v[2] = (float)((long double)o[2] + tz);
    return out;
}
static const uint8_t animal_ray_a5ef50_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x3c, 0x56, 0x8b, 0xf1};
REWRITE("Animal::RayA5EF50", 0xa5ef50, animal_ray_a5ef50_original, &animal_ray_a5ef50, &g_animal_camera_points);

// ---- Effect::Value7B06C0(effect in esi, a, b) (0x7B06C0): takes a share of the effect's direction off its place ---------
// v = Effect::Value7B05E0 (0x7B05E0, a and b in ecx and edx, 0 on the stack). The place (+0x30..) loses the direction
// (+0x10..) times v (x, y steps floats, z unrounded) and +0x3C becomes 1 (nothing is left on the FPU).
static volatile long g_effect_pulls;
extern "C" void __stdcall forever_effect_value_7b06c0(uint8_t *e, uint32_t a, uint32_t b) {
    g_effect_pulls++;
    const long double v = ((long double(__fastcall *)(uint32_t, uint32_t, uint32_t))0x7b05e0)(a, b, 0);
    float *const f = (float *)e;
    volatile float px = (float)((long double)f[4] * v), py = (float)((long double)f[5] * v);
    const long double pz = v * f[6];
    *(volatile uint32_t *)(e + 0x3c) = 0x3f800000;
    volatile float *const p = f + 12;
    p[2] = (float)((long double)p[2] - pz);
    p[0] = (float)((long double)p[0] - px);
    p[1] = (float)((long double)p[1] - py);
}
extern "C" __attribute__((naked)) void forever_effect_value_7b06c0_regs() {   // effect in esi; a, b on the stack
    __asm__("pushl 8(%esp)\n\tpushl 8(%esp)\n\tpushl %esi\n\tcalll _forever_effect_value_7b06c0@12\n\tret $8");
}
static const uint8_t effect_value_7b06c0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x50, 0x8b, 0x4e, 0x14};
REWRITE("Effect::Value7B06C0", 0x7b06c0, effect_value_7b06c0_original, (void *)&forever_effect_value_7b06c0_regs, &g_effect_pulls);

// ---- Track::TurnBC5D50(out in ecx, n in edx) (0xBC5D50): the 3 x 4 matrix turned about y by (n + 2) quarter turns back --
// An identity (each row's fourth the largest float; 16-byte aligned) turned by 0xBEDF70 with (n + 2) times -pi/2 (a
// float); the first three of each row copied into out (the fourths left). Gives out.
static volatile long g_track_quarter_turns2;
static float *__fastcall track_turn_bc5d50(float *out, int32_t n) {
    g_track_quarter_turns2++;
    const float big = 3.40282347e38f;
    alignas(16) float m[12] = {1, 0, 0, big, 0, 1, 0, big, 0, 0, 1, big};
    volatile float angle = (float)((long double)(n + 2) * *(const float *)0xf7808c);
    ((void(__thiscall *)(float *, float))0xbedf70)(m, angle);
    for (int r = 0; r < 3; r++)
        for (int k = 0; k < 3; k++) ((volatile uint32_t *)out)[r * 4 + k] = ((const uint32_t *)m)[r * 4 + k];
    return out;
}
static const uint8_t track_turn_bc5d50_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x3c, 0x83, 0xc2, 0x02};
REWRITE("Track::TurnBC5D50", 0xbc5d50, track_turn_bc5d50_original, &track_turn_bc5d50, &g_track_quarter_turns2);

// ---- Window::Place6ADB60(out) (0x6ADB60): a part's rectangle on the screen ---------------------------------------------
// Without a parent (+0x3C): its rectangle (+0x7C..+0x88). Else the rectangle plus the parent's place on the screen
// (Window::ScreenPosition, 0x6AD560), and with flag 0x1000 (+0x94) first plus the parent's scroll (+0x8C, +0x90) (those
// sums floats). Gives out.
static volatile long g_window_screen_rects;
static float *__thiscall window_place_6adb60(const uint8_t *self, float *out) {
    g_window_screen_rects++;
    auto m = [&](uint32_t at) { return (long double)*(const float *)(self + at); };
    uint8_t *const parent = *(uint8_t *const *)(self + 0x3c);
    if (!parent) {
        for (int k = 0; k < 4; k++) ((volatile uint32_t *)out)[k] = ((const uint32_t *)(self + 0x7c))[k];
        return out;
    }
    volatile float t[4];
    if ((*(const uint32_t *)(self + 0x94) >> 12) & 1) {
        const long double sx = *(const float *)(parent + 0x8c), sy = *(const float *)(parent + 0x90);
        t[0] = (float)(sx + m(0x7c)), t[1] = (float)(m(0x80) + sy), t[2] = (float)(sx + m(0x84)), t[3] = (float)(m(0x88) + sy);
    } else
        for (int k = 0; k < 4; k++) t[k] = *(const float *)(self + 0x7c + k * 4);
    float place[2];
    const float *const p = ((const float *(__thiscall *)(uint8_t *, float *))0x6ad560)(parent, place);
    volatile float *const o = out;
    o[0] = (float)((long double)t[0] + p[0]);
    o[1] = (float)((long double)t[1] + p[1]);
    o[2] = (float)((long double)t[2] + p[0]);
    o[3] = (float)((long double)t[3] + p[1]);
    return out;
}
static const uint8_t window_place_6adb60_original[] = {0x83, 0xec, 0x18, 0x56, 0x8b, 0xf1, 0x8b, 0x4e, 0x3c, 0x85, 0xc9};
REWRITE("Window::Place6ADB60", 0x6adb60, window_place_6adb60_original, &window_place_6adb60, &g_window_screen_rects);

// ---- Water::Cell90B150(x, y, z, amount) (0x90B150): moves water by places given in the grid's units --------------------
// i = floor((x - +8) / (+0x10 - +8) n), j likewise for y, k = floor((z - +0xC) / (+0x14 - +0xC) n) (n = +0, the steps
// floats), then Water::Cell908060 (0x908060) with i, j, k and amount.
static volatile long g_water_place_moves;
static void __thiscall water_cell_90b150(uint8_t *self, float x, float y, float z, float amount) {
    g_water_place_moves++;
    auto m = [&](uint32_t at) { return (long double)*(const float *)(self + at); };
    const int32_t n = *(const int32_t *)self;
    auto cell = [&](float v, uint32_t lo, uint32_t hi) {
        volatile float f = (float)(((long double)v - m(lo)) / (m(hi) - m(lo)) * (long double)n);
        volatile float c = (float)floor_h2(f);
        return (int32_t)x87_fistp64_h2(c);
    };
    const int32_t i = cell(x, 8, 0x10), j = cell(y, 8, 0x10), k = cell(z, 0xc, 0x14);
    ((void(__thiscall *)(uint8_t *, int32_t, int32_t, int32_t, float))0x908060)(self, i, j, k, amount);
}
static const uint8_t water_cell_90b150_original[] = {0x83, 0xec, 0x14, 0xd9, 0x44, 0x24, 0x18, 0x8b, 0x41, 0x08};
REWRITE("Water::Cell90B150", 0x90b150, water_cell_90b150_original, &water_cell_90b150, &g_water_place_moves);

// ---- Animal::StepA91180(shape) (0xA91180): a 16 x 16 map of which places of a fence shape lie inside -------------------
// The shape ({corners, count} at +0) gets its +8..+0x24 cleared, then for z and then x from -1.875 to 1.875 by 0.25 a bit
// (from bit 0 of +8 on) set when Fence::CountA90B50 (0xA90B50) puts (x, 0, z) inside.
static volatile long g_fence_maps;
static void __thiscall animal_step_a91180(void *, uint8_t *shape) {
    g_fence_maps++;
    uint32_t *bits = (uint32_t *)(shape + 8);
    for (int k = 0; k < 8; k++) bits[k] = 0;
    uint32_t mask = 1;
    const long double step = *(const float *)0xf270b8, end = *(const float *)0xf2705c;
    for (volatile float z = *(const float *)0xf91904;;) {
        for (volatile float x = *(const float *)0xf91904;;) {
            const float p[3] = {x, 0.0f, z};
            if (((uint8_t(__stdcall *)(const void *, const float *))0xa90b50)(shape, p) & 0xff) *bits |= mask;
            mask <<= 1;
            const long double nx = (long double)x + step;
            x = (float)nx;
            if (!(nx < end)) break;
        }
        if (!mask) mask = 1, bits++;
        const long double nz = (long double)z + step;
        z = (float)nz;
        if (!(nz < end)) break;
    }
}
static const uint8_t animal_step_a91180_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x24, 0x33, 0xc0, 0x53};
REWRITE("Animal::StepA91180", 0xa91180, animal_step_a91180_original, &animal_step_a91180, &g_fence_maps);

// ---- Terrain::Corner8F3650() (0x8F3650): the edges of each terrain ring, each wider than the last ---------------------
// From the grid (*self): origin (+0xC, +0x10) and size (bytes +4, +5 times +0x14, +0x18; floats). For each ring (0x30
// bytes at +8, count +4): the origin moves out by the margins (a, b start at 4), the margins double, the size grows by
// them (all floats), then Terrain::Scale8F2480 (0x8F2480) fills the ring.
static volatile long g_terrain_rings;
static void __thiscall terrain_corner_8f3650(const uint8_t *self) {
    g_terrain_rings++;
    const uint8_t *const g = *(const uint8_t *const *)self;
    float origin[2], size[2];
    volatile float *const o = origin, *const s = size;
    *(volatile uint32_t *)&o[0] = *(const uint32_t *)(g + 0xc), *(volatile uint32_t *)&o[1] = *(const uint32_t *)(g + 0x10);
    s[0] = (float)((long double)g[4] * *(const float *)(g + 0x14));
    s[1] = (float)((long double)g[5] * *(const float *)(g + 0x18));
    volatile float a = 4.0f, b = 4.0f;
    for (int32_t i = 0; i < *(const int32_t *)(self + 4); i++) {
        o[0] = (float)((long double)o[0] - a);
        o[1] = (float)((long double)o[1] - b);
        a = (float)((long double)a + a);
        b = (float)((long double)b + b);
        s[0] = (float)((long double)a + s[0]);
        s[1] = (float)((long double)b + s[1]);
        ((void(__thiscall *)(const uint8_t *, const float *, const float *))0x8f2480)(*(const uint8_t *const *)(self + 8) + i * 0x30, origin, size);
    }
}
static const uint8_t terrain_corner_8f3650_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf8, 0x83, 0xec, 0x1c, 0x53, 0x56};
REWRITE("Terrain::Corner8F3650", 0x8f3650, terrain_corner_8f3650_original, &terrain_corner_8f3650, &g_terrain_rings);

// ---- Window::Hit655360(p) (0x655360): whether a point hits the window (and not its child's rounded corner) -------------
// 0 (only al) when p is outside the rectangle (+0x7C, Rect::Contains 0xBE7780) or below its bottom - 1. With no child
// (+0x100) 1. Else with the point inside (lx, ly = p - the corner): 1 when lx is left of or past the child's span (+0x7C,
// +0x84), else 1 only when (lx - (child x + 19), ly + 1) is farther than 18.
static volatile long g_window_hits;
static uint8_t __thiscall window_hit_655360(const uint8_t *self, const float *p) {
    g_window_hits++;
    if (!(((uint8_t(__thiscall *)(const void *, const float *))0xbe7780)(self + 0x7c, p) & 0xff)) return 0;
    auto m = [](const uint8_t *w, uint32_t at) { return (long double)*(const float *)(w + at); };
    if (m(self, 0x88) - *(const float *)0xf264dc < p[1]) return 0;
    const uint8_t *const k = *(const uint8_t *const *)(self + 0x100);
    if (!k) return 1;
    volatile float lx = (float)((long double)p[0] - m(self, 0x7c)), ly = (float)((long double)p[1] - m(self, 0x80));
    if (lx < *(const float *)(k + 0x7c)) return 1;
    if (lx > *(const float *)(k + 0x84)) return 1;
    const long double dx = (long double)lx - (m(k, 0x7c) + *(const float *)0xf279b8);
    const long double dy = (long double)ly - *(const float *)0xf268f0;
    return x87_sqrt_h2(dy * dy + dx * dx) > *(const float *)0xf360c4;
}
static const uint8_t window_hit_655360_original[] = {0x83, 0xec, 0x08, 0x53, 0x8b, 0x5c, 0x24, 0x10, 0x56, 0x8b, 0xf1};
REWRITE("Window::Hit655360", 0x655360, window_hit_655360_original, &window_hit_655360, &g_window_hits);

// ---- Triangle record swaps (32 bytes): the three point numbers, the two floats at +0x10 (into b through the FPU) and
// +0x18 trade places; +0xC and +0x1C stay.
static inline void swap_tri_h2(uint32_t *a, uint32_t *b) {
    const uint32_t a0 = a[0], a1 = a[1], a2 = a[2], a6 = a[6];
    alignas(4) uint32_t a4[2] = {a[4], a[5]};
    a[0] = b[0], a[1] = b[1], a[2] = b[2], a[4] = b[4], a[5] = b[5], a[6] = b[6];
    b[0] = a0, b[1] = a1, b[2] = a2;
    fpu_copy_h2(b + 4, a4);
    fpu_copy_h2(b + 5, a4 + 1);
    b[6] = a6;
}

// ---- Loop::StepCDF760(n in ecx, list in edx, points, dir) (0xCDF760): sorts n triangles, nearest along dir last --------
// n - 1 passes over all neighbouring pairs, swapping when Vector::CompareCDF200 (0xCDF200) of the pair is above 0.
static volatile long g_triangle_bubbles;
static void __fastcall loop_step_cdf760(int32_t n, uint8_t *list, const float *points, const float *dir) {
    g_triangle_bubbles++;
    for (int32_t pass = n - 1; pass > 0; pass--)
        for (int32_t j = 0; j < n - 1; j++) {
            uint32_t *const a = (uint32_t *)(list + j * 0x20), *const b = (uint32_t *)(list + j * 0x20 + 0x20);
            if (((int32_t(__fastcall *)(const void *, const void *, const float *, const float *))0xcdf200)(a, b, points, dir) > 0) swap_tri_h2(a, b);
        }
}
static const uint8_t loop_step_cdf760_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x34, 0x53, 0x8d, 0x59, 0xff};
REWRITE("Loop::StepCDF760", 0xcdf760, loop_step_cdf760_original, &loop_step_cdf760, &g_triangle_bubbles);

// ---- Loop::StepCDF690(n in ecx, list in edx, points, dir) (0xCDF690): heap-sorts n triangles -------------------------
// Heaps from n - 1 down with Loop::StepCDF4C0 (0xCDF4C0), then for k = n - 1 down to 0 swaps the first with triangle k
// and sifts the first down among k.
static volatile long g_triangle_heapsorts;
static void __fastcall loop_step_cdf690(int32_t n, uint8_t *list, const float *points, const float *dir) {
    g_triangle_heapsorts++;
    auto sift = (void(__fastcall *)(int32_t, int32_t, uint8_t *, const float *, const float *))0xcdf4c0;
    for (int32_t i = n - 1; i >= 0; i--) sift(i, n, list, points, dir);
    for (int32_t k = n - 1; k >= 0; k--) {
        swap_tri_h2((uint32_t *)list, (uint32_t *)(list + k * 0x20));
        sift(0, k, list, points, dir);
    }
}
static const uint8_t loop_step_cdf690_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x24, 0x53, 0x56, 0x8b, 0xd9};
REWRITE("Loop::StepCDF690", 0xcdf690, loop_step_cdf690_original, &loop_step_cdf690, &g_triangle_heapsorts);

// ---- Table::StepCE2BB0(mesh in ecx, p in edx) (0xCE2BB0): whether p is inside a closed mesh (odd hits straight up) -----
// For each triangle (count +0x24; word point numbers at +0x2C, three each; points of 16 bytes at +0x28): a, b, c and
// the ray from p along (0, 1, 0) through Plane::BelowCDB0E0 (0xCDB0E0) with a, b - a, c - a (floats). Gives the count's
// lowest bit (only al).
static volatile long g_mesh_insides;
static uint8_t __fastcall table_step_ce2bb0(const uint8_t *self, const float *p) {
    g_mesh_insides++;
    uint32_t hits = 0;
    const float dir[3] = {0.0f, 1.0f, 0.0f};
    for (uint32_t i = 0; i < *(const uint32_t *)(self + 0x24); i++) {
        const uint16_t *const ix = *(const uint16_t *const *)(self + 0x2c) + i * 3;
        const float *const pts = *(const float *const *)(self + 0x28);
        const float *const a = pts + ix[0] * 4, *const b = pts + ix[1] * 4, *const c = pts + ix[2] * 4;
        float ca[3], ba[3], t;
        volatile float *const vca = ca, *const vba = ba;
        for (int k = 0; k < 3; k++) vca[k] = (float)((long double)c[k] - a[k]);
        for (int k = 0; k < 3; k++) vba[k] = (float)((long double)b[k] - a[k]);
        if (((uint8_t(__fastcall *)(const float *, const float *, const float *, const float *, const float *, float *))0xcdb0e0)(a, ba, ca, p, dir, &t) & 0xff) hits++;
    }
    return hits & 1;
}
static const uint8_t table_step_ce2bb0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x54, 0x8b, 0x41, 0x24};
REWRITE("Table::StepCE2BB0", 0xce2bb0, table_step_ce2bb0_original, &table_step_ce2bb0, &g_mesh_insides);

// ---- Water::Step91C0E0(origin, dir, t) (0x91C0E0): whether a ray meets any of the water's triangles ---------------------
// Triangles of 0x30 bytes at +0x310 (count +0x318): v0 (+0), v1 (+0x10), v2 (+0x20); Plane::BelowCDB0E0 (0xCDB0E0) with
// a copy of v0, v1 - v0 and v2 - v0 (floats). 1 at the first that meets it, else 0 (only al).
static volatile long g_water_ray_hits;
static uint8_t __thiscall water_step_91c0e0(const uint8_t *self, const float *origin, const float *dir, float *t) {
    g_water_ray_hits++;
    for (int32_t i = 0; i < *(const int32_t *)(self + 0x318); i++) {
        const float *const e = (const float *)(*(const uint8_t *const *)(self + 0x310) + i * 0x30);
        float v0[3], e1[3], e2[3];
        for (int k = 0; k < 3; k++) ((volatile uint32_t *)v0)[k] = ((const uint32_t *)e)[k];
        volatile float *const ve1 = e1, *const ve2 = e2;
        for (int k = 0; k < 3; k++) ve1[k] = (float)((long double)e[4 + k] - e[k]);
        for (int k = 0; k < 3; k++) ve2[k] = (float)((long double)e[8 + k] - e[k]);
        const uint8_t hit = ((uint8_t(__fastcall *)(const float *, const float *, const float *, const float *, const float *, float *))0xcdb0e0)(v0, e1, e2, origin, dir, t);
        if (hit) return 1;
    }
    return 0;
}
static const uint8_t water_step_91c0e0_original[] = {0x55, 0x8b, 0xec, 0x83, 0xe4, 0xf0, 0x83, 0xec, 0x34, 0x53, 0x56, 0x8b, 0xf1};
REWRITE("Water::Step91C0E0", 0x91c0e0, water_step_91c0e0_original, &water_step_91c0e0, &g_water_ray_hits);
