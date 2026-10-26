# Rendering

The graphics engine ("GE", from Frontier's engine fgdk3, whose source paths `c:\coding\fgdk3\Code\Windows\GE_*` appear in its error messages) and what the game draws with it.

| Address | Name | What it does |
|---|---|---|
| `0xc77890` | VertexShaderFragments::Build | Assembles the library of vertex shader fragments the engine builds its vertex shaders from: about 129 `vs_1_1` fragments (`VSF_TRANSFORM_POS_BLEND0/1`, `VSF_TRANSFORM_NORMAL_*`, `..._TANGENT_*`, `VSF_NORMALISE_NORMAL`, `VSF_BUMP*`, `VSF_WATERBUMPSCALE`, `VSF_LIGHTING_{AMBIENT,DIFFUSE,SPECULAR,EMISSIVE}SOURCE_*`, `VSF_LIGHT_*` for four lights, `VSF_LIGHTING_OUTPUT_*`, ...), each put through D3DX's shader assembler (`0xCF6275`) and kept; errors are reported against `GE_Fragment.h`. Called once, from `0xC045A0` |
| `0x8a2ef0` | SceneryRenderer::CollectInstances (pass 3) | Gathers scenery instances to draw: for every instance group and every instance in it (0x150 bytes each, position at +0x60), checks its layer mask against the pass's 64-bit mask (`0x10F07B8`), measures its distance from the camera (`0x14002C0`), compares it with the level-of-detail distances (per type, scaled by the detail settings at `0x10F07C0` / `0x10F087C`) and copies its world matrix into the draw batch of the level of detail it falls in. Virtual slot 6 of the table `0xF7A1EC` |
| `0x895a90` | SceneryRenderer::CollectInstances (pass 2) | The same as `0x8A2EF0` for another pass (slot 4 of `0xF7A1EC`) |
| `0x891f40` | SceneryRenderer::CollectInstances (pass 1) | The same as `0x8A2EF0` for another pass (slot 2 of `0xF7A1EC`) |
| `0xc96dc0` | GE_Device::RenderScene | Draws a frame's scene on the graphics device (called from `0xC9B600`): first sorts the scene's enabled lights (0x90-byte entries) into lists by type, then sets up and draws each part of the scene through the device, with a 38-way switch over what is being drawn |
| `0xadbe60` | LaserShow::DrawBeams | Draws a laser show's beams (`0x13CE508` is the current laser display): takes the beam colour (`0xAD8AA0`), measures the distance from the camera, and builds the beam geometry with its spread, rotation and trails (random flicker from `_rand`). Called from `0xADA8D0` |
| `0x8ab170` | SceneryRenderer::CollectInstances (pass 4) | The same as `0x8A2EF0` for another pass (slot 8 of `0xF7A1EC`) |
| `0xc2cb20` | Shape::RunCommands | Runs a shape's list of drawing commands, a switch over the command codes; an unknown one is reported (`Bad command %d encountered in shape`). Called from `0xCD8520` |
| `0x865c40` | Model::DebugReport | Writes a model's statistics as text: its levels of detail, and for each its meshes by kind (`Static`, `Alpha`, `Billboard`, `SBone`, `Bone`, `Morph`, `Unknown`) with their triangle counts, warning when a level keeps more than 60% of the previous one's triangles; and the number of water particles. Called from `0x86E1D0` |
| `0x86a430` | SceneryModel::FindLightsAndWater | Reads a scenery model's special bones: lights (`lightstart%d`, `lightend%d`, `lightcolour%d`, with `radius`, `simple` / `medium`, `flexi`) and water effects (`watertrail%d`, `waterwake%d`, `waterstart%d` / `waterend%d`), complaining about bones without a partner and lights with too many colour bones (`Static light for '%s' has %d colour bones!`). Called from `0x86D3A0` and `0x86D5E0` |
| `0x87cf90` | ParkRenderer::SetupScenery | Sets up the drawing state for the park's scenery each frame: the shadow-map horizon and the alpha reference values for scenery items (`AlphaRefSI`, `AlphaRefSILow`) and billboards, from the detail settings. Called from `0x47EB80` |
| `0x47eb80` | ParkRenderer::Render | Draws the park: the terrain (`RCT3Terrain`), water (`WaterManager`), the sky (`Sky`), scenery (`0x87A130`, set up by `0x87CF90`) and the effects, for the camera `0x13D0DB8`. Called from `0x480E40` |
| `0xcb3320` | PixelShader11::Generate | Writes a `ps.1.1` pixel shader from a fixed-function texture-stage setup: `tex t%i` for each texture, then each stage's operation as `mov`, `mul` (`_x2`, `_x4`), `add` (`_bias`), `sub`, `mad`, `lrp` or `dp3` (`_bx2`) on `r0` / `r1`. Slot 19 of `0xFC0AB4` |
| `0x89d190` | Billboard::Create | Makes the billboard (flat stand-in picture) of a scenery item for drawing it far away (`SIFillZ`), reporting `Missing billboard texture for '%s'` and `Broken Billboard`. Called from four places in the scenery renderer |
| `0x868000` | Model::UpdateLights | Updates a model's lights and water effects each frame: switches lamps on (`LampOn`), flickers fluorescent tubes (`FlourescTubeFlicker2`) and emits water trails. Called from 22 places: every kind of ride car, scenery and more |
| `0x85b3c0` | SceneryRenderer::DrawShadows | Draws the scenery's shadows and blueprint pass (`ShapeBlueprintOpaque`, `BillboardShadowAlphaMask`), each part inside a profiling section (`Section::Enter` / `Leave`). Called from `0x85BD40` |
| `0x9723e0` | Renderer::SetupAlphaRefs | Sets up the alpha reference values for drawing scenery items and billboards (`AlphaRefSI`, `AlphaRefSILow`, `AlphaRefBillboard`) and the terrain it needs |
| `0x94e790` | WaterEffects::LoadShaders | Loads the water effects' shaders: water jets and blobs (`WaterJet`, `WaterJetRefraction`, `WaterBlobAlpha`, `WaterBlobAdditive`, `WaterBlobRefraction`, `WaterBlobPerPixelRefraction`). Called from `0x477CC0` |
| `0x8c7030` | Terrain::SetupHorizonShadow | Sets up the terrain's horizon shadow texture coordinates (`HorizonShadowUV`, `HorizonShadowUVHalf`). Called from `0x8C8BA0` |
| `0x8410f0` | SceneryItemVisual::CheckResolved | Reports what a scenery item's visual is missing: `Unresolved SceneryItemVisualDesc`, `StaticShapeData`, `FlexiTexture`, `TextureStyle`, `BoneShape`, `BoneAnim`, `Inconsistent number of animations in LODs`, `No animations for BoneShape`. Called from `0x842870` |
| `0xcee580` | ShapeMaterial::FromName | Turns a shape material's name into its kind: `AlphaMask`, `Alpha`, `Glass`, `FillZ`, `Caustics`, `Water`, `SIGlass`, `SIAlphaMaskSpecular100Reflection`, `SIAlphaText`, `SIOpaqueText`, `Mask`, ... |
| `0xccb610` | ShapeMaterial::FromName (2) | The same as `0xCEE580`, a second copy for another kind of shape |
| `0xcd8a60` | Shape::Draw | Draws a shape by its material kind (`AlphaMask`, `Alpha`, `Glass`), through the shape commands. Called from `SceneryRenderer::DrawShadows` and two others |
| `0x8e96b0` | Terrain::SetupShadowShader | Sets up the shader that adds the terrain's horizon shadow (`AddToDest`, `TerrainShadowHorizon`). Called from `0x8EA330` |
| `0x963910` | WaterParticles::LoadTextures | Loads the water particle textures (`WaterParticle.bmp`, `WaterParticleAlpha.bmp`), seeding the random number generator for their variations |
| `0x864150` | Model::CheckShapes | Reports a model's missing parts: `Missing StaticShapeData for '%s'`, `No BoneShapeData for '%s'` (`Broken Boned Shape`), `Missing BoneShape for '%s'`. Called from 14 places |
| `0x863c30` | Model::CheckShapes (2) | The same checks as `0x864150`, for another kind of model. Called from `0x864010` and `0x880FB0` |
| `0xce24d0` | Model::CheckAnimations | Reports a model's missing animations and bone shapes per level of detail (`Anim %d in '%s'`, `Missing animation`, `Lod %d in %s`, `Missing bone shape`). Called from three places |
| `0x9fe5a0` | Lighting::LoadResources | Loads the lighting resources: the light map, lights (`Lights\Lights`), the full-screen effects (`FullscreenModulateX2`, `FullscreenFadeToBlack`) and the terrain light map. Called from three places |
| `0xc1c4b0` | GlareEffect::LoadShaders | Loads the glare effect's shaders: threshold ramps, streak blur, Kawase blur and applying the glare (`GE_GlareEffect_*`). Called from `0xC1C920` |
| `0xc96960` | GE_Device::SetupDepthOfField | Sets up the depth-of-field fill (`GE_Device_DofFill`) and its colour target. Called from `0xC045A0` |
| `0x738d10` | QueueType::LoadTexture | Loads a queue type's ground texture, reporting `Missing texture for queue type`. Called from `0x71AB90` and `0x71F4C0` |
| `0x9e2070` | Rain::LoadResources | Loads the rain's resources: its particles and shaders (`Rain\Rain`, `RainAdditive`, `ParticleAdditive`, `Flare_00`). Called from five places |
| `0x986990` | WaterBlobs::LoadShaders | Loads the shaders of water blobs and bubbles (`WaterBlobAlpha`, `WaterBlobAlphaAdditive`, `Bubble0`). Called from `0x47E700` |
| `0x8ffc90` | WaterManager::LoadShaders | Loads the simulated water's shaders (`WaterSimulated`, `...Shader`, `...Caustics`, `...Refraction`, `PoolStencilMask`). Called from `WaterManager::WaterManager` |
| `0x911d30` | Water::LoadReflectionShaders | Loads the water reflection and ripple shaders (`WaterReflectionAnimatedBumpMap`, `WaterRippleNeg`, `WaterRipplePos`, `WaterTest`, `WaterStencil`). Called from `0x94AE40` |
| `0x936a90` | WaterFlowManager::DebugText | Writes the water flow manager's render mode for the developer (`RenderMode_WireFrame`, `_DisableTessellation`, `_Normal`). Called from `ParkRenderer::Render` |
| `0x950830` | WaterJet::LoadShaders | Loads the water jet's shaders (`WaterJet`, `WaterJetRefraction`). Called from `0x955750` |
| `0x47e700` | Renderer::SetupTransforms | Sets up the environment cube map and light map transforms (`EnvCubemapTransform`, `LightmapTransform`). Called from `0x47EAE0` |
| `0x40d010` | Model::CheckBoneAnimations | Reports a bone shape's missing animations (`BoneShape '%s' is missing animations`, `Animation %d missing`). Called from nine places |
| `0xce1390` | ShapeShaders::Load | Loads the shape shaders (`ShapeAll`, `ShapeAlphaMask`, `ShapeShadowOpaque`, `ShapeShadowAlphaMask`, `ShapeShadowAlpha`). Called from three places |
| `0xcb1650` | DepthOfField::LoadShaders | Loads the depth-of-field shaders (`GE_DOF_ApplyRamp`, `GE_DOF_Apply2`, `GE_DOF_Apply3`, `GE_OpaqueTexture`). Called from `0xCB18F0` |
| `0x9ffff0` | LightVolumes::LoadShaders | Loads the light volume shaders (`LightVolumeFront`, `LightVolumeBack`, `LightVolumeBackAlways`, `Lightsphere`). Called from `0x47DB20` |
| `0xc061d0` | GE_Device::LoadPassShaders | Loads the device's depth-of-field, shadow and occlusion query shaders (`GE_Device_DofFill`, `GE_Device_ShadowRender`, `GE_Device_OcclusionQuery`). Called from three places |
| `0x703c70` | Terrain::LoadGridShaders | Loads the terrain grid shaders: billboards and the owned, construction-rights and selected grids (`TerrainOwnedGrid`, `TerrainConstructionOwnedGrid`, `TerrainSelectedGrid`). Called from `0x7048E0` |
| `0x8e4ee0` | Terrain::DebugChess | The developer's chessboard terrain patterns (`Chess: Odd`, `Chess: Even`) |
| `0xcec310` | ShapeMaterial::FromName (3) | Turns a material name into its kind: `AlphaMask`, `Alpha`, `Glass`, `Reflection`, `Chrome`. Called from `0xCEC590` |
| `0x811860` | AlphaMask::LoadBitmaps | Loads the alpha mask bitmaps (`AlphaMaskBitmap`, `AlphaMaskFadeBitmap`). Called from three places |
| `0x9de680` | EnvironmentMap::LoadShaders | Loads the environment cube map shaders (`EnvCubemap`, `EnvCubemapColour`). Called from `0x9DF0E0` |
| `0x85b060` | Shadows::LoadShaders | Loads the shadow shaders (`ShadowDrawSolidColour`, `ShadowCopy`). Called from `0x85BC70` |
| `0x914660` | WaterDrop::LoadShaders | Loads the water drop shaders (`WaterDrop`, `WaterDropWash`). Called from `0x477BD0` |
| `0xc9c210` | Shadows::LoadShaders (2) | Loads the shadow projection shaders (`GE_Shadows_ShadowProject`, `GE_Shadows_DebugShadowBuffer`). Called from `GE_Device::LoadPassShaders` |
| `0x964d70` | WaterWake::LoadShaders | Loads the water wake shaders (`WaterWakeBump`, `WaterWakeAlpha`). Called from `0x964EB0` |
| `0x4323f0` | SceneryRenderer::BuildAlphaBatches | Builds the scenery's alpha-blended draw batches (`SIAlpha`). Called from `0x433120` |
| `0x704160` | Terrain::DrawBillboards | Draws the far-away billboards over the terrain (`BillboardStandard`). Called from `0x439280` and `DebugInfo::Draw` |
| `0x9df240` | Sky::Render | Draws the sky and the environment cube map (`Sky`, `EnvCubemap`) |
| `0x9de820` | Sky::LoadEnvMaps | Loads the sky's environment cube maps (`Sky\EnvCubemap_`). Called from `0x9DF140` |
| `0x421db0` | AdMenu::DrawPreview | Draws an advert's preview picture (`SIOpaque`) |
| `0xcbab50` | Inflate::Codes | zlib's decoding of literal / length and distance codes (`invalid literal/length code`, `invalid distance code`). Called from `Inflate` |
| `0x959ae0` | Shadows::DrawDebugBuffer | Draws the shadow buffer for the developer (`GE_Shadows_DebugShadowBuffer`). Called from `0x9493A0` |
| `0x869bf0` | SceneryVisual::CheckLODs | Reports a scenery visual's levels of detail missing their shapes (`Missing shp for svd LOD '%s'`, `Broken SVD`, `Missing BoneShape for svd LOD '%s'`). Called from two places |
| `0x8f7870` | Terrain::DetailShader | Gets the terrain detail shader (`TerrainDetail`). Called from ten places |
| `0x864a40` | Model::CheckAnimationEnd | Reports an animation whose end time isn't positive. Called from `Model::UpdateLights` |
| `0x644fc0` | SceneryVisual::CheckLOD | Reports a missing scenery visual for a level of detail (`%s LOD %d`, `Missing svd`). Called from `0x6455C0` |
| `0x8a12e0` | SceneryRenderer::AddInstance | Adds one scenery instance to the draw batches of its level of detail: for each mesh of its model whose layer matches the pass's mask, appends the instance's world matrix (+0x30 to +0x6C of the instance) and its colour to the mesh's batch (0x50-byte entries; meshes with the special flags 0x9004 go into 0x60-byte entries with their extra values). Called by `SceneryRenderer::CollectInstances` and `0x88E7B0`; `0x8A2660`, `0x8A1DE0` and `0x8A0B90` are versions of it with the same calls for the other passes |
| `0x8a2660` | SceneryRenderer::AddInstance (2) | A version of `SceneryRenderer::AddInstance` for another pass (the same calls and globals). Called from `0x88E7B0` and `SceneryRenderer::CollectInstances` |
| `0x8a1de0` | SceneryRenderer::AddInstance (3) | A version of `SceneryRenderer::AddInstance` for another pass. Called from `0x88E7B0` and `SceneryRenderer::CollectInstances` |
| `0x8a0b90` | SceneryRenderer::AddInstance (4) | A version of `SceneryRenderer::AddInstance` for another pass. Called from `0x88E7B0` and `SceneryRenderer::CollectInstances` |
| `0x88e7b0` | SceneryRenderer::CollectInstance | The single-instance version of `SceneryRenderer::CollectInstances`: checks one instance's layer mask against the pass (`0x10F07B8`), picks its level of detail from its distance and the detail settings (`0x10F07C0` / `0x10F087C`), and adds it to the batches (`SceneryRenderer::AddInstance` and its versions), keeping per-pass counts at `0x1311070`-`0x13110F0` |
| `0x88e200` | SceneryRenderer::CollectInstance (2) | The same as `0x88E7B0` for another pass (its counts at `0x1310FE0`-`0x1311060`) |
| `0x697f90` | GUIRenderer::Render | Draws the GUI items queued for the frame: walks the list of 64-byte entries (`0x12F84CC`, starting from `0x10EEDCC`, 0xFFFF ending it), setting the device's shaders and textures for each and drawing it by its kind (a 10-way switch), then empties the list. Called from six places, the front end and the desktop among them |
| `0x696220` | GUIRenderer::Render (2) | A second version of `GUIRenderer::Render` for the other list of queued GUI items (the same calls and globals) |
| `0xcc26d0` | FlexiTextureType::FlexiTextureType | Constructor of the FlexiTexture resource type: builds the resource type base (`0xC2FFB0`) from its tag `ftx`, name `FlexiTexture` and group `FGDK` (texts made with `0x401080`, released after), then sets its table `0xFC31A8`; returns it |
| `0x910ff0` | Water::MakeSpiralTextures | Makes two 64x64 textures (`0xBFACB0`, kept at +0x48 and +0x44) for the water shaders: works out a 128x128 field of surface normals for a three-armed spiral (twice the distance from the centre plus three times the angle over 2 pi, its fraction giving the ridges, sloped between 0.2 and 0.8), scaled by 0.4, averages it down to 64x64, then fills the first texture with the normals' x and z halved and the second with 1 - 2(0.15 v + 0.5), clamped to 0..1, as ARGB bytes (alpha 255; the border left flat). Called from Water::LoadReflectionShaders |
| `0x910490` | Water::MakeRippleTextures | Makes two 64x64 textures (kept at +0x40 and +0x3C) for the water ripple shaders: a 128x128 field of surface normals for one circular ripple, sloping outward just outside radius 0.75 and inward just inside it (within 0.1 of it, flat elsewhere), averaged down to 64x64 and written as ARGB bytes the same way as Water::MakeSpiralTextures. Called from Water::LoadReflectionShaders |
| `0x962c90` | WaterParticles::MakeBlob | Makes one random lumpy blob for the water particle texture: an octahedron subdivided n times (4 when called) into a sphere, each new point put on the unit sphere with a height of the average of its edge's two ends plus (Random::Unit - 0.5) times the roughness (0.6), then has it drawn (`0x95FAA0`, `0x960810`, `0x9619F0`) into the given part of the 0x180 x 0x200 image (columns 0-0x80 or 0x80-0x100). Called twice from WaterParticles::LoadTextures |
| `0x42f320` | SceneryRenderer::QuadVertices | Fills four vertices of a camera-facing quad for the alpha batch: corners at the centre plus or minus the two given half-axes, each with the given colour and texture coordinates from the given rectangle. Called from SceneryRenderer::BuildAlphaBatch |
| `0xcb1e50` | PixelShader11::RegisterName | Writes the name of a ps.1.1 source register for the generated pixel shader: `v0`, `r0`/`r1`, `t`<n>, ... by the low four bits of its code, with `1-` in front for the complement modifier (0x10). Called from PixelShader11::Generate |
| `0x42fe40` | SceneryRenderer::SiftDown | Moves an entry down the alpha batch's heap of draw items (+0x64, count +0x6C) until both children have no larger value at +0x24 (a max-heap on that value). Called from SceneryRenderer::BuildAlphaBatch |
| `0x435a40` | Colour::FromARGB | Unpacks a 32-bit 0xAARRGGBB colour into red, green, blue and alpha floats from 0 to 1. Used by Sky::Render and WaterfallManager::WaterfallManager |
| `0x402c40` | Colour::ToARGB | Packs red, green, blue and alpha values from 0 to 1 into a 32-bit 0xAARRGGBB colour (each times 255, rounded). Used by RideOpModeDialog::Build, TrackSpecial::Directions and Window::DrawHighlight |
| `0x454850` | Colour::FromARGB | Unpacks a 32-bit 0xAARRGGBB colour into red, green, blue and alpha floats from 0 to 1 (the same code as `0x435a40`) |
| `0x402bf0` | Colour::ToRGB | Packs red, green and blue values from 0 to 1 into an opaque 32-bit 0xFFRRGGBB colour. Used by StartupScreen::Draw, TrackEditor::Build and Messages::Redirect |
| `0x4261f0` | Lighting::DownwardDirection | Gives the vector at +0x120 with its y (+0x124) made negative when it is positive, so it points down. Used by DebugInfo::Draw, Renderer::SetupAlphaRefs and WaterParticles::Update |
| `0x95faa0` | Mesh::ComputeNormals | Works out a mesh's vertex normals: clears them, adds each triangle's unit face normal (its height scaled by 0.4) to its three corners, then makes each normal unit length (0,0,1 when it is zero). Called from WaterParticles::MakeBlob |
| `0x41fa20` | D3DResource::GetDesc | Copies the object's 32-byte description (from +8) into the buffer given; returns 0, or D3DERR_INVALIDCALL (0x8876086C) when no buffer is given |
| `0xcd63a0` | RenderStats::AddBatch | Adds a draw call to the render statistics: its primitive count (a triangle list's indices / 3, a strip's or fan's count - 2) to the total, one more batch, the smallest and largest batch; when asked also the bytes it uses (its vertex range times the vertex size, plus 2 bytes per index when it has indices). Used by about 65 drawing functions for the developer's statistics |
| `0xbfacb0` | Device::CreateTexture | Makes a texture of the given width and height on the graphics device (`0x13D0DB8`): its mipmap level count is one per halving of the larger side, at most the number asked for (when not 0); an 84-byte texture object made by `0xBFAB20` with the format given; null without a device. Used for the water bump textures and others |
| `0x9d4fc0` | ParticleSource::SetPosition | Sets the particle source's position (+0x60) and, when it has a description (+0xCC), works out its other vectors again (`0x9C6780`, into +0x70 and +0x80) |
| `0xa62ef0` | MainCamera::UpdateMode | Calls slot 3 of the camera's current mode (mode n of the list at +0x3B8, n at +0x3B4) |
| `0x94ddd0` | WaterDrops::SetEmitterDirection | Sets emitter n (208 bytes each at +0x1A4) to mode 1 with the given direction (+0x70) |
| `0xc6f000` | RenderState::SetFogenable | Sets the Direct3D render state 28 (D3DRS_FOGENABLE) to the value given (the device's SetRenderState, slot 57), only when it differs from the value kept at +0x404, which it then keeps |
| `0xc6f030` | RenderState::SetAmbient | Sets the Direct3D render state 139 (D3DRS_AMBIENT) to the value given (the device's SetRenderState, slot 57), only when it differs from the value kept at +0x40c, which it then keeps |
| `0xc6f070` | RenderState::SetPointsize | Sets the Direct3D render state 154 (D3DRS_POINTSIZE) to the value given (the device's SetRenderState, slot 57), only when it differs from the value kept at +0x410, which it then keeps |
| `0xc6f0e0` | RenderState::SetNormalizenormals | Sets the Direct3D render state 143 (D3DRS_NORMALIZENORMALS) to the value given (the device's SetRenderState, slot 57), only when it differs from the value kept at +0x41c, which it then keeps |
| `0xc6f120` | RenderState::SetColorwriteenable | Sets the Direct3D render state 168 (D3DRS_COLORWRITEENABLE) to the value given (the device's SetRenderState, slot 57), only when it differs from the value kept at +0x420, which it then keeps |
| `0xc6f160` | RenderState::SetZwriteenable | Sets the Direct3D render state 14 (D3DRS_ZWRITEENABLE) to the value given (the device's SetRenderState, slot 57), only when it differs from the value kept at +0x424, which it then keeps |
| `0xc6f190` | RenderState::SetStencilenable | Sets the Direct3D render state 52 (D3DRS_STENCILENABLE) to the value given (the device's SetRenderState, slot 57), only when it differs from the value kept at +0x428, which it then keeps |
| `0xc6f1c0` | RenderState::SetStencilfail | Sets the Direct3D render state 53 (D3DRS_STENCILFAIL) to the value given (the device's SetRenderState, slot 57), only when it differs from the value kept at +0x42c, which it then keeps |
| `0xc6f1f0` | RenderState::SetStencilzfail | Sets the Direct3D render state 54 (D3DRS_STENCILZFAIL) to the value given (the device's SetRenderState, slot 57), only when it differs from the value kept at +0x430, which it then keeps |
| `0xc6f220` | RenderState::SetStencilpass | Sets the Direct3D render state 55 (D3DRS_STENCILPASS) to the value given (the device's SetRenderState, slot 57), only when it differs from the value kept at +0x434, which it then keeps |
| `0xc6f250` | RenderState::SetStencilfunc | Sets the Direct3D render state 56 (D3DRS_STENCILFUNC) to the value given (the device's SetRenderState, slot 57), only when it differs from the value kept at +0x438, which it then keeps |
| `0xc6f280` | RenderState::SetStencilref | Sets the Direct3D render state 57 (D3DRS_STENCILREF) to the value given (the device's SetRenderState, slot 57), only when it differs from the value kept at +0x43c, which it then keeps |
| `0xc6f2b0` | RenderState::SetStencilmask | Sets the Direct3D render state 58 (D3DRS_STENCILMASK) to the value given (the device's SetRenderState, slot 57), only when it differs from the value kept at +0x440, which it then keeps |
| `0xc6f2e0` | RenderState::SetStencilwritemask | Sets the Direct3D render state 59 (D3DRS_STENCILWRITEMASK) to the value given (the device's SetRenderState, slot 57), only when it differs from the value kept at +0x444, which it then keeps |
| `0xc903a0` | RenderState::SetCullmode | Sets the Direct3D render state 22 (D3DRS_CULLMODE) to the value given (the device's SetRenderState, slot 57), only when it differs from the value kept at +0x2270, which it then keeps |
| `0xc903d0` | RenderState::SetAlphablendenable | Sets the Direct3D render state 27 (D3DRS_ALPHABLENDENABLE) to the value given (the device's SetRenderState, slot 57), only when it differs from the value kept at +0x2274, which it then keeps |
| `0xc90400` | RenderState::SetSrcblend | Sets the Direct3D render state 19 (D3DRS_SRCBLEND) to the value given (the device's SetRenderState, slot 57), only when it differs from the value kept at +0x2278, which it then keeps |
| `0xc90430` | RenderState::SetDestblend | Sets the Direct3D render state 20 (D3DRS_DESTBLEND) to the value given (the device's SetRenderState, slot 57), only when it differs from the value kept at +0x227c, which it then keeps |
| `0xc90460` | RenderState::SetAlphatestenable | Sets the Direct3D render state 15 (D3DRS_ALPHATESTENABLE) to the value given (the device's SetRenderState, slot 57), only when it differs from the value kept at +0x2280, which it then keeps |
| `0xc90490` | RenderState::SetAlphafunc | Sets the Direct3D render state 25 (D3DRS_ALPHAFUNC) to the value given (the device's SetRenderState, slot 57), only when it differs from the value kept at +0x2284, which it then keeps |
| `0xc904c0` | RenderState::SetAlpharef | Sets the Direct3D render state 24 (D3DRS_ALPHAREF) to the value given (the device's SetRenderState, slot 57), only when it differs from the value kept at +0x2288, which it then keeps |
| `0xc90520` | RenderState::SetZfunc | Sets the Direct3D render state 23 (D3DRS_ZFUNC) to the value given (the device's SetRenderState, slot 57), only when it differs from the value kept at +0x2290, which it then keeps |
| `0xc90550` | RenderState::SetZwriteenable | Sets the Direct3D render state 14 (D3DRS_ZWRITEENABLE) to the value given (the device's SetRenderState, slot 57), only when it differs from the value kept at +0x2294, which it then keeps |
| `0x42f670` | QuadTree::MarkChanged | Marks node n of a four-way tree (28-byte nodes at +8) and each node above it (parent (n - 1) / 4) with flag 0x10, stopping at the first one already marked |
| `0x488320` | QuadTree::MarkChanged | Marks node n of a four-way tree (28-byte nodes at +8) and each node above it (parent (n - 1) / 4) with flag 0x10, stopping at the first one already marked |
| `0x7b2a40` | QuadTree::MarkChanged | Marks node n of a four-way tree (28-byte nodes at +8) and each node above it (parent (n - 1) / 4) with flag 0x10, stopping at the first one already marked |
| `0xa0be40` | QuadTree::MarkChanged | Marks node n of a four-way tree (28-byte nodes at +8) and each node above it (parent (n - 1) / 4) with flag 0x10, stopping at the first one already marked |
| `0xb5b5d0` | QuadTree::MarkChanged | Marks node n of a four-way tree (28-byte nodes at +8) and each node above it (parent (n - 1) / 4) with flag 0x10, stopping at the first one already marked |
| `0xb6b880` | QuadTree::MarkChanged | Marks node n of a four-way tree (28-byte nodes at +8) and each node above it (parent (n - 1) / 4) with flag 0x10, stopping at the first one already marked |
| `0x4301e0` | QuadTree::QuadTree | Constructor of a four-way spatial tree (a quadtree) over the area between the two corners given: keeps its owner, makes the node array (28 bytes per node, how many from `0xCDB5E0`) and sets the root square (+0xC origin, +0x14 size) to twice the area, centred on it |
| `0x430770` | QuadTree::UpdateBounds | Brings the bounds of node n up to date when it is marked changed (flag 0x10): updates its changed children (nodes 4n+1 to 4n+4) first, notes which children hold anything (bits 0-3), and takes the lowest and highest values of its children and of its own items (32 bytes each) as its range (+4, +8); then clears the mark |
| `0x4311c0` | QuadTree::Insert | Puts an item into the tree: finds the node that holds its rectangle (`0xCDB600`), adds the item there (`0x430520`) and marks that node and those above it changed (`0x42f670`, QuadTree::MarkChanged); gives the node in the high 16 bits and the item's slot in the low ones |
| `0x433c30` | QuadTree::Cull | Culls the tree against six planes (the camera's view): updates the bounds first when needed (`0x430770`), then tests the root's box against each plane; when it is cut by a plane the tree is walked (`0x433600`), when it is wholly inside everything is taken (`0x432f50`); nothing when it is outside one |
| `0x44c450` | QuadTree::Cull | Culls the tree against six planes (the camera's view): updates the bounds first when needed (`0x430770`), then tests the root's box against each plane; when it is cut by a plane the tree is walked (`0x44bd20`), when it is wholly inside everything is taken (`0x44ba10`); nothing when it is outside one |
| `0x488e50` | QuadTree::QuadTree | Constructor of a four-way spatial tree (a quadtree) over the area between the two corners given: keeps its owner, makes the node array (28 bytes per node, how many from `0xCDB5E0`) and sets the root square (+0xC origin, +0x14 size) to twice the area, centred on it |
| `0x4891a0` | QuadTree::UpdateBounds | Brings the bounds of node n up to date when it is marked changed (flag 0x10): updates its changed children (nodes 4n+1 to 4n+4) first, notes which children hold anything (bits 0-3), and takes the lowest and highest values of its children and of its own items (32 bytes each) as its range (+4, +8); then clears the mark |
| `0x489ab0` | QuadTree::Insert | Puts an item into the tree: finds the node that holds its rectangle (`0xCDB600`), adds the item there (`0x489030`) and marks that node and those above it changed (`0x488320`, QuadTree::MarkChanged); gives the node in the high 16 bits and the item's slot in the low ones |
| `0x48bb30` | QuadTree::Cull | Culls the tree against six planes (the camera's view): updates the bounds first when needed (`0x4891a0`), then tests the root's box against each plane; when it is cut by a plane the tree is walked (`0x48b0b0`), when it is wholly inside everything is taken (`0x48a4e0`); nothing when it is outside one |
| `0x48c460` | QuadTree::Cull | Culls the tree against six planes (the camera's view): updates the bounds first when needed (`0x4891a0`), then tests the root's box against each plane; when it is cut by a plane the tree is walked (`0x48bdc0`), when it is wholly inside everything is taken (`0x48b370`); nothing when it is outside one |
| `0x7b3e60` | QuadTree::QuadTree | Constructor of a four-way spatial tree (a quadtree) over the area between the two corners given: keeps its owner, makes the node array (28 bytes per node, how many from `0xCDB5E0`) and sets the root square (+0xC origin, +0x14 size) to twice the area, centred on it |
| `0x7b41b0` | QuadTree::UpdateBounds | Brings the bounds of node n up to date when it is marked changed (flag 0x10): updates its changed children (nodes 4n+1 to 4n+4) first, notes which children hold anything (bits 0-3), and takes the lowest and highest values of its children and of its own items (32 bytes each) as its range (+4, +8); then clears the mark |
| `0x7b53a0` | QuadTree::Insert | Puts an item into the tree: finds the node that holds its rectangle (`0xCDB600`), adds the item there (`0x7b4040`) and marks that node and those above it changed (`0x7b2a40`, QuadTree::MarkChanged); gives the node in the high 16 bits and the item's slot in the low ones |
| `0x7b7d40` | QuadTree::Cull | Culls the tree against six planes (the camera's view): updates the bounds first when needed (`0x7b41b0`), then tests the root's box against each plane; when it is cut by a plane the tree is walked (`0x7b6eb0`), when it is wholly inside everything is taken (`0x7b5540`); nothing when it is outside one |
| `0x7b90a0` | QuadTree::Cull | Culls the tree against six planes (the camera's view): updates the bounds first when needed (`0x7b41b0`), then tests the root's box against each plane; when it is cut by a plane the tree is walked (`0x7b7f30`), when it is wholly inside everything is taken (`0x7b6bd0`); nothing when it is outside one |
| `0x7b91d0` | QuadTree::Cull | Culls the tree against six planes (the camera's view): updates the bounds first when needed (`0x7b41b0`), then tests the root's box against each plane; when it is cut by a plane the tree is walked (`0x7b82d0`), when it is wholly inside everything is taken (`0x7b6d80`); nothing when it is outside one |
| `0x7b93b0` | QuadTree::Cull | Culls the tree against six planes (the camera's view): updates the bounds first when needed (`0x7b41b0`), then tests the root's box against each plane; when it is cut by a plane the tree is walked (`0x7b8920`), when it is wholly inside everything is taken (`0x7b7390`); nothing when it is outside one |
| `0x7b9d70` | QuadTree::Cull | Culls the tree against six planes (the camera's view): updates the bounds first when needed (`0x7b41b0`), then tests the root's box against each plane; when it is cut by a plane the tree is walked (`0x7b9590`), when it is wholly inside everything is taken (`0x7b85f0`); nothing when it is outside one |
| `0xa0c410` | QuadTree::QuadTree | Constructor of a four-way spatial tree (a quadtree) over the area between the two corners given: keeps its owner, makes the node array (28 bytes per node, how many from `0xCDB5E0`) and sets the root square (+0xC origin, +0x14 size) to twice the area, centred on it |
| `0xa0c730` | QuadTree::UpdateBounds | Brings the bounds of node n up to date when it is marked changed (flag 0x10): updates its changed children (nodes 4n+1 to 4n+4) first, notes which children hold anything (bits 0-3), and takes the lowest and highest values of its children and of its own items (32 bytes each) as its range (+4, +8); then clears the mark |
| `0xa0d6d0` | QuadTree::Insert | Puts an item into the tree: finds the node that holds its rectangle (`0xCDB600`), adds the item there (`0xa0c590`) and marks that node and those above it changed (`0xa0be40`, QuadTree::MarkChanged); gives the node in the high 16 bits and the item's slot in the low ones |
| `0xa0e960` | QuadTree::Cull | Culls the tree against six planes (the camera's view): updates the bounds first when needed (`0xa0c730`), then tests the root's box against each plane; when it is cut by a plane the tree is walked (`0xa0e450`), when it is wholly inside everything is taken (`0xa0e240`); nothing when it is outside one |
| `0xb5bf50` | QuadTree::QuadTree | Constructor of a four-way spatial tree (a quadtree) over the area between the two corners given: keeps its owner, makes the node array (28 bytes per node, how many from `0xCDB5E0`) and sets the root square (+0xC origin, +0x14 size) to twice the area, centred on it |
| `0xb5c2e0` | QuadTree::UpdateBounds | Brings the bounds of node n up to date when it is marked changed (flag 0x10): updates its changed children (nodes 4n+1 to 4n+4) first, notes which children hold anything (bits 0-3), and takes the lowest and highest values of its children and of its own items (32 bytes each) as its range (+4, +8); then clears the mark |
| `0xb5cc00` | QuadTree::Insert | Puts an item into the tree: finds the node that holds its rectangle (`0xCDB600`), adds the item there (`0xb5c170`) and marks that node and those above it changed (`0xb5b5d0`, QuadTree::MarkChanged); gives the node in the high 16 bits and the item's slot in the low ones |
| `0xb5eb80` | QuadTree::Cull | Culls the tree against six planes (the camera's view): updates the bounds first when needed (`0xb5c2e0`), then tests the root's box against each plane; when it is cut by a plane the tree is walked (`0xb5e460`), when it is wholly inside everything is taken (`0xb5d780`); nothing when it is outside one |
| `0xb5f3b0` | QuadTree::Cull | Culls the tree against six planes (the camera's view): updates the bounds first when needed (`0xb5c2e0`), then tests the root's box against each plane; when it is cut by a plane the tree is walked (`0xb5ed60`), when it is wholly inside everything is taken (`0xb5e950`); nothing when it is outside one |
| `0xb6d090` | QuadTree::QuadTree | Constructor of a four-way spatial tree (a quadtree) over the area between the two corners given: keeps its owner, makes the node array (28 bytes per node, how many from `0xCDB5E0`) and sets the root square (+0xC origin, +0x14 size) to twice the area, centred on it |
| `0xb6d510` | QuadTree::UpdateBounds | Brings the bounds of node n up to date when it is marked changed (flag 0x10): updates its changed children (nodes 4n+1 to 4n+4) first, notes which children hold anything (bits 0-3), and takes the lowest and highest values of its children and of its own items (32 bytes each) as its range (+4, +8); then clears the mark |
| `0xb70fa0` | QuadTree::Insert | Puts an item into the tree: finds the node that holds its rectangle (`0xCDB600`), adds the item there (`0xb6d3a0`) and marks that node and those above it changed (`0xb6b880`, QuadTree::MarkChanged); gives the node in the high 16 bits and the item's slot in the low ones |
| `0xb74e80` | QuadTree::Cull | Culls the tree against six planes (the camera's view): updates the bounds first when needed (`0xb6d510`), then tests the root's box against each plane; when it is cut by a plane the tree is walked (`0xb74580`), when it is wholly inside everything is taken (`0xb732b0`); nothing when it is outside one |
| `0xb756b0` | QuadTree::Cull | Culls the tree against six planes (the camera's view): updates the bounds first when needed (`0xb6d510`), then tests the root's box against each plane; when it is cut by a plane the tree is walked (`0xb75060`), when it is wholly inside everything is taken (`0xb74250`); nothing when it is outside one |
| `0x430520` | QuadTree::Node::Add | Adds an item to a tree node: grows the node's list of 32-byte items when it is full (to 8, then doubling, the new ones constructed and the old ones copied over), then stores the item's box (its two corners) and the item; gives its slot |
| `0x431240` | QuadTree::Move | Updates an item whose box has changed: finds the node for the new box (`0xCDB600`); when it is the same node, rewrites the item's box there and marks the node changed; otherwise frees its slot in the old node (SlotTable::Free), marks the old node, adds it to the new one (QuadTree::Node::Add), gives back the new node and slot, and marks the new node |
| `0x489030` | QuadTree::Node::Add | Adds an item to a tree node: grows the node's list of 32-byte items when it is full (to 8, then doubling, the new ones constructed and the old ones copied over), then stores the item's box (its two corners) and the item; gives its slot |
| `0x489b30` | QuadTree::Move | Updates an item whose box has changed: finds the node for the new box (`0xCDB600`); when it is the same node, rewrites the item's box there and marks the node changed; otherwise frees its slot in the old node (SlotTable::Free), marks the old node, adds it to the new one (QuadTree::Node::Add), gives back the new node and slot, and marks the new node |
| `0x7b4040` | QuadTree::Node::Add | Adds an item to a tree node: grows the node's list of 32-byte items when it is full (to 8, then doubling, the new ones constructed and the old ones copied over), then stores the item's box (its two corners) and the item; gives its slot |
| `0x7b5420` | QuadTree::Move | Updates an item whose box has changed: finds the node for the new box (`0xCDB600`); when it is the same node, rewrites the item's box there and marks the node changed; otherwise frees its slot in the old node (SlotTable::Free), marks the old node, adds it to the new one (QuadTree::Node::Add), gives back the new node and slot, and marks the new node |
| `0xa0c590` | QuadTree::Node::Add | Adds an item to a tree node: grows the node's list of 32-byte items when it is full (to 8, then doubling, the new ones constructed and the old ones copied over), then stores the item's box (its two corners) and the item; gives its slot |
| `0xa0d750` | QuadTree::Move | Updates an item whose box has changed: finds the node for the new box (`0xCDB600`); when it is the same node, rewrites the item's box there and marks the node changed; otherwise frees its slot in the old node (SlotTable::Free), marks the old node, adds it to the new one (QuadTree::Node::Add), gives back the new node and slot, and marks the new node |
| `0xb5c170` | QuadTree::Node::Add | Adds an item to a tree node: grows the node's list of 32-byte items when it is full (to 8, then doubling, the new ones constructed and the old ones copied over), then stores the item's box (its two corners) and the item; gives its slot |
| `0xb5cc80` | QuadTree::Move | Updates an item whose box has changed: finds the node for the new box (`0xCDB600`); when it is the same node, rewrites the item's box there and marks the node changed; otherwise frees its slot in the old node (SlotTable::Free), marks the old node, adds it to the new one (QuadTree::Node::Add), gives back the new node and slot, and marks the new node |
| `0xb6d3a0` | QuadTree::Node::Add | Adds an item to a tree node: grows the node's list of 32-byte items when it is full (to 8, then doubling, the new ones constructed and the old ones copied over), then stores the item's box (its two corners) and the item; gives its slot |
| `0xb71020` | QuadTree::Move | Updates an item whose box has changed: finds the node for the new box (`0xCDB600`); when it is the same node, rewrites the item's box there and marks the node changed; otherwise frees its slot in the old node (SlotTable::Free), marks the old node, adds it to the new one (QuadTree::Node::Add), gives back the new node and slot, and marks the new node |
| `0xcdb600` | QuadTree::FindNode | The node of a loose four-way tree that should hold a box: starting at the root, goes down while the box is no bigger than a quarter of the node in both directions, choosing the child by which half of the node the box's middle lies in (children overlap: each covers half the node, offset by a quarter); stops at the last level |
| `0x42fa40` | Frustum::FromCamera | Copies the camera's six clipping planes (each a normal and its distance, negated; from +0x10, 32 bytes apart) into a culling query and keeps the value given |
| `0x432f50` | QuadTree::TakeAll | Walks a node wholly inside the view and the nodes below it: takes each of its items that is within the item's own draw distance (+0x20) of the camera and passes its facing test, collecting them with `0x430D00` |
| `0x433600` | QuadTree::TakeVisible | Walks a node cut by the view: tests each item's box against the planes, taking those inside, then goes on into the children, those wholly inside with QuadTree::TakeAll and the rest with itself; items are collected with `0x430D00` |
| `0x449320` | Frustum::FromCamera | Copies the camera's six clipping planes (each a normal and its distance, negated; from +0x10, 32 bytes apart) into a culling query and keeps the value given |
| `0x488520` | Frustum::FromCamera | Copies the camera's six clipping planes (each a normal and its distance, negated; from +0x10, 32 bytes apart) into a culling query and keeps the value given |
| `0x7b2cf0` | Frustum::FromCamera | Copies the camera's six clipping planes (each a normal and its distance, negated; from +0x10, 32 bytes apart) into a culling query and keeps the value given |
| `0xa0bfd0` | Frustum::FromCamera | Copies the camera's six clipping planes (each a normal and its distance, negated; from +0x10, 32 bytes apart) into a culling query and keeps the value given |
| `0xb5b8d0` | Frustum::FromCamera | Copies the camera's six clipping planes (each a normal and its distance, negated; from +0x10, 32 bytes apart) into a culling query and keeps the value given |
| `0xb6baa0` | Frustum::FromCamera | Copies the camera's six clipping planes (each a normal and its distance, negated; from +0x10, 32 bytes apart) into a culling query and keeps the value given |
| `0x44ba10` | QuadTree::TakeAll | The take-all walk of the tree culled by `0x44c450`: goes through a node wholly inside the view and the nodes below it, taking their items, collecting items with `0x44a120` |
| `0x44bd20` | QuadTree::TakeVisible | The partial walk of the tree culled by `0x44c450`: tests the items of a node cut by the view against the planes and goes on into its children (those wholly inside with `0x44ba10`, the rest with itself), collecting items with `0x44a120` |
| `0x48a4e0` | QuadTree::TakeAll | The take-all walk of the tree culled by `0x48bb30`: goes through a node wholly inside the view and the nodes below it, taking their items |
| `0x48b0b0` | QuadTree::TakeVisible | The partial walk of the tree culled by `0x48bb30`: tests the items of a node cut by the view against the planes and goes on into its children (those wholly inside with `0x48a4e0`, the rest with itself) |
| `0x48b370` | QuadTree::TakeAll | The take-all walk of the tree culled by `0x48c460`: goes through a node wholly inside the view and the nodes below it, taking their items |
| `0x48bdc0` | QuadTree::TakeVisible | The partial walk of the tree culled by `0x48c460`: tests the items of a node cut by the view against the planes and goes on into its children (those wholly inside with `0x48b370`, the rest with itself) |
| `0x7b5540` | QuadTree::TakeAll | The take-all walk of the tree culled by `0x7b7d40`: goes through a node wholly inside the view and the nodes below it, taking their items, collecting items with `0x758b10` |
| `0x7b6eb0` | QuadTree::TakeVisible | The partial walk of the tree culled by `0x7b7d40`: tests the items of a node cut by the view against the planes and goes on into its children (those wholly inside with `0x7b5540`, the rest with itself), collecting items with `0x758b10` |
| `0x7b6bd0` | QuadTree::TakeAll | The take-all walk of the tree culled by `0x7b90a0`: goes through a node wholly inside the view and the nodes below it, taking their items, collecting items with `0x417a20` |
| `0x7b7f30` | QuadTree::TakeVisible | The partial walk of the tree culled by `0x7b90a0`: tests the items of a node cut by the view against the planes and goes on into its children (those wholly inside with `0x7b6bd0`, the rest with itself), collecting items with `0x417a20` |
| `0x7b6d80` | QuadTree::TakeAll | The take-all walk of the tree culled by `0x7b91d0`: goes through a node wholly inside the view and the nodes below it, taking their items, collecting items with `0x72af00` |
| `0x7b82d0` | QuadTree::TakeVisible | The partial walk of the tree culled by `0x7b91d0`: tests the items of a node cut by the view against the planes and goes on into its children (those wholly inside with `0x7b6d80`, the rest with itself), collecting items with `0x72af00` |
| `0x7b7390` | QuadTree::TakeAll | The take-all walk of the tree culled by `0x7b93b0`: goes through a node wholly inside the view and the nodes below it, taking their items |
| `0x7b8920` | QuadTree::TakeVisible | The partial walk of the tree culled by `0x7b93b0`: tests the items of a node cut by the view against the planes and goes on into its children (those wholly inside with `0x7b7390`, the rest with itself) |
| `0x7b85f0` | QuadTree::TakeAll | The take-all walk of the tree culled by `0x7b9d70`: goes through a node wholly inside the view and the nodes below it, taking their items |
| `0x7b9590` | QuadTree::TakeVisible | The partial walk of the tree culled by `0x7b9d70`: tests the items of a node cut by the view against the planes and goes on into its children (those wholly inside with `0x7b85f0`, the rest with itself) |
| `0xa0e240` | QuadTree::TakeAll | The take-all walk of the tree culled by `0xa0e960`: goes through a node wholly inside the view and the nodes below it, taking their items, collecting items with `0xa0a330` |
| `0xa0e450` | QuadTree::TakeVisible | The partial walk of the tree culled by `0xa0e960`: tests the items of a node cut by the view against the planes and goes on into its children (those wholly inside with `0xa0e240`, the rest with itself), collecting items with `0xa0a330` |
| `0xb5d780` | QuadTree::TakeAll | The take-all walk of the tree culled by `0xb5eb80`: goes through a node wholly inside the view and the nodes below it, taking their items |
| `0xb5e460` | QuadTree::TakeVisible | The partial walk of the tree culled by `0xb5eb80`: tests the items of a node cut by the view against the planes and goes on into its children (those wholly inside with `0xb5d780`, the rest with itself) |
| `0xb5e950` | QuadTree::TakeAll | The take-all walk of the tree culled by `0xb5f3b0`: goes through a node wholly inside the view and the nodes below it, taking their items, collecting items with `0xb563b0` |
| `0xb5ed60` | QuadTree::TakeVisible | The partial walk of the tree culled by `0xb5f3b0`: tests the items of a node cut by the view against the planes and goes on into its children (those wholly inside with `0xb5e950`, the rest with itself), collecting items with `0xb563b0` |
| `0xb732b0` | QuadTree::TakeAll | The take-all walk of the tree culled by `0xb74e80`: goes through a node wholly inside the view and the nodes below it, taking their items |
| `0xb74580` | QuadTree::TakeVisible | The partial walk of the tree culled by `0xb74e80`: tests the items of a node cut by the view against the planes and goes on into its children (those wholly inside with `0xb732b0`, the rest with itself) |
| `0xb74250` | QuadTree::TakeAll | The take-all walk of the tree culled by `0xb756b0`: goes through a node wholly inside the view and the nodes below it, taking their items, collecting items with `0xb44a30` |
| `0xb75060` | QuadTree::TakeVisible | The partial walk of the tree culled by `0xb756b0`: tests the items of a node cut by the view against the planes and goes on into its children (those wholly inside with `0xb74250`, the rest with itself), collecting items with `0xb44a30` |
| `0x92b9e0` | Water::AskTileChange | When a change to the ground would affect a waterfall, asks the player first, with the game text `Water_AllowTileChangeResponse_WaterfallPrompt` (in a message window), the water manager made when needed |
| `0xcb0e70` | Shader::DumpBad | Writes a shader that failed to compile to `ShaderBad.sha`, between `Shader Start` and `Shader End` lines |
| `0xc484a0` | ShaderCache::Save | Writes the compiled shader cache to `SCCache.bin` (through Writer::Write4 and the writer helpers) |
| `0x9256a0` | Waterfall::ConfirmDelete | Asks before deleting a waterfall, with the game text `Waterfall_Delete` (in a message window), keeping a reference to the waterfall for the answer |
| `0x924f60` | Waterfall::ShowLandNotOwned | Shows the game text `Waterfall_DeletionFailure_LandNotOwned` (in a message window) when a waterfall cannot be deleted because the land is not the park's |
| `0x87f5b0` | Model::ReportMissingLOD | Shows a message box `Missing mesh data for LOD` with the model's name (+4) |
| `0xbe6d40` | TextureResource::TextureResource | Makes a texture resource from a texture style (looked up by name in the map at `0x13D0E04`), with the size, mipmap and format settings given; creates its textures on the device (Device::CreateTexture) |
| `0x9fe380` | Terrain::MakeLightFalloffTexture | Makes the procedural texture `TerrainLightFalloff` (32 x 32, kept at +0x8C): for each pixel the light falls off with the squared distance from the middle, 0 outside the circle |
| `0x8ed250` | Terrain::Build | In outline: builds the terrain's drawing data (4.5 KB), marking `BLOCKED` cells, through the terrain and the current device |
| `0xb57590` | Shadows::DrawBlobs | In outline: draws the round shadow blobs under people (`PeopleShadowBlob`), placed with their facing (Matrix::LookAlongZSide) |
| `0xb83510` | Shadows::BlobSetup | In outline: sets up the `PeopleShadowBlob` shadow texture and its values |
| `0x9493a0` | Water::DrawDebug | In outline: the water and waterfall drawing pass with its debug drawing (`DrawSolidColourNoZ`), counting its draw calls (RenderStats::AddBatch) |
| `0x43a0c0` | Renderer::SetTextureStages | In outline: sets up the eight texture stages (`Texture0`-`Texture7`) for drawing from the main camera |
| `0x87ca90` | Billboards::Draw | In outline: draws billboards with the `BillboardStandard` shader (looked up by name) through Shape::Draw |
| `0x83b8a0` | Billboards::Setup | In outline: sets up a billboard list with the `BillboardStandard` shader |
| `0xccdd90` | ResourceFile::Load | In outline: loads a resource file's data, swapping byte order when the file's flag (`0x13D0DF2`) says so, marking `BLOCKED` entries |
| `0xbfb7b0` | ResourceFile::Save | In outline: writes a resource's data to a save stream (SaveWriter::Write), marking `BLOCKED` entries |
| `0x486a00` | Billboards::Setup2 | In outline: sets up another billboard list with the `BillboardStandard` shader |
| `0xb49b70` | Shadows::BlobCheck | In outline: checks people's shapes (Model::CheckShapes) and sets up the `PeopleShadowBlob` shadow |
| `0x9a3f10` | Fireworks::FilePath | In outline: builds a firework file's path in the `Fireworks/` folder |
| `0x9ea300` | Weather::StartLoop | In outline: starts the weather's looping sound (`WeatherLoop`), loading the rain when it rains |
| `0x54c8b0` | Ride::ParticleSound | In outline: plays a ride's particle sound (`RideParticleSound`) |
| `0x8ee760` | Terrain::ShadowHorizon | In outline: sets up the terrain's shadow horizon (`TerrainShadowHorizon`, a texture style looked up by name) and rebuilds the terrain |
| `0x8e9e40` | Terrain::MarkBlocked | In outline: goes over the terrain on the current device, marking `BLOCKED` cells |
| `0x63e310` | GUIIcon::Window | In outline: a window showing a `GUIIcon:txs` texture (TextureResource) |
| `0xc69ab0` | ScrollBar::Draw | In outline: draws a `Scrollbar` in its colours (Colour::ToRGB) |
| `0x438c50` | GUIRenderer::Bitmap | In outline: sets up the GUI renderer's bitmap drawing (`GUIRendererBitmap`, looked up by name) |
| `0x86e1d0` | SceneryItem::VisualProxyReport | In outline: reports a scenery item's visual proxy (`SceneryItemVisualProxy`) for the developer (Model::DebugReport) |
| `0xafd600` | Terrain::HoleStencil | In outline: sets up the terrain hole stencil (`TerrainHoleStencil`, looked up by name) as a pooled object |
| `0x9bfd70` | ParticleEffects::Flare | In outline: makes the `Flare_00` particle effect (ParticleDesc and its emitter) |
| `0x85ae60` | Shadows::ShapeShadow | In outline: sets up shape shadows (`ShapeShadow`, looked up by name) |
| `0x482b70` | ScreenDistortion::ScreenDistortion | In outline: the screen distortion effect (`ScreenDistortion`, looked up by name), registered as a list member, with its option and random start |
| `0x42db70` | ShaderSetup::SIOpaque | In outline: looks up the `SIOpaque` shader by name |
| `0x46b5a0` | MoviePlayback::Setup | In outline: looks up the `MoviePlayback` shader by name |
| `0x90b640` | Water::Puddle | In outline: sets up puddles (`Puddle`, looked up by name) through the water manager |
| `0xbd4e20` | ResourceTable::MarkBlocked | In outline: marks `BLOCKED` entries of a resource table |
| `0x753630` | Billboards::Setup3 | In outline: sets up a billboard list with the `BillboardStandard` shader |
| `0x97bba0` | ShaderSetup::SIFillZ | In outline: compares a shader's name with `SIFillZ` |
| `0x96a3d0` | ShaderSetup::SIFillZ2 | In outline: compares a shader's name with `SIFillZ` |
| `0x836aa0` | StyleWindow::Name | In outline: the style name text (`StyleName`) |
| `0x465860` | Ad::Texture | Looks up the `AdTexture` texture style by name |
| `0x93c390` | Debug::DrawSolidColourNoZ | Looks up the `DrawSolidColourNoZ` shader (as a list member) |
| `0x4823c0` | Debug::DrawSolidColourNoZ2 | Looks up the `DrawSolidColourNoZ` shader |
| `0x45b400` | GUIIcon::Load2 | In outline: loads a `GUIIcon:txs` texture |
| `0x6b5800` | GUIIcon::Load3 | In outline: loads a `GUIIcon:txs` texture |
| `0x7aea40` | Terrain::SelectedGrid | In outline: the selected terrain grid's drawing (`TerrainSelectedGrid`) |
| `0x958dc0` | Water::Reflection | Looks up the `WaterReflection` shader by name and sets it up |
| `0x430c10` | FlatTag::Setup | Looks up the `FlatTag` shader by name for the GUI renderer |
| `0x447530` | Terrain::Grid2Stage | Looks up the `TerrainGrid2Stage` shader by name |
| `0x92fc60` | Debug::NumPolys | Writes `NumPolys: %d` for each item of a list (developer text) |
| `0x463640` | Ad::StandardBitmap | The `StandardAdBitmap` texture's first texture |
| `0x9f8130` | Fireworks::Flare2 | In outline: the `Flare2_00` firework flare (as a list member) |
| `0x8fcf30` | Fireworks::Flares | In outline: the `FWFlares01_02` flares |
| `0x9bfb70` | ParticleEffects::Find | In outline: compares names with `ParticleEffects` |
| `0x57bb10` | ParticleEffects::Path | In outline: builds a path in `ParticleEffects` |
| `0x9a5c00` | Fireworks::MixMasterFile | In outline: the `MixMaster Display.fwd` file |
| `0x43a730` | Movies::Attract | The `Movies//Attract` folder name |
| `0x9c6210` | Particle::SetColours | Sets a particle description's two colours (+0xAC and +0xB8) from two 0xRRGGBB values, each part from 0 to 1 |
| `0xbd4060` | Vertex::SizeFromFormat | The size in bytes of a vertex from its format bits: the position (12 or 16), normal and colour parts, texture coordinates (8 bytes each, count in bits 12 to 15) and the rest |
| `0x42ff50` | Points::UpdateDepth | For point n (0x70 bytes each) sets its depth (+0x24) from the sum of its two vectors and the view matrix, the point at +0x40 from the first vector plus the second scaled, and marks it done (+0x28) |
| `0xbe8ba0` | Mesh::SetPrimitiveCount | Sets a mesh part's vertex count (from its buffers when given 0x40000000) and from it the number of primitives by its type: 0 points, 1 lines (/2), 2 a line strip (-1), 3 triangles (/3), 4 and 5 a strip or fan (-2) |
| `0x7aeb50` | Mask::ToPixels | Writes the rows of a bit mask into an image: white with full alpha for set bits, white with no alpha for clear ones |
| `0x911c80` | Animation::FrameAt | Advances an animation's time (+0x20, wrapping at 1) and from it the frame: the start and end of the frame's span (+0x24, +0x28) and how far into it (+0x2C) |
| `0x9cbbb0` | Particles::SetCount | Sets the number of particles to draw (+0xBC), scaled by the detail setting and a fade (+0xB8), and the spacing between them (+0xC0) |
| `0x7c3d50` | Fog::Setup | Sets the global fog values: one over the distance given (or -1 when it is too small), and from the object given either its single value (+0x198, when +0x1A6 is set) or its three colour values (+0x160 to +0x168) |
| `0xc946d0` | Renderer::SetMaterial | When the material given changes: keeps it and its texture set (+0x28), clears the cached states and points one slot to each of its textures (0x14 bytes each, count +0xCA), marking them unset |
| `0x8f35b0` | Terrain::LodDistance | A terrain patch's distance for detail: half its smaller side plus its cell size times (2 to the level minus 1), capped |
| `0x8f8660` | Sound::SetVolumeScale | Sets the volume scale of the current sound (`0x1339F58`, kept between 0.2 and 1) and its volume from its kind's level (`0x10EDD40`), or 0 when that kind is muted (`0x12F0168`) |
| `0x6b6400` | Terrain::SlopeShade | A grey for a terrain cell (0x18 bytes each) from the height difference across it, from black up to `0x7F7F7F` |
| `0xccf9f0` | Vertices::Gather | Builds n vertices of 32 bytes: a position from one table and a normal from another (by the two indices of each source entry) and texture coordinates from the entry plus an offset |
| `0xccf970` | Vertices::Gather2 | The same vertex gathering as Vertices::Gather (another copy) |
| `0x959f30` | Light::FadeAlpha | An alpha for a point from its distance to a light's centre (+0x3C, +0x40): full inside its near radius (+0x44), none beyond the far one (+0x48), times its strength (+0x38), in the top byte of a colour |
| `0xcc68c0` | Nodes::MatrixFor | The matrix (64 bytes) for node n, from the local list (+0x18) or the shared one (+4) by the node's flags, and which one it was |
| `0x8fdb30` | Quad::UpdateCounts | Sets a shape's vertex and index counts (+0x18, +0x1C) to 4 and 6 when all four of its corners are set, otherwise 0, and keeps the totals `0x133A028` and `0x133A02C` up to date |
| `0x47c3f0` | Sound::MuteKind | Counts a mute on a kind of sound (`0x12F0168`, or sets it to 10000 when asked) and sets each sound of that kind to silent |
| `0x9d9c40` | Sizes::Pick | A size in pixels from two choices: one of three bases (0 to 2) times one of three scales (1, 0.5 or `0xF28420`) |
| `0x7c3080` | Value::Band2 | Which band a value falls in by fixed limits (1, 2, ...) |
| `0xa35440` | Lights::ColourFor | The colour (4 values) of the light with the id given among the object's lights (+0xF0, count +0x108), or all 0 |
| `0xceb590` | Materials::CompareOpacity | A sort comparison of two drawn parts by their materials' opacity (+0x54, 1 when not see-through), for parts that have one (+0x58) |
| `0x47c470` | Sound::UnmuteKind | Counts a mute off a kind of sound (`0x12F0168`, to 0 when asked or below 0) and gives each sound of that kind its volume back (pairs with Sound::MuteKind) |
| `0x47dd10` | Sound::SetLevels | Sets two levels of a sound (+0x5C, +0x60), each kept between 0 and 1 |
| `0x9e84e0` | Colour::Pack | Packs a colour of four values from 0 to 1 into one value (alpha, red, green, blue) at +8 |
| `0x489a10` | Mesh::PointAndNormal | The position and normal of a mesh point by its packed number (part in the high half, point in the low half) |
| `0x8ef180` | Vertices::SetSecond | Copies a list of values into the second value of each 16-byte vertex (count twice the sum of +8 and +0xC) |
| `0x94dec0` | Animations::Set | Sets animation n (0xD0 bytes each at +0x1A4): its three values, frame count and rate, the frame time ((count - 1) / rate), its mode (4) and a cleared position |
| `0x9f7110` | Firework::SetFade | Sets a firework's fade value (+0xC) from a level between 0 and 1, along a curve |
| `0xce45f0` | Sound::VolumeToHundredthsDb | Turns a volume from 0 to 1 into hundredths of a decibel for DirectSound: -10000 (silent) below a floor, 0 at 1 or more, otherwise from log10 of it |
| `0xc193e0` | Sound::VolumeToHundredthsDb2 | The same volume conversion (another copy) |
| `0xb96f90` | Colour::PackArgs | Packs a colour from four values between 0 and 1 (red, green, blue, alpha) into one value (alpha, red, green, blue) |
| `0x856640` | Draw::CallByType | Calls the drawing routine for an item's type from the table `0x10F2DB0` (eight per mode `0x13D2888`), with a default rectangle of 0, 0, 1, 1 |
| `0x94de60` | Animations::SetMove | Sets animation n (0xD0 bytes each at +0x1A4) to mode 3, moving from one point to another |
| `0x425700` | Colour::PackTo | Packs a colour of four values from 0 to 1 into one value (alpha, red, green, blue) |
| `0x8645c0` | Model::Bounds | A model's box: the stored corners when its bit 10 is set (none when empty), otherwise worked out (`0x864150`) |
| `0x9eb350` | Sequence::Advance | Moves a running sequence's time on (+4) and steps to its next item (12 bytes each at +0x14) when its time passes, starting it (`0x9EAF50`) or ending the sequence (time -1) after the last |
| `0x9c6330` | Particle::SecondColour | Packs a particle description's second colour (+0xB8) into one opaque value |
| `0x9c62d0` | Particle::FirstColour | Packs a particle description's first colour (+0xAC) into one opaque value |
| `0xbd3190` | Updatable::SetActive | Switches an object on or off (+0x11): joins the shared update list (`0x13D0D84`) when it has work (+0x24), or leaves it (`0xBD2850`) |
| `0xbe0a50` | VertexDeclaration::Build | In outline: builds a vertex layout for a shader by adding elements (`0xBDC770`, each a stream byte and a usage) by the kinds of the shader's inputs (2, 0xC to 0x10, 0x16, 0x17, 0x19, 0x1A, ...) |
| `0x9148a0` | Water::Build9148A0 | In outline: works through a grid of water cells building its geometry and colours, with the water manager (WaterManager::Get) and the developers' draw shader (Debug::DrawSolidColourNoZ2) |
| `0x91fa80` | Water::Trace91FA80 | In outline: from a point, finds the terrain cell (Grid::CellAt) and goes over the water system's bodies (+0x1B4, +0x138), measuring directions and distances case by case |
| `0x9225b0` | Water::UpdateWaterfalls | In outline: updates the waterfalls (Waterfall::Update) or, over the water system's bodies (+0x1B0, +0x1B4), checks each cell's edges against the terrain (`0x8FB1F0`) and makes waterfalls where water spills (WaterfallManager::Get, `0x925DD0`) |
| `0x904170` | Water::DrawJets904170 | In outline: draws and moves water effects: for each one (kinds 4, 5 and 8) sets its animation (`0x94DE10`, Animations::SetMove), picks a detail level (`0x13D3184`) and builds geometry over the terrain |
| `0x9f2f20` | Fireworks::Draw9F2F20 | In outline: draws a firework part: works out its points (`0x9EEA50`), turns them by its matrices and draws them with a shared shader (`0x10F2FA0`, `0x1341C90`) |
| `0x94b170` | Water::DrawReflection94B170 | In outline: when there is water to draw, sets up a scaled view (Matrix::ScaleOffset2D) turned by the camera's angle and draws the water's reflection pass (`0x9FBA50`), unless turned off (`0x12F018C` +0xB0) |
| `0x85bd40` | Shadows::Render85BD40 | In outline: renders the scenery shadows: works out the light's view (`0x85CB30`, `0x864760`) with an orthographic matrix (Matrix::Ortho), sizes it to the shadow texture, and draws them (SceneryRenderer::DrawShadows) inside a timed critical section |
| `0x902b80` | Water::BuildMesh902B80 | In outline: builds water mesh strips for the current device (ForCurrent(3)): gathers the cells (`0x9084E0`, `0x908940`, `0x908470`) and fills vertex and index buffers (`0xBFFB60`) |
| `0x8f7c00` | Terrain::DrawPass8F7C00 | In outline: draws a terrain pass: sets a scaled view (Matrix::ScaleOffset2D), draws its batches (`0x8F7750`) counting them (Counter::Increment, RenderStats::AddBatch), one per layer (`0x1311620`) |
| `0xc9d530` | Particles::DrawC9D530 | In outline: draws a particle system: works out its fade (an alpha from +0x17C), sets up the matrices and fills its vertex batches (`0xC9D0F0`) |
| `0xc8da50` | Device::PickFormatC8DA50 | In outline: goes over a list of formats or modes and picks the one that fits the device's version numbers (+0x2080, +0x2084: above 0x33, under 0x36, above 0x2F), then sets it up (`0xC73150`, `0xC8BA70`) |
| `0x9447b0` | Water::Draw9447B0 | In outline: draws the water: when there is any (+0x68 vs +0x74, or `0x95DC50`), sets up the views (`0xCD7D00` on `0x1400160`), and for each body within view (screen positions between -1 and 1) draws its pieces (`0x941C00`, `0x93DF70`) |
| `0xaf8fb0` | Mesh::FillAF8FB0 | In outline: fills a vertex and index buffer for the current device (ForCurrent(7), `0xBFFB60`) from the points given |
| `0x93f4f0` | Water::MakeNormalTexture | In outline: makes a 64 by 64 texture (Device::CreateTexture) from a 128 by 128 grid of wave values: works out each one's slope (arctangent, square roots) and writes it as a colour |
| `0x93e840` | Water::MakeWaveTexture | In outline: like Water::MakeNormalTexture, makes a 64 by 64 texture from a 128 by 128 grid of wave values, with a ring falloff around a radius |
| `0xa93a50` | Grid::DrawOverlayA93A50 | In outline: draws a coloured grid overlay over the terrain (colour 0x4CFFFF00, yellow at low alpha) for the current device: fills lines along each row and column (`0xA91D70`) |
| `0xc9c490` | Particles::EmitC9C490 | In outline: makes particles along a moving emitter by how far it went (steps by several distance limits): takes records from a free list (FreeList13E6DF4::Take), points them along the movement (Matrix::LookAlongZSide) and sets their corners (`0xC9B970`) |
| `0x9cbdf0` | Particles::Spawn9CBDF0 | In outline: when a time window (+0x58 to +0x5C) is crossed, makes a number of particles (+0xBC), each turned by its own random spins about x, y and z (+0x88 to +0x9C, Matrix::RotateX2, Matrix::TurnAboutY, Matrix::RotateZ2) |
| `0xc9ef70` | Batches::BuildC9EF70 | In outline: builds the draw batches of a model for its parts (its flag +0xAD): finds each part's material (`0xBD6E10`), sorts them by kind (`0xCB63C0`) and fills the list given |
| `0x8eb860` | Water::Build8EB860 | In outline: builds a water piece's shapes in shared six-entry arrays (`0x1317DD0`, `0x1317D50`): makes them (`0x8E6E10`, `0x8EA650`) and joins their edges as pairs (PairIds::Update) |
| `0xc73fe0` | Device::PickFormatC73FE0 | In outline: like Device::PickFormatC8DA50, picks among formats by the device's version numbers (+0x2080, +0x2084) |
| `0xc2bac0` | Skeleton::BuildC2BAC0 | In outline: sets up a model's bones: goes over up to 0x105 bone slots (`0xC2CA20`), lines up the ones out of range (Matrix::LookAlongZSide) and makes their tables (0x50 and 8 bytes each) |
| `0xc9fbb0` | Batches::BuildC9FBB0 | In outline: another batch builder like Batches::BuildC9EF70, with three flags for what to build and a buffer for the current device (ForCurrent(3), `0xC31770`) |
| `0x8f28b0` | Terrain::FillBuffers8F28B0 | In outline: fills a terrain patch's vertex and index buffers from its layers and cells |
| `0xca09f0` | Batches::SortCA09F0 | In outline: groups a model's batches by material (+0x4C) and builds their draw lists, like the batch builders at `0xC9EF70` |
| `0x8c0cf0` | Terrain::DrawPass8C0CF0 | In outline: draws a terrain pass for the visible rows (`0x8BEEF0` on `0x1400160`), counting each batch (Counter::Increment, RenderStats::AddBatch) |
| `0xc1d7e0` | Particles::BuildC1D7E0 | In outline: builds a particle's quad (Triangle::FromBase, turned by its angle) and works out its size from its values (+0x90) and a time factor (`0xDDD040`) |
| `0x447740` | Terrain::DrawGrid447740 | In outline: draws the terrain grid overlay: two scaled views (Matrix::ScaleOffset2D) and a pass of batches (`0x447610`) |
| `0x927d60` | Water::DebugDraw927D60 | In outline: draws water debugging shapes in translucent white, green and yellow (Colour::FromARGB 0x3FFFFFFF, 0x3F00FF00, 0x3FFFFF00) for the points of a water piece (+0x32C) |
| `0x908c00` | Water::Colours908C00 | In outline: works out the colours of a water piece's points from their directions and a time factor (`0xDDD040`), as alphas |
| `0x9757d0` | Particles::Draw9757D0 | In outline: draws a particle list for the current device (ForCurrent(3)) inside a timed critical section, building each one's quad from its direction and colour |
| `0x9525d0` | Water::DrawPieces9525D0 | In outline: draws the water pieces with the id given for the current device (ForCurrent(3)): for each, works out its direction and its colours from a table by angle |
| `0xa00bf0` | Shadows::FitA00BF0 | In outline: fits a shadow view to a list of points (+0x6C): finds their range, the terrain, and the light direction (`0x85CB30`, normalised from `0xF86DE0`) |
| `0x933a20` | WaterFlow::Draw933A20 | In outline: draws a water flow for the current device (ForCurrent(1)) with the flow manager (WaterFlowManager::Get) and its colours (`0x10F0C2C`, `0x10F0C30`) |
| `0x80ce90` | Frustum::Cull80CE90 | In outline: culls a list of boxes against a view's planes (normalised, Box::NearFarOffsets2 for each plane's near and far corners) and keeps the ones inside (`0x80C8B0`) |
| `0xbd5360` | Draw::QuadBD5360 | In outline: a long run of maths building a drawn shape from a position, sizes and colours, ending with the device setup (`0xC1FC30`) |
| `0x83a8c0` | SceneryRenderer::DrawItems83A8C0 | In outline: draws a list of scenery items (+0xE4, count +0xE8): turns each one (Matrix::TurnColumnsAboutY), looks up its texture (`0x463530`) and draws it with the shared shader (`0x10F2FA0`) |
| `0x962390` | Mesh::Build962390 | In outline: builds a mesh from a grid of points: makes its vertices (16 bytes each) and triangles (12 bytes each) and works out the normals (Mesh::ComputeNormals) |
| `0x863140` | Scenery::CollectVisible863140 | In outline: goes over a scenery set's parts by kind (0 or 3) and keeps the ones within the view distances (+0x168, +0x178) |
| `0x9c8390` | Particles::Update9C8390 | In outline: updates a particle system's emitters (+0x54, count +0x5C): turns them by their spins (Matrix::TurnAboutX, Matrix::TurnAboutY), points them (Matrix::LookAlongZSide, LookAlongYSide) and spawns particles (Particles::Spawn9CBDF0) |
| `0x96d180` | Water::DrawJets96D180 | In outline: draws water jets with work to do (+0x24, +0x44) for the current device (ForCurrent(3)), building their shapes (`0x9F8B40`) |
| `0x9052a0` | Water::Visible9052A0 | In outline: finds the water parts the camera sees (MainCamera::Get) over the terrain, checking each one (`0x903AA0`) |
| `0x857340` | SceneryRenderer::DrawList857340 | In outline: draws a list of scenery parts for the current device (ForCurrent(3)): sets each one's matrix (`0x42D200`, `0x42DEA0`) and skips the faded ones (flag 0x4000 with a small value) |
| `0xccce30` | Shapes::DrawCCCE30 | In outline: draws a set of shapes (+0x44) with their shaders (ShapeShaders::Load, `0xCC1F30`), counting the batches (Counter::Increment) and releasing them after |
| `0x94ce10` | Water::MakeSphereTexture | In outline: makes a 64 by 64 texture (Device::CreateTexture) from a hemisphere: each pixel's normal from its distance from the centre (the square root of 1 minus it squared) |
| `0x9d0200` | Particles::FillBuffers9D0200 | In outline: fills the particle vertex and index buffers for the current device (ForCurrent(3), `0xBFFB60`) from the systems with particles to draw (+0x38 above 1) |
| `0xcd1660` | Shapes::BuildCD1660 | In outline: builds two shapes' points (`0xCD0C60`) and joins them by a detail level (`0x13FE0F4`, 0 to 3) over their lists |
| `0xbc0f10` | Model::BuildLodBC0F10 | In outline: builds the draw data for a model's detail level (Lod::LevelFor): finds its material (`0xBD6E10`), allocates its buffers (`0xBC0A00`, 64 bytes per part) and fills them |
| `0xc06910` | Texture::LoadC06910 | In outline: loads a texture described by a record: its scale (1 over its first value), its name (from +0x28, `0xCA1B10`) and its kind (+0x3C), or another load (`0xBD51C0`) |
| `0x8ec9d0` | Water::BuildAll8EC9D0 | In outline: builds the water pieces (Water::Build8EB860 for each) in groups of up to 100 and 0x1000 points, then fills them for the current device (ForCurrent(3)) |
| `0x96c930` | Water::DrawJets96C930 | In outline: another water jet drawing pass like Water::DrawJets96D180, for the jets with work at +0x14 and +0x34 |
| `0x92c990` | Water::DebugDraw92C990 | In outline: in debug mode 1 (+0x310), draws the water pieces with the developers' shader (Debug::DrawSolidColourNoZ2) and their links (`0x92BFD0`, `0x919A50`) |
| `0xc03170` | Device::BuildModesC03170 | In outline: builds the list of a device's sizes or modes (`0x13D301C` made when needed), halving down from the largest, and rounds each one's values |
| `0x426f40` | Sky::UpdateLight | In outline: works out the sky and sun lighting for the time of day from curves (`0x426950`), the weather (Weather::Get) and the sun's angle (`0x426560`, `0x4266A0`), with a special case for mode 2 (`0x10ED780`) |
| `0x9ecc50` | Firework::Update9ECC50 | In outline: a firework's update: moves it on (`0x9EC8B0`), and when its time is up (+0x40) and it has a part to fire (`0x4751E0`), fires it and works out its direction |
| `0x9592d0` | Water::DrawReflections9592D0 | In outline: for more than ten items, draws the water reflections over the terrain with the water manager (WaterManager::Get, `0x940FB0`, `0x959020`) |
| `0xbc1860` | Model::BuildPartsBC1860 | In outline: builds the parts of a model up to a count (+4), when it is ready (+0xDD): goes over its lists and adds the parts that fit |
| `0x926780` | Water::DebugDraw926780 | In outline: when the developers' water view is on (`0x1103B9C`, +0x315), draws the water's lines and points in green, red, yellow and magenta (Colour::ToARGB) by the view mode (+0x310) |
| `0x58dfd0` | Placement::DebugDraw58DFD0 | In outline: when the developers' view is on (`0x12F54C0`), draws a placement's points and links in magenta (Colour::ToARGB, Registry::Ensure, `0x481620`) for a few kinds |
| `0x8595a0` | SceneryRenderer::DrawFaded8595A0 | In outline: draws scenery parts (+0xE0) for the current device (ForCurrent(3)) with a fade amount below 1 scaled into a byte, skipping parts with flag 0x10 |
| `0x8ca830` | Terrain::DrawPass8CA830 | In outline: another terrain pass: visible rows (`0x8BEEF0` on `0x1400160`), a scaled view (Matrix::ScaleOffset2D) and batches (`0x8C49D0`, `0x8534A0`) |
| `0x999310` | FireworkLauncher::Fire999310 | In outline: fires from a firework launcher: finds its free slots (FireworkLauncher::FindSlots_(2)), aims the shot (`0x445270`, `0x445D40`) along its direction and launches it (`0xC21230`) |
| `0x858e50` | SceneryRenderer::DrawList858E50 | In outline: another scenery draw list like SceneryRenderer::DrawList857340, over the parts at +0xC0 |
| `0xbd82a0` | Text::LayoutBD82A0 | In outline: lays out a text with its markup: when on (Flags::Bit0), goes over its characters and splits out the parts in braces (`{`, WideString::Substring) to handle them (`0xBD77C0`) |
| `0x89e620` | SceneryRenderer::Pass89E620 | In outline: a scenery drawing pass over a set's lists (kind 2 entries, flag bit 3): sets up (`0x87B480`, `0x853AE0`) and draws each group (`0x855A20`, `0x8536C0`, `0x89C9D0`) |
| `0x857be0` | SceneryRenderer::DrawRange857BE0 | In outline: draws a range of scenery parts (under 0x3FFF) for the current device (ForCurrent(3)) |
| `0xc947f0` | Renderer::CopyStateC947F0 | In outline: copies a renderer's state from another, value by value |
| `0x98ed30` | Effect::Update98ED30 | In outline: an effect's update when not flagged (+0x54, +0x211): finds its target (`0x98AEC0`) and starts it (`0x98E9A0`) |
| `0xc199b0` | Texture::RebuildC199B0 | In outline: when a texture's size changed (+0x170 vs +0x17C), works out its levels and makes it again (Device::CreateTexture) |
| `0xbd89e0` | Text::LayoutBD89E0 | In outline: another text layout like Text::LayoutBD82A0 (when Flags::Bit0 is set), measuring its parts (`0xBD80A0`) |
| `0x986060` | Effect::Place986060 | In outline: places an effect over the terrain as seen from the camera (MainCamera::Get), rounding its cells |
| `0x8ea650` | Water::Shape8EA650 | In outline: works out a water shape's surface over the terrain: normalises its direction (+0x20 to +0x28) and compares its corner heights |
| `0xc8ba70` | Device::ApplyC8BA70 | In outline: applies a device mode when it can (`0xBE90C0`): resets (`0xBE9040`), sets it (`0xBDCF60`, +0x4EC) and refreshes the resources when it changed (+0x1A44) |
| `0xce36a0` | Mesh::WeldCE36A0 | In outline: joins a mesh's points that lie within a small distance (sorted by one axis), making new index and point tables |
| `0x965d90` | Water::MergeBodies965D90 | In outline: goes over the water bodies (`0x9653C0`, `0x9651D0`, `0x9654F0`) joining the smaller ones into the larger (`0x965A50`) |
| `0x9e66b0` | Firework::Draw9E66B0 | In outline: draws a firework part over its time (+0x1B8, rounded to frames) with a batch (`0x4377E0`) |
| `0x7c0b80` | Effects::Draw7C0B80 | In outline: draws a list of effects (+0x20) inside a timed critical section, counting the batches (Counter::Increment) and building each one's shape (`0x9F8B40`) |
| `0xc957e0` | Effect::SetParamC957E0 | In outline: sets one of an effect's settings by number from the value given (the same shape as Effect::SetParamC75800) |
| `0xc76870` | Effect::SetParamC76870 | In outline: the same setting-by-number for another kind of effect |
| `0xc71070` | Effect::SetParamC71070 | In outline: the same setting-by-number for another kind of effect |
| `0x971060` | Water::FillBuffers971060 | In outline: fills water vertex and index buffers for the current device (ForCurrent(3), `0xBFFB60`) for a range of pieces, from their directions |
| `0xce0670` | Mesh::BuildCE0670 | In outline: builds a mesh's tables from its parts (+0x18): 16-byte and 32-byte entries filled in groups of four |
| `0xcdf840` | Mesh::BuildCDF840 | In outline: the same mesh table build as Mesh::BuildCE0670 (another copy) |
| `0x9686a0` | Water::DrawRipples9686A0 | In outline: draws water ripples for the current device (ForCurrent(3)) with an alpha from the value given, for each one (+0x24) |
| `0xbd78a0` | Text::LayoutBD78A0 | In outline: another text layout like Text::LayoutBD82A0 (when Flags::Bit0 is set), breaking lines at the newline character |
| `0xce0d10` | Mesh::BuildCE0D10 | In outline: the same mesh table build as Mesh::BuildCE0670 (another copy) |
| `0x5725d0` | Ride::DebugDraw5725D0 | In outline: in the developers' view mode 3 (`0x12F54C0`), draws a ride's points (+0x192 of them) and links (Registry::Ensure, `0x481620`) |
| `0x93df70` | Water::Clip93DF70 | In outline: clips a water piece to the view: transforms its points (`0xCD78A0`) and keeps their range |
| `0xcdfed0` | Mesh::BuildCDFED0 | In outline: the same mesh table build as Mesh::BuildCE0670 (another copy) |
| `0x864da0` | Scenery::DrawVisible864DA0 | In outline: collects the scenery in view (Scenery::CollectVisible863140), sorts it by camera distance (MainCamera::Get) and draws it (`0xCD7DC0`) |
| `0x9f91b0` | Firework::Ground9F91B0 | In outline: works out where a firework part meets the terrain, cell by cell (rounded) |
| `0x9742b0` | Particles::Draw9742B0 | In outline: when particles are on (`0x12F018C` +0x24 +1), draws a particle list for the current device (ForCurrent(3)), each quad from its direction and its colour times a fade (+0x40) |
| `0xaf6d60` | Effect::BlendAF6D60 | In outline: when not flagged (+0x60), makes a blender for an effect (Blender::Blender, FloatBlenderElement) and starts it (`0x98A770`) |
| `0x85e7b0` | Scenery::HitTest85E7B0 | In outline: whether a ray hits a scenery item's pieces (`0xCDEB10`), checking each one (`0x827530`) and keeping the nearest within a distance |
| `0xcb63c0` | Batches::SortKeyCB63C0 | In outline: a draw batch's sort key by the renderer's mode (`0x13D0DB8`): from its material (`0xCB5BE0`) or its depth, rounded |
| `0x92eef0` | Water::Jets92EEF0 | In outline: makes a water jet's geometry (`0x92E7F0` with its counts): points it (Matrix::LookAlongZSide) and checks its owner (`0x92E8B0`) |
| `0x486da0` | Model::FindParts486DA0 | In outline: for a kind 3 model, finds its parts by name (`0xCC8EE0`) and sets each one up (`0x485690`, 0x60 bytes each) |
| `0x97bce0` | Shapes::Draw97BCE0 | In outline: draws a set of shapes (+0x4C, count +0x50) with a batch (`0x499160`), counting it (Counter::Increment) |
| `0xcdc480` | Mesh::JoinCDC480 | In outline: joins two meshes' parts by their counts (+0x7C tables) into one |
| `0x92d560` | Waterfall::Draw92D560 | In outline: draws a waterfall (`0x92C710`, `0x92BF40`) unless off (WaterfallManager::Get +0x314), with its times (+0x1F0, +0x1F4) rounded to frames |
| `0xc74ac0` | Device::PickFormatC74AC0 | In outline: another format picker like Device::PickFormatC8DA50, with flag 2 and the device's version numbers |
| `0x407af0` | Draw::Lines407AF0 | In outline: draws lines between points for the current device (ForCurrent(3), Call10IfReady), each quad from the line's direction, counting the batch (RenderStats::AddBatch) |
| `0x926fa0` | Water::LinkPoints926FA0 | In outline: links two water points when their directions agree, and in debug draws the link in green (Colour::FromARGB 0x3F00FF00) |
| `0x971cd0` | Water::Draw971CD0 | In outline: draws a water piece by its kind (0, ...), filling its buffers (Water::FillBuffers971060) when flagged (+0xB5) |
| `0xcdbcb0` | Shapes::HitCDBCB0 | In outline: checks a shape's parts (up to 0x80) against a box (`0xBCC740`) |
| `0xc1cea0` | Particles::QuadC1CEA0 | In outline: builds a particle's quad from two triangles (Triangle::FromBase) by its kind |
| `0x9c4c10` | Particles::Emit9C4C10 | In outline: when on (+0x90) and moving, makes particles along a movement by distance, scaled by its values (+0x64, +0x78, +0x7C) |
| `0xce6880` | Shapes::FillCE6880 | In outline: fills a shape's vertices (+0x18) with a colour from the alpha given |
| `0x890b50` | Scenery::Shadow890B50 | In outline: works out a scenery part's shadow corners in a shared eight-point table (`0x1311190`), six sides at a time |
| `0x923350` | Waterfall::Build923350 | In outline: builds a waterfall's shape when flagged (+0x300): checks it (Water::DebugDraw927D60) and fills its two sides (`0x9197A0`, `0x919C80`) |
| `0x4988c0` | Scenery::Visible4988C0 | In outline: goes over a set's parts (+0x14, +0x24) marking the shown ones and, when flagged (+0x7C), refreshes (`0x498630`) |
| `0x8905d0` | Scenery::Shadow8905D0 | In outline: another shadow corner pass like Scenery::Shadow890B50 (table `0x1311100`) |
| `0x85eee0` | Scenery::Visible85EEE0 | In outline: whether a scenery part should be drawn by its kind (low 16 bits of +8: 0, ...) and its sub-kinds (4, 5, 7) |
| `0xc92db0` | Device::CheckModesC92DB0 | In outline: when asked or flagged (+0x4E8), checks the device's modes (`0xBDCBD0`, up to 8 and 0xA0 entries) |
| `0x903bf0` | Water::Setup903BF0 | In outline: sets up the water parts that need it (+0x2C): builds their table (Table::Build900DA0) and handles kind 8 pieces |
| `0x861340` | Scenery::Lod861340 | In outline: picks a scenery item's detail by its camera distance (MainCamera::Get) and, for kind 0x12, sets it up (`0x860790`) |
| `0x86d880` | Scenery::Bounds86D880 | In outline: when not flagged (+0x40), works out a scenery item's box from its shapes (`0x864620`, Model::CheckShapes) |
| `0xbc34f0` | Resource::CopyNameBC34F0 | In outline: copies a resource's name and data (`0xBCE1D0`, `0xBE5020`) |
| `0xce4cc0` | Sound::UpdateCE4CC0 | In outline: a sound's update when playing (+0x20): checks its buffer (`0xCE63E0`) and its state flags |
| `0x88d700` | SceneryRenderer::Pass88D700 | In outline: another scenery drawing pass like SceneryRenderer::Pass89E620 (`0x856400`, `0x855A20`) |
| `0x812650` | Scenery::DrawSorted812650 | In outline: draws a list (+0x64) sorted by depth (List::SortBy34) inside a timed critical section |
| `0xced5f0` | Mesh::BuildCED5F0 | In outline: builds a mesh's parts (+0x70 of them, each with +0x28 pieces, `0xCE9470`) |
| `0xccc720` | Ads::DrawCCC720 | In outline: draws a list of shapes (+0x44) with their textures, using the advert texture (Ad::Texture with `0x13D0E34`) for flagged ones |
| `0x44b360` | Effects::Update44B360 | In outline: goes over a list of effects (+0x10) and, unless off (+0x21 and the owner's +0x1A6), updates each one by its kind (+0x30: 1, ...) |
| `0xce3fe0` | Mesh::SplitCE3FE0 | In outline: splits a mesh's parts into a new list (12 bytes, `0xCE35A0` for each), or none when it is empty |
| `0xaf3000` | FireworkDisplay::MakeAF3000 | In outline: when not flagged (+0x74), makes a firework display (FireworkDisplay::FireworkDisplay) and starts it (`0x98BC00`) |
| `0xc250e0` | Skeleton::AimC250E0 | In outline: turns a bone toward a direction, when it is long enough, and builds its matrix |
| `0x90fbb0` | Water::Draw90FBB0 | In outline: draws water pieces over the terrain when there are some (+4 vs +0x10), through each part's draw slot (+0x2C) |
| `0x414770` | Grid::Normals414770 | In outline: works out the normals of a grid's rows (+0x1C), normalising each one |
| `0x414d20` | Grid::Visible414D20 | In outline: finds the grid cells the camera sees (MainCamera::Get) from its direction (+0x78) |
| `0xa5fe80` | Sound::UpdateA5FE80 | In outline: another sound update like Sound::UpdateA61260, for kind 1: its volume (`0xA5ED70`) and fade (+0x40, `0xA5FC30`) |
| `0x979220` | Particles::Draw979220 | In outline: when particles are on (`0x12F018C` +0x24 +2), draws a list inside a timed critical section |
| `0xc5b220` | Animation::StepC5B220 | In outline: moves an animation on by a time: works out the frame by its length and wraps or holds at the end |
| `0xc5b710` | Animation::StepC5B710 | In outline: the same animation step for another kind of track |
| `0xbd5c70` | Draw::QuadBD5C70 | In outline: draws a shape (Draw::QuadBD5360) and finishes it (`0xC1E790`) |
| `0xcf1290` | Shapes::DrawCF1290 | In outline: another shape set draw like Shapes::DrawCCCE30 (ShapeShaders::Load, `0xCC1F30`, Counter::Increment) |
| `0x8583e0` | SceneryRenderer::DrawFaded8583E0 | In outline: another faded scenery draw like SceneryRenderer::DrawFaded8595A0, for sets under 0x10001 parts |
| `0x9640f0` | Fireworks::Update9640F0 | In outline: goes over a firework list's parts (+0x10) that are on (+8), aiming them (`0x445270`) and checking their distances |
| `0xc90990` | Device::CheckStateC90990 | In outline: checks the device's state against what was last set (+0x4EC vs +0x21A8, +0x420, flags at +0x1CB8 and +0x1CAC) |
| `0x8cafd0` | Terrain::DrawPass8CAFD0 | In outline: another terrain pass like Terrain::DrawPass8C0CF0 for mode 3 |
| `0xc25b50` | Skeleton::BuildC25B50 | In outline: builds the matrices of a model's bones (+0x20), four at a time |
| `0xc912c0` | Device::ResetC912C0 | In outline: when asked or flagged (+0x4E8, +0x1CB4), goes over the device's resources and resets them |
| `0x439280` | Lighting::Update439280 | In outline: a timed lighting update inside a critical section: makes the shared light (`0x1106D8C`) and sets it from the sun's downward direction (Lighting::DownwardDirection) |
| `0x99a270` | Fireworks::Draw99A270 | In outline: draws a firework display's shots when it has some (+0x1BC, +0x1CC, +0x1F8) |
| `0xcde660` | Shapes::HitCDE660 | In outline: checks a shape's parts (up to 0x80) against a box for an object with flag 0x200000 |
| `0xccbda0` | Particles::DrawCCBDA0 | In outline: draws particles for the current device (ForCurrent(3)), each from its direction |
| `0xcde1a0` | Mesh::JoinCDE1A0 | In outline: another mesh join like Mesh::JoinCDC480 |
| `0x484f60` | Model::Draw484F60 | In outline: draws a model turned by its angle (Matrix::TurnColumnsAboutY) with the shared shader (`0x10F2FA0`, Table10::Lookup) and its texture |
| `0x9278d0` | Water::Distance9278D0 | In outline: the nearest distance from a point to a water piece's planes (shown in translucent white in debug) |
| `0xc75310` | Effect::SetByKindC75310 | In outline: sets an effect's value n by kind (0, ...) (`0xC5C7F0`) |
| `0x89f460` | Billboard::Make89F460 | In outline: makes a billboard for a scenery part (Billboard::Create) at its rounded size, when within a distance (+0x30) |
| `0xa7ed40` | Animal::DrawA7ED40 | In outline: sets an animal's look: its box (`0xA7EBE0`, `0xBCC740`), its direction (Matrix::LookAlongYSide) and its parts (`0xA7BF80`) |
| `0xb48330` | WildAnimal::PoseB48330 | In outline: works out an animal's pose when it moved more than its kind's step (`0x10F27DC` by +0xAC, 0x1C bytes each) and builds its parts' matrices (`0x869400`) |
| `0x97a700` | Particles::Collect97A700 | In outline: when particles are on (`0x12F018C` +0x24 +2), collects the ones to draw (`0x97A4F0`) |
| `0x869400` | Model::PartMatrices869400 | In outline: builds the matrices of a model's shown parts (+0x104, count +0x108, `0x8625D0`) and its slot (+0x100) |
| `0x938720` | Water::Strip938720 | In outline: builds an index strip for a water piece's points (+0x3C) above 0 |
| `0xbf3500` | Text::DrawBF3500 | In outline: draws a text with an alpha (rounded, kept between 0 and 255) |
| `0xbf8100` | Draw::LinesBF8100 | In outline: draws lines for the current device (ForCurrent(3), Call10IfReady) |
| `0xa002d0` | Lighting::ApplyA002D0 | In outline: applies the lights of a list (+0x14): their colours (`0x438C10`, `0x454B20`) scaled into bytes |
| `0xbf2e70` | Text::DrawBF2E70 | In outline: another text draw like Text::DrawBF3500, with an alpha kept between 0 and 255 |
| `0x8beef0` | Terrain::VisibleRows | In outline: works out the range of terrain rows and columns in view (used by the terrain passes, on `0x1400160`), rounded |
| `0x9f79f0` | Fireworks::Fire9F79F0 | In outline: fires the parts of a list (+0x18) that are on (`0x9F7370`) |
| `0x975190` | Particles::Draw975190 | In outline: another timed particle draw like Particles::Draw9757D0, for kind 3 entries within limits (under 10001 and 15001) |
| `0xc6a040` | ScrollBar::DrawC6A040 | In outline: draws a scroll bar with its colours (Colour::ToRGB, ScrollBar::Draw) |
| `0x9c75e0` | Particles::Update9C75E0 | In outline: another emitter update like Particles::Update9C8390 over the emitters (+0x54, count +0x5C) |
| `0xccf100` | Vertices::ColourCCF100 | In outline: writes colours into a list of vertices, four at a time |
| `0x938e90` | Water::DrawStrip938E90 | In outline: builds a water piece's strip (Water::Strip938720) and draws it for the current device (ForCurrent(0)) |
| `0x474580` | Draw::Box474580 | In outline: draws a box with its colour (values scaled into bytes) with the shared line drawer (Registry::Ensure, `0x481790`) |
| `0xbd44d0` | Text::SetSizeBD44D0 | In outline: when a text's size changes (+0x50, +0x54), rebuilds its buffer for the current device (ForCurrent(3)) |
| `0x998ef0` | FireworkLauncher::DebugDraw | In outline: draws a launcher's shot path (FireworkLauncher::FindSlots_(2), `0x409370`) with the shared line drawer (Registry::Ensure) for the selected one |
| `0xbf1c40` | Text::LayoutBF1C40 | In outline: lays out a text's characters until the end |
| `0x431fd0` | Effects::Update431FD0 | In outline: goes over a list of effects (+0x10) updating the shown ones, measuring their distances, unless the global flag `0x1400306` is set |
| `0xc6fdb0` | Effect::SetByKindC6FDB0 | In outline: another set-by-kind like Effect::SetByKindC75310 |
| `0xbf1600` | Text::LayoutBF1600 | In outline: another text layout like Text::LayoutBF1C40 |
| `0xca1530` | Font::RenderGlyphs | In outline: draws a font's characters into a bitmap with Windows (TextOutA at the middle of each cell) and keeps the cells that have pixels |
| `0xc31c70` | Mesh::FillC31C70 | In outline: fills a mesh's buffers (`0xC31770`) for a count, copying four values at a time |
| `0x998630` | Fireworks::Aim998630 | In outline: aims a firework at the point under the mouse (MainCamera::Get, `0xCD79C0`), normalising its direction (+0x1E8) |
| `0x4833d0` | Lights::Find4833D0 | In outline: finds the free light slots of a list and clears a 0x27-entry table |
| `0x97b040` | Shapes::SetCount97B040 | In outline: when a shape set's count changes (+0x48), releases or makes its parts (+0x4C, count +0x50) |
| `0x461650` | Texture::Make461650 | In outline: when it is on (+0x35) and not too big (up to 0x800, +0x28), makes a texture (Device::CreateTexture, twice the size, format 5) |
| `0x9e89a0` | Sky::BlendColours | In outline: moves a set of shared colour values (`0x1341A80` on, 0x30 bytes each) toward their targets by a rate (Value::MoveToward with 0.05) |
| `0x855560` | Water::DrawReflection855560 | In outline: when reflections are on (`0x12F018C` +0xB0), draws the water's reflection (WaterManager::Get, `0x949210`, Spring::Constants) |
| `0x8fd290` | Water::DrawRange8FD290 | In outline: draws a range of water pieces (under 0x3FFF) for the current device (ForCurrent(3)) |
| `0x978970` | Particles::Draw978970 | In outline: draws particles for the current device (ForCurrent(3)), each by its fade (+0x8C, between 0 and 1) |
| `0x811280` | Shadows::View811280 | In outline: sets up a shadow view with an orthographic matrix (Matrix::Ortho, 1 to 1000) by its kind (0, 1, 2), with `0x466660` |
| `0x425d40` | Sky::Colour425D40 | In outline: works out a sky colour (`0x9FBED0`) kept at most 1 and packed into a colour value |
| `0xbf3ba0` | Texture::ConvertBF3BA0 | In outline: converts a texture's pixels for the sizes 0x40, 0xC0 and 0x100, giving 0 for others |
| `0xbf2880` | Text::DrawBF2880 | In outline: another text draw with an alpha kept between 0 and 255 |
| `0x9acb70` | Fireworks::SourceAt | In outline: points a firework's particle source along its direction (Matrix::LookAlongZSide, ParticleSource::SetPosition) when not flagged (+0xFC) |
| `0x946fc0` | Water::Find946FC0 | In outline: finds the water pieces with the id given (+0x3C) and gathers them (`0x926200`, `0x90F8F0`) |
| `0xbf2290` | Text::DrawBF2290 | In outline: another text draw with an alpha kept between 0 and 255 |
| `0x4992d0` | Shapes::Draw4992D0 | In outline: draws a shape (`0x499160`) for the current device (ForCurrent(3)) along its direction |
| `0x9fbf40` | Fireworks::Face9FBF40 | In outline: turns a firework part to face the camera (MainCamera::Get), normalising the ways |
| `0xc01ba0` | Device::HandleC01BA0 | In outline: handles a device message by its kind (low byte of +0x24: 0x12, ...) when on (`0x10F2EC4`) and with more than three entries (+0x128) |
| `0x90cae0` | Water::DrawFlagged90CAE0 | In outline: draws the flagged water parts (+0x4C, count +0x54) for the current device (ForCurrent(3)) |
| `0x8b0f00` | SceneryBehaviour::Start8B0F00 | In outline: starts a scenery behaviour (SceneryBehaviour::SetActive, SlotPool::Add) with an empty name |
| `0xbf78f0` | Draw::SetupBF78F0 | In outline: sets up drawing for the current device (ForCurrent(3)) with two states (Call10IfReady) |
| `0x9ca400` | Particles::Clear9CA400 | In outline: clears a particle system's emitters (+0x58) (`0x5501E0`, `0x9CD440`) |
| `0x91b6b0` | Waterfall::Draw91B6B0 | In outline: draws a waterfall's grid (at least 2 by 2, +0x154, +0x158) for the current device (ForCurrent(1)) with the waterfall manager |
| `0xbf0520` | Text::LayoutBF0520 | In outline: another text layout like Text::LayoutBF1C40 |
| `0xbf1060` | Text::LayoutBF1060 | In outline: another text layout like Text::LayoutBF1C40 |
| `0xbf0ac0` | Text::LayoutBF0AC0 | In outline: another text layout like Text::LayoutBF1C40 |
| `0x916620` | Water::Update916620 | In outline: when the water options are off (`0x10EDBA0`, `0x10EDD08`), the plain update (`0x916580`), otherwise each piece (+0x14, `0x916310`) |
| `0x9c7fe0` | Particles::Setup9C7FE0 | In outline: below a rate (+0x4C), sets up a particle system's emitters (+0xD0) |
| `0x44d260` | Effects::Aim44D260 | In outline: when on (+0x154), aims an effect's parts (+0x15C) (`0x445270`) |
| `0x978e80` | Shapes::Shader978E80 | In outline: when a shape's settings change, releases its old shader and makes the new one (`0xCC1F30`) |
| `0x9fb600` | Fireworks::Update9FB600 | In outline: goes over a firework display's shots (+0x74, 0x48 bytes each) by their times |
| `0xbeffa0` | Text::LayoutBEFFA0 | In outline: another text layout like Text::LayoutBF0520 |
| `0x4824b0` | Debug::Draw4824B0 | In outline: draws developer shapes with the shared shader (Debug::DrawSolidColourNoZ2) |
| `0xaca430` | Colour::FromHSV | In outline: turns a hue (wrapped into 0 to 360), saturation and value into a colour, by the hue's sixth of the circle |
| `0x96f160` | Water::DebugDraw96F160 | In outline: when the developers' view is on (`0x12F018C` +0x20), draws water lines (Registry::Ensure, `0x481620`) |
| `0x8e84d0` | Water::Reflections8E84D0 | In outline: turns water reflections on or off by the settings (`0x10ED780`, +0x1C0), with `0x8E6150` |
| `0x8f8f10` | Terrain::LoadTextures8F8F10 | In outline: opens and reads the terrain's resources by name (`0xBDADC0`, `0xBDAC20`) |
| `0x9c44b0` | Particles::Near9C44B0 | In outline: whether an emitter is near the camera (`0x130654C`) within its distance (+0x130) |
| `0xbf8f30` | Draw::SetupBF8F30 | In outline: another drawing setup for the current device (ForCurrent(3), Call10IfReady) |
| `0x8f6920` | Terrain::Colours8F6920 | In outline: packs a list of terrain colours into bytes, four at a time |
| `0x856d00` | SceneryRenderer::Draw856D00 | In outline: draws n scenery parts for the current device (ForCurrent(3)) unless turned off (`0x130BC95`) |
| `0xcdca90` | Mesh::JoinCDCA90 | In outline: another mesh join like Mesh::JoinCDC480 |
| `0x9749c0` | Particles::Draw9749C0 | In outline: another particle draw (when on, `0x12F018C` +0x24 +2) for the current device |
| `0xcc8490` | Model::PartsCC8490 | In outline: another model parts pass like Model::PartsCC8810 |
| `0xc1d460` | Particles::QuadC1D460 | In outline: another particle quad like Particles::QuadC1CEA0 (Triangle::FromBase) |
| `0x96a940` | Shapes::Draw96A940 | In outline: draws a set of shapes (+0x3C) with a batch (`0x499160`), counting it (Counter::Increment) |
| `0x7e45c0` | Model::Update7E45C0 | In outline: updates a model's lights (Model::UpdateLights) and parts (`0x861050`), with more when flagged (+0xB8) |
| `0x473f30` | Labels::Update473F30 | In outline: goes over a list of labels (+0xC) by the camera (MainCamera::Get) and updates the shown ones (`0x471180`) |
| `0x91ba70` | Waterfall::Make91BA70 | In outline: for flag 0x80, makes the waterfall manager and activates it; with debug mode 1 (`0x133A174` +0x310), more |
| `0x979880` | Particles::Draw979880 | In outline: another particle draw (when on) for the current device |
| `0x97a000` | Particles::Draw97A000 | In outline: another timed particle draw like Particles::Draw975190, within limits (under 10001, 15001) |
| `0xc70230` | Effect::SetByKindC70230 | In outline: sets an effect's value by number: a colour (Colour::FromARGB) for 0, ... 0xC |
| `0x9ef480` | Fireworks::DebugDraw9EF480 | In outline: when on (+0x124, +0x19C), draws a firework's path in green (0xFF00C000) with the shared line drawer |
| `0x993a50` | FireworkSlot::KeyAt | In outline: makes a firework slot's key frame (FireworkSlotKeyFrame) for a time, finding its key (Keys::FindTime) when not flagged (+0xCC) |
| `0x9d7b00` | FireworkDisplay::Load9D7B00 | In outline: makes a firework display (FireworkDisplay::FireworkDisplay) and fills it (`0x98D860`, `0x991000`, `0x9887A0`) |
| `0x9e4e60` | Fireworks::Fill9E4E60 | In outline: fills a firework part's buffers for the current device (ForCurrent(3), `0xBFFB60`) |
| `0x9f5770` | Fireworks::Update9F5770 | In outline: a firework list's update (`0x9F4480`) over its entries (+0x4C) |
| `0xbefa90` | Text::LayoutBEFA90 | In outline: another text layout like Text::LayoutBF0520 |
| `0x7c0240` | Draw::List7C0240 | In outline: unless told not to, draws the shown entries of a list for the current device (ForCurrent) |
| `0xa0f190` | Effects::DrawA0F190 | In outline: draws a list of effects (+0x20) counting the batch (Counter::Increment) |
| `0x89f8f0` | SceneryRenderer::Pass89F8F0 | In outline: runs a scenery pass (SceneryRenderer::Pass89E620) and finds its entry by kind (SortedTable::FindKind) |
| `0x8ee400` | Water::Draw8EE400 | In outline: when on (`0x1106DC1`), draws a list of water parts (+0x14) |
| `0xce9b30` | Shapes::FadeCE9B30 | In outline: a shape's fade, kept between 0 and 1 and scaled into a byte |
| `0x977a30` | Particles::Draw977A30 | In outline: another particle draw (when on) within limits (under 10001, 15001) |
| `0xbef5a0` | Text::LayoutBEF5A0 | In outline: another text layout like Text::LayoutBF0520 |
| `0xbd4940` | Text::SetSizeBD4940 | In outline: when a text's size changes (+0x50, +0x54), clears it (`0xBD48F0`) and sets it again |
| `0x482d60` | Draw::Lines482D60 | In outline: draws lines for the current device (ForCurrent) |
| `0x453b80` | Texture::Load453B80 | In outline: reads a resource (`0xBDADC0`) and, when small enough (under 0x281), makes its record |
| `0xc76f70` | Effect::CopyKeysC76F70 | In outline: copies an effect's key n (below +0x498) |
| `0xc95ee0` | Renderer::CopySlotC95EE0 | In outline: copies a renderer slot n (below 0x40) |
| `0xcf0400` | Shapes::TexturesCF0400 | In outline: goes over a shape list's parts (+4) setting their textures from the shared table (`0x1106E38`) |
| `0xc5abc0` | Animation::StepC5ABC0 | In outline: another animation step like Animation::StepC5B220 |
| `0x93bd50` | Water::Jet93BD50 | In outline: a water jet's update when not flagged (bit 1 of +8, +0xD4) (MasterBlasterJet::Update) |
| `0x974e60` | Particles::Same974E60 | In outline: whether two particle setups are the same (their first values and flag) |
| `0x420800` | Texture::Make420800 | In outline: makes a 256-wide texture (Device::CreateTexture) for entry n of a list (+0x11C) and fills it (`0x41F910`) |
| `0x85f4f0` | Scenery::HitTest85F4F0 | In outline: another scenery hit test like Scenery::HitTest85E7B0 (`0xCDEB10`) |
