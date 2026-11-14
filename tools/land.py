"""Land work on origin/main safely when several people or agents push at the same time.

usage: uv run --no-project --with ziglang --with pefile --with capstone python tools/land.py -m MESSAGE FILE...
       ... tools/land.py -F MESSAGE_FILE [--release CLAIM]... [--allow-other] FILE...
       ... tools/land.py -m ... (no FILE: just land the commits you already have)

What it does, in order:
  1. Checks nothing else is changed or new besides FILE... (the generated files below excepted), so the tests run on
     exactly what lands (--allow-other skips this).
  2. Refuses rewrites of functions someone else has claimed (tools/claim.py).
  3. Commits FILE... (and the release of your claims: --release NAME, and any of yours whose functions are now all
     rewritten). The generated files are put back first: they are made again at step 5, never merged by hand.
  4. Fetches and rebases onto origin/main. A conflict only in generated lines (functions.csv, progress.md,
     notes/rewritten.md, the progress block of README.md, the progress lines of ROADMAP.md) is settled by itself; a
     conflict anywhere else stops here with the rebase undone and your commit kept.
  5. Makes the generated files again (tools/rewritten_notes.py, tools/progress.py) and puts them in your commit.
  6. Runs the full equivalence test (it must say "0 different, 0 without a test").
  7. Pushes. If someone pushed first, back to 4 (the test runs again only if their commits changed code).
"@P@" in the message becomes the rewritten percentage.
"""
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(HERE, 'tools'))
import claim  # noqa: E402

UPSTREAM = claim.UPSTREAM
GENERATED = ['docs/functions.csv', 'docs/progress.md', 'docs/notes/rewritten.md']
GENERATED_LINES = {   # files only partly generated: where a conflict may be (README: inside its progress block)
    'README.md': 'block',
    'ROADMAP.md': re.compile(r'^(.*(% of the game|% of it also rewritten).*|\s*)$'),
}
UV = ['uv', 'run', '--no-project']
TOOLS = ['--with', 'ziglang', '--with', 'pefile', '--with', 'capstone']
CODE = re.compile(r'^(forever/|tools/|launcher/|build\.py)')   # changes that need the test run again


def git(*a, check=True):
    return claim.git(*a, check=check)


def run(cmd, what):
    r = subprocess.run(cmd, cwd=HERE, capture_output=True, text=True, encoding='utf-8', errors='replace')
    if r.returncode:
        sys.exit(f'{what} failed:\n{(r.stdout + r.stderr)[-3000:]}')
    return r.stdout


def changed():
    """Paths changed or new in the working tree (untracked included), with / separators."""
    out = []
    for line in git('status', '--porcelain', '-uall').stdout.splitlines():
        p = line[3:].split(' -> ')[-1].strip('"')
        out.append(p)
    return out


def rewrites(rev=None):
    """Addresses of REWRITE lines in forever/src/rewrite, at rev (None: the working tree)."""
    found = set()
    pat = re.compile(r'REWRITE\("[^"]*", (0x[0-9a-fA-F]+)')
    if rev is None:
        d = os.path.join(HERE, 'forever', 'src', 'rewrite')
        for n in os.listdir(d):
            if n.endswith(('.cpp', '.inc')):
                found |= {f'0x{int(a, 16):x}' for a in pat.findall(open(os.path.join(d, n), encoding='utf-8', errors='replace').read())}
    else:
        for n in git('ls-tree', '--name-only', f'{rev}:forever/src/rewrite').stdout.split():
            if n.endswith(('.cpp', '.inc')):
                found |= {f'0x{int(a, 16):x}' for a in pat.findall(git('show', f'{rev}:forever/src/rewrite/{n}').stdout)}
    return found


