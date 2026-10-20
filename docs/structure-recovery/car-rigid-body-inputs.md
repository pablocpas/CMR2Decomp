# Rigid-body forces, axes and suspension loads

`Car_Integrate` (`0x00440840`) consumes the car's existing corner forces and
body axes. Its former byte cursors identify these records:

| Original accesses | Recovered access | Evidence |
| --- | --- | --- |
| `0x66c`, then `0x654` | `cornerForce[3]`, then `[1]` | Two vectors separated by 24 bytes; both contributions are added twice on the yaw-only path. |
| Force offset minus `0x438` | `wheelPos[index].x` | `0x66c - 0x438 = 0x234`, the X component of wheel position 3; the next iteration selects position 1. |
| `0x648 + 0x24..0` | `cornerForce[3..0]` | The full-rotation path visits the four lower corners in descending order. |
| `0x270/0x274/0x278 + 0x24..0` | `corners[index].x/y/z` | The lever arm subtracts the car's world position from the corresponding world corner before taking its force cross product. |
| `0x360`, then `0x378` | `bodyAxisVectors[0]`, then `[2]` | The yaw-only update rotates right and forward; it does not update up. |

The 36-byte `bodyAxisVectors[3]` view shares the existing right/up/forward
members and nine-word view. It adds no storage. It also supplies the contiguous
three-vector input to `FixBasis_Integrate`. A `FixMatrix` has padding between
rows and is not interchangeable with this basis. Layout checks tie each named
axis to the corresponding row.

The integrator now uses these fields and array indices without car-relative
byte casts. `yawOnly`, `leverArm`, `cornerTorque`, `angularStep`, `positionStep`,
`acceleration` and `work` describe the observed branch, cross product, integrated
steps and reused scratch vector. Force accumulation order and fixed-point
arithmetic stay intact.

Full measurement improves the integrator's byte-audit score from 71.61% to
73.71% and fuzzy similarity from 80.39% to 81.25%. All 2924 exact functions
and every other score are preserved, with zero data issues.

`Car_UpdateWheelLoads` (`0x0043c640`, formerly named wheel torques) computes
scalar load shares. `cornerWeight[8]` names the table at `0x8b8`:

- `Car_SetupDriveTrain` (`0x0043fb50`) initializes the first four entries to
  one quarter of the car's mass.
- The updater caps suspension-height differences and subtracts their combined
  height/travel correction from that quarter-mass share.
- `Car_SetRideHeight` (`0x0043dff0`) sets `suspensionLoadScale` (`0x8d8`) to
  `mass / 8`; the updater uses it to convert height differences into load shares.
- `Car_UpdateCornerLoads` (`0x004348c0`) consumes these values when computing
  corner grip limits. The surface-compression word view also spans the table.

The updater's special tail is selected by `partHidden[3]`, read through
`StageTiming_GetCarReplayTailEntry`; it is not a replay-state test. That tail's
native-offset cursor remains unchanged in this batch: straightforward index
and pointer variants regress its original instruction match. No type or size
of the load fields changes.

The differential car-step harness now executes the complete integrator, its
matrix/basis helpers and corner rebuild rather than intercepting that pass.
Its 288 cases cover yaw-only and full rotation, three bases, ground settling,
suspension offsets, embedded/external matrix aliases and zero/fractional/full
time steps. Guards protect forces, setup data, unrelated car fields, pointers
and matrix padding; the previous-velocity snapshot and zero-step motion have
explicit invariant checks.

`check_64bit_car_integrate.py` runs five production bodies with native pointers
under ASan/UBSan and compares the complete heap projection with original x86
execution for the same 288 cases. Only the fixture translates the original
pointer-free Car spans; production code uses native fields and pointers.
