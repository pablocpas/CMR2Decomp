#ifndef _CAR_H
#define _CAR_H

#include "FixedPoint.h"
#include "SceneNode.h"
#include "Graphics.h"

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
    FixVector field_0x564;            // 0x564  ground normal used while field_0xbac[4] is set
    BYTE field_0x570[0x54];
    FixVector field_0x5c4;            // 0x5c4
    FixVector field_0x5d0;            // 0x5d0
    BYTE field_0x5dc[0xc];
    FixVector cornerLoad[8];          // 0x5e8  normal force of the ground at each corner
    FixVector cornerForce[8];         // 0x648  force accumulated at each corner
    FixVector baseForce;              // 0x6a8  constant force applied every step
    FixVector groundDir[2];           // 0x6b4  front/rear rolling direction on the ground plane
    FixVector groundAxis[2];          // 0x6cc  front/rear lateral axis on the ground plane
    FixVector wheelLean;              // 0x6e4  lean of the wheel frame, integrated like lean
    FixVector lean;                   // 0x6f0  body lean (x/z tilt) fed into the body matrix
    BYTE field_0x6fc[0x20];
    SceneNode *pNode0x71c;            // 0x71c
    SceneNode *pNode0x720;            // 0x720
    SceneNode *pNode0x724;            // 0x724  second body part (optional)
    SceneNode *pExtraNodes[4];        // 0x728  optional extra parts (0x728 set means all four)
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
    int tipRatio;                     // 0x77c  sideways slide relative to the tip-over threshold, eased
    int field_0x780;                  // 0x780  engine acceleration while no gear is engaged
    int field_0x784;                  // 0x784  drag factor of the engine speed
    BYTE field_0x788[0x8];
    int field_0x790;                  // 0x790
    int field_0x794;                  // 0x794
    BYTE field_0x798[0x4];
    int field_0x79c;                  // 0x79c  how fast the rolling direction follows the body
    BYTE field_0x7a0[0x4];
    int field_0x7a4;                  // 0x7a4
    BYTE field_0x7a8[0x4];
    int field_0x7ac;                  // 0x7ac
    int field_0x7b0;                  // 0x7b0  excess revs after limiting
    int field_0x7b4;                  // 0x7b4
    BYTE field_0x7b8[0x4];
    int field_0x7bc[8];               // 0x7bc
    int field_0x7dc[8];               // 0x7dc
    int field_0x7fc;                  // 0x7fc  set from the difficulty (0x43e530)
    int field_0x800;                  // 0x800
    int field_0x804;                  // 0x804
    int field_0x808[4];               // 0x808
    int field_0x818;                  // 0x818  ramps -1..1 with flag0x1d0[0]/[1] (0x494540)
    int field_0x81c;                  // 0x81c
    int field_0x820;                  // 0x820
    int field_0x824;                  // 0x824  steering torque
    BYTE field_0x828[0x4];
    int field_0x82c;                  // 0x82c
    int field_0x830;                  // 0x830
    int field_0x834;                  // 0x834
    int field_0x838;                  // 0x838
    int field_0x83c;                  // 0x83c  swing phase of 0x838
    int field_0x840;                  // 0x840
    int field_0x844;                  // 0x844  target of the 0x848 swing
    int field_0x848;                  // 0x848
    int field_0x84c;                  // 0x84c  swing phase (0..1)
    int wheelTorque[4];               // 0x850  drive/brake torque per wheel
    int wheelLoad[4];                 // 0x860  paired per axle; Car_BalanceWheelPairs evens each pair out
    int field_0x870[4];               // 0x870
    int field_0x880[4];               // 0x880
    int field_0x890[4];               // 0x890  filtered wheel spin (front lean)
    int field_0x8a0[4];               // 0x8a0  filtered wheel spin (body lean)
    int cornerMass;                   // 0x8b0  mass carried by each touching corner
    int field_0x8b4;                  // 0x8b4
    int field_0x8b8[8];               // 0x8b8  per-wheel torque rebuilt every step (8 corners)
    int field_0x8d8;                  // 0x8d8
    int cornerHeight[8];              // 0x8dc  ground height under each box corner
    int field_0x8fc;                  // 0x8fc
    BYTE field_0x900[0x1c];
    int field_0x91c;                  // 0x91c
    int field_0x920;                  // 0x920
    int field_0x924;                  // 0x924
    BYTE field_0x928[0x30];
    int field_0x958;                  // 0x958  how far the car sank into the ground (<= 0)
    BYTE field_0x95c[0x10];
    int field_0x96c;                  // 0x96c
    BYTE field_0x970[0x18];
    int wheel0x988[4];                // 0x988
    int field_0x998[4];               // 0x998  suspension height of each corner, front lean frame
    int wheel0x9a8[4];                // 0x9a8  suspension height of each corner, body lean frame
    int field_0x9b8;                  // 0x9b8  damping of wheelLean
    int field_0x9bc;                  // 0x9bc
    BYTE field_0x9c0[0x4];
    int field_0x9c4;                  // 0x9c4
    BYTE field_0x9c8[0x10];
    int wheel0x9d8[4];                // 0x9d8
    BYTE field_0x9e8[0x24];
    int cornerGripA[8];               // 0xa0c  grip limits of a corner without a wheel
    int cornerGripB[8];               // 0xa2c
    int field_0xa4c[4];               // 0xa4c
    int field_0xa5c[4];               // 0xa5c
    BYTE field_0xa6c[0x4];
    int field_0xa70;                  // 0xa70
    BYTE field_0xa74[0x8];
    int field_0xa7c;                  // 0xa7c
    BYTE field_0xa80[0xc];
    int field_0xa8c;                  // 0xa8c  camera shake
    BYTE field_0xa90[0xc];
    unsigned short steepTime;         // 0xa9c  steps spent on a slope too steep to stand on
    short cornerTriangle[8];          // 0xa9e  cached collision triangle under each corner
    short wheelSurface[4];            // 0xaae
    BYTE field_0xab6[0x8];
    short field_0xabe[4];             // 0xabe  surface of each wheel at the last step
    short wheelSurfaceType[4];        // 0xac6  surface id under each wheel
    BYTE field_0xace[0x30];
    short field_0xafe;                // 0xafe  engine startup countdown
    BYTE field_0xb00[0x10];
    unsigned short heading;           // 0xb10  12-bit angle
    BYTE field_0xb12[0x6];
    short tipAngle;                   // 0xb18  12-bit angle the body tips by
    char field_0xb1a;                 // 0xb1a  index of this car in the timing records
    BYTE field_0xb1b[0x2];
    char field_0xb1d;                 // 0xb1d
    char field_0xb1e;                 // 0xb1e
    char field_0xb1f;                 // 0xb1f
    char field_0xb20;                 // 0xb20  requested gear
    char field_0xb21;                 // 0xb21  shift delay
    char field_0xb22;                 // 0xb22  shift direction
    BYTE field_0xb23[0x1];
    char field_0xb24;                 // 0xb24  shift mode snapshot
    BYTE field_0xb25;                 // 0xb25  countdowns started by 0x43b020
    BYTE field_0xb26;
    BYTE field_0xb27;
    BYTE field_0xb28;                 // 0xb28
    BYTE field_0xb29;                 // 0xb29
    BYTE field_0xb2a[0x2];
    char cornerFlags[8];              // 0xb2c  set while a corner is disabled
    char field_0xb34;                 // 0xb34
    BYTE field_0xb35[0xd];
    char field_0xb42;                 // 0xb42
    BYTE field_0xb43[0x5];
    int field_0xb48;                  // 0xb48
    int field_0xb4c;                  // 0xb4c  engine restart flag
    BYTE field_0xb50[0x10];
    int field_0xb60;                  // 0xb60
    int field_0xb64;                  // 0xb64
    BYTE field_0xb68[0xc];
    int field_0xb74;                  // 0xb74
    int field_0xb78;                  // 0xb78  rev limiter active
    BYTE field_0xb7c[0x8];
    int field_0xb84;                  // 0xb84
    BYTE field_0xb88[0xc];
    int field_0xb94;                  // 0xb94  automatic shift lock
    int field_0xb98;                  // 0xb98  current gear is at or below the best one
    int field_0xb9c;                  // 0xb9c  automatic gearbox enabled
    BYTE field_0xba0[0xc];
    int field_0xbac[8];               // 0xbac
    int field_0xbcc[4];               // 0xbcc
    int field_0xbdc;                  // 0xbdc
    int field_0xbe0;                  // 0xbe0
    int field_0xbe4;                  // 0xbe4
    int wheelSlipping[4];             // 0xbe8  set while the wheel spins faster than the ground
    int field_0xbf8;                  // 0xbf8
    int field_0xbfc;                  // 0xbfc
    int field_0xc00;                  // 0xc00  8 corners instead of 4 when set
    BYTE field_0xc04[0x8];
    int field_0xc0c;                  // 0xc0c
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
    int cornerHeight[4];            // 0xec  ground height under the body corners
};

