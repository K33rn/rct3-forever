"""Read destructors from the game's code as ordered lists of small steps, forever/src/rewrite/dtor_ops_list.inc.
The compiler wrote most destructors out of the same few pieces, in the class's order: set a table or clear a field,
delete a member if there is one (its virtual destructor), free a member's memory, release a counted member, run a
member's or the base's destructor, and a few fixed odds and ends. This follows each destructor's code in a line (its
only branches skip a step when a member is null) and writes the steps down; dtor_ops.cpp runs them. Anything it
doesn't know (a loop, a call with computed arguments, arithmetic) leaves the function out, listed with --why. The
exception frame's bookkeeping (stores to the stack) is not rebuilt. The equivalence tester checks every one.
usage: dtor_ops.py <exe> [--why]"""
import collections, csv, glob, os, re, sys
import capstone, pefile
from capstone import x86

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
pe = pefile.PE(sys.argv[1], fast_load=True); img = pe.get_memory_mapped_image(); BASE = 0x400000
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
md.detail = True
SAVED = ('ebx', 'esi', 'edi', 'ebp')
SET, IF, VCALL, CDECL, CALL, GCOPY, SETM, REL, FREEM, TEXTREL = range(10)   # the step kinds (dtor_ops.cpp)
FREE_ALIGNED, FREE = 0xbce010, 0xbce000


class Unknown(Exception):
    pass


def is_mem(op, base=None):
    return op.type == x86.X86_OP_MEM


