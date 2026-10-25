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
| `0x699ea0` | GUIRenderer::GUIRenderer | Constructs the GUI renderer and makes it the current one (`0x12F84C8`): its shaders (`GUIRendererColour`, `GUIRendererBitmap`, `GUIRendererZMask`) and empty draw lists (`0x10EEDB4`, `0x10EEDCC`, `0x10EEDCE` set to 0xFFFF). Called from 37 places |
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
| `0xbce000` | Memory::Free | Frees a block from the game's allocator (`0xBCDFC0`): jumps to the C runtime's `free`. Called from thousands of places, most of all destructors |
| `0x805700` | Handle::For | Returns the handle of a given object, with one more reference: looks it up by the object's id (+0x20, +0x24) through `0x805650` (or the null handle for no object). Used by the reference constructors |
| `0xc41460` | OverlayFile::ReadDebugData | Reads a resource overlay's debug data: its version (1 to 5, else `Unable to locate debug data - file version is not 1, 2, 3, 4 or 5`) and byte order (a flag byte; values are byte-swapped when it is set), then steps over its sections by their sizes. Called from `0xC42C30` and `0xC430F0` |
| `0xcbb970` | Inflate::Flush | zlib's inflate_flush: copies as much output as fits from the sliding window to the output buffer (the part up to the window's end, then wrapping to its start), advancing the counts and updating the check value through the check function when there is one; turns a buffer error into success when anything was copied. Called from Inflate and Inflate::Codes |
| `0x860a20` | Options::ApplyKeys | For each of the 47 key commands, looks up the option chosen for it (`0x10F0940`) in the options table at `0x10ED860` (20 bytes each) and copies that entry's values at `0x10ED860` and `0x10ED874` into the command's tables at `0x10F07C0` and `0x10F087C`. Called from Options::Load |
| `0xc3b2c0` | OverlayFile::FindStreamedItem | Finds a streamed item by id in the overlay's sorted table (+0x4C, count +0x48, 20 bytes each) by binary search, then among the entries with that id the one whose 16-bit value matches the one given; null when there is none. Called from OverlayFile::LoadStreamedItem |
| `0x443ee0` | UpdateListMember::UpdateListMember | Constructor of a member of one of the four global update lists (table `0xF28E74`): joins list 0-3 (heads `0x10EC768`, `0x10EC780`, `0x10EC798`, `0x10EC7B0`) at its end, or at its front when the flag is set (GameTime::Update runs list 1 every 90 seconds of game time, list 2 every 360 and list 3 every 2,880). Used by the constructors of Park, Money, WildAnimal and others |
| `0xcbbbc0` | ShaderPreprocessorLexer::PreviousState | The lexer's yy_get_previous_state (a flex scanner, 251 states): runs the state machine over the text matched so far to find the state to back up to, remembering the last accepting state. Called from ShaderPreprocessor::Directive |
| `0xc095a0` | ScannerLexer::PreviousState | The same flex yy_get_previous_state for the script scanner (1,527 states). Called from Scanner::Lex |
| `0xca1eb0` | Parser::Allocate | Takes a block of the given size (at least 4, rounded up to the given alignment) from the parser's free list, first fit, splitting off what is left over when it is big enough; null when nothing fits. Called from Parser::Parse |
| `0x807970` | SortedMap::FindText | Binary search for a text key in a sorted array of C texts: gives the position where it is (or would go) and whether it was found. Called from SortedMap::Insert |
| `0xcbbca0` | ShaderPreprocessorLexer::TryNulTransition | The lexer's yy_try_NUL_trans (flex): the state reached from the given one on a NUL character, or 0 when that ends in the jam state. Called from ShaderPreprocessor::Directive |
| `0xc09670` | ScannerLexer::TryNulTransition | The same flex yy_try_NUL_trans for the script scanner. Called from Scanner::Lex |
| `0xca1f80` | Parser::Free | Gives a block back to the parser's free list, kept in address order, merging it with the free blocks just before and after it; counts it off the live blocks and its size off the total. Called from several destructors |
| `0x801450` | SortedArray::Find | Binary search for a number in a sorted array of 32-bit values: gives the position where it is (or would go) and whether it was found. Used by ManagedObject::SaveFile and SaveFileQuick |
| `0x435730` | List::RemoveValue | Finds the first node of a linked list holding the given value (+8) and unlinks it (checking its neighbours point back at it, and stopping the program with a breakpoint if not), clearing its links; returns the node, or null when the value is not there |
| `0x475f10` | List::RemoveValue | Finds the first node of a linked list holding the given value (+8) and unlinks it (checking its neighbours point back at it, and stopping the program with a breakpoint if not), clearing its links; returns the node, or null when the value is not there |
| `0x49ec50` | List::RemoveValue | Finds the first node of a linked list holding the given value (+8) and unlinks it (checking its neighbours point back at it, and stopping the program with a breakpoint if not), clearing its links; returns the node, or null when the value is not there |
| `0xacd780` | List::RemoveValue | Finds the first node of a linked list holding the given value (+8) and unlinks it (checking its neighbours point back at it, and stopping the program with a breakpoint if not), clearing its links; returns the node, or null when the value is not there |
| `0xbd7580` | Array::Reserve | Makes sure an array has room for at least n items: when its capacity (+4) is smaller, grows it from the capacity (at least 1) by its growth step (+8; doubling when the step is 0) until it is enough, then resizes it to that with `0xbd6750` |
| `0xbd75e0` | Array::Reserve | Makes sure an array has room for at least n items: when its capacity (+4) is smaller, grows it from the capacity (at least 1) by its growth step (+8; doubling when the step is 0) until it is enough, then resizes it to that with `0xbd67f0` |
| `0xbdf5f0` | Array::Reserve | Makes sure an array has room for at least n items: when its capacity (+4) is smaller, grows it from the capacity (at least 1) by its growth step (+8; doubling when the step is 0) until it is enough, then resizes it to that with `0xbdd740` |
| `0xc1af80` | Array::Reserve | Makes sure an array has room for at least n items: when its capacity (+4) is smaller, grows it from the capacity (at least 1) by its growth step (+8; doubling when the step is 0) until it is enough, then resizes it to that with `0xc1ad10` |
| `0xc48bd0` | Array::Reserve | Makes sure an array has room for at least n items: when its capacity (+4) is smaller, grows it from the capacity (at least 1) by its growth step (+8; doubling when the step is 0) until it is enough, then resizes it to that with `0xc47d30` |
| `0xc48c30` | Array::Reserve | Makes sure an array has room for at least n items: when its capacity (+4) is smaller, grows it from the capacity (at least 1) by its growth step (+8; doubling when the step is 0) until it is enough, then resizes it to that with `0xc47dd0` |
| `0xc48c90` | Array::Reserve | Makes sure an array has room for at least n items: when its capacity (+4) is smaller, grows it from the capacity (at least 1) by its growth step (+8; doubling when the step is 0) until it is enough, then resizes it to that with `0xc47e70` |
| `0xc48cf0` | Array::Reserve | Makes sure an array has room for at least n items: when its capacity (+4) is smaller, grows it from the capacity (at least 1) by its growth step (+8; doubling when the step is 0) until it is enough, then resizes it to that with `0xc47f10` |
| `0xc48d50` | Array::Reserve | Makes sure an array has room for at least n items: when its capacity (+4) is smaller, grows it from the capacity (at least 1) by its growth step (+8; doubling when the step is 0) until it is enough, then resizes it to that with `0xc47fb0` |
| `0xc48db0` | Array::Reserve | Makes sure an array has room for at least n items: when its capacity (+4) is smaller, grows it from the capacity (at least 1) by its growth step (+8; doubling when the step is 0) until it is enough, then resizes it to that with `0xc48050` |
| `0xc9bf70` | Array::Reserve | Makes sure an array has room for at least n items: when its capacity (+4) is smaller, grows it from the capacity (at least 1) by its growth step (+8; doubling when the step is 0) until it is enough, then resizes it to that with `0xc9bcc0` |
| `0xcb28e0` | Array::Reserve | Makes sure an array has room for at least n items: when its capacity (+4) is smaller, grows it from the capacity (at least 1) by its growth step (+8; doubling when the step is 0) until it is enough, then resizes it to that with `0xcb23d0` |
| `0xcb2940` | Array::Reserve | Makes sure an array has room for at least n items: when its capacity (+4) is smaller, grows it from the capacity (at least 1) by its growth step (+8; doubling when the step is 0) until it is enough, then resizes it to that with `0xcb2470` |
| `0xcb29a0` | Array::Reserve | Makes sure an array has room for at least n items: when its capacity (+4) is smaller, grows it from the capacity (at least 1) by its growth step (+8; doubling when the step is 0) until it is enough, then resizes it to that with `0xcb2510` |
| `0xcbce60` | Array::Reserve | Makes sure an array has room for at least n items: when its capacity (+4) is smaller, grows it from the capacity (at least 1) by its growth step (+8; doubling when the step is 0) until it is enough, then resizes it to that with `0xcbc5a0` |
| `0xcbcf50` | Array::Reserve | Makes sure an array has room for at least n items: when its capacity (+4) is smaller, grows it from the capacity (at least 1) by its growth step (+8; doubling when the step is 0) until it is enough, then resizes it to that with `0xcbc640` |
| `0xcbe170` | Array::Reserve | Makes sure an array has room for at least n items: when its capacity (+4) is smaller, grows it from the capacity (at least 1) by its growth step (+8; doubling when the step is 0) until it is enough, then resizes it to that with `0xcbd900` |
| `0xcbe6a0` | Array::Reserve | Makes sure an array has room for at least n items: when its capacity (+4) is smaller, grows it from the capacity (at least 1) by its growth step (+8; doubling when the step is 0) until it is enough, then resizes it to that with `0xcbe1d0` |
| `0xbcf040` | String::Compare | Compares two texts by their characters (signed bytes) up to the shorter length: the difference at the first character that differs, otherwise the difference in length. Used by Billboard::Create, Resources::FindTrackStyles, FileSystem::ResolveDevice and others |
| `0xbce970` | WideString::Compare | The same comparison for wide texts (16-bit characters). Used by RideSelectionWindow::LoadResources, TrackImporter::ReadTD, Pool::PartFromName and others |
| `0xbcf540` | String::FindLast | Searches a text backwards from the given position (or from its last character when it is -1) for a character: its position, or -1. Called from Game::Initialise |
| `0x80ee80` | PointerArray::RemoveSwap | Takes the given value out of an array (+0x8C, count +0x94) by moving the last entry into its place. Called from two destructors |
| `0x9ceb20` | ParticleBaseDesc::RemoveLink | Takes the given value out of the description's array (+0x70, count +0x78), moving the later entries down. Called from ParticleBaseDesc::~ParticleBaseDesc |
| `0xbcf120` | String::CompareWide | Compares a text with a wide text character by character up to the shorter length: the difference at the first that differs, otherwise the difference in length. Called from AvatarDatabase::Load |
| `0xbcea50` | WideString::CompareNarrow | Compares a wide text with a text the same way. Called from Files::FindCoasters |
| `0x7fb900` | SortedMap::FindText | Binary search for a text key in a sorted array of C texts: gives the position where it is (or would go) and whether it was found (the same code as `0x807970`) |
| `0x805c10` | SortedMap::FindText | Binary search for a text key in a sorted array of C texts: gives the position where it is (or would go) and whether it was found (the same code as `0x807970`) |
| `0xbced70` | WideString::Find | Searches a wide text forwards from the given position for a character: its position, or -1 when the end comes first or the position is past the end. Used by Movies::List, SceneryItem::SetupEffects and SIDSoundEditor::Build |
| `0xbced20` | WideString::FindChar | The same forward search for a 16-bit character. Called from Text::FromNumber |
| `0xbd1940` | KeyCombination::Set | Sets a key combination from up to four keys, keeping the non-zero ones in order and their count (+0x10). Used by the developer key commands (ToggleAttractSequence, CycleAnimsPeople, DecAutoAttDist, ...) |
| `0xa6bf80` | PhysicsWorld::RemoveBody | Takes the given body out of the world's list (+0x60, count +0x68), moving the later ones down. Called from a destructor |
| `0x4796f0` | Options::SetFloat | Sets number option n (the table at `0x10ED860`, 20 bytes each) to the value given, kept between the option's minimum (+8) and maximum (+0xC). Called from Game::Initialise |
| `0x479630` | Options::SetInt | Sets whole-number option n (the table at `0x10ED668`, 20 bytes each) to the value given, kept between its minimum (+8) and maximum (+0xC). Used by Language::Switch, GameOptionsDialog::BuildSound and FrontEnd::Start |
| `0x4ff6f0` | HandleRef::FromMember | Makes the given reference (the reference base class `0xF26AE0`) refer to the handle kept at +0x18: counts a use of the handle (+8) and puts the reference first in the handle's list of references (+0x20) |
| `0x6dfad0` | HandleRef::FromMember | Makes the given reference (the reference base class `0xF26AE0`) refer to the handle kept at +0x80: counts a use of the handle (+8) and puts the reference first in the handle's list of references (+0x20). Used by Window builder |
| `0x6dfb10` | HandleRef::FromMember | Makes the given reference (the reference base class `0xF26AE0`) refer to the handle kept at +0x94: counts a use of the handle (+8) and puts the reference first in the handle's list of references (+0x20). Used by Window builder |
| `0x7f4a40` | HandleRef::FromMember | Makes the given reference (the reference base class `0xF26AE0`) refer to the handle kept at +0x48: counts a use of the handle (+8) and puts the reference first in the handle's list of references (+0x20). Used by Vendor::Sell |
| `0x7f4a80` | HandleRef::FromMember | Makes the given reference (the reference base class `0xF26AE0`) refer to the handle kept at +0xec: counts a use of the handle (+8) and puts the reference first in the handle's list of references (+0x20). Used by Vendor::Sell |
| `0x825b20` | HandleRef::FromMember | Makes the given reference (the reference base class `0xF26AE0`) refer to the handle kept at +0xc0: counts a use of the handle (+8) and puts the reference first in the handle's list of references (+0x20). Used by Constructor |
| `0x92ecd0` | HandleRef::FromMember | Makes the given reference (the reference base class `0xF26AE0`) refer to the handle kept at +0x44: counts a use of the handle (+8) and puts the reference first in the handle's list of references (+0x20) |
| `0x98a5f0` | HandleRef::FromMember | Makes the given reference (the reference base class `0xF26AE0`) refer to the handle kept at +0xa0: counts a use of the handle (+8) and puts the reference first in the handle's list of references (+0x20). Used by FireworkDisplay::Check |
| `0x80ed70` | PointerArray::RemoveSwap2 | Takes the given value out of an array (+0x5C, count +0x64) by moving the last entry into its place. Called from two destructors |
| `0xcbb250` | Inflate::BuildHuffmanTable | zlib's huft_build: from a list of code lengths builds the decoding tables (counting the codes of each length, working out the first code of each, then filling the multi-level lookup tables with their base values and extra bits from the lists given); reports an incomplete or over-subscribed set of lengths. Called from Inflate::BuildBitLengthTree and Inflate::BuildTrees |
| `0xcc0670` | Inflate::Fast | zlib's inflate_fast: decodes literals and length/distance pairs straight from the input with the two tables while at least 258 bytes of output room and 10 bytes of input are left, copying matches from the sliding window (wrapping around it), using the bit masks at `0x10F4580`; hands back to the slower decoder at the end of a block or an invalid code. Called from Inflate::Codes |
| `0x4d1aa0` | ManagedObject::ActivateUnlessFlagged | Activates the object (ManagedObject::Activate) unless flag 1 at +0x2C is set. Shared by many person-info classes (slots 7, 8, 16) |
| `0x5487a0` | GeneralEntry::Slot3 | Calls the entry's own slots 11, 8, 7 and 9 in turn. Slots 3 and 4 of GeneralEntry and WaterfallSplashEntry |
| `0x475180` | Random::Ran0 | A second random number generator, Numerical Recipes' ran0: its state (`0x10EC9E0`, kept XORed with 123459876) is advanced by the minimal standard generator (times 48271, using Schrage's method with 44488, modulo 2^31 - 1); returns the low 23 bits divided by 2^23, a number from 0 up to 1. Used for the many random chances of guests, staff and scenery |
| `0xcdee80` | SlotPool::Add | Puts an object into the first free slot of a pool (slots +0, free list +8 with its count +0x10, growing by +0x14 slots when none are free), keeping the lowest (+0x18) and one past the highest (+0x1C) used slot; the slot is what the caller keeps (litter bins, guest injection points, ...) |
| `0xcdede0` | SlotPool::Remove | Empties slot n of the pool and puts it on the free list (growing the list when needed); when it was the lowest used slot, finds the next used one |
| `0xcd2b30` | ListMember::ListMember | Constructor of an object (table `0xFC34EC`) that keeps the list it is given at +4 and adds itself to it (PointerArray::Add). Used by about 220 constructors |
| `0xcd2b50` | ListMember::~ListMember | Puts its table back and takes itself out of its list (PointerArray::Remove) |
| `0xcdb9b0` | PointerArray::Add | Appends a pointer to an array ({data, capacity, count, growth step}), growing it by the step when it is full |
| `0xcdb9e0` | PointerArray::Remove | Takes the first copy of a pointer out of an array, moving the later ones down |
| `0xbcfb10` | String::FromC | Makes a shared text from C text: use count 1, its length, a capacity rounded up to 16 bytes, an aligned block holding a copy. Used by about 220 functions |
| `0x402b90` | ArrayConstruct | Calls the given function once for each of the n items of an array (the compiler's array constructor loop). Used by about 180 functions |
| `0xbe4060` | SaveWriter::Write | Writes bytes to a save stream: waits until the writer is ready (`0xC569B0`), writes them (`0xC53530`), then waits again. Used by about 130 save functions |
| `0xc569b0` | SaveWriter::Wait | Waits for the stream's writing to finish, running `0xC56320` until its busy flag clears (while marking `0x13E6C60`) |
| `0xbb5160` | OffsetRef::Owner | When the entry's first value is not -1, the object it is embedded in (this - 0x38); otherwise the object stored at +0x24 |
| `0xbb3e70` | OffsetRef::Owner2 | When the first value is not -1, this - 0x38; otherwise the object at +0x18. Used by about 145 functions |
| `0xbb4280` | OffsetRef::Owner3 | When the first value is not -1, this - 0x18; otherwise the object at +0x12C. Used by about 115 functions |
| `0xbb51b0` | OffsetRef::Offset38 | Returns 0x38 |
| `0xbb3d70` | OffsetRef::Offset18 | Returns 0x18 |
| `0xbe7450` | Rect::Assign | Copies a rectangle (four values). Used by about 115 functions |
| `0xbd1420` | Flags::Bit0 | Bit 0 of the value at +4. Used by about 105 functions |
| `0xcd23a0` | RunUntilDone | Calls `0xCD21E0` with its two arguments until it returns -1 |
| `0xbe8fe0` | Call10IfReady | When `0xBE8E40` gives something, `0xBE8EA0` with all ten arguments; otherwise 0 |
| `0xbec340` | ForCurrent | `0xBEC1A0` with the first value of what `0xBD9670` gives and the argument |
| `0xbd9fa0` | UpdateAndCheck | Runs `0xC374C0` and `0xC37220`, then whether `0x13D0DF0` is 0 |
| `0x67b4b0` | Thunk67B3C0 | Calls `0x67B3C0` |
| `0xa63450` | MainCamera::Get | The main camera (kept in `0x13429A8`): made the first time it is asked for (1,056 bytes, MainCamera::MainCamera) and activated; returns it. Used by about 150 functions |
| `0x491140` | EnclosureManager::Get | The enclosure manager (kept in `0x13D0258`): made the first time (216 bytes, EnclosureManager::EnclosureManager) and activated; returns it. Used by about 115 functions |
| `0xbd0520` | String::SharedEmpty | The shared empty text (at `0x13D0D6C`): made once (use count 1, length 0, a 16-byte block holding 0; freed at exit), its use count raised when asked; returns it. Used by about 105 functions |
| `0x410c90` | NamedTextRef::NamedTextRef | Constructor of a text reference found by name (tables `0xF264EC`, then `0xF26530`): makes the name from C text when asked (`0x403E10`) and looks it up with `0xBD15A0`. Used by about 115 functions |
| `0x805650` | Handle::ForId | The handle for a 64-bit object id: the one in the handle map (`0xBE82B0`) with its use count raised, or a new 40-byte handle (the id, use count 1, an empty list of references) added to the map (`0xBE8230`); the next free id (`0x10F06C8`) is moved past it when needed. Used by about 95 functions |
| `0x808840` | IdMap::Resolve | Turns an id from a save file into the id it has in this game: the one already chosen for it in the map, or a new mapping to the next free id (`0x10F06C8`, 64-bit, then advanced). Used by about 95 load functions |
| `0x40a010` | PathManager::Get | The path manager (kept in `0x130625C`): made the first time (208 bytes, PathManager::PathManager) and activated; returns it. Used by about 90 functions |
| `0xbd02b0` | String::Append | Appends text to a shared text: in place when this is its only user (`0xBCFC30`), otherwise into a new copy (`0xBCFB80`) that this then holds. Used by about 90 functions |
| `0xbd0810` | WideString::Substring | A new wide text holding the part of this one from the start position for the given length (kept within the text). Used by about 90 functions |
| `0x403ce0` | WideString::FromCAndWide | A new wide text made of the C text given followed by the wide text given. Used by about 90 functions |
| `0x444090` | GameTime::Get | The game clock (kept in `0x1106C54`): made when asked for (96 bytes, GameTime::GameTime) and activated; returns it. Used by about 80 functions |
| `0x436770` | WaterManager::Get | The water manager (kept in `0x133A270`): made the first time (376 bytes, WaterManager::WaterManager) and activated; returns it. Used by about 75 functions |
| `0xcd23d0` | String::FromNumber | A new shared text holding a whole number written in decimal (`%d`). Used by about 80 functions |
| `0x403e10` | String::AssignC | Sets a shared text to a copy of C text (its length found, then `0xBD0CE0`); returns it. Used by about 75 functions |
| `0xbd15a0` | TextRef::TextRef | Constructor of a reference to a text (table `0xFA14D0`): keeps the text table's entry (+0x24 of the table given), the name (its use count raised) and joins the entry's list of references. Used by about 65 functions |
| `0x441030` | Listener440::Listener440 | Constructor of an object (table `0xF28CC4`) that joins the list of listeners of the object `0x440FD0` gives (its list at +0x1C), first |
| `0x440ed0` | Listener440::~Listener440 | Puts its table back and leaves that list (stopping the program if the list is found broken) |
| `0x48a400` | IdSet::IdSet | Constructor of a set of ids: clears its first value and sets its storage up with `0xCDED30` (256 slots, growing by 128) |
| `0xbdf160` | LookupValue | The first value of the entry `0xBDD210` finds for the key, or 0 when there is none |
| `0xce4770` | Flags::Set20Clear21 | Sets the byte at +0x20 and clears the one at +0x21 |
| `0x4882d0` | ResourceHolder::~ResourceHolder | Puts its table back (`0xF2E308`) and, when it holds something (+0x18), releases it with the class's release function (`0xF2E30C`) and clears it. Used by about 55 destructors |
| `0x843900` | Zero6 | Clears six values. Used by about 50 functions |
| `0x40f6e0` | Reference::ToObject | Makes this a reference (the reference base class `0xF26AE0`) to the given object: its handle (`0x805700`) at +4, joining the handle's list of references first |
| `0x481560` | Registry::Ensure | Makes the global at `0x12F4604` (36 bytes, all zero) the first time it is needed |
| `0xcd6480` | Counter::Increment | Adds one to the value at +0xC. Used by about 45 functions |
| `0xbcf710` | WideString::FromWideN | Makes a shared wide text from up to n characters of wide text (stopping at a 0): use count 1, length n, a capacity rounded up to 16 characters |
| `0xbd1550` | TextRef::TextRefEmpty | Constructor of a text reference (table `0xFA14D0`) that refers to no text (`0xBD0C50` with 0) |
| `0xbe47b0` | Callback::~Callback | Deletes the object it holds at +4 (its deleting destructor with 1, given this), then `0xBCDDF0` |
| `0x4042d0` | Forwarder::Slot3 | Calls slot 3 of the object at +4 with the value and returns the value |
| `0x4363d0` | Timer::Ticks | Reads the processor's time-stamp counter (rdtsc). Used by about 35 timing functions |
| `0xbec8a0` | ReportMissingInterface | Reports `Failed to get interface in %s (line %d).` with the source file and line given (`0xC34850`). Used by about 35 functions |
| `0x458150` | SharedText::Copy1106DC4 | Makes this hold the shared text kept in `0x1106DC4`, raising its use count |
| `0xbe40b0` | Get8Then40 | The value at +0x40 of the object at +8 of the object this points to |
| `0x6d6180` | RegisteredItem::RegisteredItem | Constructor of a base class (table `0xF63F2C`) whose objects all go in one global list (`0x1305F44`, count `0x1305F4C`, growing to twice its size plus 10): keeps its place in the list at +4. Used by about 45 constructors |
| `0x468450` | ShowMessageBox | Shows a Windows message box (MessageBoxA) with two wide texts, first turning each into plain text in spare room at the end of its own buffer. Used by about 60 functions to report problems |
| `0x49b9f0` | ReportMissingResourceSymbol | When the flag (Flags::Bit0) is clear, shows a message box `Missing Resource Symbol` with the symbol's name |
| `0x40c380` | Table2C::Value | The value at +8 of entry n (36 bytes each) of the table at +0x2C |
| `0xbce060` | Memory::FreeCounted | Frees a block, counting it off the live blocks (`0x13D0D48`) |
| `0x4797b0` | Options::SetBool | Sets yes/no option n (the table at `0x10EDAB8`, 8 bytes each) to 1 or 0 |
| `0xbe4090` | SaveWriter::Run | `0xC50B70` with the argument, then waits for the writing to finish (SaveWriter::Wait) |
| `0xce7cc0` | Registry::Ensure2 | Makes the global at `0x1400B70` (36 bytes: zeros, -1 at +4) the first time it is needed |
| `0x413650` | Object413::Object413 | Constructor (table `0xF26E00`): clears +4..+0xC, keeps itself at +0x10 and sets up from the argument with `0x412EA0` |
| `0x458130` | Init458 | Runs `0x458010` and returns its argument |
| `0xcc1c60` | Release2C | `0xCC4EA0` with the value at +0x2C |
| `0x624440` | WaterfallManager::Get | The waterfall manager (kept in `0x133A174`): made the first time (1,008 bytes, WaterfallManager::WaterfallManager) and activated; returns it |
| `0x4420e0` | WideString::AssignWide | Sets a wide text to a copy of a 0-ended wide text (its length found, then `0xBD0640`); returns it |
| `0xc34850` | Report | Formats a message from the text and the values after it (`0xC345E0`, then `0xC32720`), as ReportMissingInterface and others use |
| `0x59a2d0` | Clamp0To3 | Gives 1, 2 or 3 for those values and 0 for anything else |
| `0xbc51e0` | Extent::Clear | Clears the first two values and sets the three at +0x1C, +0x2C and +0x3C to the largest float |
| `0x9ead70` | Weather::Get | The weather (kept in `0x1341930`): made when asked for (280 bytes, Weather::Weather) and activated; returns it |
| `0x68f640` | AssertFailed | Shows an `Assertion Failed!` message (`File: %s`, `Line: %d`, `Expression: %s`, `Details: %s`) |
| `0xc1a8c0` | Timer::Milliseconds | The time in milliseconds from the high-resolution counter (QueryPerformanceCounter times 1000, divided by the counter's frequency kept at `0x13E47C0`) |
| `0xbcfa40` | String::FromCN | Makes a shared text from n characters of C text: use count 1, length n, a capacity rounded up to 16 bytes, an aligned block holding the copy |
| `0xbce020` | Memory::AllocCounted | Allocates a 16-byte-aligned block, counting it among the live blocks (`0x13D0D48`) |
| `0xbe8400` | SortedTree::LastValue | Follows the tree's +0x10 links from its root (+4) down to the last node holding data and returns that node's value (+0xC); 0 for an empty tree |
| `0xbb5130` | OffsetRef::Owner4 | When the first value is not -1, this - 0x18; otherwise the object at +0x14 |
| `0x8fb150` | PooledObject::PooledObject | Constructor (table `0xF7CF54`) that puts the object in the slot pool (SlotPool::Add) and keeps its slot at +4 |
| `0x8fb110` | PooledObject::~PooledObject | Puts its table back and empties its slot in the pool (SlotPool::Remove) |
| `0x7fade0` | ListMember7F::ListMember7F | Constructor (table `0xF75138`) of a member of the given object's list (its +0x14, joined first), keeping the object (+0x10) and the value given (+0x14) |
| `0x7e5100` | Buffer::Free | Frees the block this points to (aligned free) |
| `0x7e5110` | Buffer16::Reserve | Makes room for n 16-byte entries when its capacity (a 16-bit count at +4) is smaller: the old block freed and a new one allocated |
| `0xc374c0` | Static::EnsureC374C0 | Sets up a static object once (`0xC365A0`, freed at exit) and returns it |
| `0x48caf0` | Call44CC40And489A10 | `0x44CC40` then `0x489A10`, both with the same three arguments |
| `0xcdb780` | Scroller::Reset | Sets both of its value triples to (0.1, 100, 0) |
| `0xcded30` | SlotPool::Init | Sets up a slot pool for n slots, growing by the step given: the slots cleared, the free list holding every slot (the highest first, so the lowest is taken first), the used range empty |
| `0xbd0e90` | String::Substring | A new text holding the part of this one from the start position for the given length (kept within the text) |
| `0x46f560` | Array8::SetCapacity | Resizes an array of 8-byte items to n: a new aligned block whose items are constructed (`0x402AE0`, through ArrayConstruct), the old items that fit copied over and the old block freed; returns n |
| `0x423de0` | Array4::SetCapacity | Resizes an array of 4-byte items to n, copying over those that fit; returns n |
| `0x41cab0` | NamedTextRef2::NamedTextRef2 | Constructor of a text reference found by name (table `0xF27364`; the name made from C text when asked, with `0xF27398`), like NamedTextRef |
| `0xcd27d0` | WideString::ToLower | A new wide text holding this one with every character made lower-case (tolower) |
| `0xbd12c0` | String::AssignString | Sets a text to a copy of another text's characters (`0xBD0720` with its data and length); returns it |
| `0xbe4030` | SaveWriter::Finish | Runs `0xC50250`, waits for the writing to finish (SaveWriter::Wait) and gives the stream's value at +0x44 (0 when it is -1) |
| `0x808700` | IdMap::IdMap | Constructor of an id map: two empty arrays and a growth step of 32 |
| `0x7e50e0` | Buffer16::Buffer16 | Clears a buffer header: no block, and its three 16-bit counts 0 |
| `0x67a3b0` | Window::Slot48Self | Calls the window's slot 48 with its value at +0xE0 and itself |
| `0x475230` | Random::Ran0Range | A random number between two values (Random::Ran0): the first when the second is not larger, otherwise first + (second - first) x Ran0 |
| `0xbf4680` | Memory::FreePlain | Frees a block with the runtime's free |
| `0xc69eb0` | Object69E::Object69E | Constructor built on `0xC69910` (with three of its arguments) that sets its table `0xFB82A8` and keeps the fourth argument at +0xD0 |
| `0x558f80` | Interface04::Get | The interface (id in `0x130BBEC`) of the object its reference at +4 points to, looked up once (flag +0x18) |
| `0x7a66b0` | Interface44::Get | The interface (id in `0x1306C7C`) of the object its reference at +0x44 points to, or null |
| `0xb44970` | Interface44b::Get | The interface (id in `0x13CE94C`) of the object its reference at +0x44 points to, looked up once (flag +0x58) |
| `0x772b50` | Interface50::Get | When +0x48 is set, the guest interface (id in `0x130654C`) of the object its reference at +0x50 points to; otherwise null |
| `0x510d90` | Table310::Entry | Entry n of the table at +0x310 (28 bytes each): the interface (id in `0x12F515C`) of the object it refers to, looked up once per entry |
