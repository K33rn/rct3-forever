"""What a function in RCT3plus.exe touches, to help work out what it does.

usage: python tools/describe.py "<game folder>\\RCT3plus.exe" 0x707680 [0x...]   digest of these functions
       python tools/describe.py "<game folder>\\RCT3plus.exe" --biggest 50 [--skip 0]   the biggest ones not yet done

For each function: its size and status; the functions it calls (with the names known so far) and the ones that call
it; the text strings it uses; the globals it reads or writes; the float constants it loads; how many virtual calls
and switch tables it has. Names come from docs/functions.csv and the tables in docs/notes/*.md. The disassembly of
the whole program is cached next to the exe's path in out/ (first run takes a few minutes). Needs pefile and
capstone (uv run --with pefile --with capstone).
"""
import collections
import csv
import glob
import os
import pickle
import re
import struct
import sys

import capstone
import pefile

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BASE = 0x400000
TEXT = (0x401000, 0x401000 + 0xB24CE5)
RDATA = (0xF26000, 0x10E7000)
DATA = (0x10E7000, 0x10E7000 + 0x33809C)


def functions():
    return {int(r['address'], 16): r for r in csv.DictReader(open(os.path.join(HERE, 'docs', 'functions.csv')))}


def vtable_slots(img):
    """function -> [(class, slot)] for the virtual function tables listed in docs/notes/classes.md"""
    slots = collections.defaultdict(list)
    p = os.path.join(HERE, 'docs', 'notes', 'classes.md')
    if not os.path.exists(p):
        return slots
    for m in re.finditer(r'\| (\w+)::\1 \| Constructor of \w+, virtual function table `0x([0-9a-f]+)`',
                         open(p, encoding='utf-8').read()):
        vt = int(m.group(2), 16)
        for k in range(200):
            f = img.u32(vt + 4 * k)
            if not TEXT[0] <= f < TEXT[1]:
                break
            slots[f].append((m.group(1), k))
    return slots


def notes_names():
    """`| `0xADDR` | Name | ...` rows of docs/notes/*.md"""
    names = {}
    for p in glob.glob(os.path.join(HERE, 'docs', 'notes', '*.md')):
        for m in re.finditer(r'^\|\s*`0x([0-9a-fA-F]+)`\s*\|\s*([^|]+?)\s*\|', open(p, encoding='utf-8').read(), re.M):
            names[int(m.group(1), 16)] = m.group(2).strip('`* ')
    return names


class Image:
    def __init__(self, exe):
        self.pe = pefile.PE(exe, fast_load=True)
        self.mem = self.pe.get_memory_mapped_image()

    def bytes(self, va, n):
        return self.mem[va - BASE:va - BASE + n]

    def u32(self, va):
        return struct.unpack_from('<I', self.mem, va - BASE)[0]

    def string(self, va):
        """the text at va (ASCII, or UTF-16), if it looks like text"""
        if not (RDATA[0] <= va < DATA[1]):
            return None
        b = self.bytes(va, 200)
        m = re.match(rb'[\t\n\r -~]{3,}\x00', b)
        if m:
            return m.group(0)[:-1].decode()
        m = re.match(rb'(?:[\t\n\r -~]\x00){3,}\x00\x00', b)
        if m:
            return 'L"' + m.group(0)[:-2].decode('utf-16le') + '"'
        return None


def analyse(img, funcs):
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True
    out = {}
    for va, r in funcs.items():
        n = int(r['size'])
        info = {'calls': [], 'strings': [], 'globals': collections.Counter(), 'floats': [], 'vcalls': 0,
                'switches': 0, 'jumps_out': []}
        for ins in md.disasm(img.bytes(va, n), va):
            if ins.mnemonic == 'call':
                op = ins.operands[0]
                if op.type == capstone.x86.X86_OP_IMM:
                    info['calls'].append(op.imm)
                elif op.type == capstone.x86.X86_OP_MEM and op.mem.base != 0:
                    info['vcalls'] += 1
                elif op.type == capstone.x86.X86_OP_MEM and op.mem.base == 0 and op.mem.index == 0:
                    info['calls'].append(('import', op.mem.disp & 0xffffffff))
            if ins.mnemonic == 'jmp':
                op = ins.operands[0]
                if op.type == capstone.x86.X86_OP_IMM and not (va <= op.imm < va + n):
                    info['jumps_out'].append(op.imm)
                if op.type == capstone.x86.X86_OP_MEM and op.mem.index != 0 and op.mem.scale == 4:
                    info['switches'] += 1
            for op in ins.operands:
                v = None
                if op.type == capstone.x86.X86_OP_IMM:
                    v = op.imm & 0xffffffff
                elif op.type == capstone.x86.X86_OP_MEM and op.mem.base == 0 and op.mem.index == 0:
                    v = op.mem.disp & 0xffffffff
                if v is None or ins.mnemonic in ('call', 'jmp') or not (RDATA[0] <= v < DATA[1]):
                    continue
                s = img.string(v)
                if s is not None:
                    info['strings'].append(s)
                elif ins.mnemonic.startswith('fld') or ins.mnemonic.startswith('fmul') or \
                        ins.mnemonic.startswith('fadd') or ins.mnemonic.startswith('fcom') or \
                        ins.mnemonic.startswith('fsub') or ins.mnemonic.startswith('fdiv'):
                    if op.size == 4:
                        info['floats'].append(struct.unpack('<f', img.bytes(v, 4))[0])
                    elif op.size == 8:
                        info['floats'].append(struct.unpack('<d', img.bytes(v, 8))[0])
                elif DATA[0] <= v < DATA[1]:
                    info['globals'][v] += 1
        out[va] = info
    return out


