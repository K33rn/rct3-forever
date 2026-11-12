"""Find functions that only set game globals to fixed values (start-up tables, constant matrices built on the stack and
copied out) by running them. Each candidate (documented, no calls, ends in ret) runs twice in an emulator with
different registers and stack contents. It is kept only when it reads nothing but its own stack and the exe's
read-only parts (.text, .rdata), writes nothing but globals (.data) and its stack, and leaves the same globals, the
same eax and the same stack pointer in both runs. The result, forever/src/rewrite/consts_emul.cpp, lists the global
bytes each one sets (in the order first written); the equivalence tester checks each against the game's own.
usage: const_emul.py <exe>"""
import csv, os, random, re, sys
import capstone, pefile
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_MEM_READ, UC_HOOK_MEM_WRITE, UcError
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP, UC_X86_REG_ESP

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
pe = pefile.PE(sys.argv[1], fast_load=True); img = pe.get_memory_mapped_image(); BASE = 0x400000
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
RO = [(0x401000, 0x401000 + 0xb24ce5), (0xf26000, 0xf26000 + 0x1c0200)]
DATA = (0x10e7000, 0x10e7000 + 0x33809c)
STACK, DONE = 0x30000000, 0x40000000
REGS = [UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP]

emu = Uc(UC_ARCH_X86, UC_MODE_32)
emu.mem_map(BASE, (len(img) + 0xfff) & ~0xfff)
emu.mem_map(STACK, 0x10000)
emu.mem_map(DONE, 0x1000)
emu.mem_write(DONE, b'\xcc' * 16)
state = {'bad': False, 'writes': None}


def on_read(uc, access, addr, size, value, data):
    if not (STACK <= addr < STACK + 0x10000 or any(a <= addr and addr + size <= b for a, b in RO)):
        state['bad'] = True


def on_write(uc, access, addr, size, value, data):
    if STACK <= addr < STACK + 0x10000:
        return
    if not (DATA[0] <= addr and addr + size <= DATA[1]):
        state['bad'] = True
        return
    for i in range(size):
        if addr + i not in state['writes']:
            state['writes'][addr + i] = None   # remember the order first written; the value is read at the end


emu.hook_add(UC_HOOK_MEM_READ, on_read)
emu.hook_add(UC_HOOK_MEM_WRITE, on_write)


def run(a, seed):
    rnd = random.Random(seed)
    emu.mem_write(BASE, bytes(img))   # the image as it is in the exe each run
    emu.mem_write(STACK, bytes(rnd.getrandbits(8) for _ in range(0x10000)))
    for r in REGS:
        emu.reg_write(r, rnd.getrandbits(32))
    esp = STACK + 0x8000
    emu.mem_write(esp, DONE.to_bytes(4, 'little'))
    emu.reg_write(UC_X86_REG_ESP, esp)
    state['bad'] = False
    state['writes'] = {}
    try:
        emu.emu_start(a, DONE, count=200000)
    except UcError:
        return None
    if state['bad'] or emu.reg_read(UC_X86_REG_ESP) is None:
        return None
    final = {ad: emu.mem_read(ad, 1)[0] for ad in state['writes']}
    return list(state['writes']), final, emu.reg_read(UC_X86_REG_EAX), emu.reg_read(UC_X86_REG_ESP) - esp


