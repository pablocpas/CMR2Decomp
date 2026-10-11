# Session descriptors, ghost meshes and replay storage

Baseline: `3bf43d4`. The MSVC6 validation checkout excludes the pre-existing
weather-texture changes in `RallyData.cpp`; the ghost-field rename is included.
Original function/global addresses and the order of expressions are retained.

## Reproduced crashes

The arcade time-trial fixture first faults in `Mesh_Rebuild` after ghost setup.
`StageTiming_SetNodeMeshAlpha` writes mesh flags at the Win32 offset `0x30`.
That offset contains `Mesh::pTriangles` in native x64. The consumer now writes
`pMesh->flags`, preserving the original flags operation and the triangle pointer.

After fixing that corruption, the same fixture faults in `malloc_usable_size`
from `Disk_ReleaseBuffer`. The ghost loader compares an original-format file
against `sizeof(ReplayStream)`, rejects it, and frees a borrowed archive member.
OpenCMR2 now converts the original disk header into an independently allocated
native stream and only frees buffers owned by the standalone file loader.

The host fixture faults in `strnlen` during session setup. The password store
at Win32 offset `0x34` overwrites half the native session-name pointer. The
whole session descriptor and its twenty-entry enumeration array are now typed.
The browser's password parameter also carries a `char *` all the way from
`FrontendMenu_UpdateNetworkSessionBrowser`, eliminating its intermediate `int`.

## Shared declarations and layout evidence

| Record / member | Win32 | Native x64 | Producer / consumer |
| --- | ---: | ---: | --- |
| `DPSESSIONDESC2` / port `NetSessionDesc` | 0x50 | 0x58 | Network descriptor creation, enumeration, setters and getters |
| Session name pointer | 0x30 | 0x30 | Name setter and session provider |
| Session password pointer | 0x34 | 0x38 | Password setter, browser and session provider |
| Enumeration storage | 20 descriptors | 20 native descriptors | Reset, callback copy, join selection |
| `ReplayStream` | 0x110 | 0x138 | Allocation, recording, playback, load and save |
| `ReplayStream::steeringModeOption` | DWORD at 0 | DWORD at 0 | Numeric controller option at +0x124, input decoder's steering mode |
| `ReplayEvent` | 0x6 | 0x6 | Run-length event recording/playback |
| `ReplayInputLane` | 0x114c | 0x114c | Input replay and car/part snapshots |
| `ReplayStateLane` | 0x5c | 0x5c | State and pose replay |
| `ReplayPose` | 0x8c | 0x8c | Two fixed-point matrices and velocity |
| `ReplaySample` | 0x10 | 0x10 | Packed pose encoder / decoder |

`ReplayStream.h` is shared by recording, timing and race consumers. All header
fields have central Win32 size/offset checks. The sample bitfields retain their
original representation and size check. Runtime queries, ghost visibility,
camera events and periodic pose recording use the shared fields. The pose
destination is a `ReplaySample *` through the encoder wrapper, with no integer
address parameter.

The first replay word was declared as `void *pInput`, but its producer returns
the numeric controller option and its consumer only selects a steering mode.
It is now `DWORD steeringModeOption`. It must survive disk round trips as a
number, including nonzero values other than one. This also means the original
playing/recording offsets +4/+0xc remain valid after the corrected declaration;
their replacement with fields is not evidence that they caused the reported
post-finish replay loop.

## Original file representation in OpenCMR2

The generated disk layout describes the original 0x110-byte header independently
of the native struct. `ReplayDisk_Read` checks dimensions and exact payload
length, copies scalar fields and relocates owned native arrays. It leaves lane
cursors unset until the existing lane-selection code chooses them.
`ReplayDisk_Write` exports the original header and pointer-free payload, clears
serialized runtime array addresses and preserves the numeric steering option.

Input replay snapshots retain the original 0x4d0-byte part and 0xc24-byte car
slots. Native capture packs those layouts; restore copies their scalar runs
into live records, preserving native node, emitter and geometry pointers.
The 32-bit restore path retains the existing original implementation.

## Names supported by evidence

| Old symbol | Recovered name | Evidence |
| --- | --- | --- |
| `g_unk0x005a0068` | `g_networkSession` | Descriptor passed to session operations |
| `CGameInfo::m_unk0x0059fa20` | `m_enumeratedSessions` | Twenty session callback copies and join consumers |
| `g_unk0x00588758` | `g_activeReplayStream` | Replay allocation/loading, reset and swap |
| `g_unk0x0058875c` | `g_ghostCar` | Ghost activation and restart |
| `Car::field_0xc0c` | `isGhost` | Ghost activation, physics and effects exclusion |
| `StageTiming_InstallCarPartModelGeometry` | `Replay_RecordCarPoseSample` | Calls the car pose encoder with a sample destination |

Unresolved replay flags, frame payload words and numeric vertex buffers keep
their existing unknown names. Public byte-buffer interfaces retain casts at
the boundary; they do not add displacement-based runtime field access.

## Validation and practical limits

- Full MSVC6 build, measure and changed-TU gate: 3364 functions compared,
  2924 byte-exact retained, no s/fz/x regression.
- `differential_crash_runtime_records.py`: 665 original/rebuilt cases with
  independent memory models, complete changed bodies, provider traces,
  destination checks and stack cleanup, including enumeration errors and
  inactive providers.
- `check_64bit_crash_runtime_records.py`: production native declarations and
  actual runtime bodies with pointers above 4 GiB; ASan/UBSan/LSan pass.
  Five negative controls detect ghost offset corruption, incorrect replay
  reset, the old password offset, browser pointer truncation and sample
  pointer truncation.
- Complete registered differential suite: 131 harnesses, zero failures.
- OpenCMR2 native build and all 21 CTests pass, including disk replay
  round trips, malformed-file cases, and 500 original-identical physics steps
  at each of six render cadences.
- Real headless arcade time trial (mode 7, two cars) and local host (mode 8)
  each complete 500 ticks and 375 driving ticks. The recorded-replay fixture
  completes 1000 ticks: compressed packet cursors advance continuously and
  playback ends normally. Packet indices count compressed records, not
  simulation updates; replay completion is checked independently.

This reproduces and fixes specific crash paths, not every possible SIGSEGV.
The replay fixture exercises recording/restoration/playback directly; the
actual post-finish menu transition still needs reproduction. Terrain appearing
late and the Italy light flash remain under investigation. The lighting-file
disk/native boundary recorded in the preceding batch remains unresolved.

The isolated syntax inventory changes raw accesses 815 -> 760, fixed sizes
63 -> 61, hexadecimal multipliers 2180 -> 2179, unknown occurrences 9963 ->
9900 and unique unknown spellings 1226 -> 1222. These inventory counts are not
an estimate of all remaining 64-bit bugs.
