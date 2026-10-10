#ifndef _CAR_PHYSICS_H
#define _CAR_PHYSICS_H

#include "FixedPoint.h"

// Contact state of one car with the ground (0x2a4 bytes).
struct CarContact {
    FixVector wheelCorners[4][4];   // 0x000 corners of each wheel's contact patch
    FixVector points[18];           // 0x0c0 body patch corners (0-3), then the skid trail
    FixVector wheelFrontMid[4];     // 0x198 middle of the front edge of each wheel patch
    FixVector wheelRearMid[4];      // 0x1c8 middle of the rear edge
    int pointGroundY[18];           // 0x1f8 ground height under each point
    int wheelGroundY[4];            // 0x240 ground height under each wheel
    int shadowLevel;               // 0x250 16.16 shadow intensity/softness
    int wheelGrip[4];               // 0x254 1.0 when the wheel is on the ground
    short wheelTriangle[4];         // 0x264 cached collision triangle of each wheel
    short pointTriangle[18];        // 0x26c cached collision triangle of each point
    short trailEdge;                // 0x290 body corner the skid trail starts from
    short pointCount;               // 0x292 points in use
    int ghostContact;              // 0x294 ghost body patch, opaque shadow, no skid trail
    int wheelPatchesEnabled;       // 0x298 update/draw wheel patches
    int skidIndexOffset;           // 0x29c subtract 0/1 before range testing
    int skidRangeAscending;        // 0x2a0 reverse range indices when zero
};

#endif
