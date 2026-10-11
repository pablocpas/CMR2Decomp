# Co-driver pointers and car control modes

Baseline: `d183f5a`. Three migration families cover six source/header files through
34 reproducible operations. The call-pointer and queue-record families are
connected; control fields form an independent family. The final recipe replay
reproduces every source byte. All original FUNCTION/GLOBAL annotations remain.

The full MSVC6 build preserves **2924 exact functions out of 3364**, without any
s/fz/x regression. `Race_DrawPlayerCallSlots` improves s **56.3771% → 57.9336%**
and fz **78.9279% → 80.6273%**, while restoring an incorrect centering expression
to the original executable's behavior. The corrected deformation hull remains
exact.

Native pointer/integer diagnostics decrease **204 → 194**, across four functions.
**400 of the original 594** diagnostics have been removed; 194 remain. Raw accesses
decrease **1092 → 1081**. Unknown identifier occurrences decrease **10232 → 10105**,
unique spellings **1251 → 1247**, and unnamed field declarations **1481 → 1476**.
Fixed-size and hexadecimal-multiplier inventories remain 74 and 2194. These are
reproducible syntactic inventories, not proof that all remaining arithmetic is
record addressing or that the entire game now compiles natively.

## Pointer family and file classification

| Original symbol / API | Recovered type / name | Evidence |
| --- | --- | --- |
| 0x537094 | `unsigned int *g_loadedCoDriverCalls` | The loader finds `%s.cod` in the stage archive. The returned file payload is the array of packed 32-bit numeric call IDs indexed by the callout queue. |
| 0x537358 | `unsigned int *g_coDriverCalls` | Queue indexes the published pointer with byte call/previous-call IDs. Successful loading publishes the loaded pointer; release clears only the published pointer. |
| `Race_DecodeCallSlotIcon` texture output | `Texture **` | Decoder returns `g_arrowTexture`, itself created by the texture loader. The output is passed by address from queue/drawing locals and then delivered to `Sprite_Queue`. |
| Draw `texA` / `texB`; queue `texture` | `Texture *` | All output callers retain a full native pointer; drawing no longer converts integers back to texture pointers. |

The `.cod` payload contains **numeric words**, not offsets relocated into pointers.
No file field is widened. The two globals are native addresses of the file data;
the archive owns that data. Missing-file behavior is retained: the loaded pointer
becomes null, while the published pointer changes only on a successful load.
Release clears publication but retains the loaded-file pointer. The fixtures
explicitly cover both paths rather than silently changing the lifetime.

The draw entry point's second parameter remains the original unused int. Its
separate caller-side pointer casts are still pending and are not counted as
removed in this family.

## Queue slots and car control fields

`RaceCallRecord` now has one shared declaration in RaceCallRecord.h. Its runtime
layout is pointer-free and stays twelve bytes on x86 and x64.

| Win32 offset | Field | Evidence |
| --- | --- | --- |
| 0x00 | `int primaryCallId` | Queue stores the packed primary word; drawing decodes it and sends it to audio slot assignment. |
| 0x04 | `int secondaryCallId` | Queue stores zero or the separated/previous call word; drawing handles its second icon/audio call. |
| 0x08 | `unsigned int flags` | Low byte is the countdown, 0x100 marks a queued slot, 0x200 marks its assignment to audio slots. |

Queueing, resets, route/call traversal, drawing and slot shifting all use these
names. The packed IDs remain 32-bit numeric values; they are never pointer
representations. Size/offset checks are centralized in LayoutChecks.cpp.

Car fields keep their existing int types and Win32 offsets, with new central
checks. Their native positions follow Car's preceding pointers.

| Offset | New name | Evidence |
| --- | --- | --- |
| 0xb88 | `steeringInputMode` | Indicator dispatch writes 0 for digital input, 1 for filtered analog input, 2 for direct analog steering. AutoGear sets the steering angle directly when the value is 2. |
| 0xb8c | `analogThrottle` | Dispatch marks a mapped analog accelerator. AutoGear consumes the six-bit control intensity directly for throttle torque when set. |
| 0xb90 | `analogBrake` | Dispatch marks a mapped analog brake. AutoGear scales braking with its control intensity when set. |

`g_controlCar` replaces 0x58e0a0: the control update selects it through Car_Get and
passes it to human/AI control paths. `g_carAxisInputEnabled[3]` replaces 0x58e0a8:
the controller-mapping producer tests bindings for steering, throttle and brake.
Every declaration/producer/consumer receives the rename. The indicator body
reads/writes the Car fields directly, removing eleven raw accesses and its
redundant Car casts. AutoGear and replay encoding use the same named fields.
The numeric DeviceInfo axis cursor remains a separate field-recovery task.

## Original icon centering restored

The new complete draw harness found an existing transcription error: the
single-icon path centered using the wobble angle instead of the rotated icon
width. With a screen width of 800 and an icon rotated down to six pixels, the
original draws at x=397; the old transcription draws at x=860.

Original instructions at 0x417c75..0x417c8f leave the rotated short width in AX,
store it in the rectangle, or load the unchanged width when no rotation is
needed; a missing icon clears both. At 0x417d55 the single-icon path sign-extends
that width and divides by two before centering. The final source takes the
width from `rectMain.w` through the existing r temporary. The wobble temporaries
are signed shorts, matching the original word loads/negations and low-word
angle tests. Neither the original game's behavior nor its known slot quirks
are changed. No inline assembly, pragmas or volatile additions were used.

## Validation and boundaries

- Full build, full measurement, fast comparison preparation and changed-TU gate
  pass. All 3364 functions retain s/fz/exactness, except the draw improvement.
- `differential_codriver_calls.py`: **4480** original/rebuilt scenarios, executing
  all five complete producer/release/decoder/queue/draw bodies. The decoder also
  passes an independent packed-ID/icon model. Complete guarded heap, queue state,
  texture outputs, source/destination rectangles, colors, provider arguments,
  call order and x86 ABI agree. The sine table is initialized, exercising the
  actual rotating/centering path.
- `check_64bit_codriver_calls.py`: the same **4480** scenarios against original
  x86 with five actual native bodies, full queue and render/provider traces.
  Texture outputs write eight bytes, retain native identities above 4 GiB, and
  keep adjacent guards. Production Sprite/queue declarations are used; texture
  and graphics fixtures implement only the opaque pointer/prefix contracts
  accessed here. ASan/UBSan pass and pointer/integer casts are compiler errors.
- Existing player-indicator harness compares the entire dispatch with an
  independent signed arithmetic model, guarded memory and provider traces.
- `check_64bit_player_indicators.py`: **324** native cases against original x86
  and that model. Actual body and production Car/DeviceInfo declarations verify
  native field positions, complete Car guards, heap, device-button merging and
  provider traces, including extreme axis values, separate/shared/missing
  bindings, completed racers and disabled axes. ASan/UBSan pass.
- `check_64bit_route_nodes.py` uses the production queue declaration after the
  rename: its **576** native original-x86 comparisons still pass.
- The compatibility projection retains **367 existing compiler errors**, with
  no new or removed diagnostics; the complete error log is retained. It is not
  a complete native game build.

The full registered suite passes: **129 harnesses, zero failures**, with three
workers. Measured source/executable/PDB hashes and all FUNCTION/GLOBAL
annotations are unchanged after validation. Replay hashes, matching delta and
final inventories accompany this document. All remaining
families stay in the active objective.
