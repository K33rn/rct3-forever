"""Read the window lists the park desktop's constructor (RCTDesktop::CreateWindows, 0x6249D0) makes and write them to
forever/src/rewrite/desktop_lists.inc. Between its start (the title, the base desktop, the first member) and its end
(two windows made with new, a few plain members) the constructor is one call after another, each making a window list
member of the desktop:
  kind 0: 0x6B3190 on the member with (file, line, 0, the desktop, a global of the game)
  kind 1: 0x6B3390 on the member with (0, the desktop, a value)
  kind 2: a key combination made on the stack (0xBD1940 with a key and three zeros), then 0x6B3570 on the member with
          (file, line, 0, the desktop, the key combination)
This checks that the stretch holds nothing else (every instruction is one of those calls, one of its pushes, the
member's address, the exception frame's state, or the one plain store the rewrite has by hand) and writes each call's
member, line and global/value/key in order.
usage: desktop_lists.py <exe>"""
import os, re, sys
import capstone, pefile

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
pe = pefile.PE(sys.argv[1], fast_load=True); img = pe.get_memory_mapped_image(); BASE = 0x400000
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
AT, SIZE, FILE = 0x6249d0, 5906, 0xf445b8
ins = [f'{x.mnemonic} {x.op_str}'.strip() for x in md.disasm(bytes(img[AT - BASE:AT - BASE + SIZE]), AT)]
first = ins.index('call 0xbd37a0') + 1
last = ins.index('mov bl, 0x9f')
assert ins[last + 1] == 'push 0x150'
rows, pushed, member, key = [], [], None, None
by_hand = 0
for s in ins[first:last]:
    if (m := re.fullmatch(r'push (0x[0-9a-f]+|\d+)', s)):
        pushed.append(int(m.group(1), 0))
    elif s == 'push esi':
        pushed.append('desktop')
    elif s == 'push ebp':
        pushed.append(0)   # ebp holds 0 through the whole stretch
    elif s in ('push ecx', 'push edx') and key == 'address':
        pushed.append('key')
    elif (m := re.fullmatch(r'lea ecx, \[esi \+ (0x[0-9a-f]+)\]', s)):
        member = int(m.group(1), 16)
    elif s == 'lea ecx, [esp + 0x38]' and len(pushed) == 4:
        member = 'key'           # the key combination's place on the stack (four values pushed: 0x28 + 0x10)
    elif s in ('lea ecx, [esp + 0x28]', 'lea edx, [esp + 0x28]') and not pushed and key == 'made':
        key = 'address'
    elif re.fullmatch(r'mov (byte|dword) ptr \[esp \+ 0x(44|48|4c|50|54|58)\], (0x[0-9a-f]+|\d+)', s):
        pass                     # the exception frame's state
    elif s == 'mov dword ptr [esi + 0x414], ebp' and not rows:
        by_hand += 1             # the one plain store, before the first call
    elif s == 'call 0x6b3190':
        assert len(pushed) == 5 and pushed[4] == FILE and pushed[2] == 0 and pushed[1] == 'desktop' and isinstance(member, int), (pushed, member)
        rows.append((0, member, pushed[3], pushed[0]))
        pushed, member = [], None
    elif s == 'call 0x6b3390':
        assert len(pushed) == 3 and pushed[2] == 0 and pushed[1] == 'desktop' and isinstance(member, int), (pushed, member)
        rows.append((1, member, 0, pushed[0]))
        pushed, member = [], None
    elif s == 'call 0xbd1940':
        assert member == 'key' and pushed[:3] == [0, 0, 0] and key is None, (pushed, member)
        key, key_value, pushed, member = 'made', pushed[3], [], None
    elif s == 'call 0x6b3570':
        assert len(pushed) == 5 and pushed[4] == FILE and pushed[2] == 0 and pushed[1] == 'desktop' and pushed[0] == 'key' and isinstance(member, int), (pushed, member)
        rows.append((2, member, pushed[3], key_value))
        pushed, member, key = [], None, None
    else:
        raise SystemExit('not understood: ' + s)
assert not pushed and member is None and key is None and by_hand == 1
print(len(rows), 'window lists:', [sum(1 for r in rows if r[0] == k) for k in range(3)])
out = ['// The window lists RCTDesktop::CreateWindows makes one after another, in order: {kind, member, line, value} (kind 0:',
       '// 0x6B3190 with a global of the game; 1: 0x6B3390 with a value; 2: 0x6B3570 with a key combination made from the key).',
       '// Written by tools/desktop_lists.py, which checks the stretch holds nothing else. Do not edit by hand.']
out += [f'    {{{k}, {member:#x}, {line:#x}, {value:#x}}},' for k, member, line, value in rows]
open(os.path.join(HERE, 'forever', 'src', 'rewrite', 'desktop_lists.inc'), 'w', newline='\n').write('\n'.join(out) + '\n')
