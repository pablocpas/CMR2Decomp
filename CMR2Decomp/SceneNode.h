#ifndef _SCENE_NODE_H
#define _SCENE_NODE_H

#include "FixedPoint.h"

// Node types (SceneNode::type); the object pointer is freed by type.
#define SCENE_NODE_MESH     0
#define SCENE_NODE_TYPE1    1
#define SCENE_NODE_TYPE2    2
#define SCENE_NODE_EMPTY    3

// Hierarchical transform node used by the vehicle/scene code (0x18c bytes).
struct SceneNode {
    SceneNode *pNext;           // 0x0   next sibling
    SceneNode *pFirstChild;     // 0x4
    SceneNode *pParent;         // 0x8
    void *pObject;              // 0xc   mesh/light/... depending on type
    FixVector translation;      // 0x10  last translation passed to SetTransform
    FixAngles angles;           // 0x1c  last rotation passed to SetTransform
    short sector;               // 0x24  track sector the node sits in (-1 = none)
    short slot;                 // 0x26  index in g_sceneNodes
    short neighbourSectors[3];  // 0x28
    short field_0x2e;
    unsigned int flags;         // 0x30  low byte 0xfd marks the root node
    BYTE field_0x34[0x24];
    FixMatrix local;            // 0x58  transform at creation / reset
    FixMatrix current;          // 0x98  local transform (right/up/forward/position)
    FixMatrix world;            // 0xd8  current * parent world
    float worldF[16];           // 0x118 float copy of world for Direct3D
    int allocated;              // 0x158 free the node memory on destroy
    BYTE field_0x15c[0x14];
    SceneNode *pNextInSector;   // 0x170
    int dirty;                  // 0x174
    int type;                   // 0x178
    BYTE viewMask;
    BYTE field_0x17d[3];
    int visible;                // 0x180
    int useParentWorld;         // 0x184 world = parent world (no multiply)
    BYTE colour[4];             // 0x188
};

// GLOBAL: CMR2 0x0067f36c
extern SceneNode *g_sceneNodes[4096];
// GLOBAL: CMR2 0x00683370
extern int g_sceneNodeCount;

void SceneNode_Init(SceneNode *pNode);
int SceneNode_Unused(SceneNode *pNode);
void SceneNode_Attach(SceneNode *pNode, SceneNode *pParent);
void SceneNode_SetObject(SceneNode *pNode, int type, void *pObject);
void SceneNode_UpdateTree(SceneNode *pNode, int unused);
int SceneNode_IsVisible(SceneNode *pNode);
void SceneNode_SetTransform(SceneNode *pNode, FixVector *pTranslation, FixAngles *pAngles);
void SceneNode_SetPosition(SceneNode *pNode, FixVector *pPosition);
void SceneNode_SetRotation(SceneNode *pNode, FixAngles *pAngles);
int SceneNode_Destroy(SceneNode *pNode);
void SceneNode_Free(SceneNode *pNode);
SceneNode *SceneNode_Create(SceneNode *pParent);
SceneNode *SceneNode_CreateRoot(void);
int SceneNode_Reparent(SceneNode *pNode, SceneNode *pNewParent);
void SceneNode_Rotate(SceneNode *pNode, FixVector *pTranslation, FixAngles *pAngles);

#include "Sector.h"
#include "Mesh.h"

void Scene_MarkShadowPartDirty(SceneNode *pNode, Mesh *pMesh);

// Light zone of a sector (0x14 bytes): vertices whose intensity (at 0x20) the
// scene lights attenuate; each covers the next `count` (at 0x28) shadow mesh vertices.
struct LightZone {
    short field_0x0;
    unsigned short vertexCount;     // 0x2
    BYTE field_0x4[8];
    BYTE *pVertices;                // 0xc  0x30 bytes each: x at 8, z at 0xc, intensity at 0x20
    int field_0x10;
};

// Type-specific object release, one registry per SceneNode::type
void Scene_FreeType2Object(void *pObject);
SceneNode *SceneType2_Create(FixVector *pTranslation, FixAngles *pAngles, SceneNode *pNode, SceneNode *pParent);
int SceneType2_ReleaseAll(void);
void Scene_FreeType1Object(void *pObject);

#endif
