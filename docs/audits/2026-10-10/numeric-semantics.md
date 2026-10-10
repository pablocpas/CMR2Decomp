# Numeric semantics and preservation (2026-10-10)

OpenCMR2 exposed differences caused by translating MSVC6 numeric conversions
to ordinary C++ casts. This audit distinguishes porting errors from reconstruction
errors and behaviour already present in the original.

## Confirmed original behaviour

Original + SilentPatch has x87 control word `0x027f` during the recorded race:
53-bit arithmetic precision and nearest-even integer conversions. This is a
measurement under Wine with builtin DirectDraw, not all Windows/driver setups.

| Original address | Operation | Semantics |
| --- | --- | --- |
| `0x004b7b20` | `Scene_InitFixedMathTables` | FISTP to signed 64-bit, then keep the low integer/word. All 16,896 entries are initialized. |
| `0x004946c0` | `AutoGear_UpdateRollingFollowRate` | Round the angle before indexing the sine table; truncation changes engine torque. |
| `0x00494960` | `AutoGear_UpdateSecondarySwing` | Round the handbrake force phase. |
| `0x0042c890` | `Car_UpdateEngineNoteFalloff` | Previous boundary stored as float, current retained in x87, integer conversions through FISTP. |

The complete original math tables were dumped through an observer and compared
with the latest MSVC6 rebuild: all 16,896 entries match, including both singular
tangent entries clamped to INT_MAX. The existing `/QIfist` configuration is
necessary: Clang ordinarily truncates the same source casts. Replacing historical
casts with `llrint` in this matching repository is not justified; it belongs in
the portable implementation.

The patched original and latest standalone MSVC6 rebuild also agree on physical
car state in a controlled 500-step Finland stage 1 excerpt. This does not prove
all-game equivalence. SilentPatch applies only to the original-address binary;
the standalone rebuild is an additional unpatched control.

## Historical scheduler defect

At `0x0042c9c0`, the previous boundary is stored to float. The current boundary
proceeds directly to FISTP at `0x0042c9d4`. At 100 ms and 25 Hz, with the stored
float 0.001 multiplier, the current boundary is slightly above 2.5 and rounds to
3. The saved previous boundary is exactly 2.5 and rounds to 2 on the next
rendered frame. This counts the third step again.

Captures measured 206 updates in (10 s, 18 s] at 30/60/144 FPS and 212 at
240 FPS, instead of the stored 25 Hz rate (200 updates). The original instructions
and measured FPU mode explain the pattern. Preserve it here for historical
fidelity. OpenCMR2 computes both boundaries in the same precision to prevent
extra steps and separately fixes a frame-dependent extra countdown step. These
are intentional modernization changes, not missing original code.

## Changes to this repository

Corrected comments about truncation and complete table coverage; documented
scheduler precision. No statements, constants, types or build flags changed.
This improves the reconstruction's documentation without claiming a higher
machine-code match percentage. Future proven reconstruction errors belong here
first, verified against original instructions. Intentional behaviour changes
belong in the modernization port.

`numeric-semantics.json` fingerprints the executable, compiler, patch, observer
and captures. The original recorder and stage comparison are in OpenCMR2's
`tools/reference_capture` and `tests/physics_parity.py`.
