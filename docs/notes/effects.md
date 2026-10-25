# Effects



| Address | Name | What it does |
|---|---|---|
| `0x920910` | Waterfall::Update | Runs the waterfalls: the falling water (gravity -9.8) and its particles, and the `WaterfallSound`. Called from `0x9225A0` / `0x9225B0` |
| `0x94f250` | WaterParticles::Update | Moves a set of water particles (slot 0 of `0xF7E438`): random spray directions, gravity (-9.81), and a switch over the kind of particle |
| `0x923f60` | Waterfall::CreationFailureText | Explains why a waterfall can't be made: the same body of water at both ends, the bounding box, an edge too narrow, the water too far apart, the height difference too small, land not owned, ... (`Waterfall_CreationFailure_*`), as message lines. Called from five places in the waterfall code |
| `0x9e3640` | RainManager::DebugText | Writes the rain's developer statistics (`RainManager:`): rate, velocity, numbers of emitters and splash emitters, camera position, particles wrapped, rendered and not rendered, rain particles. Called from `0x9D30E0` |
| `0x51a6b0` | RideCar::SetupWaterDrops | Sets up the water dripping from a ride car (`DropWater`, with the sound `DropWaterLoopFader`): its meta-particles (`MetaParticle`). Called from `0x5242A0` and `0x526A70` |
| `0x831d20` | SceneryItem::SetupEffects | Sets up a scenery item's effects from its model's bones: particle effects (`particlestart%d` / `particleend%d`, loaded from `ParticleEffects\*.frw`) and firework launchers (`fireworkstart%d` / `fireworkend%d`), and the vendor of a stall (`Vendor`, complaining `Vendor's matrix has gone AWOL`). Called from `0x832C30` and `0x8335E0` |
