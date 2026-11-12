"""Find functions that only set fields of their record (ecx) and of a record given as their argument, by running them:
record set-ups (empty lists that point at themselves, fixed values), moves (fields copied over, the source's fields
cleared), copies to other offsets. A candidate (documented, no calls, no exception frame, ends in ret or ret 4) runs
four times in an emulator, its records moved and filled with new random bytes each time. It is kept when it reads
nothing but the records, its stack and the exe's read-only parts, writes nothing but the records and its stack, and
every byte it writes ends up, in every run, the same fixed value, the same byte of a record as it was before the
call, or part of the same pointer into a record; and what it returns is one of those too. The result,
forever/src/rewrite/records_emul.cpp, lists the bytes each one sets (in the order first written), all worked out
from the records as they were before any is written; the equivalence tester checks each against the game's own.
usage: record_emul.py <exe>"""
import collections, csv, glob, os, random, re, sys
import capstone, pefile
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_MEM_READ, UC_HOOK_MEM_WRITE, UcError
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP, UC_X86_REG_ESP

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
pe = pefile.PE(sys.argv[1], fast_load=True); img = pe.get_memory_mapped_image(); BASE = 0x400000
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
RO = [(0x401000, 0x401000 + 0xb24ce5), (0xf26000, 0xf26000 + 0x1c0200)]
AREA, SPAN, STACK, DONE = 0x10000000, 0x1000, 0x30000000, 0x40000000
RUNS = 16   # enough that a branch on the records' contents shows
emu = Uc(UC_ARCH_X86, UC_MODE_32)
emu.mem_map(BASE, (len(img) + 0xfff) & ~0xfff)
emu.mem_write(BASE, bytes(img))
emu.mem_map(AREA, 0x100000)
emu.mem_map(STACK, 0x10000)
emu.mem_map(DONE, 0x1000)
emu.mem_write(DONE, b'\xcc' * 16)
state = {'bases': (0, 0), 'bad': False, 'order': None, 'nrec': 1}


def rec_of(addr, size):
    for r in range(state['nrec']):
        b = state['bases'][r]
        if b <= addr and addr + size <= b + SPAN:
            return r
    return None


def on_read(uc, access, addr, size, value, data):
    if STACK <= addr < STACK + 0x10000 or rec_of(addr, size) is not None or any(a <= addr and addr + size <= b for a, b in RO):
        return
    state['bad'] = True


def on_write(uc, access, addr, size, value, data):
    if STACK <= addr < STACK + 0x10000:
        return
    r = rec_of(addr, size)
    if r is None:
        state['bad'] = True
        return
    for i in range(size):
        key = (r, addr + i - state['bases'][r])
        if key not in state['order']:
            state['order'][key] = None


emu.hook_add(UC_HOOK_MEM_READ, on_read)
emu.hook_add(UC_HOOK_MEM_WRITE, on_write)


def run(a, nrec, pops, seed):
    rnd = random.Random(seed * 7919 + a)
    bases = (AREA + 0x4000 * seed + 0x100 * (seed % 3), AREA + 0x80000 - 0x4000 * seed - 0x40 * (seed % 5))
    state['bases'], state['nrec'] = bases, nrec
    init = []
    for r in range(nrec):
        data = bytes(rnd.getrandbits(8) for _ in range(SPAN))
        emu.mem_write(bases[r], data)
        init.append(data)
    esp = STACK + 0x8000
    emu.mem_write(esp, DONE.to_bytes(4, 'little') + (bases[1] if nrec == 2 else rnd.getrandbits(32)).to_bytes(4, 'little'))
    for reg in (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP):
        emu.reg_write(reg, rnd.getrandbits(32))
    emu.reg_write(UC_X86_REG_ECX, bases[0])
    emu.reg_write(UC_X86_REG_ESP, esp)
    state['bad'], state['order'] = False, {}
    try:
        emu.emu_start(a, DONE, count=300000)
    except UcError:
        return None
    if state['bad'] or emu.reg_read(UC_X86_REG_ESP) != esp + 4 + pops:
        return None
    final = [bytes(emu.mem_read(bases[r], SPAN)) for r in range(nrec)]
    return bases, init, final, list(state['order']), emu.reg_read(UC_X86_REG_EAX)


