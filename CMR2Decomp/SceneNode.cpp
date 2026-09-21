#include <windows.h>
#include "SceneNode.h"
#include "FileBuffer.h"
#include "Graphics.h"

SceneNode *g_sceneNodes[4096];
int g_sceneNodeCount;

// Per-frame statistics of SceneNode_UpdateTree
// GLOBAL: CMR2 0x00683374
int g_sceneStatCopied;
// GLOBAL: CMR2 0x00683378
int g_sceneStatMultiplied;
// GLOBAL: CMR2 0x0068337c
int g_sceneStatClean;
// GLOBAL: CMR2 0x00683380
int g_sceneStatHidden;

unsigned short g_sqrtTable[4096];
int g_sinTable[4096];

// Rodrigues rotation scratch values (shared globals in the original)
// GLOBAL: CMR2 0x0067f250
int g_rotSin;
// GLOBAL: CMR2 0x0067f248
int g_rotCos;
// GLOBAL: CMR2 0x0068336c
int g_rotOneMinusCos;
// GLOBAL: CMR2 0x0067f254
int g_rotAxisXX;
// GLOBAL: CMR2 0x0067f258
int g_rotAxisYY;
// GLOBAL: CMR2 0x0067f25c
int g_rotAxisZZ;
// GLOBAL: CMR2 0x0067f238
int g_rotAxisXY;
// GLOBAL: CMR2 0x0067f244
int g_rotAxisXZ;
// GLOBAL: CMR2 0x0067f24c
int g_rotAxisYZ;

// Rotates vectors A and B about the unit axis K by angle (Rodrigues) and
// renormalises them. The original expands this three times over locals, so
// it is a macro rather than a function.
#define ROTATE_ABOUT_AXIS(kx, ky, kz, ax, ay, az, bx, by, bz, angle)                  \
    {                                                                                 \
        int r00, r01, r02, r10, r11, r12, r20, r21, r22;                              \
        FixVector v;                                                                  \
        int len;                                                                      \
                                                                                      \
        g_rotSin = FixSin(-(angle));                                                  \
        g_rotCos = FixCos(angle);                                                     \
        g_rotOneMinusCos = 0x10000 - g_rotCos;                                        \
        g_rotAxisXX = FixMul(kx, kx);                                                 \
        g_rotAxisYY = FixMul(ky, ky);                                                 \
        g_rotAxisZZ = FixMul(kz, kz);                                                 \
        g_rotAxisXY = FixMul(FixMul(kx, ky), g_rotOneMinusCos);                       \
        g_rotAxisXZ = FixMul(FixMul(kx, kz), g_rotOneMinusCos);                       \
        g_rotAxisYZ = FixMul(FixMul(kz, ky), g_rotOneMinusCos);                       \
                                                                                      \
        r00 = FixMul(g_rotCos, 0x10000 - g_rotAxisXX) + g_rotAxisXX;                  \
        r01 = g_rotAxisXY - FixMul(kz, g_rotSin);                                     \
        r02 = FixMul(ky, g_rotSin) + g_rotAxisXZ;                                     \
        r10 = FixMul(kz, g_rotSin) + g_rotAxisXY;                                     \
        r11 = FixMul(g_rotCos, 0x10000 - g_rotAxisYY) + g_rotAxisYY;                  \
        r12 = g_rotAxisYZ - FixMul(kx, g_rotSin);                                     \
        r20 = g_rotAxisXZ - FixMul(ky, g_rotSin);                                     \
        r21 = FixMul(kx, g_rotSin) + g_rotAxisYZ;                                     \
        r22 = FixMul(g_rotCos, 0x10000 - g_rotAxisZZ) + g_rotAxisZZ;                  \
                                                                                      \
        v.x = FixMul(r20, az) + FixMul(r10, ay) + FixMul(r00, ax);                    \
        v.y = FixMul(r21, az) + FixMul(r11, ay) + FixMul(r01, ax);                    \
        v.z = FixMul(r22, az) + FixMul(r12, ay) + FixMul(r02, ax);                    \
        len = FixVecLength(&v);                                                       \
        if (len == 0) {                                                               \
            ax = 0;                                                                   \
            ay = 0;                                                                   \
            az = 0;                                                                   \
        } else {                                                                      \
            FixVecScaleRecip((FixVector *)&ax, &v, len);                              \
        }                                                                             \
                                                                                      \
        v.x = FixMul(r20, bz) + FixMul(r10, by) + FixMul(r00, bx);                    \
        v.y = FixMul(r21, bz) + FixMul(r11, by) + FixMul(r01, bx);                    \
        v.z = FixMul(r22, bz) + FixMul(r12, by) + FixMul(r02, bx);                    \
        len = FixVecLength(&v);                                                       \
        if (len == 0) {                                                               \
            bx = 0;                                                                   \
            by = 0;                                                                   \
            bz = 0;                                                                   \
        } else {                                                                      \
            FixVecScaleRecip((FixVector *)&bx, &v, len);                              \
        }                                                                             \
    }

