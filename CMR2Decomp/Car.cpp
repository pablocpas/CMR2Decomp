#include <windows.h>
#include "Graphics.h"
#include "Car.h"
#include "FileBuffer.h"
#include "Game.h"
#include "GameInfo.h"
#include "RallyData.h"
#include "Mesh.h"
#include "main.h"
#include <string.h>

Car *g_cars[40];
int g_carCount;
Car *g_carBuffer;
Car *g_pCurrentCar;
CarTransforms g_carTransforms[8];
FixMatrix g_carWheelTransforms[8][4];
short g_carOrderCount;
short g_carOrder[48];
int g_carViewScale[15][2];
SceneNode *g_viewNodes[5];
// GLOBAL: CMR2 0x00538e40
BYTE g_unk0x00538e40[0xc0];
// GLOBAL: CMR2 0x00538e04
int g_unk0x00538e04[2];
// GLOBAL: CMR2 0x00538e0c
BYTE g_unk0x00538e0c[4];
// GLOBAL: CMR2 0x00538d2c
BYTE g_unk0x00538d2c[0xcc];
// View of the final eight bytes of the camera records at 0x00538df0.
#define g_unk0x00538df0 ((int *)(g_unk0x00538d2c + 0xc4))
// GLOBAL: CMR2 0x00538df8
short g_unk0x00538df8[2];
// GLOBAL: CMR2 0x00538c98
int g_unk0x00538c98[2];
// GLOBAL: CMR2 0x00538f00
int g_unk0x00538f00[2];
// View camera records: 2 per player (the player's two view modes), 100 bytes each.
// GLOBAL: CMR2 0x00539018
BYTE g_viewRecords[4][100];
#define g_unk0x0053901a (g_viewRecords[0] + 2)
#define g_unk0x0053901c (g_viewRecords[0] + 4)
// GLOBAL: CMR2 0x00538ca0
FixMatrix g_unk0x00538ca0[2];

// Difference between the two stored view positions for one car slot.
// FUNCTION: CMR2 0x004220d0
void Car_GetViewPositionDelta(FixVector *pOut, unsigned int view)
{
    FixVector a;
    FixVector b;
    int offset = (view & 0xff) * 100;

    FixMatrix_GetPosition(&b, (FixMatrix *)(g_unk0x00538e40 + offset));
    FixMatrix_GetPosition(&a, (FixMatrix *)(g_unk0x00538d2c + 4 + offset));
    pOut->x = a.x - b.x;
    pOut->y = a.y - b.y;
    pOut->z = a.z - b.z;
}

// FUNCTION: CMR2 0x00422f90
void FUN_00422f90(unsigned int index, int value)
{
    g_unk0x00538e04[index & 0xff] = value;
}

// FUNCTION: CMR2 0x00422f50
int FUN_00422f50(BYTE index)
{
    if (*(int *)(g_unk0x00538d2c + index * 100) != 8)
        return *(int *)(g_unk0x0053901c + ((unsigned int)g_unk0x00538e0c[index] + index * 2) * 100);
    return 8;
}

// FUNCTION: CMR2 0x00422fb0
BYTE FUN_00422fb0(BYTE index)
{
    return g_unk0x0053901a[((unsigned int)g_unk0x00538e0c[index] + index * 2) * 100];
}

// match 66%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00423d70
FixMatrix *FUN_00423d70(unsigned int index)
{
    BYTE state = (BYTE)RallyDataState();
    if ((BYTE)index < state)
        return (FixMatrix *)((BYTE *)g_unk0x00538ca0 + (index & 0xff) * 0x40);
    index &= 0xff;
    return Car_Get(index)->pWorld;
}

// Release callback of Car_AllocateTable.
// FUNCTION: CMR2 0x00428790
int Car_FreeTable(void)
{
    if (g_carBuffer != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_carBuffer);
        g_carBuffer = NULL;
    }
    g_carCount = 0;
    return 1;
}

// Allocates one Car per slot and registers the 0x428790 callback.
// FUNCTION: CMR2 0x004287c0
void Car_AllocateTable(int count)
{
    Car *pBuffer;
    int i;

    pBuffer = (Car *)CFileBuffer::AllocateLockedBuffer(count * sizeof(Car));
    g_carBuffer = pBuffer;
    g_carCount = count;
    for (i = 0; i < count; i++) {
        g_cars[i] = pBuffer;
        pBuffer++;
    }
    CGame::RegisterCallback(Car_FreeTable, NULL);
}

unsigned int RallyData_FUN_00407e70(void);
BYTE *FUN_0041b390(void);

// Marks which player camera records are too far from, or face away from, the
// car. The fourth word is set when every active camera has been rejected.
// FUNCTION: CMR2 0x00428a00
void FUN_00428a00(Car *pCar)
{
    int *pRejected;
    FixMatrix *pView;
    FixVector position;
    FixVector forward;
    FixVector delta;
    unsigned int count;
    unsigned int rejected;
    unsigned int i;

    pRejected = (int *)((BYTE *)pCar + 0xb68);
    pRejected[0] = 0;
    pRejected[1] = 0;
    pRejected[2] = 0;
    if ((char)RallyData_FUN_00407e70() != 0 && **(char **)(FUN_0041b390() + 4) != '\n' &&
        **(char **)(FUN_0041b390() + 4) != '\t' && **(char **)(FUN_0041b390() + 4) != '\r') {
        rejected = 0;
        count = RallyDataState() & 0xff;
        pView = (FixMatrix *)(g_unk0x00538d2c + 4);
        for (i = 0; i < count; i++, pView = (FixMatrix *)((BYTE *)pView + 100)) {
            FixMatrix_GetPosition(&position, pView);
            FixMatrix_GetForward(&forward, pView);
            if (*pRejected == 0) {
                delta.x = pCar->position.x - position.x;
                delta.y = pCar->position.y - position.y;
                delta.z = pCar->position.z - position.z;
                if (FIX_ABS(delta.x) > 0x3c0000 || FIX_ABS(delta.y) > 0x3c0000 ||
                    FIX_ABS(delta.z) > 0x3c0000 ||
                    FixMul(delta.x, delta.x) + FixMul(delta.y, delta.y) +
                            FixMul(delta.z, delta.z) >
                        0xe100000 ||
                    FixMul(forward.x, delta.x) + FixMul(forward.y, delta.y) +
                            FixMul(forward.z, delta.z) <
                        0)
                    *pRejected = 1;
            }
            if (*pRejected != 0)
                rejected++;
            pRejected++;
        }
        if (rejected == count)
            *(int *)((BYTE *)pCar + 0xb70) = 1;
    }
}

unsigned int RallyData_FUN_00407e70(void);
int FUN_004054b0(unsigned int param1);
void FUN_0046b760(int index, int reset);
extern float g_65536f;

// Picks the up to three cars to display in one split-screen view (nearest to
// the view position, in or near the viewport) and hides the remaining ones.
// match 59%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00428bf0
void FUN_00428bf0(unsigned int view, short *pRect)
{
    char visible[8];
    int i;
    int j;
    int k;
    int car;
    int bestIdx;
    int bestDist;
    SceneNode *pView;
    FixMatrix *pCurrent;
    FixVector pos;
    FixVector otherPos;
    FixVector delta;
    FixVector up;
    int distTable[8];
    FixMatrix savedWorld;
    int proj1[2];
    int proj2[2];
    int ix;
    int iy;
    int xx;
    int yy;
    int extend;
    float ftmp;
    int sx;
    int sy;

    if (CGameInfo::FUN_00406410(0x10))
        return;
    if ((char)RallyData_FUN_00407e70() && RallyData_FUN_00411880() != 0) {
        FUN_0046b760(0, 1);
        FUN_0046b760(1, 1);
        return;
    }
    if (!((char)RallyData_FUN_00407e70() || CGameInfo::FUN_00405d80() == 8 || CGameInfo::FUN_00405d80() == 9 ||
          CGameInfo::FUN_00405d80() == 10 || CGameInfo::FUN_00405d80() == 11 ||
          CGameInfo::FUN_00405d80() == 12 || CGameInfo::FUN_00405d80() == 8))
        return;

    i = g_carOrderCount;
    if (i > 0) {
        short *p = g_carOrder;
        do {
            visible[*p++] = 0;
        } while (--i);
    }

    pView = g_viewNodes[view & 0xff];
    pCurrent = &pView->current;
    FixMatrix_GetPosition(&pos, pCurrent);

    i = 0;
    if (g_carOrderCount > 0) {
        do {
            FixMatrix_GetPosition(&otherPos, &Car_Get(g_carOrder[i])->pNode0x71c->current);
            delta.x = pos.x - otherPos.x;
            delta.y = pos.y - otherPos.y;
            delta.z = pos.z - otherPos.z;
            if (FIX_ABS(delta.x) <= 0x960000 && FIX_ABS(delta.y) <= 0x960000 && FIX_ABS(delta.z) <= 0x960000)
                distTable[g_carOrder[i]] = FixMul(delta.x, delta.x) + FixMul(delta.y, delta.y) + FixMul(delta.z, delta.z);
            else
                distTable[g_carOrder[i]] = 0x7fbc0000;
            i++;
        } while (i < g_carOrderCount);
    }

    FixMatrix_GetUp(&up, pCurrent);
    FIX_NORMALIZE_INTO(up, up)

    savedWorld = pView->world;
    pView->world = pView->current;

    j = 1;
    if (g_carOrderCount > 1) {
        do {
            FixMatrix_GetPosition(&otherPos, &Car_Get(g_carOrder[j])->pNode0x71c->current);
            FUN_004bad40(proj1, &otherPos, (BYTE *)pView);
            proj1[0] >>= 16;
            proj1[1] >>= 16;
            FixVecScale(&delta, &up, *(int *)Car_Get(g_carOrder[j])->field_0x758);
            delta.x += otherPos.x;
            delta.y += otherPos.y;
            delta.z += otherPos.z;
            FUN_004bad40(proj2, &delta, (BYTE *)pView);
            proj2[0] >>= 16;
            proj2[1] >>= 16;
            if ((proj1[0] == -100 && proj1[1] == -100) || (proj2[0] == -100 && proj2[1] == -100)) {
                distTable[g_carOrder[j]] = 0x7fbc0000;
            } else {
                ftmp = (float)(proj1[0] - proj2[0]) / (float)(int)g_pGraphics->resX;
                ix = (int)(ftmp * g_65536f);
                ftmp = (float)(proj1[1] - proj2[1]) / (float)(int)g_pGraphics->resY;
                iy = (int)(ftmp * g_65536f);
                xx = FixMul(FixMul(ix, 0x2800000), 0x28f);
                yy = FixMul(FixMul(iy, 0x1e00000), 0x28f);
                extend = FixMul(FixSqrt(FixMul(xx, xx) + FixMul(yy, yy)), 0x4b0000) >> 16;
                sx = (int)g_pGraphics->resX * extend / 0x280;
                sy = (int)g_pGraphics->resY * extend / 0x1e0;
                if (proj1[0] < (short)(pRect[0] - sx) || proj1[0] > (short)(pRect[0] + pRect[2] + sx) ||
                    proj1[1] < (short)(pRect[1] - sy) || proj1[1] > (short)(pRect[1] + pRect[3] + sy))
                    distTable[g_carOrder[j]] = 0x7fbc0000;
            }
            j++;
        } while (j < g_carOrderCount);
    }
    pView->world = savedWorld;

    for (k = 0; k < 3; k++) {
        bestIdx = -1;
        bestDist = 0x7fff0000;
        if (g_carOrderCount > 0) {
            for (i = 0; i < g_carOrderCount; i++) {
                car = g_carOrder[i];
                if (visible[car] == 0) {
                    if (car == 0) {
                        visible[car] = 2;
                        break;
                    }
                    if (distTable[car] < bestDist) {
                        bestIdx = car;
                        bestDist = distTable[car];
                    }
                }
            }
            if (bestIdx != -1) {
                if (bestDist > 0xe100000)
                    break;
                visible[bestIdx] = 2;
            }
        }
    }

    i = 0;
    if (g_carOrderCount > 0) {
        do {
            FUN_0046b760(g_carOrder[i], visible[g_carOrder[i]] == 2);
            i++;
        } while (i < g_carOrderCount);
    }
}

// FUNCTION: CMR2 0x0042b5f0
Car *Car_Get(int index)
{
    return g_cars[index];
}

// Dampens the car's secondary body motion while this mode is active.
// FUNCTION: CMR2 0x00437d70
void Car_DampenBodyMotion(void)
{
    if (g_pCurrentCar->field_0xb28 == 0 && g_pCurrentCar->field_0xb74 != 0) {
        int change = g_pCurrentCar->field_0x7a4 - *(int *)g_pCurrentCar->field_0x7a8;
        g_pCurrentCar->field_0x5d0.z -= FixMul(0x1999, change);
    }
}

// Road speed of a wheel from its spin: raw (unit 2), in km/h (0) or mph (1).
// FUNCTION: CMR2 0x0042b600
int Car_GetWheelSpeed(Car *pCar, BYTE wheel, int unit)
{
    int speed;

    speed = FixMul(pCar->wheelLoad[wheel], 0x4000);
    switch (unit) {
    case 0:
        return FixMul(speed, 0x431168);
    case 1:
        return FixMul(speed, 0x6bfa9f);
    }
    return speed;
}

#define ADD_POSITION(p, pos)    \
    (p)->x += (pos).x;          \
    (p)->y += (pos).y;          \
    (p)->z += (pos).z;          \
    (p)++;

// Selects the per-gear speed table of the current car (eight 16.16 values at
// 0x7dc) and rescales the engine-speed accumulator (0x788) accordingly, then
// normalises the table by 0x123d7.
// FUNCTION: CMR2 0x0043e1f0
void FUN_0043e1f0(unsigned int param_1)
{
    int i;
    int v;

    switch (param_1) {
    case 4:
        g_pCurrentCar->field_0x7dc[0] = 0;
        g_pCurrentCar->field_0x7dc[1] = 0x553f;
        g_pCurrentCar->field_0x7dc[2] = 0x7893;
        g_pCurrentCar->field_0x7dc[3] = 0x9ef9;
        g_pCurrentCar->field_0x7dc[4] = 0xc28f;
        g_pCurrentCar->field_0x7dc[5] = 0xe4dd;
        g_pCurrentCar->field_0x7dc[6] = 0x10ac0;
        g_pCurrentCar->field_0x7dc[7] = 0xffffaac1;
        break;
    case 3:
        g_pCurrentCar->field_0x7dc[0] = 0;
        g_pCurrentCar->field_0x7dc[1] = 0x5374;
        g_pCurrentCar->field_0x7dc[2] = 0x73b6;
        g_pCurrentCar->field_0x7dc[3] = 0x9604;
        g_pCurrentCar->field_0x7dc[4] = 0xb4fd;
        g_pCurrentCar->field_0x7dc[5] = 0xd26e;
        g_pCurrentCar->field_0x7dc[6] = 0xf26e;
        g_pCurrentCar->field_0x7dc[7] = 0xffffac8c;
        *(int *)g_pCurrentCar->field_0x788 = *(int *)g_pCurrentCar->field_0x788 + 0x1eb;
        break;
    case 2:
        g_pCurrentCar->field_0x7dc[0] = 0;
        g_pCurrentCar->field_0x7dc[1] = 0x5168;
        g_pCurrentCar->field_0x7dc[2] = 0x6ed9;
        g_pCurrentCar->field_0x7dc[3] = 0x8d0e;
        g_pCurrentCar->field_0x7dc[4] = 0xa76c;
        g_pCurrentCar->field_0x7dc[5] = 0xc000;
        g_pCurrentCar->field_0x7dc[6] = 0xd9db;
        g_pCurrentCar->field_0x7dc[7] = 0xffffae98;
        *(int *)g_pCurrentCar->field_0x788 = *(int *)g_pCurrentCar->field_0x788 + 0x3d7;
        break;
    case 1:
        g_pCurrentCar->field_0x7dc[0] = 0;
        g_pCurrentCar->field_0x7dc[1] = 0x4f9d;
        g_pCurrentCar->field_0x7dc[2] = 0x69fb;
        g_pCurrentCar->field_0x7dc[3] = 0x83d7;
        g_pCurrentCar->field_0x7dc[4] = 0x9999;
        g_pCurrentCar->field_0x7dc[5] = 0xad4f;
        g_pCurrentCar->field_0x7dc[6] = 0xc189;
        g_pCurrentCar->field_0x7dc[7] = 0xffffb063;
        *(int *)g_pCurrentCar->field_0x788 = *(int *)g_pCurrentCar->field_0x788 + 0x5c2;
        break;
    case 0:
        g_pCurrentCar->field_0x7dc[0] = 0;
        g_pCurrentCar->field_0x7dc[1] = 0x4d91;
        g_pCurrentCar->field_0x7dc[2] = 0x651e;
        g_pCurrentCar->field_0x7dc[3] = 0x7ae1;
        g_pCurrentCar->field_0x7dc[4] = 0x8c08;
        g_pCurrentCar->field_0x7dc[5] = 0x9ae1;
        g_pCurrentCar->field_0x7dc[6] = 0xa8f5;
        g_pCurrentCar->field_0x7dc[7] = 0xffffb26f;
        *(int *)g_pCurrentCar->field_0x788 = *(int *)g_pCurrentCar->field_0x788 + 0x7ae;
        break;
    }
    for (i = 0; i < 8; i++) {
        v = g_pCurrentCar->field_0x7dc[i];
        g_pCurrentCar->field_0x7dc[i] = FixDiv(v, 0x123d7);
    }
}

// Sets three handling factors of the current car from a 16.16 level.
// FUNCTION: CMR2 0x0043e530
void FUN_0043e530(int level)
{
    g_pCurrentCar->field_0x804 = FixMul(0xcccc, level) + 0x9999;
    g_pCurrentCar->field_0x7fc = FixMul(0xa3d, level) + 0xccc;
    g_pCurrentCar->field_0x800 = FixMul(0x3333, level) + 0x1999;
}

BYTE *FUN_00456be0(int index);
SceneNode *SceneNode_FindByType(SceneNode *pNode, unsigned int type);
int SceneNode_Reparent(SceneNode *pNode, SceneNode *pNewParent);
int RallyData_FUN_00411060(void);
void FUN_0042cb90(char mode, SceneNode **pWheels);

// Binds a car to its loaded model: body nodes, wheels and view nodes, and
// moves both body nodes under the stage root.
// FUNCTION: CMR2 0x0043e5a0
void FUN_0043e5a0(int model, Car *pCar)
{
    BYTE *pModel = FUN_00456be0(model);
    SceneNode *pBody = *(SceneNode **)(pModel + 8);
    SceneNode *pBody2 = *(SceneNode **)(pModel + 4);

    pCar->pNode0x71c = pBody;
    pCar->pNode0x720 = pBody2;
    pCar->pWheelNodes[0] = SceneNode_FindByType(pBody, 1);
    pCar->pWheelNodes[1] = SceneNode_FindByType(pBody, 2);
    pCar->pWheelNodes[2] = SceneNode_FindByType(pBody, 3);
    pCar->pWheelNodes[3] = SceneNode_FindByType(pBody, 4);
    pCar->pViewNodeFar = SceneNode_FindByType(pBody2, 0xe);
    pCar->pViewNodeNear = SceneNode_FindByType(pBody2, 0xf);
    FUN_0042cb90(FUN_00456be0(pCar->field_0xb1a)[0x20], pCar->pWheelNodes);
    if (*(int *)((BYTE *)pBody + 8) != RallyData_FUN_00411060())
        SceneNode_Reparent(pBody, (SceneNode *)RallyData_FUN_00411060());
    if (*(int *)((BYTE *)pCar->pNode0x720 + 8) != RallyData_FUN_00411060())
        SceneNode_Reparent(pCar->pNode0x720, (SceneNode *)RallyData_FUN_00411060());
}

// Recomputes the eight world-space corners of the car's box from its
// half extents and world matrix, then applies the suspension offsets.
// match 76%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0043eef0
void Car_UpdateCorners(Car *pCar)
{
    int hx;
    int hy;
    int hz;
    FixMatrix *pM;
    int ax;
    int ay;
    int az;
    FixVector *p;

    hx = pCar->halfExtents.x;
    pM = pCar->pWorld;
    hy = pCar->halfExtents.y;
    hz = pCar->halfExtents.z;
    g_pCurrentCar = pCar;

    ax = FixMul(pM->right.x, hx);
    ay = FixMul(pM->up.x, hy);
    az = FixMul(pM->forward.x, hz);
    pCar->corners[4].x = az + ay + ax;
    pCar->corners[5].x = (ay - az) + ax;
    pCar->corners[7].x = (ay - az) - ax;
    pCar->corners[6].x = (az - ax) + ay;

    ax = FixMul(pM->right.y, hx);
    ay = FixMul(pM->up.y, hy);
    az = FixMul(pM->forward.y, hz);
    pCar->corners[4].y = az + ay + ax;
    pCar->corners[5].y = (ay - az) + ax;
    pCar->corners[7].y = (ay - az) - ax;
    pCar->corners[6].y = (az - ax) + ay;

    ax = FixMul(pM->right.z, hx);
    ay = FixMul(pM->up.z, hy);
    az = FixMul(pM->forward.z, hz);
    pCar->corners[4].z = az + ay + ax;
    pCar->corners[5].z = (ay - az) + ax;
    pCar->corners[7].z = (ay - az) - ax;
    pCar->corners[6].z = (az - ax) + ay;

    pCar->corners[0].x = -pCar->corners[7].x;
    pCar->corners[1].x = -pCar->corners[6].x;
    pCar->corners[2].x = -pCar->corners[5].x;
    pCar->corners[3].x = -pCar->corners[4].x;
    pCar->corners[0].y = -pCar->corners[7].y;
    pCar->corners[1].y = -pCar->corners[6].y;
    pCar->corners[2].y = -pCar->corners[5].y;
    pCar->corners[3].y = -pCar->corners[4].y;
    pCar->corners[0].z = -pCar->corners[7].z;
    pCar->corners[1].z = -pCar->corners[6].z;
    pCar->corners[2].z = -pCar->corners[5].z;
    pCar->corners[3].z = -pCar->corners[4].z;

    p = pCar->corners;
    ADD_POSITION(p, pCar->position)
    ADD_POSITION(p, pCar->position)
    ADD_POSITION(p, pCar->position)
    ADD_POSITION(p, pCar->position)
    ADD_POSITION(p, pCar->position)
    ADD_POSITION(p, pCar->position)
    ADD_POSITION(p, pCar->position)
    ADD_POSITION(p, pCar->position)

    if (g_pCurrentCar->field_0xb64 == 0)
        Car_ApplyCornerOffsets();
}

// Shifts the four upper corners of g_pCurrentCar by the scaled body
// offset vectors (suspension travel).
// FUNCTION: CMR2 0x0043f280
void Car_ApplyCornerOffsets(void)
{
    FixVector v[3];

    FixVecScale(&v[0], &g_pCurrentCar->right, g_pCurrentCar->scale0x764);
    FixVecScale(&v[1], &g_pCurrentCar->forward, g_pCurrentCar->scale0x76c);
    FixVecScale(&v[2], &g_pCurrentCar->right, g_pCurrentCar->scale0x768);
    g_pCurrentCar->corners[4].x -= v[0].x;
    g_pCurrentCar->corners[4].y -= v[0].y;
    g_pCurrentCar->corners[4].z -= v[0].z;
    g_pCurrentCar->corners[4].x -= v[1].x;
    g_pCurrentCar->corners[4].y -= v[1].y;
    g_pCurrentCar->corners[4].z -= v[1].z;
    g_pCurrentCar->corners[5].x -= v[0].x;
    g_pCurrentCar->corners[5].y -= v[0].y;
    g_pCurrentCar->corners[5].z -= v[0].z;
    g_pCurrentCar->corners[5].x += v[1].x;
    g_pCurrentCar->corners[5].y += v[1].y;
    g_pCurrentCar->corners[5].z += v[1].z;
    g_pCurrentCar->corners[6].x += v[2].x;
    g_pCurrentCar->corners[6].y += v[2].y;
    g_pCurrentCar->corners[6].z += v[2].z;
    g_pCurrentCar->corners[6].x -= v[1].x;
    g_pCurrentCar->corners[6].y -= v[1].y;
    g_pCurrentCar->corners[6].z -= v[1].z;
    g_pCurrentCar->corners[7].x += v[2].x;
    g_pCurrentCar->corners[7].y += v[2].y;
    g_pCurrentCar->corners[7].z += v[2].z;
    g_pCurrentCar->corners[7].x += v[1].x;
    g_pCurrentCar->corners[7].y += v[1].y;
    g_pCurrentCar->corners[7].z += v[1].z;
}

// Scene node stored at byte offset `off + field` inside the car table.
#define CAR_NODE(off, field) (*(SceneNode **)((int)g_carBuffer + (off) + (field)))

// Applies the stored transforms of every car to its scene nodes for the
// given view. Cars closer than 2.0 to the view get the transforms as they
// are; farther cars are pulled 1.5 units towards the view and their axes
// scaled by (1 - 1.5 / distance), which is stored in g_carViewScale.
// FUNCTION: CMR2 0x004292c0
void Car_ApplyViewTransforms(int viewIndex)
{
    FixVector viewPos;
    FixVector delta;
    FixVector bodyPos;
    FixVector axis;
    int pull;
    int n;
    int i;
    short *pOrder;
    int carIndex;
    int wheel;
    int offset;
    int wheelOffset;
    int scale;
    int len;
    CarTransforms *pT;
    FixMatrix *pWheel;

    pull = 0x18000;
    FixMatrix_GetPosition(&viewPos, &g_viewNodes[viewIndex]->current);
    i = g_carOrderCount - 1;
    if (i >= 0) {
        pOrder = &g_carOrder[i];
        n = i + 1;
        do {
            scale = 0x10000;
            carIndex = *pOrder;
            pT = &g_carTransforms[carIndex];
            i = carIndex;
            FixMatrix_GetPosition(&bodyPos, &pT->body);
            wheel = 4;
            offset = carIndex * sizeof(Car);
            pWheel = g_carWheelTransforms[carIndex];
            wheelOffset = offset + 0x738;
            do {
                FixMatrix_CopyRotation(pWheel, &CAR_NODE(wheelOffset, 0)->current);
                wheelOffset += 4;
                pWheel++;
                wheel--;
            } while (wheel != 0);
            delta.x = viewPos.x - bodyPos.x;
            delta.y = viewPos.y - bodyPos.y;
            delta.z = viewPos.z - bodyPos.z;
            len = FixVecLength(&delta);
            if (len > 0x20000) {
                if (len != 0) {
                    scale = 0x10000 - FixDiv(pull, len);
                    FixVecScaleRecip(&delta, &delta, len);
                    FixVecScale(&delta, &delta, pull);
                    bodyPos.x += delta.x;
                    bodyPos.y += delta.y;
                    bodyPos.z += delta.z;
                    FixMatrix_SetPosition(&bodyPos, &CAR_NODE(offset, 0x71c)->current);
                    FixMatrix_GetRight(&axis, &pT->body);
                    FixVecScale(&axis, &axis, scale);
                    FixMatrix_SetRight(&axis, &CAR_NODE(offset, 0x71c)->current);
                    FixMatrix_GetUp(&axis, &pT->body);
                    FixVecScale(&axis, &axis, scale);
                    FixMatrix_SetUp(&axis, &CAR_NODE(offset, 0x71c)->current);
                    FixMatrix_GetForward(&axis, &pT->body);
                    FixVecScale(&axis, &axis, scale);
                    FixMatrix_SetForward(&axis, &CAR_NODE(offset, 0x71c)->current);
                    FixMatrix_SetPosition(&bodyPos, &CAR_NODE(offset, 0x720)->current);
                    FixMatrix_GetRight(&axis, &pT->body2);
                    FixVecScale(&axis, &axis, scale);
                    FixMatrix_SetRight(&axis, &CAR_NODE(offset, 0x720)->current);
                    FixMatrix_GetUp(&axis, &pT->body2);
                    FixVecScale(&axis, &axis, scale);
                    FixMatrix_SetUp(&axis, &CAR_NODE(offset, 0x720)->current);
                    FixMatrix_GetForward(&axis, &pT->body2);
                    FixVecScale(&axis, &axis, scale);
                    FixMatrix_SetForward(&axis, &CAR_NODE(offset, 0x720)->current);
                    carIndex = i;
                }
            } else {
                FixMatrix_CopyRotation(&pT->body, &CAR_NODE(offset, 0x71c)->current);
                FixMatrix_CopyRotation(&pT->body2, &CAR_NODE(offset, 0x720)->current);
            }
            pOrder--;
            g_carViewScale[carIndex][viewIndex] = scale;
            n--;
        } while (n != 0);
    }
}

