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
