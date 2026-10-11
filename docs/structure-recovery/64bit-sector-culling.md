# Native sector culling preserves the terrain bounds

Baseline: `9be32b5`. Validation excludes the unrelated weather-texture worktree
edit. Original function/global annotations and expression order are preserved.

`Sector_CullGridAroundViewNode` reads the four x/z bounding corners to select
visible terrain cells. It then stores distance and two state words at Win32
offsets +0x7c/+0x80/+0x84. Native pointer growth moves the bounds themselves
over those offsets: the stores overwrite three corner coordinates. Subsequent
frames cull against the corrupted rectangle, while the immediate neighboring
cells remain selected by the explicit neighbor conditions.

The reproduced failure is consistent with the report that terrain only appears
when the car is almost on top of it. This batch fixes the demonstrated memory
corruption; it does not claim visual acceptance on every stage or camera.

| Sector region | Win32 | Native x64 | Evidence |
| --- | ---: | ---: | --- |
| Record size | 0x88 | 0xa0 | Four runtime pointers and fixed scalar geometry |
| `bounds[4][2]` | 0x5c | 0x70 | `Sector_ComputeBounds` produces rectangles, culling tests all four corners |
| `viewDistanceSquared` | 0x7c | 0x90 | Sum of squared x/z displacement from the culling view |
| `field_0x80` | 0x80 | 0x94 | Cleared when selected; no further semantic evidence |
| `field_0x84` | 0x84 | 0x98 | Set to one when selected; no further semantic evidence |

The opaque twelve-byte tail becomes three integer fields. All three stores
use those fields, with central Win32 offsets and the existing size check.
`g_unk0x006ed5f0` becomes `g_visibleSectorIndices`: culling appends unsigned-short
cell indices, and the ground/shadow/static-object/node/reflection draw passes
consume the same list. All repeated declarations and consumers are updated.

The original 0x88 disk record remains unchanged. OpenCMR2's existing native
stage loader unpacks it into the wider record; regenerating the disk layout
does not change the generated scalar runs or disk sizes.

## Validation

- Full MSVC6 build, measure and changed-TU gate pass. All 3364 functions retain
  their scores, including all 2924 exact functions.
- The existing original/rebuilt sector harness covers 1950 cases of detail
  distance, view orientation, near/far radii, boundary cells and shifted grids.
  It now independently models distance/state writes and checks the complete
  64 KiB heap, including every bounds rectangle, pointer and guard region.
- Native sanitizer check uses production records and the actual culling,
  position lookup, triangle and matrix helpers. It covers 384 camera/grid/radius
  combinations, three passes each, with pointers above 4 GiB. Complete records,
  masks and lists agree with the model and the bounds remain unchanged.
  ASan/UBSan pass. Restoring the old offsets fails the bounds assertion.
- The complete registered differential suite passes: 131 harnesses.
- OpenCMR2 passes all 22 CTests. Its new `sector_culling` test calls the compiled
  game functions with the actual startup math tables, covers the same 384
  repeated-culling cases and checks complete records. It uses a large allocation
  to exercise high pointers even when the Linux executable uses a low brk heap.
  The six-cadence original physics comparison and runtime host/ghost/replay
  scenarios continue to pass.

This batch removes three raw accesses (760 -> 757), twelve unknown occurrences
(9900 -> 9888) and one unknown spelling. Two tail words remain conservatively
named by offset because their complete meanings are not recovered. That
increases the syntactic count of unnamed field declarations by one while
replacing an opaque array with its observed scalar representation.

The reported Italy light flash and exact post-finish replay menu loop still
need reproduction. The lighting-file disk/native records and the stage-object
matrix access in mesh cloning remain identified candidates, not fixes included
in this batch.
