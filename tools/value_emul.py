"""Find functions that only map one argument to a value (switches over cases and ranges: version numbers to formats,
error codes to texts) by running them, and write them as tables of value ranges. A candidate (documented, no calls,
no exception frame, ends in ret) takes its argument in ecx, edx or on the stack; it is kept when, run in an emulator,
it reads nothing but its argument and the exe's read-only parts, writes nothing but its stack, and its result is the
same everywhere between the constants it compares with: checked at each constant and its neighbours, and at random
values in each range (and the same whatever the other registers hold). The result,
forever/src/rewrite/values_emul.cpp, lists each one's ranges (first value of the range, result); the equivalence
tester checks each against the game's own. usage: value_emul.py <exe> [--why]"""
import bisect, csv, os, random, sys
import capstone, pefile
from capstone import x86
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_MEM_READ, UC_HOOK_MEM_WRITE, UcError
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP, UC_X86_REG_ESP

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
pe = pefile.PE(sys.argv[1], fast_load=True); img = pe.get_memory_mapped_image(); BASE = 0x400000
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
md.detail = True
RO = [(0x401000, 0x401000 + 0xb24ce5), (0xf26000, 0xf26000 + 0x1c0200)]
STACK, DONE = 0x30000000, 0x40000000
OTHERS = [UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP]
emu = Uc(UC_ARCH_X86, UC_MODE_32)
emu.mem_map(BASE, (len(img) + 0xfff) & ~0xfff)
emu.mem_write(BASE, bytes(img))
emu.mem_map(STACK, 0x10000)
emu.mem_map(DONE, 0x1000)
emu.mem_write(DONE, b'\xcc' * 16)
bad = [False]
ARGSLOT = STACK + 0x8004


def on_read(uc, access, addr, size, value, data):
    if STACK + 0x7000 <= addr < STACK + 0x8008:   # its stack, the return address and the argument
        return
    if not any(a <= addr and addr + size <= b for a, b in RO):
        bad[0] = True


dirty = [False]


def on_write(uc, access, addr, size, value, data):
    if not (STACK + 0x7000 <= addr < STACK + 0x8000):
        bad[0] = True
        dirty[0] = True   # it wrote where it shouldn't (the exe's image, the return page): put them back after the run


emu.hook_add(UC_HOOK_MEM_READ, on_read)
emu.hook_add(UC_HOOK_MEM_WRITE, on_write)
rnd = random.Random(7)


def run(a, where, x, pops, small=None):
    # the other inputs it might have: random, or (small) all the same small value, so a second argument shows
    other = (lambda: rnd.getrandbits(32)) if small is None else (lambda: small)
    esp = STACK + 0x8000
    emu.mem_write(esp, DONE.to_bytes(4, 'little') + (x if where == 'stack' else other()).to_bytes(4, 'little') + other().to_bytes(4, 'little'))
    for r in OTHERS:
        emu.reg_write(r, other())
    emu.reg_write(UC_X86_REG_ECX, x if where == 'ecx' else other())
    emu.reg_write(UC_X86_REG_EDX, x if where == 'edx' else other())
    emu.reg_write(UC_X86_REG_ESP, esp)
    bad[0] = False
    dirty[0] = False
    try:
        emu.emu_start(a, DONE, count=20000)
    except UcError:
        return None
    finally:
        if dirty[0]:
            emu.mem_write(BASE, bytes(img))
            emu.mem_write(DONE, bytes([0xcc]) * 16)
    if bad[0] or emu.reg_read(UC_X86_REG_ESP) != esp + 4 + pops:
        return None
    return emu.reg_read(UC_X86_REG_EAX)


def model(a, n, ins):
    pops = int(ins[-1].op_str, 0) if ins[-1].op_str else 0
    reads = set()
    for x in ins:
        for op in x.operands:
            if op.type == x86.X86_OP_MEM and op.mem.base == x86.X86_REG_ESP:
                reads.add(op.mem.disp)
    uses_stack = any(d == 4 for d in reads)
    # where the argument is: try each, keep the one the result depends on
    for where in (['stack'] if uses_stack else []) + ['ecx', 'edx']:
        consts = {0}
        for x in ins:
            for op in x.operands:
                if op.type == x86.X86_OP_IMM:
                    consts.add(op.imm & 0xffffffff)   # every constant: what the argument may be compared with
        points = set()
        for c in consts:
            for d in (-2, -1, 0, 1, 2):
                points.add((c + d) & 0xffffffff)
        points |= {0x7fffffff, 0x80000000, 0xffffffff}
        bounds = sorted(points)
        values = []
        for p in bounds:
            v = run(a, where, p, pops)
            if v is None:
                break
            values.append(v)
        else:
            if len(set(values)) <= 1:
                continue   # the result doesn't depend on this one
            # each range (from one point to the next) must give one result: random values inside, and other registers
            ok = True
            for k, p in enumerate(bounds):
                hi = bounds[k + 1] if k + 1 < len(bounds) else 0x100000000
                for x in (range(p + 1, hi) if hi - p <= 0x1000 else [rnd.randrange(p, hi) for _ in range(64)]):   # every value of a small range
                    if run(a, where, x, pops) != values[k]:
                        ok = False
                        break
                if not ok:
                    break
                if run(a, where, p, pops) != values[k] or any(run(a, where, p, pops, small) != values[k] for small in (0, 1, 2, 3, 5, 0xffffffff)):
                    ok = False
                    break
            if not ok:
                return None
            ranges = []
            for p, v in zip(bounds, values):
                if not ranges or ranges[-1][1] != v:
                    ranges.append((p, v))
            return where, pops, ranges
        return None
    return None


