"""RCT3 Forever progress: which of RCT3plus.exe's functions are rewritten, documented or still to do.

usage: python tools/progress.py [functions.csv from the local Ghidra export]
With a path, the function list (address, size, Ghidra's name for library functions, callers) is refreshed into
docs/functions.csv first; the decompiled code itself never comes into the repo. Then docs/functions.csv gets each
function's status, docs/progress.md is rewritten and the README's progress section updated.

Status:
  rewritten   a REWRITE(...) in forever/src/rewrite/*.cpp replaces it
  documented  understood, not rewritten yet: described in a table row of docs/notes/*.md (the address in the row's
              first column), or named in docs/exe-map.md or docs/research-physics-and-guests.md
  library     Microsoft's code linked into the exe: what Ghidra recognised by its signature, and the ranges in
              docs/notes/library.md. It stays as it is
  generated   pieces the compiler made, not functions of their own: exception clean-up blocks (Unwind@, Catch@)
              and split-off switch cases; they go with the function they belong to
  to do       everything else
"""
import csv
import glob
import os
import re
import sys

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FUNCS = os.path.join(HERE, 'docs', 'functions.csv')
DOCS = [os.path.join(HERE, 'docs', 'exe-map.md'), os.path.join(HERE, 'docs', 'research-physics-and-guests.md')]
NOTES = os.path.join(HERE, 'docs', 'notes')
ORDER = ['rewritten', 'documented', 'to do', 'library', 'generated']


def import_list(src):
    rows = []
    with open(src, newline='') as f:
        for r in csv.DictReader(f):
            name = r['name']
            # Ghidra's placeholder names carry no information; library names it recognised are kept
            if re.match(r'(FUN|thunk_FUN|SUB|LAB)_', name):
                name = ''
            rows.append({'address': r['address'], 'size': r['size'], 'name': name, 'callers': r['callers']})
    return rows


def load():
    with open(FUNCS, newline='') as f:
        return list(csv.DictReader(f))


