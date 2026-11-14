"""Read the debug console's number editors (DebugEditor::Int, Int32, UInt32, Float, Int16, UInt16, Int8, UInt8) and write
what differs between them to forever/src/rewrite/debug_editors.inc. The eight constructors are one constructor: a
named base window, a label with its text box, a slider when asked for, and an edit field, the slider and the field
given a range record {table, the value's place, lowest, highest, step}. They differ only in the name, the class table,
the range record's table, and the size of the values the range record holds (4 bytes for Int, Int32, UInt32 and
Float, 2 for Int16 and UInt16, 1 for Int8 and UInt8). This checks that each is the Int editor with only those
differences (jumps compared by the instruction they go to; the exception handler set aside) and writes, for
each, its address, those three addresses, the value size and its first bytes.
usage: debug_editors.py <exe>"""
import os, re, sys
import capstone, pefile

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
pe = pefile.PE(sys.argv[1], fast_load=True); img = pe.get_memory_mapped_image(); BASE = 0x400000
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
EDITORS = [('Int', 0xca88d0, 1206), ('Int32', 0xca8dc0, 1206), ('UInt32', 0xca92b0, 1206), ('Float', 0xcaab80, 1206),
           ('Int16', 0xca97a0, 1218), ('UInt16', 0xca9ca0, 1218), ('Int8', 0xcaa1a0, 1206), ('UInt8', 0xcaa690, 1206)]
# three more with no name: their base window is given an empty text (0xBD05B0) instead of one made from a name
UNNAMED = [('Unnamed', 0xcabc00, 1170), ('Unnamed', 0xcac0d0, 1170), ('Unnamed', 0xcac5a0, 1170)]
SIZES = {0x14: 4, 0x10: 2, 0xc: 1}   # the range record's size -> the size of its values


def read(at, n):
    """The function's instructions with jumps as offsets within it, its constants (name, class table, range table, the
    range record's size), and the same instructions with those constants and the exception handler set aside."""
    ins = [(x.address, f'{x.mnemonic} {x.op_str}') for x in md.disasm(bytes(img[at - BASE:at - BASE + n]), at)]
    assert ins[0][1] == 'push -1' and ins[1][1].startswith('push 0xe') and ins[-1][1] == 'ret 0x1c', at
    index = {a: k for k, (a, t) in enumerate(ins)}   # jumps compared by the instruction they go to
    text = [re.sub(r'0x[0-9a-f]+', lambda m: 'I%d' % index[int(m.group(0), 16)], t) if t.startswith('j') else t for a, t in ins]
    name = [int(t.split()[1], 16) for t in text if re.fullmatch(r'push 0xf[bc][0-9a-f]{4}', t)] or [0]   # (0: none)
    table = [int(t.split(', ')[1], 16) for t in text if re.fullmatch(r'mov dword ptr \[ebp\], 0xf[bc][0-9a-f]{4}', t)]
    ranges = [int(t.split(', ')[1], 16) for t in text if re.fullmatch(r'mov dword ptr \[eax\], 0xf[bc][0-9a-f]{4}', t)]
    assert len(name) == 1 and len(table) == 1 and len(ranges) == 2 and ranges[0] == ranges[1], (hex(at), name, table, ranges)
    record = [int(t.split()[1], 16) for t in text if re.fullmatch(r'push 0x(14|10|c)', t)]
    masked = [re.sub(r'0xf[bc][0-9a-f]{4}', 'K', t) for t in text[2:]]
    return name[0], table[0], ranges[0], record, masked


name0, table0, range0, record0, base = read(0xca88d0, 1206)
# the lines a smaller value size may change: the range record's size and how its values are copied into it
CHANGED = [r'push 0x(14|10|c)', r'mov (e?[a-d]x|[a-d]l), (dword|word|byte) ptr \[esp \+ 0x(4c|50|54|58)\]',
           r'mov (dword|word|byte) ptr \[eax \+ (4|8|9|0xa|0xc|0x10)\], (e?[a-d]x|[a-d]l)', r'mov dword ptr \[eax\], K', r'mov esi, eax',
           r'mov dword ptr \[esp \+ 0x(54|58)\], esp']   # (the exception frame's note of a by-value argument's place, kept in an
                                                          # argument slot no longer read: which slot follows the value size)
