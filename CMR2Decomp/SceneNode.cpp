#include <windows.h>
#include <stdio.h>
#include "SceneNode.h"
#include "FileBuffer.h"
#include "Graphics.h"
#include "Game.h"
#include "Sector.h"
#include "Mesh.h"

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
unsigned short g_atanTable[512];

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
            while (p->pNext != pNode)
                p = p->pNext;
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

// Type 1 / type 2 scene objects: fixed pointer tables, released by lookup.
// GLOBAL: CMR2 0x006dffa4
void *g_sceneType1Objects[60];
// Colour and strength of the car shadows.
// GLOBAL: CMR2 0x006e0098
DWORD g_shadowColour;
// GLOBAL: CMR2 0x006e0b38
int g_shadowLevel;
// GLOBAL: CMR2 0x006e0b48
int g_sceneType1Count;
// GLOBAL: CMR2 0x00683388
void *g_sceneType2Objects[256];
// GLOBAL: CMR2 0x006838cc
int g_sceneType2Count;

// FUNCTION: CMR2 0x004b3480
void FUN_004b3480(void *pObject)
{
    int i;

    for (i = 0; i < 60; i++) {
        if (g_sceneType1Objects[i] != NULL && g_sceneType1Objects[i] == pObject) {
            g_sceneType1Objects[i] = NULL;
            CFileBuffer::FreeGenericFileBuffer(pObject);
            g_sceneType1Count--;
            return;
        }
    }
    g_sceneType1Count--;
}

// A Direct3D light plus the device light index it occupies.
struct SceneLight {
    D3DLIGHT7 light;
    int index;
};

// GLOBAL: CMR2 0x006e0b9c
int g_sceneLightCallbackSet;

// Release callback: frees every light still allocated.
// FUNCTION: CMR2 0x004b3450
int Scene_FreeAllLights(void)
{
    int i;

    g_sceneLightCallbackSet = 0;
    for (i = 0; i < 60; i++) {
        if (g_sceneType1Objects[i] != NULL)
            FUN_004b3480(g_sceneType1Objects[i]);
    }
    return 1;
}

// Creates a Direct3D light (0 point, 1 spot, 2 directional, 3 parallel point) in the
// first free slot and optionally hangs it from a scene node placed at pPosition.
// Colour components and position are 16.16 fixed point.
// FUNCTION: CMR2 0x004b2f80
SceneNode *Scene_CreateLight(int type, int r, int g, int b, FixVector *pPosition, FixAngles *pAngles,
                             SceneNode *pParent)
{
    SceneNode *pNode;
    int index;
    float fr, fg, fb;
    FixVector dir;
    SceneLight *pLight;

    pNode = NULL;
    if (g_sceneLightCallbackSet == 0) {
        CGame::RegisterCallback(Scene_FreeAllLights, NULL);
        g_sceneLightCallbackSet = 1;
    }
    for (index = 0; index < 60; index++) {
        if (g_sceneType1Objects[index] == NULL)
            goto found;
    }
    return NULL;

found:

    pLight = (SceneLight *)CFileBuffer::AllocateLockedBuffer(sizeof(SceneLight));
    g_sceneType1Objects[index] = pLight;
    memset(pLight, 0, sizeof(D3DLIGHT7));
    fr = (float)r * CGraphics::m_oneOver65536;
    fg = (float)g * CGraphics::m_oneOver65536;
    fb = (float)b * CGraphics::m_oneOver65536;
    ((SceneLight *)g_sceneType1Objects[index])->index = index;
    ((SceneLight *)g_sceneType1Objects[index])->light.dcvDiffuse.r = fr;
    ((SceneLight *)g_sceneType1Objects[index])->light.dcvDiffuse.g = fg;
    ((SceneLight *)g_sceneType1Objects[index])->light.dcvDiffuse.b = fb;
    ((SceneLight *)g_sceneType1Objects[index])->light.dcvDiffuse.a = 1.0f / 65536.0f;
    ((SceneLight *)g_sceneType1Objects[index])->light.dcvSpecular.r = 0.0f;
    ((SceneLight *)g_sceneType1Objects[index])->light.dcvSpecular.g = 0.0f;
    ((SceneLight *)g_sceneType1Objects[index])->light.dcvSpecular.b = 0.0f;
    ((SceneLight *)g_sceneType1Objects[index])->light.dcvSpecular.a = 0.0f;
    ((SceneLight *)g_sceneType1Objects[index])->light.dcvAmbient.r = 0.0f;
    ((SceneLight *)g_sceneType1Objects[index])->light.dcvAmbient.g = 0.0f;
    ((SceneLight *)g_sceneType1Objects[index])->light.dcvAmbient.b = 0.0f;
    ((SceneLight *)g_sceneType1Objects[index])->light.dcvAmbient.a = 0.0f;
    ((SceneLight *)g_sceneType1Objects[index])->light.dvPosition.x = (float)pPosition->x * CGraphics::m_oneOver65536;
    ((SceneLight *)g_sceneType1Objects[index])->light.dvPosition.y = (float)pPosition->y * CGraphics::m_oneOver65536;
    ((SceneLight *)g_sceneType1Objects[index])->light.dvPosition.z = (float)pPosition->z * CGraphics::m_oneOver65536;
    ((SceneLight *)g_sceneType1Objects[index])->light.dvAttenuation0 = 1.0f;
    ((SceneLight *)g_sceneType1Objects[index])->light.dvAttenuation1 = 0.0f;
    ((SceneLight *)g_sceneType1Objects[index])->light.dvAttenuation2 = 0.0f;
    ((SceneLight *)g_sceneType1Objects[index])->light.dvDirection.x = 0.0f;
    ((SceneLight *)g_sceneType1Objects[index])->light.dvDirection.y = 0.0f;
    ((SceneLight *)g_sceneType1Objects[index])->light.dvDirection.z = 1.0f;
    ((SceneLight *)g_sceneType1Objects[index])->light.dvFalloff = 1.0f;
    ((SceneLight *)g_sceneType1Objects[index])->light.dvTheta = 0.5f;
    ((SceneLight *)g_sceneType1Objects[index])->light.dvPhi = 1.0f;

    switch (type) {
    case 0:
        ((SceneLight *)g_sceneType1Objects[index])->light.dltType = D3DLIGHT_POINT;
        ((SceneLight *)g_sceneType1Objects[index])->light.dcvAmbient.r = fr;
        ((SceneLight *)g_sceneType1Objects[index])->light.dcvAmbient.g = fg;
        ((SceneLight *)g_sceneType1Objects[index])->light.dcvAmbient.b = fb;
        ((SceneLight *)g_sceneType1Objects[index])->light.dcvAmbient.a = 1.0f / 65536.0f;
        ((SceneLight *)g_sceneType1Objects[index])->light.dvRange = 60.0f;
        ((SceneLight *)g_sceneType1Objects[index])->light.dvAttenuation0 = 0.0f;
        ((SceneLight *)g_sceneType1Objects[index])->light.dvAttenuation1 = 0.0f;
        ((SceneLight *)g_sceneType1Objects[index])->light.dvAttenuation2 = 0.1f;
        break;
    case 1:
        ((SceneLight *)g_sceneType1Objects[index])->light.dltType = D3DLIGHT_SPOT;
        ((SceneLight *)g_sceneType1Objects[index])->light.dcvAmbient.r = fr;
        ((SceneLight *)g_sceneType1Objects[index])->light.dcvAmbient.g = fg;
        ((SceneLight *)g_sceneType1Objects[index])->light.dcvAmbient.b = fb;
        ((SceneLight *)g_sceneType1Objects[index])->light.dcvAmbient.a = 1.0f / 65536.0f;
        ((SceneLight *)g_sceneType1Objects[index])->light.dvRange = 100.0f;
        ((SceneLight *)g_sceneType1Objects[index])->light.dvFalloff = 1.0f;
        ((SceneLight *)g_sceneType1Objects[index])->light.dvTheta = 0.2f;
        ((SceneLight *)g_sceneType1Objects[index])->light.dvPhi = 2.0f;
        ((SceneLight *)g_sceneType1Objects[index])->light.dvAttenuation0 = 0.0f;
        ((SceneLight *)g_sceneType1Objects[index])->light.dvAttenuation1 = 0.0f;
        ((SceneLight *)g_sceneType1Objects[index])->light.dvAttenuation2 = 0.0f;
        break;
    case 2:
        ((SceneLight *)g_sceneType1Objects[index])->light.dltType = D3DLIGHT_DIRECTIONAL;
        dir.x = -pPosition->x;
        dir.y = -pPosition->y;
        dir.z = -pPosition->z;
        FIX_NORMALIZE_INTO(dir, dir);
        ((SceneLight *)g_sceneType1Objects[index])->light.dvDirection.x = (float)dir.x * CGraphics::m_oneOver65536;
        ((SceneLight *)g_sceneType1Objects[index])->light.dvDirection.y = (float)dir.y * CGraphics::m_oneOver65536;
        ((SceneLight *)g_sceneType1Objects[index])->light.dvDirection.z = (float)dir.z * CGraphics::m_oneOver65536;
        break;
    case 3:
        ((SceneLight *)g_sceneType1Objects[index])->light.dltType = D3DLIGHT_PARALLELPOINT;
        break;
    }

    CGraphics::m_pTextureManager->pD3D->SetLight(index, &((SceneLight *)g_sceneType1Objects[index])->light);
    CGraphics::m_pTextureManager->pD3D->LightEnable(index, TRUE);
    if (pParent != NULL) {
        pNode = SceneNode_Create(pParent);
        SceneNode_SetObject(pNode, 1, g_sceneType1Objects[index]);
        SceneNode_SetTransform(pNode, pPosition, pAngles);
    }
    g_sceneType1Count++;
    return pNode;
}

