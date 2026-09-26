#include <windows.h>
#include <string.h>
#include "Mesh.h"
#include "FileBuffer.h"
#include "Graphics.h"
#include "Frontend.h"
#include <stdio.h>

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

// GLOBAL: CMR2 0x0052101c
char g_strVertexBufferFull[] =
    "Failed to add shape to vertex buffer\nTrack block with too many vertices will be transparent\n"
    "Requested Vertices : %d  Limit : %d";

// Groups the triangles of a mesh by texture into index lists (once).
// match 44%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004b1ac0
void Mesh_BuildParts(Mesh *pMesh)
{
    MeshPart **ppPart;
    int count;
    int last;
    int i;
    int k;
    int off;
    int n;
    unsigned int lo;
    int hi;
    unsigned short *pIndex;

    if (pMesh->partCount >= 1)
        return;
    count = 0;
    last = -99;
    if (pMesh->triangleCount > 0) {
        int *pTex = (int *)((BYTE *)pMesh->pTriangles + 4);
        for (i = pMesh->triangleCount; i != 0; i--) {
            if (last != *pTex) {
                count++;
                last = *pTex;
            }
            pTex += 0x13;
        }
    }
    pMesh->partCount = count;
    if (count > 0) {
        ppPart = pMesh->pParts;
        for (i = count; i != 0; i--)
            *ppPart++ = (MeshPart *)CFileBuffer::AllocateLockedBuffer(0x1c);
    }
    off = 0;
    count = 0;
    last = -99;
    ppPart = pMesh->pParts - 1;
    for (i = 0; i < pMesh->triangleCount; i++) {
        n = *(int *)((BYTE *)pMesh->pTriangles + off + 4);
        if (last != n) {
            count++;
            ppPart++;
            (*ppPart)->indexCount = 0;
            (*ppPart)->texture = *(int *)((BYTE *)pMesh->pTriangles + off + 4);
            (*ppPart)->field_0x4 = *(int *)((BYTE *)pMesh->pTriangles + off + 8);
            last = n;
        }
        for (k = 3; k != 0; k--)
            (*ppPart)->indexCount++;
        off += 0x4c;
    }
    if (count > 0) {
        ppPart = pMesh->pParts;
        for (i = count; i != 0; i--) {
            (*ppPart)->pData = (unsigned short *)CFileBuffer::AllocateLockedBuffer((*ppPart)->indexCount << 1);
            ppPart++;
        }
    }
    count = 0;
    last = -99;
    ppPart = pMesh->pParts - 1;
    off = 0;
    for (i = 0; i < pMesh->triangleCount; i++) {
        n = *(int *)((BYTE *)pMesh->pTriangles + off + 4);
        if (last != n) {
            ppPart++;
            count++;
            (*ppPart)->indexCount = 0;
            last = n;
        }
        pIndex = (unsigned short *)((BYTE *)pMesh->pTriangles + off + 0x40);
        for (k = 3; k != 0; k--) {
            (*ppPart)->pData[(*ppPart)->indexCount] = *pIndex++;
            (*ppPart)->indexCount++;
        }
        off += 0x4c;
    }
    if (count > 0) {
        ppPart = pMesh->pParts;
        for (i = count; i != 0; i--) {
            hi = -1;
            lo = 999;
            for (k = 0; k < (*ppPart)->indexCount; k++) {
                if ((*ppPart)->pData[k] < lo) {
                    lo = (*ppPart)->pData[k];
                    (*ppPart)->minIndex = lo;
                }
                if (hi < (int)(*ppPart)->pData[k]) {
                    hi = (*ppPart)->pData[k];
                    (*ppPart)->maxIndex = hi;
                }
            }
            ppPart++;
        }
        ppPart = pMesh->pParts;
        for (i = count; i != 0; i--) {
            for (k = 0; k < (*ppPart)->indexCount; k++)
                (*ppPart)->pData[k] -= (short)(*ppPart)->minIndex;
            ppPart++;
        }
    }
}

