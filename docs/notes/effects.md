# Effects



| Address | Name | What it does |
|---|---|---|
| `0x920910` | Waterfall::Update | Runs the waterfalls: the falling water (gravity -9.8) and its particles, and the `WaterfallSound`. Called from `0x9225A0` / `0x9225B0` |
| `0x94f250` | WaterParticles::Update | Moves a set of water particles (slot 0 of `0xF7E438`): random spray directions, gravity (-9.81), and a switch over the kind of particle |