// FUNCTION: CMR2 0x004adf60
void FUN_004adf60(void *pObject)
{
    void **pSlot;
    int count;

    count = 0;
    pSlot = g_sceneType2Objects;
    do {
        if (*pSlot != NULL && *pSlot == pObject) {
            *pSlot = NULL;
            count++;
        }
        pSlot++;
    } while ((int)pSlot < (int)&g_sceneType2Objects[256]);
    if (count > 0) {
        CFileBuffer::FreeGenericFileBuffer(pObject);
        g_sceneType2Count--;
    }
}

// Creates the root node of the scene graph and marks it as the root.
// FUNCTION: CMR2 0x004ac760
SceneNode *SceneNode_CreateRoot(void)
{
    CGraphics::m_pTextureManager->pRootNode = SceneNode_Create(NULL);
    CGraphics::m_pTextureManager->pRootNode->flags =
        (CGraphics::m_pTextureManager->pRootNode->flags & 0xfffffffd) | 0xfd;
    return CGraphics::m_pTextureManager->pRootNode;
}

// GLOBAL: CMR2 0x006838c8
int g_sceneType2CallbackRegistered;

// Exit callback: releases every type 2 object still registered.
// FUNCTION: CMR2 0x004ae070
int SceneType2_ReleaseAll(void)
{
    void **pSlot;

    pSlot = g_sceneType2Objects;
    do {
        if (*pSlot != NULL)
            FUN_004adf60(*pSlot);
        pSlot++;
    } while ((int)pSlot < (int)&g_sceneType2Objects[256]);
    g_sceneType2CallbackRegistered = 0;
    return 1;
}

// Allocates a type 2 object (0x104 bytes) and attaches it to pNode, creating
// the node under pParent (with the given transform) when none is passed.
// FUNCTION: CMR2 0x004adfa0
SceneNode *SceneType2_Create(FixVector *pTranslation, FixAngles *pAngles, SceneNode *pNode, SceneNode *pParent)
{
    SceneNode *p;
    void **pSlot;
    void *pObject;
    int i;
    unsigned int flags;

    if (g_sceneType2CallbackRegistered == 0) {
        CGame::RegisterCallback(SceneType2_ReleaseAll, NULL);
        g_sceneType2CallbackRegistered = 1;
    }
    if (pNode == NULL)
        p = SceneNode_Create(pParent);
    else
        p = pNode;
    i = 0;
    *(BYTE *)&p->flags = 0xff;
    for (i = 0; i < 256; i++) {
        if (g_sceneType2Objects[i] == NULL) {
            pObject = CFileBuffer::AllocateLockedBuffer(0x104);
            g_sceneType2Objects[i] = pObject;
            SceneNode_SetObject(p, SCENE_NODE_TYPE2, pObject);
            if (pNode == NULL) {
                p->translation.x = pTranslation->x;
                p->translation.y = pTranslation->y;
                p->translation.z = pTranslation->z;
                p->angles.x = pAngles->x;
                p->angles.y = pAngles->y;
                p->angles.z = pAngles->z;
                SceneNode_SetTransform(p, pTranslation, pAngles);
            }
            g_sceneType2Count++;
            return p;
        }
    }
    return NULL;
}

// FUNCTION: CMR2 0x004b4a80
void Scene_GetShadowColour(DWORD *pColour, int *pLevel)
{
    *pColour = g_shadowColour;
    *pLevel = g_shadowLevel;
}

// Scene lighting: ambient plus a light colour scaled by 50 levels.
// GLOBAL: CMR2 0x006e0118
FixVector g_sceneAmbient;
// GLOBAL: CMR2 0x006e0348
FixVector g_sceneLightColour;
// GLOBAL: CMR2 0x006e0244
int g_sceneLightDirty;
// GLOBAL: CMR2 0x006e0b34
BYTE *g_sceneLightTable;            // 50 RGBA colours
// GLOBAL: CMR2 0x006dfdc8
DWORD *g_sceneLightTableD3D;        // the same as D3D ARGB
// GLOBAL: CMR2 0x006dfd9c
DWORD *g_sceneShadowTable;          // shadow colour at each level
// GLOBAL: CMR2 0x006deacc
DWORD *g_sceneShadowTableD3D;

// Rebuilds the light and shadow colour tables.
// TODO: CMR2 0x004b3940 (implemented, match 82%)
void Scene_BuildLightTables(void)
{
    FixVector c;
    BYTE *p;
    DWORD *pShadow;
    int level;
    int step;
    int offset;
    int v;
    BYTE alpha;

    if (g_sceneLightTable != NULL && g_sceneLightTableD3D != NULL) {
        step = FixDiv(0x10000, 0x310000);
        level = 0;
        p = g_sceneLightTable;
        offset = 0;
        pShadow = g_sceneShadowTable;
        do {
            FixVecScale(&c, &g_sceneLightColour, level);
            c.y += g_sceneAmbient.y;
            c.x += g_sceneAmbient.x;
            c.z += g_sceneAmbient.z;
            v = c.x >> 16;
            if (v < 0)
                v = 0;
            else if (v > 0xff)
                v = 0xff;
            p[0] = (BYTE)v;
            v = c.y >> 16;
            if (v < 0)
                v = 0;
            else if (v > 0xff)
                v = 0xff;
            p[1] = (BYTE)v;
            v = c.z >> 16;
            if (v < 0)
                v = 0;
            else if (v > 0xff)
                v = 0xff;
            p[2] = (BYTE)v;
            p[3] = 0xff;
            *(DWORD *)((BYTE *)g_sceneLightTableD3D + offset) = ((p[0] | 0xffffff00) << 8 | p[1]) << 8 | (v & 0xff);
            *pShadow = g_shadowColour;
            alpha = (BYTE)FixMulShift32(g_shadowLevel, level);
            ((BYTE *)pShadow)[3] = alpha;
            *(DWORD *)((BYTE *)g_sceneShadowTableD3D + offset) =
                ((((DWORD)alpha << 8 | ((BYTE *)pShadow)[0]) << 8) | ((BYTE *)pShadow)[1]) << 8 | ((BYTE *)pShadow)[2];
            offset += 4;
            p += 4;
            pShadow++;
            level += step;
        } while (offset < 200);
    }
}

