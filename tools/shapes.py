"""Common shapes of small functions, described with their constants, written to docs/notes/shapes.md.

usage: python tools/shapes.py "<game folder>\\RCT3plus.exe"

The compiler makes the same instructions for many small functions that differ only in an address or an offset.
Each shape below was worked out once, by reading one of them; every function with exactly that shape (the same
instructions, constants aside) is then described with its own constants:
  - module start-up: an initializer that starts a module up (0xBCDE60 runs the module's list of initialisers in
    turn, undoing those done when one fails, and gives that failure's error number or -1), throws the error if
    there is one (0xBCDF40) and registers the module's shut-down to run at exit;
  - QueryInterface: given an object, asks its class (virtual slot 1) for the interface with the id kept in a
    global, returning the object plus the interface's offset, or null;
  - cached interface: the same through a reference the object holds, the result kept in the object with a flag;
  - factory: allocates an object of a given size and constructs it, returning it (or null);
  - global clean-up: deletes the object a global points to (virtual slot 0, with 1: destroy and free) and clears
    the global.
"""
import csv
import glob
import os
import re
import sys

import capstone
import pefile

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BASE = 0x400000
ALLOC = 0xBCDFC0


def norm(ins):
    out = []
    for i in ins:
        o = re.sub(r'0x[0-9a-f]+', 'N', i.op_str)
        out.append(i.mnemonic + ' ' + re.sub(r'\b\d+\b', 'N', o))
    return ' ; '.join(out)


MODULE = ('sub esp, N ; mov ecx, esp ; call N ; mov ecx, N ; call N ; push N ; call N ; pop ecx ; ret ')
QI = ('push esi ; mov esi, ecx ; test esi, esi ; je N ; mov eax, dword ptr [esi] ; call dword ptr [eax + N] ; '
      'test eax, eax ; je N ; mov ecx, dword ptr [N] ; cmp ecx, dword ptr [eax + N] ; jge N ; '
      'mov edx, dword ptr [eax + N] ; mov eax, dword ptr [edx + ecx*N] ; test eax, eax ; je N ; '
      'mov eax, dword ptr [eax + N] ; add eax, esi ; pop esi ; ret  ; xor eax, eax ; pop esi ; ret ')
CLEANUP = ('mov ecx, dword ptr [N] ; test ecx, ecx ; je N ; mov eax, dword ptr [ecx] ; push N ; '
           'call dword ptr [eax] ; mov dword ptr [N], N ; ret ')