def explain(a, nrec, pops):
    runs = [run(a, nrec, pops, s) for s in range(1, RUNS + 1)]
    if any(r is None for r in runs) or any(r[3] != runs[0][3] for r in runs) or not runs[0][3]:
        return None
    order = runs[0][3]
    written = set(order)
    # where each byte value sits in each record before the call, per run
    where = [[collections.defaultdict(list) for _ in range(nrec)] for _ in runs]
    for k, r in enumerate(runs):
        for x in range(nrec):
            for o, b in enumerate(r[1][x]):
                where[k][x][b].append(o)
    expl = {}
    for (rr, o) in order:
        vals = [r[2][rr][o] for r in runs]
        if len(set(vals)) == 1 and not all(r[1][rr][o] == vals[0] for r in runs):
            expl[(rr, o)] = ('set', vals[0])
            continue
        cands = None
        for k, v in enumerate(vals):
            c = {(x, p) for x in range(nrec) for p in where[k][x][v]}
            cands = c if cands is None else cands & c
            if not cands:
                break
        if cands:
            expl[(rr, o)] = ('copy',) + min(cands, key=lambda c: (c != (rr, o), c))   # the byte itself (unchanged), else the first
            continue
        expl[(rr, o)] = None
    # pointers: dwords that are a record's address plus a fixed amount
    for (rr, o) in order:
        if expl[(rr, o)] is not None:
            continue
        o4 = o - o % 4
        if not all((rr, o4 + i) in written for i in range(4)):
            return None
        dw = [int.from_bytes(r[2][rr][o4:o4 + 4], 'little') for r in runs]
        hit = None
        for x in range(nrec):
            ds = {(d - r[0][x]) & 0xffffffff for d, r in zip(dw, runs)}
            if len(ds) == 1:
                hit = (x, ds.pop())
        if hit is None:
            return None
        for i in range(4):
            expl[(rr, o4 + i)] = ('ptr', hit[0], hit[1], i)
    for (rr, o), e in expl.items():
        if e[0] == 'ptr' and not all(expl.get((rr, o - e[3] + i), (None,))[0] == 'ptr' for i in range(4)):
            return None
    # what it returns
    eaxs = [r[4] for r in runs]
    ret = None
    if len(set(eaxs)) == 1:
        ret = ('value', eaxs[0])
    for x in range(nrec):
        ds = {(e - r[0][x]) & 0xffffffff for e, r in zip(eaxs, runs)}
        if len(ds) == 1:
            ret = ('ptr', x, ds.pop())
    if ret is None:
        return None
    return order, expl, ret


TAKEN = set()
for f in glob.glob(os.path.join(HERE, 'forever', 'src', 'rewrite', '*.*')):
    if not f.endswith('records_emul.cpp'):
        TAKEN |= {int(x, 16) for x in re.findall(r'(?:REWRITE\("[^"]+",|^[A-Z_0-9]+\()\s*(0x[0-9a-fA-F]+)', open(f, encoding='utf-8').read(), re.M)}
path = os.path.join(HERE, 'forever', 'src', 'rewrite', 'records_emul.cpp')
mine = set()
if os.path.exists(path):
    mine = {int(x, 16) for x in re.findall(r'^REWRITE\("[^"]+", (0x[0-9a-f]+)', open(path).read(), re.M)}
found = []
for r in csv.DictReader(open(os.path.join(HERE, 'docs', 'functions.csv'), encoding='utf-8')):
    a, n = int(r['address'], 16), int(r['size'])
    if (r['status'] != 'documented' and a not in mine) or n < 40 or a in TAKEN:
        continue
    ins = list(md.disasm(bytes(img[a - BASE:a - BASE + n]), a))
    if not ins or ins[-1].address + ins[-1].size != a + n or ins[-1].mnemonic != 'ret':
        continue
    if any(i.mnemonic == 'call' or 'fs:' in i.op_str or i.mnemonic.startswith('f') or (i.mnemonic.startswith('j') and not i.op_str.startswith('0x')) for i in ins):
        continue
    pops = int(ins[-1].op_str, 16) if ins[-1].op_str else 0
    if pops not in (0, 4):
        continue
    got = explain(a, 2 if pops == 4 else 1, pops)
    if got:
        found.append((a, n, pops, got))

