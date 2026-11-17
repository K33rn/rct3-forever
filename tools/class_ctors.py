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
REG = r'(e[a-d]x|e[sd]i|ebp|ebx|__below)'
# Constructors whose last lines are written by hand in classes.cpp (a function run after everything else): the
# instruction those lines start with, and the function. Everything before it is still read and checked here.
HAND_AFTER = {0x7df320: ('call 0x425920', 'person_after'), 0x947720: ('mov ecx, 0x133a2bc', 'water_manager_after'),
              0xb4b7a0: ('mov eax, dword ptr [0x10f28e8]', 'link_last_b4b7a0'), 0x6404d0: ('fld dword ptr [0x10eead0]', 'scaled_value_6404d0'),
              0x9d91f0: ('lea ecx, [esp + 0x10]', 'empty_reference_9d91f0'), 0xa5d6f0: ('push 6', 'slots_a5d6f0'),
              0x76b880: ('call 0x475180', 'random_values_76b880')}
# Stretches inside the member set-up written by hand (classes.cpp): from the instruction they start with to the one they
# end with (both taken), and the function that does them. The stretch must leave the stack as it found it; the registers
# it writes are forgotten.
HAND_STEPS = {0x56bf80: [('lea ecx, [esp + 0x2c]', 'call 0xbd99e0', 'ride_train_status_text')],
              0x9e8ff0: [('fld dword ptr [0x10f1cf8]', 'call 0x9e87e0', 'weather_start')],
              0x830740: [('lea eax, [esp + 0x44]', 'call 0x48c9c0', 'member_from_pair_830740')],
              0x707680: [('call 0x704a50', 'call 0x707530', 'park_figures')],
              0x9e51d0: [('mov eax, 0x3f800000', 'mov dword ptr [esi + 0x3e0], eax', 'sky_colours')],
              0x947720: [('cmp dword ptr [0x133a274], edi', 'call 0x403f30', 'water_manager_name')]}
HANDS = []   # the hand-written stretches' functions, in the order class_hand numbers them
num = lambda s: int(s, 0)


def ret_args(fn):
    """How many dwords a function takes off the stack (its first ret)."""
    for x in md.disasm(bytes(img[fn - BASE:fn - BASE + 0x8000]), fn):   # (long enough for the largest constructors)
        if x.mnemonic == 'ret':
            return num(x.op_str) // 4 if x.op_str else 0
    return None


read_globals = set()   # the game globals the steps read as values: the tester gives each a value of the case's
ref_setters = set()   # the member functions given a stack reference (kind 25): the tester stands in for them as for 0x560D20
value_calls = set()   # the functions taking nothing whose value a member gets (kind 23): the tester stands in for them


def leaves_ecx(fn):
    # whether a function writes ecx and edx before reading them, in its first straight run (it can be called with whatever
    # is in them)
    seen = set()
    for x in md.disasm(bytes(img[fn - BASE:fn - BASE + 0x80]), fn):
        s = x.mnemonic + ' ' + x.op_str
        if s == 'call 0xddd770':
            continue   # (__chkstk, the stack probe: it gives ecx back as it found it; edx it doesn't touch)
        if x.mnemonic.startswith('j') or x.mnemonic in ('call', 'ret'):
            return True   # (neither read before it: a read returns False below)
        for r in ('ecx', 'edx'):
            if r in seen:
                continue
            m = re.fullmatch(r'(mov|lea|xor) ' + r + r', (.*)', s)
            reads = re.search(r'\b' + r + r'\b|\b' + r[1] + r'[lhx]\b', m.group(2) if m else s)
            if m and not (m.group(1) == 'xor' and m.group(2) == r) and not reads:
                seen.add(r)
            elif m and m.group(1) == 'xor' and m.group(2) == r:
                seen.add(r)
            elif reads:
                return False
    return False


def returns_this(fn):
    # whether a function's first ret gives back what it got in ecx (mov eax, ecx, directly or through a register kept since)
    holds = {'ecx'}
    for x in md.disasm(bytes(img[fn - BASE:fn - BASE + 0x400]), fn):
        s = x.mnemonic + ' ' + x.op_str
        if x.mnemonic == 'ret':
            return 'eax' in holds
        if x.mnemonic.startswith('j') or x.mnemonic == 'call':
            if x.mnemonic == 'call':
                holds -= {'eax', 'ecx', 'edx'}
            if x.mnemonic.startswith('j') and x.mnemonic != 'jmp':
                continue
            if x.mnemonic == 'jmp':
                return False
        m = re.fullmatch(r'mov (e[a-d]x|e[sd]i|ebp|ebx), (e[a-d]x|e[sd]i|ebp|ebx)', s)
        if m:
            if m.group(2) in holds: holds.add(m.group(1))
            else: holds.discard(m.group(1))
            continue
        if x.mnemonic == 'push':
            continue
        if x.mnemonic == 'pop' and x.op_str in holds:
            holds.discard(x.op_str)
            continue
        w = re.match(r'(mov|lea|xor|or|and|add|sub|inc|dec|neg|not|shl|shr|sar|imul|movzx|movsx|pop) (e[a-d]x|e[sd]i|ebp|ebx)\b', s)
        if w:
            holds.discard(w.group(2))
    return False


