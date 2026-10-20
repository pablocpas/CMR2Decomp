# Object collision projections

The sector-table loader builds each mesh collision record's eight corners,
rotates them by the combined record/placement matrix, and supplies an array
containing the resulting right, up and forward axes. These are inputs to
`Collision_BuildObjectPointBounds` (`0x004873f0`). The output pointer belongs
to `SectorObjectBounds::pProjection`.

The projection is the existing twelve-byte `ObjectPointBounds` record:

| Offset | Field | Producer and consumer evidence |
| --- | --- | --- |
| `0x00` | `directionX` | Normalized horizontal axis X shifted right by nine; expanded by 512 in the object-box builder. |
| `0x02` | `directionZ` | The corresponding Z component. |
| `0x04` | `top` | Updated when a corner is higher than the current packed upper bound. |
| `0x06` | `bottom` | Updated when a corner is lower than the current packed lower bound. |
| `0x08` | `halfWidth` | Maximum positive projection onto the first axis, shifted right by nine. |
| `0x0a` | `halfLength` | Maximum positive projection onto the perpendicular axis, shifted right by nine. |

The height comparisons expand the previously stored short before testing the
next corner. Keep that quantization order, signed shifts and short truncation;
replacing the loop with an unquantized min/max would change the original.
The first axis uses up instead of right when `abs(right.y) > 0xfd70`.

The original loop copies each corner's X, Y and Z separately before projecting
it. Expressing those copies through a `FixVector *` removes the biased word
cursor and retains the original compiler's traversal. Reading the right-axis Y
component after clearing the two output heights also follows the original
instruction order. These changes improve the producer's byte-audit score
without changing the record format.

The full measurement records a byte-audit improvement from 75.64% to 77.79%
and fuzzy similarity from 93.27% to 94.49% for this producer. Both object
shape setters remain byte-exact; no previously exact function is lost.

`Collision_BuildObjectBox` (`0x00487c40`, previously named as sprite
orientation) expands the six fields into a `CollisionBox`, adds the supplied
world position and constructs its four lower-height points. It sets `pVertex`
to that position, clears `pArray`, and marks the box built. It preserves the
secondary flag and points, and leaves an already built box untouched.

Entries with no projection use `Collision_SetObjectSphere` (`0x00487e00`).
It stores the position, bounds radius and first mesh collision record's height;
there is no material access. `g_collisionRecordHeight` (`0x00591494`) names
that height store, for which no reader has been found.

The loader's list distribution uses a `SectorObjectEntry *` cursor instead
of reinterpreting it as a `SectorObjectBounds *`. The two records happen to
have equal sizes on x86; their matching strides do not make their types
interchangeable.

Validation exercises the original and rebuilt sector-table lifecycle plus
forty direct producer/consumer cases at both signs of the axis threshold,
height quantization boundaries, signed-short overflow and cached-box reuse.
Whole-heap guards check the packed record, box fields, previous points and
input geometry. The native ASan/UBSan fixture covers the actual loader,
projection producer and object-box consumer with native pointers.