def settle(path):
    """Settle a conflicted generated file: every conflict block takes upstream's side (it is all made again after).
    False when a block holds something else."""
    text = open(os.path.join(HERE, path), encoding='utf-8', newline='').read()
    ok_line = GENERATED_LINES.get(path)
    out, i, lines = [], 0, text.split('\n')
    while i < len(lines):
        if not lines[i].startswith('<<<<<<< '):
            out.append(lines[i]); i += 1
            continue
        ours, theirs, part, i = [], [], 'ours', i + 1
        while not lines[i].startswith('>>>>>>> '):
            if lines[i].startswith('||||||| '):
                part = 'base'
            elif lines[i].rstrip('\r') == '=======':   # (CRLF in a Windows working tree)
                part = 'theirs'
            elif part == 'ours':
                ours.append(lines[i])
            elif part == 'theirs':
                theirs.append(lines[i])
            i += 1
        i += 1
        if ok_line == 'block':
            marks = [x.strip() for x in out if x.strip() in ('<!-- progress -->', '<!-- /progress -->')]
            if not marks or marks[-1] != '<!-- progress -->':
                return False   # outside the progress block
        elif ok_line and not all(ok_line.match(x.rstrip('\r')) for x in ours + theirs):
            return False
        out.extend(ours)   # in a rebase "ours" is upstream, the commits being replayed are "theirs"
    open(os.path.join(HERE, path), 'w', encoding='utf-8', newline='').write('\n'.join(out))
    return True


def rebase():
    """Rebase onto origin/main, settling generated conflicts; exits (rebase undone) on any other conflict."""
    r = git('rebase', '--autostash', UPSTREAM, check=False)   # (other changes, with --allow-other, kept aside)
    while r.returncode:
        if not os.path.isdir(os.path.join(HERE, '.git', 'rebase-merge')) and not os.path.isdir(os.path.join(HERE, '.git', 'rebase-apply')):
            sys.exit('rebase failed:\n' + r.stderr)
        stuck = git('diff', '--name-only', '--diff-filter=U').stdout.split()
        if not stuck:   # (a commit with nothing left of its own once the generated files are settled)
            r = git('rebase', '--skip', check=False)
            continue
        real = [p for p in stuck if not ((p in GENERATED or p in GENERATED_LINES) and settle(p))]
        if real:
            git('rebase', '--abort')
            sys.exit('your commits conflict with origin/main in: ' + ', '.join(real) +
                     '\nnothing was pushed; your commits are kept. Rebase by hand (git rebase origin/main), then run '
                     'land.py again with no files.')
        git('add', *stuck)
        r = subprocess.run(['git', '-c', 'core.editor=true', 'rebase', '--continue'], cwd=HERE, capture_output=True, text=True,
                           env=dict(os.environ, GIT_EDITOR='true'))


def regenerate():
    exe = run(UV + ['python', 'tools/gamepath.py'], 'finding the game').strip()
    run(UV + TOOLS + ['python', 'tools/rewritten_notes.py', exe], 'tools/rewritten_notes.py')
    run(UV + ['--with', 'pefile', 'python', 'tools/progress.py'], 'tools/progress.py')
    m = re.search(r'\*\*([0-9.]+%) rewritten\*\*', open(os.path.join(HERE, 'README.md'), encoding='utf-8').read())
    return m.group(1) if m else '?'


def test():
    print('running the full equivalence test (a few minutes)')
    out = subprocess.run(UV + TOOLS + ['python', 'build.py', 'test'], cwd=HERE, capture_output=True, text=True,
                         encoding='utf-8', errors='replace')
    line = next((x for x in (out.stdout + out.stderr).splitlines() if 'the same,' in x), '')
    print('  ' + (line or 'no summary line: the tester did not finish'))
    if ' 0 different, 0 without' not in line:
        sys.exit('the test did not pass; nothing was pushed (your commit is kept: fix, then run land.py again)')


