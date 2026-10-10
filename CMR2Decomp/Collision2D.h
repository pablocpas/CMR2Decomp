#ifndef _COLLISION2D_H
#define _COLLISION2D_H

#include "FixedPoint.h"
#include <stddef.h>

// Runtime collision box: axes/points plus native pointers to object geometry.
// This record is constructed in memory, never loaded as a file header.
struct CollisionBox {
    int halfWidth;          // 0x00 extent along axisA
    int halfLength;         // 0x04 extent along axisB
    int top;                // 0x08 world upper y
    int bottom;             // 0x0c world lower y
    FixVector axisA;        // 0x10
    FixVector axisB;        // 0x1c
    BYTE pad_0x28[0x8];
    union {
        FixVector points[8];    // 0x30 first four are the footprint
        int pointWords[24];     // same pointer-free vector span
    };
    FixVector *pArray;      // 0x90 eight world corners
    FixVector *pVertex;     // 0x94 world centre
};

// Original footprint cursor points at y within the pointer-free word view.
inline FixVector *CollisionBoxPointAtY(int *p)
{
    return (FixVector *)((BYTE *)p - offsetof(FixVector, y));
}

// The four corners of the quad the collision tests run against.
extern FixVector g_collisionQuad[4];

int Collision_RayQuad(FixVector *pDir, int *pEdge, BYTE *pCorner);

#endif
