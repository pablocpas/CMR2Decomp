#include <windows.h>
#include <stdlib.h>
#include "Sector.h"
#include "SceneNode.h"
#include "Graphics.h"
#include "Game.h"

int g_sectorsPerRow;
Sector *g_sectors[14096];
int g_sectorHalfSize;
int g_sectorVisibleBits[128];
int g_sectorSize;
int g_sectorRows;
int g_sectorScale;
int g_sectorCount;
int g_sectorCullEnabled;
int g_sectorCullDisabled;
int g_sectorOriginX;
int g_sectorOriginZ;
int g_unk0x006ed5e8;
int g_unk0x006ed5ec;

// FUNCTION: CMR2 0x004b9360
void Sector_GetGridDimensions(int *columns, int *rows)
{
    *columns = g_sectorsPerRow;
    *rows = g_sectorRows;
}

// Counts of the records that follow the 0x30-byte header of a stage mesh file.
struct MeshFileHeader {
    int field_0x0;
    int textureFile;               // 0x4  passed to the texture loader
    int field_0x8;
    int field_0xc;                 // 0xc  offset of the texture name table
    int field_0x10;                // 0x10 records of 0x14
    int field_0x14;                // 0x14 records of 0x30
    unsigned short nodeCount;      // 0x18 records of 0x18c
    unsigned short meshCount;      // 0x1a records of 0x120
    unsigned short objectCount;    // 0x1c records of 0xa0
    unsigned short sectorCount;    // 0x1e records of 0x88
    int triangleCount;             // 0x20 records of 0x4c
    unsigned short field_0x24;
    unsigned short recordCount;    // 0x26 records of 0x5c
    unsigned short partCount;      // 0x28 records of 0x1c
    int field_0x2c;
};

// Meshes registered by the mesh file loader, indexed by total mesh size.
// GLOBAL: CMR2 0x0066f124
void *g_unk0x0066f124[8192];
extern int g_unk0x0067f228;
extern StageObject *g_stageObjects[6000];

void FUN_004b98f0(int *p, int value);
void FUN_004b9910(int param1, int param2, unsigned int param3, int param4, int param5);
void Mesh_UploadVertices(Mesh *pMesh);
void Mesh_BuildParts(Mesh *pMesh);

// Magic of a C3D model file.
// GLOBAL: CMR2 0x0052110c
char g_strC3dMagic[] = "PP_F";

void *FUN_004b93c0(BYTE *pData, int param_2, unsigned int param_3);

// Builds the scene of a C3D model file, if the data really is one.
// FUNCTION: CMR2 0x004b9380
int FUN_004b9380(unsigned int data, unsigned int parent, unsigned int textures)
{
    int result = 0;

    if (strncmp((char *)data, g_strC3dMagic, 4) == 0)
        result = (int)FUN_004b93c0((BYTE *)data, parent, textures);
    return result;
}

