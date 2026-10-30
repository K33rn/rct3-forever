"""Lists the copies of the template rewrites in forever/src/rewrite/generated.inc, from your own RCT3plus.exe.

usage: python tools/gen_rewrites.py "<game folder>\\RCT3plus.exe"

Some rewrites are templates (forever/src/rewrite/templates.cpp) that stand for hundreds of compiler-made functions
differing only in an address: which destructor a deleting destructor calls, which global holds an interface's id.
This finds every function with exactly the template's instructions (constants aside) and writes one line per copy:
its address, the constants it uses and its first bytes (checked before it is replaced). Only addresses and those
few bytes come from the exe; the code of each rewrite is the hand-written template.
"""
import csv
import glob
import os
import re
import sys

import capstone
import pefile

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BASE = 0x400000

DELETING = ('push esi ; mov esi, ecx ; call N ; test byte ptr [esp + N], N ; je N ; push esi ; call N ; '
            'add esp, N ; mov eax, esi ; pop esi ; ret N')
MODULE = 'sub esp, N ; mov ecx, esp ; call N ; mov ecx, N ; call N ; push N ; call N ; pop ecx ; ret '
CACHED = ('push edi ; mov edi, ecx ; mov al, byte ptr [edi + N] ; test al, al ; jne N ; mov eax, dword ptr [edi + N] ; '
          'push esi ; mov esi, dword ptr [eax + N] ; test esi, esi ; je N ; mov edx, dword ptr [esi] ; mov ecx, esi ; '
          'call dword ptr [edx + N] ; test eax, eax ; je N ; mov ecx, dword ptr [N] ; cmp ecx, dword ptr [eax + N] ; '
          'jge N ; mov eax, dword ptr [eax + N] ; mov eax, dword ptr [eax + ecx*N] ; test eax, eax ; je N ; '
          'mov eax, dword ptr [eax + N] ; add eax, esi ; jmp N ; xor eax, eax ; mov dword ptr [edi + N], eax ; '
          'mov byte ptr [edi + N], N ; pop esi ; mov eax, dword ptr [edi + N] ; pop edi ; ret ')
CLEANUP = ('mov ecx, dword ptr [N] ; test ecx, ecx ; je N ; mov eax, dword ptr [ecx] ; push N ; '
           'call dword ptr [eax] ; mov dword ptr [N], N ; ret ')
QUERY = ('push esi ; mov esi, ecx ; test esi, esi ; je N ; mov eax, dword ptr [esi] ; call dword ptr [eax + N] ; '
         'test eax, eax ; je N ; mov ecx, dword ptr [N] ; cmp ecx, dword ptr [eax + N] ; jge N ; '
         'mov edx, dword ptr [eax + N] ; mov eax, dword ptr [edx + ecx*N] ; test eax, eax ; je N ; '
         'mov eax, dword ptr [eax + N] ; add eax, esi ; pop esi ; ret  ; xor eax, eax ; pop esi ; ret ')


