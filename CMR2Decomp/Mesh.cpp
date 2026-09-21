#include <windows.h>
#include <string.h>
#include "Mesh.h"
#include "FileBuffer.h"
#include "Graphics.h"

Mesh *g_meshes[4096];
int g_meshCount;
int g_meshTotalSize;

// FUNCTION: CMR2 0x004ab710
int Mesh_GetField0x10(Mesh *pMesh)
{
    return pMesh->field_0x10;
}

// FUNCTION: CMR2 0x004ab7e0
void Mesh_Free(void *pMesh)
{
    int i;
    int j;
    Mesh **pp;

    i = 0;
    pp = g_meshes;
    while (*pp == NULL || *pp != pMesh) {
        pp++;
        i++;
        if (pp > &g_meshes[4095])
            return;
    }
    g_meshTotalSize -= ((Mesh *)pMesh)->sizeUnits;
    for (j = 0; j < g_meshes[i]->partCount; j++) {
        CFileBuffer::FreeGenericFileBuffer(g_meshes[i]->pParts[j]->pData);
        g_meshes[i]->pParts[j]->pData = NULL;
        CFileBuffer::FreeGenericFileBuffer(g_meshes[i]->pParts[j]);
        g_meshes[i]->pParts[j] = NULL;
    }
    g_meshes[i] = NULL;
    g_meshCount--;
}

// FUNCTION: CMR2 0x004ab8a0
Mesh *Mesh_Alloc(void)
{
    int i;

    for (i = 0; i < 4096; i++) {
        if (g_meshes[i] == NULL) {
            g_meshes[i] = (Mesh *)CFileBuffer::AllocateLockedBuffer(sizeof(Mesh));
            g_meshes[i]->field_0x118 = 0;
            if (i != g_meshCount)
                return NULL;
            g_meshCount++;
            return g_meshes[i];
        }
    }
    return NULL;
}

// FUNCTION: CMR2 0x004ab8f0
void Mesh_SetVertexColours(Mesh *pMesh, BYTE *pRGB)
{
    int i;
    int v;

    for (i = 0; i < pMesh->triangleCount; i++) {
        for (v = 0; v < 3; v++) {
            pMesh->pTriangles[i].colour[v][0] = pRGB[0];
            pMesh->pTriangles[i].colour[v][1] = pRGB[1];
            pMesh->pTriangles[i].colour[v][2] = pRGB[2];
        }
    }
    Mesh_Rebuild(pMesh);
}

// FUNCTION: CMR2 0x004ab960
void Mesh_SetVertexAlpha(Mesh *pMesh, BYTE alpha)
{
    int i;
    int v;

    for (i = 0; i < pMesh->triangleCount; i++) {
        for (v = 0; v < 3; v++)
            pMesh->pTriangles[i].colour[v][3] = alpha;
    }
    Mesh_Rebuild(pMesh);
}

// Shuffles the RGBA bytes stored for one triangle vertex into the D3D ARGB
// layout expected by the vertex buffer.
#define MESH_SET_VERTEX_COLOUR(v)                                                   \
    do {                                                                            \
        colour = *(DWORD *)pMesh->pTriangles[i].colour[v];                          \
        *(DWORD *)((char *)pMesh->pVertexData +                                     \
                   pMesh->pTriangles[i].vertexIndex[v] * 0x30 + 0x18) =             \
            RGBA_MAKE(((BYTE *)&colour)[0], ((BYTE *)&colour)[1],                   \
                      ((BYTE *)&colour)[2], ((BYTE *)&colour)[3]);                  \
    } while (0)

// Re-writes the vertex colours of the locked copy of the vertex array (shuffling
// the stored RGBA bytes into the D3D ARGB layout) and uploads the whole array
// into the mesh's shared vertex buffer.
// FUNCTION: CMR2 0x004b1ea0
void Mesh_Rebuild(Mesh *pMesh)
{
    int i;
    DWORD colour;
    void *pVertices;

    for (i = 0; i < pMesh->triangleCount; i++) {
        MESH_SET_VERTEX_COLOUR(0);
        MESH_SET_VERTEX_COLOUR(1);
        MESH_SET_VERTEX_COLOUR(2);
    }

    CGraphics::m_pTextureManager->pVertexBuffers[pMesh->vertexBufferIndex]->Lock(0x821, &pVertices, NULL);
    memcpy((char *)pVertices + pMesh->vertexOffset * 0x30, pMesh->pVertexData,
           pMesh->field_0x10 * 0x30);
    CGraphics::m_pTextureManager->pVertexBuffers[pMesh->vertexBufferIndex]->Unlock();
}