// Relocates the stage mesh file: turns the offsets stored in the node, mesh,
// object and sector records into pointers, registers them in the scene node,
// mesh, stage object and sector tables, loads the textures and re-uploads the
// vertex buffers of every mesh.
// match 40%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004b93c0
void *FUN_004b93c0(BYTE *pData, int param_2, unsigned int param_3)
{
    int count = 0;
    BYTE *pNodes;
    BYTE *pMeshArray;
    BYTE *pStageObjects;
    BYTE *pSectors;
    BYTE *pTriangles;
    BYTE *pVertexData;
    BYTE *pLightLevels;
    BYTE *pVertexFlags;
    BYTE *pRecords;
    int textureRecords;
    unsigned short recordSize;
    int savedNodeCount;
    int i;
    int j;
    int k;
    int *pField;
    BYTE *p;

    pNodes = pData + 0x30;
    pMeshArray = pNodes + *(unsigned short *)(pData + 0x18) * 0x18c;
    pStageObjects = pMeshArray + (*(unsigned short *)(pData + 0x1a) + *(unsigned short *)(pData + 0x1c)) * 0x120;
    pSectors = pStageObjects + *(unsigned short *)(pData + 0x1c) * 0xa0;
    pTriangles = pSectors + *(unsigned short *)(pData + 0x1e) * 0x88 + *(unsigned short *)(pData + 0x28) * 0x1c;
    pVertexData = pTriangles + *(int *)(pData + 0x20) * 0x4c;
    pLightLevels = pVertexData + *(int *)(pData + 0x14) * 0x30;
    pVertexFlags = pLightLevels + *(int *)(pData + 0x14) * 4;
    textureRecords = *(int *)(pData + 0xc) + (int)pData;
    recordSize = *(unsigned short *)(pData + 0x26);
    pRecords = pVertexFlags + *(int *)(pData + 0x10) * 0x14;
    if (*(int *)(pData + 0x20) != 0) {
        pField = (int *)(pTriangles + 4);
        do {
            k = 10;
            do {
                if (*pField != -1)
                    *pField += (int)CGraphics::m_textureCount;
                pField++;
                k--;
            } while (k != 0);
            count++;
            pField += 0x13 - 10; // Ten of the triangle's 19 dwords were consumed above.
        } while (count < *(unsigned int *)(pData + 0x20));
    }
    FUN_004b9910((int)(pRecords + recordSize * 0x5c), textureRecords,
                 *(unsigned short *)(pData + 0x24), *(int *)(pData + 4), param_3);
    savedNodeCount = g_sceneNodeCount;
    if (*(unsigned short *)(pData + 0x18) != 0) {
        for (i = 0, p = pNodes; i < *(unsigned short *)(pData + 0x18); i++, p += 0x18c) {
            FUN_004b98f0((int *)p, (int)pNodes);
            FUN_004b98f0((int *)(p + 4), (int)pNodes);
            if (param_2 == 0 || *(short *)(p + 0x24) != -1) {
                FUN_004b98f0((int *)(p + 8), (int)pNodes);
                *(short *)(p + 0x24) = -1;
            } else {
                SceneNode_Attach((SceneNode *)p, (SceneNode *)param_2);
            }
            FUN_004b98f0((int *)(p + 0x170), (int)pNodes);
            FUN_004b98f0((int *)(p + 0xc), (int)pMeshArray);
            g_sceneNodes[g_sceneNodeCount + i] = (SceneNode *)p;
            *(short *)(p + 0x26) = (short)(g_sceneNodeCount + i);
        }
        g_sceneNodeCount += *(unsigned short *)(pData + 0x18);
    }
    if (*(short *)(pData + 0x26) != 0) {
        for (j = 0, p = pRecords; j < *(unsigned short *)(pData + 0x26); j++, p += 0x5c) {
            FUN_004b98f0((int *)(p + 0x58), (int)pRecords);
            g_unk0x0066f124[g_meshTotalSize + j] = p;
        }
        g_meshTotalSize += *(unsigned short *)(pData + 0x26);
    }
    if (*(short *)(pData + 0x1a) != 0) {
        for (i = 0, p = pMeshArray + 0x113; i < *(unsigned short *)(pData + 0x1a); i++, p += 0x120) {
            j = 0;
            if (*p != 0) {
                pField = (int *)(p - 0x107);
                do {
                    FUN_004b98f0(pField + 6, (int)pTriangles);
                    FUN_004b98f0(pField, (int)pVertexData);
                    FUN_004b98f0(pField + 10, (int)pLightLevels);
                    FUN_004b98f0(pField + 5, (int)pVertexFlags);
                    Mesh_UploadVertices((Mesh *)((BYTE *)pField - 0xc));
                    Mesh_BuildParts((Mesh *)((BYTE *)pField - 0xc));
                    *(unsigned int *)((BYTE *)pField + 0x24) &= 0xfffbffff;
                    j++;
                    pField += 0x42;
                } while (j < *p);
            }
            FUN_004b98f0((int *)(p - 7), (int)pRecords);
            if (*(short *)(pData + 0x1e) == 0)
                g_meshes[g_meshCount + i] = (Mesh *)(p - 0x113);
        }
        if (*(short *)(pData + 0x1e) == 0)
            g_meshCount += *(unsigned short *)(pData + 0x1a);
    }
    if (*(unsigned short *)(pData + 0x1c) != 0) {
        for (j = *(unsigned short *)(pData + 0x1a), p = pMeshArray + j * 0x120 + 0x113;
             j < *(unsigned short *)(pData + 0x1c) + *(unsigned short *)(pData + 0x1a);
             j++, p += 0x120) {
            i = 0;
            if (*p != 0) {
                pField = (int *)(p - 0x107);
                do {
                    FUN_004b98f0(pField + 6, (int)pTriangles);
                    FUN_004b98f0(pField, (int)pVertexData);
                    FUN_004b98f0(pField + 10, (int)pLightLevels);
                    FUN_004b98f0(pField + 5, (int)pVertexFlags);
                    Mesh_UploadVertices((Mesh *)((BYTE *)pField - 0xc));
                    Mesh_BuildParts((Mesh *)((BYTE *)pField - 0xc));
                    *(unsigned int *)((BYTE *)pField + 0x24) &= 0xfffbffff;
                    i++;
                    pField += 0x42;
                } while (i < *p);
            }
            FUN_004b98f0((int *)(p - 7), (int)pRecords);
        }
        i = 0;
        if (*(short *)(pData + 0x1c) != 0) {
            for (p = pStageObjects + 0x98; i < *(unsigned short *)(pData + 0x1c); i++, p += 0xa0) {
                FUN_004b98f0((int *)(p - 0x8c),
                             (int)(pMeshArray + *(unsigned short *)(pData + 0x1a) * 0x120));
                FUN_004b98f0((int *)p, (int)pStageObjects);
                g_stageObjects[g_unk0x0067f228 + i] = (StageObject *)(p - 0x98);
            }
        }
        g_unk0x0067f228 += *(unsigned short *)(pData + 0x1c);
    }
    if (*(short *)(pData + 0x1e) != 0) {
        for (i = 0, p = pSectors; i < *(unsigned short *)(pData + 0x1e); i++, p += 0x88) {
            FUN_004b98f0((int *)(p + 0x10), (int)pMeshArray);
            g_sectors[i] = (Sector *)p;
        }
        g_sectorCount = *(unsigned short *)(pData + 0x1e);
    }
    return g_sceneNodes[savedNodeCount];
}

