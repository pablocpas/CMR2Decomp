# Car spawn: runtime arguments and surface-state names

Baseline: `a7c93fc`, 2924 byte-exact functions. Priority is pointer/integer
conversions remaining in the current sources, including repeated declarations
and callers. This batch removes 17 diagnostics (593 → 576 in the same temporary
native compatibility snapshot); raw-access inventory falls 1341 → 1337.

`Car_Spawn` (0x43c7f0) now accepts `Car *`, `SceneNode *`, and `FixVector *`
for its three runtime addresses. Its caller in StageTiming uses those types,
including the repeated declaration. SceneNode.current is accessed by member:
its old 0x98 position changes when the preceding pointers expand on x64.
The two local CARF/CARB offset macros and raw short stores disappear.

| Original offset | Runtime field / view | Evidence |
|---|---|---|
| Car 0x210..0x26f | collisionCornersLocal[8] | Spawn constructs four lower wheelPos and four upperCornersLocal; the original leftover search walks all eight vectors. |
| Car 0xb1c | flags, byte | Existing named field, spawn writes 3. |
| Car 0xb16 | maxSteeringAngleDegrees, short | Existing steering consumer and setter use this limit; spawn writes 0x2aa. |
| Car 0xafe | engineStartTimer, short | Existing engine timer consumer; spawn resets it. |
| Car 0xa74 | surfaceNoise, int | Both surface producers derive it from g_surfaceNoise and smooth toward the target; HudDash reads it. |
| Car 0xa78 | surfaceNoiseTarget, int | Both producers assign the target; StageObject_CopyCarSurfaceNoiseTarget copies it into the current value. |
| Car 0xb29 | surfaceDragLevel, byte | Car_SetSurfaceDragLevel sets it; both surface producers select one of seven groups of nine entries from g_surfaceDrag. |

The collision corner union describes a runtime pointer-free span, preserving
both the existing lower/upper names and the original contiguous eight-vector
view. It adds no guessed fields and changes neither serialized records nor
runtime storage size. The original no-op self copies remain. A simple vector
index changed MSVC6 register allocation; an unsigned sizeof loop limit changed
the original signed branch. The retained typed byte induction uses a signed
sizeof bound and reproduces the original instruction sequence exactly.
Central LayoutChecks verify the overlay, member offsets and byte/short widths;
existing Car size and pointer-offset checks remain active on Win32.

All provisional fields beyond the three evidenced surface names retain their
original identifiers. CarDamage_BuildRelativeVelocityHull stays byte-exact with
the corrected original order: vertices 4–5 use field_0x770[1], 6–7 use [0].

## Verification

The existing registered differential_car_spawn.py exercises the entire routine
in 5376 combinations against the original, checking all provider calls and the
complete heap. Model, selection and tuning leaves remain controlled providers.
The new check_64bit_car_spawn.py compiles the actual current body and four actual
matrix getters using the runtime headers and portable fixed-point primitives.
Its 336 scenarios cover fourteen car types, four cheat combinations and six
selection configurations; native addresses exceed 4 GB. It compares the whole
primitive heap projection and gravity globals against the original under ASan
and UBSan. Original offsets exist only inside the test oracle. It also checks
the five supplied node pointers, both newly assigned matrix pointers, and the
remaining null pointer fields. The model-binding branch remains controlled by
the existing Win32 provider fixture; native scenarios pass the already-bound
model flag.

The native initial-state (354 scenarios) and surface-contact (480 scenarios)
checks also pass after the shared-header and name changes. The compatibility
snapshot audit preserves 328 other platform/API errors, so its 576 diagnostics
are an inventory, not a successful whole-game native build. Car and SurfaceTypes
compile without additional errors in that snapshot. Its setup and limitations
are documented in 64bit-car-surface-cursors.md.

Full build, measurement, changed-TU gate and registered suite results are saved
in the accompanying files. No function may lose s, fz or byte-exact status.
