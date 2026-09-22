#ifndef _COLLISION2D_H
#define _COLLISION2D_H

#include "FixedPoint.h"

// The four corners of the quad the collision tests run against.
extern FixVector g_collisionQuad[4];

int Collision_RayQuad(FixVector *pDir, int *pEdge, BYTE *pCorner);

#endif
