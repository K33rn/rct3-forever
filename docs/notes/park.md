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