#define SCALE_NODE_AXES(pNode, s)                                            \
    (pNode)->current.right.x = FixMul((pNode)->local.right.x, s);            \
    (pNode)->current.right.y = FixMul((pNode)->local.right.y, s);            \
    (pNode)->current.right.z = FixMul((pNode)->local.right.z, s);            \
    (pNode)->current.up.x = FixMul((pNode)->local.up.x, s);                  \
    (pNode)->current.up.y = FixMul((pNode)->local.up.y, s);                  \
    (pNode)->current.up.z = FixMul((pNode)->local.up.z, s);                  \
    (pNode)->current.forward.x = FixMul((pNode)->local.forward.x, s);        \
    (pNode)->current.forward.y = FixMul((pNode)->local.forward.y, s);        \
    (pNode)->current.forward.z = FixMul((pNode)->local.forward.z, s);        \
    (pNode)->useParentWorld = 0;                                             \
    for (p = (pNode); p != NULL; p = p->pParent)                             \
        p->dirty = 1;

// Places the two view-dependent child nodes of every car (0x748 in front of
// the body towards the view, 0x74c behind it) along the car -> view
// direction, scaled with the distance.
// match 75%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00429810
void Car_UpdateViewNodes(int viewIndex)
{
    int len;
    short *pOrder;
    int nearDist;
    int n;
    int s;
    int i;
    int farDist;
    FixVector local;
    FixVector viewPos;
    FixVector pos;
    FixVector carPos;
    FixVector delta;
    Car *pCar;
    SceneNode *p;

    nearDist = 0x1999;
    farDist = 0x8000;
    FixMatrix_GetPosition(&viewPos, &g_viewNodes[viewIndex]->current);
    i = g_carOrderCount - 1;
    if (i >= 0) {
        pOrder = &g_carOrder[i];
        n = i + 1;
        do {
            pCar = Car_Get(*pOrder);
            if (pCar->pViewNodeNear != NULL || pCar->pViewNodeFar != NULL) {
                FixMatrix_GetPosition(&carPos, &pCar->pNode0x720->current);
                delta.x = carPos.x - viewPos.x;
                delta.y = carPos.y - viewPos.y;
                delta.z = carPos.z - viewPos.z;
                if (FIX_ABS(delta.x) <= 0x800000 && FIX_ABS(delta.y) <= 0x800000 && FIX_ABS(delta.z) <= 0x800000) {
                    len = FixVecLength(&delta);
                    if (len == 0)
                        goto next;
                    FixVecScaleRecip(&delta, &delta, len);
                } else {
                    delta.x /= 128;
                    delta.y /= 128;
                    delta.z /= 128;
                    len = FixVecLength(&delta);
                    FixVecScaleRecip(&delta, &delta, len);
                    len <<= 7;
                }
                if (len != 0) {
                    FixMatrix_InverseRotateVector(&local, &delta, &pCar->pNode0x720->current);
                    if (pCar->pViewNodeNear != NULL) {
                        FixVecScale(&pos, &local, nearDist);
                        FixMatrix_SetPosition(&pos, &(pCar->pViewNodeNear)->current);
                        s = FixDiv(nearDist, len) + 0x10000;
                        SCALE_NODE_AXES(pCar->pViewNodeNear, s)
                    }
                    if (pCar->pViewNodeFar != NULL) {
                        FixVecScale(&pos, &local, -farDist);
                        FixMatrix_SetPosition(&pos, &(pCar->pViewNodeFar)->current);
                        s = 0x10000 - FixDiv(farDist, len);
                        SCALE_NODE_AXES(pCar->pViewNodeFar, s)
                    }
                }
            }
next:
            pOrder--;
            n--;
        } while (n != 0);
    }
}

// Physics time scale (16.16, 1.0 in the shipped data).
int g_physicsTimeStep = 0x10000;

// Updates the body axes of g_pCurrentCar: the up vector is pulled towards
// the wheel plane and the velocity, forward/right are re-orthogonalised,
// then a fraction of the velocity is turned into lateral force and damped.
// match 88%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00436630
void Car_UpdateBodyAxes(void)
{
    int step;
    int damping;
    int maxStep;
    FixVector localUp;
    FixVector world;
    FixVector diff;

    if (g_pCurrentCar->field_0xb28 > 0 && g_pCurrentCar->field_0xb74 != 0) {
        damping = FixMul(0x624, g_physicsTimeStep);
        maxStep = FixMul(0x312, g_physicsTimeStep);
        step = FixMul(g_pCurrentCar->angularVelocity.y, 0x3e80000);
        step = step - FixMul(step, damping);
        g_pCurrentCar->angularVelocity.y = FixMul(step, 0x41);

        if (FIX_ABS(g_pCurrentCar->speed) < 0x28f) {
            localUp.x = 0;
            localUp.y = 0;
            localUp.z = 0;
        } else {
            localUp.y = 0;
            localUp.x = g_sinTable[(unsigned short)(g_pCurrentCar->heading + 0x400) & 0xfff];
            localUp.z = -g_sinTable[g_pCurrentCar->heading & 0xfff];
        }
        FixMatrix_RotateVector(&world, &localUp, g_pCurrentCar->pWorld);
        if (FixVecDot(&g_pCurrentCar->velocity, &g_pCurrentCar->right) >= 0)
            FixVecScale(&world, &world, FixMul(g_pCurrentCar->speed, 0x2604));
        else
            FixVecScale(&world, &world, -FixMul(g_pCurrentCar->speed, 0x2604));

        localUp.x = g_pCurrentCar->right.x + world.x;
        localUp.y = g_pCurrentCar->right.y + world.y;
        localUp.z = g_pCurrentCar->right.z + world.z;
        FIX_NORMALIZE_INTO(localUp, localUp)
        diff.x = localUp.x - g_pCurrentCar->right.x;
        diff.y = localUp.y - g_pCurrentCar->right.y;
        diff.z = localUp.z - g_pCurrentCar->right.z;
        {
            FixVector *pUp;
            step = FixVecLength(&diff);
            if (step < maxStep) {
                pUp = &g_pCurrentCar->right;
                pUp->x = localUp.x;
                pUp->y = localUp.y;
                pUp->z = localUp.z;
            } else {
                FixVecScaleRecip(&localUp, &diff, step);
                FixVecScale(&localUp, &localUp, maxStep);
                pUp = &g_pCurrentCar->right;
                localUp.x = localUp.x + pUp->x;
                localUp.y = localUp.y + pUp->y;
                localUp.z = localUp.z + pUp->z;
                FIX_NORMALIZE_INTO((*pUp), localUp)
            }
        }

        {
            FixVector *pForward;
            int d = FixVecDot(&g_pCurrentCar->right, &g_pCurrentCar->forward);
            FixVecScale(&localUp, &g_pCurrentCar->right, d);
            pForward = &g_pCurrentCar->forward;
            localUp.x = pForward->x - localUp.x;
            localUp.y = pForward->y - localUp.y;
            localUp.z = pForward->z - localUp.z;
            FIX_NORMALIZE_INTO((*pForward), localUp)
        }

        {
            FixVector *pRight;
            FixVecCross(&localUp, &g_pCurrentCar->forward, &g_pCurrentCar->right);
            pRight = &g_pCurrentCar->up;
            FIX_NORMALIZE_INTO((*pRight), localUp)
        }

        {
            int d = FixVecDot(&g_pCurrentCar->up, &g_pCurrentCar->velocity);
            FixVecScale(&localUp, &g_pCurrentCar->up, d);
            localUp.x = g_pCurrentCar->velocity.x - localUp.x;
            localUp.y = g_pCurrentCar->velocity.y - localUp.y;
            localUp.z = g_pCurrentCar->velocity.z - localUp.z;
        }
        if (FixVecLength(&localUp) > 0) {
            int s = FixMul(FixMul(0x1e0000, g_pCurrentCar->speed), g_pCurrentCar->field_0x81c);
            if (g_pCurrentCar->field_0x1d8 != 0)
                s += FixMul(s, 0x20000);
            FixVecScale(&localUp, &g_pCurrentCar->forward, s);
            g_pCurrentCar->cornerForce[0].x += localUp.x;
            g_pCurrentCar->cornerForce[0].y += localUp.y;
            g_pCurrentCar->cornerForce[0].z += localUp.z;
            g_pCurrentCar->cornerForce[1].x += localUp.x;
            g_pCurrentCar->cornerForce[1].y += localUp.y;
            g_pCurrentCar->cornerForce[1].z += localUp.z;
        }

        localUp.x = g_pCurrentCar->velocity.x;
        localUp.y = g_pCurrentCar->velocity.y;
        localUp.y = 0;
        localUp.z = g_pCurrentCar->velocity.z;
        {
            int len = FixVecLength(&localUp);
            if (len > 0) {
                int d;
                FixVecScaleRecip(&localUp, &localUp, len);
                d = FixVecDot(&g_pCurrentCar->normal0x498, &localUp) - FixVecDot(&g_pCurrentCar->groundNormal, &localUp);
                if (d > 0x41) {
                    int s = FixMul(d, g_pCurrentCar->field_0x9c4);
                    if (s > 0x10000)
                        s = 0x10000;
                    s = 0x10000 - s;
                    g_pCurrentCar->velocity.x = FixMul(g_pCurrentCar->velocity.x, s);
                    g_pCurrentCar->velocity.y = FixMul(g_pCurrentCar->velocity.y, s);
                    g_pCurrentCar->velocity.z = FixMul(g_pCurrentCar->velocity.z, s);
                }
            }
        }
    }
}

// Resets the current wheels' loads to the engine torque (front, rear or all
// four, by the drive split) after a gear change.
// FUNCTION: CMR2 0x0043f570
void FUN_0043f570(Car *pCar)
{
    int load;

    pCar->field_0xb1e = 1;
    if (pCar->field_0x7b4 == 0) {
        load = FixMul(pCar->field_0x7a4, pCar->field_0x7dc[1]);
        pCar->wheelLoad[3] = load;
        pCar->wheelLoad[2] = load;
    } else {
        if (pCar->field_0x7b4 == 0x10000) {
            load = FixMul(pCar->field_0x7a4, pCar->field_0x7dc[1]);
        } else {
            load = FixMul(pCar->field_0x7a4, pCar->field_0x7dc[1]);
            pCar->wheelLoad[3] = load;
            pCar->wheelLoad[2] = load;
        }
        pCar->wheelLoad[1] = load;
        pCar->wheelLoad[0] = load;
    }
    pCar->field_0xb1f = 10;
    g_pCurrentCar->field_0x7ac = g_pCurrentCar->field_0x7a4;
}

// Sets up the current car's four drive flags and the per-wheel share of the
// torque (a quarter of 0x75c each).
// FUNCTION: CMR2 0x0043fb50
void FUN_0043fb50(void)
{
    int i;

    for (i = 0; i < 4; i++) {
        g_pCurrentCar->field_0xbac[i] = 1;
        g_pCurrentCar->field_0xb28++;
    }
    g_pCurrentCar->field_0xb28 = 4;
    g_pCurrentCar->field_0x8b4 = g_pCurrentCar->field_0x75c / 4;
    for (i = 3; i >= 0; i--)
        g_pCurrentCar->field_0x8b8[i] = g_pCurrentCar->field_0x8b4;
}

extern FixVector g_carStepAccel;

// Sets the current car's corner mass and the per-corner spring rates from its
// mass and suspension lengths.
// FUNCTION: CMR2 0x0043fbd0
void FUN_0043fbd0(void)
{
    int k;
    int i;

    g_pCurrentCar->cornerMass = g_pCurrentCar->field_0x75c / 4;
    g_carStepAccel.x = 0;
    g_carStepAccel.y = 0;
    g_carStepAccel.z = 0;
    k = FixMul(g_pCurrentCar->cornerMass + 0x1e0000, 0x8000);
    g_pCurrentCar->field_0xa5c[0] = FixMul(k, *(int *)((BYTE *)g_pCurrentCar + 0x1a4) + *(int *)((BYTE *)g_pCurrentCar + 0x88));
    g_pCurrentCar->field_0xa4c[0] = FixMul(k, *(int *)((BYTE *)g_pCurrentCar + 0x1a4) + *(int *)((BYTE *)g_pCurrentCar + 0x80));
    for (i = 1; i < 4; i++) {
        g_pCurrentCar->field_0xa5c[i] = g_pCurrentCar->field_0xa5c[0];
        g_pCurrentCar->field_0xa4c[i] = g_pCurrentCar->field_0xa4c[0];
    }
}

// Same as Car_UpdateBodyAxes without the final velocity damping.
// FUNCTION: CMR2 0x0043fcc0
void Car_UpdateBodyAxesNoDamping(void)
{
    int step;
    int damping;
    int maxStep;
    FixVector localUp;
    FixVector world;
    FixVector diff;

    if (g_pCurrentCar->field_0xb28 > 0 && g_pCurrentCar->field_0xb74 != 0) {
        damping = FixMul(0x624, g_physicsTimeStep);
        maxStep = FixMul(0x312, g_physicsTimeStep);
        step = FixMul(g_pCurrentCar->angularVelocity.y, 0x3e80000);
        step = step - FixMul(step, damping);
        g_pCurrentCar->angularVelocity.y = FixMul(step, 0x41);

        if (FIX_ABS(g_pCurrentCar->speed) < 0x28f) {
            localUp.x = 0;
            localUp.y = 0;
            localUp.z = 0;
        } else {
            localUp.y = 0;
            localUp.x = g_sinTable[(unsigned short)(g_pCurrentCar->heading + 0x400) & 0xfff];
            localUp.z = -g_sinTable[g_pCurrentCar->heading & 0xfff];
        }
        FixMatrix_RotateVector(&world, &localUp, g_pCurrentCar->pWorld);
        if (FixVecDot(&g_pCurrentCar->velocity, &g_pCurrentCar->right) >= 0)
            FixVecScale(&world, &world, FixMul(g_pCurrentCar->speed, 0x2604));
        else
            FixVecScale(&world, &world, -FixMul(g_pCurrentCar->speed, 0x2604));

        localUp.x = g_pCurrentCar->right.x + world.x;
        localUp.y = g_pCurrentCar->right.y + world.y;
        localUp.z = g_pCurrentCar->right.z + world.z;
        FIX_NORMALIZE_INTO(localUp, localUp)
        diff.x = localUp.x - g_pCurrentCar->right.x;
        diff.y = localUp.y - g_pCurrentCar->right.y;
        diff.z = localUp.z - g_pCurrentCar->right.z;
        {
            FixVector *pUp;
            step = FixVecLength(&diff);
            if (step < maxStep) {
                pUp = &g_pCurrentCar->right;
                pUp->x = localUp.x;
                pUp->y = localUp.y;
                pUp->z = localUp.z;
            } else {
                FixVecScaleRecip(&localUp, &diff, step);
                FixVecScale(&localUp, &localUp, maxStep);
                pUp = &g_pCurrentCar->right;
                localUp.x = localUp.x + pUp->x;
                localUp.y = localUp.y + pUp->y;
                localUp.z = localUp.z + pUp->z;
                FIX_NORMALIZE_INTO((*pUp), localUp)
            }
        }

        {
            FixVector *pForward;
            int d = FixVecDot(&g_pCurrentCar->right, &g_pCurrentCar->forward);
            FixVecScale(&localUp, &g_pCurrentCar->right, d);
            pForward = &g_pCurrentCar->forward;
            localUp.x = pForward->x - localUp.x;
            localUp.y = pForward->y - localUp.y;
            localUp.z = pForward->z - localUp.z;
            FIX_NORMALIZE_INTO((*pForward), localUp)
        }

        {
            FixVector *pRight;
            FixVecCross(&localUp, &g_pCurrentCar->forward, &g_pCurrentCar->right);
            pRight = &g_pCurrentCar->up;
            FIX_NORMALIZE_INTO((*pRight), localUp)
        }

        {
            int d = FixVecDot(&g_pCurrentCar->up, &g_pCurrentCar->velocity);
            FixVecScale(&localUp, &g_pCurrentCar->up, d);
            localUp.x = g_pCurrentCar->velocity.x - localUp.x;
            localUp.y = g_pCurrentCar->velocity.y - localUp.y;
            localUp.z = g_pCurrentCar->velocity.z - localUp.z;
        }
        if (FixVecLength(&localUp) > 0) {
            int s = FixMul(FixMul(0x1e0000, g_pCurrentCar->speed), g_pCurrentCar->field_0x81c);
            if (g_pCurrentCar->field_0x1d8 != 0)
                s += FixMul(s, 0x20000);
            FixVecScale(&localUp, &g_pCurrentCar->forward, s);
            g_pCurrentCar->cornerForce[0].x += localUp.x;
            g_pCurrentCar->cornerForce[0].y += localUp.y;
            g_pCurrentCar->cornerForce[0].z += localUp.z;
            g_pCurrentCar->cornerForce[1].x += localUp.x;
            g_pCurrentCar->cornerForce[1].y += localUp.y;
            g_pCurrentCar->cornerForce[1].z += localUp.z;
        }

    }
}

// Copies the body axes and position of g_pCurrentCar into its body matrix
// and rebuilds it from the wheel plane.
// FUNCTION: CMR2 0x00432c20
void Car_StoreBodyMatrix(void)
{
    g_pCurrentCar->pBodyMatrix->position = g_pCurrentCar->position;
    g_pCurrentCar->pBodyMatrix->right = g_pCurrentCar->right;
    g_pCurrentCar->pBodyMatrix->up = g_pCurrentCar->up;
    g_pCurrentCar->pBodyMatrix->forward = g_pCurrentCar->forward;
    Car_UpdateBodyMatrix();
}

// int field of g_pCurrentCar at byte offset off
#define CAR_INT(off) (*(int *)((int)g_pCurrentCar + (off)))

#define FIX_NORMALIZE_FLIP(v)                                                       \
    {                                                                               \
        int len = FixVecLength(&v);                                                 \
        if (len == 0) {                                                             \
            v.x = 0;                                                                \
            v.y = 0;                                                                \
            v.z = 0;                                                                \
        } else {                                                                    \
            FixVecScaleRecip(&v, &v, len);                                          \
            if (v.y < 0) {                                                          \
                v.y = -v.y;                                                         \
                v.x = -v.x;                                                         \
                v.z = -v.z;                                                         \
            }                                                                       \
        }                                                                           \
    }

// Tilts the body matrix of g_pCurrentCar to the plane through the wheel
// contact points (suspension compressions), keeping the right vector, and
// lifts its position by the mean compression.
// match 74%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00432cc0
void Car_UpdateBodyMatrix(void)
{
    FixVector a;
    FixVector n1;
    FixVector n2;
    FixVector b;
    FixVector world;
    FixVector forward;
    FixVector up;
    FixVector right;
    int rearAvg;
    int frontAvg;
    int comp[4];
    FixVector p2;
    FixVector p1;
    FixVector p0;
    int i;

    for (i = 0; i < 4; i++) {
        comp[i] = g_pCurrentCar->wheel0x988[i] + g_pCurrentCar->wheel0x9d8[i] + g_pCurrentCar->wheel0x9a8[i];
        if (CGameInfo::FUN_004063f0(6) != 0) {
            if (comp[i] < -0x5999)
                comp[i] = -0x5999;
        } else if (comp[i] < -0x1999) {
            comp[i] = -0x1999;
        }
    }
    rearAvg = FixMul(0x8000, comp[3] + comp[2]);
    frontAvg = FixMul(0x8000, comp[1] + comp[0]);

    p0 = g_pCurrentCar->wheelPos[0];
    p1 = g_pCurrentCar->wheelPos[1];
    p2 = g_pCurrentCar->wheelPos[2];
    a.x = p1.x - p0.x;
    a.y = comp[1] - comp[0];
    a.z = p1.z - p0.z;
    b.x = p2.x - p0.x;
    b.y = rearAvg - comp[0];
    b.z = -p0.z;
    FixVecCross(&n1, &a, &b);
    FIX_NORMALIZE_FLIP(n1)

    p2 = g_pCurrentCar->wheelPos[2];
    p1 = g_pCurrentCar->wheelPos[3];
    p0 = g_pCurrentCar->wheelPos[0];
    a.x = p1.x - p2.x;
    a.y = comp[3] - comp[2];
    a.z = p1.z - p2.z;
    b.x = p0.x - p2.x;
    b.y = frontAvg - comp[2];
    b.z = -p2.z;
    FixVecCross(&n2, &a, &b);
    FIX_NORMALIZE_FLIP(n2)

    a.x = n2.x + n1.x;
    a.y = n2.y + n1.y;
    a.z = n2.z + n1.z;
    FIX_NORMALIZE_INTO(a, a)
    FixMatrix_RotateVector(&world, &a, g_pCurrentCar->pWorld);
    FixMatrix_GetRight(&right, g_pCurrentCar->pBodyMatrix);
    FixMatrix_GetUp(&up, g_pCurrentCar->pBodyMatrix);
    FixMatrix_GetForward(&forward, g_pCurrentCar->pBodyMatrix);
    up.x = world.x;
    up.y = world.y;
    up.z = world.z;
    {
        int d = FixVecDot(&right, &world);
        right.x = right.x - FixMul(world.x, d);
        right.y = right.y - FixMul(world.y, d);
        right.z = right.z - FixMul(world.z, d);
    }
    FIX_NORMALIZE_INTO(right, right)
    FixVecCross(&forward, &right, &world);
    FIX_NORMALIZE_INTO(forward, forward)
    {
        int lift = FixMul(frontAvg + rearAvg, 0x8000);
        a.x = FixMul(g_pCurrentCar->up.x, lift) + g_pCurrentCar->position.x;
        a.y = FixMul(g_pCurrentCar->up.y, lift) + g_pCurrentCar->position.y;
        a.z = FixMul(g_pCurrentCar->up.z, lift) + g_pCurrentCar->position.z;
    }
    FixMatrix_SetRight(&right, g_pCurrentCar->pBodyMatrix);
    FixMatrix_SetUp(&up, g_pCurrentCar->pBodyMatrix);
    FixMatrix_SetForward(&forward, g_pCurrentCar->pBodyMatrix);
    FixMatrix_SetPosition(&a, g_pCurrentCar->pBodyMatrix);
}

extern BYTE *g_pCarSetup;

// unsigned short field of g_pCurrentCar at byte offset off
#define CAR_USHORT(off) (*(unsigned short *)((int)g_pCurrentCar + (off)))

// Advances the per-wheel suspension travel angle (0xb08) from the wheel load
// and turns it into the per-wheel contact-point offset (0x6fc/0x700), scaled
// by the per-wheel spring constant of the car setup.
// match 90%: MSVC picked the setup array (0x240) as the induction-variable base
// instead of the wheel-load array (0x860) the original used; same logic.
// GLOBAL: CMR2 0x00511398
double g_unk0x00511398 = -0.009947183943243459;
// FUNCTION: CMR2 0x004336f0
void Car_UpdateWheelTravel(void)
{
    int i;
    int load;
    int absLoad;
    short delta;

    for (i = 0; i < 4; i++) {
        load = CAR_INT(0x860 + i * 4);
        absLoad = FIX_ABS(load);
        if (absLoad > 0x10000) {
            if (load > 0) {
                CAR_USHORT(0xb08 + i * 2) += -0x28b;
            } else {
                CAR_USHORT(0xb08 + i * 2) += 0x28b;
            }
        } else {
            delta = (short)(__int64)((double)load * g_unk0x00511398);
            CAR_USHORT(0xb08 + i * 2) += delta;
        }
        if (*(int *)(g_pCarSetup + 0x240 + i * 4) > 0) {
            CAR_INT(0x6fc + i * 8) =
                FixMul(g_sinTable[CAR_USHORT(0xb08 + i * 2) & 0xfff],
                       FixMul(*(int *)(g_pCarSetup + 0x240 + i * 4), 0xa3d));
            CAR_INT(0x700 + i * 8) =
                FixMul(-g_sinTable[(unsigned short)(CAR_USHORT(0xb08 + i * 2) + 0x400) & 0xfff],
                       FixMul(*(int *)(g_pCarSetup + 0x240 + i * 4), 0xa3d));
        } else {
            CAR_INT(0x700 + i * 8) = 0;
            CAR_INT(0x6fc + i * 8) = 0;
        }
    }
}
// Normalises a body axis of g_pCurrentCar in place through a pointer.
#define CAR_NORMALIZE_AXIS(field)                                                   \
    {                                                                               \
        FixVector *pAxis = &g_pCurrentCar->field;                                   \
        FixVector *pSrc = pAxis;                                                    \
        int len = FixVecLength(pSrc);                                               \
        if (len == 0) {                                                             \
            pAxis->x = 0;                                                           \
            pAxis->y = 0;                                                           \
            pAxis->z = 0;                                                           \
        } else {                                                                    \
            FixVecScaleRecip(pAxis, pSrc, len);                                     \
        }                                                                           \
    }

// Flags the box corners of the current car that are off the ground and lists
// (from 0xb36) the ones that touch it.
// match 78%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0042f820
void FUN_0042f820(void)
{
    int i;

    g_pCurrentCar->field_0xb34 = 0;
    g_pCurrentCar->field_0xb35[0] = 1;
    for (i = 7; i >= 0; i--) {
        int h = g_pCurrentCar->cornerHeight[i] + 0x1999;

        if (g_pCurrentCar->corners[i].y <= h) {
            g_pCurrentCar->cornerFlags[i] = 0;
            g_pCurrentCar->field_0xb35[0] = 0;
            g_pCurrentCar->field_0xb35[1 + g_pCurrentCar->field_0xb34] = (BYTE)i;
            g_pCurrentCar->field_0xb34++;
        } else {
            g_pCurrentCar->cornerFlags[i] = 1;
        }
    }
}

