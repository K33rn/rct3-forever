# Checks hand-written rewrites: copies whose constants differ must not run the same code. usage: tools/copies_audit.py (from the repo root)
import re, glob, csv, collections, pefile, capstone
from gamepath import game_exe
pe = pefile.PE(game_exe(), fast_load=True); img = pe.get_memory_mapped_image(); B = 0x400000
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32); md.detail = True
sizes = {int(r['address'], 16): int(r['size']) for r in csv.DictReader(open('docs/functions.csv', encoding='utf-8'))}


def slots(a, n):
    code = img[a - B:a - B + n]; out = bytearray(code); vals = []
    for i in md.disasm(bytes(code), a):
        e = i.encoding; base = i.address - a
        rel = i.group(capstone.CS_GRP_JUMP) or i.group(capstone.CS_GRP_CALL)
        for off, sz in ((e.disp_offset, e.disp_size), (e.imm_offset, e.imm_size)):
            if not sz: continue
            raw = int.from_bytes(code[base + off:base + off + sz], 'little')
            if rel and off == e.imm_offset:
                t = i.operands[0].imm; raw = t - a if a <= t < a + n else t
            vals.append((base + off, i.mnemonic, raw)); out[base + off:base + off + sz] = bytes(sz)
    return bytes(out), vals


groups = collections.defaultdict(list)
for p in glob.glob('forever/src/rewrite/*.cpp'):
    s = open(p, encoding='utf-8').read()
    arrays = {m.group(1): [int(h, 16) for h in re.findall(r'0x[0-9a-fA-F]+', m.group(2))]
              for m in re.finditer(r'uint32_t\s+(\w+)\[\]\s*=\s*\{([^}]*)\}', s)}
    for m in re.finditer(r'REWRITE_COPIES\("([^"]+)",\s*(\w+)', s):
        groups[m.group(1)] += [(a, 'array') for a in arrays.get(m.group(2), [])]
    for m in re.finditer(r'REWRITE\("([^"]+)",\s*(0x[0-9a-fA-F]+),\s*([^\n]*)', s):
        # the function part only: what the copy actually runs
        fn = re.search(r'&(\w+(?:<[^>]*>)?)', m.group(3))
        groups[m.group(1)].append((int(m.group(2), 16), fn.group(1) if fn else m.group(3)))
n = 0
for name, copies in sorted(groups.items()):
    shapes = collections.defaultdict(list)
    for a, fn in copies:
        if a not in sizes: continue
        try: mb, v = slots(a, sizes[a])
        except Exception: continue
        shapes[mb].append((a, v, fn))
    for mb, g in shapes.items():
        if len(g) < 2: continue
        for k in range(len(g[0][1])):
            off, mn = g[0][1][k][0], g[0][1][k][1]
            vals = {x[1][k][2] for x in g}
            if len(vals) < 2: continue
            if mn == 'push' and off in (3, 9) and min(vals) >= 0xdf0000: continue
            byfn = collections.defaultdict(set)
            for x in g: byfn[x[2]].add(x[1][k][2])
            bad = {f: v for f, v in byfn.items() if len(v) > 1}
            if bad:
                n += 1
                f, v = next(iter(bad.items()))
                print(f'{name}: +{off:#x} {mn} values {sorted(hex(x) for x in v)[:4]} share {f[:60]}')
print(n, 'real slots to read')
