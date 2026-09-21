#ifndef _CAR_H
#define _CAR_H

#include "FixedPoint.h"
#include "SceneNode.h"

// Car instance (0xc24 bytes, one per slot in g_carBuffer). Only the fields
// used by the decompiled code are named; offsets are in the comments.
struct Car {
    BYTE field_0x0[0x1d8];
    int field_0x1d8;                  // 0x1d8
    BYTE field_0x1dc[0x28];
    FixVector halfExtents;            // 0x204
    FixVector wheelPos[4];            // 0x210  wheel positions in body space
    BYTE field_0x240[0x30];
    FixVector corners[8];             // 0x270  world-space box corners
    FixVector position;               // 0x2d0
    BYTE field_0x2dc[0x84];
    FixVector right;                  // 0x360  body axes (rows of the body matrix)
    FixVector up;                     // 0x36c
    FixVector forward;                // 0x378
    BYTE field_0x384[0xc];
    FixVector targetUp;               // 0x390  up/forward the body relaxes towards
    FixVector targetForward;          // 0x39c
    BYTE field_0x3a8[0x60];
    FixVector velocity;               // 0x408
    BYTE field_0x414[0x10];
    int field_0x424;                  // 0x424
    BYTE field_0x428[0x64];
    int field_0x48c;                  // 0x48c
    BYTE field_0x490[0x4];
    int field_0x494;                  // 0x494
    int field_0x498;                  // 0x498
    BYTE field_0x49c[0x4];
    int field_0x4a0;                  // 0x4a0
    BYTE field_0x4a4[0x1a4];
    FixVector force0x648;             // 0x648
    FixVector force0x654;             // 0x654
    BYTE field_0x660[0xbc];
    SceneNode *pNode0x71c;            // 0x71c
    SceneNode *pNode0x720;            // 0x720
    BYTE field_0x724[0x14];
    SceneNode *pWheelNodes[4];        // 0x738
    SceneNode *pViewNodeNear;         // 0x748  child node placed towards the view
    SceneNode *pViewNodeFar;          // 0x74c  child node placed away from the view
    FixMatrix *pWorld;                // 0x750
    FixMatrix *pBodyMatrix;           // 0x754
    BYTE field_0x758[0xc];
    int scale0x764;                   // 0x764
    int scale0x768;                   // 0x768
    int scale0x76c;                   // 0x76c
    BYTE field_0x770[0x8];
    int steer;                        // 0x778
    BYTE field_0x77c[0xa0];
    int field_0x81c;                  // 0x81c
    BYTE field_0x820[0xfc];
    int field_0x91c;                  // 0x91c
    int field_0x920;                  // 0x920
    int field_0x924;                  // 0x924
    BYTE field_0x928[0x60];
    int wheel0x988[4];                // 0x988
    BYTE field_0x998[0x10];
    int wheel0x9a8[4];                // 0x9a8
    BYTE field_0x9b8[0xc];
    int field_0x9c4;                  // 0x9c4
    BYTE field_0x9c8[0x10];
    int wheel0x9d8[4];                // 0x9d8
    BYTE field_0x9e8[0x128];
    unsigned short heading;           // 0xb10  12-bit angle
    BYTE field_0xb12[0x16];
    BYTE field_0xb28;                 // 0xb28
    BYTE field_0xb29[0x3b];
    int field_0xb64;                  // 0xb64
    BYTE field_0xb68[0xc];
    int field_0xb74;                  // 0xb74
    BYTE field_0xb78[0xac];
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
void Car_UpdateBodyAxes(void);
void Car_UpdateBodyAxesNoDamping(void);
void Car_StoreBodyMatrix(void);
void Car_UpdateBodyMatrix(void);
void Car_RelaxBodyAxes(int bFast);

#endif
