"""Find functions that only set fields of records, by running them: record set-ups (empty lists that point at
themselves, fixed values), moves (fields copied over, the source's fields cleared), copies to other offsets, setters
storing the values they're given. A candidate (documented, no calls, no exception frame, no FPU, ends in ret) is tried
with its records laid out as: its record in ecx and its arguments values; its record in ecx and a second record as its
first argument; or (no ecx) a record as its first argument. It runs 48 times in an emulator, its records moved and
filled with new bytes each time (random, half zeros, small values, 0 and 1, or small dwords), its arguments new random values. It is kept when every instruction ran in some run, it reads nothing but the
records, its stack and the exe's read-only parts, writes nothing but the records and its stack, and every byte it
writes ends up, in every run, the same fixed value, the same byte of a record or argument as it was before the call,
or part of the same pointer into a record; and what it returns is one of those too. The result,
forever/src/rewrite/records_emul.cpp, lists the bytes each one sets (in the order first written), all worked out from
the records as they were before any is written; the equivalence tester checks each against the game's own.
usage: record_emul.py <exe>"""
import collections, csv, glob, os, random, re, sys
import capstone, pefile
from capstone import x86
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_MEM_READ, UC_HOOK_MEM_WRITE, UC_HOOK_CODE, UcError
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP, UC_X86_REG_ESP

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
pe = pefile.PE(sys.argv[1], fast_load=True); img = pe.get_memory_mapped_image(); BASE = 0x400000
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
md.detail = True
RO = [(0x401000, 0x401000 + 0xb24ce5), (0xf26000, 0xf26000 + 0x1c0200)]
AREA, SPAN, STACK, DONE = 0x10000000, 0x4000, 0x30000000, 0x40000000
RUNS = 48   # enough that a branch on the records' contents shows
emu = Uc(UC_ARCH_X86, UC_MODE_32)
emu.mem_map(BASE, (len(img) + 0xfff) & ~0xfff)
emu.mem_write(BASE, bytes(img))
emu.mem_map(AREA, 0x200000)
emu.mem_map(STACK, 0x10000)
emu.mem_map(DONE, 0x1000)
emu.mem_write(DONE, b'\xcc' * 16)
state = {'bases': (), 'bad': False, 'order': None}
LAYOUTS = ('this', 'this+other', 'first')


def rec_of(addr, size):
    for r, b in enumerate(state['bases']):
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
        state['dirty'] = True   # it wrote where it shouldn't (the exe's image, the return page): put them back after the run
        return
    for i in range(size):
        key = (r, addr + i - state['bases'][r])
        if key not in state['order']:
            state['order'][key] = None


covered = set()


def on_code(uc, addr, size, data):
    covered.add(addr)


emu.hook_add(UC_HOOK_MEM_READ, on_read)
emu.hook_add(UC_HOOK_MEM_WRITE, on_write)
emu.hook_add(UC_HOOK_CODE, on_code)


