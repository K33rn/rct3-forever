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
mine = set()   # the constructors taken before (functions.csv already calls them rewritten)
if os.path.exists(os.path.join(HERE, 'forever', 'src', 'rewrite', 'class_list.inc')):
    mine = {int(x, 16) for x in re.findall(r'^REWRITE\("[^"]+", (0x[0-9a-f]+)', open(os.path.join(HERE, 'forever', 'src', 'rewrite', 'class_list.inc')).read(), re.M)}
REG = r'(e[a-d]x|e[sd]i|ebp|ebx)'
num = lambda s: int(s, 0)


def ret_args(fn):
    """How many dwords a function takes off the stack (its first ret)."""
    for x in md.disasm(bytes(img[fn - BASE:fn - BASE + 0x2000]), fn):
        if x.mnemonic == 'ret':
            return num(x.op_str) // 4 if x.op_str else 0
    return None


def written(r, lines):
    """Whether any of these instructions may change register r (a call changes eax, ecx and edx)."""
    low = {'eax': 'al', 'ebx': 'bl', 'ecx': 'cl', 'edx': 'dl'}.get(r)
    for x in lines:
        if x.startswith('call') and r in ('eax', 'ecx', 'edx'):
            return True
        if re.match(r'(mov|lea|xor|or|and|add|sub|inc|dec|neg|not|shl|shr|sar|imul|movzx|movsx|setne|sete|xchg) (' + r + (('|' + low) if low else '') + r')', x) or x == 'pop ' + r:
            return True
    return False


