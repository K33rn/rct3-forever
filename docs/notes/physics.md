# Physics

The rigid-body physics engine (crashing ride cars, the buggy, thrown and dropped objects): bodies made of points, collided with the terrain, water and collision triangles.

| Address | Name | What it does |
|---|---|---|
| `0xa7a290` | PhysicsWorld::Step | One step of the rigid-body physics: every body added since the last step (list at +0x78) is paired with each active body it can touch (not one it is jointed to, nor in the same group; `0xA720B0` makes the pair) and becomes active (+0x6C); then the contacts are worked out (`0xA6C4D0`, `0xA6C3A0`), the bodies collided with the terrain (`0xA75A00`) and each body moved on (`0xA6F930`) |
| `0xa75a00` | PhysicsWorld::CollideTerrain | Collides the physics bodies with the ground: makes sure the terrain (`RCT3Terrain`, 90 x 90) exists, then for each body group and body tests its points against the terrain heights and the water, making contacts |
| `0xa77740` | RigidBody::RigidBody | Sets up a rigid body of one of five shapes (a switch on the shape): a box is 8 corner points at plus or minus its half-sizes (each made by `0xA6B140`); damping starts at 0.99 and the contact bounds empty (`FLT_MAX`). Used by crashing ride cars (`0x506ED0`), the buggy (`0xA65730`), `0x7C6580` and `0xB587E0` |
| `0xa68080` | PhysicsWorld::CollectTriangles | Collects the collision triangles bodies can hit: for each object in a list, finds its collision mesh and copies each triangle's corners into the physics world's triangle list (`0x1342A00`, count `0x13429E8`), transformed by the object's matrix |
| `0xa73df0` | RigidBody::FindContacts | Works out a body's contacts with the collected triangles: refreshes the body when it moved (`0xA6F3A0`), resets the contact tables (`0x13C9C4C`, `0x13CDC4C`) to "far" and returns the deepest penetration in its third argument |
