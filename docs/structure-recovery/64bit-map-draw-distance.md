# Preliminary map and native draw distance

Baseline: `096b134`. This batch migrates the complete graphics-distance and
option-preview-panel families. The unrelated weather-texture worktree edit is
excluded from the matching checkout and commit.

## Reproduced preliminary-map crash

Entering Information in the preliminary stage screen reproducibly crashes in
`SceneNode_Destroy`. GDB shows the first call to the world release callback
coming from a call to `OptionMenu_TransformHudPointToScreen` in the split panel.
That transform is declared `int`, ends without returning a value and is called
only for its two output coordinates. With Clang's optimization, this undefined
behavior turns its call into execution of the following world release function.
The second call then traverses the freed world. The saved backtrace captures
that failure; this cause is a prototype error, rather than a pointer offset.

The transform now returns `void`, consistently in its shared header, definition
and repeated declaration. Seven duplicate aliases of the existing `FixInterp`
record disappear. Its picture pointer becomes `Texture *`; redundant record and
texture casts disappear from the producers and consumers. The observed table,
selected index and filled-entry count become `g_optionPreviewPanels`,
`g_selectedOptionPreviewPanel` and `g_optionPreviewPanelCount`. The sentinel
0xff and original selection/animation order are retained. Central size/offset
checks cover the original 0x50-byte record and the fields involved.

The port regression enters Information, waits for the enlarged map and both
fade phases to reach their full values, holds the screen for five seconds from
entry and then starts the race. It completes 500 simulation ticks, including
375 driving ticks. Host, arcade time trial and recorded replay scenarios pass
as well. This is an execution check using real assets, without visual rendering.
All four scenarios also pass with an isolated native RallyData object that
excludes the pre-existing weather edit. The checked-in batch therefore does not
depend on that pending change; the main worktree and executable retain it.

## Route distance limits

`StageTiming_SetViewRouteDistanceLimit` writes the positive near/far limits from
the selected route node at Graphics offsets +0x3c4/+0x3c8. The record contains
pointer-bearing textures and DirectDraw members, so those locations move in a
native build. `Render_GetDetailDistanceScale` reads the actual near-limit member;
the producer writes elsewhere and the consumer retains a stale value.

The eight opaque bytes become `int baseDrawDistance` and `int maxDrawDistance`.
All six raw stores/reads in the route function, the detail-scale consumer and
the debug-overlay reads use these fields. `sizeof(Graphics)` remains 0x3cc on
Win32; central checks fix the two original offsets. The positive-only writes,
far-limit clamp, reversing node clamp, provider calls and floating-point
rounding are unchanged. No new pointer cache crosses a provider call.

The native ASan/UBSan fixture uses production Graphics and route-node records,
actual function bodies and allocation addresses above 4 GiB. Its 7,680 cases
cover ten detail settings, forward/backward limits, inactive players, signed
limits and node bounds. It checks the entire graphics object and all route
records, plus output distances and recipients. Restoring only the old route
offsets fails the whole-object model. This fixes demonstrated memory writes
and distance calculations; visual draw distance still needs acceptance in the
user's stages and cameras.

## Validation and progress

- Full MSVC6 build, central layout checks and changed-file gate pass.
- Every one of the 3,364 measured functions retains its previous byte and fuzzy
  scores, including all 2,924 byte-exact functions.
- The new original/rebuilt point-transform harness checks 1,024 cases against
  an independent model, including aliased outputs, full poisoned-heap guards,
  signed coordinates and preserved registers/stack cleanup. EAX is deliberately
  not treated as a result: callers consume only the two output coordinates.
- The existing camera-limit harness covers 6,000 cases, including providers
  that replace the Graphics object during the call.
- The complete suite passes all 132 registered differential harnesses.
- OpenCMR2 passes all 22 CTests, including four real runtime scenarios and the
  physics comparison against the original at six frame cadences.
- A syntax-only return-type audit of all 70 native game translation units
  reports no remaining return-type warnings or compile errors.

The syntactic inventory decreases from 757 to 751 raw accesses, from 9,888 to
9,813 unknown occurrences, from 1,221 to 1,212 distinct unknown spellings and
from 1,448 to 1,440 unnamed field declarations. These are work-list counts,
not a percentage of proven 64-bit compatibility.

The multi-stage light flash remains unconfirmed. The native lighting-file
relocation still uses Win32 zone/item/header strides and integer pointer slots,
and stage-object matrix reads in mesh cloning remain candidates for a later
complete family migration. Neither is included as a speculative visual fix.
