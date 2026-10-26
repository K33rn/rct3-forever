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
| `0x65f830` | ParkDialog::ShowPage | Switches the park dialog to page n (current at +0x15C): builds that page (ParkDialog::Build, the guest wants page, `0x70A6F0`, the landscape and wild animal settings windows and others), titles it from the table `0x10EEC70` and lays it out |
| `0x70a6f0` | ParkDialog::BuildFlagsPage | In outline: one page of the park dialog: a list and a column of check boxes, each bound to a bit (1, 4, 0x10, ...) of the park's flags (+0x5C of Park::Get) and labelled with a game text, and a weather list set to the current weather (Weather::Get) |
| `0x6ce2c0` | Window::Build6CE2C0 | In outline: lays out a window with a skinned background, a row of parts (+0xE4) sized to fit, and number texts made with String::FromNumber |
| `0x693460` | Layout::Place693460 | In outline: places a box among its neighbours (`0x690EE0`) along one axis by its flag (bit 0x4000), moving its edges (the Set_ helpers) until they fit, and refreshes them (`0x6931F0`, `0x417C20`) |
| `0x583360` | PlacementCursor::Update583360 | In outline: updates a placement tool under the mouse: when the GUI is not in the way, finds the ground point from the camera (MainCamera::Get, `0xCD79C0`), snaps its height to terrain levels (Terrain::HeightLevel) and follows the mouse as it moves |
| `0xa47d30` | Graph::LayoutA47D30 | In outline: lays out a graph window: its axes and labels (`0xA45310` sized 400 by 100, `0xA45570`, `0xA451D0`), five marks, its lines (+0x22C) and bars (+0x240), each set to its value (Window::SetValueEC) |
| `0x6f8360` | Window::Layout6F8360 | Places each of the window's parts (+0x12C to past +0x1A0) at fixed positions and sizes worked out from the window's width and height (its rectangle at +0x7C), with Window::UpdateRect |
| `0xa1fdb0` | Window::LayoutA1FDB0 | Places the window's parts at positions and sizes worked out from its size (Window::UpdateRect for each) |
| `0x7f1fc0` | ListWindow::Build7F1FC0 | In outline: builds a window with a titled list box (ListBox::ListBox, ListWindow::SetCount) filled from the items given |
| `0x5fa250` | NumberSpinner::Update5FA250 | In outline: a number control's update by its mode (+0xF0): steps its value by the time held (`0x6A7CB0`, `0x6A7C70`), kept within its limits (with money checked against -100000 and the park's cash) |
| `0xa23fd0` | Window::BuildA23FD0 | In outline: builds a window from a shared title text (`0x134271C`): a 0x220-byte part (`0x6846F0`), a 0x13C-byte part (`0x67BE80`) and others |
| `0x6c2500` | Gauge::Draw6C2500 | In outline: draws a gauge or meter when shown (+0x101): its colours (Colour::ToARGB, a grey-green and a red), its scale (Node::TotalScale) and its marks (`0x6C2260`) up to its value |
| `0x9b7940` | Window::Build9B7940 | In outline: builds a window from a title text: a part from the shared text `0x13049E4` and others (`0x6A8D00`) |
| `0x64c950` | VIPDetails::Build64C950 | In outline: builds a VIP's details window with a shared title (`0x12F8000`) and its stats (VIPDetails::BuildStats) |
| `0x650180` | GuestDetails::Build650180 | In outline: builds a guest's details page from its title and the guest's thoughts (GuestDetails::BuildThoughts) |
| `0x64f8d0` | GuestDetails::Build64F8D0 | In outline: another guest details page built like GuestDetails::Build650180, from its title and the guest's thoughts (GuestDetails::BuildThoughts) |
| `0x9b70c0` | Window::Build9B70C0 | In outline: builds a window like Window::Build9B7940, from a title and the shared texts `0x13049E4` and `0x1302A9C` |
| `0x691d80` | Layout::Snap691D80 | In outline: snaps a box's position to whole pixels and checks it against the boxes given (overlap tests), giving 1 when it fits |
| `0x6bbae0` | Window::Build6BBAE0 | In outline: builds a window from a shared title (`0x1305DD0`) and a list (Listener440) |
| `0x9dc0d0` | Window::Build9DC0D0 | In outline: builds a window from a title with a list box (`0x65A2D0`, `0x6AB2A0`, ListBox::ListBox) |
| `0x9d8740` | DatWindow::Open9D8740 | In outline: when not already open (+0x9C), makes a 0x70-byte GUI description (GUI2_DatDesc) for the object (+0x98) and opens it (`0x9D7430`) |
| `0x9c3b50` | Window::Build9C3B50 | In outline: builds a window from a shared title (`0x13414A0`, `0x67DAC0`) and a part with the shared text `0x12FEF8C` (`0x6AB4E0`, 0x210 wide) |
| `0xa31430` | Label::FormatValueA31430 | In outline: makes a value's text by its kind (+0x54: 1, ...): the number scaled by its unit (+0x58 +0x10, `0xCD2520`, `0x442120`) |
| `0xa236b0` | Window::BuildA236B0 | In outline: builds a window like Window::BuildA23FD0 from the shared title `0x1342724` |
| `0x9b6980` | ListWindow::Fill9B6980 | In outline: fills a list window with up to 8 rows (ListWindow::SetCount), each with its text, and lays them out (Window::UpdateRect) |
| `0x48ee80` | Marker::Update48EE80 | In outline: an on-screen marker's update by its state (+0xC): places it on the GUI root (Window::SetRelationAndLayout with `0x12F83F0`) over its target (`0x48EBC0`) |
| `0x9ba570` | Window::Build9BA570 | In outline: builds a window from the shared title `0x134145C` with a part of style 0x5010 (`0x6A8D00`) |
| `0xa41c40` | ScenarioList::FillA41C40 | In outline: fills a list with the scenarios (+0x244): each row numbered `%02d` in place of `%NUMBER%` in its text, and its title (Scenario::TitleText) |
| `0x837060` | Window::Build837060 | In outline: builds a window from the shared title `0x130B624` (`0x67BE80` with 4) and a list (Listener440) |
| `0x6a1120` | Window::HandleKey6A1120 | In outline: handles a window message by its number: 0x9000 (`0x443B70`), above 0x5000 (`0x444F10`), and passes others on (`0x442C60`) |
| `0x9bb220` | Window::Build9BB220 | In outline: builds a window from the shared title `0x1341450` |
| `0x9b9eb0` | Window::Build9B9EB0 | In outline: builds a window from its title like Window::Build9B7940 |
| `0x7a86f0` | Toolbar::Build7A86F0 | In outline: builds a row of image buttons (Button::SetImages with five images each) with their texts (TextRef::Assign: `0x12FDF08`, `0x12FAFE0`, `0x12FD5A0`, ...) |
| `0x67dfd0` | Window::Build67DFD0 | In outline: builds a window from the shared title `0x12F83C8` (`0x67DAC0`) |
| `0x4a3820` | Info::Text4A3820 | In outline: makes an info text by its kind (+0xBC): from a named game text (NamedTextRefF30778) and the values of the thing (`0x496610`) |
| `0x6ac080` | Window::Draw6AC080 | In outline: draws a window: when shown (+0xD4), renders its background (`0x694E90`, GUIRenderer::Render) and works out its screen corners (Window::ScreenPosition), rounded |
| `0x6957f0` | Layout::Apply6957F0 | In outline: applies a layout by its kind: one box, or several (at +0, +0x18, +0xA8 and the last) laid out with `0x6950B0` |
| `0x88b470` | List::Draw88B470 | In outline: draws a list of parts (+0xE0) at its scale (Node::TotalScale), each with its background (`0x694E90`) |
| `0x6cd960` | Window::Build6CD960 | In outline: builds a window from the shared title `0x1305E48` |
| `0x5c6610` | TrackedRide::Height5C6610 | In outline: a ride piece's height at a point (`0x5C14D0`), or, when below 0 and not flagged (+0x170), from the middle of its span (Span::Middle) |
| `0xacfdb0` | Graph::DrawACFDB0 | In outline: draws a graph window's contents when not flagged (+0x108): its frame (`0x6ACE50`) and its lines (`0x695E80`) |
| `0x663680` | Window::Build663680 | In outline: builds a window for a thing (`0x4E4410`, `0x8DC9B0`) with two parts of style 0x9165 (`0x6A2620`) and opens it (MainWindow::Build) |
| `0x7491e0` | AvatarEditor::Draw7491E0 | In outline: draws the avatar editor's figure turned by its angles (Matrix::TurnColumnsAboutY, Shape::TurnAboutX), with its parts (`0x7BED80`, `0x7BEDB0`) when not flagged (+0xE49) |
| `0x6084f0` | Window::Fill6084F0 | In outline: fills a window's list from a list of things (`0x1305D54`), finding each by name (stricmp) and skipping hidden ones unless the developers' view is on (`0x1103B9C`) |
| `0x9ddc20` | Window::Build9DDC20 | In outline: builds a window from its title like Window::Build9B7940 |
| `0x7a4380` | Window::Layout7A4380 | In outline: places a window's parts (Window::UpdateRect) and, with items (+0x17C), lays out each of its rows |
| `0x5f4060` | Window::Layout5F4060 | In outline: lays out a window by the screen (`0x13D0DB8`, narrower than 700 handled) and places it (Window::SetRelation) |
| `0x6a9ef0` | Window::Draw6A9EF0 | In outline: draws a window's frame (`0x6ACE50`) and its title text, cut to fit its width (`0x6AE070`) |
| `0xa5c260` | Texts::LookupA5C260 | In outline: gives one of a set of game texts by number (each checked loaded, `0x12FDDC0`, `0x12FA3FC`, `0x1305B88`, `0x12FADB8`, ...) |
| `0x9bac90` | Window::Build9BAC90 | In outline: builds a window from the shared title `0x1341448` |
| `0x99b6a0` | Window::Layout99B6A0 | In outline: lays out a window from a list (`0x1103C58` +0x50, up to 0x400 entries, `0x6A4AB0`) |
| `0x8854b0` | List::Scroll8854B0 | In outline: keeps a list's shown rows (+0x108 +0xF4, +0x110 +0xF8) in range and finds the row under a point (+0xE0 rows) |
| `0x5fcce0` | KeyBinding::Read5FCCE0 | In outline: reads a key combination (KeyCombination::Set) from the window's keys when flagged (+0x94 bit 4): each key of kind 0x2000000 by its code (0x200001D, ...) |
| `0x4bcf80` | PlacementCursor::Update4BCF80 | In outline: another placement cursor update like PlacementCursor::Update583360, skipped when the GUI is in the way (`0x12F8404` +0x258) or flagged (+0x50, +0x94, +0x98, +0xDC, +0x120, +0x168) |
| `0x9c2f50` | Window::Build9C2F50 | In outline: builds a window from its title (copied when none is given) like the other window builders |
| `0x9b4bc0` | Dial::Layout9B4BC0 | In outline: lays out ten parts round a circle (cosine and sine of their angles) and sets each one's value (`0x1B0` +0x18) |
| `0xa48ca0` | Graph::LabelsA48CA0 | In outline: lays out a graph's labels from shared texts (`0x12FE4F8`, `0x12FCB28`, `0x12F9FD4`) at fixed positions (`0xA455D0`, `0xA45310`: 663, 32, 250, ...) |
| `0x7e22e0` | Info::Text7E22E0 | In outline: makes an info text for a thing (+0x114) from its values (`0x758C30`, `0x7739D0`, `0x4FF4A0`) |
| `0xa56cc0` | Window::BuildA56CC0 | In outline: builds a window from the shared title `0x1342968` |
| `0x6ed990` | Objectives::Draw6ED990 | In outline: draws the objectives list (+0x108): finds each objective (Objectives::Find) and, when not done (+0x55), sets its image (SkinItem::Texture) |
| `0x45c780` | Window::Layout45C780 | In outline: lays out a window's parts by the terrain size (Window::UpdateRect) |
| `0x9abd00` | Window::Build9ABD00 | In outline: builds a window from the shared title `0x1341420` |
| `0xc68870` | Text::FormatC68870 | In outline: makes a text sized by the GUI settings (`0x13E6D00` +0x19C) into a new buffer |
| `0x809be0` | Game::ShowStart809BE0 | In outline: shows a start screen: `0x617E40` with `0x12F575C`, `0x9B0890`, an empty text faded in over 0.1 (`0x617EB0`) and `0x733270` |
| `0x672df0` | Window::Layout672DF0 | In outline: when not flagged (+0xF2), sets a part's value (Set_+0xd0) and lays the window out (Window::Layout) |
| `0x6bef60` | Money::Text6BEF60 | In outline: makes a money text (Money::Format_(2)) and trims it when the amount is a whole number of units (n % 100 = 0) |
| `0x467cb0` | Window::Build467CB0 | In outline: builds a window from the shared title `0x12EFFF8` |
| `0x6edeb0` | Objectives::Clear6EDEB0 | In outline: releases the objectives list's parts (+0x104, count +0x108) and rebuilds it (`0x6E9D70`) |
| `0x6cca80` | List::DrawRows6CCA80 | In outline: draws a list's rows (+0xE4): each row's item (`0x6C8180`) and, when not done, its image (SkinItem::Texture) |
| `0x68e3c0` | Bar::Layout68E3C0 | In outline: lays out a bar by its fill (above 1 handled), sets its value (Set_+0xd0), draws its frame (`0x6ACE50`) and its marks (+0x104) |
| `0x9e6d60` | Keys::Camera9E6D60 | In outline: when not flagged (+0x408), checks keys 0x2F and 0x30 (KeyState::Held) to move the camera |
| `0x64dc40` | Window::Build64DC40 | In outline: builds a window from the shared title `0x12F8040` |
| `0x643030` | Window::Layout643030 | In outline: when not flagged (+0x152), places a window (Window::SetRectAndRefresh, Window::UpdateRect) and finds its selected entry (+0x124) |
| `0x79f040` | Info::Text79F040 | In outline: makes an info text for the selected thing (Interface44::Get, `0x4C0850`) |
| `0x438140` | Camera::Focus438140 | In outline: focuses the camera (MainCamera::Get, `0xCD7BB0`, `0xCD6850`) on a target (+0x308) over the terrain |
| `0xa95240` | Window::LayoutA95240 | In outline: places a window's parts (`0x63AAB0`, Window::UpdateRect) |
| `0x59fac0` | Placement::Wait59FAC0 | In outline: waits for a placement plan for the selected thing (`0x12F5608`) inside a timed loop (Timer::Milliseconds) |
| `0x6621b0` | Window::SetShown6621B0 | In outline: shows or hides a window part (+0x64) when it changes, refreshing its links when not flagged (+0x44) |
| `0x97fa40` | Window::Build97FA40 | In outline: builds a window from the shared title `0x1341290` |
| `0x74e710` | AvatarEditor::Pick74E710 | In outline: sets up the avatar editor's five parts (`0x746390`) and the animation name text (AvatarEditor::AnimationName) |
| `0x9b5a00` | Dial::Build9B5A00 | In outline: builds a dial window's parts (Window::UpdateRect, Window::SetRelation) and sets its value (`0x9C8E10`) |
| `0x6ccf70` | List::Clear6CCF70 | In outline: releases a list's parts (+0xE0, count +0xE4) |
| `0x473320` | Camera::Select473320 | In outline: when the thing given is the selected one (+0x64) in mode 1 and not flagged (+0x88), moves the camera to it (`0x130654C`, MainCamera::Get) |
| `0x7187a0` | Window::Build7187A0 | In outline: builds a window from the shared title `0x130623C` |
| `0x99e8e0` | Fireworks::List99E8E0 | In outline: lays out the firework display list from its parts (Get_+0x108, Table104::Entry) |
| `0x9b5eb0` | Dial::Layout9B5EB0 | In outline: another dial layout like Dial::Layout9B4BC0, up to the dial's count (+0x140) |
| `0x9da640` | Text::Join9DA640 | In outline: joins texts (`0x9DA5E0`) into one |
| `0x9db4c0` | Window::Layout9DB4C0 | In outline: places a window's parts (Window::SetRelationAndLayout) by its mode (+0x130, 2, ...) |
| `0x6942a0` | Layout::Fit6942A0 | In outline: fits a box by its flags (bit 0x4000), checking the shared tables (`0x12F85FC`, `0x12F84D8`, `0x691830`) |
| `0x68d840` | Window::Close68D840 | In outline: closes a window when allowed (+0xE4, `0x10EDBB0`), the plain way (`0x68D750`) or by making the next one (`0x1305BE4`) |
| `0x747fe0` | AvatarEditor::Layout747FE0 | In outline: lays out the avatar editor's parts (`0xA455D0`, `0xA45310` with 300 by 250, `0xA457A0`, `0xA45700`) |
| `0x63eed0` | Options::ResolutionText | In outline: makes the text of a screen size, `%d x %d` from its width and height (+0x48, +0x4C), for mode 1 |
| `0x81f5c0` | SceneryBehaviour::Text81F5C0 | In outline: makes a scenery behaviour's text by its owner's case (SceneryBehaviour::Owner) |
| `0xa3aa10` | Window::RefreshA3AA10 | In outline: when flagged (+0x189), refreshes a window (`0x673AC0`) from the install folder text (`0x1106DC4`) |
| `0xa5e8a0` | Window::BuildA5E8A0 | In outline: builds a window from the shared title `0x134299C` |
| `0xae2830` | Window::DrawAE2830 | In outline: draws a window's frame (`0x6ACE50`) and, when it has height, its background (GUIRenderer, `0x694E90`), with a case for narrow ones |
| `0x449580` | GUI::Setup449580 | In outline: sets up a GUI renderer with a 40000-byte buffer and its matrices (`0xCD78A0`) |
| `0x69d5a0` | List::FillTexts69D5A0 | In outline: when it has no handler, fills a list's texts from a counted buffer (Memory::AllocCounted, WideString::FromWide) |
| `0x684c40` | Window::HandleEvent684C40 | In outline: handles a window event by its number (0x10000, ...), timed (Timer::Milliseconds), with its parts (+0x128) |
| `0x63ada0` | Window::Layout63ADA0 | In outline: when not flagged (+0xD4), places a window (Window::UpdateRect) and its part (+0xDC) |
| `0xacef90` | Graph::DrawACEF90 | In outline: another graph draw like Graph::DrawACFDB0, when not flagged (+0x100) |
| `0x99a900` | Window::Build99A900 | In outline: builds a window from the shared title `0x13413A4` |
| `0x65fba0` | Window::Build65FBA0 | In outline: builds a window from the shared title `0x12F80E0` |
| `0x681810` | Window::Drag681810 | In outline: when a drag has moved far enough, moves a window to whole pixels and checks its children (Window::ForEachChild) |
| `0x654840` | Window::Build654840 | In outline: builds a window from the shared title `0x12F802C` |
| `0xb433a0` | Window::BuildB433A0 | In outline: builds a window from the shared title `0x13CE99C` |
| `0xba89a0` | Window::BuildBA89A0 | In outline: builds a window from the shared title `0x13D0618` |
| `0x65a2d0` | Window::Build65A2D0 | In outline: builds a window from the shared title `0x12F809C` |
| `0xae2100` | Graph::DrawAE2100 | In outline: draws a graph's frame (`0x6ACE50`), its lines (`0x694FB0`) and five marks (`0x695E80`) |
| `0x597760` | AttractionPerson::Link597760 | In outline: another attraction person link like AttractionPerson::Link587D00 |
| `0xa9ac20` | Window::BuildA9AC20 | In outline: builds a window from the shared title `0x13CDFCC` |
| `0x88b0a0` | List::Draw88B0A0 | In outline: another list draw like List::Draw88B470, at its scale (Node::TotalScale) with its background (`0x694E90`) |
| `0x789e20` | Window::Build789E20 | In outline: builds a window from the shared title `0x1306A74` |
| `0x9ac3b0` | Window::Title9AC3B0 | In outline: places a window and sets its title text, cut at a `:` when it has one (WideString::Find, Window::SetText114) |
| `0x683060` | Window::Build683060 | In outline: builds a window from the shared title `0x12F8410` |
| `0x484880` | Window::Layout484880 | In outline: lays out a window (`0x6A65B0`, Window::Layout) and refreshes its link |
| `0x9c0e10` | Window::Layout9C0E10 | In outline: lays out a window and refreshes its parts (`0x9BB8E0`, `0x9BB9F0`, `0x9BCE00`) |
| `0x5981e0` | Window::Build5981E0 | In outline: builds a window from the shared title `0x12F54B0` |
| `0x485750` | Window::Build485750 | In outline: builds a window from the shared title `0x12F4630` |
| `0x4d6dd0` | Window::Build4D6DD0 | In outline: builds a window from the shared title `0x12F4F08` |
| `0x7199a0` | Window::Build7199A0 | In outline: builds a window from the shared title `0x1306248` |
| `0x65b7f0` | Window::Build65B7F0 | In outline: builds a window from the shared title `0x12F80AC` |
| `0xa58310` | Window::BuildA58310 | In outline: builds a window (`0xA55E00`) and its parts |
| `0x666020` | List::Layout666020 | In outline: lays out a list's rows (+0x178) (`0x6ACBD0`) |
| `0x66fa40` | Window::Build66FA40 | In outline: builds a window from the shared title `0x12F8170` |
| `0xa08bc0` | Window::BuildA08BC0 | In outline: builds a window from the shared title `0x1342268` |
| `0x88a5f0` | Window::Build88A5F0 | In outline: builds a window from the shared title `0x1310FA8` |
| `0x692cf0` | Layout::Place692CF0 | In outline: places a box by its options (a size at most 0, a slot below 0, a count against `0x12F8610`) |
| `0x64e110` | Window::Build64E110 | In outline: builds a window from the shared title `0x12F8010` |
| `0x6392e0` | Text::AsianFont6392E0 | In outline: for Asian languages (Language::IsAsian), finds the font entry for a size (under 0x400) in a table |
| `0x692950` | Layout::Place692950 | In outline: another box placement like Layout::Place692CF0, growing the shared table when needed (`0x6918E0`) |
| `0xaea080` | Graph::DrawAEA080 | In outline: another graph draw like Graph::DrawACFDB0, when not flagged (+0x100) |
| `0xa297d0` | Info::TextA297D0 | In outline: makes an info text from a shared text (`0xF31F48`) for a thing |
| `0xa06fc0` | Window::FillA06FC0 | In outline: fills a window's list from its entries (`0x687DF0`, `0x685AC0`, `0x687720`) and their texts |
| `0x680160` | Window::Fill680160 | In outline: the same list fill (another copy) |
| `0x751db0` | AvatarEditor::Draw751DB0 | In outline: draws the avatar editor's view (GUIRenderer), growing a shared list by 0x10 (`0x13064EC`, `0x13064F8`) |
| `0x611c20` | Colours::For611C20 | In outline: a colour by number (up to 0x4000; above 0x80 a pale blue-green from Colour::ToRGB) |
| `0x660190` | Window::Build660190 | In outline: builds a window from the shared title `0x12F80D8` |
| `0xc6ba50` | ScrollBar::DrawC6BA50 | In outline: draws a scroll bar (ScrollBar::Draw) and its parts |
| `0x690580` | Window::Build690580 | In outline: makes a 0xD8-byte window part with the shared title `0x12F8498` |
| `0x667f90` | Window::Build667F90 | In outline: builds a window from the shared title `0x12F812C` |
| `0x48cbc0` | Label::Number48CBC0 | In outline: makes a number text (`%d`) for a value from 0 to 4 |
| `0xac4ef0` | Window::BuildAC4EF0 | In outline: builds a window from the shared title `0x13CE400` |
| `0x65b010` | Window::Build65B010 | In outline: builds a window from the shared title `0x12F80A4` |
| `0xac8ff0` | Window::DrawAC8FF0 | In outline: draws a window's frame (`0x6ACE50`), rounded |
| `0x7ef5e0` | Window::Build7EF5E0 | In outline: builds a window from the shared title `0x130A8CC` |
| `0x6ba6e0` | Window::Build6BA6E0 | In outline: builds a window from the shared title `0x1305DD4` |
| `0x7f2f00` | Window::Build7F2F00 | In outline: builds a window from the shared title `0x130AB60` |
| `0x443b70` | Window::Message443B70 | In outline: makes a window message's result by its kind (0, ...); the handler Window::HandleKey6A1120 uses for message 0x9000 |
| `0x4436b0` | Window::Message4436B0 | In outline: another message result maker by kind like Window::Message443B70 |
| `0x443340` | Window::Message443340 | In outline: another message result maker by kind like Window::Message443B70 |
| `0x442fd0` | Window::Message442FD0 | In outline: another message result maker by kind like Window::Message443B70 |
| `0x442c60` | Window::Message442C60 | In outline: another message result maker by kind; the one Window::HandleKey6A1120 passes other messages to |
| `0x48d240` | Window::Message48D240 | In outline: another message result maker by kind like Window::Message443B70 |
| `0xa367e0` | Window::LayoutA367E0 | In outline: places a window (Window::UpdateRect) and its rows (+0x13C) that pass a check (`0x981700`) |
| `0x6a0100` | Window::Fit6A0100 | In outline: fits a window's parts by their sizes |
| `0x9dbb10` | Window::Sizes9DBB10 | In outline: sets a window's sizes (Sizes::Pick) and its text |
| `0x6ff880` | Window::Build6FF880 | In outline: builds a window from its title (copied when none is given) |
| `0x83d730` | Window::Build83D730 | In outline: builds a window from the shared title `0x130B734` |
| `0x7ea7a0` | Window::Build7EA7A0 | In outline: builds a window from the shared title `0x130A88C` |
| `0x6cd410` | Window::Event6CD410 | In outline: handles event 0x14 for its part (+0x118) (`0x6C3870`) |
| `0x88aa10` | Window::Build88AA10 | In outline: builds a window from the shared title `0x1310FB8` |
| `0xbab740` | Window::BuildBAB740 | In outline: builds a window from the shared title `0x13D063C` |
| `0xc6af40` | Window::MakeC6AF40 | In outline: makes a window part (`0xC65F20`, `0xC69F90` with 0x84) |
| `0xc6b710` | Window::MakeC6B710 | In outline: makes a window part (`0xC66D30`) |
| `0x61e040` | Window::Build61E040 | In outline: builds a window from the shared title `0x12F57AC` |
| `0xa7cea0` | Window::LayoutA7CEA0 | In outline: lays out a window (`0x6A65B0`, Window::Layout, Window::SetRectAndRefresh) |
| `0xa364a0` | List::RowOKA364A0 | In outline: whether row n of a list is usable (more than six rows handled, within its count +0x108) |
| `0x6fc2a0` | Window::Build6FC2A0 | In outline: builds a window from the shared title `0x1306138` |
| `0x57ce30` | Window::Layout57CE30 | In outline: lays out a ride window when it has a ride (+0xF8) and it passes a check (`0x5A1A10`) |
| `0x48f640` | Marker::Select48F640 | In outline: a marker's update for its target (+0xD4, +0xDC) with the camera |
| `0x672520` | Window::Build672520 | In outline: builds a window from the shared title `0x12F81B8` |
| `0xa26da0` | Window::BuildA26DA0 | In outline: builds a window from the shared title `0x134273C` |
| `0x788b50` | Window::Build788B50 | In outline: builds a window from the shared title `0x1306A6C` |
| `0x6948f0` | Layout::Fit6948F0 | In outline: fits a box among others, growing the shared table (`0x12F8618`, `0x12F861C`, `0x693300`) |
| `0x997a30` | Window::Build997A30 | In outline: builds a window from the shared title `0x1341384` |
| `0x6aed50` | Window::Hit6AED50 | In outline: whether a point hits a window by the GUI root's and its own flags (bits 23, 7, 0x18) |
| `0xa5e120` | Awards::TextA5E120 | In outline: makes an awards text (Awards::Get, `0xA5B900`) when the count passes the shown one (+0x114) |
| `0x6b6ad0` | Window::Build6B6AD0 | In outline: builds a window from the shared title `0x1305D98` |
| `0x66a860` | Window::Layout66A860 | In outline: places a window's parts (Window::UpdateRect) |
| `0xb522e0` | Window::BuildB522E0 | In outline: builds a window from the shared title `0x13CEA4C` |
| `0x9ab8b0` | Fireworks::FindByName | In outline: finds an entry by name in the firework list (`0x1341408` +0x10, WideString::Compare) |
| `0x63f710` | Clock::Text63F710 | In outline: makes a time text from the game time (GameTime::Get +0x4C, scaled) |
| `0x663cf0` | Window::Update663CF0 | In outline: a window's update when not flagged (+0x2C) (`0x880F60`) |
| `0x6a07e0` | Window::Build6A07E0 | In outline: builds a window from its title |
| `0x994e60` | Window::Build994E60 | In outline: builds a window from the shared title `0x1341374` |
| `0x99e160` | Window::Build99E160 | In outline: builds a window from the shared title `0x13413B4` |
| `0x6a3e90` | List::Scroll6A3E90 | In outline: scrolls a list (`0x6A4AB0`) to keep its view (+0x158 to +0x160) in range |
| `0x608b40` | Window::Build608B40 | In outline: builds a window from the shared title `0x12F5724` |
| `0x6af7e0` | Window::BuildTitled | In outline: the window base that many window builders call first: builds a window from a title (by default the shared title `0x1305CFC`) |
| `0x43e790` | Text::Make43E790 | In outline: makes two texts and, when asked, joins them |
| `0x61d000` | Text::Name61D000 | In outline: makes a text with `%NAME%` replaced for each of a list's entries (+0x8C) |
| `0x6e3b40` | Text::Name6E3B40 | In outline: makes a name text (NameText_0x0) and copies it |
| `0x9dede0` | Text::Three9DEDE0 | In outline: makes three texts from shared texts (`0xF858B0`, `0xF858AC`, `0xF858A8`) |
| `0x6a43e0` | Window::Build6A43E0 | In outline: builds a window from the shared title `0x1305C68` |
| `0x681cc0` | Window::Build681CC0 | In outline: builds a window from the shared title `0x12F83FC` |
| `0x9aa8a0` | Window::Build9AA8A0 | In outline: builds a window from the shared title `0x134140C` |
| `0xa7ff10` | Window::BuildA7FF10 | In outline: builds a window from the shared title `0x13CDE98` |
| `0x42abc0` | Window::Build42ABC0 | In outline: builds a window from the shared title `0x1103BC8` |
| `0x666570` | Window::Part666570 | In outline: makes a 0x118-byte window part (`0x6663D0`) |
| `0x7513b0` | AvatarEditor::Sort7513B0 | In outline: goes over the avatar editor's shared list (`0x13064EC`) by its values |
| `0xaeaf80` | Graph::DrawAEAF80 | In outline: another graph draw when not flagged (+0x100) |
| `0x67f7f0` | List::Layout67F7F0 | In outline: places a list window and its rows (+0x16C) |
| `0x444f10` | Window::Message444F10 | In outline: another message result maker (rounded sizes); the one Window::HandleKey6A1120 uses above 0x5000 |
| `0x6e68c0` | Objectives::Text6E68C0 | In outline: an objective's text from its name (ReleaseAnimalsCondition::Name) |
| `0x6e2560` | Objectives::Text6E2560 | In outline: an objective's text from its name (NameText_0x0) |
| `0x6954f0` | Layout::Place6954F0 | In outline: another box placement, setting up when the shared value is unset (`0x10EEDB4` = 0xFFFF, `0x6930B0`) |
| `0x6afe60` | Window::Update6AFE60 | In outline: a window's update with the shared object (`0x1342708`) and a value (`0xA1D240`) |
| `0x599990` | Window::Build599990 | In outline: builds a window from the shared title `0x12F54BC` |
| `0x5996a0` | Window::Build5996A0 | In outline: builds a window from the shared title `0x12F54BC` (another one) |
| `0x599c80` | Window::Build599C80 | In outline: builds a window from the shared title `0x12F54BC` (another one) |
| `0x700b00` | Window::Build700B00 | In outline: builds a window from its title |
| `0x6c6900` | Window::Layout6C6900 | In outline: places a window's parts (Window::UpdateRect) |
| `0xaa5500` | Window::BuildAA5500 | In outline: builds a window from the shared title `0x13CE004` |
| `0x8e2300` | Window::Build8E2300 | In outline: builds a window from the shared title `0x1313C84` |
| `0x6e7e10` | Objectives::Text6E7E10 | In outline: an objective's text from its name (NameText_0x0) |
| `0x9c34b0` | Window::Build9C34B0 | In outline: builds a window from the shared title `0x134148C` |
| `0xc6eb40` | Window::MakeC6EB40 | In outline: makes a window part (`0xC65F20`) |
| `0xa55e40` | List::DrawA55E40 | In outline: places a list and its rows, each with the shared image `0x13049E4` (SkinItem::Texture) |
| `0x6e93e0` | Window::Build6E93E0 | In outline: builds a window from the shared title `0x13060B4` |
| `0x6cf110` | Window::Build6CF110 | In outline: builds a window from the shared title `0x1305E40` |
| `0x4842e0` | Window::Layout4842E0 | In outline: lays out a window (`0x6A65B0`, Window::Layout) when given one |
| `0x9b4420` | Window::Layout9B4420 | In outline: places a window's parts (Window::UpdateRect) |
| `0x62e0b0` | Window::Build62E0B0 | In outline: builds a window from the shared title `0x12F7E10` |
| `0x6b8580` | Window::Mode6B8580 | In outline: refreshes a window (`0x6B84F0`) with a case for mode 2 (+0xD4) |
| `0xb5aca0` | Window::BuildB5ACA0 | In outline: builds a window from the shared title `0x13CEAC8` |
| `0x814480` | Window::Build814480 | In outline: builds a window from the shared title `0x130B384` |
| `0x6aae60` | Window::Build6AAE60 | In outline: builds a window from the shared title `0x1305CD8` |
| `0xa84580` | Window::BuildA84580 | In outline: builds a window from the shared title `0x13CDEB8` |
| `0x925250` | Window::Build925250 | In outline: builds a window from the shared title `0x133A160` |
| `0x69e5a0` | List::Texts69E5A0 | In outline: makes a list's texts (+0x1C) from a shared text (`0xF5E1C4`) |
| `0x7cd6e0` | Info::Text7CD6E0 | In outline: makes an info text from two shared texts (`0xF31F48`, `0xF31FA8`) |
| `0x68c370` | Text::Join68C370 | In outline: joins a separator text (`0xF265E4`) and the text given |
| `0xc67210` | Window::RectC67210 | In outline: sets a window's rectangle (Rect::Assign) and its own part |
| `0x67d550` | List::Layout67D550 | In outline: lays out a list's rows |
| `0xa82220` | Money::TextA82220 | In outline: makes a money text, or the "unlimited" one (Money::FormatOrUnlimited) |
| `0x67bbb0` | Window::Build67BBB0 | In outline: builds a window from the shared title `0x12F8220` |
| `0x686720` | Window::Build686720 | In outline: builds a window from the shared title `0x12F8458` |
| `0x6a8f90` | Window::Fill6A8F90 | In outline: another list fill like Window::FillA06FC0 (`0x687DF0`, `0x685AC0`) |
| `0x5facf0` | Window::Build5FACF0 | In outline: builds a window from the shared title `0x12F5704` |
| `0x695220` | Layout::Place695220 | In outline: another box placement like Layout::Place6954F0 |
| `0x665af0` | Window::Build665AF0 | In outline: builds a window from the shared title `0x12F8120` |
| `0x750350` | AvatarEditor::Reset750350 | In outline: when flagged (+0xE49), resets the avatar editor's parts (`0x746390` with -1) |
| `0x685e30` | Window::Build685E30 | In outline: builds a window from the shared title `0x12F843C` |
| `0x608e50` | Window::Build608E50 | In outline: builds a window from the shared title `0x12F5720` |
| `0xa47190` | Graph::ValueA47190 | In outline: sets a graph part's value (Window::SetRelatedValue with 1.4615) |
| `0x641e40` | Window::Make641E40 | In outline: makes a window part (`0x6C21F0`, kind 0x200000C, 10) |
| `0x631740` | Window::Build631740 | In outline: builds a window from the shared title `0x12F7E44` |
| `0xa586c0` | Window::BuildA586C0 | In outline: builds a window from the shared title `0x1342960` |
| `0x6d1260` | Label::Float6D1260 | In outline: makes a number text (`%g`) from a value (+0x78) |
| `0xa6c0e0` | Camera::FocusA6C0E0 | In outline: when there is a shared target (`0x13429B4`), moves the camera to it (MainCamera::Get) |
| `0x57acb0` | Window::Build57ACB0 | In outline: builds a window from the shared title `0x12F53B4` |
| `0x666ae0` | Window::Part666AE0 | In outline: makes a window part (Window::Part666570, `0x6456F0`) |
| `0x7bcb20` | Window::Find7BCB20 | In outline: finds a window entry (`0x495720`) when there is one |
| `0x55d570` | Window::Build55D570 | In outline: builds a window from the shared title `0x12F5350` |
| `0x654500` | Window::Build654500 | In outline: builds a window from the shared title `0x12F7FF8` |
| `0x62b620` | Window::Build62B620 | In outline: builds a window from the shared title `0x12F7DFC` |
| `0x4ddd80` | Window::Build4DDD80 | In outline: builds a window from the shared title `0x12F4F20` |
| `0x6456f0` | Window::Build6456F0 | In outline: builds a window from the shared title `0x12F7F94` |
| `0xa842d0` | Window::BuildA842D0 | In outline: builds a window from the shared title `0x13CDEB0` |
| `0x61fd30` | Window::Update61FD30 | In outline: activates a window's part and acts for a slot below 0 |
| `0xa26330` | Window::UpdateA26330 | In outline: when on (+0xE8) with the shared window (`0x12F80D4`) |
| `0x6d5380` | Window::Build6D5380 | In outline: builds a window from the shared title `0x1305F30` |
| `0xa29b60` | Info::TextA29B60 | In outline: makes an info text (`0xA290D0`) with flag 0x10 handled |
| `0x6d5c40` | Info::Text6D5C40 | In outline: makes an info text |
| `0x6ed030` | Window::Build6ED030 | In outline: builds a window from the shared title `0x13060E0` |
| `0xa5e480` | Window::BuildA5E480 | In outline: builds a window from the shared title `0x1342994` |
| `0x6307c0` | Window::Build6307C0 | In outline: builds a window from the shared title `0x12F7E30` |
| `0xaef9c0` | Window::BuildAEF9C0 | In outline: builds a window from the shared title `0x13CE5A0` |
| `0xad6610` | Window::BuildAD6610 | In outline: builds a window from the shared title `0x13CE4D0` |
| `0x9b6660` | Window::Colour9B6660 | In outline: places a window and sets its colour from its values (Colour::ToARGB) |
| `0x678b60` | Window::Build678B60 | In outline: builds a window from the shared title `0x12F81EC` |
| `0x9bbab0` | Window::Build9BBAB0 | In outline: builds a window from the shared title `0x1341438` |
| `0x6ed4f0` | Window::Build6ED4F0 | In outline: builds a window from the shared title `0x13060EC` |
| `0x6aeab0` | Window::Hit6AEAB0 | In outline: another window hit test like Window::Hit6AED50 |
| `0x679fb0` | Window::Build679FB0 | In outline: builds a window from the shared title `0x12F8204` |
| `0x48ebc0` | Marker::Text48EBC0 | In outline: makes a marker's text from the first character of a text |
| `0x9bcb60` | Window::Layout9BCB60 | In outline: lays out a window and refreshes it (`0x9D5510`) |
| `0x6daec0` | Window::Layout6DAEC0 | In outline: refreshes (`0x6D4870`) and places a window |
| `0x5f7c50` | Window::Build5F7C50 | In outline: builds a window from the shared title `0x12F56E0` |
| `0x9c08b0` | Window::Update9C08B0 | In outline: refreshes a window part (`0x9BE750`) when not flagged (+0x38) |
| `0x719fd0` | Window::Check719FD0 | In outline: checks a window row (`0x981700`) unless told not to |
| `0x6f1be0` | Window::Fade6F1BE0 | In outline: moves a window's fades (+0xD4, +0x110) toward 1 |
| `0x9bff50` | Window::Build9BFF50 | In outline: builds a window from the shared title `0x1341464` |
| `0x42a770` | Window::Build42A770 | In outline: builds a window from the shared title `0x1103BCC` |
| `0x98fa60` | List::Pass98FA60 | In outline: goes over a list's entries (+0x44) |
| `0x6a5370` | Window::Keys6A5370 | In outline: sets up a window's key combinations (KeyCombination::Set, one with 0x4000000) |
| `0x61ea40` | MessageConsole::Open | In outline: when on (+0xF8), opens the developers' message console (MessageConsole::MessageConsole) |
| `0x6a32a0` | Window::Build6A32A0 | In outline: builds a window from the shared title `0x1305C54` |
| `0xa7afe0` | Window::BuildA7AFE0 | In outline: builds a window from the shared title `0x13CDE50` |
| `0x9f6170` | Window::Build9F6170 | In outline: builds a window from the shared title `0x1341D18` |
| `0x6607b0` | List::Layout6607B0 | In outline: lays out a list (+0x184 rows) |
| `0x9b1d50` | Window::Text9B1D50 | In outline: sets a window's text when it changes |
| `0x8e5610` | Window::Build8E5610 | In outline: builds a window from the shared title `0x1313C9C` |
| `0xae6780` | List::GrowAE6780 | In outline: grows a list (ObjectArray::SetCapacity on +0x4C) and fills it from another (+0x44) |
| `0x86f050` | Window::Build86F050 | In outline: builds a window from the shared title `0x1310EB8` |
| `0xa06b10` | Window::ShowA06B10 | In outline: makes a window part and acts when asked |
| `0x9ef7f0` | Window::Build9EF7F0 | In outline: builds a window from the shared title `0x1341C80` |
| `0x61ef90` | Window::Update61EF90 | In outline: a window update (`0x42B3C0`) |
| `0x67d060` | Window::Build67D060 | In outline: builds a window from the shared title `0x12F83C4` |
| `0x69ebf0` | Window::Build69EBF0 | In outline: builds a window from the shared title `0x1305BFC` |
| `0x67d830` | Window::Build67D830 | In outline: builds a window from the shared title `0x12F83CC` |
| `0x798fd0` | Toolbar::Build798FD0 | In outline: builds an image button (Button::SetImages with five images) |
| `0x78a280` | Window::Build78A280 | In outline: builds a window from the shared title `0x1306A78` |
| `0x63b780` | Window::Build63B780 | In outline: builds a window from the shared title `0x12F7E8C` |
| `0x61e7b0` | MessageConsole::Make | In outline: makes the developers' message console (MessageConsole::MessageConsole) and activates it |
| `0x6b1be0` | Window::Build6B1BE0 | In outline: builds a window from the shared title `0x1305D3C` |
| `0x6e4fe0` | Window::Build6E4FE0 | In outline: builds a window from the shared title `0x130604C` |
| `0x9a3920` | Window::Build9A3920 | In outline: builds a window from the shared title `0x13413D0` |
| `0xc64840` | Window::ScaleC64840 | In outline: works out a window's scale when the shared value is unset (`0x13E6D08`) |
| `0x694c10` | Layout::Setup694C10 | In outline: sets up the layout table when its value is unset (`0x10EEDB4` = 0xFFFF, `0x6930B0`) |
| `0x9aec50` | Window::Text9AEC50 | In outline: sets a window's text when it changes |
| `0x444890` | Window::Message444890 | In outline: another message result maker |
| `0xa54220` | Window::TextsA54220 | In outline: makes a window's texts from a shared text (`0xF30568`, `0xA532A0`) |
| `0x7f4540` | Window::Build7F4540 | In outline: builds a window from the shared title `0x130AB70` |
| `0x68e950` | Window::Build68E950 | In outline: builds a window from the shared title `0x12F8488` |
| `0x667710` | Window::Build667710 | In outline: builds a window from the shared title `0x12F8130` |
| `0x4ca0b0` | Window::Build4CA0B0 | In outline: builds a window from the shared title `0x12F4E40` |
| `0x980160` | Window::Layout980160 | In outline: places a window's rows (Window::UpdateRect) |
| `0xacad60` | Graph::DrawACAD60 | In outline: draws a graph's frame (`0x6ACE50`) and contents when not flagged (+0x100) |
| `0x6b2880` | Window::Update6B2880 | In outline: a window update (`0xBD36C0`) with no entries (+0x34) |
| `0x601710` | Window::Part601710 | In outline: makes a 0x130-byte window part |
| `0x6c3050` | Window::Build6C3050 | In outline: builds a window from the shared title `0x1305DF4` |
| `0x6a9600` | Window::Layout6A9600 | In outline: places a window when its owner is not flagged (+0xD8 +0xD8) |
| `0x633c90` | Window::Build633C90 | In outline: builds a window from the shared title `0x12F7E58` |
| `0xa217a0` | Window::BuildA217A0 | In outline: builds a window from the shared title `0x1342714` |
| `0xa21100` | Window::BuildA21100 | In outline: builds a window from the shared title `0x1342720` |
| `0xac94f0` | Window::BuildAC94F0 | In outline: builds a window from the shared title `0x13CE4A0` |
| `0x9a36b0` | Window::Build9A36B0 | In outline: builds a window from the shared title `0x13413D4` |
| `0x9a3440` | Window::Build9A3440 | In outline: builds a window from the shared title `0x13413C4` |
| `0x9a31d0` | Window::Build9A31D0 | In outline: builds a window from the shared title `0x13413C0` |
| `0x6abd90` | Window::Build6ABD90 | In outline: builds a window from the shared title `0x1305CE0` |
| `0x666870` | Window::Part666870 | In outline: makes a window part (Window::Part666570) with its child |
| `0x619180` | Window::Build619180 | In outline: builds a window from the shared title `0x12F5780` |
| `0x4c2100` | Info::Text4C2100 | In outline: makes an info text from two shared texts (`0xF31F48`, `0xF31FA8`) |
| `0xa49240` | Window::BuildA49240 | In outline: builds a window from the shared title `0x1342910` |
| `0x6a2620` | Window::Build6A2620 | In outline: builds a window from the shared title `0x1305C4C` |
| `0x9dd650` | Window::List9DD650 | In outline: fills a window from a shared list (`0x134180C` +0x14) |
| `0xc65490` | Window::MessageC65490 | In outline: handles a window message by kind (not 8; 0x10005, ...) |
| `0x4787c0` | Game::Pump4787C0 | In outline: pumps Windows messages with 0.1 (Windows::PumpMessages) and calls `0xA1F980` |
| `0x6e1b30` | Objectives::Event6E1B30 | In outline: handles an event from its own part (+8 +0x104) (`0xA2C580`) |
| `0x48fd90` | Window::Build48FD90 | In outline: builds a window from the shared title `0x12F4670` |
| `0xa55b30` | Window::LayoutA55B30 | In outline: places a window's parts (Window::UpdateRect) |
| `0x6ba290` | Window::Event6BA290 | In outline: for event 7, lays the window out |
| `0x8868b0` | Window::Build8868B0 | In outline: builds a window from the shared title `0x1310FB0` |
| `0x99cf40` | Window::Build99CF40 | In outline: builds a window from its title |
| `0xac9a00` | Window::BuildAC9A00 | In outline: builds a window from the shared title `0x13CE494` |
| `0x67b0e0` | Window::Update67B0E0 | In outline: with the GUI root (`0x12F83F0`), refreshes a window (`0xA1F8B0`) |
| `0xa70180` | Window::MakeA70180 | In outline: makes a list member window part (ListMember with `0x10EC9FC`, Listener440) |
| `0x98c4d0` | Window::Build98C4D0 | In outline: builds a window from the shared title `0x1341320` |
| `0x6072a0` | Window::Build6072A0 | In outline: builds a window from the shared title `0x12F5708` |
| `0x69a000` | Window::Part69A000 | In outline: makes a window part with a value |
| `0x61da30` | Window::Build61DA30 | In outline: builds a window from the shared title `0x12F579C` |
| `0x888140` | Window::Build888140 | In outline: builds a window from the shared title `0x1310FA0` |
| `0x69ccf0` | List::Sort69CCF0 | In outline: sorts a list's entries (a 0x337-entry counted buffer, Memory::AllocCounted) |
| `0x668300` | Window::Event668300 | In outline: handles an event from itself by its mode (+0x16C = 1) |
| `0xa55200` | PlayerProfile::Make | In outline: makes the player profile (ManagedPlayerProfile) and activates it |
| `0xa40700` | Window::BuildA40700 | In outline: builds a window from the shared title `0x13428E8` |
| `0x71a5b0` | Window::Build71A5B0 | In outline: builds a window from the shared title `0x1306254` |
| `0x68b1a0` | Window::Build68B1A0 | In outline: builds a window from the shared title `0x12F8448` |
| `0x871300` | Window::Build871300 | In outline: builds a window from the shared title `0x1310ED8` |
| `0xb4f310` | Window::BuildB4F310 | In outline: builds a window from the shared title `0x13CE9F0` |
| `0x7b1930` | Window::Build7B1930 | In outline: builds a window from the shared title `0x1306CC4` |
| `0x6ecd40` | Window::Build6ECD40 | In outline: builds a window from the shared title `0x13060E4` |
| `0xa5c970` | Window::PickA5C970 | In outline: gives a part by case (0, ...) |
| `0x63ffd0` | Window::Build63FFD0 | In outline: builds a window from the shared title `0x12F7ED0` |
| `0x678310` | Scrollbar::Clamp678310 | In outline: keeps a scroll position (+0xF0) between 0 and its range (+0x84 minus +0x7C) |
| `0x64ac90` | Window::Title64AC90 | In outline: makes a window's title, copying a default when none is given |
| `0x634820` | Window::Title634820 | In outline: the same title maker (another copy) |
| `0x79d4f0` | Window::Make79D4F0 | In outline: makes a window part when there is none |
| `0x9911e0` | List::Grow9911E0 | In outline: grows a list by one (ObjectArray::SetCapacity) |
| `0xae7920` | Window::BuildAE7920 | In outline: builds a window from the shared title `0x13CE580` |
| `0xae6a10` | Window::BuildAE6A10 | In outline: builds a window from the shared title `0x13CE584` |
| `0x9cb730` | Window::Part9CB730 | In outline: makes a 0x1C0-byte window part |
| `0x991430` | Window::Link991430 | In outline: links a window part when given one |
| `0x9de250` | Window::Layout9DE250 | In outline: places a window's parts (Window::UpdateRect) |
| `0x61d7f0` | Window::Text61D7F0 | In outline: makes a text (`0x7C8700`) when there is one |
| `0x99d410` | Window::Build99D410 | In outline: builds a window from its title |
| `0x63f310` | Window::Text63F310 | In outline: makes a window text from an empty one (`0xF264AD`) |
| `0x634ae0` | Window::Build634AE0 | In outline: builds a window from the shared title `0x12F7E64` |
| `0x8f9b70` | Window::Build8F9B70 | In outline: builds a window from the shared title `0x1339FC0` |
| `0x78fec0` | Window::Title78FEC0 | In outline: makes a window's title, copying a default when none is given |
| `0xc61530` | Window::FadeC61530 | In outline: when a window's value (+0xD0) is 0, clears a flag (Flags::Bit0) |
| `0x9c1fd0` | Window::Scroll9C1FD0 | In outline: a step when a part's value (+0x84) is positive |
| `0x8e5df0` | Window::Build8E5DF0 | In outline: builds a window from the shared title `0x1313CA4` |
| `0x7007a0` | Window::Layout7007A0 | In outline: places a window's parts (Window::UpdateRect) |
| `0x6857f0` | Text::Empty6857F0 | In outline: when not flagged (+0xE4), checks whether a text is empty |
| `0xb90d30` | Window::BuildB90D30 | In outline: builds a window from the shared title `0x13D01E4` |
| `0xa4aec0` | Window::BuildA4AEC0 | In outline: builds a titled window (Window::BuildTitled) with a list member (ListMember453) |
| `0x9bc930` | Window::Text9BC930 | In outline: sets a window's text from an empty one when it has one (+0x58 +0xC) |
| `0xa58e80` | List::LayoutA58E80 | In outline: lays out a list (+0x184 rows) |
| `0x67f3b0` | Window::Build67F3B0 | In outline: builds a window from the shared title `0x12F83DC` |
| `0xa3d9d0` | Window::TextA3D9D0 | In outline: sets a window's text when not flagged (+0x378) |
| `0xa35890` | Window::LayoutA35890 | In outline: places a window's parts (Window::UpdateRect) |
| `0x42e060` | Window::Build42E060 | In outline: builds a window from the shared title `0x1103C00` |
| `0xb5b290` | Window::BuildB5B290 | In outline: builds a window from the shared title `0x13CEAD0` |
| `0xc64550` | Window::SizeC64550 | In outline: works out a window's size from its parts |
| `0xaca200` | Graph::RangeACA200 | In outline: a graph's range from its values |
| `0x667990` | Window::Place667990 | In outline: places a window by its screen position (Window::ScreenPosition) |
| `0x643ec0` | Window::Build643EC0 | In outline: builds a window from the shared title `0x12F7EF8` |
| `0xa60c10` | Window::FadeA60C10 | In outline: a window fade step when far enough from 1 |
| `0xa2d640` | Window::LayoutA2D640 | In outline: places a window's parts (Window::UpdateRect) |
| `0x630fb0` | Window::Build630FB0 | In outline: builds a window from the shared title `0x12F7E40` |
| `0xb9f980` | Window::BuildB9F980 | In outline: builds a window from the shared title `0x13D05DC` |
| `0x68f1c0` | Window::Build68F1C0 | In outline: builds a window from the shared title `0x12F848C` |
| `0x68d2b0` | Window::Build68D2B0 | In outline: builds a window from the shared title `0x12F8480` |
| `0x681050` | Window::Drag681050 | In outline: a drag step once the mouse has moved far enough |
| `0x6b3e10` | Window::Update6B3E10 | In outline: a window step when on (+0x40034) |
| `0x6dc5a0` | Objectives::ParkValueText | In outline: makes a park value condition's text (ParkValueCondition::Name) |
| `0x65a870` | Window::Event65A870 | In outline: handles event 1 |
| `0x688280` | Window::Build688280 | In outline: builds a window from the shared title `0x12F846C` |
| `0x5f4ba0` | Window::Build5F4BA0 | In outline: builds a window from the shared title `0x12F56F0` |
| `0x6fcf50` | Window::Kind6FCF50 | In outline: acts by a window's kind (+0x14) |
| `0x664b10` | Window::Build664B10 | In outline: builds a window from the shared title `0x12F8110` |
| `0x702ca0` | Window::Build702CA0 | In outline: builds a window from the shared title `0x1306184` |
| `0x9b2340` | Window::Build9B2340 | In outline: builds a window from the shared title `0x1341430` |
| `0x6a23e0` | Window::Build6A23E0 | In outline: builds a window from the shared title `0x1305C48` |
| `0x6869f0` | Window::Build6869F0 | In outline: builds a window from its title |
| `0x67c370` | Window::Build67C370 | In outline: builds a window from the shared title `0x12F8228` |
| `0x6a3540` | Window::Build6A3540 | In outline: builds a window from the shared title `0x1305C5C` |
| `0x917210` | Window::Build917210 | In outline: builds a window from the shared title `0x133A13C` |
| `0x78a750` | Window::Layout78A750 | In outline: places a window's parts (Window::UpdateRect) |
| `0x98d860` | Info::Text98D860 | In outline: makes an info text from a shared text (`0xF31F48`) |
| `0x6a8d00` | Window::Build6A8D00 | In outline: builds a window from the shared title `0x1305CB0` |
| `0x670380` | Window::Build670380 | In outline: builds a window from the shared title `0x12F8188` |
| `0x63a3a0` | Window::Build63A3A0 | In outline: builds a window from the shared title `0x12F7E80` |
| `0x79c950` | Toolbar::Same79C950 | In outline: a step when two shared values match (`0x1306BCC`, `0x1306BD8`) |
| `0x6435f0` | Window::Draw6435F0 | In outline: draws a window part (GUIRenderer) |
| `0xa33350` | Window::LinkA33350 | In outline: links a window part when given one |
| `0xb43e30` | Window::BuildB43E30 | In outline: builds a window from the shared title `0x13CE990` |
| `0x630ab0` | Window::Build630AB0 | In outline: builds a window from the shared title `0x12F7E38` |
| `0xc603a0` | Window::FocusC603A0 | In outline: a step when it is the window system's focused one (`0x13E6D00` +0x190) |
| `0x5eabd0` | Window::Build5EABD0 | In outline: builds a window from the shared title `0x12F5644` |
| `0xa04da0` | Window::BuildA04DA0 | In outline: builds a window from the shared title `0x1341DD8` |
| `0x6ab4e0` | Window::Build6AB4E0 | In outline: builds a window from the shared title `0x1305CCC` |
| `0x467a80` | Window::Build467A80 | In outline: builds a window from the shared title `0x12EFFF4` |
| `0xa4c320` | Window::BuildA4C320 | In outline: builds a window from the shared title `0x1342924` |
| `0x6fc5d0` | Window::Event6FC5D0 | In outline: handles an event from its own part (+0x164) |
| `0x6b4040` | Window::Build6B4040 | In outline: builds a window from the shared title `0x1305D84` |
| `0x6c22f0` | Window::Update6C22F0 | In outline: a window step when on (+0x2C) |
| `0x69fc20` | Window::Build69FC20 | In outline: builds a window from the shared title `0x1305C08` |
| `0x8dd470` | Window::Build8DD470 | In outline: builds a window from the shared title `0x1313C74` |
| `0x7e3d90` | Window::Build7E3D90 | In outline: builds a window from the shared title `0x1309DF8` |
| `0x685bd0` | Window::Build685BD0 | In outline: builds a window from the shared title `0x12F8464` |
| `0x65d780` | Window::Build65D780 | In outline: builds a window from the shared title `0x12F80C4` |
| `0x6a57c0` | Window::Build6A57C0 | In outline: builds a window from the shared title `0x1305C8C` |
| `0x6a18b0` | Window::Build6A18B0 | In outline: builds a window from the shared title `0x1305C38` |
| `0x7ef3d0` | Window::Build7EF3D0 | In outline: builds a window from the shared title `0x130A8D0` |
| `0x8716b0` | Window::Build8716B0 | In outline: builds a window from the shared title `0x1310EDC` |
| `0x6d0d60` | Window::Refresh6D0D60 | In outline: refreshes a window (`0x6D4690`) |
| `0x6a84d0` | Window::Build6A84D0 | In outline: builds a window from the shared title `0x1305CAC` |
| `0x687f30` | Window::Build687F30 | In outline: builds a window from the shared title `0x12F8468` |
| `0x67dd20` | List::Pass67DD20 | In outline: goes over a list's entries (+0x11C) |
| `0x4873b0` | Window::Build4873B0 | In outline: builds a window from the shared title `0x12F462C` |
| `0xc64350` | Window::RectC64350 | In outline: sets a window's rectangle (Rect::Assign) |
| `0xb4f560` | Window::BuildB4F560 | In outline: builds a window from the shared title `0x13CEA2C` |
| `0x9f6e50` | Window::Build9F6E50 | In outline: builds a window from the shared title `0x1341D20` |
| `0x6ab2a0` | Window::Build6AB2A0 | In outline: builds a window from the shared title `0x1305CD4` |
| `0x6a1b50` | Window::Build6A1B50 | In outline: builds a window from the shared title `0x1305C34` |
| `0x6dca90` | Objectives::ShopProfitText | In outline: makes a shop profit condition's text (ShopProfitCondition::Name) |
| `0x6ac730` | Window::Build6AC730 | In outline: builds a window from the shared title `0x1305CE8` |
| `0x648020` | Window::Build648020 | In outline: builds a window from the shared title `0x1306A88` |
| `0xc6bfe0` | Window::MakeC6BFE0 | In outline: makes a window part (`0xC65F20`) |
| `0xc5e520` | Device::SetC5E520 | In outline: a device setting when asked |
| `0xa21490` | ListWindow::ClearA21490 | In outline: empties a list window (ListWindow::SetCount(0)) |
| `0x6ab730` | Window::Build6AB730 | In outline: builds a window from the shared title `0x1305CD0` |
| `0x41ce20` | Window::Build41CE20 | In outline: builds a window from the shared title `0x1103B34` |
| `0xacf3a0` | Window::UpdateACF3A0 | In outline: a window step when on (+0x2C) |
| `0x6f5680` | Window::Update6F5680 | In outline: a window update when not flagged (+0x9C) |
| `0xacc6a0` | Window::BuildACC6A0 | In outline: builds a window from the shared title `0x13CE490` |
| `0x6ea360` | Window::Build6EA360 | In outline: builds a window from the shared title `0x13060C8` |
| `0x673660` | Window::Draw673660 | In outline: draws a window's frame (`0x6ACE50`) |
| `0x6218d0` | Window::Layout6218D0 | In outline: places a window's parts (Window::UpdateRect) |
| `0x813fc0` | Window::Build813FC0 | In outline: builds a window from the shared title `0x130B37C` |
| `0x70a1d0` | Window::Build70A1D0 | In outline: builds a window from the shared title `0x13061F0` |
| `0xa63950` | Window::ModeA63950 | In outline: acts by a window's mode (+0x1FC) |
| `0xa2d9e0` | Window::ShowA2D9E0 | In outline: a step when asked or when it has a part (+0x100) |
| `0x6a6100` | Window::Build6A6100 | In outline: builds a window from the shared title `0x1305C9C` |
| `0x67dac0` | Window::Build67DAC0 | In outline: builds a window from the shared title `0x1305CC4` |
| `0x4e41b0` | Window::Build4E41B0 | In outline: builds a window from the shared title `0x12F4F90` |
| `0xb01e70` | Window::BuildB01E70 | In outline: builds a window from the shared title `0x13CE690` |
| `0x6bc920` | Window::Event6BC920 | In outline: handles an event, passing it on first (Thunk) |
| `0x503d10` | Window::Build503D10 | In outline: builds a window from the shared title `0x12F5054` |
| `0x97dc80` | Window::Refresh97DC80 | In outline: refreshes a window (`0x6AFC20`) |
| `0x6aa6c0` | Window::Size6AA6C0 | In outline: a window size for values up to 0x100 |
| `0x9dd9b0` | Window::Build9DD9B0 | In outline: builds a window from the shared title `0x1341834` |
| `0x9dbe60` | Window::Build9DBE60 | In outline: builds a window from the shared title `0x1341828` |
| `0x5551c0` | Window::Narrow5551C0 | In outline: a step when the GUI root's width (`0x12F83F0` +0x88 minus +0x80) is under a limit |
| `0x7ea4e0` | List::Pass7EA4E0 | In outline: goes over a list (+0x4F0) |
| `0x6a6370` | Window::Build6A6370 | In outline: builds a window from the shared title `0x1305CA0` |
| `0x654fc0` | Window::Build654FC0 | In outline: builds a window from the shared title `0x12F8078` |
| `0x654d60` | Window::Build654D60 | In outline: builds a window from the shared title `0x12F8070` |
| `0x6aa8b0` | Window::Size6AA8B0 | In outline: a window size for values up to 0x100 |
| `0x6aa4d0` | Window::Size6AA4D0 | In outline: a window size for values up to 0x100 |
| `0x991c30` | List::Add991C30 | In outline: adds a list entry when there is room (+0x44 vs +0x4C) |
| `0x6b49a0` | Window::Build6B49A0 | In outline: builds a window from the shared title `0x1305D78` |
| `0x6180b0` | Window::Build6180B0 | In outline: builds a window from the shared title `0x12F5778` |
| `0x69b830` | Window::Text69B830 | In outline: makes a window text from an empty one |
| `0xc63ed0` | Window::PassC63ED0 | In outline: goes over a window's parts |
| `0x9f6920` | Window::Place9F6920 | In outline: places a window part (Window::SetRelationAndLayout with 6) |
| `0x6f2830` | Window::Update6F2830 | In outline: a window step when a value is negative |
| `0xb8ba60` | Info::TextB8BA60 | In outline: makes an info text from a shared text (`0xF31F48`) |
| `0xa8ac30` | Info::TextA8AC30 | In outline: makes an info text from a shared text (`0xF31F48`) |
| `0x73e790` | Toolbar::Button73E790 | In outline: builds a button (Constructor with 0x22) |
| `0x687480` | Window::Build687480 | In outline: builds a window from the shared title `0x12F845C` |
| `0x4efdf0` | Window::Build4EFDF0 | In outline: builds a window from the shared title `0x12F8144` |
| `0x638f80` | Window::Value638F80 | In outline: a value from `0x638DA0` |
| `0xa533a0` | Window::ModeA533A0 | In outline: a step in mode 3 |
| `0xa33800` | Window::LinkA33800 | In outline: a step with a part when it has one |
| `0xa7f870` | Window::ModeA7F870 | In outline: a step in mode 2 |
| `0xa1be00` | Window::LoopA1BE00 | In outline: a window loop |
| `0xa58130` | Window::MessageA58130 | In outline: handles a message by its handler |
| `0xa1e230` | Window::UpdateA1E230 | In outline: a step when on (+0xA4) with a part (+0x84) |
| `0x990400` | Window::Text990400 | In outline: a window text when not flagged (+0x88) |
| `0x968370` | Window::Text968370 | In outline: makes a window text from an empty one |
| `0x65be40` | List::Pass65BE40 | In outline: goes over a list (+0xD8) |
| `0xa1cf10` | Window::LinkA1CF10 | In outline: a step with a part (+0x3C) |
| `0xc62ff0` | Window::ChildC62FF0 | In outline: a step for another window that passes its check |
| `0x686550` | Window::Text686550 | In outline: makes a text (`0x53B850`) |
| `0x420630` | AdMenu::Hide420630 | In outline: hides the ad menu's media (AdMenu::ShowMedia(-1)) |
| `0x6276e0` | Labels::Click6276E0 | In outline: a step with the label under the mouse (Labels::Pick474A40) |
| `0x985ae0` | Window::Link985AE0 | In outline: links a window part |
| `0x6c2e80` | Window::Money6C2E80 | In outline: shows a money text (Money::Text6BEF60) |
| `0xa3b690` | Window::MakeA3B690 | In outline: makes a part |
| `0x479850` | Window::Make479850 | In outline: makes a part |
| `0x458840` | Window::Make458840 | In outline: makes a part |
| `0x9eea50` | Window::Mode9EEA50 | In outline: a step in mode 1 |
| `0x67a960` | Window::Mode67A960 | In outline: a step in modes 2 or 3 |
| `0x6a0560` | Window::Make6A0560 | In outline: makes a part |
| `0x6f14e0` | Window::Refresh6F14E0 | In outline: refreshes a window (`0x6D4620`) |
| `0x621100` | Save::Write621100 | In outline: writes values to a save (SaveWriter::Write, four bytes) |
| `0xa3ebe0` | Window::CheckA3EBE0 | In outline: a check when given an object that has a part |
| `0x67cdd0` | Window::Value67CDD0 | In outline: a value with no entries handled |
| `0x6be5a0` | Money::Text2 | In outline: makes a money text with two decimal places (Money::Format_(2)) |
| `0x41d750` | Window::Draw41D750 | In outline: draws a window's frame (`0x6ACE50`) |
| `0x8863a0` | Window::Layout8863A0 | In outline: places a window's parts (Window::UpdateRect) |
| `0xb4fa70` | Window::RefreshB4FA70 | In outline: a refresh (`0x665E90`) |
| `0xa35550` | Window::PlaceA35550 | In outline: places a window by its screen position (Window::ScreenPosition) |
| `0x9be910` | Window::Update9BE910 | In outline: a window update when not flagged (+0x1C) |
| `0x9be750` | Window::Update9BE750 | In outline: a window update when not flagged (+0x38) |
| `0x6d4690` | Window::Layout6D4690 | In outline: places a window's parts (Window::UpdateRect) |
| `0x707070` | Window::ParkRating707070 | In outline: shows the park rating's breakdown (Park::RatingBreakdown) |
| `0x61f220` | Window::Draw61F220 | In outline: draws a window's frame (`0x6ACE50`) |
| `0xace520` | Window::UpdateACE520 | In outline: a window step when on (+0x2C) |
| `0x68c770` | Window::Refresh68C770 | In outline: a refresh (`0x6879A0`) |
| `0x664620` | Window::Part664620 | In outline: a step with its part (+0x1B4 +0xC) |
| `0x65ab80` | Window::Mode65AB80 | In outline: a step in mode 1 |
| `0x98deb0` | Window::Mode98DEB0 | In outline: a step unless in mode 1 (+0x108) |
| `0xb929f0` | Message::ResultB929F0 | In outline: another message result maker like Window::Message444890 |
| `0xb32b60` | Message::ResultB32B60 | In outline: the same message result maker (another copy) |
| `0xb10dc0` | Message::ResultB10DC0 | In outline: the same message result maker (another copy) |
| `0xa2af70` | Message::ResultA2AF70 | In outline: the same message result maker (another copy) |
| `0xa2a440` | Message::ResultA2A440 | In outline: the same message result maker (another copy) |
| `0xa0af40` | Message::ResultA0AF40 | In outline: the same message result maker (another copy) |
| `0x8fcd50` | Message::Result8FCD50 | In outline: the same message result maker (another copy) |
| `0x8f8d10` | Message::Result8F8D10 | In outline: the same message result maker (another copy) |
| `0x7bc4d0` | Message::Result7BC4D0 | In outline: the same message result maker (another copy) |
| `0x7bc1b0` | Message::Result7BC1B0 | In outline: the same message result maker (another copy) |
| `0x7578e0` | Message::Result7578E0 | In outline: the same message result maker (another copy) |
| `0x703a90` | Message::Result703A90 | In outline: the same message result maker as Window::Message444890 (another copy) |
| `0x69ae70` | Message::Result69AE70 | In outline: the same message result maker (another copy) |
| `0x69acb0` | Message::Result69ACB0 | In outline: the same message result maker (another copy) |
| `0x5a36f0` | Message::Result5A36F0 | In outline: the same message result maker (another copy) |
| `0x5a3510` | Message::Result5A3510 | In outline: the same message result maker (another copy) |
| `0x5a3350` | Message::Result5A3350 | In outline: the same message result maker (another copy) |
| `0x59a5f0` | Message::Result59A5F0 | In outline: the same message result maker (another copy) |
| `0x560d20` | Message::Result560D20 | In outline: the same message result maker (another copy) |
| `0x4a33d0` | Message::Result4A33D0 | In outline: the same message result maker (another copy) |
| `0x4a3130` | Message::Result4A3130 | In outline: the same message result maker (another copy) |
| `0x4a2e90` | Message::Result4A2E90 | In outline: the same message result maker (another copy) |
| `0x4a2bf0` | Message::Result4A2BF0 | In outline: the same message result maker (another copy) |
| `0x4a2950` | Message::Result4A2950 | In outline: the same message result maker (another copy) |
| `0x4a26b0` | Message::Result4A26B0 | In outline: the same message result maker (another copy) |
| `0x4a2410` | Message::Result4A2410 | In outline: the same message result maker (another copy) |
| `0x4a2170` | Message::Result4A2170 | In outline: the same message result maker (another copy) |
| `0x4a1ed0` | Message::Result4A1ED0 | In outline: the same message result maker (another copy) |
| `0x44e2a0` | Message::Result44E2A0 | In outline: the same message result maker (another copy) |
| `0x41cb90` | Message::Result41CB90 | In outline: the same message result maker (another copy) |
| `0x4040f0` | Message::Result4040F0 | In outline: the same message result maker (another copy) |
| `0x403f30` | Message::Result403F30 | In outline: the same message result maker (another copy) |
| `0x68e200` | Window::Shared68E200 | In outline: a step when the shared value (`0x1305BE4`) is 0 |
| `0x67b550` | Window::Mode67B550 | In outline: a step in modes 2 or 3 |
| `0xae2670` | Window::DrawAE2670 | In outline: draws a window's frame (`0x6ACE50`) |
| `0x9b1fe0` | Window::Update9B1FE0 | In outline: a window update when not flagged (+0x20) |
| `0x983230` | Window::Check983230 | In outline: a check (`0x982C70` with +0x58) |
| `0x6af120` | Window::Flags6AF120 | In outline: a step when flagged (bit 9 or bit 7) |
| `0x4c5460` | Shop::Text4C5460 | In outline: a text when not flagged (+0x378) |
| `0xc60f90` | Window::UpdateC60F90 | In outline: a window update when not flagged (+0xE8) |
| `0x6aaaa0` | Window::Draw6AAAA0 | In outline: draws a window part (GUIRenderer) |
| `0xc65160` | Window::PartC65160 | In outline: a step with a part (+0xF0) |
| `0x6ae130` | Window::Check6AE130 | In outline: a window list check |
| `0x9d5510` | Window::Refresh9D5510 | In outline: a refresh with an empty handle |
| `0x7891e0` | Window::Draw7891E0 | In outline: draws a window's frame (`0x6ACE50`) |
| `0x6472e0` | Window::Refresh6472E0 | In outline: a refresh (`0x645EB0` with +0x14) |
| `0x6b5d10` | Colour::Make6B5D10 | In outline: a colour from three values, the first kept in range |
| `0x642e80` | Window::Part642E80 | In outline: a step with a part |
| `0x6ace50` | Window::DrawFrame6ACE50 | In outline: draws a window's frame (as the other notes call it); many draw functions call it first |
| `0x6b0740` | Window::Loop6B0740 | In outline: goes over a list to its end |
| `0x61ede0` | Window::Mode61EDE0 | In outline: a step in mode 1 |
| `0xc61330` | Window::FadeC61330 | In outline: a step when a value is 0 |
| `0x65ffe0` | Window::Event65FFE0 | In outline: handles event 7 |
| `0xa33650` | Window::CheckA33650 | In outline: a check unless given nothing |
| `0x9c1870` | Window::Text9C1870 | In outline: a text when not flagged (+0x128) |
| `0x67cbd0` | Window::Update67CBD0 | In outline: a step when on (+0x10) |
| `0x6610f0` | Window::Build6610F0 | In outline: builds a window (Window::Build663680) |
| `0x66fdf0` | Window::Text66FDF0 | In outline: makes a text record |
| `0x4e4410` | Window::Main4E4410 | In outline: builds the main window (MainWindow::Build(0)) |
| `0xba26d0` | Window::SharedBA26D0 | In outline: a step unless the shared window (`0x1342708`) is on (+0xA4) |
| `0x98e350` | Window::Check98E350 | In outline: a check with no part (+0x140) |
| `0x6eb620` | Window::Text6EB620 | In outline: makes a text record |
| `0x6ae2f0` | Window::Check6AE2F0 | In outline: a window list check |
| `0xa04420` | Window::TextA04420 | In outline: a text (Text::JoinA04010) |
| `0x6733e0` | Window::Layout6733E0 | In outline: lays out a window (Window::Layout672DF0) |
| `0x6da760` | Window::Layout6DA760 | In outline: lays out a window (Window::Layout6D4690) |
| `0x6419f0` | Window::Count6419F0 | In outline: a step for a positive count |
| `0x79d200` | Toolbar::Inside79D200 | In outline: whether a point is inside fixed bounds (`0xF6FD60`, `0xF6FD64`) |
| `0x98e760` | Window::Make98E760 | In outline: makes a list member part (ListMember with `0x10EC9F8`) |
| `0x7afad0` | Window::Make7AFAD0 | In outline: makes a list member part (ListMember with `0x10EC9F8`) |
| `0x6eb470` | Window::Refresh6EB470 | In outline: a refresh (`0x6E9D70` with 0) |
| `0x6a3c00` | Window::Fill6A3C00 | In outline: fills buffers for the current device |
| `0x98b850` | Window::Check98B850 | In outline: a check when not flagged (+0xD4) |
| `0x683970` | Window::Draw683970 | In outline: draws a window (Window::Draw6A9EF0) |
| `0x6deda0` | Window::Name6DEDA0 | In outline: a name text (NameText_0x0) |
| `0xa8c380` | Window::LinkA8C380 | In outline: a step with a part |
| `0xa81ed0` | Window::LayoutA81ED0 | In outline: places a window's parts (Window::UpdateRect) |
| `0xa463a0` | Window::TextA463A0 | In outline: sets a text from a shared one (`0x12FE854`) |
| `0x6e9b80` | Window::Event6E9B80 | In outline: handles events 0x11 or 7 |
| `0x6cd750` | Window::Step6CD750 | In outline: a step (`0x6C5E50`) |
| `0x6ae630` | Window::Check6AE630 | In outline: a window list check |
| `0x9c12f0` | Window::Update9C12F0 | In outline: a step when on (+0x10) |
| `0x6d10d0` | Window::Link6D10D0 | In outline: a step when given a part |
| `0xa54e10` | Window::StepA54E10 | In outline: a step (`0xA52B40`) |
| `0x983450` | Window::Text983450 | In outline: makes a text record |
| `0x64b190` | Window::Check64B190 | In outline: a check when given an object that has a part |
| `0x991000` | Window::Update991000 | In outline: an update when not flagged (+0xFC) |
| `0x98e9a0` | Window::Step98E9A0 | In outline: a step (`0x98BC00`) |
| `0x6cf4b0` | Window::Count6CF4B0 | In outline: a step when a count (+0xC) is above 3 |
| `0xc67cf0` | Window::RectC67CF0 | In outline: sets a rectangle (Rect::Assign) |
| `0x6e53d0` | Window::Name6E53D0 | In outline: a name text (NameText_0x0) |
| `0xc66730` | Window::FadeC66730 | In outline: a step when a value (+0xD0) is not 0 |
| `0xb9e690` | Window::MakeB9E690 | In outline: makes a part (`0x6C3BE0`) |
| `0x6a7fc0` | Window::Mode6A7FC0 | In outline: a step in mode 1 |
| `0x42aa30` | Window::Add42AA30 | In outline: adds an entry when there is room (+0xD8 vs +0xDC) |
| `0x9f6c90` | Window::Alpha9F6C90 | In outline: an alpha rounded from a shared fade (`0x1103B6C` +0x180) |
| `0x665e90` | List::Pass665E90 | In outline: goes over entries (+0x178) |
| `0x6c8460` | Window::Make6C8460 | In outline: makes a window part (Listener440) |
| `0x98c090` | Window::Update98C090 | In outline: an update when not flagged (+0x20A) |
| `0xc62e70` | Window::RectC62E70 | In outline: sets a rectangle (Rect::Assign) |
| `0x6a4260` | Window::Activate6A4260 | In outline: makes a part active (Updatable::SetActive(1)) |
| `0x6e8e70` | Window::Value6E8E70 | In outline: a value with a part |
| `0x6ae4a0` | Window::Check6AE4A0 | In outline: a window list check |
| `0xa4ad40` | Window::StepA4AD40 | In outline: a step (`0x6A65B0`) |
| `0x98bf10` | Window::Update98BF10 | In outline: an update when not flagged (+0x20A) |
| `0x789ca0` | Window::Number789CA0 | In outline: a number text (String::FromNumber) |
| `0x64a840` | Window::Draw64A840 | In outline: draws a window part (GUIRenderer) |
| `0x695e80` | Layout::Mark695E80 | In outline: a step with the layout table, set up first when its value is unset (`0x10EEDB4` = 0xFFFF) |
| `0x6b5b90` | Colour::Value6B5B90 | In outline: a value kept within a limit |
| `0x4e2630` | TerrainEditor::Open | In outline: builds the terrain editor window (TerrainEditor::Build) |
| `0xc63840` | Window::CheckC63840 | In outline: a check (`0xC5FA90`) |
| `0x9c2ae0` | Window::Point9C2AE0 | In outline: a point check against a shape with more than three points |
| `0x9c2810` | Window::Point9C2810 | In outline: the same (another copy) |
| `0x65ec80` | Window::Mouse65EC80 | In outline: a step (`0x65EB50`) when a value is below 0 |
| `0x691110` | Text::Measure691110 | In outline: a loop over a text's characters |
| `0xac9880` | Window::OpenAC9880 | In outline: builds the window titled `0x13CE4A0` (Window::BuildAC94F0) |
| `0xa4bb00` | Window::LinkA4BB00 | In outline: sets a part when it changes |
| `0xa40450` | Window::PlaceA40450 | In outline: places a window when asked (Window::UpdateRect) |
| `0x84a750` | Scenery::Text84A750 | In outline: a text when not flagged (+0xC8) |
| `0x70b860` | Window::Event70B860 | In outline: for event 7, refreshes (`0x6AFC20`) |
| `0x6e5a80` | Objectives::MinAnimalHealthText | In outline: makes a minimum animal health condition's text with its number (MinAnimalHealthCondition::Name) |
| `0xa2c410` | Window::TextA2C410 | In outline: a text from entries (+0x1C) |
| `0x4161f0` | Window::Make4161F0 | In outline: makes a part with 3.0 (`0x685070`) |
| `0xa52b40` | Window::StepA52B40 | In outline: a window step |
| `0x6aac50` | Window::Draw6AAC50 | In outline: draws a window part (GUIRenderer) by its flags |
| `0x691290` | Text::Words691290 | In outline: goes over a text's words (spaces, 0x20) |
| `0x690150` | Window::Count690150 | In outline: a refresh (`0x6B0E30`) under a count |
| `0x67eb20` | Window::Layout67EB20 | In outline: a step (Window::Mode67A960) and places the window (Window::UpdateRect) |
| `0x4531d0` | Message::Number4531D0 | In outline: a message from a text with a four-digit number ("%s%04d") |
| `0xb09660` | Window::DrawB09660 | In outline: draws a part (GUIRenderer, `0x692710`) |
| `0x9be5e0` | Window::Part9BE5E0 | In outline: makes a part (`0x67BE80`) and places it (Window::SetRelationAndLayout with 6) |
| `0x6950b0` | Layout::Step6950B0 | In outline: a step with the layout table, set up first when its value is unset (`0x10EEDB4` = 0xFFFF) |
| `0xac9d60` | Window::OpenAC9D60 | In outline: builds the window titled `0x13CE494` (Window::BuildAC9A00) |
| `0xa2ab50` | Window::TextA2AB50 | In outline: a text from entries (+0x1C) |
| `0x6a4b40` | Window::Layout6A4B40 | In outline: places a window (Window::UpdateRect) and refreshes it (`0x6A4820`) |
| `0xc64e40` | Window::HitC64E40 | In outline: a hit test against a part's rectangle (Rect::Contains) |
| `0xa57530` | Window::SharedA57530 | In outline: a step when two shared values are set (`0x1304A24`, `0x13044AC`) |
| `0xa2b5f0` | Window::NameA2B5F0 | In outline: makes a named text from a shared object (`0x1342788` +0x5C) |
| `0x8df9d0` | Window::Check8DF9D0 | In outline: checks a box and lays out the window (Window::SetChecked(1), Window::Layout) |
| `0x7c8700` | Info::Text7C8700 | In outline: makes an info text from a wide string and a shared one (`0xF31F48`) |
| `0x6aba60` | Window::Layout6ABA60 | In outline: places a window (Window::UpdateRect) |
| `0x67a410` | Window::Flag67A410 | In outline: a step when a flag changes (bit 25) |
| `0xc62700` | Window::RectC62700 | In outline: a step for an empty rectangle when not flagged (+0x12) |
| `0xaefca0` | Scrollbar::RangeAEFCA0 | In outline: a step when a range (+0x84 minus +0x7C) is above a limit |
| `0xc65800` | Window::ScrollC65800 | In outline: a step against the window system's value (`0x13E6D00` +0xD4) |
| `0xa644a0` | Window::MakeA644A0 | In outline: makes a list member part (ListMember with `0x10EC9F8`) |
| `0x7e3ae0` | Window::Case7E3AE0 | In outline: a value by a mode (+0x114) |
| `0xc679e0` | Window::StepC679E0 | In outline: a step (`0xC675A0`) with a part |
| `0x6f5880` | Window::Update6F5880 | In outline: an update when not flagged (+0x9C) |
| `0x6f24e0` | Window::Check6F24E0 | In outline: a step when a part is not flagged (+0xA8 bit 3, +0xAC) |
| `0x64aee0` | Window::List64AEE0 | In outline: goes over a list (List::Pass665E90) |
| `0x42b850` | Window::Text42B850 | In outline: a text from a wide string and a message (Window::Message442C60) |
| `0x6b2ff0` | WindowList::Make | In outline: makes a window list (WindowList, `0xBD2740`) |
| `0x6417f0` | Window::Count6417F0 | In outline: a step for a positive count |
| `0x68f990` | Window::Count68F990 | In outline: a step under a count (+0x16C) |
| `0x63b270` | Window::Values63B270 | In outline: sets a window's values (Window::SetRelatedValue, Window::SetValueEC) |
| `0xa37040` | Window::PlaceA37040 | In outline: a step by its screen position (Window::ScreenPosition) against a limit |
| `0xa30980` | Info::TextA30980 | In outline: makes an info text from a wide string and a shared one (`0xF31F48`) |
| `0xa2d3d0` | Window::LayoutA2D3D0 | In outline: places two parts (Window::UpdateRect) |
| `0xa290d0` | Info::TextA290D0 | In outline: the same info text maker (another copy) |
| `0x99c500` | Window::Layout99C500 | In outline: lays out a window and places a part (Window::SetRelationAndLayout with 6) |
| `0x6b98d0` | Window::Case6B98D0 | In outline: a value by case |
| `0x690960` | Window::Camera690960 | In outline: a step with a child (Window::ChildrenSlot2) and the camera |
| `0x673860` | Window::Layout673860 | In outline: lays out a window (Window::Layout672DF0) when a check fails |
| `0x81afa0` | Info::Text81AFA0 | In outline: makes an info text from a wide string and a shared one (`0xF31F48`) |
| `0x6b3190` | WindowList::Make6B3190 | In outline: makes a window list (WindowList, `0xBD2740`) |
| `0x5f8290` | WindowList::Make5F8290 | In outline: makes a window list (WindowList, `0x6A5E20` with 0, 1, 1) |
| `0xa3ee80` | Window::UpdateA3EE80 | In outline: an update when not flagged (+0x20) |
| `0xa3c3d0` | Window::ClockA3C3D0 | In outline: in mode 1 (+0x4C), a step with the processor clock (_clock) |
| `0xa2e430` | Window::UpdateA2E430 | In outline: an update when not flagged (+0x30) |
| `0x81b100` | Window::Open81B100 | In outline: for -0x78, builds a window (Window::Build6A2620 with 0x9165) |
| `0xa2e2d0` | Window::UpdateA2E2D0 | In outline: the same (another copy) |
| `0xa06400` | Window::MoneyA06400 | In outline: shows a money text (`0xA06380`, Window::Money6C2E80) |
| `0x6b3570` | WindowList::Make6B3570 | In outline: makes a window list (WindowList, `0xBD2740`) |
| `0x639d70` | Window::Text639D70 | In outline: makes a text from a message (Message::Result4040F0 with `0x1303CFC`) |
| `0xac5390` | Window::OpenAC5390 | In outline: builds the window titled `0x13CE404` (Window::BuildAC4EF0) |
| `0xa2c660` | Window::TextA2C660 | In outline: makes texts from shared ones (`0xF264AD`, `0xF751C0`) |
| `0x993f00` | Window::Update993F00 | In outline: an update when not flagged (+0xE8) |
| `0xa35ae0` | Window::PlaceA35AE0 | In outline: places a window (Window::UpdateRect, Window::ScreenPosition) |
| `0x6f2010` | Window::Name6F2010 | In outline: a name text with a shared one (`0xF67028`) |
| `0x9c15c0` | Window::Mode9C15C0 | In outline: makes a part and a step unless in mode 1 (+0xF8) |
| `0x98ab20` | Window::Link98AB20 | In outline: links parts |
| `0x65d1d0` | Window::Layout65D1D0 | In outline: places two parts (Window::UpdateRect) |
| `0x639ec0` | Window::Value639EC0 | In outline: for 5, a value (Window::Value638F80) |
| `0x612000` | Window::Half612000 | In outline: a step for the first half of a count |
| `0x6205a0` | MessageConsole::AddEntry | In outline: makes a message console entry (ObjectMessageConsoleEntry) and activates it |
| `0xa44b00` | Window::IdA44B00 | In outline: a step for ids from 0x1ED8C9C up to a count (+0x244) |
| `0xa1e060` | Window::MoveA1E060 | In outline: moves every entry to another list (List::MoveAllTo) |
| `0x683b10` | Window::Shared683B10 | In outline: makes a part when the shared value (`0x1305BE4`) is 0 |
| `0xaa6280` | Window::DrawAA6280 | In outline: makes a part and draws (`0x694E90` with `0x10F25F8`) |
| `0x70f4f0` | Window::Draw70F4F0 | In outline: makes a part and draws (`0x694E90` with `0x10EF65C`) |
| `0x66a620` | Window::Layout66A620 | In outline: places a window (Window::UpdateRect) |
| `0x41bf90` | Window::Value41BF90 | In outline: a step (`0x6ADB60`) when a value is positive |
| `0x996140` | Window::Layout996140 | In outline: places a window (Window::UpdateRect) |
| `0x7873d0` | Window::Part7873D0 | In outline: a part with a text from `0x13069B4` (`0x67BE80` with 0x80) |
| `0x67e6a0` | Window::Build67E6A0 | In outline: builds a window (Window::Build67DFD0) with no entries handled |
| `0x6671d0` | Window::Mode6671D0 | In outline: in mode 1 (+0x170), sets a rectangle and refreshes (Window::SetRectAndRefresh) |
| `0x617440` | Window::Flag617440 | In outline: a step when a flag changes |
| `0x4c8f40` | Window::Part4C8F40 | In outline: a part with a text from `0x12F4CAC` (`0x67BE80` with 0x80) |
| `0xa62f90` | Window::CheckA62F90 | In outline: a step when a check fails |
| `0xa3c530` | Window::StepA3C530 | In outline: a step (`0xA3C1C0`) when given a part |
| `0x981be0` | Window::Update981BE0 | In outline: an update when not flagged (+0x40) |
| `0x6fd710` | Window::Slot6FD710 | In outline: a step for a non-negative slot |
| `0x633ff0` | Window::Edit633FF0 | In outline: a step unless the "cannot edit" message is shown (TrackedRide::ShowCannotEdit) |
| `0xad0710` | Window::PlaceAD0710 | In outline: when on (+0x44), places a window |
| `0x97e0a0` | Window::Layout97E0A0 | In outline: lays out a window and refreshes it when a value changes (+0xF0) |
| `0x9a7d90` | Window::Check9A7D90 | In outline: a check when not flagged (+0x88) |
| `0x6eabf0` | Window::Layout6EABF0 | In outline: places a window (Window::UpdateRect) |
| `0x701f50` | Window::Case701F50 | In outline: for its own value (+0x124), a step by case |
| `0x7be2f0` | Window::Make7BE2F0 | In outline: makes a window part (Listener440, Constructor) |
| `0x687df0` | ListWindow::Fill687DF0 | In outline: sets a list window's count (ListWindow::SetCount) and fills it |
| `0x5f4640` | WindowList::Make5F4640 | In outline: makes a window list (WindowList, `0x441090`) |
| `0x9b5560` | Window::Reset9B5560 | In outline: clears a number field's binding and a list's selection (NumberField::BindFloat(0), ListWindow::SetSelection(0)) |
| `0x995e40` | Window::Step995E40 | In outline: when on (+0x1EC), a step (`0x6C1DA0`) |
| `0x6ba5a0` | Window::Count6BA5A0 | In outline: a step under a count (`0x6C1DA0` +0x11C) |
| `0x6f75c0` | Window::Update6F75C0 | In outline: with no part (+0xA0), an update (Window::Update6F5680) |
| `0x9fac60` | NamedObject::Make9FAC60 | In outline: makes a list member object (ListMember with `0x10EC9E4`) |
| `0x6487a0` | Window::Set6487A0 | In outline: a step (`0x630360`) and sets a value (Set_+0x100) |
| `0x47a260` | Window::Text47A260 | In outline: a text for 0 to 0x1D |
| `0x9d4c70` | Window::Check9D4C70 | In outline: a check when not flagged (+0x234) |
| `0x9da430` | Window::Shared9DA430 | In outline: a step when the shared value (`0x1300718`) is set, unless given 2 |
| `0x9da260` | Window::Shared9DA260 | In outline: a step when the shared value (`0x1305798`) is set, unless given 2 |
| `0x6e2fb0` | Objectives::MoneyText6E2FB0 | In outline: a condition text with a money value or "unlimited" (Money::FormatOrUnlimited) |
| `0x6dd4b0` | Objectives::MoneyText6DD4B0 | In outline: the same (another copy) |
| `0x6dd000` | Objectives::MoneyText6DD000 | In outline: the same (another copy) |
| `0x99d1d0` | Window::Draw99D1D0 | In outline: draws a window's frame (Window::DrawFrame6ACE50) and makes a part |
| `0x6cc710` | Window::Value6CC710 | In outline: a value from a loop |
| `0x6e7060` | Objectives::EnclosureVisitorsText | In outline: makes an enclosure visitors condition's text with its number (EnclosureVisitorsCondition::Name) |
| `0x6e4ba0` | Objectives::NumberText6E4BA0 | In outline: a condition text with a number (String::FromNumber) |
| `0x6e4580` | Objectives::NumberText6E4580 | In outline: the same (another copy) |
| `0x98f7c0` | Window::Text98F7C0 | In outline: a text when not flagged (+0x4C) |
| `0xa4c6c0` | WindowList::MakeA4C6C0 | In outline: makes a window list (WindowList, `0x6AF710`) |
| `0x7af770` | Window::Text7AF770 | In outline: a text when not flagged (+0x4C) |
| `0x659de0` | Window::Layout659DE0 | In outline: places a window (Window::UpdateRect) |
| `0xc64220` | Window::RectC64220 | In outline: sets a rectangle (Rect::Assign, `0xC5E300`) |
| `0x9c8c80` | Window::Make9C8C80 | In outline: when on (+0x1B4), makes a part |
| `0xc61200` | Window::DrawC61200 | In outline: clears a flag (Flags::Bit0) and draws in black (`0xC5DAE0` with 0xFF000000) |
| `0x6b37a0` | Window::Make6B37A0 | In outline: makes a list member part (ListMember with `0x10EC9E8`) |
| `0xa5d3c0` | Text::NameA5D3C0 | In outline: fills a text's "%NAME%" marker (Texts::LookupA5C260) |
| `0x9db2d0` | Text::Name9DB2D0 | In outline: fills a text's "%NAME%" marker (Window::Shared9DA260) |
| `0x64b060` | Window::List64B060 | In outline: goes over a list (Window::List64AEE0, `0x7E3AC0`) |
| `0x61fc00` | Window::Activate61FC00 | In outline: activates a part and acts on a negative value |
| `0x683420` | Window::Keys683420 | In outline: sets key combinations (KeyCombination::Set) |
| `0x7eead0` | Info::Event7EEAD0 | In outline: for event 0x34426822, makes info texts (Info::Texts7EB930) |
| `0x7e2870` | Window::Case7E2870 | In outline: a value by a part's value (+0x114 +0x440) |
| `0x6c81b0` | Window::Random6C81B0 | In outline: a random step with a positive value (+0xC) and entries (+0xC4) |
| `0xc5f130` | Window::HitC5F130 | In outline: a hit test (Rect::Contains) |
| `0x6b0540` | Window::Make6B0540 | In outline: makes a part when not flagged (+4) |
| `0x682f30` | Window::Round682F30 | In outline: a value rounded when not flagged (+0x269) |
| `0x670ab0` | WindowList::Make670AB0 | In outline: makes a window list (WindowList) |
| `0x441960` | Window::Set441960 | In outline: sets a part (+0x58) when it changes |
| `0x6c9b80` | ListWindow::Fill6C9B80 | In outline: sets a list window's count (ListWindow::SetCount from `0x6C80C0`) |
| `0xc62bd0` | Window::ParentC62BD0 | In outline: a step for a window and its parent |
| `0xc623f0` | Window::FlagsC623F0 | In outline: a step (`0xC622D0`) when both low flags are set |
| `0x6e9d70` | Window::Make6E9D70 | In outline: makes a list member part (Listener440, ListMember with `0x10EC9E4`) |
| `0x615dc0` | Window::Place615DC0 | In outline: places a part (Window::SetRelationAndLayout with 6) and acts when not flagged (+0xE0) |
| `0x9bcf80` | Window::Update9BCF80 | In outline: an update when not flagged (+0x38) and flagged (+0x49) |
| `0x6adcc0` | Window::Loop6ADCC0 | In outline: a loop |
| `0xa1e530` | Window::LoopA1E530 | In outline: when on (+0xA4) with a part (+0x84), goes over a list |
| `0xa1e410` | Window::LoopA1E410 | In outline: the same (another copy) |
| `0x9ebb50` | Window::Make9EBB50 | In outline: makes a list member part (ListMember with `0x10EC9E4`, Listener440) |
| `0x7c8870` | Info::Text7C8870 | In outline: makes an info text from a shared one (`0xF31F48`) and a wide string |
| `0x6a7df0` | Window::Mode6A7DF0 | In outline: a step in mode 1 |
| `0x6fdf10` | Window::List6FDF10 | In outline: goes over a linked list (+0x4C) and acts on each entry by its kind (+0x14) |
| `0x9ca180` | Window::Add9CA180 | In outline: grows the list at +0xCC (`0x9C6490`) and puts its argument last, grows +0x58 (Array::Resize5501E0) and makes a handle |
| `0x7af8a0` | Window::Text7AF8A0 | In outline: when not flagged (+0x4C), a text from its part (+0x38 +0xC) |
| `0x9b3a30` | Window::Bind9B3A30 | Sets the object it shows (+0xF0) and, when there is one, binds two number fields to its floats (+0x130, +0x134) and a check box to a flag (+0x198) |
| `0x9b37b0` | Window::Bind9B37B0 | Sets the object it shows (+0xD4) and, when there is one, binds three number fields to its floats (+0x88, +0x8C, +0x90) and a check box to a flag (+0x84) |
| `0x70e5b0` | Window::Event70E5B0 | In outline: for 0x10 clears a part's value (+0x40 +0x124); for 7 acts with the shared value (`0x1306194`) |
| `0x6fdd50` | Window::Nth6FDD50 | In outline: walks n entries along the linked list at +0x4C |
| `0x6b6100` | Window::Scale6B6100 | In outline: a point scaled to a window's width and height (+0x84 minus +0x7C, +0x88 minus +0x80) |
| `0x6dde60` | Objectives::CountText6DDE60 | In outline: a condition's name with its count (+0x70) after it (": %d") |
| `0x6ddb30` | Objectives::CountText6DDB30 | In outline: the same (another copy) |
| `0x6dd800` | Objectives::CountText6DD800 | In outline: the same (another copy) |
| `0x6a7600` | Window::First6A7600 | In outline: finds the first of 12 parts (+0x34 on) whose flag is set (+0x94 bit 5) |
| `0x66cce0` | Window::Clear66CCE0 | In outline: frees its buffer (+0xDC), clears +0xDC and +0xE0, and more (`0x12F4B34`) |
| `0x9a2880` | MixMaster::Layout | In outline: lays out the window (Window::Layout99C500), refreshes (`0x99FE70`) and, when not scrubbing, stops scrubbing (MixMaster::NoScrub) |
| `0x6db1e0` | Window::Place6DB1E0 | In outline: refreshes (`0x6D4870`) and places parts beside each other by their widths |
| `0xa2aa30` | Message::ResultA2AA30 | In outline: makes a message result (Message::ResultA2A440) from a text (`0x495BF0`) |
| `0x9830a0` | Window::Make9830A0 | In outline: sets up a window part and clears +0x38, +0x3C, +0x40 |
| `0x6b3390` | WindowList::Make6B3390 | In outline: makes a window list (WindowList) |
| `0x690b70` | Window::Screen690B70 | In outline: works out a window's rectangle and a point on the screen (Windows rectangle and point calls) |
| `0x67ade0` | Window::Shrink67ADE0 | In outline: moves a value (+0xC) toward a limit (+0x10) by a time step, stopping there |
| `0x68b570` | Window::Rows68B570 | In outline: a step over rows, with none (+0xD8) handled |
| `0x61b970` | Window::Help61B970 | In outline: builds a help button (HelpButton::Build2) and a named text (`0x1303CFC`) |
| `0x78b340` | Window::Draw78B340 | In outline: draws a part in grey (0xFFD6D6D6) |
| `0x9aa0a0` | Window::Place9AA0A0 | In outline: gets its rectangle and sets it 10 wide (Window::SetRectAndRefresh) |
| `0x9ecb30` | Window::Make9ECB30 | In outline: makes a part from its argument, or the shared one (`0x1341C68`) when none |
| `0xa2c2f0` | Window::MakeA2C2F0 | In outline: a message result (Message::ResultA2AF70) and a 0xC0-byte part when there is none |
| `0x6ef2f0` | Window::Flag6EF2F0 | In outline: when flagged (bit 1 of -0x40), makes a part |
| `0x6931f0` | Array::Resize6931F0 | In outline: resizes a list of 0x24-byte entries to n |
| `0x49e530` | NamedObject::Make49E530 | In outline: makes a list member object (ListMember with `0x10EC9E8`) with its tables (`0xF305C4`, `0xF305BC`) |
| `0x6c6c10` | ListWindow::Fill6C6C10 | In outline: sets a list window's count (ListWindow::SetCount from `0x6C8110`) and fills it |
| `0x6879a0` | Folder::Find6879A0 | In outline: under a count (+0x190 vs +0x2D0), finds folders by name (Folder::Find687720) |
| `0x70b750` | Window::Key70B750 | In outline: a step unless key 0x20 is held (KeyState::Held) |
| `0x6b2440` | Window::Make6B2440 | In outline: makes a window part (`0x685070`) |
| `0x981dd0` | Window::Copy981DD0 | In outline: copies a value (+0x44) and a linked part (+0x1C) |
| `0xaa5b70` | Window::LayoutAA5B70 | In outline: unless a shared value is set (`0x10EDCC0`), lays out a window 30 high |
| `0xa2b830` | Window::SharedA2B830 | In outline: a step with the shared object's list (`0x12F4E4C` +0x64) |
| `0x984100` | Research::CategoryInstance | In outline: makes a research category (ResearchCategoryInstance) |
| `0x634d90` | Window::Ref634D90 | In outline: a step with its referenced object (Window::Slot48Self, Reference::ToObject on +0x14C) |
| `0x9b93a0` | Window::Event9B93A0 | In outline: for event 0x13 only, a step with the shared object's part (+0x100 +0x278) |
| `0x6f1280` | Window::Make6F1280 | In outline: with no part (+0xA4), makes a 0x160-byte one |
| `0x44a560` | NamedObject::Make44A560 | In outline: makes a list member object (ListMember with `0x10EC9E8`, Listener440) |
| `0x6663d0` | List::Visible6663D0 | In outline: a step over entries (+0x178) in a range (+0x17C, +0x180) |
| `0x68d640` | Window::Scroll68D640 | In outline: with a part (+0xE0), moves its value (+0x128) by a change |
| `0x9bb8e0` | Window::Set9BB8E0 | In outline: sets a part (+0xFC) and its slot (+0x100) from the shared id (`0x1341508`) |
| `0x68d090` | Window::Layout68D090 | In outline: with a part (+0xEC), lays it out by the window's size |
| `0x6acc60` | Screen::Scale6ACC60 | In outline: a value scaled by the screen's area (Screen::AreaScale), rounded |
| `0x6a4820` | Window::Layout6A4820 | In outline: works out a part's place from the window's size and two values (+0xE4, +0xE8) |
| `0x479b20` | Text::Slot479B20 | In outline: a text for 0 to 0x1D when two shared values differ (`0x10ED860`, `0x10ED864`) |
| `0xc67600` | Window::AddChild | In outline: sets up a child (`0xC675A0`), points it at its parent (+0x18) and tells it (slot 0x40) |
| `0xa20aa0` | ListWindow::FillA20AA0 | In outline: clears a list window (ListWindow::SetCount(0)) and fills it from its part (+0xF0) |
| `0x9d5b20` | NamedObject::Make9D5B20 | In outline: makes a list member object (ListMember with `0x10EC9E4`) with its table (`0xF850AC`) |
| `0x620c20` | Window::Text620C20 | In outline: a text from a message (Message::Result4040F0 with `0x1303CFC`) |
| `0xc5eb00` | Window::HitC5EB00 | In outline: gives 1 when a check passes (slot 0xC) |
| `0xc5f260` | Window::HitC5F260 | In outline: the same with slot 0x24 |
| `0xc5f020` | Window::HitC5F020 | In outline: the same with slot 0x14 |
| `0xbd7f30` | Device::RectBD7F30 | In outline: a rectangle with its ends in order |
| `0xc60080` | Window::ShowC60080 | In outline: when a flag changes (+0x105 vs +0x104), acts by it |
| `0xc5f410` | Window::HitC5F410 | In outline: a hit test against its rectangle (slot 0x70, Rect::Contains) |
| `0xa532a0` | Window::CountA532A0 | In outline: counts up through entries |
| `0x6e9ec0` | Window::Case6E9EC0 | In outline: a step by case (1: 0x24, ...) |
| `0x694fb0` | Layout::Draw694FB0 | In outline: a draw step with the layout table (`0x10EEDB4`, `0x12F84D0`) |
| `0xc62510` | Window::DrawC62510 | In outline: a draw step with the window system's device (`0x13E6D00` +0x108) |
| `0x615870` | Window::Place615870 | In outline: places a part from another part's rectangle (+0xD4) |
| `0xc5ed20` | Window::HitC5ED20 | In outline: gives 1 when a check passes (slot 0x1C) |
| `0xc5ef20` | Window::HitC5EF20 | In outline: gives 1 when a check passes (slot 0x10) |
| `0xc5ee20` | Window::HitC5EE20 | In outline: gives 1 when a check passes (slot 0x2C) |
| `0x544cf0` | Window::Text544CF0 | In outline: a text from its part (+0xF0, slot 0x188) |
| `0x9bd260` | Window::Update9BD260 | In outline: an update when not flagged (+0x38) |
| `0x7efb60` | Staff::LoadUniforms | In outline: loads from the "\Uniforms" folder |
| `0xa07e00` | Window::MakeA07E00 | In outline: unless the shared value is set (`0x12F8084`), makes a 0x54-byte part |
| `0x678190` | Window::Step678190 | In outline: a step with its list's part (+0xF0, slot 0xF8) |
| `0x49d480` | Window::Number49D480 | In outline: unless the shared value is set (`0x1305DD8`), a step with a window (`0x6C1DA0`, `0x6BE4C0`) |
| `0xc629e0` | Window::RectC629E0 | In outline: unless flagged (bit 0x80000 of +0x1C), sets its rectangle (slot 0x70, Rect::Assign) |
| `0x99c1a0` | Window::Fits99C1A0 | In outline: a step when rows (+0x108) times a height fit the window |
| `0x6ffc30` | Window::Mouse6FFC30 | In outline: with a part (+0x11C), a screen step with the GUI root's size |
| `0x6c6050` | Window::Name6C6050 | In outline: makes a part (+0x88) and a named text (`0x1303CFC`) |
| `0x6c5f50` | Window::Name6C5F50 | In outline: the same with +0x84 |
| `0x6c5e50` | Window::Name6C5E50 | In outline: the same with +0x80 |
| `0x6c5d50` | Window::Name6C5D50 | In outline: the same with +0x7C |
| `0x6b21b0` | Window::Make6B21B0 | In outline: makes a window part (`0x685070` with 1.0) |
| `0x6b1f20` | Window::Make6B1F20 | In outline: the same (another copy) |
| `0x9c2360` | Window::Bind9C2360 | In outline: sets the object it shows (+0xDC) and binds fields to it (Window::Set10C with +0x7C, +0x80, ...) |
| `0x69b4a0` | Input::ImeInit | In outline: sets up text input: its table (`0xF2DFD4`), the window's input context and the keyboard layout list |
| `0x610d30` | GameOptions::Units | In outline: builds the game options units (GameOptionsDialog::BuildUnits) and, when flagged, lays out and refreshes |
| `0xa57210` | Campaign::ProfileText | In outline: a text with the campaign profile name (Campaign::ProfileName) |
| `0xc64d40` | Window::LoopC64D40 | In outline: clears a flag (+0x9C) and goes over its parent's list (+0xF0 +0xB0) |
| `0x479c30` | Text::Slot479C30 | In outline: a text for 0 to 0x50 when its shared flags agree (`0x10EDAB8`) |
| `0x479a20` | Text::Slot479A20 | In outline: a text for 0 to 0x18 when two shared values differ (`0x10ED668`) |
| `0xb00d50` | Window::Screen | In outline: a screen step with the GUI root's size |
| `0x65d340` | Window::Set65D340 | In outline: when given a part, sets it (+0xF4) and goes over its entries |
| `0x6206f0` | Window::Make6206F0 | In outline: makes a 0x90-byte part |
| `0x9df140` | Window::Changed9DF140 | In outline: when the shared value (`0x1341868`) changes, takes it and sets a flag (+8) |
| `0x623db0` | Window::Place623DB0 | In outline: with a part (+0xDC), places it (Window::SetRelation with 6) when not flagged |
| `0x48e380` | Window::Rect48E380 | In outline: a step with its rectangle (slot 0x20) |
| `0x68b470` | Window::Draw68B470 | In outline: draws a window's frame (Window::DrawFrame6ACE50) and more |
| `0xc5fc40` | Window::HitC5FC40 | In outline: a hit test with points |
| `0x97cd70` | Window::List97CD70 | In outline: a step with a list (+0x16C) |
| `0x8cd630` | Window::Place8CD630 | In outline: with a part (+0xE0), places it (Window::SetRelation with 6) when not flagged |
| `0x70e4c0` | Window::Release70E4C0 | In outline: sends a release (Window::SendRelease) and acts with the shared value (`0x1306194`) |
| `0x642740` | Window::Rect642740 | In outline: when not flagged (+0x152), a step with its rectangle |
| `0x9dd8c0` | Window::Text9DD8C0 | In outline: a text by a shared object's value (`0x134180C` +0x14 = 4) |
| `0x662fe0` | Window::Handle662FE0 | In outline: makes a handle and links it |
| `0x6f2740` | Window::Check6F2740 | In outline: a check when not flagged (+0x9C) |
| `0x6adb60` | Window::Place6ADB60 | In outline: a point from its place (+0x7C), or its parent's when it has one (+0x3C) |
| `0xa56900` | Window::StepA56900 | In outline: with a part (+0xE8), a step (`0xA561B0`) |
| `0x690f00` | Text::Layout690F00 | In outline: a text layout step with sizes |
| `0x65ae80` | Window::Value65AE80 | In outline: a value (+0x170, or +0x168 when negative) |
| `0x63d300` | Window::Width63D300 | In outline: a step with a part's width |
| `0xa7c8c0` | Window::TextA7C8C0 | In outline: a text from shared values |
| `0x9d5de0` | Window::Make9D5DE0 | In outline: fills a record from its arguments |
| `0x99def0` | Window::Step99DEF0 | In outline: a step with an object (`0x1341524`) |
| `0xa2e720` | Window::UpdateA2E720 | In outline: an update when its part is not flagged (+0x30) |
| `0x63a5b0` | Window::Make63A5B0 | In outline: sets up a record with its tables (`0xF3A07C`) |
| `0x61bd50` | Window::Make61BD50 | In outline: makes a 0x268-byte part |
| `0x6a2f80` | Window::Scroll6A2F80 | In outline: unless flagged, a fraction from a value (slot 0x100) |
| `0x6b38f0` | Window::List6B38F0 | In outline: goes over a list (+0x20) |
| `0x9bd170` | Window::Update9BD170 | In outline: a step for a part not flagged (+0x1C) |
| `0xc62ae0` | Window::HitC62AE0 | In outline: a hit test (`0xC5E300`) |
| `0x67f150` | Window::Shared67F150 | In outline: with a part (+0x130), a step with the shared object (`0x1106C54`) |
| `0x6b5fb0` | Window::Slot6B5FB0 | In outline: a step for an object with a slot (`0x130A8A0`) |
| `0x6ade60` | Window::Flag6ADE60 | In outline: a check unless flagged (+0x94 bit 6) |
| `0x639680` | Window::Table639680 | In outline: a step with a table entry (`0xF488B4`, 0x8C bytes) and the shared value (`0x12F4618`) |
| `0x61f3f0` | MessageConsole::Show | In outline: makes the message console (MessageConsole::Make) and acts by a value against a limit |
| `0x9a02f0` | Window::Build9A02F0 | In outline: builds a window from the shared title `0x1305CC4` |
| `0x9a00a0` | Window::Build9A00A0 | In outline: the same (another copy) |
| `0x98b9f0` | Window::Set98B9F0 | In outline: sets values (+0x44, +0x14) and a handle |
| `0x61c0c0` | Window::Place61C0C0 | In outline: places a part by the window's height |
| `0x4780a0` | NamedObject::Make4780A0 | In outline: makes a list member object (ListMember with `0x10EC9E8`) |
| `0x98b280` | Window::Check98B280 | In outline: a check with a part (+4 +0xC) |
| `0x700060` | Window::Place700060 | In outline: a step with the window's height |
| `0x6be4c0` | Money::Text6BE4C0 | In outline: a money text with two decimals (Money::Format_(2)), with zero handled |
| `0x476950` | Effects::Loop476950 | In outline: goes over the shared list (`0x10ECA38`) |
| `0x6a49d0` | Window::Place6A49D0 | In outline: places a part by the window's size unless flagged (bit 22) |
| `0xa550e0` | Window::FreeA550E0 | In outline: unless the shared value is set (`0x1342958`), frees the shared buffer (`0x1342954`) |
| `0x69f420` | Window::Text69F420 | In outline: a text (`0x53B850`) with its part (+0x314) |
| `0xc63570` | Window::FocusC63570 | In outline: when its check passes, tells the window system (`0x13E6D00`, slot 0xF4) |
| `0x6af460` | Window::Root6AF460 | In outline: with no window, uses the GUI root's part (`0x12F83F0` +0xDC) |
| `0xac7a40` | Window::DrawAC7A40 | In outline: draws a window's frame (Window::DrawFrame6ACE50) and more when not flagged (+0xF4) |
| `0x9b3550` | Window::Bind9B3550 | In outline: sets the object it shows (+0xD4); with none, clears a number field's binding |
| `0x661c50` | Window::Check661C50 | In outline: clears a value (+0xE4) and a check with its part (+0xE0, slot 0x148) |
| `0x68d1d0` | Window::SetCount68D1D0 | In outline: sets a count (+0xEC) when it changes, freeing its buffer (+0xE8) at 0 |
| `0x9be500` | ParticleDesigner::Tree | In outline: with a part (+0x140), builds the particle designer tree (ParticleDesigner::BuildTree) |
| `0xa2c580` | Window::TextA2C580 | In outline: a text from entries (+0x1C) |
| `0x9a7f80` | Window::Handle9A7F80 | In outline: makes a handle and links it |
| `0x6c3d50` | Window::Value6C3D50 | In outline: a value, reversed in mode 5 (+0x44) |
| `0x42b9b0` | NamedObject::Make42B9B0 | In outline: makes a list member object (Listener440, ListMember with `0x10EC9E8`) |
| `0x48e000` | NamedObject::Make48E000 | In outline: makes a list member object (ListMember with `0x10EC9E8`) |
| `0xb5ca50` | NamedObject::MakeB5CA50 | In outline: makes a list member object (ListMember with `0x10EC9E4`) |
| `0x9c0d30` | Window::Source9C0D30 | In outline: makes a particle source, refreshes (Window::Refresh9D5510) and acts |
| `0x6c3570` | Window::Countdown6C3570 | In outline: counts a shared value down (`0x1305DE8`) and at 0 with another set makes a 0x120-byte part |
| `0x467950` | Window::Screen467950 | In outline: a step with the GUI root's size |
| `0xc639e0` | Window::HitC639E0 | In outline: a hit test with points |
| `0x7f0440` | Window::Place7F0440 | In outline: places a part at the bottom of the window |
| `0x68ee40` | Window::Text68EE40 | In outline: a step with a text at least 4 long |
| `0x6a21c0` | Window::Release6A21C0 | In outline: plays the button release sound (Sound::GUIBRelease) and acts with its part (+0xD4) |
| `0x6a20e0` | Window::Release6A20E0 | In outline: the same (another copy) |
| `0xa2acc0` | Window::TextA2ACC0 | In outline: a text from entries (+0x1C) with a shared one (`0xF89274`) |
| `0x6abc90` | Window::Flag6ABC90 | In outline: with a part (+0x13C), sets or clears a flag (bit 24 of +0x94) |
| `0x69b160` | Input::ImeStep | In outline: a step with the window's input context (+0x14) |
| `0x483ad0` | Labels::Set483AD0 | In outline: when a value changes (+0xC) and on, a step with a screen point |
| `0x6620e0` | Window::Text6620E0 | In outline: a text when flagged (+0x58) and not flagged (+0x54) |
| `0x6202d0` | Window::Make6202D0 | In outline: with the shared value (`0x10EDD18`), makes a 0x70-byte part |
| `0x67e960` | Window::Scale67E960 | In outline: a fraction from part n's width |
| `0x67af00` | Window::Make67AF00 | In outline: makes a part with a value (+0xE8) |
| `0x6fd190` | Window::Nth6FD190 | In outline: walks to entry n (+0x40) of a list (+0x4C) |
| `0x6b0d50` | Window::SetParent6B0D50 | In outline: sets a part (+0x3C) when it changes, by a flag (bit 2) |
| `0x6644d0` | Window::Make6644D0 | In outline: builds a window (Window::Build83D730) with its tables (`0xF4E628`, `0xF4E614`) |
| `0x61b8a0` | Window::Values61B8A0 | In outline: sets a window's values (Window::SetRelatedValue, Window::SetValueEC) and a step (`0x67A7A0`) |
| `0xc66100` | Window::LinkC66100 | In outline: a step with a linked part (+0x74) |
| `0x9c1a20` | Window::Check9C1A20 | In outline: a check when not flagged (+0x128) |
| `0xa431b0` | Window::CaseA431B0 | In outline: unless given its own part, clears a value (+0xC4) and acts by a mode (+0xC0) |
| `0x6ef5d0` | Window::Count6EF5D0 | In outline: counts up entry n (+8) and acts by n, making parts and texts |
| `0x605980` | Options::Camera605980 | In outline: unless told not to, saves an option (Options::SetInt), a camera step, sets a list's selection from the camera (+0x3B4) and lays out |
| `0x6149d0` | Window::Build6149D0 | In outline: builds a window from the shared title `0x12F5740` |
| `0x679450` | Window::Place679450 | In outline: places a window by its corner flags (+0xF4 bits 2 and 3) |
| `0xa40c80` | Window::LayoutA40C80 | In outline: a large layout: sets a text, a shared width (`0x12F9570`) and places parts |
| `0x62ca40` | Window::Build62CA40 | In outline: builds a window from the shared title `0x12F7E08` |
| `0x70bad0` | Window::Layout70BAD0 | In outline: places a window's parts by its size (Window::UpdateRect) |
| `0x7f1280` | Toolbar::Button7F1280 | In outline: makes a 0x144-byte image button (Button::SetImages with five images) and places it |
| `0x6d4c10` | Window::Build6D4C10 | In outline: builds a window from the shared title `0x1305F34` |
| `0x6820f0` | Window::Animate6820F0 | In outline: steps an animation time (+0x278) and goes over queued entries (+0x294) |
| `0x83bd00` | Window::Build83BD00 | In outline: builds a window from the shared title `0x130B630` |
| `0x6846f0` | Window::Build6846F0 | In outline: builds a window from the shared title `0x12F8428` |
| `0x9aadd0` | Window::Build9AADD0 | In outline: builds a window from the shared title `0x1341424` |
| `0x4d6730` | Window::Build4D6730 | In outline: builds a titled window (Window::BuildTitled) with a list member and its tables (`0xF33950`) |
| `0x4d7480` | Window::Build4D7480 | In outline: builds a window from the shared title `0x12F4F00` |
| `0x9be000` | Window::Layout9BE000 | In outline: places parts by the window's size, by a flag (+0x148) |
| `0x4bd4e0` | Window::Build4BD4E0 | In outline: builds a window from the shared title `0x12F4BD8` |
| `0x9b4720` | Window::Layout9B4720 | In outline: places two parts centred by their sizes |
| `0x67be80` | Window::Build67BE80 | In outline: builds a window from the shared title `0x12F8208` |
| `0x9b5140` | Window::Layout9B5140 | In outline: the same with other parts |
| `0x9b56a0` | Window::Layout9B56A0 | In outline: the same with other parts |
| `0x9b6320` | Window::Layout9B6320 | In outline: the same with other parts |
| `0x6afc20` | Window::Refresh6AFC20 | In outline: once (+0x94 bit 0), checks a flag (Window::Flag6ADE60) and acts with its parent |
| `0x440bf0` | Effect::Set440BF0 | In outline: sets a value (+0x70) when it changes, under 0x14 |
| `0x620470` | Window::Make620470 | In outline: makes a 0xA8-byte part |
| `0x6b2720` | Window::Fill6B2720 | In outline: fills a record from its arguments and the shared value (`0x1305D58`) |
| `0x67a7a0` | Window::Show67A7A0 | In outline: sets or clears a flag (bit 24 of +0x94) and lays out with a part (+0xE8) |
| `0x67b3c0` | Window::SetContent67B3C0 | In outline: sets a part (+0x11C) and its content (Window::SetContent) unless flagged (bit 22) |
| `0x640dd0` | Window::Desc640DD0 | In outline: with no part, makes a GUI description (GUI2_DatDesc) |
| `0xaf1f30` | Window::NameAF1F30 | In outline: makes a part (+0x80) and a named text (`0x1303CFC`) |
| `0x63aab0` | Window::Height63AAB0 | In outline: keeps a height of at least 131 and lays out |
| `0x6a65b0` | Window::Take6A65B0 | In outline: takes a part (+0x28) and acts when on (+0x24) |
| `0xa2a8e0` | Window::TextA2A8E0 | In outline: makes a part (+0x40) and a text (`0x495BF0`) |
| `0xa451d0` | Window::ScaleA451D0 | In outline: a value from n by case, with fixed scales and offsets |
| `0x6f0d80` | Window::Flags6F0D80 | In outline: 0 unless flagged (+0xD0); else a step over flags from 0x80 |
| `0x6ae070` | Window::Title6AE070 | In outline: a title text from a message (Message::Result4040F0) |
| `0x79a090` | Window::Check79A090 | In outline: a check when not flagged (+0x4D8) |
| `0x67a6b0` | Window::Hit67A6B0 | In outline: whether a point lies inside its rectangle (Window::Place6ADB60, Rect::Contains) |
| `0x6a22a0` | Window::Set6A22A0 | In outline: sets a part (+0x13C) when it changes, letting go of the old one |
| `0x981700` | Window::Check981700 | In outline: with a slot (+0x44), 1 when both flags are set (+0x4D, +0x4E) |
| `0x67b820` | Window::Flag67B820 | In outline: a flag step (Window::Flag67A410) and lays out by its part's flag |
| `0xa5dc30` | Window::Six | In outline: makes its list six long (Array::ResizeA5D5E0(6)) and fills new entries |
| `0xa80290` | Window::GetA80290 | In outline: when not flagged (+0x154), takes a value (`0x12F4B34`) into +0x150 |
| `0xae7c50` | Window::TextAE7C50 | In outline: makes an empty text record |
| `0x9825c0` | Window::Copy9825C0 | In outline: copies parts (Window::Copy981DD0) from a list and their flags |
| `0x6e6830` | Window::Make6E6830 | In outline: makes a part (`0x6C3BE0`) |
| `0xc69f00` | Window::AddChildC69F00 | In outline: clears a flag (+0x9C), sets up a child, points it at its parent and tells it |
| `0x6d5740` | Window::Make6D5740 | In outline: makes a part unless a check passes (`0x6E9890`) |
| `0xa45670` | Window::ArcA45670 | In outline: a point on a circle (cosine and sine of an angle) scaled by the window's height |
| `0x9dac40` | Window::Match9DAC40 | In outline: whether a part matches two values (3 matches any) |
| `0x67aca0` | Window::Children67ACA0 | In outline: a child step (Window::ChildrenSlot10) and a check unless flagged (bit 22) |
| `0xaf1eb0` | Window::TextAF1EB0 | In outline: a step (`0x9812F0`) and makes an empty text record |
| `0xa39f80` | Window::TextA39F80 | In outline: the same (another copy) |
| `0x98c730` | Fireworks::Flag98C730 | In outline: sets or clears a flag (bit 22) by whether there are fireworks (Fireworks::Count98AEC0) |
| `0x67a5b0` | Window::Release67A5B0 | In outline: when a press matches, plays the button release sound (Sound::GUIBRelease) and a check |
| `0x67a630` | Window::Release67A630 | In outline: the same (another copy) |
| `0x98f8f0` | Fireworks::Window98F8F0 | In outline: with fireworks (Fireworks::Count98AEC0), makes a 0x2C8-byte part |
| `0x67ac20` | Window::Children67AC20 | In outline: a child step (Window::ChildrenSlot9) and a check unless flagged |
| `0x984530` | Window::Text984530 | In outline: a text from its part (+0x78 +8), empty when none |
| `0x61cb70` | Window::Text61CB70 | In outline: a text from a part (+0x48), empty when none |
| `0x6542d0` | Window::Make6542D0 | In outline: a step (Window::Slot48Self) and makes a 0x180-byte part |
| `0x757580` | Entertainer::CostumeMenu | In outline: adds the entertainer costume menu (Entertainer::AddCostumeMenu) and sets a window's values |
| `0xa30200` | Window::PartA30200 | In outline: a step with a part (by a shared id `0x1342780`) |
| `0x672160` | Window::Find672160 | In outline: a find over entries (+0x148, +0x14C) |
| `0x6e99b0` | Window::Slot6E99B0 | In outline: a step for slot (n + m) of 32 bytes |
| `0xacafe0` | Graph::UpdateACAFE0 | In outline: an update when not flagged (+0x100) |
| `0xac4e20` | Window::LayoutAC4E20 | In outline: keeps a height (Window::Height63AAB0) and goes over a list (+0xE0) |
| `0x682c00` | Window::Countdown682C00 | In outline: counts a time (+0x2C) down by a step and acts |
| `0x65bc10` | Window::Place65BC10 | In outline: places a part 18 high, by a width when it has no part (+0xF0) |
| `0x643d20` | Window::Make643D20 | In outline: with a part (+0x124), makes a 0xC-byte record |
| `0x4ccc10` | NamedObject::Make4CCC10 | In outline: makes a list member object with one of two tables (`0x10EC9E8`, `0x10EC9EC`) |
| `0xa58c10` | Window::MakeA58C10 | In outline: a step (`0x7FAF90` with +0x174) and makes a part |
| `0x9f7240` | NamedObject::Make9F7240 | In outline: makes a list member object (ListMember with `0x10EC9E8`) |
| `0x9a62b0` | Window::Make9A62B0 | In outline: makes a part |
| `0xbaa4f0` | Window::ResetBAA4F0 | In outline: takes a part (Window::Take6A65B0), clears a flag (+0x145) and releases a part (+0x130) |
| `0xa26780` | Window::TextA26780 | In outline: a step (`0x6B1670`) and a text (`0xA261B0`) |
| `0x98f970` | Window::Entry98F970 | In outline: a step for entry n (0x28 bytes at +0x40) not flagged |
| `0x75a6a0` | Window::Text75A6A0 | In outline: for 0, a text from a part (+0x88) |
| `0x75a5d0` | Window::Text75A5D0 | In outline: for 0, a text from a part (+0x84) |
| `0x700180` | Window::Set700180 | In outline: sets a value (+0x18C) and makes a 0x268-byte part |
| `0x6742e0` | Window::Reset6742E0 | In outline: with a part (+0x120), clears a shared value (`0x10EED3C`) and goes over entries |
| `0x6278e0` | Window::Root6278E0 | In outline: a step with the GUI root |
| `0x6190b0` | Window::Make6190B0 | In outline: makes a part with a value (+0x70) |
| `0x97c980` | Window::Layout97C980 | In outline: places a window (Window::UpdateRect) |
| `0x659d10` | Window::Loop659D10 | In outline: a loop (Window::Loop6ADCC0 with 4.0) and a value kept above a limit |
| `0x65eb50` | Window::Point65EB50 | In outline: a step with a point (`0x624770`) |
| `0xa25f10` | Window::LayoutA25F10 | In outline: places a window (Window::UpdateRect) |
| `0x981b10` | Window::Step981B10 | In outline: when a check passes (Window::Check981700), a step (`0x9814B0`) |
| `0x6ad8f0` | Window::Flags6AD8F0 | In outline: a step by flags (bits 5, 9, 10) |
| `0x679df0` | Window::Text679DF0 | In outline: makes an empty text and a 0xC-byte record |
| `0x6a8400` | Window::Mode6A8400 | In outline: a step in mode 1 (+0xF0) with a part (+0xF4) |
| `0x6728b0` | Window::Link6728B0 | In outline: a step with a linked part (+0xF8) |
| `0x9aabb0` | Window::Place9AABB0 | In outline: with a part (+0xD8), places it 30 wide |
| `0xa54c90` | Window::MakeA54C90 | In outline: makes a window part |
| `0xa54ba0` | Window::MakeA54BA0 | In outline: the same (another copy) |
| `0x9776a0` | Window::Make9776A0 | In outline: makes a window part |
| `0x70e6d0` | Window::Shared70E6D0 | In outline: a step with the shared value (`0x1306194`) |
| `0x660d50` | List::Grow660D50 | In outline: grows a list when n passes its count (+0x178) (Array::Resize460F20) |
| `0x645620` | Window::Part645620 | In outline: a step with a part (+0x140) |
| `0x9f67c0` | Window::Make9F67C0 | In outline: makes a 0x268-byte part |
| `0x9b3fb0` | Window::Bind9B3FB0 | In outline: sets the object it shows (+0xD4) and binds fields to it |
| `0x979c30` | Window::Make979C30 | In outline: makes a window part |
| `0x667480` | Window::Width667480 | In outline: a step with a part's width |
| `0x63b1a0` | Window::Place63B1A0 | In outline: when not flagged (+0xD4), places a part |
| `0x5fca00` | Window::Flag5FCA00 | In outline: sets a flag (Window::Flag6ABC90) and acts |
| `0x6a5d10` | Window::Make6A5D10 | In outline: makes a window part |
| `0x65aab0` | Window::Mode65AAB0 | In outline: a step by a mode (+0x168) |
| `0x655360` | Window::Hit655360 | In outline: a step when a point lies inside (Rect::Contains) |
| `0x6203a0` | Window::Make6203A0 | In outline: makes a 0x70-byte part |
| `0xa5b830` | Window::MakeA5B830 | In outline: makes a 0x78-byte part |
| `0x9daf50` | Window::Make9DAF50 | In outline: the same (another copy) |
| `0x9bde70` | Window::Name9BDE70 | In outline: with nothing set (+0x2FC), sets a name |
| `0xa1ef90` | Window::PartA1EF90 | In outline: a step with a part (+0x84) |
| `0x9bd0a0` | Window::Check9BD0A0 | In outline: a check when not flagged (+0x38) |
| `0xac8e40` | Window::UpdateAC8E40 | In outline: an update when not flagged (+0x120) |
| `0xac7b20` | Graph::PassAC7B20 | In outline: goes over entries (+0xEC) |
| `0x6cc950` | Window::Set6CC950 | In outline: sets a value (+0xEC) and acts with a part (+0x3C) |
| `0x6a2930` | Window::Shared6A2930 | In outline: unless flagged (bit 0x20), a step with the shared value (`0x1305C40`) |
| `0x67c9f0` | Window::Set67C9F0 | In outline: sets a value (+0xF8) and makes a part |
| `0x66d100` | Window::Text66D100 | In outline: a text from a shared one (`0xF307E0`) |
| `0x64b580` | Window::Event64B580 | In outline: for event 0x31A955 only, a step |
| `0x6630d0` | Window::Handle6630D0 | In outline: makes a handle and links it |
| `0xa36bd0` | Window::SharedA36BD0 | In outline: when asked or with a part (+0x124), a step with the shared object's list (`0x12F4E4C`) |
| `0xaca140` | Graph::MakeACA140 | In outline: makes a part from its part's value (+0x3C +0x3C) |
| `0x9a6ed0` | Window::Reset9A6ED0 | In outline: clears a flag (+0x60) and acts when not flagged (+0x40) |
| `0x6ad9c0` | Window::Flags6AD9C0 | In outline: a step by flags (bits 6, 5) |
| `0x65adc0` | Window::Step65ADC0 | In outline: a step with a part (slot 0x108) |
| `0x671ca0` | Window::Part671CA0 | In outline: a step with a part (+0x130) |
| `0x6fef60` | Window::Clear6FEF60 | In outline: for 0, releases a part (+0xD8) |
| `0x64b320` | Window::Text64B320 | In outline: a text from a part (`0x783750` with +0x118) and a shared value (`0x1301EF0`) |
| `0xacd6c0` | Graph::PartACD6C0 | In outline: with a part (+0x10C), a step |
| `0xacaaa0` | Graph::MakeACAAA0 | In outline: makes a 0x268-byte part |
| `0xa53a90` | Window::FormatA53A90 | In outline: formats into a 32-character buffer |
| `0xa209e0` | Window::SetA209E0 | In outline: sets two values (+0xDC, +0xE0) and acts |
| `0x7006a0` | Window::Part7006A0 | In outline: with a part (+0x20), a step with a value (+0x30) |
| `0x9b3240` | Window::Bind9B3240 | In outline: sets the object it shows (+0xD4) and acts |
| `0x9b4190` | Window::Bind9B4190 | In outline: the same (another copy) |
| `0x6af600` | Window::Part6AF600 | In outline: a step with a part (+0x4C) |
| `0x6af540` | Window::Part6AF540 | In outline: a step with a part (+0x64) |
| `0x6927f0` | Text::Fit6927F0 | In outline: a check with a text's size |
| `0x9a9a00` | Window::Copy9A9A00 | In outline: copies a value (+0x7C into +0x114) and makes a part |
| `0x663560` | Window::Reset663560 | In outline: clears a flag (+0x4C) and acts when not flagged (+0x2C) |
| `0x661d30` | Window::Check661D30 | In outline: clears a value (+0xE4) and a check with a part (slot 0x14C) |
| `0x6b8b40` | Window::Point6B8B40 | In outline: a step with a point |
| `0xa26270` | Window::TextA26270 | In outline: makes a text |
| `0x9f3d00` | Window::Part9F3D00 | In outline: with a part (+0x50), a step |
| `0x9891e0` | Window::Part9891E0 | In outline: with a part (+0x114), a step (`0x475BA0`) |
| `0x6a9d30` | Window::Step6A9D30 | In outline: a step with a work buffer |
| `0x6f22f0` | Window::Part6F22F0 | In outline: with a part (+0xA0), a step when not flagged (+0x9C) |
| `0x670910` | Window::Make670910 | In outline: makes a 0xC-byte part |
| `0x65a1d0` | Window::Make65A1D0 | In outline: makes a 0x268-byte part |
| `0x643a10` | Window::Make643A10 | In outline: the same (another copy) |
| `0x5445e0` | Window::Children5445E0 | In outline: goes over children (Window::Children67AC20) and acts |
| `0x69ea20` | Window::Size69EA20 | In outline: a step with a part's size |
| `0x6dc010` | Window::Place6DC010 | In outline: a step (`0x6D4870`) and places by a part's value (+0xD4 +0x80) |
| `0x6db500` | Window::Place6DB500 | In outline: the same (another copy) |
| `0x6dab30` | Window::Place6DAB30 | In outline: the same (another copy) |
| `0xa261b0` | Window::TextA261B0 | In outline: makes a text |
| `0x99fdb0` | Window::Text99FDB0 | In outline: the same (another copy) |
| `0x99ad00` | Window::Text99AD00 | In outline: the same (another copy) |
| `0xae6d40` | Graph::ValueAE6D40 | In outline: a rounded value times a scale, with a part (+0xF0) |
| `0x681c00` | Window::Point681C00 | In outline: a step with a point |
| `0x6af2e0` | Window::Part6AF2E0 | In outline: a step with a part (+0xB8) |
| `0x6a1060` | Window::Check6A1060 | In outline: unless flagged (0x800), a step (slot 0xFC) |
| `0x6710b0` | Window::Clear6710B0 | In outline: empties a list (+0x40) |
| `0x6dbc90` | Window::Place6DBC90 | In outline: a step (`0x6D4870`) and places by a part's value (+0xD4 +0x80) |
| `0x6abbd0` | Window::Rect6ABBD0 | In outline: takes a rectangle from a part (+0xF0, slot 0x20) and acts |
| `0x67ec90` | Window::Rect67EC90 | In outline: the same (another copy) |
| `0xa58b60` | Window::SetA58B60 | In outline: sets a value (+0x170) and acts with a part (+0x174) |
| `0x683e60` | Window::Text683E60 | In outline: makes a text |
| `0x678560` | Window::Set678560 | In outline: sets a value (+0xF0) and makes a part |
| `0x65eaa0` | Window::Make65EAA0 | In outline: makes a 0x268-byte part |
| `0x6873d0` | Window::Text6873D0 | In outline: a text, with no part (+0x178) handled |
| `0x6c2140` | Window::Set6C2140 | In outline: sets a value (+0xF0) and acts |
| `0x69f370` | Window::Pass69F370 | In outline: goes over entries with a point |
| `0x6649b0` | Window::Make6649B0 | In outline: makes a 0xC-byte part |
| `0x6a1800` | Window::Key6A1800 | In outline: handles a key (Window::HandleKey6A1120) with a rounded value |
| `0x6d4870` | Window::Place6D4870 | In outline: lays out (Window::Layout6D4690) and places by a part's value (+0xD4 +0x80) |
| `0x6a0f40` | Window::Text6A0F40 | In outline: makes a text |
| `0xb01280` | Window::MakeB01280 | In outline: makes a 0x268-byte part |
| `0xa1e9b0` | Window::FindA1E9B0 | In outline: a step when a find passes (`0xA1ACF0` with +0x88) |
| `0xa1d400` | Window::InitA1D400 | In outline: sets up a record with empty texts (WideString::Empty) |
| `0xa58ce0` | Window::PassA58CE0 | In outline: goes over entries (+0x184) |
| `0x825530` | Window::Part825530 | In outline: a step (Window::Slot48Self) and makes a part when none (+0x140) |
| `0x7aed10` | Toolbar::Children7AED10 | In outline: a step with children (Window::ChildrenSlot2) and `0x8E5910` |
| `0x6714b0` | Window::Find6714B0 | In outline: a find with none handled |
| `0x63a0d0` | Gui::Make63A0D0 | In outline: makes a 0x20-byte part |
| `0x672080` | Gui::Pass672080 | In outline: goes over entries (+0x120) |
| `0x6dbe80` | Gui::Make6DBE80 | In outline: makes a 0x78-byte part |
| `0x6db880` | Gui::Make6DB880 | In outline: makes a 0x78-byte part |
| `0x6db3a0` | Gui::Make6DB3A0 | In outline: makes a 0x78-byte part |
| `0x6da3f0` | Gui::Make6DA3F0 | In outline: makes a 0x78-byte part |
| `0x6da280` | Gui::Make6DA280 | In outline: makes a 0x78-byte part |
| `0x6da110` | Gui::Make6DA110 | In outline: makes a 0x78-byte part |
| `0x6d9f40` | Gui::Make6D9F40 | In outline: makes a 0x78-byte part |
| `0x6d9d70` | Gui::Make6D9D70 | In outline: makes a 0x78-byte part |
| `0x6d9b90` | Gui::Make6D9B90 | In outline: makes a 0x78-byte part |
| `0x6d9a20` | Gui::Make6D9A20 | In outline: makes a 0x78-byte part |
| `0x6da560` | Gui::Make6DA560 | In outline: makes a 0x70-byte part |
| `0x683810` | Gui::Make683810 | In outline: makes a 0x268-byte part |
| `0x6838c0` | Window::Value6838C0 | In outline: with no part (+0x3C), a step with a value (+0xC4) |
| `0x6b97c0` | Window::Shared6B97C0 | In outline: once (`0x1305DB0`), makes a 0x164-byte part |
| `0x68f720` | Window::Point68F720 | In outline: a step with a point |
| `0x61ed30` | Window::Last61ED30 | In outline: a value from the last entry (+0x110, +0x114) |
| `0x7f09d0` | Person::Activate7F09D0 | In outline: activates a window (Window::Activate6A4260) and acts |
| `0x6d58d0` | Window::Pass6D58D0 | In outline: when not flagged (+0x40), goes over entries |
| `0x680d20` | Window::Draw680D20 | In outline: draws in black with a shared device (`0x1305CF0`, slot 0x3C) |
| `0x6808f0` | Window::Draw6808F0 | In outline: the same (another copy) |
| `0x6eab20` | Window::Rect6EAB20 | In outline: takes a rectangle from a part (+0xE4) and acts |
| `0x6bf900` | Window::Copy6BF900 | In outline: copies values from another |
| `0x64a320` | Window::Info64A320 | In outline: a step with an info text (Info::Text7C8700) |
| `0x5fc960` | Window::Flag5FC960 | In outline: sets a flag (Window::Flag6ABC90) and acts |
| `0x6ff1e0` | Window::Part6FF1E0 | In outline: with a part (+0xD8), a step |
| `0x6d92b0` | Window::Init6D92B0 | In outline: fills a record from an object and its arguments |
| `0x6d8da0` | Window::Init6D8DA0 | In outline: the same (another copy) |
| `0x6d8a50` | Window::Init6D8A50 | In outline: the same (another copy) |
| `0x6d8250` | Window::Init6D8250 | In outline: the same (another copy) |
| `0x6d80e0` | Window::Init6D80E0 | In outline: the same (another copy) |
| `0x6d7f70` | Window::Init6D7F70 | In outline: the same (another copy) |
| `0x6d7e10` | Window::Init6D7E10 | In outline: the same (another copy) |
| `0x6d7cb0` | Window::Init6D7CB0 | In outline: the same (another copy) |
| `0x6d7b50` | Window::Init6D7B50 | In outline: the same (another copy) |
| `0x6d79e0` | Window::Init6D79E0 | In outline: the same (another copy) |
| `0x6d83c0` | Window::Init6D83C0 | In outline: the same (another copy) |
| `0x679ec0` | Window::Empty679EC0 | In outline: empties a text and acts with a part (+0xFC) |
| `0x6c3440` | Gui::Make6C3440 | In outline: makes a 0x120-byte part |
| `0x67f0b0` | Gui::Make67F0B0 | In outline: makes a 0x174-byte part |
| `0x67d4b0` | Gui::Pass67D4B0 | In outline: goes over entries (+0x11C) |
| `0x6ecf90` | Gui::Make6ECF90 | In outline: makes a 0x18C-byte part |
| `0x6f5f30` | Window::Find6F5F30 | In outline: a find in a list |
| `0x6bf490` | Window::Find6BF490 | In outline: the same (another copy) |
| `0x68efa0` | Window::Find68EFA0 | In outline: the same (another copy) |
| `0x67aff0` | Window::Mode67AFF0 | In outline: in mode 2 (bits of +0xE4), a check (slot 0xFC) |
| `0x70ba30` | Window::Uncheck70BA30 | In outline: clears a value (+0x144) and unchecks (Window::SetChecked) |
| `0x700e90` | Window::List700E90 | In outline: a step with a list (+0x174) |
| `0x669f00` | Window::Rect669F00 | In outline: takes a rectangle from a part (+0xD8, slot 0x20) and acts |
| `0x643df0` | Window::List643DF0 | In outline: a step with a list (+0x3C) |
| `0x685a20` | Window::Set685A20 | In outline: sets a value (+0x174) and a scaled float |
| `0x659c70` | Window::Pass659C70 | In outline: goes over entries with a work value |
| `0x642d00` | Window::Pass642D00 | In outline: the same (another copy) |
| `0x6c20a0` | Window::Size6C20A0 | In outline: a step with its size |
| `0x6c3be0` | Window::Init6C3BE0 | In outline: clears a record |
| `0x61c1a0` | Window::Set61C1A0 | In outline: sets a value (+0x28) to minus an object's (+0xFC) and more |
| `0x700a60` | Window::Part700A60 | In outline: a step with a part's value (+0xE0 +0x114) |
| `0x632ef0` | AttractionDetails::Tab632EF0 | In outline: when a tab is available (AttractionDetails::TabAvailable), a step by tab |
| `0x61c000` | Window::Check61C000 | In outline: a step unless asked and not flagged |
| `0x65f2f0` | Window::Size65F2F0 | In outline: a step with its size |
| `0x9bb9f0` | Window::Bind9BB9F0 | In outline: sets a value (+0x104) and binds (Window::Bind9B3240) |
| `0x6b5770` | Gui::Terrain6B5770 | In outline: a step when the terrain is not flagged |
| `0x6f2160` | Gui::Init6F2160 | In outline: sets up a record with empty texts (WideString::Empty) |
| `0x633b90` | Gui::Make633B90 | In outline: makes a 0x1B0-byte part |
| `0x660560` | Gui::Make660560 | In outline: makes a 0x160-byte part |
| `0x67c7c0` | Window::Check67C7C0 | In outline: when on (+0xF4), a step |
| `0x619020` | Text::Assign619020 | In outline: assigns a two-byte text (WideString::AssignWide) and acts |
| `0x6bf770` | Window::Free6BF770 | In outline: with a part (+0x6C), frees it |
| `0x682990` | Window::Shared682990 | In outline: a step with a shared object's value (`0x12F83F0` +0xDC) |
| `0x6ba1e0` | Window::Width6BA1E0 | In outline: a step with a width |
| `0x668cc0` | Window::Width668CC0 | In outline: the same (another copy) |
| `0x687d60` | Folder::Find687D60 | In outline: a find (Folder::Find6879A0) |
| `0x62c7b0` | Window::Case62C7B0 | In outline: a step by case (0, ...) |
| `0x680b50` | Window::SetPoint680B50 | In outline: copies a point into its values (+0xD4 on) |
| `0x667320` | Window::Mode667320 | In outline: in mode 1 or 2 (+0x170), a step (Thunk67B3C0 with `0x12FC394`) |
| `0x6c34e0` | Window::Count6C34E0 | In outline: with a shared count (`0x1305DE8`), a step |
| `0x6b84f0` | Window::Clear6B84F0 | In outline: empties a list (+0x118) |
| `0x640a90` | Window::Pass640A90 | In outline: goes over entries (+0x64, +0x68) |
| `0x6adf50` | Window::Check6ADF50 | In outline: a check with a work buffer |
| `0x671ff0` | Window::Slot671FF0 | In outline: when flagged (bit 1), a step for an object's slot (+0x150) under a count (+0x128) |
| `0x632d00` | AttractionDetails::Tab632D00 | In outline: when a tab is available (AttractionDetails::TabAvailable), a step |
| `0x6b0470` | Window::Flag6B0470 | In outline: when not flagged (+0x2C) and the other is flagged (bit 13), a step |
| `0x659510` | Gui::Make659510 | In outline: makes a 0x13C-byte part |
| `0x634360` | Gui::Make634360 | In outline: makes a 0x224-byte part |
| `0x6f2410` | Gui::Init6F2410 | In outline: sets up a record with empty texts (WideString::Empty) |
| `0x6e5eb0` | Gui::Make6E5EB0 | In outline: makes a 0x10-byte part |
| `0x6722d0` | Gui::Make6722D0 | In outline: makes a 0x10-byte part |
| `0x6e3380` | Gui::Make6E3380 | In outline: makes a 0x10-byte part |
| `0x6e75c0` | Gui::List6E75C0 | In outline: a step with a list (+8, +0xC) |
| `0x685ac0` | Gui::Pass685AC0 | In outline: goes over entries (+0x170) |
| `0x671ab0` | Gui::Set671AB0 | In outline: sets a size when it changes |
| `0x673350` | Window::Check673350 | In outline: a check (slot 0x118) with 16 |
| `0x6b0cd0` | Window::Make6B0CD0 | In outline: makes a window (Window::Make6B0370) and acts |
| `0x6b0c50` | Window::Make6B0C50 | In outline: the same (another copy) |
| `0x6b0bd0` | Window::Make6B0BD0 | In outline: the same (another copy) |
| `0x6b0b50` | Window::Make6B0B50 | In outline: the same (another copy) |
| `0x68edc0` | Window::Reset68EDC0 | In outline: clears a value (+0xF4) and sets a flag (+0xE4) |
| `0x683c60` | Window::Shared683C60 | In outline: with a part (+0x144) and a shared value (`0x1305BE4`) of 0, a step |
| `0x671d60` | Window::Part671D60 | In outline: a step with a part (+0x130) |
| `0x6850c0` | Window::Mode6850C0 | In outline: in mode 1 (+0x10), a step by a value |
| `0x6ebd40` | Window::Make6EBD40 | In outline: makes a part with a value (+0x188) |
| `0x6a0bb0` | Window::Value6A0BB0 | In outline: a step with an object's value (+0x74) |
| `0x665920` | Window::Set665920 | In outline: sets a size when it changes, 0 handled |
| `0x665850` | Window::Set665850 | In outline: the same (another copy) |
| `0x665780` | Window::Set665780 | In outline: the same (another copy) |
| `0x62c6d0` | Window::Case62C6D0 | In outline: a step by case (7: 9, ...) |
| `0x66a580` | Window::Shared66A580 | In outline: with no shared value (`0x1306194`), a step |
| `0x6ad870` | Window::Flag6AD870 | In outline: when flagged (bit 11 of +0x94), a step |
| `0x61ddd0` | Window::Array61DDD0 | In outline: makes an array of n entries of 0x14 bytes |
| `0x6a2360` | Window::Part6A2360 | In outline: when on (+0x144), a step with a part (+0x154) |
| `0x674af0` | Window::Slot674AF0 | In outline: when flagged (bit 1), a step for an object's slot (+0x150) under a count (+0x128) |
| `0x641580` | Window::Count641580 | In outline: for a positive count, a step with a list (+0x28) |
| `0x6f0ee0` | Window::Part6F0EE0 | In outline: a step with a part (+0x74) |
| `0x642e00` | Window::Size642E00 | In outline: a step with its size |
| `0x623d00` | Window::Point623D00 | In outline: sets a value from a point (`0x682D80`) |
| `0x620250` | Window::Case620250 | In outline: a step by case (3, ...) |
| `0x7003c0` | Window::Rect7003C0 | In outline: takes a rectangle from a part (+0xD4, slot 0x20) and acts |
| `0x677090` | Window::Case677090 | In outline: for kind 7 from its part (+0x108), a step |
| `0x6aade0` | Gui::Make6AADE0 | In outline: makes a 0x10-byte part |
| `0x61f700` | Gui::Pass61F700 | In outline: goes over entries (+0xF4) |
| `0x63fea0` | Gui::Release63FEA0 | In outline: takes one from a count (+0x14) and frees at zero |
| `0x69f2f0` | Window::Slot69F2F0 | In outline: a step with slot n (+0x2CC) |
| `0x65ad40` | Window::Check65AD40 | In outline: a step when a check passes (slot 0xFC) |
| `0x62c950` | Window::Loop62C950 | In outline: a loop up to n |
| `0x6de2f0` | Window::Name6DE2F0 | In outline: a name text and more |
| `0x6df9e0` | Window::Part6DF9E0 | In outline: a step with a part (+0x84) |
| `0x630d80` | Window::Part630D80 | In outline: with a part's part (+0x1C +0x190), a step |
| `0x62c9d0` | Window::Mode62C9D0 | In outline: in mode 7 (+0x70), a step |
| `0x700e10` | Window::Clear700E10 | In outline: empties a list (+0x174) |
| `0x6c9d50` | Window::Count6C9D50 | In outline: a step when a count (`0x6C80C0` on +0x218) is under 8 |
| `0x67e8f0` | Window::Step67E8F0 | In outline: a step with many values |
| `0x661660` | Ref::Step661660 | In outline: adds one to a count and acts |
| `0x63e550` | Window::Draw63E550 | In outline: draws a frame (Window::DrawFrame6ACE50) and acts with a part (+0xD8) |
| `0x6b0e30` | Window::Part6B0E30 | In outline: with a part (+0x3C), tells it (slot 0x18) |
| `0x6a2c40` | Window::Width6A2C40 | In outline: makes a part and a step with a width |
| `0x644d90` | Window::Value644D90 | In outline: a value with another (+0x1B0) |
| `0x6247f0` | Window::List6247F0 | In outline: a step with a list (+0x4C) |
| `0x671c30` | Window::Find671C30 | In outline: a find over a part's list (+0x128 +0x64) |
| `0x6673b0` | Window::Content6673B0 | In outline: sets content (Window::SetContent) from a part's entry |
| `0x675b70` | Window::List675B70 | In outline: a step with a list (+0x108) |
| `0x660740` | Window::Fraction660740 | In outline: a fraction from values |
| `0x62ab20` | Window::Mode62AB20 | In outline: unless in mode 1 (+0x70), a step |
| `0x617eb0` | Window::Text617EB0 | In outline: a step when a text check fails (Text::Check403C20) |
| `0x667120` | Window::Set667120 | In outline: sets two values (+0x16C, +0x170) to n and acts |
| `0x69f290` | Gui::Pass69F290 | In outline: goes over entries (+0x47C) |
| `0x6bcb20` | Gui::Assign6BCB20 | In outline: takes a reference to another's part when it differs |
| `0x6208d0` | Gui::Assign6208D0 | In outline: takes a reference to another's part when it differs |
| `0x6bfaf0` | Gui::Init6BFAF0 | In outline: sets up a record with empty texts (WideString::Empty) |
| `0x6b9e70` | Window::Value6B9E70 | In outline: makes a part and a step with a value (+0x340 +0x80) |
| `0x68d7d0` | Window::Shared68D7D0 | In outline: with a shared value (`0x1305BE4`) of 0, makes a part |
| `0x682080` | Window::Shared682080 | In outline: a step with a shared object's value (`0x12FFAA0` +0xC +4 +0xBC) |
| `0x617e40` | Window::Init617E40 | In outline: clears a flag and sets a value to -0x38 (+0x32) |
| `0x671de0` | Window::Part671DE0 | In outline: a step with a part (+0x18) |
| `0x640280` | Window::Copy640280 | In outline: copies a value and an array (Array4::Assign) from another |
| `0x66a770` | Window::Shared66A770 | In outline: with a shared value (`0x1342708`), a check |
| `0x700290` | Window::Place700290 | In outline: a step with a place from n (five steps) and a value (+0x19C) |
| `0x6ea090` | Window::Refresh6EA090 | In outline: when asked, refreshes (Window::Refresh6AFC20) |
| `0x6a29f0` | Window::Part6A29F0 | In outline: with a part (+0x13C), a step |
| `0x67ed80` | Window::Check67ED80 | In outline: a step when a check fails (slot 0x10C) |
| `0x691b10` | Rect::Assign691B10 | In outline: copies a rectangle (Rect::Assign) and a value (+0x10) |
| `0x6e9af0` | Window::Make6E9AF0 | In outline: makes a part and a step (`0x6EF470`) |
| `0x6b69e0` | Window::Make6B69E0 | In outline: makes a part and goes over entries |
| `0x68f4e0` | ListWindow::Clear68F4E0 | In outline: sets a count to 0 (ListWindow::SetCount) and a find (Window::Find68EFA0) |
| `0x63ad40` | Window::Release63AD40 | In outline: releases a part (+0xD8) |
| `0x6701d0` | Window::Loop6701D0 | In outline: goes over a tree's values (SortedTree::LastValue) |
| `0x642a90` | Window::Step642A90 | In outline: a step (`0x4677B0`) and more when two values differ |
| `0x632b40` | AttractionDetails::Tab632B40 | In outline: when a tab is available (AttractionDetails::TabAvailable), a step |
| `0x6162d0` | Window::Format6162D0 | In outline: formats a value into a 256-character buffer |
| `0x6c6dd0` | Window::Count6C6DD0 | In outline: a step when a count (`0x6C8110`) is under 5 |
| `0x65b5f0` | Window::Check65B5F0 | In outline: a step when a check fails (slot 0x10) |
| `0x642c50` | Window::Shared642C50 | In outline: a step with a shared table (`0x10EEAD4`) |
| `0x63d0b0` | Window::Free63D0B0 | In outline: sets its table (`0xF494C0`), releases a part and frees when asked |
| `0x6f0fb0` | Window::Flag6F0FB0 | In outline: when flagged (0x40 of +0xD0), a step with a part (+0x74) |
| `0x667600` | Window::Entry667600 | In outline: a step with a part's entry (+0x164 +0x164, slot 0x28) |
| `0x63b440` | Window::Flags63B440 | In outline: when one part is flagged and the other not (bit 0 of +0x94), a step |
| `0x630250` | Window::Device630250 | In outline: a device step (slot 0x38) and more |
| `0x6b2d00` | Window::Shared6B2D00 | In outline: goes over entries with a shared value (`0x1305D54`) |
| `0x6708b0` | Window::Image6708B0 | In outline: when a part is on (+0x58 +0x140), adds a custom image (AdMenu::AddCustomImage) |
| `0x6246c0` | Window::Shared6246C0 | In outline: with no shared object (`0x13429A8`), a step |
| `0x69f230` | Gui::Pass69F230 | In outline: goes over entries (+0x57C) |
| `0x634660` | Gui::Make634660 | In outline: makes a 0x170-byte part |
| `0x6b0180` | Window::Point6B0180 | In outline: a step with a point |
| `0x69e9c0` | Window::Step69E9C0 | In outline: a step with a work value |
| `0x65ec20` | Window::Point65EC20 | In outline: a step when a point (`0x682D50`) is not zero |
| `0x646880` | Window::Group646880 | In outline: goes over a guest group (Guest::Group) |
| `0x6bc7f0` | Window::Value6BC7F0 | In outline: a step (Thunk) and a value (+0x70) |
| `0x68b400` | Window::Point68B400 | In outline: a step with a point plus values |
| `0x685400` | Window::Shared685400 | In outline: when on (+0xF9), a step with a shared value (`0x10EED88`) |
| `0x641600` | Window::Count641600 | In outline: for a positive n, a step with an object (`0x13063D0`) |
| `0x616270` | Window::Format616270 | In outline: formats into a 256-character buffer |
| `0x69eea0` | Window::Value69EEA0 | In outline: a step with a value (+0x1D4) |
| `0x682b90` | Window::Copy682B90 | Copies two values (+0xF8, +0xFC into +0xF0, +0xF4) |
| `0x6a5f40` | Window::Layout6A5F40 | In outline: with a part (+0x7C), lays out (Window::Layout) |
| `0x6a41d0` | Window::Rect6A41D0 | In outline: takes a rectangle (slot 0xF4) and acts |
| `0x671ea0` | Window::Point671EA0 | In outline: a step with an object's point (+0xF8, +0xFC) |
| `0x642da0` | Window::Point642DA0 | In outline: the same (another copy) |
| `0x666f90` | Window::Part666F90 | In outline: with a part's part (+0x1C +0x174), a step |
| `0x611a80` | Window::Array611A80 | In outline: makes an aligned buffer of n four-byte entries |
| `0x6c38b0` | Window::Step6C38B0 | In outline: a step (`0x6C3870`) and a value (+0x5C) |
| `0x6b9ba0` | Window::Case6B9BA0 | In outline: a value by case (6, ...) |
| `0x5fc790` | Window::Rect5FC790 | In outline: takes two rectangles (slot 0x20) and acts |
| `0x6cca10` | Window::Set6CCA10 | In outline: sets a value (+0xDC) and acts, 0 handled |
| `0x6ba070` | Window::Pass6BA070 | In outline: goes over entries (4, 0x1D0) |
| `0x62c900` | Window::Pass62C900 | In outline: goes over entries (+0x30) |
| `0x6e9910` | Window::Slot6E9910 | In outline: for a slot under 8 (+0x50), a step |
| `0x67cad0` | Window::Point67CAD0 | In outline: a step with a part's point (+0xDC) |
| `0x6595d0` | Window::Case6595D0 | In outline: a step by case (0, ...) |
| `0x6c82e0` | Window::Reset6C82E0 | In outline: clears a value (+0xD8) and acts with a list (+0x30) |
| `0x68ed50` | Window::Reset68ED50 | In outline: clears a value (+0xF4) and sets a flag (+0xE4) |
| `0x6837c0` | Window::Children6837C0 | In outline: a step with children (Window::ChildrenSlot6) and clears a flag (bit 27) |
| `0x683770` | Window::Flag683770 | In outline: when flagged (bit 0 of +0x97), a step with a part (+0x130) |
| `0x6695b0` | List::Free6695B0 | In outline: goes over a list's entries and frees them |
| `0x65fb30` | Window::Init65FB30 | In outline: clears a value and points a list at its own storage |
| `0x654280` | List::Free654280 | In outline: goes over a list's entries and frees them |
| `0x6bc4d0` | Window::Make6BC4D0 | In outline: makes a part with 0x2D |
| `0x6a00b0` | Window::Slot6A00B0 | In outline: with a slot (+0x174), a step |
| `0x6809f0` | Window::Flag6809F0 | In outline: sets a flag (0x80000) on a part and acts when n passes a value (+0x358) |
| `0x67df20` | Window::Entry67DF20 | In outline: for n under a part's count (+0x104 +4), a step (`0x67CD80`) |
| `0x6645b0` | Window::Part6645B0 | In outline: with a part (+0xD8 +0xC), a step with an object (`0x130B5C8`) |
| `0x6ad4c0` | Window::Value6AD4C0 | In outline: with a value, a step |
| `0x67cd80` | Window::Next67CD80 | In outline: for n plus one under a count (+4), a step |
| `0x6a8180` | Window::List6A8180 | In outline: a step with a part's list (+0xDC +0x124) |
| `0x68d750` | Window::Part68D750 | In outline: with a part (+0xE0), a step |
| `0x615d70` | Window::List615D70 | In outline: a step with a part's list (+0xD8 +0x114) |
| `0x6092c0` | Window::Fill6092C0 | In outline: fills (Window::Fill6084F0) and acts |
| `0x6db740` | Window::Free6DB740 | In outline: with a buffer (+0x7C), frees it |
| `0x6d4620` | Window::Width6D4620 | In outline: a step with a part's width (+0xD4) |
| `0x678ec0` | Window::Flag678EC0 | In outline: 0 unless flagged (bit 0 of +0x94), else a value |
| `0x6a7da0` | Window::Mode6A7DA0 | In outline: a step by mode (+0xF0: 1, ...) |
| `0x688230` | Window::Part688230 | In outline: with a part (+0x178 +0xE4), a step (`0x685BA0`) |
| `0x660630` | Window::Fraction660630 | In outline: a fraction from values |
| `0x67edf0` | Window::Check67EDF0 | In outline: a step (Window::Slot48Self) and a check (slot 0x10C) |
| `0x63fe50` | Gui::Init63FE50 | In outline: sets up a record with empty texts (WideString::Empty) |
| `0x6bfb50` | Gui::Pass6BFB50 | In outline: goes over entries (+0x11C) |
| `0x65f2a0` | Window::Size65F2A0 | In outline: a step with its width plus n |
| `0x642cb0` | Window::Shared642CB0 | In outline: a step (`0x467A30`) and a shared table (`0x10EEAD4`) |
| `0x6eed60` | Window::Hit6EED60 | In outline: a step when a point lies inside (Rect::Contains) |
| `0x6f2490` | Window::Step6F2490 | In outline: a step with many values |
| `0x6b16f0` | Window::Part6B16F0 | In outline: with a part (+0x44 +4), a step |
| `0x6809a0` | Window::Centre6809A0 | In outline: a step with half its size |
| `0x670620` | Window::Width670620 | In outline: a part's width (+0x134) plus more |
| `0x63d140` | Window::Point63D140 | In outline: a step with a point |
| `0x63d010` | Window::Free63D010 | In outline: sets its table (`0xF494C0`) and releases a part |
| `0x639770` | Window::Layout639770 | In outline: in mode 1 (+0x1C), lays out (Window::Layout) |
| `0x6c80c0` | Window::Count6C80C0 | In outline: a count over entry n (0x20 bytes at +0x30) |
| `0x6af710` | Window::Init6AF710 | In outline: points a record at itself and sets values |
| `0x6a1b00` | Window::Value6A1B00 | In outline: a step with two values (slots 0xFC, 0x100) |
| `0x61ece0` | Window::Console61ECE0 | In outline: when on (+0xF8), opens the message console (MessageConsole::Open) |
| `0x6e9960` | Window::Slot6E9960 | In outline: for a slot under 8 (+0x50), a step |
| `0x6b5f40` | Window::Interface6B5F40 | In outline: with an object (`0x130B5C8`), a step |
| `0x6b2cb0` | Window::Shared6B2CB0 | In outline: goes over entries with a shared value (`0x1305D54`) |
| `0x6a7a10` | Window::Owner6A7A10 | In outline: a step with its owner's value |
| `0x683620` | WideString::Grow683620 | In outline: makes a two-byte text unique with room for two more (WideString::MakeUnique) |
| `0x67bb60` | Window::Part67BB60 | In outline: when flagged (bit 22) with a part's entries (+0x124), a step |
| `0x670670` | Window::Part670670 | In outline: with a part (+0x58), a step |
| `0x63a180` | Window::Part63A180 | In outline: with a part (+0x3C), a step |
| `0x6e8b70` | Window::Find6E8B70 | In outline: a find (Record::Find460E80) and acts |
| `0x6c8030` | Window::Slot6C8030 | In outline: for a slot under 8 (+0x50), a step |
| `0x6b1740` | Window::List6B1740 | In outline: a step with a list (+0x40) |
| `0x6b08f0` | Window::Part6B08F0 | In outline: a part step (Window::Part6AF2E0) and unless flagged (bit 2), more |
| `0x63d060` | Window::Init63D060 | In outline: sets up a record with its table (`0xF494C0`) and a value |
| `0x6182b0` | Window::Shared6182B0 | In outline: a step with a shared object's width (`0x12F83F0`) |
| `0x5f4790` | Window::Set5F4790 | In outline: sets a value (+0xF4) and acts with a part (+0x84) |
| `0x624990` | Window::Shared624990 | In outline: sets a shared flag (bit 1 of `0x1313C64`) when needed |
| `0x680ab0` | Window::Ring680AB0 | In outline: a step with a ring buffer's next entry (12 bytes) |
| `0x6fde90` | Grid::Slot6FDE90 | In outline: with a slot (+0x40), a value |
| `0x6ad690` | Window::Screen6AD690 | In outline: a screen position (Window::ScreenPosition) and points |
| `0x6455c0` | Window::Lod6455C0 | In outline: a level-of-detail check (SceneryVisual::CheckLOD) |
| `0x630d30` | Window::Shared630D30 | In outline: a step (Window::Slot48Self) and with no shared count (`0x1106C54`), more |
| `0x6b16b0` | Window::Flag6B16B0 | In outline: with a part (+0x3C), a step by a flag (+0x94) |
| `0x6b1670` | Window::Flag6B1670 | In outline: the same (another copy) |
| `0x6a1f00` | Window::Flags6A1F00 | In outline: a step by flags (0xC0 of +0x140) |
| `0x69f1e0` | ListWindow::Count69F1E0 | In outline: sets a count (ListWindow::SetCount) and a step (`0x69EFF0`) |
| `0x673310` | Window::Check673310 | In outline: 0 for none, else a check |
| `0x6c3870` | Window::Pass6C3870 | In outline: goes over entries (`0x6C8180` with +0x58) |
| `0x6a3b60` | Window::Init6A3B60 | In outline: fills a record from its arguments |
| `0x642860` | Window::Children642860 | In outline: children (Window::ChildrenSlot6) and acts with a part (+0x118) |
| `0x642700` | Window::Step642700 | In outline: a step (`0x467780`) and acts with no part (+0x124) |
| `0x640ec0` | Window::Part640EC0 | In outline: a step with a part (+0x44) |
| `0x61cef0` | Window::Count61CEF0 | In outline: a check with a count (+0x8C) and a part (+0x94) |
| `0x669d40` | WideString::Add669D40 | In outline: makes a two-byte text unique with room for one more (WideString::MakeUnique) and adds a character |
| `0x644e90` | Window::Flags644E90 | In outline: sets flags (+0x1B4) and acts by bit 0 |
| `0x63a8a0` | Window::Layout63A8A0 | In outline: lays out (Window::Layout) and refreshes (Window::Refresh6AFC20) |
| `0x6bc8b0` | Window::Value6BC8B0 | In outline: a step (Thunk) and a value (+0x70) |
| `0x6af3e0` | Window::Focus6AF3E0 | In outline: with a first focusable window (Window::FirstFocusable), a step |
| `0x69b100` | Window::Keyboard69B100 | In outline: a step with the keyboard layout (HKL) |
| `0x65b470` | Window::Check65B470 | In outline: when a part is not flagged and a check passes (slot 0x10), a step |
| `0x630780` | Window::Fade630780 | In outline: a rounded fade from a shared object's value (`0x1103B6C` +0x180) |
| `0x69e950` | Window::Set69E950 | Sets two values (+0xD8, +0xDC) |
| `0x68fc10` | Gui::Pass68FC10 | In outline: goes over entries (+0x164) |
| `0x6721c0` | Gui::Pass6721C0 | In outline: goes over entries (+0x14C) |
| `0x6f0f60` | Gui::Copy6F0F60 | In outline: copies values from another record |
| `0x6c3d20` | Gui::Assign6C3D20 | In outline: frees its buffer and remakes it from another |
| `0x620930` | Gui::List620930 | In outline: a step with a list (+0x8) |
| `0x6b8a40` | Window::Shared6B8A40 | In outline: with no shared value (`0x1305DD8`), a step (`0x6C1DA0`) |
| `0x660520` | Window::Event660520 | In outline: an event step (Window::Event65FFE0) and for events 0x10, 1 or 3, more |
| `0x6bc690` | Window::Round6BC690 | In outline: n over m plus a half, rounded |
| `0x6b8a00` | Window::Shared6B8A00 | In outline: with no shared value (`0x1305DD8`), a step (`0x6C1DA0`) |
| `0x6428a0` | Window::Skin6428A0 | In outline: with a part (+0x118), a skin texture (SkinItem::Texture) |
| `0x63a090` | Window::Event63A090 | In outline: a step (Window::Step642A90) and for events 0x10, 1 or 3, more |
| `0x9f01e0` | Window::Children9F01E0 | In outline: children (Window::ChildrenSlot4) and a step with a part (+0xE0, slot 0xFC) |
| `0x6e9890` | Window::Find6E9890 | In outline: a find with an object's list (+0x48, +0x4C) |
| `0x6c3c80` | Window::Add6C3C80 | In outline: grows by 10 when full (`0x6C3A80`) and adds |
| `0x6706d0` | Window::Reset6706D0 | In outline: clears a value (+0x38) and acts with another (+0x3C) |
| `0x661e20` | Window::Step661E20 | In outline: a step with values |
| `0x65a720` | Window::Check65A720 | In outline: a step when a check fails (slot 0x10) |
| `0x63da80` | Window::Init63DA80 | In outline: clears a value and points a list at its own storage |
| `0x6175c0` | Window::Add6175C0 | In outline: grows by one (`0x6173B0`) and adds |
| `0x6e9ab0` | Window::Make6E9AB0 | In outline: makes a part with 10 |
| `0x6e8780` | Window::Find6E8780 | In outline: a find (Record::Find460E80) and acts |
| `0x6db790` | Window::Add6DB790 | In outline: grows by one (`0x6C3A80`) and adds |
| `0x65a6d0` | Window::Point65A6D0 | In outline: a step with a point |
| `0x6f1010` | Window::Flags6F1010 | In outline: a value from flags (+0xD0) |
| `0x6c8110` | Window::Pass6C8110 | In outline: goes over entries (+0x1C) |
| `0x69cf50` | Window::Keyboard69CF50 | In outline: when on (+0x20) with a shared value (`0x10EDBB0`), a keyboard layout step |
| `0x690b00` | Window::Children690B00 | In outline: children (Window::ChildrenSlot4) and sets a shared flag (`0x13D3250`) |
| `0x63b400` | Window::Content63B400 | In outline: sets content (Thunk67B3C0) and a step (`0x67B4C0`) |
| `0x6a7720` | Window::Init6A7720 | In outline: fills a record from its arguments |
| `0x6a6760` | Window::Shared6A6760 | In outline: a step with a shared table (`0x10EF210`) |
| `0x685530` | Window::Device685530 | In outline: with a shared value (`0x12F8434`), a step with the device (slot 0xCC) |
| `0x62ac60` | Window::Check62AC60 | In outline: a step when a part's check passes (+8, slot 4) |
| `0x6c8080` | Window::Slot6C8080 | In outline: a step with an object's values (+0x4C, +0x50) |
| `0x6a7cb0` | Window::Flag6A7CB0 | In outline: a step with a part (+0xDC) by a flag |
| `0x6a7c70` | Window::Flag6A7C70 | In outline: the same with +0xE0 |
| `0x6715e0` | Window::Toggle6715E0 | Turns a flag (+0x124) on or off and acts |
| `0x63a860` | Window::Value63A860 | In outline: when on (+0xD4), a value from n with fixed scales |
| `0x620dd0` | Window::Init620DD0 | In outline: sets up a record with its table (`0xF43E60`) pointing at itself |
| `0x6e98d0` | Window::Slot6E98D0 | In outline: a step with an object's values (+0x48, +0x4C) |
| `0x69ebb0` | Button::Image69EBB0 | In outline: sets a button's single image (Button::SetSingleImage) |
| `0x6c3910` | Window::Shared6C3910 | In outline: a step with a shared part (`0x1305E20` +0xD4) and sets a value (Window::Set6CC950) |
| `0x67e670` | Window::Step67E670 | In outline: a step with values |
| `0x67b4c0` | Window::Set67B4C0 | In outline: sets a value (+0x120) from an object (+8) |
| `0x65ea60` | Window::Current65EA60 | In outline: a current-object step (`0x5F7710`) and a value (+0x134) |
| `0x479740` | Window::Pair479740 | In outline: sets a pair of values and another from a shared table entry (`0x10ED868`) |
| `0x70b720` | Grid::Case70B720 | In outline: a case step (Window::Case701F50) and acts when its part (+0x148) is current |
| `0x6bc8f0` | Window::Value6BC8F0 | In outline: a step (Thunk) and a value (+0x70) |
| `0x6bc880` | Window::Value6BC880 | In outline: the same (another copy) |
| `0x6bc850` | Window::Value6BC850 | In outline: the same (another copy) |
| `0x6876f0` | Window::Set6876F0 | In outline: sets a flag (+0x2CC) and copies a part's value (+0x168 +0x170) |
| `0x685070` | Window::Init685070 | In outline: a list member (ListMember with `0x10EC9E8`) with its table (`0xF5182C`) |
| `0x680110` | Window::Flag680110 | In outline: when not flagged (+0x2E8), sets a shared flag (bit 0 of `0x1103C58` +0x2DC) |
| `0x665a00` | Window::Grow665A00 | In outline: grows when n passes a count (+4) |
| `0x6659d0` | Window::Grow6659D0 | In outline: the same (another copy) |
| `0x6659a0` | Window::Grow6659A0 | In outline: the same (another copy) |
| `0x6ff0d0` | Grid::Step6FF0D0 | In outline: a step (Window::Step642700) and a part's part (+0x154 +0xD8) |
| `0x6d78e0` | Window::Free6D78E0 | In outline: sets its table (`0xF64250`), a step (`0xB6C570`) and frees when asked |
| `0x6c8180` | Window::Entry6C8180 | In outline: for (n under 8, m), a value |
| `0x6a2be0` | Window::Flag6A2BE0 | In outline: 0 when flagged (bit 0 of +0x97), else a step |
| `0x672130` | Window::Add672130 | In outline: grows by one (`0x6719E0`) and adds |
| `0x66a810` | Window::Add66A810 | In outline: grows by one (`0x66A4C0`) and adds |
| `0x666e30` | Window::Part666E30 | In outline: with a part's part (+0x1C +0x1C8), a step |
| `0x65c170` | Window::Byte65C170 | In outline: a byte from a table (+0x24) and acts with a part (+0x20) |
| `0x624610` | Window::Selected624610 | In outline: whether a shared object (`0x1306CA4` +0xC) is the selected one (`0x12F503C`) |
| `0x6245e0` | Window::Selected6245E0 | In outline: the same with `0x13CE688` |
| `0x6e9a80` | Window::Entry6E9A80 | In outline: for (n, m under 1), a value |
| `0x6b2fc0` | Window::SetPoint6B2FC0 | In outline: copies a point into its values (+0x24 on) |
| `0x664810` | Window::Init664810 | In outline: sets up entry n's handle (0x14 bytes at +0x138) |
| `0x6595a0` | Window::Refresh6595A0 | In outline: for 3, refreshes (Window::Refresh6AFC20) |
| `0x6ef470` | Window::Reset6EF470 | In outline: clears values (+8, +0xC, ...) |
| `0x66a7e0` | Window::Free66A7E0 | In outline: with a buffer (+0xDC), frees it |
| `0x6554d0` | Window::Part6554D0 | In outline: with a part (+0x100), a value |
| `0x63d4b0` | Window::Set63D4B0 | In outline: sets a value (+0x24) when it changes and acts with a part (+0x20) |
| `0x5fcae0` | Window::Flags5FCAE0 | In outline: clears a flag (+0x154) and adds flags (+0x140) into a part (+0x13C) |
| `0x6fd260` | Gui::List6FD260 | In outline: a step with a list (+0x4C) |
| `0x66c6f0` | Gui::Release66C6F0 | In outline: takes one from a count (+0x8) and frees at zero |
| `0x66b330` | Gui::Release66B330 | In outline: takes one from a count (+0x8) and frees at zero |
| `0x6defb0` | Gui::Make6DEFB0 | In outline: makes a 0x10-byte part |
| `0x701910` | Window::Nth701910 | In outline: a list step (Window::List700E90) and the nth entry (Window::Nth6FDD50) |
| `0x6acd70` | Window::Scale6ACD70 | In outline: sets a value (+0x8C) from a scaled point (Window::Scale6ACBD0) |
| `0x6ab170` | Window::Step6AB170 | In outline: a step with a work value |
| `0x62e810` | Window::Terrain62E810 | In outline: a terrain step (Terrain::Get, `0x8D30D0`) |
| `0x62e790` | Window::Terrain62E790 | In outline: the same (another copy) |
| `0x70b6f0` | Grid::Case70B6F0 | In outline: a case step (Window::Case701F50) and acts when its part (+0x148) is current |
| `0x6a1ed0` | Window::Check6A1ED0 | In outline: when flagged (0x80 of +0x140), checks a box (Window::SetChecked) from a flag (+0x160) |
| `0x69efa0` | Window::Layout69EFA0 | In outline: lays out (Window::Layout) and a step with its owner (slot 0xC0) |
| `0x69e990` | Window::Flag69E990 | In outline: sets or clears a flag (0x400000 of +0x94) |
| `0x66c6c0` | Window::Compare66C6C0 | In outline: a step when two texts match (Text::CompareCD5640) |
| `0x467780` | Window::Loop467780 | In outline: a loop (Window::Loop6B0740) and acts with a part (+0xE4) |
| `0x6a0c50` | Window::Slot6A0C50 | In outline: with a slot (+0xD8), a step |
| `0x682800` | Window::Screen682800 | In outline: a screen position (Window::ScreenPosition) and points |
| `0x62e460` | Window::Shared62E460 | In outline: with a shared object's part (`0x12F4620` +0x58), takes it (Window::Take484780) |
| `0x7018e0` | Window::List7018E0 | In outline: a list step (Window::List700E90) and acts when on (+0xE4) |
| `0x6ea040` | Window::Panel6EA040 | In outline: a panel step (Panel::ChildrenSlot2ThenRelate) and acts when flagged |
| `0x6c3650` | Window::Init6C3650 | In outline: fills a record from an object (slot 0xC) |
| `0x6c7fd0` | Window::Kind6C7FD0 | In outline: a value from an object's kind (+0x58) |
| `0x6a2bb0` | Window::Set6A2BB0 | Sets a value (+0xE4) and a flag (0x1000000 of +0x94) |
| `0x6ed2e0` | Window::Text6ED2E0 | In outline: a text (Window::Text6EB620) and makes a part (Gui::Make6ECF90) |
| `0x6428e0` | Window::Key6428E0 | In outline: for key 0x1B (escape), lays out (Window::Layout) |
| `0x62e7c0` | Window::Terrain62E7C0 | In outline: when the terrain is on (+0x5D), a step |
| `0x6a9570` | Window::Event6A9570 | In outline: tells a part (+0xD4, slot 4 with 0x24) and acts when on (+0xEC) |
| `0x6a4b10` | Window::Set6A4B10 | In outline: sets a value (+0xE0) and lays out (Window::Layout6A4820) |
| `0x68d7a0` | Window::Shared68D7A0 | In outline: with a shared value (`0x1305BE4`) of -1, a step (`0x69B140`) |
| `0x685ba0` | Window::Slot685BA0 | In outline: gives slot n's part (+0x580 by +0x278) |
| `0x685b70` | Window::Slot685B70 | In outline: the same with +0x578 |
| `0x67b910` | Window::Handle67B910 | In outline: a step with a handle (`0xF50614`) |
| `0x631c20` | Window::Step631C20 | In outline: a step with the largest float |
| `0x62b0b0` | Window::Part62B0B0 | In outline: with a part (+0x2F0), a step with a value (+0x2F4) |
| `0x6ef4a0` | Window::Value6EF4A0 | In outline: 3 with a part (+0x38), else another value |
| `0x6d44f0` | Window::Case6D44F0 | In outline: for kind 7 with its own value (+0xE4), a step |
| `0x6c3940` | Window::Value6C3940 | In outline: a value with a part (+0x44), none handled |
| `0x6854f0` | Window::Step6854F0 | In outline: a step (Window::Step642700) and acts when on (+0x2CC) |
| `0x672350` | Window::Entry672350 | In outline: gives entry n's part (0x4C bytes at +0x10) |
| `0x65bbe0` | Window::Release65BBE0 | In outline: releases a part and frees when asked |
| `0x6e1e40` | Window::Slot6E1E40 | In outline: gives slot n's part (+0x80) |
| `0x6b26e0` | Window::Free6B26E0 | In outline: sets its table (`0xF60C64`), a step (`0xBD3170`) and frees when asked |
| `0x6aba30` | Window::Part6ABA30 | In outline: with a part (+0x13C), a step |
| `0x6a4ab0` | Window::Value6A4AB0 | In outline: a value from a part's part when flagged (bit 22) |
| `0x6854a0` | Window::Set6854A0 | In outline: a step (slot 0xF0 with 4.0) and acts with a part (+0x3C) |
| `0x682d80` | Window::Size682D80 | In outline: gives a size from its edges (+0xEC, +0xFC, ...) |
| `0x682d50` | Window::Size682D50 | In outline: the same with +0xF4 |
| `0x681330` | Window::Pass681330 | In outline: passes n to a part (+0xD4, slot 0xF4) |
| `0x648710` | Window::Pass648710 | In outline: a step (Window::Slot48Self) and a pass (Park::Pass774340) |
| `0x544570` | Window::Children544570 | In outline: children (Window::Children67ACA0) and acts with a part (+0x140) |
| `0x483d80` | Window::List483D80 | In outline: a step with a list |
| `0x477ea0` | Window::Make477EA0 | In outline: makes a part and a step (`0x6ACAC0`) |
| `0x6e6740` | Window::Step6E6740 | In outline: a step with two values (+0xC, +0x18) |
| `0x68d560` | Window::Offset68D560 | In outline: a value from parts' values less another (+0xFC) |
| `0x65bd00` | Window::Part65BD00 | In outline: with a part (+0x20), a step |
| `0x63d280` | Window::Part63D280 | In outline: the same (another copy) |
| `0x5fb220` | Window::Shared5FB220 | In outline: a shared step (Window::Shared6B2D00) and sets inactive (Updatable::SetActive2) |
| `0x6f0e70` | Window::Text6F0E70 | In outline: a find (Text::FindBCEC90) with a wild animal value (WildAnimal::ValueB44EF0) |
| `0x6c3e40` | Window::Check6C3E40 | In outline: for none, a step |
| `0x6a66e0` | Window::Take6A66E0 | In outline: with a part (+0x70), takes it (Window::Take6A65B0) |
| `0x69ef40` | Window::Set69EF40 | In outline: sets a value (+0x570) and acts unless flagged (bit 22) |
| `0x655430` | Window::Children655430 | In outline: children (Window::ChildrenSlot2) and goes over entries |
| `0x617590` | Window::Free617590 | In outline: with a buffer (+0x1C), frees it |
| `0x6d60c0` | Window::Slot6D60C0 | In outline: when on (+0x54), a slot step (Window::Slot6C8030) |
| `0x6c8000` | Window::Slot6C8000 | In outline: for an object's slot under 5 (+0x58) that is this one, a step |
| `0x6ad600` | Window::Flag6AD600 | In outline: sets a flag (0x800 of +0x94) and acts with its parent (+0x3C) |
| `0x675550` | Window::Clear675550 | In outline: empties a list |
| `0x673ac0` | Window::Clear673AC0 | In outline: empties a list |
| `0x630360` | Window::SetPoint630360 | In outline: copies a point into its values (+0xF0 on) |
| `0x62c750` | Window::Check62C750 | In outline: a step when a check fails (slot 0xFC) |
| `0x6a9550` | Window::Sound6A9550 | In outline: releases a GUI sound (Sound::GUIBRelease) and tells a part (+0xD4, slot 4 with 0x23) |
| `0x6a0740` | Window::Interface6A0740 | In outline: with an interface (`0x1305C18`), a step |
| `0x680a40` | Window::Screen680A40 | In outline: a screen step (Window::Screen690B70) and sets a flag (+0x340) |
| `0x67b980` | Window::Interface67B980 | In outline: with an interface (`0x12F8218`), a step |
| `0x630a80` | Window::Make630A80 | In outline: makes a part and lays out (Window::Layout) |
| `0x62e5b0` | Window::Step62E5B0 | In outline: a step with a value (+0x144) |
| `0x9bbdf0` | Window::Rect9BBDF0 | In outline: a rectangle step (Window::Rect6A41D0) |
| `0x9f5fe0` | Window::Flag9F5FE0 | In outline: a flag step (Window::Flag6A2BE0) with work values |
| `0x680140` | Window::Reset680140 | In outline: a set step (Window::Set6876F0) and clears a shared value (`0x1103C58` +0x2DC) |
| `0x67a940` | Window::Copy67A940 | In outline: copies values into a part (+0xF0, +0xE4 on) |
| `0x6b58c0` | Window::Part6B58C0 | In outline: with a part (+0x40000), a step |
| `0x680a90` | Window::Free680A90 | Frees a buffer and clears the pointer |
| `0xa55810` | Window::UpdateA55810 | In outline: takes a part (Window::Take6A66E0) and updates entries (Window::UpdateEntries) |
| `0xa1a6f0` | Window::SetA1A6F0 | In outline: sets a value (+0xB0) and a flag on a shared window (0x400000 of `0x12F83F0` +0x94) |
| `0x6ebd20` | ListWindow::Select6EBD20 | In outline: refreshes (Window::Refresh6EB470) and selects (ListWindow::SetSelection) |
| `0x6d7870` | Window::Init6D7870 | In outline: sets its table (`0xF64250`) and a find (Animal::FindB6C520) |
| `0x6c9d30` | ListWindow::Select6C9D30 | In outline: fills (ListWindow::Fill6C9B80) and selects (ListWindow::SetSelection) |
| `0x6c8160` | Window::Entry6C8160 | In outline: for n under 5, a value |
| `0x6c6db0` | ListWindow::Select6C6DB0 | In outline: fills (ListWindow::Fill6C6C10) and selects (ListWindow::SetSelection) |
| `0x6af100` | Window::Hit6AF100 | In outline: a hit step (Window::Hit6AED50) with a handle (`0xF606D0`) |
| `0x6af0e0` | Window::Hit6AF0E0 | In outline: the same with `0xF606B0` |
| `0x6af0c0` | Window::Hit6AF0C0 | In outline: the same with `0xF606A0` |
| `0x6af0a0` | Window::Hit6AF0A0 | In outline: the same with `0xF60670` |
| `0x6af080` | Window::Hit6AF080 | In outline: the same with `0xF60660` |
| `0x68eda0` | Window::Reset68EDA0 | Clears a value (+0xF4) and a flag (+0xE4) |
| `0x667680` | Window::Set667680 | In outline: sets values (Window::Set667120) from a value (slot 0x15C) |
| `0x6c3970` | Window::Pass6C3970 | In outline: a pass (Window::Pass6C3870 with 5) and sets a value to 360 (+0x74) |
| `0x6c21f0` | Window::Shared6C21F0 | In outline: with no shared count (`0x1305DE8`), clears another (`0x1305DEC`) |
| `0x6a6350` | Window::Layout6A6350 | In outline: with a part (+0xEC), lays out (Window::Layout6A5F40) |
| `0x690ee0` | Window::Device690EE0 | In outline: a device step (slot 0x78) with three values |
| `0x68d5b0` | Window::Part68D5B0 | In outline: with a part (+0xE0), tells it (slot 0xE0 with 0x10000) |
| `0x667f70` | Window::Place667F70 | In outline: places (Window::Place667990) and sets a flag (+0x20) |
| `0x6631a0` | Window::Scenery6631A0 | In outline: sets scenery values (Scenery::Set83DBC0) |
| `0x685140` | Window::Set685140 | In outline: sets a value to 1 (+0x10) and acts when asked |
| `0x6a1e70` | Window::Check6A1E70 | In outline: checks a box (Window::SetChecked) from a part's value (+0x13C, slot 0x10) |
| `0x672dd0` | Window::Part672DD0 | In outline: with a part (+0x128), a step |
| `0x62e490` | Window::Update62E490 | In outline: makes a part (`0x5F76F0`) and updates entries (Window::UpdateEntries) |
| `0x47ca80` | Sound::Unmute47CA80 | In outline: unmutes a kind of sound (Sound::UnmuteKind) |
| `0x47ca60` | Sound::Mute47CA60 | In outline: mutes a kind of sound (Sound::MuteKind) |
| `0x6eaa60` | Window::Make6EAA60 | In outline: makes a part (Window::Make6E9AF0) |
| `0x6b4be0` | MapDialog::Build6B4BE0 | In outline: builds the map dialog (MapDialog::Build) and a step |
| `0x6b2d70` | Window::Set6B2D70 | In outline: a device set (Device::SetBD3750 with 1) and copies a flag (+0x60 into +0x6C) |
| `0x6a3140` | Window::Set6A3140 | Sets a value (+0x10C) and a flag (0x1000000 of +0x94) |
| `0x68d590` | Window::Part68D590 | In outline: with a part (+0xE0), tells it (slot 0xE0) |
| `0x67a8e0` | Window::Set67A8E0 | In outline: sets a value (+0xD0) |
| `0x9bbf60` | Window::Layout9BBF60 | In outline: lays out (Window::Layout) and children (Window::ChildrenSlot4) |
| `0x9bbf40` | Window::Refresh9BBF40 | In outline: refreshes (Window::Refresh6AFC20) and a step (Window::Step642700) |
| `0x9a9ad0` | Window::Layout9A9AD0 | In outline: lays out (Window::Layout) and children (Window::ChildrenSlot4) |
| `0x9b2ef0` | Window::Set9B2EF0 | In outline: a set step (Window::Set67A8E0) |
| `0x6d7890` | Window::Free6D7890 | In outline: sets its table (`0xF64250`) and a step (`0xB6C570`) |
| `0x6c8c30` | Window::Reset6C8C30 | In outline: a reset (Window::Reset6C82E0) and children (Window::ChildrenSlot4) |
| `0x6c6880` | Window::Reset6C6880 | In outline: the same (another copy) |
| `0x65d170` | Window::Step65D170 | In outline: a step with a part (+0x154, slot 0x10) and another (Window::Step642700) |
| `0x659c10` | Window::Key659C10 | In outline: for key 0x1B (escape), lays out (Window::Layout) |
| `0x6a31a0` | Window::Value6A31A0 | In outline: a value (slot 0x100) |
| `0x67ab40` | Window::Value67AB40 | Gives a part's value (+0xE8 +0xD0) |
| `0x65aa90` | Window::Mode65AA90 | Sets a mode (+0x168) and acts (slot 0xE4) |
| `0x642a50` | Window::Skin642A50 | In outline: a skin texture (SkinItem::Texture with 1) |
| `0x638f30` | Windows::Memory638F30 | In outline: the machine's memory status (GlobalMemoryStatus) |
| `0x624580` | Window::Flag624580 | In outline: when flagged (bit 1 of +0x10), a step |
| `0x624560` | Window::Flag624560 | In outline: when flagged (bit 0 of +0x10), a step |
| `0x624540` | Window::Flag624540 | In outline: when flagged (bit 4 of +0x68), a step |
| `0x624520` | Window::Flag624520 | In outline: when flagged (bit 0 of +0x68), a step |
| `0x6ea070` | Window::Panel6EA070 | In outline: a panel step (Panel::ChildrenSlot4ThenLayout) and lays out |
| `0x6c87c0` | Window::Part6C87C0 | In outline: with no part (+0x58), a step (slot 0x58) |
| `0x65b400` | Window::Call65B400 | In outline: a call on a part (+0x16C) with a value (+0x178) |
| `0x6ea020` | Window::Mode6EA020 | In outline: mode steps (Window::Mode6A7DA0, Window::Mode6A8400 with 0) |
| `0x6a7b70` | Window::Set6A7B70 | Sets a part's value (+0xE4 +0x150) |
| `0x68d5e0` | Window::Children68D5E0 | In outline: a step (slot 0x100) and children (Window::ChildrenSlot2) |
| `0x68d540` | Window::Offset68D540 | Gives a part's value (+0xE0 +0x128) less a value (+0xFC) |
| `0x669640` | List::Free669640 | In outline: for a list, frees its entries (List::Free6695B0) |
| `0x6543f0` | List::Free6543F0 | In outline: the same (List::Free654280) |
| `0x62ac40` | Window::Call62AC40 | In outline: a step (Window::Slot48Self) and a call on a part (+0x144) |
| `0x42a3c0` | Window::Children42A3C0 | In outline: children (Window::ChildrenSlot4) and lays out |
| `0x6fee20` | Window::Step6FEE20 | In outline: a step (Window::Step6AB170) and gives its argument |
| `0x6ef410` | Window::Shared6EF410 | In outline: with a shared object (`0x13060FC`), tells it (its first slot with 1) |
| `0x6b8cd0` | Window::Part6B8CD0 | In outline: a part step (Window::Part642E80) and gives its argument |
| `0x5fc0f0` | Window::Part5FC0F0 | In outline: the same (another copy) |
| `0x69b150` | Window::ImeClose69B150 | Tells the input method editor to close its candidate list (ImmNotifyIME 0x11) |
| `0x69b140` | Window::ImeCancel69B140 | Tells the input method editor to cancel its composition (ImmNotifyIME 0x15, 1) |
| `0x6ed400` | Window::Call6ED400 | In outline: tells a part (+0x168, slot 4) |
| `0x699e90` | Window::Value699E90 | In outline: reports a missing resource (Report::MissingResourceSymbol2) and gives a value |
| `0x670730` | Window::Part670730 | In outline: with a part (+0x58), a step (Window::Step678190) |
| `0x700510` | Window::Call700510 | In outline: tells a part (+0xD4, slot 0xE4) |
| `0x67ab70` | Window::Address67AB70 | Gives the address of a part's value (+0xF0 +0xE4) |
| `0x67ab60` | Window::Address67AB60 | Gives the address of a part's value (+0xF0 +0x114) |
| `0x665910` | Buffer::Free665910 | Frees a buffer |
| `0x665840` | Buffer::Free665840 | The same (another copy) |
| `0x665770` | Buffer::Free665770 | The same (another copy) |
| `0x611ae0` | Buffer::Free611AE0 | The same (another copy) |
| `0x6acac0` | Window::Shared6ACAC0 | Sets a shared value (`0x1305CF0`) |
| `0x6ef460` | Window::Slot6EF460 | Gives slot n (+8) |
| `0x6ef450` | Window::Count6EF450 | Adds one to slot n's count (+8) |
