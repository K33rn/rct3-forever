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
# the rewritten functions no other rewrite file takes: (were) this list's, so read again even when a run left one out of it
_others = set()
for _f in os.listdir(os.path.join(HERE, 'forever', 'src', 'rewrite')):
    if _f != os.path.basename(OUT) and _f.endswith(('.cpp', '.inc')):
        _others |= {int(x, 16) for x in re.findall(r'(?:REWRITE\("[^"]+",|^[A-Z_0-9]+\()\s*(0x[0-9a-fA-F]+)', open(os.path.join(HERE, 'forever', 'src', 'rewrite', _f), encoding='utf-8').read(), re.M)}
mine |= {a for a, r in rows.items() if r['status'] == 'rewritten' and a not in _others}
SEPARATOR = 0xf2738c
VSLOTS = {0x20: 1}   # the virtual slots called (op 60) and how many values each takes (0x20: a child's size, the tester's own)
FPU_PROGRAM = []   # the x87 runs' steps (code, kind, value), all constructors' in one table (op 59 gives where its run starts)
# the run steps: x87 instructions as the game has them, replayed one by one (titled.cpp)
FPU_CODES = {'fld': 1, 'fild': 21, 'fst': 3, 'fstp': 4, 'fadd': 5, 'fsub': 6, 'fsubr': 7, 'fmul': 8, 'fdiv': 9, 'fdivr': 10,
             'faddp': 11, 'fsubp': 12, 'fsubrp': 13, 'fmulp': 14, 'fdivp': 15, 'fdivrp': 16}


class Reject(Exception):
    pass


CDECL = {0xbcdfc0, 0xbcdfd0, 0xbce000, 0xbce010}   # memory: the caller takes the arguments off


def ret_args(fn, depth=0):
    """How many dwords a function takes off the stack (its first ret, within the function)."""
    if fn in CDECL:
        return 0
    size = int(rows[fn]['size']) if fn in rows else 0x3000
    for x in md.disasm(bytes(img[fn - BASE:fn - BASE + size]), fn):
        if x.mnemonic == 'ret':
            return num(x.op_str) // 4 if x.op_str else 0
    # (a jump table inside the function stops the straight read: follow the branches from the start instead)
    todo, seen = [fn], set()
    while todo:
        a = todo.pop()
        while fn <= a < fn + size + 0x40 and a not in seen:   # (the listed size can stop before the epilogue)
            seen.add(a)
            x = next(md.disasm(bytes(img[a - BASE:a - BASE + 16]), a), None)
            if x is None:
                break
            if x.mnemonic == 'ret':
                return num(x.op_str) // 4 if x.op_str else 0
            if x.mnemonic.startswith('j') and re.fullmatch(r'0x[0-9a-f]+', x.op_str):
                to = num(x.op_str)
                if x.mnemonic == 'jmp' and not fn <= to < fn + size + 0x40 and depth < 3:
                    return ret_args(to, depth + 1)   # (a jump on into another function: it returns for this one)
                todo.append(to)
                if x.mnemonic == 'jmp':
                    break
            elif x.mnemonic == 'jmp':
                break
            a += x.size
    return None


# The front part's calls, in order, when the text is given and when it isn't (a release is 0xBCE010 then 0xBCE000).
FRONT_CALLS = ['0xbcdfc0', '0xbcf850', '0xbd0490', '0x403be0', '0x403bb0'] + ['0xbce010', '0xbce000'] * 4 + ['0x41c740']


def parse(a, n):
    ins = list(md.disasm(bytes(img[a - BASE:a - BASE + n]), a))
    if ins and ins[-1].mnemonic != 'ret':
        # the listed size can stop before the epilogue (the compiler put it just after), even inside an instruction:
        # read on from the last whole one to its ret
        n = ins[-1].address + ins[-1].size - a
        for x in md.disasm(bytes(img[a - BASE + n:a - BASE + n + 0x200]), a + n):
            ins.append(x)
            if x.mnemonic in ('ret', 'int3'):
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
    # (the parent or the title chosen into a register: ecx, or another the base is then given)
    sel0 = next((k for k in range(k41, len(t)) if re.fullmatch(r'mov ' + REG + r', dword ptr \[(0x1[0-9a-f]{6})\]', t[k]) and ins[k - 1].mnemonic == 'jne'), None)
    if sel0 is None:
        raise Reject('parent or title')
    sel_reg = re.match(r'mov (\w+),', t[sel0]).group(1)
    kb = next(k for k, c in calls if k > sel0 and c != '0x41cb90')

    base = t[kb][5:]
    if not re.fullmatch(r'0x[0-9a-f]+', base):
        raise Reject('base call ' + base)
    base = num(base)
    front = [c for k, c in calls if k < kb]
    # (two front additions: a text made from a literal first of all, its place given to the base (group "start text"),
    # and a text reference made just before the base, given to it and destroyed just after)
    start_text = front[:2] == ['0xbcdfc0', '0xbcf850'] and front[2:4] == ['0xbcdfc0', '0xbcf850']
    if start_text:
        front = front[2:]
    ref_before = front[-1:] == ['0x41cb90']
    if ref_before:
        front = front[:-1]
    if front != FRONT_CALLS:
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
        # the register's value as of instruction k: look back for its last write (past the copy call for the registers
        # a call keeps)
        for q in range(k - 1, -1 if s in ('ebx', 'esi', 'edi', 'ebp') else k41, -1):
            if t[q].startswith('call ') and s in ('eax', 'ecx', 'edx'):
                if t[q] == 'call 0x41cb90' and s == 'eax' and ref_before:
                    return ('ref', q)   # the text reference made just before
                return ('?', t[q])
            if (m := re.fullmatch(r'lea ' + s + r', \[esp \+ (0x[0-9a-f]+|\d+)\]', t[q])):
                return ('place', num(m.group(1)) - depth_at[q])
            if s in ('eax', 'ebx', 'ecx', 'edx') and re.match(r'\w+ ' + s[1] + r'[lh]\b', t[q]) and not t[q].startswith(('push ', 'cmp ', 'test ')):
                return ('?', t[q])   # (a byte of it written: sete dl, mov al, ...)
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
            if re.match(r'\w+ ' + s + r'\b', t[q]) and not t[q].startswith(('push ', 'cmp ', 'test ')):   # (those only read it)
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
    if sel_reg != 'ecx':
        # the base made on the window: the register ecx was kept in at the start, put back just before it, unchanged since
        # (or read back from where the window was kept on the stack)
        km = next((q for q in range(min(24, len(t))) if re.fullmatch(r'mov ' + REG + ', ecx', t[q])), None)
        keep = t[km][4:7] if km is not None else None
        kc = kb - 1   # (the exception state's stores passed over)
        while re.fullmatch(r'mov (byte|dword) ptr \[esp \+ (0x[0-9a-f]+|\d+)\], (0x[0-9a-f]+|\d+|[a-d]l)', t[kc]):
            kc -= 1
        saved = {num(z.group(1)) - depth_at[q] for q in range(km or 0, kc) if keep and
                 (z := re.fullmatch(r'mov dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\], ' + keep, t[q]))}
        def rewrites(q):
            u = t[q]
            if u.startswith('pop ' + keep):
                return True
            if (z := re.fullmatch(r'mov ' + keep + r', dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', u)) and num(z.group(1)) - depth_at[q] in saved:
                return False
            return bool(re.match(r'(?!push|cmp|test)\w+ ' + keep + r'\b', u))
        if not keep or t[kc] != 'mov ecx, ' + keep or any(rewrites(q) for q in range(km + 1, kc)):
            raise Reject('base not on the window')
    extras = [value_at(k) for k in pushes[2:]]
    front_name = None
    if start_text:
        # push 0x10; call 0xBCDFC0; ...; push LIT; mov ecx, eax; call 0xBCF850; jmp; xor eax, eax; mov [esp + N], eax
        kt = [k for k, c in calls][1]
        zl = re.fullmatch(r'push (0x[0-9a-f]+)', t[kt - 2]) if t[kt - 1] == 'mov ecx, eax' else None
        zs = re.fullmatch(r'mov dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\], eax', t[kt + 3]) if t[kt + 2] == 'xor eax, eax' and t[kt + 1].startswith('jmp') else None
        ka = [k for k, c in calls][0]
        kp = max(q for q in range(ka) if t[q].startswith('push '))
        if not zl or not zs or t[kp] != 'push 0x10' or any(not re.fullmatch(r'mov dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\], ' + REG, u) for u in t[kp + 1:ka]):
            raise Reject('start text')
        front_name = (num(zl.group(1)), num(zs.group(1)) - depth_at[kt + 3])
    ref = None
    for i, v in enumerate(extras):
        if v[0] == 'ref':
            q = v[1]
            zz = [re.fullmatch(r'push (0x[0-9a-f]+|\d+)', t[q - j]) for j in (4, 3, 2)]
            zc = re.fullmatch(r'lea ecx, \[esp \+ (0x[0-9a-f]+|\d+)\]', t[q - 1])
            if not all(zz) or not zc or ret_args(0x41cb90) != 3:
                raise Reject('reference made with')
            ref = (num(zz[2].group(1)), num(zz[1].group(1)), num(zz[0].group(1)), num(zc.group(1)) - depth_at[q - 1])
            extras[i] = ('ref', 0)
        elif v[0] == 'place' and front_name and v[1] == front_name[1]:
            extras[i] = ('startname', front_name[0])
    if any(v[0] not in ('const', 'arg', 'ref', 'startname') for v in extras) or ref_before != (ref is not None) or \
            start_text != any(v[0] == 'startname' for v in extras):
        raise Reject('base extra %s' % (extras,))
    if ref:
        # just after the base: the reference destroyed (lea ecx, [esp + its place]; call 0xBD1880), then the copy's release
        zd = [u for u in t[kb + 1:kb + 6] if not re.fullmatch(r'mov byte ptr \[esp \+ (0x[0-9a-f]+|\d+)\], (0x[0-9a-f]+|\d+|[a-d]l)', u)]
        zl2 = re.fullmatch(r'lea ecx, \[esp \+ (0x[0-9a-f]+|\d+)\]', zd[0])
        if not zl2 or zd[1] != 'call 0xbd1880' or num(zl2.group(1)) - depth_at[t.index(zd[0], kb)] != ref[3] or \
                not (zd[2].startswith('test byte ptr') or re.fullmatch(r'mov al, byte ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', zd[2])):
            raise Reject('reference not destroyed after the base')
    # the first: ecx chosen from the parent (an argument) or the title global
    q = pushes[0]
    if t[q] != 'push ' + sel_reg:
        raise Reject('base parent push ' + t[q])
    sel = [k for k in range(k41, kb) if re.fullmatch(r'mov ' + sel_reg + r', dword ptr \[(0x1[0-9a-f]{6})\]', t[k])]
    if len(sel) != 1 or ins[sel[0] - 1].mnemonic not in ('jne',):
        raise Reject('parent or title')
    if sel_reg != 'ecx' and any(re.match(r'(?!push|cmp|test)\w+ ' + sel_reg + r'\b', u) or
                                (sel_reg in ('eax', 'ebx', 'ecx', 'edx') and re.match(r'\w+ ' + sel_reg[1] + r'[lh]\b', u))
                                for u in t[sel[0] + 1:pushes[0]]):
        raise Reject('parent register written before the base')
    title = num(t[sel[0]][len('mov ' + sel_reg + ', dword ptr ['):-1])
    parent = value_at_reg(t, sel[0] - 1, sel_reg, depth_at, nargs, k41)   # what it holds when it isn't null
    if not parent or parent[0] != 'arg' or not 1 <= parent[1] <= min(nargs, 7):
        raise Reject('parent is %s' % (parent,))
    parent_arg = parent[1]   # (the parent's argument: the first, or another)
    if t[pushes[1]] != 'push eax':
        # the text kept in another register: eax moved into it right after the copy, and nothing writing it since
        rt = t[pushes[1]][5:]
        if rt not in ('ebx', 'esi', 'edi', 'ebp') or t[k41 + 1] != f'mov {rt}, eax' or \
                any(re.match(r'(?!push|cmp|test)\w+ ' + rt + r'\b', u) for u in t[k41 + 2:pushes[1]]):
            raise Reject('text push')
    # the text given: what's pushed for WideString::AssignWide (0x403BB0) is argument 2
    k3 = next(k for k, c in calls if c == '0x403bb0')
    q = k3 - 1
    while not t[q].startswith('push '):
        q -= 1
    q -= 1
    while not t[q].startswith('push '):
        q -= 1
    text_v = value_at_reg(t, q + 1, t[q][5:], depth_at, nargs, 0)
    if not text_v or text_v[0] != 'arg' or text_v[1] == parent_arg or not 1 <= text_v[1] <= min(nargs, 7):
        raise Reject('text is %s' % (text_v,))
    text_arg = text_v[1]
    base_this = value_at_reg(t, kb, 'ecx', depth_at, nargs, k41) if False else None
    p = dict(nargs=nargs, base=base, extras=extras, title=title, kb=kb, ins=ins, t=t, depth_at=depth_at, at=at, ref=ref, front_name=front_name,
             parent_arg=parent_arg, text_arg=text_arg)
    p['ops'] = tail(p)
    return p


FLAGS = ('add', 'sub', 'and', 'or', 'xor', 'inc', 'dec', 'cmp', 'test', 'call', 'neg', 'shl', 'shr', 'sar', 'imul')


