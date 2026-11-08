"""Disassemble game functions from your own RCT3plus.exe, to study what they do before rewriting them.
usage: disasm.py ADDR [ADDR ...]   (hex addresses, e.g. 0x445440)
A function's size comes from docs/functions.csv; an address that isn't a function start gets 64 bytes.
This prints the game's instructions for reading only: never paste them, or a decompiler's output, into the project."""
import csv, os, sys
import capstone, pefile
from gamepath import HERE, game_exe

BASE = 0x400000
pe = pefile.PE(game_exe(), fast_load=True)
img = pe.get_memory_mapped_image()
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
rows = {int(r['address'], 16): r for r in csv.DictReader(open(os.path.join(HERE, 'docs', 'functions.csv'), encoding='utf-8'))}
for arg in sys.argv[1:]:
    a = int(arg, 16)
    r = rows.get(a)
    n = int(r['size']) if r else 64
    print(f'== {a:#x} ({n} bytes)' + (f' {r["name"]}, {r["status"]}' if r else ''))
    for i in md.disasm(bytes(img[a - BASE:a - BASE + n]), a):
        print(f'  {i.address:#x}  {i.mnemonic} {i.op_str}')
