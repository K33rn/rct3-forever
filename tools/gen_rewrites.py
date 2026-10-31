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
    ('TEXT_CREATE2', 0x475020, [(1, 0)], [(14, 0), (16, 0)]),                 # ret 8: C text, make fn
    ('TEXT_CREATE3', 0xB82F50, [(1, 0)], [(14, 0), (16, 0)]),                 # ret 0xc
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
    ('LAZY_FACTORY_K1', 0x425920, [(1, 0)], [(6, 1), (9, 0), (16, 0), (18, 0)], (9, 16), [[(6, 1), (19, 0), (25, 0)]]),
    ('FACTORY_REGS', 0x7FD540, [(1, 0)], [(8, 0), (20, 0)], (8,)),            # size, ctor; the two arguments in ecx, edx
    ('RENDER_STATE', 0xC6F000, [], [(2, 1), (10, 0)], (2, 10, 13), [[(2, 1), (13, 0)]]),   # the kept value's field, state
    ('FREE_LIST_TAKE', 0xBD2110, [], [(5, 1), (6, 0), (0, 1), (3, 1), (7, 1), (17, 1), (20, 1)], (),
     [[(0, 1), (12, 1), (16, 0)], [(7, 1), (14, 1), (19, 0)], [(20, 1), (22, 0)]]),   # base, make, then its fields
    ('LISTENER_DTOR', 0x440ED0, [], [(2, 1), (0, 1)], (0,)),                    # the table put back, the node's field
    ('LISTENER_DTOR_QUIET', 0x413680, [], [(2, 1), (0, 1)], (0,)),              # the same, a broken list left as it is
    ('DTOR_TABLES_MEMBER', 0x41B5B0, [(1, 0)], [(10, 1), (11, 1), (8, 1), (12, 1), (14, 0), (17, 0)], (8,)),
    ('DTOR_DELETE2', 0x6157F0, [(1, 0)], [(9, 1), (10, 1), (11, 1), (18, 1), (26, 0)], (11, 18)),   # 2 tables, 2 members
    ('DTOR5_DELETE2', 0x65D0D0, [(1, 0)], [(9, 1), (10, 1), (11, 0), (11, 1), (12, 0), (12, 1), (13, 0), (13, 1), (14, 1),
                                           (21, 1), (29, 0)], (11, 12, 13, 14, 21)),   # 5 tables, 2 members, base
    ('DTOR_MEMBER_IF', 0x4D3B50, [(1, 0)], [(13, 1), (14, 0), (17, 0)], (13,)),   # member field, its dtor, base dtor
    ('LAZY_SINGLETON', 0x436100, [(1, 0)], [(6, 1), (9, 0), (16, 0), (17, 0), (18, 0), (20, 0)], (9, 16, 17, 18),
     [[(6, 1), (21, 1), (27, 1)]]),   # global, size, ctor arguments, ctor
    ('LOAD', 0x462DB0, [], [(61, -1), (50, 0)], (61,)),                          # another shape: stride, resize
    ('QUADTREE_ADD', 0x430520, [(2, 0)], [(29, 0)]),                            # the items' constructor
    ('QUADTREE_INSERT', 0x4311C0, [], [(37, 0), (41, 0)]),                     # its node's Add, its MarkChanged
    ('INTERFACE_FORWARD', 0x4AB940, [], [(6, 1), (2, 1), (26, 0), (15, 1), (61, 0)], (2, 6, 15, 26, 27, 28, 31, 34, 43, 54, 55, 56, 61),
     [[(2, 1), (27, 0), (31, 1), (55, 0)], [(6, 1), (34, 1)], [(26, 0), (28, 1), (54, 0), (56, 1)], [(15, 1), (43, 1)]]),
    ('START_KEY_FULL', 0xEB2A80, [], [(6, 0), (5, 0), (4, 0), (12, 0), (11, 0), (10, 0), (2, 0), (1, 0), (13, 1), (15, 0)],
     (1, 2, 4, 5, 6, 10, 11, 12, 13, 15)),
    ('START_KEY_FULL0', 0xEB45B0, [], [(4, 0), (3, 0), (2, 0), (13, 0), (12, 0), (11, 0), (8, 0), (7, 0), (14, 1), (16, 0)],
     (1, 2, 3, 4, 7, 8, 11, 12, 13, 14, 16)),
    ('ARRAY_FREE16', 0x81DC00, [], [(10, 0), (8, 0)], (10,)),                   # 16-byte header: size, dtor
    ('OBJECT_CONSTRUCT', 0x7404F0, [(2, 0)], [(25, 0), (22, 0), (21, 0)], (10, 25), [[(10, 2), (25, 0)]]),
    ('TRIVIAL_SETCAP', 0x46F560, [(2, 0)], [(28, 0), (26, 0)], (28,)),
    ('TRIVIAL_SETCAP', 0x7E5400, [(2, 0)], [(29, 0), (27, 0)], (29,)),
    ('TRIVIAL_SETCAP', 0x50FC40, [(2, 0)], [(29, 0), (27, 0)], (29,)),
    ('OBJECT_CONSTRUCT', 0x50FE90, [(2, 0)], [(24, 0), (21, 0), (20, 0)], (24,)),
    ('OBJECT_CONSTRUCT', 0x821480, [(2, 0)], [(24, 0), (21, 0), (20, 0)], (24,)),
    ('PAIR_RELEASE', 0x4A35B0, [(1, 0)], []),
    ('SORTED_MAP_CTOR', 0x7FBD70, [(1, 0)], []),
    ('BEHAVIOUR_CREATE', 0x46DFA0, [(1, 0)], [(8, 0), (21, 0), (26, 0)], (8,)),
    ('INTERFACE_FLOAT', 0x52C870, [], [(5, 1), (2, 1), (26, 0), (15, 1), (31, 1), (30, 1), (35, 0), (37, 0)],
     (2, 5, 15, 26, 27, 29, 30, 31, 35, 37), [[(2, 1), (27, 0)], [(26, 0), (29, 1)]]),
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
    func_size = {int(r['address'], 16): int(r['size']) for r in funcs}

    def inside(ins, target):   # within the function's whole byte range (not just the last disassembled instruction,
        start = ins[0].address  # which can stop short of trailing bytes a jump may land on)
        return start <= target < start + func_size.get(start, ins[-1].address + ins[-1].size - start)

    def layout_by_index(ins, loose):
        # as layout(), but jumps by the instruction they go to and no size for the instructions with masked constants
        # (so a constant's encoding, push imm8 or imm32, doesn't matter)
        at = {i.address: k for k, i in enumerate(ins)}
        return [(None if k in loose else i.size,
                 at.get(i.operands[0].imm, 'mid') if i.group(capstone.CS_GRP_JUMP) and i.operands[0].type == X.X86_OP_IMM
                 and inside(ins, i.operands[0].imm) else None) for k, i in enumerate(ins)]

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

    def value(ins, k, j):   # j < 0: the scale of operand -j's memory reference (lea r, [x*scale])
        if j < 0:
            return ins[k].operands[-j].mem.scale
        o = ins[k].operands[j]
        return (o.imm if o.type == X.X86_OP_IMM else o.mem.disp) & 0xffffffff
    for macro, rep, ignore, params, *rest in MATCH:
        # optional: instructions with masked constants, and groups of operands that must hold the same value
        loose = rest[0] if rest else ()
        same = rest[1] if len(rest) > 1 else ()
        b = body(rep)
        skip = set(ignore) | set(params) | {kj for group in same for kj in group}
        fixed = {kj: v for kj, v in operands(b).items() if kj not in skip}
        MATCHED.append((macro, rep, shape_of(b, loose), layout_by_index(b, loose), fixed, params, len(b), loose, same))
    counts = {m[0]: 0 for m in MATCH}
    # ObjectArray::SetCapacity: matched by its shape with every constant masked (the copies differ in registers and
    # in how they multiply by the item size), its constants found by their roles
    def masked(ins):
        return [(i.mnemonic, re.sub(r'(0x[0-9a-f]+|\d+)', 'N', i.op_str), i.size) for i in ins]
    SETCAP = [masked(body(a)) for a in (0x420C10, 0x455CE0, 0x463060)]
    RESIZE = [masked(body(a)) for a in (0x420B70, 0x68EFA0, 0x460E80)]   # ObjectArray::Resize: destroys first, then makes n new (two item-size encodings)
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
            if len(ins) != length or shape_of(ins, loose) != shape or layout_by_index(ins, loose) != lay:
                continue
            ops = operands(ins)
            if any(ops.get(kj) != v for kj, v in fixed.items()):
                continue
            if any(len({value(ins, *kj) for kj in group}) != 1 for group in same):
                continue
            out.append(f'{macro}(' + ', '.join([f'{va:#x}'] + [f'{value(ins, *kj):#x}' for kj in params] + [first]) + ')')
            counts[macro] += 1
            hit = True
            break   # one entry per function (two representatives may match the same copies)
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

    # multi-member destructors: set K tables, delete N members (slot 0, flag 1, null-checked), run the base; each
    # instruction is accounted for, so the rewrite is exact. Emitted as MEMBER_DTOR(at, base, ntables, first8, data...)
    emitted = {int(l[l.index('(')+1:l.index(',')], 16) for l in out if '(' in l and ',' in l and l[l.index('(')+1:l.index(',')].startswith('0x')}
    n_md = 0

    def parse_md(a, n):
        xs0 = [i for i in md.disasm(mem[a - BASE:a - BASE + n], a) if i.mnemonic not in ('nop', 'int3')]
        def is_seh(i):
            return (i.mnemonic == 'mov' and i.operands[0].type == X.X86_OP_MEM and i.operands[1].type == X.X86_OP_IMM
                    and i.operands[0].mem.base and i.reg_name(i.operands[0].mem.base) == 'esp' and not i.operands[0].mem.index)
        # a register zeroed once (xor r,r) stays zero through these destructors and the compiler places the zeroing
        # anywhere; hoist them out so the rigid pattern matchers see the null-check right after the member load.
        zeros = set()
        for i in xs0:
            if i.mnemonic == 'xor' and i.operands[0].type == X.X86_OP_REG and i.operands[1].type == X.X86_OP_REG \
                    and i.operands[0].reg == i.operands[1].reg:
                zeros.add(i.reg_name(i.operands[0].reg))
        xs = [i for i in xs0 if not is_seh(i) and not (i.mnemonic == 'xor' and i.operands[0].type == X.X86_OP_REG
              and i.operands[1].type == X.X86_OP_REG and i.operands[0].reg == i.operands[1].reg)]
        Ln = len(xs); used = [False] * Ln; regoff = {}; ops = []; nmembers = 0; ndirect = 0; args = False; k = 0
        def is_null_check(ins, reg):   # test reg,reg  or  cmp reg, <a register known to be zero>
            if ins.mnemonic == 'test' and ins.op_str == f'{reg}, {reg}':
                return True
            return (ins.mnemonic == 'cmp' and ins.operands[0].type == X.X86_OP_REG and ins.reg_name(ins.operands[0].reg) == reg
                    and ins.operands[1].type == X.X86_OP_REG and ins.reg_name(ins.operands[1].reg) in zeros)
        def is_clear(ins, off):   # mov [esi+off], 0  or  mov [esi+off], <zero register>
            if ins.mnemonic != 'mov' or ins.operands[0].type != X.X86_OP_MEM:
                return False
            d = ins.operands[0].mem
            if not (d.base and ins.reg_name(d.base) == 'esi' and not d.index and (d.disp & 0xffffffff) == off):
                return False
            s = ins.operands[1]
            return (s.type == X.X86_OP_IMM and (s.imm & 0xffffffff) == 0) or (s.type == X.X86_OP_REG and ins.reg_name(s.reg) in zeros)
        # regoff maps a register to its offset within this (esi); ops is the exact order of field stores
        # (0, offset, value), member deletes (1, offset, 0) and direct destructor calls (2, addr, this-offset).
        # Register saves happen in the prologue (before the first op); any standalone push after that is a call
        # argument, so the function is not a pure stores/deletes/thiscall-destructors sequence and we reject it.
        while k < Ln:
            i = xs[k]; m = i.mnemonic; op = i.op_str
            if m == 'lea' and i.operands[1].type == X.X86_OP_MEM:
                me = i.operands[1].mem; b = i.reg_name(me.base) if me.base else None
                if b == 'esi' and not me.index:
                    regoff[i.reg_name(i.operands[0].reg)] = me.disp & 0xffffffff; used[k] = True; k += 1; continue
            if m == 'mov' and i.operands[0].type == X.X86_OP_REG and i.operands[1].type == X.X86_OP_REG:
                r0 = i.reg_name(i.operands[0].reg); r1 = i.reg_name(i.operands[1].reg)
                if r1 == 'esi' or r1 in regoff:   # copy a this-pointer between registers
                    regoff[r0] = 0 if r1 == 'esi' else regoff[r1]; used[k] = True; k += 1; continue
            if m == 'mov' and i.operands[0].type == X.X86_OP_MEM and i.operands[1].type == X.X86_OP_IMM:
                d = i.operands[0].mem; v = i.operands[1].imm & 0xffffffff; b = i.reg_name(d.base) if d.base else None
                if not d.index and (b == 'esi' or b in regoff):   # a field store to this (any value, tables included)
                    ops.append((0, (d.disp & 0xffffffff) + (0 if b == 'esi' else regoff[b]), v)); used[k] = True; k += 1; continue
            # release a refcounted member: mov ecx,[esi+off]; null-check; je L; dec word[ecx+rc]; cmp word[ecx+rc],0;
            # jne CLR; mov e?x,[ecx]; [push 1;] call [e?x+slot]; CLR: clear [esi+off]; L: ...  The call is either the
            # scalar deleting destructor (slot 0, push 1 = free) or a plain destructor (slot 4, no flag).
            if (m == 'mov' and op.startswith('ecx, dword ptr [esi') and i.operands[1].type == X.X86_OP_MEM
                    and not i.operands[1].mem.index and k + 8 < Ln
                    and is_null_check(xs[k+1], 'ecx') and xs[k+2].mnemonic == 'je'
                    and xs[k+3].mnemonic == 'dec' and xs[k+3].operands[0].type == X.X86_OP_MEM
                    and xs[k+3].operands[0].size == 2 and xs[k+3].operands[0].mem.base
                    and i.reg_name(xs[k+3].operands[0].mem.base) == 'ecx' and not xs[k+3].operands[0].mem.index
                    and xs[k+4].mnemonic == 'cmp' and xs[k+4].operands[0].type == X.X86_OP_MEM
                    and xs[k+4].operands[0].size == 2 and xs[k+4].operands[0].mem.base
                    and i.reg_name(xs[k+4].operands[0].mem.base) == 'ecx'
                    and (xs[k+4].operands[0].mem.disp & 0xffffffff) == (xs[k+3].operands[0].mem.disp & 0xffffffff)
                    and xs[k+4].operands[1].type == X.X86_OP_IMM and (xs[k+4].operands[1].imm & 0xffffffff) == 0
                    and xs[k+5].mnemonic == 'jne'
                    and xs[k+6].op_str in ('eax, dword ptr [ecx]', 'edx, dword ptr [ecx]')):
                vreg = xs[k+6].op_str[:3]   # eax or edx
                flag1 = xs[k+7].mnemonic == 'push' and xs[k+7].op_str == '1'
                ci = k + 8 if flag1 else k + 7                 # the call
                cl = ci + 1                                    # the clear
                call = xs[ci] if ci < Ln else None
                if (call is not None and cl < Ln and call.mnemonic == 'call' and call.operands[0].type == X.X86_OP_MEM
                        and call.operands[0].mem.base and i.reg_name(call.operands[0].mem.base) == vreg
                        and not call.operands[0].mem.index and cl + 1 <= Ln
                        and is_clear(xs[cl], i.operands[1].mem.disp & 0xffffffff)
                        and xs[k+5].operands[0].imm == xs[cl].address and xs[k+2].operands[0].imm == xs[cl+1].address):
                    slot = call.operands[0].mem.disp & 0xffffffff
                    if (slot, flag1) in ((0, True), (4, False)):   # (scalar deleting, free) or (plain destructor)
                        rc = xs[k+3].operands[0].mem.disp & 0xffffffff
                        ops.append((4, i.operands[1].mem.disp & 0xffffffff, rc | (slot << 16) | ((1 if flag1 else 0) << 24)))
                        nmembers += 1
                        for j in range(k, cl + 1): used[j] = True
                        for rr in ('ecx', 'eax', 'edx'): regoff.pop(rr, None)
                        k = cl + 1; continue
            if (m == 'mov' and op.startswith('ecx, dword ptr [esi') and k + 5 < Ln
                    and xs[k+1].mnemonic == 'test' and xs[k+1].op_str == 'ecx, ecx' and xs[k+2].mnemonic == 'je'
                    and xs[k+3].op_str in ('eax, dword ptr [ecx]', 'edx, dword ptr [ecx]')
                    and xs[k+4].mnemonic == 'push' and xs[k+4].op_str == '1'
                    and xs[k+5].op_str in ('dword ptr [eax]', 'dword ptr [edx]')
                    and xs[k+2].operands[0].imm == xs[k+6].address):
                ops.append((1, i.operands[1].mem.disp & 0xffffffff, 0)); nmembers += 1
                for j in range(k, k+6): used[j] = True
                for rr in ('ecx', 'eax', 'edx'): regoff.pop(rr, None)   # clobbered by the delete
                k += 6; continue
            # free a pointer member: mov r,[esi+off]; test r,r; je L; push r; call DELETE (__cdecl); add esp,4;
            # mov [esi+off],0; where L is the instruction after the clear (the whole thing runs only when non-null)
            if (m == 'mov' and i.operands[0].type == X.X86_OP_REG and i.operands[1].type == X.X86_OP_MEM and k + 7 < Ln):
                d = i.operands[1].mem; r0 = i.reg_name(i.operands[0].reg); b = i.reg_name(d.base) if d.base else None
                off = d.disp & 0xffffffff
                if (b == 'esi' and not d.index and xs[k+1].mnemonic == 'test' and xs[k+1].op_str == f'{r0}, {r0}'
                        and xs[k+2].mnemonic == 'je' and xs[k+3].mnemonic == 'push' and xs[k+3].op_str == r0
                        and xs[k+4].mnemonic == 'call' and xs[k+4].operands[0].type == X.X86_OP_IMM
                        and xs[k+5].mnemonic == 'add' and xs[k+5].op_str == 'esp, 4'
                        and xs[k+6].mnemonic == 'mov' and xs[k+6].operands[0].type == X.X86_OP_MEM
                        and xs[k+6].operands[1].type == X.X86_OP_IMM and (xs[k+6].operands[1].imm & 0xffffffff) == 0
                        and xs[k+6].operands[0].mem.base and i.reg_name(xs[k+6].operands[0].mem.base) == 'esi'
                        and not xs[k+6].operands[0].mem.index and (xs[k+6].operands[0].mem.disp & 0xffffffff) == off
                        and xs[k+2].operands[0].imm == xs[k+7].address):
                    ops.append((3, off, xs[k+4].operands[0].imm & 0xffffffff)); nmembers += 1
                    for j in range(k, k+7): used[j] = True
                    k += 7; continue
            if m == 'call' and i.operands[0].type == X.X86_OP_IMM:
                # a direct thiscall destructor: ecx is a known this-offset and no argument was pushed for it
                if 'ecx' in regoff and not args:
                    ops.append((2, i.operands[0].imm & 0xffffffff, regoff['ecx'])); ndirect += 1
                    for rr in ('ecx', 'eax', 'edx'): regoff.pop(rr, None)
                    used[k] = True; k += 1; continue
                return None   # a call we cannot replay exactly (takes args, or unknown this)
            if m == 'push' and ops:
                args = True   # an argument push after the first op: not a pure destructor
            k += 1
        # any leftover store to this (a reg-sourced one we can't replay) makes the rewrite inexact: reject it
        for j in range(Ln):
            if used[j]:
                continue
            ins = xs[j]
            if ins.mnemonic not in ('push', 'pop', 'sub', 'add', 'ret', 'xor', 'jmp', 'lea', 'mov'):
                return None
            if ins.mnemonic == 'mov' and ins.operands[0].type == X.X86_OP_MEM:
                me = ins.operands[0].mem; b = ins.reg_name(me.base) if me.base else None
                if b == 'esi' or b in regoff:
                    return None
        thiscalls, cdeclcalls = [], []   # op-2 targets (this in ecx) and op-3 targets (operator delete, __cdecl)
        for t in ops:
            if t[0] == 2 and t[1] not in thiscalls:
                thiscalls.append(t[1])
            if t[0] == 3 and t[2] not in cdeclcalls:
                cdeclcalls.append(t[2])
        if nmembers >= 2 and len(thiscalls) <= 3 and len(cdeclcalls) <= 3:
            return ops, thiscalls, cdeclcalls
        return None

    for r in funcs:
        va, n = int(r['address'], 16), int(r['size'])
        if n < 50 or n > 400 or va in handwritten or va in emitted:
            continue
        res = parse_md(va, n)
        if not res:
            continue
        ops, thiscalls, cdeclcalls = res
        data = [x for t in ops for x in t]
        t3 = (thiscalls + [0, 0, 0])[:3]   # thiscall destructor targets, and operator-delete targets, for the test
        f3 = (cdeclcalls + [0, 0, 0])[:3]
        first = ', '.join(f'0x{b:02x}' for b in mem[va - BASE:va - BASE + 8])
        out.append(f'MEMBER_DTOR({va:#x}, {t3[0]:#x}, {t3[1]:#x}, {t3[2]:#x}, {f3[0]:#x}, {f3[1]:#x}, {f3[2]:#x}, '
                   f'{first}, ' + ', '.join(f'{x:#x}' for x in data) + ')')
        n_md += 1
    counts['MEMBER_DTOR'] = n_md

    # Object::GetRef: a 29-byte get-and-addref accessor `*out = this->[OFF]; ++*(uint32_t*)this->[OFF]; return out`.
    # Shape-matched (so both disp8 and disp32 offset encodings are caught): push ecx; mov ecx,[ecx+OFF]; mov eax,
    # [esp+8]; mov [eax],ecx; mov edx,[ecx]; inc edx; mov [esp],0; mov [ecx],edx; pop ecx; ret 4.  Only OFF varies.
    gr_mn = ['push', 'mov', 'mov', 'mov', 'mov', 'inc', 'mov', 'mov', 'pop', 'ret']
    gr_fixed = {2: 'eax, dword ptr [esp + 8]', 3: 'dword ptr [eax], ecx', 4: 'edx, dword ptr [ecx]',
                5: 'edx', 6: 'dword ptr [esp], 0', 7: 'dword ptr [ecx], edx', 8: 'ecx', 9: '4'}
    n_gr = 0
    for r in funcs:
        va, n = int(r['address'], 16), int(r['size'])
        if n < 20 or n > 40 or va in handwritten or va in emitted:
            continue
        xs = list(md.disasm(mem[va - BASE:va - BASE + n], va))
        if [i.mnemonic for i in xs] != gr_mn or xs[0].op_str != 'ecx':
            continue
        o = xs[1].operands
        if not (xs[1].op_str.startswith('ecx, dword ptr [ecx') and o[1].type == X.X86_OP_MEM and not o[1].mem.index):
            continue
        if any(xs[k].op_str != v for k, v in gr_fixed.items()):
            continue
        off = o[1].mem.disp & 0xffffffff
        first = ', '.join(f'0x{x:02x}' for x in mem[va - BASE:va - BASE + 8])
        out.append(f'GETREF({va:#x}, {off:#x}, {first})')
        n_gr += 1
    counts['GETREF'] = n_gr

    # Object::SetFields: set some fields of this to constants and return this (no calls, no branches, ret 0). The
    # body is only prologue/epilogue reg saves, this propagation between registers, and `mov [this+off], imm`.
    def parse_setfields(va, n):
        xs = [i for i in md.disasm(mem[va - BASE:va - BASE + n], va) if i.mnemonic not in ('nop', 'int3')]
        def is_seh(i):
            return (i.mnemonic == 'mov' and i.operands[0].type == X.X86_OP_MEM and i.operands[1].type == X.X86_OP_IMM
                    and i.operands[0].mem.base and i.reg_name(i.operands[0].mem.base) == 'esp' and not i.operands[0].mem.index)
        xs = [i for i in xs if not is_seh(i)]
        if not xs or xs[-1].mnemonic != 'ret' or (xs[-1].operands and (xs[-1].operands[0].imm & 0xffffffff) != 0):
            return None
        thisreg = {'ecx'}; pairs = []; retthis = False
        for i in xs[:-1]:
            m = i.mnemonic
            if m == 'push' and i.operands[0].type == X.X86_OP_REG:
                continue
            if m == 'pop':
                continue
            if m == 'mov' and i.operands[0].type == X.X86_OP_REG and i.operands[1].type == X.X86_OP_REG:
                r0 = i.reg_name(i.operands[0].reg); r1 = i.reg_name(i.operands[1].reg)
                if r0 == 'eax' and r1 in thisreg:
                    retthis = True
                if r1 in thisreg:
                    thisreg.add(r0)
                continue
            if m == 'mov' and i.operands[0].type == X.X86_OP_MEM and i.operands[1].type == X.X86_OP_IMM:
                d = i.operands[0].mem; b = i.reg_name(d.base) if d.base else None
                if b in thisreg and not d.index:
                    pairs.append((d.disp & 0xffffffff, i.operands[1].imm & 0xffffffff, i.operands[0].size)); continue
            return None
        if retthis and pairs:
            return pairs
        return None

    n_sf = 0
    for r in funcs:
        va, n = int(r['address'], 16), int(r['size'])
        if n < 8 or n > 120 or va in handwritten or va in emitted:
            continue
        pairs = parse_setfields(va, n)
        if not pairs:
            continue
        first = ', '.join(f'0x{x:02x}' for x in mem[va - BASE:va - BASE + 8])
        data = ', '.join(f'{x:#x}' for p in pairs for x in p)
        out.append(f'SETFIELDS({va:#x}, {first}, {data})')
        n_sf += 1
    counts['SETFIELDS'] = n_sf

    # Object::FreeField: if this->[OFF] non-null, operator delete (0xbce010) it, then null the field.
    ffref = [i.mnemonic for i in md.disasm(mem[0x42ff30 - BASE:0x42ff30 - BASE + 28], 0x42ff30)]
    n_ff = 0
    for r in funcs:
        va, n = int(r['address'], 16), int(r['size'])
        if n != 28 or va in handwritten or va in emitted:
            continue
        xs = list(md.disasm(mem[va - BASE:va - BASE + n], va))
        if [i.mnemonic for i in xs] != ffref:
            continue
        load = xs[2]; clear = xs[8]; call = next((i for i in xs if i.mnemonic == 'call'), None)
        if (load.operands[1].type != X.X86_OP_MEM or not load.operands[1].mem.base
                or md.reg_name(load.operands[1].mem.base) != 'esi' or load.operands[1].mem.index
                or clear.operands[0].type != X.X86_OP_MEM
                or (clear.operands[0].mem.disp & 0xffffffff) != (load.operands[1].mem.disp & 0xffffffff)
                or call is None or call.operands[0].type != X.X86_OP_IMM or (call.operands[0].imm & 0xffffffff) != 0xbce010):
            continue
        off = load.operands[1].mem.disp & 0xffffffff
        first = ', '.join(f'0x{x:02x}' for x in mem[va - BASE:va - BASE + 8])
        out.append(f'FREEFIELD({va:#x}, {off:#x}, {first})')
        n_ff += 1
    counts['FREEFIELD'] = n_ff

    # Destructor::TablesThenBase: K stores `mov [ecx+off],imm`, then `jmp base`. A destructor thunk that sets some
    # vtable/table fields then tail-calls the (void, this-only) base destructor. K any count.
    n_ttb = 0
    for r in funcs:
        va, n = int(r['address'], 16), int(r['size'])
        if n < 13 or n > 80 or va in handwritten or va in emitted:
            continue
        xs = list(md.disasm(mem[va - BASE:va - BASE + n], va))
        if len(xs) < 2 or xs[-1].mnemonic != 'jmp' or xs[-1].operands[0].type != X.X86_OP_IMM:
            continue
        pairs = []
        okk = True
        for s in xs[:-1]:
            if (s.mnemonic == 'mov' and s.operands[0].type == X.X86_OP_MEM and s.operands[0].mem.base
                    and md.reg_name(s.operands[0].mem.base) == 'ecx' and not s.operands[0].mem.index
                    and s.operands[1].type == X.X86_OP_IMM and s.operands[0].size == 4):
                pairs.append((s.operands[0].mem.disp & 0xffffffff, s.operands[1].imm & 0xffffffff))
            else:
                okk = False; break
        if not okk or len(pairs) < 2:
            continue
        base = xs[-1].operands[0].imm & 0xffffffff
        first = ', '.join(f'0x{x:02x}' for x in mem[va - BASE:va - BASE + 8])
        data = ', '.join(f'{x:#x}' for p in pairs for x in p)
        out.append(f'TABLES_THEN_BASE({va:#x}, {base:#x}, {first}, {data})')
        n_ttb += 1
    counts['TABLES_THEN_BASE'] = n_ttb

    # DeletingDestructor (member-free variant): push esi; mov esi,ecx; call DTOR; test [esp+8],1; je; mov ecx,esi;
    # call FREE; mov eax,esi; pop esi; ret 4.  dtor(this); if(flag&1) free(this); return this.  DTOR/FREE thiscall.
    d2mn = [i.mnemonic for i in md.disasm(mem[0xbc0e50 - BASE:0xbc0e50 - BASE + 28], 0xbc0e50)]
    n_d2 = 0
    for r in funcs:
        va, n = int(r['address'], 16), int(r['size'])
        if n != 28 or va in handwritten or va in emitted:
            continue
        xs = list(md.disasm(mem[va - BASE:va - BASE + n], va))
        if [i.mnemonic for i in xs] != d2mn:
            continue
        calls = [i for i in xs if i.mnemonic == 'call' and i.operands[0].type == X.X86_OP_IMM]
        if len(calls) != 2:
            continue
        dtor = calls[0].operands[0].imm & 0xffffffff
        free = calls[1].operands[0].imm & 0xffffffff
        first = ', '.join(f'0x{x:02x}' for x in mem[va - BASE:va - BASE + 8])
        out.append(f'DELETING2({va:#x}, {dtor:#x}, {free:#x}, {first})')
        n_d2 += 1
    counts['DELETING2'] = n_d2

    # Object::Forward2: forward both args to a member method (thiscall) and return this. Byte-identical but the call.
    f2ref = mem[0x66a350 - BASE:0x66a350 - BASE + 26]
    f2mn = [i.mnemonic for i in md.disasm(f2ref, 0x66a350)]
    n_f2 = 0
    for r in funcs:
        va, n = int(r['address'], 16), int(r['size'])
        if n != 26 or va in handwritten or va in emitted:
            continue
        b = mem[va - BASE:va - BASE + n]
        xs = list(md.disasm(b, va))
        if [i.mnemonic for i in xs] != f2mn:
            continue
        ci = next(j for j, i in enumerate(xs) if i.mnemonic == 'call')
        if xs[ci].operands[0].type != X.X86_OP_IMM:
            continue
        ca = xs[ci].address - va; cl = xs[ci].size
        masked = bytearray(b); refm = bytearray(f2ref)
        for k in range(ca + cl - 4, ca + cl):
            masked[k] = 0; refm[k] = 0
        if masked != refm:
            continue
        method = xs[ci].operands[0].imm & 0xffffffff
        first = ', '.join(f'0x{x:02x}' for x in b[:8])
        out.append(f'FORWARD2({va:#x}, {method:#x}, {first})')
        n_f2 += 1
    counts['FORWARD2'] = n_f2

    # Object::CountList: count a null-terminated this->[0]-linked chain. All copies byte-identical (no parameters).
    clref = mem[0x5908f0 - BASE:0x5908f0 - BASE + 30]
    n_cl = 0
    for r in funcs:
        va, n = int(r['address'], 16), int(r['size'])
        if n != 30 or va in handwritten or va in emitted:
            continue
        if mem[va - BASE:va - BASE + 30] != clref:
            continue
        first = ', '.join(f'0x{x:02x}' for x in mem[va - BASE:va - BASE + 8])
        out.append(f'COUNT_LIST({va:#x}, {first})')
        n_cl += 1
    counts['COUNT_LIST'] = n_cl

    # Object::CallThenTail: push esi; mov esi,ecx; call A; mov ecx,esi; pop esi; jmp B.  A(this); tail-call B(this).
    cttmn = [i.mnemonic for i in md.disasm(mem[0x530130 - BASE:0x530130 - BASE + 16], 0x530130)]
    n_ctt = 0
    for r in funcs:
        va, n = int(r['address'], 16), int(r['size'])
        if n != 16 or va in handwritten or va in emitted:
            continue
        xs = list(md.disasm(mem[va - BASE:va - BASE + n], va))
        if [i.mnemonic for i in xs] != cttmn:
            continue
        ca, jm = xs[2], xs[5]
        if not (ca.mnemonic == 'call' and ca.operands[0].type == X.X86_OP_IMM and jm.mnemonic == 'jmp' and jm.operands[0].type == X.X86_OP_IMM):
            continue
        a_t = ca.operands[0].imm & 0xffffffff
        b_t = jm.operands[0].imm & 0xffffffff
        first = ', '.join(f'0x{x:02x}' for x in mem[va - BASE:va - BASE + 8])
        out.append(f'CALL_THEN_TAIL({va:#x}, {a_t:#x}, {b_t:#x}, {first})')
        n_ctt += 1
    counts['CALL_THEN_TAIL'] = n_ctt

    # Object::DbgSingleton: push ebp; mov ebp,esp; mov ecx,GLOBAL; call METHOD; cmp ebp,esp; call __RTC_CheckEsp;
    # pop ebp; ret.  method(global); the checked-build esp assertion is omitted.
    dsmn = [i.mnemonic for i in md.disasm(mem[0xcc17f0 - BASE:0xcc17f0 - BASE + 22], 0xcc17f0)]
    n_ds = 0
    for r in funcs:
        va, n = int(r['address'], 16), int(r['size'])
        if n != 22 or va in handwritten or va in emitted:
            continue
        xs = list(md.disasm(mem[va - BASE:va - BASE + n], va))
        if [i.mnemonic for i in xs] != dsmn:
            continue
        movg, c1, c2 = xs[2], xs[3], xs[5]
        if not (movg.mnemonic == 'mov' and movg.operands[0].type == X.X86_OP_REG and md.reg_name(movg.operands[0].reg) == 'ecx'
                and movg.operands[1].type == X.X86_OP_IMM and c1.mnemonic == 'call' and c1.operands[0].type == X.X86_OP_IMM
                and c2.mnemonic == 'call' and c2.operands[0].type == X.X86_OP_IMM and (c2.operands[0].imm & 0xffffffff) == 0xde120d):
            continue
        gaddr = movg.operands[1].imm & 0xffffffff
        method = c1.operands[0].imm & 0xffffffff
        first = ', '.join(f'0x{x:02x}' for x in mem[va - BASE:va - BASE + 8])
        out.append(f"DBG_SINGLETON({va:#x}, {gaddr:#x}, {method:#x}, {first})")
        n_ds += 1
    counts['DBG_SINGLETON'] = n_ds

    # Object::GetField: a (checked-build) getter returning this->[OFF]. Shape of 0xcc2610; OFF at the mov eax,[eax+OFF].
    gfmn = [i.mnemonic for i in md.disasm(mem[0xcc2610 - BASE:0xcc2610 - BASE + 24], 0xcc2610)]
    n_gf = 0
    for r in funcs:
        va, n = int(r['address'], 16), int(r['size'])
        if n != 24 or va in handwritten or va in emitted:
            continue
        xs = list(md.disasm(mem[va - BASE:va - BASE + n], va))
        if [i.mnemonic for i in xs] != gfmn:
            continue
        g = xs[6]
        if not (g.mnemonic == 'mov' and g.operands[0].type == X.X86_OP_REG and md.reg_name(g.operands[0].reg) == 'eax'
                and g.operands[1].type == X.X86_OP_MEM and g.operands[1].mem.base and md.reg_name(g.operands[1].mem.base) == 'eax'
                and not g.operands[1].mem.index):
            continue
        off = g.operands[1].mem.disp & 0xffffffff
        first = ', '.join(f'0x{x:02x}' for x in mem[va - BASE:va - BASE + 8])
        out.append(f'GETFIELD({va:#x}, {off:#x}, {first})')
        n_gf += 1
    counts['GETFIELD'] = n_gf

    # Object::GlobalMemberCall: test ecx,ecx; je; push ecx; mov ecx,[GLOBAL]; add ecx,4; call METHOD; ret.
    # if(this) method(*GLOBAL + 4, this).
    gmcmn = [i.mnemonic for i in md.disasm(mem[0xc0a840 - BASE:0xc0a840 - BASE + 20], 0xc0a840)]
    n_gmc = 0
    for r in funcs:
        va, n = int(r['address'], 16), int(r['size'])
        if n != 20 or va in handwritten or va in emitted:
            continue
        xs = list(md.disasm(mem[va - BASE:va - BASE + n], va))
        if [i.mnemonic for i in xs] != gmcmn:
            continue
        movg, addi, call = xs[3], xs[4], xs[5]
        if not (movg.mnemonic == 'mov' and movg.operands[1].type == X.X86_OP_MEM and not movg.operands[1].mem.base
                and addi.mnemonic == 'add' and addi.operands[1].type == X.X86_OP_IMM and (addi.operands[1].imm & 0xffffffff) == 4
                and call.mnemonic == 'call' and call.operands[0].type == X.X86_OP_IMM):
            continue
        gaddr2 = movg.operands[1].mem.disp & 0xffffffff
        method = call.operands[0].imm & 0xffffffff
        first = ', '.join(f'0x{x:02x}' for x in mem[va - BASE:va - BASE + 8])
        out.append(f'GLOBAL_MEMBER_CALL({va:#x}, {gaddr2:#x}, {method:#x}, {first})')
        n_gmc += 1
    counts['GLOBAL_MEMBER_CALL'] = n_gmc

    # Object::CondForward: byte-identical but the call target. if(this->[0xc]>=0) method(this->[0], this->[0xc], this->[8]).
    cfref = mem[0x851c70 - BASE:0x851c70 - BASE + 20]
    cfmn = [i.mnemonic for i in md.disasm(cfref, 0x851c70)]
    n_cf = 0
    for r in funcs:
        va, n = int(r['address'], 16), int(r['size'])
        if n != 20 or va in handwritten or va in emitted:
            continue
        b = mem[va - BASE:va - BASE + n]
        xs = list(md.disasm(b, va))
        if [i.mnemonic for i in xs] != cfmn or xs[7].mnemonic != 'call' or xs[7].operands[0].type != X.X86_OP_IMM:
            continue
        ca = xs[7].address - va; cl = xs[7].size
        m = bytearray(b); rf = bytearray(cfref)
        for k in range(ca + cl - 4, ca + cl):
            m[k] = 0; rf[k] = 0
        if m != rf:
            continue
        method = xs[7].operands[0].imm & 0xffffffff
        first = ', '.join(f'0x{x:02x}' for x in b[:8])
        out.append(f'COND_FORWARD({va:#x}, {method:#x}, {first})')
        n_cf += 1
    counts['COND_FORWARD'] = n_cf

    print(', '.join(f'{n} {m}' for m, n in counts.items()))
    open(os.path.join(HERE, 'forever', 'src', 'rewrite', 'generated.inc'), 'w', newline='\n').write('\n'.join(out) + '\n')
    print(n_del, 'deleting destructors,', n_q, 'interface queries,', n_m, 'module globals,', n_c, 'cached interfaces,', n_l, 'struct array loads,', n_g, 'global clean-ups,', n_rn + n_rt,
          'reference constructors,', n_af, 'object array frees,', n_st, 'table setters,',
          n_sn, 'start-up names,', n_sc, 'class descriptions,', n_ss, 'sections')


if __name__ == '__main__':
    main()