// Copies the vertices of a mesh into the first shared vertex buffer with
// room for them.
// FUNCTION: CMR2 0x004b1d00
void Mesh_UploadVertices(Mesh *pMesh)
{
    int i;
    void *pVertices;

    for (i = 0; i < 100; i++) {
        if ((unsigned int)(CGraphics::m_pTextureManager->vertexBufferFill[i] + pMesh->field_0x10) < 2000) {
            CGraphics::m_pTextureManager->pVertexBuffers[i]->Lock(0x821, &pVertices, NULL);
            memcpy((BYTE *)pVertices + CGraphics::m_pTextureManager->vertexBufferFill[i] * 0x30, pMesh->pVertexData,
                   pMesh->field_0x10 * 0x30);
            CGraphics::m_pTextureManager->pVertexBuffers[i]->Unlock();
            pMesh->vertexBufferIndex = i;
            pMesh->vertexOffset = CGraphics::m_pTextureManager->vertexBufferFill[i];
            CGraphics::m_pTextureManager->vertexBufferFill[i] += pMesh->field_0x10;
            return;
        }
    }
    sprintf(CFrontend::m_stringDest, g_strVertexBufferFull,
            CGraphics::m_pTextureManager->vertexBufferFill[i] + pMesh->field_0x10, 2000);
}

// Copies the (changed) vertices of a mesh back into its slot of the vertex buffer.
// FUNCTION: CMR2 0x004b2020
void Mesh_RefreshVertices(Mesh *pMesh)
{
    void *pVertices;

    CGraphics::m_pTextureManager->pVertexBuffers[pMesh->vertexBufferIndex]->Lock(0x821, &pVertices, NULL);
    memcpy((BYTE *)pVertices + pMesh->vertexOffset * 0x30, pMesh->pVertexData, pMesh->field_0x10 * 0x30);
    CGraphics::m_pTextureManager->pVertexBuffers[pMesh->vertexBufferIndex]->Unlock();
}

// Sets the diffuse colour of every vertex of a mesh and refreshes its vertex buffer copy.
// FUNCTION: CMR2 0x004b2090
void Mesh_SetColourAndRefresh(Mesh *pMesh, DWORD colour)
{
    DWORD *p;
    int count;
    int i;
    void *pVertices;

    count = pMesh->field_0x10;
    if (count > 0) {
        p = pMesh->pVertexData + 6;
        for (i = count; i != 0; i--) {
            *p = colour;
            p += 12;
        }
    }
    CGraphics::m_pTextureManager->pVertexBuffers[pMesh->vertexBufferIndex]->Lock(0x821, &pVertices, NULL);
    memcpy((BYTE *)pVertices + pMesh->vertexOffset * 0x30, pMesh->pVertexData, count * 0x30);
    CGraphics::m_pTextureManager->pVertexBuffers[pMesh->vertexBufferIndex]->Unlock();
}

// Meshes cloned from others (see Mesh_CloneInto).
// GLOBAL: CMR2 0x00667364
short g_meshCloneBase[4096];
// GLOBAL: CMR2 0x00677124
Mesh *g_meshClones[4096];
// GLOBAL: CMR2 0x0067f22c
int g_meshCloneCount;

// FUNCTION: CMR2 0x004ab9c0
void Mesh_ResetCloneCount(void)
{
    g_meshCloneCount = 0;
}

