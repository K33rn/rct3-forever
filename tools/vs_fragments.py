"""Read the vertex shader fragments the graphics device's constructor (0xC77890) assembles, and write where each
goes, forever/src/rewrite/device_fragments.inc. The constructor has the same block written out 129 times, one per
fragment: make a fragment record around a source text, assemble the text, keep the code, store the record in a slot of
the device. This checks that every block is that same block (the same instructions once the scratch registers, stack
slots and the frame's state byte are set aside) and writes each one's slot and the address of its text. The texts
themselves stay in the game: only their addresses are written.
usage: vs_fragments.py <exe>"""
import collections, os, re, sys
import capstone, pefile

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
pe = pefile.PE(sys.argv[1], fast_load=True); img = pe.get_memory_mapped_image(); BASE = 0x400000
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
AT, SIZE = 0xc77890, 78620
ins = [(x.address, f'{x.mnemonic} {x.op_str}'.strip()) for x in md.disasm(bytes(img[AT - BASE:AT - BASE + SIZE]), AT)]
starts = [i for i in range(len(ins) - 1) if ins[i][1] == 'push 0x10' and ins[i + 1][1] == 'call 0xbcdfc0']
KNOWN = {0xfba0bc, 0xfba090, 0xfba084, 0xbcdfc0, 0xbcdfd0, 0xcf6275, 0xdd696e, 0xc34850}
shapes, table = collections.Counter(), []
for s in starts:
    e = s
    while not (re.fullmatch(r'mov dword ptr \[edx \+ 0x[0-9a-f]+\], eax', ins[e][1]) and ins[e - 2][1] == 'mov edx, dword ptr [ebp - 0x128c]'):
        e += 1
    body = ins[s:e + 1]
    shape = []
    for a, t in body:
        t = 'STATE' if 'ebp - 4]' in t else t
        t = re.sub(r'\[ebp - 0x[0-9a-f]+\]', '[L]', t)
        t = re.sub(r'^(j\w+) 0x[0-9a-f]+', r'\1 T', t)
        t = re.sub(r'\b(eax|ecx|edx)\b', 'R', t)
        t = re.sub(r'\b(al|cl|dl)\b', 'r', t)
        t = re.sub(r'\[R \+ 0x2[0-9a-f]{3}\]', '[R + SLOT]', t)
        shape.append(re.sub(r'0xf[bc][0-9a-f]{4}', 'C', t))
    shapes[tuple(shape)] += 1
    texts = sorted({int(x, 16) for a, t in body for x in re.findall(r'0x[0-9a-f]{6,}', t)} - KNOWN)
    texts = [x for x in texts if not AT <= x < AT + SIZE]
    slot = int(re.search(r'\[edx \+ (0x[0-9a-f]+)\]', body[-1][1]).group(1), 16)
    assert len(texts) == 1 and len(body) == 130, (hex(ins[s][0]), texts, len(body))
    table.append((slot, texts[0], e))
assert len(shapes) == 1 and len(table) == 129, (len(shapes), len(table))
assert all(table[k][2] + 1 == starts[k + 1] for k in range(len(starts) - 1)), 'something between two blocks'
assert len({s for s, t, e in table}) == 129
out = ['// The vertex shader fragments the device\'s constructor assembles, in its order: {slot in the device, address of the',
       '// source text in the game}. Written by tools/vs_fragments.py (which checks the 129 blocks are one block). Do not edit by hand.']
out += [f'    {{{slot:#x}, {text:#x}}},' for slot, text, e in table]
open(os.path.join(HERE, 'forever', 'src', 'rewrite', 'device_fragments.inc'), 'w', newline='\n').write('\n'.join(out) + '\n')
print(len(table), 'fragments; the blocks run from', hex(ins[starts[0]][0]), 'to', hex(ins[table[-1][2]][0]))