// Removes a scene node from the linked list of its current sector.
// FUNCTION: CMR2 0x004b8aa0
void Sector_RemoveNode(SceneNode *pNode)
{
    short sectorIndex = pNode->sector;
    Sector *sector = g_sectors[sectorIndex];
    SceneNode *current = sector->pFirstNode;
    SceneNode *previous = NULL;
    if (current != pNode) {
        do {
            previous = current;
            current = previous->pNextInSector;
        } while (current != pNode);
    }
    if (previous == NULL)
        sector->pFirstNode = current->pNextInSector;
    else
        previous->pNextInSector = current->pNextInSector;
    g_sectors[sectorIndex]->nodeCount--;
    current->pNextInSector = NULL;
    current->sector = -1;
}

// FUNCTION: CMR2 0x004b7da0
int Sector_IsVisible(int iSector)
{
    if (g_sectorCullDisabled == 0 &&
        (g_sectorVisibleBits[iSector >> 5] & (1 << (iSector & 0x1f))) <= 0u &&
        g_sectorCullEnabled != 0)
        return 0;
    return 1;
}

#include "FileBuffer.h"

// Frees the part index lists of every sector mesh and clears the sector
// table (registered callback of 0x4b8270).
// FUNCTION: CMR2 0x004b8540
BYTE FUN_004b8540(void)
{
    int i;
    int j;
    BYTE *pModel;
    BYTE **ppPart;

    for (i = 0; i < g_sectorCount; i++) {
        pModel = (BYTE *)g_sectors[i]->pMesh;
        if (pModel != NULL) {
            j = 0;
            if (*(int *)(pModel + 0x100) > 0) {
                ppPart = (BYTE **)(pModel + 0x38);
                do {
                    CFileBuffer::FreeGenericFileBuffer(*(void **)(*ppPart + 0x14));
                    *(void **)(*ppPart + 0x14) = NULL;
                    CFileBuffer::FreeGenericFileBuffer(*ppPart);
                    *ppPart = NULL;
                    j++;
                    ppPart++;
                } while (j < *(int *)(pModel + 0x100));
            }
        }
    }
    CGraphics::m_unk0x0072d56c = 0;
    memset(g_sectors, 0, 0x1000 * sizeof(Sector *));
    g_sectorCount = 0;
    g_sectorCullEnabled = 0;
    return 1;
}

// FUNCTION: CMR2 0x004b85f0
int Sector_FromPosition(FixVector *pPos)
{
    int offset;
    int row;
    int col;
    int iSector;

    // the subtrahend order matters: MSVC evaluates these two subtractions
    // right to left, and the original binary has (z - halfSize) - originZ.
    offset = (pPos->z - g_sectorOriginZ - g_sectorHalfSize < 0) ?
             g_sectorOriginZ - pPos->z + g_sectorHalfSize :
             pPos->z - g_sectorOriginZ - g_sectorHalfSize;
    row = (FixMul(offset, g_sectorScale) >> 16);

    offset = (pPos->x - g_sectorOriginX + g_sectorHalfSize < 0) ?
             g_sectorOriginX - pPos->x - g_sectorHalfSize :
             pPos->x - g_sectorOriginX + g_sectorHalfSize;
    col = (FixMul(offset, g_sectorScale) >> 16);

    iSector = g_sectorsPerRow * row + col;
    if ((short)iSector < 0 || (short)iSector >= (short)g_sectorCount)
        iSector = 0;
    return iSector;
}

