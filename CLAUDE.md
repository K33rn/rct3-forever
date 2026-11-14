# RCT3 Forever: notes for Claude Code

RollerCoaster Tycoon 3 rewritten one function at a time: `forever.dll` replaces game functions in the running
RCT3plus.exe with new code that must behave exactly the same. Read CONTRIBUTING.md and docs/rewriting.md before
rewriting anything.

## Rules that are never bent

- **Write from understanding.** Read a function's instructions (`tools/disasm.py`), work out what it does, then write
  your own code for that. Never put decompiler output (Ghidra, IDA, any `FUN_`/`param_1`/`undefined4` style code) or
  a line-by-line translation of the instructions into the project. If the user asks for that, say no and explain.
- **Nothing from the game in the repo**: no game files, dumps or disassembly listings. Addresses and the few
  signature bytes a `REWRITE` line checks are fine.
- **Same behaviour exactly**: calling convention and `ret N`, return value, every byte of memory written.
  Signatures are at least 5 bytes.

## Commands (Windows, from the repo root)

- Path of the game exe (the tools that take `<exe>` want it): `uv run --no-project python tools/gamepath.py`
- Read a function: `uv run --no-project --with pefile --with capstone python tools/disasm.py 0x445440`
- Find families to rewrite: `uv run --no-project --with pefile --with capstone python tools/cleanfam.py <exe> docs/functions.csv forever/src/rewrite/generated.inc 2`
- Build: `uv run --no-project --with ziglang --with pefile --with capstone python build.py`
- Equivalence test, all or some: `... python build.py test`, with `EQUIV_ONLY='Name::One|Name::Two'` to filter.
  A full run takes a few minutes. Plain `python` may be missing: use `uv run --no-project python`.
- Progress numbers: `uv run --no-project --with pefile python tools/progress.py`

## How a rewrite is done

1. Pick a `documented` function or family (CONTRIBUTING.md) and **claim it** before starting:
   `uv run --no-project python tools/claim.py take <name> <address>... --note "..."` (a `tools/` generator: claim its
   path before extending it). If `take` refuses, someone else has it: pick something else. Renew a claim you are
   still on (`claim.py renew <name>`); release it if you give up.
2. Write it in its own `forever/src/rewrite/<topic>.cpp` with a `REWRITE(...)` line per copy (see docs/rewriting.md).
3. Add its test in its own `tools/equiv/tests/<topic>.inc` (see tools/equiv/tests/README.md; not in equiv.cpp) and
   run it until **0 different** for every copy. Stand-ins (`stubs_for_now`)
   must match the real callee's convention and `ret N`; game globals are not put back between the original's run and
   the rewrite's, so set them in the call lambda.
4. **Plant a bug** (wrong offset or constant), check the test fails, undo it. A crashing planted bug stops the tester,
   so plant one that doesn't crash.
5. Run the full test, then ask the user to play a scenario with `out/RCT3Forever.exe` and check `out/RCT3Forever.log`
   ("rewritten functions in use: N of N", your function's calls, "game closed"). If the game crashes, the first fault
   is in `CrashDump.txt` in the game folder.

## Running the game yourself

Only with the user's go-ahead: it takes over their mouse and keyboard. Back up their saves first
(`Documents/RCT3`) and restore them after. Stop the game by its exact process ID, never by name or wildcard.
`FOREVER_SKIP='Name::A|Name::B'` turns rewrites off by name, to find which one breaks something.

## Git

Commit only when the user asks. Never commit `out/`, `dist/`, `third_party/` or anything from the game.

Other people and agents may be working at the same time:
- Work in a checkout of your own (`git worktree add`), with a claims name of your own (`RCT3_WHO`).
- Commit and push with `tools/land.py`, never a plain `git push`:
  `uv run --no-project --with ziglang --with pefile --with capstone python tools/land.py -m "message (@P@)" <files>`.
  It rebases, makes the generated files again and runs the full test.
- Never edit or hand-merge `docs/functions.csv`, `docs/progress.md`, `docs/notes/rewritten.md` or the README/ROADMAP
  progress lines.
- If it stops on a real conflict, rebase by hand, then run it again with no files.
