#include <windows.h>
#include "Collision2D.h"

// GLOBAL: CMR2 0x00591438
FixVector g_collisionQuad[4];

#define FIX_ABS(x) ((x) < 0 ? -(x) : (x))

// Intersects the ray pDir with the four sides of the collision quad and returns
// the distance to the closest side (0x7d000000 when it misses), along with the
// side that was hit.
// match 22%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00489060
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

// GLOBAL: CMR2 0x005914a8
FixVector g_unk0x005914a8;
// GLOBAL: CMR2 0x005914b8
FixVector g_unk0x005914b8;
// GLOBAL: CMR2 0x005915e8
FixVector g_unk0x005915e8;

// Splits a movement of `amount` along pDir between the two collision boxes: the
// scale*amount part goes to pB and the opposite of the remainder to pA.  Both
// boxes get their four local vectors, their eight-vector array and the vertex
// at +0x94 moved by the same amount.
// match 77%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// The instruction sequence is the original's; only the register numbering of
// the induction variables and the base/index order of the two 0x60 loops differ.
// FUNCTION: CMR2 0x004894b0
void FUN_004894b0(int *pA, int *pB, int *pDir, int amount, int scale)
{
    FixVector v;
    int t;
    int rest;
    int *p;
    int i;
    int j;

    g_unk0x005915e8.x = pDir[0];
    g_unk0x005915e8.y = pDir[1];
    g_unk0x005915e8.z = pDir[2];
    if (amount > 0) {
        t = FixMul(amount, scale);
        rest = amount - t;
        if (scale != 0) {
            FixVecScale(&v, (FixVector *)pDir, t);
            // the original accumulates y, z and x in this order
            g_unk0x005914b8.y += v.y;
            g_unk0x005914b8.z += v.z;
            g_unk0x005914b8.x += v.x;
            if ((int *)pB[0x25] != NULL && pB[0x24] != 0) {
                ((int *)pB[0x25])[0] += v.x;
                ((int *)pB[0x25])[1] += v.y;
                ((int *)pB[0x25])[2] += v.z;
                for (i = 0; i < 0x60; i += 0xc) {
                    *(int *)(pB[0x24] + i) += v.x;
                    *(int *)(pB[0x24] + i + 4) += v.y;
                    *(int *)(pB[0x24] + i + 8) += v.z;
                }
                p = pB + 0xd;
                for (j = 4; j != 0; j--) {
                    p[-1] += v.x;
                    p[0] += v.y;
                    p[1] += v.z;
                    p += 3;
                }
            }
        }
        if (scale != 0x10000) {
            FixVecScale(&v, (FixVector *)pDir, -rest);
            g_unk0x005914a8.y += v.y;
            g_unk0x005914a8.z += v.z;
            g_unk0x005914a8.x += v.x;
            if ((int *)pA[0x25] != NULL && pA[0x24] != 0) {
                ((int *)pA[0x25])[0] += v.x;
                ((int *)pA[0x25])[1] += v.y;
                ((int *)pA[0x25])[2] += v.z;
                for (i = 0; i < 0x60; i += 0xc) {
                    *(int *)(pA[0x24] + i) += v.x;
                    *(int *)(pA[0x24] + i + 4) += v.y;
                    *(int *)(pA[0x24] + i + 8) += v.z;
                }
                p = pA + 0xd;
                for (j = 4; j != 0; j--) {
                    p[-1] += v.x;
                    p[0] += v.y;
                    p[1] += v.z;
                    p += 3;
                }
            }
        }
    }
}

// Clamps the magnitude of each component of v to limit (y only when clampY).
// match 86%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0048c6e0
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

extern int g_physicsScale;

// Clamps a vector to the scaled limits: eight units on X/Z, four on Y.
// match 56%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0048c750
void FUN_0048c750(int *v)
{
    if (v[0] > FixMul(g_physicsScale, 0x80000))
        v[0] = FixMul(g_physicsScale, 0x80000);
    else if (v[0] < -FixMul(g_physicsScale, 0x80000))
        v[0] = -FixMul(g_physicsScale, 0x80000);
    if (v[1] > FixMul(g_physicsScale, 0x40000))
        v[1] = FixMul(g_physicsScale, 0x40000);
    else if (v[1] < -FixMul(g_physicsScale, 0x40000))
        v[1] = -FixMul(g_physicsScale, 0x40000);
    if (v[2] > FixMul(g_physicsScale, 0x80000))
        v[2] = FixMul(g_physicsScale, 0x80000);
    else if (v[2] < -FixMul(g_physicsScale, 0x80000))
        v[2] = -FixMul(g_physicsScale, 0x80000);
}

