# Windows, dialogs and menus

The game's user interface: each window or dialog is built by one function that creates its controls by name (the names are keys into the game's text files, e.g. `GameOptionsDialog_Shadows`).

| Address | Name | What it does |
|---|---|---|
| `0x60ab70` | GameOptionsDialog::Build | Builds the graphics options dialog: level of detail and draw distance (`GameOptionsDialog_LODTitle`, `..._DrawDistanceTitle`, `..._GeometryImposterDistance`), the screen resolution list (`%d x %d x %d`, with the aspect ratio), the quality presets (`QualityVsPerformance_HighPerformance` ... `_HighQuality`, `_Custom`), anti-aliasing, shadows and the other graphics switches, with their tooltips. Called from `0x610E30` |
| `0x4f89e0` | TrackEditor::Build | Builds the coaster track-building window: curve buttons (`CurveL3`...`CurveR3`), slopes (`SlopeD3`...`SlopeU3`), banking (`BankL3`...`BankR3`, `BankInv`), chain lift, enclosed, water, booster, width, split, spin angle, auto-complete, demolish, height marks and the angle / time / speed parameters (`TrackPieceParam_*`). Called from `0x4E30F0` |
| `0xa4c810` | FrontScreen::Build | Builds the front-end menus: the main menu (play, tools, options, exit), the play menu (profile, sandbox, tutorial, custom scenario, demo), the custom scenario and sandbox menus with their file lists (`\Parks\`, `\Scenarios\`, `\Start New Scenarios\`), the tools menu (scenario editor, people designer), the front-screen logo and the expansion-pack icons. Called from `0x43B0B0` |
| `0xad1850` | LaserDesigner::Build | Builds the laser show designer: beam width, elevation, rotation speed, colour, symbols to project (`LaserWritingWord`, image scroll), scale and duration, centring, number of beams, trail persistence, spread, power, duration, cost and beam type. Called from `0xAD5700` and `0xAD6460` |
| `0x544e00` | RideOpModeDialog::Build | Builds a tracked ride's operating mode dialog: block sections, lift chain speed, launch speed, speed, number of circuits, ride duration, synchronising stations, the load to wait for (full, 3/4, 1/2, 1/4, any), minimum and maximum waiting time, departure interval, leaving if another train arrives, and low friction. Called from `0x632290` |
| `0x6279d0` | ParkView::HandleCommands | Runs every frame in the park view (slot 0 of the table `0xF445DC`, set up at `0x624B9E`): checks about 80 key commands (objects of 0xA4 bytes from +0x4E8 to +0x6400 in the view) and does what each is for: toggles a display setting, or opens something (the water, waterfall and water-flow managers, the terrain flattener, particle effects, fireworks, the ride camera window, debug aids), creating it first when it doesn't exist yet. Does nothing while `0x12F841C` is set. Keeps the camera position at +0x649C |
| `0x4d9ef0` | AttractionDialog::BuildIcons | Builds the icon bar of a ride's window: editor, select / clear saved area, camera, delete (and the destroy buttons for changing rooms and pool complexes), name, save track (with `Save Track to be unlocked by level completion`), music, ride camera, queue line sign, pool path, entrance and exit, with their tooltips. Called from `0x632790` |
| `0x5406f0` | MusicChooser::Build | Builds the music chooser: the music sorted into its genres (`MusicGenre_Save_Other`, `_Alternative`, `_Blues`, `_Childrens`, `_Classical`, `_Country`, `_Dance`, `_EasyListening`, `_Electronic`, `_Folk`, `_House`, `_Industrial`, `_Techno`, `_Trance`, `_HipHop`, `_Holiday`, `_Jazz`, `_NewAge`, ...). Called from `0x5435A0` |
| `0x5fe8a0` | GameOptionsDialog::BuildSound | Builds the sound and regional page of the options: background music, cycling music, the GUI, music and game volumes, temperature units, distance and speed units, currency, language, brightness and importing music. Called from `0x608B40` |
| `0x603660` | GameOptionsDialog::BuildCamera | Builds the camera and photo page of the options: camera controls, fly-camera sensitivity, isometric rotation tolerance, which photos are allowed (all, vistas, coasters), scrolling at the screen edge, and developer settings (`Attract Min:` / `Max:` / `Angle:`, `Sound Shell Min:` / `Max:`). Called from `0x608E50` |
| `0x749dc0` | AvatarEditor::Build | Builds the people designer (`.\Code\Common\AvatarEditor.cpp`): name, random, return, person type, appearance (head, body, legs, skin, upper and lower clothing), optional items, `hold item in right hand` and `spin preview`. Called from `0x7507D0` |
| `0x6f9670` | VIPConditionSetupDialog::Build | Builds the scenario editor's VIP dialog: the VIP's initial happiness and energy, preferred intensity, nausea tolerance, fame, target happiness, the ticks for enjoying the stay and keeping crowds away, autograph and litter tolerance, and what the VIP wants to see (animals, viewing galleries). Called from `0x6FC2A0` |
| `0x6ca020` | ChallengeSetupDialog::Build | Builds the scenario editor's challenge dialog: the challenge's name, description, acceptance and completion texts, its condition, and its prizes (cash, research boost, publicity, ...). Called from `0x6CC2B0` |
| `0x70c330` | ParkDialog::Build | Builds the park window: park name, buying land and building rights (and, in the editor, marking land as unowned, owned, buyable or unlockable), centring on the entrance, entrance fee (`FinancialSummary_EntranceFee_CannotBeChanged_TT`), land and building-rights prices, opening and closing times and the date. Called from `0x65F830` and `0x660190` |
| `0x655500` | TopToolbar::Build | Builds the toolbar along the top of the park view: day / night, objectives, pause, play, fast and super-fast forward, money, guests in the park, the park rating bar (thumbs down / up), temperature and weather, date and time, the avatar's name, fireworks and escaped animals. Called from `0x659770` |
| `0x5f5060` | MainToolbar::Build | Builds the main toolbar and its menus: view, map, land, staff (`Man`), file (career map, quit), delete, rides, shops, scenery, paths. Called from `0x659510` |
| `0x9a0b90` | FireworkDisplayDialog::Build | Builds the firework display editor (`FireworkMaster_*`): choosing and looping the music, its waveform, the keyframe bar and time scroll, play / pause / stop / reload, time scale, and the laser and water-jet designers. Called from `0x9A80A0` |
| `0x8d0bf0` | LandDialog::Build | Builds the land-shaping window: brush radius, mode, raise, lower (also as a crater or canyon), smooth, and moving single vertices, with help texts. Called from `0x616680` |
| `0x503f60` | RideCamera::BuildGUI | Builds the ride camera's controls on a circular panel (`CircularPanelDialog`): pin, quit, next camera type, previous / next camera, previous / next ride and exit (`RideCamPin` ... `RideCamGUI_Exit`). Called from the ten places that start the ride camera (the park view's key commands among them) |
| `0xb05430` | DartCamera::BuildGUI | Builds the dart camera's controls (aiming darts at animals): pin, quit, previous / next camera, exit and the four corners of the cross-hair (`CrossHairTopLeft` ...). Called from the tables `0xF99D90` (slot 205) and `0xF9B2FC` (slot 74) |
| `0xb4d870` | AnimalInfoPanel::Build | Builds an animal's information panel: size, weight, habitat, diet, gestation (`#GESTATION_MONTHS#`), average age at maturity (`#MATURITY_YEARS#`), average litter (`#AV_LITTER#`), rarity and minimum space (`#MINSPACE#`). Called from `0xB4F9E0` |
| `0x8883a0` | SIDSoundEditor::Build | Builds the developer's sound editor for scenery items (`SIDSE_*`): the on, off, volume and pitch tracks of an animation's timeline, inserting a key at the playing point, minimum and maximum distance and sound category; reports `Mismatched number of AnimSounds to animations`. Called from `0x88A5F0` |
| `0x43b0b0` | FrontEnd::Start | Starts the front end: shows the first loading screen (`LoadingScreen_FirstInit`), loads the environment maps (`ShinyEnvMap`, `ChromeEnvMap`, `WaterEnvMap`, `HorizonShadowMap`), sets up the front-end music playlist (`Summer Air.wma`, `Tribal Wild.wma`, `Bush Baby.wma`, ... `Haunted House.wma`) and builds the front-end menus (`FrontScreen::Build`). Called from `0x4032D0` (slot 12 of `0xF264E4`) and `0x43EB90` |
| `0x795380` | StaffTasksDialog::BuildEvaluate | Builds the page of a staff member's tasks that chooses what to evaluate: flat rides, roller coasters, food and drink stalls, souvenir stalls, vistas, staff, pools and viewing galleries (`StaffTasks_Evaluate_*`). Called from `0x7970A0` |
| `0xaa7100` | PoolPathBuilder::Build | Builds a path-building window in the pool code (called from `0xA84ED0`, slot 138 of `0xF90D9C`): curve left / straight / right (`PathCurveL1`, `PathCurve0`, `PathCurveR1`), slopes (`PathSlopeD2` ... `PathSlopeU2`), delete, camera follow and expanding the window |
| `0x710060` | PathEditor::Build | Builds the footpath building window: curves (`PathCurveL1` / `0` / `R1`), slopes (`PathSlopeD2` ... `U2`), delete, and cutting into the terrain (`PathEditor_CutIntoTerrain_*`). Called from `0x711DA0` (slot 217 of `0xF698D4`) |
| `0x872970` | SceneryMenu::Build | Builds the scenery menu: trees and plants, fences, walls (roofs and buildings), path extras, theme props, ride events, firework sources, park entrance, terrain surfaces, adverts and billboards, and saved selections (`Obj_Scenery_*`). Called from `0x8749E0` |
| `0x556220` | RidesMenu::Build | Builds the rides menu: roller coasters, junior coasters, thrill, water, gentle and transport rides, pools, other, importing, and the pool and enclosure designers and wild animals (`Obj_Rides_*`). Called from `MainToolbar::Build` |
| `0x61a0c0` | ManagementMenu::Build | Builds the management menu: park, finances, staff, objectives (and setting them up), the ride and guest lists, research (and the invention editor), and challenges (`Man_*`). Called from `MainToolbar::Build` |
| `0x609310` | GameOptionsDialog::BuildGraphicsDetails | Fills in the detailed graphics settings of the options dialog: imposter distance, blending between scenery levels of detail, texture compression, aggressive z-bias, anti-aliasing levels and shadow types (`GameOptionsDialog_*`). Called from `0x610D30` |
| `0xa21b30` | TutorialSetupArea::Build | Builds the developer's tutorial setup window (`.\Code\Common\TutorialSetupArea.cpp`): the list of window positions, forcing a window on top, restricting track placement, adding a special trigger, window width and forcing the simple camera. Called from `0xA1A7D0` |
| `0x57e180` | AttractionVehicleDialog::Build | Builds a ride's vehicle page: changing the car type, the numbers of trains and cars and the capacity, and colours for the attraction, track, cars, tunnel lights and lights (`AttractionVehicleDialog*`). Called from `0x632870` |
| `0xaed820` | WaterJetDesigner::Build | Builds the water jet designer (it shares the laser designer's labels): elevation, radius, pressure, colour, scale, duration, cost and name. Called from `0xAEF020` and `0xAEF810` |
| `0x66e3e0` | CoasterStatsDialog::Build | Builds the window that shows a finished track's statistics before building it: space required, cost, excitement, intensity, nausea, maximum and average speed, ride time and length, maximum vertical and lateral G, air time, drops, highest drop and inversions, with previous / next car, build and cancel. Called from `0x66FA40` |
| `0xa957a0` | PoolEditor::Build | Builds the pool designer's controls: height, water, solid, delete, depth (shallow, normal, deep), straight or diagonal pieces, small or large brush, and the grid (`Pool_HeightToggle`, `Pool_GridToggle`). Called from `0xA84E20` (slot 119 of `0xF90D9C`) |
| `0x9f0600` | CameraSequenceEditor::Build | Builds the camera recorder window: play and pause, the main or viewfinder camera view, smoothing towards new targets and record. Called from `0x9ED8C0` and `0x9EE050` |
| `0x4057a0` | StartupScreen::Draw | Shows the start-up screens: the Atari logo and the legal notices (`Startup_Legal_1` ... `_7`), fading them in and out, with a Korean special case and the Windows version (`GetVersionExA`). Slot 5 of `0xF264E4` |
| `0x6249d0` | RCTDesktop::CreateWindows | Creates the park view's windows (`.\Code\Common\Gui\RCTDesktop.cpp`): 151 calls of the window maker `0x6B3190`, keeping each window in a global (`0x12F5848` ... `0x12F7B20`). Called from `0x6278E0` |
| `0x9a8460` | FireworkMaster::Build | Builds the firework show window: firework bases, displays, the mix master, loading from `\Fireworks`, the timeline, rotating and the display placer. Called from four places |
| `0xb504e0` | WildAnimalWindow::Build | Builds an animal's window: its view, statistics, thoughts, adoptions, information and the dart camera (`WildAnimalGUI_*Window`). Called from `0xB34BF0` and `0xB51750` |
| `0x62f160` | ViewMenu::Build | Builds the view menu: grid, contours, people, supports, path and ride heights, scenery visibility, transparent terrain and animal walking areas (`Land_*`). Called from `MainToolbar::Build` |
| `0x621bd0` | ProgramInfoDialog::Fill | Fills in the About window (`.\Code\Common\ProgramInfoDialog.cpp`): the program's version information, with the expansion pack (`_AOP`) texts, laid out in a 530 x 250 window. Called from the ProgramInfoDialog window builder |
| `0xa857e0` | PoolDesigner::Build | Builds the pool designer's main window: editor, paths, extras, type, changing rooms, loading a saved pool (`LoadPool`), viewport and colour. Called from four places, `SavedSelection::Save` among them |
| `0x41e400` | AdMenu::ShowMedia | Shows the picture or movie chosen for an advert: its shape as a ratio (`8:1` ... `1:8`) and size, and the warnings `AdMenu_ImageTooLarge` / `AdMenu_MovieTooLarge`, and which scenery it suits (`AdMenu_SuitableScenery`). Called from the AdMenu window and two others |
| `0x5f85f0` | FileMenu::Build | Builds the file menu: about RCT3, screenshot, pause, sound and video options, game options, load game and save game (`File_*`). Called from `MainToolbar::Build` |
| `0x8cf4c0` | LandDialog::BuildVertexMenu | Builds the land window's vertex sub-menu: free-form, neighbour and quantised vertex moving, each in two sizes (`Land_Vertex_*`). Called from `LandDialog::Build` |
| `0x8ce810` | LandDialog::BuildSmoothMenu | Builds the land window's smoothing sub-menu: round, square, flat, flat with move, average (`Land_Brush_Height_Smooth_*`). Called from `LandDialog::Build` |
| `0x6167b0` | LandMenu::Build | Builds the land menu: height, paint, water and waterfall editors (`Land_Brush_Height`, `Land_Brush_Paint`, `Land_Water`, `Land_Waterfall`). Called from `MainToolbar::Build` |
| `0x584e40` | ShopsMenu::Build | Builds the shops menu: food shops, drink shops, stalls and utilities (`Obj_Shops_*`). Called from `MainToolbar::Build` |
| `0x55c650` | RideTestResults::Fill | Fills in a ride's test results: each rating as `%.2f`, or `AttractionResultsDialogLabel_NotAssessed` before the ride is tested (15 labels). Called from the RideTest window |
| `0x674b70` | ObjectFilter::Build | Builds the filter buttons of an object menu: an icon per filter in colour and greyed out (`ObjectFilter_Colour_`, `ObjectFilter_Grey_`, tooltips `ObjectFilter_TT_`) and the style filters (`StyleFilter_`). Called from `AdMenu::ShowMedia` and `0x6708B0` |
| `0x8d0370` | LandDialog::BuildHeightModeMenu | Builds the land window's height-brush modes: spray (with its speed) and drag (`Land_Brush_Height_Mode_*`). Called from `LandDialog::Build` |
| `0xa09a30` | SavedSelectionMenu::Build | Builds the saved selection sub-menu of the scenery menu: grab, colour, list and the paint tool (`SavedSelection_Grab`, `SavedSelection_List`, `GUI2_PaintTool`). Called from `SceneryMenu::Build` |
| `0x6743c0` | ObjectFilter::Names | Makes the names of the object filter categories (25 wide strings): `Coaster_Steel`, `Coaster_Wooden`, `Coaster_Suspended`, `Coaster_Water`, `Transport_Standard`, `Transport_Safari`, `Scenery_Tree`, `Scenery_Plant`, `Scenery_Shrub`, `Scenery_Flower`, `Scenery_SmallProp`, `Scenery_MediumProp`, ... Called from `ObjectFilter::Build` |
| `0x623090` | ProgramInfoDialog::BuildTitle | Builds the About window's title: `ProgramInfo_Title`, the version as `( %d.%d.%d.%d )` and `ProgramInfo_Copyright`. Called from the ProgramInfoDialog window builder |
| `0x673b90` | TrackImporter::Layout | Lays out the track importer window: the ride list with expand all / contract all (`RideSelectionWindow_ExpandAll`, `_ContractAll`), 8 rectangles updated. Called from the TrackImporter window |
| `0x5f7110` | DeleteMenu::Build | Builds the delete menu: delete a selection or a single item (`GUI2_DeleteSelection`, `GUI2_DeleteSingleItem`). Called from `MainToolbar::Build` |
| `0x66d5c0` | CoasterStatsDialog::Fill | Fills in the values of the finished track's statistics window: sizes as `%d x %d`, ratings and figures as `%.2f` (`???` when unknown), money through `0x6BE770`. Called from `CoasterStatsDialog::Build` and `0x66DB80` |
| `0xa5a0e0` | TutorialPanel::Build | Builds the front screen's tutorial panel: the tutorial's description and the start button (`TutorialDescription`, `StartTutorialButton`). Called from `FrontScreen::Build` |
| `0xc6c590` | DebugConsole::BuildWindow | Builds one of the debug console's windows: a client area, a scrollbar and a filler (`window filler`, `client`, `scrollbar`). Called from `DebugConsole::Build` |
| `0x681360` | Desktop::Create | Creates the GUI desktop, the root of the window tree (`.\Code\Common\Gui\Desktop.cpp`). Called from `RCTDesktop::CreateWindows` |
| `0x8ce120` | LandDialog::BuildPaintButtons | Makes the land window's terrain paint buttons, one per terrain texture (`LandButton%d`). Called from `LandDialog::Build` |
| `0x7489d0` | AvatarEditor::BuildPreview | Makes the people designer's preview of the person, with its shadow (`PersonPreviewShadowBlob`). Called from `AvatarEditor::Build` |
| `0x57bc40` | RideParticleList::Fill | Lists a ride's particle effects for the developer: each as `<Particle Name>` : `<Bone Name>` : `<Car Name>`. Called from a window builder |
| `0x6377d0` | AttractionDetails::BuildRideTabs | Builds the tabs of a ride's window for its vehicles, test results, operating mode and maintenance (`AttractionDetails_RideVehiclesHelp`, `_RideTestHelp`, `_RideOpModeHelp`). Called from three places |
| `0x634fa0` | AttractionDetails::BuildTabs | Builds the main tabs of an attraction's window: view, finances, thoughts, customers and the state chooser (`AttractionDetails_*Help`). Called from five places |
| `0x6575b0` | TopToolbar::Update | Keeps the top toolbar up to date: shows which speed is selected (`Top_Pause`, `Top_Play`, `Top_FastForward`, `Top_SuperFastForward` and their `Selected` pictures), the park rating (`%.0f`, and the developer's ` Debug ideal guests: `) and the guests in the park, with their tooltips |
| `0x652c50` | GuestDetails::BuildGroupTabs | Builds the tabs of a guest group's window: rides, statistics, cash, group, photo and the viewport (`GuestDetails_Group*Help`). Called from `0x473320` and `0x6542D0` |
| `0x6512a0` | GuestDetails::BuildTabs | Builds the tabs of a guest's window: view, thoughts, rides visited, what they carry, options and the viewport (`GuestDetails_*Help`). Called from `0x651F40` and `0x652940` |
| `0xa80c80` | PoolMaintenanceDialog::Build | Builds the developer settings of a pool's cleaning: dirt each guest makes every 16 seconds (`Peep dirt unit`), how much a full tank of fluid cleans (`Fluid power factor`), the dirt reporting factor, the most fluid used, dirt per segment and the fluid left, with explanations. Called from `0x632930` and `0x6332A0` |
| `0x74ec80` | AvatarEditor::BuildDialogs | Builds the people designer's dialogs: adding, saving and deleting an avatar (`AvatarEditor_AddAvatar*`, `_SaveAvatar*`, `_DeleteAvatar*`, with `%NAME%`). Called from `0x750EA0` |
| `0x712190` | PathMenu::Build | Builds the paths menu: the path and queue lists and buttons, the path editor pop-up, and camera follow (`PathEditorAreaPopupButton`, `PathList`, `QueueList`, `PathEditorCameraFollowButton`). Called from `0x5F4F30` |
| `0x62d2a0` | ScenarioEditorMenu::Build | Builds the scenario editor's main menu: invention setup, landscape editor, financial, guest, park and animal options, objectives, challenges and saving (`SE_B_*`, tooltips `SE_TT_*`) |
| `0x68a640` | FileDialog::Confirm | The file dialog's confirmations: renaming (`FD_RenameFile`, `%OLDNAME%` / `%NEWNAME%`) and deleting a file (`FD_ConfirmDelete`, `FD_DeleteFile`, `%FILENAME%`), checking the file exists |
| `0xc6cee0` | MessageBox::Build | Builds the engine's message box: `Message`, `Warning` or `Error`, with a `Continue` button |
| `0xc6ddc0` | QuestionBox::Build | Builds the engine's yes / no question box (`Question`, `Why yes`, `Heck no`; `find me` names a part of it). Called from `0xC6EA40` |
| `0x7473b0` | AvatarEditor::BuildList | Builds the people designer's list of avatars and group members: value, create, save and delete buttons with their icons and tooltips (`AvatarEditor_TT_*`). Called from `0x7507D0` |
| `0x62ba60` | SaveGameDialog::Save | Saves the game from the save dialog: needs a name (`SGD_NeedAName`), refuses names starting with a dot or a space, asks before overwriting (`SGD_ConfirmSave`, `%FILENAME%`) and reports `SGD_SaveSuccessful` (`.dat`) |
| `0xa07350` | SaveBuildingDialog::Save | Saves a building made of scenery (`MyBuilding%d.dat` as the default name), with the same name checks and overwrite question (`SGD_*`) |
| `0x6367b0` | AttractionDetails::BuildPoolTabs | Builds the tabs of a pool's window: editing the pool, details, maintenance, operating mode, stall items and construction. Called from `0xA7B310` |
| `0x8cc740` | LandDialog::BuildPaint | Builds the land window's paint page: brush radius, base and cliff textures (`Land_Brush_Paint_Base`, `_Cliff`). Called from `0x616520` |
| `0x41d9c0` | AdMenu::Layout | Lays out the advert window, with the queue and unqueue image buttons (`AdMenu_QueueImage`, `AdMenu_UnQueueImage`) |
| `0x651f40` | GuestDetails::BuildStatsTabs | Builds the statistics, group, cash and passport tabs of a guest's window (`GuestDetails_*Help`) |
| `0x64d250` | VIPDetails::Build | Builds a VIP's details: temperament and opinion of the park (`VIPDetails_Temprement_TT`, `VIPDetails_ParkOpinion_TT`). Called from `0x6510E0` |
| `0xba78d0` | EnclosureDetails::BuildTabs | Builds the tabs of an animal enclosure's window: editor, shelter, gallery, item and viewport. Called from three places |
| `0x6ee3b0` | SandboxScenario::Layout | Lays out a sandbox scenario's entry: its name and description (`SandboxScenario_Name`, `_Description`, `#SANDBOX#`) |
| `0x4e1ca0` | TerrainEditor::Build | Builds an editor window with view, rotate and colour (and the terrain flattener under the edited object). Called from three places |
| `0x606960` | GameOptionsDialog::BuildPhotos | Fills in the photo and scrolling options (`GameOptionsDialog_AllowAllPhotos`, `_AllowVistaPhotos`, `_AllowCoasterPhotos`, `_AutoScrollAtEdge`, with their tooltips). Called from `0x610D30` |
| `0x664da0` | SceneryDetails::BuildTabs | Builds a scenery item's window: colour, destroy, the firework mix master and adverts (`Scenery_SetFlexiColours`, `FireworkMaster_MixMaster_TT`, `AdMenu_SelectCustomImage_TT`). Called from `0x473320` |
| `0x635e90` | StallDetails::BuildTabs | Builds the tabs of a stall's window: items, extras and adverts. Called from `0x58FBE0` |
| `0x46e810` | Passport::Build | Builds the passport window: the stamps still needed and choosing a gift (`Passport_StampsNeededLabelText`, `Passport_ChooseGiftLabelText`). Called from `0x46F120` |
| `0x41fca0` | AdMenu::BuildFolderList | Builds the custom picture browser's folder list: up, back and folders (`AdMenu_DirUp`, `CustObj_FolderBack`, `CustObj_Folder`) |
| `0xbac0f0` | SceneryFilter::Build | Builds the scenery filter buttons for large, medium and small props (`ObjectFilter_Colour_Scenery_`, `ObjectFilter_TT_Scenery_`). Called from `0xBA7570` |
| `0x4ec9a0` | AttractionNameMusic::Build | Builds the name and music fields of an attraction's window. Called from `0x4E15A0` |
| `0x662750` | SceneryEditor::Build | Builds the scenery editor's window: rotate, exposing scenery, viewport and colour. Called from `0x41B0B0` |
| `0x4f1e60` | TrackEditor::BuildSpecialPieces | Builds the buttons of the special track pieces: station, photo section, vertical, brakes and block brakes (`SpecialOptButton`). Called from `0x4F8100` |
| `0xaec200` | WaterJetDesigner::Save | Saves a water-jet design (`.wjt`) with the save dialog's name checks and overwrite question (`SGD_*`, `%FILENAME%`) |
| `0xad0980` | LaserDesigner::Save | Saves a laser design (`.las`) with the save dialog's name checks and overwrite question (`SGD_*`) |
| `0x9a7100` | FireworkDisplay::Save | Saves a firework display (`.fwd`) with the save dialog's name checks and overwrite question (`SGD_*`) |
| `0xacb7a0` | LaserDesigner::BuildColourTool | Builds the laser designer's colour tool, with its OK button (`LaserDesigner_Tool_Colour_TT`, `LaserWritingDesigner_Ok`) |
| `0xacf690` | LaserDesigner::BuildRotationTool | Builds the laser designer's rotation-speed tool, with its OK button (`LaserDesigner_Tool_RotVel_TT`) |
| `0x9b94b0` | ParticleDescWindow::Build | Builds the firework particle editor's window, with editing the particle's texture (`Fireworks_Small_Star`, `TT_GUI2_ParticleDescWindow_EditTexture`). Called from `0x9BB220` |
| `0x653aa0` | GuestGroupWindow::Build | Builds a guest group's member list (`GUI2_Guest_Group_Avatar`, on a curved background). Called from `0x654340` |
| `0x64b660` | Passport::Update | Fills in a guest's passport: the stamps (`PassportStamp`) and the messages to place posts to get stamps, that the gift is already got, or how many more stamps are needed for which gift (`%NUMBER%`, `%GIFT%`). Called from `0x650A60` |
| `0x9ad530` | ExpansionMessage::Build | Builds the message that an expansion pack is needed (`AOP_Name_0`, `AOP_RequiredMessage`) |
| `0xa9cca0` | PoolEditor::BuildSegmentTypes | Builds the pool editor's special segment buttons: high dive and jacuzzi (`SegmentTypeHighDive`, `SegmentTypeJacuzzi`). Called from `0xA9D4A0` |
| `0x554a30` | RideSelectionWindow::Fill | Fills in the ride selection window: building a saved track (`BuildSaved`, `RideSelectionWindow_BuildSavedTrack`), the number of designs available (`%NUMBER%`) and sizes as `%d x %d`. Called from `0x677110` |
| `0x7507d0` | AvatarEditor::BuildTypes | Builds the people designer's type buttons: child, teenage and adult, male and female (`PersonEditor_*Icon`). Called from `0x750F90` |
| `0x9ee050` | CameraSequenceEditor::BuildFile | Builds the load and save buttons of the camera recorder. Called from `0x9F3FD0` |
| `0xa4bc80` | FrontScreen::BuildExpansionBadges | Builds the front screen's badges of the installed expansion packs (`FrontScreen_AOPBadge_`, `AOP_InstalledMessage`, `AOP_Name_`). Called from `0xA4C320` |
| `0x78e670` | StaffWindow::BuildTabs (1) | Builds the tabs of a staff member's window: statistics, tasks and costume / uniform colours (`Staff_Stats`, `Staff_Tasks`, `Staff_Uniform_Colours`). Called from `0x73BA10`; `0x78DFC0` and `0x78D910` are the same for other kinds of staff |
| `0x78dfc0` | StaffWindow::BuildTabs (2) | The same as `0x78E670` for another kind of staff. Called from `0x7971C0` |
| `0x78d910` | StaffWindow::BuildTabs (3) | The same as `0x78E670` for another kind of staff (`Staff_Uniform_Colour`). Called from `0x79DE70` |
| `0x78d290` | StaffWindow::BuildCommon | Builds the parts every staff member's window has: thoughts, name, details and the viewport (`Staff_Thoughts`, `Staff_Details`). Called from the three above and four others |
| `0xa021f0` | SavedSelectionMenu::BuildItem | Makes an item of the saved selection menu with its icon (`GUIIcon`, `SavedSelection_SubMenuHelp`). Called from `0xA04420` |
| `0x6b42a0` | MapDialog::Build | Builds the park map window with its people, rides and scenery tabs (`MapDialog_PeopleTabTitle`, `_RidesTabTitle`, `_SceneryTabTitle`). Called from `0x6B4BE0` |
| `0x8cbeb0` | LandDialog::BuildTextureList | Builds the land window's list of terrain textures (`LandType`, with empty slots `Land_Empty`). Called from `LandDialog::BuildPaint` |
| `0x66bc30` | TrackBrowser::Locked | Tells the player a saved track is locked until a scenario level is completed (`TrackBrowser_LockedHelp`, `%SCENARIO%`, `%LEVEL%`) |
| `0xae9830` | WaterJetDesigner::BuildSizeTool | Builds the water-jet designer's size tool (`WaterJetDesigner_Tool_Size_TT`), with its OK button |
| `0xaeb4f0` | WaterJetDesigner::BuildPressureTool | Builds the water-jet designer's pressure tool, in `m/s` (`WaterJetDesigner_Tool_Pressure_TT`) |
| `0xace7e0` | LaserDesigner::BuildSpreadTool | Builds the laser designer's beam-spread tool (`LaserDesigner_Tool_Spread_TT`) |
| `0xa4a750` | FrontScreen::BuildTitle | Writes the game's short name and the expansion packs' names on the front screen (`RCTName_short`, `AOP_Name_`) |
| `0x819f90` | BinWindow::Build | Builds a litter bin's window: how full it is and when it was last emptied (`Bin_AmountFull`, `Bin_LastEmptied`). Called from `0x81A570` |
| `0x870c60` | StyleMenu::Build | Builds the scenery style choices: each style's icon and name (`StyleInternalName`, `StyleIcon`, `StyleName_`) |
| `0x638790` | AttractionDetails::BuildCoasterTabs | Builds the graphs and track tabs of a coaster's window, with the ride tabs (`AttractionDetails_GraphsHelp`, `_ConstructionHelp`). Called from three places |
| `0x637250` | AttractionDetails::BuildGalleryTabs | Builds the maintenance and editing tabs of a viewing gallery's window. Called from `0xBACB70` |
| `0x9eda70` | FlyingCameraRoute::FileMenu | Loads and saves a flying camera route (`.fcr`; `Load Flying Camera Route`, `Save Flying Camera Route`) |
| `0xa255b0` | TutorialEditor::FileMenu | Loads and saves a tutorial (`\Tutorials`, `.tut`; `Load Tutorial`, `Save Tutorial`) |
| `0x9bd360` | ParticleDesigner::BuildTree | Draws the particle designer's tree of effects with its connecting lines (`GUI2_ParticleDesigner_Tree*`), calling itself for each branch |
| `0xba8320` | EnclosureEditor::Build | Builds the enclosure editor's rotate button. Called from `0xBA74C0` |
| `0xa44570` | FrontScreen::SaveBeforeLoad | Asks whether to save before loading another game (`FSM_SaveBeforeLoad_Title`, `_Message`). Called from `0xA44B00` and `0xA450D0` |
| `0x78f490` | StaffWindow::BuildEntertainer | Builds an entertainer's statistics and costume tabs (`Staff_Costume_Colours`). Called from `0x754DE0` |
| `0x78f9f0` | StaffWindow::BuildTabs (4) | Builds the statistics and tasks tabs of another kind of staff member. Called from `0x7A68F0` |
| `0x63d4e0` | ColourSwatch::Build | Builds a colour swatch button with its surround (`ColourSwatchButton`, `FlexiColourSurround`). Called from `0x63DB00` |
| `0x5fe390` | GameOptionsDialog::BuildUnits | Fills in the units and screen options: Celsius / Fahrenheit, metric / imperial / SI and full screen. Called from `0x610D30` |
| `0x6b8d60` | FinanceWindow::BuildTabs | Builds the finance window's monthly summary and graphs tabs. Called from `0x6B9270` |
| `0x80bdb0` | ObjectMenu::BuildAvailability | Builds the all / only available choice of an object menu (`ASM_All`, `ASM_OnlyAvailable`) |
| `0x792c50` | StaffWindow::AnimalKeeperStats | Writes an animal keeper's work: dung cleared and troughs filled. Called from `0x796EE0` |
| `0x792130` | StaffWindow::MechanicStats | Writes a mechanic's work: rides inspected and fixed. Called from `0x796D20` |
| `0x6755a0` | RideSelectionWindow::BuildItem | Makes an item of the ride selection list, with showing its children and rotation (`ShowChildren`, `Rota`). Called from three places |
| `0x642160` | DeleteSelection::Build | Builds the delete-selection tool: clear and delete (`GUI2_DeleteSelection_*`) |
| `0x997ef0` | FireworkDisplaysWindow::Build | Builds the list of firework displays, with adding one (`TT_FireworkDisplaysWindow_AddDisplay`). Called from `0x9A5AA0` |
| `0x99da30` | FireworkDisplayDialog::BuildGlyphs | Makes the small pictures of the effects in a display (`.\Code\Common\Particles\GUI2_FireworkDisplayDialog.cpp`: water jet, laser and firework glyphs). Called from `0x99FF30` |
| `0xc6a720` | DebugConsole::BuildTitleBar | Builds the title bar of a debug console window, with close and maximise. Called from `DebugConsole::BuildWindow` |
| `0x670c00` | RideSelectionWindow::LoadResources | Loads the ride selection window's resources, with a Chinese special case |
| `0xbb1960` | RideMaintenanceDialog::BuildMechanic | Builds the button that assigns a mechanic in a ride's maintenance page (`TT_RideMaintenanceDialog_MechanicButton`) |
| `0x78ed20` | StaffWindow::BuildTabs (5) | Builds the statistics and uniform tabs of another kind of staff member (`Staff_Uniform_Colours`). Called from `0x7E8F50` |
| `0x649e90` | GuestWindow::AvatarHighlight | Switches the avatar-style highlight of a person on or off (`Person_AvatarStyleHighlight_On` / `_Off`), refusing in land the park doesn't own (`Pincer_NotOwned`) |
| `0x787810` | ParkGuestsSummary::Build | Builds the park's guest summary with its actions and thoughts pages (`ParkGuestsSummary`). Called from `0x619D50` |
| `0x617a00` | ProgressBar::Build | Builds a progress bar from its left, middle and right pieces (`Blender_Bar_Left`, `_Mid`, `_Right`). Called from `Language::LoadProgress` |
| `0x41d080` | AdMenu::FileTypes | Makes the list of picture and movie file types adverts can use: `JPEG`, `BMP`, `DDS`, `DIB`, `HDR`, `JPG`, `PFM`, `PNG`, `PPM`, `TGA`, `WMV`, `MPG`, ... Called from `0x4214F0` |
| `0xa8c7b0` | PoolEditor::BuildRotate | Builds the pool editor's rotate button. Called from `0xA85540` |
| `0xa26970` | TutorialWindow::BuildButtons | Builds the tutorial window's continue and quit buttons (`TutorialWindow_Continue`, `_Quit`). Called from `0xA26DA0` |
| `0x4351b0` | CircleArrow::Draw | Draws the circular rotate arrow, active or not (`CircleArrowActive`, `CircleArrowDeActive`). Called from `0x435850` |
| `0xa09420` | SavedSelectionDialog::Build | Builds the window for placing a saved selection: its title icon, rotate and colours (`SavedSelectionDialog_TitleIcon`, `PlaceSavedSelection_FlexiColour_Tooltip`). Called from `0xA07E00` |
| `0xa44140` | CareerMap::BuildScenarioIcon | Makes a scenario's icon on the career map, with its padlock locked or unlocked (`ScenarioIcon_`, `GUI2PadlockLocked`, `GUI2PadlockUnlocked`). Called from `ParkView::HandleCommands` |
| `0x9dd050` | CampaignDialog::Build | Builds the marketing campaign window: park vouchers, attraction vouchers, advertising, and choosing the attraction (`Campaign_Dialog_*`, `Campaign_Select_Attraction`) |
| `0x57ca10` | AttractionVehicleDialog::UpdateCounts | Updates a ride's numbers of trains and cars, saying when the station is too short for more cars or the ride allows no more (`RideStationTooShortForMoreCars`, `RideDoesNotAllowMoreCars`). Called from `AttractionVehicleDialog::Build` |
| `0x66d1c0` | CoasterStatsDialog::BuildGForce | Builds the G-force part of the track statistics window (`ExpeditionGeForceText`, `ExpeditionGeForceIcon`). Called from `0x66FA40` |
| `0x4f3150` | TrackDesigner::BuildCarSettings | Builds the track designer's car spin and holding time settings (`TrackDesigner_CarSpin_TT`, `TrackDesigner_HoldingTime_TT`). Called from `0x4F3CF0` |
| `0x99efe0` | ParticleDesigner::BuildTimeStrip | Builds the particle designer's time strip (`GUI2_ParticleDesigner_TimeStrip`, `%.2d:%.2d`, slider arrows) |
| `0x622bf0` | ProgramInfoDialog::BuildTabs | Builds the About window's tabs: program information and credits (`File_AboutRCT`, `File_Credits_Stars`). Called from the ProgramInfoDialog window builder |
| `0x4d9b10` | AttractionDialog::PlacementErrors | Tells the player why an entrance, exit or fence can't be placed: the ride has no station or isn't placed (`CantPlaceEntranceAsRideHasNoStation`, `CantPlaceExitAsRideHasNoStation`, `CantChooseFenceAsRideIsNotPlaced`). Called from `AttractionDialog::BuildIcons` |
| `0x6884c0` | FileDialog::BuildButtons | Builds the file dialog's buttons: save, open, cancel and delete (`FD_Label_*`) |
| `0x659770` | MainWindow::Build | Builds the main corner windows of the park view (`MainBottomRight`, `MainBottomLeft`) and the top toolbar (`TopToolbar::Build`). Called from 80 places |
| `0xa94a30` | PoolEditor::BuildGrid | Makes the pool placement grid (`PoolPlacementGrid`, `PoolPlacementGrid2`). Called from `PoolEditor::Build` |
| `0x844220` | PlaceObject::ErrorText | Says why an object can't be placed: land not owned, it intersects something or the ground, too high, must be on water, can't be under water, must end on a pool edge (`PlaceObject_*`). Called from `0x4EC920` and `0xA956F0` |
| `0x7f2a50` | StaffWindow::BuildHire | Builds the staff window's hire buttons: handyman, mechanic, security guard, entertainer, animal keeper (`Staff_Hire_*`). Called from the Staff window |
| `0x62b290` | SaveGameDialog::DefaultName | Picks a free default name for a saved game: `MyGame.dat`, `MyGame%d.dat`. Called from `SaveGameDialog::Save` |
| `0xa08010` | SavedSelectionMenu::FileMenu | Loads and saves a saved building (`*.dat`, default `MyBuilding.dat`). Called from the SavedSelectionGrabber window and `SavedSelectionMenu::Build` |
| `0x4237f0` | AdMenu::BuildCustomImage | Makes the custom image button of the advert window (`CustObj_Icon`, `AdMenu_CustomImage`). Called from `0x674180` |
| `0xcab640` | StructViewer::Build | Builds the debug structure viewer's window (`StructCompositor`, `Struct border1`). Called from `0xCACA70` and `0xCACE00` |
| `0x8cd720` | LandDialog::BuildRadius | Builds the land window's brush radius slider (`GUI2_RadiusSliderBackground`, `CircularViewSolid`). Called from three places |
| `0x7485b0` | AvatarEditor::BuildLeader | Builds the people designer's group leader buttons (`AvatarEditor_TT_CurrentIsLeader`, `_MakeCurrentLeader`). Called from two places |
| `0x57d170` | AttractionVehicleDialog::Capacity | Writes a ride's passengers per car and per ride (`AttractionVehicleDialogLabelPassengersPerCar`, `...PerRide`, `%NUMBER%`). Called from `AttractionVehicleDialog::Build` |
| `0x701590` | VIPRouteDialog::BuildDrag | Builds the drag button of the VIP route editor (`ArrowsUpDown`, `VIPRouteDialog_TT_DragButton`) |
| `0x667c30` | ColourSwatch::Highlight | Draws a colour swatch's surround and highlight (`FlexiColourSurround`, `FlexiColourHighlightBlue`, `...White`). Called from `0x668300` |
| `0x41f2c0` | AdMenu::Caching | Shows that downloaded content is being cached (`MassiveWaiting_1`, `Caching_LiveContent_TT`). Called from a window builder |
| `0x652940` | VIPDetails::BuildTabs | Builds the statistics tab of a VIP's window (`GuestDetails_VIPStatsHelp`). Called from `0x7F82C0` |
| `0xacb270` | LaserDesigner::BuildColourWheel | Builds the laser designer's colour wheel (`ColourSelection_CircF`, `_CircO`). Called from `LaserDesigner::BuildColourTool` |
| `0x66b360` | TrackBrowser::ItemName | Writes a track's name in the track browser: custom tracks and locked ones (`TrackBrowser_CustomTrack`, `TrackBrowser_LockedSuffix`). Called from `0x66CCE0` |
| `0xaf04a0` | WaterJetDesigner::BuildIdentBar | Builds the water jet's bar in the display editor (`WaterJetIdentbar`). Called from two places |
| `0xad7460` | LaserDesigner::BuildIdentBar | Builds the laser's bar in the display editor (`LaserIdentbar`). Called from two places |
| `0x686260` | FileDialog::DeleteCampaign | The button that deletes a career profile (`CampaignDelete`, `FD_TT_CampaignDelete`) |
| `0xa20c50` | TutorialSetup::TriggerNames | Makes the names of the tutorial triggers: open, close, click (buttons only), scenery item placed, window already open (`On Open`, `On Close`, ...). Called from `0xA21490` |
| `0x6c4f40` | ChallengeSetupDialog::PublicityList | Makes the list of publicity boosts for a challenge prize: none, small, medium, large, huge (`Challenge_PublicityBoostList*`). Called from `ChallengeSetupDialog::Build` |
| `0x6c4b30` | ChallengeSetupDialog::StartConditions | Makes the list of when a challenge starts: a time after the game starts, or after achieving apprentice or entrepreneur (`ChallengeSetupDialog_StartCondition*`). Called from `ChallengeSetupDialog::Build` |
| `0x78f1c0` | StaffWindow::BuildStats | Builds a staff member's statistics tab (`Staff_Stats`) |
| `0x640b20` | ObjectIcon::ExpansionGlyph | Puts an expansion pack's glyph on an object's icon (`AOP_GlyphTexture_`, `AOP_Name_`). Called from three places |
| `0x607e70` | GameOptionsDialog::KeyBindingHelp | Shows the key binding help (`GameOptionsDialog_KeyBindingHelp`, `_NoKeyBindingHelp`). Called from `0x6084F0` |
| `0xac8060` | LaserDesigner::Selection | Draws a laser designer selection box (`Blender_SelectionF`, `_SelectionO`). Called from two places |
| `0xac86e0` | LaserDesigner::BuildBar | Builds a bar of the laser designer from its pieces (`Blender_Bar_Left`, `_Mid`, `_Right`). Called from `0xAC8A30` |
| `0x5f6d70` | FileMenu::ConfirmExit | Asks whether to quit, or go back to the career map (`FSM_ConfirmExit_Message`, `FSM_ConfirmExitToCareerMap_Message`) |
| `0x5f9220` | FileMenu::SaveBeforeLoad | Asks whether to save before loading (`FSM_SaveBeforeLoad_Title`, `_Message`) |
| `0x9db8f0` | CampaignDialog::BuildSelect | Builds the choosing of an attraction for a marketing campaign, with cancel (`Campaign_Select_Attraction`, `SMB_Label_Cancel`) |
| `0x6ae890` | GUIHighlight::LoadMasks | Loads the shapes used to darken around a highlighted window (`GUIHighlightMask_Circle_Darken`, `_Dialog_Darken`, `_LozengeWithShadow_Darken`). Called from `0x6AFE60` |
| `0x660e20` | PersonIcon::Get | Picks a person's icon: child, teenage or adult, male or female, or VIP (`PersonEditor_*Icon`, `VIP_Icon`). Called from four places |
| `0x70f870` | PathEditor::CutIntoTerrainButton | Switches the path editor's cutting into the terrain on or off (`PathEditor_CutIntoTerrain_on` / `_off`) |
| `0x4f1a70` | TrackEditor::RideHeightsButton | Switches showing ride heights in the track editor (`AttractionEditor_IconRideHeights`, `...On`). Called from two places |
| `0xb6fe70` | AnimalCamera::ModeNames | Makes the animal camera's mode names: static, head and follow (`AnimalCam_*`). Called from `0xB72BE0` |
| `0xc6be00` | DebugConsole::BuildListBox | Builds a debug console list box (`DDHelper`, `DDListBox`). Called from `0xC6EB40` |
| `0xc6a480` | DebugConsole::BuildMenu | Builds a debug console menu (`Menu compositor`, `menu filler`). Called from `0xC6CB20` |
| `0xcab9c0` | StructEditor::Build | Builds the debug structure editor's window (`StructEditor`, `Struct border2`) |
| `0xc6b400` | DebugConsole::BuildSlider | Builds a debug console slider (`Slider`, `Track`). Called from 11 places |
| `0xc32f60` | DebugConsole::BuildWatch | Builds the debug console's watch window: names and values (`Watch enviro`). Called from `DebugConsole::Build` |
| `0xa1f060` | WindowArea::Names | Makes the names of the window areas used by tutorials (`WindowArea:Dragger`, `WindowArea:ToolTipDialog`). Called from `0xA58C10`; `0xA1F350` is the same |
| `0xa1f350` | WindowArea::Names (2) | The same as `0xA1F060` |
| `0x490e80` | CameraRoute::AddWaypoint | Adds a waypoint to a camera route from the park view's key commands (`Add Waypoint`). Called from `ParkView::HandleCommands` |
| `0x463690` | AdMenu::StyleNames | Makes the advert styles' names (`Rota`, `Stat`: rotating and static) |
| `0xa23330` | TutorialSetup::StepLabel | Writes a tutorial step's label (`Step: %d`, `Global`). Called from `0xA1EF20` |
| `0xa824f0` | TrackBrowser::BuildButtons | Builds the track browser's build and cancel buttons (`TrackBrowser_BuildThis`, `_Cancel`) |
| `0x66b660` | TrackBrowser::BuildButtons (2) | The same as `0xA824F0` for another browser |
| `0x65d9b0` | MessageBox::BuildOkCancel | Builds a message box's OK and cancel buttons (`SMB_Label_OK`, `SMB_Label_Cancel`) |
| `0xaf2560` | SoakedDesigner::BuildIdentBar | Builds the bar of a Soaked! effect in the display editor (`SpecialSoakedIdentbar`). Called from two places |
| `0xa7cc20` | PoolComplex::ConfirmSell | Asks before selling a pool complex (`ConfirmSellPoolComplex_*`). Called from `0xA7CE40` |
| `0x984ec0` | ResearchCategories::Help | Gives the research categories' help texts (`Obj_Rides_*Help`, `Obj_ShopsHelp`, ...). Called from three places |
| `0x984d30` | ResearchCategories::GreyIcons | Gives the research categories' greyed icons (`Obj_Rides_*_Grey`, `Obj_Shops_Grey`, ...). Called from the InventionEditor window |
| `0x984ba0` | ResearchCategories::Icons | Gives the research categories' icons (`Obj_Rides_TransportRides`, `Obj_Rides_GentleRides`, ..., `Obj_Shops`, `Obj_Scenery`, `Obj_Paths`). Called from two places |
| `0x4840b0` | SingleItemDelete::Message | Builds the single-item delete message for animated items (`SingleItemDeleteEditor_AI_Message`, `_Title`). Called from `RCTDesktop::CreateWindows` |
| `0xbb5310` | AttractionDetails::ButtonFlags | Turns the attraction window's button flag names into values (`ButtonFlagStateChooser`, `View`, `Thoughts`, `Finances`, `Customers`, `StallItems`, `Adverts`, `StallExtras`, ...). Called from `0xBB5470` |
| `0x874840` | EntranceExitMenu::Build | Builds the entrance and exit buttons (exit first). Called from `0x6616D0` |
| `0x8746a0` | EntranceExitMenu::Build (2) | The same with the entrance first. Called from `0x6618C0` |
| `0x4c9140` | AttractionList::Categories | Makes the extra categories of the attraction list (`Pools`, `Animals`) |
| `0x4cb940` | AttractionList::ParameterNames | Makes the attraction list's column names: status, popularity, satisfaction, profit, ... (`AttractionListDialogParameter*`). Called from two places |
| `0x4e4790` | RideCarEditor::BoneNames | Lists a ride car's bones for the developer's car editor (`TransformCar`, `BoneCarFront`, `BoneCarRear`, `BoneLinkFront`, `BoneLinkRear`, `BoneCamera*`, `BoneWheel*`). Called from a window builder |
| `0xae9330` | WaterJetDesigner::FileButtons | The water jet designer's load and save buttons (`WaterJetDesigner_LoadWaterJet`, `_SaveWaterJet`) |
| `0xacd350` | LaserDesigner::FileButtons | The laser designer's load and save buttons (`LaserDesigner_LoadLaser`, `_SaveLaser`) |
| `0x9bdd70` | ParticleDesigner::FileButtons | The particle designer's load and save buttons (`TT_ParticleDesignerDialog_Load`, `_Save`) |
| `0x98c210` | FireworkDisplay::LookButtons | The firework display's automatic and free look buttons (`FireworkDisplay_AutoLook`, `_FreeLook`) |
| `0xa06ec0` | SavedSelectionGrabber::Buttons | The saved selection grabber's save and list buttons (`SavedSelectionGrabber_TT_Save`, `SavedSelection_ListHelp`) |
| `0x8b6810` | PhotoSpotWindow::Buttons | A photo spot's details button (`Staff_InfoIcon`, `PhotoSpot_Details`) |
| `0x8b14b0` | PhotoSpotWindow::Buttons (2) | The same as `0x8B6810` for another kind of photo spot |
| `0x66b7f0` | RideCategories::Icons | Gives the ride categories' icons: transport, gentle, water, roller coasters, thrill, other, junior (`Obj_Rides_*`). Called from `0x66FA40` |
| `0x88c3d0` | SignWindow::Buttons | A sign's options button (`Obj_Scenery`, `Sign_Options`) |
| `0x65c090` | ColourSwatch::Highlight (2) | Draws a colour swatch's white or blue highlight. Called from `0x65C170` |
| `0x63d3f0` | ColourSwatch::Highlight (3) | Draws a colour swatch's blue or white highlight. Called from `0x63D4B0` |
| `0x789be0` | StaffList::Columns | The staff list's thoughts and status columns |
| `0x7894e0` | StaffList::Columns (2) | The same as `0x789BE0`. Called from a window builder |
| `0xb9fbd0` | ViewMenu::AnimalWalkingGrid | Switches the grid showing where animals can walk on or off (`AnimalWalkingGridOn`, `Off`). Called from `ViewMenu::Build` and two others |
| `0xaa5f80` | PoolPathBuilder::CameraButtons | The pool path builder's guest camera and exit buttons. Called from `PoolPathBuilder::Build` |
| `0x712040` | PathMenu::CameraButtons | The path menu's guest camera and exit buttons. Called from `PathMenu::Build` |
| `0x9da3a0` | CampaignDialog::Kinds | The kinds of marketing campaign: attraction vouchers, advertising, park vouchers (`Campaign_*`). Called from `0x9DDC20` |
| `0x617740` | LoadingScreen::Name | Picks the loading screen (`LoadingScreen`, `_Chinese`, `_SChinese`). Called from `0x618C70` |
| `0x6bacc0` | FinanceWindow::BuildSummary | Builds the finance window's monthly summary (25 controls), noting when the entrance fee can't be changed (`FinancialSummary_EntranceFee_CannotBeChanged_TT`). Called from `0x6BBAE0` |
| `0x4caad0` | AttractionList::BuildItem | Makes an attraction's line in the attraction list, with its figures (`AttractionNumbers::Rate`), and pools (`Obj_Pools`). Called from three places |
| `0xa49ab0` | FrontScreen::BuildTutorialSelect | Builds the front screen's tutorial selection (`FrontScreen_SelectTutorial`, 46 controls) |
| `0x677110` | RideSelectionWindow::BuildList | Builds the ride selection list: rides, saved designs, animals (`WildAnimal::Describe`), and guest groups by kind (`Group_Containing_`, `_AdultMale`, `_AdultFemale`, `_ChildMale`, `_ChildFemale`). Called from three places |
| `0x78a960` | ParkDialog::BuildGuestWants | Builds the park window's page of what guests want, such as pools (`PeepsWantPools`). Called from `0x65F830` |
| `0x64c060` | VIPDetails::BuildStats | Builds a VIP's statistics page (on a curved background). Called from `0x64C950` and `VIPDetails::Build` |
| `0x9b8290` | ParticleDescWindow::BuildMotion | Builds the particle editor's motion page (`Particle Relative`). Called from `0x9BB220` |
| `0x9b8b40` | ParticleDescWindow::BuildAppearance | Builds the particle editor's appearance page. Called from `0x9BB220` |
| `0x64ea20` | GuestDetails::BuildStats | Builds a guest's statistics page. Called from `0x650F20` |
| `0x6de470` | ThemedAttractionsCondition::Build | Builds the scenario editor's list of styles for a themed attractions condition (`StylesListBox`). Called from `0x6DEF40` |
| `0x64f1c0` | GuestDetails::BuildThoughts | Builds a guest's thoughts page. Called from `0x64F8D0` and `0x650180` |
| `0x68b800` | FileDialog::BuildFileName | Builds the file dialog's file name box (`FD_TT_filename`). Called from `0x68C370` |
| `0x611460` | GameOptionsDialog::BuildTabs | Builds the options window's tabs, with the camera settings tab (`GameOptionsDialog_CameraSettingsTabHelp`). Called from `0x5F8160` |
| `0x675be0` | ObjectSelection::BuildItem | Makes an object's item in an object selection window, with its expansion pack badge (`ObjectSelectionWindow_AOPBadge_`). Called from five places |
| `0x669750` | DetailsWindow::BuildBackground | Builds a details window's curved background. Called from four places |
| `0x448790` | AttractionDetails::RideCamButton | Builds the double-click ride camera button of an attraction's window (`DoubleClickRideCam`). Called from `AttractionDetails::BuildTabs` |
| `0x6888d0` | FileDialog::Rename | Renames a file from the file dialog, reporting `FD_RenameFailed`. Called from `0xAD11A0` |
| `0xaf4150` | WaterJetDesigner::DefaultName | Gives a new water jet design its default name (`WaterJetName_`) |
| `0xad7e10` | LaserDesigner::DefaultName | Gives a new laser design its default name (`LaserName_`) |
| `0x6e78c0` | NumSceneryCondition::Build | Builds the scenario editor's list of scenery kinds for a scenery condition (`ClassesListBox`). Called from `0x6E8100` |
| `0x66ac40` | TrackBrowser::DesignName | Writes a track design's name (`TrackDesign_`, `Design(`...). Called from `TrackBrowser::ItemName` and `0x66C250` |
| `0x7469d0` | AvatarEditor::AnimationName | Looks up a person animation's display name (`PeopleAnim_Name_%d`). Called from six places in the people designer |
| `0x6eedb0` | CareerMap::Padlock | Draws a scenario's padlock (`GUI2Padlock`). Called from `0x6E93E0` |
| `0x756d60` | Entertainer::BuildCostumeMenu | Builds an entertainer's costume menu (`Entertainer_Costume_Menu`). Called from `0x7573E0` |
| `0xca9ca0` | DebugEditor::UInt16 | The debug console's editor for a 16-bit unsigned value (`UInt16Editor`); `0xCA97A0`, `0xCAA690`, `0xCAA1A0`, `0xCA92B0`, `0xCA8DC0`, `0xCA88D0`, `0xCAAB80` and `0xCAB070` are the same for the other value types |
| `0xca97a0` | DebugEditor::Int16 | The debug console's editor for a 16-bit value (`Int16Editor`) |
| `0xcaa690` | DebugEditor::UInt8 | The debug console's editor for an 8-bit unsigned value (`UInt8Editor`) |
| `0xcaa1a0` | DebugEditor::Int8 | The debug console's editor for an 8-bit value (`Int8Editor`) |
| `0xca92b0` | DebugEditor::UInt32 | The debug console's editor for a 32-bit unsigned value (`UInt32Editor`) |
| `0xca8dc0` | DebugEditor::Int32 | The debug console's editor for a 32-bit value (`Int32Editor`) |
| `0xca88d0` | DebugEditor::Int | The debug console's editor for an integer (`IntEditor`) |
| `0xcaab80` | DebugEditor::Float | The debug console's editor for a float (`FloatEditor`). Called from `0xCACE00` |
| `0xcab070` | DebugEditor::Bool | The debug console's editor for a true / false value (`BoolEditor`) |
| `0xcace00` | DebugEditor::Vector | The debug console's editor for a vector: x, y, z and a zero button. `0xCACA70` is another version |
| `0xcaca70` | DebugEditor::Vector (2) | The same as `0xCACE00` |
| `0x6b9270` | FinanceWindow::Build | Builds the finance management window (`FinancialManagement_Title`). Called from `0x619960` and `0x6B97C0` |
| `0x6e5f30` | SpeciesCondition::Build | Builds the scenario editor's species list for an animal condition (`SpeciesListBox`). Called from `0x6E6BC0` |
| `0x6e3400` | SpeciesCondition::Build (2) | The same as `0x6E5F30` for another condition. Called from `0x6E3E50` |
| `0xa371a0` | StallItemsWindow::BuildItem | Makes an item of a stall's items window, dragged on to add it (`StallItemsWindow_DragOn`, `%ITEM%`). Called from a window builder |
| `0xa37610` | StallItemsWindow::SelectType | Builds the stall type choice (`Stall_SelectType`). Called from a window builder |
| `0x68c930` | FileDialog::NoFiles | Shows that a folder has no files (`FD_NoFiles`) |
| `0x63db00` | ColourSwatches::Build | Builds a row of colour swatches (`Swatch`). Called from `0x65A720` |
| `0x65cc40` | ColourSwatches::Build (2) | The same as `0x63DB00`. Called from `0x65B5F0` |
| `0x53bf80` | AttractionMusic::Build | Builds an attraction's music button and tooltip (`AttractionViewDialogMusicToolTip`). Called from six places |
| `0x53ba70` | AttractionMusic::None | Shows that an attraction plays no music (`AttractionViewDialogNoMusic`) |
| `0xb94fd0` | EnclosureWindow::WalkInfo | The button that shows where animals can walk in an enclosure (`ToggleEnclosureWalkInfo_TT`). Called from a window builder |
| `0x9c37a0` | ParticleDesigner::BarIcon | Makes the particle designer's bar icons (`GUI2_ParticleDesigner_BarIcon`) |
| `0x9c1af0` | ParticleDesigner::BuildBar | Builds a particle designer bar (`GUI2_ParticleDesigner_Bar`). Called from three places |
| `0x6b17b0` | WindowArea::SetRelation | Places a window area in the window tree like `Window::SetRelation` (`.\Code\Common\Gui\WindowArea.cpp`; a broken link stops at a breakpoint with the assert `_relation`, whose message is a joke). Called from `0xA3B660` |
| `0xa2de90` | StallItemLine::Profit | Writes a stall item's profit line (`StallItemLine_Profit`, `%NAME%`, `%PROFIT%`). Called from `0xA2EB70`; `0xA35F80` is the same |
| `0xa35f80` | StallItemLine::Profit (2) | The same as `0xA2DE90`. Called from the StallItemLine window |
| `0x686c70` | LoadSaveDialog::NoPacks | Says an expansion pack the file needs is missing (`LoadSaveDialog_Error_NoPacks`) |
| `0x63ccb0` | Window::BuildCloseButton | Makes a window's close button (`CloseButton`). Called from a window builder |
| `0x62aca0` | Editor::RotateButton | Makes an editor's rotate button (`ATE_TT_Rotate`). Called from seven editors |
| `0x4ebeb0` | AttractionEditor::NewSegmentButton | Makes the new track segment button (`ATE_TT_NewTrackSegment`). Called from `AttractionEditor::Build` |
| `0x4bdb80` | AttractionFinances::Labels | Writes the labels of an attraction's finances page (`AttractionFinancesDialogLabel_`). Called from a window builder |
| `0xac8300` | Designer::Instructions | Shows the laser and water-jet designers' instructions (`BlenderUIIntsructions_TT`). Called from three places |
| `0x7eedc0` | StaffWindow::Title | The staff window's title (`Man_Staff`). Called from `0x619B10` |
| `0x98e070` | MixMaster::NoScrub | The mix master's music that can't be scrubbed (`Fireworks_MixMaster_MusicNoScrub`). Called from two places |
| `0x6b0990` | Window::Layout | Lays a window out again when it needs it (flags at +0x94): recalculates its size and position. Called from 617 places |
| `0x6b0ea0` | Window::~Window | Destroys a window (tables `0xF607D8` / `0xF607C0`): takes it out of the window tree and destroys its children. Called from 608 places |
| `0x6ad000` | Window::DrawHighlight | Draws a window's highlight when the desktop has highlighting on (a translucent blue tint). Called from 607 places |
| `0x6b0940` | Window::SetHeight | Sets a window's height from its text height (`0x6ADCC0`), keeping its position. Called from 602 places |
| `0x6afe10` | Window::SendClick | Sends a window a click event (event table `0xF60700`), telling the tutorial when it is watching (`0xA1F8A0`). Called from 592 places; `0x6AFDC0` sends the other click event |
| `0x6afdc0` | Window::SendRelease | Sends a window its other click event (event table `0xF606F8`), telling the tutorial when it is watching (`0xA1F890`). Called from 588 places |
| `0x6ad520` | Window::HandleEvent | Gives a window an event (virtual slot 0x31), passing it to its parent window when it doesn't handle it. Called from 557 places |
| `0x4dd260` | EntranceExitPickArea::EntranceExitPickArea | Constructor of the `EntranceExitPickArea` window (made by `0x4dd870`, 216 bytes): builds the window base (`0x63B780`) with the name `EntranceExitPickArea`, sets its tables `0xf344b8`, `0xf344a0`, then builds and lays out its contents (Window::SetRelationAndLayout, MainWindow::Build, Window::UpdateRect); returns it |
| `0x4e2d80` | CircularPanelPopupWindow::CircularPanelPopupWindow | Constructor of the `GUI2_CircularPanelPopupWindow` window (made by `0x4e3080`, 216 bytes): builds the window base (`0x63B780`) with the name `GUI2_CircularPanelPopupWindow`, sets its tables `0xf34c40`, `0xf34c2c`, then builds and lays out its contents (MainWindow::Build, Window::SetRelationAndLayout, Window::UpdateRect); returns it |
| `0x5a0b30` | TrackedRideGraphsDialog::TrackedRideGraphsDialog | Constructor of the `TrackedRideGraphsDialog` window (made by `0x633560`, 224 bytes): builds the window base (`0x63B780`) with the name `TrackedRideGraphsDialog`, sets its tables `0xf3dca0`, `0xf3dc88`, then builds and lays out its contents (Window::SetRelation, Window::UpdateRect); returns it |
| `0x64e550` | RidesVisitedArea::RidesVisitedArea | Constructor of the `GUI2_RidesVisitedArea` window: builds the window base (`0x63B780`) with the name `GUI2_RidesVisitedArea`, using `GUI2_GuestBackground`, `GUI2_StripeArea` for its parts, sets its tables `0xf4ac20`, `0xf4ac0c`, `0xf4b6f0`, `0xf4b6d8`, then builds and lays out its contents (Window::SetRelationAndLayout, TextRef::~TextRef, Window::UpdateRect); returns it |
| `0x650a60` | GuestPassportArea::GuestPassportArea | Constructor of the `GUI2_GuestPassportArea` window (made by `0x651070`, 244 bytes): builds the window base (`0x63B780`) with the name `GUI2_GuestPassportArea`, using `GUI2_GuestBackground` for its parts, sets its tables `0xf4b2a0`, `0xf4b28c`, `0xf4b284`, `0xf4b6f0`, then builds and lays out its contents (Window::UpdateRect, TextRef::~TextRef, Name::Assign, NameTable::Set, String::~String, Window::SetRelationAndLayout); returns it |
| `0x668700` | StallExtrasDialog::StallExtrasDialog | Constructor of the `GUI2_StallExtrasDialog` window (made by `0x6331f0`, 220 bytes): builds the window base (`0x63B780`) with the name `GUI2_StallExtrasDialog`, sets its tables `0xf4ee00`, `0xf4edec`, then builds and lays out its contents (ShopExtraTick window, Window::UpdateRect, Window::SetRelationAndLayout); returns it |
| `0x6689c0` | StallItemsDialog::StallItemsDialog | Constructor of the `GUI2_StallItemsDialog` window (made by `0x633140`, 220 bytes): builds the window base (`0x63B780`) with the name `GUI2_StallItemsDialog`, sets its tables `0xf4ecf8`, `0xf4ece0`, then builds and lays out its contents (MultiSelectSwatch window, Window builder, Window::UpdateRect, Window::SetRelationAndLayout); returns it |
| `0x71a2e0` | PathListPanel::PathListPanel | Constructor of the `PathListPanel` window (made by `0x711e50`, 220 bytes): builds the window base (`0x63B780`) with the name `PathListPanel`, sets its tables `0xf6a088`, `0xf6a070`, then builds and lays out its contents (Window::UpdateRect, Window::SetRelationAndLayout); returns it |
| `0x824ea0` | RideEventArea::RideEventArea | Constructor of the `GUI2_RideEventArea` window: builds the window base (`0x63B780`) with the name `GUI2_RideEventArea`, sets its tables `0xf76690`, `0xf76678`; returns it |
| `0x8250f0` | RideEventArea::Create | Makes a new `GUI2_RideEventArea` window for this object: allocates 220 bytes and builds it in place (the window base `0x63B780` with that name, the object's +0x40 as its parent and its +0x4C kept at +0xD8, tables `0xf76690`, `0xf76678`); returns it, or null when out of memory |
| `0x8bac30` | WaterCannonArea::WaterCannonArea | Constructor of the `GUI2_WaterCannonArea` window (made by `0x8bb130`, 256 bytes): builds the window base (`0x63B780`) with the name `GUI2_WaterCannonArea`, sets its tables `0xf7a958`, `0xf7a940`, `0xf26ae0`, `0xf7aa4c`, then builds and lays out its contents (Handle::For, Window::SetRelationAndLayout); returns it |
| `0x8cbb10` | PaintMasterPanel::PaintMasterPanel | Constructor of the `PaintMasterPanel` window (made by `0x8cbe40`, 220 bytes): builds the window base (`0x63B780`) with the name `PaintMasterPanel`, sets its tables `0xf7af30`, `0xf7af18`, then builds and lays out its contents (Window::UpdateRect, Window::SetRelationAndLayout); returns it |
| `0x8d2870` | LandMasterPanel::LandMasterPanel | Constructor of the `LandMasterPanel` window (made by `0x8d2b40`, 220 bytes): builds the window base (`0x63B780`) with the name `LandMasterPanel`, sets its tables `0xf7b468`, `0xf7b454`, then builds and lays out its contents (Window::UpdateRect, Window::SetRelationAndLayout); returns it |
| `0x9a80a0` | FireworkMasterDialog::FireworkMasterDialog | Constructor of the `GUI2_FireworkMasterDialog` window (made by `0x9a83f0`, 220 bytes): builds the window base (`0x63B780`) with the name `GUI2_FireworkMasterDialog`, sets its tables `0xf81e50`, `0xf81e38`, then builds and lays out its contents (FireworkDisplayDialog::Build, Window::UpdateRect, Window::SetRelationAndLayout, MainWindow::Build); returns it |
| `0xa090f0` | SavedSelectionPlacerPanel::SavedSelectionPlacerPanel | Constructor of the `SavedSelectionPlacerPanel` window (made by `0xa09390`, 220 bytes): builds the window base (`0x63B780`) with the name `SavedSelectionPlacerPanel`, sets its tables `0xf875f0`, `0xf875d8`, then builds and lays out its contents (Window::SetRelationAndLayout, Window::UpdateRect); returns it |
| `0xa7fa90` | ChangingRoomPickPanel::ChangingRoomPickPanel | Constructor of the `ChangingRoomPickPanel` window (made by `0xa85030`, 252 bytes): builds the window base (`0x63B780`) with the name `ChangingRoomPickPanel`, sets its tables `0xf3a074`, `0xf8ffa8`, `0xf8ff94`, `0xf8ff90`, then builds and lays out its contents (Window::SetRelationAndLayout, Window::UpdateRect); returns it |
| `0xb0ed00` | PlaceWAItemArea::PlaceWAItemArea | Constructor of the `PlaceWAItemArea` window (made by `0xba7620`, 252 bytes): builds the window base (`0x63B780`) with the name `PlaceWAItemArea`, sets its tables `0xf97b68`, `0xf97b88`, `0xf97b70`, `0xf97b6c`, then builds and lays out its contents (Window::SetRelationAndLayout, Window::UpdateRect); returns it |
| `0x631d30` | AttractionDetails::TabAvailable | Whether a details tab (0-11) applies to the attraction: each tab has a flag (0 2, 1 0x4000, 2 0x8000, 3 8, 4 0x20, 5 0x40, 6 0x10000, 7 0x80000, 8 0x20000, 9 0x10, 10 4, 11 0x100); for attractions whose kind (+1 of the type at +0xA8 of +0x34) is above 2 the tab is left out when the type's flags (+0x40) have it, otherwise and for other tabs it is available. Used by AttractionDetails::BuildTabs and StallDetails::BuildTabs |
| `0x6ad560` | Window::ScreenPosition | A window's position on the screen: its own position (+0x7C) plus each parent's (+0x3C up the chain), adding a parent's scroll offset (+0x8C) when the child has flag 0x1000. Used by ColourSwatches::Build, the window builders and Window::Layout |
| `0x617290` | Vector2::SnapToGrid | Snaps a 2D point down to a grid of the given spacing: each coordinate becomes floor(value / spacing) x spacing. Called from FrontScreen::BuildExpansionBadges |
| `0x4ef080` | TrackDesigner::CarSettingMode | Sets the track designer's car setting mode (+0x810, left alone while it is 0) from the given car's value at +0x1BC: 3 when it is negative, 2 when positive, 1 when zero. Called from TrackDesigner::BuildCarSetting |
| `0x45e430` | AdMenu::ScaleLevel | Turns a ratio into a level for the ad media: 8a / b of 64 gives 0, 32 gives 1, 16 gives 2, 4 gives 4, 2 gives 5, 1 gives 6, anything else 3. Called from AdMenu::ShowMedia |
| `0xbe7780` | Rect::Contains | Whether a point lies in a rectangle (left, top, right, bottom), edges included. Called from GUIRenderer::Render |
| `0x543b90` | RideOpModeDialog::DefaultSecondValue | When the dialog has its second setting control (+0x190): if its value (+0x19C) is negative, sets it and the control's value (+0x28C) to 20 and refreshes the control (`0x6AFC20`); otherwise `0x6B0990`. Called from RideOpModeDialog::Build |
| `0x543b40` | RideOpModeDialog::DefaultFirstValue | The same for the first setting control (+0x17C, value +0x188, the control's +0x288). Called from RideOpModeDialog::Build |
| `0x6ae7c0` | Window::ForEachChild | Hands a request to every child window (the list at +0x24), keeping its place with a cursor node in the list so children can be removed while it runs (stopping the program if the list is found broken) |
| `0x681280` | WindowRequest::Visit | The visit shared by the window requests: gets the child's window interface (`0x6812A0`) and, when it has one, calls the request's own slot 1 with it |
| `0x6812a0` | QueryInterface 0x1305d10 | Returns the given object's interface with the id in `0x1305D10` (the window interface; asking its class through virtual slot 1), or null |
| `0x6ad740` | WindowRequest::CallSlot2 | A window request's own function: calls the child's slot 2 with the request's value (+4) |
| `0x6ad780` | WindowRequest::CallSlot4 | Calls the child's slot 4 with the request's value |
| `0x6adaa0` | WindowRequest::CallSlot7 | Calls the child's slot 7 with the request's value |
| `0x6adae0` | WindowRequest::CallSlot6 | Calls the child's slot 6 with the request's value |
| `0x6ade00` | WindowRequest::CallSlot9 | Calls the child's slot 9 with the request's value |
| `0x6ade40` | WindowRequest::CallSlot10 | Calls the child's slot 10 with the request's value |
| `0x6afd60` | Window::ChildrenSlot2 | Passes the call on to slot 2 of every child (request table `0xF606E8`), then `0x6A1E20`. Shared by the window classes |
| `0x6afd90` | Window::ChildrenSlot4 | Passes the call on to slot 4 of every child (request table `0xF606F0`), then `0x6A1E20`. Shared by the window classes |
| `0x6af3a0` | Window::ChildrenSlot7 | Passes the call on to slot 7 of every child (request table `0xF60708`). Shared by the window classes |
| `0x6af3c0` | Window::ChildrenSlot6 | Passes the call on to slot 6 of every child (request table `0xF60710`). Shared by the window classes |
| `0x6af420` | Window::ChildrenSlot9 | Passes the call on to slot 9 of every child (request table `0xF60718`). Shared by the window classes |
| `0x6af440` | Window::ChildrenSlot10 | Passes the call on to slot 10 of every child (request table `0xF60720`). Shared by the window classes |
| `0x6acda0` | Window::ZeroPair | Gives (0, 0). Shared by the window classes |
| `0x63aa90` | Window::ChildrenSlot4ThenDelete | Passes slot 4 on to every child (`0x6AFD90`), then deletes the window (its deleting destructor with 1). Shared by several dialogs |
| `0x67abe0` | Panel::ChildrenSlot2ThenRelate | Passes slot 2 on to every child (`0x6AFD60`), then, when it has a related window (+0xF8), Window::SetRelation with 2. Shared by panels such as ChangingRoomPickPanel and RidesVisitedArea |
| `0x67ac00` | Panel::ChildrenSlot4ThenLayout | Passes slot 4 on to every child (`0x6AFD90`), then lays the window out when it has a related window (+0xF8). Shared by the same panels |
| `0x490000` | Window::RectFromSize | Sets the window's rectangle (Window::UpdateRect) to start at 0,0 with its own width and height (from +0x7C..+0x88) |
| `0xac7fb0` | Window::PointToScreen | Turns a point inside the window into a screen position: the window's offset (-0x48 from this interface) plus the point plus Window::ScreenPosition |
| `0x5f7890` | AttractionPersonStrategy::PointSlot20 | Turns the given point into one relative to the window (its position +0x7C plus offset +0x8C) and hands it to slot 20 of the window kept in `0x12F83F0` |
| `0x5f7750` | AttractionPersonStrategy::PointSlot35 | The same, handing the point to slot 35 of the window in `0x12F83F0` |
| `0x5f77b0` | AttractionPersonStrategy::PointSlot38 | The same, handing the point to slot 38 |
| `0x5f7810` | AttractionPersonStrategy::PointSlot41 | The same, handing the point to slot 41 |
| `0x99b2b0` | Window::PointToScreen2 | The same as `0xAC7FB0`: the window's offset plus the point plus Window::ScreenPosition. Shared by window classes such as InventibleAnimation |
| `0xac7020` | FloatBlenderElement::Blend | Sets its value (+0x40) between those of two elements (their interface `0x13CE434`) by the weight given: w x second + (1 - w) x first. Slot 8 of FloatBlenderElement |
| `0x63acb0` | Window::DeleteChild40 | Deletes the window kept at +0x40 (deleting destructor with 1), then `0x6A1E20`. Shared by several dialogs and panels |
| `0x63b4e0` | Window::DestroyMember28 | Clears +0x24, and destroys and frees the object kept at +0x28, then `0x6A1E20`. Shared by several dialogs and panels |
| `0x58fbe0` | StallInstance::OpenDetails | Opens the stall's details window: makes a 576-byte window with StallDetails::BuildTabs for this stall, then calls its slot 2 with the argument. Slot 2 of StallInstance |
| `0x52f4c0` | RideInstance::OpenDetails | Opens the ride's details window: a 748-byte window made by AttractionDetails::BuildRideTabs for this ride, then its slot 2 with the argument. Slot 2 of RideInstance and AnimatedRideInstance |
| `0xa7b310` | ChangingRoomInstance::OpenDetails | Opens the changing room's details window: a 656-byte window made by AttractionDetails::BuildPoolTabs, then its slot 2 with the argument. Slot 2 of ChangingRoomInstance |
| `0x4beea0` | AttractionInstance::OpenDetails | Opens an attraction's details window: a 400-byte window made by AttractionDetails::BuildTabs, then, unless the argument is -1, its slot 2 with it. Slot 2 of AttractionInstance and SpecialAttractionInstance |
| `0x67a8a0` | Window::SetValueEC | Sets the window's value at +0xEC and calls its slot 57. Used by about 70 window builders |
| `0x6a0040` | ListWindow::SetSelection | Changes the selected entry (+0x174): when it differs, tells every child the old selection is going (request table `0xF5E5D0`), stores the new one, then tells every child the new one (request table `0xF5E5C0`). Used by about 55 functions |
| `0x6a0b20` | ListWindow::SetCount | Changes the number of entries (+0x170): `0x460F20` with it, the selection cleared (ListWindow::SetSelection -1) when it is no longer inside, then `0x6A0560` |
| `0x6835a0` | WindowOwner::~WindowOwner | Puts its table back (`0xF270F4`), clears the window's back pointer (+0xDC) and deletes the window unless it is already being deleted (+0x264) |
| `0x6af6c0` | WindowList::WindowList | Constructor of a window list (table `0xF60794`): an empty list (head at +4 / +0x10) and the value given at +0x1C |
| `0x6b0150` | WindowList::~WindowList | Puts its table back (`0xF60794`) and removes every entry (`0x6AF760`) until the list is empty |
| `0x6abd60` | CheckBox::BindFlag | Ties a check box to a flag: keeps the address (+0x13C) and the bit (+0x140) and sets the box from it (Window::SetChecked; unticked when there is no address) |
| `0x67a580` | Window::SetChecked | Sets the window's ticked state (slots 62 and 63) when it differs, then redraws it (`0x67A410`). Used by about 40 functions |
| `0x67a8c0` | Window::SetRelatedValue | Sets +0xF0 of the related window (+0xF8), when there is one |
| `0x6a1e20` | Window::UpdateEntries | Runs `0x6A1E70` for each entry of the window's list (+4) |
| `0x63b3d0` | Window::AddWithText | `0x67B4B0` with the third argument and 0, then `0x63B270` with the first two |
| `0x6b01e0` | Window::SetContent | Sets the window's content (+0xCC) and, when asked, sizes the window to it (its position plus the content's size, `0x6AFAF0`) |
| `0x6afd40` | Window::SetRectAndRefresh | `0x6AFAF0` with the rectangle given, then `0x6AFC20` |
| `0x6a8300` | NumberField::BindFloat | With a value: keeps it (+0xF4), sets the mode (+0xF0) to 1, `0x6A9E50` with 0x800, 0x800 and rebuilds (`0x6A8220`); without one the mode becomes 0 |
| `0x6a8380` | NumberField::BindInt | With a value: keeps it (+0xF4), sets the mode to 2 and rebuilds; without one the mode becomes 0 |
| `0x6a82d0` | NumberField::SetLimits | Keeps three values (+0xF8, +0xFC, +0x100) and rebuilds (`0x6A8220`) |
| `0x6a8350` | NumberField::SetLimits2 | The same (another copy of the code) |
| `0x67b4f0` | Label::SetText | Sets the text of the window's text part (+0x114 of the window at +0xF0, sharing the text given) and updates the window (`0x6AFC20`) |
| `0x67b710` | Button::SetImages | Sets a button's images: two pairs (+0x104/+0x10C unticked, +0x108/+0x110 ticked, the second of each pair while highlighted (flag 0x20)), +0x114 and a seventh at +0x128; shows the one for its state now (Window::SetContent) and, when asked, sizes the button to the first image |
| `0x67b800` | Button::SetSingleImage | Gives a button the same image for every state (Button::SetImages with it six times) and the last value given. Used by about 95 window builders |
| `0x63c840` | WindowPart::WindowPart | Constructor of a window part (tables `0xF49148`, `0xF49134`, `0xF49108`; built on WindowList with the value given) that registers its class's fields once (the class description in the argument, or `0x12F7E90`) |
| `0x41bea0` | Window::SetText114 | Makes the window's text at +0x114 share the text given |
| `0x6a9e50` | Window::SetFlagsDC | Sets the bits of the window's flags (+0xDC) chosen by the mask to the values given, updating what depends on bit 0 (its text style and values from `0x13037D8` and `0x1302BD8`) when that bit changes |
| `0x6a7920` | Window::SetOffsets148 | Sets +0x148 and +0x14C of the window at +0xE4 to the two (signed byte) values given |
| `0x6a2b80` | Window::SetPairE4 | Sets +0xE4 and +0xE8 and clears flag 0x1000000 of +0x94 |
| `0x6a3120` | Window::Set10C | Sets +0x10C and clears flag 0x1000000 of +0x94 |
| `0x67bb20` | Window::ShowContent | Sets the content (`0x4D7E70`), then updates the window (`0x6AFC20`) when there is content, otherwise `0x6B0990` |
| `0x63a8e0` | Window::Reattach | `0x6B0990`, the window updated (`0x6AFC20`), then Window::SetRelationAndLayout with 0, +0xF4 and the position (+0x7C) of the window at +0xE0 |
| `0x41f210` | Text AdMenu_CustomImageBrowser | Makes the given text a copy of the game text `AdMenu_CustomImageBrowser` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x4c9090` | Text AttractionListDialog_Title | Makes the given text a copy of the game text `AttractionListDialog_Title` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x607c50` | Text GameOptionsDialog_Title | Makes the given text a copy of the game text `GameOptionsDialog_Title` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x619510` | Text LGD_Title | Makes the given text a copy of the game text `LGD_Title` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x622fe0` | Text ProgramInfoDialog_Title | Makes the given text a copy of the game text `ProgramInfoDialog_Title` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x62b910` | Text SGD_Title | Makes the given text a copy of the game text `SGD_Title` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x65d6d0` | Text MusicGenre_Title | Makes the given text a copy of the game text `MusicGenre_Title` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x65f440` | Text PID_Title | Makes the given text a copy of the game text `PID_Title` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x66ba40` | Text TrackBrowser_Title | Makes the given text a copy of the game text `TrackBrowser_Title` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x6b48f0` | Text MapDialog_Title | Makes the given text a copy of the game text `MapDialog_Title` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x6b9710` | Text FinancialManagement_Title | Makes the given text a copy of the game text `FinancialManagement_Title` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x6f95c0` | Text VIPConditionSetupDialog_Title | Makes the given text a copy of the game text `VIPConditionSetupDialog_Title` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x6ff580` | Text VIPRouteDialog_Title | Makes the given text a copy of the game text `VIPRouteDialog_Title` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x787520` | Text Man_GuestsHelp | Makes the given text a copy of the game text `Man_GuestsHelp` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x888050` | Text SIDSE_Title | Makes the given text a copy of the game text `SIDSE_Title` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x97d900` | Text IED_Title | Makes the given text a copy of the game text `IED_Title` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x980600` | Text Research_Title | Makes the given text a copy of the game text `Research_Title` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x98c370` | Text FireworkDisplay_NextDisplayTT | Makes the given text a copy of the game text `FireworkDisplay_NextDisplayTT` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x9a5ff0` | Text FireworkMaster_Save_Title | Makes the given text a copy of the game text `FireworkMaster_Save_Title` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x9a60a0` | Text FireworkMaster_Save_Title | Makes the given text a copy of the game text `FireworkMaster_Save_Title` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x9a6150` | Text FireworkMaster_Load_Title | Makes the given text a copy of the game text `FireworkMaster_Load_Title` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x9a6200` | Text FireworkMaster_Delete | Makes the given text a copy of the game text `FireworkMaster_Delete` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x9a6730` | Text FireworkMaster_Load_Title | Makes the given text a copy of the game text `FireworkMaster_Load_Title` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x9a67e0` | Text FireworkMaster_Save_Title | Makes the given text a copy of the game text `FireworkMaster_Save_Title` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x9a6890` | Text FireworkMaster_Load_Title | Makes the given text a copy of the game text `FireworkMaster_Load_Title` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x9a6940` | Text FireworkMaster_Delete | Makes the given text a copy of the game text `FireworkMaster_Delete` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0xa08450` | Text SavedSelection_Load | Makes the given text a copy of the game text `SavedSelection_Load` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0xa08500` | Text SavedSelection_Save | Makes the given text a copy of the game text `SavedSelection_Save` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0xa085b0` | Text SavedSelection_Delete | Makes the given text a copy of the game text `SavedSelection_Delete` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0xacd140` | Text LaserDesigner_LoadLaser | Makes the given text a copy of the game text `LaserDesigner_LoadLaser` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0xacd1f0` | Text LaserDesigner_SaveLaser | Makes the given text a copy of the game text `LaserDesigner_SaveLaser` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0xacd2a0` | Text LaserDesigner_DeleteLaser | Makes the given text a copy of the game text `LaserDesigner_DeleteLaser` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0xacd610` | Text LaserDesigner_Title | Makes the given text a copy of the game text `LaserDesigner_Title` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0xae9120` | Text WaterJetDesigner_LoadWaterJet | Makes the given text a copy of the game text `WaterJetDesigner_LoadWaterJet` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0xae91d0` | Text WaterJetDesigner_SaveWaterJet | Makes the given text a copy of the game text `WaterJetDesigner_SaveWaterJet` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0xae9280` | Text WaterJetDesigner_DeleteWaterJet | Makes the given text a copy of the game text `WaterJetDesigner_DeleteWaterJet` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0xae95f0` | Text WaterJetDesigner_Title | Makes the given text a copy of the game text `WaterJetDesigner_Title` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0xb02400` | Text DartCamName | Makes the given text a copy of the game text `DartCamName` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0xb15680` | Text WildAnimalState_Eat | Makes the given text a copy of the game text `WildAnimalState_Eat` (looked up by name with `0x4040F0`), or the empty text when it has none; returns it |
| `0x67d320` | Window::PointToScreenB0 | The same as Window::PointToScreen with the window's offset at -0x50 from this interface |
| `0x67f300` | Window::PointToScreen78 | The same as Window::PointToScreen with the window's offset at -0x88 from this interface |
| `0x996680` | Window::PointToScreen50 | The same as Window::PointToScreen with the window's offset at -0xb0 from this interface |
| `0x9973d0` | Window::PointToScreenA0 | The same as Window::PointToScreen with the window's offset at -0x60 from this interface |
| `0x9b9cd0` | Window::PointToScreenB8 | The same as Window::PointToScreen with the window's offset at -0x48 from this interface |
| `0xa213a0` | Window::PointToScreenB8 | The same as Window::PointToScreen with the window's offset at -0x48 from this interface |
| `0xa21a40` | Window::PointToScreenB8 | The same as Window::PointToScreen with the window's offset at -0x48 from this interface |
| `0xaccb40` | Window::PointToScreenB8 | The same as Window::PointToScreen with the window's offset at -0x48 from this interface |
| `0xae2090` | Window::PointToScreenB0 | The same as Window::PointToScreen with the window's offset at -0x50 from this interface |
| `0x5fd440` | Window::SetText GameOptionsDialog_KeyBindingsHelp | Passes slot 2 on to every child (Window::ChildrenSlot2), then sets the window's text to the game text `GameOptionsDialog_KeyBindingsHelp` |
| `0x5fe210` | Window::SetText GameOptionsDialog_GameSettingsTabHelp | Passes slot 2 on to every child (Window::ChildrenSlot2), then sets the window's text to the game text `GameOptionsDialog_GameSettingsTabHelp` |
| `0x601980` | Window::SetText GameOptionsDialog_GraphicSettingsTabHelp | Passes slot 2 on to every child (Window::ChildrenSlot2), then sets the window's text to the game text `GameOptionsDialog_GraphicSettingsTabHelp` |
| `0x605810` | Window::SetText GameOptionsDialog_CameraSettingsTabHelp | Passes slot 2 on to every child (Window::ChildrenSlot2), then sets the window's text to the game text `GameOptionsDialog_CameraSettingsTabHelp` |
| `0x607500` | Window::SetText GameOptionsDialog_ScenicSettingsTabHelp | Passes slot 2 on to every child (Window::ChildrenSlot2), then sets the window's text to the game text `GameOptionsDialog_ScenicSettingsTabHelp` |
| `0x68fe00` | OptionList::Set68FE00 | Checks that the index is below the number of options (+0x164; an assertion `out of range` otherwise) and sets the option's text (`0x6ae070`) |
| `0x68fe50` | OptionList::Set68FE50 | Checks that the index is below the number of options (+0x164; an assertion `out of range` otherwise) and sets the option's text (`0x67a7a0`) |
| `0x68fef0` | OptionList::Set68FEF0 | Checks that the index is below the number of options (+0x164; an assertion `out of range` otherwise) and sets the option's text (`0x67bb20`) |
| `0x68ff40` | OptionList::Set68FF40 | Checks that the index is below the number of options (+0x164; an assertion `out of range` otherwise) and sets the option's text (`0x67b4f0`) |
| `0x68ffe0` | OptionList::Set68FFE0 | Checks that the index is below the number of options (+0x164; an assertion `out of range` otherwise) and sets the option's text (`0x67a8c0`) |
| `0x690030` | OptionList::Set690030 | Checks that the index is below the number of options (+0x164; an assertion `out of range` otherwise) and sets the option's text (`0x67a8a0`) |
| `0x6baae0` | Window::SetText FinancialSummary_MonthlySummaryHelp | Passes slot 2 on to every child (Window::ChildrenSlot2), then sets the window's text to the game text `FinancialSummary_MonthlySummaryHelp` |
| `0x97d9b0` | FilteredList::Build | Fills a filtered item list; when nothing matches it shows the game text `NoItemsMeetFilterCriteria` instead |
| `0x9b26b0` | FireworkMode::ShowInstructions | Shows the firework display mode's instructions in the identification bar (the game text `FireworkIdentbarInstructions`) |
| `0xaef810` | WaterJetDesigner::OpenFiles | Opens the file window for saved water jet displays (folder `WaterJets`) next to the water jet designer |
| `0xad6460` | LaserDesigner::OpenFiles | Opens the file window for saved laser shows (folder `Lasers`) next to the laser designer |
| `0x69a6e0` | Sound::GUIClick | Plays the interface sound `GUIClick` (a sound made with `0x47CEE0`, one of three variations picked with rand() % 3) |
| `0x69a830` | Sound::GUIBClick | Plays the interface sound `GUIBClick` the same way |
| `0x69a980` | Sound::GUIBRelease | Plays the interface sound `GUIBRelease` the same way |
| `0x69aad0` | Sound::GUIWarning | Plays the interface sound `GUIWarning` the same way |
| `0x607d00` | Options::ShowCantSave | Shows the game text `CantSaveOptionsTxt` (in a message window) when the options could not be saved |
| `0x9bc740` | ParticleDesigner::Title | The particle designer window's title, `Advanced Particle Designer` |
| `0x674180` | AdMenu::AddCustomImage | Adds the `Custom` image entry to the ad menu (AdMenu::BuildCustomImage) and relates it to the menu |
| `0xb4f7c0` | AnimalCamera::BuildWindow | Builds the animal camera's window area (`AnimalCamWindowArea`, RideCamera::BuildGUI) |
| `0x7573e0` | Entertainer::AddCostumeMenu | Adds the entertainer's `Costume` menu (Entertainer::BuildCostumeMenu) |
| `0x6019f0` | Window::SizeForLanguage | A window size that depends on the game's language (compared after `0x453630`): 500 wide in German, 460 otherwise, and 20 taller than the part at +0x184 |
| `0x444b10` | Language::ChooseEnglishOrOther | Runs `0x443A20` when the game's language is English and `0x443AE0` otherwise |
| `0x9d83d0` | FireworkDisplay::NewDefaults | Sets up a new firework display's details: its name from the game text `FireworkDisplayDefaultName`, an empty second text, 0x528 and 0xFFFF, and a reference that refers to nothing |
| `0xa5c090` | ProgramInfo::AddAwardsTab | Adds the awards tab to the program information window, with the tooltip game text `PID_TT_AwardsTabPopUp` |
| `0x9db1c0` | ProgramInfo::AddCampaignsTab | Adds the campaigns tab to the program information window, with the tooltip game text `PID_TT_CampaignsTabPopUp` |
| `0x6be770` | Money::FormatOrUnlimited | Writes an amount of money as text (Money::Format and its variants), or the game text `Cash_Unlimited` for a park with unlimited money |
| `0x6c8cd0` | LanguageList::Build | In outline: fills the list of languages (with `Norwegian` among them) in a list window, its count and selection, the label and the layout |
| `0x6a6ac0` | ButtonBar::Build | In outline: builds a row of buttons named `Button%d`, each with its image (Button::SetSingleImage), placed and related to the bar |
| `0x5027d0` | CustomButtons::Build | In outline: builds the `CustomButton` buttons of a window, numbering them, with their images and related values |
| `0x4f8100` | TrackDesigner::BuildBrakeSpeed | In outline: rebuilds the track designer's special pieces panel, with the brake speed tooltip `TrackDesigner_BrakeSpeed_TT` |
| `0xa46870` | TextSizeWindow::TextSizeWindow | In outline: a window part with the `LargeText` and `SmallText` choices (its name `0x13428F4` by default) |
| `0x4b8760` | Window::LayoutForLanguage | In outline: lays a window out differently for `Korean`, `Dutch` and `BrazilianPortuguese` (wider texts) |
| `0xa2ede0` | IconNameWindow::IconNameWindow | In outline: a window part showing an `Icon` and a `Name` |
| `0x6a4d20` | SpinBox::SpinBox | In outline: a number field with `IncButton` and `DecButton` buttons and their images |
| `0x610e30` | Window::BuildForGerman | In outline: builds a window, laid out differently when the language is `German` |
| `0x67faf0` | BorderedWindow::Build | In outline: builds a window with the `GUI2SolidGreyBorder` border and its buttons |
| `0x69f710` | ListBox::ListBox | In outline: a list box window with its `ListBoxButton` buttons |
| `0x9adc90` | ResearchTree::BuildRoot | In outline: builds the root of the research tree view (`SpecialTreeRoot`) and its numbered items |
| `0x9ae410` | ResearchTree::Item | In outline: an item of the research tree view with its icon (`GUIIcon:txs`) |
| `0x433120` | Swatch::DrawBackground | In outline: draws a colour swatch's background (`GUI2Swatch_Background`) through the alpha batches |
| `0x7519b0` | Swatch::DrawTick | In outline: draws a colour swatch's tick (`GUI2Swatch_Tick`) |
| `0x465180` | CustomContent::OpenMovie | In outline: opens a movie from the `CustomContent` folder (MovieDecoder::Open) |
| `0xc66dd0` | ScrollBar::Build | In outline: builds a scroll bar with its `scroll filler` part |
| `0x608130` | Window::BuildForGerman2 | In outline: builds another window laid out differently when the language is `German` |
| `0xa4b1a0` | Window::LayoutForEnglish | In outline: lays a window out for English or for Asian languages (Language::IsAsian), setting its texts |
| `0xa36c90` | KeyBindingWindow::Build | In outline: builds the key binding part of the options window, with its click sound (`Plip`) and key combinations |
| `0x6787b0` | GUIIcon::Load | In outline: loads a `GUIIcon` texture (TextureResource) by name |
| `0x836530` | StyleWindow::Build | In outline: builds the style chooser (`StyleName`) for scenery, reacting to held keys |
| `0x6a98f0` | Swatch::Build | In outline: builds a colour swatch window with its tick (`GUI2Swatch_Tick`) |
| `0x6452f0` | FlexiColourWindow::Build | In outline: builds the flexi-colour controls (`Flexicolour_%d`) |
| `0x623eb0` | LandDialog::RadiusButtons | In outline: builds the land tool's radius buttons (`Land_RadiusButton`) |
| `0x448d40` | RideCamera::BuildWindow | In outline: builds the ride camera's window area (`RideCamWindowArea`, RideCamera::BuildGUI) |
| `0x687ab0` | FileWindow::DirectoryName | In outline: the name text of a file window's folder (`DirectoryName_` + its name) |
| `0x68be70` | DatDescWindow::Build | In outline: builds the description window of a data file (`GUI2_DatDesc`), reporting it when it cannot be loaded |
| `0xaff7d0` | WildAnimalUnlock::Build | In outline: builds the wild animal unlock window with its check box and the game text `WildAnimalUnlockText` |
| `0xa25af0` | TutorialWindow::Build | In outline: builds a tutorial window with its `Tutorial Name` |
| `0x4b8e10` | AttractionCustomersDialog::Labels | In outline: builds the attraction customers dialog's labels (`AttractionCustomersDialogLabel_` + name), with the attraction's rates |
| `0x4b90e0` | AttractionCustomersDialog::Labels2 | In outline: more labels of that dialog (`AttractionCustomersDialogLabel_` + name) |
| `0x719710` | Tooltip::Make | In outline: makes a tooltip text, or `invalid tooltip` when there is none |
| `0xb70cb0` | DartCam::Build | In outline: builds the dart camera's top button (`DartCam_TopButton`) |
| `0x61ffe0` | MessageAlert::Show | In outline: shows a message alert (`Message alert`) about an object, with its sound |
| `0xa2eb70` | MultiSelectSwatch::Build | In outline: builds the stall items' colour choice, with the tooltip `MultiSelectSwatch_SelectAvailableColsTT` and each item's profit |
| `0x65eea0` | PaintTool::BuildViewport | In outline: builds the paint tool's viewport (`GUI2_PaintTool_Viewport`) |
| `0xa82a40` | PoolInfo::Labels | In outline: the pool information labels (`PoolInfoLabel_` + name) |
| `0x66c7d0` | AttractionResultsDialog::Labels | In outline: the attraction results dialog's labels (`AttractionResultsDialogLabel_` + name) |
| `0x55d300` | AttractionResultsDialog::Labels2 | In outline: more labels of that dialog (`AttractionResultsDialogLabel_` + name) |
| `0x660ad0` | AvatarEditor::CrownIcon | In outline: the avatar editor's crown icon (`AvatarEditor_CrownIcon`) |
| `0x677f30` | RideSelection::GroupItem | In outline: a ride selection item for a group (`Group_Containing_` + name) |
| `0x677d40` | RideSelection::GroupItem2 | In outline: another ride selection group item (`Group_Containing_` + name) |
| `0x4052a0` | DemoText::Item | In outline: a demo text item (`DemoOut_` + number) |
| `0xa41620` | Scenario::NameText | In outline: a scenario's name text (`Scenario` + its name) |
| `0x4d7230` | RideCamera::BuildWindow2 | In outline: builds the ride camera's window area (`RideCamWindowArea`) for this window |
| `0xaac350` | PoolPlacementGrid::Build | In outline: the pool placement grid controls (`PoolPlacementGrid`) |
| `0x746f50` | PersonEditor::UnderlineColour | In outline: the person editor's underline colour icon (`PersonEditor_UnderlineColourIcon`) |
| `0xaca850` | ColourSelection::Block | In outline: a block of the colour selection (`ColourSelection_Block`) with its skin texture |
| `0x597fb0` | AttractionCustomersDialog::SoldLabel | In outline: the customers dialog's `AttractionCustomersDialogLabel_Sold` label |
| `0x6bd2a0` | Money::Label | In outline: a money label (`Money`) |
| `0xa4b8f0` | Window::LayoutForEnglish2 | In outline: lays a window out for English or for Asian languages (Language::IsAsian) |
| `0x70ea00` | LandDialog::RadiusWindow | In outline: builds the land tool's `Radius` window (LandDialog::BuildRadius) |
| `0x8368d0` | StyleWindow::Build2 | In outline: another style chooser (`StyleName`) |
| `0x86f3e0` | ObjectGroup::Name | In outline: an object group's name (`Obj_Group`) |
| `0x4f0a70` | TrackDesigner::StateText | In outline: a state text (`State` + number) |
| `0x8dc760` | HelpButton::Build | In outline: builds a `Help` button with its images and text |
| `0xc6c280` | Desktop::Desktop | In outline: the `Desktop` window |
| `0xa1e6a0` | Ride::SpecialName | In outline: the `Special` name text |
| `0xbb5470` | AttractionDetails::ButtonFlagName | In outline: a details button flag's name (`ButtonFlag` + name, AttractionDetails::ButtonFlags) |
| `0x6bdf10` | Money::ShowNoMoney | Shows the game text `NoMoney` (in a message window) when the park cannot afford something |
| `0x5a49a0` | TrackedRide::ShowCannotEdit | Shows the game text `CannotEditTrackedRideWhileOpen` (in a message window) |
| `0x61b7a0` | HelpButton::Build2 | In outline: builds a `Help` button with its image and text |
| `0x993830` | FireworkDisplay::WatchMessage | In outline: a message window with the game text `FireworkDisplay_WatchDisplay` |
| `0x46d8b0` | PassportPost::SetupPanel | In outline: opens the passport post's setup panel (`Passport_SetupPanel`) |
| `0x819ab0` | LitterBin::StatisticsPanel | In outline: opens the litter bin's statistics panel (`Bin_Statistics`) |
| `0x748910` | AvatarEditor::Title | The avatar editor's title (the game text `AvatarEditor_Title`) |
| `0x6ed440` | Objectives::Button | In outline: the objectives button with the game text `Man_Objectives` |
| `0x8256d0` | RideEvent::Panel | In outline: the ride events panel with the game text `Obj_Scenery_RideEvents` |
| `0x622b70` | ProgramInfo::CreditsHelp | Passes slot 2 to every child, then sets the window's text to the game text `ProgramInfoDialog_CreditsPanelTabHelp` |
| `0x621b50` | ProgramInfo::InfoHelp | The same with `ProgramInfoDialog_ProgramInfoTabHelp` |
| `0xb70110` | WildAnimalCam::NextRideText | The game text `WAM_NextRide` |
| `0xb700e0` | WildAnimalCam::PrevRideText | The game text `WAM_PrevRide` |
| `0xb700b0` | WildAnimalCam::NextCamText | The game text `WAM_NextCam` |
| `0xb70080` | WildAnimalCam::PrevCamText | The game text `WAM_PrevCam` |
| `0x496ba0` | Font::System | Looks up the `SystemFont` text resource |
| `0xc661d0` | Font::Arial | Makes the `Arial` font at the size the GUI settings give |
| `0x68fc50` | OptionList::SetValue | Sets the value of option n (8 bytes each at +0x190), asserting `_index < m_numOptions` (`out of range`) |
| `0x68fea0` | OptionList::Set2 | Another option setter with the same `out of range` assertion |
| `0x6902d0` | OptionList::SetText | Sets option n's text, with the `out of range` assertion |
| `0x68fd60` | OptionList::SetImage | Sets option n's image (Button::SetSingleImage), with the `out of range` assertion |
| `0x68ff90` | OptionList::Set3 | Another option setter with the `out of range` assertion |
| `0x68fdb0` | OptionList::SetImage2 | Another option image setter with the `out of range` assertion |
| `0x68fd10` | OptionList::Set4 | Another option setter with the `out of range` assertion |
| `0x68fcc0` | OptionList::Set5 | Another option setter (`0x67B3C0`) with the `out of range` assertion |
| `0x672440` | Rota::Check | In outline: compares names with `Rota` |
| `0x672380` | Frontier::Name | The `Frontier` name text |
| `0x5fe280` | Window::SizeForLanguage2 | A window size that depends on the game's language: 500 wide in German, 460 otherwise, the height from the part at +0x154 (like Window::SizeForLanguage) |
| `0x5fd4b0` | Window::SizeForLanguage3 | The same, the height from the part at +0xE0 |
| `0x607570` | Window::SizeForLanguage4 | The same language-dependent width (500 in German, 460 otherwise) |
| `0x605880` | Window::SizeForLanguage5 | The same language-dependent width (500 in German, 460 otherwise) |
| `0xb6f920` | WildAnimalList::FixForLanguage | When the game's language is not English, goes over the list's entries (+0x54, count +0x60) and changes the marked ones (`0xB3E390`) |
| `0x6a8220` | NumberField::Update | Keeps the value a number field is bound to (+0xF4) inside its limits, by its kind (+0xF0): 1 a float between +0xF8 and +0xFC, 2 a whole number between +0xF8 and +0xFC, 3 a byte between +0xF8 and +0xF9; then redraws the field (`0x6A7DF0`) |
| `0x6acb30` | Screen::AreaScale | The width and height of the GUI's area (`0x12F83F0`) divided by the screen's, or 1 and 1 when there is no area |
| `0x49cac0` | Units::Convert | Converts a value for showing by its kind: 0 and 1 unchanged, 2 through `0x49CA10`, 3 and 4 by the unit setting (`0x10EE248`), 5 times `0x12F4AF4`, anything else 0 |
| `0x749120` | AvatarEditor::Tick | Advances two looping timers of the avatar editor: one wrapping at +0xD10 and, when not paused (+0xD6C), one wrapping at 1 |
| `0x61c280` | Tabs::Animate | Moves the tab offsets (+0xE4, count +0xE8) by the time: the selected tab (+0xE0) comes forward, the others go back to at most 0 |
| `0x757eb0` | Blink::Due | Whether a timer (+0xBC) has passed its period (from the table `0xFC35D8` by +0xB8); when it has, takes off one period, or all whole periods when asked |
| `0x502520` | Cursor::MoveByScreen | Moves a position (+0x20, +0x24) by a screen movement, scaled by +0x30 and the GUI area's size |
| `0x758880` | Filter::Accepts | Whether an item passes a filter by its kind (+0x120) and option (+0x121): the bits tested in the item's flags (+0x2C) are 1 or 2, 4 or 8, 0x10 or 0x20, or any |
| `0x63a920` | Window::OutsideEllipse | Whether a point lies outside the ellipse filling the window's area, shrunk by its margin (+0xD4) |
| `0x9f6490` | List::FindSelected | Looks through a list (+0xE4) for the entry with the id given, keeps its value in the owner (+0xDC, +0x15C) and clears +0x160 |
| `0x884890` | Columns::Hit | Whether a point falls in column n of a list (column positions at +0xFC), scaled by the list's width |
| `0x58fd30` | Style::ColourFor | One of six colours of a style (+0x1CC) for a state (0 to 2) and whether it is highlighted; 5 for any other state |
| `0x6adc50` | Window::FirstFocusable | The first child of the window's lists (+0x64, then +0x4C) whose flags (+0x94) have bit 4 set, or null |
| `0x619890` | Window::PartFor | One of ten parts of a window by number (0 to 9), or null |
| `0xca8560` | UIntSlider::SetValue | Sets the bound whole number (+4, unsigned) from a fraction between the minimum (+8) and maximum (+0xC) |
| `0x621370` | ProgramInfo::MinSize | The program info window's size, at least 530 by 200 |
| `0x682b30` | Drag::Move | Keeps the last drag position (+0xF0) and sets the new one (+0xF8) relative to the start (+0xE0), then tells the owner (+0xDC) |
| `0x483f40` | Screen::ToUnit | Turns a screen position into -1 to 1 across the GUI area |
| `0xca7dd0` | FloatSlider::SetValue8 | Sets the bound decimal number (+4, its +8) from a fraction between the minimum (+8) and maximum (+0xC) |
| `0xca7c00` | FloatSlider::SetValue4 | The same, into the bound object's +4 |
| `0xca7a30` | FloatSlider::SetValue | The same, into the bound value itself |
| `0xc637c0` | Items::SizeAt | The width or height (+0xD0 or +0xD4, by the flag at +0x1C) of item n of the list (+0xB0), or 0 |
| `0xca83d0` | UShortSlider::SetValue | Sets the bound 16-bit number (unsigned) from a fraction between the minimum (+8) and maximum (+0xA) |
| `0xca82c0` | ShortSlider::SetValue | Sets the bound 16-bit number (signed) from a fraction between the minimum (+8) and maximum (+0xA) |
| `0x5fcb10` | Toggles::ClearIfNone | Clears the shared flag (+0x13C) of the window's bit unless one of its options (+0x148, count +0x150) still has its bit set |
| `0x682cd0` | Drag::Start | Marks a drag started (+0x258) and keeps the pointer's place relative to the GUI root (`0x12F83F0`, +0xDC), then updates (`0x6B0990`) |
| `0x61dca0` | List::ScrollToRow | Sets the first shown row (+0xE0) from a position, by the row height, and tells the scroll bar (+0x3C) |
| `0x6b60a0` | Blink::Advance | Moves a looping time (+0x6010) on and keeps it below 1 |
| `0xca81b0` | ByteSlider::SetValue | Sets the bound byte (unsigned) from a fraction between the minimum (+8) and maximum (+9) |
| `0xca80a0` | CharSlider::SetValue | Sets the bound byte (signed) from a fraction between the minimum (+8) and maximum (+9) |
| `0x436d70` | Bar::Fill | How full a bar is: its value (+0x2E8) over the maximum `0x10EDA7C`, kept between 0 and 1 |
| `0x682910` | Drag::Offset | The drag's offset (+0x25C) while dragging (+0x258), otherwise 0 |
| `0xc5e3c0` | ScrollBar::SetPageSize | Sets a scroll bar's page size (+0xDC, at most 1) and keeps the position (+0xE0) from going past the end |
| `0x99d300` | Graph::PointFor | A point of a graph from a value pair: x across from its scale and offset (+0xDC), y up or down the height |
