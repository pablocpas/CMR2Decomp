# Collision face point groups

The original classifier at `0x00490720` reads four primary vectors from
`CollisionBox + 0x30` and, conditionally, four corresponding secondary vectors
from `+0x60`. A byte cursor advances by twelve while a separate counter
publishes indices zero through three. The groups occupy the existing eight-point
span; the pointers that follow it are unchanged.

`CollisionBox::primaryPoints[4]` and `secondaryPoints[4]` name these groups
alongside the existing complete `points[8]` and word views. Win32 layout checks
and native x64 assertions cover both offsets and group sizes. The car-box frame
update copies the primary group into the secondary group and retains the
`built` flag in `hasSecondaryVertices`, establishing the previous-frame use
of the secondary car points. Object-box builders populate only the primary
group. The flag still controls whether the classifier may consult the second
group.

`Collision_ClassifyFaceVertices` now reads the vector components directly
through these fields and uses its published vertex index to address them.
This removes both byte casts, the numeric structure offsets and the redundant
byte cursor. Direct component accesses reproduce the original compiler's
addressing better than taking intermediate point pointers: the local byte-audit
score improves from 90.54% to 97.03%. Candidate indices refer to primary points
even when the corresponding secondary point affects acceptance.

Full measurement preserves all 2924 exact functions and every other score.
The classifier's fuzzy score rises from 93.78% to 97.57%; the weighted global
fuzzy score rises from 94.44% to 94.45%, with zero data issues.

The classifier first counts signed normal distances for all four primary
vertices. Its tangent projections retain vertices between the edge's endpoints.
On the selected side, an enabled secondary group rejects a point only when
the secondary point lies farther from the plane. A zero-distance primary point
remains eligible. `useParallelSide`, the deciding wrapper's parameter, selects
`VehiclePhysics_SelectParallelCollisionSide` instead of
`VehiclePhysics_TestBehindFaceCollision`.

Two neighbouring names are corrected from their observed data accesses:

| Address | Name | Evidence |
| --- | --- | --- |
| `0x005914d8` | `g_collisionObjectHasBox` | Assigned only `pEntry->pBounds->pProjection != 0`; selects box versus sphere contact data and spin scaling. It is never incremented. |
| `0x00487e50` | `Collision_ApplyObjectOverlapDamping` | Tests the object sphere centre against the expanded car box, then calls `CarPhysics_UpdateWheelSlipAndVelocityDamping`. It accesses no lighting or rendering state. |

`Collision_ResolveStaticObstacleContact` reads the placement's existing `flags`
member rather than casting its opaque byte view. Its parameters identify the
car, object reference, unused position argument and stored contact ID.

The differential face-decision harness now also executes the real classifier.
Its 288 direct cases cover clean and refreshed directions, both selected sides,
secondary points that approach or recede, zero distances, segment endpoints and
signed coordinate wrap. An independent plane/segment model checks distances,
candidate order and counts. Global write-width checks, poisoned candidate tails,
whole-heap guards and stdcall register/stack checks protect unrelated state.