def written(r, lines):
    """Whether any of these instructions may change register r (a call changes eax, ecx and edx)."""
    low = {'eax': 'al', 'ebx': 'bl', 'ecx': 'cl', 'edx': 'dl'}.get(r)
    for x in lines:
        if x.startswith('call') and r in ('eax', 'ecx', 'edx'):
            return True
        if re.match(r'(mov|lea|xor|or|and|add|sub|inc|dec|neg|not|shl|shr|sar|imul|movzx|movsx|setne|sete|xchg) (' + r + (('|' + low) if low else '') + r')\b', x) or x == 'pop ' + r:
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
    while q < len(t) and re.fullmatch(r'mov ' + REG + r', dword ptr \[' + REG + r'\]|mov ecx, ' + REG + r'|mov ecx, dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]|mov (?:e[sd]i|ebp|ebx), dword ptr \[esp \+ (?:0x[0-9a-f]+|\d+)\]|call dword ptr \[e[a-d]x\]', t[q]):
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
        AT[0] = ins[k].address
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
            if not f or f[0] != 'new' or f[1] != len(made) - 1 or f[1] in field_entry or made[f[1]][1] is None or nulls or \
                    made[f[1]][3] is None and made[f[1]][1] == 0x8088c0:
                raise Reject('field add')
            if lst == ('L', 0x1c):
                parent = -1
            elif lst and lst[0] == 'list' and lst[1] in field_entry:
                parent = field_entry[lst[1]]
            else:
                raise Reject('field list %s' % (lst,))
            size, ctor, args, table = made[f[1]]
            field_entry[f[1]] = len(entries)
            entries.append((size, ctor, table or 0, args, parent))
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
            if strict and returns_this(num(m.group(1))):
                regs['eax'] = ('new', len(made) - 1)   # (a constructor that gives back the record)
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
        elif (m := re.fullmatch(r'je (0x[0-9a-f]+)', s)) and strict and tested and tested[0] == 'new' and tested[1] in field_entry and k + 8 < len(t):
            # the record added to a struct's list, or to the class's list when the struct is null, as two calls:
            #   je ELSE; lea ecx, [S + 4]; call add; mov X, [L kept]; jmp JOIN; ELSE: mov X, [L kept]; mov ecx, X; call add; JOIN:
            f = pushed[-1] if pushed else None
            # (or the list chosen first, the struct already in ecx: je ELSE; add ecx, 4; jmp JOIN; ELSE: mov ecx, L; JOIN: call add)
            ml = re.fullmatch(r'mov ecx, ' + REG, t[k + 3])
            if (regs.get('ecx') == tested and t[k + 1] == 'add ecx, 4' and t[k + 2].startswith('jmp ') and ml
                    and ins[k + 3].address == num(m.group(1)) and regs.get(ml.group(1)) == ('L', 0x1c) and ins[k + 4].address == num(t[k + 2][4:])
                    and t[k + 4] == 'call 0x8087e0' and f and f[0] == 'new' and f[1] == len(made) - 1 and f[1] not in field_entry
                    and (made[f[1]][3] is not None or made[f[1]][1] not in (None, 0x8088c0)) and not nulls):
                pushed.pop()
                depth -= 4
                size, ctor, args, table = made[f[1]]
                field_entry[f[1]] = len(entries)
                entries.append((size, ctor, table or 0, args, field_entry[tested[1]]))
                for r in ('eax', 'ecx', 'edx'):
                    regs.pop(r, None)
                k += 5
                continue
            # (or the list chosen first: je ELSE; mov ecx, S; add ecx, 4; jmp JOIN; ELSE: mov ecx, L; JOIN: call add)
            mr = re.fullmatch(r'mov ecx, ' + REG, t[k + 1])
            ml = re.fullmatch(r'mov ecx, ' + REG, t[k + 4])
            if (mr and regs.get(mr.group(1)) == tested and t[k + 2] == 'add ecx, 4' and t[k + 3].startswith('jmp ') and ml
                    and ins[k + 4].address == num(m.group(1)) and regs.get(ml.group(1)) == ('L', 0x1c) and ins[k + 5].address == num(t[k + 3][4:])
                    and t[k + 5] == 'call 0x8087e0' and f and f[0] == 'new' and f[1] == len(made) - 1 and f[1] not in field_entry
                    and (made[f[1]][3] is not None or made[f[1]][1] not in (None, 0x8088c0)) and not nulls):
                pushed.pop()
                depth -= 4
                size, ctor, args, table = made[f[1]]
                field_entry[f[1]] = len(entries)
                entries.append((size, ctor, table or 0, args, field_entry[tested[1]]))
                for r in ('eax', 'ecx', 'edx'):
                    regs.pop(r, None)
                k += 6
                continue
            # (or: je ELSE; mov X, S; lea ecx, [X + 4]; call add; jmp JOIN; ELSE: mov ecx, L; call add; mov X, [S kept]; JOIN:)
            mx = re.fullmatch(r'mov ' + REG + ', ' + REG, t[k + 1])
            ml = re.fullmatch(r'mov ecx, ' + REG, t[k + 5])
            mk = re.fullmatch(r'mov ' + REG + r', dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', t[k + 7]) if k + 8 < len(t) else None
            if (mx and regs.get(mx.group(2)) == tested and t[k + 2] == 'lea ecx, [%s + 4]' % mx.group(1) and t[k + 3] == 'call 0x8087e0'
                    and t[k + 4].startswith('jmp ') and ins[k + 5].address == num(m.group(1)) and ml and regs.get(ml.group(1)) == ('L', 0x1c)
                    and t[k + 6] == 'call 0x8087e0' and mk and mk.group(1) == mx.group(1) and ins[k + 8].address == num(t[k + 4][4:])
                    and f and f[0] == 'new' and f[1] == len(made) - 1 and f[1] not in field_entry and (made[f[1]][3] is not None or made[f[1]][1] not in (None, 0x8088c0)) and not nulls):
                pushed.pop()
                depth -= 4
                if stack.get(num(mk.group(2)) - depth) != tested:
                    raise Reject('field two adds kept')
                size, ctor, args, table = made[f[1]]
                field_entry[f[1]] = len(entries)
                entries.append((size, ctor, table or 0, args, field_entry[tested[1]]))
                for r in ('eax', 'ecx', 'edx'):
                    regs.pop(r, None)
                regs[mx.group(1)] = tested
                k += 8
                continue
            # (or the struct read back from where it was kept: je ELSE; mov ecx, [S kept]; add ecx, 4; call add; mov X, [L kept];
            # jmp JOIN; ELSE: mov X, [L kept]; mov ecx, X; call add; JOIN:)
            ms = re.fullmatch(r'mov ecx, dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', t[k + 1])
            m4 = re.fullmatch(r'mov ' + REG + r', dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', t[k + 4])
            m6 = re.fullmatch(r'mov ' + REG + r', dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', t[k + 6]) if k + 9 < len(t) else None
            if (ms and stack.get(num(ms.group(1)) - depth) == tested and t[k + 2] == 'add ecx, 4' and t[k + 3] == 'call 0x8087e0'
                    and m4 and t[k + 5].startswith('jmp ') and ins[k + 6].address == num(m.group(1)) and m6 and m6.group(1) == m4.group(1)
                    and m4.group(1) != 'ecx' and t[k + 7] == 'mov ecx, ' + m4.group(1) and t[k + 8] == 'call 0x8087e0'
                    and ins[k + 9].address == num(t[k + 5][4:])
                    and f and f[0] == 'new' and f[1] == len(made) - 1 and f[1] not in field_entry
                    and (made[f[1]][3] is not None or made[f[1]][1] not in (None, 0x8088c0)) and not nulls):
                pushed.pop()
                depth -= 4
                if stack.get(num(m4.group(2)) - depth) != ('L', 0x1c) or stack.get(num(m6.group(2)) - depth - 4) != ('L', 0x1c):
                    raise Reject('field two adds list kept')
                size, ctor, args, table = made[f[1]]
                field_entry[f[1]] = len(entries)
                entries.append((size, ctor, table or 0, args, field_entry[tested[1]]))
                for r in ('eax', 'ecx', 'edx'):
                    regs.pop(r, None)
                regs[m4.group(1)] = ('L', 0x1c)
                k += 9
                continue
            ma = re.fullmatch(r'lea ecx, \[' + REG + r' \+ 4\]', t[k + 1])
            mb = re.fullmatch(r'mov ' + REG + r', dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', t[k + 3])
            mc = re.fullmatch(r'mov ' + REG + r', dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', t[k + 5])
            if (not f or f[0] != 'new' or f[1] != len(made) - 1 or f[1] in field_entry or made[f[1]][3] is None and made[f[1]][1] in (None, 0x8088c0) or nulls
                    or not ma or regs.get(ma.group(1)) != tested or t[k + 2] != 'call 0x8087e0' or not mb or not mc
                    or mb.group(1) != mc.group(1) or not t[k + 4].startswith('jmp ') or ins[k + 5].address != num(m.group(1))
                    or t[k + 6] != 'mov ecx, ' + mb.group(1) or t[k + 7] != 'call 0x8087e0' or ins[k + 8].address != num(t[k + 4][4:])):
                raise Reject('field two adds')
            pushed.pop()
            depth -= 4
            if stack.get(num(mb.group(2)) - depth) != ('L', 0x1c) or stack.get(num(mc.group(2)) - depth - 4) != ('L', 0x1c):   # (on the null path the record is still pushed)
                raise Reject('field two adds list')
            size, ctor, args, table = made[f[1]]
            field_entry[f[1]] = len(entries)
            entries.append((size, ctor, table or 0, args, field_entry[tested[1]]))
            for r in ('eax', 'ecx', 'edx'):
                regs.pop(r, None)
            regs[mb.group(1)] = ('L', 0x1c)
            k += 8
            continue
        elif (m := re.fullmatch(r'je (0x[0-9a-f]+)', s)) and strict:
            # the record not made: its null path (checked at the jmp that ends the made path)
            if not tested or tested[0] != 'new' or tested[1] != len(made) - 1 or nulls:
                raise Reject('field je')
            nulls.append((num(m.group(1)), tested[1], dict(regs), dict(stack), depth))
        elif (m := re.fullmatch(r'jmp (0x[0-9a-f]+)', s)) and strict:
            # the null path sets the same places to null that the made path set to the record, then joins
            if not nulls or ins[k + 1].address != nulls[-1][0]:
                raise Reject('field jmp')
            join, i, regs0, stack0, depth0 = num(m.group(1)), *nulls.pop()[1:]
            j = k + 1
            while ins[j].address != join:
                x = t[j]
                # (judged by the registers and the stack as they were at the je: the null path's own)
                if (mm := re.fullmatch(r'xor ' + REG + ', ' + REG, x)) and mm.group(1) == mm.group(2) and (regs.get(mm.group(1)) == ('new', i) or regs0.get(mm.group(1)) == ('new', i)):
                    regs0 = dict(regs0); regs0[mm.group(1)] = ('const', 0)   # (followed on the null path)
                elif (mm := re.fullmatch(r'mov ' + REG + ', ' + REG, x)) and regs0.get(mm.group(2)) == ('const', 0) and regs.get(mm.group(1)) == ('new', i):
                    regs0 = dict(regs0); regs0[mm.group(1)] = ('const', 0)   # (the register the made path holds the record in, null here)
                elif (mm := re.fullmatch(r'mov dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\], ' + REG, x)) and regs0.get(mm.group(2)) == ('const', 0) and \
                        (stack.get(num(mm.group(1)) - depth) == ('new', i) or stack0.get(num(mm.group(1)) - depth0) == ('new', i)):
                    pass
                elif (mm := re.fullmatch(r'mov dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\], 0', x)) and stack.get(num(mm.group(1)) - depth) == ('new', i):
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


STATE = r'mov byte ptr \[esp \+ (?:0x[0-9a-f]+|\d+)\], (?:0x[0-9a-f]+|\d+|[a-d]l)'   # the exception frame's state