def read(a, n):
    ins = list(md.disasm(bytes(img[a - BASE:a - BASE + n]), a))
    if not ins or ins[-1].mnemonic != 'ret' or ins[-1].op_str or ins[-1].address + ins[-1].size != a + n:
        raise Unknown('does not end in a plain ret')
    at = {x.address: k for k, x in enumerate(ins)}
    regs = {'ecx': ('this', 0)}        # register -> ('this', offset) | ('member', offset of the field it was read from) | ('global', address)
    ops = []
    started = False

    def reg(op):
        return ins[0].reg_name(op.reg)

    def this_off(op):   # a memory operand that is a field of the object -> its offset, else None
        m = op.mem
        if m.index or m.segment or not m.base:
            return None
        b = regs.get(ins[0].reg_name(m.base))
        return b[1] + m.disp if b and b[0] == 'this' else None

    def noise(x):
        """the exception frame, saved registers and stack bookkeeping"""
        o = x.operands
        if x.mnemonic == 'mov' and len(o) == 2:
            if is_mem(o[0]) and o[0].mem.base == x86.X86_REG_ESP:
                return True                                   # the frame's state, or this kept on the stack
            if 'fs:' in x.op_str:
                return True
            if o[0].type == x86.X86_OP_REG and is_mem(o[1]) and o[1].mem.base == x86.X86_REG_ESP:
                regs[reg(o[0])] = None                        # the saved frame link (or this back from the stack: then later uses fail)
                return True
        if x.mnemonic in ('sub', 'add') and x.op_str.startswith('esp,'):
            return True
        if x.mnemonic == 'pop':
            regs[reg(o[0])] = None
            return True
        return False

    def block(i, end, member):
        """the steps from ins[i] up to the address `end`, inside a null check of the member in register `member`"""
        out = []
        while ins[i].address < end:
            x, o = ins[i], ins[i].operands
            if noise(x):
                i += 1
                continue
            # delete or call through its table: mov r, [member]; (push constant)*; call [r + slot]
            if x.mnemonic == 'mov' and o[0].type == x86.X86_OP_REG and is_mem(o[1]) and not o[1].mem.index and o[1].mem.disp == 0 \
                    and ins[0].reg_name(o[1].mem.base) == member:
                table = reg(o[0])
                j, args = i + 1, []
                while ins[j].mnemonic == 'push' and ins[j].operands[0].type == x86.X86_OP_IMM:
                    args.append(ins[j].operands[0].imm & 0xffffffff)
                    j += 1
                c = ins[j]
                if c.mnemonic == 'call' and is_mem(c.operands[0]) and ins[0].reg_name(c.operands[0].mem.base) == table and not c.operands[0].mem.index \
                        and member == 'ecx' and len(args) <= 2:
                    args = args[::-1]
                    out.append((VCALL, len(args), c.operands[0].mem.disp, args[0] if args else 0, args[1] if len(args) > 1 else 0))
                    for r in ('eax', 'ecx', 'edx'):
                        regs[r] = None
                    i = j + 1
                    continue
                raise Unknown('a call through a member\'s table with other arguments')
            # free its memory: push member; call fn; add esp, 4
            if x.mnemonic == 'push' and o[0].type == x86.X86_OP_REG and reg(o[0]) == member:
                c, d = ins[i + 1], ins[i + 2]
                if c.mnemonic == 'call' and c.operands[0].type == x86.X86_OP_IMM and d.mnemonic == 'add' and d.op_str == 'esp, 4':
                    out.append((CDECL, 0, 0, c.operands[0].imm, 0))
                    for r in ('eax', 'ecx', 'edx'):
                        regs[r] = None
                    if member in ('eax', 'ecx', 'edx'):
                        member = None
                    i += 3
                    continue
                raise Unknown('a member pushed for something else')
            # release a counted member: dec word [member + rc]; cmp word [member + rc], 0; jne skip; mov r, [member]; (push 1); call [r + slot]
            if x.mnemonic == 'dec' and is_mem(o[0]) and o[0].size == 2 and ins[0].reg_name(o[0].mem.base) == member and member == 'ecx':
                c, j = ins[i + 1], ins[i + 2]
                if c.mnemonic == 'cmp' and c.op_str.endswith(', 0') and c.operands[0].mem.disp == o[0].mem.disp and j.mnemonic == 'jne':
                    skip = j.operands[0].imm
                    inner = block(i + 3, skip, member)
                    if len(inner) == 1 and inner[0][0] == VCALL and inner[0][1] <= 1 and (inner[0][1] == 0 or inner[0][3] == 1):
                        out.append((REL, inner[0][1], o[0].mem.disp, inner[0][2], 0))
                        i = at[skip]
                        member = None
                        continue
                raise Unknown('a counted release of another shape')
            s = step(i)
            if s is None:
                raise Unknown(f'in a null check: {x.mnemonic} {x.op_str}')
            if any(k[0] == IF for k in s[1]):
                member = None                                 # another member taken inside: this one isn't used again
            out += s[1]
            i = s[0]
        if ins[i].address != end:
            raise Unknown('a null check ends inside a step')
        return out

    def text_release(i, r, off):
        """a shared text let go: mov r, [this + off]; dec dword [r]; mov r2, [this + off]; cmp dword [r2], 0; jne end; test r2, r2; je end;
        mov r3, [r2 + 0xc]; push r3; call FreeAligned; push r2; call Free; add esp, 8; end:"""
        def skip(j):
            while noise(ins[j]):
                j += 1
            return j
        j = i + 1
        x = ins[j]
        if not (x.mnemonic == 'dec' and is_mem(x.operands[0]) and x.operands[0].size == 4 and x.operands[0].mem.disp == 0
                and not x.operands[0].mem.index and ins[0].reg_name(x.operands[0].mem.base) == r):
            return None
        j += 1
        x = ins[j]
        if not (x.mnemonic == 'mov' and x.operands[0].type == x86.X86_OP_REG and is_mem(x.operands[1]) and this_off(x.operands[1]) == off):
            raise Unknown('a count taken down for something else')
        r2 = reg(x.operands[0])
        j += 1
        x = ins[j]
        zero = x.mnemonic == 'cmp' and is_mem(x.operands[0]) and x.operands[1].type == x86.X86_OP_REG and regs.get(reg(x.operands[1])) == ('zero', 0)
        if not (x.mnemonic == 'cmp' and is_mem(x.operands[0]) and x.operands[0].size == 4 and x.operands[0].mem.disp == 0
                and ins[0].reg_name(x.operands[0].mem.base) == r2 and (x.op_str.endswith(', 0') or zero)):
            raise Unknown('a count taken down for something else')
        j = skip(j + 1)
        if ins[j].mnemonic != 'jne':
            raise Unknown('a count taken down for something else')
        end = ins[j].operands[0].imm
        j = skip(j + 1)
        zero = ins[j].mnemonic == 'cmp' and ins[j].operands[0].type == x86.X86_OP_REG and reg(ins[j].operands[0]) == r2 \
            and ins[j].operands[1].type == x86.X86_OP_REG and regs.get(reg(ins[j].operands[1])) == ('zero', 0)
        if not (ins[j].mnemonic == 'test' and ins[j].op_str == f'{r2}, {r2}' or zero) or ins[j + 1].mnemonic != 'je' or ins[j + 1].operands[0].imm != end:
            raise Unknown('a text release of another shape')
        j += 2
        seq = ins[j:j + 6]
        if not (seq[0].mnemonic == 'mov' and seq[0].operands[0].type == x86.X86_OP_REG and is_mem(seq[0].operands[1])
                and ins[0].reg_name(seq[0].operands[1].mem.base) == r2 and seq[0].operands[1].mem.disp == 0xc
                and seq[1].mnemonic == 'push' and seq[1].op_str == reg(seq[0].operands[0])
                and seq[2].mnemonic == 'call' and seq[2].operands[0].type == x86.X86_OP_IMM and seq[2].operands[0].imm == FREE_ALIGNED
                and seq[3].mnemonic == 'push' and seq[3].op_str == r2
                and seq[4].mnemonic == 'call' and seq[4].operands[0].type == x86.X86_OP_IMM and seq[4].operands[0].imm == FREE
                and seq[5].mnemonic == 'add' and seq[5].op_str == 'esp, 8' and ins[j + 6].address == end):
            raise Unknown('a text release of another shape')
        for c in ('eax', 'ecx', 'edx', r, r2):
            if regs.get(c) != ('zero', 0) or c in ('eax', 'ecx', 'edx'):
                regs[c] = None
        return j + 6, [(TEXTREL, 0, 0, off, 0)]

    def step(i):
        """one step at ins[i] outside any null check -> (next index, steps) or None"""
        x, o = ins[i], ins[i].operands
        m = x.mnemonic
        if m == 'xor' and o[0].type == x86.X86_OP_REG and o[1].type == x86.X86_OP_REG and o[0].reg == o[1].reg and o[0].size == 4:
            regs[reg(o[0])] = ('zero', 0)
            return i + 1, []
        if m == 'mov' and len(o) == 2:
            # clear a field from a zeroed register
            if is_mem(o[0]) and o[1].type == x86.X86_OP_REG and o[1].size == 4 and regs.get(reg(o[1])) == ('zero', 0):
                off = this_off(o[0])
                if off is not None:
                    return i + 1, [(SET, 4, 0, off, 0)]
                return None
            # set a field: mov [this + off], constant
            if is_mem(o[0]) and o[1].type == x86.X86_OP_IMM:
                off = this_off(o[0])
                if off is not None:
                    return i + 1, [(SET, o[0].size, 0, off, o[1].imm & 0xffffffff)]
                b = regs.get(ins[0].reg_name(o[0].mem.base)) if o[0].mem.base else None
                if b and b[0] == 'member' and not o[0].mem.index and o[0].size == 4 and 0 <= b[1] < 0x10000:
                    return i + 1, [(SETM, 4, b[1], o[0].mem.disp, o[1].imm & 0xffffffff)]   # through a member's pointer
                return None
            if o[0].type == x86.X86_OP_REG and o[1].type == x86.X86_OP_REG:
                regs[reg(o[0])] = regs.get(reg(o[1]))
                return i + 1, []
            if o[0].type == x86.X86_OP_REG and is_mem(o[1]):
                r = reg(o[0])
                off = this_off(o[1])
                if off is not None and o[0].size == 4:
                    regs[r] = ('member', off)
                    # a null check of it: test r, r; (fields set, frame noise); je end
                    j = i + 1
                    t = text_release(i, r, off)
                    if t:
                        return t
                    # free its memory whatever it is: push r; (noise); call fn (the stack put right later)
                    if ins[j].mnemonic == 'push' and ins[j].operands[0].type == x86.X86_OP_REG and reg(ins[j].operands[0]) == r:
                        k = j + 1
                        while noise(ins[k]):
                            k += 1
                        if ins[k].mnemonic == 'call' and ins[k].operands[0].type == x86.X86_OP_IMM:
                            for c in ('eax', 'ecx', 'edx'):
                                regs[c] = None
                            return k + 1, [(FREEM, 0, 0, off, ins[k].operands[0].imm)]
                        raise Unknown('a member pushed for a call with more arguments')
                    zero = ins[j].mnemonic == 'cmp' and ins[j].operands[0].type == x86.X86_OP_REG and reg(ins[j].operands[0]) == r \
                        and ins[j].operands[1].type == x86.X86_OP_REG and regs.get(reg(ins[j].operands[1])) == ('zero', 0)
                    if ins[j].mnemonic == 'test' and ins[j].op_str == f'{r}, {r}' or zero:
                        j += 1
                        before = []
                        while ins[j].mnemonic == 'mov':
                            if noise(ins[j]):
                                j += 1
                                continue
                            s = step(j)
                            if not s or any(k[0] != SET for k in s[1]):
                                break
                            before += s[1]
                            j = s[0]
                        if ins[j].mnemonic == 'je' and ins[j].operands[0].imm > ins[j].address:
                            end = ins[j].operands[0].imm
                            inner = block(j + 1, end, r)
                            regs[r] = None
                            return at[end], before + [(IF, 0, 0, off, len(inner))] + inner
                        raise Unknown('a member tested for something else')
                    return i + 1, []
                if not o[1].mem.base and not o[1].mem.index and o[0].size == 4:
                    # a global kept unless it is -1: mov r, [g1]; cmp r, -1; (noise); je end; mov [g0], r
                    g1 = o[1].mem.disp & 0xffffffff
                    j = i + 1
                    if ins[j].mnemonic == 'cmp' and ins[j].op_str == f'{r}, -1':
                        j += 1
                        while noise(ins[j]):
                            j += 1
                        w = ins[j + 1]
                        if ins[j].mnemonic == 'je' and w.mnemonic == 'mov' and is_mem(w.operands[0]) and not w.operands[0].mem.base \
                                and w.operands[1].type == x86.X86_OP_REG and reg(w.operands[1]) == r and ins[j].operands[0].imm == ins[j + 2].address:
                            regs[r] = None
                            return j + 2, [(GCOPY, 0, 0, w.operands[0].mem.disp & 0xffffffff, g1)]
                    return None
                return None
        if m == 'lea' and not o[1].mem.index and o[1].mem.base:
            b = regs.get(ins[0].reg_name(o[1].mem.base))
            if b and b[0] == 'this':
                regs[reg(o[0])] = ('this', b[1] + o[1].mem.disp)
                return i + 1, []
            return None
        if m == 'call' and o[0].type == x86.X86_OP_IMM:
            c = regs.get('ecx')
            if c and c[0] == 'this':
                for r in ('eax', 'ecx', 'edx'):
                    regs[r] = None
                return i + 1, [(CALL, 0, 0, o[0].imm, c[1])]   # a member's or the base's destructor
            return None
        return None

    i = 0
    while i < len(ins) - 1:
        x = ins[i]
        if noise(x):
            i += 1
            continue
        if x.mnemonic == 'push' and not started and (x.operands[0].type == x86.X86_OP_REG or i < 3):
            i += 1                                            # the frame (its two constants come first) and the saved registers
            continue
        s = step(i)
        if s is None:
            raise Unknown(f'{x.mnemonic} {re.sub("0x[0-9a-f]+", "N", x.op_str)}')
        if s[1]:
            started = True
        ops += s[1]
        i = s[0]
    if not ops:
        raise Unknown('nothing in it')
    if len(ops) > 60:
        raise Unknown('more than 60 steps')
    for k, c, small, x, y in ops:   # what the tester's stand-ins hold: table entries under 16, offsets inside its objects
        if k in (VCALL,) and not 0 <= small < 0x40 or k == REL and not (0 <= x < 0x40 and 0 <= small < 0x3c):
            raise Unknown('a table entry past the 16th or a far count')
        if k in (SET, IF, FREEM, TEXTREL) and not 0 <= x < 0xff0 or k == SETM and not (0 <= x < 0x7f0 and small < 0xff0) or k == CALL and not 0 <= y < 0x1000:
            raise Unknown('a field past 0xFF0')
    return ops


