"""Rank the not-yet-rewritten template families: functions byte-identical once imm/disp operands are masked.
usage: cleanfam.py <exe> docs/functions.csv forever/src/rewrite/generated.inc [min_copies] [--show ADDR]
Per family: copies, size, x87 ops, calls, SEH, and which operands truly vary (rel calls compared as absolute targets)."""
import sys, csv, pefile, capstone
from collections import defaultdict
pe = pefile.PE(sys.argv[1], fast_load=True); img = pe.get_memory_mapped_image(); BASE = 0x400000
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32); md.detail = True
args = sys.argv[4:]
show = {int(x, 16) for x in args[args.index('--show') + 1].split(',')} if '--show' in args else None
minc = int(args[0]) if args and args[0].isdigit() else 3


def scan(a, n):
    out = bytearray(img[a - BASE:a - BASE + n]); vals = []; fpu = calls = 0; seh = False; last = 0
    for i in md.disasm(bytes(img[a - BASE:a - BASE + n]), a):
        enc = i.encoding; base = i.address - a; last = base + i.size
        if i.mnemonic[0] == 'f' and i.mnemonic != 'fs': fpu += 1
        if 'fs:' in i.op_str: seh = True
        if i.mnemonic == 'call': calls += 1
        rel = i.group(capstone.CS_GRP_JUMP) or i.group(capstone.CS_GRP_CALL)
        for off, sz in ((enc.disp_offset, enc.disp_size), (enc.imm_offset, enc.imm_size)):
            if not sz: continue
            raw = int.from_bytes(img[a - BASE + base + off:a - BASE + base + off + sz], 'little')
            if rel and off == enc.imm_offset:
                local = a <= i.operands[0].imm < a + n
                vals.append((base + off, i.mnemonic, (i.operands[0].imm - a) if local else i.operands[0].imm))
            else:
                vals.append((base + off, i.mnemonic, raw))
            out[base + off:base + off + sz] = bytes(sz)
    if last != n: raise ValueError
    return bytes(out), vals, fpu, calls, seh


emitted = {int(l[l.index('(') + 1:l.index(',')], 16) for l in open(sys.argv[3]) if '(0x' in l and ',' in l}
groups = defaultdict(list)
for r in csv.DictReader(open(sys.argv[2], encoding='utf-8')):
    if r['status'] != 'documented': continue
    n = int(r['size']); a = int(r['address'], 16)
    if n < 24 or n > 700 or a in emitted: continue
    try: m, v, f, c, s = scan(a, n)
    except Exception: continue
    groups[m].append((a, n, v, f, c, s))
rows = []
for g in groups.values():
    if len(g) < minc: continue
    vary = [k for k in range(len(g[0][2])) if len({x[2][k][2] for x in g}) > 1]
    rows.append((len(g) * g[0][1], g, vary))
rows.sort(key=lambda r: -r[0])
if show is not None:
    for tot, g, vary in rows:
        if any(x[0] in show for x in g):
            print('copies', [hex(x[0]) for x in g])
            for k in vary:
                print(f'  +{g[0][2][k][0]:#x} {g[0][2][k][1]}:', [hex(x[2][k][2]) for x in g])
    sys.exit()
tot_int = sum(r[0] for r in rows if not r[1][0][3] and not r[1][0][5])
print('integer, no-SEH family bytes:', tot_int)
for tot, g, vary in rows:
    x = g[0]
    print(f'{len(g):3} x {x[1]:4} = {tot:6} fpu={x[3]:3} calls={x[4]:2} seh={int(x[5])} vary={len(vary):2} {hex(x[0])}')
