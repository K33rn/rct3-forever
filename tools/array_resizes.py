"""Read the "resize an array of records" functions from the game's code and write them as rows,
forever/src/rewrite/array_resizes_list.inc. These all have one shape (the compiler wrote the same container code out
for each record type): when n differs from the count, allocate n records and construct them (ArrayConstruct, 0x402B90),
copy the first min(n, count) old records a field at a time, free the old ones, and keep the new array and n. What
differs is the record's size, its constructor and which fields a copy takes. This reads those three from each
function: the size and constructor from the ArrayConstruct call, the fields from the copy loop that handles the last
few records (the stores through the pointer that moves on by one record each time). A function that doesn't fit is
left out (listed with --why); the equivalence tester checks every one that does against the game's own.
usage: array_resizes.py <exe> [--why]"""
import csv, glob, os, re, sys
import capstone, pefile
from capstone import x86

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
pe = pefile.PE(sys.argv[1], fast_load=True); img = pe.get_memory_mapped_image(); BASE = 0x400000
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
md.detail = True
ALLOC, FREE, CONSTRUCT = 0xbcdfd0, 0xbce010, 0x402b90


class Unknown(Exception):
    pass


def read(a, n):
    ins = list(md.disasm(bytes(img[a - BASE:a - BASE + n]), a))
    if ins[0].mnemonic != 'mov' or 'fs:' not in ins[0].op_str:
        raise Unknown('no exception frame at the start')
    if ins[-1].mnemonic != 'ret' or ins[-1].op_str != '4':
        raise Unknown('not one argument')
    if any(x.mnemonic.startswith(('f', 'rep')) for x in ins):
        raise Unknown('floating point or a block copy')
    calls = [(i, x.operands[0].imm) for i, x in enumerate(ins) if x.mnemonic == 'call' and x.operands[0].type == x86.X86_OP_IMM]
    if [c for _, c in calls] != [ALLOC, CONSTRUCT, FREE]:
        raise Unknown('other calls')
    # ArrayConstruct(items, size, count, constructor): the four pushes before it
    ci = calls[1][0]
    pushes = ins[ci - 4:ci]
    if any(p.mnemonic != 'push' for p in pushes) or pushes[0].operands[0].type != x86.X86_OP_IMM or pushes[2].operands[0].type != x86.X86_OP_IMM:
        raise Unknown('the construct call')
    ctor, size = pushes[0].operands[0].imm, pushes[2].operands[0].imm
    # the array is {items, count}: count read at +4 first, both written at the end
    this = None
    for x in ins[:14]:
        if x.mnemonic == 'mov' and x.op_str.endswith(', ecx') and x.operands[0].type == x86.X86_OP_REG:
            this = x.reg_name(x.operands[0].reg)
    if not this:
        raise Unknown('this is not kept')
    tail = [x for x in ins[calls[2][0]:] if x.mnemonic == 'mov' and x.operands[0].type == x86.X86_OP_MEM and x.reg_name(x.operands[0].mem.base) == this]
    if [x.operands[0].mem.disp for x in tail] != [0, 4, 0]:
        raise Unknown('the end is not items = 0, count, items')
    # the copy loops: "dec r; jne back" between the construct and the free; the last one copies a record at a time
    loops = [i for i in range(ci, calls[2][0]) if ins[i].mnemonic == 'jne' and ins[i - 1].mnemonic == 'dec' and ins[i].operands[0].imm < ins[i].address]
    if len(loops) != 2:
        raise Unknown(f'{len(loops)} copy loops')
    def body(j):
        start = next(k for k, x in enumerate(ins) if x.address == ins[j].operands[0].imm)
        return start, ins[start:j - 1]
    start, last = body(loops[1])
    step = last[-1]
    if step.mnemonic != 'add' or step.operands[1].imm != size:
        raise Unknown('the last loop does not move on by one record')
    cursor = step.reg_name(step.operands[0].reg)
    lea = [x for x in ins[loops[0]:start] if x.mnemonic == 'lea' and x.reg_name(x.operands[0].reg) == cursor]
    if len(lea) != 1:
        raise Unknown('where the last loop starts')
    bias = lea[0].operands[1].mem.disp
    def stores(code, reg):
        out = []
        for x in code:
            if x.mnemonic == 'mov' and x.operands[0].type == x86.X86_OP_MEM and x.reg_name(x.operands[0].mem.base) == reg and not x.operands[0].mem.index:
                if x.operands[1].type != x86.X86_OP_REG:
                    raise Unknown('a constant stored in a record')
                out.append((x.operands[0].mem.disp, x.operands[0].size))
        return out
    fields = sorted((d + bias, w) for d, w in stores(last, cursor))
    if not fields or fields[0][0] < 0 or fields[-1][0] + fields[-1][1] > size:
        raise Unknown('fields outside the record: ' + str(fields))
    for k, (o, w) in enumerate(fields[:-1]):
        if o + w > fields[k + 1][0]:
            raise Unknown('fields overlap')
    # the first loop does four records a time: four times the stores
    _, first = body(loops[0])
    step4 = first[-1]
    if step4.mnemonic != 'add' or step4.operands[1].imm != 4 * size:
        raise Unknown('the first loop does not move on by four records')
    if len(stores(first, step4.reg_name(step4.operands[0].reg))) != 4 * len(fields):
        raise Unknown('the first loop copies other fields')
    if any(w not in (1, 2, 4) for _, w in fields):
        raise Unknown('a field size')
    return size, ctor, fields