// Re-evaluates which sector the node's world position falls in, records
// the neighbouring sectors it overlaps (within 4.5 units) and moves the node
// between the sector lists.
// match 89%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004b8690
void SceneNode_UpdateSector(SceneNode *pNode)
{
    int n;
    short iSector;
    FixVector pos;
    int bLeft;
    int bRight;
    Sector *pSector;
    int c;
    SceneNode *p;
    SceneNode *pPrev;
    n = 0;
    iSector = pNode->sector;
    bLeft = 0;
    bRight = 0;
    pSector = g_sectors[iSector];
    pNode->neighbourSectors[0] = -1;
    pNode->neighbourSectors[1] = -1;
    pNode->neighbourSectors[2] = -1;
    c = pSector->x;
    if (pNode->world.position.x < c - g_sectorHalfSize || pNode->world.position.x > c + g_sectorHalfSize ||
        (c = pSector->z, pNode->world.position.z < c - g_sectorHalfSize) ||
        pNode->world.position.z > c + g_sectorHalfSize) {
        pos.x = pNode->world.position.x;
        pos.y = pNode->world.position.y;
        pos.z = pNode->world.position.z;
        iSector = (short)Sector_FromPosition(&pos);
    }
    pSector = g_sectors[iSector];
    c = pSector->x;
    if (pNode->world.position.x - 0x48000 < c - g_sectorHalfSize) {
        bLeft = 1;
        pNode->neighbourSectors[0] = iSector - 1;
        n = 1;
    } else if (pNode->world.position.x + 0x48000 > c + g_sectorHalfSize) {
        bRight = 1;
        pNode->neighbourSectors[0] = iSector + 1;
        n = 1;
    }
    c = pSector->z;
    if (pNode->world.position.z + 0x48000 > g_sectorHalfSize + c) {
        pNode->neighbourSectors[n++] = iSector - (short)g_sectorsPerRow;
        if (bLeft)
            pNode->neighbourSectors[n] = iSector - (short)g_sectorsPerRow - 1;
        else if (bRight)
            pNode->neighbourSectors[n] = iSector - (short)g_sectorsPerRow + 1;
    } else if (pNode->world.position.z - 0x48000 < c - g_sectorHalfSize) {
        pNode->neighbourSectors[n++] = iSector + (short)g_sectorsPerRow;
        if (bLeft)
            pNode->neighbourSectors[n] = iSector + (short)g_sectorsPerRow - 1;
        else if (bRight)
            pNode->neighbourSectors[n] = iSector + (short)g_sectorsPerRow + 1;
    }

    if (iSector != pNode->sector && iSector >= 0 && iSector < g_sectorCount) {
        pPrev = NULL;
        p = g_sectors[pNode->sector]->pFirstNode;
        if (p != NULL) {
            while (p != pNode) {
                if (p == NULL)
                    break;
                pPrev = p;
                p = p->pNextInSector;
            }
            if (p != NULL) {
                if (pPrev == NULL)
                    g_sectors[pNode->sector]->pFirstNode = p->pNextInSector;
                else
                    pPrev->pNextInSector = p->pNextInSector;
                p->pNextInSector = NULL;
                g_sectors[pNode->sector]->nodeCount--;
            }
            p = g_sectors[iSector]->pFirstNode;
            if (p == NULL) {
                g_sectors[iSector]->pFirstNode = pNode;
            } else {
                for (pPrev = p->pNextInSector; pPrev != NULL; pPrev = pPrev->pNextInSector)
                    p = pPrev;
                p->pNextInSector = pNode;
            }
            g_sectors[iSector]->nodeCount++;
            pNode->sector = iSector;
        }
    }

}


// Finds the ground vertex of a sector nearest (in x/z) to one of its corners
// (0: -x+z, 1: +x+z, 2: +x-z, 3: -x-z), stores its offset from that corner
// and returns its height (0 when the first vertex is the nearest).
// FUNCTION: CMR2 0x004b8e30
int Sector_NearestCornerHeight(unsigned int side, int index)
{
    int cx;
    int cz;
    int height;
    unsigned int i;
    float *pVert;
    SectorMesh *pMesh;
    FixVector best;
    FixVector d;

    cx = 0;
    cz = 0;
    switch (side) {
    case 0:
        cx = g_sectors[index]->x - g_sectorHalfSize;
        cz = g_sectors[index]->z + g_sectorHalfSize;
        break;
    case 1:
        cx = g_sectors[index]->x + g_sectorHalfSize;
        cz = g_sectors[index]->z + g_sectorHalfSize;
        break;
    case 2:
        cx = g_sectors[index]->x + g_sectorHalfSize;
        cz = g_sectors[index]->z - g_sectorHalfSize;
        break;
    case 3:
        cx = g_sectors[index]->x - g_sectorHalfSize;
        cz = g_sectors[index]->z - g_sectorHalfSize;
        break;
    }
    height = 0;
    best.x = g_sectors[index]->corners[side].x - cx;
    best.y = 0;
    best.z = g_sectors[index]->corners[side].z - cz;
    pMesh = &((SectorMesh *)g_sectors[index]->pMesh)[g_sectors[index]->pMesh->lodIndex];
    pVert = pMesh->pVertices;
    best.x = -cx - (int)(__int64)(pVert[0] * -65536.0);
    best.y = 0;
    best.z = -cz - (int)(__int64)(pVert[2] * -65536.0);
    for (i = 1; i < ((SectorMesh *)g_sectors[index]->pMesh)[g_sectors[index]->pMesh->lodIndex].vertexCount; i++) {
        d.x = -cx - (int)(__int64)(pVert[0] * -65536.0);
        d.y = 0;
        d.z = -cz - (int)(__int64)(pVert[2] * -65536.0);
        if (FixVecLength(&d) < FixVecLength(&best)) {
            height = (int)(__int64)(pVert[1] * 65536.0);
            best = d;
        }
        pVert += 12;
    }
    g_sectors[index]->corners[side].x = best.x;
    g_sectors[index]->corners[side].z = best.z;
    return height;
}

