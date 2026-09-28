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

        if ((int)uVar6 < 1)
            uVar15 = (unsigned int)((0 < (int)uVar7) + 2);
        else
            uVar15 = (unsigned int)(0 < (int)uVar7);
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
            if (iVar5 < 0x42) {
                if (iVar5 < -0x41) {
                    uVar15 = (unsigned int)-(iVar13 + uVar7);
                    tmp = iVar13 + uVar7;
                    if (-1 < (int)uVar15)
                        tmp = (int)uVar15;
                    local_14 = FixDiv(tmp, -iVar5);
                    bVar16 = 1;
                }
            } else {
                uVar15 = (unsigned int)(iVar13 - uVar7);
                local_14 = FixDiv((int)uVar15, iVar5);
                bVar16 = 1;
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
