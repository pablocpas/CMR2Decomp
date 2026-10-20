# Car collision box construction

Recovered from the original functions' callers, field stores and geometry use:

| Original address | Recovered name | Evidence |
| --- | --- | --- |
| `0x00486c30` | `Collision_BuildCarBox` | Every caller passes a car's collision box, body axes, position and eight world corners. Computes horizontal axes and extents, stores the geometry pointers and sets `built`. |
| `0x00486fc0` | `Collision_BuildBoxFootprint` | Called only by the box builder. Scales the box axes by their half extents and writes four world footprint points at the box's top height. |
| `0x0048ca40` | `Collision_GetCarBox` | Returns `&g_carCollisionBoxes[index]`; sector-edge collision assigns the result to `g_collisionFace`. |
| `0x00590ed0` | `g_carCollisionBoxes` | Eight `CollisionBox` records, selected by car order or `Car::index`. Shared by car-pair and sector collision code. |

`Collision_BuildCarBox`'s parameters are a `CollisionBox *`, the car's nine
body-axis words, its `FixVector *` world position and its eight world corners.
The axes come from `Car::right`, followed by `up` and `forward`. If the absolute
right-axis Y component exceeds `0xfd70`, the builder starts with the up axis;
otherwise it starts with right. It clears Y and normalizes that axis, then
constructs the perpendicular horizontal axis. A nonzero `built` returns
before reading any geometry arguments.

The former word-index accesses map to the existing checked `CollisionBox`
layout: half extents at `0x00/0x04`, top/bottom at `0x08/0x0c`, axes at
`0x10/0x1c`, `built` at `0x28`, points at `0x30`, and native geometry pointers
at `0x90/0x94` in the original x86 layout. Construction preserves the
secondary-point flag at `0x2c` and the four previous-frame points at `0x60`.

The footprint helper uses the signed combinations `A-B`, `A+B`, `-A-B`,
`-A+B`, where each axis is scaled by its half extent. It translates X/Z by
the car's position and sets every output Y to `top`. The four later points
and all geometry pointers stay untouched. Locals `widthAxis`, `lengthAxis`,
`offset`, `alongA`, `alongB` and `absRightY` describe those observed operations.

`differential_collision_boxes.py` executes both complete bodies and the
getter against the original executable. It covers axis selection at both
signs of the threshold, zero axes, cached boxes with unreadable geometry,
signed and overflowing footprint arithmetic, whole-heap guards, secondary
points, input geometry and the stdcall register/stack contract. The footprint
also has an independent integer model. Existing fixed-point helpers execute
without interception.
