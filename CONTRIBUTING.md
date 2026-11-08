# Contributing to RCT3 Forever

Thanks for helping rewrite RollerCoaster Tycoon 3. Every rewritten function replaces one of the game's own, so each one
has to behave exactly the same. This page is the checklist; [docs/rewriting.md](docs/rewriting.md) explains how.

## What you need

- Your own copy of **RollerCoaster Tycoon 3 Platinum / Gold** (with Soaked! and Wild!), Steam or disc, unpatched. The
  equivalence tester runs the game's own functions next to yours, so it can't work without the game. Complete Edition
  is a different program and isn't supported.
- Windows, [uv](https://docs.astral.sh/uv/) and git. The build fetches the compiler (zig) and Dear ImGui by itself.
- Build and test: `uv run --no-project --with ziglang --with pefile --with capstone python build.py test`
- Read a game function: `uv run --no-project --with pefile --with capstone python tools/disasm.py 0x445440`. It finds
  your RCT3plus.exe through `tools/gamepath.py` (the `RCT3_EXE` variable, the game folder in `out/RCT3Forever.ini`, or
  the usual Steam folder). Tools that take `<exe>` want that path.
- Using Claude Code? [CLAUDE.md](CLAUDE.md) gives it these rules and commands. Playing the scenario at the end is
  still yours to do.

## The rules

1. **Write from understanding.** Work out what a function does, then write your own code that does the same. Never
   paste decompiler output (Ghidra, IDA and the like), not even renamed or tidied up.
2. **Nothing from the game goes in the repo**: no game files, assets, dumps or disassembler projects. Addresses and the
   few signature bytes a `REWRITE` line checks are fine.
3. **Match exactly**: the same calling convention and stack clean-up (`ret N`), the same return value, the same memory
   written. Signatures must be at least 5 bytes.

## Picking something to rewrite

- Look at [docs/progress.md](docs/progress.md) and the `documented` rows of [docs/functions.csv](docs/functions.csv).
- Families (functions the compiler copied many times) give the most per rewrite: `tools/cleanfam.py` lists them.
- Check the open issues, and comment on one (or open one) to claim it, so two people don't do the same function.

## Before you open a pull request

- [ ] Put your rewrites in their own file, `forever/src/rewrite/<topic>.cpp`, so pull requests rarely conflict.
- [ ] Add a test to `tools/equiv/equiv.cpp` and get **0 different** from `build.py test`, for every copy.
- [ ] **Plant a bug** in your rewrite (a wrong offset or constant), run the test again and see it fail, then undo it.
      A test that can't catch a planted bug isn't testing anything.
- [ ] Run the game with your build (`out/RCT3Forever.exe`), play a scenario for a few minutes and check
      `out/RCT3Forever.log`: "rewritten functions in use: N of N", your function's call count, and "game closed".
- [ ] Run `tools/progress.py` so the progress numbers include your work.

In the pull request, paste the tester's summary line, say which bug you planted, and paste those log lines.

## Review

A maintainer reads the code to check it was written from understanding, runs the tester and one planted bug, and
plays a scenario before merging. The tester catches wrong behaviour; only a person can catch copied code.

By contributing you agree that your code is released under the project's [MIT license](LICENSE).