def load(exe, funcs):
    cache = os.path.join(HERE, 'out', 'describe.cache')
    if os.path.exists(cache):
        return pickle.load(open(cache, 'rb'))
    img = Image(exe)
    a = analyse(img, funcs)
    imports = {}
    img.pe.parse_data_directories(directories=[pefile.DIRECTORY_ENTRY['IMAGE_DIRECTORY_ENTRY_IMPORT']])
    for d in img.pe.DIRECTORY_ENTRY_IMPORT:
        for i in d.imports:
            imports[i.address] = (i.name or b'?').decode()
    os.makedirs(os.path.dirname(cache), exist_ok=True)
    pickle.dump((a, imports), open(cache, 'wb'))
    return a, imports


def main():
    exe = sys.argv[1]
    funcs = functions()
    a, imports = load(exe, funcs)
    slots = vtable_slots(Image(exe))
    names = {va: r['name'] for va, r in funcs.items() if r['name']}
    names.update(notes_names())
    callers = collections.defaultdict(set)
    for va, info in a.items():
        for c in info['calls'] + info['jumps_out']:
            if isinstance(c, int):
                callers[c].add(va)

    def name(v):
        if isinstance(v, tuple):
            return imports.get(v[1], hex(v[1]))
        return f'{v:#x}' + (f' {names[v]}' if v in names else '')

    args = sys.argv[2:]
    if args and args[0] == '--biggest':
        k = int(args[1])
        skip = int(args[3]) if len(args) > 3 and args[2] == '--skip' else 0
        todo = sorted((va for va, r in funcs.items() if r['status'] == 'to do' and va not in names),
                      key=lambda v: -int(funcs[v]['size']))
        addrs = todo[skip:skip + k]
    else:
        addrs = [int(x, 16) for x in args]
    for va in addrs:
        r, info = funcs[va], a[va]
        print(f'== {va:#x} {names.get(va, "")}  {r["size"]} bytes, {r["status"]}')
        if slots.get(va):
            print('  virtual: ' + ', '.join(f'{c} slot {k}' for c, k in slots[va][:8]) +
                  (f' (+{len(slots[va]) - 8} more)' if len(slots[va]) > 8 else ''))
        cs = collections.Counter(name(c) for c in info['calls'])
        if cs:
            print('  calls: ' + ', '.join(f'{k}' + (f' x{n}' if n > 1 else '') for k, n in cs.most_common(40)))
        if info['jumps_out']:
            print('  jumps to: ' + ', '.join(name(c) for c in info['jumps_out'][:8]))
        cl = sorted(callers.get(va, ()))
        if cl:
            print(f'  called from ({len(cl)}): ' + ', '.join(name(c) + (f' [{slots[c][0][0]}.{slots[c][0][1]}]' if slots.get(c) else '') for c in cl[:12]))
        if info['strings']:
            ss = list(dict.fromkeys(info['strings']))
            print(f'  strings ({len(ss)}): ' + ' | '.join(s[:70] for s in ss[:60]))
        if info['globals']:
            print('  globals: ' + ', '.join(f'{g:#x}' + (f' x{n}' if n > 1 else '')
                                            for g, n in info['globals'].most_common(16)))
        if info['floats']:
            fs = list(dict.fromkeys(round(f, 6) for f in info['floats']))
            print('  floats: ' + ', '.join(f'{f:g}' for f in fs[:24]))
        if info['vcalls'] or info['switches']:
            print(f'  virtual calls {info["vcalls"]}, switch tables {info["switches"]}')


if __name__ == '__main__':
    main()
