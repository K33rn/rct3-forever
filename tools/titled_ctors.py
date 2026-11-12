"""Find the window classes' constructors that windows.cpp's titled_construct can take over, and write them to
forever/src/rewrite/titled_list.inc. These constructors all start the same way (the game inlined one function): the
window's text made from a title object and the text given (or a copy of the title), the base class built with the
parent (or the title object), that text and a few more values, the copy let go. Then each class sets its own members.

A constructor is taken when: its front part makes exactly the expected calls in the expected order (the tester checks
what they do), the base gets (parent or title, text) and then values that are constants or the constructor's own
arguments, and every instruction after the base is understood as one of the member steps below, with the mark (the
title object used, +4, when there's no parent) where the code has it.

usage: titled_ctors.py [--why PREFIX] (reads your RCT3plus.exe through tools/gamepath.py)"""
import collections, csv, os, re, sys
import capstone, pefile
from gamepath import HERE, game_exe

pe = pefile.PE(game_exe(), fast_load=True)
img = pe.get_memory_mapped_image()
BASE = 0x400000
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
rows = {int(r['address'], 16): r for r in csv.DictReader(open(os.path.join(HERE, 'docs', 'functions.csv'), encoding='utf-8'))}
REG = r'(e[a-d]x|e[sd]i|ebp|ebx)'
num = lambda s: int(s, 0)
OUT = os.path.join(HERE, 'forever', 'src', 'rewrite', 'titled_list.inc')
mine = set()
if os.path.exists(OUT):
    mine = {int(x, 16) for x in re.findall(r'^REWRITE\("[^"]+", (0x[0-9a-f]+)', open(OUT).read(), re.M)}
SEPARATOR = 0xf2738c


class Reject(Exception):
    pass


CDECL = {0xbcdfc0, 0xbcdfd0, 0xbce000, 0xbce010}   # memory: the caller takes the arguments off


def ret_args(fn):
    """How many dwords a function takes off the stack (its first ret, within the function)."""
    if fn in CDECL:
        return 0
    size = int(rows[fn]['size']) if fn in rows else 0x3000
    for x in md.disasm(bytes(img[fn - BASE:fn - BASE + size]), fn):
        if x.mnemonic == 'ret':
            return num(x.op_str) // 4 if x.op_str else 0
    return None


# The front part's calls, in order, when the text is given and when it isn't (a release is 0xBCE010 then 0xBCE000).
FRONT_CALLS = ['0xbcdfc0', '0xbcf850', '0xbd0490', '0x403be0', '0x403bb0'] + ['0xbce010', '0xbce000'] * 4 + ['0x41c740']


