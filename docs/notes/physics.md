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
| `0xc20950` | Matrix::RotateZ | Turns a matrix about the z axis by an angle (each row's first and second values) |
| `0xbee000` | Matrix::RotateZ2 | The same turn about the z axis (another copy) |
| `0xa5f530` | CameraAngles::Clamp2 | Keeps a camera's tilt (+0x20) between -1.5677 and 1.5677 and brings its turning angle (+0x24) into 0 to 2 pi |
| `0x875160` | UnitSquare::Rotate | Turns a position in a unit square (0 to 1) by a rotation (0 to 3) |
| `0x47bd90` | Sound::DistanceFade | How loud a sound is from its distance to the listener (`0x10F4620`): 1 inside its near distance (+0x80), 0 beyond its far one (+0x84), straight in between |
| `0xb1f750` | Matrix::Copy3x4 | Copies the first twelve values of a matrix |
| `0x94ca00` | Vector::NormaliseFast | Normalises a vector with the fast inverse square root (`0x5F3759DF` and one Newton step), leaving tiny ones as they are |
| `0x5d3970` | Ease::Out | Eases a time over a total: straight at first, slowing down (squared) over the last stretch; unchanged when the total is too short |
| `0x9e85f0` | Value::MoveToward | Moves a value toward a target by a rate times the time, stopping at it; gives 1 once it is there |
| `0xce4920` | Curve::LogBlend | A value between two points' values at a position, eased on a log scale (the factor log10 2) toward one end |
| `0x581530` | Frame::PickAxis | Copies one of a frame's axes: the first when +4 is set, otherwise the second |
| `0x80e090` | Box::ThinnestAxis | Which axis (0, 1 or 2) a box is thinnest along |
| `0x5bce60` | Segment::Middle2 | The point halfway between the points of the parts at +0x2C and +0x28 |
| `0x5bc0b0` | Colour::Blend | Blends four whole-number values toward another set by a fraction, rounded |
| `0xce47b0` | Curve::LinearBlend | A value between two points' values at a position, in a straight line |
| `0x7c32f0` | Average::AddCell | Adds a cell's position (+0x88, +0x89) to a running average (count +0x1D0, values +0x1D4 and +0x1D8) |
| `0xa637e0` | Object::PartMatrix | Copies the matrix of the object's part (+0xE0, its matrix at +0x60) |
| `0x577940` | Object::Bounds | Copies the object's bounds (four points from +0x220), with their fourth values set to the largest float |
| `0x4259f0` | Vector::LerpPoint | The point between two points by a fraction, with 1 as its fourth value |
| `0xcd0660` | SinTable::Sin | The sine of an angle from a table of a quarter turn (256 entries), with the sign by the quadrant |
| `0x96ea70` | Box::NearFarOffsets | For a direction, the byte offsets of a box's nearest and furthest corner values (minimum at 0, 4, 8; maximum at +0x10, +0x14, +0x18) |
| `0x80c340` | Box::NearFarOffsets2 | The same near and far corner offsets (another copy) |
| `0x4668d0` | Matrix::ScaleOffset2D | Makes a matrix that scales x and y by the values given and moves by minus the origin times the scale |
| `0xa17470` | Vector::DeadZone | Sets each part of a vector that lies between two small limits to 0 |
| `0xbc3f20` | Object::Scale | An object's scale value: its own (+0x100) when positive, the default (`0x10F2B28` or `0x10F2B2C`) times minus it when negative, or the default |
| `0xbc3eb0` | Object::Scale2 | The same as Object::Scale for its other value (+0xFC, defaults `0x10F2B20` and `0x10F2B24`) |
| `0xa6b2a0` | VerletPoint::SetVelocity | Sets a Verlet point's previous position (+0x20) to its position minus the velocity given times the time step `0xF8F954` |
| `0xa67ee0` | VerletPoint::Interpolated | The point between a Verlet point's previous and current positions by the global fraction `0x13429E0` |
| `0x4bb400` | Vector::TransformPointInPlace | Transforms a point by a 4x4 matrix in place, translation included |
| `0xce16b0` | Vector::Lerp | The point between two points by a fraction |
| `0x466a60` | Vector::Lerp2 | The same point between two points (another copy) |
| `0xa67e40` | Vector::LerpMarked | The point between two points by a fraction, with its fifth value set to not-a-number |
| `0x788140` | List::SortByDC | Sorts part of a list (+0x18C) of objects by their +0xDC value, largest first |
| `0xa88680` | Motion::DistanceAt | The distance covered by a time when speeding steadily from one speed to another over a set time, then going on at the second speed |
| `0x5d3900` | Ease::In | Eases a time over a total: slow at first (squared over the first stretch), straight after; unchanged when the total is too short |
| `0x8b5230` | Tracker::SetVelocity | Sets a velocity (+0xC to +0x14) from the move from the last position (+0x1C) to the one given times a rate, and counts an update (+0x2C) |
| `0xa6b240` | VerletPoint::Velocity | A Verlet point's velocity: its position minus its previous one, over the time step `0xF8F980` |
| `0x5bada0` | Damping::For | A damping amount for a speed: half of it when fast, a falling share when slower, a fixed share when stopped (scaled by `0x12F55CC`) |
| `0x4664e0` | Matrix::Ortho | Makes an orthographic projection: x and y scaled by 2 over the width and height, depth by one over (far - near) and moved by near |
| `0xb47a30` | Vector::LerpInto | Writes the point between two points by a fraction into the third one given |
| `0xb34fa0` | Progress::Advance2 | Adds time to a progress value (+0x138, up to 1); gives whether it is past a threshold |
| `0xcc4890` | Vector4::Add | Adds a four-value vector to this one |
| `0x912c20` | Spring::Constants | Works out a spring's two constants from its length (+4 minus +0xC), and a third when asked |
| `0xc5a860` | Vector::Lerp3 | The point between two points by a fraction (another copy) |
| `0xaf8e30` | Contact::Make | Fills a contact record: two points, a value and a pair |
| `0xaf75d0` | Contact::Make2 | The same contact record (another copy) |
| `0xa17520` | Particle::Launch | Starts a particle at a point raised by the height given, with its two values, and marks it active |
| `0x55fc10` | Vector::Quadrant | Which of four directions a flat vector mostly points (0 to 3), by its larger part and its sign (the vector is passed in a register) |
| `0xbc4f90` | Angle::Sector | Which sector an angle falls in: moved by an offset, wrapped to one turn and scaled, rounded down |
| `0xcc48f0` | Vector4::Scale | Multiplies a four-value vector by a number |
| `0x925d20` | Points::Within | Whether two points are closer than the distance given |
| `0x4412c0` | Angle::Octant | Which of eight directions an angle (+0x48 less +0x54) points in |
| `0xbba430` | PointSet::Walk | In outline: goes over lists of points (from `0xBB69E0`), measuring lengths (square roots of four-value differences) and passing the totals to `0xC2F650`; called from `0xBBBFE0` |
| `0x4dfe40` | Camera::Follow4DFE40 | In outline: points a camera along a direction (Matrix::LookAlongZSide), tilts it (Matrix::TurnAboutX), turns it toward its target, keeps it above the terrain height (`0x8F6760`) and adds a random shake when moving fast |
| `0x90a290` | Grid::Solve90A290 | In outline: a pass over a square grid (+0, +4) of values: sets up its rows and columns (`0x909DC0`, `0x90A0D0`, `0x909C10`, `0x909F10`) and then works out each cell (`0x9060F0`) |
| `0xcd9840` | Collision::SweepSphereTriangle | In outline: when a moving sphere first touches a triangle: solves the quadratic for the face (inside when both barycentric values are at least 0 and add to under 1), then the edges and corners, keeping the earliest time below the one given |
| `0xa64800` | Camera::UpdateA64800 | In outline: a camera's update from the object it follows (Object::PartMatrix): picks the main axis, blends toward the new view by a time factor (`0xDDD040`, `0xCC5C80`) and points along it (Matrix::LookAlongZSide) |
| `0xa6ca10` | Constraint::SolveA6CA10 | In outline: solves a constraint between points by its kind (+0: 1, ...), measuring the distances between them and moving them back to their lengths |
| `0x9f21d0` | Fireworks::HitTest9F21D0 | In outline: whether a ray passes through a firework part: works out its points (`0x9EEA50`), checks the ray against their spheres and then the triangles between them (`0x9F1FA0`), keeping the nearest |
| `0x9177b0` | Water::Flow9177B0 | In outline: a long run of maths on a water piece's values (+0 on), ending in a comparison; uses a translucent white (Colour::FromARGB 0x3FFFFFFF) |
| `0x92aca0` | Water::Split92ACA0 | In outline: splits a list of points by height: finds the points lower than their neighbours, then groups the rest into pieces (`0x926360`) |
| `0x8ef1e0` | Normals::Smooth8EF1E0 | In outline: goes over two lists of points (+0, +4) normalising their directions and then over their groups |
| `0x4284b0` | Curves::Setup4284B0 | In outline: fills a set of curves with fixed key points (`0x426790`: time, value) and three marks (`0x428450` at 9.0), with a different set when +0x669 is set |
| `0x906950` | Points::Merge906950 | In outline: merges two lists of points and groups (+0, and the one given) into one, keeping their order |
| `0x506ed0` | Car::Break506ED0 | In outline: breaks a model apart: checks its shapes (Model::CheckShapes), makes a rigid body for it (RigidBody::RigidBody) and sets its pieces flying with random speeds (VerletPoint::SetVelocity) |
| `0x92a260` | Water::Group92A260 | In outline: groups water points (sorted by height) into pieces, joining the ones close enough together and refreshing them (`0x417C20`) |
| `0x87a7f0` | Cuts::Split87A7F0 | In outline: cuts shapes along pairs of points (PairIds::Get): where a pair crosses the cut, adds a point at the crossing (`0x87A370` at the fraction between them) and sorts the pieces by side |
| `0x9619f0` | Particles::Scatter9619F0 | In outline: scatters points over the triangles of a shape (`0x960E90`, up to three tries each) with random positions and directions |
| `0x9feb90` | Particles::Bounce9FEB90 | In outline: keeps particles inside a box (+0x58): turns their speed around when they pass its sides (abs or minus abs) and checks the terrain under them |
| `0xbb9410` | PointSet::Walk2 | In outline: another pass like PointSet::Walk over a list of points, measuring lengths and passing them to `0xC2F650` |
| `0xcda7f0` | Ray::BoxSlabs | In outline: whether a ray meets a box, by the slab method: the entry and exit distances along each axis, giving 0 when it misses |
| `0xbb88a0` | PointSet::Walk3 | In outline: another pass like PointSet::Walk over points, measuring lengths and passing them to `0xC2F650` |
| `0xbc8aa0` | Object::AimBC8AA0 | In outline: works out an object's facing toward a point (`0xBC8500`), normalising the way to it, and applies it (`0xBC5D50`) |
| `0xa19e30` | Camera::MoveA19E30 | In outline: moves a camera toward a point: measures the way, slows down near the end (under a distance and above a limit, +0x10C) and eases its speed (`0xDDEBC0`) |
| `0xb02a00` | Animal::BlendPoseB02A00 | In outline: when not flagged (+0x2FC), blends an animal's pose toward the next (`0xB01420`, `0xC2F650`) and its positions (Vector::LerpInto), the blend kept between 0 and 1 |
| `0xc1f1b0` | Polygon::ClipC1F1B0 | In outline: clips a polygon against a plane: keeps the points on the inside and adds a point where an edge crosses (by the signs of their distances) |
| `0xa63d10` | Camera::ZoomA63D10 | In outline: moves a camera's two values (+0x34 toward +0x3C, +0x38 toward +0x40) by steps when they differ by more than a little (`0xA6B430`) |
| `0xc75800` | Effect::SetParamC75800 | In outline: sets one of an effect's settings by number (0 to 0xB and up) from the value given, kept between 0 and 1 for some |
| `0xcd0cf0` | Shapes::JoinCD0CF0 | In outline: works out the points joining two shapes (`0xCD0510`), the distance between them and a count of steps from it (`0xFC3484` per unit), eased (`0xDDEBC0`) |
| `0x95fce0` | Grid::Walk95FCE0 | In outline: steps through grid cells along a direction by the signs of its parts, counting the cells crossed on each axis |
| `0x7af0a0` | Camera::Orbit7AF0A0 | In outline: when not flagged (+0x54), points a camera along its target's direction (Matrix::LookAlongZSide) and turns it by its angles (Matrix::TurnAboutY, Matrix::TurnAboutX) |
| `0xbb9d80` | PointSet::Walk4 | In outline: another pass over a list of points like PointSet::Walk |
| `0x929bc0` | Water::Link929BC0 | In outline: links a water piece's points to the ones around it (`0x926FA0`) where they face the same way, and refreshes them (`0x417C20`) |
| `0x960810` | Points::InBox960810 | In outline: picks the points of a list (16 bytes each) that lie inside a box (+0x20 to +0x34) into a new list |
| `0xa17a40` | Camera::PathA17A40 | In outline: for mode 1 (+0x50), moves along a camera path (`0xA0F990`), measures its turn and turns the view (Vector::RotateAround) |
| `0x4b5350` | Camera::Follow4B5350 | In outline: another follow camera like Camera::Follow4DFE40: points it (Matrix::LookAlongZSide) and tilts it (Matrix::TurnAboutX) when its checks pass (`0x4B51E0`) |
| `0xa0f990` | CameraPath::Step | In outline: moves along a camera path by a distance: measures the way to the next point and its direction against the path's (+0x84, +0xA4), and moves on to the next point when past it |
| `0x906300` | Points::Merge906300 | In outline: merges a list of points into another, like Points::Merge906950 |
| `0xbc7940` | Object::HitTestBC7940 | In outline: whether a ray within a distance hits an object's parts by kind (2, and 6 to 0x19), using its two ends (`0xBC5AB0`, `0xBC5B40`) |
| `0x960e90` | Particles::PointOnShape | In outline: picks a point on a shape for a particle: checks it is inside its box (+0x20 to +0x30) and rounds it to a cell |
| `0x970040` | Points::Sort970040 | In outline: sorts a list of 24-byte points by their first value (from the top down) |
| `0x85d520` | Box::RayHit85D520 | In outline: whether a ray meets a box: inside already, or where it crosses each face plane, checking the point is within the face |
| `0x95ec30` | Water::Edge95EC30 | In outline: works out where water meets a height along an edge, by case (kind 3, `0x95DE00`) |
| `0xcd06d0` | Shapes::JoinCD06D0 | In outline: another shape join like Shapes::JoinCD0CF0: the distance between two points and a step count from it (`0xFC3484` per unit) |
| `0x9095b0` | Water::Colours9095B0 | In outline: another water colour pass like Water::Colours908C00 for pieces of two or more points |
| `0x8c4ac0` | Terrain::RayHeight8C4AC0 | In outline: walks a ray along the terrain heights until it drops below them, unless the direction is flat |
| `0xcc69a0` | Animation::SampleCC69A0 | In outline: samples an animation's tracks at a time: finds each key (Keys::Find2) and blends between keys |
| `0xccfab0` | Points::TransformCCFAB0 | In outline: transforms a list of points by a matrix, four at a time |
| `0x8133c0` | Boxes::Overlap8133C0 | In outline: finds the boxes of a list that overlap box n |
| `0xc1e970` | Polygon::SideC1E970 | In outline: which side of a plane a polygon's points lie on, by the signs of their distances |
| `0x9614c0` | Grid::Points9614C0 | In outline: goes over a grid of points (+0 by +4), joining each row's last point and normalising their directions |
| `0x907600` | Water::Spread907600 | In outline: spreads values along a water piece's points, keeping them at least 0 |
| `0x4180e0` | Polygon::Simplify4180E0 | In outline: removes points of a polygon (+0xC) that add nothing (`0x418090`), down to three |
| `0x44a750` | Effect::Bounds44A750 | In outline: grows a box to take in an effect's values when they are positive (+0, +4, +0x30) |
| `0x812110` | Regions::Find812110 | In outline: finds a region in a list (+0x48) and measures it (`0x80FF10`) |
| `0x8df1c0` | Water::Clamp8DF1C0 | In outline: keeps a list of water values within limits (1, and 1 less a margin) |
| `0xbb8430` | PointSet::StepBB8430 | In outline: one step of a point walk (like PointSet::Walk) when a length is given, passing it on (`0xC2F650`) |
| `0x938030` | Water::Nearest938030 | In outline: the nearest of a list of water points to a point, four at a time |
| `0x9084e0` | Water::Cell9084E0 | In outline: the water cell of a point within a grid of pieces (+0, edges checked) |
| `0xc617f0` | Particles::EmitC617F0 | In outline: when moving (+0xBC), launches up to four particles (`0xC21230`) |
| `0x8bbef0` | Ray::Triangle8BBEF0 | In outline: whether a ray meets a triangle: its distance and barycentric values (each at least 0, adding to under 1) |
| `0x7c6e70` | Camera::Blend7C6E70 | In outline: points a camera along a direction (Matrix::LookAlongZSide) and blends it by a time factor (`0xCC5C80`) when close |
| `0xbbd970` | Track::FrameBBD970 | In outline: a track point's frame: normalises its direction and points along it (Matrix::LookAlongZ, `0xBEE0C0`) |
| `0xbee8c0` | Vertices::TransformBEE8C0 | In outline: transforms a list of vertices by a matrix in a loop (used for track meshes) |
| `0xbc8110` | Object::HitBC8110 | In outline: a hit test on an object (Object::HitTestBC7940 within 0.5) and, when it hits, the facing (`0xBC5D50`) for kinds 1 and 3 |
| `0x8f3750` | Rect::Clip8F3750 | In outline: clips a point or span against a rectangle's sides (left, right, top, bottom) |
| `0x431450` | Effect::Bounds431450 | In outline: another effect box grow like Effect::Bounds44A750 |
| `0x820ba0` | SceneryBehaviour::Swing820BA0 | In outline: checks a behaviour's part (`0x81FAF0`) and works out a swing from a fixed angle (cosine of `0xF76318`) |
| `0x816cb0` | SceneryBehaviour::Pick816CB0 | In outline: with a reference (WithReference8155F0), picks a part at random (`0x81F4D0`, `0x8156C0`, `0x4751E0`) |
| `0x9ce6d0` | Particles::InRange9CE6D0 | In outline: whether a point is within an emitter's range (its radius against a limit and +0x134) |
| `0x8775c0` | Scenery::Angles8775C0 | In outline: loads scenery (Scenery::Load876810) and works out the two angles of a direction (arctangents) |
| `0x506a20` | Car::Ground506A20 | In outline: walks a ray across the terrain (Terrain::RayMarch8BD5B0) for a ride car (+0x44) and keeps the hit |
| `0xcc5c80` | Quaternion::Slerp | In outline: blends between two rotations by a fraction along the sphere (sines of the angle between them), falling back to a straight blend when they are nearly the same |
| `0xc5a1f0` | Quaternion::Slerp2 | In outline: the same rotation blend (another copy; used to blend bone poses) |
| `0x8ead40` | Water::Normals8EAD40 | In outline: works out a water piece's normals over the terrain, normalising each |
| `0x5f07b0` | Camera::Aim5F07B0 | In outline: when not flagged (+0x58), points a camera along a direction (Matrix::LookAlongZSide) when it is long enough |
| `0x8c0170` | Terrain::LowestPoint | In outline: the lowest of a list of terrain points (the first for one or two) |
| `0xa176b0` | Camera::AimA176B0 | In outline: normalises a direction for a camera's aim |
| `0xbb9090` | PointSet::StepBB9090 | In outline: another point walk step like PointSet::StepBB8430 |
| `0xbc5580` | Track::SnapBC5580 | In outline: snaps a track piece's height (Grid::SnapIndex with 0.1) by its kind (4 handled) |
| `0x820740` | SceneryBehaviour::Step820740 | In outline: a behaviour's step (`0x8156C0`) |
| `0xbb6ce0` | PointSet::StepBB6CE0 | In outline: another point walk step like PointSet::StepBB8430 |
| `0x415d80` | Grid::Wobble415D80 | In outline: goes over a grid's entries (+0x20) and updates its wobble (Wobble::Update) |
| `0x9eef30` | Fireworks::Pick9EEF30 | In outline: finds the firework part under the mouse (MainCamera::Get, `0xCD79C0`) |
| `0x81f120` | SceneryBehaviour::Parts81F120 | In outline: goes over a behaviour's parts (+0x6C) not flagged (+0x18) |
| `0x95e900` | Water::Edge95E900 | In outline: another water edge case like Water::Edge95EC30 |
| `0xbb7140` | PointSet::WalkBB7140 | In outline: another point walk like PointSet::Walk |
| `0xbbd650` | Track::SpeedBBD650 | In outline: a track point's speed value from its distance (square roots) within a limit (+0x1C) |
| `0xae1b40` | Graph::FindAE1B40 | In outline: finds the shown entry (+0x40, count +0x44) at a value |
| `0xc209e0` | Matrix::FromAxisAngle | In outline: makes a rotation from a turn vector (its length is the angle, using its cosine and sine), leaving tiny turns alone |
| `0x419100` | Grid::Normal419100 | In outline: normalises a direction for a grid point |
| `0xa6f0a0` | Points::StepA6F0A0 | In outline: moves a list of points on by a time, four at a time |
| `0xae1850` | Graph::FindAE1850 | In outline: when on (+0x60), finds a shown entry (+0x40, count +0x44) |
| `0xc21df0` | Skeleton::BuildC21DF0 | In outline: builds a bone's matrices (`0xBEE460` on +0x70) and goes over its children (+0x5C chain at +0x60) |
| `0x821ae0` | SceneryBehaviour::Range821AE0 | In outline: with a reference (WithReference8155F0), checks its counts (+0x4C, +0x6C) |
| `0x510990` | Car::Points510990 | In outline: goes over n car points |
| `0x8100e0` | Regions::Find8100E0 | In outline: finds a region for a point, searching the children (it calls itself) |
| `0x95e640` | Water::Edge95E640 | In outline: another water edge case, the fraction kept between 0 and 1 |
