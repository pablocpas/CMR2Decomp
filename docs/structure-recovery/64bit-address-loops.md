# Full-width table traversal limits

Baseline: 18c29a1. This batch removes 79 pointer/integer diagnostics in 58
functions: the reviewed compatibility inventory falls from 435 to 356.
All 3364 s/fz/x rows are unchanged, including all 2924 byte-exact functions
and the corrected relative-velocity deformation hull.

## Evidence and representation

The selected operands were diagnosed by the native compiler as pointers;
ordinary integer comparisons are not included. Array cursors remain pointers.
Their address comparisons now use INT_PTR, which retains the full native
address and the original signed comparison. Comparing pointers directly would
select unsigned address ordering in MSVC6. No address is stored in a 32-bit
integer by these changed comparisons. This does not recover the layouts of
opaque records or claim that every function containing a changed limit is
otherwise ready for 64 bits.

Several limits also stop depending on the global that happened to follow an
array in the original executable:

| Owned record/table | Original layout and limit | Evidence |
| --- | --- | --- |
| ControlsDeviceSelection | ushort slots[8] at 0; lastDevice at 0x10 | Entry reads eight slot mappings, exit writes eight mappings; both now stop at slots+8. The last-device member is retained. |
| NetPlayer[8] | size 0x80; flags at 4; total 0x400 | ClearReadyFlags and BlockStatisticsReception advance one record between flag words. The numeric terminal address is array address + sizeof(array) + offsetof(flags), retaining the original biased cursor. No rank-table adjacency is needed. |
| ChampionshipTables | eight accumulated point words at 0x38, order bytes at 0x58 | AddChampionshipPoints stops at points+8; the full wrapper's scoring, round bytes, ordering and tie handling are exercised. |
| RallyStageTables | times[16] at 0x20; penalties at 0x60; size 0x80 | ResetStageResults stops at times+16 and preserves all 16 order, position, penalty and tie-break stores. |
| RallyOverallTables | times[16] at 0x20; count at 0x60; size 0x64 | ResetOverallPlayerTimes stops at times+16, preserving count and positions. |
| CarSceneRecord[16] | bodyNode at 4, rootNode at 8, stride 0x24 | LoadStageCarsAndEffects clears those two members in all 16 records. Its terminal numeric address uses offsetof(bodyNode); on native builds the member bias is 8 and stride 0x48. |
| ReplayLevelState | levels[16] at 0, bufferCount at 0x40, pending[8] at 0x44 | ResetPairedCarValues writes the 16 levels and only the first eight separate pending flags; bufferCount, the state pending array and remaining flags are retained. The existing interior cursor's comparison is widened. |

These are existing runtime records. No serialized layout was enlarged and no
new record was invented. NetPlayer now has central sizeof and eleven member
offset checks, including its flags, statistics, split/time arrays and footer.
Other record checks already existed. Unknown members retain their existing
names. Interior member/word cursors retain the original arithmetic; widening
these comparisons is not a claim that all cross-subobject arithmetic has been
removed.

## Validation

The registered whole-function x86 harness executes 592 cases against both the
original executable and this rebuild. It checks independent free/volume/flag/
time/controller/championship models, complete heap contents, owned table
contents, surrounding record fields, provider order and stdcall/callee-saved
registers. Native execution checks 512 scenarios with pointers above 4 GB,
actual recovered headers and full record guards under ASan/UBSan. Allocation,
release, audio, controller and championship sort leaves are controlled.

The complete first-model-failure path of LoadStageCarsAndEffects is exercised:
all six pointer tables and all 16 body/root pairs are checked, along with the
allocation/setup call sequence and failure return. Its later model/render
paths still contain unrelated raw accesses and are outside this fixture's
coverage. Native controller fixtures use the actual ControllerData and
ControlsDeviceSelection declarations extracted from this checkout.

The full build, measurement, changed-TU gate and all 123 registered harnesses
pass (zero failures). Accompanying matching, focused,
native, gate and suite artifacts record the results.

## Remaining work

The raw-access inventory remains 1304: this batch primarily fixes truncating
loop conditions, not field layouts. 356 pointer/integer diagnostics remain.
RallyData_RestoreAllCarRaceRecords still holds its record address in an unsigned
int and needs a typed traversal. StageObject_InitCarSceneTables still has a
second archive traversal over the fixed StageBlock byte storage; its embedded
GenericFile pointers require recovering the whole storage layout first.
Other opaque raw accesses in functions changed here remain explicit work.

The compatibility projection uses the same external port baseline ee06f54
and retains all compiler errors: 370 additional platform/header errors remain,
unchanged from the preceding batch. The zero active native layout assertions
and reduced warning count do not mean the whole native game builds. Disk-backed
relocation records still require separate file/runtime forms in OpenCMR2.