// Sets the four corner points of every sector from the ground mesh heights
// around each grid vertex (lowest nearby vertex plus one unit, or 10 units
// when no neighbouring sector has a mesh).
// FUNCTION: CMR2 0x004b8b90
void Sector_BuildCorners(void)
{
    FixVector pos;
    unsigned int row;
    unsigned int col;
    int rowStart;
    int cur;
    int left;
    int above;
    int aboveLeft;
    int h;
    int height;
    int y;
    int hasAboveLeft;
    int hasAbove;
    int hasLeft;
    int hasCur;

    height = 0x7fff0000;
    pos.x = g_sectors[0]->x - g_sectorHalfSize;
    pos.z = g_sectors[0]->z + g_sectorHalfSize;
    for (row = 0; row < (unsigned int)g_sectorRows + 1; row++) {
        above = (row - 1) * g_sectorsPerRow;
        rowStart = g_sectorsPerRow * row;
        cur = rowStart;
        aboveLeft = -1;
        left = -1;
        for (col = 0; col < (unsigned int)g_sectorsPerRow + 1; col++) {
            if (cur >= g_sectorCount || above >= g_sectorCount)
                break;
            hasAboveLeft = 0;
            hasAbove = 0;
            hasLeft = 0;
            hasCur = 0;
            if (aboveLeft >= 0) {
                if (g_sectors[aboveLeft]->pMesh != NULL) {
                    h = Sector_NearestCornerHeight(2, aboveLeft);
                    if (h < 0x7fff0000)
                        height = h;
                }
                hasAboveLeft = 1;
            }
            if (above >= 0 && above <= rowStart - 1) {
                if (g_sectors[above]->pMesh != NULL) {
                    h = Sector_NearestCornerHeight(3, above);
                    if (h < height)
                        height = h;
                }
                hasAbove = 1;
            }
            if (left >= 0 && left < g_sectorCount) {
                if (g_sectors[left]->pMesh != NULL) {
                    h = Sector_NearestCornerHeight(1, left);
                    if (h < height)
                        height = h;
                }
                hasLeft = 1;
            }
            if (cur >= 0 && (unsigned int)(cur - rowStart) <= (unsigned int)g_sectorsPerRow) {
                if (g_sectors[cur]->pMesh != NULL) {
                    h = Sector_NearestCornerHeight(0, cur);
                    if (h < height)
                        height = h;
                }
                hasCur = 1;
            }
            if (height == 0x7fff0000)
                y = 0xa0000;
            else
                y = height + 0x10000;
            height = 0x7fff0000;
            if (hasAboveLeft) {
                g_sectors[aboveLeft]->corners[2].x = pos.x;
                g_sectors[aboveLeft]->corners[2].y = y;
                g_sectors[aboveLeft]->corners[2].z = pos.z;
            }
            if (hasAbove) {
                g_sectors[above]->corners[3].x = pos.x;
                g_sectors[above]->corners[3].y = y;
                g_sectors[above]->corners[3].z = pos.z;
            }
            if (hasLeft) {
                g_sectors[left]->corners[1].x = pos.x;
                g_sectors[left]->corners[1].y = y;
                g_sectors[left]->corners[1].z = pos.z;
            }
            if (hasCur) {
                g_sectors[cur]->corners[0].x = pos.x;
                g_sectors[cur]->corners[0].y = y;
                g_sectors[cur]->corners[0].z = pos.z;
            }
            aboveLeft = above;
            above++;
            left = cur;
            cur++;
            pos.x += g_sectorSize;
        }
        pos.x = g_sectors[0]->x - g_sectorHalfSize;
        pos.z -= g_sectorSize;
    }
}

// Appends the node to the sector its world position falls in.
// FUNCTION: CMR2 0x004b8b10
void FUN_004b8b10(SceneNode *pNode)
{
    FixVector pos;
    int index;
    Sector *pSector;
    SceneNode *pLast;

    if (pNode == NULL)
        return;
    pos.x = pNode->world.position.x;
    pos.y = pNode->world.position.y;
    pos.z = pNode->world.position.z;
    index = (short)Sector_FromPosition(&pos);
    pSector = g_sectors[index];
    pLast = pSector->pFirstNode;
    if (pLast == NULL)
        pSector->pFirstNode = pNode;
    else {
        for (; pLast->pNextInSector != NULL;
             pLast = pLast->pNextInSector)
            ;
        pLast->pNextInSector = pNode;
    }
    g_sectors[index]->nodeCount++;
    pNode->sector = (WORD)index;

}