rows = list(csv.DictReader(open(os.path.join(HERE, 'docs', 'functions.csv'), encoding='utf-8')))
import glob, re
TAKEN = set()
for f in glob.glob(os.path.join(HERE, 'forever', 'src', 'rewrite', '*.cpp')):
    if not f.endswith('values_emul.cpp'):
        TAKEN |= {int(x, 16) for x in re.findall(r'REWRITE\("[^"]+", (0x[0-9a-fA-F]+)', open(f, encoding='utf-8').read())}
found = []
OUTPUT = os.path.join(HERE, 'forever', 'src', 'rewrite', 'values_emul.cpp')
mine = set()   # its own earlier finds (functions.csv already calls them rewritten)
if os.path.exists(OUTPUT):
    mine = {int(x, 16) for x in re.findall(r'^REWRITE\("[^"]+", (0x[0-9a-f]+)', open(OUTPUT).read(), re.M)}
for r in rows:
    if r['status'] != 'documented' and int(r['address'], 16) not in mine and r['name'] != 'Values::Map':
        continue
    a, n = int(r['address'], 16), int(r['size'])
    if n < 24:
        continue
    ins = list(md.disasm(bytes(img[a - BASE:a - BASE + n]), a))
    if not ins or ins[-1].mnemonic != 'ret' or ins[-1].address + ins[-1].size != a + n:
        continue
    if any(x.mnemonic == 'call' or 'fs:' in x.op_str or x.mnemonic.startswith(('f', 'rep')) or x.mnemonic in ('int', 'int3', 'in', 'out', 'cpuid', 'rdtsc') for x in ins):
        continue
    if a in TAKEN:
        continue
    m = model(a, n, ins)
    if m:
        found.append((a, n) + m)

lines = ['// Functions that only map one argument to a value, found by running them (tools/value_emul.py): each is its',
         '// ranges of argument values (from the first value of each range on) and the result for each. Do not edit by hand.',
         '#include "rewrite.h"', '',
         'struct ValueRange {', '    uint32_t from, value;', '};',
         'static volatile long g_value_maps;',
         'template <const ValueRange *R, uint32_t N> static uint32_t lookup(uint32_t x) {',
         '    g_value_maps++;',
         '    uint32_t lo = 0, hi = N;   // the last range starting at or below x (the first starts at 0)',
         '    while (hi - lo > 1) {',
         '        uint32_t mid = (lo + hi) / 2;',
         '        if (R[mid].from <= x) lo = mid; else hi = mid;',
         '    }',
         '    return R[lo].value;',
         '}',
         'template <const ValueRange *R, uint32_t N> static uint32_t __fastcall value_ecx(uint32_t x) { return lookup<R, N>(x); }',
         'template <const ValueRange *R, uint32_t N> static uint32_t __fastcall value_edx(uint32_t, uint32_t x) { return lookup<R, N>(x); }',
         'template <const ValueRange *R, uint32_t N> static uint32_t __cdecl value_stack(uint32_t x) { return lookup<R, N>(x); }',
         'template <const ValueRange *R, uint32_t N> static uint32_t __stdcall value_stack4(uint32_t x) { return lookup<R, N>(x); }',
         '']
info = []
for a, n, where, pops, ranges in found:
    if where == 'stack' and pops not in (0, 4) or where != 'stack' and pops != 0:
        continue
    entry = {'ecx': 'value_ecx', 'edx': 'value_edx', 'stack': 'value_stack' if pops == 0 else 'value_stack4'}[where]
    lines.append(f'static const ValueRange values_{a:x}[] = {{' + ', '.join(f'{{{p:#x}, {v:#x}}}' for p, v in ranges) + '};')
    lines.append(f'static const uint8_t values_{a:x}_original[] = {{' + ', '.join(f'0x{b:02x}' for b in img[a - BASE:a - BASE + min(n, 8)]) + '};')
    lines.append(f'REWRITE("Values::Map", {a:#x}, values_{a:x}_original, (&{entry}<values_{a:x}, {len(ranges)}>), &g_value_maps);')
    info.append((a, {'ecx': 0, 'edx': 1, 'stack': 2}[where] + (1 if pops else 0) * (where == 'stack'), ranges))
lines.append('// each one: its address, where its argument is (0 ecx, 1 edx, 2 the stack, 3 the stack taken off), how many ranges, their starts')
lines.append('extern const uint32_t value_info[];')
lines.append('const uint32_t value_info[] = {' + ''.join(f'{a:#x}, {w}, {len(r)}, ' + ''.join(f'{p:#x}, ' for p, _ in r) for a, w, r in info) + '0};')
open(os.path.join(HERE, 'forever', 'src', 'rewrite', 'values_emul.cpp'), 'w', newline='\n').write('\n'.join(lines) + '\n')
print(len(info), 'functions,', sum(n for a, n, w, p, r in found if any(i[0] == a for i in info)), 'bytes')
for a, n, where, pops, ranges in sorted(found, key=lambda f: -f[1])[:20]:
    print(hex(a), n, where, pops, len(ranges), 'ranges')
