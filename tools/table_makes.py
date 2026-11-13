"""Read the "make with a shared table" constructors from the game's code and write them as rows of a table,
forever/src/rewrite/table_makes_list.inc. These functions all have one shape (the game's compiler wrote the same thing out
each time): keep some values from the arguments in the object, then, the first time (a bit in a global says whether
it's been done), fill a shared table through Table::Fill16 (0x416110) with sixteen two-byte values, and point the
object at the table. What differs is which values are kept and where, the global, the table and the sixteen values.
This reads exactly those facts from each function by following its straight-line code: constants put in registers and
stack bytes, the arguments, what is pushed, what is stored. A function with anything it doesn't know is left out (and
listed with --why). The equivalence tester checks every one against the game's own.
usage: table_makes.py <exe> [--why]"""
import csv, glob, os, re, sys
import capstone, pefile
from capstone import x86

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
pe = pefile.PE(sys.argv[1], fast_load=True); img = pe.get_memory_mapped_image(); BASE = 0x400000
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
md.detail = True
FILL16 = 0x416110
LOW = {'al': 'eax', 'bl': 'ebx', 'cl': 'ecx', 'dl': 'edx'}
WORD = {'ax': 'eax', 'bx': 'ebx', 'cx': 'ecx', 'dx': 'edx'}


class Unknown(Exception):
    pass


