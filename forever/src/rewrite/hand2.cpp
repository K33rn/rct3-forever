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
