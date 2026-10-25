# Rewriting a game function

RCT3 Forever replaces the game's functions one at a time with new code. Each rewrite takes over every call of the
original, so the rest of the game keeps working while more and more of it becomes open source.

## The rules

1. **Understand, then write.** Study what the function does: its disassembly, what it reads and writes, how the
   game uses the results. Then write a new function that does the same thing, in clear C++ with names and comments.
   **Never paste decompiler output (Ghidra, IDA, Binary Ninja...) into the project, not even tidied up.** That would
   be the game's code in another form. Facts are fine: what a function does, its address, the layout of an object,
   the formula it uses.
2. **Nothing from the game in the repo:** no game files, no extracted assets, no bytes of its code. The only
   exception is the few first bytes of each original function, which Forever checks before replacing it.
3. **Same behaviour.** The rewrite must leave the game in the same state as the original: the same fields, the
   same order of effects, the same calling convention.
4. **Prove it in the game.** Run the game with the rewrite and show it behaves the same, with something you can see
   or measure. Write down how in the table below.
5. **Update the progress:** `python tools/progress.py` picks up the new `REWRITE(...)` and refreshes
   [progress.md](progress.md) and the README.

## Studying a function

Use a disassembler or decompiler on **your own copy** of RCT3plus.exe (Ghidra is free). Keep its project and any
output outside this repository. Read it, take notes in your own words, then close it and write the rewrite.
The function list in [functions.csv](functions.csv) has every function's address and size; give
`tools/progress.py` a fresh `functions.csv` exported from Ghidra (address, size, name, callers) to rebuild it.

## How a rewrite is put in

`forever/src/rewrite/<part of the game>.cpp`:

```cpp
#include "rewrite.h"

static volatile long g_calls;

static void __fastcall push(GraphedValue *g, void *, float value) { g_calls++; ... }

static const uint8_t push_original[] = {0xff, 0x49, 0x04, 0x79, 0x07, 0x8b, 0x41, 0x08};
REWRITE("GraphedValue::Push", 0x611af0, push_original, push, &g_calls);
```

At start-up `forever.dll` checks that the game's function at that address still starts with those bytes (at least
5). It then writes a jump to the new function over them. If the bytes don't match, it leaves the function alone
and logs `not replaced`. `RCT3Forever.log` reports how often each rewrite has run. `Rewrites=0` in
`RCT3Forever.ini` turns every rewrite off, which is useful for comparing.

**Calling conventions** (the game was built with Microsoft Visual C++ 2003, 32-bit):

| The game's function | Write it as |
|---|---|
| member function (`this` in `ecx`, arguments on the stack, callee pops) | `__fastcall f(T *self, void *unused_edx, args...)` |
| `__stdcall` / `__cdecl` | the same convention |
| arguments in registers other than `ecx` / `edx` | a small `naked` entry in assembly that moves them, then calls the C++ |

Floating-point results come back in `st(0)` with every convention, as in the game.

## Rewritten so far

| Function | Address | What it does | Checked in the game |
|---|---|---|---|
| `GraphedValue::Push` | `0x611af0` | Records a new value of a park statistic (rating, guests, cash, profit...) in its 64-entry history, newest first | Sandbox park, 2026-10-24: 20 calls in the first 4 minutes. The Park & Peep Editor's guests-in-park (the newest graph value) matched the game's toolbar counter, 31, and adults + teenagers added up |

## Good next candidates

Small functions that are already understood (see [the exe map](exe-map.md) and the
[research notes](research-physics-and-guests.md)):

- Queue appeal (`0x532fd0`), a formula over the queue's length.
- Park entrance picking (`0x708960` count, `0x708970` by index, `0x708980` random choice).
- The ride-choice weights and score parts (`0x76e3a0`, `0x76e010`, `0x76e210`).
- One train physics step (`0x567390`) is bigger, but fully described in the research notes.
