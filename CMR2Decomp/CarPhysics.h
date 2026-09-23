#ifndef _CAR_PHYSICS_H
#define _CAR_PHYSICS_H

#include <windows.h>
#include "FixedPoint.h"

// Contact state of one car with the ground (0x2a4 bytes).
struct CarContact {
    FixVector wheelCorners[4][4];   // 0x000 corners of each wheel's contact patch
    FixVector skidPoints[32];       // 0x0c0 skid mark trail
    int wheelGroundY[4];            // 0x240 ground height under each wheel
    int field_0x250;                // 0x250
    int wheelGrip[4];               // 0x254 1.0 when the wheel is on the ground
    short wheelTriangle[4];         // 0x264 cached collision triangle of each wheel
    BYTE field_0x26c[0x28];         // 0x26c
    int field_0x294;                // 0x294 not driven (ghost / remote car)
    int field_0x298;                // 0x298
    int field_0x29c;                // 0x29c
    int field_0x2a0;                // 0x2a0
};

#endif
