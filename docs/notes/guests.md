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