// Clones a mesh and moves its vertices (positions and normals) into the
// local space of pSource->matrix (at +0x18), with the matrix axes
// normalised; the clone gets its own parts and vertex buffer slot.
// match 69%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004abaf0
Mesh *Mesh_CloneInto(Mesh *pSrc, BYTE *pSource)
{
    FixMatrix m;
    FixVector v;
    FixVector d;
    FixVector r;
    FixMatrix *pM;
    Mesh *pMesh;
    int i;
    int off;
    int k;

    g_meshCloneBase[g_meshCloneCount] = (short)g_meshCount;
    pMesh = Mesh_Alloc();
    g_meshClones[g_meshCloneCount] = pMesh;
    g_meshCloneCount++;
    *(int *)((BYTE *)pMesh + 0x108) = *(int *)((BYTE *)pSrc + 0x108);
    *(int *)((BYTE *)pMesh + 0x10c) = 0;
    pMesh->sizeUnits = 0;
    ((BYTE *)pMesh)[0x111] = ((BYTE *)pSrc)[0x111];
    ((BYTE *)pMesh)[0x112] = ((BYTE *)pSrc)[0x112];
    ((BYTE *)pMesh)[0x113] = ((BYTE *)pSrc)[0x113];
    pMesh->field_0x118 = 0;
    pMesh->field_0x11c = 0;
    *(int *)((BYTE *)pMesh + 0x114) = *(int *)((BYTE *)pSrc + 0x114);
    for (k = 0; k < 0xc; k++)
        ((BYTE *)pMesh)[k] = ((BYTE *)pSrc)[k];
    pMesh->field_0x10 = pSrc->field_0x10;
    pMesh->pVertexData = (DWORD *)CFileBuffer::AllocateLockedBuffer(pMesh->field_0x10 * 0x30);
    for (i = 0, off = 0; i < pMesh->field_0x10; i++, off += 0x30)
        memcpy((BYTE *)pMesh->pVertexData + off, (BYTE *)pSrc->pVertexData + off, 0x30);
    pMesh->triangleCount = pSrc->triangleCount;
    pMesh->pTriangles = (MeshTriangle *)CFileBuffer::AllocateLockedBuffer(pMesh->triangleCount * 0x4c);
    for (i = 0, off = 0; i < pMesh->triangleCount; i++, off += 0x4c)
        memcpy((BYTE *)pMesh->pTriangles + off, (BYTE *)pSrc->pTriangles + off, 0x4c);
    *(void **)((BYTE *)pMesh + 0x20) = CFileBuffer::AllocateLockedBuffer(pMesh->triangleCount * 0x14);
    for (i = 0, off = 0; i < pMesh->triangleCount; i++, off += 0x14)
        memcpy(*(BYTE **)((BYTE *)pMesh + 0x20) + off, *(BYTE **)((BYTE *)pSrc + 0x20) + off, 0x14);
    *(int **)((BYTE *)pMesh + 0x34) = (int *)CFileBuffer::AllocateLockedBuffer(pMesh->field_0x10 << 2);
    for (i = 0; i < pMesh->field_0x10; i++)
        (*(int **)((BYTE *)pMesh + 0x34))[i] = (*(int **)((BYTE *)pSrc + 0x34))[i];
    ((BYTE *)pMesh)[0x104] = ((BYTE *)pSrc)[0x104];
    *(int *)((BYTE *)pMesh + 0x30) = *(int *)((BYTE *)pSrc + 0x30);
    pM = (FixMatrix *)(pSource + 0x18);
    *(int *)((BYTE *)pMesh + 0x2c) = *(int *)((BYTE *)pSrc + 0x2c);

    FixMatrix_GetRight(&v, pM);
    FIX_NORMALIZE_INTO(v, v);
    FixMatrix_SetRight(&v, &m);
    FixMatrix_GetUp(&v, pM);
    FIX_NORMALIZE_INTO(v, v);
    FixMatrix_SetUp(&v, &m);
    FixMatrix_GetForward(&v, pM);
    FIX_NORMALIZE_INTO(v, v);
    FixMatrix_SetForward(&v, &m);
    FixMatrix_GetPosition(&v, pM);
    FixMatrix_SetPosition(&v, &m);
    for (i = 0, off = 0; i < pMesh->field_0x10; i++, off += 0x30) {
        float *pF = (float *)((BYTE *)pMesh->pVertexData + off);

        d.x = (int)(__int64)(pF[0] * CGraphics::m_65536) - v.x;
        d.y = (int)(__int64)(pF[1] * CGraphics::m_65536) - v.y;
        d.z = (int)(__int64)(pF[2] * CGraphics::m_65536) - v.z;
        FixMatrix_InverseRotateVector(&r, &d, &m);
        pF[0] = (float)r.x * CGraphics::m_oneOver65536;
        pF[1] = (float)r.y * CGraphics::m_oneOver65536;
        pF[2] = (float)r.z * CGraphics::m_oneOver65536;
        d.x = (int)(__int64)(pF[3] * CGraphics::m_65536);
        d.y = (int)(__int64)(pF[4] * CGraphics::m_65536);
        d.z = (int)(__int64)(pF[5] * CGraphics::m_65536);
        FixMatrix_InverseRotateVector(&r, &d, &m);
        pF[3] = (float)r.x * CGraphics::m_oneOver65536;
        pF[4] = (float)r.y * CGraphics::m_oneOver65536;
        pF[5] = (float)r.z * CGraphics::m_oneOver65536;
    }
    Mesh_BuildParts(pMesh);
    Mesh_UploadVertices(pMesh);
    return pMesh;
}

extern int g_unk0x0067f228;
extern int g_sectorCount;
extern Sector *g_sectors[14096];

// Static stage objects (see StageObject in Sector.h).
// GLOBAL: CMR2 0x00669364
StageObject *g_stageObjects[6000];

// FUNCTION: CMR2 0x004ab9b0
StageObject *StageObject_GetNext(StageObject *pObject)
{
    return pObject->pNext;
}

