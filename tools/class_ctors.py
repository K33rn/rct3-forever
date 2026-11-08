"""Find the saved classes' constructors that classes.cpp's class_construct can take over, and write their member and
field lists to forever/src/rewrite/class_list.inc. A constructor is taken only when every instruction is understood:
the base (0x800A80) built for the description given or the class's default, member set-up the generator can follow
(stores of values, of pointers into the object, calls on members with plain values), the standard first-use field
registration (0x2C-byte records made by a constructor with plain values, given a table, added with 0x8087E0), the mark,
and nothing more. usage: class_ctors.py  (reads your RCT3plus.exe through tools/gamepath.py)"""
import csv, os, re, sys
import capstone, pefile
from gamepath import HERE, game_exe

pe = pefile.PE(game_exe(), fast_load=True)
img = pe.get_memory_mapped_image()
BASE = 0x400000
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
rows = {int(r['address'], 16): r for r in csv.DictReader(open(os.path.join(HERE, 'docs', 'functions.csv'), encoding='utf-8'))}
taken = set()
for f in os.listdir(os.path.join(HERE, 'forever', 'src', 'rewrite')):
    if f != 'class_list.inc':
        taken |= {int(x, 16) for x in re.findall(r'(?:REWRITE\("[^"]+",|^[A-Z_0-9]+\()\s*(0x[0-9a-fA-F]+)', open(os.path.join(HERE, 'forever', 'src', 'rewrite', f), encoding='utf-8').read(), re.M)}
REG = r'(e[a-d]x|e[sd]i|ebp|ebx)'
num = lambda s: int(s, 0)


def ret_args(fn):
    """How many dwords a function takes off the stack (its first ret)."""
    for x in md.disasm(bytes(img[fn - BASE:fn - BASE + 0x2000]), fn):
        if x.mnemonic == 'ret':
            return num(x.op_str) // 4 if x.op_str else 0
    return None


class Reject(Exception):
    pass


