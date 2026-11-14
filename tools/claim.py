"""Claims: who is working on what, so two people (or agents) never rewrite the same function or edit the same
generator at the same time.

A claim is one small file, claims/<name>.txt, on origin/main: who holds it, since when, and what it covers (function
addresses and/or repo paths such as tools/titled_ctors.py). One file per claim, so two claims never touch the same
lines. Claims are published straight to origin/main with git's plumbing (a temporary index, commit-tree, push): your
working tree, index and half-done work are never touched. If someone else pushes first, it fetches and checks again.

A claim lapses DAYS days after it was taken or last renewed; tools/land.py takes your claim away for you when it lands
the work (--release NAME, or by itself once every address it covers is rewritten).

usage: uv run --no-project python tools/claim.py take NAME ADDR|PATH... [--note TEXT]
       uv run --no-project python tools/claim.py renew NAME
       uv run --no-project python tools/claim.py release NAME
       uv run --no-project python tools/claim.py list            (who holds what; lapsed ones marked)
       uv run --no-project python tools/claim.py check ADDR|PATH...
Who you are: the RCT3_WHO variable, else git's user.name. Give each agent its own (RCT3_WHO="K3rn/agent-2").
"""
import csv
import datetime
import io
import os
import re
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DIR = 'claims'
DAYS = 5
REMOTE, BRANCH = 'origin', 'main'
UPSTREAM = f'{REMOTE}/{BRANCH}'


def git(*args, env=None, input=None, check=True):
    r = subprocess.run(['git', *args], cwd=HERE, env=env, input=input, capture_output=True, text=True, encoding='utf-8')
    if check and r.returncode:
        sys.exit(f'git {" ".join(args)} failed:\n{r.stderr.strip()}')
    return r


def who():
    return os.environ.get('RCT3_WHO') or git('config', 'user.name').stdout.strip() or 'someone'


def today():
    return datetime.date.today()


def item(s):
    """An address (any case, with or without 0x) as 0x-lowercase hex, or a repo path with / separators."""
    if re.fullmatch(r'(0x)?[0-9a-fA-F]{5,8}', s):
        return f'0x{int(s, 16):x}'
    return s.replace('\\', '/').lstrip('./')


def parse(text):
    c = {'who': '', 'since': '', 'renewed': '', 'covers': [], 'note': ''}
    for line in text.splitlines():
        k, _, v = line.partition(':')
        k, v = k.strip(), v.strip()
        if k == 'covers':
            c['covers'] = v.split()
        elif k in c:
            c[k] = v
    return c


def render(c):
    return (f"who: {c['who']}\nsince: {c['since']}\nrenewed: {c['renewed']}\ncovers: {' '.join(c['covers'])}\n"
            f"note: {c['note']}\n")


def lapsed(c):
    try:
        return today() > datetime.date.fromisoformat(c['renewed'] or c['since']) + datetime.timedelta(days=DAYS)
    except ValueError:
        return True


def claims(rev=UPSTREAM):
    """{name: claim} as rev has them."""
    out = {}
    names = git('ls-tree', '--name-only', f'{rev}:{DIR}', check=False).stdout.split()
    for n in names:
        if n.endswith('.txt'):
            out[n[:-4]] = parse(git('show', f'{rev}:{DIR}/{n}').stdout)
    return out


def covers(cover, thing):
    """Does a covered item take in thing? An address only itself; a path itself and everything under it."""
    return cover == thing or (not cover.startswith('0x') and thing.startswith(cover.rstrip('/') + '/'))


def holders(things, rev=UPSTREAM, me=None, skip=None):
    """[(name, claim, thing)] for live claims of someone other than me that cover any of things."""
    hits = []
    for name, c in claims(rev).items():
        if name == skip or lapsed(c) or (me and c['who'] == me):
            continue
        for t in things:
            if any(covers(x, t) for x in c['covers']):
                hits.append((name, c, t))
    return hits


def rewritten(rev=UPSTREAM):
    text = git('show', f'{rev}:docs/functions.csv').stdout
    return {f"0x{int(r['address'], 16):x}" for r in csv.DictReader(io.StringIO(text)) if r['status'] == 'rewritten'}


