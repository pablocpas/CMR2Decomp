# Main integration — 2026-10-01

## Sources

- Main advanced from `0e18a20` to the matching review at `16527f6`.
- Both matching reviews were merged in `4d75801`, preserving the newer
  bitfield implementations and validating the conflicting session helpers.
- Working changes were copied from frozen patches of matching-review and
  matching-low-review. The original working directories were not committed,
  reset or cleaned. Their patches are preserved in `/tmp/cmr2-main-integration`.
- Wine/window rendering support was recovered from port/silentpatch-window.
  Its earlier gameplay fixes were compared with the newer implementations;
  older versions were not substituted over the matching review. The replay
  table correction in its working directory already exists in main.
- The old frontend and wave branches were checked for newly annotated
  functions missing from main: none were missing. Their branches remain
  available as historical references.
- SDK paths are the original pinned Git submodules again, replacing the
  review worktrees' absolute links, which pointed back into this checkout.

## Verified snapshots

| Metric | Frozen 16527f6 | Both reviews | Recovered working changes | Low-score matching batch |
|---|---:|---:|---:|---:|
| Measured reccmp functions | 3365 | 3367 | 3367 | 3367 |
| Reccmp score exactly 1.0 | 2406 | 2429 | 2433 | 2434 |
| Source functions audited by bytes | 3363 | 3363 | 3363 | 3363 |
| Exact after relocation | 2497 | 2523 | 2528 | 2529 |

Every snapshot was built with MSVC6 using the same translation-unit flags.
No originally exact function was lost in these comparisons. The final portable
build has 2529 exact functions, 32 more than the frozen reference;
834 source functions remain non-exact. Reccmp accuracy is 92.59%, a similarity
score distinct from the count of byte-exact functions. There are
14 functions with unresolved operands, which are included in those 834 and
are never counted as exact. Global data: 3197 variables, zero issues.

Thirty-two native differential harnesses passed on the portable build, including
collision/checkpoints under all four x87 rounding modes, session enumeration,
camera replacements, shadows, rotations, rankings, queues, replay slots,
registry failures, text coordinates, callbacks, network tables and knockout
seeding/round selection and scene reparenting. The final logs are retained in
`/tmp/cmr2-main-integration/clean-final`; build hashes and matching metadata
are saved in `CMR2PROGRESS/provenance.json`. Tests now print results only in the console.

The final build unifies the loader/frontend scratch buffer and the shared
1/65536 constant at their actual original addresses, and removes duplicate
annotations from declarations. It also uses the same build command in CI
and locally. Only Wine/MSVC6 execution was run here; Windows CI and a full
interactive race were not run during this integration.

## Low-score matching work

| Original address | Function | Reccmp before | Reccmp after | Byte-exact |
|---|---|---:|---:|---|
| 0x004069c0 | RallyData_InitKnockoutBracket | 11.17% | 66.24% | No |
| 0x00505e10 | CGameInfo::FUN_00505e10 | 22.73% | 79.17% | No |
| 0x004cf3f0 | FUN_004cf3f0 | 50.00% | 90.91% | No |
| 0x004735a0 | FUN_004735a0 | 41.03% | 100.00% | Yes |
| 0x004ac7a0 | SceneNode_Reparent | 38.46% | 94.44% | No |

The bracket initializer now preserves the original bitfields, four constant
random-divisor branches, AI call ordering, human separation and final ordering.
It also corrects the two-driver draw: the original uses even draws for the
second driver, unlike the other rounds. The native harness checks 6000 bracket
cases and 6000 round-selector cases against the original machine code, including
random values, call arguments/order and memory guards. The old reference build
fails bracket case 4, providing a negative control for this behavior correction.

The selector's original 101 bytes are reproduced after relocation. Its fallback
and switch-table paths are also checked by the native harness. The other three
functions and scene reparenting remain explicitly pending despite their improved
similarity scores. Reparenting differs only in its return instruction (98.08%
under the byte auditor, 94.44% under reccmp); 12000 native cases confirm return
values, sibling rewiring, ancestor flags, failure paths and memory guards.

## Next matching work

Use `CMR2PROGRESS/nonmatching.tsv`, sorted by increasing matching score, and
the matching report from main. Group
functions by common structural problems and inspect original calls, field
widths, branch conditions and fixed-point helpers before adjusting register
allocation. Keep all implemented functions annotated, including low scores.

A later lot must preserve existing exact functions, global data, and the
differential tests relevant to its behavior. Rebuild before measuring; never
compare reports from unrelated branches or an earlier executable.

## Handoff closure

The subsequent low-score batch adds two exact functions: `Font_GetTextHeight`
(`0x0040b730`, 89 bytes) and the stage-start control reset `FUN_0047b870`
(`0x0047b870`, 254 bytes). `CGameInfo::FUN_00406010` improves from 16.95% to
58.70% reccmp similarity and corrects record labels, three-byte clearing and
car/manual bits; it remains non-exact. The new differential record reset harness
checks 6000 cases against the original and an independent complete-memory model.
The previous implementation fails case 0, providing a negative control.

At that closure, the reports superseded the integration snapshot above: 2531 of 3363
source functions are byte-exact after relocation, 832 remain (including the
14 unresolved functions), and global data has zero issues. This is 34 exact
gains over frozen `16527f6`, with no exact losses. The goal of 700 remaining
functions is pending; 132 additional exact gains are required. All 33 native differential harnesses pass on the final build. Work stops here
at the user's request for a clean handoff. See `HANDOFF.md` and
`CMR2PROGRESS/provenance.json` for the current verification and next steps.