// Visible sector indices collected by 0x004b7de0 (one short per sector).
// GLOBAL: CMR2 0x006ed5f0
unsigned short g_unk0x006ed5f0[4096];
int Tri2D_Contains(int *pPoint, int *pTri);

// Sector culling pass of a scene node: walks the sector grid around its world
// position and marks every sector whose bounding rectangle overlaps the screen
// triangle built from the node position and the "radius" (the fixed far plane
// distance), storing the squared distance of each marked sector.
// match 68%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004b7de0
void FUN_004b7de0(SceneNode *pNode, int unused)
{
    FixVector pos;
    FixVector dir;
    FixVector origin;
    FixVector forward;
    FixVector scaled;
    int tri[6];
    short iSector;
    int radius;
    int spread;
    int rem;
    int quo;
    int colStart;
    int colEnd;
    int rowStart;
    int rowEnd;
    int row;
    int col;
    int index;
    int len;
    int ex;
    int ez;
    int fx;
    int fz;
    int dx;
    int dz;

    radius = CGraphics::m_farPlaneFixed;
    memset(g_sectorVisibleBits, 0, sizeof(g_sectorVisibleBits));
    g_sectorCullEnabled = 0;
    pos.y = 0;
    pos.x = pNode->world.position.x;
    pos.z = pNode->world.position.z;
    spread = FixMulShift32(radius, g_sectorScale);
    iSector = (short)Sector_FromPosition(&pos);
    rem = iSector % g_sectorsPerRow;
    quo = iSector / g_sectorsPerRow;
    colStart = rem - spread - 1;
    if (colStart < 0)
        colStart = 0;
    colEnd = rem + spread + 2;
    if (colEnd > g_sectorsPerRow)
        colEnd = g_sectorsPerRow;
    rowStart = quo - spread - 1;
    if (rowStart < 0)
        rowStart = 0;
    rowEnd = quo + spread + 2;
    if (rowEnd > g_sectorRows)
        rowEnd = g_sectorRows;
    dir.x = pNode->world.forward.x;
    dir.y = 0;
    dir.z = pNode->world.forward.z;
    len = FixVecLength(&dir);
    if (len == 0) {
        dir.x = 0;
        dir.y = 0;
        dir.z = 0;
    } else {
        FixVecScaleRecip(&dir, &dir, len);
    }
    tri[0] = pNode->world.position.x;
    tri[1] = pNode->world.position.z;
    ex = FixMul(dir.x, radius);
    ez = FixMul(dir.z, radius);
    fz = FixMul(dir.z, FixMul(radius, 0x10000));
    fx = -FixMul(dir.x, FixMul(radius, 0x10000));
    tri[4] = tri[0] + fz + ex;
    tri[5] = tri[1] + fx + ez;
    tri[2] = tri[0] - fz + ex;
    tri[3] = tri[1] - fx + ez;
    FixMatrix_GetPosition(&origin, &pNode->world);
    FixMatrix_GetForward(&forward, &pNode->world);
    FixVecScale(&scaled, &forward, 0xa0000);
    origin.x = origin.x - scaled.x;
    origin.y = origin.y - scaled.y;
    origin.z = origin.z - scaled.z;
    for (row = rowStart; row < rowEnd; row++) {
        for (col = colStart; col < colEnd; col++) {
            index = row * g_sectorsPerRow + col;
            if (Tri2D_Contains(g_sectors[index]->bounds[2], tri) ||
                Tri2D_Contains(g_sectors[index]->bounds[3], tri) ||
                Tri2D_Contains(g_sectors[index]->bounds[1], tri) ||
                Tri2D_Contains(g_sectors[index]->bounds[0], tri) ||
                index == iSector - g_sectorsPerRow - 1 || index == iSector - g_sectorsPerRow ||
                index == iSector - g_sectorsPerRow + 1 || index == iSector - 1 || index == iSector ||
                index == iSector + 1 || index == iSector + g_sectorsPerRow - 1 ||
                index == iSector + g_sectorsPerRow || index == iSector + g_sectorsPerRow + 1) {
                dx = g_sectors[index]->x - tri[0];
                dz = g_sectors[index]->z - tri[1];
                *(int *)((BYTE *)g_sectors[index] + 0x7c) = FixMul(dx, dx) + FixMul(dz, dz);
                if (index < g_sectorCount && index >= 0) {
                    g_unk0x006ed5f0[g_sectorCullEnabled] = (short)index;
                    g_sectorCullEnabled++;
                    *(int *)((BYTE *)g_sectors[index] + 0x80) = 0;
                    *(int *)((BYTE *)g_sectors[index] + 0x84) = 1;
                    g_sectorVisibleBits[index >> 5] |= 1 << (index & 0x1f);
                }
            }
        }
    }
}

