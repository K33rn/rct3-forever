# Rides

Tracked rides, flat rides and their cars and trains.

| Address | Name | What it does |
|---|---|---|
| `0x51ee60` | RideCar::FindBones | Sets up a ride car from its model: looks up its bones by name (`Pivot`, `CarFront` / `CarRear`, the cameras `CameraFront`, `CameraRear`, `NoseCamera`, `BombCameraMid`, `Tank`, `LinkFront` / `LinkRear`, axles `AxleF` / `AxleR` (also spelt `AxelF` / `AxelR`), wheels `WheelFL`, `WheelFR`, `WheelRL`, `WheelRR`, `WheelL`, `WheelR`) and the restraint animations, and reports models that break the rules (`missing bone Pivot for car`, `Axles not allowed for link car:`, `Seperate wheels not allowed for link car:`, `missing .shp/.shs for LOD`, `Unexpected number of restraint anims`) |
| `0x5dc5b0` | MiniGolf::SetupAnimations | Sets up a mini golf course's player animations: walking on, walking off and looping (`WalkOn`, `WalkOff`, `Loop`, `Walk`), each hole's strokes (`%02dStroke%02d`) and the ball. Called from `0x59AD50` |
