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
| `RCT3Forever.exe` (launcher) | Asks for the game folder once (opening at the Steam copy when there is one), checks it's the supported version, starts it with Forever loaded before any of the game's own code runs |
| `forever.dll` | Runs inside the game: checks the version again, turns on the Cheats Menu, puts the rewritten functions in place |
| Cheats Menu | The in-game menu (F9) for guests and the park, on by default |
| Rewritten functions | 5,205 so far: every deleting destructor, interface lookup (also cached), start-up module global, struct-array load, global clean-up, reference constructor, object-array free and table setter that has the template's exact shape (templates checked on each of their 4,002 copies), the game's random number generator, the park's statistics graphs, park entrances, guests' taste for intense rides, parts of how they choose rides, growable arrays of every item size, references between objects and reading them from save files, saving arrays of structs (with the save file's checksum), object pools, and the memory and list helpers the compiler copied hundreds of times. Each one is proven bit-for-bit identical to the original by the equivalence test. See [the rewrite list](docs/rewriting.md#rewritten-so-far) |

## Progress

How much of the game is rewritten. Every function in RCT3plus.exe is counted: **rewritten** (Forever's code
runs), **documented** (understood and described, next to rewrite), or **to do**. Library code that ships inside the exe
and pieces the compiler split off from functions are counted separately; they don't need rewriting.

<!-- progress -->
```
Rewritten    █████████████▏░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  26.34%
Understood   ██████████████████████████████████████████████████ 100.00%
             0%        20%       40%       60%       80%       100%
```

**26.34% of the game's code is rewritten** and 100.00% is understood and documented (by code size; understood includes the rewritten part).

| Status | Functions | Code | Share of the game's code |
|---|---:|---:|---:|
| Rewritten | 13,010 | 2,190,277 bytes | 26.34% |
| Documented, not rewritten yet | 12,709 | 6,126,472 bytes | 73.66% |
| To do | 0 | 0 bytes | 0.00% |
| **Game total** | **25,719** | **8,316,749 bytes** | |
| Library code from Microsoft (stays as is; see [library.md](docs/notes/library.md)) | 2,850 | 874,041 bytes | |
| Compiler-made pieces (go with their function) | 42,200 | 506,334 bytes | |

Details: [docs/progress.md](docs/progress.md).
<!-- /progress -->

## Install and play

1. Download **`RCT3Forever-0.1-setup.exe`** and run it. It installs RCT3 Forever into `Program Files`, with Start
   Menu shortcuts (and a desktop one if you tick it) and an uninstaller in Windows' *Apps* list.
2. Start **RCT3 Forever** from the Start Menu (or the last page of the installer). The first time, it asks you to select
   the game folder (the one with `RCT3plus.exe`). If the game is in your Steam libraries, the picker opens there, so
   you only need to press *Select folder*. Your choice is remembered.
3. The game starts. In a park, the top-left corner shows "RCT3 Forever - Cheats Menu: press F9 in a park".

Start RCT3 through RCT3 Forever whenever you want Forever. Starting the game from Steam as usual gives you the plain
game. To pick a different game folder later, hold **Shift** while starting RCT3 Forever, or use the Start Menu's
*RCT3 Forever (choose the game folder)*.

**Without installing:** `RCT3Forever-0.1.zip` has the same program. Unzip it anywhere and double-click
`RCT3Forever.exe`. Keep its files together: with the `RCT3Forever.ini` it comes with, that copy keeps its settings and
logs in its own folder.

**Supported game:** `RCT3plus.exe` from RollerCoaster Tycoon 3 Platinum / Gold, the version with Soaked! and Wild!
(Steam app 2700, or the disc), unpatched. Edits made by mod tools that leave the program's code alone (such as a
changed front-screen menu) are fine. *RollerCoaster Tycoon 3: Complete Edition* (2020) is a different program and
isn't supported.

## Settings (`RCT3Forever.ini`)

An installed copy keeps its settings in `%APPDATA%\RCT3 Forever\RCT3Forever.ini` (paste that into Explorer's address
bar), made from the defaults on the first start. The zip's copy uses the `RCT3Forever.ini` next to it. Uninstalling
leaves the settings file, so a reinstall remembers them.

| Setting | Default | Meaning |
|---|---|---|
| `[Game] Folder` | chosen on the first start | The game folder |
| `[Forever] CheatsMenu` | `1` | `0` turns the Cheats Menu off |
| `[Forever] Rewrites` | `1` | `0` runs the game's original code instead of every rewritten function, for comparing |
| `[CheatsMenu] ToggleKey` | `F9` | Key that opens the menu: F1–F12, Insert, Home, End, Delete, PageUp, PageDown, Pause, ScrollLock |
| `[CheatsMenu] UiScalePercent` | `110` | Menu size |
| `[CheatsMenu] HookGameInput` | `1` | `0` if the game crashes on start (the game then also sees clicks on the menu) |

