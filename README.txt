PEEP EDITOR for RollerCoaster Tycoon 3 (Platinum / Complete, Steam or disc with Soaked! and Wild!)
====================================================================================================

An in-game menu to view and change any guest ("peep") in your park: their money, feelings, needs,
ride preferences, where they are walking, what they are thinking, and every other value they carry.

INSTALL
  With RCT3 Workshop: Mod Loader > Install content... > pick PeepEditor.zip > Install. It goes in as a
  "Code mod" beside RCT3plus.exe; Disable / Uninstall there take it out again.

  By hand:
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
  - The guest you select is always shown in the park while the menu is open: a white pin with their
    name, and the route they are walking.
  - Track on screen (Overview or Path tab): shows the guest in the park itself - a coloured pin with
    their name and what they are doing, the route they plan to walk (dots, ending in a flag), and a
    fading trail of where they have walked. Up to 8 guests at once, each in its own colour (their name
    in the list turns that colour). Tracking stays on screen when the menu is closed; tick route /
    trail / labels to choose what is drawn, or "Stop tracking all" in All guests.
  - Go to: send the guest somewhere - pick any ride, shop or stall in the park and press "Go here".
    The game plans the walk (shown on screen with Track), then they queue and ride, or walk up and buy.
    Guests walk in groups, so the whole group goes. Notes:
      * only while the group is walking about - not while queueing, riding, shopping or sitting;
      * the attraction must be open (new shops start closed: open them in their own window first);
      * guests only buy what they need, so "make them hungry & thirsty" (on by default) gives them a
        reason to buy when sent to a shop;
      * the order is kept until they get there: if the group's own mind changes on the way, it is sent
        again (shown as "kept on course"). "Cancel order" stops that.
    "Leave the park" sends them home.
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
  - If the game crashes on start with the mod, set HookGameInput=0 in PeepEditor.ini and send
    PeepEditor.log and CrashDump.txt from the game folder.

Built with Claude Code (AI) from the research in this project; uses Dear ImGui (licence in LICENSES.txt).