def rewritten():
    found = {}
    for p in glob.glob(os.path.join(HERE, 'forever', 'src', 'rewrite', '*.cpp')):
        src = open(p, encoding='utf-8').read()
        for m in re.finditer(r'REWRITE\("([^"]+)",\s*(0x[0-9a-fA-F]+)', src):
            found[int(m.group(2), 16)] = m.group(1)
        # REWRITE_COPIES("name", array, ...): every address in that array
        arrays = {m.group(1): m.group(2) for m in re.finditer(r'uint32_t\s+(\w+)\[\]\s*=\s*\{([^}]*)\}', src)}
        for m in re.finditer(r'REWRITE_COPIES\("([^"]+)",\s*(\w+)', src):
            for a in re.findall(r'0x[0-9a-fA-F]+', arrays.get(m.group(2), '')):
                found[int(a, 16)] = m.group(1)
    # template rewrites: one line per copy in generated.inc (see tools/gen_rewrites.py)
    inc = os.path.join(HERE, 'forever', 'src', 'rewrite', 'generated.inc')
    if os.path.exists(inc):
        names = {'DELETING': 'DeletingDestructor', 'QUERY': 'QueryInterface', 'MODULE': 'ModuleGlobal', 'CACHED': 'CachedInterface', 'LOAD': 'StructArrayLoad', 'CLEANUP': 'GlobalCleanup', 'REF_NULL': 'Reference::Reference',
                 'REF_TO': 'Reference::Reference(object)', 'ARRAY_FREE': 'ObjectArray::Free', 'SET_TABLE': 'SetTable',
                 'START_NAME': 'Start::Name', 'START_CLASS': 'Start::ClassDescription', 'START_SECTION': 'Start::Section',
                 'NAMED_TEXT_REF': 'NamedTextRef::NamedTextRef', 'GAME_TEXT_COPY': 'GameText::Copy',
                 'TEXT_REF_FROM_TEXT': 'NamedTextRef::NamedTextRef(text)', 'START_KEY': 'Start::KeyCommand', 'START_KEY3': 'Start::KeyCommand', 'START_KEY1': 'Start::KeyCommand',
                 'START_KEY1_FLAG': 'Start::KeyCommand', 'START_KEY_FULL': 'Start::KeyCommand', 'START_KEY_FULL0': 'Start::KeyCommand',
                 'TEXT_WIDEN': 'Text::Widen', 'TEXT_CREATE': 'Text::Create', 'TABLE20_FREE': 'Table20::Free', 'START_TEXT': 'Start::Text', 'RESOURCE_TYPE': 'ResourceType::ResourceType',
                 'QUERY_REPORT': 'QueryInterface::OrReport',
                 'WIDE_COPY_OBJECT': 'WideText::CopyOf(object)', 'WIDE_COPY_GLOBAL': 'WideText::CopyOf', 'WIDE_COPY_THIS': 'WideText::CopyOf(this)',
                 'START_BEHAVIOUR': 'Start::Behaviour',
                 'FACTORY2': 'Factory::Create', 'FACTORY_FIELDS': 'Factory::CreateFromFields',
                 'OBJECT_SETCAP': 'ObjectArray::SetCapacity',
                 'GLOBAL_FACTORY': 'Factory::Global', 'LAZY_FACTORY': 'Factory::Lazy', 'LAZY_FACTORY_INIT': 'Factory::LazyInit',
                 'DTOR_TABLES': 'Destructor::Tables', 'FREE_TWO': 'OwnedPair::FreeFields',
                 'OBJECT_RESIZE': 'ObjectArray::Resize', 'WITH_REFERENCE': 'ObjectRef::CallWith',
                 'POINTER_RESIZE': 'PointerArray::Resize', 'ARRAY_RESERVE': 'Array::Reserve', 'GUI_INIT': 'GuiPart::Init',
                 'GUI_MAKE': 'GuiPart::Make', 'GUI_CREATE2': 'GuiPart::Create', 'CONDITION_NAME': 'ObjectiveCondition::Name',
                 'FACTORY_K3': 'Factory::CreateWith', 'GLOBAL_FACTORY_K1': 'Factory::GlobalWith', 'FIELD_FACTORY': 'Factory::IntoField',
                 'REF_INTERFACE': 'Reference::Interface', 'DTOR_DELETE_MEMBER': 'Destructor::DeleteMember',
                 'DTOR_MEMBER_BASE': 'Destructor::MemberThenBase', 'FACTORY_K2': 'Factory::CreateWith2', 'FACTORY_K5': 'Factory::CreateWith5',
                 'FACTORY_2K': 'Factory::CreateAround', 'CTOR_BASE_TABLE': 'Constructor::BaseAndTable',
                 'SET_TWO_VECTORS': 'Vectors::SetTwoAndValue', 'LAZY_FACTORY_K1': 'Factory::LazyWith',
                 'FACTORY_REGS': 'Factory::CreateFromRegisters', 'RENDER_STATE': 'RenderState::Set', 'FREE_LIST_TAKE': 'FreeList::Take',
                 'LISTENER_DTOR': 'Listener::~Listener', 'DTOR_MEMBER_IF': 'Destructor::MemberThenBaseNullSafe', 'LAZY_SINGLETON': 'Singleton::Get',
                 'LISTENER_DTOR_QUIET': 'Listener::~Listener(quiet)', 'DTOR_TABLES_MEMBER': 'Destructor::TablesAndMember',
                 'DTOR_DELETE2': 'Destructor::DeleteTwoMembers', 'DTOR5_DELETE2': 'Destructor::FiveTablesTwoMembers',
                 'QUADTREE_ADD': 'QuadTree::Node::Add', 'QUADTREE_INSERT': 'QuadTree::Insert', 'INTERFACE_FORWARD': 'Interface::Forward', 'GETREF': 'Object::GetRef', 'SETFIELDS': 'Object::SetFields', 'FREEFIELD': 'Object::FreeField', 'TABLES_THEN_BASE': 'Destructor::TablesThenBase', 'DELETING2': 'DeletingDestructor(memberfree)', 'FORWARD2': 'Object::Forward2', 'COUNT_LIST': 'Object::CountList', 'CALL_THEN_TAIL': 'Object::CallThenTail', 'DBG_SINGLETON': 'Object::DbgSingleton', 'GETFIELD': 'Object::GetField', 'GLOBAL_MEMBER_CALL': 'Object::GlobalMemberCall', 'COND_FORWARD': 'Object::CondForward', 'ARRAY_PUSH': 'Object::ArrayPush', 'DELMEMBER_FIELD': 'Object::DelMemberField', 'FREE_TWO_FIELDS': 'Object::FreeTwoFields', 'DBG_WRAP2': 'Object::DbgWrap2', 'CTOR_BASE_ARG': 'Object::CtorBaseArg', 'ALLOC_STORE': 'Object::AllocStore', 'COND_CALL_SET': 'Object::CondCallSet', 'FREE_TWO_TABLE': 'Object::FreeTwoTable', 'GRID_GROW': 'Object::GridGrow', 'QUAD_COLLECT': 'Object::QuadCollect'}
        for m in re.finditer(r'^(\w+)\((0x[0-9a-fA-F]+)', open(inc).read(), re.M):
            found[int(m.group(2), 16)] = names.get(m.group(1), m.group(1))
    return found


