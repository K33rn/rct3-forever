# Guests

Guests (people visiting the park). More on how they choose rides is in [the research notes](../research-physics-and-guests.md).

| Address | Name | What it does |
|---|---|---|
| `0x765990` | Guest::Update | A guest's main update (virtual slot 47 of Guest, also used by VIP and BodyGuard): runs the guest's current state (+0x190) and its timers, needs and decisions, using the park (`0x1306194`) and the random number generator (28 calls); also where the name cheats take effect (it checks the guest's name against `thirsty bob`, `vomit man`, `flameboy`, `lobster face`, `slow burner`, `happy camper`, `sad panda` and `make like a tree`; `ParticleEffects\PurpleCloud.frw` is one of their effects) |
| `0x7c8b10` | Person::StateName | Writes the name of a person's current state for display (virtual slot 36 of Person, used by guests and all staff): `Queueing - Start`, `- PushingIn`, `- Waiting`, `- Paying`, ..., `Swimming - Idle`, `- Shower`, `- Diving`, `- SunLounger`, `- Splashing Someone`, `HighDive - ...` and the other states, with seven switches over state and sub-state |