// Light colour at a level (0..1.0).
// TODO: CMR2 0x004b3ae0 (implemented, match 87%)
void Scene_GetLightColour(DWORD *pColour, int level)
{
    int i;

    if (g_sceneLightDirty != 0) {
        Scene_BuildLightTables();
        g_sceneLightDirty = 0;
    }
    i = level * 0x31 >> 16;
    if (i < 0) {
        *pColour = *(DWORD *)g_sceneLightTable;
        return;
    }
    if (i > 0x31)
        i = 0x31;
    *pColour = ((DWORD *)g_sceneLightTable)[i];
}

// Light colour of the scene (absolute, 16.16 per channel).
// GLOBAL: CMR2 0x006e01a0
FixVector g_sceneLight;
// GLOBAL: CMR2 0x006e01ec
BYTE g_sceneAmbientColour[4];       // last ambient colour set
// GLOBAL: CMR2 0x006e020c
DWORD g_sceneAmbientD3D;
// GLOBAL: CMR2 0x006dfdc4
BYTE g_sceneLightColourBytes[4];

// Derives the shadow colour and strength from the contrast between the
// ambient and the light: the shadow is tinted by the ambient minus its
// weakest channel, or with boost set, a darker copy of the ambient and a
// stronger shadow.
// FUNCTION: CMR2 0x004b4910
void Scene_UpdateShadowColour(int boost)
{
    FixVector c;
    int r;
    int g;
    int b;

    g_shadowLevel = FIX_ABS(g_sceneAmbient.x - g_sceneLight.x);
    g_shadowLevel += FIX_ABS(g_sceneAmbient.y - g_sceneLight.y);
    g_shadowLevel += FIX_ABS(g_sceneAmbient.z - g_sceneLight.z);
    g_shadowLevel = FixMul(g_shadowLevel, 0x55);
    if (g_shadowLevel > 0x10000)
        g_shadowLevel = 0x10000;
    g_shadowLevel = FixMul(g_shadowLevel, 0xff0000);
    c = g_sceneAmbient;
    if (boost != 0) {
        g_shadowLevel = FixMul(g_shadowLevel, 0x20000);
        if (g_shadowLevel > 0xff0000)
            g_shadowLevel = 0xff0000;
        FixVecScale(&c, &c, 0x2aac);
        r = c.x;
        g = c.y;
        b = c.z;
    } else {
        r = c.x;
        g = c.y;
        b = c.z;
        if (g >= r && r <= b) {
            g -= r;
            b -= r;
            r = 0;
        } else if (g <= r && g <= b) {
            r -= g;
            b -= g;
            g = 0;
        } else {
            r -= b;
            g -= b;
            b = 0;
        }
    }
    ((BYTE *)&g_shadowColour)[0] = (BYTE)(r >> 16);
    ((BYTE *)&g_shadowColour)[1] = (BYTE)(g >> 16);
    ((BYTE *)&g_shadowColour)[2] = (BYTE)(b >> 16);
}

// Sets the ambient colour of the scene (RGBA bytes).
// TODO: CMR2 0x004b3740 (implemented, match 63%)
void Scene_SetAmbient(BYTE *pColour, int boost)
{
    g_sceneAmbientD3D = ((((DWORD)pColour[3] << 8 | pColour[0]) << 8) | pColour[1]) << 8 | pColour[2];
    if (pColour[0] != g_sceneAmbientColour[0] || pColour[1] != g_sceneAmbientColour[1] ||
        pColour[2] != g_sceneAmbientColour[2])
        g_sceneLightDirty = 1;
    g_sceneAmbientColour[0] = pColour[0];
    g_sceneAmbientColour[1] = pColour[1];
    g_sceneAmbientColour[2] = pColour[2];
    g_sceneAmbientColour[3] = pColour[3];
    g_sceneAmbient.x = pColour[0] << 16;
    g_sceneLightColour.x = g_sceneLight.x - (pColour[0] << 16);
    g_sceneAmbient.y = pColour[1] << 16;
    g_sceneLightColour.y = g_sceneLight.y - (pColour[1] << 16);
    g_sceneAmbient.z = pColour[2] << 16;
    g_sceneLightColour.z = g_sceneLight.z - (pColour[2] << 16);
    Scene_UpdateShadowColour(boost);
    CGraphics::m_pTextureManager->pD3D->SetRenderState(D3DRENDERSTATE_AMBIENT, g_sceneAmbientD3D);
}

// Sets the light colour of the scene.
// FUNCTION: CMR2 0x004b3f90
void Scene_SetLight(FixVector *pLight, int boost)
{
    if (pLight->x != g_sceneLight.x || pLight->y != g_sceneLight.y || pLight->z != g_sceneLight.z)
        g_sceneLightDirty = 1;
    g_sceneLight.x = pLight->x;
    g_sceneLight.y = pLight->y;
    g_sceneLight.z = pLight->z;
    g_sceneLightColourBytes[0] = (BYTE)((unsigned int)g_sceneLight.x >> 16);
    g_sceneLightColourBytes[1] = (BYTE)((unsigned int)g_sceneLight.y >> 16);
    g_sceneLightColourBytes[2] = (BYTE)((unsigned int)g_sceneLight.z >> 16);
    g_sceneLightColour.x = g_sceneLight.x - g_sceneAmbient.x;
    g_sceneLightColour.y = g_sceneLight.y - g_sceneAmbient.y;
    g_sceneLightColour.z = g_sceneLight.z - g_sceneAmbient.z;
    Scene_UpdateShadowColour(boost);
}

// Light colour at a level (0..1.0), as D3D ARGB.
// TODO: CMR2 0x004b3b40 (implemented, match 83%)
void Scene_GetLightColourD3D(DWORD *pColour, int level)
{
    int i;

    if (g_sceneLightDirty != 0) {
        Scene_BuildLightTables();
        g_sceneLightDirty = 0;
    }
    i = level * 0x31 >> 16;
    if (i < 0) {
        *pColour = g_sceneLightTableD3D[0];
        return;
    }
    if (i > 0x31)
        i = 0x31;
    *pColour = g_sceneLightTableD3D[i];
}

// Shadow colour at a level (0..1.0), as D3D ARGB.
// TODO: CMR2 0x004b3ba0 (implemented, match 83%)
void Scene_GetShadowColourD3D(DWORD *pColour, int level)
{
    int i;

    if (g_sceneLightDirty != 0) {
        Scene_BuildLightTables();
        g_sceneLightDirty = 0;
    }
    i = level * 0x31 >> 16;
    if (i < 0) {
        *pColour = g_sceneShadowTableD3D[0];
        return;
    }
    if (i > 0x31)
        i = 0x31;
    *pColour = g_sceneShadowTableD3D[i];
}

// GLOBAL: CMR2 0x006e0124
int g_sceneLightState[30];
// GLOBAL: CMR2 0x006e01c4
int g_sceneLightState2[10];
// GLOBAL: CMR2 0x006e019c
void *g_sceneSectorLights;          // per sector
// GLOBAL: CMR2 0x006dfd90
void *g_sceneSectorLights2;         // per sector
// GLOBAL: CMR2 0x006dfd98
Mesh **g_sceneShadowMeshes;         // shadow mesh of each sector
// GLOBAL: CMR2 0x006dfdcc
unsigned short *g_sceneLightData;   // lighting data of the stage
// GLOBAL: CMR2 0x006dfdbc
BYTE *g_sceneLightZones;            // 0x14 bytes each
// GLOBAL: CMR2 0x006dfdfc
int *g_sceneSectorFlags;            // per sector
// GLOBAL: CMR2 0x006e0204
short *g_sceneSectorZone;           // zone of each sector (-1 none)
// GLOBAL: CMR2 0x006deab8
BYTE g_sceneLightFlag;
// GLOBAL: CMR2 0x006e0b99
BYTE g_sceneLightFlag2;

