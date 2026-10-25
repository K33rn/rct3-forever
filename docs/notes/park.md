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