# matched families: (macro, representative, operands ignored, parameter operands), operands as (instruction, operand)
MATCH = [
    ('NAMED_TEXT_REF', 0x403F30, [(1, 0)], [(37, 0), (72, 1), (127, 1)]),   # suffix, first table, class table
    ('GAME_TEXT_COPY', 0x41F210, [(1, 0)], [(9, 0)]),                        # the text's name
    ('TEXT_REF_FROM_TEXT', 0x403E50, [(1, 0)], [(16, 0), (30, 1), (49, 1)]),  # suffix, first table, class table
    ('START_KEY', 0xEB2580, [], [(5, 0), (6, 0), (10, 0), (11, 0), (13, 1), (15, 0)]),   # keys, names, global, dtor
    ('START_KEY3', 0xEB23A0, [], [(4, 0), (5, 0), (6, 0), (10, 0), (11, 0), (13, 1), (15, 0)]),
    ('START_KEY1', 0xEB25D0, [], [(4, 0), (11, 0), (12, 0), (13, 0), (14, 1), (16, 0)]),
    ('START_KEY1_FLAG', 0xEB2940, [], [(4, 0), (11, 0), (12, 0), (13, 0), (14, 1), (16, 0)]),
    ('TEXT_WIDEN', 0x401050, [], []),
    ('TEXT_CREATE', 0x45C000, [(1, 0)], [(14, 0), (16, 0)]),                  # the C text, the function making it
    ('TABLE20_FREE', 0x80F710, [(1, 0)], []),
    ('START_TEXT', 0xECA490, [(1, 0)], [(13, 0), (20, 0), (34, 1), (76, 0)]),  # text, suffix, global, dtor
    ('RESOURCE_TYPE', 0xBB28E0, [(1, 0)], [(17, 0), (31, 0), (45, 0), (101, 1), (53, 0)], (53,)),   # + its flag
    ('WITH_REFERENCE', 0x536EE0, [(1, 0)], [(25, 1), (30, 1), (32, 0), (35, 0)], (30,)),   # table, field, function, dtor
    ('QUERY_REPORT', 0x42A430, [], [(10, 1)]),                               # the global holding the id
    ('WIDE_COPY_OBJECT', 0x6D2F00, [(1, 0)], [(7, 1)]),
    ('WIDE_COPY_GLOBAL', 0x4CE710, [(1, 0)], [(7, 1)]),
    ('WIDE_COPY_THIS', 0x6DCDF0, [(1, 0)], [(8, 1)]),
    ('START_BEHAVIOUR', 0xEAAAD0, [(1, 0)], [(13, 0), (19, 0), (22, 1), (40, 0)]),
    # factories: memory of SIZE bytes, constructed (or null): from two arguments, or from two fields of this
    ('GLOBAL_FACTORY', 0x436210, [(1, 0)], [(6, 0), (14, 0), (15, 0)], (6,), [[(15, 0), (22, 0)]]),   # size, ctor, global
    ('LAZY_FACTORY', 0x44CC40, [(1, 0)], [(6, 1), (9, 0), (17, 0)], (9,), [[(6, 1), (18, 0), (24, 0)]]),   # global, size, ctor
    ('LAZY_FACTORY_INIT', 0x40DFE0, [(1, 0)], [(6, 1), (10, 0), (18, 0), (20, 0), (21, 1), (22, 0), (22, 1)],
     (10, 18, 22), [[(6, 1), (26, 0)]]),   # global, size, ctor argument, ctor, table, byte field and its value
    ('CACHED', 0x558F20, [], [(2, 1), (5, 1), (7, 1), (15, 1), (25, 0)], (2, 5, 7, 25, 26, 30, 31, 33),
     [[(2, 1), (26, 0), (31, 0)], [(25, 0), (30, 0), (33, 1)]]),   # flag, reference, object, id, cache
    ('DTOR_TABLES', 0x426C90, [(1, 0)], [(10, 1), (11, 1), (13, 0), (15, 1)]),   # tables, member destructor, base table
    ('FREE_TWO', 0x42FD80, [(1, 0)], [(9, 1)], (9, 16), [[(9, 1), (16, 0)]]),   # the field freed first
    ('POINTER_RESIZE', 0xBD6750, [], []),
    ('ARRAY_RESERVE', 0xBD7580, [], [(28, 0)]),                                 # its resize (a tail jump)
    ('GUI_INIT', 0x6D79E0, [(1, 0)], [(14, 1), (27, 1), (28, 1), (29, 1)], (), [[(14, 1), (31, 1), (43, 1)]]),
    ('GUI_MAKE', 0x6D9A20, [(1, 0)], [(7, 0), (15, 1), (22, 1), (23, 1), (24, 1)], (7,), [[(15, 1), (25, 1), (32, 1)]]),
    ('GUI_CREATE2', 0x6D9980, [(1, 0)], [(7, 0), (18, 1), (25, 1), (26, 1), (27, 1)], (7,), [[(18, 1), (28, 1), (35, 1)]]),
    ('CONDITION_NAME', 0x6DC430, [(1, 0)], [(17, 0)]),                         # the condition's name
    ('FACTORY_K3', 0x4AEE60, [(1, 0)], [(6, 0), (17, 0), (15, 0), (14, 0), (13, 0)], (6, 13, 14, 15)),   # size, ctor, args
    ('GLOBAL_FACTORY_K1', 0x4137A0, [(1, 0)], [(6, 0), (15, 0), (13, 0), (16, 0)], (6, 13), [[(16, 0), (23, 0)]]),
    ('FIELD_FACTORY', 0x6D4300, [(1, 0)], [(7, 0), (19, 0), (17, 0), (16, 0), (20, 0)], (7, 16, 17, 20, 28),
     [[(20, 0), (28, 0)]]),   # size, ctor, two args (then this), the field
    ('REF_INTERFACE', 0x4D1800, [], [(0, 1), (10, 1)], (0,)),                 # the reference's field, the id
    ('REF_INTERFACE', 0x7307C0, [], [(0, 1), (10, 1)], (0,)),
    ('REF_INTERFACE', 0x4CA9B0, [], [(0, 1), (11, 1)], (0,)),
    ('ARRAY_FREE', 0x424510, [], [(10, 0), (8, 0)], (10,)),                     # another shape: size, dtor
    ('DTOR_DELETE_MEMBER', 0x490070, [(1, 0)], [(9, 1), (10, 1), (11, 1), (20, 0)], (11,)),   # tables, member, base
    ('DTOR_MEMBER_BASE', 0x42E3B0, [(1, 0)], [(9, 1), (11, 0), (14, 0)], (9,)),   # member field, its dtor, base dtor
    ('FACTORY_K2', 0x5F80D0, [(1, 0)], [(6, 0), (16, 0), (14, 0), (13, 0)], (6, 13, 14)),   # size, ctor, args
    ('FACTORY_K5', 0x6D02C0, [(1, 0)], [(6, 0), (19, 0), (17, 0), (16, 0), (15, 0), (14, 0), (13, 0)], (6, 13, 14, 15, 16, 17)),
    ('FACTORY_K5', 0x6D0330, [(1, 0)], [(6, 0), (19, 0), (17, 0), (16, 0), (15, 0), (14, 0), (13, 0)], (6, 14, 15, 16, 17)),
    ('FACTORY_2K', 0x464620, [(1, 0)], [(6, 0), (20, 0), (18, 0), (15, 0)], (6, 15, 18)),   # size, ctor, first/last arg
    ('CTOR_BASE_TABLE', 0x471EF0, [], [(2, 0), (3, 1)]),                        # base constructor, table
    ('SET_TWO_VECTORS', 0x4884B0, [], []),
    ('FACTORY2', 0x430B70, [(1, 0)], [(6, 0), (19, 0)], (6,)),            # size (push imm8), constructor
    ('FACTORY2', 0x5F2E70, [(1, 0)], [(6, 0), (19, 0)], (6,)),            # size (push imm32), constructor
    ('FACTORY_FIELDS', 0x633140, [(1, 0)], [(7, 0), (16, 1), (15, 1), (22, 0)], (7, 15, 16)),   # size, fields, ctor
]