void FUN_004a3240(int unused);
int Scene_AttenuateSectorLight(int sector, int light);
extern int g_unk0x005210c0;

// Lights every vertex of a mesh from its per-vertex light levels.
#define MESH_LIGHT_VERTICES(m)                                                      \
    {                                                                               \
        int *pLevel = (m)->pLightLevels;                                            \
        BYTE *pColour = (BYTE *)(m)->pVertexData + 0x18;                            \
        for (i = 0; i < (m)->field_0x10; i++) {                                     \
            Scene_GetLightColourD3D((DWORD *)pColour, *pLevel);                     \
            pLevel++;                                                               \
            pColour += 0x30;                                                        \
        }                                                                           \
    }

// Relights a sector when the scene ambient or light colour changed since its
// last update (ground mesh, static objects, nodes), then its shadow mesh,
// attenuated by D3D light 1 through the sector light zone when enabled.
// TODO: CMR2 0x004b3c00 (implemented, match 39%)
void Scene_RelightSector(int sector)
{
    Sector *pSector;
    Mesh *pMesh;
    Mesh *pShadow;
    StageObject *pObject;
    SceneNode *pNode;
    LightZone *pZone;
    DWORD colour;
    int *pLevel;
    BYTE *pVertex;
    int *pZoneVertex;
    int intensity;
    int force;
    int i;
    int j;
    int k;

    force = 0;
    if (g_sceneSectorLights == NULL)
        return;
    if (g_sceneAmbientColour[0] != ((BYTE *)g_sceneSectorLights)[sector * 4] ||
        g_sceneAmbientColour[1] != ((BYTE *)g_sceneSectorLights)[sector * 4 + 1] ||
        g_sceneAmbientColour[2] != ((BYTE *)g_sceneSectorLights)[sector * 4 + 2] ||
        g_sceneLightColourBytes[0] != ((BYTE *)g_sceneSectorLights2)[sector * 4] ||
        g_sceneLightColourBytes[1] != ((BYTE *)g_sceneSectorLights2)[sector * 4 + 1] ||
        g_sceneLightColourBytes[2] != ((BYTE *)g_sceneSectorLights2)[sector * 4 + 2]) {
        ((DWORD *)g_sceneSectorLights)[sector] = *(DWORD *)g_sceneAmbientColour;
        ((DWORD *)g_sceneSectorLights2)[sector] = *(DWORD *)g_sceneLightColourBytes;
        pSector = g_sectors[sector];
        if (pSector != NULL) {
            pMesh = (Mesh *)pSector->pMesh;
            if (pMesh != NULL) {
                MESH_LIGHT_VERTICES(pMesh);
                Mesh_RefreshVertices(pMesh);
            }
            for (pObject = pSector->pObjects; pObject != NULL; pObject = pObject->pNext) {
                pMesh = pObject->pMesh;
                if ((pMesh->flags & 0x80) == 0) {
                    Scene_GetLightColourD3D(&colour, pObject->lightLevel);
                    Mesh_SetColourAndRefresh(pMesh, colour);
                } else {
                    MESH_LIGHT_VERTICES(pMesh);
                    Mesh_RefreshVertices(pMesh);
                }
            }
            for (pNode = pSector->pFirstNode; pNode != NULL; pNode = pNode->pNextInSector)
                FUN_004a3240((int)pNode);
            if (g_sceneShadowMeshes == NULL)
                return;
            if (g_sceneShadowMeshes[sector] != NULL)
                force = 1;
        }
    }
    if (((g_sceneShadowMeshes != NULL && g_sceneShadowMeshes[sector] != NULL && g_unk0x005210c0 != 0 &&
          Scene_AttenuateSectorLight(sector, 1) != 0) ||
         force) &&
        (pShadow = g_sceneShadowMeshes[sector]) != NULL) {
        pLevel = pShadow->pLightLevels;
        if (g_unk0x005210c0 != 0) {
            pZone = &((LightZone *)g_sceneLightZones)[g_sceneSectorZone[sector]];
            pVertex = (BYTE *)pShadow->pVertexData;
            if (pZone->vertexCount != 0) {
                pZoneVertex = (int *)(pZone->pVertices + 0x28);
                for (k = 0; k < pZone->vertexCount; k++) {
                    intensity = pZoneVertex[-2];
                    if (intensity == 0x10000) {
                        for (j = 0; j < *pZoneVertex; j++) {
                            Scene_GetShadowColourD3D((DWORD *)(pVertex + 0x18), *pLevel);
                            pVertex += 0x30;
                            pLevel++;
                        }
                    } else {
                        for (j = 0; j < *pZoneVertex; j++) {
                            Scene_GetShadowColourD3D((DWORD *)(pVertex + 0x18), FixMul(intensity, *pLevel));
                            pLevel++;
                            pVertex += 0x30;
                        }
                    }
                    pZoneVertex += 12;
                }
                Mesh_RefreshVertices(pShadow);
                return;
            }
        } else {
            pVertex = (BYTE *)pShadow->pVertexData + 0x18;
            for (i = 0; i < pShadow->field_0x10; i++) {
                Scene_GetShadowColourD3D((DWORD *)pVertex, *pLevel);
                pLevel++;
                pVertex += 0x30;
            }
        }
        Mesh_RefreshVertices(pShadow);
    }
}

// One mesh of a shadow caster with its per-vertex working buffers (0x58 bytes).
struct ShadowPart {
    BYTE field_0x0[0x30];
    Mesh *pMesh;                // 0x30 mesh (or its shadow cylinder)
    SceneNode *pNode;           // 0x34 node that owns the mesh
    void *pVertexWork;          // 0x38 12 bytes per vertex
    void *pVertexWork2;         // 0x3c 12 bytes per vertex
    void *pVertexFlags;         // 0x40 4 bytes per vertex
    void *pVertexWork3;         // 0x44 12 bytes per vertex
    BYTE *pVertices;            // 0x48 0x30 bytes per vertex, colour 0xff000000
    void *pTriangleWork;        // 0x4c 12 bytes per triangle
    short field_0x50;
    short field_0x52;
    int field_0x54;
};

// A scene node hierarchy that casts shadows (0x14 bytes).
struct ShadowCaster {
    SceneNode *pNode;           // 0x0
    ShadowPart *pParts;         // 0x4
    BYTE partCount;             // 0x8
    int field_0xc;              // 0xc
    int field_0x10;
};

// Marks one mesh part of a registered shadow caster for rebuilding.
// FUNCTION: CMR2 0x004b4100
void Scene_MarkShadowPartDirty(SceneNode *pNode, Mesh *pMesh)
{
    ShadowCaster *pCaster = NULL;
    ShadowPart *pPart = NULL;
    int i;
    int count;
    if (g_sceneShadowMeshes != NULL) {
        count = g_sceneLightFlag & 0xff;
        i = 0;
        if (count > 0) {
            do {
                ShadowCaster *p = (ShadowCaster *)g_sceneLightState[i];
                if (p->pNode == pNode) {
                    pCaster = p;
                    i = count;
                }
                ++i;
            } while (i < count);
            if (pCaster != NULL) {
                i = 0;
                count = pCaster->partCount;
                if (count > 0) {
                    do {
                        if (pCaster->pParts[i].pMesh == pMesh) {
                            pPart = &pCaster->pParts[i];
                            i = count;
                        }
                        ++i;
                    } while (i < count);
                    if (pPart != NULL) {
                        pPart->field_0x54 = 1;
                        pCaster->field_0xc = 1;
                    }
                }
            }
        }
    }
}

