#ifndef _CAR_PHYSICS_H
#define _CAR_PHYSICS_H

#include <windows.h>
#include "FixedPoint.h"

// Contact state of one car with the ground (0x2a4 bytes).
struct CarContact {
    FixVector wheelCorners[4][4];   // 0x000 corners of each wheel's contact patch
    FixVector bodyCorners[4];       // 0x0c0 corners of the body's contact patch
    FixVector skidPoints[14];       // 0x0f0 skid mark trail behind the body
    FixVector wheelFrontMid[4];     // 0x198 middle of the front edge of each wheel patch
    FixVector wheelRearMid[4];      // 0x1c8 middle of the rear edge
    BYTE field_0x1f8[0x48];         // 0x1f8
    int wheelGroundY[4];            // 0x240 ground height under each wheel
    int field_0x250;                // 0x250
    int wheelGrip[4];               // 0x254 1.0 when the wheel is on the ground
    short wheelTriangle[4];         // 0x264 cached collision triangle of each wheel
    BYTE field_0x26c[0x24];         // 0x26c
    short trailEdge;                // 0x290 body corner the skid trail starts from
    short pointCount;               // 0x292 body corners plus skid points in use
    int field_0x294;                // 0x294 not driven (ghost / remote car)
    int field_0x298;                // 0x298
    int field_0x29c;                // 0x29c
    int field_0x2a0;                // 0x2a0
};

#endif