def main():
    pe = pefile.PE(sys.argv[1], fast_load=True)
    mem = pe.get_memory_mapped_image()
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True
    X = capstone.x86
    funcs = [r for r in csv.DictReader(open(os.path.join(HERE, 'docs', 'functions.csv')))]
    out = ['// Generated by tools/gen_rewrites.py from RCT3plus.exe: the copies of the template rewrites in templates.cpp',
           '// (address, constants, the first bytes checked before replacing). No game code: addresses only.']
    n_del = n_q = n_m = n_c = n_l = n_g = 0
    # shapes read from one representative: the function at that address, its constants aside
    def sig_at(a):
        n = next(int(r['size']) for r in funcs if int(r['address'], 16) == a)
        ins = [i for i in md.disasm(mem[a - BASE:a - BASE + n], a) if i.mnemonic not in ('int3', 'nop')]
        return ' ; '.join(i.mnemonic + ' ' + re.sub(r'\b\d+\b', 'N', re.sub(r'0x[0-9a-f]+', 'N', i.op_str)) for i in ins)
    LOAD = sig_at(0x425470)

    # strict shapes: every offset and small constant as in the representative; only addresses (code, tables,
    # globals: 0x400000 and up) may differ, and the instructions listed in `loose` may differ in any constant
    def strict(ins, loose=()):
        return [i.mnemonic + ' ' + (re.sub(r'0x[0-9a-f]+', 'N', i.op_str) if k in loose else
                                    re.sub(r'0x[0-9a-f]{6,}', lambda m: 'G' if int(m.group(0), 16) >= BASE else m.group(0), i.op_str))
                for k, i in enumerate(ins)]

    def body(a):
        n = next(int(r['size']) for r in funcs if int(r['address'], 16) == a)
        return [i for i in md.disasm(mem[a - BASE:a - BASE + n], a) if i.mnemonic not in ('int3', 'nop')]

    def addresses(ins):
        return [v for i in ins for o in i.operands if o.type == X.X86_OP_IMM
                for v in [o.imm & 0xffffffff] if v >= BASE]

    REF_NULL, REF_TO = strict(body(0x460440)), strict(body(0x410330))
    FREE_LOOSE = (10,)   # push <item size>
    ARRAY_FREE = strict(body(0x420B40), FREE_LOOSE)
    n_rn = n_rt = n_af = n_st = 0
    # start-up initialisers: also the same layout (instruction sizes, jumps to the same offsets)
    def inside(ins, target):
        return ins[0].address <= target < ins[-1].address + ins[-1].size

    def layout(ins, va):   # instruction sizes, and where jumps inside the function go (jumps out are operands)
        return [(i.size, (i.operands[0].imm - va) if i.group(capstone.CS_GRP_JUMP) and i.operands[0].type == X.X86_OP_IMM
                 and inside(ins, i.operands[0].imm) else None) for i in ins]
    section_shape = lambda ins: [x if k != 19 else re.sub(r'^push (0x[0-9a-f]+|\d+)$', 'push KIND', x) for k, x in enumerate(strict(ins))]   # the section's kind
    START = {}
    for kind, rep in (('name', 0xEA8790), ('class', 0xEA86C0), ('section', 0xEA93C0)):
        b = body(rep)
        START[kind] = (section_shape(b) if kind == 'section' else strict(b), layout(b, rep))
    n_sn = n_sc = n_ss = 0
    # functions the hand-written rewrites already replace (REWRITE and the arrays of REWRITE_COPIES)
    handwritten = set()
    for p in glob.glob(os.path.join(HERE, 'forever', 'src', 'rewrite', '*.cpp')):
        src = open(p, encoding='utf-8').read()
        handwritten |= {int(m.group(1), 16) for m in re.finditer(r'REWRITE\("[^"]+",\s*(0x[0-9a-fA-F]+)', src)}
        for m in re.finditer(r'uint32_t\s+\w+\[\]\s*=\s*\{([^}]*)\}', src):
            handwritten |= {int(h, 16) for h in re.findall(r'0x[0-9a-fA-F]+', m.group(1))}
    # families matched against a representative: the same instructions and layout, every address the same as the
    # representative's except the parameters (instruction, operand) and the ones ignored (exception handlers)
    def operands(ins):
        found = {}
        for k, i in enumerate(ins):
            for j, o in enumerate(i.operands):
                v = o.imm if o.type == X.X86_OP_IMM else o.mem.disp if o.type == X.X86_OP_MEM else None
                if i.group(capstone.CS_GRP_JUMP) and (o.type != X.X86_OP_IMM or inside(ins, o.imm)):
                    continue   # jumps inside the function: compared by layout()
                if v is not None and (v & 0xffffffff) >= BASE:
                    found[(k, j)] = v & 0xffffffff
        return found
    MATCHED = []
    # an entry may name instructions whose small constants are parameters too (masked in the shape): a 5th item
    def shape_of(ins, loose):
        return [re.sub(r'(0x[0-9a-f]+|\d+)', 'K', x) if k in loose else x for k, x in enumerate(strict(ins))]

    def value(ins, k, j):
        o = ins[k].operands[j]
        return (o.imm if o.type == X.X86_OP_IMM else o.mem.disp) & 0xffffffff
    for macro, rep, ignore, params, *rest in MATCH:
        # optional: instructions with masked constants, and groups of operands that must hold the same value
        loose = rest[0] if rest else ()
        same = rest[1] if len(rest) > 1 else ()
        b = body(rep)
        skip = set(ignore) | set(params) | {kj for group in same for kj in group}
        fixed = {kj: v for kj, v in operands(b).items() if kj not in skip}
        MATCHED.append((macro, rep, shape_of(b, loose), layout(b, rep), fixed, params, len(b), loose, same))
    counts = {m[0]: 0 for m in MATCH}
    # ObjectArray::SetCapacity: matched by its shape with every constant masked (the copies differ in registers and
    # in how they multiply by the item size), its constants found by their roles
    def masked(ins):
        return [(i.mnemonic, re.sub(r'(0x[0-9a-f]+|\d+)', 'N', i.op_str), i.size) for i in ins]
    SETCAP = [masked(body(a)) for a in (0x420C10, 0x455CE0, 0x463060)]
    RESIZE = [masked(body(a)) for a in (0x420B70,)]   # ObjectArray::Resize: destroys first, then makes n new
    n_sc_ = 0
    n_rs_ = 0
    for r in funcs:
        va, n = int(r['address'], 16), int(r['size'])
        if n > 600 or va in handwritten:
            continue
        ins = [i for i in md.disasm(mem[va - BASE:va - BASE + n], va) if i.mnemonic not in ('int3', 'nop')]
        s = ' ; '.join(i.mnemonic + ' ' + re.sub(r'\b\d+\b', 'N', re.sub(r'0x[0-9a-f]+', 'N', i.op_str)) for i in ins)
        first = ', '.join(f'0x{b:02x}' for b in mem[va - BASE:va - BASE + 8])
        st = strict(ins)
        if st == REF_NULL or st == REF_TO:
            a = addresses(ins)
            if a[0] != 0xF26AE0 or a[1] != (0x805750 if st == REF_NULL else 0x805700):
                continue
            out.append(f'{"REF_NULL" if st == REF_NULL else "REF_TO"}({va:#x}, {a[2]:#x}, {first})')
            if st == REF_NULL:
                n_rn += 1
            else:
                n_rt += 1
            continue
        if masked(ins) in RESIZE:
            calls = [i.operands[0].imm & 0xffffffff for i in ins if i.mnemonic == 'call']
            imms = [i.operands[0].imm & 0xffffffff for i in ins if i.mnemonic == 'push' and i.operands[0].type == X.X86_OP_IMM]
            leas = [i.operands[1].mem.disp for i in ins if i.mnemonic == 'lea' and i.operands[1].mem.index == 0]
            header = next((d for d in leas if d > 0), None)
            # pushes: -1, handler, then dtor, size (destroy), dtor, ctor, size (make)
            if calls == [0xDDCF39, 0xBCE010, 0xBCDFD0, 0xDDCF99] and len(imms) == 7 and imms[2] == imms[4] and                     imms[3] == imms[6] and header and -header in leas:
                out.append(f'OBJECT_RESIZE({va:#x}, {imms[3]:#x}, {header:#x}, {imms[5]:#x}, {imms[2]:#x}, {first})')
                n_rs_ += 1
                continue
        if masked(ins) in SETCAP:
            calls = [i.operands[0].imm & 0xffffffff for i in ins if i.mnemonic == 'call']
            def pushes_before(target):
                k = next(k for k, i in enumerate(ins) if i.mnemonic == 'call' and i.operands[0].imm & 0xffffffff == target)
                found = []
                for i in reversed(ins[:k]):
                    if i.mnemonic == 'push' and i.operands[0].type == X.X86_OP_IMM:
                        found.insert(0, i.operands[0].imm & 0xffffffff)
                    if len(found) == {0xDDCF99: 3, 0xDDCF39: 2}[target] or i.mnemonic == 'call':
                        break
                return found
            if len(calls) == 5 and calls[:2] == [0xBCDFD0, 0xDDCF99] and calls[3:] == [0xDDCF39, 0xBCE010]:
                dtor, ctor, size = pushes_before(0xDDCF99)
                dtor2, size2 = pushes_before(0xDDCF39)
                # the header in front of the items (the count at its start): lea items, [block + header] when made,
                # lea block, [items - header] when freed
                leas = [i.operands[1].mem.disp for i in ins if i.mnemonic == 'lea' and i.operands[1].mem.index == 0]
                header = next((d for d in leas if d > 0), None)
                if (dtor, size) == (dtor2, size2) and header and -header in leas:
                    out.append(f'OBJECT_SETCAP({va:#x}, {size:#x}, {header:#x}, {ctor:#x}, {dtor:#x}, {calls[2]:#x}, {first})')
                    n_sc_ += 1
                    continue
        hit = False
        for macro, rep, shape, lay, fixed, params, length, loose, same in MATCHED:
            if len(ins) != length or shape_of(ins, loose) != shape or layout(ins, va) != lay:
                continue
            ops = operands(ins)
            if any(ops.get(kj) != v for kj, v in fixed.items()):
                continue
            if any(len({value(ins, *kj) for kj in group}) != 1 for group in same):
                continue
            out.append(f'{macro}(' + ', '.join([f'{va:#x}'] + [f'{value(ins, *kj):#x}' for kj in params] + [first]) + ')')
            counts[macro] += 1
            hit = True
        if hit:
            continue
        imm = lambda k, j=0: ins[k].operands[j].imm & 0xffffffff
        disp = lambda k, j=0: ins[k].operands[j].mem.disp & 0xffffffff
        if (st, layout(ins, va)) == START['name']:
            if [imm(9), imm(12), imm(16)] == [0xBEC4B0, 0xBECA70, 0xBEC5E0]:
                out.append(f'START_NAME({va:#x}, {imm(7):#x}, {disp(14):#x}, {first})')
                n_sn += 1
            continue
        if (st, layout(ins, va)) == START['class']:
            if [imm(7), imm(17)] == [0xBCDFC0, 0x805FD0] and disp(18) == disp(25) and imm(6) == 0x3C:
                out.append(f'START_CLASS({va:#x}, {imm(14):#x}, {imm(13):#x}, {disp(18):#x}, {first})')
                n_sc += 1
            continue
        if (section_shape(ins), layout(ins, va)) == START['section']:
            if [imm(7), imm(15), imm(24), imm(35), imm(37), imm(41)] == [0xBCDFC0, 0xBCF850, 0x46FAD0, 0xBCE010, 0xBCE000, 0xDDC870]:
                out.append(f'START_SECTION({va:#x}, {imm(13):#x}, {imm(19):#x}, {imm(22, 1):#x}, {imm(40):#x}, {first})')
                n_ss += 1
            continue
        if strict(ins, FREE_LOOSE) == ARRAY_FREE:
            a = addresses(ins)   # the branch, the item destructor, the iterator, the free
            if a[2:] != [0xDDCF39, 0xBCE010]:
                continue
            out.append(f'ARRAY_FREE({va:#x}, {ins[10].operands[0].imm:#x}, {a[1]:#x}, {first})')
            n_af += 1
            continue
        if [(i.mnemonic, i.op_str.split(',')[0]) for i in ins] == [('mov', 'dword ptr [ecx]'), ('ret', '')] and                 n >= 7 and ins[0].operands[1].type == X.X86_OP_IMM:
            t = ins[0].operands[1].imm & 0xffffffff
            if 0xF26000 <= t < 0x10E7000:
                out.append(f'SET_TABLE({va:#x}, {t:#x}, {first})')
                n_st += 1
            continue
        if s == DELETING:
            calls = [i.operands[0].imm & 0xffffffff for i in ins if i.mnemonic == 'call']
            test = next(i for i in ins if i.mnemonic == 'test')
            if test.operands[0].mem.disp != 8 or test.operands[1].imm != 1:
                continue
            out.append(f'DELETING({va:#x}, {calls[0]:#x}, {calls[1]:#x}, {first})')
            n_del += 1
        elif s == MODULE:
            calls = [i.operands[0].imm & 0xffffffff for i in ins if i.mnemonic == 'call']
            g = next(i.operands[1].imm & 0xffffffff for i in ins if i.mnemonic == 'mov' and i.op_str.startswith('ecx, 0x'))
            f = next(i.operands[0].imm & 0xffffffff for i in ins if i.mnemonic == 'push')
            if ins[0].op_str != 'esp, 8' or calls[1:] != [0xBCDF40, 0xDDC870]:
                continue
            out.append(f'MODULE({va:#x}, {calls[0]:#x}, {g:#x}, {f:#x}, {first})')
            n_m += 1
        elif s == CACHED:
            d = lambda k, j: ins[k].operands[j].mem.disp & 0xffffffff
            flag, ref, obj, ident = d(2, 1), d(5, 1), d(7, 1), d(15, 1)
            consts = (d(12, 0), d(16, 1), d(18, 1), d(22, 1), d(26, 0), d(27, 0), d(29, 1), ins[27].operands[1].imm)
            cache = consts[4]
            if consts != (4, 0x18, 0x14, 0x10, cache, flag, cache, 1) or max(flag, ref, cache) >= 0x1000:
                continue
            out.append(f'CACHED({va:#x}, {flag:#x}, {ref:#x}, {obj:#x}, {ident:#x}, {cache:#x}, {first})')
            n_c += 1
        elif s == LOAD:
            calls = [i.operands[0].imm & 0xffffffff for i in ins if i.mnemonic == 'call' and
                     i.operands[0].type == X.X86_OP_IMM]
            stride = next(i.operands[1].imm for i in ins if i.mnemonic == 'add' and i.op_str.startswith('ebp, '))
            if calls[:2] != [0x809210, 0x809210] or len(calls) != 3:
                continue
            out.append(f'LOAD({va:#x}, {stride:#x}, {calls[2]:#x}, {first})')
            n_l += 1
        elif s == CLEANUP:
            g = ins[0].operands[1].mem.disp & 0xffffffff
            if ins[4].operands[0].imm != 1 or ins[6].operands[0].mem.disp & 0xffffffff != g or ins[6].operands[1].imm != 0:
                continue
            out.append(f'CLEANUP({va:#x}, {g:#x}, {first})')
            n_g += 1
        elif s == QUERY:
            ins_id = next(i for i in ins if i.mnemonic == 'mov' and i.op_str.startswith('ecx, dword ptr [0x'))
            cmp = next(i for i in ins if i.mnemonic == 'cmp')
            tbl = next(i for i in ins if i.op_str.startswith('edx, dword ptr [eax'))
            off = [i for i in ins if i.op_str.startswith('eax, dword ptr [eax')][0]
            if (cmp.operands[1].mem.disp, tbl.operands[1].mem.disp, off.operands[1].mem.disp) != (0x18, 0x14, 0x10):
                continue
            out.append(f'QUERY({va:#x}, {ins_id.operands[1].mem.disp & 0xffffffff:#x}, {first})')
            n_q += 1
    counts['OBJECT_SETCAP'] = n_sc_
    counts['OBJECT_RESIZE'] = n_rs_
    print(', '.join(f'{n} {m}' for m, n in counts.items()))
    open(os.path.join(HERE, 'forever', 'src', 'rewrite', 'generated.inc'), 'w', newline='\n').write('\n'.join(out) + '\n')
    print(n_del, 'deleting destructors,', n_q, 'interface queries,', n_m, 'module globals,', n_c, 'cached interfaces,', n_l, 'struct array loads,', n_g, 'global clean-ups,', n_rn + n_rt,
          'reference constructors,', n_af, 'object array frees,', n_st, 'table setters,',
          n_sn, 'start-up names,', n_sc, 'class descriptions,', n_ss, 'sections')


if __name__ == '__main__':
    main()