def main():
    pe = pefile.PE(sys.argv[1], fast_load=True)
    mem = pe.get_memory_mapped_image()
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True
    X = capstone.x86
    funcs = {int(r['address'], 16): r for r in csv.DictReader(open(os.path.join(HERE, 'docs', 'functions.csv')))}
    notes = {}
    for p in glob.glob(os.path.join(HERE, 'docs', 'notes', '*.md')):
        if os.path.basename(p) == 'shapes.md':
            continue
        for m in re.finditer(r'^\| `0x([0-9a-fA-F]+)` \| ([^|]+?) \|', open(p, encoding='utf-8').read(), re.M):
            notes[int(m.group(1), 16)] = m.group(2)

    def nm(t):
        return f'`{t:#x}`' + (f' ({notes[t]})' if t in notes else '')

    groups = {'module': [], 'qi': [], 'cached': [], 'factory': [], 'cleanup': []}
    for va, r in sorted(funcs.items()):
        n = int(r['size'])
        if r['status'] not in ('to do', 'documented') or va in notes or n > 160:
            continue
        ins = [i for i in md.disasm(mem[va - BASE:va - BASE + n], va) if i.mnemonic not in ('int3', 'nop')]
        s = norm(ins)
        imms = lambda: [i.operands[0].imm & 0xffffffff for i in ins if i.mnemonic in ('call', 'push') and
                        i.operands and i.operands[0].type == X.X86_OP_IMM]
        if s == MODULE:
            glob_ = next(i.operands[1].imm & 0xffffffff for i in ins if i.mnemonic == 'mov' and i.op_str.startswith('ecx, 0x'))
            push = next(i.operands[0].imm & 0xffffffff for i in ins if i.mnemonic == 'push')
            groups['module'].append(f'| `{va:#x}` | Module start-up | Starts the module `{glob_:#x}` up at start-up '
                                    f'(its initialisers through `0xBCDE60`, throwing on failure through `0xBCDF40`) and '
                                    f'registers its shut-down `{push:#x}` with `atexit` |')
        elif s == QI:
            g = next(i.operands[1].mem.disp & 0xffffffff for i in ins if i.mnemonic == 'mov' and
                     i.op_str.startswith('ecx, dword ptr [0x'))
            groups['qi'].append(f'| `{va:#x}` | QueryInterface {g:#x} | Returns the given object\'s interface with the '
                                f'id in `{g:#x}` (asking its class through virtual slot 1), or null |')
        elif s == CLEANUP:
            g = ins[0].operands[1].mem.disp & 0xffffffff
            groups['cleanup'].append(f'| `{va:#x}` | Clean-up {g:#x} | Deletes the object the global `{g:#x}` points to '
                                     f'(if any) and clears the global |')
        else:
            calls = [i for i in ins if i.mnemonic == 'call' and i.operands[0].type == X.X86_OP_IMM]
            # factory: push size ; call alloc ; ... call ctor ; ret (with exception frame or not)
            if len(calls) == 2 and calls[0].operands[0].imm & 0xffffffff == ALLOC and n <= 140 and \
                    not any(i.mnemonic == 'call' and i.operands[0].type != X.X86_OP_IMM for i in ins):
                size = None
                for i in ins:
                    if i is calls[0]:
                        break
                    if i.mnemonic == 'push' and i.operands[0].type == X.X86_OP_IMM:
                        size = i.operands[0].imm & 0xffffffff
                ctor = calls[1].operands[0].imm & 0xffffffff
                if size and size < 0x100000:
                    groups['factory'].append(f'| `{va:#x}` | Create | Makes a new object: allocates {size} bytes and '
                                             f'constructs them with {nm(ctor)}; returns it, or null when out of memory |')
                    continue
            # cached interface of a referenced object: uses one global id, one virtual call (slot 1), stores the
            # result and sets a byte flag
            gl = [i for i in ins if any(o.type == X.X86_OP_MEM and o.mem.base == 0 and o.mem.index == 0 and
                                        0x10E7000 <= (o.mem.disp & 0xffffffff) for o in i.operands)]
            vc = [i for i in ins if i.mnemonic == 'call' and i.operands[0].type == X.X86_OP_MEM and
                  i.operands[0].mem.disp == 4]
            flag = [i for i in ins if i.mnemonic == 'mov' and i.op_str.startswith('byte ptr [') and i.op_str.endswith(', 1')]
            if len(vc) == 1 and len(flag) == 1 and not calls and gl and n <= 120:
                g = next(o.mem.disp & 0xffffffff for i in gl for o in i.operands if o.type == X.X86_OP_MEM and o.mem.base == 0)
                fo = flag[0].operands[0].mem.disp
                groups['cached'].append(f'| `{va:#x}` | Interface {g:#x} | Returns the interface with the id in `{g:#x}` of '
                                        f'the object a reference held by this one points to, looking it up only the '
                                        f'first time (kept with the flag at +{fo:#x}), or null |')
    out = ['# Common shapes of small functions', '',
           'Generated by `tools/shapes.py`: functions with one of the common shapes described in the script, each with',
           'its own constants.', '']
    titles = {'module': 'Module start-up', 'qi': 'QueryInterface', 'cached': 'Cached interfaces', 'factory': 'Factories',
              'cleanup': 'Global clean-up'}
    for k, rows in groups.items():
        out += [f'## {titles[k]}', '', '| Address | Name | What it does |', '|---|---|---|'] + rows + ['']
    open(os.path.join(HERE, 'docs', 'notes', 'shapes.md'), 'w', encoding='utf-8', newline='\n').write('\n'.join(out))
    print({k: len(v) for k, v in groups.items()})


if __name__ == '__main__':
    main()
