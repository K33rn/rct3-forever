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
   bit, with the FPU at 24-, 53- and 64-bit precision. A rewrite without a test, or a test that crashes, fails the run.
   To check only some rewrites, set `EQUIV_ONLY` to part of their name (`EQUIV_ONLY=Pool`); `EQUIV_VERBOSE=1` prints
   each copy's address as it is checked (useful when a test hangs).
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

**Functions with many copies.** The compiler made a separate copy of a template function for every type that uses
it, so many functions exist dozens or hundreds of times, identical apart from where they sit.
`python tools/dupes.py "<game folder>\RCT3plus.exe"` lists the biggest groups of copies still to do; with an address
after it, it prints every copy of that function as a C array. One rewrite then replaces them all:

```cpp
static const uint32_t unlink_copies[] = { 0x40a890, 0x468370, /* ... */ };
REWRITE_COPIES("ListNode::Unlink", unlink_copies, unlink_original, unlink, &g_unlinks);
```

The equivalence test runs on every copy. Rewrites that call the game's memory functions (`0xbce000` free,
`0xbce010` free of an aligned block, `0xbcdfd0` aligned allocation) or the handle functions (`0x8055c0` release,
`0x805750` the null handle, `0x805650` a handle by id, `0x808840` resolving a saved id, `0x809210` reading the save
stream) are tested with stand-ins that record each call, so what gets allocated and freed, and in
which order, is compared too.

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

Template rewrites (`forever/src/rewrite/templates.cpp`) cover functions the compiler made hundreds of times that
differ only in an address; `tools/gen_rewrites.py` lists each exact copy from your exe in `generated.inc`
(addresses and first bytes only), and the equivalence test checks every copy, with the game function each one
calls stood in for while it is checked.

All rewrites pass the equivalence test: 300 random cases at each of the three FPU precisions, or 20 for each copy of a
function with many copies.

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
| `OwnedPointer::Reset` | 345 copies (e.g. `0x410960`) | Frees what an owned pointer holds and clears it | Vanilla Hills: called |
| `OwnedPair::Free` | 20 copies (e.g. `0x408190`) | Frees the two blocks an object owns | (equivalence test on every copy) |
| `SharedBuffer::Release` | 16 copies (e.g. `0x4012e0`), 12,475 call sites | Drops a reference to a shared buffer; the last one frees its data and the buffer | Vanilla Hills: 2,592 calls while the park opened |
| `ListNode::Unlink` | 26 copies (e.g. `0x40a890`) | Takes a node out of a two-way list and clears it; a broken list stops at a breakpoint, as in the game | Vanilla Hills: 2,838 calls |
| `BaseClass::SetClass` | 140 copies (e.g. `0x441fc0`) | Sets an object's virtual function table to one base class's (`0xf28db8`) | Vanilla Hills: 819,660 calls |
| `Array<size>::SetCapacity` | 193 copies for items of 2, 4, 6, 8, 12, 16, 20 and 24 bytes (e.g. `0x417c20` for 4) | Reallocates a growable array to n items, keeping the first min(n, capacity) | Vanilla Hills: 45,515 calls for 4-byte items, 1,741 for 16, 50 for 20, 42 for 8, 12 for 12 |
| `Array<size>::Allocate` | 67 copies for items of 1, 2, 4, 8, 16 and 44 bytes (e.g. `0x41b910` for 4) | Drops an array's contents and makes room for n items | Vanilla Hills: called |
| `ObjectRef::Assign` | 108 copies (e.g. `0x410100`) | Makes a reference point at what another one points at: leaves the old handle's list of references, releases it, joins the new one's | Vanilla Hills: 130,831 calls |
| `ObjectRef::DeleteTarget` | 31 copies (e.g. `0x490800`) | Deletes the object a reference points at and points it at nothing | Vanilla Hills: 191 objects deleted through it |
| `ReferenceField::Load` | 90 copies (e.g. `0x44eeb0`) | Reads a reference from a save file: the saved object id (from the stream's buffer, or by the stream's read `0x809210`), turned into the loaded object's id (`0x808840`), then points the reference at that object's handle (`0x805650`) | Vanilla Hills: 4,631 references loaded through it while the scenario opened |
| `ObjectRef::Construct` | 13 copies (e.g. `0x42e030`) | Makes a new reference (class `0xf26ae0`) pointing at nothing: joins the null handle's list of references | Vanilla Hills: 100,805 references made |
| `Pool::Free` | 20 copies (e.g. `0x8501c0`) | Puts an item back on its pool's free list; when every item of the pool is free, the pool's blocks go back to memory | Vanilla Hills: 2,877,853 calls in 6 minutes |
| `StructArrayField<size>::Size` | 62 copies for structs of 3 to 116 bytes (e.g. `0x460a90` for 28) | Bytes an array of structs saves as: 8 (its length and count) plus each struct's fields' sizes | Vanilla Hills saved: 3,600 calls over 11 struct sizes |
| `StructArrayField<size>::Save` | 59 copies for structs of 3 to 116 bytes (e.g. `0x460960` for 28) | Saves an array of structs: its length, its count, then each struct's fields, keeping the save file's running checksum | Vanilla Hills saved through it (1.5 MB) and loaded back intact |
| `DeletingDestructor` | 2,321 copies (template `deleting<destructor>`, listed in `generated.inc`) | What `delete` calls through a class's table: runs the class's destructor, frees the object when bit 0 of the flag is set, returns it | Vanilla Hills: 1,011,518 calls |
| `ModuleGlobal` | 871 copies (template `module_global<start, global, destructor>`) | A global built before `main`: starts a list of modules (`0xBCE1A0`, `0x805AB0`, ...), constructs the global from the result (`0xBCDF40`, which throws the error unless it is -1) and registers its destructor with `atexit` | All 871 run at every start; the game starts and Vanilla Hills plays normally |
| `QueryInterface` | 194 copies (template `query<id>`) | Asks an object's class (virtual slot 1) for the interface with the id in a global: the class's table by id (+0x14, count +0x18), the interface's offset (+0x10); null when missing | Vanilla Hills: 486,554 calls |
| `Attraction::IdealGuests` | `0xa88450` | A non-ride attraction's share of IdealGuests: 3 x its value at +0x70 | (equivalence test; the parks tried have no such attraction) |

## Good next candidates

Small functions that are already understood (see [the exe map](exe-map.md) and the
[research notes](research-physics-and-guests.md)):

- Queue appeal (`0x532fd0`), a formula over the queue's length.
- The ride-choice weights and score parts (`0x76e3a0`, `0x76e010`, `0x76e210`).
- One train physics step (`0x567390`) is bigger, but fully described in the research notes.
