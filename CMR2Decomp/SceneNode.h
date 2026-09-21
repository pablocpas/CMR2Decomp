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
    BYTE field_0x118[0x40];
    int allocated;              // 0x158 free the node memory on destroy
    BYTE field_0x15c[0x14];
    SceneNode *pNextInSector;   // 0x170
    int dirty;                  // 0x174
    int type;                   // 0x178
    BYTE field_0x17c;
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
int SceneNode_Reparent(SceneNode *pNode, SceneNode *pNewParent);
void SceneNode_Rotate(SceneNode *pNode, FixVector *pTranslation, FixAngles *pAngles);

// Track sector; nodes are linked through SceneNode::pNextInSector.
struct TrackSector {
    int x;                  // 0x00 sector centre x (16.16)
    int field_0x4;
    int z;                  // 0x08 sector centre z (16.16)
    BYTE field_0xc[0x10];
    SceneNode *pNodeList;   // 0x1c nodes whose position falls in this sector
    int nodeCount;          // 0x20
};

// GLOBAL: CMR2 0x0071f608
extern TrackSector *g_trackSectors[256];
// GLOBAL: CMR2 0x0071f600
extern int g_trackSectorRowStride;
// GLOBAL: CMR2 0x0072d248
extern int g_trackSectorHalfSize;
// GLOBAL: CMR2 0x0072d568
extern int g_trackSectorCount;
// GLOBAL: CMR2 0x0072d55c
extern int g_unk0x0072d55c;
// GLOBAL: CMR2 0x0072d570
extern int g_unk0x0072d570;
// GLOBAL: CMR2 0x0072d578
extern int g_unk0x0072d578;
// GLOBAL: CMR2 0x0072d258
extern int g_unk0x0072d258[64];
// GLOBAL: CMR2 0x006ef5f0
extern int g_unk0x006ef5f0;
// GLOBAL: CMR2 0x006ef5f4
extern int g_unk0x006ef5f4;

// Track sectors (0x4b7da0..0x4b8690)
int Sector_IsVisible(int iSector);
int FUN_004b85f0(FixVector *pPosition);
void SceneNode_UpdateSector(SceneNode *pNode);

// Type-specific object release, one registry per SceneNode::type
void Mesh_Free(void *pMesh);
void FUN_004b3480(void *pObject);
void FUN_004adf60(void *pObject);

#endif