def documented(addrs):
    text = ' '.join(open(p, encoding='utf-8').read() for p in DOCS if os.path.exists(p))
    text = re.sub(r'## Program layout.*?(?=\n## )', '', text, flags=re.S)   # section addresses, not functions
    seen = {int(h, 16) for h in re.findall(r'0x([0-9a-fA-F]{6,7})\b', text)}
    return seen & addrs


def noted(addrs):
    """addresses in the first column of the tables in docs/notes/*.md (library.md: ranges)"""
    found, ranges = set(), []
    for p in glob.glob(os.path.join(NOTES, '*.md')):
        for line in open(p, encoding='utf-8'):
            m = re.match(r'\|([^|]*)\|', line)
            if not m:
                continue
            first = m.group(1)
            if os.path.basename(p) == 'library.md':
                r = re.match(r'\s*`0x([0-9a-fA-F]+)`\s*[–-]\s*`0x([0-9a-fA-F]+)`', first)
                if r:
                    ranges.append((int(r.group(1), 16), int(r.group(2), 16)))
                continue
            found |= {int(h, 16) for h in re.findall(r'`0x([0-9a-fA-F]+)`', first)}
    return found & addrs, ranges


def generated(name):
    return name.startswith(('Unwind@', 'Catch@', 'caseD_', 'switchD_'))


