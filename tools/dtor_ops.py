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
SET, IF, VCALL, CDECL, CALL, GCOPY, SETM, REL, FREEM, TEXTREL, SETP, CALLP, VECDEL, LOOP, GSET, IFB, GMOVE, ARG, GCALL, UNLINK, MCALL, GIF, IFNEG1 = range(23)   # the step kinds (dtor_ops.cpp)
FREE_ALIGNED, FREE, VEC_DTOR = 0xbce010, 0xbce000, 0xddcf39


class Unknown(Exception):
    pass


def is_mem(op, base=None):
    return op.type == x86.X86_OP_MEM


def read(a, n):
    # the recorded size of some stops a few bytes short of the end (inside the frame being put back): read on to the ret
    ins = []
    for x in md.disasm(bytes(img[a - BASE:a - BASE + n + 24]), a):
        ins.append(x)
        if x.address + x.size >= a + n:
            if x.mnemonic == 'ret' or x.address >= a + n + 16:
                break
    if not ins or ins[-1].mnemonic != 'ret' or ins[-1].op_str:
        raise Unknown('does not end in a plain ret')
    at = {x.address: k for k, x in enumerate(ins)}
    regs = {'ecx': ('this', 0)}        # register -> ('this', offset) | ('member', offset of the field it was read from) | ('global', address)
    ops = []
    started = False
    pending = []                       # arguments pushed for the next call: ('c', constant) or ('f', field offset)
    frees = [0]                        # frees whose argument is still on the stack
    odd_adds = []                      # stack adjustments that aren't those: fine only as the frame's release at the end

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
        if x.mnemonic == 'sub' and x.op_str.startswith('esp,'):
            return True
        if x.mnemonic == 'add' and x.op_str.startswith('esp,'):
            if o[1].imm == 4 * frees[0] and frees[0]:
                frees[0] = 0
            else:
                odd_adds.append(x.address)
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
            # a value put in the member: mov [member + d], constant
            if x.mnemonic == 'mov' and is_mem(o[0]) and o[1].type == x86.X86_OP_IMM and o[0].size == 4 and not o[0].mem.index \
                    and o[0].mem.base and ins[0].reg_name(o[0].mem.base) == member and 0 <= o[0].mem.disp < 0x400:
                out.append((SETP, 0, 0, o[0].mem.disp, o[1].imm & 0xffffffff))
                i += 1
                continue
            # its destructor called directly: (mov ecx, member;) call fn
            j = i + 1 if x.mnemonic == 'mov' and x.op_str == f'ecx, {member}' else i
            if (j > i or member == 'ecx') and ins[j].mnemonic == 'call' and ins[j].operands[0].type == x86.X86_OP_IMM:
                out.append((CALLP, 0, 0, ins[j].operands[0].imm, 0))
                for r in ('eax', 'ecx', 'edx'):
                    regs[r] = None
                if member in ('eax', 'ecx', 'edx'):
                    member = None
                i = j + 1
                continue
            # an array of records deleted: mov r, [member - 4]; lea r2, [member - 4]; push dtor; push r; push size; push member;
            # call VecDtor; push r2; call FreeAligned; add esp, 4
            if x.mnemonic == 'mov' and o[0].type == x86.X86_OP_REG and is_mem(o[1]) and not o[1].mem.index and o[1].mem.disp == -4 \
                    and o[1].mem.base and ins[0].reg_name(o[1].mem.base) == member:
                q = ins[i + 1:i + 11]
                r1 = reg(o[0])
                extra = 0
                if q[0].mnemonic == 'push' and q[0].operands[0].type == x86.X86_OP_REG and reg(q[0].operands[0]) in SAVED and reg(q[0].operands[0]) != member:
                    q, extra = q[1:], 1                       # a register saved for the block's address
                if (q[0].mnemonic == 'lea' and is_mem(q[0].operands[1]) and q[0].operands[1].mem.disp == -4 and ins[0].reg_name(q[0].operands[1].mem.base) == member
                        and q[1].mnemonic == 'push' and q[1].operands[0].type == x86.X86_OP_IMM
                        and q[2].mnemonic == 'push' and q[2].op_str == r1
                        and q[3].mnemonic == 'push' and q[3].operands[0].type == x86.X86_OP_IMM
                        and q[4].mnemonic == 'push' and q[4].op_str == member
                        and q[5].mnemonic == 'call' and q[5].operands[0].type == x86.X86_OP_IMM and q[5].operands[0].imm == VEC_DTOR
                        and q[6].mnemonic == 'push' and q[6].op_str == reg(q[0].operands[0])
                        and q[7].mnemonic == 'call' and q[7].operands[0].type == x86.X86_OP_IMM and q[7].operands[0].imm == FREE_ALIGNED
                        and q[8].mnemonic == 'add' and q[8].op_str == 'esp, 4'):
                    out.append((VECDEL, 0, 0, q[3].operands[0].imm, q[1].operands[0].imm))
                    for r in ('eax', 'ecx', 'edx'):
                        regs[r] = None
                    member = None
                    i += 10 + extra
                    continue
                raise Unknown('an array delete of another shape')
            # free its memory: push member; (a value put in it); call fn; add esp, 4
            if x.mnemonic == 'push' and o[0].type == x86.X86_OP_REG and reg(o[0]) == member:
                w = ins[i + 1]
                if w.mnemonic == 'mov' and is_mem(w.operands[0]) and w.operands[1].type == x86.X86_OP_IMM and w.operands[0].size == 4 \
                        and not w.operands[0].mem.index and w.operands[0].mem.base and ins[0].reg_name(w.operands[0].mem.base) == member \
                        and 0 <= w.operands[0].mem.disp < 0x400:
                    out.append((SETP, 0, 0, w.operands[0].mem.disp, w.operands[1].imm & 0xffffffff))
                    i += 1
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

    def is_null_check(x, r):
        zero = x.mnemonic == 'cmp' and x.operands[0].type == x86.X86_OP_REG and reg(x.operands[0]) == r \
            and x.operands[1].type == x86.X86_OP_REG and regs.get(reg(x.operands[1])) == ('zero', 0)
        return x.mnemonic == 'test' and x.op_str == f'{r}, {r}' or zero

    def loop(j, end, count_off):
        """for each pointer in a member array (while the index is under the count at count_off): the steps on each one that is there.
        (push saved;) top: mov a, [this + array]; mov p, [a + index*4]; null check; je next; steps; next: mov c, [this + count]; inc index;
        cmp index, c; jl top; (pop saved;) end:"""
        if ins[j].mnemonic == 'push' and ins[j].operands[0].type == x86.X86_OP_REG and reg(ins[j].operands[0]) in SAVED:
            j += 1
        def padding(x):   # what the compiler puts before a loop's top to line it up
            o = x.operands
            return x.mnemonic == 'nop' or x.mnemonic == 'mov' and len(o) == 2 and o[0].type == x86.X86_OP_REG and o[1].type == x86.X86_OP_REG and o[0].reg == o[1].reg                 or x.mnemonic == 'lea' and not o[1].mem.index and o[1].mem.disp == 0 and o[1].mem.base == o[0].reg
        while padding(ins[j]):
            j += 1
        top = ins[j].address
        x, y = ins[j], ins[j + 1]
        if not (x.mnemonic == 'mov' and x.operands[0].type == x86.X86_OP_REG and is_mem(x.operands[1]) and this_off(x.operands[1]) is not None):
            raise Unknown('a loop of another shape')
        array_off, areg = this_off(x.operands[1]), reg(x.operands[0])
        m = y.operands[1].mem if y.mnemonic == 'mov' and len(y.operands) == 2 and is_mem(y.operands[1]) else None
        if not (m is not None and y.operands[0].type == x86.X86_OP_REG and m.base and ins[0].reg_name(m.base) == areg and m.index and m.scale == 4 and m.disp == 0):
            raise Unknown('a loop of another shape')
        index, p = ins[0].reg_name(m.index), reg(y.operands[0])
        if regs.get(index) != ('zero', 0) or index == p or index not in SAVED:
            raise Unknown('a loop whose index is not a kept zero register')
        k = j + 2
        if not is_null_check(ins[k], p) or ins[k + 1].mnemonic != 'je':
            raise Unknown('a loop of another shape')
        nxt = ins[k + 1].operands[0].imm
        regs[index] = ('index', 0)
        regs[areg] = None
        inner = block(k + 2, nxt, p)
        t = at[nxt]
        q = ins[t:t + 4]
        if not (q[0].mnemonic == 'mov' and q[0].operands[0].type == x86.X86_OP_REG and is_mem(q[0].operands[1]) and this_off(q[0].operands[1]) == count_off
                and q[1].mnemonic == 'inc' and q[1].op_str == index
                and q[2].mnemonic == 'cmp' and q[2].op_str == f'{index}, {reg(q[0].operands[0])}'
                and q[3].mnemonic == 'jl' and q[3].operands[0].imm == top):
            raise Unknown('a loop that ends another way')
        t += 4
        while ins[t].address < end:
            if ins[t].mnemonic != 'pop':
                raise Unknown('a loop that ends another way')
            regs[reg(ins[t].operands[0])] = None
            t += 1
        if ins[t].address != end or any(k_[0] == LOOP for k_ in inner) or len(inner) > 40:
            raise Unknown('a loop that ends another way')
        for r in ('eax', 'ecx', 'edx', index, p):
            regs[r] = None
        return t, [(LOOP, 0, len(inner), array_off, count_off)] + inner

    def step(i):
        """one step at ins[i] outside any null check -> (next index, steps) or None"""
        x, o = ins[i], ins[i].operands
        m = x.mnemonic
        if m == 'xor' and o[0].type == x86.X86_OP_REG and o[1].type == x86.X86_OP_REG and o[0].reg == o[1].reg and o[0].size == 4:
            regs[reg(o[0])] = ('zero', 0)
            return i + 1, []
        if m == 'mov' and len(o) == 2:
            # a global copied to another: mov [g0], r with r holding a global's value
            if is_mem(o[0]) and o[1].type == x86.X86_OP_REG and o[1].size == 4 and not o[0].mem.base and not o[0].mem.index and not o[0].mem.segment \
                    and (regs.get(reg(o[1])) or ('', 0))[0] == 'gvalue' and (o[0].mem.disp & 0xffffffff) >= 0x1000000:
                return i + 1, [(GMOVE, 0, 0, o[0].mem.disp & 0xffffffff, regs[reg(o[1])][1])]
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
                if not o[0].mem.base and not o[0].mem.index and not o[0].mem.segment and (o[0].mem.disp & 0xffffffff) >= 0x1000000:
                    return i + 1, [(GSET, o[0].size, 0, o[0].mem.disp & 0xffffffff, o[1].imm & 0xffffffff)]   # a global
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
                    while True:
                        y = ins[j]
                        if y.mnemonic == 'xor' and y.operands[0].type == x86.X86_OP_REG and y.operands[1].type == x86.X86_OP_REG \
                                and y.operands[0].reg == y.operands[1].reg and reg(y.operands[0]) != r and y.operands[0].size == 4:
                            regs[reg(y.operands[0])] = ('zero', 0)   # a zero register made in between
                        elif y.mnemonic == 'push' and y.operands[0].type == x86.X86_OP_REG and reg(y.operands[0]) in SAVED and reg(y.operands[0]) != r \
                                and regs.get(reg(y.operands[0])) is None and not pending:
                            pass                                     # a register saved in between
                        elif y.mnemonic == 'add' and y.op_str.startswith('esp,') and noise(y):
                            pass                                     # the stack put right in between
                        else:
                            break
                        j += 1
                    t = text_release(i, r, off) if j == i + 1 else None
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
                            frees[0] += 1
                            return k + 1, [(FREEM, 0, 0, off, ins[k].operands[0].imm)]
                        raise Unknown('a member pushed for a call with more arguments')
                    zero = ins[j].mnemonic == 'cmp' and ins[j].operands[0].type == x86.X86_OP_REG and reg(ins[j].operands[0]) == r \
                        and ins[j].operands[1].type == x86.X86_OP_REG and regs.get(reg(ins[j].operands[1])) == ('zero', 0)
                    if ins[j].mnemonic == 'test' and ins[j].op_str == f'{r}, {r}' or zero:
                        j += 1
                        k = j
                        while noise(ins[k]):
                            k += 1
                        if ins[k].mnemonic == 'jle' and ins[k].operands[0].imm > ins[k].address:   # a count: a loop over a member array
                            return loop(k + 1, ins[k].operands[0].imm, off)
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
                    if ins[j].mnemonic == 'cmp' and ins[j].op_str == f'{r}, -1':
                        k = j + 1
                        while noise(ins[k]):
                            k += 1
                        if ins[k].mnemonic == 'je' and ins[k].operands[0].imm > ins[k].address:   # steps unless the field is -1
                            end = ins[k].operands[0].imm
                            inner = block(k + 1, end, None)
                            return at[end], [(IFNEG1, 0, 0, off, len(inner))] + inner
                        raise Unknown('a field compared with -1 for something else')
                    return i + 1, []
                if off is not None and o[0].size == 1:
                    # a flag: mov r8, [this + off]; test r8, r8; (noise); jne / je end; steps; end:
                    j = i + 1
                    if ins[j].mnemonic == 'test' and ins[j].op_str == f'{r}, {r}':
                        j += 1
                        while noise(ins[j]):
                            j += 1
                        if ins[j].mnemonic in ('jne', 'je') and ins[j].operands[0].imm > ins[j].address:
                            end = ins[j].operands[0].imm
                            regs[{'al': 'eax', 'bl': 'ebx', 'cl': 'ecx', 'dl': 'edx'}.get(r, r)] = None
                            inner = block(j + 1, end, None)
                            return at[end], [(IFB, 1 if ins[j].mnemonic == 'jne' else 0, 0, off, len(inner))] + inner
                    return None
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
                    if g1 >= 0x1000000:
                        if is_null_check(ins[j], r):
                            k = j + 1
                            before = []
                            while ins[k].mnemonic == 'mov':
                                if noise(ins[k]):
                                    k += 1
                                    continue
                                st = step(k)
                                if not st or any(q[0] not in (SET, GSET) for q in st[1]):
                                    break
                                before += st[1]
                                k = st[0]
                            if ins[k].mnemonic == 'je' and ins[k].operands[0].imm > ins[k].address:
                                end = ins[k].operands[0].imm
                                regs[r] = ('gmember', g1)
                                inner = block(k + 1, end, r)
                                regs[r] = None
                                return at[end], before + [(GIF, 0, 0, g1, len(inner))] + inner
                            raise Unknown('a global tested for something else')
                        regs[r] = ('gvalue', g1)              # a global's value: an object to call, or to copy on
                        return i + 1, []
                    return None
                return None
        if m == 'lea' and not o[1].mem.index and o[1].mem.base:
            b = regs.get(ins[0].reg_name(o[1].mem.base))
            if b and b[0] == 'this':
                regs[reg(o[0])] = ('this', b[1] + o[1].mem.disp)
                return i + 1, []
            return None
        if m == 'test' and o[0].type == x86.X86_OP_REG and o[1].type == x86.X86_OP_REG and o[0].reg == o[1].reg \
                and (regs.get(reg(o[0])) or ('', 0))[0] == 'this':
            # a list node in the object taken out of its list: the node {next, previous} at this + off, checked against its
            # neighbours; a node that isn't linked both ways runs into a breakpoint (int3), as in the game
            n, off = reg(o[0]), regs[reg(o[0])][1]
            # a base's part of the object: test this, this; (noise); je zero; lea r, [this + off]; jmp on; zero: xor r, r; on:
            # (the compiler's "this + off, or null for a null this"; this is never null in a destructor, so it is this + off)
            j = i + 1
            while noise(ins[j]):
                j += 1
            q = ins[j:j + 4]
            if len(q) == 4 and q[0].mnemonic == 'je' and q[1].mnemonic == 'lea' and q[2].mnemonic == 'jmp' and q[3].mnemonic == 'xor'                     and not q[1].operands[1].mem.index and ins[0].reg_name(q[1].operands[1].mem.base) == n                     and q[0].operands[0].imm == q[3].address and q[2].operands[0].imm == q[3].address + q[3].size                     and q[3].operands[0].reg == q[3].operands[1].reg == q[1].operands[0].reg:
                regs[reg(q[1].operands[0])] = ('this', off + q[1].operands[1].mem.disp)
                return j + 4, []
            j, before = i + 1, []
            while ins[j].mnemonic == 'mov':
                if noise(ins[j]):
                    j += 1
                    continue
                st = step(j)
                if not st or any(k[0] != SET for k in st[1]):
                    break
                before += st[1]
                j = st[0]
            q = ins[j:j + 19]
            if len(q) == 19 and q[0].mnemonic == 'je' and q[1].mnemonic == 'mov' and q[1].operands[0].type == x86.X86_OP_REG \
                    and q[4].mnemonic == 'mov' and q[4].operands[0].type == x86.X86_OP_REG:
                end, b, a = q[0].operands[0].imm, reg(q[1].operands[0]), reg(q[4].operands[0])
                want = [('je', None), ('mov', f'{b}, dword ptr [{n} + 4]'), ('test', f'{b}, {b}'), ('je', None), ('mov', f'{a}, dword ptr [{n}]'),
                        ('test', f'{a}, {a}'), ('je', None), ('cmp', f'dword ptr [{a} + 4], {n}'), ('jne', None), ('cmp', f'dword ptr [{b}], {n}'), ('jne', None),
                        ('mov', f'dword ptr [{a} + 4], {b}'), ('mov', f'{a}, dword ptr [{n} + 4]'), ('mov', f'{b}, dword ptr [{n}]'), ('mov', f'dword ptr [{a}], {b}'),
                        ('mov', f'dword ptr [{n}], 0'), ('mov', f'dword ptr [{n} + 4], 0'), ('jmp', None), ('int3', '')]
                ok = all(x.mnemonic == m_ and (t is None and x.operands[0].imm == (end if m_ != 'jmp' else q[18].address + 1) or t is not None and x.op_str == t)
                         for x, (m_, t) in zip(q, want))
                if ok and q[18].address == end and len({a, b, n}) == 3:
                    for r in (a, b, n):
                        regs[r] = None
                    return at[end + 1], before + [(UNLINK, 0, 0, off, 0)]
            raise Unknown('a list node taken out another way')
        if m == 'push':
            if o[0].type == x86.X86_OP_IMM:
                pending.append(('c', o[0].imm & 0xffffffff))
                return i + 1, []
            v = regs.get(reg(o[0]))
            if v and v[0] == 'member':
                pending.append(('f', v[1]))                   # a field's value
                return i + 1, []
            if v == ('zero', 0):
                pending.append(('c', 0))
                return i + 1, []
            if v is None and reg(o[0]) in SAVED:
                return i + 1, []                              # a register saved part way through
            return None
        if m == 'call' and o[0].type == x86.X86_OP_IMM:
            c = regs.get('ecx')
            if c and c[0] in ('this', 'gvalue') and len(pending) <= 3:
                args = [(ARG, 1 if k == 'f' else 0, 0, v, 0) for k, v in reversed(pending)]
                del pending[:]
                for r in ('eax', 'ecx', 'edx'):
                    regs[r] = None
                if c[0] == 'this':
                    return i + 1, args + [(CALL, len(args), 0, o[0].imm, c[1])]   # a member's or the base's destructor
                return i + 1, args + [(GCALL, len(args), 0, o[0].imm, c[1])]      # on a global object
            if c and c[0] == 'member' and len(pending) <= 3:
                args = [(ARG, 1 if k == 'f' else 0, 0, v, 0) for k, v in reversed(pending)]
                del pending[:]
                for r in ('eax', 'ecx', 'edx'):
                    regs[r] = None
                return i + 1, args + [(MCALL, len(args), 0, o[0].imm, c[1])]      # on the object a member points to, whatever it is
            return None
        return None

    i = 0
    while i < len(ins) - 1:
        x = ins[i]
        if noise(x):
            i += 1
            continue
        if x.mnemonic == 'push' and not started and not pending and (
                x.operands[0].type == x86.X86_OP_REG and (regs.get(reg(x.operands[0])) or ('this',))[0] == 'this' or x.operands[0].type != x86.X86_OP_REG and i < 3):
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
    if len(ops) > 200:
        raise Unknown('more than 200 steps')
    if pending or frees[0]:
        raise Unknown('arguments left on the stack')
    last_call = max((x.address for x in ins if x.mnemonic == 'call'), default=0)
    if any(ad < last_call for ad in odd_adds):
        raise Unknown('the stack put right by another amount')
    if len({x for k, c, sm, x, y in ops if k in (CALL, CDECL, CALLP, GCALL, MCALL)} | {y for k, c, sm, x, y in ops if k == FREEM}) > 40:
        raise Unknown('more than 40 functions called')
    for k, c, small, x, y in ops:   # what the tester's stand-ins hold: table entries under 16, offsets inside its objects
        if k in (VCALL,) and not 0 <= small < 0x40 or k == REL and not (0 <= x < 0x40 and 0 <= small < 0x3c):
            raise Unknown('a table entry past the 16th or a far count')
        if k in (MCALL,) and not 0 <= y < 0x7ff0 or k == IFNEG1 and not 0 <= x < 0x7ff0 or k == UNLINK and not 0 <= x < 0x7ff0 or k == IFB and not 0 <= x < 0x7ff0 or k == ARG and c == 1 and not 0 <= x < 0x7ff0 or k == LOOP and not (0 <= x < 0x7ff0 and 0 <= y < 0x7ff0) or k in (SET, IF, FREEM, TEXTREL) and not 0 <= x < 0x7ff0 or k == VECDEL and not 0 < x < 0x10000 or k == SETM and not (0 <= x < 0x7f0 and small < 0x7ff0) or k == CALL and not 0 <= y < 0x8000:
            raise Unknown('a field past 0x7FF0')
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
