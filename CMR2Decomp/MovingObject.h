#ifndef _MOVING_OBJECT_H
#define _MOVING_OBJECT_H

#include "FixedPoint.h"
#include "SectorObjectEntry.h"
struct SceneNode;

// Runtime moving-object state; constructed by the mesh loader and collision queue.
struct MovingObject {
    SectorObjectEntry *pEntry;  // 0x000 runtime sector-list entry
    SceneNode *pObject;         // 0x004 scene node (head of the object's node chain)
    FixMatrix keyB;             // 0x008 previous key
    FixMatrix keyA;             // 0x048 next key
    FixMatrix current;          // 0x088 interpolated matrix
    FixMatrix *pMatrix;         // 0x0c8 matrix the motion integrates (keyB)
    int mode;                   // 0x0cc 1 = sliding on the ground, 2 = airborne
    int objectType;             // 0x0d0
    FixVector groundNormal;     // 0x0d4
    FixVector spin;             // 0x0e0 angular step
    FixVector velocity;         // 0x0ec
    FixVector field_0xf8;       // 0x0f8
    FixVector probe;            // 0x104 ground-probe half extents (x, y, z)
    int groundHeight;           // 0x110 last ground height under the probe
    int actionState;            // 0x114 node action / sound callback state
    int sectorIndex;           // 0x118 sector passed by the collision caller
    short groundTriangle;       // 0x11c last ground triangle (-1 = none)
    short field_0x11e;
    int field_0x120;            // 0x120
    int moved;                  // 0x124 the interpolated matrix changed this frame
};

struct MovingObjects {
    MovingObject entries[40];    // 0x0000
    BYTE meshCount;                       // 0x2e40
    BYTE field_0x2e41[0x103];
    int carDistance[40 * 8];              // 0x2f44 [entry * 8 + car]
    BYTE count;                           // 0x3444 entries in use
};

#endif