def parse(a, n):
    ins = list(md.disasm(bytes(img[a - BASE:a - BASE + n]), a))
    if ins and ins[-1].mnemonic != 'ret' and ins[-1].address + ins[-1].size == a + n:
        # the listed size can stop before the epilogue (the compiler put it just after): read on to its ret
        for x in md.disasm(bytes(img[a - BASE + n:a - BASE + n + 0x80]), a + n):
            ins.append(x)
            if x.mnemonic in ('ret', 'jmp', 'call') or x.mnemonic.startswith('j'):
                break
        n = ins[-1].address + ins[-1].size - a
    if not ins or ins[-1].address + ins[-1].size != a + n or ins[-1].mnemonic != 'ret' or not ins[-1].op_str:
        raise Reject('end')
    nargs = num(ins[-1].op_str) // 4
    t = [x.mnemonic + ' ' + x.op_str for x in ins]
    at = {x.address: i for i, x in enumerate(ins)}
    calls = [(k, s[5:]) for k, s in enumerate(t) if s.startswith('call ')]
    k41 = next((k for k, c in calls if c == '0x41c740'), None)
    if k41 is None:
        raise Reject('no copy')
    kb = next(k for k, c in calls if k > k41)
    base = t[kb][5:]
    if not re.fullmatch(r'0x[0-9a-f]+', base):
        raise Reject('base call ' + base)
    base = num(base)
    if [c for k, c in calls if k < kb] != FRONT_CALLS:
        raise Reject('front calls')
    if f'push {SEPARATOR:#x}' not in t[:kb]:
        raise Reject('separator')
    # the stack: depth (bytes below the entry's return address) at each instruction, read in order
    depth, regs = 0, {'ecx': ('this',)}
    slots = {}
    title = None
    for k in range(kb + 1):
        s, x = t[k], ins[k]
        if k < kb:
            pass
        if (m := re.fullmatch(r'push (.+)', s)):
            depth += 4
        elif (m := re.fullmatch(r'pop (.+)', s)):
            depth -= 4
        elif (m := re.fullmatch(r'sub esp, (0x[0-9a-f]+|\d+)', s)):
            depth += num(m.group(1))
        elif (m := re.fullmatch(r'add esp, (0x[0-9a-f]+|\d+)', s)):
            depth -= num(m.group(1))
        elif x.mnemonic == 'call' and k < kb:
            fn = num(x.op_str)
            depth -= 4 * (ret_args(fn) or 0)
        if (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', s)):
            slot = num(m.group(2)) - depth
            regs[m.group(1)] = ('arg', slot // 4) if slot >= 4 and slot % 4 == 0 and slot // 4 <= nargs else ('local', slot)
        elif (m := re.fullmatch(r'mov ' + REG + ', ' + REG, s)):
            regs[m.group(1)] = regs.get(m.group(2))
        elif (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[(0x1[0-9a-f]{6})\]', s)):
            regs[m.group(1)] = ('global', num(m.group(2)))
        elif (m := re.fullmatch(r'xor ' + REG + ', ' + REG, s)) and m.group(1) == m.group(2):
            regs[m.group(1)] = ('const', 0)
        elif (m := re.fullmatch(r'mov ' + REG + r', (0x[0-9a-f]+|\d+)', s)):
            regs[m.group(1)] = ('const', num(m.group(2)))
        elif re.match(r'(mov|lea|add|sub|or|and|xor|inc|dec) ' + REG + ',', s):
            regs[re.match(r'\w+ ' + REG, s).group(1)] = None
    # the base call: ecx = this, then (parent or title, text, extra...) pushed in reverse just before it
    if regs.get('ecx') != ('this',) and not (t[kb - 1].startswith('mov ecx, ') and regs.get(t[kb - 1][9:]) == ('this',)):
        pass
    want = ret_args(base)
    if want is None or want < 2 or want > 6:
        raise Reject('base arguments %s' % want)
    pushes = []
    k = kb - 1
    while len(pushes) < want and k > k41:
        if t[k].startswith('push '):
            pushes.append(k)
        k -= 1
    if len(pushes) != want:
        raise Reject('base pushes')
    # pushes[0] is the first argument (parent or title), [1] the text, then the extras in order
    def value_at(k):
        s = t[k][5:]
        if re.fullmatch(r'0x[0-9a-f]+|\d+', s):
            return ('const', num(s))
        # the register's value as of instruction k: look back for its last write
        for q in range(k - 1, k41, -1):
            if (m := re.fullmatch(r'mov ' + s + r', dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', t[q])):
                d = depth_at[q]
                slot = num(m.group(1)) - d
                return ('arg', slot // 4) if slot >= 4 and slot % 4 == 0 and slot // 4 <= nargs else ('local', slot)
            if (m := re.fullmatch(r'xor ' + s + ', ' + s, t[q])):
                return ('const', 0)
            if (m := re.fullmatch(r'mov ' + s + r', (0x[0-9a-f]+|\d+)', t[q])):
                return ('const', num(m.group(1)))
            if (m := re.fullmatch(r'mov ' + s + ', ' + REG, t[q])):
                s = m.group(1)
                continue
            if re.match(r'\w+ ' + s + r'\b', t[q]):
                return ('?', t[q])
        return ('?', s)
    depth_at = []
    d = 0
    for k in range(len(t)):
        depth_at.append(d)
        s, x = t[k], ins[k]
        if s.startswith('push '):
            d += 4
        elif s.startswith('pop '):
            d -= 4
        elif (m := re.fullmatch(r'sub esp, (0x[0-9a-f]+|\d+)', s)):
            d += num(m.group(1))
        elif (m := re.fullmatch(r'add esp, (0x[0-9a-f]+|\d+)', s)):
            d -= num(m.group(1))
        elif x.mnemonic == 'call' and re.fullmatch(r'0x[0-9a-f]+', x.op_str):
            d -= 4 * (ret_args(num(x.op_str)) or 0)
    extras = [value_at(k) for k in pushes[2:]]
    if any(v[0] not in ('const', 'arg') for v in extras):
        raise Reject('base extra %s' % (extras,))
    # the first: ecx chosen from the parent (an argument) or the title global
    q = pushes[0]
    if t[q] != 'push ecx':
        raise Reject('base parent push ' + t[q])
    sel = [k for k in range(k41, kb) if re.fullmatch(r'mov ecx, dword ptr \[(0x1[0-9a-f]{6})\]', t[k])]
    if len(sel) != 1 or ins[sel[0] - 1].mnemonic not in ('jne',):
        raise Reject('parent or title')
    title = num(t[sel[0]][len('mov ecx, dword ptr ['):-1])
    parent = value_at_reg(t, sel[0] - 1, 'ecx', depth_at, nargs, k41)   # what ecx holds when it isn't null
    if parent != ('arg', 1):
        raise Reject('parent is %s' % (parent,))
    if t[pushes[1]] != 'push eax':
        raise Reject('text push')
    # the text given: what's pushed for WideString::AssignWide (0x403BB0) is argument 2
    k3 = next(k for k, c in calls if c == '0x403bb0')
    q = k3 - 1
    while not t[q].startswith('push '):
        q -= 1
    q -= 1
    while not t[q].startswith('push '):
        q -= 1
    if value_at_reg(t, q + 1, t[q][5:], depth_at, nargs, 0) != ('arg', 2):
        raise Reject('text is not argument 2')
    base_this = value_at_reg(t, kb, 'ecx', depth_at, nargs, k41) if False else None
    p = dict(nargs=nargs, base=base, extras=extras, title=title, kb=kb, ins=ins, t=t, depth_at=depth_at, at=at)
    p['ops'] = tail(p)
    return p


FLAGS = ('add', 'sub', 'and', 'or', 'xor', 'inc', 'dec', 'cmp', 'test', 'call', 'neg', 'shl', 'shr', 'sar', 'imul')


def tail(p):
    """The member steps after the base and the copy's release, as ops (kind, off, value, fn, args):
    0 dword store, 1 byte store, 2 store argument value, 3 call fn on the member at +off (thiscall) with constant args,
    5 store the global at value as it is then, 6 store a pointer to +value of the object, 7 store the shared empty wide
    string, 10 store at +off a new object: value bytes made by fn with constant args (null when there was no memory),
    12 the mark (with no parent, the title object's +4 = 1)."""
    ins, t, depth_at, at, nargs = p['ins'], p['t'], p['depth_at'], p['at'], p['nargs']
    kb = p['kb']
    # the copy's release: test byte ptr [esp + N], 1; ...; je END
    j = kb + 1
    while ins[j].mnemonic != 'je':
        j += 1
    start = at.get(int(ins[j].op_str, 0))
    if start is None:
        raise Reject('tail start')
    # the registers as the tail starts: each one's last write before it (in address order), when understood
    def known(reg, before):
        for q in range(before - 1, -1, -1):
            s = t[q]
            if re.fullmatch(r'(push|pop) ' + reg, s) and s.startswith('push'):
                continue
            if s == 'pop ' + reg:
                return None
            if (m := re.fullmatch(r'xor ' + reg + ', ' + reg, s)):
                return ('const', 0)
            if (m := re.fullmatch(r'mov ' + reg + r', (0x[0-9a-f]+|\d+)', s)):
                return ('const', num(m.group(1)))
            if (m := re.fullmatch(r'mov ' + reg + ', ecx', s)) and not any(u.startswith('call') or re.match(r'\w+ ecx,', u) for u in t[:q]):
                return ('this',)
            if (m := re.fullmatch(r'mov ' + reg + ', ' + REG, s)):
                return known(m.group(1), q)
            if (m := re.fullmatch(r'mov ' + reg + r', dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', s)):
                slot = num(m.group(1)) - depth_at[q]
                if slot >= 4 and slot % 4 == 0 and slot // 4 <= nargs:
                    return ('arg', slot // 4)
                return stored(slot, q)
            if re.match(r'(mov|lea|add|sub|or|and|xor|inc|dec|shl|shr|sar|imul|neg|not|movzx|movsx) ' + reg + r'\b', s) or \
                    re.match(r'mov ' + reg[1] + '[lh],', s) or (s.startswith('call') and reg in ('eax', 'ecx', 'edx')):
                return None
        return None

    def stored(slot, before):   # a stack local's last value
        for q in range(before - 1, -1, -1):
            if (m := re.fullmatch(r'mov (dword ptr )?\[esp \+ (0x[0-9a-f]+|\d+)\], ' + REG, t[q])) and num(m.group(2)) - depth_at[q] == slot:
                return known(m.group(3), q)
            if (m := re.fullmatch(r'mov (dword|byte) ptr \[esp \+ (0x[0-9a-f]+|\d+)\], .*', t[q])) and num(m.group(2)) - depth_at[q] == slot:
                return None
        return None

    R = {r: known(r, start) for r in ('eax', 'ebx', 'ecx', 'edx', 'esi', 'edi', 'ebp')}
    if ('this',) not in R.values():
        raise Reject('tail this')
    ops, pushed, nulls, serial, temp = [], [], [], 0, None
    flag = None   # what the flags were last set from: ('test', value)
    k = start
    d = depth_at[start]
    while True:
        s, x = t[k], ins[k]
        val = lambda r: R.get(r)
        if x.mnemonic == 'pop' or s.startswith('mov dword ptr fs:[0]'):
            # the epilogue: registers given back, the exception frame, eax = the object, ret
            for q in range(k, len(t)):
                u = t[q]
                if (z := re.fullmatch(r'mov eax, ' + REG, u)):
                    R['eax'] = val(z.group(1))
                elif not (re.fullmatch(r'pop ' + REG, u) or re.fullmatch(r'mov ecx, dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', u)
                          or u.startswith(('mov dword ptr fs:[0], ecx', 'add esp, ', 'ret '))):
                    raise Reject('epilogue: ' + u)
            if R.get('eax') != ('this',):
                raise Reject('returns')
            break
        if (m := re.fullmatch(r'mov (dword|byte|word) ptr \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\], (.+)', s)) and val(m.group(2)) == ('this',) or \
                (m and val(m.group(2)) and val(m.group(2))[0] == 'member'):
            base_off = 0 if val(m.group(2)) == ('this',) else val(m.group(2))[1]
            off, v, size = base_off + num(m.group(3) or '0'), m.group(4), m.group(1)
            if re.fullmatch(r'0x[0-9a-f]+|\d+', v):
                ops.append(({'dword': 0, 'byte': 1, 'word': 4}[size], off, num(v), 0, []))
            elif size == 'dword' and v in R and val(v):
                w = val(v)
                if w[0] == 'const':
                    ops.append((0, off, w[1], 0, []))
                elif w[0] == 'arg':
                    ops.append((2, off, w[1], 0, []))
                elif w[0] == 'global':
                    ops.append((5, off, w[1], 0, []))
                elif w == ('this',):
                    ops.append((6, off, 0, 0, []))
                elif w[0] == 'member':
                    ops.append((6, off, w[1], 0, []))
                elif w[0] == 'empty':
                    ops.append((7, off, 0, 0, []))
                elif w[0] == 'made':
                    ops.append((10 if w[4] == 'returned' else 11, off, w[1], w[2], w[3], w[6]))
                else:
                    raise Reject('tail store ' + s)
            elif size == 'byte' and v in ('al', 'bl', 'cl', 'dl') and val('e' + v[0] + 'x') and val('e' + v[0] + 'x')[0] in ('const', 'byte'):
                ops.append((1, off, val('e' + v[0] + 'x')[1] & 0xff, 0, []))
            else:
                raise Reject('tail store ' + s)
        elif re.fullmatch(r'mov (dword|byte) ptr \[esp \+ (0x[0-9a-f]+|\d+)\], (0x[0-9a-f]+|\d+|[a-d]l|e[a-d]x|e[sd]i|ebp|ebx)', s):
            pass   # locals and the exception frame's state
        elif (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', s)):
            slot = num(m.group(2)) - d
            R[m.group(1)] = ('arg', slot // 4) if slot >= 4 and slot % 4 == 0 and slot // 4 <= nargs else stored(slot, k)
        elif (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[(0x1[0-9a-f]{6})\]', s)):
            R[m.group(1)] = ('global', num(m.group(2)))
        elif (m := re.fullmatch(r'mov ' + REG + ', ' + REG, s)):
            R[m.group(1)] = val(m.group(2))
        elif (m := re.fullmatch(r'xor ' + REG + ', ' + REG, s)) and m.group(1) == m.group(2):
            R[m.group(1)] = ('const', 0); flag = None
        elif (m := re.fullmatch(r'or ' + REG + r', 0xffffffff', s)):
            R[m.group(1)] = ('const', 0xffffffff); flag = None
        elif (m := re.fullmatch(r'mov ' + REG + r', (0x[0-9a-f]+|\d+)', s)):
            R[m.group(1)] = ('const', num(m.group(2)))
        elif (m := re.fullmatch(r'mov ([abcd])l, (0x[0-9a-f]+|\d+)', s)):
            R['e' + m.group(1) + 'x'] = ('byte', num(m.group(2)) & 0xff)
        elif (m := re.fullmatch(r'lea ' + REG + r', \[' + REG + r' \+ (0x[0-9a-f]+|\d+)\]', s)) and val(m.group(2)) and val(m.group(2))[0] in ('this', 'member'):
            R[m.group(1)] = ('member', (0 if val(m.group(2)) == ('this',) else val(m.group(2))[1]) + num(m.group(3)))
        elif (m := re.fullmatch(r'(test ' + REG + ', ' + REG + '|cmp ' + REG + ', ' + REG + ')', s)):
            a_, b_ = (m.group(2), m.group(3)) if m.group(2) else (m.group(4), m.group(5))
            flag = ('test', val(a_)) if (a_ == b_ or val(b_) == ('const', 0)) else None
        elif (m := re.fullmatch(r'lea ecx, \[esp \+ (0x[0-9a-f]+|\d+)\]', s)):
            R['ecx'] = ('stack', num(m.group(1)) - d)
        elif (m := re.fullmatch(r'call (0x[0-9a-f]+)', s)) and val('ecx') and val('ecx')[0] == 'stack' and not (
                t[k - 1].startswith('lea ecx') and ins[k - 2].mnemonic == 'je' and flag is None):
            # a temporary on the stack, made with constant values (only while making a new object, which it's for)
            fn, slot = num(m.group(1)), val('ecx')[1]
            n_ = ret_args(fn)
            if not nulls or n_ is None or n_ > 3 or n_ > len(pushed) or temp is not None:
                raise Reject('stack temporary %s %s %s %s' % (bool(nulls), n_, len(pushed), temp is not None))
            targs = [pushed.pop() for _ in range(n_)]
            if any(not isinstance(v, int) for v in targs):
                raise Reject('stack temporary values')
            d -= 4 * n_
            temp = (slot, fn, targs, nulls[-1][1][2])
            R['eax'] = ('temp', slot); R['ecx'] = None; R['edx'] = None
            flag = None
        elif (m := re.fullmatch(r'(or|and) ' + REG + r', (0x[0-9a-f]+|\d+)', s)) and (val(m.group(2)) is None or val(m.group(2))[0] in ('const', 'byte', 'eh')):
            R[m.group(2)] = ('eh',); flag = None   # the exception bookkeeping bits (what's been built)
        elif (m := re.fullmatch(r'test ([abcd])l, (0x[0-9a-f]+|\d+)', s)) and val('e' + m.group(1) + 'x') in (('eh',),):
            flag = ('ehbit', num(m.group(2)))
        elif x.mnemonic == 'je' and flag and flag[0] == 'ehbit':
            # the temporary destroyed when it was built: lea ecx, [esp + N]; call D; then the jump's target
            z = re.fullmatch(r'lea ecx, \[esp \+ (0x[0-9a-f]+|\d+)\]', t[k + 1])
            z2 = re.fullmatch(r'call (0x[0-9a-f]+)', t[k + 2])
            if not z or not z2 or at.get(int(x.op_str, 0)) != k + 3 or temp is None or num(z.group(1)) - d != temp[0] or ret_args(num(z2.group(1))) != 0:
                raise Reject('temporary destroyed')
            ops.append((15, 0, 0, num(z2.group(1)), []))
            temp = None
            for r in ('eax', 'ecx', 'edx'):
                R[r] = None
            k += 3
            flag = None
            continue
        elif (m := re.fullmatch(r'xor ([abcd])l, ([abcd])l', s)) and m.group(1) == m.group(2):
            R['e' + m.group(1) + 'x'] = ('byte', 0); flag = None
        elif s == 'call 0xbd0490' and val('ecx') in (('byte', 1), ('const', 1)) and not pushed:
            R['eax'] = ('empty',); R['ecx'] = None; R['edx'] = None
            flag = None
        elif (s == 'call 0x805750' or s == 'call 0x805700' and val('ecx') == ('const', 0)) and not pushed:
            # a reference member at M: its handle (the empty one, or the one of no object) at M + 4, and the link at
            # M + 8 put first in the handle's list
            z = re.fullmatch(r'mov dword ptr \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\], eax', t[k + 1])
            if not z or not val(z.group(1)) or val(z.group(1))[0] not in ('this', 'member'):
                raise Reject('reference member')
            member = (0 if val(z.group(1)) == ('this',) else val(z.group(1))[1]) + num(z.group(2) or '0') - 4
            body = [re.sub(REG, 'R', u) for u in t[k + 2:k + 9]]
            if body != ['mov R, dword ptr [R + 0x20]', 'mov R, dword ptr [R]', 'mov dword ptr [R], R', 'mov R, dword ptr [R]', 'mov dword ptr [R + 4], R',
                        'mov dword ptr [R + 4], R', 'mov dword ptr [R], R']:
                raise Reject('reference link')
            link = re.fullmatch(r'mov dword ptr \[' + REG + r'\], ' + REG, t[k + 4]).group(1)
            if val(link) != ('member', member + 8):
                raise Reject('reference link register')
            ops.append((8 if s == 'call 0x805750' else 9, member, 0, 0, []))
            for r in ('eax', 'ecx', 'edx'):
                R[r] = None
            for u in t[k + 2:k + 9]:   # registers written by the link
                if (z := re.fullmatch(r'mov ' + REG + ', .*', u)):
                    R[z.group(1)] = None
            flag = None
            k += 9
            continue
        elif (m := re.fullmatch(r'test ([abcd])l, ([abcd])l', s)) and m.group(1) == m.group(2):
            flag = ('noparent?', 'e' + m.group(1) + 'x')   # a byte flag: set when there was no parent (the tester checks)
        elif (m := re.fullmatch(r'mov ([abcd])l, byte ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', s)):
            R['e' + m.group(1) + 'x'] = ('stackbyte', num(m.group(2)) - d)
        elif (m := re.fullmatch(r'cmp byte ptr \[esp \+ (0x[0-9a-f]+|\d+)\], (0|[abcd]l)', s)) and (m.group(2) == '0' or val('e' + m.group(2)[0] + 'x') in (('const', 0), ('byte', 0))):
            flag = ('noparent?', 'stack')
        elif x.mnemonic == 'je' and flag and flag[0] == 'noparent?':
            m1 = re.fullmatch(r'mov ' + REG + r', dword ptr \[(0x1[0-9a-f]{6})\]', t[k + 1])
            if not m1 or num(m1.group(2)) != p['title'] or t[k + 2] != f'mov byte ptr [{m1.group(1)} + 4], 1' or at.get(int(x.op_str, 0)) != k + 3:
                raise Reject('tail flag mark')
            ops.append((12, 0, 0, 0, []))
            R[m1.group(1)] = None
            k += 3
            flag = None
            continue
        elif x.mnemonic == 'jne' and flag and flag[1] == ('arg', 1):
            # the mark: mov R, [title]; mov byte ptr [R + 4], 1; then the jump's target
            m1 = re.fullmatch(r'mov ' + REG + r', dword ptr \[(0x1[0-9a-f]{6})\]', t[k + 1])
            if not m1 or num(m1.group(2)) != p['title'] or t[k + 2] != f'mov byte ptr [{m1.group(1)} + 4], 1' or at.get(int(x.op_str, 0)) != k + 3:
                raise Reject('tail mark ' + t[k + 1] + ' | ' + t[k + 2])
            ops.append((12, 0, 0, 0, []))
            R[m1.group(1)] = None
            k += 3
            continue
        elif (m := re.fullmatch(r'push (0x[0-9a-f]+|\d+)', s)):
            pushed.append(num(m.group(1))); d += 4
        elif (m := re.fullmatch(r'push ' + REG, s)) and val(m.group(1)) and val(m.group(1))[0] in ('const', 'global', 'arg', 'this', 'member', 'temp'):
            v = val(m.group(1))
            pushed.append(v[1] if v[0] == 'const' else ('self', 0) if v[0] == 'this' else ('self', v[1]) if v[0] == 'member' else
                          ('temp', 0) if v[0] == 'temp' else ('glob' if v[0] == 'global' else 'given', v[1])); d += 4
        elif s == 'call 0xbcdfc0' and pushed and isinstance(pushed[-1], int):
            # a new object: followed as it runs when there is memory (the null branch checked at the jmp past it)
            serial += 1
            R['eax'] = ('alloc', pushed[-1], serial)
            R['ecx'] = None; R['edx'] = None
            flag = None
        elif (m := re.fullmatch(r'add esp, (0x[0-9a-f]+|\d+)', s)) and num(m.group(1)) // 4 <= len(pushed):
            for _ in range(num(m.group(1)) // 4):
                pushed.pop()
            d -= num(m.group(1))
        elif x.mnemonic == 'je' and flag and flag[1] and flag[1][0] == 'alloc':
            nulls.append((int(x.op_str, 0), flag[1]))
        elif x.mnemonic == 'jmp' and nulls and ins[k + 1].address == nulls[-1][0]:
            # past the null branch: it must only set the register the object is in to null, then join here
            target, token = nulls.pop()
            z = re.fullmatch(r'xor ' + REG + ', ' + REG, t[k + 1])
            if not z or z.group(1) != z.group(2) or ins[k + 2].address != int(x.op_str, 0) or not val(z.group(1)) or \
                    val(z.group(1))[0] != 'made' or val(z.group(1))[5] != token[2]:
                raise Reject('new object null')
            k += 2
            flag = None
            continue
        elif (m := re.fullmatch(r'call (0x[0-9a-f]+)', s)) and val('ecx') and val('ecx')[0] == 'alloc':
            fn, token = num(m.group(1)), val('ecx')
            n_ = ret_args(fn)
            if n_ is None or n_ > len(pushed) or n_ > 5:
                raise Reject('new object constructor')
            args = [pushed.pop() for _ in range(n_)]
            d -= 4 * n_
            tinfo = (temp[1], temp[2]) if temp and temp[3] == token[2] else None
            if any(isinstance(v, tuple) and v[0] == 'temp' for v in args) and not tinfo:
                raise Reject('temporary of another object')
            for r in list(R):
                if R[r] == token:
                    R[r] = ('made', token[1], fn, args, 'memory', token[2], tinfo)
            R['eax'] = ('made', token[1], fn, args, 'returned', token[2], tinfo)
            R['ecx'] = None; R['edx'] = None
            flag = None
        elif (m := re.fullmatch(r'call (0x[0-9a-f]+)', s)):
            fn, this = num(m.group(1)), val('ecx')
            if not this or this[0] not in ('this', 'member') or ret_args(fn) != len(pushed) or len(pushed) > 5:
                raise Reject('tail call ' + s)
            ops.append((3, 0 if this == ('this',) else this[1], 0, fn, pushed[::-1]))
            d -= 4 * len(pushed); pushed = []
            for r in ('eax', 'ecx', 'edx'):
                R[r] = None
            flag = None
        else:
            raise Reject('tail: ' + s)
        if x.mnemonic in FLAGS and not s.startswith(('test', 'cmp')):
            flag = None
        k += 1
    if pushed or nulls or temp is not None:
        raise Reject('tail pushed')
    made_with_temp = [o for o in ops if o[0] in (10, 11) and len(o) > 5 and o[5]]
    if len(made_with_temp) != sum(1 for o in ops if o[0] == 15):
        raise Reject('temporaries not destroyed')
    if ops.count((12, 0, 0, 0, [])) > 1:
        raise Reject('two marks')
    return ops


def value_at_reg(t, k, reg, depth_at, nargs, stop):
    for q in range(k - 1, stop, -1):
        if (m := re.fullmatch(r'mov ' + reg + r', dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', t[q])):
            slot = num(m.group(1)) - depth_at[q]
            return ('arg', slot // 4) if slot >= 4 and slot % 4 == 0 and slot // 4 <= nargs else ('local', slot)
        if (m := re.fullmatch(r'mov ' + reg + ', ' + REG, t[q])):
            reg = m.group(1)
            continue
        if re.match(r'\w+ ' + reg + r'\b', t[q]) and not t[q].startswith(('test', 'cmp', 'push')):
            return ('?', t[q])
    return ('?', reg)


if __name__ == '__main__':
    why = sys.argv[sys.argv.index('--why') + 1] if '--why' in sys.argv else None
    reasons = collections.Counter(); rb = collections.Counter(); ok = 0; okb = 0
    out = ["// Window classes' constructors taken over by titled_construct (titled.cpp); written by tools/titled_ctors.py."]
    info, callees, temps_used = [], {}, set()
    for a, r in sorted(rows.items()):
        n = int(r['size'])
        if (r['status'] != 'documented' and a not in mine) or n > 4000:
            continue
        code = bytes(img[a - BASE:a - BASE + n])
        if SEPARATOR.to_bytes(4, 'little') not in code:
            continue
        try:
            p = parse(a, n)
            if p['nargs'] < 2 or p['nargs'] > 6:
                raise Reject('arguments %d' % p['nargs'])
        except (Reject, StopIteration, ValueError, IndexError, KeyError) as e:
            key = (type(e).__name__ + ' ' + str(e))[:60]
            reasons[key] += 1; rb[key] += n
            if why and key.startswith(why):
                print(hex(a), e)
            continue
        ok += 1; okb += n
        ex = p['extras']
        out.append(f'static const TitledExtra titled_{a:x}_extras[] = {{' + (', '.join(f'{{{int(e[0] == "arg")}, {e[1]:#x}}}' for e in ex) or '{0, 0}') + '};')
        def op_text(o):
            kind, off, value, fn, args = o[:5]
            tinfo = o[5] if len(o) > 5 else None
            vals = [x[1] if isinstance(x, tuple) else x for x in args]
            glob = sum(1 << q for q, x in enumerate(args) if isinstance(x, tuple) and x[0] == 'glob')
            given = sum(1 << q for q, x in enumerate(args) if isinstance(x, tuple) and x[0] == 'given')
            selfs = sum(1 << q for q, x in enumerate(args) if isinstance(x, tuple) and x[0] == 'self')
            temps = sum(1 << q for q, x in enumerate(args) if isinstance(x, tuple) and x[0] == 'temp')
            tfn, targs = tinfo if tinfo else (0, [])
            return (f'{{{kind}, {off:#x}, {value:#x}, {fn:#x}, {len(args)}, {{{", ".join(hex(v) for v in vals) or "0"}}}, {glob:#x}, {given:#x}, {selfs:#x}, '
                    f'{temps:#x}, {tfn:#x}, {len(targs)}, {{{", ".join(hex(v) for v in targs) or "0"}}}}}')
        out.append(f'static const TitledOp titled_{a:x}_ops[] = {{' + (', '.join(op_text(o) for o in p['ops']) or '{0, 0, 0, 0, 0, {0}, 0, 0, 0, 0, 0, 0, {0}}') + '};')
        out.append(f'static const uint8_t titled_{a:x}_original[] = {{' + ', '.join(f'0x{b:02x}' for b in img[a - BASE:a - BASE + 7]) + '};')
        out.append(f'REWRITE("Window::Titled", {a:#x}, titled_{a:x}_original, (&titled_construct{p["nargs"]}<{p["title"]:#x}, {p["base"]:#x}, titled_{a:x}_extras, {len(ex)}, '
                   f'titled_{a:x}_ops, {len(p["ops"])}>), &g_titled_constructs);')
        info.append((a, p['title'], p['nargs'], p['base'], 2 + len(ex)))
        for o in p['ops']:
            if o[0] in (3, 10, 11):
                callees[o[3]] = len(o[4])
            if len(o) > 5 and o[5]:
                temps_used.add(o[5][0])
            if o[0] == 15:
                temps_used.add(o[3])
    out.append('// each constructor: its address, title global, how many values it takes, its base and how many values that takes')
    out.append('extern const uint32_t titled_info[];')
    out.append('const uint32_t titled_info[] = {' + ''.join(f'{a:#x}, {tl:#x}, {na}, {b:#x}, {bn}, ' for a, tl, na, b, bn in info) + '0};')
    out.append('// the functions the members are made with or called, and how many values each takes (the tester stands in for them)')
    out.append('extern const uint32_t titled_callees[];')
    out.append('const uint32_t titled_callees[] = {' + ''.join(f'{fn:#x}, {c}, ' for fn, c in sorted(callees.items())) + '0};')
    if temps_used - {0x4040f0, 0xbd1880}:
        print('temporaries the tester has no stand-in for:', [hex(x) for x in temps_used - {0x4040f0, 0xbd1880}])
    open(OUT, 'w', newline='\n').write('\n'.join(out) + '\n')
    print(ok, 'constructors,', okb, 'bytes')
    for k, v in rb.most_common(15):
        print(f'{v:7d} {reasons[k]:4d} {k}')