void Sector_RebuildNodeLists(void);
void Sector_ComputeBounds(void);
extern int g_unk0x0067f228;
extern int g_finCount;
extern BYTE *g_finData;

// Rebuilds the whole sector grid of the stage that was just loaded: the cell
// size, the scale used to convert world coordinates to grid coordinates, the
// grid origin and row count, the stage object list of every sector and the
// sector indices of the fin.dat records.
// match 56%: same logic and call order; MSVC6 keeps g_sectors[0] and the two
// loop counters in different registers than the original, so most of the diff
// is register renaming
// match 56%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004b8270
void FUN_004b8270(void)
{
    int i;
    int n;
    int minX;
    int maxZ;
    int halfSize;
    Sector **ppSector;

    CGraphics::m_unk0x0072d56c = 1;
    g_sectorSize = abs(g_sectors[1]->x - g_sectors[0]->x);
    g_sectorHalfSize = FixMul(g_sectorSize, 0x8000);
    g_sectorScale = FixDiv(0x10000, g_sectorSize);
    minX = g_sectors[0]->x;
    g_sectorOriginX = minX;
    maxZ = g_sectors[0]->z;
    g_sectorOriginZ = maxZ;
    if ((unsigned int)g_sectorCount > 1) {
        n = g_sectorCount - 1;
        ppSector = &g_sectors[1];
        do {
            if (minX > (*ppSector)->x) {
                minX = (*ppSector)->x;
                g_sectorOriginX = minX;
            }
            if (maxZ < (*ppSector)->z) {
                maxZ = (*ppSector)->z;
                g_sectorOriginZ = maxZ;
            }
            ppSector++;
        } while (--n != 0);
    }
    halfSize = g_sectorSize / 2;
    g_unk0x006ed5e8 = minX - halfSize;
    g_unk0x006ed5ec = halfSize + maxZ;
    g_sectorsPerRow = 1;
    if (g_sectors[0]->z == g_sectors[1]->z) {
        i = 1;
        while (i < g_sectorCount - 1) {
            g_sectorsPerRow = i + 1;
            if (g_sectors[i]->z != g_sectors[i + 1]->z)
                break;
            i++;
        }
    }
    g_sectorRows = (unsigned int)g_sectorCount / (unsigned int)g_sectorsPerRow;
    for (i = 0; i < (unsigned int)g_unk0x0067f228; i++) {
        short index = (short)Sector_FromPosition((FixVector *)g_stageObjects[i]);
        g_stageObjects[i]->pNext = g_sectors[index]->pObjects;
        g_sectors[index]->pObjects = g_stageObjects[i];
        g_sectors[index]->field_0x18++;
    }
    Sector_RebuildNodeLists();
    if (g_finData != NULL) {
        for (i = 0; i < (unsigned int)g_finCount; i++) {
            Sector *pSector = g_sectors[*(int *)(g_finData + i * 0x30 + 0x28)];
            *(int *)(g_finData + i * 0x30 + 0x28) = (int)pSector->pFinRecords;
            pSector->pFinRecords = g_finData + i * 0x30;
            pSector->finCount++;
        }
    }
    Sector_BuildCorners();
    Sector_ComputeBounds();
    CGame::RegisterCallback(FUN_004b8540, NULL);
}

// Rebuilds the node list of every sector from the positions of the root's children.
// FUNCTION: CMR2 0x004b8450
void Sector_RebuildNodeLists(void)
{
    unsigned int i;
    Sector **ppSector;
    SceneNode *pNode;
    SceneNode *pLast;
    FixVector pos = { 0, 0, 0 };
    int index;
    for (i = 0; i < (unsigned int)g_sceneNodeCount; i++) {
        if (g_sceneNodes[i] != NULL)
            g_sceneNodes[i]->pNextInSector = NULL;
    }
    ppSector = g_sectors;
    do {
        if (*ppSector != NULL) {
            (*ppSector)->pFirstNode = NULL;
            (*ppSector)->nodeCount = 0;
        }
        ppSector++;
    } while (ppSector < &g_sectors[4096]);
    for (pNode = CGraphics::m_pTextureManager->pRootNode->pFirstChild; pNode != NULL; pNode = pNode->pNext) {
        if ((BYTE)pNode->flags != 0xff) {
            pos.x = pNode->current.position.x;
            pos.y = pNode->current.position.y;
            pos.z = pNode->current.position.z;
            pNode->sector = (short)Sector_FromPosition(&pos);
            index = pNode->sector;
            if (g_sectors[index]->pFirstNode == NULL) {
                g_sectors[index]->pFirstNode = pNode;
            } else {
                for (pLast = g_sectors[index]->pFirstNode; pLast->pNextInSector != NULL; pLast = pLast->pNextInSector)
                    ;
                pLast->pNextInSector = pNode;
            }
            g_sectors[index]->nodeCount++;
            pNode->sector = (short)index;
        }
    }

}

