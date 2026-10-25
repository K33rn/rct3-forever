# Animals



| Address | Name | What it does |
|---|---|---|
| `0xb67c50` | WildAnimalLocomotorMoveTo::Update | Moves a wild animal towards its destination (virtual slot 11 of WildAnimalLocomotorMoveTo): steering and speed along its path, with a switch over the movement state |
| `0xb277a0` | WAL_RunAway::Update | A wild animal running away (virtual slot 11 of WAL_RunAway): finds where to flee to from what frightens it and moves it there |
| `0xa65730` | Buggy::Setup | Sets up the buggy vehicle (`ACAM\Buggy`): its hull and four wheels (`wheelfl`, `wheelfr`, `wheelbl`, `wheelbr`) and its engine sounds (`BuggyIdle`, `BuggyRev`, `BuggyGear`). Called from `0xA6C0E0` |
| `0xb6dd50` | AnimalCamera::Update | Moves the animal camera (`ACamChase` and `ACamHead` modes: chasing an animal, or from its head), slot 35 of `0xF9B578` |
| `0xb3ec30` | WildAnimal::Check | A wild animal's periodic check (virtual slot 8 of WildAnimal): its needs and condition and the messages about it (`WildAnimalMessage_AboutToBeConfiscated`), and the animal name cheats (an animal named `hungry joe`, `thirsty bob`, `mustafa p`, `rick griffiths`, `happy camper`, `sad panda`, `outta here`, `outta here slower` or `commence output`) |
| `0xb2b260` | WAS_Scratch::Update | A wild animal scratching (virtual slot 14 of WAS_Scratch): walks it to something to scratch against and plays `WAS_ScratchAnim` |
| `0xb1d320` | WALMoveSpline::Update | Moves a wild animal along a spline (virtual slot 10 of WALMoveSpline), turning in steps of 20, 45 and 90 degrees |
| `0xb25e60` | WAS_PlayWithItem::Update | A wild animal playing with a toy (virtual slot 12 of WAS_PlayWithItem): goes to the item and plays with it, sometimes chasing it (`WAS_Chase`) |
| `0xb0aec0` | WildAnimal::DebugStats | Writes an animal's developer statistics: `RawName`, `FMName`, enclosure assessment and space, health, food, drink, social, enclosure, cleanliness, tiredness, sleeping, `ColonFullness`, greed. Called from `0xB0C650` |
| `0x4506f0` | Helicopter::Update | The helicopter that brings animal crates on a rope (slot 0 of the table `0xF2952C`, Helicopter's second table): keeps the rope and crate (+0x168) at the helicopter's position and heading, updates its lights (`0x868000`) and runs its state (+0x110, six states: flying along its route, lowering and releasing the crate, ...) |
| `0xb0c1b0` | WildAnimal::StatusText | Writes an animal's status: rescued, pregnant and when it is due (`WildAnimal_Rescued`, `WildAnimal_Pregnant`, `WildAnimal_PregnantDueDate` with `%NUMMONTHS%`). Called from a window builder |
| `0xb7ef80` | WildAnimal::GiveBirth | Handles an animal giving birth: the message `WildAnimalMessage_AboutToGiveBirth`, the young (placed at the mother's `Bip01` bone), with random chances |
| `0xb49e90` | AnimalShelter::SetupWater | Sets up the water of an enclosure's pond or bath: its bottom pieces (`bottom01` ... `bottom04`) and water (`WaterManager`). Called from `0xB51AD0` |
| `0xa0b1c0` | WildLife::Load | Loads the park's wild life: ducks, seagulls, rays, sharks and fish (`Animals\Duck\Duck`, `Animals\Seagull\Seagull`, `Animals\Ray\Ray`, `Animals\Shark\Shark`, `Animals\Fish\Mackeral`) with their three levels of detail (`Body_D01_L1` ...). Called from `0x4787C0` |
| `0xafe510` | WildAnimal::Describe | Writes an animal's rescue status and rarity (`WildAnimal_Rescued`, `WildAnimalRarity`, ` (%.2f)`). Called from `0x677110` |
| `0xb494f0` | AnimalShelter::SetupBottom | Sets up the bottom pieces of an enclosure's water (`bottom01` ... `bottom04`). Called from `0xB4AC70` |
| `0xb4bfc0` | AnimalShelter::SetupParticles | Sets up an animal building's particle effects from its model's bones (`particlestart%d` / `particleend%d`, loaded from `ParticleEffects\*.frw`). Called from `0xB4C660` |
| `0xf13da0` | AnimalAnimations::Names | Makes the names of the animals' animations at start-up (31 wide strings: `Walk`, `WalkSick`, `Run`, `Leap`, `Stand`, `StandSick`, `Idle1`, `Idle2`, `EatIn`, `EatLoop`, `EatOut`, `DrinkIn`, ...), cleaned up at exit |
| `0xb37f70` | WildAnimal::SexAgeText | Writes an animal's sex and age group: `Female` / `Male`, `Adult` / `Baby`. Called from five places |
| `0x6e1e70` | NumAnimalsCondition::Build | Builds the scenario editor's condition on the number of animals of a species: the species list and whether bought animals count (`NumAnimalsOfSpeciesCondition_CountBoughtAnimals`). Called from `0x6E2850` |
| `0xba6cd0` | AnimalWindow::BuildTabs | Builds the view and feeding tabs of an animal's window. Called from `0xB85650` |
| `0xb08510` | WildAnimalItemWindow::Build | Builds an animal toy or item's window: its name and view (`WildAnimalItemGUI_NameWindow`). Called from `0xB55B50` |
| `0xb84390` | WildAnimalVisual::Setup | Sets up an animal's model and finds its head and neck bones (`Bip01 Head`, `Bip01 Neck`), reporting `Missing Wild Animal Resources` and `missing model data for %S`. Called from four places |
| `0xb39c40` | AnimalNames::Load | Loads the animal name lists for the language (French, Italian, Norwegian, Chinese, Korean, simplified Chinese, Japanese, Danish). Called from `0xB3D8B0` |
| `0xb3a1a0` | AnimalNames::Pick | Picks a random animal name for the language (Spanish, German, Swedish, Brazilian Portuguese, Dutch; with the middle name `AnimalMiddleNameThe`). Called from `0xB3D8B0` |
| `0xb21f60` | AnimalPlay::StateNames | Makes the names of the animal play states (`State_Finished`, `State_StandWait`, `State_StandOnce`, `State_SitWait`, `State_MoveTo`, `State_OrientSelf`, `State_Play`, `State_PlayTimeout`, `State_PlayIn1`, ...) |
| `0xb49100` | AnimalCrate::LoadModels | Loads the animal crates' models (`WildAnimals\Crates\Crates`, `Crate02Small`, `Crate%02d`). Called from `0xB4B540` and `0xB4C660` |
| `0xbbf1d0` | AnimalFood::Names | Writes the names of the animal food kinds: foliage, hay and meat. Called from `WildAnimal::DebugStats` and `AnimalInfoPanel::Build` |