// FUNCTION: CMR2 0x004ac820
void SceneNode_Rotate(SceneNode *pNode, FixVector *pTranslation, FixAngles *pAngles)
{
    int rx, ry, rz;
    int ux, uy, uz;
    int fx, fy, fz;
    SceneNode *p;

    ry = pNode->current.right.y;
    rx = pNode->current.right.x;
    rz = pNode->current.right.z;
    uy = pNode->current.up.y;
    ux = pNode->current.up.x;
    fy = pNode->current.forward.y;
    fx = pNode->current.forward.x;
    uz = pNode->current.up.z;
    fz = pNode->current.forward.z;

    if (pAngles->y != 0)
        ROTATE_ABOUT_AXIS(ux, uy, uz, rx, ry, rz, fx, fy, fz, pAngles->y)
    if (pAngles->z != 0)
        ROTATE_ABOUT_AXIS(fx, fy, fz, rx, ry, rz, ux, uy, uz, pAngles->z)
    if (pAngles->x != 0)
        ROTATE_ABOUT_AXIS(rx, ry, rz, ux, uy, uz, fx, fy, fz, pAngles->x)

    pNode->current.right.y = ry;
    pNode->current.right.x = rx;
    pNode->current.right.z = rz;
    pNode->current.up.y = uy;
    pNode->current.up.x = ux;
    pNode->current.up.z = uz;
    pNode->current.forward.y = fy;
    pNode->current.forward.x = fx;
    pNode->current.forward.z = fz;
    pNode->current.position.x += pTranslation->x;
    pNode->current.position.y += pTranslation->y;
    pNode->current.position.z += pTranslation->z;
    for (p = pNode; p != NULL; p = p->pParent)
        p->dirty = 1;
}

// FUNCTION: CMR2 0x004ac140
void SceneNode_Init(SceneNode *pNode)
{
    SceneNode *p;

    pNode->pNext = NULL;
    pNode->pFirstChild = NULL;
    pNode->type = SCENE_NODE_EMPTY;
    pNode->pObject = NULL;
    pNode->field_0x17c = 0xff;
    pNode->pParent = NULL;
    pNode->sector = -1;
    pNode->pNextInSector = NULL;
    pNode->visible = 1;
    for (p = pNode; p != NULL; p = p->pParent)
        p->dirty = 1;
    pNode->useParentWorld = 0;
    pNode->colour[0] = 0x28;
    pNode->colour[1] = 0x28;
    pNode->colour[2] = 0x28;
    pNode->colour[3] = 0xff;
    pNode->translation.x = 0;
    pNode->translation.y = 0;
    pNode->translation.z = 0;
    pNode->angles.x = 0;
    pNode->angles.y = 0;
    pNode->angles.z = 0;
    pNode->allocated = 1;
    FixMatrix_Identity(&pNode->local);
    FixMatrix_Identity(&pNode->current);
    FixMatrix_Identity(&pNode->world);
}

// FUNCTION: CMR2 0x004ac1f0
int SceneNode_Unused(SceneNode *pNode)
{
    return 1;
}

// Appends pNode to pParent's child list (or resets its matrices when it
// is a root).
// FUNCTION: CMR2 0x004ac200
void SceneNode_Attach(SceneNode *pNode, SceneNode *pParent)
{
    SceneNode *p;
    SceneNode *pNext;

    if (pParent != NULL) {
        p = pParent->pFirstChild;
        if (p != NULL) {
            for (pNext = p->pNext; pNext != NULL; pNext = pNext->pNext)
                p = pNext;
            p->pNext = pNode;
            pNode->pParent = pParent;
            return;
        }
        pParent->pFirstChild = pNode;
        pNode->pParent = pParent;
        return;
    }
    FixMatrix_Identity(&pNode->world);
    FixMatrix_Identity(&pNode->current);
}