// Mesh nodes whose flags type is 6, 14 or 15 cast no shadow.
#define CASTS_SHADOW(n)                                                              \
    ((n)->type == SCENE_NODE_MESH && (n)->pObject != NULL && ((n)->flags & 0xff) != 6 && \
     ((n)->flags & 0xff) != 0xf && ((n)->flags & 0xff) != 0xe)

static __forceinline void ShadowPart_Init(ShadowPart *pPart)
{
    int i;

    pPart->pVertexWork = CFileBuffer::AllocateLockedBuffer(pPart->pMesh->field_0x10 * 0xc);
    pPart->pVertexFlags = CFileBuffer::AllocateLockedBuffer(pPart->pMesh->field_0x10 * 4);
    pPart->pVertexWork2 = CFileBuffer::AllocateLockedBuffer(pPart->pMesh->field_0x10 * 0xc);
    pPart->pTriangleWork = CFileBuffer::AllocateLockedBuffer(pPart->pMesh->triangleCount * 0xc);
    pPart->pVertices = (BYTE *)CFileBuffer::AllocateLockedBuffer(pPart->pMesh->field_0x10 * 0x30);
    for (i = 0; i < pPart->pMesh->field_0x10; i++)
        *(DWORD *)(pPart->pVertices + i * 0x30 + 0x1c) = 0xff000000;
    pPart->pVertexWork3 = CFileBuffer::AllocateLockedBuffer(pPart->pMesh->field_0x10 * 0xc);
    pPart->field_0x54 = 1;
}

// Registers pNode and every mesh below it as a shadow caster (at most 29). Small
// objects (flags type <= 4) use a cylinder around the mesh unless exactMeshes is set.
// TODO: CMR2 0x004b45d0 (implemented, match 55%)
void Scene_AddShadowCaster(SceneNode *pNode, int exactMeshes)
{
    ShadowCaster *pCaster;
    ShadowPart *pPart;
    SceneNode *pChild;
    SceneNode *p;

    if (pNode == NULL || g_sceneLightFlag >= 29)
        return;
    pCaster = (ShadowCaster *)CFileBuffer::AllocateLockedBuffer(sizeof(ShadowCaster));
    g_sceneLightState[g_sceneLightFlag++] = (int)pCaster;
    pCaster->pNode = pNode;
    pCaster->pParts = NULL;
    pCaster->partCount = 0;
    pCaster->field_0xc = 1;
    if (CASTS_SHADOW(pNode))
        pCaster->partCount = 1;
    for (pChild = pNode->pFirstChild; pChild != NULL; pChild = pChild->pNext) {
        for (p = pChild; p != NULL; p = p->pFirstChild) {
            if (CASTS_SHADOW(p))
                pCaster->partCount++;
        }
    }
    pCaster->pParts = (ShadowPart *)CFileBuffer::AllocateLockedBuffer(pCaster->partCount * sizeof(ShadowPart));
    pPart = pCaster->pParts;
    if (CASTS_SHADOW(pNode)) {
        pPart->pMesh = (Mesh *)pNode->pObject;
        if (exactMeshes == 0 && (pNode->flags & 0xff) <= 4)
            pPart->pMesh = Mesh_GetShadowCylinder((Mesh *)pNode->pObject);
        ShadowPart_Init(pPart);
        pPart->field_0x50 = 0;
        pPart->pNode = pNode;
        pPart->field_0x52 = 0;
        pPart++;
    }
    for (pChild = pNode->pFirstChild; pChild != NULL; pChild = pChild->pNext) {
        for (p = pChild; p != NULL; p = p->pFirstChild) {
            if (CASTS_SHADOW(p)) {
                pPart->pMesh = (Mesh *)p->pObject;
                if (exactMeshes == 0 && (pNode->flags & 0xff) <= 4)
                    pPart->pMesh = Mesh_GetShadowCylinder((Mesh *)p->pObject);
                ShadowPart_Init(pPart);
                pPart->pNode = p;
                pPart->field_0x50 = 0;
                pPart->field_0x52 = 0;
                pPart++;
            }
        }
    }
}

extern int *g_triangleVertexHeights;
void Mesh_BuildParts(Mesh *pMesh);
void Mesh_UploadVertices(Mesh *pMesh);

// GLOBAL: CMR2 0x006dfe00
FixVector g_sceneLightDir;
// GLOBAL: CMR2 0x006e00a0
FixVector g_sceneLightBasis[3];
// GLOBAL: CMR2 0x006e0b28
FixVector g_sceneLightRight;        // basis[0] flattened
// GLOBAL: CMR2 0x006dfda0
FixVector g_sceneLightForward;      // basis[2] flattened
// GLOBAL: CMR2 0x006e01b0
float g_sceneLightDirF[3];
// GLOBAL: CMR2 0x006e0220
float g_sceneLightBasisF[9];
// GLOBAL: CMR2 0x005210ec
char g_strShadowMeshName[] = "SHAD%d";