def layout_step(t, k, R):
    """A child laid out, from t[k] (mov ecx, [this + C]): its size from its slot 0x20 (given a place on the stack), a
    rectangle on the stack {x, y: constants; size w and h each plus or minus a game float constant, worked out on the
    FPU and stored as floats}, and 0x6B1640 (SetRelationAndLayout) on the child with the relation, a member and the
    rectangle. Followed value by value; the stack must end where it began and no saved register may change. Returns
    (the index after it, the op) or None."""
    m = re.fullmatch(r'mov ecx, dword ptr \[' + REG + r' \+ (0x[0-9a-f]+|\d+)\]', t[k])
    if not m or R.get(m.group(1)) != ('this',):
        return None
    me, child = m.group(1), num(m.group(2))
    sim = {'ecx': ('child', child)}
    pushes, local, fpu, dd = [], {}, [], 0
    for q in range(k + 1, min(len(t), k + 48)):
        s = t[q]
        if re.fullmatch(r'mov byte ptr \[esp \+ (0x[0-9a-f]+|\d+)\], (0x[0-9a-f]+|\d+|[a-d]l)', s):
            continue   # the exception frame's state
        if (mm := re.fullmatch(r'mov ' + REG + r', dword ptr \[' + REG + r'\]', s)) and sim.get(mm.group(2)) == ('child', child):
            sim[mm.group(1)] = ('table', child)
        elif (mm := re.fullmatch(r'mov ' + REG + r', dword ptr \[eax(?: \+ 4)?\]', s)) and sim.get('eax') == ('size',) and mm.group(1) != 'eax':
            sim[mm.group(1)] = ('bits', 1 if '+ 4' in s else 0)   # the size's value copied as it is
        elif (mm := re.fullmatch(r'mov dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\], ' + REG, s)) and (sim.get(mm.group(2)) or ('',))[0] == 'bits':
            local[num(mm.group(1)) - dd] = sim[mm.group(2)]
        elif (mm := re.fullmatch(r'mov ' + REG + r', dword ptr \[' + me + r' \+ (0x[0-9a-f]+|\d+)\]', s)):
            v = num(mm.group(2))
            sim[mm.group(1)] = ('child', v) if v == child else ('member', v)
        elif (mm := re.fullmatch(r'lea ' + REG + r', \[esp \+ (0x[0-9a-f]+|\d+)\]', s)):
            sim[mm.group(1)] = ('local', num(mm.group(2)) - dd)
        elif (mm := re.fullmatch(r'push (0x[0-9a-f]+|\d+)', s)):
            pushes.append(('imm', num(mm.group(1)))); dd += 4
        elif (mm := re.fullmatch(r'push ' + REG, s)):
            reg = mm.group(1)
            if reg in sim:
                pushes.append(sim[reg])
            elif R.get(reg) == ('this',):
                pushes.append(('self',))
            else:   # a register the tail holds a constant in
                pushes.append(('imm', R[reg][1]) if R.get(reg) and R[reg][0] == 'const' else None)
            dd += 4
        elif (mm := re.fullmatch(r'call dword ptr \[' + REG + r' \+ 0x20\]', s)):
            if sim.get(mm.group(1)) != ('table', child) or sim.get('ecx') != ('child', child) or not pushes or not pushes[-1] or pushes[-1][0] != 'local':
                return None
            pushes.pop(); dd -= 4
            for r in ('eax', 'ecx', 'edx'):
                sim.pop(r, None)
            sim['eax'] = ('size',)
        elif (mm := re.fullmatch(r'fld dword ptr \[eax(?: \+ 4)?\]', s)) and sim.get('eax') == ('size',):
            fpu.append(('s', 1 if '+ 4' in s else 0))
        elif (mm := re.fullmatch(r'f(add|sub) dword ptr \[(0x[0-9a-f]+)\]', s)) and fpu and fpu[-1][0] == 's':
            fpu[-1] = ('op', fpu[-1][1], mm.group(1), num(mm.group(2)))
        elif (mm := re.fullmatch(r'fstp dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', s)) and fpu:
            local[num(mm.group(1)) - dd] = fpu.pop()
        elif (mm := re.fullmatch(r'mov dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\], (0x[0-9a-f]+|\d+)', s)):
            local[num(mm.group(1)) - dd] = ('imm', num(mm.group(2)))
        elif s == 'call 0x6b1640':
            if sim.get('ecx') != ('child', child) or len(pushes) != 3 or fpu:
                return None
            rect, parent, relation = pushes
            if not rect or rect[0] != 'local' or not parent or parent[0] not in ('member', 'child', 'self') or not relation or relation[0] != 'imm':
                return None
            r0, r1, r2, r3 = (local.get(rect[1] + 4 * i) for i in range(4))
            if not (r0 and r0[0] == 'imm' and r1 and r1[0] == 'imm'):
                return None
            flags, adds = relation[1] << 8 | (32 if parent[0] == 'self' else 0), [0, 0]
            for i, r_ in enumerate((r2, r3)):
                if r_ and r_[:2] == ('op', i):        # w / h plus or minus a game float
                    flags |= (r_[2] == 'sub') << i
                    adds[i] = r_[3]
                elif r_ == ('s', i):                  # only passed through the FPU
                    flags |= 4 << i
                elif r_ == ('bits', i):               # copied as it is
                    flags |= 16
                else:
                    return None
            if flags & 16 and (r2, r3) != (('bits', 0), ('bits', 1)):
                return None
            return q + 1, (40, child, 0 if parent[0] == 'self' else parent[1], 0x6b1640, [r0[1], r1[1], adds[0], adds[1], flags])
        else:
            return None
        if any(r in sim for r in ('ebx', 'esi', 'edi', 'ebp')):
            return None   # (a saved register written: not this shape)
    return None


