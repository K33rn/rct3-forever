"""Find the compiler's field-by-field copy functions (`dst = src`, returning dst): every instruction loads a field of the
source and stores it to the same field of the destination. They are written out as the byte ranges they copy, in
forever/src/rewrite/copies.cpp. Anything else in a function (calls, branches, clearing, a field stored somewhere else)
rejects it. usage: copy_funcs.py <exe>"""
import csv, os, sys
import capstone, pefile
from capstone import x86

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
pe = pefile.PE(sys.argv[1], fast_load=True); img = pe.get_memory_mapped_image(); BASE = 0x400000
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32); md.detail = True
PARENT = {}
for full, parts in {x86.X86_REG_EAX: (x86.X86_REG_AX, x86.X86_REG_AL, x86.X86_REG_AH), x86.X86_REG_EBX: (x86.X86_REG_BX, x86.X86_REG_BL, x86.X86_REG_BH),
                    x86.X86_REG_ECX: (x86.X86_REG_CX, x86.X86_REG_CL, x86.X86_REG_CH), x86.X86_REG_EDX: (x86.X86_REG_DX, x86.X86_REG_DL, x86.X86_REG_DH),
                    x86.X86_REG_ESI: (x86.X86_REG_SI,), x86.X86_REG_EDI: (x86.X86_REG_DI,), x86.X86_REG_EBP: (x86.X86_REG_BP,)}.items():
    PARENT[full] = full
    for p in parts:
        PARENT[p] = full


SIZES = {}
_plain = {}


def plain_copy(a):
    if a not in _plain:
        _plain[a] = False
        c = analyse(a, SIZES[a]) if a in SIZES else None
        _plain[a] = bool(c) and all(len(x) == 2 for x in c)
    return _plain[a]


def analyse(a, n):
    regs = {x86.X86_REG_ECX: ('dst', 0)}
    depth = 0          # bytes pushed since entry
    pushed = []        # what each push put on the stack (None: a register saved for later)
    copied = []
    uses_src = False
    ins = list(md.disasm(bytes(img[a - BASE:a - BASE + n]), a))
    if not ins or ins[-1].address + ins[-1].size != a + n:
        return None
    for i in ins[:-1]:
        ops = i.operands
        if i.mnemonic == 'push' and ops[0].type == x86.X86_OP_REG:
            depth += 4
            pushed.append(regs.get(PARENT.get(ops[0].reg)))
        elif i.mnemonic == 'call' and ops[0].type == x86.X86_OP_IMM:
            # an assign of one member: Member(dst + o) = Member(src + o), the callee taking its source off the stack
            arg = pushed.pop() if pushed else None
            this = regs.get(x86.X86_REG_ECX)
            if not arg or not this or arg[0] != 'src' or this[0] != 'dst' or arg[1] != this[1]:
                return None
            if not plain_copy(ops[0].imm):   # only members that are copied field by field themselves
                return None
            depth -= 4
            copied.append(('call', this[1], ops[0].imm))
            for r in (x86.X86_REG_EAX, x86.X86_REG_ECX, x86.X86_REG_EDX):
                regs.pop(r, None)
        elif i.mnemonic == 'pop' and ops[0].type == x86.X86_OP_REG:
            depth -= 4
            if pushed:
                pushed.pop()
            regs.pop(PARENT.get(ops[0].reg, ops[0].reg), None)
        elif i.mnemonic == 'xor' and ops[0].type == ops[1].type == x86.X86_OP_REG and ops[0].reg == ops[1].reg and ops[0].size == 4:
            if PARENT.get(ops[0].reg) is None:
                return None
            regs[PARENT[ops[0].reg]] = ('const', 0)
        elif i.mnemonic == 'or' and ops[0].type == x86.X86_OP_REG and ops[1].type == x86.X86_OP_IMM and ops[1].imm == -1 and ops[0].size == 4:
            if PARENT.get(ops[0].reg) is None:
                return None
            regs[PARENT[ops[0].reg]] = ('const', 0xffffffff)
        elif i.mnemonic == 'mov' and ops[0].type == x86.X86_OP_REG and ops[1].type == x86.X86_OP_IMM and ops[0].size == 4:
            if PARENT.get(ops[0].reg) is None:
                return None
            regs[PARENT[ops[0].reg]] = ('const', ops[1].imm & 0xffffffff)
        elif i.mnemonic in ('mov', 'lea') and ops[0].type == x86.X86_OP_REG:
            d = PARENT.get(ops[0].reg)
            if d is None:
                return None
            s = ops[1]
            if i.mnemonic == 'mov' and s.type == x86.X86_OP_REG:
                v = regs.get(PARENT.get(s.reg))
                if v is None:
                    return None
                regs[d] = v
            elif s.type == x86.X86_OP_MEM and s.mem.index == 0 and s.mem.segment == 0:
                base = PARENT.get(s.mem.base) if s.mem.base != x86.X86_REG_ESP else 'esp'
                disp = s.mem.disp
                if base == 'esp':
                    if i.mnemonic == 'mov' and disp - depth == 4 and ops[0].size == 4:
                        regs[d] = ('src', 0)
                        uses_src = True
                    else:
                        return None
                else:
                    bv = regs.get(base)
                    if not bv or bv[0] not in ('src', 'dst'):
                        return None
                    if i.mnemonic == 'lea':
                        regs[d] = (bv[0], bv[1] + disp)
                    elif bv[0] == 'src':
                        regs[d] = ('field', bv[1] + disp, ops[0].size)
                    else:
                        return None
            else:
                return None
        elif i.mnemonic == 'mov' and ops[0].type == x86.X86_OP_MEM and ops[1].type in (x86.X86_OP_REG, x86.X86_OP_IMM):
            m = ops[0].mem
            if m.index or m.segment or m.base == x86.X86_REG_ESP:
                return None
            bv = regs.get(PARENT.get(m.base))
            if not bv or bv[0] not in ('src', 'dst'):
                return None
            off, size = bv[1] + m.disp, ops[0].size
            if ops[1].type == x86.X86_OP_IMM:
                v = ('const', ops[1].imm & 0xffffffff)
            else:
                v = regs.get(PARENT.get(ops[1].reg))
                if v and v[0] == 'const' and ops[1].size != 4:
                    v = ('const', (v[1] >> (8 if ops[1].reg in (x86.X86_REG_AH, x86.X86_REG_BH, x86.X86_REG_CH, x86.X86_REG_DH) else 0)) & ((1 << 8 * ops[1].size) - 1))
            if v and v[0] == 'dst' and bv[0] == 'dst' and size == 4:
                copied.append(('set', 'self', off, 4, v[1]))
                continue
            if not v or v[0] not in ('field', 'const'):
                return None
            if v[0] == 'field':
                if bv[0] != 'dst' or v[1] != off or v[2] != size:
                    return None
                copied.append((off, size))
            else:
                if bv[0] == 'src':   # a source field read but not yet stored must not change under it
                    for w in regs.values():
                        if w[0] == 'field' and w[1] < off + size and off < w[1] + w[2] and (w[1], w[2]) not in copied:
                            return None
                if any(c[0] == 'set' and c[1] == bv[0] and c[2] < off + size and off < c[2] + c[3] for c in copied if len(c) == 5):
                    return None
                copied.append(('set', bv[0], off, size, v[1] & ((1 << 8 * size) - 1)))
                if bv[0] == 'src' and any(len(c) == 2 and c[0] < off + size and off < c[0] + c[1] for c in copied):
                    pass   # copied before it was cleared: fine, the order is kept
        elif i.mnemonic == 'nop':
            pass
        else:
            return None
    last = ins[-1]
    args = last.operands[0].imm if last.operands else 0
    if last.mnemonic != 'ret' or args not in (0, 4) or (uses_src and args != 4) or depth != 0 or not copied:
        return None
    ret = regs.get(x86.X86_REG_EAX)
    if not ret or ret[0] not in ('dst', 'const'):
        return None
    if args == 4 and ret == ('dst', 0) and all(len(c) == 2 for c in copied):
        return copied
    return copied + [('ret', args, ret)]


