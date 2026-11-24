"""Which rewritten game functions leave ecx and/or edx as they were, and how many argument bytes each takes off the stack.

The game was built with whole-program optimisation: where its compiler knew a function doesn't touch ecx or edx, the
callers keep values in them across the call (0x7CE52D keeps ecx across Value::Band2, for one). A rewrite built the
usual way may change both, so forever.dll puts these rewrites behind a small stub that keeps them
(rewrite::callable in forever/src/rewrite/rewrite.cpp). tools/land.py runs this with the other generated files.

A function counts as keeping a register when none of its instructions write it and every function it calls directly
keeps it too (an indirect call, an import or a call into the unknown counts as writing both). Only claims that are sure
are made: keeping a register the original did change would break a value given back in edx.

usage: kept_regs.py          (writes forever/src/rewrite/kept_regs.inc from your own RCT3plus.exe)
Only addresses and numbers go in the file: none of the game's code."""
import csv, os, sys
import capstone, pefile
from capstone import x86
from gamepath import HERE, game_exe

BASE = 0x400000
pe = pefile.PE(game_exe(), fast_load=True)
img = pe.get_memory_mapped_image()
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
md.detail = True

rows = list(csv.DictReader(open(os.path.join(HERE, 'docs', 'functions.csv'), encoding='utf-8')))
size = {int(r['address'], 16): int(r['size']) for r in rows}
rewritten = sorted(int(r['address'], 16) for r in rows if r['status'] == 'rewritten')

ECX = {x86.X86_REG_ECX, x86.X86_REG_CX, x86.X86_REG_CL, x86.X86_REG_CH}
EDX = {x86.X86_REG_EDX, x86.X86_REG_DX, x86.X86_REG_DL, x86.X86_REG_DH}
BOTH = 3   # bit 0: writes ecx, bit 1: writes edx
UNKNOWN = None


def instructions(at):
    n = size.get(at)
    if not n:
        return None
    return list(md.disasm(bytes(img[at - BASE:at - BASE + n]), at))


writes_memo, busy = {}, set()


def writes(at):
    """The registers (bits) a function may write, its callees included."""
    if at in writes_memo:
        return writes_memo[at]
    if at in busy:
        return BOTH   # a cycle: no claim
    ins = instructions(at)
    if ins is None:
        return BOTH
    busy.add(at)
    w = 0
    end = at + size[at]
    for i in ins:
        if i.group(capstone.CS_GRP_CALL) or (i.mnemonic == 'jmp' and i.operands and i.operands[0].type == x86.X86_OP_IMM
                                              and not at <= i.operands[0].imm < end):
            op = i.operands[0] if i.operands else None
            if op is not None and op.type == x86.X86_OP_IMM:
                w |= writes(op.imm)
            else:
                w = BOTH
        elif i.mnemonic == 'jmp' and i.operands and i.operands[0].type != x86.X86_OP_IMM:
            w = BOTH   # a jump through a table or register: may leave the function
        try:
            _, written = i.regs_access()
        except capstone.CsError:
            w = BOTH
            written = ()
        if any(r in ECX for r in written):
            w |= 1
        if any(r in EDX for r in written):
            w |= 2
        if w == BOTH:
            break
    busy.discard(at)
    writes_memo[at] = w
    return w


def popped(at, seen=()):
    """The argument bytes a function takes off the stack (its ret's number), or None when it isn't clear."""
    ins = instructions(at)
    if ins is None or at in seen:
        return None
    found = set()
    end = at + size[at]
    for i in ins:
        if i.mnemonic == 'ret':
            found.add(i.operands[0].imm if i.operands else 0)
        elif i.mnemonic == 'jmp' and i.operands and i.operands[0].type == x86.X86_OP_IMM and not at <= i.operands[0].imm < end:
            p = popped(i.operands[0].imm, seen + (at,))   # a tail call: what that one takes
            if p is None:
                return None
            found.add(p)
    return found.pop() if len(found) == 1 else None


lines, kept, unclear = [], 0, 0
for at in rewritten:
    keep = BOTH & ~writes(at)
    if not keep:
        continue
    n = popped(at)
    if n is None or n % 4 or n > 64:
        unclear += 1
        continue
    kept += 1
    lines.append(f'{{{at:#08x}, {keep}, {n}}},')
out = os.path.join(HERE, 'forever', 'src', 'rewrite', 'kept_regs.inc')
with open(out, 'w', encoding='utf-8', newline='\n') as f:
    f.write('// Made by tools/kept_regs.py: {address, registers the original keeps (1 ecx, 2 edx), argument bytes it pops}\n')
    f.write('\n'.join(lines) + '\n')
print(f'{len(rewritten)} rewritten; {kept} keep ecx or edx ({unclear} more left out: their argument bytes are unclear)')
print(f'wrote {out}')
