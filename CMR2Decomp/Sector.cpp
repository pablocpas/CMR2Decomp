#include <windows.h>
#include "Sector.h"
#include "SceneNode.h"

int g_sectorsPerRow;
Sector *g_sectors[14096];
int g_sectorHalfSize;
int g_sectorVisibleBits[198];
int g_sectorScale;
int g_sectorCount;
int g_sectorCullEnabled;
int g_sectorCullDisabled;
int g_sectorOriginX;
int g_sectorOriginZ;

// FUNCTION: CMR2 0x004b7da0
int Sector_IsVisible(int iSector)
{
    if (g_sectorCullDisabled == 0 &&
        (g_sectorVisibleBits[iSector >> 5] & (1 << (iSector & 0x1f))) == 0 &&
        g_sectorCullEnabled != 0)
        return 0;
    return 1;
}

// FUNCTION: CMR2 0x004b85f0
int Sector_FromPosition(FixVector *pPos)
{
    int offset;
    int row;
    int col;
    int iSector;

    offset = pPos->z - g_sectorHalfSize - g_sectorOriginZ;
    if (offset < 0)
        offset = g_sectorHalfSize - pPos->z + g_sectorOriginZ;
    row = FixMulShift32(offset, g_sectorScale);

    offset = pPos->x - g_sectorOriginX + g_sectorHalfSize;
    if (offset < 0)
        offset = g_sectorOriginX - pPos->x - g_sectorHalfSize;
    col = FixMulShift32(offset, g_sectorScale);

    iSector = g_sectorsPerRow * row + col;
    if ((short)iSector < 0 || (short)iSector >= (short)g_sectorCount)
        iSector = 0;
    return iSector;
}

// Re-evaluates which sector the node's world position falls in, records
// the neighbouring sectors it overlaps (within 4.5 units) and moves the node
// between the sector lists.
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
    pos.x = pNode->world.position.x;
    bLeft = 0;
    bRight = 0;
    pSector = g_sectors[iSector];
    pNode->neighbourSectors[0] = -1;
    pNode->neighbourSectors[1] = -1;
    pNode->neighbourSectors[2] = -1;
    c = pSector->x;
    if (pos.x < c - g_sectorHalfSize || c + g_sectorHalfSize < pos.x ||
        (c = pSector->z, pNode->world.position.z < c - g_sectorHalfSize) ||
        c + g_sectorHalfSize < pNode->world.position.z) {
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
    } else if (c + g_sectorHalfSize < pNode->world.position.x + 0x48000) {
        bRight = 1;
        pNode->neighbourSectors[0] = iSector + 1;
        n = 1;
    }
    c = pSector->z;
    if (c + g_sectorHalfSize < pNode->world.position.z + 0x48000) {
        pNode->neighbourSectors[n] = iSector - (short)g_sectorsPerRow;
        if (bLeft)
            pNode->neighbourSectors[n + 1] = iSector - (short)g_sectorsPerRow - 1;
        else if (bRight)
            pNode->neighbourSectors[n + 1] = iSector - (short)g_sectorsPerRow + 1;
    } else if (pNode->world.position.z - 0x48000 < c - g_sectorHalfSize) {
        pNode->neighbourSectors[n] = iSector + (short)g_sectorsPerRow;
        if (bLeft)
            pNode->neighbourSectors[n + 1] = iSector + (short)g_sectorsPerRow - 1;
        else if (bRight)
            pNode->neighbourSectors[n + 1] = iSector + (short)g_sectorsPerRow + 1;
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
    if (pSector->pFirstNode == NULL) {
        pSector->pFirstNode = pNode;
    } else {
        pLast = pSector->pFirstNode;
        while (pLast->pNextInSector != NULL)
            pLast = pLast->pNextInSector;
        pLast->pNextInSector = pNode;
    }
    g_sectors[index]->nodeCount++;
    pNode->sector = (WORD)index;
}

// Recorre los sectores activos y actualiza la visibilidad de sus nodos.
// TODO: CMR2 0x0049d290 (implemented, match 7%)
void FUN_0049d290(int param1)
{
    unsigned short *pIndex;
    BYTE *pSector;
    BYTE *pNode;
    int i;
    int count;

    count = *(int *)0x72d570;
    if (count <= 0)
        return;
    pIndex = (unsigned short *)0x6ed5f0;
    for (i = 0; i < count; i++) {
        pSector = *(BYTE **)(0x71f608 + (unsigned int)*pIndex * 4);
        pNode = *(BYTE **)(pSector + 0x1c);
        if (pNode != NULL && *(int *)(pNode + 0x180) != 0) {
            BYTE *pRef = *(BYTE **)0x59be6c;

            *(int *)(pNode + 0x108) = *(int *)(pRef + 0x108);
            *(int *)(pNode + 0x10c) = *(int *)(pRef + 0x10c);
            *(int *)(pNode + 0x110) = *(int *)(pRef + 0x110);
        }
        pIndex++;
    }
}