// Reads the lighting data of a stage: the light direction and basis, then
// the shadow zones (one per sector with a shadow mesh). Offsets in the data
// are turned into pointers, zones are matched to their sector and their
// items to the scene objects there, and a shadow mesh is built for each.
// TODO: CMR2 0x004b4aa0 (implemented, match 34%)
void Scene_LoadLighting(int *pData)
{
    BYTE *pZone;
    BYTE *p;
    int *pHeader;
    int *pItem;
    int *pObj;
    Mesh *pMesh;
    FixVector *pBasis;
    unsigned short *pCursor;
    unsigned int s;
    unsigned int found;
    unsigned int next;
    int zone;
    int item;
    int off;
    int itemOff;
    int zoneOff;
    int d;
    int i;
    BOOL done;

    if (pData == NULL)
        return;
    g_sceneLightDir.x = pData[0];
    g_sceneLightDir.y = pData[1];
    g_sceneLightDir.z = pData[2];
    g_sceneLightData = (unsigned short *)(pData + 3);
    pBasis = g_sceneLightBasis;
    do {
        pCursor = g_sceneLightData;
        g_sceneLightData = pCursor + 6;
        pBasis->x = *(int *)pCursor;
        pBasis->y = *(int *)(pCursor + 2);
        pBasis->z = *(int *)(pCursor + 4);
        pBasis++;
    } while (pBasis < &g_sceneLightBasis[3]);
    g_sceneLightRight.x = g_sceneLightBasis[0].x;
    g_sceneLightDirF[0] = (float)g_sceneLightDir.x * (float)CGraphics::m_oneOver65536;
    g_sceneLightRight.z = g_sceneLightBasis[0].z;
    g_sceneLightRight.y = 0;
    g_sceneLightForward.x = g_sceneLightBasis[2].x;
    g_sceneLightForward.y = 0;
    g_sceneLightForward.z = g_sceneLightBasis[2].z;
    g_sceneLightDirF[1] = (float)g_sceneLightDir.y * (float)CGraphics::m_oneOver65536;
    g_sceneLightDirF[2] = (float)g_sceneLightDir.z * (float)CGraphics::m_oneOver65536;
    for (i = 0; i < 9; i++)
        g_sceneLightBasisF[i] = (float)(&g_sceneLightBasis[0].x)[i] * (float)CGraphics::m_oneOver65536;

    // Zones, followed by their items, headers, vertices, triangles and
    // vertex heights.
    g_sceneLightZones = (BYTE *)(pCursor + 7);
    p = g_sceneLightZones + *g_sceneLightData * 0x14;
    for (zone = 0, off = 0; zone < *g_sceneLightData; zone++, off += 0x14) {
        *(BYTE **)(g_sceneLightZones + off + 0xc) = p;
        p += *(unsigned short *)(g_sceneLightZones + off + 2) * 0x30;
    }
    for (zone = 0, off = 0; zone < *g_sceneLightData; zone++, off += 0x14) {
        *(BYTE **)(g_sceneLightZones + off + 0x10) = p;
        p += 0x14;
    }
    for (zone = 0, off = 0; zone < *g_sceneLightData; zone++, off += 0x14) {
        pHeader = *(int **)(g_sceneLightZones + off + 0x10);
        pHeader[2] = (int)p;
        p += pHeader[1] * 0x30;
    }
    for (zone = 0, off = 0; zone < *g_sceneLightData; zone++, off += 0x14) {
        pHeader = *(int **)(g_sceneLightZones + off + 0x10);
        pHeader[3] = (int)p;
        p += pHeader[0] * 0x4c;
    }
    for (zone = 0, off = 0; zone < *g_sceneLightData; zone++, off += 0x14) {
        pHeader = *(int **)(g_sceneLightZones + off + 0x10);
        pHeader[4] = (int)p;
        p += pHeader[1] * 4;
    }

    // Sector of each zone.
    for (zone = 0, off = 0; zone < *g_sceneLightData; zone++, off += 0x14) {
        found = 0xffffffff;
        s = 0;
        if (g_sectorCount == 0) {
            *(short *)(g_sceneLightZones + off) = -1;
            continue;
        }
        do {
            next = s;
            if (g_sectors[s]->x == *(int *)(g_sceneLightZones + off + 4) &&
                g_sectors[s]->z == *(int *)(g_sceneLightZones + off + 8)) {
                next = g_sectorCount;
                found = s;
            }
            s = next + 1;
        } while (s < (unsigned int)g_sectorCount);
        if ((int)found < 0)
            *(short *)(g_sceneLightZones + off) = -1;
        else
            *(short *)(g_sceneLightZones + off) = (short)found;
    }

    // Scene object under each item.
    for (zone = 0, zoneOff = 0; zone < *g_sceneLightData; zone++, zoneOff += 0x14) {
        for (item = 0, itemOff = 0; item < *(unsigned short *)(g_sceneLightZones + zoneOff + 2);
             item++, itemOff += 0x30) {
            pItem = (int *)(*(BYTE **)(g_sceneLightZones + zoneOff + 0xc) + itemOff);
            for (s = 0; s < (unsigned int)g_sectorCount; s++) {
                done = FALSE;
                pObj = *(int **)((BYTE *)g_sectors[s] + 0x14);
                if (pObj == NULL)
                    continue;
                do {
                    if (done)
                        break;
                    d = pObj[0] - pItem[0];
                    if (FIX_ABS(d) < 0x28f) {
                        d = pObj[2] - pItem[1];
                        if (FIX_ABS(d) < 0x28f) {
                            pItem[0xb] = (int)pObj;
                            done = TRUE;
                        }
                    }
                    pObj = (int *)pObj[0x26];
                } while (pObj != NULL);
                if (done)
                    s = g_sectorCount;
            }
            pItem[8] = 0x10000;
        }
    }

    // Each shadow triangle takes the texture of its item's object.
    for (zone = 0, off = 0; zone < *g_sceneLightData; zone++, off += 0x14) {
        pHeader = *(int **)(g_sceneLightZones + off + 0x10);
        for (i = 0, itemOff = 0; i < pHeader[0]; i++, itemOff += 0x4c) {
            int *pTri = (int *)(pHeader[3] + 4 + itemOff);
            int *pOwner = (int *)(*(BYTE **)(g_sceneLightZones + off + 0xc) + *pTri * 0x30);
            *pTri = *(int *)(*(BYTE **)(*(BYTE **)(pOwner[0xb] + 0xc) + 0x24) + 4);
        }
    }

    // A shadow mesh per zone.
    g_sceneShadowMeshes = (Mesh **)CFileBuffer::AllocateLockedBuffer(g_sectorCount * 4);
    g_sceneSectorZone = (short *)CFileBuffer::AllocateLockedBuffer(g_sectorCount * 2);
    s = 0;
    if (g_sectorCount != 0) {
        do {
            s++;
            g_sceneShadowMeshes[s - 1] = NULL;
            g_sceneSectorZone[s - 1] = -1;
        } while (s < (unsigned int)g_sectorCount);
    }
    for (zone = 0, off = 0; zone < *g_sceneLightData; zone++, off += 0x14) {
        if (*(short *)(g_sceneLightZones + off) == -1)
            continue;
        g_sceneShadowMeshes[*(unsigned short *)(g_sceneLightZones + off)] = Mesh_Alloc();
        g_sceneSectorZone[*(unsigned short *)(g_sceneLightZones + off)] = (short)zone;
        pMesh = g_sceneShadowMeshes[*(unsigned short *)(g_sceneLightZones + off)];
        ((BYTE *)pMesh)[0x108] = 0xff;
        ((BYTE *)pMesh)[0x109] = 0xff;
        ((BYTE *)pMesh)[0x10a] = 0xff;
        for (i = 0x10c; i < 0x120; i++)
            ((BYTE *)pMesh)[i] = 0;
        pHeader = *(int **)(g_sceneLightZones + off + 0x10);
        for (i = 0xc; i != 0; i--)
            sprintf((char *)pMesh, g_strShadowMeshName, zone);
        pMesh->field_0x10 = pHeader[1];
        pMesh->pVertexData = (DWORD *)pHeader[2];
        pMesh->triangleCount = pHeader[0];
        pMesh->pTriangles = (MeshTriangle *)pHeader[3];
        *(int *)((BYTE *)pMesh + 0x20) = 0;
        ((BYTE *)pMesh)[0x104] = 0;
        *(int *)((BYTE *)pMesh + 0x34) = pHeader[4];
        *(unsigned int *)((BYTE *)pMesh + 0x30) = (itemOff & 0xffffe02d) | 0x402d;
        *(int *)((BYTE *)pMesh + 0x2c) = 0;
        Mesh_BuildParts(pMesh);
        Mesh_UploadVertices(pMesh);
    }
}

// Frees the lighting of a stage (callback registered by Scene_InitLighting).
// FUNCTION: CMR2 0x004b5510
int Scene_FreeLighting(void)
{
    unsigned int i;

    if (g_sceneSectorLights != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_sceneSectorLights);
        g_sceneSectorLights = NULL;
    }
    if (g_sceneSectorLights2 != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_sceneSectorLights2);
        g_sceneSectorLights2 = NULL;
    }
    if (g_sceneLightTable != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_sceneLightTable);
        g_sceneLightTable = NULL;
    }
    if (g_sceneLightTableD3D != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_sceneLightTableD3D);
        g_sceneLightTableD3D = NULL;
    }
    if (g_sceneShadowTable != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_sceneShadowTable);
        g_sceneShadowTable = NULL;
    }
    if (g_sceneShadowTableD3D != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_sceneShadowTableD3D);
        g_sceneShadowTableD3D = NULL;
    }
    if (g_sceneShadowMeshes != NULL) {
        for (i = 0; i < (unsigned int)g_sectorCount; i++) {
            if (g_sceneShadowMeshes[i] != NULL) {
                Mesh_Free(g_sceneShadowMeshes[i]);
                CFileBuffer::FreeGenericFileBuffer(g_sceneShadowMeshes[i]);
                g_sceneShadowMeshes[i] = NULL;
            }
        }
        CFileBuffer::FreeGenericFileBuffer(g_sceneShadowMeshes);
        g_sceneShadowMeshes = NULL;
    }
    if (g_sceneSectorFlags != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_sceneSectorFlags);
        g_sceneSectorFlags = NULL;
    }
    if (g_sceneSectorZone != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_sceneSectorZone);
        g_sceneSectorZone = NULL;
    }
    g_triangleVertexHeights = NULL;
    return 1;
}

