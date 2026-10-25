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
