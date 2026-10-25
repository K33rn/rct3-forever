"""The game's own class list, read out of RCT3plus.exe, written to docs/notes/classes.md.

usage: python tools/classes.py "<game folder>\\RCT3plus.exe"

The game describes its object classes to itself: at start-up, for every class, a small initializer creates a class
description (0x805FD0: base class, the class's name, a factory function) and keeps it in a global. The factory
allocates an object of the class and calls its constructor; the constructor sets up the object's virtual function
tables and members, and the first time the class is used, registers the fields the object saves (0x8088C0, with each
field's name and type). Following that chain for every class gives each initializer, factory and constructor its
class, and each constructor the list of fields it registers. Needs pefile and capstone.
"""
import collections
import csv
import os
import struct
import sys

import capstone
import pefile

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BASE = 0x400000
CLASS_INFO = 0x805FD0     # ClassInfo(base, name, factory)
ALLOC = 0xBCDFC0          # the game's allocator
FIELD_TYPES = {'float32', 'int32', 'bool', 'string', 'struct', 'array', 'managedobjectptr', 'uint32', 'int16',
               'uint16', 'int8', 'uint8', 'float64', 'int64', 'uint64', 'matrix44', 'vector3', 'list', 'enum'}


def main():
    exe = sys.argv[1]
    pe = pefile.PE(exe, fast_load=True)
    mem = pe.get_memory_mapped_image()
    funcs = {int(r['address'], 16): r for r in csv.DictReader(open(os.path.join(HERE, 'docs', 'functions.csv')))}
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True

    def code(va):
        n = int(funcs[va]['size']) if va in funcs else 64
        return list(md.disasm(mem[va - BASE:va - BASE + n], va))

    def string(va):
        if not 0xF26000 <= va < 0x141F09C:
            return None
        b = mem[va - BASE:va - BASE + 120]
        end = b.find(b'\0')
        s = b[:end]
        return s.decode() if end >= 2 and all(32 <= c < 127 for c in s) else None

    def imm(ins):
        op = ins.operands[0]
        return op.imm & 0xffffffff if op.type == capstone.x86.X86_OP_IMM else None

    # 1. the initializers: push factory; push "Name"; push base; call ClassInfo; mov [global], eax
    classes = []
    for va in funcs:
        ins = code(va)
        for k, i in enumerate(ins):
            if i.mnemonic != 'call' or imm(i) != CLASS_INFO:
                continue
            pushes = [x for x in ins[max(0, k - 8):k] if x.mnemonic == 'push']
            if len(pushes) < 3:
                continue
            name = string(imm(pushes[-2]) or 0)
            factory = imm(pushes[-3])
            glob = None
            for x in ins[k + 1:k + 4]:
                if x.mnemonic == 'mov' and x.operands[0].type == capstone.x86.X86_OP_MEM and \
                        x.operands[0].mem.base == 0 and x.op_str.endswith('eax'):
                    glob = x.operands[0].mem.disp & 0xffffffff
                    break
            if name and factory in funcs:
                classes.append({'name': name, 'init': va, 'factory': factory, 'info': glob})

    # 2. each factory: allocate size bytes, then call the constructor
    for c in classes:
        size = ctor = None
        last_push = None
        for i in code(c['factory']):
            if i.mnemonic == 'push' and imm(i) is not None:
                last_push = imm(i)
            if i.mnemonic == 'call':
                t = imm(i)
                if t == ALLOC:
                    size = last_push
                elif t is not None and size is not None and ctor is None:
                    ctor = t
        c['size'], c['ctor'] = size, ctor

    # 3. each constructor: its first virtual function table and the saved fields it registers
    for c in classes:
        c['vtable'], c['fields'] = None, []
        if c['ctor'] not in funcs:
            continue
        this = capstone.x86.X86_REG_ECX
        for i in code(c['ctor']):
            # `this` arrives in ecx and is usually kept in another register
            if this == capstone.x86.X86_REG_ECX and i.mnemonic == 'mov' and i.op_str.endswith(', ecx') and \
                    i.operands[0].type == capstone.x86.X86_OP_REG:
                this = i.operands[0].reg
            # the class's own table: the first one stored at offset 0 of `this`
            if c['vtable'] is None and i.mnemonic == 'mov' and len(i.operands) == 2 and \
                    i.operands[0].type == capstone.x86.X86_OP_MEM and i.operands[0].mem.base == this and \
                    i.operands[0].mem.disp == 0 and i.operands[0].mem.index == 0 and \
                    i.operands[1].type == capstone.x86.X86_OP_IMM and \
                    0xF26000 <= (i.operands[1].imm & 0xffffffff) < 0x10E7000:
                c['vtable'] = i.operands[1].imm & 0xffffffff
            if i.mnemonic == 'push' and imm(i) is not None:
                s = string(imm(i))
                # field names are capitalised (or m_...); all-lowercase ones are the field types
                if s and s.isidentifier() and s != s.lower() and s not in c['fields']:
                    c['fields'].append(s)

    classes.sort(key=lambda c: c['name'].lower())
    seen = collections.Counter(c['name'] for c in classes)
    out = ['# The game\'s classes', '',
           'Generated by `tools/classes.py` from the class descriptions RCT3plus.exe creates at start-up: each class\'s',
           'initializer (makes the class description: name, base class, factory), factory (allocates an object of the',
           'class and constructs it) and constructor (sets up the object, and on the class\'s first use registers the',
           'fields it saves, listed here in the order they are registered). Sizes are the bytes the factory allocates.',
           '',
           '| Address | Name | What it does |', '|---|---|---|']
    done = set()
    for c in classes:
        n = c['name']
        if c['init'] not in done:
            out.append(f'| `{c["init"]:#x}` | {n} class description | Creates the class description of {n} '
                       f'(global `{c["info"]:#x}`), with its factory `{c["factory"]:#x}` |' if c['info'] else
                       f'| `{c["init"]:#x}` | {n} class description | Creates the class description of {n}, with its '
                       f'factory `{c["factory"]:#x}` |')
            done.add(c['init'])
        if c['factory'] not in done:
            what = f'allocates {c["size"]} bytes and constructs it' if c['size'] else 'and constructs it'
            out.append(f'| `{c["factory"]:#x}` | {n}::Create | Makes a new {n}: {what}'
                       + (f' (constructor `{c["ctor"]:#x}`)' if c['ctor'] else '') + ' |')
            done.add(c['factory'])
        if c['ctor'] and c['ctor'] not in done and c['ctor'] in funcs:
            f = ', '.join(c['fields'][:40]) + (', ...' if len(c['fields']) > 40 else '')
            vt = f', virtual function table `{c["vtable"]:#x}`' if c['vtable'] else ''
            desc = f'Constructor of {n}{vt}: sets up its members' + \
                   (f', and on first use registers its saved fields: {f}' if c['fields'] else '')
            out.append(f'| `{c["ctor"]:#x}` | {n}::{n} | {desc} |')
            done.add(c['ctor'])
    open(os.path.join(HERE, 'docs', 'notes', 'classes.md'), 'w', encoding='utf-8', newline='\n').write(
        '\n'.join(out) + '\n')
    total = sum(int(funcs[a]['size']) for a in done if a in funcs)
    print(f'{len(classes)} classes ({sum(1 for v in seen.values() if v > 1)} names twice), {len(done)} functions, '
          f'{total} bytes')


if __name__ == '__main__':
    main()