rows = [r for r in csv.DictReader(open(os.path.join(HERE, 'docs', 'functions.csv'), encoding='utf-8'))]
out = []
total = 0
for r in rows:
    if r['status'] != 'documented':
        continue
    a, n = int(r['address'], 16), int(r['size'])
    if n < 16:
        continue
    ins = list(md.disasm(bytes(img[a - BASE:a - BASE + n]), a))
    if not ins or ins[-1].mnemonic != 'ret' or ins[-1].address + ins[-1].size != a + n:
        continue
    if any(x.mnemonic == 'call' or 'fs:' in x.op_str or x.mnemonic.startswith(('rep', 'in', 'out', 'int', 'cpuid', 'rdtsc')) for x in ins):
        continue
    one, two = run(a, 1), run(a, 2)
    if not one or not two or one[0] == [] or one[1:] != two[1:] or one[0] != two[0]:
        continue
    order, final, eax, moved = one
    if moved != 4 + (int(ins[-1].op_str, 0) if ins[-1].op_str else 0):
        continue
    # runs of bytes in first-written order
    runs = []
    for ad in order:
        if runs and runs[-1][0] + len(runs[-1][1]) == ad:
            runs[-1][1].append(final[ad])
        else:
            runs.append([ad, [final[ad]]])
    out.append((a, n, eax, int(ins[-1].op_str, 0) // 4 if ins[-1].op_str else 0, runs))
    total += n

lines = ['// Functions that set game globals to fixed values, found by running them (tools/const_emul.py): each is the global',
         '// bytes it sets, in the order first written, and what it returns. Do not edit by hand.',
         '#include "rewrite.h"', '',
         'struct ConstRun {', '    uint32_t at, length;', '    const uint8_t *bytes;', '};',
         'static volatile long g_const_sets;',
         'template <const ConstRun *RUNS, uint32_t COUNT> static void set_runs() {',
         '    g_const_sets++;',
         '    for (uint32_t k = 0; k < COUNT; k++) __builtin_memcpy((void *)(uintptr_t)RUNS[k].at, RUNS[k].bytes, RUNS[k].length);',
         '}',
         '// the calling conventions: the arguments (if any) are left on the stack for the caller or taken off (ret N)',
         'template <const ConstRun *RUNS, uint32_t COUNT, uint32_t EAX> static uint32_t __cdecl set_globals() { set_runs<RUNS, COUNT>(); return EAX; }',
         'template <const ConstRun *RUNS, uint32_t COUNT, uint32_t EAX> static uint32_t __stdcall set_globals1(uint32_t) { set_runs<RUNS, COUNT>(); return EAX; }',
         'template <const ConstRun *RUNS, uint32_t COUNT, uint32_t EAX> static uint32_t __stdcall set_globals2(uint32_t, uint32_t) { set_runs<RUNS, COUNT>(); return EAX; }',
         '']
kept = 0
for a, n, eax, nargs, runs in out:
    if nargs > 2:
        continue
    for i, (ad, bs) in enumerate(runs):
        lines.append(f'static const uint8_t const_{a:x}_{i}[] = {{{", ".join(f"0x{b:02x}" for b in bs)}}};')
    lines.append(f'static const ConstRun const_{a:x}_runs[] = {{' + ', '.join(f'{{{ad:#x}, {len(bs)}, const_{a:x}_{i}}}' for i, (ad, bs) in enumerate(runs)) + '};')
    lines.append(f'static const uint8_t const_{a:x}_original[] = {{' + ', '.join(f'0x{b:02x}' for b in img[a - BASE:a - BASE + min(n, 8)]) + '};')
    entry = {0: 'set_globals', 1: 'set_globals1', 2: 'set_globals2'}[nargs]
    lines.append(f'REWRITE("Globals::SetFixed", {a:#x}, const_{a:x}_original, (&{entry}<const_{a:x}_runs, {len(runs)}, {eax:#x}>), &g_const_sets);')
    kept += 1
lines.append('// each one: its address and how many values it takes (the tester calls it so)')
lines.append('extern const uint32_t const_info[];')
lines.append('const uint32_t const_info[] = {' + ''.join(f'{a:#x}, {nargs}, ' for a, n, eax, nargs, runs in out if nargs <= 2) + '0};')
open(os.path.join(HERE, 'forever', 'src', 'rewrite', 'consts_emul.cpp'), 'w', newline='\n').write('\n'.join(lines) + '\n')
print(kept, 'functions,', sum(n for a, n, e, k, r in out if k <= 2), 'bytes')
for a, n, eax, nargs, runs in sorted(out, key=lambda o: -o[1])[:15]:
    print(hex(a), n, 'args', nargs, 'runs', len(runs), 'bytes set', sum(len(b) for _, b in runs))
