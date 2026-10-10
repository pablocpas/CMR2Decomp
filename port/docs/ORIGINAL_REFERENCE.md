# Executable reference: original + SilentPatch

The original executable with the real SilentPatch is now recorded independently
of OpenCMR2. The corrected port now matches the fixture's physical vehicle state
for 500 updates at six render cadences; full-state/full-game equivalence remains
open. Numeric conversion fixes and intentional timing corrections are documented
in [PHYSICS_PARITY.md](PHYSICS_PARITY.md).

The primary vehicle and game-speed checks across render FPS are documented in
[PHYSICS_PARITY.md](PHYSICS_PARITY.md). The port retains 25 updates per second
and identical measured vehicle states against the patched original across
30..240 FPS in this fixture. It deliberately removes duplicate scheduling steps
observed in the original, so wall-clock pacing differs from that defect.

## Historical recordings before numeric fixes (2026-10-09)

The frozen fixture is [`tests/reference/finland-stage1`](../tests/reference/finland-stage1).
It contains compressed raw recordings, the scalar schema, resolved input CSV,
SHA-256 provenance and field-by-field comparison reports. Executables, DLLs and
game assets are not distributed in this fixture.

The 2026-10-10 corrected captures are retained in `corrected/`; its acceptance
report checks 2,015 canonical vehicle/structure values at each of 500 steps.
The original math dump adds a separate exact test of all 16,896 table entries.
The historical recordings and reports below remain unchanged.

The scenario loads Finland stage 1 through the frontend and garage, using fresh
configuration, one default car (model 0), automatic transmission, seed 1700000000,
60 rendered frames per virtual second and 500 observed ticks. The first 125 ticks
are countdown; the remaining 375 apply throttle, steering, braking and handbrake.
This is an excerpt of a stage, not a full rally or all possible physics behaviour.

| Comparison | Result |
| --- | --- |
| Original + SilentPatch, independent repeat | All canonical fields identical for 500 ticks. Raw addresses differ. |
| OpenCMR2, independent repeat | All canonical fields identical for 500 ticks. |
| Original + SilentPatch vs latest standalone decompilation | Car fields match except two interpolation bytes at +0xa90/+0xa91. Parts, damage, checkpoints, per-car timing, world/body matrices and shadow transforms match. Time, RNG, interpolated/contact records differ. Full-state equivalence is **not** established. |
| Original + SilentPatch vs OpenCMR2 before correction | 929 scalar fields differ at some point. Car position Y differs at tick 0: 633390 vs 633388. Velocity X first differs at tick 9: -49 vs -50. Trajectories subsequently diverge. |

The original SHA-256 is
`21b2dd13798724bcfe0c64b44216fb3e772c86addd330e821e53d5962fca3817`,
checked against the decompilation project's reference. The latest decompilation's
executable, PDB and all 127 source file fingerprints were checked against its
build manifest before capture. The fixture records their exact versions.

SilentPatch is Build 1 (08.08.2015), from the existing local toolchain. Its DLL
and INI are fingerprinted. The recorder checks that it actually changed game
code: 130 code bytes changed, excluding recorder detours. Loading a DLL alone
would not pass that check. The standalone decompilation is an **additional
unpatched control**: SilentPatch uses original code addresses and cannot simply
be copied onto a relocated executable. The authoritative side here is the
patched original, not a claim of a patched standalone decompilation.

## Observation and coverage

`tools/reference_capture/capture.cpp` is a small MSVC6 observer DLL that forwards
DirectSoundCreate to the real SilentPatch. Original physics functions still run.
It verifies displaced instruction bytes before installing trampolines, injects
resolved controls immediately before the car update, and records state after
`Car_DecrementContactTimers`, at the same boundary as `stage_probe`.

Both recorders share `src/diagnostics/state_capture.h`. CMRSTAT1 preserves raw
bytes for cars, part/damage/contact tables, per-car timing/checkpoints, stored
transforms, shadow/wheel transforms and dereferenced world/body matrices. It
also records control values, car order, physics step/scale, stage clock, game
clock and the CRT RNG state. The Windows recorder tracks and checks the MSVC RNG
sequence against the real CRT; it does not substitute random return values.

