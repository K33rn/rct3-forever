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