def read(a, n):
    """-> (keeps, made, table, words, pops) or raises Unknown(why)."""
    ins = list(md.disasm(bytes(img[a - BASE:a - BASE + n]), a))
    regs = {'ecx': ('this',)}          # register -> what it holds
    stack = {}                         # byte offset from the stack pointer at entry -> a constant byte
    wrote_args = set()                 # argument slots (dwords) overwritten since entry
    depth = 0                          # how far esp is below its value at entry
    keeps, pushes = [], []
    made = table = vtable = pops = None
    guard_seen = False

    def slot(op):   # a stack operand's offset from the entry esp
        return op.mem.disp - depth

    def value(name):
        return regs.get(name, ('?',))

    for x in ins:
        m, ops = x.mnemonic, x.operands
        if m == 'sub' and x.op_str.startswith('esp,'):
            depth += ops[1].imm
        elif m == 'add' and x.op_str.startswith('esp,'):
            depth -= ops[1].imm
        elif m == 'push':
            if ops[0].type != x86.X86_OP_REG:
                raise Unknown('push of a non-register')
            depth += 4
            pushes.append(value(x.reg_name(ops[0].reg)))
            for b in range(4):
                stack.pop(-depth + b, None)
        elif m == 'pop':
            depth -= 4
            regs[x.reg_name(ops[0].reg)] = ('?',)
        elif m == 'xor' and ops[0].type == x86.X86_OP_REG and ops[1].type == x86.X86_OP_REG and ops[0].reg == ops[1].reg:
            name = x.reg_name(ops[0].reg)
            if name in LOW:
                regs[LOW[name]] = ('byte', 0)
            else:
                regs[name] = ('c', 0)
        elif m == 'test':
            pass
        elif m == 'jne':
            guard_seen = True          # the skip when the table is already made: the code carries on in a line
        elif m == 'or' and ops[0].type == x86.X86_OP_REG and ops[1].type == x86.X86_OP_REG:
            d, s = value(x.reg_name(ops[0].reg)), value(x.reg_name(ops[1].reg))
            if d[0] == 'global' and s == ('c', 1):
                regs[x.reg_name(ops[0].reg)] = ('global|1', d[1])
            else:
                raise Unknown('or of ' + str((d, s)))
        elif m == 'lea':
            mem = ops[1].mem
            base = value(x.reg_name(mem.base)) if mem.base else None
            if mem.base == x86.X86_REG_ESP or mem.index:
                raise Unknown('lea of the stack')
            if base == ('this',):
                regs[x.reg_name(ops[0].reg)] = ('this+', mem.disp)
            elif base and base[0] == 'this+':
                regs[x.reg_name(ops[0].reg)] = ('this+', base[1] + mem.disp)
            else:
                raise Unknown('lea of ' + str(base))
        elif m == 'mov':
            d, s = ops
            if d.type == x86.X86_OP_REG:
                name = x.reg_name(d.reg)
                if s.type == x86.X86_OP_IMM:
                    if name in LOW:
                        regs[LOW[name]] = ('byte', s.imm & 0xff)
                    elif d.size == 4:
                        regs[name] = ('c', s.imm & 0xffffffff)
                    else:
                        raise Unknown('mov to ' + name)
                elif s.type == x86.X86_OP_REG:
                    if d.size != 4:
                        raise Unknown('mov between small registers')
                    regs[name] = value(x.reg_name(s.reg))
                else:   # from memory
                    mem = s.mem
                    if mem.index:
                        raise Unknown('indexed read')
                    if mem.base == x86.X86_REG_ESP:
                        if d.size != 4:
                            raise Unknown('small read of the stack')
                        o = slot(s)
                        if o >= 4 and o % 4 == 0 and o not in wrote_args and all(o + b not in stack for b in range(4)):
                            regs[name] = ('arg', o // 4 - 1)
                        elif o in stack and o + 1 in stack:
                            regs[name] = ('word', stack[o] | stack[o + 1] << 8)   # the two bytes Fill16 reads
                        else:
                            raise Unknown(f'read of stack {o:#x}')
                    elif mem.base == 0:   # a global
                        g = mem.disp & 0xffffffff
                        if name in LOW:
                            regs[LOW[name]] = ('global byte', g)
                        elif d.size == 4:
                            regs[name] = ('global', g)
                        else:
                            raise Unknown('small read of a global')
                    else:
                        base = value(x.reg_name(mem.base))
                        if d.size != 4 or base[0] not in ('arg',):
                            raise Unknown('read through ' + str(base))
                        regs[name] = ('at', base[1], mem.disp)
            else:   # to memory
                mem = d.mem
                if mem.index:
                    raise Unknown('indexed write')
                if s.type == x86.X86_OP_IMM:
                    v = ('c', s.imm & 0xffffffff)
                else:
                    name = x.reg_name(s.reg)
                    v = value(LOW.get(name, name))
                    if name in LOW and v[0] != 'byte':
                        raise Unknown('byte write of ' + str(v))
                if mem.base == x86.X86_REG_ESP:
                    if d.size != 1:
                        raise Unknown('wide write to the stack')
                    o = slot(d)
                    stack[o] = v[1] & 0xff
                    if o >= 4:
                        wrote_args.add(o & ~3)
                elif mem.base == 0:
                    g = mem.disp & 0xffffffff
                    if v != ('global|1', g) or d.size != 4:
                        raise Unknown('write to a global')
                    made = g
                else:
                    base = value(x.reg_name(mem.base))
                    if d.size != 4:
                        raise Unknown('small write to the object')
                    if base == ('this',):
                        off = mem.disp
                    elif base[0] == 'this+':
                        off = base[1] + mem.disp
                    else:
                        raise Unknown('write through ' + str(base))
                    if v[0] == 'c' and off == 0:
                        vtable = v[1]
                    elif v[0] == 'at':
                        keeps.append((off, v[1], v[2]))
                    else:
                        raise Unknown('keeps ' + str(v))
        elif m == 'call':
            if ops[0].type != x86.X86_OP_IMM or ops[0].imm != FILL16:
                raise Unknown('another call')
            if value('ecx')[0] != 'c':
                raise Unknown('the table is not a constant')
            table = value('ecx')[1]
            words = pushes[-16:][::-1]   # the last pushed is the first argument
            if len(words) != 16 or any(w[0] != 'word' for w in words):
                raise Unknown('the sixteen values: ' + str(words))
            depth -= 0x40
            regs = {k: v for k, v in regs.items() if k in ('ebx', 'esi', 'edi', 'ebp')}   # what a call keeps
        elif m == 'rep movsd':   # a block of values kept: count in ecx, from an argument's pointer (esi) to the object (edi)
            n_, src, dst = value('ecx'), value('esi'), value('edi')
            if n_[0] != 'c' or src[0] != 'arg' or dst[0] != 'this+' or n_[1] > 64:
                raise Unknown('a block copy of ' + str((n_, src, dst)))
            keeps += [(dst[1] + 4 * k, src[1], 4 * k) for k in range(n_[1])]
            regs['ecx'], regs['esi'], regs['edi'] = ('c', 0), ('?',), ('?',)
        elif m == 'ret':
            pops = ops[0].imm if ops else 0
        else:
            raise Unknown(m + ' ' + x.op_str)
    if value('eax') != ('this',):
        raise Unknown('does not return this')
    if None in (made, table, pops) or vtable != table:
        raise Unknown('no flag, table or return')
    return keeps, made, table, [w[1] for w in words], pops


rows = list(csv.DictReader(open(os.path.join(HERE, 'docs', 'functions.csv'), encoding='utf-8')))
TAKEN = set()
for f in glob.glob(os.path.join(HERE, 'forever', 'src', 'rewrite', '*.cpp')):
    TAKEN |= {int(v, 16) for v in re.findall(r'REWRITE\("[^"]+", (0x[0-9a-fA-F]+)', open(f, encoding='utf-8').read())}
OUT = os.path.join(HERE, 'forever', 'src', 'rewrite', 'table_makes_list.inc')
found, why = [], []
for r in rows:
    a, n = int(r['address'], 16), int(r['size'])
    if r['status'] != 'documented' and r['name'] != 'Table::Make' or a in TAKEN or n < 100 or n > 2000:
        continue
    code = bytes(img[a - BASE:a - BASE + n])
    if code.count(b'\xe8') == 0:
        continue
    ins = list(md.disasm(code, a))
    calls = [x for x in ins if x.mnemonic == 'call']
    if len(calls) != 1 or calls[0].operands[0].type != x86.X86_OP_IMM or calls[0].operands[0].imm != FILL16:
        continue
    try:
        keeps, made, table, words, pops = read(a, n)
        if pops != 0x40:
            raise Unknown(f'takes {pops:#x} bytes off the stack')
        if len(keeps) > 40:
            raise Unknown('keeps more than 40 values')
        found.append((a, n, keeps, made, table, words))
    except Unknown as e:
        why.append((a, n, str(e)))
lines = ['// The "make with a shared table" constructors taken over by table_make (table_makes.cpp); written by tools/table_makes.py.',
         '// Each: its first bytes, the sixteen values for the table, the values kept {to, argument, from}, then the rewrite']
for a, n, keeps, made, table, words in found:
    first = ', '.join(f'0x{b:02x}' for b in img[a - BASE:a - BASE + 14])
    ks = ', '.join(f'{{{to:#x}, {arg}, {frm:#x}}}' for to, arg, frm in keeps)
    lines.append(f'static const uint8_t table_make_{a:x}_original[] = {{{first}}};')
    lines.append(f'static const uint16_t table_make_{a:x}_words[16] = {{{", ".join(f"{w:#x}" for w in words)}}};')
    lines.append(f'static const TableKeep table_make_{a:x}_keeps[] = {{{ks}}};')
    lines.append(f'REWRITE("Table::Make", {a:#x}, table_make_{a:x}_original, (&table_make<{made:#x}, {table:#x}, table_make_{a:x}_words, table_make_{a:x}_keeps, {len(keeps)}>), &g_table_makes);')
lines.append('// for the tester: each address and its global')
lines.append('extern const uint32_t table_make_info[];')
lines.append('const uint32_t table_make_info[] = {' + ''.join(f'{a:#x}, {made:#x}, ' for a, n, keeps, made, table, words in found) + '0};')
open(OUT, 'w', newline='\n').write('\n'.join(lines) + '\n')
print(len(found), 'functions,', sum(f[1] for f in found), 'bytes;', len(why), 'left out')
if '--why' in sys.argv:
    for a, n, e in why:
        print(f'  {a:#x} {n}: {e}')
