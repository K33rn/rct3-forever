# Physics

The rigid-body physics engine (crashing ride cars, the buggy, thrown and dropped objects): bodies made of points, collided with the terrain, water and collision triangles.

| Address | Name | What it does |
|---|---|---|
| `0xa7a290` | PhysicsWorld::Step | One step of the rigid-body physics: every body added since the last step (list at +0x78) is paired with each active body it can touch (not one it is jointed to, nor in the same group; `0xA720B0` makes the pair) and becomes active (+0x6C); then the contacts are worked out (`0xA6C4D0`, `0xA6C3A0`), the bodies collided with the terrain (`0xA75A00`) and each body moved on (`0xA6F930`) |
| `0xa75a00` | PhysicsWorld::CollideTerrain | Collides the physics bodies with the ground: makes sure the terrain (`RCT3Terrain`, 90 x 90) exists, then for each body group and body tests its points against the terrain heights and the water, making contacts |
| `0xa77740` | RigidBody::RigidBody | Sets up a rigid body of one of five shapes (a switch on the shape): a box is 8 corner points at plus or minus its half-sizes (each made by `0xA6B140`); damping starts at 0.99 and the contact bounds empty (`FLT_MAX`). Used by crashing ride cars (`0x506ED0`), the buggy (`0xA65730`), `0x7C6580` and `0xB587E0` |
| `0xa68080` | PhysicsWorld::CollectTriangles | Collects the collision triangles bodies can hit: for each object in a list, finds its collision mesh and copies each triangle's corners into the physics world's triangle list (`0x1342A00`, count `0x13429E8`), transformed by the object's matrix |
| `0xa73df0` | RigidBody::FindContacts | Works out a body's contacts with the collected triangles: refreshes the body when it moved (`0xA6F3A0`), resets the contact tables (`0x13C9C4C`, `0x13CDC4C`) to "far" and returns the deepest penetration in its third argument |
| `0xbed970` | Matrix::LookAlongZ | Sets a matrix's rotation rows from a direction and an up hint: the direction normalised as row 2 (0,0,1 when it is zero), row 1 the normalised cross product direction x up, row 0 the cross product of rows 1 and 2. Used by RideCarSwingingPart::Update and AnimalCamera::Update |
| `0xbed820` | Matrix::LookAlongY | The same with the direction as row 1: row 2 the normalised cross product up x direction, row 0 completing the set from the other two. Used by RideCarSpinningPart::Update and WildAnimalLocomotorMoveTo::Update |
| `0xbed590` | Matrix::LookAlongX | The same with the direction as row 0: row 1 the normalised cross product up x direction, row 2 completing the set. Used by RideCar::FindWheels |
| `0xbed440` | Matrix::LookAlongXUp | The direction as row 0 and row 2 the normalised cross product direction x up, row 1 completing the set. Used by RideCarSwingingPart::Update |
| `0xbedac0` | Matrix::LookAlongZSide | The direction as row 2 and row 0 the normalised cross product up x direction, row 1 completing the set. Used by Shape::Draw, Helicopter::Activate and WaterCannon::Update |
| `0x52bae0` | RideCarSpinningPart::TrackMotion | Works out a spinning part's velocity and acceleration from its position over time (given the frame time; nothing when it is not positive): the first call keeps the position (+0x90 of the part at +0xF4) at +0x1D0, the second the velocity (change over time) at +0x1E0, later calls also the acceleration at +0x1F0; +0x1C8 counts how many are known (0-3). Called from RideCarSpinningPart::Update |
| `0xbed6e0` | Matrix::LookAlongYSide | The direction (normalised) as row 1 and row 0 the normalised cross product direction x up, row 2 completing the set. Used by Person::Swim, WALMoveSpline::Update and AttractionPersonInfo::MoveTowards |
| `0xa6c3a0` | PhysicsWorld::Integrate | One Verlet step for each of the world's particles (+0x40, count +0x48): new position (+0x10) = position + damping (+0x30) x (position - previous position (+0x20)) + force (+0x40) x the particle's +0x0 x the step factor in `0x13429F0`; the old position becomes the previous one. Called from PhysicsWorld::Step |
| `0xbfb0b0` | Vector::RotateAround | Rotates a vector about a unit axis by an angle (Rodrigues): the part along the axis kept, the perpendicular part turned by cos and sin of the angle. Used by Helicopter::Activate, RideCar::SetupModel and LaserShow::DrawBeams |
| `0xbfb1c0` | Vector::RotateAroundSinCos | The same rotation about a unit axis given the angle's sine and cosine. Used by RideCarSwingingPart::Update, AnimalCamera::Update and RideCar::UpdateEffects |
| `0x8a08e0` | Matrix::NearestAxis | Which of a matrix's three axes a vector lines up with best, by the largest dot product (the second axis's halved): 3 or 0 for the first (pointing with or against it), 4 or 1 for the second, 5 or 2 for the third. Used by Model::DebugReport and SceneryRenderer's instance collection |
| `0xa6b310` | DistanceConstraint::DistanceConstraint | Constructor of a constraint keeping two particles apart: keeps the two particles, the stiffness, the flag and the extra value given, their current distance (positions at +0x10) and its square as the rest length, and the sum of their +0 values (their inverse masses; 0 when the sum is 0). Used by Buggy::Setup and RigidBody::RigidBody |
| `0xc20680` | Shape::TurnAboutX | Turns a matrix about its x axis by an angle: rows 1 and 2 (+0x10, +0x20) rotated into each other by its cosine and sine. Called from Shape::RunCommands |
| `0xbedd30` | Matrix::TurnAboutX | The same turn of rows 1 and 2 by an angle. Used by WaterCannon::Update, RideCarSwingingPart::Update and AnimalCamera::Update |
| `0xc207a0` | Shape::TurnAboutZ | Turns a matrix about its z axis by an angle: rows 0 and 1 rotated into each other. Called from Shape::RunCommands |
| `0xbede50` | Matrix::TurnAboutZ | The same turn of rows 0 and 1 by an angle. Used by RideCarSwingingPart::Update |
| `0xa6ef50` | PhysicsWorld::AtRest | Whether every particle of a body (+0xC, count +0x10) is at rest: its speed (the change of position over the last step, times 60) squared is no more than the limit in `0x13C2A04`. Called from PhysicsWorld::CollideTerrain |
| `0xc20710` | Shape::TurnAboutY | Turns a matrix about its y axis by an angle: rows 0 and 2 rotated into each other. Called from Shape::RunCommands |
| `0xbeddc0` | Matrix::TurnAboutY | The same turn of rows 0 and 2 by an angle. Used by Helicopter::Activate, Waterfall::Update and RideCarSpinningPart::Update |
| `0xc208c0` | Matrix::TurnColumnsAboutY | Turns a matrix about the y axis from the other side: the x and z values of each row rotated into each other by the angle. Called from WALMoveSpline::Update |
| `0x8fed20` | Whirlpool::FindCell | Binary search of the whirlpool's cells (+0x10, count +0x18), sorted by row then column, for the given column and row: gives the cell and, when asked, its position; null when there is none. Called from Whirlpool::Update |
| `0x529200` | Angle::Wrap | Brings an angle into the range 0 to 2 pi by adding or taking away whole turns. Called from RideCarSpinningPart::Update |
| `0xbedf70` | Matrix::TurnColumnsAboutY | Turns a matrix about the y axis from the other side: the x and z values of each row rotated into each other by the angle (the same code as `0xc208c0`) |
| `0xa6b140` | Particle::Particle | Sets a particle up: its inverse mass, its position and previous position both at the given point, damping 0.99, +0x34 1 and no force. Called from RigidBody::RigidBody |
| `0x85d2f0` | Box::Transform | The axis-aligned box around a box once it is transformed by a matrix: starts both corners at the matrix's translation, then for each matrix entry adds the smaller of the two corner coordinates times it to the minimum and the larger to the maximum. Called from SceneryModel::FindLightsAndWater |
| `0xa61b90` | CameraAngles::Clamp | Keeps a camera's two tilt angles (+0x30, +0x34) between -1.5677 and 1.5677 (just under a quarter turn) and brings its turning angle (+0x38) into 0 to 2 pi by taking away whole turns |
| `0xb00c00` | Quaternion::ToMatrix | Turns a rotation quaternion (x, y, z, w) into the rotation part of a matrix (rows at 0, 4 and 8) |
| `0xcb5690` | Vector::TransformTwice | Transforms a point by one 4x4 matrix and the result by another (both with their translations) |
| `0x97abd0` | Vertices::Transform | Transforms n vertices (each a position and a normal, 6 floats) by a matrix: the position with the translation, the normal without |
| `0x856830` | Vertices::Transform2 | The same transform of positions and normals (the same code as `0x97ABD0`) |
| `0xc35e50` | Entries::Highest | The entry (12 bytes each, at +0xA0, count +0xB4) whose second value is the largest, or null when there are none |
| `0xceefb0` | Matrices::Get | Copies matrix n (64 bytes each, at +0x30, count +0x2C) into the one given, when n is in range |
| `0xbe7690` | Rect::Union | Grows a rectangle to take in another (the smaller left and top, the larger right and bottom) |
| `0xbe7520` | Rect::Intersect | Shrinks a rectangle to its overlap with another (the larger left and top, the smaller right and bottom) |
| `0x88cfd0` | Box::ClosestPoint | The point of a box (between its two corners) nearest the point given: each coordinate kept between the box's minimum and maximum |
| `0x481360` | Vector::TransformPoint | Transforms a point by a 4x4 matrix, translation included |
| `0xbb6050` | Vector::TransformPoint2 | The same point transform (another copy) |
| `0x9f8340` | Box::Overshoot2D | How far a point lies outside a box in x and y (0 inside) |
| `0xbe75e0` | Rect::UnionInto | Writes into the rectangle given (the second argument) the smallest one holding this one and the third (the smaller left and top, the larger right and bottom) |
| `0xbe7470` | Rect::IntersectInto | Writes into the rectangle given (the second argument) the overlap of this one and the third (the larger left and top, the smaller right and bottom) |
| `0xc73730` | Matrix::Transpose | Writes the transpose of a 4x4 matrix into another |
| `0xc73680` | Matrix::Transpose2 | The same 4x4 transpose (another copy) |
| `0x585870` | Matrix::FacingRotation | Makes the 4x4 matrix for one of four facings (quarter turns about the vertical axis: 1, 2 and 3; anything else is no turn) |
| `0x4812c0` | Vector::Rotate | Rotates a vector by a 4x4 matrix (no translation) into the one given |
| `0x58a620` | Vector::RotateInPlace | Rotates a vector by a 4x4 matrix in place (no translation) |
| `0xca35d0` | Matrix::RotateVector | Rotates a vector by this matrix (no translation) into the one given |
| `0xbc6140` | Segment::Middle | The point halfway between an object's two end points (+0x10C and +0x110, when it has them) |
| `0x9731c0` | Key::Compare | A sort comparison of two keys of nine values, value by value: -1, 0 or 1 |
| `0x4cc210` | Object::Matrix | Copies the matrix of the object's part (+0x41C, its matrix at +0x60), or the identity matrix when there is none |
| `0x4240b0` | Spline::SegmentPoints | The four control points of segment n (points of 0x24 bytes at +0x44): the start, the start plus its out-handle (+0x18), the end plus its in-handle (+0xC) and the end |
| `0x90c170` | Polygon::Area | The area of a polygon (points of 8 bytes at +0x24, count +0x28), by the shoelace sum |
| `0x7c3370` | Grid::CurvedOffsets | Turns a position on a grid (width +4, height +5) into two offsets: each from -0.5 to 0.5 across the grid, scaled, squared with its sign kept and scaled again |
| `0xc1e7e0` | Plane::BoxCornerDistance | Takes the box corner furthest along a plane's normal and gives how far along a direction it lies from the plane; 0 when the direction runs along the plane, otherwise 1 or -1 for the side |
| `0xb35000` | Grid::SpiralStep | Moves one step along a square spiral out from a cell of a grid (width +4, height +5) and gives whether the new cell is inside the grid |
| `0x727de0` | Quad::EdgeMiddles | Gives five points made from the averages of pairs of a quad's corner values (its edge middles and centre) |
| `0xbc61e0` | Segment::MiddleOfEnds | The point halfway between the last points of an object's two end lists (+0x10C and +0x110, points of 0x24 bytes) |
| `0x5be7f0` | Segment::Direction | The unit direction from the point of the part at +0x2C to that of the part at +0x28, or straight up (0, 0, 1) when they are the same |
| `0x856550` | Matrix::From4x3 | Expands a matrix of four rows of three into a 4x4 matrix (last column 0, 0, 0, 1) |
| `0x414270` | Wobble::Update | Sets three wobble values (+0x130 to +0x138) from sines of a phase (+0x150, reset to 5 when it runs out) with their amplitudes (+0x140 to +0x148) |
| `0x5d3850` | Ease::InOut | Eases a time over a total: slow at the start and the end (squared over the first and last stretch), straight in between; unchanged when the total is too short |
| `0x7c4690` | SinCos::Lookup | Sine and cosine of an angle from a table of a quarter turn (256 entries), with the signs and order set by the quadrant |
| `0x7c28a0` | Angle::TurnToward | Turns an angle (+0x110) toward the one given by at most a step (the difference brought into -pi to pi); gives 1 when it turned only part of the way |
| `0x7a2180` | Objects::AnyNear | Whether any object of the list `0x10EFB90` is closer than a set distance to the point given |
| `0xa6b190` | VerletPoint::Init | Sets up a moving point for Verlet steps: its position, its previous position (the position minus the velocity given times the time step `0xF8F980`), a damping of 0.99, a weight of 1 and no force |
| `0xa6eff0` | VerletPoints::AllSettled | Whether every point of a set is nearly still: gives 0 and the squared speed of the first point moving faster than the limit `0x13C2A04`, 1 when none is |
| `0x505e90` | Vector4::Transform | Transforms a four-value vector by a 4x4 matrix in place |
| `0xbc4ff0` | Grid::SnapIndex | The number of the nearest whole step for a value (steps of a given size); when the value is not within a tolerance of that step, gives the step it lies in instead and sets the flag |
| `0x8e30c0` | Bits::MergeLowest | Compares a cost with the best so far: a lower one replaces it and its four two-bit fields; an equal one keeps the lower of each two-bit field (register-passed arguments) |
| `0x4262f0` | Vector::SideDirection | The unit vector across the vector at +0x3F0 in the ground plane (up crossed with it), or its unnormalised value with 1 as the last part when that is zero |
| `0x481220` | Matrix::RotateVector2 | Rotates a vector by this matrix (no translation) into the one given (another copy of Matrix::RotateVector) |
| `0x47d7d0` | Matrix::InverseTransformPoint | Takes a point into a frame: subtracts the frame's position (+0x30) and multiplies by its rows (each with its own fourth value added) |
| `0xc219d0` | Lerp::Current | The current point of a blend between two points (+0x20 and +0x10) by its amount (+0x30), or the end point once the amount reaches 1 |
| `0x92dfd0` | Points::MidpointLength | The length of the point halfway between point n of two lists (0x24 bytes each) |
| `0xbd4c90` | Frame::Turn | Turns a frame's two axis pairs (+0x10/+0x14 and +0x20/+0x24) by an angle (cosine and sine) and updates the values that follow from them |
| `0x493930` | Grid::Contains | Whether a point falls in a cell of a grid (origin +0xC, +0x10; cell size +0x14, +0x18; width +4, height +5) |
| `0x7455f0` | Camera::SetFromHeight | Works out a camera distance from the height given and the field of view (+0x290, tan of half of it), sets the camera's offset (+0x260 to +0x268) from its direction and marks it changed (bits 2, and 1 when +0x2A7 is set) |
| `0x4141e0` | FrameTime::Smooth | Smooths the frame time: the first 100 frames are added up and their average used from then on, unless a cap is set (+0x11C), when the time is doubled and capped |
| `0xc20830` | Matrix::RotateX | Turns a matrix about the x axis by an angle (each row's second and third values) |
| `0xbedee0` | Matrix::RotateX2 | The same turn about the x axis (another copy) |
| `0xc1c2e0` | Triangle::FromBase | Three points of a triangle (an arrow head) from a base point, a direction and a side offset |
| `0x5bcee0` | Segment::MiddleOfEnds2 | The point halfway between the last points of the end lists of the parts at +0x2C and +0x28 (points of 0x24 bytes) |
| `0x50bd80` | Frame::Flip | Flips a frame: negates its value at +0x10, its direction (+0xD0 to +0xD8) and its other axis (+0xE0 to +0xE8), and toggles the flag at +0xC |
| `0x8ae2f0` | Curve::Falloff | A value past a limit, eased back by its kind: 0 half the overshoot, 1 the whole overshoot, 2 the overshoot squared, 3 squared over the limit |
| `0xb6bb70` | SphereQuery::Set | Sets up a sphere query: its centre, radius squared, a value, and the box around the sphere (centre minus and plus the radius) |
| `0xb5b740` | SphereQuery::Set2 | The same sphere query setup (another copy) |
| `0x7b2e30` | SphereQuery::Set3 | The same sphere query setup (another copy) |
| `0x488660` | SphereQuery::Set4 | The same sphere query setup (another copy) |
| `0x8e7130` | Bounds::Grow | Grows a box (+0x190 to +0x19C) to take in the one given and keeps the value given (+0x1A0) |