// Relaxes the body up and forward vectors of g_pCurrentCar towards their
// targets (0x390/0x39c) with a rate that grows with the distance, then
// re-orthogonalises the basis and writes it to the world matrix.
// bFast selects the faster blend rates.
// FUNCTION: CMR2 0x00431220
void Car_RelaxBodyAxes(int bFast)
{
    int rateMin;
    int rateRange;
    int gain;
    FixVector diff;
    FixVector proj;
    int len;
    int t;
    int d;

    if (bFast != 0) {
        rateMin = 0x8000;
        rateRange = 0x8000;
    } else {
        rateMin = 0x1999;
        rateRange = 0x4ccc;
    }
    gain = FixMul(0xc0000, g_physicsTimeStep);

    if (FIX_ABS(g_pCurrentCar->field_0x920) > 0xfae1) {
        diff.x = g_pCurrentCar->up.x - g_pCurrentCar->targetUp.x;
        diff.y = g_pCurrentCar->up.y - g_pCurrentCar->targetUp.y;
        diff.z = g_pCurrentCar->up.z - g_pCurrentCar->targetUp.z;
        len = FixVecLength(&diff);
        if (len > 0) {
            t = FixMul(len, gain);
            if (t > 0x10000)
                t = 0x10000;
            FixVecScale(&diff, &diff, FixMul(t, rateRange) + rateMin);
            g_pCurrentCar->up.x = g_pCurrentCar->targetUp.x + diff.x;
            g_pCurrentCar->up.y = g_pCurrentCar->targetUp.y + diff.y;
            g_pCurrentCar->up.z = g_pCurrentCar->targetUp.z + diff.z;
            CAR_NORMALIZE_AXIS(up)
        }
    }

    if (FIX_ABS(g_pCurrentCar->field_0x924) > 0x3333 && FIX_ABS(g_pCurrentCar->field_0x91c) < 0x1999) {
        diff.x = g_pCurrentCar->forward.x - g_pCurrentCar->targetForward.x;
        diff.y = g_pCurrentCar->forward.y - g_pCurrentCar->targetForward.y;
        diff.z = g_pCurrentCar->forward.z - g_pCurrentCar->targetForward.z;
        d = FixVecDot(&diff, &g_pCurrentCar->up);
        FixVecScale(&proj, &g_pCurrentCar->up, d);
        diff.x = diff.x - proj.x;
        diff.y = diff.y - proj.y;
        diff.z = diff.z - proj.z;
        len = FixVecLength(&diff);
        if (len > 0) {
            t = FixMul(len, gain);
            if (t > 0x10000)
                t = 0x10000;
            FixVecScale(&diff, &diff, (FixMul(t, rateRange) - 0x10000) + rateMin);
            g_pCurrentCar->forward.x = g_pCurrentCar->forward.x + diff.x;
            g_pCurrentCar->forward.y = g_pCurrentCar->forward.y + diff.y;
            g_pCurrentCar->forward.z = g_pCurrentCar->forward.z + diff.z;
            CAR_NORMALIZE_AXIS(forward)
        }
    }

    d = FixVecDot(&g_pCurrentCar->up, &g_pCurrentCar->forward);
    FixVecScale(&diff, &g_pCurrentCar->up, d);
    g_pCurrentCar->forward.x = g_pCurrentCar->forward.x - diff.x;
    g_pCurrentCar->forward.y = g_pCurrentCar->forward.y - diff.y;
    g_pCurrentCar->forward.z = g_pCurrentCar->forward.z - diff.z;
    CAR_NORMALIZE_AXIS(forward)
    FixVecCross(&g_pCurrentCar->right, &g_pCurrentCar->up, &g_pCurrentCar->forward);
    CAR_NORMALIZE_AXIS(right)
    g_pCurrentCar->pWorld->right = g_pCurrentCar->right;
    g_pCurrentCar->pWorld->up = g_pCurrentCar->up;
    g_pCurrentCar->pWorld->forward = g_pCurrentCar->forward;
    g_pCurrentCar->pWorld->position = g_pCurrentCar->position;
}

// Velocity of each box corner of g_pCurrentCar: v + w x r, with the
// angular velocity in body space and the corners as +-half extents.
// match 66%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00434f50
void Car_UpdateCornerVelocities(void)
{
    FixVector *pCv;
    FixMatrix *pM;
    FixVector *pVel;
    FixVector *pW;
    int wyhz;
    int wzhy;
    int wzhx;
    int wxhz;
    int wxhy;
    int wyhx;
    int a;
    int b;
    int c;
    int d;
    int e;
    int f;
    int c0;
    int c1;
    int c2;
    int c3;

    pM = g_pCurrentCar->pWorld;
    pW = &g_pCurrentCar->angularVelocity;
    pVel = &g_pCurrentCar->velocity;
    pCv = g_pCurrentCar->cornerVelocity;
    wyhz = FixMul(pW->y, g_pCurrentCar->halfExtents.z);
    wzhy = FixMul(pW->z, g_pCurrentCar->halfExtents.y);
    wzhx = FixMul(pW->z, g_pCurrentCar->halfExtents.x);
    wxhz = FixMul(pW->x, g_pCurrentCar->halfExtents.z);
    wxhy = FixMul(pW->x, g_pCurrentCar->halfExtents.y);
    wyhx = FixMul(pW->y, g_pCurrentCar->halfExtents.x);

    a = FixMul(pM->right.x, wyhz);
    b = FixMul(pM->right.x, wzhy);
    c = FixMul(pM->up.x, wzhx);
    d = FixMul(pM->up.x, wxhz);
    e = FixMul(pM->forward.x, wxhy);
    f = FixMul(pM->forward.x, wyhx);
    pCv[0].x = (((c - f) - e) - d) + b + a;
    pCv[1].x = (((d - f) - e) - a) + c + b;
    pCv[2].x = (((f - e) - d) - c) + b + a;
    pCv[3].x = (((f - e) - c) - a) + d + b;

    a = FixMul(pM->right.y, wyhz);
    b = FixMul(pM->right.y, wzhy);
    c = FixMul(pM->up.y, wzhx);
    d = FixMul(pM->up.y, wxhz);
    e = FixMul(pM->forward.y, wxhy);
    f = FixMul(pM->forward.y, wyhx);
    pCv[0].y = (((c - f) - e) - d) + b + a;
    pCv[1].y = (((d - f) - e) - a) + c + b;
    pCv[2].y = (((f - e) - d) - c) + b + a;
    pCv[3].y = (((f - e) - c) - a) + d + b;

    a = FixMul(pM->right.z, wyhz);
    b = FixMul(pM->right.z, wzhy);
    c = FixMul(pM->up.z, wzhx);
    d = FixMul(pM->up.z, wxhz);
    e = FixMul(pM->forward.z, wxhy);
    f = FixMul(pM->forward.z, wyhx);
    c0 = (((c - f) - e) - d) + b + a;
    c1 = (((d - f) - e) - a) + c + b;
    pCv[1].z = c1;
    pCv[0].z = c0;
    c2 = (((f - e) - d) - c) + b + a;
    c3 = (((f - e) - c) - a) + d + b;
    pCv[3].z = c3;
    pCv[2].z = c2;

    pCv[4].x = -pCv[3].x;
    pCv[4].y = -pCv[3].y;
    pCv[4].z = -c3;
    pCv[5].x = -pCv[2].x;
    pCv[5].y = -pCv[2].y;
    pCv[5].z = -c2;
    pCv[6].x = -pCv[1].x;
    pCv[6].y = -pCv[1].y;
    pCv[6].z = -c1;
    pCv[7].x = -pCv[0].x;
    pCv[7].y = -pCv[0].y;
    pCv[7].z = -c0;

    pCv[0].x = pCv[0].x + pVel->x;
    pCv[0].y = pCv[0].y + pVel->y;
    pCv[0].z = pCv[0].z + pVel->z;
    pCv[1].x = pCv[1].x + pVel->x;
    pCv[1].y = pCv[1].y + pVel->y;
    pCv[1].z = pCv[1].z + pVel->z;
    pCv[2].x = pCv[2].x + pVel->x;
    pCv[2].y = pCv[2].y + pVel->y;
    pCv[2].z = pCv[2].z + pVel->z;
    pCv[3].x = pCv[3].x + pVel->x;
    pCv[3].y = pCv[3].y + pVel->y;
    pCv[3].z = pCv[3].z + pVel->z;
    pCv[4].x = pCv[4].x + pVel->x;
    pCv[4].y = pCv[4].y + pVel->y;
    pCv[4].z = pCv[4].z + pVel->z;
    pCv[5].x = pCv[5].x + pVel->x;
    pCv[5].y = pCv[5].y + pVel->y;
    pCv[5].z = pCv[5].z + pVel->z;
    pCv[6].x = pCv[6].x + pVel->x;
    pCv[6].y = pCv[6].y + pVel->y;
    pCv[6].z = pCv[6].z + pVel->z;
    pCv[7].x = pCv[7].x + pVel->x;
    pCv[7].y = pCv[7].y + pVel->y;
    pCv[7].z = pCv[7].z + pVel->z;

}

// Corner i of the box with its y replaced by the ground height under it.
#define GROUND_CORNER(v, i)                                                         \
    v.x = g_pCurrentCar->corners[i].x;                                              \
    v.y = g_pCurrentCar->cornerHeight[i];                                           \
    v.z = g_pCurrentCar->corners[i].z;

// Picks the box face closest to horizontal and rebuilds the ground normal
// from the two triangles formed by its four corners projected onto the ground.
// FUNCTION: CMR2 0x0042de20
void Car_UpdateGroundNormal(void)
{
    int absRight = FIX_ABS(g_pCurrentCar->right.y);
    int absUp = FIX_ABS(g_pCurrentCar->up.y);
    int absForward = FIX_ABS(g_pCurrentCar->forward.y);
    FixVector a;
    FixVector b;
    FixVector c;
    FixVector d;
    FixVector ab;
    FixVector cb;
    FixVector cd;
    FixVector ad;
    FixVector n;

    if (absUp >= absRight && absUp >= absForward) {
        b.x = g_pCurrentCar->corners[0].x;
        a.x = g_pCurrentCar->corners[1].x;
        c.x = g_pCurrentCar->corners[2].x;
        d.x = g_pCurrentCar->corners[3].x;
        b.y = g_pCurrentCar->cornerHeight[0];
        a.y = g_pCurrentCar->cornerHeight[1];
        c.y = g_pCurrentCar->cornerHeight[2];
        d.y = g_pCurrentCar->cornerHeight[3];
        b.z = g_pCurrentCar->corners[0].z;
        a.z = g_pCurrentCar->corners[1].z;
        c.z = g_pCurrentCar->corners[2].z;
        d.z = g_pCurrentCar->corners[3].z;
    } else if (absRight >= absUp && absRight >= absForward) {
        b.x = g_pCurrentCar->corners[0].x;
        a.x = g_pCurrentCar->corners[1].x;
        c.x = g_pCurrentCar->corners[4].x;
        d.x = g_pCurrentCar->corners[5].x;
        b.y = g_pCurrentCar->cornerHeight[0];
        a.y = g_pCurrentCar->cornerHeight[1];
        c.y = g_pCurrentCar->cornerHeight[4];
        d.y = g_pCurrentCar->cornerHeight[5];
        b.z = g_pCurrentCar->corners[0].z;
        a.z = g_pCurrentCar->corners[1].z;
        c.z = g_pCurrentCar->corners[4].z;
        d.z = g_pCurrentCar->corners[5].z;
    } else {
        b.x = g_pCurrentCar->corners[0].x;
        a.x = g_pCurrentCar->corners[2].x;
        c.x = g_pCurrentCar->corners[4].x;
        d.x = g_pCurrentCar->corners[6].x;
        b.y = g_pCurrentCar->cornerHeight[0];
        a.y = g_pCurrentCar->cornerHeight[2];
        c.y = g_pCurrentCar->cornerHeight[4];
        d.y = g_pCurrentCar->cornerHeight[6];
        b.z = g_pCurrentCar->corners[0].z;
        a.z = g_pCurrentCar->corners[2].z;
        c.z = g_pCurrentCar->corners[4].z;
        d.z = g_pCurrentCar->corners[6].z;
    }

    ab.x = a.x - b.x;
    ab.y = a.y - b.y;
    ab.z = a.z - b.z;
    cb.x = c.x - b.x;
    cb.y = c.y - b.y;
    cb.z = c.z - b.z;
    FixVecCross(&n, &cb, &ab);
    if (n.y < 0) {
        n.x = -n.x;
        n.y = -n.y;
        n.z = -n.z;
    }
    {
        FixVector *pOut = &g_pCurrentCar->groundNormal;
        int len = FixVecLength(&n);
        if (len == 0) {
            pOut->x = 0;
            pOut->y = 0;
            pOut->z = 0;
        } else {
            FixVecScaleRecip(pOut, &n, len);
        }
    }

    ad.x = a.x - d.x;
    ad.y = a.y - d.y;
    ad.z = a.z - d.z;
    cd.x = c.x - d.x;
    cd.y = c.y - d.y;
    cd.z = c.z - d.z;
    FixVecCross(&n, &cd, &ad);
    if (n.y < 0) {
        n.x = -n.x;
        n.y = -n.y;
        n.z = -n.z;
    }
    FIX_NORMALIZE_INTO(n, n);

    {
        FixVector *pOut = &g_pCurrentCar->groundNormal;
        n.x += pOut->x;
        n.y += pOut->y;
        n.z += pOut->z;
        {
            int len = FixVecLength(&n);
            if (len == 0) {
                pOut->x = 0;
                pOut->y = 0;
                pOut->z = 0;
            } else {
                FixVecScaleRecip(pOut, &n, len);
            }
        }
    }
}

FixVector g_leanDamping;
FixVector g_leanAccel;
FixVector g_leanDelta;
FixVector g_carAccel;
FixBasis g_leanBasis;

int g_physicsScale = 0x10000;

// Integrates the body lean (the chassis pitching/rolling against its own
// acceleration) and rebuilds the body matrix from the two lean angles.
// match 85%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00443250
void Car_UpdateBodyLean(void)
{
    int len;
    int maxStep;
    int angleZ;
    int angleX;
    int sinZ;
    int cosZ;
    int sinX;
    int cosX;
    int z;

    g_carAccel.x = g_pCurrentCar->velocityNext.x - g_pCurrentCar->velocity.x;
    g_carAccel.y = g_pCurrentCar->velocityNext.y - g_pCurrentCar->velocity.y;
    g_carAccel.z = g_pCurrentCar->velocityNext.z - g_pCurrentCar->velocity.z;
    FixMatrix_InverseRotateVector(&g_leanAccel, &g_carAccel, g_pCurrentCar->pWorld);
    if (FixVecLength(&g_leanAccel) == 0 || g_pCurrentCar->field_0xb60 != 0) {
        g_leanAccel.x = 0;
        g_leanAccel.y = 0;
        g_leanAccel.z = 0;
    }
    FixVecScale(&g_leanAccel, &g_leanAccel, g_physicsScale);
    g_leanAccel.z = g_leanAccel.z * 2;

    len = FixVecLength(&g_pCurrentCar->lean);
    if (len > 0) {
        int damping = -FixMul(len, g_pCurrentCar->field_0x9bc);
        FixVecScaleRecip(&g_leanDamping, &g_pCurrentCar->lean, len);
        FixVecScale(&g_leanDamping, &g_leanDamping, damping);
    } else {
        g_leanDamping.x = 0;
        g_leanDamping.y = 0;
        g_leanDamping.z = 0;
    }

    g_leanDelta.x = g_leanDamping.x + g_leanAccel.x;
    g_leanDelta.y = g_leanDamping.y + g_leanAccel.y;
    g_leanDelta.z = g_leanDamping.z + g_leanAccel.z;
    FixVecScale(&g_leanDelta, &g_leanDelta, FixMul(g_physicsTimeStep, 0x13333));
    g_leanDelta.y = 0;
    len = FixVecLength(&g_leanDelta);
    maxStep = FixMul(0x312, g_physicsTimeStep);
    if (len > maxStep) {
        FixVecScaleRecip(&g_leanDelta, &g_leanDelta, len);
        FixVecScale(&g_leanDelta, &g_leanDelta, FixMul(0x312, g_physicsTimeStep));
    }

    g_pCurrentCar->lean.x += g_leanDelta.x;
    g_pCurrentCar->lean.y += g_leanDelta.y;
    g_pCurrentCar->lean.z += g_leanDelta.z;
    {
        FixVector *pLean = &g_pCurrentCar->lean;
        g_leanAccel = *pLean;
    }

    if (FIX_ABS(g_leanAccel.x) > 0x170a) {
        g_leanAccel.x = g_leanAccel.x > 0 ? 0x170a : -0x170a;
    }
    z = g_leanAccel.z;
    if (FIX_ABS(z) > 0x170a) {
        if (z > 0) {
            z = 0x170a;
        } else {
            z = -0x170a;
        }
        g_leanAccel.z = z;
    }
    if (z < 0) {
        z = -z;
    }

    angleZ = FixAtan2(z, FixMul(0x10000, 0x10000));
    if (g_leanAccel.z < 0) {
        angleZ = -angleZ;
    }
    cosZ = g_sinTable[(angleZ + 0x400) & 0xfff];
    sinZ = g_sinTable[angleZ & 0xfff];

    angleX = FixAtan2(FIX_ABS(g_leanAccel.x), FixMul(0x10000, 0x10000));
    if (g_leanAccel.x < 0) {
        angleX = -angleX;
    }
    sinX = g_sinTable[angleX & 0xfff];
    cosX = g_sinTable[(angleX + 0x400) & 0xfff];

    g_leanBasis.right.z = 0;
    g_leanBasis.right.x = cosX;
    g_leanBasis.right.y = -sinX;
    g_leanBasis.up.x = FixMul(cosZ, sinX);
    g_leanBasis.up.y = FixMul(cosZ, cosX);
    g_leanBasis.up.z = sinZ;
    g_leanBasis.forward.x = FixMul(-sinZ, sinX);
    g_leanBasis.forward.y = FixMul(-sinZ, cosX);
    g_leanBasis.forward.z = cosZ;

    g_pCurrentCar->pBodyMatrix->right = g_leanBasis.right;
    g_pCurrentCar->pBodyMatrix->up = g_leanBasis.up;
    g_pCurrentCar->pBodyMatrix->forward = g_leanBasis.forward;
}

// Evens out the two wheels of an axle once they drift too far apart.
// FUNCTION: CMR2 0x0043af70
void Car_BalanceWheelPairs(void)
{
    int diff = g_pCurrentCar->wheelLoad[0] - g_pCurrentCar->wheelLoad[1];
    if (diff < 0) {
        diff = -diff;
    }
    if (diff > 0x1999) {
        int sum = g_pCurrentCar->wheelLoad[1] + g_pCurrentCar->wheelLoad[0];
        int avg = FixMul(sum, 0x8000);
        g_pCurrentCar->wheelLoad[1] = avg;
        g_pCurrentCar->wheelLoad[0] = avg;
    }
    diff = g_pCurrentCar->wheelLoad[2] - g_pCurrentCar->wheelLoad[3];
    if (diff < 0) {
        diff = -diff;
    }
    if (diff > 0x1999) {
        int sum = g_pCurrentCar->wheelLoad[3] + g_pCurrentCar->wheelLoad[2];
        int avg = FixMul(sum, 0x8000);
        g_pCurrentCar->wheelLoad[3] = avg;
        g_pCurrentCar->wheelLoad[2] = avg;
    }
}

// arccos as a 12-bit angle: 4096 entries for a dot product in [-1, 1]
// GLOBAL: CMR2 0x006e6ef4
short g_acosTable[4096];

// Builds the sideways (friction) force at every corner that is sliding against
// its contact normal, sharing the given grip between the sliding corners.
// FUNCTION: CMR2 0x0043a920
void Car_ApplyCornerFriction(int grip)
{
    int sliding[8];
    int strength[8];
    FixVector dir;
    int count = 0;
    int cornerCount;
    int i;

    if (g_pCurrentCar->field_0xc00 == 0) {
        cornerCount = 4;
    } else {
        cornerCount = 8;
        if (g_pCurrentCar->field_0xb34 > 0) {
            grip = FixDiv(g_pCurrentCar->field_0x75c, 0x80000);
        }
    }

    i = 0;
    if (cornerCount > 0) {
        do {
            sliding[i] = 0;
            if ((g_pCurrentCar->field_0xc00 == 0 && g_pCurrentCar->field_0xbac[i] != 0) ||
                (g_pCurrentCar->field_0xc00 != 0 && g_pCurrentCar->cornerFlags[i] == 0)) {
                int d = FixVecDot(&g_pCurrentCar->cornerAxis[i], &g_pCurrentCar->cornerNormal[i]);
                if (d >= 0x10000) {
                    strength[i] = 0;
                } else {
                    strength[i] = (0x400 - FixAcos(d)) * 0x1680;
                }
                if (strength[i] > 0xa0000 &&
                    g_pCurrentCar->cornerAxis[i].y < g_pCurrentCar->cornerNormal[i].y) {
                    int along = FixVecDot(&g_pCurrentCar->cornerNormal[i], &g_pCurrentCar->cornerVelocity[i]);
                    FixVecScale(&dir, &g_pCurrentCar->cornerNormal[i], along);
                    dir.x = g_pCurrentCar->cornerVelocity[i].x - dir.x;
                    dir.y = g_pCurrentCar->cornerVelocity[i].y - dir.y;
                    dir.z = g_pCurrentCar->cornerVelocity[i].z - dir.z;
                    if (FixVecDot(&dir, &g_pCurrentCar->cornerAxis[i]) < 0) {
                        sliding[i] = 1;
                        count++;
                    }
                }
            }
            i++;
        } while (i < cornerCount);

        if (count > 0) {
            int share = FixDiv(0x140000, count << 16);
            i = 0;
            do {
                if (sliding[i] != 0) {
                    int along;
                    int len;
                    int s;

                    along = FixVecDot(&g_pCurrentCar->cornerAxis[i], &g_pCurrentCar->cornerNormal[i]);
                    FixVecScale(&dir, &g_pCurrentCar->cornerNormal[i], along);
                    dir.x = g_pCurrentCar->cornerAxis[i].x - dir.x;
                    dir.y = g_pCurrentCar->cornerAxis[i].y - dir.y;
                    dir.z = g_pCurrentCar->cornerAxis[i].z - dir.z;
                    len = FixVecLength(&dir);
                    if (len == 0) {
                        dir.x = 0;
                        dir.y = 0;
                        dir.z = 0;
                    } else {
                        FixVecScaleRecip(&dir, &dir, len);
                    }
                    along = FixVecDot(&dir, &g_pCurrentCar->cornerVelocity[i]);
                    if (FIX_ABS(along) > 0x10000) {
                        if (along < 0) {
                            dir.x = -dir.x;
                            dir.y = -dir.y;
                            dir.z = -dir.z;
                        }
                    } else {
                        FixVecScale(&dir, &dir, along);
                    }
                    strength[i] = strength[i] - 0xa0000;
                    s = FixMul(strength[i], 0x51e);
                    if (s > 0x10000) {
                        s = 0x10000;
                    }
                    s = -FixMul(s, FixMul(grip, share));
                    FixVecScale(&dir, &dir, s);
                    g_pCurrentCar->cornerForce[i].x += dir.x;
                    g_pCurrentCar->cornerForce[i].y += dir.y;
                    g_pCurrentCar->cornerForce[i].z += dir.z;
                }
                i++;
            } while (i < cornerCount);
        }
    }
}

extern BYTE *g_pCarSetup;

// Scratch values of the tyre model (Car_UpdateTyreForces).
// GLOBAL: CMR2 0x0053c9b4
int g_tyreGripScale;
// GLOBAL: CMR2 0x0053c9b8
FixVector g_tyreLatForce;
// GLOBAL: CMR2 0x0053c9d8
FixVector g_carStepAccel;
// GLOBAL: CMR2 0x0053c9e8
FixVector g_tyreLongForce;
// GLOBAL: CMR2 0x0053ca4c
int g_tyreSlipScale;
// GLOBAL: CMR2 0x0053ca88
FixVector g_tyreForce;
// GLOBAL: CMR2 0x0053ca94
int g_wheelSpinStep;
// GLOBAL: CMR2 0x0053cae0
int g_tyreForceMax;
// GLOBAL: CMR2 0x0053cc20
int g_tyreForceRecip;

#define FIX_SQR(x) FixMul((x), (x))

inline int FixVecNormalizeLen(FixVector *pOut, FixVector *pV)
{
    int len;

    len = FixVecLength(pV);
    if (len == 0) {
        pOut->x = 0;
        pOut->y = 0;
        pOut->z = 0;
    } else {
        FixVecScaleRecip(pOut, pV, len);
    }
    return len;
}

// Brakes a spinning wheel: the brake of its axle plus its rolling resistance.
#define WHEEL_BRAKE(i, resist)                                                              \
    if (g_pCurrentCar->wheelLoad[i] >= 1) {                                                 \
        g_pCurrentCar->wheelTorque[i] -= (i) > 1 ? brakeRear : brakeFront;                  \
        g_pCurrentCar->wheelTorque[i] = g_pCurrentCar->wheelTorque[i] - (resist);           \
    } else if (g_pCurrentCar->wheelLoad[i] < 0) {                                           \
        g_pCurrentCar->wheelTorque[i] += (i) > 1 ? brakeRear : brakeFront;                  \
        g_pCurrentCar->wheelTorque[i] = g_pCurrentCar->wheelTorque[i] + (resist);           \
    }

// Integrates the wheel spin; an undriven wheel (or one against the gear)
// stops instead of reversing.
#define WHEEL_SPIN(i)                                                                       \
    old = g_pCurrentCar->wheelLoad[i];                                                      \
    g_pCurrentCar->wheelLoad[i] += FixMul(g_pCurrentCar->wheelTorque[i], g_wheelSpinStep);  \
    if (g_pCurrentCar->field_0xb1e == 0 || g_pCurrentCar->field_0xb84 != 0 ||               \
        g_pCurrentCar->field_0x7b4 == ((i) > 1 ? 0x10000 : 0)) {                            \
        if ((old >= 1 && g_pCurrentCar->wheelLoad[i] < 0) ||                               \
            (old < 0 && g_pCurrentCar->wheelLoad[i] > 0))                                   \
            g_pCurrentCar->wheelLoad[i] = 0;                                                \
    } else {                                                                                \
        gear = g_pCurrentCar->field_0x7bc[g_pCurrentCar->field_0xb1e];                      \
        if ((gear > 0 && g_pCurrentCar->wheelLoad[i] < 0) ||                                \
            (gear < 0 && g_pCurrentCar->wheelLoad[i] > 0))                                  \
            g_pCurrentCar->wheelLoad[i] = 0;                                                \
    }

// Longitudinal and lateral force scaled down to at most 1.0 before being
// combined; the length is restored afterwards.
#define TYRE_FORCE_SCALED(i)                                                                \
    g_tyreForceRecip = FixDiv(0x10000, g_tyreForceMax);                                     \
    FixVecScale(&g_tyreLongForce, &dir[i], FixMul(longForce, g_tyreForceRecip));            \
    FixVecScale(&g_tyreLatForce, &axis[i], FixMul(latForce, g_tyreForceRecip));             \
    g_tyreForce.x = g_tyreLatForce.x + g_tyreLongForce.x;                                   \
    g_tyreForce.y = g_tyreLatForce.y + g_tyreLongForce.y;                                   \
    g_tyreForce.z = g_tyreLatForce.z + g_tyreLongForce.z;                                   \
    mag = FixMul(FIX_SQR(FixVecNormalizeLen(&g_pCurrentCar->cornerForce[i], &g_tyreForce)), g_tyreForceMax)

// Sliding friction of the box corners without a wheel: the part of each corner
// velocity along the ground plane is opposed by a force proportional to it (plus
// the step acceleration while almost stopped), clamped to the corner grip.
// match 80%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00438460
void Car_UpdateCornerFriction(void)
{
    int i;
    int d;
    int len;
    int force;
    FixVector tangent;
    FixVector dir;

    if (g_pCurrentCar->field_0xb74 != 0)
        return;
    for (i = 7; i >= 0; i--) {
        if (g_pCurrentCar->cornerFlags[i] != 0)
            continue;
        d = FixVecDot(&g_pCurrentCar->cornerVelocity[i], &g_pCurrentCar->groundNormal);
        FixVecScale(&tangent, &g_pCurrentCar->groundNormal, d);
        tangent.x = g_pCurrentCar->cornerVelocity[i].x - tangent.x;
        tangent.y = g_pCurrentCar->cornerVelocity[i].y - tangent.y;
        tangent.z = g_pCurrentCar->cornerVelocity[i].z - tangent.z;
        len = FixVecLength(&tangent);
        if (len > 0) {
            FixVecScaleRecip(&dir, &tangent, len);
            force = -FixMul(len, g_pCurrentCar->cornerMass);
            if (len < 0x10000)
                force -= FixVecLength(&g_carStepAccel);
            if (abs(force) > g_pCurrentCar->cornerGripB[i]) {
                if (force > 0)
                    force = g_pCurrentCar->cornerGripA[i];
                else
                    force = -g_pCurrentCar->cornerGripA[i];
            }
            FixVecScale(&g_pCurrentCar->cornerForce[i], &dir, force);
        }
    }
}