A settings file from before the menu's rename still works: its `[Forever] PeepEditor` and `[PeepEditor]` settings
are read when the new names aren't there.

`RCT3Forever.log` (in the same folder as the settings) shows what Forever did: the version check, each rewritten function put in
place, and how often each one has run. `CheatsMenu.log` is the menu's own log.

## If something goes wrong

- **"This RCT3plus.exe isn't the version RCT3 Forever supports":** see *Supported game* above.
- **The game runs but there's no F9 menu:** look in `RCT3Forever.log`. `unknown game version` means the exe isn't
  the supported one. `Cheats Menu off` means `CheatsMenu=0` in the settings.
- **The game crashes with Forever but not without it:** set `Rewrites=0` and try again. If that helps, one of the
  rewritten functions is at fault. Please report it with your `RCT3Forever.log`.

## How it works

The game is a 32-bit Windows program that always loads at the same address and has no copy protection wrapper.
The launcher starts it paused, holds it at its entry point until Windows has loaded the game's libraries, loads
`forever.dll` into it, and lets it run. `forever.dll`:

- checks the exe's build stamp (`0x434E4089`, linked 2005-10-13) and size, and does nothing on any other version;
- points the game's import of `Direct3DCreate9` at the Cheats Menu, which draws itself on the game's
  Direct3D 9 device;
- for each rewritten function, checks the original's first bytes and writes a jump to Forever's version over them,
  so every call in the game goes to the new code.

## Building

Windows, with [uv](https://docs.astral.sh/uv/) (it fetches Python and Zig's C++ compiler):

```
uv run --no-project --with ziglang --with pefile python build.py
```

This makes `out/forever.dll`, `out/RCT3Forever.exe`, `dist/RCT3Forever-0.1.zip` and the installer
`dist/RCT3Forever-0.1-setup.exe` (made with [NSIS](https://nsis.sourceforge.io) from `installer/RCT3Forever.nsi`).
`python build.py test` (same `uv run` prefix) checks every rewritten function against the game's own one, using
your RCT3plus.exe (see [docs/rewriting.md](docs/rewriting.md)). Dear ImGui and NSIS are downloaded into
`third_party/` on the first build (NSIS is checked against its known SHA-256). To try a build, copy `forever/RCT3Forever.ini` into `out/` and run
`out/RCT3Forever.exe`.

| Folder | Contents |
|---|---|
| `launcher/` | The launcher |
| `installer/` | The installer script (NSIS) |
| `forever/src/` | `forever.dll`: start-up, version check, function replacement |
| `forever/src/rewrite/` | The rewritten game functions, one file per part of the game |
| `cheatsmenu/` | The Cheats Menu (built into `forever.dll`) |
| `docs/` | [Progress](docs/progress.md) and [the function list](docs/functions.csv), [rewriting functions](docs/rewriting.md), [the map of RCT3plus.exe](docs/exe-map.md), [research notes](docs/research-physics-and-guests.md) |
| `tools/progress.py` | Updates the progress numbers: run it after adding a rewrite or documenting a function |
| `tools/dupes.py` | Finds functions the compiler copied many times, for one rewrite to replace them all |
| `tools/equiv/` | The equivalence tester: runs original and rewritten functions side by side on random inputs |

## Contributing a rewritten function

See [CONTRIBUTING.md](CONTRIBUTING.md), then [docs/rewriting.md](docs/rewriting.md). In short, you work out what a game
function does, then write new code that does the same. Never paste decompiler output, and never commit anything from
the game: no game files, no copied code. Then prove in the game that it behaves the same.

## Legal

RCT3 Forever is an independent fan project, not affiliated with or endorsed by Frontier Developments or Atari.
RollerCoaster Tycoon is their trademark. The project contains no game code, files or assets. It only runs on a
copy of the game you own, and it changes the running game in memory, never the game's files. Its code was written
for this project and is under the [MIT license](LICENSE), which covers RCT3 Forever's own code only, not RollerCoaster
Tycoon 3 or any file from the game. Dear ImGui (third_party/, downloaded when building) keeps its own MIT license.

## Credits

- K3rn, author.
- Written with the help of Claude (Anthropic).
- Dear ImGui (Omar Cornut) draws the Cheats Menu.
- The RCT3 Importer / libOVL authors (Jonathan Wilson, belgabor et al.), for their structure research.
- OpenRCT2, the inspiration for rewriting a game step by step on top of the original.
