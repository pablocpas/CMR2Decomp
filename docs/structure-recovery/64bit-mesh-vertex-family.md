# Mesh source vertices and object clone placement

Baseline: `6140f47`. 2924 byte-exact functions preserved; `Mesh_CloneInto`
rises from 76.92% to 77.33%, no function loses score.

## MeshVertexF

Stage meshes keep their source vertices in the 0x30-byte record the file
stores and Direct3D uploads (`D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE |
D3DFVF_SPECULAR | D3DFVF_TEX2`, 0x2d2). The record holds no pointers, so its
layout is the same on every target (ON-DISK, no native copy needed).

| Offset | Field | Evidence |
| --- | --- | --- |
| 0x00 | `x, y, z` | read as floats by `Mesh_ReadVertexFixed`, the colour writers' position tests and the clone transform |
| 0x0c | `nx, ny, nz` | rotated as normals in `Mesh_CloneInto` |
| 0x18 | `diffuseColour` | packed ARGB written by the ground/object/sky colour writers and `Stage_SetHeightColours` |
| 0x1c | `specularColour` | alpha in the top byte (`(DWORD)alpha << 24`) |
| 0x20 | `textureCoordinates[2][2]` | two u/v pairs of the FVF |

It lives in its own header, `MeshVertex.h`, included only by the TUs that use
it (Graphics, LayoutChecks, Mesh, StageObjects, TrackCollision, TrackLighting).
Declaring it in `Mesh.h` changed code in TUs that never use it:
`GameMenu_SetupStageResults` lost 1.5 points, and moving `Mesh_CloneInto`'s
prototype into `Mesh.h` as well made `FrontendMenu_UpdateNetworkRallySetup`
lose its exact match. Graphics.cpp includes it where its old local definition was.

## Consumers converted

- `Mesh_CloneInto` (0x4abaf0) takes `StageObject *` and reads
  `pSource->matrix`. The old `(FixMatrix *)(pSource + 0x18)` is the original
  offset of that matrix, but a native StageObject places it later (pointer
  fields precede it), so the 64-bit build normalised the wrong bytes. Vertex
  and triangle copies use `sizeof(MeshVertexF)` / `sizeof(MeshTriangle)`.
- `Mesh_ReadVertexFixed` (0x46b440), the ground/object/sky/light-mesh colour
  writers (0x4923d0, 0x492470, 0x492520, 0x4920d0, 0x492220) and
  `Stage_SetHeightColours` (0x491d40) index `MeshVertexF` instead of
  `i * 0x30 + 0x18`.
- `RallyData_SetupWeatherTextureEntries`: the weather map comes from
  `CTexture::FindLoadTexture`, so its source rectangle is read from
  `Texture::field_0x11c` and `Texture::width` (0x11c/0x120 in the original
  layout, later on 64-bit) instead of raw offsets.

## Validation

- `tests/differential_mesh_vertex_family.py`: 960 cases of the clone, colour
  writers, vertex reader and height colours against the original executable,
  with complete guarded heaps and independent models. The harness writes
  `g_stageHeightMin`/`g_stageHeightScale` separately: they are two globals
  here, not one 8-byte block.
- `tests/check_64bit_mesh_vertex_family.py`: 192 native clones built with
  ASan/UBSan and heap pointers above 4 GB, plus a negative control that rejects
  the old +0x18 placement read.
- Full differential suite passes.

Found by OpenCMR2's `tools/audit_raw_access.py`, which compares each raw
access's covered fields on i386 and x86-64.