def main():
    rows = import_list(sys.argv[1]) if len(sys.argv) > 1 else load()
    rw = rewritten()
    addrs = {int(r['address'], 16) for r in rows}
    doc = documented(addrs)
    notes, lib_ranges = noted(addrs)
    for r in rows:
        a = int(r['address'], 16)
        if a in rw:
            r['status'], r['name'] = 'rewritten', rw[a]
        elif generated(r['name']):
            r['status'] = 'generated'
        elif any(lo <= a < hi for lo, hi in lib_ranges):
            r['status'] = 'library'
        elif a in doc or a in notes:
            r['status'] = 'documented'
        elif r['name'] and r['name'] != 'entry':
            r['status'] = 'library'
        else:
            r['status'] = 'to do'
    with open(FUNCS, 'w', newline='') as f:
        w = csv.DictWriter(f, ['address', 'size', 'name', 'callers', 'status'], lineterminator='\n')
        w.writeheader()
        w.writerows(rows)

    count = {s: 0 for s in ORDER}
    size = {s: 0 for s in ORDER}
    for r in rows:
        count[r['status']] += 1
        size[r['status']] += int(r['size'])
    game = ['rewritten', 'documented', 'to do']
    total_n, total_b = sum(count[s] for s in game), sum(size[s] for s in game)
    pct = lambda v, t: f'{100 * v / t:.2f}%' if t else '-'

    bar_w = 40
    done = round(bar_w * size['rewritten'] / total_b) if total_b else 0
    known = round(bar_w * size['documented'] / total_b) if total_b else 0
    bar = '█' * done + '▓' * known + '░' * (bar_w - done - known)

    # documented counts every game function a note describes, rewritten ones included
    noted_b = size['documented'] + sum(int(r['size']) for r in rows
                                       if r['status'] == 'rewritten' and int(r['address'], 16) in notes | doc)
    summary = [
        f'`{bar}` **{pct(size["rewritten"], total_b)} rewritten**, {pct(noted_b, total_b)} understood '
        f'and documented (by code size; the rewritten part included)',
        '',
        '| Status | Functions | Code | Share of the game\'s code |',
        '|---|---:|---:|---:|',
    ]
    for s in game:
        label = 'Documented, not rewritten yet' if s == 'documented' else s.capitalize()
        summary.append(f'| {label} | {count[s]:,} | {size[s]:,} bytes | {pct(size[s], total_b)} |')
    summary.append(f'| **Game total** | **{total_n:,}** | **{total_b:,} bytes** | |')
    summary.append(f'| Library code from Microsoft (stays as is; see [library.md](docs/notes/library.md)) | {count["library"]:,} | {size["library"]:,} bytes | |')
    summary.append(f'| Compiler-made pieces (go with their function) | {count["generated"]:,} | {size["generated"]:,} bytes | |')

    def table(status, limit=None, key=None):
        sel = [r for r in rows if r['status'] == status]
        if key: sel.sort(key=key)
        lines = ['| Address | Size | Name | Called from |', '|---|---:|---|---:|']
        for r in sel[:limit]:
            lines.append(f'| `{r["address"]}` | {int(r["size"]):,} | {r["name"] or "-"} | {r["callers"]} |')
        return lines

    out = ['# RCT3 Forever progress', '',
           'Generated by `tools/progress.py` from [functions.csv](functions.csv), the list of every function in '
           'RCT3plus.exe (address, size, how many places call it, status). Only the list is here: the game\'s code '
           'is never part of this project.', '', *summary, '',
           '## Rewritten', '', *table('rewritten', key=lambda r: int(r['address'], 16)), '',
           '## Understood and documented, not rewritten yet', '',
           'Described in [the notes](notes/), [the exe map](exe-map.md) or '
           '[the research notes](research-physics-and-guests.md): the next ones to rewrite.', '', *table('documented', key=lambda r: int(r['address'], 16)), '',
           '## To do: the most-called functions', '',
           'The game functions called from the most places. Small ones high on this list are good next targets; '
           'the full list is in functions.csv.', '',
           *table('to do', 100, key=lambda r: -int(r['callers'])), '']
    open(os.path.join(HERE, 'docs', 'progress.md'), 'w', encoding='utf-8', newline='\n').write('\n'.join(out))

    readme = os.path.join(HERE, 'README.md')
    text = open(readme, encoding='utf-8').read()
    block = '<!-- progress -->\n' + '\n'.join(summary) + '\n\nDetails: [docs/progress.md](docs/progress.md).\n<!-- /progress -->'
    text = re.sub(r'<!-- progress -->.*?<!-- /progress -->', lambda m: block, text, flags=re.S)
    open(readme, 'w', encoding='utf-8', newline='\n').write(text)
    print(f'{total_n} game functions: ' + ', '.join(f'{count[s]} {s}' for s in ORDER))


if __name__ == '__main__':
    main()