// Tyre model of a car on the ground: spins the wheels up and down with the
// drive torque, brakes and handbrake, then turns the rolling and lateral slip
// of each wheel into a force at its corner, capped by a friction ellipse.
// With the car in the air (or without tyre contact) only the wheel spin is
// integrated.
// match 63%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004387a0
void Car_UpdateTyreForces(void)
{
    FixVector dir[4];
    FixVector axis[4];
    FixVector v;
    FixVector *pNormal;
    int dirValid[4];
    int axisValid[4];
    int rollSpeed[4];
    int brakeFront;
    int brakeRear;
    int longForce;
    int latForce;
    int longAssist;
    int latAssist;
    int resist;
    int speed;
    int slip;
    int crossing;
    int grip;
    int mag;
    int limA;
    int limB;
    int a;
    int b;
    int d;
    int e;
    int s;
    int old;
    int gear;
    int len;
    short angle;
    int i;

    dirValid[0] = 1;
    dirValid[1] = 1;
    dirValid[2] = 1;
    dirValid[3] = 1;
    axisValid[0] = 1;
    axisValid[1] = 1;
    axisValid[2] = 1;
    axisValid[3] = 1;
    pNormal = &g_pCurrentCar->groundNormal;
    latForce = 0;
    longForce = 0;
    latAssist = 0;
    longAssist = 0;
    if (g_pCurrentCar->flag0x1d0[2] == 0 && g_pCurrentCar->flag0x1d0[3] == 0 &&
        g_pCurrentCar->field_0xb1e != 0) {
        if (g_pCurrentCar->speed < 0x1999) {
            brakeFront = FixMul(0x1999, g_pCurrentCar->field_0x830);
            brakeRear = FixMul(0x1999, 0x10000 - g_pCurrentCar->field_0x830);
        } else {
            brakeFront = 0;
            brakeRear = 0;
        }
    } else {
        brakeFront = FixMul(FixMul(g_pCurrentCar->field_0x838, *(int *)(g_pCarSetup + 0x3fc)),
                            g_pCurrentCar->field_0x830);
        brakeRear = FixMul(FixMul(g_pCurrentCar->field_0x838, *(int *)(g_pCarSetup + 0x400)),
                           0x10000 - g_pCurrentCar->field_0x830);
    }
    if (g_pCurrentCar->field_0x1d8 != 0)
        brakeRear += g_pCurrentCar->field_0x848;

    if (g_pCurrentCar->field_0xb28 == 0 || g_pCurrentCar->field_0xb74 == 0) {
        g_wheelSpinStep = FixMul(g_physicsTimeStep, 0xa0000);
        for (i = 3; i >= 0; i--) {
            resist = *(int *)(g_pCarSetup + 0x3ec + i * 4);
            WHEEL_BRAKE(i, resist);
            WHEEL_SPIN(i);
        }
        Car_BalanceWheelPairs();
        return;
    }

    // Rolling directions and lateral axes of the front and rear wheels,
    // flattened onto the ground.
    d = -FixVecDot(&g_pCurrentCar->wheelDirRear, &g_pCurrentCar->groundNormal);
    v.x = g_pCurrentCar->wheelDirRear.x + FixMul(pNormal->x, d);
    v.y = g_pCurrentCar->wheelDirRear.y + FixMul(g_pCurrentCar->groundNormal.y, d);
    v.z = g_pCurrentCar->wheelDirRear.z + FixMul(g_pCurrentCar->groundNormal.z, d);
    len = FixVecNormalizeLen(&dir[0], &v);
    dir[1] = dir[0];
    dirValid[1] = len != 0;
    dirValid[0] = len != 0;
    d = -FixVecDot(&g_pCurrentCar->wheelAxisRear, &g_pCurrentCar->groundNormal);
    v.x = g_pCurrentCar->wheelAxisRear.x + FixMul(pNormal->x, d);
    v.y = g_pCurrentCar->wheelAxisRear.y + FixMul(g_pCurrentCar->groundNormal.y, d);
    v.z = g_pCurrentCar->wheelAxisRear.z + FixMul(g_pCurrentCar->groundNormal.z, d);
    len = FixVecNormalizeLen(&axis[0], &v);
    axis[1] = axis[0];
    axisValid[1] = len != 0;
    axisValid[0] = len != 0;
    d = -FixVecDot(&g_pCurrentCar->wheelDirFront, &g_pCurrentCar->groundNormal);
    v.x = g_pCurrentCar->wheelDirFront.x + FixMul(pNormal->x, d);
    v.y = g_pCurrentCar->wheelDirFront.y + FixMul(g_pCurrentCar->groundNormal.y, d);
    v.z = g_pCurrentCar->wheelDirFront.z + FixMul(g_pCurrentCar->groundNormal.z, d);
    len = FixVecNormalizeLen(&dir[2], &v);
    dir[3] = dir[2];
    dirValid[3] = len != 0;
    dirValid[2] = len != 0;
    FixVecCross(&g_tyreForce, &g_pCurrentCar->wheelDirFront, &g_pCurrentCar->up);
    d = -FixVecDot(&g_tyreForce, pNormal);
    v.x = g_tyreForce.x + FixMul(pNormal->x, d);
    v.y = FixMul(g_pCurrentCar->groundNormal.y, d) + g_tyreForce.y;
    v.z = FixMul(g_pCurrentCar->groundNormal.z, d) + g_tyreForce.z;
    len = FixVecNormalizeLen(&axis[2], &v);
    axis[3] = axis[2];
    axisValid[3] = len != 0;
    axisValid[2] = len != 0;
    g_pCurrentCar->groundDir[0] = dir[0];
    g_pCurrentCar->groundDir[1] = dir[2];
    g_pCurrentCar->groundAxis[0] = axis[0];
    g_pCurrentCar->groundAxis[1] = axis[2];

    g_tyreGripScale = FixMul(g_pCurrentCar->field_0x8b4, g_physicsScale);
    g_tyreSlipScale = FixMul(g_physicsScale, 0xf000);
    g_wheelSpinStep = FixMul(g_physicsTimeStep, 0x11113);

    // Wheel spin: brakes, then the drive torque limited by the grip.
    for (i = 3; i >= 0; i--) {
        if (g_pCurrentCar->field_0xbac[i] != 0) {
            resist = *(int *)(g_pCarSetup + 0x3ec + i * 4);
            WHEEL_BRAKE(i, resist);
            if (dirValid[i] != 0) {
                speed = FixVecDot(&dir[i], &g_pCurrentCar->cornerVelocity[i]);
                rollSpeed[i] = speed;
                slip = speed * 4 - g_pCurrentCar->wheelLoad[i];
                crossing = FALSE;
                if ((speed > 0 && slip < 0) || (speed < 0 && slip > 0))
                    crossing = TRUE;
                g_pCurrentCar->wheelSlipping[i] = 0;
                if (speed * 4 < 1) {
                    if (g_pCurrentCar->wheelLoad[i] < speed * 4 || g_pCurrentCar->wheelLoad[i] > 0x1999)
                        g_pCurrentCar->wheelSlipping[i] = 1;
                } else if (speed * 4 < g_pCurrentCar->wheelLoad[i] || g_pCurrentCar->wheelLoad[i] < -0x1999) {
                    g_pCurrentCar->wheelSlipping[i] = 1;
                }
                if (i < 2 || g_pCurrentCar->field_0x1d8 == 0 || crossing) {
                    longForce = FixMul(slip, g_tyreSlipScale) * 4;
                    if (g_pCurrentCar->flag0x1d0[2] == 0 && g_pCurrentCar->flag0x1d0[3] == 0) {
                        grip = g_pCurrentCar->field_0xa5c[i];
                    } else {
                        if (slip < 0)
                            slip = -slip;
                        if (FixVecDot(&axis[i], &g_pCurrentCar->cornerVelocity[i]) < 0)
                            d = -FixVecDot(&axis[i], &g_pCurrentCar->cornerVelocity[i]);
                        else
                            d = FixVecDot(&axis[i], &g_pCurrentCar->cornerVelocity[i]);
                        a = FixMul(slip, 0x4000) - d;
                        if (a < 0)
                            a = 0;
                        a = FixMul(a, 0xccc) + 0x51e;
                        if (*(int *)((BYTE *)g_pCurrentCar + 0x90 + i * 0x24) != 0)
                            a += FixMul(0xe666, *(int *)((BYTE *)g_pCurrentCar + 0x90 + i * 0x24));
                        grip = FixMul(g_pCurrentCar->field_0xa5c[i], a);
                    }
                    if (grip < longForce || (grip = -grip, longForce < grip))
                        longForce = grip;
                    if (g_pCurrentCar->flag0x1d0[2] == 0 && g_pCurrentCar->flag0x1d0[3] == 0)
                        slip = longForce / 4;
                    else
                        slip = longForce / 8;
                    if (g_pCurrentCar->field_0xb1f != 0)
                        slip = FixMul(slip, 0x1999);
                    g_pCurrentCar->wheelTorque[i] += slip;
                } else {
                    g_pCurrentCar->wheelTorque[i] = 0;
                    WHEEL_BRAKE(i, resist);
                }
            }
        }
        WHEEL_SPIN(i);
    }
    Car_BalanceWheelPairs();

    // Tyre forces.
    for (i = 3; i >= 0; i--) {
        if (g_pCurrentCar->field_0xbac[i] == 0)
            continue;
        if (dirValid[i] != 0) {
            slip = rollSpeed[i] * 4 - g_pCurrentCar->wheelLoad[i];
            g_pCurrentCar->field_0x870[i] = slip;
            if (FIX_ABS(slip) < 0x10000) {
                d = FixVecDot(&dir[i], &g_carStepAccel);
                longAssist = -d;
                e = FixMul(rollSpeed[i], g_tyreGripScale);
                if ((longAssist > 0 && e > 0) || (longAssist < 0 && e < 0)) {
                    if (FIX_ABS(e) < FIX_ABS(longAssist))
                        longAssist -= e;
                    else
                        longAssist = 0;
                }
                gear = g_pCurrentCar->field_0x7bc[g_pCurrentCar->field_0xb1e];
                if (((gear > 0 && longAssist < 0) || (gear < 0 && longAssist > 0) || gear == 0) &&
                    g_pCurrentCar->field_0x838 == 0)
                    longAssist = 0;
            }
            longForce = -FixMul(FixMul(slip, 0x4000), g_tyreGripScale);
        }
        if (axisValid[i] != 0) {
            speed = FixVecDot(&axis[i], &g_pCurrentCar->cornerVelocity[i]);
            g_pCurrentCar->field_0x880[i] = speed;
            if (FIX_ABS(speed) < 0x10000)
                latAssist = -FixVecDot(&axis[i], &g_carStepAccel);
            latForce = -FixMul(speed, g_tyreGripScale);
        }
        longForce += longAssist;
        latForce += latAssist;
        if (FIX_ABS(longForce) <= 0x10000 || FIX_ABS(longForce) < FIX_ABS(latForce)) {
            if (FIX_ABS(latForce) > 0x10000 && FIX_ABS(longForce) <= FIX_ABS(latForce)) {
                g_tyreForceMax = FIX_ABS(latForce);
                TYRE_FORCE_SCALED(i);
            } else {
                FixVecScale(&g_tyreLongForce, &dir[i], longForce);
                FixVecScale(&g_tyreLatForce, &axis[i], latForce);
                g_tyreForce.x = g_tyreLatForce.x + g_tyreLongForce.x;
                g_tyreForce.y = g_tyreLatForce.y + g_tyreLongForce.y;
                g_tyreForce.z = g_tyreLatForce.z + g_tyreLongForce.z;
                mag = FIX_SQR(FixVecNormalizeLen(&g_pCurrentCar->cornerForce[i], &g_tyreForce));
            }
        } else {
            g_tyreForceMax = FIX_ABS(longForce);
            TYRE_FORCE_SCALED(i);
        }

        // Friction ellipse of the tyre.
        if (longForce < 0)
            longForce = -longForce;
        if (latForce < 0)
            latForce = -latForce;
        if (i < 2) {
            a = *(int *)((BYTE *)g_pCurrentCar + 0x84 + i * 0x24) + *(int *)((BYTE *)g_pCurrentCar + 0x1a8 + i * 0xc);
            b = *(int *)((BYTE *)g_pCurrentCar + 0x8c + i * 0x24) + *(int *)((BYTE *)g_pCurrentCar + 0x1a8 + i * 0xc);
        } else {
            a = *(int *)((BYTE *)g_pCurrentCar + 0x84 + i * 0x24) - 0x4ccc + *(int *)((BYTE *)g_pCurrentCar + 0x1a8 + i * 0xc);
            b = *(int *)((BYTE *)g_pCurrentCar + 0x8c + i * 0x24) - 0x4ccc + *(int *)((BYTE *)g_pCurrentCar + 0x1a8 + i * 0xc);
        }
        if (longForce < 0x6666) {
            limB = FixMul(g_pCurrentCar->field_0xa5c[i], b);
            limA = FixMul(g_pCurrentCar->field_0xa4c[i], a);
        } else if (latForce < 0x6666) {
            limB = g_pCurrentCar->field_0xa5c[i];
            limA = g_pCurrentCar->field_0xa4c[i];
        } else {
            angle = FixAtan2(FixMul(longForce, a), latForce);
            a = FixMul(a, g_sinTable[(angle + 0x400) & 0xfff]);
            s = g_sinTable[angle & 0xfff];
            limA = FixMul(FixSqrt(FixMul(a, a) + FixMul(s, s)), g_pCurrentCar->field_0xa4c[i]);
            angle = FixAtan2(FixMul(longForce, b), latForce);
            b = FixMul(b, g_sinTable[(angle + 0x400) & 0xfff]);
            s = g_sinTable[angle & 0xfff];
            limB = FixMul(FixSqrt(FixMul(s, s) + FixMul(b, b)), g_pCurrentCar->field_0xa5c[i]);
            brakeRear = limA;
        }
        if (limA < mag)
            mag = limB;
        FixVecScale(&g_pCurrentCar->cornerForce[i], &g_pCurrentCar->cornerForce[i], mag);

        // Rolling resistance of the tyre.
        if (*(int *)((BYTE *)g_pCurrentCar + 0x90 + i * 0x24) != 0) {
            d = FixVecDot(&g_pCurrentCar->cornerVelocity[i], &dir[i]);
            if (d != 0) {
                e = -FixMul(d / 4, FixMul(g_pCurrentCar->field_0x8b4, *(int *)((BYTE *)g_pCurrentCar + 0x90 + i * 0x24)));
                FixVecScale(&g_tyreForce, &dir[i], e);
                g_pCurrentCar->cornerForce[i].x += g_tyreForce.x;
                g_pCurrentCar->cornerForce[i].y += g_tyreForce.y;
                g_pCurrentCar->cornerForce[i].z += g_tyreForce.z;
            }
        }
    }
    Car_ApplyCornerFriction(g_tyreGripScale);
}


int FUN_00469bc0(void *pCar, int index);

extern double g_unk0x00511300;

// Orientation of a lean vector: up along (lean.x, 1, lean.z) with the tilt
// limited to about 0.09, right the X axis made perpendicular to it.
#define LEAN_BASIS(lean)                                                     \
    t = (lean);                                                              \
    if (FIX_ABS(t.x) > 0x170a)                                               \
        t.x = t.x < 1 ? -0x170a : 0x170a;                                    \
    if (FIX_ABS(t.z) > 0x170a)                                               \
        t.z = t.z < 1 ? -0x170a : 0x170a;                                    \
    t.y = 0x10000;                                                           \
    FIX_NORMALIZE_INTO(up, t);                                               \
    xAxis.x = 0x10000;                                                       \
    xAxis.y = 0;                                                             \
    xAxis.z = 0;                                                             \
    FixVecScale(&t, &up, FixVecDot(&up, &xAxis));                            \
    t.x = xAxis.x - t.x;                                                     \
    t.y = xAxis.y - t.y;                                                     \
    t.z = xAxis.z - t.z;                                                     \
    FIX_NORMALIZE_INTO(right, t);                                            \
    FixVecCross(&t, &right, &up);                                            \
    FIX_NORMALIZE_INTO(fwd, t);                                              \
    m.right = right;                                                         \
    m.up = up;                                                               \
    m.forward = fwd

// Damping of a lean vector: against it, proportional to its length.
#define LEAN_DAMP(lean, rate)                                                \
    len = FixVecLength(&(lean));                                             \
    if (len > 0) {                                                           \
        k = -FixMul(len, (rate));                                            \
        FixVecScaleRecip(&damp, &(lean), len);                               \
        FixVecScale(&damp, &damp, k);                                        \
    } else {                                                                 \
        damp.x = 0;                                                          \
        damp.y = 0;                                                          \
        damp.z = 0;                                                          \
    }

// Starts the first pending countdown of the current car (200 steps).
// FUNCTION: CMR2 0x0043b020
void FUN_0043b020(void)
{
    if (g_pCurrentCar->field_0xbdc != 0) {
        g_pCurrentCar->field_0xbfc = 1;
        g_pCurrentCar->field_0xb25 = 200;
        return;
    }
    if (g_pCurrentCar->field_0xbe0 != 0) {
        g_pCurrentCar->field_0xbfc = 1;
        g_pCurrentCar->field_0xb26 = 200;
        return;
    }
    if (g_pCurrentCar->field_0xbe4 != 0) {
        g_pCurrentCar->field_0xbfc = 1;
        g_pCurrentCar->field_0xb27 = 200;
    }
}

// Rates of the car scaled by the physics time step.
// FUNCTION: CMR2 0x0043b090
void FUN_0043b090(Car *pCar)
{
    pCar->field_0x790 = FixMul(0xccc, g_physicsTimeStep);
    pCar->field_0x834 = FixMul(0xccc, g_physicsTimeStep);
    pCar->field_0x840 = FixMul(0x1999, g_physicsTimeStep);
    pCar->field_0x820 = FixMul(0x3333, g_physicsTimeStep);
}

// Suspension of the car: two lean vectors (the wheel frame and the body)
// are pushed by the car's acceleration and damped, and the height of each
// corner of the body under the leaned frames gives the suspension travel of
// that corner. The body lean also rolls with the steering on the cars that
// use it.
// match 81%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0043b100
void Car_UpdateSuspension(void)
{
    FixMatrix m;
    FixVector box[4];
    FixVector right;
    FixVector up;
    FixVector fwd;
    FixVector damp;
    FixVector delta;
    FixVector accel;
    FixVector scaled;
    FixVector diff;
    FixVector flat;
    FixVector xAxis;
    FixVector t;
    FixVector *pV;
    int len;
    int k;
    int i;
    short angle;

    if (g_pCurrentCar->field_0xb28 == 0 || g_pCurrentCar->field_0xb74 == 0) {
        accel.x = 0;
        accel.y = 0;
        accel.z = 0;
        scaled.x = 0;
        scaled.y = 0;
        scaled.z = 0;
    } else {
        diff.x = g_pCurrentCar->velocityNext.x - g_pCurrentCar->velocity.x;
        diff.y = g_pCurrentCar->velocityNext.y - g_pCurrentCar->velocity.y;
        diff.z = g_pCurrentCar->velocityNext.z - g_pCurrentCar->velocity.z;
        FixMatrix_InverseRotateVector(&accel, &diff, g_pCurrentCar->pWorld);
        len = FixVecLength(&accel);
        accel.y = 0;
        if (len == 0 || g_pCurrentCar->field_0xb60 != 0) {
            accel.x = 0;
            accel.y = 0;
            accel.z = 0;
        }
        FixVecScale(&scaled, &accel, g_physicsScale);
    }

    // Wheel frame.
    LEAN_DAMP(g_pCurrentCar->wheelLean, g_pCurrentCar->field_0x9b8);
    delta.x = damp.x + accel.x;
    delta.y = damp.y + accel.y;
    delta.z = damp.z + accel.z;
    FixVecScale(&delta, &delta, FixMul(g_physicsTimeStep, 0x13333));
    flat = delta;
    flat.y = 0;
    len = FixVecLength(&flat);
    if (len > 0x312) {
        FixVecScaleRecip(&flat, &flat, len);
        FixVecScale(&flat, &flat, 0x312);
        delta.x = flat.x;
        delta.z = flat.z;
    }
    g_pCurrentCar->wheelLean.x += delta.x;
    g_pCurrentCar->wheelLean.y += delta.y;
    g_pCurrentCar->wheelLean.z += delta.z;
    LEAN_BASIS(g_pCurrentCar->wheelLean);
    box[0].x = g_pCurrentCar->halfExtents.x;
    box[0].y = 0;
    box[0].z = g_pCurrentCar->halfExtents.z;
    box[1].x = g_pCurrentCar->halfExtents.x;
    box[1].y = 0;
    box[1].z = -g_pCurrentCar->halfExtents.z;
    box[2].x = -g_pCurrentCar->halfExtents.x;
    box[2].y = 0;
    box[2].z = g_pCurrentCar->halfExtents.z;
    box[3].x = -g_pCurrentCar->halfExtents.x;
    box[3].y = 0;
    box[3].z = -g_pCurrentCar->halfExtents.z;
    for (i = 0; i < 4; i++) {
        FixMatrix_RotateVector(&t, &box[i], &m);
        g_pCurrentCar->field_0x998[i] = t.y;
        g_pCurrentCar->field_0x998[i] += *(int *)(g_pCarSetup + 0x3dc + i * 4);
    }

    // Body.
    scaled.z = FixMul(scaled.z, 0x20000);
    LEAN_DAMP(g_pCurrentCar->lean, g_pCurrentCar->field_0x9bc);
    delta.x = scaled.x + damp.x;
    delta.y = scaled.y + damp.y;
    delta.z = scaled.z + damp.z;
    FixVecScale(&delta, &delta, FixMul(g_physicsTimeStep, 0x13333));
    flat = delta;
    flat.y = 0;
    len = FixVecLength(&flat);
    if (len > FixMul(0x312, g_physicsTimeStep)) {
        FixVecScaleRecip(&flat, &flat, len);
        FixVecScale(&flat, &flat, FixMul(0x312, g_physicsTimeStep));
        delta.x = flat.x;
        delta.z = flat.z;
    }
    g_pCurrentCar->lean.x += delta.x;
    g_pCurrentCar->lean.y += delta.y;
    g_pCurrentCar->lean.z += delta.z;
    LEAN_BASIS(g_pCurrentCar->lean);
    if (g_pCurrentCar->field_0xb1d != 0 && g_pCurrentCar->field_0x7bc[g_pCurrentCar->field_0xb1e] != 0) {
        t.x = 0;
        t.y = 0;
        t.z = 0x10000;
        angle = (short)((double)FixMul(g_pCurrentCar->field_0xb1d << 16, 0x3333) * g_unk0x00511300);
        FixMatrix_FromAxisAngle(&m, &t, angle);
        pV = &right;
        for (i = 3; i != 0; i--) {
            FixMatrix_RotateVector(&t, pV, &m);
            FIX_NORMALIZE_INTO(pV[0], t);
            pV++;
        }
    }
    m.right = right;
    m.up = up;
    m.forward = fwd;
    for (i = 0; i < 4; i++) {
        FixMatrix_RotateVector(&t, &box[i], &m);
        g_pCurrentCar->wheel0x9a8[i] = t.y;
        g_pCurrentCar->wheel0x9a8[i] += *(int *)(g_pCarSetup + 0x3dc + i * 4);
    }
    if (g_pCurrentCar->field_0xb28 != 4) {
        for (i = 7; i >= 0; i--)
            g_pCurrentCar->field_0x8b8[i] = g_pCurrentCar->field_0x8b4;
        return;
    }
    Car_UpdateWheelTorques();
}

// Normalises the per-wheel slip, turns it into wheel torque and, on the cars
// that use it, feeds the torque back towards half the drive torque.
// match 81%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0043c640
void Car_UpdateWheelTorques(void)
{
    int quarter = g_pCurrentCar->field_0x75c / 4;
    int maxSlip = 0;
    int *pSlip = g_pCurrentCar->field_0x998;
    int i = 4;

    do {
        int v = *pSlip;
        if (FIX_ABS(v) > maxSlip) {
            maxSlip = FIX_ABS(v);
        }
        pSlip++;
        i--;
    } while (i != 0);

    if (maxSlip > 0x4000) {
        int scale = FixDiv(0x4000, maxSlip);
        for (i = 0; i < 4; i++) {
            g_pCurrentCar->field_0x998[i] = FixMul(g_pCurrentCar->field_0x998[i], scale);
        }
    }

    for (i = 0; i < 4; i++) {
        int load = FixMul(g_pCurrentCar->field_0x998[i], 0x40000) + g_pCurrentCar->field_0x808[i];
        g_pCurrentCar->field_0x8b8[i] = quarter - FixMul(g_pCurrentCar->field_0x8d8, load);
    }

    if (FUN_00469bc0(g_pCurrentCar, 3) != 0) {
        int rate = FixMul(g_pCurrentCar->speed, 0x2000);
        int half;

        if (rate > 0x10000) {
            rate = 0x10000;
        }
        half = g_pCurrentCar->field_0x75c / 2;
        for (i = 0; i < 4; i++) {
            int target = i < 2 ? half : 0;
            g_pCurrentCar->field_0x8b8[i] += FixMul(rate, target - g_pCurrentCar->field_0x8b8[i]);
        }
    }
}

// GLOBAL: CMR2 0x0053cc1c
BYTE *g_pCarSetup;
// GLOBAL: CMR2 0x00519c88
int g_dragBase = 0x10000;

