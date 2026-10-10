# Vehicle physics, game speed and render FPS

The corrected port matches original + SilentPatch for the controlled Finland
stage 1 excerpt: 500 updates at 30, 60, 120, 144 and 240 FPS and variable cadence.
This is exact integer/bit comparison, without tolerances or parameter tuning.
It establishes this fixture's behaviour, not full-game equivalence.

## Corrections and evidence (2026-10-10)

MSVC6 `/QIfist` emits x87 FISTP conversions using nearest-even rounding. Ordinary
C++ casts compiled with Clang truncate. Explicit `llrint` conversions restore
startup math tables, engine/handbrake phases, steering, body rotation and impact
angles, preserving the original narrowing after the signed 64-bit result. The
original vehicle constants, fixed-point integration and force rules are retained.

All 16,896 entries of the five startup tables match dumps from both original +
SilentPatch and the latest MSVC6 decompilation build. `math_tables_test` compares
those actual computed tables against the frozen original dump without game assets.

The stage regression compares 2,015 canonical values per tick: every captured
car field except four interpolation-phase bytes and the remaining steps of the
current render frame, plus complete parts, damage, checkpoints, per-car timing,
world/body matrices and shadow transforms. The stored simulation rate remains
in the comparison. Controls, race state, physics scale and timestep are checked
separately through the explicit vehicle comparison. The same 897 asset hashes
used for the original fixture were verified before freezing the corrected runs.

| Render cadence | All 500 vehicle updates vs original + SilentPatch | Updates in (10 s, 18 s] |
| --- | --- | --- |
| 30 FPS | Identical | 200 |
| 60 FPS | Identical | 200 |
| 120 FPS | Identical | 200 |
| 144 FPS | Identical | 200 |
| 240 FPS | Identical | 200 |
| Variable | Identical | 200 |

The variable cadence repeats 7/13/33/5/22/48/9/23 ms intervals. The probes load
through the real frontend with fresh settings, default model 0 and automatic
gears. Each run contains 125 waiting/countdown updates and 375 driving updates,
including acceleration, steering, lateral contact, braking and handbrake.

## Timing defects preserved in the historical repo, fixed in the port

The original recorder measured race x87 control word `0x027f`: 53-bit arithmetic
and nearest-even conversion. Disassembly of `Car_UpdateEngineNoteFalloff` shows
that its previous time boundary is stored as float at `0x0042c9c0`, while its
current boundary retains double precision before FISTP at `0x0042c9d4`. At certain
half-step boundaries the current value rounds forward, but the saved previous
value rounds backward when read next frame. That repeats an already counted step.

This explains the original's observed 206 updates in the same eight seconds at
30/60/144 FPS and 212 at 240 FPS. OpenCMR2 evaluates both boundaries in the same
double precision while retaining nearest-even conversion and the original signed
interpolation phase. It now consistently runs the stored 25 Hz rate. Therefore
**wall-clock pacing deliberately differs from the original's duplicate-step bug**;
we do not claim identical original wall-time speed or copy that FPS dependency.

The pre-race hold originally checked completion after scheduling another waiting
update. At some render cadences this added a 126th countdown update. The port
checks the completed hold before another waiting-state step; the next race state
consumes the outstanding step. Each tested cadence now has 125 waiting updates.
The five-step overload cap, stored alternative rates, pause/menu rebasing and
32-bit millisecond wrap retain their contracts and have scheduler tests.

Startup remains frame-quantized: the first driving update is observed at 5.233 s
at 30 FPS and 5.041 s at 240 FPS after entering race resource mode. Steady pacing
is 25 Hz; identical loading/fade timing and hardware-input latency are not proven.
Presentation/trail/contact buffers, shared RNG, interpolation and game-clock
values still differ in the full-state comparison. Authoritative wheel-ground
contacts remain part of the car-state regression. Native Windows presentation,
all cars/surfaces, complete stages, manual gears, replays and multiplayer need
additional fixtures.

CMR2Decomp already reproduces the nearest-even conversions with `/QIfist`.
Its comments and numeric audit now document that requirement and the historical
clock bug; its statements and constants were preserved. These changes improve
documentation, without claiming a new machine-code matching score.

## Frozen evidence and regression commands

- [Corrected six-cadence acceptance](../tests/reference/finland-stage1/corrected/acceptance.json)
- [Source, binary, asset and audit provenance](../tests/reference/finland-stage1/corrected/manifest.json)
- [Full original vs corrected port at 60 FPS](../tests/reference/finland-stage1/corrected/original-vs-port-60.json)
- [Original capture setup and historical results](ORIGINAL_REFERENCE.md)

The previous `fps/` and baseline recordings remain untouched for before/after
comparison. The corrected recordings are under `corrected/`.

```sh
cmake --preset linux-x86 -DOPENCMR2_BINK_TEST_DATA_DIR="$HOME/cmr2game"
cmake --build --preset linux-x86
ctest --test-dir build/linux-x86 --output-on-failure \
  -R 'math_tables|car_scheduler|physics_original_parity'

# Retain all six new recordings and a report, using a new output directory:
python3 tests/physics_parity.py --probe build/linux-x86/tests/stage_probe \
  --data "$HOME/cmr2game" --output build/physics-check-next
```

The regression fails for any vehicle difference, state/control mismatch, invalid
capture or cadence that does not produce 200 updates in the measured eight seconds.
`tools/reference_capture/physics.py` remains a diagnostic report generator whose
exit status means a valid report was produced; read its embedded verdicts.
