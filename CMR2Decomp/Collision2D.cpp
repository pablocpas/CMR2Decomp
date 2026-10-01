#include <windows.h>
#include "Collision2D.h"

// GLOBAL: CMR2 0x00591438
FixVector g_collisionQuad[4];

#define FIX_ABS(x) ((x) < 0 ? -(x) : (x))

// Intersects the ray pDir with the four sides of the collision quad and returns
// the distance to the closest side (0x7d000000 when it misses), along with the
// side that was hit.
// match 22%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES).
// Every expression and branch was traced against the disassembly; what does not
// match is the frame layout: the original keeps the returned distance in a stack
// slot (ebp-0x14) while MSVC keeps ours in edi, which shifts every other slot.
// FUNCTION: CMR2 0x00489060
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

    cross = FixMul(d3.x, d0.z) - FixMul(d0.x, d3.z);
    if (FIX_ABS(cross) > 0x28f) {
        recip = FixDiv(0x10000, cross);
        t = FixMul(FixMul(d0.z, ax) + FixMul(d0.x, az), recip);
        if (t >= 0 && t <= 0x10000) {
            t = FixMul(recip, FixMul(d3.z, ax) + FixMul(d3.x, az));
            if (t >= 0 && t <= 0x10000) {
                cross = FixMul(d3.x, pDir->z) - FixMul(d3.z, pDir->x);
                if (FIX_ABS(cross) < 0x290) {
                    result = 0x7d000000;
                } else {
                    recip = FixDiv(0x10000, cross);
                    t = FixMul(recip, FixMul(pDir->z, ax) + FixMul(pDir->x, az));
                    if (t < 0 || t > 0x10000) {
                        result = 0x7d000000;
                    } else {
                        t = -FixMul(recip, FixMul(ax, d3.z) + FixMul(az, d3.x));
                        if (t < 0 || t > 0x7cffffff) {
                            result = 0x7d000000;
                        } else {
                            *pEdge = 0;
                            *pCorner = 0;
                            result = t;
                        }
                    }
                    u = FixMul(recip, FixMul(pDir->z, cx) + FixMul(pDir->x, cz));
                    if (u >= 0 && u <= 0x10000) {
                        u = -FixMul(recip, FixMul(d3.z, cx) + FixMul(d3.x, cz));
                        if (u >= 0 && u < result) {
                            *pEdge = 0;
                            *pCorner = 1;
                            result = u;
                        }
                    }
                }

                cross = FixMul(pDir->x, d0.z) - FixMul(d0.x, pDir->z);
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
// FUNCTION: CMR2 0x0048c6e0
void FUN_0048c6e0(int *v, int *limit, int clampY)
{
    int a;

    a = v[0] < 0 ? -v[0] : v[0];
    if (a > limit[0])
        v[0] = v[0] < 0 ? -limit[0] : limit[0];
    if (clampY != 0) {
        a = v[1] < 0 ? -v[1] : v[1];
        if (a > limit[1])
            v[1] = v[1] < 0 ? -limit[1] : limit[1];
    }
    a = v[2] < 0 ? -v[2] : v[2];
    if (a > limit[2])
        v[2] = v[2] < 0 ? -limit[2] : limit[2];
}

extern int g_physicsScale;

// Clamps a vector to the scaled limits: eight units on X/Z, four on Y.
// match 56%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0048c750
void FUN_0048c750(int *v)
{
    if (v[0] > FixMul(0x80000, g_physicsScale))
        v[0] = FixMul(0x80000, g_physicsScale);
    else if (v[0] < -FixMul(0x80000, g_physicsScale))
        v[0] = -FixMul(0x80000, g_physicsScale);
    if (v[1] > FixMul(0x40000, g_physicsScale))
        v[1] = FixMul(0x40000, g_physicsScale);
    else if (v[1] < -FixMul(0x40000, g_physicsScale))
        v[1] = -FixMul(0x40000, g_physicsScale);
    if (v[2] > FixMul(0x80000, g_physicsScale))
        v[2] = FixMul(0x80000, g_physicsScale);
    else if (v[2] < -FixMul(0x80000, g_physicsScale))
        v[2] = -FixMul(0x80000, g_physicsScale);
}


extern int g_unk0x00591490;
extern FixVector g_unk0x00591498;

// Resolves a collision between a car and the reference object: normalises the
// offset of the car's centre, projects the car's extent onto it, and either
// pushes the car out of the sphere or reports the impact side.
// match 16%: implementada, MSVC6 reutiliza slots de pila y ordena distinto el prologo/cuerpo
// FUNCTION: CMR2 0x00489b20
int FUN_00489b20(int *param_1, int *param_2, unsigned int *param_3, int param_4)
{
    int *piVar3 = param_1;
    int local_1c = 0;
    int local_14 = 0;
    int local_10 = 0;
    char local_5 = 0;
    int iVar4, iVar5, iVar13, iVar14;
    unsigned int uVar6, uVar7, uVar8, uVar15;
    unsigned int local_3c, local_34;
    int tmp;

    iVar13 = g_unk0x00591498.x - *(int *)(param_1[0x25]);
    iVar14 = g_unk0x00591498.z - *(int *)(param_1[0x25] + 8);
    uVar15 = (unsigned int)FixSqrt(FixMul(iVar13, iVar13) + FixMul(iVar14, iVar14));
    if (uVar15 == 0) {
        local_3c = 0;
        local_34 = 0;
    } else {
        iVar4 = (int)(0x100000000i64 / (__int64)(int)uVar15);
        local_3c = (unsigned int)FixMul(iVar13, iVar4);
        local_34 = (unsigned int)FixMul(iVar14, iVar4);
    }
    iVar4 = FixMul(param_1[6], local_34) + FixMul(param_1[4], local_3c);
    iVar5 = FixMul(param_1[9], local_34) + FixMul(param_1[7], local_3c);
    uVar6 = (unsigned int)(FixMul(param_1[6], iVar14) + FixMul(param_1[4], iVar13));
    uVar7 = (unsigned int)(FixMul(param_1[9], iVar14) + FixMul(param_1[7], iVar13));

    uVar15 = uVar6;
    if ((int)uVar6 < 0)
        uVar15 = -uVar6;

    if (*param_1 < (int)uVar15) {
        uVar15 = uVar7;
        if ((int)uVar7 < 0)
            uVar15 = -uVar7;
        if ((int)uVar15 > param_1[1])
            goto LABEL_00489f05;

        if ((int)uVar6 >= 1)
            uVar15 = (unsigned int)(0 < (int)uVar7);
        else
            uVar15 = (unsigned int)((0 < (int)uVar7) + 2);
        uVar8 = (unsigned int)FixMul(g_unk0x00591490, g_unk0x00591490);
        iVar14 = g_unk0x00591498.x - param_1[uVar15 * 3 + 0xc];
        iVar4 = g_unk0x00591498.z - param_1[uVar15 * 3 + 0xe];
        uVar15 = (unsigned int)FixMul(iVar4, iVar4);
        iVar13 = (int)uVar15 + FixMul(iVar14, iVar14);
        if ((int)uVar8 < iVar13)
            goto LABEL_0048a1e0;
        iVar14 = FixMul(local_34, iVar4) + FixMul(local_3c, iVar14);
        iVar4 = FixMul(local_34, local_34) + FixMul(local_3c, local_3c);
        uVar15 = (unsigned int)(FixMul(iVar14, iVar14) - FixMul(iVar4, iVar13 - (int)uVar8));
        if (-1 < (int)uVar15) {
            uVar15 = (unsigned int)FixSqrt(uVar15);
            uVar15 = uVar15 - iVar14;
            local_10 = FixDiv((int)uVar15, iVar4);
        }
        local_5 = 3;
    } else {
LABEL_00489f05:
        iVar14 = *param_1 + g_unk0x00591490;
        iVar13 = g_unk0x00591490 + param_1[1];
        uVar15 = uVar6;
        if ((int)uVar6 < 0)
            uVar15 = -uVar6;
        if (iVar14 < (int)uVar15)
            goto LABEL_0048a1e0;
        uVar15 = uVar7;
        if ((int)uVar7 < 0)
            uVar15 = -uVar7;
        if (iVar13 < (int)uVar15)
            goto LABEL_0048a1e0;

        {
            int bVar16 = 0;
            int bVar2 = 0;
            if (iVar4 < 0x42) {
                if (iVar4 < -0x41) {
                    uVar15 = (unsigned int)-(uVar6 + iVar14);
                    tmp = uVar6 + iVar14;
                    if (-1 < (int)uVar15)
                        tmp = (int)uVar15;
                    local_1c = FixDiv(tmp, -iVar4);
                    goto LABEL_00489f8f;
                }
            } else {
                uVar15 = (unsigned int)(iVar14 - uVar6);
                local_1c = FixDiv((int)uVar15, iVar4);
LABEL_00489f8f:
                bVar2 = 1;
            }
            if (iVar5 >= 0x42) {
                uVar15 = (unsigned int)(iVar13 - uVar7);
                local_14 = FixDiv((int)uVar15, iVar5);
                bVar16 = 1;
            } else {
                if (iVar5 < -0x41) {
                    uVar15 = (unsigned int)-(iVar13 + uVar7);
                    tmp = iVar13 + uVar7;
                    if (-1 < (int)uVar15)
                        tmp = (int)uVar15;
                    local_14 = FixDiv(tmp, -iVar5);
                    bVar16 = 1;
                }
            }
            if (bVar2 && local_1c < 0x7d000000)
                local_5 = 1;
            else
                local_1c = 0x7d000000;
            if (bVar16 && local_14 < local_1c) {
                local_5 = 2;
                local_1c = local_14;
            }
            if ((0 < local_1c) && (bVar16 || bVar2))
                local_10 = local_1c;
            if (local_5 == 0)
                goto LABEL_0048a1e0;
        }
    }

    g_unk0x005914a8.z = 0;
    if (local_10 < 1) {
        g_unk0x005914a8.x = 0;
    } else {
        int iVar;
        int *piVar9;
        iVar = -FixMul(local_10, param_4);
        g_unk0x005914a8.x = FixMul(local_3c, iVar);
        g_unk0x005914a8.z = FixMul(local_34, iVar);
        piVar9 = (int *)piVar3[0x25];
        if ((piVar9 != 0) && (piVar3[0x24] != 0)) {
            *piVar9 = *piVar9 + g_unk0x005914a8.x;
            *(int *)(piVar3[0x25] + 4) = *(int *)(piVar3[0x25] + 4);
            *(int *)(piVar3[0x25] + 8) = *(int *)(piVar3[0x25] + 8) + g_unk0x005914a8.z;
            iVar = 0;
            do {
                *(int *)(iVar + piVar3[0x24]) = *(int *)(iVar + piVar3[0x24]) + g_unk0x005914a8.x;
                *(int *)(iVar + 4 + piVar3[0x24]) = *(int *)(iVar + 4 + piVar3[0x24]);
                *(int *)(iVar + 8 + piVar3[0x24]) = *(int *)(iVar + 8 + piVar3[0x24]) + g_unk0x005914a8.z;
                iVar += 0xc;
            } while (iVar < 0x60);
            piVar9 = piVar3 + 0xd;
            iVar = 4;
            do {
                piVar9[-1] = piVar9[-1] + g_unk0x005914a8.x;
                *piVar9 = *piVar9;
                piVar9[1] = piVar9[1] + g_unk0x005914a8.z;
                piVar9 += 3;
                iVar--;
            } while (iVar != 0);
        }
    }
    g_unk0x005914a8.y = 0;
    *param_2 = FixDiv(FixMul(uVar6, *piVar3), *piVar3 + g_unk0x00591490);
    uVar15 = (unsigned int)FixDiv(FixMul(uVar7, piVar3[1]), g_unk0x00591490 + piVar3[1]);
    *param_3 = uVar15;

LABEL_0048a1e0:
    return (int)(unsigned char)local_5;
}

// Collision flags published for the rest of the physics step.
// GLOBAL: CMR2 0x005915dc
int g_unk0x005915dc;
// GLOBAL: CMR2 0x00591468
int g_unk0x00591468;

void FUN_0048c870(BYTE index, BYTE other, int *pDelta, int flag);

// Resolves the collision of `car` against the oriented box `pBox`: builds the
// correction vector from the two box axes and the two factors the sphere test
// returns, rotates it into world space, picks the face to push along (the
// direction to the reference object, or one of the two axes) and moves the car
// and the body of the box by the tangential remainder of the correction.
// match 59%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00489750
int FUN_00489750(int car, int *pBox, int scale)
{
    FixVector a;
    FixVector b;
    FixVector v;
    FixVector along;
    int factor;
    int t;
    int dot;
    int dx;
    int dy;
    int dz;
    int i;
    int j;
    int *p;
    char side;

    side = (char)FUN_00489b20(pBox, &factor, (unsigned int *)&t, scale);
    if (side == 0)
        return 0;

    a.x = FixMul(pBox[4], factor);
    a.y = FixMul(pBox[5], factor);
    a.z = FixMul(pBox[6], factor);
    b.x = FixMul(pBox[7], t);
    b.y = FixMul(pBox[8], t);
    b.z = FixMul(pBox[9], t);
    v.x = a.x + b.x;
    v.y = a.y + b.y;
    v.z = a.z + b.z;
    FixMatrix_InverseRotateVector((FixVector *)(car + 0x5dc), &v,
                                  *(FixMatrix **)(car + 0x750));

    if (side == 3) {
        int *pRaw = *(int **)(pBox + 0x94);

        g_unk0x005915e8.x = pRaw[0] + v.x;
        g_unk0x005915e8.x -= g_unk0x00591498.x;
        g_unk0x005915e8.y = pRaw[1] + v.y;
        g_unk0x005915e8.y = 0;
        g_unk0x005915e8.z = pRaw[2] + v.z;
        g_unk0x005915e8.z -= g_unk0x00591498.z;
        FIX_NORMALIZE_INTO(g_unk0x005915e8, g_unk0x005915e8)
    } else if (side == 1) {
        g_unk0x005915e8 = *(FixVector *)(pBox + 4);
    } else {
        g_unk0x005915e8 = *(FixVector *)(pBox + 7);
    }

    dot = FixVecDot(&g_unk0x005915e8, &g_unk0x005914a8);
    FixVecScale(&along, &g_unk0x005915e8, dot);
    dx = along.x - g_unk0x005914a8.x;
    dy = along.y - g_unk0x005914a8.y;
    dz = along.z - g_unk0x005914a8.z;

    if (*(int **)(pBox + 0x94) != NULL && *(int *)(pBox + 0x90) != 0) {
        p = *(int **)(pBox + 0x94);
        p[0] += dx;
        p[1] += dy;
        p[2] += dz;
        for (i = 0; i < 0x60; i += 0xc) {
            *(int *)(*(int *)(pBox + 0x90) + i) += dx;
            *(int *)(*(int *)(pBox + 0x90) + i + 4) += dy;
            *(int *)(*(int *)(pBox + 0x90) + i + 8) += dz;
        }
        p = pBox + 0xd;
        for (j = 4; j != 0; j--) {
            p[-1] += dx;
            p[0] += dy;
            p[1] += dz;
            p += 3;
        }
    }

    FUN_0048c870(*(BYTE *)(car + 0xb1a), (BYTE)0xff, (int *)&along, 1);
    g_unk0x005915dc = 0x8000;
    g_unk0x00591468 = 0x1578d;
    return 1;
}

extern char g_unk0x00590ecc[4];
extern char g_unk0x005914c4[4];
extern char g_unk0x005914d4;
extern char g_unk0x005915f4;

// The collision scratch record each car points at while a contact is resolved:
// a 0x98-byte block whose entries from index 4 on are world-space contact
// points. The pointers are set up by the caller (0x48a1f0).
// GLOBAL: CMR2 0x005915e0
FixVector *g_pContacts0x005915e0;
// GLOBAL: CMR2 0x00591394
FixVector *g_pContacts0x00591394;

struct Car;

// Minimal declaration of the game-info singleton (the real one lives in
// GameInfo.h; keeping it local avoids perturbing the line table of this file).
class CGameInfo
{
public:
    static int FUN_004063f0(int param1);
};

void Car_SpawnDebris(int size, FixVector *pPos, Car *pCar, FixVector *pAxes, int count, int glassChance);
void FUN_00466ef0(Car *pCar, int *param_2, FixVector *param_3, int param_4, unsigned char param_5, int param_6);

// Collision box of a car while a contact is resolved (0x98 bytes): the two body
// axes from 0x10, the eight contact points built by FUN_00486c30 and the two
// pointers at 0x90/0x94 to the point array and to the box vertex.
struct CollisionBox {
    BYTE pad_0x00[0x10];
    FixVector axisA;        // 0x10
    FixVector axisB;        // 0x1c
    BYTE pad_0x28[0x8];
    FixVector points[8];    // 0x30
    int *pArray;            // 0x90
    int *pVertex;           // 0x94
};

extern int FUN_00488640(int *pBoxA, int *pBoxB, FixVector *pOffset, int scale);

// Collision bookkeeping shared with the stage object collision code
// (defined in StageObjects.cpp): the per-car contact list and its length, the
// last pair result and the two objects that walk the car order.
// GLOBAL: CMR2 0x005913e0
int g_unk0x005913e0;
// GLOBAL: CMR2 0x005913f4
int g_unk0x005913f4;

extern int g_unk0x00591390;
extern BYTE g_unk0x005913dc[4];
extern BYTE g_unk0x005913f8[8][8];
extern BYTE g_unk0x00590ed0[8][0x98];
extern BYTE g_unk0x00590ec8[4];
extern BYTE g_unk0x005914a4[4];
extern int g_physicsTimeStep;

int FUN_00407270(void);
unsigned int RallyData_GetFlag24(void);
unsigned int RallyData_FUN_00407e90(void);
int FUN_00487b80(int r1, int r2, int *pA, int *pB);
void FUN_0047d850(Car *pCar, int *param_2);
void FUN_00486c30(int *pObj, int *param2, int *param3, FixVector *pVerts);
int FUN_0048a5f0(int param_1, int param_2);
void FUN_0048ae90(int param_1, int param_2);

// The four corner sector words of a car, copied as a single eight byte block.
struct CarCornerWords {
    short w[4];
};

// Tests every pair of cars of the given order for contact: pairs whose corner
// sector words share an index are run through the 2D box overlap test
// (FUN_00487b80) and, for cars that are not locked, through the box collision
// response (FUN_0048a5f0); the two cars are then linked in each other's contact
// list and the impact is resolved. Finally the separation timers of every car
// are decayed and, while the race is being verified, the collision box of each
// car is rebuilt.
// match 78%: same logic, calls and constants; MSVC6 countdowns our outer pair loop with induction variables (the original increments the counters), which also shifts the stack slots of the pair count and the pattern index, and copies the four sector words as two dwords where the original used word/dword/word.
// FUNCTION: CMR2 0x0048a1f0
void FUN_0048a1f0(int param_1, short *param_2, short param_3)
{
    short *p1;
    short *p2;
    int found;
    int i;
    int j;
    int k;
    int n;
    int m;
    int carA;
    int carB;
    CarCornerWords idsB;
    CarCornerWords idsA;
    int flags[8];

    for (i = 0; i < 8; i++)
        flags[i] = 0;
    *(int *)g_unk0x005913dc = 0;
    g_unk0x005913e0 = 0;
    n = (int)param_3 - 1;
    if (n > 0) {
        k = 1;
        p1 = param_2;
        for (; k < param_3; k++) {
            p2 = p1 + 1;
            j = param_3 - k;
            do {
                carA = param_1 + p1[0] * 0xc24;
                carB = param_1 + p2[0] * 0xc24;
                idsA = *(CarCornerWords *)(carA + 0xb00);
                idsB = *(CarCornerWords *)(carB + 0xb00);
                found = 0;
                for (i = 0; i < 4; i++) {
                    for (m = 0; m < 4; m++) {
                        if (idsA.w[m] == idsB.w[i]) {
                            found = 1;
                            if (FUN_00487b80(*(int *)(carA + 0x758), *(int *)(carB + 0x758),
                                             (int *)(carA + 0x2d0), (int *)(carB + 0x2d0))) {
                                flags[*(char *)(carB + 0xb1a)] = 1;
                                flags[*(char *)(carA + 0xb1a)] = 1;
                                if (*(int *)(carA + 0xc08) == 0 && *(int *)(carB + 0xc08) == 0 &&
                                    *(int *)(carA + 0xc0c) == 0 && *(int *)(carB + 0xc0c) == 0 &&
                                    (*(int *)(carA + 0xc18) == 0 || *(int *)(carB + 0xc18) == 0)) {
                                    g_pContacts0x005915e0 = (FixVector *)&g_unk0x00590ed0[p1[0]];
                                    g_pContacts0x00591394 = (FixVector *)&g_unk0x00590ed0[p2[0]];
                                    FUN_00486c30((int *)g_pContacts0x005915e0, (int *)(carA + 0x360),
                                                 (int *)(carA + 0x2d0), (FixVector *)(carA + 0x270));
                                    FUN_00486c30((int *)g_pContacts0x00591394, (int *)(carB + 0x360),
                                                 (int *)(carB + 0x2d0), (FixVector *)(carB + 0x270));
                                    found = FUN_0048a5f0(carA, carB);
                                    g_unk0x00591390 = 0;
                                    if (found != 0) {
                                        g_unk0x005913f8[*(char *)(carA + 0xb1a)]
                                                       [g_unk0x005913dc[*(char *)(carA + 0xb1a)]] =
                                            *(char *)(carB + 0xb1a);
                                        g_unk0x005913dc[*(char *)(carA + 0xb1a)]++;
                                        g_unk0x005913f8[*(char *)(carB + 0xb1a)]
                                                       [g_unk0x005913dc[*(char *)(carB + 0xb1a)]] =
                                            *(char *)(carA + 0xb1a);
                                        g_unk0x005913dc[*(char *)(carB + 0xb1a)]++;
                                        FUN_0048ae90(carA, carB);
                                        g_unk0x00591390 = 1;
                                    }
                                }
                            }
                            goto pair_done;
                        }
                    }
                }
            pair_done:
                g_unk0x005913f4 = found;
                p2++;
            } while (--j);
            p1++;
        }
    }
    if (n >= 0) {
        p2 = param_2 + n;
        j = n + 1;
        do {
            carA = param_1 + p2[0] * 0xc24;
            if (*(int *)(carA + 0xc08) != 0) {
                if (*(int *)(carA + 0x970) == 0) {
                    if (flags[*(char *)(carA + 0xb1a)] == 0)
                        *(int *)(carA + 0xc08) = 0;
                } else {
                    i = *(int *)(carA + 0x970) - g_physicsTimeStep;
                    *(int *)(carA + 0x970) = i;
                    if (i < 0)
                        *(int *)(carA + 0x970) = 0;
                }
                *(int *)(carA + 0x974) += FixMul(g_physicsTimeStep, 0x4000);
            } else {
                *(int *)(carA + 0x974) = 0;
            }
            p2--;
        } while (--j);
    }
    if (((BYTE)FUN_00407270() != 0 || (BYTE)RallyData_GetFlag24() != 0 ||
         (BYTE)RallyData_FUN_00407e90() != 0) &&
        CGameInfo::FUN_004063f0(0) != 0 && n >= 0) {
        p2 = param_2 + n;
        j = n + 1;
        do {
            g_pContacts0x005915e0 = (FixVector *)&g_unk0x00590ed0[p2[0]];
            carA = param_1 + p2[0] * 0xc24;
            FUN_00486c30((int *)g_pContacts0x005915e0, (int *)(carA + 0x360),
                         (int *)(carA + 0x2d0), (FixVector *)(carA + 0x270));
            FUN_0047d850((Car *)carA, (int *)g_pContacts0x005915e0);
            p2--;
        } while (--j);
    }
}

// Pushes the two cars' collision boxes apart after a contact: adds up the
// corner offsets of both quads, normalises the result into the contact normal
// and, when one of the two cars has no contact corner left, moves each box
// along its own axis by the projection of the normal; the normal is published
// at 0x5915e8 and each car's contact offset is rotated by the other box's
// offset. When both cars do have corners the boxes are handed to the
// deformation solver instead.
// FUNCTION: CMR2 0x0048a5f0
int FUN_0048a5f0(int param_1, int param_2)
{
    FixVector normal;
    FixVector sum;
    FixVector delta;
    FixVector tmp;
    int result;
    int countA;
    int countB;
    int total;
    int len;
    int dx;
    int dy;
    int dz;
    int i;

    result = FUN_00488640((int *)g_pContacts0x005915e0, (int *)g_pContacts0x00591394,
                          (FixVector *)(param_1 + 0x2e8), 0x8000);
    if (result != 0) {
    sum.x = 0;
    sum.y = 0;
    sum.z = 0;
    normal.x = 0;
    normal.y = 0;
    normal.z = 0;
    countA = 0;
    if (g_unk0x005915f4 > 0) {
        for (i = 0; i < g_unk0x005915f4; i++) {
            delta.x = *(int *)(param_1 + (g_unk0x005914c4[i] * 3 + 0x9c) * 4) -
                      *(int *)(param_1 + 0x2d0);
            delta.y = *(int *)(param_1 + g_unk0x005914c4[i] * 0xc + 0x274) -
                      *(int *)(param_1 + 0x2d4);
            delta.z = *(int *)(param_1 + g_unk0x005914c4[i] * 0xc + 0x278) -
                      *(int *)(param_1 + 0x2d8);
            delta.y = 0;
            sum.x += delta.x;
            sum.z += delta.z;
            if (g_unk0x00590ec8[i] == 0) {
                if (FixVecDot(&normal, &((CollisionBox *)g_pContacts0x00591394)->axisA) < 0) {
                    normal.x -= ((CollisionBox *)g_pContacts0x00591394)->axisA.x;
                    normal.y -= ((CollisionBox *)g_pContacts0x00591394)->axisA.y;
                    normal.z -= ((CollisionBox *)g_pContacts0x00591394)->axisA.z;
                } else {
                    normal.x += ((CollisionBox *)g_pContacts0x00591394)->axisA.x;
                    normal.y += ((CollisionBox *)g_pContacts0x00591394)->axisA.y;
                    normal.z += ((CollisionBox *)g_pContacts0x00591394)->axisA.z;
                }
            } else {
                if (FixVecDot(&normal, &((CollisionBox *)g_pContacts0x00591394)->axisB) < 0) {
                    normal.x -= ((CollisionBox *)g_pContacts0x00591394)->axisB.x;
                    normal.y -= ((CollisionBox *)g_pContacts0x00591394)->axisB.y;
                    normal.z -= ((CollisionBox *)g_pContacts0x00591394)->axisB.z;
                } else {
                    normal.x += ((CollisionBox *)g_pContacts0x00591394)->axisB.x;
                    normal.y += ((CollisionBox *)g_pContacts0x00591394)->axisB.y;
                    normal.z += ((CollisionBox *)g_pContacts0x00591394)->axisB.z;
                }
            }
        }
        countA = g_unk0x005915f4;
    }
    countB = 0;
    if (g_unk0x005914d4 > 0) {
        for (i = 0; i < g_unk0x005914d4; i++) {
            delta.x = *(int *)(param_2 + (g_unk0x00590ecc[i] * 3 + 0x9c) * 4) -
                      *(int *)(param_1 + 0x2d0);
            delta.y = *(int *)(param_2 + g_unk0x00590ecc[i] * 0xc + 0x274) -
                      *(int *)(param_1 + 0x2d4);
            delta.z = *(int *)(param_2 + g_unk0x00590ecc[i] * 0xc + 0x278) -
                      *(int *)(param_1 + 0x2d8);
            delta.y = 0;
            sum.x += delta.x;
            sum.z += delta.z;
            if (g_unk0x005914a4[i] == 0) {
                if (FixVecDot(&normal, &((CollisionBox *)g_pContacts0x005915e0)->axisA) < 0) {
                    normal.x -= ((CollisionBox *)g_pContacts0x005915e0)->axisA.x;
                    normal.y -= ((CollisionBox *)g_pContacts0x005915e0)->axisA.y;
                    normal.z -= ((CollisionBox *)g_pContacts0x005915e0)->axisA.z;
                } else {
                    normal.x += ((CollisionBox *)g_pContacts0x005915e0)->axisA.x;
                    normal.y += ((CollisionBox *)g_pContacts0x005915e0)->axisA.y;
                    normal.z += ((CollisionBox *)g_pContacts0x005915e0)->axisA.z;
                }
            } else {
                if (FixVecDot(&normal, &((CollisionBox *)g_pContacts0x005915e0)->axisB) < 0) {
                    normal.x -= ((CollisionBox *)g_pContacts0x005915e0)->axisB.x;
                    normal.y -= ((CollisionBox *)g_pContacts0x005915e0)->axisB.y;
                    normal.z -= ((CollisionBox *)g_pContacts0x005915e0)->axisB.z;
                } else {
                    normal.x += ((CollisionBox *)g_pContacts0x005915e0)->axisB.x;
                    normal.y += ((CollisionBox *)g_pContacts0x005915e0)->axisB.y;
                    normal.z += ((CollisionBox *)g_pContacts0x005915e0)->axisB.z;
                }
            }
        }
        countB = g_unk0x005914d4;
    }
    total = (countA + countB) << 16;
    FixVecScaleRecip(&sum, &sum, total);
    len = FixVecLength(&normal);
    if (len == 0) {
        normal.x = 0;
        normal.y = 0;
        normal.z = 0;
    } else {
        FixVecScaleRecip(&normal, &normal, len);
    }
    if (g_unk0x005915f4 == 0 || g_unk0x005914d4 == 0) {
        FixVecScale(&delta, &normal, FixVecDot(&g_unk0x005914a8, &normal));
        dx = delta.x - g_unk0x005914a8.x;
        dy = delta.y - g_unk0x005914a8.y;
        dz = delta.z - g_unk0x005914a8.z;
        if (((CollisionBox *)g_pContacts0x005915e0)->pVertex != NULL &&
            ((CollisionBox *)g_pContacts0x005915e0)->pArray != NULL) {
            ((CollisionBox *)g_pContacts0x005915e0)->pVertex[0] += dx;
            ((CollisionBox *)g_pContacts0x005915e0)->pVertex[1] += dy;
            ((CollisionBox *)g_pContacts0x005915e0)->pVertex[2] += dz;
            for (i = 0; i < 0x60; i += 0xc) {
                *(int *)((BYTE *)((CollisionBox *)g_pContacts0x005915e0)->pArray + i) += dx;
                *(int *)((BYTE *)((CollisionBox *)g_pContacts0x005915e0)->pArray + i + 4) += dy;
                *(int *)((BYTE *)((CollisionBox *)g_pContacts0x005915e0)->pArray + i + 8) += dz;
            }
            for (i = 0; i < 0x30; i += 0xc) {
                *(int *)((BYTE *)g_pContacts0x005915e0 + i + 0x30) += dx;
                *(int *)((BYTE *)g_pContacts0x005915e0 + i + 0x34) += dy;
                *(int *)((BYTE *)g_pContacts0x005915e0 + i + 0x38) += dz;
            }
        }
        FUN_0048c870(*(char *)(param_1 + 0xb1a), *(char *)(param_2 + 0xb1a), (int *)&delta, 1);
        FixVecScale(&delta, &normal, FixVecDot(&g_unk0x005914b8, &normal));
        dx = delta.x - g_unk0x005914b8.x;
        dy = delta.y - g_unk0x005914b8.y;
        dz = delta.z - g_unk0x005914b8.z;
        if (((CollisionBox *)g_pContacts0x00591394)->pVertex != NULL &&
            ((CollisionBox *)g_pContacts0x00591394)->pArray != NULL) {
            ((CollisionBox *)g_pContacts0x00591394)->pVertex[0] += dx;
            ((CollisionBox *)g_pContacts0x00591394)->pVertex[1] += dy;
            ((CollisionBox *)g_pContacts0x00591394)->pVertex[2] += dz;
            for (i = 0; i < 0x60; i += 0xc) {
                *(int *)((BYTE *)((CollisionBox *)g_pContacts0x00591394)->pArray + i) += dx;
                *(int *)((BYTE *)((CollisionBox *)g_pContacts0x00591394)->pArray + i + 4) += dy;
                *(int *)((BYTE *)((CollisionBox *)g_pContacts0x00591394)->pArray + i + 8) += dz;
            }
            for (i = 0; i < 0x30; i += 0xc) {
                *(int *)((BYTE *)g_pContacts0x00591394 + i + 0x30) += dx;
                *(int *)((BYTE *)g_pContacts0x00591394 + i + 0x34) += dy;
                *(int *)((BYTE *)g_pContacts0x00591394 + i + 0x38) += dz;
            }
        }
        FUN_0048c870(*(char *)(param_2 + 0xb1a), *(char *)(param_1 + 0xb1a), (int *)&delta, 1);
    }
    g_unk0x005915e8.x = normal.x;
    g_unk0x005915e8.y = normal.y;
    g_unk0x005915e8.z = normal.z;
    FixMatrix_InverseRotateVector((FixVector *)(param_1 + 0x5dc), &sum,
                                  *(FixMatrix **)(param_1 + 0x750));
    sum.x += *(int *)(*(int *)((BYTE *)g_pContacts0x005915e0 + 0x94));
    sum.y += *(int *)(*(int *)((BYTE *)g_pContacts0x005915e0 + 0x94) + 4);
    sum.z += *(int *)(*(int *)((BYTE *)g_pContacts0x005915e0 + 0x94) + 8);
    tmp.x = sum.x - *(int *)(*(int *)((BYTE *)g_pContacts0x00591394 + 0x94));
    tmp.y = sum.y - *(int *)(*(int *)((BYTE *)g_pContacts0x00591394 + 0x94) + 4);
    tmp.z = sum.z - *(int *)(*(int *)((BYTE *)g_pContacts0x00591394 + 0x94) + 8);
    FixMatrix_InverseRotateVector((FixVector *)(param_2 + 0x5dc), &tmp,
                                  *(FixMatrix **)(param_2 + 0x750));
    }
    return result;
}

// Resolves a collision between two cars. Clamps each car's contact offset (the
// vector at +0x5dc) by its half extents, builds the world-space velocity of
// that offset out of the angular velocity at that point plus the body velocity,
// and projects both on the collision normal 0x5915e8. The closing speed along
// the normal, weighted by each body's mass (0x75c), gives the common rebound
// velocity; the difference to each car is the impulse it takes along the
// normal, which is turned into a linear correction (0x5c4) and, crossed with
// the contact offset, into an angular one (0x5d0). Cars that are locked
// (0xc00) get a four-times stronger offset correction and are marked at 0xc00
// while separated. When the two do not actually touch, the impulse along the
// contact axis is passed to the body-deformation solver (0x466ef0); otherwise
// the tangential relative motion is normalized and debris is thrown along it.
// match 44%: reviewed (W172) - calls, constants and branch logic match; only the
// stack-slot allocation and the register numbering of the FPU-dense blocks differ
// (see CONOCIMIENTO 4.t).
// FUNCTION: CMR2 0x0048ae90
void FUN_0048ae90(int param_1, int param_2)
{
    FixVector tmp;
    FixVector vA;
    FixVector vB;
    FixVector dv;
    FixVector sep;
    FixVector axis;
    FixVector impA;
    FixVector impB;
    FixVector *pContact;
    int dotA;
    int dotB;
    int num;
    int x;
    int dA;
    int dB;
    int len;
    int len2;
    int size;
    int bSepA;
    int bSepB;

    FUN_0048c6e0((int *)(param_1 + 0x5dc), (int *)(param_1 + 0x204), 0);
    FUN_0048c6e0((int *)(param_2 + 0x5dc), (int *)(param_2 + 0x204), 0);

    FixVecCross(&tmp, (FixVector *)(param_1 + 0x420), (FixVector *)(param_1 + 0x5dc));
    FixMatrix_RotateVector(&vA, &tmp, *(FixMatrix **)(param_1 + 0x750));
    vA.x += *(int *)(param_1 + 0x408);
    vA.y += *(int *)(param_1 + 0x40c);
    vA.z += *(int *)(param_1 + 0x410);

    FixVecCross(&tmp, (FixVector *)(param_2 + 0x420), (FixVector *)(param_2 + 0x5dc));
    FixMatrix_RotateVector(&vB, &tmp, *(FixMatrix **)(param_2 + 0x750));
    vB.x += *(int *)(param_2 + 0x408);
    vB.y += *(int *)(param_2 + 0x40c);
    vB.z += *(int *)(param_2 + 0x410);

    dotA = FixVecDot(&g_unk0x005915e8, &vA);
    dotB = FixVecDot(&g_unk0x005915e8, &vB);
    num = FixMul(dotA, *(int *)(param_1 + 0x75c)) + FixMul(dotB, *(int *)(param_2 + 0x75c));
    x = FixDiv(num, *(int *)(param_1 + 0x75c) + *(int *)(param_2 + 0x75c));
    dA = x - dotA;
    dB = x - dotB;

    impA.x = FixMul(g_unk0x005915e8.x, dA);
    impA.y = FixMul(g_unk0x005915e8.y, dA);
    impA.z = FixMul(g_unk0x005915e8.z, dA);

    bSepA = 0;
    if (*(int *)(param_1 + 0xc00) == 0) {
        if (FIX_ABS(dA) > 0x9999 &&
            !(*(int *)(param_1 + 0xb64) != 0 && *(int *)(param_2 + 0xb64) != 0)) {
            bSepA = 1;
            tmp.x = FixMul(-*(int *)(param_1 + 0x364), 0x40000);
            tmp.y = FixMul(-*(int *)(param_1 + 0x370), 0x40000);
            tmp.z = FixMul(-*(int *)(param_1 + 0x37c), 0x40000);
            *(int *)(param_1 + 0x5dc) += tmp.x;
            *(int *)(param_1 + 0x5e0) += tmp.y;
            *(int *)(param_1 + 0x5e4) += tmp.z;
        }
    }

    FixVecScale(&impB, &g_unk0x005915e8, dB);

    bSepB = 0;
    if (*(int *)(param_2 + 0xc00) == 0) {
        if (FIX_ABS(dB) > 0x9999 &&
            !(*(int *)(param_1 + 0xb64) != 0 && *(int *)(param_2 + 0xb64) != 0)) {
            bSepB = 1;
            tmp.x = FixMul(-*(int *)(param_2 + 0x364), 0x40000);
            tmp.y = FixMul(-*(int *)(param_2 + 0x370), 0x40000);
            tmp.z = FixMul(-*(int *)(param_2 + 0x37c), 0x40000);
            *(int *)(param_2 + 0x5dc) += tmp.x;
            *(int *)(param_2 + 0x5e0) += tmp.y;
            *(int *)(param_2 + 0x5e4) += tmp.z;
        }
    }

    if (*(char *)(param_1 + 0xb35) != 1 && impA.y < 0) {
        int oldY = impA.y;
        impA.y = 0;
        impB.y -= oldY;
    }
    if (*(char *)(param_2 + 0xb35) != 1 && impB.y < 0) {
        int oldY = impB.y;
        impB.y = 0;
        impA.y -= oldY;
    }

    FixVecScale(&impA, &impA, g_physicsScale);
    FixMatrix_InverseRotateVector(&tmp, &impA, *(FixMatrix **)(param_1 + 0x750));
    *(int *)(param_1 + 0x5c4) += impA.x;
    *(int *)(param_1 + 0x5c8) += impA.y;
    *(int *)(param_1 + 0x5cc) += impA.z;

    FixVecCross(&axis, &tmp, (FixVector *)(param_1 + 0x5dc));
    if (bSepA) {
        FixVecScale(&axis, &axis, 0x40000);
        *(int *)(param_1 + 0xc00) = 1;
        *(int *)(param_1 + 0x96c) = 0x10000;
    } else {
        FixVecScale(&axis, &axis, 0x20000);
    }
    *(int *)(param_1 + 0x5d0) += axis.x;
    *(int *)(param_1 + 0x5d4) += axis.y;
    *(int *)(param_1 + 0x5d8) += axis.z;
    FUN_0048c750((int *)(param_1 + 0x5d0));

    FixVecScale(&impB, &impB, g_physicsScale);
    FixMatrix_InverseRotateVector(&tmp, &impB, *(FixMatrix **)(param_2 + 0x750));
    *(int *)(param_2 + 0x5c4) += impB.x;
    *(int *)(param_2 + 0x5c8) += impB.y;
    *(int *)(param_2 + 0x5cc) += impB.z;

    FixVecCross(&axis, &tmp, (FixVector *)(param_2 + 0x5dc));
    if (bSepB) {
        FixVecScale(&axis, &axis, 0x40000);
        *(int *)(param_2 + 0xc00) = 1;
        *(int *)(param_2 + 0x96c) = 0x10000;
    } else {
        FixVecScale(&axis, &axis, 0x20000);
    }
    *(int *)(param_2 + 0x5d0) += axis.x;
    *(int *)(param_2 + 0x5d4) += axis.y;
    *(int *)(param_2 + 0x5d8) += axis.z;
    FUN_0048c750((int *)(param_2 + 0x5d0));

    if (g_unk0x005915f4 != 0) {
        pContact = g_pContacts0x005915e0 + (g_unk0x005914c4[0] + 4);
        FUN_00466ef0((Car *)param_1, (int *)pContact, &g_unk0x005915e8, 0, 0, 0);
        FUN_00466ef0((Car *)param_2, (int *)pContact, &g_unk0x005915e8, 0, 2, 0);
    } else if (g_unk0x005914d4 != 0) {
        pContact = g_pContacts0x00591394 + (g_unk0x00590ecc[0] + 4);
        FUN_00466ef0((Car *)param_1, (int *)pContact, &g_unk0x005915e8, 0, 2, 0);
        FUN_00466ef0((Car *)param_2, (int *)pContact, &g_unk0x005915e8, 0, 0, 0);
    }

    dv.x = vA.x - vB.x;
    dv.y = vA.y - vB.y;
    dv.z = vA.z - vB.z;
    {
        int d = FixVecDot(&g_unk0x005915e8, &dv);
        sep.x = dv.x - FixMul(g_unk0x005915e8.x, d);
        sep.y = dv.y - FixMul(g_unk0x005915e8.y, d);
        sep.z = dv.z - FixMul(g_unk0x005915e8.z, d);
    }

    if (CGameInfo::FUN_004063f0(4) != 0) {
        *(int *)(param_1 + 0x5c4) -= impA.x;
        *(int *)(param_1 + 0x5c8) -= impA.y;
        *(int *)(param_1 + 0x5cc) -= impA.z;
        *(int *)(param_1 + 0x408) += impA.x * 2;
        *(int *)(param_1 + 0x40c) += impA.y * 2;
        *(int *)(param_1 + 0x410) += impA.z * 2;
        *(int *)(param_2 + 0x5c4) -= impB.x;
        *(int *)(param_2 + 0x5c8) -= impB.y;
        *(int *)(param_2 + 0x5cc) -= impB.z;
        *(int *)(param_2 + 0x408) += impB.x * 2;
        *(int *)(param_2 + 0x40c) += impB.y * 2;
        *(int *)(param_2 + 0x410) += impB.z * 2;
    }

    len = FixVecLength(&sep);
    if (len > 0x3333) {
        FixVecScaleRecip(&sep, &sep, -len);
        FixVecCross(&axis, &sep, &g_unk0x005915e8);
        len2 = FixVecLength(&axis);
        if (len2 == 0) {
            axis.x = 0;
            axis.y = 0;
            axis.z = 0;
        } else {
            FixVecScaleRecip(&axis, &axis, len2);
        }

        if (g_unk0x005915f4 != 0) {
            pContact = g_pContacts0x005915e0 + (g_unk0x005914c4[0] + 4);
            impA.x = pContact->x - *(int *)(param_1 + 0x2d0);
            impA.y = pContact->y - *(int *)(param_1 + 0x2d4);
            impA.z = pContact->z - *(int *)(param_1 + 0x2d8);
            impB.x = pContact->x - *(int *)(param_2 + 0x2d0);
            impB.y = pContact->y - *(int *)(param_2 + 0x2d4);
            impB.z = pContact->z - *(int *)(param_2 + 0x2d8);
        } else if (g_unk0x005914d4 != 0) {
            pContact = g_pContacts0x00591394 + (g_unk0x00590ecc[0] + 4);
            impA.x = pContact->x - *(int *)(param_1 + 0x2d0);
            impA.y = pContact->y - *(int *)(param_1 + 0x2d4);
            impA.z = pContact->z - *(int *)(param_1 + 0x2d8);
            impB.x = pContact->x - *(int *)(param_2 + 0x2d0);
            impB.y = pContact->y - *(int *)(param_2 + 0x2d4);
            impB.z = pContact->z - *(int *)(param_2 + 0x2d8);
        }

        impA.y = 0;
        FixVecScale(&impA, &impA, 0xcccc);
        impB.y = 0;
        FixVecScale(&impB, &impB, 0xcccc);

        size = FixMul(len, 0x20000);
        if (size > 0x10000)
            size = 0x10000;

        if (*(int *)(param_1 + 0xb70) == 0 && *(int *)(param_2 + 0xb70) == 0) {
            Car_SpawnDebris(size, &impA, (Car *)param_1, &sep, 0x90000, 0x6666);
            Car_SpawnDebris(size, &impB, (Car *)param_2, &sep, 0x90000, 0x6666);
        }
    }
}

// Bumped by 0x4878a0 while a car is scraping along a wall.
// GLOBAL: CMR2 0x005914d8
int g_unk0x005914d8;
// Corner copy the wall collision keeps for the other body of the contact:
// the corners of the object box at 0x5915f8 (StageObjects.cpp), from +0x30.
extern int g_unk0x005915f8[0x26];
#define g_unk0x00591628 ((FixVector *)&g_unk0x005915f8[0xc])

// Slides one car along a static obstacle. Zeroes the vertical contact offset,
// clamps the offset by the half extents (including y this time) and turns the
// angular velocity at that point into the world-space velocity of the contact.
// The normal speed 0x5915e8 . v is compared against the two thresholds the wall
// solver left in 0x591468 / 0x5915dc to decide whether the impact is strong
// enough to separate the car (and lock it for a frame) or only to bounce it.
// The contact object of the obstacle may ask for a damped impulse (0x2001000),
// which scales the rebound by 0x28f. The impulse along the normal is rotated
// into body space and added to the linear correction 0x5c4, and its moment
// about the contact offset 0x5d0 is built either from the (scaled) relative
// velocity cross the offset, or from a vertical axis offset by the contact
// point when the car is already separating -- in the latter case a strong
// impact also raises the pitch velocity 0x40c. The contact slot is recorded in
// 0xae0 (up to five per step) and the deformation solver gets the contact
// point when it is new. Returns whether the impact was damped.
// match 40%: reviewed (W172) - calls, constants and branch logic match; the
// diff is the FPU-dense block scheduling and the reused stack slots of the
// original (CONOCIMIENTO 4.t).
// FUNCTION: CMR2 0x0048be20
int FUN_0048be20(int param_1, int *param_2, int param_3, int param_4)
{
    FixVector tmp;
    FixVector vA;
    FixVector imp;
    FixVector off;
    FixVector spin;
    FixVector vPos;
    FixVector vAxis;
    int flagA;
    int flagB;
    int flagC;
    int impulse;
    int dot;
    int found;
    int i;
    short *pSlot;

    flagA = 0;
    flagB = 0;
    flagC = 0;
    *(int *)(param_1 + 0x5e0) = 0;
    FUN_0048c6e0((int *)(param_1 + 0x5dc), (int *)(param_1 + 0x204), 1);

    FixVecCross(&tmp, (FixVector *)(param_1 + 0x420), (FixVector *)(param_1 + 0x5dc));
    FixMatrix_RotateVector(&vA, &tmp, *(FixMatrix **)(param_1 + 0x750));
    vA.x += *(int *)(param_1 + 0x408);
    vA.y += *(int *)(param_1 + 0x40c);
    vA.z += *(int *)(param_1 + 0x410);

    if (*(int *)(param_1 + 0xb64) == 0 && *(int *)(param_1 + 0xc00) == 0) {
        dot = FixVecDot(&g_unk0x005915e8, &vA);
        if (dot < 0)
            dot = -dot;
        if (g_unk0x00591468 < dot) {
            flagA = 1;
            flagB = 1;
        } else if (g_unk0x005915dc < dot) {
            flagA = 1;
            flagB = 0;
        }
    }

    impulse = -FixVecDot(&g_unk0x005915e8, &vA);
    if ((*(unsigned int *)(*param_2 + 0x10) & 0x2001000) != 0 && impulse != 0) {
        flagC = 1;
        flagB = 0;
        impulse = FixMul(impulse, 0x28f);
    }

    FixVecScale(&imp, &g_unk0x005915e8, impulse);
    FixVecScale(&imp, &imp, g_physicsScale);

    FixMatrix_InverseRotateVector(&tmp, &imp, *(FixMatrix **)(param_1 + 0x750));
    *(int *)(param_1 + 0x5c4) += imp.x;
    *(int *)(param_1 + 0x5c8) += imp.y;
    *(int *)(param_1 + 0x5cc) += imp.z;

    found = 0;
    if (*(char *)(param_1 + 0xb3e) > 0) {
        pSlot = (short *)(param_1 + 0xad6);
        i = 0;
        do {
            if (*pSlot == param_4) {
                found = 1;
                break;
            }
            i++;
            pSlot++;
        } while (i < *(char *)(param_1 + 0xb3e));
    }
    if (!found) {
        if (g_unk0x005914d8 == 0) {
            FUN_00466ef0((Car *)param_1, (int *)&g_unk0x00591498, &g_unk0x005915e8,
                         g_unk0x00591490, 1, 0);
        } else if (g_unk0x005915f4 != 0) {
            FUN_00466ef0((Car *)param_1,
                         (int *)(param_1 + (g_unk0x005914c4[0] + 0x34) * 0xc),
                         &g_unk0x005915e8, 0, 0, 0);
        } else if (g_unk0x005914d4 != 0) {
            FUN_00466ef0((Car *)param_1,
                         (int *)&g_unk0x00591628[g_unk0x00590ecc[0]],
                         &g_unk0x005915e8, 0, 2, 0);
        }
    }

    FixMatrix_RotateVector(&vPos, (FixVector *)(param_1 + 0x5dc), *(FixMatrix **)(param_1 + 0x750));
    vPos.x += *(int *)(param_1 + 0x2d0);
    vPos.y += *(int *)(param_1 + 0x2d4);
    vPos.z += *(int *)(param_1 + 0x2d8);

    if (flagC == 0 && flagA != 0) {
        *(int *)(param_1 + 0x96c) = 0x10000;
        *(int *)(param_1 + 0xc00) = 1;
        *(int *)(param_1 + 0xc04) = 1;
    }
    *(int *)(param_1 + 0x5e0) = 0;

    if (flagA == 0) {
        FixVecCross(&spin, &tmp, (FixVector *)(param_1 + 0x5dc));
        if (g_unk0x005914d8 == 0) {
            FixVecScale(&spin, &spin, 0x30000);
        } else {
            spin.x = FixMul(spin.x, 0x18000);
            spin.y = FixMul(spin.y, 0x18000);
            spin.z = FixMul(spin.z, 0x18000);
        }
    } else if (flagB != 0) {
        vAxis.x = FixMul(-*(int *)(param_1 + 0x364), 0x40000);
        vAxis.y = FixMul(-*(int *)(param_1 + 0x370), 0x40000);
        vAxis.z = FixMul(-*(int *)(param_1 + 0x37c), 0x40000);
        off.x = vAxis.x + *(int *)(param_1 + 0x5dc);
        off.y = vAxis.y + *(int *)(param_1 + 0x5e0);
        off.z = vAxis.z + *(int *)(param_1 + 0x5e4);
        FixVecCross(&spin, &tmp, &off);
        FixVecScale(&spin, &spin, 0x40000);
        *(int *)(param_1 + 0x40c) += 0x4000;
    } else {
        vAxis.x = FixMul(-*(int *)(param_1 + 0x364), 0x10000);
        vAxis.y = FixMul(-*(int *)(param_1 + 0x370), 0x10000);
        vAxis.z = FixMul(-*(int *)(param_1 + 0x37c), 0x10000);
        off.x = vAxis.x + *(int *)(param_1 + 0x5dc);
        off.y = vAxis.y + *(int *)(param_1 + 0x5e0);
        off.z = vAxis.z + *(int *)(param_1 + 0x5e4);
        FixVecCross(&spin, &tmp, &off);
        spin.x = FixMul(spin.x, 0x40000);
        spin.y = FixMul(spin.y, 0x40000);
        spin.z = FixMul(spin.z, 0x40000);
    }
    *(int *)(param_1 + 0x5d0) += spin.x;
    *(int *)(param_1 + 0x5d4) += spin.y;
    *(int *)(param_1 + 0x5d8) += spin.z;
    FUN_0048c750((int *)(param_1 + 0x5d0));

    if (*(char *)(param_1 + 0xb3f) < 5) {
        *(short *)(param_1 + 0xae0 + *(char *)(param_1 + 0xb3f) * 2) = (short)param_4;
        *(char *)(param_1 + 0xb3f) = *(char *)(param_1 + 0xb3f) + 1;
    }

    if (CGameInfo::FUN_004063f0(4) != 0) {
        *(int *)(param_1 + 0x5c4) -= imp.x;
        *(int *)(param_1 + 0x5c8) -= imp.y;
        *(int *)(param_1 + 0x5cc) -= imp.z;
        *(int *)(param_1 + 0x408) += imp.x * 2;
        *(int *)(param_1 + 0x40c) += imp.y * 2;
        *(int *)(param_1 + 0x410) += imp.z * 2;
    }
    return flagC;
}