// One physics step of the rigid body: sums the corner forces into a force and
// a torque, integrates linear and angular velocity, moves the body and rebuilds
// its world matrix.
// match 73%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00440840
void Car_Integrate(void)
{
    FixVector force;
    FixVector tmp;
    FixVector angStep;
    FixVector torque;
    FixVector posStep;
    FixVector cross;
    FixVector r;
    FixVector accel;
    int locked = 0;
    int off;
    int halfStep;
    int drag;

    if (g_pCurrentCar->angularVelocity.x == 0 && g_pCurrentCar->angularVelocity.z == 0) {
        locked = 1;
    }
    force = g_pCurrentCar->baseForce;
    torque.x = 0;
    torque.y = 0;
    torque.z = 0;

    if (locked != 0) {
        off = 0x66c;
        do {
            FixVector *pForce = (FixVector *)((BYTE *)g_pCurrentCar + off);
            tmp = *pForce;
            force.x = force.x + tmp.x;
            force.y = force.y + tmp.y;
            force.z = force.z + tmp.z;
            force.x = force.x + tmp.x;
            force.y = force.y + tmp.y;
            force.z = force.z + tmp.z;
            torque.y = torque.y +
                       FixMul(FixVecDot(pForce, &g_pCurrentCar->forward),
                              *(int *)((BYTE *)g_pCurrentCar + off - 0x438)) * 2;
            off -= 0x18;
        } while (off >= 0x648);
    } else {
        off = 0x24;
        do {
            FixVector *pForce = (FixVector *)((BYTE *)g_pCurrentCar + off + 0x648);
            tmp = *pForce;
            force.x = force.x + tmp.x;
            force.y = force.y + tmp.y;
            force.z = force.z + tmp.z;
            r.x = *(int *)((BYTE *)g_pCurrentCar + off + 0x270) - g_pCurrentCar->position.x;
            r.y = *(int *)((BYTE *)g_pCurrentCar + off + 0x274) - g_pCurrentCar->position.y;
            r.z = *(int *)((BYTE *)g_pCurrentCar + off + 0x278) - g_pCurrentCar->position.z;
            FixVecCross(&cross, &tmp, &r);
            torque.x = torque.x + cross.x;
            torque.y = torque.y + cross.y;
            torque.z = torque.z + cross.z;
            off -= 0xc;
        } while (off >= 0);
        FixMatrix_InverseRotateVector(&tmp, &torque, g_pCurrentCar->pWorld);
        torque = tmp;
    }

    torque.y = FixMul(torque.y, 0xb333);
    FixVecScale(&force, &force, 0xd999);
    drag = FixMul(FIX_ABS(g_pCurrentCar->speed), *(int *)(g_pCarSetup + 0x408) + g_dragBase);
    FixVecScale(&tmp, &g_pCurrentCar->velocity, drag);
    force.x = force.x - tmp.x;
    force.y = force.y - tmp.y;
    force.z = force.z - tmp.z;

    FixVecScale(&accel, &force, g_pCurrentCar->field_0x760);
    accel.x = accel.x + g_pCurrentCar->field_0x5c4.x;
    accel.y = accel.y + g_pCurrentCar->field_0x5c4.y;
    accel.z = accel.z + g_pCurrentCar->field_0x5c4.z;
    FixVecScale(&accel, &accel, 0x3333);
    g_pCurrentCar->velocityNext = g_pCurrentCar->velocity;
    FixVecScale(&accel, &accel, g_physicsTimeStep);
    g_pCurrentCar->velocity.x = g_pCurrentCar->velocity.x + accel.x;
    g_pCurrentCar->velocity.y = g_pCurrentCar->velocity.y + accel.y;
    g_pCurrentCar->velocity.z = g_pCurrentCar->velocity.z + accel.z;
    g_pCurrentCar->speed = FixVecLength(&g_pCurrentCar->velocity);

    FixVecScale(&posStep, &g_pCurrentCar->velocity, g_physicsTimeStep);
    halfStep = g_physicsTimeStep / 2;
    FixVecScale(&accel, &accel, halfStep);
    posStep.x = posStep.x - accel.x;
    posStep.y = posStep.y - accel.y;
    posStep.z = posStep.z - accel.z;

    if (locked != 0) {
        int angAccelY = -FixMul(torque.y, g_pCurrentCar->inertia.y);

        angAccelY = angAccelY - FixMul(g_pCurrentCar->field_0x5d0.y,
                                       FixMul(g_pCurrentCar->inertia.y, g_pCurrentCar->field_0x75c));
        angAccelY = FixMul(angAccelY, g_physicsTimeStep);
        g_pCurrentCar->angularVelocity.y = g_pCurrentCar->angularVelocity.y + angAccelY;
        angStep.y = FixMul(g_pCurrentCar->angularVelocity.y, g_physicsTimeStep);
        halfStep = g_physicsTimeStep / 2;
        angAccelY = FixMul(angAccelY, halfStep);
        angStep.y = angStep.y - angAccelY;
    } else {
        FixVector angAccel;

        angAccel.x = -FixMul(torque.x, g_pCurrentCar->inertia.x);
        angAccel.y = -FixMul(torque.y, g_pCurrentCar->inertia.y);
        angAccel.z = -FixMul(torque.z, g_pCurrentCar->inertia.z);
        angAccel.x = angAccel.x - FixMul(g_pCurrentCar->field_0x5d0.x,
                                         FixMul(g_pCurrentCar->inertia.x, g_pCurrentCar->field_0x75c));
        angAccel.y = angAccel.y - FixMul(g_pCurrentCar->field_0x5d0.y,
                                         FixMul(g_pCurrentCar->inertia.y, g_pCurrentCar->field_0x75c));
        angAccel.z = angAccel.z - FixMul(g_pCurrentCar->field_0x5d0.z,
                                         FixMul(g_pCurrentCar->inertia.z, g_pCurrentCar->field_0x75c));
        FixVecScale(&angAccel, &angAccel, g_physicsTimeStep);
        g_pCurrentCar->angularVelocity.x = g_pCurrentCar->angularVelocity.x + angAccel.x;
        g_pCurrentCar->angularVelocity.y = g_pCurrentCar->angularVelocity.y + angAccel.y;
        g_pCurrentCar->angularVelocity.z = g_pCurrentCar->angularVelocity.z + angAccel.z;
        FixVecScale(&angStep, &g_pCurrentCar->angularVelocity, g_physicsTimeStep);
        halfStep = g_physicsTimeStep / 2;
        FixVecScale(&angAccel, &angAccel, halfStep);
        angStep.x = angStep.x - angAccel.x;
        angStep.y = angStep.y - angAccel.y;
        angStep.z = angStep.z - angAccel.z;
    }

    if (g_pCurrentCar->field_0xb34 > 0) {
        int upDot = FixVecDot(&g_pCurrentCar->groundNormal, &g_pCurrentCar->up);
        if (FIX_ABS(angStep.x) < 6 && FIX_ABS(upDot) > 0xcccc) {
            angStep.x = 0;
        }
        if ((g_pCurrentCar->flag0x1d0[0] == 0 && g_pCurrentCar->flag0x1d0[1] == 0) ||
            FIX_ABS(g_pCurrentCar->speed) < 0x28f) {
            if (FIX_ABS(angStep.y) < 6) {
                angStep.y = 0;
            }
        }
        if (FIX_ABS(angStep.z) < 6) {
            angStep.z = 0;
        }
        if (g_pCurrentCar->speed < 0xccc) {
            posStep.x = 0;
            posStep.y = 0;
            posStep.z = 0;
        }
    }

    if (locked != 0) {
        angStep.x = 0;
        angStep.z = 0;
    }
    g_pCurrentCar->position.x = g_pCurrentCar->position.x + posStep.x;
    g_pCurrentCar->position.y = g_pCurrentCar->position.y + posStep.y;
    g_pCurrentCar->position.z = g_pCurrentCar->position.z + posStep.z;
    g_pCurrentCar->field_0xb60 = 0;
    if (angStep.x == 0 && angStep.y == 0 && angStep.z == 0) {
        if (g_pCurrentCar->speed < 0x51e) {
            g_pCurrentCar->field_0xb60 = 1;
        }
    } else if (locked == 0) {
        FixBasis_Integrate(&g_pCurrentCar->right, &angStep);
    } else {
        off = 0;
        do {
            FixVector *pAxis = (FixVector *)((BYTE *)g_pCurrentCar + off + 0x360);
            tmp.x = FixMul(angStep.y, pAxis->z);
            tmp.y = 0;
            tmp.z = -FixMul(angStep.y, pAxis->x);
            pAxis->x = pAxis->x + tmp.x;
            pAxis->y = pAxis->y + tmp.y;
            pAxis->z = pAxis->z + tmp.z;
            off += 0x18;
        } while (off < 0x24);
    }

    g_pCurrentCar->pWorld->right = g_pCurrentCar->right;
    g_pCurrentCar->pWorld->up = g_pCurrentCar->up;
    g_pCurrentCar->pWorld->forward = g_pCurrentCar->forward;
    g_pCurrentCar->pWorld->position = g_pCurrentCar->position;
    Car_UpdateCorners(g_pCurrentCar);
}

// Builds the force each wheel puts on the ground: first the drive/brake torque
// per wheel (with the handbrake and the traction limits), then the tyre force
// itself, combining the rolling and the lateral slip and capping it with the
// tyre's friction ellipse. Only the right hand wheels are solved; the left
// hand ones are mirrored from them. See also Car_UpdateTyreForces.
// match 54%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00441500
void Car_UpdateWheelForces(void)
{
    FixVector dirRear;
    FixVector dirFront;
    FixVector axis[4];
    FixVector dir;
    FixVector force;
    int dot[4];
    int brakeA;
    int brakeB;
    int scale;
    int loadScale;
    int torqueScale;
    int torqueLimit;
    int i;

    if (g_pCurrentCar->flag0x1d0[2] == 0 && g_pCurrentCar->flag0x1d0[3] == 0 &&
        g_pCurrentCar->field_0xb1e != 0) {
        brakeA = FixMul(0x1999, g_pCurrentCar->field_0x830);
        brakeB = FixMul(0x1999, 0x10000 - g_pCurrentCar->field_0x830);
    } else {
        brakeA = FixMul(FixMul(g_pCurrentCar->field_0x838, *(int *)(g_pCarSetup + 0x3fc)),
                        g_pCurrentCar->field_0x830);
        brakeB = FixMul(FixMul(g_pCurrentCar->field_0x838, *(int *)(g_pCarSetup + 0x400)),
                        0x10000 - g_pCurrentCar->field_0x830);
    }
    if (g_pCurrentCar->field_0x1d8 != 0) {
        brakeB = brakeB + g_pCurrentCar->field_0x848;
    }

    dirRear = g_pCurrentCar->wheelDirRear;
    axis[0] = g_pCurrentCar->wheelAxisRear;
    dirFront = g_pCurrentCar->wheelDirFront;
    {
        FixVector cross;
        int len;

        FixVecCross(&cross, &g_pCurrentCar->wheelDirFront, &g_pCurrentCar->up);
        len = FixVecLength(&cross);
        if (len == 0) {
            axis[1].x = 0;
            axis[1].y = 0;
            axis[1].z = 0;
        } else {
            FixVecScaleRecip(&axis[1], &cross, len);
        }
    }

    scale = FixMul(g_pCurrentCar->field_0x8b4, g_physicsScale);
    loadScale = FixMul(g_physicsScale, 0xf000);
    torqueScale = FixMul(g_physicsTimeStep, 0x11113);
    torqueLimit = FIX_ABS(FixMul(g_pCurrentCar->field_0x794,
                                 g_pCurrentCar->field_0x7dc[g_pCurrentCar->field_0xb1e]));

    i = 3;
    do {
        int load;
        int slip;
        int target;
        int brake;
        int limited;

        dir = i > 1 ? dirFront : dirRear;
        limited = *(int *)(g_pCarSetup + 0x3ec + i * 4);
        load = g_pCurrentCar->wheelLoad[i];
        if (load >= 1) {
            brake = i > 1 ? brakeB : brakeA;
            g_pCurrentCar->wheelTorque[i] = g_pCurrentCar->wheelTorque[i] - brake;
            g_pCurrentCar->wheelTorque[i] = g_pCurrentCar->wheelTorque[i] - limited;
        } else if (load < 0) {
            brake = i > 1 ? brakeB : brakeA;
            g_pCurrentCar->wheelTorque[i] = g_pCurrentCar->wheelTorque[i] + brake;
            g_pCurrentCar->wheelTorque[i] = g_pCurrentCar->wheelTorque[i] + limited;
        }

        slip = FixVecDot(&dir, &g_pCurrentCar->cornerVelocity[i]);
        load = g_pCurrentCar->wheelLoad[i];
        dot[i] = slip;
        target = slip * 4 - load;
        if (i <= 1 || g_pCurrentCar->field_0x1d8 == 0 ||
            (slip > 0 && target < 0) || (slip < 0 && target > 0)) {
            int grip;
            int limit = FixMul(target, loadScale) * 4;

            if (g_pCurrentCar->flag0x1d0[2] == 0 && g_pCurrentCar->flag0x1d0[3] == 0) {
                grip = g_pCurrentCar->field_0xa5c[i];
            } else {
                int slide = FIX_ABS(FixVecDot(&axis[i], &g_pCurrentCar->cornerVelocity[i]));
                int room = FixMul(FIX_ABS(target), 0x4000) - slide;

                if (room < 0) {
                    room = 0;
                }
                room = FixMul(room, 0xccc) + 0x51e;
                if (*(int *)((BYTE *)g_pCurrentCar + 0x90 + i * 0x24) != 0) {
                    room = room + FixMul(0xe666, *(int *)((BYTE *)g_pCurrentCar + 0x90 + i * 0x24));
                }
                grip = FixMul(g_pCurrentCar->field_0xa5c[i], room);
            }
            if (grip < limit || (grip = -grip, limit < grip)) {
                limit = grip;
            }
            if (g_pCurrentCar->flag0x1d0[2] == 0 && g_pCurrentCar->flag0x1d0[3] == 0) {
                limit = limit / 4;
            } else {
                limit = limit / 8;
            }
            g_pCurrentCar->wheelTorque[i] = g_pCurrentCar->wheelTorque[i] + limit;
        } else {
            g_pCurrentCar->wheelTorque[i] = 0;
            load = g_pCurrentCar->wheelLoad[i];
            if (load >= 1) {
                brake = i > 1 ? brakeB : brakeA;
                g_pCurrentCar->wheelTorque[i] = g_pCurrentCar->wheelTorque[i] - brake;
                g_pCurrentCar->wheelTorque[i] = g_pCurrentCar->wheelTorque[i] - limited;
            } else if (load < 0) {
                brake = i > 1 ? brakeB : brakeA;
                g_pCurrentCar->wheelTorque[i] = g_pCurrentCar->wheelTorque[i] + brake;
                g_pCurrentCar->wheelTorque[i] = g_pCurrentCar->wheelTorque[i] + limited;
            }
        }

        load = g_pCurrentCar->wheelLoad[i];
        g_pCurrentCar->wheelLoad[i] = g_pCurrentCar->wheelLoad[i] +
                                      FixMul(g_pCurrentCar->wheelTorque[i], torqueScale);
        if (torqueLimit < g_pCurrentCar->wheelLoad[i] ||
            (torqueLimit = -torqueLimit, g_pCurrentCar->wheelLoad[i] < torqueLimit)) {
            g_pCurrentCar->wheelLoad[i] = torqueLimit;
        }
        torqueLimit = FIX_ABS(torqueLimit);
        if (g_pCurrentCar->field_0xb1e == 0 || g_pCurrentCar->field_0xb84 != 0 ||
            g_pCurrentCar->field_0x7b4 == (i > 1 ? 0x10000 : 0x0)) {
            if ((load >= 1 && g_pCurrentCar->wheelLoad[i] < 0) ||
                (load < 0 && g_pCurrentCar->wheelLoad[i] > 0)) {
                g_pCurrentCar->wheelLoad[i] = 0;
            }
        } else {
            int gear = g_pCurrentCar->field_0x7bc[g_pCurrentCar->field_0xb1e];

            if ((gear > 0 && g_pCurrentCar->wheelLoad[i] < 0) ||
                (gear < 0 && g_pCurrentCar->wheelLoad[i] > 0)) {
                g_pCurrentCar->wheelLoad[i] = 0;
            }
        }
        i -= 2;
    } while (i >= 0);

    g_pCurrentCar->wheelTorque[0] = g_pCurrentCar->wheelTorque[1];
    g_pCurrentCar->wheelTorque[2] = g_pCurrentCar->wheelTorque[3];
    g_pCurrentCar->wheelLoad[0] = g_pCurrentCar->wheelLoad[1];
    g_pCurrentCar->wheelLoad[2] = g_pCurrentCar->wheelLoad[3];

    i = 3;
    do {
        int susp;
        int lateral;
        int fLong;
        int fLat;
        int len;
        int combined;

        dir = i > 1 ? dirFront : dirRear;
        axis[i] = i > 1 ? axis[1] : axis[0];
        susp = dot[i] * 4 - g_pCurrentCar->wheelLoad[i];
        g_pCurrentCar->field_0x870[i] = susp;
        fLong = -FixMul(FixMul(susp, 0x4000), scale);
        lateral = FixVecDot(&axis[i], &g_pCurrentCar->cornerVelocity[i]);
        g_pCurrentCar->field_0x880[i] = lateral;
        fLat = -FixMul(lateral, scale);

        if (FIX_ABS(fLong) <= 0x10000 || FIX_ABS(fLong) < FIX_ABS(fLat)) {
            if (FIX_ABS(fLat) > 0x10000 && FIX_ABS(fLong) <= FIX_ABS(fLat)) {
                int recip = FixDiv(0x10000, FIX_ABS(fLat));
                int nLong = FixMul(fLong, recip);
                int nLat = FixMul(fLat, recip);

                force.x = FixMul(axis[i].x, nLat) + FixMul(dir.x, nLong);
                force.y = FixMul(axis[i].y, nLat) + FixMul(dir.y, nLong);
                force.z = FixMul(axis[i].z, nLat) + FixMul(dir.z, nLong);
                combined = FixMul(FIX_SQR(FixVecNormalizeLen(&g_pCurrentCar->cornerForce[i], &force)), FIX_ABS(fLat));
            } else {
                force.x = FixMul(axis[i].x, fLat) + FixMul(dir.x, fLong);
                force.y = FixMul(axis[i].y, fLat) + FixMul(dir.y, fLong);
                force.z = FixMul(axis[i].z, fLat) + FixMul(dir.z, fLong);
                combined = FIX_SQR(FixVecNormalizeLen(&g_pCurrentCar->cornerForce[i], &force));
            }
        } else {
            int recip = FixDiv(0x10000, FIX_ABS(fLong));
            int nLong = FixMul(fLong, recip);
            int nLat = FixMul(fLat, recip);

            force.x = FixMul(axis[i].x, nLat) + FixMul(dir.x, nLong);
            force.y = FixMul(axis[i].y, nLat) + FixMul(dir.y, nLong);
            force.z = FixMul(axis[i].z, nLat) + FixMul(dir.z, nLong);
            combined = FixMul(FIX_SQR(FixVecNormalizeLen(&g_pCurrentCar->cornerForce[i], &force)), FIX_ABS(fLong));
        }

        // Friction ellipse of the tyre, wider on the easier grip settings.
        {
            int longAbs = FIX_ABS(fLong);
            int latAbs = FIX_ABS(fLat);
            int extra = *(int *)((BYTE *)g_pCurrentCar + 0x1a8 + i * 0xc);
            int a = *(int *)((BYTE *)g_pCurrentCar + 0x84 + i * 0x24);
            int b;
            int limA;
            int limB;
            int c;
            int s;
            short angle;

            if (i < 2) {
                b = *(int *)((BYTE *)g_pCurrentCar + 0x8c + i * 0x24);
            } else {
                a = a - 0x4ccc;
                b = *(int *)((BYTE *)g_pCurrentCar + 0x8c + i * 0x24) - 0x4ccc;
            }
            a = a + extra;
            b = b + extra;
            if (CGameInfo::FUN_00405d90() == 2) {
                if ((BYTE)RallyData_FUN_00406940() == 0 && (BYTE)RallyData_FUN_00406950() == 1) {
                    b = FixMul(b, 0x18000);
                    a = FixMul(a, 0x18000);
                }
                if ((BYTE)RallyData_FUN_00406940() == 0 && (BYTE)RallyData_FUN_00406950() == 2) {
                    b = FixMul(b, 0x13333);
                    a = FixMul(a, 0x13333);
                }
            }
            if (longAbs < 0x6666) {
                limB = FixMul(g_pCurrentCar->field_0xa5c[i], b);
                limA = FixMul(g_pCurrentCar->field_0xa4c[i], a);
            } else if (latAbs < 0x6666) {
                limB = g_pCurrentCar->field_0xa5c[i];
                limA = g_pCurrentCar->field_0xa4c[i];
            } else {
                angle = FixAtan2(FixMul(longAbs, a), latAbs);
                c = FixMul(a, g_sinTable[(angle + 0x400) & 0xfff]);
                s = g_sinTable[angle & 0xfff];
                limA = FixMul(FixSqrt(FixMul(c, c) + FixMul(s, s)), g_pCurrentCar->field_0xa4c[i]);
                angle = FixAtan2(FixMul(longAbs, b), latAbs);
                c = FixMul(b, g_sinTable[(angle + 0x400) & 0xfff]);
                s = g_sinTable[angle & 0xfff];
                limB = FixMul(FixSqrt(FixMul(s, s) + FixMul(c, c)), g_pCurrentCar->field_0xa5c[i]);
            }
            if (limA < combined) {
                combined = limB;
            }
            FixVecScale(&g_pCurrentCar->cornerForce[i], &g_pCurrentCar->cornerForce[i], combined);
        }

        if (*(int *)((BYTE *)g_pCurrentCar + 0x90 + i * 0x24) != 0) {
            int along = FixVecDot(&g_pCurrentCar->cornerVelocity[i], &dir);

            if (along != 0) {
                int drag = -FixMul(along / 4,
                                   FixMul(g_pCurrentCar->field_0x8b4,
                                          *(int *)((BYTE *)g_pCurrentCar + 0x90 + i * 0x24)));

                g_pCurrentCar->cornerForce[i].x = g_pCurrentCar->cornerForce[i].x + FixMul(dir.x, drag);
                g_pCurrentCar->cornerForce[i].y = g_pCurrentCar->cornerForce[i].y + FixMul(dir.y, drag);
                g_pCurrentCar->cornerForce[i].z = g_pCurrentCar->cornerForce[i].z + FixMul(dir.z, drag);
            }
        }
        i -= 2;
    } while (i >= 0);

    g_pCurrentCar->field_0x870[0] = g_pCurrentCar->field_0x870[1];
    g_pCurrentCar->field_0x870[2] = g_pCurrentCar->field_0x870[3];
    g_pCurrentCar->cornerForce[0] = g_pCurrentCar->cornerForce[1];
    g_pCurrentCar->cornerForce[2] = g_pCurrentCar->cornerForce[3];
}

// GLOBAL: CMR2 0x0053a3a8
BYTE g_unk0x0053a3a8[8][0xfc];
// GLOBAL: CMR2 0x0053acc8
BYTE g_unk0x0053acc8[16];
// GLOBAL: CMR2 0x0053acf0
int g_unk0x0053acf0[16];
// GLOBAL: CMR2 0x0053c5a0
BYTE g_unk0x0053c5a0[10][0x60];
// GLOBAL: CMR2 0x0053c9a8
int g_unk0x0053c9a8;

struct CarShortValues {
    short a, b, c, d;
};
// GLOBAL: CMR2 0x0053a230
CarShortValues g_unk0x0053a230[8];

// FUNCTION: CMR2 0x0042b5b0
void FUN_0042b5b0(int first, int count)
{
    int n = first + count;

    if (first < n) {
        short *p = &g_unk0x0053a230[first].b;

        n -= first;
        do {
            p[-1] = 0;
            p[0] = 0;
            p[1] = 0;
            p[2] = 0;
            p += 4;
            n--;
        } while (n != 0);
    }
}

void FUN_004b23b0(float value);

// FUNCTION: CMR2 0x0042b7e0
void FUN_0042b7e0(void)
{
    FUN_004b23b0(100.0f);
    g_unk0x0053c9a8 = 6;
}

unsigned short FUN_0040bbc0(unsigned short);
void FUN_0042b720(int, char);
void FUN_0042bcd0(void);

// GLOBAL: CMR2 0x0053a314
short g_unk0x0053a314[8];
// GLOBAL: CMR2 0x0053bd6c
short g_unk0x0053bd6c[26];
// GLOBAL: CMR2 0x0053c9a0
short g_unk0x0053c9a0;

// Builds the race car order for the given number of cars and clears the
// per-car "drawn" flags.
// match 80%: below the 90% bar; the logic is complete, MSVC just keeps the first loop counter in ESI instead of EAX
// FUNCTION: CMR2 0x0042b660
void FUN_0042b660(int count)
{
    int i;
    int j;

    for (i = 0; i < count; i++) {
        g_carOrder[i] = i;
        g_unk0x0053a314[i] = i;
        g_unk0x0053bd6c[i] = i;
    }
    *(int *)&g_unk0x0053acc8[0] = 0;
    g_carOrderCount = count;
    g_unk0x0053c9a0 = count;
    g_carOrder[44] = count;
    g_carOrder[25] = count;
    *(int *)&g_unk0x0053acc8[4] = 0;
    for (j = 0; j < 2; j++) {
        g_unk0x0053acc8[j] = FUN_0040bbc0(j);
        if (!CGameInfo::FUN_00405e00() && j >= (int)(BYTE)RallyDataState())
            FUN_0042b720(j, -1);
    }
    FUN_0042bcd0();
}

// FUNCTION: CMR2 0x0042b6f0
short *Car_GetOrder(void)
{
    return g_carOrder;
}

// FUNCTION: CMR2 0x0042b700
short Car_GetOrderCount(void)
{
    return g_carOrderCount;
}

// FUNCTION: CMR2 0x0042b710
BYTE FUN_0042b710(int index)
{
    return g_unk0x0053acc8[index];
}

// FUNCTION: CMR2 0x0042b720
void FUN_0042b720(int index, char value)
{
    g_unk0x0053acc8[index] = value;
}

// Counts down field 0xb43 of every car in the list.
// FUNCTION: CMR2 0x0042bc80
void FUN_0042bc80(short *pList, short count)
{
    int i;
    Car *pCar;

    for (i = count - 1; i >= 0; i--) {
        pCar = &g_carBuffer[pList[i]];
        if (pCar->field_0xb43[0] != 0)
            pCar->field_0xb43[0]--;
    }
}

// Copies a car's render transforms (body matrices, wheel records, ground data).
// FUNCTION: CMR2 0x0042c7b0
void FUN_0042c7b0(CarTransforms *pDst, CarTransforms *pSrc)
{
    int i;

    FixMatrix_CopyRotation(&pSrc->body2, &pDst->body2);
    FixMatrix_CopyRotation(&pSrc->body, &pDst->body);
    for (i = 0; i < 4; i++) {
        memcpy(pDst->field_0x80 + i * 0x18, pSrc->field_0x80 + i * 0x18, 0x18);
        pDst->cornerHeight[i] = pSrc->cornerHeight[i];
    }
    pDst->groundNormal = pSrc->groundNormal;
}

// FUNCTION: CMR2 0x0042c840
void FUN_0042c840(int first, int count)
{
    int i;

    for (i = first; i < first + count; i++)
        g_unk0x0053acf0[i] = 1;
}

// FUNCTION: CMR2 0x0042c870
void FUN_0042c870(int index)
{
    g_unk0x0053acf0[index] = 1;
}

// Per-frame timing state shared with the HUD: a "seeded" flag for the three
// timestamps and the window start/end the cars are placed in.
// GLOBAL: CMR2 0x0053b530
BYTE g_unk0x0053b530;
// GLOBAL: CMR2 0x0053bd64
int g_unk0x0053bd64;
// GLOBAL: CMR2 0x0053bd40
int g_unk0x0053bd40;
// GLOBAL: CMR2 0x0053a374
int g_unk0x0053a374;
// GLOBAL: CMR2 0x005113a0
float g_unk0x005113a0 = 0.001f;

BYTE FUN_00404f30(void);
void FUN_00404f10(void);
int FUN_0041f360(void);

// Walks the car order and recomputes each car's fraction of the timing window
// (the engine note falloff), writing it to the car and returning the largest.
// match 51%: same logic, but MSVC6 keeps the original in an EBP frame with
// memory-resident loop counters; the register allocation does not reproduce.
// FUNCTION: CMR2 0x0042c890
int FUN_0042c890(int *pOut)
{
    int i;
    int max;
    int offset;
    int mid;
    float fromStart;
    float toEnd;
    Car *pCar;

    if ((g_unk0x0053b530 & 1) == 0) {
        g_unk0x0053b530 |= 1;
        g_unk0x0053bd64 = CMain::GetFrameTime();
    }
    if ((g_unk0x0053b530 & 2) == 0) {
        g_unk0x0053b530 |= 2;
        g_unk0x0053bd40 = CMain::GetFrameTime();
    }
    if ((g_unk0x0053b530 & 4) == 0) {
        g_unk0x0053b530 |= 4;
        g_unk0x0053a374 = CMain::GetFrameTime();
    }
    g_unk0x0053bd40 = CMain::GetFrameTime();
    if (FUN_00404f30() != 0)
        g_unk0x0053a374 = g_unk0x0053bd40;
    if (FUN_0041f360() != 0)
        g_unk0x0053a374 = g_unk0x0053bd40;
    FUN_00404f10();
    max = 0;
    for (i = 0; i < g_carOrderCount; i++) {
        pCar = &g_carBuffer[g_carOrder[i]];
        if (g_carOrder[i] >= 1 &&
            *(float *)(pCar->field_0xa90 + 8) == *(float *)(g_carBuffer->field_0xa90 + 8)) {
            *(int *)pCar->field_0xa90 = *(int *)g_carBuffer->field_0xa90;
            offset = g_carBuffer->field_0xb43[0];
        } else {
            fromStart = (float)(unsigned int)(g_unk0x0053a374 - g_unk0x0053bd64) *
                        g_unk0x005113a0 * *(float *)(pCar->field_0xa90 + 8);
            toEnd = (float)(unsigned int)(g_unk0x0053bd40 - g_unk0x0053bd64) *
                    g_unk0x005113a0 * *(float *)(pCar->field_0xa90 + 8);
            mid = (int)(__int64)toEnd;
            *(int *)pCar->field_0xa90 = (int)((toEnd - (float)mid) * g_65536f);
            offset = mid - (int)(__int64)fromStart;
            if (offset > 5)
                offset = 5;
        }
        pCar->field_0xb43[0] = (char)offset;
        if (max < (offset & 0xff))
            max = offset & 0xff;
    }
    *pOut = *(int *)g_carBuffer->field_0xa90;
    g_unk0x0053a374 = g_unk0x0053bd40;
    return max;
}

