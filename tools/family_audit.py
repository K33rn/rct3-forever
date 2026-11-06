"""Check every generated family for a constant that differs between its copies but isn't one of its parameters.
Families are matched with immediates and displacements masked, so a copy can look the same and still differ.
usage: family_audit.py <exe> forever/src/rewrite/generated.inc
Lists, per family, each operand slot that varies and whose values are not all among the copies' parameters."""
import re, sys, pefile, capstone
from collections import defaultdict

pe = pefile.PE(sys.argv[1], fast_load=True); img = pe.get_memory_mapped_image(); BASE = 0x400000
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32); md.detail = True
sizes = {}
import csv, os
for r in csv.DictReader(open(os.path.join(os.path.dirname(sys.argv[2]), '..', '..', '..', 'docs', 'functions.csv'), encoding='utf-8')):
    sizes[int(r['address'], 16)] = int(r['size'])


def slots(a, n):
    """masked bytes and the (offset, mnemonic, value) of each operand; call and jump targets compared as absolute"""
    code = img[a - BASE:a - BASE + n]
    out = bytearray(code); vals = []; end = 0
    for i in md.disasm(bytes(code), a):
        e = i.encoding; base = i.address - a; end = base + i.size
        rel = i.group(capstone.CS_GRP_JUMP) or i.group(capstone.CS_GRP_CALL)
        for off, sz in ((e.disp_offset, e.disp_size), (e.imm_offset, e.imm_size)):
            if not sz:
                continue
            raw = int.from_bytes(code[base + off:base + off + sz], 'little')
            if rel and off == e.imm_offset:
                t = i.operands[0].imm
                raw = t - a if a <= t < a + n else t
            vals.append((base + off, i.mnemonic, raw, sz))
            out[base + off:base + off + sz] = bytes(sz)
    return bytes(out), vals, end


fam = defaultdict(list)
for line in open(sys.argv[2]):
    m = re.match(r'(\w+)\((0x[0-9a-f]+)(.*)\)\s*$', line.strip())
    if not m:
        continue
    at = int(m.group(2), 16)
    args = [int(x, 0) for x in re.findall(r'0x[0-9a-f]+|(?<![\w.])\d+(?![\w.])', m.group(3))]
    # the trailing arguments are the function's own first bytes, the check before replacing: not parameters
    for n in range(len(args), 0, -1):
        if args[-n:] == list(img[at - BASE:at - BASE + n]):
            args = args[:-n]
            break
    fam[m.group(1)].append((at, set(args)))

problems = 0
for name, copies in sorted(fam.items()):
    shapes = defaultdict(list)
    for a, params in copies:
        n = sizes.get(a)
        if not n:
            continue
        try:
            mb, v, end = slots(a, n)
        except Exception:
            continue
        shapes[mb].append((a, v, params))
    for mb, group in shapes.items():
        if len(group) < 2:
            continue
        first = group[0][1]
        for k in range(len(first)):
            values = {g[1][k][2] for g in group}
            if first[k][1] == 'push' and first[k][0] in (3, 9) and min(values) >= 0xdf0000 and max(values) < 0xeb0000:
                continue   # the exception frame's handler: only for clean-up, the rewrite has its own
            if len(values) < 2:
                continue
            sz = first[k][3]
            def covered(value, params):
                s = value - (1 << (8 * sz)) if value >> (8 * sz - 1) else value
                return value in params or (s & 0xffffffff) in params or abs(s) in params
            bad = [g for g in group if not covered(g[1][k][2], g[2])]
            if bad:
                problems += 1
                print(f'{name}: {len(group)} copies, +{first[k][0]:#x} {first[k][1]} varies {sorted(hex(x) for x in values)[:6]}; not a parameter in {len(bad)}, e.g. {bad[0][0]:#x}')
print(problems, 'slots to look at')
