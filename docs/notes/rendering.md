# Rendering

The graphics engine ("GE", from Frontier's engine fgdk3, whose source paths `c:\codinggdk3\Code\Windows\GE_*` appear in its error messages) and what the game draws with it.

| Address | Name | What it does |
|---|---|---|
| `0xc77890` | VertexShaderFragments::Build | Assembles the library of vertex shader fragments the engine builds its vertex shaders from: about 129 `vs_1_1` fragments (`VSF_TRANSFORM_POS_BLEND0/1`, `VSF_TRANSFORM_NORMAL_*`, `..._TANGENT_*`, `VSF_NORMALISE_NORMAL`, `VSF_BUMP*`, `VSF_WATERBUMPSCALE`, `VSF_LIGHTING_{AMBIENT,DIFFUSE,SPECULAR,EMISSIVE}SOURCE_*`, `VSF_LIGHT_*` for four lights, `VSF_LIGHTING_OUTPUT_*`, ...), each put through D3DX's shader assembler (`0xCF6275`) and kept; errors are reported against `GE_Fragment.h`. Called once, from `0xC045A0` |
| `0x8a2ef0` | SceneryRenderer::CollectInstances (pass 3) | Gathers scenery instances to draw: for every instance group and every instance in it (0x150 bytes each, position at +0x60), checks its layer mask against the pass's 64-bit mask (`0x10F07B8`), measures its distance from the camera (`0x14002C0`), compares it with the level-of-detail distances (per type, scaled by the detail settings at `0x10F07C0` / `0x10F087C`) and copies its world matrix into the draw batch of the level of detail it falls in. Virtual slot 6 of the table `0xF7A1EC` |
| `0x895a90` | SceneryRenderer::CollectInstances (pass 2) | The same as `0x8A2EF0` for another pass (slot 4 of `0xF7A1EC`) |
| `0x891f40` | SceneryRenderer::CollectInstances (pass 1) | The same as `0x8A2EF0` for another pass (slot 2 of `0xF7A1EC`) |
| `0xc96dc0` | GE_Device::RenderScene | Draws a frame's scene on the graphics device (called from `0xC9B600`): first sorts the scene's enabled lights (0x90-byte entries) into lists by type, then sets up and draws each part of the scene through the device, with a 38-way switch over what is being drawn |
| `0xadbe60` | LaserShow::DrawBeams | Draws a laser show's beams (`0x13CE508` is the current laser display): takes the beam colour (`0xAD8AA0`), measures the distance from the camera, and builds the beam geometry with its spread, rotation and trails (random flicker from `_rand`). Called from `0xADA8D0` |
