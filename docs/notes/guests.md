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