// Frees the mesh parts of every stage object and empties the table and the
// vertex buffers.
// FUNCTION: CMR2 0x004ab720
void StageObject_FreeAll(void)
{
    StageObject **pSlot;
    Mesh *pMesh;
    int i;

    pSlot = g_stageObjects;
    do {
        if (*pSlot != NULL) {
            pMesh = (*pSlot)->pMesh;
            if (pMesh != NULL) {
                g_meshTotalSize -= pMesh->sizeUnits;
                for (i = 0; i < pMesh->partCount; i++) {
                    CFileBuffer::FreeGenericFileBuffer(pMesh->pParts[i]->pData);
                    pMesh->pParts[i]->pData = NULL;
                    CFileBuffer::FreeGenericFileBuffer(pMesh->pParts[i]);
                    pMesh->pParts[i] = NULL;
                }
            }
            *pSlot = NULL;
            g_unk0x0067f228--;
        }
        for (i = 0; i < 100; i++)
            CGraphics::m_pTextureManager->vertexBufferFill[i] = 0;
        pSlot++;
        CGraphics::m_pTextureManager->field_0x348 = 0;
    } while ((int)pSlot < (int)&g_stageObjects[6000]);
}

// Frees every cloned mesh (and the parts of its source mesh slot).
// FUNCTION: CMR2 0x004ab9d0
void Mesh_FreeClones(void)
{
    Mesh **ppClone;
    short *pBase;
    Mesh *pClone;
    int n;
    int i;

    if (g_meshCloneCount > 0) {
        ppClone = g_meshClones;
        pBase = g_meshCloneBase;
        n = g_meshCloneCount;
        do {
            pClone = *ppClone;
            if (pClone != NULL) {
                if (pClone->pVertexData != NULL) {
                    CFileBuffer::FreeGenericFileBuffer(pClone->pVertexData);
                    pClone->pVertexData = NULL;
                }
                if (pClone->pTriangles != NULL) {
                    CFileBuffer::FreeGenericFileBuffer(pClone->pTriangles);
                    pClone->pTriangles = NULL;
                }
                if (pClone->pField20 != NULL) {
                    CFileBuffer::FreeGenericFileBuffer(pClone->pField20);
                    pClone->pField20 = NULL;
                }
                if (pClone->pLightLevels != NULL) {
                    CFileBuffer::FreeGenericFileBuffer(pClone->pLightLevels);
                    pClone->pLightLevels = NULL;
                }
                if (g_meshes[(unsigned short)*pBase] != NULL) {
                    for (i = 0; i < pClone->partCount; i++) {
                        CFileBuffer::FreeGenericFileBuffer(pClone->pParts[i]->pData);
                        pClone->pParts[i]->pData = NULL;
                        CFileBuffer::FreeGenericFileBuffer(pClone->pParts[i]);
                        pClone->pParts[i] = NULL;
                    }
                    g_meshes[(unsigned short)*pBase] = NULL;
                    g_meshCount--;
                }
                CFileBuffer::FreeGenericFileBuffer(*ppClone);
                *ppClone = NULL;
                g_meshCloneCount--;
            }
            pBase++;
            ppClone++;
        } while (--n != 0);
    }
}

// Recreates the vertex buffers and uploads every mesh LOD in use again
// (meshes, sector ground meshes, stage objects), e.g. after a device reset.
// FUNCTION: CMR2 0x004b2110
void Mesh_ReuploadAll(void)
{
    unsigned int i;

    CGraphics::ReleaseVertexBuffers();
    CGraphics::FUN_004b1980();
    for (i = 0; i < (unsigned int)g_meshCount; i++) {
        if (g_meshes[i] != NULL)
            Mesh_UploadVertices((Mesh *)((BYTE *)g_meshes[i] + ((BYTE *)g_meshes[i])[0x112] * 0x108));
    }
    for (i = 0; i < (unsigned int)g_sectorCount; i++) {
        if (g_sectors[i] != NULL && g_sectors[i]->pMesh != NULL)
            Mesh_UploadVertices((Mesh *)((BYTE *)g_sectors[i]->pMesh + g_sectors[i]->pMesh->lodIndex * 0x108));
    }
    for (i = 0; i < (unsigned int)g_unk0x0067f228; i++) {
        if (g_stageObjects[i] != NULL)
            Mesh_UploadVertices((Mesh *)((BYTE *)g_stageObjects[i]->pMesh + ((BYTE *)g_stageObjects[i]->pMesh)[0x112] * 0x108));
    }
}