## Resumed matching batch

After the clean handoff at `8f2727c`, the 700-remaining goal was resumed.
Three functions now reproduce their original bytes after relocation:
`FUN_00505590` (26.20% reccmp to 588 exact bytes), `RallyData_FUN_004207f0`
(45.45% to 37 exact bytes), and `FUN_00403110` (45.83% to 239 exact bytes).
The changes recover fixed-point helpers, calculation/store order and the
original field-relative record walk without changing shared headers or flags.

Audit for this batch: 2534/3363 exact, 829 remaining, zero data issues, no exact losses.
All 33 native differential harnesses pass on this executable. The cumulative
exact gain over `16527f6` was 37. After this batch, 129 additional exact
gains are required to reach 700 remaining. See `HANDOFF.md` for the current
commands, evidence and useful remaining differences.

## Final handoff

The final in-progress improvement, `Events_Add` (`0x0046e620`), moves from
20% reccmp similarity to its original 121 bytes exactly after relocation.
Direct indexing by the global event count recovers the original register
lifetimes and store order. Compiler flags and shared declarations are unchanged.

The complete audit now confirms **2535/3363 exact, 828 remaining**, zero global
data issues, and no exact losses against `34dd8e3`. The cumulative gain against
`16527f6` is 38 exact functions. All 33 native differential harnesses pass on
this final executable. Source, EXE and PDB hashes agree with the build manifest.

Work stops at the user's request for a clean handoff. The goal of 700 remaining
is paused and needs 128 more exact gains. `HANDOFF.md` records the reproduction
commands, remaining differences and discarded experiments;
`CMR2PROGRESS/provenance.json` identifies the verified build.

## Agent campaign integration (2026-10-03)

