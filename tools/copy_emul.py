"""Find field copies and setters that copy_funcs.py can't follow (loops over entries, odd register use) by running them.
Each candidate (no calls, no exception frame, ends in ret / ret 4) runs twice in an emulator on two different random
records. It is kept only when it reads nothing but the two records and its own stack, writes nothing but the
destination and its stack, returns the destination, and every destination byte it writes ends up, in both runs, as
the source byte at the same offset or as the same fixed value. The result, forever/src/rewrite/copies_emul.cpp, lists
the byte ranges copied and the values set; the equivalence tester then checks each against the game's own on random
records. usage: copy_emul.py <exe>"""
import csv, glob, os, random, re, sys
import capstone, pefile
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_MEM_READ, UC_HOOK_MEM_WRITE, UcError
from unicorn.x86_const import UC_X86_REG_ECX, UC_X86_REG_ESP, UC_X86_REG_EAX

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
pe = pefile.PE(sys.argv[1], fast_load=True); img = pe.get_memory_mapped_image(); BASE = 0x400000
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
DST, SRC, STACK, SPAN = 0x10000000, 0x20000000, 0x30000000, 0x2000
DONE = 0x40000000   # return address: a page of int3

emu = Uc(UC_ARCH_X86, UC_MODE_32)
emu.mem_map(BASE, (len(img) + 0xfff) & ~0xfff)
emu.mem_write(BASE, bytes(img))
for at in (DST, SRC, STACK, DONE):
    emu.mem_map(at, 0x10000)
emu.mem_write(DONE, b'\xcc' * 16)
bad = []


at = {'dst': DST, 'src': SRC}   # where this run's records are (moved between runs, so pointers into them can't pass as values)


def on_read(uc, access, addr, size, value, data):
    if not (at['src'] <= addr < at['src'] + SPAN or at['dst'] <= addr < at['dst'] + SPAN or STACK <= addr < STACK + 0x10000):
        bad.append(addr)


writes = set()


def on_write(uc, access, addr, size, value, data):
    if at['dst'] <= addr < at['dst'] + SPAN:
        writes.update(range(addr - at['dst'], addr - at['dst'] + size))
    elif not STACK <= addr < STACK + 0x10000:
        bad.append(addr)


emu.hook_add(UC_HOOK_MEM_READ, on_read)
emu.hook_add(UC_HOOK_MEM_WRITE, on_write)


def run(a, args, seed):
    rng = random.Random(seed)
    at['dst'], at['src'] = DST + 0x2000 * seed, SRC + 0x2000 * (3 - seed)
    src = bytes(rng.randrange(256) for _ in range(SPAN))
    dst = bytes(rng.randrange(256) for _ in range(SPAN))
    emu.mem_write(at['src'], src); emu.mem_write(at['dst'], dst)
    esp = STACK + 0x8000
    emu.mem_write(esp, DONE.to_bytes(4, 'little') + at['src'].to_bytes(4, 'little'))
    emu.reg_write(UC_X86_REG_ESP, esp); emu.reg_write(UC_X86_REG_ECX, at['dst'])
    bad.clear(); writes.clear()
    try:
        emu.emu_start(a, DONE, count=200000)
    except UcError:
        return None
    if bad or emu.reg_read(UC_X86_REG_EAX) != at['dst'] or emu.reg_read(UC_X86_REG_ESP) != esp + 4 + args:
        return None
    return src, bytes(emu.mem_read(at['dst'], SPAN)), set(writes)


def classify(a, args):
    one, two, three = run(a, args, 1), run(a, args, 2), run(a, args, 3)
    if not one or not two or not three or not (one[2] == two[2] == three[2]) or not one[2]:
        return None
    ops = []   # (offset, 'copy') or (offset, value)
    for o in sorted(one[2]):
        if all(r[1][o] == r[0][o] for r in (one, two, three)) and args == 4:
            ops.append((o, 'copy'))
        elif one[1][o] == two[1][o] == three[1][o]:
            ops.append((o, one[1][o]))
        else:
            return None
    return ops


taken = set()
for f in glob.glob(os.path.join(HERE, 'forever', 'src', 'rewrite', '*.*')):
    if not f.endswith('copies_emul.cpp'):
        taken |= {int(x, 16) for x in re.findall(r'(?:REWRITE\("[^"]+",|^[A-Z_0-9]+\()\s*(0x[0-9a-fA-F]+)', open(f, encoding='utf-8').read(), re.M)}
