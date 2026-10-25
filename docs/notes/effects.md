# Effects



| Address | Name | What it does |
|---|---|---|
| `0x920910` | Waterfall::Update | Runs the waterfalls: the falling water (gravity -9.8) and its particles, and the `WaterfallSound`. Called from `0x9225A0` / `0x9225B0` |
| `0x94f250` | WaterParticles::Update | Moves a set of water particles (slot 0 of `0xF7E438`): random spray directions, gravity (-9.81), and a switch over the kind of particle |
| `0x923f60` | Waterfall::CreationFailureText | Explains why a waterfall can't be made: the same body of water at both ends, the bounding box, an edge too narrow, the water too far apart, the height difference too small, land not owned, ... (`Waterfall_CreationFailure_*`), as message lines. Called from five places in the waterfall code |
| `0x9e3640` | RainManager::DebugText | Writes the rain's developer statistics (`RainManager:`): rate, velocity, numbers of emitters and splash emitters, camera position, particles wrapped, rendered and not rendered, rain particles. Called from `0x9D30E0` |
| `0x51a6b0` | RideCar::SetupWaterDrops | Sets up the water dripping from a ride car (`DropWater`, with the sound `DropWaterLoopFader`): its meta-particles (`MetaParticle`). Called from `0x5242A0` and `0x526A70` |
| `0x831d20` | SceneryItem::SetupEffects | Sets up a scenery item's effects from its model's bones: particle effects (`particlestart%d` / `particleend%d`, loaded from `ParticleEffects\*.frw`) and firework launchers (`fireworkstart%d` / `fireworkend%d`), and the vendor of a stall (`Vendor`, complaining `Vendor's matrix has gone AWOL`). Called from `0x832C30` and `0x8335E0` |
| `0x82d7f0` | FireworkLauncher::FindSlots | Finds a firework launcher's slots in its model (`Slot %d in %s`), reporting `Missing firework slot matrix`. Called from `0x82F710` and `0x989440` |
| `0x9ea840` | Weather::Thunder | Plays thunder (`Thunder1`, `Thunder2`). Called from `0x9EAF50` and `0x9EAFA0` |
| `0x999ac0` | FireworkSlot::Highlight | Draws the highlight of a firework launcher's slot (`FireworkSlotHighlight`, `SlotVectorLocus`). Called from `0x99E160` |
| `0x929700` | WaterfallManager::DebugText | Writes the waterfalls' developer statistics and render modes (`WaterfallManager:`, `Num waterfalls: %d`, `RenderMode_Normal`, `_WireFrame`, `_Rasterization`, ...). Called from `0x92C990` |
| `0x93c6f0` | WaterJetManager::DebugText | Writes the water jets' developer statistics (`WaterJetManager:`, `Wibble: %s`, `Num WaterJets: %d`, render modes, particle kinds). Called from `0x93CA60` |
| `0x98a870` | FireworkDisplay::Check | Checks a firework show's attraction (`Error: Show has invalid attraction`) and its default name (`FireworkDisplayDefaultName`). Called from two places |
| `0x82ddb0` | FireworkLauncher::FindSlots (2) | Finds a launcher's slots (`Slot %d in %s`, `Missing firework slot matrix`), for the display editor. Called from three places |
| `0xa280a0` | Darts::Effects | The effects of a dart hitting: dust (`ParticleEffects\Dust.frw`), a gun shot and a balloon pop (`GunShot1`, `BalloonPop`). Called from three places |
| `0x825780` | FountainShape::Names | Recognises the special fountain display names `shark`, `truck` and `doom` |
| `0x939870` | WaterJet::DebugText | Writes a water jet's developer statistics (`Speed`, `DistForSpeed`, numbers of points and emitters, start, end and ID). Called from `WaterJetManager::DebugText` |
| `0xede240` | FountainKinds::Names | Makes the fountain kinds' names at start-up (`splash`, `jet`, `fountain`) |