// FUNCTION: CMR2 0x004ac260
void SceneNode_SetObject(SceneNode *pNode, int type, void *pObject)
{
    pNode->type = type;
    pNode->pObject = pObject;
}

// Recomputes the world matrices of pNode, its siblings and their subtrees.
// FUNCTION: CMR2 0x004ac280
void SceneNode_UpdateTree(SceneNode *pNode, int unused)
{
    SceneNode *pParent;
    SceneNode *p;

    while (pNode != NULL) {
        if (SceneNode_IsVisible(pNode)) {
            pParent = pNode->pParent;
            if (pParent != NULL && pNode->type >= 0 && pNode->type <= 3) {
                if (pNode->dirty == 1 || pParent->dirty == 1) {
                    if ((char)pParent->flags == -3) {
                        pNode->world = pNode->current;
                        g_sceneStatCopied++;
                    } else if (pNode->useParentWorld != 0) {
                        pNode->world = pNode->pParent->world;
                        g_sceneStatCopied++;
                    } else {
                        FixMatrix_Multiply(&pNode->world, &pNode->current, &pParent->world);
                        g_sceneStatMultiplied++;
                    }
                    if (pNode->sector != -1 && (char)pNode->flags != -3)
                        SceneNode_UpdateSector(pNode);
                    for (p = pNode; p != NULL; p = p->pParent)
                        p->dirty = 1;
                } else {
                    g_sceneStatClean++;
                }
            }
            if (pNode->pFirstChild != NULL)
                SceneNode_UpdateTree(pNode->pFirstChild, unused);
        } else {
            g_sceneStatHidden++;
        }
        pNode = pNode->pNext;
    }
}

// FUNCTION: CMR2 0x004ac390
int SceneNode_IsVisible(SceneNode *pNode)
{
    if (pNode->sector >= 0 && pNode->visible != 0 && (pNode->flags & 0x1000) == 0)
        return Sector_IsVisible(pNode->sector);
    return 1;
}

// FUNCTION: CMR2 0x004ac3d0
void SceneNode_SetTransform(SceneNode *pNode, FixVector *pTranslation, FixAngles *pAngles)
{
    if (pNode == NULL)
        return;
    switch (pNode->type) {
    case SCENE_NODE_MESH:
    case SCENE_NODE_EMPTY:
        pNode->current = pNode->local;
        SceneNode_Rotate(pNode, pTranslation, pAngles);
        break;
    case SCENE_NODE_TYPE1:
    case SCENE_NODE_TYPE2:
        pNode->translation = *pTranslation;
        pNode->angles = *pAngles;
        pNode->current = pNode->local;
        SceneNode_Rotate(pNode, pTranslation, pAngles);
        break;
    }
    pNode->useParentWorld = 0;
    do {
        pNode->dirty = 1;
        pNode = pNode->pParent;
    } while (pNode != NULL);
}

// FUNCTION: CMR2 0x004ac480
void SceneNode_SetPosition(SceneNode *pNode, FixVector *pPosition)
{
    if (pNode != NULL) {
        if (pNode->type > 0 && pNode->type <= 2)
            pNode->translation = *pPosition;
        pNode->current.position = *pPosition;
        pNode->useParentWorld = 0;
        do {
            pNode->dirty = 1;
            pNode = pNode->pParent;
        } while (pNode != NULL);
    }
}