Ten agents worked in isolated worktrees (`decomp/ag1`..`ag10`), each on one file slice,
and their branches were merged through `decomp/integration-agents` (two conflicts
resolved in `Race.cpp` and `GameMenus.cpp`: the agents' byte-exact bodies won, keeping
main's renames). Main then rebuilds to **2632 byte-exact / 731 pending of 3363**,
**0 exact losses** against the 2572-function baseline, `reccmp-datacmp` 0 issues, and
all 71 native differential harnesses pass. Closures include one pending main-session
edit committed as `b06c353` (0x4853c0). The search tooling gained
`permute_batch --addresses` and `fastcmp` file-based compilation (`FASTCMP_TIMEOUT`),
which removes the wineserver pipe deadlock. Per-function evidence, including the
residual diffs of the functions left open, is in `CMR2PROGRESS/nonmatching.tsv` and the
agents' reports.

Pass 2 of the same campaign (agents 2nd/3rd waves) merges in `db4f77e`: Main rebuilds to
**2651 byte-exact / 712 pending**, 0 exact losses versus the frozen baseline and versus
pass 1, `reccmp-datacmp` 0 issues, and 71/71 differential harnesses pass. The session's
own pending `StageTiming.cpp` edit (0x480cb0 `FIX_ABS` clamp) landed as its own commit and
became the 79th closure.

Pass 3 (recovery wave after the agent stop) merges in `5e602d6`: Main rebuilds to
**2656 byte-exact / 707 pending**, 0 losses versus pass 2, 0 data issues and 71/71
differential harnesses. A successor wave resumed the stopped worktrees, preserving or
reverting their uncommitted pending edits.

## Match3 integration and debris-frame fixes (2026-10-03)

Integrated `decomp/match3` through `67f6cc7` (six new commits), keeping main's
function names and split-HUD, preview-loading, exhaust and sector-visibility
fixes. The final matching build has **2685/3363 byte-exact functions (+7),
678 pending, zero exact losses and zero global-data issues** against `fe4bca3`.
New exact entries: `0x4e48b0`, `0x5034f0`, `0x47fcb0`, `0x46e340`,
`0x476e00`, `0x469e40` and `0x483100`.

Checking the complete 36-byte frame passed to `Car_SpawnDebris` reproduced
incorrect adjacent-stack reads in `0x46a500`, `0x48fb80`, `0x48ae90` and
`0x483100`. Each now uses three contiguous vectors in the original order.
The surface-contact regression now reads all three vectors, and the new debris
regression checks the other three callers. The signed remainder expansion in
`0x406580` keeps its increment inside the negative branch, preserving the
agent's parity correction and main's exact language loader at `0x4f4b90`.

The 74-harness suite passed on the initial integration before the debris-frame
corrections. Targeted checks after those corrections passed 288 debris-call
fixtures (176 spawns), 208 surface contacts and 512 unlock-flag/model fixtures;
a separate audit compared 256 complete oriented-box constructions. The final
build, symbols, matching reports and source hashes agree. **The expanded full
76-harness suite remains pending**: work was stopped at the user's request
before running it. `build/windowed/CMR2.exe` still contains the previous
validated gameplay fixes, without this match3/debris integration; rebuild it
before testing the new changes interactively.

Pass 7 (agent wave 3) merges in the commit above: integration measures **2689 byte-exact /
674 pending**, 0 exact losses vs pass 6, 0 data issues (differential suite running at the
time of the merge; previous pass was 76/0). Main's own reports stay at the last successful
measurement until the session's in-progress `Car.h` refactor compiles again.

Pass 8 (wave-3 preservation) merges above: the stopped agents' verified pending states are
committed and integrated, bringing integration to **2704 byte-exact / 659 pending**, 0 exact
losses vs pass 7, 0 data issues, and **76/76 differential harnesses**. One behaviour
regression found by `differential_stage_sound_reset.py` (a cursor rewrite of `FUN_00418f20`
writing `pattern` into `time`) was reverted before this merge. Main's own reports remain at
the last successful measurement until the session's in-progress `Car.h` refactor compiles.

Main's reports are regenerated at **2704 byte-exact / 659 pending**, 0 data issues and
**76/76 differential harnesses** on the committed tree (the session's in-progress `Car.h`
refactor was set aside to `/home/pablo/main-session-Car.h.patch` so the tree builds).

Pass 9 (wave-4 closure batches + preserved states) merges above: Main measures
**2718 byte-exact / 645 pending**, 0 exact losses, 0 data issues and **76/76
differential harnesses**. Four functions whose exactness had drifted with the
translation-unit context (0x4f4b90, 0x425a90, 0x426810, 0x426b90) were audited:
their sources were intact and they are back in the pending list as near-miss
targets. Wave-4 closures: 0x456d90, 0x41d2b0, 0x423f30, 0x4916a0, 0x4ee460,
0x4edef0, 0x47bad0, 0x506080, 0x402f90, 0x4a8bf0 and the preserved-state extras.

match3 integration (option A): `decomp/match3`'s 14 committed commits are merged in
`bc1488a`. Two closures came in (0x468c10, 0x4930e0) plus score improvements
(0x466030 53->97%, Car_SmoothForceFeedback 46->69%). The pattern pass 7c3576e had
narrowed locals in four functions (FUN_0041e8d0, FUN_004483e0, FUN_00498620,
FUN_0048fb80), which changed behaviour; differential_ai_telemetry, _race_estimates,
_race_handler and _surface_collision caught it and those locals were restored to
`int`. Final state: **2720 byte-exact / 643 pending**, 0 losses, 0 data issues,
**76/76 differential harnesses**.

## StageTiming byte-matching continuation (2026-10-04)

The source batch following `2a6471f` closes two functions in `StageTiming.cpp`:

| Original address | Function | Byte audit before | Exact original bytes |
| --- | --- | ---: | ---: |
| `0x00490d50` | `FUN_00490d50` | 88.17% | 169 |
| `0x00508ee0` | `FUN_00508ee0` | 89.32% | 125 |

The loader preserves a local pointer to its count block while advancing the
data cursor, reproducing the original register lifetimes and loads. The level
update indexes the values relative to the returned record base, recovering the
original pointer setup. Compiler options, shared declarations and the auditor
are unchanged.

The complete rebuild and clean measurement confirm **2722/3363 byte-exact
functions, 641 pending, no exact losses, no similarity decreases in the byte
audit, and zero global-data issues** versus the frozen 2720-function base. All
**76/76 differential harnesses pass** on this executable. Source, EXE and PDB
hashes agree with the manifest; 11 functions still have unresolved operands.

Validation logs are in `scripts/work/byte-campaign/build.log`,
`measure-clean.log` and `suite-stable.log`. Experiments are local, ignored
snapshots with a `.cpp.snapshot` extension so reccmp does not scan their
annotations. The first measurement scanned the experimental copies and was
superseded by the clean run. The first suite used a subsequently missing
`/tmp` compiler link and was superseded by the full successful run with
`CMR2_TOOLS=/home/pablo/colin_mcrae_linux/tools`.

The initial targets `0x004556f0`, `0x00456250` and `0x00459630` remain pending
and unchanged. Declaration-order and expression variants did not close them.
The residual diff for `0x004556f0` still moves `test bl,bl` across the two output
stores. The constant at `0x005113c0` is confirmed as the double `10.0`, but
`0x00459630` also needs its stack/register and conversion sequence recovered.
No experimental variants or speculative constant mappings were incorporated.

## Frontend and compiler-context recovery (2026-10-04)

The next batch adds two exact functions against the frozen 2722-function build:

| Original address | Function | Byte audit before | Exact original bytes |
| --- | --- | ---: | ---: |
| `0x0044e130` | `FUN_0044e130` | 99.52% | 1784 |
| `0x004f4b90` | `FUN_004f4b90` | 81.76% | 281 |

The stage-times header uses explicit branches for the final stage and the
formatted stage number, recovering the original variadic argument setup.
The language loader's body is unchanged: reversing an equivalent branch in
`FUN_00500c80` recovers its original compiler context and stack layout. The
initializer's normalized instruction stream remains identical to the frozen
build. The condition retains its original short-circuit queries and both arms
retain their assignments.

The packed search initially found the language closure only while testing
neutral variants of other functions alongside it; keeping only improving
bodies lost that closure. The dependency was reduced to the one initializer
branch. No other packed variants, including two non-exact score improvements,
were applied. Local snapshots and search logs are in
`scripts/work/byte-campaign2`; the packed search is in
`/tmp/cmr2-byte-campaign2-packed`.

The full build and measurement confirm **2724/3363 byte-exact functions,
639 pending, no exact losses, no byte-audit similarity decreases, and zero
global-data issues** against the 2722-function base. **76/76 differential
harnesses pass** on the new binary. Compiler options and the auditor are
unchanged, all source/EXE/PDB hashes match the manifest, and the same 11
functions still contain unresolved operands. Reproduction logs are
`scripts/work/byte-campaign2/build.log`, `measure.log` and `suite.log`.

## Menu, race-order and mesh-flag byte matching (2026-10-04)

This batch adds three exact functions against the frozen 2724-function build:

| Original address | Function | Byte audit before | Exact original bytes |
| --- | --- | ---: | ---: |
| `0x004a04a0` | `Menu_ValidateCursor` | 97.30% | 77 |
| `0x0042b660` | `Car_BuildRaceOrder` | 80.00% | 140 |
| `0x004b2d90` | `SceneNode_SetMeshFlagBits` | 57.14% | 125 |

The menu cursor search computes each candidate from the current cursor and
number of attempts inside the loop. No cursor write occurs before the successful
return, so it preserves the search and restores the original load scheduling.
The race-order initializer clears the same eight bytes with one `memset` after
its count stores, recovering the original registers and instruction order.
The mesh-flag traversal saves the current sibling in `pChild` and advances `p`
through the first-child chain before following the saved sibling's next link.
This retains the original traversal and recovers its register lifetimes and
loop tests. All three instruction bodies are relocated byte-exact.

Experiments that failed to close a function were left out, including three
non-exact improvements from the packed search. Candidate snapshots and the
frozen audit are under `scripts/work/byte-campaign3`; the isolated packed search
is under `/tmp/cmr2-byte-campaign3-packed`. Sources, compiler flags and the byte
auditor used for the frozen comparison are unchanged apart from the three
function edits above.

The full rebuild and measurement confirm **2727/3363 byte-exact functions,
636 pending, no exact losses, no byte-audit similarity decreases and zero
global-data issues** against the 2724-function base. **76/76 registered
differential harnesses pass** on this final binary. This suite checks its
registered targets; the three new closures are proved by the byte audit.
The same 11 functions have unresolved operands. Source, EXE and PDB hashes
match the build manifest, and compiler flags and the byte auditor match the
frozen base. Logs are `scripts/work/byte-campaign3/build.log`, `measure.log`
and `suite.log`; `verification.json` records the closure checks.

## Network classification counter byte matching (2026-10-04)

This batch closes `FUN_0040ad20` (`0x0040ad20`, 363 original bytes), improving
its byte-audit score from **94.12% to exact** against the frozen 2727-function
build. The local classification count now receives `++g_netClassCount` directly,
replacing the separate addition and global assignment. The initializer still
calls the providers in the same order, fills the same records and unused slots,
and passes the same count and comparator to `qsort`. The direct increment
recovers the original load into EBX and its `inc` instruction.

Single-unit comparison found exactly this gain with no exact losses or
similarity decreases in NetPlayers. Other local and packed experiments in
StageObjects, Graphics, Input, StageTiming, GameInfo and NetPlayers did not close
more functions and were not integrated. In particular, the lighting variant
for `0x00463fe0` remains experimental. Testing 330 equivalent branch variants
of previously exact functions did not recover a neighbouring closure. The
lower-score packed search produced four non-exact improvements; none was
applied. These searches refine the remaining work without changing the
matching flags or annotating speculative addresses.

Frozen reports, snapshots and logs are under `scripts/work/byte-campaign4`.
The isolated packed searches are `/tmp/cmr2-byte-campaign4-context` and
`/tmp/cmr2-byte-campaign4-lower`.

The full build and measurement confirm **2728/3363 byte-exact functions,
635 pending, no exact losses, no byte-audit similarity decreases and zero
global-data issues** against the frozen 2727-function base. **76/76 registered
differential harnesses pass** on this binary. The classification closure is
proved by the byte audit; the suite covers its registered targets. Compiler
flags, compiler identity and the byte auditor are unchanged, and the source,
EXE and PDB hashes match the manifest. The same 11 functions have unresolved
operands. Reproduction logs are `scripts/work/byte-campaign4/build.log`,
`measure.log` and `suite.log`; `verification.json` records the closure checks.

## Knockout names and unlock-query branch recovery (2026-10-04)

This batch adds three byte-exact functions against the frozen 2728-function
build:

| Original address | Function | Byte audit before | Exact original bytes |
| --- | --- | ---: | ---: |
| `0x004736b0` | `Knockout_GetDriverNameForSide` | 60.99% | 209 |
| `0x00473810` | `Knockout_GetCarNameForSide` | 60.99% | 209 |
| `0x00408e30` | `RallyData_FUN_00408e30` | 86.81% | 238 |

Each knockout side now contains both its AI-name branch and its local-record
formatting branch. Removing the shared `driver` temporary and using one final
return recovers the original branches, repeated provider queries and argument
setup. Empty matches still return the blank string, and unsupported sides still
return the existing frontend buffer. Driver and car names retain their original
providers and format strings.

The unlock query defers the additional `0x12` override to a final condition for
bit 7, after the per-bit branch chain. It still queries `0x12` only when the
bit-7 override for `0` failed, retaining the call order and short-circuit behavior.
The original compiler emits that block after the other per-bit cases; this
source form recovers the complete 238-byte instruction body.

Local mesh-bound, split-time temporary, replay comparison and bitfield-layout
variants did not close other functions and were not incorporated. The accepted
bodies were compared with their isolated exact candidates before rebuilding.
Frozen reports, candidate snapshots and logs are in
`scripts/work/byte-campaign5`.

The full build and measurement confirm **2731/3363 byte-exact functions,
632 pending, no exact losses, no byte-audit similarity decreases and zero
global-data issues** against the frozen 2728-function base. **76/76 registered
differential harnesses pass** on the final binary. The three closures are
proved by the byte audit; the suite covers its registered targets. Compiler
flags, compiler identity and the byte auditor are unchanged, and source, EXE
and PDB hashes match the manifest. The same 11 functions have unresolved
operands. Logs are `scripts/work/byte-campaign5/build.log`, `measure.log`
and `suite.log`; `verification.json` records the closure checks.

## Rectangle-outline assignment order (2026-10-04)

This batch makes `FUN_0050cb30` (`0x0050cb30`) byte-exact against the
frozen 2731-function build: **82.54% to exact, 214 original bytes**.
The function draws four sides using a temporary four-short rectangle. Reordering
its initial top-side assignments and its later left-side assignments recovers
the original load registers and the placement of the constant thickness stores.
All four draw calls receive the same coordinates, colour and mode as before;
there are no new temporaries, casts, helpers or compiler options.

Isolated comparison of every audited GameInfo function found this single gain
with no exact losses or similarity decreases. Language selection, table copying,
collision-loop counters, colour-menu indexing, tyre initialization and stage-time
expression variants did not close other functions and were not integrated.
The stage-time expression experiments recovered part of the original instruction
order but still differ in the lap-count register; those candidates remain
experimental. Candidate snapshots, the frozen reports and logs are under
`scripts/work/byte-campaign6`.

The full build and measurement confirm **2732/3363 byte-exact functions,
631 pending, no exact losses, no byte-audit similarity decreases and zero
global-data issues** against the frozen 2731-function base. **76/76 registered
differential harnesses pass** on this binary. The rectangle closure is proved
by the byte audit; the suite covers its registered targets. Compiler flags,
compiler identity and the byte auditor are unchanged, and source, EXE and PDB
hashes match the manifest. The same 11 functions have unresolved operands.
Logs are `scripts/work/byte-campaign6/build.log`, `measure.log` and `suite.log`;
`verification.json` records the closure checks.

## Large menu and upright-physics closures (2026-10-04)

This batch closes two large functions against the frozen 2732-function build,
covering **8993 original bytes**:

| Original address | Function | Byte audit before | Exact original bytes |
| --- | --- | ---: | ---: |
| `0x0042fb20` | `Car_SolveUpright` | 97.51% | 5874 |
| `0x004541c0` | `FUN_004541c0` | 99.44% | 3119 |

The upright solver now assigns the angular-velocity length to the existing
`dot` temporary before fixed multiplication, and uses that temporary again for
`0x10000 - mag` before calculating damping. Both expressions retain their
original evaluation order and values. This recovers the original stack-slot
allocation for the solver and the inlined fixed-vector helpers: 46 differing
instructions disappear, making the entire 1851-instruction body exact. No
helper assembly, constants, structure layouts or compiler settings changed.
Reordering scalar or vector declarations alone did not affect the result.

The menu initializes `i` before its entries pointer in the ten-row loop.
These independent assignments retain the same pointer, count and iterations,
while recovering the original EBX/EBP allocation and instruction order through
the entry provider, heading draw and loop setup. The complete 1062-instruction
function now matches. Null-entry behavior and all draw calls remain the same.

Isolated audits of every recorded function in Car and GameMenus found exactly
these gains, with no exact losses or similarity decreases. Accepted bodies were
compared with the isolated exact snapshots before rebuilding. Frozen reports,
variant snapshots and logs are in `scripts/work/byte-campaign7`.

The full build and measurement confirm **2734/3363 byte-exact functions,
629 pending, no exact losses, no byte-audit similarity decreases and zero
global-data issues** against the frozen 2732-function base. **76/76 registered
differential harnesses pass** on the final binary. The two complete function
closures are proved by the byte audit; the suite covers its registered targets.
Compiler flags, compiler identity and the byte auditor are unchanged, and
source, EXE and PDB hashes match the manifest. The same 11 functions have
unresolved operands. A simultaneous `--windowed` runtime build replaced the first matching EXE
while the suite was executing. That run reported 21 failures caused by the
missing EXE, and is retained as `suite-interrupted.log`. To remove that race,
the same source snapshot, compiler flags, build scripts and tests were run in
`/tmp/cmr2-byte7-validation`. Its fully audited and tested matching artifacts
were then installed in `build`, after checking that the working sources still
matched the isolated manifest. The windowed artifacts are retained under
`scripts/work/byte-campaign7/windowed-build`.

Reproduction logs are `scripts/work/byte-campaign7/build-isolated.log`,
`measure-isolated.log` and `suite.log`; `verification.json` records the closure
and source/artifact checks.

## Complete scene-node rotation closure (2026-10-04)

This batch makes `SceneNode_Rotate` (`0x004ac820`) byte-exact against the frozen
2734-function build: **93.08% to exact, 4193 original bytes and 1358 instructions**.

The initial basis copy now visits x/y/z for each axis, and the final node writes
copy those components individually in the same order. The basis remains a
`FixBasis` containing three contiguous `FixVector` objects; no normalization
access depends on adjacent unrelated scalar locals. The matrix construction,
fixed-point expressions, angle order, translation and parent dirty propagation
retain their original operations.

The local pointer normalizer saves its destination in `dest` before calculating
the vector length. That source form recovers the destination pointer setup
before the inlined length calculation, including the zero-length branch and
reciprocal scaling. The right-axis expansion uses the pointer normalizer for
both output vectors; the other two expansions retain direct normalization for
the first vector. A macro parameter expresses that distinction without changing
the normalization arithmetic or adding assembly. These changes recover all
remaining loads, stores, stack slots and branch offsets in the complete function.

Isolated comparison of every audited SceneNode function found exactly this gain,
with no exact losses or similarity decreases. Pointer signature, return type,
declaration and branch variants that did not close the function were discarded.
An initial inspection of `Car_Spawn` identified matrix-copy scheduling differences;
no changes to that function were integrated. Frozen reports, candidate snapshots,
the accepted diff and logs are under `scripts/work/byte-campaign8`.

The full build and measurement confirm **2735/3363 byte-exact functions,
628 pending, no exact losses, no byte-audit similarity decreases and zero
global-data issues** against the frozen 2734-function base. **76/76 registered
differential harnesses pass**, including `differential_rotation.py`, which runs
6000 original/rebuilt rotations and checks node guards, input buffers and scratch
globals. It executes the actual rotation code without mocked callees. The complete
closure is proved by the byte audit; the remaining suite covers its registered
targets. Compiler flags, compiler identity and the byte auditor are unchanged,
and source, EXE and PDB hashes match the manifest. The same 11 functions have
unresolved operands.

Validation ran in `/tmp/cmr2-byte8-validation` to keep the matching artifacts
stable while other runtime builds could run. The verified build and reports
were installed in the main workspace after checking that its sources still
matched the isolated manifest. Reproduction logs are
`scripts/work/byte-campaign8/build-isolated.log`, `measure-isolated.log` and
`suite.log`; `verification.json` records the closure and source/artifact checks.

## Complete car-spawn closure and option coverage (2026-10-04)

This batch makes `Car_Spawn` (`0x0043c7f0`) byte-exact against the frozen
2735-function build: **72.67% to exact, 6024 original bytes and 1244 instructions**.

The model basis at `param_2 + 0x98` is now held in a local `FixMatrix *pBasis`,
used by the three existing axis getters, instead of modifying the integer
parameter. The address, getter order, destinations and subsequent matrix copies
stay the same. This recovers the original instruction scheduling through the
model setup and car-type initialization blocks, reaching 98.71% by itself.
The option-7 branch now expresses the field-`0x788` doubling as `*= 2`, which
recovers the original register allocation through that branch and the following
limit and reciprocal calculations. The full routine, including its switch
cases, setup selection and final state writes, is now exact. No assembly,
structure layout, tuning constants or compiler options changed.

Isolated comparison of every audited Car function found this single gain, with
no exact losses or similarity decreases. Component-copy and pointer variants
that did not close the routine were discarded. Experiments with row temporaries
and branch layout in the 3566-byte list function `0x0050b1c0` did not close it
and were not integrated. Frozen reports, candidate snapshots, the accepted diff
and logs are under `scripts/work/byte-campaign9`.

The existing spawn harness always returned zero for `0x004063f0`, leaving the
modified option-7 block and the option-6 height adjustment unexercised. It now
controls the queried flags and covers their four combinations over all fourteen
car types. The test expands from **288 to 5376 cases**, comparing the original
and rebuilt routines' complete heap/car state and provider/setup call traces.
Scene, model and selection providers remain controlled; this is a function-level
oracle rather than a full game session. The harness remains in the existing
76-target registry, and its tested source hash is recorded in the verification.

The full build and measurement confirm **2736/3363 byte-exact functions,
627 pending, no exact losses, no byte-audit similarity decreases and zero
global-data issues** against the frozen 2735-function base. **76/76 registered
differential harnesses pass**, including the expanded spawn oracle. The complete
function closure is proved by the byte audit; the suite covers its registered
targets. Compiler flags, compiler identity and the byte auditor are unchanged,
and source, EXE and PDB hashes match the manifest. The same 11 functions have
unresolved operands.

Validation ran in `/tmp/cmr2-byte9-validation` to keep matching artifacts stable.
The verified build and reports were installed in the workspace after checking
that its sources, test scripts and target registry still matched the isolated
snapshot. Reproduction logs are `scripts/work/byte-campaign9/build-isolated.log`,
`measure-isolated.log` and `suite.log`; `verification.json` records the closure,
source/artifact checks and spawn-harness coverage.

## Cube map rendering closure (2026-10-04)

This batch makes the six-face cube map renderer, `FUN_0049e1f0`
(`0x0049e1f0`), byte-exact: **98.05% to exact, 1823 original bytes and
565 instructions** against the frozen 2736-function build.

The pointer to the node's current transform is prepared before mesh selection.
The face matrix is local to the six-face loop. These recover the original
pointer scheduling and temporary matrix stack slots without changing arithmetic,
render calls, structures, compiler options or assembly. Isolated comparison
of every audited Graphics function found this single gain, no exact losses
and no similarity decreases. Other sound, menu, view and deformation
experiments did not close their targets and were not integrated.

The full build and measurement confirm **2737/3363 byte-exact functions,
626 pending, zero exact losses, zero byte-audit similarity decreases and
zero global-data issues**. **76/76 registered differential harnesses pass**.
The cube map routine has no direct registered harness; its closure is established
by the complete relocated byte comparison, while the suite checks its registered
targets. Compiler identity, flags, auditor and original binary hash are unchanged.
The same 11 functions have unresolved operands. Source, EXE and PDB hashes match
the manifest.

Validation ran in `/tmp/cmr2-byte10-validation`; artifacts and reports were
installed after checking the workspace sources and tests against that snapshot.
Frozen reports, candidates, accepted diff and full-unit verification are under
`scripts/work/byte-campaign10`. Reproduction logs are `build-isolated.log`,
`measure-isolated.log` and `suite.log`; `verification.json` records the full gate.

## Ten-agent campaign 11 (2026-10-05)

Ten agents worked in isolated snapshots (`/home/pablo/cmr2-campaign11/agN`,
outside the repository) with exclusive ownership of one translation unit each.
The frozen base was the live workspace (2737/3363 exact, 626 pending, 11
unresolved operands, zero data issues, 76/76 harnesses), snapshotted before any
experiment. Integration ran in `/home/pablo/cmr2-campaign11/integration` with
`run_batch.sh` (reset → patches → build → measure → prepare_fastcmp → full
differential suite → gate) and was installed with `install_validated.py`.

Batches 1, B, C and D bring the matching build to **2744/3363 byte-exact
functions, 619 pending, zero exact losses, zero byte-audit similarity decreases,
zero global-data issues, the same 11 unresolved operands and 76/76 differential
harnesses**. Source, EXE and PDB hashes match `build/manifest.json`; compiler
flags, compiler identity and the byte auditor are unchanged.

| Original address | Function | Original bytes | Byte audit before | Batch |
| --- | --- | ---: | ---: | --- |
| `0x00485860` | `FUN_00485860` | 2917 | 90,36% | 1 |
| `0x004984b0` | `CarShadow_SetLevel` | 177 | 85,45% | B |
| `0x004ac200` | `SceneNode_Attach` | 87 | 96,77% | B |
| `0x004b07a0` | `Particle_Spawn` | 2456 | 94,61% | B |
| `0x004b6340` | `Graphics_GetTriangleHeight` | 1199 | 95,50% | B |
| `0x00479360` | `FUN_00479360` | 4094 | 88,36% | C |
| `0x0047a3d0` | `FUN_0047a3d0` | 823 | 83,37% | D |

Causes recovered: the skid-segment walk uses a signed `char` car index and an
`i > 0xff` clamp, and the two contact vectors take the original stack slots
(renaming `pos` to `matrixPos`); `CarShadow_SetLevel` builds its colour arrays
with aggregate initializers and `FixMul(...) >> 16`; `SceneNode_Attach` walks the
first-child chain with a `for` loop; `Particle_Spawn` selects the free slot in a
`for` loop and copies the source vector component-wise; the triangle-height
helper normalizes `vertices[1]-vertices[0]` before `vertices[2]-vertices[0]`;
`FUN_00479360` and `FUN_0047a3d0` were recovered with the coordinated
`Sound_SetPan` ABI change (see below).

### Coordinated ABI change: Sound_SetPan

The original call sites mask the second argument with `and eax,0xffff` before
calling `Sound_SetPan`, which only happens when the parameter is 32-bit and the
argument expression is 16-bit. The declaration and definition were changed from
`unsigned short pan` to `int pan` in `Sound.cpp` (declaration and definition),
`Race.cpp`, `RallyData.cpp`, `StageObjects.cpp` and `SurfaceTypes.cpp`. The
definition body is byte-identical; the mangled symbol changes for every caller,
so the change was integrated atomically. The batch gate shows a new exact
function (`0x479360`) and no losses.

### Scaffolding fixes

- The isolated tree hardlinked `build/manifest.json` to the workspace;
  `build.py` rewrites it in place, which clobbered the main manifest through the
  shared inode. The runner now removes it before building and the installer
  breaks hardlinks before copying artifacts.
- `reccmp-build.yml`'s `project:` pointed at the main checkout, so reccmp read
  annotations from `main` while measuring the isolated tree. With patched sources
  this dropped 172 function entities and failed 20 differential harnesses. The
  runner now retargets `project:` at the isolated tree; verified (3367 functions,
  all entities restored).

### Rejected work

`StageTiming.cpp`'s `0x459630` was closed by the agent only by adding
`__asm mov eax, i` to the existing fixed-point helper. Adding assembly to force
the result is not allowed, so that hunk was rejected and the function remains
pending. Other experimental variants that did not close a function were not
integrated.

### Batch E (2026-10-05)

A later wave salvaged four more closures from the agent snapshots and integrated
them with batch E, bringing the build to **2747/3363 exact, 616 pending**, no
losses, no similarity decreases, zero data issues and 76/76 harnesses:

| Original address | Function | Original bytes | Byte audit before |
| --- | --- | ---: | ---: |
| `0x00410100` | `FUN_00410100` | 2911 | 88,21% |
| `0x00478f50` | `FUN_00478f50` | 845 | 99,64% |
| `0x004bc290` | `FUN_004bc290` | 305 | 98,11% |

A coordinated change of `FUN_0040bd60`'s second parameter to `int` (definition
and all seven declarations) made `FUN_004ea510` (148 bytes) exact but regressed
`FUN_0050f1d0` from exact to 88,24%: one of its call sites requires the 16-bit
parameter. The change was rejected; `0x004ea510` stays pending until the
per-call-site argument shapes are understood.

