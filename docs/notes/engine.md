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
| `0x5d10b0` | Settings5D::Defaults | Sets ten values to 3, 3, 4, 0, 0, 0, 0, 0, 1, 2 |
| `0x454800` | Static::Ensure1106D50 | Sets up the static object at `0x1106D50` once (`0x453F30`) and returns it |
| `0xbb5190` | OffsetRef::Owner5 | When the first value is not 2, this - 0x38; otherwise the object at +4 |
| `0xaa0250` | Table30::Entry | Entry n of the table at +0x30 (44 bytes each, the reference at +0x10 of each, count +0x34): the interface of the object it refers to, looked up once per entry; null outside the table |
| `0x989090` | Table104::Entry | Entry n (88 bytes each) of the table at +0x104 (count +0x108), or null outside it |
| `0x85cb20` | IntToFloat | The whole number given as a float |
| `0x8439b0` | Box::Assign | Copies six values (a box's corners) |
| `0x4a1880` | Offset138 | The value at +0x138, plus 0x38 when `0xBB5150` is true and it is not 0 |
| `0xca55f0` | Writer::Write4 | Writes a 4-byte value at the writer's position (+0xC, through its own slot 3) and moves the position on by 4 |
| `0xbdf180` | LookupValue2 | The first value of the entry `0xBDD210` finds for the key, or 0 (the same code as `0xBDF160`) |
| `0xbb4230` | Entry::IsSet2 | Whether the first value is not -1 (the same as `0xBB5120`) |
| `0x967c10` | Object967::Object967 | Constructor that sets the table `0xF7E654` and keeps its argument at +8 |
| `0x964f30` | Object964::Object964 | Constructor (table `0xF7E4F4`): its argument at +4, 3 at +8, itself at +0x14 and 0 at +0x18 |
| `0x7fa730` | PointerTable::Value44 | The value at +0x44 of entry n of the pointer table at +4, or 0 when that entry is empty |
| `0x5d3d00` | ArrayDestroy | Calls the given function with 1 once for each of the n items of an array (the compiler's array delete loop) |
| `0x581220` | SmallArray::SmallArray | Clears its flag, sets its capacity to 4 and gives it a 16-byte block |
| `0x59ac60` | Interface54b::Get | The interface (id in `0x12F5544`) of the object its reference at +0x54 points to, looked up once (flag +0x68) |
| `0x558f20` | Interface04b::Get | The same as `0x558F80`: the interface (id in `0x130BBEC`) of the object its reference at +4 points to, looked up once |
| `0x4c5890` | Interface60::Get | The interface (id in `0x12F5300`) of the object its reference at +0x60 points to, looked up once (flag +0x74) |
| `0x484c70` | Node::TotalScale | The node's own scale (+0xC4) times that of every parent up the chain (+0x3C) |
| `0x4749d0` | SetTableF2CEB4 | Puts the table `0xF2CEB4` back (what is left of a destructor) |
| `0x4535c0` | ListMember453::ListMember453 | Constructor (table `0xF296F8`) of a member of the global list at `0x10EC7E4`, joined first |
| `0x4535f0` | ListMember453::~ListMember453 | Puts its table back and leaves that list (stopping the program if the list is found broken) |
| `0x436100` | TerrainFlattener::Get | The terrain flattener (kept in `0x1313C60`): made the first time (136 bytes, TerrainFlattener::TerrainFlattener) and activated; returns it |
| `0xbcf940` | WideString::Reserve | Makes an empty shared wide text with room for n characters: use count 1, length 0, a capacity rounded up to 16 characters, the block starting with 0 |
| `0xcec560` | Table10::Lookup | The entry of the table at +0x10 whose index `0xCEB190` works out from the given object's +0x28 and +0x24 and the two values given |
| `0xb44da0` | Table7C::Current | The interface of the object entry n of the table at +0x7C (28 bytes each, n at +0x78) refers to, looked up once per entry; null when n is negative |
| `0xb36e30` | Interface44c::Get | The interface (id in `0x13CE94C`) of the object its reference at +0x44 points to, looked up once (flag +0x58) |
| `0x4d1800` | Interface58::Get | The interface (id in `0x130654c`) of the object its reference at +0x58 points to, or null |
| `0x4d1880` | Interface58::Get | The interface (id in `0x1306c2c`) of the object its reference at +0x58 points to, or null |
| `0x552780` | Interface44::Get | The interface (id in `0x12f51b4`) of the object its reference at +0x44 points to, or null |
| `0x557b30` | Interface54::Get | The interface (id in `0x1306444`) of the object its reference at +0x54 points to, or null |
| `0x557b70` | Interface54::Get | The interface (id in `0x1306444`) of the object its reference at +0x54 points to, or null |
| `0x7307c0` | InterfaceBD8::Get | The interface (id in `0x1310f90`) of the object its reference at +0xbd8 points to, or null |
| `0x759800` | Interface1C0::Get | The interface (id in `0x13112f0`) of the object its reference at +0x1c0 points to, or null. Used by WaterCannon::Update |
| `0x79e260` | Interface510::Get | The interface (id in `0x12f4f44`) of the object its reference at +0x510 points to, or null |
| `0x7f5fc0` | Interface44::Get | The interface (id in `0x13d0680`) of the object its reference at +0x44 points to, or null |
| `0x828820` | Interface14C::Get | The interface (id in `0x1103b60`) of the object its reference at +0x14c points to, or null. Used by TrackPiece::Activate |
| `0x939340` | Interface3C::Get | The interface (id in `0x12f5608`) of the object its reference at +0x3c points to, or null |
| `0x984900` | Interface80::Get | The interface (id in `0x13412b8`) of the object its reference at +0x80 points to, or null |
| `0x9984a0` | Interface1BC::Get | The interface (id in `0x130b5c8`) of the object its reference at +0x1bc points to, or null |
| `0x9c4850` | Interface9C::Get | The interface (id in `0x13414b4`) of the object its reference at +0x9c points to, or null |
| `0x9d9bc0` | Interface60::Get | The interface (id in `0x1342768`) of the object its reference at +0x60 points to, or null |
| `0xa28e20` | Interface48::Get | The interface (id in `0x13427a0`) of the object its reference at +0x48 points to, or null. Used by Passport::Build |
| `0xa7eb40` | Interface44::Get | The interface (id in `0x13cde74`) of the object its reference at +0x44 points to, or null |
| `0xb47e50` | InterfaceB4::Get | The interface (id in `0x13ce968`) of the object its reference at +0xb4 points to, or null |
| `0xb51a40` | Interface138::Get | The interface (id in `0x13ce968`) of the object its reference at +0x138 points to, or null |
| `0xb59d50` | Interface144::Get | The interface (id in `0x13ceabc`) of the object its reference at +0x144 points to, or null. Used by WildAnimalItem::~WildAnimalItem |
| `0x558fe0` | Interface20::GetCached | The interface (id in `0x130bbec`) of the object its reference at +0x20 points to, looked up the first time only (kept at +0x30 with the flag at +0x34), or null |
| `0x559040` | Interface20::GetCached | The interface (id in `0x130bbec`) of the object its reference at +0x20 points to, looked up the first time only (kept at +0x30 with the flag at +0x34), or null |
| `0x71df50` | Interface4C::GetCached | The interface (id in `0x130646c`) of the object its reference at +0x4c points to, looked up the first time only (kept at +0x5c with the flag at +0x60), or null |
| `0x981ab0` | Interface2C::GetCached | The interface (id in `0x13412c8`) of the object its reference at +0x2c points to, looked up the first time only (kept at +0x3c with the flag at +0x40), or null |
| `0x9bc6e0` | Interface24::GetCached | The interface (id in `0x1341524`) of the object its reference at +0x24 points to, looked up the first time only (kept at +0x34 with the flag at +0x38), or null |
| `0xa97710` | Interface64::GetCached | The interface (id in `0x13ce3bc`) of the object its reference at +0x64 points to, looked up the first time only (kept at +0x74 with the flag at +0x78), or null |
| `0xaccc40` | Interface60::GetCached | The interface (id in `0x13ce450`) of the object its reference at +0x60 points to, looked up the first time only (kept at +0x70 with the flag at +0x74), or null. Used by LaserDesigner::Build |
| `0xae8c80` | Interface4C::GetCached | The interface (id in `0x13ce450`) of the object its reference at +0x4c points to, looked up the first time only (kept at +0x5c with the flag at +0x60), or null. Used by WaterJetDesigner::Build |
| `0xae8ce0` | Interface68::GetCached | The interface (id in `0x13ce450`) of the object its reference at +0x68 points to, looked up the first time only (kept at +0x78 with the flag at +0x7c), or null. Used by WaterJetDesigner::Build |
| `0xaf0e90` | Interface60::GetCached | The interface (id in `0x1341354`) of the object its reference at +0x60 points to, looked up the first time only (kept at +0x70 with the flag at +0x74), or null |
| `0xb449d0` | Interface44::GetCached | The interface (id in `0x13ce94c`) of the object its reference at +0x44 points to, looked up the first time only (kept at +0x54 with the flag at +0x58), or null. Used by WildAnimal::FullName, WildAnimal::Release |
| `0xb53710` | Interface44::GetCached | The interface (id in `0x13ce968`) of the object its reference at +0x44 points to, looked up the first time only (kept at +0x54 with the flag at +0x58), or null. Used by WildAnimal::NameBaby, WildAnimal::Birth |
| `0xb57530` | Interface6C::GetCached | The interface (id in `0x13ce730`) of the object its reference at +0x6c points to, looked up the first time only (kept at +0x7c with the flag at +0x80), or null |
| `0xb7b8f0` | Interface60::GetCached | The interface (id in `0x13cea98`) of the object its reference at +0x60 points to, looked up the first time only (kept at +0x70 with the flag at +0x74), or null. Used by WildAnimal::GiveBirth |
| `0x416f10` | FlatTagManager::Get | The object kept in `0x1103c1c`: made the first time it is needed (88 bytes, constructed by `0x430010` (FlatTagManager::FlatTagManager)) and activated; returns it |
| `0x4366e0` | SimpleResearch::Get | The object kept in `0x1341308`: made the first time it is needed (168 bytes, constructed by `0x9852b0` (SimpleResearch::SimpleResearch)) and activated; returns it |
| `0x47daa0` | Sky::Get | The object kept in `0x1341864`: made the first time it is needed (1,120 bytes, constructed by `0x9e51d0` (Sky::Sky)) and activated; returns it |
| `0x47dc30` | WaterFlowManager::Get | The object kept in `0x133a1d8`: made the first time it is needed (440 bytes, constructed by `0x92f800` (WaterFlowManager::WaterFlowManager)) and activated; returns it |
| `0x7f0060` | StaffSettings::Get | The object kept in `0x130aad8`: made the first time it is needed (72 bytes, constructed by `0x7efce0` (StaffSettings::StaffSettings)) and activated; returns it |
| `0x809520` | GUI2_CurrentDatDesc::Get | The object kept in `0x12f7eb4`: made the first time it is needed (88 bytes, constructed by `0x63f550` (GUI2_CurrentDatDesc::GUI2_CurrentDatDesc)) and activated; returns it |
| `0x87ecd0` | FlexiColourGroupCache::Get | The object kept in `0x130ac4c`: made the first time it is needed (72 bytes, constructed by `0x80d930` (FlexiColourGroupCache::FlexiColourGroupCache)) and activated; returns it |
| `0xa5de40` | Awards::Get | The object kept in `0x134297c`: made the first time it is needed (136 bytes, constructed by `0xa5d6f0` (Awards::Awards)) and activated; returns it |
| `0x42a430` | QueryInterfaceReport 0x12f0088 | The interface (id in `0x12f0088`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0x42a480` | QueryInterfaceReport 0x12f464c | The interface (id in `0x12f464c`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0x440f10` | ListMember::LeaveF28CC8 | Destructor (table `0xf28cc8`) of a list member: leaves its list (stopping the program if the list is found broken) |
| `0x440f50` | ListMember::LeaveF28CCC | Destructor (table `0xf28ccc`) of a list member: leaves its list (stopping the program if the list is found broken) |
| `0x440f90` | ListMember::LeaveF28CD4 | Destructor (table `0xf28cd4`) of a list member: leaves its list (stopping the program if the list is found broken) |
| `0x483de0` | ListMember::LeaveF2DFD0 | Destructor (table `0xf2dfd0`) of a list member: leaves its list (stopping the program if the list is found broken) |
| `0x484020` | QueryInterfaceReport 0x12f0020 | The interface (id in `0x12f0020`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0x4ca9b0` | InterfaceD8::Get | The interface (id in `0x12f4c64`) of the object its reference at +0xd8 points to, or null |
| `0x4ca9f0` | InterfaceD8::Get | The interface (id in `0x13cdf10`) of the object its reference at +0xd8 points to, or null |
| `0x4caa30` | InterfaceD8::Get | The interface (id in `0x13ce968`) of the object its reference at +0xd8 points to, or null |
| `0x4ce710` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x4d3ae0` | QueryInterfaceReport 0x12f4f30 | The interface (id in `0x12f4f30`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0x4de920` | QueryInterfaceReport 0x12f4f44 | The interface (id in `0x12f4f44`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0x4df7b0` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x503c90` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x593f60` | ListMember::LeaveF3CF10 | Destructor (table `0xf3cf10`) of a list member: leaves its list (stopping the program if the list is found broken) |
| `0x620a80` | ListMember::LeaveF43E60 | Destructor (table `0xf43e60`) of a list member: leaves its list (stopping the program if the list is found broken) |
| `0x670740` | QueryInterfaceReport 0x13412b8 | The interface (id in `0x13412b8`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0x678e70` | QueryInterfaceReport 0x12f81fc | The interface (id in `0x12f81fc`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0x67b9a0` | QueryInterfaceReport 0x12f8218 | The interface (id in `0x12f8218`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0x6a0760` | QueryInterfaceReport 0x1305c18 | The interface (id in `0x1305c18`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0x6cd8e0` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6d2f00` | NameText 0x0 | Makes the given text a new copy of the text at +8 of the object `0x0` points to (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6d2f80` | NameText 0x0 | Makes the given text a new copy of the text at +8 of the object `0x0` points to (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6d3000` | NameText 0x0 | Makes the given text a new copy of the text at +8 of the object `0x0` points to (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6d3080` | NameText 0x0 | Makes the given text a new copy of the text at +8 of the object `0x0` points to (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6d3100` | NameText 0x0 | Makes the given text a new copy of the text at +8 of the object `0x0` points to (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6d3180` | NameText 0x0 | Makes the given text a new copy of the text at +8 of the object `0x0` points to (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6d3200` | NameText 0x0 | Makes the given text a new copy of the text at +8 of the object `0x0` points to (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6d3280` | NameText 0x0 | Makes the given text a new copy of the text at +8 of the object `0x0` points to (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6d3300` | NameText 0x0 | Makes the given text a new copy of the text at +8 of the object `0x0` points to (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6d3380` | NameText 0x0 | Makes the given text a new copy of the text at +8 of the object `0x0` points to (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6d3400` | NameText 0x0 | Makes the given text a new copy of the text at +8 of the object `0x0` points to (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6d3480` | NameText 0x0 | Makes the given text a new copy of the text at +8 of the object `0x0` points to (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6d3500` | NameText 0x0 | Makes the given text a new copy of the text at +8 of the object `0x0` points to (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6d3580` | NameText 0x0 | Makes the given text a new copy of the text at +8 of the object `0x0` points to (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6d3600` | NameText 0x0 | Makes the given text a new copy of the text at +8 of the object `0x0` points to (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6d3680` | NameText 0x0 | Makes the given text a new copy of the text at +8 of the object `0x0` points to (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6d3700` | NameText 0x0 | Makes the given text a new copy of the text at +8 of the object `0x0` points to (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6d3780` | NameText 0x0 | Makes the given text a new copy of the text at +8 of the object `0x0` points to (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6d3800` | NameText 0x0 | Makes the given text a new copy of the text at +8 of the object `0x0` points to (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6d3880` | NameText 0x0 | Makes the given text a new copy of the text at +8 of the object `0x0` points to (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6d3900` | NameText 0x0 | Makes the given text a new copy of the text at +8 of the object `0x0` points to (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6d3980` | NameText 0x0 | Makes the given text a new copy of the text at +8 of the object `0x0` points to (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6d3a00` | NameText 0x0 | Makes the given text a new copy of the text at +8 of the object `0x0` points to (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6d3a80` | NameText 0x0 | Makes the given text a new copy of the text at +8 of the object `0x0` points to (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6e9360` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6ebca0` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6f26c0` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x7291a0` | InterfaceD8::Get | The interface (id in `0x1306444`) of the object its reference at +0xd8 points to, or null |
| `0x735fd0` | Interface94::Get | The interface (id in `0x12f51b4`) of the object its reference at +0x94 points to, or null |
| `0x741fb0` | Interface4C::Get | The interface (id in `0x1306920`) of the object its reference at +0x4c points to, or null |
| `0x757520` | ListMember::LeaveF6C374 | Destructor (table `0xf6c374`) of a list member: leaves its list (stopping the program if the list is found broken) |
| `0x772c00` | Interface6C::Get | The interface (id in `0x13064bc`) of the object its reference at +0x6c points to, or null |
| `0x772f70` | Interface104::Get | The interface (id in `0x13063bc`) of the object its reference at +0x104 points to, or null |
| `0x788eb0` | InterfaceDC::Get | The interface (id in `0x130654c`) of the object its reference at +0xdc points to, or null |
| `0x7b0750` | Interface138::Get | The interface (id in `0x1309020`) of the object its reference at +0x138 points to, or null |
| `0x7c3430` | Interface1F0::Get | The interface (id in `0x1309e10`) of the object its reference at +0x1f0 points to, or null |
| `0x7c44f0` | QueryInterfaceReport 0x1306c0c | The interface (id in `0x1306c0c`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0x7eed40` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x7fae20` | ListMember::LeaveF75138 | Destructor (table `0xf75138`) of a list member: leaves its list (stopping the program if the list is found broken) |
| `0x824790` | QueryInterfaceReport 0x130b598 | The interface (id in `0x130b598`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0x827eb0` | Interface8C::Get | The interface (id in `0x1310f90`) of the object its reference at +0x8c points to, or null |
| `0x828120` | QueryInterfaceReport 0x1103b60 | The interface (id in `0x1103b60`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0x8720c0` | ListMember::LeaveF78B60 | Destructor (table `0xf78b60`) of a list member: leaves its list (stopping the program if the list is found broken) |
| `0x8e94e0` | ListMember::LeaveF7CCBC | Destructor (table `0xf7ccbc`) of a list member: leaves its list (stopping the program if the list is found broken) |
| `0x946f80` | ListMember::LeaveF7E29C | Destructor (table `0xf7e29c`) of a list member: leaves its list (stopping the program if the list is found broken) |
| `0x99bd10` | QueryInterfaceReport 0x13413e8 | The interface (id in `0x13413e8`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0x9b6900` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x9c4370` | QueryInterfaceReport 0x13414b4 | The interface (id in `0x13414b4`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0xa56bd0` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0xa82680` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0xb12cb0` | QueryInterfaceReport 0x13ce74c | The interface (id in `0x13ce74c`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0xb17a30` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0xb19c40` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0xb2d630` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0xb2f590` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0xb70140` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0xb701c0` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0xb7b950` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0xb7c070` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0xb7c2e0` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0xb7c720` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0xbda9f0` | QueryInterfaceReport 0x13e6b08 | The interface (id in `0x13e6b08`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0xbf4f10` | QueryInterfaceReport 0x13e4904 | The interface (id in `0x13e4904`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0xc0a5b0` | ParserNode::DeletingDestructor | Destroys the parser node (`0xc09bf0`) and, when bit 0 of the flag is set, gives its memory back to the parser (Parser::Free); returns it |
| `0xc0a860` | ParserNode::DeletingDestructor | Destroys the parser node (`0xc0a750`) and, when bit 0 of the flag is set, gives its memory back to the parser (Parser::Free); returns it |
| `0xc0dc50` | ParserNode::DeletingDestructor | Destroys the parser node (`0xc0dc80`) and, when bit 0 of the flag is set, gives its memory back to the parser (Parser::Free); returns it |
| `0xc0ddb0` | ParserNode::DeletingDestructor | Destroys the parser node (`0xc0dde0`) and, when bit 0 of the flag is set, gives its memory back to the parser (Parser::Free); returns it |
| `0xc0def0` | ParserNode::DeletingDestructor | Destroys the parser node (`0xc0df20`) and, when bit 0 of the flag is set, gives its memory back to the parser (Parser::Free); returns it |
| `0xc0e2a0` | ParserNode::DeletingDestructor | Destroys the parser node (`0xc0e2d0`) and, when bit 0 of the flag is set, gives its memory back to the parser (Parser::Free); returns it |
| `0xc0e5b0` | ParserNode::DeletingDestructor | Destroys the parser node (`0xc0e5e0`) and, when bit 0 of the flag is set, gives its memory back to the parser (Parser::Free); returns it |
| `0xc0e8b0` | ParserNode::DeletingDestructor | Destroys the parser node (`0xc0e8e0`) and, when bit 0 of the flag is set, gives its memory back to the parser (Parser::Free); returns it |
| `0xc0eca0` | ParserNode::DeletingDestructor | Destroys the parser node (`0xc0ecd0`) and, when bit 0 of the flag is set, gives its memory back to the parser (Parser::Free); returns it |
| `0xc0efb0` | ParserNode::DeletingDestructor | Destroys the parser node (`0xc0efe0`) and, when bit 0 of the flag is set, gives its memory back to the parser (Parser::Free); returns it |
| `0xc0f290` | ParserNode::DeletingDestructor | Destroys the parser node (`0xc0f2c0`) and, when bit 0 of the flag is set, gives its memory back to the parser (Parser::Free); returns it |
| `0xc0f480` | ParserNode::DeletingDestructor | Destroys the parser node (`0xc0f4b0`) and, when bit 0 of the flag is set, gives its memory back to the parser (Parser::Free); returns it |
| `0xc0f680` | ParserNode::DeletingDestructor | Destroys the parser node (`0xc0f6b0`) and, when bit 0 of the flag is set, gives its memory back to the parser (Parser::Free); returns it |
| `0xc0f820` | ParserNode::DeletingDestructor | Destroys the parser node (`0xc0d460`) and, when bit 0 of the flag is set, gives its memory back to the parser (Parser::Free); returns it |
| `0xc1b060` | ListMember::LeaveFAFF14 | Destructor (table `0xfaff14`) of a list member: leaves its list (stopping the program if the list is found broken) |
| `0xc37970` | QueryInterfaceReport 0x13e6b34 | The interface (id in `0x13e6b34`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0xc379c0` | QueryInterfaceReport 0x13e6b34 | The interface (id in `0x13e6b34`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0xc37a10` | QueryInterfaceReport 0x13e6b44 | The interface (id in `0x13e6b44`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0xc37a60` | QueryInterfaceReport 0x13e6b44 | The interface (id in `0x13e6b44`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0xca3810` | ChainMember::~ChainMember | Destructor (table `0xfbf370`): takes the object out of its owner's chain (the owner at +0x10, the chain from its +0x3C through each member's +0x14) by linking past it, then puts back the base table `0xfa589c` |
| `0xca38a0` | ChainMember::~ChainMember | Destructor (table `0xfbf370`): takes the object out of its owner's chain (the owner at +0x10, the chain from its +0x3C through each member's +0x14) by linking past it, then puts back the base table `0xfa589c` |
| `0xca3a20` | ChainMember::~ChainMember | Destructor (table `0xfbf370`): takes the object out of its owner's chain (the owner at +0x10, the chain from its +0x3C through each member's +0x14) by linking past it, then puts back the base table `0xfa589c` |
| `0xca3ae0` | ChainMember::~ChainMember | Destructor (table `0xfbf370`): takes the object out of its owner's chain (the owner at +0x10, the chain from its +0x3C through each member's +0x14) by linking past it, then puts back the base table `0xfa589c` |
| `0xca3be0` | ChainMember::~ChainMember | Destructor (table `0xfbf370`): takes the object out of its owner's chain (the owner at +0x10, the chain from its +0x3C through each member's +0x14) by linking past it, then puts back the base table `0xfa589c` |
| `0xca3da0` | ChainMember::~ChainMember | Destructor (table `0xfbf370`): takes the object out of its owner's chain (the owner at +0x10, the chain from its +0x3C through each member's +0x14) by linking past it, then puts back the base table `0xfa589c` |
| `0xca3ea0` | ChainMember::~ChainMember | Destructor (table `0xfbf370`): takes the object out of its owner's chain (the owner at +0x10, the chain from its +0x3C through each member's +0x14) by linking past it, then puts back the base table `0xfa589c` |
| `0xca41e0` | ChainMember::~ChainMember | Destructor (table `0xfbf370`): takes the object out of its owner's chain (the owner at +0x10, the chain from its +0x3C through each member's +0x14) by linking past it, then puts back the base table `0xfa589c` |
| `0xca4230` | ChainMember::~ChainMember | Destructor (table `0xfbf370`): takes the object out of its owner's chain (the owner at +0x10, the chain from its +0x3C through each member's +0x14) by linking past it, then puts back the base table `0xfa589c` |
| `0xca4280` | ChainMember::~ChainMember | Destructor (table `0xfbf370`): takes the object out of its owner's chain (the owner at +0x10, the chain from its +0x3C through each member's +0x14) by linking past it, then puts back the base table `0xfa589c` |
| `0xca42d0` | ChainMember::~ChainMember | Destructor (table `0xfbf370`): takes the object out of its owner's chain (the owner at +0x10, the chain from its +0x3C through each member's +0x14) by linking past it, then puts back the base table `0xfa589c` |
| `0xca4320` | ChainMember::~ChainMember | Destructor (table `0xfbf370`): takes the object out of its owner's chain (the owner at +0x10, the chain from its +0x3C through each member's +0x14) by linking past it, then puts back the base table `0xfa589c` |
| `0xca4370` | ChainMember::~ChainMember | Destructor (table `0xfbf370`): takes the object out of its owner's chain (the owner at +0x10, the chain from its +0x3C through each member's +0x14) by linking past it, then puts back the base table `0xfa589c` |
| `0xcb8b30` | QueryInterfaceReport 0x13e6fdc | The interface (id in `0x13e6fdc`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0xcb8b80` | QueryInterfaceReport 0x13e6fcc | The interface (id in `0x13e6fcc`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0xcb8bd0` | QueryInterfaceReport 0x13e6fb8 | The interface (id in `0x13e6fb8`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0xcb8c20` | QueryInterfaceReport 0x13e6f80 | The interface (id in `0x13e6f80`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0xcb8c70` | QueryInterfaceReport 0x13e6f94 | The interface (id in `0x13e6f94`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0xe03ae0` | QueryInterfaceReport 0x141dcd4 | The interface (id in `0x141dcd4`) of the given object (asking its class through slot 1); when it has none and the second argument is set, reports it (ReportMissingInterface); null when missing |
| `0x403e50` | NamedTextRefF26520::NamedTextRefF26520 | Constructor of a text reference found by name (tables `0xf264e4`, then `0xf26520`; the name made from C text when asked), like NamedTextRef |
| `0x40f870` | NamedTextRefF26AE4::NamedTextRefF26AE4 | Constructor of a text reference found by name (tables `0xf26ad4`, then `0xf26ae4`; the name made from C text when asked), like NamedTextRef |
| `0x420b70` | ObjectArray44::Resize | Makes an array of 44-byte objects hold n new ones: destroys and frees the old ones (the count kept in front), then allocates n, constructs them and keeps the count in front; returns n. Item functions: `0xe06cfb`, `0x420bb1`, `0x41d990`, `0xddcf39`, `0x420bf0`, `0x420bee`, `0x41f850`, `0xddcf99` |
| `0x460de0` | WithReference45FA50 | Calls `0x45fa50` with a temporary reference (table `0xf2a7cc`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0x471ef0` | ConstructorF2CD0C | Constructor: builds the base part (`0x471df0`), then sets its own table `0xf2cd0c`; returns the object |
| `0x473c00` | ConstructorF2CE64 | Constructor: builds the base part (`0x471df0`), then sets its own table `0xf2ce64`; returns the object |
| `0x485560` | ConstructorF2E034 | Constructor: builds the base part (`0xbd1550` (TextRef::TextRefEmpty)), then sets its own table `0xf2e034`; returns the object |
| `0x4855d0` | ConstructorF2E03C | Constructor: builds the base part (`0xbd1550` (TextRef::TextRefEmpty)), then sets its own table `0xf2e03c`; returns the object |
| `0x498080` | ConstructorF2EAAC | Constructor: builds the base part (`0xbd1550` (TextRef::TextRefEmpty)), then sets its own table `0xf2eaac`; returns the object |
| `0x4980a0` | ConstructorF2EAB4 | Constructor: builds the base part (`0xbd1550` (TextRef::TextRefEmpty)), then sets its own table `0xf2eab4`; returns the object |
| `0x4982a0` | ConstructorF2EABC | Constructor: builds the base part (`0xbd1550` (TextRef::TextRefEmpty)), then sets its own table `0xf2eabc`; returns the object |
| `0x4982e0` | ConstructorF2EAC4 | Constructor: builds the base part (`0xbd1550` (TextRef::TextRefEmpty)), then sets its own table `0xf2eac4`; returns the object |
| `0x49c850` | ObjectArray52::Resize | Makes an array of 52-byte objects hold n new ones: destroys and frees the old ones (the count kept in front), then allocates n, constructs them and keeps the count in front; returns n. Item functions: `0xe0fc0b`, `0x49c891`, `0x49bbc0`, `0xddcf39`, `0x49c8d0`, `0x49c8ce`, `0x49b780`, `0xddcf99` |
| `0x4a1df0` | NamedTextRefF30778::NamedTextRefF30778 | Constructor of a text reference found by name (tables `0xf306e8`, then `0xf30778`; the name made from C text when asked), like NamedTextRef |
| `0x4a2090` | NamedTextRefF30788::NamedTextRefF30788 | Constructor of a text reference found by name (tables `0xf306f0`, then `0xf30788`; the name made from C text when asked), like NamedTextRef |
| `0x4a2330` | NamedTextRefF30798::NamedTextRefF30798 | Constructor of a text reference found by name (tables `0xf306f8`, then `0xf30798`; the name made from C text when asked), like NamedTextRef |
| `0x4a25d0` | NamedTextRefF307A8::NamedTextRefF307A8 | Constructor of a text reference found by name (tables `0xf30700`, then `0xf307a8`; the name made from C text when asked), like NamedTextRef |
| `0x4a2870` | NamedTextRefF307B8::NamedTextRefF307B8 | Constructor of a text reference found by name (tables `0xf30708`, then `0xf307b8`; the name made from C text when asked), like NamedTextRef |
| `0x4a2b10` | NamedTextRefF307C8::NamedTextRefF307C8 | Constructor of a text reference found by name (tables `0xf30710`, then `0xf307c8`; the name made from C text when asked), like NamedTextRef |
| `0x4a2db0` | NamedTextRefF307D8::NamedTextRefF307D8 | Constructor of a text reference found by name (tables `0xf30718`, then `0xf307d8`; the name made from C text when asked), like NamedTextRef |
| `0x4a3050` | NamedTextRefF307E8::NamedTextRefF307E8 | Constructor of a text reference found by name (tables `0xf30720`, then `0xf307e8`; the name made from C text when asked), like NamedTextRef |
| `0x4a32f0` | NamedTextRefF307F8::NamedTextRefF307F8 | Constructor of a text reference found by name (tables `0xf30728`, then `0xf307f8`; the name made from C text when asked), like NamedTextRef |
| `0x4a6150` | ObjectArray28::Resize | Makes an array of 28-byte objects hold n new ones: destroys and frees the old ones (the count kept in front), then allocates n, constructs them and keeps the count in front; returns n. Item functions: `0xe10c5b`, `0x4a6191`, `0x4a47a0`, `0xddcf39`, `0x4a61d0`, `0x4a61ce`, `0x4a59d0`, `0xddcf99` |
| `0x4b4a70` | FreeMember0 | Frees the block kept at +0x0, when there is one, and clears it |
| `0x4e6490` | NamedTextRefF35540::NamedTextRefF35540 | Constructor of a text reference found by name (tables `0xf3551c`, then `0xf35540`; the name made from C text when asked), like NamedTextRef |
| `0x536ee0` | WithReference530FE0 | Calls `0x530fe0` with a temporary reference (table `0xf38160`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0x53b7b0` | ConstructorF264EC | Constructor: builds the base part (`0xbd1550` (TextRef::TextRefEmpty)), then sets its own table `0xf264ec`; returns the object |
| `0x53b7d0` | ConstructorF26530 | Constructor: builds the base part (`0xbd1550` (TextRef::TextRefEmpty)), then sets its own table `0xf26530`; returns the object |
| `0x55ad30` | WithReference4AA7E0 | Calls `0x4aa7e0` with a temporary reference (table `0xf30b3c`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0x55b7c0` | ObjectArray224::Resize | Makes an array of 224-byte objects hold n new ones: destroys and frees the old ones (the count kept in front), then allocates n, constructs them and keeps the count in front; returns n. Item functions: `0xe1e8bb`, `0x55b804`, `0x55af30`, `0xddcf39`, `0x55b849`, `0x55b847`, `0x55a090`, `0xddcf99` |
| `0x560c60` | NamedTextRefF2E03C::NamedTextRefF2E03C | Constructor of a text reference found by name (tables `0xf2e034`, then `0xf2e03c`; the name made from C text when asked), like NamedTextRef |
| `0x56bee0` | ObjectArray28::Resize | Makes an array of 28-byte objects hold n new ones: destroys and frees the old ones (the count kept in front), then allocates n, constructs them and keeps the count in front; returns n. Item functions: `0xe1f2cb`, `0x56bf21`, `0x506810`, `0xddcf39`, `0x56bf60`, `0x56bf5e`, `0x507b60`, `0xddcf99` |
| `0x59d710` | WithReference4B52D0 | Calls `0x4b52d0` with a temporary reference (table `0xf2cd3c`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0x5a2140` | ConstructorF26AD4 | Constructor: builds the base part (`0xbd1550` (TextRef::TextRefEmpty)), then sets its own table `0xf26ad4`; returns the object |
| `0x5a25c0` | ConstructorF26AE4 | Constructor: builds the base part (`0xbd1550` (TextRef::TextRefEmpty)), then sets its own table `0xf26ae4`; returns the object |
| `0x5bfe80` | NamedTextRefF3D9F4::NamedTextRefF3D9F4 | Constructor of a text reference found by name (tables `0xf3d9d8`, then `0xf3d9f4`; the name made from C text when asked), like NamedTextRef |
| `0x5c6560` | WithReference510800 | Calls `0x510800` with a temporary reference (table `0xf36f64`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0x5c71d0` | WithReference4E6410 | Calls `0x4e6410` with a temporary reference (table `0xf35618`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0x5d78d0` | WithReference4E6410 | Calls `0x4e6410` with a temporary reference (table `0xf35618`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0x5d7980` | WithReference4E6410 | Calls `0x4e6410` with a temporary reference (table `0xf35618`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0x5d7bc0` | WithReference560F00 | Calls `0x560f00` with a temporary reference (table `0xf3b08c`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0x5d7c70` | WithReference560F00 | Calls `0x560f00` with a temporary reference (table `0xf3b08c`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0x5e1500` | ObjectArray28::Resize | Makes an array of 28-byte objects hold n new ones: destroys and frees the old ones (the count kept in front), then allocates n, constructs them and keeps the count in front; returns n. Item functions: `0xe25d2b`, `0x5e1541`, `0x5d78a0`, `0xddcf39`, `0x5e1580`, `0x5e157e`, `0x5dad50`, `0xddcf99` |
| `0x5ecfe0` | WithReference5EB870 | Calls `0x5eb870` with a temporary reference (table `0xf3fadc`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0x5ed090` | WithReference5EB9B0 | Calls `0x5eb9b0` with a temporary reference (table `0xf3fae0`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0x5ed140` | WithReference5EB9B0 | Calls `0x5eb9b0` with a temporary reference (table `0xf3fae0`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0x5efcc0` | WithReference5EB9B0 | Calls `0x5eb9b0` with a temporary reference (table `0xf3fae0`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0x640320` | ObjectArray28::Resize | Makes an array of 28-byte objects hold n new ones: destroys and frees the old ones (the count kept in front), then allocates n, constructs them and keeps the count in front; returns n. Item functions: `0xe318fb`, `0x640361`, `0x63fea0`, `0xddcf39`, `0x6403a0`, `0x64039e`, `0x63fe50`, `0xddcf99` |
| `0x66a350` | ForwardBE79B0 | `0xbe79b0` with its two arguments; returns the first argument it was given |
| `0x6cf7b0` | ConstructorF63488 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf63488`; returns the object |
| `0x6cf800` | ConstructorF63498 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf63498`; returns the object |
| `0x6cf840` | ConstructorF634A8 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf634a8`; returns the object |
| `0x6cf880` | ConstructorF634B8 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf634b8`; returns the object |
| `0x6cf8c0` | ConstructorF634C8 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf634c8`; returns the object |
| `0x6cf900` | ConstructorF634D8 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf634d8`; returns the object |
| `0x6cf940` | ConstructorF634E8 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf634e8`; returns the object |
| `0x6cf980` | ConstructorF634F8 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf634f8`; returns the object |
| `0x6cf9c0` | ConstructorF63508 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf63508`; returns the object |
| `0x6cfa00` | ConstructorF63518 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf63518`; returns the object |
| `0x6cfa40` | ConstructorF63528 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf63528`; returns the object |
| `0x6cfa80` | ConstructorF63538 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf63538`; returns the object |
| `0x6cfac0` | ConstructorF63548 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf63548`; returns the object |
| `0x6cfb00` | ConstructorF63558 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf63558`; returns the object |
| `0x6cfb40` | ConstructorF63568 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf63568`; returns the object |
| `0x6cfb80` | ConstructorF63578 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf63578`; returns the object |
| `0x6cfbc0` | ConstructorF63588 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf63588`; returns the object |
| `0x6cfc00` | ConstructorF63598 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf63598`; returns the object |
| `0x6cfc40` | ConstructorF635A8 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf635a8`; returns the object |
| `0x6cfc80` | ConstructorF635B8 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf635b8`; returns the object |
| `0x6cfcc0` | ConstructorF635C8 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf635c8`; returns the object |
| `0x6cfd00` | ConstructorF635D8 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf635d8`; returns the object |
| `0x6cfd40` | ConstructorF635E8 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf635e8`; returns the object |
| `0x6cfd80` | ConstructorF635F8 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf635f8`; returns the object |
| `0x6d79b0` | ConstructorF64260 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf64260`; returns the object |
| `0x6d7b20` | ConstructorF64308 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf64308`; returns the object |
| `0x6d7c80` | ConstructorF643B0 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf643b0`; returns the object |
| `0x6d7de0` | ConstructorF64458 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf64458`; returns the object |
| `0x6d7f40` | ConstructorF64500 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf64500`; returns the object |
| `0x6d80b0` | ConstructorF645A8 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf645a8`; returns the object |
| `0x6d8220` | ConstructorF64650 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf64650`; returns the object |
| `0x6d8390` | ConstructorF646F8 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf646f8`; returns the object |
| `0x6d8500` | ConstructorF64798 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf64798`; returns the object |
| `0x6d86a0` | ConstructorF648EC | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf648ec`; returns the object |
| `0x6d87c0` | ConstructorF648FC | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf648fc`; returns the object |
| `0x6d8950` | ConstructorF64A4C | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf64a4c`; returns the object |
| `0x6d8a20` | ConstructorF64A5C | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf64a5c`; returns the object |
| `0x6d8c60` | ConstructorF64C44 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf64c44`; returns the object |
| `0x6d8d70` | ConstructorF64C5C | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf64c5c`; returns the object |
| `0x6d8f10` | ConstructorF64D00 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf64d00`; returns the object |
| `0x6d90a0` | ConstructorF64E1C | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf64e1c`; returns the object |
| `0x6d9280` | ConstructorF64ED0 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf64ed0`; returns the object |
| `0x6d9580` | ConstructorF650C4 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf650c4`; returns the object |
| `0x6d9630` | ConstructorF650D4 | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf650d4`; returns the object |
| `0x6dcdf0` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6dd2a0` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6dd5f0` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6dd920` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6ddc50` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6de0e0` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6de3d0` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6df2f0` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6df940` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6e1da0` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6e2da0` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6e3160` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6e4370` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6e4930` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6e4f40` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6e7660` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6e7720` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x6f0ce0` | ConstructorF66EBC | Constructor: builds the base part (`0x6d6180` (RegisteredItem::RegisteredItem)), then sets its own table `0xf66ebc`; returns the object |
| `0x6f19c0` | NameText 0x0 | Makes the given text a new copy of the wide text kept in `0x0` (WideString::FromWide), or the empty text when there is none; returns it |
| `0x72b000` | FreeMember0 | Frees the block kept at +0x0, when there is one, and clears it |
| `0x730cc0` | NamedTextRefF6A874::NamedTextRefF6A874 | Constructor of a text reference found by name (tables `0xf6a86c`, then `0xf6a874`; the name made from C text when asked), like NamedTextRef |
| `0x738a40` | NamedTextRefF6AACC::NamedTextRefF6AACC | Constructor of a text reference found by name (tables `0xf6aaa8`, then `0xf6aacc`; the name made from C text when asked), like NamedTextRef |
| `0x745d80` | FreeMember0 | Frees the block kept at +0x0, when there is one, and clears it |
| `0x746060` | FreeMember0 | Frees the block kept at +0x0, when there is one, and clears it |
| `0x753300` | NamedTextRefF5323C::NamedTextRefF5323C | Constructor of a text reference found by name (tables `0xf5322c`, then `0xf5323c`; the name made from C text when asked), like NamedTextRef |
| `0x755630` | FreeMember0 | Frees the block kept at +0x0, when there is one, and clears it |
| `0x757710` | ConstructorF6C514 | Constructor: builds the base part (`0xbd1550` (TextRef::TextRefEmpty)), then sets its own table `0xf6c514`; returns the object |
| `0x7577a0` | ConstructorF6C51C | Constructor: builds the base part (`0xbd1550` (TextRef::TextRefEmpty)), then sets its own table `0xf6c51c`; returns the object |
| `0x786d40` | ForwardCDED30 | `0xcded30` (SlotPool::Init) with its two arguments; returns the first argument it was given |
| `0x799b60` | FreeMember0 | Frees the block kept at +0x0, when there is one, and clears it |
| `0x79b090` | ForwardCDED30 | `0xcded30` (SlotPool::Init) with its two arguments; returns the first argument it was given |
| `0x7b3ae0` | FreeMember0 | Frees the block kept at +0x0, when there is one, and clears it |
| `0x7b3d10` | FreeMember0 | Frees the block kept at +0x0, when there is one, and clears it |
| `0x7be680` | NamedTextRefF719B4::NamedTextRefF719B4 | Constructor of a text reference found by name (tables `0xf71994`, then `0xf719b4`; the name made from C text when asked), like NamedTextRef |
| `0x7c53b0` | FreeMember0 | Frees the block kept at +0x0, when there is one, and clears it |
| `0x7c86a0` | FreeMember0 | Frees the block kept at +0x0, when there is one, and clears it |
| `0x7dc790` | ObjectArray28::Resize | Makes an array of 28-byte objects hold n new ones: destroys and frees the old ones (the count kept in front), then allocates n, constructs them and keeps the count in front; returns n. Item functions: `0xe5875b`, `0x7dc7d1`, `0x6fd630`, `0xddcf39`, `0x7dc810`, `0x7dc80e`, `0x77a500`, `0xddcf99` |
| `0x7fcaa0` | NamedTextRefF751B0::NamedTextRefF751B0 | Constructor of a text reference found by name (tables `0xf751b8`, then `0xf751b0`; the name made from C text when asked), like NamedTextRef |
| `0x805500` | ForwardBE79B0 | `0xbe79b0` with its two arguments; returns the first argument it was given |
| `0x80f700` | ClearPair | Clears two values |
| `0x80f710` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x816100` | WithReference8155F0 | Calls `0x8155f0` with a temporary reference (table `0xf75b0c`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0x819240` | ForwardCDED30 | `0xcded30` (SlotPool::Init) with its two arguments; returns the first argument it was given |
| `0x81cfc0` | ForwardCDED30 | `0xcded30` (SlotPool::Init) with its two arguments; returns the first argument it was given |
| `0x81e8f0` | ForwardCDED30 | `0xcded30` (SlotPool::Init) with its two arguments; returns the first argument it was given |
| `0x820f70` | WithReference8155F0 | Calls `0x8155f0` with a temporary reference (table `0xf75b0c`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0x83cef0` | ObjectArray52::Resize | Makes an array of 52-byte objects hold n new ones: destroys and frees the old ones (the count kept in front), then allocates n, constructs them and keeps the count in front; returns n. Item functions: `0xe605cb`, `0x83cf31`, `0x83b340`, `0xddcf39`, `0x83cf70`, `0x83cf6e`, `0x83b2e0`, `0xddcf99` |
| `0x841ed0` | ObjectArray28::Resize | Makes an array of 28-byte objects hold n new ones: destroys and frees the old ones (the count kept in front), then allocates n, constructs them and keeps the count in front; returns n. Item functions: `0xe60b8b`, `0x841f11`, `0x41d920`, `0xddcf39`, `0x841f50`, `0x841f4e`, `0x734070`, `0xddcf99` |
| `0x84f550` | ClearPair | Clears two values |
| `0x84f560` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x84f730` | ClearPair | Clears two values |
| `0x84f740` | ClearPair | Clears two values |
| `0x84f750` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x84f810` | ClearPair | Clears two values |
| `0x84f820` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x84f8e0` | ClearPair | Clears two values |
| `0x84f8f0` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x84f9b0` | ClearPair | Clears two values |
| `0x84f9c0` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x84fa80` | ClearPair | Clears two values |
| `0x84fa90` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x84fb50` | ClearPair | Clears two values |
| `0x84fb60` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x84fc20` | ClearPair | Clears two values |
| `0x84fc30` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x84fcf0` | ClearPair | Clears two values |
| `0x84fd00` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x84fdc0` | ClearPair | Clears two values |
| `0x84fdd0` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x84fe90` | ClearPair | Clears two values |
| `0x84fea0` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x84ff60` | ClearPair | Clears two values |
| `0x84ff70` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x863b60` | FreeMember0 | Frees the block kept at +0x0, when there is one, and clears it |
| `0x87bbe0` | ClearPair | Clears two values |
| `0x87bbf0` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x8ae4b0` | FreeMember0 | Frees the block kept at +0x0, when there is one, and clears it |
| `0x8b0ae0` | ForwardCDED30 | `0xcded30` (SlotPool::Init) with its two arguments; returns the first argument it was given |
| `0x8b49f0` | ForwardCDED30 | `0xcded30` (SlotPool::Init) with its two arguments; returns the first argument it was given |
| `0x8b6380` | ForwardCDED30 | `0xcded30` (SlotPool::Init) with its two arguments; returns the first argument it was given |
| `0x8f8cf0` | ConstructorF7CDB8 | Constructor: builds the base part (`0xbd1550` (TextRef::TextRefEmpty)), then sets its own table `0xf7cdb8`; returns the object |
| `0x8fb1a0` | ForwardCDED30 | `0xcded30` (SlotPool::Init) with its two arguments; returns the first argument it was given |
| `0x8ff690` | ClearPair | Clears two values |
| `0x909b40` | ClearPair | Clears two values |
| `0x909b50` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x909e40` | ClearPair | Clears two values |
| `0x909e50` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x90b310` | ConstructorF264E4 | Constructor: builds the base part (`0xbd1550` (TextRef::TextRefEmpty)), then sets its own table `0xf264e4`; returns the object |
| `0x90b390` | ConstructorF26520 | Constructor: builds the base part (`0xbd1550` (TextRef::TextRefEmpty)), then sets its own table `0xf26520`; returns the object |
| `0x969220` | ClearPair | Clears two values |
| `0x969230` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x969500` | ClearPair | Clears two values |
| `0x969510` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x969e10` | ClearPair | Clears two values |
| `0x969e20` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x96a0f0` | ClearPair | Clears two values |
| `0x96a100` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x96da60` | ClearPair | Clears two values |
| `0x96da70` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x96dd40` | ClearPair | Clears two values |
| `0x96dd50` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x96e020` | ClearPair | Clears two values |
| `0x96e030` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x96e300` | ClearPair | Clears two values |
| `0x96e310` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x97b930` | ClearPair | Clears two values |
| `0x97b940` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x97ba00` | ClearPair | Clears two values |
| `0x97ba10` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x97bad0` | ClearPair | Clears two values |
| `0x97bae0` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0x981600` | ForwardBE79B0 | `0xbe79b0` with its two arguments; returns the first argument it was given |
| `0x9835e0` | NamedTextRefF7F764::NamedTextRefF7F764 | Constructor of a text reference found by name (tables `0xf7f744`, then `0xf7f764`; the name made from C text when asked), like NamedTextRef |
| `0x9868d0` | NamedTextRefF7CF80::NamedTextRefF7CF80 | Constructor of a text reference found by name (tables `0xf7cf78`, then `0xf7cf80`; the name made from C text when asked), like NamedTextRef |
| `0x99fff0` | WithReference98A6E0 | Calls `0x98a6e0` with a temporary reference (table `0xf7ff20`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0x9a7ee0` | WithReference98BC00 | Calls `0x98bc00` with a temporary reference (table `0xf7ff18`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0x9c4890` | ForwardCDED30 | `0xcded30` (SlotPool::Init) with its two arguments; returns the first argument it was given |
| `0x9d7a50` | WithReference98A770 | Calls `0x98a770` with a temporary reference (table `0xf7ff1c`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0x9d8170` | ObjectArray52::Resize | Makes an array of 52-byte objects hold n new ones: destroys and frees the old ones (the count kept in front), then allocates n, constructs them and keeps the count in front; returns n. Item functions: `0xe7528b`, `0x9d81b1`, `0x9955d0`, `0xddcf39`, `0x9d81f0`, `0x9d81ee`, `0x9d7050`, `0xddcf99` |
| `0x9e12f0` | FreeMember0 | Frees the block kept at +0x0, when there is one, and clears it |
| `0x9e1310` | FreeMember0 | Frees the block kept at +0x0, when there is one, and clears it |
| `0xa2a360` | NamedTextRefF8926C::NamedTextRefF8926C | Constructor of a text reference found by name (tables `0xf8925c`, then `0xf8926c`; the name made from C text when asked), like NamedTextRef |
| `0xa591e0` | ObjectArray68::Resize | Makes an array of 68-byte objects hold n new ones: destroys and frees the old ones (the count kept in front), then allocates n, constructs them and keeps the count in front; returns n. Item functions: `0xe8228b`, `0xa59221`, `0xa58e00`, `0xddcf39`, `0xa59260`, `0xa5925e`, `0xa58d90`, `0xddcf99` |
| `0xa65670` | NamedTextRefF2951C::NamedTextRefF2951C | Constructor of a text reference found by name (tables `0xf29484`, then `0xf2951c`; the name made from C text when asked), like NamedTextRef |
| `0xa7e700` | WithReference7CBF50 | Calls `0x7cbf50` with a temporary reference (table `0xf72040`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0xa8f540` | ObjectArray28::Resize | Makes an array of 28-byte objects hold n new ones: destroys and frees the old ones (the count kept in front), then allocates n, constructs them and keeps the count in front; returns n. Item functions: `0xe857cb`, `0xa8f581`, `0x471cf0`, `0xddcf39`, `0xa8f5c0`, `0xa8f5be`, `0x4c5f30`, `0xddcf99` |
| `0xaa1000` | ObjectArray28::Resize | Makes an array of 28-byte objects hold n new ones: destroys and frees the old ones (the count kept in front), then allocates n, constructs them and keeps the count in front; returns n. Item functions: `0xe86c2b`, `0xaa1041`, `0xa9a020`, `0xddcf39`, `0xaa1080`, `0xaa107e`, `0xa9ab80`, `0xddcf99` |
| `0xab42d0` | WithReferenceAAF2E0 | Calls `0xaaf2e0` with a temporary reference (table `0xf92a9c`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0xab4380` | WithReferenceAAF2E0 | Calls `0xaaf2e0` with a temporary reference (table `0xf92a9c`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0xab5e10` | ObjectArray28::Resize | Makes an array of 28-byte objects hold n new ones: destroys and frees the old ones (the count kept in front), then allocates n, constructs them and keeps the count in front; returns n. Item functions: `0xe880fb`, `0xab5e51`, `0x7a7090`, `0xddcf39`, `0xab5e90`, `0xab5e8e`, `0xa7ca10`, `0xddcf99` |
| `0xabfec0` | ObjectArray88::Resize | Makes an array of 88-byte objects hold n new ones: destroys and frees the old ones (the count kept in front), then allocates n, constructs them and keeps the count in front; returns n. Item functions: `0xe885ab`, `0xabff01`, `0xabed50`, `0xddcf39`, `0xabff40`, `0xabff3e`, `0xabee60`, `0xddcf99` |
| `0xac2de0` | NamedTextRefF93478::NamedTextRefF93478 | Constructor of a text reference found by name (tables `0xf93470`, then `0xf93478`; the name made from C text when asked), like NamedTextRef |
| `0xad7780` | WithReferenceAD6EF0 | Calls `0xad6ef0` with a temporary reference (table `0xf952e0`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0xadfba0` | WithReference98BC00 | Calls `0x98bc00` with a temporary reference (table `0xf7ff18`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0xae4f40` | ObjectArray28::Resize | Makes an array of 28-byte objects hold n new ones: destroys and frees the old ones (the count kept in front), then allocates n, constructs them and keeps the count in front; returns n. Item functions: `0xe8bf6b`, `0xae4f81`, `0x4785f0`, `0xddcf39`, `0xae4fc0`, `0xae4fbe`, `0xae3290`, `0xddcf99` |
| `0xaf07c0` | WithReferenceAEFF30 | Calls `0xaeff30` with a temporary reference (table `0xf96618`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0xaf2490` | WithReference4AAD80 | Calls `0x4aad80` with a temporary reference (table `0xf30b40`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0xaf5160` | WithReference98BC00 | Calls `0x98bc00` with a temporary reference (table `0xf7ff18`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0xb01930` | ConstructorF29484 | Constructor: builds the base part (`0xbd1550` (TextRef::TextRefEmpty)), then sets its own table `0xf29484`; returns the object |
| `0xb01a20` | ConstructorF2951C | Constructor: builds the base part (`0xbd1550` (TextRef::TextRefEmpty)), then sets its own table `0xf2951c`; returns the object |
| `0xb10990` | ConstructorF97EAC | Constructor: builds the base part (`0xbd1550` (TextRef::TextRefEmpty)), then sets its own table `0xf97eac`; returns the object |
| `0xb10c20` | ConstructorF97EB4 | Constructor: builds the base part (`0xbd1550` (TextRef::TextRefEmpty)), then sets its own table `0xf97eb4`; returns the object |
| `0xb10d80` | ConstructorF97EB4 | Constructor: builds the base part (`0xbd1550` (TextRef::TextRefEmpty)), then sets its own table `0xf97eb4`; returns the object |
| `0xb22f20` | WithReferenceB21200 | Calls `0xb21200` with a temporary reference (table `0xf98758`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0xb306d0` | WithReferenceB2FBC0 | Calls `0xb2fbc0` with a temporary reference (table `0xf98f98`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0xb32a80` | NamedTextRefF990A8::NamedTextRefF990A8 | Constructor of a text reference found by name (tables `0xf990a0`, then `0xf990a8`; the name made from C text when asked), like NamedTextRef |
| `0xb531a0` | ForwardCDED30 | `0xcded30` (SlotPool::Init) with its two arguments; returns the first argument it was given |
| `0xb54a30` | WithReferenceB53610 | Calls `0xb53610` with a temporary reference (table `0xf9a9a0`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0xb7d000` | WithReferenceB24230 | Calls `0xb24230` with a temporary reference (table `0xf975cc`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0xb7d0b0` | WithReferenceB1B930 | Calls `0xb1b930` with a temporary reference (table `0xf98464`, built on the reference base `0xF26AE0`) to the given object, then destroys the reference (`0x40f730`) |
| `0xb86270` | ForwardCDED30 | `0xcded30` (SlotPool::Init) with its two arguments; returns the first argument it was given |
| `0xb8cae0` | ObjectArray28::Resize | Makes an array of 28-byte objects hold n new ones: destroys and frees the old ones (the count kept in front), then allocates n, constructs them and keeps the count in front; returns n. Item functions: `0xe98e9b`, `0xb8cb21`, `0x4aa730`, `0xddcf39`, `0xb8cb60`, `0xb8cb5e`, `0xb897f0`, `0xddcf99` |
| `0xb92910` | NamedTextRefF9C774::NamedTextRefF9C774 | Constructor of a text reference found by name (tables `0xf9c76c`, then `0xf9c774`; the name made from C text when asked), like NamedTextRef |
| `0xb99660` | FreeMember0 | Frees the block kept at +0x0, when there is one, and clears it |
| `0xb99720` | FreeMember0 | Frees the block kept at +0x0, when there is one, and clears it |
| `0xba4f10` | NamedTextRefF9D0EC::NamedTextRefF9D0EC | Constructor of a text reference found by name (tables `0xf9d0e4`, then `0xf9d0ec`; the name made from C text when asked), like NamedTextRef |
| `0xbad310` | ForwardCDED30 | `0xcded30` (SlotPool::Init) with its two arguments; returns the first argument it was given |
| `0xbb0370` | ObjectArray28::Resize | Makes an array of 28-byte objects hold n new ones: destroys and frees the old ones (the count kept in front), then allocates n, constructs them and keeps the count in front; returns n. Item functions: `0xe9c01b`, `0xbb03b1`, `0xbafa20`, `0xddcf39`, `0xbb03f0`, `0xbb03ee`, `0xbb0300`, `0xddcf99` |
| `0xbe3d50` | ConstructorFA5614 | Constructor: builds the base part (`0xc57b00`), then sets its own table `0xfa5614`; returns the object |
| `0xbf9dd0` | ConstructorFA59E8 | Constructor: builds the base part (`0xc31000`), then sets its own table `0xfa59e8`; returns the object |
| `0xc189f0` | ConstructorFAFCD0 | Constructor: builds the base part (`0xc16e90`), then sets its own table `0xfafcd0`; returns the object |
| `0xc46960` | ClearPair | Clears two values |
| `0xc77360` | ClearPair | Clears two values |
| `0xc77390` | ForwardBE79B0 | `0xbe79b0` with its two arguments; returns the first argument it was given |
| `0xcc17f0` | CallCC1D70 | Calls `0xcc1d70` |
| `0xcc2ce0` | CallCC4520 | Calls `0xcc4520` |
| `0xcc3e40` | CallCC45B0 | Calls `0xcc45b0` |
| `0xcc4f10` | CallCC5610 | Calls `0xcc5610` |
| `0xce75e0` | ClearPair | Clears two values |
| `0xce75f0` | Table20::Free | Frees a table of 20-byte entries (count at +4): for each entry the blocks of its pointer array (+4, count +8), that array and its block at +0xC; then the table itself, clearing the pointer |
| `0xf043b0` | CallCC2660 | Calls `0xcc2660` |
| `0xf25630` | CallCC1F00 | Calls `0xcc1f00` |
| `0xf25650` | Call401020 | Calls `0x401020` |
| `0xf25670` | Call401020 | Calls `0x401020` |
| `0xf25690` | Call401020 | Calls `0x401020` |
| `0xf256b0` | CallCC2930 | Calls `0xcc2930` |
| `0xf256d0` | Call401020 | Calls `0x401020` |
| `0xf256f0` | Call401020 | Calls `0x401020` |
| `0xf25710` | CallCC49D0 | Calls `0xcc49d0` |
| `0xf25730` | CallCC4A00 | Calls `0xcc4a00` |
| `0xf25750` | Call401020 | Calls `0x401020` |
| `0xf25770` | Call401020 | Calls `0x401020` |
| `0xf25790` | Call401020 | Calls `0x401020` |
| `0xf257b0` | CallCC56D0 | Calls `0xcc56d0` |
| `0xf257d0` | Call401020 | Calls `0x401020` |
| `0xf257f0` | Call401020 | Calls `0x401020` |
| `0x409db0` | DeleteGlobal 0x10ff9c8 | Destroys (`0x409980` (Destructor)) and frees the object the global `0x10ff9c8` points to, if any, and clears the global |
| `0x409ec0` | DeleteGlobal 0x10ff9c4 | Destroys (`0x409ba0` (Destructor)) and frees the object the global `0x10ff9c4` points to, if any, and clears the global |
| `0x40d980` | DeleteGlobal 0x10ffa14 | Destroys (`0x40c980` (Destructor)) and frees the object the global `0x10ffa14` points to, if any, and clears the global |
| `0x42dde0` | DeleteGlobal 0x1103be0 | Destroys (`0x42d350` (Destructor)) and frees the object the global `0x1103be0` points to, if any, and clears the global |
| `0x436b20` | DeleteMember2E4 | Deletes the object kept at +0x2e4 (its deleting destructor with 1), when there is one, and clears it |
| `0x4371b0` | DeleteMember1B0 | Deletes the object kept at +0x1b0 (its deleting destructor with 1), when there is one, and clears it |
| `0x47eb50` | DeleteGlobal 0x12f018c | Destroys (`0x47e4e0` (Destructor)) and frees the object the global `0x12f018c` points to, if any, and clears the global |
| `0x4965e0` | DeleteGlobal 0x12f4694 | Destroys (`0x495e90` (Destructor)) and frees the object the global `0x12f4694` points to, if any, and clears the global |
| `0x496b70` | DeleteGlobal 0x12f469c | Destroys (`0xc091b0`) and frees the object the global `0x12f469c` points to, if any, and clears the global |
| `0x49abb0` | DeleteGlobal 0x12f46dc | Destroys (`0x49a980` (Destructor)) and frees the object the global `0x12f46dc` points to, if any, and clears the global |
| `0x512480` | DeleteGlobal 0x12f50b4 | Destroys (`0x50cda0`) and frees the object the global `0x12f50b4` points to, if any, and clears the global |
| `0x548f40` | DeleteMember70 | Deletes the object kept at +0x70 (its deleting destructor with 1), when there is one, and clears it |
| `0x5490e0` | DeleteMember70 | Deletes the object kept at +0x70 (its deleting destructor with 1), when there is one, and clears it |
| `0x5908f0` | List::Count | The number of nodes in a linked list (counting until the sentinel) |
| `0x620d90` | DeleteGlobal 0x12f57d0 | Destroys (`0x620bd0`) and frees the object the global `0x12f57d0` points to, if any, and clears the global |
| `0x63a840` | DeleteMemberD8 | Deletes the object kept at +0xd8 (its deleting destructor with 1), when there is one, and clears it |
| `0x6926e0` | DeleteGlobal 0x12f84c8 | Destroys (`0x6914a0` (Destructor)) and frees the object the global `0x12f84c8` points to, if any, and clears the global |
| `0x6d6160` | DeleteMember58 | Deletes the object kept at +0x58 (its deleting destructor with 1), when there is one, and clears it |
| `0x6f0ea0` | DeleteMemberA0 | Deletes the object kept at +0xa0 (its deleting destructor with 1), when there is one, and clears it |
| `0x6f0ec0` | DeleteMemberA4 | Deletes the object kept at +0xa4 (its deleting destructor with 1), when there is one, and clears it |
| `0x6fd3a0` | List::Count | The number of nodes in a linked list (counting until the sentinel) |
| `0x713740` | DeleteMember224 | Deletes the object kept at +0x224 (its deleting destructor with 1), when there is one, and clears it |
| `0x7260e0` | DeleteGlobal 0x1306344 | Destroys (`0x725de0` (Destructor)) and frees the object the global `0x1306344` points to, if any, and clears the global |
| `0x744f00` | DeleteGlobal 0x13064a4 | Destroys (`0x743f60` (Destructor)) and frees the object the global `0x13064a4` points to, if any, and clears the global |
| `0x757b10` | DeleteGlobal 0x1306530 | Destroys (`0x7576d0` (Destructor)) and frees the object the global `0x1306530` points to, if any, and clears the global |
| `0x79fb90` | List::Count | The number of nodes in a linked list (counting until the sentinel) |
| `0x7a96e0` | List::Count | The number of nodes in a linked list (counting until the sentinel) |
| `0x7fa7c0` | DeleteGlobal 0x130abd8 | Destroys (`0x7fa630` (Destructor)) and frees the object the global `0x130abd8` points to, if any, and clears the global |
| `0x8245b0` | DeleteMember140 | Deletes the object kept at +0x140 (its deleting destructor with 1), when there is one, and clears it |
| `0x82e340` | List::Count | The number of nodes in a linked list (counting until the sentinel) |
| `0x8545c0` | DeleteGlobal 0x130bbf8 | Destroys (`0x851a50` (Destructor)) and frees the object the global `0x130bbf8` points to, if any, and clears the global |
| `0x85bce0` | DeleteGlobal 0x130bc98 | Destroys (`0x85abe0` (Destructor)) and frees the object the global `0x130bc98` points to, if any, and clears the global |
| `0x884310` | DeleteGlobal 0x1310f74 | Destroys (`0x883020` (Destructor)) and frees the object the global `0x1310f74` points to, if any, and clears the global |
| `0x8b09b0` | DeleteGlobal 0x1311230 | Destroys (`0x8b0400` (Destructor)) and frees the object the global `0x1311230` points to, if any, and clears the global |
| `0x8b1a00` | List::Count | The number of nodes in a linked list (counting until the sentinel) |
| `0x8f9310` | DeleteGlobal 0x1339fb4 | Destroys (`0x8f8a00` (Destructor)) and frees the object the global `0x1339fb4` points to, if any, and clears the global |
| `0x9655a0` | DeleteGlobal 0x133b4d4 | Destroys (`0x964fe0` (Destructor)) and frees the object the global `0x133b4d4` points to, if any, and clears the global |
| `0x983700` | DeleteGlobal 0x13412dc | Destroys (`0x982dc0` (Destructor)) and frees the object the global `0x13412dc` points to, if any, and clears the global |
| `0x9c6030` | DeleteGlobal 0x13414ec | Destroys (`0x9c5c90` (Destructor)) and frees the object the global `0x13414ec` points to, if any, and clears the global |
| `0x9f55b0` | List::Count | The number of nodes in a linked list (counting until the sentinel) |
| `0xa1d5f0` | List::Count | The number of nodes in a linked list (counting until the sentinel) |
| `0xa1d810` | List::Count | The number of nodes in a linked list (counting until the sentinel) |
| `0xa1da30` | List::Count | The number of nodes in a linked list (counting until the sentinel) |
| `0xa84b50` | DeleteMember2C | Deletes the object kept at +0x2c (its deleting destructor with 1), when there is one, and clears it |
| `0xa8db80` | List::Count | The number of nodes in a linked list (counting until the sentinel) |
| `0xb17b50` | List::Count | The number of nodes in a linked list (counting until the sentinel) |
| `0xbb7cf0` | List::Count | The number of nodes in a linked list (counting until the sentinel) |
| `0xbc0e50` | DeletingDestructorBC0C70 | Deleting destructor: runs `0xbc0c70` and, when bit 0 of the flag is set, frees the object (Memory::FreeCounted); returns it |
| `0xbd6750` | PointerArray::Resize | Resizes an array of 4-byte values ({count, capacity, ..., data}) to the capacity given, copying over those that fit |
| `0xbd67f0` | PointerArray::Resize | Resizes an array of 4-byte values ({count, capacity, ..., data}) to the capacity given, copying over those that fit |
| `0xbd8060` | PointerArray::PushBD7580 | Appends a value to an array (making room with `0xbd7580`) |
| `0xbd8080` | PointerArray::PushBD75E0 | Appends a value to an array (making room with `0xbd75e0`) |
| `0xc0a840` | Parser::FreeIfAny | Gives the block back to the parser (Parser::Free) when there is one |
| `0xc0a8b0` | Parser::FreeIfAny | Gives the block back to the parser (Parser::Free) when there is one |
| `0xc0a970` | Parser::FreeIfAny | Gives the block back to the parser (Parser::Free) when there is one |
| `0xc0aa30` | Parser::FreeIfAny | Gives the block back to the parser (Parser::Free) when there is one |
| `0xc0aab0` | Parser::FreeIfAny | Gives the block back to the parser (Parser::Free) when there is one |
| `0xc0aaf0` | Parser::FreeIfAny | Gives the block back to the parser (Parser::Free) when there is one |
| `0xc0ab30` | Parser::FreeIfAny | Gives the block back to the parser (Parser::Free) when there is one |
| `0xc0ab70` | Parser::FreeIfAny | Gives the block back to the parser (Parser::Free) when there is one |
| `0xc0abb0` | Parser::FreeIfAny | Gives the block back to the parser (Parser::Free) when there is one |
| `0xc0abf0` | Parser::FreeIfAny | Gives the block back to the parser (Parser::Free) when there is one |
| `0xc0ac30` | Parser::FreeIfAny | Gives the block back to the parser (Parser::Free) when there is one |
| `0xc0ac70` | Parser::FreeIfAny | Gives the block back to the parser (Parser::Free) when there is one |
| `0xc0dd30` | ListNode::UnlinkCheckedC0DD70 | When the node is properly linked (its neighbours point back at it), runs `0xc0dd70` and clears its links |
| `0xc0de70` | ListNode::UnlinkCheckedC0DEB0 | When the node is properly linked (its neighbours point back at it), runs `0xc0deb0` and clears its links |
| `0xc0df90` | ListNode::UnlinkCheckedC0DFD0 | When the node is properly linked (its neighbours point back at it), runs `0xc0dfd0` and clears its links |
| `0xc0e340` | ListNode::UnlinkCheckedC0E380 | When the node is properly linked (its neighbours point back at it), runs `0xc0e380` and clears its links |
| `0xc0e650` | ListNode::UnlinkCheckedC0E690 | When the node is properly linked (its neighbours point back at it), runs `0xc0e690` and clears its links |
| `0xc0e9a0` | ListNode::UnlinkCheckedC0E9E0 | When the node is properly linked (its neighbours point back at it), runs `0xc0e9e0` and clears its links |
| `0xc0ed70` | ListNode::UnlinkCheckedC0EDB0 | When the node is properly linked (its neighbours point back at it), runs `0xc0edb0` and clears its links |
| `0xc0f090` | ListNode::UnlinkCheckedC0F0D0 | When the node is properly linked (its neighbours point back at it), runs `0xc0f0d0` and clears its links |
| `0xc0f350` | ListNode::UnlinkCheckedC0F390 | When the node is properly linked (its neighbours point back at it), runs `0xc0f390` and clears its links |
| `0xc0f550` | ListNode::UnlinkCheckedC0F590 | When the node is properly linked (its neighbours point back at it), runs `0xc0f590` and clears its links |
| `0xc0f720` | ListNode::UnlinkCheckedC0F760 | When the node is properly linked (its neighbours point back at it), runs `0xc0f760` and clears its links |
| `0xc1ad10` | PointerArray::Resize | Resizes an array of 4-byte values ({count, capacity, ..., data}) to the capacity given, copying over those that fit |
| `0xc1b040` | PointerArray::PushC1AF80 | Appends a value to an array (making room with `0xc1af80`) |
| `0xc32ee0` | List::Count | The number of nodes in a linked list (counting until the sentinel) |
| `0xc47d30` | PointerArray::Resize | Resizes an array of 4-byte values ({count, capacity, ..., data}) to the capacity given, copying over those that fit |
| `0xc47dd0` | PointerArray::Resize | Resizes an array of 4-byte values ({count, capacity, ..., data}) to the capacity given, copying over those that fit |
| `0xc47e70` | PointerArray::Resize | Resizes an array of 4-byte values ({count, capacity, ..., data}) to the capacity given, copying over those that fit |
| `0xc47f10` | PointerArray::Resize | Resizes an array of 4-byte values ({count, capacity, ..., data}) to the capacity given, copying over those that fit |
| `0xc47fb0` | PointerArray::Resize | Resizes an array of 4-byte values ({count, capacity, ..., data}) to the capacity given, copying over those that fit |
| `0xc48050` | PointerArray::Resize | Resizes an array of 4-byte values ({count, capacity, ..., data}) to the capacity given, copying over those that fit |
| `0xc4cd20` | PointerArray::PushC48BD0 | Appends a value to an array (making room with `0xc48bd0`) |
| `0xc4cd40` | PointerArray::PushC48C30 | Appends a value to an array (making room with `0xc48c30`) |
| `0xc4cd60` | PointerArray::PushC48C90 | Appends a value to an array (making room with `0xc48c90`) |
| `0xc4cd80` | PointerArray::PushC48CF0 | Appends a value to an array (making room with `0xc48cf0`) |
| `0xc4cda0` | PointerArray::PushC48D50 | Appends a value to an array (making room with `0xc48d50`) |
| `0xc4cdc0` | PointerArray::PushC48DB0 | Appends a value to an array (making room with `0xc48db0`) |
| `0xc96da0` | DeletingDestructorC964D0 | Deleting destructor: runs `0xc964d0` and, when bit 0 of the flag is set, frees the object (Memory::FreeCounted); returns it |
| `0xc9bcc0` | PointerArray::Resize | Resizes an array of 4-byte values ({count, capacity, ..., data}) to the capacity given, copying over those that fit |
| `0xc9d0d0` | PointerArray::PushC9BF70 | Appends a value to an array (making room with `0xc9bf70`) |
| `0xcb23d0` | PointerArray::Resize | Resizes an array of 4-byte values ({count, capacity, ..., data}) to the capacity given, copying over those that fit |
| `0xcb2470` | PointerArray::Resize | Resizes an array of 4-byte values ({count, capacity, ..., data}) to the capacity given, copying over those that fit |
| `0xcb2b00` | PointerArray::PushCB28E0 | Appends a value to an array (making room with `0xcb28e0`) |
| `0xcb2b20` | PointerArray::PushCB2940 | Appends a value to an array (making room with `0xcb2940`) |
| `0xcbc5a0` | PointerArray::Resize | Resizes an array of 4-byte values ({count, capacity, ..., data}) to the capacity given, copying over those that fit |
| `0xcbd490` | PointerArray::PushCBCE60 | Appends a value to an array (making room with `0xcbce60`) |
| `0xce92a0` | DeleteGlobal 0x1400b70 | Destroys (`0xce8a10` (Destructor)) and frees the object the global `0x1400b70` points to, if any, and clears the global |
| `0xcee290` | DeleteGlobal 0x1408b80 | Destroys (`0xced240` (Destructor)) and frees the object the global `0x1408b80` points to, if any, and clears the global |
| `0xcefda0` | DeleteGlobal 0x141ab2c | Destroys (`0xcef7e0` (Destructor)) and frees the object the global `0x141ab2c` points to, if any, and clears the global |
| `0x4027f0` | String::MakeEmpty | Makes the given text the shared empty text; returns it |
| `0x41f850` | String::MakeEmpty | Makes the given text the shared empty text; returns it |
| `0x429fb0` | ClearThree | Clears three values (an empty array) |
| `0x439740` | ShareText2E0 | Gives a shared copy of the text kept at +0x2e0 (its use count raised) |
| `0x448170` | String::MakeEmpty | Makes the given text the shared empty text; returns it |
| `0x478360` | FreeTwoBlocks | Frees the blocks kept at +0x8 and +0x0, when there are any, and clears them |
| `0x484cd0` | ClearThree | Clears three values (an empty array) |
| `0x4884b0` | SetTwoVectorsAndValue | Sets two 3-value vectors (+0, +0xC) and a value (+0x18) from the arguments |
| `0x4885f0` | SetTwoVectorsAndValue | Sets two 3-value vectors (+0, +0xC) and a value (+0x18) from the arguments |
| `0x4b5040` | ShareText380 | Gives a shared copy of the text kept at +0x380 (its use count raised) |
| `0x4caab0` | ShareText6C | Gives a shared copy of the text kept at +0x6c (its use count raised) |
| `0x4e63e0` | ShareText138 | Gives a shared copy of the text kept at +0x138 (its use count raised) |
| `0x510500` | ShareTextD4 | Gives a shared copy of the text kept at +0xd4 (its use count raised) |
| `0x510690` | String::MakeEmpty | Makes the given text the shared empty text; returns it |
| `0x530130` | Call4BEE40Then52F780 | `0x4bee40` (Virtual thunk) then `0x52f780` |
| `0x530140` | Call4C10A0Then52F780 | `0x4c10a0` then `0x52f780` |
| `0x53cb80` | ShareText34 | Gives a shared copy of the text kept at +0x34 (its use count raised) |
| `0x53cba0` | ShareText38 | Gives a shared copy of the text kept at +0x38 (its use count raised) |
| `0x552cd0` | Call552BC0Then4D2B20 | `0x552bc0` then `0x4d2b20` |
| `0x591170` | ShareText64 | Gives a shared copy of the text kept at +0x64 (its use count raised) |
| `0x597c10` | Call4C1680Then597030 | `0x4c1680` then `0x597030` |
| `0x598ca0` | Call67B820Then598910 | `0x67b820` then `0x598910` |
| `0x5a0060` | Call59FA20Then59FA80 | `0x59fa20` then `0x59fa80` |
| `0x5ce020` | ShareText6B8 | Gives a shared copy of the text kept at +0x6b8 (its use count raised) |
| `0x68e8b0` | String::MakeEmpty | Makes the given text the shared empty text; returns it |
| `0x691d50` | ClearThree | Clears three values (an empty array) |
| `0x6a0730` | Call6A0560Then6A41C0 | `0x6a0560` then `0x6a41c0` |
| `0x6bcda0` | String::MakeEmpty | Makes the given text the shared empty text; returns it |
| `0x6c4b10` | ShareText7C | Gives a shared copy of the text kept at +0x7c (its use count raised) |
| `0x6c6d20` | Call6C6C10Then6ACE50 | `0x6c6c10` then `0x6ace50` |
| `0x6c9ca0` | Call6C9B80Then6ACE50 | `0x6c9b80` then `0x6ace50` |
| `0x6eb610` | Call6EB470Then6ACE50 | `0x6eb470` then `0x6ace50` |
| `0x7321c0` | ClearThree | Clears three values (an empty array) |
| `0x73ab80` | ClearThree | Clears three values (an empty array) |
| `0x751230` | ClearThree | Clears three values (an empty array) |
| `0x79f8d0` | Call79F810Then7CF900 | `0x79f810` then `0x7cf900` |
| `0x7b2c10` | SetTwoVectorsAndValue | Sets two 3-value vectors (+0, +0xC) and a value (+0x18) from the arguments |
| `0x7b2c80` | SetTwoVectorsAndValue | Sets two 3-value vectors (+0, +0xC) and a value (+0x18) from the arguments |
| `0x7b2dc0` | SetTwoVectorsAndValue | Sets two 3-value vectors (+0, +0xC) and a value (+0x18) from the arguments |
| `0x7b2ec0` | SetTwoVectorsAndValue | Sets two 3-value vectors (+0, +0xC) and a value (+0x18) from the arguments |
| `0x7b2f30` | SetTwoVectorsAndValue | Sets two 3-value vectors (+0, +0xC) and a value (+0x18) from the arguments |
| `0x7ea780` | ShareText1B0 | Gives a shared copy of the text kept at +0x1b0 (its use count raised) |
| `0x7f6050` | Call4D1900Then4D1840 | `0x4d1900` then `0x4d1840` (AttractionPersonInfo::Person) |
| `0x7fba40` | FreeTwoBlocks | Frees the blocks kept at +0x8 and +0x0, when there are any, and clears them |
| `0x7fbd00` | FreeTwoBlocks | Frees the blocks kept at +0x8 and +0x0, when there are any, and clears them |
| `0x801ce0` | FreeTwoBlocks | Frees the blocks kept at +0x8 and +0x0, when there are any, and clears them |
| `0x807c90` | FreeTwoBlocks | Frees the blocks kept at +0x8 and +0x0, when there are any, and clears them |
| `0x81ab10` | ShareText20 | Gives a shared copy of the text kept at +0x20 (its use count raised) |
| `0x81ab30` | ShareText60 | Gives a shared copy of the text kept at +0x60 (its use count raised) |
| `0x851c70` | Release8501C0 | When its slot (+0xc) is not negative, `0x8501c0` with it and +0x0 |
| `0x851e60` | Release8503D0 | When its slot (+0xc) is not negative, `0x8503d0` with it and +0x0 |
| `0x852060` | Release8505E0 | When its slot (+0xc) is not negative, `0x8505e0` with it and +0x0 |
| `0x852260` | Release8507F0 | When its slot (+0xc) is not negative, `0x8507f0` with it and +0x0 |
| `0x852460` | Release850A00 | When its slot (+0xc) is not negative, `0x850a00` with it and +0x0 |
| `0x852630` | Release850C10 | When its slot (+0xc) is not negative, `0x850c10` with it and +0x0 |
| `0x852840` | Release850E20 | When its slot (+0xc) is not negative, `0x850e20` with it and +0x0 |
| `0x852a40` | Release851030 | When its slot (+0xc) is not negative, `0x851030` with it and +0x0 |
| `0x852c40` | Release851240 | When its slot (+0xc) is not negative, `0x851240` with it and +0x0 |
| `0x8530a0` | Release8516A0 | When its slot (+0xc) is not negative, `0x8516a0` with it and +0x0 |
| `0x8532a0` | Release8518B0 | When its slot (+0xc) is not negative, `0x8518b0` with it and +0x0 |
| `0x87a7c0` | ClearThree | Clears three values (an empty array) |
| `0x91ca60` | FreeTwoBlocks | Frees the blocks kept at +0x8 and +0x0, when there are any, and clears them |
| `0x947670` | ClearThree | Clears three values (an empty array) |
| `0x94dbc0` | FreeTwoBlocks | Frees the blocks kept at +0x8 and +0x0, when there are any, and clears them |
| `0x982d50` | FreeTwoBlocks | Frees the blocks kept at +0x8 and +0x0, when there are any, and clears them |
| `0x99cc10` | ShareText18 | Gives a shared copy of the text kept at +0x18 (its use count raised) |
| `0x9c1280` | FreeTwoBlocks | Frees the blocks kept at +0x8 and +0x0, when there are any, and clears them |
| `0x9c6f00` | FreeTwoBlocks | Frees the blocks kept at +0x8 and +0x0, when there are any, and clears them |
| `0x9d1570` | String::MakeEmpty | Makes the given text the shared empty text; returns it |
| `0xa26190` | ShareText7C | Gives a shared copy of the text kept at +0x7c (its use count raised) |
| `0xa39f40` | ShareTextB4 | Gives a shared copy of the text kept at +0xb4 (its use count raised) |
| `0xa39f60` | ShareTextB8 | Gives a shared copy of the text kept at +0xb8 (its use count raised) |
| `0xa4c560` | Call6ACE50ThenA48CA0 | `0x6ace50` then `0xa48ca0` |
| `0xac3e30` | ClearThree | Clears three values (an empty array) |
| `0xb10f80` | ShareText44 | Gives a shared copy of the text kept at +0x44 (its use count raised) |
| `0xb5b7d0` | SetTwoVectorsAndValue | Sets two 3-value vectors (+0, +0xC) and a value (+0x18) from the arguments |
| `0xb5b840` | SetTwoVectorsAndValue | Sets two 3-value vectors (+0, +0xC) and a value (+0x18) from the arguments |
| `0xb6bc00` | SetTwoVectorsAndValue | Sets two 3-value vectors (+0, +0xC) and a value (+0x18) from the arguments |
| `0xb6bc70` | SetTwoVectorsAndValue | Sets two 3-value vectors (+0, +0xC) and a value (+0x18) from the arguments |
| `0xb870a0` | ShareText16C | Gives a shared copy of the text kept at +0x16c (its use count raised) |
| `0xba5e40` | ClearThree | Clears three values (an empty array) |
| `0xbd2110` | FreeList13D0D9C::Take | Takes an item from the free list `0x13d0d9c` (making a new block of items with `0xbd2050` when it is empty), counting it among the used ones and keeping the most ever used; null when none can be made |
| `0xbe8e40` | FreeList13D0EB4::Take | Takes an item from the free list `0x13d0eb4` (making a new block of items with `0xbd2050` when it is empty), counting it among the used ones and keeping the most ever used; null when none can be made |
| `0xbea270` | FreeList13D0F4C::Take | Takes an item from the free list `0x13d0f4c` (making a new block of items with `0xbd2050` when it is empty), counting it among the used ones and keeping the most ever used; null when none can be made |
| `0xbea840` | FreeList13D0EE0::Take | Takes an item from the free list `0x13d0ee0` (making a new block of items with `0xbd2050` when it is empty), counting it among the used ones and keeping the most ever used; null when none can be made |
| `0xbea8a0` | FreeList13D0F28::Take | Takes an item from the free list `0x13d0f28` (making a new block of items with `0xbd2050` when it is empty), counting it among the used ones and keeping the most ever used; null when none can be made |
| `0xbea900` | FreeList13D0F04::Take | Takes an item from the free list `0x13d0f04` (making a new block of items with `0xbd2050` when it is empty), counting it among the used ones and keeping the most ever used; null when none can be made |
| `0xbfde90` | FreeList13D2A0C::Take | Takes an item from the free list `0x13d2a0c` (making a new block of items with `0xbd2050` when it is empty), counting it among the used ones and keeping the most ever used; null when none can be made |
| `0xbfe320` | FreeList13D2A30::Take | Takes an item from the free list `0x13d2a30` (making a new block of items with `0xbd2050` when it is empty), counting it among the used ones and keeping the most ever used; null when none can be made |
| `0xbfe380` | FreeList13D2A78::Take | Takes an item from the free list `0x13d2a78` (making a new block of items with `0xbd2050` when it is empty), counting it among the used ones and keeping the most ever used; null when none can be made |
| `0xbfe3e0` | FreeList13D2A54::Take | Takes an item from the free list `0x13d2a54` (making a new block of items with `0xbd2050` when it is empty), counting it among the used ones and keeping the most ever used; null when none can be made |
| `0xc50920` | FreeList13E6CC0::Take | Takes an item from the free list `0x13e6cc0` (making a new block of items with `0xbd2050` when it is empty), counting it among the used ones and keeping the most ever used; null when none can be made |
| `0xc9bda0` | FreeList13E6DF4::Take | Takes an item from the free list `0x13e6df4` (making a new block of items with `0xbd2050` when it is empty), counting it among the used ones and keeping the most ever used; null when none can be made |
| `0xc9ef10` | FreeList13E6E28::Take | Takes an item from the free list `0x13e6e28` (making a new block of items with `0xbd2050` when it is empty), counting it among the used ones and keeping the most ever used; null when none can be made |
| `0xca40b0` | DeletingDestructorBF4680 | Deleting destructor: runs `0xbf4680` (Memory::FreePlain) and, when bit 0 of the flag is set, frees the object (Memory::FreePlain); returns it |
| `0xca4100` | DeletingDestructorCA38A0 | Deleting destructor: runs `0xca38a0` (ChainMember::~ChainMember) and, when bit 0 of the flag is set, frees the object (Memory::FreePlain); returns it |
| `0xca4120` | DeletingDestructorCA3A20 | Deleting destructor: runs `0xca3a20` (ChainMember::~ChainMember) and, when bit 0 of the flag is set, frees the object (Memory::FreePlain); returns it |
| `0xca4140` | DeletingDestructorCA3AE0 | Deleting destructor: runs `0xca3ae0` (ChainMember::~ChainMember) and, when bit 0 of the flag is set, frees the object (Memory::FreePlain); returns it |
| `0xca4160` | DeletingDestructorCA3BE0 | Deleting destructor: runs `0xca3be0` (ChainMember::~ChainMember) and, when bit 0 of the flag is set, frees the object (Memory::FreePlain); returns it |
| `0xca4180` | DeletingDestructorCA3DA0 | Deleting destructor: runs `0xca3da0` (ChainMember::~ChainMember) and, when bit 0 of the flag is set, frees the object (Memory::FreePlain); returns it |
| `0xca41a0` | DeletingDestructorCA3EA0 | Deleting destructor: runs `0xca3ea0` (ChainMember::~ChainMember) and, when bit 0 of the flag is set, frees the object (Memory::FreePlain); returns it |
| `0xca41c0` | DeletingDestructorBF4680 | Deleting destructor: runs `0xbf4680` (Memory::FreePlain) and, when bit 0 of the flag is set, frees the object (Memory::FreePlain); returns it |
| `0xca4210` | DeletingDestructorBF4680 | Deleting destructor: runs `0xbf4680` (Memory::FreePlain) and, when bit 0 of the flag is set, frees the object (Memory::FreePlain); returns it |
| `0xca4260` | DeletingDestructorBF4680 | Deleting destructor: runs `0xbf4680` (Memory::FreePlain) and, when bit 0 of the flag is set, frees the object (Memory::FreePlain); returns it |
| `0xca42b0` | DeletingDestructorBF4680 | Deleting destructor: runs `0xbf4680` (Memory::FreePlain) and, when bit 0 of the flag is set, frees the object (Memory::FreePlain); returns it |
| `0xca4300` | DeletingDestructorBF4680 | Deleting destructor: runs `0xbf4680` (Memory::FreePlain) and, when bit 0 of the flag is set, frees the object (Memory::FreePlain); returns it |
| `0xca4350` | DeletingDestructorBF4680 | Deleting destructor: runs `0xbf4680` (Memory::FreePlain) and, when bit 0 of the flag is set, frees the object (Memory::FreePlain); returns it |
| `0xcb2a60` | String::MakeEmpty | Makes the given text the shared empty text; returns it |
| `0xcbcfe0` | String::MakeEmpty | Makes the given text the shared empty text; returns it |
| `0xcbd010` | ShareText1C | Gives a shared copy of the text kept at +0x1c (its use count raised) |
| `0xcbd030` | ShareTextC | Gives a shared copy of the text kept at +0xc (its use count raised) |
| `0xcc9640` | String::MakeEmpty | Makes the given text the shared empty text; returns it |
| `0xcdba70` | ClearThree | Clears three values (an empty array) |
| `0x408210` | FreeFourBlocks | Frees the four blocks the object holds (+0, +4, +8, +0xC) |
| `0x40b520` | FreeFourBlocks | Frees the four blocks the object holds (+0, +4, +8, +0xC) |
| `0x423fb0` | Array4::Assign | Makes an array of 4-byte values a copy of another (its old block freed, a new one of the other's count); returns it |
| `0x476830` | List::InsertBefore | Makes a new 12-byte node holding the value given and links it in before the given node; gives the new node |
| `0x47cac0` | List::InsertBefore | Makes a new 12-byte node holding the value given and links it in before the given node; gives the new node |
| `0x49a120` | FreeFourBlocks | Frees the four blocks the object holds (+0, +4, +8, +0xC) |
| `0x4b4a90` | Array4::Assign | Makes an array of 4-byte values a copy of another (its old block freed, a new one of the other's count); returns it |
| `0x4c1e80` | SetA8AndUpdate | Sets the value at +0xa8 and runs `0x4c0c90` |
| `0x59e020` | SetB0AndUpdate | Sets the value at +0xb0 and runs `0x59be00` |
| `0x63ee80` | Array4::Assign | Makes an array of 4-byte values a copy of another (its old block freed, a new one of the other's count); returns it |
| `0x6a60b0` | List::InsertBefore | Makes a new 12-byte node holding the value given and links it in before the given node; gives the new node |
| `0x6d96f0` | Array4::Assign | Makes an array of 4-byte values a copy of another (its old block freed, a new one of the other's count); returns it |
| `0x704c50` | Set1A8AndUpdate | Sets the value at +0x1a8 and runs `0x704a50` |
| `0x704c70` | Set1ACAndUpdate | Sets the value at +0x1ac and runs `0x704a50` |
| `0x704cb0` | Set1B4AndUpdate | Sets the value at +0x1b4 and runs `0x704a50` |
| `0x704cd0` | Set1B8AndUpdate | Sets the value at +0x1b8 and runs `0x704a50` |
| `0x7337d0` | Array4::Assign | Makes an array of 4-byte values a copy of another (its old block freed, a new one of the other's count); returns it |
| `0x733880` | Array4::Assign | Makes an array of 4-byte values a copy of another (its old block freed, a new one of the other's count); returns it |
| `0x7404f0` | ObjectArray28::Construct | Constructor of an array of n 28-byte objects made with new[] (the count kept in front, each constructed) |
| `0x772360` | ObjectArray28::Construct | Constructor of an array of n 28-byte objects made with new[] (the count kept in front, each constructed) |
| `0x7c0a60` | Array::AddOne7C06E0 | Makes room for one more item (growing by 16 with `0x7c06e0` when full) and counts it |
| `0x7c0ab0` | Array::AddOne7C0780 | Makes room for one more item (growing by 16 with `0x7c0780` when full) and counts it |
| `0x7c0b00` | Array::AddOne7C0830 | Makes room for one more item (growing by 16 with `0x7c0830` when full) and counts it |
| `0x80ed30` | PointerArray::Push64 | Appends a value to the array at +0x0 (count +0x64), doubling it (at least 32) when full |
| `0x80ee30` | PointerArray::Push94 | Appends a value to the array at +0x0 (count +0x94), doubling it (at least 32) when full |
| `0x817bd0` | ObjectArray44::Construct | Constructor of an array of n 44-byte objects made with new[] (the count kept in front, each constructed) |
| `0x820460` | ObjectArray28::Construct | Constructor of an array of n 28-byte objects made with new[] (the count kept in front, each constructed) |
| `0x8e7740` | FreeFourBlocks | Frees the four blocks the object holds (+0, +4, +8, +0xC) |
| `0x8e7ad0` | FreeFourBlocks | Frees the four blocks the object holds (+0, +4, +8, +0xC) |
| `0x9cdd40` | FreeFourBlocks | Frees the four blocks the object holds (+0, +4, +8, +0xC) |
| `0x9f9060` | PointerArray::Push74 | Appends a value to the array at +0x0 (count +0x74), doubling it (at least 32) when full |
| `0x9f90f0` | PointerArray::Push80 | Appends a value to the array at +0x0 (count +0x80), doubling it (at least 32) when full |
| `0xa0ee90` | Array::AddOneA0ECB0 | Makes room for one more item (growing by 16 with `0xa0ecb0` when full) and counts it |
| `0xa0eee0` | Array::AddOneA0ED40 | Makes room for one more item (growing by 16 with `0xa0ed40` when full) and counts it |
| `0xa0ef30` | Array::AddOneA0EDF0 | Makes room for one more item (growing by 16 with `0xa0edf0` when full) and counts it |
| `0xa46590` | List::InsertBefore | Makes a new 12-byte node holding the value given and links it in before the given node; gives the new node |
| `0xa5b5e0` | Array4::Assign | Makes an array of 4-byte values a copy of another (its old block freed, a new one of the other's count); returns it |
| `0xa6bcc0` | PointerArray::Push48 | Appends a value to the array at +0x0 (count +0x48), doubling it (at least 32) when full |
| `0xa6bda0` | PointerArray::Push5C | Appends a value to the array at +0x0 (count +0x5c), doubling it (at least 32) when full |
| `0xa6bde0` | PointerArray::Push68 | Appends a value to the array at +0x0 (count +0x68), doubling it (at least 32) when full |
| `0xacd810` | List::InsertBefore | Makes a new 12-byte node holding the value given and links it in before the given node; gives the new node |
| `0xb3a990` | List::InsertBefore | Makes a new 12-byte node holding the value given and links it in before the given node; gives the new node |
| `0xb467a0` | ObjectArray28::Construct | Constructor of an array of n 28-byte objects made with new[] (the count kept in front, each constructed) |
| `0xb75db0` | Array::AddOneB75BA0 | Makes room for one more item (growing by 16 with `0xb75ba0` when full) and counts it |
| `0xb76260` | Array::AddOneB76170 | Makes room for one more item (growing by 16 with `0xb76170` when full) and counts it |
| `0xb83320` | Set54AndUpdate | Sets the value at +0x54 and runs `0xb83140` |
| `0xb83330` | Set58AndUpdate | Sets the value at +0x58 and runs `0xb83140` |
| `0xb83340` | Set5CAndUpdate | Sets the value at +0x5c and runs `0xb83140` |
| `0xb8cb80` | ObjectArray28::Construct | Constructor of an array of n 28-byte objects made with new[] (the count kept in front, each constructed) |
| `0xb8cd40` | ObjectArray28::Construct | Constructor of an array of n 28-byte objects made with new[] (the count kept in front, each constructed) |
| `0xb99500` | Array4::Assign | Makes an array of 4-byte values a copy of another (its old block freed, a new one of the other's count); returns it |
| `0xbbcc40` | List::InsertBefore | Makes a new 12-byte node holding the value given and links it in before the given node; gives the new node |
| `0xbe8fb0` | FreeListItem13D0EB4::Delete | Destroys the item (`0xbe8b10`) and, when bit 0 of the flag is set, puts it back on the free list `0x13d0eb4`; returns it |
| `0xbebac0` | FreeListItem13D0F28::Delete | Destroys the item (`0xbeba10`) and, when bit 0 of the flag is set, puts it back on the free list `0x13d0f28`; returns it |
| `0xbff3b0` | FreeListItem13D2A78::Delete | Destroys the item (`0xbff310`) and, when bit 0 of the flag is set, puts it back on the free list `0x13d2a78`; returns it |
| `0xc0ccf0` | FindByName0 | Finds the entry of the list at +0x0 whose name (+4) matches the given text, ignoring case; null when there is none or no name is given |
| `0xc0cd50` | FindByName0 | Finds the entry of the list at +0x0 whose name (+4) matches the given text, ignoring case; null when there is none or no name is given |
| `0xc0cdb0` | FindByName0 | Finds the entry of the list at +0x0 whose name (+4) matches the given text, ignoring case; null when there is none or no name is given |
| `0xc0ce10` | FindByName0 | Finds the entry of the list at +0x0 whose name (+4) matches the given text, ignoring case; null when there is none or no name is given |
| `0xc0ce70` | FindByName0 | Finds the entry of the list at +0x0 whose name (+4) matches the given text, ignoring case; null when there is none or no name is given |
| `0xc0ced0` | FindByName0 | Finds the entry of the list at +0x0 whose name (+4) matches the given text, ignoring case; null when there is none or no name is given |
| `0xc0cf30` | FindByName0 | Finds the entry of the list at +0x0 whose name (+4) matches the given text, ignoring case; null when there is none or no name is given |
| `0xc0cf90` | FindByName0 | Finds the entry of the list at +0x0 whose name (+4) matches the given text, ignoring case; null when there is none or no name is given |
| `0xc479c0` | Array4::Reset | Empties an array of 4-byte values and gives it room for n (its old block freed) |
| `0xc47a50` | Array4::Reset | Empties an array of 4-byte values and gives it room for n (its old block freed) |
| `0xc47ae0` | Array4::Reset | Empties an array of 4-byte values and gives it room for n (its old block freed) |
| `0xc47b70` | Array4::Reset | Empties an array of 4-byte values and gives it room for n (its old block freed) |
| `0xc47c00` | Array4::Reset | Empties an array of 4-byte values and gives it room for n (its old block freed) |
| `0xc47c90` | Array4::Reset | Empties an array of 4-byte values and gives it room for n (its old block freed) |
| `0xc9be00` | FreeListItem13E6DF4::Delete | Destroys the item (`0xc9bab0`) and, when bit 0 of the flag is set, puts it back on the free list `0x13e6df4`; returns it |
| `0xcb22a0` | Array4::Reset | Empties an array of 4-byte values and gives it room for n (its old block freed) |
| `0xcb2330` | Array4::Reset | Empties an array of 4-byte values and gives it room for n (its old block freed) |
| `0xcc1d70` | DestroyBE86E0 | Runs `0xbe86e0` (Thunk) on the object |
| `0xcc1f00` | DestroyBE88B0 | Runs `0xbe88b0` on the object |
| `0xcc2630` | DestroyCC25D0 | Runs `0xcc25d0` on the object |
| `0xcc4520` | DestroyBE86E0 | Runs `0xbe86e0` (Thunk) on the object |
| `0xcc45b0` | DestroyBE86E0 | Runs `0xbe86e0` (Thunk) on the object |
| `0xcc49d0` | DestroyBE88B0 | Runs `0xbe88b0` on the object |
| `0xcc4a00` | DestroyBE88B0 | Runs `0xbe88b0` on the object |
| `0xcc5610` | DestroyBE86E0 | Runs `0xbe86e0` (Thunk) on the object |
| `0xcc56d0` | DestroyBE88B0 | Runs `0xbe88b0` on the object |
| `0xce9ed0` | FreeFourBlocks | Frees the four blocks the object holds (+0, +4, +8, +0xC) |
| `0xcea2f0` | FreeFourBlocks | Frees the four blocks the object holds (+0, +4, +8, +0xC) |
| `0xcea700` | FreeFourBlocks | Frees the four blocks the object holds (+0, +4, +8, +0xC) |
| `0x4141a0` | FreeThreeBlocks | Frees the blocks kept at +0x14, +0x80, +0x94 |
| `0x423bb0` | Field::struct | Constructor of a saved field of type `struct`: the field base (`0x8088c0` with the name given, the type text and the argument) and its table `0xf27b20` |
| `0x42f6f0` | SlotTable::Free | Frees slot n of a table (32-byte slots at +0x18, clearing its +0x1C), keeping the lowest free slot (+0xC) and moving the end of the used slots (+0x10) back past the free ones |
| `0x455710` | Field::string | Constructor of a saved field of type `string`: the field base (`0x8088c0` with the name given, the type text and the argument) and its table `0xf29780` |
| `0x4883c0` | SlotTable::Free | Frees slot n of a table (32-byte slots at +0x18, clearing its +0x1C), keeping the lowest free slot (+0xC) and moving the end of the used slots (+0x10) back past the free ones |
| `0x4a8bf0` | Field::array | Constructor of a saved field of type `array`: the field base (`0x8088c0` with the name given, the type text and the argument) and its table `0xf30a40` |
| `0x531060` | List8::Add | Adds the value given to the end of the list at +0x8 (a new 12-byte node) |
| `0x59ab30` | List8::Add | Adds the value given to the end of the list at +0x8 (a new 12-byte node) |
| `0x59c780` | ForEach5DEB00 | Runs `0x5deb00` (CableLiftTrain::Setup (2)) on every item, from `0x59b030` and moving on with `0x5d49d0` until there are no more |
| `0x59c7b0` | ForEach5DEE90 | Runs `0x5dee90` on every item, from `0x59b030` and moving on with `0x5d49d0` until there are no more |
| `0x59c7e0` | ForEach5DEFD0 | Runs `0x5defd0` (CableLiftTrain::Setup) on every item, from `0x59b030` and moving on with `0x5d49d0` until there are no more |
| `0x59c810` | ForEach5DF360 | Runs `0x5df360` on every item, from `0x59b030` and moving on with `0x5d49d0` until there are no more |
| `0x59c840` | ForEach5DF4A0 | Runs `0x5df4a0` (ThrillLiftTrain::Setup) on every item, from `0x59b030` and moving on with `0x5d49d0` until there are no more |
| `0x59c870` | ForEach5DFB60 | Runs `0x5dfb60` on every item, from `0x59b030` and moving on with `0x5d49d0` until there are no more |
| `0x5a22b0` | Field::array | Constructor of a saved field of type `array`: the field base (`0x8088c0` with the name given, the type text and the argument) and its table `0xf3de30` |
| `0x5a22e0` | Field::array | Constructor of a saved field of type `array`: the field base (`0x8088c0` with the name given, the type text and the argument) and its table `0xf3de44` |
| `0x5c20a0` | List8::Add | Adds the value given to the end of the list at +0x8 (a new 12-byte node) |
| `0x7b2ac0` | SlotTable::Free | Frees slot n of a table (32-byte slots at +0x18, clearing its +0x1C), keeping the lowest free slot (+0xC) and moving the end of the used slots (+0x10) back past the free ones |
| `0x7fbd70` | SortedMap::SortedMap | Constructor of a sorted map with room for n keys and n values (two blocks), growing by the step given (128 when it is not positive) |
| `0x7fbdf0` | SortedMap::FindValue7FB900 | Looks the key up in the sorted keys (+0x14, `0x7fb900`) and gives the address of its value in the values (+0x0), or null |
| `0x7fbed0` | SortedMap::SortedMap | Constructor of a sorted map with room for n keys and n values (two blocks), growing by the step given (128 when it is not positive) |
| `0x7fd920` | SortedMap::FindValue7FB9C0 | Looks the key up in the sorted keys (+0x14, `0x7fb9c0`) and gives the address of its value in the values (+0x0), or null |
| `0x802840` | SortedMap::SortedMap | Constructor of a sorted map with room for n keys and n values (two blocks), growing by the step given (128 when it is not positive) |
| `0x8028c0` | SortedMap::FindValue801450 | Looks the key up in the sorted keys (+0x14, `0x801450`) and gives the address of its value in the values (+0x0), or null |
| `0x805df0` | SortedMap::SortedMap | Constructor of a sorted map with room for n keys and n values (two blocks), growing by the step given (128 when it is not positive) |
| `0x805e80` | SortedMap::FindValue805C10 | Looks the key up in the sorted keys (+0x14, `0x805c10`) and gives the address of its value in the values (+0x0), or null |
| `0x808510` | SortedMap::SortedMap | Constructor of a sorted map with room for n keys and n values (two blocks), growing by the step given (128 when it is not positive) |
| `0x808590` | SortedMap::FindValue807970 | Looks the key up in the sorted keys (+0x14, `0x807970`) and gives the address of its value in the values (+0x0), or null |
| `0x80b670` | List8::Add | Adds the value given to the end of the list at +0x8 (a new 12-byte node) |
| `0x846410` | List8::Add | Adds the value given to the end of the list at +0x8 (a new 12-byte node) |
| `0x8464a0` | List8::Add | Adds the value given to the end of the list at +0x8 (a new 12-byte node) |
| `0x8e7200` | FreeThreeBlocks | Frees the blocks kept at +0x10, +0x1c, +0x20 |
| `0x8eee50` | FreeThreeBlocks | Frees the blocks kept at +0x10, +0x28, +0x2c |
| `0x982f10` | SortedMap::SortedMap | Constructor of a sorted map with room for n keys and n values (two blocks), growing by the step given (128 when it is not positive) |
| `0xa0bec0` | SlotTable::Free | Frees slot n of a table (32-byte slots at +0x18, clearing its +0x1C), keeping the lowest free slot (+0xC) and moving the end of the used slots (+0x10) back past the free ones |
| `0xb0e270` | Field::pathnodearray | Constructor of a saved field of type `pathnodearray`: the field base (`0x8088c0` with the name given, the type text and the argument) and its table `0xf97b3c` |
| `0xb11c30` | SortedMap::FindValueB117E0 | Looks the key up in the sorted keys (+0x14, `0xb117e0`) and gives the address of its value in the values (+0x0), or null |
| `0xb5b650` | SlotTable::Free | Frees slot n of a table (32-byte slots at +0x18, clearing its +0x1C), keeping the lowest free slot (+0xC) and moving the end of the used slots (+0x10) back past the free ones |
| `0xb6b900` | SlotTable::Free | Frees slot n of a table (32-byte slots at +0x18, clearing its +0x1C), keeping the lowest free slot (+0xC) and moving the end of the used slots (+0x10) back past the free ones |
| `0xbd6510` | FreeThreeBlocks | Frees the blocks kept at +0xc, +0x10, +0x14 |
| `0xbd6620` | Math::NextPrime | The smallest prime number above n (at least 2), found by trial division: the size given to hash tables |
| `0xbdd460` | Math::NextPrime | The smallest prime number above n (at least 2), found by trial division: the size given to hash tables |
| `0xbdd4b0` | Math::NextPrime | The smallest prime number above n (at least 2), found by trial division: the size given to hash tables |
| `0xc47350` | FreeThreeBlocks | Frees the blocks kept at +0xc, +0x10, +0x14 |
| `0xc47630` | FreeThreeBlocks | Frees the blocks kept at +0xc, +0x10, +0x14 |
| `0xc478d0` | Math::NextPrime | The smallest prime number above n (at least 2), found by trial division: the size given to hash tables |
| `0xc47920` | Math::NextPrime | The smallest prime number above n (at least 2), found by trial division: the size given to hash tables |
| `0xcbc4f0` | Math::NextPrime | The smallest prime number above n (at least 2), found by trial division: the size given to hash tables |
| `0xcc47a0` | GetC | Returns the value at +0xC |
| `0xcc5670` | GetC | Returns the value at +0xC |
| `0x42bd60` | Chain504::DeletingDestructor | Deleting destructor of a chain of blocks: deletes the next one (+0x504) the same way first, then frees this one when bit 0 of the flag is set; returns it |
| `0x42bd90` | Chain504::DeletingDestructor | Deleting destructor of a chain of blocks: deletes the next one (+0x504) the same way first, then frees this one when bit 0 of the flag is set; returns it |
| `0x42bdc0` | Chain1704::DeletingDestructor | Deleting destructor of a chain of blocks: deletes the next one (+0x1704) the same way first, then frees this one when bit 0 of the flag is set; returns it |
| `0x42c1e0` | Chain504::Free | Frees the chain of blocks this points to: each block's next (+0x504) deleted first, then the block |
| `0x42c210` | Chain504::Free | Frees the chain of blocks this points to: each block's next (+0x504) deleted first, then the block |
| `0x42c240` | Chain1704::Free | Frees the chain of blocks this points to: each block's next (+0x1704) deleted first, then the block |
| `0x453ed0` | ListMemberF296F8::DeletingDestructor | Deleting destructor of a list member (table `0xf296f8`): leaves its list (stopping the program if the list is found broken) and frees the object when bit 0 of the flag is set |
| `0x4ef1b0` | FreeMember4 | Frees the block kept at +0x4, when there is one, and clears it |
| `0x7c4670` | FreeMember4 | Frees the block kept at +0x4, when there is one, and clears it |
| `0x7c75e0` | FreeMember4 | Frees the block kept at +0x4, when there is one, and clears it |
| `0x7e6b80` | Array::SetCount | Sets an array's count (+8), first making room for it (`0x417C20`) when its capacity (+4) is smaller |
| `0x7f0cd0` | Array::SetCount | Sets an array's count (+8), first making room for it (`0x417C20`) when its capacity (+4) is smaller |
| `0x7faf30` | ListMemberF75138::DeletingDestructor | Deleting destructor of a list member (table `0xf75138`): leaves its list (stopping the program if the list is found broken) and frees the object when bit 0 of the flag is set |
| `0x80ce70` | Array::SetCount | Sets an array's count (+8), first making room for it (`0x417C20`) when its capacity (+4) is smaller |
| `0x84db10` | ChainC04::DeletingDestructor | Deleting destructor of a chain of blocks: deletes the next one (+0xc04) the same way first, then frees this one when bit 0 of the flag is set; returns it |
| `0x84db40` | Chain1004::DeletingDestructor | Deleting destructor of a chain of blocks: deletes the next one (+0x1004) the same way first, then frees this one when bit 0 of the flag is set; returns it |
| `0x84f620` | ChainC04::Free | Frees the chain of blocks this points to: each block's next (+0xc04) deleted first, then the block |
| `0x84f650` | Chain1004::Free | Frees the chain of blocks this points to: each block's next (+0x1004) deleted first, then the block |
| `0x872120` | ListMemberF78B60::DeletingDestructor | Deleting destructor of a list member (table `0xf78b60`): leaves its list (stopping the program if the list is found broken) and frees the object when bit 0 of the flag is set |
| `0x87b650` | Variant::Set1D | Sets a tagged value: its 16-bit type to 0x1d and its value (+4) to the one given |
| `0x8ea2b0` | ListMemberF7CCBC::DeletingDestructor | Deleting destructor of a list member (table `0xf7ccbc`): leaves its list (stopping the program if the list is found broken) and frees the object when bit 0 of the flag is set |
| `0x9408f0` | FreeMember4 | Frees the block kept at +0x4, when there is one, and clears it |
| `0x9491b0` | ListMemberF7E29C::DeletingDestructor | Deleting destructor of a list member (table `0xf7e29c`): leaves its list (stopping the program if the list is found broken) and frees the object when bit 0 of the flag is set |
| `0x96fef0` | Array::SetCount | Sets an array's count (+8), first making room for it (`0x417C20`) when its capacity (+4) is smaller |
| `0xa30820` | FreeMember4 | Frees the block kept at +0x4, when there is one, and clears it |
| `0xb78d20` | Array::SetCount | Sets an array's count (+8), first making room for it (`0x417C20`) when its capacity (+4) is smaller |
| `0xbe8940` | FreeList13D0EB4::Put | Puts an item back on the free list `0x13d0eb4` and counts it |
| `0xbe9910` | FreeList13D0EE0::Put | Puts an item back on the free list `0x13d0ee0` and counts it |
| `0xbe9c70` | FreeList13D0F04::Put | Puts an item back on the free list `0x13d0f04` and counts it |
| `0xbfd630` | FreeList13D2A30::Put | Puts an item back on the free list `0x13d2a30` and counts it |
| `0xbfd990` | FreeList13D2A54::Put | Puts an item back on the free list `0x13d2a54` and counts it |
| `0xc0a270` | Variant::Set21 | Sets a tagged value: its 16-bit type to 0x21 and its value (+4) to the one given |
| `0xc0a290` | Variant::Set1F | Sets a tagged value: its 16-bit type to 0x1f and its value (+4) to the one given |
| `0xc0a2b0` | Variant::Set24 | Sets a tagged value: its 16-bit type to 0x24 and its value (+4) to the one given |
| `0xc0a310` | Variant::Set25 | Sets a tagged value: its 16-bit type to 0x25 and its value (+4) to the one given |
| `0xcc1cd0` | DeletingDestructorCC1B90 | Deleting destructor: runs `0xcc1b90` and, when bit 0 of the flag is set, frees the object; returns it |
| `0xcc4480` | DeletingDestructorCC4140 | Deleting destructor: runs `0xcc4140` and, when bit 0 of the flag is set, frees the object; returns it |
| `0xcc4700` | DeletingDestructorCC2ED0 | Deleting destructor: runs `0xcc2ed0` and, when bit 0 of the flag is set, frees the object; returns it |
| `0xcc5530` | DeletingDestructorCC5240 | Deleting destructor: runs `0xcc5240` and, when bit 0 of the flag is set, frees the object; returns it |
| `0xcc5570` | DeletingDestructorCC57D0 | Deleting destructor: runs `0xcc57d0` and, when bit 0 of the flag is set, frees the object; returns it |
| `0xc07f80` | MainWindow::Create | Creates the game's main window: registers the window class `Main` (with the arrow cursor), sizes it for the client area (AdjustWindowRectEx) and centres it on the desktop, unless a window of that class already exists (FindWindowExA) |
| `0x82a4e0` | SoundGroup::CheckLooping | Checks a group of sounds; when it mixes looping and non-looping sounds, shows a message box `Mixed loop and non-loop sounds` (`It shouldn't happen`) |
| `0x69b690` | TextInput::SetImeOpen | Turns the Windows input method (for Asian text) on or off with ImmSetOpenStatus, reporting `SetOpenStatus(` with the result |
| `0xc56bc0` | Stream::CreateIoEvent | Sets up a file stream's reading: among others creates its event `Stream%02dIoEvent` (CreateEventA, numbered by the stream) |
| `0xbdd210` | HashMap::Find | Looks a key up in a hash map: hashes it (`0xBDBF70`), takes the bucket (hash modulo the bucket count at +4, each bucket a start and end in the table at +0xC), compares the keys there (+0x10, with `0xBDBFD0`) and gives the address of the matching value (+0x14), or null |
| `0xb393e0` | Language::Index | A number for the game's language, by comparing its name in turn with `English`, `Dutch`, `Finnish`, `French`, `Italian`, `German`, `Swedish`, `BrazilianPortuguese`, `Norwegian`, `Danish`, `Spanish`, `Chinese`, `Korean`, `SChinese` and `Japanese` |
| `0x91e8a0` | TerrainFlattenRecord::TerrainFlattenRecord | Constructor of a managed object (class description `0x133A144`) holding a terrain flattening record; on first use registers its saved fields: `OriginalPosLeft`, `OriginalPosRight`, `StartPosLeft`, `StartPosRight`, `HiWaterPos`, `LoWaterPos`, `FlattenTilesArray` (`FlattenTileCol`, `FlattenTileRow`, `HeightArrayItem0`-`3`), `NumFlattenedTiles`, `Flags`, `DepthFactor`, an oriented box (`OBBPos`, `OBBExtents`, `OBBOr` ...) and `ScanLineTilesArray` |
| `0x80a7f0` | ResourceNames::Load | In outline: reads the names of a resource file's entries (`Name`, `RCT3_Name`, `AOP_FSName_`), formatting the file's path with `%s`, through the resource lookups |
| `0x439c90` | Music::FindFolder | In outline: compares names with the `music` folder and builds its path text (with the shared text `0x1106DC4`) |
| `0xc08310` | MainWindow::Run | Runs the game's main window: checks the Windows version (NT 5 or later), shows the window, saves the screen saver and power-off settings and turns the screen saver off (SystemParametersInfoA) while the game runs, then the message loop; an error from the game is shown in a message box `Error from game` |
| `0xc6cb20` | DebugConsole::BuildSubmenu | In outline: builds a submenu (`subm`) of the developer console (DebugConsole::BuildMenu) |
| `0xc17ec0` | Script::MatchTag | In outline: compares tags in a text (with an `ARSE` tag among them), through text substrings and comparisons |
| `0x6b64b0` | PathPlacement::Check | In outline: checks a path placement against the path manager and terrain, giving a failure code (`DFAIL` among the texts) |
| `0xc40480` | OverlayFile::LoadChecked | In outline: opens an overlay file, relocates it and checks its types (OverlayFile::Open, Relocate, CheckTypeIndex), reporting problems and marking `BLOCKED` entries |
| `0xc3ad70` | OverlayFile::ReportBlocked | In outline: builds and reports a message about `BLOCKED` entries |
| `0x498ec0` | ShaderText::Setup | In outline: sets up text drawing with the `SIOpaqueText` shader (looked up by name) |
| `0xdfe7e0` | Thread::WaitWithMessages | Waits on handles while still handling the thread's window messages (WaitForMultipleObjects with a timeout, PeekMessage / DispatchMessage), raising the thread's priority meanwhile; `AMUnblock` is its private message |
| `0xc10d20` | ScannerLexer::GetNextBuffer | The script scanner's yy_get_next_buffer (flex): refills the input buffer, reporting `fatal flex scanner internal error--end of buffer missed` when the buffer's end was passed |
| `0xcbe360` | ShaderPreprocessorLexer::GetNextBuffer | The same flex yy_get_next_buffer for the shader preprocessor (the same fatal error text) |
| `0xcbc8e0` | ShaderPreprocessorLexer::PushState | flex's yy_push_state: grows the start-condition stack by 25 (malloc / realloc), reporting `out of memory expanding start-condition stack` when that fails |
| `0xc0ae30` | ScannerLexer::PushState | The same yy_push_state for the script scanner |
| `0xc09ae0` | Parser::Error | Formats a parser error (vsnprintf) and makes `unexpected T_SYMBOL` messages readable by cutting them and adding the symbol |
| `0xc16180` | Parser::ParseText | Parses a script text: sets the scanner's input to the text given, runs the parser and clears the input afterwards |
| `0xc10e30` | Parser::StartDefault | Starts parsing in the `default` state (looked up by name) and allocates a node (196 bytes, Parser::Allocate) |
| `0xc0f940` | Parser::NodeDefault | Looks up the `default` state and allocates a parser node |
| `0xc12680` | Parser::NodeDefault2 | The same for another node kind (`default` state, Parser::Allocate, a constructor) |
| `0xc12400` | Parser::NodeDefault3 | The same for another node kind |
| `0xc0f850` | Parser::NodeDefault4 | The same for another node kind |
| `0xa3e960` | Log::Error | Writes `Error: ` and the message given to the log (SaveWriter::Write), when there is a log |
| `0xa3e8e0` | Log::Warning | Writes `Warning: ` and the message to the log |
| `0xa3e860` | Log::Trace | Writes `Trace: ` and the message to the log |
| `0xbf9a00` | Error::StatusText | Writes an error's text `Status:0x%08x at %s:%d: %s` into the buffer given (at most its size) |
| `0xc08170` | Application::Ask | Shows a message box titled `Application` (OK, or OK and Cancel when asked) and gives 1 for OK, 2 for Abort, 3 for Retry, 4 for Ignore, 0 otherwise |
| `0xc08110` | MainWindow::Destroy | Destroys the main window and unregisters the `Main` window class |
| `0xdfd8d0` | OleAut::Load | Loads `OleAut32.dll` the first time it is needed |
| `0xc56b20` | Stream::IoEventName | Names a stream's event `Stream%02dIoEvent` |
| `0xc435c0` | Resource::NullName | Makes the name `Null` |
| `0xc38520` | Debug::FilesAndLines | The `FilesAndLines` setting text |
| `0xbe4940` | FileSystem::Zip | Looks up the `ZipFS` file system by name |
| `0x805cd0` | PersistenceInfo::Register | Registers the `PersistenceInfo` class name and keeps it |
| `0xc55800` | Resource::Proxy | Looks up the `PROXY` resource type by name |
| `0x7feac0` | Characters::Load | In outline: loads the `Characters` resources (the text set and the shared texts) |
| `0xcb27c0` | Text::AppendMain | Appends `Main` (and others) to a text |
| `0x9eba70` | TestMode::OpenLog | Opens `TestModeLogFile.txt` and writes a start line with the time in milliseconds (Timer::Milliseconds) |
| `0xc3b860` | ResourceFile::ReadBlocked | Reads a 4-byte value from a resource file (swapping its bytes when the file needs it), for its `BLOCKED` entries |
| `0x423b70` | Field::float32 | Constructor of a saved field of type `float32` (4 bytes; the field base `0x8088C0` and its table) |
| `0x44cac0` | Field::bool | Constructor of a saved field of type `bool` (1 byte) |
| `0x50c680` | Field::managedobjectptr | Constructor of a saved field of type `managedobjectptr` (8 bytes) |
| `0x4e6050` | Field::managedobjectptr2 | Another `managedobjectptr` field constructor (8 bytes) |
| `0xc90830` | Colour::DefaultFormat | Sets up a colour value's format (3) |
| `0x98ac70` | HandleRef::FromRef | Makes the given reference (the reference base `0xF26AE0`) refer to the same handle as another reference (its +4): counts a use of it and joins its list first |
| `0x5d10f0` | FlagSlots::Set | Stores a value in the slot of the flag given: 1, 2, 4, ... 0x200 go to slots 0 to 9; other values are ignored |
| `0x84d2d0` | Entry::Compare | A sort comparison: by kind (+8) first, then within the kind by the first value (kind 0), the first value and the +0x18 flag (kind 1) or the second value (kind 2), then by +0xC, +0x14 (reversed) and +0x10; gives -1, 0 or 1 |
| `0x6b9a90` | GlobalTable::ByIndex | Gives one of 25 fixed game objects by number (0 to 24, e.g. 0 is `0x1301AF8`), or null for any other number |
| `0xbfd270` | BlockPool::Free | Frees a block of a pool: merges it with a free block before and after it and returns the merged ones' records to a free list (`0x13D2A0C`, count `0x13D2A14`) |
| `0xbe9590` | BlockPool::Free2 | The same for another pool (free list `0x13D0F4C`, count `0x13D0F54`) |
| `0xc477d0` | KeyMap::Find | Finds the value stored for a key of 48 bytes (12 values xored together as the hash, buckets at +0xC, entries of 0x30 bytes at +0x10) |
| `0x966da0` | List::Merge | Merges two sorted lists into the one given, using the comparison passed in |
| `0x85aad0` | Arrays::Compact | Removes the empty entries of a set of parallel arrays, keeping the order, and renumbers the references to them (through the table `0x130CDB0`) |
| `0xceb030` | Record::Copy | Copies a record (the first byte and the values from +0x10 on) |
| `0xbf4910` | GroupedList::MoveAcross | Moves an object across the boundary of its group in its owner's list (+0x94, count +0x8C; six groups whose bounds are 12 bytes apart from +0x4C): it swaps places with the entry at the boundary and the boundary moves by one, into the first part on true, out of it on false |
| `0xbb2220` | Key::MatchesPattern | Whether a key matches a pattern of 16-byte parts (at 0, +0x10, +0x2C, ...): each part must be equal, unless the pattern's part is the wildcard `0xF9EBB4` |
| `0x87b700` | Record::Copy2 | Copies a record (the first byte and the values from +4 on, skipping +0x18 and +0x1C) |
| `0x9f89b0` | Record::Copy3 | Copies a record value by value |
| `0xe03cd0` | SortedTable::Find | Binary search of the table `0x141DCE4` (32-byte entries, count `0x141DCDC`) for a six-part key; gives whether it was found and its place (or where it would go) |
| `0xb786b0` | Entries::UpdateMatching | Copies the record given over each entry of a list (0x70 bytes each) whose +0xC and +0x10 match its own |
| `0xaadfc0` | Slots::Clear | Sets every slot value from +0x58 to +0xE4 to -1 (none) |
| `0xc52060` | Span::Clip | Fills a 32-byte span record for the part of this range (start +0x14 minus +0x10, length +0xC) that lies inside the range given |
| `0x466970` | PairIds::Get | The number of an unordered pair of numbers: looked up in `0x111EFE0` (by the smaller one), or a new record is added to `0x1106E90` (count `0x12DFFE8`) and its number given |
| `0xe02530` | Keys::FindFrom | The key (16 bytes each, time first; at +0x10, count +0xC) for a time, starting from a guess: checks the next two, then searches between them and the end |
| `0xe02470` | Keys::FindFrom2 | The same key search for another key list (at +8, count +4) |
| `0xc5e9f0` | Object::SetHeld | Switches a mode (+0x110): switched on, it saves the flags (+0x1C), sets bits 0x180 and saves and clears +0xD8 (also in +0x104), +0xF0 and +0xF4; switched off, it puts them back |
| `0xcd60e0` | WideString::CompareRaw | Compares a wide string with a plain wide text, character by character: -1, 0 or 1 (an empty or missing one counts as the smallest) |
| `0xca45c0` | PairMap::Find | The value stored for a pair of numbers in a sorted table (12-byte entries at +0x20, count +0x14), found by halving steps; -1 when it is not there |
| `0x90c710` | Timers::Advance | Adds time to each timer (32-byte entries at +0x40, count +0x48) and removes the ones that have reached their end (+0x10) by moving the last one into their place; then goes over a second list (+0x54) |
| `0xc197d0` | Bits::PackScaled | Packs the whole part of a number times a constant into three bit fields (10, 9 and 9 bits) of one value |
| `0xb34ef0` | Progress::Advance | Adds time to a progress value (+0x134, up to 1) and a share of the change times a count (+0xD8) to a second one (+0x150, up to 1); gives whether the first is past a threshold |
| `0xa6bfd0` | Lists::Remove | Removes an item from the first list (+0x6C, count +0x74) where it is found, keeping the order, or else from the second list (8-byte entries at +0x78, count +0x80) |
| `0x9c60c0` | Record::CopyResetLimits | Copies a record, setting three of its values (+0x3C, +0x4C, +0x5C) to the largest float instead |
| `0x970c80` | Description::Differs | Whether two descriptions differ, compared by their kind (1 or 2 compare more of their values); 1 also when the first is missing |
| `0x59f270` | Object59F270::Object59F270 | Constructor (table `0xF3DC68`): copies a 4x4 matrix's rows, three values, a point and a flag, and starts an empty list |
| `0x46c6c0` | Entries::CountNegative | How many entries (0x50 bytes each at +0x6C, count +0x70) have a value below 0 at +0x40 |
| `0xbe63b0` | VertexData::SetElement | Copies element n of a vertex layout (its size from the type table `0x10F2BA8`) into the record at its offset |
| `0xbfcb60` | Object0FA5B08::Object0FA5B08 | Constructor (tables `0xFA5AA8`, then `0xFA5B08`): joins its owner's list (+0x46C, +0x14) and sets its mode (+0x7C) to 6 or 7 (by the setting `0x13D3175`), with 8 added when flagged |
| `0xa517c0` | Slots::Reserve | Works out how many of the first slots (8 bytes each at +0xC, count +0x10) to keep: 6, or 3 when +0x14 is set, plus the slots of kinds 3, 4 and 5, kept below the count; then marks the empty ones (kind 0) among them as kind 1 |
| `0x9f8430` | Key::FromSmallList | Sorts a short list of numbers and packs each plus one into the 16-bit parts of a 64-bit key (up to four) |
| `0x4ff3a0` | Value::Band | Which of seven bands (0 to 6) a value falls in, by six fixed limits (`0xF36898` up to `0xF28218`) |
| `0xcc4aa0` | Key::CompareMasked | Compares two three-part keys, only on the parts both masks have (bits 1, 2, 4): -1, 0 or 1 |
| `0xbb5880` | Condition::Test | Tests a value against a condition's number (+8) by its operator (+0: 2 equal, 3 not equal, 4 less, 5 at most, 6 more, 7 at least) |
| `0xcd5750` | String::CompareRaw | Compares two plain texts character by character: -1, 0 or 1 (an empty or missing one counts as the smallest) |
| `0xa3fb60` | Table::Map16 | Maps a number (0 to 7) through one of two fixed eight-entry tables, picked by the flag |
| `0xce6f80` | Record::Copy4 | Copies a record (the first byte and the values from +0x10 on) |
| `0x9c6170` | Record::Copy5 | Copies a record value by value |
| `0x40ba70` | Record::Copy6 | Copies a record (the first byte and the values from +4 on) |
| `0xb07010` | Object::GetMatrix | Copies the object's 4x4 matrix (+0xD0) |
| `0xcc75a0` | Nodes::MarkChain | Marks node n and the nodes it leads to (0x60 bytes each, next at +0x50) with bit 0x800, stopping at one already marked; with the flag clear it first checks node n can start a chain |
| `0x5bc310` | Record::Clear | Sets the 40 values of a record to 0 |
| `0xbd28a0` | Node::Unlink | Takes a node out of its list, clears its totals (+0x30 to +0x40) and does the same for each of its children |
| `0x413e40` | VertexLayout::Add | Adds an element to a vertex layout (up to 16, count +0x20) and grows its size (+0x24) by the element's size (from the type table `0x10F2BA8`) |
| `0xcb78e0` | Buffer::LayOut | Lays out a buffer's parts one after another, each start rounded up to the alignment (+4), from the start given by its owner; counts the parts of kinds 1 and 2 |
| `0xbcedc0` | WideString::Find | The place of a wide text inside a wide string, searching from the place given, or -1 |
| `0xa5fd90` | Speed::Scale | Applies a change to a speed factor (+0x20): times it when the factor is above 1, added otherwise; kept between 1 and 1000, ignored when the change is near 1 |
| `0x87a6d0` | SortedPointers::Find | Binary search of a sorted list of objects for one matching the key's +4 and +0xC values: gives whether it was found and its place (or where it would go) |
| `0xcea9b0` | IdMap::Find | The value for an id in a pair of sorted arrays (ids at +8, values at +0xC, count +0x10): checks the first and last, then halves; -1 when missing |
| `0xcea5a0` | IdMap::Find2 | The same id lookup (another copy) |
| `0xcea180` | IdMap::Find3 | The same id lookup (another copy) |
| `0x40b7d0` | IdMap::Find4 | The same id lookup (another copy) |
| `0x5eb1b0` | Object::SetMatrix | Copies a 4x4 matrix into the object (+0x70) |
| `0x42c910` | Buckets::Init | Sets up 64 buckets (0x5C bytes each) with empty lists and no owner (-1), and keeps the value given (+0x1704) |
| `0xbeecb0` | Indices::CopyOffset | Copies 16-bit indices, adding an offset to each when one is given |
| `0xceaa40` | IdMap::Compact | Removes the empty entries of an id map's arrays, keeping the order, and renumbers its index list (through the table `0x1412AF0`) |
| `0xcea630` | IdMap::Compact2 | The same for another id map (table `0x140EAF0`) |
| `0xcea210` | IdMap::Compact3 | The same for another id map (table `0x140AAF0`) |
| `0xc51870` | Request::Request | Constructor of a request record: joins its owner's list (+0x10), keeps its range (start, start plus length), flags and callback, and takes the next number of `0x13E6C94` (never 0) |
| `0x8e79f0` | IdMap::Compact4 | The same compaction as IdMap::Compact for another id map (table `0x1313CD0`) |
| `0x40b860` | IdMap::Compact5 | The same compaction for another id map (table `0x10FFA20`) |
| `0x84dd70` | Record::Copy7 | Copies a record (the first value and those from +0x10 to +0x5C) |
| `0xbcf600` | String::FindLast | The last place of a text inside a string, searching backwards from the place given (the end when -1), or -1 |
| `0xca6f90` | Slider::SetValue | Sets the bound value (+4) from a fraction: between the minimum (+8) and maximum (+0xC), rounded down to whole steps (+0x10) |
| `0xc1aea0` | Buckets::ClearLists | Empties the list of each entry in bucket n, or in every bucket when n is below 0 |
| `0xc0a060` | Refs20::CopyFrom | Constructor (table `0xFAF9B4`): copies twenty counted references from another one (each counted one more use) and its value at +4 |
| `0xc09fe0` | Refs20::Refs20 | Constructor (table `0xFAF9B4`): keeps the owner given and twenty counted references from it (each counted one more use) |
| `0xbcf460` | String::Find | The place of a text inside a string, searching from the place given, or -1 |
| `0x973140` | Key::Compare2 | A sort comparison of two keys of six values, value by value: -1, 0 or 1 |
| `0x88f750` | Key::Compare3 | The same six-value comparison (another copy) |
| `0x84dcf0` | Record::Copy8 | Copies a record (the first value and those from +0x10 to +0x58) |
| `0x90b410` | Timers::CountDown | Counts each timer (20 bytes each at +4, count +0xC) down by the time times a rate (+0) and removes the ones that run out by moving the last one into their place |
| `0x940ae0` | Pairs::SortOnce | Sorts a list of pairs (8 bytes each) by their second value, once (marked sorted at +0xC) |
| `0xc16660` | Bits::AllSet | Whether every block of a range (the block size at +4) has its bit set in the bit list (+0xC, words +0x10) |
| `0x8fef80` | SortedPointers::FindXY | Binary search of a sorted list of entries (+0x30, count +0x34) for the one at a position (x, y), only among the flagged ones (+8) when asked; gives it and its place |
| `0x58a6c0` | Object3CC90::Object3CC90 | Constructor (table `0xF3CC90`): keeps the owner given and a copy of a 4x4 matrix |
| `0xc71b30` | Slots4::Clear | Clears four slots (0x90 bytes each from +0x10), each marked as 1 with every other value 0 |
| `0xbbd210` | Object::SetMatrixAndMore | Copies a 4x4 matrix into the object (+0x10) and more values after it |
| `0x713310` | Entries::ClearFlags | Clears a run of flag bytes and values from +0x168 on |
| `0xb78630` | Entries::FindMatching | Finds the entry (0x70 bytes each) of a list whose +0xC and +0x10 match the object's, and gives its first value |
| `0x7a2cf0` | SortedTable::FindVersion | Binary search of a sorted table (24-byte entries at +8, count +4) for a 64-bit key (+0x20, +0x24): gives whether it was found and its place |
| `0x5d11c0` | FlagSlots::Get | The value in the slot of the flag given (1, 2, 4, ... 0x200 for slots 0 to 9), or -1 (pairs with FlagSlots::Set) |
| `0xe02670` | Keys::Find | The key (16 bytes each, time first; at +0x10, count +0xC) a time falls in: 0 before the first, the last after the last, otherwise by halving |
| `0xe025f0` | Keys::Find2 | The same key search for another key list (at +8, count +4) |
| `0xc517a0` | Range::Clip | Writes the overlap of two ranges (an empty range at the start when they do not meet) |
| `0xbc0b00` | Lod::LevelFor | The detail level to use for the number given, within the object's levels (+4) less the ones held back (+5), or -1 when there are none |
| `0xc32c60` | NamedList::Find | Finds the entry of a list whose name matches the text given (an empty text when none is given) |
| `0xbce310` | WideString::FromInt | Writes a whole number as wide text in the base given (digits `0-9a-z`), with a minus sign when negative and at most the length given |
| `0x8fde70` | Record::Copy9 | Copies a record (the first byte and some values from +4 on) |
| `0x84dc70` | Record::Copy10 | Copies a record of 19 values |
| `0x84dbf0` | Record::Copy11 | Copies a record of 19 values (another copy) |
| `0xc760f0` | Record::Copy12 | Copies a record of 18 values |
| `0x7fb9c0` | SortedInts::Find | Binary search of a sorted list of numbers: gives whether the number was found and its place (or where it would go) |
| `0xbc2a30` | Pairs::IndexOf | The number of a pair of ids: finds the first id (+0x24), then the second among its partners (+0x28), and gives their slot (from +0x18) |
| `0x7e5050` | Key::Matches | Whether an entry matches a search key, by the entry's kind bits (+0xF): kinds 4 and 6 never; kind 0 by its two bytes and the optional third; otherwise by its whole value |
| `0x9f9130` | SortedTable::Find64 | Binary search of a sorted list (+0x84, count +0x8C) for a 64-bit key: gives whether it was found and its place |
| `0x90c5c0` | SortedTable::Find20 | Binary search of a sorted table of 20-byte entries for a number: gives whether it was found and its place (or where it would go) |
| `0xa90980` | SortedPointers::FindBy8 | Binary search of a sorted list of objects by their +8 value: gives whether it was found and its place |
| `0x8feda0` | SortedPointers::FindXY2 | Binary search of a sorted list of entries (+0x14, count +0x18) for the one at a position (x, y); gives it and its place |
| `0xc22b60` | Pairs::FindSorted | Finds a pair of numbers in a sorted list of pairs by power-of-two steps, or -1 |
| `0x84db70` | Record::Copy13 | Copies a record of 18 values |
| `0xbfef90` | Pools::ResetCounts | Resets the use counts of each pool of a list (refilling the ones used past a share `0xFA5C88`) and of a second list |
| `0xa1ce90` | List::MoveAllTo | Moves every entry of a list into the list at +0x88 |
| `0x799780` | Object::SetMatrix50 | Copies a 4x4 matrix into the object (+0x50) |
| `0xe042b0` | Keys::RemoveAt | Removes entry n of a list of 12-byte entries (+0x10, count +0x14), moving the later ones down and joining its span onto the one before |
| `0xc47750` | KeyMap::Clear | Clears every entry of a key map (48 bytes each at +0x10, with their bucket marks at +0x14) and sets its count (+8) to 0 |
| `0x4453d0` | SortedTable::Find8 | Binary search of a sorted table of 8-byte entries for a number: gives whether it was found and its place (or where it would go) |
| `0x80e3a0` | SortedIndex::LowerBound | The place in a sorted index (+0x44, count +0x48; entries of 0x1C bytes at +0x40, sorted by +8 from high to low) for an entry's +8 value |
| `0xbfb2b0` | Vector::Compare | A sort comparison of two vectors, x then y then z: -1, 0 or 1 |
| `0xae0980` | Shape::SameAs | Whether two shapes are the same: their point counts (+0x50), their values at +0x54 and their points (+0x40) |
| `0xca77b0` | IntSlider::SetValue | Sets the bound whole number (+4) from a fraction between the minimum (+8) and maximum (+0xC) |
| `0xa9f450` | Direction::Dispatch | Calls `0xA9F360` for a direction of 1, 2, 4 or 8; 0 for any other |
| `0xc94f00` | Record::Copy14 | Copies a record of 17 values |
| `0x9e1340` | Record::Copy15 | Copies a record of 17 values (another copy) |
| `0x46c480` | Record::Copy16 | Copies a record of 17 values (another copy) |
| `0x94d9c0` | Record::Copy17 | Copies a record (the first byte and some values from +0x10 on) |
| `0xbb6970` | SortedShorts::Find | Finds the place for a number in a sorted list of 16-bit numbers, starting from a guess by the count times a ratio (+0x2C) |
| `0x8fd720` | SortedPointers::FindInt | Binary search of a sorted list of objects by their first value; gives it and its place (or where it would go) |
| `0x713110` | Description::Compare | Whether two descriptions match: 0 when they differ, 1 when they match, -1 when only their second byte differs |
| `0x4cc880` | Entries::FindByKey | The value of the active entry (0x2C bytes each at +0x94, count +0x9C) whose flags meet the mask and whose four values match, or 0 |
| `0x940a20` | Entries::Remove | Removes the entry (16 bytes each) with the id given by moving the last one into its place; gives whether it was there |
| `0xca7620` | IntSlider::SetValue2 | Sets the bound whole number (+4) from a fraction between the minimum (+8) and maximum (+0xC), signed |
| `0x8ff150` | SortedPointers::FindXY3 | Binary search of a sorted list of entries (+0x3C, count +0x40) for the one at a position (x, y); gives it and its place |
| `0x8386a0` | Grid::Range | The smallest of the cells' first values and the largest of their second values over a whole grid (+0x10 by +0x14, cells of 0x14 bytes at +0x18) |
| `0x80eee0` | List::SortBy28 | Sorts a list of objects (+0x8C, count +0x94) by their +0x28 value (bubble sort) |
| `0x80edc0` | List::SortBy34 | Sorts a list of objects (+0x5C, count +0x64) by their +0x34 value, largest first (bubble sort) |
| `0x49a740` | Heap::Swap | Swaps entry n with the last entry of a heap list and keeps their places (+0xC) in the shared table `0x12F46DC` up to date |
| `0x88fa10` | SortedTable::FindKind | Binary search of a sorted table (12-byte entries at +0x50, count +0x54) for a number of a kind (the byte at +4): gives whether it was found and its place |
| `0xce7260` | SortedPointers::FindFlagged | Binary search of a sorted list (+0x18, count +0x1C) for a number, with or without its flag (+4): gives whether it was found and its place |
| `0xc47490` | Table::Clear | Clears every entry of a table (20 bytes each at +0x10, with their marks at +0x14), its count (+8) and its buckets (+0xC) |
| `0xc166e0` | Bits::SetRange | Sets the bit of every block of a range (the block size at +4) in the bit list (+0xC), counting the ones newly set (+0x14) |
| `0x50a550` | Object::CopySettings | Copies a set of settings (+0xF8 to past +0x128) from another object |
| `0x4d4460` | Matrix::Copy | Copies a 4x4 matrix |
| `0x4277e0` | List678::Remove | Removes an item from the list at +0x678 (count +0x680), keeping the order |
| `0xcc4a30` | Bytes3::Equal | Whether two three-byte values are the same |
| `0xbebee0` | ResourceRefFA57D0::ResourceRefFA57D0 | Constructor (table `0xFA57D0`): keeps its owner, values and kind and joins the owner's list at +0x244 |
| `0xbeb9a0` | ResourceRefFA5798::ResourceRefFA5798 | Constructor (table `0xFA5798`): the same, joining the owner's list at +0x214 |
| `0x4edd00` | FlagSlots::SlotFor | The slot number for a flag (1, 2 and 4 by the value given, then 8 to 0x200 to 14 to 19), or 32 for any other |
| `0x557e70` | Entries::Differs | Whether the entry with the three values given (16 bytes each at +0x94, count +0x98) has a different third value from the one given (or the one given is not 0 when there is none) |
| `0x88f9a0` | SortedTable::LowerBoundKind | The place for a number of a kind (the low bit of each 0x150-byte entry at +0x14, count +0x18) in a sorted table |
| `0xbd5110` | NamedList::Find2 | Finds the entry of a list (+0x204) whose name (+0x24) matches the text given |
| `0x939550` | Matrix::Copy3x3Pos | Copies the three axes and the position of a matrix (leaving their fourth values) |
| `0xcdc420` | SortedPointers::Find64 | Binary search of a sorted list of objects by their two first values: gives whether it was found and its place |
| `0xbd2170` | TypeList::Copy | Copies a list of up to four types, looking up each one's size from its top four bits in `0xFA14D8`, and clears the rest |
| `0xdf88f0` | Math::IntSqrt | The whole-number square root of a number (a first guess by powers of two, then three Newton steps; 0x8000 for very large ones) |
| `0xcc21f0` | ListNode::IsLinked | Whether a list node is properly in a list (its neighbours point back to it) |
| `0xcbc740` | ShaderPreprocessorLexer::SwitchToBuffer | flex's yy_switch_to_buffer: saves the current buffer's place and characters and makes the buffer given current |
| `0xc0ac90` | ScannerLexer::SwitchToBuffer | The same yy_switch_to_buffer for the script scanner |
| `0x827e00` | Entries::EarlierSame | Whether any of the entries before n (0x58 bytes each) has the same id (+0x54) and its two bits set in the object's mask (+0x134) |
| `0xc64ce0` | Items::SetValueAt | Sets the value (+0xD0) of item n of the owner's list (+0xF0, +0xB0) and clears +0x9C |
| `0xa93990` | Object::SetKind | Sets a kind (+0x144) and from it a group (+0x140): 1 for none (-1), 2 for 0 and 1, 4 for 5, 3 for 2 to 9, 0 otherwise |
| `0xaad7f0` | Object::KindCode | A code for the object's kind (+0x120): 0x10 by default, 0x30 for 5, 0x2F for 6, 8 and 9, 0x2E for 7, and for 2 and 3 its value (+0x124) plus 0x10, at most 0x30 |
| `0xbc5eb0` | Pieces::PairAllowed | Whether two pieces make an allowed pair: one of kind 0x4A or 0x4C with the other of kind 0 and type 1, 2, 4 or 5 |
| `0xc9e7f0` | Slots::Reset | Marks every slot of a table empty (-1, 0) and chains the free list through the shared count `0x10F3410`, ending it with -1 |
| `0xbd2780` | TypeLists::Contains | Whether a list holds a type list the same as the one given (same count and the same values) |
| `0x80e200` | SortedIndex::LowerBound2 | The place in a sorted index (+0xC into +8, count +0x10, sorted by +8) for a value |
| `0x772990` | Slots::ValueFor | The value for an id among the object's slots (+0xAC, +0xC0, +0xD4, ... with values at +0x9C, +0xA0, ...), or the default (+0x78) |
| `0x988f60` | Keys::FindTime | The key (32 bytes each at +0x40, count +0x4C) a time falls in, by halving; -1 when there are none |
| `0x8bfef0` | FreeList::Release | Takes entry n out of a chain (16-byte entries, next at +4) and puts it on the free list (+8) |
| `0x9f4da0` | List::MinimumValues | Clears the first entry's value and raises every entry's value below 0.5 to 0.1 |
| `0x7a9150` | Timers::Tick55 | For each object of a list (+0x50C), counts down its 55 timers that are flagged (+0x14 bits, values from +0x18), clearing the flag of each that runs out |
| `0x80e340` | SortedTable::LowerBound1C | The place in a sorted table (0x1C bytes each at +0x40, count +0x48) for a value |
| `0xc505b0` | Slots32::Add | When the table is open (`0x13E6C9C`), puts an entry in the current free slot of 32 (`0x13E6BB8`), counts it and moves on to the next free one (bit 13 of its +0x10); gives the slot, or 0xFF |
| `0x9465e0` | Queue::HasId | Whether a queue (+0x64, from +0x7C to +0x80) holds an active entry with the id given (+0x3C) |
| `0x8e0c30` | Values4::Snap | Snaps each of four values to an earlier one when they are within a tolerance, so near-equal values become equal |
| `0xc44360` | Bytes3List::Equal | Whether a list of three-byte values matches the one given |
| `0xbdd0d0` | PackedRef::Resolve | Turns a packed reference into an address: the top two bits pick one of three bases, else bit 13 picks a block and the low bits are the offset |
| `0xc51260` | Request::CopyFrom | Constructor of a request record from another: copies its values and joins the owner's list (+0x10) |
| `0x9fca90` | ActiveList::Remove | Clears entry n's active flag (0x30 bytes each, +0xC) and takes it out of the active list (+0x64, count +0x6C) |
| `0xc5f510` | Window::AllowedMoves | Takes off the allowed directions or actions that the window's flags (+0x1C) forbid, for windows of style 3 without bit 0x40 |
| `0x9885e0` | Object::SetAxes | Copies a matrix's three axes and position into the object (+0x70), leaving their fourth values |
| `0x988500` | Object::SetAxes2 | The same (another copy) |
| `0x80e1a0` | SortedPointers::LowerBound8 | The place in a sorted list of objects (+8, count +0x10) for a value of their +8 |
| `0xc50140` | Stream::Fail | Records a stream's error and its code (+0xC4, +0xC8) the first time (bit 2 of +0x10), calling the handler `0x13E6BB4` for error 0x1004 |
| `0xaae2d0` | Object::CopyArea | Copies another object's area (+0x148 to +0x154) when it has one (+0x144), otherwise marks it as having none |
| `0x7a6710` | Bits::FromIndex | The bit for a number from 1 to 8 (1 to 0x80), or 0 |
| `0xcbbd70` | ShaderPreprocessorLexer::FlushBuffer | flex's yy_flush_buffer: empties a buffer, and reloads the scanner's state when it is the current one |
| `0xc09740` | ScannerLexer::FlushBuffer | The same yy_flush_buffer for the script scanner |
| `0xca06a0` | List::Clear | Returns every node of a list to the shared free list (`0x13E6E28`, count `0x13E6E30`) and leaves it empty |
| `0xbff8d0` | ResourceRefFA5D28::ResourceRefFA5D28 | Constructor (table `0xFA5D28`): keeps its owner and value and joins the owner's list at +0xC4 |
| `0x965d30` | Lists::Init | Sets up an object's four empty lists and its other values (one to -1) |
| `0x7a8f30` | Objects::HasWithBit | Whether the list at +0x50C holds an object with the same id (+4) and, when a bit is given, that bit set in its flags (+0x14) |
| `0x560460` | Object3AFD0::CopyFrom | Copies an object made of layered parts (tables `0xF3AFD0` and `0xF3AFC8`) and its values from another |
| `0xbe8aa0` | MeshPart::CopyFrom | Constructor (table `0xFA56F8`): copies another mesh part's 19 values and counts one more use of each of its buffers |
| `0xbbcce0` | RefList::Find | Finds the entry of a list with the two values given, counts one more use of it and gives its value (+0xC), or 0 |
| `0x9734a0` | SortedPointers::FindFirst | Binary search of a sorted list of objects (+0xC, count +0x10) by their first value: gives whether it was found and its place |
| `0x84f1a0` | SortedPointers::FindFirst2 | The same search over a list given (unsigned values) |
| `0x966e40` | List::TakeAll | Makes a list empty and moves every node of another list into it |
| `0x9cedb0` | PackedSlots::Set | Sets the values of a slot found by a packed number (block in the top 12 bits, slot of 64 bytes in the low 20) |
| `0xbff2b0` | ResourceRefFA5CF0::ResourceRefFA5CF0 | Constructor (table `0xFA5CF0`): keeps its owner and value and joins the owner's list at +0x94 |
| `0xdfe1a0` | Memory::Move | Copies bytes between places that may overlap (backwards when the target is after the source), like memmove |
| `0xcb2a00` | Pairs::InsertionSort | Sorts a list of 8-byte pairs (+0xC, count +0) by their 16-bit second value (insertion sort) |
| `0x884680` | Values::SumFirst | The sum of the first n values of a list (+0xC) |
| `0x9e2e60` | Lists::ClearMarks | Clears the mark of every entry of two lists (0x30 bytes at +0x20, count +0x28; 0x20 bytes at +0x2C, count +0x34) |
| `0x9883d0` | Object7FC84::Object7FC84 | Constructor (table `0xF7FC84`): keeps its owner, a point, a value and three flags, with size 0xF0 |
| `0xcdb720` | Counter::AddFraction | Sets a rate and adds it (scaled) to a running fraction, keeping the whole steps (+4) and the rest (+8) |
| `0x73ee30` | List::MoveIn | Takes an entry out of its list and puts it into the object's list |
| `0xbe7a90` | Tree::UpdateUp | Copies a node's values (+8, +0xC) up to its parents while it is their first child (+0x10), and then to the parent it is the other child of (+0x1C) |
| `0xbd3240` | Updatable::SetActive2 | Switches an object on or off (+0x118): joins the shared list at `0x13D0D84` +0x5C, or leaves it (`0xBD29E0`) |
| `0x95a510` | Mesh::ResetParts | Clears every part's id (-1) and flag (16 bytes each at +0x14, count +0x1C) and the second list (+0x5C, count +0x60) |
| `0x8bff50` | Ranges::FindFrom | From entry n on (16 bytes each), the first of the kind given whose range (+8 to +0xC) holds the value |
| `0x7a7040` | HandleRef::FromRefBit | Makes a reference (table `0xF26AE0`) to the same handle as another and keeps the bit for the number given |
| `0xc46e20` | Segment3D::Segment3D | Constructor (tables `0xFB7158`, then `0xFB715C`): keeps two values and two points |
| `0xa402b0` | ObjectA402B0::ObjectA402B0 | Constructor: keeps its owner, sets its table to `0xF8B730` and clears its values |
| `0x469c00` | Movies::FindAll | In outline: lists the movie files (`Movie*.avi`) in the game's movie folder and reads the number in each name (the characters after `Movie`), keeping them in order |
| `0x5e92a0` | LayeredObjects::Gather5E92A0 | In outline: goes over a list of objects (+0x130) by their layered kinds (tables `0xF3F714`, `0xF3AF70`, `0xF3F71C`, `0xF3AFD0`, `0xF3AFC0`), copying each kind into its own new array (20, 36 and 32 bytes each) |
| `0xc8abc0` | Device::CheckModeC8ABC0 | In outline: checks one mode or format entry against what the device offers (flags from +0x20E8, the stage at +0x1A00, numbers under 0xAA), case by case; gives 1 when it is usable |
| `0xc55910` | Stream::UpdateC55910 | In outline: a stream's update by its state (+0x1C): starts a read when idle, checks the time (Timer::Milliseconds) and the request's state (5, bit 0x20) and moves on |
| `0x7fdb30` | Loader::Step7FDB30 | In outline: one step of a loader by its stage (+0x10): opens and reads a resource (`0xBDAC20`, `0xBDADC0`, `0xBD9F00`, `0xBD9D20`) and releases what it no longer needs |
| `0x53fbd0` | TextList::Parse | In outline: reads a text list line by line: skips spaces, lines starting `#` are comments and lines starting `*` start a new section, other lines become entries |
| `0xcc3470` | Atlas::PackCC3470 | In outline: lays out rectangles in an image (CRect, 0x55 slots, a 0x100-entry table, `0xCC4D20` for each) |
| `0xa61260` | Sound::UpdateA61260 | In outline: a sound's update by its kind (+0x38 = 1): works out its pitch as a power of two (f2xm1, fscale) and its volume (`0xA60C10`, `0xA5ED70`) and applies them (`0xA609D0`) |
| `0xe030b0` | Keys::RemoveBefore | In outline: drops the keys of a list (times at +8, count +4) older than a time, keeping one before it, and moves the rest down |
| `0xae5400` | Resources::FindByName | In outline: finds a loaded resource whose name matches the one given (WideString::Compare), loading the file when needed (ManagedObject::LoadFile) |
| `0x993fa0` | Event::Update993FA0 | In outline: a timed event's update: checks the date (GameTime::Get) against its start and, for kind 0x16, starts it (`0x4767D0`) |
| `0x49f810` | Table256::Build49F810 | In outline: fills a 256-entry table (`0x49F7B0` first) with rounded values worked out from a curve |
| `0x900da0` | Table::Build900DA0 | In outline: builds a shared lookup (`0x133A044`, up to `0x133A048` entries, at most 0x80) from a list (+0x18) and links each entry's parts |
| `0x4153c0` | Grid::Build4153C0 | In outline: builds a grid of at least 3 by 3 (+0x18, +0x1C): its cells (0x50 bytes each), its links (12 bytes each, `0x414D20`) and their rows |
| `0xba1e90` | PathGrid::BuildBA1E90 | In outline: builds the links of a path grid's cells within a rectangle: lists each cell's parts and makes a 200-byte record for each link |
| `0x41a090` | Regions::Update41A090 | In outline: goes over a list of regions and finds the ones that overlap a rectangle (their bounds at +0x38, +0x3C), releasing and rebuilding them (`0xBE79B0`) |
| `0xa62340` | Sound::UpdateA62340 | In outline: another sound update like Sound::UpdateA61260: for kind 1 (not 2), sets the pitch from its value (`0xA62230`) and keeps a change between -12 and 12 |
| `0xb936d0` | Files::FindB936D0 | In outline: lists files matching a pattern (`0xF27998`, `0xBE5570`) and makes an entry for each one found |
| `0x7bd760` | Characters::FindStubs | In outline: for the `Characters\` folder, lists its `*_Stub.common.ovl` files and loads each one found |
| `0x478a30` | Game::ShutDownPark | In outline: clears up a park's systems: `0x814B00`, the object at `0x12F503C` (`0x988A40`), the one at `0x13417D0`, the clean-up of `0x1341C50`, the crowd sounds (Crowd::Sounds(0)) and more (`0x7B50D0`) |
| `0x58ee80` | Placement::Collect58EE80 | In outline: gathers a list of entries (`0x58EA70`) into a new array and sorts or trims it |
| `0xc56320` | Stream::ProcessC56320 | In outline: runs a stream's update (Stream::UpdateC55910), then for state 7 takes request records from a free list (FreeList13E6CC0::Take, Request::CopyFrom) and starts them, timed (Timer::Milliseconds) |
| `0xbe30e0` | Resources::ReadTableBE30E0 | In outline: reads a table of names and values from a resource into new lists (texts, 12-byte entries) |
| `0x464b30` | LiveContent::Find | In outline: builds the path of the game's `\LiveContent\` folder (from the shared install folder text, `0x1106DC4`) and looks through it |
| `0x742780` | Ids::Allocate742780 | In outline: finds free numbers in a list (+0x40), skipping up to ten taken ones, and grows it by ten when full (`0xBD05B0`) |
| `0xcaea60` | Loader::StartCAEA60 | In outline: when not started (+0xF8) and not paused (`0x13D0DF1`), makes a name text and starts loading it (`0xC3C3E0`) |
| `0x4405f0` | NamedObject::Register4405F0 | In outline: joins a named object to its list (ListMember with `0x10EC9E8`) and, when it has a name, adds it to the name table (Name::Assign on `0x1400128`, NameTable::Set) |
| `0xa52cb0` | Campaign::FindFiles | In outline: lists the files in the campaign folder (Campaign::FolderName, `0xBE5570`) |
| `0xa3efe0` | Text::ReadLinesA3EFE0 | In outline: reads a text resource's lines into a list, skipping empty ones |
| `0xc3bb50` | ResourcePath::SplitC3BB50 | In outline: splits a resource path at its last `:` and looks up the part after it (`0xBCEC90`), for files in state 5 (+0x60) |
| `0x7a2710` | Path::Refresh7A2710 | In outline: goes over the things of a list (`0x1306C2C`) refreshing each (`0x7A06D0`) with the path manager |
| `0x9a50a0` | Text::Split9A50A0 | In outline: splits a text (from `0x13D3058`) into parts (WideString::Substring) into a new list |
| `0xc17070` | Path::SplitRecursive | In outline: splits a path or name at a separator (`0xBE84F0`, String::Substring) and handles the rest the same way (it calls itself) |
| `0x59f520` | Placement::Plan59F520 | In outline: plans a placement inside a timed step (Timer::Milliseconds): releases the old plan (`0xBE79B0`), works out the new one (`0x5BEB30`, `0x5C07F0`) |
| `0xc4ea20` | Strings::BuildC4EA20 | In outline: builds a table of texts (8-byte entries) for up to four kinds by flags (0x10 shifted) and makes empty strings for them |
| `0xa83e10` | Folder::RescanA83E10 | In outline: releases a list of entries and builds a folder path from the install folder text (`0x1106DC4`) to read again |
| `0xca2f10` | Path::MatchCA2F10 | In outline: compares a path with patterns (`0xF27998`, `0xF2E7A4`) and makes the match's parts |
| `0xb0e7c0` | Path::LoadNodesB0E7C0 | In outline: reads saved path nodes from a block (`0x809210`, four bytes at a time, checking the block's size) and makes them (`0xB0E0F0`) |
| `0xcac5a0` | Loader::OpenCAC5A0 | In outline: opens a resource by name (`0xBD05B0`, Object69E with 4), making its name text when none is given |
| `0xcac0d0` | Loader::OpenCAC0D0 | In outline: the same resource open (another copy) |
| `0xcabc00` | Loader::OpenCABC00 | In outline: the same resource open (another copy) |
| `0xbe5b20` | Module::LoadBE5B20 | In outline: runs the module list (ModuleList::Run), stops on an error (Error::ThrowIf) and makes the module's name text |
| `0xbce410` | WideString::FormatBCE410 | In outline: formats a wide text with `%` codes: reads each code's width digits and kind (0 to `l`, ...) and writes its value |
| `0x470560` | Save::Write470560 | In outline: writes a record's values to a save, four bytes at a time (SaveWriter::Write), including its parts when it has them |
| `0x79d920` | SlotPool::Make79D920 | In outline: adds an entry to a slot pool (SlotPool::Add), finds its owner (`0x79D4F0`) and makes a 400-byte record for it when it has parts (+0x50) |
| `0x90cf00` | SortedTable::Insert90CF00 | In outline: adds an entry to a sorted table (+0x80): grows it when full (`0x90C510`), finds its place (SortedTable::Find20) and moves the rest up |
| `0xa02a00` | Objects::CollectA02A00 | In outline: makes a list of the things of a set (+0x64, ObjectArray::SetCapacity, Interface8C::Get) |
| `0xc18df0` | Stream::OpenC18DF0 | In outline: opens a stream for a path (Path::SplitRecursive) and starts its request (`0xC55050`, `0xC501D0`) |
| `0xbe1bd0` | VertexDeclaration::FindBE1BD0 | In outline: the slot for a vertex element in a declaration (from +0x1C1E), or by its kind (+4: 0, ...) |
| `0x46b980` | Movies::FindWmv | In outline: lists the `*.wmv` files of a folder (`0xBE5570`) |
| `0xbda190` | File::ReadFGR | In outline: reads a file that starts with the letters `FGR` and its version bytes (`0xBE5020`) |
| `0x7884b0` | List::Grow7884B0 | In outline: adds an entry to a list (+0x184, +0x188), growing it (`0x787DC0`) when full |
| `0x7fcc30` | Files::Find7FCC30 | In outline: lists files matching a pattern (`0xBE5570`) |
| `0xac3600` | Names::UniqueAC3600 | In outline: makes a unique name for a new thing (`0xAC2BC0`) by adding a number to its text |
| `0x739bc0` | Names::Unique739BC0 | In outline: the same unique name maker for another kind of thing (`0x738540`) |
| `0x731970` | Names::Unique731970 | In outline: the same unique name maker for another kind of thing (`0x730AA0`) |
| `0xc59170` | Stream::ReadLinesC59170 | In outline: reads lines from a stream while not done (+0x14), joining their characters (`0x7FBFF0`, String::Append) |
| `0x4c0850` | Info::Text4C0850 | In outline: makes an info text from its parts (+0x6C), with more than two characters and two parts handled |
| `0x9b0890` | Texts::Load9B0890 | In outline: when not loaded (+0x14) or asked, loads a set of texts (`0x9AA6D0`, from `0x13D3058`) |
| `0xbded20` | Shader::BuildBDED20 | In outline: builds a shader's code buffer (`0xBDD860`, its size +4 plus 0x10) and its parts by kind (0xE handled, `0xBDEB50`) |
| `0xbca360` | Kinds::CodeBCA360 | In outline: a code number for a kind (0 gives 1; others give 0x13, 0x1F, ...) |
| `0xa4b4d0` | Text::FindA4B4D0 | In outline: looks through a text for a marker (`0xF8E578`) and its parts |
| `0x7eb930` | Info::Texts7EB930 | In outline: makes a thing's info texts when it has more than two parts (+0x414) |
| `0x4189a0` | List::Refresh4189A0 | In outline: goes over a list refreshing its entries (`0x417A20`) |
| `0xbf5f20` | Models::UpdateBF5F20 | In outline: goes over a list of models updating the ones not flagged (+0xED) whose value is 0 (`0xBF5D00`) |
| `0x7bcf20` | Loader::Read7BCF20 | In outline: opens and reads a resource by name (`0xBDAC20`) |
| `0x419510` | Regions::Rebuild419510 | In outline: releases and rebuilds a set of regions (`0xBE79B0`) over a list, from their first and last entries |
| `0x497300` | Save::Flush497300 | In outline: when its buffer is full (+0x810 vs +0x814), writes it out (SaveWriter::Finish) in parts of up to 0x400 bytes |
| `0xa38ae0` | List::UpdateA38AE0 | In outline: sets up (`0xA36B40`) and goes over a list updating the entries not flagged (+0x18) that pass a check (`0xA364A0`) |
| `0xcad190` | Loader::OpenCAD190 | In outline: another resource open like Loader::OpenCAC5A0 (Object69E with 0x84) |
| `0xc42c30` | Overlay::ReadDebug | In outline: reads an overlay file's debug data (OverlayFile::ReadDebugData) after setting up (`0xC429E0`) and finishes its buffer (SaveWriter::Finish) |
| `0x7314b0` | Names::Unique7314B0 | In outline: another unique name maker like Names::Unique731970 |
| `0x45fe30` | Folder::Find45FE30 | In outline: looks for a folder by name (`0xBD0960`) and, when missing, builds its path from the install folder text |
| `0xbda5c0` | File::ReadBDA5C0 | In outline: reads a file's header (`0xBE5020`, four bytes) and, for version 5 with data, its parts |
| `0x7ffdf0` | AIDDatabase::ReadEntry | In outline: reads a database entry's fields (`0x7FC020`, `0x7FD440`, AIDDatabaseEntry::Fields, `0x7FBE30`) |
| `0xc0cff0` | Parser::NodeC0CFF0 | In outline: a parser node handler by its kind (+2: 2, ...) |
| `0xcc8810` | Model::PartsCC8810 | In outline: goes over a model's parts (+0x24, +4) handling the ones that are on |
| `0x9138f0` | Array8::Add9138F0 | In outline: adds entries to a list (+0x90), growing it by ten when full (Array8::SetCapacity) |
| `0xba5470` | Names::UniqueBA5470 | In outline: another unique name maker (`0xBA4CF0`) |
| `0xba5a90` | Fences::LoadFolder | In outline: lists and loads the files in the `Enclosures\Fences\` folder (`0xBA4E40`) |
| `0xac3a80` | Pools::LoadFolder | In outline: lists and loads the files in the `Pool\` folder (`0xAC2D10`) |
| `0x73a660` | Queues::LoadFolder | In outline: lists and loads the files in the `Queue\` folder (`0x738690`) |
| `0x731e10` | Paths::LoadFolder | In outline: lists and loads the files in the `Path\` folder (`0x730BF0`) |
| `0xc4da10` | Text::JoinC4DA10 | In outline: joins the parts of a list (+0xC) into one text (String::Append) |
| `0xceab50` | Pairs::InsertCEAB50 | In outline: adds a pair to a sorted list, keeping it in order |
| `0x85a620` | Pairs::Insert85A620 | In outline: the same sorted pair insert (another copy) |
| `0x98b4c0` | Links::Set98B4C0 | In outline: sets an object's link (+0x48) when it changes |
| `0xaa1210` | ObjectList::GrowAA1210 | In outline: grows a list (+0x1C to +0x20) to take in index n (ObjectArray::SetCapacity) and links its new entries |
| `0x744a90` | Loader::Find744A90 | In outline: looks up a resource from an empty name (`0xC16D10`) and uses it when found |
| `0xbe70d0` | VertexLayout::BuildBE70D0 | In outline: builds a vertex layout from a mesh's parts (up to +0x30), adding each element (VertexLayout::Add) after a cleared table of 0xFFFF entries |
| `0xc59ab0` | Text::SplitC59AB0 | In outline: splits a text into two parts (both starting empty) |
| `0x8fc9d0` | Terrain::Buffers8FC9D0 | In outline: makes a terrain patch's records (0xA4 bytes and a 0x12C0-byte buffer) |
| `0xb87790` | Info::TextB87790 | In outline: another info text maker like Info::Text4C0850, from its parts (+0x5C) |
| `0x66c250` | Scenario::Load66C250 | In outline: loads a file (ManagedObject::LoadFile) and checks what it holds by kind |
| `0xcc1420` | Matrices::ResizeCC1420 | In outline: resizes a list of 4x4 matrices (64 bytes each) to n, keeping the old ones |
| `0xce5230` | Sound::UpdateCE5230 | In outline: a sound's update when it has something (+0x24, +0x20): runs Sound::UpdateCE4CC0 and sets its value when it changed (+0x18) |
| `0xc4d000` | File::ReadC4D000 | In outline: makes a 0x1C-byte record and reads its header (`0xBE5020`, four bytes) |
| `0xcb73d0` | ShaderFile::ReadCB73D0 | In outline: reads a shader file's version (two bytes at +0xB6) and checks it against `0x13E6F68` when nothing is loaded yet (+0x6C, +0x70) |
| `0x4a5dc0` | Path::Drive4A5DC0 | In outline: walks a path text to its end and checks for a drive letter (`:`) in its first characters |
| `0xc430f0` | Overlay::ReadDebugC430F0 | In outline: another overlay debug data read like Overlay::ReadDebug |
| `0xc52570` | Stream::FlagsC52570 | In outline: sets a stream's flag bits (`0x13E6C38`) and, unless paused (`0x13E6CA8`), handles the ones with bit 17 and bit 1 or kind 10 |
| `0xc39f50` | Resource::ReplaceC39F50 | In outline: releases a resource and makes a 0x14-byte record for its replacement |
| `0xbe2450` | Resource::FindBE2450 | In outline: goes over a list until it finds a match by kind (2, ...) |
| `0x801fe0` | Values::Get801FE0 | In outline: fills the values given from a record (`0x801E30`) when they are asked for |
| `0x45fad0` | Folder::List45FAD0 | In outline: lists a folder's entries from an empty name |
| `0xbe29e0` | Resource::CollectBE29E0 | In outline: finds a resource (Resource::FindBE2450) and gathers its entries into a new array |
| `0x52ee80` | List::Find52EE80 | In outline: sets up (`0x52EDD0`) and finds an entry in a list |
| `0xc1a4f0` | Com::CreateC1A4F0 | In outline: starts COM (CoInitializeEx) and makes an object (CoCreateInstance with the class at `0x10461E4`) and uses it when made |
| `0xa89bf0` | Info::TextA89BF0 | In outline: another info text maker like Info::Text4C0850, from its parts (+0x68) |
| `0xc72410` | Effect::InitC72410 | In outline: sets up an effect (`0xC07350`), clears its four slots (Slots4::Clear) and handles up to four entries (+0x2F0) |
| `0xc07350` | Effect::BaseC07350 | In outline: sets up an effect's base (`0xBD6930`, `0xBD90D0`) with an empty text |
| `0x493b60` | Points::Fill493B60 | In outline: fills a list of n points (8 bytes each) from their pairs (Call44CC40And489A10) |
| `0xa045c0` | Files::FindDat | In outline: lists the `*.dat` files of a folder |
| `0xdfe200` | Math::ClipDFE200 | In outline: clips ranges given as start and size against limits (negative starts handled) |
| `0xc3d560` | ResourceFile::CheckC3D560 | In outline: for files with fewer than three parts (+0x6C +0x30), loads one when none is loaded (+0x9C, `0xC3B8E0`) |
| `0x476450` | List::Kinds476450 | In outline: goes over a list's entries (+0x84) by kind (1, ...) |
| `0x431810` | Tree::Drain431810 | In outline: takes the last values from a sorted tree while it has some (SortedTree::LastValue) |
| `0xc4a380` | Table::BuildC4A380 | In outline: builds a table of n 16-bit entries and an index |
| `0xc39c30` | Resource::OpenC39C30 | In outline: opens a resource (`0xC38500`, `0xCA55A0` with 3, `0xCA5590`) |
| `0xc17890` | Path::MatchC17890 | In outline: splits a path (`0xBE84F0`, String::Substring) and checks its first part (`0xC167C0`) |
| `0xb73980` | PooledList::MakeB73980 | In outline: makes a list member object (ListMember with `0x10EC9E4`, PooledObject) and sets it up (`0x4B4740`) |
| `0xa53f00` | Files::ListA53F00 | In outline: lists files into a list of names |
| `0x9ad0f0` | Files::List9AD0F0 | In outline: lists files (`0xBE5570`) into a list |
| `0x98da70` | Info::Text98DA70 | In outline: another info text maker like Info::Text4C0850, from its parts (+0xEC) |
| `0xc3ff60` | ResourceFile::GetC3FF60 | In outline: gets entry n of a resource file (in range of +0x2C) when it is open (+0x30), with state 5 handled |
| `0xc17bb0` | Path::MatchC17BB0 | In outline: another path match like Path::MatchC17890 |
| `0xc17580` | Path::MatchC17580 | In outline: another path match like Path::MatchC17890 |
| `0xc4e710` | Strings::MakeC4E710 | In outline: makes n empty strings in a new array |
| `0xae50f0` | List::UpdateAE50F0 | In outline: goes over a list's entries not flagged (+0x18) with a part (+0x14) |
| `0xc002a0` | Input::KeyEventC002A0 | In outline: handles a key event: turns its scan code into a virtual key with the keyboard layout (MapVirtualKeyExA with +0x324) and records it, with a case for held keys |
| `0xcc3160` | Atlas::PackCC3160 | In outline: another rectangle packing like Atlas::PackCC3470 (0x55 slots, a 0x100-entry table) |
| `0x7397d0` | Names::Unique7397D0 | In outline: another unique name maker |
| `0x40d540` | List::Grow40D540 | In outline: grows a list (+8) to take in an entry (`0x40C6A0`) |
| `0xa5f700` | Sound::UpdateA5F700 | In outline: another sound update for kind 1 like Sound::UpdateA61260 |
| `0xae8e00` | Files::FindWjt | In outline: lists the `*.wjt` files of a folder |
| `0xacce20` | Files::FindLas | In outline: lists the `*.las` files of a folder |
| `0x9673f0` | List::SortBy9673F0 | In outline: sorts a list with the comparison given (two items handled directly) |
| `0x444c30` | Text::Font444C30 | In outline: picks a text's font, with a case for Asian languages (Language::IsAsian) |
| `0xc93570` | Renderer::ResetC93570 | In outline: resets a renderer (`0xC056C0`) and sixteen of its slots |
| `0xcb7d30` | Shader::LoadCB7D30 | In outline: loads a shader (`0xC536D0`) and makes its record (kind 0x12) |
| `0x5d1840` | Ride::Range5D1840 | In outline: works out a ride's range by its kind |
| `0xcb1966` | Table::FindCB1966 | In outline: finds an entry matching two values (register-passed table) |
| `0xbf9390` | Model::MaterialBF9390 | In outline: finds a model part's material (`0xBD6E10`) |
| `0xc41060` | Overlay::OpenC41060 | In outline: opens an overlay file (OverlayFile::Open with -1) |
| `0xbf68e0` | Resource::LinkBF68E0 | In outline: links a resource when it has one |
| `0xa048f0` | Folder::PathA048F0 | In outline: builds a folder path from the install folder text (`0x1106DC4`) |
| `0xbfad80` | File::ReadBFAD80 | In outline: reads a file's header (`0xBE5020`, two four-byte values) |
| `0x7b7a80` | List::Copy7B7A80 | In outline: copies a list (+0x4C) into a new array (32 bytes each) |
| `0x9bd9f0` | Files::FindFrw | In outline: lists the `*.frw` files of a folder |
| `0x9a5d00` | Files::FindFwd | In outline: lists the `*.fwd` firework files of a folder |
| `0xca49f0` | Input::KeyCA49F0 | In outline: handles a key code (`0xCA4970`) by its low byte |
| `0x8015a0` | Text::ReadText8015A0 | In outline: reads a saved text from a block (`0x809210`, two bytes at a time), checking the block's size |
| `0x970650` | Table::Pick970650 | In outline: gives one of a set of values by number (0, ...) |
| `0x9a63c0` | Files::FindFwd2 | In outline: another listing of the `*.fwd` firework files |
| `0x5101a0` | Matrices::Resize5101A0 | In outline: resizes a list of 4x4 matrices to n (like Matrices::ResizeCC1420) |
| `0x431b80` | List::Copy431B80 | In outline: copies a list (+0x50) into a new 32-byte-entry array |
| `0xb73e80` | List::CopyB73E80 | In outline: copies a list (+0x64) into a new 32-byte-entry array |
| `0x44aec0` | List::Copy44AEC0 | In outline: copies a list (+0x2C) into a new 32-byte-entry array |
| `0xbd7220` | Text::BufferBD7220 | In outline: makes a 0xA42-entry counted buffer (Memory::AllocCounted) for a text's characters |
| `0xcbc140` | ShaderPreprocessorLexer::Append | In outline: appends a text to the scanner's output (String::Append) until its end |
| `0xcbbe90` | ShaderPreprocessorLexer::Append2 | In outline: the same append (another copy) |
| `0xbe7b00` | List::RemoveBE7B00 | In outline: removes an entry from a list when found (`0xBE7DB0`) |
| `0xc95510` | Effect::SetFlagC95510 | In outline: sets an effect's value from a flag record (its fourth byte) |
| `0xc765a0` | Effect::SetFlagC765A0 | In outline: the same flag setter for another kind of effect |
| `0xc70da0` | Effect::SetFlagC70DA0 | In outline: the same flag setter for another kind of effect |
| `0x455260` | Text::Find455260 | In outline: finds an empty entry in a list of texts |
| `0xc582c0` | Stream::CheckC582C0 | In outline: sets up (`0xC50250`) and checks a stream (`0xCB09C0`) |
| `0x7c14d0` | Save::Write7C14D0 | In outline: writes a record's four values and more to a save (SaveWriter::Write) |
| `0x6f07f0` | Table::Pick6F07F0 | In outline: gives a value by number (0, ...) |
| `0x49c4a0` | Tasks::Pick49C4A0 | In outline: picks a task from a list by the date (GameTime::Get) |
| `0xb12590` | Text::ShareB12590 | In outline: makes a shared text (ShareText44) and a record from it |
| `0x618c70` | LoadingScreen::Make618C70 | In outline: sets up a loading screen with two empty texts and its name (LoadingScreen::Name) |
| `0xcb3080` | Shader::CompileCB3080 | In outline: sets up (`0xC46720`) and runs a shader build step (`0xCF754D`) |
| `0xc733a0` | Device::FindModeC733A0 | In outline: goes over a list for an entry matching a value (+0x10) |
| `0xcaf330` | ResourceFile::NameCAF330 | In outline: makes a resource file's name (`0xC3D0E0`) |
| `0x7fc100` | Text::ReadText7FC100 | In outline: another saved text read from a block (`0x809210`, two bytes at a time) |
| `0x47d160` | NamedObject::Make47D160 | In outline: makes a list member object (ListMember with `0x10EC9E8`) and its part (`0xCE5A80`) |
| `0xb5de60` | List::CopyB5DE60 | In outline: copies a list (+0x24) into a new 32-byte-entry array |
| `0x99fb20` | Folder::LoadOnce99FB20 | In outline: unless loaded (`0x13CE594`), builds a path from the install folder text and loads it |
| `0x99f890` | Folder::LoadOnce99F890 | In outline: the same load-once (`0x13CE4C8`) |
| `0xbe2d30` | Resource::TableBE2D30 | In outline: collects a resource's entries (Resource::CollectBE29E0, `0xBE27B0`) |
| `0xc536d0` | Loader::PumpC536D0 | In outline: a timed loader step (Timer::Milliseconds) |
| `0xbe7e00` | Regions::CaseBE7E00 | In outline: a region step by case (`0xBE7950`) |
| `0xc33730` | Resource::MakeC33730 | In outline: makes a resource part (`0xBD05B0`, `0xC6AC40`) |
| `0xa04010` | Text::JoinA04010 | In outline: joins a text and a shared one (`0xF2A7D4`) |
| `0x687720` | Folder::Find687720 | In outline: finds a folder by name (`0xBD0960`) |
| `0x91c720` | Array::Resize91C720 | In outline: resizes a list of 0x30-byte entries to n |
| `0x424290` | Array::Resize424290 | In outline: resizes a list of 0x24-byte entries to n |
| `0x48ad70` | List::Copy48AD70 | In outline: copies a list into a new 32-byte-entry array |
| `0xa067b0` | Array::MakeA067B0 | In outline: makes a 20-byte-entry array with a count |
| `0x8860f0` | List::Index8860F0 | In outline: makes an index for a list (+0xD4 +8) |
| `0x840900` | Scenery::Make840900 | In outline: makes a scenery list member (ListMember with `0x10EC9F0`) with a path table (`0x488290`) |
| `0xc55580` | Stream::SetPausedC55580 | In outline: pauses or resumes the stream system (`0x13E6CF8`) |
| `0xcea730` | IdMap::Remove | In outline: removes the entry with the id given from an id map (the partner of IdMap::Find) and gives its place |
| `0xcea320` | IdMap::Remove2 | In outline: the same id map remove (another copy) |
| `0xce9f00` | IdMap::Remove3 | In outline: the same id map remove (another copy) |
| `0x9cdd70` | IdMap::Remove4 | In outline: the same id map remove (another copy) |
| `0x8e7b00` | IdMap::Remove5 | In outline: the same id map remove (another copy) |
| `0x8e7770` | IdMap::Remove6 | In outline: the same id map remove (another copy) |
| `0x49a150` | IdMap::Remove7 | In outline: the same id map remove (another copy) |
| `0x40b550` | IdMap::Remove8 | In outline: the same id map remove (another copy) |
| `0x408240` | IdMap::Remove9 | In outline: the same id map remove (another copy) |
| `0x881900` | Text::Make881900 | In outline: makes a text from an empty one |
| `0xc54dc0` | Stream::NameC54DC0 | In outline: makes a stream's name (`0xCA24F0`) |
| `0x4cbaa0` | Name::Find4CBAA0 | In outline: finds a name in a list ignoring case (WideString::ToLower, WideString::Compare) |
| `0xc4d7a0` | Text::FindC4D7A0 | In outline: finds a text entry (`0xC47500`) |
| `0xc31770` | Mesh::SetFormatC31770 | In outline: sets a mesh buffer's format by its kind (0x40, ...) |
| `0x8d3140` | Terrain::TimedUpdate | In outline: a timed terrain update inside a critical section |
| `0xbe5fd0` | Module::LoadBE5FD0 | In outline: another module load like Module::LoadBE5B20 (ModuleList::Run, Error::ThrowIf) |
| `0xbe90c0` | Device::CanApply | In outline: whether a device setting can be applied (used by Device::ApplyC8BA70): resets (`0xBE9040`) and compares its value (+0x3C8) |
| `0xc36880` | Resource::ReleaseAllC36880 | In outline: releases every entry of a list (+0x7C, `0xC30890`) |
| `0xbcbb70` | Table::MakeBCBB70 | In outline: makes a table of 0x18-byte entries |
| `0xa6ff20` | Array::ResizeA6FF20 | In outline: resizes a list of 0x30-byte entries to n |
| `0x96f560` | Array::Resize96F560 | In outline: resizes a list of 0x30-byte entries to n |
| `0x94e4b0` | Array::Resize94E4B0 | In outline: resizes a list of 0x30-byte entries to n |
| `0xbd9670` | Text::FindBD9670 | In outline: finds a text entry (`0xBD65D0`) |
| `0xc18a70` | List::CopyC18A70 | In outline: copies a list (+0x10) into a new array |
| `0xcb2c60` | ShaderFile::HeaderCB2C60 | In outline: reads a shader file's 12-byte header (`0xCB2660`, `0xBE5020`) |
| `0x47cee0` | NamedObject::Make47CEE0 | In outline: makes a list member object (ListMember with `0x10EC9E8`) with its part (`0xCE59B0`) |
| `0xa91240` | Fence::TablesA91240 | In outline: makes a 0x14-byte record at start-up |
| `0x42a050` | Array::Resize42A050 | In outline: resizes a list of 32-byte entries to n |
| `0x4900f0` | NamedObject::Make4900F0 | In outline: makes a list member object (ListMember with `0x10EC9E8`) with its part (`0x48E000`) |
| `0x7c1780` | Load::Read7C1780 | In outline: reads a saved record from a block (`0x809210`, four bytes at a time) |
| `0xc18700` | Path::MatchC18700 | In outline: another path match (`0xBE84F0`, String::Substring) |
| `0xcc2f10` | Timer::StepCC2F10 | In outline: a time step from a count (`0xCC4740`) times a rate (`0x10F45D0`) |
| `0xc303c0` | Resource::PassC303C0 | In outline: goes over the shared list (`0x13E69BC` entries) |
| `0xbd9200` | Text::FindBD9200 | In outline: another text entry find (`0xBD65D0`) |
| `0xcef210` | Name::FindCEF210 | In outline: finds an entry by its name text |
| `0xcc9290` | Name::FindCC9290 | In outline: the same name find (another copy) |
| `0xcb0be0` | ShaderFile::OpenCB0BE0 | In outline: makes a part (0x2E) and opens a file (`0xC51360` with 4) |
| `0xafd3b0` | Array::ResizeAFD3B0 | In outline: resizes a list of 0x30-byte entries to n |
| `0x7be430` | Text::Make7BE430 | In outline: makes a text from a value |
| `0xa1bfe0` | Text::MakeA1BFE0 | In outline: makes a text from a shared one (`0xF265E4`) |
| `0xc28f20` | List::PassC28F20 | In outline: goes over a list (+4) when the value is not negative |
| `0xb4d470` | Window::TitleB4D470 | In outline: makes a window's title, copying a default when none is given |
| `0x9e17e0` | Array::Resize9E17E0 | In outline: resizes a list of 0x30-byte entries to n |
| `0x50ff10` | Array::Resize50FF10 | In outline: resizes a list of 32-byte entries to n |
| `0x9e1080` | Array::Resize9E1080 | In outline: resizes a list of 32-byte entries to n |
| `0x90d9c0` | Array::Resize90D9C0 | In outline: resizes a list of 32-byte entries to n |
| `0xca1b10` | Font::MakeCA1B10 | In outline: makes a font with the screen's device context (GetDC) |
| `0xc1ef70` | Resource::CheckC1EF70 | In outline: a resource check |
| `0xc02180` | System::VersionC02180 | In outline: reads the Windows version (GetVersionExA) and acts on NT 5 (2000/XP) |
| `0x7895a0` | Name::Find7895A0 | In outline: finds a name in a list ignoring case |
| `0xcae150` | ResourceFile::NameCAE150 | In outline: finds a part of a file name (`0xCADDF0`, String::Find of `0xFC078C`) |
| `0xa83890` | File::LoadA83890 | In outline: loads a file (ManagedObject::LoadFile) after `0xA820E0` |
| `0xca6cc0` | Resource::CheckCA6CC0 | In outline: a resource check (`0xCA6BC0`) when on (+0xC) |
| `0xc69190` | Resource::FindC69190 | In outline: finds a resource by name unless told not to |
| `0xc37290` | Resource::NameC37290 | In outline: makes a resource name from a C string (String::FromC) |
| `0xc181c0` | Path::MakeC181C0 | In outline: makes a path record |
| `0x808910` | Save::Text808910 | In outline: writes a text to a save, two bytes at a time (SaveWriter::Write) |
| `0xcb1f40` | ShaderFile::ReadCB1F40 | In outline: reads a shader file entry, clearing a pair first (ClearPair) |
| `0x806610` | Load::Read806610 | In outline: reads a two-byte value from a saved block |
| `0xceb190` | IdMap::Take | In outline: removes an entry by id (IdMap::Remove3) and acts on it |
| `0xae6460` | List::AddAE6460 | In outline: grows a list by one (ObjectArray::SetCapacity) |
| `0x4a0470` | NamedObject::Make4A0470 | In outline: makes a list member object (ListMember with `0x10EC9E4`) |
| `0xc102d0` | Resource::MakeC102D0 | In outline: makes a resource record (`0xC0A670`) |
| `0x4585d0` | Record::Make4585D0 | In outline: makes a record (`0x458010`) |
| `0x4583a0` | Record::Make4583A0 | In outline: the same record maker (another copy) |
| `0x458170` | Record::Make458170 | In outline: the same record maker (another copy) |
| `0x66ffa0` | Folder::Path66FFA0 | In outline: builds a path from the install folder text |
| `0x7ff180` | Text::Check7FF180 | In outline: a check on part of a text (WideString::Substring) |
| `0x470b20` | Load::Read470B20 | In outline: reads a four-byte value from a saved block |
| `0x69d9d0` | Input::Keyboard69D9D0 | In outline: a key step with the keyboard layout (GetKeyboardLayout) |
| `0x7374b0` | List::Index7374B0 | In outline: makes an index for a list (+0x5C entries) |
| `0x44e080` | Array::Resize44E080 | In outline: resizes a list to n |
| `0x90d7a0` | Array::Resize90D7A0 | In outline: resizes a list to n |
| `0xc4d580` | Text::FindC4D580 | In outline: another text entry find (`0xC47500`) |
| `0xbe0470` | Text::FindBE0470 | In outline: finds an entry by a C string |
| `0x8d9ef0` | Save::Write8D9EF0 | In outline: writes values to a save (SaveWriter::Write, four bytes) |
| `0x45b690` | Save::Write45B690 | In outline: writes values to a save (SaveWriter::Write, four bytes) |
| `0xcd25c0` | Resource::FindCD25C0 | In outline: finds a resource (`0xBCEE60`) |
| `0x812d40` | List::Range812D40 | In outline: a step over a range |
| `0xc5d1c0` | Vector::SameC5D1C0 | In outline: whether two points are nearly the same |
| `0x7fc4a0` | Load::Make7FC4A0 | In outline: makes a part |
| `0xbe0190` | Resource::FindBE0190 | In outline: finds a resource (`0xBDD290`) |
| `0xa7aa00` | Loop::StepA7AA00 | In outline: a loop step |
| `0xc4a180` | ShaderFile::ReadC4A180 | In outline: reads a four-byte value (`0xBE5020`) |
| `0x926360` | Array::Resize926360 | In outline: resizes a list to n |
| `0xa6fd20` | Array::ResizeA6FD20 | In outline: resizes a list to n |
| `0x809210` | Load::ReadBytes | In outline: reads n bytes from a saved block, checking against its bounds (+8, +0xC) |
| `0xc02a40` | Timer::StepC02A40 | In outline: a step when a time (+8) is positive |
| `0x402550` | Text::Grow402550 | In outline: grows a text when the other is longer |
| `0x746590` | Text::Make746590 | In outline: makes a text |
| `0xa83bb0` | Text::MakeA83BB0 | In outline: makes a text record |
| `0x743c20` | Text::Make743C20 | In outline: makes a text record |
| `0x784be0` | Text::Make784BE0 | In outline: makes a text record |
| `0xe04348` | Math::LoopE04348 | In outline: a loop over n values |
| `0xbc9f30` | List::PassBC9F30 | In outline: a step past the shared count (`0x13D0B10`) |
| `0xcedf80` | Shapes::GrowCEDF80 | In outline: grows a shape list (+8) |
| `0x55a090` | Record::Make55A090 | In outline: makes a record with an empty handle |
| `0xcae870` | ResourceFile::FindCAE870 | In outline: a resource file step by the shared flag (`0x13D0DF1`) |
| `0xc01120` | Device::StepC01120 | In outline: a step with the shared device (`0x13D0D84` +0x144) |
| `0x809020` | Save::Write809020 | In outline: writes values to a save (SaveWriter::Write, four bytes) |
| `0xc2eb20` | Resource::FindC2EB20 | In outline: a resource step (`0xC5C990`) |
| `0xc0c9b0` | Resource::ReleaseAllC0C9B0 | In outline: releases every entry of a list (+0x7C) |
| `0x403930` | Record::Make403930 | In outline: makes a part |
| `0x5f02b0` | Array::Resize5F02B0 | In outline: resizes a list to n |
| `0x4aa090` | Array::Resize4AA090 | In outline: resizes a list to n |
| `0xc53340` | Stream::StepC53340 | In outline: a stream step for a non-negative slot |
| `0xc1bff0` | Callback::CallC1BFF0 | In outline: calls a handler when it has one (+0x14) |
| `0x9e1c50` | List::Copy9E1C50 | In outline: copies a list (Array::Resize9E17E0) |
| `0xce80a0` | Registry::StepCE80A0 | In outline: a step with the registry (Registry::Ensure2) |
| `0xc1bbb0` | Text::MakeC1BBB0 | In outline: makes a text from a C string when it is not empty |
| `0x757b40` | Array::Make757B40 | In outline: makes an array |
| `0x7fa7f0` | Array::Make7FA7F0 | In outline: makes an array |
| `0xcb1480` | ShaderFile::StepCB1480 | In outline: a shader file step (`0xCB1360`) |
| `0x40fdd0` | Record::Make40FDD0 | In outline: makes a 0x30-byte record |
| `0xcadf80` | ResourceFile::NameCADF80 | In outline: a file name step (`0xCADDF0`) |
| `0xbe6b70` | Table::FillBE6B70 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0xc36d80` | Resource::SlotC36D80 | In outline: a step for a slot in range (`0xCADA40`) |
| `0xcdf4c0` | Loop::StepCDF4C0 | In outline: a loop |
| `0x99a080` | Record::Make99A080 | In outline: makes a record with an empty handle |
| `0x881fd0` | Record::Copy881FD0 | In outline: copies an entry (Thunk) |
| `0x53d720` | Record::Make53D720 | In outline: makes a record |
| `0xc04d00` | Device::CaseC04D00 | In outline: a device step by case |
| `0xbc2ef0` | Memory::RangeBC2EF0 | In outline: a step over a range |
| `0xe00920` | Memory::StepE00920 | In outline: a step when given a buffer |
| `0xbc31d0` | Memory::StepBC31D0 | In outline: a step (`0xBC30F0`) |
| `0xbe1a00` | Table::PassBE1A00 | In outline: goes over a table (+0x20C entries) |
| `0x7cf180` | Record::Make7CF180 | In outline: makes a 0xC-byte record |
| `0xc464d0` | Text::LoopC464D0 | In outline: a loop over a text |
| `0xc48fc0` | Array::MakeC48FC0 | In outline: makes an array of n values |
| `0x499c30` | NamedObject::Make499C30 | In outline: makes a list member object (ListMember with `0x10EC9E4`) |
| `0x9d8210` | ManagedObject::Make9D8210 | In outline: makes a managed object (ManagedObject with `0x13CE454`) |
| `0x750190` | Name::Find750190 | In outline: finds a name ignoring case |
| `0xc68d80` | Text::ParseC68D80 | In outline: reads a value from the rest of a text (WideString::Substring) |
| `0x417a20` | Array::Resize417A20 | In outline: resizes a list to n |
| `0xc3c3e0` | Resource::MakeC3C3E0 | In outline: makes a part (Constructor) |
| `0xc587f0` | Stream::StepC587F0 | In outline: a stream step (`0xCB06E0`) |
| `0xbc87b0` | Memory::StepBC87B0 | In outline: a step unless told not to |
| `0xcd21e0` | Resource::FindCD21E0 | In outline: finds a resource (`0xBCEE60`) |
| `0xc4ef00` | Loop::StepC4EF00 | In outline: a loop while more than one is left |
| `0x909f10` | List::Find909F10 | In outline: finds an entry up to n |
| `0x80f7d0` | List::Find80F7D0 | In outline: finds an entry up to n |
| `0xaf73a0` | Name::FindAF73A0 | In outline: a name step ignoring case (WideString::ToLower) |
| `0xadff60` | Name::FindADFF60 | In outline: the same (another copy) |
| `0xc339e0` | Resource::MakeC339E0 | In outline: makes a resource part (Resource::MakeC33730) |
| `0x909c10` | List::Find909C10 | In outline: finds an entry up to n |
| `0x87bcb0` | List::Find87BCB0 | In outline: finds an entry up to n |
| `0x499720` | List::Grow499720 | In outline: grows a list to hold n |
| `0xca46f0` | List::FindCA46F0 | In outline: a step for a positive count |
| `0xce76b0` | List::FindCE76B0 | In outline: finds an entry up to n |
| `0xb333e0` | Text::MakeB333E0 | In outline: makes a text from an empty one |
| `0xc236c0` | List::PassC236C0 | In outline: goes over n entries |
| `0x9d0f40` | List::Grow9D0F40 | In outline: grows a list to hold n |
| `0xcef460` | Name::FindCEF460 | In outline: finds an entry by name (`0xCEF120`) |
| `0xc57110` | Path::HasDrive | In outline: a step for a path that is shorter than two or has no drive letter (no ':' second) |
| `0xbdeb50` | Text::FindBDEB50 | In outline: a step with a text entry |
| `0xcbe870` | Text::MakeCBE870 | In outline: makes a text from an empty one |
| `0x7fc8e0` | Text::Trim7FC8E0 | In outline: a text without its last four characters (WideString::Substring) |
| `0x48d090` | Text::Number48D090 | In outline: writes a number as text ("%d") |
| `0xce7ef0` | Registry::StepCE7EF0 | In outline: a step with the registry (Registry::Ensure2) |
| `0xcb7b80` | ShaderFile::FindCB7B80 | In outline: a find by name for entries of kind 2 |
| `0xbb81d0` | Record::MakeBB81D0 | In outline: makes a 0x18-byte record |
| `0x7fee40` | Map::Find7FEE40 | In outline: a find in a sorted map (SortedMap::FindValue7FB9C0) |
| `0xcec590` | Pairs::AddCEC590 | In outline: adds a pair (Pairs::InsertCEAB50) |
| `0x46b250` | Text::Empty46B250 | In outline: whether a text is empty |
| `0xbe5570` | Module::RunBE5570 | In outline: a module step (ModuleList::Run) |
| `0xbbbe30` | List::PassBBBE30 | In outline: goes over a list |
| `0x42aee0` | Text::Lines42AEE0 | In outline: splits a text at line breaks (WideString::Find(10)) |
| `0xc51e00` | Stream::ReadC51E00 | In outline: a step when a stream has more (+0x14 vs +0x18) |
| `0xbec1a0` | Memory::StepBEC1A0 | In outline: a step by a flag's sign |
| `0xc2eee0` | Resource::StepC2EEE0 | In outline: a resource step (`0xC5C990`) |
| `0xbe2250` | VertexDeclaration::StepBE2250 | In outline: a step with a declaration (VertexDeclaration::FindBE1BD0) |
| `0xbdac20` | Text::MakeBDAC20 | In outline: makes a text from an empty one |
| `0xc5c7f0` | Device::FindC5C7F0 | In outline: a find in a list |
| `0xa28780` | Table::SetA28780 | In outline: sets a table (SetTableF2CEB4) |
| `0xc553e0` | Stream::SetC553E0 | In outline: a stream step on or off |
| `0x497bc0` | Save::Write497BC0 | In outline: a save step (Save::Flush497300) |
| `0x497a20` | Save::Write497A20 | In outline: the same (another copy) |
| `0x8aee00` | NamedObject::Make8AEE00 | In outline: makes a list member object (ListMember with `0x10EC9E4`) |
| `0x492b30` | Record::Make492B30 | In outline: makes a 0x98-byte record |
| `0x620f30` | Load::Read620F30 | In outline: reads a four-byte value from a saved block |
| `0x401310` | Record::Check401310 | In outline: a step by a value's sign |
| `0xc5c990` | Device::FindC5C990 | In outline: a find in a list |
| `0xbe4b30` | Loop::StepBE4B30 | In outline: a loop |
| `0xc4f0e0` | KeyMap::Step | In outline: a step with a key map entry (KeyMap::Find) |
| `0xbeb4b0` | List::LoopBEB4B0 | In outline: goes over a linked list |
| `0xceb980` | List::FindCEB980 | In outline: a step under a count (+0x18) |
| `0xc40280` | OverlayFile::Step | In outline: a step that relocates an overlay file (OverlayFile::Relocate) |
| `0x98e4f0` | Loop::Step98E4F0 | In outline: a loop |
| `0xc46160` | Resource::SetC46160 | In outline: a step when two values change (+0x10, +0x14) |
| `0x9acf50` | Name::Find9ACF50 | In outline: a name step ignoring case |
| `0x6a39d0` | Text::Make6A39D0 | In outline: makes a text from a shared one (`0xF27994`) |
| `0xc36bd0` | Text::MakeC36BD0 | In outline: makes a text from an empty one |
| `0xc042a0` | Loop::StepC042A0 | In outline: a loop |
| `0xcb0510` | ShaderFile::MakeCB0510 | In outline: makes a part (0x1E) |
| `0xc575d0` | Record::MakeC575D0 | In outline: makes a record |
| `0xc1be20` | Module::StepC1BE20 | In outline: a module load (Module::LoadBE5B20) |
| `0xc4d380` | Record::MakeC4D380 | In outline: makes a 0xC-byte record |
| `0x457e10` | Text::Copy457E10 | In outline: copies a C string |
| `0xb84e00` | Buffer::MakeB84E00 | In outline: makes a buffer (Buffer16) |
| `0xb851b0` | Buffer::MakeB851B0 | In outline: makes a buffer (`0xB84E00`) |
| `0xc6adb0` | List::LastC6ADB0 | In outline: a step for the last entry |
| `0x638da0` | Flags::Read638DA0 | In outline: reads a set of flags (bits 0x4000, 0x200, ...) into a value |
| `0x43ad20` | Math::Log43AD20 | In outline: a value from a base-2 log of a constant (`0xF276F0`) |
| `0x801ae0` | List::Find801AE0 | In outline: a find over a list (+4) |
| `0x7fab10` | Record::Make7FAB10 | In outline: makes a part |
| `0xce3d60` | List::PassCE3D60 | In outline: goes over entries (+0x28) |
| `0xcaddf0` | ResourceFile::NameCADDF0 | In outline: a file name step by a value's sign |
| `0xcc04e0` | Shader::StepCC04E0 | In outline: a shader step (`0xCBE430`) |
| `0xcbe1d0` | Array::ResizeCBE1D0 | In outline: resizes a list to n |
| `0xc48e30` | Text::MakeC48E30 | In outline: makes a text from an empty one |
| `0xc3c260` | Path::IsAbsolute | In outline: whether a path is long enough and has a drive letter (':' second) |
| `0xb0def0` | Array::ResizeB0DEF0 | In outline: resizes a list to n |
| `0xcbd900` | Array::ResizeCBD900 | In outline: resizes a list to n |
| `0x409660` | List::Grow409660 | In outline: grows a list to hold n |
| `0xc513a0` | Stream::PartC513A0 | In outline: a step with a part |
| `0xbe3820` | Resource::StepBE3820 | In outline: a step |
| `0x9ffe70` | List::Grow9FFE70 | In outline: grows a list to hold n |
| `0xc51c80` | Stream::SizeC51C80 | In outline: a step for sizes above 0x7FFFFFFF |
| `0xbd6160` | Resource::StepBD6160 | In outline: a step (`0xC1FC30`) |
| `0x714ff0` | Record::Make714FF0 | In outline: makes a record with an empty handle |
| `0x47d570` | NamedObject::Make47D570 | In outline: makes a list member object (ListMember with `0x10EC9E8`) |
| `0xa27aa0` | NamedObject::MakeA27AA0 | In outline: makes a list member object (ListMember with `0x10EC9E4`) |
| `0xcba4e0` | Shader::StepCBA4E0 | In outline: a shader step (`0xC46720`) |
| `0x40d2b0` | Record::Step40D2B0 | In outline: a step (`0x40C3C0`) |
| `0xc49180` | Array::MakeC49180 | In outline: makes an array of n values |
| `0x808bf0` | Array::Make808BF0 | In outline: makes an array of n values from a list (+0x10) |
| `0x58eb20` | Array::Resize58EB20 | In outline: resizes a list of 20-byte entries to n |
| `0xcebc50` | Array::SetCountCEBC50 | In outline: sets a count (+0x3C) when it changes |
| `0xc5c670` | Device::FindModeC5C670 | In outline: finds a list entry matching two values (+0x14, +0x18) |
| `0xc03e00` | Device::CheckC03E00 | In outline: a device step that handles the D3D "invalid call" error (0x8876086C) |
| `0x49dad0` | Text::Decimal49DAD0 | In outline: writes a number with two decimals ("%.2f") |
| `0x49d950` | Text::Decimal49D950 | In outline: the same (another copy) |
| `0x49d7d0` | Text::Decimal49D7D0 | In outline: the same (another copy) |
| `0x49d650` | Text::Decimal49D650 | In outline: the same (another copy) |
| `0x49a5c0` | Array::Grow49A5C0 | In outline: grows an array of four-byte values |
| `0x47d3f0` | NamedObject::Make47D3F0 | In outline: makes a list member object (ListMember with `0x10EC9E8`) with its part (NamedObject::Make47CEE0) |
| `0xc2b950` | Resource::ClearC2B950 | In outline: clears two parts (`0xC2B7B0`, `0xC256F0`) |
| `0x406d70` | Movies::Check | In outline: a check over the movie list (Movies::List) for empty names |
| `0xc6ac40` | Resource::MakeC6AC40 | In outline: makes a 0xE0-byte part (`0xC65F20` with 0x80) |
| `0xc365a0` | Resource::MakeC365A0 | In outline: makes a part (`0xBE86F0` with 0x80, `0xBE79B0`) |
| `0x48cf20` | Folder::Find48CF20 | In outline: finds a folder by name (`0xBD0960` with `0xF28E58`) |
| `0xce6400` | Memory::CopyCE6400 | In outline: copies n bytes, four at a time, when given both buffers |
| `0xc109c0` | Resource::MakeC109C0 | In outline: makes a resource record (Resource::MakeC102D0) |
| `0x9f7fc0` | Array::Grow9F7FC0 | In outline: grows a list of 64-byte entries to hold n |
| `0x617f40` | Text::Compare617F40 | In outline: compares two C strings |
| `0x442af0` | Text::Asian442AF0 | In outline: a step by whether the language is an Asian one (Language::IsAsian) |
| `0xbcbe20` | List::StepBCBE20 | In outline: a step for n before the last entry |
| `0xbc2d80` | Mesh::MakeBC2D80 | In outline: makes a mesh buffer with its format (Mesh::SetFormatC31770) |
| `0x449aa0` | Array::Grow449AA0 | In outline: grows a list of 64-byte entries to hold n |
| `0xc580b0` | Loop::StepC580B0 | In outline: a loop up to n |
| `0xc0eab0` | Resource::MakeC0EAB0 | In outline: makes a part (Constructor) |
| `0xa0acb0` | Record::MakeA0ACB0 | In outline: makes parts (`0xBDADC0`, Constructor) |
| `0xcbe700` | Array::AddCBE700 | In outline: adds an entry, reserving room first (Array::Reserve) |
| `0xca2cf0` | Font::StepCA2CF0 | In outline: a step when on (+0x10) |
| `0xc4c320` | Array::MakeC4C320 | In outline: makes an array of n values |
| `0xc00680` | Device::CaseC00680 | In outline: a step by case for values under 0x1F |
| `0xbfdfd0` | Range::MakeBFDFD0 | In outline: makes a part for a non-empty range |
| `0xbea3b0` | Range::MakeBEA3B0 | In outline: the same (another copy) |
| `0xca6a50` | Memory::CopyCA6A50 | In outline: a copy into a buffer, made when there is none |
| `0xbfab20` | Resource::StepBFAB20 | In outline: a step (`0xC31000`) when asked |
| `0xbe80b0` | Regions::CaseBE80B0 | In outline: a step by case (`0xBE7A00`) |
| `0xcbcc10` | Shader::SetCBCC10 | In outline: sets a shader part (`0xCBE010`, `0xCBBB00`) |
| `0xc51b10` | Stream::StepC51B10 | In outline: a stream step under a count (+0x1C) |
| `0xc10b30` | Resource::MakeC10B30 | In outline: makes a part (Constructor, `0xC0A110`) |
| `0xa3e3e0` | Object::MakeA3E3E0 | In outline: when on (+0xC) with a part (+0x10), makes an object (ObjectA402B0) |
| `0x8be610` | Terrain::Make8BE610 | In outline: a terrain step |
| `0xa3d4c0` | Module::LoadA3D4C0 | In outline: a module load (Module::LoadBE5B20) and a check (`0xBE40D0`) |
| `0x9fe100` | Array::Grow9FE100 | In outline: grows a list of 0x30-byte entries to hold n |
| `0x4573e0` | Text::Scan4573E0 | In outline: goes over a C string when given one |
| `0xc1b480` | Record::MakeC1B480 | In outline: makes an 8-byte record |
| `0x4303c0` | Array::Grow4303C0 | In outline: grows a list of 0x30-byte entries to hold n |
| `0xe03e90` | SortedTable::StepE03E90 | In outline: a step when an entry is not found (SortedTable::Find) |
| `0xcc8ee0` | Name::FindCC8EE0 | In outline: a find over entries (+0xC) |
| `0xba3440` | Array::ResizeBA3440 | In outline: resizes a list of 0xB8-byte entries to n |
| `0x803400` | Load::Read803400 | In outline: reads four-byte values from a saved block (Load::ReadBytes) |
| `0x68f040` | Array::Resize68F040 | In outline: resizes a list of 0xC-byte entries to n |
| `0x68b6a0` | Array::Copy68B6A0 | In outline: copies a list (ObjectArray28::Resize) |
| `0x9d84f0` | Record::Make9D84F0 | In outline: makes a record with an empty handle (`0x9D7FC0` with +0x70) |
| `0x7bb970` | Record::Make7BB970 | In outline: makes parts (`0xBDADC0`, Constructor) |
| `0x875ce0` | Scenery::Make875CE0 | In outline: makes a pooled object (PooledObject) with a path table (`0x488290`) |
| `0x497f20` | Save::Range497F20 | In outline: a step for n within a range |
| `0xce6090` | Device::ErrorText | In outline: gives a text for a device error code |
| `0xcc50e0` | Shader::StepCC50E0 | In outline: a shader step (`0xCC1DD0` with fill values, `0xCC4F30`) |
| `0xc44500` | Resource::SameC44500 | In outline: a step when a value matches (+8) |
| `0xc37c10` | Name::SameC37C10 | In outline: compares two names by length and text (WideString::Compare) |
| `0x6841a0` | Text::Cut6841A0 | In outline: cuts a text (WideString::Substring, WideString::MakeUnique) |
| `0xbc6d60` | Memory::CheckBC6D60 | In outline: a step when a check passes (`0xBC5200`) |
| `0xcc94e0` | Name::FindCC94E0 | In outline: a find by name (`0xCC91A0`) over entries (+0xC) |
| `0xb105f0` | Record::MakeB105F0 | In outline: makes parts |
| `0xce6570` | Sound::Init | In outline: starts COM (CoInitialize) and creates the DirectSound device (DirectSoundCreate) into `0x1400B68` |
| `0xce9000` | Shapes::PassCE9000 | In outline: goes over entries with a positive count (+0x64) |
| `0x452bd0` | Record::Make452BD0 | In outline: makes parts (`0xBDADC0`, Constructor) |
| `0x40fb70` | NamedObject::Make40FB70 | In outline: makes a list member object (ListMember with `0x10EC9E8`) with an Object413 |
| `0xce7a90` | SortedPointers::StepCE7A90 | In outline: a step when a flagged entry is found (SortedPointers::FindFlagged) |
| `0xb11d40` | Array::ResizeB11D40 | In outline: resizes a list of four-byte entries to n |
| `0x460f20` | Array::Resize460F20 | In outline: resizes a list of four-byte entries to n |
| `0xa53c20` | Array::ResizeA53C20 | In outline: resizes a list of eight-byte entries to n |
| `0x40c7e0` | Array::Grow40C7E0 | In outline: grows a list of 0x24-byte entries to hold n |
| `0xcba270` | ShaderFile::ReadCBA270 | In outline: reads a shader file entry, clearing a pair first, for entries marked 's' |
| `0x8081f0` | Load::Read8081F0 | In outline: reads a 12-byte value from a saved block (Load::ReadBytes) |
| `0xc01720` | Record::MakeC01720 | In outline: makes a record (`0xC01600`) |
| `0xbd0ab0` | Folder::FindBD0AB0 | In outline: finds by a name text, with -1 handled |
| `0xbc5900` | Track::SnapBC5900 | In outline: unless a mode is 0 or 1, snaps (Track::SnapBC5580) |
| `0x9843e0` | Text::Edit9843E0 | In outline: a text edit step (WideString::MakeUnique) |
| `0xcad5c0` | Debug::Printf | In outline: formats a message (vsprintf) and prints it (printf) |
| `0xbe6480` | Device::MakeTextures | In outline: creates textures in a loop (Device::CreateTexture) |
| `0xcb6270` | Array::MakeCB6270 | In outline: makes a list of 16-byte entries (`0xCB61E0`) |
| `0x801e30` | Record::Make801E30 | In outline: makes a 0x50-byte record, cleared |
| `0x45b2b0` | Resource::Reset45B2B0 | In outline: tears down and remakes a part (Destructor, `0xBE68A0`) |
| `0xce3450` | Table::FindCE3450 | In outline: a find (`0xCE3100`), with -1 handled |
| `0xbecd70` | Text::AddBECD70 | In outline: makes a text and acts when the shared count (`0x13D23BC`) is over 299 |
| `0xbc92e0` | Memory::CheckBC92E0 | In outline: a step when a check passes (`0xBC8A30`) |
| `0xbd6930` | Record::MakeBD6930 | In outline: makes a record with an empty text (TextRef::TextRefEmpty) |
| `0xbd2a30` | List::ValueBD2A30 | In outline: a step over entries comparing values |
| `0xbd0960` | Folder::FindByName | In outline: finds a folder by a C-string name, with none handled |
| `0xbe4cd0` | Loop::StepBE4CD0 | In outline: a loop |
| `0x7bc370` | Text::Length7BC370 | In outline: a step for a text between 5 and 0xFFF characters long |
| `0x446a10` | Text::Edit446A10 | In outline: a text edit step (WideString::MakeUnique) |
| `0x802a20` | Save::Text802A20 | In outline: writes a text to a save, two bytes at a time (SaveWriter::Write) |
| `0x4355e0` | Record::Make4355E0 | In outline: makes a record |
| `0xaaabf0` | Array::ResizeAAABF0 | In outline: resizes a list of 0x38-byte entries to n |
| `0x8e9330` | Array::Grow8E9330 | In outline: grows a list of 0x130-byte entries to hold n |
| `0x87bee0` | Array::Grow87BEE0 | In outline: grows a list of 0x80-byte entries to hold n |
| `0x735850` | Array::Grow735850 | In outline: grows a list of 0x98-byte entries to hold n |
| `0x617600` | Windows::PumpMessages | In outline: pumps waiting Windows messages (PeekMessageA, TranslateMessage), with a time value |
| `0x743e10` | Folder::Init743E10 | In outline: a step with the install folder text (Init458) |
| `0xc3eee0` | Stream::OpenC3EEE0 | In outline: opens a stream (`0xC51360` with 2, Stream::StepC53340) |
| `0xbe9630` | List::PassBE9630 | In outline: goes over a list (+0x20) |
| `0xbc33a0` | Text::MakeBC33A0 | In outline: makes a C string buffer |
| `0x49e3b0` | Array::Grow49E3B0 | In outline: grows a list of 0x30-byte entries to hold n |
| `0x40d840` | Array::Grow40D840 | In outline: grows a list (+0x34) |
| `0xcd29f0` | Loop::StepCD29F0 | In outline: a loop while a value is not negative |
| `0xbe5220` | Module::RunBE5220 | In outline: a module step (ModuleList::Run, Error::ThrowIf) |
| `0x457c00` | Text::Case457C00 | In outline: a step for values 0x10D to 0x10F |
| `0x445af0` | SortedTable::Add445AF0 | In outline: a step when an entry is not found (SortedTable::Find8) |
| `0xba32c0` | Array::ResizeBA32C0 | In outline: resizes a list of 24-byte entries to n |
| `0xb92c80` | Pair::CompareB92C80 | In outline: compares two pairs (`0xCD5640`) |
| `0x4a36e0` | Pair::Compare4A36E0 | In outline: the same (another copy) |
| `0xaa17a0` | Array::ResizeAA17A0 | In outline: resizes a list of 0x2C-byte entries to n |
| `0x457740` | Table::Fill457740 | In outline: fills a 0x100-entry table |
| `0xc46810` | Table::ReadCTBL | In outline: reads a block marked "CTBL" (0x4C425443) and a two-byte value (`0xBE5020`) |
| `0xc25a10` | List::PassC25A10 | In outline: goes over entries (+0xC) that are set |
| `0xc1cae0` | Table::FillC1CAE0 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0xa1ea60` | Record::MakeA1EA60 | In outline: makes a 0xC-byte record (`0xA1D400`) |
| `0xcb0a20` | ShaderFile::MakeCB0A20 | In outline: a loader step (Loader::PumpC536D0) and a part (0x16) |
| `0xe00ba0` | Memory::StepE00BA0 | In outline: a step (`0xDFD8F0`, Memory::StepE00920) |
| `0xc443c0` | Resource::SameC443C0 | In outline: a step when values match (+8) |
| `0xcc6060` | Model::MakeCC6060 | In outline: makes 64-byte and four-byte entry arrays for a model's parts (+0x24) |
| `0x9d4140` | Text::Make9D4140 | In outline: makes a text record |
| `0x9738e0` | SortedPointers::Step9738E0 | In outline: a step when a first entry is not found (SortedPointers::FindFirst) |
| `0x46b750` | System::Version46B750 | In outline: reads the Windows version (GetVersionExA) and acts on versions above 3 |
| `0xcebe40` | Array::GrowCEBE40 | In outline: grows a list of 0x70-byte entries to hold n |
| `0xce78e0` | Array::GrowCE78E0 | In outline: the same (another copy) |
| `0x4d01a0` | Array::Grow4D01A0 | In outline: grows a list of 0x2C-byte entries to hold n |
| `0x483864` | Text::Compare483864 | In outline: compares characters |
| `0xcedc50` | Shapes::FlagCEDC50 | In outline: a step for a part that is on (+0x50) |
| `0x40c6a0` | Array::Grow40C6A0 | In outline: grows a list of 0x70-byte entries to hold n |
| `0xb78ea0` | Entries::SortB78EA0 | In outline: sorts entries (Entries::SortB782A0 over +8) |
| `0xcbe500` | Shader::StepCBE500 | In outline: a step (`0xCBDFB0`) with a buffer |
| `0x87f420` | Text::Make87F420 | In outline: makes a text from a shared one (`0xF3F4A4`) |
| `0x8099c0` | Record::Make8099C0 | In outline: a step (`0x555A60`) and a part |
| `0x43abe0` | Text::Check43ABE0 | In outline: a step for an empty text |
| `0xbf5930` | Text::StepBF5930 | In outline: a step (`0xBF4B20`) past a count (+0x14) |
| `0xbe28a0` | VertexDeclaration::FillBE28A0 | In outline: a fill step (VertexDeclaration::StepBE2250, `0xBDC800`) |
| `0xbd9d20` | Resource::LinkBD9D20 | In outline: sets a part (+0x24) when it changes |
| `0xe00060` | Memory::StepE00060 | In outline: a step with a buffer, with none handled |
| `0xc9bfd0` | Device::MakeTexturesC9BFD0 | In outline: creates 64 one-pixel textures (Device::CreateTexture(1, 1, 0, 1)) |
| `0xc386d0` | Resource::MakeC386D0 | In outline: a step (`0xC424B0` with `0x13E6B0C`) |
| `0xbfd310` | List::PassBFD310 | In outline: goes over a list (+0x20) |
| `0x6930b0` | Array::Resize6930B0 | In outline: resizes a list of 64-byte entries to n (the layout table) |
| `0x499160` | Table::Fill499160 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0xc31080` | Resource::SlotC31080 | In outline: a step for a slot under the shared count (`0x13E69BC`) |
| `0x991af0` | Array::Resize991AF0 | In outline: resizes a list of 0x28-byte entries to n |
| `0x823040` | Array::Resize823040 | In outline: resizes a list of 0x28-byte entries to n |
| `0x51be40` | Array::Resize51BE40 | In outline: resizes a list of 0x28-byte entries to n |
| `0x455df0` | Array::Copy455DF0 | In outline: copies a list of 24-byte entries |
| `0xc03040` | Table::FillC03040 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0x80fa40` | Array::Grow80FA40 | In outline: grows a list of 16-byte entries to hold n |
| `0x49a7e0` | Array::Grow49A7E0 | In outline: the same (another copy) |
| `0x42e780` | Array::Grow42E780 | In outline: the same (another copy) |
| `0x42d730` | Array::Grow42D730 | In outline: the same (another copy) |
| `0xc93330` | Shader::CaseC93330 | In outline: a step by a byte's case |
| `0xce19f0` | Text::LowerCE19F0 | In outline: lower-cases characters (_tolower) |
| `0xcc4010` | Shader::StepCC4010 | In outline: a shader step (`0xCC1DD0` with fill values, `0xCC3E60`) |
| `0x9f98a0` | Array::Grow9F98A0 | In outline: grows a list of 0x90-byte entries to hold n |
| `0x964bc0` | Array::Grow964BC0 | In outline: grows a list of 0x1018-byte entries to hold n |
| `0x94e250` | Array::Grow94E250 | In outline: grows a list of 0xD0-byte entries to hold n |
| `0xce0540` | Record::MakeCE0540 | In outline: makes a 0x28-byte record |
| `0xac6b90` | Array::ResizeAC6B90 | In outline: resizes a list of 0x24-byte entries to n |
| `0xcb79b0` | ShaderFile::OpenCB79B0 | In outline: makes a part (6) and opens a file (`0xC51360` with 4) |
| `0xc02540` | Timer::StepC02540 | In outline: a timed step (Timer::Milliseconds, `0xC50080`) |
| `0x818480` | Array::Resize818480 | In outline: resizes a list of 0x2C-byte entries to n |
| `0xcbe010` | HashTable::Grow | In outline: when full (+8 = +4), grows to the next prime size (Math::NextPrime) |
| `0xceee80` | Shapes::PassCEEE80 | In outline: goes over entries (+0x24) |
| `0xcec0c0` | IdMap::StepCEC0C0 | In outline: a step when an id is found (IdMap::Find3 with +0x28) |
| `0xbeac90` | Memory::MakeBEAC90 | In outline: a loop with a 0x1000-byte part (`0xBEA150`) |
| `0x6bf640` | Array::Free6BF640 | In outline: frees an array of 24-byte entries, tearing each down (`0x6BCDB0`) |
| `0xbfb680` | Module::LoadBFB680 | In outline: a module load (Module::LoadBE5B20 with 1) and a check (`0xBE40D0`) |
| `0xb6d270` | Array::GrowB6D270 | In outline: grows a list of 0xC-byte entries to hold n |
| `0x9722b0` | Description::Check9722B0 | In outline: a step when descriptions differ (Description::Differs) |
| `0xc964d0` | Device::LoopC964D0 | In outline: a step (`0xC051F0`) over entries (+0x2190) |
| `0x684300` | Text::Split684300 | In outline: splits a text into two parts (WideString::Substring) |
| `0xc73150` | Table::FillC73150 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0xb9ff30` | Save::WriteB9FF30 | In outline: writes four four-byte values to a save (SaveWriter::Write) |
| `0x9c8ec0` | Save::Write9C8EC0 | In outline: the same (another copy) |
| `0x55bcd0` | Save::Write55BCD0 | In outline: the same (another copy) |
| `0x533460` | Save::Write533460 | In outline: the same (another copy) |
| `0x94e380` | Array::Grow94E380 | In outline: grows a list of 0x60-byte entries to hold n |
| `0x947540` | Array::Resize947540 | In outline: resizes a list of 16-byte entries to n |
| `0x8ffa60` | Array::Grow8FFA60 | In outline: grows a list of 0x60-byte entries to hold n |
| `0x8c8a70` | Table::Fill8C8A70 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0x696000` | Table::Fill696000 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0x536030` | Reference::Make536030 | In outline: makes a 0xC-byte reference (Reference) |
| `0xc1cc20` | Table::FillC1CC20 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0xc08ba0` | Text::LoopC08BA0 | In outline: a loop over a list (`0xC08890`) |
| `0xbdce30` | VertexDeclaration::LoopBDCE30 | In outline: goes over n elements, with 0xFF handled |
| `0x87c800` | Table::Fill87C800 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0x85b290` | Table::Fill85B290 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0x455510` | Array::Resize455510 | In outline: resizes a list of 0x90-byte entries to n |
| `0xcd28c0` | Loop::StepCD28C0 | In outline: a loop |
| `0xcc7200` | Model::FindCC7200 | In outline: finds an entry matching a value |
| `0xc00c80` | Device::VersionC00C80 | In outline: a value by a version number (0x30000, ...) |
| `0x5aeda0` | Array::Resize5AEDA0 | In outline: resizes a list of 0x28-byte entries to n |
| `0x4b6760` | Array::Resize4B6760 | In outline: resizes a list of 20-byte entries to n |
| `0x447610` | Table::Fill447610 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0xbd3dc0` | Device::CaseBD3DC0 | In outline: a value by case (0x3C, ...) |
| `0x9fa0e0` | SortedTable::Step9FA0E0 | In outline: a step when an entry is found (SortedTable::Find64) |
| `0x8f8ac0` | Text::Colon8F8AC0 | In outline: reads a text up to a ':' |
| `0xcd1530` | Table::StepCD1530 | In outline: a step (`0xCD0C60`) over entries |
| `0xc291d0` | Resource::SlotC291D0 | In outline: a step (`0xC21380`) for a slot under a count (+0xC) |
| `0x9f46b0` | Load::Read9F46B0 | In outline: reads a four-byte value from a saved block (Load::ReadBytes) |
| `0xcba000` | ShaderFile::CheckCBA000 | In outline: a step for a buffer of more than 0x15AF bytes |
| `0xc63b40` | Record::MakeC63B40 | In outline: makes a 0x1C-byte record |
| `0xc48240` | Loop::StepC48240 | In outline: a loop while more than one is left |
| `0xaf9dd0` | Array::ResizeAF9DD0 | In outline: resizes a list of 0x2C-byte entries to n |
| `0x9f99d0` | Array::Grow9F99D0 | In outline: grows a list of 0x24-byte entries to hold n |
| `0x8cb5c0` | Array::Resize8CB5C0 | In outline: resizes a list of 20-byte entries to n |
| `0x513490` | Array::Resize513490 | In outline: resizes a list of 20-byte entries to n |
| `0xc2f480` | Resource::LoopC2F480 | In outline: a loop and a step for a slot under a count (+0x18) |
| `0xa1d0f0` | Name::FindA1D0F0 | In outline: a find by comparing names (WideString::Compare) |
| `0x69d460` | Input::ImeComposition | In outline: when flagged (8), reads the IME composition text (ImmGetCompositionStringW) |
| `0x417e40` | List::Add417E40 | In outline: adds an entry when full (+0x14 vs +0xC), under 0x10 |
| `0x410420` | NamedObject::Make410420 | In outline: makes a list member object (ListMember with `0x10EC9E8`) with an Object413 (0xB) |
| `0xcba3c0` | ShaderFile::CheckCBA3C0 | In outline: a check on a buffer's place against its end |
| `0x498510` | List::Grow498510 | In outline: grows a list (+8) to hold one more |
| `0x9d4960` | Resource::Count9D4960 | In outline: a step (`0xBCE9D0`) for a positive count |
| `0x807700` | Load::Read807700 | In outline: reads a four-byte value from a saved block (Load::ReadBytes) |
| `0xbd80a0` | Table::FillBD80A0 | In outline: fills a 0x1D table with 0xFFFF (`0x416110`) |
| `0x8f7750` | Table::Fill8F7750 | In outline: fills a two-entry table with 0xFFFF (`0x416110`) |
| `0x4164c0` | Table::Fill4164C0 | In outline: fills a four-entry table with 0xFFFF (`0x416110`) |
| `0xbd2b80` | List::LoopBD2B80 | In outline: goes over a linked list |
| `0xcb5be0` | Shader::SetCB5BE0 | In outline: sets a value (+0x60) when it changes, with under two handled |
| `0xc3a6d0` | Resource::FindC3A6D0 | In outline: a step when found (`0xC42410`) |
| `0xa53de0` | File::LoadA53DE0 | In outline: a step (Window::StepA52B40) and loads a file (ManagedObject::LoadFile) |
| `0x55b1b0` | Record::Make55B1B0 | In outline: makes a record with an empty handle |
| `0xcefa90` | Array::GrowCEFA90 | In outline: grows a list of eight-byte entries to hold n |
| `0xcef970` | Array::GrowCEF970 | In outline: the same (another copy) |
| `0xcaf740` | ResourceFile::MakeCAF740 | In outline: makes a part (Constructor, `0xCADD90`) |
| `0xa01ce0` | Save::WriteA01CE0 | In outline: writes four four-byte values to a save (SaveWriter::Write) |
| `0x5ba680` | Save::Write5BA680 | In outline: the same (another copy) |
| `0x800340` | Text::Make800340 | In outline: makes a text (`0x6A5E20`) |
| `0x45c520` | Effect::Case45C520 | In outline: reads two parts' values (slot 0xFC of +0xE4 and +0xE8) and acts by an id (0x4089A00, ...), keeping counts under 0xFF |
| `0x7e72a0` | Heap::Pop7E72A0 | In outline: takes the first entry off a heap of +0x18 entries at +0x10 (-1 when empty), moves the last one in and sifts it down by value |
| `0x4c1560` | Text::Buffer4C1560 | In outline: makes a buffer the size a virtual call gives (slot 0x58) and fills it |
| `0xa03ef0` | File::LoadA03EF0 | In outline: joins a name with a shared text (`0xF2A7D4`) and loads that file (ManagedObject::LoadFile) |
| `0x68faf0` | Array::Remove68FAF0 | In outline: removes entry n, checking "_index >= 0" (AssertFailed) |
| `0xce9470` | Array::AddCE9470 | In outline: adds an entry, growing the list at +0x7C (200 at first, then double) when +0x80 reaches +0x84 |
| `0xc55240` | Stream::MakeC55240 | In outline: makes a stream record with an empty name (String::FromC) |
| `0xc43660` | Resource::MakeC43660 | In outline: unless a check passes (`0xBDA990`), makes a 0xC-byte record |
| `0xbeb270` | Range::FindBEB270 | In outline: goes over a list (+0x1BC) making a part for each range (Range::MakeBEA3B0) |
| `0xca58b0` | Font::MakeCA58B0 | In outline: sets up a record: its table (`0xFBF5B0`), two bytes and a value from its argument, and clears the rest |
| `0xbee0c0` | Matrix::Invert3 | Inverts a 3x3 matrix in place: works out its cofactors and divides by the determinant |
| `0xc33550` | Resource::MakeC33550 | In outline: a step (`0xC5E410`) and makes a part |
| `0x44a200` | List::AddNumber44A200 | In outline: grows the list (`0x449290`, double) when full and adds a number written as text ("%d") |
| `0x49cd90` | Text::Number49CD90 | In outline: writes a number rounded down (to a 128-character buffer) |
| `0x45eb50` | Text::Format45EB50 | In outline: formats into a 1024-character buffer when not already set (+4) |
| `0x458010` | Folder::Special458010 | In outline: finds a special shell folder's path (an item list, IMalloc, a 260-character path) |
| `0xce7450` | Array::SetCountCE7450 | In outline: sets a count (+0x28) when it changes, tearing down entries past it |
| `0xc2ed00` | Resource::FindC2ED00 | In outline: when it has none (+0x24), finds one (Device::FindC5C990) |
| `0xbdf950` | Table::InitBDF950 | In outline: once (`0x13D0E74` bit 0), fills a shared table with 0xFFFF (`0x416110`) and points at it (`0x13D0E4C`) |
| `0xc2fb10` | HashTable::AddRef | In outline: finds an entry by name (`0xC2FA90`) and adds one to its count, or adds it |
| `0xbfec60` | Range::FindBFEC60 | In outline: goes over a list (+0x3C) making a part for each range (Range::MakeBFDFD0) |
| `0x9c71d0` | Array::Resize9C71D0 | In outline: resizes a list to n |
| `0x53a080` | Array::Resize53A080 | In outline: resizes a list to n |
| `0xa32c00` | Array::ResizeA32C00 | In outline: resizes a list to n |
| `0x823d90` | Array::Resize823D90 | In outline: resizes a list to n |
| `0xcae420` | ResourceFile::CheckCAE420 | In outline: a step with a file name (ResourceFile::NameCADF80) when a check fails (`0xCAD810`) |
| `0xc10730` | Resource::MakeC10730 | In outline: makes a resource record (Resource::MakeC102D0) |
| `0xaaafd0` | Array::ClearAAAFD0 | In outline: clears a count (+0xC) and frees its buffer (+0x10) when it owns it |
| `0x5d0ea0` | Record::Make5D0EA0 | In outline: makes a 0x2C-byte record pointing at its own storage |
| `0x4c6930` | Handle::Make4C6930 | In outline: makes a handle and links it |
| `0xce32b0` | Table::AddCE32B0 | In outline: adds an entry when not found (`0xCE3250`), taking the next number (+0x40) |
| `0x429190` | Sky::Blend429190 | In outline: blends two shared colours by a fraction (+0x66C) into shared values (`0x1103B78` on) |
| `0xc08680` | Timer::CpuSpeed | In outline: times the processor against the performance counter (QueryPerformanceFrequency, QueryPerformanceCounter, rdtsc) |
| `0xc66a70` | Window::MakeC66A70 | In outline: makes a window part (`0xC65F20`) with its table (`0xFB7A10`) |
| `0xcbd110` | Text::MakeCBD110 | In outline: makes a text record, empty when given nothing |
| `0xbdf2d0` | HashTable::InsertBDF2D0 | In outline: grows when full (`0xBE0840`), hashes a key (`0xBDBF70`) modulo its size (+4) and probes for a slot |
| `0xbdf1c0` | HashTable::InsertBDF1C0 | In outline: the same with `0xBE0730` |
| `0xbe0840` | HashTable::GrowBE0840 | In outline: when full (+8 = +4), grows to the next prime size (Math::NextPrime, `0xBDFBB0`) and puts the entries back |
| `0xbe0730` | HashTable::GrowBE0730 | In outline: the same with `0xBDFAE0` |
| `0xb959c0` | Array::ResizeB959C0 | In outline: resizes a list to n |
| `0x991910` | Array::Resize991910 | In outline: resizes a list to n |
| `0x81dc30` | Array::Resize81DC30 | In outline: resizes a list to n |
| `0x744f60` | Array::Resize744F60 | In outline: resizes a list to n |
| `0x5d7260` | Array::Resize5D7260 | In outline: resizes a list to n |
| `0x96f7f0` | Array::Resize96F7F0 | In outline: resizes a list of 24-byte entries to n |
| `0x80c8b0` | Array::Resize80C8B0 | In outline: resizes a list of 24-byte entries to n |
| `0xcc4280` | Timer::CheckCC4280 | In outline: a step when a time step (Timer::StepCC2F10) differs from its value (+0x30) |
| `0x813280` | List::Remove813280 | In outline: removes entry n, moving the rest down |
| `0x620ac0` | Record::Make620AC0 | In outline: sets up a record with an empty text and cleared values |
| `0xcdc2a0` | List::FindCDC2A0 | In outline: a search over a sorted list |
| `0xcc5240` | Shader::MakeCC5240 | In outline: sets up a record with its table (`0xFC31FC`) |
| `0xc47380` | ShaderFile::HashC47380 | In outline: a step (`0xC4CDE0`) and a hash of four values |
| `0xbe6a60` | Table::InitBE6A60 | In outline: fills a table with 0xFFFF (`0x416110`), clears +0x28 and +0x3C, keeps its argument (+0x2C) |
| `0x47a150` | Text::Slot47A150 | In outline: a text for 0 to 0x17 (to a 512-character buffer) |
| `0x461080` | Array::Free461080 | In outline: frees an array of four-byte entries, tearing each down (`0x4012E0`) |
| `0xc5ec10` | Device::StepC5EC10 | In outline: clears a value (+0x80) and gives 1 when a check passes |
| `0xc1b1c0` | Table::GrowC1B1C0 | In outline: adds 0x18-byte records until there is one for a byte value (+0x11) |
| `0xbe2fd0` | List::CountBE2FD0 | In outline: counts a linked list (+0x30) |
| `0x734250` | Record::Make734250 | In outline: sets up a record with cleared bytes and a handle |
| `0x42c340` | Array::Free42C340 | In outline: frees each of its entries (+0x3C, +0x44) |
| `0x788030` | List::BinarySearch788030 | In outline: a binary search over a sorted list (+0x180) between two places |
| `0x4c9f20` | List::BinarySearch4C9F20 | In outline: the same over +0x174 |
| `0x47e9d0` | Array::Resize47E9D0 | In outline: resizes a list of 0xE0-byte entries to n |
| `0xcdeb10` | SortedPointers::GetOrAdd | In outline: gives the entry for a key when found (SortedPointers::Find64), else makes a 0x94-byte one |
| `0xcd5260` | Text::CompareCD5260 | In outline: compares two texts |
| `0xcd5150` | Text::CompareCD5150 | In outline: the same (another copy) |
| `0xc440c0` | Table::ReadDEFN | In outline: reads a block marked "DEFN" (0x4E464544) and two two-byte values (`0xBE5020`) |
| `0xbe9a60` | Memory::StepBE9A60 | In outline: a step with a count (+0x20) and sizes (+0x28, +0x2C, +0xC) |
| `0xa5d5e0` | Array::ResizeA5D5E0 | In outline: resizes a list to n |
| `0xa3e680` | Object::MakeA3E680 | In outline: when on (+0xC) with parts (+0x10, +0x1C), makes a 0x78-byte one |
| `0x464510` | Array::Resize464510 | In outline: resizes a list to n |
| `0x424540` | Array::Resize424540 | In outline: resizes a list to n |
| `0x438960` | Text::Scan438960 | In outline: goes over a C string |
| `0xc59040` | Stream::NameC59040 | In outline: a stream's name, empty when it has none (+0xC) |
| `0xc21b40` | Matrix::TransformC21B40 | Transforms a point by a matrix |
| `0xbbde40` | Track::MakeBBDE40 | In outline: fills a record from its arguments |
| `0xcd9730` | Vector::ConeCD9730 | In outline: a test against a cone around a direction (cross product against a scaled length) |
| `0xcafc70` | ResourceFile::NameCAFC70 | In outline: a name record when there is none (+0x34) |
| `0xcaf5d0` | ResourceFile::NameCAF5D0 | In outline: the same for +0x5C |
| `0xc36720` | Resource::FindC36720 | In outline: finds by a file name (ResourceFile::NameCAF330) in a list (+0x64) |
| `0x9fa5a0` | Fireworks::Count9FA5A0 | In outline: a count from entry n (0x24 bytes) |
| `0xcc1880` | Shader::CompareCC1880 | Compares two records by their first value, then by more when the shared flag (`0x13F7084`) is unset |
| `0xc5a0e0` | Vector::OntoC5A0E0 | In outline: a step with a point's place along a direction, kept in range |
| `0xcb6070` | Shader::InitCB6070 | In outline: sets values (+0x1C to the largest float, others to 0) and makes a four-byte buffer |
| `0xb82520` | Array::ResizeB82520 | In outline: resizes a list of 20-byte entries to n |
| `0x913ca0` | Array::Resize913CA0 | In outline: the same (another copy) |
| `0x90b4a0` | Array::Resize90B4A0 | In outline: the same (another copy) |
| `0x7e5400` | Array::Resize7E5400 | In outline: the same (another copy) |
| `0x4ba2c0` | Text::Once4BA2C0 | In outline: once (`0x12F4BC8`), makes a shared text record |
| `0xbfa1e0` | Text::FindBFA1E0 | In outline: when it has none (+0x28), finds one (Device::FindC5C7F0) |
| `0xc23470` | BoneManager::GetMatrixIDFromName | Gives a bone's matrix id from its name; reports "Calling BoneManager::GetMatrixIDFromName with an ID! You already HAVE an ID!" and gives -1 when handed a small number |
| `0xcafe00` | Zlib::InitCheck | In outline: zlib's stream start check: gives -6 (version error) unless the version text starts with '1' and the stream size is 0x38, -2 with no stream |
| `0xcbd4e0` | HashTable::InitCBD4E0 | In outline: sizes a table to a prime (Math::NextPrime), clears its count and makes its buckets |
| `0xc33450` | Text::FormatC33450 | In outline: formats a message by name (NamedList::Find) into a 4096-character buffer |
| `0xcb6b90` | Mesh::SizeCB6B90 | In outline: a buffer size from a vertex format (Vertex::SizeFromFormat), rounded up to 64 |
| `0xc18cf0` | Resource::MakeC18CF0 | In outline: sets up a record with its table (`0xFAFC60`) and a callback (`0xC167C0`) |
| `0xc01600` | Device::MakeC01600 | In outline: sets up a device record (0x254 bytes of work space) |
| `0xbd70b0` | Text::SumBD70B0 | In outline: adds up two-byte values |
| `0xcb1d50` | ShaderFile::ReadCB1D50 | In outline: reads a value from a buffer when there is room |
| `0xc089a0` | Timer::CountdownC089A0 | In outline: counts a shared value down (`0x13D3274`) and when it reaches 0, resets it (`0x10F2F70`) and acts |
| `0xcd3bb0` | Text::FindCD3BB0 | In outline: a text find |
| `0xcd3ab0` | Text::FindCD3AB0 | In outline: the same (another copy) |
| `0xcd3800` | Text::FindCD3800 | In outline: the same (another copy) |
| `0xcd3700` | Text::FindCD3700 | In outline: the same (another copy) |
| `0xca6bc0` | Table::TakeCA6BC0 | In outline: takes a free slot from a list (+0xA) or makes room |
| `0xc10840` | Resource::MakeC10840 | In outline: makes parts (Constructor on +4, +0x18) |
| `0xbfee90` | List::LoopBFEE90 | In outline: goes over a linked list (+0x24) |
| `0x8ffb90` | Array::Resize8FFB90 | In outline: resizes a list to n |
| `0x8f8bf0` | Array::Resize8F8BF0 | In outline: resizes a list to n |
| `0xc38810` | Record::MakeC38810 | In outline: makes a 0x2C-byte record |
| `0x5aef00` | Array::Free5AEF00 | In outline: frees an array of 0x1C-byte entries (+0x20), tearing each down |
| `0x46e6a0` | List::Pop46E6A0 | In outline: takes one off a count (+8) |
| `0xbcbf90` | Bezier::Point | In outline: a point on a cubic curve at t from four points (the 1-t and t cubes) |
| `0xc56a10` | System::VersionC56A10 | In outline: sets a shared flag (`0x10F31F8`) and reads the Windows version (OSVERSIONINFOA) |
| `0x457550` | File::Info457550 | In outline: reads data into a 2048-character buffer |
| `0x479530` | Text::ReadFloats | In outline: reads floats from texts ("%f", sscanf) |
| `0x4613f0` | Record::Make4613F0 | In outline: sets up a record with an empty text and cleared values |
| `0x438040` | Effect::Release438040 | In outline: releases a part (+0x1B0) and formats a 256-character name |
| `0xc05e00` | Device::StepC05E00 | In outline: when not flagged (+0x4C0), fills seven values |
| `0xabc4f0` | Array::ResizeABC4F0 | In outline: resizes a list to n |
| `0xa6b710` | Array::ResizeA6B710 | In outline: resizes a list to n |
| `0x939430` | Array::Resize939430 | In outline: resizes a list to n |
| `0x50fc40` | Array::Resize50FC40 | In outline: resizes a list to n |
| `0x4eed70` | Array::Resize4EED70 | In outline: resizes a list to n |
| `0x5d77a0` | Array::Resize5D77A0 | In outline: resizes a list to n |
| `0x5d7630` | Array::Resize5D7630 | In outline: resizes a list to n |
| `0x5d7530` | Array::Resize5D7530 | In outline: resizes a list to n |
| `0x5d73d0` | Array::Resize5D73D0 | In outline: resizes a list to n |
| `0xcbc640` | Array::ResizeCBC640 | In outline: resizes a list of eight-byte entries to n |
| `0xc63220` | Window::CaseC63220 | In outline: acts by a message's kind |
| `0xc50b70` | Stream::SetC50B70 | In outline: sets a value (+0x40) when it changes, with flag bit 11 handled |
| `0xc302c0` | Resource::ValueC302C0 | In outline: a value from its part (slot 0x14) and the shared list (`0x13E69BC`), 0 with none |
| `0xc06500` | Device::LoopC06500 | In outline: goes over a list (+0x24) reading four bytes from each |
| `0xbfd780` | Memory::StepBFD780 | In outline: a step with a count (+0x24) |
| `0xbd76c0` | Text::FindBD76C0 | In outline: a find by name (NamedList::Find2) |
| `0xce7df0` | Shapes::StepCE7DF0 | In outline: a step under a count (+0x1C) |
| `0xce5a80` | NamedObject::PartCE5A80 | In outline: sets up a record with its table (`0xFC7FAC`) |
| `0xc47500` | ShaderFile::FindC47500 | In outline: finds an entry by a hash of four values |
| `0xcc24d0` | Shader::DestroyAll | In outline: destroys entries one at a time (DestroyCC25D0) |
| `0xc30ec0` | Resource::PassC30EC0 | In outline: goes over the shared list (`0x13E69B8`, `0x13E69BC` entries) |
| `0x495720` | Text::Length495720 | In outline: a step with a C string's length |
| `0x446910` | List::Find446910 | In outline: a find over entries (+8) |
| `0xc73c20` | Debug::Output | In outline: sends a text to the debugger output (an output string) |
| `0xcba820` | Text::ReadNumberCBA820 | In outline: reads a number from a C string after skipping tabs and spaces |
| `0xc58f40` | Stream::NameC58F40 | In outline: a stream's name, empty when it has none (+0xC) |
| `0xc50390` | Stream::StepC50390 | In outline: a step with a part (+0xF0) |
| `0xbe3ee0` | Timer::StepBE3EE0 | In outline: with a shared part (`0x13D0E84`), a step against a time (Timer::Milliseconds) |
| `0xce2bb0` | Table::StepCE2BB0 | In outline: a step over entries |
| `0x87c030` | Array::Resize87C030 | In outline: resizes a list to n |
| `0x80fb70` | Array::Resize80FB70 | In outline: resizes a list to n |
| `0x445a00` | Array::Resize445A00 | In outline: resizes a list to n |
| `0xcbd2e0` | Text::MakeCBD2E0 | In outline: makes a text, empty when given nothing |
| `0x823ea0` | Array::Free823EA0 | In outline: frees an array, tearing each entry down |
| `0xbfcc20` | Device::MakeBFCC20 | In outline: fills a record from its arguments |
| `0xa544a0` | Record::MakeA544A0 | In outline: sets up a record with its table (`0xF8EA64`) and an empty text |
| `0xcef120` | Name::MakeCEF120 | In outline: makes a name text from a list |
| `0xcc91a0` | Name::MakeCC91A0 | In outline: the same (another copy) |
| `0xc47660` | ShaderFile::HashC47660 | In outline: a step (`0xC4CED0`) and a hash of twelve values modulo a size |
| `0xc331a0` | Resource::MakeC331A0 | In outline: makes a record |
| `0xbdcbd0` | VertexDeclaration::StepBDCBD0 | In outline: a step over bytes |
| `0x9f8770` | Math::Split9F8770 | In outline: splits a 64-bit value into parts |
| `0x7e70d0` | Array::Clear7E70D0 | In outline: clears a count (+0xC) and frees its buffer (+0x10) when it owns it |
| `0xcc4390` | Timer::StepCC4390 | In outline: a step with two values |
| `0xbe8ea0` | Resource::MakeBE8EA0 | In outline: fills a record from its arguments |
| `0x734160` | Path::Copy734160 | Copies a path record's values from another |
| `0xcb8000` | ShaderFile::SortCB8000 | In outline: a sort step over eight-byte pairs |
| `0x4467d0` | Effects::Find4467D0 | In outline: a find loop |
| `0xc34700` | Debug::AssertText | In outline: builds an "Assertion failure:" message in a 4096-character buffer |
| `0x670230` | Path::FileName670230 | In outline: the part of a path after its last '\' (WideString::FindLast, WideString::Substring) |
| `0x82a2a0` | Array::Resize82A2A0 | In outline: resizes a list to n |
| `0xaf7710` | Array::ResizeAF7710 | In outline: resizes a list to n |
| `0x860e60` | Array::Resize860E60 | In outline: resizes a list to n |
| `0x7335f0` | Array::Resize7335F0 | In outline: resizes a list to n |
| `0xb76170` | Array::ResizeB76170 | In outline: resizes a list to n |
| `0x63e1c0` | Array::Resize63E1C0 | In outline: resizes a list to n |
| `0x49b820` | Record::Copy49B820 | In outline: copies a record from another |
| `0xcdce20` | Plane::FromPoints | In outline: a plane's normal from three points (a cross product) |
| `0xcb5ec0` | Mesh::SetCB5EC0 | In outline: frees its buffer and fills values from its arguments |
| `0xc0aff0` | Text::StepC0AFF0 | In outline: for kind 7, when a check passes (`0xC09C50` with 8, 0x1D), counts up |
| `0xc0b0e0` | Text::StepC0B0E0 | In outline: for kind 0x1D, the same with 0x1D, 0xFFFF |
| `0xc4de80` | Shader::RegisterC4DE80 | In outline: registers shader names into a list (+0x2C) |
| `0xbfcdb0` | Device::MakeBFCDB0 | In outline: fills a record from its arguments |
| `0xcb7140` | Mesh::StepCB7140 | In outline: when its kind matches the shared one (`0x13E6F68`), a buffer step |
| `0xc75090` | Device::CountC75090 | In outline: a step with a count |
| `0xc2b5d0` | Text::FindC2B5D0 | In outline: a find by a C string (length worked out when -1) |
| `0xc04ac0` | Array::MakeC04AC0 | In outline: makes a list of n eight-byte entries |
| `0x40f990` | NamedObject::Make40F990 | In outline: makes an object (Object413) and fills it |
| `0xc695c0` | Window::StepC695C0 | In outline: when a check passes (`0xC65310`), a step with its parent's value |
| `0xc43000` | Resource::FindC43000 | In outline: a step (`0xC429E0`) and a find |
| `0xc0fcc0` | Resource::NameC0FCC0 | In outline: when given a name, a find over a list (+0x1C) |
| `0xc4ced0` | HashTable::GrowC4CED0 | In outline: when full, grows to the next prime size (Math::NextPrime, `0xC48A50`) |
| `0xc4cde0` | HashTable::GrowC4CDE0 | In outline: the same with `0xC48950` |
| `0xc09090` | Text::InitC09090 | In outline: a step (`0xC09020`), sets a size of 8000 and clears values |
| `0x9f8680` | Math::Split9F8680 | In outline: splits a 64-bit value into parts |
| `0x43a7e0` | Record::Make43A7E0 | In outline: makes a record |
| `0x497150` | Record::Make497150 | In outline: makes a part |
| `0xcdb2a0` | Vector::NearCDB2A0 | In outline: a test of a point's distance from another |
| `0xc42b10` | Record::MakeC42B10 | In outline: makes a 0x14-byte record |
| `0x9c5b80` | List::Clear9C5B80 | In outline: goes over a list |
| `0xc3d0e0` | ResourceFile::NameC3D0E0 | In outline: a name from a table of 0xC or 0x10-byte entries |
| `0xbf64e0` | Text::StepBF64E0 | In outline: a step with a value (+0x9C) over a list |
| `0xb3a0b0` | Text::MakeB3A0B0 | In outline: makes a text from a shared one (`0xF28E54`) |
| `0xc9b970` | Plane::FromPointsC9B970 | In outline: a plane's normal from three points (a cross product) |
| `0xc46b30` | ShaderFile::ReadC46B30 | In outline: reads four bytes and two two-byte values (`0xBE5020`) |
| `0xc36f50` | Resource::SameC36F50 | In outline: a step when given its own part (+0xA4) |
| `0xabee60` | Record::MakeABEE60 | In outline: sets up a record with handles |
| `0xa70400` | Window::ResetA70400 | In outline: clears a shared flag (`0x13429EC`) and goes over a 65-entry list |
| `0x48c7f0` | Record::Make48C7F0 | In outline: fills a record |
| `0xc668c0` | Window::MakeC668C0 | In outline: makes a window part (`0xC65F20`) |
| `0xc539c0` | Stream::MakeC539C0 | In outline: makes a 0xFC-byte part |
| `0xc4e120` | Shader::RegisterC4E120 | In outline: registers names into a list (+0x1C) |
| `0xc4e040` | Shader::RegisterC4E040 | In outline: the same (another copy) |
| `0x46cfb0` | Array::Resize46CFB0 | In outline: resizes a list to n |
| `0xc73950` | Device::MakeC73950 | In outline: sets up a record with its table (`0xFBA0BC`) and a name |
| `0xc385d0` | Resource::MakeC385D0 | In outline: makes a part from its argument, or the shared one (`0x13E6B24`) |
| `0xbd81c0` | HashTable::GrowBD81C0 | In outline: when full, grows to the next prime size (Math::NextPrime, `0xBD74D0`) |
| `0xbd0db0` | Text::MakeBD0DB0 | In outline: makes a text record |
| `0x9e1ab0` | Array::Resize9E1AB0 | In outline: resizes a list to n |
| `0x693300` | Array::Resize693300 | In outline: resizes a list to n |
| `0x61c320` | Array::Resize61C320 | In outline: resizes a list to n |
| `0x430690` | Array::Resize430690 | In outline: resizes a list to n |
| `0x409a50` | Record::Get409A50 | In outline: a value from an entry (Thunk) |
| `0xbe53c0` | Memory::StepBE53C0 | In outline: takes a reference and a step with the shared value (`0x13D0E98`) |
| `0xbd0080` | Text::MakeBD0080 | In outline: makes a text record |
| `0xcc5450` | Shader::StepCC5450 | In outline: a shader step (`0xCC4820`) |
| `0xc57b00` | Stream::MakeC57B00 | In outline: makes a record |
| `0xc550b0` | Stream::MakeC550B0 | In outline: makes a record |
| `0x981ee0` | Record::Make981EE0 | In outline: sets up a record with its table (`0xF7F6B8`) |
| `0x6b0370` | Window::Make6B0370 | In outline: sets up a record with its tables (`0xF60640`, `0xF60794`) |
| `0xc46300` | Table::WriteCDCL | In outline: writes a block marked "CDCL" (0x4C434443) with its count (+0x10) and entries (Writer::Write4) |
| `0xc9fad0` | Batches::BuildC9FAD0 | In outline: for a value of at least 0.5, builds batches (Batches::BuildC9EF70) |
| `0xbd2440` | List::LoopBD2440 | In outline: goes over a list (+0x4C) |
| `0x9cd960` | Record::Fill9CD960 | In outline: fills a record from its arguments |
| `0x9c1e10` | Window::Clear9C1E10 | In outline: clears a list (Array4::SetCapacity(0)) and acts with parts (+0xFC, +0x100) |
| `0x802330` | Load::Step802330 | In outline: a load step (`0x809430`) |
| `0x53c920` | Record::Make53C920 | In outline: makes a record |
| `0x42b4b0` | Record::Make42B4B0 | In outline: makes a 0x78-byte record |
| `0xcd3470` | Text::FindCD3470 | In outline: a find over a two-byte text |
| `0xcd3390` | Text::FindCD3390 | In outline: the same (another copy) |
| `0xcd39d0` | Text::FindCD39D0 | In outline: a text find |
| `0xcd3150` | Text::FindCD3150 | In outline: a find over a C string |
| `0xcd3070` | Text::FindCD3070 | In outline: the same (another copy) |
| `0xcd5370` | Text::CompareCD5370 | In outline: compares a text with a C string |
| `0xc63c70` | Record::MakeC63C70 | In outline: makes a 0x1C-byte record |
| `0xbe3740` | Text::StepBE3740 | In outline: 0 for an empty text |
| `0x98b360` | Record::Make98B360 | In outline: sets up a record with handles |
| `0xcb8630` | Resource::MakeCB8630 | In outline: makes a part from its argument, or the shared one (`0x13E6F98`) |
| `0xcb8420` | Resource::MakeCB8420 | In outline: the same with `0x13E6FA4` |
| `0xc5cc70` | Device::LoopC5CC70 | In outline: goes over a list (+0x1C) |
| `0x540610` | Text::Check540610 | In outline: a check with a 1024-character buffer |
| `0x403780` | Record::Make403780 | In outline: when not flagged (+0x24), makes parts |
| `0x403130` | Record::Make403130 | In outline: when not flagged (+0x1C), makes parts |
| `0xbd77c0` | Text::BraceBD77C0 | In outline: for a text whose second character is '{', makes a record |
| `0xbcc660` | Array::GrowBCC660 | In outline: makes a list one larger (0x24-byte entries) and copies the old one |
| `0xbc30f0` | Memory::StepBC30F0 | In outline: a step with a value (+0x40) |
| `0xb33e60` | Record::MakeB33E60 | In outline: sets up a record with its table (`0xF99120`) and a callback (`0xB32760`) |
| `0x455fd0` | Record::Make455FD0 | In outline: sets up a record with cleared values |
| `0x44a120` | List::Add44A120 | In outline: grows a list (double) when full and adds a 0x30-byte record |
| `0x40cbc0` | List::Loop40CBC0 | In outline: a loop over entry n (0x70 bytes) |
| `0xc96610` | Device::IndexC96610 | In outline: makes an index buffer the size of a sum (Text::SumBD70B0) |
| `0xc751b0` | Device::IndexC751B0 | In outline: the same (another copy) |
| `0xc6fc70` | Device::IndexC6FC70 | In outline: the same (another copy) |
| `0xbe4450` | Record::MakeBE4450 | In outline: sets up a record with its table (`0xF270D4`) and a slot of -1 |
| `0x968290` | List::Add968290 | In outline: adds an entry, growing (100 at first) when full |
| `0x9651d0` | List::Add9651D0 | In outline: the same with +0x10, +0x14 |
| `0x81dac0` | Record::Copy81DAC0 | Copies a record's values from another |
| `0x762860` | List::Copy762860 | In outline: copies a list |
| `0x49bae0` | List::Copy49BAE0 | In outline: the same (another copy) |
| `0xcc2b50` | Table::FreeSlotCC2B50 | In outline: finds a free slot among 0x400 |
| `0x851970` | Array::Make851970 | In outline: sets up three 0xC-byte entries |
| `0x808f40` | Load::Read808F40 | In outline: reads eight-byte values from a saved block (Load::ReadBytes) |
| `0xc060f0` | Record::InitC060F0 | In outline: sets up a record pointing at its own storage |
| `0xbe5140` | Module::StepBE5140 | In outline: a module step (ModuleList::Run, Error::ThrowIf) |
| `0xa39bb0` | Record::MakeA39BB0 | In outline: makes a record |
| `0x9d6f70` | Record::Make9D6F70 | In outline: sets up a record with an empty text |
| `0x809430` | Text::Make809430 | In outline: makes a text record |
| `0x4178d0` | Array::Resize4178D0 | In outline: resizes a list to n |
| `0xcc1ab0` | Shader::MakeCC1AB0 | In outline: fills a record from its arguments |
| `0xc70620` | Device::StagesC70620 | In outline: sets texture stages from a count up to 4 (slot 0x10C) |
| `0xbd33a0` | List::LoopBD33A0 | In outline: goes over entries (+0xC) |
| `0xc56d80` | Stream::MakeC56D80 | In outline: makes a 0x104-byte part |
| `0xbd0640` | Text::MakeBD0640 | In outline: makes a text record |
| `0xbd0350` | Text::MakeBD0350 | In outline: the same (another copy) |
| `0xbc9490` | Memory::StepBC9490 | In outline: a step with many values |
| `0x98b000` | Record::Make98B000 | In outline: sets up a record with handles |
| `0x9650e0` | List::Add9650E0 | In outline: adds an entry, growing (100 at first) when full |
| `0x42d400` | List::Pass42D400 | In outline: goes over entries (+8) |
| `0xcd3620` | Text::FindCD3620 | In outline: a text find |
| `0xcd3900` | Text::FindCD3900 | In outline: the same (another copy) |
| `0xcd1450` | Table::StepCD1450 | In outline: a step (`0xCD0C60`) |
| `0xcb2fa0` | ShaderFile::FindCB2FA0 | In outline: a find by a value (`0xCF6B63`) in a list (+0x24) |
| `0xcb2ec0` | ShaderFile::FindCB2EC0 | In outline: the same with +0x14 |
| `0xbffdc0` | Device::StepBFFDC0 | In outline: when not flagged (+0x21C), a step (slot 0x20) and 0 |
| `0xc463f0` | ShaderFile::CheckC463F0 | In outline: 0 unless a count matches (+8) |
| `0xbd62e0` | Record::MakeBD62E0 | In outline: sets up a record with its table (`0xFA5338`) |
| `0xcbe430` | Shader::InitCBE430 | In outline: clears values and an empty text |
| `0xc43d10` | Resource::LoopC43D10 | In outline: goes over entries (+0x24) |
| `0xc3b3c0` | Resource::SlotC3B3C0 | In outline: a step for slot n under a count (+0x2C) with a part (+0x6C) |
| `0xc0aee0` | Parser::MakeC0AEE0 | In outline: makes a parser node (Parser::Allocate(4, 0xC)) |
| `0xbd0ce0` | Text::MakeBD0CE0 | In outline: makes a text record |
| `0xcadb10` | ResourceFile::SlotCADB10 | In outline: a step for a non-negative slot |
| `0xc3ac00` | Resource::FindC3AC00 | In outline: a search over entries (+0x2C) |
| `0xc0d640` | Text::StepC0D640 | In outline: a step with two texts |
| `0xc0ba00` | Parser::MakeC0BA00 | In outline: sets up a node with its table (`0xFAFAAC`) |
| `0xce59b0` | NamedObject::PartCE59B0 | In outline: sets up a record with its table (`0xFC7FAC`) |
| `0xcc5380` | Shader::StepCC5380 | In outline: a shader step (`0xCC4820`, GetC) |
| `0xc5c060` | Device::CountC5C060 | In outline: a count over entries (+0x24) |
| `0xc57040` | Stream::MakeC57040 | In outline: makes a 0x12C-byte part |
| `0xc43840` | Array::GrowC43840 | In outline: grows a list of four-byte entries to hold n |
| `0xc21520` | Curves::StepC21520 | In outline: past a count (+0x34) with a value under 1, makes a record |
| `0x9d2910` | Record::Make9D2910 | In outline: makes a 16-byte record |
| `0x612250` | Game::Step612250 | In outline: a large step over a list of entries (+0xE4, +0xF4) by their flags, with a second list of numbers (+0xE8) counted down and up (+0xF0) |
| `0xcc96b0` | Shader::BuildCC96B0 | In outline: builds a 0x4C-byte shader record: a list of n values and 0x60-byte entries with empty names for each of its parts (+0x2C, +0x30) |
| `0xc0bed0` | Parser::InitC0BED0 | In outline: sets up a parser with its table (`0xFAFADC`) and empty lists |
| `0x4214f0` | Effect::Rebuild4214F0 | In outline: releases (Effect::Release420DA0) and swaps a shared counted part (+0x19C into +0x15C +0x114) |
| `0x4014b0` | Text::Copy4014B0 | In outline: makes its own copy of a shared text when shared |
| `0x445440` | Record::Copy445440 | Copies a record and its 30 entries from another |
| `0x9648f0` | Record::Copy9648F0 | Copies a record and its 16 entries from another |
| `0x933570` | Record::Make933570 | In outline: sets up a record with cleared values and handles |
| `0x84f2f0` | Record::Move84F2F0 | In outline: moves a record's values from another, clearing the other's (+0x10, +0x2C, slot +0x28 = -1) |
| `0x8e7e60` | Record::Copy8E7E60 | In outline: copies a record's values from another |
| `0x4263a0` | Record::Copy4263A0 | In outline: copies a record's values from another |
| `0x425b80` | Record::Copy425B80 | In outline: the same (another copy) |
| `0xc70770` | Record::CopyC70770 | In outline: the same (another copy) |
| `0x94d850` | Record::Copy94D850 | In outline: copies a record's values from another |
| `0x47e5a0` | Record::Copy47E5A0 | Copies five values (+0x10, +0xA0, +0xB0, +0xC0, +0xD0) from another record |
| `0x516ce0` | Record::Init516CE0 | In outline: sets up a record's values (an identity part, zeros) |
| `0x425a60` | Record::Init425A60 | In outline: sets up a record's values (zeros and -1.0) |
| `0xca6900` | Table::FreeCA6900 | In outline: puts slot n back on a free list |
| `0xc02060` | Record::InitC02060 | In outline: sets up a record with its table (`0xFA5EC0`) and empty lists |
| `0xbffb60` | Resource::RefBFFB60 | In outline: for a negative flag, makes a 0x34-byte resource reference (ResourceRefFA5D28) with the device's value (+0x45C) |
| `0xbe5020` | ShaderFile::Read | Reads n bytes: from the shared stream (`0x13D0E84`) when there is one, else opens the file (`0xC51360`) |
| `0x6a5e20` | Window::Make6A5E20 | In outline: makes a window part (Listener440) with its tables (`0xF4D8B8`, `0xF5F5A0`) |
| `0x416110` | Table::Fill16 | In outline: fills a table with sixteen two-byte values (the many 0xFFFF table fillers call it) |
| `0xcd5640` | Text::CompareCD5640 | In outline: compares two texts character by character |
| `0x442120` | Text::Scan442120 | In outline: goes over a two-byte text to its end |
| `0xc686c0` | Font::MakeC686C0 | In outline: makes a font record with Arial (Font::Arial) and its table (`0xFB7D58`) |
| `0xcc1f30` | Shader::MakeCC1F30 | In outline: makes a part (`0xCC1990`) |
| `0xc65f20` | Window::InitC65F20 | In outline: sets up a window record with its table (`0xFB7640`), cleared flags and an empty name |
| `0xbd3c60` | Device::FormatSizeBD3C60 | In outline: a size by format number (1: 0x1E, ..., 0x100: 0x50) |
| `0x805fd0` | Regions::Make805FD0 | In outline: makes a region record, with the shared one (`0x130AC04`) when given none |
| `0xbcc740` | Track::PointAt | In outline: for a distance between 0 and the length (+0xC), finds the point along a track's pieces |
| `0xc3b130` | Resource::BinarySearch | In outline: a binary search over 0x18-byte entries (+0x6C +0x10) by a value |
| `0x454930` | Record::Copy454930 | In outline: copies a record's values from another |
| `0x436b40` | Record::Copy436B40 | In outline: the same (another copy) |
| `0x94ccd0` | Record::Copy94CCD0 | In outline: the same (another copy) |
| `0x485690` | Record::Copy485690 | In outline: copies a record and takes a reference to its shared part |
| `0x403c20` | Text::Check403C20 | In outline: a check on a text, empty when given none |
| `0xbdadc0` | Static::Reset | In outline: clears a shared value (`0x13D0DF0`) and swaps the shared text (`0x13D0DF8`) for a new one (`0xBD05B0`) |
| `0xbd05b0` | Static::MakeText | In outline: makes a text, empty when given none |
| `0xbd1370` | List::ClearBD1370 | In outline: clears flags (+4) and a count (+8) and goes over entries |
| `0x40de60` | Record::Make40DE60 | In outline: makes a 0x1C-byte record |
| `0x40dd10` | Record::Make40DD10 | In outline: the same (another copy) |
| `0xbd34e0` | Node::Remove | In outline: unless a type list holds it (TypeLists::Contains), unlinks it (Node::Unlink) and clears a flag (+0x10) |
| `0xbd36c0` | Node::Detach | In outline: when on (+0x10), unlinks it (Node::Unlink) and goes over its list (+0x14) |
| `0xcdb7b0` | Math::Round | In outline: a value rounded down from a scaled sum |
| `0xbd9f00` | Text::Add | In outline: adds to a text (String::Append) |
| `0x7fbf50` | List::Add7FBF50 | In outline: grows two lists when full (`0x417C20`, `0x7FB7C0`) and adds |
| `0x7ce810` | Handle::Make7CE810 | In outline: makes a handle and links it |
| `0x7a90c0` | Handle::Make7A90C0 | In outline: the same (another copy) |
| `0x63ac10` | Window::Make63AC10 | In outline: with no value (+0x40), takes one (slot 8) and makes a 32-byte part |
| `0xbdb590` | Resource::Get | In outline: makes sure of a shared resource (Static::EnsureC374C0) and finds it (Resource::FindC36720) |
| `0x5d2970` | TrackedRide::Copy5D2970 | In outline: copies a value from another of the same kind (`0xF3AFB0`) |
| `0xbb83a0` | Track::InitBB83A0 | In outline: fills a record from its arguments |
| `0x40a6e0` | Text::Assign40A6E0 | In outline: lets go of a shared text and takes another |
| `0xc516d0` | Stream::LoopC516D0 | In outline: goes over a linked list |
| `0xbd2050` | Array::AddBlock | In outline: makes a block of n times a size and adds it to a list (`0xBD1FC0`) |
| `0x496610` | Record::Get496610 | In outline: a value (Thunk), else makes a 0xCC-byte one |
| `0xc96450` | Device::InitC96450 | In outline: sets up eight entries |
| `0x4b7940` | Text::Assign4B7940 | In outline: sets texts from a shared one (`0xF27994`) and a record's |
| `0xcebf80` | Record::InitCEBF80 | In outline: clears a record's values |
| `0xc50aa0` | Stream::StepC50AA0 | In outline: a step with flags (bits 2, 11) |
| `0x455040` | Effects::Entry455040 | In outline: a value from entry n (0x90 bytes) of kind 1 |
| `0xbd1800` | List::CopyBD1800 | In outline: sets up a list from another with its table (`0xFA14D0`) |
| `0x41cd80` | Text::Make41CD80 | In outline: with nothing set, makes a 0x1C-byte record |
| `0xc67710` | Window::AddChildC67710 | In outline: sets up a child, points it at its parent and tells it |
| `0xc5cc00` | Device::LinkC5CC00 | In outline: a step with a linked part |
| `0xbe45e0` | Module::MatchBE45E0 | In outline: a module step (ModuleList::Run) and a path match (Path::MatchC17580) |
| `0x47e120` | Record::Copy47E120 | In outline: copies a record's values from another |
| `0xc5cb90` | Device::LinkC5CB90 | In outline: a step with a linked part |
| `0xbdaa50` | Resource::Get2 | In outline: makes sure of a shared resource (Static::EnsureC374C0) |
| `0x9c5d50` | Record::Get9C5D50 | In outline: a value (Thunk), else makes a 0x38-byte one |
| `0xbdd180` | VertexDeclaration::FindBDD180 | In outline: a find over elements |
| `0xcc6200` | Record::CopyCC6200 | In outline: copies values into its part (+4) |
| `0xbe40d0` | Module::CheckBE40D0 | In outline: when on (+0xC), a check by a part's flags (bits 1, 13) |
| `0xc699c0` | Window::MakeC699C0 | In outline: makes a window part (`0xC69910`) |
| `0xc66d30` | Window::MakeC66D30 | In outline: sets up a window record (Window::InitC65F20) with its table (`0xFB7B98`) |
| `0xbc6d00` | Track::CheckBC6D00 | In outline: for rides flagged 0x400000 with more than one station and both ends set, a step |
| `0x9e8290` | Record::Copy9E8290 | In outline: copies a record's values from another |
| `0x7cd120` | Record::Make7CD120 | In outline: sets up a record with its tables (`0xF70DE0`) |
| `0xce3250` | Table::RemoveCE3250 | In outline: removes an entry from an index (+0xC) and a list (+0x38) |
| `0xce3100` | Table::RemoveCE3100 | In outline: the same with +8 and +0x14 |
| `0x4548d0` | Record::Init4548D0 | In outline: sets a record's values (0, 1, zeros) |
| `0xbca7e0` | Device::CaseBCA7E0 | In outline: a value by case (1, 0x13, 0x2A, ...) |
| `0x7b5070` | Effect::Next7B5070 | In outline: in mode 1, a step with the next entry |
| `0x7b5010` | Effect::Next7B5010 | In outline: a step with the next entry |
| `0x45eae0` | Animal::Pass45EAE0 | In outline: when not flagged (+0x884), goes over entries (+0x18) |
| `0xc675a0` | Window::DetachC675A0 | In outline: with a parent (+0x18), a step with a linked part (+0x74) |
| `0xbe5360` | Module::LoopBE5360 | In outline: with the shared list (`0x13D0E90`) and flag (`0x10F2CD8`), goes over it |
| `0xbdc880` | Table::SumBDC880 | In outline: goes over a table (+0xC to +0x20C) |
| `0xa1ce30` | List::LoopA1CE30 | In outline: goes over a linked list |
| `0x6e3ae0` | Window::Free6E3AE0 | In outline: frees a buffer (+0x7C) and clears it and a count (+0x80) |
| `0x40c300` | Model::Part40C300 | In outline: for part n (0x24 bytes) with nothing set, a step (`0xCC63E0`) |
| `0xbf5bf0` | Text::LinkBF5BF0 | In outline: a step with a list (+0x34) |
| `0x985fe0` | Table::Init985FE0 | In outline: fills 16 entries with the largest float |
| `0x633b30` | Record::Init633B30 | In outline: sets up a record with its table (`0xF47F0C`) |
| `0xc5e410` | Window::FadeC5E410 | In outline: clears a flag (+0x9C) and sets a value from 1 minus another (+0xDC) |
| `0xc5cb30` | Device::LinkC5CB30 | In outline: a step with a linked part |
| `0xc307d0` | Resource::ValueC307D0 | In outline: a value from entry n (+0x28, 16 bytes) |
| `0xbd31f0` | Node::SetBD31F0 | In outline: clears a flag (+0x98) and a step (`0xBD2960`), or sets it |
| `0x7defa0` | Person::Find7DEFA0 | In outline: a find with a list (+0x270) |
| `0x498630` | Effects::Point498630 | In outline: a step with points and a value (+0x24) |
| `0x495f50` | Static::Step495F50 | In outline: resets (Static::Reset) and makes a part |
| `0x417d70` | List::Init417D70 | In outline: sets up a list with room for 16 |
| `0xcdf110` | Table::FindCDF110 | In outline: a find over a shared table (`0x1400778`) |
| `0xcd3550` | Text::FindCD3550 | In outline: a text find |
| `0xcc4c50` | Shader::ScanCC4C50 | In outline: goes over bytes until seven matches |
| `0xc4ddb0` | Shader::RegisterC4DDB0 | In outline: registers names into a list (+0x3C) |
| `0xc04bb0` | Device::CaseC04BB0 | In outline: a value by case (0x13: +0x4F4, ...) |
| `0xbf47d0` | Text::InitBF47D0 | In outline: sets up a record with its table (`0xFA589C`) |
| `0xc52120` | Stream::StepC52120 | In outline: a step with two values |
| `0xc4df70` | Shader::RegisterC4DF70 | In outline: registers names into a list (+0x1C) |
| `0xc256f0` | Array::GrowC256F0 | In outline: grows a list of four-byte entries to hold n |
| `0xbd2fc0` | List::StepBD2FC0 | In outline: a step over entries (+0x24) |
| `0xc9ed10` | Batches::InitC9ED10 | In outline: sets up a record (`0xCA4480` with +0x38) |
| `0xc5bec0` | Device::CountC5BEC0 | In outline: a count over entries (+0x28) |
| `0xc11010` | Resource::MakeC11010 | In outline: makes a part with its table (`0xFAF9BC`) |
| `0xbd2d70` | Table::InitBD2D70 | In outline: sets up four entries |
| `0xcd4cc0` | Text::FindCD4CC0 | In outline: finds a two-byte text in another |
| `0xbe03a0` | Text::ValueBE03A0 | In outline: a value from a shared table (`0x10F2BA8`) by a byte, and a C string |
| `0x7be220` | Text::Set7BE220 | In outline: a step with a text's length (Text::Length495720) under a count |
| `0x735b70` | Path::Make735B70 | In outline: makes a pooled object (PooledObject) |
| `0x45c250` | Effect::Name45C250 | In outline: with nothing set (+0x300), sets a name |
| `0x417570` | Regions::Set417570 | In outline: sets a region (Regions::Set417180) and acts |
| `0xcf0330` | Shapes::StepCF0330 | In outline: a step over entries |
| `0xcdf760` | Loop::StepCDF760 | In outline: a loop down from n |
| `0xbb9cb0` | Track::InitBB9CB0 | In outline: sets up a record pointing at its own storage |
| `0xcc4140` | Shader::InitCC4140 | In outline: sets up a record with its table (`0xFC31F4`) |
| `0xc194c0` | Resource::MakeC194C0 | In outline: makes a record with its table (`0xFAFDB0`) |
| `0xbbd580` | Track::StepBBD580 | In outline: a step (`0xBC28D0`), with 0xFFFF handled |
| `0xcdf690` | Loop::StepCDF690 | In outline: a loop down from n (Loop::StepCDF4C0) |
| `0xc90e70` | Device::CheckC90E70 | In outline: with no part (+0x2144), a check by flags |
| `0xc5dae0` | Window::DrawC5DAE0 | In outline: a draw step with a text (0x2B) |
| `0xbf5a90` | Text::StepBF5A90 | In outline: when not flagged (+0xED), a step (`0xBF4B20`) |
| `0xcc4f80` | Shader::CompareCC4F80 | Compares two records by their first value, then more |
| `0xcc3eb0` | Shader::CompareCC3EB0 | The same (another copy) |
| `0xc6fb70` | Device::CountC6FB70 | In outline: a step with a count |
| `0xc05080` | Device::SetC05080 | In outline: sets values (+0x4E0, +0x4E4) and acts |
| `0x98d6f0` | Record::Make98D6F0 | In outline: makes a 0x90-byte record |
| `0x438b20` | Effect::Step438B20 | In outline: a step with many values |
| `0xc160b0` | Resource::MakeC160B0 | In outline: makes a record |
| `0xc051f0` | Device::StepC051F0 | In outline: when on (+0x4C0, +0x4C1), a step (slot 0x50) |
| `0xbf92c0` | Text::StepBF92C0 | In outline: a step with many values |
| `0x43ab10` | Effect::Find43AB10 | In outline: a find with a value (+0x40) and a mode (+0x3C) |
| `0xc4ff20` | File::WriteC4FF20 | In outline: writes a buffer to a file (WriteFile) |
| `0xcb03b0` | Zlib::EndCB03B0 | In outline: zlib's stream end: gives -2 (stream error) with no stream or state (+0x1C), else frees it |
| `0xbc0d10` | Debug::CheckBC0D10 | In outline: stops in the debugger (breakpoint) when a value is set (+0x80) |
| `0xc324f0` | Text::FormatC324F0 | In outline: formats into a 128-character buffer |
| `0xc65b90` | Window::StepC65B90 | In outline: a step with many values |
| `0x433dd0` | Sky::Make433DD0 | In outline: makes parts |
| `0x4042f0` | Effect::Step4042F0 | In outline: a step with many values |
| `0x98b1b0` | Record::Make98B1B0 | In outline: sets up a record with handles |
| `0x98b0e0` | Record::Make98B0E0 | In outline: the same (another copy) |
| `0x946730` | Water::Find946730 | In outline: a find |
| `0xcc6310` | Model::Part2CC6310 | In outline: a step for part n |
| `0xcb6ac0` | ShaderFile::ReadHeaderCB6AC0 | In outline: reads four-byte values (ShaderFile::Read into +0x34, +0x40, ...) |
| `0xbdfbb0` | HashTable::ResizeBDFBB0 | In outline: sizes a table to a prime (Math::NextPrime) and puts the entries back |
| `0xbdfae0` | HashTable::ResizeBDFAE0 | In outline: the same (another copy) |
| `0xbbbd60` | List::PassBBBD60 | In outline: goes over entries (+0x1C +4) |
| `0xb81800` | List::AddB81800 | In outline: grows a list by one (ObjectArray::SetCapacity on +0x11C) and a handle |
| `0x834190` | Scenery::Make834190 | In outline: makes a 32-byte part (`0xBCEE80`) |
| `0xcc5700` | Shader::MakeCC5700 | In outline: makes a part |
| `0xcc4da0` | Shader::MakeCC4DA0 | In outline: the same (another copy) |
| `0xc05c50` | Device::StepC05C50 | In outline: when not flagged (+0x4C0), a step with the shared value (`0x13D317C`) |
| `0xc04110` | Device::StepC04110 | In outline: a step (slot 0xC) and a loop |
| `0xc37680` | Resource::MakeC37680 | In outline: makes a part, with none handled |
| `0xc2f9d0` | Resource::LoopC2F9D0 | In outline: goes over entries (+4) |
| `0xc1f960` | Skeleton::StepC1F960 | In outline: a step with points |
| `0x804590` | Regions::Init804590 | In outline: sets up a record with the shared count (`0x130AC08` +0x14) |
| `0x725a20` | Path::Clear725A20 | In outline: tears down parts |
| `0x5bb130` | TrackPiece::Init5BB130 | In outline: sets a value of 10 and a step (`0x611A80` with 1, 200) |
| `0x9d7090` | Record::Copy9D7090 | In outline: copies a record's values from another |
| `0x982660` | Record::Get982660 | In outline: a value (Thunk), made when none |
| `0xcd4880` | Text::FindCD4880 | In outline: finds a C string in another |
| `0xc33670` | Text::FormatC33670 | In outline: formats into a 128-character buffer |
| `0x9f3fd0` | Record::Init9F3FD0 | In outline: sets up a record with its table (`0xF867B4`) |
| `0xc434f0` | Resource::InitC434F0 | In outline: sets up a record with its table (`0xFB12A4`) |
| `0xc2f910` | Resource::MakeC2F910 | In outline: a step (`0xCA68B0`) and makes a part |
| `0xc18640` | Path::InitC18640 | In outline: sets up a record with its table (`0xFAFC8C`) |
| `0xc18490` | Path::InitC18490 | In outline: the same with `0xFAFC88` |
| `0x877f30` | Record::Copy877F30 | In outline: copies a record and takes a reference to its shared part |
| `0x753fe0` | Guest::Make753FE0 | In outline: fills a record from its arguments |
| `0x426bc0` | Sky::Make426BC0 | In outline: makes a window part |
| `0xcd7d00` | Camera::InitCD7D00 | In outline: sets values to the largest float and more |
| `0xc37d70` | Resource::LastC37D70 | In outline: a step with the last entry (+4, +8) |
| `0xc07a90` | Device::StepC07A90 | In outline: a step with a work buffer |
| `0x809800` | Regions::Count809800 | In outline: a step for fewer than two entries |
| `0x501a90` | Ride::Init501A90 | In outline: sets up a record with cleared values |
| `0x4f05b0` | Ride::Make4F05B0 | In outline: makes a part |
| `0x49b930` | Static::Pass49B930 | In outline: goes over entries with the shared count (`0x1106C54`) |
| `0x470300` | Device::Step470300 | In outline: takes a value from the device (slot 0xC0) and acts |
| `0x42c490` | Sky::Find42C490 | In outline: a find over a list (+0x20) |
| `0xc5bd50` | Device::StepC5BD50 | In outline: a step with the device's part (+0x10 +0x24C) and a value (+0x1C) |
| `0xc16e90` | Resource::InitC16E90 | In outline: sets up a record with its table (`0xFAFC60`) and a callback (`0xC167C0`) |
| `0xc0e8e0` | Resource::InitC0E8E0 | In outline: sets up a record with its tables (`0xFAFB0C`, `0xFAFB08`) |
| `0xbdfe60` | Ref::StepBDFE60 | In outline: adds one to a count and acts |
| `0xce3040` | Buffer::FreeCE3040 | In outline: frees an aligned buffer (+8) and more |
| `0xc58560` | Device::CountC58560 | In outline: 0 with no part (+0x44), else a count |
| `0xc38440` | Resource::LastC38440 | In outline: a value from the last entry |
| `0xbc8970` | Track::MinBC8970 | In outline: keeps a shared value (`0x10F2B30`) below a limit |
| `0x7f11a0` | Person::Value7F11A0 | In outline: a value from an object (slot 0x90) |
| `0x497240` | Static::Last497240 | In outline: a step with the last entry (+0x81C) |
| `0x45f980` | Text::Init45F980 | In outline: sets up a record with empty texts (WideString::Empty) |
| `0x40cca0` | Effect::Entry40CCA0 | In outline: a step for entry n (0x70 bytes) |
| `0xcf01e0` | Shapes::InitCF01E0 | In outline: fills a record with its arguments and the shared value (`0x10F2FA0`) |
| `0xc54720` | Device::SetC54720 | In outline: sets a shared flag (`0x13E6C60` = 1) and acts |
| `0xc044e0` | Device::StepC044E0 | In outline: a step, with none given handled |
| `0x7f10e0` | Person::Value7F10E0 | In outline: a value from an object (its first slot via +0x48) |
| `0x79f970` | Terrain::Make79F970 | In outline: fills a record from an object (slot 0x18) and its arguments |
| `0xcc2360` | Shader::FreeAllCC2360 | In outline: tears down each entry it finds (`0xCC25D0`) until none is left |
| `0xcb8860` | Resource::MakeCB8860 | In outline: makes a part, with none handled |
| `0xc69cf0` | Window::InitC69CF0 | In outline: fills a record from many values |
| `0xbd95b0` | Memory::TakeBD95B0 | In outline: takes room from the shared pool (`0x13D0DB0`) |
| `0xbafe30` | Track::HandleBAFE30 | In outline: makes a handle and links it |
| `0xcbdaf0` | Memory::StepCBDAF0 | In outline: with the shared pool (`0x13F7014`), frees a part (+0x24) |
| `0xcb8a20` | Resource::MakeCB8A20 | In outline: makes a part, with none handled |
| `0xca2550` | Shared::MakeCA2550 | In outline: once (`0x13E6F20`), makes a four-byte record |
| `0xc3a560` | Resource::StepC3A560 | In outline: a step (`0xC3A4B0`) and more |
| `0xbf54d0` | Text::PassBF54D0 | In outline: goes over entries (+4) |
| `0xba2ef0` | Track::HandleBA2EF0 | In outline: makes a handle and links it |
| `0xb99db0` | Track::CheckB99DB0 | In outline: a check with two answers |
| `0xce7d30` | Camera::LimitCE7D30 | In outline: lowers a value (+0x20) to n when n is smaller |
| `0xcbd220` | Shader::CheckCBD220 | In outline: a check with none handled |
| `0xcb2700` | Model::WriteHeaderCB2700 | In outline: writes a "D3DT" tag and four-byte values (Writer::Write4) |
| `0xca6560` | Resource::NameCA6560 | In outline: a text from a name buffer (+0x448) |
| `0xc61140` | Device::StepC61140 | In outline: when not flagged (+0x9C), a step by flags (0x180) |
| `0xc3c6d0` | Resource::EntryC3C6D0 | In outline: entry n under a count (+0x50) |
| `0xc3c610` | Resource::EntryC3C610 | In outline: the same with +0x58 |
| `0xc3c790` | Resource::EntryC3C790 | In outline: entry n under a count (+0x2C) |
| `0xc0db10` | Text::FindC0DB10 | In outline: a two-byte text from slot n (+8), or a given one |
| `0xc0da10` | Text::FindC0DA10 | In outline: the same (another copy) |
| `0xc0d880` | Text::FindC0D880 | In outline: the same (another copy) |
| `0xc0d780` | Text::FindC0D780 | In outline: the same (another copy) |
| `0xbdcf60` | VertexDeclaration::StepBDCF60 | In outline: with no part (+0xC), a loop (VertexDeclaration::LoopBDCE30) |
| `0xca3750` | Resource::InitCA3750 | In outline: fills a record from an object and its arguments |
| `0xc3aff0` | Resource::StepC3AFF0 | In outline: a step for n under a count (+0x2C), not in mode 1 (+0x60) |
| `0xbfdef0` | Record::InitBFDEF0 | In outline: fills a record from two values and clears a two-byte value |
| `0xbea2d0` | Record::InitBEA2D0 | In outline: the same (another copy) |
| `0xbeb390` | Device::MakeBEB390 | In outline: makes a buffer of n times m with the device (+4 +0x24C) |
| `0xc66410` | Window::StepC66410 | In outline: a step when a value changes (+0xE8, +0xF0) |
| `0xc311c0` | Stream::AddC311C0 | In outline: adds n to a count (+0x1C) and acts |
| `0xc03c60` | Device::StepC03C60 | In outline: a step with cleared values |
| `0xbd6410` | ShaderFile::ReadBD6410 | In outline: reads four-byte values (ShaderFile::Read) |
| `0x463880` | Record::Init463880 | In outline: sets up a record with its table (`0xF2A87C`) |
| `0x44c7e0` | Vector::Length44C7E0 | In outline: a step with a point's length |
| `0x409cf0` | List::Find409CF0 | In outline: a find over entries (+0xC, +0x18) |
| `0x45e0e0` | Library::Free45E0E0 | In outline: frees a loaded library (FreeLibrary on +0x888) and more |
| `0x471f20` | List::Add471F20 | In outline: grows a list by two when full (`0x4710B0`) and adds |
| `0x42c550` | Sky::Find42C550 | In outline: a find over a list (+0x20) |
| `0x41c660` | Record::Check41C660 | In outline: a check with none handled |
| `0x6209c0` | Window::Check6209C0 | In outline: a check with none handled |
| `0x612170` | Record::Init612170 | In outline: sets up a record with its tables (`0xF42AC0`, `0xF42AAC`) |
| `0x456140` | List::Find456140 | In outline: a find over entries (+4) |
| `0x443a20` | Text::Format443A20 | In outline: formats into a 32-character buffer |
| `0xc3ba90` | Resource::StepC3BA90 | In outline: a step with a part (+0x6C) |
| `0xbff0e0` | Resource::InitBFF0E0 | In outline: sets up a record with its table (`0xFA5CD4`) |
| `0xbeb7c0` | Resource::InitBEB7C0 | In outline: sets up a record with its table (`0xFA577C`) |
| `0xbc67b0` | Track::CountBC67B0 | In outline: a count over entries |
| `0x475ae0` | Text::Step475AE0 | In outline: a step with a work buffer |
| `0xc934b0` | Batches::InitC934B0 | In outline: fills a record from its arguments |
| `0xc586b0` | Device::CountC586B0 | In outline: 0 with no part (+0x44), else a count |
| `0x403210` | Static::Reset403210 | In outline: when on (+0x1C), resets (Static::Reset) |
| `0x403020` | Static::Reset403020 | In outline: when on (+0x24), resets (Static::Reset) |
| `0xcba910` | Text::ParseNumberCBA910 | In outline: reads a number from a C string into a 64-bit value |
| `0xcafb20` | Resource::TextCAFB20 | In outline: makes a text |
| `0xc34340` | Resource::MakeC34340 | In outline: makes a 0x110-byte part |
| `0xc33c00` | Resource::MakeC33C00 | In outline: a step (`0xC326A0`) and makes a 0x110-byte part |
| `0xc03fb0` | Device::ValueC03FB0 | In outline: a value from the device (slot 0x20) |
| `0xbfed80` | Device::MakeBFED80 | In outline: makes a buffer of n times m with the device (+4 +0x24C) |
| `0xbebbd0` | Resource::InitBEBBD0 | In outline: fills a record from its arguments |
| `0xbb7580` | Track::PassBB7580 | In outline: goes over a part's entries (+0x10 +0x24) |
| `0xcb81f0` | Resource::MakeCB81F0 | In outline: makes a part, with none handled |
| `0xc9e5e0` | Batches::ReleaseC9E5E0 | In outline: takes one from a count (+0x68) and acts |
| `0xc64100` | Window::StepC64100 | In outline: unless flagged (0x80000), a step (`0xC5E300`) |
| `0xc20cf0` | Resource::StepC20CF0 | In outline: a step (`0xCA3540`) and more |
| `0xc0dc80` | Text::InitC0DC80 | In outline: sets up a record with its table (`0xFAFAAC`) and a text |
| `0xbeb6e0` | Resource::InitBEB6E0 | In outline: fills a record from its arguments |
| `0xbb6880` | Track::PassBB6880 | In outline: goes over entries (+0x18) |
| `0xcc90f0` | Ref::AssignCC90F0 | In outline: takes a reference to another's part when it differs |
| `0xc5fa90` | Window::ScaleC5FA90 | In outline: a value scaled by a shared value (`0x13E6D00` +0x194) |
| `0xc03a90` | Device::MakeC03A90 | In outline: makes a 16-byte part |
| `0xc01020` | Device::CountC01020 | In outline: a count |
| `0xbe4530` | Error::InitBE4530 | In outline: sets up a record with its table (`0xF270D4`) and -1 |
| `0xbd1620` | Record::InitBD1620 | In outline: sets up a record with its table (`0xFA14D0`) and a value (+0x24) |
| `0xb96e10` | Record::InitB96E10 | In outline: sets up a record with its table (`0xF28EA8`) |
| `0xcb6fb0` | Shader::PassCB6FB0 | In outline: goes over entries (+0x14) |
| `0xca5dc0` | Table::CheckCA5DC0 | In outline: throws an error (0x2A02) when bits do not match |
| `0xc07180` | Device::ClearC07180 | In outline: empties a list (+0x24) |
| `0xbe2030` | Record::InitBE2030 | In outline: fills a record from a value |
| `0x495220` | Text::Parse495220 | In outline: goes over a C string |
| `0xced190` | Shapes::StepCED190 | In outline: a step with a value (+0x58) |
| `0xcd32e0` | Text::CompareCD32E0 | In outline: compares two-byte texts |
| `0xcbd3e0` | String::StepCBD3E0 | In outline: a step with the shared empty string (String::SharedEmpty) |
| `0xc572c0` | Device::StepC572C0 | In outline: for -1, unless a shared flag is set (`0x10F31F8`), a step |
| `0xc56f00` | Device::MakeC56F00 | In outline: sets a shared flag (`0x13E6C60` = 1) and makes a 0x3C-byte part |
| `0xc65ae0` | Window::CheckC65AE0 | In outline: a check with none handled |
| `0xbeba10` | Resource::InitBEBA10 | In outline: sets up a record with its table (`0xFA5798`) |
| `0xbd1750` | Record::InitBD1750 | In outline: sets up a record with its table (`0xFA14D0`) and a value |
| `0xce9710` | Camera::SetCE9710 | In outline: sets a value from another when it differs |
| `0xca5800` | Table::FindCA5800 | In outline: a find from a point below a limit (+0x20) |
| `0xc3a4b0` | Resource::StepC3A4B0 | In outline: a step with a list (+0x14) |
| `0xc2fc30` | Table::EntryC2FC30 | In outline: for a nonzero slot, a two-byte entry from a table |
| `0xc18550` | Path::CopyC18550 | In outline: sets up a record with its table (`0xFAFC8C`) from another |
| `0xc0fb50` | Parser::MakeC0FB50 | In outline: makes a part (Parser::Allocate 4, 0x44) |
| `0xbf9e30` | Text::StepBF9E30 | In outline: a step with work buffers |
| `0xbc6940` | Track::CountBC6940 | In outline: a count over entries |
| `0xbb5a60` | Track::CountBB5A60 | In outline: the same (another copy) |
| `0x4977e0` | Static::Make4977E0 | In outline: makes a part |
| `0x418e40` | Effects::Find418E40 | In outline: a find with an entry (Thunk) |
| `0xce3160` | Table::SetCE3160 | In outline: removes a slot (Table::RemoveCE3100) and acts, -1 handled |
| `0xcbdbb0` | Shader::CheckCBDBB0 | In outline: a step with none handled |
| `0xcb7740` | Mesh::FreeCB7740 | In outline: a mesh step (Mesh::StepCB7060) and takes one from a shared count (`0x13E6F6C`) |
| `0xc62cf0` | Window::StepC62CF0 | In outline: a step (`0xC622D0`) with values |
| `0xc36b00` | Resource::MakeC36B00 | In outline: makes a part |
| `0xc0efe0` | Resource::InitC0EFE0 | In outline: sets up a record with its tables (`0xFAFB24`, `0xFAFB20`) |
| `0xbe5720` | Shared::MakeBE5720 | In outline: once (`0x13D0E78`), makes a 0x40-byte record |
| `0x9d0ce0` | Ref::Assign9D0CE0 | In outline: takes a reference to another's part when it differs |
| `0x9c5660` | Record::Copy9C5660 | In outline: copies a record from another |
| `0xc57950` | Engine::MakeC57950 | In outline: makes a 0x10-byte part |
| `0x41ca00` | Engine::Make41CA00 | In outline: makes a 0x10-byte part |
| `0x410610` | Engine::Handle410610 | In outline: makes a handle and links it |
| `0x438a70` | Engine::Make438A70 | In outline: makes a 0x10-byte part |
| `0xcd2470` | Engine::MakeCD2470 | In outline: makes a 0x10-byte part |
| `0xcb7230` | Engine::PassCB7230 | In outline: goes over entries (+0x50) |
| `0xc57760` | Engine::MakeC57760 | In outline: makes a 0x10-byte part |
| `0xc56e60` | Engine::MakeC56E60 | In outline: makes a 0xF8-byte part |
| `0xcd2520` | Engine::MakeCD2520 | In outline: makes a 0x10-byte part |
| `0xcafbd0` | Engine::MakeCAFBD0 | In outline: makes a 0x10-byte part |
| `0xc008d0` | Engine::PassC008D0 | In outline: goes over entries (+0x164) |
| `0xcd3230` | Text::CompareCD3230 | In outline: compares two-byte texts |
| `0xcd2fc0` | Text::CompareCD2FC0 | In outline: compares C strings |
| `0xcd2f10` | Text::CompareCD2F10 | In outline: the same (another copy) |
| `0xc5d840` | Window::StepC5D840 | In outline: a step (slot 0x74) with a point |
| `0xc0bc90` | Text::CaseC0BC90 | In outline: for a kind between 8 and 0x1C, a step |
| `0xbee5b0` | Vector::StepBEE5B0 | In outline: scales a point (Vector::ScaleBEE2C0) and more |
| `0x475260` | Math::Step475260 | In outline: rounds (Math::Round) and acts with a shared value (`0x12F00B4`) |
| `0xbdc9b0` | VertexDeclaration::StepBDC9B0 | In outline: with no part (+0xC) and a count, a step (slot 0xF8) |
| `0xbd5250` | Device::ValueBD5250 | In outline: a value from the device (slot 0xB4) |
| `0xb78c20` | Ref::ReleaseB78C20 | In outline: takes one from a count (+8) and acts |
| `0x446360` | Record::Init446360 | In outline: sets up a record with its table (`0xF28EA8`) |
| `0xc5e300` | Window::ScaleC5E300 | In outline: a step (slot 0x70) with a value scaled by a shared value (`0x13E6D00` +0x194) |
| `0xcefdd0` | Shapes::EntryCEFDD0 | In outline: a value for entry n (16 bytes) |
| `0x45b190` | Record::Copy45B190 | In outline: copies a record from another |
| `0x430f70` | Sky::Entry430F70 | In outline: for entry n (0x30 bytes) under a count (+0x4C), a step |
| `0xcb69e0` | ShaderFile::ReadCB69E0 | In outline: reads four-byte values (ShaderFile::Read) |
| `0xca5a10` | Table::CheckCA5A10 | In outline: a step when n is outside a range (+0x38, +0x3C) |
| `0x454c00` | Device::Step454C00 | In outline: a step with the device (slot 0x14 with 1) |
| `0x454b50` | Device::Step454B50 | In outline: the same (another copy) |
| `0x42bff0` | Sky::Next42BFF0 | In outline: the next entry while under a count (+0x4C, +0x50) |
| `0x42bf40` | Sky::Next42BF40 | In outline: the same (+0x40, +0x44) |
| `0x4185c0` | Effects::Init4185C0 | In outline: sets up a record with its table (`0xF270D8`) and a listener (Listener440) |
| `0xc62da0` | Window::CaseC62DA0 | In outline: a step by kind, 8 handled |
| `0xc1c220` | Resource::PartC1C220 | In outline: with a part (+0x1C), a step |
| `0xc09a20` | Text::StepC09A20 | In outline: with a part (+4), a step (Get8Then40) |
| `0xcb2510` | Buffer::SetCB2510 | In outline: sets a size when it changes, 0 handled |
| `0xc38a10` | Resource::FindC38A10 | In outline: a find over a list (+0x14) |
| `0xbe8430` | Math::StepBE8430 | In outline: a 64-bit step |
| `0xbc2ce0` | Track::FreeBC2CE0 | In outline: frees (Memory::FreeCounted) and clears a flag (+0x18) |
| `0xcc2250` | Ref::ReleaseCC2250 | In outline: takes one from a count (+4) and frees at zero |
| `0xc3b9e0` | Resource::StepC3B9E0 | In outline: a step with a part's list (+0x6C +0x3C +0x18) |
| `0xbff310` | Resource::InitBFF310 | In outline: sets up a record with its table (`0xFA5CF0`) |
| `0xbe86f0` | Resource::InitBE86F0 | In outline: sets up a record with its table (`0xFA56D0`) |
| `0xbbd140` | Track::FindBBD140 | In outline: clears a value (+0x18) and a find (RefList::Find) |
| `0xba8e00` | Track::CheckBA8E00 | In outline: when not flagged (+0x158), a step with no part |
| `0x49b780` | Record::Init49B780 | In outline: clears a record |
| `0x418da0` | Effects::Step418DA0 | In outline: a step with many values |
| `0xc5d7a0` | Window::StepC5D7A0 | In outline: a step with a part (+0x18, slot 0x78) |
| `0xc2b8b0` | Resource::PartC2B8B0 | In outline: with a part (+0x24), a step |
| `0xc167c0` | Resource::CompareC167C0 | In outline: compares two records by a value (+4) |
| `0xc0bad0` | Parser::MakeC0BAD0 | In outline: makes a part (Parser::Allocate 4, 0x1C) |
| `0xc0bb70` | Parser::MakeC0BB70 | In outline: makes a part (Parser::Allocate 4, 0x20) |
| `0xcc29d0` | Shader::SharedCC29D0 | In outline: with no shared value (`0x13F70D8`), a step |
| `0xc63460` | Window::CheckC63460 | In outline: a check (slot 0x70) |
| `0xbff010` | Resource::InitBFF010 | In outline: fills a record from its arguments |
| `0xbd2ed0` | Table::StepBD2ED0 | In outline: a step with many values |
| `0x44b890` | Effect::Pass44B890 | In outline: goes over entries |
| `0xc55190` | Device::SharedC55190 | In outline: when not asked and with no shared part (`0x13E6C98`), a step |
| `0xc0d5a0` | Text::CountC0D5A0 | In outline: 0 for none, else a count |
| `0x497980` | Engine::Init497980 | In outline: sets up a record with empty texts (WideString::Empty) |
| `0xc38210` | Engine::InitC38210 | In outline: sets up a record with its table (`0xFB0AC8`) |
| `0x496540` | Engine::Make496540 | In outline: makes a 0x8-byte part |
| `0x42c870` | Engine::Make42C870 | In outline: makes a 0x508-byte part |
| `0x42c7d0` | Engine::Make42C7D0 | In outline: makes a 0x508-byte part |
| `0x42c9c0` | Engine::Make42C9C0 | In outline: makes a 0x1708-byte part |
| `0x438e90` | Engine::Make438E90 | In outline: makes a 0x4-byte part |
| `0xc04430` | Engine::MakeC04430 | In outline: makes a 0x8-byte part |
| `0x45e390` | Effect::Round45E390 | In outline: a step with a rounded value (+0x20) and a flag (+0x14) |
| `0xcd88e0` | Camera::SetCD88E0 | In outline: sets three values (+0xFC on) when they change |
| `0xcada70` | Resource::EntryCADA70 | In outline: for n not negative and no part (+0x5C), a step |
| `0xca1940` | Font::MetricsCA1940 | In outline: takes a font's text metrics and character widths (ABC) |
| `0xc75fe0` | Device::StepC75FE0 | In outline: with no value (+0x1A40 +0x18), a step with the device (slot 0x10C) |
| `0xc6b2a0` | Window::StepC6B2A0 | In outline: a step with a part (+0xE4, slot 0xBC) and another (+0xE0 +0xB0) |
| `0xc58220` | Device::PassC58220 | In outline: goes over entries down from a count (+0x44) |
| `0xc1a1b0` | Resource::StepC1A1B0 | In outline: a step with two work values |
| `0xc183f0` | Path::CopyC183F0 | In outline: sets up a record with its table (`0xFAFC88`) from another |
| `0xc16280` | Parser::TextC16280 | In outline: parses a text (Parser::ParseText) |
| `0xc161e0` | Resource::MakeC161E0 | In outline: makes a record (Resource::MakeC160B0) |
| `0xba0340` | Track::FindBA0340 | In outline: a find in a list |
| `0x4569e0` | Record::Make4569E0 | In outline: makes a record (Record::Make455FD0) and acts |
| `0x401110` | Array::Get401110 | In outline: stops in the debugger for a negative n, else an entry |
| `0xcc2d40` | Shader::CompareCC2D40 | In outline: compares two records by their first value |
| `0xcc1b90` | Shader::InitCC1B90 | In outline: sets up a record with its table (`0xFC317C`) |
| `0xcb2660` | Mesh::HalveCB2660 | In outline: a loop halving a count (+0x14) |
| `0xc12560` | Resource::InitC12560 | In outline: sets up a record from another's value (+4) |
| `0xbee1e0` | Matrix::StepBEE1E0 | In outline: a step with a matrix's values |
| `0x49dc50` | Text::Case49DC50 | In outline: a record by case |
| `0x468590` | Array::Get468590 | In outline: stops in the debugger for a negative n, else an entry |
| `0xc69730` | Window::StepC69730 | In outline: a step (Window::StepC5D840) and more |
| `0xc66c30` | Window::InitC66C30 | In outline: fills a record from many values |
| `0xc46770` | Model::WriteC46770 | In outline: writes a "CTBL" tag (Writer::Write4) and a value (`0xCA6460`) |
| `0xc43450` | Resource::InitC43450 | In outline: sets up a record with its table (`0xFB12A4`) pointing at itself |
| `0xc32720` | Text::FormatC32720 | In outline: formats a message with its arguments into a 4096-character buffer |
| `0xbff540` | Resource::InitBFF540 | In outline: fills a record from its arguments |
| `0x462fc0` | Record::Find462FC0 | In outline: a find in a list |
| `0x471180` | Labels::Init471180 | In outline: sets up a record with its table (`0xF2CCCC`) |
| `0xca6620` | Resource::NameCA6620 | In outline: a text from a name buffer (+0x448) |
| `0xca64c0` | Resource::NameCA64C0 | In outline: the same (another copy) |
| `0xc6b340` | Window::StepC6B340 | In outline: a step (Window::StepC679E0) and a part (+0xE0 +0xB0) |
| `0xc44020` | Model::WriteC44020 | In outline: writes a "DEFN" tag (Writer::Write4) and a value (`0xCA6460`) |
| `0xc33ea0` | Resource::PartC33EA0 | In outline: a step with a part (+0x120) |
| `0xc081e0` | Device::CursorC081E0 | In outline: a point from the cursor and window rectangle |
| `0xbaf4d0` | Track::SharedBAF4D0 | In outline: a step with a shared list (`0x13D064C` on) |
| `0x4631a0` | Record::Find4631A0 | In outline: a find in a list |
| `0x460e80` | Record::Find460E80 | In outline: the same (another copy) |
| `0xc469e0` | ShaderFile::ReadTagC469E0 | In outline: reads a four-byte tag and checks it is "STBL" |
| `0xbd6f10` | Resource::InitBD6F10 | In outline: sets up a record with its table (`0xFA533C`) |
| `0xbd90d0` | Resource::InitBD90D0 | In outline: sets up a record with its table (`0xFA5340`) |
| `0x4251c0` | Sky::Pass4251C0 | In outline: goes over entries (+0x44, +0x48) |
| `0xc509a0` | Record::InitC509A0 | In outline: fills a record from a value and clears a flag |
| `0xc25920` | Resource::MakeC25920 | In outline: makes an 8-byte part |
| `0x463400` | Record::Last463400 | In outline: a step with the last entry (+0x18) |
| `0x4281f0` | Sky::Step4281F0 | In outline: a step with points and 7.2 |
| `0xce4830` | Math::StepCE4830 | In outline: a value from two records' second values |
| `0xc65360` | Window::SlotC65360 | In outline: with a slot (+0xF4), a step |
| `0xce9630` | Camera::SlotCE9630 | In outline: with no slot (+0xC = -1), a step with a value (+4) |
| `0xcb7ae0` | Text::CompareNoCaseCB7AE0 | In outline: compares C strings without case |
| `0xc57e40` | Device::CaseC57E40 | In outline: a value by case, for negative n |
| `0xc480f0` | Record::InitC480F0 | In outline: fills a record with 0xFFFF, 0 and more |
| `0xc0f4b0` | Resource::InitC0F4B0 | In outline: sets up a record with its tables (`0xFAFB40`, `0xFAFB3C`) |
| `0xc0e420` | Resource::InitC0E420 | In outline: sets up a record from another's value (+4) |
| `0xbc0e70` | Track::SharedBC0E70 | In outline: once (bit 0 of `0x13D0A40`), sets up a shared value |
| `0xcc2420` | Shader::FreeAllCC2420 | In outline: tears down each entry it finds (`0xCC25D0`) until none is left |
| `0xca5ac0` | Text::StepCA5AC0 | In outline: a step with a 16-character buffer |
| `0xcb9370` | Memory::MakeCB9370 | In outline: takes 0x40 bytes from a callback (+0x20) and fills them |
| `0xc19240` | Bits::RangeC19240 | In outline: sets a range of bits (Bits::SetRange) |
| `0xc0ecd0` | Resource::InitC0ECD0 | In outline: sets up a record with its table (`0xFAFB14`) |
| `0xc05310` | Device::StepC05310 | In outline: a step with a value, the device's (+0x454) when none |
| `0xbdd740` | Buffer::SetBDD740 | In outline: sets a size when it changes, 0 handled |
| `0xbd4fe0` | Device::ReleaseBD4FE0 | In outline: with a part (+0x208), releases another (+0x1F8) |
| `0xbd23a0` | Device::StepBD23A0 | In outline: a step with a part (slot 0xC) |
| `0xba5ef0` | List::AddBA5EF0 | In outline: grows a list by 16 when full (Array::Resize5501E0) and adds |
| `0x494f10` | Grid::Size494F10 | In outline: sets a size from two values |
| `0xcc27a0` | Shader::StepCC27A0 | In outline: a step with an object |
| `0xc662f0` | Window::InitC662F0 | In outline: fills a record from its arguments |
| `0xc380e0` | Resource::BufferC380E0 | In outline: makes an aligned buffer of four bytes per entry of another (+0xC) |
| `0x428320` | Static::Shared428320 | In outline: with no shared count (`0x1106C54`), a step |
| `0xbd0c50` | Engine::MakeBD0C50 | In outline: makes a 0x10-byte part |
| `0x416c00` | Engine::Reset416C00 | In outline: resets (Static::Reset) and more |
| `0xc8b760` | Engine::InitC8B760 | In outline: sets up a record with its table (`0xFBEE2C`) |
| `0x443ae0` | Engine::Make443AE0 | In outline: makes a 0x10-byte part |
| `0x4098c0` | Engine::Make4098C0 | In outline: makes a 0x1C-byte part |
| `0x496970` | Engine::Reset496970 | In outline: resets (Static::Reset) and more |
| `0xc235c0` | Engine::PassC235C0 | In outline: goes over entries (+0x20) |
| `0x42f9b0` | Engine::Copy42F9B0 | In outline: copies values from another record |
| `0xc3c100` | Engine::MakeC3C100 | In outline: makes a 0x10-byte part |
| `0xbd9c90` | Engine::MakeBD9C90 | In outline: makes a 0x10-byte part |
| `0x46a9f0` | Labels::Make46A9F0 | In outline: with no part, makes a 0x60-byte one |
| `0xc44660` | Model::WriteC44660 | In outline: writes a "SDCL" tag and a value (Writer::Write4) |
| `0xc32210` | Resource::MakeC32210 | In outline: makes a part |
| `0xc31350` | Resource::InitC31350 | In outline: sets up a record with its table (`0xFB0318`) |
| `0xbe8b10` | Resource::InitBE8B10 | In outline: sets up a record with its table (`0xFA56F8`) |
| `0xbceba0` | Text::FindBCEBA0 | In outline: finds a two-byte text in a table |
| `0xbb7bd0` | Track::StepBB7BD0 | In outline: a step (`0xBC28D0`), with 0xFFFF handled |
| `0x691830` | Window::Set691830 | In outline: sets a size when it changes |
| `0x498210` | Static::Set498210 | In outline: the same (another copy) |
| `0x46f8e0` | Labels::Distance46F8E0 | In outline: a step with the distance between two points |
| `0x446710` | Record::Set446710 | In outline: sets a size when it changes |
| `0xc442d0` | Record::InitC442D0 | In outline: fills a record from its arguments |
| `0xc30710` | Resource::StepC30710 | In outline: with no part (+0x2C) and not flagged (+4), a step with another (+0x1C) |
| `0xbd2290` | List::StepBD2290 | In outline: a step with a list (+4) |
| `0x493a90` | Static::Count493A90 | In outline: for a positive count, a step |
| `0xcc4b40` | Shader::CheckCC4B40 | In outline: a check (`0xCC4BD0` with 0x55) |
| `0xcb61e0` | Shader::FreeCB61E0 | In outline: with a part (+0x84), frees it |
| `0xc58620` | Device::CountC58620 | In outline: 0 with no part (+0x44), else a count |
| `0xbe41c0` | Text::MatchBE41C0 | In outline: matches two C strings |
| `0xbd2e40` | String::StepBD2E40 | In outline: a step with the shared empty string (String::SharedEmpty) |
| `0x484780` | Window::Take484780 | In outline: with a part (+0x58), takes it (Window::Take6A65B0) |
| `0x46d7c0` | Labels::Shared46D7C0 | In outline: when given, a step with a shared value (`0x10EC998`) |
| `0xcf02a0` | Shapes::InitCF02A0 | In outline: fills a record from its arguments |
| `0xc2e930` | Resource::PartC2E930 | In outline: with a part (+8), a step with a value (+0x24) |
| `0xc023c0` | Record::InitC023C0 | In outline: sets up a record (Record::InitC02060) with its table (`0xFA5ED0`) |
| `0xbcfe60` | Text::StepBCFE60 | In outline: a step with a 1024-character buffer |
| `0xbcec90` | Text::FindBCEC90 | In outline: finds a C string in a table |
| `0xbcbac0` | Track::StepBCBAC0 | In outline: a step with many values |
| `0x429fc0` | Registry::Step429FC0 | In outline: makes sure of a registry key (Registry::Ensure) and goes over entries |
| `0xc48ac0` | Buffer::FreeC48AC0 | In outline: frees aligned buffers (+0xC, +0x10, ...) |
| `0xc489c0` | Buffer::FreeC489C0 | In outline: the same (another copy) |
| `0xc47010` | Model::WriteC47010 | In outline: writes a "PSU" tag and a value (Writer::Write4) |
| `0xc0e120` | Resource::InitC0E120 | In outline: sets up a record from another's value (+4) |
| `0xbfc1f0` | Text::FlagsBFC1F0 | In outline: when three flags are set (bits 0-2 of +0x7C), a step |
| `0xbe87e0` | Resource::FreeBE87E0 | In outline: with a part (+0x10), tears it down in turn |
| `0xbd6540` | HashTable::AddBD6540 | In outline: grows a table when needed (HashTable::GrowBD81C0) and adds by a key's remainder |
| `0xbc5310` | Track::CompareBC5310 | In outline: a check (`0xBC40A0`) between two records |
| `0x446610` | Windows::VersionCheck446610 | In outline: a check with the Windows version (GetVersionEx) |
| `0x428290` | Sky::Step428290 | In outline: a step with points and 7.2 |
| `0x41c180` | Effects::Flag41C180 | In outline: when a flag is set (Flags::Bit0) and an object is flagged (bit 25), a step |
| `0x419070` | Effects::Loop419070 | In outline: goes over entries (Thunk) |
| `0xc0dde0` | Text::InitC0DDE0 | In outline: sets up a record with its table (`0xFAF9C0`) and a text |
| `0xbf6350` | List::StepBF6350 | In outline: a step with a list |
| `0xbcf260` | Text::FindBCF260 | In outline: finds a C string in a table |
| `0x430ee0` | Sky::Entry430EE0 | In outline: for entry n (0x30 bytes) under a count (+0x4C), a step |
| `0x494fe0` | Static::Limit494FE0 | In outline: a step with n kept at most a count |
| `0xcb72e0` | Buffer::FreeCB72E0 | In outline: frees an aligned buffer and more |
| `0xc62360` | Window::StepC62360 | In outline: a step (slot 0x70) |
| `0xc37f00` | Resource::LoopC37F00 | In outline: goes over a part's entries (+4 +8) |
| `0xbe27b0` | Resource::InitBE27B0 | In outline: a find (Resource::FindBE2450) and sets up a record with its table (`0xFA55E4`) |
| `0xbe0100` | Record::SwapBE0100 | In outline: swaps values with another record |
| `0x6dbf50` | Window::Value6DBF50 | In outline: a value with a handle (`0xF64F78`) |
| `0x6b3a70` | Window::Init6B3A70 | In outline: fills a record from many values |
| `0x644e00` | Window::Value644E00 | In outline: a value from a part (+0x1AC, slot 4) with another (+0x1B0) |
| `0x4c7a90` | Shop::Set4C7A90 | In outline: a step when n matches a value (+0xC4) |
| `0x4576b0` | Ref::Assign4576B0 | In outline: lets go of its part and takes another's |
| `0x410a10` | Effect::Find410A10 | In outline: a find over a list (+0x20) |
| `0x410980` | Ref::Assign410980 | In outline: lets go of its part and takes another's |
| `0xcadca0` | Resource::EntryCADCA0 | In outline: for n not negative and no part (+0x5C), a step |
| `0xc622d0` | Window::StepC622D0 | In outline: a step with a part (+0xB0) |
| `0xc30da0` | Resource::StepC30DA0 | In outline: with no part (+0x2C) and not flagged (+4), a step with another (+0x20) |
| `0xc028d0` | Record::StepC028D0 | In outline: a step with many values |
| `0xbe8db0` | Resource::ModeBE8DB0 | In outline: with a shared value (`0x10F2F00`), a step in mode 1 (+0x4C) |
| `0xbbcb30` | Track::StepBBCB30 | In outline: a step, with its own storage (+0xC) handled |
| `0x6a6020` | Window::Step6A6020 | In outline: the same (another copy) |
| `0x47cb10` | Labels::Step47CB10 | In outline: the same (another copy) |
| `0x476880` | Labels::Step476880 | In outline: the same (another copy) |
| `0x470270` | Labels::List470270 | In outline: a step with a list and a rectangle |
| `0x45b600` | Record::Step45B600 | In outline: with a part, a step |
| `0x483160` | Window::Style483160 | In outline: sets a window's style (with or without a frame) and its rectangle |
| `0x40ce20` | Effect::Entry40CE20 | In outline: a step with entry n (0x70 bytes) and the device |
| `0xcb0320` | Zlib::StepCB0320 | In outline: with a stream in state 6, a step |
| `0xcae390` | ResourceFile::NameCAE390 | In outline: a name (ResourceFile::NameCADF80) and more |
| `0xc60260` | Window::CaseC60260 | In outline: a step by kind (1, ...) |
| `0xc504f0` | Buffer::FreeC504F0 | In outline: frees aligned buffers (+8, +0xC) and more |
| `0xc30d10` | Resource::CheckC30D10 | In outline: stops in the debugger when flagged (+0x18) |
| `0xc21d60` | Record::InitC21D60 | In outline: clears a record |
| `0xbe7a00` | Resource::CheckBE7A00 | In outline: false with no part (+8), else a check |
| `0x4350b0` | Sky::Assign4350B0 | In outline: frees its buffer and remakes it from another |
| `0xc2ee50` | Resource::EntryC2EE50 | In outline: for n under a count (+0x18), a step with no part (+0x24) |
| `0xc019a0` | Device::KeyboardC019A0 | In outline: a step with the keyboard layout (HKL) |
| `0xbfa2f0` | Resource::EntryBFA2F0 | In outline: for n under a count (+0x10), a step with no part (+0x28) |
| `0x6acbd0` | Window::Scale6ACBD0 | In outline: a point scaled by the screen's area (Screen::AreaScale) |
| `0xcd0c60` | Camera::StepCD0C60 | In outline: a step with many values |
| `0xc63370` | Window::CheckC63370 | In outline: a step when a check passes (Device::StepC5F7A0) |
| `0xc2b7b0` | Resource::GrowC2B7B0 | In outline: makes an aligned buffer for n when n is below a count |
| `0xbf9bb0` | Text::SharedBF9BB0 | In outline: once (bit 0 of `0x13D29B0`), sets up a shared value |
| `0xbf7530` | Text::StepBF7530 | In outline: a step with many values |
| `0xbe8cd0` | Resource::SlotBE8CD0 | In outline: with slot n (+0x20), a step |
| `0xbe49f0` | Ref::InitBE49F0 | In outline: takes a reference to a part |
| `0xb92bb0` | Ref::ReleaseB92BB0 | In outline: takes one from a count (+4) and frees at zero |
| `0xb32d90` | Ref::ReleaseB32D90 | In outline: the same (another copy) |
| `0x6615d0` | Ref::Release6615D0 | In outline: the same (another copy) |
| `0x4634a0` | Engine::Pass4634A0 | In outline: goes over entries (+0x18) |
| `0xc1aa80` | Engine::MakeC1AA80 | In outline: makes a 0x44-byte part |
| `0x49dfd0` | Engine::Make49DFD0 | In outline: sets up a window part |
| `0xc10ca0` | Engine::MakeC10CA0 | In outline: makes a part (Parser::Allocate 4, 0xC4) |
| `0xc10940` | Engine::MakeC10940 | In outline: makes a part (Parser::Allocate 4, 0x94) |
| `0xc0e4c0` | Engine::MakeC0E4C0 | In outline: makes a part (Parser::Allocate 4, 0x8C) |
| `0xc12600` | Engine::MakeC12600 | In outline: makes a part (Parser::Allocate 4, 0x44) |
| `0xc110e0` | Engine::MakeC110E0 | In outline: makes a part (Parser::Allocate 4, 0x54) |
| `0xc0ec20` | Engine::MakeC0EC20 | In outline: makes a part (Parser::Allocate 4, 0x44) |
| `0xc0e1b0` | Engine::MakeC0E1B0 | In outline: makes a part (Parser::Allocate 4, 0x58) |
| `0x48d5d0` | Engine::Make48D5D0 | In outline: makes a 0x10-byte part |
| `0xcd6770` | Engine::MakeCD6770 | In outline: makes a 0x18-byte part |
| `0xbc3aa0` | Engine::TerrainBC3AA0 | In outline: a step when the terrain is not flagged |
| `0xc344e0` | Engine::MakeC344E0 | In outline: makes a 0x14-byte part |
| `0xc34460` | Engine::MakeC34460 | In outline: makes a 0x14-byte part |
| `0xc33da0` | Engine::MakeC33DA0 | In outline: makes a 0x14-byte part |
| `0xc33d20` | Engine::MakeC33D20 | In outline: makes a 0x14-byte part |
| `0x475440` | Engine::Make475440 | In outline: makes a 0x20-byte part |
| `0xc59620` | Engine::MakeC59620 | In outline: makes a 0x18C-byte part |
| `0xc595a0` | Engine::MakeC595A0 | In outline: makes a 0x154-byte part |
| `0x4683b0` | Engine::Make4683B0 | In outline: makes a 0xF0-byte part |
| `0x471220` | Labels::Grow471220 | In outline: grows when a count (+0x114) plus n passes its room (+0x110) |
| `0x42de10` | List::Add42DE10 | In outline: adds when there is room (+8, +0xC) |
| `0xcbc990` | Shader::StepCBC990 | In outline: a step by a shared table (`0xFC1340`) and the last entry |
| `0xca2ba0` | Resource::ClearCA2BA0 | In outline: with a part (+0x10), empties a list |
| `0xca29c0` | Resource::MakeCA29C0 | In outline: makes a part (Resource::MakeC18CF0) |
| `0xc5f380` | Window::StepC5F380 | In outline: when not flagged (+0x9C), a step with a work buffer |
| `0xc5e270` | Window::SetPointC5E270 | In outline: copies a point into its values (+0x150 on) |
| `0xc0f2c0` | Resource::InitC0F2C0 | In outline: sets up a record with its tables (`0xFAFB34`, `0xFAFB30`) |
| `0xc05630` | Device::ResetC05630 | In outline: clears a shared value (`0x13D321C`) and a step (slot 0xE8) |
| `0xbea680` | Resource::RangeBEA680 | In outline: a step for a range of n entries from a start |
| `0xbd3310` | Device::StepBD3310 | In outline: a step with many values |
| `0xbceaa0` | Text::FindBCEAA0 | In outline: finds a text in a table |
| `0xba4cf0` | Track::PassBA4CF0 | In outline: goes over entries |
| `0x409830` | Device::Step409830 | In outline: a step with the device |
| `0x484ed0` | List::Pass484ED0 | In outline: goes over entries (+8) |
| `0xc69910` | Window::InitC69910 | In outline: sets up a window record (Window::InitC65F20) and a value |
| `0xc48410` | Resource::FindC48410 | In outline: a find by a C string over entries (+0x18) |
| `0xc05430` | Device::StepC05430 | In outline: a step with many values |
| `0xbeb650` | Resource::ListBEB650 | In outline: a step with a list (+0x1A4) |
| `0x4952e0` | Static::Add4952E0 | In outline: adds an entry, growing when full |
| `0x42e5f0` | Sky::Pass42E5F0 | In outline: goes over entries with a shared count (`0x1103C14`) |
| `0xc48370` | Resource::EntryC48370 | In outline: for a nonzero slot, a value from entry n (+0x18) |
| `0xc312c0` | Resource::InitC312C0 | In outline: sets up a record with its table (`0xFB0318`) |
| `0xc16920` | Resource::StepC16920 | In outline: a step with a work buffer |
| `0xc072b0` | Device::StepC072B0 | In outline: a step (slot 0x30) and a loop (List::LoopBEB4B0) |
| `0xbe6780` | Array::SetBE6780 | In outline: grows to hold n when needed and sets entry n |
| `0xba0060` | Track::EntryBA0060 | In outline: a value from an entry (+0x28 plus n) |
| `0x4987c0` | Static::Step4987C0 | In outline: a step with two work values |
| `0xce7860` | Camera::EntryCE7860 | In outline: for nonzero n, a step with entry n (0x14 bytes) |
| `0xc9c410` | Batches::ListC9C410 | In outline: a step with a list (+4, +8) |
| `0xbe9510` | Device::StepBE9510 | In outline: a step with a part (+4, slot 0x2C) |
| `0xbaf790` | Track::SharedBAF790 | In outline: a step when a shared list has an entry (Track::SharedBAF4D0) |
| `0x498840` | Static::Step498840 | In outline: a step with two work values |
| `0x464960` | Scenery::Clear464960 | In outline: when not flagged (+0x884), clears (Scenery::Clear4632E0) |
| `0x41c0e0` | Ref::Assign41C0E0 | In outline: takes a reference to another's part when it differs |
| `0xcc7640` | Model::EntryCC7640 | In outline: a step for entry n (0x60 bytes) |
| `0xc61c20` | Window::StepC61C20 | In outline: a step with work values |
| `0xc61770` | Window::StepC61770 | In outline: the same (another copy) |
| `0xc2fa90` | Table::FindC2FA90 | In outline: 0 for an empty name, else a find |
| `0xbd5090` | Device::StepBD5090 | In outline: a step with a list (+0x204) |
| `0xbce9d0` | Text::FindBCE9D0 | In outline: finds a two-byte text in a table |
| `0xbc3fe0` | Track::ScaleBC3FE0 | In outline: a check with an object's scale (Object::Scale2) |
| `0x6ab1a0` | Window::Count6AB1A0 | In outline: adds one to a count and a value |
| `0x6ab220` | Window::Count6AB220 | In outline: the same (another copy) |
| `0x49dd30` | Text::Value49DD30 | In outline: a value by a shared mode (`0x10EE248`) |
| `0x40c1b0` | Effect::Step40C1B0 | In outline: a step with many values |
| `0xc3b0b0` | Resource::StepC3B0B0 | In outline: with no part (+0x6C) and a shared flag (`0x10F3058`), a step |
| `0xc258a0` | Resource::GrowC258A0 | In outline: frees and remakes a buffer when n passes its room |
| `0xc1c430` | Resource::MakeC1C430 | In outline: a step (Resource::PartC1C220) and makes a part (+0xC) |
| `0x468850` | Labels::Device468850 | In outline: when on (+4), a step with the device (slot 0xCC) |
| `0xc968a0` | Batches::StepC968A0 | In outline: when not flagged (+0x4C1), a step |
| `0xc5e470` | Window::ValueC5E470 | In outline: a value (slot 0x70) |
| `0xc466a0` | Model::WriteC466A0 | In outline: writes a "SDCL" tag and a value (Writer::Write4) |
| `0xc3b6b0` | Resource::StepC3B6B0 | In outline: with more than two entries (+0x6C +0x30), a step |
| `0xbd9530` | Table::EntryBD9530 | In outline: for kind 1, entry n of a table (+0x10) |
| `0xbcf0a0` | Text::FindBCF0A0 | In outline: finds a C string in a table |
| `0xbbcd40` | Track::StepBBCD40 | In outline: a step with a work buffer |
| `0xbaf570` | Track::SharedBAF570 | In outline: a check with a shared list (`0x13D0650`, `0x13D065C`) |
| `0x455640` | List::Find455640 | In outline: a find over entries (+4) |
| `0xcc6280` | Model::SetPartCC6280 | Sets part n's flag to 1 and its value (8 bytes at +0x14) |
| `0xc75290` | Renderer::ClearC75290 | In outline: clears (Renderer::ClearC056C0) and a value (+0x67C) |
| `0x4d66b0` | Shop::Step4D66B0 | In outline: a step with a value (+0x14) and a byte (+0xC) |
| `0x4094f0` | Effect::Part4094F0 | In outline: with a part (+8), a step |
| `0xce7370` | Camera::AddCE7370 | In outline: grows a list by 100 when full (`0xCE7110`) and adds |
| `0xccb260` | Shader::StepCCB260 | In outline: a step with a value (+0x2C) and a shared one (`0x13FE034`) |
| `0xcc4bd0` | Shader::ScanCC4BD0 | In outline: goes over bytes until a match |
| `0xc2fed0` | Resource::SharedC2FED0 | In outline: with a shared list (`0x13E69B0`), a step |
| `0xbbcfa0` | Track::ListBBCFA0 | In outline: a step with a list |
| `0xcb9240` | Text::StepCB9240 | In outline: a step (`0xBF47B0` with `0x13E6FBC`) |
| `0xc693d0` | Window::StepC693D0 | In outline: a step with a work buffer |
| `0xbdd860` | Memory::AlignBDD860 | In outline: rounds a count up to a multiple of n and acts |
| `0x45e510` | Effect::Make45E510 | In outline: makes a part with values |
| `0xca4970` | List::AddCA4970 | In outline: grows a list (twice plus 10) when full and adds |
| `0xc55360` | Stream::InitC55360 | In outline: a stream name (Stream::NameC54DC0 with 0, 9) and more |
| `0xc21100` | Text::MatchC21100 | In outline: matches two C strings |
| `0xbd16d0` | Record::InitBD16D0 | In outline: sets up a record with its table (`0xFA14D0`) and a value |
| `0xbcf8c0` | Record::InitBCF8C0 | In outline: fills a record with 1 and another's value (+4) |
| `0xc50c70` | Stream::CheckC50C70 | In outline: stops in the debugger when a size (+4) is negative |
| `0xbe97a0` | Device::StepBE97A0 | In outline: when not flagged (+0x24), a step |
| `0xbdbfd0` | Text::CompareBDBFD0 | In outline: compares two texts |
| `0x49a400` | Record::Init49A400 | In outline: clears a record |
| `0x44a690` | Effect::Entry44A690 | In outline: for entry n (0x40 bytes) under a count (+0x28), a step |
| `0xbc5200` | Track::CompareBC5200 | In outline: compares two records |
| `0xcefe90` | Shapes::EntryCEFE90 | In outline: a value for entry n plus 9 (16 bytes) |
| `0xc57530` | File::MoveC57530 | In outline: renames or moves a file |
| `0xbea1c0` | Resource::PassBEA1C0 | In outline: goes over entries |
| `0xbe82c0` | Resource::ListBE82C0 | In outline: with a part (+8), goes over a list |
| `0x42e8b0` | Sky::Entry42E8B0 | In outline: for entry n (16 bytes) of a shared table (`0x1103C0C`) that is on, a step |
| `0x460870` | Engine::Entry460870 | In outline: a value from an entry (+0x28 plus n) |
| `0x460660` | Engine::Entry460660 | In outline: a value from an entry (+0x28 plus n) |
| `0xca33c0` | Engine::MakeCA33C0 | In outline: makes a 0x10-byte part |
| `0xc34560` | Engine::MakeC34560 | In outline: makes a 0x14-byte part |
| `0xc33e20` | Engine::MakeC33E20 | In outline: makes a 0x14-byte part |
| `0xcec1f0` | Engine::MakeCEC1F0 | In outline: makes a 0x8-byte part |
| `0xc6ea40` | Engine::MakeC6EA40 | In outline: makes a 0x110-byte part |
| `0xcc82b0` | Engine::PassCC82B0 | In outline: goes over entries (+0x24) |
| `0xc07bb0` | Engine::MakeC07BB0 | In outline: makes a 0x18-byte part |
| `0x49cc80` | Engine::Make49CC80 | In outline: makes a 0x18-byte part |
| `0xc343f0` | Engine::MakeC343F0 | In outline: makes a 0x8-byte part |
| `0xc33cb0` | Engine::MakeC33CB0 | In outline: makes a 0x8-byte part |
| `0x42dd30` | Engine::Terrain42DD30 | In outline: a step when the terrain is not flagged |
| `0xc38c90` | Engine::MakeC38C90 | In outline: makes a 0x8-byte part |
| `0x4028c0` | Engine::Make4028C0 | In outline: makes a 0x10-byte part |
| `0x401080` | Engine::Make401080 | In outline: makes a 0x10-byte part |
| `0xc0fae0` | Engine::MakeC0FAE0 | In outline: makes a part (Parser::Allocate 4, 0x44) |
| `0xc0fa30` | Engine::MakeC0FA30 | In outline: makes a part (Parser::Allocate 4, 0x94) |
| `0x456e60` | File::Read456E60 | In outline: reads from a file (FILE), -1 on failure |
| `0x436a30` | Sky::Make436A30 | In outline: makes a 0x3FA-byte buffer (Memory::AllocCounted) and acts |
| `0xce1d30` | Camera::StepCE1D30 | In outline: unless flagged (bit 0 of +0x1C), a step with a value (+0x20) |
| `0xcc1a20` | Shader::StepCC1A20 | In outline: a step with work values |
| `0xc69f90` | Window::InitC69F90 | In outline: sets up a window record (Window::InitC65F20) and values |
| `0xc67960` | Window::PartC67960 | In outline: with an object's part (+0x18), a step |
| `0xc3b490` | Resource::StepC3B490 | In outline: with a part (+0x6C) of more than one entry, a step |
| `0xbd3290` | Device::ListBD3290 | In outline: a step with a list (+0x2C) |
| `0xbcf170` | Text::FindBCF170 | In outline: finds a text in a table |
| `0xbacff0` | Track::CountBACFF0 | In outline: with fewer than two entries (+0x1D8), a step |
| `0xac6b10` | Graph::NextAC6B10 | In outline: the next entry while under a count (+0x4C, +0x50) |
| `0x42b3c0` | Shared::Make42B3C0 | In outline: once (`0x12F5798`), makes a 0x58-byte record |
| `0xc678e0` | Window::PartC678E0 | In outline: with an object's part (+0x18), a step |
| `0xc67780` | Window::DetachC67780 | In outline: clears a flag (+0x9C) and detaches (Window::DetachC675A0) |
| `0xc470a0` | ShaderFile::ReadTagC470A0 | In outline: reads a four-byte tag and checks it |
| `0xc326a0` | Text::MakeC326A0 | In outline: makes a buffer three times n long, none handled |
| `0xc32360` | Text::FormatC32360 | In outline: formats into a 100-character buffer |
| `0xbe46c0` | ModuleList::RunBE46C0 | In outline: runs a module list (ModuleList::Run) and throws on failure |
| `0xbdc800` | VertexDeclaration::MoveBDC800 | In outline: moves two-byte entries within its storage (+0xC) |
| `0xbd1430` | Record::CheckBD1430 | In outline: a check with a pointer (+4, low bits cleared) |
| `0x40cf90` | Effect::Entry40CF90 | In outline: a step for entry n (0x24 bytes) |
| `0xcb1360` | Shader::PartCB1360 | In outline: with a part (+0x18), a step |
| `0xca5b90` | Table::CheckCA5B90 | In outline: throws an error (0x2A01) when bits do not match |
| `0xc091c0` | Text::FormatC091C0 | In outline: formats a message with its arguments into a 2048-character buffer |
| `0xbfc0d0` | Text::FlagsBFC0D0 | In outline: when two flags are set (bits 0-1 of +0x7C), a value |
| `0x6dfa50` | Window::Part6DFA50 | In outline: a step with a part (+0x98) |
| `0x49ddb0` | Text::Value49DDB0 | In outline: a value by a shared mode (`0x10EE248`) |
| `0x43b030` | Effect::Init43B030 | In outline: sets flags (+0x2C, +0x2D) and more |
| `0xca5020` | Mesh::FreeCA5020 | In outline: frees a mesh (Mesh::FreeCB7740) and a buffer (+8) |
| `0xca3b70` | Resource::InitCA3B70 | In outline: fills a record from an object and its arguments |
| `0xca3680` | Resource::InitCA3680 | In outline: sets up a text record (Text::InitBF47D0) with its table (`0xFBF348`) |
| `0xbfd200` | Device::StepBFD200 | In outline: a step with a part (+4, slot 0x2C) |
| `0x42a2b0` | List::Add42A2B0 | In outline: adds when there is room (+4, +8) |
| `0xcc5050` | Shader::StepCC5050 | In outline: a step with work values |
| `0xcc3f80` | Shader::StepCC3F80 | In outline: the same (another copy) |
| `0xcbc800` | Text::EndsCBC800 | In outline: a step when a buffer ends with two zero bytes |
| `0xcb9130` | Text::StepCB9130 | In outline: a step (`0xBF47B0` with `0x13E6F9C`) |
| `0xcb9020` | Text::StepCB9020 | In outline: the same with `0x13E6F98` |
| `0xcb8f10` | Text::StepCB8F10 | In outline: the same with `0x13E6FA4` |
| `0xcb8de0` | Text::StepCB8DE0 | In outline: the same with `0x13E6FA0` |
| `0xc37e30` | Resource::LoopC37E30 | In outline: goes over entries (+8) |
| `0xc0ad50` | Text::EndsC0AD50 | In outline: a step when a buffer ends with two zero bytes |
| `0x4956b0` | Text::Step4956B0 | In outline: a step with a two-byte value and a C string |
| `0x409c80` | Table::Entry409C80 | In outline: an entry by the high half of n |
| `0xce6290` | Camera::ResetCE6290 | In outline: for none, sets a value to -1 |
| `0xcd90e0` | Camera::StepCD90E0 | In outline: with no part (+8) and another (+0xEC), a step |
| `0xcc1990` | Shader::StepCC1990 | In outline: a step with many values |
| `0xcb9f50` | Zlib::StepCB9F50 | In outline: in state 4 or 5, a callback (+0x24) |
| `0xcb9300` | Zlib::ValueCB9300 | In outline: gives a value (+0x3C) when asked |
| `0xc48a50` | HashTable::ResizeC48A50 | In outline: sizes a table to a prime (Math::NextPrime) and puts the entries back |
| `0xc48950` | HashTable::ResizeC48950 | In outline: the same (another copy) |
| `0xc25830` | Resource::ArrayC25830 | In outline: makes an aligned array of n entries of 0xB0 bytes |
| `0xc0e5e0` | Resource::InitC0E5E0 | In outline: sets up a record with its tables (`0xFAFAF8`, `0xFAFAF4`) |
| `0xbf5b60` | Text::ListBF5B60 | In outline: a step with a list (+0x1C) |
| `0xbe8340` | Resource::PartBE8340 | In outline: with a part (+8), a step |
| `0xbcfaa0` | Record::InitBCFAA0 | In outline: fills a record with 1 and a two-byte value |
| `0x6a4cb0` | Window::Check6A4CB0 | In outline: with an object, a step (slot 4) |
| `0x40f790` | Effect::List40F790 | In outline: a step with a list (+8, +0xC) |
| `0x40b3b0` | Effect::Grow40B3B0 | In outline: grows when a count (+0x48) plus n passes its room (+0x44) |
| `0xc63500` | Window::StepC63500 | In outline: with a part (+0xC0 +4), a step |
| `0xbe4650` | ModuleList::RunBE4650 | In outline: runs a module list (ModuleList::Run) and throws on failure |
| `0xbc8a30` | Object::FaceBC8A30 | In outline: a face step (Object::FaceBC8500) |
| `0x430d00` | List::Add430D00 | In outline: adds when there is room (+4, +8) |
| `0x40a940` | Effect::List40A940 | In outline: a step with a list (+8, +0xC) |
| `0xcbbe20` | Shader::ParseCBBE20 | In outline: goes over a C string with a list (+0x20) |
| `0xbf52f0` | Text::StepBF52F0 | In outline: a step with a part's value (+0x10 +8) and a list (+0x1C) |
| `0xbec600` | Buffer::AssignBEC600 | In outline: when it owns its buffer (+8), frees it and copies another |
| `0xbcfb80` | Record::InitBCFB80 | In outline: fills a record with 1 and another's value (+4) |
| `0xbad5c0` | Track::CountBAD5C0 | In outline: 0 with fewer than two entries (+0x1D8), else a step |
| `0x624860` | Ref::Step624860 | In outline: adds one to a count and acts |
| `0x624770` | Window::Point624770 | In outline: with no shared object (`0x13429A8`), a step with a point |
| `0x496f70` | Static::Text496F70 | In outline: a step with a 1024-character buffer and a value (+0x10) |
| `0x45b240` | Record::Part45B240 | In outline: a step with its part |
| `0xcdc3b0` | Camera::StepCDC3B0 | In outline: a step with a flag |
| `0xc10260` | Resource::PartC10260 | In outline: for an object, a step with its value (+4) |
| `0xc0f6b0` | Resource::InitC0F6B0 | In outline: sets up a record with its tables (`0xFAFB4C`, `0xFAFB48`) |
| `0xc0e2d0` | Resource::InitC0E2D0 | In outline: sets up a record with its tables (`0xFAFAEC`, `0xFAFAE8`) |
| `0xc09d40` | Text::AddC09D40 | In outline: adds one to a count (+4) and acts |
| `0xbd74d0` | HashTable::ResizeBD74D0 | In outline: sizes a table to a prime (Math::NextPrime) and puts the entries back |
| `0x43ef40` | Ref::Step43EF40 | In outline: adds one to a count and acts |
| `0xcc7530` | Model::EntryCC7530 | In outline: for entry n (0x60 bytes) with no slot (+0x58 = -1), a step |
| `0x49c8f0` | Record::Init49C8F0 | In outline: clears a record and fills two values |
| `0x470d70` | Labels::Step470D70 | In outline: a step with an identity-like start (0, 1.0) |
| `0x45e320` | Effect::Kind45E320 | In outline: for kind 7 (+4), a step |
| `0x45e270` | Effect::Kind45E270 | In outline: for kind 4 (+4), a step |
| `0xc342b0` | Resource::InitC342B0 | In outline: sets up a record with its table (`0xFB06BC`) |
| `0xc0a750` | Text::PartC0A750 | In outline: with a part (+0x14), a step |
| `0x42fd80` | Sky::Part42FD80 | In outline: with a part (+0x64), a step |
| `0x417fe0` | Sky::Tag417FE0 | In outline: a find (Sky::Find431E30) with the flat tag manager |
| `0xcc4690` | Ref::ReleaseCC4690 | In outline: takes one from a count (+0x1C) and frees at zero |
| `0xca3e30` | Resource::InitCA3E30 | In outline: fills a record from its arguments |
| `0xca3d30` | Resource::InitCA3D30 | In outline: the same (another copy) |
| `0xc5be30` | Device::StepC5BE30 | In outline: a step with the device's part (+0x10 +0x24C) and a value (+0x20) |
| `0xc56fb0` | Device::InitC56FB0 | In outline: fills a record from its arguments |
| `0xc2fd20` | Resource::SharedC2FD20 | In outline: once (bit 0 of `0x13E6990`), sets up a shared value |
| `0xc23650` | Math::CosineRuleC23650 | Gives (b*b + c*c - a*a) / (2*b*c) for three side lengths (the cosine rule), limited |
| `0xc93cc0` | Batches::EntryC93CC0 | In outline: with entry n (12 bytes at +0x21AC) set, a step |
| `0xc0fc50` | Text::FindC0FC50 | In outline: with a part (+0x1114), a find by a C string |
| `0xc09270` | Text::FlagC09270 | In outline: with a part (+0x2C), a step when a flag is set (Flags::Bit0) |
| `0xbf9ee0` | Text::StepBF9EE0 | In outline: a step (`0xC30290`) with a work buffer |
| `0xbea150` | Record::InitBEA150 | In outline: sets a count to 1 and makes a four-byte buffer |
| `0xbb2350` | Track::CopyBB2350 | In outline: copies 18 values from another record |
| `0x49ab40` | List::Add49AB40 | In outline: adds when there is room (+8, +0xC) |
| `0xcc2de0` | Shader::StepCC2DE0 | In outline: a step with work values |
| `0xcc2a70` | Shader::FreeCC2A70 | In outline: with a shared object (`0x13F70D8`), deletes it |
| `0xc377c0` | Resource::AddC377C0 | In outline: adds when there is room (+8, +0xC) |
| `0xc36530` | Resource::FreeC36530 | In outline: tears down references (ResourceRef) |
| `0xc04230` | Device::SizeC04230 | In outline: a format size (Device::FormatSizeBD3C60) by a shared flag (`0x13D3170`) |
| `0xbec910` | Resource::MakeBEC910 | In outline: a step (`0xBEC570`) and makes a part |
| `0xbe66c0` | Array::SetBE66C0 | In outline: for n under a count, sets entry n |
| `0x45ad60` | Game::Initialise45AD60 | In outline: starts the game (Game::Initialise) with its arguments |
| `0x457210` | Game::Shared457210 | In outline: a step by shared flags (`0x10EDC80`, `0x1106DC0`, `0x1339FB4`, `0x1106C48`) |
| `0x419000` | Effects::List419000 | In outline: a step with a list (+0x20) |
| `0x40f800` | Effect::List40F800 | In outline: a step with a list (+8, +0xC) |
| `0x40bb90` | Effect::Init40BB90 | In outline: sets values to the largest float and more |
| `0xcef770` | Shapes::InitCEF770 | In outline: sets up nine 16-byte entries |
| `0xcbda80` | Memory::SharedCBDA80 | In outline: once, makes the shared pool (`0x13F7014`, `0xCBD050`) |
| `0xcbc870` | Text::CopyCBC870 | In outline: copies n bytes into a new buffer (malloc) with two zero bytes |
| `0xcb8140` | Record::InitCB8140 | In outline: fills a record from a value and clears another |
| `0xc2b840` | Record::InitC2B840 | In outline: clears a record |
| `0xc0f7b0` | Resource::SharedC0F7B0 | In outline: once, makes a shared value (`0x13E32F8`, `0xC0BE40`) |
| `0xc0adc0` | Text::CopyC0ADC0 | In outline: copies n bytes into a new buffer (malloc) with two zero bytes |
| `0xcbea20` | Engine::RefCBEA20 | In outline: adds one to a count and acts |
| `0x418880` | Engine::Make418880 | In outline: makes a 0x20-byte part |
| `0x4558f0` | Engine::Assign4558F0 | In outline: takes a reference to another's part when it differs |
| `0xc371a0` | Engine::ListC371A0 | In outline: a step with a list (+0x4C) |
| `0xcbe640` | Engine::RefCBE640 | In outline: adds one to a count and acts |
| `0xc0cc90` | Engine::ListC0CC90 | In outline: a step with a list (+0x94) |
| `0xc1b390` | Engine::MakeC1B390 | In outline: makes a 0x20-byte part |
| `0xc024b0` | Engine::MakeC024B0 | In outline: makes a 0x178-byte part |
| `0xcd84c0` | Engine::PassCD84C0 | In outline: goes over entries (+0xE0) |
| `0x40d4c0` | Engine::Make40D4C0 | In outline: makes a 0x4C-byte part |
| `0xc04060` | Device::FindC04060 | In outline: a find by a byte |
| `0xc03d90` | Device::StepC03D90 | In outline: a step by a shared flag (`0x13D3170`) |
| `0xc03d20` | Device::StepC03D20 | In outline: the same (another copy) |
| `0xbfd560` | Device::StepBFD560 | In outline: when not flagged (+0x24), a step |
| `0x4881c0` | Effect::List4881C0 | In outline: a step with an object's list |
| `0x4830f0` | Window::Style4830F0 | In outline: fills a record from its values (2 or 4 by a flag) |
| `0x475ba0` | Labels::Reset475BA0 | In outline: clears a flag (+0x48) and acts with a part (+0x40) |
| `0x45bb00` | Effect::Part45BB00 | In outline: a step when a part (+0x2FC, slot 8) has a value |
| `0xca4e50` | List::AddCA4E50 | In outline: with a part (+0x10), adds (List::AddCA4970) |
| `0xca4480` | Batches::InitCA4480 | In outline: clears a value and more |
| `0xc5bfd0` | Device::StepC5BFD0 | In outline: a step with the device's part (+0x10 +0x24C) and a value (+0x1C) |
| `0x495110` | Static::Step495110 | In outline: a step with a value |
| `0xcefcb0` | Shapes::EntryCEFCB0 | In outline: a step for an entry by the high bits of n (plus 9) and the low 20 bits |
| `0xca8850` | String::StepCA8850 | In outline: makes a string (String::String) and acts |
| `0xc6caa0` | Window::InitC6CAA0 | In outline: fills a record from its arguments |
| `0xbc40a0` | Track::ScaleBC40A0 | In outline: a check with an object's scale (Object::Scale) |
| `0xbad550` | Track::CountBAD550 | In outline: 0 with fewer than two entries (+0x1D8), else a step |
| `0xba4e40` | Track::PassBA4E40 | In outline: goes over entries down from a count (+8) |
| `0xba4dd0` | Track::PassBA4DD0 | In outline: goes over entries (+8) |
| `0xcc4210` | Shader::LoopCC4210 | In outline: a loop |
| `0xcbd050` | Memory::MakePoolCBD050 | In outline: makes a 0x28-byte record (malloc) |
| `0xca7e30` | Parser::FloatCA7E30 | In outline: reads a float from a C string (atof) and sets it |
| `0xca7c60` | Parser::FloatCA7C60 | In outline: the same (another copy) |
| `0xca7a90` | Parser::FloatCA7A90 | In outline: the same (another copy) |
| `0xca7020` | Parser::FloatCA7020 | In outline: the same (another copy) |
| `0xca7690` | Parser::LongCA7690 | In outline: reads a whole number from a C string (atol) and sets it |
| `0xca7ec0` | Parser::TextCA7EC0 | In outline: copies up to n characters into a 100-character buffer and sets it |
| `0xca7cf0` | Parser::TextCA7CF0 | In outline: the same (another copy) |
| `0xca7b20` | Parser::TextCA7B20 | In outline: the same (another copy) |
| `0xca70b0` | Parser::TextCA70B0 | In outline: the same (another copy) |
| `0xc69e20` | Window::InitC69E20 | In outline: fills a record from many values |
| `0xc532d0` | Stream::StepC532D0 | In outline: with values (+0x50, +0x54), a step when two others match |
| `0xc0be40` | Resource::MakeC0BE40 | In outline: makes a 0x28-byte record (malloc) |
| `0xbd6aa0` | Device::StateBD6AA0 | In outline: for state 0x12, sets a flag (+0x1A8) when it changes |
| `0xbc5510` | Track::CompareBC5510 | In outline: compares two values |
| `0x49ed70` | Text::Add49ED70 | In outline: adds when there is room (+0x28, +0x2C) |
| `0x499950` | Static::Step499950 | In outline: a step with two work values |
| `0x498e50` | Static::Entry498E50 | In outline: sets a value of entry n (0x18 bytes at +0x18) |
| `0x42f830` | Sky::Array42F830 | In outline: makes an aligned array of n entries of 0x70 bytes |
| `0x457da0` | Text::Init457DA0 | In outline: clears a record and copies a C string |
| `0x42e580` | Sky::Pass42E580 | In outline: goes over entries with a shared count (`0x1103C14`) |
| `0xcefc40` | Shapes::EntryCEFC40 | In outline: a step for an entry by the high bits of n and the low 20 bits |
| `0xcc2e60` | Shader::InitCC2E60 | In outline: a step (`0xCC2D00`) and clears a value (+0x10) |
| `0xcc1810` | Shader::InitCC1810 | In outline: fills a record from its arguments |
| `0xc413f0` | ShaderFile::ReadC413F0 | In outline: reads a four-byte value (ShaderFile::Read) and acts when asked |
| `0xc211c0` | Resource::MakeC211C0 | In outline: makes a part (Resource::MakeC43660 with `0x13E6B0C`) |
| `0xc0df20` | Text::InitC0DF20 | In outline: sets up a record with its table (`0xFAF9C4`) |
| `0xc09020` | Text::PartC09020 | In outline: with a part (+0x3C), a step |
| `0xbec570` | Ref::InitBEC570 | In outline: takes a reference to another's part |
| `0xbc52a0` | Track::CompareBC52A0 | In outline: compares two records |
| `0x4815b0` | Labels::Free4815B0 | In outline: frees a buffer (+4) and a part (+0x14), deleting when asked |
| `0xc46ce0` | Resource::SameC46CE0 | In outline: whether two records match (+4, then `0xC46A80`) |
| `0xc2c730` | Resource::InitC2C730 | In outline: sets up a record with its table (`0xFB01CC`) and a shared value (`0x10F3020`) |
| `0xc0a6e0` | Text::InitC0A6E0 | In outline: sets up a text from a C string, none as empty |
| `0xc07f20` | Windows::KeyC07F20 | In outline: a window message step with key states |
| `0xbf9f70` | Text::PassBF9F70 | In outline: with values (+0x50, +0x10), goes over entries |
| `0xbdab30` | Text::StepBDAB30 | In outline: a step with a 1024-character buffer |
| `0xbd0160` | WideString::AppendBD0160 | In outline: appends another two-byte text (WideString::MakeUnique) |
| `0xbbcdc0` | Track::ClearBBCDC0 | In outline: with a part (+0xC), clears it |
| `0xba09e9` | Track::FragmentBA09E9 | In outline: a fragment that sets a flag (+0x24) |
| `0x49f7b0` | Text::Init49F7B0 | In outline: fills 0x400 entries (+0xC) |
| `0xc636a0` | Window::FlagsC636A0 | In outline: flags (Window::FlagsC623F0) and a value (+0xF4) |
| `0xc21a70` | Record::StepC21A70 | In outline: when a value (+0x30) is at least 1, copies values |
| `0xbacb00` | Track::StepBACB00 | In outline: a step with cleared values (Zero6) |
| `0x483ba0` | Labels::Set483BA0 | In outline: sets a value (+0x10) when it changes, negatives handled |
| `0x46de00` | Labels::Add46DE00 | In outline: adds an entry (+8 plus one) |
| `0x46b920` | Labels::Add46B920 | In outline: grows a list by 8 when full (Array::Resize460F20) and adds |
| `0x42db10` | Sky::Pass42DB10 | In outline: goes over entries (+8) with a byte rectangle |
| `0xce3f80` | Mesh::WeldCE3F80 | In outline: welds (Mesh::WeldCE36A0) and more |
| `0xcc2660` | Shader::InitCC2660 | In outline: a step (`0xCC1E20`) and clears a value |
| `0xc6fd50` | Renderer::ClearC6FD50 | In outline: clears (Renderer::ClearC056C0) and a value (+0x67C) |
| `0xc67b40` | Window::SharedC67B40 | In outline: when a shared flag is set (`0x13E6D00` +0x134), a step (`0xC66CD0`) |
| `0xbff610` | Resource::InitBFF610 | In outline: sets up a record with its table (`0xFA5D0C`) |
| `0xbebcb0` | Resource::InitBEBCB0 | In outline: sets up a record with its table (`0xFA57B4`) |
| `0x496ea0` | Text::Step496EA0 | In outline: a step with a 64-character two-byte buffer |
| `0x466ad0` | Labels::Lerp466AD0 | In outline: a blend (Vector::Lerp2) of a shared entry (`0x12DFFF0`) |
| `0x435850` | Sky::Shared435850 | In outline: a step with a shared table (`0x10EC6F4`) |
| `0xcb09c0` | Zlib::SetCB09C0 | In outline: sets a stream (+0x18) and acts by its flags (+0x44) |
| `0xca3ed0` | Resource::StepCA3ED0 | In outline: a step with a part (+4) and a work buffer |
| `0xca3dd0` | Resource::StepCA3DD0 | In outline: the same (another copy) |
| `0xca3b10` | Resource::StepCA3B10 | In outline: the same (another copy) |
| `0xca3a50` | Resource::StepCA3A50 | In outline: the same (another copy) |
| `0xc71aa0` | Device::TakeC71AA0 | In outline: takes from a free list (FreeList13D0EB4::Take) and more |
| `0xc46ad0` | Model::WriteC46AD0 | In outline: writes an "ITBL" tag (Writer::Write4) and more when given |
| `0xc42500` | ShaderFile::ReadC42500 | In outline: reads a four-byte value (ShaderFile::Read) and acts when asked |
| `0xc42410` | ShaderFile::ReadC42410 | In outline: the same (another copy) |
| `0xc347f0` | Resource::SharedC347F0 | In outline: with a shared value (`0x13E6A24`), makes a part |
| `0xc2ea70` | Resource::InitC2EA70 | In outline: sets up a record with its table (`0xFB01CC`) |
| `0xc07c20` | Device::InitC07C20 | In outline: sets up a record with its table (`0xFA6270`) and a list |
| `0xbe6820` | Resource::StepBE6820 | In outline: with more than one entry (+0x32) and an empty first, a step |
| `0xbdbf70` | Text::StepBDBF70 | In outline: a step with a text |
| `0x49ccf0` | Text::Init49CCF0 | In outline: sets up a record with its table (`0xF30550`) and a list |
| `0xc69450` | Window::SetC69450 | In outline: a step when a set passes (Device::SetC5E520) |
| `0xc0a7c0` | Record::InitC0A7C0 | In outline: fills a record with 1 and points it at itself |
| `0xbecec0` | Resource::SharedBECEC0 | In outline: with a shared value (`0x13D23C0`), a step |
| `0xbc0c70` | Track::InitBC0C70 | In outline: sets up a record with its table (`0xFA0068`) |
| `0x428450` | Sky::Curve428450 | In outline: a curve step (Curves::Step426790) with a scaled value |
| `0xcbdfb0` | Buffer::FreeCBDFB0 | In outline: frees a buffer (+0xC) and a part (+0x10) |
| `0xbfc500` | Text::ValueBFC500 | In outline: with no value (+0x40), a value from another (+0x3C) |
| `0xbebaf0` | Resources::StepBEBAF0 | In outline: with no part (+0x18), a list step (Resources::ListBEAFC0) |
| `0x408550` | Effect::Copy408550 | In outline: copies a record from another |
| `0xca7580` | Parser::TextCA7580 | In outline: copies up to n characters into a 100-character buffer and sets it |
| `0xca72f0` | Parser::TextCA72F0 | In outline: the same (another copy) |
| `0xca7260` | Parser::TextCA7260 | In outline: the same (another copy) |
| `0xca71d0` | Parser::TextCA71D0 | In outline: the same (another copy) |
| `0xca7140` | Parser::TextCA7140 | In outline: the same (another copy) |
| `0xca78b0` | Parser::TextCA78B0 | In outline: the same (another copy) |
| `0xca7490` | Parser::TextCA7490 | In outline: the same (another copy) |
| `0xbe8680` | Resource::CountBE8680 | In outline: with a part (+8), a count |
| `0x46d850` | Labels::Shared46D850 | In outline: a value when a shared object is not flagged (`0x12F0028` +0x5C) |
| `0x41a8e0` | Effects::Part41A8E0 | In outline: with no part (+0x50), a step |
| `0xceedc0` | Shapes::GrowCEEDC0 | In outline: makes a buffer of four bytes per entry plus one and copies |
| `0xce2310` | Mesh::GrowCE2310 | In outline: the same (another copy) |
| `0xce22b0` | Mesh::GrowCE22B0 | In outline: the same (another copy) |
| `0xce2250` | Mesh::GrowCE2250 | In outline: the same (another copy) |
| `0xcc8320` | Model::GrowCC8320 | In outline: the same (another copy) |
| `0xcaf6e0` | Resource::EntryCAF6E0 | In outline: for n not negative and no part (+0x5C), a step |
| `0xc0d460` | Text::CountC0D460 | In outline: with a part (+4), a count |
| `0xbcf770` | Text::InitBCF770 | In outline: sets a room rounded up to 16 and more |
| `0x48d9b0` | Labels::Fade48D9B0 | In outline: a step with twice a value (+0x14), above 1 handled |
| `0x46f650` | Labels::Set46F650 | In outline: grows by 0x20 when n passes a count (Array8::SetCapacity) and sets |
| `0xce6820` | Camera::SetCE6820 | In outline: sets a value when it changes |
| `0xcbdcc0` | Memory::ListCBDCC0 | In outline: a step with a list (+0x44) |
| `0xc62610` | Window::DrawC62610 | In outline: a draw (Window::DrawC62510) |
| `0xc43f90` | ShaderFile::ReadC43F90 | In outline: reads a four-byte value (ShaderFile::Read) and acts on success |
| `0xbfbe80` | Device::UnlinkBFBE80 | In outline: unlinks a part (Device::LinkC5CB30 on +0x60) and clears it |
| `0x420590` | Effects::Copy420590 | In outline: a step with another's values |
| `0xce5850` | Mesh::SharedCE5850 | In outline: a step with a shared table (`0x10F45EC`) |
| `0xca36f0` | Resource::StepCA36F0 | In outline: a step with a part's value (+4 +0xE8) |
| `0xc6c530` | Window::InitC6C530 | In outline: fills a record from many values |
| `0xc43be0` | Array::GrowC43BE0 | In outline: grows (Array::GrowC43840) when n passes a count (+0x24) |
| `0xc40420` | Resource::SlotC40420 | In outline: a slot step (Resource::SlotC3B3C0) and more (`0xC3D090`) |
| `0xbfc350` | Text::ValueBFC350 | In outline: with no value (+0x3C), a value from another (+0x38) |
| `0xba92f0` | Track::TimeBA92F0 | In outline: takes n from a time (+0x9C) and checks it |
| `0x45e210` | Effect::Kind45E210 | In outline: for kind 3 (+4) with no part (+0xC), a step |
| `0x45e1b0` | Effect::Kind45E1B0 | In outline: for kind 1 (+4), a step |
| `0x448410` | Effect::Part448410 | In outline: with a part (+0xE8), a step (`0x83DBC0`) |
| `0xcd81f0` | Camera::PassCD81F0 | In outline: when on (+0xF8), goes over entries (+0x10C, +0x110) |
| `0xc429e0` | Engine::MakeC429E0 | In outline: makes a 0xC-byte part |
| `0xc0cc30` | Engine::ListC0CC30 | In outline: a step with a list (+0x7C) |
| `0x47e680` | Engine::Pass47E680 | In outline: goes over entries (+0xD8) |
| `0xc0cbd0` | Engine::ListC0CBD0 | In outline: a step with a list (+0x64) |
| `0x45f920` | Engine::Assign45F920 | In outline: takes a reference to another's part when it differs |
| `0x471df0` | Engine::Init471DF0 | In outline: sets up a record with its table (`0xF2CCF8`) |
| `0xce3400` | Engine::MakeCE3400 | In outline: makes a 0xC-byte part |
| `0xcb5780` | Engine::AssignCB5780 | In outline: frees its buffer and remakes it from another |
| `0x49bbc0` | Engine::Free49BBC0 | In outline: tears down references (ObjectRef) |
| `0xce9160` | Engine::PassCE9160 | In outline: goes over entries (+0x1C) |
| `0xc361e0` | Engine::ListC361E0 | In outline: a step with a list (+0x64) |
| `0x435140` | Engine::Make435140 | In outline: makes a 0xC-byte part |
| `0x455f80` | Engine::Init455F80 | In outline: sets up a record with empty texts (WideString::Empty) |
| `0xceb3c0` | Engine::MakeCEB3C0 | In outline: makes a 0x4-byte part |
| `0x40c160` | Engine::Make40C160 | In outline: makes a 0x4-byte part |
| `0xc36830` | Engine::MakeC36830 | In outline: makes a 0x14-byte part |
| `0xc00880` | Engine::PassC00880 | In outline: goes over entries (+0x10C) |
| `0xceb930` | Engine::PassCEB930 | In outline: goes over entries (+0x20) |
| `0xbf4690` | Engine::PassBF4690 | In outline: goes over entries (+0x14) |
| `0xcb5a50` | Shader::PartCB5A50 | In outline: with a part (+0xA0) and none other (+0xAC), a step |
| `0xc502a0` | Stream::StepC502A0 | In outline: a step with a part (+0x28, slot 8) and acts with another (+0x14) |
| `0xc064a0` | Device::ListC064A0 | In outline: a step with a list |
| `0xc05d20` | Device::ResetC05D20 | In outline: clears values (+0x4B0, +0x4BC, ...) |
| `0xbc2c20` | Track::MaxBC2C20 | In outline: raises a shared value (`0x13D0A4C`) to a two-byte count (+4) |
| `0x495ef0` | Static::Count495EF0 | In outline: with no entries (+0xC8), a step |
| `0x48a450` | Effect::Add48A450 | In outline: adds to a slot pool (SlotPool::Add) |
| `0x4899b0` | Effect::Step4899B0 | In outline: with a part, a step with an object's value (+0x10) |
| `0x47e260` | Labels::Flag47E260 | In outline: copies a shared flag (`0x10EDB98`) into a value (+0xC1) |
| `0x4767d0` | Labels::Add4767D0 | In outline: when flagged (bit 0 of +0x74), adds when there is room (+0x80, +0x84) |
| `0x437960` | Sky::Make437960 | In outline: with no part (+0x2E4), makes one (Labels::Make46A9F0) |
| `0xc907d0` | Batches::SharedC907D0 | In outline: once (bit 0 of `0x13E6DAC`), sets up a shared value |
| `0xc3b8e0` | Bits::SizeC3B8E0 | In outline: a step when the number of 32-bit words for n changes |
| `0xbdd290` | HashTable::FindBDD290 | In outline: a find by a text's hash (Text::StepBDBF70) remainder |
| `0xbdd020` | HashTable::FreeBDD020 | In outline: frees a buffer (+4) and a part (+0xC), deleting when asked |
| `0xbdc770` | VertexDeclaration::AddBDC770 | In outline: adds a two-byte entry to its storage (+0xC) |
| `0xbd5300` | Device::StepBD5300 | In outline: a step with the device (slot 0xCC) and a value |
| `0x410750` | Effect::Make410750 | In outline: makes a part |
| `0x409370` | Device::Step409370 | In outline: a step with halved values |
| `0xc10650` | Parser::FindC10650 | In outline: makes a text (Parser::MakeC0AEE0) and a find (Text::FindC0DA10) |
| `0xbe8240` | Resource::CaseBE8240 | In outline: a step by a check (Resource::CheckBE7A00) |
| `0x498460` | Static::Value498460 | In outline: a value with a value (+0x40) and work values |
| `0xcbc490` | HashTable::FindCBC490 | In outline: a find by a hash (`0xCBBB00`) remainder |
| `0xcb5e60` | Buffer::FreeCB5E60 | Frees a buffer and clears the pointer |
| `0xcadd90` | Ref::InitCADD90 | In outline: takes a reference to another's part and more |
| `0xcad810` | Resource::ColonCAD810 | In outline: a step with the position of a colon (`0xBCF3E0` with 0x3A), -1 handled |
| `0xc46d80` | ShaderFile::ReadC46D80 | In outline: reads a four-byte value into +4 and acts on success |
| `0xc1b2d0` | Resource::FreeC1B2D0 | In outline: sets its table (`0xFAFF14`), tears down a list and frees when asked |
| `0xbcb040` | Track::StepBCB040 | In outline: a step with an object's value (+0x1C) and a list (+8) |
| `0xbc0a00` | Memory::AlignBC0A00 | In outline: rounds a value (+4) up to a multiple of n and adds |
| `0x48ca90` | Effect::Step48CA90 | In outline: a step with values |
| `0x413810` | Effect::Free413810 | In outline: sets its table (`0xF26E00`), tears down a list and frees when asked |
| `0x48cb60` | TextRef::Assign48CB60 | In outline: assigns a text reference (TextRef::Assign) |
| `0x48c960` | Effect::Step48C960 | In outline: with a part and an object's value (slot 0x28), a step |
| `0xc62670` | Window::SharedC62670 | In outline: unless flagged (0x80000), a step with a shared part (`0x13E6D00` +0x108) |
| `0xc383e0` | Resource::MakeC383E0 | In outline: makes a part and acts with a list (+4 +0x10) |
| `0xc37220` | Resource::CheckC37220 | In outline: with no part (+4), reports a message (Report) |
| `0xc32e80` | Resource::ListC32E80 | In outline: a step with a list |
| `0xc01ab0` | Device::InitC01AB0 | In outline: sets up a record with its table (`0xFA5E00`) |
| `0xbe9040` | Resource::StepBE9040 | In outline: a step with work values |
| `0xbcf980` | Text::GrowBCF980 | In outline: grows its room (rounded up to 16) when n passes it |
| `0x4951b0` | Static::Free4951B0 | In outline: frees a buffer (+0x1C) and more |
| `0x4762c0` | Labels::Step4762C0 | In outline: when on (+0x6C), a step (`0x475D50`) |
| `0x4364d0` | Sky::Time4364D0 | In outline: a step with a time entry (8 bytes by +0x328) |
| `0x436430` | Sky::Time436430 | In outline: records the tick count (Timer::Ticks) into a time entry |
| `0x40c4f0` | Effect::Entry40C4F0 | In outline: a value for entry n under a count (+0x38), a fixed one outside |
| `0xcc2160` | Shader::UnlinkCC2160 | In outline: when linked (ListNode::IsLinked), a step |
| `0xc66cd0` | Window::StepC66CD0 | In outline: a step by a flag (+0x70), a shared flag (`0x13E6D00` +0x134) and a part (+0xC0) |
| `0xc32b40` | Text::InitC32B40 | In outline: fills a record from two bytes and a C string |
| `0xbfdbc0` | Resource::PartBFDBC0 | In outline: with a count (+0x24), a step with a part (+0x20 +4) |
| `0xbfd8f0` | Resource::PartBFD8F0 | In outline: the same (another copy) |
| `0xbcf9e0` | Text::InitBCF9E0 | In outline: fills a record with 1 and a room rounded up to 16 |
| `0x4278b0` | Sky::Step4278B0 | In outline: a step |
| `0x4278b0` | Sky::Init4278B0 | In outline: sets up a record with its table (`0xF27CAC`) and a shared value (`0x1103B6C`) |
| `0xcbbb00` | Text::HashCBBB00 | In outline: a hash of a text |
| `0xcb80f0` | Resource::PassCB80F0 | In outline: goes over entries down from n |
| `0xc631d0` | Window::StepC631D0 | In outline: clears a flag (+0x9C) and acts with a part (+0x14) |
| `0xc05150` | Device::CopyC05150 | In outline: with a part (+0x4EC +0xC), makes a 0x1F0-byte copy |
| `0x41bc70` | Effects::Step41BC70 | In outline: a step with values |
| `0xc9c3c0` | Batches::InitC9C3C0 | In outline: sets a value and points a record at itself |
| `0x488070` | Effect::Billboards488070 | In outline: sets a value (+0x100) and sets up billboards (Billboards::Setup2) |
| `0x475d00` | Labels::Add475D00 | In outline: grows by 10 when full (`0x475A30`) and adds |
| `0x475cb0` | Labels::Add475CB0 | In outline: the same (another copy) |
| `0x40c260` | Effect::Entry40C260 | In outline: adds a value to entry n (0x24 bytes) and acts |
| `0xcb0b60` | Zip::CheckEndCB0B60 | Checks a zip end-of-central-directory record (signature 0x06054B50, disk 0, matching counts) |
| `0xc37b40` | Resource::CheckC37B40 | In outline: with an object, a step (slot 4) |
| `0xc053c0` | Device::SetPointC053C0 | In outline: copies a point into its values (+0x4C8 on) |
| `0xbe9dd0` | Resource::PartBE9DD0 | In outline: with a count (+0x20), a step with a part (+0x18 +4) |
| `0xbe9be0` | Resource::PartBE9BE0 | In outline: the same (another copy) |
| `0xbe6920` | Resource::InitBE6920 | In outline: fills a record from its arguments |
| `0xbd48f0` | Device::PartBD48F0 | In outline: with a part (+0xB0), a step |
| `0xbd3750` | Device::SetBD3750 | In outline: sets a flag (+0x11) and acts with a list (+0x14) |
| `0xbd2960` | Table::ResetBD2960 | In outline: clears a value (+0x90) and goes over entries (+4) |
| `0xbc3f90` | Track::NearBC3F90 | In outline: whether two points are within a distance |
| `0x4c13c0` | Shop::List4C13C0 | In outline: goes over a list (+0xCC) |
| `0x4b48a0` | Ride::Free4B48A0 | In outline: tears down a listener (Listener440) |
| `0x49bc60` | List::Free49BC60 | In outline: goes over a list's entries and frees them |
| `0x475c10` | Labels::Shared475C10 | In outline: sets a shared value (`0x10ECA08`) and goes over a shared table (`0x10ECA14`) |
| `0xcad9f0` | Resource::StepCAD9F0 | In outline: with a part (+0x60), a step |
| `0xca56a0` | Table::StepCA56A0 | In outline: a step with an object's value |
| `0xc64c90` | Window::StepC64C90 | In outline: clears a flag (+0x9C) and acts with a part (+0xF4) |
| `0xc21270` | Record::CompareC21270 | In outline: compares two records by their first float |
| `0xc16750` | Resource::SizeC16750 | In outline: a step with a size (+8 less +0x14) |
| `0xc099a0` | Text::ParseC099A0 | In outline: goes over a C string |
| `0xc08890` | Device::SharedC08890 | In outline: once (bit 0 of `0x13D3298`), sets up a shared value |
| `0xbe6730` | Resource::PassBE6730 | In outline: goes over entries (+4) |
| `0xbd5200` | Device::ResetBD5200 | In outline: clears a value (+0x50) and more |
| `0xbcfc30` | Text::GrowBCFC30 | In outline: grows its room (rounded up to 16) when n passes it |
| `0xbab6f0` | Track::StepBAB6F0 | In outline: a step with values, 1 handled |
| `0x499e40` | Static::Value499E40 | In outline: a value from a step (Static::Value498460) and 6 |
| `0x48c8e0` | Effect::Query48C8E0 | In outline: with a part, a query step with a work buffer |
| `0x48c5d0` | Effect::Query48C5D0 | In outline: the same (another copy) |
| `0x4614f0` | List::Entry4614F0 | In outline: for entry n under a count (+0x18) with a positive count (+0x44), a value |
| `0x4093d0` | Device::Step4093D0 | In outline: a step with a point |
| `0xca6100` | Table::StepCA6100 | In outline: when not flagged (+0x30) and under a count (+0x38, +0x40), a step |
| `0xca2b40` | Text::CheckCA2B40 | In outline: a check with a text |
| `0xc50340` | Stream::RangeC50340 | In outline: a step (+0x28, slot 8) and for n in a range (+0x60, +0x64), more |
| `0xc3ace0` | Resource::CheckC3ACE0 | In outline: false for -1, else a check |
| `0xc30cc0` | Resource::ValueC30CC0 | In outline: a value from a part (+0x14, slot 0x14), none handled |
| `0xc0e3c0` | Resource::InitC0E3C0 | In outline: sets up a record and references (Refs20 with `0x13E3318` +0x198) |
| `0xbe8d60` | Resource::EmptyBE8D60 | In outline: with a part (+0x1C), empties a range |
| `0xbcddf0` | Ref::ReleaseBCDDF0 | In outline: takes one from a count and frees at zero |
| `0xbacc10` | Track::CountBACC10 | In outline: for a positive count, goes over entries |
| `0x481450` | Labels::Free481450 | In outline: frees a buffer (+4) and a part (+0x14) |
| `0x480f80` | Labels::Camera480F80 | In outline: clears a flag (+0x20) and copies a camera (Camera::CopyFrom) |
| `0x475720` | Labels::Set475720 | In outline: when on (+0x6D), sets a value (+0x20) |
| `0x412ea0` | Effect::Pass412EA0 | In outline: goes over a list (+4) |
| `0xce46f0` | Mesh::CopyCE46F0 | In outline: copies a buffer (Memory::CopyCE6400 of +0x10) into +0x14 |
| `0xcdef80` | Loop::PartCDEF80 | In outline: with a part, a step |
| `0xcc1dd0` | Shader::InitCC1DD0 | In outline: sets up a record with its table (`0xFC3184`) |
| `0xca84a0` | Table::StepCA84A0 | In outline: a step with two values (+8, +0xC) |
| `0xc4c490` | Resource::FreeC4C490 | In outline: frees a part and itself when asked |
| `0xc259c0` | Resource::SharedC259C0 | In outline: with a shared part (`0x13E4920`), tears it down |
| `0xc16890` | Resource::StepC16890 | In outline: a step with many values |
| `0xbe7db0` | Resource::PartBE7DB0 | In outline: with no value (+0x14) and a part (+4), a step |
| `0xbe79b0` | Record::InitBE79B0 | In outline: fills a record from two values |
| `0xbdf790` | VertexDeclaration::AddBDF790 | In outline: adds a two-byte entry to its storage |
| `0xbbe5c0` | Track::ByteBBE5C0 | In outline: a step with a byte from a part (+4 +0x15D plus n) |
| `0xbbcc90` | Track::LoopBBCC90 | In outline: goes over a list until its own storage |
| `0x475da0` | Labels::Count475DA0 | In outline: adds or takes one from a shared count (`0x12F0140`), not below 0 |
| `0xce5960` | Mesh::FreeCE5960 | In outline: sets its table (`0xFC7FAC`), frees a part (+0x14) and itself when asked |
| `0xc614e0` | Window::FadeC614E0 | In outline: a fade (Window::FadeC61330) and a device step (Device::StepC5D920) |
| `0xc60f40` | Window::ResetC60F40 | In outline: clears flags (+0x9C, +0xE8) and acts |
| `0xc4d530` | Buffer::FreeC4D530 | In outline: frees a buffer and a part, deleting when asked |
| `0xc3b810` | Resource::StepC3B810 | In outline: a step with a work value |
| `0xbfc150` | Text::FlagsBFC150 | In outline: a value unless three flags are set (bits 0-2 of +0x7C) |
| `0xbe4140` | Module::StepBE4140 | In outline: a step when a module check passes (Module::CheckBE40D0) |
| `0xbda040` | Text::StepBDA040 | In outline: a step with a text |
| `0x4179b0` | Effects::Array4179B0 | In outline: makes an aligned array of n entries of 16 bytes |
| `0x413c70` | Effect::Colour413C70 | In outline: packs a colour from scaled values |
| `0xc58770` | Text::CheckC58770 | In outline: a check with a text |
| `0xc3c210` | Buffer::FreeC3C210 | In outline: with a buffer (+8), frees it |
| `0xc1e790` | Vector::StepC1E790 | In outline: a step with a point |
| `0xbc7f90` | Track::ScaleBC7F90 | In outline: a step with a scaled value (`0x10F2B30`) for modes 1 or 3 |
| `0xbc78a0` | Track::JoinBC78A0 | In outline: whether a join (Track::JoinBC6F00) gives all zero values |
| `0x497100` | Static::Save497100 | In outline: a text step (Static::Text496F70) and writes four bytes (SaveWriter::Write) |
| `0x496e50` | Module::Loop496E50 | In outline: a module loop (Module::LoopBE5360) |
| `0x496dc0` | Module::Loop496DC0 | In outline: the same (another copy) |
| `0x45ef10` | Table::Entry45EF10 | In outline: for n under 0x10000 and m under 7, a value under a count (+8) |
| `0x41bd00` | Effects::Point41BD00 | In outline: a step with a point |
| `0x4152a0` | Effects::Init4152A0 | In outline: clears a value (+0x20) and goes over two-byte entries |
| `0x408700` | Effect::Entry408700 | In outline: a step with an entry by the high half of n and halved values |
| `0xceb140` | Camera::FindCEB140 | In outline: a step when a find passes (IdMap::Find3) |
| `0xcc2110` | Shader::UnlinkCC2110 | In outline: with no part (+4) and a count (+8), unlinks (Shader::UnlinkCC2160) |
| `0xcc20c0` | Shader::CallCC20C0 | In outline: a call (CallCC1D70) that sets three values |
| `0xc46a80` | Text::CompareC46A80 | In outline: compares two two-byte texts, none handled |
| `0xc3d090` | Resource::EntryC3D090 | In outline: for entry n under a count (+0x2C), a step with a value (+0x78) |
| `0xc37bc0` | Resource::CheckC37BC0 | In outline: with an object, a step (slot 4) |
| `0xc09bf0` | Text::CaseC09BF0 | In outline: a step by case (+4: 2, ...) |
| `0xbbcbc0` | Track::ListBBCBC0 | In outline: a step with a list |
| `0xba95b0` | Track::CheckBA95B0 | In outline: a step with the opposite of a check (+0x40, slot 0x3C) |
| `0x475c60` | Labels::Add475C60 | In outline: grows by 10 when full (`0x475A30`) and adds |
| `0xceff50` | Shapes::DeviceCEFF50 | In outline: a device step (slot 0xB8) |
| `0xcebc00` | Camera::SlotCEBC00 | In outline: with a slot (+8), a step |
| `0xcc4840` | Shader::EntryCC4840 | In outline: for n under a count (+0x14), entry n (0x1C bytes by +0x18) |
| `0xc659a0` | Window::PassC659A0 | In outline: goes over a list (+0xB0) |
| `0xbfc290` | Text::CopyBFC290 | Copies a value (+0x10) into two (+0x5C, +0x74) and more |
| `0xba8db0` | Track::CheckBA8DB0 | In outline: when not flagged (+0x158), a step with no part |
| `0x468800` | Labels::Free468800 | In outline: sets its table (`0xF2B950`) and acts with a part (+8) |
| `0xce3650` | Record::InitCE3650 | In outline: clears a record |
| `0xcdbab0` | Record::InitCDBAB0 | In outline: clears a record |
| `0xcc9050` | Shader::MaxCC9050 | In outline: raises a shared value (`0x13FA024`) to n |
| `0xc501d0` | Stream::StepC501D0 | In outline: unless flagged (bit 13), a step with a part (+0x28, slot 8) |
| `0xc20fb0` | Resource::ReleaseC20FB0 | In outline: for none or an empty one, takes one from a count (+0x1C) |
| `0xc0a620` | Text::InitC0A620 | In outline: sets up a text from a C string, none as empty |
| `0xbc53d0` | Track::MoveBC53D0 | In outline: adds scaled values (`0x10F2B30`, `0x10F2B34`) to a position |
| `0xbbe570` | Track::StepBBE570 | In outline: a step with many values |
| `0xcec040` | Engine::PassCEC040 | In outline: goes over entries (+0x70) |
| `0xbd2850` | Engine::PartBD2850 | In outline: a step with a part (+0x4) |
| `0x471370` | Engine::Pass471370 | In outline: goes over entries (+0x114) |
| `0x40c490` | Engine::Pass40C490 | In outline: goes over entries (+0x38) |
| `0xce2400` | Engine::PassCE2400 | In outline: goes over entries (+0x14) |
| `0xc3a2b0` | Engine::ReleaseC3A2B0 | In outline: takes one from a count (+4) and frees at zero |
| `0xce2450` | Engine::PassCE2450 | In outline: goes over entries (+0x14) |
| `0xc50b20` | Engine::ListC50B20 | In outline: a step with a list (+0xCC) |
| `0xc059c0` | Engine::CopyC059C0 | In outline: copies values from another record |
| `0xbf5240` | Engine::ListBF5240 | In outline: a step with a list (+0x1C) |
| `0x438c10` | Engine::Make438C10 | In outline: makes a 0x10-byte part |
| `0xc38350` | Engine::ReleaseC38350 | In outline: takes one from a count (+4) and frees at zero |
| `0xc2f210` | Engine::ListC2F210 | In outline: a step with a list (+0x4) |
| `0xbfa8a0` | Engine::ListBFA8A0 | In outline: a step with a list (+0x4) |
| `0x45e8c0` | Engine::Assign45E8C0 | In outline: frees its buffer and remakes it from another |
| `0xc00970` | Engine::PassC00970 | In outline: goes over entries (+0x164) |
| `0xce21f0` | Engine::PassCE21F0 | In outline: goes over entries (+0x14) |
| `0xca1d50` | Engine::PassCA1D50 | In outline: goes over entries (+0x4) |
| `0xc62290` | Engine::ListC62290 | In outline: a step with a list (+0xB0) |
| `0xce6370` | Camera::LimitCE6370 | In outline: for n under 100, a step (slot 0x44 with 100) |
| `0xc5ded0` | Window::SetPointC5DED0 | In outline: copies a point into its values (+0xD0 on) |
| `0xc35f10` | Resource::FindC35F10 | In outline: a find over entries (+0xB4) |
| `0xbd0f90` | String::FindBD0F90 | In outline: makes a part and a find (String::Find) |
| `0xbc5e20` | Track::ValueBC5E20 | In outline: a value with a part (+0x20), none handled |
| `0xbaef20` | Track::CountBAEF20 | In outline: a step when a count is 0 (Track::CountBAD550) |
| `0xba4b90` | Track::EntryBA4B90 | In outline: a step with entry n's part (0x14 bytes at +0x58) |
| `0x4797e0` | Labels::Shared4797E0 | In outline: a step with a shared entry (`0x12F0154`) when it differs |
| `0x46c170` | Labels::Owner46C170 | In outline: a step with its owner (SceneryBehaviour::Owner) |
| `0x4689a0` | Labels::Free4689A0 | In outline: sets its table (`0xF2B950`), acts with a part (+8) and frees when asked |
| `0x4465a0` | Module::Check4465A0 | In outline: clears a value (+0x1004) and a module check (Module::CheckBE40D0) |
| `0x42fdf0` | Sky::Free42FDF0 | In outline: with a buffer (+0x64), frees it |
| `0xce4c70` | Mesh::CheckCE4C70 | In outline: 0 when on (+0x20), else a check |
| `0xcadc50` | Resource::EntryCADC50 | In outline: for n not negative and no part (+0x5C), a step |
| `0xc5fff0` | Window::StepC5FFF0 | In outline: clears a flag (+0x9C) and acts with a part (+0x100) |
| `0xc58060` | Text::CompareNoCaseC58060 | In outline: compares C strings without case |
| `0xc51900` | Stream::StepC51900 | In outline: a step with a shared table entry (`0x13E6BB0`) |
| `0xc50250` | Stream::StepC50250 | In outline: a step (+0x28, slot 8) and acts by values (+0x3C, +0x44) |
| `0xc1aba0` | Resource::SlotC1ABA0 | In outline: with slot n's part (+4), a step |
| `0xbd29e0` | Table::ResetBD29E0 | In outline: clears a value (+0x114) and goes over entries (+4) |
| `0xbe9b90` | Resource::FindBE9B90 | In outline: a find over entries (+0x18) |
| `0xbe06e0` | Buffer::FreeBE06E0 | In outline: frees a buffer (+0xC) and a part (+0x10) |
| `0xbe0690` | Buffer::FreeBE0690 | In outline: the same (another copy) |
| `0xbb65a0` | Bits::SizeBB65A0 | In outline: a step with the number of 32-bit words for n |
| `0x4984c0` | Static::Reset4984C0 | In outline: sets a flag (+0x70) and goes over entries (+0x60) |
| `0x467a30` | Labels::Mode467A30 | In outline: when a shared mode (`0x12EFFF0`) changes, a step with a shared table (`0x10EC8F4`) |
| `0x448380` | Effect::Width448380 | In outline: a step with a width |
| `0x41ab40` | Effects::Rect41AB40 | In outline: a step with a rectangle from two points |
| `0x417010` | Regions::Merge417010 | In outline: merges another's values (+0xC, +0x38) |
| `0xcee340` | Shapes::InitCEE340 | In outline: sets up a record with its table (`0xFC88B8`) and a value |
| `0xce35a0` | Mesh::CopyCE35A0 | In outline: grows an array (Array8::SetCapacity) to another's size and copies |
| `0xccb3c0` | Shader::InitCCB3C0 | In outline: sets up a record with its table (`0xFC320C`) and a value |
| `0xcad970` | Resource::CheckCAD970 | In outline: when not flagged (+0xF9), a step by a global (`0x13D0DF1`) |
| `0xc65310` | Window::SlotC65310 | In outline: a value by a slot (+0xF4), -1 handled |
| `0xc46ec0` | ShaderFile::ReadC46EC0 | In outline: reads a four-byte value (ShaderFile::Read) |
| `0xc32ba0` | Text::InitC32BA0 | In outline: sets a value (+0x10) and points a record at itself |
| `0xc30890` | Resource::EntryC30890 | In outline: for n not negative, a step with a value (+0x20) |
| `0xc212d0` | Record::InitC212D0 | In outline: sets values to the largest float and more |
| `0xc165f0` | Bits::TestC165F0 | In outline: a bit step for n under a count of words (+0x10) |
| `0xc0fc00` | Text::FindC0FC00 | In outline: a find by name (FindByName0) into +0x1134 |
| `0xbd65d0` | HashTable::FindBD65D0 | In outline: a find by a key's remainder over buckets (+0xC) |
| `0xba4d80` | Track::LastBA4D80 | In outline: 0 with no entries (+8), else the last |
| `0x418830` | Effects::Init418830 | In outline: sets a value and a record (Record::InitBE79B0) with a callback |
| `0x41ab90` | Effects::Rect41AB90 | In outline: a step with a rectangle from a point and a size |
| `0xce8280` | Camera::RegistryCE8280 | In outline: a step with entry n (0x70 bytes) of a registry (Registry::Ensure2) |
| `0xc5d720` | Window::StepC5D720 | In outline: a step with a flag (+0x11) and a value (+0x10) |
| `0xc0a670` | Text::CopyC0A670 | In outline: copies a C string from another |
| `0x4e5d40` | Ride::Point4E5D40 | In outline: a step with a point |
| `0x418090` | Effects::Grow418090 | In outline: grows when a count (+0xC) plus n passes its room (+0x14) |
| `0x40be10` | Effect::Find40BE10 | In outline: a step when a find passes (IdMap::Find4) |
| `0xcc4f30` | Shader::InitCC4F30 | In outline: fills a record from its arguments |
| `0xcc3e60` | Shader::InitCC3E60 | In outline: fills a record from its arguments |
| `0xcc2c30` | Shader::TakeCC2C30 | In outline: adds one to a count (+0x1000) and clears a value |
| `0xcc2ae0` | Shader::InitCC2AE0 | In outline: sets up 0x400 four-byte entries and a count of 0x400 |
| `0xc574e0` | File::CheckC574E0 | In outline: a check on a file name |
| `0xc43910` | Resource::PassC43910 | In outline: goes over entries |
| `0xc30620` | Resource::CheckC30620 | In outline: stops in the debugger when flagged (+0x18) |
| `0xc2b760` | Resource::PassC2B760 | In outline: goes over entries |
| `0xc19960` | Math::ScaleC19960 | In outline: n times a fixed scale (`0xFAFE94`), rounded, less one |
| `0xc19910` | Math::ScaleC19910 | In outline: the same with `0xFAFE90` |
| `0xc198c0` | Math::ScaleC198C0 | In outline: the same with `0xFAFE8C` |
| `0xc19880` | Math::ScaleC19880 | In outline: the same with `0xFAFE88` |
| `0xbdccc0` | Buffer::FreeBDCCC0 | In outline: frees a buffer (+4) and a part (+0xC) |
| `0xbda140` | ShaderFile::ReadBDA140 | In outline: reads a two-byte value (ShaderFile::Read) and acts when asked |
| `0x436df0` | Sky::Labels436DF0 | In outline: with a part (+0x35C), adds a label (`0x475D50`) |
| `0x431020` | Sky::Room431020 | In outline: a step when there is room (+8, +0xC) |
| `0x40c2b0` | Effect::Entry40C2B0 | In outline: for entry n (0x24 bytes) with no part (+0x14), a step |
| `0x408750` | Effect::Entry408750 | In outline: a step with an entry by the high half of n and halved values |
| `0xcec740` | Camera::EntryCEC740 | In outline: a step with entry n (0x70 bytes) of a made record |
| `0xce1670` | Camera::SharedCE1670 | In outline: goes over entries with a shared count (`0x140081C`) |
| `0xcdf0a0` | Loop::CaseCDF0A0 | In outline: a value by case (0, ...) |
| `0xcc2ed0` | Shader::StepCC2ED0 | In outline: a step (`0xCC44F0` with 0) and acts with a part (+0x10) |
| `0xcbab10` | Text::StepCBAB10 | In outline: a step with many values |
| `0xc16d10` | Resource::SharedC16D10 | In outline: with a shared list (`0x13E33A4`), a step |
| `0xbe9c30` | Resource::StepBE9C30 | In outline: with a value (+0x20), a step (slot 0xC) |
| `0xbd51c0` | Device::CheckBD51C0 | In outline: when an object is flagged (bit 0 of +0x1C) with a count (+0x18), a step |
| `0xbd2740` | Record::InitBD2740 | In outline: sets up a record with its table (`0xF264B0`) pointing at itself |
| `0xbcf3e0` | Text::FindCharBCF3E0 | In outline: the position of a character from n on, -1 past the end |
| `0xbc9570` | Track::StepBC9570 | In outline: a step with many values |
| `0x49ec10` | Text::Room49EC10 | In outline: a step when there is room (+8, +0xC) |
| `0x49e0b0` | Text::Value49E0B0 | In outline: a value from two points and a value (+0xC) with fixed scales |
| `0x475610` | Labels::Part475610 | In outline: with a part (+0x44), fills given values |
| `0x44a710` | Effect::Room44A710 | In outline: a step when there is room (+8, +0xC) |
| `0x4160d0` | Effects::Set4160D0 | In outline: frees a buffer (+0x14) and sets a value (+0x1C) |
| `0x40f140` | Effect::Flag40F140 | In outline: when asked, sets a flag (+0x10) |
| `0xcec4c0` | Camera::RoomCEC4C0 | In outline: a step when there is room (+8, +0xC) |
| `0xce9240` | Camera::EntryCE9240 | In outline: a step with entry n (0x70 bytes at +8) |
| `0xce7c80` | Camera::RoomCE7C80 | In outline: a step when there is room (+8, +0xC) |
| `0xcd2160` | Text::PassCD2160 | In outline: goes over a text |
| `0xcc58a0` | Ref::AssignCC58A0 | In outline: lets go of its part (Ref::ReleaseCC2250) and takes another's |
| `0xcc2050` | Ref::AssignCC2050 | In outline: the same (another copy) |
| `0xc694b0` | Window::HitC694B0 | In outline: when flagged (0x10000), a step when a point lies inside (Rect::Contains) |
| `0xc60a10` | Window::PartC60A10 | In outline: with a part (+0xE0), tells it (slot 0x80) |
| `0xbbd020` | Track::ListBBD020 | In outline: a step with a list |
| `0x4ded50` | Ride::Reset4DED50 | In outline: clears a value (+0x220) and a step with a part (+0xC, slot 0x14) |
| `0x4d0980` | Shop::Room4D0980 | In outline: a step when there is room (+8, +0xC) |
| `0x49aae0` | Static::Room49AAE0 | In outline: the same (another copy) |
| `0x476790` | Labels::Count476790 | In outline: adds one to a shared count (`0x12F0144`) and goes over a shared table (`0x10ECA14`) |
| `0x42e950` | Sky::Room42E950 | In outline: a step when there is room (+8, +0xC) |
| `0x42dda0` | Sky::Room42DDA0 | In outline: the same (another copy) |
| `0x40d440` | Effect::Room40D440 | In outline: the same (another copy) |
| `0x409b60` | Effect::Room409B60 | In outline: the same (another copy) |
| `0xcd7f60` | Camera::PartCD7F60 | In outline: with a part (+8), a step with its value (+0x10) |
| `0xcafdc0` | Zlib::CheckCAFDC0 | In outline: a check on a stream's state (+0x1C, +0x24) |
| `0xc57ac0` | Text::StepC57AC0 | In outline: a step with a text |
| `0xc57a80` | Text::StepC57A80 | In outline: the same (another copy) |
| `0xc168e0` | Resource::CheckC168E0 | In outline: with a part (+0x2C), a check (`0xC50B20`) |
| `0xc054c0` | Device::StepC054C0 | In outline: a step with the device's part (+0x24C) |
| `0xbe0a10` | VertexDeclaration::AddBE0A10 | In outline: adds an entry (VertexDeclaration::AddBDC770) and a value (Text::ValueBE03A0) |
| `0xbd20d0` | Array::BlockBD20D0 | In outline: with no free entry (+0x10), adds a block (Array::AddBlock) |
| `0x49aaa0` | Heap::Step49AAA0 | In outline: a heap swap (Heap::Swap) and a step with entry n (16 bytes) |
| `0x488290` | Effect::Init488290 | In outline: fills a record from its arguments |
| `0x47c200` | Labels::Shared47C200 | In outline: goes over a shared table (`0x10EDD60`) |
| `0x45ee10` | Table::Find45EE10 | In outline: a find over entries (+0x64) |
| `0xcec780` | Camera::EntryCEC780 | In outline: a step with entry n (0x70 bytes) of a made record |
| `0xceab10` | Buffer::FreeCEAB10 | In outline: frees two buffers |
| `0xce3210` | Mesh::AddCE3210 | In outline: grows by 0x100 when full (`0xCE2EF0`) and adds two two-byte values |
| `0xc9bd60` | Batches::InitC9BD60 | In outline: points a record at itself and sets a value to the largest float |
| `0xc674e0` | Window::PartC674E0 | In outline: with a part's part (+0x100 +0xC0 +4), a step |
| `0xc63730` | Window::SlotC63730 | In outline: -1 unless an object's owner (+0x18) is this one |
| `0xc60e60` | Window::AssignC60E60 | In outline: when a part (+0x14) differs, a step |
| `0xc30c80` | Resource::ResetC30C80 | In outline: clears a flag (+0x18) and acts with values (+0x10, +4) |
| `0xc0bdc0` | Text::ReleaseC0BDC0 | In outline: takes one from a two-byte count (+0x40) and acts at zero |
| `0xbd3810` | Record::InitBD3810 | In outline: sets up a record with its table (`0xFA153C`) pointing at itself |
| `0xbd37a0` | Record::InitBD37A0 | In outline: the same with `0xFA1538` |
| `0xbd19c0` | Record::SameBD19C0 | In outline: a step when a value (+0x10) differs from another's |
| `0x499910` | Static::Room499910 | In outline: a step when there is room (+8, +0xC) |
| `0x482a00` | Labels::Part482A00 | In outline: with a part (+0x1C), a step |
| `0x47ddc0` | Labels::Value47DDC0 | In outline: a value from n when positive |
| `0x47be30` | Labels::Value47BE30 | In outline: makes a part and a value when on (+0x14) |
| `0x40d480` | Effect::Room40D480 | In outline: a step when there is room (+8, +0xC) |
| `0xcb06a0` | Zlib::SetCB06A0 | In outline: sets a value (+0x54) and a stream step (Stream::SetC50B70) |
| `0xc55050` | Device::FlagC55050 | In outline: sets a shared flag (`0x13E6C60`) and acts by bit 1 (+0x10) |
| `0xc48190` | Resource::FreeC48190 | In outline: sets its table (`0xFB7158`), frees a buffer (+0x14) and itself when asked |
| `0xc413b0` | ShaderFile::ReadC413B0 | In outline: reads a two-byte value (ShaderFile::Read) and acts when asked |
| `0xc348b0` | Resource::MakeC348B0 | In outline: makes a part and a step with its value (+0x38) |
| `0xc32600` | Resource::PartC32600 | In outline: when not flagged (+0x108) with a part (+0x100), a step |
| `0xc0a8d0` | Text::InitC0A8D0 | In outline: sets up a record with its table (`0xFAF9C0`) and a text |
| `0xbdcb10` | Resource::ReleaseBDCB10 | In outline: releases a part (+0xC) and frees when asked |
| `0xbd6e10` | Draw::QuadBD6E10 | In outline: when not flagged (+0x1A0), sets a quad from -1 to 1 (Draw::QuadBD5C70) |
| `0xbd38c0` | Record::FreeBD38C0 | In outline: sets its table (`0xFA153C`) and acts when on (+0x118) |
| `0xbd3880` | Record::FreeBD3880 | In outline: sets its table (`0xFA1538`) and acts when on (+0x98) |
| `0xbce120` | Memory::AllocBCE120 | In outline: an allocation of n bytes with an alignment of 1 |
| `0xbcb000` | Track::PassBCB000 | In outline: goes over entries |
| `0xbb23c0` | Track::FreeBB23C0 | In outline: with a value (+0x40), frees memory (CoTaskMemFree on +0x44) |
| `0xba9530` | Track::InitBA9530 | In outline: sets its table (`0xF9D718`) and acts |
| `0x496500` | Ref::Release496500 | In outline: lets go of its part and frees when asked |
| `0x46b520` | Labels::Shared46B520 | In outline: sets a shared value (`0x10EC97C`) and goes over a shared table (`0x10EC980`) |
| `0x438f30` | Ref::Release438F30 | In outline: lets go of its part and frees when asked |
| `0x4030e9` | Static::Fragment4030E9 | In outline: a fragment that sets a value to 2 (+0x14) |
| `0xce9200` | Shapes::FillCE9200 | In outline: fills (Shapes::FillCE85B0) |
| `0xcc17b0` | Matrices::ResizeCC17B0 | In outline: resizes (Matrices::ResizeCC1420) |
| `0xcb06e0` | ShaderFile::MakeCB06E0 | In outline: for an empty name, makes (ShaderFile::MakeCB0510) |
| `0xcafd80` | Zlib::ResetCAFD80 | In outline: with a stream state (+0x1C), clears values and resets |
| `0xca4420` | Buffer::FreeCA4420 | In outline: with a buffer (+4), frees it |
| `0xc31280` | Resource::FreeC31280 | In outline: sets its table (`0xFB0318`) and tears down a list |
| `0xc2f190` | Record::InitC2F190 | In outline: fills a record with a value and clears another |
| `0xbfa820` | Record::InitBFA820 | In outline: the same (another copy) |
| `0xbcee80` | Text::FindCharBCEE80 | In outline: the position of a two-byte character from n on |
| `0xba9600` | Track::PartBA9600 | In outline: with a part (+0xAC), tells it (slot 4) |
| `0xba0490` | Track::FreeBA0490 | In outline: sets its table (`0xF6AC88`) and tears down a list |
| `0x441230` | Effect::Round441230 | In outline: a rounded scaled difference of two values (+0x48, +0x54) |
| `0x413680` | Effect::Free413680 | In outline: sets its table (`0xF26E00`) and tears down a list |
| `0x402dc0` | Static::Copy402DC0 | In outline: copies values from another |
| `0xcefd60` | Shapes::RoomCEFD60 | In outline: a step when there is room (+8, +0xC) |
| `0xcefd20` | Shapes::RoomCEFD20 | In outline: the same (another copy) |
| `0xcebdd0` | Camera::FreeCEBDD0 | In outline: with a part, frees it |
| `0xce7570` | Camera::FreeCE7570 | In outline: the same (another copy) |
| `0xcc2320` | List::LinkCC2320 | Links a node after another (sets each one's pointer to the other) |
| `0xc51360` | Stream::OpenC51360 | In outline: with a part (+0xE4), a step with an entry |
| `0xc30fc0` | Resource::SharedC30FC0 | In outline: for an object, a step with a shared table (`0x10F3040`) |
| `0xc1ab10` | Resource::ArrayC1AB10 | In outline: sets a count and makes an aligned buffer of n four-byte entries |
| `0xbfc570` | Device::UnlinkBFC570 | In outline: unlinks a part (Device::LinkC5CB30 on +0x1C) and clears it |
| `0xbfc3c0` | Device::UnlinkBFC3C0 | In outline: the same (another copy) |
| `0xbf65d0` | List::StepBF65D0 | In outline: a step with a list |
| `0xbe8870` | Resource::FreeBE8870 | In outline: frees (Resource::FreeBE87E0) and acts with a value (+4) |
| `0xbe3e60` | ModuleList::StartBE3E60 | In outline: when not started, starts a module list (ModuleList::Start10F2F90) |
| `0xba9350` | Track::SlotBA9350 | In outline: -1 when a value (+0xA8) is -1.0, else a value |
| `0x4998d0` | Static::Free4998D0 | In outline: with a part, frees it |
| `0x47dd80` | Labels::Value47DD80 | In outline: a value from n when positive |
| `0x41ad20` | Effects::Rect41AD20 | In outline: a step with a rectangle from a point and a size |
| `0x41ac20` | Effects::Rect41AC20 | In outline: the same (another copy) |
| `0x40c5c0` | Effect::Free40C5C0 | In outline: with a part, frees it |
| `0x40c550` | Effect::Free40C550 | In outline: the same (another copy) |
| `0xcd8260` | Camera::PassCD8260 | In outline: goes over entries |
| `0xcc25d0` | Shader::FindCC25D0 | In outline: a find in a list, none handled |
| `0xca59d0` | SaveWriter::RunCA59D0 | In outline: under a count (+0x38, +0x40), runs a save writer (SaveWriter::Run) |
| `0xc67e80` | Window::FlagC67E80 | In outline: by a flag (+0x70) and a shared flag (`0x13E6D00` +0x134), clears a part's flag |
| `0xc65450` | Window::ListC65450 | In outline: a step with a part's list (+0xF8 +0xB0) |
| `0xc1a0e0` | Resource::StepC1A0E0 | In outline: sets a value to 1 (+8) and a step with a part (+0x14, slot 0x24) |
| `0xc05ad0` | Device::FreeC05AD0 | In outline: when on (+4), frees a part (+0xC) and itself when asked |
| `0xbecf30` | Resource::SharedBECF30 | In outline: a shared step (Resource::SharedBECEC0) and goes over entries |
| `0xbdcb50` | VertexDeclaration::PassBDCB50 | In outline: goes over two-byte entries (+0x18) |
| `0xbcd1f0` | Track::CaseBCD1F0 | In outline: a value by case (0, ...) |
