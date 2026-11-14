"""Read the cheats table's constructor (Cheats::Init, 0x410ED0) and write what differs between its 32 blocks,
forever/src/rewrite/cheats.inc. The constructor has one block written out per cheat: make the text "CheatPeepName_",
add the cheat's own ending to it, let the first text go, make the cheat's entry with that name and a number, and put
it in the table. This checks that the 32 blocks are that one block (the same instructions once the scratch registers,
stack slots, the frame's state byte and the table's place are set aside) that the numbers are 0 to 31 in order and the names are let go last to first at the end, and writes each one's ending
(the address of the text in the game, not the text).
usage: cheat_names.py <exe>"""
import collections, os, re, sys
import capstone, pefile

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
pe = pefile.PE(sys.argv[1], fast_load=True); img = pe.get_memory_mapped_image(); BASE = 0x400000
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
AT, SIZE = 0x410ed0, 8107
ins = [(x.address, f'{x.mnemonic} {x.op_str}'.strip()) for x in md.disasm(bytes(img[AT - BASE:AT - BASE + SIZE + 16]), AT)]
# a block: from "push 0x10; (state); (the last entry stored;) call Alloc" to the entry's constructor call and what follows it
starts = [i for i, (a, t) in enumerate(ins) if t == 'push 0x10' and any(u == 'call 0xbcdfc0' for _, u in ins[i + 1:i + 4])]
makes = [i for i, (a, t) in enumerate(ins) if t == 'call 0x410d50']
assert len(starts) == 32 and len(makes) == 32, (len(starts), len(makes))
shapes, table = collections.Counter(), []
for k, (s, m) in enumerate(zip(starts, makes)):
    body = ins[s:m + 1]
    # the block's skeleton: its calls, the constants it pushes, its branches and what it does to counts. (The moves between
    # registers and stack slots around them are scheduled two ways by the compiler; both put the same values in the same places.)
    shape = []
    for a, t in body:
        m = t.split()[0]
        if m == 'call' or m in ('je', 'jne', 'jmp', 'dec', 'inc', 'test', 'cmp', 'xor') or re.fullmatch(r'push (0x[0-9a-f]+|\d+)', t) or t.startswith('add esp'):
            t = re.sub(r'\[esp \+ 0x[0-9a-f]+\]|\[esp \+ \d+\]', '[L]', t)
            t = re.sub(r'^(j\w+) 0x[0-9a-f]+', r'\1 T', t)
            t = re.sub(r'push 0xf26[0-9a-f]{3}', 'push TEXT', t)
            t = re.sub(r'\b(eax|ecx|edx|esi)\b', 'R', t)
            shape.append('add esp' if t.startswith('add esp') else t)   # (how much the stack is put right by varies: the first block also clears the table's allocation)
    texts = [int(x, 16) for a, t in body for x in re.findall(r'push (0xf26[0-9a-f]{3})', t)]
    assert len(texts) == 2 and texts[0] == 0xf26d30, (k, [hex(x) for x in texts])
    j = next(i for i in range(len(body) - 1, 0, -1) if body[i][1] == 'push ecx')   # the name's place on the stack; the number is pushed just before
    number = int(body[j - 1][1].split()[1], 0)
    after = [t for a, t in body[j + 1:] if t.startswith('push')]
    assert after == ['push 0'], (k, after)                       # the value given before the name is always 0
    pushes = [i for i, t in enumerate(shape) if t.startswith('push')]
    shape[pushes[-2]] = 'push NUMBER'                             # the cheat's number: the push before the last (the 0)
    table.append((texts[1], number))
    shapes[tuple(shape)] += 1
# where each block keeps its name (the place given to the text join, less the one value pushed before it), and the order the
# end of the function lets the names go in
slots = []
for s_, m_ in zip(starts, makes):
    body = ins[s_:m_ + 1]
    j = next(i for i, (a, t) in enumerate(body) if t == 'call 0x403be0')
    lea = [t for a, t in body[j - 4:j] if t.startswith('lea') and 'ecx' not in t.split(',')[0]]
    slots.append(int(re.search(r'esp \+ (0x[0-9a-f]+)', lea[0]).group(1), 16) - 4)
assert len(set(slots)) == 32
tail = ins[makes[-1] + 1:]
released = [int(re.search(r'esp \+ (0x[0-9a-f]+)', tail[i - 1][1]).group(1), 16) for i, (a, t) in enumerate(tail) if t == 'dec dword ptr [eax]']
order = [slots.index(x) for x in released]
assert sorted(order) == list(range(32)), order
print('the names are let go in the order', order)
print(len(shapes), 'shape(s) of block;', [c for s, c in shapes.most_common()])
if len(shapes) > 1:
    a, b = [s for s, c in shapes.most_common(2)]
    print(len(a), len(b))
    for x, y in zip(a, b):
        if x != y:
            print('  differ:', x, '|', y)
assert len(shapes) == 1
assert [n for t, n in table] == list(range(32)), 'the numbers are not 0 to 31 in order'
assert order == list(range(31, -1, -1)), 'the names are not let go last to first'
out = ['// The 32 cheats in the order Cheats::Init makes them (numbered 0 to 31): the address in the game of the text that ends',
       '// each one\'s name. Written by tools/cheat_names.py (which checks the 32 blocks are one block). Do not edit by hand.']
out += [f'    {text:#x},' for text, number in table]
open(os.path.join(HERE, 'forever', 'src', 'rewrite', 'cheats.inc'), 'w', newline='\n').write('\n'.join(out) + '\n')
print([(hex(t), n) for t, n in table[:5]], '...', [(hex(t), n) for t, n in table[-3:]])
