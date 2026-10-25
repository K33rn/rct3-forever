# Rides

Tracked rides, flat rides and their cars and trains.

| Address | Name | What it does |
|---|---|---|
| `0x51ee60` | RideCar::FindBones | Sets up a ride car from its model: looks up its bones by name (`Pivot`, `CarFront` / `CarRear`, the cameras `CameraFront`, `CameraRear`, `NoseCamera`, `BombCameraMid`, `Tank`, `LinkFront` / `LinkRear`, axles `AxleF` / `AxleR` (also spelt `AxelF` / `AxelR`), wheels `WheelFL`, `WheelFR`, `WheelRL`, `WheelRR`, `WheelL`, `WheelR`) and the restraint animations, and reports models that break the rules (`missing bone Pivot for car`, `Axles not allowed for link car:`, `Seperate wheels not allowed for link car:`, `missing .shp/.shs for LOD`, `Unexpected number of restraint anims`) |
| `0x5dc5b0` | MiniGolf::SetupAnimations | Sets up a mini golf course's player animations: walking on, walking off and looping (`WalkOn`, `WalkOff`, `Loop`, `Walk`), each hole's strokes (`%02dStroke%02d`) and the ball. Called from `0x59AD50` |
| `0x529ca0` | RideCarSpinningPart::Update | Moves a spinning part of a ride car (virtual slot 12 of RideCarSpinningPart): its angle and angular speed from the car's motion and gravity (-9.81), with friction and the angle kept within a full turn (2 pi) |
| `0x570bd0` | RideTrainInstance::Update | A train's update on its track (virtual slot 16 of RideTrainInstance): moves its cars, plays the guests' screams (`TrackedRideGuestScream`), splashes (`SplashSound`), boosters (`Boost`) and locking to a spiral lift hill (`LockToSpiralLiftHill`) |
| `0x4d8310` | SavedSelection::Save | Saves a track or pool design to the player's files: coasters to `\Coasters\` / `Coaster Designs\` as `.trk` (numbering the name `(%d)` if it exists, with the scenery `CoasterSceneryItems`), pools to `\Pools\` / `Pool Designs\` as `.pool`, and reports success (`TrackDesignSavedSuccessfully`, `PoolComplexSavedSuccessfully`). Slot 69 of `0xF33D30` |
