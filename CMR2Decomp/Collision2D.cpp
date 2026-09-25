#include <windows.h>
#include "Collision2D.h"

FixVector g_collisionQuad[4];

#define FIX_ABS(x) ((x) < 0 ? -(x) : (x))

// Intersects the ray pDir with the four sides of the collision quad and returns
// the distance to the closest side (0x7d000000 when it misses), along with the
// side that was hit.
// TODO: CMR2 0x00489060 (implemented, match 22%)
// Every expression and branch was traced against the disassembly; what does not
// match is the frame layout: the original keeps the returned distance in a stack
// slot (ebp-0x14) while MSVC keeps ours in edi, which shifts every other slot.
int Collision_RayQuad(FixVector *pDir, int *pEdge, BYTE *pCorner)
{
    FixVector d3;
    FixVector d0;
    int ax;
    int az;
    int bz;
    int cx;
    int cz;
    int dx;
    int cross;
    int recip;
    int t;
    int u;
    int result;

    result = 0x7d000000;
    d3.x = g_collisionQuad[3].x - g_collisionQuad[2].x;
    d3.y = g_collisionQuad[3].y - g_collisionQuad[2].y;
    d3.z = g_collisionQuad[3].z - g_collisionQuad[2].z;
    d0.x = g_collisionQuad[1].x - g_collisionQuad[0].x;
    d0.y = g_collisionQuad[1].y - g_collisionQuad[0].y;
    d0.z = g_collisionQuad[1].z - g_collisionQuad[0].z;
    ax = g_collisionQuad[0].x - g_collisionQuad[2].x;
    az = g_collisionQuad[2].z - g_collisionQuad[0].z;
    bz = g_collisionQuad[3].z - g_collisionQuad[0].z;
    cx = g_collisionQuad[1].x - g_collisionQuad[2].x;
    cz = g_collisionQuad[2].z - g_collisionQuad[1].z;
    dx = g_collisionQuad[0].x - g_collisionQuad[3].x;

    cross = FixMul(d0.z, d3.x) - FixMul(d0.x, d3.z);
    if (FIX_ABS(cross) > 0x28f) {
        recip = FixDiv(0x10000, cross);
        t = FixMul(FixMul(d0.z, ax) + FixMul(d0.x, az), recip);
        if (t >= 0 && t <= 0x10000) {
            t = FixMul(FixMul(d3.z, ax) + FixMul(d3.x, az), recip);
            if (t >= 0 && t <= 0x10000) {
                cross = FixMul(d3.x, pDir->z) - FixMul(d3.z, pDir->x);
                if (FIX_ABS(cross) < 0x290) {
                    result = 0x7d000000;
                } else {
                    recip = FixDiv(0x10000, cross);
                    t = FixMul(FixMul(pDir->z, ax) + FixMul(pDir->x, az), recip);
                    if (t < 0 || t > 0x10000) {
                        result = 0x7d000000;
                    } else {
                        t = -FixMul(FixMul(d3.z, ax) + FixMul(d3.x, az), recip);
                        if (t < 0 || t > 0x7cffffff) {
                            result = 0x7d000000;
                        } else {
                            *pEdge = 0;
                            *pCorner = 0;
                            result = t;
                        }
                    }
                    u = FixMul(FixMul(pDir->z, cx) + FixMul(pDir->x, cz), recip);
                    if (u >= 0 && u <= 0x10000) {
                        u = -FixMul(FixMul(d3.z, cx) + FixMul(d3.x, cz), recip);
                        if (u >= 0 && u < result) {
                            *pEdge = 0;
                            *pCorner = 1;
                            result = u;
                        }
                    }
                }

                cross = FixMul(d0.z, pDir->x) - FixMul(d0.x, pDir->z);
                if (FIX_ABS(cross) < 0x290) {
                    return result;
                }
                recip = FixDiv(0x10000, cross);
                u = FixMul(FixMul(pDir->z, ax) + FixMul(pDir->x, az), recip);
                if (u >= 0 && u <= 0x10000) {
                    u = FixMul(FixMul(d0.z, ax) + FixMul(d0.x, az), recip);
                    if (u >= 0 && u < result) {
                        *pEdge = 1;
                        *pCorner = 2;
                        result = u;
                    }
                }
                u = FixMul(FixMul(pDir->z, dx) + FixMul(pDir->x, bz), recip);
                if (u < 0) {
                    return result;
                }
                if (u > 0x10000) {
                    return result;
                }
                u = FixMul(FixMul(d0.z, dx) + FixMul(d0.x, bz), recip);
                if (u < 0) {
                    return result;
                }
                if (result <= u) {
                    return result;
                }
                *pEdge = 1;
                *pCorner = 3;
                return u;
            }
        }
    }
    return result;
}

// Clamps the magnitude of each component of v to limit (y only when clampY).
// TODO: CMR2 0x0048c6e0 (implemented, match 86%)
void FUN_0048c6e0(int *v, int *limit, int clampY)
{
    int a;

    a = v[0] < 0 ? -v[0] : v[0];
    if (limit[0] < a)
        v[0] = v[0] < 0 ? -limit[0] : limit[0];
    if (clampY != 0) {
        a = v[1] < 0 ? -v[1] : v[1];
        if (limit[1] < a)
            v[1] = v[1] < 0 ? -limit[1] : limit[1];
    }
    a = v[2] < 0 ? -v[2] : v[2];
    if (limit[2] < a)
        v[2] = v[2] < 0 ? -limit[2] : limit[2];
}

