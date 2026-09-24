#ifndef _MESH_H
#define _MESH_H

// Triangle of a Mesh (0x4c bytes); per-vertex colour bytes at 0x34/0x38/0x3c.
struct MeshTriangle {
    BYTE field_0x0[0x34];
    BYTE colour[3][4];              // 0x34  r,g,b,a per vertex
    unsigned short vertexIndex[3];  // 0x40  index into the mesh vertex array
    BYTE field_0x46[0x6];
};

// Triangles of a mesh sharing one texture, as an index list (0x1c bytes).
struct MeshPart {
    int texture;                // 0x0  texture of the triangles
    int field_0x4;              // 0x4
    int minIndex;               // 0x8  first vertex used (indices are relative to it)
    int maxIndex;               // 0xc
    int indexCount;             // 0x10
    unsigned short *pData;      // 0x14 vertex indices
    int field_0x18;             // 0x18
};

// Renderable mesh (0x120 bytes); only the fields used so far are named.
struct Mesh {
    BYTE field_0x0[0xc];
    DWORD *pVertexData;         // 0xc  source copy of the vertex array
    int field_0x10;             // 0x10 vertex count
    int vertexBufferIndex;      // 0x14 index into D3DTextureManager::pVertexBuffers
    int vertexOffset;           // 0x18 first vertex used inside that buffer
    int field_0x1c;
    void *pField20;             // 0x20 per-mesh buffer, freed with the clone
    MeshTriangle *pTriangles;   // 0x24
    int triangleCount;          // 0x28
    int field_0x2c;
    unsigned int flags;         // 0x30 0x80: lit per vertex (else one level for the whole mesh)
    int *pLightLevels;          // 0x34 light level of each vertex
    MeshPart *pParts[50];       // 0x38
    int partCount;              // 0x100
    BYTE field_0x104[0xc];
    BYTE sizeUnits;             // 0x110
    BYTE field_0x111[7];
    int field_0x118;
    int field_0x11c;
};

// GLOBAL: CMR2 0x0067b124
extern Mesh *g_meshes[4096];
// GLOBAL: CMR2 0x0067f224
extern int g_meshCount;
// GLOBAL: CMR2 0x0067f230
extern int g_meshTotalSize;

Mesh *Mesh_Alloc(void);
void Mesh_Free(void *pMesh);
void Mesh_SetVertexColours(Mesh *pMesh, BYTE *pRGB);
void Mesh_SetVertexAlpha(Mesh *pMesh, BYTE alpha);
void Mesh_Rebuild(Mesh *pMesh);
int Mesh_GetField0x10(Mesh *pMesh);
Mesh *Mesh_GetShadowCylinder(Mesh *pMesh);
void Mesh_RefreshVertices(Mesh *pMesh);
void Mesh_SetColourAndRefresh(Mesh *pMesh, DWORD colour);

#endif
