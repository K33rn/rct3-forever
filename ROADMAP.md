# Roadmap

✅ done · 🔨 in progress · ⬜ planned

## At a glance

| | Milestone | Status |
|---|---|---|
| ✅ | Launcher: finds the game, checks the version, starts RCT3 with Forever loaded | Done |
| ✅ | `forever.dll`: runs inside the game, puts the rewritten functions in place, leaves the game folder untouched | Done |
| ✅ | Equivalence tester: checks every rewritten function against the original before it is used | Done |
| ✅ | Cheats Menu: Guests and Park tabs, built in and on by default | Done |
| ✅ | Understanding and documenting the game's code (`docs/notes`) | **Done**: 100% of the game's code is documented in `docs/notes` (25.42% of it also rewritten; `tools/rewritten_notes.py` keeps notes for the rewritten copies). Many rows are "In outline:" descriptions to revisit before rewriting |
| 🔨 | Rewriting the game, one function at a time | **In progress**: 12535 functions, 25.42% of the game's code |
| ⬜ | Cheats Menu: a third tab, **Debug**, with the game's own developer tools | Next |
| ⬜ | Smooth frame rates: full 60 and 120 fps | After the game is fully rewritten |

The [progress page](docs/progress.md) has the current numbers, updated with every change.

## Part 1: where things stand

### ✅ Cheats Menu

Built into RCT3 Forever and on by default. Press F9 in a park to open it.

| | Tab | What it does |
|---|---|---|
| ✅ | Guests | Pick any guest and view or change them: overview, feelings and needs, money, thoughts, preferences, memory, path; send them somewhere; lock values; changes for every guest at once (happy and healthy, needs filled, cash) |
| ✅ | Park | The park's live statistics, its money (cash, loan, bank, land and construction-rights prices), appeal, opening hours, entrance fee, and new guests of any group type |
| ⬜ | Debug | The game's developer tools, see Part 2 |

### 🔨 RCT3 Forever

| | Part | Status |
|---|---|---|
| ✅ | Launcher (checks the game version, starts RCT3 with Forever loaded) | Done |
| ✅ | `forever.dll` (runs inside the game, puts the rewritten functions in place, leaves the game folder untouched) | Done |
| ✅ | Equivalence tester (checks every rewritten function against the original before it is used) | Done |
| ✅ | Understanding and documenting the game's code | Done (see above) |
| 🔨 | Rewriting the game, one function at a time | In progress (see above) |

## Part 2: what's planned

### ⬜ Next: the Debug tab

The game's developers left their own tools in RCT3: about 120 commands in a key group called `ProgrammerAids`,
normally hidden until the special name `jbspecial` is entered. They will become the Cheats Menu's third tab,
**Debug**, next to Guests and Park, there to use without any code.

**No overlap with what the menu already does.** Where a developer command does what the Guests or Park tab already
does, the two are combined in the tab that has it, instead of appearing twice:

| Developer command | Combined into | How |
|---|---|---|
| `AddMoney` | Park tab, money | The Park tab already sets the park's cash and loan; the command becomes a quick "add money" button there |
| `AddPeople` | Park tab, new guests | The Park tab already brings in new guests of any group type; the command joins that, not a second spawn button |
| `ResetPeople`, `KillAllPeople` | Guests tab, changes for every guest | With the other everyone-at-once changes (`KillAllPeople` with a confirmation) |

The rest are new, and the Debug tab has them, grouped:

| Group | Commands (the game's own names) |
|---|---|
| Park | `BuyAllLand`, `InventAll`, `UnlockAllScenarios`, `UnlockCoasterDesigns` |
| Guests (display and behaviour) | `SlowMoPeople`, `CycleAnimsPeople`, `CycleOneAnimPeople`, `ToggleGuestGroupBehaviourFreq`, `ResetSoldItemNames` |
| Rides | `BreakAllRides`, `ResetAttractions`, `ToggleRideRatingsKeys`, `IncRideStatsDisplayState`, ride camera tuning (`IncRideCamParameter`, `RideCamToggleFixedOrient`, ...) |
| Animals | `CreateDuckFlock`, `CraeteSeagull` (spelt so in the game), `KillAnimals`, `ResetAnimalStats`, `UpdateAnimalStats` |
| Time and weather | `ResetTime`, `GameSlowMo`, `WeatherTypeChange`, the rain controls (`RainIncreaseRate`, `RainToggleDebugRender`, ...) |
| Litter | `DropLitter`, `RemoveAllLitter` |
| Camera | `FlyingCamera`, `CamFlyMode` |
| Fireworks | `ToggleFireworks`, `ToggleFireworksAltPath` |
| Display and performance | `ToggleFPS`, `TogglePerfGraphs`, `ToggleDetailTimings`, `ToggleRenderStats`, `ToggleUI`, `ToggleTerrain`, `WireFrameChange`, `ToggleDummyTextures`, `CycleDebugTextRendering`, `ToggleDebugInfoEditor` |
| Water | water flow, waterfall, water jet and droplet debugging (`WaterFlowToggleDebugRender`, `WaterfallRebuild`, `WaterJetReset`, ...) |
| Attract mode and tutorials | `ToggleAttractSequence`, `StartAttractSequence`, `TutorialSetup`, `OpenScenarioStartAttractMenu` |

| | Step |
|---|---|
| ✅ | Find the commands: the ones above, with the special name kept at `0x10EC6B0` in the game's data |
| ⬜ | Work out how `jbspecial` unlocks the commands and how each command runs when its key is pressed |
| ⬜ | Combine the overlapping commands into the Guests and Park tabs (the first table) |
| ⬜ | Add the Debug tab: one button or switch per remaining command, grouped as above, calling the game's own code directly so no name or key is needed |
| ⬜ | Check each command in game and mark the unsafe ones (some only make sense for the developers, and some may break a park) with a warning or a confirmation before they run |
| ⬜ | Add a setting in `RCT3Forever.ini` to hide the Debug tab for anyone who doesn't want it |

### ⬜ Next major update: smooth frame rates

Full **60 fps** and **120 fps** support. This comes once the whole game is rewritten: the frame timing runs through
the game's own loop, its simulation steps and its drawing, and Forever needs all of that as its own code to change
how often a frame is made without changing how the park behaves.

| | Step |
|---|---|
| ⬜ | The whole game rewritten |
| ⬜ | The game's frame loop, simulation timing and drawing understood as one piece |
| ⬜ | A frame-rate setting in `RCT3Forever.ini`: 60 or 120 fps |
| ⬜ | Checked in game: parks behave the same at every frame rate (guests, rides and money move at the game's speed, not the frame rate) |

### 🔨 Rewriting and understanding

| | Step |
|---|---|
| ✅ | The whole game understood and documented in `docs/notes` |
| 🔨 | Keep rewriting the game's functions, each one checked by the equivalence tester and in game before it is used |
| ⬜ | Revisit the "In outline:" notes before rewriting those functions |
| ⬜ | The whole game rewritten |
