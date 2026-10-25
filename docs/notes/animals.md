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
