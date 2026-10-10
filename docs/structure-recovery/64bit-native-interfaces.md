# Native drawing, menu text and network interfaces

Baseline: bb1b336. The corrected compatibility worklist falls from 501 to
435 conversions in this batch (66 diagnostic occurrences in 35 functions).
All 3364 s/fz/x rows remain identical, including the 2924 exact functions and
the previously corrected deformation hull. Full build, measurement, changed-TU
gate, complete suite and native scenarios accompany this document.

## Evidence and representations

| Runtime record | Original layout | Evidence |
| --- | --- | --- |
| Quad2DRenderVertex | xyz 0/4/8; reserved bytes 0xc..0x17; colour/specular 0x18/0x1c; uv 0x20/0x24; reserved 0x28..0x2f; size 0x30 | FixedTriangle writes these members; Queue copies all three complete records. Reserved bytes stay unknown and untouched. |
| Quad2DVertices | three vertices; size 0x90 | Both queue bodies and all render consumers. |
| Quad2D | vertices 0; Texture* 0x90; flags 0x94; size 0x98 | Producer stores and drawing texture/flag consumers. Native texture is eight bytes, flags moves to 0x98, stride becomes 0xa0. |
| Quad2DInputVertex | xyz 0/4/8; RGBA 0xc; uv 0x10/0x14; size 0x18 | FixedTriangle's integer-to-float conversion and RGBA packing. |
| MenuItem | optional char* text 0; short localized id 4; flags 6; type 7; value 8; min/max 0xa/0xb; submenu 0xc; action 0x10; size 0x14 | Profile entry builders supply actual string buffers; both in-race draws use literal text or GetTextString(id/id+1). Native items have a 0x20 stride. |
| Menu | optional char* title 0; short localized title id 4; items 0x14; callbacks 0x1cc..0x1d8; item callbacks 0x1dc; size 0x1e0 | Menu_Init and FrontendDraw_MenuPath establish the literal/localized distinction. Native items begin at 0x20. |

The shared render records moved to Sprite.h. Every record/member above has
central Win32 sizeof/offsetof checks. These are runtime records, not serialized
files. New pointer fields grow naturally in a native build.

Quad destination arguments genuinely mix numeric layer/render masks and direct
addresses: branches test 8, 0x10, 0x20 and 0x40, in that priority; the fallback
uses the address itself. They therefore use UINT_PTR, not a fabricated object
pointer. The explicit final conversion to the 32-bit flags member is retained:
render consumers use numeric flags and never reconstruct a pointer from flags.
Both native destination and stored Texture* retain all eight bytes. Current
production callers pass numeric masks. Original x86 tests additionally cover
unaligned direct destinations; native typed records require natural alignment,
so native direct-destination fixtures use aligned records.

The former stringId members were addresses, not localized ids. They are now
char* text, as are all six menu construction interfaces and the drawing locals.
Actual localized ids retain their short fields. All callers and repeated
prototypes were updated. No semantic names were guessed for reserved fields.
The two Quad layer counters now have evidenced g_quad2DCountA/B names.

Network payloads use void*, matching DirectPlay's payload API and multiple
packet types. CreateLocalPlayer names use char*, received sender-id output uses
DWORD* (DirectPlay's DPID is DWORD), and notification data stays a native void*.
The SetPlayerData function-pointer prototype carries void*, replacing the old
DWORD payload carrier. All producer/consumer declarations and stack-message
call sites were updated. The Windows platform interfaces themselves remain the
original ones; this is not a Linux networking implementation.

The empty Sound_NoOpMusicCallback receives diagnostic text/node addresses and
small numeric codes. Its ignored argument uses INT_PTR to preserve that mixed
contract. Game_UpdateType3ObjectState's ignored camera-source argument is a
void*: the source record is not dereferenced and no guessed layout is added.
Per-view temporary results of SceneNode_FindByType remain SceneNode* throughout.

## Matching and coverage

The first menu typing build caused a four-instruction register/load-order change
in the existing results panel's sum of two packed rectangle DWORDs. Equivalent
pointer/index forms and literal member names did not recover it. An inline
word-span helper with an accumulating local recovers the baseline instruction
sequence and all scores. It preserves both complete packed words, including the
high shorts; replacing this sum with x+width would alter original behavior.
The original short-array storage and its remaining primitive word read stay in
place. A union storage trial was not retained because it did not preserve score.
This rectangle has no embedded pointers; it is not a native pointer-layout fix.

Three new registered differential harnesses execute complete original and
rebuilt bodies: 304 queue scenarios, 360 in-race text draw scenarios and 576
Send/SetLocal scenarios. Models check whole guarded heaps/layers, copied reserved
bytes, float/colour conversion, unsigned counts, mask priority and caps, separate
sticky overflow flags, literal/localized text identities, rectangles/colours,
provider order, HRESULT paths and stdcall/callee-saved ABI. Queues execute without
controlled leaves. Menu rendering/header/text providers and network COM leaves
are controlled. Existing COM-output and receive harnesses still exercise full
CreateLocalPlayer and Receive paths, including allocation failure.

Actual current bodies also run under x64 ASan/UBSan: 288 queue, 360 menu and
1536 network scenarios. The menu fixture uses the real Menu, Texture and Graphics
headers, with offsets shifted after native pointers. Network fixtures control
COM/allocation leaves and use pointer-width SDK stand-ins; they do not exercise
a real DirectPlay implementation. Destination, texture, text, payload, name,
output and buffer identities exceed 4 GB. Complete native layers/input records
and message buffers remain guarded.

## Remaining integration work

The compatibility inventory is a diagnostic snapshot, not a complete native
game build. Merging changed Windows networking/music bodies back into the old
port projection exposes additional platform-stub/type errors: 370 errors versus
328 previously (306 remain in Graphics.cpp). All diagnostics are retained in the
audit metadata; missing new headers are not used to hide casts. The standalone
native tests above cover the converted interfaces independently of that older
platform projection. Port adapters need updating when these bodies are synced.

The inventory still contains legacy accesses to runtime menus through byte
buffers and fixed strides, collision-box constructors and sector entry tables,
on-disk relocation and other interfaces. Typed menu text does not claim those
old menu pools are native safe. Their producers/consumers require later coherent
batches. No source function or original bug is silently corrected in this batch.
