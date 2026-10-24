PEEP EDITOR for RollerCoaster Tycoon 3 (Platinum / Complete, Steam or disc with Soaked! and Wild!)
====================================================================================================

An in-game menu to view and change any guest ("peep") in your park: their money, feelings, needs,
ride preferences, where they are walking, what they are thinking, and every other value they carry.

INSTALL
  1. Close the game.
  2. Copy d3d9.dll and PeepEditor.ini from this zip into the game folder, next to RCT3plus.exe.
     Steam: C:\Program Files (x86)\Steam\steamapps\common\Rollercoaster Tycoon 3 Gold
     (In Steam: right-click the game > Manage > Browse local files.)
  3. Start the game. "Peep Editor loaded - press F9 in a park" shows in the top-left corner for a few
     seconds.

UNINSTALL
  Delete d3d9.dll and PeepEditor.ini from the game folder (and PeepEditor.log if it is there).
  Nothing else is changed - the game's own files and your saves are never touched.

USE
  Load a park, then press F9 (change the key in PeepEditor.ini).
  - Left: every guest. Search by name, sort by clicking a column, click a guest to open them.
  - Overview: name (type a new one and press Rename), age group and sex numbers, what they are doing, where they are,
    time in park, rides done, what they bought, favourite ride, queue time, and switches such as
    "Wants to leave the park".
  - Money: cash in their pocket, bank money for ATMs, and everything they have spent.
  - Feelings & needs: happiness, energy, nausea, sunburn (current and target), food, drink, toilet
    (food and drink: 1 = full; toilet: 1 = desperate), wetness. The game moves current values toward
    the targets; tick "lock" to hold a value.
  - Preferences: the intensity range they like, nausea tolerance, pool preference.
  - Path: their current route, point by point, with the walking distance left and the point they head for.
  - Thoughts: their last six thoughts (the game's own thought names), and give them a new one.
  - Memory: every byte of the guest, with a box to write any value at any offset.
  - All guests: make everyone happy, fill everyone's needs, cure nausea, give everyone cash, set
    everyone's intensity range.
  While the mouse is over the menu the game ignores clicks; typing in a box doesn't reach the game.

NOTES
  - Saving the park saves your changes (they are the guests' real values).
  - Locked values are held only while the game runs.
  - Made for RCT3plus.exe (the Platinum / Complete Edition executable). Other versions are not supported.
  - Single-player game, no online parts: this mod only edits your own park in memory.
  - A log is written to PeepEditor.log in the game folder.

Built with Claude Code (AI) from the research in this project; uses Dear ImGui (licence in LICENSES.txt).