`schema.py` uses the actual 32-bit C++ layouts and compile-time size assertions to
locate 2,368 scalar fields per car's captured structures. `compare.py` retains
exact fixed-point integers and float bits. Pointer fields, named padding and
unused matrix W components are excluded from comparison, but remain in the raw
recording. Opaque bytes are included rather than silently discarded. Reports
list first/last differing ticks, values and counts for every differing field.
Incomplete files, missing/duplicate blocks, incorrect sizes, invalid car indices
and missing/mismatched footers are rejected.

This does **not** dump every global or everything reachable from pointers. Moving
PartState records, debris, AI, hardware input filters, full replay streams,
multiplayer/ghost state and moving stage objects need additional fixtures.
Interpolated records and shared RNG remain part of the comparison; their
mismatches are not evidence on their own of a vehicle integrator mismatch.
The historical position and velocity mismatches were stronger evidence; they
are now resolved in the corrected fixture.

The virtual clock is enabled after initial Windows setup, then reset to 100000 ms
on entering race mode. The default-configuration dialog is acknowledged, Enter
is sent during boot/menu navigation, and Bink's intro wait is removed while its
frames decode normally. These are recorded fixture adaptations. The process
ends after its last recorded tick. Configuration and saves are isolated; asset
directories are linked read-only by convention and never used for saves. The
recordings cover simulation with scripted resolved inputs, not real-time
presentation, audio fidelity or keyboard latency.

## Reproduce from local executable/toolchain files

Requirements: 32-bit Wine, a display (or Xvfb), MSVC6 and the existing original
and latest decompilation builds; Python `pefile` and `capstone`; Clang with the
32-bit development libraries for schema generation. Defaults locate the sibling
`CMR2Decomp` and `tools` directories. All asset files remain user supplied.

Use a **new** output directory and a disposable win32 Wine prefix. For a virtual
display, start `Xvfb :95 -screen 0 1024x768x24` separately and set `DISPLAY=:95`.

```sh
mkdir -p /tmp/opencmr2-reference-prefix
WINEARCH=win32 WINEPREFIX=/tmp/opencmr2-reference-prefix wineboot -u

python3 tools/reference_capture/prepare.py --data ~/cmr2game \
  --output build/reference-new --wineprefix /tmp/opencmr2-reference-prefix
python3 tools/reference_capture/schema.py build/reference-schema-new

python3 tools/reference_capture/record.py build/reference-new original \
  --wineprefix /tmp/opencmr2-reference-prefix
python3 tools/reference_capture/record.py build/reference-new decomp \
  --wineprefix /tmp/opencmr2-reference-prefix
```

`record.py` fingerprints the prepared files before starting, configures only the
disposable prefix, uses Wine's builtin DirectDraw (the local dgVoodoo wrapper
failed during this capture), and verifies completion. The decompilation side
gets registry paths pointing at its isolated copy. Existing captures are never
overwritten. Prepare another output for an independent repeat.

Capture the port with the original recording's resolved inputs:

```sh
mkdir -p build/reference-port-new/user
XDG_DATA_HOME="$PWD/build/reference-port-new/user" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy OPENCMR2_HEADLESS=1 \
OPENCMR2_REFERENCE_CLOCK=1 \
OPENCMR2_STATE_CAPTURE="$PWD/build/reference-port-new/state.bin" \
  build/linux-x86/tests/stage_probe ~/cmr2game \
  build/reference-port-new/trace.jsonl 60 500 \
  tests/reference/finland-stage1/inputs.csv

python3 tools/reference_capture/compare.py \
  tests/reference/finland-stage1/original-state.bin.gz \
  build/reference-port-new/state.bin \
  --schema tests/reference/finland-stage1/schema.json \
  --output build/reference-port-new/comparison.json
```

The comparator exits 0 for identical canonical state, 1 for differences and 2
for invalid/incomplete input. It accepts raw or gzip recordings. Normal gameplay
is unaffected by the optional probe environment variables.

## Next fidelity work

Start before the first observed tick: investigate car/ground initialisation and
numeric conversion behaviour against the original machine code. The original
scheduler contains inline x87 `fistp` conversions; Linux casts currently truncate.
That is a concrete audit target, not yet a demonstrated cause of all trajectory
differences. Preserve the frozen reference while changing these paths, and compare
both full state and authoritative car fields after each correction. Timing/RNG
and presentation differences require their own investigation.

After this fixture matches, expand to other surfaces, cars, collisions, restarts,
manual gears, pause/focus and race completion. A matching short Finland fixture
cannot establish universal equivalence.
