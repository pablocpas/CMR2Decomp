# Replay event exhaustion (2026-10-11)

Replay_PlayStreamFrame (original 0x0046cfa0) must choose its event-count
array before reading it. Input streams have a NULL pStates array. The old
source evaluated pStates[lane].eventCount first and then overwrote the
result for input streams. MSVC6 retained that invalid read.

The Japan Rally Challenge capture of the previous measured build faulted
at relocated 0x004dc0fa, reading address 0x58. The instruction reads
[ecx + edx + 0x58], where edx came from ReplayStream +0x30 (pStates).
This is a decompilation error, independent of pointer width.

The original executable does this when the search found no later event:

```text
0046d186 cmp dl, 0xff
0046d189 jne 0046d1b4
0046d18b mov eax, [esi + 0x1c]   ; stream type
0046d18e test eax, eax
0046d190 jne 0046d1aa           ; state format only
... input format: base from [esi + 0x24]
0046d1a1 mov dl, [ecx + eax*4 + 0x1148]
0046d1a8 jmp 0046d1b4
0046d1aa imul ecx, ecx, 0x5c
0046d1ad mov edx, [esi + 0x30]
0046d1b0 mov dl, [ecx + edx + 0x58]
0046d1b4 dec dl
```

The source now uses if (type == 0) / else, matching the original branch.
No replay format, controls, vehicle constants or clock rule was changed.
OpenCMR2 carries the same correction and tests both formats with a NULL
unused array, empty event tables, compressed-repeat boundaries and final
event selection. The native and i386 tests pass.

The default MSVC6 executable was rebuilt and remeasured. All 2925 formerly
byte-exact functions remain exact; no exact function regressed. The event
function remains non-exact. All 135 registered differential harnesses pass;
those harnesses do not themselves exercise this particular event tail.

Local executable/capture evidence is retained in the sibling OpenCMR2
build/reference-challenge directory. The incomplete old decompilation
capture is failure evidence, not a valid parity reference. The separate
Japan rival trajectory stall remains under investigation and is not
claimed fixed by this event-count correction.