// FUNCTION: CMR2 0x004ac4f0
void SceneNode_SetRotation(SceneNode *pNode, FixAngles *pAngles)
{
    FixVector zero;

    zero.x = 0;
    zero.y = 0;
    zero.z = 0;
    if (pNode == NULL)
        return;
    switch (pNode->type) {
    case SCENE_NODE_MESH:
    case SCENE_NODE_EMPTY:
        pNode->current.right.x = pNode->local.right.x;
        pNode->current.right.y = pNode->local.right.y;
        pNode->current.right.z = pNode->local.right.z;
        pNode->current.up.x = pNode->local.up.x;
        pNode->current.up.y = pNode->local.up.y;
        pNode->current.up.z = pNode->local.up.z;
        pNode->current.forward.x = pNode->local.forward.x;
        pNode->current.forward.y = pNode->local.forward.y;
        pNode->current.forward.z = pNode->local.forward.z;
        SceneNode_Rotate(pNode, &zero, pAngles);
        break;
    case SCENE_NODE_TYPE1:
    case SCENE_NODE_TYPE2:
        pNode->current.right.x = pNode->local.right.x;
        pNode->current.right.y = pNode->local.right.y;
        pNode->current.right.z = pNode->local.right.z;
        pNode->current.up.x = pNode->local.up.x;
        pNode->current.up.y = pNode->local.up.y;
        pNode->current.up.z = pNode->local.up.z;
        pNode->current.forward.x = pNode->local.forward.x;
        pNode->current.forward.y = pNode->local.forward.y;
        pNode->current.forward.z = pNode->local.forward.z;
        SceneNode_Rotate(pNode, &zero, pAngles);
        break;
    }
    pNode->useParentWorld = 0;
    do {
        pNode->dirty = 1;
        pNode = pNode->pParent;
    } while (pNode != NULL);
}

// Destroys pNode and its subtree, unlinking it from its parent.
// FUNCTION: CMR2 0x004ac620
int SceneNode_Destroy(SceneNode *pNode)
{
    SceneNode *pChild;
    SceneNode *pNext;
    SceneNode *pParent;
    SceneNode *p;

    pChild = pNode->pFirstChild;
    while (pChild != NULL) {
        pNext = pChild->pNext;
        SceneNode_Destroy(pChild);
        pChild = pNext;
    }
    pParent = pNode->pParent;
    if (pParent != NULL) {
        p = pParent->pFirstChild;
        if (p == pNode) {
            pParent->pFirstChild = pNode->pNext;
        } else {
            for (pNext = p->pNext; pNext != pNode; pNext = pNext->pNext)
                p = pNext;
            p->pNext = pNode->pNext;
        }
    }
    SceneNode_Free(pNode);
    return 1;
}

// FUNCTION: CMR2 0x004ac680
void SceneNode_Free(SceneNode *pNode)
{
    void *pObject;

    pObject = pNode->pObject;
    if (pObject != NULL) {
        switch (pNode->type) {
        case SCENE_NODE_MESH:
            Mesh_Free(pObject);
            break;
        case SCENE_NODE_TYPE1:
            FUN_004b3480(pObject);
            break;
        case SCENE_NODE_TYPE2:
            FUN_004adf60(pObject);
            break;
        }
    }
    g_sceneNodes[pNode->slot] = NULL;
    g_sceneNodeCount--;
    if (pNode->allocated != 0)
        CFileBuffer::FreeGenericFileBuffer(pNode);
}

// FUNCTION: CMR2 0x004ac6f0
SceneNode *SceneNode_Create(SceneNode *pParent)
{
    SceneNode *pNode;
    int i;

    for (i = 0; i < 4096; i++) {
        if (g_sceneNodes[i] == NULL) {
            pNode = (SceneNode *)CFileBuffer::AllocateLockedBuffer(sizeof(SceneNode));
            if (pNode == NULL)
                return NULL;
            SceneNode_Init(pNode);
            SceneNode_Attach(pNode, pParent);
            g_sceneNodes[i] = pNode;
            pNode->slot = (short)g_sceneNodeCount;
            g_sceneNodeCount++;
            return g_sceneNodes[i];
        }
    }
    return NULL;
}

// Moves pNode (and its subtree) under pNewParent.
// FUNCTION: CMR2 0x004ac7a0
int SceneNode_Reparent(SceneNode *pNode, SceneNode *pNewParent)
{
    SceneNode *p;
    SceneNode *pNext;

    if (pNode == NULL)
        return 0;
    if (pNewParent == pNode->pParent)
        return 0;
    if (pNewParent->pParent == pNode)
        return 0;
    p = pNode->pParent->pFirstChild;
    if (p == pNode) {
        pNode->pParent->pFirstChild = pNode->pNext;
    } else {
        for (pNext = p->pNext; pNext != pNode; pNext = pNext->pNext) {
            if (pNext == NULL)
                return 0;
            p = pNext;
        }
        p->pNext = pNode->pNext;
    }
    pNode->pParent = pNewParent;
    if (pNewParent->pFirstChild == NULL) {
        pNewParent->pFirstChild = pNode;
        pNode->pNext = NULL;
    } else {
        pNode->pNext = pNewParent->pFirstChild;
        pNewParent->pFirstChild = pNode;
    }
    do {
        pNode->dirty = 1;
        pNode = pNode->pParent;
    } while (pNode != NULL);
    return 1;
}

