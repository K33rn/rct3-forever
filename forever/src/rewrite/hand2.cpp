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