def named_text_step(t, ins, k, regs):
    """A member's wide text set from a named text, from t[k] (lea ecx, [esp + X], the member stage having taken the pushes
    of 1, the table and the name): the named text made in a temporary on the stack (0x4040F0); the member's old text let
    go; the member given a new wide text copied from the named one's characters (0xBCF7D0 on 16 new bytes, or none
    without memory), or the shared empty one when it has none (0xBD0490); the temporary let go (0xBD1880). Returns (the
    index after it, the member's offset, the registers it leaves changed, the plain stores into the object between the
    lea and the call, which the call can't see: done first), or None. The exception frame's state stores are passed over."""
    seq, at, stores = [], [], []
    q = k
    while q < len(t) and len(seq) < 35:
        m = re.fullmatch(r'mov (dword|byte) ptr \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\], (0x[0-9a-f]+|\d+)', t[q])
        if len(seq) == 1 and m and regs.get(m.group(2)) and regs[m.group(2)][0] == 'this':
            stores.append((0 if m.group(1) == 'dword' else 1, regs[m.group(2)][1] + num(m.group(3) or '0'), num(m.group(4)), 0, []))
        elif not re.fullmatch(STATE, t[q]):
            seq.append(t[q])
            at.append(q)
        q += 1
    if len(seq) < 35:
        return None
    zero = lambda x: x == '0' or regs.get(x) == ('const', 0)
    is_zero_test = lambda line, r: line == 'test %s, %s' % (r, r) or ((m := re.fullmatch(r'cmp ' + r + ', (0|' + REG + ')', line)) and zero(m.group(1)))
    mx = re.fullmatch(r'lea ecx, \[esp \+ (0x[0-9a-f]+|\d+)\]', seq[0])
    if not mx or seq[1] != 'call 0x4040f0':
        return None
    m1 = re.fullmatch(r'mov ' + REG + r', dword ptr \[eax \+ 8\]', seq[2])
    m3 = re.fullmatch(r'mov eax, dword ptr \[' + REG + r' \+ (0x[0-9a-f]+|\d+)\]', seq[3])
    if not m1 or not m3 or not regs.get(m3.group(1)) or regs[m3.group(1)][0] != 'this' or m1.group(1) in ('eax', 'ecx', 'edx'):
        return None
    r1, member = m1.group(1), seq[3].split(', ', 1)[1]
    m5 = re.fullmatch(r'mov ' + REG + ', ' + re.escape(member), seq[5])
    if seq[4] != 'dec dword ptr [eax]' or not m5:
        return None
    r2 = m5.group(1)
    m6 = re.fullmatch(r'cmp dword ptr \[' + r2 + r'\], (0|' + REG + ')', seq[6])
    if not m6 or not zero(m6.group(1)) or not seq[7].startswith('jne ') or not is_zero_test(seq[8], r2) or not seq[9].startswith('je '):
        return None
    if seq[10:16] != ['mov eax, dword ptr [%s + 0xc]' % r2, 'push eax', 'call 0xbce010', 'push ' + r2, 'call 0xbce000', 'add esp, 8']:
        return None
    if num(seq[7][4:]) != ins[at[16]].address or num(seq[9][3:]) != ins[at[16]].address:
        return None
    if not is_zero_test(seq[16], r1) or not seq[17].startswith('je ') or seq[18:21] != ['push 0x10', 'call 0xbcdfc0', 'add esp, 4']:
        return None
    if not re.fullmatch(r'mov dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\], eax', seq[21]) or not is_zero_test(seq[22], 'eax') or not seq[23].startswith('je '):
        return None
    if seq[24:27] != ['push ' + r1, 'mov ecx, eax', 'call 0xbcf7d0'] or not seq[27].startswith('jmp ') or seq[28] != 'xor eax, eax' or not seq[29].startswith('jmp '):
        return None
    if num(seq[23][3:]) != ins[at[28]].address or seq[30:32] != ['mov cl, 1', 'call 0xbd0490'] or num(seq[17][3:]) != ins[at[30]].address:
        return None
    if num(seq[27][4:]) != ins[at[32]].address or num(seq[29][4:]) != ins[at[32]].address:
        return None
    store = 'mov %s, eax' % member
    lea = 'lea ecx, [esp + %#x]' % (num(mx.group(1)) - 12)   # the temporary again, the three pushes gone
    if sorted(seq[32:34]) != sorted([lea, store]) or seq[34] != 'call 0xbd1880':
        return None
    off = regs[m3.group(1)][1] + num(re.search(r'\+ (0x[0-9a-f]+|\d+)\]', member).group(1))
    return at[34] + 1, off, (r1, r2), stores


def resource_step(t, ins, k, regs, pushed_already=False):
    """A member looked up by name in the game's resources, from t[k]: a text made from the name (0xBCFB10 on 16 new
    bytes, or none without memory), the resource manager (the global 0x13D0E04) asked for it (0xBDF160 or 0xBDF1A0, given
    the text's place), the answer stored in the member, the text let go. Returns (the index after it, the name, the
    lookup, the member's offset, the register it leaves changed), or None. The exception frame's state stores between
    are passed over. With pushed_already, t[k] is the allocation (the member stage has taken the push of 0x10, and
    whatever came between). The 29 instructions:
       0 push 0x10; 1 call alloc; 2 add esp, 4; 3 mov [esp + A], eax; 4 test/cmp eax, 0; 5 je NULL; 6 push NAME;
       7 mov ecx, eax; 8 call 0xBCFB10; 9 jmp J; 10 NULL: xor eax, eax; 11 J: mov [esp + T], eax;
       12-14 mov ecx, [0x13D0E04] / lea R, [esp + T] / push R (lea before push; ecx set after its use as R);
       15 call lookup; 16 mov [this + OFF], eax; 17 mov eax, [esp + T]; 18 dec [eax]; 19 mov eax, [esp + T];
       20 cmp [eax], 0; 21 jne END; 22 mov X, eax; 23 mov eax, [eax + 0xC]; 24 push eax; 25 call 0xBCE010; 26 push X;
       27 call 0xBCE000; 28 add esp, 8; END."""
    seq, at = (['push 0x10'], [k]) if pushed_already else ([], [])
    q = k
    while q < len(t) and len(seq) < 29:
        if not re.fullmatch(STATE, t[q]):
            seq.append(t[q])
            at.append(q)
        q += 1
    if len(seq) < 29 or seq[:3] != ['push 0x10', 'call 0xbcdfc0', 'add esp, 4']:
        return None
    zero = lambda r: regs.get(r) == ('const', 0)
    mz = re.fullmatch(r'cmp eax, ' + REG, seq[4])
    if (not re.fullmatch(r'mov dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\], eax', seq[3]) or not (seq[4] == 'test eax, eax' or (mz and zero(mz.group(1))))
            or not seq[5].startswith('je ')):
        return None
    mn = re.fullmatch(r'push (0x[0-9a-f]+)', seq[6])
    if not mn or seq[7:9] != ['mov ecx, eax', 'call 0xbcfb10'] or not seq[9].startswith('jmp ') or seq[10] != 'xor eax, eax':
        return None
    mt = re.fullmatch(r'mov dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\], eax', seq[11])
    if ins[at[10]].address != num(seq[5][3:]) or not mt or ins[at[11]].address != num(seq[9][4:]):
        return None
    slot = mt.group(1)
    three = seq[12:15]
    lea = next((x for x in three if re.fullmatch(r'lea ' + REG + r', \[esp \+ ' + slot + r'\]', x)), None)
    if not lea:
        return None
    r = lea.split()[1].rstrip(',')
    manager = 'mov ecx, dword ptr [0x13d0e04]'
    if sorted(three) != sorted([manager, lea, 'push ' + r]) or three.index(lea) > three.index('push ' + r):
        return None
    if r == 'ecx' and three.index(manager) < three.index('push ecx'):
        return None
    mf = re.fullmatch(r'call (0xbdf160|0xbdf1a0)', seq[15])
    ms = re.fullmatch(r'mov dword ptr \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\], eax', seq[16])
    if not mf or not ms or not regs.get(ms.group(1)) or regs[ms.group(1)][0] != 'this':
        return None
    load = 'mov eax, dword ptr [esp + %s]' % slot
    mc = re.fullmatch(r'cmp dword ptr \[eax\], (0|' + REG + r')', seq[20])
    mx = re.fullmatch(r'mov ' + REG + ', eax', seq[22])
    if seq[17:20] != [load, 'dec dword ptr [eax]', load] or not mc or not (mc.group(1) == '0' or zero(mc.group(1))):
        return None
    if not seq[21].startswith('jne ') or not mx or mx.group(1) in ('eax', 'ecx', 'edx'):
        return None
    x = mx.group(1)
    if seq[23:29] != ['mov eax, dword ptr [eax + 0xc]', 'push eax', 'call 0xbce010', 'push ' + x, 'call 0xbce000', 'add esp, 8']:
        return None
    end = at[28] + 1
    if end >= len(t) or ins[end].address != num(seq[21][4:]):
        return None
    return end, num(mn.group(1)), num(mf.group(1)), regs[ms.group(1)][1] + num(ms.group(2) or '0'), x


# (the same as titled_ctors.py's: an aligned frame's argument reads written as esp slots)
def ebp_frame_args(t, ins):
    """A function with an aligned frame (push ebp; mov ebp, esp; and esp, -N) reads its arguments as [ebp + K]: written
    here as the same slot through esp ([esp + K - 4 + the depth counted so far: pushes, subtractions and callees' own
    pops; the alignment is not counted, as the readers below don't count it either), so the readers that know arguments
    by their esp slot know these. Only while ebp is never written after the frame is made; else the text is left as it is."""
    if len(t) < 3 or t[0] != 'push ebp' or t[1] != 'mov ebp, esp' or not re.fullmatch(r'and esp, 0xfffffff[08]', t[2]):
        return t
    for q in range(3, len(t)):
        u = t[q]
        if re.match(r'(mov|lea|add|sub|xor|or|and|inc|dec|pop|xchg|imul|shl|shr|sar|movzx|movsx) ebp\b', u) and \
                not (u == 'pop ebp' and q + 1 < len(t) and ins[q + 1].mnemonic == 'ret'):
            return t
    out, d = list(t), 0
    for q, u in enumerate(t):
        if q >= 3:
            z = re.search(r'dword ptr \[ebp \+ (0x[0-9a-f]+|\d+)\]', u)
            if z and num(z.group(1)) >= 8:
                out[q] = u.replace(z.group(0), 'dword ptr [esp + %#x]' % (num(z.group(1)) - 4 + d))
        x = ins[q]
        if u.startswith('push '):
            d += 4
        elif u.startswith('pop '):
            d -= 4
        elif (m := re.fullmatch(r'sub esp, (0x[0-9a-f]+|\d+)', u)):
            d += num(m.group(1))
        elif (m := re.fullmatch(r'add esp, (0x[0-9a-f]+|\d+)', u)):
            d -= num(m.group(1))
        elif x.mnemonic == 'call' and re.fullmatch(r'0x[0-9a-f]+', x.op_str):
            d -= 4 * (ret_args(num(x.op_str)) or 0)
    return out


AT = [0]   # the address of the instruction being read, for the WHY listing


