# Runtime collision boxes: shared geometry and native pointer access

Baseline: `8f268f4`, 2924 byte-exact functions. The same native compatibility
snapshot records 576 → 558 pointer/integer diagnostics, eliminating 18:
14 in Collision_SplitBoxSeparationMovement, three in
StageObject_UpdateCarBoxShadowLighting, and one in
Collision_ResolveCarTurnedObjectContact. Its additional platform/API errors
remain 328; this count is an inventory, not a whole-game native build.
The raw-access inventory falls 1337 → 1319 without changing audit patterns.

Collision2D.cpp and StageObjects.cpp previously repeated the same 0x98-byte
box with int pointers to vector geometry. Collision2D.h now owns one shared
runtime definition. The existing top/bottom members in StageObjects establish
the meaning of the opaque eight bytes in the other definition. No file layout
is changed: these records are constructed in memory.

| Win32 offset | Field | Evidence |
|---|---|---|
| 0x00 / 0x04 | halfWidth / halfLength | Projection limits used by both box tests and construction. |
| 0x08 / 0x0c | top / bottom | Vertical overlap rejection in Collision_TestOrientedBoxCornerOverlap. |
| 0x10 / 0x1c | axisA / axisB | Dot products, separation directions and footprint construction. |
| 0x28..0x2f | pad_0x28 | Existing opaque state span retained; not renamed without recovering its producers in this batch. |
| 0x30..0x8f | points[8], pointWords[24] | Fixed vector span, first four footprint points. The word overlay contains the original cursor biased to y. |
| 0x90 | pArray: FixVector * | Eight world corners, moved by the same contact displacement. |
| 0x94 | pVertex: FixVector * | World centre, moved before the corners. |

All offsets, total size and both view widths are checked centrally. Native
layout is 0xa0 bytes: pArray remains at 0x90 and pVertex moves to 0x98.
The point-word span contains no pointers; CollisionBoxPointAtY subtracts
offsetof(FixVector, y) to recover named x/y/z members from the original
interior cursor. This keeps cursor arithmetic inside an actual array while
preserving MSVC6 scheduling. A direct vector-pointer cursor and byte counter
divided by sizeof(FixVector) lost matching; both trials were discarded.
The retained corner loops use vector indices and reproduce the original
streams, improving two register-allocation scores without fuzzy losses.

The complete split, sphere/box and car/box interfaces now accept CollisionBox *;
all repeated declarations and callers agree. Other box consumers use native
vector members, including separation and turned-object correction.
Turned-object entries are StageObject **; their flag read uses the existing
field_0x10 member, whose native position follows the expanded mesh pointer.
The ON-DISK StageObject warning in Sector.h remains applicable: the port must
separate serialized offsets from runtime pointers before its file loader uses
native records.

Two globals are named with producer/consumer evidence, retaining their GLOBAL
addresses: g_collisionPushB at 0x5914b8 is the accumulated movement of box B;
g_collisionContactNormal at 0x5915e8 receives the contact direction/selected box
axis and is used to project impulses and tangential corrections.

## Verification

The new registered differential_collision_boxes.py executes four complete
bodies against the original: 3500 independent split-model scenarios (including
shared geometry, null pointers, negative and extreme split scales), 1008 real
sphere/box scenarios, 147 independently predicted lighting-bound cases and
128 turned-object contacts. It checks whole poisoned heaps, global writes,
provider traces, stdcall cleanup and preserved registers. Geometry and fixed
math are real; turned-object overlap and lighting application remain controlled
leaves. Sphere return comparison uses the low byte consumed by original callers;
the original leaves upper EAX bits unspecified on some returns.

The existing registered car-contact harness still exercises car/box and box
separation. The new check_64bit_collision_boxes.py compiles both complete split
and sphere bodies with the current production header and portable math: 432
scenarios compare whole primitive heaps and globals with the original under
ASan/UBSan. Actual boxes, centres and corner arrays have addresses above 4 GB;
all supplied native pointers remain intact. The existing native contact check
also passes its four actual contact bodies. Its optional historical audit still
accepts the old local box definition and prototypes.

Measurement checks all 3364 functions: 2924 remain exact, no s/fz/x regression.
Split score improves 72.99% → 74.94%, turned-object score 88.79% → 89.48%; both
fuzzy scores stay unchanged. Full suite and gate logs accompany this document.

## Remaining construction work

The old car pool g_unk0x00590ed0[8][0x98], the object scratch block
 g_unk0x005915f8[0x26], and int-based producers
StageObject_QueueViewLensFlare / StageObject_BuildSpriteExtentOrientation still
need native runtime records and pointer assignments. The typed consumers and
focused tests do not establish that those original constructors or the whole
game are already x64-safe. These are coherent follow-up work, along with the
headlight box consumer; they are not hidden or dismissed as fixed.
