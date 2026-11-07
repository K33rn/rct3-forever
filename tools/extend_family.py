"""Find more copies of the generated template rewrites: functions with the same instructions as a generated copy whose
constants are encoded differently (an imm32 where the generated copy has an imm8, say), so gen_rewrites' byte-shape
matchers pass them by. A copy is taken only when every operand that differs from the representative is one of the
template's parameters (or a jump inside the function, or the exception handler a frame pushes); the parameters are read
from the same operands. Writes forever/src/rewrite/extra.inc in the generated.inc format.
usage: extend_family.py <exe> [candidate addresses...]   (no addresses: every documented function of 2+ copies)"""
import csv, os, re, sys
import capstone, pefile

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
pe = pefile.PE(sys.argv[1], fast_load=True); img = pe.get_memory_mapped_image(); BASE = 0x400000
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32); md.detail = True
rows = list(csv.DictReader(open(os.path.join(HERE, 'docs', 'functions.csv'), encoding='utf-8')))
size = {int(r['address'], 16): int(r['size']) for r in rows}
status = {int(r['address'], 16): r['status'] for r in rows}


def operands(a):
    """(mnemonic, [values]) per instruction; jump/call targets inside the function as offsets from its start"""
    out = []
    for i in md.disasm(bytes(img[a - BASE:a - BASE + size[a]]), a):
        vals = []
        for o in i.operands:
            if o.type == capstone.x86.X86_OP_IMM:
                v = o.imm & 0xffffffff
                if (i.group(capstone.CS_GRP_JUMP) or i.group(capstone.CS_GRP_CALL)) and a <= v < a + size[a]:
                    v = ('local', v - a)
                vals.append(v)
            elif o.type == capstone.x86.X86_OP_MEM:
                vals.append(('mem', o.mem.base, o.mem.index, o.mem.scale, o.mem.disp & 0xffffffff))
            else:
                vals.append(('reg', o.reg))
        out.append((i.mnemonic, vals))
    return out


def flat(ops):
    """the numbers in each operand, by position"""
    res = []
    for k, (mn, vals) in enumerate(ops):
        for j, v in enumerate(vals):
            if isinstance(v, int):
                res.append(((k, j, 'imm'), v))
            elif v[0] == 'mem':
                res.append(((k, j, 'disp'), v[4]))
    return res


gen = []
for line in open(os.path.join(HERE, 'forever', 'src', 'rewrite', 'generated.inc')):
    m = re.match(r'(\w+)\((0x[0-9a-f]+)(.*)\)\s*$', line.strip())
    if not m:
        continue
    at = int(m.group(2), 16)
    args = re.findall(r'0x[0-9a-f]+|(?<![\w.])\d+(?![\w.])', m.group(3))
    vals = [int(x, 0) for x in args]
    nbytes = 0
    for n in range(len(vals), 0, -1):
        if vals[-n:] == list(img[at - BASE:at - BASE + n]):
            nbytes = n
            break
    gen.append((m.group(1), at, vals[:len(vals) - nbytes], nbytes))
genops = {}


def shape(ops):
    return tuple((mn, tuple(v if not isinstance(v, int) and v[0] == 'reg' else (v[0], v[1], v[2], v[3]) if not isinstance(v, int) and v[0] == 'mem' else 'n' for v in vals)) for mn, vals in ops)


by_shape = {}
for macro, at, params, nb in gen:
    if at not in size:
        continue
    ops = operands(at)
    by_shape.setdefault(shape(ops), []).append((macro, at, params, nb, ops))

emitted = {at for _, at, _, _ in gen}
cands = [int(x, 16) for x in sys.argv[2:] if x.startswith('0x')] or [a for a in size if status[a] == 'documented']
lines, taken = [], 0
for a in cands:
    if a in emitted or status.get(a) != 'documented':
        continue
    try:
        ops = operands(a)
    except Exception:
        continue
    for macro, at, params, nb, rops in by_shape.get(shape(ops), []):
        # where each parameter sits in the representative (every place it appears)
        rf, tf = flat(rops), flat(ops)
        where = [[p for p, v in rf if v == param] for param in params]
        if any(not w for w in where):
            continue
        newp = []
        for w in where:
            got = {dict(tf)[p] for p in w}
            if len(got) != 1:
                break
            newp.append(got.pop())
        else:
            pos = {p for w in where for p in w}
            # every other differing number must be the frame's handler (an imm pushed from 0xdf.. to 0xeb..)
            ok = True
            for (p, rv), (_, tv) in zip(rf, tf):
                if p in pos or rv == tv:
                    continue
                k = p[0]
                if rops[k][0] == 'push' and 0xdf0000 <= rv < 0xeb0000 and 0xdf0000 <= tv < 0xeb0000:
                    continue
                ok = False
                break
            # local jumps must land on the same instruction (their offsets can differ with encodings)
            if ok:
                rl = [v for mn, vals in rops for v in vals if isinstance(v, tuple) and v[0] == 'local']
                tl = [v for mn, vals in ops for v in vals if isinstance(v, tuple) and v[0] == 'local']
                starts_r = [i.address - at for i in md.disasm(bytes(img[at - BASE:at - BASE + size[at]]), at)]
                starts_t = [i.address - a for i in md.disasm(bytes(img[a - BASE:a - BASE + size[a]]), a)]
                ok = len(rl) == len(tl) and all(
                    (x[1] in starts_r and y[1] in starts_t and starts_r.index(x[1]) == starts_t.index(y[1])) for x, y in zip(rl, tl))
            if ok:
                first = ', '.join(f'0x{b:02x}' for b in img[a - BASE:a - BASE + max(nb, 8)])
                lines.append(f'{macro}({a:#x}, ' + ', '.join(f'{v:#x}' for v in newp) + (', ' if newp else '') + f'{first})')
                taken += size[a]
                break
