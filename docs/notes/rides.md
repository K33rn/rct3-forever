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
