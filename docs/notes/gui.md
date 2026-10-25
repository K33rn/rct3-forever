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
| `0x6c8cd0` | ChallengeSetupDialog::Layout | Lays out the challenge setup dialog (29 rectangles; a Norwegian special case). |
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
