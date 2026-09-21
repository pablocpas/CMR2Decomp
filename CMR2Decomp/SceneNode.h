#ifndef _SCENE_NODE_H
#define _SCENE_NODE_H

#include "FixedPoint.h"

// Hierarchical transform node used by the vehicle/scene code. Only the
// fields touched so far are named.
struct SceneNode {
    BYTE field_0x0[8];
    SceneNode *pParent;     // 0x8
    BYTE field_0xc[0x8c];
    FixVector right;        // 0x98
    int field_0xa4;
    FixVector up;           // 0xa8
    int field_0xb4;
    FixVector forward;      // 0xb8
    int field_0xc4;
    FixVector position;     // 0xc8
    BYTE field_0xd4[0xa0];
    int dirty;              // 0x174
};

void SceneNode_Rotate(SceneNode *pNode, FixVector *pTranslation, unsigned short *pAngles);

#endif