// FUNCTION: CMR2 0x0042ca70
CarTransforms *FUN_0042ca70(int index)
{
    return &g_carTransforms[index];
}

// FUNCTION: CMR2 0x0042ca90
BYTE *FUN_0042ca90(int index)
{
    return g_unk0x0053a3a8[index];
}

// FUNCTION: CMR2 0x0042cab0
FixMatrix *FUN_0042cab0(int index)
{
    return g_carWheelTransforms[index];
}

// FUNCTION: CMR2 0x0042cac0
BYTE *FUN_0042cac0(int index)
{
    return g_unk0x0053c5a0[index];
}

BYTE *FUN_00456be0(int index);

// Whether the car uses the narrow wheel setup (car class 6 on the normal
// surfaces); param2 also accepts the 'A' variant.
// FUNCTION: CMR2 0x0042cae0
int FUN_0042cae0(Car *pCar, int param2)
{
    char stage;

    stage = *FUN_00456be0(pCar->field_0xb1a);
    if (!CGameInfo::FUN_004063f0(6) && pCar->field_0xb29 == 6 && stage != 0xb && stage != 8 && stage != 0xa &&
        stage != 0xd && (FUN_00456be0(pCar->field_0xb1a)[0x20] != 'A' || param2 != 0))
        return 1;
    return 0;
}

// Sets the suspension geometry of the current car from the ride-height
// setting (0..0x10000).
// FUNCTION: CMR2 0x0043dff0
void FUN_0043dff0(int param_1)
{
    int off;
    int inv;
    int n0;
    int n1;

    if (CGameInfo::FUN_004063f0(6) != 0)
        param_1 = -0x6666;
    g_pCurrentCar->field_0x9b8 = FixMul(param_1, 0x6666) + 0x9999;
    g_pCurrentCar->field_0x9bc = FixMul(param_1, 0x3333) + 0x6666;
    n0 = FixMul(param_1, -0x1aaaa) + 0x50000;
    n1 = FixMul(param_1, -0x1999) + 0x3333;
    g_pCurrentCar->field_0x8d8 = g_pCurrentCar->field_0x75c / 8;
    *(int *)g_pCurrentCar->field_0x9c0 = FixDiv(0x10000, n1);
    g_pCurrentCar->field_0x9c4 = FixDiv(0x10000, n0);
    inv = 0x10000 - param_1;
    for (off = 0x9f8; off < 0xa08; off += 4) {
        *(int *)((BYTE *)g_pCurrentCar + off - 0x10) = 0x4ccc;
        *(int *)((BYTE *)g_pCurrentCar + off) = FixMul(param_1, 0x5999) + 0x4000;
        *(int *)((BYTE *)g_pCurrentCar + off - 0x70) = FixMul(inv, 0x1999);
        *(int *)((BYTE *)g_pCurrentCar + off - 0x80) = FixMul(param_1, 0x8000) + 0x8000;
    }
    *(int *)((BYTE *)g_pCurrentCar + 0xa08) = -0x1999;
}

// FUNCTION: CMR2 0x0043e160
void FUN_0043e160(int value)
{
    g_pCurrentCar->field_0x82c = FixMul(0x13333, value) + 0x3333;
}

// FUNCTION: CMR2 0x0043e190
void FUN_0043e190(int value)
{
    g_pCurrentCar->field_0x830 = value;
}

// FUNCTION: CMR2 0x0043e1b0
void FUN_0043e1b0(int value)
{
    g_pCurrentCar->field_0x7b4 = value;
}

// FUNCTION: CMR2 0x0043e1d0
void FUN_0043e1d0(BYTE value)
{
    g_pCurrentCar->field_0xb29 = value;
}

int FUN_00437f90(void);

// Engine torque of the current car (less the speed drag) split between the
// front and rear wheels by the drive split.
// match 63%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00442fc0
void FUN_00442fc0(void)
{
    int torque;
    int front;
    int value;

    value = FUN_00437f90() - FixMul(g_pCurrentCar->field_0x784, FixMul(g_pCurrentCar->field_0x7a4, g_pCurrentCar->field_0x7a4));
    g_pCurrentCar->field_0x780 = value;
    torque = FixMul(g_pCurrentCar->field_0x780, g_pCurrentCar->field_0x7bc[g_pCurrentCar->field_0xb1e]);
    front = FixMul(torque, g_pCurrentCar->field_0x7b4) / 2;
    g_pCurrentCar->wheelTorque[0] = front;
    g_pCurrentCar->wheelTorque[1] = front;
    front = torque / 2 - front;
    g_pCurrentCar->wheelTorque[2] = front;
    g_pCurrentCar->wheelTorque[3] = front;
}

// Integrates the ride-height accumulator of the current car toward the
// average wheel load (or the engine speed while it is off the ground).
// match 88%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004430b0
void FUN_004430b0(void)
{
    int hi;
    int lo;

    if (g_pCurrentCar->field_0xb1e == 0) {
        g_pCurrentCar->field_0x7a4 += FixMul(g_pCurrentCar->field_0x780, 0xa0000);
        if (g_pCurrentCar->field_0x7a4 > g_pCurrentCar->field_0x794) {
            g_pCurrentCar->field_0x7a4 = g_pCurrentCar->field_0x794;
            return;
        }
    } else {
        lo = (FixMul(g_pCurrentCar->wheelLoad[0], g_pCurrentCar->field_0x7bc[g_pCurrentCar->field_0xb1e]) +
              FixMul(g_pCurrentCar->wheelLoad[1], g_pCurrentCar->field_0x7bc[g_pCurrentCar->field_0xb1e])) / 2;
        hi = (FixMul(g_pCurrentCar->wheelLoad[2], g_pCurrentCar->field_0x7bc[g_pCurrentCar->field_0xb1e]) +
              FixMul(g_pCurrentCar->wheelLoad[3], g_pCurrentCar->field_0x7bc[g_pCurrentCar->field_0xb1e])) / 2;
        g_pCurrentCar->field_0x7a4 +=
            FixMul(FixMul(lo - hi, g_pCurrentCar->field_0x7b4) - g_pCurrentCar->field_0x7a4 + hi, 0x10000);
    }
}

// FUNCTION: CMR2 0x00443230
void FUN_00443230(void)
{
    g_pCurrentCar->field_0x7ac = g_pCurrentCar->field_0x7a4;
}

// FUNCTION: CMR2 0x00417760
int FUN_00417760(int index)
{
    return Car_Get(index)->field_0xc10 != 0;
}

// Starts (param2 != 0) or stops the camera shake of a view: 0 idle, 1 active,
// 2 locked, 3 stopping.
// FUNCTION: CMR2 0x00423010
void FUN_00423010(int view, int start)
{
    if (start != 0) {
        if (g_unk0x00538f00[view] != 2) {
            if (g_unk0x00538f00[view] == 0) {
                g_unk0x00538df8[view] = 0;
                g_unk0x00538c98[view] = 0;
            }
            g_unk0x00538f00[view] = 1;
        }
        return;
    }
    if (g_unk0x00538f00[view] != 0)
        g_unk0x00538f00[view] = 3;
}


// Whether the car shows its clean wheels: not yet damaged and not on one
// of the snow/night stages.
// FUNCTION: CMR2 0x0042cb50
BOOL FUN_0042cb50(Car *pCar)
{
    char stage;

    stage = *FUN_00456be0(pCar->field_0xb1a);
    if (pCar->field_0xb29 <= 1 && stage != 11 && stage != 8 && stage != 10 && stage != 13)
        return TRUE;
    return FALSE;
}

void Graphics_ReloadTexture(Texture *pTexture);

// GLOBAL: CMR2 0x00519c94
char g_strWheelVariantL[4] = "L";
// GLOBAL: CMR2 0x00519c98
char g_strWheelVariantN[4] = "N";

// Swaps the textures of the four wheel meshes between their "L" and "N"
// variants (the letter 9 characters from the end of the texture name) to
// match FUN_0042cb50, reloading every texture that changed.
// FUNCTION: CMR2 0x0042cb90
void FUN_0042cb90(char mode, SceneNode **pWheels)
{
    int w;
    int t;
    int k;
    Mesh *pMesh;
    Texture *pTex;

    if (mode == 'A')
        return;
    for (w = 0; w < 4; w++) {
        if (pWheels[w] == NULL)
            continue;
        pMesh = *(Mesh **)((BYTE *)pWheels[w] + 0xc);
        for (t = 0; t < pMesh->triangleCount; t++) {
            for (k = 0; k < 10; k++) {
                pTex = CGraphics::m_pTextureManager->textureBuffer[((int *)&pMesh->pTriangles[t])[k + 1]];
                if (pTex == NULL)
                    continue;
                if (!FUN_0042cb50(g_pCurrentCar)) {
                    if (strncmp(pTex->name + strlen(pTex->name) - 9, g_strWheelVariantL, 1) == 0) {
                        strncpy(pTex->name + strlen(pTex->name) - 9, g_strWheelVariantN, 1);
                        Graphics_ReloadTexture(pTex);
                    }
                } else if (strncmp(pTex->name + strlen(pTex->name) - 9, g_strWheelVariantN, 1) == 0) {
                    strncpy(pTex->name + strlen(pTex->name) - 9, g_strWheelVariantL, 1);
                    Graphics_ReloadTexture(pTex);
                }
            }
        }
    }
}

int Track_GetGroundHeightSurface(FixVector *pPoint, FixVector *pNormal, short *pTri, short *pSurfaceClass,
                                 unsigned short *pSurface, int defaultY);

// GLOBAL: CMR2 0x0053cadc
int g_gravityScale;

// Velocity of the box corners of the current car: the two opposite corners
// get the rotational component (half-width times the angular velocity through
// the world matrix) plus the body velocity; the other six copy them.
// match 76%: same logic; MSVC put pWorld in ESI and pVel in EDI instead of
// the EDI/ESI the original used (induction/pointer register numbering).
// FUNCTION: CMR2 0x00443a20
void Car_UpdateCornerVelocityPair(void)
{
    FixMatrix *pWorld;
    FixVector *pAngVel;
    FixVector *pVel;
    FixVector *pCorner;
    int a;
    int b;

    pWorld = g_pCurrentCar->pWorld;
    pAngVel = &g_pCurrentCar->angularVelocity;
    pVel = &g_pCurrentCar->velocity;
    pCorner = g_pCurrentCar->cornerVelocity;
    a = FixMul(pAngVel->z, g_pCurrentCar->halfExtents.x);
    b = FixMul(pAngVel->y, g_pCurrentCar->halfExtents.x);
    pCorner[0].x = FixMul(pWorld->up.x, a) - FixMul(pWorld->forward.x, b);
    pCorner[0].y = FixMul(pWorld->up.y, a) - FixMul(pWorld->forward.y, b);
    pCorner[0].z = FixMul(pWorld->up.z, a) - FixMul(pWorld->forward.z, b);
    pCorner[2].x = -pCorner[0].x;
    pCorner[2].y = -pCorner[0].y;
    pCorner[2].z = -pCorner[0].z;
    pCorner[0].x += pVel->x;
    pCorner[0].y += pVel->y;
    pCorner[0].z += pVel->z;
    pCorner[2].x += pVel->x;
    pCorner[2].y += pVel->y;
    pCorner[2].z += pVel->z;
    pCorner[1] = pCorner[0];
    pCorner[3] = pCorner[2];
    pCorner[4] = pCorner[0];
    pCorner[5] = pCorner[0];
    pCorner[6] = pCorner[2];
    pCorner[7] = pCorner[2];
}

// FUNCTION: CMR2 0x00443d10
void Car_FollowGround(void)
{
    FixVector n;
    FixVector oldNormal;
    FixVector dn;
    FixVector rel;
    FixVector corner;
    FixVector p;
    FixVector d;
    FixVector t;
    unsigned short surface;
    int ease;
    int along;
    int slow;
    int h;
    int len;
    int k;
    int i;

    g_pCurrentCar->cornerHeight[0] =
        Track_GetGroundHeightSurface(&g_pCurrentCar->position, &n, &g_pCurrentCar->cornerTriangle[0],
                                     &g_pCurrentCar->wheelSurface[0], &surface, g_pCurrentCar->cornerHeight[0]);
    if (g_pCurrentCar->field_0xbac[4] != 0) {
        g_pCurrentCar->cornerHeight[0] += g_pCurrentCar->field_0x8fc;
        n = g_pCurrentCar->field_0x564;
        g_pCurrentCar->wheelSurface[0] = 0x2f;
    }
    if (g_pCurrentCar->wheelSurface[0] == 0xf && g_pCurrentCar->field_0xa7c == 0 && g_pCurrentCar->field_0xbf8 == 0)
        g_pCurrentCar->field_0xa7c = 0x190000;

    // Ease the ground normal towards the new one.
    oldNormal = g_pCurrentCar->groundNormal;
    n.x -= g_pCurrentCar->groundNormal.x;
    n.y -= g_pCurrentCar->groundNormal.y;
    n.z -= g_pCurrentCar->groundNormal.z;
    ease = FixMul(g_physicsTimeStep, 0x8000);
    if (ease > 0x8000)
        ease = 0x8000;
    FixVecScale(&dn, &n, ease);
    dn.x = n.x - dn.x;
    dn.y = n.y - dn.y;
    dn.z = n.z - dn.z;
    n.x = g_pCurrentCar->groundNormal.x + dn.x;
    n.y = g_pCurrentCar->groundNormal.y + dn.y;
    n.z = g_pCurrentCar->groundNormal.z + dn.z;
    FIX_NORMALIZE_INTO(g_pCurrentCar->groundNormal, n);
    along = FixVecDot(&g_pCurrentCar->velocity, &g_pCurrentCar->groundNormal);
    slow = FixMul(g_pCurrentCar->speed, 0x8000);
    if (slow > 0x10000)
        slow = 0x10000;
    slow = FixMul(0x10000 - slow, 0x1999);
    if (g_pCurrentCar->field_0xb74 != 0) {
        if (along <= slow) {
            g_pCurrentCar->up = g_pCurrentCar->groundNormal;
            FixVecScale(&t, &g_pCurrentCar->up, FixVecDot(&g_pCurrentCar->right, &g_pCurrentCar->up));
            t.x = g_pCurrentCar->right.x - t.x;
            t.y = g_pCurrentCar->right.y - t.y;
            t.z = g_pCurrentCar->right.z - t.z;
            FIX_NORMALIZE_INTO(g_pCurrentCar->right, t);
            FixVecCross(&t, &g_pCurrentCar->right, &g_pCurrentCar->up);
            FIX_NORMALIZE_INTO(g_pCurrentCar->forward, t);
        } else {
            g_pCurrentCar->groundNormal = oldNormal;
        }
    }
    g_pCurrentCar->pWorld->right = g_pCurrentCar->right;
    g_pCurrentCar->pWorld->up = g_pCurrentCar->up;
    g_pCurrentCar->pWorld->forward = g_pCurrentCar->forward;
    g_pCurrentCar->pWorld->position = g_pCurrentCar->position;

    // Drop the lowest corner of the box onto the ground.
    t.x = -g_pCurrentCar->groundNormal.x;
    t.y = -g_pCurrentCar->groundNormal.y;
    t.z = -g_pCurrentCar->groundNormal.z;
    FixMatrix_InverseRotateVector(&rel, &t, g_pCurrentCar->pWorld);
    corner.x = g_pCurrentCar->halfExtents.x;
    corner.y = -g_pCurrentCar->halfExtents.y;
    corner.z = g_pCurrentCar->halfExtents.z;
    if (rel.y >= 0)
        corner.y = g_pCurrentCar->halfExtents.y;
    if (rel.x < 0)
        corner.x = -g_pCurrentCar->halfExtents.x;
    if (rel.z < 0)
        corner.z = -g_pCurrentCar->halfExtents.z;
    FixMatrix_RotateVector(&p, &corner, g_pCurrentCar->pWorld);
    p.x += g_pCurrentCar->position.x;
    p.y += g_pCurrentCar->position.y;
    p.z += g_pCurrentCar->position.z;
    d.x = g_pCurrentCar->position.x - p.x;
    d.y = g_pCurrentCar->cornerHeight[0] - p.y;
    d.z = g_pCurrentCar->position.z - p.z;
    h = FixDiv(FixVecDot(&g_pCurrentCar->groundNormal, &d), g_pCurrentCar->groundNormal.y);
    for (i = 0; i < 8; i++) {
        g_pCurrentCar->cornerAxis[i].x = 0;
        g_pCurrentCar->cornerAxis[i].y = 0x10000;
        g_pCurrentCar->cornerAxis[i].z = 0;
        g_pCurrentCar->cornerNormal[i] = g_pCurrentCar->cornerAxis[i];
    }
    g_pCurrentCar->normal0x498 = g_pCurrentCar->groundNormal;
    for (i = 1; i < 4; i++) {
        g_pCurrentCar->cornerHeight[i] = g_pCurrentCar->cornerHeight[0];
        g_pCurrentCar->wheelSurface[i] = g_pCurrentCar->wheelSurface[0];
    }
    ((char *)&g_pCurrentCar->field_0xb34)[1] = 0;
    g_pCurrentCar->field_0xb34 = 4;
    ((char *)&g_pCurrentCar->field_0xb34)[2] = 0;
    ((char *)&g_pCurrentCar->field_0xb34)[3] = 1;
    ((char *)&g_pCurrentCar->field_0xb34)[4] = 2;
    ((char *)&g_pCurrentCar->field_0xb34)[5] = 3;
    g_pCurrentCar->cornerFlags[0] = 0;
    g_pCurrentCar->cornerFlags[1] = 0;
    g_pCurrentCar->cornerFlags[2] = 0;
    g_pCurrentCar->cornerFlags[3] = 0;
    g_pCurrentCar->cornerFlags[4] = 1;
    g_pCurrentCar->cornerFlags[5] = 1;
    g_pCurrentCar->cornerFlags[6] = 1;
    g_pCurrentCar->cornerFlags[7] = 1;
    if (h > 0) {
        g_pCurrentCar->position.y += h;
        g_pCurrentCar->pWorld->position = g_pCurrentCar->position;
    }
    if (h >= -0x1999) {
        if (h < 0)
            g_pCurrentCar->field_0x958 = h;
        else
            g_pCurrentCar->field_0x958 = 0;
        if (along < 0) {
            FixVecScale(&t, &g_pCurrentCar->groundNormal, along);
            g_pCurrentCar->velocity.x -= t.x;
            g_pCurrentCar->velocity.y -= t.y;
            g_pCurrentCar->velocity.z -= t.z;
        }
        g_pCurrentCar->baseForce.x = 0;
        g_pCurrentCar->baseForce.y = 0;
        g_pCurrentCar->baseForce.z = 0;
        g_pCurrentCar->angularVelocity.z = 0;
        g_pCurrentCar->angularVelocity.x = 0;
    } else {
        g_pCurrentCar->field_0x958 = -0x1999;
        g_pCurrentCar->baseForce.x = 0;
        g_pCurrentCar->baseForce.y = -FixMul(g_pCurrentCar->field_0x75c, FixMul(0x8000, g_gravityScale));
        g_pCurrentCar->baseForce.z = 0;
    }
    if (h < -0xcccc) {
        if (g_pCurrentCar->field_0xb74 != 0) {
            // Tip over with the change of the ground normal.
            FixMatrix_InverseRotateVector(&t, &dn, g_pCurrentCar->pWorld);
            t.y = 0;
            len = FixVecLength(&t);
            if (len > 10000)
                FixVecScale(&t, &t, FixDiv(10000, len));
            k = -FixMul(g_pCurrentCar->halfExtents.y, FixMul(0x4ccc, g_physicsTimeStep));
            g_pCurrentCar->angularVelocity.x -= FixMul(t.z, k);
            g_pCurrentCar->angularVelocity.z += FixMul(t.x, k);
        }
        g_pCurrentCar->field_0xb74 = 0;
        return;
    }
    g_pCurrentCar->field_0xb74 = 1;
}

// Switches the car to the tumbling (eight corner) model and stores the
// ground normal in its body axes.
#define CAR_START_TUMBLING()                                                                         \
    g_pCurrentCar->field_0xc00 = 1;                                                                  \
    g_pCurrentCar->field_0x96c = 0x10000;                                                            \
    g_pCurrentCar->field_0x91c = FixVecDot(&g_pCurrentCar->groundNormal, &g_pCurrentCar->right);     \
    g_pCurrentCar->field_0x920 = FixVecDot(&g_pCurrentCar->groundNormal, &g_pCurrentCar->up);        \
    g_pCurrentCar->field_0x924 = FixVecDot(&g_pCurrentCar->groundNormal, &g_pCurrentCar->forward)

// Roll-over: on a slope too steep to stand on (over about 30 degrees) the
// car gets a growing torque that tips it over; otherwise, sliding sideways
// faster than the wheels can hold tips the body, and past the limit the car
// starts to tumble.
// match 76%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004373c0
void Car_UpdateRollover(void)
{
    FixVector t;
    FixVector a;
    FixVector b;
    FixVector c;
    FixVector dir;
    int k;
    int sum;
    int limit;
    int slide;
    int tip;
    int target;
    int diff;
    int rate;
    int mag;
    int i;

    k = 0;
    if (g_pCurrentCar->field_0xb28 == 0 && g_pCurrentCar->field_0xb34 == 0)
        return;
    if (g_pCurrentCar->groundNormal.y < 0x10000 && (short)(0x400 - FixAcos(g_pCurrentCar->groundNormal.y)) > 0x155 &&
        g_pCurrentCar->field_0xb42 <= 0) {
        FixVecScale(&t, &g_pCurrentCar->groundNormal, 0x280000);
        FixMatrix_InverseRotateVector(&a, &t, g_pCurrentCar->pWorld);
        FixVecScale(&t, &g_pCurrentCar->baseForce,
                    FixMul(g_pCurrentCar->steepTime << 16, FixMul(FixDiv(0x10000, 0xc80000), 0x8000)));
        FixMatrix_InverseRotateVector(&b, &t, g_pCurrentCar->pWorld);
        FixVecCross(&c, &b, &a);
        FixVecScale(&c, &c, g_pCurrentCar->field_0x760);
        if (g_pCurrentCar->field_0xc00 == 0) {
            CAR_START_TUMBLING();
        }
        g_pCurrentCar->field_0x5d0.x += c.x;
        g_pCurrentCar->field_0x5d0.y += c.y;
        g_pCurrentCar->field_0x5d0.z += c.z;
        if (g_pCurrentCar->steepTime < 200)
            g_pCurrentCar->steepTime++;
    } else {
        g_pCurrentCar->steepTime = 0;
    }
    if (g_pCurrentCar->field_0xc00 != 0)
        return;

    // Sideways slide against the grip of the wheels.
    sum = 0;
    for (i = 0; i < 4; i++)
        sum += *((BYTE *)g_pCurrentCar + 0x1a0 + i * 0xc) << 16;
    limit = FixMul(FixDiv(sum, 0x40000), 0x3d1);
    FixVecScale(&t, &g_pCurrentCar->groundNormal, FixVecDot(&g_pCurrentCar->groundNormal, &g_pCurrentCar->forward));
    t.x = g_pCurrentCar->forward.x - t.x;
    t.y = g_pCurrentCar->forward.y - t.y;
    t.z = g_pCurrentCar->forward.z - t.z;
    FIX_NORMALIZE_INTO(dir, t);
    slide = FixVecDot(&g_pCurrentCar->velocity, &dir);
    tip = 0;
    if (FIX_ABS(slide) > limit) {
        sum = 0;
        for (i = 0; i < 4; i++)
            sum += *((BYTE *)g_pCurrentCar + 0x1a1 + i * 0xc) << 16;
        if (limit != 0)
            k = FixDiv(0x10000, FixMul(FixDiv(sum, 0x40000), 0x3d1) - limit);
        if (slide > 0)
            limit = -limit;
        tip = FixMul(slide + limit, k);
        if (FIX_ABS(tip) > 0x10000)
            tip = tip < 1 ? -0x10000 : 0x10000;
    }
    target = FixMul(tip, 0x10000);
    diff = target - g_pCurrentCar->tipRatio;
    rate = FixMul(g_physicsTimeStep, 0x1999);
    if (FIX_ABS(diff) < rate) {
        g_pCurrentCar->tipRatio = target;
    } else {
        if (diff < 1)
            rate = -rate;
        g_pCurrentCar->tipRatio += rate;
    }
    mag = FIX_ABS(g_pCurrentCar->tipRatio);
    g_pCurrentCar->tipAngle = FixAtan2(g_pCurrentCar->tipRatio, 0x20000);
    if (mag > 0xcccc) {
        g_pCurrentCar->tipAngle = 0;
        CAR_START_TUMBLING();
        g_pCurrentCar->field_0x5d0.x += FixMul(FixMul(slide, g_physicsScale), -0xa0000);
    }
}

extern int g_unk0x0053c9d4;

// Lowers the current car's target (0x7a4) toward 0x794 minus a fading offset.
int FUN_00458310(int index);

// Transfers ride height between the two cars so the one further back along
// the road is raised; only active in the 2-player game (state 2).
// match 58%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00433e80
void FUN_00433e80(BYTE *param_1, short *param_2, short param_3)
{
    BYTE *pCar1;
    BYTE *pCar2;
    int diff;
    int dist;
    int t;
    int amount;

    if ((BYTE)RallyDataState() != 2)
        return;
    if (param_3 != 2)
        return;
    if ((BYTE)RallyData_GetFlag24() == 0)
        return;
    if ((BYTE)CGameInfo::FUN_00406440() == 0)
        return;
    pCar1 = param_1 + param_2[0] * 0xc24;
    pCar2 = param_1 + param_2[1] * 0xc24;
    diff = FUN_00458310(*(char *)(pCar1 + 0xb1a)) - FUN_00458310(*(char *)(pCar2 + 0xb1a));
    dist = diff * 0x10000;
    if (dist < 0)
        dist = -dist;
    if (dist <= 0x30000) {
        t = 0;
    } else if (dist < 0xf0000) {
        t = FixMul(dist - 0x30000, FixDiv(0x10000, 0xc0000));
        if (t < 0)
            t = 0;
        else if (t > 0x10000)
            t = 0x10000;
    } else {
        t = 0x10000;
    }
    amount = FixMul(t, 0xccc);
    if (diff >= 0) {
        *(int *)(pCar1 + 0x788) = *(int *)(pCar1 + 0x78c) - amount;
        *(int *)(pCar2 + 0x788) = *(int *)(pCar2 + 0x78c) + amount;
        return;
    }
    *(int *)(pCar2 + 0x788) = *(int *)(pCar2 + 0x78c) - amount;
    *(int *)(pCar1 + 0x788) = *(int *)(pCar1 + 0x78c) + amount;
}

