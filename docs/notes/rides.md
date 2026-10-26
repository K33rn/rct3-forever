# Rides

Tracked rides, flat rides and their cars and trains.

| Address | Name | What it does |
|---|---|---|
| `0x51ee60` | RideCar::FindBones | Sets up a ride car from its model: looks up its bones by name (`Pivot`, `CarFront` / `CarRear`, the cameras `CameraFront`, `CameraRear`, `NoseCamera`, `BombCameraMid`, `Tank`, `LinkFront` / `LinkRear`, axles `AxleF` / `AxleR` (also spelt `AxelF` / `AxelR`), wheels `WheelFL`, `WheelFR`, `WheelRL`, `WheelRR`, `WheelL`, `WheelR`) and the restraint animations, and reports models that break the rules (`missing bone Pivot for car`, `Axles not allowed for link car:`, `Seperate wheels not allowed for link car:`, `missing .shp/.shs for LOD`, `Unexpected number of restraint anims`) |
| `0x5dc5b0` | MiniGolf::SetupAnimations | Sets up a mini golf course's player animations: walking on, walking off and looping (`WalkOn`, `WalkOff`, `Loop`, `Walk`), each hole's strokes (`%02dStroke%02d`) and the ball. Called from `0x59AD50` |
| `0x529ca0` | RideCarSpinningPart::Update | Moves a spinning part of a ride car (virtual slot 12 of RideCarSpinningPart): its angle and angular speed from the car's motion and gravity (-9.81), with friction and the angle kept within a full turn (2 pi) |
| `0x570bd0` | RideTrainInstance::Update | A train's update on its track (virtual slot 16 of RideTrainInstance): moves its cars, plays the guests' screams (`TrackedRideGuestScream`), splashes (`SplashSound`), boosters (`Boost`) and locking to a spiral lift hill (`LockToSpiralLiftHill`) |
| `0x4d8310` | SavedSelection::Save | Saves a track or pool design to the player's files: coasters to `\Coasters\` / `Coaster Designs\` as `.trk` (numbering the name `(%d)` if it exists, with the scenery `CoasterSceneryItems`), pools to `\Pools\` / `Pool Designs\` as `.pool`, and reports success (`TrackDesignSavedSuccessfully`, `PoolComplexSavedSuccessfully`). Slot 69 of `0xF33D30` |
| `0x5b5d80` | TrackedRideInstance::SetupTrack | Sets a tracked ride up from its track data (virtual slot 3 of TrackedRideInstance), with the rides whose track is a fixed file handled specially: crazy golf, elevator, launcher free fall and roto drop (`<name>:trr`, `Tracks\TrackedRides\<name>\<name>`); the special track pieces it looks for are the speeds (`Slow`, `Med`, `Fast`), `Chain`, `Stop`, `Start` and `WaterPlume` |
| `0x87fd50` | SIDDatabaseEntry::Load | Loads a ride or scenery definition (virtual slot 2 of SIDDatabaseEntry): its track files (`Tracks\Coasters`, `Tracks\TrackedRides`, `\Track`), the fixed-track rides' models (`Style\Vanilla\Rides\CrazyGolf\CrazyGolf`, `Elevator`, `LauncherFreeFall`, `RotoDrop`) and their named pieces (`Station`, `Straight`, `PathCorner`, slopes, the crazy golf holes `Hole01` ...) |
| `0x52dac0` | RideCarSwingingPart::Update | Moves a swinging part of a ride car (virtual slot 12 of RideCarSwingingPart): its swing angle and speed from the car's motion, with damping, the angle kept between -pi and pi |
| `0x508360` | RideCar::UpdateEffects | Shared by every kind of ride car (virtual slot 7 of RideCarInstance, RideCarCrash, RideCarSpinningPart, RideCarSpinSwingPart, RideCarSwingingPart): the car's effects in the world: splashing into water (`WaterManager`), the terrain under it, its lights (`0x868000`) and sounds |
| `0x56dc80` | RideTrain::BuildCars | Builds a train's cars from the ride's car definitions (front, second, middle, penultimate, rear, link cars), creating each `RideCarInstance`, and reports missing parts (`no SVD for car`, `no .shp/.shs for car`, `missing .shp/.shs for LOD`, `train %s: no car for %d of %d ...`, `Undefined ride car`). Called from six places |
| `0x4e30f0` | AttractionEditor::Build | Builds the coaster editor's window: view, track area, rotate, state chooser, colour, new track segment, the robot arm angle editor, and the track-building window itself (`TrackEditor::Build`). Called from `0x634D90` and `0x66BC30` |
| `0x5189a0` | RideCar::ReadCameraSettings | Reads a ride car's on-ride camera settings by name: `Lookahead`, `G disp mult` / `max` / `smoothing`, `Shake min speed`, `Shake factor`, `Shake H max` / `V max` (and their `chain` versions for chain lifts) and `Camera Smoothing`. Called from `RideCar::FindBones` |
| `0x519540` | RideCar::CreateParts | Creates a ride car's moving parts: spinning (`RideCarSpinningPart`) and swinging (`RideCarSwingingPart`) parts, refusing them on link cars (`Swing/spin part not allowed for link car:`, `illegal ride car part`). Called from `RideCar::FindBones` and `0x519B30` |
| `0x5ce1d0` | Platform::FindPieces | Finds a station platform's pieces by name (`Start`, `End`, `StartEnd`, `Middle`, stairs `2stair`, `1stairLeft`, `1stairRight`, and the track ends `trackStart`, `trackEnd`, `trackStartEnd`), reporting `missing SID for platform`. Called from `0x4F1050` and `0x5CF540` |
| `0x4e9060` | MiniGolfHole::Setup | Sets up a mini golf hole's animations: walking on and off, each stroke (`%02dStroke%02d`), walking and the ball. Called from four places |
| `0x5f2f00` | MiniGolfHole::FindSplines | Finds a mini golf hole's paths: the walk-on and walk-off splines (`missing WalkOn spline`, `missing WalkOff spline`), the strokes (`%02dStroke%02d`), walking and looping |
| `0x5e8480` | TrackSection::MatchFlags | Reads the track-section flags of a ride's track pieces (`ONLYTRACKSECTION`, `ONLYSTARTWITHTRACKSECTION`). Called from six places |
| `0x5af800` | TrackedRide::CheckTrains | Checks that a tracked ride has trains with seats, reporting `No RideTrains for ...` and `RideTrain with no seats` (with the ride and train names). Called from eight places |
| `0x572c50` | Ride::FillWithWater | Fills a ride with water (a water ride's start): plays `StartSound`, `FillWithWaterLoopFader` and `StopSound` |
| `0x4b0bd0` | WhaleShow::Setup | Sets up the whale show: its icon (`WhaleShowIcon`), its fixed parts (`static0`) and camera (`Camera0`). Called from `0x4B22A0` and `0x4B2660` |
| `0xbb47d0` | TrackPiece::ParseFlags | Reads a track piece's flags from its name: inverted (`inv`), chain lift (`chain`), down, booster, capped, joined (`joined`, `joined_r`). Called from `0x4F1050` and `0x849F70` |
| `0x4e8730` | MiniGolfHole::SetupPlayers | Sets up a mini golf hole's player animations (walk on / off, `%02dStroke%02d`, walk, ball). Called from `0x4EA940` |
| `0xa3cb60` | TrackImporter::Import | Imports a track design from `\Coasters\`, with its errors: file not found, unknown file format, unsupported ride type, unsupported or misfit ride part (`TrackImporter_Error_*`) |
| `0xa98400` | Pool::PartFromName | Turns a pool part's name into its kind: `divingboard`, `ladder`, `shower`, `sunlounger`, `light`, `lifeguard`, `wavemachine`, `underwaterwall`, `centralsolid`, `centralwater`, `side`, `wholetile`, ... |
| `0x5a63b0` | Platform::ReadSettings | Reads a station platform's settings from its overlay: `PlatformHeight`, `PlatformOffset`, `WalkAlongPlatform`, `StepXOffset`, `StepXBase`, `InsideEntranceExitXOffset`, reporting the missing ones. Called from `0x5B5710` |
| `0x5956c0` | Stall::AddCarriedItem | Adds a carried item to a stall's extras (`ShopExtraInstance`), reporting `Missing CED '%d' for CID '%S'` and `No CID for carried item`. Called from four places |
| `0x519ef0` | RideCar::FindCameras | Finds a ride car's cameras (`camera01`, `camera02`). Called from `0x5274B0` |
| `0x5df4a0` | ThrillLiftTrain::Setup | Sets up a thrill lift's train and its left and right splines, reporting `No ThrillLiftTrain for ...`, `ThrillLiftTrain: missing lspline` / `rspline`. Called from `0x59C840` and `0x5E92A0` |
| `0x5cc010` | Ride::SetupWaterSpray | Sets up a ride's water spray particles (`ParticleStart05_rsWaterSpray`, `ParticleEnd05_...`, `ParticleStart11_...`) |
| `0x59b230` | TrackSection::Check | Checks a ride's track sections, reporting unresolved ones (`Unresolved TrackSection`, `see trr file`) and missing resources (`Track Section '%s' - '%s' is missing water spline resources!`, `... is missing resources!`). Called from five places |
| `0x562520` | RideTrain::SetupSounds | Sets up a train's track sounds: slow, medium and fast (left and right), the chain lift and water plumes (`TrackSlowSound`, `TrackMedSound`, `TrackFastSound`, `ChainSound`, `WaterPlume`). Called from `0x564AF0` and `0x5AA920` |
| `0x4a4030` | Attraction::LoadIcon | Loads an attraction's icon, reporting `No :sid`, `Icon missing texture for attraction` and `unknown attraction type`. Called from `0xA7FF10` |
| `0xa30ae0` | FlexiColours::Load | Reads an object's colour choices (`DefaultFlexiColours`, `Flexicolour_`). Called from `0xA2E200` |
| `0x5370b5` | Ride::CheckEntrances | Checks that a ride has the right entrances and exits and says what is wrong (`MissingEntrance`, `NoEntrance`, `MissingExit`, `NoExit`, with developer messages such as `XXX Must have exactly 1 entrance`), for rides and pool rides |
| `0x5819e0` | RideCar::FindWheels | Finds a ride car's four wheels (`WheelFR`, `WheelFL`, `WheelRR`, `WheelRL`). Called from `0x581F20` |
| `0x564af0` | FixedRide::LoadCar | Loads the cars of the fixed-track rides: elevator, launcher free fall and roto drop (`Cars\TrackedRideCars\<name>\<name>`, `:rit`) |
| `0x532110` | RideBreakdown::Names | Makes the names of the kinds of breakdown: none, emergency stop, restraints or doors that fail to open or close, ... (`RideBreakdownType*`). Called from `0x4B9D60` and `0x53A490` |
| `0x4e9e50` | MiniGolfHole::SetupStrokes | Sets up a mini golf hole's stroke animations (`%02dStroke%02d`, walk, ball). Called from `0x4EB1F0` and `0x4EB650` |
| `0x5a6db0` | RideTrain::Load | Loads a ride's trains from `Cars\CoasterCars` or `Cars\TrackedRideCars`, reporting `Unresolved RideTrain`. Called from three places |
| `0x7218e0` | PathType::FromName | Turns a path type's name into its kind (`Asphalt`, `Crazy`, ...) |
| `0x5c9640` | RideSound::Rotate | Plays a ride's rotation start and loop sounds (`RTCRotateStart`, `RTCRotateLoop`). Called from four places |
| `0xa8a240` | PoolComplex::PeopleText | Writes how many people are in a pool complex (`PeopleInPoolComplex0` ... `4`, `X`). Called from `0xA8AE20` |
| `0x5b5710` | TrackedRide::SetTrains | Sets a tracked ride's trains from its `SETTRAIN` lines (`None`), then checks them (`TrackedRide::CheckTrains`). Called from `TrackedRideInstance::SetupTrack` and two others |
| `0x5a7380` | RideCar::Name | Gets a ride car's name (`Name`, `CarName`, or `<car name undefined>`). Called from `0x5A78D0` and `0x66CE00` |
| `0x5defd0` | CableLiftTrain::Setup | Sets up a cable lift's train (`RideTrainInstance`), reporting `No CableLiftTrain for ...`. Called from `0x59C7E0` and `0x5E92A0` |
| `0x5deb00` | CableLiftTrain::Setup (2) | The same as `0x5DEFD0` for the other direction. Called from `0x59C780` and `0x5E92A0` |
| `0x519b30` | RideTrain::LoadCarModel | Loads a ride car's model (its `.shp`), reporting `missing .shp for car` / `no .shp for car`. Called from `RideTrain::BuildCars` |
| `0xbc4240` | TrackPiece::SpecialNames | Makes the names of the special track pieces: straight brakes and block brakes, station middle, photo section, S-bends (also tilted), corkscrews, half and quarter loops, loops, ... Called from the TrackSpecial window and two others |
| `0x5a3f50` | Ride::FindInspectionPoints | Finds a ride's inspection points for mechanics (`InspectionPointLeft`, `InspectionPointRight`) |
| `0x66db80` | CoasterStats::LoadExample | Loads the example track for the statistics window (`[Giga]Expedition GeForce.trk`, read through `TrackedRideInstanceSaveHeader` / `TrackedRidePreview`). Called from `0x66FA40` |
| `0xa82700` | PoolComplex::LoadPreview | Reads a saved pool's preview (`PoolComplexSaveHeader`, `TrackedRidePreview`). Called from `0xA84580` |
| `0x544740` | RideOpModeDialog::ModeText | Looks up the text of an operating mode (`RideOpModeDialog_` + the mode's name), reporting `Undefined symbol`. Called from `RideOpModeDialog::Build` |
| `0x544250` | RideOpModeDialog::LoadIcon | Loads an operating mode's icon, with `SquarePlusSign` and `missing icon` as fall-backs. Called from `RideOpModeDialog::Build` |
| `0x4d14b0` | AttractionNumbers::Rate | Writes an attraction's figures per hour or per week (`AttractionNumberArea_PerHour`, `_PerWeek`). Called from three places |
| `0xbbfa30` | SwingMode::FromName | Turns a ride car's swing mode name into its value: none, free, about R, about F, spinning about U (`SwingMode*`) |
| `0xbb3e90` | AtStation::FromName | Turns a station condition's name into its value: at station, all, any, exactly one, at least one, none (`AtStation*`) |
| `0x589180` | TrackSpecial::Directions | Makes the special track pieces' up, down, left and right buttons (`TrackSpecial_*`). Called from three places |
| `0x87c130` | Blueprint::LoadShaders | Loads the shaders that draw objects as blueprints while placing them (`ShapeBlueprint`, `ShapeNormalEdge`, `TerrainTileBlueprint`). Called from `0x87C930` |
| `0xbb45f0` | LeaveRideBehaviour::FromName | Turns how guests leave a ride into its value: walking off, or as a rigid body (also down a water slide or from a high dive) (`LeaveRideBehaviour*`) |
| `0xbbfde0` | SwingAnchor::FromName | Turns a swinging part's anchor into its value: the parent's pivot or origin (`SwingAnchor*`) |
| `0x55f5a0` | RideTrain::StateName | Writes a train's state: waiting to load, loading, ready to go, restraints closing, doors closing, waiting to go at the station (also in sync with another train), ... (`State*`) |
| `0x535020` | Ride::QueueText | Writes how many people are queueing (`PeopleQueueing0` ... `4`, `X`) |
| `0x5318c0` | Ride::RidersText | Writes how many people are on a ride (`PeopleOnRide0` ... `4`, `X`) |
| `0x532600` | RideBreakdown::StateName | Writes a breakdown's state: awaiting breakdown, calling a mechanic, waiting for them to arrive, being fixed, fixed (`BreakdownState*`) |
| `0x5117e0` | RideCar::MissingBone | Reports a bone missing from a ride car (`missing bone ... for car`). Called from `RideCar::FindBones` |
| `0xb93330` | Attraction::LoadIcon (2) | Loads an attraction's icon, reporting a missing texture. Called from `0xBA89A0` |
| `0x553760` | RideSeating::Names | Makes the ride seating kinds' names: stand, sit, sledge, lay, bike, car, horse, row with oars, ... (`RideSeating_*`). Called from `0x5538E0` |
| `0xbc4c80` | TrackPiece::Flags | Turns a track piece's flag names into values (`Car`, `Join`, `Alt`, `Travel`, `Water`, `AltJoin`, `Speed0` ... `Speed5`). Called from `0x59A500` |
| `0x81d120` | Seats::Names | Makes the seat bone names (`peep%d`, `peep0%d`). Called from `0x81E050` |
| `0x591220` | Seats::Names (2) | The same, as `Peep%d` / `Peep0%d`. Called from `0x5912C0` |
| `0x510520` | RideCar::SeatNames | A ride car's seat bone names (`Peep%d`, `Peep0%d`). Called from `RideCar::FindBones` and `0x5105C0` |
| `0xbbf8a0` | RideAnimation::FromName | Turns a ride animation's name into its kind: starting, running, stopping, idle, restraints opening / closing / idle, doors, ... (`Anim*`). Called from `0xBBFC20` |
| `0x50b390` | RideCar::StateName | Writes a ride car's state: travel starting, travelling, slowing, aligning, aligned, crashing, off the track, restraints closing, ... (`State*`) |
| `0xbb40e0` | OpMode::Names | Makes the operating mode names: continuous circuit, powered launch, reverse inclined launch, boat, race, shuttle, upwards launch, free-fall drop, ... (`OpMode*`). Called from `RideOpModeDialog::Build` and `0xBB42A0` |
| `0x5a92f0` | Ride::RouteErrors | Reports that a ride has no route between its stations or back to a station (`RideNoRouteBetweenStations`, `RideNoRouteBackToStation`). Called from `0x56A660` |
| `0xbc4da0` | Ride::StateNames | Makes a ride's state names: stopped, starting, running, stopping, and the special states (in motion, holding, releasing, returning, water tank, ...). Called from `0xBC6940` |
| `0x59a080` | RideSplines::RenderModeNames | Makes the developer's spline drawing modes' names: off, lines, track, block sections, stations, segments, points, ... (`RenderSplines*`). Called from `0x59A500` |
| `0x5cc7b0` | TrackPiece::Activate | Activates a track piece: resolves its symbols in its overlay (`TrackPiece::Activation(): failed to resolve symbol`), its car transform (`CarTransform`) and scenery |
| `0x485fb0` | RideCar::SetupModel | Sets up a ride car's model and its front camera (`CameraFront`), checking its shapes. Called from two places |
| `0x880fb0` | SceneryItem::Activate | Activates a scenery item definition: finds its visual (`No :svd for :sid`) and checks its models' shapes. Called from `0x8826F0` |
| `0x5c9a90` | RideSound::RotateStop | Plays a ride's rotation stop sound (`RTCRotateStop`) |
| `0x4b5ce0` | Aquarium::Setup | Sets up the aquarium's track (`Aquarium:trr`) |
| `0x4aae00` | FlatRide::Screams | Plays the guests' screams on a flat ride, at random (`FlatRideGuestScream`). Called from `0x4AF910` |
| `0x5341d0` | Ride::CheckReliability | Checks a ride's reliability and warns when it is low (`RideReliabilityLow`). Called from `0x4AF910` |
| `0x583ef0` | PathArrow::Build | Makes the arrow showing a path's direction (`PathArrow`). Called from five places |
| `0x49e670` | Waypoint::Draw | Draws a route waypoint's ring (`WaypointRing`). Called from `0x49ED40` |
| `0x5912c0` | Stall::FindPeepBones | Finds a stall's guest positions, reporting `No peep bones for stall '%S'`. Called from `0x5964F0` |
| `0x4f6200` | TrackEditor::AutoCompleteCost | Refuses to auto-complete a track the player can't afford (`RideAutoCompleteNotEnoughCash`, `%NAME%`). Called from six places |
| `0x487750` | RideCar::FindPivot | Finds a ride car model's pivot (`Pivot`). Called from `0x487B90` |
| `0x5a93f0` | Ride::MissingInspectionPoint | Reports a ride without an inspection point (`Missing inspection point`) |
| `0xa3fc20` | TrackImporter::AddPiece | Adds an imported track piece, matching its curve and flags (lift chain, enclosed, station), reporting `No match for track piece %d`. Called from two places |
| `0x65e590` | TrackImporter::Music | Sets an imported ride's music (`Attraction_Music`). Called from `TrackImporter::Load` |
| `0x88c920` | Sign::NoEntry | Sets up a no entry sign (`Sign_NoEntry`). Called from `0x88CD90` |
| `0xa3dc00` | TrackImporter::ReadTD | Reads a RollerCoaster Tycoon 1 or 2 track design for importing (`.td4`, `.td6`). Called from `TrackImporter::Import` |
| `0x5e4970` | TrackedRide::CreateTrains | Creates a tracked ride's trains (each a `RideTrainInstance` of 0x3F0 bytes) on its track, through the ride's track interface (`0x59ACC0`, `0x5D4870`), placing them by the 4-way kind of ride. Called from `TrackedRide::CheckTrains` |
| `0x5964f0` | StallInstance::Activate | A stall's activation (virtual slot 3, run once by `ManagedObject::Activate`): finds and activates its definition by name, and finds where its customers stand (`Stall::FindPeepBones`) |
| `0x4b2660` | AnimatedRideInstance::Activate | An animated (flat) ride's activation (virtual slot 3): finds and activates its definition, sets up its stations (`0x539090`) and, for the whale show, the show (`WhaleShow::Setup`) |
| `0x5874c0` | SpecialAttractionInstance::Activate | A special attraction's activation (virtual slot 3): finds and activates its definition by name and sets up its parts |
| `0x59cf00` | Track::Activate | A ride track's activation (virtual slot 3): checks its track sections (`TrackSection::Check`) and builds its pieces and splines |
| `0xbc4ba0` | TrackImporter::BankName | Name of a track piece's bank, for the track importer: 0 `Flat`, 1 `45Right`, 2 `90Right`, 3 `135Right`, 4 `Inverted`, 5 `135Left`, 6 `90Left`, 7 `45Left`; null otherwise |
| `0xbc4af0` | TrackImporter::SlopeName | Name of a track piece's slope: 0 `Flat`, 1 `MediumUp`, 2 `MediumDown`, 3 `SteepUp`, 4 `SteepDown`, 5 `VerticalUp`, 6 `VerticalDown`, 7 `JoinsSplines`; null otherwise |
| `0xbc4150` | TrackImporter::CurveName | Name of a track piece's curve: 0 `Straight`, 1 `GentleLeft`, 2 `MediumLeft`, 3 `SharpLeft`, 4 `GentleRight`, 5 `MediumRight`, 6 `SharpRight`, 7 `Special`; null otherwise |
| `0xbcd250` | RideCar::PositionName | Name of a car's position in its train (used by RideTrain::BuildCars): 0 `Front`, 1 `Second`, 2 `Middle`, 3 `Penultimate`, 4 `Rear`, 5 `Link`; null otherwise |
| `0xbbf9f0` | RideCar::WheelName | Name of a car's wheel bone (used by RideCar::FindBones): 0 `WheelFR`, 1 `WheelFL`, 2 `WheelRR`, 3 `WheelRL`; null otherwise |
| `0x4de1c0` | Ride::BreakdownStatusName | Name of a ride's breakdown status: 0 `BreakStatus_Fixed`, 1 `BreakStatus_AwaitingBreak`, 2 `BreakStatus_CallMechanic`, 3 `BreakStatus_WaitMechanicArrive`, 4 `BreakStatus_BeingFixed`; null otherwise |
| `0x4bef70` | Ride::StateName | Name of a ride's open state: 0 `StateClosed`, 1 `StateTesting`, 2 `StateOpen`; null otherwise |
| `0xa3fab0` | TrackImporter::MapPieceCode | Maps an imported piece code (0-9) to the game's, by one of two tables: normally 0 1 2 3 4 5 6 7 3 6, with the flag 0 1 1 2 4 4 5 7 3 6. Called from TrackImporter::AddPiece |
| `0x94e000` | WaterDrops::EmitterCount | How many drops emitter n (208 bytes each at +0x1A4) gives: its +0x10 divided by its interval (+0xC), rounded up; 0 when the interval is not positive. Called from Model::DebugReport |
| `0xbcef20` | WideString::FindLast | Searches a wide text backwards from the given position (or from its last character when it is -1) for a character: its position, or -1. Used by MiniGolfHole::Setup, FindSplines and SetupPlayers |
| `0xa3fa60` | TrackImporter::OppositeDirection | The direction opposite the one given (in eax), of eight: 0-3 become 4-7 and 4-7 become 0-3. Called from TrackImporter::AddPiece |
| `0x424d10` | CustomPersistingSpline::StartDirection | The direction at the spline's start: the first point's +0x18 vector, reversed. Slot 14 of CustomPersistingSpline |
| `0x424d50` | CustomPersistingSpline::EndDirection | The direction at the spline's end: the last point's (36 bytes each, count +8) vector at -0x18 from its end, reversed. Slot 15 of CustomPersistingSpline |
| `0xa7b3b0` | ChangingRoomInstance::Slot57 | What the object's slot 33 gives times `0x4BFA20`'s value when it is positive, else 0. Slot 57 of ChangingRoomInstance |
| `0xa7b3e0` | ChangingRoomInstance::Slot45 | Runs `0x4C35E0`, then sets +0x1E8 to 1. Slot 45 of ChangingRoomInstance |
| `0x4e6600` | TrackedRidePersonInfo::Slot7 | Runs `0x4D1AA0`, then copies +0xB8 into +0xF0. Slot 7 of the tracked ride, mini golf and walk-along-track person infos |
| `0x84b2e0` | TrackPiece::Slot18 | Asks slot 18 of the part at +0x40 of the piece's interface (`0x130BBCC`), or 0 when it has none. Shared by TrackPiece and PoolPiece |
| `0x84b300` | TrackPiece::Slot17 | The same for slot 17. Shared by TrackPiece and PoolPiece |
| `0x424e70` | CustomPersistingSpline::Length | The spline's length: when it has changed (+0x1C) recomputes it (after `0x423DE0` and `0x424540`) as the sum of the segment lengths (slot 5) from +0x10 to the last point, keeping it at +0x20 and clearing the flag; returns it. Slot 3 of CustomPersistingSpline |
| `0x52bc80` | RideCarSpinSwingPart::ResetMotion | Forgets the part's tracked motion: clears the count of known values (+0x1C8) and the position, velocity and acceleration kept by RideCarSpinningPart::TrackMotion (+0x1D0 to +0x1F8). Slot 14 of RideCarSpinSwingPart |
| `0x5292a0` | RideCarSpinningPart::Reset | RideCarSpinSwingPart::ResetMotion, then clears +0x214, +0x218 and the flag at +0x1C0. Slot 14 of RideCarSpinningPart |
| `0x52ce80` | RideCarSwingingPart::Reset | RideCarSpinSwingPart::ResetMotion, then clears +0x2A0 to +0x2AC and the flag at +0x1C0. Slot 14 of RideCarSwingingPart |
| `0x52f560` | RideInstance::Slot18 | When the ride's slot 19 agrees to the value (with 0), sets it with AttractionInstance::SetValue70. Slot 18 of RideInstance |
| `0x5749e0` | RideTrainInstance::Slot9 | Runs the train's slots 14 and 15 with the argument, then for each car (count +0xC4) `0x561240` and `0x526A70`, then slot 16 and `0x5725D0`. Slots 9-11 of RideTrainInstance |
| `0x551210` | RideParticleManager::Update | Passes the frame time on to its four members (+8, +0x38, +0x80 and +0x98) through their update functions. Slot 0 of RideParticleManager's update table |
| `0x561240` | RideTrainInstance::Car | Car n of a train: the interface (id in `0x12F50D4`) of the object entry n of the car table (+0xC0, 28 bytes each, count +0xC4) refers to, looked up once per entry; null past the end |
| `0x5a7300` | TrackedRideInstance::Entry | Entry n of the table at +0x6AC (28 bytes each, count +0x6B0): the interface (id in `0x12F537C`) of the object it refers to, looked up once per entry; null past the end |
| `0x4ac5a0` | AnimatedRideInstance::SeatPoint | Gives seat or inspection point n of the ride (from the scenery item of its model, through the interfaces `0x130BBEC` and `0x130B5C8`); when the ride's model has no such point, shows a message box `Attempt to access missing seat/inspection point` naming the ride and the point (`%s seat/inspection point %d`) |
| `0xa3f580` | TrackImporter::MatchRCT1 | Finds the RCT3 ride for an imported RCT1 track and vehicle: reports `Creating RCT1 ( Track %d Vehicle %d ) as ( Track '%s' Vehicle '%s' )` when it finds one and `No match for RCT1 ( Track %d Vehicle %d )` when it does not |
| `0xbb42a0` | OpMode::FromName | The number of the ride operating mode whose name (OpMode::Names, `0xBB40E0`) matches the text given, case ignored (the text made plain from wide, `OpMode` meanwhile) |
| `0x5538e0` | RideSeating::FromName | The number of the seating type whose name (RideSeating::Names, prefix `RideSeating_`) matches the text given, case ignored |
| `0xbbfc20` | RideAnimation::FromName2 | The number of the ride animation whose name (RideAnimation::FromName's list) matches the text given, case ignored |
| `0x52c350` | RideCar::CheckPivot | Finds the `Pivot` dummy in a ride car's shape; when it is missing, shows a message box `'Pivot' dummy is missing in the shape` |
| `0x4d4650` | Slide::Update | In outline: updates a slide ride (`Slide`), timing riders with the game clock and Random::Ran0 |
| `0x487b90` | RideTrain::LoadCars | In outline: loads a train's cars (RideTrain::Load), turning their parts with the shape turns, `None` when a car has no part |
| `0x9c7b00` | Particle::NameText | In outline: the name text of a particle effect, made from `ParticleName_` and the effect's name |
| `0x4ad2f0` | RobotArm::Update | In outline: updates the robot arm ride (`RobotArm`) and its riders, with Random::Ran0 |
| `0x4ad000` | RobotArm::Rider | In outline: finds a robot arm rider's person (`RobotArm`) |
| `0x533ab0` | Queue::SignName | In outline: finds a queue's sign (`queuesign`) by name |
| `0x5351e0` | RideCar::PeepSlot | In outline: the seat slot of a ride car (`Peep01` ...) |
| `0x66ce00` | RideDetails::CarName | In outline: shows a ride car's name in the details window (RideCar::Name), `None` when there is none |
| `0xa3f770` | TrackImporter::Coaster | In outline: matches an imported track to a coaster (`Coaster`) |
| `0xb048a0` | DartHelicopter::Start | In outline: starts the dart helicopter (`DartHelicopter`) with the main camera |
| `0x581750` | RobotArm::JointName | In outline: the robot arm's joint names (`IK_RobotJoint` + number) |
| `0xab3b50` | PoolSegment::Sound | In outline: makes a pool segment sound (`PoolSegment`) |
| `0xb05170` | FireDart::Sound | In outline: makes the fire dart sound (`FireDart`) after loading the darts |
| `0x85c890` | EarthThump::Sound | In outline: makes the `EarthThump` sound at a random time |
| `0x54ed80` | Burn::Sound | In outline: makes the `Burn` sound |
| `0x9d26f0` | PMSound::Data | In outline: the `PMSoundData` entries (numbered) |
| `0x4cd2b0` | Developer::BreakAllRides | The developers' BreakAllRides command: goes over every attraction and sets each one to break down on its next loop, reporting `All rides set to break on next loop...` |
| `0xbb5cb0` | TrackSection::TKSCount | In outline: reads a `TKSCount` value from a track section's name |
| `0xbbe6d0` | Model::HandAttachment | In outline: finds the `attachmentdummyrhand` attachment point (case ignored) |
| `0x82a3d0` | Ride::RunningSound | In outline: makes a ride's `RunningSound` |
| `0x882f20` | Resources::StyleFolder | In outline: finds the styles (`style`) with Resources::FindStyles |
| `0x875e40` | Supports::Path | In outline: builds a path in the `Supports` folder |
| `0x44e930` | Helicopter::Setup | In outline: sets up the `Helicopter` |
| `0xad0630` | RotationSpeed::Text | Writes a rotation speed with the unit `rads/s` |
| `0xcc1020` | Model::FindHead | Finds the `Bip01 Head` bone by name |
| `0xc23570` | Model::BoneName | A bone's name, or `Unknown bone` |
| `0xc28e90` | Model::BoneName2 | A bone's name from the model's list, or `Unknown bone` |
| `0xb0e2f0` | Path::LoadNodes | Reads a `pathnodearray` from the data |
| `0x4efb90` | Track::Name | The `Track_` name text |
| `0x4c1000` | Attraction::ClosedText | The `Closed` text |
| `0xa67b40` | Buggy::Name | The `Buggy` name text |
| `0xc29300` | Skeleton::PoseC29300 | In outline: works out a pose for a set of bones: makes 0x90-byte working entries, finds each bone (Pairs::FindSorted, `0xC23470`), and turns bones toward a target (Vector::RotateAround, arctangents) |
| `0xc26020` | Skeleton::UpdateC26020 | In outline: updates the bones of a model's animation: for each bone with the right flags (+0x50 & 0x23) and blend under 1, blends its pose (`0xC5A1F0`) and builds the matrices of its parts (+0x20, +0x2C) |
| `0xc26ef0` | Skeleton::UpdateAllC26EF0 | In outline: the same bone update as Skeleton::UpdateC26020 for every bone (no flag check) |
| `0xc29ed0` | Skeleton::PoseC29ED0 | In outline: another bone posing pass like Skeleton::PoseC29300: finds each bone of a chain (`0xC23470`), sets it up (`0xC21380`, `0xC220E0`, `0xC258A0`) and turns the bones toward a target |
| `0x50d310` | Car::UpdateVisual50D310 | In outline: updates a ride car's look each frame (when shown, +0x250): its lights (Model::UpdateLights), its two sides (`0x50B7E0`), its parts' matrices (`0x869400`) and its animated parts (+0x8B4) |
| `0xcc76c0` | Skeleton::BuildMatricesCC76C0 | In outline: builds the matrices of a model's bones, children first (it calls itself), for the bones that use the part given |
| `0x7bf550` | Track::HitTest7BF550 | In outline: whether a ray or point hits a track piece's parts: checks each part of the piece and its extra parts (`0xBBE5C0`), with special cases for kinds 2 and 3 |
| `0x5c79a0` | TrackedRide::HitTest5C79A0 | In outline: whether a ray hits a tracked ride's track (TrackedRide::TrackInterface): goes over its pieces (`0xCDEB10`) and their triangles, keeping the nearest hit |
| `0x56a660` | Ride::PlaceEntrance56A660 | In outline: places a ride part at a terrain cell (RCT3Terrain, a 90 by 90 grid, Grid::CellAt) facing the right way (Vector::Quadrant), with a special case for kind 0x2F |
| `0xc220e0` | Skeleton::UpdateBone | In outline: updates one bone and then its children (it calls itself): blends its pose (`0xC5A1F0`) when its blend is under 1 and builds its matrix |
| `0x861ae0` | Model::BoneMatrix861AE0 | In outline: the matrix of a model's bone n: from its animation (`0xCEF080`) when it has one, otherwise from its node (Table2C::Value, Nodes::MatrixFor) |
| `0xc27d80` | Skeleton::UpdateAllC27D80 | In outline: another whole-skeleton bone update like Skeleton::UpdateAllC26EF0 |
| `0xbbbfe0` | Track::BuildBBBFE0 | In outline: builds a track's sections from a list (or all of them): counts them (List::Count), sets each up (`0xBB65A0`) and, for kind 2, joins them (`0xBBBD60`, `0xBBBE30`) |
| `0x56fb20` | Train::Update56FB20 | In outline: a train's update: checks its first and last cars (RideTrainInstance::Car: speed +0x74C, flag +0x744) and goes over all its cars, with special cases for kind 6 and ride kind 0x2D |
| `0x7c6580` | Model::Draw7C6580 | In outline: draws a model by its kind (+0xB8: 0x112, 0x11F, 0x13C to 0x13E have their own case), setting up the matrices and device (`0xBEE460`, `0xC1FC30`) |
| `0x4e7590` | AttractionPerson::Update4E7590 | In outline: the update of a person on an attraction (AttractionPersonInfo::Person): unless flagged (+0x118), checks its box against the ride's parts (`0xBCC740`) by its state (+0x13C) |
| `0x5ee150` | AttractionPerson::Update5EE150 | In outline: another attraction person update: finds the person (AttractionPersonInfo::Person) and its place (`0x5EBEB0`, `0x5F2460`), and acts by its slot (+0x11C: -1, -2 or a slot) and flags (+0x190) |
| `0x5dbd70` | TrackedRide::Update5DBD70 | In outline: a tracked ride's update for the selected one (`0x12F5608`, kind 3 or flag 0x40): its track (TrackedRide::Interface2, TrackInterface), its two ends (`0x5CA360` for 1 and 0) and its sections (+0x118) |
| `0xc248e0` | Skeleton::AimC248E0 | In outline: turns a chain of bones toward a target point (arctangents, Vector::RotateAround), like Skeleton::PoseC29300 |
| `0xc2adf0` | Skeleton::DebugDrawC2ADF0 | In outline: draws a model's bones for the developers: each bone and its links in greys and reds (`0xBF6E70` with colours 0xFFBFBFBF, 0xFF7F7F7F, 0xFFBF0000, 0xFFFF0000) |
| `0xc286d0` | Skeleton::UpdateAllC286D0 | In outline: another whole-skeleton bone update like Skeleton::UpdateAllC26EF0 |
| `0x509c50` | Car::SetMode509C50 | In outline: switches a ride car's mode (+0xE0, 3 handled first) to the one given, checking its parts (`0x5094B0`) |
| `0x5e77c0` | TrackedRide::CheckPlace5E77C0 | In outline: checks a track piece being placed (when +0x68 is clear) with its two rotations (Clamp0To3), the ride's track (TrackedRide::TrackInterface) and the tile it is on (OffsetRef::Owner2) |
| `0x5c4eb0` | TrackedRide::HitTest5C4EB0 | In outline: whether a ray hits a ride's part (`0xBC7940`) when it is shown (`0x5C1590`, not flag 0x10000), turning the test half a turn (Matrix::TurnAboutZ by pi) for parts flagged 0x10 |
| `0x5c4740` | TrackedRide::Select5C4740 | In outline: selects or highlights a ride's part at a cell (for rides of kind 3 or flag 0x40 only, otherwise `0x5C2590`), by its flags (+0x190, +0x18C) |
| `0x932160` | TrackedRide::WaterSplash932160 | In outline: for a water ride's track (TrackedRide::TrackInterface) on a tile of kind 2 or more (OffsetRef::Owner2), works out the splash from the car's height and speed with the water flow manager (WaterFlowManager::Get) |
| `0x5094b0` | Car::Attach5094B0 | In outline: attaches a ride car to its place on the track (+0xDC, Table310::Entry, kinds 2 and 3), pointing it the right way (Matrix::LookAlongZSide) |
| `0x517c10` | Car::SetSeat517C10 | In outline: moves a ride car's rider to seat n (below the seat count `0x50E6D0`) when it changes (+0x318), refreshing the link (`0x4E6410`) |
| `0x4e6ca0` | AttractionPerson::Update4E6CA0 | In outline: another attraction person update like AttractionPerson::Update4E7590: for the selected attraction, measures the way to its spot and checks its parts |
| `0x4f7140` | TrackEditor::Update4F7140 | In outline: the track editor's update: refreshes the ride (`0x4EDF70`) for the selected one (`0x12F5608`), and in mode 5 with a piece (+0x52C) works out the cost of finishing the track (TrackEditor::AutoCompleteCost) |
| `0x553060` | TrackedRide::Sections553060 | In outline: goes over a ride's track sections (`0x12F5338`) and lays out each one (`0x559D10`, Interface20::GetCached, `0x552BC0`) |
| `0x4ff730` | Ride::Footprint4FF730 | In outline: when the ride is not being moved (+0xC4 +0xD4), finds the cells its parts cover (Grid::Range) on the terrain |
| `0xbbb690` | Track::BuildBBB690 | In outline: goes over a track's sections (+0x50) measuring them (PointSet::Walk2) and flags the ones past the limits |
| `0x4e7e00` | AttractionPerson::Update4E7E00 | In outline: another attraction person update like AttractionPerson::Update4E7590, checking its box against the ride (`0xBCC740`) |
| `0x526100` | Car::Splash526100 | In outline: when shown (+0x250), checks the water under a ride car (Grid::CellAt, WaterManager::Get, `0x93DBF0`) to make a splash |
| `0x523a00` | Ride::Update523A00 | In outline: a ride's update when not flagged (+0x180): goes over its parts (+0x7B4, `0x518300`) and updates the ones not done (+0x24) |
| `0x5b9180` | TrackedRide::Sections5B9180 | In outline: for a ride in mode 3 (+0xB0), goes over its track sections (`0x12F5338`), checks each one (`0x559BF0`) and lays out the flagged ones (+0x850, `0x559D10`) |
| `0x588260` | AttractionPerson::Walk588260 | In outline: moves a person toward their spot on an attraction (AttractionPersonInfo::MoveTowards), turned by the spot's facing (Matrix::FacingRotation) |
| `0x4e6670` | AttractionPerson::Update4E6670 | In outline: another attraction person update like AttractionPerson::Update4E6CA0, when it has a target (+0x128) |
| `0x5c58b0` | TrackedRide::Ends5C58B0 | In outline: works out a ride piece's end points and directions (`0x5C0200`), normalising each |
| `0x4f26a0` | Ride::Linked4F26A0 | In outline: whether a ride part is linked to another: the same one, or in mode 3 with a link (+0x4A8) matching one of its ends (+0x34, +0x3C), and for the selected ride (`0x12F5608`) |
| `0x5e6c30` | TrackPiece::Place5E6C30 | In outline: places a track piece unless blocked (`0x5D9720`): makes it (TrackPiece::TrackPiece), sets it up at its place (`0x5CDB00`) and, when asked, for the selected ride (`0x12F5608`) |
| `0x5ed1f0` | Ride::Update5ED1F0 | In outline: a ride's update by its part's kind (`0x5316A0`, +0xAC: 0, ...) |
| `0x5cd4f0` | TrackedRide::Sounds5CD4F0 | In outline: turns a ride's sounds with it (RideSound::Rotate, +0x21C and +0x224) when on (+0x218), and refreshes (`0x5C1070`, `0x5D4940`) |
| `0x5ca360` | TrackedRide::End5CA360 | In outline: sets up one end of a ride's track (by the flag given) from the middle of its span (Span::Middle) with the current tool (`0x12F5638`, +0x114) |
| `0xab51d0` | PoolPiece::PlaceAB51D0 | In outline: places a pool piece (PoolPiece::PoolPiece) when not flagged (+0x194), with its kind (`0xBCA7E0`) and its owner (`0xAA96E0`) |
| `0x573ef0` | Ride::PlaceEntrance573EF0 | In outline: places a ride's entrance or exit: with no slot (+0x194 below 0) the plain way (Ride::PlaceEntrance56A660), otherwise among its slots (+0x192) on the terrain |
| `0x5bb960` | Car::Brake5BB960 | In outline: changes a ride car's speed by an amount, kept within its limits (+0, +0x10) when shown (+0x250) |
| `0xa9b950` | PoolComplex::SceneryA9B950 | In outline: refreshes a pool's parts (`0xA9B800`, `0xA9AA60`) and makes a scenery item for it (SceneryItem::SceneryItem) |
| `0xbc2330` | Track::BuildMeshBC2330 | In outline: builds a track's mesh: its sections (Track::BuildBBBFE0) and their vertices (64 bytes each, `0xBEE8C0`) |
| `0xab45d0` | PoolComplex::BuildAB45D0 | In outline: builds a pool complex's parts (`0xA9A440`, `0xAC2BC0`, `0xA85410`) when not flagged (+0x194) |
| `0xbc7350` | Track::CompareBC7350 | In outline: a sort comparison of two track pieces by their snapped height (Grid::SnapIndex with `0x10F2B30`, `0x13D0A80`), then `0xBC5480` |
| `0x5c7450` | TrackedRide::Special5C7450 | In outline: for ride kinds 0x4A to 0x4C, when not flagged (+0x18C), sets up their special part (`0x5BD5A0`) |
| `0x516e90` | Car::Move516E90 | In outline: moves a ride car on by a time (`0x10EE444`) when on (+0xC0), with its train's cars (+0x7B4, `0x50E6A0`) |
| `0xabbc00` | PoolComplex::CheckABBC00 | In outline: for kinds outside 2 to 4, when not flagged (+0x178), checks a pool piece (`0xABC990`) and its first part (`0xAB1020`) |
| `0x5c0200` | TrackedRide::EndPoints5C0200 | In outline: works out a ride piece's end points (used by TrackedRide::Ends5C58B0) unless flagged (+0xAC), with its slot (+0x26C) |
| `0x4d50a0` | AttractionPerson::Place4D50A0 | In outline: when not flagged (+0x74), places a person on an attraction (AttractionPersonInfo::Person) with its matrix (Matrix::Copy) and tile (OffsetRef::Owner2) |
| `0x5e2550` | TrackedRide::Rebuild5E2550 | In outline: refreshes a ride (`0x5A3A30`) and goes over the selected ride's pieces (`0x12F5608`, +0x16C chain) making each one again (TrackPiece::TrackPiece) |
| `0x4f2cb0` | Ride::Linked4F2CB0 | In outline: for rides in mode 3, checks a part with flag 0x400000 and more than two ends (+0xE4) against the shared list (`0x130BBEC`) |
| `0x5eba30` | TrackedRide::Check5EBA30 | In outline: when not flagged (+0x118), checks the ride's piece (kind 3 or flag 0x40 only) |
| `0x5b9c60` | Ride::Place5B9C60 | In outline: places a ride part on the terrain along a direction (normalised from `0xF3EF30`) |
| `0x736d50` | Ride::Part736D50 | In outline: refreshes a ride's part (`0x5316A0` on +0xA4), or the plain way (`0x72D2D0`) |
| `0x563e20` | Ride::Spin563E20 | In outline: for ride kinds 0x2E and 0x2F, turns the ride by its speed (+0x184) over time (Matrix::TurnAboutY) |
| `0x5ea110` | TrackedRide::Update5EA110 | In outline: a tracked ride's update when not flagged (+0x68): its track (TrackedRide::TrackInterface) and its train's flag (+0x64 +0xB4) |
| `0xbc6f00` | Track::JoinBC6F00 | In outline: whether two track pieces join: their ends (flag 0x400000, at least three) and their kinds (`0xBC4B50`, not 7) |
| `0xab4b70` | PoolComplex::LinkAB4B70 | In outline: links a pool piece to its complex (`0x13CE028`, `0xA9F290`, `0xA9A440`) |
| `0x5a81b0` | AttractionPersonInfo::Make | In outline: makes the person record for an attraction by its kind: mini golf (MiniGolfPersonInfo) for kind 9, others for 0xD and more |
| `0x5e0c10` | TrackedRide::CanPlace5E0C10 | In outline: whether a track piece can be placed: its ride's track (TrackedRide::TrackInterface) and its check (`0x4C13C0`) |
| `0x717840` | Path::Act717840 | In outline: acts on a path piece by its case (0: `0x716C30`, ...) and its check (Path::Check717400) |
| `0xa8e130` | PoolComplex::CopyA8E130 | In outline: makes a pool complex record with four 20-byte links from another one when it is not flagged (+0x194) |
| `0x5ab390` | Track::Make5AB390 | In outline: makes a track for a ride (`0x533E20`, Track::Track) and refreshes it (`0x5A3A30`) |
| `0x5c6180` | TrackedRide::Check5C6180 | In outline: for pieces other than kinds 4 to 8, when not flagged (+0x154), checks the tool (`0x12F5638`) against the ride's count (+0x150 +0x148) |
| `0x587d00` | AttractionPerson::Link587D00 | In outline: links a person to an attraction (`0x4AA760`) when not flagged (+0x100) |
| `0x9dca50` | Ride::Window9DCA50 | In outline: a ride window's check: the park (`0x6C1DA0`) and the first attraction of a kind (AttractionInstance::FirstOf) |
| `0x50e210` | Car::Draw50E210 | In outline: sets up a ride car's look (not kind 6): points it (Matrix::LookAlongZSide), builds its part matrices (Model::PartMatrices869400) and its lights (Model::UpdateLights) |
| `0x5ab770` | AttractionPerson::Link5AB770 | In outline: another attraction person link like AttractionPerson::Link587D00 |
| `0x5a85b0` | TrackedRide::Station5A85B0 | In outline: for ride kind 0xF, finds a station entry (`0x4C5460`, TrackedRideInstance::Entry) |
| `0x4df1c0` | Ride::Text4DF1C0 | In outline: makes a ride's text when not flagged (+0x4C) |
| `0x5bebb0` | Ride::Frame5BEBB0 | In outline: a ride part's frame: normalises its direction and points along it (Matrix::LookAlongZ) |
| `0x5cfda0` | Ride::Frame5CFDA0 | In outline: the same part frame (another copy) |
| `0x537840` | Ride::Seats537840 | In outline: for a ride of kind 0x19, goes over its seats when not flagged (+0x21C) |
| `0x5b8c40` | Ride::Link5B8C40 | In outline: links a ride part (`0x4B52D0`) and its owner (+0x380) |
| `0x4d5d40` | Ride::Text4D5D40 | In outline: makes a ride's text by its kind (+0x114) |
| `0x535610` | Ride::Owner535610 | In outline: with entries and not flagged (+0x74), finds the ride's owner (Interface04::Get) |
| `0x515890` | Car::Update515890 | In outline: a ride car's update when shown (+0x250), with the shared ride state (`0x12F4F94`) |
| `0xa018d0` | Ride::KindA018D0 | In outline: for n above 0, acts by the selected ride's kind (`0x130B5C8`, +0x9C +0x48) |
| `0x57b560` | TrackedRide::Stations57B560 | In outline: goes over a ride's stations (TrackedRideInstance::Entry) |
| `0x568350` | Ride::Tool568350 | In outline: a ride tool's update when not flagged (+0xF0) within its count (+0xEC +0x158) |
| `0x568860` | Ride::Slot568860 | In outline: when not flagged (+0x5C) with a slot (+0x280) inside the ride's count (+0x6B0) |
| `0x5abc20` | Ride::Mode5ABC20 | In outline: acts by a ride part's mode (+0x100: 1, ...) and its kind (+0xB4 = 3) |
| `0xa87e00` | PoolComplex::PartA87E00 | In outline: a pool complex part by its kind (0, 3: PointerTable::Value44) |
| `0x51e150` | Car::Update51E150 | In outline: a ride car's timed update when shown (+0x250) |
| `0x5e7f50` | TrackPiece::Check5E7F50 | In outline: a track piece check for the selected ride (`0x12F5608`) by its options |
| `0x59c270` | Ride::Gather59C270 | In outline: when not flagged (+0x84), gathers a ride's parts by kind (LayeredObjects::Gather5E92A0) |
| `0x5a43e0` | Ride::Slots5A43E0 | In outline: a ride's slots up to its count (+0x6B0), with kind 4 (+0x38C) handled |
| `0xa753f0` | Coaster::UpdateA753F0 | In outline: when on (+0xA0), refreshes (`0xA6F3A0`) and acts in mode 2 |
| `0x52c4c0` | Car::Crash52C4C0 | In outline: resets a ride car's crash (RideCarCrash::Reset) and acts unless flagged (+0x1C1, +0xD8) |
| `0x51b7d0` | Car::Parts51B7D0 | In outline: when on (+0x924), goes over a car's parts (+0x90C, count +0x910) handling each (`0x51B590`) |
| `0x4c7b40` | AttractionPerson::Place4C7B40 | In outline: places a person on an attraction (`0x4C5E10`) when not flagged (+0x74) |
| `0x5a2880` | TrackedRide::Section5A2880 | In outline: works out a track section (`0x564550`, `0x12F5338`, `0x559BF0`) |
| `0x58ca20` | AttractionPerson::Spot58CA20 | In outline: finds a person's spot on an attraction (AttractionPersonInfo::Person) |
| `0x5e11e0` | TrackedRide::Check5E11E0 | In outline: checks a ride's track (TrackedRide::Check5D9E80) and collects its parts (SmallArray, `0x5BF3C0`) |
| `0x5359d0` | Ride::Section5359D0 | In outline: in mode 2 (+0x70), when the ride's checks fail, finds its section (`0x12F5338`) |
| `0x5ed900` | AttractionPerson::Spots5ED900 | In outline: finds a person's spots on an attraction (`0x5F2460`) for the selected one |
| `0x54e360` | Ride::Particles54E360 | In outline: makes a ride's particle manager (RideParticleManager) and its parts (+0x5C, `0x12F526C`) |
| `0x51e5d0` | Car::Parts51E5D0 | In outline: goes over a car's parts handling each (`0x51B590`) |
| `0x5f1250` | Ride::Update5F1250 | In outline: a ride's update when not flagged (+0x58) |
| `0x5adca0` | TrackedRide::Stations5ADCA0 | In outline: goes over a ride's stations (+0x6B0, TrackedRideInstance::Entry) |
| `0x5c1a40` | TrackPiece::Turn5C1A40 | In outline: a track piece's turn (Clamp0To3) when not flagged (+0xAC) |
| `0x5dfba0` | TrackPiece::Link5DFBA0 | In outline: links two different pieces when the second is not flagged (+0x170) |
| `0x51dbd0` | Car::Make51DBD0 | In outline: makes a ride car's part (`0x9D4C70`) |
| `0x4abdf0` | Ride::Tile4ABDF0 | In outline: finds a ride's tile (OffsetRef::Owner2) |
| `0x5c1070` | TrackPiece::Refresh5C1070 | In outline: refreshes a track piece (Set_+0xfc) when not flagged (+0xAC) |
| `0x5c5620` | TrackedRide::HitTest5C5620 | In outline: another ride part hit test like TrackedRide::HitTest5C4EB0, for shown parts (not flag 0x10000) |
| `0x53d8f0` | Ride::Find53D8F0 | In outline: finds an entry among a ride's parts (+0x34) |
| `0x534970` | Ride::Section534970 | In outline: a ride's section (`0x12F5338`, `0x557BD0`) |
| `0x5a8e50` | Ride::Place5A8E50 | In outline: for kinds 9 and 0xD, places at a scenery footprint's centre (SceneryItem::FootprintCentre) |
| `0x5e7520` | TrackPiece::Select5E7520 | In outline: refreshes a piece (`0x5E64F0`) for the selected ride (`0x12F5608`) |
| `0x526c40` | Car::Tilt526C40 | In outline: a car's pose (`0x50B9B0`) tilted back (Matrix::TurnAboutX by minus its angle) |
| `0x5ea560` | TrackedRide::Update5EA560 | In outline: another tracked ride update when not flagged (+0x68) |
| `0x7214a0` | Path::Flags7214A0 | In outline: path flags check (+0xD0, +0xD1) when asked |
| `0x51de90` | Car::SetMode51DE90 | In outline: sets a ride car's mode (+0x38C) when it changes, with mode 1 handled |
| `0x5db260` | Ride::CanOpen5DB260 | In outline: whether a ride can open (`0x5CFB10`) |
| `0x4c5b60` | AttractionPerson::Spot4C5B60 | In outline: a person's spot on an attraction (`0x5BEB30`) when not flagged (+0x74) |
| `0x5e6550` | TrackPiece::Selected5E6550 | In outline: acts on the selected ride (`0x12F5608`) when there is one |
| `0x592240` | Ride::Case592240 | In outline: acts by a case (1, ...) |
| `0x4ef6c0` | Ride::Hit4EF6C0 | In outline: a ride part hit test (`0x5DA220` within 0.5) when close enough |
| `0x5ebf30` | AttractionPerson::Check5EBF30 | In outline: an attraction person check when not flagged (+0x118) for the selected one |
| `0x4e84c0` | AttractionPerson::Check4E84C0 | In outline: the same attraction person check (another copy) |
| `0x552dd0` | Ride::Kind552DD0 | In outline: acts by a ride part's kind (+0xB4: 1, ...) |
| `0x4c8740` | Shop::Mode4C8740 | In outline: a shop step by its mode (+0xC4 = 2) and a value's sign |
| `0x4c5620` | AttractionPerson::Face4C5620 | In outline: points a person on an attraction (Matrix::LookAlongXUp on its +0x30) and turns it (Matrix::TurnAboutY by +0x8C) |
| `0x834de0` | Ride::Mode834DE0 | In outline: in mode 1 (+0xB0) when its check passes, acts by its second mode (+0xB4) |
| `0x4b22a0` | Shop::Refresh4B22A0 | In outline: refreshes a shop (`0x52F710`) unless told not to |
| `0x7888d0` | List::Find7888D0 | In outline: finds an entry in a list range (`0x7881B0`, `0x788030` up to +0x188) |
| `0x53de10` | Ride::Camera53DE10 | In outline: a ride step by the camera over its list |
| `0x573c00` | Ride::Hit573C00 | In outline: a ride hit test with the current tool (`0x12F5638`, `0x5DA220` within 0.5) |
| `0x500ab0` | Car::Update500AB0 | In outline: a car update (`0x59AB70`) with its part (+0xB4 +0x70) |
| `0x4c84e0` | Shop::Mode4C84E0 | In outline: a shop step by its mode (+0xC4 = 2) when not flagged (+0x74) |
| `0x5d57f0` | TrackedRide::Next5D57F0 | In outline: a step to the next station (+0xE8) when not flagged (+0x68) |
| `0x5d0a00` | TrackedRide::Check5D0A00 | In outline: a check for a ride flagged 0x400000 with more than two stations (+0xE4) |
| `0x50e730` | Car::Pose50E730 | In outline: a car's pose when not flagged (+0x250) |
| `0x4c0db0` | Shop::CountText | In outline: writes a shop's count (`0x4CDAE0` on +0xA8) as a number text ("%d") |
| `0x73bbd0` | Ride::Activate73BBD0 | In outline: activates a ride part in mode 1 (+0x5A1) |
| `0x5cdb00` | Ride::Name5CDB00 | In outline: sets a ride's name text (`0x560F00`, TextRef::Assign) |
| `0x572320` | Ride::Count572320 | In outline: acts when a ride count (`0x5636A0`) is positive |
| `0x5ba900` | Ride::Count5BA900 | In outline: acts when a ride's count (+0x20 then +0x50) is under 1 |
| `0x56b040` | Ride::Mode56B040 | In outline: a step when the current ride's mode (+0x38C) is 0xC |
| `0x54f9e0` | Ride::List54F9E0 | In outline: goes over a ride's list entries not flagged (+0x18) |
| `0x4f6cd0` | Ride::Event4F6CD0 | In outline: unless in mode 1 (+0x50), handles an event with the shared window (`0x1342708`) |
| `0x51b590` | Car::Spot51B590 | In outline: a car's spot (`0x518390`) when not flagged (+0xC) |
| `0x5c9370` | TrackedRide::Check5C9370 | In outline: a check for a ride flagged 0x400000 with more than two stations, when not flagged (+0x130) |
| `0x5a0110` | Ride::Time5A0110 | In outline: a step (`0x59FFA0`) once a time passes its value (+0x50) |
| `0x5a0380` | Ride::Time5A0380 | In outline: the same timed step (another copy) |
| `0x4940c0` | Ride::Parts4940C0 | In outline: goes over a ride's parts (+0x234) |
| `0x5c07f0` | TrackPiece::Pose5C07F0 | In outline: a track piece's pose (`0x5C0720`, Matrix::LookAlongZSide) |
| `0x54f1a0` | Ride::Particles54F1A0 | In outline: makes a ride particle manager (RideParticleManager) with a name (`0x12F52C4`) |
| `0x54eb40` | Ride::Particles54EB40 | In outline: the same with another name (`0x12F5250`) |
| `0x52d390` | Ride::Check52D390 | In outline: a ride check (`0x12F537C`) when not flagged (+0xD8) |
| `0x5ee9a0` | AttractionPerson::Select5EE9A0 | In outline: when not flagged (+0x118), acts on the selected ride (`0x12F5608`) |
| `0x5da510` | TrackedRide::Find5DA510 | In outline: a step when a piece is its own (`0x5D6E80`) |
| `0x4f3f40` | Ride::Kind4F3F40 | In outline: a step over parts of kinds 1, 3 or 4 |
| `0x4f1740` | Ride::Station4F1740 | In outline: a station check (`0x5D1D80` with +0x5DC, +0x5E0, `0x5D0630`) |
| `0x5ada60` | TrackedRide::Flags5ADA60 | In outline: a step when either shared flag (`0x12F5528`, `0x12F5529`) is set |
| `0x58b7a0` | Ride::Hit58B7A0 | In outline: a hit test for the selected ride (`0x12F5608`) |
| `0x5e0090` | TrackPiece::Find5E0090 | In outline: finds a piece among a ride's pieces (+0x158) |
| `0x4f5060` | Ride::Refresh4F5060 | In outline: refreshes a ride (`0x4F3B90` with 1) |
| `0x4c0450` | Shop::Time4C0450 | In outline: a shop step with a positive time and an entry (+0xCC) |
| `0x4b4480` | Ride::Owner4B4480 | In outline: a ride step with its owner (OffsetRef::Owner4) |
| `0x5e8260` | TrackedRide::Update5E8260 | In outline: a tracked ride update when not flagged (+0x68) |
| `0x531240` | Ride::Slot531240 | In outline: a ride step with an attraction slot (AttractionInstance::Slot12) |
| `0x58c800` | AttractionPerson::Find58C800 | In outline: a step for an attraction's person (AttractionPersonInfo::Person) |
| `0x58b580` | Ride::Hit58B580 | In outline: another hit test for the selected ride (`0x12F5608`) |
| `0x5591e0` | Ride::Check5591E0 | In outline: a ride check when not flagged (+0x18) |
| `0x5ae990` | TrackedRide::Make5AE990 | In outline: makes a 0x3F0-byte ride part |
| `0x50dff0` | Car::Flat50DFF0 | In outline: a flat (x, z) length and direction check |
| `0x4ce9b0` | Shop::Text4CE9B0 | In outline: a shop text when not flagged (+0x4C) |
| `0x4d0cb0` | Shop::Grow4D0CB0 | In outline: grows a shop list (+0x9C) |
| `0x54f670` | Ride::List54F670 | In outline: goes over a ride's list |
| `0x4914f0` | Ride::Update4914F0 | In outline: a ride update when not flagged (+0x428) |
| `0x59dc60` | TrackSegment::Make | In outline: makes a track segment (TrackSegment::TrackSegment) |
| `0x574740` | Ride::Slot574740 | In outline: a ride step when its slot (+0xBC) is in range of the current tool's count (`0x12F5638` +0x114) |
| `0x5e1f20` | TrackPiece::Count5E1F20 | In outline: a track step for a positive count |
| `0x4eb3d0` | AttractionPerson::Update4EB3D0 | In outline: an attraction person update when not flagged (+0x118) |
| `0x5d00f0` | TrackPiece::Pose5D00F0 | In outline: a track piece pose with its turn (Clamp0To3) |
| `0x4f0c70` | Ride::Step4F0C70 | In outline: a ride step (`0x4F06B0`) |
| `0x4e4b20` | Window::Build4E4B20 | In outline: builds a window from the shared title `0x12F4FA4` |
| `0x490a60` | Ride::Pass490A60 | In outline: goes over a ride's parts (+0x15C) |
| `0x4af1f0` | Ride::Link4AF1F0 | In outline: links a ride's part (`0x4AA760`) |
| `0x5a9cc0` | Ride::Update5A9CC0 | In outline: a ride update when not flagged (+0x324) |
| `0x5864e0` | Ride::Check5864E0 | In outline: a ride check when not flagged (+0x1EC) |
| `0x4698e0` | Ride::Slot4698E0 | In outline: a step when it has a slot |
| `0x5e4760` | TrackPiece::Make5E4760 | In outline: makes a 0x14-byte record |
| `0x4e7380` | Ride::Check4E7380 | In outline: a ride check (`0x531730`) |
| `0x4bbf30` | Ride::Pick4BBF30 | In outline: a pick under the mouse (MainCamera::Get) |
| `0x5eade0` | TrackedRide::Check5EADE0 | In outline: a check on a ride's part |
| `0x59c580` | Ride::Update59C580 | In outline: a ride update when not flagged (+0x84) |
| `0x531c30` | Ride::Count531C30 | In outline: a ride step for a positive count |
| `0x568150` | Ride::Update568150 | In outline: a ride update when not flagged (+0xF0) |
| `0x70f680` | Ride::Step70F680 | In outline: a ride step (`0x4F06B0`) |
| `0x5516d0` | Ride::List5516D0 | In outline: goes over a ride's list |
| `0x416650` | Ride::Mode416650 | In outline: a step in modes other than 0 and 1 (+0x174) |
| `0x5e6a40` | TrackPiece::Link5E6A40 | In outline: links a piece when given one |
| `0x4ee130` | Ride::Part4EE130 | In outline: a ride step with its part (+0x5CC) |
| `0x511e90` | Car::Pass511E90 | In outline: goes over a car's parts (+0x314) |
| `0x5bab50` | TrackPiece::Table5BAB50 | In outline: sets up a two-entry shared table (`0x12F5580`, 0x10 bytes each) |
| `0x7bf360` | Ride::Check7BF360 | In outline: a check when not flagged (+0x77) |
| `0x512550` | Car::Pass512550 | In outline: goes over a car's parts (+8) |
| `0x5da330` | TrackedRide::Selected5DA330 | In outline: acts on the selected ride (`0x12F5608`) |
| `0x58c320` | Ride::All58C320 | In outline: goes over the rides from the selected one (`0x12F5608`, next at +0x188) |
| `0x555520` | Ride::Step555520 | In outline: a ride step (`0x4A1800`) |
| `0x5a3b80` | TrackedRide::Refresh5A3B80 | In outline: a refresh (`0x5D06C0` with 1) |
| `0x58d200` | Ride::Case58D200 | In outline: acts by a value (+0xB4) |
| `0x5c5f80` | TrackPiece::Refresh5C5F80 | In outline: a piece refresh (`0x5C5EE0`) |
| `0x5c1590` | TrackPiece::Has5C1590 | In outline: whether a piece's ride has both parts (+0x30, +0x34) |
| `0x5a6bd0` | Ride::Text5A6BD0 | In outline: makes a text from a shared one (`0xF265E4`) |
| `0x50eb50` | Car::Update50EB50 | In outline: a car update when not flagged (+0x250) |
| `0x58c140` | Ride::All58C140 | In outline: goes over the rides from the selected one (`0x12F5608`, next at +0x16C) |
| `0x5edbe0` | AttractionPerson::Select5EDBE0 | In outline: a step with the selected ride (`0x12F5608`) |
| `0x7ac530` | Attraction::First7AC530 | In outline: a step with the first attraction (AttractionInstance::FirstOf(0)) |
| `0x7ac350` | Attraction::First7AC350 | In outline: the same (another copy) |
| `0x5db080` | TrackedRide::Extent5DB080 | In outline: works out an extent, clearing it first (Extent::Clear) |
| `0x58f560` | Ride::Check58F560 | In outline: a ride check (`0x58EA70`) |
| `0x5bae10` | TrackPiece::Time5BAE10 | In outline: a step with a non-negative time when on (+0x98) |
| `0x567c60` | Ride::Update567C60 | In outline: a ride update when not flagged (+0xF0) |
| `0x4eb1f0` | AttractionPerson::Update4EB1F0 | In outline: an attraction person update when not flagged (+0x118) |
| `0x5d6220` | TrackedRide::Slot5D6220 | In outline: a step with a parted object's slot (PartedObject::Slot4) |
| `0x5d6050` | TrackedRide::Slot5D6050 | In outline: a step with a parted object's slot (PartedObject::Slot2) |
| `0x5d5e80` | TrackedRide::Slot5D5E80 | In outline: the same (another copy) |
| `0x5d5cb0` | TrackedRide::Slot5D5CB0 | In outline: a step with a parted object's slot (PartedObject::Slot1) |
| `0x4f3cf0` | Ride::SetMode4F3CF0 | In outline: changes a ride's mode, with modes 6 and 3 handled unless the new one is 5 |
| `0x4c3eb0` | Shop::Case4C3EB0 | In outline: a value by case |
| `0x5e3f00` | TrackPiece::Link5E3F00 | In outline: a step when given a piece |
| `0x5f1fd0` | TrackPiece::Check5F1FD0 | In outline: a check with the current tool (`0x12F565C`) |
| `0x59d540` | Ride::Refresh59D540 | In outline: gathers a ride's parts (Ride::Gather59C270(0, 1)) |
| `0x582af0` | Ride::Terrain582AF0 | In outline: a ride step over the terrain |
| `0x526a70` | Car::Mode526A70 | In outline: a car step in mode 0x10 (+0x384) |
| `0xa3d7a0` | Ride::CanOpenA3D7A0 | In outline: a step when a ride can open (`0x5CFB10`) |
| `0x5c7000` | TrackedRide::Check5C7000 | In outline: a check when not flagged (+0x2F0) |
| `0x5c6e30` | TrackedRide::Check5C6E30 | In outline: a check when not flagged (+0x2D4) |
| `0x4d2540` | Shop::Link4D2540 | In outline: links a shop's part |
| `0x4845c0` | Ride::Case4845C0 | In outline: a step for cases 1 or 4 |
| `0x511130` | Car::Update511130 | In outline: a car update when not flagged (+0x180) |
| `0x579d40` | Ride::Time579D40 | In outline: a step with a positive time and no part (+4) |
| `0x5a9ec0` | Ride::Update5A9EC0 | In outline: a ride update when not flagged (+0x324) |
| `0x4bf1f0` | Shop::Count4BF1F0 | In outline: a step with no entries handled |
| `0x4dfb10` | Ride::Empty4DFB10 | In outline: a step when two counts (+0x380, +0x388) add to under 1 |
| `0x5fb250` | Ride::Update5FB250 | In outline: an update when not flagged (+8) |
| `0x5a1e10` | Ride::Section5A1E10 | In outline: a step with a ride's section (`0x12F5338`) |
| `0x4ac090` | Ride::Count4AC090 | In outline: a step past a count (+0x31C) |
| `0xa7f3b0` | AttractionPerson::StepA7F3B0 | In outline: a step with an attraction's person (AttractionPersonInfo::Person) |
| `0x559e00` | Ride::Get559E00 | In outline: a step with an object (Interface04b::Get) |
| `0x533e20` | Shop::Step533E20 | In outline: a shop step (`0x4C1CD0`) |
| `0x4edf70` | Ride::Step4EDF70 | In outline: a step unless told not to |
| `0x5c6c80` | TrackedRide::Check5C6C80 | In outline: a check when not flagged (+0x1E0) |
| `0x50e980` | Car::Shared50E980 | In outline: a step when the shared value (`0x10EE438`) is set |
| `0x4d02e0` | Shop::Slot4D02E0 | In outline: a step with no slot (+0x88 negative) |
| `0x548cf0` | Ride::Slot548CF0 | In outline: a step with no slot (+0x74 negative) |
| `0x5bc3b0` | TrackPiece::Blend5BC3B0 | In outline: a blend of two values by a fraction, rounded |
| `0x59de70` | Ride::Loop59DE70 | In outline: goes over a linked list |
| `0x57a320` | Ride::Time57A320 | In outline: a step when a value is below 0 |
| `0x4b0a20` | Ride::List4B0A20 | In outline: goes over a list |
| `0x567fa0` | Ride::Update567FA0 | In outline: an update when not flagged (+0xF0) |
| `0x5a90e0` | Ride::Slot5A90E0 | In outline: a step with an attraction slot (AttractionInstance::Slot51) |
| `0x4d1900` | Shop::Link4D1900 | In outline: links a shop's part |
| `0x56f410` | Path::Queue56F410 | In outline: a queue (kind 0xD) step |
| `0x5cbe70` | TrackedRide::Check5CBE70 | In outline: a check when given nothing |
| `0x55c450` | Ride::Make55C450 | In outline: makes a part |
| `0x543be0` | Ride::Check543BE0 | In outline: a step when there is no part or its check fails (`0xBB41F0`) |
| `0x58bab0` | Ride::Selected58BAB0 | In outline: a step with the selected ride (`0x12F5608`) |
| `0x53a490` | Ride::Round53A490 | In outline: a value rounded |
| `0x4d09c0` | Shop::Make4D09C0 | In outline: makes a part (`0x4B4740`) |
| `0x4f1c50` | Ride::Station4F1C50 | In outline: a station step (`0x5D1FD0`) |
| `0x595dd0` | ShopItem::Make | In outline: makes a shop item (ShopItemInstance) |
| `0x5c1ee0` | TrackPiece::Check5C1EE0 | In outline: a check (`0xBC9280`) |
| `0x5f79d0` | Ride::On5F79D0 | In outline: a step when on (+0x114) |
| `0x4de280` | Ride::Check4DE280 | In outline: a step when a check passes |
| `0x4b6890` | Ride::Make4B6890 | In outline: makes a list member part (ListMember with `0x10EC9F8`) |
| `0x4ca6e0` | Shop::Find4CA6E0 | In outline: finds an entry in a list range (`0x4C9F20` up to +0x17C) |
| `0x5eed40` | AttractionPerson::Update5EED40 | In outline: an update when not flagged (+0x118) |
| `0x493ee0` | Ride::Count493EE0 | In outline: a step for a positive count |
| `0x479fb0` | Shop::Item479FB0 | In outline: a shop item for slot 0 or 1 |
| `0x4c1f70` | Info::Text4C1F70 | In outline: a step with an info text (Info::Text4C0850) |
| `0x5268e0` | Car::Check5268E0 | In outline: a step for a ride flagged 0x400000 with more than one station |
| `0x5dfef0` | TrackPiece::Find5DFEF0 | In outline: finds a piece among a ride's pieces (+0x158) |
| `0x56f710` | Ride::Time56F710 | In outline: a step when a value (+0x70) is positive |
| `0x593370` | Ride::Part593370 | In outline: a step with a part |
| `0x56bd20` | Ride::Update56BD20 | In outline: an update when not flagged (+0xF0) |
| `0x529990` | Car::Pivot529990 | In outline: checks a car's pivot (RideCar::CheckPivot) |
| `0x4c06c0` | Ride::NameInText | In outline: fills a text's "%RIDENAME%" marker with a ride's name |
| `0x5a7740` | Car::Description | In outline: a car's description text, "<car description undefined>" when it has none |
| `0x4df960` | Ride::Make4DF960 | In outline: makes a list member part (ListMember with `0x10EC9F8`) |
| `0x5d6650` | TrackedRide::List5D6650 | In outline: makes a small list (SmallArray) |
| `0x5ce040` | TrackedRide::Step5CE040 | In outline: a step (`0x5A4A90`) |
| `0x5036d0` | Ride::Window5036D0 | In outline: a step with a window (`0x67B3C0`) |
| `0x5d46e0` | TrackedRide::Pass5D46E0 | In outline: goes over entries (+0xF0) |
| `0x569f10` | Ride::Step569F10 | In outline: a step (`0x4E6410`) |
| `0x566b50` | Ride::Time566B50 | In outline: a step when a value (+0x70) is positive |
| `0x536400` | Ride::Find536400 | In outline: finds an entry up to n |
| `0x500d10` | Car::Part500D10 | In outline: a step with no part (+0xB4 +0x70) |
| `0x4f8830` | Ride::Find4F8830 | In outline: finds an entry matching a value |
| `0x4b6d10` | Ride::Step4B6D10 | In outline: a step (`0x4B52D0`) |
| `0x4b5060` | Ride::Link4B5060 | In outline: a check when given a part |
| `0x510e00` | Car::Tool510E00 | In outline: a step with the current tool (`0x12F5638`) |
| `0x558da0` | Ride::Update558DA0 | In outline: an update when not flagged (+0x18) |
| `0x5501e0` | Array::Resize5501E0 | In outline: resizes a list to n |
| `0x50b7e0` | Car::Case50B7E0 | In outline: a pose step by case |
| `0x5c1890` | TrackPiece::Value5C1890 | In outline: a value when not flagged (+0x170) |
| `0x5bdc00` | TrackPiece::Check5BDC00 | In outline: a check with a part |
| `0x59ade0` | Ride::Update59ADE0 | In outline: an update when not flagged (+0xD8) |
| `0x523790` | Car::Update523790 | In outline: an update when not flagged (+0x180) |
| `0x50b9b0` | Car::Pose50B9B0 | In outline: a car pose step when a value is positive |
| `0x5a4a90` | TrackedRide::Platforms | In outline: loads from the "Tracks\Platforms" folder |
| `0x58aa30` | Ride::Time58AA30 | In outline: a timed step (`0x58A7A0`, Device::TimeBCC4C0) |
| `0x58a8b0` | Ride::Time58A8B0 | In outline: a timed step (`0x58A7A0`, `0xBCC740`) |
| `0x4b6b90` | Ride::Check4B6B90 | In outline: a step when a check fails (`0x4B51E0`) |
| `0xaa9ab0` | PoolComplex::RideAA9AB0 | In outline: a step with a ride's section (`0x12F5338`, Ride::Get559E00) |
| `0x4c8360` | Shop::Mode4C8360 | In outline: a step unless in mode 1 (+0xC4), with a positive value (+0x90) |
| `0x55a530` | Ride::Link55A530 | In outline: links parts (`0x4AA7E0`) |
| `0x5240b0` | Car::Check5240B0 | In outline: a check by a value against a limit, or mode 2 (+0x384) |
| `0x58bfd0` | Ride::All58BFD0 | In outline: goes over the rides from the selected one (`0x12F5608`, next at +0x188), for those flagged 0x400000 with more than one station |
| `0x58be60` | Ride::All58BE60 | In outline: the same with next at +0x16C |
| `0x561c00` | Ride::Step561C00 | In outline: a step unless told not to, when not flagged (+0x5C) |
| `0x7ad8a0` | Ride::Case7AD8A0 | In outline: a value by case (0x84, ...) |
| `0x59e490` | Ride::Selected59E490 | In outline: a step with the selected ride (`0x12F5608`) when a check fails |
| `0x53eb40` | Ride::Pass53EB40 | In outline: goes over a ride's parts (+0x34) |
| `0x5f22f0` | TrackPiece::Ref5F22F0 | In outline: a step with the current tool (`0x12F565C`, WithReference5EB9B0) |
| `0x5738d0` | Ride::Check5738D0 | In outline: a check when not flagged (+0x254) |
| `0x54ef30` | Ride::Clear54EF30 | In outline: clears a part's value (Set_+0xc0(0)) when it has one |
| `0x7ad1f0` | Attraction::First7AD1F0 | In outline: a step with the first attraction (AttractionInstance::FirstOf(0)) |
| `0x7ad080` | Attraction::First7AD080 | In outline: the same (another copy) |
| `0x57a4d0` | Ride::Tool57A4D0 | In outline: a step with the current tool (`0x12F5638`) and selected ride (`0x12F5608`) |
| `0x534800` | Ride::Count534800 | In outline: a step (`0x532930`) for a positive count |
| `0x4032d0` | Ride::Kind4032D0 | In outline: a step when flagged (+0x58) or a part's kind is 0xE |
| `0x9d4800` | Car::LoadParts | In outline: loads ".\Cars\CarParts.pc" |
| `0x53be10` | Ride::Event53BE10 | In outline: unless event 0x13 from itself, a step under a count (+0x274, +0x278) |
| `0x51e460` | Car::Time51E460 | In outline: a step with a non-negative time over entries (+4) |
| `0x5c45e0` | TrackPiece::Name5C45E0 | In outline: a step with a name (`0x560F00`, Scenery::Step84B150) |
| `0x4cf580` | Shop::Count4CF580 | In outline: a step when a count (`0x4CE0E0`) is above 2 |
| `0x54c550` | Ride::Part54C550 | In outline: a step with a part not flagged (+0xC8) |
| `0x4b1d60` | Ride::Entry4B1D60 | In outline: a step when an entry is not set (Entry::IsSet) |
| `0x5bf000` | TrackPiece::Kind5BF000 | In outline: a step for flagged pieces (bit 2) of kinds 0x5B to 0x5D and others |
| `0x59e330` | Ride::Tool59E330 | In outline: a step (`0x4B52D0`) with the current tool (`0x12F5638`) |
| `0x57a180` | Ride::Part57A180 | In outline: a step with a part (`0x569DC0`) that is on (+0x130) |
| `0x4c1d20` | Shop::Time4C1D20 | In outline: when a value (+0x7C) is below 0, a step with the date (GameTime::Get) |
| `0x500250` | Car::Range500250 | In outline: a step for a positive count with a grid range (Grid::Range) |
| `0x5bd9d0` | TrackPiece::Kind5BD9D0 | In outline: for kinds 0x29, 0x4D, 0x4E and others, makes a 0xC0-byte part |
| `0x518790` | Array::Resize518790 | In outline: resizes a list of 16-byte entries to n |
| `0x4cde30` | Shop::Path4CDE30 | In outline: a step with the path manager for a set entry |
| `0x4a3ed0` | Ride::Text4A3ED0 | In outline: makes a part and a text (Text::MakeBDAC20) |
| `0x567e40` | Ride::Tool567E40 | In outline: a step with the current tool (`0x12F5638`, `0x5D6860`) |
| `0x4cffe0` | Shop::Check4CFFE0 | In outline: a check when not flagged (+0x4C) |
| `0x5a38d0` | Ride::Tool5A38D0 | In outline: a step with the current tool (`0x12F5638`) when not flagged (+0x378) |
| `0x51b430` | Car::Selected51B430 | In outline: a step for the selected ride unless its kind is 3 or it is flagged 0x40 |
| `0x7cfcc0` | Person::Ride7CFCC0 | In outline: sets a person's state to 0x102 and acts by a value (+0xB8) |
| `0x5cf820` | TrackedRide::Count5CF820 | In outline: a step (`0x5BD740`) for a positive count |
| `0x531e30` | Ride::Random531E30 | In outline: when asked, a random step (Random::Ran0) |
| `0x5f21a0` | TrackPiece::Step5F21A0 | In outline: a step for -2 when not flagged (+0x18) |
| `0x5aa650` | Ride::Count5AA650 | In outline: a step when a count (+0x7B4) is under 1 |
| `0x4cdf90` | Shop::Slot4CDF90 | In outline: a step with a slot (+0x54) when not flagged (+0x50) |
| `0x4bb990` | Shop::Step4BB990 | In outline: a step (`0x4BB920`) |
| `0x5e29d0` | TrackPiece::Extent5E29D0 | In outline: clears an extent (Extent::Clear) and acts when not flagged (+0x18C) |
| `0x5ae050` | TrackedRide::Entry5AE050 | In outline: a step with a ride's first entry (TrackedRideInstance::Entry(0)) |
| `0x5d04a0` | TrackedRide::Step5D04A0 | In outline: a step (`0x5D1590`, `0x5D15A0`) |
| `0x798210` | Guest::Mode798210 | In outline: a value by a mode (+0x190: 0xD, ...) |
| `0x5be6a0` | TrackPiece::Face5BE6A0 | In outline: points (Matrix::LookAlongZSide) when a count is over 100 |
| `0x57a7e0` | Ride::Case57A7E0 | In outline: a step by case |
| `0x4f3980` | Ride::Mode4F3980 | In outline: in mode 4 (+0x144), a step when not flagged (+0x668) |
| `0x4c7600` | Shop::Hit4C7600 | In outline: a hit test (`0x4C7420`, `0x5DA220`) |
| `0x56f5c0` | Ride::Loop56F5C0 | In outline: a loop when not flagged (+0xF0) |
| `0x4eb0a0` | AttractionPerson::Check4EB0A0 | In outline: a check (`0xBCC740`) when not flagged (+0x13C) |
| `0xa7f6b0` | AttractionPerson::StepA7F6B0 | In outline: a step with an attraction's person (AttractionPersonInfo::Person) |
| `0x592980` | Ride::Slot592980 | In outline: a step for n under a count (+0x1BC) when not flagged (+0x204) |
| `0x4df520` | Ride::Text4DF520 | In outline: a text when not flagged (+0x4C) |
| `0x4b63c0` | Shop::Text4B63C0 | In outline: a text when not flagged (+0x50) |
| `0x5bdec0` | TrackPiece::Check5BDEC0 | In outline: a check for rides not flagged 0x400000 or with fewer than three stations |
| `0x92e910` | Ride::Hit92E910 | In outline: a ride part hit test (TrackedRide::HitTest5C4EB0 within 0.5) |
| `0x5aa220` | Ride::Ids5AA220 | In outline: makes the shared id set (`0x12F4634`, IdSet) in a loop |
| `0x50b4b0` | Car::Count50B4B0 | In outline: a step for counts above 3 when not flagged (+0x108) |
| `0x4d29d0` | Shop::Step4D29D0 | In outline: when a check passes, a step (`0x4D2800` with 3) |
| `0x5d6500` | TrackedRide::List5D6500 | In outline: makes a small list (SmallArray) and fills it (`0x5BF340`) |
| `0x4b2510` | Shop::Items4B2510 | In outline: goes over a shop's items (+0x3EC) making their texts (Shop::ItemText4B0190) |
| `0x576bf0` | Ride::Random576BF0 | In outline: when asked, a random value in a range (Random::Ran0Range with +0x90, +0x94) |
| `0x536160` | Ride::Get536160 | In outline: a step with an object (Interface50::Get) |
| `0x52f5c0` | Ride::Mark52F5C0 | In outline: a green (0xFF00FF00) mark at a scenery footprint's centre |
| `0x511d40` | Car::Step511D40 | In outline: a step (`0x86D0A0`) and more for 0 with a part (+0x404) |
| `0x511bf0` | Car::Step511BF0 | In outline: the same with `0x86D050` |
| `0x511aa0` | Car::Step511AA0 | In outline: the same with `0x86D000` |
| `0x511950` | Car::Step511950 | In outline: the same with `0x86CFB0` |
| `0x4aca40` | Ride::Update4ACA40 | In outline: an update when not flagged (+0x3C4) |
| `0x581300` | Ride::Case581300 | In outline: a step by case |
| `0x7afc90` | Ride::Step7AFC90 | In outline: a step (`0x410100`) unless told not to |
| `0xa7f570` | AttractionPerson::StepA7F570 | In outline: a step with an attraction's person (AttractionPersonInfo::Person) |
| `0x5da7d0` | TrackPiece::Pass5DA7D0 | In outline: goes over a ride's pieces (+0x158) (`0x510800`) |
| `0x594a90` | Ride::Count594A90 | In outline: a step with a part's count (+0x1CC +4) |
| `0x5df360` | TrackedRide::Pass5DF360 | In outline: a step with positive counts (+0xF8, +0xE8) |
| `0x5dee90` | TrackedRide::Pass5DEE90 | In outline: a step with positive counts (+0xF0, +0xE8) |
| `0x4f52e0` | Ride::Round4F52E0 | In outline: a step with a value rounded from a scale when not flagged (+0x54 +0x1BC) |
| `0x5e67d0` | TrackPiece::Tool5E67D0 | In outline: a step with the current tool (`0x12F5638`) |
| `0x54fc50` | Ride::Activate54FC50 | In outline: activates a part |
| `0x569c60` | Ride::Check569C60 | In outline: a check when not flagged (+0x5C) and a part is not in mode 1 |
| `0x50c0d0` | Car::Slot50C0D0 | In outline: a step (`0x94EF60`) when it has a slot |
| `0x50c290` | Car::Step50C290 | In outline: a step (`0xB82870`) with a slot (+4) |
| `0x4f0930` | Ride::Text4F0930 | In outline: makes a text from a shared one (`0xF35D48`) |
| `0x4f07f0` | Ride::Text4F07F0 | In outline: makes a text from a shared one (`0xF28914`) |
| `0x4f06b0` | Ride::Text4F06B0 | In outline: makes a text from a shared one (`0xF288E8`) |
| `0x4e11c0` | Ride::Part4E11C0 | In outline: a step (`0xA06E30`) with a part (+0x30) |
| `0xbc5c10` | Track::StepBC5C10 | In outline: a step (`0xBC5AB0`, `0xBC5B40`) |
| `0x5267a0` | Car::Terrain5267A0 | In outline: a step over the terrain |
| `0x5079b0` | Car::Step5079B0 | In outline: a step when not flagged (+0xD8) for an entry above 1 |
| `0x642910` | Window::Round642910 | In outline: for 2, a value rounded |
| `0x4c1160` | Shop::Text4C1160 | In outline: a text when the shared value (`0x1303738`) is set |
| `0x5f0670` | TrackPiece::Step5F0670 | In outline: a step (`0x5EFD60`, `0x5F0600`) |
| `0x4ce5d0` | Shop::Text4CE5D0 | In outline: a text when a slot check fails (Shop::Slot4CDF90) |
| `0x536da0` | Ride::Shared536DA0 | In outline: a step unless it is the shared object's part (`0x12F4F18` +0x278) |
| `0x5362b0` | Ride::Section5362B0 | In outline: a step with a ride's section (`0x12F5338`) under a count |
| `0x4cd9a0` | Shop::Part4CD9A0 | In outline: a step with a part |
| `0x56a1b0` | Ride::Check56A1B0 | In outline: a check when not flagged (+0x254) |
| `0x566cd0` | Ride::Tool566CD0 | In outline: unless a kind is 10, a step with the current tool's count (`0x12F5638` +0x150) |
| `0x5173d0` | Car::Pass5173D0 | In outline: a step (`0x513740`) over a car's parts (+0x314) |
| `0x50b690` | Car::Check50B690 | In outline: a step for rides not flagged 0x400000 or with fewer than two stations |
| `0x5cf540` | TrackedRide::Platforms5CF540 | In outline: finds a platform's pieces (Platform::FindPieces) |
| `0x4eef50` | Ride::Clear4EEF50 | In outline: a step (`0x4BCF30`) and empties a list |
| `0x56a530` | Ride::Check56A530 | In outline: a check when not flagged (+0x5C) |
| `0x5b5c50` | TrackedRide::Shared5B5C50 | In outline: a step when it is the shared object's part (`0x12F4FE4` +0x5C4) |
| `0x416380` | Ride::Case416380 | In outline: a step by a value (+0x174) |
| `0x4bb620` | Shop::Round4BB620 | In outline: a value rounded |
| `0x577bd0` | Ride::Tool577BD0 | In outline: a step with the current tool (`0x12F5638`, `0x5D4940`) |
| `0x5bf510` | TrackPiece::Value5BF510 | In outline: a value (`0x5BEB30`) above 1 |
| `0x586880` | Ride::Owner586880 | In outline: a step unless its owner's kind is 0xB (OffsetRef::Owner5) |
| `0x586750` | Ride::Owner586750 | In outline: the same (another copy) |
| `0x53db90` | Ride::Text53DB90 | In outline: a shared text (ShareText34) for each of a ride's parts (+0x34) |
| `0x5a7fa0` | Ride::Update5A7FA0 | In outline: a step and an update when not flagged (+0x378) |
| `0x564550` | Ride::Check564550 | In outline: a check when not flagged (+0xF0) |
| `0x4d4f70` | Shop::Step4D4F70 | In outline: a step (`0x4B52D0`, `0x4C5240`) |
| `0xbc5090` | Track::StepBC5090 | In outline: for rides not flagged 0x400000 or with fewer than two stations, goes over entries (+0x70) |
| `0x56a400` | Ride::Check56A400 | In outline: a check when not flagged (+0x254) |
| `0x5604c0` | Ride::Clear5604C0 | In outline: tears down a part and more (+0x2DC) |
| `0x491260` | Ride::Count491260 | In outline: a loop up to n when not flagged (+0x428) |
| `0x5c7280` | TrackedRide::Extent5C7280 | In outline: a loop clearing an extent (Extent::Clear) |
| `0x4f0f30` | Ride::Mode4F0F30 | In outline: with no slot (+0x7F8), a step unless in mode 3 or 1 (+0x144) |
| `0x59be00` | Ride::Activate59BE00 | In outline: activates a part with the current tool (`0x12F5638`) |
| `0x56a2e0` | Ride::Check56A2E0 | In outline: a check when not flagged (+0x254) |
| `0x511010` | Car::Check511010 | In outline: a step for values above 2 when not flagged (+0x88C) |
| `0x4ee570` | Ride::Round4EE570 | In outline: a value rounded from a scale, negated when asked |
| `0x4ce890` | Shop::Entry4CE890 | In outline: when not flagged (+0x4C), a step with a ride's entry (TrackedRideInstance::Entry) |
| `0x5abae0` | Ride::Refresh5ABAE0 | In outline: a refresh (`0x52F710`) and a step when not flagged (+0x338) |
| `0x5c3ca0` | TrackPiece::Time5C3CA0 | In outline: a step (`0x5C17F0`) with a non-negative time |
| `0x5ae460` | Ride::Value5AE460 | In outline: a value with a text when a value (+0x868) is below 0 |
| `0x4c8110` | Shop::Mode4C8110 | In outline: in mode 2 (+0xC4), a step with an object (Interface60::Get) |
| `0x5ea970` | AttractionPerson::Compare5EA970 | Compares two attraction people: first by a flag (bit 22 of +0x108 +0x94), then by a value (`0x5EA940`) |
| `0x534e60` | Ride::Text534E60 | In outline: makes an empty text and, with entries and not flagged (+0x1DC), fills it from its part |
| `0x5d9bd0` | TrackedRide::Selected5D9BD0 | In outline: unless told not to, a step with the selected ride (`0x12F5608`) when not flagged (+0x170) |
| `0x4c0c90` | Shop::Text4C0C90 | In outline: with a part (+0xA8), makes an info text (`0xF31F48`) |
| `0x4cdd10` | Shop::Nearest4CDD10 | In outline: finds the nearest entry in a list (+0x64) |
| `0x4af910` | Ride::Age4AF910 | In outline: checks reliability (Ride::CheckReliability), adds the time to a value (+0x360) unless flagged (bit 14 of +0xE4), and goes over entries (+0x368) |
| `0x563300` | Ride::Find563300 | In outline: finds an entry among entries (+0xC4) not flagged |
| `0x4de590` | Ride::Step4DE590 | In outline: a step with its part (virtual slot 0x54) |
| `0x4cf190` | Shop::Make4CF190 | In outline: when given nothing, makes one (`0x4CCFB0`) |
| `0x59d7b0` | Ride::Toggle59D7B0 | In outline: when its owner is flagged (bit 8 of +0x30), turns a flag over (+0xB4) |
| `0x54c7a0` | Ride::Loop54C7A0 | In outline: a loop over a list |
| `0x5c1dd0` | TrackPiece::Check5C1DD0 | In outline: when a check passes (`0x5BF180`), a step for pieces flagged (+1 bit 8) |
| `0x5f1140` | TrackPiece::Check5F1140 | In outline: a check when not flagged (+0x58) |
| `0x58af90` | Ride::Time58AF90 | In outline: a timed step (`0x58A7A0`) |
| `0x4cf470` | Shop::Reset4CF470 | In outline: a reset (`0x501FB0`), a camera step and clears a slot (+0x84 = -1) |
| `0x463530` | Scenery::Check463530 | In outline: a check when not flagged (+0x3C) |
| `0x584bb0` | Ride::Slot584BB0 | In outline: a step (Offset138 or `0x4A18B0`) |
| `0x4aa3e0` | Ride::Check4AA3E0 | In outline: a check when not flagged (+0x200) |
| `0x555740` | Ride::Check555740 | In outline: a check (`0x4A1800`, Offset138) |
| `0x50c3d0` | Car::Cross50C3D0 | In outline: a cross product step when a length is above 0.5 |
| `0x4f7030` | TrackEditor::AutoComplete4F7030 | In outline: with a part (+0x4EC), shows the auto-complete cost (TrackEditor::AutoCompleteCost(1)) and gives 0 |
| `0x4cc710` | Shop::Terrain4CC710 | In outline: clears a flag (+0xA4) and a step when the terrain is not flagged |
| `0x4c7e40` | Shop::Check4C7E40 | In outline: a check when not flagged (+0x74) |
| `0x5aa810` | Ride::Check5AA810 | In outline: a check when not flagged (+0x378) |
| `0x5c0e30` | TrackPiece::Make5C0E30 | In outline: makes a 0xC-byte record |
| `0x83a7b0` | Scenery::Ride83A7B0 | In outline: a step with its part's selected ride (`0x12F5608`) |
| `0xbc65d0` | Track::LastBC65D0 | In outline: for rides not flagged 0x400000 or with fewer than two stations, the last 0x24-byte entry |
| `0x472b40` | Ride::Mode472B40 | In outline: a value by a mode (+0xE0) |
| `0x71c730` | Path::Case71C730 | In outline: a path piece by case |
| `0x5a79e0` | TrackedRide::Text5A79E0 | In outline: a text when it has none (+0x700) |
| `0x5a78d0` | TrackedRide::Text5A78D0 | In outline: the same (another copy) |
| `0x573af0` | Ride::Reset573AF0 | In outline: clears a slot (+0x228 = -1) and, when a check passes, sets a time (+0x68) |
| `0x536c90` | Ride::Terrain536C90 | In outline: a step when the terrain is not flagged and not flagged (+0x21C) |
| `0x4df040` | Ride::Next4DF040 | In outline: steps a counter (+0x250) around |
| `0x58a400` | Ride::Station58A400 | In outline: finds a station (`0x5D1D80` with +0xF4, +0xF8) into +0x104 |
| `0x5f2460` | TrackPiece::End5F2460 | In outline: picks an end record (-2: +0xA4, -1: +0xB0) |
| `0x5eaad0` | AttractionPerson::Reset5EAAD0 | In outline: clears a flag (+0x16D) and goes over its list (+0x170) |
| `0x50d1b0` | Car::Time50D1B0 | In outline: adds a time to a value (+0x96C) and acts |
| `0x4b79d0` | Shop::Set4B79D0 | In outline: when not flagged (+0x15C), sets a value (+0x138) for 0 to 10 |
| `0x5bc210` | TrackPiece::Lerp5BC210 | Blends two integer pairs by a fraction, rounded |
| `0x5b55c0` | TrackedRide::Step5B55C0 | In outline: a step and, with no part (+0x70) and not flagged (+0x861), more (`0x5B0370`) |
| `0x5a8a80` | TrackedRide::Reset5A8A80 | In outline: clears slots (+0x3E4 to +0x3EC = -1) and goes over its stations |
| `0x592130` | Ride::Check592130 | In outline: a check when not flagged (+0x204) |
| `0x4d2700` | Shop::Link4D2700 | In outline: a step with a linked part (+0x48) |
| `0x5636a0` | Ride::Count5636A0 | In outline: counts entries (+0xC4) |
| `0x4acd60` | Ride::Check4ACD60 | In outline: a step when a value differs from the shared one (`0x12F4B68`) |
| `0x513910` | Car::Step513910 | In outline: a step with a slot when on (+0x250) |
| `0x4c58f0` | Shop::Value4C58F0 | In outline: a step by a value against a limit |
| `0x5a7ea0` | TrackedRide::Step5A7EA0 | In outline: steps (Thunk, `0x531200`) |
| `0x588ff0` | Ride::Place588FF0 | In outline: places a part beside another (+0x108) |
| `0x506dd0` | Ride::Mode506DD0 | In outline: 0 in modes 2 and 3 (+0xE0) |
| `0x49d270` | Text::Mode49D270 | In outline: a text by the shared mode (`0x10EE248`) |
| `0x49ceb0` | Text::Mode49CEB0 | In outline: the same (another copy) |
| `0x440d10` | Effect::Set440D10 | In outline: when a value changes (+0x6C vs +0x74) or flagged (+0x7C), takes it and acts |
| `0x598910` | Ride::Text598910 | In outline: an empty text filled from its part (+0x144, slot 0x4C) |
| `0x530030` | Ride::Set530030 | In outline: sets a value (SetA8AndUpdate) and a part (+0x294) |
| `0x4ace60` | Ride::SetMode4ACE60 | In outline: sets a mode (+0x354) when it changes, clearing a value (+0x360) |
| `0x7b4800` | Effect::Inside7B4800 | In outline: unless told not to, whether a box lies inside its bounds |
| `0x41c900` | Window::Text41C900 | In outline: a text unless a flag is set (Flags::Bit0) |
| `0x58ad50` | Ride::Time58AD50 | In outline: a timed step (`0x58A7A0`) with a value (+0x38) |
| `0x58ac50` | Ride::Time58AC50 | In outline: the same (another copy) |
| `0x5c0d30` | TrackPiece::Update5C0D30 | In outline: a scenery update (Scenery::Update848C90) and a step when not flagged (+0x174) |
| `0x5c0c30` | TrackPiece::Update5C0C30 | In outline: the same with Scenery::Step848B20 |
| `0x5c0b30` | TrackPiece::Update5C0B30 | In outline: the same with Scenery::Step8489B0 |
| `0x5c0a30` | TrackPiece::Update5C0A30 | In outline: the same with Scenery::Step848840 |
| `0x5bb860` | TrackPiece::Clear5BB860 | In outline: clears a flag (+0x214), a record (Record::Clear) and a handle |
| `0x568760` | Ride::Tool568760 | In outline: a step with the current tool (`0x12F5638`) and entry +0x27C (32 bytes) |
| `0x563a30` | Ride::Check563A30 | In outline: a check when not flagged (+0x254) |
| `0x5112f0` | Car::Lights5112F0 | In outline: updates lights (Model::UpdateLights) and goes over its parts (+0x314) |
| `0x5bb3a0` | Ride::Last5BB3A0 | In outline: a step with the last 0x24-byte entry |
| `0x501da0` | Ride::Flag501DA0 | In outline: a step comparing a flag (bit 22) with a count (slot 0x5C) |
| `0x4cd820` | Shop::Find4CD820 | In outline: a find over a list (+0x64) |
| `0x5dab20` | TrackedRide::Check5DAB20 | In outline: 0 unless a check passes (`0x5657D0`) |
| `0x4b51e0` | Ride::Check4B51E0 | In outline: a check when not flagged (+0x5C) |
| `0x559d10` | Ride::Cells559D10 | In outline: with no cached object, a ride's cells (Ride::Cells557EE0) |
| `0x559c20` | Ride::Cells559C20 | In outline: the same with Interface04b::Get |
| `0x4b00a0` | Ride::Check4B00A0 | In outline: 0 when its part is flagged (bit 4), more when flagged (bit 14) |
| `0x5bcb20` | TrackPiece::Handle5BCB20 | In outline: makes a handle and links it |
| `0x4e1410` | Ride::Make4E1410 | In outline: fills a record from its arguments |
| `0x5d4e20` | TrackedRide::Pass5D4E20 | In outline: goes over stations (+0xE8) when not flagged (+0x68) |
| `0x51a5c0` | Car::Pass51A5C0 | In outline: goes over entries (+0x7B4) |
| `0xbc64e0` | Track::LengthBC64E0 | In outline: for rides not flagged 0x400000 or with fewer than two stations, a step |
| `0x579f00` | Ride::Part579F00 | In outline: with no part (+4), a step with +0x40 |
| `0x53b440` | Ride::Kind53B440 | In outline: unless its kind is 0x13, a step with 1.0 |
| `0x5d9830` | TrackPiece::Find5D9830 | In outline: a find over pieces (+0x158) |
| `0x5d6410` | TrackPiece::Step5D6410 | In outline: when given a piece, a step unless told not to |
| `0x591af0` | Ride::Text591AF0 | In outline: an empty text filled when not flagged (+0x1C4) |
| `0x5863f0` | Ride::Text5863F0 | In outline: the same when not flagged (+0x1AC) |
| `0x523910` | Car::Update523910 | In outline: an update when not flagged (+0x180) |
| `0x50bbe0` | Car::Step50BBE0 | In outline: with the shared object (`0x12F4F94`), a step for a slot |
| `0x4f6f40` | TrackEditor::AutoComplete4F6F40 | In outline: with a part (+0x4F0), hides the auto-complete cost (TrackEditor::AutoCompleteCost(0)) |
| `0x4acc70` | Ride::Step4ACC70 | In outline: a step (`0x4AB750`) and more when the shared flag (`0x12F4B60`) is set |
| `0x54c290` | Ride::List54C290 | In outline: goes over a list (+0x78) |
| `0x53e190` | Ride::Pass53E190 | In outline: goes over entries (+0x34) |
| `0x593280` | Ride::List593280 | In outline: goes over a list (+0x208) |
| `0x500160` | Car::Set500160 | In outline: sets a value (+0xCC) when it changes, by a mode (+0x100) |
| `0x5a1660` | TrackedRide::Set5A1660 | In outline: sets a value (+0x6F4) and acts with its owner |
| `0x555a60` | Ride::Loop555A60 | In outline: goes over the shared list (`0x10EE8BC`) |
| `0x532bb0` | Ride::Check532BB0 | In outline: a check when not flagged (+0x21C) |
| `0x5ca270` | TrackedRide::Check5CA270 | In outline: a step with a check (TrackedRide::Check5C7000) |
| `0x568670` | Ride::Tool568670 | In outline: a step with the current tool (`0x12F5638`, `0x5D7D20`) |
| `0x5b0370` | TrackedRide::Stations5B0370 | In outline: goes over stations (+0x6B0) with a step (`0x4C1740` with 0) |
| `0x59ba50` | Ride::Value59BA50 | In outline: a value when not flagged (+0x84) |
| `0x580140` | Ride::Set580140 | In outline: sets a byte (+0x100) and a list (Array4::Assign) |
| `0x5bef10` | TrackPiece::Side5BEF10 | In outline: a step with 1 or -1 by a side |
| `0x55fa00` | Ride::Blend55FA00 | In outline: blends three values (+0x320, +0x3DC) |
| `0x5340e0` | Ride::Count5340E0 | In outline: counts when not flagged (+0x21C) |
| `0x533ff0` | Ride::Count533FF0 | In outline: the same (another copy) |
| `0x5bb2b0` | TrackPiece::Length5BB2B0 | In outline: a step with two parts' lengths (+0x18) |
| `0x5d9a00` | TrackedRide::End5D9A00 | In outline: with the selected ride, a step (TrackedRide::End5CA360) |
| `0x5a48b0` | TrackedRide::Section5A48B0 | In outline: a step with a ride's section (`0x12F5338`) |
| `0x5ede30` | AttractionPerson::Select5EDE30 | In outline: a step with the selected ride from a value (+0x120, `0x5EBEB0`) |
| `0x5de900` | TrackedRide::Track5DE900 | In outline: a step with the track interface (TrackedRide::TrackInterface) with none at +0x70 |
| `0x56fa10` | Ride::Update56FA10 | In outline: an update when not flagged (+0x5C) |
| `0x510c50` | Car::Slot510C50 | In outline: for a part (0x90 bytes) with no slot (+0x430), a step |
| `0x4d1ab0` | Shop::Set4D1AB0 | In outline: sets a value (+0xB4) and a mode of 5 (+0xB0) |
| `0x5db910` | Ride::CanOpen5DB910 | In outline: a step when a ride cannot open (`0x5CFB10`) |
| `0x5d5450` | TrackedRide::Count5D5450 | In outline: a count from the selected ride |
| `0x5c1330` | TrackPiece::Check5C1330 | In outline: a check when not flagged (+0x214) |
| `0x5be100` | TrackPiece::Extent5BE100 | In outline: clears an extent (Extent::Clear) and fills it (`0x847530`) |
| `0x591e60` | Ride::Find591E60 | In outline: a find over a list (+0x220) |
| `0x56f930` | Ride::Update56F930 | In outline: an update when not flagged (+0x5C) |
| `0x560fe0` | Ride::Check560FE0 | In outline: a check when not flagged (+0x5C) |
| `0x537760` | Ride::Count537760 | In outline: counts when not flagged (+0x21C) |
| `0x5fb800` | Ride::Changed5FB800 | In outline: when a value (+0xC) differs from another (`0x47BE20`), takes it (`0x47C4E0`) |
| `0x534c00` | Ride::Section534C00 | In outline: a step with a ride's section (`0x12F5338`, `0x557C20`) and links (WithReference4AA7E0) |
| `0x51b350` | Car::Check51B350 | In outline: a check when on (+0x250) and not in mode 0x10 (+0x384) |
| `0x4cf2a0` | Shop::Make4CF2A0 | In outline: with nothing given, makes one (`0x4CD030`) |
| `0x4affc0` | Ride::Check4AFFC0 | In outline: 0 when its part is flagged (bit 4), more when flagged (bit 14) |
| `0x561420` | Ride::Owner561420 | In outline: a step with its owner |
| `0x5ed820` | AttractionPerson::Reset5ED820 | In outline: sets slots (+0x11C = -2, +0x120 = 0) and more |
| `0x4cf7c0` | Shop::Count4CF7C0 | In outline: a step with a count (`0x4CE0E0`) |
| `0x5d9640` | TrackedRide::Selected5D9640 | In outline: with the selected ride not flagged (+0x170), a step |
| `0x5a51a0` | TrackedRide::Set5A51A0 | In outline: for 0, sets a value (+0x3B4) |
| `0x53e070` | Ride::Make53E070 | In outline: makes a record |
| `0x4de430` | Ride::Reset4DE430 | In outline: a step (slot 0x7C) and sets a value to 2 (+8) |
| `0x5e6910` | TrackPiece::Step5E6910 | In outline: steps (`0x5DBAD0`, `0x5C95B0`) |
| `0x5c0120` | TrackPiece::Check5C0120 | In outline: a check when not flagged (+0x154) |
| `0x59e600` | Ride::Step59E600 | In outline: a step with many values |
| `0x563cc0` | Ride::Check563CC0 | In outline: a check when not flagged (+0x254) |
| `0x54f0a0` | Ride::Slot54F0A0 | In outline: with a part (+0x44) and no slot (+0x74), a step |
| `0x5293c0` | Car::Round5293C0 | In outline: a value from a place (+0x2B0, +0x2B4), rounded |
| `0x51bad0` | Car::Pass51BAD0 | In outline: goes over entries (+0x7B4) with a step (`0x518300`) |
| `0x5d9cf0` | TrackedRide::Selected5D9CF0 | In outline: with no selected ride, a step (`0x5CD4D0`) |
| `0x5d9af0` | TrackedRide::Selected5D9AF0 | In outline: with the selected ride, a piece check (TrackPiece::Check5BDC00) |
| `0x5d9920` | TrackedRide::Selected5D9920 | In outline: a count with the selected ride |
| `0x592050` | Ride::Step592050 | In outline: a step unless in mode 2 (+0x70) |
| `0x5026d0` | Ride::Shared5026D0 | In outline: with the shared object (`0x12F503C`) and its check (slot 0x68), a step |
| `0x4ef990` | Ride::Shared4EF990 | In outline: with the shared value (`0x10EDC98`), a step |
| `0x4df670` | Ride::Check4DF670 | In outline: with no part (+0x250), a check when not flagged (+0x4C) |
| `0x5dac10` | TrackedRide::Count5DAC10 | In outline: 0 with no entries (+0xF0) |
| `0x54e640` | Ride::Once54E640 | In outline: once (+0x40 bit 0), a step |
| `0x5498c0` | Ride::Pass5498C0 | In outline: goes over entries (+0x58, +0x5C) |
| `0x5da910` | TrackPiece::Entry5DA910 | In outline: a step for entry n (0x1C bytes at +0xDC) under a count (+0x158) |
| `0x5d0dc0` | TrackedRide::Find5D0DC0 | In outline: a find over a list |
| `0x597b30` | Ride::Handle597B30 | In outline: makes a handle and links it |
| `0x4ec380` | Ride::Make4EC380 | In outline: makes a part |
| `0x4ca600` | Shop::Make4CA600 | In outline: makes a 0x104-byte part |
| `0x4ca520` | Shop::Make4CA520 | In outline: the same (another copy) |
| `0x4ca440` | Shop::Make4CA440 | In outline: the same (another copy) |
| `0x7cfb00` | Person::Ride7CFB00 | In outline: sets a person's state to 0x102 and draws a model (Model::Draw7C6580) |
| `0x59d940` | Ride::Entry59D940 | In outline: a step with an entry that is set (Entry::IsSet2) |
| `0x58b9d0` | Ride::Count58B9D0 | In outline: a count with the selected ride |
| `0x503b80` | Ride::Window503B80 | In outline: with a part (+0x18), a window step (Ride::Window5036D0) |
| `0x503ab0` | Ride::Window503AB0 | In outline: the same (another copy) |
| `0x503860` | Ride::Window503860 | In outline: the same (another copy) |
| `0x4f7820` | Ride::Check4F7820 | In outline: 0 with no part (+0x4A8), more unless kind 3 |
| `0x534d80` | Ride::Count534D80 | In outline: counts when not flagged (+0x21C) |
| `0x529bd0` | Car::Check529BD0 | In outline: a check when not flagged (+0xD8) |
| `0x4fe690` | Ride::Part4FE690 | In outline: a step with its part (+0x100, or +0xD8) |
| `0x4bbae0` | Shop::Set4BBAE0 | In outline: when a value changes (+0x130), makes a 0x268-byte part |
| `0x5d4d20` | TrackedRide::Check5D4D20 | In outline: 0 with no entries |
| `0x59b880` | Ride::Make59B880 | In outline: makes a 32-byte part |
| `0x585920` | Ride::Owner585920 | In outline: a step when its owner's kind is 0xB |
| `0x4dea30` | Ride::Set4DEA30 | In outline: a step (`0x7A0B50`) when its part differs |
| `0x513a60` | Car::Update513A60 | In outline: a large ride car update: with no slot (+0x318), finds its ride, the current tool and its list (+0x8D0) and moves along it |
| `0x577d00` | Ride::Update577D00 | In outline: a large ride update: adds a time (+0x64), for kind 0x33 and others acts with its ride's part (+0x38C) and the current tool's value (+0x108) by many kinds (0xD, 0x16, 0xF, 0x34, ...) |
| `0x5274b0` | Car::Move5274B0 | In outline: a large car move: keeps 1 over a shared step (`0x10EE448`), finds its ride's section and the current tool, and works out a place along the track |
| `0x5242e0` | Car::Update5242E0 | In outline: a large car update with its ride, when the ride is not flagged (+0x861) |
| `0x5e2b20` | TrackPiece::Build5E2B20 | In outline: a large piece build: clears a list (Array::Resize5D77A0(0)) and values and fills from its ride |
| `0x764790` | Guest::Case764790 | In outline: a large guest step with two parts (by shared ids `0x12F4C64`, `0x12F5494`) and a case |
| `0x515bc0` | Car::Parts515BC0 | In outline: with the shared object (`0x12F4F94`) and its ride, steps each car part slot (Car::Step50BBE0 with 1, 4, 6, 9, 0xB, ...) by flags |
| `0x568b80` | Ride::Step568B80 | In outline: a large ride step: counts up (+0x304) and works with a car's part (+0x198) |
| `0x5d2ab0` | TrackedRide::Ray5D2AB0 | In outline: a large step for kinds 1, 9 and 10 with a ray step (`0xBCC740`) |
| `0x539090` | Ride::Section539090 | In outline: a large step with a ride's section, making a 0x48-byte part when it has none |
| `0x53eea0` | Ride::Text53EEA0 | In outline: a large text step comparing names (WideString::Compare) |
| `0x525a50` | Car::SetMode525A50 | In outline: sets a car's mode (+0x384) and acts by it (6: Car::Update5242E0, ...) |
| `0x784dd0` | GuestGroup::Check784DD0 | In outline: a check when not flagged (+0x11C) |
| `0x52d5d0` | Ride::Check52D5D0 | In outline: when on (+0x1C0), a check with its ride when not flagged (+0xD8) |
| `0x4d28a0` | Shop::Reset4D28A0 | In outline: sets two modes to 2 (+0xB0, +0xB4) and acts with its part |
| `0x4d7e70` | Shop::Text4D7E70 | In outline: with nothing given, makes an empty text record |
| `0x5da220` | TrackedRide::Hit5DA220 | In outline: a hit test (TrackedRide::Check5D9E80) with many values |
| `0x4abd00` | Ride::Text4ABD00 | In outline: a ride text (Ride::Text534E60) and more when not flagged (+0x384) |
| `0x4af400` | Ride::Step4AF400 | In outline: steps (`0x52EDD0`, Ride::Check532BB0) and acts with an entry that is not set |
| `0x4ab7e0` | Ride::Slot4AB7E0 | In outline: when flagged (bit 14), a slot from n by counts (+0x370 / +0x394) |
| `0x4c7f80` | Shop::Reset4C7F80 | In outline: clears values, sets one to 1.0 and a flag from a check (slot 0x144) |
| `0x5c0720` | TrackPiece::Check5C0720 | In outline: a check when not flagged (+0xAC) |
| `0x4a46d0` | Ride::Text4A46D0 | In outline: with no part (+0x17C), a shared text (+0x78) |
| `0x4c10a0` | Attraction::Closed | In outline: in mode 2 (+0x70), clears a flag (bit 6) and shows the closed text (Attraction::ClosedText) |
| `0x4c1680` | Shop::Check4C1680 | In outline: a check when not flagged (+0x100) |
| `0x4a90f0` | Ride::Slot4A90F0 | In outline: a slot step (Ride::Slot531240) and with the shared value, goes over entries (+0x454) |
| `0x582db0` | Ride::Next582DB0 | In outline: steps a counter (+0xD8) round 0 to 3 and acts with a part (+0x94) |
| `0x53cad0` | Ride::Set53CAD0 | In outline: sets a scaled value (+4) and takes a shared part (+0x34) |
| `0x59a1e0` | Ride::Compare59A1E0 | In outline: compares a value with 0 |
| `0x5dbcd0` | TrackPiece::Time5DBCD0 | In outline: with a non-negative time, a step (`0x5D97A0`) and compares with a value (+0x108) |
| `0x5527c0` | Ride::Part5527C0 | In outline: a step with a part (by a shared id `0x12F51B4`) |
| `0x535520` | Ride::Section535520 | In outline: under a count (slot 0x138), a step with section n |
| `0x5d97a0` | TrackedRide::Check5D97A0 | In outline: 1 when a check passes (`0x5D9720`), else another (`0x59AF60`) |
| `0x531b10` | Ride::Slot531B10 | In outline: for n from 0 under a count (slot 0x138), a step when not flagged (+0x21C) |
| `0x5294a0` | Car::Angle5294A0 | In outline: the difference of two angles kept within half a turn |
| `0x531730` | Ride::Slot531730 | In outline: the same check for n under a count (slot 0x138) |
| `0x5316a0` | Ride::Slot5316A0 | In outline: the same (another copy) |
| `0x4d2800` | Shop::Event4D2800 | In outline: for 3, unless a check passes, a step with an object (`0x12F4EB0`) |
| `0x5302a0` | Ride::Case5302A0 | In outline: false for 0, true for others |
| `0x835350` | Scenery::Step835350 | In outline: a step (`0x4DE970`) and acts with a part |
| `0x5903c0` | Ride::Make5903C0 | In outline: makes a 0xAC-byte part from its part (+0x1C8) |
| `0x585b20` | Ride::Make585B20 | In outline: the same with +0x1D0 |
| `0x4a8910` | Ride::Make4A8910 | In outline: the same with +0x3A8 |
| `0xbc5b40` | Track::EndB | In outline: an end value by case (0: +0x24, 1: +0x2C, ...) |
| `0xbc5ab0` | Track::EndA | In outline: an end value by case (0: +0x20, 1: +0xD8, ...) |
| `0x4de8a0` | Ride::Value4DE8A0 | In outline: with a part, a value from an object (`0x12F4F44`, slot 0x10) |
| `0x58ea70` | Ride::Refresh58EA70 | In outline: when on (+0x10), a refresh (`0x58E950`) and takes a value (slot 0x1C) |
| `0x4e5fb0` | AttractionPerson::Event4E5FB0 | In outline: with the attraction's person, unless a shop event passes, a step in mode 2 |
| `0x5601f0` | Ride::Value5601F0 | In outline: a value from a byte (+9), at least 1 |
| `0x5bf4a0` | TrackPiece::Compare5BF4A0 | In outline: compares two values |
| `0x7c4340` | Person::Check7C4340 | In outline: when not flagged (+0xC7), 0 unless 0 or 2, else a check |
| `0x7c3100` | Person::Mode7C3100 | In outline: when flagged (+0xC7) and a check passes, sets a value to 0x140 |
| `0x4d2b20` | Shop::Reset4D2B20 | In outline: sets two modes (+0xB0 = 4, +0xB4 = 3) and acts with its part |
| `0x5dbbf0` | TrackedRide::Check5DBBF0 | In outline: a check (TrackedRide::Check5D97A0) when one value is up to another |
| `0x50e6d0` | Car::Kind50E6D0 | In outline: 2, or 3 or 4 when flagged (bit 3), by counts (+0x7B4, +0x8C0) |
| `0x5d3ec0` | TrackedRide::Copy5D3EC0 | In outline: copies a value from another of the same kind (`0xF3F71C`) |
| `0x5d3e60` | TrackedRide::Copy5D3E60 | In outline: the same with `0xF3F714` |
| `0x5d3d40` | TrackedRide::Copy5D3D40 | In outline: the same with `0xF3F70C` |
| `0x552d70` | Ride::Part552D70 | In outline: a step with a part (by a shared id `0x12F51B4`) |
| `0x552850` | Ride::Part552850 | In outline: the same (another copy) |
| `0x52ce20` | Car::Pivot52CE20 | In outline: checks a pivot (RideCar::CheckPivot) and a step |
| `0x4aa860` | Ride::Case4AA860 | In outline: a step by case from 5 |
| `0x507950` | Car::Set507950 | In outline: sets a value (+0xF4) and acts with a part (+0x134) |
| `0x4bfaa0` | Shop::Value4BFAA0 | In outline: a value from its part's values |
| `0x4c4ba0` | Shop::Swing4C4BA0 | In outline: adds to a value (+0x90), kept above a shared limit |
| `0x4d1bb0` | Shop::Part4D1BB0 | In outline: a step with a part (by a shared id `0x12F4C64`) |
| `0x5b54e0` | TrackedRide::Check5B54E0 | In outline: 0 unless a check passes (slot 0x4C) |
| `0x5d1270` | TrackedRide::Case5D1270 | In outline: a value by case (0x20: when a mode is not 2, ...) |
| `0x51bbb0` | Car::Find51BBB0 | In outline: a find over entries (+0x7B4) |
| `0x5cdf30` | Ride::Text5CDF30 | In outline: a text from a shared one (`0xF3F4A4`) |
| `0x5bdd90` | TrackPiece::Value5BDD90 | In outline: a value from its owner (slot 0x94) by a flag (+0x190) |
| `0x5a1ce0` | TrackedRide::Slot5A1CE0 | In outline: a step when a slot (+0x2A0) is -1 or past a count |
| `0x500860` | Car::Reset500860 | In outline: clears values |
| `0x5be8b0` | TrackPiece::Last5BE8B0 | In outline: a step with the last 0x24-byte entry |
| `0x5eebe0` | AttractionPerson::Select5EEBE0 | In outline: sets a value (+0xBC = 1) and a step with the selected ride |
| `0x5dbad0` | TrackedRide::Make5DBAD0 | In outline: makes a part |
| `0x5bdb30` | TrackPiece::Value5BDB30 | In outline: a value when its owner's value (+0x10) is 0 |
| `0x5a3ab0` | TrackedRide::Owner5A3AB0 | In outline: a step with its owner and a value (+0x6F4) |
| `0x5637a0` | Ride::Slot5637A0 | In outline: with no slot (+0x308 = -2), -1 when there are no entries |
| `0x58b3b0` | Ride::All58B3B0 | In outline: goes over rides from the selected one |
| `0x585c00` | Ride::Part585C00 | In outline: a step with a part (+0x2C) |
| `0x4ff580` | Car::Count4FF580 | In outline: a step for a positive count |
| `0x58b2e0` | Ride::All58B2E0 | In outline: goes over rides from the selected one |
| `0x4afef0` | Ride::Check4AFEF0 | In outline: 0 when its part is flagged (bit 4), more when not flagged (bit 14) |
| `0x562b70` | Ride::Mode562B70 | In outline: in mode 2 (+0x60), a step when not flagged |
| `0x535450` | Ride::Section535450 | In outline: sets section n's value (+0x64) and tells (slot 0xE0) |
| `0x525fc0` | Car::Update525FC0 | In outline: an update when not flagged (+0x180) |
| `0x4cf0c0` | Shop::Set4CF0C0 | In outline: with nothing given, a step (`0x4CCF50`) |
| `0x5ab230` | Ride::Values5AB230 | In outline: takes two values (slots 0x40, 0x3C) and acts |
| `0x4de7d0` | Ride::Init4DE7D0 | In outline: sets up a record with its table (`0xF322E8`) |
| `0x5f3f20` | Ride::Part5F3F20 | In outline: a step with an object's slot (`0x12F4B34`) |
| `0x4ceff0` | Shop::Set4CEFF0 | In outline: with nothing given, a step (`0x4CCF00`) |
| `0x5cf2c0` | TrackedRide::Check5CF2C0 | In outline: a check when not flagged (+0xAC) |
| `0x5c2150` | TrackPiece::Tool5C2150 | In outline: a step with the current tool (`0x12F5638`, `0x5D4950`) |
| `0x4debc0` | Ride::Step4DEBC0 | In outline: a step with its part (slot 0x54) |
| `0x5d0300` | TrackPiece::Side5D0300 | In outline: a step with 1 or -1 by a side |
| `0x5bc660` | TrackPiece::Check5BC660 | In outline: a check when not flagged (+0x108) |
| `0x595600` | Ride::Entry595600 | In outline: a step for an entry (`0x5947B0`) not flagged |
| `0x561180` | Ride::Time561180 | In outline: with a negative time (+0x70), a step when not flagged (+0xB4) |
| `0x5928c0` | Ride::Update5928C0 | In outline: an update when not flagged (+0x204) |
| `0x5039f0` | Ride::Window5039F0 | In outline: with a part (+0x18), a window step |
| `0x503930` | Ride::Window503930 | In outline: the same (another copy) |
| `0x52ba20` | Car::Value52BA20 | In outline: a value unless its owner's kind is 0x53 |
| `0x4c8230` | Shop::Mode4C8230 | In outline: in mode 1 (+0xC4), a step with a value (+0xD8) |
| `0x591a30` | Ride::Check591A30 | In outline: a check when not flagged (+0x204) |
| `0x586330` | Ride::Check586330 | In outline: a check when not flagged (+0x1EC) |
| `0x4abc40` | Ride::Check4ABC40 | In outline: a check when not flagged (+0x3C4) |
| `0x5a7d00` | Ride::Check5A7D00 | In outline: a check when not flagged (+0x378) |
| `0x4deb00` | Ride::Set4DEB00 | In outline: a step when its part differs (slot 0x54) |
| `0x4c74d0` | Shop::Value4C74D0 | In outline: a value when not flagged (+0x58) |
| `0x590440` | Ride::Part590440 | In outline: a step with a part (+0x2C) |
| `0x585cd0` | Ride::Step585CD0 | In outline: a step with a work buffer |
| `0x4f3ad0` | Ride::Step4F3AD0 | In outline: a step with a value (+0x5E4) |
| `0x5d5730` | TrackPiece::Count5D5730 | In outline: a step with a count (+0x11C) plus one |
| `0x5c1410` | TrackPiece::Shared5C1410 | In outline: unless a shared flag is set (`0x12F55F0`), a step with a work buffer |
| `0x5ae580` | Ride::Handle5AE580 | In outline: makes a handle and links it |
| `0x582a30` | Ride::SetPart582A30 | In outline: sets a part (+0x1B8) when it changes |
| `0x5bf180` | TrackPiece::Check5BF180 | In outline: a check when on |
| `0x4f5470` | Ride::Check4F5470 | In outline: a step when a check passes (slot 4) |
| `0x5bcc10` | TrackPiece::Scenery5BCC10 | In outline: a scenery step (Scenery::Step84BB60) with the current tool |
| `0x555420` | Ride::Case555420 | In outline: a step by a byte's case (0, ...) |
| `0x4de970` | Ride::Time4DE970 | In outline: adds to a time (+0x10) unless a shared flag is set (`0x10EDC18`) |
| `0x5a7240` | Ride::Value5A7240 | In outline: 100 when not flagged (+0x390), else a value |
| `0x59e270` | Ride::Handle59E270 | In outline: makes a handle and links it |
| `0x562eb0` | Ride::Step562EB0 | In outline: when given, a step |
| `0x569e50` | Ride::Tool569E50 | In outline: a step with a value (+0x228) and the current tool |
| `0x4cf380` | Shop::Mode4CF380 | In outline: for mode 3 or 1, a step with a part (+0x10) |
| `0x99fe70` | Fireworks::Cost99FE70 | In outline: a total load (Cars::TotalLoad) and a money text (Money::FormatOrUnlimited) |
| `0x5bd7a0` | TrackPiece::Terrain5BD7A0 | In outline: a step when the terrain is not flagged |
| `0x5adf90` | Ride::Pass5ADF90 | In outline: goes over entries (+0x6B0) |
| `0x58bda0` | Ride::Point58BDA0 | In outline: a point from the selected ride (Ride::Selected58BAB0, +0x18) |
| `0x58bce0` | Ride::Point58BCE0 | In outline: the same with +0xC |
| `0x58b1f0` | Ride::Value58B1F0 | In outline: a value from a part (slot 0x28) |
| `0x4f1980` | Ride::Check4F1980 | In outline: a check with a work buffer |
| `0x5cde00` | Ride::Step5CDE00 | In outline: a step with many values |
| `0x5cc680` | TrackedRide::Handle5CC680 | In outline: when it matches a part's value (+0x200 +0xC), makes a handle |
| `0x50cf50` | Car::Add50CF50 | In outline: adds when there is room |
| `0x4b59c0` | Ride::Check4B59C0 | In outline: a step when a check fails (Ride::Check4B51E0) |
| `0x5d1e69` | TrackPiece::Fragment5D1E69 | In outline: a fragment that sets a part (`0xF3F6D4`) under a flag |
| `0x579ff0` | Ride::Init579FF0 | In outline: fills a record from its arguments |
| `0x4bbbc0` | Ride::Shared4BBBC0 | In outline: with a shared value (`0x1313C88` +0x20), a step with a value (+0x398) |
| `0x549ba0` | Ride::Part549BA0 | In outline: a step with a part (+0x4C) |
| `0x5f1f20` | Ride::Handle5F1F20 | In outline: makes a handle and links it |
| `0x5ae770` | Ride::Handle5AE770 | In outline: the same (another copy) |
| `0x59af70` | Ride::Step59AF70 | In outline: a step with many values |
| `0x58e880` | Array::Grow58E880 | In outline: grows a list to hold n plus one |
| `0x4d5580` | Ride::Value4D5580 | In outline: a value when not flagged (+0x378) |
| `0x4aa5b0` | Ride::Value4AA5B0 | In outline: a value when not flagged (+0x3C4) |
| `0x49d5a0` | Text::Format49D5A0 | In outline: formats a value into a 128-character buffer |
| `0x49af92` | GameTime::Fragment49AF92 | In outline: a fragment that takes the game time |
| `0x49ae02` | GameTime::Fragment49AE02 | In outline: the same (another copy) |
| `0x496fe0` | Text::Step496FE0 | In outline: a step with a 1024-character buffer |
| `0x59bd50` | Ride::Check59BD50 | In outline: a check when not flagged (+0x84) |
| `0x562c40` | Ride::Kind562C40 | In outline: for kind 0x1D (+0x60), a step when not flagged (+0x238) |
| `0x5d44e0` | TrackPiece::Set5D44E0 | In outline: sets a value (+4) when it changes |
| `0x5d43c0` | TrackPiece::Set5D43C0 | In outline: the same (another copy) |
| `0x5d42a0` | TrackPiece::Set5D42A0 | In outline: the same (another copy) |
| `0x5e1010` | TrackedRide::Pass5E1010 | In outline: goes over entries (+0x114) |
| `0x5dfe40` | TrackedRide::Track5DFE40 | In outline: a step with the tracked ride and track interfaces |
| `0x5d7b10` | TrackPiece::Handle5D7B10 | In outline: makes a handle and links it |
| `0x55ae80` | Ride::Handle55AE80 | In outline: the same (another copy) |
| `0x55add0` | Ride::Handle55ADD0 | In outline: the same (another copy) |
| `0x52c7c0` | Car::Check52C7C0 | In outline: a check when not flagged (+0xD8) |
| `0x513740` | Car::Reset513740 | In outline: clears a flag (+0x130) and makes a handle |
| `0x508280` | Car::Handle508280 | In outline: makes a handle and links it |
| `0x4ef0e0` | Ride::Mode4EF0E0 | In outline: in mode 3 or 1 (+0x144), a step |
| `0x58c670` | Ride::Interface58C670 | In outline: with an interface (Interface44::Get), a step |
| `0x573a40` | Ride::Update573A40 | In outline: an update when not flagged (+0x5C) |
| `0x53b530` | Ride::Init53B530 | In outline: fills a record from its arguments |
| `0x50bf80` | Car::Shared50BF80 | In outline: when a shared flag is set (`0x12F50E1`), a step with a shared value (`0x12F50DC`) |
| `0x58f7e0` | Ride::Refresh58F7E0 | In outline: refreshes (Ride::Refresh58EA70) and acts with a point |
| `0x532b00` | Ride::SetPoint532B00 | In outline: copies a point into its values (+0x1C0 on) |
| `0x4c7420` | Shop::Update4C7420 | In outline: an update when not flagged (+0x58) |
| `0x5d08a0` | TrackPiece::Init5D08A0 | In outline: fills a record from its arguments |
| `0x4d3390` | Shop::Handle4D3390 | In outline: a step (`0x4D2C10`) and a handle (Handle::For) |
| `0x563870` | Rides::Pass563870 | In outline: goes over entries (+0xC4) |
| `0x5a4d10` | Rides::Pass5A4D10 | In outline: goes over entries (+0x6B0) |
| `0x501fc0` | Rides::Make501FC0 | In outline: makes a 0x268-byte part |
| `0x5d6de0` | Rides::Pass5D6DE0 | In outline: goes over entries (+0xE8) |
| `0x5d6ac0` | Rides::Pass5D6AC0 | In outline: goes over entries (+0xE8) |
| `0x55ac90` | Rides::Handle55AC90 | In outline: makes a handle and links it |
| `0x4b9cc0` | Rides::Make4B9CC0 | In outline: makes a 0x10-byte part |
| `0x5dba20` | TrackedRide::Make5DBA20 | In outline: makes a part |
| `0x5a3d60` | TrackedRide::Value5A3D60 | In outline: a value when not flagged (+0x378) |
| `0x59c8d0` | Ride::Tool59C8D0 | In outline: goes over entries with the current tool |
| `0x557dc0` | Ride::Pass557DC0 | In outline: goes over entries (+0x98) |
| `0x53b5f0` | Ride::Init53B5F0 | In outline: a step (`0x441060`) and sets up a record with its table (`0xF38A74`) |
| `0x4e1050` | Ride::Point4E1050 | In outline: a point from a part (+0x270 +0x40, slot 0x1C) |
| `0x4c3670` | Shop::Check4C3670 | In outline: a check with a part (+0x140, slot 0x3C) |
| `0x5327b0` | Ride::Mode5327B0 | In outline: 0 unless in mode 2 (+0x70), else a step |
| `0x4ee950` | Ride::Extent4EE950 | In outline: clears an extent (Extent::Clear) and acts |
| `0x4aa4f0` | Ride::Value4AA4F0 | In outline: a value when not flagged (+0x3C4) |
| `0x5301f0` | Ride::Set5301F0 | In outline: sets a value to 1 (+0x118) and acts |
| `0x51e8c0` | Car::Buffer51E8C0 | In outline: makes an aligned buffer for each entry (+0x7B4) |
| `0x5cf980` | TrackedRide::Slot5CF980 | In outline: a value from slot n (+0x28) under a count (+0x68) |
| `0x4f0e80` | Ride::Slot4F0E80 | In outline: a step with slot n (+0x748) |
| `0x4ef1d0` | Ride::Set4EF1D0 | In outline: a step when a value changes |
| `0x4f3b90` | Ride::Settings4F3B90 | In outline: sets defaults (Settings5D::Defaults) and acts |
| `0x4ee810` | Ride::Step4EE810 | In outline: a step (`0x4BBDA0`) and a value (+0x144) |
| `0x4ee760` | Ride::Step4EE760 | In outline: the same with `0x4BBD80` |
| `0x4ee6b0` | Ride::Step4EE6B0 | In outline: the same with `0x4BBD60` |
| `0x5605f0` | Ride::Count5605F0 | In outline: a count (`0xBCD1F0`) |
| `0x532ca0` | Ride::SetPoint532CA0 | In outline: copies a point into its values (+0x1B4 on) |
| `0x5d0950` | TrackPiece::Init5D0950 | In outline: copies a list (List::CopyBD1800) and sets up a record with its table (`0xF3D9F4`) |
| `0x53b850` | Ride::Text53B850 | In outline: makes a text |
| `0x5be200` | TrackPiece::Set5BE200 | In outline: a step when on-ness (+0x2FC) changes |
| `0x5376c0` | Ride::Pass5376C0 | In outline: when not flagged (+0x21C), goes over entries |
| `0x4cd780` | Shop::Pass4CD780 | In outline: goes over a list (+0x64) |
| `0x4aa670` | Ride::Part4AA670 | In outline: a step with a part (+0xA4) |
| `0x5c5ee0` | TrackPiece::Step5C5EE0 | In outline: a step with cleared values (Zero6) |
| `0x58abb0` | Ride::Point58ABB0 | In outline: a step (`0x58A7A0`) and a point |
| `0x58aef0` | Ride::Last58AEF0 | In outline: a step (`0x58A7A0`) and the last 0x24-byte entry |
| `0x59db10` | Ride::Check59DB10 | In outline: a check when not flagged (+0x84) |
| `0x4c59f0` | Shop::Interface4C59F0 | In outline: a step with a value from an interface (Interface60::Get, +0xBC) |
| `0x501b50` | Rides::Init501B50 | In outline: sets up a record with its table (`0xF368B0`) |
| `0x563600` | Rides::Pass563600 | In outline: goes over entries (+0xC4) |
| `0x563560` | Rides::Pass563560 | In outline: goes over entries (+0xC4) |
| `0x5634c0` | Rides::Pass5634C0 | In outline: goes over entries (+0xC4) |
| `0x563420` | Rides::Pass563420 | In outline: goes over entries (+0xC4) |
| `0x5a46f0` | Rides::Pass5A46F0 | In outline: goes over entries (+0x6B0) |
| `0x5d6d40` | Rides::Check5D6D40 | In outline: a value when not flagged (+0x68) |
| `0x5d6ca0` | Rides::Check5D6CA0 | In outline: a value when not flagged (+0x68) |
| `0x5d6c00` | Rides::Check5D6C00 | In outline: a value when not flagged (+0x68) |
| `0x5d6a20` | Rides::Check5D6A20 | In outline: a value when not flagged (+0x68) |
| `0x5d6980` | Rides::Check5D6980 | In outline: a value when not flagged (+0x68) |
| `0x5d68e0` | Rides::Check5D68E0 | In outline: a value when not flagged (+0x68) |
| `0x4b9c20` | Rides::Make4B9C20 | In outline: makes a 0x10-byte part |
| `0x5137f0` | Rides::Handle5137F0 | In outline: makes a handle and links it |
| `0x53ca00` | Rides::Make53CA00 | In outline: makes a 0x10-byte part |
| `0x534ce0` | Rides::Update534CE0 | In outline: an update when not flagged (+0x21C) |
| `0x5a3eb0` | Rides::Check5A3EB0 | In outline: a value when not flagged (+0x378) |
| `0x5122f0` | Rides::Pass5122F0 | In outline: goes over entries (+0x314) |
| `0x4d4520` | Rides::Update4D4520 | In outline: an update when not flagged (+0x74) |
| `0x559140` | Rides::Update559140 | In outline: an update when not flagged (+0x34) |
| `0x5590a0` | Rides::Update5590A0 | In outline: an update when not flagged (+0x18) |
| `0x563bb0` | Rides::Check563BB0 | In outline: a value when not flagged (+0xF0) |
| `0x563920` | Rides::Check563920 | In outline: a value when not flagged (+0xF0) |
| `0x4e0c50` | Rides::Update4E0C50 | In outline: an update when not flagged (+0x4C) |
| `0x5d1f20` | Rides::Pass5D1F20 | In outline: goes over entries (+0x10) |
| `0x518490` | Rides::Pass518490 | In outline: goes over entries (+0x7B4) |
| `0x5c17f0` | Rides::Check5C17F0 | In outline: a value when not flagged (+0xAC) |
| `0x59f320` | Rides::Make59F320 | In outline: makes a 0xC-byte part |
| `0x5d6b60` | Rides::Pass5D6B60 | In outline: goes over entries (+0xF8) |
| `0x5daa80` | TrackedRide::Check5DAA80 | In outline: a step when a check fails (`0x5657D0`) |
| `0x584ce0` | Ride::Find584CE0 | In outline: a find, -1 when none (Offset138) |
| `0x5124b0` | Car::Step5124B0 | In outline: a step with many values |
| `0x4f6c10` | Ride::Cost4F6C10 | In outline: with a part (+0x614), the track editor's auto-complete cost (TrackEditor::AutoCompleteCost) |
| `0x5f2560` | Ride::Value5F2560 | In outline: a value with a large start (1e6) |
| `0x5d2020` | TrackPiece::Handle5D2020 | In outline: a step with a handle (`0xF3F6B8`) |
| `0x5bd670` | TrackPiece::Check5BD670 | In outline: unless its owner is flagged (bit 0x10 of +0x18), a step |
| `0x5446a0` | Window::Part5446A0 | In outline: a step (Window::Slot48Self) and acts with a part (+0x140) |
| `0x5d71c0` | TrackPiece::Find5D71C0 | In outline: a find in a list |
| `0x4cef50` | Shop::Copy4CEF50 | In outline: copies a record from another |
| `0x4cdc10` | Shop::List4CDC10 | In outline: a step with a list (+0x64) |
| `0x50b130` | Car::Count50B130 | In outline: with no entries (+0x2C) and n positive, a step with a list (+0x28) |
| `0x5d5690` | TrackPiece::Part5D5690 | In outline: with a part (+8), a step |
| `0x5d55f0` | TrackPiece::Part5D55F0 | In outline: the same (another copy) |
| `0x5cfc40` | TrackedRide::Byte5CFC40 | In outline: goes over a byte count (+0x30) |
| `0x4cd6e0` | Shop::Pass4CD6E0 | In outline: goes over a list (+0x64) |
| `0x5be020` | TrackPiece::Same5BE020 | In outline: whether two pieces have the same values (+0x64, +0x68) |
| `0x54c410` | Ride::List54C410 | In outline: a step with a list (+0x78) |
| `0x58ae50` | Ride::Point58AE50 | In outline: a step (`0x58A7A0`) and a point |
| `0x4ca910` | Shop::Part4CA910 | In outline: with a part (+0xD8 +0xC), a step |
| `0x596450` | Ride::Shared596450 | In outline: a step with the shared object's list (`0x12F4E4C`) |
| `0x591c20` | Ride::List591C20 | In outline: a step with a list (+0x208) |
| `0x500940` | Car::List500940 | In outline: a step with a list (+0x7C) |
| `0x5bbf10` | TrackPiece::Step5BBF10 | In outline: a step (`0x5BC5A0`) and a value (+0x1E0) |
| `0x5b5440` | TrackedRide::Step5B5440 | In outline: a step, when asked |
| `0x5a3e10` | TrackedRide::Check5A3E10 | In outline: checks (Ride::Check532BB0, Ride::Check4AA3E0) and acts |
| `0x58f740` | Ride::Count58F740 | In outline: a count from a part (+0xC, slot 0x10) |
| `0x5559c0` | Ride::Step5559C0 | In outline: a step with a part (`0x4A1800`) |
| `0x548f60` | Ride::Part548F60 | In outline: a step with a part (+0x4C) |
| `0x4efa70` | Ride::Mode4EFA70 | In outline: with a part (+0x7EC) in mode 4 (+0x144), a step |
| `0x549f20` | Ride::Position549F20 | In outline: a position (Ride::Position549C50) and acts with a part (+0x4C) |
| `0x548ea0` | Ride::Slot548EA0 | In outline: a slot (Ride::Slot548CF0) and acts with a part (+0x4C) |
| `0x57ac20` | Ride::Part57AC20 | In outline: a step with a part (+0x194) |
| `0x549b10` | Ride::Part549B10 | In outline: a step with a part (+0x4C) |
| `0x4cd910` | Shop::Find4CD910 | In outline: a find over a list (+0x64) |
| `0x518400` | Rides::Pass518400 | In outline: goes over entries (+0x7B4) |
| `0x531830` | Rides::Check531830 | In outline: a value when not flagged (+0x21C) |
| `0x4d45c0` | Rides::Check4D45C0 | In outline: a value when not flagged (+0x378) |
| `0x4ab750` | Rides::Check4AB750 | In outline: a value when not flagged (+0x3C4) |
| `0x59ad50` | Rides::Update59AD50 | In outline: an update when not flagged (+0x84) |
| `0x512250` | Rides::Pass512250 | In outline: goes over entries (+0x314) |
| `0x574950` | Rides::Update574950 | In outline: an update when not flagged (+0x5C) |
| `0x56f8a0` | Rides::Update56F8A0 | In outline: an update when not flagged (+0xF0) |
| `0x4cca70` | Rides::Pass4CCA70 | In outline: goes over entries (+0xC4) |
| `0x562f70` | Rides::Pass562F70 | In outline: goes over entries (+0xC4) |
| `0x562cf0` | Rides::Pass562CF0 | In outline: goes over entries (+0xC4) |
| `0x59da80` | Rides::Update59DA80 | In outline: an update when not flagged (+0x84) |
| `0x4e5910` | Rides::Make4E5910 | In outline: makes a 0x21C-byte part |
| `0x4e0bc0` | Rides::Update4E0BC0 | In outline: an update when not flagged (+0x4C) |
| `0x59b170` | Rides::Update59B170 | In outline: an update when not flagged (+0x84) |
| `0x59bcc0` | Ride::Set59BCC0 | In outline: for 0, sets a value (+0x64) |
| `0x573e60` | Ride::Cars573E60 | In outline: goes over a train's cars (RideTrainInstance::Car) |
| `0x563000` | Ride::Pass563000 | In outline: goes over entries (+0xC4) |
| `0x549000` | Ride::Part549000 | In outline: a step with a part (+0x4C) |
| `0x49aee2` | GameTime::Fragment49AEE2 | In outline: a fragment that takes the game time |
| `0x49ad52` | GameTime::Fragment49AD52 | In outline: the same (another copy) |
| `0x5f19c0` | Ride::Slot5F19C0 | In outline: for slot -2, a step with a value (+0x48) |
| `0x58df40` | Ride::Set58DF40 | In outline: a step (slot 0x20) and sets a value |
| `0x54dee0` | Ride::List54DEE0 | In outline: a step with a list |
| `0x4de6a0` | Ride::Part4DE6A0 | In outline: when given, a step with its part (slot 0x54) |
| `0x58de70` | Array::Grow58DE70 | In outline: grows a list to hold n |
| `0x569dc0` | Ride::Tool569DC0 | In outline: a step when the current tool's value (+0x108) is at most another (+0x6C) |
| `0x553c50` | Ride::Layout553C50 | In outline: lays out (Window::Layout673860) and acts with a point |
| `0x4c5ea0` | Shop::Mode4C5EA0 | In outline: in mode 1 (+0xC0), a step with an interface (Interface60::Get) |
| `0x4c5e10` | Shop::Mode4C5E10 | In outline: the same (another copy) |
| `0x591dd0` | Ride::Find591DD0 | In outline: for n not negative, a find over a list (+0x208) |
| `0x537b90` | Ride::Reset537B90 | In outline: copies a value (+0x118 into +0x11C) and resets (Ride::Reset4DE430) |
| `0x532930` | Ride::Entry532930 | In outline: 0 for negative n, else a step |
| `0x531ba0` | Ride::Entry531BA0 | In outline: the same (another copy) |
| `0x531a80` | Ride::Entry531A80 | In outline: the same (another copy) |
| `0x5e02c0` | TrackedRide::Check5E02C0 | In outline: a check (TrackedRide::Check5DAA80) with another ride |
| `0x5da9f0` | TrackedRide::Entry5DA9F0 | In outline: for n under a count (+0xE0), a step |
| `0x58a7a0` | Ride::Part58A7A0 | In outline: with a part (+8), a step with a work buffer |
| `0x54f910` | Ride::List54F910 | In outline: a step with a list (+0x78) |
| `0x54f880` | Ride::List54F880 | In outline: the same (another copy) |
| `0x5105c0` | Car::Loop5105C0 | In outline: a loop |
| `0x5c0f80` | TrackPiece::List5C0F80 | In outline: a step with a list (+0x1E4) |
| `0x591f40` | Ride::List591F40 | In outline: a step with a list (+0x220) |
| `0x591190` | Ride::Part591190 | In outline: a step with a part (+0x64) |
| `0x58d100` | Ride::Slot58D100 | In outline: a value for a slot, -1 handled |
| `0x531610` | Ride::Count531610 | In outline: a step when n reaches a count (slot 0x138) |
| `0x531580` | Ride::Count531580 | In outline: the same (another copy) |
| `0x50bea0` | Car::Dot50BEA0 | In outline: a dot product of two points, compared to a limit |
| `0x5b7270` | TrackedRide::Check5B7270 | In outline: a step when a check fails (`0xBB4190`) |
| `0x58bc50` | Ride::Point58BC50 | In outline: a point from the selected ride (Ride::Selected58BAB0) |
| `0x53dcc0` | Ride::Free53DCC0 | In outline: when given, tears down (Destructor) |
| `0x4eeb20` | Ride::List4EEB20 | In outline: when not flagged (+0x6C8), a step with a list (+0x4D8) |
| `0x5890f0` | Ride::Check5890F0 | In outline: false with no part (+0x104), else a check |
| `0x550360` | Ride::List550360 | In outline: a step with a list |
| `0x54b0b0` | Ride::List54B0B0 | In outline: the same (another copy) |
| `0x54b020` | Ride::List54B020 | In outline: the same (another copy) |
| `0x5ab300` | Ride::Free5AB300 | In outline: tears down references (ObjectRef) |
| `0x4c8dd0` | Shop::Layout4C8DD0 | In outline: lays out (Window::Layout643030) and acts with a point |
| `0x518300` | Car::Find518300 | In outline: a step when a find passes (`0x5121D0`) |
| `0x4afdd0` | Ride::Step4AFDD0 | In outline: a step with a value (`0x4A9430`) plus 3 |
| `0x4a35b0` | Rides::Release4A35B0 | In outline: takes one from a count (+4) and frees at zero |
| `0x4c9e90` | Rides::Pass4C9E90 | In outline: goes over entries (+0x17C) |
| `0x563da0` | Rides::Check563DA0 | In outline: a value when not flagged (+0x254) |
| `0x59bc40` | Rides::Update59BC40 | In outline: an update when not flagged (+0x44) |
| `0x59bbc0` | Rides::Update59BBC0 | In outline: an update when not flagged (+0x44) |
| `0x59bb40` | Rides::Update59BB40 | In outline: an update when not flagged (+0x44) |
| `0x5ebeb0` | Rides::Check5EBEB0 | In outline: a value when not flagged (+0x118) |
| `0x59b9d0` | Rides::Update59B9D0 | In outline: an update when not flagged (+0x84) |
| `0x5c1770` | Rides::Check5C1770 | In outline: a value when not flagged (+0xAC) |
| `0x5121d0` | Rides::Check5121D0 | In outline: a value when not flagged (+0x180) |
| `0x59b950` | Rides::Update59B950 | In outline: an update when not flagged (+0x84) |
| `0x59b0f0` | Rides::Update59B0F0 | In outline: an update when not flagged (+0x84) |
| `0x59abe0` | Rides::Update59ABE0 | In outline: an update when not flagged (+0x84) |
| `0x563b30` | Rides::Check563B30 | In outline: a value when not flagged (+0x254) |
| `0x5b9b20` | Rides::Set5B9B20 | In outline: sets a size when it changes |
| `0x5499a0` | Rides::Pass5499A0 | In outline: goes over entries (+0x5C) |
| `0x5a4830` | Rides::Update5A4830 | In outline: an update when not flagged (+0x338) |
| `0x591d40` | Ride::List591D40 | In outline: a step with a list (Ride::List591C20) |
| `0x56a0c0` | Ride::Slot56A0C0 | In outline: a step with a slot (Ride::Slot5637A0) |
| `0x54c380` | Ride::List54C380 | In outline: a step with a list (+0x48) |
| `0x4ce800` | Shop::List4CE800 | In outline: a step with a list (+0x64) |
| `0x5eecb0` | AttractionPerson::Step5EECB0 | In outline: a step with the selected ride (`0x12F5608`) |
| `0x5bb030` | TrackPiece::Tool5BB030 | In outline: a check (Ride::Check564550) and a tool step (`0x12F5338`) |
| `0x590500` | Ride::Part590500 | In outline: a step with a part (+0x2C) |
| `0x58fc90` | Ride::Step58FC90 | In outline: a step (`0xA30180`) and more |
| `0x552bc0` | Ride::Part552BC0 | In outline: a step with a part (+0x44) |
| `0x5ae6f0` | Ride::Slot5AE6F0 | In outline: for an object's slot (+0xBC) under a count (+0x3D4), a step |
| `0x559af0` | Ride::Part559AF0 | In outline: a step with a part (+0x54) |
| `0x543d80` | Ride::Part543D80 | In outline: with no part (+0xFC) and another (+0xF0), a step |
| `0x5000e0` | Car::Part5000E0 | In outline: with a part (+0x100), a step |
| `0x55be00` | Ride::Entry55BE00 | In outline: a value from an entry (+0x28 plus n) |
| `0x533590` | Ride::Entry533590 | In outline: the same (another copy) |
| `0x4ccfb0` | Shop::List4CCFB0 | In outline: a step with a list (+0x64) |
| `0x5924d0` | Ride::List5924D0 | In outline: goes over a list (+0x220) |
| `0x5613a0` | Ride::Time5613A0 | In outline: a time step (Ride::Time561180) with a work buffer |
| `0x50be20` | Car::Flag50BE20 | In outline: with a part (+0x404) and a shared flag (bit 2 of `0x1400304`), a step |
| `0x4ec920` | Ride::Error4EC920 | In outline: with a part (+0x18), an error text (PlaceObject::ErrorText) |
| `0x4acf80` | Ride::Mode4ACF80 | In outline: in mode 2 (+0x70), a step with a part (+0xCC) |
| `0x5beb30` | TrackPiece::Step5BEB30 | In outline: a step with cleared values (Zero6) |
| `0x4fe610` | Ride::Instances4FE610 | In outline: goes over a ride's instances (AttractionInstance::FirstOf) |
| `0x5eeee0` | AttractionPerson::Count5EEEE0 | In outline: adds one to a count (+0x120) and a step (`0x5EBEB0`) |
| `0x5d0640` | TrackPiece::Value5D0640 | In outline: a value from a part (+0x18 +8 +0x14) |
| `0x5d0420` | TrackPiece::Hit5D0420 | In outline: a hit test (Object::HitBC8110) with a clamped value |
| `0x5a8130` | TrackedRide::Entry5A8130 | In outline: with entries (+0x6F4 +8), a step for entry n (0x28 bytes) |
| `0x5657d0` | Ride::Check5657D0 | In outline: a check with an interface (`0x12F5544`) |
| `0x5606a0` | Ride::Value5606A0 | In outline: a value with a part (`0xBCD1F0`) of more than two |
| `0x50fe90` | Car::Array50FE90 | In outline: makes an array of n entries of 0x14 bytes |
| `0x5987a0` | Ride::Part5987A0 | In outline: with no value (+8) and a part (+0xC), a step |
| `0x580230` | Ride::Array580230 | In outline: a step with a small array (SmallArray) |
| `0x552c50` | Ride::Part552C50 | In outline: a step with a part (+0x44) |
| `0x5e9220` | AttractionPerson::Step5E9220 | In outline: a step with many values |
| `0x5ba7a0` | TrackPiece::Entry5BA7A0 | In outline: a value from an entry (+0x28 plus n) |
| `0x54bbb0` | Ride::Clear54BBB0 | In outline: a step (slot 0x28) and clears a flag (bit 0 of +0x48) |
| `0x54b3a0` | Ride::Clear54B3A0 | In outline: a step (`0x549A20`) and clears flags (bits 0-2 of +0x40) |
| `0x510f90` | Car::Check510F90 | In outline: when on (+0x250), a check with a point (+0x1A0) |
| `0x58ee00` | Ride::Clear58EE00 | In outline: empties an array (Array::Resize58EB20 with 0) |
| `0x4ee440` | Ride::Settings4EE440 | In outline: sets defaults (Settings5D::Defaults) and acts |
| `0x5eb0e0` | AttractionPerson::Step5EB0E0 | In outline: a step with an info's ride and person (AttractionPersonInfo) |
| `0x59d8c0` | Ride::Interface59D8C0 | In outline: a step with an interface's part (Interface54b::Get +0x6F4) |
| `0x4e2c40` | Ride::Step4E2C40 | In outline: a step with many values |
| `0x4cd660` | Shop::Find4CD660 | In outline: a find over a list (+0x64) |
| `0x4adbd0` | Rides::Entry4ADBD0 | In outline: a value from an entry (+0x28 plus n) |
| `0x4ad9c0` | Rides::Entry4AD9C0 | In outline: a value from an entry (+0x28 plus n) |
| `0x510910` | Rides::Make510910 | In outline: makes a 0x10-byte part |
| `0x5a6b50` | Rides::Update5A6B50 | In outline: an update when not flagged (+0x378) |
| `0x549a20` | Rides::Pass549A20 | In outline: goes over entries (+0x5C) |
| `0x4c52c0` | Rides::Check4C52C0 | In outline: a value when not flagged (+0x378) |
| `0x5ae3e0` | Rides::Update5AE3E0 | In outline: an update when not flagged (+0x378) |
| `0x5cc740` | Rides::Update5CC740 | In outline: an update when not flagged (+0x214) |
| `0x59bf20` | Rides::Update59BF20 | In outline: an update when not flagged (+0x44) |
| `0x5a1530` | Rides::Make5A1530 | In outline: makes a 0x344-byte part |
| `0x4a1670` | Rides::Assign4A1670 | In outline: takes a reference to another's part when it differs |
| `0x5f0600` | Rides::Update5F0600 | In outline: an update when not flagged (+0x58) |
| `0x5d1df0` | Rides::Make5D1DF0 | In outline: makes a 0x2C-byte part |
| `0x5a4f80` | Rides::Update5A4F80 | In outline: an update when not flagged (+0x378) |
| `0x55c0f0` | Rides::Pass55C0F0 | In outline: goes over entries (+0x44) |
| `0x549aa0` | Rides::Pass549AA0 | In outline: goes over entries (+0x5C) |
| `0x5a7dc0` | Rides::Update5A7DC0 | In outline: an update when not flagged (+0x378) |
| `0x4ca030` | Shop::Value4CA030 | In outline: a step with a value (+0x10) |
| `0x5d9720` | TrackedRide::Selected5D9720 | In outline: a step with the selected ride (`0x12F5608`) |
| `0x5d6860` | TrackPiece::Entry5D6860 | In outline: 0 for n at or past a count (+0x100), else a value |
| `0x5d67e0` | TrackPiece::Entry5D67E0 | In outline: the same with +0xE0 |
| `0x5a1bd0` | TrackedRide::Kind5A1BD0 | In outline: a value unless of kind 10 or 0xF (+0x38C) |
| `0x5779f0` | Ride::Step5779F0 | In outline: a step with a work buffer |
| `0x4f66f0` | Ride::Mode4F66F0 | In outline: in mode 1, 3 or 4 (+0x144), a step |
| `0x4f6670` | Ride::Mode4F6670 | In outline: the same (another copy) |
| `0x4f65f0` | Ride::Mode4F65F0 | In outline: the same (another copy) |
| `0x5fb1a0` | Window::Skin5FB1A0 | In outline: when a part is on (+0x134 +0x3C), a skin texture (SkinItem::Texture) |
| `0x595f70` | Ride::List595F70 | In outline: a step with a list (Ride::List591C20) |
| `0x561b80` | Ride::Kind561B80 | In outline: for kind 0x46 (+0x60), a time step (Ride::Time561180) |
| `0x561550` | Ride::Kind561550 | In outline: for kind 0xD with a time, a step |
| `0x530f40` | Ride::Part530F40 | In outline: a step with a part (+0x51C) |
| `0x52f980` | Car::Case52F980 | In outline: a value by case |
| `0x524220` | Car::Step524220 | In outline: a step with its owner's value (slot 0x94) |
| `0x5113f0` | Car::Step5113F0 | In outline: when on (+0x924), a step (`0x50E630` with +0x90C) |
| `0x4c4c00` | Shop::Random4C4C00 | Sets a value (+0x90) to a random value between 1 and 2 (Random::Ran0Range) and more |
| `0x5cf730` | TrackedRide::Step5CF730 | In outline: a step with many values |
| `0x512390` | Car::Interface512390 | In outline: a step with interfaces (`0x12F537C`, Interface48b::Get) |
| `0x4c2370` | Shop::Step4C2370 | In outline: a step (`0x49BC60`) |
| `0x5f2d10` | Ride::Grow5F2D10 | In outline: grows a list when n passes its count (ObjectArray::SetCapacity) |
| `0x4ceed0` | Shop::List4CEED0 | In outline: a step with a list (+0x64) |
| `0x4cc090` | Name::Find4CC090 | In outline: for none, a find (Name::Find4CBAA0) |
| `0x598880` | Ride::Part598880 | In outline: a step (Window::Slot48Self) and acts with a part (+0x144) |
| `0x598620` | Ride::Part598620 | In outline: with a part's part (+0x1C +0x198), a step |
| `0x4cd590` | Shop::Find4CD590 | In outline: a find over a list (+0x64) |
| `0x507af0` | Car::Step507AF0 | In outline: a step with an identity-like start (1.0, 0, ...) |
| `0x59eca0` | Ride::Range59ECA0 | In outline: a step for a range, an empty one handled |
| `0x579c50` | Ride::Set579C50 | In outline: with a part (+4), a step when a flag (+0x48) changes |
| `0x52cbf0` | Car::Value52CBF0 | In outline: a step when a value from an interface (`0x12F50D4`) is not negative |
| `0x5f4850` | Ride::Part5F4850 | In outline: for its own part (+0x40) and kind 0x400, a step |
| `0x50e630` | Car::Pass50E630 | In outline: goes over entries (+4) |
| `0x4f3c40` | Ride::Value4F3C40 | In outline: a step with a value (+0x80C) |
| `0x4ab8d0` | Ride::List4AB8D0 | In outline: a step with a value (+0x370) and a list (+0xCC) |
| `0x5dbc60` | TrackedRide::Check5DBC60 | In outline: a step when a check fails (TrackedRide::Check5DBBF0) |
| `0x594830` | Ride::Clear594830 | In outline: empties a list (+0x208) |
| `0x518390` | Car::Find518390 | In outline: a step when a find fails (Car::Find518300) |
| `0x4a8a00` | Ride::Owner4A8A00 | In outline: a value from an owner (OffsetRef::Owner4, +0xC) |
| `0x5c20e0` | TrackPiece::List5C20E0 | In outline: a step with a list (+0x338) |
| `0x5bb0c0` | TrackPiece::Count5BB0C0 | In outline: when on (+0x98), a step with a count (+0x9C) plus one |
| `0x59ab70` | Ride::List59AB70 | In outline: a step with a list (+0xB8) |
| `0x5310a0` | Ride::List5310A0 | In outline: a step with a list (+0x29C) |
| `0x4c82f0` | Shop::Kind4C82F0 | In outline: for a part of kind 3 or flagged (0x40), a step |
| `0x4bb920` | Ride::Part4BB920 | In outline: clears a flag (+0x2E8) and acts with a part (+0x2E0) |
| `0x5a1ad0` | TrackedRide::Owner5A1AD0 | In outline: a step with an owner (OffsetRef::Owner3) and a value (+0x6F4) |
| `0x5859f0` | Ride::Owner5859F0 | In outline: a value when an owner (OffsetRef::Owner5) is of kind 0xB |
| `0x5aa7a0` | TrackedRide::Entry5AA7A0 | In outline: a step for entry n (0x28 bytes) not on |
| `0x561330` | Ride::Value561330 | In outline: a value when a check passes (`0x12F5544`) |
| `0x4ade80` | Ride::Step4ADE80 | In outline: a step (`0x4A36A0`) and more |
| `0x4a8990` | Ride::Owner4A8990 | In outline: a value from an owner (OffsetRef::Owner4, +0x10) |
| `0x5f0590` | Rides::Update5F0590 | In outline: an update when not flagged (+0x58) |
| `0x4b4740` | Rides::Init4B4740 | In outline: sets up a record with its table (`0xF270F4`) |
| `0x5d7730` | Rides::Assign5D7730 | In outline: frees its buffer and remakes it from another |
| `0x5d4470` | Rides::Assign5D4470 | In outline: frees its buffer and remakes it from another |
| `0x5d4350` | Rides::Assign5D4350 | In outline: frees its buffer and remakes it from another |
| `0x5d74d0` | Rides::Assign5D74D0 | In outline: frees its buffer and remakes it from another |
| `0x5d7370` | Rides::Assign5D7370 | In outline: frees its buffer and remakes it from another |
| `0x5d1cd0` | Rides::Pass5D1CD0 | In outline: goes over entries (+0x10) |
| `0x5c95e0` | Rides::List5C95E0 | In outline: a step with a list (+0x338) |
| `0x564480` | Rides::Update564480 | In outline: an update when not flagged (+0xF0) |
| `0x5329c0` | Rides::Update5329C0 | In outline: an update when not flagged (+0x21C) |
| `0x580040` | Rides::Pass580040 | In outline: goes over entries (+0xFC) |
| `0x55bc70` | Rides::Pass55BC70 | In outline: goes over entries (+0x44) |
| `0x5efd60` | Rides::Update5EFD60 | In outline: an update when not flagged (+0x58) |
| `0x4fe500` | Rides::Terrain4FE500 | In outline: a step when the terrain is not flagged |
| `0x4f1df0` | Rides::Clear4F1DF0 | In outline: empties a list (+0x7C8) |
| `0x58e950` | Rides::Pass58E950 | In outline: goes over entries (+0x8) |
| `0x533dc0` | Rides::Free533DC0 | In outline: tears down references (ObjectRef) |
| `0x5d0d60` | Rides::Make5D0D60 | In outline: makes a 0xC-byte part |
| `0x4a8fd0` | Rides::Assign4A8FD0 | In outline: frees its buffer and remakes it from another |
| `0x57a110` | Ride::Random57A110 | Sets a value (+0x14) from a random value between 0.8 and 1 (Random::Ran0Range) |
| `0x4ce790` | Shop::List4CE790 | In outline: a step with a list (+0x64) |
| `0x52ee10` | Car::Step52EE10 | In outline: a step with many values |
| `0x59dbb0` | Ride::Link59DBB0 | In outline: when a link check passes (TrackPiece::Link5E6A40), a step |
| `0x5d5550` | TrackPiece::Flag5D5550 | In outline: 1 unless flagged (bit 2 of +0x18) |
| `0x5beac0` | TrackPiece::Step5BEAC0 | In outline: a step with its owner (+0x40, slot 0x24) |
| `0x59f3f0` | Ride::Loop59F3F0 | In outline: goes over a list (+0x58) |
| `0x584aa0` | Ride::Case584AA0 | In outline: a step by a byte's case (7, ...) |
| `0x5bc5a0` | TrackPiece::Reset5BC5A0 | In outline: clears values (+0x4C, +0x50) |
| `0x4ce0e0` | Shop::Slot4CE0E0 | In outline: a step when a slot check fails (Shop::Slot4CDF90) |
| `0x4eea00` | Ride::Slot4EEA00 | In outline: when n matches a value (+0x4A4) and not flagged (+0x538), a step |
| `0x5643a0` | Ride::Case5643A0 | In outline: a value by case (9, ...) |
| `0x4cdcb0` | Shop::List4CDCB0 | In outline: a step with a list (+0x64) |
| `0x5d0820` | TrackPiece::Part5D0820 | In outline: with a part, a step |
| `0x56a150` | Ride::Time56A150 | In outline: a step with the selected ride and a time difference (+0x68, +0x6C) |
| `0x548c40` | Ride::Part548C40 | In outline: a step with a part (+0x4C) |
| `0x52f780` | Car::Make52F780 | In outline: makes a part with a value (+0x298) |
| `0x4cdb30` | Shop::List4CDB30 | In outline: a step with a list (+0x64) |
| `0x4cd250` | Shop::List4CD250 | In outline: the same (another copy) |
| `0x4b5ae0` | Ride::Check4B5AE0 | In outline: a step when a check fails (Ride::Check4B51E0) |
| `0x4b5a80` | Ride::Check4B5A80 | In outline: the same (another copy) |
| `0x5a1920` | TrackedRide::Shared5A1920 | In outline: a value by shared flags (`0x12F554C`, `0x10EDC88`) |
| `0x5f4dd0` | Ride::Part5F4DD0 | In outline: with a part (+0x13C), a step |
| `0x5d0720` | TrackPiece::Part5D0720 | In outline: with a part, a step |
| `0x584de0` | Ride::Find584DE0 | In outline: a step when a find passes (Offset138) |
| `0x581670` | Ride::Pass581670 | In outline: goes over entries (+4) |
| `0x543ea0` | Ride::Value543EA0 | In outline: a step with two values (`0x4A94B0`, +0x414) |
| `0x4e0f20` | Ride::Free4E0F20 | In outline: tears down (`0x501F80`) and frees |
| `0x4cce20` | Shop::Part4CCE20 | In outline: with an object, a step with its value (+4) |
| `0x5f8400` | Ride::Set5F8400 | In outline: sets a value (+0x4C) and acts with a part (+0x48 +0x28) |
| `0x4bf020` | Ride::Slot4BF020 | In outline: with a slot (slot 0x84), a step |
| `0x4b76f0` | Ride::Find4B76F0 | In outline: a find (Shop::Find4CA6E0) with values (+0x140 on) |
| `0x5d4070` | TrackPiece::Check5D4070 | In outline: when a check passes (`0x5CFB10`) with no part (+8), a step |
| `0x581600` | Ride::Array581600 | In outline: makes an aligned array of n entries of 12 bytes |
| `0x534f80` | Ride::Sum534F80 | In outline: a total over entries |
| `0x4ea320` | Ride::Count4EA320 | In outline: adds one to a count (+0x120) and acts |
| `0x5c14d0` | TrackPiece::Value5C14D0 | In outline: a value from its owner (+0x40, slot 0x94) |
| `0x50d2b0` | Car::Time50D2B0 | In outline: for n not negative and not flagged (+0x280), a step |
| `0x501a30` | Car::Reset501A30 | In outline: resets (Car::Reset500860) and more |
| `0x4b7090` | Ride::Free4B7090 | In outline: tears down (`0x501F80`) and frees |
| `0x5cfd40` | TrackedRide::Point5CFD40 | In outline: a step with a point (`0x5CFCE0`) |
| `0x5019d0` | Car::Init5019D0 | In outline: clears a value and points a list at its own storage (+0x88) |
| `0x4befc0` | Ride::Shared4BEFC0 | In outline: with no shared value (`0x1305DD8`), a step (`0x6C1DA0`) |
| `0x4bbc80` | Ride::Set4BBC80 | In outline: sets a flag (+0x390) and another to 1 (+0x378) |
| `0x4a4950` | Ride::Shared4A4950 | In outline: a step with a shared table (`0x10EE270`) |
| `0x5f7690` | Ride::Current5F7690 | In outline: when not the current object (`0x12F56D4`), a step |
| `0x5826d0` | Ride::Value5826D0 | In outline: a value from a part (+0x1B8, slot 0x6C) with another (+0x210) |
| `0x667550` | Window::Id667550 | In outline: for an id in a range from 0x5B3280B, a step with a part (+0xD8) |
| `0x5e64f0` | Rides::Pass5E64F0 | In outline: goes over entries (+0x114) |
| `0x510710` | Rides::Free510710 | In outline: tears down references (ObjectRef) |
| `0x4cee70` | Rides::List4CEE70 | In outline: a step with a list (+0x64) |
| `0x4c4220` | Rides::Free4C4220 | In outline: tears down references (ObjectRef) |
| `0x4ccf00` | Rides::List4CCF00 | In outline: a step with a list (+0x64) |
| `0x4cc830` | Rides::Pass4CC830 | In outline: goes over entries (+0x9C) |
| `0x50edb0` | Rides::Pass50EDB0 | In outline: goes over entries (+0x8C0) |
| `0x59fa20` | Rides::Clear59FA20 | In outline: empties a list (+0x6C) |
| `0x4c1740` | Rides::List4C1740 | In outline: a step with a list (+0xCC) |
| `0x5f3d70` | Ride::Release5F3D70 | In outline: tells two parts (+0x110, +0x118) with their values |
| `0x5d3ff0` | TrackPiece::Check5D3FF0 | In outline: for a shared record (`0xF3F73C`), a step, none handled |
| `0x5bf360` | TrackPiece::Copy5BF360 | In outline: copies a flag (+0x234) and an array (Array4::Assign) |
| `0x5b5b60` | TrackedRide::Step5B5B60 | In outline: steps (Thunk, `0x531200`) |
| `0x5b5b00` | TrackedRide::Step5B5B00 | In outline: the same (another copy) |
| `0x54f610` | Ride::Part54F610 | In outline: a part step (Ride::Part54C550 with +0x60 and shared values) |
| `0x501bf0` | Car::Case501BF0 | In outline: a step by case (+0xB0) |
| `0x4de100` | Ride::Check4DE100 | In outline: a step when a check fails (slot 0x3C) |
| `0x4cc470` | Shop::Kind4CC470 | In outline: for none, a car kind (Car::Kind50E6D0) |
| `0x5d6e80` | TrackedRide::Track5D6E80 | In outline: a step with the tracked ride's track interface |
| `0x5936a0` | Ride::Part5936A0 | In outline: a step with a part (+0x44) |
| `0x57abc0` | Ride::Shared57ABC0 | In outline: sets a shared flag (`0x12F53A8`) and a step with a part (+0x158, slot 0xF8) |
| `0x565850` | Ride::Check565850 | In outline: a step when a check fails (Ride::Check5657D0) |
| `0x530150` | Ride::Compare530150 | In outline: compares a text (+0x220) (WideString::Compare) |
| `0x5cfce0` | TrackedRide::Point5CFCE0 | In outline: a step with a clamped value (Clamp0To3) |
| `0x59da20` | Ride::Interface59DA20 | In outline: a step with an interface's part (Interface54b::Get +0x6F4) |
| `0x4d65a0` | Shop::Point4D65A0 | In outline: with a part (+0x40), sets a point (Ride::SetPoint532CA0) |
| `0x593fa0` | Ride::Free593FA0 | In outline: sets its table (`0xF3CF10`), tears down a list and frees when asked |
| `0x52f7f0` | Car::Slot52F7F0 | In outline: a value (slot 0x74), negatives handled |
| `0x4f0c10` | Ride::Pass4F0C10 | In outline: with a part (+0x80C), goes over entries |
| `0x4cd030` | Shop::Count4CD030 | In outline: a count from a part (+0x74 +4) |
| `0x4ccf50` | Shop::Count4CCF50 | In outline: the same (another copy) |
| `0x4c8ec0` | Shop::Point4C8EC0 | In outline: a step with a point |
| `0x5a1b70` | TrackedRide::Owner5A1B70 | In outline: a value with owners (OffsetRef::Owner3, OffsetRef::Owner2) |
| `0x58eda0` | Ride::Init58EDA0 | In outline: clears a record |
| `0x4c4c80` | Shop::Random4C4C80 | Sets a value (+0x90) to a random value between 1 and 2 (Random::Ran0Range) and more |
| `0x5b04e0` | TrackedRide::Check5B04E0 | In outline: a step when a check fails (Rides::Check5A3EB0) |
| `0x5a2c20` | TrackedRide::Handle5A2C20 | In outline: a step with a handle (`0xF30A7C`) |
| `0x58de20` | Ride::Assign58DE20 | In outline: frees its buffer (+8) and takes another's values |
| `0x58a830` | Ride::Init58A830 | In outline: sets up a record with its table (`0xF3CC90`) and a value |
| `0x57bbf0` | Ride::Assign57BBF0 | In outline: when a part (+0x164) differs, a step |
| `0x4ef940` | Ride::List4EF940 | In outline: a step with a list (+0x7C8) |
| `0x597c30` | Ride::Part597C30 | In outline: with no part (+0x154), a step |
| `0x577a70` | Ride::Cars577A70 | In outline: goes over a train's cars (RideTrainInstance::Car) |
| `0x54bab0` | Ride::Clear54BAB0 | In outline: when flagged (bit 2 of +0x40), clears a value (+0x74) |
| `0x54b740` | Ride::Clear54B740 | In outline: the same with bit 3 |
| `0x548a90` | Ride::Flip548A90 | Negates two values (+0, +0x28) and more |
| `0x510150` | Car::Array510150 | In outline: makes an aligned array of n entries of 64 bytes |
| `0x4bf0b0` | Ride::Shared4BF0B0 | In outline: with no shared count (`0x1106C54`), a step |
| `0x4a9430` | Ride::Flag4A9430 | In outline: when a part is flagged (bit 4 of +0xA8 +0x30), a value |
| `0x5e69f0` | AttractionPerson::Step5E69F0 | In outline: a step with an object's value (+0x1B8) and a check (TrackPiece::Check5C0120) |
| `0x5a0ab0` | TrackedRide::Width5A0AB0 | In outline: a step with a width |
| `0x59a500` | Ride::Shared59A500 | In outline: with a shared value (`0x12F54C0`), a step with a shared table (`0x10EE9B4`) |
| `0x5d20c0` | TrackPiece::Step5D20C0 | In outline: a step with a handle |
| `0x5bad40` | TrackPiece::Reset5BAD40 | In outline: clears values (+0x84, +4, ...) |
| `0x5b56c0` | TrackedRide::Set5B56C0 | In outline: sets a flag (+0x861) when it changes |
| `0x581490` | Ride::Pass581490 | In outline: goes over entries with one less a value |
| `0x50af10` | Car::Mode50AF10 | In outline: in mode 3 (+0x1C8), a step |
| `0x4edc80` | Ride::Step4EDC80 | In outline: when given, a step |
| `0x576ba0` | Ride::Init576BA0 | In outline: clears a flag and acts with a part (+0x24) |
| `0x513160` | Car::Init513160 | In outline: sets up a handle pointing at itself |
| `0x4cd600` | Shop::Count4CD600 | In outline: a count over a list (+0x64) |
| `0x5fa1b0` | Window::Case5FA1B0 | In outline: a step by case (+0xF0: 0, ...) |
| `0x5f84c0` | Ride::Part5F84C0 | In outline: with a part (+0x24), a step |
| `0x5d7d20` | TrackPiece::Selected5D7D20 | In outline: when not asked, a step with the selected ride (`0x12F5608`) |
| `0x5bd620` | TrackPiece::Part5BD620 | In outline: with a part (+0x31C), a list step (`0xAFD9D0`) |
| `0x5b75d0` | TrackedRide::Count5B75D0 | In outline: a step with counts (+0x6F4 +8, +0x3CC) |
| `0x5b0460` | TrackedRide::Make5B0460 | In outline: with a part (+0x700) and a different value (+0x3D8), a step (TrackedRide::Make5AE990) |
| `0x58c7b0` | Ride::Mode58C7B0 | In outline: sets a mode to 3 (+0x100) and a step (slot 0x58) |
| `0x585aa0` | Ride::Count585AA0 | Adds one to a count (+0x90) and n to a total (+0x1F4) |
| `0x4ee8f0` | Ride::Shared4EE8F0 | In outline: with a shared value (`0x10EDC98`), a step |
| `0x4e1120` | Ride::Part4E1120 | In outline: with a part (+0x164), a pass (Bird::PassA06560) |
| `0x4cdae0` | Shop::Find4CDAE0 | In outline: a find over a list (+0x64) |
| `0x4c4e30` | Shop::Random4C4E30 | In outline: for a range, a random value in it (Random::Ran0Range) |
| `0x5528b0` | Ride::Case5528B0 | In outline: with no part (+0xF4), a step by case (+0xB0) |
| `0x548bf0` | Ride::Part548BF0 | In outline: a step with a part (+0x4C) |
| `0x52f730` | Car::Part52F730 | In outline: with a part (+0x258), a step with the largest float |
| `0x5f48f0` | Ride::Part5F48F0 | In outline: with no part (+0x54), a step |
| `0x5d24e0` | TrackPiece::Handle5D24E0 | In outline: a step with a handle (`0xF3F6DC`) |
| `0x58cd40` | Ride::Near58CD40 | In outline: whether two points are within a distance |
| `0x4bf3f0` | Ride::Find4BF3F0 | In outline: -1 for none, else a find |
| `0x5d1fd0` | TrackPiece::Step5D1FD0 | In outline: a step with a handle |
| `0x576b00` | Ride::Value576B00 | In outline: one less a value (+8) when at least a half |
| `0x4f68b0` | Ride::Mode4F68B0 | In outline: in mode 1, 3 or 4 (+0x144), a step |
| `0x5dbba0` | TrackedRide::Diff5DBBA0 | In outline: a step with the difference of two values |
| `0x5d1b40` | TrackPiece::Count5D1B40 | In outline: a count by bits |
| `0x540570` | Ride::Step540570 | In outline: a step with values |
