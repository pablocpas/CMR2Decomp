# Runtime sector entries, bounds and moving-object storage

Baseline: `3eb2b4f`. The reviewed migration covers the runtime tables built by
`RallyData_RebuildSectorStaticObjectTables`, their mesh and collision consumers,
and the moving-object owner shared by StageObjects and FixedPoint. The final
recipe contains 89 operations across ten source/header files, in two connected
families. The generator applied both families together; the final recipe replay
reproduces every resulting file, including reviewed follow-ups to the shared
Collision2D declaration and native index width.

Pointer/integer diagnostics decrease **257 → 208** across nine functions;
raw accesses decrease **1201 → 1122**. Of the original 594 conversions, 386
have been removed. Unknown identifier occurrences decrease 10301 → 10232;
unique spellings decrease 1259 → 1251. Fixed sizes decrease 78 → 75 and hex
multipliers 2203 → 2200. These are reproducible syntactic inventories, not proof
that all remaining numeric multipliers represent record strides.

The full build retains **2924 byte-exact functions out of 3364**. Every other
function keeps its s/fz/x metrics, except two improvements:

| Function | s before → after | fz before → after |
| --- | --- | --- |
| `RallyData_RebuildSectorStaticObjectTables` | 61.9817% → 62.3596% | 76.4582% → 76.5449% |
| `Collision_TestCarAgainstSectorObjects` | 78.5088% → 78.6813% | 91.2281% → 91.4286% |

The previously corrected `CarDamage_BuildRelativeVelocityHull` remains exact.

## Runtime allocation and evidence

These records are constructed after stage relocation, not read from disk.
Allocation sizes and pointer-table strides now follow their native types.

| Record / Win32 layout | Fields | Evidence |
| --- | --- | --- |
| `SectorObjectEntry`, 8 bytes | +0 `StageObject *pObject`; +4 `SectorObjectBounds *pBounds` | The constructor stores the placement and a separate bounds allocation. Sector list getters return the address of the span pointer; traversal dereferences that pointer and advances by entries. |
| `SectorObjectBounds`, 8 bytes | +0 `int radius`; +4 `ObjectPointBounds *pProjection` | Box records receive half the rotated diagonal length and a projection pointer; spherical records receive the width/radius value and null. |
| `ObjectPointBounds`, 12 bytes | +0/+2 direction x/z; +4/+6 top/bottom; +8/+a half-width/half-length, all `short` | `Collision_BuildObjectPointBounds` writes these six values from eight rotated points; the sprite box consumer scales them back by 512. |
| `MovingObject`, 0x128 bytes | +0 `SectorObjectEntry *pEntry`; +4 scene node; +8/+48/+88 matrices; +c8 matrix pointer; +118 sector index | The queue stores the runtime entry and caller's sector; initialization follows its placement and collision records. The physics TU uses the same state and owner. |
| `MovingObjects`, 0x3448 bytes | 40 entries; +2e40 mesh count; +2e41 0x103 unknown bytes; +2f44 320 car distances; +3444 active count | Loader constructs forty scene nodes; queue and eviction update the active prefix and per-car distance table. |

All fields of the moved/new runtime records have centralized Win32 sizeof and
offset checks. Both duplicate definitions of MovingObject/MovingObjects now
come from MovingObject.h. Native entries and bounds have 16-byte strides;
MovingObject's fields and the owner arrays grow with their actual pointers.

The shared object collision box at 0x5915f8 is an existing `CollisionBox`, with
native centre/corner pointers, rather than an int array. Its repeated declaration
and corner view in Collision2D agree with the producer and both consumers.
The integer cache marker in its pointer-free +0x28 padding retains its original
full-word access; naming/removing that view is still pending.

Entry flag helpers use native `ptrdiff_t` indexes. They calculate an unsigned
native-width address distance using `UINT_PTR` before dividing by the actual
entry size. This preserves Win32's unsigned distance division, avoids subtracting
pointers belonging to different arrays, and rejects identities 64 GiB away
without narrowing the quotient to a 32-bit index. These integers describe a
range test; no pointer is stored in a 32-bit integer.

The constructor reuses one cursor for entry span publication and bounds filling.
Explicit pointer views preserve this original lifetime and assignment order.
Replacing that lifetime with a fresh bounds expression initially reduced fz;
the rejected candidate did not enter the final sources. Advancing the collision
point cursor as FixVector* also reduced matching; its existing integer view of
the pointer-free vector span is retained, with a typed input pointer and a
member-based starting address. No inline assembly, pragma or volatile was added.

## Names supported by producers and consumers