def walk_fields(ins, t, k0, regs0):
    """The field registration, from the cast to the last field added or closed, followed value by value. Each field: a
    record allocated (0xBCDFC0), constructed with plain values, given its table, then added (0x8087E0) to the class's
    field list (L, the class's part + 0x1C) or to the list of a struct field made before (its +4, or L when that one is
    null); a struct field may then be closed (its slot 0). Returns the entries in order -- (size, ctor, table, args,
    parent) for a field, parent -1 for L or the entry index of the struct, and (0, 0, 0, [], entry) for a close -- and
    the index of the last instruction taken."""
    regs = dict(regs0)
    stack, pushed, depth = {}, [], 0
    entries, field_entry = [], {}   # field number -> its entry index once added
    made = []                       # field number -> [size, ctor, args, table]
    nulls = []                      # (null path address, field number) of records being made
    tested, strict = None, False
    adds = [q for q in range(k0, len(t)) if t[q] == 'call 0x8087e0']
    if not adds:
        raise Reject('fields')
    last = adds[-1]
    q = last + 1
    while q < len(t) and re.fullmatch(r'mov ' + REG + r', dword ptr \[' + REG + r'\]|mov ecx, ' + REG + r'|mov ecx, dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]|call dword ptr \[e[a-d]x\]', t[q]):
        if t[q].startswith('call'):
            last = q
        q += 1

    def val(x):
        if re.fullmatch(r'0x[0-9a-f]+|\d+', x):
            return ('const', num(x))
        return regs.get(x)

    def plain(v):
        if v and v[0] in ('const', 'imm'):
            return v[1]
        raise Reject('field argument %s' % (v,))

    def forget(x):   # an instruction we don't follow: forget the register it writes
        m = re.match(r'[a-z]+ (e[a-d]x|e[sd]i|ebp|[a-d]l)\b', x)
        if m:
            r = m.group(1)
            regs.pop('e' + r[0] + 'x' if r.endswith('l') else r, None)

    seen_cast = False
    k = k0
    while k <= last:
        s = t[k]
        if nulls and ins[k].address == nulls[-1][0]:
            # the null path joins the made path here (je into it, past the constructor and table): keep what both agree on
            _, i, regs_then, stack_then, depth_then = nulls.pop()
            if depth != depth_then or made[i][1] is None or made[i][3] is None:
                raise Reject('field merge')
            # (null on the null path and the record on the made path is the record, which is null when not made)
            agree = lambda v, then: then == v or (v == ('new', i) and then == ('const', 0))
            regs = {r: v for r, v in regs.items() if agree(v, regs_then.get(r))}
            stack = {o: v for o, v in stack.items() if agree(v, stack_then.get(o))}
        if re.fullmatch(r'call dword ptr \[e[a-d]x \+ 4\]', s):
            seen_cast = True
        if s == 'call 0xbcdfc0' and seen_cast:
            strict = True
        if (m := re.fullmatch(r'push (0x[0-9a-f]+|\d+)', s)):
            pushed.append(('imm', num(m.group(1))))
            depth += 4
        elif (m := re.fullmatch(r'push ' + REG, s)):
            pushed.append(regs.get(m.group(1)))
            depth += 4
        elif (m := re.fullmatch(r'pop ' + REG, s)):
            if not pushed:
                raise Reject('field pop')
            pushed.pop()
            depth -= 4
            regs.pop(m.group(1), None)
        elif (m := re.fullmatch(r'add esp, (0x[0-9a-f]+|\d+)', s)):
            n = num(m.group(1))
            depth -= n
            del pushed[len(pushed) - n // 4:]
        elif s == 'call 0xbcdfc0' and strict:
            if not pushed or not pushed[-1] or pushed[-1][0] != 'imm' or len(field_entry) != len(made):
                raise Reject('field allocation')
            made.append([pushed[-1][1], None, None, None])
            for r in ('ecx', 'edx'):
                regs.pop(r, None)
            regs['eax'] = ('new', len(made) - 1)
        elif s == 'call 0x8087e0' and strict:
            f = pushed.pop() if pushed else None
            depth -= 4
            lst = regs.get('ecx')
            if not f or f[0] != 'new' or f[1] != len(made) - 1 or f[1] in field_entry or made[f[1]][3] is None or nulls:
                raise Reject('field add')
            if lst == ('L', 0x1c):
                parent = -1
            elif lst and lst[0] == 'list' and lst[1] in field_entry:
                parent = field_entry[lst[1]]
            else:
                raise Reject('field list %s' % (lst,))
            size, ctor, args, table = made[f[1]]
            field_entry[f[1]] = len(entries)
            entries.append((size, ctor, table, args, parent))
            for r in ('eax', 'ecx', 'edx'):
                regs.pop(r, None)
        elif (m := re.fullmatch(r'call dword ptr \[(e[a-d]x)\]', s)) and strict:
            v, this = regs.get(m.group(1)), regs.get('ecx')
            if not v or v[0] != 'vt' or this != ('new', v[1]) or v[1] not in field_entry:
                raise Reject('field close')
            entries.append((0, 0, 0, [], field_entry[v[1]]))
            for r in ('eax', 'ecx', 'edx'):
                regs.pop(r, None)
        elif (m := re.fullmatch(r'call (0x[0-9a-f]+)', s)):
            n = ret_args(num(m.group(1)))
            if n is None or len(pushed) < n:
                raise Reject('field call arguments: ' + s)
            if strict:
                this = regs.get('ecx')
                if not this or this[0] != 'new' or this[1] != len(made) - 1 or made[-1][1] is not None or n > 4:
                    raise Reject('field constructor: ' + s)
                made[-1][1] = num(m.group(1))
                made[-1][2] = [plain(x) for x in pushed[len(pushed) - n:]][::-1]
            del pushed[len(pushed) - n:]
            depth -= 4 * n
            for r in ('eax', 'ecx', 'edx'):
                regs.pop(r, None)
        elif s.startswith('call'):
            if strict:
                raise Reject('field call: ' + s)
            for r in ('eax', 'ecx', 'edx'):
                regs.pop(r, None)
        elif (m := re.fullmatch(r'mov dword ptr \[' + REG + r'\], (0x[0-9a-f]+)', s)) and strict:
            v = regs.get(m.group(1))
            if not v or v[0] != 'new' or v[1] != len(made) - 1 or made[-1][1] is None or made[-1][3] is not None:
                raise Reject('field table: ' + s)
            made[-1][3] = num(m.group(2))
        elif (m := re.fullmatch(r'mov dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\], (.+)', s)):
            stack[num(m.group(1)) - depth] = val(m.group(2))
        elif re.fullmatch(r'mov byte ptr \[esp \+ (0x[0-9a-f]+|\d+)\], (0x[0-9a-f]+|\d+|[a-d]l)', s):
            pass   # the exception state
        elif (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', s)):
            regs[m.group(1)] = stack.get(num(m.group(2)) - depth)
        elif (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[' + REG + r'\]', s)) and regs.get(m.group(2)) and regs[m.group(2)][0] == 'new':
            regs[m.group(1)] = ('vt', regs[m.group(2)][1])
        elif (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[' + REG + r' \+ 0x10\]', s)) and not strict:
            regs[m.group(1)] = ('part10',)
        elif (m := re.fullmatch(r'mov ' + REG + ', ' + REG, s)):
            regs[m.group(1)] = regs.get(m.group(2))
        elif (m := re.fullmatch(r'xor ' + REG + ', ' + REG, s)) and m.group(1) == m.group(2) and not (strict and nulls):
            regs[m.group(1)] = ('const', 0)
        elif (m := re.fullmatch(r'mov ' + REG + r', (0x[0-9a-f]+|\d+)', s)):
            regs[m.group(1)] = ('const', num(m.group(2)))
        elif (m := re.fullmatch(r'add ' + REG + ', ' + REG, s)) and not strict and ('part10',) in (regs.get(m.group(1)), regs.get(m.group(2))):
            regs[m.group(1)] = ('L', 0)   # the class's part of the description: the description + its offset
        elif (m := re.fullmatch(r'lea ' + REG + r', \[' + REG + r' \+ ' + REG + r'\]', s)) and not strict and ('part10',) in (regs.get(m.group(2)), regs.get(m.group(3))):
            regs[m.group(1)] = ('L', 0)
        elif (m := re.fullmatch(r'add ' + REG + r', (0x[0-9a-f]+|\d+)', s)) and regs.get(m.group(1)) and regs[m.group(1)][0] == 'L':
            regs[m.group(1)] = ('L', regs[m.group(1)][1] + num(m.group(2)))
        elif (m := re.fullmatch(r'lea ' + REG + r', \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\]', s)) and regs.get(m.group(2)) and regs[m.group(2)][0] in ('L', 'new'):
            base, off = regs[m.group(2)], num(m.group(3) or '0')
            if base[0] == 'L':
                regs[m.group(1)] = ('L', base[1] + off)
            elif off == 4:
                regs[m.group(1)] = ('list', base[1])
            else:
                raise Reject('field lea: ' + s)
        elif (m := re.fullmatch(r'(?:test|cmp) ' + REG + ', ' + REG, s)):
            tested = regs.get(m.group(1)) if (m.group(1) == m.group(2) or regs.get(m.group(2)) == ('const', 0)) else None
        elif (m := re.fullmatch(r'je (0x[0-9a-f]+)', s)) and strict:
            # the record not made: its null path (checked at the jmp that ends the made path)
            if not tested or tested[0] != 'new' or tested[1] != len(made) - 1 or nulls:
                raise Reject('field je')
            nulls.append((num(m.group(1)), tested[1], dict(regs), dict(stack), depth))
        elif (m := re.fullmatch(r'jmp (0x[0-9a-f]+)', s)) and strict:
            # the null path sets the same places to null that the made path set to the record, then joins
            if not nulls or ins[k + 1].address != nulls[-1][0]:
                raise Reject('field jmp')
            join, i = num(m.group(1)), nulls.pop()[1]
            j = k + 1
            while ins[j].address != join:
                x = t[j]
                if (mm := re.fullmatch(r'xor ' + REG + ', ' + REG, x)) and mm.group(1) == mm.group(2) and regs.get(mm.group(1)) == ('new', i):
                    pass
                elif (mm := re.fullmatch(r'mov dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\], ' + REG, x)) and regs.get(mm.group(2)) == ('const', 0) and stack.get(num(mm.group(1)) - depth) == ('new', i):
                    pass
                else:
                    raise Reject('field null path: ' + x)
                j += 1
            k = j
            continue
        elif (m := re.fullmatch(r'jne (0x[0-9a-f]+)', s)) and strict:
            # a struct's list, or L when the struct is null: lea ecx, [S + 4]; jne; mov ecx, [L kept]
            lst = regs.get('ecx')
            mm = re.fullmatch(r'mov ecx, dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', t[k + 1])
            mr = re.fullmatch(r'mov ecx, ' + REG, t[k + 1])
            fallback = stack.get(num(mm.group(1)) - depth) if mm else regs.get(mr.group(1)) if mr else None
            if not lst or lst[0] != 'list' or tested != ('new', lst[1]) or fallback != ('L', 0x1c) or ins[k + 2].address != num(m.group(1)):
                raise Reject('field fallback')
            k += 2
            continue
        elif strict:
            raise Reject('field step: ' + s)
        elif not s.startswith(('j', 'cmp', 'test')):
            forget(s)
        k += 1
    if not entries or len(field_entry) != len(made) or nulls:
        raise Reject('fields')
    return entries, last, regs


class Reject(Exception):
    pass


def parse(a, n):
    ins = list(md.disasm(bytes(img[a - BASE:a - BASE + n]), a))
    if not ins or ins[-1].address + ins[-1].size != a + n or ins[-1].mnemonic != 'ret' or ins[-1].op_str not in ('8', '0xc', '0x10', '0x14', '0x18', '0x1c'):
        raise Reject('not 2 to 7 arguments')
    nargs = num(ins[-1].op_str) // 4
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
        elif (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[' + REG + r'(?: \+ (4))?\]', s)) and regs.get(m.group(2)) == ('arg', 2):
            regs[m.group(1)] = ('arg2at', num(m.group(3) or '0'))   # the base's two values given through a pointer
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
    base = [p for p in pushed[-3:]][::-1]
    if desc is None or len(pushed) < 3 or base not in ([('desc',), ('arg', 2), ('arg', 3)], [('desc',), ('arg2at', 0), ('arg2at', 4)]):
        raise Reject('base arguments %s' % pushed[-3:])
    pair = base[1][0] == 'arg2at'
    if regs.get('ecx') != ('this', 0):
        raise Reject('base this')
    # the members: up to the description's first-use check
    for r in ('eax', 'ecx', 'edx'):
        regs.pop(r, None)
    depth -= 4 * len(pushed[-3:])
    pushed = []
    ops = []
    pending = None   # a member made with new, until it is stored
    dirty = set()    # argument slots the constructor wrote over (its locals reuse them): no longer the argument
    k = kb + 1
    while k < len(t):
        s = t[k]
        if re.fullmatch(r'(mov al, byte ptr|cmp byte ptr) \[' + REG + r' \+ 4\].*', s):
            break
        if s == 'mov cl, 1':
            regs['ecx'] = ('cl1',)
        elif s == 'call 0xbd0490' and regs.get('ecx') == ('cl1',):
            for r in ('ecx', 'edx'):
                regs.pop(r, None)
            regs['eax'] = ('empty',)
        elif s == 'call 0x805750' or (s == 'call 0x805700' and regs.get('ecx') and regs['ecx'][0] == 'const'):
            # the reference member's handle, linked in: mov [M + 4], eax; mov eax, [eax + 0x20]; then the link at M + 8.
            # 0x805750 gives the empty handle, 0x805700 (Handle::For) the handle of the object in ecx (a plain value)
            target = None if s == 'call 0x805750' else regs['ecx'][1]
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
            ops.append((6, member, 0, 0, []) if target is None else (9, member, target, 0, []))
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
            elif v in ('bl', 'cl', 'dl', 'al') and regs.get('e' + v[0] + 'x') and regs['e' + v[0] + 'x'][0] in ('const', 'const8') and m.group(1) == 'byte':
                ops.append((1, off, regs['e' + v[0] + 'x'][1] & 0xff, 0, []))
            elif v in regs and regs[v] and regs[v][0] == 'this' and m.group(1) == 'dword':
                ops.append((2, off, regs[v][1], 0, []))
            elif v in regs and regs[v] and regs[v][0] == 'arg' and m.group(1) == 'dword':
                ops.append((8, off, regs[v][1], 0, []))
            elif v in ('bl', 'cl', 'dl', 'al') and regs.get('e' + v[0] + 'x') and regs['e' + v[0] + 'x'][0] == 'arg8' and m.group(1) == 'byte':
                ops.append((12, off, regs['e' + v[0] + 'x'][1], 0, []))   # an argument's low byte (a bool)
            elif v == 'eax' and regs.get('eax') == ('empty',) and m.group(1) == 'dword':
                ops.append((7, off, 0, 0, []))
            elif v == 'eax' and regs.get('eax') == ('made',) and m.group(1) == 'dword' and pending and pending[3] == len(ops):
                ops.append((10, off, pending[0], pending[1], pending[2]))   # nothing else happened since the new: same order
                pending = None
                regs.pop('eax', None)
            elif v in regs and regs[v] and regs[v][0] == 'global' and m.group(1) == 'dword':
                ops.append((5, off, regs[v][1], 0, []))
            else:
                raise Reject('member store: ' + s)
        elif (m := re.fullmatch(r'mov (dword|byte) ptr \[esp \+ (0x[0-9a-f]+|\d+)\], .*', s)):
            slot = num(m.group(2)) - depth - 4 * len(pushed)
            if slot >= 4:
                dirty.add(slot // 4)
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
        elif (m := re.fullmatch(r'or ' + REG + r', 0xffffffff', s)):
            regs[m.group(1)] = ('const', 0xffffffff)
        elif (m := re.fullmatch(r'(?:mov ([a-d])l, (?:(0x[0-9a-f]+|\d+)|([a-d])l)|xor ([a-d])l, \4l)', s)):
            # a low byte written: the whole register stays known when it was, else only its low byte is (const8)
            r = m.group(1) or m.group(4)
            if m.group(4):
                byte = 0
            elif m.group(2):
                byte = num(m.group(2)) & 0xff
            else:
                src = regs.get('e' + m.group(3) + 'x')
                byte = src[1] & 0xff if src and src[0] in ('const', 'const8') else None
            full = regs.get('e' + r + 'x')
            if byte is None:
                regs.pop('e' + r + 'x', None)
            elif full and full[0] == 'const':
                regs['e' + r + 'x'] = ('const', full[1] & ~0xff | byte)
            else:
                regs['e' + r + 'x'] = ('const8', byte)
        elif (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', s)):
            slot = num(m.group(2)) - depth - 4 * len(pushed)
            if slot < 4 or slot % 4 or slot // 4 > nargs or slot // 4 in dirty:   # slot 4: the description as given (registration reloads it)
                raise Reject('member load ' + s)
            regs[m.group(1)] = ('arg', slot // 4)
        elif (m := re.fullmatch(r'mov ([a-d])l, byte ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', s)):
            slot = num(m.group(2)) - depth - 4 * len(pushed)
            if slot < 8 or slot % 4 or slot // 4 > nargs or slot // 4 in dirty:
                raise Reject('member byte load ' + s)
            regs['e' + m.group(1) + 'x'] = ('arg8', slot // 4)   # only the low byte is the argument's
        elif (m := re.fullmatch(r'push (0x[0-9a-f]+|\d+)', s)):
            pushed.append(('imm', num(m.group(1))))
        elif (m := re.fullmatch(r'push ' + REG, s)) and regs.get(m.group(1)) and regs[m.group(1)][0] == 'const':
            pushed.append(('imm', regs[m.group(1)][1]))
        elif (m := re.fullmatch(r'push ' + REG, s)) and regs.get(m.group(1)) and regs[m.group(1)][0] == 'global':
            pushed.append(('glob', regs[m.group(1)][1]))
        elif (m := re.fullmatch(r'push ' + REG, s)) and regs.get(m.group(1)) and regs[m.group(1)][0] == 'this':
            pushed.append(('self', regs[m.group(1)][1]))   # a pointer into the object itself
        elif s == 'call 0xbcdfc0':
            # a member made with new: size bytes (0xBCDFC0), the class's constructor run on them with plain values (or
            # null when there was no memory); the result is stored into the member later (kind 10, see the store)
            if len(pushed) != 1 or pushed[0][0] != 'imm' or pending or t[k + 1] != 'add esp, 4':
                raise Reject('member new')
            size, pushed, j = pushed[0][1], [], k + 2
            while not t[j].startswith('je '):
                ok = re.fullmatch(r'mov (dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\], eax|byte ptr \[esp \+ (0x[0-9a-f]+|\d+)\], (0x[0-9a-f]+|\d+))', t[j]) or t[j] == 'test eax, eax' or (
                    (m := re.fullmatch(r'cmp eax, ' + REG, t[j])) and regs.get(m.group(1)) == ('const', 0))
                if not ok:
                    raise Reject('member new check: ' + t[j])
                j += 1
            null_at = num(t[j][3:])
            args = []
            j += 1
            while t[j] != 'mov ecx, eax':
                if (m := re.fullmatch(r'push (0x[0-9a-f]+|\d+)', t[j])):
                    args.append(('imm', num(m.group(1))))
                elif (m := re.fullmatch(r'push ' + REG, t[j])) and regs.get(m.group(1)) and regs[m.group(1)][0] in ('const', 'global'):
                    args.append(('imm' if regs[m.group(1)][0] == 'const' else 'glob', regs[m.group(1)][1]))
                else:
                    raise Reject('member new argument: ' + t[j])
                j += 1
            cm = re.fullmatch(r'call (0x[0-9a-f]+)', t[j + 1])
            if not cm or len(args) > 4 or ret_args(num(cm.group(1))) != len(args) or not t[j + 2].startswith('jmp ') or ins[j + 3].address != null_at or t[j + 3] != 'xor eax, eax' or ins[j + 4].address != num(t[j + 2][4:]):
                raise Reject('member new shape')
            pending = (size, num(cm.group(1)), args[::-1], len(ops))
            for r in ('ecx', 'edx'):
                regs.pop(r, None)
            regs['eax'] = ('made',)
            k = j + 4
            continue
        elif s == 'call 0xddcf99' and len(pushed) == 5:
            # the compiler's array constructor (eh_vector_constructor_iterator, stdcall): array, size, count, ctor, dtor
            ops.append((11, 0, 0, 0xddcf99, pushed[::-1]))
            pushed = []
            for r in ('eax', 'ecx', 'edx'):
                regs.pop(r, None)
        elif (m := re.fullmatch(r'call (0x[0-9a-f]+)', s)):
            fn = num(m.group(1))
            this = regs.get('ecx')
            if not this or this[0] != 'this' or ret_args(fn) != len(pushed) or len(pushed) > 4:
                raise Reject('member call: ' + s)
            ops.append((3, this[1], 0, fn, pushed[::-1]))
            pushed = []
            for r in ('eax', 'ecx', 'edx'):
                regs.pop(r, None)
        elif re.fullmatch(r'(test|cmp) (e[sd]i|ebp|ebx), (e[sd]i|ebp|ebx)', s) or ins[k].mnemonic == 'jne':
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
    if pending:
        raise Reject('member new not stored')
    # the registration: the standard cast, then field blocks
    rest = t[k:]
    # the name steps: a text made from a name and a value (0xBECA40), given to the description (0xBECC10), let go
    # (0xBEC5E0). The value is a constant, or a member's offset (sub R, esi with R pointing into the object).
    names = []
    steps = [q for q, x in enumerate(rest) if x == 'call 0xbeca40']
    if len(steps) > 2:
        raise Reject('name steps')
    for q in steps:
        if rest[q + 1:q + 2] != ['push eax'] or not any(x == 'call 0xbecc10' for x in rest[q:q + 5]) or not any(x == 'call 0xbec5e0' for x in rest[q:q + 9]):
            raise Reject('name step')
        if not re.fullmatch(r'lea ecx, \[esp \+ (0x[0-9a-f]+|\d+)\]', rest[q - 1]):
            raise Reject('name text')
        nm = re.fullmatch(r'push (0x[0-9a-f]+)', rest[q - 2])
        if not nm:
            raise Reject('name')
        if (m := re.fullmatch(r'push (0x[0-9a-f]+|\d+)', rest[q - 3])):
            value = num(m.group(1))
        elif (m := re.fullmatch(r'push ' + REG, rest[q - 3])) and re.fullmatch(r'sub ' + m.group(1) + r', ' + REG, rest[q - 4]):
            r, base = m.group(1), rest[q - 4].split(', ')[1]
            # the register must still hold what the member set-up left in it: nothing in between writes it
            if regs.get(base) != ('this', 0) or not regs.get(r) or regs[r][0] != 'this' or written(r, rest[:q - 4]) or written(base, rest[:q - 4]):
                raise Reject('name offset')
            value = regs[r][1]
        elif (m := re.fullmatch(r'push ' + REG, rest[q - 3])) and regs.get(m.group(1)) and regs[m.group(1)][0] == 'const' and not written(m.group(1), rest[:q - 3]):
            value = regs[m.group(1)][1]
        else:
            raise Reject('name value: ' + rest[q - 3])
        names.append((num(nm.group(1)), value))
    if steps:
        q = steps[0]
        rest = rest[:q] + [x for x in rest[q:] if x not in ('call 0xbeca40', 'call 0xbecc10', 'call 0xbec5e0')]
    calls = [s for s in rest if s.startswith('call')]
    if not calls or not re.fullmatch(r'call dword ptr \[e[a-d]x \+ 4\]', calls[0]):   # the description's slot 1, through any register
        raise Reject('cast: %s' % calls[:1])
    index = next((num(m.group(1)) for s in rest for m in [re.fullmatch(r'mov e[a-d]x, dword ptr \[(0x[0-9a-f]{7})\]', s)] if m and num(m.group(1)) != desc), None)
    if index is None:
        raise Reject('index')
    fields, last, regs = walk_fields(ins, t, k, regs)
    # the end: the mark, then giving back
    tail = t[last + 1:]
    # what it does with the object then: gives it to a global (instance), links a member into a game list (the list a
    # global points to, or one inside what it points to), or counts it in a global; followed value by value
    instance, count, link, step = 0, 0, None, 0

    def byte_of(r):   # the known low byte of a register, if any
        v = regs.get('e' + r[0] + 'x')
        return v[1] & 0xff if v and v[0] in ('const', 'const8') else None

    for s in tail:
        # a plain value stored into the object after the registration: nothing between touches the object, so it joins
        # the member steps' end
        if (m := re.fullmatch(r'mov (dword|byte) ptr \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\], (0x[0-9a-f]+|\d+|[a-d]l|e[a-d]x|e[sd]i|ebp|ebx)', s)) and regs.get(m.group(2)) and regs[m.group(2)][0] == 'this':
            off, v = regs[m.group(2)][1] + num(m.group(3) or '0'), m.group(4)
            if re.fullmatch(r'0x[0-9a-f]+|\d+', v):
                value = num(v)
            elif m.group(1) == 'byte' and len(v) == 2 and byte_of(v) is not None:
                value = byte_of(v)
            elif m.group(1) == 'dword' and regs.get(v) and regs[v][0] == 'const':
                value = regs[v][1]
            else:
                value = None
            if value is not None:
                ops.append((0 if m.group(1) == 'dword' else 1, off, value, 0, []))
                continue
        if (m := re.fullmatch(r'mov byte ptr \[e[a-d]x \+ 4\], ([a-d]l)', s)) and byte_of(m.group(1)) == 1:
            continue   # the mark, from a register holding 1
        if (m := re.fullmatch(r'mov dword ptr \[(0x[0-9a-f]{7})\], (e[sd]i|ebp|ebx)', s)) and not instance and regs.get(m.group(2)) == ('this', 0):
            instance = num(m.group(1))
            continue
        if (m := re.fullmatch(r'mov (e[a-d]x), dword ptr \[(0x[0-9a-f]{7})\]', s)):
            regs[m.group(1)] = None if num(m.group(2)) == desc else ('gv', num(m.group(2)), -1)
            continue
        if (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\]', s)) and regs.get(m.group(2)) and regs[m.group(2)][0] == 'gv':
            g = regs[m.group(2)]
            if m.group(3) and g[2] < 0:
                regs[m.group(1)] = ('gv', g[1], num(m.group(3)))   # the list inside what the global points to
            elif not m.group(3):
                regs[m.group(1)] = ('first', g)                    # the list's first node
            else:
                raise Reject('end load: ' + s)
            continue
        if (m := re.fullmatch(r'lea ' + REG + r', \[' + REG + r' \+ (0x[0-9a-f]+|\d+)\]', s)) and regs.get(m.group(2)) and regs[m.group(2)][0] == 'this':
            regs[m.group(1)] = ('this', regs[m.group(2)][1] + num(m.group(3)))
            continue
        if (m := re.fullmatch(r'mov dword ptr \[' + REG + r'( \+ 4)?\], ' + REG, s)):
            at, plus4, v = regs.get(m.group(1)), bool(m.group(2)), regs.get(m.group(3))
            # the link, in its order: node[0] = first; first[1] = node; node[1] = head; head[0] = node
            if step == 0 and not plus4 and at and at[0] == 'this' and v and v[0] == 'first':
                link, step = [v[1][1], v[1][2], at[1]], 1
            elif step == 1 and plus4 and v == ('this', link[2]) and at == ('first', ('gv', link[0], link[1])):
                step = 2
            elif step == 2 and plus4 and at == ('this', link[2]) and v == ('gv', link[0], link[1]):
                step = 3
            elif step == 3 and not plus4 and at == ('gv', link[0], link[1]) and v == ('this', link[2]):
                step = 4
            else:
                raise Reject('end store: ' + s)
            continue
        if (m := re.fullmatch(r'inc ' + REG, s)) and regs.get(m.group(1)) and regs[m.group(1)][0] == 'gv' and regs[m.group(1)][2] < 0:
            regs[m.group(1)] = ('counted', regs[m.group(1)][1])
            continue
        if (m := re.fullmatch(r'mov dword ptr \[(0x[0-9a-f]{7})\], ' + REG, s)) and regs.get(m.group(2)) == ('counted', num(m.group(1))) and not count:
            count = num(m.group(1))
            continue
        if (m := re.fullmatch(r'pop ' + REG, s)):
            regs.pop(m.group(1), None)
            continue
        if re.fullmatch(r'push ' + REG, s) or s.startswith(('mov dword ptr fs:[0]', 'add esp', 'ret', 'je', 'jne', 'test', 'cmp byte ptr [esp')):
            continue
        if re.fullmatch(r'mov byte ptr \[e[a-d]x \+ 4\], 1', s):
            continue
        if (m := re.fullmatch(r'mov (e[a-d]x|[a-d]l|e[sd]i|ebp|ebx), (?:byte ptr \[esp \+ (?:0x[0-9a-f]+|\d+)\]|' + REG + r'|dword ptr \[esp \+ (?:0x[0-9a-f]+|\d+)\])', s)):
            r = m.group(1)
            regs.pop(r if len(r) == 3 else 'e' + r[0] + 'x', None)   # the flag, or this reloaded to return it (the test checks the result)
            continue
        raise Reject('end: ' + s)
    if step not in (0, 4):
        raise Reject('end link')
    tail_of = (instance, link, count) if (instance or link or count) else None
    return desc, index, ops, fields, names, nargs, pair, tail_of


out = ['// Saved classes\' constructors taken over by class_construct (classes.cpp); written by tools/class_ctors.py.']
done = total = 0
reasons = {}
for a, r in sorted(rows.items()):
    n = int(r['size'])
    if (r['status'] != 'documented' and a not in mine) or a in taken or n < 150 or n > 4000:
        continue
    if b'\xe8' not in bytes(img[a - BASE:a - BASE + n]) or 'call 0x800a80' not in [x.mnemonic + ' ' + x.op_str for x in md.disasm(bytes(img[a - BASE:a - BASE + n]), a)]:
        continue
    try:
        desc, index, ops, fields, names, nargs, pair, tail_of = parse(a, n)
    except (Reject, StopIteration, AttributeError, ValueError) as e:
        key = str(e)[:70]
        if os.environ.get('WHY') and (key.startswith(os.environ["WHY"]) or os.environ["WHY"] == "*"): print(hex(a), e)
        reasons[key] = reasons.get(key, 0) + 1
        continue
    out.append(f'static const ClassOp class_{a:x}_ops[] = {{' + ', '.join(f'{{{k}, {o:#x}, {v:#x}, {fn:#x}, {len(args)}, {{{", ".join(hex(x[1]) for x in args) or "0"}}}, {sum(1 << q for q, x in enumerate(args) if x[0] == "glob"):#x}, {sum(1 << q for q, x in enumerate(args) if x[0] == "self"):#x}}}' for k, o, v, fn, args in ops) + '};' if ops else f'static const ClassOp class_{a:x}_ops[] = {{{{0, 0, 0, 0, 0, {{0}}, 0, 0}}}};')
    out.append(f'static const ClassField class_{a:x}_fields[] = {{' + ', '.join(f'{{{s:#x}, {c:#x}, {tb:#x}, {len(args)}, {{{", ".join(hex(x) for x in args) or "0"}}}, {par}}}' for s, c, tb, args, par in fields) + '};')
    out.append(f'static const uint8_t class_{a:x}_original[] = {{' + ', '.join(f'0x{b:02x}' for b in img[a - BASE:a - BASE + 7]) + '};')
    if names:
        out.append(f'static const ClassName class_{a:x}_names[] = {{' + ', '.join(f'{{{nm:#x}, {v:#x}}}' for nm, v in names) + '};')
    tail = f', class_{a:x}_names, {len(names)}' if names else ''
    if pair or tail_of:
        tail = (tail or ', nullptr, 0') + (', true' if pair else ', false')
    if tail_of:
        instance, link, count = tail_of
        link = link or [0, -1, 0]
        out.append(f'static const ClassTail class_{a:x}_tail = {{{instance:#x}, {link[0]:#x}, {link[1]}, {link[2]:#x}, {count:#x}}};')
        tail += f', &class_{a:x}_tail'
    out.append(f'REWRITE("Class::Construct", {a:#x}, class_{a:x}_original, (&class_construct{nargs if nargs != 3 else ""}<{desc:#x}, {index:#x}, class_{a:x}_ops, {len(ops)}, class_{a:x}_fields, {len(fields)}{tail}>), &g_class_constructs);')
    done += 1
    total += n
open(os.path.join(HERE, 'forever', 'src', 'rewrite', 'class_list.inc'), 'w', newline='\n').write('\n'.join(out) + '\n')
print(done, 'constructors,', total, 'bytes; not taken:', sorted(reasons.items(), key=lambda kv: -kv[1])[:12])
