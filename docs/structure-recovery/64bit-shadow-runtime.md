# Native shadow buffers and recovered dirty state

Baseline: `9e7b67a`. Validation runs in an isolated checkout, excluding the
pre-existing, unrelated `RallyData.cpp` worktree edit. The port snapshot includes
its native stage/fin.dat/AI loaders and canonical x64 physics capture.

## Pointer family

`Scene_AddShadowCaster` allocates a runtime `ShadowCaster`, its `ShadowPart`
array and six buffers per part. Neither record comes from disk. Their shared
declarations now live in `ShadowPart.h`, with nineteen central Win32 layout
checks in `LayoutChecks.cpp`.

| Record / buffer | Win32 | x64 | Evidence |
| --- | ---: | ---: | --- |
| `ShadowCaster` | 0x14 | 0x20 | Native node and part-array pointers, allocated with `sizeof(ShadowCaster)` |
| `ShadowPart` | 0x58 | 0x78 | Eight native pointers after its twelve-float light basis |
| Triangle work entry | 12 | 24 | `Scene_EmitProjectedShadowMesh` stores three vertex pointers per face; `Scene_ProjectCasterIntoLightZone` reads those pointers |

The triangle work buffer is `BYTE **`, and its allocation uses three native
pointer elements per triangle. Its producer keeps a full-width address
difference and accounts for the difference between integer-index and pointer
slot widths. Without both corrections, x64 reserves half the required buffer
and overlapping writes corrupt the vertex references.

The projector's byte cursor now advances by `sizeof(ShadowPart)`, and its
translation reads `pNode->worldF[12..14]`. An indexed part walk was tested but
reduced s/fz in MSVC6; the retained byte cursor has a `64BIT` explanation and
follows the native record size. An indexed triangle destination also reduced
the emitter's scores; the retained cursor explicitly scales native pointer
slots and is checked against original x86 with high native addresses.

All allocation, emission, projection and cleanup consumers use the same
triangle-buffer declaration.

## Names supported by producers and consumers

| Original field / symbol | Recovered name | Evidence |
| --- | --- | --- |
| `ShadowPart::field_0x54` | `verticesDirty` | Init / mark-dirty set it; `Scene_ProjectDirtyShadowParts` rebuilds the vertex projection buffers and clears it |
| `ShadowCaster::field_0xc` | `projectionDirty` | Mark-dirty sets it; dirty-part projection consumes and clears it |
| `ShadowCaster::field_0x10` | `meshDirty` | Node shadow emission sets it; dirty-part projection rebuilds the emitted mesh and clears it |
| 0x006e0354 | `g_shadowVertexRemap` | Unsigned-short source-to-emitted vertex mapping, initialized to 0xffff and reused for shared vertices |

Names describe recovered behavior, rather than claiming to be the original
developers' spellings. Numeric vertex work buffers keep their existing names.

## Original shared-vertex behavior restored

The old emitter reused a vertex at `pVtx + s * 0x30`, where `pVtx` had already
advanced over newly emitted vertices. Original x86 instead indexes from the
start of the output buffer. For faces `(0,1,2)` and `(0,2,4)`, the original
references output vertices `(0,1,2,0,2,3)`.

The corrected emitter uses `pPart->pVertices + s * 0x30`. This fixes a
decompilation transcription error, preserves the original behavior and
improves s **71.5152% -> 72.1774%**, fz **77.1717% -> 77.4194%**.

## Validation

- Full MSVC6 build, measure and changed-TU gate pass. All **3364** functions
  are compared with the baseline: **2924** remain exact, no s/fz/x regression.
- `differential_shadow_buffers.py`: **80** complete registration, emission
  and projection scenarios against original x86. One through four parts,
  front/behind/mixed/shared/empty faces, signed/clamped strengths, complete
  work buffers, output batches, immutable input, allocation guards and stack
  cleanup are checked. A separate vertex-remap model verifies shared vertices.
- `check_64bit_shadow_buffers.py`: the same **80** registration/emission
  scenarios with actual bodies and production ShadowPart/SceneNode/Mesh
  declarations. Pointers above 4 GiB, eight-byte triangle slots, native
  allocation sizes, complete work buffers and guards agree with original x86.
  ASan/UBSan pass. Negative controls restore the half-sized allocation, packed
  four-byte destination slots and advanced shared-vertex origin; all are caught.
- The complete registered differential suite passes: **130 harnesses, zero
  failures**, with three workers.
- The isolated OpenCMR2 x86-64 build compiles/links and passes **19 tests**,
  including 500 original-identical Finland steps at all six render cadences.

The syntax inventory decreases raw accesses **818 -> 815**, fixed sizes
**64 -> 63**, hexadecimal multipliers **2181 -> 2180**, unknown occurrences
**9967 -> 9963**, unnamed field declarations **1451 -> 1448**, unique unknown
spellings **1227 -> 1226**. These counts concern the isolated committed
baseline; unrelated worktree edits are not attributed to this batch.

## Remaining boundary

The lighting-file zone/header/item loader still relocates Win32 records in
place. `LightZone` and `LightZoneVertex::pOwner` need their own disk/native
family. The focused native test covers shadow registration/emission; full
light-zone projection is compared against original x86, not claimed fully
native-ready by this batch. The successful Finland physics fixture likewise
does not prove every graphics/shadow setting or every stage.
