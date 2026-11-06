# Roadmap

## Part 1: where things stand

### Park & Peep Editor: available

Built into RCT3 Forever and on by default. Press F9 in a park to open it.

| Tab | What it does | Status |
|---|---|---|
| Guests | Pick any guest and view or change them: overview, money and the rest of their details; lock values | Available |
| Park | The park's live statistics, its money, and spawning guests | Available |

The editor is also released on its own as a standalone mod (installed with the Mod Loader). Use one or the other:
Forever's launcher warns when both are installed.

### RCT3 Forever: in progress

| Part | Status |
|---|---|
| Launcher (checks the game version, starts RCT3 with Forever loaded) | Done |
| `forever.dll` (runs inside the game, puts the rewritten functions in place, leaves the game folder untouched) | Done |
| Equivalence tester (checks every rewritten function against the original before it is used) | Done |
| Rewriting the game, one function at a time | **In progress**: 8,755 functions, 7.82% of the game's code |
| Understanding and documenting the game's code (`docs/notes`) | **Done**: 100% of the game's code is documented in `docs/notes` (7.82% of it also rewritten; `tools/rewritten_notes.py` keeps notes for the rewritten copies). Many rows are "In outline:" descriptions to revisit before rewriting |

The [progress page](docs/progress.md) has the current numbers, updated with every change.

## Part 2: what's planned

### A third tab: Debug

The game's developers left their own tools in RCT3: about 120 commands in a key group called `ProgrammerAids`,
normally hidden until the special name `jbspecial` is entered. The plan is to bake these into the editor's menu as a
third tab, **Debug**, next to Guests and Park, so they are there to use without any code.

The commands, grouped as the Debug tab would show them:

| Group | Commands (the game's own names) |
|---|---|
| Park and money | `AddMoney`, `BuyAllLand`, `InventAll`, `UnlockAllScenarios`, `UnlockCoasterDesigns` |
| Guests | `AddPeople`, `KillAllPeople`, `ResetPeople`, `SlowMoPeople`, `CycleAnimsPeople`, `CycleOneAnimPeople`, `ToggleGuestGroupBehaviourFreq`, `ResetSoldItemNames` |
| Rides | `BreakAllRides`, `ResetAttractions`, `ToggleRideRatingsKeys`, `IncRideStatsDisplayState`, ride camera tuning (`IncRideCamParameter`, `RideCamToggleFixedOrient`, ...) |
| Animals | `CreateDuckFlock`, `CraeteSeagull` (spelt so in the game), `KillAnimals`, `ResetAnimalStats`, `UpdateAnimalStats` |
| Time and weather | `ResetTime`, `GameSlowMo`, `WeatherTypeChange`, the rain controls (`RainIncreaseRate`, `RainToggleDebugRender`, ...) |
| Litter | `DropLitter`, `RemoveAllLitter` |
| Camera | `FlyingCamera`, `CamFlyMode` |
| Fireworks | `ToggleFireworks`, `ToggleFireworksAltPath` |
| Display and performance | `ToggleFPS`, `TogglePerfGraphs`, `ToggleDetailTimings`, `ToggleRenderStats`, `ToggleUI`, `ToggleTerrain`, `WireFrameChange`, `ToggleDummyTextures`, `CycleDebugTextRendering`, `ToggleDebugInfoEditor` |
| Water | water flow, waterfall, water jet and droplet debugging (`WaterFlowToggleDebugRender`, `WaterfallRebuild`, `WaterJetReset`, ...) |
| Attract mode and tutorials | `ToggleAttractSequence`, `StartAttractSequence`, `TutorialSetup`, `OpenScenarioStartAttractMenu` |

Steps:

1. Work out how `jbspecial` unlocks the commands (the name is kept at `0x10EC6B0` in the game's data) and how each
   command runs when its key is pressed.
2. Add the Debug tab: one button or switch per command, grouped as above, calling the game's own code directly so
   no name or key is needed.
3. Check each command in game and mark the ones that are unsafe (some only make sense for the developers, and some
   may break a park), with a warning or a confirmation before they run.
4. Add a setting in `RCT3Forever.ini` to hide the Debug tab for anyone who doesn't want it.

### Rewriting and understanding

- Keep rewriting the game's functions, each one checked by the equivalence tester and in game before it is used.
- Keep documenting the game's code in `docs/notes`, working towards the whole game being understood and then
  rewritten.
