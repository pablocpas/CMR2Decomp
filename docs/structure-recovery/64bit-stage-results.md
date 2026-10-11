# Finland results and category best records

Baseline: `80a16a0`. Validation uses an isolated matching checkout, excluding
the existing weather-texture edit in RallyData.cpp. That edit stays outside
this batch in both repositories.

## Reproduced crash

The native probe enters Single Rally, Finland stage 1, drives 375 simulation
steps and requests the standard finish/result transition at simulation tick
500. Enter advances the actual result headers and Continue menu. Before the
fix, GDB stops in `CGenericFileLoader::StrLowerPolish` with `str == 0x1e`.
The caller is `GameMenus_DrawTextRow`, reached by `GameMenu_DrawSplitTimes`.
The saved backtrace is in `64bit-stage-results-backtrace.txt`.

The row function takes variadic text pointers but reads them by incrementing
`&pText`, assuming the Win32 stack calling convention. Native x64 passes some
arguments in registers: that read obtains the number 30 instead of another
string pointer. It now uses `va_start`, `va_arg(char *)` and `va_end`.
All 25 call sites supply `(char *)NULL` as the terminator, instead of passing
an integer through an untyped argument slot. Text mutation, formatting,
separator geometry and draw order are preserved.

The finish regression is checked in OpenCMR2's `stage_probe` and
`runtime_scenarios.py`. It uses the existing timeout-finish record function
and callback rule to enter the game's real result sequence; it does not drive
the entire route to cross the finish line. It automatically presses Enter,
requires stage index 1 and at least 100 driving steps there. Both Single Rally
and Championship now continue to Finland stage 2, with 778 driving steps in
the 1,500-tick scenario. The probe uses real game assets and headless rendering.

## Pointer and record family

`RallyData_GetAvailableCategorySaveRecord` returns a view beginning at +0x84
in a pointer-free 0x650-byte saved profile. The five merge functions index
these best records; four previously accepted the view as an `int`, and the
arcade merger stored it in an `int`. Both arcade consumers also converted a
current-record pointer to `int` before adding +0x20.

`CategoryBestResults` describes the observed 0x504-byte best-result region:

| Offset | Entries | Selection |
| --- | --- | --- |
| 0x000 | Three 16-byte player results | Difficulty |
| 0x030 | Eight by three 12-byte rally results | Country, difficulty |
| 0x150 | Eight by twelve 8-byte stage times | Country, stage |
| 0x450 | Three by three 12-byte arcade results | Group, difficulty |
| 0x4bc | Three by three 8-byte arcade times | Group, stage |

Its end coincides with profile +0x588, where unrelated profile settings start.
It contains integer data only, so its layout is identical on 32 and 64 bits.
Central checks fix the record sizes and every array boundary. The unused fourth
word of a player entry remains `field_0xc`; no meaning is inferred for it.

All four merge declarations and calls now share typed pointer prototypes.
All five destination address calculations use the appropriate arrays, and
the two arcade source calculations preserve the full pointer. Existing local
bitfield overlays and integer-word merge helpers retain their original masks
and order. The byte-view producer and other profile display/code-generation
consumers remain separate migration work; this batch covers all five merge
entry points and their callers, rather than claiming the whole saved profile
has been recovered.

The result setup's ghost name/total and the current arcade-time getter use the
existing GameInfo record fields. Country, stage and difficulty globals become
`g_frontendRecordCountry`, `g_frontendRecordStage` and
`g_frontendRecordDifficulty`, supported by their selection producers. Renaming
the adjacent arcade-group global moved an unrelated exact language-selection
function from 100% to 70% in MSVC6; automatic subset isolation retained the
three other names and left that name pending.

## Validation

- Full MSVC6 build and changed-TU gate pass. All 3,364 functions retain their
  prior byte/fuzzy scores and all 2,924 exact functions remain exact.
  The row function improves from 85.71% to 86.77% byte similarity and from
  88.89% to 89.95% fuzzy similarity.
- The new original/rebuilt row harness checks 54 cases against a text/geometry
  model: empty/one/many strings, repeated pointers, Polish conversion, complete
  poisoned heap, scratch-buffer suffix, separators and the cdecl ABI. It also
  checks 176 result-setup prefixes through ghost name/split initialization.
- The record harness checks 4,032 whole-function cases against the original and
  an independent packed-word model: every country, stage, difficulty/group,
  unused/equal/better/worse entries, absent direct providers, full heap, dirty
  flags, minimum updates and the arcade getter.
- The native ASan/UBSan fixture executes the actual five merge bodies and row
  body with production records and addresses above 4 GiB. Its 960 record cases
  compare the entire guarded heap and all dirty flags; 18 row cases check text
  pointers and separator positions. Restoring the old argument walk or the
  truncated arcade pointer makes the sanitizer tests fail.
- The complete differential suite has 134 registered harnesses. OpenCMR2's
  22 CTests include physics parity at six cadences and the six real runtime
  scenarios (host, arcade time trial, replay, map and both Finland results).
  All six also pass with an isolated native RallyData object built from the
  committed port source, excluding the pending weather edit.

The stable inventory moves from 751 to 749 syntactic raw accesses, from 9,813
to 9,788 unknown occurrences and from 1,212 to 1,209 distinct unknown spellings.
One explicit unnamed field is added for the player record's unproven word.
The detector counts raw dereferences; it does not count every eliminated
pointer truncation or address calculation. These counts are a migration work
list, not a percentage of verified 64-bit compatibility.
