# Engine

The rest of Frontier's engine: resources, files, streams, parsing.

| Address | Name | What it does |
|---|---|---|
| `0xc12770` | Parser::Parse | A bison-generated LALR parser (its tables and state at `0x13E32D0`-`0x13E3318`; messages `syntax error, unexpected %s, expecting %s or %s`), reading tokens from the flex-generated scanner `0xC11160` (`fatal flex scanner internal error`, `input buffer overflow, can't enlarge buffer`). Called from `0xC160B0` and `0xC16180` |
| `0x458a10` | Game::Initialise | Starts the game up: creates the player's folders under `My Documents\RCT3` (`\Coasters`, `\Campaigns`, `\Pools`, `\Scenarios`, `\Start New Scenarios`, `\Parks`, `\LaserEffects`, `\FireworkEffects`, `\WaterJetEffects`, `\LaserWriting`, `\Fireworks`), copies in the default `MusicGenre.txt` and the `Stratosphere` firework display, reads the install's uninstall registry key, loads `RCT3.dgf` and starts the engine's systems; shows a message box when something fails (`SEQUENCE_ERROR_DIALOG`). Called from `0x45AD60` |
| `0x410ed0` | Cheats::Init | Builds the table of the 32 cheats switched on by giving a guest a name (`CheatPeepName_` + `Rockefeller`, `Sawyer`, `Mouse`, `Fawkes`, `Atari`, `Moss`, `Lean`, `Hitchcock`, `Photostory`, `Ati`, `Sick`, `PeepCam`, `CoasterHeights`, `Unbreakable`, `RideAllRides`, `RideAllCoasters`, `BigExplosion`, `DavidBraben`, `LowFriction`, `NoFriction`, `FPS`, `GhostTown`, `TrainStaff`, `TunnelShark`, `AllowPathWalls`, `UndergroundSI`, `SoundWarp`, `DavidWalsh`, `AllGroupsIndividuals`, `AnimalsConstantlyBreed`, `FunnyDarts`, `AllowLargeMovies`). Called from `0x4137A0` |