// FUNCTION: CMR2 0x00433fd0
void FUN_00433fd0(void)
{
    int target = g_pCurrentCar->field_0x794 - FixMul(g_pCurrentCar->field_0xb1d << 16, 0x3333);

    if (g_pCurrentCar->field_0x7a4 - target > -0x290) {
        g_pCurrentCar->field_0x7a4 = target;
        if (g_pCurrentCar->field_0xb1d == 0)
            g_pCurrentCar->field_0xb1d = 3;
    }
    if (g_pCurrentCar->field_0xb1d != 0) {
        g_pCurrentCar->field_0xb1d--;
        g_pCurrentCar->field_0x7ac = g_pCurrentCar->field_0x7a4;
        return;
    }
    g_pCurrentCar->field_0x7ac = g_pCurrentCar->field_0x7a4;
}

// Ground grip factor of the current car from the slope it stands on.
// FUNCTION: CMR2 0x00434070
void FUN_00434070(void)
{
    int grip;
    int y = g_pCurrentCar->groundNormal.y;

    if (y < 0xf0a3) {
        grip = FixMul(FixDiv(0x10000, 0x3d70), y - 0xb333);
        if (grip < 0)
            grip = 0;
        g_unk0x0053c9d4 = FixMul(0xe666, grip);
        return;
    }
    g_unk0x0053c9d4 = 0xe666;
}

// Counts field 0xb1f of the current car down while it is slow compared to
// the load of the (front or rear) wheels; clears it once it is fast enough.
// FUNCTION: CMR2 0x004340f0
void FUN_004340f0(void)
{
    int load;

    if (g_pCurrentCar->field_0xb1f != 0) {
        if (g_pCurrentCar->field_0x7b4 == 0)
            load = g_pCurrentCar->speed * 4 - g_pCurrentCar->wheelLoad[2];
        else
            load = g_pCurrentCar->speed * 4 - g_pCurrentCar->wheelLoad[0];
        if (load > -0x1999) {
            g_pCurrentCar->field_0xb1f = 0;
            return;
        }
        g_pCurrentCar->field_0xb1f--;
    }
}

// Steering: the rolling direction of the car eases towards the body's
// right axis (faster with field_0x79c), and the steered wheels turn from it
// by the steering angle, which wobbles with the position on the stage on
// the cars whose setup asks for it.
// match 78%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00434140
void Car_UpdateSteering(void)
{
    FixMatrix m;
    FixVector right;
    FixVector t;
    FixVector d;
    short angle;
    int x;
    int z;
    int diff;
    int w;
    int k;

    if (FIX_ABS(g_pCurrentCar->speed) < 0x28f) {
        angle = 0;
    } else {
        angle = g_pCurrentCar->heading;
        if (*(int *)(g_pCarSetup + 0x3d8) > 0) {
            x = g_pCurrentCar->position.x;
            z = g_pCurrentCar->position.z;
            if (FIX_ABS(x) - FIX_ABS(z) < 0)
                diff = FIX_ABS(z) - FIX_ABS(x);
            else
                diff = FIX_ABS(x) - FIX_ABS(z);
            w = FixMul(0x20000, (diff % 0x401 - 0x200) * 0x40);
            if (g_pCurrentCar->speed < 0x10000)
                w = FixMul(w, g_pCurrentCar->speed);
            angle += (short)(__int64)((double)FixMul(w, *(int *)(g_pCarSetup + 0x3d8)) * g_unk0x00511300);
        }
    }
    right = g_pCurrentCar->right;
    FixVecScale(&t, &g_pCurrentCar->up, FixVecDot(&g_pCurrentCar->up, &g_pCurrentCar->wheelDirFront));
    t.x = g_pCurrentCar->wheelDirFront.x - t.x;
    t.y = g_pCurrentCar->wheelDirFront.y - t.y;
    t.z = g_pCurrentCar->wheelDirFront.z - t.z;
    FIX_NORMALIZE_INTO(g_pCurrentCar->wheelDirFront, t);
    d.x = right.x - g_pCurrentCar->wheelDirFront.x;
    d.y = right.y - g_pCurrentCar->wheelDirFront.y;
    d.z = right.z - g_pCurrentCar->wheelDirFront.z;
    k = FixMul(FixMul(FixVecLength(&d), 0x13333),
               FixMul(FixMul(g_pCurrentCar->field_0x79c, 0xcccd) + 0x3333, g_physicsTimeStep));
    if (k >= 0x10000) {
        g_pCurrentCar->wheelDirFront = right;
    } else {
        FixVecScale(&d, &d, k);
        g_pCurrentCar->wheelDirFront.x += d.x;
        g_pCurrentCar->wheelDirFront.y += d.y;
        g_pCurrentCar->wheelDirFront.z += d.z;
        FIX_NORMALIZE_INTO(g_pCurrentCar->wheelDirFront, g_pCurrentCar->wheelDirFront);
    }
    if (angle == 0) {
        g_pCurrentCar->wheelDirRear = g_pCurrentCar->wheelDirFront;
    } else {
        FixMatrix_FromAxisAngle(&m, &g_pCurrentCar->up, angle);
        FixMatrix_RotateVector(&t, &g_pCurrentCar->wheelDirFront, &m);
        FIX_NORMALIZE_INTO(g_pCurrentCar->wheelDirRear, t);
    }
    FixVecCross(&g_pCurrentCar->wheelAxisRear, &g_pCurrentCar->wheelDirRear, &g_pCurrentCar->up);
}

// GLOBAL: CMR2 0x0053c9d4
int g_unk0x0053c9d4;
// Direction of gravity.
// GLOBAL: CMR2 0x0053cad0
FixVector g_gravityDir;

// Ground load: the normal force of the ground spread over the touching
// corners, and from the suspension load of each wheel its grip limits.
// match 76%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004348c0
void Car_UpdateCornerLoads(void)
{
    FixVector n0;
    FixVector f;
    int k;
    int n;
    int load;
    int grip;
    int i;
    int *pGripB;

    n = g_pCurrentCar->field_0xb34;
    if (n == 0) {
        g_pCurrentCar->cornerMass = 0;
    } else {
        if (n == 4)
            load = g_pCurrentCar->field_0x75c / 4;
        else if (n == 2)
            load = g_pCurrentCar->field_0x75c / 2;
        else {
            load = g_pCurrentCar->field_0x75c;
            if (n != 1)
                load = load / n;
        }
        g_pCurrentCar->cornerMass = load;
    }
    n0 = g_pCurrentCar->groundNormal;
    load = -FixMul(g_pCurrentCar->cornerMass + 0x1e0000, FixVecDot(&g_gravityDir, &n0));
    FixVecScale(&f, &n0, FixMul(load, 0x10000));
    grip = FixMul(g_pCurrentCar->speed, FixMul(0x1cccc, 0x10000));
    if (grip > 0x1cccc)
        grip = 0x1cccc;
    grip += 0x10000;
    for (i = 0; i < 8; i++) {
        pGripB = &g_pCurrentCar->cornerGripB[i];
        if (g_pCurrentCar->cornerFlags[i] == 0 || (i < 4 && g_pCurrentCar->field_0xbac[i] != 0)) {
            if (n != 4) {
                g_pCurrentCar->cornerLoad[i].x += f.x;
                g_pCurrentCar->cornerLoad[i].y += f.y;
                g_pCurrentCar->cornerLoad[i].z += f.z;
            }
            if (i < 4 && g_pCurrentCar->field_0xb74 != 0) {
                load = FixMul(g_pCurrentCar->field_0x8b8[i] + 0x1e0000, FixMul(g_pCurrentCar->cornerAxis[i].y, 0x8000));
                g_pCurrentCar->field_0xa5c[i] =
                    FixMul(load, *(int *)((BYTE *)g_pCurrentCar + 0x1a4 + i * 0xc) +
                                     *(int *)((BYTE *)g_pCurrentCar + 0x88 + i * 0x24));
                g_pCurrentCar->field_0xa4c[i] =
                    FixMul(load, *(int *)((BYTE *)g_pCurrentCar + 0x1a4 + i * 0xc) +
                                     *(int *)((BYTE *)g_pCurrentCar + 0x80 + i * 0x24));
            } else {
                g_pCurrentCar->cornerGripA[i] = FixMul(load, *(int *)((BYTE *)g_pCurrentCar + 0x88 + i * 0x24));
                *pGripB = FixMul(load, *(int *)((BYTE *)g_pCurrentCar + 0x80 + i * 0x24));
                g_pCurrentCar->cornerGripA[i] = FixMul(g_pCurrentCar->cornerGripA[i], grip);
                *pGripB = FixMul(*pGripB, grip);
            }
        } else if (i < 4) {
            *pGripB = 0;
            g_pCurrentCar->cornerGripA[i] = 0;
            g_pCurrentCar->field_0xa4c[i] = 0;
            g_pCurrentCar->field_0xa5c[i] = 0;
        } else {
            *pGripB = 0;
            g_pCurrentCar->cornerGripA[i] = 0;
        }
    }
    if (g_pCurrentCar->field_0xb34 > 0 || g_pCurrentCar->field_0xb28 != 0) {
        // The base force without its push into the ground, shared by the
        // touching corners.
        FixVecScale(&f, &n0, FixVecDot(&n0, &g_pCurrentCar->baseForce));
        f.x = g_pCurrentCar->baseForce.x - f.x;
        f.y = g_pCurrentCar->baseForce.y - f.y;
        f.z = g_pCurrentCar->baseForce.z - f.z;
        FixVecScale(&f, &f, 0x10000 - g_unk0x0053c9d4);
        if (g_pCurrentCar->field_0xb74 == 0) {
            if (g_pCurrentCar->field_0xb34 > 0)
                FixVecScaleRecip(&g_carStepAccel, &f,
                                 (int)(__int64)((double)g_pCurrentCar->field_0xb34 * CGraphics::m_65536));
            else
                FixVecScaleRecip(&g_carStepAccel, &f,
                                 (int)(__int64)((double)g_pCurrentCar->field_0xb28 * CGraphics::m_65536));
        } else if (g_pCurrentCar->field_0xb28 > 0) {
            FixVecScaleRecip(&g_carStepAccel, &f, (int)(__int64)((double)g_pCurrentCar->field_0xb28 * CGraphics::m_65536));
        } else {
            FixVecScaleRecip(&g_carStepAccel, &f, (int)(__int64)((double)g_pCurrentCar->field_0xb34 * CGraphics::m_65536));
        }
        k = FixMul(FixDiv(0x10000, 0x3d70), -0xb333);
        if (k < 0)
            k = 0;
        FixVecScale(&g_carStepAccel, &g_carStepAccel, k);
    }
    if (g_pCurrentCar->field_0xb34 > 0)
        g_pCurrentCar->baseForce = f;
}

// Updates the engine speed from the selected gear or the startup animation,
// then applies the rev limit and its excess-speed flag.
// match 61%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004380c0
void Car_UpdateEngineSpeed(void)
{
    int front;
    int rear;
    int excess;
    short angle;

    if (g_pCurrentCar->field_0xb1e != 0 && g_pCurrentCar->field_0xb84 == 0) {
        front = FixMul(g_pCurrentCar->wheelLoad[0], g_pCurrentCar->field_0x7bc[g_pCurrentCar->field_0xb1e]) + FixMul(g_pCurrentCar->wheelLoad[1], g_pCurrentCar->field_0x7bc[g_pCurrentCar->field_0xb1e]);
        rear = front;
        if (g_pCurrentCar->field_0x1d8 == 0 || g_pCurrentCar->field_0x7b4 == 0)
            rear = FixMul(g_pCurrentCar->wheelLoad[2], g_pCurrentCar->field_0x7bc[g_pCurrentCar->field_0xb1e]) + FixMul(g_pCurrentCar->wheelLoad[3], g_pCurrentCar->field_0x7bc[g_pCurrentCar->field_0xb1e]);
        g_pCurrentCar->field_0x7a4 += FixMul(FixMul(front / 2 - rear / 2, g_pCurrentCar->field_0x7b4) - g_pCurrentCar->field_0x7a4 + rear / 2,
                                  0x10000);
    } else {
        if (g_pCurrentCar->field_0xb4c == 0) {
            if (g_pCurrentCar->field_0xafe == 0) {
                g_pCurrentCar->field_0x7a4 += FixMul(g_pCurrentCar->field_0x780, 0xa0000);
            } else if (g_pCurrentCar->field_0xafe > 0) {
                angle = (short)(__int64)((double)FixMul(FixMul(0x190000 - FixMul(0x41, g_pCurrentCar->field_0xafe << 16), 0xa3d),
                                                             0x8c0000) * g_unk0x00511300);
                g_pCurrentCar->field_0x7a4 = FixMul(FixMul(g_sinTable[(unsigned short)angle & 0xfff], 0x10000), g_pCurrentCar->field_0x794);
                g_pCurrentCar->field_0xafe -= (short)FixMulShift32(g_physicsTimeStep, 0x3e80000);
                if (g_pCurrentCar->field_0xafe < 0)
                    g_pCurrentCar->field_0xafe = 0;
                if (angle < 0x400)
                    g_pCurrentCar->field_0x79c = 0x10000;
            }
        } else if (g_pCurrentCar->field_0xafe < 0x3e9) {
            g_pCurrentCar->field_0xb4c = 0;
            g_pCurrentCar->field_0xafe = 25000;
        } else {
            g_pCurrentCar->field_0xafe -= (short)FixMulShift32(g_physicsTimeStep, 0x3e80000);
            if (g_pCurrentCar->field_0xafe < 0)
                g_pCurrentCar->field_0xafe = 0;
        }
    }

    if (g_pCurrentCar->field_0x7a4 < 0) {
        g_pCurrentCar->field_0x7a4 = 0;
    } else if (g_pCurrentCar->field_0x7a4 > g_pCurrentCar->field_0x794) {
        excess = g_pCurrentCar->field_0x7a4 - g_pCurrentCar->field_0x794;
        if (excess <= 0xcccc) {
            g_pCurrentCar->field_0xb78 = 0;
        } else {
            excess = FixMul(excess - 0xcccc, 0x10000);
            if (excess > 0x10000)
                excess = 0x10000;
            g_pCurrentCar->field_0x7b0 = FixMul(excess, 0x51eb);
            g_pCurrentCar->field_0xb78 = 1;
        }
        g_pCurrentCar->field_0x7a4 = g_pCurrentCar->field_0x794;
        return;
    }
    g_pCurrentCar->field_0xb78 = 0;
}

// Rebuilds the body matrix axes and lifts the body along its up axis by the
// front suspension height. When field_0xb74 is set the axes are taken from
// the world matrix instead.
// FUNCTION: CMR2 0x00444a70
void FUN_00444a70(void)
{
    FixVector v[3];
    FixVector tmp;
    FixVector *pForward;
    FixMatrix *pBody;
    int scale;
    int i;

    if (g_pCurrentCar->field_0xb74 != 0) {
        FixMatrix_GetRight(&v[0], g_pCurrentCar->pBodyMatrix);
        FixMatrix_GetUp(&v[1], g_pCurrentCar->pBodyMatrix);
        FixMatrix_GetForward(&v[2], g_pCurrentCar->pBodyMatrix);
        for (i = 0; i < 3; i++) {
            FixMatrix_RotateVector(&tmp, &v[i], g_pCurrentCar->pWorld);
            v[i] = tmp;
        }
        FixMatrix_SetRight(&v[0], g_pCurrentCar->pBodyMatrix);
        FixMatrix_SetUp(&v[1], g_pCurrentCar->pBodyMatrix);
        pForward = &v[2];
        pBody = g_pCurrentCar->pBodyMatrix;
    } else {
        FixMatrix_SetRight(&g_pCurrentCar->right, g_pCurrentCar->pBodyMatrix);
        FixMatrix_SetUp(&g_pCurrentCar->up, g_pCurrentCar->pBodyMatrix);
        pForward = &g_pCurrentCar->forward;
        pBody = g_pCurrentCar->pBodyMatrix;
    }
    FixMatrix_SetForward(pForward, pBody);
    scale = g_pCurrentCar->wheel0x988[0];
    FixVecScale(&tmp, &g_pCurrentCar->up, scale);
    tmp.x += g_pCurrentCar->position.x;
    tmp.y += g_pCurrentCar->position.y;
    tmp.z += g_pCurrentCar->position.z;
    g_pCurrentCar->pBodyMatrix->position = tmp;
}

// Lifts the current car out of the ground by the deepest penetration of any
// of its eight box corners, rising or sinking as needed.
// FUNCTION: CMR2 0x004458d0
void FUN_004458d0(void)
{
    int maxDrop = 0;
    int maxRise = -0x3e80000;
    int foundDrop = 0;
    int foundRise = 0;
    int i;
    int d;
    int lift = 0;

    for (i = 7; i >= 0; i--) {
        d = g_pCurrentCar->cornerHeight[i] - g_pCurrentCar->corners[i].y;
        if (d >= 0 && d > maxDrop) {
            maxDrop = d;
            foundDrop = 1;
        }
        if (d <= 0 && d > maxRise) {
            maxRise = d;
            foundRise = 1;
        }
    }
    if (foundDrop)
        lift = maxDrop;
    else if (foundRise)
        lift = maxRise;
    g_pCurrentCar->position.y += lift;
    g_pCurrentCar->corners[0].y += lift;
    g_pCurrentCar->corners[1].y += lift;
    g_pCurrentCar->corners[2].y += lift;
    g_pCurrentCar->corners[3].y += lift;
    g_pCurrentCar->corners[4].y += lift;
    g_pCurrentCar->corners[5].y += lift;
    g_pCurrentCar->corners[6].y += lift;
    g_pCurrentCar->corners[7].y += lift;
}

// GLOBAL: CMR2 0x0053cdb4
int g_unk0x0053cdb4;

// FUNCTION: CMR2 0x00445a20
int FUN_00445a20(void)
{
    if (FUN_00422f50(0) != 9 && g_unk0x0053cdb4 == 0)
        return 0;
    return 1;
}

BYTE *FUN_0042ca90(int index);
int FUN_0048df10(BYTE *pCar);
int FUN_0048d930(BYTE *p);

// Rotation of car `index`'s camera placement.
// FUNCTION: CMR2 0x00423a00
FixMatrix *FUN_00423a00(FixMatrix *pOut, BYTE index)
{
    FixMatrix_CopyRotationFrom(pOut, (FixMatrix *)FUN_0042ca90(index));
    return pOut;
}

// Position of a car's camera target node (at +0x750).
// FUNCTION: CMR2 0x00423db0
FixVector *FUN_00423db0(FixVector *pOut, BYTE index)
{
    FixMatrix_GetPosition(pOut, *(FixMatrix **)((BYTE *)Car_Get(index) + 0x750));
    return pOut;
}

// Current camera mode record (100 bytes) of a view.
#define VIEW_MODE_RECORD(i) (g_unk0x0053901c - 4 + ((unsigned int)g_unk0x00538e0c[i] + (i) * 2) * 100)

struct Unk00423ee0Block {
    int value[16];
};

void FixMatrix_Interpolate(FixMatrix *pOut, FixMatrix *pA, FixMatrix *pB, int tRight, int tAxis, int tPos, int mode);

// Interpolates between two state records (matrix and the values at +0x48).
// match 63%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00423de0
void FUN_00423de0(BYTE *pOut, BYTE *pA, BYTE *pB, int t)
{
    int off;

    FixMatrix_Interpolate((FixMatrix *)(pOut + 8), (FixMatrix *)(pA + 8), (FixMatrix *)(pB + 8), t, t, t, 1);
    *(int *)(pOut + 0x48) = *(int *)(pA + 0x48) + FixMul(*(int *)(pB + 0x48) - *(int *)(pA + 0x48), t);
    *(int *)(pOut + 0x4c) = *(int *)(pA + 0x4c) + FixMul(*(int *)(pB + 0x4c) - *(int *)(pA + 0x4c), t);
    for (off = 0x54; off <= 0x5c; off += 4)
        *(int *)(pOut + off) = *(int *)(pA + off) + FixMul(*(int *)(pB + off) - *(int *)(pA + off), t);
    *(int *)(pOut + 0x50) = *(int *)(pA + 0x50) + FixMul(*(int *)(pB + 0x50) - *(int *)(pA + 0x50), t);
    *(int *)(pOut + 0x60) = *(int *)(pA + 0x60) + FixMul(*(int *)(pB + 0x60) - *(int *)(pA + 0x60), t);
}

// Copies the state record at src into dst (fields 0x4..0x64 except 0x0).
// FUNCTION: CMR2 0x00423ee0
void FUN_00423ee0(BYTE *dst, BYTE *src)
{
    *(int *)(dst + 0x4) = *(int *)(src + 0x4);
    *(Unk00423ee0Block *)(dst + 0x8) = *(Unk00423ee0Block *)(src + 0x8);
    *(int *)(dst + 0x48) = *(int *)(src + 0x48);
    *(int *)(dst + 0x4c) = *(int *)(src + 0x4c);
    *(int *)(dst + 0x54) = *(int *)(src + 0x54);
    *(int *)(dst + 0x58) = *(int *)(src + 0x58);
    *(int *)(dst + 0x5c) = *(int *)(src + 0x5c);
    *(int *)(dst + 0x50) = *(int *)(src + 0x50);
    *(int *)(dst + 0x60) = *(int *)(src + 0x60);
}

// Scale of the render distance for the detail level: base * (1 + step).
// FUNCTION: CMR2 0x00423f30
int FUN_00423f30(void)
{
    float steps[10];
    float base;

    steps[0] = -0.5f;
    steps[1] = 0.0f;
    steps[2] = 0.5f;
    steps[3] = 1.0f;
    steps[4] = 1.5f;
    steps[5] = 2.0f;
    steps[6] = 2.5f;
    steps[7] = 3.0f;
    steps[8] = 3.5f;
    steps[9] = 4.0f;
    base = (float)(*(int *)&g_pGraphics->field921_0x3c4 * CGraphics::m_oneOver65536);
    float scale = base * steps[CGameInfo::FUN_00405ca0()];

    return (int)(__int64)((scale + base) * CGraphics::m_65536);
}

// FUNCTION: CMR2 0x00423fc0
void FUN_00423fc0(int view)
{
    FUN_0048df10(VIEW_MODE_RECORD(view));
}

// FUNCTION: CMR2 0x00421980
void FUN_00421980(unsigned int view)
{
    view &= 0xff;
    FUN_0048d930(VIEW_MODE_RECORD(view));
}

// FUNCTION: CMR2 0x00437f90
int FUN_00437f90(void)
{
    return FixMul(g_pCurrentCar->field_0x79c, *(int *)(g_pCarSetup + 0x404));
}

int FUN_00476520(BYTE index);
int FUN_0048ca90(void);

// Whether view mode `mode` is available for car `index`.
// FUNCTION: CMR2 0x004232a0
int FUN_004232a0(int index, int mode)
{
    int result;

    result = 0;
    switch (mode) {
    case 1:
    case 2:
    case 4:
    case 5:
    case 6:
    case 10:
        return 1;
    case 7:
        return (unsigned int)FUN_0048ca90() > 0;
    case 3:
        result = FUN_00476520((BYTE)index);
        break;
    }
    return result;
}

// Clears `count` car records (0xc24 bytes) from `first`.
// FUNCTION: CMR2 0x0042b740
void FUN_0042b740(int first, int count)
{
    int i;

    for (i = first; i < count + first; i++)
        memset((BYTE *)g_carBuffer + i * 0xc24, 0, 0xc24);
}

float FUN_004b23a0(void);
void FUN_004b23b0(float value);
void FUN_00466630(int value);
extern int g_unk0x0053c9a8;

// GLOBAL: CMR2 0x005210c8
float g_65536f = 65536.0f;

// Counts down the frame-rate hold (restoring rate 1.0 when it ends) and
// passes the current rate on to the stage objects.
// FUNCTION: CMR2 0x0042b790
void FUN_0042b790(void)
{
    if (g_unk0x0053c9a8 != 0) {
        if (g_unk0x0053c9a8 == 1)
            FUN_004b23b0(1.0f);
        g_unk0x0053c9a8--;
    }
    {
        float rate = FUN_004b23a0();
        int scaled;

        __asm {
            fld rate
            fmul g_65536f
            fistp scaled
        }
        FUN_00466630(scaled);
    }
}

// Sets the physics scale (value / 25, at least 0.6) and the time step
// (its reciprocal).
// FUNCTION: CMR2 0x00433840
void FUN_00433840(int value)
{
    g_physicsScale = FixMul(0xa3d, value);
    if (g_physicsScale < 0x9999)
        g_physicsScale = 0x9999;
    g_physicsTimeStep = FixDiv(0x10000, g_physicsScale);
}

void FixMatrix_GetForward(FixVector *pOut, FixMatrix *pM);

// Heading (12-bit angle) of a view's camera from its forward vector.
// match 32%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00421fe0
void FUN_00421fe0(short *pOut, unsigned int view)
{
    FixVector forward;
    unsigned int az;
    unsigned int ax;
    short angle;

    FixMatrix_GetForward(&forward, (FixMatrix *)(g_unk0x00538d2c + 4 + (view & 0xff) * 100));
    az = forward.z < 0 ? -forward.z : forward.z;
    ax = forward.x < 0 ? -forward.x : forward.x;
    if (ax == 0) {
        angle = 0;
    } else if (az == 0) {
        angle = 0x400;
    } else if ((int)az < (int)ax) {
        angle = 0x400 - g_atanTable[(unsigned int)FixDiv(az, ax) >> 7];
    } else {
        angle = g_atanTable[(unsigned int)FixDiv(ax, az) >> 7];
    }
    *pOut = angle;
    if (forward.z < 0)
        *pOut = 0x800 - angle;
    if (forward.x < 0)
        *pOut = -*pOut;
}

void FixMatrix_CopyRotationFrom(FixMatrix *pDst, FixMatrix *pSrc);
void FixMatrix_SetPosition(FixVector *pV, FixMatrix *pM);

// A car's body matrix, raised by its camera shake when that option is on.
// match 33%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00423a30
FixMatrix *FUN_00423a30(FixMatrix *pOut, BYTE car)
{
    FixVector up;
    FixVector position;
    int shake;

    FixMatrix_CopyRotationFrom(pOut, Car_Get(car)->pBodyMatrix);
    if (CGameInfo::FUN_004063f0(6)) {
        FixMatrix_GetUp(&up, Car_Get(car)->pWorld);
        shake = FixMul(Car_Get(car)->field_0xa8c, 0x8000);
        up.x = FixMul(up.x, shake);
        up.y = FixMul(up.y, shake);
        up.z = FixMul(up.z, shake);
        FixMatrix_GetPosition(&position, pOut);
        position.y += up.y;
        position.x += up.x;
        position.z += up.z;
        FixMatrix_SetPosition(&position, pOut);
    }
    return pOut;
}

int RallyData_FUN_00421500(void);
void RallyData_FUN_00421530(int index, int *pOut);
int StageObject_UsesExtendedMode(void);

typedef void (*CarFadeCallback)(BYTE index);
void FUN_004283e0(BYTE index, CarFadeCallback pfnDone, int param3, int param4, int param5, char force);

// Starts the reset fade for the current car when its reset request is active.
// FUNCTION: CMR2 0x00431c10
void FUN_00431c10(void)
{
    int zero = 0;

    if (g_pCurrentCar->field_0xbf8 != 0)
        FUN_004283e0((BYTE)g_pCurrentCar->field_0xb1a, (CarFadeCallback)0x431c50, 2, 1, zero, (char)zero);
}