mine = set()
path = os.path.join(HERE, 'forever', 'src', 'rewrite', 'copies_emul.cpp')
if os.path.exists(path):
    mine = {int(x, 16) for x in re.findall(r'^REWRITE\("[^"]+", (0x[0-9a-f]+)', open(path).read(), re.M)}
found = []
for r in csv.DictReader(open(os.path.join(HERE, 'docs', 'functions.csv'), encoding='utf-8')):
    a, n = int(r['address'], 16), int(r['size'])
    if (r['status'] != 'documented' and a not in mine) or n < 24 or a in taken:
        continue
    ins = list(md.disasm(bytes(img[a - BASE:a - BASE + n]), a))
    if not ins or ins[-1].address + ins[-1].size != a + n or ins[-1].mnemonic != 'ret':
        continue
    if any(i.mnemonic == 'call' or 'fs:' in i.op_str or (i.mnemonic.startswith('j') and not i.op_str.startswith('0x')) for i in ins):
        continue
    args = int(ins[-1].op_str, 16) if ins[-1].op_str else 0
    if args not in (0, 4):
        continue
    ops = classify(a, args)
    if ops:
        found.append((a, n, args, ops))

out = ['// Field copies and setters found by running them (tools/copy_emul.py): each is the byte ranges of its record copied from',
       '// the source and the bytes set to fixed values; they return the record. Do not edit by hand.',
       '#include "rewrite.h"', '', 'struct EmulOp {', '    uint16_t off, len;', '    uint32_t value;   // len bytes of value, or a copy when len is 0x8000 + length', '};',
       'static volatile long g_emul_copies;',
       'template <const EmulOp *OPS, uint32_t COUNT> static void apply(uint8_t *dst, const uint8_t *src) {',
       '    g_emul_copies++;',
       '    for (uint32_t k = 0; k < COUNT; k++) {',
       '        const EmulOp &op = OPS[k];',
       '        if (op.len & 0x8000) __builtin_memmove(dst + op.off, src + op.off, op.len & 0x7fff);',
       '        else __builtin_memcpy(dst + op.off, &op.value, op.len);',
       '    }', '}',
       'template <const EmulOp *OPS, uint32_t COUNT> static void *__thiscall copy_record(uint8_t *dst, const uint8_t *src) {',
       '    apply<OPS, COUNT>(dst, src);', '    return dst;', '}',
       'template <const EmulOp *OPS, uint32_t COUNT> static void *__thiscall set_record(uint8_t *dst) {',
       '    apply<OPS, COUNT>(dst, nullptr);', '    return dst;', '}', '']
for a, n, args, ops in found:
    runs = []   # [off, len, kind, bytes]
    for o, k in ops:
        kind = 'copy' if k == 'copy' else 'set'
        if runs and runs[-1][0] + runs[-1][1] == o and runs[-1][2] == kind and (kind == 'copy' or runs[-1][1] < 4):
            runs[-1][1] += 1
            if kind == 'set':
                runs[-1][3].append(k)
        else:
            runs.append([o, 1, kind, [] if kind == 'copy' else [k]])
    items = ', '.join(f'{{{o:#x}, {0x8000 + l:#x}, 0}}' if kind == 'copy' else f'{{{o:#x}, {l}, {int.from_bytes(bytes(v), "little"):#x}}}' for o, l, kind, v in runs)
    out.append(f'static const EmulOp emul_{a:x}[] = {{{items}}};')
    out.append(f'static const uint8_t emul_{a:x}_original[] = {{' + ', '.join(f'0x{b:02x}' for b in img[a - BASE:a - BASE + 8]) + '};')
    fn, name = ('copy_record', 'Record::CopyRanges') if args == 4 else ('set_record', 'Record::SetRanges')
    out.append(f'REWRITE("{name}", {a:#x}, emul_{a:x}_original, (&{fn}<emul_{a:x}, {len(runs)}>), &g_emul_copies);')
open(path, 'w', newline='\n').write('\n'.join(out) + '\n')
print(len(found), 'functions,', sum(f[1] for f in found), 'bytes;', sum(f[2] == 4 for f in found), 'copies')
