# Car object contacts and pointer-free route nodes

Baseline: `43a9b83`. Two independent migration families cover eleven source/header
files through 31 reproducible operations. Every producer, repeated prototype and
consumer of the route node family uses `RallyRouteNode *`. Contact producers and
consumers use numeric members of the runtime Car instead of its Win32 offsets.
All touched/new layouts have centralized Win32 checks in LayoutChecks.cpp.

The full build preserves **2924 exact functions out of 3364**, with no s/fz/x
regression anywhere. `CarPhysics_UpdateWheelSlipAndVelocityDamping` improves
s **80.3922% → 88.2353%**, keeping fz **99.0196%**. The deformation-hull correction
remains exact. Pointer/integer diagnostics decrease **208 → 204** across the
compatibility projection: all four remaining conversions in
`Race_UpdateCarRouteAndWrongWayState` disappear. Of the original 594 diagnostics,
390 have been removed; 204 remain. Raw accesses decrease **1122 → 1092**,
fixed sizes **75 → 74**, and hexadecimal multipliers **2200 → 2194**.
These inventories are syntactic counts, not proof that every multiplier is a
record stride. Unknown identifier counts remain 10232 occurrences / 1251 unique
spellings. The new numeric views name proven fields while retaining unknown
regions and old aliases.

## Runtime Car contact fields

Car contains native pointers before these members; hard-coded Win32 positions
read the wrong fields when pointers grow. The recovered views keep the Win32
Car size 0xc24 and preserve both old buffer aliases and overlapping corner data.

| Win32 offset | Field / size | Evidence |
| --- | --- | --- |
| 0xad6 | `short contactSurfaces[2][5]`, 20 bytes | Obstacle response compares previous surface IDs in row 0, appends at most five pending IDs to row 1; object traversal copies pending IDs back to row 0. |
| 0xaea | 20 unknown bytes | Unused remainder of the original 40-byte alias; no invented meaning. |
| 0xb35 | Nine unknown bytes | Existing ground-normal consumers keep the original buffer view. |
| 0xb3e | `char previousContactCount` | Bounds the scan of previous surface IDs, with the original signed-byte semantics. |
| 0xb3f | `char pendingContactCount` | Cleared before traversal, incremented by obstacle response while below five, then copied to previous count. |
| 0xb40 | Two unknown bytes | Remaining bytes of the old 13-byte alias. |
| 0xbbc | `int objectContactState[8]`, 32 bytes | Cleared before sector-object traversal. First four alias `cornerOnGround[4..7]`; slip/damping reads and updates them with `field_0x8fc[0..3]`. Last four alias the unresolved `field_0xbcc`. |

The traversal also reads sector/neighbours, collision radius, position, body
axes, corners and the c18/c20 control fields through existing members. The first
two neighbour shorts are copied together with `memcpy` and `sizeof`, preserving
the original word-copy/store order without a raw integer view. The slip loop
indexes existing corner and normal vectors, contact state and height-offset
arrays. Its original arithmetic, comparison order and velocity damping remain.

## Pointer-free .cat file nodes

`RallyData_LoadSelectedRouteNodes` divides the loaded file size by 44 and never
relocates a field inside a node. The disk record is therefore also a valid native
record, with the same 44-byte size on x86 and x64. This differs from the
pointer-bearing StageObject/Mesh records documented in the previous batch.

| Offset | Field | Evidence |
| --- | --- | --- |
| 0x00 | `FixVector position` | Direction calculation reads x/z, and ground-position getter returns x/0/z. No new use of stored y is introduced. |
| 0x0c | Twelve unknown bytes | No semantics inferred. |
| 0x18 | `BYTE flags` | Bit 0 marks a checkpoint; loader chooses the final marked node, checkpoint builder collects them. Bit 1 disallows reset at the node. |
| 0x19 | `char callId` | Route/call traversal tests signed -1 as the sentinel and passes other IDs as BYTE to the callout queue. |
| 0x1a | Two unknown bytes | No semantics inferred. |
| 0x1c / 0x20 | `int backwardNearLimit`, `backwardFarLimit` | Camera distance override reads this pair while travelling backwards. |
| 0x24 / 0x28 | `int forwardNearLimit`, `forwardFarLimit` | Camera distance override reads this pair while travelling forwards. |

The complete family includes loading, available-record getter, ground-position
getter, direction cache, camera distance override, reset placement, checkpoint
splits and wrong-way/call-node traversal. Native node pointers replace the two
integer pointer locals in the latter function. Availability/null behavior,
looped-route handling, call sentinel signedness, and original reset behavior
(including when it refreshes its node pointer) are unchanged.

A direct typed-node cursor initially reduced the loader's s/fz scores. That
candidate was rejected. The final loader takes the address of `node.flags` and
walks its original byte cursor by `sizeof(RallyRouteNode)`. This pointer-free
disk-field cursor preserves the original generated code while eliminating both
literal field offsets and hard-coded record strides. No assembly, pragmas or
volatile additions were used.

## Validation

- Full MSVC6 build, complete measurement, fast comparison preparation, and
  changed-TU gate pass. Every function retains its exactness and s/fz metrics,
  apart from the slip function's s improvement.
- `differential_route_nodes.py`: 576 full original/rebuilt scenarios. Actual
  loader, reset, wrong-way/call traversal, checkpoint and direction/position
  bodies run with resource/scene/audio providers controlled at their boundaries.
  Complete guarded heap, immutable node records, caches, race/call/checkpoint
  globals, arguments/provider order and x86 calling ABI agree. A mutation that
  reads the wrong position component is rejected. Direction tests initialize
  the original fixed-point root table and exercise nonzero directions.
- Existing route-progress and camera-clip harnesses also cover their changed
  callees, including the camera provider that replaces the graphics object.
- `check_64bit_route_nodes.py`: 576 native scenarios compared with original x86,
  using eight actual bodies and production layouts. Full direction cache,
  race/call/checkpoint state, reset fields and immutable file nodes agree;
  native pointers above 4 GiB pass ASan/UBSan and casts are compilation errors.
- `check_64bit_sector_object_tables.py` now also compiles the actual Car object
  traversal and slip bodies: 96 native table lifecycles, 768 contact traversals
  and 288000 slip cases. An independent fixed-point slip model compares every
  byte of Car; contact tests guard all other Car fields and preserve full native
  member addresses. The original/rebuilt sector lifecycle harness retains its
  288 scenarios, including traversal and contact-provider traces.
- Compatibility projection retains **367 pre-existing compiler errors**, with
  no new or removed errors. All diagnostics are retained in the accompanying
  error log; this does not establish a complete native game build.

The registered full suite passes: **128 harnesses, zero failures**, with three
workers. All measured source/executable/PDB hashes and FUNCTION/GLOBAL
annotations are unchanged after validation; the validation report records this. Matching deltas, recipe replay hashes, native
inventory and readability inventories accompany this document.

## Remaining work

The original disk/native split for pointer-bearing stage placements, meshes and
collision records is still required in the port. The per-car collision-box
owner `g_unk0x00590ed0` remains a byte array and needs its complete producer/consumer
family migrated; the new native contact fixture controls response leaves and
does not establish that owner's native layout. Camera graphics offsets and the
collision-box integer marker also remain separate families. None is hidden by
these tests. Co-driver texture/data outputs and other remaining pointer/offset
families continue under the active goal.
