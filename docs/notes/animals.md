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