Waves continued to be interrupted by provider/session outages; each agent's work
survives in its isolated snapshot and is salvaged with `unitverify` before
integration.

### Batch F (2026-10-05)

`Car.cpp`'s suspension and ride-height passes now reproduce their original
bytes, taking the build to **2749/3363 exact, 614 pending**, no losses, no
similarity decreases, zero data issues and 76/76 harnesses:

| Original address | Function | Original bytes | Byte audit before |
| --- | --- | ---: | ---: |
| `0x00443bf0` | `Car_UpdateSuspensionPass` | 178 | 96,97% |
| `0x0043dff0` | `Car_SetRideHeight` | 197 | 95,65% |

Both loops now index typed fields (`corners[i].y` / `cornerHeight[i]`, and
`field_0x9e8[0x20]` / `wheel0x988` / `field_0x978` by index) instead of byte
offsets, which recovers the original base/index SIB addressing. The same patch
improves several other Car functions (for example `Car_UpdateTyreForces` from
64,36% to 74,26%) without regressing any exact function.

### Batch G (2026-10-05)

Two more near-miss closures land with batch G, taking the build to
**2751/3363 exact, 612 pending**, no losses, no similarity decreases, zero data
issues and 76/76 harnesses:

| Original address | Function | Original bytes | Byte audit before |
| --- | --- | ---: | ---: |
| `0x00407b10` | `Knockout_PropagateWinners` | 779 | 98,25% |
| `0x004d8ed0` | `FUN_004d8ed0` | 1405 | 98,66% |

