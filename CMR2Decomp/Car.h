#ifndef _CAR_H
#define _CAR_H

#include "FixedPoint.h"
#include "SceneNode.h"

// Car instance (0xc24 bytes, one per slot in g_carBuffer). Only the fields
// used by the decompiled code are named; offsets are in the comments.
struct Car {
    BYTE field_0x0[0x204];
    FixVector halfExtents;          // 0x204
    BYTE field_0x210[0x60];
    FixVector corners[8];           // 0x270  world-space box corners
    FixVector position;             // 0x2d0
    BYTE field_0x2dc[0x84];
    FixVector vec0x360;             // 0x360
    BYTE field_0x36c[0xc];
    FixVector vec0x378;             // 0x378
    BYTE field_0x384[0x398];
    SceneNode *pNode0x71c;          // 0x71c
    SceneNode *pNode0x720;          // 0x720
    BYTE field_0x724[0x14];
    SceneNode *pWheelNodes[4];      // 0x738
    SceneNode *pViewNodeNear;       // 0x748  child node placed towards the view
    SceneNode *pViewNodeFar;        // 0x74c  child node placed away from the view
    FixMatrix *pWorld;              // 0x750
    BYTE field_0x754[0x10];
    int scale0x764;                 // 0x764
    int scale0x768;                 // 0x768
    int scale0x76c;                 // 0x76c
    BYTE field_0x770[0x3f4];
    int field_0xb64;                // 0xb64
    BYTE field_0xb68[0xbc];
};

// Stored transforms of a car, written by the physics and applied to the
// scene nodes each frame.
struct CarTransforms {
    FixMatrix body;                 // 0x0   applied to Car::pNode0x71c
    FixMatrix body2;                // 0x40  applied to Car::pNode0x720
    BYTE field_0x80[0x7c];
};

// GLOBAL: CMR2 0x0053b560
extern CarTransforms g_carTransforms[16];
// GLOBAL: CMR2 0x0053bda0
extern FixMatrix g_carWheelTransforms[16][4];
// GLOBAL: CMR2 0x0053a3a0
extern short g_carOrderCount;
// GLOBAL: CMR2 0x0053b500
extern short g_carOrder[48];
// GLOBAL: CMR2 0x0053a324
extern int g_carViewScale[15][2];
// GLOBAL: CMR2 0x00538e2c
extern SceneNode *g_viewNodes[8];

// GLOBAL: CMR2 0x0053aba8
extern Car *g_cars[64];
// GLOBAL: CMR2 0x0053bd68
extern int g_carCount;
// GLOBAL: CMR2 0x0053c9a4
extern Car *g_carBuffer;
// GLOBAL: CMR2 0x0053cc18
extern Car *g_pCurrentCar;

void Car_AllocateTable(int count);
Car *Car_Get(int index);
void Car_UpdateCorners(Car *pCar);
void Car_ApplyCornerOffsets(void);
void Car_ApplyViewTransforms(int viewIndex);
void Car_UpdateViewNodes(int viewIndex);

#endif