// Up to three sectors next to the one containing pPos that lie within 4.5
// units of it (left/right, above/below and the diagonal); -1 when unused.
// Returns the sector id of pPos (its callers store it in the car's own field).
// FUNCTION: CMR2 0x004b8910
short Sector_GetNeighbours(FixVector *pPos, short *pOut)
{
    Sector *pSector;
    short index;
    int left;
    int right;
    unsigned int k;

    left = 0;
    right = 0;
    k = 0;
    pOut[0] = -1;
    pOut[1] = -1;
    pOut[2] = -1;
    index = (short)Sector_FromPosition(pPos);
    pSector = g_sectors[index];
    if (pPos->x - 0x48000 < pSector->x - g_sectorHalfSize) {
        left = 1;
        pOut[0] = index - 1;
        k = 1;
    } else if (pPos->x + 0x48000 > pSector->x + g_sectorHalfSize) {
        right = 1;
        pOut[0] = index + 1;
        k = 1;
    }
    if (pPos->z + 0x48000 > g_sectorHalfSize + pSector->z) {
        pOut[k++] = index - g_sectorsPerRow;
        if (left)
            pOut[k] = index - g_sectorsPerRow - 1;
        else if (right)
            pOut[k] = index - g_sectorsPerRow + 1;
    } else if (pPos->z - 0x48000 < pSector->z - g_sectorHalfSize) {
        pOut[k++] = g_sectorsPerRow + index;
        if (left)
            pOut[k] = g_sectorsPerRow - 1 + index;
        else if (right)
            pOut[k] = g_sectorsPerRow + 1 + index;
    }
    if (pOut[0] >= (short)g_sectorCount || pOut[0] < -1)
        pOut[0] = -1;
    if (pOut[1] >= (short)g_sectorCount || pOut[1] < -1)
        pOut[1] = -1;
    if (pOut[2] >= (short)g_sectorCount || pOut[2] < -1)
        pOut[2] = -1;
    return index;
}

// Bounding rectangle (x/z) of each sector's ground mesh, or of the sector
// square when it has none.
// FUNCTION: CMR2 0x004b9170
void Sector_ComputeBounds(void)
{
    Sector **ppSector;
    Sector *pSector;
    SectorMesh *pMesh;
    float *pVertex;
    int minX, maxX, minZ, maxZ;
    int n;
    int i;

    ppSector = g_sectors;
    for (i = 0; i < g_sectorCount; i++, ppSector++) {
        pSector = *ppSector;
        if (pSector->pMesh == NULL) {
            pSector->bounds[0][0] = pSector->x - g_sectorHalfSize;
            (*ppSector)->bounds[0][1] = (*ppSector)->z - g_sectorHalfSize;
            (*ppSector)->bounds[1][0] = g_sectorHalfSize + (*ppSector)->x;
            (*ppSector)->bounds[1][1] = (*ppSector)->z - g_sectorHalfSize;
            (*ppSector)->bounds[3][0] = (*ppSector)->x - g_sectorHalfSize;
            (*ppSector)->bounds[3][1] = (*ppSector)->z + g_sectorHalfSize;
            (*ppSector)->bounds[2][0] = g_sectorHalfSize + (*ppSector)->x;
            (*ppSector)->bounds[2][1] = (*ppSector)->z + g_sectorHalfSize;
        } else {
            pMesh = &((SectorMesh *)pSector->pMesh)[pSector->pMesh->lodIndex];
            pVertex = pMesh->pVertices;
            n = pMesh->vertexCount;
            minX = (int)(__int64)pVertex[0];
            maxX = minX;
            minZ = maxZ = (int)(__int64)pVertex[2];
            for (pVertex += 12; n > 1; n--, pVertex += 12) {
                if (pVertex[0] < (float)minX)
                    minX = (int)(__int64)pVertex[0];
                if ((float)maxX < pVertex[0])
                    maxX = (int)(__int64)pVertex[0];
                if ((float)maxZ < pVertex[2])
                    maxZ = (int)(__int64)pVertex[2];
                if (pVertex[2] < (float)minZ)
                    minZ = (int)(__int64)pVertex[2];
            }
            minX = (int)(__int64)((double)minX * CGraphics::m_65536);
            pSector->bounds[0][0] = minX;
            minZ = (int)(__int64)((double)minZ * CGraphics::m_65536);
            (*ppSector)->bounds[0][1] = minZ;
            maxX = (int)(__int64)((double)maxX * CGraphics::m_65536);
            (*ppSector)->bounds[1][0] = maxX;
            (*ppSector)->bounds[1][1] = minZ;
            (*ppSector)->bounds[3][0] = minX;
            maxZ = (int)(__int64)((double)maxZ * CGraphics::m_65536);
            (*ppSector)->bounds[3][1] = maxZ;
            (*ppSector)->bounds[2][0] = maxX;
            (*ppSector)->bounds[2][1] = maxZ;
        }
    }
}