The knockout bracket final word is now written with the same bitfield swap
temporary used by the quarter/semi ordering loops, which stops MSVC6 from
common-subexpression-eliminating the mask and recovers the original 779-byte
body. The frontend screen uses a single colour local for item and line (the
original does not coalesce two), recovering the original stack slots.

A `FUN_0040bd60` declaration change to `int` was reverted before this batch: the
call site in `FUN_0050f1d0` has no width adjustment and needs the 16-bit
parameter, so the earlier `FUN_004ea510` closure cannot be integrated without
regressing an exact function. That function stays pending.

### Batch J (2026-10-05)

`FUN_00449ce0` (424 bytes) becomes exact: the `maxLen` ternary was lazily
evaluated inside the surrounding `&&`, while the original materialises `resX`
before the chat-line-length test; an explicit `if/else` recovers it. The batch
also improves `FUN_0044d960` to 99,71% by duplicating the item-position
assignment in both arms of the `g_unk0x005413f8` branch. Build: **2753/3363
exact, 610 pending**, no losses, no similarity decreases, zero data issues,
76/76 harnesses.

An earlier attempt in this campaign (batch I) added the Graphics operand work
but changed the translation-unit context enough to drop three already-accepted
closures (`0x4b07a0`, `0x4b6340`, `0x4bc290`). The batch gate originally compared
only against the frozen base, so it did not flag the regression. `verify_batch.py`
now measures exact losses and similarity decreases against the **last installed**
report, and the Graphics changes were rejected.

