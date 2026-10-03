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
