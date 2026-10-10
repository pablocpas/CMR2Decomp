# Stage collision tables: fixed disk records, native runtime pointers

Baseline: `5d991a5`, 2924 byte-exact functions. This batch removes 57
pointer/integer diagnostics (558 → 501 in the same temporary compatibility
snapshot), recovering twelve global names from the loader and its consumers.
The raw-access inventory falls 1319 → 1304; nine literal hex strides disappear.
Three previously implicit reserved fields remain provisionally named because
only their widths and positions are demonstrated.

## Records and evidence

TrackCollisionData.h centralizes the duplicated TrackTriangle definition and
four related views. Disk records contain no pointers and keep their original
widths on both x86 and x64. The runtime pointer block grows with pointer width.
All sizes, member offsets, index-array and pointer-view widths have central
LayoutChecks; the native fixture checks the disk sizes and native pointer slot.

| Record | Original layout | Evidence |
|---|---|---|
| TrackCollisionHeader | x 0x00, z 0x04, five signed short levelCounts at 0x08, unknown short 0x12; size 0x14 | Loader reads five counts then skips one word; traversal subtracts the first two words as the grid origin. |
| TrackQuadNode | signed short triangleCount at 0x00, unknown short 0x02, int firstIndex at 0x04; size 8 | Count -1 selects an internal node; firstIndex is then the child base, otherwise the first triangle-list position. |
| TrackTriangle | three unsigned short indices at 0x00, packed 7-bit surface / 9-bit flags at 0x06; size 8 | Triangle copying reads indices unsigned; height and face readers consume the low seven surface bits. |
| TrackCollisionCount | unsigned short count at 0x00, unknown upper word 0x02; size 4 | Loader advances four bytes but reads only the low word for vertex and triangle counts. No meaning is invented for the upper word. |
| TrackCollisionTables | five level pointers then vertex pointer; size 0x18 on Win32 | Original global slots 0x591b00..0x591b14 are traversed contiguously; native pointer slots must remain contiguous too. |

The twelve recovered globals identify the triangle table, collision header,
triangle-count record, runtime table pointers, vertex view, triangle-index
lists, grid row/column counts, vertex-count record, complete block, list-count
record and copied node counts. GLOBAL addresses remain recorded. The original
vertex slot 0x591b14 is a documented macro view of the single runtime pointer
block at 0x591b00, rather than a separate allocation. Tests locate that member
through its owner when there is no standalone PDB entity.

The original traversal can reach slot 5 before it tests the depth guard.
That slot is the adjacent vertex pointer: it can interpret the vertex payload
as node data at the boundary. The six-slot traversal union preserves this
original edge case while removing dependence on linker ordering and an
out-of-bounds access across separate globals. The native test supplies the
same controlled sixth-slot node payload as the existing original-code fixture,
covering both a level-5 leaf and the >=5 rejection. This is preserved original
behavior, not an added sixth serialized quadtree table.

## Accesses and interfaces

The loader stores native typed pointers and uses offsetof/sizeof for its fixed
header and record strides. Node-count and list-index byte products explicitly
retain signed int arithmetic when sizeof is involved, including a controlled
negative node-count fixture. It never writes native pointers into disk bytes.
Triangle copy/read and quadtree traversal use arrays and named fields.
StageTiming_CopyTriangleVertices takes FixVector[3] output and an ignored
unsigned-short pointer: its sole caller supplies the existing short output of
the face reader. StageObject_AverageWheelGroundLighting uses actual vectors for
its triangle and corner cursor, eliminating its three address conversions.
The existing lighting differential harness still exercises that complete caller.

Removal by function: loader 13, Track_GetTriangle 18, Track_FindTriangle 9,
StageTiming_CopyTriangleVertices 9, face reader 4, Track_GetHeight 1 and the
lighting caller 3. All function bytes and s/fz/x scores remain identical.

## Validation

The new registered differential_track_tables.py checks 144 complete loader
scenarios and 264 triangle-reader scenarios with independent pointer/copy
models, the whole guarded heap and a separate 1 MB vertex allocation. It covers
unsigned indices 32768/65535, signed triangle offsets, packed surface bits,
zero counts, unknown high words and outputs overlapping vertices or indices.
Original and rebuilt bodies execute all reads themselves; only release
registration is controlled. It also checks global write widths and stdcall /
callee-saved ABI. The existing track_geometry harness checks full traversal
with a controlled nearest-triangle leaf.

check_64bit_track_tables.py compiles five actual current bodies with the actual
header: loader, three triangle readers and traversal. Its 444 x64 scenarios
compare complete original primitive heaps and projected pointer slots under
ASan/UBSan. Table, vertex, output and registered callback addresses exceed
4 GB; pointer slots keep all eight bytes and serialized record sizes stay fixed.
Native leaf controls are registration and the nearest-triangle provider.

The compatibility inventory includes new untracked headers before compilation;
otherwise missing-header errors suppress diagnostics and give a false count.
The corrected snapshot contains all current headers, retains 328 platform/API
errors outside these paths, and reports 501 address conversions. It is an
inventory, not a successful native build of the entire game. Scope and snapshot
method are described in 64bit-car-surface-cursors.md.

Full build, measurement, changed-TU gate and complete registered suite results
are saved alongside this document. No required function is deferred within
this table-access batch. Other collision-box constructors and sector-list
records remain separate work, as recorded in 64bit-collision-boxes.md.
