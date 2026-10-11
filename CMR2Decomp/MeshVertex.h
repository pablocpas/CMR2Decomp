#ifndef _MESHVERTEX_H
#define _MESHVERTEX_H

// Kept out of Mesh.h: declaring it there shifts code in TUs that never use it
// (GameMenu_SetupStageResults, FrontendMenu_UpdateNetworkRallySetup).

// ON-DISK: pointer-free 0x30-byte source vertex, also uploaded as D3DFVF_XYZ |
// D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX2 (0x2d2).
// Shared by stage geometry, object clones, lighting and shadow cylinders.
struct MeshVertexF {
    float x, y, z;              // 0x00 position
    float nx, ny, nz;           // 0x0c normal
    DWORD diffuseColour;       // 0x18 packed ARGB
    DWORD specularColour;      // 0x1c packed ARGB
    float textureCoordinates[2][2]; // 0x20 two u/v pairs
};

#endif
