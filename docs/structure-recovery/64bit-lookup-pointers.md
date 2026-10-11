# Sound curves, AI reference routes and timing workspaces

Baseline: 09a2b1b. Three reviewed families were generated and applied together
by scripts/migrate_64bit.py. Its unchanged-baseline gate and complete candidate
gate both passed; all three families were accepted with no parked group. The
recipe is scripts/migrations/lookup-tables.json and its persistent migration
report and probe logs accompany this document.

Pointer/integer diagnostics fall from 286 to 257: 29 removed across nine
functions. Raw accesses fall from 1231 to 1201. All 3364 s/fz/x rows remain
identical and 2924 functions remain byte-exact. From the original 594-conversion
inventory, 337 entries have now been removed. The deformation-hull correction
remains exact.

## Sound curve ownership and native members

| SoundCurve / Win32 offset | Field | Evidence |
| --- | --- | --- |
| +0 | int count | The interpolators divide the input range by this interval count; the sample arrays contain count+1 nominal entries. |
| +4 / +8 | int min / max | The range predicate checks the inclusive bounds; interpolation subtracts min and divides by max-min. |
| +0xc | unsigned short* pSamples | Stage initialization assigns the static word sample arrays; the word interpolator reads the indexed pitch/pan values. |
| +0x10 | BYTE* pBytes | Stage initialization assigns the static byte arrays; the byte interpolator reads the volume curve. |

The existing SoundCurve definition moved into SoundCurve.h. Its three consumers
now take SoundCurve*, and definitions, repeated declarations and all callers in
NetRace, SurfaceTypes and StageObjects agree. Static initialization and pointer
assignments retain their original order. Eight unknown pointer aliases and the
int-array alias are replaced by descriptor members and typed addresses.

The four descriptors now have names supported by their primary consumers:
g_engineSoundCurve, g_engineLoadSoundCurve, g_engineWhineSoundCurve and
g_speedSoundCurve. The engine-sound routine identifies its three range queries
as engine, load and whine; the speed-sound routine consumes the fourth. The
whine curve is also used by moving-object sound updates.

These are runtime descriptors, initialized with bounds and populated with
addresses of static sample arrays, rather than file records. Win32 sizeof is
20, with both pointer offsets checked centrally. Native sizeof is 32, with
sample pointers at 16 and 24. The min/max/count members keep their numeric type.
The interpolation expression order, integer truncation, zero-step branch and
inclusive range predicate are unchanged. Endpoint fixtures provide accessible
storage for the original index+1 read even when its interpolation weight is
zero; runtime sample-array capacities are unchanged.

## Pointer-free AI archive records and their runtime pointer table

| AIReferenceRoute / fixed file offset | Field | Evidence |
| --- | --- | --- |
| +0 | int headingDegrees[370] | Telemetry compares these fixed-degree targets with the body-axis heading. |
| +0x5c8 | int speedSamples[370] | Telemetry compares them with the fixed conversion of Car.speed and uses the resulting difference to blend heading targets. |
| +0xb90 | int positionX[370] | The route publisher adds the route origin's x; telemetry subtracts Car.position.x and forms target direction differences. |
| +0x1158 | int positionZ[370] | The publisher adds origin z; telemetry uses the matching z direction differences. |

The AI archive loader places these records in the loaded byte stream and the
publisher advances by their complete 0x1720-byte size. All four arrays comprise
signed numeric words, with no embedded pointers. AIReferenceRoute.h marks this
layout ON-DISK. sizeof and every member offset remain the same on both tested
ABIs. Translating x/z in place retains the serialized shape and original
coordinate wrapping.

The nine-entry global is now AIReferenceRoute* g_aiReferenceRoutes[9]. Its slot
zero and eight following entries remain one owned array; the publisher writes
from slot one and consumers use their 1-based variant. The table holds native
addresses rather than truncating them to int. The formerly unused integer
argument also receives its demonstrated BYTE** table-address type throughout
the producer/declaration flow. It remains unused in the publisher.

Telemetry indexes the route members directly and reads named Car members for
position, right axis, velocity, speed, first-wheel slip and steering angle.
The steering angle explicitly retains the original signed short interpretation.
These reads follow Car's native layout, including fields after its pointer band.
Existing central Car layout checks cover those fields. No raw read of a route
pointer or hardcoded pointer-table stride remains in these two functions.

