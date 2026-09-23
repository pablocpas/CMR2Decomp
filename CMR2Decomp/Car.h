#ifndef _CAR_H
#define _CAR_H

#include "FixedPoint.h"
#include "SceneNode.h"

// Car instance (0xc24 bytes, one per slot in g_carBuffer). Only the fields
// used by the decompiled code are named; offsets are in the comments.
struct Car {
    BYTE field_0x0[0x1d0];
    char flag0x1d0[4];                // 0x1d0
    BYTE field_0x1d4[0x4];
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
    FixVector wheelAxisRear;          // 0x3a8  lateral axis used by the rear wheels
    FixVector wheelDirRear;           // 0x3b4  rolling direction of the rear wheels
    FixVector wheelEmitter[4];        // 0x3c0  wheel dust/smoke emitter in body space
    FixVector wheelDirFront;          // 0x3f0  rolling direction of the front wheels
    FixVector inertia;                // 0x3fc  used to turn the summed torque into angular acceleration
    FixVector velocity;               // 0x408
    FixVector velocityNext;           // 0x414  velocityNext - velocity is the acceleration of the last step
    FixVector angularVelocity;        // 0x420  body space
    FixVector cornerVelocity[8];      // 0x42c  world-space velocity of each box corner
    FixVector groundNormal;           // 0x48c  normal of the box face the car rests on (Car_UpdateGroundNormal)
    FixVector normal0x498;            // 0x498
    FixVector cornerAxis[8];          // 0x4a4  per-corner reference axis
    FixVector cornerNormal[8];        // 0x504  per-corner contact normal
    BYTE field_0x564[0x60];
    FixVector field_0x5c4;            // 0x5c4
    FixVector field_0x5d0;            // 0x5d0
    BYTE field_0x5dc[0x6c];
    FixVector cornerForce[8];         // 0x648  force accumulated at each corner
    FixVector baseForce;              // 0x6a8  constant force applied every step
    BYTE field_0x6b4[0x3c];
    FixVector lean;                   // 0x6f0  body lean (x/z tilt) fed into the body matrix
    BYTE field_0x6fc[0x20];
    SceneNode *pNode0x71c;            // 0x71c
    SceneNode *pNode0x720;            // 0x720
    BYTE field_0x724[0x14];
    SceneNode *pWheelNodes[4];        // 0x738
    SceneNode *pViewNodeNear;         // 0x748  child node placed towards the view
    SceneNode *pViewNodeFar;          // 0x74c  child node placed away from the view
    FixMatrix *pWorld;                // 0x750
    FixMatrix *pBodyMatrix;           // 0x754
    BYTE field_0x758[0x4];
    int field_0x75c;                  // 0x75c
    int field_0x760;                  // 0x760
    int scale0x764;                   // 0x764
    int scale0x768;                   // 0x768
    int scale0x76c;                   // 0x76c
    BYTE field_0x770[0x8];
    int speed;                        // 0x778  length of the velocity vector
    BYTE field_0x77c[0x18];
    int field_0x794;                  // 0x794
    BYTE field_0x798[0xc];
    int field_0x7a4;                  // 0x7a4
    BYTE field_0x7a8[0x4];
    int field_0x7ac;                  // 0x7ac
    BYTE field_0x7b0[0x4];
    int field_0x7b4;                  // 0x7b4
    BYTE field_0x7b8[0x4];
    int field_0x7bc[4];               // 0x7bc
    BYTE field_0x7cc[0x10];
    int field_0x7dc[4];               // 0x7dc
    BYTE field_0x7ec[0x1c];
    int field_0x808[4];               // 0x808
    BYTE field_0x818[0x4];
    int field_0x81c;                  // 0x81c
    BYTE field_0x820[0xc];
    int field_0x82c;                  // 0x82c
    int field_0x830;                  // 0x830
    BYTE field_0x834[0x4];
    int field_0x838;                  // 0x838
    BYTE field_0x83c[0xc];
    int field_0x848;                  // 0x848
    BYTE field_0x84c[0x4];
    int wheelTorque[4];               // 0x850  drive/brake torque per wheel
    int wheelLoad[4];                 // 0x860  paired per axle; Car_BalanceWheelPairs evens each pair out
    int field_0x870[4];               // 0x870
    int field_0x880[4];               // 0x880
    BYTE field_0x890[0x24];
    int field_0x8b4;                  // 0x8b4
    int field_0x8b8[4];               // 0x8b8  per-wheel torque rebuilt every step
    BYTE field_0x8c8[0x10];
    int field_0x8d8;                  // 0x8d8
    int cornerHeight[8];              // 0x8dc  ground height under each box corner
    BYTE field_0x8fc[0x20];
    int field_0x91c;                  // 0x91c
    int field_0x920;                  // 0x920
    int field_0x924;                  // 0x924
    BYTE field_0x928[0x60];
    int wheel0x988[4];                // 0x988
    int field_0x998[4];               // 0x998
    int wheel0x9a8[4];                // 0x9a8
    BYTE field_0x9b8[0x4];
    int field_0x9bc;                  // 0x9bc
    BYTE field_0x9c0[0x4];
    int field_0x9c4;                  // 0x9c4
    BYTE field_0x9c8[0x10];
    int wheel0x9d8[4];                // 0x9d8
    BYTE field_0x9e8[0x64];
    int field_0xa4c[4];               // 0xa4c
    int field_0xa5c[4];               // 0xa5c
    BYTE field_0xa6c[0x4];
    int field_0xa70;                  // 0xa70
    BYTE field_0xa74[0x3a];
    short wheelSurface[4];            // 0xaae
    BYTE field_0xab6[0x5a];
    unsigned short heading;           // 0xb10  12-bit angle
    BYTE field_0xb12[0x8];
    char field_0xb1a;                 // 0xb1a  index of this car in the timing records
    BYTE field_0xb1b[0x3];
    char field_0xb1e;                 // 0xb1e
    BYTE field_0xb1f[0x9];
    BYTE field_0xb28;                 // 0xb28
    BYTE field_0xb29;                 // 0xb29
    BYTE field_0xb2a[0x2];
    char cornerFlags[8];              // 0xb2c  set while a corner is disabled
    char field_0xb34;                 // 0xb34
    BYTE field_0xb35[0x2b];
    int field_0xb60;                  // 0xb60
    int field_0xb64;                  // 0xb64
    BYTE field_0xb68[0xc];
    int field_0xb74;                  // 0xb74
    BYTE field_0xb78[0xc];
    int field_0xb84;                  // 0xb84
    BYTE field_0xb88[0x24];
    int field_0xbac[8];               // 0xbac
    BYTE field_0xbcc[0x34];
    int field_0xc00;                  // 0xc00  8 corners instead of 4 when set
    BYTE field_0xc04[0xc];
    int field_0xc10;                  // 0xc10
    BYTE field_0xc14[0x10];
};

