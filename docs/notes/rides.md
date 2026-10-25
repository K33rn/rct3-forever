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