// GLOBAL: CMR2 0x0053b560
extern CarTransforms g_carTransforms[8];
// GLOBAL: CMR2 0x0053bda0
extern FixMatrix g_carWheelTransforms[8][4];
// GLOBAL: CMR2 0x0053a3a0
extern short g_carOrderCount;
// GLOBAL: CMR2 0x0053b500
extern short g_carOrder[48];
// GLOBAL: CMR2 0x0053a324
extern int g_carViewScale[15][2];
// GLOBAL: CMR2 0x00538e2c
extern SceneNode *g_viewNodes[5];

// arccos as a 12-bit angle: 4096 entries for a dot product in [-1, 1]
// GLOBAL: CMR2 0x006e6ef4
extern short g_acosTable[4096];

inline short FixAcos(int x)
{
    int neg = 0;
    double t;

    if (x < 0) {
        x = -x;
        neg = 1;
    }
    if (x > 0x10000) {
        return g_acosTable[4095];
    }
    t = (double)x * CGraphics::m_oneOver65536 * -4095.0;
    if (neg) {
        return -g_acosTable[-(__int64)t];
    }
    return g_acosTable[-(__int64)t];
}

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
void Car_UpdateEngineSpeed(void);
void Car_UpdateAutomaticGear(void);

// Defined in FixedPoint.cpp; declared here because adding it to FixedPoint.h
// perturbs the code MSVC6 generates for every translation unit that includes it.
void FixBasis_Integrate(FixVector *pRows, FixVector *pW);

#endif