def parse(a, n):
    ins = list(md.disasm(bytes(img[a - BASE:a - BASE + n]), a))
    if not ins or ins[-1].address + ins[-1].size != a + n or ins[-1].mnemonic != 'ret' or ins[-1].op_str != '0xc':
        raise Reject('not ret 0xc')
    t = [x.mnemonic + ' ' + x.op_str for x in ins]
    kb = [k for k, x in enumerate(t) if x == 'call 0x800a80']
    if len(kb) != 1:
        raise Reject('base')
    kb = kb[0]
    # the start: registers and the stack up to the base call
    regs = {'ecx': ('this', 0)}
    depth = 0   # bytes pushed since entry
    pushed = []
    desc = None
    after_jne = False
    for k in range(kb):
        s = t[k]
        if s.startswith(('push -1', 'push 0xe')) and k < 3 or 'fs:[0]' in s:
            if s.startswith('push'):
                depth += 4
            continue
        if (m := re.fullmatch(r'push ' + REG, s)):
            depth += 4
            pushed.append(regs.get(m.group(1)))
        elif (m := re.fullmatch(r'push (0x[0-9a-f]+|\d+)', s)):
            depth += 4
            pushed.append(('const', num(m.group(1))))
        elif (m := re.fullmatch(r'sub esp, (0x[0-9a-f]+|\d+)', s)):
            depth += num(m.group(1))
        elif (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', s)):
            slot = num(m.group(2)) - depth
            regs[m.group(1)] = ('arg', slot // 4) if slot >= 4 and slot % 4 == 0 else None
        elif (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[(0x[0-9a-f]{7})\]', s)) and after_jne:
            if regs.get(m.group(1)) != ('arg', 1):
                raise Reject('desc')
            desc = num(m.group(2))
            regs[m.group(1)] = ('desc',)
        elif (m := re.fullmatch(r'mov ' + REG + ', ' + REG, s)):
            regs[m.group(1)] = regs.get(m.group(2))
        elif (m := re.fullmatch(r'xor ' + REG + ', ' + REG, s)) and m.group(1) == m.group(2):
            regs[m.group(1)] = ('const', 0)
        elif re.fullmatch(r'(test|cmp) ' + REG + ', ' + REG, s) or ins[k].mnemonic in ('jne', 'je'):
            pass
        elif re.fullmatch(r'mov (dword|byte) ptr \[esp \+ (0x[0-9a-f]+|\d+)\], .*', s):
            pass   # locals and the exception frame's state
        else:
            raise Reject('start: ' + s)
        after_jne = ins[k].mnemonic == 'jne'
    if desc is None or len(pushed) < 3 or [p for p in pushed[-3:]][::-1] != [('desc',), ('arg', 2), ('arg', 3)]:
        raise Reject('base arguments %s' % pushed[-3:])
    if regs.get('ecx') != ('this', 0):
        raise Reject('base this')
    # the members: up to the description's first-use check
    for r in ('eax', 'ecx', 'edx'):
        regs.pop(r, None)
    pushed = []
    ops = []
    k = kb + 1
    while k < len(t):
        s = t[k]
        if re.fullmatch(r'(mov al, byte ptr|cmp byte ptr) \[' + REG + r' \+ 4\].*', s):
            break
        if s == 'mov cl, 1' and t[k + 1] == 'call 0xbd0490':
            m = re.fullmatch(r'mov dword ptr \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\], eax', t[k + 2])
            if not m or not regs.get(m.group(1)) or regs[m.group(1)][0] != 'this':
                raise Reject('empty string member')
            ops.append((7, regs[m.group(1)][1] + num(m.group(2) or '0'), 0, 0, []))
            for r in ('eax', 'ecx', 'edx'):
                regs.pop(r, None)
            k += 3
            continue
        elif s == 'call 0x805750':
            # the reference member's empty handle, linked in: mov [M + 4], eax; mov eax, [eax + 0x20]; then the link at M + 8
            m = re.fullmatch(r'mov dword ptr \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\], eax', t[k + 1])
            if not m or not regs.get(m.group(1)) or regs[m.group(1)][0] != 'this' or num(m.group(2) or '0') != 4:
                raise Reject('reference member')
            member = regs[m.group(1)][1]
            body = [re.sub(REG, 'R', x) for x in t[k + 2:k + 9]]
            if body != ['mov R, dword ptr [R + 0x20]', 'mov R, dword ptr [R]', 'mov dword ptr [R], R', 'mov R, dword ptr [R]', 'mov dword ptr [R + 4], R',
                        'mov dword ptr [R + 4], R', 'mov dword ptr [R], R']:
                raise Reject('reference link')
            link = re.fullmatch(r'mov dword ptr \[' + REG + r'\], ' + REG, t[k + 4]).group(1)
            if regs.get(link) != ('this', member + 8):
                raise Reject('reference link register')
            ops.append((6, member, 0, 0, []))
            for r in ('eax', 'ecx', 'edx'):
                regs.pop(r, None)
            k += 9
            continue
        elif (m := re.fullmatch(r'mov (dword|byte) ptr \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\], (.+)', s)) and isinstance(regs.get(m.group(2)), tuple) and regs[m.group(2)][0] == 'this':
            off = regs[m.group(2)][1] + num(m.group(3) or '0')
            v = m.group(4)
            if re.fullmatch(r'0x[0-9a-f]+|\d+', v):
                ops.append((0 if m.group(1) == 'dword' else 1, off, num(v), 0, []))
            elif v in regs and regs[v] and regs[v][0] == 'const' and m.group(1) == 'dword':
                ops.append((0, off, regs[v][1], 0, []))
            elif v in ('bl', 'cl', 'dl', 'al') and regs.get('e' + v[0] + 'x') and regs['e' + v[0] + 'x'][0] == 'const' and m.group(1) == 'byte':
                ops.append((1, off, regs['e' + v[0] + 'x'][1] & 0xff, 0, []))
            elif v in regs and regs[v] and regs[v][0] == 'this' and m.group(1) == 'dword':
                ops.append((2, off, regs[v][1], 0, []))
            elif v in regs and regs[v] and regs[v][0] == 'global' and m.group(1) == 'dword':
                ops.append((5, off, regs[v][1], 0, []))
            else:
                raise Reject('member store: ' + s)
        elif re.fullmatch(r'mov (dword|byte) ptr \[esp \+ (0x[0-9a-f]+|\d+)\], .*', s):
            pass
        elif (m := re.fullmatch(r'lea ' + REG + r', \[' + REG + r' \+ (0x[0-9a-f]+|\d+)\]', s)) and regs.get(m.group(2)) and regs[m.group(2)][0] == 'this':
            regs[m.group(1)] = ('this', regs[m.group(2)][1] + num(m.group(3)))
        elif (m := re.fullmatch(r'lea ecx, \[(e[a-d]x|e[sd]i|ebp|ebx)\]', s)) and regs.get(m.group(1)) and regs[m.group(1)][0] == 'this':
            regs['ecx'] = regs[m.group(1)]
        elif (m := re.fullmatch(r'mov ' + REG + ', ' + REG, s)):
            regs[m.group(1)] = regs.get(m.group(2))
        elif (m := re.fullmatch(r'xor ' + REG + ', ' + REG, s)) and m.group(1) == m.group(2):
            regs[m.group(1)] = ('const', 0)
        elif (m := re.fullmatch(r'mov ' + REG + r', (0x[0-9a-f]+|\d+)', s)):
            regs[m.group(1)] = ('const', num(m.group(2)))
        elif (m := re.fullmatch(r'push (0x[0-9a-f]+|\d+)', s)):
            pushed.append(num(m.group(1)))
        elif (m := re.fullmatch(r'push ' + REG, s)) and regs.get(m.group(1)) and regs[m.group(1)][0] == 'const':
            pushed.append(regs[m.group(1)][1])
        elif (m := re.fullmatch(r'call (0x[0-9a-f]+)', s)):
            fn = num(m.group(1))
            this = regs.get('ecx')
            if not this or this[0] != 'this' or ret_args(fn) != len(pushed) or len(pushed) > 4:
                raise Reject('member call: ' + s)
            ops.append((3, this[1], 0, fn, pushed[::-1]))
            pushed = []
            for r in ('eax', 'ecx', 'edx'):
                regs.pop(r, None)
        elif re.fullmatch(r'(test|cmp) (e[sd]i|ebp|ebx), (e[sd]i|ebp|ebx)', s) or s in ('xor bl, bl', 'mov bl, 1') or ins[k].mnemonic == 'jne':
            pass
        elif (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[(0x[0-9a-f]{7})\]', s)) and num(m.group(2)) == desc:
            regs[m.group(1)] = ('desc',)
        elif (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[(0x[0-9a-f]{7})\]', s)):
            regs[m.group(1)] = ('global', num(m.group(2)))
        elif ins[k].mnemonic == 'pop' and k + 1 < len(t):
            pass   # a register given back early
        else:
            raise Reject('member: ' + s)
        k += 1
    if pushed:
        raise Reject('left pushed')
    # the registration: the standard cast, then field blocks
    rest = t[k:]
    name = 0
    if 'call 0xbeca40' in rest:
        q = rest.index('call 0xbeca40')
        if rest[q + 1:q + 2] != ['push eax'] or not any(x == 'call 0xbecc10' for x in rest[q:q + 5]) or not any(x == 'call 0xbec5e0' for x in rest[q:q + 9]):
            raise Reject('name step')
        name = num(re.fullmatch(r'push (0x[0-9a-f]+)', rest[q - 2]).group(1))
        rest = rest[:q] + [x for x in rest[q:] if x not in ('call 0xbeca40', 'call 0xbecc10', 'call 0xbec5e0')]
    calls = [s for s in rest if s.startswith('call')]
    if calls[:1] != ['call dword ptr [eax + 4]']:
        raise Reject('cast: %s' % calls[:1])
    index = next((num(m.group(1)) for s in rest for m in [re.fullmatch(r'mov e[a-d]x, dword ptr \[(0x[0-9a-f]{7})\]', s)] if m and num(m.group(1)) != desc), None)
    if index is None:
        raise Reject('index')
    fields = []
    j = 0
    other = [s for s in calls[1:] if s not in ('call 0xbcdfc0', 'call 0x8087e0')]
    if len(other) != calls.count('call 0xbcdfc0'):
        raise Reject('registration calls: %s' % other[:3])
    for m_ in re.finditer('', ''):
        pass
    k2 = 0
    while k2 < len(rest):
        if rest[k2] == 'call 0xbcdfc0':
            size = num(re.fullmatch(r'push (0x[0-9a-f]+|\d+)', rest[k2 - 1]).group(1))
            c = next(q for q in range(k2 + 1, len(rest)) if rest[q].startswith('call ') and rest[q] != 'call 0x8087e0')
            args = []
            q = c - 1
            while q > k2 and not rest[q].startswith('mov ecx'):
                q -= 1
            for p in rest[k2 + 1:c]:
                if (m := re.fullmatch(r'push (0x[0-9a-f]+|\d+)', p)):
                    args.append(num(m.group(1)))
                elif (m := re.fullmatch(r'push ' + REG, p)):
                    if not regs.get(m.group(1)) or regs[m.group(1)][0] != 'const':
                        raise Reject('field argument ' + p)
                    args.append(regs[m.group(1)][1])
            table = re.fullmatch(r'mov dword ptr \[' + REG + r'\], (0x[0-9a-f]+)', rest[c + 1])
            if not table or len(args) > 4 or ret_args(num(rest[c][5:])) != len(args):
                raise Reject('field shape')
            fields.append((size, num(rest[c][5:]), num(table.group(2)), args[::-1]))
            k2 = c
        k2 += 1
    if calls.count('call 0x8087e0') != len(fields) or not fields:
        raise Reject('fields')
    # the end: the mark, then giving back
    tail = rest[max(i for i, s in enumerate(rest) if s == 'call 0x8087e0') + 1:]
    for s in tail:
        if re.fullmatch(r'(pop|push) ' + REG, s) or s.startswith(('mov ecx, dword ptr [esp', 'mov dword ptr fs:[0]', 'add esp', 'ret', 'je', 'jne', 'test', 'cmp byte ptr [esp', 'mov al, byte ptr [esp')):
            continue
        if re.fullmatch(r'mov (e[a-d]x), dword ptr \[(0x[0-9a-f]{7})\]', s) and num(s.split('[')[1][:-1]) == desc:
            continue
        if re.fullmatch(r'mov byte ptr \[e[a-d]x \+ 4\], 1', s) or re.fullmatch(r'mov eax, ' + REG, s):
            continue
        raise Reject('end: ' + s)
    return desc, index, ops, fields, name


out = ['// Saved classes\' constructors taken over by class_construct (classes.cpp); written by tools/class_ctors.py.']
done = total = 0
reasons = {}
for a, r in sorted(rows.items()):
    n = int(r['size'])
    if r['status'] != 'documented' or a in taken or n < 150 or n > 4000:
        continue
    if b'\xe8' not in bytes(img[a - BASE:a - BASE + n]) or 'call 0x800a80' not in [x.mnemonic + ' ' + x.op_str for x in md.disasm(bytes(img[a - BASE:a - BASE + n]), a)]:
        continue
    try:
        desc, index, ops, fields, name = parse(a, n)
    except (Reject, StopIteration, AttributeError, ValueError) as e:
        key = str(e)[:70]
        if os.environ.get('WHY') and key.startswith(os.environ['WHY']): print(hex(a), e)
        reasons[key] = reasons.get(key, 0) + 1
        continue
    out.append(f'static const ClassOp class_{a:x}_ops[] = {{' + ', '.join(f'{{{k}, {o:#x}, {v:#x}, {fn:#x}, {len(args)}, {{{", ".join(hex(x) for x in args) or "0"}}}}}' for k, o, v, fn, args in ops) + '};' if ops else f'static const ClassOp class_{a:x}_ops[] = {{{{0, 0, 0, 0, 0, {{0}}}}}};')
    out.append(f'static const ClassField class_{a:x}_fields[] = {{' + ', '.join(f'{{{s:#x}, {c:#x}, {tb:#x}, {len(args)}, {{{", ".join(hex(x) for x in args) or "0"}}}}}' for s, c, tb, args in fields) + '};')
    out.append(f'static const uint8_t class_{a:x}_original[] = {{' + ', '.join(f'0x{b:02x}' for b in img[a - BASE:a - BASE + 7]) + '};')
    out.append(f'REWRITE("Class::Construct", {a:#x}, class_{a:x}_original, (&class_construct<{desc:#x}, {index:#x}, class_{a:x}_ops, {len(ops)}, class_{a:x}_fields, {len(fields)}{f', {name:#x}' if name else ''}>), &g_class_constructs);')
    done += 1
    total += n
open(os.path.join(HERE, 'forever', 'src', 'rewrite', 'class_list.inc'), 'w', newline='\n').write('\n'.join(out) + '\n')
print(done, 'constructors,', total, 'bytes; not taken:', sorted(reasons.items(), key=lambda kv: -kv[1])[:12])