// Relative position of the current car against every other car in the race
// order: returns 1 as soon as one overlaps it (distance below the sum of the
// two body radii), keeping the relative position in 0x53ca58 and its largest
// absolute component in 0x53ca24.
// GLOBAL: CMR2 0x0053c9c4
int g_unk0x0053c9c4;
// GLOBAL: CMR2 0x0053ca24
int g_unk0x0053ca24;
// GLOBAL: CMR2 0x0053ca58
FixVector g_unk0x0053ca58;
// GLOBAL: CMR2 0x0053cae8
FixVector g_unk0x0053cae8;
// match 69%: same logic; MSVC kept the loop counter in the parameter slot and
// in EDI in the original, and in ESI here (register numbering).
// FUNCTION: CMR2 0x00431d80
int FUN_00431d80(int param_1)
{
    short *pOrder;
    short count;
    short i;
    int result;
    Car *pOther;
    int inv;

    result = 0;
    if (param_1 == 0 && RallyData_FUN_00421500() == 0)
        return 0;
    pOrder = Car_GetOrder();
    count = Car_GetOrderCount();
    RallyData_FUN_00421530(param_1, (int *)&g_unk0x0053cae8);
    for (i = 0; i < count; i++) {
        pOther = Car_Get(pOrder[i]);
        if (pOther->field_0xb1a != g_pCurrentCar->field_0xb1a && StageObject_UsesExtendedMode() != 0) {
            g_unk0x0053ca58.x = g_unk0x0053cae8.x - pOther->position.x;
            g_unk0x0053ca58.y = g_unk0x0053cae8.y - pOther->position.y;
            g_unk0x0053ca58.z = g_unk0x0053cae8.z - pOther->position.z;
            if (FIX_ABS(g_unk0x0053ca58.x) > FIX_ABS(g_unk0x0053ca58.y) &&
                FIX_ABS(g_unk0x0053ca58.x) > FIX_ABS(g_unk0x0053ca58.z))
                g_unk0x0053ca24 = FIX_ABS(g_unk0x0053ca58.x);
            else if (FIX_ABS(g_unk0x0053ca58.y) > FIX_ABS(g_unk0x0053ca58.x) &&
                     FIX_ABS(g_unk0x0053ca58.y) > FIX_ABS(g_unk0x0053ca58.z))
                g_unk0x0053ca24 = FIX_ABS(g_unk0x0053ca58.y);
            else
                g_unk0x0053ca24 = FIX_ABS(g_unk0x0053ca58.z);
            if (g_unk0x0053ca24 != 0) {
                inv = (int)(0x100000000 / (__int64)g_unk0x0053ca24);
                g_unk0x0053ca58.x = FixMul(g_unk0x0053ca58.x, inv);
                g_unk0x0053ca58.y = FixMul(g_unk0x0053ca58.y, inv);
                g_unk0x0053ca58.z = FixMul(g_unk0x0053ca58.z, inv);
            }
            g_unk0x0053c9c4 = FixSqrt(FixMul(g_unk0x0053ca58.x, g_unk0x0053ca58.x) +
                                      FixMul(g_unk0x0053ca58.y, g_unk0x0053ca58.y) +
                                      FixMul(g_unk0x0053ca58.z, g_unk0x0053ca58.z));
            g_unk0x0053c9c4 = FixMul(g_unk0x0053c9c4, g_unk0x0053ca24);
            if (g_unk0x0053c9c4 < *(int *)pOther->field_0x758 + *(int *)g_pCurrentCar->field_0x758) {
                result = 1;
                break;
            }
        }
    }
    return result;
}

// Counts the current car's wheels near the ground and shares its weight out.
// match 35%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00432b30
void FUN_00432b30(void)
{
    int i;
    int d;

    g_pCurrentCar->field_0xb28 = 0;
    for (i = 0; i < 4; i++) {
        d = g_pCurrentCar->cornerHeight[i] - g_pCurrentCar->corners[i].y;
        if (d < 0)
            d = g_pCurrentCar->corners[i].y - g_pCurrentCar->cornerHeight[i];
        if (d < 0xcccc) {
            g_pCurrentCar->field_0xbac[i] = 1;
            g_pCurrentCar->field_0xb28++;
        } else {
            g_pCurrentCar->field_0xbac[i] = 0;
        }
    }
    switch (g_pCurrentCar->field_0xb28) {
    case 0:
        g_pCurrentCar->field_0x8b4 = 0;
        return;
    case 4:
        g_pCurrentCar->field_0x8b4 = g_pCurrentCar->field_0x75c / 4;
        return;
    case 2:
        g_pCurrentCar->field_0x8b4 = g_pCurrentCar->field_0x75c / 2;
        return;
    case 1:
        g_pCurrentCar->field_0x8b4 = g_pCurrentCar->field_0x75c;
        return;
    }
    g_pCurrentCar->field_0x8b4 = g_pCurrentCar->field_0x75c / g_pCurrentCar->field_0xb28;
}

void FUN_00437fd0(void);

// Updates the engine torque figure and splits it between the wheels, or
// clears the wheel torques when the car is off its wheels.
// match 57%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00437dc0
void FUN_00437dc0(void)
{
    int value;

    if (g_pCurrentCar->field_0xb78 != 0) {
        if (g_pCurrentCar->field_0xb1e != 0)
            g_pCurrentCar->field_0x780 = -g_pCurrentCar->field_0x7b0;
        else
            g_pCurrentCar->field_0x780 = 0;
    } else {
        value = g_pCurrentCar->field_0x7a4;
        if (value >= g_pCurrentCar->field_0x794) {
            g_pCurrentCar->field_0x780 = 0;
        } else if (g_pCurrentCar->field_0xb1e != 0 && g_pCurrentCar->field_0xb84 == 1) {
            g_pCurrentCar->field_0x780 = FixMul(g_pCurrentCar->field_0x784, FixMul(value, value));
        } else {
            g_pCurrentCar->field_0x780 =
                FUN_00437f90() - FixMul(g_pCurrentCar->field_0x784, FixMul(g_pCurrentCar->field_0x7a4, g_pCurrentCar->field_0x7a4));
        }
    }
    if (g_pCurrentCar->field_0xb1e != 0 && g_pCurrentCar->field_0xb84 == 0) {
        int torque = FixMul(g_pCurrentCar->field_0x780, g_pCurrentCar->field_0x7bc[g_pCurrentCar->field_0xb1e]);
        int front = FixMul(torque, g_pCurrentCar->field_0x7b4) / 2;
        g_pCurrentCar->wheelTorque[0] = front;
        g_pCurrentCar->wheelTorque[1] = front;
        g_pCurrentCar->wheelTorque[2] = torque / 2 - front;
        g_pCurrentCar->wheelTorque[3] = torque / 2 - front;
    } else {
        g_pCurrentCar->wheelTorque[0] = 0;
        g_pCurrentCar->wheelTorque[1] = 0;
        g_pCurrentCar->wheelTorque[2] = 0;
        g_pCurrentCar->wheelTorque[3] = 0;
    }
    Car_UpdateTyreForces();
    FUN_00437fd0();
}

// Filters each driven wheel's spin (peak hold with decay) while it keeps the
// same surface and touches the ground.
// FUNCTION: CMR2 0x00437fd0
void FUN_00437fd0(void)
{
    int i;
    int a;
    int d;

    for (i = 0; i < 4; i++) {
        if (g_pCurrentCar->field_0xabe[i] == g_pCurrentCar->wheelSurface[i] && g_pCurrentCar->field_0xbac[i] != 0 &&
            g_pCurrentCar->field_0xb74 != 0) {
            a = g_pCurrentCar->field_0x870[i] < 0 ? -g_pCurrentCar->field_0x870[i] : g_pCurrentCar->field_0x870[i];
            d = a - g_pCurrentCar->field_0x890[i];
            if (d < 0)
                g_pCurrentCar->field_0x890[i] = a;
            else
                g_pCurrentCar->field_0x890[i] += FixMul(d, *(int *)((BYTE *)g_pCurrentCar + 0x98 + i * 0x24));
            a = g_pCurrentCar->field_0x880[i] < 0 ? -g_pCurrentCar->field_0x880[i] : g_pCurrentCar->field_0x880[i];
            d = a - g_pCurrentCar->field_0x8a0[i];
            if (d < 0)
                g_pCurrentCar->field_0x8a0[i] = a;
            else
                g_pCurrentCar->field_0x8a0[i] += FixMul(d, *(int *)((BYTE *)g_pCurrentCar + 0x98 + i * 0x24));
        } else {
            g_pCurrentCar->field_0x8a0[i] = 0;
            g_pCurrentCar->field_0x890[i] = 0;
        }
    }
}

// GLOBAL: CMR2 0x00538d20
int g_unk0x00538d20[2];
// GLOBAL: CMR2 0x005391a8
int g_unk0x005391a8[2];
// GLOBAL: CMR2 0x005391b0
int g_unk0x005391b0[2];
// GLOBAL: CMR2 0x005391c4
int g_unk0x005391c4[2];

void FUN_00447ca0(unsigned int index);
int SceneNode_Destroy(SceneNode *pNode);

// Releases both players' view nodes and resets their view state (callback).
// match 33%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00421590
int FUN_00421590(void)
{
    BYTE i;

    for (i = 0; i < 2; i++) {
        if (g_viewNodes[i] != NULL) {
            SceneNode_Destroy(g_viewNodes[i]);
            g_viewNodes[i] = NULL;
        }
        *(int *)(g_unk0x00538d2c + i * 100) = 0;
        g_unk0x00538d20[i] = -0x10000;
        g_unk0x00538e0c[i] = 0;
        g_unk0x005391b0[i] = 0;
        g_unk0x005391c4[i] = 0;
        g_unk0x005391a8[i] = 0;
        g_unk0x00538f00[i] = 0;
        FUN_00447ca0(i);
    }
    return 1;
}

// Lifts the current car out of the ground by the deepest penetration of a
// free corner.
// match 14%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0042f8c0
void FUN_0042f8c0(void)
{
    int found = 0;
    int deepest = 0;
    int i;
    int d;
    int lift;

    for (i = 7; i >= 0; i--) {
        if (g_pCurrentCar->cornerFlags[i] == 0) {
            d = g_pCurrentCar->cornerHeight[i] - g_pCurrentCar->corners[i].y;
            if (d > 0 && deepest < d) {
                found = 1;
                deepest = d;
            }
        }
    }
    lift = 0;
    if (found)
        lift = deepest;
    g_pCurrentCar->position.y += lift;
    for (i = 0; i < 8; i++)
        g_pCurrentCar->corners[i].y += lift;
}

// Lifts the current car out of the ground by the deepest penetration of a
// free corner, rising or sinking as needed.
// FUNCTION: CMR2 0x0042f9d0
void FUN_0042f9d0(void)
{
    int lift = 0;
    int maxDrop = 0;
    int maxRise = -0x3e80000;
    int foundDrop = 0;
    int foundRise = 0;
    int i;
    int d;

    for (i = 7; i >= 0; i--) {
        if (g_pCurrentCar->cornerFlags[i] != 0)
            continue;
        d = g_pCurrentCar->cornerHeight[i] - g_pCurrentCar->corners[i].y;
        if (d >= 0 && d > maxDrop) {
            maxDrop = d;
            foundDrop = 1;
        }
        if (d <= 0 && d > maxRise) {
            maxRise = d;
            foundRise = 1;
        }
    }
    if (foundDrop)
        lift = maxDrop;
    else if (foundRise)
        lift = maxRise;
    g_pCurrentCar->position.y += lift;
    g_pCurrentCar->corners[0].y += lift;
    g_pCurrentCar->corners[1].y += lift;
    g_pCurrentCar->corners[2].y += lift;
    g_pCurrentCar->corners[3].y += lift;
    g_pCurrentCar->corners[4].y += lift;
    g_pCurrentCar->corners[5].y += lift;
    g_pCurrentCar->corners[6].y += lift;
    g_pCurrentCar->corners[7].y += lift;
}

// Per-wheel spin flag of each car (load change above half a unit this step).
// GLOBAL: CMR2 0x0053ac48
int g_unk0x0053ac48[8][4];

// Integrates each car's wheel rotation angles from the wheel loads and flags
// the wheels spinning fast.
// match 65%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0042b4a0
void FUN_0042b4a0(short *pList, short count)
{
    int n;
    int index;
    int w;
    int v;
    Car *pCar;
    short *p;

    for (n = count, p = pList + count - 1; n > 0; n--, p--) {
        index = *p;
        pCar = &g_carBuffer[index];
        for (w = 0; w < 4; w++) {
            v = FixMul(pCar->wheelLoad[w], g_physicsTimeStep);
            if (pCar->field_0xb60 == 0 || pCar->flag0x1d0[2] > 0 || pCar->flag0x1d0[3] > 0 ||
                pCar->field_0x1d8 > 0)
                (&g_unk0x0053a230[index].a)[w] += (short)(__int64)(v * -0.009947183943243459);
            if (v < 0)
                v = -v;
            if (v <= 0x8000)
                g_unk0x0053ac48[index][w] = 0;
            else
                g_unk0x0053ac48[index][w] = 1;
        }
    }
}

int FUN_0045c720(char car, int wheel);
void FUN_0049c440(Mesh *pMesh, int mask, int value);

// Sets four wheel mesh states for each car in draw order.
// FUNCTION: CMR2 0x0042be30
void FUN_0042be30(void)
{
    int order;
    int wheel;
    int car;
    int state;
    int appearance;
    BYTE *pNode;

    for (order = g_carOrderCount - 1; order >= 0; order--) {
        car = g_carOrder[order];
        for (wheel = 0; wheel < 4; wheel++) {
            state = FUN_0045c720(car, wheel);
            if (g_unk0x0053ac48[car][wheel] != 0) {
                switch (state) {
                case 0: appearance = 6; break;
                case 1: appearance = 4; break;
                case 2: appearance = 7; break;
                case 3: appearance = 9; break;
                default: appearance = 6; break;
                }
            } else {
                switch (state) {
                case 0: appearance = 0; break;
                case 1: appearance = 3; break;
                case 2: appearance = 5; break;
                case 3: appearance = 8; break;
                default: appearance = 0; break;
                }
            }
            pNode = *(BYTE **)((BYTE *)g_carBuffer + car * 0xc24 + 0x738 + wheel * 4);
            FUN_0049c440(*(Mesh **)(pNode + 0xc), 1, appearance);
            pNode = *(BYTE **)((BYTE *)g_carBuffer + car * 0xc24 + 0x728 + wheel * 4);
            if (pNode != NULL)
                FUN_0049c440(*(Mesh **)(pNode + 0xc), 1, appearance);
        }
    }
}

// Smooths the current car's three force feedback levels.
// match 20%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00442e90
void FUN_00442e90(void)
{
    BYTE *car = (BYTE *)g_pCurrentCar;
    int i;
    int base = 0x880;
    int tag = 0xab6;
    int rate = 0x128;

    for (i = 0; i < 3; i++, base -= 8, tag -= 4, rate -= 0x48) {
        if (*(short *)(car + tag + 0x10) == *(short *)(car + tag) &&
            *(int *)(car + base + 0x33c) != 0 && *(int *)(car + 0xb74) != 0) {
            int magnitude = *(int *)(car + base);
            int delta;
            if (magnitude < 0) magnitude = -magnitude;
            delta = magnitude - *(int *)(car + base + 0x20);
            if (delta < 0)
                *(int *)(car + base + 0x20) = magnitude;
            else
                *(int *)(car + base + 0x20) += FixMul(delta, *(int *)(car + rate));

            magnitude = *(int *)(car + base + 0x10);
            if (magnitude < 0) magnitude = -magnitude;
            delta = magnitude - *(int *)(car + base + 0x30);
            if (delta < 0)
                *(int *)(car + base + 0x30) = magnitude;
            else
                *(int *)(car + base + 0x30) += FixMul(delta, *(int *)(car + rate));
        } else {
            *(int *)(car + base + 0x20) = 0;
            *(int *)(car + base + 0x30) = 0;
        }
    }
    *(int *)(car + 0x8a0) = *(int *)(car + 0x8a4);
    *(int *)(car + 0x890) = *(int *)(car + 0x8a4);
    *(int *)(car + 0x8a8) = *(int *)(car + 0x8ac);
    *(int *)(car + 0x898) = *(int *)(car + 0x8ac);
}

// GLOBAL: CMR2 0x005391cc
int g_unk0x005391cc[2];

extern const double g_unk0x00511380;

// Integrates contact forces and body motion, including the transition to tumbling.
// match 70%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00435470
void Car_IntegrateContacts(void)
{
    FixVector force, torque, tmp, bodyForce, cross, accel, angAccel, posStep, angStep;
    short angles[3];
    int i, corner, scale, drag;

    Car_UpdateBodyAxes();
    Car_UpdateRollover();
    Car_DampenBodyMotion();
    torque.x = torque.y = torque.z = 0;
    force = g_pCurrentCar->baseForce;
    if (CGameInfo::FUN_004063f0(1)) {
        FixVecScale(&force, &force, 0x6666);
    } else {
        FixVecScale(&force, &force, 0xcccc);
    }
    if (g_pCurrentCar->field_0xb34 > 0) {
        scale = FixMul(g_pCurrentCar->speed, 0x13333);
        if (scale > 0x13333) scale = 0x13333;
        scale += 0x10000;
        for (i = g_pCurrentCar->field_0xb34 - 1; i >= 0; i--) {
            corner = *(signed char *)((BYTE *)g_pCurrentCar + 0xb36 + i);
            if (corner >= 4) {
                FixVecScale(&tmp, &g_pCurrentCar->cornerForce[corner], scale);
                tmp.x += g_pCurrentCar->cornerLoad[corner].x;
                tmp.y += g_pCurrentCar->cornerLoad[corner].y;
                tmp.z += g_pCurrentCar->cornerLoad[corner].z;
                force.x += g_pCurrentCar->cornerForce[corner].x;
                force.y += g_pCurrentCar->cornerForce[corner].y;
                force.z += g_pCurrentCar->cornerForce[corner].z;
            } else if (g_pCurrentCar->field_0xc00 != 0) {
                if (g_pCurrentCar->field_0xb74 != 0) {
                    tmp = g_pCurrentCar->cornerForce[corner];
                } else {
                    FixVecScale(&tmp, &g_pCurrentCar->cornerForce[corner], scale);
                }
                tmp.x += g_pCurrentCar->cornerLoad[corner].x;
                tmp.y += g_pCurrentCar->cornerLoad[corner].y;
                tmp.z += g_pCurrentCar->cornerLoad[corner].z;
            } else {
                tmp.x = tmp.y = tmp.z = 0;
            }
            FixMatrix_InverseRotateVector(&bodyForce, &tmp, g_pCurrentCar->pWorld);
            // The eight local corner positions start at 0x210 (four are named wheelPos).
            FixVecCross(&cross, &bodyForce, (FixVector *)((BYTE *)g_pCurrentCar + 0x210 + corner * 12));
            torque.x += cross.x;
            torque.y += cross.y;
            torque.z += cross.z;
        }
        if (g_pCurrentCar->field_0xc00 != 0) {
            if ((torque.x > 0 && g_pCurrentCar->angularVelocity.x < 0) ||
                (torque.x < 0 && g_pCurrentCar->angularVelocity.x > 0)) {
                int damping = FixMul(FIX_ABS(g_pCurrentCar->angularVelocity.x), 0x50000);
                if (damping > 0x10000) damping = 0x10000;
                torque.x = FixMul(torque.x, FixMul(0x10000 - damping, 0xf333));
            }
            if ((torque.y > 0 && g_pCurrentCar->angularVelocity.y < 0) ||
                (torque.y < 0 && g_pCurrentCar->angularVelocity.y > 0)) {
                int damping = FixMul(FIX_ABS(g_pCurrentCar->angularVelocity.y), 0x50000);
                if (damping > 0x10000) damping = 0x10000;
                torque.y = FixMul(torque.y, FixMul(0x10000 - damping, 0xcccc));
            }
            if ((torque.z > 0 && g_pCurrentCar->angularVelocity.z < 0) ||
                (torque.z < 0 && g_pCurrentCar->angularVelocity.z > 0)) {
                int damping = FixMul(FIX_ABS(g_pCurrentCar->angularVelocity.z), 0x50000);
                if (damping > 0x10000) damping = 0x10000;
                torque.z = FixMul(torque.z, FixMul(0x10000 - damping, 0xf333));
            }
        }
        for (i = 3; i >= 0; i--) {
            if (g_pCurrentCar->field_0xbac[i] != 0) {
                tmp = g_pCurrentCar->cornerForce[i];
                force.x += tmp.x;
                force.y += tmp.y;
                force.z += tmp.z;
                if (g_pCurrentCar->field_0xc00 == 0) {
                    FixMatrix_InverseRotateVector(&bodyForce, &tmp, g_pCurrentCar->pWorld);
                    FixVecCross(&cross, &bodyForce, &g_pCurrentCar->wheelPos[i]);
                    torque.x += cross.x;
                    torque.y += cross.y;
                    torque.z += cross.z;
                }
            }
        }
    }
    if (g_pCurrentCar->field_0xc00 == 0 &&
        FixMul(FIX_ABS(g_pCurrentCar->field_0x5d0.x) + FIX_ABS(g_pCurrentCar->field_0x5d0.z),
               g_pCurrentCar->field_0x75c) > 0xbb80000) {
        g_pCurrentCar->field_0xc00 = 1;
        g_pCurrentCar->field_0x96c = 0x10000;
        *(int *)g_pCurrentCar->field_0xc04 = 1;
        g_pCurrentCar->field_0x91c = FixVecDot(&g_pCurrentCar->groundNormal, &g_pCurrentCar->right);
        g_pCurrentCar->field_0x920 = FixVecDot(&g_pCurrentCar->groundNormal, &g_pCurrentCar->up);
        g_pCurrentCar->field_0x924 = FixVecDot(&g_pCurrentCar->groundNormal, &g_pCurrentCar->forward);
    }
    drag = FixMul(FIX_ABS(g_pCurrentCar->speed), *(int *)(g_pCarSetup + 0x408) + g_dragBase);
    FixVecScale(&tmp, &g_pCurrentCar->velocity, drag);
    force.x -= tmp.x;
    force.y -= tmp.y;
    force.z -= tmp.z;
    FixVecScale(&accel, &force, g_pCurrentCar->field_0x760);
    accel.x += g_pCurrentCar->field_0x5c4.x;
    accel.y += g_pCurrentCar->field_0x5c4.y;
    accel.z += g_pCurrentCar->field_0x5c4.z;
    angAccel.x = -FixMul(torque.x, g_pCurrentCar->inertia.x);
    angAccel.y = -FixMul(torque.y, g_pCurrentCar->inertia.y);
    angAccel.z = -FixMul(torque.z, g_pCurrentCar->inertia.z);
    angAccel.x -= FixMul(g_pCurrentCar->field_0x5d0.x, FixMul(g_pCurrentCar->inertia.x, g_pCurrentCar->field_0x75c));
    angAccel.y -= FixMul(g_pCurrentCar->field_0x5d0.y, FixMul(g_pCurrentCar->inertia.y, g_pCurrentCar->field_0x75c));
    angAccel.z -= FixMul(g_pCurrentCar->field_0x5d0.z, FixMul(g_pCurrentCar->inertia.z, g_pCurrentCar->field_0x75c));
    FixVecScale(&accel, &accel, 0x3333);
    g_pCurrentCar->velocityNext = g_pCurrentCar->velocity;
    FixVecScale(&accel, &accel, g_physicsTimeStep);
    g_pCurrentCar->velocity.x += accel.x;
    g_pCurrentCar->velocity.y += accel.y;
    g_pCurrentCar->velocity.z += accel.z;
    g_pCurrentCar->speed = FixVecLength(&g_pCurrentCar->velocity);
    FixVecScale(&angAccel, &angAccel, g_physicsTimeStep);
    g_pCurrentCar->angularVelocity.x += angAccel.x;
    g_pCurrentCar->angularVelocity.y += angAccel.y;
    g_pCurrentCar->angularVelocity.z += angAccel.z;
    FixVecScale(&posStep, &g_pCurrentCar->velocity, g_physicsTimeStep);
    FixVecScale(&accel, &accel, g_physicsTimeStep / 2);
    posStep.x -= accel.x;
    posStep.y -= accel.y;
    posStep.z -= accel.z;
    if (g_pCurrentCar->steepTime != 0) {
        if (FIX_ABS(g_pCurrentCar->angularVelocity.x) > 0x3333)
            g_pCurrentCar->angularVelocity.x = g_pCurrentCar->angularVelocity.x > 0 ? 0x3333 : -0x3333;
        if (FIX_ABS(g_pCurrentCar->angularVelocity.y) > 0x3333)
            g_pCurrentCar->angularVelocity.y = g_pCurrentCar->angularVelocity.y > 0 ? 0x3333 : -0x3333;
        if (FIX_ABS(g_pCurrentCar->angularVelocity.z) > 0x3333)
            g_pCurrentCar->angularVelocity.z = g_pCurrentCar->angularVelocity.z > 0 ? 0x3333 : -0x3333;
    }
    if (g_pCurrentCar->field_0xc00 == 0) {
        FixVecScale(&angStep, &g_pCurrentCar->angularVelocity, g_physicsTimeStep);
        FixVecScale(&angAccel, &angAccel, g_physicsTimeStep / 2);
        angStep.x -= angAccel.x;
        angStep.y -= angAccel.y;
        angStep.z -= angAccel.z;
        angles[0] = (short)(__int64)((double)angStep.x * g_unk0x00511380);
        angles[1] = (short)(__int64)((double)angStep.y * g_unk0x00511380);
        angles[2] = (short)(__int64)((double)angStep.z * g_unk0x00511380);
    } else {
        angles[0] = (short)(__int64)((double)FixMul(g_pCurrentCar->angularVelocity.x, g_physicsTimeStep) * g_unk0x00511380);
        angles[1] = (short)(__int64)((double)FixMul(g_pCurrentCar->angularVelocity.y, g_physicsTimeStep) * g_unk0x00511380);
        angles[2] = (short)(__int64)((double)FixMul(g_pCurrentCar->angularVelocity.z, g_physicsTimeStep) * g_unk0x00511380);
    }
    if (g_pCurrentCar->field_0xb34 > 0) {
        int upDot = FixVecDot(&g_pCurrentCar->groundNormal, &g_pCurrentCar->up);
        if (FIX_ABS(angles[0]) < 10 && FIX_ABS(upDot) > 0xcccc) angles[0] = 0;
        if ((g_pCurrentCar->flag0x1d0[0] == 0 && g_pCurrentCar->flag0x1d0[1] == 0) ||
            FIX_ABS(g_pCurrentCar->speed) < 0x28f || g_pCurrentCar->field_0xb74 == 0) {
            if (FIX_ABS(angles[1]) < 10) angles[1] = 0;
        }
        if (FIX_ABS(angles[2]) < 10) angles[2] = 0;
        if (g_pCurrentCar->speed < 0xccc) posStep.x = posStep.y = posStep.z = 0;
    }
    if (g_pCurrentCar->field_0xbfc != 0) {
        if (g_pCurrentCar->field_0xbdc != 0) {
            g_pCurrentCar->angularVelocity.z = 0;
            g_pCurrentCar->angularVelocity.x = 0;
            angles[2] = angles[0] = 0;
        } else if (g_pCurrentCar->field_0xbe0 != 0) {
            g_pCurrentCar->angularVelocity.y = 0;
            g_pCurrentCar->angularVelocity.x = 0;
            angles[1] = angles[0] = 0;
        }
    }
    g_pCurrentCar->position.x += posStep.x;
    g_pCurrentCar->position.y += posStep.y;
    g_pCurrentCar->position.z += posStep.z;
    g_pCurrentCar->field_0xb60 = 0;
    if (angles[0] == 0 && angles[1] == 0 && angles[2] == 0) {
        if (g_pCurrentCar->speed < 0x51e) g_pCurrentCar->field_0xb60 = 1;
    } else {
        FixBasis_Rotate((FixBasis *)&g_pCurrentCar->right, (unsigned short *)angles);
    }
    g_pCurrentCar->pWorld->right = g_pCurrentCar->right;
    g_pCurrentCar->pWorld->up = g_pCurrentCar->up;
    g_pCurrentCar->pWorld->forward = g_pCurrentCar->forward;
    g_pCurrentCar->pWorld->position = g_pCurrentCar->position;
    Car_UpdateCorners(g_pCurrentCar);
}
