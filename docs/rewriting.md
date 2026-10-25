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
4. **Prove it's the same: the equivalence test.** Add a test for it to `tools/equiv/equiv.cpp` and run
   `python build.py test`. The tester loads your RCT3plus.exe at the address the game runs at, runs the original and
   the rewrite on hundreds of random inputs, and requires the same result and the same memory afterwards, bit for
   bit, with the FPU at 24-, 53- and 64-bit precision. A rewrite without a test fails the run.
5. **Prove it in the game.** Run the game with the rewrite until `RCT3Forever.log` shows it being called, and check
   the game behaves normally. Write down how in the table below.
6. **Update the progress:** `python tools/progress.py` picks up the new `REWRITE(...)` and refreshes
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

Floating-point results come back in `st(0)` with every convention, as in the game. Forever is compiled for the x87
FPU (`-march=i686`, no SSE), as the game was, so the same float expression rounds the same way. Write float formulas
in the same order of operations as the game does: `(a * 5 + v) * (1/6)` and `(a * 5 + v) / 6` can differ in the
last bit, and the equivalence test will catch it.

A rewrite calls other game functions through their addresses (for example
`(float(__cdecl *)())0x475180`), so it uses whichever version is in place: the game's or Forever's.

## Rewritten so far

All rewrites pass the equivalence test (300 random cases at each of the three FPU precisions).

| Function | Address | What it does | Checked in the game |
|---|---|---|---|
| `GraphedValue::Push` | `0x611af0` | Records a new value of a park statistic (rating, guests, cash, profit...) in its 64-entry history, newest first | Sandbox, 2026-10-24: guests in park from the graph = toolbar (31). Vanilla Hills: 6,005 calls |
| `Random::SetState` | `0x475170` | Sets the generator's state (in its stored form) | Called when a park starts |
| `Random::GetState` | `0x475220` | Returns the state in its stored form | (equivalence test) |
| `Random::Advance` | `0x4751e0` | One MINSTD step (x 48271 mod 2^31 - 1); returns the new stored state | Vanilla Hills: 38,000+ calls while the park opened |
| `Random::Unit` | `0x475180` | One step, then a number in [0, 1) from the state's low 23 bits | Sandbox: 1.27 million calls in 3 minutes, guests spawned (17 -> 55) and walked in normally |
| `ParkEntrances::Count` | `0x708960` | How many park entrances there are | (equivalence test) |
| `ParkEntrances::At` | `0x708970` | The entrance at an index | (equivalence test) |
| `ParkEntrances::Random` | `0x708980` | An entrance picked with equal chances | (equivalence test; nothing in the exe calls it directly) |
| `Guest::SetIntensityPreference` | `0x758bb0` | Sets the 0-3 preference and, optionally, the fixed intensity band for it | Vanilla Hills: 306 calls for 306 new guests |
| `Guest::IdealIntensity` | `0x758c30` | Min + f x (Max - Min), f = 0.25 / 0.5 / 0.7 / 0.9 | Vanilla Hills: 732,409 calls in 3 minutes of ride choices |
| `Object::WeatherInterface` | `0x49ac80` | The object's weather interface (id at `0x13cde74`), or null | Vanilla Hills: 1,334 calls |
| `Object::FitInterface` | `0x4bd9d0` | The object's fit interface (id at `0x13d0680`), or null | Vanilla Hills: 777 calls |
| `Ride::LogScorePart` | `0x4bfc00` | Running average of a part of the guests' score for the ride: (5 x log + value) / 6 | Vanilla Hills: 4,319 calls |
| `Attraction::IdealGuests` | `0xa88450` | A non-ride attraction's share of IdealGuests: 3 x its value at +0x70 | (equivalence test; the parks tried have no such attraction) |

## Good next candidates

Small functions that are already understood (see [the exe map](exe-map.md) and the
[research notes](research-physics-and-guests.md)):

- Queue appeal (`0x532fd0`), a formula over the queue's length.
- The ride-choice weights and score parts (`0x76e3a0`, `0x76e010`, `0x76e210`).
- One train physics step (`0x567390`) is bigger, but fully described in the research notes.