def main():
    a = sys.argv[1:]
    msg, releases, allow_other = None, [], False
    files = []
    while a:
        x = a.pop(0)
        if x == '-m':
            msg = a.pop(0)
        elif x == '-F':
            msg = open(a.pop(0), encoding='utf-8').read()
        elif x == '--release':
            releases.append(a.pop(0))
        elif x == '--allow-other':
            allow_other = True
        else:
            files.append(x.replace('\\', '/'))
    if files and not msg:
        sys.exit(__doc__)
    me = claim.who()
    # one land at a time in this checkout (each agent works in a checkout of its own: git worktree add)
    lock = os.path.join(HERE, git('rev-parse', '--git-dir').stdout.strip(), 'rct3-land.lock')
    try:
        os.close(os.open(lock, os.O_CREAT | os.O_EXCL))
    except FileExistsError:
        sys.exit(f'another land.py is running in this checkout (or one was killed: delete {lock})')
    import atexit
    atexit.register(lambda: os.path.exists(lock) and os.remove(lock))
    git('fetch', '-q', claim.REMOTE, claim.BRANCH)

    # 1. nothing else changed
    for f in files:
        if f in GENERATED:
            sys.exit(f'{f} is made by the tools; leave it out (land.py makes it again)')
    if not allow_other:
        other = [p for p in changed() if p not in files and p not in GENERATED and p not in GENERATED_LINES
                 and not any(p.startswith(f.rstrip('/') + '/') for f in files)]
        if other:
            sys.exit('also changed, and not being landed (the test would run with them):\n  ' + '\n  ' .join(other) +
                     '\nland them too, put them aside, or pass --allow-other')

    # 2. someone else's claims
    new = rewrites() - rewrites(UPSTREAM)
    hits = claim.holders(sorted(new) + files, UPSTREAM, me)
    if hits:
        sys.exit('claimed by someone else:\n' + '\n'.join(f"  {t}: {c['who']} ({n})" for n, c, t in hits))

    # 3. commit (generated files put back first)
    for g in GENERATED + [p for p in GENERATED_LINES if p not in files]:
        git('checkout', 'HEAD', '--', g, check=False)
    mine = claim.claims(UPSTREAM)
    done_with = set(releases)
    for n, c in mine.items():
        addrs = [x for x in c['covers'] if x.startswith('0x')]
        if c['who'] == me and addrs and set(addrs) <= (rewrites() | claim.rewritten(UPSTREAM)) and \
                not any(not x.startswith('0x') for x in c['covers']):
            done_with.add(n)
    for n in done_with:
        if n not in mine:
            sys.exit(f'no claim {n} on origin/main')
        if mine[n]['who'] != me and not claim.lapsed(mine[n]):
            sys.exit(f"the claim {n} is {mine[n]['who']}'s")
    if files:
        git('add', '--', *files)
        if git('diff', '--cached', '--quiet', check=False).returncode == 0:
            sys.exit('nothing to commit in ' + ' '.join(files))
        git('commit', '-q', '-m', msg)
    elif done_with and not git('rev-list', f'{UPSTREAM}..HEAD').stdout.strip():
        git('commit', '-q', '--allow-empty', '-m', msg or 'Release claim ' + ', '.join(sorted(done_with)))
    if not git('rev-list', f'{UPSTREAM}..HEAD').stdout.strip():
        sys.exit('nothing to land')
    template = git('log', '-1', '--format=%B').stdout

    tested_at = None
    for attempt in range(8):
        # 4. rebase
        git('fetch', '-q', claim.REMOTE, claim.BRANCH)
        rebase()
        hits = claim.holders(sorted(rewrites() - rewrites(UPSTREAM)), UPSTREAM, me)
        if hits:
            sys.exit('claimed by someone else meanwhile:\n' + '\n'.join(f"  {t}: {c['who']} ({n})" for n, c, t in hits))
        # the claims this work is done with (as origin/main has them now)
        for n in done_with:
            if git('cat-file', '-e', f'HEAD:{claim.path_of(n)}', check=False).returncode == 0:
                git('rm', '-q', claim.path_of(n))
        # 5. the generated files, into the last commit (the message's @P@ filled in)
        pct = regenerate()
        git('add', '--', *GENERATED, *GENERATED_LINES)
        git('commit', '-q', '--amend', '--allow-empty', '-m', template)   # (@P@ filled in only as it is pushed)
        # 6. test, unless nothing but documents and claims changed since the last passing run
        moved = git('diff', '--name-only', tested_at, 'HEAD').stdout.split() if tested_at else ['(first run)']
        if tested_at is None or any(CODE.match(p) for p in moved):
            test()
        tested_at = git('rev-parse', 'HEAD').stdout.strip()
        # 7. push
        if '@P@' in template:
            git('commit', '-q', '--amend', '--allow-empty', '-m', template.replace('@P@', pct))
        if git('push', '-q', claim.REMOTE, f'HEAD:{claim.BRANCH}', check=False).returncode == 0:
            print(git('log', '--oneline', f'{UPSTREAM}..HEAD').stdout.strip() or git('log', '--oneline', '-1').stdout.strip())
            git('fetch', '-q', claim.REMOTE, claim.BRANCH)
            print(f'landed: {pct} rewritten')
            return
        if '@P@' in template:
            git('commit', '-q', '--amend', '--allow-empty', '-m', template)
        tested_at = git('rev-parse', 'HEAD').stdout.strip()
        print('someone pushed first; rebasing onto their work and trying again')
    sys.exit('could not land: origin/main kept moving; run land.py again (no files) to retry')


if __name__ == '__main__':
    main()