def run(a, layout, nargs, pops, seed):
    """One run: records 0 (and 1), and the arguments as a last read-only 'record'."""
    rnd = random.Random(seed * 7919 + a)
    nrec = 2 if layout == 'this+other' else 1
    bases = (AREA + 0x8000 * seed + 0x100 * (seed % 3), AREA + 0x1f0000 - 0x8000 * seed - 0x40 * (seed % 5) - SPAN)[:nrec]
    state['bases'] = bases
    init = []
    mode = seed % 6   # random bytes, half of them 0, small values, 0 or 1, small dwords: so tests and comparisons show
    for b in bases:
        if mode == 1:
            data = bytes(rnd.getrandbits(8) if rnd.getrandbits(1) else 0 for _ in range(SPAN))
        elif mode == 2:
            data = bytes(rnd.getrandbits(2) for _ in range(SPAN))
        elif mode == 3:
            data = bytes(rnd.getrandbits(1) for _ in range(SPAN))
        elif mode == 4:
            data = b''.join(rnd.getrandbits(2).to_bytes(4, 'little') for _ in range(SPAN // 4))
        else:
            data = bytes(rnd.getrandbits(8) for _ in range(SPAN))
        emu.mem_write(b, data)
        init.append(data)
    args = [(rnd.getrandbits(32) if rnd.getrandbits(1) else 0) if mode == 1 else rnd.getrandbits(2) if mode == 2 else rnd.getrandbits(1) if mode == 3 else rnd.getrandbits(32)
            for _ in range(nargs)]
    if layout == 'this+other':
        args[0] = bases[1]
    if layout == 'first':
        args[0] = bases[0]
    esp = STACK + 0x8000
    emu.mem_write(esp, DONE.to_bytes(4, 'little') + b''.join(x.to_bytes(4, 'little') for x in args) + bytes(16))
    for reg in (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP):
        emu.reg_write(reg, rnd.getrandbits(32))
    emu.reg_write(UC_X86_REG_ECX, bases[0] if layout != 'first' else rnd.getrandbits(32))
    emu.reg_write(UC_X86_REG_ESP, esp)
    state['bad'], state['order'] = False, {}
    state['dirty'] = False
    try:
        emu.emu_start(a, DONE, count=400000)
    except UcError:
        return None
    finally:
        if state['dirty']:
            emu.mem_write(BASE, bytes(img))
            emu.mem_write(DONE, bytes([0xcc]) * 16)
    if state['bad'] or emu.reg_read(UC_X86_REG_ESP) != esp + 4 + pops:
        return None
    final = [bytes(emu.mem_read(b, SPAN)) for b in bases]
    init.append(b''.join(x.to_bytes(4, 'little') for x in args))   # the arguments, read only
    return bases, init, final, list(state['order']), emu.reg_read(UC_X86_REG_EAX)


def explain(a, layout, nargs, pops, ins):
    covered.clear()
    runs = [run(a, layout, nargs, pops, s) for s in range(1, RUNS + 1)]
    if any(r is None for r in runs) or any(r[3] != runs[0][3] for r in runs) or not runs[0][3]:
        return None
    if any(i.address not in covered for i in ins if i.mnemonic not in ('nop', 'int3') and not i.mnemonic.startswith('lea') or i.mnemonic == 'lea' and i.address not in covered and False):
        return None   # some of it never ran: a way through it the runs didn't take, so not shown to do the same
    nsrc = len(runs[0][1])   # records, then the arguments
    nrec = len(runs[0][0])
    order = runs[0][3]
    written = set(order)
    where = [[collections.defaultdict(list) for _ in range(nsrc)] for _ in runs]
    for k, r in enumerate(runs):
        for x in range(nsrc):
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
            c = {(x, p) for x in range(nsrc) for p in where[k][x][v]}
            cands = c if cands is None else cands & c
            if not cands:
                break
        expl[(rr, o)] = ('copy',) + min(cands, key=lambda c: (c != (rr, o), c)) if cands else None
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
    eaxs = [r[4] for r in runs]
    ret = None
    if len(set(eaxs)) == 1:
        ret = ('value', eaxs[0])
    for x in range(nrec):
        ds = {(e - r[0][x]) & 0xffffffff for e, r in zip(eaxs, runs)}
        if len(ds) == 1:
            ret = ('ptr', x, ds.pop())
    if ret is None:   # a dword of a record or argument as it was
        for x in range(nsrc):
            for off in range(0, len(runs[0][1][x]) - 3):
                if all(int.from_bytes(r[1][x][off:off + 4], 'little') == e for r, e in zip(runs, eaxs)):
                    ret = ('copy', x, off)
                    break
            if ret:
                break
    if ret is None:
        return None
    return order, expl, ret, nrec


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
    if (r['status'] != 'documented' and a not in mine and r['name'] != 'Record::Fields') or n < 30 or a in TAKEN:
        continue
    ins = list(md.disasm(bytes(img[a - BASE:a - BASE + n]), a))
    if not ins or ins[-1].address + ins[-1].size != a + n or ins[-1].mnemonic != 'ret':
        continue
    if any(i.mnemonic == 'call' or 'fs:' in i.op_str or i.mnemonic.startswith('f') or (i.mnemonic.startswith('j') and not i.op_str.startswith('0x')) for i in ins):
        continue
    pops = int(ins[-1].op_str, 16) if ins[-1].op_str else 0
    if pops > 12 or pops % 4:
        continue
    # arguments: what ret takes off, or (cdecl) the stack slots the code reads
    nargs = pops // 4
    if not pops:
        depth, top = 0, 0
        for i in ins:
            if i.mnemonic == 'push':
                depth += 4
            elif i.mnemonic == 'pop':
                depth -= 4
            for op in i.operands:
                if op.type == x86.X86_OP_MEM and op.mem.base == x86.X86_REG_ESP:
                    top = max(top, (op.mem.disp - depth) // 4)
        nargs = min(top, 3)
    for layout in LAYOUTS:
        if layout != 'this' and nargs == 0:
            continue
        got = explain(a, layout, nargs, pops, ins)
        if got:
            found.append((a, n, layout, nargs, pops, got))
            break

out = ['// Record set-ups, moves, copies and setters found by running them (tools/record_emul.py): the bytes each sets in',
       '// its records (0, 1), each a fixed value, a byte of a record or of its arguments (the last) as it was, or part of a',
       '// pointer into a record; all worked out from the records as they were before any is written. Do not edit by hand.',
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
       '// what it returns: 0 a fixed value, 1 a pointer into record FROM, 2 a dword of record FROM (or the arguments) as it was',
       'template <uint32_t KIND, uint32_t FROM, uint32_t VALUE> static uint32_t record_result(uint8_t *const *records, uint32_t was) {',
       '    return KIND == 0 ? VALUE : KIND == 1 ? (uint32_t)(uintptr_t)records[FROM] + VALUE : was;',
       '}',
       '#define RECORD_ENTRY(NAME, CONV, PARAMS, ARGS, RECS)                                                            \\',
       '    template <const RecordOp *OPS, uint32_t COUNT, uint32_t TOTAL, uint32_t RK, uint32_t RF, uint32_t RV>          \\',
       '    static uint32_t CONV NAME PARAMS {                                                                              \\',
       '        uint32_t args[4] = ARGS;   /* the arguments, as the game\'s stack has them */                              \\',
       '        uint8_t *records[3] = RECS;                                                                                 \\',
       '        uint32_t was = RK == 2 ? *(uint32_t *)(records[RF] + RV) : 0;                                               \\',
       '        apply_record<OPS, COUNT, TOTAL>(records);                                                                   \\',
       '        return record_result<RK, RF, RV>(records, was);                                                             \\',
       '    }',
       '#define C ,',
       '#define P(x) (uint32_t)(uintptr_t)(x)',
       'RECORD_ENTRY(this0, __thiscall, (uint8_t * self), {0}, {self C nullptr C nullptr})',
       'RECORD_ENTRY(this1, __thiscall, (uint8_t * self, uint32_t a), {a}, {self C (uint8_t *)args C nullptr})',
       'RECORD_ENTRY(this2, __thiscall, (uint8_t * self, uint32_t a, uint32_t b), {a C b}, {self C (uint8_t *)args C nullptr})',
       'RECORD_ENTRY(this3, __thiscall, (uint8_t * self, uint32_t a, uint32_t b, uint32_t c), {a C b C c}, {self C (uint8_t *)args C nullptr})',
       'RECORD_ENTRY(other1, __thiscall, (uint8_t * self, uint8_t *o), {P(o)}, {self C o C (uint8_t *)args})',
       'RECORD_ENTRY(other2, __thiscall, (uint8_t * self, uint8_t *o, uint32_t b), {P(o) C b}, {self C o C (uint8_t *)args})',
       'RECORD_ENTRY(other3, __thiscall, (uint8_t * self, uint8_t *o, uint32_t b, uint32_t c), {P(o) C b C c}, {self C o C (uint8_t *)args})',
       'RECORD_ENTRY(first1, __stdcall, (uint8_t * r), {P(r)}, {r C (uint8_t *)args C nullptr})',
       'RECORD_ENTRY(first2, __stdcall, (uint8_t * r, uint32_t b), {P(r) C b}, {r C (uint8_t *)args C nullptr})',
       'RECORD_ENTRY(first3, __stdcall, (uint8_t * r, uint32_t b, uint32_t c), {P(r) C b C c}, {r C (uint8_t *)args C nullptr})',
       'RECORD_ENTRY(cfirst1, __cdecl, (uint8_t * r), {P(r)}, {r C (uint8_t *)args C nullptr})',
       'RECORD_ENTRY(cfirst2, __cdecl, (uint8_t * r, uint32_t b), {P(r) C b}, {r C (uint8_t *)args C nullptr})',
       'RECORD_ENTRY(cfirst3, __cdecl, (uint8_t * r, uint32_t b, uint32_t c), {P(r) C b C c}, {r C (uint8_t *)args C nullptr})',
       '#undef P',
       '#undef C',
       '']
info = []
for a, n, layout, nargs, pops, (order, expl, ret, nrec) in found:
    if layout != 'this' and pops == 0 and layout != 'first':
        continue   # a cdecl second record: not written out
    if layout == 'this' and pops == 0 and nargs:
        nargs = 0   # a thiscall without stack arguments that reads its caller's stack is not taken
        continue
    argsrec = nrec   # where the arguments are in records[]
    runs = []
    def src(x):
        return x if x < nrec else (2 if layout == 'this+other' else 1)
    for (rr, o) in order:
        e = expl[(rr, o)]
        last = runs[-1] if runs else None
        if e[0] == 'set':
            if last and last['to'] == rr and last['kind'] == 0 and last['at'] + last['length'] == o and last['length'] < 4:
                last['bytes'].append(e[1]); last['length'] += 1; continue
            runs.append({'to': rr, 'kind': 0, 'from': 0, 'shift': 0, 'at': o, 'length': 1, 'bytes': [e[1]]})
        elif e[0] == 'copy':
            f = src(e[1])
            if last and last['to'] == rr and last['kind'] == 1 and last['from'] == f and last['at'] + last['length'] == o and last['value'] + last['length'] == e[2]:
                last['length'] += 1; continue
            runs.append({'to': rr, 'kind': 1, 'from': f, 'shift': 0, 'at': o, 'length': 1, 'value': e[2]})
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
    rk, rf, rv = {'value': (0, 0, ret[1] if ret[0] == 'value' else 0)}.get(ret[0], (0, 0, 0))
    if ret[0] == 'ptr':
        rk, rf, rv = 1, ret[1], ret[2]
    elif ret[0] == 'copy':
        rk, rf, rv = 2, src(ret[1]), ret[2]
    fn = {'this': f'this{nargs}', 'this+other': f'other{nargs}', 'first': f'{"first" if pops else "cfirst"}{nargs}'}[layout]
    out.append(f'REWRITE("Record::Fields", {a:#x}, records_{a:x}_original, (&{fn}<records_{a:x}, {len(runs)}, {len(order)}, {rk}, {rf}, {rv:#x}>), &g_record_sets);')
    info.append((a, {'this': 0, 'this+other': 1, 'first': 2 if pops else 3}[layout], nargs))
out.append('// each one: its address, its layout (0 its record and values, 1 a second record first, 2 / 3 (cdecl) no ecx: a record')
out.append('// first) and how many arguments it takes')
out.append('extern const uint32_t record_info[];')
out.append('const uint32_t record_info[] = {' + ''.join(f'{a:#x}, {l}, {k}, ' for a, l, k in info) + '0};')
open(path, 'w', newline='\n').write('\n'.join(out) + '\n')
print(len(info), 'functions,', sum(f[1] for f in found if any(i[0] == f[0] for i in info)), 'bytes')
for a, n, layout, nargs, pops, g in sorted(found, key=lambda f: -f[1])[:15]:
    print(hex(a), n, layout, nargs, pops, len(g[0]), 'bytes set')
