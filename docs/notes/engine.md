# Engine

The rest of Frontier's engine: resources, files, streams, parsing.

| Address | Name | What it does |
|---|---|---|
| `0xc12770` | Parser::Parse | A bison-generated LALR parser (its tables and state at `0x13E32D0`-`0x13E3318`; messages `syntax error, unexpected %s, expecting %s or %s`), reading tokens from the flex-generated scanner `0xC11160` (`fatal flex scanner internal error`, `input buffer overflow, can't enlarge buffer`). Called from `0xC160B0` and `0xC16180` |
| `0x458a10` | Game::Initialise | Starts the game up: creates the player's folders under `My Documents\RCT3` (`\Coasters`, `\Campaigns`, `\Pools`, `\Scenarios`, `\Start New Scenarios`, `\Parks`, `\LaserEffects`, `\FireworkEffects`, `\WaterJetEffects`, `\LaserWriting`, `\Fireworks`), copies in the default `MusicGenre.txt` and the `Stratosphere` firework display, reads the install's uninstall registry key, loads `RCT3.dgf` and starts the engine's systems; shows a message box when something fails (`SEQUENCE_ERROR_DIALOG`). Called from `0x45AD60` |
| `0x410ed0` | Cheats::Init | Builds the table of the 32 cheats switched on by giving a guest a name (`CheatPeepName_` + `Rockefeller`, `Sawyer`, `Mouse`, `Fawkes`, `Atari`, `Moss`, `Lean`, `Hitchcock`, `Photostory`, `Ati`, `Sick`, `PeepCam`, `CoasterHeights`, `Unbreakable`, `RideAllRides`, `RideAllCoasters`, `BigExplosion`, `DavidBraben`, `LowFriction`, `NoFriction`, `FPS`, `GhostTown`, `TrainStaff`, `TunnelShark`, `AllowPathWalls`, `UndergroundSI`, `SoundWarp`, `DavidWalsh`, `AllGroupsIndividuals`, `AnimalsConstantlyBreed`, `FunnyDarts`, `AllowLargeMovies`). Called from `0x4137A0` |
| `0xc4a710` | DataExport::WriteDefines | Writes data out as C source text: `#define _<NAME>_SIZE` lines and values, numbers converted with `_itoa` (a developer tool for turning data into C tables). Called from `0xC4C570` |
| `0xc44720` | DataExport::WriteArrays | Writes data out as C source text: `extern const` declarations and `static const int name[] = { ... };` / `static const bool` arrays. Called from `0xC4C570` |
| `0xcbec90` | ShaderPreprocessor::Directive | The C-like preprocessor of the engine's shader language: handles `#include`, `#define` (with macro redefinition warnings), `#if` / `#elif` / `#else` / `#endif` and the engine's `bind` definitions (`[size]`), reporting errors such as `unknown preprocessor directive '%s'`, `cannot open include file: '%s'`, `invalid array size in bind definition`. Called from `0xCC0400` |
| `0x69ba20` | Language::FromName | Turns a language name into the game's language number: `ENGLISH`, `JAPANESE`, `KOREAN`, `SPANISH`, `FRENCH`, `FINNISH`, `DUTCH`, `ITALIAN`, `GERMAN`, `DANISH`, `NORWEGIAN`, `SWEDISH`, `chinese`, `schinese` (case-insensitive compares). Called from five places in the text loading code |
| `0xc348f0` | DebugConsole::Build | Builds the engine's debug message window: a message window and a stream window (`Message window compositor`, `Stream window compositor`) with scrollbars, and `Debug`, `Clear`, `Test` and output targets (`->File`, `->Game`, `->IDE`). Called from `0xC35C20` |
| `0xbeca70` | Name::Register | Makes a name object (0x1C bytes, table `0xFA5818`) from a String and records it in the table of up to 1,000 names at `0x13D1418` (count at `0x13D23B8`); returns it. Called by 784 start-up initializers (see [initializers.md](initializers.md)) |
| `0xbec4b0` | String::String | Makes a String from C text: copies it into a block of its own (or refers to it, when the second argument says so); `0xBEC5E0` frees it |
| `0xbec5e0` | String::~String | Frees a String's text when it owns it |
| `0xbcf850` | WideString::Create | Makes a reference-counted wide string from C text: count 1, length, capacity rounded up to 16, and the text widened to 16-bit characters |
| `0x46fad0` | Section::Section | Sets up a named section: takes a reference to its name, its kind, an empty list of child sections and a pointer to itself. Created by 107 start-up initializers (see [initializers.md](initializers.md)) |
| `0x46fc00` | Section::Enter | Enters a section: its parent becomes the current section (`0x12F0058`), it is linked into the parent's list of children (or into the list of top-level sections at `0x12F005C`, made on first use) and marked as entered. Called from 33 places, `ParkRenderer::Render` among them |
| `0x46f990` | Section::Leave | Leaves a section: the current section (`0x12F0058`) goes back to its parent |
| `0x6b1330` | Window::SetRelation | Places a window in the window tree relative to another one (8 kinds: as its first or last child, before or after it, ...): unlinks it from where it was (a broken link stops at a breakpoint, with the assert text `_relation`) and links it in, taking on the other window's flags. Every window builder uses it (see [windows.md](windows.md)) |
| `0x6b1640` | Window::SetRelationAndLayout | `Window::SetRelation`, after laying the window out (`0x6AFAF0`) |
| `0x6afaf0` | Window::UpdateRect | Works out a window's rectangle again and, when it changed, passes the change on (`0xBE7450`) and to the windows that depend on it |
| `0xbf9c80` | Error::Throw | Throws one of the engine's errors: jumps (`longjmp`) to the innermost handler; when there is none, shows `THROWN ERROR WAS NOT CAUGHT` in a `FATE-AL` message box and exits |
| `0xc11160` | Scanner::Lex | The flex-generated scanner that `Parser::Parse` reads tokens from: matches the input against its tables (a switch over the actions), with flex's error `start-condition stack underflow` |
| `0x47aea0` | Options::Load | Reads the player's settings from `My Documents\Atari\Options.txt` when the file exists (`0xBE45E0`). Called from `Game::Initialise` |
| `0x47b570` | Options::Save | Takes the current display settings (`0x12F4618`: resolution and the rest) into the options, each clamped between its option's minimum and maximum, and writes them to `\Atari\Options.txt`. Called from `FrontEnd::Start`, `0x607D00` and `0x6392E0` |
| `0x404ba0` | Movies::List | Lists the movie files in the game's `Movies` folder (`*.wmv`). Called from `StartupScreen::Draw` and two others |
| `0x9ebc70` | AttractSequence::Save | Writes a camera sequence for the attract mode to `AttractSequences\*.dat`: lines of `"%s",%I64d` and `,%f,%I64d`. Called from `ParkView::HandleCommands` (a key command) and `0x9EC540` |
| `0x453630` | Resources::LoadCommon | Loads the `*.common.ovl` resource files of a folder (such as `GUI`). Called from 39 places, `StartupScreen::Draw` among them |
| `0xc3d890` | OverlayFile::LoadStreamedItem | Loads a streamed item of a resource overlay file (`.ovl`): finds the loader for its type and feeds it the data, reporting `Attempting streamed unstructured data load before finalising previous streamed unstructured data in overlay '%s'`, `Failed to make a loader for item ... for stream ID ...`, `Resource loader for type '%s' does not support streamed data`. Called from `0xC3FF60` and `0xC41060` |
| `0xc38d50` | Messages::Format | Formats one of the engine's messages by its level (`Moan :`, `Warning :`, `Error :`, `Serious error :`) with its file and line (`%s(%d): %s%s`) and include chain (`included from here`) |
| `0x4689f0` | MovieRecorder::Start | Starts recording the screen to a movie: picks the next free `Movie%04d.wmv`, loads Windows Media (`WMVCORE`: `WMCreateProfileManager`, `WMCreateWriter`) and sets up its video input (`VideoIn1`). Called from `0x46A9F0` |
| `0x43caa0` | DebugInfo::Draw | Draws the developer's frame information: `FPS: %.1f (%.1fms)`, total people and people drawn, the people drawing mode (`Off`, `Simple`, `Complex`, `Dummy`, SSE, animation cycling), the current and target detail state and the minimum / maximum frame rate |
| `0x886eb0` | SIDSoundEditor::Save | Writes a scenery item's sounds to a file from the sound editor: each with `MinDistance`, `MaxDistance`, `SoundCategory` and its `AnimSounds` lines (`time=` ...), reporting `Couldn't write file`. Called from `0x887D70` |
| `0x5fd5c0` | Language::Switch | Changes the game's language: loads that language's GUI text (`GUI`, `Language_` + name, `*.common.ovl`, with `_Alternate`) and tells the player it takes effect later (`GameOptionsDialog_SettingWillTakeEffect`) |
| `0xcb9410` | Inflate | zlib's inflate (decompressing deflate data), compiled into the engine's own code (its messages `invalid block type`, `invalid stored block lengths`, `too many length or distance symbols`, `invalid bit length repeat`); the D3DX library has its own copy. Called from `0xCAFF20` |
| `0x53cd30` | MusicGenres::Save | Writes the player's music genre file `My Documents\Atari\MusicGenre.txt` with its header lines (`MusicGenre_File_Title`, `MusicGenre_File_Format`, between `#` rules). Called from `0x53ECB0` |
| `0x69dc10` | IMEManager::Check | Checks the Windows input method (IME) for typing Asian languages (`.\Code\Common\Gui\IMEManager.cpp`) and tells the player to switch it on or that the language is invalid (`IMEMessage_SwitchOnIME`, `IMEMessage_LangInvalid`). Called from `0x457C00` |
| `0xa3b860` | Files::FindCoasters | Searches folders for saved coasters (`\Coasters\`, `*.*`), skipping the Windows and recycle-bin folders (`WINNT`, `RECYCLED`, `RECYCLER`). Called from `0xA3C3D0` |
| `0x453f30` | GUI::LoadResources | Loads the GUI's resources for the screen size (`GUI\`, `GUI\640\`, `GUI\800\`, `GUI\1024\`, `GUI\1280\`) and the guests' name lists (`GuestName_Male`, `GuestName_Female`, `GuestNameSuffix`). Called from three places |
| `0x803d30` | ManagedObject::LoadFile | Loads saved objects from a file, reporting `FD_FailedToOpenFileToRead`, objects that can't be loaded (`ManagedObject_Unloadable`, `%NAME%`) and `FD_NoObjectsInFile`; a saved ride starts with `TrackedRideInstanceSaveHeader`. Called from 13 places |
| `0x461a50` | LiveContent::Find | Finds downloaded content in `\LiveContent\` (`.dat`): adverts by kind (`RCT3Rotating`, `RCT3Static`) and shape (`1x1`, `1x2`, `1x4`, `1x8`, `2x1`, `4x1`, `8x1`). Called from `0x463400` and `0x463690` |
| `0x618340` | Language::LoadProgress | Loads the GUI text for the language (`Chinese`, `SChinese` handled specially) showing the progress as ` (%d%%)` |
| `0xa45a20` | FrontScreen::LoadResources | Loads the front screen's resources (`Resources::LoadCommon`), with special cases for Chinese and simplified Chinese |
| `0x883140` | Style::Load | Reads a scenery style's definition: its index, internal name, map colour (`MapColourRed` / `Green` / `Blue`), displayed name (`StyleName_`) and fences (`FenceName`). Called from `0x554A30` and `0x97CE60` |
| `0x4043c0` | StartupScreen::LoadLogos | Loads the start-up logos: Atari, ATI, Frontier, RCT3 and the ESRB demo logo, with the Chinese and simplified Chinese versions and warning screen. Called from `0x43EB90` |
| `0x4293a0` | EnvironmentMaps::Load | Loads the environment maps used for reflections: shiny, chrome and water (`EnvMapShiny`, `EnvMapChrome`, `EnvMapWater`). Called from `0x429E80` |
| `0xc4c570` | ShaderSource::Write | Writes shader source text for the shader compiler: `#define` lines, the input and output structures (`VS_INPUT`, `VS_OUTPUT`, `PS_INPUT`, `PS_OUTPUT`), `#define DEFAULT_MAIN` and `#include <vsdefault>` / `<psdefault>`, with the data tables (`DataExport::WriteArrays`, `WriteDefines`). Called from `0xC4D580` and `0xC4D7A0` |
| `0x43f410` | Resources::LoadStartup | Loads the resources the game needs from the start: the system font, the GUI for each screen size (`GUI\640\Resolution` ... `GUI\1280\Resolution`), `main`, the ride icons and test resources (`Test\Shapes`, `Test\SceneryItemVisuals`, `Test\SceneryItems`) |
| `0x427950` | DebugInfo::DrawLighting | Writes the developer's lighting values: the night ambient colour, environment map colour, fresnel and reflective water colour (`NightAmbientR: %.3f` ...). Called from `DebugInfo::Draw` |
| `0x803560` | ManagedObject::LoadFileQuiet | Loads saved objects from a file like `ManagedObject::LoadFile`, reporting `FD_FailedToOpenFileToRead` and unloadable objects. Called from `0x7FAF90` |
| `0x804650` | ManagedObject::SaveFile | Saves objects to a file, keeping the old one as `.bak`, reporting `FD_FailedToOpenFileToWriteToIt` and `FD_FailedToWriteToIt`. Called from `0x7FAFA0` |
| `0x804d50` | ManagedObject::SaveFileQuick | Saves objects to a file, reporting `FD_FailedToOpenFileToWriteToIt` and `FD_FailedToWriteToIt`. Called from 13 places |
| `0x444100` | Language::LoadFonts | Loads the fonts for the language: special ones for Korean, Japanese, Chinese and simplified Chinese, and Danish and Norwegian. Called from `0x444780` and `0x444C30` |
| `0x412ef0` | Cheats::Names | Makes the names that switch cheats on when given to a guest: `David Braben`, `Andrew Thomas`, `Andrew Gillett`, `FPS`, `Ghost Town`, `Isambard Kingdom Brunel`, `Rick Griffiths`, `Alvin Swazonegger`, `Mornington Crescent`, `M Brookes`, `Alistair Lindsay`, `David Walsh`, ... (case-insensitive compares). Called from `0x7CD6E0` and `0x7CD9B0` |
| `0xa1c6b0` | Tutorial::LoadSteps | Loads a tutorial's text: its title and each step's title and text (`TutorialText_`, `_Title`, `_Step%d_`). Called from `0xA1DFE0` and `0xA1E000` |
| `0xc547e0` | FileSystem::ResolveDevice | Works out which device a path is on: a folder (`.dir/`), the CD (`cdrom0:`) or the host (`host1:`). Called from three places |
| `0x442570` | Language::LoadDayFonts | Loads the fonts for the days and dates of the Asian languages (`Day_Korean`, `Day_Chinese`, `Day_Japanese`, `Day_SChinese`). Called from `Language::LoadFonts` and `0x48D5B0` |
| `0x446b80` | Graphics::CheckDriver | Identifies the graphics card and driver (`VendorId`, `DeviceId`, `DriverId`, printed `%4x` / `%8x%8x`) and Windows version (`Win98`, `WinMe`, `Win2000`, `WinXP`) and looks up known fixes (`Fix`). Called from `0x4470B0` |
| `0xc1b6d0` | CommandLine::Parse | Reads the command line's options (`.\Code\Common\CLIOpts.cpp`), reporting `Too few arguments :` and `Unrecognised argument :`. Called from `0xC1BBB0` |
| `0x744090` | AvatarDatabase::Load | Reads the saved avatars: each one's version and name |
| `0x743500` | AvatarDatabaseEntry::Read | Reads an avatar's parts: member, type, sub-type, part, palette, swimsuit and ride intensity. Called from `0x745070` |
| `0x64a3c0` | Cheats::Names2 | Makes more of the guest names that switch cheats on: `Sound Aficionado`, `Challenge Man`, `No debug please`, `I want to be alone`, `Mono Man`, `Stereo Dude`, `Three Ears`, `When Seagulls Attack` (`To The Max`), `When Ducks Attack`, ... (case-insensitive compares) |
| `0xa1c290` | Tutorial::ExportText | Writes the tutorials' text out to the developer's text file (`c:\coding\rct3\resource\gui\english\Tutorials.txt`): `; Text for`, `TutorialText_`, `_Title:`, `_Step%d_`, `StepTitle:`, `Text:` |
| `0x802410` | ManagedObject::TypeNames | Makes the names of the kinds of saved object, for loading them from a file (30 names: `QueueLine`, `PathTile`, `PathGround`, `PathFlying`, `PathQueue`, `PathGroundVisual`, `PathTypeDatabaseEntry`, `QueueTypeDatabaseEntry`, `QueueTypeGroundSurface`, `GuestGroup`, `Guest`, `VIP`, ...). Called from `ManagedObject::LoadFile` |
| `0x47a3a0` | KeyBinding::Name | Writes a key binding's name: `ctrl`, `shift`, `alt` and the key (`Key_`, from the `:txt` text) |
| `0xcaff20` | Inflate::Stream | Decompresses a zlib stream: checks its header (`unknown compression method`, `invalid window size`, `incorrect header check`, `need dictionary`), inflates it (`Inflate`) and checks the result (`incorrect data check`) |
| `0x496130` | Resources::LoadTextures | Loads a resource folder's textures, with stand-ins for missing ones (`nullbmp`, `Demo`). Called from `0x496610` |
| `0x43eb90` | Resources::LoadEnvironment | Loads the environment and shadow maps (`EnvMapShiny`, `EnvMapChrome`, `ShadowMapHorizon`) and shows the cache-warming message (`CacheWarmingMessage`). Called from `Resources::LoadStartup` |
| `0xc045a0` | Graphics::Initialise | Starts the graphics: checks for DirectX 9 (`DirectX9 not found.`), creates the Direct3D device (`Failed to create Direct3D Device.`, `Your graphics hardware or drivers do not meet the minimum requirements ...`) and builds the vertex shader fragments (`VertexShaderFragments::Build`). Called from `Game::Initialise` |
| `0x457880` | CrashDump::Write | Writes `CrashDump.txt` when the game crashes: the exception (`Unhandled exception "%s" in "%s" at %04x:%08x`), the version and the address read or written. Called from `0x45AD60` |
| `0x452ed0` | Resources::LanguageFolder | Picks the resource folders for Chinese, Korean, simplified Chinese, Japanese and Thai. Called from `Resources::LoadCommon` |
| `0x800460` | ManagedObject::ContentError | Reports content that can't be loaded (`ManagedObject_ContentError`, `ManagedObject_Unloadable`, `%NAME%`). Called from seven places |
| `0xcaf090` | OverlayData::FindFiles | Finds a resource's overlay files (common and unique), reporting `no overlay file for %s`. Called from `0xC36880` and `0xCAF6E0` |
| `0x464690` | GameZone::Setup | Sets up the online game zone (`roller_coaster_tycoon_pc_na`, `RCT3_GameZone`, `Sample Game`, `Sample Zone`). Called from `0x456F60` |
| `0x49cfb0` | Units::SpeedName | Writes a speed unit: `m/s`, `mph`, `km/h` (`km/t` in Danish and Norwegian), or `???`. Called from `0x49D7D0` |
| `0x413f00` | DebugInfo::GameState | Writes the developer's game state text: the cloth manager, front screen or in game, waiting for a stable frame rate (`State: Off`, `WaitingForFrameRate`, `On`, ...) |
| `0x69b230` | IMEManager::KnownIMEs | Recognises the Chinese input methods by file name (`TINTLGNT.IME`, `CINTLGNT.IME`, `MSTCIPHA.IME`, `PINTLGNT.IME`, `MSSCIPYA.IME`). Called from `0x69D5A0` |
| `0x69cac0` | IMEManager::LoadIMM | Loads the input method functions from `IMM32.DLL` (`ImmLockIMC`, `ImmUnlockIMC`, `GetReadingString`, `ShowReadingWindow`, ...). Called from `FrontEnd::Start` and `0x457C00` |
| `0x405510` | StartupScreen::Start | Starts the start-up screens (`RCT3Startup`, with simplified Chinese) |
| `0xc35a20` | Messages::Redirect | Sends the engine's messages to the developer's IDE or to a file (`Redirecting to IDE`, `Redirecting to file`) |
| `0xc4f590` | Shaders::LoadDefaults | Loads the default vertex and pixel shader sources (`vsdefault`, `psdefault`). Called from `GE_Device::SetupDepthOfField` |
| `0xaac130` | PathFinder::DebugText | Writes a path search's developer statistics (`Cost: %.1f:`, `Nodes Visited: %d`) |
| `0x45df00` | GameZone::LoadM4D | Loads the in-game advertising library (`m4d.dll`: `MMT_Initialize`, `MMT_DeInitialize`, `MMT_NetUpdate`, `MMT_FlushImpressionCache`, `MMT_GetZoneHeaders`, ...). Called from `GameZone::Setup` |
| `0x4470b0` | Graphics::LoadFixes | Reads the graphics workarounds for known cards and drivers (`Graphics.fix`: `BeefynessForShaderModel`, `AmbientLightForViewport3D`, `Radeon7x00BillboardMadness`, `NVidiaSpecularMasking`, `NoReflections`, `NoGlare`, `NoExplicitDownsampling`, ...). Called from `Game::Initialise` |
| `0x453430` | Language::FromIndex | Gives a language's name by its number: `English`, `French`, `German`, `Dutch`, `Italian`, `Spanish`, `Danish`, `Norwegian`, `Swedish`, `Finnish`, `Chinese`, `Korean`, ... Called from four places |
| `0x617840` | Language::IsChinese | Checks for Chinese or simplified Chinese |
| `0x61c9b0` | MessageConsoleEntry::Fields | Registers a message console entry's saved fields (a `resourcesymbol` `Text` and a `float32` `Time`). Called from `0x6203A0` |
| `0x7fd9d0` | AIDDatabaseEntry::Fields | Registers an attraction definition's saved fields (`OVERLAYFILENAME`, `SYMBOLNAME`, `ATTRTYPE`). Called from `0x7FFDF0` |
| `0x552950` | PersonInfo::Fields | Registers the saved fields shared by the ride person infos (`StationIndex`, `SeatState`). Called by the MiniGolf, SplinePath, TrackedRide and WalkAlongTrack person infos |
| `0x699ea0` | GUIRenderer::Shaders | Gets the GUI renderer's shaders (`GUIRendererColour`, `GUIRendererBitmap`, `GUIRendererZMask`). Called from 37 places |
| `0x6b0230` | SkinItem::Texture | Gets a GUI skin item's texture, reporting `No texture for skin item` / `Missing skin texture`. Called from 73 places |
| `0x456fd0` | CrashDump::ExceptionName | Turns an exception code into its name (`Access Violation`, `Datatype Misalignment`, `Breakpoint`, `In Page Error`, `Illegal Instruction`, `No Memory`, ...). Called from `CrashDump::Write` |
| `0xcbb7e0` | Inflate::BuildTrees | zlib's building of the inflate code trees, with its errors (`oversubscribed distance tree`, `incomplete distance tree`, `empty distance tree with lengths`, ...). Called from `Inflate` |
| `0xcd65a0` | DebugInfo::ResourceStats | Writes the developer's resource statistics, one line per resource type (`RS: %20s   PL: %10d   BT: %5d   DB: %10d   TC: %5d   MIN: %8d   MAX: %8d`) and the total. Called from `DebugInfo::Draw` |
| `0x707530` | Scenario::DefaultText | Gives a new scenario its default name and description (`Scenario_DefaultName`, `_DefaultDescription`). Called from `Park::Park` and `NewGame::Start` |
| `0x4c12a0` | Attraction::DebugFlags | Writes an attraction's flags for the developer (`NEVO`, `INSP`, `CRSH`, `BDP`, `MFD`, joined as `%s:%s:%s:...`). Called from `0x4B9D60` |
| `0x7ff060` | RideTrainInstance::FileFields | Registers the fields a saved ride train is found by in a file (`RideTrainSymbolName`, `RideTrainOverlayName`). Called from `0x7FFDF0` |
| `0x7fd5b0` | SIDDatabaseEntry::FileFields | Registers the fields a saved scenery definition is found by (`OVERLAYFILENAME`, `SYMBOLNAME`). Called from `0x7FFDF0` |
| `0x7ffd20` | PPGDatabaseEntry::FileFields | Registers the fields a saved people part group definition is found by (`SymbolName`). Called from `0x7FFDF0` |
| `0x7fe9a0` | CarriedItemInstance::FileFields | Registers a saved carried item's link to its definition (`CIDEntry`). Called from `0x7FFDF0` |
| `0x7fe7d0` | CIDDatabaseEntry::FileFields | Registers the fields a saved carried item definition is found by (`SymbolName`). Called from `0x7FFDF0` |
| `0x7fe640` | AttractionInstance::FileFields | Registers a saved attraction's link to its definition (`AIDDatabaseEntry`). Called from `0x7FFDF0` |
| `0x7fd800` | SceneryItem::FileFields | Registers a saved scenery item's link to its definition (`DATABASEENTRY`). Called from `0x7FFDF0` |
| `0x47c840` | DebugInfo::Sounds | Writes the developer's sound statistics (`NumSounds: %d`, `NumRideTrainSounds: %d`, per category). Called from `DebugInfo::Draw` |
| `0xdfeae0` | Control::LoadTypeLibrary | Loads a COM type library (`control.tlb`, `LoadRegTypeLib` / `LoadTypeLib`). Called from `0xE00BA0` |
| `0xcc0400` | ShaderPreprocessor::ReadSource | Reads a shader source file for the preprocessor (`source`, `unexpected end of file`). Called from `0xCC04E0` |
| `0xcbb740` | Inflate::BuildBitLengthTree | zlib's building of the bit-length code tree (`oversubscribed dynamic bit lengths tree`, `incomplete dynamic bit lengths tree`). Called from `Inflate` |
| `0x9e88b0` | DebugInfo::Weather | The developer's manual weather change: sunny, sunny and cloudy, cloudy, light rain, heavy rain, thunderstorm, snowstorm. Called from `DebugInfo::Draw` |
| `0xc65e90` | Framework::RemoveWidget | Removes a widget from the debug framework's list (`Widget %p removed from CL`). Called from two places |
| `0xc60d70` | Framework::AddWidget | Adds a widget to the debug framework's list (`Widget %p added to CL`). Called from two places |
| `0x440010` | GameState::Name | Writes the game state's name: banner sequence, front screen, attract sequence, career map, tutorial, in game, scenario start, ... (`State*`). Called from `DebugInfo::Draw` |
| `0x4533c0` | Language::IsAsian | Checks for Chinese, Korean, Japanese, simplified Chinese or Thai. Called from 11 places |
| `0x457fa0` | Resources::LoadMain | Loads the main resource files (`Main.common.ovl`, `Main.unique.ovl`, `Style\Vanilla\Style.common.ovl`, `Style.unique.ovl`). Called from `Game::Initialise` |
| `0xca53a0` | Platform::Name | Names the target platform: `windows`, `PS2`, `Xbox`, `GameCube`, `PSP`, `unknown_target` |
| `0xc19685` | MovieDecoder::Format | Checks a movie's video format (`vids`, `YV12`) |
| `0xf03880` | TexCoords::Names | Makes the texture coordinate names at start-up (`TexCoord0` ... `TexCoord3`) |
| `0xc49300` | ShaderSource::WriteStruct | Writes a shader's input or output structure as source text: `struct name {`, each member with its `: register(v...)` or semantic, `};`. Called from `ShaderSource::Write` |
| `0x4a49b0` | Resources::FindTrackStyles | Searches the styles' track folders (`*\Tracks\*\track*`, `Style`) for their `.common.ovl` / `.unique.ovl` files, calling itself for sub-folders. Called from `0x4A5A40` |
| `0xc3f330` | OverlayFile::LoadHeader | Reads a resource overlay file's header and its types' loaders, reporting `file "%s" not opened`, `No handler for type '%s' (version %s) in overlay '%s'`, `Bad version %s in overlay '%s'`, `Bad header (missing file?) '%s'`. Called from `0xC41060` |
| `0xc40730` | OverlayFile::LoadStructuredData | Loads an overlay's structured data, reporting `Failed to allocate memory for structured resource in overlay '%s'`. Called from `0xC41060` |
| `0xc3c850` | OverlayFile::Relocate | Applies an overlay's relocation table, reporting entries that don't map within it (`Relocation table entry %s (%s) doesn't map within the overlay '%s'`). Called from three places |
| `0x626b90` | Screenshot::Save | Saves a screenshot as the next free `Shot%04d.bmp` (counting the existing `Shot*.bmp`). Called from `DebugInfo::Draw` and two others |
| `0x7cd9b0` | Person::PickBodyParts | Picks a person's body parts at random from the suitable ones, reporting `Can't find any suitable body parts for %s %s` and `Missing People Resources`. Called from `0x762A10` and `0x7EB590` |
| `0x53e280` | TrackImporter::Load | Loads a track file for importing, with the error box (`TrackImporter_Error_Title`). Called from two places |
| `0xae6e40` | LaserWriting::List | Lists the laser writing files (`\LaserWriting\*.lwr`). Called from `0xAE7920` |
| `0xa51f80` | Campaign::ListScenarios | Lists the campaign's scenarios (`Campaigns\Base\Scenario%02d.dat`, with the expansions' `AOP_FSName_`). Called from `0xA41870` |
| `0xa2ba10` | CarriedItems::Load | Loads the carried item definitions (`CarriedItems`, `*:cid`, `*.common.ovl`). Called from `0xA2D290` |
| `0x66ddd0` | TrackFile::ReadHeader | Reads a saved track's header (`TrackedRideInstanceSaveHeader`, `:trr`). Called from two places |
| `0x995810` | FireworkDisplay::Load | Loads a firework display, refusing one that is too big (`FireworksTooBig`). Called from `0x9A57D0` |
| `0x46aa90` | MovieDecoder::Open | Opens a movie for playing (`.\Code\Windows\MovieDecoder.cpp`), finding its video stream (`vids`). Called from `AdMenu::DrawPreview` and two others |
| `0xc4e200` | ShaderCache::Load | Loads the compiled shader cache (`STCache.bin`). Called from `0xC4F0C0` |
| `0xc48640` | ShaderCache::Save | Writes the compiled shader cache (`STCache.bin`). Called from three places |
| `0xc4f280` | ShaderCache::LoadSource | Loads the shader source cache (`SCCache.bin`). Called from `Shaders::LoadDefaults` |
| `0x476b40` | MovieDecoder::OpenAudio | Opens a movie's sound through a DirectShow sample grabber (`Sample Grabber`, `auds`). Called from `0x476FB0` |
| `0x8821a0` | Resources::FindStyles | Searches for the style folders' resource files (`Style`, `*.common.ovl`), calling itself for sub-folders. Called from `0x882F20` |
| `0x45bb70` | FileMenu::DefaultName | Makes a default file name (`default.` + extension). Called from `FlyingCameraRoute::FileMenu` and `TutorialEditor::FileMenu` |
| `0xa41870` | NewGame::Load | Loads a saved game (`MyGame.dat` by default). Called from `0xA431B0` and `0xA44B00` |
| `0xc3d1d0` | OverlayFile::CheckTypeIndex | Reports a bad resource type index in an overlay (`Bad Resource Type index ... for item ... in overlay '%s'`). Called from two places |
| `0x803040` | ManagedObject::ReportUnloadable | Reports an object that can't be loaded (`ManagedObject_Unloadable`, `%NAME%`). Called from five places |
| `0x4a5a40` | Resources::FindStyleFolders | Finds the styles' track, enclosure and fence folders (`Style`, `Tracks\Coasters`, `Tracks\TrackedRides`, `Enclosures\Fences`). Called from `0x4A7190` |
| `0xa52740` | Campaign::FolderName | Writes a campaign's folder name (`\Campaigns\`, the expansions' `AOP_FSName_`). Called from six places |
| `0x9d2d40` | Particles::Load | Loads the particle skins and sounds (`Particles\Particles`, `*:psi`, `*:snd`, `BurnTrail`). Called from `0x477680` |
| `0x438f70` | FrontEnd::LoadStartup | Loads the front end's start-up park (`startup.dat`). Called from `FrontEnd::Start` |
| `0xb38740` | WildAnimal::Log | Writes a line to the developer's animal log (`AnimalLog.txt`, `%4.2f: %12s: %s`) |
| `0xa565d0` | DatDesc::Load | Loads a file's description for the file lists (`GUI2_DatDesc`). Called from two places |
| `0xbce010` | Memory::FreeAligned | Frees a block from the aligned allocator (through the C runtime's `free`). Called from 17,743 places; `OwnedPointer::Reset` and the array rewrites use it |
| `0xbcdfd0` | Memory::AllocAligned | Allocates a block 16-byte aligned (`_aligned_malloc` of the size rounded up to 16), returning it placed so it ends at the end of the rounded block. Called from 1,813 places |
| `0xcc4d60` | Colour::Colour | Makes a colour from red, green and blue: stored as blue, green, red, 0. Called from 3,008 places (most of them start-up colour tables) |
| `0x8087e0` | FieldRegistry::Add | Adds a saved field's description to the registry, which keeps them sorted by name (`0x8085D0`). Called from 2,734 places (every class's field registration) |
| `0x8085d0` | SortedMap::Insert | Inserts a key and value into a sorted pair of arrays (growing them by their step when full), shifting the later entries up. Called from `FieldRegistry::Add` |
| `0xbd0490` | WideString::Empty | Returns the shared empty wide string, made on first use (and freed at exit). Called from 2,507 places |
| `0xbd1880` | TextRef::~TextRef | Destroys a reference to a text (table `0xFA14D0`): lets go of what it refers to and releases its wide string, freeing it with the last reference. Called from 2,418 places |
| `0xbd14e0` | TextRef::Assign | Makes a text reference refer to what another one does, releasing its old wide string. Called from 625 places |
| `0x40f730` | ObjectRef::~ObjectRef | Destroys a reference to an object (the reference class `0xF26AE0`, see `ObjectRef::Construct`): leaves its handle's list of references (a broken list stops at a breakpoint). Called from 2,022 places |
| `0x800860` | ManagedObject::Activate | Makes sure a managed object is set up: the first time (flags at +0x2C), runs its virtual slot 2 (set up) and then slot 3 (after set up). Called from 1,464 places |
| `0xbcdf40` | Error::ThrowIf | Throws an engine error (`Error::Throw`) when given an error number other than -1. Called from 1,361 places (start-up registrations among them) |
| `0xbcf7d0` | WideString::FromWide | Makes a reference-counted wide string from a wide C string (count 1, length, capacity rounded up to 16). Called from 1,348 places |
| `0x403be0` | WideString::AssignC | Sets a wide string from C text (its length, then `0xBD0720`). Called from 1,170 places |
| `0xbd0720` | WideString::Append | Appends characters to a wide string, copying it first when it is shared (a new block of the combined length rounded up to 16). Called from `WideString::AssignC` and others |
| `0x403bb0` | WideString::AssignWide | Sets a wide string from another one's characters (`0xBD0640`). Called from 1,034 places |
| `0xbcffe0` | WideString::MakeUnique | Makes a wide string's text its own before changing it: when shared, copies it into a new string and lets go of the old one. Called from 865 places |
| `0x805750` | Handle::Null | Returns the handle that refers to nothing, with one more reference (made on first use: 0x28 bytes). Called from 730 places; `ObjectRef::Construct` and the other reference rewrites use it |
| `0x41c740` | WideString::Copy | Copies a wide string (or the empty one). Called from 618 places |
| `0xbeca40` | Name::Assign | Sets a name entry from another (copying its number at +0xC) with a value. Called from 611 places |
| `0xbecc10` | NameTable::Set | Stores a value under a name's number in a table, growing the table (by 4) to fit. Called from 611 places |
| `0x40a090` | Terrain::Get | Returns the park's terrain (`0x1311650`), creating it (`RCT3Terrain`, 0x98 bytes, 90 x 90) and activating it the first time. Called from 526 places |
| `0xbd99e0` | ResourceRef::~ResourceRef | Destroys a reference to a resource (tables `0xFA5388` / `0xFA5380`). Called from 714 places |
| `0x401c30` | Text::FromNumber | Writes a number as text in a given base, with the digits `0123456789ABCDEF`. Called from `0xA83E10` and `LaserWriting::DefaultName` |
| `0xbdb620` | File::SaveSafely | Writes a file through a temporary one (`.tmp`), then puts it in place. Called from `SIDSoundEditor::Save` |
| `0xbdae70` | Resources::OpenOverlayPair | Opens a resource's pair of overlay files (`.common`, `.unique`). Called from `Resources::FindTrackStyles` and three others |
| `0xc3e840` | OverlayFile::Open | Opens a resource overlay file, reporting `Resource_OverlayFile: file "%s" is already open`. Called from seven places |
| `0x8bcc90` | Grid::CellAt | Turns a position into a grid cell (column and row, rounding down), using the grid's origin (+0xC, +0x10) and cell size (+0x14, +0x18), and says whether it's inside. Called from many places with the terrain |
| `0x59acc0` | TrackedRide::TrackInterface | Gets (and caches at +0x64) an interface of the object the reference at +0x54 points to, through its class's interface table (id `0x12F5544`). `0x5D4870` is the same with id `0x12F54E0` |
| `0x5d4870` | TrackedRide::Interface2 | The same as `0x59ACC0` with the interface id `0x12F54E0` |