# ops: runs of bytes of one kind, in first-written order
out = ['// Record set-ups, moves and copies found by running them (tools/record_emul.py): the bytes each sets in its record',
       '// (0, ecx) and the one it is given (1), each a fixed value, a byte of a record as it was, or part of a pointer into a',
       '// record; all worked out from the records as they were before any is written. Do not edit by hand.',
       '#include "rewrite.h"', '',
       'struct RecordOp {', '    uint8_t to, kind, from, shift;   // kind 0: fixed bytes, 1: copied, 2: a pointer into record from', '    uint16_t at, length;',
       '    uint32_t value;                   // the fixed bytes, the offset copied from, or what the pointer adds', '};',
       'static volatile long g_record_sets;',
       'template <const RecordOp *OPS, uint32_t COUNT, uint32_t TOTAL>',
       'static void apply_record(uint8_t *const *records) {',
       '    g_record_sets++;',
       '    uint8_t made[TOTAL];   // every byte first, from the records as they were',
       '    uint32_t k = 0;',
       '    for (uint32_t i = 0; i < COUNT; i++) {',
       '        const RecordOp &op = OPS[i];',
       '        for (uint32_t b = 0; b < op.length; b++, k++) {',
       '            if (op.kind == 0) made[k] = (uint8_t)(op.value >> (8 * b));',
       '            else if (op.kind == 1) made[k] = records[op.from][op.value + b];',
       '            else made[k] = (uint8_t)(((uint32_t)(uintptr_t)records[op.from] + op.value) >> (8 * (op.shift + b)));',
       '        }',
       '    }',
       '    k = 0;',
       '    for (uint32_t i = 0; i < COUNT; i++)',
       '        for (uint32_t b = 0; b < OPS[i].length; b++, k++) ((volatile uint8_t *)records[OPS[i].to])[OPS[i].at + b] = made[k];',
       '}',
       'template <uint32_t KIND, uint32_t VALUE> static uint32_t record_result(uint8_t *const *records) {',
       '    return KIND == 0 ? VALUE : (uint32_t)(uintptr_t)records[KIND - 1] + VALUE;',
       '}',
       'template <const RecordOp *OPS, uint32_t COUNT, uint32_t TOTAL, uint32_t RK, uint32_t RV>',
       'static uint32_t __thiscall set_own(uint8_t *self) {',
       '    uint8_t *records[2] = {self, nullptr};',
       '    apply_record<OPS, COUNT, TOTAL>(records);',
       '    return record_result<RK, RV>(records);',
       '}',
       'template <const RecordOp *OPS, uint32_t COUNT, uint32_t TOTAL, uint32_t RK, uint32_t RV>',
       'static uint32_t __thiscall set_both(uint8_t *self, uint8_t *other) {',
       '    uint8_t *records[2] = {self, other};',
       '    apply_record<OPS, COUNT, TOTAL>(records);',
       '    return record_result<RK, RV>(records);',
       '}', '']
info = []
for a, n, pops, (order, expl, ret) in found:
    runs = []
    for (rr, o) in order:
        e = expl[(rr, o)]
        last = runs[-1] if runs else None
        if e[0] == 'set':
            if last and last['to'] == rr and last['kind'] == 0 and last['at'] + last['length'] == o and last['length'] < 4:
                last['bytes'].append(e[1]); last['length'] += 1; continue
            runs.append({'to': rr, 'kind': 0, 'from': 0, 'shift': 0, 'at': o, 'length': 1, 'bytes': [e[1]]})
        elif e[0] == 'copy':
            if last and last['to'] == rr and last['kind'] == 1 and last['from'] == e[1] and last['at'] + last['length'] == o and last['value'] + last['length'] == e[2]:
                last['length'] += 1; continue
            runs.append({'to': rr, 'kind': 1, 'from': e[1], 'shift': 0, 'at': o, 'length': 1, 'value': e[2]})
        else:
            if last and last['to'] == rr and last['kind'] == 2 and last['from'] == e[1] and last['value'] == e[2] and last['at'] + last['length'] == o and last['shift'] + last['length'] == e[3]:
                last['length'] += 1; continue
            runs.append({'to': rr, 'kind': 2, 'from': e[1], 'shift': e[3], 'at': o, 'length': 1, 'value': e[2]})
    items = []
    for x in runs:
        v = int.from_bytes(bytes(x['bytes']), 'little') if x['kind'] == 0 else x['value']
        items.append(f'{{{x["to"]}, {x["kind"]}, {x["from"]}, {x["shift"]}, {x["at"]:#x}, {x["length"]}, {v:#x}}}')
    out.append(f'static const RecordOp records_{a:x}[] = {{{", ".join(items)}}};')
    out.append(f'static const uint8_t records_{a:x}_original[] = {{' + ', '.join(f'0x{b:02x}' for b in img[a - BASE:a - BASE + min(8, n)]) + '};')
    rk, rv = (0, ret[1]) if ret[0] == 'value' else (ret[1] + 1, ret[2])
    fn = 'set_both' if pops == 4 else 'set_own'
    out.append(f'REWRITE("Record::Fields", {a:#x}, records_{a:x}_original, (&{fn}<records_{a:x}, {len(runs)}, {len(order)}, {rk}, {rv:#x}>), &g_record_sets);')
    info.append((a, 2 if pops == 4 else 1))
out.append('// each one: its address and how many records it takes')
out.append('extern const uint32_t record_info[];')
out.append('const uint32_t record_info[] = {' + ''.join(f'{a:#x}, {k}, ' for a, k in info) + '0};')
open(path, 'w', newline='\n').write('\n'.join(out) + '\n')
print(len(found), 'functions,', sum(f[1] for f in found), 'bytes')
for a, n, pops, g in sorted(found, key=lambda f: -f[1])[:15]:
    print(hex(a), n, pops, len(g[0]), 'bytes set')
