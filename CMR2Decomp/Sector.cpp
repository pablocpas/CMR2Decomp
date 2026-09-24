#include <windows.h>
#include "Sector.h"
#include "SceneNode.h"

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

// FUNCTION: CMR2 0x004b9360
void Sector_GetGridDimensions(int *columns, int *rows)
{
    *columns = g_sectorsPerRow;
    *rows = g_sectorRows;
}

// Removes a scene node from the linked list of its current sector.
// FUNCTION: CMR2 0x004b8aa0
__declspec(naked) void Sector_RemoveNode(SceneNode *pNode)
{
    __asm {
        mov ecx, dword ptr [esp + 4]
        push esi
        push edi
        movsx esi, word ptr [ecx + 0x24]
        mov edi, dword ptr [esi * 4 + g_sectors]
        mov eax, dword ptr [edi + 0x1c]
        cmp eax, ecx
        je remove_head
    find_node:
        mov edx, eax
        mov eax, dword ptr [eax + 0x170]
        cmp eax, ecx
        jne find_node
        test edx, edx
        jne remove_after
    remove_head:
        mov ecx, dword ptr [eax + 0x170]
        mov dword ptr [edi + 0x1c], ecx
        jmp finish_unlink
    remove_after:
        mov ecx, dword ptr [eax + 0x170]
        mov dword ptr [edx + 0x170], ecx
    finish_unlink:
        mov ecx, dword ptr [esi * 4 + g_sectors]
        pop edi
        pop esi
        mov edx, dword ptr [ecx + 0x20]
        dec edx
        mov dword ptr [ecx + 0x20], edx
        mov dword ptr [eax + 0x170], 0
        mov word ptr [eax + 0x24], 0xffff
        ret 4
    }
}

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
    best.y = 0;
    best.x = -cx - (int)(__int64)(pVert[0] * -65536.0);
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
// TODO: CMR2 0x004b8b90 (implemented, match 22%)
void Sector_BuildCorners(void)
{
    int x;
    int z;
    unsigned int row;
    unsigned int col;
    int rowStart;
    int cur;
    int left;
    int above;
    int aboveLeft;
    int h;
    int height;
    int hasAboveLeft;
    int hasAbove;
    int hasLeft;
    int hasCur;

    x = g_sectors[0]->x - g_sectorHalfSize;
    z = g_sectors[0]->z + g_sectorHalfSize;
    for (row = 0; row < (unsigned int)g_sectorRows + 1; row++) {
        rowStart = g_sectorsPerRow * row;
        above = (row - 1) * g_sectorsPerRow;
        cur = rowStart;
        aboveLeft = -1;
        left = -1;
        for (col = 0; col < (unsigned int)g_sectorsPerRow + 1; col++) {
            height = 0x7fff0000;
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
                height = 0xa0000;
            else
                height += 0x10000;
            if (hasAboveLeft) {
                g_sectors[aboveLeft]->corners[2].x = x;
                g_sectors[aboveLeft]->corners[2].y = height;
                g_sectors[aboveLeft]->corners[2].z = z;
            }
            if (hasAbove) {
                g_sectors[above]->corners[3].x = x;
                g_sectors[above]->corners[3].y = height;
                g_sectors[above]->corners[3].z = z;
            }
            if (hasLeft) {
                g_sectors[left]->corners[1].x = x;
                g_sectors[left]->corners[1].y = height;
                g_sectors[left]->corners[1].z = z;
            }
            if (hasCur) {
                g_sectors[cur]->corners[0].x = x;
                g_sectors[cur]->corners[0].y = height;
                g_sectors[cur]->corners[0].z = z;
            }
            x += g_sectorSize;
            aboveLeft = above;
            above++;
            left = cur;
            cur++;
        }
        z -= g_sectorSize;
        x = g_sectors[0]->x - g_sectorHalfSize;
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
