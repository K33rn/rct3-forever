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
| `0x9a48a0` | Fireworks::Entry9A48A0 | In outline: a step unless asked and a table entry exists (Table104::Entry) |
| `0x9766c0` | Water::Light9766C0 | In outline: a water step with the downward light direction |
| `0x7d3fb0` | Person::Check7D3FB0 | In outline: a step when a check fails |
| `0x761440` | Guest::Check761440 | In outline: a check when not flagged (+0x444) |
| `0x96ee00` | Water::Loop96EE00 | In outline: a loop |
| `0x94bfd0` | Water::Slot94BFD0 | In outline: a value for n in range of its count (+0x59) |
| `0x860790` | Scenery::Pose860790 | In outline: turns a scenery item's pose (Matrix::TurnColumnsAboutY by +0xB0) |
| `0x80c530` | Regions::Loop80C530 | In outline: a loop |
| `0x789000` | GuestGroup::Part789000 | In outline: a step with a part |
| `0x778b00` | Park::Part778B00 | In outline: a step with a part (+0x48) |
| `0x8378f0` | Scenery::Make8378F0 | In outline: makes a part (0x21) |
| `0xa9fb70` | PoolComplex::CheckA9FB70 | In outline: a check with a part |
| `0x96e0f0` | Water::Grow96E0F0 | In outline: grows a list to hold n (`0x96C780`) |
| `0x7c63a0` | Person::Normal7C63A0 | In outline: a normalised direction |
| `0x728a50` | Path::Value728A50 | In outline: a value when a check passes |
| `0x840450` | Scenery::Part840450 | In outline: a step with a part (+0x9C) |
| `0x7f8d60` | Person::Value7F8D60 | In outline: a value from a pointer table (PointerTable::Value44 with +0x121) |
| `0xabe230` | Fence::PassABE230 | In outline: goes over entries (+0x60) |
| `0xabcbb0` | Fence::EdgeABCBB0 | In outline: a step with a tile's edge point (Tile::EdgePoint) |
| `0x94c2f0` | Water::Add94C2F0 | In outline: a step when adding n keeps the count (+0x58) under 0xFF |
| `0x94c480` | Water::Add94C480 | In outline: the same (another copy) |
| `0x90d380` | Water::Find90D380 | In outline: a find in a sorted table (SortedTable::Find20 on +0x80) |
| `0x74dee0` | Guest::Grow74DEE0 | In outline: grows a list (`0x744F60` with +0x18 + 1) |
| `0x744550` | Guest::Part744550 | In outline: a step with a part |
| `0xb4ac70` | Animal::TerrainB4AC70 | In outline: a step over the terrain |
| `0x830380` | Scenery::Slot830380 | In outline: a step when it has a slot (+0x190) |
| `0x419d90` | Grid::Count419D90 | In outline: a step for a positive count |
| `0x419f10` | Grid::Count419F10 | In outline: a step for a positive count |
| `0x494d40` | Terrain::Step494D40 | In outline: a terrain step |
| `0x902570` | Water::Slot902570 | In outline: a step when it has a slot |
| `0x854820` | Scenery::Kind854820 | In outline: a step unless a kind is 2 |
| `0x752cc0` | Guests::Update752CC0 | In outline: unless the global flag `0x1400306` is set, a step |
| `0x8e8d80` | Water::Loop8E8D80 | In outline: a loop |
| `0x8dcee0` | Terrain::Count8DCEE0 | In outline: a step for a positive count |
| `0x815470` | SceneryBehaviour::Shared815470 | In outline: a step when one shared count passes another (`0x130B388`, `0x130B418`) |
| `0x8917a0` | Scenery::Grid8917A0 | In outline: a step for a grid with cells |
| `0xa90fb0` | Fence::CheckA90FB0 | In outline: a step when two checks fail (`0xA906C0`, Fence::CountA90B50) |
| `0x715b40` | Path::MouseStep | In outline: places at the mouse (Path::PlaceAtMouse) when a value is below 0 |
| `0xabd040` | Fence::PassABD040 | In outline: goes over entries not flagged (+0x60, +0x18) |
| `0x8f2580` | Terrain::Case8F2580 | In outline: a terrain step by case |
| `0x7d63e0` | Person::Mode7D63E0 | In outline: in mode 8 (+0x190) once a time passes a table value (`0xFC35D8`), updates a person (Person::Update7CF350) |
| `0x8d6220` | Terrain::Flags8D6220 | In outline: a step for an entry flagged (+5, +4) |
| `0x717260` | Path::Build717260 | In outline: builds a path piece (Path::Build7143C0) |
| `0x94c610` | Water::Add94C610 | In outline: a step when adding n keeps a count (+0x59) under 0xFF |
| `0x926600` | Water::Make926600 | In outline: makes parts (`0xBDADC0`, Constructor on +0x84) |
| `0xafdaf0` | Animal::RangeAFDAF0 | In outline: a step inside a range (+0x30) |
| `0x9036d0` | Whirlpool::FindAndSet | In outline: finds a whirlpool's cell (Whirlpool::FindCell) and sets it (Whirlpool::Cell) |
| `0x89c5f0` | Scenery::Shared89C5F0 | In outline: when the global flag `0x1400306` is set, goes over entries |
| `0x817420` | SceneryBehaviour::Slot817420 | In outline: a step for a 32-byte slot not flagged (+0x18) |
| `0x75ae90` | Guest::Stage75AE90 | In outline: a step with no stage (+0x194 = -1) |
| `0x84afe0` | Scenery::Step84AFE0 | In outline: a step (`0x532A80`) unless flagged (+0x1C) |
| `0x994ce0` | Fireworks::Terrain994CE0 | In outline: a step over the terrain |
| `0xb766d0` | Animal::TerrainB766D0 | In outline: a step over the terrain |
| `0x7a06d0` | Terrain::Loop7A06D0 | In outline: goes over a linked list |
| `0x8b2210` | Scenery::Slot8B2210 | In outline: a step for an entry not flagged (+0x7C, +0x18) |
| `0x848b20` | Scenery::Step848B20 | In outline: a step (`0x82FAB0`) when not flagged (+0x6C) |
| `0x8489b0` | Scenery::Step8489B0 | In outline: a step (`0x82FA30`) when not flagged (+0x6C) |
| `0x848840` | Scenery::Step848840 | In outline: a step (`0x82F9C0`) when not flagged (+0x6C) |
| `0x936d70` | Water::Loop936D70 | In outline: a loop over parts |
| `0x72b1c0` | Path::Inside72B1C0 | In outline: a step inside the path grid (PathManager::Get, Cell::StepInside) |
| `0x89c480` | Scenery::Shared89C480 | In outline: when the global flag `0x1400306` is set, goes over entries |
| `0xabd790` | Fence::RectABD790 | In outline: goes over a rectangle of cells |
| `0x9473a0` | Water::Table9473A0 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0x823260` | SceneryBehaviour::Slot823260 | In outline: a step (`0x94EEB0`) when it has a slot |
| `0x76ef40` | Guest::Check76EF40 | In outline: a check unless the park is flagged (+0x5C bit 0x40), for an entry in mode 2 (+0x70) |
| `0x929420` | Water::Pair929420 | In outline: a step when given both parts, unless told not to |
| `0x838530` | Scenery::SetCount838530 | In outline: sets a count (+0x34) when it changes |
| `0x834710` | Scenery::Text834710 | In outline: a check on a scenery item's text (Scenery::Link834260) |
| `0x8345a0` | Scenery::Text8345A0 | In outline: the same (another copy) |
| `0x7ae6f0` | Terrain::Range7AE6F0 | In outline: a check that cells lie inside a byte range |
| `0x7861d0` | GuestGroup::Slot7861D0 | In outline: a step for slot 0 to 6 (`0x1342768`) |
| `0xb76560` | Animal::TerrainB76560 | In outline: a step over the terrain |
| `0x934360` | Water::Grow934360 | In outline: grows a list by 16 when full (ObjectArray::SetCapacity) |
| `0x7b2560` | Person::Group7B2560 | In outline: a step when a person's group (Guest::Group) differs |
| `0x94c790` | Water::Add94C790 | In outline: a step when adding n keeps a count (+0x59) under 0xFF |
| `0x87a370` | Scenery::Normal87A370 | In outline: a normalised direction, with length 0 handled |
| `0x830210` | Scenery::Check830210 | In outline: a check when not flagged (+0x18C) |
| `0x7c3ff0` | Person::Water7C3FF0 | In outline: a step with the water manager when it has a slot (+0x22C) |
| `0x7b0460` | Person::Kind7B0460 | In outline: a step when not flagged (+0x3C) for kinds other than 6 and 0x1E |
| `0x728d70` | Path::Case728D70 | In outline: a path value by case |
| `0x9ffcd0` | Fireworks::Terrain9FFCD0 | In outline: a step over the terrain for entries (+0x10) |
| `0x8fafa0` | Water::Camera8FAFA0 | In outline: a step with the camera (MainCamera::Get, `0xA62D90`) |
| `0x925540` | Water::Same925540 | In outline: a step when given its own value (+0x34) |
| `0x8c0520` | Terrain::Below8C0520 | In outline: a step against the terrain's lowest point (Terrain::LowestPoint) |
| `0x930ac0` | Water::Table930AC0 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0x91a330` | Water::Copy91A330 | In outline: copies a list (Array::Resize417A20) |
| `0x8da760` | Water::Pass8DA760 | In outline: a step with the water manager over entries |
| `0x7c1230` | Person::Find7C1230 | In outline: finds an entry (+0x20) |
| `0x938b70` | Water::Find938B70 | In outline: finds an entry matching four bytes |
| `0x919da0` | Water::Terrain919DA0 | In outline: a step over the terrain |
| `0x4473c0` | Terrain::Step4473C0 | In outline: a terrain step |
| `0x994ac0` | Fireworks::Slot994AC0 | In outline: a step when it has a slot |
| `0x87c510` | Scenery::Value87C510 | In outline: a step by how far a value (+0x48) is from 0.5 |
| `0x82ef60` | Scenery::Cell82EF60 | In outline: a step when the cell changes (+0xA0, +0xA4) (Scenery::Cell82B350) |
| `0x779b60` | Park::Pass779B60 | In outline: goes over entries (+0x48) |
| `0x8ea430` | Water::Flag8EA430 | In outline: a step for an entry (0x130 bytes) not flagged (+1) |
| `0x7c08e0` | Person::Shared7C08E0 | In outline: unless the global flag `0x1400306` is set, a step against a value (+0x2C) |
| `0x95aa10` | Water::Clip95AA10 | In outline: a step with a clipped rectangle (Rect::Clip8F3750) |
| `0x8fee20` | Water::Slot8FEE20 | In outline: a step when it has a slot, unless told not to |
| `0x8781c0` | Scenery::Step8781C0 | In outline: a step (`0x827EF0`) |
| `0x86ebb0` | Scenery::Loop86EBB0 | In outline: goes over a linked list |
| `0x7ff910` | Person::Pass7FF910 | In outline: goes over entries (+0x3C) |
| `0x7366d0` | Path::Part7366D0 | In outline: a step with a part |
| `0xb9f740` | PathGrid::CentreB9F740 | In outline: a step with a scenery footprint's centre (SceneryItem::FootprintCentre) |
| `0x958ec0` | Water::Table958EC0 | In outline: fills a 4 by 4 table with 0xFFFF (`0x416110`) |
| `0x8ddaf0` | Terrain::Step8DDAF0 | In outline: a terrain step |
| `0x86df00` | Scenery::Make86DF00 | In outline: makes a record (`0x48C9C0`, SetTableF2CEB4) |
| `0x826920` | Scenery::Loop826920 | In outline: a loop with a part |
| `0x96acc0` | Water::Find96ACC0 | In outline: finds a water entry for a cell (+8, +0xC) |
| `0x901da0` | Water::Table901DA0 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0x7df090` | Person::Clear7DF090 | In outline: clears a list (ObjectArray28::Resize(0)) when a value is negative |
| `0x71ad30` | Path::Count71AD30 | In outline: a step under n when not flagged (+0x428) |
| `0xabd620` | Fence::RectABD620 | In outline: goes over a rectangle of cells |
| `0x855a20` | Scenery::Case855A20 | In outline: a step by case |
| `0x80e4f0` | Regions::Range80E4F0 | In outline: a step within a range |
| `0x4cc919` | Shop::Terrain4CC919 | In outline: a step over the terrain |
| `0x9e32c0` | Fireworks::Time9E32C0 | In outline: a step with a positive time when a value (+0x58) is not 0 |
| `0x930c20` | Water::Table930C20 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0x7f5330` | Person::Link7F5330 | In outline: links parts |
| `0x783450` | Park::Pass783450 | In outline: goes over entries (+0x48) with the park (Park::Get) |
| `0x720e10` | Path::Activate720E10 | In outline: activates a path piece and acts when it is flagged (+6) |
| `0x9769e0` | Water::Show9769E0 | In outline: when the shared flag (`0x12F018C` +0x24 +2) is set, a terrain step |
| `0x8ddea0` | Terrain::Step8DDEA0 | In outline: a terrain step |
| `0x88ad50` | Scenery::Part88AD50 | In outline: a step with a part (+0x170) |
| `0xc9d0f0` | Table::FillC9D0F0 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0x92bfd0` | Water::Table92BFD0 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0x7279a0` | Path::Terrain7279A0 | In outline: a step over the terrain |
| `0x7f83c0` | Person::Link7F83C0 | In outline: a step with a part |
| `0x733d90` | PathManager::Create733D90 | In outline: makes the shared path manager (`0x130625C`, PathManager::PathManager) and activates it |
| `0x4550c0` | List::Pass4550C0 | In outline: goes over entries not flagged (+0x82, +0x80) |
| `0xc1c990` | Table::FillC1C990 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0xb9b090` | PathGrid::InsideB9B090 | In outline: a step for a cell inside the grid (+0x64, +0x68) |
| `0xac2830` | Fence::TerrainAC2830 | In outline: a step over the terrain for a negative value |
| `0xaa1c40` | PoolComplex::SlotAA1C40 | In outline: a step for n under a count (+0x34) |
| `0x891df0` | Scenery::Pass891DF0 | In outline: goes over entries (+0x14) |
| `0x937420` | Water::Loop937420 | In outline: a loop |
| `0x901f00` | Water::Table901F00 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0x87c6b0` | Scenery::Table87C6B0 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0x8468f0` | Scenery::Count8468F0 | In outline: a step for a positive count with a part (+0x9C) |
| `0xc1cd50` | Table::FillC1CD50 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0x969920` | Water::Find969920 | In outline: finds a water entry for a cell (+8, +0xC) |
| `0x94e090` | Water::Count94E090 | In outline: a step against a count (+8) |
| `0x88e0b0` | Scenery::Flag88E0B0 | In outline: goes over entries (+0x3C8) flagged (bit 4) |
| `0x861050` | Scenery::Pose861050 | In outline: unless told not to, a scenery pose (Scenery::Pose860790) |
| `0x77bf40` | GuestGroup::Check77BF40 | In outline: checks a group (GuestGroup::Check778580(1), `0x778880` with 0xC) |
| `0xa7a630` | Animal::TerrainA7A630 | In outline: a step over the terrain |
| `0x963fa0` | Water::Manager963FA0 | In outline: when asked, a step with the water manager |
| `0x7a0030` | Terrain::Step7A0030 | In outline: a step (`0x7A0860` with 0) |
| `0x7e6c20` | Person::Points7E6C20 | In outline: a step with more than two points |
| `0x8211f0` | SceneryBehaviour::Check8211F0 | In outline: a step when a check passes (`0x81FAF0`) |
| `0x79d3a0` | Terrain::Step79D3A0 | In outline: a terrain step |
| `0x7516e0` | Guest::Find7516E0 | In outline: unless told not to, finds an entry matching a value (+0x14) |
| `0xb91550` | Enclosure::FindB91550 | In outline: a step with the enclosure manager (`0xB9F8A0`) |
| `0x8ff000` | Water::Slot8FF000 | In outline: a step for a slot under a count |
| `0x7141a0` | Path::Link7141A0 | In outline: a step when given a part |
| `0x92c130` | Water::Table92C130 | In outline: fills a 0x1C by 0x1C table with 0xFFFF (`0x416110`) |
| `0x8f86f0` | Water::Shared8F86F0 | In outline: a step when the shared flag (`0x10F0B20`) is set |
| `0x726ea0` | Path::Flag726EA0 | In outline: a step when the path manager is flagged (+0x97) |
| `0x95f910` | Water::Range95F910 | In outline: a step over a range |
| `0x746390` | Guest::Update746390 | In outline: a step when flagged (+0xE49) |
| `0x9137a0` | Water::Count9137A0 | In outline: a step when a count (+0x78) is above 1 |
| `0x71e570` | PathQueue::Make | In outline: activates a part and makes a path queue (PathQueue) |
| `0xbcb910` | Range::StepBCB910 | In outline: a step comparing values |
| `0xb9f4e0` | PathGrid::PassB9F4E0 | In outline: goes over entries (+0x78) that are set |
| `0x9384a0` | Water::Fill9384A0 | In outline: when it has two parts (+0xC, +0x18), a step for the device (ForCurrent(0)) |
| `0x92c5c0` | Water::Step92C5C0 | In outline: a step (`0x90F880` with +0x58) |
| `0x9143c0` | Water::Table9143C0 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0x71be20` | Path::Points71BE20 | In outline: a step with a path's points (Path::Points) |
| `0x912150` | Water::Check912150 | In outline: a step when a check passes and a flag is set (+0x10) |
| `0x903aa0` | Water::Edge903AA0 | In outline: a step when not flagged (+0xC +0x54) |
| `0x8e21b0` | Terrain::Step8E21B0 | In outline: a terrain step |
| `0xaaa110` | PoolComplex::LoopAAA110 | In outline: a loop up to n |
| `0x8ddc50` | Terrain::Step8DDC50 | In outline: a terrain step |
| `0x8541a0` | Scenery::Slot8541A0 | In outline: a step (`0x851450` with +0x118, +0x114) when it has a slot |
| `0xb9d990` | PathGrid::CellB9D990 | In outline: a value for a cell inside the grid (+0x64, +0x68) that is set (+0x74) |
| `0x8d67a0` | Terrain::Loop8D67A0 | In outline: a loop over cells |
| `0x7a6790` | Terrain::Kind7A6790 | In outline: for kind 0xF with no part (+0x194), a step (`0x4751E0`) |
| `0x71d7b0` | Path::Count71D7B0 | In outline: a step for counts above 1 |
| `0x90a150` | Water::Count90A150 | In outline: a step for one entry |
| `0x8b23b0` | Scenery::Flat8B23B0 | In outline: a flat length step (`0x8B15A0`) |
| `0x8383b0` | Scenery::Pass8383B0 | In outline: goes over entries (+0x14) |
| `0x7c58a0` | Person::Face7C58A0 | In outline: points a person (Matrix::LookAlongZSide) and acts on a non-zero value |
| `0x6f7700` | Park::Date6F7700 | In outline: a value from the date (GameTime::Get), rounded |
| `0x8dd8b0` | Terrain::Step8DD8B0 | In outline: a terrain step |
| `0x84bb60` | Scenery::Step84BB60 | In outline: a step (`0x532A80` with +0x44) over seven values |
| `0x829060` | Scenery::Flags829060 | In outline: a step with a part (+0x9C) and matching flags |
| `0x75ee40` | Guest::Check75EE40 | In outline: when on (+0x404), compares a value against a limit |
| `0x756170` | Guest::Spot756170 | In outline: a step with a spot (`0x7578C0` with +0x121) |
| `0x7523e0` | Guest::Matrix7523E0 | In outline: works out matrices (`0xCD78A0`, `0xCD75C0`) |
| `0x97ce60` | Water::Loop97CE60 | In outline: a loop |
| `0x943670` | Water::Check943670 | In outline: a step when two values differ (+0x68, +0x74) or a check passes |
| `0x7e9be0` | Person::Case7E9BE0 | In outline: a step by a value |
| `0xb6c3a0` | Animal::TerrainB6C3A0 | In outline: a step over the terrain |
| `0xa914b0` | Fence::DistanceA914B0 | In outline: a distance (`0xA90D00`) over entries (+0x14) |
| `0x9fbd90` | Fireworks::Fill9FBD90 | In outline: fills buffers for the current device |
| `0x9732b0` | Water::Clear9732B0 | In outline: tears down entries (+4) |
| `0x9506f0` | Water::Table9506F0 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0x92b820` | Water::List92B820 | In outline: goes over entries not flagged (+0x18) |
| `0x8e88e0` | Terrain::Step8E88E0 | In outline: a terrain step |
| `0x8d3600` | Terrain::Ids8D3600 | In outline: makes a part and the shared id set (`0x12F4634`, IdSet) |
| `0x7853a0` | GuestGroup::Slot7853A0 | In outline: a step for slot 0 to 6 with a part |
| `0xa9f6a0` | PoolComplex::WaterA9F6A0 | In outline: a step with the water manager (`0x905140`) |
| `0x901c20` | Water::Solve901C20 | In outline: a water find (Water::Find8FF790) and a grid solve (Grid::Solve90A290) |
| `0x838cc0` | Scenery::Count838CC0 | In outline: for a positive count, a step (Call44CC40And489A10) |
| `0x771f90` | Park::Pass771F90 | In outline: goes over entries (+0x14) not flagged (+0x18) |
| `0x7541a0` | Guests::Init7541A0 | In outline: makes a part and sets a shared value (`0x13064FC`, `0x7537B0`) |
| `0x783c60` | GuestGroup::Update783C60 | In outline: an update when not flagged (+0x11C) |
| `0x776c60` | Park::Tile776C60 | In outline: unless a kind is 2 (+0x8B), a step with a path tile's centre (PathTile::Centre) |
| `0x95de00` | Water::Compare95DE00 | In outline: a step comparing heights |
| `0x912010` | Water::Table912010 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0x836360` | Scenery::Make836360 | In outline: makes a part when not flagged (+0x194) |
| `0x9e63c0` | Fireworks::Table9E63C0 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0x482260` | Draw::Count482260 | In outline: a step for more than one entry |
| `0x8b6fb0` | Scenery::Update8B6FB0 | In outline: an update when not flagged (+0x120) |
| `0x856bc0` | Scenery::Count856BC0 | In outline: a step (`0xCEFF50`) for a positive count |
| `0x7789c0` | GuestGroup::Check7789C0 | In outline: makes a part and checks it (`0x76FF60`) |
| `0x778880` | GuestGroup::Check778880 | In outline: makes a part and checks it (`0x76FFA0`) |
| `0x8b7780` | Scenery::Text8B7780 | In outline: a text when not flagged (+0x4C) |
| `0x7e6a40` | Person::Terrain7E6A40 | In outline: a step over the terrain |
| `0x747240` | Guest::Make747240 | In outline: makes a part and goes over entries |
| `0x733320` | Path::Flag733320 | In outline: a step by a flag's sign and when not flagged (+8) |
| `0x9d5f90` | Fireworks::Time9D5F90 | In outline: a step with a positive value (+0xC) over a linked list |
| `0x998b90` | Fireworks::Changed998B90 | In outline: a step (`0x989180`) when two values differ (+0xF8, +0xE8) |
| `0x9061c0` | Water::Value9061C0 | In outline: a value for counts above 3 |
| `0x7f93e0` | Person::Near7F93E0 | In outline: for kinds 4, 0 or 0x16, a step when a point is within a distance |
| `0x726d60` | Path::Height726D60 | In outline: a value from the terrain |
| `0x71e430` | PathFlying::Make | In outline: activates a part and makes a flying path (PathFlying) |
| `0x9e6500` | Fireworks::Table9E6500 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0x959020` | Water::Table959020 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0x923c60` | Water::Step923C60 | In outline: a step (`0x923A90` with +0x130) with no part (+0x124) |
| `0x875aa0` | Scenery::Count875AA0 | In outline: a step for a positive count with a part |
| `0x862480` | Scenery::Case862480 | In outline: a step by an entry's kind when there are entries |
| `0x83b340` | Scenery::Clear83B340 | In outline: tears down entries (+0x2C) |
| `0x779a20` | Park::Pass779A20 | In outline: goes over entries (+0x48) in mode 4 or others |
| `0x769cb0` | PathManager::Create769CB0 | In outline: makes the shared path manager (`0x130625C`, PathManager::PathManager) and activates it |
| `0x760650` | Guest::Check760650 | In outline: a check when not flagged (+0x334) |
| `0x71f340` | Path::Table71F340 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0x703950` | Path::Terrain703950 | In outline: a step over the terrain |
| `0x96e6b0` | Water::Slot96E6B0 | In outline: a step for a slot |
| `0x9e6290` | Fireworks::Table9E6290 | In outline: fills a table with 0xFFFF (`0x416110`) |
| `0x454f10` | List::Pass454F10 | In outline: goes over entries not flagged (+0x82) and flagged (+0x81) |
| `0x8d6e90` | Terrain::Pick8D6E90 | In outline: a ray from a screen point (MainCamera::Get, Camera::ScreenRay) for picking |
| `0x8b4530` | Scenery::Terrain8B4530 | In outline: a step over the terrain |
| `0x847210` | Scenery::Centre847210 | In outline: for rides flagged 0x400000, a step at a scenery footprint's centre |
| `0x7e6750` | Person::Loop7E6750 | In outline: a loop up to n |
| `0x724840` | Path::Inside724840 | In outline: a step for a cell inside a byte range (+0x1C, +0x1D) |
| `0x96ff10` | Water::Grow96FF10 | In outline: grows a list (`0x96F7F0`) to hold one more |
| `0x8e3f40` | Terrain::Step8E3F40 | In outline: a terrain step |
| `0x7a1840` | Terrain::Link7A1840 | In outline: a step with a part |
| `0x771e50` | Park::Pass771E50 | In outline: goes over entries (+0x14) not flagged (+0x18) |
| `0x71b370` | Path::Terrain71B370 | In outline: a value from the terrain |
| `0x7a8f90` | Terrain::Same7A8F90 | In outline: a step when a value matches |
| `0x7542e0` | Guests::Find7542E0 | In outline: a find comparing pairs (`0xCD5640`) |
| `0x83cd30` | Scenery::Make83CD30 | In outline: makes a part when it has one (+0x16C) and is on (+0x184) |
| `0x8b6e80` | Scenery::Update8B6E80 | In outline: an update when not flagged (+0xD8) |
| `0x7a0180` | Terrain::Link7A0180 | In outline: a step with a part |
| `0x778fc0` | Park::Pass778FC0 | In outline: a step (`0x7745B0`) and goes over entries (+0x48) |
| `0x916110` | Water::Camera916110 | In outline: a step with the camera over the terrain |
| `0x8f2780` | Terrain::Fill8F2780 | In outline: with a part, a step for the device (ForCurrent(1)) |
| `0x8e6150` | Water::Normal8E6150 | In outline: a normalise step when a value is big enough |
| `0x87c330` | Scenery::Count87C330 | In outline: a step with a part's positive count (+8 +0x24) |
| `0x80b6b0` | Scenery::Place80B6B0 | In outline: places a scenery item (Scenery::Place80B260) |
| `0x799910` | Guest::Make799910 | In outline: makes a 32-byte part with a length |
| `0x564270` | Ride::Pass564270 | In outline: goes over entries not flagged (+0x18) whose part is not in mode 4 (+0x388) |
| `0x836bd0` | Scenery::Update836BD0 | In outline: an update when not flagged (+0x194) |
| `0x788270` | GuestGroup::Find788270 | In outline: finds an entry by name (WideString::Compare) among a count (+0x194) |
| `0x71bb40` | Path::Tile71BB40 | In outline: for 2, a step with a path tile's centre (PathTile::Centre) |
| `0x713de0` | Path::Terrain713DE0 | In outline: a step over the terrain |
| `0x4839a0` | Grid::Same4839A0 | In outline: a step when a cell matches (+4, +8) |
| `0x98fd00` | List::Pass98FD00 | In outline: goes over entries (+0x44) other than 0x3424E9D9 |
| `0x80df00` | Regions::Texture80DF00 | In outline: creates a 64-wide texture (Device::CreateTexture(0x40, 0, 0, 1)) and fills cells inside it |
| `0x91a4a0` | Water::Copy91A4A0 | In outline: copies a list (Array::Resize417A20) |
| `0x7453a0` | Guest::Leader7453A0 | In outline: a step with a group's leader (AvatarGroup::Leader) |
| `0x704cf0` | Park::Awards704CF0 | In outline: a loop with the park awards (Awards::Get) |
| `0x862f00` | Scenery::Slot862F00 | In outline: a step (`0x498360`) when it has a slot (+0x144) |
| `0xb982d0` | PathGrid::MakeB982D0 | In outline: sets up a 100-entry path table (`0x488290`) over the terrain |
| `0x7aad50` | Terrain::Check7AAD50 | In outline: a check unless the park is flagged (+0x5C bit 0x40), for an entry in mode 2 (+0x70) |
| `0x7021e0` | Path::Terrain7021E0 | In outline: a step with the terrain for entries (+0x124) |
| `0x8a0530` | SceneryRenderer::Step8A0530 | In outline: a step (`0x89CAF0`) and a pass (SceneryRenderer::Pass89F8F0) |
| `0x716160` | Path::Terrain716160 | In outline: a step over the terrain |
| `0x44b170` | Terrain::Step44B170 | In outline: a terrain step |
| `0x958c90` | Water::Count958C90 | In outline: a step for counts above 10 |
| `0x81d370` | SceneryBehaviour::Shared81D370 | In outline: a step when two shared values differ (`0x130B4A0`, `0x130B4AC`) |
| `0x79a150` | Guest::Make79A150 | In outline: a step (`0x7CADD0`) and makes a part (`0x799910`) |
| `0x7798f0` | Park::Pass7798F0 | In outline: goes over entries (+0x48) in mode 4 or others |
| `0x725f30` | Path::Link725F30 | In outline: a step when given a part |
| `0xafaed0` | Animal::TerrainAFAED0 | In outline: a step over the terrain |
| `0x784840` | GuestGroup::Check784840 | In outline: a check (`0x7847B0`) when not flagged (+0x11C) |
| `0x77dd90` | Park::Link77DD90 | In outline: a step with a part |
| `0x7156e0` | Path::Link7156E0 | In outline: links a path part (Path::Link7141A0) |
| `0x983b00` | Fireworks::Time983B00 | In outline: a step with a positive value (+0xC4) while a time (+0xBC) is under a limit |
| `0x83b1c0` | Scenery::Range83B1C0 | In outline: a step when two values are the same (+0x24, +0x28) |
| `0x499b10` | List::Pass499B10 | In outline: goes over entries (+0x24) that are set |
| `0x9ec2e0` | Fireworks::Loop9EC2E0 | In outline: goes over a list |
| `0x93d340` | Water::Table93D340 | In outline: sets up a 0x100-entry path table (`0x488290`) over the terrain |
| `0x8bce50` | Terrain::Cell8BCE50 | In outline: a step for a cell inside a count (+4) |
| `0x8057d0` | Regions::Loop8057D0 | In outline: a loop when two values differ |
| `0x491e30` | Grid::Table491E30 | In outline: sets up a 0x80-entry path table (`0x488290`) over the terrain |
| `0x97e200` | Water::Pass97E200 | In outline: goes over entries (+0x11C) |
| `0x77dc70` | Park::Value77DC70 | In outline: a value from entries not flagged (+0x334) |
| `0x77db50` | Park::Value77DB50 | In outline: the same (another copy) |
| `0x4632e0` | Scenery::Clear4632E0 | In outline: tears down entries (+0x18) |
| `0x8c6f10` | Terrain::Corner8C6F10 | In outline: a step with a terrain corner's position (Terrain::CornerPosition) |
| `0x8849b0` | Scenery::Find8849B0 | In outline: finds an entry by a value among entries (+0xF4) |
| `0x7ad760` | Terrain::Value7AD760 | In outline: a step by a value (+0x52C) |
| `0xa0bc60` | Bird::WaterA0BC60 | In outline: a step with the water manager (`0x9459F0`) |
| `0x8291a0` | Scenery::Flags8291A0 | In outline: a step for entries whose bit is set (+0x134) |
| `0x7e8d20` | Person::Terrain7E8D20 | In outline: a step over the terrain |
| `0x919c80` | Waterfall::Flag919C80 | In outline: a step when the waterfall manager is flagged (+0x68 bit 0x10) |
| `0x7cf7e0` | Person::Link7CF7E0 | In outline: a step with a part |
| `0x435ea0` | Grid::Round435EA0 | In outline: a value rounded |
| `0x482a40` | Effect::Release482A40 | In outline: unless a shared value is set (`0x10EDBA0`), releases its part (+0x1C) and clears it |
| `0x4fe790` | Path::Points4FE790 | In outline: when a check passes and the path manager's cell is set, goes over a path's points (Path::Points) |
| `0x828b90` | Scenery::Key828B90 | In outline: unless key 0x2A is held (KeyState::Held), a terrain step |
| `0x8233d0` | SceneryBehaviour::Cell8233D0 | In outline: when the terrain is not flagged, finds its owner's cell (Grid::CellAt) |
| `0xac1fe0` | Fence::SetAC1FE0 | In outline: sets a border's cells (`0xABFDA0`) with -1 for missing values |
| `0xac0a50` | Fence::RectAC0A50 | In outline: goes over a rectangle's cells inside its bounds (+0x4C to +0x58) |
| `0xa9f170` | PoolComplex::TerrainA9F170 | In outline: a step at a point when the terrain is not flagged |
| `0x824220` | SceneryBehaviour::Pass824220 | In outline: goes over entries four at a time, checking a value (+0x2E0) against a limit |
| `0x4659b0` | Scenery::Save4659B0 | In outline: makes a database save (MADatabaseSave) for an object |
| `0x8ea140` | Water::Count8EA140 | In outline: with no part (+0x130), a step for counts above 1 |
| `0x8c6bc0` | Terrain::Corner8C6BC0 | In outline: when asked and not flagged (+0x13), a step by a scaled value |
| `0x846bb0` | Scenery::Ride846BB0 | In outline: for kind 0x12, a step with a ride flagged 0x400000 with more than one station |
| `0x93ca60` | Water::Draw93CA60 | In outline: a draw step with the shared device (`0x13D0DB8`, slot 0xB8) |
| `0x7797d0` | Park::Pass7797D0 | In outline: goes over entries (+0x48, list at +0x60) with a part each |
| `0x8c6ce0` | Terrain::Corner8C6CE0 | In outline: for a cell inside a count (+4), when not flagged (+0x13 bit 2), a step |
| `0x825a00` | Scenery::Mouse825A00 | In outline: unless a shared value is set (`0x13429A8`), a screen-point step with the camera and the GUI root's size |
| `0x709b10` | Path::Make709B10 | In outline: makes a pooled object (PooledObject) with its table (`0xF68FFC`) |
| `0x910100` | Water::Init910100 | In outline: sets up a 0x80-slot pool (SlotPool::Init) and clears values, keeping its argument (+0x30) |
| `0x81d4a0` | SceneryBehaviour::Find81D4A0 | In outline: goes over the shared behaviour list (`0x130B49C`) for one whose owner matches |
| `0x81d5c0` | SceneryBehaviour::Find81D5C0 | In outline: the same (another copy) |
| `0x92e3f0` | Water::Nearest92E3F0 | In outline: finds the nearest entry (starting from 1e6) |
| `0x89f0f0` | Scenery::Ground89F0F0 | In outline: grounds each of its 0x150-byte entries (+0x3C4, Scenery::Ground89EDD0) |
| `0x86e9a0` | Window::Open86E9A0 | In outline: makes a 0x1A4-byte window part titled `0x1310EB8` |
| `0x907bb0` | Water::Cell907BB0 | In outline: a cell from a value's place within a range (+8 to +0x10), rounded down |
| `0x8e6c10` | Water::Scale8E6C10 | Fills a matrix from its values (+0xF4 on) scaled by two values |
| `0x82fc30` | Scenery::Height82FC30 | In outline: when not flagged (+0x130) and its part is not (bit 5), places its footprint (Placement::Footprint) and reads a height (Terrain::HeightLevel) |
| `0x7ecc80` | Person::Path7ECC80 | In outline: a step with the path manager at its cell (+0x80, +0x84) |
| `0x7bde80` | Person::Texts7BDE80 | In outline: makes a text record for each entry (+0x30) |
| `0xb9f630` | PathGrid::PassB9F630 | In outline: when the terrain is not flagged, goes over entries (+0x78) |
| `0x4913e0` | Grid::Table4913E0 | In outline: sets up a 0x100-entry path table (`0x488290`), its table (`0xF2E9A4`) and clears a flag (+0x428) |
| `0x5f3df0` | Ride::Part5F3DF0 | In outline: a step with its part's slot from the shared id (`0x12F4B34`) |
| `0x993920` | Fireworks::Update993920 | In outline: an update when not flagged (+0x88) |
| `0x802e30` | Regions::Find802E30 | In outline: a find in a sorted array (SortedArray::Find on +0x24) |
| `0x773040` | Park::Tile773040 | In outline: with a part (+0x104), a step at a path tile's centre (PathTile::Centre) by a fraction |
| `0x8d3830` | Terrain::Layers8D3830 | In outline: with the shared value (`0x13CE668`), builds terrain layers (TerrainLayers::BuildAFA7C0) and more |
| `0x8470f0` | Scenery::Fill8470F0 | In outline: fills a record from its values |
| `0x8c6e00` | Terrain::Corner8C6E00 | In outline: when asked and not flagged (+0x13 bit 4), a step by a scaled value |
| `0x774060` | Park::Count774060 | In outline: counts entries (+0x48, list at +0x60) |
| `0x85a9c0` | Scenery::Pop85A9C0 | In outline: takes the last entry off two lists (+8, +0x10) |
| `0x843b20` | Scenery::Place843B20 | In outline: a place step with a part that is on (+0x14) |
| `0x90c060` | Water::Camera90C060 | In outline: with entries (+0x28), a step with the camera |
| `0x80b7e0` | Regions::Find80B7E0 | In outline: a find over a list (+4) |
| `0x7d3ea0` | Person::Waterfall7D3EA0 | In outline: a step with the waterfall manager at its point (+0x80, +0x84) |
| `0x852f90` | Scenery::Bit852F90 | In outline: a step with a bit from a count (+0xC) |
| `0x827f40` | Scenery::Terrain827F40 | In outline: a step when the terrain is not flagged |
| `0x94ad30` | Water::Terrain94AD30 | In outline: with terrain entries (+0x7C), a step (`0x8F3720`) |
| `0x84aa80` | Scenery::Update84AA80 | In outline: an update when not flagged (+0x6C) |
| `0x7f5490` | Person::Link7F5490 | In outline: a step with a linked part (+0x544) |
| `0x7dc680` | Person::Pass7DC680 | In outline: goes over n people with an object (`0x13CE94C`) |
| `0x414320` | Effect::Pass414320 | In outline: goes over entries (+0x20) by distance from a point |
| `0x8b9eb0` | Scenery::Make8B9EB0 | In outline: makes a list member part (ListMember with `0x10EC9F8`) |
| `0x826c60` | Scenery::Check826C60 | In outline: a check with its part |
| `0x74e3a0` | Guest::Load74E3A0 | In outline: a guest part (Guest::Part744550), a loader find (Loader::Find744A90) and more |
| `0x6bc580` | Attractions::Pass6BC580 | In outline: goes over the attractions from the first (AttractionInstance::FirstOf(0)) |
| `0x49a4b0` | Effects::Add49A4B0 | In outline: for 0x10, makes a 0x1C-byte record for an id (IdMap::Remove7) when there is none |
| `0x444780` | Effects::Time444780 | In outline: a value from the shared object's time (`0x1106C54` +0x54), rounded |
| `0x8f34a0` | Terrain::Centre8F34A0 | In outline: a terrain patch's centre from its size and place |
| `0x862df0` | Scenery::Part862DF0 | In outline: a value for part n (0x48 bytes) under a count (+0x108) |
| `0xb9be60` | PathGrid::EntryB9BE60 | In outline: a step for entry n (0xB8 bytes) not flagged |
| `0xaa1d90` | PoolComplex::PassAA1D90 | In outline: goes over entries (+0x30) |
| `0x8499a0` | Scenery::Set8499A0 | In outline: sets a value (+0x94) when it changes and not flagged |
| `0x82e740` | Scenery::Load82E740 | In outline: reads four-byte values from a saved block (Load::ReadBytes) |
| `0x82af50` | Scenery::Link82AF50 | In outline: a step with a linked part (+0x150) |
| `0x77be30` | Guest::Kind77BE30 | In outline: for kind 0xB (+0x7C), a check on a person not flagged (+0x334) |
| `0x938d80` | Water::Add938D80 | In outline: grows a list by 0x100 when full (`0x937890`) and adds an entry from two points |
| `0x867b70` | Scenery::Value867B70 | In outline: for entry n of kind 3, a table value (Table2C::Value) |
| `0x863030` | Scenery::Reset863030 | In outline: with a slot (+0x28), a step (SceneryRenderer::Step8A0530) and clears it |
| `0x852d60` | Scenery::Bit852D60 | In outline: a step with a bit from a count (+0xC) |
| `0x445c30` | Effects::Slot445C30 | In outline: a step with a slot (SortedTable::Add445AF0 with 3, 3) of 0x1FB4 bytes |
| `0xb88d10` | Animal::FindB88D10 | In outline: finds an entry among entries (+0x48, +0x4C) |
| `0x9fa6b0` | Fireworks::Count9FA6B0 | In outline: a count from entry n (0x24 bytes) |
| `0x98dda0` | Window::Update98DDA0 | In outline: clears a value (+0x110) and an update when not flagged (+0xB0) |
| `0x8f2480` | Terrain::Scale8F2480 | In outline: a point scaled by a grid's size (+8, +0xC) |
| `0x977f00` | Water::Draw977F00 | In outline: when two cells differ, a draw step with the shared device |
| `0x943570` | Water::Draw943570 | In outline: a draw step with the shared device |
| `0x935910` | Water::Release935910 | In outline: takes one off a part's count (+0x28 +4) and frees it at 0 |
| `0x877980` | Scenery::Load877980 | In outline: a scenery load step (Scenery::Load876810) |
| `0x7f6440` | Person::Attraction7F6440 | In outline: when not flagged (+0xC0), a step with its attraction and person (AttractionPersonInfo) |
| `0x7bec40` | Person::Part7BEC40 | In outline: picks a record (+0x1C or +0x38) when not flagged (+0x77) |
| `0x9fb360` | Fireworks::Set9FB360 | In outline: for entry n (0x90 bytes) with no slot (+0x6C = -1), sets values |
| `0x8e6880` | Water::Time8E6880 | In outline: a fraction of the shared day (`0x1103B6C` +0x110 between +0x20 and +0x200), kept between 0 and 1 |
| `0x835910` | Scenery::Link835910 | In outline: a step with a linked part (+0x4C) |
| `0x7a3860` | Terrain::Version7A3860 | In outline: the entry for a version (SortedTable::FindVersion), or adds one |
| `0x7856c0` | GuestGroup::Slot7856C0 | In outline: goes over slots 0 to 6 (+0x60) |
| `0x724e60` | Path::Buffer724E60 | In outline: makes a device buffer of n four-byte entries (ForCurrent(0)) and fills it |
| `0xaf3820` | Animal::PassAF3820 | In outline: goes over entries (+0x50) |
| `0xa75700` | Animal::PoseA75700 | In outline: a pose (`0xA6D7B0`) and a point step |
| `0x8f2380` | Terrain::Near8F2380 | In outline: a step over grid entries within a distance |
| `0x8be510` | Terrain::Fill8BE510 | In outline: fills values from a list |
| `0x86eab0` | Scenery::Count86EAB0 | In outline: counts up (+0xF8) and goes over a list (+0x130) |
| `0x852e90` | Scenery::Bit852E90 | In outline: a step with a bit from a count (+0xC) |
| `0x802f40` | Regions::Loop802F40 | In outline: goes over the shared list (`0x10F06D0`) |
| `0xac1a00` | Fence::GrowAC1A00 | In outline: grows a list (Fence::GrowAC11D0) and acts |
| `0x725e30` | Path::Init725E30 | In outline: clears a value, makes a part and a step (`0x730B30`) |
| `0xaa02d0` | PoolComplex::FindAA02D0 | In outline: a find (`0xA9F360`) and a step |
| `0x884540` | Scenery::Range884540 | In outline: a step for n within a range (+4, +8) |
| `0x827a40` | Scenery::Activate827A40 | In outline: activates a part when not flagged |
| `0x7e9dd0` | Person::Step7E9DD0 | In outline: moves a cell one step by direction, within bounds |
| `0x7cafc0` | Person::Event7CAFC0 | In outline: when not flagged (+0xC7, bit 0x800000 of +0x3EC), acts by an event (9, ...) |
| `0x856ac0` | Scenery::Grow856AC0 | In outline: when n passes its room (+0x154), frees and remakes two buffers (+0x148, +0x14C) at twice n |
| `0x802ce0` | Save::Write802CE0 | In outline: writes a value (SaveWriter::Write) and mixes in a check value |
| `0x7a1190` | Terrain::Step7A1190 | In outline: a step (`0x7A0000`) with its part (+0x510) |
| `0x754670` | Guest::Make754670 | In outline: fills a record with its table (`0xF6C0BC`) from its arguments |
| `0x730800` | Path::Release730800 | In outline: with a value (+0xD0), releases its part (+0xD4) and clears it |
| `0x713c40` | Path::Check713C40 | In outline: unless a check passes (`0x71C670`), a step for kinds other than 3 |
| `0xb890a0` | Animal::PassB890A0 | In outline: goes over entries (+0x48, +0x4C) |
| `0xa7a000` | Animal::PassA7A000 | In outline: makes a part and goes over entries (+0xC, +0x10) |
| `0x9f9c10` | Fireworks::Bit9F9C10 | In outline: a step with a bit from a count (+0xC) |
| `0x7ec730` | Person::Set7EC730 | In outline: sets a value (+0x41C); with 0 clears two slots (+0x420, +0x428) |
| `0x560720` | Ride::Height560720 | In outline: a point along a direction and a height step when the terrain is not flagged |
| `0x925ac0` | Water::Segment925AC0 | In outline: a point's distance along a line segment, with ends handled |
| `0x8de100` | Terrain::Step8DE100 | In outline: a terrain cell step when the terrain is not flagged |
| `0x7f8710` | Person::Mode7F8710 | In outline: a guest step (Guest::Mode75ABF0) with a pointer table value (+0x121) |
| `0x7adb70` | Terrain::Make7ADB70 | In outline: unless the shared value is set (`0x1306C68`), makes a part |
| `0x8dd9f0` | Terrain::Step8DD9F0 | In outline: a terrain cell step when the terrain is not flagged |
| `0x89fc80` | Scenery::Make89FC80 | In outline: a step (`0x946F50`) and sets up a record with its table (`0xF7A254`) |
| `0x852c60` | Scenery::Bit852C60 | In outline: a step with a bit from a count (+0xC) |
| `0x8283b0` | Scenery::Copy8283B0 | Copies a record's values from another |
| `0x7540a0` | Guests::Make7540A0 | In outline: unless the shared value is set (`0x13064FC`), makes a 0x2C-byte part |
| `0x482160` | Draw::Box482160 | In outline: a box around a point |
| `0xb9d130` | PathGrid::MakeB9D130 | In outline: unless the shared value is set (`0x133A270`), makes a 0x178-byte part |
| `0x867a20` | Scenery::Check867A20 | In outline: a check when a value is up to 0 and there are no entries (+0x108) |
| `0x852740` | Scenery::Bit852740 | In outline: a step with a bit from a count (+0xC) |
| `0x849d80` | Scenery::Kind849D80 | In outline: with parts, acts by a kind (0x12, 0x2E, ...) |
| `0x941c00` | Water::Shared941C00 | In outline: with a part (+0xB0 +0x10), a step once (`0x133A31C`) |
| `0x8de200` | Terrain::Step8DE200 | In outline: clears a shared flag (`0x1313C64` bit 8) and a step when the terrain is not flagged |
| `0x8ddda0` | Terrain::Step8DDDA0 | In outline: a terrain cell step when the terrain is not flagged |
| `0x5e63f0` | TrackPiece::Kind5E63F0 | In outline: for kind 0x1D, a step when not flagged (+0x18C) |
| `0x840350` | Scenery::Make840350 | In outline: makes a scenery part, none when given nothing |
| `0x7a4970` | Terrain::Count7A4970 | In outline: counts up (+0xC) and acts |
| `0x822cb0` | SceneryBehaviour::Loop822CB0 | In outline: goes over the shared list (`0x10F0784`) |
| `0x7e9f10` | Person::Terrain7E9F10 | In outline: with a part (+0x4E0), a terrain step |
| `0xa8bb10` | Animal::CountA8BB10 | In outline: counts up a total (+0x6C) and a count by kind (+0x70 + kind) |
| `0x7c52b0` | Person::Slot7C52B0 | In outline: a check for slot n (0x14 bytes at +0x128) |
| `0x8de000` | Terrain::Step8DE000 | In outline: a terrain cell step when the terrain is not flagged |
| `0x88f3f0` | Scenery::Cell88F3F0 | In outline: a value for a cell of grid n (+0x390, 64 bytes each) |
| `0x774700` | Park::Slot774700 | In outline: a step for slot n under a count (slot 0x58) |
| `0x8b5bd0` | SceneryBehaviour::Matrix8B5BD0 | In outline: a matrix from its owner's part with a value (+0xFC) |
| `0x737a70` | Path::Make737A70 | In outline: makes a 0xD8-byte part |
| `0x497890` | Save::Step497890 | In outline: a step with a value (+0x818) |
| `0x9fb510` | Fireworks::Entry9FB510 | In outline: for entry n (0x24 bytes) with no slot (+8 = -1), a step |
| `0x86ef60` | Scenery::Make86EF60 | In outline: sets up a record with its tables (`0xF3A07C`) |
| `0x7468b0` | Guest::Make7468B0 | In outline: the same (another copy) |
| `0x728f00` | Path::Case728F00 | In outline: a value by a kind (slot 0x30) |
| `0x727720` | Path::Nearest727720 | In outline: finds the nearest (from the largest float) |
| `0x723a80` | Path::Clear723A80 | In outline: clears a record's values (+0x64 on) |
| `0x723990` | Path::Clear723990 | In outline: the same (another copy) |
| `0x7238a0` | Path::Clear7238A0 | In outline: the same (another copy) |
| `0x87f6c0` | Scenery::Value87F6C0 | In outline: a value (+0x38) with the shared count (`0x10ED7D0`) |
| `0x923a90` | Water::Camera923A90 | In outline: unless the shared value is set (`0x13429A8`), a step with the camera and the water manager |
| `0x8c49d0` | Terrain::Sine8C49D0 | In outline: a step with the sine of minus a value (+0xE0) |
| `0x8533b0` | Scenery::Bit8533B0 | In outline: a step with a bit from a count (+0xC) |
| `0x844b30` | Scenery::Reset844B30 | In outline: clears a value (+0x2C) and goes over a list (+0xC) |
| `0x76b790` | Guest::Load76B790 | In outline: reads values from a saved block |
| `0x44f150` | Effects::Load44F150 | In outline: reads values from a saved block |
| `0x739ad0` | Path::Pass739AD0 | In outline: goes over entries (+8) |
| `0x731880` | Path::Pass731880 | In outline: the same (another copy) |
| `0xbae8e0` | Enclosure::FindBAE8E0 | In outline: a find (`0xBAD550`, `0xBAD5C0`) |
| `0x852650` | Scenery::Bit852650 | In outline: a step with a bit from a count (+0xC) |
| `0x874f20` | Scenery::Add874F20 | In outline: adds an entry, growing a list (+0x7C) when full |
| `0x811670` | Regions::Draw811670 | In outline: a draw step with the shared device (slot 0xC8) |
| `0x7a9400` | Terrain::Shared7A9400 | In outline: a step with the shared object (`0x1106C54`), made when none |
| `0xbadcc0` | Enclosure::CheckBADCC0 | In outline: 0 unless found (`0xBAD5C0`) |
| `0x8850f0` | Scenery::Find8850F0 | In outline: a find over entries (+0xF8) by a value |
| `0x8847a0` | Scenery::Slot8847A0 | In outline: a slot from a value scaled to a range |
| `0x7a12d0` | Terrain::Step7A12D0 | In outline: a step with its part (+0x510) |
| `0x741940` | Guest::Add741940 | In outline: grows a list by two when full (ObjectArray::SetCapacity on +0x514) and adds |
| `0x93d870` | Water::Grid93D870 | In outline: makes a grid of 0x30-byte cells |
| `0x8619e0` | Scenery::Case8619E0 | In outline: a value by an entry's kind |
| `0x7a02b0` | Terrain::Step7A02B0 | In outline: a step with its part (+0x510) |
| `0x95dd10` | Water::Fill95DD10 | In outline: fills values from a list |
| `0x470a10` | Draw::Slot470A10 | In outline: a step for a 24-byte slot |
| `0xada9c0` | Animal::HeightADA9C0 | In outline: a height step when the terrain is not flagged |
| `0x8db980` | Terrain::Case8DB980 | In outline: sets a shared callback (`0x10F0AD8`) and acts by it |
| `0x8c5250` | Terrain::Make8C5250 | In outline: fills a record with its table (`0xF7ABB8`) |
| `0x88bd80` | SceneryBehaviour::Terrain88BD80 | In outline: a step with its owner when the terrain is not flagged |
| `0x83fad0` | Scenery::Add83FAD0 | In outline: adds an entry, growing a list (500 at first) when full |
| `0x72b030` | Path::Quad72B030 | In outline: a cell's quad (Cell::Quad) and a step |
| `0x71cfa0` | Path::Terrain71CFA0 | In outline: a step when the terrain is not flagged |
| `0xa8e970` | Animal::LoopA8E970 | In outline: goes over the shared list (`0x10F266C`) |
| `0x9f9da0` | Fireworks::Find9F9DA0 | In outline: a find over entries (+0x1C) |
| `0x81b260` | SceneryBehaviour::Text81B260 | In outline: with no part (+0xC8), a text check (`0x403C20`) |
| `0x960480` | Water::Next960480 | In outline: a step for the next entry of a list (+0x60) |
| `0x8e90e0` | Water::Remove8E90E0 | In outline: removes by id (IdMap::Remove6) and acts |
| `0x885c80` | Scenery::Value885C80 | In outline: a value from its part (+0xFC +0xDC) and a step (`0x884B50`) |
| `0xa1d240` | Window::ValueA1D240 | In outline: a value from a part (+0xB4) |
| `0xb84800` | Animal::AddB84800 | In outline: adds to an entry's value (`0xB829C0`) |
| `0x925f60` | Water::Set925F60 | In outline: sets its part's values (+0x328) |
| `0x85d200` | Scenery::Round85D200 | In outline: a value rounded from a scale |
| `0x8531b0` | Scenery::Bit8531B0 | In outline: a step with a bit from a count (+0xC) |
| `0x852b50` | Scenery::Bit852B50 | In outline: the same (another copy) |
| `0x852950` | Scenery::Bit852950 | In outline: the same (another copy) |
| `0x852370` | Scenery::Bit852370 | In outline: the same (another copy) |
| `0x852170` | Scenery::Bit852170 | In outline: a step with a bit from a count (+0xC) |
| `0x851f70` | Scenery::Bit851F70 | In outline: the same (another copy) |
| `0x8532c0` | Scenery::Bit8532C0 | In outline: the same (another copy) |
| `0x8530c0` | Scenery::Bit8530C0 | In outline: the same (another copy) |
| `0x852a60` | Scenery::Bit852A60 | In outline: the same (another copy) |
| `0x852860` | Scenery::Bit852860 | In outline: the same (another copy) |
| `0x852280` | Scenery::Bit852280 | In outline: the same (another copy) |
| `0x852080` | Scenery::Bit852080 | In outline: the same (another copy) |
| `0x851e80` | Scenery::Bit851E80 | In outline: the same (another copy) |
| `0x8e6d20` | Water::Point8E6D20 | In outline: a point from a matrix's values |
| `0x834030` | Scenery::Make834030 | In outline: unless the shared value is set (`0x130B5D0`), makes a 0xC-byte part |
| `0x799a40` | Guest::Grow799A40 | In outline: grows a buffer (+0x10C) in steps of 8 to hold n more |
| `0x714e40` | Path::Step714E40 | In outline: a step with the shared value (`0x1306224`, `0x713240`) and a linked part (+0x218) |
| `0x9eade0` | Weather::Start9EADE0 | In outline: unless the shared value is set (`0x10EDAE0`), starts the weather loop (Weather::StartLoop) |
| `0x91c630` | Array::Resize91C630 | In outline: resizes a list to n |
| `0x91a0b0` | Array::Resize91A0B0 | In outline: resizes a list to n |
| `0x9123e0` | Water::Make9123E0 | In outline: fills a record from its arguments |
| `0x86f5b0` | Scenery::Pass86F5B0 | In outline: with no part value (+0xCC +0x5C), goes over a list (+0x148) |
| `0x863a60` | Array::Resize863A60 | In outline: resizes a list to n |
| `0x856900` | Scenery::Points856900 | In outline: goes over n points |
| `0x74dc30` | Guest::Set74DC30 | In outline: sets its part's value (+0xE4 +4) and acts (`0x749D70`) |
| `0xa91c80` | Fence::QuadA91C80 | In outline: fills a 0x60-byte quad record from its corners |
| `0x989300` | Fireworks::Find989300 | In outline: a search over entries (+0x58) |
| `0x8ff6a0` | Water::Pass8FF6A0 | In outline: goes over entries (+4) |
| `0x97aca0` | Water::Points97ACA0 | In outline: goes over n points |
| `0x939ac0` | Array::Resize939AC0 | In outline: resizes a list to n |
| `0x81fa00` | SceneryBehaviour::Check81FA00 | In outline: a check when not flagged (+0xF4) |
| `0x7a10b0` | Terrain::Step7A10B0 | In outline: a step with its part (+0x510) |
| `0x7a0fd0` | Terrain::Step7A0FD0 | In outline: the same (another copy) |
| `0x72c640` | Path::Text72C640 | In outline: a text record when it has none (+0xC) |
| `0xb758e0` | Animal::TerrainB758E0 | In outline: a step when the terrain is not flagged |
| `0xb5f5e0` | Animal::TerrainB5F5E0 | In outline: the same (another copy) |
| `0x7bb530` | Person::Terrain7BB530 | In outline: the same (another copy) |
| `0x910270` | Water::Device910270 | In outline: a step with the shared device's value (slot 0xCC) |
| `0x8b3cc0` | Scenery::List8B3CC0 | In outline: goes over a list (+0x64) |
| `0x871f20` | Scenery::Kind871F20 | In outline: when a flag is set and its part has a value, acts by its kind (+0xCC +0x48) |
| `0x79cb60` | Toolbar::Next79CB60 | In outline: when two shared values differ, keeps a shared index (`0x1306BB8`) in range |
| `0x73ca70` | Guest::Buffer73CA70 | In outline: reserves a buffer (Buffer16::Reserve) and fills it |
| `0xb9f8a0` | PathGrid::EntryB9F8A0 | In outline: entry n (0xB8 bytes), 0 for a negative n |
| `0x95a700` | Water::Entry95A700 | In outline: a value from a 16-byte entry (+0x14 by +0x1C) |
| `0x91c0e0` | Water::Step91C0E0 | In outline: a step with three values |
| `0x905ba0` | Water::Fill905BA0 | In outline: fills values from its arguments |
| `0x891680` | Scenery::Add891680 | In outline: adds an entry, with a free list (+0x14) or growing |
| `0x867ec0` | Scenery::Pass867EC0 | In outline: goes over a part's entries (+0xC) |
| `0x73e520` | Guest::Link73E520 | In outline: a step with a linked part (+8) |
| `0x716c30` | Path::Step716C30 | In outline: a step with its part (+0x23C) |
| `0x876660` | Scenery::Pass876660 | In outline: goes over a part's entries (+0x9C) |
| `0x82b220` | Scenery::Set82B220 | In outline: a step (`0x829B70`), sets a value (+0xAC) and acts with a part (+0x84) |
| `0x7855e0` | GuestGroup::Slots7855E0 | In outline: goes over slots 0 to 6 (+0x60) |
| `0x724300` | Path::Slot724300 | In outline: when a slot check passes (Path::Slot72FA20), a value from a count (+0x9D) |
| `0x7241e0` | Path::Slot7241E0 | In outline: the same with Path::Slot72F610 and +0x9C |
| `0x9454e0` | Water::Remove9454E0 | In outline: removes entry n (+0x64) from its list (Entries::Remove) |
| `0x9761b0` | Water::Step9761B0 | In outline: a step with a value or the shared one (`0x1310E60`) |
| `0xb53fb0` | Animal::LoopB53FB0 | In outline: goes over the shared list (`0x13CEA54`) |
| `0x82a7b0` | Scenery::Handle82A7B0 | In outline: makes a handle and links it |
| `0x47cdd0` | Effects::Reset47CDD0 | In outline: with the shared value (`0x10ED6F4`), goes over the effects (Effects::Loop47CCD0) and clears a shared value |
| `0x476a30` | Effects::Reset476A30 | In outline: with the shared value (`0x10ED708`), goes over (Effects::Loop476950) |
| `0x8b9dd0` | Scenery::Mode8B9DD0 | In outline: unless in mode 2 or 3 (+0x1F0), a handle step |
| `0x8b6730` | Scenery::Release8B6730 | In outline: with a slot (+0xA4), removes it from its pool (SlotPool::Remove) and clears it |
| `0x7a03a0` | Terrain::List7A03A0 | In outline: goes over a list (+0x540) |
| `0x77fa10` | GuestGroup::Pass77FA10 | In outline: when a group check passes (GuestGroup::Check778580(1)), goes over entries |
| `0x772590` | Park::Make772590 | In outline: a step (`0x7724A0`) and fills a record |
| `0x750f90` | Guest::Make750F90 | In outline: with no part (+0x3C), makes a 0x224-byte one |
| `0x7731f0` | Guest::Mode7731F0 | In outline: a step for modes under 7 (+0x190) |
| `0x9ed8c0` | Fireworks::Release9ED8C0 | In outline: releases its part (+0x148) and clears it |
| `0x965bd0` | Water::Step965BD0 | In outline: a step with a count (+0x20) |
| `0x83f900` | Scenery::Same83F900 | In outline: a step when a value matches its own (+0xE8) |
| `0x726c70` | Path::Terrain726C70 | In outline: a terrain step finding the nearest |
| `0x89caf0` | Scenery::Cell89CAF0 | In outline: a step for a cell of a grid (16 per cell) that is set |
| `0x84a9a0` | Scenery::Check84A9A0 | In outline: a check when not flagged (+0xC8) |
| `0x7526a0` | Guests::Step7526A0 | In outline: when not flagged (+0x6C, +0x84), a step with the shared value (`0x14002F8`) |
| `0x4413d0` | Park::Value4413D0 | In outline: a value from the park (Park::Get) or the shared one (`0x1306194`) |
| `0x42c610` | Sky::Step42C610 | In outline: a step with a count (+0x1C) |
| `0x416c90` | Effect::Make416C90 | In outline: makes parts |
| `0xaa0760` | PoolComplex::CountAA0760 | In outline: counts entries (+0x34) |
| `0x929590` | Water::Set929590 | In outline: sets two points (+0x3D0, +0x3E0) and acts |
| `0x82b060` | Scenery::Set82B060 | In outline: a step (`0x829B70`), sets a value (+0xA4) and acts when not flagged (+0x130) |
| `0x7f4810` | Person::Value7F4810 | In outline: with no part (+0x194), sets a value to 0xF3 (+0xB8) and acts by a time (+0xBC) |
| `0x85edf0` | Scenery::Find85EDF0 | In outline: a find (`0x827530`) |
| `0x7ca650` | Person::Slot7CA650 | In outline: a step for slot n (0x14 bytes at +0x128) when its cell changes |
| `0x88af00` | Scenery::Camera88AF00 | In outline: unless the shared value is set (`0x13429A8`), a step with the camera |
| `0x49c740` | Effects::Shared49C740 | In outline: a step with the shared object (`0x1106C54`) |
| `0x42e2b0` | Effect::Link42E2B0 | In outline: a step with a linked part (+0x144) |
| `0x90b230` | Water::Cell90B230 | In outline: a cell from a value's place within a range, scaled by a count |
| `0x9ee710` | Fireworks::Reset9EE710 | In outline: releases (`0x9ED8C0`) and goes over the shared list (`0x1341CFC` +4) |
| `0xb88030` | Animal::LinkB88030 | In outline: a step with a linked part (+8) |
| `0x90b150` | Water::Cell90B150 | In outline: a cell from a value's place within a range (+8 to +0x10), scaled by a count |
| `0x852550` | Scenery::Bit852550 | In outline: a step with a bit from a count (+0xC) |
| `0x851d60` | Scenery::Bit851D60 | In outline: the same (another copy) |
| `0x83fda0` | Scenery::Rect83FDA0 | In outline: a terrain step over a byte rectangle |
| `0x7ae5a0` | Terrain::Step7AE5A0 | In outline: a step when the terrain is not flagged |
| `0x774620` | Park::Pass774620 | In outline: when given a value, goes over entries |
| `0x44fba0` | Effects::Reset44FBA0 | In outline: when given an object, clears its flag (+0x100) and tells it (slot 0x30) |
| `0x925830` | Water::Camera925830 | In outline: when on (+0x170) and the shared value is unset, a camera step |
| `0x8e9520` | Water::Make8E9520 | In outline: sets up a record with its tables (`0xF7CCBC`) |
| `0x881b80` | Scenery::Text881B80 | In outline: unless a flag is set (Flags::Bit0), makes a text |
| `0x808340` | Regions::Copy808340 | In outline: copies an entry's values (+0x28 + n) |
| `0x801a00` | Regions::Init801A00 | In outline: sets up a record with cleared values |
| `0x9fb280` | Fireworks::Entry9FB280 | In outline: for entry n (0x90 bytes) with a slot (+0x6C), a step |
| `0x98aec0` | Fireworks::Count98AEC0 | In outline: counts over the shared list (`0x10F0CE8`) |
| `0x801d50` | Regions::Bytes801D50 | In outline: a step over a 16-byte buffer |
| `0x709590` | Path::Make709590 | In outline: makes a record |
| `0x876cd0` | Scenery::Make876CD0 | In outline: makes a 0x14-byte record |
| `0x7bc9b0` | Person::Check7BC9B0 | In outline: a step when a value (`0x749B60`, `0x495220`) passes a count (+0x30) |
| `0x7a6c70` | Terrain::Mode7A6C70 | In outline: when on (+0x538), sets a mode to 0xF (+0x190) and clears a value (+0x194) |
| `0x750ea0` | Guest::Case750EA0 | In outline: a step by a mode (+0x18) |
| `0x9e8ec0` | Fireworks::Time9E8EC0 | In outline: with a positive time (+8), a step (`0x9FBED0`) |
| `0x973040` | Water::Step973040 | In outline: a step with many values |
| `0x93cc50` | Water::Set93CC50 | In outline: clears a flag with no part (+4), else a step with its value (+0x30) |
| `0x91f9a0` | Water::Grow91F9A0 | In outline: grows a list (ObjectArray::SetCapacity to +0x154) and fills it |
| `0x8b09e0` | Scenery::Shared8B09E0 | In outline: a step (`0x682D50`) and once (`0x1311284`) more |
| `0x85ad90` | Scenery::Make85AD90 | In outline: makes a part (`0x8E9480`) |
| `0x83da90` | Scenery::Init83DA90 | In outline: sets up a record with its tables (`0xF77398`, `0xF77380`, `0xF77378`) |
| `0x7a9270` | Terrain::Mode7A9270 | In outline: for an entry in mode 2 (+0x70), a handle step |
| `0x90f770` | Water::Find90F770 | In outline: a find over entries (+0x4C, +0x54) |
| `0x8dcb70` | Terrain::Step8DCB70 | In outline: a step when the terrain is not flagged |
| `0x8dcaa0` | Terrain::Step8DCAA0 | In outline: the same (another copy) |
| `0x840d90` | Scenery::Terrain840D90 | In outline: a step when the terrain is not flagged |
| `0x73c240` | Guest::Terrain73C240 | In outline: a step with a value (+0x80) when the terrain is not flagged |
| `0x92e070` | Water::Fraction92E070 | In outline: a value from n over a count minus one |
| `0x882c10` | Scenery::Clear882C10 | In outline: tears down texts and more |
| `0xab6050` | PoolComplex::ColoursAB6050 | In outline: a pool part's colours by its kind (+0x120): fills colour bytes (0xC0, 0x48) for kinds 5 to 9 and splits a colour (+0x118) into bytes |
| `0x878360` | Scenery::Place878360 | In outline: a large scenery place step: makes the shared terrain (`0x1311650`, RCT3Terrain, 90 by 90) when there is none, reads the enclosure manager's cell and goes over its entries (+0x7C, +0x80) |
| `0x83dc00` | Scenery::Mouse83DC00 | In outline: a large mouse step with the GUI root (`0x12F8404`, `0x12F83F0`): when on, places items by the mouse with the camera and the shared terrain |
| `0x8f4df0` | Terrain::Count8F4DF0 | In outline: a large terrain count: makes a list sized by the shared count (`0x1311620`) and counts entries by a byte |
| `0x7dad70` | Person::Timed7DAD70 | In outline: a large timed person step (`0x1308CA8`, Timer::Ticks) that makes pool parts (PoolComplex::MakeAC6870) |
| `0x939ee0` | Water::Step939EE0 | In outline: a large water step below a limit (`0x10F0C5C`), with a ride's values when asked |
| `0x90dea0` | Water::Rebuild90DEA0 | In outline: releases its part (+0x20), frees a buffer (+0x24) and rebuilds with the shared terrain (`0x1311650`) |
| `0x8c5360` | Terrain::Ray8C5360 | In outline: a large ray step against the ground (Terrain::RayGround) by grid steps (+0x14) |
| `0x9344d0` | Water::Ride9344D0 | In outline: a large step with the current tool's ride (flag bit 15) and a part's values (+0x98, +0xA8) |
| `0x734400` | Path::Terrain734400 | In outline: a large path step with the shared terrain (`0x1311650`, 90 by 90) and cell sizes |
| `0x930f30` | Water::Build930F30 | In outline: a large water build: makes room (`0x92E7F0` for 2n and 6n-6) and fills from a part (+0x120, +0x124) |
| `0xba0b40` | Enclosure::RectBA0B40 | In outline: a large step over a rectangle kept inside its grid (+0x64, +0x68), with the shared terrain |
| `0x935a30` | Water::Rebuild935A30 | In outline: a large rebuild: a step (`0x92EA60`) and releases parts (+0x24, +0x2C, +0x30) |
| `0x8efd20` | Water::Normals8EFD20 | In outline: works out each triangle's normal (cross products) for a list |
| `0x95c490` | Water::Rebuild95C490 | In outline: releases its part (+0x30), fills index lists with 0xFFFF and rebuilds |
| `0x92fcb0` | Water::Ride92FCB0 | In outline: a large step when on (+0x1B1) with a ride part |
| `0x91cad0` | Waterfall::Step91CAD0 | In outline: unless the waterfall manager is flagged (+0x68 bit 0), a large step from its points (+0x180 on) over the terrain; else copies them and gives 1 |
| `0x8f6cb0` | Terrain::Count8F6CB0 | In outline: clears a list sized by the terrain's size and the shared count (`0x1311618`) and fills it |
| `0x854a20` | Scenery::Cell854A20 | In outline: for a cell's entry (+0x28), counts it up and acts by case |
| `0x493200` | Terrain::Step493200 | In outline: a step with the shared terrain, made when none (90 by 90) |
| `0x91a5d0` | Waterfall::Step91A5D0 | In outline: a large step scaling values by a time (+0xE8, +0xE0) and a fraction under 1 |
| `0x90ba10` | Water::Terrain90BA10 | In outline: with a part (+0x18) and entries, a terrain step |
| `0x786340` | GuestGroup::Slots786340 | In outline: goes over slots 0 to 6 (+0x60) for those flagged (bit 6) |
| `0x7a0b50` | Terrain::Link7A0B50 | In outline: a step over a list (+0x540) |
| `0x93da00` | Water::Cell93DA00 | In outline: values for a cell inside the grid (+0x58, +0x59), 0x30 bytes each |
| `0x93dbf0` | Water::Cell93DBF0 | In outline: the same, 0 outside |
| `0x9935e0` | Fireworks::Load9935E0 | In outline: reads four-byte values from a saved block (Load::ReadBytes) |
| `0x54ad80` | Ride::Load54AD80 | In outline: the same (another copy) |
| `0x7c9e60` | Person::Terrain7C9E60 | In outline: a step when the terrain is not flagged |
| `0x94ef60` | Water::Entry94EF60 | In outline: for entry n (0xD0 bytes) of kind 4, a value by a count under 0xE |
| `0x829b70` | Scenery::Footprint829B70 | In outline: with a part (+0x9C) in turn 0 or 2, turns its cells (Footprint::RotateCell) |
| `0x79c0a0` | Toolbar::Find79C0A0 | In outline: a find when two shared values differ (`0x1306BCC`, `0x1306BD8`) |
| `0x82fb30` | Scenery::SetCell82FB30 | In outline: for 0, when a cell changes (+0xCC to +0xD4), turns it (Scenery::Cell82B350) and sets it |
| `0x61cf40` | Scenery::Make61CF40 | In outline: with a part (+0x74 +0xC) not flagged, makes a part |
| `0xa906c0` | Fence::CellA906C0 | In outline: a cell from a point, rounded down |
| `0x869890` | Scenery::Add869890 | In outline: a step (`0x865580`) and makes a 0xC-byte record |
| `0x4944d0` | Ride::Clamp4944D0 | In outline: keeps a range inside a count (+8) |
| `0x847530` | Scenery::Turn847530 | In outline: with its owner (slot 0x94), a turn (Clamp0To3) |
| `0x833f90` | Scenery::Ref833F90 | In outline: takes a reference to a shared part and a value |
| `0x736630` | Path::Link736630 | In outline: puts an entry at the front of a linked list (+0x54) |
| `0x72ca50` | Path::Tick72CA50 | In outline: every fourth time (+0x8D), activates a path tile (PathTile::Activate) and sets a value |
| `0x71dc50` | Path::Bounds71DC50 | In outline: when asked, grows byte bounds (+0x9C on) to hold a cell |
| `0x86d0a0` | Scenery::SetPoint86D0A0 | In outline: for 0, sets a point (+0x30 to +0x38) when it changes |
| `0x845850` | Scenery::Check845850 | In outline: a check unless flagged (slot 0x28) |
| `0x827530` | Scenery::Turn827530 | In outline: turns a cell by a turn (+0x10) |
| `0x8fb1f0` | Water::Find8FB1F0 | In outline: a find over the shared list (`0x1339FD8`) |
| `0x7837d0` | GuestGroup::Slots7837D0 | In outline: goes over slots 0 to 6 (+0x60) |
| `0x773840` | Park::Find773840 | In outline: a find over entries (+8, +0x20) |
| `0x9e3240` | Fireworks::AddTime | In outline: adds 50 times n to a value (+0x28), kept between 0 and 2000 |
| `0x5f7900` | Ride::Drag5F7900 | In outline: a step once the mouse has moved far enough from where it started |
| `0x9d4eb0` | Fireworks::Time9D4EB0 | In outline: when on with a part, counts a time (+0x84) down |
| `0x7e8ca0` | Park::Rebuild7E8CA0 | In outline: a rebuild step (Park::Rebuild7E7440) and more for kind 4 |
| `0x72d250` | Path::Part72D250 | In outline: a step with a part (by a shared id `0x130646C`) |
| `0x8606e0` | Scenery::Count8606E0 | In outline: counts a part's entries (+0xC +0x14) |
| `0x72b140` | Path::Quad72B140 | In outline: for kind 3, a cell's quad (Cell::Quad) and fills |
| `0x865410` | Scenery::Shapes865410 | In outline: when flagged (bit 3), checks shapes (Model::CheckShapes) |
| `0x93cd30` | Water::Make93CD30 | In outline: with no part (+4), a step with the shared object (`0x12F468C`) |
| `0x8fb270` | Water::Find8FB270 | In outline: a find over the shared list (`0x1339FD8`) |
| `0x7737d0` | Park::Find7737D0 | In outline: a find over entries (+8, +0x20) |
| `0x492ed0` | Terrain::Height492ED0 | In outline: a height when the terrain is not flagged |
| `0x833cc0` | Scenery::Link833CC0 | In outline: a step with a linked part (+0x48) |
| `0x84d220` | Scenery::Copy84D220 | In outline: copies a record's values from another |
| `0x71b0f0` | Path::Case71B0F0 | In outline: a value by case |
| `0x772c90` | Park::Find772C90 | In outline: a find over entries (+8, +0x10) |
| `0x7ae680` | Terrain::Inside7AE680 | In outline: whether a cell lies inside its byte bounds (+4 to +7) |
| `0x7cadd0` | Person::Link7CADD0 | In outline: clears a value (+0x350) and a step with a linked part (+0x35C) |
| `0x83f800` | Scenery::Set83F800 | In outline: for 0 clears a list (ObjectArray52::Resize(0)), and acts when its one entry differs |
| `0x72eca0` | Path::Refresh72ECA0 | In outline: updates (Path::Update7266B0), refreshes (Path::Refresh72E7C0(1)) and acts with a part |
| `0x93e7d0` | Water::Clip93E7D0 | In outline: with a part (+0xB0), a clipped step (Water::Clip95AA10) |
| `0x93cbe0` | Water::Release93CBE0 | In outline: with a part (+4), a step with the shared object (`0x12F468C`) |
| `0x869950` | Scenery::Find869950 | In outline: a find over a list (+0xF4) |
| `0x8609a0` | Scenery::Value8609A0 | In outline: a value from entries (+0x18) |
| `0x94dd40` | Water::Entry94DD40 | In outline: for entry n (0xD0 bytes) in mode 1, copies a value |
| `0x7289d0` | Path::Case7289D0 | In outline: a step by a kind (+0x60) |
| `0x4377e0` | Effects::Value4377E0 | In outline: a value from a shared table (`0x10F2BA8`) by a byte |
| `0x758f70` | Guest::Check758F70 | In outline: 1 when a check passes (`0x7DF1F0`), else a step with the park |
| `0x8348e0` | SceneryBehaviour::Centre8348E0 | In outline: a point from its owner's values |
| `0x45eea0` | List::Find45EEA0 | In outline: a find over entries (+4, +8) |
| `0x8fe240` | Water::Entry8FE240 | In outline: a point from entry n (0x60 bytes at +0xA8) |
| `0x8605f0` | Scenery::Value8605F0 | In outline: a value from a part scaled by a table (`0x10F087C`) and a shared value |
| `0x704a50` | Path::Cell704A50 | In outline: a cell (+0x1A8, +0x1AC, or +0x1B4 when flagged) |
| `0x6c3cc0` | List::Count6C3CC0 | In outline: 0 when empty, else a count step |
| `0x588ae0` | Ride::Part588AE0 | In outline: a step with a part (by a shared id `0x12F53F0`) |
| `0x838710` | Scenery::Value838710 | In outline: a value by a kind (0, 0xB, 0x1E, ...) |
| `0x95a570` | Water::Remove95A570 | In outline: removes an entry from an index (+0x20) and a list (+8) |
| `0x483d20` | List::Clear483D20 | In outline: empties a list |
| `0x53ddb0` | Ride::List53DDB0 | In outline: goes over section n's list |
| `0x9e0d70` | Fireworks::AddTime9E0D70 | In outline: adds 100 times n to a value (+0x3C), kept in range |
| `0x96fdf0` | Water::Make96FDF0 | In outline: makes a part (`0x965610`) |
| `0x976290` | Water::Make976290 | In outline: makes a part (Object967) |
| `0x970f20` | Water::Entry970F20 | In outline: a loop from entry n (+0x90, eight bytes) |
| `0x9060f0` | Water::Value9060F0 | In outline: a value from a grid cell (+0x44) and its neighbours |
| `0x852480` | Scenery::Bit852480 | In outline: a step with a bit from a count (+0xC) |
| `0x7e6dd0` | Person::Add7E6DD0 | In outline: grows a list when full (Array::Resize7E5400) and adds |
| `0x7a2090` | Terrain::Step7A2090 | In outline: a terrain step |
| `0x431e30` | Sky::Find431E30 | In outline: a find over entries (+0x40, +0x4C) |
| `0x742170` | Guest::Assign742170 | In outline: lets go of a shared part and takes another |
| `0x42dea0` | Terrain::Entry42DEA0 | In outline: entry n (16 bytes) when the terrain is not flagged |
| `0x418f00` | Effects::Step418F00 | In outline: a step with an entry (Thunk on +0x10) |
| `0x937930` | Water::Init937930 | In outline: fills a record from its arguments |
| `0x919430` | Waterfall::Set919430 | In outline: sets points (+0xA0 on) |
| `0x8f3650` | Terrain::Corner8F3650 | In outline: a step with the terrain's corner (+0xC, +0x10) |
| `0x8851e0` | Scenery::Fraction8851E0 | In outline: a fraction from a point (`0x682D80`) and a size (+8) |
| `0x834a90` | Scenery::Part834A90 | In outline: a step with a part (+0x5C) |
| `0x8058f0` | Regions::Loop8058F0 | In outline: with values, goes over a list (+0x10) |
| `0x7713c0` | Guest::Ratio7713C0 | In outline: clears a flag (+0xB0) and sets a value (+0x58) to a share of two values (+0x40, +0x44) |
| `0x8a09f0` | Scenery::Pass8A09F0 | In outline: goes over entries (+0x54) |
| `0x8726a0` | Scenery::Case8726A0 | In outline: for 0x18, a step with a list (+0x1B0) |
| `0x73e6b0` | Guest::Count73E6B0 | In outline: 0 with no entries (+0x518), else a step |
| `0x876740` | Scenery::Pass876740 | In outline: goes over a part's entries (+0x9C +0x34) |
| `0x9788a0` | Water::Grow9788A0 | In outline: when n passes its room (+0x6C), frees and remakes two buffers (+0x60, +0x64) |
| `0x724770` | Path::Cell724770 | In outline: a step with a byte cell |
| `0x8b53b0` | SceneryBehaviour::Step8B53B0 | In outline: with a slot (+0xF4), a step from its owner |
| `0x836d00` | Scenery::Update836D00 | In outline: an update when not flagged (+0x194) |
| `0x713390` | Path::Terrain713390 | In outline: a step when the terrain is not flagged |
| `0x99e810` | Fireworks::Snap99E810 | In outline: snaps to a row (Grid::SnapToRow) and a table entry (Table104::Entry) |
| `0x4c5a90` | Shop::Mode4C5A90 | In outline: when an object is in mode 0xF (+0x194), a step when not flagged (+0x74) |
| `0x7cf710` | Person::Slot7CF710 | In outline: for slot n under a count (slot 0x58), a step |
| `0x781c40` | Array::Free781C40 | In outline: frees an array, tearing each entry down |
| `0x73aa80` | Guest::Update73AA80 | In outline: an update when not flagged (+0x60) |
| `0x506950` | Car::Handles506950 | In outline: for a positive count, makes handles |
| `0x551600` | Ride::Clear551600 | In outline: empties a list |
| `0x551530` | Ride::Clear551530 | In outline: the same (another copy) |
| `0x551460` | Ride::Clear551460 | In outline: the same (another copy) |
| `0x551390` | Ride::Clear551390 | In outline: the same (another copy) |
| `0x96e5e0` | Water::Pass96E5E0 | In outline: goes over entries (+0x50) |
| `0x93d580` | Water::Reset93D580 | In outline: with the shared object (`0x133A274`), resets (Static::Reset) |
| `0x851c90` | Scenery::Bit851C90 | In outline: a step with a bit from a count (+0xC) |
| `0x82fdf0` | Scenery::Part82FDF0 | In outline: a step with a part (+0x34) |
| `0x740420` | Guest::Set740420 | In outline: when given a value and none is set (+0x500), sets it |
| `0x71d900` | Path::Reset71D900 | In outline: resets (Static::Reset) and makes parts |
| `0x9fa210` | Fireworks::Entry9FA210 | In outline: a step for entry n (0x90 bytes) |
| `0x849ea0` | Scenery::Set849EA0 | In outline: sets a value (+0x90) when it changes, under a count |
| `0x73a4d0` | Guest::Handle73A4D0 | In outline: makes a handle and links it |
| `0x7230b0` | Path::Byte7230B0 | In outline: a value from a byte (+0x88) |
| `0x713240` | Path::Check713240 | In outline: a step when a check passes (`0x713180`) |
| `0x713180` | Path::InGrid713180 | In outline: when set, a check with the path manager |
| `0x90f4d0` | Water::Pass90F4D0 | In outline: goes over entries |
| `0x8267c0` | Scenery::Part8267C0 | In outline: a step with a part (+0x18) |
| `0x81bb00` | SceneryBehaviour::List81BB00 | In outline: a step with a list (+0x60) |
| `0x721020` | Path::Join721020 | In outline: when asked, a join check |
| `0x9142c0` | Water::Shared9142C0 | In outline: when the shared object is flagged (bit 0x10), a step for a count under 50 not in mode 1 |
| `0x9051d0` | Water::Find9051D0 | In outline: a find (SortedPointers::FindInt) |
| `0x8df080` | Terrain::Curve8DF080 | In outline: a value from a table by a value, first entry below the start |
| `0x84d3a0` | Scenery::SetCell84D3A0 | In outline: sets a cell (+0x34, +0x38) when it changes |
| `0x54c6d0` | Ride::Clear54C6D0 | In outline: empties a list |
| `0x9797c0` | Water::Draw9797C0 | In outline: when the shared flag is set, sets device state (slot 0xA4 with 5, 2) and draws |
| `0x976880` | Water::Make976880 | In outline: makes a part |
| `0x974900` | Water::Draw974900 | In outline: the same draw (another copy) |
| `0x8e03e0` | Terrain::Step8E03E0 | In outline: a terrain step (Terrain::Step8DFC30) and more |
| `0x8dce20` | Terrain::Check8DCE20 | In outline: a check with the terrain |
| `0x84a620` | Scenery::Update84A620 | In outline: an update when not flagged (+0x6C) |
| `0x784640` | GuestGroup::Slot784640 | In outline: a step with a slot (+0x40) |
| `0x774ce0` | Park::Find774CE0 | In outline: a find over entries (+0x48) |
| `0x72f310` | Path::Rect72F310 | In outline: goes over a byte rectangle |
| `0x702110` | Grid::Step702110 | In outline: a step with many values |
| `0x937a00` | Water::Free937A00 | In outline: frees buffers (+0x38, ...) |
| `0x8b70f0` | Scenery::Update8B70F0 | In outline: an update when not flagged (+0x120) |
| `0x7a9340` | Terrain::Handle7A9340 | In outline: makes a handle and links it |
| `0x79ff40` | Terrain::List79FF40 | In outline: goes over a list (+0x540) |
| `0x77b750` | Park::Pass77B750 | In outline: goes over entries (+0x48) |
| `0xaaf420` | Fence::StepAAF420 | In outline: a step with a value (+0x140) and a cell (+0x128) |
| `0x8a0ac0` | Scenery::Pass8A0AC0 | In outline: goes over entries (+0x54) |
| `0x82bd10` | Scenery::Part82BD10 | In outline: a step with a part (+0x58) |
| `0x725c10` | Path::Pass725C10 | In outline: goes over entries (+0x10) |
| `0x890510` | Scenery::Entry890510 | In outline: a step for entry n (0x150 bytes) |
| `0x91d8c0` | Water::Add91D8C0 | In outline: grows a list by 16 when full (Array::Resize91C630) and adds |
| `0x878100` | Scenery::Step878100 | In outline: a step with a part's value (+0xCC +0x4C) |
| `0x7ed110` | Person::Step7ED110 | In outline: a step with an object (`0x13064A0`) |
| `0x96c3b0` | Water::Make96C3B0 | In outline: makes a window part |
| `0x96c090` | Water::Make96C090 | In outline: the same (another copy) |
| `0x96be30` | Water::Make96BE30 | In outline: the same (another copy) |
| `0x96a570` | Water::Pass96A570 | In outline: goes over entries down from a count (+0x30) |
| `0x969860` | Water::Pass969860 | In outline: the same with +0x20 |
| `0x9681d0` | Water::Make9681D0 | In outline: makes a window part |
| `0x965a50` | Water::Find965A50 | In outline: a find over a list (+0x20) |
| `0x9652e0` | Water::Add9652E0 | In outline: adds when there is room (+0x34, +0x38) |
| `0x9399a0` | Water::Pass9399A0 | In outline: goes over entries (+0x14) |
| `0x92e510` | Water::Value92E510 | In outline: a value from a part (+4 +0xC) |
| `0x7f1ed0` | Person::Case7F1ED0 | In outline: a step by a kind (+0x120: 4, ...) |
| `0x93d7b0` | Water::Rect93D7B0 | In outline: goes over a byte rectangle |
| `0x919b20` | Waterfall::Terrain919B20 | In outline: a step when the terrain is not flagged |
| `0x834c20` | Scenery::Set834C20 | In outline: sets a mode to 1 (+0x74) and a value (+0x7C) |
| `0x6b6920` | Grid::Terrain6B6920 | In outline: a step when the terrain is not flagged |
| `0x807da0` | Regions::Step807DA0 | In outline: a step with a part (+0x24) |
| `0x763380` | Guest::State763380 | In outline: unless in state 0x11 (+0x190), a step |
| `0x735a20` | Path::Rect735A20 | In outline: goes over a rectangle |
| `0x6f7900` | Grid::Pass6F7900 | In outline: goes over entries (+0x12C) |
| `0x6f7840` | Grid::Pass6F7840 | In outline: the same with +0x100 |
| `0x95f3d0` | Water::Step95F3D0 | In outline: a step with work buffers |
| `0x930de0` | Water::Copy930DE0 | In outline: copies a record (`0x92E690`) |
| `0x90ae30` | Water::Count90AE30 | In outline: a step for a positive count |
| `0x7ca940` | Person::Value7CA940 | In outline: in mode 2 (+0x194), a value from a loop (Person::Loop7CA7B0) |
| `0x7a0480` | Terrain::Part7A0480 | In outline: a step with a part (+0x510) |
| `0x724050` | Path::Pass724050 | In outline: goes over entries (+0x70) |
| `0x6fdb50` | Grid::Bits6FDB50 | In outline: for n not negative, a step with a bit table (+0x4C) |
| `0x6fdc10` | Grid::Bits6FDC10 | In outline: the same (another copy) |
| `0x7caf00` | Person::Update7CAF00 | In outline: an update when not flagged (+0x334) |
| `0x79f810` | Terrain::Part79F810 | In outline: a step with a part (+0x510) |
| `0x7500d0` | Guest::Pass7500D0 | In outline: with a part (+0xE4), goes over entries |
| `0x7035d0` | Grid::Terrain7035D0 | In outline: a step when the terrain is not flagged |
| `0x965b10` | Water::Find965B10 | In outline: a find over a list (+0x20) |
| `0x8536c0` | Scenery::Step8536C0 | In outline: a step with a record |
| `0x7a91b0` | Terrain::Handle7A91B0 | In outline: when given, makes a handle and links it |
| `0x7a6a40` | Terrain::Count7A6A40 | In outline: a count |
| `0x72caf0` | Path::Part72CAF0 | In outline: a step with a part (+0xAC) |
| `0x8ea590` | Terrain::Entry8EA590 | In outline: a step for entry n (0x130 bytes) of an object (+0x118) |
| `0x8765a0` | Scenery::Pass8765A0 | In outline: goes over entries (+0x4C) |
| `0x826d70` | Scenery::Init826D70 | In outline: fills a record with handles from its arguments |
| `0x81cf00` | SceneryBehaviour::Find81CF00 | In outline: a find over entries (+0x68) |
| `0x81ce40` | SceneryBehaviour::Find81CE40 | In outline: the same (another copy) |
| `0x97a430` | Water::Make97A430 | In outline: makes a window part |
| `0x9782b0` | Water::Make9782B0 | In outline: the same (another copy) |
| `0x977e40` | Water::Make977E40 | In outline: the same (another copy) |
| `0x977210` | Water::Make977210 | In outline: the same (another copy) |
| `0x976ec0` | Water::Make976EC0 | In outline: the same (another copy) |
| `0x968000` | Water::Make968000 | In outline: the same (another copy) |
| `0x96ba80` | Water::Make96BA80 | In outline: the same (another copy) |
| `0x96b6d0` | Water::Make96B6D0 | In outline: the same (another copy) |
| `0x96b1d0` | Water::Make96B1D0 | In outline: the same (another copy) |
| `0x96afe0` | Water::Make96AFE0 | In outline: the same (another copy) |
| `0x967f20` | Water::Make967F20 | In outline: the same (another copy) |
| `0x79b340` | Toolbar::Make79B340 | In outline: makes a window part |
| `0x913360` | Water::Part913360 | In outline: with a part (+0x8C), tears it down |
| `0x8d9e30` | Terrain::Step8D9E30 | In outline: unless a shared flag is set (`0x1310E7C`), a step when on (+0x16) |
| `0x882ad0` | Scenery::Entry882AD0 | In outline: a step for entry n (0x4C bytes) |
| `0x871240` | Scenery::Make871240 | In outline: makes a 0xC-byte part |
| `0x843a60` | Scenery::Step843A60 | In outline: a step with many values |
| `0x784b20` | GuestGroup::Make784B20 | In outline: makes a 0xC-byte part |
| `0x703400` | Grid::Terrain703400 | In outline: a step when the terrain is not flagged |
| `0x846d20` | Scenery::Step846D20 | In outline: a step (`0x828560`) with work values |
| `0x83ff70` | Scenery::Step83FF70 | In outline: a step with a shared object (`0x13429A8`) and points |
| `0x7881b0` | GuestGroup::Part7881B0 | In outline: a step with a part's value (+0x19C +0xF8) |
| `0x7729f0` | Park::Find7729F0 | In outline: a find over a list (+0xAC) |
| `0x761260` | Guest::Handle761260 | In outline: makes a handle and links it |
| `0x71d6e0` | Path::Terrain71D6E0 | In outline: a step when the terrain is not flagged |
| `0x93c630` | Water::List93C630 | In outline: a step with a list (+0x14) |
| `0x842710` | Scenery::Make842710 | In outline: with no part (+0x124), makes a 0x270-byte one |
| `0x8152a0` | SceneryBehaviour::Check8152A0 | In outline: a check when not flagged (+0x18) |
| `0x761a80` | Guest::Check761A80 | In outline: a check when not flagged (+0x530) |
| `0x71ba90` | Path::Step71BA90 | In outline: a step with a value (+0xA4) and slot 0x30 |
| `0x8ea090` | Terrain::Remove8EA090 | In outline: removes from a map (IdMap::Remove5 on +0x158) |
| `0x77f960` | Park::Add77F960 | In outline: grows a list by 8 when full (Array::Resize5501E0) and adds |
| `0x71b010` | Path::Terrain71B010 | In outline: a step when the terrain is not flagged |
| `0x703320` | Grid::Terrain703320 | In outline: the same (another copy) |
| `0x936ee0` | Water::Part936EE0 | In outline: a step with a part (+0x5C) |
| `0x7cf660` | Person::Mode7CF660 | In outline: unless in mode 6 (+0x350), a step when given |
| `0x784700` | GuestGroup::Find784700 | In outline: a find over a list (+0x44) |
| `0x94eeb0` | Water::Entry94EEB0 | In outline: when asked, a step for entry n (0x60 bytes) |
| `0x7722b0` | Park::Handle7722B0 | In outline: makes a handle and links it |
| `0x970a90` | Water::Entry970A90 | In outline: a value for entry n (12 bytes) |
| `0x91bf90` | Waterfall::Step91BF90 | In outline: a terrain step with values |
| `0x8da8d0` | Terrain::Point8DA8D0 | In outline: a step with a point |
| `0x84f680` | Scenery::Make84F680 | In outline: with no part, makes a 0x1008-byte one |
| `0x7f76b0` | Person::Handle7F76B0 | In outline: makes a handle and links it |
| `0x739540` | Guest::Make739540 | In outline: makes a 0x180-byte part |
| `0x731220` | Path::Make731220 | In outline: makes a 0xBF8-byte part |
| `0x723b70` | Path::Byte723B70 | In outline: a step by a byte (+0x8B) |
| `0x714d90` | Path::Set714D90 | In outline: sets a value (+0x160) from a byte and acts |
| `0x92ee40` | Water::Check92EE40 | In outline: a check when not flagged (+0x170) |
| `0x8c6760` | Scenery::Step8C6760 | In outline: a step with many values |
| `0x87c460` | Scenery::Terrain87C460 | In outline: for n not negative, a terrain step |
| `0x85b1a0` | Scenery::Bits85B1A0 | In outline: a step by bits |
| `0x82fd40` | Scenery::SetPoint82FD40 | In outline: sets a point (+0x120 on) when it changes |
| `0x7c61d0` | Person::Step7C61D0 | In outline: a step with values |
| `0x754540` | Guest::Shared754540 | In outline: once (`0x13064FC`), makes a 0x2C-byte record |
| `0x7034f0` | Grid::Terrain7034F0 | In outline: a step when the terrain is not flagged |
| `0x9d13f0` | Particles::Pass9D13F0 | In outline: goes over entries (+0x78) |
| `0x836100` | Scenery::Make836100 | In outline: makes a 0xC0-byte part |
| `0x79aad0` | Toolbar::Handle79AAD0 | In outline: makes a handle and links it |
| `0x8f3340` | Terrain::Part8F3340 | In outline: a step with a part (+0x2C, slots 0x14, 8) |
| `0x837ad0` | Scenery::Init837AD0 | In outline: clears a value (+4) and sets another to 0x20000 (+8) |
| `0x81a760` | SceneryBehaviour::Kind81A760 | In outline: sets a kind (+0x70) and a value from a shared table (`0x10F0718`) |
| `0x753180` | Guest::Part753180 | In outline: with a part (+0x14), a step |
| `0x938cd0` | Water::Cell938CD0 | In outline: a value for a cell (row times a width plus column) |
| `0x87ed50` | Scenery::Parse87ED50 | In outline: goes over a C string |
| `0x9978a0` | Fireworks::Step9978A0 | In outline: a step (`0x997450`) and acts with a shared value (`0x10F0CE8`) |
| `0x73e600` | Park::Pass73E600 | In outline: goes over entries (+0x518) |
| `0x840e60` | Park::Pass840E60 | In outline: goes over entries (+0x14) |
| `0x820aa0` | Park::Handle820AA0 | In outline: makes a handle and links it |
| `0x8e5910` | Park::Terrain8E5910 | In outline: a step when the terrain is not flagged |
| `0x748f50` | Park::Make748F50 | In outline: makes a 0x268-byte part |
| `0x8427c0` | Park::Make8427C0 | In outline: makes a 0x270-byte part |
| `0x71d9d0` | Park::Pass71D9D0 | In outline: goes over entries (+0x70) |
| `0x711f90` | Park::Handle711F90 | In outline: makes a handle and links it |
| `0x9a9af0` | Park::Make9A9AF0 | In outline: makes a 0x268-byte part |
| `0x9e0e80` | Park::Terrain9E0E80 | In outline: a step when the terrain is not flagged |
| `0x98c880` | Park::Pass98C880 | In outline: goes over entries (+0x48) |
| `0x844760` | Park::Pass844760 | In outline: goes over entries (+0x28) |
| `0x847040` | Scenery::Turn847040 | In outline: turns a cell (Scenery::Turn827530) and acts with points |
| `0x7290f0` | Path::Check7290F0 | In outline: a check with the path manager |
| `0x938670` | Water::Find938670 | In outline: a find over entries (+0x3C) |
| `0x925eb0` | Water::Part925EB0 | In outline: a step with a part (+0x328) |
| `0x8d3740` | Terrain::Part8D3740 | In outline: with a part (+0x80), a step with a value (+0x54) |
| `0x8b6570` | Scenery::Point8B6570 | In outline: a step with an object's point (+0x80, +0x84) |
| `0x8bfa60` | Scenery::Pass8BFA60 | In outline: goes over entries (+0x7C) |
| `0x87ee10` | Scenery::Read87EE10 | In outline: -1 when a read would run past the end, else reads |
| `0x714ce0` | Path::Copy714CE0 | In outline: copies a byte record from another |
| `0x9733f0` | Water::Step9733F0 | In outline: a step with many values |
| `0x863b80` | Scenery::Copy863B80 | In outline: copies a value and a byte from another |
| `0x8246e0` | SceneryBehaviour::Owner8246E0 | In outline: a step with its owner (SceneryBehaviour::Owner) when a shared flag is set (`0x130B564`) |
| `0x791420` | GuestGroup::Value791420 | In outline: with a part (+0x34), a value from another (+0xC, slot 0x34) |
| `0x7340b0` | Path::Free7340B0 | In outline: tears down references (ObjectRef) and more |
| `0x71d3d0` | Path::Joins71D3D0 | In outline: whether two path pieces join (bytes +0x8D, values +0xA4) |
| `0x9fb460` | Fireworks::Entry9FB460 | In outline: a step for entry n (0x24 bytes) that is set |
| `0x9ea5d0` | Fireworks::Init9EA5D0 | In outline: sets a value to 3.0 (+0x114) and acts with a shared flag (`0x1341C40`) |
| `0x9a9bd0` | Fireworks::Point9A9BD0 | In outline: a step with a point |
| `0x98ae10` | Record::Shared98AE10 | In outline: a step with a shared value (`0x10F0CE8`) |
| `0x7cc770` | Person::Step7CC770 | In outline: a step with a check (slot 0xF4) and a byte |
| `0x7cab60` | Person::Slot7CAB60 | In outline: with a slot (+0x194), a step (`0x7E39F0`) |
| `0x754b40` | Guest::Case754B40 | In outline: a value by case (+0x414) |
| `0x7135a0` | Path::Point7135A0 | In outline: a step with a point |
| `0x8a0660` | Scenery::Bit8A0660 | In outline: for an entry flagged (bit 0 of +0x3B0), a step |
| `0x89cf30` | Scenery::Bit89CF30 | In outline: the same (another copy) |
| `0x80dca0` | Regions::Cell80DCA0 | In outline: a cell of a 32x32x32 grid, out of range handled |
| `0x7a1fd0` | Terrain::Init7A1FD0 | In outline: fills a record with 0x20 and its arguments |
| `0x745730` | Guest::Entry745730 | In outline: clears entry n (16 bytes at +0xD94) |
| `0x714f30` | Path::Part714F30 | In outline: a step with a part (+0x214) |
| `0x9d7150` | Particles::List9D7150 | In outline: a step with a list (+8, +0xC) |
| `0x9c8e10` | Particles::Count9C8E10 | In outline: -1 with no entries (+0x14C), else a step |
| `0x9654f0` | Water::Next9654F0 | In outline: the next entry while under a count (+0x28, +0x2C) |
| `0x965440` | Water::Next965440 | In outline: the same (+0x1C, +0x20) |
| `0x8e83b0` | Terrain::SetEntry8E83B0 | In outline: sets a value of entry n (0x130 bytes) of +0x118 |
| `0x805a00` | Regions::Pass805A00 | In outline: goes over entries (Thunk) |
| `0x7e5520` | Person::Init7E5520 | In outline: fills a record from its arguments |
| `0x7195c0` | Path::Part7195C0 | In outline: a step with a part (+0x174) |
| `0x925bc0` | Water::Manager925BC0 | In outline: a step with the water manager (WaterManager::Get) |
| `0x7ffb20` | Person::Check7FFB20 | In outline: a check with a work list |
| `0x7ee3b0` | Person::Width7EE3B0 | In outline: a step when a width (+0x80, +0x88) lies between two limits |
| `0x7b66b0` | Terrain::Add7B66B0 | In outline: adds to a slot pool (SlotPool::Add) |
| `0x783b30` | GuestGroup::Check783B30 | In outline: 1 for none, else a check |
| `0x9f9e90` | Fireworks::Find9F9E90 | In outline: a find over entries (+0x30, +0x3C) |
| `0x9c7130` | Particles::Find9C7130 | In outline: a find |
| `0x942280` | Water::Pass942280 | In outline: goes over entries (+0x174) |
| `0x7fbb50` | Person::Read7FBB50 | In outline: reads eight bytes when there is room |
| `0x91d7d0` | Water::Step91D7D0 | In outline: a step with many values |
| `0x8e8140` | Terrain::Entry8E8140 | In outline: values of entry n (0x130 bytes) of +0x118 |
| `0x8e5570` | Terrain::Step8E5570 | In outline: a step with an object's values (+0xF8, +0xFC) |
| `0x84a580` | Scenery::Value84A580 | In outline: a value when not flagged (+0x6C) |
| `0x7ee540` | Person::Info7EE540 | In outline: a step with an info text (Info::Text7C8700) |
| `0x988910` | Record::Set988910 | In outline: sets a flag (+0x40) and acts with a part |
| `0x8d3410` | Terrain::Buffer8D3410 | In outline: takes a resource (Resource::RefBFFB60) and locks n times 6 bytes |
| `0x786ed0` | GuestGroup::Shared786ED0 | In outline: a step when two shared values differ (`0x130697C`, `0x1306988`) |
| `0x7f9f40` | Person::Check7F9F40 | In outline: a step when a check passes (`0x7DF1F0`) |
| `0x916580` | Water::Shared916580 | In outline: a step with a shared object (`0x13429A8`) |
| `0x89c9d0` | Scenery::Step89C9D0 | In outline: a step (`0x8915F0`) with many values |
| `0x876020` | Scenery::Value876020 | In outline: a value from a part (+0x10, +0x78) |
| `0x7bd390` | Terrain::List7BD390 | In outline: a step with a list (+8, +0xC) |
| `0x759c50` | Guest::State759C50 | In outline: in state 4, 0 or 0x100 (+0x190), a step |
| `0x71c670` | Path::Value71C670 | In outline: a value from bytes and a shifted float |
| `0x9c6860` | Particles::Reset9C6860 | In outline: resets a scroller (Scroller::Reset) and more |
| `0x97afa0` | Park::Pass97AFA0 | In outline: goes over entries (+0x50) |
| `0x9ac770` | Park::Make9AC770 | In outline: makes a 0x114-byte part |
| `0x93c590` | Park::Make93C590 | In outline: makes a 0xC-byte part |
| `0x9eee10` | Park::Make9EEE10 | In outline: makes a 0x80-byte part |
| `0x80e420` | Park::Make80E420 | In outline: makes a 0x2C-byte part |
| `0x85f820` | Park::Terrain85F820 | In outline: a step when the terrain is not flagged |
| `0x851bd0` | Park::Make851BD0 | In outline: makes a 0xC10-byte part |
| `0x92c520` | Park::Make92C520 | In outline: makes a 0xC-byte part |
| `0x8c8ba0` | Park::Make8C8BA0 | In outline: makes a 0xF0-byte part |
| `0x8b3db0` | Park::Make8B3DB0 | In outline: makes a 0xC-byte part |
| `0x9c1710` | Park::Pass9C1710 | In outline: goes over entries (+0x58) |
| `0x8f33f0` | Park::Pass8F33F0 | In outline: goes over entries (+0x10) |
| `0x8b04c0` | Park::Make8B04C0 | In outline: makes a 0x268-byte part |
| `0x8762f0` | Park::Make8762F0 | In outline: makes a 0xC-byte part |
| `0x70ecc0` | Park::Make70ECC0 | In outline: makes a 0x14-byte part |
| `0x9fabc0` | Fireworks::Find9FABC0 | In outline: a find in a list |
| `0x9d7fc0` | Particles::Find9D7FC0 | In outline: the same (another copy) |
| `0x9d58d0` | Particles::Check9D58D0 | In outline: with an object that has a part (+0xCC), a step (`0x9CBA80`) |
| `0x9d5830` | Particles::Check9D5830 | In outline: with an object, a check (Fireworks::Check9C5850) |
| `0x9bc3f0` | Window::Limit9BC3F0 | In outline: keeps a value at most a part's value (+0x44 +0x7C) |
| `0x991a50` | Fireworks::Find991A50 | In outline: a find in a list |
| `0x9459f0` | Water::Pass9459F0 | In outline: goes over entries |
| `0x89cfe0` | Scenery::Bit89CFE0 | In outline: for an entry not flagged (bit 0 of +0x3B0), a step |
| `0x86ee40` | Scenery::List86EE40 | In outline: a step with a list (+0x130) |
| `0x7bca80` | Terrain::Entry7BCA80 | In outline: for an object's slot (+0x54), a step with entry n (0x24 bytes) |
| `0x71d480` | Path::Same71D480 | In outline: whether two path pieces match (bytes +0x8D, values +0xA4) |
| `0x71d330` | Path::Same71D330 | In outline: the same (another copy) |
| `0x9e30f0` | Fireworks::Find9E30F0 | In outline: a find in a list |
| `0x982fe0` | Record::Add982FE0 | In outline: adds an entry, growing when full |
| `0x8ff9c0` | Terrain::Entry8FF9C0 | In outline: for nonzero n, a step with entry n (0x14 bytes) |
| `0x8552b0` | Scenery::Entry8552B0 | In outline: the same (another copy) |
| `0x8018e0` | Person::Read8018E0 | In outline: reads eight bytes when there is room |
| `0x9e1ec0` | Fireworks::Find9E1EC0 | In outline: a find in a list |
| `0x991f50` | Fireworks::Find991F50 | In outline: the same (another copy) |
| `0x976940` | Water::Set976940 | In outline: a step when two values differ |
| `0x8e0340` | Terrain::Step8E0340 | In outline: a terrain step (Terrain::Step8DFC30) with a value |
| `0x8c5150` | Scenery::Init8C5150 | In outline: clears a record |
| `0x8588d0` | Scenery::Pass8588D0 | In outline: goes over entries |
| `0x805ec0` | Regions::Add805EC0 | In outline: adds an entry, growing when full |
| `0x802900` | Person::Add802900 | In outline: the same (another copy) |
| `0x7fbe30` | Person::Add7FBE30 | In outline: the same (another copy) |
| `0x7e98a0` | Person::Kind7E98A0 | In outline: unless of kind 0xD (+0x190), a step with a shared value (`0x12F8FC0`) |
| `0x783da0` | GuestGroup::Part783DA0 | In outline: a step with a part (+0xE8) |
| `0x95a660` | Water::Remove95A660 | In outline: removes (Water::Remove95A570) and acts with a count (+0x10) |
| `0x942510` | Water::Find942510 | In outline: a find in a list |
| `0x7cac10` | Person::Part7CAC10 | In outline: a step with a part (+0x35C) |
| `0x7598b0` | Guest::Pass7598B0 | In outline: goes over entries |
| `0x758dc0` | Guest::Shared758DC0 | In outline: when on (+0x444), a step with a shared value (`0x1306194`) |
| `0x8aed60` | Scenery::Init8AED60 | In outline: clears a record |
| `0x8d6180` | Terrain::Part8D6180 | In outline: a step with a value (+0x54) |
| `0x882ce0` | Ref::Assign882CE0 | In outline: takes a reference to another's part when it differs |
| `0x838b60` | Scenery::Check838B60 | In outline: a check with a part (+0x16C) |
| `0x838ac0` | Scenery::Check838AC0 | In outline: the same (another copy) |
| `0x725d40` | Path::Init725D40 | In outline: clears a value and sets up a handle |
| `0x9f5bb0` | Fireworks::List9F5BB0 | In outline: a step with a list (+0x4C) |
| `0x9199b0` | Waterfall::Copy9199B0 | In outline: copies two values and acts |
| `0x8f49a0` | Terrain::Value8F49A0 | In outline: a value with many values |
| `0x8b3a50` | Scenery::Free8B3A50 | In outline: with a slot (+0x60), frees it (SlotPool::Remove) |
| `0x786ad0` | GuestGroup::Owner786AD0 | In outline: a step with its owner's value (SceneryBehaviour::Owner +0x120) |
| `0x720f70` | Path::Case720F70 | In outline: a value by an object's kind (slot 0x30) |
| `0x716b20` | Path::Kind716B20 | In outline: unless an object is of kind 3 (slot 0x30), a step |
| `0x91c040` | Water::Pass91C040 | In outline: goes over entries (+0x138) |
| `0x8b2170` | Scenery::Check8B2170 | In outline: 0 unless an object is on (+0x544) |
| `0x887bf0` | Scenery::Part887BF0 | In outline: with a part (+0x114 +0x158), a step (`0x8277A0`) |
| `0x808150` | Regions::Step808150 | In outline: a vector step (Vector::StepBEE5B0) and more |
| `0x7ab580` | Terrain::Bits7AB580 | In outline: a step with a bit table (+0x50C) |
| `0x717f90` | Path::Step717F90 | In outline: a step (`0x717E30`) with the path manager |
| `0x9faa40` | Fireworks::Entry9FAA40 | In outline: a step for entry n (0x90 bytes) |
| `0x9a0af0` | Fireworks::Path9A0AF0 | In outline: when not asked, a firework file path (Fireworks::FilePath) |
| `0x982c70` | Record::Count982C70 | In outline: 0 for n under 1, else a step |
| `0x976bd0` | Water::Device976BD0 | In outline: a device step when two values differ |
| `0x8fd870` | Terrain::Next8FD870 | In outline: the next entry while under a count (+0x4C, +0x50) |
| `0x8da110` | Terrain::Init8DA110 | In outline: sets a value to the largest float and a handle |
| `0x87b480` | Scenery::Step87B480 | In outline: a step with a record |
| `0x860550` | Scenery::Bits860550 | In outline: goes over a bit table (+0xC) |
| `0x82fec0` | Scenery::Pass82FEC0 | In outline: goes over a part's entries (+0x34 +0x70) |
| `0x74ddc0` | Guest::Add74DDC0 | In outline: grows a list by 16 when full (Array::Resize5501E0) and adds |
| `0x9bc890` | Window::Part9BC890 | In outline: with a part (+0x13C +0xCC), a step |
| `0x89d0f0` | Scenery::Bit89D0F0 | In outline: for an entry flagged (bit 0 of +0x3B0), a step |
| `0x877ff0` | Scenery::Free877FF0 | In outline: with a part (+0x50), frees it |
| `0x717eb0` | Path::Point717EB0 | In outline: sets a value (+0xEC) from a point (`0x682D80`) |
| `0x929b20` | Water::List929B20 | In outline: a step with a list (+0x6C) |
| `0x912340` | Water::Step912340 | In outline: a step with values (+0x38, +0x48) |
| `0x9122a0` | Water::Step9122A0 | In outline: the same (+0x34, +0x44) |
| `0x876250` | Scenery::Loop876250 | In outline: goes over a list (+4) |
| `0x73ac30` | Guest::Add73AC30 | In outline: grows a list by 16 when full (Array::Resize5501E0) and adds |
| `0x732270` | Path::Add732270 | In outline: the same (another copy) |
| `0x82e860` | Scenery::List82E860 | In outline: a step with a list (+8, +0xC) |
| `0x70c2a0` | Grid::Part70C2A0 | In outline: a step with a part (+0x1D8) |
| `0x85f8c0` | Park::Terrain85F8C0 | In outline: a step when the terrain is not flagged |
| `0x73cb50` | Park::Terrain73CB50 | In outline: a step when the terrain is not flagged |
| `0x9238e0` | Park::Pass9238E0 | In outline: goes over entries (+0x120) |
| `0x782430` | Park::Pass782430 | In outline: goes over entries (+0x48) |
| `0x733920` | Park::Pass733920 | In outline: goes over entries (+0x14) |
| `0x80dad0` | Park::Make80DAD0 | In outline: makes a 0x18-byte part |
| `0x7511a0` | Park::Copy7511A0 | In outline: copies values from another record |
| `0x98c7f0` | Park::Pass98C7F0 | In outline: goes over entries (+0x48) |
| `0x955970` | Park::Pass955970 | In outline: goes over entries (+0x15C) |
| `0x995db0` | Park::Pass995DB0 | In outline: goes over entries (+0x194) |
| `0x7739d0` | Park::Pass7739D0 | In outline: goes over entries (+0x48) |
| `0x762980` | Park::List762980 | In outline: a step with a list (+8, +0xC) |
| `0x71f220` | Park::List71F220 | In outline: a step with a list (+8, +0xC) |
| `0x882660` | Park::Reset882660 | In outline: resets (Static::Reset) and more |
| `0x8362d0` | Park::Update8362D0 | In outline: an update when not flagged (+0x194) |
| `0x973580` | Water::Step973580 | In outline: a step with many values |
| `0x90c2a0` | Water::Step90C2A0 | In outline: a step with a record |
| `0x8e0cd0` | Terrain::Part8E0CD0 | In outline: with two parts (+0x13C, +0x12C), a terrain step |
| `0x7e3960` | Person::Init7E3960 | In outline: clears values (+0x68 on) |
| `0x7724a0` | Park::Init7724A0 | In outline: fills a record with 0xF0 and zeros |
| `0x9c0210` | Window::Reset9C0210 | In outline: a step with children (Window::ChildrenSlot4) and clears a value (Window::Set9BB8E0) |
| `0x9a5630` | Fireworks::Step9A5630 | In outline: a step with an object's value (+4 +0x194) |
| `0x984870` | Record::Kind984870 | In outline: unless of kind 0xC (+0xA0), a step |
| `0x860db0` | Scenery::Set860DB0 | In outline: sets a size when it changes |
| `0x798b50` | Terrain::Part798B50 | In outline: a step with a part (+0x1C4) |
| `0x773510` | Park::Set773510 | In outline: sets a size when it changes |
| `0x7250a0` | Path::Set7250A0 | In outline: the same (another copy) |
| `0x9b0800` | Fireworks::Load9B0800 | In outline: while under a count (+0x10, +0x14), loads a file (ManagedObject::LoadFile) |
| `0x8e32e0` | Terrain::Assign8E32E0 | In outline: frees its buffer and takes another's values |
| `0x8b5510` | Scenery::Point8B5510 | In outline: a step with a point |
| `0x83f870` | Scenery::Set83F870 | In outline: for none, clears (Scenery::Set83F800) |
| `0x7c3ec0` | Person::Point7C3EC0 | In outline: a step with a point (+0x80, +0x84) |
| `0x741ed0` | Guest::Compare741ED0 | In outline: compares two-byte texts |
| `0x8fd690` | Terrain::Slot8FD690 | In outline: with no slot (+0xA4 = -1), a step |
| `0x7faa80` | Person::Make7FAA80 | In outline: makes a part |
| `0x9f5ae0` | Fireworks::List9F5AE0 | In outline: a step with a list (+0x4C) |
| `0x9eafa0` | Fireworks::Random9EAFA0 | In outline: when a time (+4) is negative, a random step |
| `0x9828e0` | Record::Make9828E0 | In outline: with no part (+0x54), makes one |
| `0x835280` | Scenery::Part835280 | In outline: a step with a part (+0x48) |
| `0x7f2e20` | Person::Case7F2E20 | In outline: a step by case (+0x13C) |
| `0x9f4480` | Fireworks::Shared9F4480 | In outline: with no shared object (`0x13429A8`), a step |
| `0x985de0` | Record::Pass985DE0 | In outline: goes over entries (+0x54) |
| `0x929670` | Water::List929670 | In outline: a step with a list (+0x6C) |
| `0x8273c0` | Scenery::Count8273C0 | In outline: for a positive count, a step |
| `0x945960` | Water::Pass945960 | In outline: goes over entries |
| `0x912de0` | Water::Shared912DE0 | In outline: a value with a shared object (`0x13429A8`), none handled |
| `0x8be480` | Scenery::Buffer8BE480 | In outline: takes a resource (Resource::RefBFFB60) and locks n times m times 6 bytes |
| `0x7cacb0` | Person::Mode7CACB0 | In outline: sets a mode to 4 (+0x350) and acts with a part (+0x35C) |
| `0x758500` | Guest::Shared758500 | In outline: for a negative slot, a step with a shared value (`0x1305DD8`) |
| `0x978810` | Water::Count978810 | In outline: for a positive count, a step |
| `0x949120` | Water::Step949120 | In outline: a step with a value (+0xE4) |
| `0x949090` | Water::Step949090 | In outline: the same (+0xE0) |
| `0x91c290` | Waterfall::Manager91C290 | In outline: a step with the waterfall manager's list (+0x68) |
| `0x8553b0` | Scenery::Entry8553B0 | In outline: a step for entry n (12 bytes) |
| `0x7b5100` | Effect::Ring7B5100 | In outline: a step with a ring buffer's entry |
| `0x7b3a50` | Effect::InRange7B3A50 | In outline: a step when an object's value (+0x80) lies in a range |
| `0x74e4b0` | Guest::Entry74E4B0 | In outline: with a count (+0xC), a step with entry n (0x14 bytes) |
| `0x905140` | Water::Pass905140 | In outline: goes over entries (+4) |
| `0x872180` | Scenery::Part872180 | In outline: a step with a part (+0x68) |
| `0x847340` | Scenery::Check847340 | In outline: a check with a part (slot 0x94) |
| `0x8098f0` | Regions::Part8098F0 | In outline: a step with a part (+0x20) |
| `0x742dc0` | Guest::Id742DC0 | In outline: takes an id (Ids::Allocate742780) |
| `0x9ed9c0` | Fireworks::Part9ED9C0 | In outline: a step with a part (+0x40) |
| `0x9c2520` | Window::Width9C2520 | In outline: a step with a width and a part's entry |
| `0x99bde0` | Fireworks::Assign99BDE0 | In outline: frees its buffer and remakes it from another |
| `0x9136d0` | Water::Value9136D0 | In outline: a step with a value (`0xDDEBC0`) |
| `0x84ba90` | Scenery::Update84BA90 | In outline: an update (Scenery::Update84A3A0) and acts with a slot (+0xA4) |
| `0x8131f0` | SceneryBehaviour::Entry8131F0 | In outline: a step for entry n (16 bytes) |
| `0x7b06c0` | Effect::Value7B06C0 | In outline: a step with a value (Effect::Value7B05E0) |
| `0x733560` | Path::Assign733560 | In outline: frees its buffer and remakes it from another |
| `0x711860` | Path::Step711860 | In outline: a step (`0x714340`) and more with a shared value (`0x10EDCB0`) |
| `0x967360` | Water::Assign967360 | In outline: frees its buffer and remakes it from another |
| `0x92bf40` | Water::Count92BF40 | In outline: adds one to a count (+0x364) and acts with a list (+0x6C) |
| `0x91c9c0` | Water::Part91C9C0 | In outline: with a part (+0x10), a step |
| `0x914010` | Water::Grow914010 | In outline: for none, grows the last entry (Water::Grow913E10) |
| `0x8e3590` | Terrain::Init8E3590 | In outline: sets up a record with its table (`0xF7C9DC`) |
| `0x8915f0` | Scenery::Step8915F0 | In outline: a step with many values |
| `0x7b4e80` | Effect::Release7B4E80 | In outline: takes one from a count (+8) and frees at zero |
| `0x738540` | Guest::Pass738540 | In outline: goes over entries |
| `0x730aa0` | Path::Pass730AA0 | In outline: the same (another copy) |
| `0x705170` | Grid::Part705170 | In outline: a step with a part (+0x19C) |
| `0x980400` | Record::Assign980400 | In outline: frees its buffer and remakes it from another |
| `0x9e1a20` | Fireworks::Assign9E1A20 | In outline: the same (another copy) |
| `0x9d1120` | Particles::Assign9D1120 | In outline: the same (another copy) |
| `0x94cd80` | Water::Step94CD80 | In outline: a step with a work buffer |
| `0x8e3250` | Terrain::Assign8E3250 | In outline: frees its buffer and remakes it from another |
| `0x8e13f0` | Terrain::Reset8E13F0 | In outline: clears a flag (+0x70) and acts with a part (+0x80) |
| `0x8bffa0` | Scenery::Entry8BFFA0 | In outline: for n under a count, a step |
| `0x856440` | Scenery::Part856440 | In outline: with no part (+0x6C), a step with a value (+0x14) |
| `0x7e5350` | Person::Assign7E5350 | In outline: frees its buffer and remakes it from another |
| `0x7e51f0` | Person::Size7E51F0 | In outline: grows when a size (bytes +4, +5) passes a count (+0xC) |
| `0x74ebb0` | Guest::Init74EBB0 | In outline: sets up a record with its table (`0xF6BD38`) |
| `0x9838d0` | Record::Pass9838D0 | In outline: goes over entries (+0x24) |
| `0x8561b0` | Scenery::Water8561B0 | In outline: a step with the water manager |
| `0x802bb0` | Person::Count802BB0 | In outline: for a positive count, a step |
| `0x7cad40` | Person::Mode7CAD40 | In outline: sets a mode to 3 (+0x350) and acts with a part (+0x35C) |
| `0x81ab50` | Park::Make81AB50 | In outline: makes a 0x10-byte part |
| `0x774340` | Park::Pass774340 | In outline: goes over entries (+0x48) |
| `0x79ae90` | Park::Make79AE90 | In outline: sets up a window part |
| `0x91c210` | Park::Terrain91C210 | In outline: a step when the terrain is not flagged |
| `0x77a530` | Park::List77A530 | In outline: a step with a list (+8, +0xC) |
| `0x749cf0` | Park::List749CF0 | In outline: a step with a list (+8, +0xC) |
| `0x7742b0` | Park::Pass7742B0 | In outline: goes over entries (+0x48) |
| `0x73a5e0` | Park::Make73A5E0 | In outline: makes a 0x80-byte part |
| `0x881ee0` | Park::Make881EE0 | In outline: makes a 0x4C-byte part |
| `0x7b3d90` | Park::Terrain7B3D90 | In outline: a step when the terrain is not flagged |
| `0x9287c0` | Park::Pass9287C0 | In outline: goes over entries (+0x370) |
| `0x882b90` | Park::Init882B90 | In outline: sets up a record with empty texts (WideString::Empty) |
| `0x808760` | Park::Pass808760 | In outline: goes over entries (+0x14) |
| `0x876190` | Park::List876190 | In outline: a step with a list (+8, +0xC) |
| `0x8760c0` | Park::List8760C0 | In outline: a step with a list (+8, +0xC) |
| `0x7a3030` | Park::List7A3030 | In outline: a step with a list (+8, +0xC) |
| `0x939e40` | Park::Pass939E40 | In outline: goes over entries (+0x14) |
| `0x840860` | Park::Pass840860 | In outline: goes over entries (+0x58) |
| `0x9d0b40` | Park::Pass9D0B40 | In outline: goes over entries (+0x218) |
| `0x9d0ac0` | Park::Pass9D0AC0 | In outline: goes over entries (+0x218) |
| `0x9a3140` | Fireworks::Entry9A3140 | In outline: a step for entry n (0x20 bytes) |
| `0x8b15a0` | Scenery::Find8B15A0 | In outline: a find over entries (+0x7C) |
| `0x860430` | Scenery::Slot860430 | In outline: for a slot not negative, a step with a part (+0xC) |
| `0x855490` | Scenery::Entry855490 | In outline: a step for entry n (12 bytes) |
| `0x7336e0` | Path::Assign7336E0 | In outline: frees its buffer and takes another's values |
| `0x702310` | Grid::Clear702310 | In outline: with a value (+0x124), clears it and acts |
| `0x9f0220` | Fireworks::Time9F0220 | In outline: with a part (+0x15C), a step when a time (+0x160) passes a limit |
| `0x7847b0` | GuestGroup::List7847B0 | In outline: a step with a list (+0x44) |
| `0x754e50` | Guest::Next754E50 | In outline: a step when n differs from a value (+0x50C), the next one handled |
| `0x92e690` | Water::Assign92E690 | In outline: frees its buffer and takes another's values |
| `0x92bec0` | Water::Count92BEC0 | In outline: adds one to a count (+0x364) and acts with a list (+0x6C) |
| `0x8029a0` | Person::Pass8029A0 | In outline: goes over entries |
| `0x787240` | GuestGroup::Layout787240 | In outline: lays out (Window::Layout643030) and acts with a point |
| `0x774dc0` | Park::Pass774DC0 | In outline: goes over entries (+0x48) |
| `0x74de60` | Guest::Set74DE60 | In outline: with a count (+0xC), sets a value (+0x10) |
| `0x9d0c60` | Particles::Set9D0C60 | In outline: sets a value (+8) and clears a flag |
| `0x9c8ff0` | Particles::Entry9C8FF0 | In outline: a value from an entry (+0x28 plus n) |
| `0x97cb00` | Water::List97CB00 | In outline: a step with a list (+0x16C) |
| `0x89ed50` | SceneryRenderer::Billboard89ED50 | In outline: a pass (SceneryRenderer::Pass89E620) and makes a billboard (Billboard::Create) |
| `0x840740` | Scenery::Value840740 | In outline: a value with a fixed start (`0xF264E0`) |
| `0x76e140` | Park::Check76E140 | In outline: a check with the largest float |
| `0x741ff0` | Guest::Part741FF0 | In outline: a step with a part (+0x4C) |
| `0x714120` | Path::Reset714120 | In outline: clears a flag and acts with a part (+0x18) |
| `0x942320` | Water::Grow942320 | In outline: grows when n reaches a count (+0x164) |
| `0x90a0d0` | Water::Entry90A0D0 | In outline: for nonzero n, a step with entry n (0x14 bytes) |
| `0x909dc0` | Water::Entry909DC0 | In outline: the same (another copy) |
| `0x87be60` | Scenery::Entry87BE60 | In outline: the same (another copy) |
| `0x86f6a0` | Scenery::Count86F6A0 | In outline: adds one to an object's count (+0xF8) and acts with a list (+0x40) |
| `0x80f990` | Regions::Entry80F990 | In outline: for nonzero n, a step with entry n (0x14 bytes) |
| `0x7bc090` | Terrain::Part7BC090 | In outline: with a part (+0x30), goes over entries |
| `0x783850` | GuestGroup::Find783850 | In outline: a find over entries (+0x60) |
| `0x749b60` | Guest::Next749B60 | In outline: the next entry of a list (+0x40) |
| `0x9f9b90` | Fireworks::Slot9F9B90 | In outline: for a negative slot (+0xC), a step |
| `0x973500` | Water::Step973500 | In outline: a step with many values |
| `0x860340` | Scenery::Pass860340 | In outline: clears a value and goes over entries |
| `0x92ed10` | Water::Init92ED10 | In outline: clears values (+4, +0x10, ...) |
| `0x8b24f0` | Scenery::Find8B24F0 | In outline: a find (Scenery::Find8B15A0) with a work buffer |
| `0x853780` | Scenery::Make853780 | In outline: makes a record (Park::Make851BD0) and sets a value (+0x24) |
| `0x81d6e0` | SceneryBehaviour::Entry81D6E0 | In outline: a step for entry n (0x60 bytes at +0x64 +0x40) |
| `0x7efc60` | Person::Format7EFC60 | In outline: formats into a 256-character buffer |
| `0x7e3640` | Person::Part7E3640 | In outline: with no value (+0xF4) and a part (+0xF0), a step |
| `0x7e2180` | Person::Pass7E2180 | In outline: goes over a part's entries (+0x158 +0x48) |
| `0x7595f0` | Guest::Shared7595F0 | In outline: with no shared value (`0x1306194`), a step |
| `0x759570` | Guest::Shared759570 | In outline: the same (another copy) |
| `0x7594e0` | Guest::Shared7594E0 | In outline: the same (another copy) |
| `0x9a9fd0` | Fireworks::Point9A9FD0 | In outline: a point from an object's values and its own |
| `0x996330` | Fireworks::Point996330 | In outline: a step with a point |
| `0x96a4f0` | Water::Pass96A4F0 | In outline: goes over entries (+0x30) |
| `0x9697e0` | Water::Pass9697E0 | In outline: the same with +0x20 |
| `0x90f6f0` | Water::Add90F6F0 | In outline: adds when there is room (+0x44, +0x48) |
| `0x90f610` | Water::Step90F610 | In outline: a step when two values differ (+4, +0x10) |
| `0x823180` | Scenery::Array823180 | In outline: makes an array of n entries of 12 bytes |
| `0x821480` | Scenery::Array821480 | In outline: makes an array of n entries of 0x28 bytes |
| `0x789390` | GuestGroup::Value789390 | In outline: a step when an object's value (slot 0x34) is at most a limit |
| `0x7752f0` | Park::Array7752F0 | In outline: makes an array of n entries of 0x14 bytes |
| `0x72f090` | Path::Kind72F090 | In outline: for an object of kind 3 (slot 0x30), a step |
| `0x711f10` | Path::Object711F10 | In outline: an object from a reference (Reference::ToObject) and more |
| `0x982860` | Record::Part982860 | In outline: with a part (+4), a step |
| `0x9466b0` | Water::Step9466B0 | In outline: a step with a work buffer |
| `0x92b960` | Water::List92B960 | In outline: a step with a list (+0x6C) |
| `0x8e2bd0` | Terrain::Current8E2BD0 | In outline: when on (+0x1C) and current (`0x12F56D4`), a step |
| `0x86eee0` | Scenery::List86EEE0 | In outline: with no part (+0xCC +0x5C), a list step (Scenery::List86EE40) |
| `0x8288a0` | Scenery::Flags8288A0 | Sets or clears the given flags (+0x7C) and acts |
| `0x8080d0` | Regions::Entry8080D0 | In outline: a step with an entry (+0x28 plus n) |
| `0x7c2740` | Person::Time7C2740 | In outline: adds n times a rate (+0x114) to a value (+0xBC) |
| `0x7a0860` | Terrain::Case7A0860 | In outline: a step by case (0, ...) |
| `0x785840` | GuestGroup::Find785840 | In outline: a find over entries (+0x60) |
| `0x7857c0` | GuestGroup::Find7857C0 | In outline: the same (another copy) |
| `0x774230` | Park::Pass774230 | In outline: goes over entries (+0x48) |
| `0x744e20` | Guest::Array744E20 | In outline: makes an array of n entries of 0x50 bytes |
| `0x71ded0` | Path::Part71DED0 | In outline: a step (Path::Part72CAF0) and a part (+0xB0) |
| `0x9c5fb0` | Particles::Make9C5FB0 | In outline: makes a part and acts with a list (+0x34) |
| `0x9846f0` | Record::Case9846F0 | In outline: a value by case |
| `0x85aca0` | Scenery::Pass85ACA0 | In outline: goes over entries |
| `0x79bef0` | Toolbar::Find79BEF0 | In outline: a find when two shared values differ (`0x1306BCC`, `0x1306BD8`) |
| `0x8125d0` | SceneryBehaviour::Step8125D0 | In outline: when a part is not flagged (+4 +0x24), a step |
| `0x7fb340` | Person::Step7FB340 | In outline: a step with an object's value (+4) |
| `0x785560` | GuestGroup::Find785560 | In outline: a find over entries (+0x60) |
| `0x7854e0` | GuestGroup::Find7854E0 | In outline: the same (another copy) |
| `0x98b440` | Record::Free98B440 | In outline: tears down references (ObjectRef) |
| `0x86f810` | Scenery::Set86F810 | In outline: sets a value (+4) and acts with a part (+0x10) |
| `0x80f610` | Regions::Compare80F610 | In outline: a step when a value (+0x1C) is at most another's |
| `0x717e30` | Path::Reset717E30 | In outline: clears values (+0x24C, +0x230, ...) |
| `0x713880` | Path::Shared713880 | In outline: with a shared value (`0x10EDCB8`) and no part (+0x2F0), a step |
| `0x9a98a0` | Fireworks::Point9A98A0 | In outline: a step with points |
| `0x801980` | Person::Read801980 | In outline: reads eight bytes when there is room |
| `0x7d6560` | Person::Case7D6560 | In outline: for kind 10 with a part (+0x308), a step |
| `0x9eeeb0` | Fireworks::Part9EEEB0 | In outline: a step (`0x9EEDB0`) and acts with a part (+0x124) |
| `0x9abbd0` | Fireworks::Split9ABBD0 | In outline: splits a text (Text::Split9A50A0) |
| `0x9851c0` | Record::Entry9851C0 | In outline: a value from an entry (+0x28 plus n) |
| `0x80a710` | Regions::Release80A710 | In outline: takes one from a count (+0xC) and frees at zero |
| `0x8002c0` | Person::Init8002C0 | In outline: sets up a record with its table (`0xF752D0`) |
| `0x7382c0` | Park::Reset7382C0 | In outline: resets (Static::Reset) and more |
| `0x8560e0` | Park::Pass8560E0 | In outline: goes over entries (+0x10) |
| `0x9839d0` | Park::Pass9839D0 | In outline: goes over entries (+0x24) |
| `0x878090` | Park::Make878090 | In outline: makes a 0x64-byte part |
| `0x787f80` | Park::Pass787F80 | In outline: goes over entries (+0x188) |
| `0x9a0540` | Park::Make9A0540 | In outline: makes a 0xE4-byte part |
| `0x829520` | Park::Pass829520 | In outline: goes over entries (+0x13C) |
| `0x7136d0` | Park::Make7136D0 | In outline: makes a 0x120-byte part |
| `0x8294a0` | Park::Pass8294A0 | In outline: goes over entries (+0x13C) |
| `0x7744b0` | Park::Pass7744B0 | In outline: goes over entries (+0x48) |
| `0x9d3ca0` | Park::Make9D3CA0 | In outline: makes a 0x14-byte part |
| `0x9cba80` | Park::Make9CBA80 | In outline: makes a 0x120-byte part |
| `0x8e8870` | Park::Terrain8E8870 | In outline: a step when the terrain is not flagged |
| `0x995560` | Park::Release995560 | In outline: takes one from a count (+0x8) and frees at zero |
| `0x827670` | Park::Make827670 | In outline: makes a 0x144-byte part |
| `0x9d12c0` | Park::Init9D12C0 | In outline: sets up a record with empty texts (WideString::Empty) |
| `0x8151c0` | Park::Update8151C0 | In outline: an update when not flagged (+0x18) |
| `0x7745b0` | Park::Pass7745B0 | In outline: goes over entries (+0x48) |
| `0x7140a0` | Path::Part7140A0 | In outline: with a part (+0x224), a value from it (+0x104) |
| `0x6fdcd0` | Grid::Clear6FDCD0 | In outline: empties a list (+0x4C) |
| `0x99b560` | Fireworks::Time99B560 | In outline: a step by a time, at most 1 handled |
| `0x95dc70` | Water::Rebuild95DC70 | In outline: when on (+0x34), rebuilds (Water::Rebuild95C490) |
| `0x9402f0` | Water::Shared9402F0 | In outline: a step by shared flags (`0x13D3184`, `0x1106DC2`, `0x1106C90`) |
| `0x86fd70` | Scenery::Empty86FD70 | In outline: empties a text and acts with a part (+0x130) |
| `0x865500` | Scenery::Clear865500 | In outline: empties a list (+0xF4) |
| `0x858300` | Scenery::Count858300 | In outline: with no entries (+0x50), a step |
| `0x808680` | Regions::List808680 | In outline: a step with a list (+8, +0xC) |
| `0x7e69c0` | Person::Step7E69C0 | In outline: a step with many values |
| `0x774e40` | Park::Pass774E40 | In outline: goes over entries (+0x48) |
| `0x72f400` | Path::Pass72F400 | In outline: with a part (+4), goes over entries |
| `0x9ec740` | Fireworks::Shared9EC740 | In outline: with a part (+0x1C), a step with a shared object (`0x13429A8`) |
| `0x99c070` | Fireworks::Slot99C070 | In outline: with a slot (+0x194), a step |
| `0x876380` | Scenery::List876380 | In outline: a step with a list (+0x1C) |
| `0x8268a0` | Scenery::Part8268A0 | In outline: a step with a part (+0xC4) |
| `0x807090` | Regions::Next807090 | In outline: the next entry while under a count (+4, +8) |
| `0x800a00` | Person::Part800A00 | In outline: a step with a part (slot 4) |
| `0x79aa50` | Toolbar::Time79AA50 | In outline: for kind 0x148 (+0xB8), a step when a time (+0xBC) is under a limit |
| `0x74e290` | Guest::Pass74E290 | In outline: goes over entries (+0xEEC) |
| `0x82fab0` | Scenery::Set82FAB0 | In outline: for 0, sets a value (+0xD4) when it changes |
| `0x82fa30` | Scenery::Set82FA30 | In outline: the same with +0xD0 |
| `0x82f9c0` | Scenery::Set82F9C0 | In outline: the same with +0xCC |
| `0x808000` | Regions::Entry808000 | In outline: a step with an entry (+0x28 plus n) |
| `0x807d20` | Regions::Last807D20 | In outline: a step with a tree's last value (SortedTree::LastValue) |
| `0x7ca730` | Person::Part7CA730 | In outline: a step with a part (+0x1F4) |
| `0x7c7d60` | Person::Part7C7D60 | In outline: a step with a part (+0x1E8) |
| `0x7a3740` | Terrain::Init7A3740 | In outline: sets up a record with its table (`0xF70058`) |
| `0x774c60` | Park::Find774C60 | In outline: a find over entries (+0x48) |
| `0x760790` | Guest::Part760790 | In outline: a step with a part (+0x4F4) |
| `0x716d20` | Path::Kind716D20 | In outline: a step when a kind check fails (Path::Kind716B20) |
| `0x714340` | Path::Part714340 | In outline: a step with a part (+0x144) |
| `0x97dfe0` | Water::Owner97DFE0 | In outline: when an object's owner (+0xF0) differs, a step with its part (+0xE4) |
| `0x881f60` | Scenery::Free881F60 | In outline: tears down a part |
| `0x7e9d20` | Person::Value7E9D20 | In outline: a value from two parts (slot 0x158, +0x400) |
| `0x783bf0` | GuestGroup::Set783BF0 | In outline: when n differs from a value (+0x13C), goes over entries |
| `0x743f90` | Guest::Free743F90 | In outline: tears down five entries |
| `0x71d670` | Path::Case71D670 | In outline: a case step (Path::Case71B0F0 with -2) on a value (+0xA4) |
| `0x71d600` | Path::Case71D600 | In outline: the same (another copy) |
| `0x71d590` | Path::Case71D590 | In outline: the same with 2 |
| `0x71d520` | Path::Case71D520 | In outline: the same with 2 |
| `0x71d2c0` | Path::Case71D2C0 | In outline: the same with -2 |
| `0x71d250` | Path::Case71D250 | In outline: the same with 2 |
| `0x71d1e0` | Path::Case71D1E0 | In outline: the same with 2 |
| `0x713810` | Path::Terrain713810 | In outline: a terrain step (Path::Terrain71DA80) with an object's values |
| `0x8ea3c0` | Terrain::Step8EA3C0 | In outline: when not flagged (+3), a step with a part (+0x50) |
| `0x8d3590` | Terrain::Point8D3590 | In outline: a step with a part's point (+0x54) |
| `0x8752e0` | Scenery::Part8752E0 | In outline: a step with an object's part (+0xBC) |
| `0x82b1b0` | Scenery::Footprint82B1B0 | In outline: a footprint step (Scenery::Footprint829B70) and sets a value (+0xE8) |
| `0x82b140` | Scenery::Footprint82B140 | In outline: the same with +0xA8 |
| `0x7a39a0` | Terrain::Add7A39A0 | In outline: when given, adds a count (+4) |
| `0x78a210` | Name::Find78A210 | In outline: for none, a find (Name::Find7895A0) |
| `0x9c6ad0` | Particles::List9C6AD0 | In outline: a step with a list (+0x5C) |
| `0x929ab0` | Water::List929AB0 | In outline: a step with a list (+0x6C) |
| `0x777180` | Park::Part777180 | In outline: a step with a part (+0x54) |
| `0x7426a0` | Guest::Part7426A0 | In outline: a step with a part (+0x50) |
| `0x966ba0` | Water::Add966BA0 | In outline: adds when there is room (+0x78, +0x7C) |
| `0x8d9b50` | Terrain::Byte8D9B50 | In outline: a step with a part's byte (+0x54 +4) |
| `0x7858c0` | GuestGroup::Pass7858C0 | In outline: goes over entries (+0x60) |
| `0x9f4170` | Fireworks::List9F4170 | In outline: a step with a list (+4) |
| `0x9a56c0` | Fireworks::Point9A56C0 | In outline: a point from a part (+0x18C) plus an offset |
| `0x871a90` | Scenery::List871A90 | In outline: a step with a list (+0x130) |
| `0x865960` | Scenery::Case865960 | In outline: a step by case (3, ...), 2 skipped |
| `0x71a270` | Path::List71A270 | In outline: a step with a list (+0x12C) |
| `0x9f42c0` | Fireworks::Camera9F42C0 | In outline: with no shared object (`0x13429A8`), a step with the main camera |
| `0x9d5040` | Particles::SetPoint9D5040 | In outline: copies a point into its values (+0x80 on) |
| `0x946640` | Water::Ring946640 | In outline: a step with a ring buffer's entry |
| `0x9423a0` | Water::Entry9423A0 | In outline: a step for entry n (12 bytes at +0x160) |
| `0x93d990` | Water::Free93D990 | In outline: frees a buffer (+0xA0) and remakes it for the grid's size |
| `0x8cd3d0` | Scenery::Point8CD3D0 | In outline: sets a value from a point (`0x682D80`) |
| `0x8603c0` | Scenery::Reset8603C0 | In outline: clears a record's values |
| `0x825b60` | Scenery::Part825B60 | In outline: a step with a part (+0xC4) |
| `0x805f60` | Regions::Find805F60 | In outline: a step when a text is found (SortedMap::FindText) |
| `0x7244d0` | Path::Slot7244D0 | In outline: a step when a slot check passes (Path::Slot730210) |
| `0x724420` | Path::Slot724420 | In outline: the same with Path::Slot72FE30 |
| `0x71ebc0` | Path::Step71EBC0 | In outline: a step with an object's value (+0xA4) |
| `0x71eb50` | Path::Step71EB50 | In outline: the same (another copy) |
| `0x71eae0` | Path::Step71EAE0 | In outline: the same (another copy) |
| `0x71ea70` | Path::Step71EA70 | In outline: the same (another copy) |
| `0x773670` | Park::Part773670 | In outline: with a part (+0xC), a step |
| `0x71bce0` | Path::Reset71BCE0 | In outline: when on (+0x8C), turns it off and acts |
| `0x7e6950` | Person::Step7E6950 | In outline: a step with many values |
| `0x92e1d0` | Water::Check92E1D0 | In outline: a check (`0xBC4200`) with its owner's value |
| `0x8ce7a0` | Scenery::Flag8CE7A0 | In outline: copies a flag (+0x134) into a part (+0x12C) and its opposite |
| `0x9f6420` | Fireworks::Clear9F6420 | In outline: empties a list (+0xE4) |
| `0x9d5d70` | Particles::Init9D5D70 | In outline: sets up a handle and values |
| `0x82ee40` | Scenery::Set82EE40 | In outline: when values change (+0xE8, +0xB4) and not flagged (bit 3), a step |
| `0x82edd0` | Scenery::Set82EDD0 | In outline: the same with +0xA8 |
| `0x7b5190` | Effect::Ring7B5190 | In outline: a step with a ring buffer's entry |
| `0x71dce0` | Path::Update71DCE0 | In outline: when on (+0xA0), an update (Path::Update71C230) |
| `0x9d3a60` | Particles::Entry9D3A60 | In outline: a value for entry n plus 0xC (16 bytes) |
| `0x8dc9d0` | Terrain::Parts8DC9D0 | In outline: with two parts (+4, +8), a step, 0 handled |
| `0x837b80` | Scenery::Pass837B80 | In outline: goes over width times height entries (+0x10, +0x14) |
| `0x7f8530` | Person::Check7F8530 | In outline: with a part (+0x5C0), a step when a value differs (`0x6FDE90`) |
| `0x754ef0` | Guest::Case754EF0 | In outline: when n matches a value (+0x50C), a step by case |
| `0x742710` | Guest::Part742710 | In outline: a step with a part (+0x50) |
| `0x97cd00` | Water::List97CD00 | In outline: a step with a list (+0x16C) |
| `0x925c70` | Water::Manager925C70 | In outline: a step with the water manager (WaterManager::Get) |
| `0x912e70` | Water::Shared912E70 | In outline: a value with a shared object (`0x13429A8`), none handled |
| `0x89d080` | Scenery::Bit89D080 | In outline: for an entry not flagged (bit 0 of +0x3B0), a step |
| `0x8604c0` | Scenery::Set8604C0 | In outline: sets a value and acts with a part (+0xC) |
| `0x82a670` | Scenery::List82A670 | In outline: a step with a list (+0x28) |
| `0x81f040` | SceneryBehaviour::Random81F040 | In outline: a random entry (0x1C bytes at +0x48) by a count (+0x6C) |
| `0x7c3ca0` | Person::Point7C3CA0 | In outline: a step with a point (+0x80, +0x84) |
| `0x7c3480` | Person::Point7C3480 | In outline: a step with a point shifted by a half |
| `0x746140` | Guest::Entry746140 | In outline: for n under a count (+0xC), a step with entry n (0x14 bytes) |
| `0x738690` | Guest::Pass738690 | In outline: goes over entries down from a count (+8) |
| `0x730bf0` | Path::Pass730BF0 | In outline: the same (another copy) |
| `0x90f880` | Park::List90F880 | In outline: a step with a list (+0x58) |
| `0x775130` | Park::Pass775130 | In outline: goes over entries (+0x48) |
| `0x72af90` | Park::Pass72AF90 | In outline: goes over entries (+0x118) |
| `0x9d69b0` | Park::Pass9D69B0 | In outline: goes over entries (+0x70) |
| `0x829aa0` | Park::Pass829AA0 | In outline: goes over entries (+0x13C) |
| `0x774440` | Park::Pass774440 | In outline: goes over entries (+0x48) |
| `0x7743d0` | Park::Pass7743D0 | In outline: goes over entries (+0x48) |
| `0x815230` | Park::Update815230 | In outline: an update when not flagged (+0x18) |
| `0x86ed10` | Park::List86ED10 | In outline: a step with a list (+0x130) |
| `0x774ec0` | Park::Pass774EC0 | In outline: goes over entries (+0x48) |
| `0x9a4350` | Park::Pass9A4350 | In outline: goes over entries (+0x4C) |
| `0x8bfe50` | Park::Assign8BFE50 | In outline: frees its buffer and remakes it from another |
| `0x80a790` | Park::Init80A790 | In outline: sets up a record with empty texts (WideString::Empty) |
| `0x9eedb0` | Park::Clear9EEDB0 | In outline: empties a list (+0x128) |
| `0x7343a0` | Park::Pass7343A0 | In outline: goes over entries (+0x14) |
| `0x9c6e80` | Park::Pass9C6E80 | In outline: goes over entries (+0xD0) |
| `0x989180` | Park::Pass989180 | In outline: goes over entries (+0x108) |
| `0x939a60` | Park::Pass939A60 | In outline: goes over entries (+0x14) |
| `0x9653c0` | Park::Make9653C0 | In outline: makes a 0x40-byte part |
| `0x92ea60` | Park::Pass92EA60 | In outline: goes over entries (+0x1F8) |
| `0x86ede0` | Park::List86EDE0 | In outline: a step with a list (+0x130) |
| `0x86ed80` | Park::List86ED80 | In outline: a step with a list (+0x130) |
| `0x9c6f70` | Particles::Copy9C6F70 | In outline: copies a flag and grows an array (Array4::SetCapacity) to another's size |
| `0x9a4a60` | Fireworks::Set9A4A60 | In outline: sets a part (+0xF4) when it changes |
| `0x99bca0` | Fireworks::Draw99BCA0 | In outline: draws a highlight (Window::DrawHighlight) and acts with no part (+0x104) |
| `0x925dd0` | Water::Current925DD0 | In outline: with the current object (`0x12F56D4`), a step |
| `0x8b5290` | Scenery::Slot8B5290 | In outline: with no slot (+0xF8 = -1), a step with a work buffer |
| `0x88f830` | Scenery::Add88F830 | In outline: grows by 0x100 when full (+0x14, +0x18) and adds |
| `0x86fdf0` | Scenery::Empty86FDF0 | In outline: empties a text and acts with a part (+0x130) |
| `0x74e060` | Guest::Count74E060 | In outline: with a part (+0x18), a step with a count (+0x1C) plus one |
| `0x9f9d10` | Fireworks::Slot9F9D10 | In outline: with a slot (+0x20), a step |
| `0x947680` | Water::Find947680 | In outline: a find over entries (+8) |
| `0x925e40` | Water::Current925E40 | In outline: with the current object (`0x12F56D4`), a step |
| `0x8535c0` | Scenery::Slot8535C0 | In outline: with a slot (+0x20), a step |
| `0x8335e0` | Scenery::Entry8335E0 | In outline: for n under a part's count (+0x9C +0x50), a step |
| `0x752b50` | Guest::Step752B50 | In outline: a step with many values |
| `0x9e1600` | Fireworks::Entry9E1600 | In outline: a step for entry n (0x20 bytes) that is on |
| `0x966830` | Water::Add966830 | In outline: grows by 0x14 when full (+0x30, +0x34) and adds |
| `0x84f280` | Scenery::Find84F280 | In outline: a find over entries down from a count (+8) |
| `0x7fbc30` | Person::Step7FBC30 | In outline: a step with a byte buffer |
| `0x7fa750` | Person::Message7FA750 | In outline: a message result (Message::Result7578E0) |
| `0x78a530` | GuestGroup::Number78A530 | In outline: a number (Window::Number789CA0) and a rectangle from a part (+0xEC) |
| `0x757aa0` | Guest::Message757AA0 | In outline: a message result (Message::Result7578E0) |
| `0x7171f0` | Path::Step7171F0 | In outline: a step with the largest float |
| `0x964e40` | Water::Add964E40 | In outline: adds when there is room (+0x10, +0x14) |
| `0x90f5a0` | Water::Count90F5A0 | In outline: a count when two values differ (+4, +0x10) |
| `0x8457e0` | Scenery::Step8457E0 | In outline: a step with cleared values (Zero6) |
| `0x81f0b0` | SceneryBehaviour::Entry81F0B0 | In outline: a step for entry n (0x1C bytes at +0x48) not flagged |
| `0x774170` | Park::Init774170 | In outline: fills a record with 0, 4 and more |
| `0x738620` | Guest::Pass738620 | In outline: goes over entries (+8) |
| `0x730b80` | Path::Pass730B80 | In outline: the same (another copy) |
| `0x92e7f0` | Water::Grow92E7F0 | In outline: grows by n when a count (+8) is below another |
| `0x8b12c0` | Scenery::Current8B12C0 | In outline: a step when it is the current one (`0x8B0410` +8) |
| `0x865490` | Scenery::Flag865490 | In outline: sets a flag (bit 5 of +0x14) when it changes |
| `0x838a30` | Scenery::Part838A30 | In outline: with a part (+0x318) and on (+0x155), a step |
| `0x80f690` | Regions::Loop80F690 | In outline: a loop while not flagged (+0x24) |
| `0x7c4190` | Person::Find7C4190 | In outline: with no part (+0x108), a find over n entries |
| `0x7bbde0` | Terrain::Parse7BBDE0 | In outline: a parse (Text::Parse495220) under a count (+0x30) |
| `0x7b4fa0` | Effect::Free7B4FA0 | In outline: with an object's slot (+0x188), frees it (SlotPool::Remove) |
| `0x709670` | Grid::Step709670 | In outline: a step with three values |
| `0x99b150` | Fireworks::Part99B150 | In outline: when on (+0x198), a step with a value (+0x18C) |
| `0x983870` | Record::Free983870 | In outline: frees a buffer (+0x18) and more |
| `0x8dd310` | Terrain::List8DD310 | In outline: a step with a list (+0x70) |
| `0x846530` | Scenery::List846530 | In outline: a step with a list |
| `0x834540` | Scenery::Link834540 | In outline: a step when a link is found (Scenery::Link834260) |
| `0x81f4d0` | SceneryBehaviour::Entry81F4D0 | In outline: entry n (0x1C bytes at +0x48) by a value (+0x88) |
| `0x9bbfc0` | Window::Part9BBFC0 | In outline: with a part (+0x13C +0xCC), a step |
| `0x8dee80` | Terrain::Same8DEE80 | In outline: unless a shared flag is set (bit 2 of `0x1313C64`), a step when values match |
| `0x82ed70` | Scenery::Set82ED70 | In outline: when values change (+0xA0, +0xA4) and not flagged (bit 3), a step |
| `0x826760` | Scenery::Part826760 | In outline: a step with a part (+0x18) |
| `0x80a5d0` | Regions::Init80A5D0 | In outline: fills a record with 2 and zeros |
| `0x7b52b0` | Effect::Next7B52B0 | In outline: the next entry of a ring buffer |
| `0x713760` | Path::Kind713760 | In outline: unless an object is of kind 3 (slot 0x30), a step |
| `0x749000` | Guest::Point749000 | In outline: a step with a point (`0x682D50`) |
| `0x71b4a0` | Path::Cell71B4A0 | In outline: a step with a cell of a part (+0x58) |
| `0x9bead0` | Window::Assign9BEAD0 | In outline: when a part (+0x188) differs, a step |
| `0x84d510` | Scenery::Flag84D510 | In outline: sets a shared flag (bit 0 of `0x1400304`) and acts with the device |
| `0x729040` | Path::SetPoint729040 | In outline: copies a point into its values (+0x120 on) |
| `0x9917a0` | Fireworks::Free9917A0 | In outline: tears down (`0x501F80`) and frees |
| `0x83fd40` | Scenery::Set83FD40 | In outline: sets a value (+0x78) from a byte and more |
| `0x7e5190` | Person::Copy7E5190 | In outline: reserves room (Buffer16::Reserve) and copies another's entries |
| `0x7b02a0` | Effect::Free7B02A0 | In outline: tears down (`0x501F80`) and frees |
| `0x79bf70` | Toolbar::Ring79BF70 | In outline: a step with a shared ring buffer's entry (`0x1306BC8`) |
| `0x736190` | Path::Shared736190 | In outline: a step with a shared table (`0x10EF74C`) |
| `0x9fcd40` | Fireworks::Draw9FCD40 | In outline: for a positive value, a device step (slot 0xAC) with a texture |
| `0x9a4210` | Fireworks::Slot9A4210 | In outline: with a part's slot (+0xF0 +4 +0x194), a step |
| `0x8f3b30` | Terrain::Step8F3B30 | In outline: a step with many values |
| `0x7fd440` | Person::Init7FD440 | In outline: clears a count and makes an empty buffer |
| `0x7fc020` | Person::Init7FC020 | In outline: the same (another copy) |
| `0x7ef160` | Person::Check7EF160 | In outline: a step when a value (+0x198) differs from a part's byte |
| `0x7b5310` | Effect::Step7B5310 | In outline: a step with values |
| `0x7705f0` | Park::Find7705F0 | In outline: a find (List::Find7888D0) with values (+0x138 on) |
| `0x96fd40` | Water::Add96FD40 | In outline: grows by 200 when full (Array::Resize96F560) and adds |
| `0x82ef00` | Scenery::Set82EF00 | In outline: when a value (+0xB0) changes and not flagged (bit 3), a step |
| `0x7295e0` | Path::Make7295E0 | In outline: makes a part with 0x2A |
| `0x716bd0` | Path::Slot716BD0 | In outline: for n under 5, a step with slot n (+0x64) |
| `0x9dabe0` | Particles::Pass9DABE0 | In outline: goes over entries (+0x18) |
| `0x9813a0` | Record::Count9813A0 | In outline: with a part (+0x54), a count |
| `0x923980` | Water::Step923980 | In outline: a step with values (+0xF0, +0x100) |
| `0x8c6810` | Scenery::Step8C6810 | In outline: a step with many values |
| `0x828050` | Scenery::Random828050 | In outline: for kind 0x1B, a random value |
| `0x8001b0` | Person::Read8001B0 | In outline: reads an entry (AIDDatabase::ReadEntry) |
| `0x7ae890` | Toolbar::Build7AE890 | In outline: builds a window (Window::Build8E5DF0 with 4, 5, 1) |
| `0x78b4c0` | GuestGroup::Step78B4C0 | In outline: a step with a part's value (+8 +0x88) |
| `0x9f6b80` | Fireworks::Device9F6B80 | In outline: a device step (slot 0x38) and more |
| `0x9dacd0` | Particles::Sum9DACD0 | In outline: a total over entries |
| `0x9664b0` | Water::Cell9664B0 | In outline: a cell from a shared grid (`0x133B520`) |
| `0x8ba370` | Scenery::Free8BA370 | In outline: tears down (`0x501F80`) and frees |
| `0x84d470` | Scenery::Set84D470 | In outline: sets a flag (+0x3C) when it changes |
| `0x99a730` | Fireworks::Make99A730 | In outline: with no part (+0xD8), makes a record (Record::Make99A080) |
| `0x92e710` | Water::Assign92E710 | In outline: frees its buffer and takes another's values |
| `0x92d210` | Water::Step92D210 | In outline: with a list (+0x32C), a step (`0x90F880` with +0x58) |
| `0x926050` | Water::Mode926050 | In outline: in mode 1 with a list (+0x328), a step |
| `0x8c51f0` | Scenery::Step8C51F0 | In outline: a step with many values |
| `0x82a6e0` | Scenery::List82A6E0 | In outline: a step with a list (+0x38) |
| `0x7e6d70` | Person::Init7E6D70 | In outline: fills a record with a value and zeros |
| `0x7a2de0` | Terrain::Find7A2DE0 | In outline: a find (SortedTable::FindVersion) and a step with entry n (0x18 bytes) |
| `0x72acc0` | Path::Init72ACC0 | In outline: fills a record with a value plus 1 and clears another |
| `0x7259c0` | Path::Init7259C0 | In outline: sets up a handle pointing at itself |
| `0x856080` | Park::Pass856080 | In outline: goes over entries (+0x10) |
| `0x846590` | Park::Make846590 | In outline: makes a 0x30-byte part |
| `0x887da0` | Park::List887DA0 | In outline: a step with a list (+0x174) |
| `0x858380` | Park::Pass858380 | In outline: goes over entries (+0x30) |
| `0x8dd2b0` | Park::List8DD2B0 | In outline: a step with a list (+0x70) |
| `0x7b4e10` | Park::Init7B4E10 | In outline: sets up a record with empty texts (WideString::Empty) |
| `0x8400b0` | Park::Pass8400B0 | In outline: goes over entries (+0x58) |
| `0x774010` | Park::Pass774010 | In outline: goes over entries (+0x48) |
| `0x8844d0` | Park::Copy8844D0 | In outline: copies values from another record |
| `0x84de00` | Park::Copy84DE00 | In outline: copies values from another record |
| `0x9174a0` | Park::Copy9174A0 | In outline: copies values from another record |
| `0x8406f0` | Park::Pass8406F0 | In outline: goes over entries (+0x58) |
| `0x8b6620` | Park::Make8B6620 | In outline: makes a 0xC-byte part |
| `0x9e0dd0` | Park::Copy9E0DD0 | In outline: copies values from another record |
| `0x981350` | Park::Pass981350 | In outline: goes over entries (+0x50) |
| `0x9cea90` | Park::Pass9CEA90 | In outline: goes over entries (+0xB8) |
| `0x87f560` | Park::Init87F560 | In outline: sets up a record with empty texts (WideString::Empty) |
| `0x7575e0` | Park::Pass7575E0 | In outline: goes over entries (+0xA4) |
| `0x753440` | Park::Init753440 | In outline: sets up a record with empty texts (WideString::Empty) |
| `0x8eed40` | Park::Copy8EED40 | In outline: copies values from another record |
| `0x8eecf0` | Park::Copy8EECF0 | In outline: copies values from another record |
| `0x9c6df0` | Park::Pass9C6DF0 | In outline: goes over entries (+0xD0) |
| `0x983980` | Park::Pass983980 | In outline: goes over entries (+0x24) |
| `0x83ce70` | Park::Pass83CE70 | In outline: goes over entries (+0xE8) |
| `0x981500` | Record::Pass981500 | In outline: with a part (+0x54), goes over entries (+0x50) |
| `0x8e8f00` | Terrain::Pass8E8F00 | In outline: goes over entries down from n |
| `0x8ce740` | Scenery::Check8CE740 | In outline: sets a shared table entry (`0x10F0ADC`) and checks a box (Window::SetChecked) |
| `0x884ae0` | Scenery::Point884AE0 | In outline: a step with a point (`0x682D80`) |
| `0x86fe60` | Scenery::Part86FE60 | In outline: a step with a part (+0x130) |
| `0x820540` | SceneryBehaviour::Init820540 | In outline: fills a record with -1 and zeros |
| `0x813190` | SceneryBehaviour::Step813190 | In outline: a step with values (+0x34, +0x3C) |
| `0x7faa20` | Person::Save7FAA20 | In outline: writes four bytes to a save (SaveWriter::Write) |
| `0x971000` | Water::Pass971000 | In outline: goes over entries down from n |
| `0x937ac0` | Water::Add937AC0 | In outline: grows when needed (`0x493860`) and adds three two-byte values |
| `0x9373c0` | Water::Count9373C0 | In outline: adds one to a count (+0x1A8) and acts |
| `0x834120` | Scenery::Find834120 | In outline: a find (Thunk) |
| `0x806840` | Regions::Load806840 | In outline: reads a block (Load::Read806610) and goes over entries |
| `0x7f9ee0` | Person::Step7F9EE0 | In outline: a step with a value (+0x5C8) |
| `0x72f110` | Path::Grow72F110 | In outline: grows a buffer when n passes its room |
| `0x72eff0` | Path::Init72EFF0 | In outline: fills a record from its arguments |
| `0x9d5d10` | Particles::Init9D5D10 | In outline: sets up a handle pointing at itself |
| `0x9c51c0` | Particles::Ring9C51C0 | In outline: a step with a shared ring buffer's entry (`0x13414B8`) |
| `0x9b2f10` | Window::Value9B2F10 | In outline: a step with a part's value (+0x108, slot 0xF4) |
| `0x9812f0` | Record::Set9812F0 | In outline: sets a value (+0x50) and goes over entries |
| `0x942110` | Water::Shared942110 | In outline: a step with a shared table (`0x10F0C6C`) |
| `0x8b43e0` | Scenery::Ring8B43E0 | In outline: a step with a shared ring buffer's entry (`0x1311234`) |
| `0x85a560` | Scenery::Slot85A560 | In outline: with no slot (+0x30 = -1), a value of 0x10 |
| `0x72f1a0` | Path::Free72F1A0 | In outline: frees a buffer (+4) and remakes it |
| `0x98adb0` | Record::Pass98ADB0 | In outline: a pass (Park::Pass989180) and acts when not flagged (+0x80) |
| `0x919be0` | Waterfall::Step919BE0 | In outline: a step with values (+0xF0) |
| `0x856350` | Scenery::Step856350 | In outline: a step with many values |
| `0x7f8840` | Person::Value7F8840 | In outline: a value from a shared table (`0xFC4288`) by kind and a pointer table |
| `0x7bb4d0` | Effect::Query7BB4D0 | In outline: sets two vectors and a value (SetTwoVectorsAndValue) |
| `0x7bb470` | Effect::Query7BB470 | In outline: a sphere query (SphereQuery::Set3) |
| `0x7572c0` | Guest::Content7572C0 | In outline: sets content (Window::SetContent67B3C0) from a slot's entry |
| `0x7545f0` | Guest::Free7545F0 | In outline: sets its table (`0xF6C0BC`), tears down a list and frees when asked |
| `0x713900` | Path::Shared713900 | In outline: with no shared value (`0x1305DD8`), a step (`0x6C1DA0`) |
| `0x9ee850` | Fireworks::Point9EE850 | In outline: a step with a point plus values |
| `0x9dad30` | Particles::Find9DAD30 | In outline: a find over entries (+0x18) |
| `0x981450` | Record::Count981450 | In outline: with a part (+0x54), a count |
| `0x97d820` | Water::List97D820 | In outline: with a part (+0xD8), a list step (Water::List97CB00) |
| `0x92e8b0` | Water::Check92E8B0 | In outline: a step when a check fails (Water::Check92E1D0) |
| `0x916520` | Water::Shared916520 | In outline: with shared values (`0x10EDBA0`, `0x10EDD08`) and not flagged (bit 5 of +8), a step |
| `0x8cc540` | Scenery::Check8CC540 | In outline: sets a value (+0x138) and checks a box (Window::SetChecked) |
| `0x7714d0` | Guest::Ratio7714D0 | In outline: when on (+0xB0), a ratio (Guest::Ratio7713C0) |
| `0x753850` | Guest::Find753850 | In outline: a find (Thunk), none handled |
| `0x7537b0` | Guest::Pool7537B0 | In outline: sets up a slot pool of 0x80 (SlotPool::Init) |
| `0x72f2b0` | Path::Cell72F2B0 | In outline: a step with a byte cell (row times a width plus column) |
| `0x919510` | Waterfall::Step919510 | In outline: a step with values (+0xF0) |
| `0x8d9c70` | Terrain::Init8D9C70 | In outline: fills a record with 0 and 0x80 |
| `0x83b2e0` | Scenery::Init83B2E0 | In outline: clears a record |
| `0x7f8660` | Person::Time7F8660 | In outline: with a positive time (+0x5B0), takes n from it |
| `0x7f03e0` | Person::Value7F03E0 | In outline: a step with a value from a part (+0xDC, slot 0xF4) |
| `0x7c2ff0` | Person::Make7C2FF0 | In outline: makes a part and acts by bits |
| `0x7134c0` | Path::Parts7134C0 | In outline: a step with two parts (+0x23C, +0x140) |
| `0x99ca60` | Fireworks::Info99CA60 | In outline: with a part (+0x20), an info text (Info::Text98DA70) |
| `0x97e050` | Water::Owner97E050 | In outline: when a part is not flagged (+0xF8 +0x4D), an owner step (Water::Owner97DFE0) |
| `0x867b20` | Scenery::Step867B20 | In outline: a step with the largest float |
| `0x84ba40` | Scenery::Set84BA40 | In outline: sets a flag (+0xBD) and acts |
| `0x847480` | Scenery::Values847480 | Gives three values (+0x24, +0x2C, ...) through its arguments |
| `0x811b70` | SceneryBehaviour::Reset811B70 | In outline: clears values (+0x28, +0x2C, ...) |
| `0x7fd4f0` | Person::Step7FD4F0 | In outline: a step with a byte buffer |
| `0x7e9130` | Person::Shared7E9130 | In outline: a value from a shared table (`0x10F0620`) |
| `0x758b50` | Guest::Shared758B50 | In outline: sets a value from a shared check (GuestGroup::Shared786ED0) |
| `0x9de610` | Particles::Release9DE610 | In outline: takes one from a count (+4) and acts |
| `0x9b30b0` | Window::Slot9B30B0 | In outline: for its own part and a set slot n (+0x38), a step |
| `0x99cab0` | Fireworks::Info99CAB0 | In outline: an info text (Info::Text98D860) with +0xD8 |
| `0x96f510` | Water::Array96F510 | In outline: makes an aligned array of n entries of 0x30 bytes |
| `0x94de10` | Water::Entry94DE10 | In outline: sets entry n's kind to 2 (0xD0 bytes at +0x1A4) and values |
| `0x9195c0` | Waterfall::Step9195C0 | In outline: a step with a work value |
| `0x8d0ad0` | Terrain::Check8D0AD0 | In outline: with no value (+0x12C +0x134), checks a box (Window::SetChecked) |
| `0x86d050` | Scenery::Set86D050 | In outline: for 0, sets a value (+0x38) when it changes |
| `0x86d000` | Scenery::Set86D000 | In outline: the same with +0x34 |
| `0x86cfb0` | Scenery::Set86CFB0 | In outline: the same with +0x30 |
| `0x846cd0` | Scenery::Value846CD0 | In outline: when on (+0xA8), a value (+0xAC) |
| `0x8277a0` | Scenery::Activate8277A0 | In outline: when a part is not flagged, activates (ManagedObject::Activate) |
| `0x772d90` | Park::Interface772D90 | In outline: with an interface (Interface1F0::Get), a step |
| `0x740390` | Guest::Count740390 | In outline: a step with a count (Guest::Count73E6B0) |
| `0x730770` | Path::Entry730770 | In outline: a step with entry (n, m) (0x14 bytes at +0x6D8) |
| `0x730720` | Path::Entry730720 | In outline: the same at +0xE8 |
| `0x9e2ec0` | Fireworks::Init9E2EC0 | In outline: sets a value to the largest float and clears another |
| `0x9ccad0` | Particles::Set9CCAD0 | In outline: sets a value (+0x64) and a step with a width |
| `0x9be480` | Window::Update9BE480 | In outline: with a part (+0x144), an update (Window::Update9BCF80) |
| `0x916070` | Water::Shared916070 | In outline: unless flagged (bit 3) and with shared values, a check (Water::Shared912DE0 with 10) |
| `0x912ee0` | Water::Shared912EE0 | In outline: a value with a shared object (`0x13429A8`), none handled |
| `0x82eeb0` | Scenery::Set82EEB0 | In outline: when a value (+0xAC) changes and not flagged (bit 3), a step |
| `0x80b940` | Regions::Step80B940 | In outline: a step with many values |
| `0x806230` | Regions::Shared806230 | In outline: with a shared object (`0x130AC08`), a step with its count |
| `0x801c90` | Person::Reset801C90 | In outline: resets a position (+4 from its start) and goes over entries |
| `0x7ef310` | Person::Flag7EF310 | In outline: when its owner is flagged (bit 1), a step with a part (slot 0x28) |
| `0x7e8e40` | Park::Rebuild7E8E40 | In outline: a rebuild (Park::Rebuild7E7440) with values |
| `0x7d4140` | Person::List7D4140 | In outline: goes over a list |
| `0x7c6530` | Person::Point7C6530 | In outline: a step with a point (+0x80, +0x84) |
| `0x758590` | Guest::Take758590 | In outline: takes n from a count when there is enough |
| `0x9e8790` | Fireworks::Point9E8790 | In outline: a point from a shared table (`0x10F1CD0`) plus an offset (`0x10F1CF8`) |
| `0x9c91d0` | Particles::Step9C91D0 | In outline: a step |
| `0x942660` | Water::Add942660 | In outline: grows by 0x100 when full and adds |
| `0x8563b0` | Scenery::Step8563B0 | In outline: a step with many values |
| `0x845070` | Scenery::Show845070 | In outline: when flagged (bit 0 of +4), shows (Scenery::Show84B880) |
| `0x839120` | Scenery::WrapAngle839120 | Wraps a value into the range 0 to a full turn and acts |
| `0x772fb0` | Park::Step772FB0 | In outline: a step (slot 0x13C) and a value (+0x318) |
| `0x754d90` | Guest::Value754D90 | In outline: a value with a kind (+0x414) and slot 0x34 |
| `0x9f90a0` | Fireworks::Pass9F90A0 | In outline: goes over width times height entries (+0x60, +0x64) |
| `0x9df950` | Particles::Init9DF950 | In outline: sets a flag and a value to 3 |
| `0x919570` | Waterfall::Step919570 | In outline: a step with values (+0xF0) |
| `0x880f60` | Scenery::Count880F60 | In outline: takes one from a count (+0xE0) and acts |
| `0x820b50` | SceneryBehaviour::Init820B50 | In outline: sets up a handle pointing at itself |
| `0x80e260` | Regions::Reset80E260 | In outline: clears values (+0x28, +0x2C, ...) |
| `0x7e52d0` | Person::Part7E52D0 | In outline: a value with a part (+4) |
| `0x7e5280` | Person::Part7E5280 | In outline: the same (another copy) |
| `0x7359a0` | Path::Entry7359A0 | In outline: for entry n (0x98 bytes) that is on, a step |
| `0x9a2d60` | Fireworks::Path9A2D60 | In outline: with a part (+8), a firework path (Fireworks::Path9A0AF0) |
| `0x9670d0` | Water::Same9670D0 | In outline: a step when two values match (+8) |
| `0x92e770` | Water::Assign92E770 | In outline: frees its buffer and takes another's values |
| `0x9162c0` | Water::Shared9162C0 | In outline: unless flagged (bit 3) and with shared values, a check (Water::Shared912DE0) |
| `0x9160c0` | Water::Shared9160C0 | In outline: the same (another copy) |
| `0x88aeb0` | Scenery::Value88AEB0 | In outline: a value from a chain of parts (+0xE0 +0x170 +0x158 ...) |
| `0x865580` | Scenery::List865580 | In outline: a step with a list (+0xF4) |
| `0x7a2e90` | Terrain::Find7A2E90 | In outline: a find (SortedTable::FindVersion) and a value from entry n (0x18 bytes) |
| `0x79f770` | Terrain::List79F770 | In outline: a step with a list (+0x540) |
| `0x718d90` | Path::Reset718D90 | In outline: unless flagged (bit 2 of +0x24C), clears a value (+0x208) |
| `0x708900` | Grid::Active708900 | In outline: sets active (SceneryBehaviour::SetActive) and a step with its owner |
| `0x9fa010` | Fireworks::Slot9FA010 | In outline: with a slot (+0xC), a step (`0x8501C0`) |
| `0x981a60` | Record::Shared981A60 | In outline: a step with a shared table (`0x10F0CBC`) |
| `0x981400` | Record::Pass981400 | In outline: with a part (+0x54), goes over entries (+0x50) |
| `0x908470` | Water::Count908470 | In outline: with more than one entry, a step |
| `0x8dfba0` | Window::Check8DFBA0 | In outline: a check (Window::Check8DF9D0) and children (Window::ChildrenSlot2) |
| `0x8439e0` | Scenery::Min8439E0 | In outline: lowers values to another's when smaller |
| `0x827db3` | Scenery::Fragment827DB3 | In outline: a fragment that goes over entries |
| `0x816790` | SceneryBehaviour::Init816790 | In outline: sets up a handle pointing at itself |
| `0x7ea2d0` | Person::Part7EA2D0 | In outline: with a part (+0x4E0), a step |
| `0x7e3cf0` | Person::Init7E3CF0 | In outline: fills a record from its arguments |
| `0x758180` | Guest::Check758180 | In outline: a check of a value (+0x114) against a scaled global |
| `0x749d70` | Guest::Part749D70 | In outline: with a part (+0xF8), goes over entries |
| `0x9e31c0` | Fireworks::Limit9E31C0 | In outline: a step when a value (+0x34) is under a shared limit (`0x10F0E78`) or there is none |
| `0x95b880` | Water::Assign95B880 | In outline: frees its buffer and takes another's values |
| `0x885a40` | Scenery::Part885A40 | In outline: with a part (+0xEC), a check (`0xCE4C70` on +0x50) |
| `0x87b430` | Scenery::Free87B430 | In outline: with a part (+0x10), frees it, deleting when asked |
| `0x79c050` | Toolbar::Find79C050 | In outline: a find when two shared values differ (`0x1306BCC`, `0x1306BD8`) |
| `0x71f2e0` | Path::Part71F2E0 | In outline: a step with a part (+0xC0) |
| `0x9fccf0` | Fireworks::Step9FCCF0 | In outline: a step with a value (+0x6C) |
| `0x9b2ea0` | Window::Content9B2EA0 | In outline: with a part (+0xFC), sets content from a shared table (`0x10EEE90`) by its kind |
| `0x8fda10` | Terrain::Untotal8FDA10 | Takes its values (+0x14, +0x18) from two shared totals (`0x133A028`, `0x133A02C`) |
| `0x87f2b0` | Scenery::Time87F2B0 | In outline: with no shared value (`0x1103C6C`), takes the time (Timer::Milliseconds) |
| `0x8659d0` | Scenery::Bit8659D0 | In outline: with a slot (+0x28), a bit step (Scenery::Bit89D0F0) |
| `0x827d00` | Scenery::Part827D00 | In outline: with a part (+0x9C), a value from it (+0x34) |
| `0x8061c0` | Regions::Shared8061C0 | In outline: a step with a shared object (`0x130AC08`) |
| `0x7cab10` | Person::List7CAB10 | In outline: a step with a list (+0x2EC) |
| `0x7732d0` | Park::Check7732D0 | In outline: a check with a byte and slot 0x38 |
| `0x738270` | Guest::Entry738270 | In outline: a step with entry n's part (0x14 bytes at +0xD4) |
| `0x94dcc0` | Water::Entry94DCC0 | In outline: a step with entry n (0x60 bytes at +0x1B4) by a flag |
| `0x9385f0` | Water::Add9385F0 | In outline: grows when full (Array::Resize417A20) and adds |
| `0x928770` | Water::Mode928770 | In outline: in mode 1 (+0x364), a step with a value (+0x370) |
| `0x917540` | Waterfall::Line917540 | Gives a straight-line value from a slope (+0x10) and two points |
| `0x8df150` | Terrain::Step8DF150 | In outline: a step over 8 entries |
| `0x8389b0` | Scenery::Set8389B0 | In outline: sets a value (+0x170) and acts with another (+0x180) |
| `0x7c3190` | Person::Entry7C3190 | In outline: a step with entry n's part (0x14 bytes at +0x128) |
| `0x7b5260` | Effect::Ring7B5260 | In outline: a step with a ring buffer's entry |
| `0x758e60` | Guest::Part758E60 | In outline: a step with a part (+0x4F0) |
| `0x7371c0` | Guest::Part7371C0 | In outline: a step with a part (+0x94) |
| `0x9e30a0` | Park::Copy9E30A0 | In outline: copies values from another record |
| `0x8eede0` | Park::Copy8EEDE0 | In outline: copies values from another record |
| `0x8eed90` | Park::Copy8EED90 | In outline: copies values from another record |
| `0x83fec0` | Park::Pass83FEC0 | In outline: goes over entries (+0x58) |
| `0x9c6900` | Park::Pass9C6900 | In outline: goes over entries (+0xD0) |
| `0x9aa6d0` | Park::Pass9AA6D0 | In outline: goes over entries (+0x10) |
| `0x9265b0` | Park::Make9265B0 | In outline: makes a 0x24-byte part |
| `0x9d0c10` | Park::Pass9D0C10 | In outline: goes over entries (+0x10) |
| `0x9c6b40` | Park::Pass9C6B40 | In outline: goes over entries (+0xD0) |
| `0x8cc4f0` | Park::Pass8CC4F0 | In outline: goes over entries (+0x134) |
| `0x808ba0` | Park::Pass808BA0 | In outline: goes over entries (+0x18) |
| `0x9bc280` | Park::Pass9BC280 | In outline: goes over entries (+0x50) |
| `0x939950` | Park::Pass939950 | In outline: goes over entries (+0x14) |
| `0x912c80` | Park::Pass912C80 | In outline: goes over entries (+0x14) |
| `0x8852d0` | Park::Pass8852D0 | In outline: goes over entries (+0xF8) |
| `0x884b50` | Park::Pass884B50 | In outline: goes over entries (+0xF4) |
| `0x808b50` | Park::Pass808B50 | In outline: goes over entries (+0x18) |
| `0x737270` | Park::List737270 | In outline: a step with a list (+0x44) |
| `0x7269c0` | Park::Terrain7269C0 | In outline: a step when the terrain is not flagged |
| `0x9f5b70` | Park::List9F5B70 | In outline: a step with a list (+0x4C) |
| `0x9c6a20` | Park::Pass9C6A20 | In outline: goes over entries (+0xD0) |
| `0x9bc3b0` | Park::Pass9BC3B0 | In outline: goes over entries (+0x50) |
| `0x91c1d0` | Park::Pass91C1D0 | In outline: goes over entries (+0x118) |
| `0x88f7d0` | Park::Pass88F7D0 | In outline: goes over entries (+0x8) |
| `0x83fcd0` | Park::Pass83FCD0 | In outline: goes over entries (+0x58) |
| `0x733b10` | Park::Pass733B10 | In outline: goes over entries (+0x14) |
| `0x8406b0` | Park::Pass8406B0 | In outline: goes over entries (+0xC8) |
| `0x8e62f0` | Park::Copy8E62F0 | In outline: copies values from another record |
| `0x8e62b0` | Park::Copy8E62B0 | In outline: copies values from another record |
| `0x9c2aa0` | Park::Pass9C2AA0 | In outline: goes over entries (+0xE4) |
| `0x9831f0` | Park::Pass9831F0 | In outline: goes over entries (+0x58) |
| `0x81ce00` | Park::Pass81CE00 | In outline: goes over entries (+0x68) |
| `0x85b250` | Park::Pass85B250 | In outline: goes over entries (+0x28) |
| `0x83ff10` | Park::Pass83FF10 | In outline: goes over entries (+0x58) |
| `0x759c10` | Park::Pass759C10 | In outline: goes over entries (+0x53C) |
| `0x8b0410` | Park::Make8B0410 | In outline: makes a 0x18-byte part |
| `0x949290` | Water::Free949290 | In outline: with an object, tears it down (Destructor) |
| `0x903680` | Water::Free903680 | In outline: with a part (+0xC), a count step (Water::Count90AE30) and frees when asked |
| `0x8b0470` | Scenery::Time8B0470 | In outline: takes a scaled time from a value (+0x14) and acts |
| `0x843d10` | Scenery::Turn843D10 | In outline: a step with the difference of two clamped turns |
| `0x827ef0` | Scenery::Value827EF0 | In outline: when on (+0x70), a value (+0x6C) |
| `0x815ef0` | SceneryBehaviour::Init815EF0 | In outline: sets up a handle pointing at itself |
| `0x7a2e40` | Terrain::Find7A2E40 | In outline: a find (SortedTable::FindVersion) and a check on entry n (0x18 bytes) |
| `0x783780` | GuestGroup::Slot783780 | In outline: for n under 7, a step |
| `0x773150` | Park::Part773150 | In outline: a step with a part (+0x148) |
| `0x71d190` | Path::Case71D190 | In outline: a case step (Path::Case71B0F0 with -1) on an object's value (+0xA4) |
| `0x71d140` | Path::Case71D140 | In outline: the same with 1 |
| `0x71d0f0` | Path::Case71D0F0 | In outline: the same with -2 |
| `0x71d0a0` | Path::Case71D0A0 | In outline: the same with 2 |
| `0x9d4e50` | Particles::Add9D4E50 | In outline: with a part (+0xCC), adds (Particles::Add9C7340) with its points |
| `0x9c52b0` | Particles::Fill9C52B0 | In outline: fills (Particles::Fill9C4A90) with a rate from a range over a time |
| `0x991190` | Fireworks::Set991190 | In outline: sets a value (+0x94) and acts on the change |
| `0x851b80` | Scenery::Find851B80 | In outline: a step when a find passes (Scenery::Find84F280) |
| `0x840660` | Scenery::Slot840660 | In outline: with an object's part slot (+0x9C +0xC0), a step |
| `0x81ca60` | SceneryBehaviour::Owner81CA60 | In outline: a step with its owner (SceneryBehaviour::Owner) |
| `0x818fb0` | SceneryBehaviour::Owner818FB0 | In outline: the same (another copy) |
| `0x774520` | Park::Shared774520 | In outline: a step with a shared table (`0x10EF8BC`) |
| `0x736060` | Guest::Part736060 | In outline: a value with a part (+0x94) |
| `0x736010` | Guest::Part736010 | In outline: the same (another copy) |
| `0x9f9b00` | Fireworks::Add9F9B00 | In outline: adds when there is room (+8, +0xC) |
| `0x9d4f70` | Particles::Reset9D4F70 | In outline: resets a scroller (Scroller::Reset) |
| `0x99c0f0` | Fireworks::Mode99C0F0 | In outline: in mode 2 (+0xC8), a step with a part (+0xB8, slot 0x10C) |
| `0x983c20` | Record::Count983C20 | In outline: a step when a count passes (Record::Count982C70) |
| `0x856160` | Scenery::Find856160 | In outline: a step when a find passes (SortedPointers::FindFirst2) |
| `0x846ff0` | Scenery::Step846FF0 | In outline: steps (`0x828560`, `0x843980`) with values |
| `0x845370` | Scenery::Find845370 | In outline: a find (`0x846A90`) with a list (+8) |
| `0x83f7b0` | Scenery::Count83F7B0 | In outline: with entries (+0xE8, +0xE4), a step (`0x87EEC0`) |
| `0x7bb640` | Terrain::Flag7BB640 | In outline: when a shared flag is set (bit 0 of `0x10EFC60`) and a part (+0x4C), a step |
| `0x786a40` | GuestGroup::Owner786A40 | In outline: a step with its owner (SceneryBehaviour::Owner) |
| `0x70b9e0` | Grid::Shared70B9E0 | In outline: with no shared value (`0x1306194`), a step |
| `0x7089e0` | Grid::Owner7089E0 | In outline: a step with its owner (SceneryBehaviour::Owner) |
| `0x9e0e20` | Fireworks::Time9E0E20 | In outline: when a shared flag is set (`0x10F0E89`), a step with a shared time (`0x14002C0`) |
| `0x9814b0` | Record::Pass9814B0 | In outline: with a part (+0x54), goes over entries (+0x50) |
| `0x94dc30` | Water::Pass94DC30 | In outline: goes over three entries (+0x158) |
| `0x90ceb0` | Water::Add90CEB0 | In outline: grows by 0x20 when full (Array::Resize417A20) and adds |
| `0x8cd5b0` | Scenery::Set8CD5B0 | In outline: sets a flag (+0xDC) and acts when on |
| `0x8c0030` | Scenery::Add8C0030 | In outline: adds n to a count (+4) and acts |
| `0x829010` | Scenery::Footprint829010 | In outline: turns a cell (Footprint::RotateCell) of entry n (0x58 bytes, +0x34) |
| `0x828fc0` | Scenery::Footprint828FC0 | In outline: the same with +0x1C |
| `0x828f70` | Scenery::Footprint828F70 | In outline: the same with +0 |
| `0x722190` | Path::Step722190 | In outline: when asked, a step |
| `0x716290` | Path::Set716290 | In outline: with an object, a set step (Path::Set714D90) |
| `0x964d20` | Water::Add964D20 | In outline: adds when there is room (+8, +0xC) |
| `0x8ea260` | Terrain::Add8EA260 | In outline: the same (another copy) |
| `0x8defc0` | Terrain::Init8DEFC0 | In outline: clears a flag and values |
| `0x865650` | Scenery::List865650 | In outline: a step with a list (+0xF4) |
| `0x85fd40` | Scenery::Count85FD40 | In outline: a count over entries |
| `0x8464e0` | Scenery::List8464E0 | In outline: a step with a list |
| `0x846450` | Scenery::List846450 | In outline: the same (another copy) |
| `0x7f0300` | Person::Pass7F0300 | In outline: goes over entries (+0x17C) |
| `0x7741e0` | Park::Add7741E0 | In outline: grows by 4 when full (`0x7735E0`) and adds |
| `0x772bb0` | Park::Bits772BB0 | In outline: a value from two bytes' bits |
| `0x7385d0` | Guest::Last7385D0 | In outline: 0 with no entries (+8), else the last |
| `0x735b20` | Path::Add735B20 | In outline: adds when there is room (+8, +0xC) |
| `0x7338d0` | Path::Entry7338D0 | In outline: for entry n (0x98 bytes), a value |
| `0x730b30` | Path::Last730B30 | In outline: 0 with no entries (+8), else the last |
| `0x9fca40` | Fireworks::Entry9FCA40 | In outline: a step for entry n (0x30 bytes) not on |
| `0x9fc910` | Fireworks::Entry9FC910 | In outline: sets a value of entry n's part (0x30 bytes, +0x60) |
| `0x9a55b0` | Fireworks::Offset9A55B0 | In outline: sets an offset (+0x10C on) from a point less a value |
| `0x990050` | Fireworks::Init990050 | In outline: sets up a handle (+8) |
| `0x98ffa0` | Fireworks::Init98FFA0 | In outline: sets up a handle (+4) |
| `0x984950` | Record::Free984950 | In outline: with an object, a step (slot 4) |
| `0x981a10` | Record::Shared981A10 | In outline: a step with a shared table (`0x10F0CBC`) |
| `0x9eaf50` | Fireworks::Set9EAF50 | In outline: sets a value (+0xC) and clears another (+4) |
| `0x9a5830` | Fireworks::Current9A5830 | In outline: when a part (+0x7C) is the current object (`0x12F56D4`), a step |
| `0x856300` | Scenery::Step856300 | In outline: a step with values |
| `0x8391a0` | Scenery::Entry8391A0 | In outline: with a slot (+0x24), adds to an effect entry (Effect::Entry40C260) |
| `0x7535e0` | Guest::Free7535E0 | In outline: sets its table (`0xF6C0B8`) and acts with a part (+0x14) |
| `0x73ca20` | Guest::Reset73CA20 | In outline: resets (Person::Reset7CF900) and copies a value (+0x51C into +0x530) |
| `0x713460` | Path::Part713460 | In outline: with a part (+0x140 +0xC), a step |
| `0x7031e0` | Grid::Draw7031E0 | In outline: with a part (+0x98), draws a grid (Terrain::DrawGrid447740) |
| `0x9fe560` | Fireworks::Room9FE560 | In outline: a step when there is room (+8, +0xC) |
| `0x9f8300` | Fireworks::Room9F8300 | In outline: the same (+0x14, +0x18) |
| `0x9a57d0` | Fireworks::Load9A57D0 | In outline: lays out (Window::Layout) and loads a firework display (FireworkDisplay::Load) |
| `0x95a3d0` | Water::Init95A3D0 | In outline: clears a record |
| `0x9506b0` | Water::Room9506B0 | In outline: a step when there is room (+8, +0xC) |
| `0x9426b0` | Water::Part9426B0 | In outline: with a part (+8) not flagged (+0xC), a step |
| `0x92e870` | Water::Value92E870 | In outline: when asked, (n less one) times a step plus a start (+0x54) |
| `0x901d60` | Water::Room901D60 | In outline: a step when there is room (+8, +0xC) |
| `0x8fffe0` | Terrain::Clear8FFFE0 | In outline: clears entry n's flag (0x60 bytes at +0xA8) when on |
| `0x8cf340` | Scenery::Children8CF340 | In outline: children (Window::ChildrenSlot2) and sets a part's flag (+0x12C +0x138) |
| `0x8bfeb0` | Scenery::Fill8BFEB0 | In outline: fills 0x400 entries of a part (+0xC) |
| `0x8b2600` | Scenery::Init8B2600 | In outline: sets up a handle pointing at itself |
| `0x84bb20` | Scenery::Step84BB20 | In outline: a step with many values |
| `0x811820` | SceneryBehaviour::Pass811820 | In outline: goes over entries (+4) |
| `0x7b39c0` | Effect::Push7B39C0 | In outline: adds a value to its stack (count at +0x104) |
| `0x7ab640` | Terrain::Part7AB640 | In outline: a step with a part (+0x44) |
| `0x7535a0` | Guest::Part7535A0 | In outline: when not flagged (+0x84) with a part (+0x90), a step with a value (+0x50) |
| `0x741860` | Guest::Terrain741860 | In outline: a terrain value (Animal::TerrainB34950) set into a guest (Guest::Set740420) |
| `0x718760` | Path::Reset718760 | In outline: a part step (Path::Part714F30) and a reset (Path::Reset714120) |
| `0x718330` | Path::Check718330 | In outline: when either flag is on (+0x298, +0x2DC), a check (Path::Check717400) |
| `0x9f82c0` | Fireworks::Room9F82C0 | In outline: a step when there is room (+8, +0xC) |
| `0x9d29e0` | Particles::Room9D29E0 | In outline: the same (another copy) |
| `0x972d70` | Water::Pass972D70 | In outline: goes over entries (+4) |
| `0x94dc80` | Water::Entry94DC80 | In outline: sets values of entry n (0x60 bytes at +0x1B4) |
| `0x92e190` | Water::Check92E190 | In outline: a check with a byte (+0x190) and its owner (slot 0x94) |
| `0x8919e0` | Scenery::Init8919E0 | In outline: clears a value and sets another to 1e9 (+0x30) |
| `0x8919a0` | Scenery::Init8919A0 | In outline: the same (another copy) |
| `0x891960` | Scenery::Init891960 | In outline: the same (another copy) |
| `0x891920` | Scenery::Init891920 | In outline: the same (another copy) |
| `0x87c670` | Scenery::Room87C670 | In outline: a step when there is room (+8, +0xC) |
| `0x834990` | Scenery::Check834990 | In outline: a check (its first slot) with the largest float |
| `0x8103a0` | SceneryBehaviour::Room8103A0 | In outline: a step when there is room (+8, +0xC) |
| `0x80f340` | Regions::Add80F340 | In outline: grows a shared list by 0x20 when full (Array::Resize61C320, `0x130AC70`) |
| `0x7a0f90` | Terrain::Flag7A0F90 | In outline: when a list step passes (Terrain::List79FF40) and flagged (bit 0 of +0x408), a step |
| `0x7a0f50` | Terrain::Flag7A0F50 | In outline: the same with bit 1 |
| `0x717f50` | Path::Check717F50 | In outline: a step when a check passes (`0x5F76F0`) |
| `0x9f9b50` | Fireworks::Room9F9B50 | In outline: a step when there is room (+8, +0xC) |
| `0x9d60d0` | Particles::Loop9D60D0 | In outline: goes over a list (+0xC) |
| `0x913760` | Water::Add913760 | In outline: grows by 4 when full (`0x9130A0`) and adds |
| `0x8e92b0` | Terrain::Free8E92B0 | In outline: with a part, frees it |
| `0x9a0ab0` | Fireworks::Value9A0AB0 | In outline: a step with a part's value (+0x1C +4 +0x18C) |
| `0x95fa60` | Water::Free95FA60 | In outline: frees buffers (+4, +0xC) |
| `0x916030` | Water::Shared916030 | In outline: unless flagged (bit 3) and with shared values, a check (Water::Shared912DE0 with 10) |
| `0x905dc0` | Water::Step905DC0 | In outline: a step with many values |
| `0x855520` | Scenery::Entry855520 | In outline: for entry n (12 bytes) with a count and an empty list, a step |
| `0x814b40` | SceneryBehaviour::Random814B40 | In outline: a random entry by a shared count (`0x130B388`) |
| `0x802b70` | Person::Pass802B70 | In outline: goes over entries |
| `0x7bed40` | Terrain::Step7BED40 | In outline: a step with a part (+0x38) by a flag |
| `0x737470` | Guest::Check737470 | In outline: with a part (+0x5C), a path check (Path::Check72E420) |
| `0x713560` | Path::Part713560 | In outline: with a part (+0x224), a step |
| `0x87bb70` | Scenery::Free87BB70 | In outline: with a part, frees it |
| `0x85a5e0` | Scenery::Free85A5E0 | In outline: frees two buffers |
| `0x808800` | Regions::Find808800 | In outline: a find by text (SortedMap::FindText on +0x14) |
| `0x7f8620` | Person::Time7F8620 | In outline: with a positive time (+0x5B0), adds n to it and acts |
| `0x7bea50` | Terrain::Shared7BEA50 | In outline: a step with a shared table (`0x10EFC64`) |
| `0x73bab0` | Guest::Check73BAB0 | In outline: a check with values (+0x51C, +0x520) |
| `0x735ae0` | Path::Free735AE0 | In outline: with a part, frees it |
| `0x8fe1e0` | Terrain::Entry8FE1E0 | In outline: a step with entry n (0x60 bytes at +0xA8) and values |
| `0x756bc0` | Guest::Init756BC0 | In outline: sets up a record with its table (`0xF6C374`) pointing at itself |
| `0x71ea30` | Path::Terrain71EA30 | In outline: a terrain step (Path::Terrain71D6E0) with bytes (+0x88, +0x89) |
| `0x94df40` | Water::Entry94DF40 | In outline: sets a value of entry n (0xD0 bytes at +0x1A4, +0x30) |
| `0x8dac50` | Terrain::Pass8DAC50 | In outline: goes over entries |
| `0x8d33d0` | Terrain::Set8D33D0 | In outline: sets a flag (+0x5D) and acts when another is on (+0x5E) |
| `0x80ecf0` | Regions::Free80ECF0 | In outline: frees a part (+0x10) and acts with another (+0x18) |
| `0x7c3230` | Person::Slot7C3230 | In outline: when two values match (+0xB8, +0x11C) and under 12, a step |
| `0x783710` | GuestGroup::Pass783710 | In outline: goes over entries (+0xE8) |
| `0x753ad0` | Guest::Free753AD0 | In outline: sets its table (`0xF6C0BC`) and tears down a list (+0x70) |
| `0x74e360` | Guest::Find74E360 | In outline: clears a value (+0xC) and a find (Loader::Find744A90) |
| `0x71ab00` | Path::Flag71AB00 | In outline: sets a flag (bit 1 of +0x14) when it changes |
| `0x6fded0` | Grid::Slot6FDED0 | In outline: with a slot (+0x40), a value |
| `0x87a780` | Scenery::Normal87A780 | In outline: a normal step (Scenery::Normal87A370) on entry n (0x24 bytes) and adds one to a count |
| `0x878320` | Scenery::Flag878320 | In outline: when a flag is set (Flags::Bit0) and a kind is above 11, a step |
| `0x8606a0` | Scenery::Colour8606A0 | Turns bytes into floats with a fixed scale |
| `0x845a10` | Scenery::Check845A10 | In outline: 1 for none, else a check |
| `0x81e4e0` | SceneryBehaviour::Init81E4E0 | In outline: fills a record from an object and a value |
| `0x7ce2e0` | Person::Follow7CE2E0 | In outline: a mode step (Person::Mode7C3100) and follows a path (Person::FollowPath) |
| `0x7c3c60` | Person::Pass7C3C60 | In outline: with a part (+0x3DC), a pass (Animal::PassA7A100) |
| `0x7bc110` | Terrain::Count7BC110 | In outline: with a part (+0x30), a count |
| `0x7a2ee0` | Terrain::Find7A2EE0 | In outline: a find (SortedTable::FindVersion) and a value from entry n (0x18 bytes) |
| `0x786830` | GuestGroup::Slot786830 | In outline: a step when a slot check passes (GuestGroup::Slot7861D0 with 2) |
| `0x734360` | Path::Entry734360 | In outline: for entry n (0x98 bytes) that is on, a step |
| `0x9d0d90` | Particles::Free9D0D90 | In outline: with a part, frees it |
| `0x9d5000` | Particles::SetPoint9D5000 | In outline: copies a point into its values (+0x70 on) |
| `0x9c5520` | Particles::Init9C5520 | In outline: sets up a record (Record::InitBE79B0) and clears a value |
| `0x99b500` | Fireworks::Scale99B500 | In outline: n over a shared value (`0x13413B0`), rounded, less one |
| `0x95dc10` | Water::Rebuild95DC10 | In outline: when on (+0x34), rebuilds (Water::Rebuild95C490) |
| `0x93c9e0` | Water::Count93C9E0 | In outline: adds one to a count (+0x34) and acts |
| `0x916280` | Water::Shared916280 | In outline: unless flagged (bit 3) and with shared values, a check (Water::Shared912DE0 with 2) |
| `0x8dfbf0` | Terrain::Clamp8DFBF0 | In outline: when its owner is flagged (bit 0), a clamp (Water::Clamp8DF1C0) |
| `0x87a330` | Scenery::Scale87A330 | In outline: n times a fixed scale, rounded, less one |
| `0x840f10` | Scenery::Log840F10 | In outline: a base-2 logarithm step on a value (Scenery::Value840740) |
| `0x7df1f0` | Person::Last7DF1F0 | In outline: with entries (+0x274), a check on the last one (0x1C bytes at +0x270) |
| `0x7c2140` | Person::Scale7C2140 | In outline: n times a shared value (`0x1106D08`), rounded, less one |
| `0x7b29b0` | Effect::Flags7B29B0 | In outline: when an object is flagged (0x3FF0 of +0x2AC), a step with an object (`0x130A8A0`) |
| `0x7a2da0` | Terrain::Find7A2DA0 | In outline: a step when a find passes (SortedTable::FindVersion) |
| `0x751830` | Guest::Loop751830 | In outline: a loop while a count (+8) is positive |
| `0x73a5a0` | Guest::Count73A5A0 | In outline: adds one to a count (+0x16C) and acts when not flagged (+0x168) |
| `0x6fd6d0` | Grid::Bits6FD6D0 | In outline: for n not negative, a bit from a table (+0x4C) |
| `0x6fc8c0` | Grid::Release6FC8C0 | In outline: releases a part (+0x68) |
| `0x90f840` | Park::Make90F840 | In outline: makes a 0xC-byte part |
| `0x7d4410` | Park::Pass7D4410 | In outline: goes over entries (+0x114) |
| `0x742280` | Park::List742280 | In outline: a step with a list (+0x4) |
| `0x9c6e40` | Park::Pass9C6E40 | In outline: goes over entries (+0xD0) |
| `0x9c6990` | Park::Pass9C6990 | In outline: goes over entries (+0xD0) |
| `0x9c6950` | Park::Pass9C6950 | In outline: goes over entries (+0xD0) |
| `0x958c50` | Park::Pass958C50 | In outline: goes over entries (+0x8) |
| `0x93ca20` | Park::List93CA20 | In outline: a step with a list (+0x14) |
| `0x8dcde0` | Park::Terrain8DCDE0 | In outline: a step when the terrain is not flagged |
| `0x80ba20` | Park::Make80BA20 | In outline: makes a 0xC-byte part |
| `0x83fe80` | Park::Pass83FE80 | In outline: goes over entries (+0x58) |
| `0x808d90` | Park::Pass808D90 | In outline: goes over entries (+0x18) |
| `0x74d840` | Park::Pass74D840 | In outline: goes over entries (+0x48) |
| `0x74d800` | Park::Pass74D800 | In outline: goes over entries (+0x48) |
| `0x7422c0` | Park::List7422C0 | In outline: a step with a list (+0x4) |
| `0x9f3dc0` | Park::Make9F3DC0 | In outline: makes a 0xC-byte part |
| `0x709db0` | Park::Terrain709DB0 | In outline: a step when the terrain is not flagged |
| `0x9916a0` | Park::Pass9916A0 | In outline: goes over entries (+0x108) |
| `0x80b900` | Park::List80B900 | In outline: a step with a list (+0x4) |
| `0x8033c0` | Park::Pass8033C0 | In outline: goes over entries (+0x14) |
| `0x844af0` | Park::List844AF0 | In outline: a step with a list (+0xC) |
| `0x7ae300` | Park::Init7AE300 | In outline: sets up a record with its table (`0xF71508`) |
| `0x742240` | Park::List742240 | In outline: a step with a list (+0x4) |
| `0x983420` | Park::Pass983420 | In outline: goes over entries (+0x58) |
| `0x9833f0` | Park::Pass9833F0 | In outline: goes over entries (+0x58) |
| `0x95ad10` | Park::Pass95AD10 | In outline: goes over entries (+0x1C) |
| `0x9831c0` | Park::Pass9831C0 | In outline: goes over entries (+0x58) |
| `0x9817b0` | Park::List9817B0 | In outline: a step with a list (+0x58) |
| `0x9659e0` | Park::Init9659E0 | In outline: sets up a record with its table (`0xF7E568`) |
| `0x843950` | Park::Copy843950 | In outline: copies values from another record |
| `0x8e8100` | Terrain::Free8E8100 | In outline: frees buffers (+0x10, +0x1C), deleting when asked |
| `0x845130` | Scenery::Handle845130 | In outline: a step with a handle (`0xF77BA0`) |
| `0x79dff0` | Toolbar::Kind79DFF0 | In outline: for a kind not 0 or 4 (+0x190), a step |
| `0x79c010` | Toolbar::Ring79C010 | In outline: a step with a shared ring buffer's entry (`0x1306BC8`) |
| `0x79bfd0` | Toolbar::Ring79BFD0 | In outline: the same (another copy) |
| `0x82a390` | Scenery::Init82A390 | In outline: sets up a handle pointing at itself |
| `0x814b00` | SceneryBehaviour::Pass814B00 | In outline: goes over entries with a shared count (`0x130B388`) |
| `0x7f86d0` | Person::Part7F86D0 | In outline: with a part (+0x5BC), a step (`0x6F1010`) |
| `0x7a69c0` | Staff::Idle7A69C0 | In outline: unless of kind 0xD or 0xE (+0x190), returns to idle (Staff::ReturnToIdle) |
| `0x758ef0` | Guest::Random758EF0 | In outline: sets a value (+0x43C) from a random value |
| `0x725d00` | Path::Set725D00 | In outline: for none, a step with a flag (+0x2C) |
| `0x7241a0` | Path::Points7241A0 | In outline: goes over a path's points (Path::Points) |
| `0x713da0` | Path::Step713DA0 | In outline: a step with the largest float |
| `0x713520` | Path::Check713520 | In outline: whether two values add to at least 1 |
| `0x9cc9f0` | Particles::Set9CC9F0 | In outline: sets a value (+0xB0) and another from it (+0xB8) |
| `0x9024b0` | Water::Find9024B0 | In outline: a find (SortedPointers::FindInt) with values |
| `0x86e810` | Scenery::Reset86E810 | In outline: clears a value (+0x24) and acts with another (+0x28) |
| `0x856400` | Scenery::Step856400 | In outline: a step with a value (+4) |
| `0x84adf0` | Scenery::Pass84ADF0 | In outline: when on (+0x3C), a pass (Scenery::Pass849730) |
| `0x840070` | Scenery::Shared840070 | In outline: with a shared value (`0x10EDC28`) and mode 2 (+0x50), a step |
| `0x7aea00` | Toolbar::Release7AEA00 | In outline: releases a part (+0x134) |
| `0x774570` | Park::Shared774570 | In outline: a step with a shared table (`0x10EF8BC`) |
| `0x73b7c0` | Guest::Shared73B7C0 | In outline: a step with a shared table (`0x10EF764`) |
| `0x733270` | Path::Shared733270 | In outline: a step with a shared table (`0x10EF734`) |
| `0x970950` | Water::Pass970950 | In outline: goes over entries (+8) |
| `0x891760` | Scenery::Free891760 | In outline: sets its table (`0xF7A1EC`), frees a buffer (+0x14) and itself when asked |
| `0x87b210` | Scenery::Part87B210 | In outline: with a part (+0x10), a step |
| `0x8655d0` | Scenery::List8655D0 | In outline: a step with a list (+0xF4) |
| `0x8453c0` | Scenery::Step8453C0 | In outline: a step with values |
| `0x8450f0` | Scenery::Handle8450F0 | In outline: a step with a handle (`0xF77BA0`) |
| `0x80ce30` | Regions::Init80CE30 | In outline: clears a count and makes an empty buffer |
| `0x8059c0` | Regions::Init8059C0 | In outline: sets up a record with its table (`0xF753F0`) |
| `0x7c43e0` | Person::Check7C43E0 | In outline: false under 2, else a check |
| `0x79dfb0` | Toolbar::Flag79DFB0 | In outline: when flagged (bit 0 of +0x408), sets a kind to 0x14 (+0x190) |
| `0x773000` | Park::Value773000 | In outline: unless in mode 1 (+0xE8), a value |
| `0x759750` | Guest::Animate759750 | In outline: sets an animation (Person::SetAnimation) |
| `0x9daba0` | Particles::Pass9DABA0 | In outline: goes over entries (+0x18) |
| `0x9cca30` | Particles::Set9CCA30 | In outline: sets a value (+0xB4) and another from it (+0xB8) |
| `0x99ae00` | Fireworks::Point99AE00 | In outline: a step with a point |
| `0x988850` | Record::Labels988850 | In outline: with a part (+0x114), resets labels (Labels::Reset475BA0) |
| `0x916240` | Water::Shared916240 | In outline: unless flagged (bit 3) and with shared values, a check (Water::Shared912EE0) |
| `0x8df040` | Terrain::Set8DF040 | In outline: sets a value (+8) and copies given values |
| `0x865610` | Scenery::List865610 | In outline: a step with a list (+0xF4) |
| `0x840620` | Scenery::Part840620 | In outline: a step when a part check passes (Scenery::Part840450) |
| `0x7c0b40` | Person::Init7C0B40 | In outline: clears a record |
| `0x771530` | Guest::Ratio771530 | In outline: when on (+0xB0), a ratio (Guest::Ratio7713C0) |
| `0x745690` | Guest::Slot745690 | In outline: sets a slot (+0xDE4) and a value from a shared table (`0xFC35D8`) |
| `0x735770` | Path::Add735770 | In outline: grows by one (Array::Resize7335F0) and adds |
| `0x9ee010` | Fireworks::Event9EE010 | In outline: for its own part, a step (`0x9F41E0`) |
| `0x9d7050` | Particles::Init9D7050 | In outline: sets up a handle pointing at itself |
| `0x9ceae0` | Particles::Add9CEAE0 | In outline: adds when there is room (+0x74, +0x78) |
| `0x9a8060` | Fireworks::Check9A8060 | In outline: a step when a check fails (Window::Check9A7D90) |
| `0x9887c0` | Record::Bit9887C0 | In outline: sets or clears bit n |
| `0x964b80` | Water::Clear964B80 | In outline: clears entry n's flag (0x1018 bytes at +8) when on |
| `0x940610` | Water::Part940610 | In outline: with a part, a step |
| `0x8f8910` | Terrain::Part8F8910 | In outline: the same (another copy) |
| `0x86fc40` | Scenery::Value86FC40 | In outline: a value from a table (+0x10) by two indices |
| `0x851930` | Scenery::Fill851930 | In outline: fills 0x40 entries (+0x1C) |
| `0x828860` | Scenery::Flag828860 | In outline: sets a flag (+0x131) and acts when it differs from a part's |
| `0x8278f0` | Scenery::Entry8278F0 | In outline: for n under a part's count (+0x9C +0x70), a step with entry n (0x44 bytes) |
| `0x8278b0` | Scenery::Entry8278B0 | In outline: the same (another copy) |
| `0x7587f0` | Guest::Reset7587F0 | In outline: clears a value (+0x54C) and in state 0x15, more |
| `0x752e40` | Guest::Shared752E40 | In outline: a step with a shared table (`0x10EF7C8`) |
| `0x727960` | Path::Value727960 | In outline: 3 unless both values are under 4 |
| `0x901be0` | Water::Count901BE0 | In outline: with a part (+0xC), a count step (Water::Count90AE30) |
| `0x86e610` | Scenery::Release86E610 | In outline: releases a part (+4) and frees when asked |
| `0x856240` | Scenery::List856240 | In outline: with a count (+0x10) and an empty list (+0xC), a step |
| `0x845ce0` | Scenery::Item845CE0 | In outline: with a scenery item (SceneryItem::Get20), a step |
| `0x7c1f70` | Person::Init7C1F70 | In outline: clears a flag and values |
| `0x7b28f0` | Guest::Pickup7B28F0 | In outline: a pickup step (Guest::Pickup7B1BA0) and more when given |
| `0x787ff0` | Park::Pass787FF0 | In outline: a pass (Park::Pass787F80) and acts |
| `0x76ffa0` | Park::Handle76FFA0 | In outline: a step with a handle (`0xF6CE6C`) |
| `0x76ff60` | Park::Handle76FF60 | In outline: the same with `0xF6CE70` |
| `0x754f80` | Guest::Value754F80 | In outline: clears a value (+0x504) and gives 5 or 4 by a flag (+0x508) |
| `0x74ccf0` | Guest::Prev74CCF0 | In outline: makes a part and a step with its value (+0x8C) less one |
| `0x74ccb0` | Guest::Next74CCB0 | In outline: makes a part and a step with its value (+0x8C) plus one |
| `0x733a50` | Path::Clear733A50 | In outline: clears entry n's flag (0x98 bytes) when on |
| `0x724fe0` | Path::Grow724FE0 | In outline: frees and remakes a buffer when n passes its room |
| `0x724540` | Path::Slot724540 | In outline: a step when a slot check passes (Path::Slot730400) |
| `0x724490` | Path::Slot724490 | In outline: the same with Path::Slot730020 |
| `0x7243e0` | Path::Slot7243E0 | In outline: the same with Path::Slot72FC30 |
| `0x7242c0` | Path::Slot7242C0 | In outline: the same with Path::Slot72F820 |
| `0x970e50` | Water::Init970E50 | In outline: clears a record |
| `0x902530` | Water::Find902530 | In outline: a find (SortedPointers::FindInt) with values |
| `0x9024f0` | Water::Find9024F0 | In outline: the same (another copy) |
| `0x87f370` | Scenery::Flag87F370 | In outline: when a flag is set (Flags::Bit0) and a part's high half is set, a step |
| `0x87f330` | Scenery::Flag87F330 | In outline: the same (another copy) |
| `0x8656a0` | Scenery::List8656A0 | In outline: a step with a list (+0xF4) |
| `0x81da70` | SceneryBehaviour::Init81DA70 | In outline: sets up a handle (+0x40) pointing at itself |
| `0x80d860` | Regions::Cell80D860 | In outline: a cell of a 32-wide grid, out of range handled |
| `0x7e4140` | Person::Find7E4140 | In outline: in mode 3 (+0xAC), a scenery find (Scenery::Find869950) |
| `0x7c4440` | Person::Case7C4440 | In outline: a value by case |
| `0x7b9d30` | Effect::Query7B9D30 | In outline: sets two vectors and a value (SetTwoVectorsAndValue) |
| `0x79adf0` | Toolbar::Init79ADF0 | In outline: fills a record from its arguments |
| `0x7142f0` | Path::Init7142F0 | In outline: sets up a handle (+0x10) pointing at itself |
| `0x710020` | Grid::Text710020 | In outline: an empty text and when n matches a value (+0x170), a step |
| `0x9f8290` | Fireworks::Clear9F8290 | In outline: clears entry n's flag (0x40 bytes at +0xC) when on |
| `0x9f41e0` | Fireworks::List9F41E0 | In outline: a step with a list (+4) |
| `0x988890` | Record::Reset988890 | In outline: resets a scroller (Scroller::Reset) |
| `0x94dac0` | Water::Init94DAC0 | In outline: clears a record |
| `0x9421a0` | Water::Shared9421A0 | In outline: a step with a shared table (`0x10F0C6C`) |
| `0x914390` | Water::Take914390 | Takes n from a value (+0x88) and acts |
| `0x843a30` | Scenery::Odd843A30 | In outline: for an odd n, a step with a value (+8) |
| `0x838800` | Scenery::Value838800 | In outline: when a part's high half is above 1 and a value (+0xE4) is not negative, a value |
| `0x81e740` | SceneryBehaviour::Owner81E740 | In outline: a step with its owner's part (SceneryBehaviour::Owner +0x84) |
| `0x7c05d0` | Person::Scale7C05D0 | Sets values (+0x24, +0x2C) from shared settings (`0x10ED89C`, `0x10ED8B0`) with fixed scales |
| `0x7bedb0` | Terrain::Step7BEDB0 | In outline: when not asked, a step with a value (+0x34) |
| `0x73d7f0` | Guest::Init73D7F0 | In outline: sets up a handle pointing at itself |
| `0x9ec600` | Fireworks::Clear9EC600 | In outline: when n is its part (+0x1C), clears it |
| `0x9d9c00` | Particles::Shared9D9C00 | In outline: for none, a check with a shared value (`0x1305DD8`) |
| `0x997450` | Fireworks::Clear997450 | In outline: empties a list (+0xD8) |
| `0x94e060` | Water::Free94E060 | In outline: frees buffers (+0, +0x18) |
| `0x94d6d0` | Water::Free94D6D0 | In outline: the same (another copy) |
| `0x940200` | Water::Shared940200 | In outline: with a shared object (`0x133A270`) and its part (+0xA8), a step |
| `0x8cd470` | Scenery::Limits8CD470 | Sets a value (+0xE4) and a number field's limits (NumberField::SetLimits) with a step of 1 |
| `0x8b0350` | Scenery::Copy8B0350 | In outline: with no part (+0x158), copies values from +0x120 |
| `0x827330` | Scenery::Free827330 | In outline: sets its table (`0xF76A98`) and acts with a part (+8) |
| `0x7a3970` | Terrain::Version7A3970 | In outline: a version step (Terrain::Version7A3860) and acts with a shared count (`0x1106C54`) |
| `0x7a2d70` | Terrain::Find7A2D70 | In outline: a step when a find passes (SortedTable::FindVersion) |
| `0x7975c0` | Terrain::Release7975C0 | In outline: releases a part (+0x51C) |
| `0x736830` | Guest::Value736830 | In outline: gives a value from a part (+0x54) |
| `0x719580` | Path::Link719580 | In outline: a link (Path::Link725F30) and a step (Path::Step711860) with a part (+0x174 +0xC) |
| `0x6ff100` | Grid::Part6FF100 | In outline: with a part's part (+0x154 +0xD8), a step |
| `0x9409c0` | Water::Add9409C0 | In outline: grows by 16 when full (Array4::SetCapacity) and adds |
| `0x86ded0` | Scenery::Check86DED0 | In outline: with a part (+0x3C) whose check fails, a step |
| `0x860760` | Scenery::Shared860760 | In outline: by shared flags and a part's value, a check |
| `0x843920` | Scenery::Init843920 | In outline: fills a record from its arguments |
| `0x829b40` | Scenery::Assign829B40 | In outline: when given, takes a reference (Ref::Assign4576B0) |
| `0x813390` | SceneryBehaviour::Pass813390 | In outline: goes over entries (+0x10) |
| `0x7f8360` | Person::Case7F8360 | In outline: a check by state (+0x190) |
| `0x7ea1e0` | Person::Loop7EA1E0 | In outline: a loop while a value (+0x414) is under 3 |
| `0x7c44a0` | Person::Check7C44A0 | In outline: a step when a check passes (slot 0xCC with 0x35) |
| `0x754790` | Guest::Release754790 | In outline: takes one from a count and frees at zero |
| `0x73cbe0` | Guest::Buffer73CBE0 | In outline: a buffer step (Guest::Buffer73CA70) and entry n (16 bytes) |
| `0x9c55d0` | Particles::List9C55D0 | In outline: a step with a list |
| `0x9a6ff0` | Fireworks::Mix9A6FF0 | In outline: a mix master file (Fireworks::MixMasterFile) and rounding (Window::Round642910) |
| `0x8fb170` | Terrain::Free8FB170 | In outline: sets its table (`0xF7CF54`) and frees a slot (SlotPool::Remove) |
| `0x8e8460` | Terrain::Flag8E8460 | In outline: sets a flag of entry n (0x130 bytes at +0x118) when it changes |
| `0x8d34d0` | Terrain::Step8D34D0 | In outline: a step with many values |
| `0x851e30` | Scenery::Slot851E30 | In outline: with a slot (+0xC), a step (`0x8501C0`) |
| `0x845c90` | Scenery::Flattener845C90 | In outline: a scenery item and the terrain flattener (TerrainFlattener::Get) |
| `0x8450c0` | Scenery::Handle8450C0 | In outline: a step with a handle (`0xF77BA0`) |
| `0x82fc00` | Scenery::Cell82FC00 | In outline: when n differs from a part's value (+0x84 +0x4C), a cell step (Scenery::Cell82B350) |
| `0x82bce0` | Scenery::Cell82BCE0 | In outline: unless flagged (bit 3), a cell step (Scenery::Cell82B350) |
| `0x827840` | Scenery::Make827840 | In outline: for n under a part's count (+0x9C +0x70), makes a record (Park::Make827670) |
| `0x7f49e0` | Person::State7F49E0 | In outline: in state 0xD (+0x190), sets a state (Person::SetState with 0, 0) |
| `0x7731c0` | Park::Kind7731C0 | In outline: a value from an object's kind (+0x190) |
| `0x772f40` | Park::Kind772F40 | In outline: the same (another copy) |
| `0x772f10` | Park::Kind772F10 | In outline: the same (another copy) |
| `0x753150` | Guest::Free753150 | In outline: sets its table (`0xF6C0B8`) and acts with a part (+0x14) |
| `0x940fb0` | Water::Entry940FB0 | In outline: a check on entry n's part (0x1C bytes at +0xC8, +0x30) |
| `0x9eec20` | Fireworks::Set9EEC20 | In outline: sets a flag (+0x19C) and acts when on |
| `0x9c54f0` | Particles::List9C54F0 | In outline: a step with a list |
| `0x99af20` | Fireworks::Free99AF20 | In outline: sets its table (`0xF80C2C`) and acts with a part (+4) |
| `0x942170` | Water::Shared942170 | In outline: a step with a shared table (`0x10F0C6C`) |
| `0x9420e0` | Water::Shared9420E0 | In outline: the same (another copy) |
| `0x90c6d0` | Water::Free90C6D0 | In outline: sets a flag (+0x2C) and frees a slot (SlotPool::Remove) |
| `0x8d37f0` | Terrain::Part8D37F0 | In outline: when asked, a step with a part (+0x54, slot 0x14) |
| `0x846af0` | Scenery::Flag846AF0 | In outline: when given, sets a flag (+0x8C) |
| `0x843980` | Scenery::Init843980 | In outline: fills a record from its arguments |
| `0x7e5ee0` | Person::Grow7E5EE0 | In outline: grows when a count plus n passes its room |
| `0x7c3050` | Person::Angle7C3050 | In outline: an angle from a byte (from a table starting at three quarter-turns) |
| `0x7589f0` | Guest::Step7589F0 | In outline: a step, none handled |
| `0x745d10` | Guest::Step745D10 | In outline: a step with fixed values (734, 527) |
| `0x9f79c0` | Fireworks::Entry9F79C0 | In outline: sets values of entry n (0x40 bytes at +0xC, +0x24) |
| `0x8b0380` | Scenery::Shared8B0380 | In outline: with no shared count (`0x1106C54`), a step |
| `0x8b0320` | Scenery::Owner8B0320 | In outline: a value from its owner (SceneryBehaviour::Owner +0x128) |
| `0x872940` | Scenery::Init872940 | In outline: sets up a record with its table (`0xF78B60`) pointing at itself |
| `0x827800` | Scenery::Part827800 | In outline: with a part (+0x84), a step |
| `0x827750` | Scenery::List827750 | In outline: with a part (+0x74), a list step (Animal::ListAFD9D0) |
| `0x7f4940` | Person::Mode7F4940 | In outline: a value by mode (+0x414: 1, ...) |
| `0x737240` | Park::List737240 | In outline: a step with a list (+0x44) |
| `0x737210` | Park::List737210 | In outline: a step with a list (+0x44) |
| `0x8dd380` | Park::List8DD380 | In outline: a step with a list (+0x70) |
| `0x80b9f0` | Park::List80B9F0 | In outline: a step with a list (+0x2C) |
| `0x9e2e30` | Park::Pass9E2E30 | In outline: goes over entries (+0x28) |
| `0x9668d0` | Park::Pass9668D0 | In outline: goes over entries (+0x30) |
| `0x81cb30` | Park::Pass81CB30 | In outline: goes over entries (+0x68) |
| `0x93c9b0` | Park::List93C9B0 | In outline: a step with a list (+0x14) |
| `0x83fca0` | Park::Pass83FCA0 | In outline: goes over entries (+0x58) |
| `0x7512b0` | Park::Release7512B0 | In outline: takes one from a count (+0x4) and frees at zero |
| `0x746500` | Park::Release746500 | In outline: takes one from a count (+0x4) and frees at zero |
| `0x936f90` | Park::List936F90 | In outline: a step with a list (+0x5C) |
| `0x887d70` | Park::List887D70 | In outline: a step with a list (+0x174) |
| `0x7ea190` | Person::Shared7EA190 | In outline: a step with a shared table (`0x10F0650`) |
| `0x7e9d90` | Person::Mode7E9D90 | In outline: a value by mode (+0x414: 1, ...) |
| `0x786800` | GuestGroup::Slot786800 | In outline: a step when a slot check passes (GuestGroup::Slot7861D0 with 2) |
| `0x759cf0` | Guest::Group759CF0 | In outline: unless in state 4 and with a part (+0x4F0 +0xC), a group step (Guest::Group) |
| `0x758c80` | Guest::Copy758C80 | In outline: gives values from its owner (-0x37C, -0x378) |
| `0x7b50d0` | Effect::Ring7B50D0 | In outline: a step with a ring buffer's entry |
| `0x7a0000` | Terrain::Reset7A0000 | In outline: a part step (Terrain::Part79F810) and clears a value (+0x190) |
| `0x79df50` | Toolbar::Kind79DF50 | Sets a kind to 0x13 (+0x190) and a value from a shared one (`0x10EFA38`) |
| `0x79df20` | Toolbar::Kind79DF20 | Sets a kind to 0xE (+0x190) and a value from a shared one (`0x10EFA28`) |
| `0x774800` | Park::Pass774800 | In outline: a pass (Park::Pass7745B0) and sets a value (+0xF0) |
| `0x72f3d0` | Path::Add72F3D0 | In outline: grows (Path::Grow72F110) and adds |
| `0x846740` | Scenery::Owner846740 | In outline: a value with owners (OffsetRef::Owner3, OffsetRef::Owner2) |
| `0x9164f0` | Water::Shared9164F0 | In outline: unless flagged (bit 3) and with shared values, a pass (Water::Pass915780) |
| `0x99bc50` | Fireworks::Step99BC50 | In outline: a step (`0x6A4B10`) with a part's width |
| `0x99bc20` | Fireworks::Part99BC20 | In outline: a step with a part's part (+0x158 +0xE4) by a flag |
| `0x988820` | Record::Labels988820 | In outline: with a part (+0x114), resets labels (Labels::Reset475BA0) |
| `0x8e58c0` | Terrain::Check8E58C0 | In outline: a step when a check passes (`0x5F76F0`) |
| `0x828560` | Scenery::Step828560 | In outline: a step with values |
| `0x7fbff0` | Person::Text7FBFF0 | In outline: makes a text (Text::MakeBD0CE0) from another |
| `0x7b9070` | Effect::Step7B9070 | In outline: a step with values |
| `0x7ae9d0` | Toolbar::Terrain7AE9D0 | In outline: with a part (+0x134) and flagged (bit 0 of +0x94), a terrain step (Terrain::Step4473C0) |
| `0x71ab90` | Path::Texture71AB90 | In outline: with no value (+0x70) and another (+0x74), loads a queue texture (QueueType::LoadTexture) |
| `0x9e3210` | Fireworks::Clear9E3210 | In outline: clears marks (Lists::ClearMarks) and a find (Fireworks::Find9E30F0) |
| `0x9d9b70` | Particles::Shared9D9B70 | In outline: with no shared count (`0x1106C54`), a step |
| `0x94c930` | Water::Green94C930 | Sets the green byte of a colour from a rounded scaled value |
| `0x94c900` | Water::Red94C900 | Sets the red byte of a colour from a rounded scaled value |
| `0x93c190` | Water::Check93C190 | In outline: for 0 with an object, a step |
| `0x8dd060` | Terrain::Case8DD060 | In outline: for an object, a value by its kind (+0x48) |
| `0x8b5380` | SceneryBehaviour::Owner8B5380 | In outline: a step with its owner's part (+0x84) and a value (+0xF4) |
| `0x8b5350` | SceneryBehaviour::Owner8B5350 | In outline: the same with +0x104 |
| `0x8b5320` | SceneryBehaviour::Owner8B5320 | In outline: the same with +0xF8 |
| `0x8844a0` | Scenery::Init8844A0 | In outline: fills a record with 1.0 and zeros |
| `0x83fc70` | Scenery::Loop83FC70 | In outline: a loop while a count (+0x58) is positive |
| `0x82b320` | Scenery::List82B320 | In outline: for n under a part's count (+0x9C +0x70), a list step (Scenery::List82A6E0) |
| `0x818f50` | SceneryBehaviour::Reset818F50 | In outline: clears a value (+0x64) and acts with a shared count (`0x1106C54`) |
| `0x7e4180` | Person::Mode7E4180 | In outline: in mode 1 (+0xAC), sets mode 2 |
| `0x7bc060` | Terrain::Release7BC060 | In outline: takes one from an object's count (+0x48) and acts |
| `0x79def0` | Toolbar::Kind79DEF0 | Sets a kind (+0x194) and a value from a shared table (`0x10EFA18`) by it |
| `0x799b30` | Terrain::Count799B30 | In outline: 0 with no entries (+0x110), else a step |
| `0x783750` | GuestGroup::Slot783750 | In outline: for n under 7 with entry n's part (0x14 bytes at +0x60), a value |
| `0x771c40` | Guest::Random771C40 | In outline: adds one to a count (+0x10) and a random step |
| `0x7597a0` | Guest::Clear7597A0 | In outline: a step when a clear passes (Person::Clear7DF090) |
| `0x73ab50` | Guest::Step73AB50 | In outline: a step with work values |
| `0x713d70` | Path::Kind713D70 | In outline: a kind step (Path::Kind713960) |
| `0x8cd440` | Scenery::Bind8CD440 | In outline: sets a value (+0xE0) and binds a number field to it (NumberField::BindFloat) |
| `0x845ab0` | Scenery::Handle845AB0 | In outline: a step with a handle (`0xF77BDC`) |
| `0x824670` | SceneryBehaviour::Part824670 | In outline: with a part (+0xB4), a step |
| `0x7d04f0` | Person::Copy7D04F0 | In outline: a step (`0x7CBF50`) and copies a flag (+0x20) |
| `0x7bed80` | Terrain::Resize7BED80 | In outline: with a part (+0x34), resizes (Matrices::ResizeCC17B0) |
| `0x7b23a0` | Guest::Pickup7B23A0 | In outline: a pickup step (Guest::Pickup7B1BA0 with 0) and acts with a part (+0x130) |
| `0x7527b0` | Guest::Call7527B0 | In outline: with a part (+0x14), a call |
| `0x73ee00` | Guest::Free73EE00 | In outline: sets its table (`0xF6AC88`) and a list step |
| `0x71f2b0` | Path::Flag71F2B0 | In outline: sets an object's flag (+0xBC) and acts with a part (+0xBC) |
| `0x9e8e10` | Fireworks::Check9E8E10 | In outline: when a value (+0x84) is not positive and two others differ, a step |
| `0x9c1230` | Window::Set9C1230 | Sets two values (+0x108, +0x10C) and acts |
| `0x9a05b0` | Fireworks::Clamp9A05B0 | In outline: a reference step and a clamp (Fireworks::Clamp994830) |
| `0x8f3720` | Terrain::Step8F3720 | In outline: a step with values |
| `0x8fe300` | Terrain::Reset8FE300 | In outline: sets a flag (+0x10) and clears a value (+0x5C) |
| `0x8d0140` | Terrain::Children8D0140 | In outline: children (Window::ChildrenSlot2) and a step with a part (+0x12C, slot 0xF8) |
| `0x8709f0` | Scenery::List8709F0 | In outline: a step with a part (+0x50) and a list step (Scenery::List86EEE0) |
| `0x86fc10` | Scenery::Near86FC10 | In outline: a near step (Scenery::Near86F890) with minus a value (Scenery::Value838710) |
| `0x869860` | Scenery::Set869860 | In outline: sets a value (+0xA4) when it changes and acts unless flagged |
| `0x83dbc0` | Scenery::Set83DBC0 | In outline: sets values (+0xEC, +0xF0, ...) |
| `0x838980` | Scenery::Kind838980 | In outline: for an object of kind 1, 2 or 6, a value |
| `0x824610` | SceneryBehaviour::Active824610 | In outline: sets active (SceneryBehaviour::SetActive) and a step with its owner |
| `0x80e4c0` | Regions::Free80E4C0 | In outline: frees a part (+0x10) and acts with another (+0x18) |
| `0x7f4a10` | Person::Dress7F4A10 | In outline: a dress step (Person::Dress7EB590) and clears a flag (+0xC2) |
| `0x752780` | Guest::Set752780 | In outline: sets a flag (+0x84) when it changes |
| `0x729010` | Path::Fill729010 | In outline: fills 4 entries (+0xF4) |
| `0x9f7970` | Fireworks::Entry9F7970 | In outline: sets values of entry n (0x40 bytes at +0xC, +0x10) |
| `0x9e2940` | Fireworks::Scale9E2940 | In outline: makes a part and scales n by its value (+0x5C) |
| `0x9c7310` | Particles::Count9C7310 | In outline: takes one from a count (+0x48) and acts |
| `0x94c960` | Water::Blue94C960 | Sets the blue byte of a colour from a rounded scaled value |
| `0x946f50` | Water::Init946F50 | In outline: sets up a record with its table (`0xF7E29C`) pointing at itself |
| `0x910460` | Water::Device910460 | In outline: a device step (slot 0xA4 with 9, 0xF) |
| `0x8e9480` | Terrain::Init8E9480 | In outline: sets up a record with its table (`0xF7CCBC`) pointing at itself |
| `0x8bc450` | Scenery::Set8BC450 | In outline: sets a flag (+0x38) and a step (slot 0xC) over its size |
| `0x85a4f0` | Scenery::Add85A4F0 | In outline: with no slot (+0x28), adds (List::Add49AB40) |
| `0x8407e0` | Scenery::Has8407E0 | Whether slot n (under a count, +0xC8) of a table (+0xC4) is set |
| `0x8245d0` | SceneryBehaviour::Release8245D0 | In outline: releases a part (+0x140) |
| `0x7ea6d0` | Person::Shared7EA6D0 | In outline: a step with a shared table (`0x10F0650`) |
| `0x7aef30` | Toolbar::Toggle7AEF30 | Turns a flag (+0x208) on or off and acts (slot 0x14) |
| `0x73d9d0` | Guest::Shared73D9D0 | In outline: a step with a shared table (`0x10EF794`) |
| `0x72f200` | Path::Free72F200 | In outline: frees a buffer and itself when asked |
| `0x713d40` | Path::Release713D40 | In outline: releases a part (+0x224) |
| `0x8b4500` | Scenery::Add8B4500 | Adds n to a value (+0x6C) and acts |
| `0x8b44a0` | Scenery::Free8B44A0 | In outline: with a slot (+0x60), frees it (SlotPool::Remove) |
| `0x86fbe0` | Scenery::Near86FBE0 | In outline: a near step (Scenery::Near86F890) with a value (Scenery::Value838710) |
| `0x845d50` | Scenery::Handle845D50 | In outline: a step with a handle (`0xF77BF0`) |
| `0x845d20` | Scenery::Handle845D20 | In outline: the same with `0xF77BEC` |
| `0x845a50` | Scenery::Handle845A50 | In outline: the same with `0xF77BD8` |
| `0x829b10` | Scenery::Flag829B10 | In outline: when flagged (bit 1 of +0x7C), clears bit 2 |
| `0x827960` | Scenery::Check827960 | In outline: when not flagged (+0x130) and a part is not flagged (bit 5), a step |
| `0x81cb00` | SceneryBehaviour::Find81CB00 | In outline: a find over entries (+0x68) |
| `0x81ca20` | SceneryBehaviour::Free81CA20 | In outline: with a slot (+0x60), frees it (SlotPool::Remove) |
| `0x7e2220` | Person::Init7E2220 | In outline: sets its table (`0xF7289C`) and sets up six text references |
| `0x7b6760` | Effect::Move7B6760 | In outline: moves an object in a tree (QuadTree::Move) by its slot (+0x18C) |
| `0x79df80` | Toolbar::Kind79DF80 | Sets a kind to 0xF (+0x190) and a value from a shared one (`0x10EFA18`) |
| `0x73c1a0` | Guest::Dress73C1A0 | In outline: a dress step (Person::Dress7EB590) and in state 0xE above 10, more |
| `0x9f5440` | Fireworks::Sum9F5440 | In outline: a total over a list (+0x4C) |
| `0x93d080` | Water::Init93D080 | In outline: clears a count and a flag |
| `0x92d1e0` | Water::Count92D1E0 | In outline: adds one to a count (+0x310) and acts |
| `0x87f240` | Scenery::Flag87F240 | In outline: when a flag is set (Flags::Bit0), a step |
| `0x87f210` | Scenery::Flag87F210 | In outline: the same (another copy) |
| `0x85bd10` | Scenery::Bits85BD10 | In outline: a bits step (Scenery::Bits85B1A0) |
| `0x845e40` | Scenery::Handle845E40 | In outline: a step with a handle (`0xF77BF8`) |
| `0x838a00` | Scenery::Values838A00 | Gives two values (+0x170, +0x174) |
| `0x814420` | SceneryBehaviour::Loop814420 | In outline: a step when a loop fails (Scenery::Loop86EBB0) |
| `0x7eecd0` | Person::Part7EECD0 | In outline: a step with a part (Window::Part642E80) |
| `0x786a90` | GuestGroup::Owner786A90 | In outline: a value from its owner (SceneryBehaviour::Owner +0xAC) |
| `0x7597d0` | Guest::Find7597D0 | In outline: a find (Person::Find7DEFA0) with a part's value (+0x400, slot 0x1C) |
| `0x725b60` | Path::Init725B60 | In outline: a list member (ListMember with `0x10EC9E8`) with its table (`0xF6A4F0`) |
| `0x709560` | Grid::Handle709560 | In outline: a step with a handle (`0xF68FEC`) |
| `0x93d960` | Water::Copy93D960 | In outline: copies a value (+0x14) and acts with a part (+4) |
| `0x90c240` | Water::Has90C240 | Whether a part (+0x38) is set |
| `0x8d30d0` | Terrain::Value8D30D0 | In outline: when not on (+0x5C), 0x36, else another value |
| `0x86e570` | Scenery::Release86E570 | In outline: releases a part (+0x40) |
| `0x86d370` | Scenery::SetCell86D370 | In outline: sets a cell (+0x50, +0x54) when it changes |
| `0x8457b0` | Scenery::Init8457B0 | In outline: sets its table (`0xF77BD0`) and clears values (Zero6) |
| `0x7b24a0` | Effect::Call7B24A0 | In outline: with a count (+0x104), a call (its first slot) with its stack |
| `0x9ced80` | Particles::Flag9CED80 | Sets the flag of an entry chosen by the high bits of n (list) and the low 20 bits (index) |
| `0x9fbf10` | Fireworks::Part9FBF10 | In outline: a value from a part's list (+0x88 +0x40) when it has entries |
| `0x9efe50` | Fireworks::Place9EFE50 | In outline: places a part (Window::Place9F6920) and a step (Fireworks::Part9EEEB0) |
| `0x9be4d0` | Window::Particles9BE4D0 | In outline: children (Window::ChildrenSlot2) and a particle source step |
| `0x959fb0` | Water::Set959FB0 | In outline: sets a flag (+0x58) and acts on the change |
| `0x93cdf0` | Water::Normal93CDF0 | In outline: sets a value to the largest float and a normal step (Vector::NormalBEE460) |
| `0x919ae0` | Waterfall::Device919AE0 | In outline: with a part (+0x148) and a shared flag (`0x10F0BDC`), a device step (slot 0x44) |
| `0x919ab0` | Waterfall::Device919AB0 | In outline: the same with +0x144 and `0x10F0BDB` |
| `0x919a80` | Waterfall::Device919A80 | In outline: the same with +0x140 and `0x10F0BD9` |
| `0x919a50` | Waterfall::Device919A50 | In outline: the same with +0x13C and `0x10F0BD8` |
| `0x8b03b0` | Scenery::Shared8B03B0 | In outline: with a shared object (`0x1106C54`), copies its value (+0x48) into +0x1C |
| `0x860920` | Scenery::Slot860920 | In outline: with slot n set (+0x18), a step (`0x40C3C0`) |
| `0x81ba90` | SceneryBehaviour::Set81BA90 | In outline: with a part (+0x64), sets a value to 1 (+0xC8) |
| `0x7e6890` | Person::Init7E6890 | In outline: clears a count and makes an empty buffer |
| `0x8fb340` | Terrain::Add8FB340 | Adds n to its first value and acts |
| `0x8b57f0` | Scenery::Count8B57F0 | In outline: 0 with no entries (+0x2C), else a step |
| `0x7b3b00` | Effect::Init7B3B00 | Sets a value to the largest float (+0xC) and clears another (+0x4C) |
| `0x761420` | Guest::Release761420 | In outline: takes one from a count and frees at zero |
| `0x75a530` | Guest::Release75A530 | In outline: the same (another copy) |
| `0x991800` | Fireworks::Pass991800 | In outline: goes over entries down from a count (+0x48) |
| `0x94dfe0` | Water::SetEntry94DFE0 | Sets a byte of entry n (0xD0 bytes at +0x1A4, +0xC8) |
| `0x94ddb0` | Water::SetEntry94DDB0 | Sets a value of entry n (0xD0 bytes at +0x1A4, +0xBC) |
| `0x941030` | Water::First941030 | In outline: the first value of a part's list (+0x13C) when it has entries |
| `0x941010` | Water::First941010 | In outline: the same with +0x120 |
| `0x8fe2a0` | Terrain::Flags8FE2A0 | Whether entry n (0x60 bytes at +0xA8) has the given flags (+0x54) |
| `0x8f7730` | Terrain::Colours8F7730 | In outline: a count (Terrain::Count8F6CB0) and colours (Terrain::Colours8F6920) |
| `0x8dd6f0` | Terrain::Flattener8DD6F0 | In outline: sets a value (+4) and the terrain flattener (TerrainFlattener::Get) |
| `0x88ce00` | Scenery::Plane88CE00 | In outline: a dot product of two points less a value (a plane distance) |
| `0x86e5c0` | Scenery::Release86E5C0 | In outline: releases a part (+4) |
| `0x847400` | Scenery::Slot847400 | In outline: with a slot (+0xA4), sets an effect byte (`0x4499E0`) |
| `0x800230` | Person::Call800230 | In outline: when n matches a value (+0xC), a call (slot 4) with its values |
| `0x7c1dc0` | Person::Angle7C1DC0 | In outline: a rounded scaled value kept to 10 bits |
| `0x717e10` | Path::Reset717E10 | In outline: a check (Path::Check717400) and clears a value (+0x2F0) |
| `0x9df9e0` | Particles::Free9DF9E0 | In outline: with a shared part (`0x1341840`), frees it |
| `0x9de660` | Particles::Free9DE660 | In outline: with a shared part (`0x1341838`), frees it |
| `0x9c72f0` | Particles::Check9C72F0 | In outline: with no entries (+0x48), a check (`0x9C69D0`) |
| `0x991680` | Fireworks::SetEntry991680 | In outline: sets a value of entry n (0x58 bytes at +0x104) and updates (Window::Update98DDA0) |
| `0x9887a0` | Record::Set9887A0 | Sets a value (+0xF4) and another to a scaled copy (+0x10C) |
| `0x87ef40` | Scenery::Ride87EF40 | In outline: makes a part and a ride part step (Ride::Part5F3DF0) |
| `0x870a60` | Scenery::Place870A60 | In outline: places (Window::Place679450) and acts with a part (+0x12C) |
| `0x860320` | Scenery::Slot860320 | In outline: with a slot (+0x2C), a step |
| `0x8e84b0` | Terrain::Check8E84B0 | Whether a difference of two values (+0x120, +0x128) is at most another (+0x1BC) |
| `0x8e58f0` | Terrain::Step8E58F0 | In outline: a step with a value (+0x12C) |
| `0x8d34b0` | Terrain::Set8D34B0 | In outline: sets a mode to 1 (+0x60) and copies a value (+0x64) |
| `0x8bc480` | Scenery::Check8BC480 | In outline: when not flagged (+0x38) and a shared flag is off, a step |
| `0x88f900` | Scenery::Free88F900 | In outline: sets its table (`0xF7A1EC`) and frees a buffer (+0x14) |
| `0x85a520` | Scenery::Heap85A520 | In outline: with a slot (+0x28), a heap step (Heap::Step49AAA0) |
| `0x772ef0` | Park::ClearBit772EF0 | Clears bit n of a value (+0xE0) |
| `0x74ec60` | Guest::Set74EC60 | In outline: sets a value (+0x178) and acts with a part (+0x184) |
| `0x9ffb70` | Fireworks::Make9FFB70 | In outline: makes a record (+0x1C) and gives the argument |
| `0x9d9ba0` | Particles::Case9D9BA0 | In outline: a value by case (0, 1, ...) |
| `0x98e740` | Record::Part98E740 | In outline: with a part (+0x18) with entries, a step |
| `0x919c40` | Waterfall::Flag919C40 | In outline: when flagged (bit 4 of +0x40), sets bit 10 |
| `0x8fc950` | Terrain::Make8FC950 | In outline: makes a record (+0x1C) and gives the argument |
| `0x839270` | Park::Pass839270 | In outline: goes over entries (+0xE8) |
| `0x9c4310` | Park::Init9C4310 | In outline: sets up a record with its table (`0xF84870`) |
| `0x828540` | Park::Init828540 | In outline: sets up a record with its table (`0xF76AF4`) |
| `0x751390` | Park::Init751390 | In outline: sets up a record with empty texts (WideString::Empty) |
| `0x751310` | Park::Init751310 | In outline: sets up a record with empty texts (WideString::Empty) |
| `0x7464e0` | Park::Init7464E0 | In outline: sets up a record with empty texts (WideString::Empty) |
| `0x9df9d0` | Park::Make9DF9D0 | In outline: makes a 0x40-byte part |
| `0x965610` | Park::Init965610 | In outline: sets up a record with its table (`0xF7E52C`) |
| `0x838960` | Scenery::Kind838960 | In outline: a value from a kind (low half of +8) |
| `0x82b300` | Scenery::Footprint82B300 | In outline: a footprint step (Scenery::Footprint829B70) and sets a value (+0xB0) |
| `0x811bc0` | SceneryBehaviour::Check811BC0 | In outline: when not flagged (+0x31), a step |
| `0x7f8600` | Person::Group7F8600 | In outline: with a guest group (Guest::Group), a step |
| `0x963e20` | Water::Shared963E20 | In outline: with a shared object (`0x133B4CC`), tells it (slot 4 with 1) |
| `0x958940` | Water::Release958940 | In outline: releases a device resource (slot 0xC8) and clears it |
| `0x910380` | Water::Release910380 | In outline: the same (another copy) |
| `0x87bae0` | Scenery::SetEntry87BAE0 | In outline: sets a value of entry n (0x80 bytes at +0xC, +0x60) |
| `0x870a20` | Scenery::Children870A20 | In outline: children (Window::ChildrenSlot2) and acts with a part (+0x12C) |
| `0x8407c0` | Scenery::Entry8407C0 | In outline: gives a value from entry n (0x4C bytes) of a made part |
| `0x8387e0` | Scenery::Step8387E0 | In outline: a step (`0xCE23E0`) |
| `0x8387c0` | Scenery::Set8387C0 | In outline: a mesh set (Mesh::SetCE23A0) |
| `0x7fb140` | Person::Map7FB140 | In outline: sets up a map (IdMap) and a value (+0x18) |
| `0x78b5d0` | GuestGroup::Step78B5D0 | In outline: a step (`0x65AA90`) with its owner's value (slot 0x15C) |
| `0x7173e0` | Path::Reset7173E0 | In outline: clears a value (+0x150) and acts with no part (+0x230) |
| `0x9fbef0` | Fireworks::Set9FBEF0 | In outline: sets a value (+0x74) and acts with a shared value (`0x10EDB90`) |
| `0x87eee0` | Scenery::Colours87EEE0 | In outline: a flexi colour group (FlexiColourGroupCache::Get on +0xF8) |
| `0x87bac0` | Scenery::Flag87BAC0 | In outline: sets a flag of entry n (0x80 bytes, +0x64) when it changes |
| `0x7ae980` | Toolbar::Check7AE980 | In outline: a check with a made part (Terrain::Make7AE350) |
| `0x7ae2e0` | Toolbar::Add7AE2E0 | Adds two values to its bytes (+4, +5) |
| `0x772eb0` | Park::Check772EB0 | In outline: a step when a check (`0x772DE0`) is 0 |
| `0x772c40` | Guest::State772C40 | In outline: sets a state (Person::SetState with 0, 0) and a path tile step (Guest::PathTile760810) |
| `0x711830` | Path::Make711830 | In outline: a step (`0x719F10`) and makes a window (Window::Make63AC10) |
| `0x94dfc0` | Water::SetEntry94DFC0 | Sets a value of entry n (0xD0 bytes at +0x1A4, +0x14) |
| `0x94dfa0` | Water::SetEntry94DFA0 | The same with +0x10 |
| `0x94df80` | Water::SetEntry94DF80 | The same with +0x50 |
| `0x94df20` | Water::SetEntry94DF20 | The same with +8 |
| `0x912f30` | Water::Shared912F30 | Whether two shared values are set (`0x10EDBA0`, `0x10EDD08`) |
| `0x90c330` | Water::Rebuild90C330 | In outline: the water manager and a rebuild (Water::Rebuild95DC10) with a part (+0x3C) |
| `0x8e81e0` | Terrain::SetEntry8E81E0 | Sets a value of entry n (0x130 bytes at +0x118, +8) |
| `0x9fba30` | Fireworks::Count9FBA30 | In outline: adds one to a shared count (`0x1341D44`) and acts past 3 |
| `0x9d5cb0` | Particles::List9D5CB0 | In outline: a step with a list (+0x1C) and an object's part (+0x30) |
| `0x968e00` | Fireworks::Reset968E00 | In outline: clears values (+4, +0xC, ...) |
| `0x95f550` | Water::Free95F550 | In outline: frees buffers (+0x3C, +0x40) |
| `0x8db780` | Terrain::Init8DB780 | In outline: sets flags (+4, +5) and more |
| `0x8b30f0` | Scenery::Camera8B30F0 | In outline: when on (+0x108), a camera step (Scenery::Camera8B1690) |
| `0x7e3ac0` | Person::Mode7E3AC0 | In outline: in mode 3 (+0x114), a value from a part (+0x118, slot 0x84) |
| `0x74e310` | Guest::Part74E310 | In outline: with a part (+4), a step |
| `0x731df0` | Path::Name731DF0 | In outline: with no value (+0xD0), a unique name (Names::Unique7314B0) |
| `0x709530` | Grid::Free709530 | In outline: a step (Sky::Entry42E8B0) and frees |
| `0x704c90` | Grid::Set704C90 | In outline: sets a flag (+0x1B0) when it changes |
| `0x8e6860` | Terrain::Clear8E6860 | In outline: when n is its part (+0x1C0), clears it |
| `0x8d3950` | Terrain::Shared8D3950 | In outline: with a shared object's part (`0x1311650` +0x80), a pass (Terrain::Pass8DAC50) |
| `0x8b49d0` | Scenery::Add8B49D0 | In outline: adds to a slot pool (SlotPool::Add) and keeps the slot (+0x60) |
| `0x870a40` | Scenery::Children870A40 | In outline: children (Window::ChildrenSlot4) and acts with a part (+0x12C) |
| `0x845c00` | Scenery::Add845C00 | In outline: adds a value from an object (slot 0x6C) to a count (+4) |
| `0x837c10` | Scenery::ClearFlags837C10 | Clears the given flags in entry n (+0x54) |
| `0x827510` | Scenery::Init827510 | In outline: clears a record |
| `0x8272e0` | Scenery::Free8272E0 | In outline: sets its table (`0xF76A98`) and acts with a part (+8) |
| `0x809410` | Regions::Part809410 | In outline: with a part, a step |
| `0x7975f0` | Terrain::Name7975F0 | In outline: a name step (Person::Name7CD380) and acts with a part (+0x51C) |
| `0x785930` | GuestGroup::Slot785930 | In outline: a slot step (GuestGroup::Slot783E40 with 1, 0) |
| `0x7578c0` | Guest::Entry7578C0 | In outline: with slot n (+4) set, a value |
| `0x742360` | Guest::Copy742360 | In outline: gives its part |
| `0x9cbc50` | Particles::Set9CBC50 | In outline: sets a value (+0xA4) and a pass (Park::Pass9C6900) |
| `0x988800` | Record::Bit988800 | Whether bit n of a value (+0xF8) is set |
| `0x9fc960` | Fireworks::SetEntry9FC960 | Sets a value of entry n (0x30 bytes at +0x54, +0x14) |
| `0x95dc50` | Water::Rebuild95DC50 | In outline: when on (+0x34), rebuilds (Water::Rebuild95C490) |
| `0x910360` | Water::Check910360 | In outline: a step when a part's check passes (+4, slot 0x34) |
| `0x89c760` | Scenery::Add89C760 | In outline: adds (Scenery::Add891680) |
| `0x8915d0` | Scenery::Add8915D0 | In outline: adds (Scenery::Add88F830) |
| `0x847460` | Scenery::Part847460 | In outline: with a part (+0xB0), a step with its value (+0x38) |
| `0x7b2930` | Effect::Current7B2930 | Whether its part (+0x48) is the current object (`0x12F56D4`) |
| `0x752680` | Guest::Mode752680 | In outline: with a slot (+0x10), a mode step (Person::Mode7CE320 on +0x20) |
| `0x6fc880` | Grid::Set6FC880 | In outline: when not flagged (+0x44), sets it and acts with a part (+0x48) |
| `0x9fbed0` | Fireworks::Value9FBED0 | In outline: with a shared value (`0x10EDB90`), half a value (+0x74) |
| `0x9e0d50` | Fireworks::Toggle9E0D50 | In outline: with a shared value (`0x1103B9C`), turns a flag (+0x54) on or off |
| `0x8fe220` | Terrain::Entry8FE220 | Gives the address of entry n (0x60 bytes at +0xA8, +0x10) |
| `0x8f96b0` | Terrain::Clear8F96B0 | Clears the terrain's value (+0x60) |
| `0x8e4d90` | Terrain::Clear8E4D90 | The same (another copy) |
| `0x8e0d90` | Terrain::Clear8E0D90 | The same (another copy) |
| `0x8df1a0` | Terrain::Children8DF1A0 | In outline: children (Window::ChildrenSlot4) and a step with a part (+0xF4, slot 0xFC) |
| `0x8d0010` | Scenery::Children8D0010 | In outline: children (Window::ChildrenSlot4) and sets a flag (Scenery::Set8CD5B0 with 1) |
| `0x88c760` | Scenery::Sign88C760 | In outline: sets active (SceneryBehaviour::SetActive) and a sign text (Sign::Text) |
| `0x88bf90` | Scenery::Inactive88BF90 | In outline: clears active (SceneryBehaviour::ClearActive) and a value (+0x58) |
| `0x83fc50` | Scenery::Call83FC50 | In outline: for an object, a call (its first slot) |
| `0x839170` | Scenery::Set839170 | In outline: sets a value (+0x100) |
| `0x837bf0` | Scenery::SetFlags837BF0 | Sets the given flags in entry n (+0x54) |
| `0x822c70` | SceneryBehaviour::Init822C70 | In outline: sets a value to -1 and makes an array of four (Scenery::Array821480) |
| `0x758850` | Guest::Set758850 | Clears a value (+0x548) and sets another (+0x54C) |
| `0x752650` | Guest::Mode752650 | In outline: a mode step (Person::Mode7CE320 on +0x20) with a part's value |
| `0x641280` | Path::Clear641280 | Clears a flag of the path manager (+0x96) |
| `0x99a250` | Fireworks::Set99A250 | In outline: sets a value (+0x224) and makes a record (Record::Make99A080) |
| `0x98fa40` | Park::Check98FA40 | Whether a pass (Park::Pass98C7F0) gives a value |
| `0x983730` | Record::Init983730 | In outline: an update list member (UpdateListMember) with its table (`0xF7F7B4`) |
| `0x919c60` | Waterfall::Flag919C60 | In outline: when flagged (bit 10 of +0x40), a waterfall manager step |
| `0x8f9690` | Terrain::Reset8F9690 | In outline: sets a value (+0x10C) and clears another (+0x68) |
| `0x8dc990` | Terrain::Part8DC990 | In outline: a step (Window::Slot48Self) and tells a part (+0x13C, slot 4) |
| `0x8718d0` | Scenery::Children8718D0 | In outline: children (Window::ChildrenSlot4) and acts with an object |
| `0x81e520` | SceneryBehaviour::Activate81E520 | In outline: unless flagged (bit 0 of +0x2C), activates (ManagedObject::Activate) |
| `0x80a650` | Regions::SetBit80A650 | Sets bit n of a value (+0xC) |
| `0x7c2120` | Person::Cell7C2120 | Whether a fence cell check fails (Fence::CellA906C0) |
| `0x772ed0` | Park::SetBit772ED0 | Sets bit n of a value (+0xE0) |
| `0x759670` | Guest::Random759670 | Sets a value (+0x430) to a random value times a fixed scale |
| `0x758370` | Guest::Add758370 | In outline: adds a scaled shared value (`0x10F24D0`) (Animal::AddA88370) |
| `0x7fafc0` | Person::Loop7FAFC0 | In outline: a step (slot 0xC) and a loop (Regions::Loop802F40) |
| `0x7357b0` | Path::Terrain7357B0 | In outline: frees (Path::Assign733560 with 0) and a terrain step (Path::Terrain734400) |
| `0x9f79a0` | Fireworks::SetEntry9F79A0 | Sets a value of entry n (0x40 bytes at +0xC, +0x20) |
| `0x9cbb80` | Particles::Has9CBB80 | Whether a part (+0x68) is set |
| `0x87b8f0` | Scenery::SetEntry87B8F0 | Sets a value of entry n (0x80 bytes at +0xC, +0x68) |
| `0x8e4b40` | Terrain::Mode8E4B40 | In outline: in mode 1 (+0x20), a pass (Terrain::Pass8E3A30) |
| `0x8dd6d0` | Terrain::Init8DD6D0 | In outline: sets its table (`0xF7C3E0`) and the terrain flattener |
| `0x8b6e60` | SceneryBehaviour::Aim8B6E60 | In outline: an aim step (SceneryBehaviour::Aim8B6A80) and clears a value (+0x90) |
| `0x87eec0` | Scenery::Colours87EEC0 | In outline: a flexi colour group (FlexiColourGroupCache::Get on +0xF8) |
| `0x9fc8f0` | Fireworks::Entry9FC8F0 | Gives a value of entry n (0x30 bytes at +0x54, +8) |
| `0x9cbb40` | Particles::Count9CBB40 | In outline: takes one from a count (+0x40) and acts |
| `0x9c5600` | Particles::Handle9C5600 | In outline: a step with a handle (`0xF848EC`) |
| `0x99aec0` | Fireworks::Free99AEC0 | In outline: sets its table (`0xF80C2C`) and acts with a part (+4) |
| `0x985050` | Record::Shared985050 | In outline: a shared step (Record::Shared981A10) with a handle (`0xF7F8BC`) |
| `0x93ce50` | Water::Rebuild93CE50 | In outline: when on (+0x2C), rebuilds (Water::Rebuild90DEA0) |
| `0x93c200` | Water::Next93C200 | Moves a counter (+0x30) on, wrapping to 0 after 1 |
| `0x833960` | Scenery::Init833960 | In outline: sets up a record (Record::InitBE79B0) with a callback |
| `0x827da0` | Scenery::Part827DA0 | In outline: a value with a part (+0x9C), none handled |
| `0x80a630` | Regions::SetBit80A630 | Sets bit n of a value (+0xC) |
| `0x807d00` | Regions::Init807D00 | In outline: sets up a record (Record::InitBE79B0) with a callback |
| `0x797630` | Terrain::Reset797630 | In outline: a step (slot 0x94) and a reset (Person::Reset7CF900) |
| `0x758910` | Guest::State758910 | In outline: unless in state 0xE (+0x190), sets that state (Person::SetState) |
| `0x746890` | Guest::Free746890 | In outline: tears down five entries (0x1C bytes at +8) |
| `0x730700` | Path::Count730700 | In outline: takes one from a count (+0xDC) and acts |
| `0x72f170` | Path::Set72F170 | Sets a value and a flag for whether it is set |
| `0x719f10` | Path::Set719F10 | In outline: a set step (Path::Set725D00 on +0x144) |
| `0x87bb00` | Scenery::FlagEntry87BB00 | Sets entry n's flag (0x80 bytes at +0xC, +0x65) |
| `0x846a50` | Scenery::Set846A50 | Sets a value (+0xA0) and acts (slot 0x60) |
| `0x8466d0` | Scenery::Init8466D0 | In outline: clears a flag and a record (`0x827510`) |
| `0x83fab0` | Scenery::Has83FAB0 | Whether a part (+0x9C) is set |
| `0x800d40` | Person::Loop800D40 | In outline: a module loop (Module::LoopBE5360) and copies a value (+0x10 into +4) |
| `0x7fb420` | Person::Check7FB420 | Whether n is 1 and the flag (bit 0) is set |
| `0x772b90` | Park::Random772B90 | Keeps two bits of a byte and puts in two random bits |
| `0x735fb0` | Guest::Slot735FB0 | In outline: with a part (+0xC4), a slot step (Scenery::Slot862F00) |
| `0x9f8b20` | Fireworks::Entry9F8B20 | Gives a value of entry n (0x24 bytes at +0x30, +4) |
| `0x9df9a0` | Particles::Device9DF9A0 | In outline: a device step (slot 0x94) |
| `0x9ceb80` | Particles::Pass9CEB80 | In outline: goes over entries (+0x84) |
| `0x988a40` | Record::Shared988A40 | In outline: with a shared object (`0x134131C`), tells it (its first slot with 1) |
| `0x9888f0` | Record::Value9888F0 | In outline: with a part's value (+4 +0xC), a value (+0x44) |
| `0x983960` | Record::ClearSlot983960 | Clears slot n (8 bytes at +0x20) |
| `0x940a90` | Water::Init940A90 | Clears two values |
| `0x8dc9b0` | Terrain::Init8DC9B0 | Sets its table (`0xF7C3E0`) and clears a value |
| `0x8db930` | Terrain::Init8DB930 | Sets two flags (+4, +5) |
| `0x8db730` | Terrain::Init8DB730 | The same (another copy) |
| `0x8db6e0` | Terrain::Init8DB6E0 | The same (another copy) |
| `0x8bb960` | SceneryBehaviour::Step8BB960 | In outline: a step (SceneryBehaviour::Step8B8B40) and sets a flag (+0xF0) |
| `0x846a90` | Scenery::Find846A90 | In outline: a find in a list (+0x3C) |
| `0x7e39f0` | Person::Table7E39F0 | Gives an entry (n, m) of a shared table (`0x10EFC88`, 17 wide) |
| `0x7c2950` | Person::Heading7C2950 | Sets a heading (+0x110) from a direction (atan2 of +0x70, +0x78) |
| `0x9eb340` | Fireworks::Random9EB340 | In outline: with a part (+0x9C), a random step (Fireworks::Random9EAFA0) |
| `0x9e3190` | Fireworks::Reset9E3190 | Clears two values (+0x58, +0xE0) |
| `0x9de570` | Particles::Count9DE570 | In outline: under 1, adds one to a count (+4) |
| `0x9d50b0` | Particles::Pass9D50B0 | In outline: with a part (+0xCC), a pass (Fireworks::Pass9C7510) |
| `0x8d3940` | Terrain::Pass8D3940 | In outline: a pass (Water::Pass8DA760) with a value (+0x7C) |
| `0x864a30` | Scenery::Pose864A30 | In outline: when flagged (bit 0 of +0x14), a pose (Scenery::Pose861050) |
| `0x7fafb0` | Person::Call7FAFB0 | In outline: calls (slots 0x1C, 0x20) |
| `0x47e1a0` | Labels::Table47E1A0 | In outline: with a part (+0xBC), a table step (Scenery::Table8A0710) |
| `0x9e8e00` | Fireworks::Entry9E8E00 | Gives entry n (by +0x80) of a shared table (`0x13419B8`, 0x1C bytes each) |
| `0x9d4ea0` | Particles::Reset9D4EA0 | Clears a flag (+0xC8) and a value (+0xC4) |
| `0x8cb700` | Scenery::Resize8CB700 | In outline: a resize (Array::Resize8CB5C0) |
| `0x8cb6f0` | Scenery::Resize8CB6F0 | In outline: the same (another copy) |
| `0x834180` | Scenery::Copy834180 | In outline: takes a reference to another's part |
| `0x7e3950` | Person::Flag7E3950 | Sets a flag (+0x77) |
| `0x771660` | Guest::Value771660 | In outline: reports a missing resource (Report::MissingResourceSymbol1) and gives a value |
| `0x7fafa0` | Person::Save7FAFA0 | In outline: saves a file (ManagedObject::SaveFile) |
| `0x7faf90` | Person::Load7FAF90 | In outline: loads a file quietly (ManagedObject::LoadFileQuiet) |
| `0x7b3a30` | Effect::Clear7B3A30 | Clears a value (+0x10) |
| `0x7b2950` | Effect::Draw7B2950 | In outline: a draw step (Effects::Draw7C0B80) |
| `0x9eb960` | Fireworks::Average9EB960 | Gives a total (Fireworks::Sum9F5440) over a value (+0x44) |
| `0x9eb8f0` | Fireworks::Clear9EB8F0 | In outline: a clear (Fireworks::Clear9EC600) |
| `0x83fd30` | Scenery::Slot83FD30 | Gives slot n of a table (+0x54) |
| `0x7f86c0` | Person::Value7F86C0 | Gives a part's value (+0x5BC +0x108) |
| `0x7f85e0` | Person::Value7F85E0 | Gives a part's value (+0x5BC +0xC0) |
| `0x7381d0` | Guest::Count7381D0 | In outline: takes one from a count (+0x74) and acts |
| `0x72f190` | Path::Init72F190 | Clears two bytes |
| `0x9e8df0` | Fireworks::Entry9E8DF0 | Gives entry n (by +0x6C) of a shared table (`0x13419B8`, 0x1C bytes each) |
| `0x9225a0` | Waterfall::Update9225A0 | In outline: unless flagged (bit 1 of +0x40), an update (Waterfall::Update) |
| `0x884520` | Buffer::Free884520 | Frees a buffer (+0xC) |
| `0x846de0` | Scenery::Kind846DE0 | Whether a part's kind (+0x20 +0x48) is not 0x2E |
| `0x905e70` | Buffer::Free905E70 | Frees a buffer |
| `0x905e10` | Buffer::Free905E10 | The same (another copy) |
| `0x724fd0` | Buffer::Free724FD0 | The same (another copy) |
| `0x966450` | Water::Init966450 | Sets its table (`0xF7E56C`) |
| `0x9655d0` | Water::Init9655D0 | Sets its table (`0xF7E4F8`) |
| `0x9653a0` | Water::Reset9653A0 | Clears two values (+0x38, +0x3C) |
| `0x800770` | Person::Flag800770 | In outline: sets a flag (bit 1 of +0x2C) and a call (slot 8) |
| `0x9260b0` | Water::Count9260B0 | Adds one to a count (+0x364) |
| `0x90c700` | Water::Diff90C700 | Gives a value (+4) less another (+0x10) |
| `0x827790` | Scenery::Shared827790 | Sets a shared value (`0x130B5B0`) |
| `0x966d50` | Water::Init966D50 | Points a value (+0x10) at itself |
| `0x9cbb30` | Particles::Count9CBB30 | Adds one to a count (+0x40) |
| `0x9c60a0` | Particles::Count9C60A0 | Adds one to a count (+0x48) |
| `0x7381c0` | Guest::Count7381C0 | Adds one to a count (+0x74) |
| `0x76dff0` | Park::Same76DFF0 | Gives its argument back |
| `0x7e1620` | Person::Build7E1620 | In outline: makes a part (`0xBCDFC0`); with none, takes a reference to a handle (+0xAC), builds a window part (Window::Build6456F0) into +0x68 and makes a scenery part (Scenery::Make61CF40) |
| `0x79f8e0` | Terrain::Init79F8E0 | In outline: sets up a handle link (Handle::Null) joined into its list, a second link and clears three values |
| `0x9d4a80` | Particles::Remove9D4A80 | In outline: finds an entry by either of two texts (Text::FindBCE9D0) and removes it, moving the last one in (Ref::Assign9D0CE0, ObjectArray::SetCapacity) |
| `0x8b2570` | SceneryBehaviour::Init8B2570 | In outline: sets up a handle link (Handle::For), its table (`0xF2CD44`), a flag and the game time's value (+0x48) |
| `0x949210` | Water::Entry949210 | In outline: for entry n (12 bytes at +0x160, under +0x164) marked stale, frees its object and finds it again (Water::Find946FC0); gives the object |
| `0x844800` | Scenery::Buy844800 | In outline: when money allows (Money::CheckCost on +0x2C), clears every slot (+0x24, count +0x28) and gives 1; else, unless asked, shows the no money message (Money::ShowNoMoney) |
| `0x81faf0` | Scenery::Link81FAF0 | In outline: whether two of four sides n and m link: equal, or m not next to n and a cached part of m's entry (0x1C bytes at +0x6C) by a shared kind (`0x130B540`) allows it |
| `0x7fa670` | Person::Init7FA670 | In outline: an object with kind text "*:ent", its tables (`0xF7510C`, `0xF6C51C`) and an empty text reference |
| `0x7577e0` | Guest::Init7577E0 | In outline: the same with its table (`0xF6C504`) |
| `0x9c69d0` | Particles::Done9C69D0 | In outline: 0 while a count (+0x9C) is above 0; else whether every entry (count +0xD0) passes a check |
| `0x9db100` | Fireworks::Make9DB100 | In outline: a random value (Random::Ran0) and makes a part (Window::Make9DAF50) |
| `0x9d0a90` | Particles::Add9D0A90 | Adds a value at the end of a list (+0x214, count +0x218), growing it first (`0x9CE530`) |
| `0x9eaf30` | Fireworks::Kind9EAF30 | In outline: a kind step (Firework::Kind9EA680) with a part (+0x70); gives its argument |
| `0x9eaf10` | Fireworks::Kind9EAF10 | In outline: the same with +0x5C |
| `0x7446d0` | Guest::Describe7446D0 | In outline: writes a record with "Version ", "Name " and "Leader " entries and describes each avatar entry (AvatarDatabaseEntry::Describe) |
| `0x49b050` | Park::Group49B050 | Sorts a kind code (0-255) into one of three groups for the park list (Park::List668DE0): through a byte table (`0x49B088`) and a four-way jump table, class 1 gives 1, class 2 gives 0, and class 0 or 3 (and any code above 0xFF) gives 2. Read from the machine code, as the decompiler cannot handle it |
