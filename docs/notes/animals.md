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
| `0xb8e240` | AnimalShelter::TroughNames | Makes the names of a shelter's troughs: eat, drink, sleep, male, female. Called from `0xB8E820` |
| `0xb71e30` | WildAnimal::ConfirmRelease | Asks before releasing an animal and reports when it can't be (`ReleaseAnimalConfirmMessage`, `ReleaseAnimalFailMessageBoxMessage`). Called from `0xB08E70` |
| `0xb6fbd0` | WildAnimal::Release | Releases an animal and tells the player, or says why it failed (`ReleaseAnimalReleasedMessage`, `ReleaseAnimalFailMessageBoxMessage`) |
| `0xa14040` | Duck::Models | Picks a duck's model (`Duck01` ... `Duck03`) |
| `0xa19830` | Seagull::Models | Picks a seagull's model (`Seagull01` ... `Seagull03`) |
| `0xa0db30` | Duck::LoadFeathers | Loads the duck feather particles (`DuckFeather`, `BillboardStandard`). Called from `0xA0E1B0` |
| `0xb03220` | Darts::Load | Loads the darts and the safari T. rex model (`WildAnimals\Darts\Darts`, `SafariTrex`). Called from `0xB05170` |
| `0xbbefa0` | WildAnimal::SpeciesStats | Writes a species' statistics for the developer: number of prey, prey, breeding happiness and space needs, vertical step reach, ... (`WildAnimalSpeciesStat_*`). Called from `WildAnimal::DebugStats` |
| `0xb4b290` | AnimalCrate::StateName | Writes an animal crate's state: closed, dropping, fading in, opening, opened, waiting to be picked up |
| `0xb10430` | AnimalFood::ModelNames | Makes the animal food models' names (`foodleaves`, `foodhay`, `foodmeat`). Called from two places |
| `0xb37de0` | WildAnimal::RarityName | Writes an animal's rarity: endangered, rare, moderate, common (`WildAnimalRarity_*`), or baby. Called from two places |
| `0xbbeca0` | WildAnimal::GeneticStats | Writes an animal's genetic statistics for the developer: hunger, enjoyment it gives guests, social, water need, adolescence, ... (`WildAnimalGeneticStat_*`). Called from `WildAnimal::DebugStats` |
| `0xb13c50` | WAS_Chase::StateName | Writes a chasing animal's state: chasing, fleeing, finished |
| `0xa19140` | Seagull::Models (2) | Picks between two seagull models |
| `0xa11810` | Duck::Models (2) | Picks between two duck models |
| `0xb3d8b0` | WildAnimal::Name | Gives an animal its name for its species (random names, `AnimalNames::Load` / `Pick`), reporting `MISSING SPECIES STRING!`. Called from two places |
| `0xb3b700` | WildAnimal::FullName | Writes an animal's full name (`%NAME%`, `%SPECIES%`, `%SUFFIX%`), with the `sad panda` cheat |
| `0xb03590` | Dart::Hit | A dart hitting an animal (`DartHit`): where it lands on the terrain |
| `0xb08e70` | AnimalCamWindow::Build | Builds the animal camera's window (`AnimalCamWindowArea`) |
| `0xb14570` | WildAnimal::Killed | Tells the player an animal was killed (`WildAnimalMessage_Killed`) |
| `0xb37860` | WildAnimal::Escaped | Handles an animal escaping (`WildAnimalState_Escaped`). Called from three places |
| `0xbaba80` | ViewingGallery::NoEnclosure | Says a viewing gallery has no suitable enclosure (`NoSuitableEnclosureForVG`) |
| `0xb83ae0` | WildAnimal::Sound | Plays an animal's sound (`AnimalSound`). Called from `0xB84800` |
| `0xb41ee0` | WildAnimal::Birth | Tells the player an animal gave birth (`WildAnimalMessage_GiveBirth`). Called from `WildAnimal::GiveBirth` and `0xB30A40` |
| `0xb04200` | HelicopterDartCam::Setup | Sets up the dart camera in the helicopter (`HelicopterDartCamMode`, `WildAnimals\Helicopter\Helicopter`). Called from `0xB04840` |
| `0x452800` | WildAnimal::Confiscated | Tells the player an animal was confiscated (`WildAnimalMessage_Confiscated`). Called from `WildAnimal::Check` |
| `0xb8f570` | AnimalShelter::Load | Loads an animal shelter's model (`Enclosures\Shelters\Shelters`, `invisibleproxy`). Called from `0xB90900` |
| `0xb41870` | WildAnimal::NameBaby | Names a newborn animal: a random name with its species and suffix (`%NAME%`, `%SPECIES%`, `%SUFFIX%`). Called from `WildAnimal::Birth` |
| `0x44fc80` | Helicopter::Activate | The helicopter's activation (virtual slot 3, run once by `ManagedObject::Activate`): sets up its model and lights (`0x86D3A0`, `0x869400`), its rope as a spline (`CustomPersistingSpline`) and its place over the terrain |
| `0xb23990` | WAS_PlayWithItem::AddProgress | Adds to the animal's play progress (+0x130), kept between 0 and 1. Called from WAS_PlayWithItem::Update |
| `0xa0a5d0` | Animal::Slot1 | False when the animal's value at +0x98 is above 2; otherwise what `0x48CA90` says for the five arguments. Slot 1 of Animal, Bird, Duck, Fish, ... |
| `0xbaa100` | RegionFI::ReportBroken | Clears +0x14 and shows the enclosure maintenance message `EnclosureMaintenance_Broken` (through `0xBA9700`). Slot 28 of RegionFI |
| `0xbaa180` | RegionFI::ReportStillBroken | Clears +0x14 and shows the message `EnclosureMaintenance_StillBroken`. Slot 29 of RegionFI |
| `0xbaa200` | RegionFI::ReportFixed | Shows the message `EnclosureMaintenance_Fixed`. Slot 30 of RegionFI |
| `0xb448b0` | WildAnimal::Interface60 | The interface (id in `0x13D0168`) of the object its reference at +0x60 points to, looked up once (kept at +0x70, flag +0x74), plus 0x50 |
| `0xb44910` | WildAnimal::Interface60Raw | The interface (id in `0x13D0168`) of the object its reference at +0x60 points to, looked up once (kept at +0x70, flag +0x74), without the +0x50 that WildAnimal::Interface60 adds |
| `0xb7fd50` | WildAnimal::StatusText | The text for a wild animal's status (+0x114): 0 asks the animal itself, 1 is WildAnimal::Escaped, 2 the text `WildAnimal_Rescued` (for a rescued animal, +0xFE), and others for states 3 and 4 |
| `0xb71bc0` | WildAnimal::ConfirmSell | Opens the confirmation for selling a wild animal, with the game text `SellAnimalConfirmMessage` and the animal's details, acting on the animal's reference when confirmed |
| `0xb6fa30` | WildAnimal::ShowSold | After a wild animal is sold, shows the game text `SellAnimalSoldMessage` (in a message window) with its details |
| `0x741280` | AnimalKeeper::NeedEnclosure | In outline: when an animal keeper is hired with no enclosure, shows the game text `EnclosureNeeded_Message`; also makes the keeper (AnimalKeeper::AnimalKeeper) at a random spot |
| `0xb223d0` | WildAnimal::SpeciesText | In outline: a wild animal's species name (`Name_WildAnimalSpecies_PolarBear` among them) with its sex and age text |
| `0xb3b230` | WildAnimal::GrownToAdult | In outline: when a young animal grows up, shows `WildAnimalMessage_GrownToAdult` and sets up its adult look (WildAnimalVisual::Setup) |
| `0xb38560` | WildAnimal::DescendantName | In outline: a descendant animal's name with its number (`AnimalDescendantSuffix`) |
| `0xb3cf30` | WildAnimal::BecomePregnant | In outline: at random (Random::Ran0) makes an animal pregnant and shows `WildAnimalMessage_Pregnant` |
| `0xb8e820` | AnimalShelter::FoodNames | In outline: the names of a shelter's troughs (`food` + number, AnimalShelter::TroughNames) |
| `0x4b6570` | Aquarium::Check | In outline: checks for an aquarium (`Aquarium:trr`) by its name |
| `0xb201a0` | WildAnimal::Move | In outline: an animal's movement step: works out its speed and turn (`0xB355F0`, `0xB35670`, `0xB355B0`), its new position and checks that it stays on the terrain and inside its area |
| `0xb76970` | Enclosure::ScoreCell | In outline: a score for a terrain cell for an animal: checks its height range, flags and enclosure (Terrain::CellHeightRange, Terrain::CellFlag12, EnclosureManager::CellAt), and its neighbours the same way |
| `0xb19e20` | WildAnimal::Steer | In outline: an animal's steering step: when it has a target, measures the way to it and its speed (+0x144), avoids what is ahead (`0xB66290`) and checks the terrain cell it is heading into (Grid::CellAt) |
| `0xadb3a0` | Animal::UpdateAdb3a0 | In outline: an animal's timed update (when +0x70 is set): works out a chance (`0xAD88E0`), sometimes shows a message (`0x6BCE00` with 0x12) and makes an effect object (Object7FC84) |
| `0xb651d0` | WildAnimal::CheckMove | In outline: whether an animal can move from one point to another: finds the terrain cells of the start, middle and end (Grid::CellAt), checks they are in its enclosure (EnclosureManager::Get) and runs the path check (`0xB619B0`) |
| `0xb36ea0` | WildAnimal::DebugDrawB36EA0 | In outline: draws lines around a wild animal (WildAnimal::Interface60Raw) with the shared line drawer (Registry::Ensure, `0x481620`, six lines) for the developers |
| `0xb2c680` | Animal::CheckEnclosure | In outline: when flagged (+0x150), checks which enclosure an animal is in (EnclosureManager::Get, `0xB9AAE0`) against the one it belongs to and acts by its state (+0x7C) |
| `0xb63260` | WildAnimal::FindSpot | In outline: looks for a free terrain cell near a point for an animal (Grid::CellAt), inside a search square (`0x10F294C`, `0x10F2948`), that it can stand on (`0x493200`, Terrain::CellFlag12) |
| `0xb774b0` | WildAnimal::CanReach | In outline: whether an animal can get from its cell to another: checks its enclosure (EnclosureManager::Get), the corner positions (Terrain::CornerPosition) and the distance against its reach, then the cell it lands in (Grid::CellAt) |
| `0xb5fec0` | WildAnimal::StepB5FEC0 | In outline: when flagged (+0x51), moves an animal toward a point inside the terrain (Grid::Contains), rounding its cell |
| `0xb619b0` | WildAnimal::PathClear | In outline: whether the way between two points is clear for an animal: finds their cells (Grid::CellAt), measures the way and runs the cell-by-cell check (`0x494960`) |
| `0xb21400` | WildAnimal::UpdateB21400 | In outline: an animal's update when active (+0xE0): its movement values (`0xB355F0`, `0xB35670`) when it has not stopped (+0x94, +0xB0) |
| `0xb31c90` | Animal::Inspect | In outline: checks an animal for the inspector: its enclosure (EnclosureManager::Get, `0xB9AAE0`, `0xB91550`) and the park inspector manager |
| `0xad9a00` | Animal::ReleaseAD9A00 | In outline: releases an animal's parts and links when not flagged (+0x74): its parts at +0x70, +0x8C, +0xA8 and others |