| Original global | Recovered name | Evidence |
| --- | --- | --- |
| 0x58c94c | `g_movingSectorEntries` | The constructor's flagged-placement branch fills list B; moving-object initialization resolves an entry in this table. |
| 0x58c948 | `g_staticSectorEntries` | The unflagged-placement branch fills list A. |
| 0x58ca70 | `g_sectorObjectBounds` | Separate allocation of radius/projection pairs referenced by every entry. |
| 0x58c944 | `g_objectPointBounds` | Six-short projection records filled by the bounds builder. |
| 0x58c938 | `g_sectorEntryCarFlags` | Byte masks queried and updated with `1 << car`. |
| 0x58c958 | `g_sectorEntryRaised` | ApplyPlayerFlags toggles the cached state with placement y/visibility changes. |
| 0x58c930 | `g_movingObjectTypes` | Integer classifications calculated from cloned mesh extents and consumed by moving initialization. |
| 0x5915f8 | `g_sectorObjectCollisionBox` | Constructed sprite-oriented collision box, consumed by turned-object contact and the shared corner view. |

`g_movingObjectMeshes` is a runtime `Mesh **`; its parallel clone index table is
`BYTE *`, and its classification table is `int *`. Allocations use the correct
pointer or integer element size. The loader's mesh local and the classifier's
mesh view are real Mesh pointers, and vertex count/data come from named members.

## Disk records retain their original Win32 representations

The earlier stage-object document misclassified variant entries as disk data.
The allocation and publication evidence above corrects that classification.
The placements and mesh headers referenced by those entries are disk-backed.

| Disk record | Win32 layout recovered here | Relocation evidence |
| --- | --- | --- |
| `StageObject`, 0xa0 bytes | +0 position; +c mesh pointer; +10 flags; +14 visibility; +18 matrix; +58 0x40 unknown bytes; +98 next pointer; +9c light level | Loader 0x4b93c0 relocates +c and +98 in place. Numeric views overlay the existing buffers without changing their Win32 layout. |
| `Mesh`, 0x120 bytes | +104 eight unknown bytes; +10c collision-record pointer; +110 size units | The stage loader relocates the collision chain against its file record array. Existing vertex/triangle pointers are also relocated. |
| `MeshCollisionRecord`, 0x5c bytes | +0 type; +4 local matrix; +44 width; +48 length; +4c height; +50 eight unknown bytes; +58 next pointer | The loader rebases +58; the constructor walks the chain and builds corners from the three numeric dimensions. |

These types carry ON-DISK notes. Their original sizes/offsets remain checked in
MSVC6. The native fixtures explicitly decode the original disk input into
separate native allocations. **The game's in-place disk loader still needs the
port's disk/native separation.** It must not interpret those original file
strides as sizeof(native pointer-bearing records).

Unknown regions remain unnamed: placement +15 padding and +58 remainder,
mesh +104, collision-record +1 padding and +50 remainder, and the moving
state's unresolved numeric fields/owner padding.

## Validation and limits

- Full MSVC6 build, full measurement and fast comparison preparation pass.
  No s/fz/x regression across all 3364 functions; 2924 remain exact.
- Changed-TU gate passes for all fourteen affected TUs.
- `differential_sector_object_tables.py`: 288 original/rebuilt scenarios,
  including construction, multiple records sharing a placement, empty lists,
  clone failure, flags, list getters, moving setup, all forty slots and eviction,
  matrix publication, cached/uncached sprite boxes and sector collision traversal.
  Complete guarded heap, owner/global outputs, provider order and stdcall ABI
  agree. Invalid flag identities are rejected; mark-all executes its real body.
- `check_64bit_sector_object_tables.py`: 96 native lifecycles compared with
  original x86 records, using seventeen actual changed bodies and production
  headers. Native allocation sizes, pointers above 4 GiB, complete canonical
  tables/owner/node state and immutable mesh/record/car inputs pass ASan/UBSan.
  A synthetic identity 64 GiB away is rejected without accessing it.
- Allocation, mesh clone ownership, scene insertion/removal, game state and
  final contact responses are controlled providers. Matrix and projection math,
  table construction and the migrated lifecycle bodies execute directly.
- The native fixture excludes `Collision_TestCarAgainstSectorObjects`: it still
  has raw Car field offsets. Its migrated entry traversal is covered against
  original x86; migrating those remaining Car accesses is a subsequent family.
- The compatibility projection decreases native cast diagnostics to 208 across
  111 functions. It retains all 367 existing compiler errors, with no additions
  or removals. This is an inventory/projection, not a complete native game build.

The complete registered suite passes: **127 harnesses, zero failures**.
The measured source/executable/PDB hashes are unchanged after validation.
The final stable status is recorded in the accompanying suite log and migration report. The recipe replay, matching delta,
readability hashes and unfiltered projection error log accompany this document.