// Object registries released by SceneNode_Free: one per SceneNode::type.
// GLOBAL: CMR2 0x0067b124
void *g_meshList[4096];
// GLOBAL: CMR2 0x0067f224
int g_meshListCount;
// GLOBAL: CMR2 0x0067f230
int g_meshTotalCount;

// GLOBAL: CMR2 0x006dffa4
void *g_unk0x006dffa4[60];
// GLOBAL: CMR2 0x006e0b48
int g_unk0x006e0b48;

// GLOBAL: CMR2 0x00683388
void *g_unk0x00683388[256];
// GLOBAL: CMR2 0x006838cc
int g_unk0x006838cc;

// Releases a mesh object: frees the vertex/index buffers hanging off each of
// its sub-objects, then clears its slot in the mesh registry.
// FUNCTION: CMR2 0x004ab7e0
void Mesh_Free(void *pMesh)
{
    void **pSlot;
    void *pSub;
    int i;
    int j;
    int offset;

    i = 0;
    pSlot = g_meshList;
    while (*pSlot == NULL || *pSlot != pMesh) {
        pSlot++;
        i++;
        if ((int)pSlot > 0x67f124)
            return;
    }
    g_meshTotalCount -= *(unsigned char *)((char *)pMesh + 0x110);
    j = 0;
    if (*(int *)((char *)g_meshList[i] + 0x100) > 0) {
        offset = 0x38;
        do {
            pSub = *(void **)((char *)g_meshList[i] + offset);
            CFileBuffer::FreeGenericFileBuffer(*(void **)((char *)pSub + 0x14));
            *(void **)((char *)pSub + 0x14) = NULL;
            CFileBuffer::FreeGenericFileBuffer(*(void **)((char *)g_meshList[i] + offset));
            j++;
            *(void **)((char *)g_meshList[i] + offset) = NULL;
            offset += 4;
        } while (j < *(int *)((char *)g_meshList[i] + 0x100));
    }
    g_meshList[i] = NULL;
    g_meshListCount--;
}

// FUNCTION: CMR2 0x004b3480
void FUN_004b3480(void *pObject)
{
    void **pSlot;
    int i;

    i = 0;
    pSlot = g_unk0x006dffa4;
    while (*pSlot == NULL || *pSlot != pObject) {
        pSlot++;
        i++;
        if ((int)pSlot >= 0x6e0094) {
            g_unk0x006e0b48--;
            return;
        }
    }
    g_unk0x006dffa4[i] = NULL;
    CFileBuffer::FreeGenericFileBuffer(pObject);
    g_unk0x006e0b48--;
}

// FUNCTION: CMR2 0x004adf60
void FUN_004adf60(void *pObject)
{
    void **pSlot;
    int count;

    count = 0;
    pSlot = g_unk0x00683388;
    do {
        if (*pSlot != NULL && *pSlot == pObject) {
            *pSlot = NULL;
            count++;
        }
        pSlot++;
    } while ((int)pSlot < 0x683788);
    if (count > 0) {
        CFileBuffer::FreeGenericFileBuffer(pObject);
        g_unk0x006838cc--;
    }
}

TrackSector *g_trackSectors[256];
int g_trackSectorRowStride;
int g_trackSectorHalfSize;
int g_trackSectorCount;
int g_unk0x0072d55c;
int g_unk0x0072d570;
int g_unk0x0072d578;
int g_unk0x0072d258[64];
int g_unk0x006ef5f0;
int g_unk0x006ef5f4;

// Can the nodes sitting in iSector be drawn?
// FUNCTION: CMR2 0x004b7da0
int Sector_IsVisible(int iSector)
{
    if (g_unk0x0072d578 == 0
        && (g_unk0x0072d258[iSector >> 5] & (1 << (iSector & 0x1f))) == 0
        && g_unk0x0072d570 != 0)
        return 0;
    return 1;
}

