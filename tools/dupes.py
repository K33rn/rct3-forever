"""Find functions in RCT3plus.exe that are exact copies of each other.

usage: python tools/dupes.py "<game folder>\\RCT3plus.exe"            the biggest groups of copies still to do
       python tools/dupes.py "<game folder>\\RCT3plus.exe" 0x410960   every copy of that function, as a C array

The compiler made a separate copy of a template function for each type that uses it, so many functions exist dozens
or hundreds of times, byte for byte the same apart from where their calls and jumps point (which are compared by
target). One rewrite can replace every copy (REWRITE_COPIES). Reads the function list from docs/functions.csv and
the code from your own exe; needs pefile and capstone (uv run --with pefile --with capstone).
"""
import collections
import csv
import os
import sys

import capstone
import pefile

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def groups(exe, statuses=('to do',)):
    pe = pefile.PE(exe)
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    out = collections.defaultdict(list)
    for r in csv.DictReader(open(os.path.join(HERE, 'docs', 'functions.csv'))):
        if r['status'] not in statuses:
            continue
        va, n = int(r['address'], 16), int(r['size'])
        if n < 6 or n > 600:
            continue
        key = []
        for ins in md.disasm(pe.get_data(va - pe.OPTIONAL_HEADER.ImageBase, n), va):
            if ins.mnemonic == 'call' or ins.mnemonic.startswith('j'):
                op = ins.op_str
                if op.startswith('0x'):
                    t = int(op, 16)
                    op = 'here+%x' % (t - va) if va <= t < va + n else op
                key.append(ins.mnemonic + ' ' + op)
            else:
                key.append(ins.bytes.hex())
        out['|'.join(key)].append((va, n, int(r['callers'])))
    return out


def main():
    exe = sys.argv[1]
    if len(sys.argv) > 2:
        want = int(sys.argv[2], 16)
        for g in groups(exe, ('to do', 'rewritten', 'documented')).values():
            if any(va == want for va, _, _ in g):
                addrs = sorted(va for va, _, _ in g)
                print(f'// {len(addrs)} copies, {g[0][1]} bytes each')
                for i in range(0, len(addrs), 8):
                    print('    ' + ', '.join(f'0x{a:06x}' for a in addrs[i:i + 8]) + ',')
                return
        sys.exit('not found')
    big = sorted((g for g in groups(exe).values() if len(g) >= 3), key=lambda g: -len(g) * g[0][1])
    for g in big[:40]:
        print(f'{len(g):4} copies x {g[0][1]:4} bytes, called from {sum(x[2] for x in g):6} places, e.g. ' +
              ' '.join(hex(x[0]) for x in g[:4]))
    print(len(big), 'groups of 3 or more copies,', sum(len(g) for g in big), 'functions')


if __name__ == '__main__':
    main()