rows = list(csv.DictReader(open(os.path.join(HERE, 'docs', 'functions.csv'), encoding='utf-8')))
SIZES.update({int(r['address'], 16): int(r['size']) for r in rows})
import glob, re
taken = set()
for f in glob.glob(os.path.join(HERE, 'forever', 'src', 'rewrite', '*.*')):
    if not f.endswith('copies.cpp'):
        taken |= {int(x, 16) for x in re.findall(r'(?:REWRITE\("[^"]+",|^[A-Z_0-9]+\()\s*(0x[0-9a-fA-F]+)', open(f, encoding='utf-8').read(), re.M)}
mine = set()
if os.path.exists(os.path.join(HERE, 'forever', 'src', 'rewrite', 'copies.cpp')):   # the copies found before
    mine = {int(x, 16) for x in re.findall(r'^REWRITE\("[^"]+", (0x[0-9a-f]+)', open(os.path.join(HERE, 'forever', 'src', 'rewrite', 'copies.cpp')).read(), re.M)}
found = []
found_sets = []
for r in rows:
    if (r['status'] != 'documented' and int(r['address'], 16) not in mine) or int(r['size']) < 24 or int(r['address'], 16) in taken:
        continue
    a, n = int(r['address'], 16), int(r['size'])
    c = analyse(a, n)
    sets = [x for x in c if len(x) != 2] if c else []
    if c and sets:
        found_sets.append((a, n, c))
        continue
    if c and max(o + s for o, s in c) <= 0x1000 and min(o for o, s in c) >= 0:
        cover = sorted({b for o, s in c for b in range(o, o + s)})
        ranges = []
        for b in cover:
            if ranges and ranges[-1][1] == b:
                ranges[-1][1] = b + 1
            else:
                ranges.append([b, b + 1])
        found.append((a, n, ranges))
