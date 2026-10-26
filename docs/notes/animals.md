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
| `0xadb3a0` | Animal::UpdateAdb3a0 | In outline: an animal's timed update (when +0x70 is set): works out a chance (`0xAD88E0`), sometimes checks a cost (Money::CheckCost with 0x12) and makes an effect object (Object7FC84) |
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
| `0xb35770` | WildAnimal::CheckSpotB35770 | In outline: finds the terrain cells of an animal (WildAnimal::Interface60) and of a point near it (Grid::CellAt), and checks them against its enclosure (EnclosureManager::Get) |
| `0xbaa9f0` | Enclosure::UpdateBAA9F0 | In outline: when on (+0x28) and not paused (`0x13D05FC`), checks a thing's place in the enclosures (`0xBAA5E0`, EnclosureManager::Get, its height +0x68) |
| `0xb51ad0` | Animal::PlaceB51AD0 | In outline: places an animal at the ground point under the mouse (MainCamera::Get, `0xCD79C0`) on the terrain (a 90 by 90 grid) |
| `0xb13f60` | Enclosure::RebuildB13F60 | In outline: rebuilds an enclosure's cells on the terrain (a 90 by 90 grid) from the enclosure manager (EnclosureManager::Get, `0xB9AAE0`) |
| `0xb31440` | Animal::UpdateB31440 | In outline: an animal's update when not flagged (+0x98) by its state (+0x78: 1, ...): finds its target (`0xB856E0`, `0xB87EB0`) and checks its box (`0xBCC740`) |
| `0xa123b0` | Animal::ChooseA123B0 | In outline: half the time (Random::Ran0), when not flagged (+0x138), picks an animal's next action (`0xA11350`) |
| `0xb130b0` | Enclosure::RebuildB130B0 | In outline: another enclosure cell rebuild like Enclosure::RebuildB13F60 |
| `0xa158d0` | Animal::ChooseA158D0 | In outline: another animal action pick like Animal::ChooseA123B0 (`0xA14B70`) |
| `0xb180c0` | WildAnimal::PlaceB180C0 | In outline: places an animal on the terrain (a 90 by 90 grid, WildAnimal::Interface60) |
| `0xaf67e0` | Animal::ReleaseAF67E0 | In outline: another release of an animal's parts like Animal::ReleaseAD9A00 (+0x5C, +0x7C) |
| `0xb62ce0` | WildAnimal::GroundB62CE0 | In outline: an animal's ground check: the cell under it (Grid::CellAt) and its height and speed values (+0x74, +0xDC) |
| `0xb5a1e0` | Animal::PlaceB5A1E0 | In outline: another animal placement like Animal::PlaceB51AD0 at the point under the mouse |
| `0xb3c420` | Animal::CheckB3C420 | In outline: checks an animal's spot on the terrain when not flagged (+0x74), comparing two values |
| `0xb2de20` | WildAnimal::SetupB2DE20 | In outline: sets up an animal on the terrain (a 90 by 90 grid) in its enclosure (EnclosureManager::Get, WildAnimal::Interface60) |
| `0xb024b0` | Animal::IdleB024B0 | In outline: when not flagged (+0x2FC), picks an animal's idle move at random (Random::Ran0, `0xDDD39A`) |
| `0xb66e70` | WildAnimal::TurnB66E70 | In outline: works out the angle between an animal's way and its target's (`0xB66910`) and eases its turn (`0xDDEBC0`) |
| `0xb65d60` | WildAnimal::MoveB65D60 | In outline: moves an animal toward a free spot (WildAnimal::FindSpot) when it has room (more than three) and is not flagged (+0xFD, +0xB0) |
| `0xa19310` | Bird::LandA19310 | In outline: for kind 0x201 not in mode 2, makes a landing route (LandingRoute) and a particle puff (Particle::Launch with 10, 20, 0.3) and starts it (`0xA103F0`) |
| `0xbab1b0` | Enclosure::CellOKBAB1B0 | In outline: whether a cell lies inside an enclosure's grid with a margin (EnclosureManager::Get, its width +0x64 and height +0x68) |
| `0xa16210` | Bird::UpdateA16210 | In outline: for kind 0x100 not flagged (+0x14C), the plain update (`0xA161D0`), otherwise half the time (Random::Ran0) picks by case |
| `0xb62460` | Animal::FollowB62460 | In outline: an animal's follow update: with no slot (+0x60 below 0) and flagged (+0x48), done; otherwise checks its target (+0x64, `0xB848F0`) |
| `0xa13340` | Bird::UpdateA13340 | In outline: another bird update: for kind 0x200 not in mode 2, by its stage (+0x6C: 0, 2) and flag (+0x15C), checks (`0xA113D0`) |
| `0xb38a30` | Animal::CanActB38A30 | In outline: whether an animal can act now: not flagged (+0xFC, the owner's bit 8), and its slot and links in order |
| `0xb30a40` | Animal::UpdateB30A40 | In outline: an animal's update when not flagged (+0xB0): checks with the shared manager (`0x13CEE84`) and, when done, acts (`0xB7AAD0`) |
| `0xb16010` | WildAnimal::PoseB16010 | In outline: sets an animal's pose by its mode (+0x84: 1, ...), pointing it (Matrix::LookAlongYSide with `0x10F2EA0`) on the terrain |
| `0xb13700` | Animal::HandleB13700 | In outline: an animal's handling by its state (+0x78: 0, ...) of the thing given |
| `0xa16f30` | Bird::PlaceA16F30 | In outline: places a bird at a cell over the terrain (a 90 by 90 grid, `0x492CD0`) |
| `0xaf1460` | Animal::UpdateAF1460 | In outline: another animal update when not flagged (+0x74), checking its parts (+0x50) and its owner's flag (+0x70 +0xFC) |
| `0xad95a0` | Animal::CopyAD95A0 | In outline: makes a managed copy of an animal (ManagedObject with `0x13CE51C`) from another when it is not flagged (+0x74), with its parts (`0x9D8210`) |
| `0xb8c290` | Animal::PickSpotB8C290 | In outline: picks a random free spot (`0x4751E0`) from a list (0x1C bytes each at +0x48) |
| `0xb42410` | WildAnimal::DrawB42410 | In outline: draws an animal turned by its angle (Matrix::TurnColumnsAboutY) with its blob shadow (Shadows::BlobSetup) |
| `0xb4c660` | Animal::UpdateB4C660 | In outline: unless off (Flags::Bit0), an animal's update for its parts |
| `0xa11f70` | Duck::MakeFlock | In outline: makes n ducks (Duck::Duck), activates them and spreads them at random (Random::Ran0) |
| `0xb23050` | WildAnimal::UpdateB23050 | In outline: an animal's update when not flagged (+0xED, +0x94): with its owner (Interface48::Get) checks (`0xB35710`) |
| `0xa16740` | Duck::Swim | In outline: a duck's update on the water (WaterManager::Get, `0x941D00`): swims (`0xA151F0`) or does something else (`0xA15EA0`) |
| `0xad6ac0` | Animal::UpdateAD6AC0 | In outline: another animal update when not flagged (+0x10C) |
| `0xb48c10` | WildAnimal::MoveB48C10 | In outline: moves an animal when not flagged (+0x1C, +0x24) and the move is long enough |
| `0xb04ae0` | Animal::UpdateB04AE0 | In outline: goes over an animal's parts while not flagged (+0x220, +0x2DC), with a case for +0x2D4 |
| `0xb1ca90` | Animal::TargetB1CA90 | In outline: unless in mode 1 (+0xC1), activates an animal and finds its target (`0xB856E0` on +0x60) |
| `0xb59de0` | Animal::AimB59DE0 | In outline: when flagged (+0x40), turns an animal toward its target (`0xB587A0`, `0xB567F0`) |
| `0x494960` | WildAnimal::PathCheck | In outline: the cell-by-cell path check used by WildAnimal::PathClear: sets up the cells between two points (`0x491E30`) and checks each on the terrain (a 90 by 90 grid) |
| `0xb3c980` | WildAnimal::MakeCarcass | In outline: makes a carcass (a 0xC0-byte Carcass) where a wild animal is (WildAnimal::Interface60) |
| `0xb2da50` | WildAnimal::SetupB2DA50 | In outline: another animal setup like WildAnimal::SetupB2DE20 (terrain, enclosures) |
| `0xaf60e0` | Animal::UpdateAF60E0 | In outline: an animal's timed update when on (+0x70) with a slot (+0xA4) and not flagged (+0x94): its chance (`0xAF49C0`) |
| `0xb66ad0` | WildAnimal::CheckSpotB66AD0 | In outline: checks a spot for an animal over the terrain within a distance (under 1.5) |
| `0xb198b0` | WildAnimal::PlaceB198B0 | In outline: places an animal (WildAnimal::Interface60) on the terrain (a 90 by 90 grid) |
| `0xb782a0` | Entries::SortB782A0 | In outline: sorts a list of 0x70-byte entries by their value at +8 |
| `0xb43a90` | WildAnimal::TurnB43A90 | In outline: turns an animal when the change is big enough and it has a target (+0x40), by case, when flagged (+0x188) |
| `0xb54ae0` | Animal::TargetB54AE0 | In outline: when not flagged (+0xC4), checks whether an animal's target (+0xC0) changed and finds the new one (`0xB539B0`) |
| `0xb8c730` | Animal::FaceB8C730 | In outline: points an animal toward its target (`0xB891A0`, `0xB89460`, Matrix::LookAlongZSide) |
| `0xb169d0` | Animal::EnclosureB169D0 | In outline: checks an animal's enclosure (EnclosureManager::Get, `0xB9AC20`) and acts by its state (+0x78) |
| `0x8189b0` | SceneryBehaviour::Pick8189B0 | In outline: with a reference (WithReference8155F0), picks a random value (`0x4751E0`), rounded |
| `0xb45c30` | Animal::UpdateB45C30 | In outline: an animal's update when not flagged (+0x58, +0x74) (`0xB33D10`) |
| `0xb1c290` | Animal::UpdateB1C290 | In outline: an animal's update with its owner (Interface48::Get) when not flagged (+0x94) |
| `0xaf6480` | Animal::CopyAF6480 | In outline: another managed copy like Animal::CopyAD95A0 (`0x13CE60C`), when not flagged (+0x60) |
| `0xb9ac20` | Enclosure::CellInside | In outline: whether a cell lies inside an enclosure's grid (from 1 up to its size, passed in a register) |
| `0xb888c0` | Animal::ActB888C0 | In outline: an animal's action by case when it has parts (+0x74 +0x44) and is not flagged (+0xB8) |
| `0xa13850` | Bird::UpdateA13850 | In outline: another bird update for kind 0x201 in stage 1 when not flagged (+0x118) |
| `0xb18660` | WildAnimal::MoveTo | In outline: makes a move-to step for an animal (WildAnimalLocomotorMoveTo) and starts it (`0xB179B0`) when not flagged (+0xEC) |
| `0xb18aa0` | WildAnimal::MakeB18AA0 | In outline: makes an animal's step over the terrain |
| `0xb19570` | WildAnimal::CheckB19570 | In outline: another animal check over the terrain (a 90 by 90 grid) |
| `0xb47040` | WildAnimal::UpdateB47040 | In outline: another animal update when not flagged (+0x9C) |
| `0xb015e0` | Animal::PoseB015E0 | In outline: builds an animal's part matrices (Model::PartMatrices869400) |
| `0xb487b0` | WildAnimal::StepB487B0 | In outline: when a time (+0xF4) is below another (+0x118), moves an animal on (`0xB47AD0`, `0xB48060`) |
| `0xb17300` | WildAnimal::PlaceB17300 | In outline: places an animal on the terrain with its record |
| `0xa15ea0` | Duck::Other | In outline: a duck's other action when not flagged (+0x128) (`0xA14B70`) |
| `0xa12990` | Animal::ChooseA12990 | In outline: another animal action pick when not flagged (+0x138) (`0xA11350`) |
| `0xa18e20` | Bird::UpdateA18E20 | In outline: another bird update for kind 0x200 not in mode 2 |
| `0xb60f50` | WildAnimal::StepB60F50 | In outline: an animal's step toward its target (`0xB84800` on +0x60) over the terrain |
| `0xb3af20` | WildAnimal::PlaceB3AF20 | In outline: places an animal (WildAnimal::Interface60) on the terrain |
| `0xb15c80` | WildAnimal::TerrainB15C80 | In outline: an animal pass over the terrain (a 90 by 90 grid) |
| `0xbaa5e0` | Enclosure::PickBAA5E0 | In outline: finds the enclosure cell under a point from the camera over the terrain |
| `0xb19170` | WildAnimal::PlaceB19170 | In outline: another animal placement over the terrain |
| `0xb67960` | WildAnimal::CellB67960 | In outline: finds an animal's terrain cell (Grid::CellAt) |
| `0xb526b0` | Animal::LinkB526B0 | In outline: links an animal's part when it has one (+0x138 +0xC) |
| `0x9857e0` | Animal::Release9857E0 | In outline: releases an animal's links |
| `0xb552a0` | Animal::CheckB552A0 | In outline: an animal check when not flagged (+0xC4) with the shared manager (`0x13CEA88`) |
| `0xb437b0` | WildAnimal::UpdateB437B0 | In outline: an animal update when not flagged (+0x118) |
| `0xb1cfc0` | WildAnimal::NearB1CFC0 | In outline: whether an animal is within a distance on the ground plane |
| `0x9a7840` | Animal::Update9A7840 | In outline: an update when not flagged (+0x40) |
| `0xb30280` | Animal::TextB30280 | In outline: an animal's text by its state (+0x78: 0, ...) |
| `0xb7aba0` | Animal::UpdateB7ABA0 | In outline: when on (+0x70), an animal's update (`0xB7A620`) |
| `0xb70920` | Enclosure::ValueB70920 | In outline: an enclosure value over its entries (+0x60) |
| `0xb699d0` | Animal::ListB699D0 | In outline: goes over an animal's list (+0x17C, more than two) |
| `0xb2fc40` | Animal::UpdateB2FC40 | In outline: an animal update when not flagged (+0xB0) |
| `0x817910` | SceneryBehaviour::Update817910 | In outline: a behaviour update when not flagged (+0x60) |
| `0xb0faa0` | Animal::StepB0FAA0 | In outline: once a time has passed (+0xB4), an animal's step over the terrain |
| `0xb44a30` | Animal::UpdateB44A30 | In outline: an animal update when not flagged (+0x74) |
| `0xb742e0` | Effect::BoundsB742E0 | In outline: another effect box grow like Effect::Bounds48B6F0 |
| `0xb5e1c0` | Effect::BoundsB5E1C0 | In outline: another effect box grow like Effect::Bounds48B6F0 |
| `0xb98d70` | PathGrid::AnimalB98D70 | In outline: sets up a path table (`0x488290` with 0x10) over the terrain for an animal |
| `0xb17710` | WildAnimal::UpdateB17710 | In outline: an animal update when not flagged (+0xCC) |
| `0xa10bc0` | Bird::AngleA10BC0 | In outline: a bird's heading from its direction (arctangent of +0xA0 and +0xA8) and distance |
| `0xb7ecf0` | WildAnimal::TerrainB7ECF0 | In outline: an animal's check over the terrain (WildAnimal::Interface60) |
| `0xb8ef10` | Animal::RefreshB8EF10 | In outline: refreshes an animal (`0xB8DF10`, `0xB8A7F0`) |
| `0xb56510` | Animal::CompareB56510 | In outline: compares a value (`0xB55E90`) against a limit |
| `0xb415f0` | WildAnimal::Make | In outline: makes a wild animal (WildAnimal::WildAnimal) and sets it up (`0xB368D0`) |
| `0xb88e20` | Animal::ListB88E20 | In outline: goes over a list's entries not flagged (+0x18) |
| `0xad7b80` | Animal::KeyAD7B80 | In outline: finds an animation key at a time (Keys::FindTime) |
| `0xb3b480` | Animal::LinkB3B480 | In outline: sets an animal's link (+0x4C) when it changes |
| `0xb46080` | WildAnimal::Decide | In outline: with nothing to do (+0x78), makes an animal decision (WAS_Decision) |
| `0xb47380` | WildAnimal::CheckB47380 | In outline: an animal check when flagged (+0xAD) |
| `0xb46530` | WildAnimal::LinkB46530 | In outline: links an animal's parts |
| `0xa7e890` | Animal::UpdateA7E890 | In outline: an animal update when not flagged (+0x100) |
| `0xb8dbd0` | Animal::LinkB8DBD0 | In outline: links an animal's part |
| `0xb94650` | Enclosure::PickB94650 | In outline: finds an enclosure cell under the mouse (MainCamera::Get) on the terrain |
| `0xb72be0` | Animal::TextB72BE0 | In outline: an animal text when not flagged (+0xB8) |
| `0xb42910` | WildAnimal::SwayB42910 | In outline: works out an animal's sway from a fixed angle (sine of `0xF2E180`) |
| `0x98bc80` | Animal::Turn98BC80 | In outline: a turn when big enough |
| `0xb67700` | WildAnimal::StepB67700 | In outline: an animal step when not flagged (+0xC2) (`0xB62430`) |
| `0xa96ee0` | Animal::ValueA96EE0 | In outline: an animal value by its state (+0x7C) unless in mode 1 (+0x90) |
| `0x437de0` | Animal::Info437DE0 | In outline: an animal's info when it has one (+0x1B8) |
| `0xb87450` | Animal::ListB87450 | In outline: goes over an animal's entries (+0x9C) when not flagged (+0x78) |
| `0xb15430` | WildAnimal::UpdateB15430 | In outline: an animal update when not flagged (+0x95, +0xB0) |
| `0xb58f40` | Animal::PassB58F40 | In outline: goes over n entries when flagged (+0x10C bit 2) |
| `0xa08750` | Animal::UpdateA08750 | In outline: an animal update when not flagged (+0x18C, +0x141) |
| `0xb462f0` | WildAnimal::StepB462F0 | In outline: an animal step with no task (+0x78) and entries (+0x80) |
| `0xba4030` | Animal::UpdateBA4030 | In outline: an animal update when not flagged (+0xC8) |
| `0xb72370` | Animal::UpdateB72370 | In outline: an animal update when not flagged (+0xB8) |
| `0xa6c690` | Animal::PassA6C690 | In outline: goes over an animal's entries |
| `0xa40080` | Animal::FindA40080 | In outline: finds an animal entry unless told not to |
| `0xb5a9b0` | Animal::PartB5A9B0 | In outline: an animal step with its part (+0x144 +0xC) |
| `0xa98c60` | PoolComplex::CachedA98C60 | In outline: a step with the cached object (Interface64::GetCached) |
| `0xa88a60` | Animal::ValueA88A60 | In outline: an animal value above a limit (`0xF29638`) |
| `0x9c5850` | Fireworks::Check9C5850 | In outline: a firework check when not flagged (+0x30) |
| `0xb47ad0` | WildAnimal::DistanceB47AD0 | In outline: an animal's distance to a point |
| `0xb725a0` | Animal::UpdateB725A0 | In outline: an animal update when not flagged (+0xB8) |
| `0xa0ffe0` | Bird::NearA0FFE0 | In outline: whether a point is further than a scaled distance |
| `0xa8b690` | Animal::PassA8B690 | In outline: goes over an animal's entries (+0xD0) |
| `0xb8bc50` | Animal::CaseB8BC50 | In outline: an animal value by case |
| `0xb1c670` | Animal::GetB1C670 | In outline: a step with an object (Interface44c::Get) |
| `0xb33f40` | Animal::PassB33F40 | In outline: goes over a list (+0x14) |
| `0xb8d9c0` | Animal::UpdateB8D9C0 | In outline: an animal update when not flagged (+0xF8) |
| `0xb3ea20` | Animal::UpdateB3EA20 | In outline: an animal update (Animal::UpdateB45C30) |
| `0xaebd40` | Animal::UpdateAEBD40 | In outline: an animal update when not flagged (+0x100) |
| `0xa7bf80` | Animal::UpdateA7BF80 | In outline: an animal update when not flagged (+0x1C4) |
| `0xbafef0` | Shop::TimeBAFEF0 | In outline: a shop time step (Shop::Time4C0450) |
| `0xb9c020` | PathGrid::ValueB9C020 | In outline: a path grid value for an entry not flagged (+0x18) |
| `0xa6ed40` | Animal::PointA6ED40 | In outline: a point check against a shape with more than three points |
| `0xb7c9a0` | Animal::UpdateB7C9A0 | In outline: an animal update when not flagged (+0x90) |
| `0xa0ef90` | Bird::UpdateA0EF90 | In outline: unless the global flag `0x1400306` is set, a bird step |
| `0xa88860` | Animal::ValueA88860 | In outline: an animal value from its entries (+0x28) |
| `0xb77db0` | Animal::StepB77DB0 | In outline: an animal step (`0xB0E0F0`) |
| `0xab0520` | Animal::ModeAB0520 | In outline: acts by an animal's mode (+0x120) |
| `0xb98aa0` | PathGrid::FlagB98AA0 | In outline: a step when a table entry is not flagged (`0x10F29BB`) |
| `0xb30090` | Animal::GetB30090 | In outline: a step with an object (Interface48::Get) |
| `0xb369d0` | Animal::CountB369D0 | In outline: an animal step for a positive count |
| `0xb9ce90` | PathGrid::CountB9CE90 | In outline: a step for a positive count |
| `0xb8be80` | Animal::UpdateB8BE80 | In outline: an animal update when not flagged (+0xF8) |
| `0xb56ec0` | Animal::TerrainB56EC0 | In outline: an animal step over the terrain |
| `0xb48060` | WildAnimal::PartB48060 | In outline: a step with a part when it has one |
| `0xb87cd0` | Animal::CheckB87CD0 | In outline: an animal check when not flagged (+0x98) |
| `0xb719b0` | Animal::PassB719B0 | In outline: goes over an animal's entries (+0xB4) |
| `0xa88470` | Animal::ValueA88470 | In outline: an animal value above 1 |
| `0xaf3640` | Animal::PassAF3640 | In outline: goes over entries (+0x50) |
| `0xa0a3d0` | Bird::TimeA0A3D0 | In outline: a step with a non-negative time |
| `0xba5820` | Animal::PassBA5820 | In outline: goes over entries (+8) |
| `0xb83140` | Animal::PathB83140 | In outline: a path step (`0x703A90` with +0x68) |
| `0xa8b8b0` | Animal::CheckA8B8B0 | In outline: an animal check (`0xA8A150`) |
| `0xaa15a0` | PoolComplex::CountAA15A0 | In outline: a step for a positive count |
| `0x998d10` | Fireworks::Count998D10 | In outline: a step for a positive count |
| `0xbb6b10` | Animal::StepBB6B10 | In outline: a step unless told not to |
| `0xb3cd60` | Animal::LoopB3CD60 | In outline: goes over a linked list |
| `0xb9bc90` | PathGrid::CountB9BC90 | In outline: a step for a positive count |
| `0xaf1290` | Animal::ValueAF1290 | In outline: an animal value when not flagged (+0x74) |
| `0xb529b0` | Animal::PartB529B0 | In outline: a step with its part (+0x138 +0xC) |
| `0xb01420` | WildAnimal::TerrainB01420 | In outline: an animal step over the terrain (WildAnimal::Interface60) |
| `0xa6f930` | Animal::ModeA6F930 | In outline: a step in mode 2 |
| `0xa6c4d0` | Animal::PassA6C4D0 | In outline: goes over entries (+0x48) |
| `0xae1390` | Animal::BigAE1390 | In outline: a step when a count is over 0x400 |
| `0xb33850` | Animal::CopyB33850 | In outline: copies an entry (Thunk) |
| `0x414470` | Animal::Pass414470 | In outline: goes over entries (+0x84) |
| `0xb87b10` | Animal::UpdateB87B10 | In outline: an update when not flagged (+0x98) |
| `0xb1fe80` | Animal::GetB1FE80 | In outline: a step with an object (Interface48::Get) |
| `0xb01c20` | WildAnimal::UpdateB01C20 | In outline: an update when not flagged (+0x228) |
| `0xb6cbd0` | Animal::PointB6CBD0 | In outline: a point check against a shape with more than three points |
| `0xb84f90` | Animal::GetB84F90 | In outline: a step with an object (`0x13064A0`) |
| `0xa1ab40` | Bird::ModeA1AB40 | In outline: a step unless in mode 1 |
| `0xb22600` | Animal::UpdateB22600 | In outline: an update when not flagged (+0xB0) |
| `0xb32420` | WildAnimal::UseShelter | In outline: makes a use-shelter decision (WAS_UseShelter) |
| `0xaebf50` | Animal::UpdateAEBF50 | In outline: a step when on (+0x2C) |
| `0xaf0150` | Animal::TextAF0150 | In outline: a text when not flagged (+0x10C) |
| `0xad7110` | Animal::TextAD7110 | In outline: the same (another copy) |
| `0xafa2e0` | Animal::SlotAFA2E0 | In outline: a step when it has a slot |
| `0xb82a30` | Animal::UpdateB82A30 | In outline: a step when not flagged (+0x50) |
| `0xaae4a0` | PoolComplex::PointAAE4A0 | In outline: a step with a point (`0xAAE430`) |
| `0xa151f0` | Bird::OnA151F0 | In outline: a step when on (+0x14C) |
| `0xa11950` | Bird::OnA11950 | In outline: a step when on (+0x15C) |
| `0xab5b70` | Animal::CaseAB5B70 | In outline: a step by case |
| `0xab2d00` | Animal::StepAB2D00 | In outline: a step with an object (`0x13CE3F8`) |
| `0xb97250` | PathGrid::StepB97250 | In outline: a step (`0xB97150`) |
| `0xabac70` | Fence::OnABAC70 | In outline: a step when on (+0xE8) |
| `0xb9e4f0` | PathGrid::FlagB9E4F0 | In outline: a step when a table entry is not flagged (`0x10F29C8`) |
| `0xab4430` | Animal::StepAB4430 | In outline: a step (`0xA9A440`) |
| `0xbae740` | Animal::CheckBAE740 | In outline: a check |
| `0xab4f90` | Animal::ListAB4F90 | In outline: goes over a list (+0x1C4) |
| `0x45ec70` | Animal::Update45EC70 | In outline: an update when not flagged (+0x884) |
| `0xb609c0` | WildAnimal::RandomB609C0 | In outline: a random point (Random::Ran0) |
| `0xb21b90` | Animal::UpdateB21B90 | In outline: an update when not flagged (+0xB0) |
| `0xb9a520` | PathGrid::MakeB9A520 | In outline: makes a part |
| `0xb76290` | Animal::UpdateB76290 | In outline: unless the global flag `0x1400306` is set, a step |
| `0xaa2400` | PoolComplex::StepAA2400 | In outline: a step (`0x63AAB0`) |
| `0xa15050` | Bird::UpdateA15050 | In outline: an update when not flagged (+0x128) |
| `0xab0780` | Animal::StepAB0780 | In outline: a step when asked |
| `0xaaa8d0` | PoolComplex::CountAAA8D0 | In outline: a step when a count is above 2 |
| `0xaa0040` | PoolComplex::LoopAA0040 | In outline: goes over a linked list |
| `0xb73cf0` | Animal::CountB73CF0 | In outline: a step for a positive count |
| `0xbb4460` | Animal::PassBB4460 | In outline: goes over entries (+0x100) |
| `0xb83380` | Animal::PassB83380 | In outline: goes over entries (+8) |
| `0xa12ee0` | Bird::UpdateA12EE0 | In outline: an update when not flagged (+0x138) |
| `0xb788a0` | Animal::LoopB788A0 | In outline: goes over a linked list |
| `0xab0930` | Animal::CaseAB0930 | In outline: a value by case |
| `0xa131c0` | Bird::UpdateA131C0 | In outline: an update when not flagged (+0x138) |
| `0xb6af50` | Animal::OnB6AF50 | In outline: a step when on (+0x74) |
| `0xb52b70` | Animal::PartB52B70 | In outline: a step with a part (+0x130) and none at +0x14C |
| `0xb99e70` | PathGrid::CountB99E70 | In outline: a step for a positive count |
| `0xab0380` | Animal::ModeAB0380 | In outline: a check by an animal's mode (+0x120) |
| `0xb6c720` | Animal::CountB6C720 | In outline: a step for a positive count |
| `0xa89220` | Animal::BandA89220 | In outline: a step with a guest band (Guest::Band) |
| `0xa14660` | Bird::WaterA14660 | In outline: a step with the water manager |
| `0xb39260` | Animal::RandomB39260 | In outline: when not flagged (+0xFE), a random value between 0.6 and 0.75 (Random::Ran0Range) |
| `0xa87a50` | Animal::SpotA87A50 | In outline: a step with a guest spot (Guest::Spot79A6B0, `0xA87450`) |
| `0xa06110` | Bird::MakeA06110 | In outline: makes a part when not flagged (+0x68) |
| `0x415c00` | Animal::Pass415C00 | In outline: goes over entries (+0x98) when there is none |
| `0xb5a840` | Animal::FlagB5A840 | In outline: a step by a flag (+0x156) |
| `0xb7e990` | Animal::UpdateB7E990 | In outline: an update when not flagged (+0xD4) (Interface48::Get) |
| `0xaeb280` | Animal::PlaceAEB280 | In outline: when on (+0x2C), places parts (Window::UpdateRect) |
| `0xae5d40` | Animal::PassAE5D40 | In outline: goes over entries not flagged (+0x18) |
| `0xafdfa0` | Animal::StepAFDFA0 | In outline: when on (+0x34), a step (`0xAFDEC0`) |
| `0xb0f930` | Animal::CheckB0F930 | In outline: a step when a check passes and a count (+0x94) is positive |
| `0xb51750` | Animal::StepB51750 | In outline: a step (`0xB354D0`) when its owner is not flagged (+8 +0xFF) |
| `0xabb910` | Animal::CaseABB910 | In outline: a step by case |
| `0xb2e770` | Animal::GetB2E770 | In outline: a step with objects (Interface48::Get, WildAnimal::Interface60) |
| `0xb2d3f0` | Animal::FlagsB2D3F0 | In outline: a step when flagged (+0xB6) and not flagged (+0xAC) |
| `0xb117e0` | Animal::CountB117E0 | In outline: a loop, with no entries handled |
| `0xa065f0` | Bird::CaseA065F0 | In outline: when asked, a step by a value (+0x14) |
| `0xb7c480` | WildAnimal::MoveTo | In outline: makes a move-to locomotor (WildAnimalLocomotorMoveTo) |
| `0xad86a0` | Animal::UpdateAD86A0 | In outline: an update when not flagged (+0x74) |
| `0xafb160` | Animal::SlotAFB160 | In outline: a step for a 0x2C-byte entry flagged (+0x10 bit 1), up to 0xFF |
| `0xb31b20` | WildAnimal::UseShelter2 | In outline: makes a use-shelter decision (WAS_UseShelter) and a part (`0x73E100`) |
| `0xb10780` | Animal::CountB10780 | In outline: a step when a count (`0xB105F0` +0x50) is under 1 |
| `0xa61f10` | Animal::SpeedA61F10 | In outline: a step when a speed (+0x144) is big enough and not positive |
| `0xb34950` | Animal::TerrainB34950 | In outline: a step over the terrain |
| `0xb20040` | WildAnimal::StepB20040 | In outline: a step with an object (WildAnimal::Interface60) |
| `0xa5fc30` | Animal::TurnA5FC30 | In outline: a turn step (sine) when a direction is long enough |
| `0xbae9d0` | Animal::SlotBAE9D0 | In outline: a step for an entry not flagged (+0x1D4, +0x18) |
| `0xa0f590` | Bird::TerrainA0F590 | In outline: a value from the terrain |
| `0xb69dc0` | Animal::TerrainB69DC0 | In outline: a step over the terrain |
| `0xb587e0` | Animal::ModeB587E0 | In outline: in mode 1 (+0x138), a step (`0xB567A0`) |
| `0xb53850` | Animal::PartB53850 | In outline: a step with a part (+0x8C) when not flagged (+0x90) |
| `0xaaa300` | PoolComplex::PointsAAA300 | In outline: a step with more than two points |
| `0xa8d8b0` | PoolComplex::Collection | In outline: makes a pool complex collection (PoolComplexCollection) |
| `0xa28b60` | Animal::CheckA28B60 | In outline: a check with a part |
| `0xaf46c0` | Animal::UpdateAF46C0 | In outline: an update when not flagged (+0x60) |
| `0xa14ef0` | Bird::ValueA14EF0 | In outline: a value when not flagged (+0x144) |
| `0xa14d90` | Bird::ValueA14D90 | In outline: a value when not flagged (+0x128) |
| `0xafa180` | Animal::SlotAFA180 | In outline: a step for a slot inside a grid (+0x2C by +0x30) |
| `0xa7c280` | Animal::UpdateA7C280 | In outline: an update when not flagged (+0x1C4) |
| `0xac5d80` | Animal::ValueAC5D80 | In outline: a value when not flagged (+0x1C) |
| `0xba4940` | WildAnimal::TerrainBA4940 | In outline: a step over the terrain (WildAnimal::Interface60) |
| `0xb931d0` | Animal::TextB931D0 | In outline: makes a part and a text (Text::MakeBDAC20) |
| `0xaa9c30` | Animal::ValueAA9C30 | In outline: a value (Animal::ValueAC5D80) |
| `0xaa33d0` | PoolPiece::SidesAA33D0 | In outline: a pool piece's turned sides (Sides::Rotate, PoolPiece::SideData) |
| `0xb23700` | WildAnimal::PlayWithAnimals | In outline: makes a play-with-animals decision (WAS_PlayWithAnimals) and a part (`0x44E540`) |
| `0xb85d90` | Animal::FaceB85D90 | In outline: points (Matrix::LookAlongZSide) |
| `0xa9b800` | PoolComplex::UpdateA9B800 | In outline: an update when not flagged (+0x1B0) |
| `0xb21280` | Animal::CaseB21280 | In outline: a value by a mode (+0x78) |
| `0xb81f20` | Animal::DistanceB81F20 | In outline: a distance step when on (+0x50) |
| `0xa13070` | Bird::LandingRoute | In outline: unless a value is 0x200, makes a landing route (LandingRoute) |
| `0xb1fb50` | WildAnimal::FaceB1FB50 | In outline: points an animal (Matrix::LookAlongZSide on +0xC0) |
| `0xaf26f0` | Animal::LinkAF26F0 | In outline: a step (`0x4A47B0`) with a part |
| `0xa7c6a0` | Animal::CheckA7C6A0 | In outline: a check when not flagged (+0x84) |
| `0xaf5210` | Animal::LinkAF5210 | In outline: a step (`0xAEFF30`) with a part |
| `0xadfc50` | Animal::LinkADFC50 | In outline: a step (`0xAD6EF0`) with a part |
| `0xb8f420` | Animal::FlagsB8F420 | In outline: a step when flagged (+0x184) and not flagged (+0x1A8) |
| `0xb563b0` | Animal::CompareB563B0 | In outline: compares a value (`0xB55E90`) |
| `0xb1c940` | WildAnimal::AngleB1C940 | In outline: a step with a cosine of twice an angle |
| `0xabbab0` | Animal::EventABBAB0 | In outline: with a part (+0x284), a step for values other than 6, 8 and 9 |
| `0xb72120` | Animal::StepB72120 | In outline: a step (`0x44E540`) unless told not to |
| `0xa91880` | Fence::FindA91880 | In outline: a find in a sorted list (SortedPointers::FindBy8 on `0x13CDF84`) |
| `0xa18cd0` | Seagull::Make | In outline: makes a seagull (Seagull) and activates it |
| `0xb47770` | WildAnimal::SlotB47770 | In outline: a step for a slot (+0x78) not flagged |
| `0x9d7e70` | Animal::Update9D7E70 | In outline: an update when not flagged (+0x68) |
| `0xa5cbf0` | Animal::CountA5CBF0 | In outline: a loop with no entries (+0x78) handled |
| `0xb45080` | WildAnimal::SlotB45080 | In outline: a step for a slot (+0x78) not flagged |
| `0xb88480` | Animal::UpdateB88480 | In outline: an update when not flagged (+0xB8) |
| `0xb7bd00` | Animal::CheckB7BD00 | In outline: a check when not flagged (+0x90) |
| `0xb45520` | WildAnimal::SlotB45520 | In outline: a step for a slot not flagged |
| `0xaad2f0` | PoolComplex::CaseAAD2F0 | In outline: unless told not to, a step by four bits |
| `0xa75800` | Animal::IndexA75800 | In outline: makes an index of four-byte entries |
| `0xaf0300` | Animal::CheckAF0300 | In outline: a check when not flagged (+0x10C) |
| `0xad72c0` | Animal::CheckAD72C0 | In outline: the same (another copy) |
| `0xb55590` | WildAnimal::FamilyMember | In outline: makes a family member (WildAnimalFamilyMember) and a part (`0x44E540`) |
| `0xb1f7e0` | WildAnimal::TerrainB1F7E0 | In outline: a step over the terrain |
| `0xa62080` | Animal::TurnA62080 | In outline: a turn step (sine of +0x38) for a short direction |
| `0xa60380` | Animal::TerrainA60380 | In outline: a step over the terrain |
| `0xb8c150` | Animal::CheckB8C150 | In outline: a check when not flagged (+0xF8) |
| `0xabb6f0` | Fence::StepABB6F0 | In outline: unless told not to, a step when on (+0xE8) |
| `0xb36bd0` | Animal::CheckB36BD0 | In outline: a check when not flagged (+0x188) |
| `0xb3ade0` | Animal::TerrainB3ADE0 | In outline: a step over the terrain |
| `0xaae920` | PoolComplex::CaseAAE920 | In outline: a point (`0xAAE430`) and a step by case |
| `0xa7c510` | Animal::StepA7C510 | In outline: a step with an object (`0x13CE3BC`, `0xA9EF10`) |
| `0xbad070` | Animal::PassBAD070 | In outline: when on (+0x21C), goes over entries (+0x1D8) |
| `0xa90e70` | Fence::NearA90E70 | In outline: a step for points beyond a distance |
| `0xba2db0` | Animal::SlotBA2DB0 | In outline: a step for an entry not flagged (+0x18) |
| `0xb58be0` | Animal::EventB58BE0 | In outline: for event 7, a step with an object (WildAnimal::Interface60) |
| `0xb5d490` | Animal::QuadTreeB5D490 | In outline: inserts an object by its matrix into a quad tree (Object::GetMatrix, QuadTree::Insert) |
| `0xab3940` | Animal::PassAB3940 | In outline: goes over entries (+0x10C) with a part |
| `0xa9f8c0` | PoolComplex::MakeA9F8C0 | In outline: makes a part and goes over entries (+0x28) |
| `0xb25c60` | WildAnimal::InteractWithItem | In outline: makes an interact-with-item decision (WAS_InteractWithItem) and a part (`0xB24230`) |
| `0xae11a0` | Animal::PassAE11A0 | In outline: goes over entries (+0x50) comparing values |
| `0xa327e0` | Animal::ValueA327E0 | In outline: a value from entries, checking a slot against a count (+0x5C) |
| `0xb020c0` | Animal::CaseB020C0 | In outline: a step by an id's case (from 0xD776E) |
| `0xb89270` | Animal::PassB89270 | In outline: goes over entries not flagged (+0x48, +0x18) |
| `0xb0aa90` | Animal::StepB0AA90 | In outline: a step (`0x6A2BE0`) when a value is below 0 |
| `0xa61dc0` | Animal::AngleA61DC0 | In outline: a direction from angles (cosine of +0x34, sine of +0x38) |
| `0xb55960` | Animal::InsideB55960 | In outline: a step when a value is inside fixed bounds (`0xF6FD60`, `0xF6FD64`) |
| `0xb69c90` | Animal::FarB69C90 | In outline: a step over the terrain when a point is far enough |
| `0xa9f570` | PoolComplex::PassA9F570 | In outline: goes over entries (+0x28) not flagged |
| `0xb33d10` | Animal::CountB33D10 | In outline: a step with a part and no entry (+0x68) |
| `0x9ca2d0` | Fireworks::Find9CA2D0 | In outline: finds an entry among entries (+0xD0) |
| `0xa5f400` | Animal::TurnA5F400 | In outline: a turn step (sine of +0x20) when a direction is long enough |
| `0xb567f0` | Animal::FlatB567F0 | In outline: a flat length step, with no second point handled |
| `0xb1b140` | Animal::StepB1B140 | In outline: a step (`0x44E540`) with a part |
| `0xac8900` | Animal::IndexAC8900 | In outline: a step (`0xAC7D90`) and makes an index of four-byte entries |
| `0xa17580` | Bird::DeadZoneA17580 | In outline: a length step with a dead zone (Vector::DeadZone) |
| `0xab1450` | Animal::EntryAB1450 | In outline: a check (Animal::CheckA8B8B0) with a table entry (Table30::Entry with +0x13C) |
| `0xa27260` | Animal::WeatherA27260 | In outline: a step with the weather (Weather::Get) and a sine of a sum |
| `0xb1ad70` | Animal::StepB1AD70 | In outline: a step (`0x44E540`) when given a part |
| `0xa60fd0` | Animal::CompareA60FD0 | In outline: a step when a value is below a limit |
| `0xa609d0` | Animal::LengthA609D0 | In outline: a step when a direction is long enough |
| `0xb55b50` | WildAnimal::ItemWindow | In outline: builds the wild animal item window (WildAnimalItemWindow::Build) and clears a key combination |
| `0xaa4060` | PoolComplex::FlagAA4060 | In outline: a step when a table entry exists (IndexedTable::Entry) and a grid value has a flag (GridArea::ValueAt) |
| `0xa282d0` | Animal::NearestA282D0 | In outline: finds the nearest entry that passes a check |
| `0xb11320` | Animal::Database | In outline: makes database entries (WAIDatabaseEntry) for entries (+0x48) |
| `0xa326a0` | Animal::ValueA326A0 | In outline: a value from entries, checking a slot against a count (+0x5C) |
| `0xa32560` | Animal::ValueA32560 | In outline: the same (another copy) |
| `0xb6bd00` | Animal::WalkB6BD00 | In outline: a walk over points with values above 0 |
| `0xb5bbe0` | Animal::WalkB5BBE0 | In outline: the same (another copy) |
| `0xa0c0a0` | Bird::WalkA0C0A0 | In outline: the same (another copy) |
| `0xba9700` | Enclosure::StepBA9700 | In outline: a step (`0xBAF4D0`) with a handle |
| `0xb7c0f0` | Animal::TextB7C0F0 | In outline: a text by shared values (`0x1103B9C`, `0x1300494`), empty otherwise |
| `0xb82be0` | Animal::CountB82BE0 | In outline: with a part (+0xA4), goes over its entries (+0x50); 0 with none |
| `0xb34810` | PathGrid::MakeB34810 | In outline: sets up a 100-entry path table (`0x488290`), its table (`0xF99170`) and clears values |
| `0xb294d0` | WildAnimal::RunAway | In outline: makes a run-away locomotion (WAL_RunAway) and more when not flagged (+0xCC) |
| `0xaf39b0` | Animal::ClampAF39B0 | In outline: keeps n between 0 and its count minus one (+0x58) |
| `0xa60af0` | Animal::TurnA60AF0 | In outline: when a direction is long enough, turns a point (+0x50) about it (Vector::RotateAround) |
| `0xb4bea0` | WildAnimal::Crate | In outline: makes a wild animal crate (WildAnimalCrate) |
| `0x994830` | Fireworks::Clamp994830 | In outline: keeps a value between 0 and a limit (+0x110) |
| `0xb27680` | Animal::HandleB27680 | In outline: makes a handle and links it |
| `0xb930b0` | Animal::CheckB930B0 | In outline: makes a part (+0x44) and a check |
| `0xb8a850` | Animal::SetB8A850 | In outline: sets a flag (+0x70) and, when not flagged (+0x1A8), a step with its part (+0x194) |
| `0xb561c0` | Animal::LinkB561C0 | In outline: links its part (+0x84 +0x14) and a scenery value (`0x89C760`) |
| `0xb36d10` | Animal::FindB36D10 | In outline: finds an entry in a list (+0x2C) |
| `0xae32f0` | Animal::CopyAE32F0 | In outline: copies another object's values (+0x50, +0x44, +0x4C) |
| `0xab0b30` | Animal::LoopAB0B30 | In outline: a loop with an object (`0x13CE028`) and a list (+0x19C) |
| `0xb12460` | WildAnimal::DatabaseB12460 | In outline: sets up a record named "*:wai" with its table (`0xF97EF4`) |
| `0xafa6b0` | Animal::PassAFA6B0 | In outline: goes over entries (+0x1C) flagged (bit 1) |
| `0xaa52b0` | PoolComplex::SidesAA52B0 | In outline: when on (+0x140), a step with a pool piece's sides (PoolPiece::SideData) |
| `0xa90d00` | Fence::DistanceA90D00 | In outline: a distance from a point to a line between two points of a list |
| `0xa8a040` | Animal::TextA8A040 | In outline: a text (`0xAA08E0`) |
| `0xb39150` | WildAnimal::Tick4 | In outline: counts up (+0x25C) and every 4th time resets it and acts, unless the shared value is set (`0x1306C68`) |
| `0xb39040` | WildAnimal::Tick5 | In outline: the same every 5th time (+0x258) |
| `0xb38f30` | WildAnimal::Tick10 | In outline: the same every 10th time (+0x254) |
| `0xac1ed0` | Fence::StepAC1ED0 | In outline: a step (`0xAC1B90`) and a handle |
| `0xa897d0` | Animal::CountA897D0 | In outline: counts entries (+0xD0) |
| `0xa896b0` | Animal::CountA896B0 | In outline: the same (another copy) |
| `0x741740` | AnimalKeeper::GoToEnclosure | In outline: gets an enclosure in need (AnimalKeeper::NeedEnclosure) and sets the person's state to 0xD |
| `0xb2f380` | WildAnimal::SmallTurn | In outline: makes a small-turn decision (WAS_SmallTurn) |
| `0xabe8f0` | Fence::RectABE8F0 | In outline: goes over a rectangle's cells inside its bounds (+0x4C to +0x58) |
| `0xb718a0` | Animal::AddB718A0 | In outline: adds an entry to a list (+0x54), growing it (200 at first) when full |
| `0xb12260` | Animal::AddB12260 | In outline: grows two lists when full (Array::ResizeB11D40, `0xB10B80`) and adds |
| `0xba3f20` | Animal::FindBA3F20 | In outline: finds an entry among entries (+0x74, +0x78) |
| `0xaaaab0` | PoolComplex::Waypoints | In outline: makes a pool waypoint list (PoolWaypointList) for entry n (0x38 bytes) |
| `0xae9720` | Animal::PartAE9720 | In outline: with a part (+0x108) not flagged (+0xD4), a step |
| `0xb94e90` | Enclosure::StepB94E90 | In outline: a step with its part (+0x38 +0x104) and the shared value (`0x1305DD8`) |
| `0xb1f4d0` | WildAnimal::MoveSpline | In outline: makes a spline move (WALMoveSpline) with a flag (+0x111) |
| `0xa89f30` | Animal::TextA89F30 | In outline: makes an info text (`0xF31F48`) |
| `0x9d11b0` | Fireworks::Sort9D11B0 | In outline: sorts value pairs, two at a time |
| `0xa86710` | Animal::UpdateA86710 | In outline: an update when not flagged (+0x48) |
| `0xa11680` | Bird::CheckA11680 | In outline: a check when not flagged (+0x154) |
| `0xa11570` | Bird::CheckA11570 | In outline: a check when not flagged (+0x138) |
| `0xae16a0` | Animal::FindAE16A0 | In outline: finds an entry not flagged |
| `0xba04d0` | Animal::InitBA04D0 | In outline: sets up a record with handles |
| `0xb33290` | Animal::MakeB33290 | In outline: makes a part (+0x44) |
| `0xa85540` | Animal::ReleaseA85540 | In outline: releases a part (+0x2C) and makes a new one |
| `0xa32440` | Animal::ValueA32440 | In outline: a value from its part (+0x9C +0x3C) over entries (+0x5C) |
| `0xa32320` | Animal::ValueA32320 | In outline: the same with +0x38 |
| `0xa32200` | Animal::ValueA32200 | In outline: the same with +0x34 |
| `0xb72270` | Animal::ResetB72270 | In outline: a reset (`0x501FB0`), clears a flag (+0x27C) and a handle |
| `0xa1e8b0` | Window::AddA1E8B0 | In outline: when not found (`0xA1ACF0`), makes a 0xC-byte record |
| `0xaa18e0` | NamedObject::MakeAA18E0 | In outline: makes a list member object (ListMember with `0x10EC9E8`) |
| `0xaf18c0` | Animal::UpdateAF18C0 | In outline: an update when not flagged (+0x74) |
| `0xb675f0` | WildAnimal::ValueB675F0 | In outline: a value from its part (+0x17C) and a count (+0x158) |
| `0xa91d70` | Fence::QuadA91D70 | In outline: draws a quad (`0xA91C80`) from its corners |
| `0xb3e440` | Animal::FindB3E440 | In outline: a find over a list (+0x1D4) |
| `0xb2f490` | WildAnimal::Heading | In outline: a direction's heading (arctangent), kept positive |
| `0xa89af0` | Animal::CountA89AF0 | In outline: counts entries (+0xD0) |
| `0xa895b0` | Animal::CountA895B0 | In outline: the same (another copy) |
| `0xb70350` | Animal::FindB70350 | In outline: a find over entries (+0x60) |
| `0xb2c490` | Animal::TextB2C490 | In outline: a text from shared values |
| `0xb16890` | Animal::TextB16890 | In outline: a text by a mode (+0x78) |
| `0xa899f0` | Animal::CountA899F0 | In outline: counts entries (+0xD0) |
| `0xa898f0` | Animal::CountA898F0 | In outline: the same (another copy) |
| `0xa894c0` | Animal::CountA894C0 | In outline: the same (another copy) |
| `0xa7ec40` | Animal::PartA7EC40 | In outline: a step with its part (+0x44 +0xC) |
| `0xb1b270` | Animal::MakeB1B270 | In outline: makes a 0x150-byte part |
| `0xb2e670` | WildAnimal::CrossB2E670 | In outline: a cross product with an object's direction (WildAnimal::Interface60) |
| `0xb2e570` | WildAnimal::CrossB2E570 | In outline: the same (another copy) |
| `0xb242b0` | WildAnimal::TimeB242B0 | In outline: unless a value is -1.0 (+0xBC), a step against an object's value (+0x40) |
| `0xaaad90` | PoolComplex::GrowAAAD90 | In outline: grows a list (Array::ResizeAAABF0) when full |
| `0xb7eb90` | Animal::StepB7EB90 | In outline: a step by a flag (+0x7C) |
| `0xb669e0` | WildAnimal::DirectionB669E0 | In outline: a direction between two points |
| `0xa3e9e0` | Animal::CheckA3E9E0 | In outline: a check for an object with a slot (`0x12F5544`) |
| `0xb91120` | NamedObject::MakeB91120 | In outline: makes a list member object (ListMember with `0x10EC9E4`) |
| `0xb90b50` | Animal::ReleaseB90B50 | In outline: a step (`0x582A30` with 0) and releases its part (+0x280) |
| `0xafe3c0` | Animal::UpdateAFE3C0 | In outline: an update when not flagged (+0x170) |
| `0xba5380` | Animal::PassBA5380 | In outline: goes over entries (+8) |
| `0xac3510` | Fence::PassAC3510 | In outline: the same (another copy) |
| `0xab5f10` | PoolPiece::PlaceAB5F10 | In outline: when flagged (+0x11C bit 0), places a piece (PoolPiece::PlaceAB51D0 with 0x58) |
| `0xa39e50` | Animal::SwapA39E50 | In outline: replaces a shared part (+0xB4) |
| `0xaf11a0` | Animal::ValueAF11A0 | In outline: a value from entries |
| `0xae0890` | Animal::AverageAE0890 | In outline: an average point |
| `0xa8a150` | Animal::CheckA8A150 | In outline: a check over entries (+0xD0) |
| `0xb7c5f0` | Animal::TerrainB7C5F0 | In outline: a step when the terrain is not flagged |
| `0xb092d0` | Animal::UpdateB092D0 | In outline: an update when not flagged (+0x48) |
| `0xa9f360` | PoolComplex::FindA9F360 | In outline: unless flagged (bit 4), a step with a table entry (IndexedTable::Entry) |
| `0xae6690` | Animal::SetAE6690 | In outline: sets a flag (+0x60) and fills entry n (0x1C bytes) |
| `0xb88110` | Animal::CheckB88110 | In outline: 0 with no part (+0x74 +0x3C) |
| `0xb82750` | Animal::InitB82750 | In outline: sets up a record with its table (`0xF9BF84`) |
| `0xaa3e90` | PoolComplex::CopyAA3E90 | In outline: copies a record with a linked part |
| `0xb6f5e0` | Animal::UpdateB6F5E0 | In outline: an update when not flagged (+0xD0) |
| `0xb61420` | WildAnimal::PointB61420 | In outline: a step with a point from another (+0x50 on) |
| `0xa9a720` | PoolComplex::UpdateA9A720 | In outline: an update when not flagged (+0x1B0) |
| `0xa88c80` | Animal::PassA88C80 | In outline: goes over entries (+0xD4, +0xD8) |
| `0xb7b460` | Animal::StepB7B460 | In outline: a step (`0xB0E0F0`) and goes over entries |
| `0xb48240` | WildAnimal::SetB48240 | In outline: when not flagged (+0xC8), sets a point (+0xE0, +0xE4) |
| `0xb62970` | WildAnimal::TimeB62970 | In outline: when not flagged (+0xFD), adds a time to a value (+0x44) |
| `0xb69f20` | Animal::StepB69F20 | In outline: with a value (+0x178), a step (`0xB0E0F0`) |
| `0xb24540` | Animal::StepB24540 | In outline: the same with +0x9C |
| `0xac2750` | Fence::StepAC2750 | In outline: a step with many values |
| `0xaa3f80` | PoolComplex::SetAA3F80 | In outline: sets a flag and a linked part |
| `0xa808b0` | Animal::CaseA808B0 | In outline: a table entry (Table30::Entry) and a step by a mode (+0x158) |
| `0xb88c30` | Animal::CheckB88C30 | In outline: sets a result to 3 and a check |
| `0xac6320` | PoolComplex::UpdateAC6320 | In outline: an update when not flagged (+0x1C) |
| `0xb478c0` | WildAnimal::CheckB478C0 | In outline: 0 unless a check passes (slot 0x24) |
| `0xae09f0` | Animal::CheckAE09F0 | In outline: 0 with no part (+0x50) |
| `0xa5f5e0` | Animal::CopyA5F5E0 | In outline: copies values from its owner (+4 +0x160, +0x140) |
| `0xb45fa0` | WildAnimal::SetB45FA0 | In outline: sets a flag (+0xAC) and makes a part with no slot (+0x78) |
| `0xaabc60` | PoolComplex::MakeAABC60 | In outline: sets a byte (+4) and makes a part |
| `0xaa5910` | PoolComplex::PassAA5910 | In outline: when given a value, goes over entries (+0x27C) |
| `0xaa0680` | PoolComplex::PassAA0680 | In outline: goes over entries (+0x28) |
| `0xa9f7e0` | PoolComplex::PassA9F7E0 | In outline: the same (another copy) |
| `0xb87370` | Animal::MakeB87370 | In outline: makes a record |
| `0xb556f0` | Animal::MakeB556F0 | In outline: makes a 0x68-byte part |
| `0xb9afb0` | PathGrid::ModeB9AFB0 | In outline: with a value of 0.4, a step for parts in modes 1 to 3 (+0x414) |
| `0xb81e50` | Animal::SumB81E50 | In outline: a step with the sum of three values (+0xA4 +8 on) |
| `0xb6ae80` | Animal::ClearB6AE80 | In outline: clears a flag (+0x58) and acts with a part (+0x64) |
| `0x9fea10` | Animal::Add9FEA10 | In outline: adds an entry, growing a list (+0x5C, +0x60) when full |
| `0xb93490` | Animal::PartB93490 | In outline: a part from +0x48 when it has none (+0x68) |
| `0xb235c0` | WildAnimal::SexAgeCheck | In outline: compares two sex and age texts (WildAnimal::SexAgeText) |
| `0xb18e70` | WildAnimal::PointB18E70 | In outline: a step with its part (+0x108) |
| `0xaf48f0` | Animal::FloatAF48F0 | In outline: makes a float blend element (FloatBlenderElement) and acts when not flagged (+0x7C) |
| `0xaa5400` | PoolComplex::CaseAA5400 | In outline: a step by case (0: `0xAA3970` on +0x140, ...) |
| `0xb891a0` | Animal::EntryB891A0 | In outline: a step for entry n (0x1C bytes at +0x60) not flagged |
| `0xb33590` | Animal::TextB33590 | In outline: unless a flag is set (Flags::Bit0), makes a text |
| `0xa905f0` | Fence::CrossA905F0 | In outline: a cross product of two directions |
| `0xa86e50` | Animal::UpdateA86E50 | In outline: updates (Animal::UpdateA86710 with an object) and acts by a value (+0x120) |
| `0xa705b0` | Animal::Overlap | In outline: a large step for two objects whose boxes overlap: sets up shared 32-entry tables once (`0x13C3110`) and adds up their parts' values |
| `0xafb740` | Animal::TerrainAFB740 | In outline: a large step over terrain cells: makes two 0x400-byte buffers and reads three neighbouring slots (Animal::SlotAFB160) |
| `0xa6d7b0` | Animal::PoseA6D7B0 | In outline: a large pose by kind (1, ...): blends points by a shared fraction (`0x13429E0`) and works out a normalised direction |
| `0xb29a70` | WildAnimal::TerrainB29A70 | In outline: a large step with the terrain's bounds and the enclosure manager's cell |
| `0xb792d0` | Animal::NearestB792D0 | In outline: a large find of the nearest (from the largest float) with the shared object (`0x12F468C`), made when none |
| `0xaf7b50` | Animal::TrackAF7B50 | In outline: a large step for entry n that is on: reads a track's ends (`0xBC5AB0`, `0xBC5B40`) |
| `0xb8a970` | Animal::PassB8A970 | In outline: with the shared value (`0x12F54C0`), goes over entries (+0x74 +0x24) |
| `0xa720b0` | Animal::Overlap2 | In outline: a large step for two objects whose boxes overlap (another copy of Animal::Overlap with `0x13C3820`) |
| `0xa72f80` | Animal::Overlap3 | In outline: the same with a 32-entry shared table (`0x13C893C`) filled with the largest float |
| `0xb64800` | WildAnimal::StepB64800 | In outline: a large step with a part's values (+0x60 on) and two values (`0xB2E960`) |
| `0xb3d250` | WildAnimal::CellB3D250 | In outline: an animal's cell (Grid::CellAt) inside the enclosure manager's grid |
| `0xb66290` | WildAnimal::StepB66290 | In outline: a large step with a point and a part's values (+0x50 on) |
| `0xa6f3a0` | Animal::NormalsA6F3A0 | In outline: works out normals for its triangles (+0x5C, +0x60) |
| `0xa97f70` | Animal::UpdateA97F70 | In outline: an update when not flagged (+0x78) with a part |
| `0xa60590` | Animal::CopyA60590 | In outline: copies values from its owner (+4 +0x160, +0x120, +0x140) |
| `0xa874d0` | Animal::UpdateA874D0 | In outline: an update when not flagged (+0x28) |
| `0xa7c190` | Animal::TextA7C190 | In outline: an empty text filled when not flagged (+0x184) |
| `0xa7a100` | Animal::PassA7A100 | In outline: goes over entries (+0xC, +0x10) with a part's list (+0x40, +0x48) |
| `0x992fd0` | Fireworks::CopyKey | In outline: makes a firework key frame (FireworkSlotKeyFrame) and copies a value (+0x48) |
| `0xa7c440` | Animal::CheckA7C440 | In outline: a check when not flagged (+0x1E0) |
| `0xb87eb0` | Animal::CheckB87EB0 | In outline: a check when not flagged (+0xB8) |
| `0xb828d0` | Animal::AddTimeB828D0 | In outline: adds a time to each entry (+4, +8) and acts |
| `0xa873a0` | Animal::UpdateA873A0 | In outline: an update when not flagged (+0x80) |
| `0xb893a0` | Animal::CheckB893A0 | In outline: a check with a value from n over a count (+0x54) |
| `0xba4890` | Enclosure::UpdateBA4890 | In outline: when its bounds are in order, updates (Enclosure::UpdateBA4550, `0xBA1E40`) and sets a slot to -2 |
| `0xb98870` | PathGrid::RectB98870 | In outline: keeps a rectangle inside the grid (+0x64, +0x68) |
| `0x81c7f0` | SceneryBehaviour::Ref81C7F0 | In outline: takes a reference and acts when not flagged (+0x1C) |
| `0xb9aae0` | WildAnimal::TerrainB9AAE0 | In outline: a step with an object's value (+0x30) when the terrain is not flagged |
| `0xa89180` | Animal::EntryA89180 | In outline: a step for entry n (0x1C bytes at +0x8C) not flagged |
| `0xb4cdc0` | WildAnimal::CaseB4CDC0 | In outline: for 0, tells three parts (+0x38, +0x8C, +0xE0) |
| `0x9d5970` | Window::Link9D5970 | In outline: with a value (+0xDC), a step with a linked part (+0xD0) |
| `0xaa9250` | PoolComplex::UpdateAA9250 | In outline: an update when not flagged (+0x10C) |
| `0x846b20` | Scenery::Check846B20 | In outline: 1 when flagged (+0xA8), else a check with its part (slot 0x44) |
| `0xaa91d0` | PoolComplex::StepAA91D0 | In outline: a step when not flagged (+0x10C) |
| `0xaa9150` | PoolComplex::StepAA9150 | In outline: the same (another copy) |
| `0xaa90d0` | PoolComplex::StepAA90D0 | In outline: the same (another copy) |
| `0xa8ebe0` | Animal::AtLeast10 | In outline: makes a list at least 10 long (`0x417C20`) and fills new entries |
| `0xb856e0` | Animal::KindB856E0 | In outline: a value by a kind (+0x61: 2, ...) within a count |
| `0xaa8a40` | PoolComplex::CheckAA8A40 | In outline: a check with a part's object (by a shared id `0x130BBEC`) |
| `0xa84ac0` | Animal::CaseA84AC0 | In outline: for 1, tells parts (+0x5C, +0xB0) |
| `0xb42390` | WildAnimal::ClampB42390 | In outline: when flagged (bit 2), keeps a value between two others (+0x1E0, +0x1E8) |
| `0xb8b9f0` | Animal::TextB8B9F0 | In outline: takes a reference to an info text (Info::TextB87790) |
| `0x9fe2a0` | Fireworks::Remove9FE2A0 | In outline: for entry n (0x30 bytes) that is on, removes it from an active list (ActiveList::Remove) |
| `0xaae350` | PoolComplex::ValueAAE350 | In outline: a value by a mode (+0x130) |
| `0xaae430` | PoolComplex::PointAAE430 | In outline: a point step when the terrain is not flagged |
| `0xb94010` | Enclosure::LinkB94010 | In outline: a step with a linked part (+0x70) |
| `0xb44ef0` | WildAnimal::ValueB44EF0 | In outline: a value from its part (+0x54 +0x54) when flagged, else from another |
| `0xa6bed0` | Animal::EntryA6BED0 | In outline: goes over entry n's list (+0x4C, 0xC bytes) |
| `0xba9fc0` | Enclosure::StepBA9FC0 | In outline: a step by a check (slots 0x3C, 0x54) |
| `0xba9f60` | Enclosure::StepBA9F60 | In outline: the same (another copy) |
| `0xb7aad0` | Animal::CheckB7AAD0 | In outline: when on (+0x41) and not flagged (+0x40), a check (`0xB7A480`) |
| `0xa10e90` | Bird::FaceA10E90 | In outline: clears a value (+0xC0) and points (Effect::FaceA0D300) |
| `0xaacec0` | PoolComplex::CaseAACEC0 | In outline: for 1, a value by case |
| `0xbad890` | Animal::TextBAD890 | In outline: an empty text filled from entries (+0x198) |
| `0xb2d560` | WildAnimal::CheckB2D560 | In outline: a check when on (+0xB4) and not flagged (+0xAC) |
| `0xa5f2c0` | Animal::SetA5F2C0 | In outline: with no part (+4) and given one, sets values |
| `0xa2e200` | Animal::CheckA2E200 | In outline: for 0, a check when not flagged (+0x30) |
| `0xb25d90` | WildAnimal::MakeB25D90 | In outline: makes a 0x100-byte part |
| `0xb0f6b0` | Animal::GetB0F6B0 | In outline: when not flagged (+0x154), takes a value |
| `0xa6faf0` | Animal::EntryA6FAF0 | In outline: a step for entry n under a third of a count (+0x4C) |
| `0x9e2f10` | Fireworks::Count9E2F10 | In outline: a step under a count (+0x38, +0x3C) |
| `0x99c730` | Fireworks::Step99C730 | In outline: a step over entries |
| `0x99c660` | Fireworks::Step99C660 | In outline: the same (another copy) |
| `0xb70be0` | Animal::UpdateB70BE0 | In outline: an update when not flagged (+0xAC) |
| `0xa938b0` | Fence::BytesA938B0 | In outline: a step with bytes (+0x15C, +0x15E) |
| `0x968f20` | Fireworks::Count968F20 | In outline: a step with no entries (+0x14) handled |
| `0x968e50` | Fireworks::Count968E50 | In outline: the same (another copy) |
| `0x7461b0` | Guest::Pass7461B0 | In outline: goes over entries (+0xD4 +0x18) |
| `0xaf4820` | Animal::FloatAF4820 | In outline: makes a float blend element and acts when not flagged (+0x60) |
| `0xad8810` | Animal::FloatAD8810 | In outline: the same when not flagged (+0x74) |
| `0xa14cc0` | Bird::CheckA14CC0 | In outline: a check when not flagged (+0x128) |
| `0xa14bf0` | Bird::CheckA14BF0 | In outline: the same (+0x144) |
| `0xa114a0` | Bird::CheckA114A0 | In outline: the same (+0x138) |
| `0xa113d0` | Bird::CheckA113D0 | In outline: the same (+0x154) |
| `0xb12d40` | Animal::PartB12D40 | In outline: a step with a part (+0x84) |
| `0xae1550` | Animal::PassAE1550 | In outline: goes over entries (+4) |
| `0xab3a80` | Animal::EntryAB3A80 | In outline: for entry n (24 bytes) under a count (+0x15C), a check; 0 outside |
| `0xa97640` | Animal::UpdateA97640 | In outline: an update when not flagged (+0x78) |
| `0xa9ff20` | PoolComplex::UpdateA9FF20 | In outline: an update when not flagged (+0x34) |
| `0xa32910` | Animal::CountA32910 | In outline: a count from its part (+0x9C) |
| `0xb47d40` | WildAnimal::LightsB47D40 | In outline: updates lights (Model::UpdateLights) and adds a time (+0x140) |
| `0xa97860` | Animal::EntryA97860 | In outline: for entry n (0x1C bytes) under a count (+0x84), a check; 0 outside |
| `0x84a8d0` | Scenery::Check84A8D0 | In outline: a check when not flagged (+0xC8) |
| `0xb12070` | Animal::ClearB12070 | In outline: clears a linked value (+0x48) and more |
| `0xb88390` | Animal::UpdateB88390 | In outline: an update when not flagged (+0xD4) |
| `0xb562e0` | Animal::PassB562E0 | In outline: goes over a part's entries (+0xBC +0x24) |
| `0xb3ad10` | Animal::SetB3AD10 | In outline: when given a value and none is set (+0x1FC), sets it |
| `0xae8bb0` | Animal::ReleaseAE8BB0 | In outline: releases a part (+0x108) |
| `0xacc280` | Graph::ReleaseACC280 | In outline: releases a part (+0x10C) |
| `0xa9fe50` | PoolComplex::UpdateA9FE50 | In outline: an update when not flagged (+0x34) |
| `0xaa3970` | PoolPiece::SidesAA3970 | In outline: when on, a pool piece's sides (PoolPiece::SideData) |
| `0xa62c90` | Animal::InitA62C90 | In outline: clears a value (+0x360) and sets another to 40 (+0x364) |
| `0x92eb70` | Water::Reset92EB70 | In outline: resets (Static::Reset) and makes parts |
| `0xabea00` | Fence::FlagsABEA00 | In outline: when its part matches, adds flags (+0x4C) |
| `0xaa1b80` | PoolComplex::SetAA1B80 | In outline: grows a list when n passes its count (Array::ResizeAA17A0) and sets |
| `0xa973b0` | Animal::EntryA973B0 | In outline: for entry n under a count (+0x84), a step |
| `0xb90900` | Animal::HandleB90900 | In outline: makes a handle and links it |
| `0xb539b0` | Animal::UpdateB539B0 | In outline: an update when not flagged (+0x74) |
| `0xa8aa90` | Animal::CountA8AA90 | In outline: with no entries (+0x68 +4), a step |
| `0xa7bec0` | Animal::CheckA7BEC0 | In outline: a check when not flagged (+0x1C4) |
| `0xb848f0` | Animal::FindB848F0 | In outline: a find over entries (+8) |
| `0xabae10` | Animal::ModeABAE10 | In outline: a step with no part (+0x120) |
| `0xa88180` | Animal::ValueA88180 | In outline: a value from two counts |
| `0xb7bc40` | Animal::UpdateB7BC40 | In outline: an update when not flagged (+0x90) |
| `0xb13ea0` | Animal::ResetB13EA0 | In outline: clears a value (+0x94) and acts with a part (+0x84) |
| `0x99ff30` | Fireworks::Part99FF30 | In outline: a step with a part (+8) |
| `0xb885c0` | Animal::UpdateB885C0 | In outline: an update when not flagged (+0x1A8) |
| `0xb54970` | Animal::MakeB54970 | In outline: makes a 0x1C-byte part |
| `0xb53da0` | Animal::HandleB53DA0 | In outline: makes a handle (`0xF9A998`) and acts |
| `0xafb0a0` | Animal::CountAFB0A0 | In outline: at 2000 (+0x54), a step (`0xAFB000`) |
| `0xaf7a10` | Animal::CountAF7A10 | In outline: at 32000 (+0x1C), a step (`0xAF7980`) |
| `0xa91180` | Animal::StepA91180 | In outline: a step with points |
| `0xa704f0` | Animal::StepA704F0 | In outline: a step with many values |
| `0x9ec540` | Fireworks::Mode9EC540 | In outline: in mode 2 (+0x70), a step |
| `0xb6c660` | Animal::CountB6C660 | In outline: a count over entries (+0x48) |
| `0xb91b40` | Animal::MakeB91B40 | In outline: makes a part |
| `0xb87f70` | Animal::CheckB87F70 | In outline: a check when not flagged (+0xD4) |
| `0xb7be40` | Animal::UpdateB7BE40 | In outline: an update when not flagged (+0x90) |
| `0xb3ac30` | Animal::InitB3AC30 | In outline: fills a record from its arguments |
| `0xb21ea0` | WildAnimal::CountB21EA0 | In outline: a step with a count (+0xE4) plus one |
| `0xa5dfd0` | Animal::SizeA5DFD0 | In outline: a step with a size (+0x80, +0x88) |
| `0xa329e0` | Animal::CountA329E0 | In outline: a count over entries (+0x9C) |
| `0xb7aa10` | Animal::HandleB7AA10 | In outline: sets a flag (+0x41) and makes a handle |
| `0xb34bf0` | WildAnimal::CheckB34BF0 | In outline: a step when not flagged (+0x47) and more |
| `0xa9cbb0` | PoolComplex::MakeA9CBB0 | In outline: makes a part, with none handled |
| `0xb9e430` | Animal::DrawB9E430 | In outline: a draw step in green (0x7F00FF00) |
| `0xb45460` | WildAnimal::BitsB45460 | In outline: a count over bits (+0x78) |
| `0xb453a0` | WildAnimal::BitsB453A0 | In outline: the same (another copy) |
| `0xafdec0` | Animal::ResetAFDEC0 | In outline: when on (+0x34), turns it off and acts |
| `0xaf3580` | Animal::AddAF3580 | In outline: grows a list by one (ObjectArray::SetCapacity on +0x58) and a handle |
| `0xac9ed0` | Graph::ScaleAC9ED0 | In outline: a step with a value, below 8 handled |
| `0xb21de0` | WildAnimal::UpdateB21DE0 | In outline: an update when not flagged (+0x94) |
| `0xafdcb0` | Animal::StepAFDCB0 | In outline: with none given and no part (+0x1090), a step |
| `0xa8b5d0` | Animal::PassA8B5D0 | In outline: goes over entries (+0xD8) |
| `0x9c7510` | Fireworks::Pass9C7510 | In outline: goes over entries (+0x9C) |