def main():
    rows = list(csv.DictReader(open(os.path.join(HERE, 'docs', 'functions.csv'), encoding='utf-8')))
    size = {int(r['address'], 16): r for r in rows}
    taken = set()
    for f in glob.glob(os.path.join(HERE, 'forever', 'src', 'rewrite', '*.cpp')) + glob.glob(os.path.join(HERE, 'forever', 'src', 'rewrite', '*.inc')):
        if not f.endswith('dtor_ops_list.inc'):
            taken |= {int(v, 16) for v in re.findall(r'(?:REWRITE\("[^"]+", |^\w+\()(0x[0-9a-fA-F]+)', open(f, encoding='utf-8').read(), re.M)}
    cands = []
    for f in glob.glob(os.path.join(HERE, 'docs', 'notes', '*.md')):
        for line in open(f, encoding='utf-8'):
            m = re.match(r'\| `(0x[0-9a-f]+)` \| (Destructor|Destructor::Steps) \|', line)
            if m:
                cands.append(int(m.group(1), 16))
    found, why = [], collections.defaultdict(list)
    for a in sorted(set(cands)):
        r = size.get(a)
        if not r or a in taken or r['status'] != 'documented' and r['name'] != 'Destructor::Steps':
            continue
        n = int(r['size'])
        try:
            found.append((a, n, read(a, n)))
        except Unknown as e:
            why[str(e)].append((a, n))
        except (IndexError, KeyError):
            why['ran off the code'].append((a, n))
    lines = ['// Destructors taken over by dtor_steps (dtor_ops.cpp), as their steps in order; written by tools/dtor_ops.py.',
             '// A step: {kind, count or size, small value, a, b} (see dtor_ops.cpp)']
    for a, n, ops in found:
        first = ', '.join(f'0x{b:02x}' for b in img[a - BASE:a - BASE + 8])
        lines.append(f'static const uint8_t dtor_{a:x}_original[] = {{{first}}};')
        lines.append(f'static const DtorStep dtor_{a:x}_steps[] = {{' + ', '.join(f'{{{k}, {c}, {s:#x}, {x:#x}, {y:#x}}}' for k, c, s, x, y in ops) + '};')
        lines.append(f'REWRITE("Destructor::Steps", {a:#x}, dtor_{a:x}_original, (&dtor_steps<dtor_{a:x}_steps, {len(ops)}>), &g_dtor_steps);')
    lines.append('// for the tester: each address, how many steps, then the steps (five values each)')
    lines.append('extern const uint32_t dtor_steps_info[];')
    lines.append('const uint32_t dtor_steps_info[] = {' + ''.join(f'{a:#x}, {len(ops)}, ' + ''.join(f'{k}, {c}, {s:#x}, {x:#x}, {y:#x}, ' for k, c, s, x, y in ops)
                                                                for a, n, ops in found) + '0};')
    open(os.path.join(HERE, 'forever', 'src', 'rewrite', 'dtor_ops_list.inc'), 'w', newline='\n').write('\n'.join(lines) + '\n')
    print(len(found), 'functions,', sum(f[1] for f in found), 'bytes;', sum(len(v) for v in why.values()), 'left out,', sum(n for v in why.values() for a, n in v), 'bytes')
    if '--why' in sys.argv:
        for e, v in sorted(why.items(), key=lambda kv: -sum(n for a, n in kv[1]))[:40]:
            print(f'  {sum(n for a, n in v):6d} {len(v):4d} {hex(v[0][0])} {e}')


main()