_path = os.path.join(HERE, 'forever', 'src', 'rewrite', 'extra.inc')
if '--loose' not in sys.argv:   # add to the list, never drop copies found before
    _old = open(_path).read().splitlines() if os.path.exists(_path) else []
    _keys = {l.split(',')[0] for l in _old}
    open(_path, 'w', newline='\n').write('\n'.join(_old + [l for l in lines if l.split(',')[0] not in _keys]) + '\n')
print(len(lines), 'copies,', taken, 'bytes')
for l in lines[:12]:
    print(' ', l[:110])


def code_values(a):
    """immediates pushed and absolute call targets, in order (the exception frame's handler left out)"""
    out = []
    for i in md.disasm(bytes(img[a - BASE:a - BASE + size[a]]), a):
        for o in i.operands:
            if o.type == capstone.x86.X86_OP_IMM and i.mnemonic in ('push', 'call'):
                v = o.imm & 0xffffffff
                if i.mnemonic == 'push' and 0xdf0000 <= v < 0xeb0000:
                    continue
                if i.mnemonic == 'call' and a <= v < a + size[a]:
                    continue
                out.append((i.mnemonic, v))
    return out


def loose(macros, cands):
    """array resizers whose only differences are how count * size is worked out and which registers hold what: the
    parameters are read from the pushes and calls at the same places, and every other push and call must agree;
    the template's own test then decides"""
    import difflib
    reps = [(m, at, p, nb) for m, at, p, nb in gen if m in macros and at in size]
    found = []
    for a in cands:
        if a in emitted or status.get(a) != 'documented' or a not in size:
            continue
        tv = code_values(a)
        mnt = [i.mnemonic for i in md.disasm(bytes(img[a - BASE:a - BASE + size[a]]), a)]
        for m, at, params, nb in reps:
            if abs(size[at] - size[a]) > 24:
                continue
            rv = code_values(at)
            if len(rv) != len(tv) or [x[0] for x in rv] != [x[0] for x in tv]:
                continue
            mnr = [i.mnemonic for i in md.disasm(bytes(img[at - BASE:at - BASE + size[at]]), at)]
            if difflib.SequenceMatcher(None, mnr, mnt, autojunk=False).ratio() < 0.85:
                continue
            newp, ok = [], True
            for pv in params:
                idx = [k for k, x in enumerate(rv) if x[1] == pv]
                if not idx:
                    newp.append(pv)   # not pushed or called (a header size, say): the same as the representative
                    continue
                got = {tv[k][1] for k in idx}
                if len(got) != 1:
                    ok = False
                    break
                newp.append(got.pop())
            pidx = {k for k, x in enumerate(rv) if x[1] in params}
            if ok and all(rv[k][1] == tv[k][1] for k in range(len(rv)) if k not in pidx):
                found.append(f'{m}({a:#x}, ' + ', '.join(f'{v:#x}' for v in newp) + ', ' + ', '.join(f'0x{b:02x}' for b in img[a - BASE:a - BASE + max(nb, 8)]) + ')')
                break
    return found


if '--loose' in sys.argv:
    extra = loose({'OBJECT_SETCAP', 'OBJECT_RESIZE', 'TRIVIAL_SETCAP'}, [a for a in size if status[a] == 'documented'])
    have = open(os.path.join(HERE, 'forever', 'src', 'rewrite', 'extra.inc')).read().splitlines()
    keep = {l.split(',')[0] for l in have}
    extra = [l for l in extra if l.split(',')[0] not in keep]
    open(os.path.join(HERE, 'forever', 'src', 'rewrite', 'extra.inc'), 'w', newline='\n').write('\n'.join(have + extra) + '\n')
    print('loose:', len(extra), 'more copies', sum(size[int(l.split('(')[1].split(',')[0], 16)] for l in extra), 'bytes')
    for l in extra[:8]:
        print(' ', l[:100])
