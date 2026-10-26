# Park



| Address | Name | What it does |
|---|---|---|
| `0x705200` | Park::RatingBreakdown | Works out the park rating and its parts, shown in the rating breakdown: base, number of guests, happy guests, lost guests, ride up-time, excitement and intensity, litter and vomit, penalties, bonus and total (`ParkRatingBreakdown_*`). Called from `0x6EF5D0`, `0x707070` and `0xA41870` |
| `0x6f3750` | VIPCondition::Describe | Writes a VIP condition's text (virtual slot 12 of VIPCondition): when the VIP arrives and what they want to visit (a pool, fireworks, a themed area, 1, 2, 3 or X roller coasters `VIPCondition_WithExcitementRatingOf` `%.2f`, viewing galleries, ...) |
| `0x6d1540` | CoasterCondition::Describe | Writes a coaster (or flume) condition's text (virtual slot 12 of CoasterCondition): the park must have at least 1, 2, 3 or N coasters / flumes, all of them, ... (`CoasterCondition_*`, with `%CLASS%` replaced) |
| `0x6d6ca0` | ObjectiveCondition::DescribeTiming | Writes the timing part of an objective's text: at any time, for N / 3 / 2 / 1 months, tested from ... (`ObjectiveCondition_AtAnyTime`, `_ForManyMonths`, `_For3Months` ... `_TestedFrom`). Called by `CoasterCondition::Describe` and the other conditions' texts (12 callers) |
| `0xa50640` | NewGame::Start | Starts a new game of a kind: a scenario (`New Scenario.dat` from `Start New Scenarios` / `Scenarios`), a sandbox (`New Sandbox.dat`), a blank landscape (`Campaigns\Base\BlankLandscape.dat`) or the coaster designer (`Campaigns\Base\CoasterDesigner.dat`) |
| `0xa432a0` | CareerMap::BuildScenario | Builds a scenario's entry on the career map: its title and text (`Scenario`, `_Title`, `_Text`), the large picture and its foreground (`CareerMap_LargeImage_`, `_LargeImageFG_`), the front-screen people and the slideshow (`\Slideshow\`). Called from `0xA44140` and `0xA44B00` |
| `0xa59430` | Tutorial::List | Lists the tutorials (`.\Tutorials\*.dat`): each one's name, description and icon (`List_Name`, `List_Description`, `Icon`), with the expansion's versions (`_AOP`, `_Alternate`). Called from `TutorialPanel::Build` |
| `0x6c5210` | ChallengePrize::Describe | Writes the prizes of a challenge: one or several, cash, research boost, publicity, an unlocked animal (`ChallengePrize_*`) |
| `0x6e0140` | Condition::ItemName | Writes the name of a shop item or stall in a condition's text, or `Unknown item`, `Unknown stall`, `<missing data>` |
| `0x6f7ac0` | VIPCondition::Notice | Gives the notice that a VIP is coming in a week or a month (`VIPCondition_WeekNotice`, `_MonthNotice`) |
| `0x554130` | Objective::Tooltip | Writes an objective's tooltip (`Invalid ToolTip` when it has none, `ObjectiveCondition_TestedFrom0`). Called from `0x554A30` |
| `0x6e3ec0` | AnimalSalesProfitCondition::Describe | Writes the current animal sales profit in a condition's text (`AnimalSalesProfitCondition_Current`), with its timing (`ObjectiveCondition::DescribeTiming`) |
| `0x82bdd0` | Scenery::CantDelete | Says why a scenery item can't be deleted: a tree, or land not owned (`Scenery_CantDelete_TreeHelp`, `NotOwned`). Called from `0x8468F0` and `0xABB3C0` |
| `0x6e28c0` | NumAnimalsCondition::Describe | Writes a number-of-animals condition's text: the current number, counting bought and bred or only bred animals (`NumAnimalsOfSpeciesCondition_*`) |
| `0xa55460` | Campaign::ProfileName | Writes a career profile's folder and name (`Campaign %04d`, `PlayerProfile_Player`). Called from `0xA57210` |
| `0x6d6ac0` | Objective::Messages | The objective messages: a condition met, a week's or a month's warning, try again next year (`ObjectiveMessage_*`) |
| `0x6dfb50` | NumItemsBoughtCondition::ModeName | Writes the mode of an items-bought condition: item and stall type, stall type, a specific item (`NumItemsBoughtCondition_Mode*`, or `Unknown mode!`). Called from a window builder |
| `0x6df770` | NumSceneryCondition::ClassificationName | Writes the kind of scenery a condition counts: flowers, shrubs, trees (`ObjectFilter_TT_Scenery_*`, or `UNKNOWN CLASSIFICATION!`). Called from two places |
| `0x6c4d80` | Challenge::StateName | Writes a challenge's state: not started, prompting the player, in progress, completed, failed, postponed, declined. Called from `0x6C85F0` |
| `0x6f1e80` | VIPCondition::Result | Writes whether a VIP's visit succeeded or failed (`VIPCondition_Success`, `_Failure`). Called from `0x7F8C80` |
| `0xee6240` | ParkThemes::Names | Makes the names of the park landscape themes at start-up: mountain, desert, snow, hill, island, spooky, space, city |
| `0x6c61e0` | Challenge::DefaultText | Gives a new challenge its default completion text (`Challenge_DefaultCompletionText`). Called from `0x6C81B0` |
| `0x707230` | SandboxScenario::Name | Writes a sandbox scenario's name (`SandboxScenario_Name`, `#SANDBOX#`, `#ME#`). Called from five places |
| `0x6bdba0` | Money::Format | Writes an amount of money in the player's currency (`Money`). `0x6BD830` and `0x6BD4C0` are the other formats |
| `0x6bd830` | Money::Format (2) | Another format of an amount of money (`Money`). Called from four places |
| `0x6bd4c0` | Money::Format (3) | Another format of an amount of money (`Money`). Called from two places |
| `0x85db30` | Placement::Footprint | Works out the ground an object takes up on the terrain grid for its kind of shape (+8) and rotation (0 to 3), a 64-case switch, using the terrain (`Terrain::Get`). Called from 15 places (scenery, paths, rides) |
| `0x832c30` | SceneryItem::Activate | A scenery item's activation (virtual slot 3, run once by `ManagedObject::Activate`): activates its definition (`0x8826F0`), works out the ground it takes up (`Placement::Footprint`) and sets up its effects (`SceneryItem::SetupEffects`), with a random variation |
| `0x72ba70` | PathTile::Activate | A path tile's activation (virtual slot 3): links it into the path network (`PathManager`) and fits it to the terrain |
| `0x79c1a0` | Park::CountBelow | Counts the entries of the ring of object pointers at `0x1306BC8` whose value at +0xB0 is below the given limit, leaving out those with flag 0x20 at +0xB4 when asked. Called from Park::RatingBreakdown |
| `0x83fbc0` | SceneryItem::RemoveLink | Takes the given object out of the item's list (+0x54, count +0x58), moving the later ones down. Called from SceneryItem::~SceneryItem |
| `0x6e9a40` | Objectives::Find | The objective in slot 0-7 of group 0-2 (the table at +0x28; the middle index can only be 0), or null when out of range or empty. Used by Objective::~Objective and Vendor::Sell |
| `0x4417a0` | GameTime::Update | Advances the game clock by the frame's time (+0xC of the time step): the elapsed time (+8), the time of day (+0xC, by 1/576 of the step, kept in range by `0x4413D0`; +0x10 a copy, or `0xDDD39A`'s value while `0x10ED848` is set). Each time the elapsed time since +0x14 passes a whole 90 seconds it runs every member of update list 1 (`0x10EC780`, virtual slot 0), every 4 such periods (360 s) list 2 (`0x10EC798`), every 32 (2,880 s) list 3 (`0x10EC7B0`). The update function of GameTime's table |
| `0x79d7c0` | Litter::Update | Moves a piece of litter each frame: while it is falling (flag 4 at +0x74) its upward speed (+0x84) drops by 12 per second and its position (+0x10) moves by its velocity (+0x80), then `0x79C2E0`; otherwise a downward speed below -0.5 is brought back by 20 per second, and anything else is set to 0. The update function of Litter's table |
| `0x845440` | PartedObject::Slot5 | Virtual slot 5 of objects made of parts (attractions, stalls, animal shelters, ...): passes the call with its two arguments on to slot 5 of every part, through a request object (table `0xF77BB8`) handed to the object's slot 32, which visits the parts |
| `0x8459a0` | PartedObject::Slot8 | Slot 8 of objects made of parts: asks slot 8 of each part (request table `0xF77BD4`) and gathers the values the parts give into the outputs passed in; returns whether any part answered |
| `0x845b30` | PartedObject::Slot10 | Slot 10 of objects made of parts: true when slot 10 of any part is true (request table `0xF77BE0`, stopping at the first) |
| `0x845bb0` | PartedObject::Slot11 | Slot 11 of objects made of parts: the slot 11 value of the first part whose slot 10 is true (request table `0xF77BE4`) |
| `0x845dd0` | PartedObject::Slot19 | Slot 19 of objects made of parts: the flags of slot 19 of every part combined with OR (request table `0xF77BF4`) |
| `0x845520` | PartedObject::Slot20 | Slot 20 of objects made of parts: passes the call with its six arguments on to slot 20 of every part (request table `0xF77BC4`) |
| `0x845730` | PartedObject::Slot21 | Slot 21 of objects made of parts: passes the call with its ten arguments on to slot 21 of every part (request table `0xF77BCC`); true only when every part returns true |
| `0x845600` | PartedObject::Slot22 | Slot 22 of objects made of parts: passes the call with its nine arguments on to slot 22 of every part (request table `0xF77BC8`) |
| `0x845b00` | PartedObject::AnySlot10Request | The slot 10 request's visit: marks it found when the part's slot 10 is true; returns whether to go on (not yet found) |
| `0x845b80` | PartedObject::FirstSlot11Request | The slot 11 request's visit: when the part's slot 10 is true keeps its slot 11 value and stops; otherwise goes on |
| `0x845db0` | PartedObject::OrSlot19Request | The slot 19 request's visit: ORs the part's slot 19 flags into the result; always goes on |
| `0x8456e0` | PartedObject::Slot21Request | The slot 21 request's visit: calls the part's slot 21 with the kept arguments, clearing the result when it returns false |
| `0x4bee20` | AttractionInstance::TypeKindIs0Or21 | True when the first byte of the attraction's type (+0xA8) is 0 or 0x15, false otherwise. Shared by the attraction instance classes (slots 14, 15, 50) |
| `0x4bef90` | AttractionInstance::SetValue70 | Sets the value at +0x70 when it changes and then calls the object's slot 50; returns 1 when it changed. Shared by the attraction instance classes (slots 18, 19, 54) |
| `0x8451d0` | PartedObject::Slot1 | Slot 1 of objects made of parts (pool complexes, animal shelters): passes the call with its two arguments on to slot 1 of every part (request table `0xF77BA4`, its visit `0x8451B0`), through the object's slot 32 |
| `0x845240` | PartedObject::Slot2 | The same for slot 2 (request table `0xF77BA8`, visit `0x845220`) |
| `0x8452b0` | PartedObject::Slot3 | The same for slot 3 (request table `0xF77BAC`, visit `0x845290`) |
| `0x845320` | PartedObject::Slot4 | The same for slot 4 (request table `0xF77BB0`, visit `0x845300`) |
| `0x845c30` | PartedObject::Slot27 | Slot 27 of objects made of parts: the sum of what slot 27 of every part gives for the argument (request table `0xF77BE8`, visit `0x845C00`) |
| `0x4a9210` | AnimatedRideInstance::Slot30 | Asks slot 30 of each part that has an object (`0x846AE0`), starting from the value passed in, and keeps the largest value any of them gives (request table `0xF30A7C`, visit `0x4A8F70`); when one answered, writes it back and returns 1 |
| `0x6d6340` | ValueCondition::IsMet | Whether a value condition is met: its current value (slot 26) compared with its target (+0x70) by its mode (+0x74): at most the target when the mode is 0, at least the target otherwise. Shared by the value conditions (animal sales profit, enclosure visitors, items bought, ...; slot 23) |
| `0x4bf1c0` | AttractionInstance::StoreSlot51 | Keeps what the object's slot 51 gives at +0xA4, at least 0. Shared by the attraction instance classes |
| `0x6d8530` | NumWellThemedAttractionsCondition::IsMet | Whether the number of well-themed attractions (`0x4CD9A0` for the theme +0x70 and +0x78) reaches the target (+0x74). Slot 23 of the condition |
| `0x6d8990` | NumAnimalsOfSpeciesCondition::Value | The condition's current value: 0 when no species is chosen (+0x88 is -1), otherwise the number of animals of it, counted by `0xB6C2E0` into +0x8C. Slot 26 of the condition |
| `0x704e20` | Park::BumpTerrainCount | Adds one to the terrain's counter at +0x58, then `0x704A50`. Slots 3-5 of Park |
| `0xabc210` | PoolSegment::Matches | True when the given object's +0x120 is 0 and its +0x138 is this segment's +4. Slots 5, 15 and 16 of PoolSegment |
| `0x7290a0` | PathQueue::Slot7 | Hands its two arguments to `0x726D60` when the queue's kind (slot 12) is 3, otherwise to `0x728A50`. Slots 7 and 8 of PathQueue |
| `0x4c14b0` | AttractionInstance::AddToList | Adds the given object to the end of the attraction's list at +0xCC (a new 12-byte node). Shared by the attraction instance classes (slots 30, 31, 66) |
| `0x4c0680` | AttractionInstance::Slot12 | While `0x12F54C0` is set: runs `0x58F740`, then calls slot 9 of every object in the attraction's list at +0xCC. Shared by AttractionInstance classes |
| `0x4aede0` | RideInstance::Slot57 | What the ride's slot 33 gives times `0x4BFA20`'s value when it is positive, else 0 (the same as ChangingRoomInstance's `0xA7B3B0`). Shared by the ride instance classes |
| `0x52fa30` | RideInstance::Get294Plus4 | Returns the value at +4 of the object at +0x294. Shared by the ride instance classes (slots 94, 95, 130) |
| `0x52fa70` | RideInstance::SumOfItems | Adds up, over the items the ride's slot 55 gives for each index below its slot 78 count, a quarter of each item's slot 11 value. Shared by the ride instance classes |
| `0x4bf190` | AttractionInstance::Slot51 | The type's value at +0x20 divided by 32, less `0x7F49C0`'s value divided by 4. Shared by the attraction instance classes |
| `0x984000` | ResearchCategoryInstance::Percent | The category's progress (+0x80) as a whole percentage (times 100, rounded). Slots 4 and 12 of ResearchCategoryInstance |
| `0x9843b0` | ResearchCategoryInstance::TitleText | The value of the text `Man_Research` (looked up through `0x41CB90`). Slot 3 of ResearchCategoryInstance |
| `0x506320` | RideCarCrash::Reset | When a crash is under way (+0xF0), clears it and its value at +0xDC. Slots 3 and 4 of RideCarCrash |
| `0x786d20` | GuestInjectionPoint::Register | Registers the object in a slot pool (SlotPool::Add, `0xCDEE80`), keeping the handle it gives at +0x58, then `0x833890`. Slots 9 and 10 of GuestInjectionPoint |
| `0x819200` | LitterBinBehaviour::Register | The same registration for a litter bin (handle at +0x58). Slot 9 of LitterBinBehaviour |
| `0x818f80` | LitterBinBehaviour::Unregister | When it has a handle (+0x58 not negative), releases it (SlotPool::Remove, `0xCDEDE0`), sets it to -1 and runs `0x8338B0`. Slot 11 of LitterBinBehaviour |
| `0x40ddc0` | ManagedObject::Slot19 | Calls `0x771F90` with 0, then deletes the object kept at +0x2C (its deleting destructor with 1) and clears it. Slot 19 of ManagedObject |
| `0x40df10` | ManagedObject::Slot28 | Calls `0x771E50` with 0, then deletes the object at +0x2C and clears it. Slot 28 of ManagedObject |
| `0x87ef00` | SIDDatabaseEntry::ScaledValue | Ten times what the entry's slot 0 gives, at most 10,000. Slot 4 of SIDDatabaseEntry |
| `0x87f270` | SIDDatabaseEntry::Kind | 0x13 when `0xBD1420` with 0 is true and bit 25 of the entry data's +4 is set, otherwise 0x12. Slots 4 and 10 of SIDDatabaseEntry |
| `0x58fdc0` | StallInstance::Slot54 | A quarter of the first item's slot 11 value (the item the stall's slot 55 gives for 0), or 0 when there is none. Slot 54 of StallInstance |
| `0x585af0` | SpecialAttractionInstance::Slot57 | When the attraction's type kind is 0xB, `0x4BFA20`'s value times +0x1F0; otherwise 0. Slot 57 of SpecialAttractionInstance |
| `0x585bd0` | SpecialAttractionInstance::Slot45 | Runs `0x4C35E0`, then clears +0x1F4 and +0x1F8. Slot 45 of SpecialAttractionInstance |
| `0x588b40` | SpecialAttractionPersonInfo::Slot20 | By the attraction type's kind: 0xB calls `0x4D2800` with 10, 0xC with 11, 0xD with 12; nothing for others. Slot 20 of SpecialAttractionPersonInfo |
| `0x7312d0` | PathTypeDatabaseEntry::Name | The path type's name: the text at +8 of its data (+0x8C), or `<unknown path type>` when there is none. Slot 2 of PathTypeDatabaseEntry |
| `0x7395f0` | QueueTypeDatabaseEntry::Name | The queue type's name: the text at +4 of its data (+0x8C), or `<unknown queue type>`. Slots 2 and 4 of QueueTypeDatabaseEntry |
| `0x4ab940` | AnimatedRideInstance::ForwardSlot1 | Passes the call on to slot 1 of the part at +0x40 of the interface (id in `0x130bbec`) of the object its reference at +0x370 points to, the interface looked up once and kept at +0x380 (flag +0x384); nothing when there is none. Slots 1 of AnimatedRideInstance |
| `0x4aba00` | AnimatedRideInstance::ForwardSlot2 | Passes the call on to slot 2 of the part at +0x40 of the interface (id in `0x130bbec`) of the object its reference at +0x370 points to, the interface looked up once and kept at +0x380 (flag +0x384); nothing when there is none. Slots 2 of AnimatedRideInstance |
| `0x4abac0` | AnimatedRideInstance::ForwardSlot3 | Passes the call on to slot 3 of the part at +0x40 of the interface (id in `0x130bbec`) of the object its reference at +0x370 points to, the interface looked up once and kept at +0x380 (flag +0x384); nothing when there is none. Slots 3 of AnimatedRideInstance |
| `0x4abb80` | AnimatedRideInstance::ForwardSlot4 | Passes the call on to slot 4 of the part at +0x40 of the interface (id in `0x130bbec`) of the object its reference at +0x370 points to, the interface looked up once and kept at +0x380 (flag +0x384); nothing when there is none. Slots 4 of AnimatedRideInstance |
| `0x586030` | SpecialAttractionInstance::ForwardSlot1 | Passes the call on to slot 1 of the part at +0x40 of the interface (id in `0x130bbec`) of the object its reference at +0x198 points to, the interface looked up once and kept at +0x1a8 (flag +0x1ac); nothing when there is none. Slots 1 of SpecialAttractionInstance |
| `0x5860f0` | SpecialAttractionInstance::ForwardSlot2 | Passes the call on to slot 2 of the part at +0x40 of the interface (id in `0x130bbec`) of the object its reference at +0x198 points to, the interface looked up once and kept at +0x1a8 (flag +0x1ac); nothing when there is none. Slots 2 of SpecialAttractionInstance |
| `0x5861b0` | SpecialAttractionInstance::ForwardSlot3 | Passes the call on to slot 3 of the part at +0x40 of the interface (id in `0x130bbec`) of the object its reference at +0x198 points to, the interface looked up once and kept at +0x1a8 (flag +0x1ac); nothing when there is none. Slots 3 of SpecialAttractionInstance |
| `0x586270` | SpecialAttractionInstance::ForwardSlot4 | Passes the call on to slot 4 of the part at +0x40 of the interface (id in `0x130bbec`) of the object its reference at +0x198 points to, the interface looked up once and kept at +0x1a8 (flag +0x1ac); nothing when there is none. Slots 4 of SpecialAttractionInstance |
| `0x591730` | StallInstance::ForwardSlot1 | Passes the call on to slot 1 of the part at +0x40 of the interface (id in `0x130bbec`) of the object its reference at +0x1b0 points to, the interface looked up once and kept at +0x1c0 (flag +0x1c4); nothing when there is none. Slots 1 of StallInstance |
| `0x5917f0` | StallInstance::ForwardSlot2 | Passes the call on to slot 2 of the part at +0x40 of the interface (id in `0x130bbec`) of the object its reference at +0x1b0 points to, the interface looked up once and kept at +0x1c0 (flag +0x1c4); nothing when there is none. Slots 2 of StallInstance |
| `0x5918b0` | StallInstance::ForwardSlot3 | Passes the call on to slot 3 of the part at +0x40 of the interface (id in `0x130bbec`) of the object its reference at +0x1b0 points to, the interface looked up once and kept at +0x1c0 (flag +0x1c4); nothing when there is none. Slots 3 of StallInstance |
| `0x591970` | StallInstance::ForwardSlot4 | Passes the call on to slot 4 of the part at +0x40 of the interface (id in `0x130bbec`) of the object its reference at +0x1b0 points to, the interface looked up once and kept at +0x1c0 (flag +0x1c4); nothing when there is none. Slots 4 of StallInstance |
| `0xa7bbc0` | ChangingRoomInstance::ForwardSlot1 | Passes the call on to slot 1 of the part at +0x40 of the interface (id in `0x130bbec`) of the object its reference at +0x170 points to, the interface looked up once and kept at +0x180 (flag +0x184); nothing when there is none. Slots 1 of ChangingRoomInstance |
| `0xa7bc80` | ChangingRoomInstance::ForwardSlot2 | Passes the call on to slot 2 of the part at +0x40 of the interface (id in `0x130bbec`) of the object its reference at +0x170 points to, the interface looked up once and kept at +0x180 (flag +0x184); nothing when there is none. Slots 2 of ChangingRoomInstance |
| `0xa7bd40` | ChangingRoomInstance::ForwardSlot3 | Passes the call on to slot 3 of the part at +0x40 of the interface (id in `0x130bbec`) of the object its reference at +0x170 points to, the interface looked up once and kept at +0x180 (flag +0x184); nothing when there is none. Slots 3 of ChangingRoomInstance |
| `0xa7be00` | ChangingRoomInstance::ForwardSlot4 | Passes the call on to slot 4 of the part at +0x40 of the interface (id in `0x130bbec`) of the object its reference at +0x170 points to, the interface looked up once and kept at +0x180 (flag +0x184); nothing when there is none. Slots 4 of ChangingRoomInstance |
| `0xa2c8e0` | CIDDatabaseEntry::Name | The entry's name: the text at +0xC of its data (+0x7C), or `<missing data>` when it has no data. Slots 2, 10 and 17 of CEDDatabaseEntry and CIDDatabaseEntry |
| `0x4a4860` | AIDDatabaseEntry::MissingDependency | Reports a missing dependency: formats the text `Missing dependency for Attraction`, makes it a wide text and hands it to `0xC211C0` (with `0x10FE634`). Slots 5 and 13 of AIDDatabaseEntry |
| `0xa2c7b0` | CIDDatabaseEntry::MissingDependency | The same report with the text `Missing dependency for Carriedtem` (spelt so in the game). Slots 5, 13 and 20 of CEDDatabaseEntry and CIDDatabaseEntry |
| `0x843c30` | SceneryItem::FootprintCentre | The world position of the middle of an object's footprint: the centre of its grid rectangle (its slot 7) times the grid's cell size plus its origin (the terrain grid at +0x54 of `0x40A090`, activated first when needed), with the height of that point (`0x85CAE0`, `0x85CB20`) |
| `0x4aeb40` | AttractionInstance::Centre | Gives the middle of the attraction's footprint (SceneryItem::FootprintCentre). Slot 5 of the attraction instance classes |
| `0xb8d930` | AnimalShelter::Centre | Gives the middle of the shelter's footprint (SceneryItem::FootprintCentre). Slot 5 of AnimalShelter |
| `0x4bf3b0` | AttractionInstance::CentreAndTrue | Writes the middle of the attraction's footprint into the given point and returns 1. Shared by the attraction instance classes (slots 27, 28, 63) |
| `0x4bfa20` | AttractionInstance::RecentTotal | The total of the attraction's ten-entry history (the table at +0x9C, 12 bytes apart) times 90 / 300 |
| `0x4bfb10` | AttractionInstance::ResetStats | Starts the attraction's statistics again: the start time (+0x7C) from the game clock (`0x1106C54`, +0x48), its counters cleared and +0x88 set to -1, the ten-entry history made afresh (`0x4BF730`) and cleared |
| `0x4c35e0` | AttractionInstance::ResetAll | AttractionInstance::ResetStats, then slots 34 and 35 of the object at +0x140. Shared by the ride and attraction instance classes (slots 45, 46, 81) |
| `0x4bf730` | HistoryArray::Resize | Resizes an array of 12-byte entries ({data, count}) to n: a new aligned block, the entries that fit copied over, the old block freed; returns the new count. Used by AttractionInstance::ResetStats |
| `0x7f49c0` | Attraction::KindValue | A value for a kind (in ecx): 100 for kind 0, 300 for kind 2, 200 otherwise. Used by AttractionInstance::Slot51 |
| `0xbb5120` | Entry::IsSet | Whether the entry's first value is not -1 |
| `0x846ae0` | SceneryItem::Get20 | Returns the value at +0x20 (the object a part belongs to, as the parted-object requests use it) |
| `0x833890` | SceneryBehaviour::SetActive | Sets flag 1 of the behaviour's flags (+0x54). Called when litter bins, signs, fountains and other behaviours register |
| `0x8338b0` | SceneryBehaviour::ClearActive | Clears flag 1 of the behaviour's flags (+0x54). Called when they unregister |
| `0x833850` | SceneryBehaviour::Owner | The scenery item a behaviour belongs to: the interface (id in `0x130B5C8`) of the object its reference at +0x44 points to, or null |
| `0x4d18c0` | AttractionPersonInfo::Attraction | The attraction a person info belongs to: the interface (id in `0x12F4C64`) of the object its reference at +0x44 points to, or null |
| `0xb7a5a0` | Interface48::Get | The interface (id in `0x13CE968`) of the object its reference at +0x48 points to, looked up once (kept at +0x58, flag +0x5C) |
| `0x8bcf80` | Terrain::CellHeightRange | The lowest and highest of the four corner heights of a terrain cell (24 bytes each, the grid's width at +4 and cells at +0x24) |
| `0x726930` | PathTile::Centre | The world position of the middle of a path tile: its grid cell (+0x22, +0x89) times the grid's cell size plus its origin, with the height the tile's slot 7 gives there |
| `0x4bf970` | AttractionInstance::FirstListed | The value of the first entry of the list at +0xB0, or 0 when it is empty |
| `0x5d48d0` | Interface54::Get | The interface (id in `0x12F54E0`) of the object its reference at +0x54 points to, looked up once (flag +0x68) |
| `0xa9f090` | IndexedTable::Entry | Entry n of a table indexed from +0x1C to +0x20 (28 bytes each, at +0x24): the interface of the object it refers to, looked up once per entry; null outside the range |
| `0x4cce80` | AttractionInstance::FirstOf | With a non-zero argument AttractionInstance::FirstListed; otherwise the value of the first entry of the list at +0x64, or 0 |
| `0x43ffb0` | KeyState::Held | Whether the key (its code minus 0x1A as a bit) is set in either of two bit masks of the table at +0x84 (entries +0x70 and +0x74) |
| `0x560f80` | Interface48b::Get | The interface (id in `0x12F5544`) of the object its reference at +0x48 points to, looked up once (flag +0x5C) |
| `0xbc78f0` | Span::Middle | The middle of the values at +0xC of the objects at +0x20 and +0x24 (their sum halved) |
| `0x85cae0` | Terrain::HeightLevel | The terrain height level for a height: the height times the scale in `0x1310E78`, rounded up to a whole number |
| `0x72f060` | Path::Points | Gives a path's points and their count: with its flag (+6) set, from the first point and one more than its count (+0xA); otherwise from the second |
| `0x8438e0` | Sides::Rotate | Turns a 4-bit set of sides (one bit per side) round by n quarter turns: ((v << 4 | v) >> (4 - n)) & 0xF |
| `0x8275d0` | Footprint::RotateCell | Turns cell coordinates within a footprint by its orientation (0-3, a quarter turn each), using the footprint's size (+0x10 of the one given) |
| `0x494920` | Terrain::CellFlag12 | The terrain cell's byte at +0x12 shifted down one bit, for the cell at the given column and row of the grid (24 bytes each) |
| `0x4948d0` | EnclosureManager::CellAt | The first value of the enclosure manager's grid cell at the given column and row (24 bytes each, at +0x6C, its width +0x64 and height +0x68), or -1 outside the grid |
| `0xaa3240` | PoolPiece::SideData | The part of a pool piece's data for one side: side bit 1 gives +0x30, 2 gives +0x48, 4 gives +0x3C, 8 gives +0x54; anything else, or a piece whose first byte is 0, gives +0x60 |
| `0x8c0100` | Terrain::CornerPosition | The world position of a corner of a terrain cell: the column (plus 1 for odd corners) times the cell width plus the origin, the corner's height from the cell data (+0x24, six values per cell) |
| `0x46dfa0` | PassportPostBehaviour::Create | Makes a new object (200 bytes, constructed by `0x46d2b0` (PassportPostBehaviour::PassportPostBehaviour)), hands it to `0x833CC0` with the argument and activates it; returns it |
| `0x708ec0` | ParkEntranceBehaviour::Create | Makes a new object (112 bytes, constructed by `0x708ac0` (ParkEntranceBehaviour::ParkEntranceBehaviour)), hands it to `0x833CC0` with the argument and activates it; returns it |
| `0x786e10` | GuestInjectionPoint::Create | Makes a new object (96 bytes, constructed by `0x786bc0` (GuestInjectionPoint::GuestInjectionPoint)), hands it to `0x833CC0` with the argument and activates it; returns it |
| `0x819d80` | LitterBinBehaviour::Create | Makes a new object (192 bytes, constructed by `0x819320` (LitterBinBehaviour::LitterBinBehaviour)), hands it to `0x833CC0` with the argument and activates it; returns it |
| `0x81b7c0` | LoudSpeakerBehaviour::Create | Makes a new object (208 bytes, constructed by `0x81abe0` (LoudSpeakerBehaviour::LoudSpeakerBehaviour)), hands it to `0x833CC0` with the argument and activates it; returns it |
| `0x81e3b0` | ParkBenchBehaviour::Create | Makes a new object (112 bytes, constructed by `0x81dd40` (ParkBenchBehaviour::ParkBenchBehaviour)), hands it to `0x833CC0` with the argument and activates it; returns it |
| `0x824010` | PathFountainBehaviour::Create | Makes a new object (264 bytes, constructed by `0x823520` (PathFountainBehaviour::PathFountainBehaviour)), hands it to `0x833CC0` with the argument and activates it; returns it |
| `0x8243e0` | QueueLineEntertainerBehaviour::Create | Makes a new object (104 bytes, constructed by `0x8240b0` (QueueLineEntertainerBehaviour::QueueLineEntertainerBehaviour)), hands it to `0x833CC0` with the argument and activates it; returns it |
| `0x825250` | RideEventBehaviour::Create | Makes a new object (240 bytes, constructed by `0x8249d0` (RideEventBehaviour::RideEventBehaviour)), hands it to `0x833CC0` with the argument and activates it; returns it |
| `0x88c6c0` | SignBehaviour::Create | Makes a new object (184 bytes, constructed by `0x88c030` (SignBehaviour::SignBehaviour)), hands it to `0x833CC0` with the argument and activates it; returns it |
| `0x8b40e0` | VistaPointBehaviour::Create | Makes a new object (352 bytes, constructed by `0x8b3310` (VistaPointBehaviour::VistaPointBehaviour)), hands it to `0x833CC0` with the argument and activates it; returns it |
| `0x8b4aa0` | WaterableBehaviour::Create | Makes a new object (128 bytes, constructed by `0x8b4680` (WaterableBehaviour::WaterableBehaviour)), hands it to `0x833CC0` with the argument and activates it; returns it |
| `0x8bb210` | WaterCannonBehaviour::Create | Makes a new object (512 bytes, constructed by `0x8b82a0` (WaterCannonBehaviour::WaterCannonBehaviour)), hands it to `0x833CC0` with the argument and activates it; returns it |
| `0xa87c40` | LifeguardChairBehaviour::Create | Makes a new object (136 bytes, constructed by `0xa87740` (LifeguardChairBehaviour::LifeguardChairBehaviour)), hands it to `0x833CC0` with the argument and activates it; returns it |
| `0xa99b00` | PoolExtraBehaviour::Create | Makes a new object (160 bytes, constructed by `0xa99050` (PoolExtraBehaviour::PoolExtraBehaviour)), hands it to `0x833CC0` with the argument and activates it; returns it |
| `0x4ed4e0` | ValueIfKind2 0x198 | The value at +0x198 when the entry's kind (+1 of OffsetRef::Owner2) is 2 or more, otherwise 0 |
| `0x50a820` | ValueIfKind2 0x160 | The value at +0x160 when the entry's kind (+1 of OffsetRef::Owner2) is 2 or more, otherwise 0 |
| `0x50a850` | ValueIfKind2 0x164 | The value at +0x164 when the entry's kind (+1 of OffsetRef::Owner2) is 2 or more, otherwise 0 |
| `0x50a880` | ValueIfKind2 0x188 | The value at +0x188 when the entry's kind (+1 of OffsetRef::Owner2) is 2 or more, otherwise 0 |
| `0x50a8b0` | ValueIfKind2 0x190 | The value at +0x190 when the entry's kind (+1 of OffsetRef::Owner2) is 2 or more, otherwise 0 |
| `0x55ef20` | ValueIfKind2 0x18c | The value at +0x18c when the entry's kind (+1 of OffsetRef::Owner2) is 2 or more, otherwise 0 |
| `0x55ef50` | ValueIfKind2 0x194 | The value at +0x194 when the entry's kind (+1 of OffsetRef::Owner2) is 2 or more, otherwise 0 |
| `0x5a1120` | ValueIfKind2 0x144 | The value at +0x144 when the entry's kind (+1 of OffsetRef::Owner2) is 2 or more, otherwise 0 |
| `0x5bd120` | ValueIfKind2 0x154 | The value at +0x154 when the entry's kind (+1 of OffsetRef::Owner2) is 2 or more, otherwise 0 |
| `0x92de30` | ValueIfKind2 0x170 | The value at +0x170 when the entry's kind (+1 of OffsetRef::Owner2) is 2 or more, otherwise 0 |
| `0x92de60` | ValueIfKind2 0x174 | The value at +0x174 when the entry's kind (+1 of OffsetRef::Owner2) is 2 or more, otherwise 0 |
| `0x7313c0` | PathTypeDatabaseEntry::MissingDependency | When the entry has data (+0x8C), formats `Missing dependency for PathType` and reports it (`0xC211C0`), like AIDDatabaseEntry::MissingDependency. Slot 5 of PathTypeDatabaseEntry |
| `0x7396e0` | QueueTypeDatabaseEntry::MissingDependency | The same with `Missing dependency for QueueType` (data at +0x8C). Slots 5 and 7 of QueueTypeDatabaseEntry |
| `0xac3420` | PoolTypeDatabaseEntry::MissingDependency | The same with `Missing dependency for PoolType` (data at +0x80). Slots 5 and 13 of PoolTypeDatabaseEntry |
| `0xac3320` | PoolTypeDatabaseEntry::Name | The pool type's name: the text at +4 of its data (+0x80), or `<unknown pool type>` when it has no data. Slots 2 and 10 of PoolTypeDatabaseEntry |
| `0x592600` | StallInstance::Vendor | Gives the stall's vendor (through its scenery item, interfaces `0x130BBEC` and `0x130B5C8`); when there is none, shows a message box `No Vendor for stall` with the stall's name |
| `0xa51a80` | Scenario::LookupText | Makes the text key `Scenario%02d_Lookup` for scenario n (with the number of the given entry added when it is not negative) and looks the text up |
| `0x596e10` | StallInstance::ReportMissingCID | When a carried item the stall should sell is missing, shows a message box `Missing CID` with `Missing CID '%d' for stall '%s'` (the item's number and the stall's name), then `0x595DD0` |
| `0x6e4cd0` | EscapedAnimalsCondition::CurrentText | The objective's current-state text: the game text `EscapedAnimalsCondition_Current` with the count written in (`%d`) |
| `0x72c280` | PathTile::CheckOwned | Checks a path tile as it is placed; among other checks, shows the game text `NotOwned` (in a message window) when the land is not the park's |
| `0x6e6c30` | ReleaseAnimalsCondition::CurrentText | The objective's progress text: the game text `ReleaseAnimalsCondition_Current` with the current count written in (`%d`), and its timing (ObjectiveCondition::DescribeTiming) |
| `0x6e55d0` | ViewingGalleriesCondition::CurrentText | The same for the viewing galleries objective (`ViewingGalleriesCondition_Current`, `%d`) |
| `0x6e46b0` | DungTargetCondition::CurrentText | The same for the dung target objective (`DungTargetCondition_Current`, `%d`) |
| `0x6df560` | NumSceneryItemsOfClassificationCondition::CurrentText | The same for the scenery items objective (`NumSceneryItemsOfClassificationCondition_Current`, `%d`) |
| `0x6e5bf0` | MinAnimalHealthCondition::CurrentText | The same for the animal health objective (`MinAnimalHealthCondition_Current`, the health written as `%.2f`) |
| `0x6616d0` | EntranceExitMenu::StandardEntrance | Builds the entrance menu with the standard entrance of the `Style\Vanilla` style (`StandardEntrance`) as its default |
| `0x6618c0` | EntranceExitMenu::StandardExit | Builds the exit menu with the standard exit of the `Style\Vanilla` style (`StandardExit`) as its default |
| `0x456200` | ManagedObject::LoadRCT3fc | Loads the file `RCT3.fc` (ManagedObject::LoadFile) into the object given |
| `0xa51d90` | Scenario::TitleText | The title text of scenario n: the game text `Scenario` + its lookup name + `_Title` (Scenario::LookupText) |
| `0x6e7190` | EnclosureVisitorsCondition::CurrentText | The objective's progress text: the game text `EnclosureVisitorsCondition_Current` with the current number written in, and its timing (ObjectiveCondition::DescribeTiming) |
| `0x4d9940` | AttractionNameEdit::Apply | Applies a new name typed for an attraction: when it is empty, shows the game text `NoEmptyAttractionNames` (in a message window) and puts the old name back (`0x4C0850`); otherwise sets it (`0x4C2100`); then, for a pool attraction (+0x40), refreshes it (`0xA7C440`, `0xA8AC30`) |
| `0x4ec760` | AttractionNameEdit::Apply2 | The same check for another attraction name field (`NoEmptyAttractionNames` when empty, else `0x4C2100`) |
| `0xb91980` | AnimalShelterNameEdit::Apply | The same check for an animal shelter's new name (`NoEmptyAttractionNames` when empty, else `0xB8BA60`) |
| `0xa2ccf0` | CarriedItem::MakeGolfBall | Makes the mini golf `GolfBall` carried item (CarriedItemInstance) |
| `0xa2cea0` | CarriedItem::MakeGolfClub | Makes the mini golf `GolfClub` carried item (CarriedItemInstance) |
| `0xa2d050` | CarriedItem::MakeKidsGolfClub | Makes the mini golf `KidsGolfClub` carried item (CarriedItemInstance) |
| `0xa2cb40` | CarriedItem::MakeCamcorder | Makes the `Camcorder` carried item (CarriedItemInstance) |
| `0xa2c990` | CarriedItem::MakeCamera | Makes the `Camera` carried item (CarriedItemInstance) |
| `0x7f8f90` | CarriedItem::MakeAutograph | Makes the `Autograph` carried item (CarriedItemInstance); called from guest code (`0x75C3B0`) |
| `0x6e0870` | NumItemsBoughtCondition::CurrentText | The objective's progress text: the game text `NumItemsBoughtCondition_Current` with the current number written in, and its timing (ObjectiveCondition::DescribeTiming) |
| `0x6c85f0` | Challenges::DebugText | Writes the challenges' state for the developer: `Don't offer challenges timer: %.2f`, `Challenge publicity boosts: %d`, then each challenge (`Challenge %d: %s, %.2f`, with Challenge::StateName) |
| `0x6c4990` | Challenge::ShowFailure | Shows a challenge's failure message, the game text `Challenge_DefaultFailureText` (in a message window) |
| `0x819b70` | LitterBin::FillText | The litter bin's fill text: how full it is (`%d%%`), or the game text `Bin_NeverEmptied` |
| `0x88c4c0` | Sign::Text | The text a sign shows, such as the game text `Sign_NoEntry` |
| `0x8b6900` | WaterCannon::Name | The water cannon's name text (the game text `SceneryItem_WaterCannon`) |
| `0x8b1330` | VistaPoint::Name | The vista point's name text (the game text `SceneryItem_Vistapoint`) |
| `0x9823e0` | InventionGroup::NameText | The research group's name text: the game text `InventionGroup_` + the group's name |
| `0x4a7380` | SoakedIdentbar::Build | In outline: builds the Soaked expansion's identification bar entries (`SpecialSoakedIdentbar`) and their animations (InventibleAnimation) |
| `0x4a6d20` | InventibleAnimation::LoadSkins | In outline: loads the `Skin` entries of an animation that can be invented (InventibleAnimation) |
| `0x7b6180` | Crowd::Sounds | In outline: sets up the crowd sounds (`CheatBonusCheer1Fade`, `CheatBonusCheer1Lp`, ...) in a slot pool |
| `0x7b6790` | Stall::SwimSuitStall | In outline: sets up the `SwimSuitStall` stall item |
| `0x6defd5` | ThemedAttractionsCondition::CurrentText | The objective's progress text: the game text `ThemedAttractionsCondition_Current` with the number written in, and its timing |
| `0x83a550` | CapObjects::Name | In outline: the `CapObjects` name text |
| `0x7085b0` | Park::ChainSounds | In outline: the park's `Standard2Chain` sounds (Crowd::Sounds) |
| `0x9d9f50` | CarriedItem::MakeVoucher | In outline: makes a `Voucher` carried item, dated by the game clock |
| `0x7f9bc0` | VIP::LeftPark | In outline: when a VIP leaves the park (`VIP_LeftPark`), with the crowd sounds |
| `0x456aa0` | LoadFileCache::Save | In outline: saves the load file cache to `RCT3.fc` (ManagedObject::SaveFileQuick) |
| `0x9af1b0` | Object9AF::Name | In outline: a name text, `NULL` when there is none |
| `0x9af040` | Object9AF::Name2 | The same for another field (`NULL` when none) |
| `0x9aeed0` | Object9AF::Name3 | The same for another field (`NULL` when none) |
| `0x819e00` | LitterBin::ReadCapacity | In outline: reads a litter bin's `capacity` setting |
| `0x8b4f90` | SceneryBehaviour::ReadCapacity | In outline: reads a behaviour's `capacity` setting |
| `0x88c780` | Sign::ReadNoEntry | In outline: reads a sign's `enablenoentry` setting |
| `0x6b6df0` | PathPlacement::Check2 | In outline: another path placement check with the path manager and terrain (`DFAIL`) |
| `0x6b67a0` | PathPlacement::Check3 | In outline: another path placement check (`DFAIL`) |
| `0x6bf870` | Category::Other | Makes the category named `Other` |
| `0x6bf800` | Category::Clear | Clears a category's values |
| `0x881db0` | SceneryItem::MissingDependency | Reports `Missing dependency for SceneryItem` (like the other database entries) |
| `0x771570` | Report::MissingResourceSymbol1 | When the flag (Flags::Bit0) is clear, shows a message box `Missing Resource Symbol` |
| `0x696130` | Report::MissingResourceSymbol2 | The same `Missing Resource Symbol` report |
| `0x431360` | Report::MissingResourceSymbol3 | The same `Missing Resource Symbol` report |
| `0x6e6e40` | EnclosureVisitorsCondition::Name | The game text `ObjectiveCondition_EnclosureVisitors` |
| `0x6e5e00` | ReleaseAnimalsCondition::Name | The game text `ObjectiveCondition_ReleaseAnimals` |
| `0x6e5860` | MinAnimalHealthCondition::Name | The game text `ObjectiveCondition_MinAnimalHealth` |
| `0xb10fa0` | WildAnimal::ItemName | In outline: a wild animal item's name (`WildAnimalItem_` + name) |
| `0x874e40` | GridMarks::MarkRect | Marks every cell of a rectangle of a grid (width +0x48, height +0x4C, cells of 0x34 bytes at +0x50; byte +1 set to 1), with the rectangle kept inside the grid, and grows the changed area (+0x54 to +0x60) to take it in |
| `0x874d90` | GridMarks::MarkRect2 | The same marking of a rectangle inside the grid, setting byte +2 of each cell instead (without growing the changed area) |
| `0xb9a810` | PathGrid::SetEdges | Sets edge bits on a grid cell (width +0x64, cells of 0x18 bytes at +0x6C, bits at +0x14) and the matching bits on the neighbouring cells for bits 1, 2, 4 and 8 |
| `0xb9a8c0` | PathGrid::ClearEdges | Clears those edge bits on a cell and its neighbours |
| `0x874cf0` | GridMarks::MarkRect0 | The same marking of a rectangle as GridMarks::MarkRect, setting byte 0 of each cell |
| `0x727890` | TileCorners::Pack | Packs eight two-bit corner values into +0x30 and four looked-up four-bit values (from `0xF6A500`) into +0x32 |
| `0xbd2ca0` | Groups::Totals | For each group, adds up its members' counts (+0x40) into +0x38 and +0x3C, joins their flags into +0x40 and keeps the largest of their +0x44 and +0x48 values |
| `0xba30b0` | Grid::QueueCell | Moves the flagged objects (+0x60) of a grid cell into the list at +0x8C and sets their +0x58 to 1 |
| `0x99c2a0` | Grid::SnapToRow | Snaps a position's height down to a whole row (`0x13413B0` apart, kept between 0 and the last row) and keeps its first value at least a minimum |
| `0x71cb80` | Grid::MarkRectForOwner | For each layer (32 bytes at +0x68, count +0x70) of the object given, sets byte +0x18 of every cell (0x4C bytes) in the rectangle given (coordinates divided by 16) |
| `0x8dc0c0` | HeightPatch::Average | For each cell of a rectangle (bounds at +4 to +7), averages the four corner heights of the terrain and writes the average to all four corners of the output |
| `0x8db7a0` | HeightPatch::Flatten | Writes one height to all four corners of each cell of a rectangle: the height given, rounded to steps when a step count is set (+8) |
| `0x7ea210` | Ticker::Update | Counts a tick (+0x410); while a timer (+0x4D8) is above 0.5 it counts it down, otherwise lowers +0x4DC by an amount that depends on the kind (+0x414) |
| `0x727d30` | TileQuarter::Corners | The four corner points of a quarter of a tile, picked by the bits of a packed value and the quarter number |
| `0x727b00` | TileCorners::Count | Counts a tile into one of three totals by its packed corner value (+0x32): the even patterns as the first kind, the higher ones as the second, mixed ones corner by corner; flagged tiles (+0x34) go to the third |
| `0x729530` | Cell::Quad | The four corner points of a terrain cell (+0x88, +0x89): positions from the cell size and origin, heights from the terrain |
| `0xbad250` | Rect::SideCell | The cell beside place n along one side (+0x1E4: 0 to 3) of a rectangle at +0x1DC, +0x1E0, when n is within its length (+0x1EC) |
| `0x838c00` | Grid::RotateCoords | Turns a cell's coordinates by the grid's rotation (+0x17C: 0 to 3) within its size |
| `0x913620` | Mover::StepBack | Counts down a time (+0x58) and sets the position back along a direction from its node (in `0x133A0F4`, 0x98 bytes each); flags it done (bit 1) when the node is gone |
| `0xafa0e0` | Terrain::CellColourAt | The colour for a terrain cell at a height (dark grey `0xFF333333` by default), from the flagged layers of the cell (0x2C bytes each) that the height reaches |
| `0x8750c0` | Grid3::Rotate | Turns a position in a 3x3 grid by a rotation (0 to 3) |
| `0x875020` | Grid3::RotateBack | Turns a position in a 3x3 grid the other way (0 to 3) |
| `0x4ee3a0` | Gate::SetState | Sets a gate's two state values (+0x20, +0x24) for an action 0 to 4 (open or shut, and which side) |
| `0xabc300` | Tile::EdgePoint | The point at the middle of a tile's edge for a direction (1, 2, 4 or 8), or its centre for any other |
| `0x95b610` | Mesh::CountForId | Adds up the vertex and index counts of a mesh's parts with the id given (16 bytes each at +0x14, count +0x1C): 3 and 3 for one shape kind, 4 and 6 for the other |
| `0xb987e0` | TrackPiece::Direction | A direction (0 to 3, or 4) from a piece's kind bits, when its slope values are big enough; -1 otherwise (the piece is passed in a register) |
| `0x727cc0` | Tile::QuarterCorners | The four corner points of a tile half, picked by two bits (another form of TileQuarter::Corners) |
| `0x5d13c0` | Settings::Downgrade | Lowers the values that only a higher setting allows (7 to 3, 8 to 4, 2 to 0 or 1, 3 to 2) in a list of ten settings |
| `0x5bc150` | Counts::AddByKind | Adds one to the count that matches a thing's kind (+0x10): 1 and 2, 4, 0x15 to 0x17, 0x23, 0x24, 0x25, 0x27 (only above a level), 0x28, 0x29, ... |
| `0x703170` | Grid::GrowChanged | Grows a grid's changed area (+0x74 to +0x80) to take in a rectangle, with the rectangle kept inside the grid (+0x6C by +0x70) |
| `0xafa080` | TerrainLayers::SetFlag | Sets the value (+0x24) and the flag (bit 2 of +0x10) of each terrain layer of a list |
| `0x7339b0` | Area::Move | Moves an area's rectangles (+0x40 to +0x5C) and every one of its items (+0x68) by an offset |
| `0x71bfe0` | Rect::AddCell | Grows a byte rectangle (+0x92 to +0x95) to take in a cell |
| `0xabc6d0` | GridArea::ValueAt | The value (+0x4C) of the cell at a position inside an area (bounds +0x4C to +0x58, cells of 0x58 bytes at +0x5C), when the cell has its part set |
| `0x5bc050` | Counts::AddByLevel | Adds one to one of four counts by which of four levels a value reaches |
| `0x55fbb0` | Attraction::SpeedScale | For the attraction kinds 0x2E to 0x31 and 0x44, its value (+0x184) times `0x10EE8F4`, kept between a minimum and 1; 0 for other kinds |
| `0x9375c0` | Grid::ContainsId | Whether a rectangle of a grid of 16-bit ids (+0x38, width +0x30) holds the id given |
| `0x4e4a00` | Car::SpeedFactor | A car's speed factor (by `0x10EE2FC`) from its part n: one value when flagged (+0x148, with `0x10EE2F8` set), 1 when +0x144 is set, otherwise a third |
| `0x9892a0` | Cars::TotalLoad | Adds up, for each car (0x58 bytes at +0x104, count +0x108), its load (+0xCC, +0xC4) divided by 10 |
| `0x548b40` | Object::Position | The position (+0x90) of the object's part (+0x41C), or of its other part (+0x418) when there is none |
| `0xabc730` | GridArea::PartAt | The part of the cell at a position inside an area (bounds +0x4C to +0x58, cells of 0x58 bytes at +0x5C) |
| `0x713650` | Walker::Step | Moves a position (+4, +8) one cell in its direction (+0x28: 0 to 3) and its height (+0x2C) by its slope kind (+0x24) |
| `0x8bcff0` | Rect::GrowByOne | Grows a byte rectangle by one cell each way, kept inside the grid (+4, +5) |
| `0x5061b0` | Ride::Offset | The ride's offset (+0x1E0) for kinds 2 and 3 (+0x1C8), or none |
| `0xaad5a0` | Grid3::RotateIndex | Turns a cell number of a 3x3 grid (0 to 8) by a direction (passed in registers) |
| `0xabc790` | GridArea::ValueAt2 | Another value of the cell at a position inside an area (bounds +0x4C to +0x58, cells of 0x58 bytes at +0x5C) |
| `0x55f940` | Ride::InRange | Sets a flag (+0x7C) when the value given (+0x24) lies between the object's limit (+0x70) and the other's (+0x18) |
| `0xa56b30` | Queue::Next | Takes the next entry of a list (+0x24's +0xD8) and starts it (`0xA565D0`), or clears it when the list is empty |
| `0x71bf70` | Cell::StepInside | Moves a cell position one step in a direction (0 to 3) and gives whether it is still inside the grid (+0x5C, +0x5D) |
| `0x829440` | Ride::IsBusy | Whether a ride has anything going on: people on it (its part's +0x70), a queue (+0x198), a pending change (+0x7C bit 2 with +0x58 bit 1 clear) or +0x1A0; otherwise looks further (+0x84) |
| `0x708520` | Park::Get | The park (`0x1306194`): makes it (a 0x1F0-byte Park) the first time and activates it when it is not active yet |
| `0x434070` | Path::Place434070 | In outline: placing on a path: gets the path manager (PathManager::Get) and the terrain, finds the tile (`0x71B370`) and acts by its kind (+0x8B: 1 or 2) and a mode (0 and up) |
| `0x8fa0b0` | Terrain::Adjust8FA0B0 | In outline: changes terrain corner heights inside a rectangle (from +0x157 on), case by case, keeping the smallest difference between neighbouring corners, then checks the result (`0x8FB1F0`) |
| `0x842870` | Scenery::LoadStep842870 | In outline: a loading step over scenery items by a stage (+0xD0: 1 to 3): resolves each item's visual (SceneryItemVisual::CheckResolved), builds names, and saves a screenshot (Screenshot::Save) in one stage |
| `0x7b0af0` | Park::ClickAt7B0AF0 | In outline: works out what lies under a point in the park: the terrain (made if needed), the path manager, water (WaterManager), and other things found along the line (`0x8C5360`, `0x944250`, `0x900B40`, `0x48C300`), with the enclosure manager for animals |
| `0x777990` | Guest::Place777990 | In outline: places a guest or group on the path at a point (PathManager::Get, `0x71B370`, Grid::CellAt), makes it (`0x7E8100`) and goes over the group's members (GuestGroup::Member) |
| `0xab8210` | Fence::Build | In outline: builds the pieces along a border (+0x120 kind 2), looking up each piece by its side and corner (`0xA9F4E0` with six sides from +0xFD to +0x103) and making them (`0x7CBF50`) |
| `0x5ca9a0` | Placement::CheckGround5CA9A0 | In outline: checks the ground under something being placed: the tile it sits on (OffsetRef::Owner2, kinds above 2, 0x15) and, cell by cell over its area, the terrain corner heights against its own height |
| `0x6fe280` | Park::FindNear6FE280 | In outline: goes over a list of things near a point (paths, PathTile::Centre; scenery, SceneryItem::FootprintCentre) by their kind and keeps the ones that match (`0x49ED70`) |
| `0xa5a840` | Park::CheckEvent | In outline: checks whether a park event of kind n should happen, from park numbers (Park::Get +0x60, counts from `0x4CD590`), the date (GameTime::Get, Park::CountBelow) and a random chance |
| `0x5a52f0` | Placement::Update5A52F0 | In outline: updates a thing being placed (when +0x378 is clear): makes its preview (`0x59BE00`, `0x9FE2A0`) and checks it against the other objects of the list (+0x16C chain) by their boxes (`0xBC5AB0`, `0xBC5B40`, `0xBCC740`) |
| `0x8c3a10` | Terrain::CellRange8C3A10 | In outline: goes over a rectangle of terrain cells, keeping the lowest and highest heights seen (starting at plus and minus 1e9) and flags for which edges were raised or lowered |
| `0x7e7440` | Park::Rebuild7E7440 | In outline: a timed rebuild inside a critical section: makes the path manager and terrain when needed, sets up a 90 by 90 grid (`0x8D41B0`) and goes over a range of entries (`0x7E51F0`, `0x7E5400`, kinds 2 to 4) refreshing them (`0x417C20`) |
| `0x8c1990` | Terrain::Normals8C1990 | In outline: works out the normals of a rectangle of terrain cells (32 bytes each): normalised sums from the cells around each one, with the edges and corners handled apart and flagged cells (+0x13) skipped |
| `0x8c2e10` | Terrain::EdgeFlags8C2E10 | In outline: goes over a rectangle of terrain cells setting flags for each cell's four sides from its neighbours, with the grid edges (+4, +5) handled apart |
| `0x7e8100` | Park::Rebuild7E8100 | In outline: another timed rebuild like Park::Rebuild7E7440 (path manager, terrain, a 90 by 90 grid, entries refreshed), with a flag to skip part of it |
| `0x466b30` | PairIds::Update | In outline: goes over the pair records (`0x1106E90`, count `0x12DFFE8`) and the table `0x111EFE0`, marking and joining pairs (PairIds::Get) by their flags |
| `0xabf250` | Fence::RebuildRect | In outline: rebuilds the pieces of an area (bounds +0x4C to +0x58) over a rectangle, cell by cell: clears the old ones and makes the new ones (`0xAAF4E0`) |
| `0x941090` | Water::HitTest941090 | In outline: whether a ray hits water, by the kind of thing it is (1 to 6): checks each part (`0x93D140`) and keeps the nearest hit |
| `0x945ae0` | Water::HeightAt945AE0 | In outline: the water height at a terrain cell: checks the water bodies (+0x24, count +0x40) and the cell (0x30 bytes each at +0x1C) and its neighbours (`0x90C240`) for the highest surface above the point |
| `0x8c7660` | Terrain::BuildEdges8C7660 | In outline: goes over a rectangle of terrain cells and for each layer (+0x48) builds the cliff pieces on each of a cell's four sides where its neighbour differs (`0x8C6760`, sides 0 to 3), skipping flagged sides (+0x13) |
| `0xa92e40` | GridTool::UpdateA92E40 | In outline: a grid tool's update by its mode (+0x140): finds the cell under the mouse (`0xA91E70`) and, when the dragged rectangle changes, updates the cells it adds or drops (`0xAC0A50`, `0xABFF60`) |
| `0x4bc500` | Scenery::DrawFootprint4BC500 | In outline: draws a scenery item's footprint marks around its centre (SceneryItem::FootprintCentre) with the shared shader set (Registry::Ensure, `0x481620`), a second set when +0x370 is clear |
| `0x558380` | Placement::Snap558380 | In outline: snaps a thing being placed to the tile at a point by its rotation (`0x42EB40`), and by the result case sets its box (Box::Assign) and its position (`0x7359A0`) |
| `0x4b3920` | Shop::UpdateQueue4B3920 | In outline: goes over a shop's slots (+0x394) and the people at each (kind 3, +0xBC grouped by the slot size), moving on the ones done (+0xB0 = 2) and starting the next (`0x4D2800`) |
| `0x821e90` | SceneryBehaviour::Update821E90 | In outline: a scenery behaviour's update: from its owner (SceneryBehaviour::Owner) and the camera distance (MainCamera::Get), starts or stops its effect (`0x94DCC0`) and its sound or animation |
| `0x729b00` | Terrain::CheckCell729B00 | In outline: checks a terrain cell's four corner heights against a level within a tolerance (`0x1306364`), then whether something is already there (Entries::FindByKey, kind 0x1F) |
| `0x8bd5b0` | Terrain::RayMarch8BD5B0 | In outline: walks a ray across the terrain cell by cell (Grid::CellAt) by the signs of its direction, checking each cell's heights for a hit |
| `0x4b2f50` | Shop::Update4B2F50 | In outline: a shop's update: goes over the people at it (kind 3 waiting, 2 done), lets the next one in (`0x4D2800`), refreshes it (`0x4B0A20`) and acts by its state (+0x354) |
| `0x8aefd0` | Scenery::DebugRating | Draws the developers' readout of a spot's rating: lines (Registry::Ensure) and text rows `Distance: %f, SD: %f`, `Slope: %f, SD: %f`, `Height: %f, SD: %f`, `Points: %d` and `Scenery: %f, Nice: %f, Built %f` (`0x496C60`) |
| `0x4f41c0` | Placement::Check4F41C0 | In outline: checks a thing being placed against what is around it (`0x5890F0` over a list, `0x5D04A0`, `0x5D1370`, `0x5D1D30`) and sets its state by the kind found (1, 3 or 6) |
| `0x8c8100` | Terrain::Strips8C8100 | In outline: goes over a terrain patch's pieces (+0x7C) by their side bit (bit 28 against +0xE4) and joins them into strips |
| `0x5e5a90` | Scenery::Update5E5A90 | In outline: a placed item's update: checks it (`0x5D9720`), its owner's parts (`0x5E3F00`) and, when it is the selected one (`0x12F5608`), refreshes its link (`0x4E6410`) |
| `0x58d400` | Placement::Step58D400 | In outline: one step of a placement tool by its mode (+0x100): works out where it goes (`0x58ECF0`, `0x58CA20`, `0x58D200`) and gives whether it is done |
| `0x5cb500` | Placement::Ground5CB500 | In outline: finds the ground for a thing being placed: the terrain (a 90 by 90 grid) and what is there (`0xAFD9D0`, `0xAF7890`, `0xAF7830`) |
| `0x848e30` | Scenery::PlaceAt848E30 | In outline: places a scenery item at a cell: sets up (`0x843A30`), checks the cell is inside the terrain, makes the item and checks whether a key is held (KeyState::Held) |
| `0x839870` | Scenery::Update839870 | In outline: a scenery item's update when it has work to do (+0x184, +0x1E8): works with the terrain and its target (+0x16C) and may show a message (`0x6C1DA0`) |
| `0x942a70` | Water::Flood942A70 | In outline: whether water at a height can spread from a cell: compares the heights around it and spreads to the neighbours (`0x941EF0`) |
| `0xa9c040` | Fence::BuyA9C040 | In outline: when a fence or border piece can be built (`0xA97800`), checks the park can pay for it (`0x6C1DA0`, its money at +0x58 against the cost) and builds it |
| `0x4f5950` | Ride::Buy4F5950 | In outline: when a ride or shop is placed (+0x60C): checks the cost against the park's money (`0x6C1DA0`, +0x58, unless money is off, +0x68 bit 1), finishes its placement (`0x5D0950`, `0x59DC60`, `0x4BBC80`) |
| `0x5e0350` | Placement::Clearance5E0350 | In outline: for most kinds (not 0x26, 0x2C, 0x32), checks a thing being placed against each of its neighbours (+0x13C, count +0x140, `0x5DBBF0`) and the ride car given |
| `0x5d7d70` | Placement::CanPlace5D7D70 | In outline: whether something can be placed with the options given: checks the selected tool (`0x12F5638`, its flags +0xA0, +0x84), its rotation (Clamp0To3) and several cases |
| `0x7e55d0` | Enclosure::CheckCell7E55D0 | In outline: checks a cell against an enclosure's rules: its kind (4 checked against a size), its enclosure (EnclosureManager::Get, its width +0x64 and height +0x68) and the case for the side given |
| `0x8c2590` | Terrain::Shade8C2590 | In outline: works out shading values for a rectangle of terrain cells (blocks of 32), from the heights around each, as bytes |
| `0x725150` | Path::Connect725150 | In outline: connects the path tiles of a rectangle (+0x14 to +0x17): finds each tile (PathManager::Get) and checks its four neighbours over the terrain |
| `0x83c520` | Scenery::Check83C520 | In outline: for scenery of kind 0x1C (+0x16C +0x48), checks its cells on the terrain (a 90 by 90 grid, `0x838AC0`, `0x838B60`) by its rotation (+0x17C) |
| `0x72a4d0` | Terrain::TileShape72A4D0 | In outline: works out a terrain tile's shape from its packed corners (+0x32: the flat patterns 0, 0x1111, 0x2222, 0x3333 and 0x4444 handled first), with its quarters' corners and edge middles (Tile::QuarterCorners, Quad::EdgeMiddles, TileQuarter::Corners) |
| `0xb9e920` | PathGrid::RebuildB9E920 | In outline: a timed rebuild of a path grid inside a critical section, over the terrain (a 90 by 90 grid), for the size given |
| `0x8e16e0` | Terrain::Level8E16E0 | In outline: evens out a set of terrain heights (+0x196 of them): joins the ones within a small step of each other and snaps them (Values4::Snap with 0.01) when flagged (+0x194) |
| `0x7162e0` | Path::Build7162E0 | In outline: builds a path piece from a description (`0x7141A0`), making the path manager when needed and giving an error by case (the text `ERROR_OUTOFMEMORY` among them) |
| `0xa7de70` | Shop::UpdateA7DE70 | In outline: a shop or stall's update when not flagged (+0x1C4): with its slot (+0x1F4) set, refreshes its items (`0x4CDE30`, `0x4D0CB0`) |
| `0xbb0d30` | Shop::ServeBB0D30 | In outline: serves at a shop when not flagged (+0x1C8) and at least two are given: checks the slot (+0x1C4, +0x1E4) and the queue (+0x1D8) and starts the sale (`0x4AD740`) |
| `0x702400` | Tool::Area702400 | In outline: finds the ground under the mouse (MainCamera::Get, `0xCD79C0`, `0x8C5360`) and the rectangle of cells it covers (`0x702110`) for an area tool |
| `0x7227f0` | Path::FindLink7227F0 | In outline: goes over a path's points (Path::Points) and finds the one that links up with the kind given, by case (`0x71EA30`, `0x71EAE0`) |
| `0x94a5f0` | Water::FillRect94A5F0 | In outline: goes over a rectangle of water cells (within +0x18 by +0x19) and, where the surface is above the terrain, sets the cell's body (`0x947680`) |
| `0x668de0` | Park::List668DE0 | In outline: builds a list of the park's things (`0x49B930`, `0x665E90`, `0x49B050`) with their names |
| `0x5c83a0` | TrackedRide::Check5C83A0 | In outline: checks the selected tool's ride piece (`0x12F5638`, `0x5D4950`) on its tile (OffsetRef::Owner2, kinds below 3) and its parts (+0xE4, +0x16C) (`0x5C6E30`) |
| `0x597030` | Ride::Update597030 | In outline: a ride's update: goes over its parts (+0x1CC) updating each (`0x9817B0`) and refreshes the ones linked to it |
| `0xa0cbd0` | Weather::UpdateA0CBD0 | In outline: a timed weather or event update inside a critical section (skipped when `0x10EDAD8` is set): counts down, picks at random (Random::Ran0, `0x4751E0`) by the date (GameTime::Get) |
| `0xafa7c0` | TerrainLayers::BuildAFA7C0 | In outline: when needed (+0x1078), builds the terrain layer grid: its size from the terrain's size and cell size, rounded, and its cells (0x2C bytes each) |
| `0x565030` | Path::Queue565030 | In outline: goes over a queue's path pieces (+0xC0, count +0xC4) with their neighbours and checks the ones of kind 5 (+0x164) against a length |
| `0x4c6a40` | Tool::Update4C6A40 | In outline: the selected tool's update (`0x12F5638`): finds its slot (`0x5E1010`) and, when found and not flagged (+0x74), acts on it |
| `0x82b5c0` | Scenery::Refresh82B5C0 | In outline: refreshes a scenery item: rebuilds its parts (`0x828CB0`, `0x827B40`, `0x876660`, `0x840620`) and, with a slot (+0x190) and not flagged (+0x18C), its link |
| `0x4f1050` | Ride::Placing4F1050 | In outline: while a ride or shop is being placed (+0x60C), makes its name and acts by its kind (+0x144: 1, 2, ...) when the thing under the mouse changes (+0x618, +0x61C) |
| `0x9489a0` | Water::Bodies9489A0 | In outline: goes over the water cells (+0x5C) and finds which body each belongs to (`0x95BC10`), joining bodies at the same height |
| `0x7e5fa0` | Park::Rebuild7E5FA0 | In outline: another timed rebuild like Park::Rebuild7E7440 over the terrain (a 90 by 90 grid) for an entry, with kind 2 tiles handled (`0x7E5350` with 0x40) |
| `0x4ea9c0` | Ride::Select4EA9C0 | In outline: after an update (`0x4D29D0`), when the ride is not being placed (+0x378), makes it the selected one with the current tool (`0x12F5638`, `0x12F5608`, `0x4E6410`) |
| `0xaab280` | PathGrid::RebuildAAB280 | In outline: a timed rebuild inside a critical section (unless a count `0x13CE030` is too high): clears (`0xA9F7E0`), sets up (`0x7CD120`, `0xAAA470`) and goes over a range of entries |
| `0x42ca70` | Groups::Merge42CA70 | In outline: goes over groups (`0x42C9C0`, `0x42BFF0`) joining the smaller ones into the larger (`0x42C490`), like Water::MergeBodies965D90 |
| `0x8e44a0` | Terrain::Mark8E44A0 | In outline: goes over a rectangle of terrain cells (+4 to +0x10) and marks the ones whose value (+0x14) is set |
| `0x49ee80` | Curve::Sample49EE80 | In outline: samples a curve at a point (scaled by `0xF2EACC`) into the values given |
| `0x56b690` | Ride::UpdateTool56B690 | In outline: when not flagged (+0x5C) and the ride is not being placed (+0x378), acts by the tool's mode (`0x12F5638`: 2, 6 and 7 with entries at +0x78) |
| `0x4d56a0` | Shop::UpdateTool4D56A0 | In outline: when not flagged (+0x58) and not being placed (+0x378), acts on the current tool (`0x12F5638`) by its result (`0x4D45C0`: 2, ...) |
| `0x79c2e0` | Tool::Cells79C2E0 | In outline: works out the cells a tool covers on the terrain (a 90 by 90 grid) from its point (+0x54), rounded |
| `0x47a790` | Park::ListNames47A790 | In outline: goes over a list of things (`0x1305D54`, count `0x1305D58`), skipping the ones that fail a check (`0xBD19C0`), and joins their names into one text |
| `0x8ec360` | Water::BuildAll8EC360 | In outline: builds the water pieces (Water::Build8EB860) and, for the current device (ForCurrent(3)), fills a shared 0x1000-entry table (`0x1317E40`) from the pair records |
| `0x95bc10` | Water::BodyAt95BC10 | In outline: the water height of a cell's body: sets it up (`0x95A210`) and goes over the cells by their kind (+8: 0, ...) |
| `0xaa4950` | GridTool::PlaceAA4950 | In outline: places a grid tool's cells at the ground under the mouse (MainCamera::Get) on the terrain (a 90 by 90 grid), after refreshing it (`0xAA3DE0`) |
| `0x5d9000` | Placement::Finish5D9000 | In outline: finishes placing a piece: refreshes it (`0x846A90`, `0x5C45E0`) and, when not flagged (+0xAC), sets it up (Interface54::Get) |
| `0x8af980` | Scenery::RateSpot8AF980 | In outline: works out a spot's rating: sets up (`0x8AED60`), finds what is near it (`0x48C300`, `0x7B9BB0`) and the terrain, keeping the nearest |
| `0x8fe4e0` | Terrain::Points8FE4E0 | In outline: picks the points of a list inside a box (+0x10 to +0x18) after growing the list when needed (`0x8FDF70`) |
| `0x82f120` | Footprint::Check82F120 | In outline: checks a footprint's cells (+0x14 of them) turned by a rotation (Footprint::RotateCell) against the terrain heights within a tolerance |
| `0xa8fcb0` | PoolComplex::SaveA8FCB0 | In outline: makes a pool complex's save data (PoolComplexSaveData) and fills it (`0xA8EF10`), and, when not flagged (+0x78), the rest (`0xA8E970`) |
| `0x8dc170` | Terrain::Smooth8DC170 | In outline: goes over a rectangle of terrain cells (bounds at +4 to +7) evening out the heights around each |
| `0xb96820` | PathGrid::CellB96820 | In outline: works out a path grid cell for a point on the terrain by its kind (+4: 0, ...), rounded |
| `0x565dc0` | Path::Queue565DC0 | In outline: for a queue (kind 0xD), goes over its pieces (+0x114 of them, `0x5D6E80`) when not flagged (+0xF0) |
| `0x4b0190` | Shop::ItemText4B0190 | In outline: makes a shop item's text when it is set (Entry::IsSet): its name, with its price when it has one |
| `0x95ad40` | Water::Spread95AD40 | In outline: spreads water from a cell by its kind (+8: 0, ...) to its neighbours (`0x95A570`), growing a shared list by 0x200 when full (`0x95A2C0`, `0x133B4B8`, `0x133B4C4`) |
| `0x728120` | Terrain::TileShape728120 | In outline: another tile shape builder like Terrain::TileShape72A4D0: edge middles (Quad::EdgeMiddles) and quarter corners (TileQuarter::Corners) by the quarter given |
| `0x8d7ac0` | Terrain::Rows8D7AC0 | In outline: rebuilds terrain row n (within the grid, +0x54): sets it up (`0xAFA6B0`, `0x8C8BA0`) and builds each of its cells (`0x8D67A0`) |
| `0x8d6fc0` | Terrain::Columns8D6FC0 | In outline: the column form of Terrain::Rows8D7AC0: rebuilds terrain column n and its cells (`0x8D67A0`) |
| `0x7a1970` | Path::Under7A1970 | In outline: finds the path tile under a point (PathManager, `0x71B370`) and its parts (InterfaceD8::Get, Interface94::Get), and for flag 2 (+0x408) what is on it (`0x12F51B4`) |
| `0xaafc00` | Fence::PiecesAAFC00 | In outline: goes over a border's packed piece values (low three bits kind 1, side bits 0x38, turn 0xC0) and handles each case when not flagged (+0x178) |
| `0x57afb0` | Ride::NameAt57AFB0 | In outline: finds what is under the mouse (MainCamera::Get, `0xCD79C0`) and makes its name texts |
| `0x5615d0` | Path::Queue5615D0 | In outline: for a queue (kind 0xD) not flagged (+0x98): with the ride's flag 0x100 and a length (+0x70), checks its kinds 0x5D to 0x5F (`0x5BF000`) |
| `0x8d68e0` | Terrain::ReadCell8D68E0 | In outline: reads a saved terrain cell from a block (`0x809210`, 4 and 1 bytes at a time), checking the block's size and its kind (+0x12, +0x1A) |
| `0x8d9060` | Terrain::GrowRows8D9060 | In outline: adds n rows to the terrain (kept under 255): sets it up (`0xAFA6B0`, `0x8C8BA0`) and makes each new row's cells |
| `0x566e00` | Path::Check566E00 | In outline: for kinds other than 0x3A to 0x3C, when not flagged (+0x98, +0xF0), checks the piece (`0x5DBCD0`) |
| `0x755bf0` | Park::Spot755BF0 | In outline: for entry n of a list (`0x1306530`), works out its spot on the terrain (a 90 by 90 grid, `0x7578C0`, `0x7BE2F0`) |
| `0x8d85c0` | Terrain::GrowColumns8D85C0 | In outline: the column form of Terrain::GrowRows8D9060: adds n columns (kept under 255) |
| `0x715170` | Path::Rebuild715170 | In outline: rebuilds a path piece from its description (`0x7141A0` on +0x23C, `0x7134C0`), or the plain way (`0x714F30`) |
| `0x841f70` | Scenery::Grow841F70 | In outline: grows a scenery list (`0x130BB24`, ObjectArray28::Resize) to n and makes the new entries (`0x733A90`) |
| `0x4b9d60` | Shop::Stats4B9D60 | In outline: works out a shop's figures from its values (`0x4BF100`, `0x4BF0B0`, `0x4B9C20`, `0x4B9CC0`) and the date (GameTime::Get) |
| `0x8bd050` | Terrain::HighestIn8BD050 | In outline: the highest terrain height inside a square around a point (Grid::CellAt for its corners) |
| `0x8d9600` | Terrain::GrowRows8D9600 | In outline: another row growing pass like Terrain::GrowRows8D9060 |
| `0x762a10` | Guest::Remember762A10 | In outline: when not flagged (+0x530), makes a record of the things a guest has seen (PersistentObjectsSeen) and fills it (`0x75AB70`) |
| `0x7143c0` | Path::Build7143C0 | In outline: builds a path piece (`0x714120`, `0x713460`) and links it |
| `0xb9dee0` | PathGrid::RectB9DEE0 | In outline: makes a list of the path grid cells in a rectangle (within its height, +0x68) |
| `0x8d7580` | Terrain::ShrinkColumns8D7580 | In outline: another terrain column pass like Terrain::Columns8D6FC0, removing columns from n on |
| `0x8d8080` | Terrain::ShrinkRows8D8080 | In outline: the row form of Terrain::ShrinkColumns8D7580 |
| `0x71b500` | Path::TileAt71B500 | In outline: finds a path tile at a cell by case, checking what is already there (Entries::FindByKey kind 0x1F) |
| `0x567720` | Path::Update567720 | In outline: a path piece's update when not flagged (+0x5C), by its kind (+0x10 values 5, 7, 8; queues 0xD) |
| `0xaba730` | Fence::RebuildABA730 | In outline: rebuilds a border when on (+0xE8): clears it (`0xAB6050`), builds it (Fence::Build) and finishes (`0xAC67C0`), with a case for kind 4 |
| `0x8e6330` | Terrain::Blend8E6330 | In outline: works out blend values for a terrain point, each kept between 0 and 1 |
| `0x83d1f0` | Scenery::Check83D1F0 | In outline: checks a scenery item at a place (+0x16C) and whether a key is held (KeyState::Held) |
| `0x439760` | Files::Find439760 | In outline: lists files matching a pattern (`0xBE5570`) like Files::FindB936D0 |
| `0x811be0` | Scenery::Insert811BE0 | In outline: adds a scenery entry to the sorted table and index (SortedTable::LowerBound1C, SortedIndex::LowerBound), with its box (Rect::Assign) |
| `0x552240` | Ride::Parts552240 | In outline: goes over a ride's parts and refreshes the ones not flagged (+0x18, +0x14 +0x40 bit 1) |
| `0x8de960` | Water::Fill8DE960 | In outline: fills water over the terrain up to a level (WaterManager::Get), within a count (+0x44) |
| `0xac0b70` | Fence::RebuildAreaAC0B70 | In outline: rebuilds the border pieces of an area (+0x24 to +0x30, Fence::RebuildRect) and checks the terrain under them |
| `0xaac670` | GridTool::PickAAC670 | In outline: finds the cell under the mouse for a grid tool (MainCamera::Get, `0xCD79C0`) on the terrain (a 90 by 90 grid) and makes a 0x24-byte record for it |
| `0x8d8b40` | Terrain::GrowColumns8D8B40 | In outline: another column growing pass like Terrain::GrowColumns8D85C0 |
| `0x8bf330` | Terrain::Blocks8BF330 | In outline: goes over the 32-cell blocks of a rectangle and the layers of each (+0x44), keeping the height ranges |
| `0x8ae830` | Scenery::RatingValue8AE830 | In outline: a spot's rating value: the falloffs of its values (Curve::Falloff with 1 and 0.5) multiplied together |
| `0x8295a0` | Scenery::Move8295A0 | In outline: moves a scenery item when not flagged (+0x1C, +0x24) and the move is long enough (`0x9D4EA0`) |
| `0x79b9f0` | Staff::FindTaskNear | In outline: finds a task near a point within a distance for a staff member: the path tile there (PathManager, `0x71B370`) on the terrain |
| `0x9a43b0` | Fireworks::List9A43B0 | In outline: fills a list from a firework display's parts (+0xF4, Get_+0x108; `0x98DEB0`) |
| `0xa91e70` | GridTool::CellUnderMouse | In outline: the grid cell under the mouse (MainCamera::Get, `0xCD79C0`) on the terrain, with its side and corner |
| `0x813910` | Regions::Add813910 | In outline: adds a region with its box (Rect::Assign): makes its records (0x20 bytes, `0x80E260`) and its list |
| `0x7a2230` | Animal::FindHome7A2230 | In outline: finds the enclosure for something (`0xAA0840`, EnclosureManager::Get, `0xB98D50`) |
| `0x8da280` | Water::Make8DA280 | In outline: makes water over an area (WaterManager): its size from the area (half the sum of its sides, square-rooted), a random value and its part (`0x8277A0`) |
| `0x6f5a50` | Park::Refresh6F5A50 | In outline: refreshes a list of things (`0x6F5680`, `0x6F11E0` with 50) and the ones linked to it (`0x12F51B4`) |
| `0x8b79c0` | SceneryBehaviour::Aim8B79C0 | In outline: when not flagged (+0x54), points a scenery behaviour along its owner's direction (SceneryBehaviour::Owner, Matrix::LookAlongZSide) |
| `0x9a05e0` | Fireworks::Load9A05E0 | In outline: for a firework entry (Table104::Entry on +0x194) high enough, builds its file path from the install folder text (`0x1106DC4`) and loads it |
| `0xa92520` | GridTool::UpdateA92520 | In outline: another grid tool update like GridTool::UpdateA92E40, when not flagged (+0x1E0, +0x114) and not in mode 1 |
| `0xb70450` | WildAnimal::CountB70450 | In outline: counts the animals (`0x6C3BE0`) on the terrain (a 90 by 90 grid), giving 0 when there are none |
| `0x891100` | Scenery::Fit891100 | In outline: checks a scenery item's box against the area (+0x30 to +0x40) and places it (`0x88F4F0`) within its slots (+0x394) |
| `0x91df50` | Water::Cell91DF50 | In outline: the water cell for a point on the terrain, rounded |
| `0x843d60` | Scenery::Flatten843D60 | In outline: flattens the ground for a scenery item when its option is on (TerrainFlattener::Get, `0x10EDC38`, `0x1313C64` bit 2) |
| `0x7c53e0` | Terrain::HeightAt7C53E0 | In outline: a terrain height at a point (a 90 by 90 grid), rounded to its cell |
| `0x8e0db0` | Terrain::Even8E0DB0 | In outline: another height evening like Terrain::Level8E16E0, over +0x197 heights, joining the ones within a small tolerance |
| `0x46fca0` | Tile::Corners46FCA0 | In outline: works out a tile's corner heights from its packed bits (four levels less the bits set) |
| `0x7f1a20` | Park::Step7F1A20 | In outline: a step over a list when not started (+0x44 below 0, `0x7EA190`), up to 0xB entries |
| `0x4d0490` | Ride::Link4D0490 | In outline: links a ride part (`0x501F80`, `0x4B52D0`) when not flagged (+0x4C) |
| `0x797d70` | Guest::FindPathNear | In outline: finds a path near a guest on the terrain (a 90 by 90 grid, `0x7C2FF0`) with the path manager |
| `0x592da0` | Shop::Refresh592DA0 | In outline: when not flagged (+0x204) and with a slot (+0x23C), refreshes a shop's items (`0x4CDE30`, `0x4D0CB0` with 4 and the next rotation) |
| `0xab23c0` | PoolComplex::PlaceAB23C0 | In outline: places a pool part by its kind (0, ...) when asked, with its record (`0xAAE430`, `0x13CE3F8`) |
| `0x8bea40` | Terrain::Blocks8BEA40 | In outline: another terrain block pass like Terrain::Blocks8BF330, keeping the lowest values |
| `0x6b6fc0` | Path::Check6B6FC0 | In outline: checks the path (PathManager::Get) over the terrain by its result (1, ...) |
| `0xaba180` | Fence::UpdateABA180 | In outline: goes over a border's pieces while not flagged (+0x1B0) and it has some (+0x1AC) |
| `0x7a13c0` | Path::CanUse7A13C0 | In outline: whether a path tile can be used, by its kind (0, 4, 0xE, 0xF, 0x13 handled) and the mask (+0x408) |
| `0xb9d510` | PathGrid::RebuildB9D510 | In outline: another path grid rebuild over the terrain (a 90 by 90 grid) |
| `0xabe3c0` | Fence::CopyABE3C0 | In outline: copies a border's values into a new buffer for its range (+0x50 to +0x58) |
| `0x9026f0` | Water::SetMode9026F0 | In outline: switches the water's mode (+0x40) by case (1, ...) with the water manager |
| `0x4ce150` | Shop::NameText4CE150 | In outline: a shop's name text, or an empty text when it has none (`0x4CDF90`) |
| `0x847c90` | Scenery::Place847C90 | In outline: places a scenery item (`0x827510`, `0x8470F0`) when its parts are given, with its pieces (+0x14) |
| `0x581f20` | Tool::Make581F20 | In outline: makes a tool's preview object (`0x581660`, `0x12F5638`, `0x5D4930`, `0xBCD1F0`, `0x86D3A0` with kind 0x17) |
| `0xafb2d0` | TerrainLayers::FindAFB2D0 | In outline: finds the terrain layers at a cell (`0xAFB160`) and keeps the ones above a height |
| `0x8c0680` | Terrain::Edge8C0680 | In outline: builds a terrain edge piece between two heights (`0x8C0170`, `0x8BE610`) |
| `0x7d5f80` | Person::Ground7D5F80 | In outline: when flagged (+0x120), keeps a person above the terrain height under it, rounded |
| `0x576270` | Ride::Update576270 | In outline: a ride part's update when not flagged (+0x5C), except for kind 0x44 |
| `0x8195d0` | Path::Near8195D0 | In outline: finds the path tile at a point (PathManager, `0x71B370`) on the terrain (a 90 by 90 grid) |
| `0x71dfb0` | Path::CanPlace71DFB0 | In outline: whether a path piece can go at a cell inside the grid (+0x5C, +0x5D), checking heights (+0xA8) over the terrain |
| `0x595ff0` | Ride::RefreshLinks595FF0 | In outline: goes over a ride's links and refreshes the ones not flagged (+0x18) |
| `0x876810` | Scenery::Load876810 | In outline: goes over a list of scenery items loading each one (`0x8826F0`) |
| `0x5a05c0` | Ride::CanOpen5A05C0 | In outline: whether a ride can open (`0x5CFB10`), showing a message when not |
| `0x8db050` | Terrain::Setup8DB050 | In outline: sets up a terrain edit over the terrain (a 90 by 90 grid) with its record (+0x7C) and a fresh id set (`0x12F4634`, IdSet::IdSet) |
| `0x4b1920` | Shop::Queue4B1920 | In outline: another shop queue pass like Shop::Update4B2F50: lets the next waiting person in (kind 3, `0x4D2800`) when there is room |
| `0x8e7260` | Terrain::Cell8E7260 | In outline: the terrain cell under a point, rounded (register-passed point) |
| `0xafccc0` | TerrainLayers::CornersAFCCC0 | In outline: works out the corner points of the terrain layers at a cell (`0xAFB160`) into a four-entry table |
| `0x6bce00` | Money::CheckCost | In outline: whether the park can pay an amount: the case of a held key (0x2D) with +0x68 bit 1 clear is handled first; otherwise, unless told not to check, gives 0 when the amount is above the money (+0x58) |
| `0x8b1d30` | Scenery::Links8B1D30 | In outline: goes over a scenery item's links (`0x771DD0`) |
| `0x849f70` | Scenery::Update849F70 | In outline: a scenery item's update when not flagged (+0x1C), with no time passing handled |
| `0x7a7e40` | Park::Spot7A7E40 | In outline: unless set (`0x1306C6C`), finds a spot on the terrain (a 90 by 90 grid, `0x7BE2F0`) |
| `0x82e8f0` | Scenery::UpdateList82E8F0 | In outline: goes over a list of scenery items updating the ones not flagged (+0x1C) |
| `0x7096e0` | Terrain::Edit7096E0 | In outline: a terrain edit over a rectangle when not flagged (+0x30) |
| `0x5db4e0` | TrackedRide::Walk5DB4E0 | In outline: walks a ride's track pieces until the end (+0x14 vs +0xC +0x20) when not flagged (+0x18C) |
| `0x872210` | Scenery::Kind1A872210 | In outline: unless off (Flags::Bit0), handles a scenery item of kind 0x1A |
| `0x79b5d0` | Path::Distance79B5D0 | In outline: the distance along the path from a point (PathManager, `0x71B370`) on the terrain |
| `0x4ef280` | Ride::PlaceAt4EF280 | In outline: places a ride or shop by its kind (+0x144: 0, ...) at a scenery footprint's centre (SceneryItem::FootprintCentre), pointing it (Matrix::LookAlongZ) unless flagged (+0x1A7) |
| `0x8b4b70` | Path::Distance8B4B70 | In outline: another distance along the path like Path::Distance79B5D0 |
| `0x4f5530` | Ride::Finish4F5530 | In outline: finishes a ride's placement when it has a part (+0x5D0): refreshes (`0x5D1F20`, `0x4F3980`) and for kind 4 with a piece (+0x660) sets its defaults (Settings5D::Defaults) |
| `0xa03630` | Terrain::FlagsA03630 | In outline: goes over terrain cells checking a flag (bit 8 of +0x12, 0x18 bytes each) over the terrain (a 90 by 90 grid) |
| `0x559400` | Ride::Check559400 | In outline: a ride's check when not flagged (+0x18) |
| `0x4d3440` | Ride::Act4D3440 | In outline: acts on a ride by its mode (+0xB0) when its part allows it (slot 0x108) |
| `0x557ee0` | Ride::Cells557EE0 | In outline: the terrain cells a ride part covers (Grid::CellAt from its position) for its pieces (+0x98) |
| `0x83b480` | Scenery::Resize83B480 | In outline: when a scenery grid's size changes (+0, +4), releases its old parts (+0x2C) and makes new ones |
| `0xa9a030` | GridTool::PickA9A030 | In outline: another cell-under-the-mouse pick like GridTool::CellUnderMouse (MainCamera::Get, `0xCD79C0`) on the terrain |
| `0x847870` | Scenery::Check847870 | In outline: a scenery check: passes when off (`0x10EDC68`) or its own check passes (slot 0x44), otherwise checks the terrain |
| `0x586b40` | Shop::Refresh586B40 | In outline: when not flagged (+0x1EC), refreshes a shop's items for each of four turns (`0x4D0CB0` with 4) |
| `0xab16c0` | Fence::CornersAB16C0 | In outline: when it has no piece (+0x120) and is not flagged (+0x20C), places the four corner pieces (`0xA90B00` at minus and plus 2) |
| `0x56b290` | Ride::UpdateTool56B290 | In outline: another ride tool update like Ride::UpdateTool56B690, when not flagged (+0x5C, +0xF0) |
| `0x54e720` | Ride::Breakdown54E720 | In outline: a ride's chance check (`0x4AA5B0`) over its parts' values (+0x60, count +0x5C) with a random roll (Random::Ran0) |
| `0x492730` | Terrain::CellOK492730 | In outline: whether a terrain cell is usable (a 90 by 90 grid), 0 for a cell outside it |
| `0x717400` | Path::Check717400 | In outline: checks a path piece's parts (+0x214) over the terrain (a 90 by 90 grid) |
| `0xab1f20` | Fence::HeightAB1F20 | In outline: a border piece's height by its kind (+0x120: 2, ...) over the terrain |
| `0x83a160` | Scenery::Footprint83A160 | In outline: works out a scenery item's footprint (Placement::Footprint) for kinds 3, 5 and 7 (not 1, 2, 6, 8), from its cells (`0x838B60`, `0x838AC0`) |
| `0x9485c0` | Water::Grow9485C0 | In outline: grows the shared water list (`0x133A2E4`, `0x940800` with 0x100) and adds the pieces given over the terrain |
| `0x492350` | Terrain::FlatEnough | In outline: whether a terrain cell is flat enough: its height range (Terrain::CellHeightRange) within a limit |
| `0x4b1ec0` | Shop::Update4B1EC0 | In outline: goes over a shop's people when not flagged (+0x3C4), checking each (Get_+0x20, `0x4AB750`) |
| `0x7abef0` | Person::Route7ABEF0 | In outline: another route finder like Person::Route7AB810 (PathManager, `0x71B370`, `0x7A6A40`) |
| `0x720a20` | Path::Slope720A20 | In outline: a path cell's slope from its height range (Terrain::CellHeightRange) |
| `0x8b6a80` | SceneryBehaviour::Aim8B6A80 | In outline: another scenery behaviour aim like SceneryBehaviour::Aim8B79C0, without a slot (+0xDC = -1) |
| `0x719080` | Path::Tool719080 | In outline: a path tool's update: refreshes (`0x5F79D0`) and, when on (+0x114) and not flagged (+0x298, +0x2DC), acts (`0x717260`) |
| `0x417180` | Regions::Set417180 | In outline: sets a region's value by its kind (the high 16 bits, 1 to 8) |
| `0xa8a6c0` | PoolComplex::PartsA8A6C0 | In outline: goes over a pool complex's parts (Table30::Entry) counting them (`0xA893B0`) |
| `0x714910` | Path::PlaceAtMouse | In outline: places a path piece at the ground under the mouse (MainCamera::Get, `0xCD79C0`), or the plain way (`0x713460`) |
| `0x678f10` | Scenery::Range678F10 | In outline: goes over a range of entries checking flag 1 (+0x94) by its mode (bits 0x60 = 0x40) |
| `0x5bb490` | Ride::Count5BB490 | In outline: counts a thing by kind (Counts::AddByKind) and, unless it passes a check (`0xBC4200`), compares its slot (+0x1FC) when not flagged (+0x201) |
| `0xab57a0` | PoolComplex::LinkAB57A0 | In outline: links a pool piece (`0xAAF2E0`) when not flagged (+0x168) |
| `0x8e2c50` | Terrain::Edit8E2C50 | In outline: a terrain edit when a value changed (+0xFC vs +0xF4), by its flags (+0x1C, +0x1D) |
| `0x891a20` | Scenery::Insert891A20 | In outline: adds a scenery entry into a sorted table by kind (SortedTable::LowerBoundKind), moving the rest (`0x88D160`) |
| `0x839290` | Scenery::Shapes839290 | In outline: goes over a scenery set's parts (+0xE4, count +0xE8) of kinds 0, 2, 3 and 4 checking their shapes (Model::CheckShapes) |
| `0x844c40` | Scenery::Collect844C40 | In outline: makes a list of the scenery parts near a point (`0x84C160`) |
| `0x7223a0` | Path::Link7223A0 | In outline: goes over a path's points (Path::Points) checking each link the other way (`0x720F70`) |
| `0xb963f0` | PathGrid::UpdateB963F0 | In outline: goes over a path grid's entries (+0xEC) and checks the ones not flagged (+0x1C) |
| `0xaa6d40` | PoolComplex::UpdateAA6D40 | In outline: goes over a pool complex's parts (`0xA85490`) updating the linked ones |
| `0xab1020` | Fence::PieceAB1020 | In outline: a border piece by its kind (+0x120: 0, 4, ...) when not flagged (+0x194) |
| `0x8dbd00` | Terrain::Smooth8DBD00 | In outline: another terrain smoothing pass like Terrain::Smooth8DC170 over a rectangle (bounds at +4 to +7) |
| `0x708f50` | SceneryBehaviour::PathUnder | In outline: finds the path tile under a scenery behaviour's owner (SceneryBehaviour::Owner, PathManager, `0x71B370`) |
| `0xab0c50` | Fence::PieceAB0C50 | In outline: another border piece by kind like Fence::PieceAB1020 |
| `0x464060` | Scenery::Check464060 | In outline: goes over a list's entries (+8, count +0xC) checking each against the shared table (`0x1106E38` +0x18) |
| `0x4e1680` | Ride::Flatten4E1680 | In outline: when a ride has a part (+0x280), sets it up (`0x4E1050`, `0xA06E30`) and flattens the ground for it (Scenery::Flatten843D60) |
| `0x5e9d60` | TrackPiece::Check5E9D60 | In outline: a track piece check: blocked (`0x5D9720`) or, when asked, for the selected ride (`0x12F5608`) |
| `0x4cfc30` | Shop::Items4CFC30 | In outline: refreshes a shop's items when it has a name (`0x4CDF90`) and is not flagged (+0x50) |
| `0x5e43b0` | TrackedRide::Check5E43B0 | In outline: checks a ride's track (TrackedRide::TrackInterface) over its pieces (+0xE8) when not flagged (+0x68) |
| `0x5d8610` | Track::CanJoin5D8610 | In outline: whether two track pieces' ends meet: works out their ends (`0xBC9430`) and checks them (Track::JoinBC6F00) |
| `0x5d9e80` | TrackedRide::Check5D9E80 | In outline: checks a ride's track (`0x59AF60`, TrackedRide::TrackInterface) once a time (+0x10C) has passed |
| `0x948220` | Water::FillTo948220 | In outline: fills water up to a level (rounded by `0x1106D08`) where it can spread (Water::Flood942A70) |
| `0x907cc0` | Water::CellOK907CC0 | In outline: whether a cell of a water grid (+0) is inside it, with the odd and even rows handled |
| `0x8e3690` | Water::SetCell8E3690 | In outline: sets a water cell inside the terrain (WaterManager::Get) |
| `0x4afa30` | Shop::Section4AFA30 | In outline: for a shop on a track, works out its section (`0x12F5338`, its member at +0x38) |
| `0x718370` | Path::Place718370 | In outline: works out where a path piece goes (`0x714120`, `0x7135A0`, `0x4CC8F0`) over the terrain |
| `0x720670` | Path::Update720670 | In outline: a path piece's update by its case over the terrain |
| `0x4627a0` | Scenery::Activate4627A0 | In outline: with a reference (WithReference45FA50), activates the entries of a list (+0x48, count +0x4C) not flagged (+0x18) |
| `0x5766d0` | Ride::Update5766D0 | In outline: a ride's update (`0x56F080`, `0x5613A0`, `0x561420`) with cases for kinds other than 0xD, 0xF, 0x16 |
| `0x4cf8a0` | Shop::Items4CF8A0 | In outline: another shop item pass like Shop::Items4CFC30 |
| `0x8b9fe0` | Scenery::Check8B9FE0 | In outline: a scenery check when asked and not flagged (+0x58), with a part (+0x54) |
| `0x56f080` | Ride::Check56F080 | In outline: checks a ride with the current tool (`0x12F5638`, `0x5DBCD0`) and a value below 0 |
| `0x8d3ce0` | Terrain::SaveCell | In outline: writes a terrain cell to a save (SaveWriter::Write: four values, then bytes) |
| `0x946800` | Water::SaveBody | In outline: writes a water body to a save (SaveWriter::Write: four values, then its bytes) |
| `0x9951c0` | Fireworks::Ground9951C0 | In outline: finds the terrain under a firework point (`0x8D6E90`) |
| `0xa97930` | Fence::UpdateA97930 | In outline: goes over a border's pieces (+0x28, count +0x2C) whose value (+0x14) is below 0 and links them |
| `0x99e490` | Fireworks::PlaceRow99E490 | In outline: snaps a firework list's point to a row (Grid::SnapToRow) and checks the slot (Get_+0x108, `0x13CE5EC`) |
| `0xb9c230` | PathGrid::MarkB9C230 | In outline: goes over a path grid (+0x64 by +0x68) marking the cells that match the ids given |
| `0xac0560` | Fence::RebuildPlay | In outline: rebuilds the border pieces around a rectangle (Fence::RebuildRect one cell wider) and plays the earth thump sound (EarthThump::Sound) |
| `0x946be0` | Water::LoadBody | In outline: reads a water body from a saved block (`0x809210`, four bytes then single bytes), checking the block's size |
| `0xb9dae0` | PathGrid::BuildB9DAE0 | In outline: another path grid pass over the terrain (a 90 by 90 grid) |
| `0xab35b0` | PoolComplex::PlaceAB35B0 | In outline: places a pool part by its mode (+0x130: 1, ...) with its record (`0xAAE430`) |
| `0xab3220` | PoolComplex::PlaceAB3220 | In outline: the same pool part placement (another copy) |
| `0xaa4180` | PoolComplex::SidesAA4180 | In outline: when not flagged, works out a pool piece's sides (`0xAA0CE0` on +0x18, +0x1C) and turns them (Sides::Rotate) |
| `0x565a60` | Path::Queue565A60 | In outline: a queue's update when not flagged (+0x98) and its length (+0x70) is above 0 |
| `0x937b20` | Water::Grid937B20 | In outline: checks water grid cells (one in three of +0x18) against an id grid (Grid::ContainsId with 0xFFFF) |
| `0x51e990` | Ride::Update51E990 | In outline: a ride's update when not flagged (+0x180) with its owner (Interface48b::Get) |
| `0x71cc40` | Path::Ground71CC40 | In outline: finds where a ray meets the ground (`0x8BC700`) for path placement |
| `0xb9f120` | PathGrid::AnimalB9F120 | In outline: a path grid check for an animal (WildAnimal::Interface60) on the terrain |
| `0xaad930` | Fence::PiecesAAD930 | In outline: another pass over a border's packed piece values by their turn bits (0xC0) like Fence::PiecesAAFC00 |
| `0xab2ec0` | PoolComplex::PlaceAB2EC0 | In outline: another pool part placement by mode like PoolComplex::PlaceAB35B0 |
| `0xabff60` | Fence::CellsABFF60 | In outline: goes over a rectangle's cells inside an area (bounds +0x4C to +0x58) handling each (used by the grid tool) |
| `0x4c7750` | Tool::Slot4C7750 | In outline: finds the current tool's slot (`0x12F5638`, `0x5E1010`) and acts when there is one |
| `0x86f890` | Scenery::Near86F890 | In outline: goes over the cells within seven of a point |
| `0x5d2170` | Ride::CanOpen5D2170 | In outline: whether a ride can open (`0x5CFB10` on its +0x18) |
| `0x80ef60` | Scenery::Find80EF60 | In outline: finds scenery near a point over the terrain (a 90 by 90 grid) |
| `0x95b8d0` | Water::Body95B8D0 | In outline: sets up a water body (`0x95A150`, `0x493860`) over n cells |
| `0xba4550` | Enclosure::UpdateBA4550 | In outline: goes over an enclosure's entries (+0x68) |
| `0x494590` | Water::At494590 | In outline: whether there is water at a cell (WaterManager, `0x93DBF0`) |
| `0x55a960` | Scenery::PlaceSingle | In outline: places one scenery item (SceneryItemPlacementSingle) when its part is not flagged (+0x18) |
| `0xba05d0` | Scenery::SetupBA05D0 | In outline: sets up a scenery item (`0x86DF00`, `0x865500`, Set_+0xfc) |
| `0x82a890` | Scenery::Kind82A890 | In outline: handles a scenery item by its kind (+0x48) with what is there (`0xAFD9D0`) |
| `0xabb3c0` | Fence::RebuildABB3C0 | In outline: clears a border's list (ObjectArray28::Resize) and builds its pieces again (Fence::PiecesAAFC00 on +0x128, +0x12C) |
| `0x94ae40` | Water::Terrain94AE40 | In outline: a water pass over the terrain (a 90 by 90 grid) |
| `0x727050` | Terrain::Edge727050 | In outline: works out the edge between two corners of a tile (0 to 3, different) over the terrain |
| `0x5c42b0` | Scenery::Link5C42B0 | In outline: sets up a scenery item (`0x84BB60`) and links its owner (+0x138) |
| `0x5d8ce0` | TrackPiece::Refresh5D8CE0 | In outline: refreshes a track piece (`0x5C45E0`) for the selected ride (`0x12F5608`) |
| `0x5d89c0` | TrackPiece::Refresh5D89C0 | In outline: the same track piece refresh (another copy) |
| `0x5114b0` | Ride::Parts5114B0 | In outline: goes over a ride's parts (+0x95C, count +0x960) handling the shown ones with a slot |
| `0x5a97b0` | Ride::Update5A97B0 | In outline: a ride's update when not flagged (+0x324) (`0x59C8A0`) |
| `0x8bc700` | Terrain::RayGround | In outline: where a ray meets the ground (used by Path::Ground71CC40), rounded to a cell |
| `0x8e5170` | Terrain::Mark8E5170 | In outline: marks terrain cells (0x28) |
| `0xaabdf0` | PoolComplex::GroundAABDF0 | In outline: places a pool part on the terrain when not flagged (+0x1C) |
| `0xa606c0` | Tool::PickA606C0 | In outline: finds what is under the mouse (`0xCD79C0`, `0x48C300`) |
| `0x9124d0` | Water::Table9124D0 | In outline: builds a table of n water entries |
| `0x825da0` | Scenery::Update825DA0 | In outline: finds a scenery item (`0x825A00`) and refreshes it |
| `0xb9a210` | PathGrid::RectB9A210 | In outline: goes over a path grid rectangle |
| `0x9eb030` | Fireworks::Ground9EB030 | In outline: checks a firework's height over the terrain |
| `0xabdbb0` | PoolComplex::WaterABDBB0 | In outline: goes over a pool's rectangle of cells with the water manager |
| `0xaaf900` | Fence::NeighboursAAF900 | In outline: when not flagged (+0x178), refreshes a border cell's neighbours (`0xABD1C0` either side) |
| `0x88d160` | Scenery::Copy88D160 | In outline: copies a scenery entry with its two boxes (Rect::Assign) |
| `0xbaf160` | Enclosure::ValueBAF160 | In outline: an enclosure value for entry n (below +0x204 +0x44) when not flagged (+0x208) |
| `0x8f9db0` | Terrain::Rect8F9DB0 | In outline: a terrain pass over a rectangle (up to +0x3B) |
| `0x7cc820` | Person::Check7CC820 | In outline: a person's check (Set_+0xc, `0x46F8E0`) |
| `0x77dec0` | GuestGroup::Pay77DEC0 | In outline: a group's check against the park (Park::Get, `0x6C1DA0`) over its members (+0x48) |
| `0x46f240` | Terrain::Near46F240 | In outline: whether two points are near each other over the terrain |
| `0xaf55a0` | Water::JetAF55A0 | In outline: starts a water effect n (`0x94EEB0` with 1, `0x94EF60`) when there is one |
| `0x436eb0` | Weather::Step436EB0 | In outline: moves a timed value on by its table (+0x1D0 by +0x2D8) when a slot matches (+0x2D4) |
| `0xb9ca60` | PathGrid::FindB9CA60 | In outline: finds an entry of a path grid matching an id (+0xAC) |
| `0x8eb0f0` | Water::Build8EB0F0 | In outline: builds water entries up to a count (+0x120) when asked |
| `0x95b320` | Water::Spread95B320 | In outline: spreads water from a cell over the terrain (`0x95A570`) |
| `0xa05e10` | Path::BuildA05E10 | In outline: builds a path from a list (growing `0x417C20` past +0x60) with the path manager |
| `0x4aa8e0` | Shop::Item4AA8E0 | In outline: makes a shop item when it is not set (Entry::IsSet) |
| `0xba2ac0` | PathGrid::RectBA2AC0 | In outline: sets up a path grid area (`0xBA0340` for its size) and goes over its cells (+0x64 by +0x68) |
| `0x582e70` | Ride::Ground582E70 | In outline: works out a ride part's height over the terrain (used by PlacementCursor::Update583360) |
| `0x9ff490` | Fireworks::Terrain9FF490 | In outline: a firework pass over the terrain (growing `0x417C20`) |
| `0x5dad90` | TrackPiece::Range5DAD90 | In outline: a track piece range check (`0x59D8C0`) |
| `0x72e420` | Path::Check72E420 | In outline: a path check by its kind (+0xD0 above 3) |
| `0xac2100` | PoolComplex::BoxAC2100 | In outline: sets a pool part's box (Box::Assign, `0xAC1B00`) and goes over its range when flagged (8) |
| `0x85cc50` | Shadows::Terrain85CC50 | In outline: sets up the light view (`0x85CB30`) over the terrain |
| `0x72d2d0` | Path::Rect72D2D0 | In outline: grows a path's changed rectangle (Rect::AddCell) with the path manager |
| `0x6fd860` | Scenery::Link6FD860 | In outline: links a scenery item (`0x829AA0`) when given one |
| `0x8f93a0` | Terrain::Below8F93A0 | In outline: finds the terrain under a point (`0x8D6E90`, +0x130) |
| `0x7bb180` | Path::Rect7BB180 | In outline: goes over a rectangle of cells with the path manager |
| `0x5844a0` | Ride::Ground5844A0 | In outline: when shown (+0x250), checks a ride part over the terrain |
| `0xa996c0` | SceneryBehaviour::UpdateA996C0 | In outline: activates and goes over a behaviour's owner's parts (SceneryBehaviour::Owner) |
| `0x904e30` | Water::Find904E30 | In outline: finds a water entry by its slot within its range |
| `0x71e6c0` | Path::Kind71E6C0 | In outline: handles a path piece by its kind (0, ...) |
| `0xac1bf0` | PoolComplex::PieceAC1BF0 | In outline: a pool piece check (`0xAC1B90`) unless told not to |
| `0x7286f0` | Terrain::Pass7286F0 | In outline: a pass over the terrain |
| `0x446080` | Terrain::Pass446080 | In outline: another pass over the terrain between two points |
| `0x9645d0` | Fireworks::Update9645D0 | In outline: goes over a firework list's parts (+0x10) that are on (+8) |
| `0x8eb3e0` | Water::Build8EB3E0 | In outline: another water build like Water::Build8EB0F0 |
| `0x491fa0` | Terrain::Check491FA0 | In outline: checks entries up to a count (+0x23C) (`0x865410`) |
| `0x4b7360` | Shop::Range4B7360 | In outline: checks a shop's distances against 150 and 250 |
| `0x4c7140` | Shop::Random4C7140 | In outline: in mode 1 (+0xC0), picks a random value in a range (Random::Ran0Range) |
| `0x71ef40` | Path::CanPlace71EF40 | In outline: whether a path piece can go at a cell (`0x71DCE0`), when the counts allow (`0x130634C`, `0x1306350`) |
| `0x798370` | Guest::PathNear798370 | In outline: a guest's path within 40 (Path::Distance79B5D0 with 0x10) |
| `0x5e7240` | Ride::Part5E7240 | In outline: a ride part by n (`0x5D3C50`) |
| `0x4d1190` | Ride::Value4D1190 | In outline: for kind 0xE (+0x154), a value rounded from its part (+0x130 scaled) |
| `0xb9d240` | PathGrid::CellB9D240 | In outline: sets up a path grid cell's record |
| `0x5e40d0` | TrackedRide::Pieces5E40D0 | In outline: goes over a ride's pieces (+0xE8) when not flagged (+0x68) |
| `0x7c59e0` | Terrain::Check7C59E0 | In outline: a check over the terrain |
| `0x70ed50` | Name::Lookup70ED50 | In outline: acts by a name's hash value (0x1EE3A652 and others) |
| `0x536880` | Ride::Update536880 | In outline: a ride's update when not flagged (+0x200) |
| `0x5365b0` | Ride::Update5365B0 | In outline: a ride's update when not flagged (+0x1E4) |
| `0x5c2220` | Ride::Find5C2220 | In outline: finds a ride entry (`0x52EDD0`) |
| `0x4f6940` | Ride::Kind4F6940 | In outline: acts by a ride's kind (1 to 4), with a piece (+0x660) for kind 4 |
| `0x8260b0` | Scenery::Update8260B0 | In outline: finds a scenery item (`0x825A00`) and refreshes it |
| `0x5aa370` | Ride::Check5AA370 | In outline: a ride check when not being placed (+0x378) |
| `0x8da980` | Water::Release8DA980 | In outline: releases water entries past a count (+0xC) |
| `0x703e90` | Grid::Toggle703E90 | In outline: when a cell's flag changes (+0x64), refreshes the grid (`0x7036E0` over +0x6C by +0x70) |
| `0x549c50` | Ride::Position549C50 | In outline: a ride part's position (Object::Position) and its target (+0x280, +0x260) |
| `0xaab930` | PoolComplex::FindAAB930 | In outline: finds a pool part (`0xAC5BD0`) |
| `0x8e8a20` | Water::Make8E8A20 | In outline: makes a 0xE0-byte water record |
| `0x8e3a30` | Terrain::Pass8E3A30 | In outline: another terrain pass |
| `0x7e16d0` | Scenery::Link7E16D0 | In outline: links a scenery item when it has one |
| `0x736a90` | Ride::Refresh736A90 | In outline: refreshes a ride's part (`0x861050` with 0) when it has one (+0x44) |
| `0xaa4ff0` | PoolComplex::UpdateAA4FF0 | In outline: refreshes a pool part (`0xAA3A40`) and acts when it is on |
| `0x940b90` | Water::Check940B90 | In outline: a water check with the shared manager (`0x13063D0`) |
| `0x8d4390` | Terrain::Setup8D4390 | In outline: sets a terrain value (Set_+0x50) and sets it up (`0x8D3A90` with `0x10ED71C`) |
| `0x55b2d0` | Ride::Compare55B2D0 | In outline: compares two rides' values (`0x4AA7E0`) |
| `0x55a6a0` | Ride::Compare55A6A0 | In outline: the same ride comparison (another copy) |
| `0x4ffe20` | Ride::Kind4FFE20 | In outline: acts by a ride's kind (9 to 0xB, 3 or flag 0x40) |
| `0x828cb0` | Scenery::Footprint828CB0 | In outline: works out a scenery item's footprint (Placement::Footprint on +0xA0, +0xA4) when not flagged (+0x130) |
| `0x84bca0` | Scenery::Activate84BCA0 | In outline: activates a scenery item when not flagged (+0x5C) |
| `0x492f40` | Terrain::Value492F40 | In outline: a terrain value at a cell |
| `0x709e10` | Terrain::Edit709E10 | In outline: another terrain edit when not flagged (+0x30) |
| `0x4b0760` | Shop::Item4B0760 | In outline: makes a shop item when it is not set (Entry::IsSet) |
| `0xa9d4a0` | PoolComplex::ListA9D4A0 | In outline: goes over a pool complex's list (+0x54) |
| `0x713960` | Path::Kind713960 | In outline: handles a path piece by its kind |
| `0x71ec30` | Path::Kind71EC30 | In outline: handles a path piece by its kind (Interface4C::GetCached) |
| `0x72cde0` | Path::Refresh72CDE0 | In outline: refreshes a path piece (`0x82FB30`) |
| `0xbad9e0` | Rect::SidesBAD9E0 | In outline: goes over a rectangle's side cells (+0x1D8 of them) by its side (+0x1E4) |
| `0x73e180` | Staff::Spot73E180 | In outline: finds a spot for a staff member over the terrain |
| `0x8ae570` | Scenery::Rating8AE570 | In outline: part of a spot's rating, from its values (+0x20, +0x28, +0x30) |
| `0x4f4b70` | Placement::Check4F4B70 | In outline: another placement check like Placement::Check4F41C0 (+0x65C) |
| `0x4c9260` | Ride::Select4C9260 | In outline: when the thing given is the ride's (+0x180), acts on its part (+0x17C) |
| `0xac02b0` | Fence::CellsAC02B0 | In outline: goes over a rectangle's cells handling each |
| `0x55a280` | Ride::Compare55A280 | In outline: another ride comparison (`0x4AA7E0`) |
| `0x559830` | Ride::Check559830 | In outline: a ride check when not flagged (+0x34) |
| `0x72ed10` | Path::Update72ED10 | In outline: finds a path piece's kind (PathManager::Get, Path::Kind71EC30) |
| `0x90f8f0` | Water::Gather90F8F0 | In outline: gathers water pieces over the terrain (used by Water::Find946FC0) |
| `0x94a340` | Water::Reflection94A340 | In outline: goes over the water parts (+0x164) with their reflection value (`0x949210`) |
| `0x93dcd0` | Water::Range93DCD0 | In outline: finds a water range from n |
| `0x8f4b50` | Terrain::Rows8F4B50 | In outline: goes over a terrain patch's rows and columns |
| `0x463a00` | Scenery::Kind463A00 | In outline: for entry n of the shared table (`0x1106E38`), acts when not flagged (+0x35) and its kind is under 6 |
| `0xabd910` | Fence::CellsABD910 | In outline: goes over a rectangle's cells and counts them |
| `0x716da0` | Path::Ratio716DA0 | In outline: a path value from two values (+0x154, +0x158) when the fraction is under 1 |
| `0x7562b0` | Guest::SetMode7562B0 | In outline: sets a guest's mode (+0x121) when it changes, with its spot (`0x7578C0`) |
| `0x72df40` | Path::Update72DF40 | In outline: a path update when not flagged (+0x11C) |
| `0x80b260` | Scenery::Place80B260 | In outline: sets a scenery item's box (Box::Assign) and places it (`0x843D10`) |
| `0x4c89b0` | Shop::Check4C89B0 | In outline: a shop check (`0x4C7600`) |
| `0x7aa730` | Terrain::Check7AA730 | In outline: a check over the terrain |
| `0xac6870` | PoolComplex::MakeAC6870 | In outline: makes a pool part over the terrain |
| `0xae3470` | List::PassAE3470 | In outline: goes over a list's entries (+4) |
| `0xaa3a40` | PoolPiece::Sides | In outline: works out a pool piece's sides (PoolPiece::SideData) when it is on |
| `0xa915f0` | Fence::MakeA915F0 | In outline: makes a border piece, placing the corners (Fence::CornersAB16C0) when it has no slot |
| `0x84b5f0` | Scenery::Update84B5F0 | In outline: a scenery update when not flagged (+0x6C) |
| `0x9fa7b0` | Fireworks::Terrain9FA7B0 | In outline: another firework pass over the terrain |
| `0x6f7330` | VIP::Visit | In outline: when there is no current one (+0x74 +0xC), brings a VIP to the park (VIP::Arrive) |
| `0x9a4cd0` | Fireworks::Place9A4CD0 | In outline: places a firework item when high enough and its check passes |
| `0xb9c5f0` | PathGrid::PassB9C5F0 | In outline: goes over a path grid (+0x64 by +0x68) |
| `0x479d30` | Shop::Item479D30 | In outline: a shop item for slot 0 or 1 |
| `0x92c710` | Water::Jets92C710 | In outline: a water jet step when not flagged (+0x324) |
| `0xb621b0` | WildAnimal::CanStep | In outline: whether an animal can step to a point over the terrain |
| `0xaaf680` | Fence::NextAAF680 | In outline: when not flagged (+0x178), checks the next cell (`0xABD1C0` on +0x128, +0x12C) |
| `0x445750` | Effects::Slots445750 | In outline: goes over an effect list's slots (+0x14) |
| `0x8e4220` | Terrain::Cell8E4220 | In outline: a terrain cell step |
| `0x8656e0` | Scenery::Make8656E0 | In outline: makes a scenery part when it has a slot |
| `0x8eef10` | Water::Points8EEF10 | In outline: goes over a water piece's points (+8) |
| `0x8de6d0` | Water::Same8DE6D0 | In outline: whether two water ranges are the same (+0x48 vs +0x4C) |
| `0x88fcd0` | Scenery::Update88FCD0 | In outline: unless the global flag `0x1400306` is set, goes over a scenery list (+0x54) |
| `0xa29e80` | Guest::LinkA29E80 | In outline: links a guest's part |
| `0x80e650` | Regions::Find80E650 | In outline: finds a region within bounds |
| `0x7266b0` | Path::Update7266B0 | In outline: a path update with what is there (`0xAFD9D0`) |
| `0x92db40` | Water::List92DB40 | In outline: goes over water entries not flagged (+0x18) |
| `0x8ff1c0` | Whirlpool::Cell | In outline: finds a whirlpool's cell (Whirlpool::FindCell) unless told not to |
| `0xa7d8a0` | Animal::ActivateA7D8A0 | In outline: links (`0x4AA760`) and activates an animal |
| `0x7036e0` | Grid::Refresh7036E0 | In outline: refreshes a grid rectangle (negative starts handled) |
| `0x82b350` | Scenery::Cell82B350 | In outline: turns a scenery cell (Footprint::RotateCell by +0xAC) |
| `0x8bca20` | Terrain::Rect8BCA20 | In outline: goes over a byte rectangle of terrain cells |
| `0x849730` | Scenery::Pass849730 | In outline: goes over n scenery entries when not flagged (+0x6C) |
| `0x462b40` | Scenery::Slot462B40 | In outline: a scenery slot (0 to 6) value |
| `0x8b1690` | Scenery::Camera8B1690 | In outline: when on (+0x108), a scenery item's update by the camera (MainCamera::Get) |
| `0x715cc0` | Path::InGrid715CC0 | In outline: a path check for a cell inside the path manager's grid (+0x5C, +0x5D) |
| `0x5d5a40` | TrackPiece::Range5D5A40 | In outline: a track piece check within a count |
| `0x566390` | Path::Queue566390 | In outline: a queue (kind 0xD) pass |
| `0x7838d0` | GuestGroup::Slot7838D0 | In outline: a group's slot step |
| `0xa9a4c0` | PoolComplex::RoundA9A4C0 | In outline: a pool step when not flagged (+0x1B0), rounded |
| `0x589440` | Track::NameText | In outline: makes a track's name text (Track::Name) |
| `0x90aef0` | Water::Cell90AEF0 | In outline: a water cell from a point, rounded |
| `0x900b40` | Water::Below900B40 | In outline: whether a point is below a water surface |
| `0x833d30` | Scenery::Activate833D30 | In outline: activates a scenery item |
| `0x759280` | Guest::Value759280 | In outline: a guest's value step when its value (+0x40C) is above 0.5 |
| `0x89f200` | Scenery::Terrain89F200 | In outline: a scenery pass over the terrain |
| `0x8bc4a0` | Terrain::Height8BC4A0 | In outline: a terrain height at a point, rounded |
| `0x55af30` | Ride::List55AF30 | In outline: goes over a ride's list |
| `0x726a10` | Terrain::Tile726A10 | In outline: a terrain tile step |
| `0x80fcc0` | Regions::Point80FCC0 | In outline: a region check for a point when there are regions (+0x48, `0x130ACA4`) |
| `0x834260` | Scenery::Link834260 | In outline: links a scenery item's part (`0x834190`) |
| `0xb6f6d0` | WildAnimal::TerrainB6F6D0 | In outline: an animal check over the terrain |
| `0x939610` | Water::Make939610 | In outline: makes a water piece's two parts |
| `0x8c1740` | Terrain::Rect8C1740 | In outline: goes over a byte rectangle when not flagged (+0x70) |
| `0x752130` | Guests::Pass752130 | In outline: goes over the guests (Interface4C +0x48) |
| `0xb15a30` | WildAnimal::TerrainB15A30 | In outline: an animal check over the terrain |
| `0x89c780` | Scenery::Place89C780 | In outline: places a scenery item when its check passes (`0x8915F0`) |
| `0x721100` | Path::Join721100 | In outline: whether two path pieces join, by case |
| `0x4bc130` | Shop::Terrain4BC130 | In outline: a shop check over the terrain |
| `0x770370` | Guest::Far770370 | In outline: in mode 1 (+0xF8), a check against a distance of 250 |
| `0x7ae350` | Terrain::Make7AE350 | In outline: a step over the terrain |
| `0x8e25f0` | Terrain::Pass8E25F0 | In outline: a pass over the terrain |
| `0x8e1480` | Terrain::Pass8E1480 | In outline: another pass over the terrain |
| `0x8dba90` | Water::Point8DBA90 | In outline: a water point's distance by its level (+7) |
| `0x88fa80` | Scenery::Update88FA80 | In outline: when the global flag `0x1400306` is unset, goes over a scenery list (+0x54) |
| `0x853800` | Scenery::Cell853800 | In outline: acts when a cell matches its own (+0x28, +0x2C), by case |
| `0x83cfb0` | Scenery::Update83CFB0 | In outline: a scenery update unless flagged (+0x184, +0x1E8) |
| `0x7158f0` | Path::Case7158F0 | In outline: a path value by case (under 5) |
| `0x902050` | Water::Grow902050 | In outline: grows a water list (+0xB0) |
| `0x72e1e0` | Path::Refresh72E1E0 | In outline: refreshes a path rectangle (Path::Rect72D2D0) with the path manager |
| `0x915bb0` | Water::Range915BB0 | In outline: a water step for n inside a range (`0x10F0BA0`, +0x10) |
| `0x853dc0` | Scenery::Release853DC0 | In outline: in mode 1 (+0x14), releases a part |
| `0x81b850` | SceneryBehaviour::Start | In outline: makes a behaviour active (SceneryBehaviour::SetActive) and adds it to a slot pool (SlotPool::Add) |
| `0x86d130` | Scenery::Model86D130 | In outline: sets a scenery item's model (+0x4C) when it changes (`0x860790`) |
| `0x80f380` | Regions::Inside80F380 | In outline: with regions (`0x130AC70`), checks whether a box lies inside one |
| `0xb62aa0` | WildAnimal::TerrainB62AA0 | In outline: an animal check over the terrain |
| `0x9f8b40` | Fireworks::Bits9F8B40 | In outline: a step over a 64-bit value's parts (under 4) |
| `0x97b6f0` | Water::Find97B6F0 | In outline: finds a water entry for a cell (+8, +0xC) |
| `0x877cd0` | Scenery::Kind877CD0 | In outline: a scenery step for kinds 0xC to 0xF when flagged (bit 15) |
| `0x867c80` | Scenery::Has867C80 | In outline: whether a scenery list (+0xC +0x14) holds an entry |
| `0x7ce430` | Person::Terrain7CE430 | In outline: a person's step over the terrain by a value's sign |
| `0x884d20` | Scenery::Point884D20 | In outline: a point check against a scenery shape with more than three points |
| `0x88dc60` | Scenery::Terrain88DC60 | In outline: a scenery check over the terrain (`0x864620`) |
| `0x915df0` | Water::Update915DF0 | In outline: a water update unless flagged (+8 bit 8), with Water::Range915BB0 |
| `0x8e3d00` | Terrain::Cell8E3D00 | In outline: a terrain cell step |
| `0x8424d0` | Scenery::Link8424D0 | In outline: links a scenery part when given one |
| `0x7875d0` | GuestGroup::Remove7875D0 | In outline: when it is its own (+0x174), clears a group's part (+0x16C) |
| `0x75e860` | Guest::Link75E860 | In outline: links a guest's part when given one |
| `0x71c230` | Path::Update71C230 | In outline: when on (+0xA0), a path step with the shared count (`0x130634C`) |
| `0x80aee0` | Box::Merge80AEE0 | In outline: grows a box to hold another |
| `0x42d8e0` | Terrain::Point42D8E0 | In outline: a terrain step at a point |
| `0x90b7e0` | Water::Terrain90B7E0 | In outline: a water step over the terrain |
| `0x835050` | Scenery::Link835050 | In outline: a scenery check with a part when given one |
| `0x715f20` | Path::InGrid715F20 | In outline: another path check for a cell inside the path manager's grid (+0x5C, +0x5D) |
| `0x838e00` | Scenery::Kind838E00 | In outline: a scenery check by kind (+0x8B = 2) unless told not to |
| `0x96fae0` | List::Sort96FAE0 | In outline: sorts 0x18-byte entries by their first value |
| `0x80cba0` | List::Sort80CBA0 | In outline: the same sort (another copy) |
| `0x875630` | Scenery::Range875630 | In outline: a scenery step for a range under 0xE5 |
| `0x429b40` | Terrain::Below429B40 | In outline: a step when a height is below 0 |
| `0x88f4f0` | Scenery::Slot88F4F0 | In outline: a scenery step for a slot when it has one |
| `0x8545f0` | Scenery::Show8545F0 | In outline: a step when given an entry and the shared flag (`0x12F018C` +0xB0) is set |
| `0x854330` | Scenery::Find854330 | In outline: finds a scenery entry up to n |
| `0x8ff430` | Whirlpool::Find8FF430 | In outline: finds a cell in a sorted list (SortedPointers::FindXY3) |
| `0x72cbb0` | Path::Refresh72CBB0 | In outline: a path step with the path manager |
| `0x95c260` | Water::Terrain95C260 | In outline: a water check over the terrain |
| `0x95a7e0` | Water::Terrain95A7E0 | In outline: the same water check (another copy) |
| `0xaa03d0` | PoolComplex::OwnerAA03D0 | In outline: a pool step when a part's owner (+0x13C) is another |
| `0x8ff790` | Water::Find8FF790 | In outline: finds a water entry up to n |
| `0x837df0` | Scenery::Count837DF0 | In outline: a scenery step by a count (under 1 handled) |
| `0x877390` | Scenery::Terrain877390 | In outline: a scenery step over the terrain |
| `0xa929f0` | Fence::UpdateA929F0 | In outline: a border update when not flagged (+0xED) |
| `0x723c20` | Path::Full723C20 | In outline: a path step when a list is full (+0x70 = +0x6C) |
| `0x8f7500` | Terrain::Refresh8F7500 | In outline: refreshes a terrain part (`0x8F47F0`) |
| `0x8828a0` | Scenery::Pass8828A0 | In outline: goes over a scenery list (+0x14) |
| `0x95df40` | Water::Edge95DF40 | In outline: another water edge case, the fraction kept between 0 and 1 |
| `0x915990` | Water::Pass915990 | In outline: goes over water entries (+0x1C) |
| `0x7a0930` | Terrain::Check7A0930 | In outline: a check with a part when there is one |
| `0x902290` | Water::Grow902290 | In outline: grows a water list (+0xB0) |
| `0xa92c20` | Fence::PointA92C20 | In outline: a border step with a part (+0x198) |
| `0x944250` | Water::Activate944250 | In outline: activates a water part |
| `0x8ee9c0` | Water::Make8EE9C0 | In outline: makes a water part (`0x4377E0` with 0x1C) |
| `0x875880` | Scenery::Find875880 | In outline: finds a scenery entry (+0x80) |
| `0x85cfd0` | Scenery::Case85CFD0 | In outline: a scenery value by case |
| `0xac6000` | PoolComplex::PointAC6000 | In outline: a pool step when not flagged (+0x1C) |
| `0x9140a0` | Water::Below9140A0 | In outline: a step when a height is below 0 |
| `0x978510` | Water::Show978510 | In outline: a step when the shared flag (`0x12F018C` +0x24 +2) is set |
| `0x976ca0` | Water::Locked976CA0 | In outline: a water step inside a critical section |
| `0x8167e0` | SceneryBehaviour::Find8167E0 | In outline: a behaviour step for a slot |
| `0x42d4e0` | Terrain::Step42D4E0 | In outline: a terrain step |
| `0x855b90` | Scenery::Set855B90 | In outline: sets a scenery value (+0x50) when it changes |
| `0x8181b0` | SceneryBehaviour::Mode8181B0 | In outline: a behaviour step in mode 1 (+0x70) |
| `0x779d20` | Park::Step779D20 | In outline: a step with the park (Park::Get) |
| `0x943360` | Water::Terrain943360 | In outline: a water step over the terrain |
| `0x822880` | SceneryBehaviour::Loop822880 | In outline: a behaviour loop |
| `0x745e40` | Guest::Link745E40 | In outline: links a guest's part |
| `0xaa1ec0` | PoolComplex::ListAA1EC0 | In outline: goes over a pool's list |
| `0x839660` | Scenery::Slot839660 | In outline: a scenery step when it has a slot |
| `0x77e1c0` | Park::Find77E1C0 | In outline: finds an entry (+0x48) |
| `0x908250` | Water::Cell908250 | In outline: a water cell step inside bounds |
| `0x871cb0` | Scenery::Flag871CB0 | In outline: a scenery step by a flag (Flags::Bit0) |
| `0x886620` | Scenery::Find886620 | In outline: finds a scenery entry (`0x8847A0`) |
| `0x9e2730` | Fireworks::Terrain9E2730 | In outline: a firework step over the terrain |
| `0x91e410` | Water::Terrain91E410 | In outline: a water step over the terrain |
| `0x915780` | Water::Pass915780 | In outline: goes over water entries (+0x10) |
| `0x88dea0` | Scenery::Water88DEA0 | In outline: a scenery step with a water part (`0x90D380`) |
| `0x849ab0` | Scenery::Show849AB0 | In outline: a scenery step when on (+0x94) |
| `0xaea420` | Animal::UpdateAEA420 | In outline: an animal step when on (+0x2C) |
| `0x876fa0` | Scenery::Terrain876FA0 | In outline: a scenery step over the terrain |
| `0x72fa20` | Path::Slot72FA20 | In outline: a path value for n in range of its count (+1) |
| `0x72f610` | Path::Slot72F610 | In outline: a path value for n in range of its count (+0) |
| `0x88f1e0` | Scenery::Loop88F1E0 | In outline: a scenery loop |
| `0x97a4f0` | Water::Same97A4F0 | In outline: a step when two cells differ |
| `0x741a30` | Guest::Link741A30 | In outline: links a guest's part when given one |
| `0x726270` | PathGrid::Make726270 | In outline: sets up a path table (`0x488290` with 0x10) |
| `0x9197a0` | Water::Terrain9197A0 | In outline: a water check over the terrain |
| `0x812f50` | List::Range812F50 | In outline: a step over a range |
| `0x901600` | Water::Terrain901600 | In outline: a water step over the terrain |
| `0x870010` | Scenery::Part870010 | In outline: a scenery step with a part when it has one |
| `0x84bf60` | Scenery::Make84BF60 | In outline: makes a scenery record with an empty handle |
| `0x723e50` | Path::Index723E50 | In outline: makes a 20-byte-entry array |
| `0x733b90` | Path::Part733B90 | In outline: a path step with a part when it has one |
| `0x92c310` | Water::Mode92C310 | In outline: a water step in mode 1 |
| `0x913420` | Water::Random913420 | In outline: a random water step (Random::Ran0) |
| `0xac11d0` | Fence::GrowAC11D0 | In outline: grows a list (ObjectArray::SetCapacity) |
| `0x492cd0` | WaterManager::Make | In outline: makes the water manager (WaterManager::WaterManager) |
| `0x7538b0` | Guests::Next7538B0 | In outline: a step when a list's place differs from its end (+4, +0x10) |
| `0x72f820` | Path::Slot72F820 | In outline: a path value for a positive n |
| `0x72fc30` | Path::Slot72FC30 | In outline: a path value for a positive n |
| `0x9f8d80` | Fireworks::Bits9F8D80 | In outline: a step over a 64-bit value's parts |
| `0x955750` | Water::Grid955750 | In outline: a water grid step (+0x1C by +0x20) |
| `0x828990` | Scenery::Terrain828990 | In outline: a scenery step over the terrain |
| `0xabce40` | Fence::StepABCE40 | In outline: a border step (`0xABC4F0`) |
| `0x88efe0` | Scenery::Loop88EFE0 | In outline: a scenery loop |
| `0x77a9e0` | Park::Pass77A9E0 | In outline: goes over a list (+0x48) |
| `0x903840` | Water::Count903840 | In outline: a water step for a positive count |
| `0x774f30` | Guest::Count774F30 | In outline: a step when a count passes the shared value (`0x1306928`) |
| `0x95f1e0` | Water::Edge95F1E0 | In outline: another water edge case |
| `0x7f0d00` | Person::Slot7F0D00 | In outline: a step when it has a slot |
| `0x724580` | Terrain::Rect724580 | In outline: a terrain step over a byte rectangle |
| `0x98eb30` | Scenery::Refresh98EB30 | In outline: a refresh (`0x501FB0`) |
| `0x7a8270` | Terrain::Mode7A8270 | In outline: acts by a mode (+0x524) |
| `0x9933f0` | Fireworks::Check9933F0 | In outline: a check when not flagged (+0x1C) |
| `0x93e5e0` | Water::Find93E5E0 | In outline: a water find when it has a part (+0xB0) and is asked |
| `0x783e40` | GuestGroup::Slot783E40 | In outline: a group step for slot 0 to 6 |
| `0x759070` | Guest::Part759070 | In outline: a guest step with its part |
| `0x74fde0` | Guest::Part74FDE0 | In outline: a guest step with its part (+0xEE4) |
| `0x913e10` | Water::Grow913E10 | In outline: grows a list (Array8::SetCapacity) |
| `0x735d20` | Path::Find735D20 | In outline: finds an entry (+0x14) |
| `0x978000` | Water::Show978000 | In outline: a step when the shared flag (`0x12F018C` +0x24 +2) is set |
| `0x912a30` | Water::Light912A30 | In outline: a water step with the downward light direction (Lighting::DownwardDirection) |
| `0x912840` | Water::Light912840 | In outline: the same (another copy) |
| `0x8e2850` | Terrain::Step8E2850 | In outline: a terrain step |
| `0x7776e0` | Park::Link7776E0 | In outline: links a part |
| `0x730210` | Path::Slot730210 | In outline: a path value for a positive n |
| `0x72fe30` | Path::Slot72FE30 | In outline: a path value for a positive n |
| `0x730020` | Path::Slot730020 | In outline: a path value for a positive n |
| `0x7264a0` | Path::Count7264A0 | In outline: a path step for a positive count |
| `0x941d00` | Water::Find941D00 | In outline: a water find when it has a part (+0xB0) |
| `0x8436d0` | Scenery::Load8436D0 | In outline: a scenery load step (Scenery::LoadStep842870 with +0xC) |
| `0x908060` | Water::Cell908060 | In outline: a water cell step inside bounds |
| `0x71c040` | Path::Terrain71C040 | In outline: a path step over the terrain |
| `0x8e59c0` | Terrain::Step8E59C0 | In outline: a terrain step |
| `0x8dfc30` | Terrain::Step8DFC30 | In outline: a terrain step (`0x475220`) |
| `0x8d3a90` | Terrain::Slot8D3A90 | In outline: a step for n under the shared count (`0x1311618`) |
| `0x8d41b0` | Terrain::Activate8D41B0 | In outline: activates a terrain part |
| `0x729640` | Path::Make729640 | In outline: makes a path record (`0x48C9C0`) |
| `0xb94970` | Enclosure::ClickB94970 | In outline: a click on an enclosure cell (Enclosure::PickB94650) |
| `0xa12cf0` | Bird::StepA12CF0 | In outline: a bird step (`0x90F5A0`) |
| `0x9e3450` | Fireworks::Step9E3450 | In outline: a step when a value (+0xC) is 0 |
| `0x94f060` | Water::Case94F060 | In outline: a water step for cases under 0xE |
| `0xa634c0` | Animal::FarA634C0 | In outline: a step when a value (+0x368) is over 1000 |
| `0x84a3a0` | Scenery::Update84A3A0 | In outline: a scenery update when not flagged (+0x6C) |
| `0x901810` | Water::Terrain901810 | In outline: a water step over the terrain |
| `0x87b250` | Scenery::Cell87B250 | In outline: a cell from a point, rounded |
| `0x721fb0` | Path::InGrid721FB0 | In outline: a check that a cell is inside the path grid (+0x5C, +0x5D) |
| `0xac2570` | Fence::PassAC2570 | In outline: goes over a list (+0x54) |
| `0x99f6b0` | Folder::Path99F6B0 | In outline: builds a path from the install folder text |
| `0x96f900` | List::Sort96F900 | In outline: a list sort step |
| `0x80c9c0` | List::Sort80C9C0 | In outline: the same sort step (another copy) |
| `0x8e33b0` | Terrain::Refresh8E33B0 | In outline: a terrain refresh (`0x8E3250` with 1) |
| `0x8bbb60` | Terrain::Below8BBB60 | In outline: a step when a height is below 0 |
| `0x8771b0` | Scenery::Terrain8771B0 | In outline: a scenery step over the terrain |
| `0x77d970` | Park::Update77D970 | In outline: an update when not flagged (+0x1BC) |
| `0x72ad20` | Path::Count72AD20 | In outline: a step past a count (+0x114) |
| `0x730400` | Path::Slot730400 | In outline: a path value for a positive n |
| `0x901a00` | Water::Terrain901A00 | In outline: a water step over the terrain |
| `0x708c80` | Path::Shared708C80 | In outline: a step with the shared count (`0x13061A8`) |
| `0x727550` | Path::Case727550 | In outline: a path check for cases under 5 |
| `0x721d80` | Path::Check721D80 | In outline: a path check |
| `0x941ef0` | Water::Range941EF0 | In outline: a water step over a byte range |
| `0x80ff10` | Regions::Value80FF10 | In outline: a region value in mode 1 (+0x48) |
| `0x7a8490` | Terrain::Link7A8490 | In outline: a step with a part |
| `0x872770` | Scenery::List872770 | In outline: goes over a scenery list |
| `0x75eaa0` | Guest::Part75EAA0 | In outline: a guest step with a part |
| `0x75ec70` | Guest::Part75EC70 | In outline: a guest step with a part |
| `0xafd7f0` | Animal::ModeAFD7F0 | In outline: a step unless in mode 1 |
| `0x4a0090` | Grid::Cell4A0090 | In outline: a grid cell from a point, rounded |
| `0xabe060` | Fence::PassABE060 | In outline: goes over a list (+0x60) |
| `0x7bbe50` | Person::Part7BBE50 | In outline: a step with a part when it has one |
| `0x8b3af0` | PathManager::Create8B3AF0 | In outline: makes the shared path manager (`0x130625C`, PathManager::PathManager) |
| `0x8b71b0` | PathManager::Create8B71B0 | In outline: the same (another copy) |
| `0x9328c0` | Water::Pass9328C0 | In outline: goes over entries (+8) |
| `0x92bc70` | Water::Terrain92BC70 | In outline: a water step over the terrain |
| `0x821020` | SceneryBehaviour::Same821020 | In outline: a step when two values are the same |
| `0x81ee70` | SceneryBehaviour::Count81EE70 | In outline: a step for a positive count |
| `0x84edc0` | Scenery::Count84EDC0 | In outline: a step for a positive count |
| `0x727380` | Path::Case727380 | In outline: a path check for cases under 5 |
| `0x753b10` | Guest::Scenery753B10 | In outline: a step with a scenery value (`0x89C760` with +0x98) |
| `0x907440` | Water::Range907440 | In outline: a water step over a range |
| `0xb58a20` | Animal::RectB58A20 | In outline: a terrain step over a byte rectangle |
| `0x950c50` | Water::Grow950C50 | In outline: grows a list (+0x1BC) |
| `0x837c30` | Scenery::Count837C30 | In outline: a step with positive counts |
| `0x8175a0` | SceneryBehaviour::Ref8175A0 | In outline: a step with a reference (WithReference8155F0) |
| `0x8f6760` | Terrain::Clip8F6760 | In outline: a terrain value from a clipped rectangle (Rect::Clip8F3750) |
| `0x84c160` | Scenery::Make84C160 | In outline: makes a scenery record with an empty handle |
| `0x8514d0` | Scenery::Grow8514D0 | In outline: grows a list to hold n (`0x84EC40`) |
| `0x71da80` | Path::Terrain71DA80 | In outline: a path step over the terrain |
| `0x8feae0` | Water::Update8FEAE0 | In outline: a water step when on (+0x2C) |
| `0x816320` | SceneryBehaviour::Slot816320 | In outline: a step for a slot |
| `0x7f06f0` | Person::Loop7F06F0 | In outline: a loop |
| `0x7ea320` | Person::Cell7EA320 | In outline: a step when a cell changes |
| `0xa283f0` | Guest::StepA283F0 | In outline: a guest step (`0x754670` with 1) |
| `0x916310` | Water::Mode916310 | In outline: a water step in mode 1 (+0x38) |
| `0x827b40` | Scenery::Terrain827B40 | In outline: a scenery step over the terrain |
| `0x90d510` | Water::Grid90D510 | In outline: a step with positive counts (+0x8C, +0x98) |
| `0x84b880` | Scenery::Show84B880 | In outline: a step when its check passes or when asked |
| `0x84b430` | Scenery::Update84B430 | In outline: an update when not flagged (+0x1C) |
| `0x966920` | Water::Loop966920 | In outline: a loop |
| `0x8d0170` | Terrain::Case8D0170 | In outline: a terrain step by case |
| `0x7b1750` | Person::Part7B1750 | In outline: a step with a part when it has one |
| `0xa90b50` | Fence::CountA90B50 | In outline: a step for a positive count |
| `0x784970` | GuestGroup::Update784970 | In outline: an update when not flagged (+0x11C) |
| `0x72d090` | Path::Update72D090 | In outline: an update when not flagged (+0x11C) |
| `0x42e990` | Placement::Step42E990 | In outline: a placement step (Placement::Footprint) |
| `0x885ad0` | Scenery::Slot885AD0 | In outline: a step when it has a slot |
| `0x82ada0` | Scenery::Link82ADA0 | In outline: a step with a part |
| `0x82abf0` | Scenery::Link82ABF0 | In outline: the same (another copy) |
| `0x7f7760` | Person::Update7F7760 | In outline: an update when not flagged (+0x5C) |
| `0x8826f0` | Scenery::Part8826F0 | In outline: a step with a part (+0xCC +0x64) |
| `0x7d56d0` | Person::Refresh7D56D0 | In outline: a refresh (`0x7D3FB0` with 0) |
| `0x8e4070` | Terrain::Range8E4070 | In outline: a step for a range (+4 to +8) |
| `0x8be890` | Terrain::Rect8BE890 | In outline: goes over a byte rectangle of cells |
| `0x743a70` | Guest::Spot743A70 | In outline: a step with a spot (Guest::PickSpot76D240) |
| `0x49f5f0` | List::Pass49F5F0 | In outline: goes over a list (+0x1C) |
| `0x8e8200` | Water::Flag8E8200 | In outline: a step for an entry not flagged (+3) |
| `0xa5eff0` | Terrain::StepA5EFF0 | In outline: a terrain step |
| `0x97adf0` | Water::Pass97ADF0 | In outline: goes over entries (+0x50) |
| `0x8bfb10` | Terrain::Length8BFB10 | In outline: a length from values (+0xB8 ...) |
| `0x8bbd40` | Terrain::Below8BBD40 | In outline: a step when a height is below 0 |
| `0x87b910` | Scenery::Face87B910 | In outline: points a scenery item (Matrix::LookAlongZSide) |
| `0x7f9110` | Person::Step7F9110 | In outline: a step (`0x6F0FB0`) |
| `0x846df0` | Scenery::Place846DF0 | In outline: a place step (`0x828560`) |
| `0x7221e0` | Path::PointsStep | In outline: a step with a path's points (Path::Points) |
| `0x8f47f0` | Terrain::Range8F47F0 | In outline: a step over a byte range |
| `0xabdec0` | Fence::ValueABDEC0 | In outline: a value for a positive count |
| `0x8475e0` | Scenery::Check8475E0 | In outline: a check with a part |
| `0x729850` | Path::Make729850 | In outline: makes a path record (`0x48C9C0`) |
| `0x778e20` | Park::Value778E20 | In outline: a value from a part when it has one |
| `0xabd1c0` | Fence::Cell | In outline: a cell value when inside the bounds (+0x4C to +0x54) |
| `0x8e5bb0` | Terrain::Step8E5BB0 | In outline: a terrain step |
| `0x91bdf0` | Waterfall::Step91BDF0 | In outline: a step with the waterfall manager (WaterfallManager::Get) |
| `0x8fe330` | Water::Slot8FE330 | In outline: a step when it has a slot |
| `0x73d830` | Guest::Mode73D830 | In outline: a guest step in mode 0xD (+0x190) |
| `0x95b6b0` | Water::Count95B6B0 | In outline: a step for a positive count (+0x1C) |
| `0x8df700` | Terrain::Slope8DF700 | In outline: a value from a distance over a scale (`0x8DF080`) |
| `0x748410` | Guest::Make748410 | In outline: makes a part |
| `0x47df10` | Scenery::Shared47DF10 | In outline: a step when the shared flag (`0x12F0189`) is set or a value is under 1 |
| `0x9131c0` | Water::Count9131C0 | In outline: a step under the shared count (`0x10F0B40`) |
| `0x8dd710` | Terrain::Step8DD710 | In outline: a terrain step |
| `0x87a130` | Scenery::Count87A130 | In outline: a step under a count (+0x60) |
| `0xaaf4e0` | Fence::UpdateAAF4E0 | In outline: an update when not flagged (+0x178) |
| `0x94bca0` | Water::Slot94BCA0 | In outline: a value for n in range of its count (+0x58) |
| `0x95ab70` | Water::Part95AB70 | In outline: a step with a part when it has one |
| `0xbaf5f0` | Enclosure::StepBAF5F0 | In outline: a step with the enclosure manager (EnclosureManager::Get) |
| `0x822db0` | SceneryBehaviour::Update822DB0 | In outline: an update when not flagged (+0xF4) |
| `0x937e90` | Water::Index937E90 | In outline: makes a two-byte-entry index for a list (+0x18) |
| `0x8eb6c0` | Water::Flag8EB6C0 | In outline: a step for an entry flagged (+0x118 +1) |
| `0x8611a0` | Scenery::Shared8611A0 | In outline: a step when a shared value is set (`0x10EDB80`) and a shared flag is not (`0x1310E84`) |
| `0x978370` | Water::Same978370 | In outline: a step when two cells differ |
| `0x8db530` | Water::Level8DB530 | In outline: a water step by a level (+7) |
| `0x8a0710` | Scenery::Table8A0710 | In outline: makes a table of 0x150-byte entries |
| `0x848c90` | Scenery::Update848C90 | In outline: an update when not flagged (+0x6C) |
| `0x9671c0` | Water::Link9671C0 | In outline: a step with a part |
| `0x84ae30` | Scenery::Make84AE30 | In outline: makes a scenery record with an empty handle |
| `0x840f50` | Scenery::Pass840F50 | In outline: goes over entries (+0xD0) |
| `0x833650` | Scenery::Mode833650 | In outline: a step in mode 1 |
| `0x813e00` | List::Find813E00 | In outline: finds an entry (+8) |
| `0x717050` | Path::Link717050 | In outline: a step when given a part |
| `0x94be40` | Water::Slot94BE40 | In outline: a value for a positive n |
| `0x939ba0` | Water::Make939BA0 | In outline: makes a water record |
| `0x7ca7b0` | Person::Loop7CA7B0 | In outline: goes over a linked list |
| `0x419bb0` | Grid::Normal419BB0 | In outline: a grid point's normal (Grid::Normal419100) |
| `0xabca20` | Fence::EdgeABCA20 | In outline: a step with a tile's edge point (Tile::EdgePoint) |
| `0x7291e0` | Path::Case7291E0 | In outline: a path value by case |
| `0x8e2a40` | Terrain::Count8E2A40 | In outline: a count rounded from a value's size times a scale (+0x1A4) |
| `0x812bb0` | Regions::Make812BB0 | In outline: makes a part |
| `0x72f480` | Path::Flag72F480 | In outline: a step when not flagged (+6) |
| `0x94c160` | Water::Slot94C160 | In outline: a value for a positive n |
| `0x84b150` | Scenery::Step84B150 | In outline: a step (`0x8487C0`) |
| `0x713f10` | Path::Shared713F10 | In outline: a step with a part when the shared flag (`0x1306224`) is set |
| `0x778c90` | Park::Part778C90 | In outline: a step with a part (+0x48) |
| `0x717c80` | Path::Time717C80 | In outline: a step when a value is up to 1 |
| `0x8e4c00` | Terrain::Step8E4C00 | In outline: a terrain step |
| `0x7a8da0` | Terrain::Case7A8DA0 | In outline: a step for case 0x38 or when not flagged (+0x29C) |
