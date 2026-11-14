"""Check that a family of functions is one function written out several times, and write what differs between them.
The copies must be the same instructions with the same operands, except: jumps are compared by the instruction they
go to; the exception handler a frame pushes (the push after push -1) is set aside; and some operands differ (constants,
addresses, calls). Those are the family's parameters, written in the order they appear, one row per copy:
    NAME(address, parameter, ..., then the copy's first 7 bytes)
to forever/src/rewrite/<name>.inc, with the operands' places in a comment, so the rewrite can say what each one is. The
rewrite is written by hand once and takes its parameters from the rows. A family that needs a parameter to be the
same in places (one address used twice, say) gets that checked too.
usage: family.py <exe> <name> <address> <address> ...   (the first copy is the one the others are compared with)"""
import os, sys
import capstone, pefile

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
pe = pefile.PE(sys.argv[1], fast_load=True); img = pe.get_memory_mapped_image(); BASE = 0x400000
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32); md.detail = True
name, copies = sys.argv[2], [int(a, 16) for a in sys.argv[3:]]
sizes = {}
for line in open(os.path.join(HERE, 'docs', 'functions.csv'), encoding='utf-8'):
    parts = line.split(',')
    if parts[0].startswith('0x'):
        sizes[int(parts[0], 16)] = int(parts[1])


def read(at):
    """Each instruction as (mnemonic, operands), and the instruction's text; operands as values, with jump targets as
    instruction numbers. The function is read to its end (past the recorded size, to the ret after it)."""
    ins = []
    for x in md.disasm(bytes(img[at - BASE:at - BASE + sizes[at] + 16]), at):
        ins.append(x)
        if x.address + x.size >= at + sizes[at] and x.mnemonic == 'ret':
            break
    index = {x.address: k for k, x in enumerate(ins)}
    out = []
    for k, x in enumerate(ins):
        ops = []
        for o in x.operands:
            if o.type == capstone.x86.X86_OP_IMM:
                v = o.imm & 0xffffffff
                if x.group(capstone.CS_GRP_JUMP) and v in index:
                    v = ('to', index[v])
                elif x.group(capstone.CS_GRP_JUMP):
                    raise SystemExit(f'{at:#x}: a jump out of the function at {x.address:#x}')
                ops.append(v)
            elif o.type == capstone.x86.X86_OP_MEM:
                ops.append(('mem', o.mem.base, o.mem.index, o.mem.scale, o.mem.disp & 0xffffffff))
            else:
                ops.append(('reg', o.reg))
        handler = k > 0 and ins[k - 1].mnemonic == 'push' and ins[k - 1].op_str == '-1' and x.mnemonic == 'push'
        out.append((x.mnemonic, ops, handler, f'{x.mnemonic} {x.op_str}', x.address - at))
    return out


first = read(copies[0])
reads = {a: read(a) for a in copies}
params = []   # (instruction number, operand number)
for a in copies[1:]:
    other = reads[a]
    if len(other) != len(first):
        raise SystemExit(f'{a:#x}: {len(other)} instructions, the first copy has {len(first)}')
    for k, ((m0, o0, h0, t0, _), (m1, o1, h1, t1, _)) in enumerate(zip(first, other)):
        if m0 != m1 or len(o0) != len(o1):
            raise SystemExit(f'{a:#x}: instruction {k} differs: {t0} / {t1}')
        for j, (v0, v1) in enumerate(zip(o0, o1)):
            if v0 == v1 or h0:
                continue
            same_kind = (isinstance(v0, int) and isinstance(v1, int)) or (
                isinstance(v0, tuple) and isinstance(v1, tuple) and v0[0] == v1[0] == 'mem' and v0[1:4] == v1[1:4] and v0[1] == 0 and v0[2] == 0)
            if not same_kind:
                raise SystemExit(f'{a:#x}: instruction {k} differs in more than a constant: {t0} / {t1}')
            if (k, j) not in params:
                params.append((k, j))
params.sort()


def value(read_, k, j):
    v = read_[k][1][j]
    return v if isinstance(v, int) else v[4]


# a value used in several places must be the same in each place for every copy, or it is a parameter of its own
rows = []
for a in copies:
    rows.append([value(reads[a], k, j) for k, j in params] + list(img[a - BASE:a - BASE + 7]))
groups = []   # parameters that are always equal to an earlier one are the same parameter
for i, p in enumerate(params):
    for g in groups:
        if all(r[g[0]] == r[i] for r in rows):
            g.append(i)
            break
    else:
        groups.append([i])
out = [f'// {name}: one function written out {len(copies)} times (checked by tools/family.py: the same instructions, jumps by',
       '// where they go, the exception handler set aside). Its parameters, by the instructions they appear in:']
for n, g in enumerate(groups):
    places = ', '.join(f'+{first[params[i][0]][4]:#x} {first[params[i][0]][3]}' for i in g)
    out.append(f'//   p{n}: {places}')
out.append('// Do not edit by hand.')
for a, r in zip(copies, rows):
    out.append(f'{name.upper()}({a:#x}, ' + ', '.join(f'{r[g[0]]:#x}' for g in groups) + ', ' + ', '.join(f'0x{b:02x}' for b in r[len(params):]) + ')')
open(os.path.join(HERE, 'forever', 'src', 'rewrite', f'{name}.inc'), 'w', newline='\n').write('\n'.join(out) + '\n')
print('\n'.join(out))