// Maps a world position to the index of the sector containing it.
// FUNCTION: CMR2 0x004b85f0
int FUN_004b85f0(FixVector *pPosition)
{
    int row;
    int col;
    int index;
    int offset;

    offset = (pPosition->z - g_trackSectorHalfSize) - g_unk0x006ef5f4;
    if (offset < 0)
        offset = (g_trackSectorHalfSize - pPosition->z) + g_unk0x006ef5f4;
    row = FixMulShift32(offset, g_unk0x0072d55c);

    offset = (pPosition->x - g_unk0x006ef5f0) + g_trackSectorHalfSize;
    if (offset < 0)
        offset = (g_unk0x006ef5f0 - pPosition->x) - g_trackSectorHalfSize;
    col = FixMulShift32(offset, g_unk0x0072d55c);

    index = g_trackSectorRowStride * row + col;
    if ((short)index < 0 || (short)g_trackSectorCount <= (short)index)
        index = 0;
    return index;
}

// Moves pNode into the list of the sector its world position falls in, and
// records up to three neighbouring sectors in the direction it is heading.
// FUNCTION: CMR2 0x004b8690
void SceneNode_UpdateSector(SceneNode *pNode)
{
    TrackSector *pSector;
    SceneNode *p;
    SceneNode *pPrev;
    FixVector position;
    int x;
    int neighbours;
    int lowerEdge;
    int upperEdge;
    short sector;

    neighbours = 0;
    sector = pNode->sector;
    x = pNode->world.position.x;
    lowerEdge = 0;
    upperEdge = 0;
    pSector = g_trackSectors[sector];
    pNode->neighbourSectors[0] = -1;
    pNode->neighbourSectors[1] = -1;
    pNode->neighbourSectors[2] = -1;

    if (x < pSector->x - g_trackSectorHalfSize
        || x > pSector->x + g_trackSectorHalfSize
        || pNode->world.position.z < pSector->z - g_trackSectorHalfSize
        || pNode->world.position.z > pSector->z + g_trackSectorHalfSize) {
        position = pNode->world.position;
        sector = (short)FUN_004b85f0(&position);
    }

    pSector = g_trackSectors[sector];
    if (x - 0x48000 < pSector->x - g_trackSectorHalfSize) {
        lowerEdge = 1;
        pNode->neighbourSectors[0] = sector - 1;
        neighbours = 1;
    } else if (pSector->x + g_trackSectorHalfSize < x + 0x48000) {
        upperEdge = 1;
        pNode->neighbourSectors[0] = sector + 1;
        neighbours = 1;
    }

    if (pSector->z + g_trackSectorHalfSize < pNode->world.position.z + 0x48000) {
        pNode->neighbourSectors[neighbours] = sector - g_trackSectorRowStride;
        if (lowerEdge)
            pNode->neighbourSectors[neighbours + 1] = (sector - g_trackSectorRowStride) - 1;
        else if (upperEdge)
            pNode->neighbourSectors[neighbours + 1] = (sector - g_trackSectorRowStride) + 1;
    } else if (pSector->z - g_trackSectorHalfSize > pNode->world.position.z - 0x48000) {
        pNode->neighbourSectors[neighbours] = g_trackSectorRowStride + sector;
        if (lowerEdge)
            pNode->neighbourSectors[neighbours + 1] = g_trackSectorRowStride - 1 + sector;
        else if (upperEdge)
            pNode->neighbourSectors[neighbours + 1] = g_trackSectorRowStride + 1 + sector;
    }

    if (sector == pNode->sector || sector < 0 || sector >= g_trackSectorCount)
        return;

    pSector = g_trackSectors[pNode->sector];
    p = pSector->pNodeList;
    if (p == NULL)
        return;
    pPrev = NULL;
    while (p != pNode) {
        if (p == NULL)
            break;
        pPrev = p;
        p = p->pNextInSector;
    }
    if (p != NULL) {
        if (pPrev == NULL)
            pSector->pNodeList = p->pNextInSector;
        else
            pPrev->pNextInSector = p->pNextInSector;
        p->pNextInSector = NULL;
        g_trackSectors[pNode->sector]->nodeCount--;
    }

    pSector = g_trackSectors[sector];
    if (pSector->pNodeList == NULL) {
        pSector->pNodeList = pNode;
    } else {
        p = pSector->pNodeList;
        while (p->pNextInSector != NULL)
            p = p->pNextInSector;
        p->pNextInSector = pNode;
    }
    pSector->nodeCount++;
    pNode->sector = sector;
}
