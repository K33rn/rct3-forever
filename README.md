# RCT3 Forever

```
Project:  RCT3 Forever
Author:   K3rn
For:      RollerCoaster Tycoon 3 Platinum / Gold (with Soaked! and Wild!), Steam or disc
Version:  0.1
```

RCT3 Forever keeps RollerCoaster Tycoon 3 alive by **rewriting the game, one function at a time, as open-source
code**. It runs on top of your own copy of the game. A launcher starts RCT3 with Forever loaded. Every function
Forever has rewritten then runs Forever's code instead of the original, and the rest of the game carries on as
before. As more functions are rewritten, more of the game becomes open source that anyone can read, fix and
improve.

**No game files are included.** You need your own RollerCoaster Tycoon 3. RCT3 Forever doesn't change anything in
the game folder: everything happens in memory while the game runs, and closing it leaves the game exactly as it was.

See the [roadmap](ROADMAP.md) for where things stand and what's planned next, including a Debug tab with the
game's own developer tools.

## What's in 0.1

| Part | What it does |
|---|---|
| `RCT3Forever.exe` (launcher) | Finds the game (Steam, or asks once), checks it's the supported version, starts it with Forever loaded before any of the game's own code runs |
| `forever.dll` | Runs inside the game: checks the version again, turns on the Park & Peep Editor, puts the rewritten functions in place |
| Park & Peep Editor | The in-game editor (F9), on by default |
| Rewritten functions | 5,205 so far: every deleting destructor, interface lookup (also cached), start-up module global, struct-array load, global clean-up, reference constructor, object-array free and table setter that has the template's exact shape (templates checked on each of their 4,002 copies), the game's random number generator, the park's statistics graphs, park entrances, guests' taste for intense rides, parts of how they choose rides, growable arrays of every item size, references between objects and reading them from save files, saving arrays of structs (with the save file's checksum), object pools, and the memory and list helpers the compiler copied hundreds of times. Each one is proven bit-for-bit identical to the original by the equivalence test. See [the rewrite list](docs/rewriting.md#rewritten-so-far) |

## Progress

How much of the game is rewritten. Every function in RCT3plus.exe is counted: **rewritten** (Forever's code
runs), **documented** (understood and described, next to rewrite), or **to do**. Library code that ships inside the exe
and pieces the compiler split off from functions are counted separately; they don't need rewriting.

<!-- progress -->
`█▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░` **3.04% rewritten**, 73.22% understood and documented (by code size)

| Status | Functions | Code | Share of the game's code |
|---|---:|---:|---:|
| Rewritten | 5,205 | 252,685 bytes | 3.04% |
| Documented | 10,419 | 6,089,053 bytes | 73.22% |
| To do | 10,094 | 1,974,929 bytes | 23.75% |
| **Game total** | **25,718** | **8,316,667 bytes** | |
| Library code from Microsoft (stays as is; see [library.md](docs/notes/library.md)) | 2,850 | 874,041 bytes | |
| Compiler-made pieces (go with their function) | 42,200 | 506,334 bytes | |

Details: [docs/progress.md](docs/progress.md).
<!-- /progress -->

## Install and play

1. Download `RCT3Forever-0.1.zip` and unzip it anywhere, e.g. `Documents\RCT3Forever`. Keep the three files
   (`RCT3Forever.exe`, `forever.dll`, `RCT3Forever.ini`) together.
2. Double-click **`RCT3Forever.exe`**. The first time, it looks for the game in your Steam libraries. If it can't
   find it, it asks you to pick the folder that has `RCT3plus.exe`, and remembers your choice in `RCT3Forever.ini`.
3. The game starts. In a park, the top-left corner shows "RCT3 Forever - Park & Peep Editor: press F9 in a park".

Start RCT3 through `RCT3Forever.exe` whenever you want Forever. Starting the game from Steam as usual gives you the
plain game.

**Already using the standalone Park & Peep Editor?** Disable it in the Mod Loader (or remove `d3d9.dll` and
`PeepEditor.ini` from the game folder), because Forever has it built in. If both are there, the launcher warns you
and offers to start with only the standalone one.

**Supported game:** `RCT3plus.exe` from RollerCoaster Tycoon 3 Platinum / Gold, the version with Soaked! and Wild!
(Steam app 2700, or the disc), unpatched. Edits made by mod tools that leave the program's code alone (such as a
changed front-screen menu) are fine. *RollerCoaster Tycoon 3: Complete Edition* (2020) is a different program and
isn't supported.

## Settings (`RCT3Forever.ini`)