def publish(change, message, attempts=6):
    """Fetch, let change(base) return {path: text or None to remove} (or exit), commit that on top of origin/main
    without touching the working tree, push; again from the start if someone pushed in between."""
    for _ in range(attempts):
        git('fetch', '-q', REMOTE, BRANCH)
        base = git('rev-parse', UPSTREAM).stdout.strip()
        files = change(base)
        with tempfile.TemporaryDirectory() as tmp:
            env = dict(os.environ, GIT_INDEX_FILE=os.path.join(tmp, 'index'))
            git('read-tree', base, env=env)
            for path, text in files.items():
                if text is None:
                    git('update-index', '--force-remove', path, env=env)
                else:
                    blob = git('hash-object', '-w', '--stdin', input=text).stdout.strip()
                    git('update-index', '--add', '--cacheinfo', f'100644,{blob},{path}', env=env)
            tree = git('write-tree', env=env).stdout.strip()
        commit = git('commit-tree', tree, '-p', base, '-m', message).stdout.strip()
        if git('push', '-q', REMOTE, f'{commit}:refs/heads/{BRANCH}', check=False).returncode == 0:
            git('fetch', '-q', REMOTE, BRANCH)
            # bring a clean checkout that was at the old tip along (anything else is left for tools/land.py)
            if git('rev-parse', 'HEAD').stdout.strip() == base and not git('status', '--porcelain', '-uno').stdout.strip():
                git('merge', '-q', '--ff-only', UPSTREAM, check=False)
            return commit
        print('someone pushed first; trying again')
    sys.exit('could not push the claim (the remote kept moving); try again')


def path_of(name):
    if not re.fullmatch(r'[A-Za-z0-9_.-]+', name):
        sys.exit('a claim name is letters, digits, "_", "-" and "."')
    return f'{DIR}/{name}.txt'


def take(name, things, note):
    me = who()

    def change(base):
        mine = claims(base).get(name)
        if mine and mine['who'] != me and not lapsed(mine):
            sys.exit(f"the claim {name} is {mine['who']}'s (since {mine['since']})")
        hits = holders(things, base, me, skip=name)
        if hits:
            sys.exit('already claimed:\n' + '\n'.join(f"  {t}: {c['who']}'s claim {n} (since {c['since']}, renewed "
                                                       f"{c['renewed'] or c['since']}): {c['note']}" for n, c, t in hits))
        done = sorted(set(things) & rewritten(base))
        if done:
            sys.exit('already rewritten: ' + ' '.join(done))
        c = {'who': me, 'since': today().isoformat(), 'renewed': today().isoformat(), 'covers': things, 'note': note}
        if mine and mine['who'] == me:
            c['since'] = mine['since']
            c['covers'] = sorted(set(mine['covers']) | set(things), key=lambda s: (not s.startswith('0x'), s))
        return {path_of(name): render(c)}
    publish(change, f'Claim {name}: {" ".join(things)} ({me})')
    print(f'claimed {name} for {me} until {today() + datetime.timedelta(days=DAYS)} (renew to keep it longer)')


def renew(name):
    me = who()

    def change(base):
        c = claims(base).get(name)
        if not c:
            sys.exit(f'no claim {name}')
        if c['who'] != me and not lapsed(c):
            sys.exit(f"the claim {name} is {c['who']}'s")
        c['who'], c['renewed'] = me, today().isoformat()
        return {path_of(name): render(c)}
    publish(change, f'Renew claim {name} ({me})')
    print(f'{name} renewed until {today() + datetime.timedelta(days=DAYS)}')


def release(name):
    me = who()

    def change(base):
        c = claims(base).get(name)
        if not c:
            sys.exit(f'no claim {name}')
        if c['who'] != me and not lapsed(c):
            sys.exit(f"the claim {name} is {c['who']}'s; only they can release it before it lapses")
        return {path_of(name): None}
    publish(change, f'Release claim {name} ({me})')
    print(f'released {name}')


def show():
    git('fetch', '-q', REMOTE, BRANCH)
    cs = claims()
    if not cs:
        print('no claims')
    for n, c in sorted(cs.items()):
        state = 'LAPSED' if lapsed(c) else 'until ' + str(datetime.date.fromisoformat(c['renewed'] or c['since']) + datetime.timedelta(days=DAYS))
        print(f"{n:24} {c['who']:16} {state:17} {' '.join(c['covers'])}" + (f"  ({c['note']})" if c['note'] else ''))


def check(things):
    git('fetch', '-q', REMOTE, BRANCH)
    hits = holders(things)
    done = set(things) & rewritten()
    for t in things:
        hs = [f"{c['who']} ({n})" for n, c, x in hits if x == t]
        print(f"{t}: " + ('rewritten already' if t in done else 'claimed by ' + ', '.join(hs) if hs else 'free'))
    return 1 if hits or done else 0


if __name__ == '__main__':
    a = sys.argv[1:]
    note = ''
    if '--note' in a:
        i = a.index('--note')
        note = a[i + 1].replace('\n', ' ')
        del a[i:i + 2]
    if a[:1] == ['take'] and len(a) >= 3:
        take(a[1], [item(x) for x in a[2:]], note)
    elif a[:1] == ['renew'] and len(a) == 2:
        renew(a[1])
    elif a[:1] == ['release'] and len(a) == 2:
        release(a[1])
    elif a[:1] == ['list']:
        show()
    elif a[:1] == ['check'] and len(a) >= 2:
        sys.exit(check([item(x) for x in a[1:]]))
    else:
        sys.exit(__doc__)