### Batches K–M (2026-10-05)

Three more closures land, taking the build to **2756/3363 exact, 607 pending**,
no losses, no similarity decreases, zero data issues and 76/76 harnesses:

| Original address | Function | Original bytes | Byte audit before |
| --- | --- | ---: | ---: |
| `0x004fdb10` | `FUN_004fdb10` | 1818 | 52,52% |
| `0x004ab7e0` | `Mesh_Free` | 191 | 86,24% |
| `0x0049fe30` | `CInput::DInputCreateDevice` | 283 | 74,44% |

The frontend menu row helper had been factored into inline helpers that evaluate
`GetTextString` once; the original expands the label/choice inline (two calls),
computes the per-case scale and issues `Sprite_Queue` inside each colour branch,
so the row draw is now a macro used by both menu functions. `Mesh_Free` and
`DInputCreateDevice` recover the original local/argument lifetimes.

The same waves improved many large functions without closing them, for example
`FUN_0040c610` (90,19% to 99,20%), `Mesh_CloneInto` (61,94% to 76,92%),
`FUN_004fd480` (84,88% to 90,01%) and `FUN_004494db0`-family physics. Changes
that only reorder instructions without a closure were not integrated when they
carried a behaviour risk; the near-miss functions in SceneNode, StageObjects,
GameMenus and FixedPoint are documented MSVC6 scheduler/register-allocation ties
that resist source-level perturbation.

### Batches N–O (2026-10-05)

| Original address | Function | Original bytes | Byte audit before |
| --- | --- | ---: | ---: |
| `0x0049d3f0` | `FUN_0049d3f0` | 1360 | 90,01% |
| `0x00489750` | `Collision_CarVsBox` | 974 | 98,73% |

`FUN_0049d3f0` reorders the `D3DVIEWPORT7` fields so MSVC6 does not reuse the
`stosd` zero register, and indexes the relight loop instead of walking a
pointer. `Collision_CarVsBox` types its four-point loop index as `short`, which
flips the original base/index SIB. Build: **2758/3363 exact, 605 pending**, no
losses, no similarity decreases, zero data issues, 76/76 harnesses.

One suite pass reported two harness timeouts while several agents were
compiling; the rerun on an idle machine passed all 76, confirming the timeouts
were load-induced, not behavioural. Later batches run the suite with an idle
machine and a larger per-harness timeout.

