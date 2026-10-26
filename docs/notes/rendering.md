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
| `0x89cc00` | Scenery::Fit89CC00 | In outline: compares a model's box (Model::Bounds) with limits |
| `0xcc8bc0` | Model::PartsCC8BC0 | In outline: another model parts pass like Model::PartsCC8810 |
| `0xcb6c90` | Batches::DrawCB6C90 | In outline: draws a model's batches (Call10IfReady for each, +0x44 of them), with an extra pass at detail level 1 (`0x13D3184`) for flags 0x30 |
| `0xc5af00` | Animation::StepC5AF00 | In outline: another animation step like Animation::StepC5B220 |
| `0x89edd0` | Scenery::Ground89EDD0 | In outline: works out a scenery part's box (`0x864620`) over the terrain |
| `0xace200` | Graph::DrawACE200 | In outline: another graph draw when not flagged (+0x100) |
| `0xc5df60` | Draw::C5DF60 | In outline: a long maths body with no calls (a drawing step) |
| `0x90c7d0` | Water::Draw90C7D0 | In outline: another water draw like Water::DrawFlagged90CAE0 for the current device |
| `0xcc10f0` | Shader::SetCC10F0 | In outline: sets a shader's values (`0xE02AB0`, `0xE02B70`) by its flags (bit 2, the kind table `0xFC4C78`) |
| `0x96a630` | Shapes::Draw96A630 | In outline: draws a shape set with a batch (`0x8534A0`), counting it |
| `0xce9830` | Shapes::FillCE9830 | In outline: fills a shape's parts (+0x18) when no target is given |
| `0x9719c0` | Shapes::Draw9719C0 | In outline: draws a shape with its alpha (+0xC, `0x5BAB50`), counting it |
| `0x85a060` | SceneryRenderer::Draw85A060 | In outline: draws scenery parts (+0x120) for the current device, each from its table entry (Table2C::Value) |
| `0x7bae80` | Fog::Update7BAE80 | In outline: a timed update that sets the fog (Fog::Setup from its distance, +0x14 +0x2C) |
| `0xcc6f00` | Animation::SampleCC6F00 | In outline: samples an animation's tracks over a range at a time (Keys::FindFrom2) |
| `0x9faf80` | Fireworks::Shots9FAF80 | In outline: goes over a display's shots (+0x5C) that have a slot, starting each (`0x8501C0`) |
| `0x859d60` | SceneryRenderer::Draw859D60 | In outline: another scenery draw like SceneryRenderer::Draw85A060, over the parts at +0x100 |
| `0x979d00` | Particles::Same979D00 | In outline: another particle setup comparison like Particles::Same974E60 |
| `0xa0d300` | Effect::FaceA0D300 | In outline: turns an effect by an angle (cosine, sine) and points it (Matrix::LookAlongZ) |
| `0x977370` | Particles::Draw977370 | In outline: draws particles for the current device (ForCurrent(3)) |
| `0x9ae960` | Fireworks::Update9AE960 | In outline: a firework update when on (+0x154) and not flagged (+0xFC) |
| `0x92f510` | Water::Jets92F510 | In outline: goes over a water piece's jets (`0x92E8B0`) not flagged (+0x18C) |
| `0x42b080` | Labels::Update42B080 | In outline: finds a label (`0x474D20`) and updates it |
| `0xc06630` | Effect::UpdateC06630 | In outline: an effect's update when not flagged (+0x4C1, +0x4C3), for its shown parts |
| `0x474a40` | Labels::Pick474A40 | In outline: finds the label under the mouse (MainCamera::Get, `0xCD79C0`) |
| `0xbef170` | Text::LayoutBEF170 | In outline: another text layout like Text::LayoutBF0520 |
| `0xbeed40` | Text::LayoutBEED40 | In outline: another text layout like Text::LayoutBF0520 |
| `0x976360` | Particles::Draw976360 | In outline: another timed particle draw inside a critical section |
| `0xe02b70` | Animation::ApplyE02B70 | In outline: applies an animation's keys from a place to a shader (used by Shader::SetCC10F0) |
| `0xcf0da0` | Shapes::BatchCF0DA0 | In outline: makes a shape batch (`0x8534A0`) unless told not to |
| `0xaa3680` | PoolComplex::FillAA3680 | In outline: fills a pool's buffers for the current device (ForCurrent(3), `0xBFFB60`) |
| `0x853af0` | Scenery::Release853AF0 | In outline: releases a scenery part's shader unless in mode 2 (+0x14) |
| `0x9f02f0` | Firework::Time9F02F0 | In outline: a firework part's update once its time (+0x1C) is reached (`0x9F5FE0`) |
| `0x864760` | Scenery::Shapes864760 | In outline: checks a scenery item's shapes (Model::CheckShapes) unless its kind is 4 |
| `0xc009b0` | Effect::ValueC009B0 | In outline: an effect value from its parts (+0x164, `0xBFFDC0`) |
| `0xc9b600` | Particles::ListC9B600 | In outline: goes over a particle list's entries |
| `0xbf7cb0` | Draw::SetupBF7CB0 | In outline: another drawing setup for the current device (ForCurrent(3), Call10IfReady) |
| `0xbcc1f0` | Object::NonZeroBCC1F0 | In outline: checks an object's values for non-zero ones (0x24 bytes each) |
| `0x9ff7c0` | Fireworks::Make9FF7C0 | In outline: makes a firework part (`0xCEFF50`) |
| `0x977770` | Particles::Shader977770 | In outline: when a particle setup changes, releases its old shader and makes the new one |
| `0x908940` | Water::Colours908940 | In outline: another water colour pass for pieces of two or more points |
| `0xb3c160` | Animal::DrawB3C160 | In outline: sets up an animal's drawing (`0xCD28C0`, `0xCD29F0`) |
| `0xbbb3d0` | Track::CopyBBB3D0 | In outline: copies a track's values in blocks |
| `0xceb670` | Batches::SortByOpacity | In outline: sorts a model's batches (+0x80) by their materials' opacity (Materials::CompareOpacity) |
| `0xbc9c70` | Model::BuildBC9C70 | In outline: builds a model's parts unless turned off (`0x13D0B14`) |
| `0x9fa2e0` | Fireworks::Ground9FA2E0 | In outline: a firework part check over the terrain (kinds other than 1) |
| `0x82f710` | Scenery::Lights82F710 | In outline: updates a scenery model's lights (Model::UpdateLights) and its parts (+0x9C +0x70) |
| `0xc056c0` | Renderer::ClearC056C0 | In outline: clears a renderer's lists (`0xBFA8E0`, `0xC2F250`) |
| `0xbeafc0` | Resources::ListBEAFC0 | In outline: goes over a resource list unless turned off (`0x13D3172`) |
| `0xce8760` | Shapes::FillCE8760 | In outline: when on (+0x34), fills a shape's parts (+0x28) |
| `0x855dd0` | Scenery::FindPart855DD0 | In outline: finds a scenery part (SortedPointers::FindFirst2) |
| `0x85ffc0` | WaterDrops::Aim | In outline: sets a water drop emitter's direction (WaterDrops::SetEmitterDirection) when the value is positive |
| `0x9e5fe0` | Fireworks::Colour9E5FE0 | In outline: a firework part's colour faded by a value, packed |
| `0x989440` | FireworkLauncher::Slots989440 | In outline: finds a launcher's slots (FireworkLauncher::FindSlots) |
| `0xcecee0` | Mesh::AddCECEE0 | In outline: adds an entry to a mesh list (+0x50) |
| `0x88d460` | SceneryRenderer::Pass88D460 | In outline: another scenery pass over a set's lists (+0xC +0x14) |
| `0x81cb60` | SceneryBehaviour::Draw81CB60 | In outline: a long maths body drawing a behaviour from its owner (SceneryBehaviour::Owner) |
| `0x46c7c0` | SceneryBehaviour::Draw46C7C0 | In outline: the same behaviour drawing (another copy) |
| `0xc9d280` | Particles::ClearC9D280 | In outline: when on (+0x68), empties a particle list (+0x34) |
| `0xb75ed0` | Animals::DrawB75ED0 | In outline: a timed draw inside a critical section |
| `0x858bb0` | SceneryRenderer::Draw858BB0 | In outline: draws scenery parts (+0xA0) for the current device |
| `0x9b2c00` | Fireworks::Check9B2C00 | In outline: checks a firework's two values are positive |
| `0xcecc40` | Mesh::ReleaseCECC40 | In outline: releases a mesh's parts' shaders (+0x64) |
| `0x7b8680` | Effect::Bounds7B8680 | In outline: another effect box grow like Effect::Bounds48B6F0 |
| `0x48b450` | Effect::Bounds48B450 | In outline: another effect box grow over set parts |
| `0x927630` | Water::Make927630 | In outline: sets up a water piece (`0x926050`) with the water manager |
| `0xc6f8e0` | Device::ApplyC6F8E0 | In outline: applies a device setting when it can (`0xBE90C0`) |
| `0x8e6980` | Water::Side8E6980 | In outline: which side of a plane a water point lies on |
| `0xcec9b0` | Mesh::PassCEC9B0 | In outline: goes over a mesh's parts (+0x4C, `0xCEC7C0`) |
| `0xcc6420` | Model::PartsCC6420 | In outline: goes over a model's parts when not flagged (+8) |
| `0xc50da0` | Stream::FillC50DA0 | In outline: fills buffers for the current device (ForCurrent(3), `0xBFFB60`) |
| `0xce1720` | Particles::TimeCE1720 | In outline: a particle time check against the shared count (`0x140081C`) |
| `0x474d20` | Labels::Pick474D20 | In outline: finds the label under the mouse (MainCamera::Get, `0xCD79C0`) |
| `0x9ec8b0` | Firework::Move9EC8B0 | In outline: moves a firework on when not flagged (+0x3C) |
| `0x9d6110` | Fireworks::MakeShells | In outline: makes firework shells with fixed settings (`0x950E10` with 0xF and 5 and set sizes) |
| `0xce8b00` | Shapes::GrowCE8B00 | In outline: grows a shape list (+0x10) |
| `0xce8330` | Shapes::FillCE8330 | In outline: when on (+0x34), fills a shape's parts (+0x28) |
| `0x44ac40` | Effect::Matrix44AC40 | In outline: works out an effect's matrix (`0xCD78A0`) when it has parts (+8, +0x58) |
| `0x481910` | Draw::Line481910 | In outline: draws a line with the shared line drawer (`0x481620`) |
| `0xce8d80` | Shapes::GrowCE8D80 | In outline: grows a shape list (+0x10) |
| `0xcf0740` | Shapes::TexturesCF0740 | In outline: another shape textures pass like Shapes::TexturesCF0400 |
| `0x485c10` | Model::Animations485C10 | In outline: for kind 3 models, checks their animations (Model::CheckAnimations) |
| `0x86d5e0` | SceneryModel::Setup | In outline: sets up a scenery model, finding its lights and water when asked (SceneryModel::FindLightsAndWater) |
| `0x9ae1a0` | Fireworks::Link9AE1A0 | In outline: links a firework part when given one |
| `0x7c24b0` | Model::Pose7C24B0 | In outline: sets a model's pose (`0x7BEDB0` on +0xD0, +0x50) |
| `0x481ef0` | Draw::RedLine | In outline: draws a red line (0xFFFF0000) with the shared line drawer |
| `0xac8a30` | LaserDesigner::Bar | In outline: builds the laser designer's bar (LaserDesigner::BuildBar) |
| `0x48a7c0` | Effect::HitTest48A7C0 | In outline: whether a ray hits an effect's parts (Ray::BoxSlabs) |
| `0x582390` | Ride::Lights582390 | In outline: updates a ride's lights (Model::UpdateLights) |
| `0x8b7370` | SceneryBehaviour::Particles | In outline: makes a meta particle (MetaParticle) for a behaviour's owner |
| `0x9cee10` | Particles::Fill9CEE10 | In outline: fills particle buffers for the current device |
| `0x99c800` | Fireworks::Time99C800 | In outline: a firework time value (`0x4756A0`) |
| `0x9c6b90` | Particles::Camera9C6B90 | In outline: when on (+0x114) with emitters (+0x9C), a step by the camera |
| `0xc08dc0` | Text::MeasureC08DC0 | In outline: measures a text to its end |
| `0xbdd9a0` | VertexDeclaration::FillBDD9A0 | In outline: fills a vertex declaration from its elements (kind 1, under 0xFF) |
| `0x426950` | Curves::Sample | In outline: samples a curve at a time (used by Sky::UpdateLight), with one or no keys handled |
| `0x8b0000` | Particles::Random8B0000 | In outline: a random particle direction (Random::Ran0), normalised |
| `0xcdb390` | Vector::DistanceToLine | In outline: a point's distance from a line, from the normalised direction |
| `0x9fc6a0` | Fireworks::Fire9FC6A0 | In outline: fires a firework (Fireworks::Fire9F79F0) and more when asked |
| `0x9e13b0` | Particles::Fill9E13B0 | In outline: fills particle buffers for the current device |
| `0xbf3950` | Text::DrawBF3950 | In outline: draws text with a size rounded from a scale (`0xF264C0`) |
| `0xbf32b0` | Text::DrawBF32B0 | In outline: the same text draw (another copy) |
| `0x9e2a60` | Fireworks::Make9E2A60 | In outline: when on (`0x10F0E88`), makes a firework part |
| `0xbf9760` | Text::DrawBF9760 | In outline: a text draw for the current device (`0x4164C0`, ForCurrent(3)) |
| `0xcf09b0` | Shapes::TexturesCF09B0 | In outline: another shape textures pass, with slot n checked against the shared table (`0x1106E38`) |
| `0x449ea0` | Labels::Draw449EA0 | In outline: draws a label (GUIRenderer, `0x692710` with size 9 by 9) |
| `0xb73450` | Effect::HitTestB73450 | In outline: another effect ray hit test like Effect::HitTest48A7C0 |
| `0x7b7530` | Effect::HitTest7B7530 | In outline: another effect ray hit test like Effect::HitTest48A7C0 |
| `0xb5d920` | Effect::HitTestB5D920 | In outline: another effect ray hit test like Effect::HitTest48A7C0 |
| `0xc90590` | Shader::ConstantsC90590 | In outline: registers shader constant names (c_tci0, c_lightPosDir, ...) |
| `0x9cd440` | ParticleEmitter::Make9CD440 | In outline: makes a particle emitter description (ParticleEmitterDesc) |
| `0xceffa0` | Shapes::IndexCEFFA0 | In outline: makes an index for a shape list (+4) |
| `0x44a320` | Effect::Show44A320 | In outline: shows an effect's parts when flagged (+0x21) |
| `0xaf5360` | WaterDrops::Aim | In outline: sets a water drop emitter's direction (WaterDrops::SetEmitterDirection from +0xE4) |
| `0x9ac930` | Fireworks::Update9AC930 | In outline: a firework update when not flagged (+0xFC) (`0x9D4E50`) |
| `0x858970` | SceneryRenderer::Draw858970 | In outline: draws scenery parts (+0x80) for the current device |
| `0x86d3a0` | SceneryModel::Setup2 | In outline: another scenery model setup, finding its lights and water when asked |
| `0xb74830` | Effect::BoundsB74830 | In outline: another effect box grow like Effect::Bounds48B6F0 |
| `0xb5e710` | Effect::BoundsB5E710 | In outline: another effect box grow like Effect::Bounds48B6F0 |
| `0x7b8bd0` | Effect::Bounds7B8BD0 | In outline: another effect box grow like Effect::Bounds48B6F0 |
| `0x46f6b0` | Labels::Add46F6B0 | In outline: adds a label, up to 5000 |
| `0xc5f570` | Device::SetC5F570 | In outline: a device setting (`0xC5F410`) |
| `0x8699c0` | Scenery::Model8699C0 | In outline: refreshes a scenery model (`0x860790`) |
| `0x454cb0` | Normals::Make454CB0 | In outline: works out direction lengths for a list of points |
| `0x85fd90` | Fireworks::MakeShells2 | In outline: makes firework shells with fixed settings (`0x950E10` with 0xF and set sizes) |
| `0xbf2060` | Text::DrawBF2060 | In outline: draws a text, character by character |
| `0xbf1a10` | Text::DrawBF1A10 | In outline: the same text draw (another copy) |
| `0x9f1fa0` | Firework::Plane9F1FA0 | In outline: a firework's crossing of a plane (non-zero dot product) |
| `0xcbd5e0` | Shader::SetCBD5E0 | In outline: sets a shader part (`0xCBBB00`) |
| `0x9f75f0` | Firework::Colour9F75F0 | In outline: a firework colour with its alpha rounded from a scale |
| `0x8c69a0` | Terrain::Clip8C69A0 | In outline: a terrain clip step |
| `0x8e6e10` | Water::Normal8E6E10 | In outline: a water normal's length |
| `0xcbdd20` | Shader::StateCBDD20 | In outline: a shader state step for states under 0xE |
| `0x46be00` | Effect::Fill46BE00 | In outline: fills effect buffers for the current device |
| `0x46e480` | SceneryBehaviour::Step46E480 | In outline: a behaviour step from its owner (SceneryBehaviour::Owner) |
| `0x9d7200` | Fireworks::Pass9D7200 | In outline: a firework step when there is more than one |
| `0xca0740` | Shapes::AddCA0740 | In outline: adds a shape when there is room (+0x40 vs +0x44) |
| `0xceeb80` | Plane::SideCEEB80 | In outline: which side of a plane a box lies on |
| `0xcd0270` | Plane::SideCD0270 | In outline: the same plane side check (another copy) |
| `0xbf2c60` | Text::DrawBF2C60 | In outline: draws text with a size rounded from a scale |
| `0xbf2670` | Text::DrawBF2670 | In outline: the same text draw (another copy) |
| `0xc2c7b0` | Resource::ReleaseC2C7B0 | In outline: releases a part (`0xCA4420`) |
| `0x84ef90` | Scenery::Light84EF90 | In outline: a scenery step with the downward light direction (Lighting::DownwardDirection) |
| `0x8b6140` | SceneryBehaviour::Step8B6140 | In outline: a behaviour step from its owner (SceneryBehaviour::Owner) |
| `0x81e050` | SceneryBehaviour::Add81E050 | In outline: adds a behaviour to a slot pool (SlotPool::Add) |
| `0x408890` | Draw::Line408890 | In outline: a line draw when the length is not 0 |
| `0x8b07b0` | Scenery::Turn8B07B0 | In outline: turns a scenery item (Matrix::TurnAboutY by +0x140) |
| `0xc2f650` | Vector::LerpC2F650 | In outline: a blend of two points by a fraction |
| `0x8b8ca0` | SceneryBehaviour::Step8B8CA0 | In outline: a behaviour step from its owner (SceneryBehaviour::Owner) |
| `0xcbea90` | Shader::SetCBEA90 | In outline: sets a shader part when it has one |
| `0x42b590` | Sky::Step42B590 | In outline: a step when on (+0xC) |
| `0x94ca80` | Water::Length94CA80 | In outline: a water direction's length |
| `0xbfbee0` | Device::StepBFBEE0 | In outline: a device step (`0xBD3DC0`) |
| `0x9c5dc0` | Particles::Loop9C5DC0 | In outline: a particle loop |
| `0xac72a0` | PoolComplex::DrawAC72A0 | In outline: a draw step with an object (`0x13CE468`) |
| `0x9aa4a0` | Particles::Fill9AA4A0 | In outline: fills particle buffers for the current device |
| `0xcc66b0` | Model::PartsCC66B0 | In outline: goes over a model's parts when not flagged (+8) |
| `0xbf1410` | Text::DrawBF1410 | In outline: draws a text, character by character |
| `0xbf0e70` | Text::DrawBF0E70 | In outline: the same text draw (another copy) |
| `0x96eae0` | Effect::Matrix96EAE0 | In outline: works out a matrix (`0xCD78A0`) |
| `0x7b5880` | Effect::Pass7B5880 | In outline: goes over an effect's parts (+0x10) |
| `0x7b4320` | Effect::Pass7B4320 | In outline: goes over an effect's parts (+0x10) |
| `0x7b3690` | Effect::Check7B3690 | In outline: an effect check unless told not to |
| `0xc5f7a0` | Device::StepC5F7A0 | In outline: a device step when on (+0xF8) |
| `0xbf08e0` | Text::DrawBF08E0 | In outline: draws a text, character by character |
| `0xbf0340` | Text::DrawBF0340 | In outline: the same text draw (another copy) |
| `0x8b5f60` | Scenery::Flat8B5F60 | In outline: a flat (x, z) length step |
| `0xbf5d00` | Text::WidthBF5D00 | In outline: a text width when on (+0xEC) |
| `0x9aa2d0` | Particles::Make9AA2D0 | In outline: makes a part |
| `0x9f3b30` | Fireworks::Step9F3B30 | In outline: a firework step with a part |
| `0xcedd90` | Shapes::UpdateCEDD90 | In outline: a step when not flagged (+0x24) |
| `0xbfa010` | Text::DrawBFA010 | In outline: a text draw step (`0xC5C7F0`) |
| `0x9d3ad0` | Fireworks::Loop9D3AD0 | In outline: a loop |
| `0x9c65b0` | Particles::Slot9C65B0 | In outline: a step with no slot (+0xC4 negative) |
| `0x9c7340` | Particles::Add9C7340 | In outline: adds an entry when there is room (+0x98 vs +0x9C) |
| `0x9af320` | Fireworks::Part9AF320 | In outline: a step for a part not flagged (+0x18) |
| `0x43a940` | Effect::Make43A940 | In outline: makes a part |
| `0xbb8000` | Vector::LerpBB8000 | In outline: a blend of two points by a fraction |
| `0xccc400` | Model::FillCCC400 | In outline: fills buffers for the current device |
| `0xcdb0e0` | Plane::BelowCDB0E0 | In outline: whether a point is below a plane |
| `0xbfa640` | Text::DrawBFA640 | In outline: a text draw step (`0xC5C7F0`) |
| `0xc5d920` | Device::StepC5D920 | In outline: a step by a flag |
| `0xafd160` | Mesh::FillAFD160 | In outline: fills a mesh (Mesh::FillAF8FB0) |
| `0xcccc70` | Model::PassCCCC70 | In outline: goes over a model's parts (+0x44) |
| `0xa99c10` | PoolComplex::CameraA99C10 | In outline: a value from the camera (MainCamera::Get) |
| `0x428fb0` | Sky::Loop428FB0 | In outline: a loop |
| `0xa92360` | Fence::CameraA92360 | In outline: a value from the camera (MainCamera::Get) |
| `0x9e0b90` | Fireworks::Height9E0B90 | In outline: a step when a height is not above 0 |
| `0x481d40` | Draw::RedLine481D40 | In outline: draws a red line (0xFFFF0000) with the shared line drawer |
| `0xbf8c50` | Text::FillBF8C50 | In outline: fills buffers for the current device |
| `0x9ed500` | Firework::Step9ED500 | In outline: a firework step (`0x9F5440`) |
| `0xcd8520` | Camera::StepCD8520 | In outline: a step unless told not to |
| `0x47c240` | Effect::Update47C240 | In outline: an update when not flagged (+0xF) |
| `0x9a4ad0` | Fireworks::Step9A4AD0 | In outline: a firework step (`0x9A4A60`) |
| `0x410180` | Effect::Update410180 | In outline: a step when on (+8) |
| `0x81d1c0` | SceneryBehaviour::Step81D1C0 | In outline: a behaviour step from its owner |
| `0xbf75c0` | Text::FillBF75C0 | In outline: fills buffers for the current device |
| `0xcec7c0` | Mesh::PassCEC7C0 | In outline: goes over a mesh's entries |
| `0x481b90` | Draw::Line481B90 | In outline: a line draw with a normalised direction |
| `0xce85b0` | Shapes::FillCE85B0 | In outline: when on (+0x34), a fill step |
| `0xbfa380` | Text::DrawBFA380 | In outline: a text draw step (`0xC5C7F0`) |
| `0x9ea050` | Fireworks::Sound9EA050 | In outline: a step with the crowd sounds (Crowd::Sounds) |
| `0x971700` | Water::Alpha971700 | In outline: a step with a value rounded from a scale |
| `0x40bc00` | Effect::Face40BC00 | In outline: points an effect (Matrix::LookAlongZSide) |
| `0x9ea680` | Firework::Kind9EA680 | In outline: a value by kind (+0x10) |
| `0xccc250` | Model::FillCCC250 | In outline: fills buffers for the current device |
| `0xccbbf0` | Model::FillCCBBF0 | In outline: the same (another copy) |
| `0xbf7200` | Text::DrawBF7200 | In outline: a text draw step (`0xBD6E10`) |
| `0xbee2c0` | Vector::ScaleBEE2C0 | In outline: a step when a value is positive |
| `0x9e4530` | Fireworks::Clear9E4530 | In outline: clears list marks (Lists::ClearMarks) |
| `0xc3b510` | Resource::KindC3B510 | In outline: a step for a part of kind 5 (+0x60) |
| `0xa008f0` | Fireworks::Table | In outline: sets up a 0x100-entry table (`0x488290`) |
| `0xcf0c00` | Shapes::StepCF0C00 | In outline: a step unless told not to |
| `0xbcc4c0` | Device::TimeBCC4C0 | In outline: a step with a non-negative time |
| `0x424ec0` | Sky::Value424EC0 | In outline: a value when not flagged (+0x1C) |
| `0xbb7800` | Vector::StepBB7800 | In outline: a step when the shared flag (`0x13D07FC`) is set |
| `0xbefe00` | Text::DrawBEFE00 | In outline: draws a text, character by character |
| `0xbef8f0` | Text::DrawBEF8F0 | In outline: the same (another copy) |
| `0x97c2f0` | Water::Release97C2F0 | In outline: releases a part |
| `0xbf86c0` | Text::FillBF86C0 | In outline: fills buffers for the current device |
| `0x7b4670` | Effect::Check7B4670 | In outline: an effect check unless told not to |
| `0x47c550` | Effect::Part47C550 | In outline: a step with a part |
| `0xc8b8e0` | Device::StepC8B8E0 | In outline: a device step (`0xC051F0`) |
| `0xbf73a0` | Text::DrawBF73A0 | In outline: a text draw step (`0xBD6E10`) |
| `0xbed120` | Vector::LengthBED120 | In outline: a length step |
| `0xbde370` | VertexDeclaration::CaseBDE370 | In outline: a step by an element's kind |
| `0x99c360` | Fireworks::Count99C360 | In outline: a step for a positive count |
| `0xbed2b0` | Vector::LengthBED2B0 | In outline: a length step |
| `0xbf6e70` | Text::FillBF6E70 | In outline: fills buffers for the current device |
| `0xbecfa0` | Vector::LengthBECFA0 | In outline: a length step |
| `0x481790` | Draw::Add481790 | In outline: adds an entry when there is room (+0x20) |
| `0x9e2cb0` | Fireworks::Make9E2CB0 | In outline: makes a part |
| `0x43aeb0` | Effect::Step43AEB0 | In outline: a step (`0x438B20` with `0xF286F4`) |
| `0x9ee8b0` | Fireworks::Camera9EE8B0 | In outline: a step with the camera (MainCamera::Get) |
| `0xbf8540` | Text::FillBF8540 | In outline: fills buffers for the current device for n entries |
| `0xbf7f80` | Text::FillBF7F80 | In outline: the same (another copy) |
| `0xbf7770` | Text::FillBF7770 | In outline: the same (another copy) |
| `0x986750` | Fireworks::Make986750 | In outline: makes a list of 0x30-byte entries when on (+0x1C) |
| `0xceb410` | Shapes::PassCEB410 | In outline: goes over entries (+0x70) |
| `0xbfb340` | Vector::AxisBFB340 | In outline: a step when the first part is the largest |
| `0xbbd3d0` | Vector::LengthBBD3D0 | In outline: a length step with no slot (+8 negative) |
| `0x975660` | Water::Light975660 | In outline: a water step with the downward light direction |
| `0x974140` | Water::Light974140 | In outline: the same (another copy) |
| `0x445d40` | Effects::Slot445D40 | In outline: a step with a slot (`0x445AF0` with 4, 6) |
| `0x481620` | Draw::LineAdd | In outline: the shared line drawer: adds a line when there is room (+0xC) |
| `0x959160` | Water::Table959160 | In outline: fills a 12 by 12 table with 0xFFFF (`0x416110`) |
| `0xa187b0` | Bird::NearA187B0 | In outline: a step when close (`0x10F2014`) unless in mode 2 (+0xC4) |
| `0xb85c20` | Animal::FaceB85C20 | In outline: points (Matrix::LookAlongZSide) for n past a count (+0x44) |
| `0xcc73c0` | Model::StepCC73C0 | In outline: a step with no entries (+0x44) |
| `0xbbce20` | Vector::StepBBCE20 | In outline: a step when a value is above 1 |
| `0xbc4e20` | Vector::FlatBC4E20 | In outline: a flat length step |
| `0xacb0e0` | Graph::ValueACB0E0 | In outline: when not flagged (+0x100), sets a value (Set_+0xd0 from +0xFC +0x40) |
| `0x8b8b40` | SceneryBehaviour::Step8B8B40 | In outline: a step with no slot (+0xFC = -1) from its owner |
| `0x47c6e0` | Effect::Part47C6E0 | In outline: a step with a part not flagged (+0x11) |
| `0xbb7ea0` | Vector::LerpBB7EA0 | In outline: a blend (Vector::LerpC2F650) when a value is below 0 |
| `0x9f7e30` | Firework::Fill9F7E30 | In outline: aims a firework (Firework::Aim9F7370) and fills buffers for the current device |
| `0x9d6680` | Fireworks::Pass9D6680 | In outline: goes over entries (+0x38) and activates them |
| `0x9f5c50` | Fireworks::Loop9F5C50 | In outline: a loop |
| `0x9bce00` | Particles::Source | In outline: makes a particle source (ParticleSource) |
| `0x428090` | Sky::Direction428090 | In outline: a direction from values (+0x3F8 ...), with length 0 handled |
| `0x7554c0` | Guest::Length7554C0 | In outline: a length step with entries (+0x510) |
| `0x425060` | Curves::Key425060 | In outline: a step for a non-negative time up to a key |
| `0xcd0510` | Camera::TimeCD0510 | In outline: a step with a non-negative time unless in mode 1 |
| `0xcd7dc0` | Camera::RayCD7DC0 | In outline: when on (+0x1A6), a step with the tangent of half the field of view (+0x190) |
| `0xbc20b0` | Track::TurnBC20B0 | In outline: turns by a byte times a step (Matrix::TurnAboutY) |
| `0xa5fab0` | Animal::RayA5FAB0 | In outline: a step with a screen ray (Camera::ScreenRay) |
| `0x487600` | Effect::Range487600 | In outline: a step when two values are the same (+0x38, +0x3C) |
| `0xccc5d0` | Model::FillCCC5D0 | In outline: fills buffers for the current device and the shared model (`0x13FA030`) |
| `0xbb7a80` | Track::StepBB7A80 | In outline: a step (`0xBC28D0`), with 0xFFFF handled |
| `0xc63d50` | Window::ClipC63D50 | In outline: a clip step comparing values |
| `0xc213b0` | Curves::StepC213B0 | In outline: a step past a count (+0x34) with a value under 1 |
| `0xbb76b0` | Track::PassBB76B0 | In outline: goes over entries (+0x50) that are set |
| `0xcdf200` | Vector::CompareCDF200 | In outline: a step comparing values |
| `0xbee460` | Vector::NormalBEE460 | In outline: a normalise step when a value is small |
| `0xcd7bb0` | Camera::FaceCD7BB0 | In outline: points the camera (Matrix::LookAlongZSide on +0x140, +0x130) |
| `0xbef450` | Text::DrawBEF450 | In outline: draws a text, character by character |
| `0xbef020` | Text::DrawBEF020 | In outline: the same (another copy) |
| `0x7b5a60` | Effect::Pass7B5A60 | In outline: goes over an effect's parts (+0x10) that are set |
| `0x489e60` | Effect::Pass489E60 | In outline: the same (another copy) |
| `0xbbd290` | Vector::LengthBBD290 | In outline: a step when a length is above a limit |
| `0x864620` | Scenery::Shapes864620 | In outline: checks a model's shapes (Model::CheckShapes) against a point |
| `0x864010` | Scenery::Shapes864010 | In outline: the same with Model::CheckShapes_(2) |
| `0x436c00` | Camera::Step436C00 | In outline: a step with the camera (MainCamera::Get, `0xCD6850`) |
| `0xbbee40` | Track::CaseBBEE40 | In outline: a value by case |
| `0x426560` | Curves::Time426560 | In outline: a step for a negative time or a large value |
| `0x988660` | Fireworks::Step988660 | In outline: a step with parts (+0xD4, +0xD8) when a check passes (`0x475610`) |
| `0x998a40` | Fireworks::Add998A40 | In outline: adds an entry, growing a list of 16-byte entries when full (+0x1F8, +0x1FC) |
| `0xbf6ff0` | Text::WaveBF6FF0 | In outline: fills buffers for the current device with a sine of a value |
| `0x414630` | Effect::Length414630 | In outline: a length step over entries (+0x98) |
| `0xcd87a0` | Camera::FaceCD87A0 | In outline: points (Matrix::LookAlongZSide) |
| `0xc53530` | Stream::StepC53530 | In outline: when on (+0x8C), a step under a count (+0x98 vs +0x58) |
| `0x480e40` | Camera::Step480E40 | In outline: a step with the camera (MainCamera::Get, `0xCD6850`) |
| `0xbf8850` | Text::WaveBF8850 | In outline: fills buffers for the current device with a sine of a value |
| `0x9f84e0` | Fireworks::Value9F84E0 | In outline: a value in mode 1 |
| `0x993dd0` | FireworkSlot::AddKey | In outline: a step with a slot's key (FireworkSlot::KeyAt) and makes a key frame (FireworkSlotKeyFrame) |
| `0x4686d0` | Grid::Round4686D0 | In outline: a value rounded |
| `0x4414f0` | Grid::Round4414F0 | In outline: a value rounded |
| `0xbedc00` | Vector::NormalBEDC00 | In outline: a normalised direction from values (+0x20 ...), with length 0 handled |
| `0x98bad0` | Animal::Value98BAD0 | In outline: a value with a part (+0x48) and an object (`0x13CE508`) |
| `0xc72210` | Device::StepC72210 | In outline: when on (+0x4C1), a step with a check |
| `0x430db0` | Effect::Shared430DB0 | In outline: when on (+8) and the global flag `0x1400306` is unset, a step |
| `0x9ec410` | Fireworks::Timer9EC410 | In outline: when on (+0x1F0), a timed step (Timer::Milliseconds) |
| `0x485e80` | Model::Draw485E80 | In outline: draws two models (Model::Draw484F60 on +0x1F4, +0x20C) |
| `0xbcc090` | Vector::StepBCC090 | In outline: a step with points |
| `0xce5ce0` | Sound::NearCE5CE0 | In outline: a step when a point is within a distance (+0x84) |
| `0xbf8e00` | Text::FillBF8E00 | In outline: fills buffers for the current device (Call10IfReady) |
| `0xbc63b0` | Track::LengthBC63B0 | In outline: a length step with fewer than two stations |
| `0xbb69e0` | Track::RoundBB69E0 | In outline: a value rounded from a scale (+0x2C) |
| `0xac8d20` | LaserDesigner::RandomColour | In outline: a random value (_rand) and a colour blend element (ColourBlenderElement) |
| `0x48a5c0` | Effect::Check48A5C0 | In outline: an effect part check |
| `0x4493f0` | Effect::Walk4493F0 | In outline: a walk over two points with values up to 0 |
| `0x42fb10` | Effect::Walk42FB10 | In outline: the same (another copy) |
| `0x9fba50` | Fireworks::Draw9FBA50 | In outline: a draw step with the shared device (`0x13D0DB8`) |
| `0x9cd320` | Particles::Set9CD320 | In outline: sets a value (+0xA0), refreshes (`0x9C60A0`, `0x9C6900` with +0xA4) and makes a handle |
| `0x9718a0` | Water::Colour9718A0 | In outline: a colour with an alpha rounded from a value (+0xC) |
| `0xca3c10` | Font::StepCA3C10 | In outline: a step when a value (+4 +0xE8) is positive |
| `0xca38d0` | Font::StepCA38D0 | In outline: the same (another copy) |
| `0x5e10c0` | TrackPiece::Time5E10C0 | In outline: a step when a time difference is below 0 (`0x5D97A0`) |
| `0x58b0a0` | Ride::Time58B0A0 | In outline: a timed step (`0x58A7A0`) over entries of 0x24 bytes |
| `0x8b5ac0` | Scenery::Face8B5AC0 | In outline: points (Matrix::LookAlongZSide on +0xB0) from a set matrix |
| `0xbc62a0` | Track::LengthBC62A0 | In outline: a step for rides not flagged 0x400000 or with fewer than two stations |
| `0xbb7030` | Track::PointBB7030 | In outline: a point step when a value is not 0 |
| `0xa62d90` | Camera::PickA62D90 | In outline: a ray from a screen point (Camera::ScreenRay) with the GUI root's size |
| `0x9cdba0` | Particles::TransformC | Transforms a particle's point by the shared matrix (`0x1341580` on) and stores it |
| `0x7a3f30` | Colours::Init7A3F30 | In outline: once (`0x1306C5C`), sets shared colours (`0x1306C50` on) |
| `0x645eb0` | Colours::Init645EB0 | In outline: once (`0x12F7FA8`), sets shared colours (`0x12F7F9C` on) |
| `0xcf1080` | Shapes::DrawCF1080 | In outline: a draw step with the shared device (`0x13D0DB8`, slot 0xB8) over entries |
| `0x470160` | Device::Viewport470160 | In outline: sets the shared device's view to 640 by 480 (slot 0x6C) |
| `0x4831f0` | Fireworks::Text4831F0 | In outline: when given a text, makes a part and sets a fade of 0.5 (Firework::SetFade) |
| `0x476320` | Effect::Step476320 | In outline: a step with its part (+0x40, slot 0x14) |
| `0x489310` | Effect::Pass489310 | In outline: goes over entries (+0x10) |
| `0xbb7470` | Track::PassBB7470 | In outline: goes over indexed entries (+0x10 +0x20) |
| `0x727bb0` | Path::Values727BB0 | In outline: fills values, with the shared one (`0x13063C0`) when flagged (bit 0) |
| `0x9ef370` | Fireworks::Pick9EF370 | In outline: picks a firework (Fireworks::Pick9EEF30) into +0x140 |
| `0x9ea1f0` | Fireworks::Text9EA1F0 | In outline: in mode 1 (`0x10ED6E0`), a number from a scaled value, rounded |
| `0xac9f90` | Graph::MarkAC9F90 | In outline: when on (+0x25), a step at the centre of its part |
| `0x9ceb91` | Particles::Step9CEB91 | In outline: a step with entries (+0x84) |
| `0x489c50` | Effect::Part489C50 | In outline: goes over a part's entries |
| `0x9e4d50` | Fireworks::Make9E4D50 | In outline: makes parts (`0xBDADC0`, Constructor on +0x58) |
| `0x958b40` | Water::Device958B40 | In outline: a step with the shared device's value (slot 0xCC), counting its bits |
| `0x7b9aa0` | Person::Point7B9AA0 | In outline: a point step from values (+0x70 on) |
| `0x420da0` | Effect::Release420DA0 | In outline: releases each entry's part (+0x118, +0x11C) |
| `0xcedb50` | Shapes::StepCEDB50 | In outline: a step for entry n (0x70 bytes) not flagged (+0x50) |
| `0x9fcbf0` | Fireworks::Draw9FCBF0 | In outline: a draw step with the shared device over entries (+0x6C) |
| `0x47ccd0` | Effects::Loop47CCD0 | In outline: goes over the shared list (`0x10EDDA8`) |
| `0xbf8b50` | Text::WaveBF8B50 | In outline: a cosine step |
| `0xcd8f90` | Camera::PassCD8F90 | In outline: goes over entries (+0xE0, +0xE4) that are set |
| `0xccf530` | Model::BlendCCF530 | In outline: a blend by a fraction with fixed scales |
| `0xc776d0` | Device::SetC776D0 | In outline: when not flagged (+0x4C1), sets a value when it changes |
| `0xcf1190` | Shapes::DrawCF1190 | In outline: a draw step with the shared device (slot 0xB8) over entries |
| `0x9ef270` | Fireworks::Clear9EF270 | In outline: with no part (+0x140), clears another's value (+0x144 +0x5C) |
| `0x940ea0` | Water::Make940EA0 | In outline: makes a part |
| `0xbf9660` | Text::WaveBF9660 | In outline: a cosine step |
| `0x9e16e0` | Particles::Pass9E16E0 | In outline: when on (+0x10), goes over entries (+0x1C) |
| `0x9c5a80` | Fireworks::Update9C5A80 | In outline: an update when not flagged (+0x30) |
| `0x7b4570` | Effect::Pass7B4570 | In outline: goes over entries (+0x10) |
| `0xb6d680` | Effect::PassB6D680 | In outline: goes over entries (+0x10) |
| `0x970b40` | Water::Make970B40 | In outline: makes a part (`0x966450`) |
| `0xbf8a50` | Text::FillBF8A50 | In outline: fills a two-entry buffer for the current device |
| `0x9bc490` | Particles::Pass9BC490 | In outline: goes over entries (+0x50), calling itself |
| `0x6f13c0` | Window::Value6F13C0 | In outline: a value from a count (+0xD0) |
| `0xb711a0` | Effect::PartB711A0 | In outline: goes over a part's entries |
| `0x7b5780` | Effect::Part7B5780 | In outline: the same (another copy) |
| `0xac7840` | Graph::SetAC7840 | In outline: links its part (+0xF0) back to itself (+0xF4) |
| `0x7b32c0` | Effect::Inside7B32C0 | In outline: whether a box lies inside its bounds |
| `0x7b2fc0` | Effect::Inside7B2FC0 | In outline: the same (another copy) |
| `0x488710` | Effect::Inside488710 | In outline: the same (another copy) |
| `0x9f5db0` | Fireworks::Check9F5DB0 | In outline: 0 with no list (+0x4C) |
| `0x9e2970` | Fireworks::Next9E2970 | In outline: steps a value (+0x1C) up to the shared count (`0x10F0E84`) |
| `0xb5c450` | Effect::PassB5C450 | In outline: goes over entries (+0x10) |
| `0xaa96e0` | PoolComplex::HandleAA96E0 | In outline: makes a handle and links it |
| `0x9f5ea0` | Fireworks::Check9F5EA0 | In outline: a check (Fireworks::Check9F5DB0) at a scaled time |
| `0x9c7a10` | Particles::Pass9C7A10 | In outline: goes over entries (+0x54, +0x58) |
| `0xb82fd0` | Track::MakeB82FD0 | In outline: makes a part (`0xBC2CA0`) |
| `0xbbe480` | Track::StepBBE480 | In outline: a step with two parts (+0xC) |
| `0x9e2640` | Fireworks::Step9E2640 | In outline: a step when a value is not 0 (+0x58) |
| `0xb5cda0` | Effect::PartB5CDA0 | In outline: goes over a part's entries |
| `0xcd8060` | Camera::PassCD8060 | In outline: with a part (+8), goes over its values |
| `0x9f4c80` | Fireworks::List9F4C80 | In outline: goes over a list (+0x4C) |
| `0xaf4aa0` | Animal::ColourAF4AA0 | In outline: makes a colour blend element (ColourBlenderElement) when not flagged (+0x98) |
| `0xad8aa0` | Animal::ColourAD8AA0 | In outline: the same when not flagged (+0x90) |
| `0x9fbb70` | Fireworks::Draw9FBB70 | In outline: a draw step with the shared device |
| `0x9c4a90` | Particles::Fill9C4A90 | In outline: fills a record from its arguments |
| `0x8db980` | Terrain::Case8DB980 | In outline: sets a shared callback (`0x10F0AD8`) and acts by it |
| `0x9bc2d0` | Particles::Pass9BC2D0 | In outline: goes over entries (+0x3C) from n |
| `0xa9a980` | PoolComplex::SetA9A980 | In outline: sets values (+0x1C4 on) |
| `0x45b520` | Resource::Reset45B520 | In outline: with a part, tears it down and acts |
| `0xb02320` | Animal::CameraB02320 | In outline: a reset (`0x501FB0`) and a step with the camera unless the shared value is set |
| `0xad89c0` | Animal::FloatAD89C0 | In outline: makes a float blend element (FloatBlenderElement) and acts when not flagged (+0xAC) |
| `0xaf49c0` | Animal::FloatAF49C0 | In outline: the same when not flagged (+0xB4) |
| `0xad88e0` | Animal::FloatAD88E0 | In outline: the same when not flagged (+0xC8) |
| `0xa60e40` | Animal::DirectionA60E40 | In outline: a direction between two points |
| `0x978da0` | Water::Make978DA0 | In outline: makes a part (Object967) |
| `0xcb7060` | Mesh::StepCB7060 | In outline: a step with parts (+0x54, +0xAC) |
| `0x44d180` | Effect::Length44D180 | In outline: a step when a direction's length is above a limit |
| `0x44c5f0` | Effect::Make44C5F0 | In outline: makes parts |
| `0x974d80` | Water::Make974D80 | In outline: makes a part (Object967) |
| `0x48a6e0` | Effect::Part48A6E0 | In outline: goes over a part's entries |
| `0x9fbc60` | Fireworks::Draw9FBC60 | In outline: a draw step with the shared device (`0x454930`) |
| `0xbc5d50` | Track::TurnBC5D50 | In outline: a matrix turned by (n + 2) times a step (Matrix::TurnColumnsAboutY) |
| `0xa5edc0` | Animal::ClampA5EDC0 | In outline: keeps a value under 1000 |
| `0x470eb0` | Draw::Step470EB0 | In outline: a draw step |
| `0x473e60` | Labels::Add473E60 | In outline: adds a label when there is room (+8, +0xC) |
| `0xbf8980` | Text::FillBF8980 | In outline: a fill step with a value |
| `0xbf7130` | Text::FillBF7130 | In outline: the same (another copy) |
| `0x9c6780` | Particles::Step9C6780 | In outline: a step with three values |
| `0x476fb0` | Effects::Find476FB0 | In outline: unless told not to, a find over the shared list (`0x10ECA14`) |
| `0x955a10` | Water::Draw955A10 | In outline: the large water draw: reads the device's view matrix (slot 0xB4), draws its pieces (Water::DrawPieces9525D0, `0x954100`), works out a colour from its values (+0x1E8 on, Colour::ToARGB), grows shared buffers in 4096 steps (Array::Resize94E4B0, `0x493860`) and fills shared draw state (`0x133A3D8` on) |
| `0x8c8d30` | Terrain::Draw8C8D30 | In outline: a large terrain draw with the shared device: sets an identity matrix (slot 0xB8), reads the view matrix (slot 0xB4) and draws |
| `0x8d4650` | Terrain::DrawTimed8D4650 | In outline: a large timed terrain draw: once per frame (`0x13119AC`) enters a timed section (`0x1311658`, Timer::Ticks), sets device state (slot 0xA4 with 9, 0xF) and draws |
| `0xc8c160` | Device::FlushC8C160 | In outline: when on (+0x4C1), goes over the queued draws (+0x1A60, +0x1A64) and sets state when it changes (+0x67C, +0x2088) |
| `0xc8e800` | Device::SetStateC8E800 | In outline: a large device state set: when not flagged, compares and takes a value (+0x6D0 vs +0x20D0) and marks what changed (+0x674, +0x67C) |
| `0x950f30` | Water::Surface950F30 | In outline: a large water surface step: works out a normalised direction from a point and fills draw values |
| `0x954100` | Water::Sort954100 | In outline: when on (+0x14), works out each piece's distance along a direction (0x80-byte pieces) and draws them in order |
| `0x8fb360` | Water::Fill8FB360 | In outline: a large water fill: with the water manager and a part (`0x949210`), fills a device buffer for the current device |
| `0x9fcda0` | Fireworks::DrawMap | In outline: a large draw from above: clears the device (black, depth 1), sets an orthographic view over the terrain's size (Matrix::Ortho from 1 to 1000) and draws |
| `0x9af4f0` | Fireworks::Draw9AF4F0 | In outline: a large firework draw with the shared device and colours (Colour::ToARGB) |
| `0x986d40` | Fireworks::Draw986D40 | In outline: a large firework draw with the device's view matrix (slot 0xB4) and the shared object (`0x133A380`) |
| `0xc91780` | Device::CameraC91780 | In outline: a large camera step when asked or when its values are set: works out matrices from its part (+0x2138) |
| `0xcdcf50` | Shapes::BuildCDCF50 | In outline: a large shape build: works out vertex and index counts, makes a 16-byte-entry list filled with the largest float, and fills it (4K and 2K-entry work tables) |
| `0x8f1060` | Terrain::Colours8F1060 | In outline: blends neighbouring colours (their red, green, blue bytes) along the edges, rounded |
| `0x952f50` | Water::Sort952F50 | In outline: when on (+0x14), works out each piece's distance along a direction and draws them in order (another copy of Water::Sort954100) |
| `0x71f4c0` | Path::Draw71F4C0 | In outline: a large path draw: sets a shared flag (`0x1400304` bit 0), a depth bias (`0x14002FC`), a matrix (`0xCD75C0`) and draws with the shared device |
| `0x9cf070` | Particles::Draw9CF070 | In outline: for n particles, works out a camera-facing direction from the view matrix (slot 0xB4), normalised, and fills quads |
| `0x9d1580` | Particles::Draw9D1580 | In outline: grows buffers to twice n (`0x9D1120`, `0x9CE490`) and fills camera-facing quads from the view matrix |
| `0x9183b0` | Water::Corners9183B0 | In outline: works out a box's corners from a matrix and sizes |
| `0xc23870` | Skeleton::StepC23870 | In outline: a large step comparing two points' distances from a centre (+0x80) over bones |
| `0x8103e0` | Regions::DrawTimed8103E0 | In outline: a large timed draw (`0x130ACA8`, Timer::Ticks) with a depth bias and the shared device |
| `0xcd69c0` | Camera::FrustumCD69C0 | In outline: works out the view's extents from the tangent of half the field of view (+0x190), the near distance (+0x1A0) and the aspect (+0x194) |
| `0xc1fc30` | Matrix::Invert4 | In outline: inverts a 4x4 matrix from its cofactors |
| `0x408a90` | Sky::Draw408A90 | In outline: a large draw with the downward light direction (Lighting::DownwardDirection), turned around |
| `0x9d30e0` | Fireworks::Draw9D30E0 | In outline: a large draw with a matrix, the shared one (`0x1400160`) when none, after stepping entries (+0xB8) |
| `0x9e79e0` | Fireworks::Draw9E79E0 | In outline: a large draw with the shared day (`0x1103B6C` +0x110) and the shared device |
| `0x9437b0` | Water::Map9437B0 | In outline: a large step with a scale from the terrain's cell size and shared values (Matrix::ScaleOffset2D) |
| `0x932ac0` | Water::Draw932AC0 | In outline: a large draw with a ride part and the shared device |
| `0x7bf0c0` | Model::Bone7BF0C0 | In outline: works out a pose from a bone's 64-byte matrix (+0x18 list) and its values |
| `0xb82090` | Model::BoneB82090 | In outline: the same from bone n's matrix (+0xB0 +0x20) |
| `0x7bee20` | Model::Bone7BEE20 | In outline: the same (another copy) |
| `0x8625d0` | Scenery::Light8625D0 | In outline: when on (+0x138), a light step for part n (0x48 bytes) by kind (3, 4) and flags |
| `0x9e7240` | Sky::Sun9E7240 | In outline: a large step from the shared day's sun direction (`0x1103B6C` +0x120 on), turned below the horizon |
| `0xbddc00` | VertexDeclaration::CopyBDDC00 | In outline: copies vertex elements by their kinds and offsets |
| `0xbde5e0` | VertexDeclaration::FillBDE5E0 | In outline: fills a declaration's header and copies its elements by kind |
| `0xbde860` | VertexDeclaration::FillBDE860 | In outline: the same (another copy) |
| `0x89fef0` | SceneryRenderer::Add89FEF0 | In outline: adds a draw, growing its list by 0x200 when full (+0x3B8 on) |
| `0x91b0c0` | Water::Indices91B0C0 | In outline: fills index lists by a flag (bit 0x40) and the shared count (`0x10F0BEC`) |
| `0x91abb0` | Water::Indices91ABB0 | In outline: the same for a grid of counts (+0x154, +0x158) |
| `0x8bdf80` | Terrain::Unproject8BDF80 | In outline: inverts the view matrix (slot 0xB4, Matrix::Invert4) and scales by the terrain's size |
| `0x857090` | Scenery::Fill857090 | In outline: adds up n parts' counts and fills buffers for the current device |
| `0xcd75c0` | Camera::Projection | In outline: works out the camera's projection from its near and far values (+0x188, +0x18C), a zoom (+0x19C) and the tangent of half the field of view |
| `0x438600` | Screen::Size438600 | In outline: the GUI root's size rounded to whole pixels and a step |
| `0x8f0d30` | Terrain::Indices8F0D30 | In outline: for a square grid of more than one cell, fills a short index list |
| `0x919610` | Water::Fade919610 | In outline: a fade from a value (+0x31C) between two limits unless told not to |
| `0xcd6850` | Camera::CopyFrom | Copies a camera's field of view and distances from another and clears its slot |
| `0x4368b0` | Labels::Screen4368B0 | In outline: with the GUI root, sets a part's size from it |
| `0x950e10` | Fireworks::AddShell | In outline: adds a firework shell to a list (+0x1B0) from its many settings |
| `0x8534a0` | Scenery::Table8534A0 | In outline: once (`0x130BC90`), fills a shared six-entry table (Table::Fill16) and points at it |
| `0x431070` | Sky::Table431070 | In outline: once (`0x1103C50`), fills a shared table (Table::Fill16) and points at it |
| `0x97c630` | Water::Alpha97C630 | In outline: sets parts' alpha (+0xC4) to 0.3 when it has no part |
| `0x694e90` | Layout::Add694E90 | In outline: grows the layout table by 0x100 when needed (Array::Resize6930B0) and adds |
| `0x9fcaf0` | Fireworks::Draw9FCAF0 | In outline: draws each entry (+0x64, +0x6C, 0x30 bytes) |
| `0x692710` | Text::Draw692710 | In outline: draws a text at a size, with its default font when none |
| `0x7b3b20` | Effect::Normalise7B3B20 | In outline: scales values by one over a total (+0x44) |
| `0x9ceca0` | Particles::Remove9CECA0 | In outline: finds and removes an entry (+0x84, +0x88) |
| `0x424000` | Curves::Sample424000 | In outline: a value for a time between 0 and an end (+0x20) |
| `0x48c300` | Effect::Step48C300 | In outline: a step with points |
| `0x7b2730` | Effect::Set7B2730 | In outline: sets a point and clears a value |
| `0xbe68a0` | Device::Size | In outline: a size from a part's value (slot 0x38) |
| `0xa51870` | Window::SumA51870 | In outline: a value from entries (+0xC, +0x10) |
| `0xa0a340` | Bird::ClampA0A340 | In outline: clears a value (+0xB0) outside fixed bounds |
| `0x466470` | Effect::Fraction466470 | In outline: clears values and works out a fraction |
| `0x9fbd30` | Fireworks::Size9FBD30 | In outline: a size (+0x74), halved when the shared value is set (`0x10EDB90`) |
| `0x47c4e0` | Effects::Set47C4E0 | In outline: sets shared entry n's value (`0x10EDD40`) and acts |
| `0x9d4de0` | Fireworks::Part9D4DE0 | In outline: with no part, clears a value (+0xCC) |
| `0xb73380` | Effect::PartB73380 | In outline: goes over a part's entries |
| `0xb731e0` | Effect::PartB731E0 | In outline: the same (another copy) |
| `0xb5d850` | Effect::PartB5D850 | In outline: the same (another copy) |
| `0xb5d6b0` | Effect::PartB5D6B0 | In outline: the same (another copy) |
| `0x7b7460` | Effect::Part7B7460 | In outline: the same (another copy) |
| `0x7b72c0` | Effect::Part7B72C0 | In outline: the same (another copy) |
| `0x474450` | Labels::Step474450 | In outline: a step with many values |
| `0x9103a0` | Water::Clear9103A0 | In outline: clears the device's depth (slot 0x3C with 4, 1.0) and acts |
| `0x488b10` | Effect::Step488B10 | In outline: a step with points |
| `0xa6fc60` | Animal::SolveA6FC60 | In outline: solves a constraint (Constraint::SolveA6CA10) and goes over entries |
| `0xbc5fd0` | Track::StepBC5FD0 | In outline: a step for a positive count |
| `0x4266a0` | Sky::Point4266A0 | In outline: a step with a point (+0x3F0 on) |
| `0x40cd60` | Effect::Entry40CD60 | In outline: a step for entry n (0x70 bytes), when asked |
| `0x80c3e0` | Regions::Draw80C3E0 | In outline: when a shared flag is set (`0x12F018C` +0x20), draws a box in black |
| `0xb74190` | Effect::FlagB74190 | In outline: when flagged (bit 4 of +8), a step |
| `0xb5e100` | Effect::FlagB5E100 | In outline: the same (another copy) |
| `0x7b7e70` | Effect::Flag7B7E70 | In outline: the same (another copy) |
| `0x44b2a0` | Effect::Entry44B2A0 | In outline: for entry n (0x40 bytes) under a count (+0x28), a step |
| `0x9e2fe0` | Fireworks::Entry9E2FE0 | In outline: a step for entry n (0x20 bytes) |
| `0x9e87e0` | Fireworks::Entry9E87E0 | In outline: with no shared flag (`0x10EDAE0`), a step for entry n of a shared table (`0x10F1CD0`) |
| `0x9fc310` | Fireworks::Step9FC310 | In outline: a step with many values |
| `0x9fc980` | Fireworks::Entry9FC980 | In outline: a step for entry n (0x30 bytes) |
| `0x46fb40` | Labels::Flag46FB40 | In outline: when flagged (bit 2 of +0x354), a step |
| `0x48aff0` | Effect::Flag48AFF0 | In outline: when flagged (bit 4 of +8), a step |
| `0x7b3890` | Effect::Step7B3890 | In outline: a step with a work buffer |
| `0xb6c100` | Effect::StepB6C100 | In outline: the same (another copy) |
| `0xb74fb0` | Effect::BoundsB74FB0 | In outline: when flagged (bit 4 of +8), updates bounds (QuadTree::UpdateBounds) |
| `0xb74dd0` | Effect::BoundsB74DD0 | In outline: the same (another copy) |
| `0xb5ecb0` | Effect::BoundsB5ECB0 | In outline: the same (another copy) |
| `0xb5ead0` | Effect::BoundsB5EAD0 | In outline: the same (another copy) |
| `0x7b94e0` | Effect::Bounds7B94E0 | In outline: the same (another copy) |
| `0x7b9300` | Effect::Bounds7B9300 | In outline: the same (another copy) |
| `0x48bd10` | Effect::Bounds48BD10 | In outline: the same (another copy) |
| `0x48bc60` | Effect::Bounds48BC60 | In outline: the same (another copy) |
| `0x431f00` | Sky::Entry431F00 | In outline: for entry n under a count (+0x4C), a step with a point |
| `0x9feae0` | Fireworks::Shared9FEAE0 | In outline: with a shared value (`0x10EDB90`), a step |
| `0x7b05e0` | Effect::Value7B05E0 | In outline: when asked, a fixed value (`0xF264E0`) |
| `0x44ea10` | Effect::Value44EA10 | In outline: a value when not flagged (+0x60) |
| `0x9faae0` | Fireworks::Entry9FAAE0 | In outline: a step for entry n (0x90 bytes) |
| `0xb75570` | Effect::StepB75570 | In outline: a step with points |
| `0xb5f270` | Effect::StepB5F270 | In outline: the same (another copy) |
| `0x7b9bb0` | Effect::Step7B9BB0 | In outline: the same (another copy) |
| `0x7b3580` | Effect::Hit7B3580 | In outline: a check with a point plus an offset |
| `0x488a00` | Effect::Hit488A00 | In outline: the same (another copy) |