def copy_step(t, k, R):
    """One member's dword at +X copied into another member's +Y (both members pointers the object keeps): only plain
    loads and the one store, from t[k]. Returns (the index after it, the op) or None."""
    sim = {}
    for q in range(k, min(len(t), k + 6)):
        s = t[q]
        if (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[' + REG + r' \+ (0x[0-9a-f]+|\d+)\]', s)):
            src = sim.get(m.group(2)) or (('this',) if R.get(m.group(2)) == ('this',) and m.group(2) not in sim else None)
            if src == ('this',):
                sim[m.group(1)] = ('member', num(m.group(3)))
            elif src and src[0] == 'member':
                sim[m.group(1)] = ('through', src[1], num(m.group(3)))
            else:
                return None
            if R.get(m.group(1)) == ('this',):
                return None   # (the object's own register overwritten)
        elif (m := re.fullmatch(r'mov dword ptr \[' + REG + r' \+ (0x[0-9a-f]+|\d+)\], ' + REG, s)):
            to, v = sim.get(m.group(1)), sim.get(m.group(3))
            if not to or to[0] != 'member' or not v or v[0] != 'through':
                return None
            return q + 1, (41, to[1], num(m.group(2)), 0, [v[1], v[2]])
        else:
            return None
    return None


def tail(p):
    """The member steps after the base and the copy's release, as ops (kind, off, value, fn, args); see titled.cpp."""
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
        return ('this',) if reg == 'ecx' else None   # nothing wrote it and nothing was called: ecx is still the object

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
    ops, pushed, nulls, serial, S = [], [], [], 0, {}
    cur = None          # the new object being made: its serial
    temps = {}          # stack slot -> temporary number
    built = set()       # temporaries and names made and not yet let go: ('temp', k) / ('name', k)
    nnames = 0
    if p.get('front_name'):
        S[p['front_name'][1]] = ('name', 0, None)   # (the text made first of all: name 0, made by the body)
        built.add(('name', 0))
        nnames = 1
    naligned = 0        # aligned blocks of memory made (the last one can be stored)
    flag = None         # what the flags were last set from
    k = start
    d = depth_at[start]

    def still(v):       # a member's value read earlier (('field', off, steps then)): good while no later step can have changed it
        return bool(v) and v[0] == 'field' and all(o[0] in (12, 18, 21, 31, 32, 33, 43, 44, 45, 57, 58, 63, 64, 65, 68, 72) or o[0] == 10 and o[3] == 0xbcf850 or o[0] in (0, 1, 2, 4, 5, 6, 7, 26, 27, 29, 30, 22) and
                                                  not (o[1] <= v[1] < o[1] + 4 or v[1] <= o[1] < v[1] + 4) for o in ops[v[2]:])

    def value_of(v):    # a value given to a call: (kind, value) as the template reads it
        if isinstance(v, int):
            return (0, v)
        return {'glob': 1, 'given': 2, 'self': 3, 'temp': 4, 'nameaddr': 5, 'field': 6, 'scratch': 7, 'tempat': 8, 'local': 9, 'fieldplace': 10, 'darg': 12}[v[0]], v[1]

    nb_used = set()
    nb_eax = ['keep']
    clobbered = set()   # argument slots the tail wrote (a later read of one is not the argument)

    nscratch = [0]
    run_locals = set()   # locals earlier x87 runs wrote (the rewrite's buffer keeps them), until the steps write the slot
    always_temps = set()
    pod_temps = set()      # temporaries with nothing to destroy (KeyCombination::Set's)   # temporaries made whatever happened (not only with a new object's memory)
    tempat_ops = {}   # a temporary's field read: the step it was read at (good while that temporary lives)

    QUIET = {0, 1, 2, 4, 5, 6, 7, 12, 22, 26, 27, 29, 30, 31, 32, 33, 43, 44, 45, 47, 48, 49, 51, 55, 72, 73, 74}   # steps that call nothing

    def take_result(v):   # a call's result still the last thing called: into a scratch value (kind 64); else None
        if not v or v[0] != 'result' or any(o[0] not in QUIET for o in ops[v[1] + 1:]):
            return None
        if nscratch[0] >= 8:
            raise Reject('scratch values')
        ops.append((64, nscratch[0], 0, 0, []))
        nscratch[0] += 1
        return ('scratch', nscratch[0] - 1)

    def clobbered_args():   # the argument numbers whose slots the tail wrote
        return {sl // 4 for sl in clobbered}

    def deref_code(v):   # a dword read inside what an argument points to, still the same (nothing called since)
        if not v or v[0] != 'deref' or any(o[0] not in QUIET for o in ops[v[4]:]):
            return None
        return ('darg', v[2] | v[3] << 8)

    def base_code(v):   # a value as a call is given it (the push forms), or None
        if not v:
            return None
        if v[0] == 'const': return v[1]
        if v[0] == 'global': return ('glob', v[1])
        if v[0] == 'arg': return ('given', v[1])
        if v == ('this',): return ('self', 0)
        if v[0] == 'member': return ('self', v[1])
        if still(v): return ('field', v[1])
        if v[0] == 'deref': return deref_code(v)
        if v[0] == 'tempat' and ('temp', v[1]) in built and v[2] < 0x40:   # (what a temporary holds)
            return ('tempat', v[1] | v[2] << 8)
        return None

    def value_block(k0):
        # at je L1 after test V, V / cmp V, Z (Z holding 0), one of:
        #   V ? V + c : 0       lea D, [V + c] / add V, c; jmp L2; L1: xor D, D; L2
        #   V's interface       mov X, [V]; mov ecx, V; call [X + 4]; test eax / cmp eax, Z; je L1; mov C, [G];
        #   (the class G)       cmp C, [eax + 0x18]; jge L1; mov D, [eax + 0x14]; mov eax, [D + C*4]; test / cmp; je L1;
        #                       mov eax, [eax + 0x10]; add eax, V; jmp L2; L1: xor eax, eax; L2
        # (the exception state's byte stores passed over). Gives (L2, the register, 0 or 1, the base, c or G).
        q = k0 - 1
        while q >= 0 and re.fullmatch(r'mov (byte|dword) ptr \[(?:esp|' + REG + r')(?: \+ (0x[0-9a-f]+|\d+))?\], (0x[0-9a-f]+|\d+|[a-d]l|' + REG + ')', t[q]):
            q -= 1
        z = re.fullmatch(r'(?:test ' + REG + ', ' + REG + '|cmp ' + REG + ', ' + REG + ')', t[q])
        if not z:
            return None
        v_ = z.group(1) or z.group(3)
        if z.group(1) and z.group(1) != z.group(2) or z.group(3) and R.get(z.group(4)) not in (('const', 0), ('byte', 0)):
            return None
        base = base_code(R.get(v_))
        if base is None:
            return None
        l1 = at.get(int(ins[k0].op_str, 0))
        zero = lambda u, r: u == f'test {r}, {r}' or (zz := re.fullmatch(r'cmp ' + r + ', ' + REG, u)) and R.get(zz.group(1)) in (('const', 0), ('byte', 0))
        L = []
        q = k0 + 1
        while len(L) < 20 and q < len(t):
            if not re.fullmatch(r'mov byte ptr \[esp \+ (0x[0-9a-f]+|\d+)\], (0x[0-9a-f]+|\d+)', t[q]):
                L.append((q, t[q]))
            q += 1
        b = [u for _, u in L]
        def ends(i, dest):   # jmp L2; L1: xor dest, dest; L2
            zj = re.fullmatch(r'jmp (0x[0-9a-f]+)', b[i]) if i < len(b) else None
            if not zj or l1 != L[i + 1][0] or b[i + 1] != f'xor {dest}, {dest}':
                return None
            l2 = at.get(num(zj.group(1)))
            return l2 if l2 == L[i + 1][0] + 1 else None
        if (z1 := re.fullmatch(r'lea ' + REG + r', \[' + v_ + r' \+ (0x[0-9a-f]+|\d+)\]', b[0])) and (l2 := ends(1, z1.group(1))):
            return l2, z1.group(1), 0, base, num(z1.group(2)), {z1.group(1)}
        if (z1 := re.fullmatch(r'add ' + v_ + r', (0x[0-9a-f]+|\d+)', b[0])) and (l2 := ends(1, v_)):
            return l2, v_, 0, base, num(z1.group(1)), {v_}
        z1 = re.fullmatch(r'mov ' + REG + r', dword ptr \[' + v_ + r'\]', b[0])
        if not z1 or b[1] != 'mov ecx, ' + v_ or b[2] != f'call dword ptr [{z1.group(1)} + 4]' or not zero(b[3], 'eax'):
            return None
        zj = [re.fullmatch(r'(je|jge) (0x[0-9a-f]+)', b[i]) for i in (4, 7, 11)]
        if not all(zj) or any(at.get(num(z_.group(2))) != l1 for z_ in zj) or [z_.group(1) for z_ in zj] != ['je', 'jge', 'je']:
            return None
        zc = re.fullmatch(r'mov ' + REG + r', dword ptr \[(0x1[0-9a-f]{6})\]', b[5])
        if not zc or b[6] != f'cmp {zc.group(1)}, dword ptr [eax + 0x18]':
            return None
        zd = re.fullmatch(r'mov ' + REG + r', dword ptr \[eax \+ 0x14\]', b[8])
        if not zd or b[9] != f'mov eax, dword ptr [{zd.group(1)} + {zc.group(1)}*4]' or not zero(b[10], 'eax'):
            return None
        if b[12] != 'mov eax, dword ptr [eax + 0x10]' or b[13] != 'add eax, ' + v_ or not (l2 := ends(14, 'eax')):
            return None
        if v_ in ('ecx', 'edx', 'eax') or zc.group(1) == v_ or zd.group(1) == v_:
            return None
        return l2, 'eax', 1, base, num(zc.group(2)), {'eax', 'ecx', 'edx', z1.group(1), zc.group(1), zd.group(1)}

    def last_text_object():   # the new object being made: 0x10 bytes, made with 0xBCF850 (a text), nothing since
        i18 = max((i for i, o in enumerate(ops) if o[0] == 18), default=None)
        if i18 is None or ops[i18][2] != 0x10:
            return False
        rest = ops[i18 + 1:]
        return len(rest) == 1 and rest[0][0] == 10 and rest[0][3] == 0xbcf850 and len(rest[0][4]) == 1 and isinstance(rest[0][4][0], int)

    def text_or_empty(k0):
        # je ELSE (on the value V, also in a register R); push 0x10; call 0xBCDFC0; add esp, 4; [mov [esp + A], eax];
        # test eax, eax; je NULL; push R; mov ecx, eax; call 0xBCF7D0; jmp END; NULL: xor eax, eax; jmp END;
        # ELSE: mov cl, 1; call 0xBD0490; END (the exception state's stores passed over). Gives (END, R) or None.
        else_ = at.get(int(ins[k0].op_str, 0))
        if else_ is None:
            return None
        L = []
        q = k0 + 1
        while q < len(t) and len(L) < 16:
            if not re.fullmatch(r'mov (byte|dword) ptr \[esp \+ (0x[0-9a-f]+|\d+)\], (0x[0-9a-f]+|\d+|[a-d]l|eax)', t[q]):
                L.append((q, t[q]))
            q += 1
        b = [u for _, u in L]
        want = ['push 0x10', 'call 0xbcdfc0', 'add esp, 4', 'test eax, eax']
        if b[:4] != want or not b[4].startswith('je ') or not re.fullmatch(r'push ' + REG, b[5]) or b[6:8] != ['mov ecx, eax', 'call 0xbcf7d0'] \
                or not b[8].startswith('jmp ') or b[9] != 'xor eax, eax' or not b[10].startswith('jmp ') or b[11:13] != ['mov cl, 1', 'call 0xbd0490']:
            return None
        holder = b[5][5:]
        if R.get(holder) != flag[1]:
            return None
        end = at.get(num(b[8][4:]))
        if end is None or end != at.get(num(b[10][4:])) or end != L[12][0] + 1 or at.get(num(b[4][3:])) != L[9][0] or else_ != L[11][0]:
            return None
        return end, holder

    def plain_release(k0, reg, at_):
        # mov A, [esp + N]; dec dword ptr [A]; mov A, [esp + N]; cmp dword ptr [A], 0 (or a register holding 0); jne END;
        # mov B, A; mov A, [A + 0xc]; push A; call 0xbce010; push B; call 0xbce000; add esp, 8; END (the exception state's
        # byte stores passed over). Gives (END, the registers written).
        L = []
        q = k0
        while len(L) < 14 and q < len(t):
            if not re.fullmatch(r'mov (byte|dword) ptr \[esp \+ (0x[0-9a-f]+|\d+)\], (0x[0-9a-f]+|\d+|[a-d]l)', t[q]):   # (the exception state)
                L.append((q, t[q]))
            q += 1
        b = [u for _, u in L]
        slot = at_ - depth_at[k0]
        z2 = re.fullmatch(r'mov ' + reg + r', dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', b[2])
        z3 = re.fullmatch(r'cmp dword ptr \[' + reg + r'\], (0|' + REG + ')', b[3])
        zj = re.fullmatch(r'jne (0x[0-9a-f]+)', b[4])
        if b[1] != f'dec dword ptr [{reg}]' or not z2 or num(z2.group(1)) - depth_at[L[2][0]] != slot or not z3 or not zj:
            return None
        if z3.group(1) != '0' and R.get(z3.group(1)) not in (('const', 0), ('byte', 0)):
            return None
        # the frees, in either of the compiler's orders: (mov B, A; mov A, [A + 0xc]; push A) or (mov C, [A + 0xc]; push C;
        # mov B, A), then call 0xbce010; push B; call 0xbce000; add esp, 8
        z5 = re.fullmatch(r'mov ' + REG + ', ' + reg, b[5])
        z6 = re.fullmatch(r'mov ' + REG + r', dword ptr \[' + reg + r' \+ 0xc\]', b[5])
        written = set()
        if z5 and b[6:9] == [f'mov {reg}, dword ptr [{reg} + 0xc]', f'push {reg}', 'call 0xbce010']:
            o2 = z5.group(1)
        elif z6 and b[6] == f'push {z6.group(1)}' and (z7 := re.fullmatch(r'mov ' + REG + ', ' + reg, b[7])) and b[8] == 'call 0xbce010':
            o2 = z7.group(1); written.add(z6.group(1))
        else:
            return None
        if b[9:12] != [f'push {o2}', 'call 0xbce000', 'add esp, 8']:
            return None
        end = at.get(num(zj.group(1)))
        if end is None or not (L[11][0] < end <= (L[12][0] if len(L) > 12 else end)):
            return None
        if o2 in ('esi', 'edi', 'ebx', 'ebp') and R.get(o2) and R[o2][0] in ('this', 'member'):
            return None
        if written & {'esi', 'edi', 'ebx', 'ebp'}:
            return None
        return end, {reg, o2, 'eax', 'ecx', 'edx'}

    def name_block(k0):
        # jne SKIP (the description given); mov X, [title]; then the flag set (mov bl, 1 / mov byte ptr [esp + N], 1), X
        # kept (mov dword ptr [esp + N], X / mov Y, X). SKIP: the check (mov al, byte ptr [U + 4]; test al, al / cmp byte
        # ptr [U + 4], bl with bl 0; U perhaps read back from where X was kept); jne END; then one or more names: the
        # member's place (push 0 / push R holding a constant / sub M, T; push M / lea A, [T + off]; sub A, T; push A /
        # mov A, M or the member kept on the stack; sub A, T; push A), push NAME, lea ecx, [esp + N], call 0xbeca40,
        # push eax and mov ecx, U (either order), call 0xbecc10, lea ecx, [esp + N], call 0xbec5e0. END: the flag
        # tested (test bl, bl / mov al, byte ptr [esp + N]; test al, al / cmp byte ptr [esp + N], bl), perhaps mov eax,
        # R (the value given back), je DONE; mov Z, [title]; mov byte ptr [Z + 4], 1. DONE. (The exception state's byte
        # stores are passed over.) Gives (DONE, [(member place, name global)...]).
        skip = at.get(int(ins[k0].op_str, 0))
        m1 = re.fullmatch(r'mov ' + REG + r', dword ptr \[(0x1[0-9a-f]{6})\]', t[k0 + 1])
        if skip is None or not m1 or num(m1.group(2)) != p['title']:
            return None
        used = {m1.group(1)} | {r for r in R if R[r] == ('arg', p.get('parent_arg', 1))}
        used_slots = set()
        dd = depth_at[k0]
        zero = lambda r: R.get(r) in (('const', 0), ('byte', 0))
        flag_at = None
        for q in range(k0 + 2, skip):
            u = t[q]
            if u == 'mov bl, 1':
                if not zero('ebx'):
                    return None
                flag_at = 'bl'
            elif (z := re.fullmatch(r'mov byte ptr \[esp \+ (0x[0-9a-f]+|\d+)\], 1', u)):
                slot = num(z.group(1)) - dd
                before = None   # the byte was cleared before (0, or a register holding 0)
                for q2 in range(k0 - 1, start - 1, -1):
                    z2 = re.fullmatch(r'mov byte ptr \[esp \+ (0x[0-9a-f]+|\d+)\], (0|[abcd]l)', t[q2])
                    if z2 and num(z2.group(1)) - depth_at[q2] == slot:
                        before = (z2.group(2), q2)
                        break
                if before is None:
                    return None
                if before[0] != '0' and known('e' + before[0][0] + 'x', before[1]) not in (('const', 0), ('byte', 0)):
                    return None
                flag_at = ('stack', slot)
                if slot >= 4: clobbered.add(slot)
            elif (z := re.fullmatch(r'mov dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\], ' + REG, u)) and z.group(2) in used:
                slot = num(z.group(1)) - dd
                used_slots.add(slot)
                if slot >= 4: clobbered.add(slot)
            elif (z := re.fullmatch(r'mov ' + REG + ', ' + REG, u)) and z.group(2) in used:
                used.add(z.group(1))
            else:
                return None
        if flag_at is None:
            return None
        if flag_at == 'bl' and 'ebx' in used:
            return None
        L = []   # (index, text) without the exception state's byte stores
        q = skip
        while len(L) < 120 and q < len(t):
            if not re.fullmatch(r'mov byte ptr \[esp \+ (0x[0-9a-f]+|\d+)\], (0x[0-9a-f]+|\d+)', t[q]):
                L.append((q, t[q]))
            q += 1
        c = [0]
        def peek(o=0):
            return L[c[0] + o][1] if c[0] + o < len(L) else ''
        def take(pt):
            z = re.fullmatch(pt, peek())
            if z: c[0] += 1
            return z
        local = {}   # registers loaded inside the block
        def holder(r):   # does r hold the description being named?
            return r in used or local.get(r) == 'used'
        def load_used():   # mov R, dword ptr [esp + N] from where X was kept
            z = re.fullmatch(r'mov ' + REG + r', dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', peek())
            if z and num(z.group(2)) - depth_at[L[c[0]][0]] in used_slots:
                local[z.group(1)] = 'used'; c[0] += 1
                return True
            return False
        load_used()
        if (z := take(r'mov ([ac])l, byte ptr \[' + REG + r' \+ 4\]')):
            if not holder(z.group(2)) or not take(f'test {z.group(1)}l, {z.group(1)}l'):
                return None
        elif (z := take(r'cmp byte ptr \[' + REG + r' \+ 4\], bl')):
            if not holder(z.group(1)) or not zero('ebx') or flag_at == 'bl':
                return None
        else:
            return None
        zj = take(r'jne (0x[0-9a-f]+)')
        if not zj:
            return None
        end = at.get(num(zj.group(1)))
        names = []
        while True:
            if end is not None and L[c[0]][0] >= end:
                break
            # the member's place
            if (z := take(r'push (0x[0-9a-f]+|\d+)')):
                off_ = num(z.group(1))
            elif (z := take(r'push ' + REG)) and R.get(z.group(1)) and R[z.group(1)][0] in ('const', 'byte') and z.group(1) not in local:
                off_ = R[z.group(1)][1]
            else:
                a_ = None
                if (z := take(r'lea ' + REG + r', \[' + REG + r' \+ (0x[0-9a-f]+|\d+)\]')) and R.get(z.group(2)) == ('this',):
                    a_, v_ = z.group(1), ('member', num(z.group(3)))
                elif (z := take(r'mov ' + REG + r', dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]')):
                    slot = num(z.group(2)) - depth_at[L[c[0] - 1][0]]
                    a_, v_ = z.group(1), S.get(slot, stored(slot, L[c[0] - 1][0]))
                elif (z := take(r'mov ' + REG + ', ' + REG)) and z.group(2) not in local:
                    a_, v_ = z.group(1), R.get(z.group(2))
                z = take(r'sub ' + REG + ', ' + REG)
                if not z or R.get(z.group(2)) != ('this',) or z.group(2) in local:
                    return None
                if a_ is None:
                    if z.group(1) in local:
                        return None
                    a_, v_ = z.group(1), R.get(z.group(1))
                if z.group(1) != a_ or not v_ or v_[0] != 'member' or not take('push ' + a_):
                    return None
                off_ = v_[1]
                local[a_] = 'place'
            zn = take(r'push (0x[0-9a-f]+)')
            if not zn or not take(r'lea ecx, \[esp \+ (0x[0-9a-f]+|\d+)\]') or not take('call 0xbeca40'):
                return None
            if take('push eax'):
                load_used()
                z = take(r'mov ecx, ' + REG)
                if not z or not holder(z.group(1)):
                    return None
            else:
                if not load_used() or not re.fullmatch(r'mov ecx, ', peek()[:9]) and False:
                    return None
                # (mov ecx, dword ptr [esp + N] read the description back: load_used took it into ecx)
                if local.get('ecx') != 'used' or not take('push eax'):
                    return None
            if not take('call 0xbecc10') or not take(r'lea ecx, \[esp \+ (0x[0-9a-f]+|\d+)\]') or not take('call 0xbec5e0'):
                return None
            local.pop('ecx', None); local.pop('eax', None); local.pop('edx', None)
            names.append((off_, num(zn.group(1))))
            if len(names) > 8:
                return None
        if not names or end is None or L[c[0]][0] != max(end, L[c[0]][0]) or any(
                not re.fullmatch(r'mov byte ptr \[esp \+ (0x[0-9a-f]+|\d+)\], (0x[0-9a-f]+|\d+)', t[q]) for q in range(end, L[c[0]][0])):
            return None
        if L[c[0] - 1][0] >= end:
            return None
        # the mark
        if flag_at == 'bl':
            if not take('test bl, bl'):
                return None
        else:
            if (z := take(r'mov al, byte ptr \[esp \+ (0x[0-9a-f]+|\d+)\]')):
                if num(z.group(1)) - depth_at[L[c[0] - 1][0]] != flag_at[1] or not take('test al, al'):
                    return None
            elif (z := take(r'cmp byte ptr \[esp \+ (0x[0-9a-f]+|\d+)\], bl')):
                if num(z.group(1)) - depth_at[L[c[0] - 1][0]] != flag_at[1] or not zero('ebx') or 'ebx' in used:
                    return None
            else:
                return None
        eax_ = 'keep'
        if (z := take(r'mov eax, ' + REG)):
            if z.group(1) in used or z.group(1) in local or z.group(1) in ('ecx', 'edx', 'eax', 'ebx'):
                return None
            eax_ = R.get(z.group(1))
        zj = take(r'je (0x[0-9a-f]+)')
        zm = take(r'mov ' + REG + r', dword ptr \[(0x1[0-9a-f]{6})\]')
        if not zj or not zm or num(zm.group(2)) != p['title'] or not take(f'mov byte ptr \\[{zm.group(1)} \\+ 4\\], 1'):
            return None
        done = at.get(num(zj.group(1)))
        last = L[c[0] - 1][0]
        if done is None or done <= last or any(not re.fullmatch(r'mov byte ptr \[esp \+ (0x[0-9a-f]+|\d+)\], (0x[0-9a-f]+|\d+)', t[q]) for q in range(last + 1, done)):
            return None
        nb_eax[0] = eax_
        nb_used.clear(); nb_used.update(used | set(local) | {zm.group(1)} | ({'ebx'} if flag_at == 'bl' else set()))
        return done, names

    edx_live = [False]          # edx written since the last call and not used since (a call may read it)
    edx_call = [None]           # a call made with edx live: the step count before it (its step must take edx = this)

    def fpu_step(k0, size_query=False):
        # a run of x87 work building values on the stack, then a direct call given them: from k0 (an x87 instruction) to
        # the call. Its steps: fld / fild / fst / fstp / fadd .. fdivr with a dword operand (a game global, the window's,
        # what a member points to, a stack local of the run or the constructor's argument), fld st(i), fxch st(i),
        # f..p st(1), fadd st(0), st(i) and the like, fchs, fld1, fldz, fstp st(0); integer copies into locals (mov R,
        # [operand]; mov [esp + N], R) and constants. Pushes before and in it: constants, the window, a member, what a
        # member points to, a local's place. The callee's ecx: the window, a member or what a member points to. The
        # FPU must end empty; nothing else may be written. Gives (index after the call, the op) or None.
        nonlocal d
        prog, depth, fpu = [], d, 0
        size_seen = []
        regs = {}       # registers set in the run: ('bits', operand) / ('local', slot) / ('ecx', value)
        written = set() # locals the run writes (entry-relative slots)
        known_locals = {}
        def operand(text, q, store=False):
            mm = re.fullmatch(r'dword ptr \[(0x[0-9a-f]+)\]', text)
            if mm:
                return (1, num(mm.group(1)))
            mm = re.fullmatch(r'dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', text)
            if mm:
                slot = num(mm.group(1)) - depth
                if slot >= 4 and slot % 4 == 0 and slot // 4 <= nargs:
                    if slot in clobbered:
                        return None
                    return (5, slot // 4)
                if slot < 0 and -slot <= 0x100 and slot % 4 == 0:
                    if slot not in written and not store and slot not in run_locals:
                        v = S.get(slot)
                        if not v or v[0] != 'const':
                            return None
                        known_locals[slot] = v[1]
                    return (4, (slot + 0x100) // 4)
                return None
            mm = re.fullmatch(r'dword ptr \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\]', text)
            if mm and regs.get(mm.group(1)) == ('size',) and not store:
                return (6, num(mm.group(2) or '0'))   # what the size query gave
            if mm:
                off = num(mm.group(2) or '0')
                v = regs.get(mm.group(1), ('r', R.get(mm.group(1))))
                if v[0] == 'bits' and v[1][0] == 2 and v[1][1] < 0x10000 and off < 0x10000:
                    return (3, v[1][1] | off << 16)   # (a member's value loaded in the run: what it points to, read through)
                v = v[1] if v[0] in ('r', 'ecx') else None
                if v == ('this',): return (2, off)
                if v and v[0] == 'member': return (2, v[1] + off)
                if v and still(v) and v[1] < 0x10000 and off < 0x10000: return (3, v[1] | off << 16)
            return None
        args_pushed = [('local', v[1]) if isinstance(v, tuple) and v[0] == 'rect' else v for v in pushed]
        q = k0
        while q < len(t):
            u, xq = t[q], ins[q]
            if (zq := re.fullmatch(r'call dword ptr \[' + REG + r' \+ 0x20\]', u)) and (regs.get(zq.group(1), ('r', R.get(zq.group(1))))[1] or ('',))[0] == 'vtable' \
                    and args_pushed and isinstance(args_pushed[-1], tuple) and args_pushed[-1][0] == 'local' and not size_seen:
                # a child's size asked for (its slot 0x20, a local to fill): a step, what it gives read through eax after
                obj = regs.get(zq.group(1), ('r', R.get(zq.group(1))))[1][1]
                ecx_now = regs.get('ecx', ('r', R.get('ecx')))[1]
                if ecx_now != obj:
                    return None
                if obj == ('this',): okind, oval = 0, 0
                elif obj[0] == 'member': okind, oval = 0, obj[1]
                elif still(obj): okind, oval = 1, obj[1]
                else:
                    return None
                sl = args_pushed.pop()[1]
                depth -= 4
                if not (sl < 0 and -sl <= 0x100):
                    return None
                written.update(sl + 4 * i for i in range(4))   # (the size filled in: two floats, and room)
                prog.append((25, okind, oval, (sl + 0x100) // 4))
                regs['eax'] = ('size',)
                for r_ in ('ecx', 'edx'):
                    regs[r_] = ('r', None)
                size_seen.append(1)
                q += 1
                continue
            if xq.mnemonic == 'call':
                break
            if re.fullmatch(r'mov byte ptr \[esp \+ (0x[0-9a-f]+|\d+)\], (0x[0-9a-f]+|\d+|[a-d]l)', u):
                q += 1; continue   # (the exception frame's state)
            mm = re.fullmatch(r'(f\w+) (dword ptr \[.+\])', u)
            if mm and mm.group(1) in FPU_CODES:
                code = FPU_CODES[mm.group(1)]
                o_ = operand(mm.group(2), q, store=code in (3, 4))
                if o_ is None:
                    return None
                if code in (3, 4):   # a store: only into a local
                    if o_[0] != 4:
                        return None
                    written.add(o_[1] * 4 - 0x100)
                prog.append((code, o_[0], o_[1]))
                fpu += {1: 1, 21: 1, 4: -1}.get(code, 0)
            elif (mm := re.fullmatch(r'fld st\((\d)\)', u)):
                prog.append((2, 0, int(mm.group(1)))); fpu += 1
            elif (mm := re.fullmatch(r'fxch st\((\d)\)', u)):
                prog.append((17, 0, int(mm.group(1))))
            elif (mm := re.fullmatch(r'f(add|sub|subr|mul|div|divr)p st\(1\)', u)):
                prog.append((FPU_CODES['f' + mm.group(1) + 'p'], 0, 1)); fpu -= 1
            elif (mm := re.fullmatch(r'f(add|sub|subr|mul|div|divr) st\(0\), st\((\d)\)|f(add|sub|subr|mul|div|divr) st\((\d)\)', u)):
                op_, i_ = (mm.group(1), mm.group(2)) if mm.group(1) else (mm.group(3), mm.group(4))
                prog.append((30 + ['add', 'sub', 'subr', 'mul', 'div', 'divr'].index(op_), 0, int(i_)))   # st(0) = st(0) op st(i)
            elif (mm := re.fullmatch(r'f(add|sub|subr|mul|div|divr) st\((\d)\), st\(0\)', u)):
                prog.append((40 + ['add', 'sub', 'subr', 'mul', 'div', 'divr'].index(mm.group(1)), 0, int(mm.group(2))))   # st(i) = st(i) op st(0)
            elif u == 'fchs ':
                prog.append((18, 0, 0))
            elif u == 'fld1 ':
                prog.append((23, 0, 0)); fpu += 1
            elif u == 'fldz ':
                prog.append((24, 0, 0)); fpu += 1
            elif u == 'fstp st(0)':
                prog.append((22, 0, 0)); fpu -= 1
            elif (mm := re.fullmatch(r'mov ' + REG + r', (dword ptr \[.+\])', u)) and operand(mm.group(2), q):
                if mm.group(1) == 'ecx' and (z := re.fullmatch(r'dword ptr \[' + REG + r' \+ (0x[0-9a-f]+|\d+)\]', mm.group(2))) and \
                        (regs.get(z.group(1), ('r', R.get(z.group(1))))[1] == ('this',)):
                    regs['ecx'] = ('ecx', ('field', num(z.group(2)), len(ops)))   # a member's object: the callee, or read from
                else:
                    regs[mm.group(1)] = ('bits', operand(mm.group(2), q))
            elif (mm := re.fullmatch(r'mov ' + REG + r', dword ptr \[ecx\]', u)) and mm.group(1) != 'ecx':
                ev = regs.get('ecx', ('r', R.get('ecx')))[1]
                if not ev:
                    return None
                regs[mm.group(1)] = ('r', ('vtable', ev))
            elif (mm := re.fullmatch(r'mov ecx, ' + REG, u)):
                v = regs.get(mm.group(1), ('r', R.get(mm.group(1))))
                if v[0] not in ('r', 'ecx') or not v[1]:
                    return None
                regs['ecx'] = ('ecx', v[1])
            elif (mm := re.fullmatch(r'mov dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\], (0x[0-9a-f]+|\d+)', u)):
                slot = num(mm.group(1)) - depth
                if not (slot < 0 and -slot <= 0x100 and slot % 4 == 0):
                    return None
                prog.append((20, 0, num(mm.group(2))) + ((slot + 0x100) // 4,)); written.add(slot)
            elif (mm := re.fullmatch(r'mov dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\], ' + REG, u)):
                slot = num(mm.group(1)) - depth
                v = regs.get(mm.group(2), ('r', R.get(mm.group(2))))
                if not (slot < 0 and -slot <= 0x100 and slot % 4 == 0):
                    return None
                if v[0] == 'bits':
                    prog.append((19, v[1][0], v[1][1]) + ((slot + 0x100) // 4,))
                elif v[0] == 'r' and v[1] and v[1][0] == 'const':
                    prog.append((20, 0, v[1][1]) + ((slot + 0x100) // 4,))
                else:
                    return None
                written.add(slot)
            elif (mm := re.fullmatch(r'lea ' + REG + r', \[esp \+ (0x[0-9a-f]+|\d+)\]', u)):
                regs[mm.group(1)] = ('local', num(mm.group(2)) - depth)
            elif (mm := re.fullmatch(r'push (0x[0-9a-f]+|\d+)', u)):
                args_pushed.append(num(mm.group(1))); depth += 4
            elif (mm := re.fullmatch(r'push ' + REG, u)):
                v = regs.get(mm.group(1), ('r', R.get(mm.group(1))))
                if v[0] == 'local':
                    args_pushed.append(('local', v[1]))
                elif v[0] == 'r' and v[1] and v[1][0] == 'stack':
                    args_pushed.append(('local', v[1][1]))
                elif v[0] == 'r' and v[1] and v[1][0] == 'const':
                    args_pushed.append(v[1][1])
                elif v[0] == 'r' and v[1] == ('this',):
                    args_pushed.append(('self', 0))
                elif v[0] == 'r' and v[1] and v[1][0] == 'member':
                    args_pushed.append(('self', v[1][1]))
                elif v[0] in ('r', 'ecx') and v[1] and still(v[1]):
                    args_pushed.append(('field', v[1][1]))
                else:
                    return None
                depth += 4
            else:
                return None
            if fpu < 0 or fpu > 7:
                return None
            q += 1
        else:
            return None
        if fpu != 0 or not prog:
            return None
        zc = re.fullmatch(r'call (0x[0-9a-f]+)', t[q])
        if not zc:
            return None
        fn = num(zc.group(1)); n_ = ret_args(fn)
        if n_ is None or n_ != len(args_pushed) or n_ > 5:
            return None
        ecx = regs.get('ecx', ('r', R.get('ecx')))[1]
        if ecx == ('this',): target = 0
        elif ecx and ecx[0] == 'member': target = ecx[1]
        elif ecx and still(ecx) and ecx[1] < 0x1000000: target = 1 << 24 | ecx[1]
        else:
            return None
        # each local given: four dwords, each written by the run or a known constant before it
        args_ = []
        for v in reversed(args_pushed):
            if isinstance(v, tuple) and v[0] == 'local':
                for i in range(4):
                    sl = v[1] + 4 * i
                    if sl not in written:
                        sv = S.get(sl)
                        if not sv or sv[0] != 'const' or not (sl < 0 and -sl <= 0x100):
                            return None
                        known_locals[sl] = sv[1]
                if not (v[1] < 0 and -v[1] <= 0x100 - 12):
                    return None
                args_.append(('local', (v[1] + 0x100) // 4))
            else:
                args_.append(v)
        for sl, val_ in sorted(known_locals.items()):
            if sl not in written:
                prog.insert(0, (20, 0, val_, (sl + 0x100) // 4))
        run_locals.update(written)   # (what this run wrote stays in the locals for later runs, until written otherwise)
        start = 4 * len(FPU_PROGRAM)   # (where its count is in the flat table: four dwords a step)
        FPU_PROGRAM.append((len(prog), 0, 0, 0))
        FPU_PROGRAM.extend(tuple(x) + (0,) * (4 - len(x)) for x in prog)
        pushed.clear()
        d = depth - 4 * n_
        return q + 1, (59, target, start, fn, args_)

    def edx_check():
        if edx_call[0] is None:
            return
        before = edx_call[0]
        edx_call[0] = None
        if len(ops) != before + 1 or ops[-1][0] not in (3, 24) or ops[-1][2] >> 31:
            raise Reject('edx given')
        o = ops[-1]
        ops[-1] = (o[0], o[1], o[2] | 1 << 31, o[3], o[4])   # (bit 31: made with edx = this)

    conds = []   # forward branches open: (end index, the op's index, registers, stack slots, depth, made things)

    while True:
        s, x = t[k], ins[k]
        val = lambda r: R.get(r)
        edx_check()
        while conds and k == conds[-1][0]:
            # a block skipped when its value is 0 (or not) ends here: the stack as at its start, nothing made in it left
            # unlet, nothing pushed; what it changed is forgotten (it may not have happened)
            end_, at_op, R0, S0, d0, built0 = conds.pop()
            if pushed or d != d0 or built != built0 or nulls:
                raise Reject('conditional block leaves')
            o = ops[at_op]
            ops[at_op] = (o[0], len(ops) - at_op - 1, o[2], o[3], o[4])
            for r in list(R):
                if R.get(r) != R0.get(r):
                    R[r] = None
            for sl in set(S) | set(S0):
                if S.get(sl) != S0.get(sl):
                    S.pop(sl, None)
            flag = None
        if conds and x.mnemonic in ('ret', 'jmp') and not (x.mnemonic == 'jmp' and at.get(int(x.op_str, 0) if re.fullmatch(r'0x[0-9a-f]+', x.op_str) else -1, 1 << 30) <= conds[-1][0]):
            if x.mnemonic == 'ret' or at.get(int(x.op_str, 0), 1 << 30) > conds[-1][0]:
                raise Reject('a jump out of a conditional block')
        if x.mnemonic == 'call':
            if 'edx' in x.op_str:
                edx_live[0] = False   # (a call through edx: it was the table, used)
            if edx_live[0]:
                if R.get('edx') != ('this',) or x.op_str.startswith('dword'):
                    raise Reject('edx given')
                edx_call[0] = len(ops)
            edx_live[0] = False
        elif re.match(r'(mov|lea|xor|movzx|movsx|or|and|add|sub) edx, ', s) or re.match(r'mov d[lh], ', s):
            edx_live[0] = True
        elif re.search(r'\b(edx|dl|dh)\b', s):
            edx_live[0] = False
        if x.mnemonic.startswith('f') and x.mnemonic not in ('fnstsw',) and (fs := fpu_step(k)):
            edx_live[0] = False
            ops.append(fs[1])   # an x87 run and the call given what it built (kind 59)
            for r in ('eax', 'ecx', 'edx'):
                R[r] = None
            for r in ('ebx', 'esi', 'edi', 'ebp'):
                pass
            k = fs[0]
            flag = None
            continue
        if not pushed and ((ls := layout_step(t, k, R)) or (ls := copy_step(t, k, R))):
            edx_live[0] = False   # (its own steps use edx as they set it; its call, 0x6B1640, reads no edx)
            ops.append(ls[1])   # a child laid out (kind 40) / a member's value copied into another's (41)
            for r in ('eax', 'ecx', 'edx'):
                R.pop(r, None)
            k = ls[0]
            continue
        if x.mnemonic == 'nop' or re.fullmatch(r'lea (e[a-ds][xip]), \[\1\]', s):
            k += 1   # (filler the compiler put in to align a jump target)
            continue
        if x.mnemonic == 'call' and None in pushed:
            raise Reject('by-value unset')
        if x.mnemonic == 'ret':
            if conds:
                raise Reject('a conditional block open at the end')
            if edx_call[0] is not None:
                raise Reject('edx given')
            if R.get('eax') != ('this',):
                raise Reject('returns')
            break
        if (m := re.fullmatch(r'pop ' + REG, s)) and not pushed:
            R[m.group(1)] = ('restored',); d -= 4; k += 1
            continue
        if s == 'mov dword ptr fs:[0], ecx' or (s.startswith('add esp, ') and not pushed and ins[k + 1].mnemonic == 'ret'):
            k += 1
            continue
        # a member's value read earlier is only good until something else happens
        if (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\]', s)) and val(m.group(2)) and val(m.group(2))[0] in ('this', 'member') \
                and m.group(1) != m.group(2):
            R[m.group(1)] = ('field', (0 if val(m.group(2)) == ('this',) else val(m.group(2))[1]) + num(m.group(3) or '0'), len(ops))
            k += 1
            continue
        if (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[' + REG + r'\]', s)) and val(m.group(2)) and val(m.group(2))[0] == 'arg':
            R[m.group(1)] = ('argderef', val(m.group(2))[1], len(ops))   # what an argument points to
            k += 1
            continue
        if (m := re.fullmatch(r'mov (byte|dword) ptr \[(0x1[0-9a-f]{6})\], (0x[0-9a-f]+|\d+)', s)):
            ops.append((51, 1 if m.group(1) == 'byte' else 4, num(m.group(3)), num(m.group(2)), []))   # a game global set
            k += 1
            continue
        if (m := re.fullmatch(r'mov (byte|dword) ptr \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\], (0x[0-9a-f]+|\d+)', s)) and val(m.group(2)) and \
                val(m.group(2))[0] == 'global' and val(m.group(2))[1] >= 0x1000000 and num(m.group(3) or '0') < 0x1000 and \
                (not nulls or len(nulls) == 1 and nulls[0][1][0] == 'alloc' and nulls[0][1][1] == cur):
            # a constant stored into the object a game global points to (kind 73; inside the new object's branch: only when
            # there was memory)
            ops.append((73, num(m.group(3) or '0'), num(m.group(4)), val(m.group(2))[1], [(1 if m.group(1) == 'byte' else 4) | (0x100 if nulls else 0)]))
            k += 1
            continue
        if (m := re.fullmatch(r'inc dword ptr \[' + REG + r'\]', s)) and val(m.group(1)) and val(m.group(1))[0] == 'argderef' and \
                all(o[0] in (29, 0, 1, 2, 4, 5, 6) for o in ops[val(m.group(1))[2]:]):
            ops.append((50, 0, val(m.group(1))[1], 0, []))   # a hold taken on the text an argument holds
            flag = None
            k += 1
            continue
        if (m := re.fullmatch(r'(and|or) ' + REG + r', (0x[0-9a-f]+|\d+)', s)) and val(m.group(2)) and val(m.group(2))[0] == 'field':
            R[m.group(2)] = ('field' + m.group(1), val(m.group(2))[1], num(m.group(3)), val(m.group(2))[2])
            flag = None; k += 1
            continue
        if (m := re.fullmatch(r'mov (dword|byte) ptr \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\], (.+)', s)) and val(m.group(2)) and \
                val(m.group(2))[0] == 'obj' and val(m.group(2))[1] == cur and val(m.group(2))[2] == 'memory' and nulls:
            # a store into the new object (its own class's constructor, inlined), only when there was memory
            off, v = num(m.group(3) or '0'), m.group(4)
            w = val(v) if v in R else None
            if re.fullmatch(r'0x[0-9a-f]+|\d+', v):
                ops.append((31 if m.group(1) == 'dword' else 33, off, num(v), 0, []))
            elif m.group(1) == 'dword' and w and w[0] == 'const':
                ops.append((31, off, w[1], 0, []))
            elif m.group(1) == 'dword' and w and w[0] == 'global':
                ops.append((32, off, w[1], 0, []))
            elif m.group(1) == 'byte' and v in ('al', 'bl', 'cl', 'dl') and val('e' + v[0] + 'x') and val('e' + v[0] + 'x')[0] in ('const', 'byte'):
                ops.append((33, off, val('e' + v[0] + 'x')[1] & 0xff, 0, []))
            elif m.group(1) == 'dword' and w == ('this',):
                ops.append((72, off, 0, 0, [('self', 0)]))   # (the window's place stored into the new object)
            elif m.group(1) == 'dword' and w and w[0] == 'arg' and w[1] not in clobbered_args():
                ops.append((72, off, 0, 0, [('given', w[1])]))   # (an argument)
            elif m.group(1) == 'dword' and still(w):
                ops.append((72, off, 0, 0, [('field', w[1])]))   # (a member's value, still as read)
            else:
                raise Reject('new object store ' + s)
        elif (m := re.fullmatch(r'mov dword ptr \[' + REG + r'\], (.+)', s)) and val(m.group(1)) and val(m.group(1))[0] == 'spslot' \
                and pushed[val(m.group(1))[1]] is None:
            # the value of a 4-byte struct given by value (made in place on the stack): to the callee the same as a pushed value
            i, v = val(m.group(1))[1], m.group(2)
            w = val(v) if v in R else None
            if re.fullmatch(r'0x[0-9a-f]+|\d+', v):
                pushed[i] = num(v)
            elif w and w[0] in ('const', 'global', 'arg', 'this', 'member'):
                pushed[i] = w[1] if w[0] == 'const' else ('self', 0) if w[0] == 'this' else ('self', w[1]) if w[0] == 'member' else \
                    ('glob' if w[0] == 'global' else 'given', w[1])
            else:
                raise Reject('by-value store ' + s)
        elif (m := re.fullmatch(r'mov dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\], esp', s)) and (R.get('eax') or ('',))[0] == 'spslot':
            S.pop(num(m.group(1)) - d, None)   # (the exception frame keeps where that struct is)
        elif (m := re.fullmatch(r'(or|and) dword ptr \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\], (0x[0-9a-f]+|\d+)', s)) and val(m.group(2)) and                 val(m.group(2))[0] in ('this', 'member'):
            # a dword of the window's own or-ed or and-ed with a constant (kind 27 / 26)
            off_ = (0 if val(m.group(2)) == ('this',) else val(m.group(2))[1]) + num(m.group(3) or '0')
            ops.append((27 if m.group(1) == 'or' else 26, off_, num(m.group(4)), 0, []))
            flag = None
        elif (m := re.fullmatch(r'(or|and) dword ptr \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\], (0x[0-9a-f]+|\d+)', s)) and still(val(m.group(2))):
            ops.append((47 if m.group(1) == 'or' else 48, val(m.group(2))[1], num(m.group(4)), 0, [num(m.group(3) or '0')]))
            flag = None
        elif (m := re.fullmatch(r'mov dword ptr \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\], (.+)', s)) and val(m.group(1)) and \
                still(val(m.group(1))):
            # a store into what a member points to (a child's value set)
            member, inner, v = val(m.group(1))[1], num(m.group(2) or '0'), m.group(3)
            w = val(v) if v in R else None
            if re.fullmatch(r'0x[0-9a-f]+|\d+', v):
                ops.append((44, member, num(v), 0, [inner]))
            elif w and w[0] == 'const':
                ops.append((44, member, w[1], 0, [inner]))
            elif w and w[0] == 'global':
                ops.append((45, member, w[1], 0, [inner]))
            elif w == ('this',):
                ops.append((74, member, inner, 0, [('self', 0)]))   # (the window's place stored into the child: kind 74)
            elif w and w[0] == 'member':
                ops.append((74, member, inner, 0, [('self', w[1])]))   # (the place of one of the window's members)
            elif w and w[0] == 'arg' and w[1] not in clobbered_args():
                ops.append((74, member, inner, 0, [('given', w[1])]))   # (an argument)
            elif w and still(w):
                ops.append((74, member, inner, 0, [('field', w[1])]))   # (a member's value, still as read)
            elif w and w[0] == 'fieldplus' and still(('field',) + w[1:3]) and w[3] < 0x10000:
                ops.append((74, member, inner, 0, [('fieldplace', w[1] | w[3] << 16)]))   # (a place inside what a member points to)
            else:
                raise Reject('child store ' + s)
            for r_ in R:   # (the member itself is unchanged)
                if R[r_] and R[r_][0] == 'field' and R[r_][2] == len(ops) - 1:
                    R[r_] = ('field', R[r_][1], len(ops))
        elif (m := re.fullmatch(r'mov dword ptr \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\], ' + REG, s)) and val(m.group(1)) and \
                val(m.group(1))[0] in ('this', 'member') and (sc := take_result(val(m.group(3)))):
            # a call's result kept in the window (kind 64, then 55)
            ops.append((55, (0 if val(m.group(1)) == ('this',) else val(m.group(1))[1]) + num(m.group(2) or '0'), sc[1], 0, []))
            for r in list(R):
                if R[r] and R[r][0] == 'result':
                    R[r] = sc
        elif (m := re.fullmatch(r'mov (dword|byte|word) ptr \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\], (.+)', s)) and val(m.group(2)) and \
                val(m.group(2))[0] in ('this', 'member'):
            base_off = 0 if val(m.group(2)) == ('this',) else val(m.group(2))[1]
            off, v, size = base_off + num(m.group(3) or '0'), m.group(4), m.group(1)
            w = val(v) if v in R else None
            if re.fullmatch(r'0x[0-9a-f]+|\d+', v):
                ops.append(({'dword': 0, 'byte': 1, 'word': 4}[size], off, num(v), 0, []))
            elif size == 'dword' and w:
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
                elif w[0] == 'obj' and w[1] == cur:
                    ops.append((22, off, 0 if w[2] == 'returned' else 1, 0, []))
                    for r_ in list(R):   # (what holds it is now that member's value: read through as one)
                        if R[r_] == w and size == 'dword' and off < 0x10000:
                            R[r_] = ('field', off, len(ops))
                elif w[0] == 'argderef' and w[2] == len(ops):
                    ops.append((29, off, w[1], 0, []))
                elif still(w):
                    ops.append((30, off, w[1], 0, []))
                elif w[0] in ('fieldand', 'fieldor') and w[1] == off and (w[3] == len(ops) or still(('field', w[1], w[3]))):
                    ops.append((26 if w[0] == 'fieldand' else 27, off, w[2], 0, []))   # (the member unchanged since it was read)
                elif w[0] == 'fieldplus' and still(('field',) + w[1:3]) and w[3] < 0x10000:
                    ops.append((71, off, 0, 0, [('fieldplace', w[1] | w[3] << 16)]))   # (a place inside what a member points to)
                elif w[0] == 'aligned' and w[1] == naligned:
                    ops.append((43, off, 0, 0, []))
                elif w[0] == 'scratch':
                    ops.append((55, off, w[1], 0, []))
                elif (dc := deref_code(w)):
                    ops.append((71, off, 0, 0, [dc]))   # (a dword inside what an argument points to, stored)
                else:
                    raise Reject('tail store ' + s)
            elif size == 'byte' and v in ('al', 'bl', 'cl', 'dl') and val('e' + v[0] + 'x') and val('e' + v[0] + 'x')[0] in ('const', 'byte'):
                ops.append((1, off, val('e' + v[0] + 'x')[1] & 0xff, 0, []))
            elif size == 'byte' and v in ('al', 'bl', 'cl', 'dl') and val('e' + v[0] + 'x') and val('e' + v[0] + 'x')[0] == 'argbyte':
                ops.append((49, off, val('e' + v[0] + 'x')[1], 0, []))   # an argument's low byte
            else:
                raise Reject('tail store ' + s)
        elif re.fullmatch(r'mov (dword|byte) ptr \[esp \+ (0x[0-9a-f]+|\d+)\], (0x[0-9a-f]+|\d+|[a-d]l|e[a-d]x|e[sd]i|ebp|ebx)', s):
            z = re.fullmatch(r'mov dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\], ' + REG, s)
            if z:   # locals (the exception frame's state is a byte or a constant)
                S[num(z.group(1)) - d] = val(z.group(2))
                run_locals.discard(num(z.group(1)) - d)
            elif (z := re.fullmatch(r'mov dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\], (0x[0-9a-f]+|\d+)', s)):
                S[num(z.group(1)) - d] = ('const', num(z.group(2)))
                run_locals.discard(num(z.group(1)) - d)
        elif (m := re.fullmatch(r'mov (e[a-d]x|e[sd]i|ebp), dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', s)) and not pushed and \
                (S.get(num(m.group(2)) - d) or ('',))[0] == 'name' and ('name', S[num(m.group(2)) - d][1]) in built and \
                (rel := plain_release(k, m.group(1), num(m.group(2)))):
            ops.append((57, S[num(m.group(2)) - d][1], 0, 0, []))
            built.discard(('name', S[num(m.group(2)) - d][1]))
            for r in rel[1]:
                R[r] = None
            k = rel[0]
            flag = None
            continue
        elif (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[' + REG + r' \+ (0x[0-9a-f]+|\d+)\]', s)) and val(m.group(2)) and val(m.group(2))[0] == 'temp' and \
                ('temp', val(m.group(2))[1]) in built and num(m.group(3)) < 0x40:
            R[m.group(1)] = ('tempat', val(m.group(2))[1], num(m.group(3)))   # (what a temporary holds: read when given)
            tempat_ops[val(m.group(2))[1]] = len(ops)
        elif (m := re.fullmatch(r'mov dword ptr \[(0x1[0-9a-f]{6})\], ' + REG, s)) and val(m.group(2)) and \
                (val(m.group(2)) == ('this',) or val(m.group(2))[0] == 'member'):
            ops.append((67, 0 if val(m.group(2)) == ('this',) else val(m.group(2))[1], 0, num(m.group(1)), []))   # the window's place kept in a global
        elif (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', s)):
            slot = num(m.group(2)) - d
            if slot in clobbered:
                raise Reject('argument slot reused')
            R[m.group(1)] = ('arg', slot // 4) if slot >= 4 and slot % 4 == 0 and slot // 4 <= nargs else S.get(slot, stored(slot, k))
        elif (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[(0x1[0-9a-f]{6}|0x[ef][0-9a-f]{5})\]', s)):
            R[m.group(1)] = ('global', num(m.group(2)))   # (a game global, or a constant the game keeps: read when used)
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
        elif (m := re.fullmatch(r'mov ([abcd])l, byte ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', s)) and \
                (num(m.group(2)) - d) >= 4 and (num(m.group(2)) - d) % 4 == 0 and (num(m.group(2)) - d) // 4 <= nargs:
            R['e' + m.group(1) + 'x'] = ('argbyte', (num(m.group(2)) - d) // 4)   # an argument's low byte
        elif (m := re.fullmatch(r'xor ([abcd])l, ([abcd])l', s)) and m.group(1) == m.group(2):
            R['e' + m.group(1) + 'x'] = ('byte', 0); flag = None
        elif (m := re.fullmatch(r'lea ' + REG + r', \[esp \+ (0x[0-9a-f]+|\d+)\]', s)):
            R[m.group(1)] = ('stack', num(m.group(2)) - d)
        elif (m := re.fullmatch(r'lea ' + REG + r', \[' + REG + r' \+ (0x[0-9a-f]+|\d+)\]', s)) and val(m.group(2)) and val(m.group(2))[0] in ('this', 'member'):
            R[m.group(1)] = ('member', (0 if val(m.group(2)) == ('this',) else val(m.group(2))[1]) + num(m.group(3)))
        elif (m := re.fullmatch(r'cmp dword ptr \[(0x1[0-9a-f]{6})\], (0|' + REG + ')', s)) and (m.group(2) == '0' or val(m.group(2)) in (('const', 0), ('byte', 0))):
            flag = ('test', ('global', num(m.group(1))))
        elif (m := re.fullmatch(r'(test ' + REG + ', ' + REG + '|cmp ' + REG + ', ' + REG + ')', s)):
            a_, b_ = (m.group(2), m.group(3)) if m.group(2) else (m.group(4), m.group(5))
            flag = ('test', val(a_)) if (a_ == b_ or val(b_) == ('const', 0)) else None
        elif (m := re.fullmatch(r'(or|and) ' + REG + r', (0x[0-9a-f]+|\d+)', s)) and (val(m.group(2)) is None or val(m.group(2))[0] in ('const', 'byte', 'eh')):
            R[m.group(2)] = ('eh',); flag = None   # the exception bookkeeping bits (what's been made)
        elif (m := re.fullmatch(r'or ' + REG + ', ' + REG, s)) and m.group(1) != m.group(2) and val(m.group(2)) and val(m.group(2))[0] == 'const' and \
                (val(m.group(1)) is None or val(m.group(1))[0] in ('const', 'byte', 'eh')):
            R[m.group(1)] = ('eh',); flag = None   # (the same, with the bit kept in a register)
        elif (m := re.fullmatch(r'test ([abcd])([lh]), (0x[0-9a-f]+|\d+)', s)) and val('e' + m.group(1) + 'x') == ('eh',):
            flag = ('ehbit', num(m.group(3)) << (8 if m.group(2) == 'h' else 0))
        elif (m := re.fullmatch(r'test ' + REG + r', (0x[0-9a-f]+|\d+)', s)) and val(m.group(1)) == ('eh',):
            flag = ('ehbit', num(m.group(2)))
        elif (m := re.fullmatch(r'test byte ptr \[esp \+ (0x[0-9a-f]+|\d+)\], (0x[0-9a-f]+|\d+)', s)) and S.get(num(m.group(1)) - d) == ('eh',):
            flag = ('ehbit', num(m.group(2)))   # (the bits as kept on the stack)
        elif (m := re.fullmatch(r'test ([abcd])([lh]), ([abcd])([lh])', s)) and m.group(1, 2) == m.group(3, 4) and val('e' + m.group(1) + 'x') == ('eh',):
            flag = ('ehsign',)   # the bookkeeping bit 0x80 or 0x8000, read through the sign (jns)
        elif (m := re.fullmatch(r'test ([abcd])l, ([abcd])l', s)) and m.group(1) == m.group(2):
            flag = ('noparent?', 'e' + m.group(1) + 'x')   # a byte flag: set when there was no parent (the tester checks)
        elif (m := re.fullmatch(r'cmp byte ptr \[esp \+ (0x[0-9a-f]+|\d+)\], (0|[abcd]l)', s)) and (m.group(2) == '0' or val('e' + m.group(2)[0] + 'x') in (('const', 0), ('byte', 0))):
            flag = ('noparent?', 'stack')
        elif x.mnemonic == 'je' and flag and flag[0] == 'noparent?':
            m1 = re.fullmatch(r'mov ' + REG + r', dword ptr \[(0x1[0-9a-f]{6})\]', t[k + 1])
            if not m1 or num(m1.group(2)) != p['title'] or t[k + 2] != f'mov byte ptr [{m1.group(1)} + 4], 1' or at.get(int(x.op_str, 0)) != k + 3:
                # not the mark: a block skipped when an argument's low byte (the one tested) is 0 (kind 63, fn 2: the byte)
                tgt = at.get(int(x.op_str, 0))
                fv = R.get(flag[1]) if flag[1] in R else None
                if pushed or nulls or tgt is None or tgt <= k + 1 or conds and tgt > conds[-1][0] or not fv or fv[0] != 'argbyte':
                    raise Reject('tail flag mark')
                ops.append((63, 0, 0, 2, [('given', fv[1])]))
                conds.append((tgt, len(ops) - 1, dict(R), dict(S), d, set(built)))
                flag = None
                k += 1
                continue
            ops.append((12, 0, 0, 0, []))
            R[m1.group(1)] = None
            k += 3
            flag = None
            continue
        elif x.mnemonic == 'jne' and flag and flag[1] == ('arg', p.get('parent_arg', 1)) and (nb := name_block(k)):
            # the description (given, or the default) named on its first use, then the default marked (kind 52, then 12)
            k, names_ = nb
            for q_, (off_, name_) in enumerate(names_):
                ops.append((52, off_, name_, int(q_ > 0), []))   # (the first decides, by the description's +4, for all)
            ops.append((12, 0, 0, 0, []))
            for r in list(R):
                if R[r] == ('arg', p.get('parent_arg', 1)) or r in ('eax', 'ecx', 'edx') or R[r] == ('global', p['title']):
                    R[r] = None
            for r in nb_used:
                R[r] = None
            if nb_eax[0] != 'keep':
                R['eax'] = nb_eax[0]
            flag = None
            continue
        elif x.mnemonic == 'jne' and flag and flag[1] == ('arg', p.get('parent_arg', 1)):
            m1 = re.fullmatch(r'mov ' + REG + r', dword ptr \[(0x1[0-9a-f]{6})\]', t[k + 1])
            if not m1 or num(m1.group(2)) != p['title'] or t[k + 2] != f'mov byte ptr [{m1.group(1)} + 4], 1' or at.get(int(x.op_str, 0)) != k + 3:
                raise Reject('tail mark ' + t[k + 1] + ' | ' + t[k + 2])
            ops.append((12, 0, 0, 0, []))
            R[m1.group(1)] = None
            k += 3
            continue
        elif x.mnemonic == 'je' and not pushed and flag and flag[0] == 'test' and (vb := value_block(k)):
            # V + c or nothing / V's interface or nothing, into a scratch value (kind 54)
            k, dest_, how_, base_, extra_, wrote_ = vb
            ops.append((54, nscratch[0], extra_, how_, [base_]))
            for r in wrote_:
                R[r] = None
            R[dest_] = ('scratch', nscratch[0])
            nscratch[0] += 1
            if nscratch[0] > 8:
                raise Reject('scratch values')
            flag = None
            continue
        elif x.mnemonic == 'je' and not pushed and not nulls and flag and flag[0] == 'test' and base_code(flag[1]) is not None and (te := text_or_empty(k)):
            # a text made from a wide text when there is one, else the empty text (kind 68): kept as a name
            k, holder_ = te
            if nnames >= 16:
                raise Reject('names')
            ops.append((68, nnames, 0, 0, [base_code(flag[1])]))
            built.add(('name', nnames))
            for r in ('eax', 'ecx', 'edx'):
                R[r] = None
            R['eax'] = ('name', nnames, None)
            nnames += 1
            flag = None
            continue
        elif x.mnemonic in ('je', 'jne') and not pushed and not nulls and flag and flag[0] == 'test' and base_code(flag[1]) is not None and \
                re.fullmatch(r'0x[0-9a-f]+', x.op_str) and (at.get(int(x.op_str, 0)) or -1) > k + 1 and \
                (not conds or at.get(int(x.op_str, 0)) <= conds[-1][0]) and not (flag[1] and flag[1][0] in ('alloc', 'namealloc')):
            # a block skipped when the value is 0 (je) or isn't (jne): kind 63, its length filled in at its end
            ops.append((63, 0, 0, int(x.mnemonic == 'jne'), [base_code(flag[1])]))
            conds.append((at[int(x.op_str, 0)], len(ops) - 1, dict(R), dict(S), d, set(built)))
            flag = None
        elif x.mnemonic == 'je' and flag and flag[0] == 'ehbit' or x.mnemonic == 'jns' and flag == ('ehsign',):
            # what the bookkeeping bit guards: a temporary's destructor (lea ecx, [esp + N]; call D) or a wide string let go
            # (the one a temporary holds, or a name)
            end_ = at.get(int(x.op_str, 0))
            if end_ is None:
                raise Reject('guarded')
            body = [u for u in t[k + 1:end_] if not re.fullmatch(r'and ' + REG + r', 0x[0-9a-f]+|mov dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\], ' + REG, u)]
            # (the bits kept on the stack are read too, to clear the one guarding this)
            body = [u for u in body if not ((z_ := re.fullmatch(r'mov ' + REG + r', dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', u)) and
                                            S.get(num(z_.group(2)) - d) == ('eh',))]
            z = re.fullmatch(r'lea ecx, \[esp \+ (0x[0-9a-f]+|\d+)\]', body[0]) if body else None
            z2 = re.fullmatch(r'call (0x[0-9a-f]+)', body[1]) if len(body) == 2 else None
            if z and z2:
                slot = num(z.group(1)) - d
                if slot not in temps or ('temp', temps[slot]) not in built or ret_args(num(z2.group(1))) != 0:
                    raise Reject('temporary destroyed')
                ops.append((15, temps[slot], 0, num(z2.group(1)), []))
                built.discard(('temp', temps[slot]))
            else:
                # (a compare with a register that holds 0 is a compare with 0; a saved register the release used is read back
                # from the stack after it, so it holds what it held before)
                body = [re.sub(r'^(cmp dword ptr \[' + REG + r'\]), ' + REG + '$', lambda z_: z_.group(1) + ', 0' if val(z_.group(3)) == ('const', 0) else z_.group(0), u)
                        for u in body]
                freed = max((i for i, u in enumerate(body) if u == 'call 0xbce000'), default=len(body))
                reloaded = {re.fullmatch(r'mov (e[sd]i|ebp|ebx), dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', u).group(1) for u in body[freed:]
                            if re.fullmatch(r'mov (e[sd]i|ebp|ebx), dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', u)}
                body = [u for i, u in enumerate(body) if i <= freed or not re.fullmatch(r'mov (e[sd]i|ebp|ebx), dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', u)]
                for u in body:
                    if (z_ := re.fullmatch(r'mov (e[sd]i|ebp|ebx), .*', u)) and z_.group(1) not in reloaded:
                        R[z_.group(1)] = None
                norm = [re.sub(r'\[esp \+ (0x[0-9a-f]+|\d+)\]', '[esp+X]', re.sub(r'\b(e[a-d]x|e[sd]i|ebp)\b', 'R', u)) for u in body]
                heads = [['mov R, dword ptr [esp+X]', 'mov R, dword ptr [R]', 'dec R', 'mov dword ptr [R], R', 'mov R, dword ptr [esp+X]', 'cmp dword ptr [R], 0', 'jne ' + x.op_str],
                         ['mov R, dword ptr [esp+X]', 'dec dword ptr [R]', 'mov R, dword ptr [esp+X]', 'cmp dword ptr [R], 0', 'jne ' + x.op_str]]
                frees = [['mov R, R', 'mov R, dword ptr [R + 0xc]', 'push R', 'call 0xbce010', 'push R', 'call 0xbce000', 'add esp, 8'],
                         ['mov R, dword ptr [R + 0xc]', 'push R', 'mov R, R', 'call 0xbce010', 'push R', 'call 0xbce000', 'add esp, 8']]
                reads = {u.split('[')[1] for u in body if re.fullmatch(r'mov e[a-d]x, dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', u)}
                if not any(norm == h_ + f_ for h_ in heads for f_ in frees) or len(reads) != 1:
                    raise Reject('guarded release @%#x' % x.address)
                slot = num(re.fullmatch(r'mov e[a-d]x, dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', body[0]).group(1)) - d
                if slot in temps and ('temp', temps[slot]) in built:
                    ops.append((16, temps[slot], 0, 0, []))
                    built.discard(('temp', temps[slot]))
                elif S.get(slot) and S[slot][0] == 'name' and ('name', S[slot][1]) in built:
                    ops.append((17, S[slot][1], 0, 0, []))
                    built.discard(('name', S[slot][1]))
                else:
                    raise Reject('release of what')
            for r in ('eax', 'ecx', 'edx'):
                R[r] = None
            k = end_
            flag = None
            continue
        elif (m := re.fullmatch(r'push (0x[0-9a-f]+|\d+)', s)):
            pushed.append(num(m.group(1))); d += 4
        elif s == 'push ecx' and t[k + 1] == 'mov eax, esp':
            pushed.append(None); d += 4   # room for a 4-byte struct given by value, filled through eax below
            R['eax'] = ('spslot', len(pushed) - 1)
            k += 2
            continue
        elif s == 'push ecx' and t[k + 2] == 'mov eax, esp' and not re.match(r'\w+ (eax|esp)\b', t[k + 1]):
            pushed.append(None); d += 4   # (the same, with one more step before eax is pointed at it)
        elif s == 'mov eax, esp' and pushed and pushed[-1] is None:
            R['eax'] = ('spslot', len(pushed) - 1)
        elif (m := re.fullmatch(r'push ' + REG, s)) and val(m.group(1)) and val(m.group(1))[0] == 'stack' and \
                (S.get(val(m.group(1))[1]) or ('',))[0] == 'name':
            pushed.append(('nameaddr', S[val(m.group(1))[1]][1])); d += 4
        elif (m := re.fullmatch(r'push ' + REG, s)) and val(m.group(1)) and val(m.group(1))[0] == 'stack' and                 (S.get(val(m.group(1))[1]) or ('',))[0] == 'obj' and S[val(m.group(1))[1]][1] == cur and last_text_object():
            # the new object a text (0x10 bytes made with 0xBCF850), kept on the stack and its place given: a name (kind 58)
            if nnames >= 16:
                raise Reject('names')
            ops.append((58, nnames, 0, 0, []))
            built.add(('name', nnames))
            S[val(m.group(1))[1]] = ('name', nnames, None)
            pushed.append(('nameaddr', nnames)); d += 4
            nnames += 1
        elif (m := re.fullmatch(r'push ' + REG, s)) and val(m.group(1)) and val(m.group(1))[0] == 'stack':
            pushed.append(('rect', val(m.group(1))[1])); d += 4   # a local's address: only a layout's rectangle is understood
        elif (m := re.fullmatch(r'push ' + REG, s)) and still(val(m.group(1))):
            pushed.append(('field', val(m.group(1))[1])); d += 4
        elif (m := re.fullmatch(r'push ' + REG, s)) and val(m.group(1)) and val(m.group(1))[0] == 'scratch':
            pushed.append(('scratch', val(m.group(1))[1])); d += 4
        elif (m := re.fullmatch(r'push ' + REG, s)) and (dc := deref_code(val(m.group(1)))):
            pushed.append(dc); d += 4   # (a dword inside what an argument points to)
        elif (m := re.fullmatch(r'push ' + REG, s)) and val(m.group(1)) and val(m.group(1))[0] == 'fieldplus' and still(('field',) + val(m.group(1))[1:3]) and \
                val(m.group(1))[1] < 0x10000 and val(m.group(1))[3] < 0x10000:
            pushed.append(('fieldplace', val(m.group(1))[1] | val(m.group(1))[3] << 16)); d += 4   # a place inside what a member points to
        elif (m := re.fullmatch(r'push ' + REG, s)) and (sc := take_result(val(m.group(1)))):
            for r in list(R):
                if R[r] == val(m.group(1)) and r != m.group(1):
                    R[r] = sc
            R[m.group(1)] = sc
            pushed.append(sc); d += 4
        elif (m := re.fullmatch(r'push ' + REG, s)) and val(m.group(1)) and val(m.group(1))[0] == 'tempat' and ('temp', val(m.group(1))[1]) in built and \
                all(o[0] not in (15, 16) for o in ops[tempat_ops[val(m.group(1))[1]]:]):
            pushed.append(('tempat', val(m.group(1))[1] | val(m.group(1))[2] << 8)); d += 4
        elif (m := re.fullmatch(r'push ' + REG, s)) and val(m.group(1)) and val(m.group(1))[0] in ('const', 'global', 'arg', 'this', 'member', 'temp'):
            v = val(m.group(1))
            pushed.append(v[1] if v[0] == 'const' else ('self', 0) if v[0] == 'this' else ('self', v[1]) if v[0] == 'member' else
                          ('temp', v[1]) if v[0] == 'temp' else ('glob' if v[0] == 'global' else 'given', v[1])); d += 4
        elif (m := re.fullmatch(r'add esp, (0x[0-9a-f]+|\d+)', s)) and num(m.group(1)) // 4 <= len(pushed):
            for _ in range(num(m.group(1)) // 4):
                pushed.pop()
            d -= num(m.group(1))
        elif s == 'call 0xbcdfc0' and pushed and isinstance(pushed[-1], int):
            serial += 1
            if cur is None or nulls == []:
                # a new object (the one before must be done with)
                if nulls:
                    raise Reject('new inside new')
                ops.append((18, 0, pushed[-1], 0, []))
                cur = serial
                R['eax'] = ('alloc', serial)
            else:
                if pushed[-1] != 0x10:
                    raise Reject('inner new')
                R['eax'] = ('namealloc', serial)
            R['ecx'] = None; R['edx'] = None
            flag = None
        elif s == 'call 0xbcdfd0' and pushed and isinstance(pushed[-1], int) and not nulls:
            # an aligned block of memory (the caller takes the size off), kept until it's stored into a member
            naligned += 1
            ops.append((42, 0, pushed[-1], 0, []))
            R['eax'] = ('aligned', naligned); R['ecx'] = None; R['edx'] = None
            flag = None
        elif x.mnemonic == 'je' and flag and flag[1] and flag[1][0] in ('alloc', 'namealloc'):
            nulls.append((int(x.op_str, 0), flag[1]))
            flag = None
        elif x.mnemonic == 'jmp' and nulls and ins[k + 1].address == nulls[-1][0]:
            # past the null branch: it must only set the register the result is in to null, then join here
            target, token = nulls.pop()
            z = re.fullmatch(r'xor ' + REG + ', ' + REG, t[k + 1])
            ok_ = z and z.group(1) == z.group(2) and ins[k + 2].address == int(x.op_str, 0) and val(z.group(1))
            if ok_ and token[0] == 'alloc':
                ok_ = val(z.group(1))[0] == 'obj' and val(z.group(1))[1] == token[1]
            elif ok_:
                ok_ = val(z.group(1))[0] == 'name' and val(z.group(1))[2] == token[1]
            if not ok_:
                raise Reject('null branch')
            k += 2
            flag = None
            continue
        elif (m := re.fullmatch(r'call (0x[0-9a-f]+)', s)) and val('ecx') and val('ecx')[0] == 'namealloc':
            if num(m.group(1)) != 0xbcf850 or not pushed or not isinstance(pushed[-1], int):
                raise Reject('name made with')
            lit = pushed.pop(); d -= 4
            token = val('ecx')
            if nnames >= 16:
                raise Reject('names')
            ops.append((21, nnames, lit, 0, []))
            built.add(('name', nnames))
            for r in list(R):
                if R[r] == token:
                    R[r] = ('name', nnames, token[1])
            R['eax'] = ('name', nnames, token[1]); R['ecx'] = None; R['edx'] = None
            nnames += 1
            flag = None
        elif (m := re.fullmatch(r'call (0x[0-9a-f]+)', s)) and val('ecx') and val('ecx')[0] == 'stack' and val('ecx')[1] in temps and \
                ('temp', temps[val('ecx')[1]]) in built and not nulls and ret_args(num(m.group(1))) == 0:
            # a temporary made outside a new object's making, destroyed (kind 15)
            ops.append((15, temps[val('ecx')[1]], 0, num(m.group(1)), []))
            built.discard(('temp', temps[val('ecx')[1]]))
            for r in ('eax', 'ecx', 'edx'):
                R[r] = None
            flag = None
        elif (m := re.fullmatch(r'call (0x[0-9a-f]+)', s)) and val('ecx') and val('ecx')[0] == 'stack' and not nulls and \
                ret_args(num(m.group(1))) is not None and 0 < ret_args(num(m.group(1))) <= min(4, len(pushed)) and \
                num(m.group(1)) in (0x41cb90, 0x4040f0, 0xbd1940):
            # a text reference made on the stack outside a new object's making: always (kind 20, value 1)
            fn, slot = num(m.group(1)), val('ecx')[1]
            n_ = ret_args(fn)
            targs = [pushed.pop() for _ in range(n_)]
            d -= 4 * n_
            if any(not isinstance(v, int) for v in targs):
                raise Reject('text reference values')
            kk = len([o for o in ops if o[0] == 20])
            if kk >= 16 or slot in temps and ('temp', temps[slot]) in built:
                raise Reject('temporaries')
            temps[slot] = kk
            ops.append((20, kk, 1, fn, targs))
            if fn == 0xbd1940:
                pod_temps.add(kk)   # (a key combination: plain values, nothing to destroy)
            else:
                built.add(('temp', kk))
            always_temps.add(kk)
            R['eax'] = ('temp', kk); R['ecx'] = None; R['edx'] = None
            flag = None
        elif (m := re.fullmatch(r'call (0x[0-9a-f]+)', s)) and val('ecx') and val('ecx')[0] == 'stack':
            # a temporary on the stack, made while making the new object (so only when there was memory)
            fn, slot = num(m.group(1)), val('ecx')[1]
            n_ = ret_args(fn)
            if cur is None or not nulls or n_ is None or n_ > 4 or n_ > len(pushed):
                raise Reject('stack temporary cur=%s nulls=%s n=%s pushed=%s @%#x' % (cur, len(nulls), n_, len(pushed), x.address))
            targs = [pushed.pop() for _ in range(n_)]
            d -= 4 * n_
            if any(not (isinstance(v, int) or v[0] in ('nameaddr', 'self', 'glob', 'given', 'temp')) for v in targs):
                raise Reject('stack temporary values')
            kk = len([o for o in ops if o[0] == 20])
            if kk >= 16 or slot in temps and ('temp', temps[slot]) in built:
                raise Reject('temporaries')
            temps[slot] = kk
            ops.append((20, kk, 0, fn, targs))
            built.add(('temp', kk))
            R['eax'] = ('temp', kk); R['ecx'] = None; R['edx'] = None
            flag = None
        elif (m := re.fullmatch(r'call (0x[0-9a-f]+)', s)) and val('ecx') and val('ecx')[0] == 'alloc' and val('ecx')[1] == cur:
            fn, token = num(m.group(1)), val('ecx')
            n_ = ret_args(fn)
            if n_ is None or n_ > len(pushed) or n_ > 5:
                raise Reject('new object constructor')
            args = [pushed.pop() for _ in range(n_)]
            d -= 4 * n_
            ops.append((10, 0, 0, fn, args))
            for r in list(R):
                if R[r] == token:
                    R[r] = ('obj', cur, 'memory')
            R['eax'] = ('obj', cur, 'returned'); R['ecx'] = None; R['edx'] = None
            flag = None
        elif s == 'call 0xbd0490' and val('ecx') in (('byte', 1), ('const', 1)) and not pushed:
            R['eax'] = ('empty',); R['ecx'] = None; R['edx'] = None
            flag = None
        elif (s == 'call 0x805750' or s == 'call 0x805700' and val('ecx') and (val('ecx') == ('const', 0) or val('ecx')[0] == 'arg')) and not pushed:
            # a reference member at M: its handle at M + 4, and the link at M + 8 put first in the handle's list
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
            ops.append((8, member, 0, 0, []) if s == 'call 0x805750' else (9, member, val('ecx')[1] if val('ecx')[0] == 'arg' else 0, 0, []))
            for r in ('eax', 'ecx', 'edx'):
                R[r] = None
            for u in t[k + 2:k + 9]:
                if (z := re.fullmatch(r'mov ' + REG + ', .*', u)):
                    R[z.group(1)] = None
            flag = None
            k += 9
            continue
        elif s == 'call 0x6b1640' and (still(val('ecx')) or val('ecx') and val('ecx')[0] == 'obj' and val('ecx')[1] == cur and not nulls) and \
                len(pushed) >= 3 and isinstance(pushed[-3], tuple) and pushed[-3][0] == 'rect':
            # a child laid out at a rectangle of constants (op 40, bit 6)
            relation, to, rect = pushed[-1], pushed[-2], pushed[-3]
            del pushed[-3:]
            d -= 12
            rv = [S.get(rect[1] + 4 * i) for i in range(4)]
            if not isinstance(relation, int) or any(not r_ or r_[0] != 'const' for r_ in rv):
                raise Reject('layout rectangle')
            if to == ('self', 0):
                how, to_off = 32, 0
            elif isinstance(to, tuple) and to[0] == 'field':
                how, to_off = 0, to[1]
            else:
                raise Reject('layout relative to')
            if val('ecx')[0] == 'obj':   # (the new object laid out: what its constructor returned, or its memory)
                ops.append((40, 0 if val('ecx')[2] == 'returned' else 1, to_off, 0x6b1640, [r_[1] for r_ in rv] + [relation << 8 | 64 | 128 | how]))
            else:
                ops.append((40, val('ecx')[1], to_off, 0x6b1640, [r_[1] for r_ in rv] + [relation << 8 | 64 | how]))
            for r in ('eax', 'ecx', 'edx'):
                R[r] = None
            flag = None
        elif (m := re.fullmatch(r'add ' + REG + r', (0x[0-9a-f]+|\d+)', s)) and still(val(m.group(1))) and num(m.group(2)) < 0x10000:
            R[m.group(1)] = ('fieldplus', val(m.group(1))[1], val(m.group(1))[2], num(m.group(2)))   # (a place inside what a member points to)
            flag = None
        elif (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[ecx\]', s)) and val('ecx') and (val('ecx') == ('this',) or val('ecx')[0] == 'member' or still(val('ecx'))) \
                and m.group(1) != 'ecx':
            R[m.group(1)] = ('vtable', val('ecx'))   # (the object's table: a call through it follows)
        elif (m := re.fullmatch(r'call dword ptr \[' + REG + r' \+ 0x20\]', s)) and pushed and isinstance(pushed[-1], tuple) and pushed[-1][0] == 'rect' and \
                (fs := fpu_step(k, size_query=True)):
            edx_live[0] = False
            ops.append(fs[1])   # a size asked for, then an x87 run with what it gave, and the call given what it built (kind 59)
            for r in ('eax', 'ecx', 'edx'):
                R[r] = None
            k = fs[0]
            flag = None
            continue
        elif (m := re.fullmatch(r'call dword ptr \[' + REG + r' \+ (0x[0-9a-f]+|\d+)\]', s)) and val(m.group(1)) and val(m.group(1))[0] == 'vtable' and \
                val(m.group(1))[1] == val('ecx') and (val('ecx') == ('this',) or val('ecx')[0] == 'member' or still(val('ecx'))) and \
                num(m.group(2)) % 4 == 0 and num(m.group(2)) < 0x200 and len(pushed) <= 4 and None not in pushed:
            # a virtual call on the window, a member or what a member points to (kind 60): what was pushed since the last
            # call is what it takes (the tester's stand-in for the slot takes as many, the same in every constructor)
            on = val('ecx')
            target = 0 if on == ('this',) else on[1] if on[0] == 'member' else 1 << 24 | on[1]
            n_ = len(pushed)
            args = [pushed.pop() for _ in range(n_)]
            d -= 4 * n_
            if any(not isinstance(v, int) and v[0] in ('temp', 'nameaddr', 'rect', 'local') and not (v[0] == 'temp' and v[1] in always_temps) for v in args):
                raise Reject('virtual call with a temporary')
            slot_ = num(m.group(2))
            if VSLOTS.setdefault(slot_, n_) != n_:
                raise Reject('virtual slot taking another count')
            ops.append((60, target, slot_, 0, args))
            for r in ('eax', 'ecx', 'edx'):
                R[r] = None
            R['eax'] = ('result', len(ops) - 1)   # (what it gave back: kept, op 64 takes it while nothing else was called)
            flag = None
        elif s in ('call 0xbce000', 'call 0xbce010') and pushed and pushed[-1] is not None and t[k + 1] == 'add esp, 4' and \
                (isinstance(pushed[-1], int) or pushed[-1][0] in ('field', 'glob', 'given', 'scratch')):
            ops.append((65, 0, 0, num(s[5:]), [pushed[-1]]))   # memory freed (0xBCE000, or 0xBCE010 aligned)
            for r in ('eax', 'ecx', 'edx'):
                R[r] = None
            flag = None
        elif (m := re.fullmatch(r'call (0x[0-9a-f]+)', s)) and val('ecx') and val('ecx')[0] == 'fieldplus' and still(('field',) + val('ecx')[1:3]):
            fn, n_ = num(m.group(1)), ret_args(num(m.group(1)))
            if n_ is None or n_ > len(pushed) or n_ > 5:
                raise Reject('field call ' + s)
            args = [pushed.pop() for _ in range(n_)]
            d -= 4 * n_
            if any((not isinstance(v, int) and v[0] in ('temp', 'nameaddr') and not (v[0] == 'temp' and v[1] in always_temps and (('temp', v[1]) in built or v[1] in pod_temps)) and not (v[0] == 'nameaddr' and ('name', v[1]) in built and any(o_[0] in (58, 68) and o_[1] == v[1] for o_ in ops))) for v in args):
                raise Reject('field call with a temporary')
            ops.append((24, val('ecx')[1], val('ecx')[3], fn, args))   # (on the place that far into what the member points to)
            for r in ('eax', 'ecx', 'edx'):
                R[r] = None
            R['eax'] = ('result', len(ops) - 1)   # (what it gave back: kept, op 64 takes it while nothing else was called)
            flag = None
        elif (m := re.fullmatch(r'call (0x[0-9a-f]+)', s)) and still(val('ecx')):
            fn, n_ = num(m.group(1)), ret_args(num(m.group(1)))
            if n_ is None or n_ > len(pushed) or n_ > 5:
                raise Reject('field call ' + s)
            args = [pushed.pop() for _ in range(n_)]
            d -= 4 * n_
            if any((not isinstance(v, int) and v[0] in ('temp', 'nameaddr') and not (v[0] == 'temp' and v[1] in always_temps and (('temp', v[1]) in built or v[1] in pod_temps)) and not (v[0] == 'nameaddr' and ('name', v[1]) in built and any(o_[0] in (58, 68) and o_[1] == v[1] for o_ in ops))) for v in args):
                raise Reject('field call with a temporary')
            ops.append((24, val('ecx')[1], 0, fn, args))
            for r in ('eax', 'ecx', 'edx'):
                R[r] = None
            R['eax'] = ('result', len(ops) - 1)
            flag = None
        elif (m := re.fullmatch(r'call (0x[0-9a-f]+)', s)):
            fn, this = num(m.group(1)), val('ecx')
            if this and this[0] == 'obj' and this[1] == cur and ret_args(fn) is not None and ret_args(fn) <= min(len(pushed), 5):
                # a call on the new object: what its constructor returned or its memory; inside the branch where there
                # was memory only then, after it always (on null when there was none)
                n_ = ret_args(fn)
                args = [pushed.pop() for _ in range(n_)]
                d -= 4 * n_
                if any(not isinstance(v, int) and v[0] in ('temp', 'nameaddr', 'rect') for v in args):
                    raise Reject('new object call with a temporary')
                ops.append((46, 0 if this[2] == 'returned' else 1, int(bool(nulls)), fn, args))
                for r in ('eax', 'ecx', 'edx'):
                    R[r] = None
                flag = None
                k += 1
                continue
            if not this or this[0] not in ('this', 'member', 'arg', 'global') or ret_args(fn) is None or ret_args(fn) > len(pushed) or \
                    ret_args(fn) > (7 if this[0] in ('this', 'member') else 5) or this[0] == 'arg' and this[1] < 2:
                raise Reject('tail call ' + s)
            n_ = ret_args(fn)
            args = [pushed.pop() for _ in range(n_)]
            d -= 4 * n_
            if any((not isinstance(v, int) and v[0] in ('temp', 'nameaddr') and not (v[0] == 'temp' and v[1] in always_temps and (('temp', v[1]) in built or v[1] in pod_temps)) and not (v[0] == 'nameaddr' and ('name', v[1]) in built and any(o_[0] in (58, 68) and o_[1] == v[1] for o_ in ops))) for v in args):
                raise Reject('member call with a temporary')
            if this[0] == 'arg':
                ops.append((61, this[1], 0, fn, args))   # on the constructor's argument (an object it was given)
            elif this[0] == 'global':
                ops.append((62, 0, this[1], fn, args))   # on the object a game global holds (read now)
            else:
                ops.append((3, 0 if this == ('this',) else this[1], 0, fn, args))
            for r in ('eax', 'ecx', 'edx'):
                R[r] = None
            R['eax'] = ('result', len(ops) - 1)   # (what it gave back: kept, op 64 takes it while nothing else was called)
            flag = None
        elif (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\]', s)) and val(m.group(2)) and \
                val(m.group(2))[0] == 'arg' and val(m.group(2))[1] not in clobbered_args() \
                and num(m.group(3) or '0') < 0x10000:
            b_ = val(m.group(2))
            R[m.group(1)] = ('deref', 'arg', b_[1], num(m.group(3) or '0'), len(ops))
        else:
            raise Reject('tail: ' + s + ' @%#x' % x.address)
        if x.mnemonic in FLAGS and not s.startswith(('test', 'cmp')):
            flag = None
        k += 1
    if pushed or nulls or built:
        raise Reject('tail left %s %s %s' % (len(pushed), len(nulls), sorted(built)))
    if ops.count((12, 0, 0, 0, [])) > 1:
        raise Reject('two marks')
    if any(isinstance(v, tuple) and v[0] == 'rect' for o in ops for v in o[4]):
        raise Reject('a local address given')
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
    info, callees, temps_used, nullable = [], {}, set(), []
    byte_flags = []   # the constructors with arguments only a byte test reads (bit n: argument n)
    refs = []   # the text references made before a base: (key, table, flag)
    all_ops = {}
    edx_uses, plain_uses = set(), set()
    rect_uses, norect_uses = set(), set()   # the callees given a run's locals (the tester notes the 16 bytes there)   # the callees made with edx = the window, and those made without
    for a, r in sorted(rows.items()):
        n = int(r['size'])
        if (r['status'] != 'documented' and a not in mine) or n > 4000:
            continue
        code = bytes(img[a - BASE:a - BASE + n])
        if SEPARATOR.to_bytes(4, 'little') not in code:
            continue
        try:
            p = parse(a, n)
            if p['nargs'] < 2 or p['nargs'] > 7:
                raise Reject('arguments %d' % p['nargs'])
        except (Reject, StopIteration, ValueError, IndexError, KeyError) as e:
            key = re.sub(r' @0x[0-9a-f]+', '', type(e).__name__ + ' ' + str(e))[:60]
            reasons[key] += 1; rb[key] += n
            if why and key.startswith(why):
                print(hex(a), e)
            continue
        ok += 1; okb += n
        ex = p['extras']
        # (is_arg 2: the text reference made just before the base, titled_refs at value; 3: the start text's place, value its literal)
        def extra_code(e):
            if e[0] == 'ref':
                refs.append(p['ref'][:3])
                return f'{{2, {len(refs) - 1:#x}}}'
            return f'{{{ {"arg": 1, "const": 0, "startname": 3}[e[0]] }, {e[1]:#x}}}'
        out.append(f'static const TitledExtra titled_{a:x}_extras[] = {{' + (', '.join(extra_code(e) for e in ex) or '{0, 0}') + '};')
        def op_text(o):
            kind, off, value, fn, args = o
            codes = []
            for v in args:
                if isinstance(v, int):
                    codes.append((0, v))
                else:
                    codes.append(({'glob': 1, 'given': 2, 'self': 3, 'temp': 4, 'nameaddr': 5, 'field': 6, 'scratch': 7, 'tempat': 8, 'local': 9, 'fieldplace': 10, 'darg': 12}[v[0]], v[1]))
            kinds = sum(c << (4 * q) for q, (c, _) in enumerate(codes))
            return f'{{{kind}, {off:#x}, {value:#x}, {fn:#x}, {len(args)}, {{{", ".join(hex(v) for _, v in codes) or "0"}}}, {kinds:#x}}}'
        out.append(f'static const TitledOp titled_{a:x}_ops[] = {{' + (', '.join(op_text(o) for o in p['ops']) or '{0, 0, 0, 0, 0, {0}, 0}') + '};')
        out.append(f'static const uint8_t titled_{a:x}_original[] = {{' + ', '.join(f'0x{b:02x}' for b in img[a - BASE:a - BASE + 7]) + '};')
        pt_ = f', {p["parent_arg"]}, {p["text_arg"]}' if (p['parent_arg'], p['text_arg']) != (1, 2) else ''
        out.append(f'REWRITE("Window::Titled", {a:#x}, titled_{a:x}_original, (&titled_construct{p["nargs"]}<{p["title"]:#x}, {p["base"]:#x}, titled_{a:x}_extras, {len(ex)}, '
                   f'titled_{a:x}_ops, {len(p["ops"])}{pt_}>), &g_titled_constructs);')
        info.append((a, p['title'], p['nargs'], p['base'], 2 + len(ex), p['parent_arg'], p['text_arg']))
        all_ops[a] = p['ops']
        # the arguments only a cast reads (V ? V + c : 0): the tester gives them 0 at times (none reads through them)
        # (and those a conditional block tests, so both of its ways are run)
        cast_args = {o[4][0][1] for o in p['ops'] if o[0] in (54, 63) and (o[0] == 63 or o[3] == 0) and isinstance(o[4][0], tuple) and o[4][0][0] == 'given'}
        deref_args = {o[2] for o in p['ops'] if o[0] in (29, 50)} | {o[1] for o in p['ops'] if o[0] == 61} | {p['parent_arg'], p['text_arg']}
        mask = sum(1 << n_ for n_ in cast_args - deref_args)
        if mask:
            nullable.append((a, mask))
        # the arguments only a block's byte test reads (flags): the tester gives them small values (0, 1 or 2)
        byte_args = {o[4][0][1] for o in p['ops'] if o[0] == 63 and o[3] == 2 and isinstance(o[4][0], tuple) and o[4][0][0] == 'given'}
        other_uses = {v[1] for o in p['ops'] if not (o[0] == 63 and o[3] == 2) for v in o[4] if isinstance(v, tuple) and v[0] == 'given'} | \
                     {o[2] for o in p['ops'] if o[0] in (2, 29, 50)} | {e[1] for e in p['extras'] if e[0] == 'arg'}
        bmask = sum(1 << n_ for n_ in byte_args - other_uses - deref_args)
        if bmask:
            byte_flags.append((a, bmask))
        for o in p['ops']:
            if o[0] in (3, 10, 24, 59, 61, 62) and o[3] != 0x6b1640:   # (0x6B1640: the tester's own stand-in, which notes the rectangle)
                callees[o[3]] = len(o[4])
                (edx_uses if o[0] in (3, 24) and o[2] >> 31 else plain_uses).add(o[3])
                (rect_uses if o[0] == 59 and any(isinstance(v, tuple) and v[0] == 'local' for v in o[4]) else norect_uses).add(o[3])
            if o[0] in (15, 20):
                temps_used.add(o[3])
    out.append('// each constructor: its address, title global, how many values it takes, its base and how many values that takes')
    out.append('// constructors with arguments only a block\'s byte test reads (bit n: argument n): the tester gives them 0, 1 or 2')
    out.append('extern const uint32_t titled_byte_flags[];')
    out.append('const uint32_t titled_byte_flags[] = {' + ''.join(f'{a_:#x}, {m_:#x}, ' for a_, m_ in byte_flags) + '0};')
    out.append('// constructors with arguments only a cast reads (bit n: argument n), which the tester gives 0 at times')
    out.append('extern const uint32_t titled_nullable[];')
    out.append('const uint32_t titled_nullable[] = {' + ''.join(f'{a:#x}, {m:#x}, ' for a, m in nullable) + '0};')
    out.append('extern const uint32_t titled_info[];')
    out.append('const uint32_t titled_info[] = {' + ''.join(f'{a:#x}, {tl:#x}, {na}, {b:#x}, {bn}, {pa}, {ta}, ' for a, tl, na, b, bn, pa, ta in info) + '0};')
    out.append('// the functions the members are made with or called, and how many values each takes (the tester stands in for them)')
    out.append('extern const uint32_t titled_callees[];')
    # (count | 0x100: always made with edx = the window, which the tester's stand-in notes too)
    # (| 0x200: given a run's locals, whose 16 bytes the tester's stand-in notes)
    out.append('const uint32_t titled_callees[] = {' + ''.join(f'{fn:#x}, {c | (0x100 if fn in edx_uses and fn not in plain_uses else 0) | (0x200 if fn in rect_uses else 0):#x}, ' for fn, c in sorted(callees.items())) + '0};')
    out.append('// the text references made just before a base (TitledExtra is_arg 2): key, table, flag')
    out.append('extern const uint32_t titled_refs[];')
    out.append('const uint32_t titled_refs[] = {' + ''.join(f'{x:#x}, {y:#x}, {z:#x}, ' for x, y, z in refs) + '0};')
    out.append('// the game globals a text is made from when they hold one (op 68): the tester gives each a text or none in the call')
    out.append('extern const uint32_t titled_text_globals[];')
    tg = sorted({o[4][0][1] for a_ in info for o in all_ops.get(a_[0], []) if o[0] == 68 and isinstance(o[4][0], tuple) and o[4][0][0] == 'glob'})
    out.append('const uint32_t titled_text_globals[] = {' + ''.join(f'{g:#x}, ' for g in tg) + '0};')
    out.append('// the game globals the steps write (op 51: off bytes; op 67: 4): the tester gives each a value first and compares them')
    out.append('extern const uint32_t titled_written_globals[];')
    wg = sorted({(o[3], o[1] if o[0] == 51 else 4) for a_ in info for o in all_ops.get(a_[0], []) if o[0] in (51, 67)})
    out.append('const uint32_t titled_written_globals[] = {' + ''.join(f'{g:#x}, {n_}, ' for g, n_ in wg) + '0};')
    out.append('// the game globals whose objects are called on (op 62): the tester gives each an object of its own in the call')
    out.append('extern const uint32_t titled_object_globals[];')
    objg = sorted({o[2] for a_ in info for o in all_ops.get(a_[0], []) if o[0] == 62} | {o[3] for a_ in info for o in all_ops.get(a_[0], []) if o[0] == 73})
    out.append('const uint32_t titled_object_globals[] = {' + ''.join(f'{g:#x}, ' for g in objg) + '0};')
    out.append('// the virtual slots called (op 60) and how many values each takes: the tester stands in for each')
    out.append('extern const uint32_t titled_vslots[];')
    out.append('const uint32_t titled_vslots[] = {' + ''.join(f'{sl:#x}, {c}, ' for sl, c in sorted(VSLOTS.items()) if sl != 0x20) + '0xffffffff};')
    out.append('// the x87 runs (op 59): each a count, then its steps {code, kind, value, local}')
    out.append('extern const uint32_t titled_fpu[];')
    out.append('const uint32_t titled_fpu[] = {' + ''.join(f'{c:#x}, {kd:#x}, {v:#x}, {lc:#x}, ' for c, kd, v, lc in FPU_PROGRAM) + '0};')
    unknown_temps = temps_used - {0x4040f0, 0xbd1880, 0x6a39d0, 0x41cb90, 0x41c740, 0xbd1940}   # (0xBD1940: the game's own runs)
    if unknown_temps:
        print('temporaries the tester has no stand-in for:', [hex(x) for x in sorted(unknown_temps)])
    # the game globals the steps read as plain values (stored, given to stand-ins, tested): the tester gives each a value of
    # the case's first, so a step reading the wrong one shows (the game's own state has most of them 0). Left out: those
    # read through (objects called on, interfaces, texts) and the title and text-copy globals.
    deref_g = set(objg) | set(tg) | {a_[1] for a_ in info}
    readg = set()
    for a_ in info:
        for o in all_ops.get(a_[0], []):
            if o[0] in (5, 32, 45):
                readg.add(o[2])
            if o[0] == 54:
                deref_g.update(v[1] for v in o[4] if isinstance(v, tuple) and v[0] == 'glob')
            elif o[0] in (3, 10, 20, 24, 46, 59, 60, 61, 62, 63, 65):
                readg.update(v[1] for v in o[4] if isinstance(v, tuple) and v[0] == 'glob')
    readg -= deref_g
    out.append('// the game globals the steps read as plain values: the tester gives each a value of the case\'s first in the call')
    out.append('extern const uint32_t titled_read_globals[];')
    out.append('const uint32_t titled_read_globals[] = {' + ''.join(f'{g:#x}, ' for g in sorted(readg)) + '0};')
    open(OUT, 'w', newline='\n').write('\n'.join(out) + '\n')
    print(ok, 'constructors,', okb, 'bytes')
    for k, v in rb.most_common(15):
        print(f'{v:7d} {reasons[k]:4d} {k}')
