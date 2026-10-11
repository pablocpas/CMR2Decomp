# Runtime particle pointers and route-record cursors

Baseline: 87790e4. The reviewed compatibility inventory falls from 356 to 322
pointer/integer diagnostics: 34 removed across 15 functions. The raw-access
inventory falls from 1304 to 1288. All 3364 s/fz/x rows remain identical;
2924 functions remain byte-exact, including the corrected deformation hull.

## Records and evidence

| Record / Win32 offset | Recovered field | Evidence |
| --- | --- | --- |
| ParticleType +0x5c | ParticleDrawCallback draw | StageTiming_InitEffectParticleTypes assigns Spark_Draw, GlassShard_Draw and Debris_Draw; Particle_DrawAll invokes them with a Particle*, ParticleType* and camera SceneNode*. |
| ParticleType +0x68 | ParticleInitCallback callback | Particle_Spawn calls this initializer with its particle, type and pointer to initialization data. Owner-index, Car* and debris-word producers establish pointer transport; motion callbacks retain their integer update parameter. |
| Particle +0x40 | FixMatrix* attachmentMatrix | Producers pass &pCar->pSceneRoot->current; particle drawing obtains this matrix's position and adds it to local particle coordinates. Null denotes no attachment. |
| RaceRecord +0 | int routeNodeIndex | UpdateCarRoute queries node position/direction using this field, advances/retreats it and updates it from route probes. FindNearestCarRouteNode supplies a nearest-node index. |
| RaceRecord +4 | int resetRouteNodeIndex | FindNearestCarRouteNode saves the initial nearest node here; both reset routines restore the current index from it. |

ParticleType and Particle are allocated in memory by Particle_Init with
sizeof(ParticleType)/sizeof(Particle); their editor and pool do not deserialize
these records. Win32 sizes remain 0x70 and 0x68. Native sizes are 144 and 128;
Particle's attachment pointer is at 72 and texture pointer at 112. Templates,
particles, cameras, matrices, initialization data and callback addresses are
exercised above 4 GB. The current headers supply the layouts used in native
fixtures. All members of both touched particle records have central original
sizeof/offsetof checks.

RaceRecord moved from a local definition to RaceRecord.h. It remains a
pointer-free 0x18-byte record with seven central member checks and a size check.
It is also copied into replay snapshots; no serialized bytes or capacity change.
Remaining fields retain their unknown names. Four previously unnamed fields
now have demonstrated names; no unknown global's role was guessed.

## Complete pointer flow

Particle_Spawn accepts a FixMatrix* attachment and void* initialization data.
All producers, callers and repeated declarations agree, including the separate
StageObjects declaration found by full linking. ParticleEdit_SetCallbacks and
ParticleType use actual draw/initializer function pointers. Draw callbacks use
a camera SceneNode*; GlassShard_Draw retains this unused argument as a pointer.
GlassShard_Init and Debris_Init now have the common void*/ParticleType*/void*
initializer signature and cast their documented payloads internally. Owner-data
initialization consumes an actual pointer rather than converting an int to one.
Update/postUpdate callbacks continue to receive their numeric update argument.

Particle_DrawAll retains the original cursor biased into field0x55 to preserve
MSVC6 scheduling, but its base and stride come from offsetof and sizeof.
DRAW_PARTICLE recovers the owned record and every formerly raw read uses its
member. Attachment translation reads named FixMatrix.position components.
The animation's address-derived seed retains only the original low 16 bits;
UINT_PTR preserves native address width before the mask. No pointer is rebuilt
from that numeric seed. Literal null checks replace two casts of an int zero.

## Preserved alias-sensitive word access

Direct RaceRecord member stores and BYTE* traversal variants change the exact
restore function's scheduling (including the order of a load and zero store).
The retained local numeric cursor uses size_t: unsigned int in MSVC6 and an
8-byte type on the tested native ABI. RaceRecord.h checks that this type can
hold a pointer on every build, independently of the original-layout macro.
The function's // 64BIT comment explains why this numeric cursor remains.

Its primitive word accesses use offsets derived from RaceRecord, rather than
hardcoded 8/4/12 displacements. Constant offset differences must be grouped
before adding/subtracting them from the cursor; that form preserves the original
load scheduling. The signed terminal comparison uses INT_PTR and a numeric
member bias calculated from the owned eight-record array. These word accesses
remain explicit because direct fields lose matching. They contain no pointer
members, and their complete behavior is exercised natively and against x86.

## Validation and limits

The new registered harness executes 616 original/rebuilt cases with independent
models for visibility, direct/animated/null textures, animation clamping/looping,
address seeding, attachment translation, colour/shade/rotation, custom callbacks,
all three initializers, callback setters, and all eight restored route records.
It checks complete guarded input/output heaps, provider order and identities,
stdcall cleanup and callee-saved registers. Billboard submission, custom draw,
random and route-reset leaves are controlled. Existing exhaustive exhaust,
debris and spawn harnesses continue to exercise their actual callers/bodies.

The native fixture executes 656 scenarios with current production headers and
portable fixed-point helpers under ASan/UBSan: actual draw, setter, three
initializer, spawn and route-restore bodies. Whole records and unknown padding
are guarded. Spawn covers empty/occupied pool selection, attachment, shade,
colour, null/init callback paths and full-width init-data transport. Its random
basis and random-size paths are outside this focused native fixture; the
existing x86 lifecycle harness exercises the broader original behavior.

Full build and measurement preserve every score. The changed-TU gate passes
and the complete suite passes all 124 harnesses with zero failures; their logs
accompany this document.

The external-port compatibility projection still reports 370 additional
compiler errors, unchanged from the previous batch, primarily platform/header
integration. All are retained, not filtered out; the normalized diagnostic
comparison has no new or removed errors. The automated --theirs merge initially
dropped the port's port/sys.h include when RallyData.cpp gained stddef.h. The
projection explicitly retains that port-only include so Sys_GetTicks remains
declared; neither the decomp nor the external port was edited for this repair. This inventory and the focused
native tests do not establish a whole-game 64-bit build: 322 conversions and
1288 raw accesses remain, including disk-backed relocation and unrelated
runtime families.
