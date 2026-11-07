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


def analyse(a, n):
    regs = {x86.X86_REG_ECX: ('dst', 0)}
    depth = 0          # bytes pushed since entry
    copied = []
    ins = list(md.disasm(bytes(img[a - BASE:a - BASE + n]), a))
    if not ins or ins[-1].address + ins[-1].size != a + n:
        return None
    for i in ins[:-1]:
        ops = i.operands
        if i.mnemonic == 'push' and ops[0].type == x86.X86_OP_REG:
            depth += 4
        elif i.mnemonic == 'pop' and ops[0].type == x86.X86_OP_REG:
            depth -= 4
            regs.pop(PARENT.get(ops[0].reg, ops[0].reg), None)
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
        elif i.mnemonic == 'mov' and ops[0].type == x86.X86_OP_MEM and ops[1].type == x86.X86_OP_REG:
            m = ops[0].mem
            if m.index or m.segment or m.base == x86.X86_REG_ESP:
                return None
            bv = regs.get(PARENT.get(m.base))
            v = regs.get(PARENT.get(ops[1].reg))
            if not bv or bv[0] != 'dst' or not v or v[0] != 'field':
                return None
            off = bv[1] + m.disp
            if v[1] != off or v[2] != ops[1].size or ops[0].size != ops[1].size:
                return None
            copied.append((off, ops[1].size))
        elif i.mnemonic == 'nop':
            pass
        else:
            return None
    last = ins[-1]
    if last.mnemonic != 'ret' or not last.operands or last.operands[0].imm != 4 or depth != 0:
        return None
    if regs.get(x86.X86_REG_EAX) != ('dst', 0) or not copied:
        return None
    return copied


rows = list(csv.DictReader(open(os.path.join(HERE, 'docs', 'functions.csv'), encoding='utf-8')))
import glob, re
taken = set()
for f in glob.glob(os.path.join(HERE, 'forever', 'src', 'rewrite', '*.*')):
    if not f.endswith('copies.cpp'):
        taken |= {int(x, 16) for x in re.findall(r'(?:REWRITE\("[^"]+",|^[A-Z_0-9]+\()\s*(0x[0-9a-fA-F]+)', open(f, encoding='utf-8').read(), re.M)}
found = []
for r in rows:
    if r['status'] != 'documented' or int(r['size']) < 24 or int(r['address'], 16) in taken:
        continue
    a, n = int(r['address'], 16), int(r['size'])
    c = analyse(a, n)
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
open(os.path.join(HERE, 'forever', 'src', 'rewrite', 'copies.cpp'), 'w', newline='\n').write('\n'.join(out) + '\n')
print(len(found), 'functions,', sum(n for _, n, _ in found), 'bytes; first', hex(found[0][0]) if found else '-')