rows = list(csv.DictReader(open(os.path.join(HERE, 'docs', 'functions.csv'), encoding='utf-8')))
TAKEN = set()
for f in glob.glob(os.path.join(HERE, 'forever', 'src', 'rewrite', '*.cpp')) + [os.path.join(HERE, 'forever', 'src', 'rewrite', 'generated.inc')]:
    TAKEN |= {int(v, 16) for v in re.findall(r'(?:REWRITE\("[^"]+", |^\w+\()(0x[0-9a-fA-F]+)', open(f, encoding='utf-8').read(), re.M)}
found, why = [], []
for r in rows:
    a, n = int(r['address'], 16), int(r['size'])
    if r['status'] != 'documented' and r['name'] != 'Array::ResizeRecords' or a in TAKEN or n < 150 or n > 3000:
        continue
    code = bytes(img[a - BASE:a - BASE + n])
    if code[:2] != b'\x64\xa1' or (CONSTRUCT - 0).to_bytes(4, 'little') and code.count(b'\xe8') < 3:
        continue
    ins = list(md.disasm(code, a))
    if [x.operands[0].imm for x in ins if x.mnemonic == 'call' and x.operands[0].type == x86.X86_OP_IMM] != [ALLOC, CONSTRUCT, FREE]:
        continue
    try:
        found.append((a, n) + read(a, n))
    except Unknown as e:
        why.append((a, n, str(e)))
lines = ['// The "resize an array of records" functions taken over by array_resize (array_resizes.cpp); written by tools/array_resizes.py.',
         '// Each: its first bytes, the fields a copy takes {offset, bytes}, then the rewrite (record size, constructor)']
for a, n, size, ctor, fields in found:
    first = ', '.join(f'0x{b:02x}' for b in img[a - BASE + 7:a - BASE + 7 + 1]) and ', '.join(f'0x{b:02x}' for b in img[a - BASE:a - BASE + 6])
    lines.append(f'static const uint8_t array_resize_{a:x}_original[] = {{{first}}};')
    lines.append(f'static const ResizeField array_resize_{a:x}_fields[] = {{' + ', '.join(f'{{{o:#x}, {w}}}' for o, w in fields) + '};')
    lines.append(f'REWRITE("Array::ResizeRecords", {a:#x}, array_resize_{a:x}_original, (&array_resize<{size:#x}, {ctor:#x}, array_resize_{a:x}_fields, {len(fields)}>), &g_array_resizes);')
lines.append('// for the tester: each address, its record size and its constructor')
lines.append('extern const uint32_t array_resize_info[];')
lines.append('const uint32_t array_resize_info[] = {' + ''.join(f'{a:#x}, {size:#x}, {ctor:#x}, ' for a, n, size, ctor, fields in found) + '0};')
open(os.path.join(HERE, 'forever', 'src', 'rewrite', 'array_resizes_list.inc'), 'w', newline='\n').write('\n'.join(lines) + '\n')
print(len(found), 'functions,', sum(f[1] for f in found), 'bytes;', len(why), 'left out')
for a, n, size, ctor, fields in found:
    print(f'  {a:#x} {n}: records of {size:#x}, {len(fields)} fields')
if '--why' in sys.argv:
    for a, n, e in why:
        print(f'  {a:#x} {n}: {e}')