// Sets up the lighting of a stage: the light and shadow tables, per-sector
// state, and the stage's lighting data (shadow meshes).
// FUNCTION: CMR2 0x004b5620
void Scene_InitLighting(int *pData, int *pHeights)
{
    int i;
    int *p;
    unsigned int k;

    p = g_sceneLightState;
    for (i = 30; i != 0; i--)
        *p++ = 0;
    p = g_sceneLightState2;
    for (i = 10; i != 0; i--)
        *p++ = 0;
    g_triangleVertexHeights = NULL;
    g_sceneSectorLights = NULL;
    g_sceneSectorLights2 = NULL;
    g_sceneLightTable = NULL;
    g_sceneLightTableD3D = NULL;
    g_sceneShadowTable = NULL;
    g_sceneShadowTableD3D = NULL;
    g_sceneShadowMeshes = NULL;
    g_sceneLightData = NULL;
    g_sceneLightZones = NULL;
    g_sceneSectorFlags = NULL;
    g_sceneLightFlag = 0;
    g_sceneLightFlag2 = 0;
    g_sceneSectorLights = CFileBuffer::AllocateLockedBuffer(g_sectorCount * 4);
    g_sceneSectorLights2 = CFileBuffer::AllocateLockedBuffer(g_sectorCount * 4);
    g_sceneLightTable = (BYTE *)CFileBuffer::AllocateLockedBuffer(200);
    g_sceneLightTableD3D = (DWORD *)CFileBuffer::AllocateLockedBuffer(200);
    g_sceneShadowTable = (DWORD *)CFileBuffer::AllocateLockedBuffer(200);
    g_sceneShadowTableD3D = (DWORD *)CFileBuffer::AllocateLockedBuffer(200);
    g_sceneSectorFlags = (int *)CFileBuffer::AllocateLockedBuffer(g_sectorCount * 4);
    k = 0;
    if (g_sectorCount != 0) {
        do {
            k++;
            g_sceneSectorFlags[k - 1] = 0;
        } while (k < (unsigned int)g_sectorCount);
    }
    Scene_LoadLighting(pData);
    if (pHeights != NULL)
        g_triangleVertexHeights = pHeights;
    CGame::RegisterCallback((void *)Scene_FreeLighting, NULL);
}

// Shadow geometry streamed into pVertexBuffer2 between Scene_BeginShadowBatch
// and Scene_EndShadowBatch, split into {first vertex, vertex count, texture,
// view mask} batches.
// GLOBAL: CMR2 0x006dead0
int g_shadowBatches[300][4];
// GLOBAL: CMR2 0x006e01f0
int *g_shadowBatch;
// GLOBAL: CMR2 0x006e0b90
void *g_shadowVertexData;
// GLOBAL: CMR2 0x006e0b94
int g_shadowVertexCount;
// GLOBAL: CMR2 0x006e0b98
BYTE g_shadowBatchCount;
// GLOBAL: CMR2 0x005210d4
int g_shadowLastTexture = -1;
// GLOBAL: CMR2 0x005210d8
int g_shadowLastFlags = -1;
// Direction the shadows are cast along (opposite of the light).
// GLOBAL: CMR2 0x006dfdd8
FixVector g_sceneShadowDir;

// FUNCTION: CMR2 0x004b5340
void Scene_SetShadowDirection(FixVector *pLightDir)
{
    FixVecScale(&g_sceneShadowDir, pLightDir, -0x10000);
}

// Locks the shadow vertex buffer and starts the first batch.
// FUNCTION: CMR2 0x004b5e60
void Scene_BeginShadowBatch(void)
{
    if (g_sceneSectorZone != NULL) {
        CGraphics::m_pTextureManager->pVertexBuffer2->Lock(DDLOCK_WAIT | DDLOCK_WRITEONLY, &g_shadowVertexData, NULL);
        g_shadowVertexCount = 0;
        g_shadowBatchCount = 0;
        g_shadowLastTexture = -1;
        g_shadowLastFlags = -1;
        g_shadowBatch = g_shadowBatches[0];
    }
}

// Closes the current batch and unlocks the shadow vertex buffer.
// FUNCTION: CMR2 0x004b5eb0
void Scene_EndShadowBatch(void)
{
    if (g_sceneSectorZone != NULL) {
        g_shadowBatch[1] = g_shadowVertexCount - g_shadowBatch[0];
        CGraphics::m_pTextureManager->pVertexBuffer2->Unlock();
    }
}

// FUNCTION: CMR2 0x004b3840
void Scene_GetAmbientColour(DWORD *pColour)
{
    *pColour = *(DWORD *)g_sceneAmbientColour;
}

// FUNCTION: CMR2 0x004b3850
void Scene_GetLightColourBytes(DWORD *pColour)
{
    *pColour = *(DWORD *)g_sceneLightColourBytes;
}

// Sets the constant attenuation of the D3D light held by a type-1 node.
// FUNCTION: CMR2 0x004b3570
void Scene_SetLightAttenuation(SceneNode *pNode, int attenuation)
{
    SceneLight *pLight;

    if (pNode != NULL && (pLight = (SceneLight *)pNode->pObject) != NULL) {
        pLight->light.dvAttenuation0 = (float)attenuation * CGraphics::m_oneOver65536;
        CGraphics::m_pTextureManager->pD3D->SetLight(pLight->index, &pLight->light);
    }
}

// Depth-first search (children before siblings) for the first node whose
// flags type byte is `type`.
// FUNCTION: CMR2 0x004ad890
SceneNode *SceneNode_FindByType(SceneNode *pNode, unsigned int type)
{
    SceneNode *pFound;

    while (pNode != NULL) {
        if ((pNode->flags & 0xff) == type)
            return pNode;
        if (pNode->pFirstChild != NULL && (pFound = SceneNode_FindByType(pNode->pFirstChild, type)) != NULL)
            return pFound;
        pNode = pNode->pNext;
    }
    return NULL;
}

// Sets the diffuse colour (16.16 per channel, clamped to 0..255) of the D3D
// light held by a type-1 node.
// FUNCTION: CMR2 0x004b34d0
void Scene_SetLightColour(SceneNode *pNode, int r, int g, int b)
{
    SceneLight *pLight;

    pLight = (SceneLight *)pNode->pObject;
    if (r < 0)
        r = 0;
    if (g < 0)
        g = 0;
    if (b < 0)
        b = 0;
    if (r > 0xff0000)
        r = 0xff0000;
    if (g > 0xff0000)
        g = 0xff0000;
    if (b > 0xff0000)
        b = 0xff0000;
    pLight->light.dcvDiffuse.r = (float)r * CGraphics::m_oneOver65536;
    pLight->light.dcvDiffuse.g = (float)g * CGraphics::m_oneOver65536;
    pLight->light.dcvDiffuse.b = (float)b * CGraphics::m_oneOver65536;
    CGraphics::m_pTextureManager->pD3D->SetLight(pLight->index, &pLight->light);
}

// Light level (0..1) of one corner of a mesh triangle for a light direction:
// four times the dot product of its vertex normal with pDir, clamped.
// TODO: CMR2 0x004b4040 (implemented, match 85%)
int Mesh_GetCornerLight(Mesh *pMesh, MeshTriangle *pTri, FixVector *pDir, int corner)
{
    float *pVertex;
    FixVector normal;
    int level;

    pVertex = (float *)((BYTE *)pMesh->pVertexData + pTri->vertexIndex[corner] * 0x30);
    normal.x = (int)(__int64)(pVertex[3] * CGraphics::m_65536);
    normal.y = (int)(__int64)(pVertex[4] * CGraphics::m_65536);
    normal.z = (int)(__int64)(pVertex[5] * CGraphics::m_65536);
    level = FixMul(FixVecDot(&normal, pDir), 0x40000);
    if (level < 0)
        return 0;
    if (level > 0x10000)
        level = 0x10000;
    return level;
}