| Setting | Default | Meaning |
|---|---|---|
| `[Game] Folder` | found on the first start | The game folder |
| `[Forever] PeepEditor` | `1` | `0` turns the Park & Peep Editor off |
| `[Forever] Rewrites` | `1` | `0` runs the game's original code instead of every rewritten function, for comparing |
| `[PeepEditor] ToggleKey` | `F9` | Key that opens the editor: F1–F12, Insert, Home, End, Delete, PageUp, PageDown, Pause, ScrollLock |
| `[PeepEditor] UiScalePercent` | `110` | Editor size |
| `[PeepEditor] HookGameInput` | `1` | `0` if the game crashes on start (the game then also sees clicks on the editor) |

`RCT3Forever.log` (next to `forever.dll`) shows what Forever did: the version check, each rewritten function put in
place, and how often each one has run. `PeepEditor.log` is the editor's own log.

## If something goes wrong

- **"This RCT3plus.exe isn't the version RCT3 Forever supports":** see *Supported game* above.
- **The game runs but there's no F9 menu:** look in `RCT3Forever.log`. `unknown game version` means the exe isn't
  the supported one. `Park & Peep Editor off` means `PeepEditor=0`, or the standalone editor is installed.
- **The game crashes with Forever but not without it:** set `Rewrites=0` and try again. If that helps, one of the
  rewritten functions is at fault. Please report it with your `RCT3Forever.log`.

## How it works

The game is a 32-bit Windows program that always loads at the same address and has no copy protection wrapper.
The launcher starts it paused, holds it at its entry point until Windows has loaded the game's libraries, loads
`forever.dll` into it, and lets it run. `forever.dll`:

- checks the exe's build stamp (`0x434E4089`, linked 2005-10-13) and size, and does nothing on any other version;
- points the game's import of `Direct3DCreate9` at the Park & Peep Editor, which draws its menu on the game's
  Direct3D 9 device;
- for each rewritten function, checks the original's first bytes and writes a jump to Forever's version over them,
  so every call in the game goes to the new code.

## Building

Windows, with [uv](https://docs.astral.sh/uv/) (it fetches Python and Zig's C++ compiler):

```
uv run --no-project --with ziglang --with pefile python build.py
```

This makes `out/forever.dll`, `out/RCT3Forever.exe` and `dist/RCT3Forever-0.1.zip`.
`python build.py test` (same `uv run` prefix) checks every rewritten function against the game's own one, using
your RCT3plus.exe (see [docs/rewriting.md](docs/rewriting.md)). Dear ImGui is downloaded into
`third_party/` on the first build. To try a build, copy `forever/RCT3Forever.ini` into `out/` and run
`out/RCT3Forever.exe`.

| Folder | Contents |
|---|---|
| `launcher/` | The launcher |
| `forever/src/` | `forever.dll`: start-up, version check, function replacement |
| `forever/src/rewrite/` | The rewritten game functions, one file per part of the game |
| `peepeditor/` | The Park & Peep Editor (built with `FOREVER` defined for Forever; its own `build.py` makes the standalone `d3d9.dll`) |
| `docs/` | [Progress](docs/progress.md) and [the function list](docs/functions.csv), [rewriting functions](docs/rewriting.md), [the map of RCT3plus.exe](docs/exe-map.md), [research notes](docs/research-physics-and-guests.md) |
| `tools/progress.py` | Updates the progress numbers: run it after adding a rewrite or documenting a function |
| `tools/dupes.py` | Finds functions the compiler copied many times, for one rewrite to replace them all |
| `tools/equiv/` | The equivalence tester: runs original and rewritten functions side by side on random inputs |

## Contributing a rewritten function

Read [docs/rewriting.md](docs/rewriting.md) first. In short, you work out what a game function does, then write new
code that does the same. Never paste decompiler output, and never commit anything from the game: no game files,
no copied code. Then prove in the game that it behaves the same.

## Legal

RCT3 Forever is an independent fan project, not affiliated with or endorsed by Frontier Developments or Atari.
RollerCoaster Tycoon is their trademark. The project contains no game code, files or assets. It only runs on a
copy of the game you own, and it changes the running game in memory, never the game's files. Its code was written
for this project and is under the [MIT license](LICENSE). Dear ImGui is under its own MIT license.

## Credits

- K3rn, author.
- Written with the help of Claude (Anthropic).
- Dear ImGui (Omar Cornut) draws the Park & Peep Editor.
- The RCT3 Importer / libOVL authors (Jonathan Wilson, belgabor et al.), for their structure research.
- OpenRCT2, the inspiration for rewriting a game step by step on top of the original.