rows = []
for label, at, n in EDITORS:
    name, table, rng, record, masked = read(at, n)
    size = 2 if n == 1218 else 1 if 0xc in record else 4   # (the range record's size: 0x10 or 0xC bytes where the Int editor's is 0x14)
    if masked != base:
        import difflib
        diff = [d for d in difflib.unified_diff(base, masked, lineterm='', n=0) if d[:1] in '+-' and not d.startswith(('+++', '---'))]
        bad = [d for d in diff if not any(re.fullmatch(c, d[1:]) for c in CHANGED)]
        assert not bad, (label, bad[:6])
        assert size != 4, label
    rows.append((label, at, name, table, rng, size, bytes(img[at - BASE:at - BASE + 7])))
    print(f'{label:7} {at:#x} name {name:#x} table {table:#x} range table {rng:#x} values of {size} byte(s)')


def unnamed_shape(at, n, named):
    """The instructions with jumps given as how far from the end they go (every jump lands after the base window's name
    is made, where the named and unnamed editors differ), the exception frame's state numbers (small values stored at
    its slots) set aside, the three addresses set aside; for the named editor, its name made from text replaced by the
    unnamed editor's empty text."""
    ins = [(x.address, f'{x.mnemonic} {x.op_str}') for x in md.disasm(bytes(img[at - BASE:at - BASE + n]), at)]
    index = {a: k for k, (a, t) in enumerate(ins)}
    out = []
    for k, (a, t) in enumerate(ins[2:], 2):
        if t.startswith('j'):
            t = t.split()[0] + ' end-%d' % (len(ins) - index[int(t.split()[1], 16)])
        t = re.sub(r'^(mov (?:byte|dword) ptr \[esp \+ 0x(?:3c|40|44|48|4c|50|54|58|5c)\]), (?:0x1?[0-9a-f]|\d|ebx)$', r'\1, STATE', t)
        out.append(re.sub(r'0xf[bc][0-9a-f]{4}', 'K', t))
    if named:   # the name made from text (from push 0x10 to the text kept) becomes the empty text made in place
        i = out.index('push 0x10')
        j = out.index('mov dword ptr [esp + 0x18], eax', i)
        assert out[i + 1:i + 3] == ['mov dword ptr [esp + 0x24], ebp', 'mov dword ptr [esp + 0x14], ebx'], out[i:i + 3]   # (kept: the frame's notes)
        out[i:j + 1] = ['push ebx', 'lea ecx, [esp + 0x1c]', 'mov dword ptr [esp + 0x24], ebp', 'mov dword ptr [esp + 0x14], ebx', 'call 0xbd05b0']
    return out


reference = unnamed_shape(0xca88d0, 1206, True)
for label, at, n in UNNAMED:
    name, table, rng, record, masked = read(at, n)
    shape = unnamed_shape(at, n, False)
    if shape != reference:
        import difflib
        raise SystemExit(f'{at:#x} is not the Int editor without a name: ' + ' | '.join(d for d in difflib.unified_diff(reference, shape, lineterm='', n=0))[:600])
    assert name == 0, hex(at)
    rows.append((label, at, 0, table, rng, 4, bytes(img[at - BASE:at - BASE + 7])))
    print(f'{label:7} {at:#x} no name   table {table:#x} range table {rng:#x} values of 4 byte(s)')

out = ['// The debug console\'s number editors (one constructor, tools/debug_editors.py): {address, name, class table, range',
       '// record table, the size of its values, the first bytes}. Do not edit by hand.']
for label, at, name, table, rng, size, first in rows:
    out.append(f'EDITOR({at:#x}, {name:#x}, {table:#x}, {rng:#x}, {size}, ' + ', '.join(f'0x{b:02x}' for b in first) + f')   // {label}')
open(os.path.join(HERE, 'forever', 'src', 'rewrite', 'debug_editors.inc'), 'w', newline='\n').write('\n'.join(out) + '\n')
