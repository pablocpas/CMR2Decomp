# Readability cleanup, preserving matching

This batch replaces established memory layouts with typed accesses and names
recovered from callers. It does not assign meanings to fields whose role is
still uncertain. The recovered names describe behavior, not original source
identifiers.

## Typed car accesses

The car-list passes now take `Car *` and select entries with pointer or array
indexing. Every executable use of the `0xc24` car stride is gone; the literal
remains in the layout assertion and the documentation of `Car`.

Known wheel loads, slip values, surfaces, wheel offsets and scene nodes use
their existing members. Record clearing and force-array clearing use `sizeof`.
Wheel rotation is a `short[8][4]` table instead of a four-member placeholder
struct accessed through casts. The view-order function uses distinct pointers
for player records and car records.

Across implementation files, the raw dereference pattern recorded in
[readability-matching.json](readability-matching.json) falls from 2296 to 2239
(57 replacements). This is a syntactic measure; it includes other cast
dereferences and is not a count of all remaining unexplained layouts.

## Recovered names

Twenty-eight identifiers containing `unk`/`Unk` now have descriptive names.
The complete rename map, including additional member and callback names, is in
[readability-matching.json](readability-matching.json).

| Old name or group | Recovered role | Evidence |
| --- | --- | --- |
| `Unk0049c2c0` | `CallbackStateMachine` | Initializes slot count, record array, update/render callback table and transition rules. |
| `Unk00817d98` | `CallbackStateRecord` | First DWORD contains packed state; second counts timer steps and resets on a transition. |
| `Unk0x005a1820` | `SessionPlayerRecord` | Session enumeration copies short/long names, stores DirectPlay player id and marks occupied slots. |
| Frontend/secondary machine globals and rule tables | Callback machines, records, initialization flags and state rules | Frontend and secondary dispatchers initialize and run the corresponding objects. |
| `m_unk0x00593ba4`, `m_unk0x00593ba8`, `m_unk0x00593cac` | Current callback record, cursor and skip-render flag | Timer loop advances the cursor; render dispatcher consumes the skip flag. |
| `g_unk0x0053a230`, `g_unk0x0053ac48` | Wheel rotation and spinning tables | Wheel integration writes them; render transforms and mesh appearance read them. |
| Car-step context globals | Car buffer, order and count | `Car_RunStepPasses` stores its three arguments. |
| `unk_isJoystick` | `supportsForceFeedback` | Set according to `DIDC_FORCEFEEDBACK`, not joystick presence. |
| Logger fields | File name and logging-disabled flag | `OpenLogFile` copies the name; `LogToFile` checks the flag before writing. |
| `unknownGraphicsOptions` | `graphicsOptions` | Graphics-option accessors manipulate its packed bits. |

All function addresses and global address identities are retained. Renamed
class globals also carry annotations on their qualified definitions so the
single-TU comparator can resolve them before a full rebuild.

## Layout and validation

`LayoutChecks.cpp` asserts the Win32 sizes and critical member offsets of the
car, callback and session-player records. It is included in both build paths.
The checks live in a separate translation unit: adding declarations and
`stddef.h` to `Game.h` changed MSVC6 code generation in unrelated functions.

The complete MSVC6 rebuild and byte audit preserve all 2922 exact functions
out of 3364. Perfect match remains 56.10%, fuzzy match remains 94.49%, and
global data comparison reports zero issues. All 103 registered differential
harnesses pass with zero failures. The annotation check and `git diff --check`
also pass.

The stored report had a 98.8968% score for the already non-exact
`FrontendMenu_DrawRallyReport` (0x004dbd80); the new audit records 98.5075%.
Recompiling a saved copy of the source from before this batch produces the
same 98.5075% score and identical relocated instructions as the cleaned
version. This reproducibility difference is recorded separately in the JSON;
it is not a change to that function caused by this cleanup.

Remaining work includes raw layouts in attached-part, replay/setup and stage
object records, and offset-named fields whose semantics still need tracing.
Each later batch should establish the layout, preserve access width and
signedness, and repeat the full matching and behavior checks.