def parse(a, n):
    ins = list(md.disasm(bytes(img[a - BASE:a - BASE + n]), a))
    if ins and ins[-1].mnemonic != 'ret' and ins[-1].address + ins[-1].size == a + n:
        # the listed size can stop before the epilogue (the compiler put it just after): read on to the ret
        for x in md.disasm(bytes(img[a - BASE + n:a - BASE + n + 0x40]), a + n):
            ins.append(x)
            if x.mnemonic in ('ret', 'int3', 'jmp', 'call'):
                break
        n = ins[-1].address + ins[-1].size - a
    if not ins or ins[-1].address + ins[-1].size != a + n or ins[-1].mnemonic != 'ret' or ins[-1].op_str not in ('8', '0xc', '0x10', '0x14', '0x18', '0x1c'):
        raise Reject('not 2 to 7 arguments')
    nargs = num(ins[-1].op_str) // 4
    t = [x.mnemonic + ' ' + x.op_str for x in ins]
    t = ebp_frame_args(t, ins)
    framed = t[:2] == ['push ebp', 'mov ebp, esp'] and any('[esp + ' in u for u, v in zip(t, [x.mnemonic + ' ' + x.op_str for x in ins]) if u != v)
    # the base: the first call, taking (description, a, b): Class::Construct (0x800A80) or a parent class's constructor
    kb = [k for k, x in enumerate(t) if x.startswith('call ')]
    if not kb or not re.fullmatch(r'call 0x[0-9a-f]+', t[kb[0]]) or ret_args(num(t[kb[0]][5:])) not in (3, 4):
        raise Reject('base')
    kb = kb[0]
    base_fn = num(t[kb][5:])
    if base_fn != 0x800a80 and t.count(t[kb]) != 1:
        raise Reject('base called twice')
    # the start: registers and the stack up to the base call
    regs = {'ecx': ('this', 0)}
    depth = 0   # bytes pushed since entry
    pushed = []
    desc = None
    after_jne = False
    kept = {}   # stack locals (by their place below the return address) holding a known value: this, kept and read back
    for k in range(kb):
        s = t[k]
        if s == 'mov eax, dword ptr [0x10fe634]':
            regs['eax'] = ('cookie',)   # the compiler's stack check: kept in a local, checked at the end (0xDDCDA7)
            continue
        if framed and (k == 1 and s == 'mov ebp, esp' or k == 2 and re.fullmatch(r'and esp, 0xfffffff[08]', s)):
            continue   # (the aligned frame: its alignment is not counted, as the argument slots were written without it)
        if s.startswith(('push -1', 'push 0xe')) and k < 6 or 'fs:[0]' in s:
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
            regs[m.group(1)] = ('arg', slot // 4) if slot >= 4 and slot % 4 == 0 else kept.get(slot)
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
        elif (m := re.fullmatch(r'mov (dword|byte) ptr \[esp(?: \+ (0x[0-9a-f]+|\d+))?\], (.*)', s)):
            # locals and the exception frame's state; this kept in a local is remembered
            slot = num(m.group(2) or '0') - depth
            kept.pop(slot, None)
            if m.group(1) == 'dword' and slot < 0 and regs.get(m.group(3)) == ('this', 0):
                kept[slot] = ('this', 0)
        else:
            raise Reject('start: ' + s)
        after_jne = ins[k].mnemonic == 'jne'
    base_n = ret_args(base_fn)   # (3; or 4, the fourth a constant)
    base = [p for p in pushed[-base_n:]][::-1]
    base_x = 0
    if base_n == 4:
        if len(base) != 4 or not base[3] or base[3][0] not in ('const', 'arg') or base[3][0] == 'arg' and not 4 <= base[3][1] <= nargs:
            raise Reject('base fourth value %s' % (base[3:],))
        base_x = base[3][1] if base[3][0] == 'const' else 0x80000000 | base[3][1]   # (bit 31: the constructor's argument n)
        base = base[:3]
    if desc is None or len(pushed) < base_n or base not in ([('desc',), ('arg', 2), ('arg', 3)], [('desc',), ('arg2at', 0), ('arg2at', 4)]):
        raise Reject('base arguments %s' % pushed[-3:])
    pair = base[1][0] == 'arg2at'
    if regs.get('ecx') != ('this', 0):
        raise Reject('base this')
    # the members: up to the description's first-use check
    for r in ('eax', 'ecx', 'edx'):
        regs.pop(r, None)
    depth -= 4 * len(pushed[-base_n:])
    pushed = []
    ops = []
    pending = None   # a member made with new, until it is stored
    left_on_stack = [0]   # cdecl pushes not yet taken off
    def reference_setter(k0):
        # lea ecx, [esp + X]; call 0xBD9AD0 (a reference made there; constant stores into the object may come between);
        # push F; push eax; push NAME; lea ecx, [this + off]; call FN (three values); lea ecx, [esp + X]; call 0xBD99E0
        # (the exception state's stores passed over). Gives (index after, the stores, off, NAME, F, FN) or None.
        z = re.fullmatch(r'lea ecx, \[esp \+ (0x[0-9a-f]+|\d+)\]', t[k0])
        if not z:
            return None
        slot = num(z.group(1)) - depth
        eh = lambda u: re.fullmatch(r'mov (byte|dword) ptr \[esp \+ (0x[0-9a-f]+|\d+)\], (0x[0-9a-f]+|\d+|[a-d]l)', u)
        q, stores = k0 + 1, []
        while q < len(t) and t[q] != 'call 0xbd9ad0':
            u = t[q]
            if eh(u):
                q += 1; continue
            mm = re.fullmatch(r'mov dword ptr \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\], (' + REG + r'|0x[0-9a-f]+|\d+)', u)
            if not mm or not regs.get(mm.group(1)) or regs[mm.group(1)][0] != 'this' or mm.group(1) == 'ecx':
                return None
            v = mm.group(3)
            if re.fullmatch(r'0x[0-9a-f]+|\d+', v): val_ = num(v)
            elif regs.get(v) and regs[v][0] == 'const': val_ = regs[v][1]
            else: return None
            stores.append((0, regs[mm.group(1)][1] + num(mm.group(2) or '0'), val_, 0, []))
            q += 1
        if q >= len(t):
            return None
        seq = []
        q += 1
        while q < len(t) and len(seq) < 8:
            if not eh(t[q]):
                seq.append((q, t[q]))
            q += 1
        b = [u for _, u in seq]
        zf = re.fullmatch(r'push (0x[0-9a-f]+|\d+)', b[0])
        zn = re.fullmatch(r'push (0x[0-9a-f]+)', b[2])
        zm = re.fullmatch(r'lea ecx, \[' + REG + r' \+ (0x[0-9a-f]+|\d+)\]', b[3])
        zc = re.fullmatch(r'call (0x[0-9a-f]+)', b[4])
        zl = re.fullmatch(r'lea ecx, \[esp \+ (0x[0-9a-f]+|\d+)\]', b[5])
        if not (zf and b[1] == 'push eax' and zn and zm and zc and zl and b[6] == 'call 0xbd99e0'):
            return None
        if not regs.get(zm.group(1)) or regs[zm.group(1)][0] != 'this' or ret_args(num(zc.group(1))) != 3:
            return None
        if num(zl.group(1)) - depth != slot:
            return None
        return seq[6][0] + 1, stores, regs[zm.group(1)][1] + num(zm.group(2)), num(zn.group(1)), num(zf.group(1)), num(zc.group(1))
    def result_store(k0):
        # the next store of eax into the object (mov dword ptr [this + off], eax), with nothing before it touching eax or
        # calling: (its index, the offset)
        for q_ in range(k0 + 1, min(k0 + 8, len(t))):
            u = t[q_]
            if (mm := re.fullmatch(r'mov dword ptr \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\], eax', u)) and regs.get(mm.group(1)) and regs[mm.group(1)][0] == 'this':
                return q_, regs[mm.group(1)][1] + num(mm.group(2) or '0')
            if u.startswith(('call', 'j', 'ret')) or re.search(r'\b(eax|ax|al|ah)\b', u):
                return None
        return None
    def zero(r):
        return regs.get(r) == ('const', 0)

    def member_at(x):   # [B + o] with B a pointer into the object: the member's offset
        mm = re.fullmatch(r'\[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\]', x)
        if mm and regs.get(mm.group(1)) and regs[mm.group(1)][0] == 'this':
            return regs[mm.group(1)][1] + num(mm.group(2) or '0')
        return None

    def skip_state(j):   # (the exception frame's state bytes between: nothing else reads them here)
        while j < len(t) and re.fullmatch(STATE, t[j]):
            j += 1
        return j

    def single(k):
        mm = re.fullmatch(r'cmp dword ptr \[(0x1[0-9a-f]{6})\], ' + REG, t[k])
        if not mm or not zero(mm.group(2)) or k + 14 >= len(t):
            return None
        j = skip_state(k + 1)
        if (mj := re.fullmatch(r'mov dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\], ' + REG, t[j])) and zero(mj.group(2)) and \
                num(mj.group(1)) - depth - 4 * len(pushed) < 0:
            j = skip_state(j + 1)   # (a local cleared between: the exception frame's)
        mh = re.fullmatch(r'jne (0x[0-9a-f]+)', t[j])
        ms = re.fullmatch(r'push (0x[0-9a-f]+|\d+)', t[j + 1])
        ml = re.fullmatch(r'mov dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\], eax', t[j + 4])
        if not mh or not ms or t[j + 2] != 'call 0xbcdfc0' or t[j + 3] != 'add esp, 4' or not ml:
            return None
        q = skip_state(j + 5)
        mc = re.fullmatch(r'cmp eax, ' + REG, t[q])
        if not mc or not zero(mc.group(1)):
            return None
        q = skip_state(q + 1)
        mn = re.fullmatch(r'je (0x[0-9a-f]+)', t[q])
        mv = re.fullmatch(r'push (' + REG[1:-1] + r'|0x[0-9a-f]+|\d+)', t[q + 1])
        mf = re.fullmatch(r'call (0x[0-9a-f]+)', t[q + 3])
        if not mn or not mv or t[q + 2] != 'mov ecx, eax' or not mf or ins[q + 4].address != num(mn.group(1)):
            return None
        v = mv.group(1)
        if re.fullmatch(r'0x[0-9a-f]+|\d+', v):
            v = num(v)
        elif regs.get(v) and regs[v][0] == 'const':
            v = regs[v][1]
        else:
            return None
        e = skip_state(q + 4)
        if e >= len(t) or ins[e].address != num(mh.group(1)) or ret_args(num(mf.group(1))) != 1:
            return None
        return e, num(mm.group(1)), num(ms.group(1)), num(mf.group(1)), v, num(ml.group(1)) - depth

    def released(k):
        mm = re.fullmatch(r'mov ' + REG + r', dword ptr (\[.*\])', t[k])
        if not mm or (off := member_at(mm.group(2))) is None or k + 8 >= len(t):
            return None
        r = mm.group(1)
        j = skip_state(k + 1)
        mc = re.fullmatch(r'(?:cmp ' + r + r', ' + REG + r'|test ' + r + ', ' + r + ')', t[j])
        if not mc or (mc.group(1) and not zero(mc.group(1))):
            return None
        j = skip_state(j + 1)
        mj = re.fullmatch(r'je (0x[0-9a-f]+)', t[j])
        mf = re.fullmatch(r'call (0x[0-9a-f]+)', t[j + 2])
        ms = re.fullmatch(r'mov dword ptr (\[.*\]), (.+)', t[j + 4])
        if not mj or t[j + 1] != 'push ' + r or not mf or num(mf.group(1)) not in (0xbce010, 0xbce000) or t[j + 3] != 'add esp, 4' or \
                not ms or member_at(ms.group(1)) != off or not (ms.group(2) == '0' or zero(ms.group(2))) or j + 5 >= len(t) or \
                ins[j + 5].address != num(mj.group(1)):
            return None
        return j + 5, off, num(mf.group(1)), r

    def cleared(k):
        mm = re.fullmatch(r'cmp ' + REG + ', ' + REG, t[k])
        if not mm or not zero(mm.group(2)) or not regs.get(mm.group(1)) or regs[mm.group(1)][0] != 'field' or regs[mm.group(1)][2] != len(ops) or \
                k + 8 >= len(t):
            return None
        count = regs[mm.group(1)][1]
        mj = re.fullmatch(r'jle (0x[0-9a-f]+)', t[k + 1])
        if not mj:
            return None
        j = k + 2
        while (mp := re.fullmatch(r'lea ' + REG + r', \[' + REG + r'\]', t[j])) and mp.group(1) == mp.group(2):
            j += 1   # (padding)
        if j + 6 >= len(t):
            return None
        m1 = re.fullmatch(r'mov ' + REG + r', dword ptr (\[.*\])', t[j])
        m2 = re.fullmatch(r'mov dword ptr \[' + REG + r' \+ ' + REG + r'\*4\], (.+)', t[j + 1])
        m3 = re.fullmatch(r'mov ' + REG + r', dword ptr (\[.*\])', t[j + 2])
        if not m1 or member_at(m1.group(2)) != count - 4 or not m2 or m2.group(1) != m1.group(1) or not zero(m2.group(2)) or \
                not m3 or member_at(m3.group(2)) != count or t[j + 3] != 'inc ' + m2.group(2) or t[j + 4] != 'cmp %s, %s' % (m2.group(2), m3.group(1)) or \
                t[j + 5] != 'jl %#x' % ins[j].address or ins[j + 6].address != num(mj.group(1)):
            return None
        v = m2.group(3)
        if re.fullmatch(r'0x[0-9a-f]+|\d+', v):
            value = num(v)
        elif regs.get(v) and regs[v][0] == 'const':
            value = regs[v][1]
        else:
            return None
        return j + 6, count - 4, value, [mm.group(1), m1.group(1), m2.group(2), m3.group(1)]

    dirty = set()    # argument slots the constructor wrote over (its locals reuse them): no longer the argument
    k = kb + 1
    filled = set()   # stores that fill a value passed by value on the stack (taken with its push)
    while k < len(t):
        s = t[k]
        AT[0] = ins[k].address
        if (mneg := re.search(r'\[' + REG + r' - (0x[0-9a-f]+|\d+)\]', s)) and regs.get(mneg.group(1)) and regs[mneg.group(1)][0] == 'this' and \
                regs[mneg.group(1)][1] >= num(mneg.group(2)):
            # (below a pointer into the object, still inside it: read as the same place reached from this)
            regs['__below'] = ('this', regs[mneg.group(1)][1] - num(mneg.group(2)))
            s = s[:mneg.start()] + '[__below]' + s[mneg.end():]
        if k in filled:
            k += 1
            continue
        # a 4-byte value passed by value: push R (room for it); mov X, esp; ... mov dword ptr [X], value; before the call
        if (m := re.fullmatch(r'push ' + REG, s)) and not (regs.get(m.group(1)) and regs[m.group(1)][0] in ('const', 'global', 'this')) \
                and (mx := re.fullmatch(r'mov ' + REG + ', esp', t[k + 1])):
            x, j = mx.group(1), k + 2
            while j < len(t) and not t[j].startswith(('call', 'push', 'j')):
                if (mv := re.fullmatch(r'mov dword ptr \[' + x + r'\], (0x[0-9a-f]+|\d+)', t[j])):
                    break
                if written(x, [t[j]]):
                    j = len(t)
                    break
                j += 1
            if j < len(t) and mv:
                pushed.append(('imm', num(mv.group(1))))
                filled.add(j)
                regs.pop(x, None)
                k += 2
                continue
        if re.fullmatch(r'(mov [a-d]l, byte ptr|cmp byte ptr) \[' + REG + r' \+ 4\].*', s):
            break
        if not pushed and not pending and (rs := reference_setter(k)):
            # a reference made on the stack, given with a name and a value to a member's function, let go (kind 25)
            k, stores, off_, name_, flag_, fn_ = rs
            ops.extend(stores)
            ops.append((25, off_, name_, fn_, [('imm', flag_)]))
            ref_setters.add(fn_)
            for r in ('eax', 'ecx', 'edx'):
                regs.pop(r, None)
            continue
        if (len(pushed) == 3 and pushed[0] == ('imm', 1) and pushed[1][0] == 'imm' and pushed[2][0] == 'imm' and not pending
                and re.fullmatch(r'lea ecx, \[esp \+ (0x[0-9a-f]+|\d+)\]', s) and (nt := named_text_step(t, ins, k, regs))):
            ops.extend(nt[3])
            ops.append((21, nt[1], pushed[2][1], pushed[1][1], []))   # a member's wide text set from a named text
            for r in ('eax', 'ecx', 'edx') + nt[2]:
                regs.pop(r, None)
            pushed = []
            k = nt[0]
            continue
        if s == 'call 0xbcdfc0' and pushed == [('imm', 0x10)] and not pending and (rs := resource_step(t, ins, k, regs, True)):
            ops.append((20, rs[3], rs[1], rs[2], []))   # a member looked up by name in the game's resources
            for r in ('eax', 'ecx', 'edx', rs[4]):
                regs.pop(r, None)
            pushed = []
            k = rs[0]
            continue
        hand = next((h for h in HAND_STEPS.get(a, []) if h[0] == s), None)
        if hand:
            # a stretch written by hand (kind 17): skipped here, the registers it writes forgotten
            if pushed or pending:
                raise Reject('hand stretch with values pushed')
            j = next((q for q in range(k, len(t)) if t[q] == hand[1]), None)
            if j is None:
                raise Reject('hand stretch end not found')
            # followed for the stack (it must end where it began) and for pointers into the object it leaves in
            # registers and locals; any other register it writes is forgotten
            d = 0
            for x in t[k:j + 1]:
                if x.startswith('push '):
                    d += 4
                elif (mm := re.fullmatch(r'add esp, (0x[0-9a-f]+|\d+)', x)):
                    d -= num(mm.group(1))
                elif (mm := re.fullmatch(r'call (0x[0-9a-f]+)', x)):
                    d -= 4 * ret_args(num(mm.group(1)))
                    for r in ('eax', 'ecx', 'edx'):
                        regs.pop(r, None)
                elif (mm := re.fullmatch(r'lea ' + REG + r', \[' + REG + r' \+ (0x[0-9a-f]+|\d+)\]', x)) and regs.get(mm.group(2)) and regs[mm.group(2)][0] == 'this':
                    regs[mm.group(1)] = ('this', regs[mm.group(2)][1] + num(mm.group(3)))
                elif (mm := re.fullmatch(r'mov dword ptr \[esp(?: \+ (0x[0-9a-f]+|\d+))?\], (.+)', x)):
                    slot = num(mm.group(1) or '0') - depth - d
                    kept.pop(slot, None)
                    if slot >= 4:
                        dirty.add(slot // 4)
                    if regs.get(mm.group(2)) and regs[mm.group(2)][0] == 'this':
                        kept[slot] = regs[mm.group(2)]
                elif x.startswith(('mov byte ptr [esp', 'mov dword ptr [e', 'mov byte ptr [e', 'fstp ', 'fld ', 'fadd ', 'cmp ', 'test ')):
                    pass
                else:
                    for r in ('eax', 'ebx', 'ecx', 'edx', 'esi', 'edi', 'ebp'):
                        if written(r, [x]):
                            regs.pop(r, None)
            if d:
                raise Reject('hand stretch moves the stack')
            if hand[2] not in HANDS:
                HANDS.append(hand[2])
            ops.append((17, 0, HANDS.index(hand[2]), 0, []))
            k = j + 1
            continue
        if s == 'mov cl, 1':
            regs['ecx'] = ('cl1',)
        elif s in ('call 0xbd0490', 'call 0xbd0520') and regs.get('ecx') == ('cl1',):
            for r in ('ecx', 'edx'):
                regs.pop(r, None)
            regs['eax'] = ('empty', 0 if s == 'call 0xbd0490' else 0xbd0520)
        elif s == 'call 0x805750' or (s == 'call 0x805700' and regs.get('ecx') and regs['ecx'][0] in ('const', 'arg', 'this')):
            # the reference member's handle, linked in: mov [M + 4], eax; mov eax, [eax + 0x20]; then the link at M + 8.
            # 0x805750 gives the empty handle, 0x805700 (Handle::For) the handle of the object in ecx (a plain value)
            target = None if s == 'call 0x805750' else regs['ecx'][1]
            of_arg = s == 'call 0x805700' and regs['ecx'][0] == 'arg'   # the handle of the object an argument gives
            of_self = s == 'call 0x805700' and regs['ecx'][0] == 'this'   # the handle of the object itself (or a member)
            if (mk := re.fullmatch(r'mov (e[a-d]x|e[sd]i|ebp|ebx), dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', t[k + 1])) and mk.group(1) != 'eax' and \
                    num(mk.group(2)) - depth - 4 * len(pushed) in kept:
                # (the link's place read back from the local it was kept in, first)
                regs[mk.group(1)] = kept[num(mk.group(2)) - depth - 4 * len(pushed)]
                del t[k + 1], ins[k + 1]
            m = re.fullmatch(r'mov dword ptr \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\], eax', t[k + 1])
            if not m or not regs.get(m.group(1)) or regs[m.group(1)][0] != 'this':
                raise Reject('reference member')
            member = regs[m.group(1)][1] + num(m.group(2) or '0') - 4   # (stored through the member's pointer or through this)
            lines, extra = t[k + 2:k + 9], 0
            # (the link's place may be worked out inside: ...; mov edx, [eax]; lea L, [M + 8]; mov [L], edx; ...)
            ml_ = re.fullmatch(r'lea ' + REG + r', \[' + REG + r' \+ 8\]', t[k + 4]) if k + 10 <= len(t) else None
            if ml_ and regs.get(ml_.group(2)) == ('this', member) and not any(re.search(r'\b' + ml_.group(1) + r'\b', x) for x in t[k + 2:k + 4]):
                lines, extra = t[k + 2:k + 4] + t[k + 5:k + 10], 1
                regs[ml_.group(1)] = ('this', member + 8)
            body = [re.sub(REG, 'R', x) for x in lines]
            if body != ['mov R, dword ptr [R + 0x20]', 'mov R, dword ptr [R]', 'mov dword ptr [R], R', 'mov R, dword ptr [R]', 'mov dword ptr [R + 4], R',
                        'mov dword ptr [R + 4], R', 'mov dword ptr [R], R']:
                raise Reject('reference link')
            link = re.fullmatch(r'mov dword ptr \[' + REG + r'\], ' + REG, lines[2]).group(1)
            if regs.get(link) != ('this', member + 8):
                raise Reject('reference link register')
            ops.append((6, member, 0, 0, []) if target is None else (16 if of_arg else 19 if of_self else 9, member, target, 0, []))
            for r in ('eax', 'ecx', 'edx'):
                regs.pop(r, None)
            k += 9 + extra
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
            elif v == 'eax' and regs.get('eax') and regs['eax'][0] == 'empty' and m.group(1) == 'dword':
                ops.append((7, off, 0, regs['eax'][1], []))
            elif v == 'eax' and regs.get('eax') == ('made',) and m.group(1) == 'dword' and pending and pending[3] == len(ops):
                if pending[4] is None:
                    ops.append((10, off, pending[0], pending[1], pending[2]))   # nothing else happened since the new: same order
                else:
                    ops.append((15, off, pending[0], pending[1], pending[2] + [('imm', pending[4])]))
                pending = None
                regs.pop('eax', None)
            elif v in regs and regs[v] and regs[v][0] == 'global' and m.group(1) == 'dword':
                ops.append((5, off, regs[v][1], 0, []))
            elif v in regs and regs[v] and regs[v][0] == 'field' and m.group(1) == 'dword' and all(
                    o[0] in (0, 1, 2, 5, 8, 12, 18) and (o[1] + (1 if o[0] in (1, 12) else 4) <= regs[v][1] or o[1] >= regs[v][1] + 4) for o in ops[regs[v][2]:]):
                ops.append((18, off, regs[v][1], 0, []))   # a member's value (unchanged since it was read) copied
            else:
                raise Reject('member store: ' + s)
        elif not pushed and (sg_ := single(k)):
            # a game object made if its global holds none (kind 30): cmp [G], 0; jne HAVE; push size; call new; add esp, 4;
            # mov [local], eax; cmp eax, 0; je NONE; push value; mov ecx, eax; call ctor; NONE: HAVE: (its constructor sets G)
            k, g_, size_, fn_, v_, slot_ = sg_
            if slot_ >= 4:
                dirty.add(slot_ // 4)   # (the new memory kept in an argument's slot: no longer the argument)
            kept.pop(slot_, None)
            ops.append((30, size_, g_, fn_, [('imm', v_)]))
            singles[a] = (g_, fn_)
            for r in ('eax', 'ecx', 'edx'):
                regs.pop(r, None)
            continue
        elif not pushed and (rel_ := released(k)):
            # a member's memory let go if there is any (kind 28): mov R, [M]; cmp R, 0; je OVER; push R; call free; add esp, 4;
            # mov [M], 0; OVER:
            k, off_, fn_, r_ = rel_
            ops.append((28, off_, 0, fn_, []))
            for r in ('eax', 'ecx', 'edx', r_):
                regs.pop(r, None)
            continue
        elif not pushed and (clr_ := cleared(k)):
            # an array member {items, count} cleared up to its count, both read again on each pass (kind 29)
            k, off_, v_, regs_ = clr_
            ops.append((29, off_, v_, 0, []))
            for r in regs_:
                regs.pop(r, None)
            continue
        elif (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\]', s)) and regs.get(m.group(2)) and regs[m.group(2)][0] == 'this':
            regs[m.group(1)] = ('field', regs[m.group(2)][1] + num(m.group(3) or '0'), len(ops))   # a member's value, as it is now
        elif (m := re.fullmatch(r'mov dword ptr \[' + REG + r' \+ ' + REG + r'\*4\], (0x[0-9a-f]+|\d+)', s)):
            array, count = regs.get(m.group(1)), regs.get(m.group(2))
            if not array or not count or array[0] != 'field' or count[0] != 'field' or count[1] != array[1] + 4 or array[2] != len(ops) or count[2] != len(ops):
                raise Reject('member array end')
            ops.append((13, array[1], num(m.group(3)), 0, []))
        elif (m := re.fullmatch(r'jmp (0x[0-9a-f]+)', s)) and 0 < num(m.group(1)) - ins[k].address < 16:
            pass   # the description chosen for the registration (the one given, or the class's own)
        elif (m := re.fullmatch(r'mov (dword|byte) ptr \[esp(?: \+ (0x[0-9a-f]+|\d+))?\], (.*)', s)):
            slot = num(m.group(2) or '0') - depth - 4 * len(pushed)
            if slot >= 4 and not (slot == 4 and m.group(1) == 'dword' and regs.get(m.group(3)) == ('desc',)):
                dirty.add(slot // 4)   # (the class's own description put in the description's slot: the registration's choice, made early)
            kept.pop(slot, None)
            if m.group(1) == 'dword' and regs.get(m.group(3)) and regs[m.group(3)][0] == 'this':
                kept[slot] = regs[m.group(3)]   # a pointer into the object kept in a local
        elif (m := re.fullmatch(r'add ' + REG + r', (0x[0-9a-f]+|\d+)', s)) and regs.get(m.group(1)) and regs[m.group(1)][0] == 'this':
            regs[m.group(1)] = ('this', regs[m.group(1)][1] + num(m.group(2)))   # a pointer into the object moved on
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
            if slot in kept:
                regs[m.group(1)] = kept[slot]   # a pointer into the object read back from its local
            elif slot < 4 or slot % 4 or slot // 4 > nargs or slot // 4 in dirty:   # slot 4: the description as given (registration reloads it)
                raise Reject('member load ' + s)
            else:
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
        elif (m := re.fullmatch(r'push ' + REG, s)) and regs.get(m.group(1)) and regs[m.group(1)][0] == 'gfield':
            pushed.append(('gobj', regs[m.group(1)][1] | regs[m.group(1)][2] << 25))   # (read as the call is made: nothing between)
        elif (m := re.fullmatch(r'push ' + REG, s)) and regs.get(m.group(1)) and regs[m.group(1)][0] == 'this':
            pushed.append(('self', regs[m.group(1)][1]))   # a pointer into the object itself
        elif s == 'call 0xbcdfc0':
            # a member made with new: size bytes (0xBCDFC0), the class's constructor run on them with plain values (or
            # null when there was no memory); the result is stored into the member later (kind 10, see the store)
            if len(pushed) != 1 or pushed[0][0] != 'imm' or pending or t[k + 1] != 'add esp, 4':
                raise Reject('member new')
            size, pushed, j = pushed[0][1], [], k + 2
            saved_at = None   # the local the new memory is kept in
            while not t[j].startswith('je '):
                if (m := re.fullmatch(r'mov dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\], eax', t[j])):
                    saved_at = num(m.group(1))
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
            table = None
            # then given a table of its own: the memory read back from its local, its first word set (the member then
            # gets the memory, not what the constructor gave back)
            if (cm and saved_at is not None and t[j + 2] == 'mov eax, dword ptr [esp + %#x]' % saved_at
                    and (m := re.fullmatch(r'mov dword ptr \[eax\], (0x[0-9a-f]+)', t[j + 3]))):
                table = num(m.group(1))
                del t[j + 2:j + 4], ins[j + 2:j + 4]
            if not cm or len(args) > 4 or ret_args(num(cm.group(1))) != len(args) or not t[j + 2].startswith('jmp ') or ins[j + 3].address != null_at or t[j + 3] != 'xor eax, eax' or ins[j + 4].address != num(t[j + 2][4:]):
                raise Reject('member new shape')
            pending = (size, num(cm.group(1)), args[::-1], len(ops), table)
            for r in ('ecx', 'edx'):
                regs.pop(r, None)
            regs['eax'] = ('made',)
            k = j + 4
            continue
        elif (m := re.fullmatch(r'call (0x[0-9a-f]+)', s)) and (s == 'call 0xbcdfd0' and len(pushed) == 1 and pushed[0][0] == 'imm' or
                                                                 not pushed and ret_args(num(m.group(1))) == 0 and leaves_ecx(num(m.group(1))) and
                                                                 (regs.get('ecx') or ('',))[0] != 'this') and \
                (st := result_store(k)):
            # a member set to what a call gives: aligned memory (0xBCDFD0, size bytes; kind 24) or a function that takes
            # nothing and reads neither ecx nor edx first (kind 23); the store taken out of the steps
            q_, off_ = st
            if s == 'call 0xbcdfd0':
                ops.append((24, off_, pushed[0][1], 0xbcdfd0, []))
                left_on_stack[0] += 1   # (cdecl: its push stays until an add esp)
                pushed = []
            else:
                ops.append((23, off_, 0, num(m.group(1)), []))
                value_calls.add(num(m.group(1)))
            del t[q_], ins[q_]
            for r in ('eax', 'ecx', 'edx'):
                regs.pop(r, None)
        elif (m := re.fullmatch(r'add esp, (0x[0-9a-f]+|\d+)', s)) and not pushed and num(m.group(1)) // 4 <= left_on_stack[0] and num(m.group(1)) % 4 == 0:
            left_on_stack[0] -= num(m.group(1)) // 4   # (the cdecl pushes of earlier calls taken off)
        elif s == 'call 0x475230' and len(pushed) == 2 and all(p_ and p_[0] == 'imm' for p_ in pushed) and k + 1 < len(t) and \
                (mf_ := re.fullmatch(r'fstp dword ptr \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\]', t[k + 1])) and \
                regs.get(mf_.group(1)) and regs[mf_.group(1)][0] == 'this':
            # a value between two floats (0x475230: the first, or a random point up to the second) stored in a member (kind 27)
            ops.append((27, regs[mf_.group(1)][1] + num(mf_.group(2) or '0'), 0, 0x475230, pushed[::-1]))
            pushed = []
            del t[k + 1], ins[k + 1]
            for r in ('eax', 'ecx', 'edx'):
                regs.pop(r, None)
        elif s == 'call 0x475180' and not pushed:
            # Random::Ran0's value (on the FPU), perhaps times a game constant, stored in a member as a float (kind 22): the
            # fmul and the fstp taken out of the steps (nothing between them uses the FPU or calls)
            j = k + 1
            by = 0
            if (m := re.fullmatch(r'fmul dword ptr \[(0x[0-9a-f]+)\]', t[j])):
                by = num(m.group(1))
                del t[j], ins[j]
            q = j
            while q < len(t) and not t[q].startswith('fstp'):
                if t[q].startswith(('f', 'call')) or ins[q].mnemonic.startswith('j'):
                    raise Reject('random value used otherwise')
                q += 1
            m = re.fullmatch(r'fstp dword ptr \[' + REG + r'(?: \+ (0x[0-9a-f]+|\d+))?\]', t[q]) if q < len(t) else None
            if not m or not regs.get(m.group(1)) or regs[m.group(1)][0] != 'this' or written(m.group(1), t[j:q]):
                raise Reject('random value stored where')
            ops.append((22, regs[m.group(1)][1] + num(m.group(2) or '0'), by, 0x475180, []))
            del t[q], ins[q]
            for r in ('eax', 'ecx', 'edx'):
                regs.pop(r, None)
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
        elif re.fullmatch(r'(test|cmp) ' + REG + ', ' + REG, s) or ins[k].mnemonic == 'jne':
            pass
        elif (m := re.fullmatch(r'cmp dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\], ' + REG, s)) and num(m.group(1)) - depth - 4 * len(pushed) == 4 and regs.get(m.group(2)) == ('const', 0):
            pass   # was a description given? (the registration's choice, made early)
        elif (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[(0x[0-9a-f]{7})\]', s)) and num(m.group(2)) == desc:
            regs[m.group(1)] = ('desc',)
        elif (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[(0x[0-9a-f]{7})\]', s)):
            regs[m.group(1)] = ('global', num(m.group(2)))
        elif (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[' + REG + r' \+ (0x[0-9a-f]+|\d+)\]', s)) and regs.get(m.group(2)) and \
                regs[m.group(2)][0] == 'global' and regs[m.group(2)][1] < 1 << 25 and num(m.group(3)) < 0x80:
            regs[m.group(1)] = ('gfield', regs[m.group(2)][1], num(m.group(3)))   # a dword inside a global's object, read now
        elif ins[k].mnemonic == 'pop' and k + 1 < len(t):
            pass   # a register given back early
        elif (m := re.fullmatch(r'lea ' + REG + r', \[' + REG + r'\]', s)) and m.group(1) == m.group(2):
            pass   # padding before a loop: does nothing
        elif (m := re.fullmatch(r'dec ' + REG, s)) and regs.get(m.group(1)) and regs[m.group(1)][0] == 'const' and \
                k + 1 < len(t) and (mj := re.fullmatch(r'jne (0x[0-9a-f]+)', t[k + 1])) and num(mj.group(1)) < ins[k].address:
            # a loop counted by a constant: read again for each pass (the same steps, in the same order)
            n_ = regs[m.group(1)][1] - 1
            regs[m.group(1)] = ('const', n_)
            if n_ < 0 or n_ > 256:
                raise Reject('member loop count')
            if n_ == 0:
                k += 2
                continue
            k = next(q_ for q_ in range(len(ins)) if ins[q_].address == num(mj.group(1)))
            continue
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
    if len(steps) > 12:
        raise Reject('name steps')
    for q in steps:
        # (the description may be read back for the give before the push: the chosen one, as the registration's)
        reread = bool(re.fullmatch(r'mov (?:ecx|esi|edi|ebx|ebp), dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', rest[q + 1])) and rest[q + 2] == 'push eax'
        if (rest[q + 1:q + 2] != ['push eax'] and not reread) or not any(x == 'call 0xbecc10' for x in rest[q:q + 6]) or not any(x == 'call 0xbec5e0' for x in rest[q:q + 10]):
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
            # the pointer into the object may be read back from the local it was kept in
            if (mm := re.fullmatch(r'mov ' + r + r', dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', rest[q - 5])) and num(mm.group(1)) - depth in kept and regs.get(base) == ('this', 0) and not written(base, rest[:q - 4]):
                names.append((num(nm.group(1)), kept[num(mm.group(1)) - depth][1]))
                continue
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

    for q in range(k, last + 1):   # a local holding this must not be written by the registration
        if (m := re.fullmatch(r'mov dword ptr \[esp(?: \+ (0x[0-9a-f]+|\d+))?\], .*', t[q])):
            for j in range(3):
                kept.pop(num(m.group(1) or '0') - depth - 4 * j, None)
    tail_depth, tail_pushed, marked = depth, [], False
    after = None
    skip_to = 0
    replaced = 0   # (a global whose object, if any, is deleted before this one takes its place)
    for ti, s in enumerate(tail):
        if ti < skip_to:
            continue
        if s == 'push 0x10' and not tail_pushed and (rs := resource_step(t, ins, last + 1 + ti, regs)):
            if not marked:
                ops.append((14, 0, 0, 0, []))
                marked = True
            ops.append((20, rs[3], rs[1], rs[2], []))
            for r in ('eax', 'ecx', 'edx', rs[4]):
                regs.pop(r, None)
            skip_to = rs[0] - (last + 1)
            continue
        if s == 'call 0xddcda7':
            regs.pop('ecx', None)
            continue
        if framed and s == 'mov esp, ebp' and not tail_pushed:
            continue   # (the aligned frame given back: its pop ebp follows)
        # the global's object before, if any, deleted (its table's slot 0, given 1): mov ecx, [G]; cmp ecx, 0; je OVER;
        # mov R, [ecx]; push 1; call [R]; OVER: (then this one put in its place, below)
        if not tail_pushed and not replaced and (m := re.fullmatch(r'mov ecx, dword ptr \[(0x1[0-9a-f]{6})\]', s)) and ti + 6 < len(tail):
            mc = re.fullmatch(r'(?:cmp ecx, ' + REG + r'|test ecx, ecx)', tail[ti + 1])
            mj = re.fullmatch(r'je (0x[0-9a-f]+)', tail[ti + 2])
            mr = re.fullmatch(r'mov ' + REG + r', dword ptr \[ecx\]', tail[ti + 3])
            if mc and (not mc.group(1) or regs.get(mc.group(1)) == ('const', 0)) and mj and mr and tail[ti + 4] == 'push 1' and \
                    tail[ti + 5] == 'call dword ptr [%s]' % mr.group(1) and ins[last + 1 + ti + 6].address == num(mj.group(1)):
                replaced = num(m.group(1))
                for r in ('eax', 'ecx', 'edx', mr.group(1)):
                    regs.pop(r, None)
                skip_to = ti + 6
                continue
        if a in HAND_AFTER and s == HAND_AFTER[a][0] and not tail_pushed:
            after = HAND_AFTER[a][1]   # the rest is the hand-written function's
            break
        if (m := re.fullmatch(r'mov ' + REG + r', dword ptr \[esp \+ (0x[0-9a-f]+|\d+)\]', s)) and num(m.group(2)) - tail_depth - 4 * len(tail_pushed) in kept:
            regs[m.group(1)] = kept[num(m.group(2)) - tail_depth - 4 * len(tail_pushed)]
            continue
        if (m := re.fullmatch(r'push (0x[0-9a-f]+|\d+)', s)):
            tail_pushed.append(('imm', num(m.group(1))))
            continue
        if (m := re.fullmatch(r'call (0x[0-9a-f]+)', s)):
            # a call on a member after the registration and the mark: the steps after the marker (kind 14)
            fn, this = num(m.group(1)), regs.get('ecx')
            if not this or this[0] != 'this' or ret_args(fn) != len(tail_pushed) or len(tail_pushed) > 4:
                raise Reject('end call: ' + s)
            if not marked:
                ops.append((14, 0, 0, 0, []))
                marked = True
            ops.append((3, this[1], 0, fn, tail_pushed[::-1]))
            tail_pushed = []
            for r in ('eax', 'ecx', 'edx'):
                regs.pop(r, None)
            continue
        if tail_pushed and not s.startswith('lea '):
            raise Reject('end pushed: ' + s)
        if s.startswith('pop '):
            tail_depth -= 4
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
            elif m.group(1) == 'dword' and regs.get(v) and regs[v][0] == 'this':
                ops.append((2, off, regs[v][1], 0, []))   # a pointer into the object itself
                continue
            else:
                value = None
            if value is not None:
                ops.append((0 if m.group(1) == 'dword' else 1, off, value, 0, []))
                continue
        if (m := re.fullmatch(r'mov ' + REG + r', (0x[0-9a-f]+|\d+)', s)):
            regs[m.group(1)] = ('const', num(m.group(2)))
            continue
        if (m := re.fullmatch(r'xor ' + REG + ', ' + REG, s)) and m.group(1) == m.group(2):
            regs[m.group(1)] = ('const', 0)
            continue
        if (m := re.fullmatch(r'mov ' + REG + ', ' + REG, s)):
            regs[m.group(1)] = regs.get(m.group(2))
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
    if a in HAND_AFTER and not after:
        raise Reject('hand-written end not found')
    if replaced:
        if replaced != instance:
            raise Reject('end replaced')
        instance |= 1   # (bit 0: the object there before deleted first)
    tail_of = (instance, link, count, after) if (instance or link or count or after) else None
    return desc, index, ops, fields, names, nargs, pair, tail_of, base_fn, base_n, base_x


out = ['// Saved classes\' constructors taken over by class_construct (classes.cpp); written by tools/class_ctors.py.']
member_fns = {}   # the functions the steps call on members and new objects, with how many values each takes (for the test's stand-ins)
done = total = 0
bases = []   # the constructors built on a parent class's constructor: (address, that base)
reasons = {}
singles = {}   # (the copies that make a game object if its global holds none: the global and the constructor)
reason_bytes = {}
for a, r in sorted(rows.items()):
    n = int(r['size'])
    if (r['status'] != 'documented' and a not in mine) or a in taken or n < 150 or n > 20000:
        continue
    calls_ = [x.op_str for x in md.disasm(bytes(img[a - BASE:a - BASE + n]), a) if x.mnemonic == 'call']
    if '0x800a80' not in calls_ and not ('0x8087e0' in calls_ and re.fullmatch(r'0x[0-9a-f]+', calls_[0])):
        continue
    try:
        desc, index, ops, fields, names, nargs, pair, tail_of, base_fn, base_n, base_x = parse(a, n)
    except (Reject, StopIteration, AttributeError, ValueError) as e:
        key = str(e)[:70]
        singles.pop(a, None)
        if os.environ.get('WHY') and (key.startswith(os.environ["WHY"]) or os.environ["WHY"] == "*"): print(hex(a), e, '(near %#x)' % AT[0])
        reasons[key] = reasons.get(key, 0) + 1
        reason_bytes[key] = reason_bytes.get(key, 0) + n
        continue
    out.append(f'static const ClassOp class_{a:x}_ops[] = {{' + ', '.join(f'{{{k}, {o:#x}, {v:#x}, {fn:#x}, {len(args)}, {{{", ".join(hex(x[1]) for x in args) or "0"}}}, {sum(1 << q for q, x in enumerate(args) if x[0] in ("glob", "gobj")):#x}, {sum(1 << q for q, x in enumerate(args) if x[0] in ("self", "gobj")):#x}}}' for k, o, v, fn, args in ops) + '};' if ops else f'static const ClassOp class_{a:x}_ops[] = {{{{0, 0, 0, 0, 0, {{0}}, 0, 0}}}};')
    out.append(f'static const ClassField class_{a:x}_fields[] = {{' + ', '.join(f'{{{s:#x}, {c:#x}, {tb:#x}, {len(args)}, {{{", ".join(hex(x) for x in args) or "0"}}}, {par}}}' for s, c, tb, args, par in fields) + '};')
    out.append(f'static const uint8_t class_{a:x}_original[] = {{' + ', '.join(f'0x{b:02x}' for b in img[a - BASE:a - BASE + 7]) + '};')
    if names:
        out.append(f'static const ClassName class_{a:x}_names[] = {{' + ', '.join(f'{{{nm:#x}, {v:#x}}}' for nm, v in names) + '};')
    tail = f', class_{a:x}_names, {len(names)}' if names else ''
    if pair or tail_of:
        tail = (tail or ', nullptr, 0') + (', true' if pair else ', false')
    if tail_of:
        instance, link, count, after = tail_of
        link = link or [0, -1, 0]
        out.append(f'static const ClassTail class_{a:x}_tail = {{{instance:#x}, {link[0]:#x}, {link[1]}, {link[2]:#x}, {count:#x}' + (f', {after}' if after else '') + '};')
        tail += f', &class_{a:x}_tail'
    if base_fn != 0x800a80:   # (another base: the parameters before it written out)
        if ', true' not in tail and ', false' not in tail:
            tail = (tail or ', nullptr, 0') + ', false'
        if '_tail' not in tail:
            tail += ', nullptr'
        tail += f', {base_fn:#x}' + (f', 4, {base_x:#x}' if base_n == 4 else '')
        bases.append((a, base_fn, base_n))
    out.append(f'REWRITE("Class::Construct", {a:#x}, class_{a:x}_original, (&class_construct{nargs if nargs != 3 else ""}<{desc:#x}, {index:#x}, class_{a:x}_ops, {len(ops)}, class_{a:x}_fields, {len(fields)}{tail}>), &g_class_constructs);')
    done += 1
    total += n
    for kind, o, v, fn, args in ops:
        if kind == 5:
            read_globals.add(v)   # (a member set to a global's value)
        if kind in (3, 10, 11, 15):
            read_globals.update(x[1] for x in args if isinstance(x, tuple) and x[0] == 'glob')
        if kind in (3, 10, 15):
            member_fns[fn] = len(args) - (kind == 15)
out.append('// the stretches written by hand (step kind 17), by number')
out.append('static void class_hand(uint32_t number, uint8_t *self, const uint32_t *given) {')
out.append('    switch (number) {')
out += [f'    case {q}: {name}(self, given); break;' for q, name in enumerate(HANDS)]
out.append('    default: (void)self, (void)given; break;')
out.append('    }')
out.append('}')
out.append('// the functions the steps call on members and new objects, each with how many values it takes (the test stands in for them)')
out.append('extern const uint32_t class_member_fns[] = {' + ', '.join(f'{fn:#x}, {c}' for fn, c in sorted(member_fns.items())) + ', 0};')
out.append('// the copies that make a game object when its global holds none (kind 30): copy, global, constructor (the tester stands in for it)')
out.append('extern const uint32_t class_singletons[];')
out.append('const uint32_t class_singletons[] = {' + ''.join(f'{a_:#x}, {g_:#x}, {f_:#x}, ' for a_, (g_, f_) in sorted(singles.items())) + '0};')
out.append('// the member functions given a stack reference (kind 25): the tester stands in for each as for 0x560D20')
out.append('extern const uint32_t class_ref_setters[];')
out.append('const uint32_t class_ref_setters[] = {' + ''.join(f'{f_:#x}, ' for f_ in sorted(ref_setters)) + '0};')
out.append("// the game globals the steps read as values: the tester gives each a value of the case's (before its own settings)")
out.append('extern const uint32_t class_read_globals[];')
out.append('const uint32_t class_read_globals[] = {' + ''.join(f'{g_:#x}, ' for g_ in sorted(read_globals)) + '0};')
out.append('// the functions taking nothing whose value a member gets (kind 23): the tester stands in for each')
out.append('extern const uint32_t class_value_calls[];')
out.append('const uint32_t class_value_calls[] = {' + ''.join(f'{f_:#x}, ' for f_ in sorted(value_calls)) + '0};')
out.append("// the constructors built on a parent class's constructor, and that base (the tester stands in for it)")
out.append('extern const uint32_t class_bases[];')
out.append('const uint32_t class_bases[] = {' + ''.join(f'{a_:#x}, {b_:#x}, {n_}, ' for a_, b_, n_ in bases) + '0};')
open(os.path.join(HERE, 'forever', 'src', 'rewrite', 'class_list.inc'), 'w', newline='\n').write('\n'.join(out) + '\n')
print(done, 'constructors,', total, 'bytes; not taken:', sum(reasons.values()), 'constructors,', sum(reason_bytes.values()), 'bytes')
for key, size in sorted(reason_bytes.items(), key=lambda kv: -kv[1])[:int(os.environ.get('TOP', '14'))]:
    print(f'  {size:7d} {reasons[key]:3d} {key}')