## Runtime timing workspace and complete parameter flow

| StageTimingSpread / offset | Field | Producer/consumer evidence |
| --- | --- | --- |
| +0 | int sampleWords[40] | A complete numeric span supports the original biased word cursor without traversing separately bounded arrays. |
| +0 | int progress[10] | BuildSplitInterpolationFactors writes the accumulated active-distance fraction, starting with zero. |
| +0x28 | int startWeight[10] | The factor builder writes the fraction before the start position. |
| +0x50 | int middleWeight[10] | It stores one minus the start and end fractions. |
| +0x78 | int endWeight[10] | It writes the fraction after the end position. |
| +0xa0 / +0xa4 | startPositionFixed / endPositionFixed | The factor builder locates these positions between fixed split boundaries; sample blending divides them by route availability. |
| +0xa8 / +0xac | startBlendFixed / endBlendFixed | Both interpolation routines form these endpoint weights and their complements. |

InitRaceDriverRecords owns the stack workspace; its existing 0xb0-byte layout
is preserved. The former raw byte prefix now has four named numeric bands and
one complete word-array view. Four unnamed trailing parameters gain names;
the unnamed prefix becomes the demonstrated word span. Every view/member has
a central offset check, and the native fixture checks the pointer-free size.

BuildSplitInterpolationFactors, BlendCarPaintDecalRows, RefreshPaintSamplesAndTimeSpread,
InterpolateCarFixedSampleRows and BuildAndSortCPUSplitTimes pass StageTimingSpread*
throughout. Initialization passes &local directly. The old pCar variable names
remain where they affect MSVC6 scheduling, but their actual type identifies the
timing workspace. Its shared word cursor begins at sampleWords[31]; the original
repeated start-weight loads and numeric arithmetic are retained. The factor
builder's preexisting volatile store remains unchanged. No new volatile, assembly
or pragma was introduced.

## Validation and remaining work

Full MSVC6 build/link, all-function measurement, metadata preparation and the
five-TU matching gate pass. The complete suite passes all 126 harnesses with
zero failures. Existing telemetry and rally-time harnesses exercise the changed
telemetry/factor/blending/interpolation bodies. The new registered harness adds
412 original/rebuilt cases with independent range/interpolation/coordinate
models, complete poisoned heaps, normalized pointer slots and write widths,
provider call order, stack cleanup and preserved registers. It includes one,
two and eight route records, empty/negative record counts, up to all 370 points,
signed coordinate wrapping, inclusive curve boundaries and zero-step lookups.

The native fixture executes eight actual production bodies with current
production headers in 548 scenarios against original x86 oracles under
ASan/UBSan: all 412 lookup/publication cases, 40 timing pipelines and 96 telemetry
cases. Inputs, outputs and unrelated bytes are compared across the complete
heap. Car's pointer-free original spans are projected into the native record
only inside the test fixture, and its whole native record must remain unchanged.
Runtime pointers and relevant storage are checked above 4 GB.

Telemetry's atan2 and route-position providers are controlled with the original
argument trace. Fixed-point arithmetic runs using the port's existing portable
helpers. For the native telemetry comparison, the x86 oracle explicitly uses
53-bit precision and truncating conversion (FPCW 0xe7f), matching native double
arithmetic and C++ casts; MSVC6 /QIfist otherwise
obeys ambient x87 rounding. The registered original/rebuilt harness separately
retains its default rounding mode. This batch changes no arithmetic expression
and does not establish equivalence of native conversions under every ambient
x87 mode. The focused native fixture does not execute external loading,
rendering, whole CPU sorting or whole race initialization.

The compatibility projection still retains 367 additional compiler errors,
with no new normalized diagnostics compared with the previous batch. Readability
counts report 10301 unknown occurrences, 1259 unique unknown identifiers and
1481 unnamed field declarations. The compact readability artifact retains
source hashes and per-file counts; its reproduce command regenerates the full
lexical inventory. A whole-game native build still requires the remaining 257
pointer/integer conversions, 1201 raw accesses, serialized relocation and
platform integration work.
