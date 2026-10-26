# Guests

Guests (people visiting the park). More on how they choose rides is in [the research notes](../research-physics-and-guests.md).

| Address | Name | What it does |
|---|---|---|
| `0x765990` | Guest::Update | A guest's main update (virtual slot 47 of Guest, also used by VIP and BodyGuard): runs the guest's current state (+0x190) and its timers, needs and decisions, using the park (`0x1306194`) and the random number generator (28 calls); also where the name cheats take effect (it checks the guest's name against `thirsty bob`, `vomit man`, `flameboy`, `lobster face`, `slow burner`, `happy camper`, `sad panda` and `make like a tree`; `ParticleEffects\PurpleCloud.frw` is one of their effects) |
| `0x7c8b10` | Person::StateName | Writes the name of a person's current state for display (virtual slot 36 of Person, used by guests and all staff): `Queueing - Start`, `- PushingIn`, `- Waiting`, `- Paying`, ..., `Swimming - Idle`, `- Shower`, `- Diving`, `- SunLounger`, `- Splashing Someone`, `HighDive - ...` and the other states, with seven switches over state and sub-state |
| `0x7e1490` | Person::UpdateState | Runs a person's current state (virtual slot 39 of Person, used by every kind of person): the special states from 0x101 up (state at +0x190) go to their own handlers (0x101 to virtual slot 41, 0x102 to `0x7D68E0`, 0x103 to `Person::UpdateSwimming`, 0x105 to `0x7D2D00`, 0x106 to `0x7D32D0`); the ordinary states are run here |
| `0x7d89e0` | Person::UpdateSwimming | A person in the water (state 0x103): counts down its timers, faces the direction it moves in (`atan2` of its velocity), finds its place in the pool (`0xAC5FA0`, `0xAC6870`), plays the shower sound (`ShowerLoopFader`) and makes splashes (`WaterManager`); `0x7D0520` does the swimming movement. Called from `Person::UpdateState` |
| `0x7d0520` | Person::Swim | Moves a swimming person (called from `Person::UpdateSwimming`): finds the pool the person is in through its pool interface (id `0x13CE41C`), then moves it by its swimming sub-state (a 10-way switch: travelling, queueing, showering, diving, sun loungers, ...), returning a float to its caller |
| `0x49bd10` | Thought::Text | Writes the text of a person's thought: looks up its text key (`%s%d`, or `Thought_Unknown`) and fills in the person's name (`%NAME%`) and a random member of their group (`%RANDOMGROUPMEMBERNAME%`, picked with the random number generator). Called from `Person::UpdateState` and four others |
| `0x7e2e60` | Guest::DescribeCounts | Writes how many rides a guest has been on and how many food items they have had: none, 1 to 4, or many (`Guest_0Rides` ... `Guest_ManyRides`, `Guest_0FoodItems` ... `Guest_ManyFoodItems`). Called from ten places in the guest windows |
| `0x7a4a90` | GuestStats::Fill | Fills in the guest statistics: the numbers of guests and groups and the groups with ... (`GuestStats_NumGuests`, `_NumGroups`, `_GroupsWith`). Called from `0x7875D0` |
| `0x7b9ee0` | Crowd::UpdateMurmur | Plays the murmur of the crowd: picks among `Murmur1L` / `R` ... `Murmur3L` / `R` (left and right) by how many people are around and where, at random. Called from `0x7BAE80` |
| `0x7d47d0` | Person::Vomit | Makes a person be sick: the `Vomit` effect (random), its puddle (with `PeopleShadowBlob`). Called from `0x7E9B50` |
| `0x75cdf0` | Guest::StateName | Writes the name of a guest's state (virtual slot 36 of Guest): its own following states (`Follow - Wait`, `- Follow Leader`, `- Flock`, `- Set Piece : Start` / `Travel` / `Gather` / `Play`), the group's state (`0x7824C0`) and otherwise the person's (`Person::StateName`) |
| `0x7824c0` | GuestGroup::StateName | Writes a guest group's state: loved up, wobbly, taking a photo, play fighting, heading for an attraction, on a ride, split up (`GuestGroup_State_*`). Called from `Guest::StateName` and `0x7E3C60` |
| `0x7c7de0` | Person::TypeName | Writes a person's type: `Child`, `Teenage`, `Adult`, `VIP`, `Handyman`, `Mechanic`, `Security Guard`, `Entertainer`, `Vendor`, `Animal Keeper`, `Paramedic`, `ParkInspector`. Called from `0x7CD9B0` |
| `0x7f55a0` | Vendor::Sell | A vendor selling: the till sound (`VendorKerching1` / `2`, at random) and the money going to the park's finances |
| `0x7ebd40` | Staff::CheckResign | Checks whether a staff member wants to resign, and resigns them, with the messages `StaffMessage_WantsToResign` and `StaffMessage_Resigned` |
| `0x742e50` | AvatarDatabaseEntry::Describe | Writes an avatar's description for the developer: member, type, sub-type, part, palette, swimsuit and ride intensity. Called from `0x7446D5` |
| `0x7adde0` | ParkInspector::Thoughts | Makes the park inspector's complaints: no maintenance staff near, a ride with no exit, no entrance, no path to the entrance or exit, inappropriate music, ... (`Thought_PI_*`). Called from `0x7A8DA0` |
| `0x4737b0` | PhotoManager::Message | Tells the player a photo was taken: of a coaster, a group at a vista point, a group, or any (`PhotoManagerMessage_*`). Called from `0x473F30` |
| `0x7e4940` | Pincer::Load | Loads the pincer that picks guests up (`Characters\Pincer\Claw`, `Claw`, `Peep01`). Called from `0x7B1BA0` |
| `0x472000` | PhotoSpot::Name | Writes a photo spot's name (`Custom group` / `photo spot` ` at ` ...) |
| `0x473c30` | Photo::Name | Writes a photo's name (`Custom guest` / `coaster` ` on ` ...) |
| `0x75e4c0` | Guest::NameCheats | Applies the guest name cheats `hungry joe` and `mustafa p`. Called from `Guest::Update` |
| `0x7fec50` | Person::PartGroupNames | Makes the names of the saved person part groups (`HeadPartGroup`, `BodyPartGroup`, `LegsPartGroup`, `SwimSuitBodyPartGroup`, `SwimSuitLegsPartGroup`). Called from `0x7FFDF0` |
| `0x7bfff0` | Person::CheckParts | Reports a person's model parts per level of detail: fixed parts unresolved, optional parts missing (`Main`, `SwimSuit`). Called from `0x7BD4A0` |
| `0x7446d5` | AvatarDatabaseEntry::Write | Writes an avatar's version, name and leader for saving |
| `0xed8d00` | HandymanTools::Mower | Makes the handyman's lawnmower tool at start-up (`People\HandyMan\Tools`, `MoterMower`, `Lawnmower`) |
| `0xed8b90` | HandymanTools::Sponge | Makes the handyman's sponge tool at start-up (`People\HandyMan\Tools`, `Spunge`, `Sponge`) |
| `0x7f92c0` | VIP::ProblemMessages | Makes a VIP's complaints: an attraction closed, broken or crashed, can't get to a waypoint (`VIP_*`). Called from two places |
| `0x761b30` | Guest::PandaCheat | The `panda` guest name cheat, with random effects. Called from `0x76A450` |
| `0x769df0` | Guest::DebugPosition | Writes a guest's grid position for the developer (`Col: `, `, Row: `). Called from `0x76D890` |
| `0x769180` | Guest::Laugh | Makes a guest laugh (`Male laugh`, `Female/child laugh`), at random |
| `0x7f95b0` | VIP::Arrive | Brings a VIP into the park and announces it (`VIP_ArrivedInPark`). Called from `0x6F7330` |
| `0xa673d0` | Person::DrawShadow | Draws a person's shadow blob (`PeopleShadowBlob`) and updates their lights |
| `0x5f0b70` | Person::LookAt | Turns a person's head to look at something (`LookAt`), at random intervals. Called from two places |
| `0x779f30` | GuestGroup::DebugWants | Writes what a guest group wants, for the developer: home, ATM, first aid, toilet, food, drink, umbrella, sun block, bench, bin (`Group's desired objects (NOT LOCALISED):`). Called from `GuestWindow::AvatarHighlight` |
| `0x40aa60` | BodyGuard::Create | Creates a VIP's bodyguards (`BodyGuard`). Called from `0x6F7330` |
| `0x745070` | AvatarGroup::Leader | Reads which avatar leads a group (`Leader`). Called from two places |
| `0x7c4970` | Person::FollowPath | Moves a person along their path for a time step: first lets the thing they are on move them (an interface's slot 0x24), otherwise walks them from point to point of their route (16-byte points at +0x198, current one at +0x1A0 of +0x19E), the distance being the time step times their speed (+0x118), keeping them on the terrain's height (`Terrain::Get`, `0x8BCC90`). Called from 17 places (staff and guests) |
| `0x7f6540` | ViewingGalleryPersonInfo::Update | Moves a guest through a viewing gallery for a time step (virtual slot 21 of the attraction person infos; returns whether they arrived): by their sub-state (+0xB4), walks them along the gallery's splines (`0xBAE8E0`, `0xBAE740`) at their walking speed (+0x118 of the person) towards their viewing slot, waiting for their animation to finish (animation lengths at `0xFC35D8`) |
| `0x5eef60` | WalkAlongTrackPersonInfo::Update | Moves a guest along a walk-along-track attraction for a time step (virtual slot 21), a 10-way switch over their sub-state, using the person (`0x4D1840`) and moving them towards their next position (`0x4D1C30`) |
| `0x4eb650` | MiniGolfPersonInfo::Update | Moves a guest through a mini golf course for a time step (virtual slot 21): walking on, playing each hole's strokes (`MiniGolfHole::SetupStrokes`) and walking off, a 13-way switch over their sub-state |
| `0x4d1840` | AttractionPersonInfo::Person | Returns the person an attraction person info belongs to. Called by all the attraction person infos |
| `0x4d1c30` | AttractionPersonInfo::MoveTowards | Moves the person towards a position for a time step, returning whether they got there. Called by all the attraction person infos |
| `0x75f380` | Guest::ReactToAttraction | A guest's reaction to an attraction they consider (virtual slot 76 of Guest, VIP and BodyGuard; only now and then, by a random check): compares the attraction's excitement, intensity and nausea (+0x230, +0x234, +0x238) with the guest's preferences (+0x41C, +0x438 / +0x43C) and has them think about it (thoughts 0x6E / 0x6F through virtual slot 0x38), and compares its price (the attraction's slot 0x84) with what the guest will pay, unless the park's finances are set to free (`0x1305DD8`, +0x68) |
| `0x7c22c0` | Guest::GenderName | Name of a guest's gender (in eax; used by Person::TypeName): 0 `Male`, 1 `Female`, otherwise `<unknown Guest>` |
| `0x7c2970` | Person::SwimToward | Moves a swimming person toward a point, stopping a given distance short of it: steps along the direction to it (or the person's facing +0x70 when within a small distance) by the remaining distance, limited to the given step either way, moving the position at +0x80; returns 1 when the full step was taken, 0 when it arrived. Called from Person::Swim |
| `0xa883a0` | Guest::Band | Which band a value falls in once divided by the unit in `0x13CDF20`: below 1 gives 0, below 2 1, below 3 2, below 4 3, below 5 4, otherwise 5. Called from Guest::Update |
| `0xb9ab80` | Guest::EntryRemaining | The part still left of entry n of the table at +0x74 (184 bytes each, from +0x20): 1 - used (+0x14) / total (+0xC), at least 0, when the entry is in use and has a total; 1 otherwise. Called from Guest::PandaCheat |
| `0x7c2f40` | Person::FaceDirection | Turns a person to face along a direction (atan2 of its x and z, through `0x7C2D40`) when its flat length is above a small limit. Used by Person::Swim and Person::UpdateSwimming |
| `0x773760` | GuestGroup::AnyMemberFlag404 | Whether any member of the group (+0x20, 20 bytes each, count +8; each through its interface `0x130654C`) has its flag at +0x404 set. Slot 4 of GuestGroup |
| `0x773a60` | GuestGroup::AnyMemberCount45C | Whether any member of the group has a positive count at +0x45C. Slot 10 of GuestGroup |
| `0x774830` | GuestGroup::AnyMemberSlot16 | Whether slot 16 of any member's part at +0x400 is true. Slot 16 of GuestGroup |
| `0x7748b0` | GuestGroup::AnyMemberSlot5And16 | Whether any member's part at +0x400 answers true to slot 5 with the given argument and also to slot 16. Slot 15 of GuestGroup |
| `0x774940` | GuestGroup::MostOfMembers | The largest value slot 17 of the members' parts at +0x400 gives (0 when there are none). Slot 17 of GuestGroup |
| `0x7749e0` | GuestGroup::AverageOfMembers4B0 | The average of the members' values at +0x4B0, or 1 when the group is empty. Slot 18 of GuestGroup |
| `0x774a80` | GuestGroup::AverageOfMembersSlot20 | The average of what slot 20 of the members' parts at +0x400 gives, or 0.5 when the group is empty. Slot 20 of GuestGroup |
| `0x774b20` | GuestGroup::AverageOfMembersSlot21 | The average of what slot 21 of the members' parts at +0x400 gives, or 0 when the group is empty. Slot 21 of GuestGroup |
| `0x7e9b50` | Staff::CountDownTimer | Counts the staff member's timer (+0x418) down by the frame time; when it runs out it starts again 16 seconds later and the member's slot 88 is called; then Person::Vomit's update for the frame. Slot 7 of the staff classes (animal keeper, entertainer, handyman, lifeguard, ...) |
| `0x7e9ee0` | Staff::ResetSlot70 | Calls `0x7C3850`, then clears +0x410 and +0x4D8 and sets +0x4DC to 0.5. Slot 70 of the staff classes |
| `0x7c3850` | Person::ClearSlot70 | Clears the person's values at +0x310, +0x2E8, +0x2DC, +0x2E0 and the flag at +0x314. Slot 70 of Person |
| `0x7d56c0` | Person::Slot48 | Calls `0x7CE430` with -1. Slot 48 of Person |
| `0x754fd0` | Entertainer::ScaleSlot55 | Sets +0x118 to +0x114 times the entertainer type's factor for its current animation (`0xFC4288` by +0xB8, from `0x7578C0`), or times 1 when the type has no such entry. Slot 55 of Entertainer |
| `0x756ab0` | Entertainer::UpdateTimer | Runs the entertainer's update for the frame (`0x756790` in state 0xD, otherwise `0x7EEC00`), then counts its timer (+0x500) down by the frame time. Slot 39 of Entertainer |
| `0x797230` | Handyman::SetSpeedFactor | Sets the handyman's factor at +0x114 by his level (+0x414): 1.0 at level 1, 1.1 at 2, 1.2 at 3, 0.9 otherwise; then `0x7D63E0` with the argument and more for a handyman with a task (+0x51C). Slot 40 of Handyman |
| `0x799750` | LifeGuard::Slot37 | In state 0xD: back to state 0 (Person::SetState 0, 0), clearing +0x1A4, and returns 1; otherwise `0x7ECD90`. Slot 37 of LifeGuard |
| `0x7a0e50` | Mechanic::Slot37 | In states 0xE, 0xF or 0x14: `0x7A0000` and clears +0x1A4, returning 1; otherwise `0x7ECD90`. Slot 37 of Mechanic |
| `0x7eec00` | Staff::UpdateState | In state 0xC runs `0x7EC8F0`; in any other state Person::UpdateState. Shared by the staff classes (slots 39, 40, 47) |
| `0x7e8fc0` | SecurityGuard::Slot37 | In state 0xD: back to state 0 (Person::SetState 0, 0), clearing +0x1A4, and returns 1; otherwise `0x7ECD90`. Slot 37 of SecurityGuard |
| `0x7418a0` | AnimalKeeper::SetSpeedFactor | Sets the keeper's factor at +0x114 by his level (+0x414): 1.1 at level 1, 1.2 at 2, 1.3 at 3, 1.0 otherwise; then, in state 0xD, more work (`0x73EE90`, ...). Slot 39 of AnimalKeeper |
| `0x756b00` | Entertainer::StateName | The name of the entertainer's state: in state 0xD the text kept in `0x12FB234`, otherwise Person::StateName. Slot 36 of Entertainer |
| `0x799ff0` | LifeGuard::StateName | The name of the lifeguard's state: in state 0xD the text kept in `0x12F8FC0`, otherwise Person::StateName. Slot 36 of LifeGuard |
| `0x7f8ef0` | VIP::StateName | The name of the VIP's state: in state 0x16 the text kept in `0x12F98B8`, otherwise Guest::StateName. Slot 36 of VIP |
| `0x7f8810` | VIP::Slot37 | In state 0x16: back to state 0 (Person::SetState 0, 0), clearing +0x1A4, and returns 1; otherwise `0x761440`. Slot 37 of VIP |
| `0x7a0e90` | Mechanic::SetSpeedFactor | Sets the mechanic's factor at +0x114 by his level (+0x414): 1.0 at level 1, 1.1 at 2, 1.2 at 3, 0.9 otherwise; then `0x7D63E0` with the argument and more. Slot 40 of Mechanic |
| `0x7cd220` | Person::SetState | Sets a person's state (+0x190) and sub-state (+0x194); while the person's debug text is on (+0x29E) it also keeps the state's text (+0x2A0, made by `0x7C9C60`) up to date, refreshing the debug display (`0x6206F0`) when it changes |
| `0x7ecd90` | Staff::ReturnToIdle | Slot 37 of the staff classes: in state 0xC, when the staff member has a task object (+0x1C0) and a sub-state above 1, releases each of the task's references (`0x75A550`, through its interface `0x130B4D0`); then back to state 0 (Person::SetState 0, 0) with +0x1A4 cleared, returning 1. In other states Person's own slot 37 (`0x7CF960`) |
| `0x75a550` | Guest::ReleaseReference | Makes reference n of the table at +0x64 (96 bytes each, from +0x40) refer to nothing: takes it out of its handle's list, releases the handle (`0x8055C0`) and joins the null handle's list, then notifies it |
| `0x7c9c60` | Person::DebugStateText | Builds the text shown for a person's state on the debug display: its state name (slot 36) with the state and sub-state numbers between separators |
| `0x758fe0` | Guest::PickValue318 | Sets the guest's value at +0x318: normally 0x23 (`0x7C3470`), but by chance (Random::Ran0) 19 (8 in 100), 36 (7 in 100) or, for a guest whose +0x120 is 0, 0xAC (5 in 100). Slot 79 of Guest, BodyGuard and VIP |
| `0x759db0` | Guest::ResetSlot70 | Person::ClearSlot70, then a random value from 0.35 to 0.65 at +0x408 (Random::Ran0), `0x7594E0`, copies of +0x408..+0x414 into +0x418..+0x424, `0x759570` and `0x7595F0`, and a random value from 0 to 0.2 at +0x430. Slot 70 of Guest, BodyGuard and VIP |
| `0x7ad730` | ParkInspector::Slot53 | One time in ten (Random::Ran0) tries `0x7AD360` first and stops if it succeeds; otherwise Person's slot 53 (`0x7DAD70`). Slot 53 of ParkInspector |
| `0x758b10` | Guest::Group | The guest's group: the interface (id in `0x1306920`) of the object its reference at +0x4F0 points to, or null (used before GuestDetails::BuildGroupTabs and by group members) |
| `0x773710` | GuestGroup::Member | Member n of a group (+0x60, 20 bytes each): the guest interface (id in `0x130654C`) of the object it refers to, or null |
| `0x7c31e0` | Person::SetAnimation | Starts an animation: unless forced, only when the person's slot 51 allows it; sets the animation (+0xB8), its time to 0 (+0xBC) and the flag at +0x1E0, clearing +0x398 for animation 0x2A |
| `0x7c2f90` | Person::UpdateGroundHeight | Sets the person's height (+0x84) to the ground height at their position (`0x7C22E0` with the terrain grid and the path manager) |
| `0x46d730` | Text peep%02d | Makes a text from the format `peep%02d` with the number given plus one |
| `0x4aabd0` | Text Peep%02d | Makes a text from the format `Peep%02d` with the number given plus one |
| `0x4aac60` | Text InspectionPoint%02d | Makes a text from the format `InspectionPoint%02d` with the number given plus one |
| `0x4aacf0` | Text LookAt%02d | Makes a text from the format `LookAt%02d` with the number given plus one |
| `0x5a9360` | Text InspectionPoint%02d | Makes a text from the format `InspectionPoint%02d` with the number given plus one |
| `0xa97770` | Text Peep%02d | Makes a text from the format `Peep%02d` with the number given plus one |
| `0xbaddd0` | Text PeepBottom%02d | Makes a text from the format `PeepBottom%02d` with the number given plus one |
| `0xbade60` | Text PeepTop%02d | Makes a text from the format `PeepTop%02d` with the number given plus one |
| `0x7bd4a0` | Person::CheckCharacterParts | Prepares a person's body part (entry +0x54 of the table at +0x14, 36 bytes each) and checks the character's parts (Person::CheckParts); when they are broken, shows a message box `Broken Character Resource` with `Body Part: %s` and the errors |
| `0x7816f0` | GuestGroup::StateText | The text for a guest group's state, such as the game text `GuestGroup_State_Wandering` |
| `0x798650` | Handyman::Sweep | In outline: the handyman's sweeping: walks the path (Person::FollowPath), faces his work and uses the `Brush` tool, then changes state |
| `0x798be0` | Handyman::Water | In outline: the handyman's watering: goes to the flower bed (its scenery behaviour's owner) and uses the `WaterCan` tool, then changes state |
| `0x7cc440` | Person::MadeWet | In outline: when a person is made wet (`MadeWet`), sets the time and reaction at random (Random::Ran0, the game clock) |
| `0x7cbfd0` | Person::Splash | In outline: makes a person's splash sound (`PeepSplash`), chosen at random |
| `0xbaeb30` | Person::InspectionPoint | In outline: the `InspectionPoint01` point name |
| `0x490880` | WayPoint::Name | In outline: makes way points named `Way Point ` + number |
| `0x77c090` | Guest::Update77C090 | In outline: one of a guest's state updates by its mode (+0xE4): checks its group (`0x7742B0`), plays crowd sounds (Crowd::Sounds) at random and acts on what it is next to (`0x7B9AA0`) |
| `0x73ee90` | Staff::UpdateInspector | In outline: a staff member's update by its stage (+0x194, up to 5): sets its state (Person::SetState), follows its path (Person::FollowPath) and, for inspectors (kind 0xE), works with the park inspector manager |
| `0x780070` | GuestGroup::Update780070 | In outline: a group's update by its state (+0x44): keeps its member count (+0x48) in step, sometimes (Random::Ran0) splits or regroups (`0x77CC80`) and goes over its members |
| `0x763440` | Guest::UpdateAction763440 | In outline: a guest's update while doing an action (state 0x11, stages 0 to 4 at +0x194): faces the right way (Person::FaceDirection), plays its steps (`0x8B0350`, `0x8B23B0`, `0x8B6E60`) and sometimes moves on at random |
| `0x780b90` | GuestGroup::Update780B90 | In outline: a timed group update inside a critical section: looks through the members (+0x48) for one in state 0x10, stage 2, and acts on the group's target (+0x88) when the time (+0xBC) is reached |
| `0x7cb120` | Person::UpdateTarget7CB120 | In outline: a person's update of its target by kind (+0x350: 1 to 3): looks up the thing it is heading for (`0x1309020`, `0x12F4C64`), works out the way and distance to it and moves on (`0x7CADD0`) |
| `0x77cc80` | GuestGroup::Split77CC80 | In outline: whether a group can split here: when it has more than one member, finds the path tile under it (PathManager, `0x71B370`) and under its next point (`0x8B0320`, `0x8B0350`) |
| `0x73cc10` | Staff::UpdateWork73CC10 | In outline: a staff member's work update for stages 0xB and up (+0x194): sets its state (Person::SetState), does the step (`0x73BBD0`) and checks its box against what it works on (`0xBCC740`) |
| `0xbadef0` | Peep::SetupParts | In outline: goes over a peep model's parts (+0x1D4, count +0x1D8), naming their top and bottom halves (`PeepTop%02d`, `PeepBottom%02d`) and setting up the ones not yet done |
| `0x763f80` | Guest::Update763F80 | In outline: a guest's update in one of its states: sets its state (Person::SetState), checks the things it is linked to (by their kinds) and, in state 10 stage 0 or stage 2, moves on |
| `0x7b1ba0` | Guest::Pickup7B1BA0 | In outline: picks up a guest with the hand tool (Pincer::Load) when nothing is held (+0x130), unless it is already the one held or flagged (+0x176, +0x148) |
| `0x73fa60` | Staff::Update73FA60 | In outline: a staff member's update: its work (Staff::UpdateWork73CC10) first, then for stage 6 its next task (`0x73E6B0`) |
| `0x782d10` | GuestGroup::Spawn782D10 | In outline: makes new guests for a group now and then (Random::Ran0, when +0x1A1 is set): places them (Guest::Place777990 with 4000) and adds them (`0x7778E0`, `0x77B750`) |
| `0x7c9f50` | Person::FindTask7C9F50 | In outline: goes over the tasks of a list (+0xD0) and picks one for the person (`0xA89450`, `0x49C4A0`) |
| `0x7aae80` | Person::FindTask7AAE80 | In outline: another task search like Person::FindTask7C9F50 for a person, from the place it is (`0x7AD8A0`) |
| `0x75d9e0` | Guest::OnRide75D9E0 | In outline: a guest's update while on an attraction (state other than -1, kinds 0x14 and 0x1C handled): checks its attraction (AttractionPersonInfo::Attraction) |
| `0x7ab810` | Person::Route7AB810 | In outline: works out a route for a person to its target: the path manager (made when needed) and the target's place |
| `0x775b30` | Guest::Watch775B30 | In outline: a guest's update while watching or waiting: checks the thing it is linked to (state 0x10 stage 2, Interface1F0::Get) and turns toward it (`0xCDF020`) |
| `0x73da00` | Staff::Update73DA00 | In outline: a staff member's update by stage (+0x194: 6 handled): finds its next task within 100 (`0x79B9F0` with 0x20) from its place (+0x52C) |
| `0x75c3b0` | Guest::Update75C3B0 | In outline: a guest's update in one of its states (0x12 handled): sets its state (Person::SetState), checks what it is linked to (`0x7F8360`) and uses the path manager |
| `0x76d890` | Guest::Update76D890 | In outline: a guest's timed update (while +0x4A0 is above 0, not flagged +0x4D0): checks the thing it is linked to (+0x4CC, Interface50::Get) and its next step (`0x772F40`) |
| `0x76d240` | Guest::PickSpot76D240 | In outline: picks a random spot for a guest within a cell area on the terrain (a 90 by 90 grid, Random::Ran0, `0x4751E0`) |
| `0x77b820` | GuestGroup::Pick77B820 | In outline: half the time (Random::Ran0), picks something for a guest of a group to do (`0x784700`) when it is free (+0x334) |
| `0x7790f0` | GuestGroup::Update7790F0 | In outline: a group's update by its state (+0x44: 2, ...): with two members, checks the first (GuestGroup::Member) is in state 0x10 stage 2 and free (+0x3EC bit 28) |
| `0x7cfee0` | Person::HandleEvent7CFEE0 | In outline: a person's handling of an event by its number (0x100 and below): sets its state (Person::SetState(2, 0)) when not flagged (+0x2D4) |
| `0x75fa30` | Guest::TryAction75FA30 | In outline: whether a guest can do something now: when free (+0x334), checks the target (`0x784840`, `0x783B30`) |
| `0x7d59c0` | Person::Swim7D59C0 | In outline: checks the water under a person (WaterManager::Get, `0x93E7D0`) and, when not flagged (+0x3C0) and in stage 2, acts on it |
| `0x7ac710` | Crowd::Update7AC710 | In outline: updates the crowd sounds (Crowd::Sounds, `0x7B5010`) by the sizes of three groups of people (+0x548 to +0x550) |
| `0x7fa020` | Guest::Need7FA020 | In outline: a guest's need check when not flagged (+0x444): compares a value (+0x5BC +0x144) with four times another (+0x310) scaled, acting on it (`0x75E740`) |
| `0x77faf0` | Guest::OnRide77FAF0 | In outline: a guest's update on a ride it is on (+0x118 +0xC): when the ride kind allows it (`0xFC4C78`), moves along (`0x7C6280`) by its mode (+0xE4, +0xE8) |
| `0x76010b` | Guest::Inspector76010B | In outline: part of a guest or staff update for kind 0xE in stage 2, acting by cases 1, 0x1E and 0x3C (register-passed object) |
| `0x769780` | Guest::TryRide769780 | In outline: whether a guest goes for something: checks it can (Guest::TryAction75FA30), its values (+0x40C) and acts |
| `0x73c310` | Staff::Idle73C310 | In outline: a staff member's idle update: with jobs waiting (+0x518) resets its state (Person::SetState), otherwise checks the cell it is on (Grid::CellAt) |
| `0x76b260` | Guest::Shop76B260 | In outline: a guest's update at a shop (+0x2B8): sets its state (Person::SetState) and, in state 0x200, buys (`0x46C7A0`, `0x7C3100`, `0x46C7C0`) |
| `0x7ce8b0` | Person::BoardCar7CE8B0 | In outline: a person boarding a ride car: finds its seat (RideCar::PeepSlot) and moves to it (`0x7C6280`) when not flagged (+0x2D4) |
| `0x75b160` | Guest::FindPath75B160 | In outline: finds where a guest goes next on the paths (`0x1306960`, `0x786AD0`, `0x7089E0`, PathTile::Centre, `0x7C9E60`) |
| `0x7c5cb0` | Person::Wander7C5CB0 | In outline: unless busy (`0x7C3480`), sometimes (Random::Ran0, by a rate) turns a person to wander |
| `0x7aa230` | Staff::Inspector7AA230 | In outline: an inspector's update (kind 0xE stage 0): finds where to go (`0x7C9E60`) when it has no place (+0x52C = -1) |
| `0x7e93d0` | Guest::PickRandom7E93D0 | In outline: picks a random thing for a guest of kind 0 or 1 over the terrain (a 90 by 90 grid, Random::Ran0, `0x7C2FF0`) |
| `0x79eb70` | Guest::PickRandom79EB70 | In outline: the same random pick as Guest::PickRandom7E93D0 (another copy) |
| `0x77abe0` | GuestGroup::Setup77ABE0 | In outline: sets up a group (`0x779D20`, `0x76DFF0` into `0x1306554`) and, when it can (`0x76F0D0`), its members (`0x777660`) |
| `0x76f0d0` | GuestGroup::CanPlace76F0D0 | In outline: whether a group can be placed: checks the terrain under it |
| `0x776600` | Guest::LookAt776600 | In outline: whether a guest looks at a thing: the same one as its target (+0xAC), turned by its angle (+0x70) |
| `0x7771f0` | Guest::Update7771F0 | In outline: a guest's update when not flagged (+0x1BC) with a target (+0x1B8), in state 0xB (+0x7C) and others |
| `0x72e7c0` | Path::Refresh72E7C0 | In outline: refreshes a path piece's parts (`0x42E8B0`) unless flagged (+0x8C) |
| `0x7f4f10` | Guest::PickRandom7F4F10 | In outline: picks one of two things at random for a guest of kind 0 or 1 (Random::Ran0) |
| `0x760810` | Guest::PathTile760810 | In outline: finds the path tile under a guest (PathManager::Get, `0x71B370`) when it has a target (+0x4CC) |
| `0x75b690` | Guest::GroupCheck75B690 | In outline: when flagged (+0x4B4) and not in stage 4, checks the guest's group (Park::Get, Guest::Group, GuestGroup::Member) |
| `0x75ef80` | Guest::Decide75EF80 | In outline: a guest's choice: finds something (`0x79C0A0`) and checks it (`0x49B610`) |
| `0x75e0d0` | Guest::Mood75E0D0 | In outline: unless in state 0x10 stage 2, now and then (Random::Ran0, by its rate +0x418) changes a guest's mood when it has more than two of something (+0x120) |
| `0x776200` | GuestGroup::HoldHands | In outline: makes a hold-hands route for a group (HoldHandsRouteController) and starts it (`0x7E5190` on +0x198) when it has at least two members |
| `0xa27600` | Guest::MoveA27600 | In outline: moves a guest by a time (`0x752680`) and, in state 10 with a target (+0x1C0), checks it |
| `0x7acca0` | Guest::RideChoice7ACCA0 | In outline: a guest's choice of the first attraction of a kind (AttractionInstance::FirstOf) by what it is linked to |
| `0x7ad360` | Guest::RideChoice7AD360 | In outline: another choice by the first attraction (AttractionInstance::FirstOf) and its state (`0x7AAD50`, not 2) |
| `0x76a450` | Guest::Animal76A450 | In outline: when not flagged (+0x530), a guest's look at an animal (`0xB75820`) |
| `0x7f6060` | Guest::Leave7F6060 | In outline: a guest's leaving step (`0xBAE8E0` on +0xB8, `0x4D2800` with 0x100) |
| `0x77a620` | GuestGroup::Move77A620 | In outline: in mode 4 (+0xE4), moves a group's members that are not on the ride (+0x118 +0xC) (`0x7C6280` with 0.5) |
| `0x776da0` | GuestGroup::Choose776DA0 | In outline: a group's choice by its state (+0x44: 0, ...), at random (Random::Ran0) among the free options |
| `0x7de9c0` | Guest::Update7DE9C0 | In outline: a guest's update: resets its state (Person::SetState) and checks what it is linked to by kind |
| `0x79a6b0` | Guest::Spot79A6B0 | In outline: finds a spot for a guest over the terrain |
| `0x7aa9c0` | Person::Route7AA9C0 | In outline: another route finder with the path manager |
| `0x7ec8f0` | Person::Behaviour7EC8F0 | In outline: a person's update at a scenery behaviour: resets its state (Person::SetState) and checks the owner's flags (+0x7C bit 4, +0x58 bit 1) |
| `0x7d4450` | Person::Lie7D4450 | In outline: when not flagged (+0x394), lays a person down: turns it along its direction and a quarter turn about x (Matrix::TurnAboutX with pi/2) |
| `0x75bdd0` | Guest::Update75BDD0 | In outline: a guest's update: sets its state (Person::SetState) and, once a time (+0xBC) is reached, checks (`0x98E350`) |
| `0x762f60` | Guest::Wet762F60 | In outline: a guest getting wet (Person::MadeWet), by its wetness level (+0x504, above 2 handled) unless flagged (+0x334) |
| `0x75a800` | Guest::Random75A800 | In outline: in mode 1 (+0x121), picks a random value for a guest, rounded |
| `0x535cc0` | Guest::Swim535CC0 | In outline: a guest's swim check: its group (Guest::Group) and the section it is in (`0x12F5338`) |
| `0x7cd380` | Person::Name7CD380 | In outline: when not flagged (+0x2B0) and with a name option (+0x2A4, `0x13064E8`), makes a person's name text |
| `0x7d2f60` | Person::Cell7D2F60 | In outline: finds the terrain cell a person is on (Grid::CellAt) |
| `0x7c38d0` | Person::Act7C38D0 | In outline: a person's action by its case (0, 1, ...) for a slot (below 0 handled) |
| `0x77d630` | GuestGroup::Members77D630 | In outline: goes over a group's members (+0x48) handling the ones not in state 4 |
| `0x7eb590` | Person::Dress7EB590 | In outline: unless of kind 5 (+0x120), picks a person's body parts (Person::PickBodyParts) and its look (`0x6C3650`) |
| `0x75ba90` | Guest::Update75BA90 | In outline: a guest's update when it has a target |
| `0x781910` | GuestGroup::Ride781910 | In outline: a group's update on a ride (state 0x10) when not flagged (+0x1BC), over its members (+0x48) |
| `0x7b0790` | Guest::Update7B0790 | In outline: a guest's update when not flagged (+0x174) once a time is reached and it moved far enough (Interface138::Get) |
| `0x737ce0` | Staff::Update737CE0 | In outline: a staff update when not flagged (+0xC0) |
| `0x7897e0` | Guest::Find7897E0 | In outline: finds something for a guest (`0x7C8700`) |
| `0x784320` | GuestGroup::Slot784320 | In outline: sets a group's slot (0 to 6) when it changes (+0x40) |
| `0x756790` | Guest::Sounds756790 | In outline: plays a guest's crowd sounds (Crowd::Sounds, `0x7B5010`) unless flagged (+0x509) |
| `0x75ca40` | Guest::Behaviour75CA40 | In outline: a guest's update at a scenery behaviour (state 0xC), resetting its state when the owner is flagged (bit 4 of +0x7C) |
| `0x7972a0` | Guest::Behaviour7972A0 | In outline: another guest update at a scenery behaviour (state 0xF) |
| `0x778580` | GuestGroup::Check778580 | In outline: a group's check over its members (+0x48) unless told not to |
| `0x76f560` | Guest::FindAttraction76F560 | In outline: goes over the attractions (AttractionInstance::FirstOf) and picks the ones a guest can use (`0xA887B0`) |
| `0x784030` | GuestGroup::Move784030 | In outline: moves a group from slot a to slot b (0 to 6) when they differ |
| `0x7d65f0` | Person::Update7D65F0 | In outline: a person's update unless busy (`0x7D3EA0`), when two values match (+0xB8, +0x318) |
| `0x761610` | Guest::Spot761610 | In outline: a guest's spot (`0x7BE2F0`) and its name text |
| `0x77e3d0` | GuestGroup::Members77E3D0 | In outline: goes over a group's members (+0x48) not flagged (+0x2D4) |
| `0xa27df0` | Guest::ListA27DF0 | In outline: goes over a list of guests not flagged (+0xC8) |
| `0x7cf350` | Person::Update7CF350 | In outline: a person's update with no stage (+0x194 = -1) and a part |
| `0x8b5cc0` | Person::Face8B5CC0 | In outline: points a person (Matrix::LookAlongZSide) when it is the one given (+0x104) |
| `0x75c150` | Guest::Update75C150 | In outline: a guest update setting its state (Person::SetState) |
| `0x7d2d00` | Person::Leave7D2D00 | In outline: when its check fails (`0xAB3A80`), sets a person's state to 0x103 |
| `0x8b5880` | Person::Mode8B5880 | In outline: a person step in modes 2 or 3 (+0x18C) with no slot (+0x30) |
| `0x7ded70` | Person::Check7DED70 | In outline: a person check when not flagged (+0x2D4) |
| `0x756550` | Guest::Sounds756550 | In outline: a crowd step that stops the crowd sounds (Crowd::Sounds(0)) |
| `0x7cef60` | Person::Update7CEF60 | In outline: a person update when not flagged (+0x2D4) |
| `0x7cc220` | Person::Find7CC220 | In outline: a person step unless told not to |
| `0x75abf0` | Guest::Mode75ABF0 | In outline: a guest step in mode 0x10 (+0x190) |
| `0x768f80` | Guest::Update768F80 | In outline: a guest update when not flagged (+0x2D4) |
| `0x7de330` | Person::Update7DE330 | In outline: an update when not flagged (+0x2D4) |
| `0x7e1cf0` | Person::Message | In outline: makes a person message (PersonMessage) |
| `0x7cede0` | Person::Time7CEDE0 | In outline: a step when a time (+0xBC) is above 1 and a check fails |
| `0x7d41e0` | Person::Value7D41E0 | In outline: a value from an object (`0x13CE3BC`) |
| `0x75b000` | Guest::Camera75B000 | In outline: a step with the camera (MainCamera::Get) and a check (`0x7C6280`) |
| `0x7c7280` | Person::Case7C7280 | In outline: a step by a value (+0x120: 5, ...) |
| `0x7bcdd0` | Person::Database | In outline: makes a database entry (PPGDatabaseEntry) |
| `0x7d5880` | Person::Check7D5880 | In outline: a step when a check fails (`0x7D3EA0`) |
| `0x786870` | GuestGroup::GolfBall | In outline: a group slot step (GuestGroup::Slot7861D0(1, 1, 1, 6)) and makes a golf ball item (CarriedItem::MakeGolfBall) |
| `0x7b3bf0` | Person::Mode7B3BF0 | In outline: a step unless a person's mode is 8 (+0x190), for kinds under 4 (+0x120) |
| `0x4c5340` | AttractionPerson::Find4C5340 | In outline: when not flagged (+0x378), the nearest person from its part (+0x364) |
| `0x4d1fa0` | Person::Toward4D1FA0 | In outline: with its part (+0x58 +0xC), a step toward a point (`0x7C6280`) |
| `0x4d1e80` | Person::SwimToward4D1E80 | In outline: with its part (+0x58 +0xC), swims toward a point (Person::SwimToward) |
| `0x7618f0` | Guest::Swap7618F0 | In outline: replaces a shared counted part (+0x578), freeing the old one when no longer used |
| `0x7e2a10` | Guest::Value7E2A10 | In outline: picks one of four shared values by a value (`0x7F86C0`) |
| `0x75e740` | Guest::Once75E740 | In outline: once (+0x404), a step with a handle |
| `0x760000` | Guest::Update760000 | In outline: a long guest update that sets a value to 0xDC and more |
| `0x7e4030` | Person::Pose7E4030 | In outline: a pose from its part's values (+0xA4 +0x60 on) |
| `0x7caa00` | Person::Find7CAA00 | In outline: a find over a linked list (+0x2EC), 0 when empty |
| `0x7ce320` | Person::Mode7CE320 | In outline: in modes 2 or 3, when on (+0x3F0), updates a target (Person::UpdateTarget7CB120) |
| `0x7adc90` | Person::Make7ADC90 | In outline: makes a record from its arguments |
| `0x7d68e0` | Person::Swim7D68E0 | In outline: a large person update by stage (+0x194): sets states (0x112, 0x11F, 0x100), counts a time (+0xBC) against a table (`0xFC35D8`), swims toward a pool part's point (Person::SwimToward, PoolComplex::PartA87E00) |
| `0x7dd270` | Person::Update7DD270 | In outline: a large person update: with no part, sets its state to 0; acts by a part's mode (2, ...) |
| `0x7d32d0` | Person::Update7D32D0 | In outline: a large person update with its part (+0x1C0), setting state 0x100 when the part is gone or flagged |
| `0x7de4d0` | Person::Update7DE4D0 | In outline: with its part, in mode 9 (+0x190) and no stage, a step |
| `0x7ce720` | Person::Link7CE720 | In outline: 0 with no part (+0x30C), else a step with it |
| `0x7c27d0` | Person::AnimFrame7C27D0 | In outline: when a time (+0xBC) has passed a table length (`0xFC35D8` by +0xB8), the number of times it has, rounded |
| `0x7c3df0` | Person::Check7C3DF0 | In outline: a check with its part (+0x1C0), 0 with none |
| `0x7ec830` | Person::Stop7EC830 | In outline: in mode 0 (+0x190), clears its value (Person::Set7EC730(0)) and acts with a part (+0x4E0) at its point |
| `0x758a30` | Guest::Check758A30 | In outline: 0 when it has a part (+0x1F0); in mode 0x11 with a part, a check |
| `0x7632d0` | Guest::Once7632D0 | In outline: once (+0x444), a step with the park |
| `0x759690` | Guest::AddValue759690 | In outline: adds to a value (+0x40C), kept between 0 and 1 |
| `0x7cf960` | Person::Case7CF960 | In outline: a value by mode (+0x190, 0x100 and others) |
| `0x4ea940` | AttractionPerson::Activate4EA940 | In outline: a step (`0x5528B0`) and activates the attraction's person |
| `0x759d50` | Guest::Update759D50 | In outline: updates (Person::Update7D65F0) and, in mode 0, a check (`0x7C3480`) |
| `0x7cf900` | Person::Reset7CF900 | In outline: steps (Person::Link7CE720, `0x7622D0`, `0x7A9550`) and clears a flag (+0x224) |