// Stored transforms of a car, written by the physics and applied to the
// scene nodes each frame.
struct CarTransforms {
    FixMatrix body;                 // 0x0   applied to Car::pNode0x71c
    FixMatrix body2;                // 0x40  applied to Car::pNode0x720
    BYTE field_0x80[0x60];
    FixVector groundNormal;         // 0xe0  normal of the ground under the car
    int groundHeight;               // 0xec
    BYTE field_0xf0[0xc];
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

// GLOBAL: CMR2 0x00519c8c
extern int g_physicsTimeStep;
// GLOBAL: CMR2 0x00519c90
extern int g_physicsScale;

// Scratch globals of the body lean solver (Car_UpdateBodyLean)
// GLOBAL: CMR2 0x0053c9f8
extern FixVector g_leanDamping;
// GLOBAL: CMR2 0x0053ca18
extern FixVector g_leanAccel;
// GLOBAL: CMR2 0x0053ca30
extern FixVector g_leanDelta;
// GLOBAL: CMR2 0x0053ca78
extern FixVector g_carAccel;
// GLOBAL: CMR2 0x0053caa8
extern FixBasis g_leanBasis;

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
void Car_UpdateCornerVelocities(void);
void Car_UpdateGroundNormal(void);
void Car_UpdateBodyLean(void);
void Car_BalanceWheelPairs(void);
void Car_ApplyCornerFriction(int grip);
void Car_UpdateWheelTorques(void);
void Car_Integrate(void);
void Car_UpdateWheelForces(void);

// Defined in FixedPoint.cpp; declared here because adding it to FixedPoint.h
// perturbs the code MSVC6 generates for every translation unit that includes it.
void FixBasis_Integrate(FixVector *pRows, FixVector *pW);

#endif