out = ['// Field-by-field copies the compiler made (`dst = src`, returning dst), found by tools/copy_funcs.py: each is the byte',
       '// ranges of its record that it copies, padding and fields it leaves alone skipped. Do not edit by hand.',
       '#include "rewrite.h"', '', 'struct CopyRange {', '    uint16_t start, end;', '};',
       'static volatile long g_field_copies;',
       'template <const CopyRange *RANGES, uint32_t COUNT> static void *__thiscall copy_fields(uint8_t *dst, const uint8_t *src) {',
       '    g_field_copies++;',
       '    for (uint32_t k = 0; k < COUNT; k++) __builtin_memmove(dst + RANGES[k].start, src + RANGES[k].start, RANGES[k].end - RANGES[k].start);',
       '    return dst;', '}', '']
for a, n, ranges in found:
    out.append(f'static const CopyRange copy_{a:x}[] = {{' + ', '.join(f'{{{s:#x}, {e:#x}}}' for s, e in ranges) + '};')
    out.append(f'static const uint8_t copy_{a:x}_original[] = {{' + ', '.join(f'0x{b:02x}' for b in img[a - BASE:a - BASE + 8]) + '};')
    out.append(f'REWRITE("Record::CopyFields", {a:#x}, copy_{a:x}_original, (&copy_fields<copy_{a:x}, {len(ranges)}>), &g_field_copies);')
out += ['', '// Copies that also set fields to fixed values (in either record), done in the original order: kind 0 copies [off, off + len)',
        '// from the source, 1 sets the destination field to value, 2 sets the source field to value, 3 assigns the member at off',
        '// through the function at value (Member::Assign(dst + off, src + off)), 4 sets the destination field to dst + value.',
        '// They return dst + RET, or RET itself when RET_CONST. SetFields have no source.',
        'struct CopyOp {', '    uint16_t off, len;', '    uint32_t kind, value;', '};',
        'template <const CopyOp *OPS, uint32_t COUNT> static void run_ops(uint8_t *dst, uint8_t *src) {',
        '    g_field_copies++;',
        '    for (uint32_t k = 0; k < COUNT; k++) {',
        '        const CopyOp &op = OPS[k];',
        '        if (op.kind == 0) __builtin_memmove(dst + op.off, src + op.off, op.len);',
        '        else if (op.kind == 3) ((void(__thiscall *)(void *, void *))(uintptr_t)op.value)(dst + op.off, src + op.off);',
        '        else if (op.kind == 4) *(uint8_t **)(dst + op.off) = dst + op.value;',
        '        else __builtin_memcpy((op.kind == 1 ? dst : src) + op.off, &op.value, op.len);',
        '    }', '}',
        'template <const CopyOp *OPS, uint32_t COUNT, bool RET_CONST, uint32_t RET> static void *__thiscall copy_and_set(uint8_t *dst, uint8_t *src) {',
        '    run_ops<OPS, COUNT>(dst, src);', '    return RET_CONST ? (void *)(uintptr_t)RET : dst + RET;', '}',
        'template <const CopyOp *OPS, uint32_t COUNT, bool RET_CONST, uint32_t RET> static void *__thiscall set_fields(uint8_t *dst) {',
        '    run_ops<OPS, COUNT>(dst, nullptr);', '    return RET_CONST ? (void *)(uintptr_t)RET : dst + RET;', '}']
for a, n, c in found_sets:
    ret = c[-1] if c[-1][0] == 'ret' else ('ret', 4, ('dst', 0))
    c = [x for x in c if x[0] != 'ret']
    if not all(0 <= (x[0] if len(x) == 2 else x[1] if x[0] == 'call' else x[2]) < 0x1000 for x in c):
        continue
    ops = []
    for x in c:
        if x[0] == 'call':
            ops.append([3, x[1], 0, x[2]])
            continue
        if len(x) == 2:
            if ops and ops[-1][0] == 0 and ops[-1][1] + ops[-1][2] == x[0]:
                ops[-1][2] += x[1]
            else:
                ops.append([0, x[0], x[1], 0])
        else:
            ops.append([{'dst': 1, 'src': 2, 'self': 4}[x[1]], x[2], x[3], x[4]])
    found.append((a, n, None))
    out.append(f'static const CopyOp copy_{a:x}[] = {{' + ', '.join(f'{{{o:#x}, {l}, {k}, {v:#x}}}' for k, o, l, v in ops) + '};')
    out.append(f'static const uint8_t copy_{a:x}_original[] = {{' + ', '.join(f'0x{b:02x}' for b in img[a - BASE:a - BASE + 8]) + '};')
    rk, rv = ret[2]
    fn = f'copy_and_set<copy_{a:x}, {len(ops)}, {int(rk == "const")}, {rv:#x}>' if ret[1] == 4 else f'set_fields<copy_{a:x}, {len(ops)}, {int(rk == "const")}, {rv:#x}>'
    out.append(f'REWRITE("Record::{"CopyAndSet" if ret[1] == 4 else "SetFields"}", {a:#x}, copy_{a:x}_original, (&{fn}), &g_field_copies);')
open(os.path.join(HERE, 'forever', 'src', 'rewrite', 'copies.cpp'), 'w', newline='\n').write('\n'.join(out) + '\n')
print(len(found), 'functions,', sum(n for _, n, _ in found), 'bytes; first', hex(found[0][0]) if found else '-')
