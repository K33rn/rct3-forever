# A map of RCT3plus.exe

What is known about the game program, for rewriting it. The addresses are for the supported exe: PE time stamp
`0x434E4089` (2005-10-13 11:10:01 UTC), image size `0x104C000`, always loaded at `0x400000` (no relocations, no
copy protection wrapper). Everything here was worked out by studying how the program behaves. It contains no code
from the game.

More detail on train physics, guest spawning and ride choice is in
[research-physics-and-guests.md](research-physics-and-guests.md).

## Program layout

| Section | Address | Size |
|---|---|---|
| `.text` (code) | `0x401000` | `0xB24CE5` |
| `.rdata` (constants, import table) | `0xF26000` | `0x1C0200` |
| `.data` | `0x10E7000` | `0x33809C` |
| `.rsrc` | `0x1420000` | `0x2B358` |

Entry point `0xDDF982`. Imports `d3d9.dll!Direct3DCreate9` (slot `0xF26430`) and `DINPUT8.dll!DirectInput8Create`
(slot `0xF26034`), plus KERNEL32, USER32, GDI32, ADVAPI32, SHELL32, ole32, OLEAUT32, VERSION, IMM32, DSOUND, AVIFIL32
and WINMM. Built with Microsoft Visual C++ 2003 (32-bit, x87 floating point).

## Saved fields

Every saved object registers its fields with `0x8088C0` (`push offset; push size; push "type"; push "Name"; call`).
Reading those registrations gives the layout of each object in memory:

| Object | Fields registered at |
|---|---|
| Park | `0x707B1B` |
| Park finances | `0x6BFF80` |
| Guest | `0x76BD23` (and `0x7DF8BE`) |
| Train | `0x56C4BB` |

## Objects

| What | Where | Notes |
|---|---|---|
| Park | `[0x1306194]` | getter `0x708520`, constructor `0x707680`, size `0x1F0` |
| Park finances | `[0x1305DD8]` | getter `0x6C1DA0`, constructor `0x6BFCE0`, size `0x1D8`. Cash `+0x58`, Loan `+0x5C`, MaxLoan `+0x60`, InterestRate `+0x64` (float), EntranceFee `+0x114` (int32, cents) |
| Attraction manager | `[0x12F4E4C]` | list at `+0x64` |
| Guest demographics | `[0x1306568]` | group type weights at `+0x70`..`+0x8C` |
| Renderer | vtable `0xFBF138` | the game's main camera hangs off it |
| Guest | vtables `0xF6C948` / `0xF74FA0` | plain and special guests |
| Thought names | `0x10EDE48` | 256 entries |

**GraphedValue** (a statistic's history, drawn in the park windows and saved): `{float *data; int index; int
capacity; int count; flag}`. The current value is `data[index]`; capacity is 64 for the park's graphs. Pushed by
`0x611AF0` (rewritten in Forever).

## Functions

| Address | What it does | In Forever |
|---|---|---|
| `0x611AF0` | GraphedValue::Push: newest slot back one (wrapping), store, count up to capacity | **rewritten** |
| `0x781DA0` | Makes a guest group of a type (fastcall, `ecx` = type 0–7, 11 = weighted random) at an injection point and sends it to an entrance; returns the group or 0 | used by the Peep Editor |
| `0x43D940` | The park's update (guest arrivals from `0x43DF37`) | |
| `0x437A10` | Works out the park's attractiveness numbers each update | |
| `0x704AB0` | The park's "guest added" handler (counts GuestsInPark, intake) | |
| `0x762A00` | A new guest's preferences | |
| `0x76E3A0` | A guest's score for a ride (wrapped by `0x76EE40`) | |
| `0x75F3A0` | After a ride: did the guest like it, favourite ride | |
| `0x567390` | One train physics step | |
| `0x7778E0` | Guest group route planner (to `0x772DE0`) | used by the Peep Editor |
| `0x8055C0` / `0x805700` | Drop / make an object handle | used by the Peep Editor |
| `0xBCDFC0` | The game's memory allocator | used by the Peep Editor |
| `0x410F20` | Registers the 32 cheats (guest names) | |

The research notes list many more, with what each one computes.