// Light level and colour of the ground at a position: those of the nearest
// (in x/z) vertex of its sector's ground mesh. Returns r, g, b bytes.
// TODO: CMR2 0x004b3860 (implemented, match 39%)
DWORD Scene_GetGroundLight(FixVector *pPos, int *pLevel)
{
    Mesh *pMesh;
    float *pVertex;
    float *pNearest;
    int *pVertexLevel;
    float dx;
    float dz;
    float d2;
    float best;
    BYTE rgb[4];
    int x;
    int z;
    int i;

    *pLevel = 0x10000;
    x = pPos->x;
    pNearest = NULL;
    *(DWORD *)rgb = 0xffffff;
    best = 32000.0f;
    z = pPos->z;
    pMesh = (Mesh *)g_sectors[(short)Sector_FromPosition(pPos)]->pMesh;
    if (pMesh != NULL) {
        pVertex = (float *)pMesh->pVertexData;
        pVertexLevel = pMesh->pLightLevels;
        for (i = 0; i < pMesh->field_0x10; i++) {
            dx = pVertex[0] - (float)x * (float)CGraphics::m_oneOver65536;
            dz = pVertex[2] - (float)z * (float)CGraphics::m_oneOver65536;
            d2 = dx * dx + dz * dz;
            if (d2 < best) {
                *pLevel = *pVertexLevel;
                pNearest = pVertex;
                best = d2;
            }
            pVertex += 12;
            pVertexLevel++;
        }
        if (pNearest != NULL) {
            rgb[2] = (BYTE)((DWORD *)pNearest)[6];
            rgb[0] = (BYTE)(((DWORD *)pNearest)[6] >> 16);
            rgb[1] = (BYTE)(((DWORD *)pNearest)[6] >> 8);
        }
    }
    return *(DWORD *)rgb;
}

// Frees every shadow caster (with its per-part buffers) and every cached
// shadow cylinder.
// TODO: CMR2 0x004b5380 (implemented, match 48%)
void Scene_FreeShadowCasters(void)
{
    int *p;
    ShadowCaster *pCaster;
    Mesh *pCyl;
    int i;

    p = g_sceneLightState;
    do {
        pCaster = (ShadowCaster *)*p;
        if (pCaster != NULL) {
            if (pCaster->pParts != NULL) {
                for (i = 0; i < pCaster->partCount; i++) {
                    if (((ShadowCaster *)*p)->pParts[i].pVertexWork != NULL) {
                        CFileBuffer::FreeGenericFileBuffer(((ShadowCaster *)*p)->pParts[i].pVertexWork);
                        ((ShadowCaster *)*p)->pParts[i].pVertexWork = NULL;
                    }
                    if (((ShadowCaster *)*p)->pParts[i].pVertexFlags != NULL) {
                        CFileBuffer::FreeGenericFileBuffer(((ShadowCaster *)*p)->pParts[i].pVertexFlags);
                        ((ShadowCaster *)*p)->pParts[i].pVertexFlags = NULL;
                    }
                    if (((ShadowCaster *)*p)->pParts[i].pVertexWork2 != NULL) {
                        CFileBuffer::FreeGenericFileBuffer(((ShadowCaster *)*p)->pParts[i].pVertexWork2);
                        ((ShadowCaster *)*p)->pParts[i].pVertexWork2 = NULL;
                    }
                    if (((ShadowCaster *)*p)->pParts[i].pVertices != NULL) {
                        CFileBuffer::FreeGenericFileBuffer(((ShadowCaster *)*p)->pParts[i].pVertices);
                        ((ShadowCaster *)*p)->pParts[i].pVertices = NULL;
                    }
                    if (((ShadowCaster *)*p)->pParts[i].pVertexWork3 != NULL) {
                        CFileBuffer::FreeGenericFileBuffer(((ShadowCaster *)*p)->pParts[i].pVertexWork3);
                        ((ShadowCaster *)*p)->pParts[i].pVertexWork3 = NULL;
                    }
                    if (((ShadowCaster *)*p)->pParts[i].pTriangleWork != NULL) {
                        CFileBuffer::FreeGenericFileBuffer(((ShadowCaster *)*p)->pParts[i].pTriangleWork);
                        ((ShadowCaster *)*p)->pParts[i].pTriangleWork = NULL;
                    }
                }
                CFileBuffer::FreeGenericFileBuffer(((ShadowCaster *)*p)->pParts);
                ((ShadowCaster *)*p)->pParts = NULL;
            }
            CFileBuffer::FreeGenericFileBuffer((void *)*p);
            *p = 0;
        }
        *p = 0;
        p++;
    } while ((int)p < (int)&g_sceneLightState[30]);
    p = g_sceneLightState2;
    do {
        pCyl = (Mesh *)*p;
        if (pCyl != NULL) {
            if (pCyl->pTriangles != NULL) {
                CFileBuffer::FreeGenericFileBuffer(pCyl->pTriangles);
                ((Mesh *)*p)->pTriangles = NULL;
            }
            if (((Mesh *)*p)->pVertexData != NULL) {
                CFileBuffer::FreeGenericFileBuffer(((Mesh *)*p)->pVertexData);
                ((Mesh *)*p)->pVertexData = NULL;
            }
            CFileBuffer::FreeGenericFileBuffer((void *)*p);
            *p = 0;
        }
        *p = 0;
        p++;
    } while ((int)p < (int)&g_sceneLightState2[10]);
    g_sceneLightFlag = 0;
    g_sceneLightFlag2 = 0;
}

struct Unk0x004a3e20;
void FUN_004a3e20(Unk0x004a3e20 *pObject, int value);
void FUN_004a3dd0(void);

// Draws the shadow batches visible in view `view` (bit of each batch mask),
// with the batch texture forced to blend mode 10.
// TODO: CMR2 0x004b6240 (implemented, match 72%)
void Scene_DrawShadowBatches(BYTE view)
{
    Texture *pTexture;
    Texture *pLast;
    int blend;
    int i;

    if (g_shadowVertexCount == 0)
        return;
    CGraphics::SetZWriteEnable(0);
    CGraphics::SetCullMode(1);
    pLast = NULL;
    g_shadowBatch = g_shadowBatches[0];
    for (i = 0; i < g_shadowBatchCount; i++) {
        if (g_shadowBatch[1] != 0 && (*(BYTE *)&g_shadowBatch[3] & (1 << view)) != 0) {
            pTexture = CGraphics::m_pTextureManager->textureBuffer[g_shadowBatch[2]];
            if (pTexture != pLast) {
                blend = pTexture->blendMode;
                FUN_004a3e20((Unk0x004a3e20 *)pTexture, 10);
                CGraphics::FUN_004a4850(0, (int)pTexture);
                FUN_004a3e20((Unk0x004a3e20 *)pTexture, blend);
                pLast = pTexture;
            }
            CGraphics::m_pTextureManager->pD3D->DrawPrimitiveVB(D3DPT_TRIANGLELIST,
                                                                CGraphics::m_pTextureManager->pVertexBuffer2,
                                                                g_shadowBatch[0], g_shadowBatch[1], 0);
        }
        g_shadowBatch += 4;
    }
    CGraphics::FUN_004a4850(0, 0);
    CGraphics::SetZWriteEnable(1);
    CGraphics::SetCullMode(CGame::FUN_0049dcb0());
    FUN_004a3dd0();
    CGraphics::FUN_004a3de0();
}
