# Native race callback storage and client pointer flow

Baseline: 2e37a49. The reviewed compatibility inventory falls from 322 to 286
pointer/integer diagnostics: 36 removed across 18 functions. The raw-access
inventory falls from 1288 to 1231. All 3364 s/fz/x rows remain identical and
2924 functions remain byte-exact, including CarDamage_BuildRelativeVelocityHull.
The original 594-conversion inventory has now lost 308 entries.

## Ownership and layout evidence

| Record / original Win32 offset | Field | Evidence |
| --- | --- | --- |
| CallbackStateMachine +0 | BYTE count | InitializeCallbackStateMachine sets the slot count; update/render/timer routines iterate that count. |
| CallbackStateMachine +4 | CallbackStateRecord* records | The initializer stores the supplied record span; callbacks inspect its state words and the timer routine updates its elapsed ticks. |
| CallbackStateMachine +8 | StateCallbackPair* callbacks | Update/render routines index the paired function pointers using the record's state byte. |
| CallbackStateMachine +0xc | unsigned int* rules | Rule promotion searches packed numeric rules terminated by 0xffffffff. |
| RaceCallbackStorage +0 | CallbackStateMachine machine | UpdateInRaceCallbackMachine initializes and dispatches the machine at original address 0x537dd0. |
| RaceCallbackStorage +0x10 | CallbackStateRecord records[2] | The same producer initializes one or two records at 0x537de0 and passes that span to the machine initializer. |
| CallbackStateRecord +0 / +4 | packedState / elapsedTicks | Initialization, rule promotion and AdvanceCallbackStateTimers establish the packed state and timer interpretation. |
| StateCallbackPair +0 / +4 | update / render | The two dispatch routines call their respective member with the machine and slot index. |
| g_unk0x00537f0c slots 0..3 | bounded numeric UI states | StageUI writes values 1..8; the result menu reads player slots as numeric labels. |
| g_unk0x00537f0c slot 4 | numeric iteration cursor | Race_UpdatePlayerViewAndDrivenCars initializes and advances this slot as an index. |
| g_unk0x00537f0c slot 5 | saved machine address | Race_EnterCurrentModeScene stores its first argument; GameMenus recovers the machine to promote its records. |

These callback records are runtime state, constructed by the game's initializer;
the owned storage is not deserialized from a file. CallbackState.h carries the
previous core definitions, with their original order and representation.
RaceCallbackStorage.h defines the recovered owner independently. The formerly
unknown byte-array global is now g_raceCallbackStorage, with its GLOBAL annotation
still attached to the definition. One unique unknown identifier is removed;
unproven names and the 1486 unnamed field declarations remain unchanged.

Original sizeof/offsetof checks cover every touched record member, including
the union views and explicit padding, under CMR2_LAYOUT_CHECK. Win32 machine,
owner and callback-pair sizes remain 16, 32 and 8. On the tested native ABI they
are 32, 48 and 16; records/callbacks/rules move to machine offsets 8/16/24 and
the owner's record array moves to 32. CallbackStateRecord stays pointer-free
and eight bytes wide; CallbackIndex stays four bytes wide.

## Whole-family changes and preserved representations

Twelve callback/teardown entry points now accept CallbackStateMachine* instead
of transporting their first argument through int or an untyped byte span.
Definitions, callers and repeated declarations agree and the complete game
links. The callback tables' existing casts and numeric second-argument
representations are retained; this batch does not claim uniform numeric
callback signatures.

All raw reads of the pointer at StageUI_GetRaceResultTable() +4 now use the
recovered records member. Its consumers in Car, Race, GameInfo, NetPlayers,
RallyData, StageObjects, StageTiming and TrackCollision use record members.
The original char/BYTE/full-word interpretation and number/order of getter
calls are retained. Replay reads of the rule byte use the packed word shifted
by 16, with the original unsigned semantics.

The getter retains its BYTE* interface for existing byte-oriented clients.
Race_HandleStageReplayViewTransitions retains a byte cursor and eight-byte
stride within the pointer-free CallbackStateRecord span, but obtains that
span through the machine member. This does not depend on a native pointer
remaining at offset four. Other callback families and unrelated raw accesses
are still outside this batch.

The mixed numeric/address scratch array uses ptrdiff_t in all declarations.
MSVC6 defines it as signed int, preserving the original code, while the tested
native ABI provides pointer width. An unconditional width check rejects an
ABI where it cannot hold a pointer. Its numeric accessor intentionally returns
int; its real caller accesses player-state slots, never the pointer slot.
The scratch array remains unnamed because its combined role does not justify
a more specific single name.

Adding the owned-storage declaration to the widely used core header changed
MSVC6 register allocation in an unrelated GameInfo function. Keeping the owner
in its separate header restores every previous score. INT_PTR also changed
allocation in GameMenu_SetupStageResults; ptrdiff_t preserves MSVC6's original
int type and the complete native address. Neither experiment's regression is
included in the final source. No assembly, pragma or volatile workaround was
introduced.

## Validation and limits

The new registered differential harness executes 1920 cases across sixteen
actual original/rebuilt client bodies, including the owned-storage producer.
It varies one/two records, slot index, packed state, poisoned unrelated bytes,
network status and event values. It checks guarded records/heaps, normalized
global writes and their widths, controlled provider call identities/order/
arguments, stack cleanup and preserved registers. State queries and the
producer's storage contract also have independent models. External scene,
network, rendering and loading leaves are controlled; this is not exhaustive
execution of every race mode or every provider's body.

The native fixture uses the actual production headers and twelve extracted
production bodies. Its 290 ASan/UBSan scenarios cover machine initialization,
update/render dispatch, timers, one/two-slot producer initialization with
varied padding, state queries, delayed fades and saving/recovering the mixed
table's pointer. Machine storage, record spans, callback tables and callback
addresses are verified above 4 GB. Promotion and external leaves are controlled;
broader original/rebuilt coverage remains in the registered suite.

Full MSVC6 build, measurement, metadata preparation and the 29-TU matching gate
pass. The complete differential suite passes all 125 harnesses with zero
failures. All scores, matching logs, native results and inventories accompany
this document. The reviewed recipe in scripts/migrations/race-state-pointers.json
replays 57 transformations across fifteen source/header files and reproduces
this final source byte for byte from the baseline.

The compatibility projection retains all compiler errors: 367 additional
errors remain. Its merge now preserves all port-only includes; restoring
Movie.h explains the three removed diagnostics, rather than a decomp behavior
change. No new normalized compiler errors appear. The external port itself was
not edited. This batch does not establish a whole-game native build: 286
pointer/integer diagnostics and 1231 raw accesses remain, alongside disk
relocation and platform integration work.
