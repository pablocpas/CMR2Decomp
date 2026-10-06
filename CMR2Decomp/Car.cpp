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
SceneNode *g_viewNodes[3];
// Previous camera state of each view (two 100-byte records laid out like the
// view records; the first dword of each is unused). Its matrices start at 0x538e40.
// GLOBAL: CMR2 0x00538e38
BYTE g_unk0x00538e38[0xc8];
#define g_unk0x00538e40 (g_unk0x00538e38 + 8)
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
// View camera records: 2 per player (the player's two view modes).
// GLOBAL: CMR2 0x00539018
CameraRecord g_viewRecords[4];
#define g_unk0x0053901a ((BYTE *)g_viewRecords + 2)
#define g_unk0x0053901c ((BYTE *)g_viewRecords + 4)
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
void View_SetDistanceOverride(BYTE index, int value)
{
    g_unk0x00538e04[index] = value;
}

// FUNCTION: CMR2 0x00422f50
int View_GetActiveCameraMode(BYTE index)
{
    if (*(int *)(g_unk0x00538d2c + index * 100) != 8)
        return *(int *)(g_unk0x0053901c + ((unsigned int)g_unk0x00538e0c[index] + index * 2) * 100);
    return 8;
}

// FUNCTION: CMR2 0x00422fb0
BYTE View_GetActiveCameraFlags(BYTE index)
{
    return g_unk0x0053901a[((unsigned int)g_unk0x00538e0c[index] + index * 2) * 100];
}

// match 67%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00423d70
FixMatrix *Car_GetCameraReferenceMatrix(BYTE index)
{
    BYTE state = (BYTE)RallyDataState();
    return index < state ? &g_unk0x00538ca0[index] : Car_Get(index)->pWorld;
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

int NetPlayers_GetPlayerField8(int index);
BYTE *Race_GetPlayerRecordPointer(int index);
unsigned int NetPlayers_IsPlayerPresent(int index);
void Car_MarkHiddenFromCameras(Car *pCar);
extern short g_unk0x0053a314[8];
extern short g_unk0x0053bd6c[26];
extern short g_unk0x0053c9a0;

// GLOBAL: CMR2 0x0053a270
short g_unk0x0053a270[80];
// GLOBAL: CMR2 0x0053a310
short g_unk0x0053a310;
// GLOBAL: CMR2 0x0053b4f0
short g_unk0x0053b4f0[8];

// Builds the race car order of the current view: the active players first (and
// whoever reports itself), then splits them into the on-screen, shadowed and
// drawn lists and re-applies the view transform of the on-screen ones.
// FUNCTION: CMR2 0x00428810
void Car_BuildViewOrder(void)
{
    short *p;
    int i;
    int j;
    int flag;
    int pCar;
    if (CGameInfo::GetConfiguredGameMode() == 8 || CGameInfo::GetConfiguredGameMode() == 9 ||
        CGameInfo::GetConfiguredGameMode() == 0xb || CGameInfo::GetConfiguredGameMode() == 0xc) {
        g_carOrder[0] = 0;
        g_carOrderCount = 1;
        for (i = 0; i < 7; i++) {
            flag = 0;
            pCar = (int)Race_GetPlayerRecordPointer(i + 1);
            if (pCar != 0 && *(int *)(pCar + 4) != 0 && *(int *)(pCar + 0xe8) != 0)
                flag = 1;
            if ((BYTE)NetPlayers_IsPlayerPresent(i) != 0 || flag) {
                g_carOrder[g_carOrderCount] = NetPlayers_GetPlayerField8(i);
                g_carOrderCount++;
            }
        }
    }
    g_carOrder[26] = 0;
    g_unk0x0053a310 = 0;
    g_unk0x0053c9a0 = 0;
    g_carOrder[25] = 0;
    g_carOrder[44] = 0;
    j = 0;
    if (g_carOrderCount > 0) {
        p = g_carOrder;
        do {
            pCar = (int)((BYTE *)g_carBuffer + *p * 0xc24);
            *(int *)(pCar + 0xb70) = 0;
            *(int *)(pCar + 0xb6c) = 0;
            *(int *)(pCar + 0xb68) = 0;
            if (*(int *)(pCar + 0xc18) == 0) {
                if (*(BYTE *)(pCar + 0xb43) > 0) {
                    g_unk0x0053b4f0[g_unk0x0053a310] = *p;
                    g_unk0x0053a310++;
                    g_unk0x0053a314[g_unk0x0053c9a0] = *p;
                    g_unk0x0053c9a0++;
                    *(int *)(pCar + 0xb64) = 0;
                    goto nextCar;
                }
            } else {
nextCar:
                if (*(BYTE *)(pCar + 0xb43) > 0) {
                    g_unk0x0053a270[g_carOrder[44]] = *p;
                    g_carOrder[44]++;
                }
            }
            if (*(int *)(pCar + 0xc18) != 0 && *(BYTE *)(pCar + 0xb43) > 0) {
                g_unk0x0053bd6c[g_carOrder[25]] = *p;
                g_carOrder[25]++;
            }
            j++;
            p++;
        } while (j < g_carOrderCount);
    }
    i = 0;
    if (g_unk0x0053a310 > 0) {
        p = g_unk0x0053b4f0;
        do {
            Car_MarkHiddenFromCameras((Car *)((BYTE *)g_carBuffer + *p * 0xc24));
            i++;
            p++;
        } while (i < g_unk0x0053a310);
    }
}

unsigned char RallyData_GetSelectionFlag26(void);
BYTE *StageUI_GetRaceResultTable(void);

// Marks which player camera records are too far from, or face away from, the
// car. The fourth word is set when every active camera has been rejected.
// match 43%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00428a00
void Car_MarkHiddenFromCameras(Car *pCar)
{
    int *pRejected;
    FixMatrix *pView;
    FixVector position;
    FixVector forward;
    FixVector delta;
    int count;
    int rejected;
    int i;

    pRejected = (int *)((BYTE *)pCar + 0xb68);
    pCar->field_0xb70 = 0;
    pCar->field_0xb6c = 0;
    *pRejected = 0;
    if ((char)RallyData_GetSelectionFlag26() != 0 && **(char **)(StageUI_GetRaceResultTable() + 4) != '\n' &&
        **(char **)(StageUI_GetRaceResultTable() + 4) != '\t' && **(char **)(StageUI_GetRaceResultTable() + 4) != '\r') {
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
                    FIX_ABS(delta.z) > 0x3c0000)
                    *pRejected = 1;
                if (*pRejected == 0 && FixVecDot(&delta, &delta) > 0xe100000)
                    *pRejected = 1;
                if (*pRejected == 0 && FixVecDot(&forward, &delta) < 0)
                    *pRejected = 1;
            }
            if (*pRejected != 0)
                rejected++;
            pRejected++;
        }
        if (rejected == count)
            pCar->field_0xb70 = 1;
    }
}

unsigned char RallyData_GetSelectionFlag26(void);
int InRaceMenu_IsPlayerEditingCarSetup(unsigned int param1);
void StageObject_SetCarValuePendingFlag(int index, int reset);
extern float g_65536f;

// Picks the up to three cars to display in one split-screen view (nearest to
// the view position, in or near the viewport) and hides the remaining ones.
// match 59%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00428bf0
void View_SelectVisibleCars(unsigned int view, short *pRect)
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

    if (CGameInfo::IsRecordFlagSet(0x10))
        return;
    if ((char)RallyData_GetSelectionFlag26() && RallyData_IsHeadToHeadRaceMode() != 0) {
        StageObject_SetCarValuePendingFlag(0, 1);
        StageObject_SetCarValuePendingFlag(1, 1);
        return;
    }
    if (!((char)RallyData_GetSelectionFlag26() || CGameInfo::GetConfiguredGameMode() == 8 || CGameInfo::GetConfiguredGameMode() == 9 ||
          CGameInfo::GetConfiguredGameMode() == 10 || CGameInfo::GetConfiguredGameMode() == 11 ||
          CGameInfo::GetConfiguredGameMode() == 12 || CGameInfo::GetConfiguredGameMode() == 8))
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
                distTable[g_carOrder[i]] = FixVecDot(&delta, &delta);
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
            FixMatrix_ProjectWorldPointToView(proj1, &otherPos, (BYTE *)pView);
            proj1[0] >>= 16;
            proj1[1] >>= 16;
            FixVecScale(&delta, &up, Car_Get(g_carOrder[j])->field_0x758);
            delta.x += otherPos.x;
            delta.y += otherPos.y;
            delta.z += otherPos.z;
            FixMatrix_ProjectWorldPointToView(proj2, &delta, (BYTE *)pView);
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
                yy = FixMul(iy, 0x1e00000);
                yy = FixMul(yy, 0x28f);
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
            StageObject_SetCarValuePendingFlag(g_carOrder[i], visible[g_carOrder[i]] == 2);
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
        int change = g_pCurrentCar->field_0x7a4 - g_pCurrentCar->field_0x7a8;
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
void Car_SelectGearSpeedTable(unsigned int param_1)
{
    int i;
    int v;

    switch (param_1) {
    case 4:
        g_pCurrentCar->gearSpeed[0] = 0;
        g_pCurrentCar->gearSpeed[1] = 0x553f;
        g_pCurrentCar->gearSpeed[2] = 0x7893;
        g_pCurrentCar->gearSpeed[3] = 0x9ef9;
        g_pCurrentCar->gearSpeed[4] = 0xc28f;
        g_pCurrentCar->gearSpeed[5] = 0xe4dd;
        g_pCurrentCar->gearSpeed[6] = 0x10ac0;
        g_pCurrentCar->gearSpeed[7] = 0xffffaac1;
        break;
    case 3:
        g_pCurrentCar->gearSpeed[0] = 0;
        g_pCurrentCar->gearSpeed[1] = 0x5374;
        g_pCurrentCar->gearSpeed[2] = 0x73b6;
        g_pCurrentCar->gearSpeed[3] = 0x9604;
        g_pCurrentCar->gearSpeed[4] = 0xb4fd;
        g_pCurrentCar->gearSpeed[5] = 0xd26e;
        g_pCurrentCar->gearSpeed[6] = 0xf26e;
        g_pCurrentCar->gearSpeed[7] = 0xffffac8c;
        g_pCurrentCar->field_0x788 = g_pCurrentCar->field_0x788 + 0x1eb;
        break;
    case 2:
        g_pCurrentCar->gearSpeed[0] = 0;
        g_pCurrentCar->gearSpeed[1] = 0x5168;
        g_pCurrentCar->gearSpeed[2] = 0x6ed9;
        g_pCurrentCar->gearSpeed[3] = 0x8d0e;
        g_pCurrentCar->gearSpeed[4] = 0xa76c;
        g_pCurrentCar->gearSpeed[5] = 0xc000;
        g_pCurrentCar->gearSpeed[6] = 0xd9db;
        g_pCurrentCar->gearSpeed[7] = 0xffffae98;
        g_pCurrentCar->field_0x788 = g_pCurrentCar->field_0x788 + 0x3d7;
        break;
    case 1:
        g_pCurrentCar->gearSpeed[0] = 0;
        g_pCurrentCar->gearSpeed[1] = 0x4f9d;
        g_pCurrentCar->gearSpeed[2] = 0x69fb;
        g_pCurrentCar->gearSpeed[3] = 0x83d7;
        g_pCurrentCar->gearSpeed[4] = 0x9999;
        g_pCurrentCar->gearSpeed[5] = 0xad4f;
        g_pCurrentCar->gearSpeed[6] = 0xc189;
        g_pCurrentCar->gearSpeed[7] = 0xffffb063;
        g_pCurrentCar->field_0x788 = g_pCurrentCar->field_0x788 + 0x5c2;
        break;
    case 0:
        g_pCurrentCar->gearSpeed[0] = 0;
        g_pCurrentCar->gearSpeed[1] = 0x4d91;
        g_pCurrentCar->gearSpeed[2] = 0x651e;
        g_pCurrentCar->gearSpeed[3] = 0x7ae1;
        g_pCurrentCar->gearSpeed[4] = 0x8c08;
        g_pCurrentCar->gearSpeed[5] = 0x9ae1;
        g_pCurrentCar->gearSpeed[6] = 0xa8f5;
        g_pCurrentCar->gearSpeed[7] = 0xffffb26f;
        g_pCurrentCar->field_0x788 = g_pCurrentCar->field_0x788 + 0x7ae;
        break;
    }
    for (i = 0; i < 8; i++) {
        v = g_pCurrentCar->gearSpeed[i];
        g_pCurrentCar->gearSpeed[i] = FixDiv(v, 0x123d7);
    }
}

// Sets three handling factors of the current car from a 16.16 level.
// FUNCTION: CMR2 0x0043e530
void Car_SetDifficultyHandling(int level)
{
    g_pCurrentCar->field_0x804 = FixMul(0xcccc, level) + 0x9999;
    g_pCurrentCar->field_0x7fc = FixMul(0xa3d, level) + 0xccc;
    g_pCurrentCar->field_0x800 = FixMul(0x3333, level) + 0x1999;
}

BYTE *StageTiming_GetStartTableRecord(int index);
SceneNode *SceneNode_FindByType(SceneNode *pNode, unsigned int type);
int SceneNode_Reparent(SceneNode *pNode, SceneNode *pNewParent);
int RallyData_GetChallengeRenderState(void);
void Car_SwapWheelTextures(char mode, SceneNode **pWheels);

// Binds a car to its loaded model: body nodes, wheels and view nodes, and
// moves both body nodes under the stage root.
// FUNCTION: CMR2 0x0043e5a0
void Car_BindModel(int model, Car *pCar)
{
    BYTE *pModel = StageTiming_GetStartTableRecord(model);
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
    Car_SwapWheelTextures(StageTiming_GetStartTableRecord(pCar->index)[0x20], pCar->pWheelNodes);
    if (*(int *)((BYTE *)pBody + 8) != RallyData_GetChallengeRenderState())
        SceneNode_Reparent(pBody, (SceneNode *)RallyData_GetChallengeRenderState());
    if (*(int *)((BYTE *)pCar->pNode0x720 + 8) != RallyData_GetChallengeRenderState())
        SceneNode_Reparent(pCar->pNode0x720, (SceneNode *)RallyData_GetChallengeRenderState());
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
    int scale;
    int len;
    CarTransforms *pT;

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
            for (wheel = 0; wheel < 4; wheel++) {
                FixMatrix_CopyRotation(&g_carWheelTransforms[carIndex][wheel],
                                       &g_carBuffer[carIndex].pWheelNodes[wheel]->current);
            }
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
                    FixMatrix_SetPosition(&bodyPos, &g_carBuffer[carIndex].pNode0x71c->current);
                    FixMatrix_GetRight(&axis, &pT->body);
                    FixVecScale(&axis, &axis, scale);
                    FixMatrix_SetRight(&axis, &g_carBuffer[carIndex].pNode0x71c->current);
                    FixMatrix_GetUp(&axis, &pT->body);
                    FixVecScale(&axis, &axis, scale);
                    FixMatrix_SetUp(&axis, &g_carBuffer[carIndex].pNode0x71c->current);
                    FixMatrix_GetForward(&axis, &pT->body);
                    FixVecScale(&axis, &axis, scale);
                    FixMatrix_SetForward(&axis, &g_carBuffer[carIndex].pNode0x71c->current);
                    FixMatrix_SetPosition(&bodyPos, &g_carBuffer[carIndex].pNode0x720->current);
                    FixMatrix_GetRight(&axis, &pT->body2);
                    FixVecScale(&axis, &axis, scale);
                    FixMatrix_SetRight(&axis, &g_carBuffer[carIndex].pNode0x720->current);
                    FixMatrix_GetUp(&axis, &pT->body2);
                    FixVecScale(&axis, &axis, scale);
                    FixMatrix_SetUp(&axis, &g_carBuffer[carIndex].pNode0x720->current);
                    FixMatrix_GetForward(&axis, &pT->body2);
                    FixVecScale(&axis, &axis, scale);
                    FixMatrix_SetForward(&axis, &g_carBuffer[carIndex].pNode0x720->current);
                    carIndex = i;
                }
            } else {
                FixMatrix_CopyRotation(&pT->body, &g_carBuffer[carIndex].pNode0x71c->current);
                FixMatrix_CopyRotation(&pT->body2, &g_carBuffer[carIndex].pNode0x720->current);
            }
            g_carViewScale[carIndex][viewIndex] = scale;
            pOrder--;
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
    FixVector pos;
    FixVector viewPos;
    FixVector local;
    FixVector carPos;
    FixVector delta;
    Car *pCar;
    SceneNode *p;

    nearDist = 0x1999;
    farDist = 0x8000;
    FixMatrix_GetPosition(&viewPos, &g_viewNodes[viewIndex]->current);
    i = g_carOrderCount - 1;
    if (i >= 0) {
        n = i + 1;
        pOrder = &g_carOrder[i];
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
            n--;
            pOrder--;
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
            localUp.x = g_sinTable[(unsigned short)(g_pCurrentCar->heading + 0x400) & 0xfff];
            localUp.y = 0;
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
                g_pCurrentCar->right = localUp;
            } else {
                FixVecScaleRecip(&localUp, &diff, step);
                FixVecScale(&localUp, &localUp, maxStep);
                localUp.x = localUp.x + g_pCurrentCar->right.x;
                localUp.y = localUp.y + g_pCurrentCar->right.y;
                localUp.z = localUp.z + g_pCurrentCar->right.z;
                pUp = &g_pCurrentCar->right;
                FIX_NORMALIZE_INTO((*pUp), localUp)
            }
        }

        {
            FixVector *pForward;
            step = FixVecDot(&g_pCurrentCar->right, &g_pCurrentCar->forward);
            FixVecScale(&localUp, &g_pCurrentCar->right, step);
            localUp.x = g_pCurrentCar->forward.x - localUp.x;
            localUp.y = g_pCurrentCar->forward.y - localUp.y;
            localUp.z = g_pCurrentCar->forward.z - localUp.z;
            pForward = &g_pCurrentCar->forward;
            FIX_NORMALIZE_INTO((*pForward), localUp)
        }

        {
            FixVector *pRight;
            FixVecCross(&localUp, &g_pCurrentCar->forward, &g_pCurrentCar->right);
            pRight = &g_pCurrentCar->up;
            FIX_NORMALIZE_INTO((*pRight), localUp)
        }

        {
            step = FixVecDot(&g_pCurrentCar->velocity, &g_pCurrentCar->up);
            FixVecScale(&localUp, &g_pCurrentCar->up, step);
            localUp.x = g_pCurrentCar->velocity.x - localUp.x;
            localUp.y = g_pCurrentCar->velocity.y - localUp.y;
            localUp.z = g_pCurrentCar->velocity.z - localUp.z;
        }
        if (FixVecLength(&localUp) > 0) {
            int s = FixMul(FixMul(0x1e0000, g_pCurrentCar->speed), g_pCurrentCar->field_0x81c);
            if (g_pCurrentCar->handbrake != 0)
                s += FixMul(s, 0x20000);
            FixVecScale(&localUp, &g_pCurrentCar->forward, s);
            g_pCurrentCar->cornerForce[0].x += localUp.x;
            g_pCurrentCar->cornerForce[0].y += localUp.y;
            g_pCurrentCar->cornerForce[0].z += localUp.z;
            g_pCurrentCar->cornerForce[1].x += localUp.x;
            g_pCurrentCar->cornerForce[1].y += localUp.y;
            g_pCurrentCar->cornerForce[1].z += localUp.z;
        }

        localUp = g_pCurrentCar->velocity;
        localUp.y = 0;
        step = FixVecLength(&localUp);
        if (step > 0) {
            {
                int d;
                FixVecScaleRecip(&localUp, &localUp, step);
                d = FixVecDot(&localUp, &g_pCurrentCar->groundNormal);
                step = FixVecDot(&localUp, &g_pCurrentCar->normal0x498) - d;
                if (step > 0x41) {
                    step = FixMul(step, g_pCurrentCar->field_0x9c4);
                    if (step > 0x10000)
                        step = 0x10000;
                    step = 0x10000 - step;
                    FixVecScale(&g_pCurrentCar->velocity, &g_pCurrentCar->velocity, step);
                }
            }
        }
    }
}

// Resets the current wheels' loads to the engine torque (front, rear or all
// four, by the drive split) after a gear change.
// FUNCTION: CMR2 0x0043f570
void Car_ResetWheelLoadsAfterShift(Car *pCar)
{
    int load;

    pCar->gear = 1;
    if (pCar->driveSplit == 0) {
        load = FixMul(pCar->field_0x7a4, pCar->gearSpeed[1]);
        pCar->wheelLoad[3] = load;
        pCar->wheelLoad[2] = load;
    } else {
        if (pCar->driveSplit == 0x10000) {
            load = FixMul(pCar->field_0x7a4, pCar->gearSpeed[1]);
        } else {
            load = FixMul(pCar->field_0x7a4, pCar->gearSpeed[1]);
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
void Car_SetupDriveTrain(void)
{
    int i;

    for (i = 0; i < 4; i++) {
        g_pCurrentCar->cornerOnGround[i] = 1;
        g_pCurrentCar->field_0xb28++;
    }
    g_pCurrentCar->field_0xb28 = 4;
    g_pCurrentCar->tyreGrip = g_pCurrentCar->field_0x75c / 4;
    for (i = 3; i >= 0; i--)
        g_pCurrentCar->field_0x8b8[i] = g_pCurrentCar->tyreGrip;
}

extern FixVector g_carStepAccel;

// Sets the current car's corner mass and the per-corner spring rates from its
// mass and suspension lengths.
// FUNCTION: CMR2 0x0043fbd0
void Car_SetupSuspensionRates(void)
{
    int k;
    int i;

    g_pCurrentCar->cornerMass = g_pCurrentCar->field_0x75c / 4;
    g_carStepAccel.x = 0;
    g_carStepAccel.y = 0;
    g_carStepAccel.z = 0;
    k = FixMul(g_pCurrentCar->cornerMass + 0x1e0000, 0x8000);
    g_pCurrentCar->field_0xa5c[0] = FixMul(k, g_pCurrentCar->wheelSurfaceFx[0].drag + g_pCurrentCar->cornerGrip[0].grip2A);
    g_pCurrentCar->field_0xa4c[0] = FixMul(k, g_pCurrentCar->wheelSurfaceFx[0].drag + g_pCurrentCar->cornerGrip[0].gripA);
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
            localUp.x = g_sinTable[(unsigned short)(g_pCurrentCar->heading + 0x400) & 0xfff];
            localUp.y = 0;
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
                g_pCurrentCar->right = localUp;
            } else {
                FixVecScaleRecip(&localUp, &diff, step);
                FixVecScale(&localUp, &localUp, maxStep);
                localUp.x = localUp.x + g_pCurrentCar->right.x;
                localUp.y = localUp.y + g_pCurrentCar->right.y;
                localUp.z = localUp.z + g_pCurrentCar->right.z;
                pUp = &g_pCurrentCar->right;
                FIX_NORMALIZE_INTO((*pUp), localUp)
            }
        }

        {
            FixVector *pForward;
            step = FixVecDot(&g_pCurrentCar->right, &g_pCurrentCar->forward);
            FixVecScale(&localUp, &g_pCurrentCar->right, step);
            localUp.x = g_pCurrentCar->forward.x - localUp.x;
            localUp.y = g_pCurrentCar->forward.y - localUp.y;
            localUp.z = g_pCurrentCar->forward.z - localUp.z;
            pForward = &g_pCurrentCar->forward;
            FIX_NORMALIZE_INTO((*pForward), localUp)
        }

        {
            FixVector *pRight;
            FixVecCross(&localUp, &g_pCurrentCar->forward, &g_pCurrentCar->right);
            pRight = &g_pCurrentCar->up;
            FIX_NORMALIZE_INTO((*pRight), localUp)
        }

        {
            step = FixVecDot(&g_pCurrentCar->velocity, &g_pCurrentCar->up);
            FixVecScale(&localUp, &g_pCurrentCar->up, step);
            localUp.x = g_pCurrentCar->velocity.x - localUp.x;
            localUp.y = g_pCurrentCar->velocity.y - localUp.y;
            localUp.z = g_pCurrentCar->velocity.z - localUp.z;
        }
        if (FixVecLength(&localUp) > 0) {
            int s = FixMul(FixMul(0x1e0000, g_pCurrentCar->speed), g_pCurrentCar->field_0x81c);
            if (g_pCurrentCar->handbrake != 0)
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
        if (CGameInfo::IsActiveCheatEnabled(6) != 0) {
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
    FixVecScale(&a, &g_pCurrentCar->up, FixMul(0x8000, frontAvg + rearAvg));
    a.x += g_pCurrentCar->position.x;
    a.y += g_pCurrentCar->position.y;
    a.z += g_pCurrentCar->position.z;
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
// GLOBAL: CMR2 0x00511398
double g_unk0x00511398 = -0.009947183943243459;
// FUNCTION: CMR2 0x004336f0
void Car_UpdateWheelTravel(void)
{
    int i;
    int absLoad;
    short delta;

    for (i = 0; i < 4; i++) {
        absLoad = FIX_ABS(g_pCurrentCar->wheelLoad[i]);
        if (absLoad > 0x10000) {
            if (g_pCurrentCar->wheelLoad[i] > 0) {
                g_pCurrentCar->wheelPhase[i] += -0x28b;
            } else {
                g_pCurrentCar->wheelPhase[i] += 0x28b;
            }
        } else {
            delta = (short)(__int64)((double)g_pCurrentCar->wheelLoad[i] * g_unk0x00511398);
            g_pCurrentCar->wheelPhase[i] += delta;
        }
        if (*(int *)(g_pCarSetup + 0x240 + i * 4) > 0) {
            g_pCurrentCar->wheelOffset[i][0] =
                FixMul(g_sinTable[g_pCurrentCar->wheelPhase[i] & 0xfff],
                       FixMul(*(int *)(g_pCarSetup + 0x240 + i * 4), 0xa3d));
            g_pCurrentCar->wheelOffset[i][1] =
                FixMul(-g_sinTable[(unsigned short)(g_pCurrentCar->wheelPhase[i] + 0x400) & 0xfff],
                       FixMul(*(int *)(g_pCarSetup + 0x240 + i * 4), 0xa3d));
        } else {
            g_pCurrentCar->wheelOffset[i][1] = 0;
            g_pCurrentCar->wheelOffset[i][0] = 0;
        }
    }
}
// Normalises a body axis of g_pCurrentCar in place through a pointer.
#define CAR_NORMALIZE_AXIS(field)                                                   \
    {                                                                               \
        FixVector *pField = &g_pCurrentCar->field;                                  \
        FixVector *pAxis = &g_pCurrentCar->field;                                   \
        int len = FixVecLength(pField);                                             \
        if (len == 0) {                                                             \
            pAxis->x = 0;                                                           \
            pAxis->y = 0;                                                           \
            pAxis->z = 0;                                                           \
        } else {                                                                    \
            FixVecScaleRecip(pAxis, pField, len);                                   \
        }                                                                           \
    }

void Car_FlagAirborneCorners(void);
void Car_SettleFreeCorners(void);
void Car_SolveUpright(void);
void Car_IntegrateWheelTravel(void);
void StageObject_UpdateCarCornerGroundHeights(Car *pCar, int count);
void CarDamage_ApplyCollisionDeformImpulse(Car *pCar, int *param_2, FixVector *param_3, int param_4,
                  unsigned char param_5, int param_6);
int NetRace_IsPlayerFadeTimed(BYTE index);

// Scratch vectors of the ground-contact step (referenced only from 0x42eae0).
// GLOBAL: CMR2 0x0053c9c8
FixVector g_carContactNormal;
// GLOBAL: CMR2 0x0053ca08
FixVector g_carContactWork;
// GLOBAL: CMR2 0x0053ca40
FixVector g_carContactWork2;
// GLOBAL: CMR2 0x0053ca50
short g_carSurfaceSnapshot[4];
// GLOBAL: CMR2 0x0053ca68
FixVector g_carContactWork3;
// GLOBAL: CMR2 0x0053ca98
FixVector g_carContactSaved;
// GLOBAL: CMR2 0x0053caf8
FixVector g_carContactWork4;
// GLOBAL: CMR2 0x0053cc08
FixVector g_carContactPoint;

// Ground contact and roll-over step of the current car: refreshes the corner
// contact normals from the body axes, decays the roll-over counters, and,
// when no corner is grounded, finds the corner that supports the body against
// the ground normal, pushes it into the deformation solver, damps the angular
// velocity when the deepest corner changes and updates the tumble timers.
// FUNCTION: CMR2 0x0042eae0
void Car_UpdateGroundContact(void)
{
    int flag;
    int i;
    int k;
    int d;
    int dot;
    short prevGrounded;
    int prevDeepest;

#define CARF(off) (*(int *)((int)g_pCurrentCar + (off)))
#define CARV(off) (*(FixVector *)((int)g_pCurrentCar + (off)))
#define CARB(off) (*(char *)((int)g_pCurrentCar + (off)))

    flag = 0;
    g_pCurrentCar->tipAngle = 0;
    g_pCurrentCar->tipRatio = 0;
    g_pCurrentCar->normal0x498 = g_pCurrentCar->groundNormal;
    for (i = 0; i < 8; i++)
        g_pCurrentCar->cornerNormal[i] = g_pCurrentCar->cornerAxis[i];

    if (g_pCurrentCar->field_0xbfc != 0) {
        if (g_pCurrentCar->field_0xbdc != 0) {
            BYTE step = (BYTE)(FixMul(0xa0000, g_physicsTimeStep) >> 16);
            if (step > g_pCurrentCar->field_0xb25)
                g_pCurrentCar->field_0xb25 = 0;
            else
                g_pCurrentCar->field_0xb25 -= step;
            if (g_pCurrentCar->field_0xb25 <= 0)
                g_pCurrentCar->field_0xbfc = 0;
        } else if (g_pCurrentCar->field_0xbe0 != 0) {
            BYTE step = (BYTE)(FixMul(0xa0000, g_physicsTimeStep) >> 16);
            if (step > g_pCurrentCar->field_0xb26)
                g_pCurrentCar->field_0xb26 = 0;
            else
                g_pCurrentCar->field_0xb26 -= step;
            if (g_pCurrentCar->field_0xb26 <= 0)
                g_pCurrentCar->field_0xbfc = 0;
        } else {
            BYTE step = (BYTE)(FixMul(0xa0000, g_physicsTimeStep) >> 16);
            if (step > g_pCurrentCar->field_0xb27)
                g_pCurrentCar->field_0xb27 = 0;
            else
                g_pCurrentCar->field_0xb27 -= step;
            if (g_pCurrentCar->field_0xb27 <= 0)
                g_pCurrentCar->field_0xbfc = 0;
        }
        if (g_pCurrentCar->field_0xbfc == 0) {
            g_pCurrentCar->field_0xbe4 = 0;
            g_pCurrentCar->field_0xbe0 = 0;
            g_pCurrentCar->field_0xbdc = 0;
            g_pCurrentCar->field_0xb27 = 0;
            g_pCurrentCar->field_0xb25 = 0;
            g_pCurrentCar->field_0xb26 = 0;
            g_pCurrentCar->field_0xbf8 = 1;
        }
    } else {
        if (NetRace_IsPlayerFadeTimed(g_pCurrentCar->index) != 0) {
            memcpy(&g_pCurrentCar->right, g_pCurrentCar->field_0x384, 0x24);
            g_pCurrentCar->position = g_pCurrentCar->positionPrev;
        } else {
            for (i = 4; i < 8; i++) {
                if (g_pCurrentCar->cornerFlags[i] == 0) {
                    if (g_pCurrentCar->field_0x95c < 0x640000)
                        g_pCurrentCar->field_0x95c += g_physicsTimeStep;
                    else
                        g_pCurrentCar->field_0xbf8 = 1;
                    break;
                }
            }
        }
    }

    prevGrounded = (char)g_pCurrentCar->field_0xb34;
    for (i = 3; i >= 0; i--)
        g_carSurfaceSnapshot[i] = g_pCurrentCar->wheelSurface[i];
    StageObject_UpdateCarCornerGroundHeights(g_pCurrentCar, 8);
    Car_FlagAirborneCorners();

    if (g_pCurrentCar->field_0xb34 != 0) {
        Car_UpdateGroundNormal();
        Car_SettleFreeCorners();
        Car_FlagAirborneCorners();
        if (prevGrounded != 0) {
            Car_SolveUpright();
            flag = 1;
        } else {
            for (i = 0; i < 8; i++) {
                if (g_pCurrentCar->cornerFlags[i] == 0) {
                dot = FixVecDot(&g_pCurrentCar->cornerVelocity[i], &g_pCurrentCar->groundNormal);
                g_carContactSaved = g_pCurrentCar->field_0x5c4;
                g_pCurrentCar->field_0x5c4.x = dot;
                g_pCurrentCar->field_0x5c4.y = 0;
                g_pCurrentCar->field_0x5c4.z = 0;

                FixVecScale(&g_carContactWork, &g_pCurrentCar->groundNormal, -0x20000);
                FixMatrix_InverseRotateVector(&g_pCurrentCar->field_0x5dc,
                                              &g_carContactWork, g_pCurrentCar->pWorld);

                d = g_pCurrentCar->field_0x5dc.x;
                if (FIX_ABS(d) > g_pCurrentCar->halfExtents.x)
                    g_pCurrentCar->field_0x5dc.x = d < 0 ? -g_pCurrentCar->halfExtents.x : g_pCurrentCar->halfExtents.x;
                d = g_pCurrentCar->field_0x5dc.y;
                if (FIX_ABS(d) > g_pCurrentCar->halfExtents.y)
                    g_pCurrentCar->field_0x5dc.y = d < 0 ? -g_pCurrentCar->halfExtents.y : g_pCurrentCar->halfExtents.y;
                d = g_pCurrentCar->field_0x5dc.z;
                if (FIX_ABS(d) > g_pCurrentCar->halfExtents.z)
                    g_pCurrentCar->field_0x5dc.z = d < 0 ? -g_pCurrentCar->halfExtents.z : g_pCurrentCar->halfExtents.z;

                g_carContactPoint.x = 0;
                g_carContactPoint.y = 0x7d000000;
                g_carContactPoint.z = 0;
                g_carContactNormal.x = 0;
                g_carContactNormal.y = 0x10000;
                g_carContactNormal.z = 0;
                for (k = 0; k < 4; k++) {
                    if (g_pCurrentCar->corners[k].y < g_carContactPoint.y)
                        g_carContactPoint = g_pCurrentCar->corners[k];
                }
                if (g_pCurrentCar->up.y < 0) {
                    FixVecScale(&g_carContactWork2, &g_pCurrentCar->up,
                                FixMul(0x20000, g_pCurrentCar->halfExtents.y));
                    g_carContactPoint.x += g_carContactWork2.x;
                    g_carContactPoint.y += g_carContactWork2.y;
                    g_carContactPoint.z += g_carContactWork2.z;
                }
                CarDamage_ApplyCollisionDeformImpulse(g_pCurrentCar, (int *)&g_carContactPoint, &g_carContactNormal, 0, 0, 0);
                FixMatrix_RotateVector(&g_carContactWork3,
                                       &g_pCurrentCar->field_0x5dc,
                                       g_pCurrentCar->pWorld);
                g_carContactWork3.x += g_pCurrentCar->position.x;
                g_carContactWork3.y += g_pCurrentCar->position.y;
                g_carContactWork3.z += g_pCurrentCar->position.z;
                g_pCurrentCar->field_0x5c4 = g_carContactSaved;
                break;
                }
            }
        }
        d = FixVecDot(&g_pCurrentCar->groundNormal, &g_pCurrentCar->velocity);
        if (d < 0) {
            FixVecScale(&g_carContactWork4, &g_pCurrentCar->groundNormal, d);
            g_pCurrentCar->velocity.x -= g_carContactWork4.x;
            g_pCurrentCar->velocity.y -= g_carContactWork4.y;
            g_pCurrentCar->velocity.z -= g_carContactWork4.z;
        }
        g_pCurrentCar->field_0x91c = FixVecDot(&g_pCurrentCar->right, &g_pCurrentCar->groundNormal);
        g_pCurrentCar->field_0x920 = FixVecDot(&g_pCurrentCar->up, &g_pCurrentCar->groundNormal);
        dot = FixVecDot(&g_pCurrentCar->forward, &g_pCurrentCar->groundNormal);
    } else {
        for (i = 0; i < 8; i++) {
            d = g_pCurrentCar->corners[i].y - g_pCurrentCar->cornerHeight[i];
            if (FIX_ABS(d) < 0x3333) {
                Car_UpdateGroundNormal();
                Car_SolveUpright();
                flag = 1;
                break;
            }
        }
        g_pCurrentCar->field_0x91c = FixVecDot(&g_pCurrentCar->right, &g_pCurrentCar->groundNormal);
        g_pCurrentCar->field_0x920 = FixVecDot(&g_pCurrentCar->up, &g_pCurrentCar->groundNormal);
        dot = FixVecDot(&g_pCurrentCar->forward, &g_pCurrentCar->groundNormal);
    }
    g_pCurrentCar->field_0x924 = dot;

    if (flag == 0) {
        g_pCurrentCar->field_0xba0[2] = 0;
        g_pCurrentCar->field_0xba0[1] = 0;
        g_pCurrentCar->field_0xba0[0] = 0;
    }

    g_pCurrentCar->field_0x958 = 0;
    prevDeepest = g_pCurrentCar->field_0xb2b;
    g_pCurrentCar->field_0xb2b = -1;
    for (i = 0; i < 4; i++) {
        if (i < 2)
            FixVecScale(&g_carContactWork, &g_pCurrentCar->up, g_pCurrentCar->field_0x770[1]);
        else
            FixVecScale(&g_carContactWork, &g_pCurrentCar->up, g_pCurrentCar->field_0x770[0]);
        g_carContactWork.x += g_pCurrentCar->corners[i].x;
        g_carContactWork.y += g_pCurrentCar->corners[i].y;
        g_carContactWork.z += g_pCurrentCar->corners[i].z;
        d = FixMul(g_pCurrentCar->cornerHeight[i + 4] + g_pCurrentCar->cornerHeight[i], 0x8000) -
            g_carContactWork.y;
        if (d > g_pCurrentCar->field_0x958) {
            g_pCurrentCar->field_0x958 = d;
            g_pCurrentCar->field_0xb2b = (char)i;
        }
    }
    if (g_pCurrentCar->field_0x958 == 0) {
        g_pCurrentCar->field_0x958 = 0x1999;
        for (i = 0; i < 4; i++) {
            if (g_pCurrentCar->cornerFlags[i] == 0) {
                d = g_pCurrentCar->corners[i].y - g_pCurrentCar->cornerHeight[i];
                if (d < g_pCurrentCar->field_0x958)
                    g_pCurrentCar->field_0x958 = d > 0 ? d : 0;
            }
        }
        g_pCurrentCar->field_0x958 = -g_pCurrentCar->field_0x958;
    }
    if (g_pCurrentCar->field_0xb2b != -1 && g_pCurrentCar->field_0xb2b != prevDeepest) {
        FixMatrix_InverseRotateVector(&g_carContactWork, &g_pCurrentCar->groundNormal,
                                      g_pCurrentCar->pWorld);
        dot = FixVecDot(&g_carContactWork, &g_pCurrentCar->angularVelocity);
        FixVecScale(&g_carContactWork, &g_carContactWork, dot);
        g_carContactWork.x = g_pCurrentCar->angularVelocity.x - g_carContactWork.x;
        g_carContactWork.y = g_pCurrentCar->angularVelocity.y - g_carContactWork.y;
        g_carContactWork.z = g_pCurrentCar->angularVelocity.z - g_carContactWork.z;
        FixVecScale(&g_carContactWork, &g_carContactWork, FixMul(0xccc, g_physicsTimeStep));
        g_pCurrentCar->angularVelocity.x -= g_carContactWork.x;
        g_pCurrentCar->angularVelocity.y -= g_carContactWork.y;
        g_pCurrentCar->angularVelocity.z -= g_carContactWork.z;
    }

    Car_RelaxBodyAxes(1);
    Car_IntegrateWheelTravel();
    d = g_pCurrentCar->field_0x920;
    if (g_pCurrentCar->field_0xc04[0] == 0 && d < 0)
        g_pCurrentCar->field_0xc04[0] = 1;
    if (d < 0)
        g_pCurrentCar->field_0xc10 = 1;
    g_pCurrentCar->field_0x96c -= FixMul(0x1eb8, g_physicsTimeStep);
    if (g_pCurrentCar->field_0x96c < 0)
        g_pCurrentCar->field_0x96c = 0;
    if (g_pCurrentCar->field_0x96c > 0)
        return;
    if (d <= 0xfc28)
        return;
    if (g_pCurrentCar->field_0xc10 != 0) {
        if (FIX_ABS(g_pCurrentCar->angularVelocity.x) >= 0x28f)
            return;
        if (FIX_ABS(g_pCurrentCar->angularVelocity.z) >= 0x28f)
            return;
    }
    if (g_pCurrentCar->field_0xb28 != 0) {
        g_pCurrentCar->field_0xc00 = 0;
        g_pCurrentCar->field_0x96c = 0;
        g_pCurrentCar->field_0xc04[0] = 0;
        g_pCurrentCar->field_0xc10 = 0;
    }
}

// Flags the box corners of the current car that are off the ground and lists
// (from 0xb36) the ones that touch it.
// match 78%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0042f820
void Car_FlagAirborneCorners(void)
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
        d = FixVecDot(&g_pCurrentCar->up, &diff);
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
    FixVector edge1;
    FixVector edge3;
    FixVector edge4;
    FixVector edge2;
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

    edge1.x = a.x - b.x;
    edge1.y = a.y - b.y;
    edge1.z = a.z - b.z;
    edge3.x = c.x - b.x;
    edge3.y = c.y - b.y;
    edge3.z = c.z - b.z;
    FixVecCross(&n, &edge3, &edge1);
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

    edge2.x = a.x - d.x;
    edge2.y = a.y - d.y;
    edge2.z = a.z - d.z;
    edge4.x = c.x - d.x;
    edge4.y = c.y - d.y;
    edge4.z = c.z - d.z;
    FixVecCross(&n, &edge4, &edge2);
    if (n.y < 0) {
        n.x = -n.x;
        n.y = -n.y;
        n.z = -n.z;
    }
    FIX_NORMALIZE_INTO(n, n);

    {
        FixVector *pOut;
        n.x += g_pCurrentCar->groundNormal.x;
        n.y += g_pCurrentCar->groundNormal.y;
        n.z += g_pCurrentCar->groundNormal.z;
        pOut = &g_pCurrentCar->groundNormal;
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
    short angleZ;
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
            g_leanAccel.z = z;
        } else {
            z = -0x170a;
            g_leanAccel.z = z;
        }
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
    g_leanBasis.up.x = FixMul(sinX, cosZ);
    g_leanBasis.up.y = FixMul(cosX, cosZ);
    g_leanBasis.up.z = sinZ;
    g_leanBasis.forward.x = FixMul(sinX, -sinZ);
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
    int along;

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
            if ((g_pCurrentCar->field_0xc00 == 0 && g_pCurrentCar->cornerOnGround[i] != 0) ||
                (g_pCurrentCar->field_0xc00 != 0 && g_pCurrentCar->cornerFlags[i] == 0)) {
                along = FixVecDot(&g_pCurrentCar->cornerAxis[i], &g_pCurrentCar->cornerNormal[i]);
                if (along >= 0x10000) {
                    strength[i] = 0;
                } else {
                    strength[i] = (0x400 - FixAcos(along)) * 0x1680;
                }
                if (strength[i] > 0xa0000 &&
                    g_pCurrentCar->cornerAxis[i].y < g_pCurrentCar->cornerNormal[i].y) {
                    along = FixVecDot(&g_pCurrentCar->cornerVelocity[i], &g_pCurrentCar->cornerNormal[i]);
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
                    int len;

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
                    along = FixMul(strength[i], 0x51e);
                    if (along > 0x10000) {
                        along = 0x10000;
                    }
                    FixVecScale(&dir, &dir, (-FixMul(along, FixMul(grip, share))));
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
    if (g_pCurrentCar->wheelLoad[i] > 0) {                                                 \
        g_pCurrentCar->wheelTorque[i] -= (i) < 2 ? brakeFront : brakeRear;                  \
        g_pCurrentCar->wheelTorque[i] = g_pCurrentCar->wheelTorque[i] - (resist);           \
    } else if (g_pCurrentCar->wheelLoad[i] < 0) {                                           \
        g_pCurrentCar->wheelTorque[i] += (i) < 2 ? brakeFront : brakeRear;                  \
        g_pCurrentCar->wheelTorque[i] = g_pCurrentCar->wheelTorque[i] + (resist);           \
    }

// Integrates the wheel spin; an undriven wheel (or one against the gear)
// stops instead of reversing.
#define WHEEL_SPIN(i) \
    old = g_pCurrentCar->wheelLoad[i]; \
    g_pCurrentCar->wheelLoad[i] += FixMul(g_pCurrentCar->wheelTorque[i], g_wheelSpinStep); \
    if (g_pCurrentCar->gear != 0 && g_pCurrentCar->field_0xb84 == 0 && \
        g_pCurrentCar->driveSplit != ((i) < 2 ? 0 : 0x10000)) { \
        gear = g_pCurrentCar->field_0x7bc[g_pCurrentCar->gear]; \
        if ((gear > 0 && g_pCurrentCar->wheelLoad[i] < 0) || \
            (gear < 0 && g_pCurrentCar->wheelLoad[i] > 0)) \
            g_pCurrentCar->wheelLoad[i] = 0; \
    } else { \
        if ((old > 0 && g_pCurrentCar->wheelLoad[i] < 0) || \
            (old < 0 && g_pCurrentCar->wheelLoad[i] > 0)) \
            g_pCurrentCar->wheelLoad[i] = 0; \
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
    int len;
    int force;
    FixVector tangent;
    FixVector dir;

    if (g_pCurrentCar->field_0xb74 != 0)
        return;
    for (i = 7; i >= 0; i--) {
        if (g_pCurrentCar->cornerFlags[i] != 0)
            continue;
        len = FixVecDot(&g_pCurrentCar->groundNormal, &g_pCurrentCar->cornerVelocity[i]);
        FixVecScale(&tangent, &g_pCurrentCar->groundNormal, len);
        tangent.x = g_pCurrentCar->cornerVelocity[i].x - tangent.x;
        tangent.y = g_pCurrentCar->cornerVelocity[i].y - tangent.y;
        tangent.z = g_pCurrentCar->cornerVelocity[i].z - tangent.z;
        len = FixVecLength(&tangent);
        if (len > 0) {
            FixVecScaleRecip(&dir, &tangent, len);
            force = -FixMul(len, g_pCurrentCar->cornerMass);
            if (len < 0x10000)
                force -= FixVecLength(&g_carStepAccel);
            if (((force) < 0 ? -(force) : (force)) > g_pCurrentCar->cornerGripB[i]) {
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
// FUNCTION: CMR2 0x004387a0
void Car_UpdateTyreForces(void)
{
    FixVector dir[4];
    FixVector axis[4];
    FixVector v;
    FixVector t;
    FixVector *pNormal;
    int dirValid[4] = {1, 1, 1, 1};
    int axisValid[4] = {1, 1, 1, 1};
    int rollSpeed[4];
    int brakeFront;
    int brakeRear;
    int longForce;
    int latForce;
    int longAssist;
    int latAssist;
    int speed;
    int slip;
    int spinSlip;
                int vectorOffset;
    int crossing;
    int grip;
    int resist;
    int limB;
    int b;
    int d;
    int d_2;
    int e;
    int s;
    int old;
    int gear;
    int len;
    short angle;
    int i;

    latForce = 0;
    longForce = 0;
    latAssist = 0;
    longAssist = 0;
    pNormal = &g_pCurrentCar->groundNormal;
    if (g_pCurrentCar->flag0x1d0[2] == 0 && g_pCurrentCar->flag0x1d0[3] == 0 &&
        g_pCurrentCar->gear != 0) {
        if (g_pCurrentCar->speed < 0x1999) {
            brakeFront = FixMul(0x1999, g_pCurrentCar->brakeBias);
            brakeRear = FixMul(0x1999, 0x10000 - g_pCurrentCar->brakeBias);
        } else {
            brakeFront = 0;
            brakeRear = 0;
        }
    } else {
        resist = FixMul(g_pCurrentCar->brakeInput, *(int *)(g_pCarSetup + 0x3fc));
        brakeFront = FixMul(resist, g_pCurrentCar->brakeBias);
        resist = FixMul(g_pCurrentCar->brakeInput, *(int *)(g_pCarSetup + 0x400));
        brakeRear = FixMul(resist, 0x10000 - g_pCurrentCar->brakeBias);
    }
    if (g_pCurrentCar->handbrake != 0)
        brakeRear += g_pCurrentCar->handbrakeForce;

    if (g_pCurrentCar->field_0xb28 != 0 && g_pCurrentCar->field_0xb74 != 0) {
        // Rolling directions and lateral axes of the front and rear wheels,
        // flattened onto the ground.
        d_2 = -FixVecDot(pNormal, &g_pCurrentCar->frontWheelDir);
        FixVecScale(&t, pNormal, d_2);
        v.x = g_pCurrentCar->frontWheelDir.x + t.x;
        v.y = g_pCurrentCar->frontWheelDir.y + t.y;
        v.z = g_pCurrentCar->frontWheelDir.z + t.z;
        len = FixVecLength(&v);
        if (len == 0) {
            dir[0].x = 0;
            dir[0].y = 0;
            dir[0].z = 0;
        } else {
            FixVecScaleRecip(&dir[0], &v, len);
        }
        dir[1] = dir[0];
        if (len == 0) {
            dirValid[1] = 0;
            dirValid[0] = 0;
        }
        d_2 = -FixVecDot(pNormal, &g_pCurrentCar->frontWheelAxis);
        FixVecScale(&t, pNormal, d_2);
        v.x = g_pCurrentCar->frontWheelAxis.x + t.x;
        v.y = g_pCurrentCar->frontWheelAxis.y + t.y;
        v.z = g_pCurrentCar->frontWheelAxis.z + t.z;
        len = FixVecLength(&v);
        if (len == 0) {
            axis[0].x = 0;
            axis[0].y = 0;
            axis[0].z = 0;
        } else {
            FixVecScaleRecip(&axis[0], &v, len);
        }
        axis[1] = axis[0];
        if (len == 0) {
            axisValid[1] = 0;
            axisValid[0] = 0;
        }
        d_2 = -FixVecDot(pNormal, &g_pCurrentCar->rearWheelDir);
        FixVecScale(&t, pNormal, d_2);
        v.x = g_pCurrentCar->rearWheelDir.x + t.x;
        v.y = g_pCurrentCar->rearWheelDir.y + t.y;
        v.z = g_pCurrentCar->rearWheelDir.z + t.z;
        len = FixVecNormalizeLen(&dir[2], &v);
        dir[3] = dir[2];
        if (len == 0) {
            dirValid[3] = 0;
            dirValid[2] = 0;
        }
        FixVecCross(&g_tyreForce, &g_pCurrentCar->rearWheelDir, &g_pCurrentCar->up);
                d_2 = -FixVecDot(pNormal, &g_tyreForce);
        FixVecScale(&t, pNormal, d_2);
                v.z = g_tyreForce.z + t.z;
                v.y = g_tyreForce.y + t.y;
                v.x = g_tyreForce.x + t.x;
        len = FixVecNormalizeLen(&axis[2], &v);
        axis[3] = axis[2];
        if (len == 0) {
            axisValid[3] = 0;
            axisValid[2] = 0;
        }
        g_pCurrentCar->groundDir[0] = dir[0];
        g_pCurrentCar->groundDir[1] = dir[2];
        g_pCurrentCar->groundAxis[0] = axis[0];
        g_pCurrentCar->groundAxis[1] = axis[2];

                g_tyreGripScale = FixMul(g_pCurrentCar->tyreGrip, g_physicsScale);
                g_tyreSlipScale = FixMul(g_physicsScale, 0xf000);
        g_wheelSpinStep = FixMul(g_physicsTimeStep, 0x11113);

        // Wheel spin: brakes, then the drive torque limited by the grip.
        for (i = 3; i >= 0; i--) {
            if (g_pCurrentCar->cornerOnGround[i] != 0) {
                resist = *(int *)(g_pCarSetup + 0x3ec + i * 4);
                WHEEL_BRAKE(i, resist);
                if (dirValid[i] != 0) {
                    speed = FixVecDot(&g_pCurrentCar->cornerVelocity[i], &dir[i]);
                    rollSpeed[i] = speed;
                    spinSlip = speed * 4 - g_pCurrentCar->wheelLoad[i];
                    crossing = FALSE;
                    if ((speed > 0 && spinSlip < 0) || (speed < 0 && spinSlip > 0))
                        crossing = TRUE;
                    g_pCurrentCar->wheelSlipping[i] = 0;
                    if (speed * 4 > 0) {
                        if (g_pCurrentCar->wheelLoad[i] > speed * 4 || g_pCurrentCar->wheelLoad[i] < -0x1999)
                            g_pCurrentCar->wheelSlipping[i] = 1;
                    } else if (g_pCurrentCar->wheelLoad[i] < speed * 4 || g_pCurrentCar->wheelLoad[i] > 0x1999) {
                        g_pCurrentCar->wheelSlipping[i] = 1;
                    }
                    if (i > 1 && g_pCurrentCar->handbrake != 0 && !crossing) {
                        g_pCurrentCar->wheelTorque[i] = 0;
                        WHEEL_BRAKE(i, resist);
                    } else {
                        longForce = FixMul(spinSlip, g_tyreSlipScale) * 4;
                        if (g_pCurrentCar->flag0x1d0[2] == 0 && g_pCurrentCar->flag0x1d0[3] == 0) {
                            grip = g_pCurrentCar->field_0xa5c[i];
                        } else {
                            resist = FixMul(FIX_ABS(spinSlip), 0x4000);
                            resist -= FIX_ABS(FixVecDot(&g_pCurrentCar->cornerVelocity[i], &axis[i]));
                            if (resist < 0)
                                resist = 0;
                            resist = FixMul(resist, 0xccc) + 0x51e;
                            if (g_pCurrentCar->cornerGrip[i].drag != 0)
                                resist += FixMul(0xe666, g_pCurrentCar->cornerGrip[i].drag);
                            grip = FixMul(g_pCurrentCar->field_0xa5c[i], resist);
                        }
                        if (longForce > grip || (grip = -grip, longForce < grip))
                            longForce = grip;
                        if (g_pCurrentCar->flag0x1d0[2] == 0 && g_pCurrentCar->flag0x1d0[3] == 0)
                            spinSlip = longForce / 4;
                        else
                            spinSlip = longForce / 8;
                        if (g_pCurrentCar->field_0xb1f != 0)
                            spinSlip = FixMul(spinSlip, 0x1999);
                        g_pCurrentCar->wheelTorque[i] += spinSlip;
                    }
                }
            }
            WHEEL_SPIN(i);
        }
        Car_BalanceWheelPairs();

        // Tyre forces.
        for (i = 3; i >= 0; i--) {
            if (g_pCurrentCar->cornerOnGround[i] == 0)
                continue;
            if (dirValid[i] != 0) {
                slip = rollSpeed[i] * 4 - g_pCurrentCar->wheelLoad[i];
                g_pCurrentCar->wheelSlip[i] = slip;
                if (FIX_ABS(slip) < 0x10000) {
                    d_2 = FixVecDot(&g_carStepAccel, &dir[i]);
                    longAssist = -d_2;
                    e = FixMul(rollSpeed[i], g_tyreGripScale);
                    if ((longAssist > 0 && e > 0) || (longAssist < 0 && e < 0)) {
                        if (FIX_ABS(longAssist) > FIX_ABS(e))
                            longAssist -= e;
                        else
                            longAssist = 0;
                    }
                    gear = g_pCurrentCar->field_0x7bc[g_pCurrentCar->gear];
                    if (((gear > 0 && longAssist < 0) || (gear < 0 && longAssist > 0) || gear == 0) &&
                        g_pCurrentCar->brakeInput == 0)
                        longAssist = 0;
                }
                longForce = -FixMul(FixMul(slip, 0x4000), g_tyreGripScale);
            }
            if (axisValid[i] != 0) {
                speed = FixVecDot(&g_pCurrentCar->cornerVelocity[i], &axis[i]);
                g_pCurrentCar->wheelSlipLateral[i] = speed;
                if (FIX_ABS(speed) < 0x10000)
                    latAssist = -FixVecDot(&g_carStepAccel, &axis[i]);
                latForce = -FixMul(speed, g_tyreGripScale);
            }
            longForce += longAssist;
            latForce += latAssist;
            if (FIX_ABS(longForce) > 0x10000 && FIX_ABS(longForce) >= FIX_ABS(latForce)) {
                g_tyreForceMax = FIX_ABS(longForce);
                g_tyreForceRecip = FixDiv(0x10000, g_tyreForceMax);
                e = FixMul(longForce, g_tyreForceRecip);
                FixVecScale(&g_tyreLongForce, &dir[i], e);
                FixVecScale(&g_tyreLatForce, &axis[i], FixMul(latForce, g_tyreForceRecip));
                g_tyreForce.x = g_tyreLongForce.x + g_tyreLatForce.x;
                g_tyreForce.y = g_tyreLongForce.y + g_tyreLatForce.y;
                g_tyreForce.z = g_tyreLongForce.z + g_tyreLatForce.z;
                resist = FixMul(FIX_SQR(FixVecNormalizeLen(&g_pCurrentCar->cornerForce[i], &g_tyreForce)), g_tyreForceMax);
            } else if (FIX_ABS(latForce) > 0x10000 && FIX_ABS(latForce) >= FIX_ABS(longForce)) {
                g_tyreForceMax = FIX_ABS(latForce);
                g_tyreForceRecip = FixDiv(0x10000, g_tyreForceMax);
                e = FixMul(longForce, g_tyreForceRecip);
                FixVecScale(&g_tyreLongForce, &dir[i], e);
                FixVecScale(&g_tyreLatForce, &axis[i], FixMul(latForce, g_tyreForceRecip));
                g_tyreForce.x = g_tyreLongForce.x + g_tyreLatForce.x;
                g_tyreForce.y = g_tyreLongForce.y + g_tyreLatForce.y;
                g_tyreForce.z = g_tyreLongForce.z + g_tyreLatForce.z;
                resist = FixMul(FIX_SQR(FixVecNormalizeLen(&g_pCurrentCar->cornerForce[i], &g_tyreForce)), g_tyreForceMax);
            } else {
                FixVecScale(&g_tyreLongForce, &dir[i], longForce);
                FixVecScale(&g_tyreLatForce, &axis[i], latForce);
                g_tyreForce.x = g_tyreLongForce.x + g_tyreLatForce.x;
                g_tyreForce.y = g_tyreLongForce.y + g_tyreLatForce.y;
                g_tyreForce.z = g_tyreLongForce.z + g_tyreLatForce.z;
                resist = FIX_SQR(FixVecNormalizeLen(&g_pCurrentCar->cornerForce[i], &g_tyreForce));
            }

            // Friction ellipse of the tyre.
            if (longForce < 0)
                longForce = -longForce;
            if (latForce < 0)
                latForce = -latForce;
            if (i < 2) {
                d_2 = g_pCurrentCar->cornerGrip[i].gripB + g_pCurrentCar->wheelSurfaceFx[i].extraGrip;
                b = g_pCurrentCar->cornerGrip[i].grip2B + g_pCurrentCar->wheelSurfaceFx[i].extraGrip;
            } else {
                d_2 = g_pCurrentCar->cornerGrip[i].gripB - 0x4ccc + g_pCurrentCar->wheelSurfaceFx[i].extraGrip;
                b = g_pCurrentCar->cornerGrip[i].grip2B - 0x4ccc + g_pCurrentCar->wheelSurfaceFx[i].extraGrip;
            }
            // brakeRear is free by now; the original keeps the first limit in it.
            if (longForce < 0x6666) {
                limB = FixMul(g_pCurrentCar->field_0xa5c[i], b);
                brakeRear = FixMul(g_pCurrentCar->field_0xa4c[i], d_2);
            } else if (latForce < 0x6666) {
                limB = g_pCurrentCar->field_0xa5c[i];
                brakeRear = g_pCurrentCar->field_0xa4c[i];
            } else {
                angle = FixAtan2(FixMul(longForce, d_2), latForce);
                d_2 = FixMul(d_2, g_sinTable[(angle + 0x400) & 0xfff]);
                s = g_sinTable[angle & 0xfff];
                brakeRear = FixSqrt(FixMul(d_2, d_2) + FixMul(s, s));
                brakeRear = FixMul(brakeRear, g_pCurrentCar->field_0xa4c[i]);
                angle = FixAtan2(FixMul(longForce, b), latForce);
                d = FixMul(b, g_sinTable[(angle + 0x400) & 0xfff]);
                s = g_sinTable[angle & 0xfff];
                limB = FixMul(FixSqrt(FixMul(d, d) + FixMul(s, s)), g_pCurrentCar->field_0xa5c[i]);
            }
            if (resist > brakeRear)
                resist = limB;
            FixVecScale(&g_pCurrentCar->cornerForce[i], &g_pCurrentCar->cornerForce[i], resist);

            // Rolling resistance of the tyre.
            if (g_pCurrentCar->cornerGrip[i].drag != 0) {
                d_2 = FixVecDot(&dir[i], &g_pCurrentCar->cornerVelocity[i]);
                if (d_2 != 0) {
                    longForce = -FixMul(d_2 / 4, FixMul(g_pCurrentCar->tyreGrip, g_pCurrentCar->cornerGrip[i].drag));
                    FixVecScale(&g_tyreForce, &dir[i], longForce);
                    (&g_pCurrentCar->cornerForce[i])->x += g_tyreForce.x;
                    (&g_pCurrentCar->cornerForce[i])->y += g_tyreForce.y;
                    (&g_pCurrentCar->cornerForce[i])->z += g_tyreForce.z;
                }
            }
        }
        Car_ApplyCornerFriction(g_tyreGripScale);
    } else {
        g_wheelSpinStep = FixMul(g_physicsTimeStep, 0xa0000);
        for (i = 3; i >= 0; i--) {
            resist = *(int *)(g_pCarSetup + 0x3ec + i * 4);
            WHEEL_BRAKE(i, resist);
            WHEEL_SPIN(i);
        }
        Car_BalanceWheelPairs();
    }
}


int StageTiming_GetCarReplayTailEntry(void *pCar, int index);

extern double g_unk0x00511300;

// Orientation of a lean vector: up along (lean.x, 1, lean.z) with the tilt
// limited to about 0.09, right the X axis made perpendicular to it.
#define LEAN_BASIS(lean)                                                     \
    t = (lean);                                                              \
    if (FIX_ABS(t.x) > 0x170a)                                               \
        t.x = t.x > 0 ? 0x170a : -0x170a;                                    \
    if (FIX_ABS(t.z) > 0x170a)                                               \
        t.z = t.z > 0 ? 0x170a : -0x170a;                                    \
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
void Car_StartPendingCountdown(void)
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
void Car_ScaleRatesByTimeStep(Car *pCar)
{
    pCar->field_0x790 = FixMul(0xccc, g_physicsTimeStep);
    pCar->field_0x834 = FixMul(0xccc, g_physicsTimeStep);
    pCar->field_0x840 = FixMul(0x1999, g_physicsTimeStep);
    pCar->field_0x820 = FixMul(0x3333, g_physicsTimeStep);
}

inline void Car_NormalizeForceVector(FixVector *pOut, FixVector *pV)
{
    int len = FixVecLength(pV);
    if (len == 0) {
        pOut->x = 0;
        pOut->y = 0;
        pOut->z = 0;
    } else {
        FixVecScaleRecip(pOut, pV, len);
    }
}

// Suspension of the car: two lean vectors (the wheel frame and the body)
// are pushed by the car's acceleration and damped, and the height of each
// corner of the body under the leaned frames gives the suspension travel of
// that corner. The body lean also rolls with the steering on the cars that
// use it.
// FUNCTION: CMR2 0x0043b100
void Car_UpdateSuspension(void)
{
    FixMatrix m;
    FixVector box[4];
    FixVector basis[3];
    FixVector damp;
    FixVector delta;
    FixVector accel;
    FixVector scaled;
    FixVector diff;
    FixVector flat;
    FixVector *pV;
    int len;
    int k;
    int i;
    short angle;

    if (g_pCurrentCar->field_0xb28 != 0 && g_pCurrentCar->field_0xb74 != 0) {
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
    } else {
        accel.x = 0;
        accel.y = 0;
        accel.z = 0;
        scaled = accel;
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
    accel = (g_pCurrentCar->wheelLean);
    if (FIX_ABS(accel.x) > 0x170a)
        accel.x = accel.x > 0 ? 0x170a : -0x170a;
    if (FIX_ABS(accel.z) > 0x170a)
        accel.z = accel.z > 0 ? 0x170a : -0x170a;
    accel.y = 0x10000;
    Car_NormalizeForceVector(&basis[1], &accel);
    basis[0].x = 0x10000;
    basis[0].y = 0;
    basis[0].z = 0;
    len = FixVecDot(&basis[0], &basis[1]);
    FixVecScale(&flat, &basis[1], len);
    flat.x = basis[0].x - flat.x;
    flat.y = basis[0].y - flat.y;
    flat.z = basis[0].z - flat.z;
    Car_NormalizeForceVector(&basis[0], &flat);
    FixVecCross(&basis[2], &basis[0], &basis[1]);
    Car_NormalizeForceVector(&basis[2], &basis[2]);
    m.right = basis[0];
    m.up = basis[1];
    m.forward = basis[2];
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
        FixMatrix_RotateVector(&flat, &box[i], &m);
        g_pCurrentCar->field_0x998[i] = flat.y;
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
    accel = (g_pCurrentCar->lean);
    if (FIX_ABS(accel.x) > 0x170a)
        accel.x = accel.x > 0 ? 0x170a : -0x170a;
    if (FIX_ABS(accel.z) > 0x170a)
        accel.z = accel.z > 0 ? 0x170a : -0x170a;
    accel.y = 0x10000;
    Car_NormalizeForceVector(&basis[1], &accel);
    basis[0].x = 0x10000;
    basis[0].y = 0;
    basis[0].z = 0;
    len = FixVecDot(&basis[0], &basis[1]);
    FixVecScale(&flat, &basis[1], len);
    flat.x = basis[0].x - flat.x;
    flat.y = basis[0].y - flat.y;
    flat.z = basis[0].z - flat.z;
    Car_NormalizeForceVector(&basis[0], &flat);
    FixVecCross(&basis[2], &basis[0], &basis[1]);
    Car_NormalizeForceVector(&basis[2], &basis[2]);
    m.right = basis[0];
    m.up = basis[1];
    m.forward = basis[2];
    if (g_pCurrentCar->field_0xb1d != 0 && g_pCurrentCar->field_0x7bc[g_pCurrentCar->gear] != 0) {
        flat.x = 0;
        flat.y = 0;
        flat.z = 0x10000;
        angle = (short)((double)FixMul(g_pCurrentCar->field_0xb1d << 16, 0x3333) * g_unk0x00511300);
        ((void (__stdcall *)(FixMatrix *, FixVector *, short))FixMatrix_FromAxisAngle)(&m, &flat, angle);
        for (i = 0; i < 3; i++) {
            FixMatrix_RotateVector(&flat, &basis[i], &m);
            Car_NormalizeForceVector(&basis[i], &flat);
        }
        m.right = basis[0];
        m.up = basis[1];
        m.forward = basis[2];
    }
    for (i = 0; i < 4; i++) {
        FixMatrix_RotateVector(&flat, &box[i], &m);
        g_pCurrentCar->wheel0x9a8[i] = flat.y;
        g_pCurrentCar->wheel0x9a8[i] += *(int *)(g_pCarSetup + 0x3dc + i * 4);
    }
    if (g_pCurrentCar->field_0xb28 == 4) {
        Car_UpdateWheelTorques();
        return;
    } else {
        for (i = 7; i >= 0; i--)
            g_pCurrentCar->field_0x8b8[i] = g_pCurrentCar->tyreGrip;
        return;
    }
}

// Normalises the per-wheel slip, turns it into wheel torque and, on the cars
// that use it, feeds the torque back towards half the drive torque.
// match 84%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
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
        int load = FixMul(g_pCurrentCar->field_0x998[i], 0x40000);
        load += g_pCurrentCar->field_0x808[i];
        g_pCurrentCar->field_0x8b8[i] = quarter - FixMul(load, g_pCurrentCar->field_0x8d8);
    }

    if (StageTiming_GetCarReplayTailEntry(g_pCurrentCar, 3) != 0) {
        int rate = FixMul(g_pCurrentCar->speed, 0x2000);

        if (rate > 0x10000) {
            rate = 0x10000;
        }
        int half = g_pCurrentCar->field_0x75c / 2;
        for (i = 0x8b8; i < 0x8c8; i += 4) {
            int target = i < 0x8c0 ? half : 0;
            *(int *)((BYTE *)g_pCurrentCar + i) += FixMul(rate, target - *(int *)((BYTE *)g_pCurrentCar + i));
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

        angAccelY = angAccelY - FixMul(FixMul(g_pCurrentCar->inertia.y, g_pCurrentCar->field_0x75c), g_pCurrentCar->field_0x5d0.y);
        angAccelY = FixMul(angAccelY, g_physicsTimeStep);
        g_pCurrentCar->angularVelocity.y = g_pCurrentCar->angularVelocity.y + angAccelY;
        angStep.y = FixMul(g_pCurrentCar->angularVelocity.y, g_physicsTimeStep);
        halfStep = g_physicsTimeStep / 2;
        angAccelY = FixMul(halfStep, angAccelY);
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
        int upDot = FixVecDot(&g_pCurrentCar->up, &g_pCurrentCar->groundNormal);
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
// FUNCTION: CMR2 0x00441500
void Car_UpdateWheelForces(void)
{
    FixVector wheelDir[2];
    FixVector axis[2];
    FixVector lateralAxis;
    FixVector longVec;
    FixVector latForceVec;
    FixVector direction;
    FixVector force;
    int dot[4];
    int brakeA;
    int brakeB;
    int scale;
    int loadScale;
    int torqueScale;
    int torqueLimit;
    int i;
    int combined;
    int fLong;
    int fLat;

    int recip;
    int a;
    int b;
    int sine;
    int limA;
    int limB;
    union { short angle; int raw; } trig;

    if (g_pCurrentCar->flag0x1d0[2] == 0 && g_pCurrentCar->flag0x1d0[3] == 0 &&
        g_pCurrentCar->gear != 0) {
        brakeA = FixMul(0x1999, g_pCurrentCar->brakeBias);
        brakeB = FixMul(0x1999, 0x10000 - g_pCurrentCar->brakeBias);
    } else {
        combined = FixMul(g_pCurrentCar->brakeInput, *(int *)(g_pCarSetup + 0x3fc));
        brakeA = FixMul(combined, g_pCurrentCar->brakeBias);
        combined = FixMul(g_pCurrentCar->brakeInput, *(int *)(g_pCarSetup + 0x400));
        brakeB = FixMul(combined, 0x10000 - g_pCurrentCar->brakeBias);
    }
    if (g_pCurrentCar->handbrake != 0) {
        brakeB = brakeB + g_pCurrentCar->handbrakeForce;
    }

    wheelDir[0] = g_pCurrentCar->frontWheelDir;
    axis[0] = g_pCurrentCar->frontWheelAxis;
    wheelDir[1] = g_pCurrentCar->rearWheelDir;
    FixVecCross(&axis[1], &g_pCurrentCar->rearWheelDir, &g_pCurrentCar->up);
    Car_NormalizeForceVector(&axis[1], &axis[1]);

    scale = FixMul(g_pCurrentCar->tyreGrip, g_physicsScale);
    loadScale = FixMul(g_physicsScale, 0xf000);
    torqueScale = FixMul(g_physicsTimeStep, 0x11113);
    torqueLimit = FIX_ABS(FixMul(g_pCurrentCar->field_0x794,
                                 g_pCurrentCar->gearSpeed[g_pCurrentCar->gear]));

    i = 3;
    do {
        int load;
        int slip;
        int brake;
        int limit;
        int crossing;

        direction = i < 2 ? wheelDir[0] : wheelDir[1];
        load = g_pCurrentCar->wheelLoad[(unsigned int)i];
        combined = *(int *)(g_pCarSetup + 0x3ec + i * 4);
        if (load > 0) {
            brake = i < 2 ? brakeA : brakeB;
            g_pCurrentCar->wheelTorque[(unsigned int)i] = g_pCurrentCar->wheelTorque[(unsigned int)i] - brake;
            g_pCurrentCar->wheelTorque[(unsigned int)i] = g_pCurrentCar->wheelTorque[(unsigned int)i] - combined;
        } else if (load < 0) {
            brake = i < 2 ? brakeA : brakeB;
            g_pCurrentCar->wheelTorque[(unsigned int)i] = g_pCurrentCar->wheelTorque[(unsigned int)i] + brake;
            g_pCurrentCar->wheelTorque[(unsigned int)i] = g_pCurrentCar->wheelTorque[(unsigned int)i] + combined;
        }

        slip = FixVecDot(&g_pCurrentCar->cornerVelocity[i], &direction);
        load = g_pCurrentCar->wheelLoad[(unsigned int)i];
        dot[i] = slip;
        fLat = slip * 4 - load;
        crossing = 0;
        if ((slip > 0 && fLat < 0) || (slip < 0 && fLat > 0)) crossing = 1;
        if (i > 1 && g_pCurrentCar->handbrake != 0 && crossing == 0) {
            g_pCurrentCar->wheelTorque[(unsigned int)i] = 0;
            load = g_pCurrentCar->wheelLoad[(unsigned int)i];
            if (load > 0) {
                brake = i < 2 ? brakeA : brakeB;
                g_pCurrentCar->wheelTorque[(unsigned int)i] = g_pCurrentCar->wheelTorque[(unsigned int)i] - brake;
                g_pCurrentCar->wheelTorque[(unsigned int)i] = g_pCurrentCar->wheelTorque[(unsigned int)i] - combined;
            } else if (load < 0) {
                brake = i < 2 ? brakeA : brakeB;
                g_pCurrentCar->wheelTorque[(unsigned int)i] = g_pCurrentCar->wheelTorque[(unsigned int)i] + brake;
                g_pCurrentCar->wheelTorque[(unsigned int)i] = g_pCurrentCar->wheelTorque[(unsigned int)i] + combined;
            }
        } else {
            int grip;
            fLong = FixMul(fLat, loadScale) * 4;

            if (g_pCurrentCar->flag0x1d0[2] == 0 && g_pCurrentCar->flag0x1d0[3] == 0) {
                grip = g_pCurrentCar->field_0xa5c[(unsigned int)i];
            } else {
                combined = FixMul(FIX_ABS(fLat), 0x4000);
                // The original indexes axes 3 and 1. The first read overruns
                // the two-axis array into uninitialised lateral scratch.
                combined -= FIX_ABS(FixVecDot(&g_pCurrentCar->cornerVelocity[i], &axis[i]));

                if (combined < 0) {
                    combined = 0;
                }
                combined = FixMul(combined, 0xccc) + 0x51e;
                if (g_pCurrentCar->cornerGrip[i].drag != 0) {
                    combined = combined + FixMul(0xe666, g_pCurrentCar->cornerGrip[i].drag);
                }
                grip = FixMul(g_pCurrentCar->field_0xa5c[(unsigned int)i], combined);
            }
            if (fLong > grip || (grip = -grip, fLong < grip)) {
                fLong = grip;
            }
            if (g_pCurrentCar->flag0x1d0[2] == 0 && g_pCurrentCar->flag0x1d0[3] == 0) {
                fLong = fLong / 4;
            } else {
                fLong = fLong / 8;
            }
            g_pCurrentCar->wheelTorque[(unsigned int)i] = g_pCurrentCar->wheelTorque[(unsigned int)i] + fLong;
        }

        load = g_pCurrentCar->wheelLoad[(unsigned int)i];
        g_pCurrentCar->wheelLoad[(unsigned int)i] = g_pCurrentCar->wheelLoad[(unsigned int)i] +
                                      FixMul(g_pCurrentCar->wheelTorque[(unsigned int)i], torqueScale);
        if (g_pCurrentCar->wheelLoad[(unsigned int)i] > torqueLimit)
            g_pCurrentCar->wheelLoad[(unsigned int)i] = torqueLimit;
        else if (g_pCurrentCar->wheelLoad[(unsigned int)i] < -torqueLimit)
            g_pCurrentCar->wheelLoad[(unsigned int)i] = -torqueLimit;
        if (g_pCurrentCar->gear != 0 && g_pCurrentCar->field_0xb84 == 0 &&
            g_pCurrentCar->driveSplit != (i < 2 ? 0 : 0x10000)) {
            int gear = g_pCurrentCar->field_0x7bc[g_pCurrentCar->gear];

            if ((gear > 0 && g_pCurrentCar->wheelLoad[(unsigned int)i] < 0) ||
                (gear < 0 && g_pCurrentCar->wheelLoad[(unsigned int)i] > 0)) {
                g_pCurrentCar->wheelLoad[(unsigned int)i] = 0;
            }
        } else {
            if ((load > 0 && g_pCurrentCar->wheelLoad[(unsigned int)i] < 0) ||
                (load < 0 && g_pCurrentCar->wheelLoad[(unsigned int)i] > 0)) {
                g_pCurrentCar->wheelLoad[(unsigned int)i] = 0;
            }
        }
        i -= 2;
    } while (i >= 0);

    dot[0] = dot[1];
    dot[2] = dot[3];
    g_pCurrentCar->wheelTorque[0] = g_pCurrentCar->wheelTorque[1];
    g_pCurrentCar->wheelTorque[2] = g_pCurrentCar->wheelTorque[3];
    g_pCurrentCar->wheelLoad[0] = g_pCurrentCar->wheelLoad[1];
    g_pCurrentCar->wheelLoad[2] = g_pCurrentCar->wheelLoad[3];

    i = 3;
    do {
        int len;

        direction = i < 2 ? wheelDir[0] : wheelDir[1];
        lateralAxis = i < 2 ? axis[0] : axis[1];
        // Slip and lateral force share the original's scalar scratch slot.
        fLat = dot[i] * 4 - g_pCurrentCar->wheelLoad[(unsigned int)i];
        g_pCurrentCar->wheelSlip[i] = fLat;
        fLong = -FixMul(FixMul(fLat, 0x4000), scale);
        fLat = FixVecDot(&g_pCurrentCar->cornerVelocity[i], &lateralAxis);
        g_pCurrentCar->wheelSlipLateral[i] = fLat;
        fLat = -FixMul(fLat, scale);

        if (FIX_ABS(fLong) > 0x10000 && FIX_ABS(fLong) >= FIX_ABS(fLat)) {
            combined = FIX_ABS(fLong);
            recip = FixDiv(0x10000, combined);
            FixVecScale(&longVec, &direction, FixMul(fLong, recip));
            FixVecScale(&latForceVec, &lateralAxis, FixMul(fLat, recip));
            force.x = longVec.x + latForceVec.x;
            force.y = longVec.y + latForceVec.y;
            force.z = longVec.z + latForceVec.z;
            combined = FixMul(FIX_SQR(FixVecNormalizeLen(&g_pCurrentCar->cornerForce[i], &force)), combined);
        } else if (FIX_ABS(fLat) > 0x10000 && FIX_ABS(fLat) >= FIX_ABS(fLong)) {
            combined = FIX_ABS(fLat);
            recip = FixDiv(0x10000, combined);
            FixVecScale(&longVec, &direction, FixMul(fLong, recip));
            FixVecScale(&latForceVec, &lateralAxis, FixMul(fLat, recip));
            force.x = longVec.x + latForceVec.x;
            force.y = longVec.y + latForceVec.y;
            force.z = longVec.z + latForceVec.z;
            combined = FixMul(FIX_SQR(FixVecNormalizeLen(&g_pCurrentCar->cornerForce[i], &force)), combined);
        } else {
            FixVecScale(&longVec, &direction, fLong);
            FixVecScale(&latForceVec, &lateralAxis, fLat);
            force.x = longVec.x + latForceVec.x;
            force.y = longVec.y + latForceVec.y;
            force.z = longVec.z + latForceVec.z;
            combined = FIX_SQR(FixVecNormalizeLen(&g_pCurrentCar->cornerForce[i], &force));
        }

        // Friction ellipse of the tyre, wider on the easier grip settings.
        {
            if (fLong < 0) fLong = -fLong;
            if (fLat < 0) fLat = -fLat;
            if (i < 2) {
                a = g_pCurrentCar->cornerGrip[i].gripB + g_pCurrentCar->wheelSurfaceFx[i].extraGrip;
                b = g_pCurrentCar->cornerGrip[i].grip2B + g_pCurrentCar->wheelSurfaceFx[i].extraGrip;
            } else {
                a = g_pCurrentCar->cornerGrip[i].gripB - 0x4ccc + g_pCurrentCar->wheelSurfaceFx[i].extraGrip;
                b = g_pCurrentCar->cornerGrip[i].grip2B - 0x4ccc + g_pCurrentCar->wheelSurfaceFx[i].extraGrip;
            }
            if (CGameInfo::GetConfiguredDifficulty() == 2) {
                if ((BYTE)RallyData_GetSelectionBits10To11() == 0 && (BYTE)RallyData_GetSelectionBits12To13() == 1) {
                    b = FixMul(b, 0x18000);
                    a = FixMul(a, 0x18000);
                }
                if ((BYTE)RallyData_GetSelectionBits10To11() == 0 && (BYTE)RallyData_GetSelectionBits12To13() == 2) {
                    b = FixMul(b, 0x13333);
                    a = FixMul(a, 0x13333);
                }
            }
            if (fLong < 0x6666) {
                limB = FixMul(g_pCurrentCar->field_0xa5c[(unsigned int)i], b);
                limA = FixMul(g_pCurrentCar->field_0xa4c[i], a);
            } else if (fLat < 0x6666) {
                limB = g_pCurrentCar->field_0xa5c[(unsigned int)i];
                limA = g_pCurrentCar->field_0xa4c[i];
            } else {
                trig.angle = FixAtan2(FixMul(fLong, a), fLat);
                a = FixMul(a, g_sinTable[(trig.raw + 0x400) & 0xfff]);
                sine = g_sinTable[trig.raw & 0xfff];
                limA = FixMul(FixSqrt(FixMul(a, a) + FixMul(sine, sine)), g_pCurrentCar->field_0xa4c[i]);
                trig.angle = FixAtan2(FixMul(fLong, b), fLat);
                // The longitudinal coefficient is dead after the first limit.
                a = FixMul(b, g_sinTable[(trig.raw + 0x400) & 0xfff]);
                sine = g_sinTable[trig.raw & 0xfff];
                limB = FixMul(FixSqrt(FixMul(a, a) + FixMul(sine, sine)), g_pCurrentCar->field_0xa5c[(unsigned int)i]);
            }
            if (combined > limA) {
                combined = limB;
            }
            FixVecScale(&g_pCurrentCar->cornerForce[i], &g_pCurrentCar->cornerForce[i], combined);
        }

        if (g_pCurrentCar->cornerGrip[i].drag != 0) {
            int along = FixVecDot(&direction, &g_pCurrentCar->cornerVelocity[i]);

            if (along != 0) {
                fLong = -FixMul(along / 4,
                                   FixMul(g_pCurrentCar->tyreGrip,
                                          g_pCurrentCar->cornerGrip[i].drag));

                FixVecScale(&force, &direction, fLong);
                g_pCurrentCar->cornerForce[i].x += force.x;
                g_pCurrentCar->cornerForce[i].y += force.y;
                g_pCurrentCar->cornerForce[i].z += force.z;
            }
        }
        i -= 2;
    } while (i >= 0);

    g_pCurrentCar->wheelSlip[0] = g_pCurrentCar->wheelSlip[1];
    g_pCurrentCar->wheelSlip[2] = g_pCurrentCar->wheelSlip[3];
    g_pCurrentCar->cornerForce[0] = g_pCurrentCar->cornerForce[1];
    g_pCurrentCar->cornerForce[2] = g_pCurrentCar->cornerForce[3];
}

// GLOBAL: CMR2 0x0053a3a8
BYTE g_unk0x0053a3a8[8][0xfc];
// GLOBAL: CMR2 0x0053acc8
BYTE g_unk0x0053acc8[16];
// "drawn" flags of the cars in the order table (0x42c840/0x42c870, indexed by
// the car index). Eight entries: the transform shadow rows start right after.
// GLOBAL: CMR2 0x0053acf0
int g_unk0x0053acf0[8];
// GLOBAL: CMR2 0x0053ad10
CarTransforms g_carTransformsShadow[8];
// GLOBAL: CMR2 0x0053c5a0
BYTE g_unk0x0053c5a0[10][0x60];
// GLOBAL: CMR2 0x0053c9a8
int g_unk0x0053c9a8;

struct CarShortValues {
    short a, b, c, d;
};
// GLOBAL: CMR2 0x0053a230
CarShortValues g_unk0x0053a230[8];

extern int g_unk0x005199c8[14];
extern int g_unk0x00519a00[14];
int *StageTiming_GetCarReplayRecord(int index);

// Per-wheel record of CarTransforms (+0x80): hub position and the three
// wheel angles (x spin, y steering, z camber), 16.16 degrees.
struct CarWheelRecord {
    FixVector pos;
    int angle[3];
};
#define CAR_WHEEL(t, w) (((CarWheelRecord *)(t)->field_0x80)[w])

// Rebuilds the transforms of the cars of a list: the previous rows are
// restored into the physics rows and moved by the ride height, and the wheel
// records of each car are refreshed from its setup record.
// Differentially checked against the original with real helpers and guarded
// transforms. Wheel angle inputs are at setup +0x240; suspension is at +0x250.
// FUNCTION: CMR2 0x0042af50
void Car_StoreRenderTransforms(short *pList, short count)
{
    int i;
    int j;
    int car;
    BYTE type;
    int factor;
    int sum;
    short typeAngle;
    short steering;
    int *pRecord;
    Car *pCar;
    BYTE *pRow;
    BYTE *pShadow;
    CarWheelRecord *pWheels;
    FixVector up;
    FixVector ride;
    FixVector pos;

    for (i = count - 1; i >= 0; i--) {
        car = pList[i];
        pCar = &g_carBuffer[car];
        type = *StageTiming_GetStartTableRecord(pCar->index);
        pRow = g_unk0x0053a3a8[car];
        pShadow = (BYTE *)&g_carTransformsShadow[car];
        memcpy(pRow, pShadow, 0x40);
        memcpy(pRow + 0x40, pShadow + 0x40, 0x40);
        for (j = 0; j < 4; j++) {
            memcpy(pRow + 0x80 + j * 0x18, pShadow + 0x80 + j * 0x18, 0x18);
            *(int *)(pRow + 0xec + j * 4) = *(int *)(pShadow + 0xec + j * 4);
        }
        *(FixVector *)(pRow + 0xe0) = *(FixVector *)(pShadow + 0xe0);
        if (pCar->field_0x958 != 0) {
            pCar->field_0x0.position.y += pCar->field_0x958;
            pCar->field_0x40.position.y += pCar->field_0x958;
        }
        FixMatrix_GetUp(&up, (FixMatrix *)pCar);
        if (type != pCar->type) {
            FixVecScale(&up, &up, g_unk0x00519a00[type] + g_unk0x005199c8[pCar->type]);
        } else {
            FixVecScale(&up, &up, g_unk0x005199c8[pCar->type]);
        }
        if (CGameInfo::IsActiveCheatEnabled(6) != 0) {
            FixMatrix_GetUp(&ride, &pCar->pNode0x71c->current);
            FixVecScale(&ride, &ride, FixMul(pCar->field_0xa8c, 0x8000));
            up.x += ride.x;
            up.y += ride.y;
            up.z += ride.z;
        }
        FixMatrix_CopyRotation((FixMatrix *)pCar, (FixMatrix *)pShadow);
        FixMatrix_CopyRotation(&pCar->field_0x40, (FixMatrix *)(pShadow + 0x40));
        *(FixVector *)(pShadow + 0xe0) = pCar->groundNormal;
        for (j = 0; j < 4; j++)
            *(int *)(pShadow + 0xec + j * 4) = pCar->cornerHeight[j];
        FixMatrix_GetPosition(&pos, (FixMatrix *)pShadow);
        pos.x += up.x;
        pos.y += up.y;
        pos.z += up.z;
        FixMatrix_SetPosition(&pos, (FixMatrix *)pShadow);
        FixMatrix_GetPosition(&pos, (FixMatrix *)(pShadow + 0x40));
        pos.x += up.x;
        pos.y += up.y;
        pos.z += up.z;
        FixMatrix_SetPosition(&pos, (FixMatrix *)(pShadow + 0x40));
    }
    for (i = count - 1; i >= 0; i--) {
        car = pList[i];
        pCar = &g_carBuffer[car];
        pShadow = (BYTE *)&g_carTransformsShadow[car];
        // One 0x18-byte record per wheel at +0x80: hub position, then the
        // three wheel angles. The original walks wheels 3..0 with every
        // pointer going down, so record j always belongs to wheel j (the
        // steering angle goes to the front wheels 0 and 1).
        pWheels = (CarWheelRecord *)(pShadow + 0x80);
        steering = pCar->field_0xb14;
        if (pCar->field_0xb70 != 0) {
            for (j = 3; j >= 0; j--)
                pWheels[j].pos = pCar->wheelEmitter[j];
        } else {
            pRecord = StageTiming_GetCarReplayRecord(car);
            for (j = 3; j >= 0; j--) {
                typeAngle = (short)(int)(__int64)((double)FixMul(0x50000,
                        *(int *)((BYTE *)pRecord + 0x240 + j * 4)) * g_unk0x00511300);
                if (j < 2) {
                    pWheels[j].angle[0] = typeAngle * 0x1680;
                    pWheels[j].angle[1] = steering * 0x1680;
                    pWheels[j].angle[2] = ((short *)&g_unk0x0053a230[car])[j] * 0x1680;
                } else {
                    pWheels[j].angle[0] = typeAngle * 0x1680;
                    pWheels[j].angle[1] = 0;
                    pWheels[j].angle[2] = ((short *)&g_unk0x0053a230[car])[j] * 0x1680;
                }
                up = pCar->wheelEmitter[j];
                up.y += *(int *)((BYTE *)pCar + 0x928 + j * 4);
                sum = *(int *)((BYTE *)StageTiming_GetCarReplayRecord(pCar->index) + 0x250 + j * 4) +
                      *(int *)((BYTE *)pCar + 0x978 + j * 4);
                if (sum > 0x10000)
                    sum = 0x10000;
                up.y += FixMul(0x10000 - sum, *(int *)((BYTE *)pCar + 0x938 + j * 4));
                up.x += *(int *)((BYTE *)pCar + 0x6fc + j * 8);
                up.y += *(int *)((BYTE *)pCar + 0x700 + j * 8);
                pWheels[j].pos = up;
            }
        }
    }
}

// FUNCTION: CMR2 0x0042b5b0
void Car_ClearWheelRotation(int first, int count)
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

void Graphics_SetPlaybackRate(float value);

// FUNCTION: CMR2 0x0042b7e0
void Car_ResetPlaybackSpeed(void)
{
    Graphics_SetPlaybackRate(100.0f);
    g_unk0x0053c9a8 = 6;
}

unsigned short Input_GetControllerSlotMapping(unsigned short);
void Car_SetDrawnFlag(int, char);
void Car_ResetRenderTransforms(void);

// GLOBAL: CMR2 0x0053a314
short g_unk0x0053a314[8];
// GLOBAL: CMR2 0x0053bd6c
short g_unk0x0053bd6c[26];
// GLOBAL: CMR2 0x0053c9a0
short g_unk0x0053c9a0;

// Builds the race car order for the given number of cars and clears the
// per-car "drawn" flags.
// match 80%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0042b660
void Car_BuildRaceOrder(int count)
{
    int i;
    int j;

    for (i = 0; i < count; i++) {
        g_carOrder[i] = i;
        g_unk0x0053a314[i] = i;
        g_unk0x0053bd6c[i] = i;
    }
    g_carOrderCount = count;
    g_unk0x0053c9a0 = count;
    g_carOrder[44] = count;
    g_carOrder[25] = count;
    memset(g_unk0x0053acc8, 0, 8);
    for (j = 0; j < 2; j++) {
        g_unk0x0053acc8[j] = Input_GetControllerSlotMapping(j);
        if (!CGameInfo::GetGameModeOptionBit19() && j >= (int)(BYTE)RallyDataState())
            Car_SetDrawnFlag(j, -1);
    }
    Car_ResetRenderTransforms();
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
BYTE Car_GetDrawnFlag(int index)
{
    return g_unk0x0053acc8[index];
}

// FUNCTION: CMR2 0x0042b720
void Car_SetDrawnFlag(int index, char value)
{
    g_unk0x0053acc8[index] = value;
}

void Collision_BuildOrientedBoxWorldCorners(FixVector *pCorners, FixVector *pCenter, FixMatrix *pRot, FixVector *pHalf);

void NetRace_DrainPendingMessages(void);
void Replay_AdvanceLiveRecordingStreams(void);
void Replay_PlaybackAllStreamFrames(void);
void NetRace_ReceiveAndIntegrateListedCars(Car *pCars, short *pIndices, short count);
void Collision_UpdateOrderedCars(BYTE *pCars, short *pOrder, short count);
void StageObject_UpdateMovingTransforms(void);
void NetRace_ExtrapolateOrderedCarPoses(Car *pCars, short *pOrder, short count);
void Replay_RecordAllStreamFrames(void);
void Car_PrepareStep(int base, short *pList, short count);
void Replay_RecordPeriodicCarSamples(void);
void StageTiming_UpdateAttachedCarParts(BYTE *pCars, short *pOrder, short count);
void StageObject_UpdateEnabledCornerContactFrames(int base, short *pList, short count);
void ForceFeedback_UpdatePlayerSlot(int param_1);
void NetRace_PackListedCars(int base, short *pList, short count);
void Car_RunStepPasses(int carBase, short *pOrder, short count);
void Car_UpdateSuspensionPass(int param_1, short *param_2, short param_3);
void Car_StepAll(int base, short *pList, short count);
void Car_PrepareBodies(int base, short *pList, short count);
void Car_IntegrateWheelRotation(int *pList, short count);

void Car_ClearRecords(int first, int count);
void Car_InvalidateTransformsRange(int first, int count);
void StageTiming_PlaceCarViewNodesAndBody(SceneNode *pNodeA, SceneNode *pNodeB, int carIndex, int param_4,
                  FixAngles *pAngles, FixVector *pPosition, int param_7);
void StageTiming_ClearCarReplayRecordRange(int first, int count);
void CarDamage_SetupOrderedCarParts(int lock, int keep, short *pOrder, short count);
void CarTyres_CommitPlayerWear(int a, int b, int count);
int StageTiming_GetStartTableField244(int unused);
int *StageTiming_GetCarStartPosition(int car);
void Events_Flush(void);
void Particle_KillAll(void);
void Car_SyncBodySceneNodes(Car *pCar);
unsigned int RallyData_GetSelectionFlag27(void);
void RallyData_ResetRaceRecordAndRouteProbe(int index);

// Scratch copies of the node pointers and the engine state of every car, saved
// while the race order is rebuilt and restored afterwards (0x42b800).
// GLOBAL: CMR2 0x0053a280
int g_unk0x0053a280[8][4];
// GLOBAL: CMR2 0x0053a300
short g_unk0x0053a300[8];
// GLOBAL: CMR2 0x0053a378
int g_unk0x0053a378[8];
// GLOBAL: CMR2 0x0053a398
BYTE g_unk0x0053a398[8];
// GLOBAL: CMR2 0x0053ab88
int g_unk0x0053ab88[8];
// GLOBAL: CMR2 0x0053abc8
int g_unk0x0053abc8[8][4];
// GLOBAL: CMR2 0x0053acd0
int g_unk0x0053acd0[8];
// GLOBAL: CMR2 0x0053b510
int g_unk0x0053b510[8];
// GLOBAL: CMR2 0x0053b538
int g_unk0x0053b538[8];
// GLOBAL: CMR2 0x0053bd44
int g_unk0x0053bd44[8];
// GLOBAL: CMR2 0x0053bd7c
int g_unk0x0053bd7c[8];

// Rebuilds the node pointers and the engine state of the given cars: saves them
// into the scratch rows, reloads the models (which clears the rows), writes the
// saved values back, refreshes the scene nodes, the surfaces and the renderer,
// and finally restores the race order.
// FUNCTION: CMR2 0x0042b800
void Car_ReloadModels(int first, int count, int param_3)
{
    int end = first + count;
    int i;
    int k;

    for (i = 0; i < count; i++)
        g_unk0x0053a300[i] = first + i;
    for (i = first; i < end; i++) {
        g_unk0x0053a378[i] = (int)g_carBuffer[i].pNode0x71c;
        g_unk0x0053b510[i] = (int)g_carBuffer[i].pNode0x720;
        g_unk0x0053a398[i] = (BYTE)g_carBuffer[i].type;
        g_unk0x0053bd7c[i] = (int)g_carBuffer[i].pNode0x724;
        g_unk0x0053b538[i] = (int)g_carBuffer[i].pViewNodeNear;
        g_unk0x0053acd0[i] = (int)g_carBuffer[i].pViewNodeFar;
        for (k = 0; k < 4; k++) {
            g_unk0x0053a280[i][k] = (int)g_carBuffer[i].pExtraNodes[k];
            g_unk0x0053abc8[i][k] = (int)g_carBuffer[i].pWheelNodes[k];
        }
        g_unk0x0053bd44[i] = g_carBuffer[i].field_0xc0c;
        g_unk0x0053ab88[i] = *(int *)((BYTE *)&g_carBuffer[i] + 0xb58);
        if (CGameInfo::GetConfiguredGameMode() == 4)
            g_unk0x0053a398[i] = 0;
    }
    Car_ClearRecords(first, count);
    Car_ClearWheelRotation(first, count);
    for (i = first; i < end; i++) {
        g_carBuffer[i].pNode0x71c = (SceneNode *)g_unk0x0053a378[i];
        g_carBuffer[i].pNode0x720 = (SceneNode *)g_unk0x0053b510[i];
        g_carBuffer[i].pNode0x724 = (SceneNode *)g_unk0x0053bd7c[i];
        g_carBuffer[i].pViewNodeNear = (SceneNode *)g_unk0x0053b538[i];
        g_carBuffer[i].pViewNodeFar = (SceneNode *)g_unk0x0053acd0[i];
        for (k = 0; k < 4; k++) {
            g_carBuffer[i].pExtraNodes[k] = (SceneNode *)g_unk0x0053a280[i][k];
            g_carBuffer[i].pWheelNodes[k] = (SceneNode *)g_unk0x0053abc8[i][k];
        }
        g_carBuffer[i].field_0xc0c = g_unk0x0053bd44[i];
        *(int *)((BYTE *)&g_carBuffer[i] + 0xb58) = g_unk0x0053ab88[i];
    }
    for (i = first; i < end; i++)
        StageTiming_PlaceCarViewNodesAndBody((SceneNode *)g_unk0x0053a378[i], (SceneNode *)g_unk0x0053b510[i], i,
                     g_unk0x0053a398[i], (FixAngles *)StageTiming_GetStartTableField244(i), (FixVector *)StageTiming_GetCarStartPosition(i), 1);
    StageTiming_ClearCarReplayRecordRange(first, count);
    CarDamage_SetupOrderedCarParts(1, param_3, g_unk0x0053a300, count);
    CarTyres_CommitPlayerWear(1, param_3, (BYTE)RallyDataState());
    Events_Flush();
    Particle_KillAll();
    Car_InvalidateTransformsRange(first, count);
    for (i = first; i < end; i++)
        Car_SyncBodySceneNodes(Car_Get(i));
    if ((BYTE)RallyData_GetSelectionFlag26() == 0 && (BYTE)RallyData_GetSelectionFlag27() == 0) {
        for (i = first; i < end; i++)
            RallyData_ResetRaceRecordAndRouteProbe(i);
    }
    Car_StoreRenderTransforms(g_unk0x0053a300, count);
}

// Rebuilds the view lists of the race and runs one whole update step over all
// of them: prepares the listed cars (wheel travel, body state, matrices), runs
// the rendering lists of the ordered, view and extra cars, then restores the
// car order and frees the temporary lists.
// FUNCTION: CMR2 0x0042baf0
void Car_UpdateAndRenderAll(void)
{
    NetRace_DrainPendingMessages();
    Car_BuildViewOrder();
    Replay_AdvanceLiveRecordingStreams();
    Replay_PlaybackAllStreamFrames();
    Car_StepAll((int)g_carBuffer, g_unk0x0053b4f0, g_unk0x0053a310);
    Car_RunStepPasses((int)g_carBuffer, (short *)&g_carViewScale[8][0], g_carOrder[26]);
    NetRace_ReceiveAndIntegrateListedCars((Car *)g_carBuffer, g_unk0x0053bd6c, g_carOrder[25]);
    Collision_UpdateOrderedCars((BYTE *)g_carBuffer, g_unk0x0053a270, g_carOrder[44]);
    StageObject_UpdateMovingTransforms();
    Car_PrepareBodies((int)g_carBuffer, g_unk0x0053b4f0, g_unk0x0053a310);
    if (g_carOrder[26] > 0)
        Car_UpdateSuspensionPass((int)g_carBuffer, (short *)&g_carViewScale[8][0], g_carOrder[26]);
    NetRace_ExtrapolateOrderedCarPoses((Car *)g_carBuffer, g_unk0x0053bd6c, g_carOrder[25]);
    Replay_RecordAllStreamFrames();
    Car_PrepareStep((int)g_carBuffer, g_unk0x0053bd6c, g_carOrder[25]);
    Car_IntegrateWheelRotation((int *)g_unk0x0053a314, g_unk0x0053c9a0);
    Car_IntegrateWheelRotation((int *)g_unk0x0053bd6c, g_carOrder[25]);
    Car_StoreRenderTransforms(g_unk0x0053a314, g_unk0x0053c9a0);
    Car_StoreRenderTransforms(g_unk0x0053bd6c, g_carOrder[25]);
    Replay_RecordPeriodicCarSamples();
    StageTiming_UpdateAttachedCarParts((BYTE *)g_carBuffer, g_unk0x0053a314, g_unk0x0053c9a0);
    StageObject_UpdateEnabledCornerContactFrames((int)g_carBuffer, g_unk0x0053b4f0, g_unk0x0053a310);
    ForceFeedback_UpdatePlayerSlot(0);
    NetRace_PackListedCars((int)g_carBuffer, g_carOrder, g_carOrderCount);
}

// Counts down field 0xb43 of every car in the list.
// FUNCTION: CMR2 0x0042bc80
void Car_DecrementContactTimers(short *pList, short count)
{
    int i;
    Car *pCar;

    for (i = count - 1; i >= 0; i--) {
        pCar = &g_carBuffer[pList[i]];
        if (pCar->field_0xb43 > 0)
            pCar->field_0xb43--;
    }
}

// Rebuilds the per-car render transforms at the start of an update: takes the
// body matrices from the scene nodes, clears the wheel records and the ground
// data, mirrors the row into the shadow copy and hands the car position to the
// transform record of the renderer.
// FUNCTION: CMR2 0x0042bcd0
void Car_ResetRenderTransforms(void)
{
    short i;
    int j;
    int car;
    int *pWheel;
    int *pHeight;
    FixVector position;

    for (i = g_carOrderCount - 1; i >= 0; i--) {
        car = g_carOrder[i];
        FixMatrix_CopyRotation(&g_carBuffer[car].pNode0x720->current, (FixMatrix *)(g_unk0x0053a3a8[car] + 0x40));
        FixMatrix_CopyRotation(&g_carBuffer[car].pNode0x71c->current, (FixMatrix *)g_unk0x0053a3a8[car]);
        pHeight = (int *)(g_unk0x0053a3a8[car] + 0xec);
        pWheel = (int *)(g_unk0x0053a3a8[car] + 0x84);
        for (j = 4; j != 0; j--) {
            pWheel[-1] = 0;
            pWheel[0] = 0;
            pWheel[1] = 0;
            pWheel[2] = 0;
            pWheel[3] = 0;
            pWheel[4] = 0;
            *pHeight = 0;
            pHeight++;
            pWheel += 6;
        }
        *(int *)(g_unk0x0053a3a8[car] + 0xe0) = 0;
        *(int *)(g_unk0x0053a3a8[car] + 0xe4) = 0x10000;
        *(int *)(g_unk0x0053a3a8[car] + 0xe8) = 0;
        memcpy(&g_carTransformsShadow[car], g_unk0x0053a3a8[car], 0xfc);
        memcpy(&g_carTransforms[car], g_unk0x0053a3a8[car], 0xfc);
        FixMatrix_GetPosition(&position, (FixMatrix *)g_unk0x0053a3a8[car]);
        Collision_BuildOrientedBoxWorldCorners((FixVector *)g_unk0x0053c5a0[car], &position, (FixMatrix *)g_unk0x0053a3a8[car],
                     &g_carBuffer[car].halfExtents);
    }
}

int Car_UsesNarrowWheels(Car *pCar, int param2);
void CarTransforms_Copy(CarTransforms *pDst, CarTransforms *pSrc);
void Car_UpdateWheelMeshStates(void);

// 16.16 degrees to a sine table index (4096 per turn); defined in GameInfo.cpp.
extern double g_unk0x00511300;

// Interpolates the render transforms of every car in the draw order between
// the physics row (0x53a3a8) and its shadow by the car's time step, rebuilds
// the four wheel matrices from the wheel records, normalises the ground normal
// and marks the nodes of the extra parts for a refresh.
// match 30%: implemented from the disassembly; the instruction sequence of the
// wheel loop, the angle conversion (g_unk0x00511300 + fistp), the FixVecLength/
// FixVecScaleRecip pair and the calls all line up, but the frame layout and the
// address temps the original kept in locals differ, and that changes the whole
// register allocation. Kept as FUNCTION so reccmp keeps measuring it.
// FUNCTION: CMR2 0x0042bf70
void Car_InterpolateRenderTransforms(void)
{
    int i;
    short *pOrder;
    int car;
    CarTransforms *pRow;
    CarTransforms *pShadow;
    CarTransforms *pTrans;
    FixMatrix *pWheelM;
    int scale;
    int wheel;
    int k;
    int n;
    FixBasis basis;
    FixVector v;
    FixVector position;
    short angles[3];
    int result[3];
    int ca;
    int sa;
    FixVector *p;
    int *pYZ;
    SceneNode *pNode;

    n = g_carOrderCount - 1;
    if (n >= 0) {
        pOrder = &g_carOrder[n];
        i = n + 1;
        do {
            car = *pOrder;
            scale = *(int *)((BYTE *)&g_carBuffer[car] + 0xa90);
            pRow = (CarTransforms *)g_unk0x0053a3a8[car];
            pShadow = &g_carTransformsShadow[car];
            pTrans = &g_carTransforms[car];
            if (g_unk0x0053acf0[g_carBuffer[car].index] != 0) {
                CarTransforms_Copy(pRow, pShadow);
                g_unk0x0053acf0[g_carBuffer[car].index] = 0;
            }
            FixMatrix_Interpolate(&pTrans->body2, &pRow->body2, &pShadow->body2,
                                  scale, scale, scale, 1);
            FixMatrix_CopyRotation(&pTrans->body2, &g_carBuffer[car].pNode0x720->current);
            FixMatrix_Interpolate(&pTrans->body, &pRow->body, &pShadow->body,
                                  scale, scale, scale, 1);
            FixMatrix_CopyRotation(&pTrans->body, &g_carBuffer[car].pNode0x71c->current);

            for (wheel = 0; wheel < 4; wheel++) {
                // Each wheel record is lerped from the physics row towards
                // the shadow: hub position, then the three wheel angles.
                v.x = CAR_WHEEL(pShadow, wheel).pos.x - CAR_WHEEL(pRow, wheel).pos.x;
                v.y = CAR_WHEEL(pShadow, wheel).pos.y - CAR_WHEEL(pRow, wheel).pos.y;
                v.z = CAR_WHEEL(pShadow, wheel).pos.z - CAR_WHEEL(pRow, wheel).pos.z;
                FixVecScale(&v, &v, scale);
                CAR_WHEEL(pTrans, wheel).pos.x = CAR_WHEEL(pRow, wheel).pos.x + v.x;
                CAR_WHEEL(pTrans, wheel).pos.y = CAR_WHEEL(pRow, wheel).pos.y + v.y;
                CAR_WHEEL(pTrans, wheel).pos.z = CAR_WHEEL(pRow, wheel).pos.z + v.z;
                v.x = CAR_WHEEL(pShadow, wheel).angle[0] - CAR_WHEEL(pRow, wheel).angle[0];
                v.y = CAR_WHEEL(pShadow, wheel).angle[1] - CAR_WHEEL(pRow, wheel).angle[1];
                v.z = CAR_WHEEL(pShadow, wheel).angle[2] - CAR_WHEEL(pRow, wheel).angle[2];
                FixVecScale(&v, &v, scale);
                result[0] = CAR_WHEEL(pRow, wheel).angle[0] + v.x;
                result[1] = CAR_WHEEL(pRow, wheel).angle[1] + v.y;
                result[2] = CAR_WHEEL(pRow, wheel).angle[2] + v.z;
                CAR_WHEEL(pTrans, wheel).angle[0] = result[0];
                CAR_WHEEL(pTrans, wheel).angle[1] = result[1];
                CAR_WHEEL(pTrans, wheel).angle[2] = result[2];

                // The right-hand wheels point backwards: 180 degrees plus the
                // wheel angles.
                if (wheel % 2 == 1) {
                    angles[0] = (short)(__int64)((double)(-result[0]) * g_unk0x00511300);
                    angles[1] = (short)(__int64)((double)(result[1] + 0xb40000) * g_unk0x00511300);
                    angles[2] = (short)(__int64)((double)(-result[2]) * g_unk0x00511300);
                } else {
                    angles[0] = (short)(__int64)((double)result[0] * g_unk0x00511300);
                    angles[1] = (short)(__int64)((double)result[1] * g_unk0x00511300);
                    angles[2] = (short)(__int64)((double)result[2] * g_unk0x00511300);
                }

                basis.right.x = 0x10000;
                basis.up.y = 0x10000;
                basis.forward.z = 0x10000;
                basis.right.y = 0;
                basis.right.z = 0;
                basis.up.x = 0;
                basis.up.z = 0;
                basis.forward.x = 0;
                basis.forward.y = 0;
                FixBasis_Rotate(&basis, (unsigned short *)angles);

                // Camber: every row of the wheel basis is turned about x
                // (its y and z), the right wheels one way and the left ones
                // the other.
                if (wheel % 2 == 1) {
                    ca = g_sinTable[0x41c];
                    sa = g_sinTable[0x1c];
                } else {
                    ca = g_sinTable[0x3e4];
                    sa = g_sinTable[0xfe4];
                }
                pYZ = &basis.right.y;
                for (k = 3; k != 0; k--) {
                    pYZ[0] = FixMul(pYZ[0], ca) - FixMul(pYZ[1], sa);
                    pYZ[1] = FixMul(pYZ[0], sa) + FixMul(pYZ[1], ca);
                    pYZ += 3;
                }

                if (Car_UsesNarrowWheels(&g_carBuffer[car], 0) != 0)
                    FixVecScale(&basis.forward, &basis.forward, 0x9999);
                if (CGameInfo::IsActiveCheatEnabled(6) != 0) {
                    p = &basis.right;
                    for (k = 3; k != 0; k--) {
                        FixVecScale(p, p, 0x28000);
                        p++;
                    }
                }

                pWheelM = &g_carWheelTransforms[car][wheel];
                FixMatrix_SetRight(&basis.right, pWheelM);
                FixMatrix_SetUp(&basis.up, pWheelM);
                FixMatrix_SetForward(&basis.forward, pWheelM);
                FixMatrix_SetPosition(&CAR_WHEEL(pTrans, wheel).pos, pWheelM);
                FixMatrix_CopyRotation(pWheelM, &g_carBuffer[car].pWheelNodes[wheel]->current);
                if (g_carBuffer[car].pExtraNodes[wheel] != NULL) {
                    FixMatrix_CopyRotation(pWheelM, &g_carBuffer[car].pExtraNodes[wheel]->current);
                    if (Car_UsesNarrowWheels(&g_carBuffer[car], 0) == 0 && Car_UsesNarrowWheels(&g_carBuffer[car], 1) != 0) {
                        FixMatrix_GetForward(&basis.forward, &g_carBuffer[car].pExtraNodes[wheel]->current);
                        FixVecScale(&basis.forward, &basis.forward, 0x9999);
                        FixMatrix_SetForward(&basis.forward, &g_carBuffer[car].pExtraNodes[wheel]->current);
                    }
                    // Mark the node and its parents up to the root dirty.
                    for (pNode = g_carBuffer[car].pExtraNodes[wheel]; pNode != NULL; pNode = pNode->pParent)
                        pNode->dirty = 1;
                }
            }

            v.x = pShadow->groundNormal.x - pRow->groundNormal.x;
            v.y = pShadow->groundNormal.y - pRow->groundNormal.y;
            v.z = pShadow->groundNormal.z - pRow->groundNormal.z;
            FixVecScale(&v, &v, scale);
            pTrans->groundNormal.x = pRow->groundNormal.x + v.x;
            pTrans->groundNormal.y = pRow->groundNormal.y + v.y;
            pTrans->groundNormal.z = pRow->groundNormal.z + v.z;
            FixVecNormalizeLen(&pTrans->groundNormal, &pTrans->groundNormal);
            for (k = 0; k < 4; k++)
                pTrans->cornerHeight[k] =
                    FixMul(pShadow->cornerHeight[k] - pRow->cornerHeight[k], scale) + pRow->cornerHeight[k];
            FixMatrix_GetPosition(&position, (FixMatrix *)pTrans);
            Collision_BuildOrientedBoxWorldCorners((FixVector *)((BYTE *)g_unk0x0053c5a0 + car * 0x60), &position,
                         (FixMatrix *)pTrans, &g_carBuffer[car].halfExtents);
            pOrder--;
        } while (--i != 0);
    }
    Car_UpdateWheelMeshStates();
}

// Copies a car's render transforms (body matrices, wheel records, ground data).
// FUNCTION: CMR2 0x0042c7b0
void CarTransforms_Copy(CarTransforms *pDst, CarTransforms *pSrc)
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
void Car_InvalidateTransformsRange(int first, int count)
{
    int i;

    for (i = first; i < first + count; i++)
        g_unk0x0053acf0[i] = 1;
}

// FUNCTION: CMR2 0x0042c870
void Car_InvalidateTransforms(int index)
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

BYTE InRaceMenu_IsOpening(void);
void InRaceMenu_ClearOpeningFlag(void);
int Race_ConsumeTransitionLatch(void);

// Walks the car order and recomputes each car's fraction of the timing window
// (the engine note falloff), writing it to the car and returning the largest.
// match 51%: same logic, but MSVC6 keeps the original in an EBP frame with
// memory-resident loop counters; the register allocation does not reproduce.
// match 50%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0042c890
int Car_UpdateEngineNoteFalloff(int *pOut)
{
    int i;
    int max;
    short offset;
    unsigned int mid;
    float fromStart;
    float toEnd;
    float frac;
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
    if (InRaceMenu_IsOpening() != 0)
        g_unk0x0053a374 = g_unk0x0053bd40;
    if (Race_ConsumeTransitionLatch() != 0)
        g_unk0x0053a374 = g_unk0x0053bd40;
    InRaceMenu_ClearOpeningFlag();
    max = 0;
    for (i = 0; i < g_carOrderCount; i++) {
        pCar = &g_carBuffer[g_carOrder[i]];
        if (g_carOrder[i] > 0 &&
            *(float *)(pCar->field_0xa90 + 8) == *(float *)(g_carBuffer->field_0xa90 + 8)) {
            *(int *)pCar->field_0xa90 = *(int *)g_carBuffer->field_0xa90;
            offset = g_carBuffer->field_0xb43;
        } else {
            fromStart = ((float)(unsigned int)(g_unk0x0053a374 - g_unk0x0053bd64) * g_unk0x005113a0) *
                        *(float *)(pCar->field_0xa90 + 8);
            toEnd = ((float)(unsigned int)(g_unk0x0053bd40 - g_unk0x0053bd64) * g_unk0x005113a0) *
                    *(float *)(pCar->field_0xa90 + 8);
            mid = (int)(__int64)toEnd;
            frac = toEnd - (float)mid;
            *(int *)pCar->field_0xa90 = (int)(frac * g_65536f);
            offset = mid - (int)(__int64)fromStart;
            if (offset > 5)
                offset = 5;
        }
        pCar->field_0xb43 = (char)offset;
        if ((offset & 0xff) > max)
            max = offset & 0xff;
    }
    *pOut = *(int *)g_carBuffer->field_0xa90;
    g_unk0x0053a374 = g_unk0x0053bd40;
    return max;
}

// FUNCTION: CMR2 0x0042ca70
CarTransforms *Car_GetTransforms(int index)
{
    return &g_carTransforms[index];
}

// FUNCTION: CMR2 0x0042ca90
BYTE *Car_GetPhysicsRow(int index)
{
    return g_unk0x0053a3a8[index];
}

// FUNCTION: CMR2 0x0042cab0
FixMatrix *Car_GetWheelTransforms(int index)
{
    return g_carWheelTransforms[index];
}

// FUNCTION: CMR2 0x0042cac0
BYTE *Car_GetRendererRecord(int index)
{
    return g_unk0x0053c5a0[index];
}

BYTE *StageTiming_GetStartTableRecord(int index);

// Whether the car uses the narrow wheel setup (car class 6 on the normal
// surfaces); param2 also accepts the 'A' variant.
// FUNCTION: CMR2 0x0042cae0
int Car_UsesNarrowWheels(Car *pCar, int param2)
{
    char stage;

    stage = *StageTiming_GetStartTableRecord(pCar->index);
    if (!CGameInfo::IsActiveCheatEnabled(6) && pCar->field_0xb29 == 6 && stage != 0xb && stage != 8 && stage != 0xa &&
        stage != 0xd && (StageTiming_GetStartTableRecord(pCar->index)[0x20] != 'A' || param2 != 0))
        return 1;
    return 0;
}

// Sets the suspension geometry of the current car from the ride-height
// setting (0..0x10000).
// FUNCTION: CMR2 0x0043dff0
void Car_SetRideHeight(int param_1)
{
    int off;
    int inv;
    int n0;
    int n1;

    if (CGameInfo::IsActiveCheatEnabled(6) != 0)
        param_1 = -0x6666;
    g_pCurrentCar->field_0x9b8 = FixMul(param_1, 0x6666) + 0x9999;
    g_pCurrentCar->field_0x9bc = FixMul(param_1, 0x3333) + 0x6666;
    n0 = FixMul(param_1, -0x1aaaa) + 0x50000;
    n1 = FixMul(param_1, -0x1999) + 0x3333;
    g_pCurrentCar->field_0x8d8 = g_pCurrentCar->field_0x75c / 8;
    g_pCurrentCar->field_0x9c0 = FixDiv(0x10000, n1);
    g_pCurrentCar->field_0x9c4 = FixDiv(0x10000, n0);
    inv = 0x10000 - param_1;
    for (off = 0; off < 4; off++) {
        ((int *)g_pCurrentCar->field_0x9e8)[off] = 0x4ccc;
        ((int *)g_pCurrentCar->field_0x9e8)[off + 4] = FixMul(param_1, 0x5999) + 0x4000;
        g_pCurrentCar->wheel0x988[off] = FixMul(0x1999, inv);
        g_pCurrentCar->field_0x978[off] = FixMul(param_1, 0x8000) + 0x8000;
    }
    g_pCurrentCar->field_0xa08 = -0x1999;
}

// FUNCTION: CMR2 0x0043e160
void Car_SetSteeringSwingTarget(int value)
{
    g_pCurrentCar->field_0x82c = FixMul(0x13333, value) + 0x3333;
}

// FUNCTION: CMR2 0x0043e190
void Car_SetBrakeBias(int value)
{
    g_pCurrentCar->brakeBias = value;
}

// FUNCTION: CMR2 0x0043e1b0
void Car_SetDriveSplit(int value)
{
    g_pCurrentCar->driveSplit = value;
}

// FUNCTION: CMR2 0x0043e1d0
void Car_SetSurfaceDragLevel(int value)
{
    g_pCurrentCar->field_0xb29 = (BYTE)value;
}

int Car_GetScaledSteerFollowRate(void);

// Engine torque of the current car (less the speed drag) split between the
// front and rear wheels by the drive split.
// match 63%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00442fc0
void Car_SplitEngineTorque(void)
{
    int torque;
    int front;
    int split;

    torque = Car_GetScaledSteerFollowRate();
    g_pCurrentCar->field_0x780 = torque - FixMul(g_pCurrentCar->field_0x784,
        FixMul(g_pCurrentCar->field_0x7a4, g_pCurrentCar->field_0x7a4));
    split = FixMul(g_pCurrentCar->field_0x780, g_pCurrentCar->field_0x7bc[g_pCurrentCar->gear]);
    front = FixMul(split, g_pCurrentCar->driveSplit) / 2;
    g_pCurrentCar->wheelTorque[0] = front;
    g_pCurrentCar->wheelTorque[1] = front;
    front = split / 2 - front;
    g_pCurrentCar->wheelTorque[2] = front;
    g_pCurrentCar->wheelTorque[3] = front;
}

// Integrates the ride-height accumulator of the current car toward the
// average wheel load (or the engine speed while it is off the ground).
// match 88%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004430b0
void Car_UpdateRideHeight(void)
{
    int hi;
    int lo;

    if (g_pCurrentCar->gear == 0) {
        g_pCurrentCar->field_0x7a4 += FixMul(g_pCurrentCar->field_0x780, 0xa0000);
        if (g_pCurrentCar->field_0x7a4 > g_pCurrentCar->field_0x794) {
            g_pCurrentCar->field_0x7a4 = g_pCurrentCar->field_0x794;
            return;
        }
    } else {
        lo = (FixMul(g_pCurrentCar->wheelLoad[0], g_pCurrentCar->field_0x7bc[g_pCurrentCar->gear]) +
              FixMul(g_pCurrentCar->field_0x7bc[g_pCurrentCar->gear], g_pCurrentCar->wheelLoad[1])) / 2;
        hi = (FixMul(g_pCurrentCar->wheelLoad[2], g_pCurrentCar->field_0x7bc[g_pCurrentCar->gear]) +
              FixMul(g_pCurrentCar->field_0x7bc[g_pCurrentCar->gear], g_pCurrentCar->wheelLoad[3])) / 2;
        g_pCurrentCar->field_0x7a4 +=
            FixMul(FixMul(g_pCurrentCar->driveSplit, lo - hi) - g_pCurrentCar->field_0x7a4 + hi, 0x10000);
    }
}

// FUNCTION: CMR2 0x00443230
void Car_CopySteeringFollowState(void)
{
    g_pCurrentCar->field_0x7ac = g_pCurrentCar->field_0x7a4;
}

// FUNCTION: CMR2 0x00417760
int Car_IsStateC10Set(int index)
{
    return Car_Get(index)->field_0xc10 != 0;
}

// Starts (param2 != 0) or stops the camera shake of a view: 0 idle, 1 active,
// 2 locked, 3 stopping.
// FUNCTION: CMR2 0x00423010
void View_SetShake(int view, int start)
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
BOOL Car_ShowsCleanWheels(Car *pCar)
{
    char stage;

    stage = *StageTiming_GetStartTableRecord(pCar->index);
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
// match Car_ShowsCleanWheels, reloading every texture that changed.
// FUNCTION: CMR2 0x0042cb90
void Car_SwapWheelTextures(char mode, SceneNode **pWheels)
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
                if (Car_ShowsCleanWheels(g_pCurrentCar)) {
                    if (strncmp(pTex->name + strlen(pTex->name) - 9, g_strWheelVariantN, 1) == 0) {
                        strncpy(pTex->name + strlen(pTex->name) - 9, g_strWheelVariantL, 1);
                        Graphics_ReloadTexture(pTex);
                    }
                } else if (strncmp(pTex->name + strlen(pTex->name) - 9, g_strWheelVariantL, 1) == 0) {
                    strncpy(pTex->name + strlen(pTex->name) - 9, g_strWheelVariantN, 1);
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
// match 75%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00443a20
void Car_UpdateCornerVelocityPair(void)
{
    FixMatrix *pWorld;
    FixVector *pVel;
    FixVector *pCorner;
    int a;
    int b;

    pVel = &g_pCurrentCar->velocity;
    pWorld = g_pCurrentCar->pWorld;
    pCorner = g_pCurrentCar->cornerVelocity;
    a = FixMul(g_pCurrentCar->halfExtents.x, g_pCurrentCar->angularVelocity.z);
    b = FixMul(g_pCurrentCar->angularVelocity.y, g_pCurrentCar->halfExtents.x);
    pCorner[0].x = FixMul(pWorld->up.x, a);
    pCorner[0].x -= FixMul(pWorld->forward.x, b);
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

void Car_FollowGround(void);
void Car_RebuildBodyAxes(void);

// Runs the suspension/geometry pass over the listed cars: selects each car,
// caches its setup, copies the wheel surfaces, rebuilds the body and keeps the
// smallest corner clearance of the eight body corners.
// FUNCTION: CMR2 0x00443bf0
void Car_UpdateSuspensionPass(int param_1, short *param_2, short param_3)
{
    short *p;
    int n;
    int i;
    int j;
    int v;

    n = param_3 - 1;
    if (n < 0)
        return;
    p = param_2 + n;
    n++;
    do {
        g_pCurrentCar = (Car *)(param_1 + *p * 0xc24);
        g_pCarSetup = (BYTE *)StageTiming_GetCarReplayRecord((int)g_pCurrentCar->index);
        Car_UpdateWheelTravel();
        g_pCurrentCar->field_0xabe[0] = g_pCurrentCar->wheelSurface[0];
        g_pCurrentCar->field_0xabe[1] = g_pCurrentCar->wheelSurface[1];
        g_pCurrentCar->field_0xabe[2] = g_pCurrentCar->wheelSurface[2];
        g_pCurrentCar->field_0xabe[3] = g_pCurrentCar->wheelSurface[3];
        Car_FollowGround();
        Car_RebuildBodyAxes();
        g_pCurrentCar->field_0x960 = 0x3e80000;
        for (i = 0; i < 8; i++) {
            v = g_pCurrentCar->corners[i].y - g_pCurrentCar->cornerHeight[i];
            if (v < g_pCurrentCar->field_0x960)
                g_pCurrentCar->field_0x960 = v;
        }
        g_pCurrentCar->field_0x960 += g_pCurrentCar->field_0x958;
        p--;
    } while (--n);
}

// out = v / |v|, or zero when v is zero: FIX_NORMALIZE_INTO with the output
// passed by pointer (computed once, the zero case stores through it).
static inline void FixVecNormalizeInto(FixVector *out, FixVector *v)
{
    int len = FixVecLength(v);
    if (len == 0) {
        out->x = 0;
        out->y = 0;
        out->z = 0;
    } else {
        FixVecScaleRecip(out, v, len);
    }
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
    FixVector step;
    unsigned short surface;
    int ease;
    int along;
    int slow;
    int h;
    int len;
    int k;
    int i;
    int damping;

    g_pCurrentCar->cornerHeight[0] =
        Track_GetGroundHeightSurface(&g_pCurrentCar->position, &n, &g_pCurrentCar->cornerTriangle[0],
                                     &g_pCurrentCar->wheelSurface[0], &surface, g_pCurrentCar->cornerHeight[0]);
    if (g_pCurrentCar->cornerOnGround[4] != 0) {
        g_pCurrentCar->cornerHeight[0] += g_pCurrentCar->field_0x8fc[0];
        n = g_pCurrentCar->field_0x564[0];
        g_pCurrentCar->wheelSurface[0] = 0x2f;
    }
    if (g_pCurrentCar->wheelSurface[0] == 0xf && g_pCurrentCar->field_0xa7c == 0 && g_pCurrentCar->field_0xbf8 == 0)
        g_pCurrentCar->field_0xa7c = 0x190000;

    // Ease the ground normal towards the new one.
    oldNormal = g_pCurrentCar->groundNormal;
    dn.x = n.x - g_pCurrentCar->groundNormal.x;
    dn.y = n.y - g_pCurrentCar->groundNormal.y;
    dn.z = n.z - g_pCurrentCar->groundNormal.z;
    ease = FixMul(0x8000, g_physicsTimeStep);
    if (ease > 0x8000)
        ease = 0x8000;
    FixVecScale(&step, &dn, ease);
    dn.x -= step.x;
    dn.y -= step.y;
    dn.z -= step.z;
    n.x = g_pCurrentCar->groundNormal.x + dn.x;
    n.y = g_pCurrentCar->groundNormal.y + dn.y;
    n.z = g_pCurrentCar->groundNormal.z + dn.z;
    FixVecNormalizeInto(&g_pCurrentCar->groundNormal, &n);
    along = FixVecDot(&g_pCurrentCar->groundNormal, &g_pCurrentCar->velocity);
    slow = FixMul(g_pCurrentCar->speed, 0x8000);
    if (slow > 0x10000)
        slow = 0x10000;
    slow = FixMul(0x10000 - slow, 0x1999);
    if (g_pCurrentCar->field_0xb74 != 0) {
        if (along <= slow) {
            g_pCurrentCar->up = g_pCurrentCar->groundNormal;
            FixVecScale(&t, &g_pCurrentCar->up, FixVecDot(&g_pCurrentCar->up, &g_pCurrentCar->right));
            t.x = g_pCurrentCar->right.x - t.x;
            t.y = g_pCurrentCar->right.y - t.y;
            t.z = g_pCurrentCar->right.z - t.z;
            FixVecNormalizeInto(&g_pCurrentCar->right, &t);
            FixVecCross(&t, &g_pCurrentCar->right, &g_pCurrentCar->up);
            FixVecNormalizeInto(&g_pCurrentCar->forward, &t);
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
    t.x = g_pCurrentCar->halfExtents.x;
    t.y = -g_pCurrentCar->halfExtents.y;
    t.z = g_pCurrentCar->halfExtents.z;
    if (rel.y >= 0)
        t.y = g_pCurrentCar->halfExtents.y;
    if (rel.x < 0)
        t.x = -g_pCurrentCar->halfExtents.x;
    if (rel.z < 0)
        t.z = -g_pCurrentCar->halfExtents.z;
    FixMatrix_RotateVector(&p, &t, g_pCurrentCar->pWorld);
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
        // Computed and never used (the asm helper is not optimised away).
        damping = FixMul(0x1eb8, g_physicsTimeStep);
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
            FixMatrix_InverseRotateVector(&corner, &dn, g_pCurrentCar->pWorld);
            corner.y = 0;
            len = FixVecLength(&corner);
            if (len > 10000)
                FixVecScale(&corner, &corner, FixDiv(10000, len));
            k = -FixMul(g_pCurrentCar->halfExtents.y, FixMul(0x4ccc, g_physicsTimeStep));
            g_pCurrentCar->angularVelocity.x -= FixMul(corner.z, k);
            g_pCurrentCar->angularVelocity.z += FixMul(corner.x, k);
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
    limit = FixMul(0x3d1, FixDiv(sum, 0x40000));
    FixVecScale(&t, &g_pCurrentCar->groundNormal, FixVecDot(&g_pCurrentCar->forward, &g_pCurrentCar->groundNormal));
    t.x = g_pCurrentCar->forward.x - t.x;
    t.y = g_pCurrentCar->forward.y - t.y;
    t.z = g_pCurrentCar->forward.z - t.z;
    FIX_NORMALIZE_INTO(dir, t);
    slide = FixVecDot(&dir, &g_pCurrentCar->velocity);
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
            tip = tip > 0 ? 0x10000 : -0x10000;
    }
    target = FixMul(tip, 0x10000);
    diff = target - g_pCurrentCar->tipRatio;
    rate = FixMul(0x1999, g_physicsTimeStep);
    if (FIX_ABS(diff) < rate) {
        g_pCurrentCar->tipRatio = target;
    } else {
        if (diff <= 0)
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
int StageTiming_GetCheckpointField12(int index);

void Car_BalanceTwoPlayerRideHeight(BYTE *param_1, short *param_2, short param_3);
void Car_StartPendingCountdown(void);
void Car_UpdateLowSpeedWheelLoadTimer(void);
void CarEffects_UpdateBrokenLightFlicker(BYTE *pCar);
void Car_UpdateSlopeGrip(void);
void Car_ShareWeightOnWheels(void);
void Car_UpdateEngineTorque(void);
void Car_IntegrateContacts(void);
void Car_UpdateEngineSpeed(void);
void Car_UpdateResponseValueCountdown(void);
int StageObject_GetCarWeatherRampValue(BYTE *pCar);
void Car_UpdateSurfaceParams(Car *pCar, int blend);
void Car_UpdateCornerLoads(void);
void Car_UpdateSteering(void);
void Car_UpdateCornerVelocities(void);
short Sector_GetNeighbours(FixVector *pPos, short *pOut);
void AutoGear_UpdateCarGearState(Car *pCar);

// One whole update step over every car of the list: rebuilds the body frame and
// the corner state of each car, runs its ground/contact, engine, steering and
// friction updates, then publishes the final position and the surface of every
// car.
// match 75%: same logic, offsets, constants and calls as the original; MSVC6
// keeps the shared loop bound in the count parameter slot of the original and
// spills it into a frame slot here, which re-numbers the registers of every
// loop prologue, and the two inner clear loops get a different instruction
// order. Kept as FUNCTION so reccmp keeps measuring it.
// FUNCTION: CMR2 0x00433890
void Car_StepAll(int base, short *pList, short count)
{
    FixVector v;
    short out[3];
    int f;
    int m;
    int i;
    int k;

    Car_BalanceTwoPlayerRideHeight((BYTE *)base, pList, count);
    for (i = count - 1; i >= 0; i--) {
        g_unk0x0053c9d4 = 0;
        g_carStepAccel.x = 0;
        g_carStepAccel.y = 0;
        g_carStepAccel.z = 0;
        g_pCurrentCar = (Car *)(base + pList[i] * 0xc24);
        Car_ScaleRatesByTimeStep(g_pCurrentCar);
        for (k = 0x300; k < 0x330; k += 0xc) {
            *(FixVector *)((BYTE *)g_pCurrentCar + k + 0x30) =
                *(FixVector *)((BYTE *)g_pCurrentCar + k);
            *(FixVector *)((BYTE *)g_pCurrentCar + k) =
                *(FixVector *)((BYTE *)g_pCurrentCar + k - 0x90);
        }
        g_pCurrentCar->positionPrev2 =
            g_pCurrentCar->positionPrev;
        g_pCurrentCar->positionPrev = g_pCurrentCar->position;
        memcpy((BYTE *)g_pCurrentCar + 0x384, (BYTE *)g_pCurrentCar + 0x360, 0x24);
        g_pCurrentCar->field_0x968 = g_pCurrentCar->field_0x964;
        g_pCurrentCar->field_0x964 = g_pCurrentCar->field_0x960;
        if (g_pCurrentCar->field_0xbfc == 0)
            Car_StartPendingCountdown();
        Car_UpdateSurfaceParams(g_pCurrentCar, StageObject_GetCarWeatherRampValue((BYTE *)g_pCurrentCar));
        Car_UpdateLowSpeedWheelLoadTimer();
        CarEffects_UpdateBrokenLightFlicker((BYTE *)g_pCurrentCar);
    }
    for (i = count - 1; i >= 0; i--) {
        g_pCurrentCar = (Car *)(base + pList[i] * 0xc24);
        AutoGear_UpdateCarGearState(g_pCurrentCar);
        for (k = 0x888; k >= 0x880; k -= 4) {
            *(int *)((BYTE *)g_pCurrentCar + k - 0xc) = 0;
            *(int *)((BYTE *)g_pCurrentCar + k + 4) = 0;
        }
    }
    for (i = count - 1; i >= 0; i--) {
        g_pCurrentCar = (Car *)(base + pList[i] * 0xc24);
        g_pCurrentCar->field_0xb74 = 1;
        FixMatrix_InverseRotateVector(&v, &g_pCurrentCar->groundNormal, g_pCurrentCar->pWorld);
        if (v.y <= 0x106 || FIX_ABS(v.x) > FixMul(FIX_ABS(v.y), 0x93cd) ||
            FIX_ABS(v.z) > FixMul(FIX_ABS(v.y), 0x10000))
            g_pCurrentCar->field_0xb74 = 0;
    }
    for (i = count - 1; i >= 0; i--) {
        g_pCurrentCar = (Car *)(base + pList[i] * 0xc24);
        g_pCarSetup = (BYTE *)StageTiming_GetCarReplayRecord((int)g_pCurrentCar->index);
        if (*(int *)(g_pCarSetup + 0x46c) == 0 && *(BYTE *)(g_pCarSetup + 0x469) != 0)
            *(BYTE *)(g_pCarSetup + 0x469) -= 1;
        *(int *)(g_pCarSetup + 0x46c) = 0;
        Car_UpdateCornerVelocities();
        memset(&g_pCurrentCar->cornerLoad, 0, 0x60);
        memset(&g_pCurrentCar->cornerForce, 0, 0x60);
        g_pCurrentCar->baseForce.x = 0;
        f = FixMul(0x8000, g_gravityScale);
        m = g_pCurrentCar->field_0x75c;
        g_pCurrentCar->baseForce.y = -FixMul(m, f);
        g_pCurrentCar->baseForce.z = 0;
        Car_ShareWeightOnWheels();
        Car_UpdateSuspension();
        Car_UpdateSlopeGrip();
        Car_UpdateCornerLoads();
        Car_UpdateSteering();
    }
    for (i = count - 1; i >= 0; i--) {
        g_pCurrentCar = (Car *)(base + pList[i] * 0xc24);
        g_pCarSetup = (BYTE *)StageTiming_GetCarReplayRecord((int)g_pCurrentCar->index);
        Car_UpdateEngineTorque();
    }
    for (i = count - 1; i >= 0; i--) {
        g_pCurrentCar = (Car *)(base + pList[i] * 0xc24);
        Car_UpdateCornerFriction();
    }
    for (i = count - 1; i >= 0; i--) {
        g_pCurrentCar = (Car *)(base + pList[i] * 0xc24);
        g_pCarSetup = (BYTE *)StageTiming_GetCarReplayRecord((int)g_pCurrentCar->index);
        Car_IntegrateContacts();
    }
    for (i = count - 1; i >= 0; i--) {
        g_pCurrentCar = (Car *)(base + pList[i] * 0xc24);
        g_pCurrentCar->field_0x7a8 = g_pCurrentCar->field_0x7a4;
        Car_UpdateEngineSpeed();
        Car_UpdateResponseValueCountdown();
        g_pCurrentCar->field_0x5c4.x = 0;
        g_pCurrentCar->field_0x5c4.y = 0;
        g_pCurrentCar->field_0x5c4.z = 0;
        g_pCurrentCar->field_0x5d0.x = 0;
        g_pCurrentCar->field_0x5d0.y = 0;
        g_pCurrentCar->field_0x5d0.z = 0;
    }
    for (i = count - 1; i >= 0; i--) {
        g_pCurrentCar = (Car *)(base + pList[i] * 0xc24);
        g_pCurrentCar->sector =
            Sector_GetNeighbours(&g_pCurrentCar->position, out);
        g_pCurrentCar->field_0xb02 = out[0];
        g_pCurrentCar->field_0xb04 = out[1];
        g_pCurrentCar->field_0xb06 = out[2];
        g_pCurrentCar->field_0x2dc = g_pCurrentCar->position;
    }
}

// Transfers ride height between the two cars so the one further back along
// the road is raised; only active in the 2-player game (state 2).
// FUNCTION: CMR2 0x00433e80
void Car_BalanceTwoPlayerRideHeight(BYTE *param_1, short *param_2, short param_3)
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
    if ((BYTE)CGameInfo::GetNetworkOptionBit11() == 0)
        return;
    pCar1 = param_1 + param_2[0] * 0xc24;
    pCar2 = param_1 + param_2[1] * 0xc24;
    diff = StageTiming_GetCheckpointField12(*(char *)(pCar1 + 0xb1a));
    diff -= StageTiming_GetCheckpointField12(*(char *)(pCar2 + 0xb1a));
    dist = diff * 0x10000;
    if (dist < 0)
        dist = -dist;
    if (dist <= 0x30000) {
        t = 0;
    } else if (dist < 0xf0000) {
        t = dist - 0x30000;
        t = FixMul(t, FixDiv(0x10000, 0xc0000));
        if (t < 0)
            t = 0;
        else if (t > 0x10000)
            t = 0x10000;
    } else {
        t = 0x10000;
    }
    amount = FixMul(t, 0xccc);
    if (diff < 0) {
        *(int *)(pCar2 + 0x788) = *(int *)(pCar2 + 0x78c) - amount;
        *(int *)(pCar1 + 0x788) = *(int *)(pCar1 + 0x78c) + amount;
    } else {
        *(int *)(pCar1 + 0x788) = *(int *)(pCar1 + 0x78c) - amount;
        *(int *)(pCar2 + 0x788) = *(int *)(pCar2 + 0x78c) + amount;
    }
}

// FUNCTION: CMR2 0x00433fd0
void Car_UpdateResponseValueCountdown(void)
{
    int target = g_pCurrentCar->field_0x794 - FixMul(g_pCurrentCar->field_0xb1d << 16, 0x3333);

    if (g_pCurrentCar->field_0x7a4 - target >= -0x28f) {
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
void Car_UpdateSlopeGrip(void)
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
void Car_UpdateLowSpeedWheelLoadTimer(void)
{
    int load;

    if (g_pCurrentCar->field_0xb1f != 0) {
        if (g_pCurrentCar->driveSplit == 0)
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
    unsigned short angle;
    int w;

    if (FIX_ABS(g_pCurrentCar->speed) < 0x28f) {
        angle = 0;
    } else {
        angle = g_pCurrentCar->heading;
        if (*(int *)(g_pCarSetup + 0x3d8) > 0) {
            if (FIX_ABS(g_pCurrentCar->position.x) - FIX_ABS(g_pCurrentCar->position.z) < 0)
                w = -FIX_ABS(g_pCurrentCar->position.x) + FIX_ABS(g_pCurrentCar->position.z);
            else
                w = FIX_ABS(g_pCurrentCar->position.x) - FIX_ABS(g_pCurrentCar->position.z);
            w = FixMul(0x20000, (w % 0x401 - 0x200) * 0x40);
            if (g_pCurrentCar->speed < 0x10000)
                w = FixMul(w, g_pCurrentCar->speed);
            w = FixMul(w, *(int *)(g_pCarSetup + 0x3d8));
            angle += (short)(__int64)((double)w * g_unk0x00511300);
        }
    }
    right = g_pCurrentCar->right;
    FixVecScale(&t, &g_pCurrentCar->up, FixVecDot(&g_pCurrentCar->rearWheelDir, &g_pCurrentCar->up));
    t.x = g_pCurrentCar->rearWheelDir.x - t.x;
    t.y = g_pCurrentCar->rearWheelDir.y - t.y;
    t.z = g_pCurrentCar->rearWheelDir.z - t.z;
    FixVecNormalizeLen(&g_pCurrentCar->rearWheelDir, &t);
    t.x = right.x - g_pCurrentCar->rearWheelDir.x;
    t.y = right.y - g_pCurrentCar->rearWheelDir.y;
    t.z = right.z - g_pCurrentCar->rearWheelDir.z;
    w = FixVecLength(&t);
    w = FixMul(w, 0x13333);
    w = FixMul(w, FixMul(FixMul(0xcccd, g_pCurrentCar->steerFollowRate) + 0x3333, g_physicsTimeStep));
    if (w >= 0x10000) {
        g_pCurrentCar->rearWheelDir = right;
    } else {
        FixVecScale(&t, &t, w);
        g_pCurrentCar->rearWheelDir.x += t.x;
        g_pCurrentCar->rearWheelDir.y += t.y;
        g_pCurrentCar->rearWheelDir.z += t.z;
        FixVecNormalizeLen(&g_pCurrentCar->rearWheelDir, &g_pCurrentCar->rearWheelDir);
    }
    if (angle == 0) {
        g_pCurrentCar->frontWheelDir = g_pCurrentCar->rearWheelDir;
    } else {
        ((void (__stdcall *)(FixMatrix *, FixVector *, short))FixMatrix_FromAxisAngle)(&m, &g_pCurrentCar->up, angle);
        FixMatrix_RotateVector(&t, &g_pCurrentCar->rearWheelDir, &m);
        FixVecNormalizeLen(&g_pCurrentCar->frontWheelDir, &t);
    }
    FixVecCross(&g_pCurrentCar->frontWheelAxis, &g_pCurrentCar->frontWheelDir, &g_pCurrentCar->up);
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
    load = -FixMul(g_pCurrentCar->cornerMass + 0x1e0000, FixVecDot(&n0, &g_gravityDir));
    FixVecScale(&f, &n0, FixMul(load, 0x10000));
    grip = FixMul(g_pCurrentCar->speed, FixMul(0x1cccc, 0x10000));
    if (grip > 0x1cccc)
        grip = 0x1cccc;
    grip += 0x10000;
    for (i = 0; i < 8; i++) {
        pGripB = &g_pCurrentCar->cornerGripB[i];
        if (g_pCurrentCar->cornerFlags[i] == 0 || (i < 4 && g_pCurrentCar->cornerOnGround[i] != 0)) {
            if (n != 4) {
                g_pCurrentCar->cornerLoad[i].x += f.x;
                g_pCurrentCar->cornerLoad[i].y += f.y;
                g_pCurrentCar->cornerLoad[i].z += f.z;
            }
            if (i < 4 && g_pCurrentCar->field_0xb74 != 0) {
                load = FixMul(FixMul(g_pCurrentCar->cornerAxis[i].y, 0x8000), g_pCurrentCar->field_0x8b8[i] + 0x1e0000);
                g_pCurrentCar->field_0xa5c[i] =
                    FixMul(load, g_pCurrentCar->wheelSurfaceFx[i].drag +
                                     g_pCurrentCar->cornerGrip[i].grip2A);
                g_pCurrentCar->field_0xa4c[i] =
                    FixMul(load, g_pCurrentCar->wheelSurfaceFx[i].drag +
                                     g_pCurrentCar->cornerGrip[i].gripA);
            } else {
                g_pCurrentCar->cornerGripA[i] = FixMul(load, g_pCurrentCar->cornerGrip[i].grip2A);
                *pGripB = FixMul(load, g_pCurrentCar->cornerGrip[i].gripA);
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
        FixVecScale(&f, &n0, FixVecDot(&g_pCurrentCar->baseForce, &n0));
        f.x = g_pCurrentCar->baseForce.x - f.x;
        f.y = g_pCurrentCar->baseForce.y - f.y;
        f.z = g_pCurrentCar->baseForce.z - f.z;
        FixVecScale(&f, &f, 0x10000 - g_unk0x0053c9d4);
        if (g_pCurrentCar->field_0xb74 == 0) {
            if (g_pCurrentCar->field_0xb34 >= 1)
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
        k = FixMul(-0xb333, FixDiv(0x10000, 0x3d70));
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

    if (g_pCurrentCar->gear == 0 || g_pCurrentCar->field_0xb84 != 0) {
        if (g_pCurrentCar->field_0xb4c == 0) {
            if (g_pCurrentCar->field_0xafe == 0) {
                g_pCurrentCar->field_0x7a4 += FixMul(g_pCurrentCar->field_0x780, 0xa0000);
            } else if (g_pCurrentCar->field_0xafe > 0) {
                angle = (short)(__int64)((double)FixMul(FixMul(0x190000 - FixMul(0x41, g_pCurrentCar->field_0xafe << 16), 0xa3d),
                                                             0x8c0000) * g_unk0x00511300);
                g_pCurrentCar->field_0x7a4 = FixMul(FixMul(g_sinTable[(unsigned short)angle & 0xfff], 0x10000), g_pCurrentCar->field_0x794);
                g_pCurrentCar->field_0xafe -= (short)(FixMul(g_physicsTimeStep, 0x3e80000) >> 16);
                if (g_pCurrentCar->field_0xafe < 0)
                    g_pCurrentCar->field_0xafe = 0;
                if (angle < 0x400)
                    g_pCurrentCar->steerFollowRate = 0x10000;
            }
        } else if (g_pCurrentCar->field_0xafe < 0x3e9) {
            g_pCurrentCar->field_0xb4c = 0;
            g_pCurrentCar->field_0xafe = 25000;
        } else {
            g_pCurrentCar->field_0xafe -= (short)FixMulShift32(g_physicsTimeStep, 0x3e80000);
            if (g_pCurrentCar->field_0xafe < 0)
                g_pCurrentCar->field_0xafe = 0;
        }
    } else {
        front = FixMul(g_pCurrentCar->wheelLoad[0], g_pCurrentCar->field_0x7bc[g_pCurrentCar->gear]) + FixMul(g_pCurrentCar->field_0x7bc[g_pCurrentCar->gear], g_pCurrentCar->wheelLoad[1]);
        rear = front;
        if (g_pCurrentCar->handbrake == 0 || g_pCurrentCar->driveSplit == 0)
            rear = FixMul(g_pCurrentCar->field_0x7bc[g_pCurrentCar->gear], g_pCurrentCar->wheelLoad[2]) + FixMul(g_pCurrentCar->field_0x7bc[g_pCurrentCar->gear], g_pCurrentCar->wheelLoad[3]);
        g_pCurrentCar->field_0x7a4 += FixMul(FixMul(g_pCurrentCar->driveSplit, front / 2 - rear / 2) - g_pCurrentCar->field_0x7a4 + rear / 2,
                                  0x10000);
    }

    excess = g_pCurrentCar->field_0x7a4;
    if (excess < 0) {
        g_pCurrentCar->field_0x7a4 = 0;
    } else if (g_pCurrentCar->field_0x794 < excess) {
        excess = excess - g_pCurrentCar->field_0x794;
        if (excess < 0xcccd) {
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
void Car_RebuildBodyAxes(void)
{
    FixVector v[3];
    FixVector tmp;
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
        FixMatrix_SetForward(&v[2], g_pCurrentCar->pBodyMatrix);
    } else {
        FixMatrix_SetRight(&g_pCurrentCar->right, g_pCurrentCar->pBodyMatrix);
        FixMatrix_SetUp(&g_pCurrentCar->up, g_pCurrentCar->pBodyMatrix);
        FixMatrix_SetForward(&g_pCurrentCar->forward, g_pCurrentCar->pBodyMatrix);
    }
    scale = g_pCurrentCar->wheel0x988[0];
    FixVecScale(&tmp, &g_pCurrentCar->up, scale);
    tmp.x += g_pCurrentCar->position.x;
    tmp.y += g_pCurrentCar->position.y;
    tmp.z += g_pCurrentCar->position.z;
    g_pCurrentCar->pBodyMatrix->position = tmp;
}

void Car_ShareWeightOnWheels(void);
void Car_LiftOutOfGround(void);
void Car_UpdateSurfaceParams(Car *pCar, int blend);
void CarDamage_StepClimbingCarRecords(int param_1, short count);
short Sector_GetNeighbours(FixVector *pPos, short *pOut);

// Prepares every car of the order list for a physics step: restores the body
// frame from the world matrix (or resets the per-step state when the car is
// not settled), refreshes the velocity magnitude, the corner velocities, the
// body axes and the wheel loads, lifts the car out of the ground and refreshes
// its surface parameters and the sector neighbours of its position.
// FUNCTION: CMR2 0x00444c10
void Car_PrepareStep(int carBase, short *pOrder, short count)
{
    short *p;
    int n;
    int i;
    int lift;
    short nb[3];
    FixVector v;
    FixVector *pRight;
    FixVector *pForward;

#define CARF(off) (*(int *)((int)g_pCurrentCar + (off)))
#define CARV(off) (*(FixVector *)((int)g_pCurrentCar + (off)))
#define CARB(off) (*(char *)((int)g_pCurrentCar + (off)))

    n = count - 1;
    if (n >= 0) {
        p = &pOrder[n];
        n++;
        do {
            g_pCurrentCar = (Car *)(carBase + *p * 0xc24);
            lift = g_pCurrentCar->field_0x960;
            if (lift < 0)
                lift = 0;

            if (g_pCurrentCar->field_0xc20 == 0) {
                for (i = 0; i < 4; i++) {
                    g_pCurrentCar->cornerPrev2[i] =
                        g_pCurrentCar->cornerPrev[i];
                    g_pCurrentCar->cornerPrev[i] =
                        g_pCurrentCar->corners[i];
                    g_pCurrentCar->field_0xabe[i] = g_pCurrentCar->wheelSurface[i];
                }
                for (i = 0; i < 8; i++) {
                    g_pCurrentCar->cornerNormal[i] =
                        g_pCurrentCar->cornerAxis[i];
                    g_pCurrentCar->cornerOnGround[4 + i] = 0;
                }
                g_pCurrentCar->field_0x7a8 = g_pCurrentCar->field_0x7a4;
                g_pCurrentCar->positionPrev2 = g_pCurrentCar->positionPrev;
                g_pCurrentCar->positionPrev = g_pCurrentCar->position;
                g_pCurrentCar->normal0x498 = g_pCurrentCar->groundNormal;
                memcpy(g_pCurrentCar->field_0x384, &g_pCurrentCar->right, 0x24);
                FixMatrix_GetPosition(&g_pCurrentCar->position, g_pCurrentCar->pWorld);
                FixMatrix_GetRight(&g_pCurrentCar->right, g_pCurrentCar->pWorld);
                FixMatrix_GetUp(&g_pCurrentCar->up, g_pCurrentCar->pWorld);
                FixMatrix_GetForward(&g_pCurrentCar->forward, g_pCurrentCar->pWorld);
                Car_UpdateCorners(g_pCurrentCar);
            }

            g_pCurrentCar->speed = FixVecLength(&g_pCurrentCar->velocity);
            for (i = 0x42c; i < 0x48c; i += 0xc)
                *(FixVector *)((BYTE *)g_pCurrentCar + i) = g_pCurrentCar->velocity;
            g_pCurrentCar->field_0xb14 = *(short *)&g_pCurrentCar->heading;
            g_pCurrentCar->field_0xb12 = g_pCurrentCar->field_0xb14;
            g_pCurrentCar->field_0x958 = 0;
            g_pCurrentCar->field_0xb5c = 0;
            g_pCurrentCar->wheelLoad[3] = FixVecDot(&g_pCurrentCar->velocity, &g_pCurrentCar->right) * 4;
            g_pCurrentCar->wheelLoad[2] = g_pCurrentCar->wheelLoad[3];
            g_pCurrentCar->wheelLoad[1] = g_pCurrentCar->wheelLoad[2];
            g_pCurrentCar->wheelLoad[0] = g_pCurrentCar->wheelLoad[1];
            g_pCurrentCar->field_0x7a4 = 0;
            g_pCurrentCar->gear = 0;
            for (i = 0; i < 8; i++) {
                int u = FixMul(g_pCurrentCar->wheelLoad[0], g_pCurrentCar->field_0x7bc[i]);
                if (u > g_pCurrentCar->field_0x7a4 && u <= g_pCurrentCar->field_0x794) {
                    g_pCurrentCar->field_0x7a4 = u;
                    g_pCurrentCar->gear = (char)i;
                }
            }
            g_pCurrentCar->field_0x7ac = g_pCurrentCar->field_0x7a4;
            g_pCurrentCar->field_0xb60 = 0;
            StageObject_UpdateCarCornerGroundHeights(g_pCurrentCar, 8);
            Car_FlagAirborneCorners();
            Car_ShareWeightOnWheels();
            Car_UpdateGroundNormal();

            if (g_pCurrentCar->field_0xc20 != 0) {
                if (g_pCurrentCar->field_0xc00 != 0) {
                    Car_LiftOutOfGround();
                    g_pCurrentCar->position.y += lift;
                    g_pCurrentCar->corners[0].y += lift;
                    g_pCurrentCar->corners[1].y += lift;
                    g_pCurrentCar->corners[2].y += lift;
                    g_pCurrentCar->corners[3].y += lift;
                    g_pCurrentCar->corners[4].y += lift;
                    g_pCurrentCar->corners[5].y += lift;
                    g_pCurrentCar->corners[6].y += lift;
                    g_pCurrentCar->corners[7].y += lift;
                    Car_FlagAirborneCorners();
                    Car_ShareWeightOnWheels();
                } else {
                    pRight = &g_pCurrentCar->right;
                    pForward = &g_pCurrentCar->forward;
                    g_pCurrentCar->up = g_pCurrentCar->groundNormal;
                    i = FixVecDot(&g_pCurrentCar->up, pRight);
                    FixVecScale(&v, &g_pCurrentCar->up, i);
                    v.x = pRight->x - v.x;
                    v.y = pRight->y - v.y;
                    v.z = pRight->z - v.z;
                    FIX_NORMALIZE_INTO(pRight[0], v)
                    FixVecCross(&v, pRight, &g_pCurrentCar->up);
                    FIX_NORMALIZE_INTO(pForward[0], v)
                    FixMatrix_SetRight(pRight, g_pCurrentCar->pWorld);
                    FixMatrix_SetUp(&g_pCurrentCar->up, g_pCurrentCar->pWorld);
                    FixMatrix_SetForward(pForward, g_pCurrentCar->pWorld);
                    Car_UpdateCorners(g_pCurrentCar);
                    StageObject_UpdateCarCornerGroundHeights(g_pCurrentCar, 8);
                    Car_LiftOutOfGround();
                    g_pCurrentCar->position.y += lift;
                    g_pCurrentCar->corners[0].y += lift;
                    g_pCurrentCar->corners[1].y += lift;
                    g_pCurrentCar->corners[2].y += lift;
                    g_pCurrentCar->corners[3].y += lift;
                    g_pCurrentCar->corners[4].y += lift;
                    g_pCurrentCar->corners[5].y += lift;
                    g_pCurrentCar->corners[6].y += lift;
                    g_pCurrentCar->corners[7].y += lift;
                    Car_FlagAirborneCorners();
                    Car_ShareWeightOnWheels();
                    Car_UpdateGroundNormal();
                }
                FixMatrix_SetPosition(&g_pCurrentCar->position, g_pCurrentCar->pWorld);
            }

            if (g_pCurrentCar->field_0xbf8 != 0) {
                for (i = 0; i < 4; i++) {
                    *(FixVector *)((BYTE *)g_pCurrentCar + 0x270 + i * 0xc + 0xc0) =
                        g_pCurrentCar->corners[i];
                    *(FixVector *)((BYTE *)g_pCurrentCar + 0x270 + i * 0xc + 0x90) =
                        g_pCurrentCar->corners[i];
                    g_pCurrentCar->field_0xabe[i] = g_pCurrentCar->wheelSurface[i];
                }
                for (i = 0; i < 8; i++) {
                    g_pCurrentCar->cornerNormal[i] =
                        g_pCurrentCar->cornerAxis[i];
                    g_pCurrentCar->cornerOnGround[4 + i] = 0;
                }
                g_pCurrentCar->field_0x7a8 = g_pCurrentCar->field_0x7a4;
                g_pCurrentCar->positionPrev2 = g_pCurrentCar->position;
                g_pCurrentCar->positionPrev = g_pCurrentCar->position;
                g_pCurrentCar->normal0x498 = g_pCurrentCar->groundNormal;
                memcpy(g_pCurrentCar->field_0x384, &g_pCurrentCar->right, 0x24);
                g_pCurrentCar->wheelLean.x = 0;
                g_pCurrentCar->wheelLean.y = 0;
                g_pCurrentCar->wheelLean.z = 0;
                g_pCurrentCar->lean.x = 0;
                g_pCurrentCar->lean.y = 0;
                g_pCurrentCar->lean.z = 0;
            }
            g_pCurrentCar->field_0xbf8 = 0;
            g_pCurrentCar->field_0xc20 = 0;
            g_pCurrentCar->field_0xb74 = 1;
            FixMatrix_InverseRotateVector(&v, &g_pCurrentCar->groundNormal, g_pCurrentCar->pWorld);
            if (!(v.y > 0x106 && FIX_ABS(v.x) <= FixMul(FIX_ABS(v.y), 0x93cd) &&
                  FIX_ABS(v.z) <= FixMul(FIX_ABS(v.y), 0x10000)))
                g_pCurrentCar->field_0xb74 = 0;
            for (i = 0; i < 4; i++) {
                g_pCurrentCar->field_0x890[i] = 0;
                g_pCurrentCar->field_0x8a0[i] = 0;
            }
            Car_UpdateSurfaceParams(g_pCurrentCar, 0);
            Car_IntegrateWheelTravel();
            g_pCurrentCar->field_0xb1d = 0;
            Car_UpdateSuspension();
            Car_StoreBodyMatrix();
            g_pCurrentCar->sector =
                Sector_GetNeighbours(&g_pCurrentCar->position, nb);
            g_pCurrentCar->field_0xb02 = nb[0];
            g_pCurrentCar->field_0xb04 = nb[1];
            g_pCurrentCar->field_0xb06 = nb[2];
            p--;
        } while (--n != 0);
    }
    CarDamage_StepClimbingCarRecords((int)pOrder, count);
}

// Lifts the current car out of the ground by the deepest penetration of any
// of its eight box corners, rising or sinking as needed.
// FUNCTION: CMR2 0x004458d0
void Car_LiftOutOfGround(void)
{
    int maxDrop = 0;
    int foundDrop = 0;
    int maxRise = -0x3e80000;
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
int View_IsPlaybackCameraActive(void)
{
    if (View_GetActiveCameraMode(0) != 9 && g_unk0x0053cdb4 == 0)
        return 0;
    return 1;
}

BYTE *Car_GetPhysicsRow(int index);
int StageObject_IsNegativeRightAngleCameraSpot(BYTE *pCar);
int StageObject_GetCarCameraSpotIndex(BYTE *p);

// Rotation of car `index`'s camera placement.
// FUNCTION: CMR2 0x00423a00
FixMatrix *View_GetCarCameraRotation(FixMatrix *pOut, BYTE index)
{
    FixMatrix_CopyRotationFrom(pOut, (FixMatrix *)Car_GetPhysicsRow(index));
    return pOut;
}

// Position of a car's camera target node (at +0x750).
// FUNCTION: CMR2 0x00423db0
FixVector *View_GetCarCameraTarget(FixVector *pOut, BYTE index)
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
// FUNCTION: CMR2 0x00423de0
void CameraState_Interpolate(CameraRecord *pOut, CameraRecord *pA, CameraRecord *pB, int t)
{
    FixMatrix_Interpolate(&pOut->matrix, &pA->matrix, &pB->matrix, t, t, t, 1);
    pOut->field_0x48 = pA->field_0x48 + FixMul(pB->field_0x48 - pA->field_0x48, t);
    pOut->field_0x4c = pA->field_0x4c + FixMul(pB->field_0x4c - pA->field_0x4c, t);
    pOut->field_0x54 = pA->field_0x54 + FixMul(pB->field_0x54 - pA->field_0x54, t);
    pOut->field_0x58 = pA->field_0x58 + FixMul(pB->field_0x58 - pA->field_0x58, t);
    pOut->field_0x5c = pA->field_0x5c + FixMul(pB->field_0x5c - pA->field_0x5c, t);
    pOut->field_0x50 = pA->field_0x50 + FixMul(pB->field_0x50 - pA->field_0x50, t);
    pOut->clearance = pA->clearance + FixMul(pB->clearance - pA->clearance, t);
}

// Copies the state record at src into dst (fields 0x4..0x64 except 0x0).
// FUNCTION: CMR2 0x00423ee0
void CameraState_Copy(CameraRecord *dst, CameraRecord *src)
{
    dst->type = src->type;
    *(Unk00423ee0Block *)&dst->matrix = *(Unk00423ee0Block *)&src->matrix;
    dst->field_0x48 = src->field_0x48;
    dst->field_0x4c = src->field_0x4c;
    dst->field_0x54 = src->field_0x54;
    dst->field_0x58 = src->field_0x58;
    dst->field_0x5c = src->field_0x5c;
    dst->field_0x50 = src->field_0x50;
    dst->clearance = src->clearance;
}

// Scale of the render distance for the detail level: base * (1 + step).
// FUNCTION: CMR2 0x00423f30
int Render_GetDetailDistanceScale(void)
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
    *(volatile float *)&base =
        (float)(*(int *)&g_pGraphics->field921_0x3c4 * CGraphics::m_oneOver65536);
    float scale = base * steps[CGameInfo::GetGraphicsOptionBits21To24()];

    return (int)(__int64)((scale + base) * CGraphics::m_65536);
}

// FUNCTION: CMR2 0x00423fc0
int View_IsNegativeRightAngleCameraSpot(int view)
{
    return StageObject_IsNegativeRightAngleCameraSpot(VIEW_MODE_RECORD(view));
}

int RallyData_IsDriverViewModeAllowed(unsigned int index, int mode);
void View_SwitchCamera(unsigned char index, int a, int b, unsigned char c, int d);
unsigned int StageObject_FindNearestCameraSpot(FixVector *pPos);
int View_GetCameraSpotIndex(unsigned int view);

// Next free view-mode slot (0..0xa) of a player's view record.
// FUNCTION: CMR2 0x004218d0
int View_FindFreeModeSlot(unsigned int view)
{
    int mode;
    int found;

    mode = *(int *)(g_unk0x0053901c +
                    (BYTE)(g_unk0x00538e0c[view & 0xff] + (char)view * 2) * 100);
    do {
        mode++;
        if (mode == 0xb)
            mode = 0;
        found = RallyData_IsDriverViewModeAllowed(view, mode);
    } while (found == 0);
    View_SwitchCamera(view, mode, 0xffff, View_GetActiveCameraFlags(view), 1);
    return mode;
}

// Moves the view's mode record to the surface the camera target sits on.
// FUNCTION: CMR2 0x00421930
void View_UpdateModeSurface(unsigned int view)
{
    FixVector target;
    unsigned int surface;

    surface = StageObject_FindNearestCameraSpot(View_GetCarCameraTarget(&target, View_GetActiveCameraFlags(view)));
    if (surface != View_GetCameraSpotIndex(view))
        View_SwitchCamera(view, 7, surface, View_GetActiveCameraFlags(view), 0);
}

// FUNCTION: CMR2 0x00421980
int View_GetCameraSpotIndex(unsigned int view)
{
    view &= 0xff;
    return StageObject_GetCarCameraSpotIndex(VIEW_MODE_RECORD(view));
}

// FUNCTION: CMR2 0x00437f90
int Car_GetScaledSteerFollowRate(void)
{
    return FixMul(g_pCurrentCar->steerFollowRate, *(int *)(g_pCarSetup + 0x404));
}

int StageObject_GetCarNodeSlotValue(BYTE index);
int StageObject_GetSelectedRecordListState(void);

// Whether view mode `mode` is available for car `index`.
// FUNCTION: CMR2 0x004232a0
int View_IsModeAvailable(BYTE index, int mode)
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
        return (unsigned int)StageObject_GetSelectedRecordListState() > 0;
    case 3:
        result = StageObject_GetCarNodeSlotValue((BYTE)index);
        break;
    }
    return result;
}

// Clears `count` car records (0xc24 bytes) from `first`.
// FUNCTION: CMR2 0x0042b740
void Car_ClearRecords(int first, int count)
{
    int i;

    for (i = first; i < count + first; i++)
        memset((BYTE *)g_carBuffer + i * 0xc24, 0, 0xc24);
}

float Graphics_GetFrameScale(void);
void Graphics_SetPlaybackRate(float value);
void StageObject_SetPhysicsScaleAndReciprocal(int value);
extern int g_unk0x0053c9a8;

// GLOBAL: CMR2 0x005210c8
float g_65536f = 65536.0f;

// Counts down the frame-rate hold (restoring rate 1.0 when it ends) and
// passes the current rate on to the stage objects.
// FUNCTION: CMR2 0x0042b790
void Physics_UpdateRateHold(void)
{
    if (g_unk0x0053c9a8 != 0) {
        if (g_unk0x0053c9a8 == 1)
            Graphics_SetPlaybackRate(1.0f);
        g_unk0x0053c9a8--;
    }
    {
        float rate = Graphics_GetFrameScale();
        int scaled;

        __asm {
            fld rate
            fmul g_65536f
            fistp scaled
        }
        StageObject_SetPhysicsScaleAndReciprocal(scaled);
    }
}

// Sets the physics scale (value / 25, at least 0.6) and the time step
// (its reciprocal).
// FUNCTION: CMR2 0x00433840
void Physics_SetScale(int value)
{
    g_physicsScale = FixMul(0xa3d, value);
    if (g_physicsScale < 0x9999)
        g_physicsScale = 0x9999;
    g_physicsTimeStep = FixDiv(0x10000, g_physicsScale);
}

void FixMatrix_GetForward(FixVector *pOut, FixMatrix *pM);

// Heading (12-bit angle) of a view's camera from its forward vector.
// FUNCTION: CMR2 0x00421fe0
void View_GetHeading(short *pOut, unsigned int view)
{
    FixVector forward;
    int az;
    int ax;
    short angle;

    FixMatrix_GetForward(&forward, (FixMatrix *)(g_unk0x00538d2c + 4 + (view & 0xff) * 100));
    az = forward.z < 0 ? -forward.z : forward.z;
    ax = forward.x < 0 ? -forward.x : forward.x;
    angle = FixAtan2(ax, az);
    *pOut = angle;
    if (forward.z < 0)
        *pOut = 0x800 - angle;
    if (forward.x < 0)
        *pOut = -*pOut;
}

void FixMatrix_CopyRotationFrom(FixMatrix *pDst, FixMatrix *pSrc);
void FixMatrix_SetPosition(FixVector *pV, FixMatrix *pM);

// A car's body matrix, raised by its camera shake when that option is on.
// FUNCTION: CMR2 0x00423a30
FixMatrix *View_GetCarBodyMatrix(FixMatrix *pOut, BYTE car)
{
    FixVector up;
    FixVector position;
    int shake;

    FixMatrix_CopyRotationFrom(pOut, Car_Get(car)->pBodyMatrix);
    if (CGameInfo::IsActiveCheatEnabled(6)) {
        FixMatrix_GetUp(&up, Car_Get(car)->pWorld);
        shake = FixMul(Car_Get(car)->field_0xa8c, 0x8000);
        FixVecScale(&up, &up, shake);
        FixMatrix_GetPosition(&position, pOut);
        position.x += up.x;
        position.y += up.y;
        position.z += up.z;
        FixMatrix_SetPosition(&position, pOut);
    }
    return pOut;
}

int RallyData_GetRouteAdditionalState(void);
void RallyData_GetRouteNodeGroundPosition(int index, int *pOut);
int StageObject_UsesExtendedMode(void);

typedef void (*CarFadeCallback)(BYTE index);
void NetRace_FadeOutPlayerScreen(BYTE index, CarFadeCallback pfnDone, int param3, int param4, int param5, char force);

void Car_ResetToStage(unsigned int param_1);

// Starts the reset fade for the current car when its reset request is active.
// FUNCTION: CMR2 0x00431c10
void Car_StartResetFade(void)
{
    union {
        BYTE channels[4];
        int value;
    } colour;

    colour.channels[0] = 0;
    colour.channels[1] = 0;
    colour.channels[2] = 0;
    colour.channels[3] = 0;

    if (g_pCurrentCar->field_0xbf8 != 0)
        NetRace_FadeOutPlayerScreen((BYTE)g_pCurrentCar->index, (CarFadeCallback)Car_ResetToStage, 2, 1, colour.value, 0);
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
int RallyData_GetActiveCarRaceRecordField0(BYTE *p);
BYTE *RallyData_GetAvailableRouteNodeRecord(int index);
int RallyData_GetRouteAvailabilityState(void);
void RallyRoute_CopyIndexedNodeDirection(unsigned int nodeIndex, FixVector *pOut);
int Car_OverlapsOtherCar(int param_1);
void Car_PlaceAtStart(int *param_1, int *param_2);
void Race_StorePlayerRouteAndClearSlots(int player);
void View_ResetCameras(int view);
void Particle_KillAll(void);

// Restores the car of the given player to the stage start: finds the closest
// record of the player that is still valid, takes its position and heading
// from the scene node (or from the stored body vectors when the record has no
// node), rebuilds the body frame, kills the particles of the previous state
// and clears the reset flags.
// FUNCTION: CMR2 0x00431c50
void Car_ResetToStage(unsigned int param_1)
{
    FixVector heading;
    FixVector position;
    int node;
    BYTE *pRecord;

    g_pCurrentCar = Car_Get(param_1 & 0xff);
    node = RallyData_GetActiveCarRaceRecordField0((BYTE *)g_pCurrentCar);
    pRecord = RallyData_GetAvailableRouteNodeRecord(node);
    if (pRecord != NULL) {
        while (Car_OverlapsOtherCar(node) != 0) {
            node--;
            if (node < 0) {
                if (RallyData_GetRouteAdditionalState() != 0)
                    node += RallyData_GetRouteAvailabilityState();
                else
                    node = 0;
            }
        }
        while ((*(BYTE *)(pRecord + 0x18) & 2) != 0 && node > 0) {
            node--;
            pRecord = RallyData_GetAvailableRouteNodeRecord(node);
        }
        RallyRoute_CopyIndexedNodeDirection(node, &heading);
        RallyData_GetRouteNodeGroundPosition(node, (int *)&position);
    } else {
        position = g_pCurrentCar->position;
        heading = g_pCurrentCar->right;
    }
    Car_PlaceAtStart((int *)&position, (int *)&heading);
    Race_StorePlayerRouteAndClearSlots(param_1 & 0xff);
    Particle_KillAll();
    if ((BYTE)param_1 < (BYTE)RallyDataState())
        View_ResetCameras(param_1);
    g_pCurrentCar->field_0xbf8 = 0;
    g_pCurrentCar->field_0xa7c = 0;
    g_pCurrentCar->field_0xa80 = 0;
}

// Checks whether the current car overlaps another car in extended mode.
// FUNCTION: CMR2 0x00431d80
int Car_OverlapsOtherCar(int param_1)
{
    short *pOrder;
    short count;
    short i;
    int result;
    Car *pOther;

    result = 0;
    if (param_1 == 0 && RallyData_GetRouteAdditionalState() == 0)
        return 0;
    pOrder = Car_GetOrder();
    count = Car_GetOrderCount();
    RallyData_GetRouteNodeGroundPosition(param_1, (int *)&g_unk0x0053cae8);
    for (i = 0; i < count; i++) {
        pOther = Car_Get(pOrder[i]);
        if (pOther->index != g_pCurrentCar->index && StageObject_UsesExtendedMode() != 0) {
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
                FixVecScaleRecip(&g_unk0x0053ca58, &g_unk0x0053ca58, g_unk0x0053ca24);
            }
            g_unk0x0053c9c4 = FixVecLength(&g_unk0x0053ca58);
            g_unk0x0053c9c4 = FixMul(g_unk0x0053c9c4, g_unk0x0053ca24);
            if (g_unk0x0053c9c4 < pOther->field_0x758 + g_pCurrentCar->field_0x758) {
                result = 1;
                i = count;
            }
        }
    }
    return result;
}

// Counts the current car's wheels near the ground and shares its weight out.
// FUNCTION: CMR2 0x00432b30
void Car_ShareWeightOnWheels(void)
{
    int i;
    int d;

    g_pCurrentCar->field_0xb28 = 0;
    for (i = 0; i < 4; i++) {
        d = FIX_ABS(g_pCurrentCar->cornerHeight[i] - g_pCurrentCar->corners[i].y);
        if (d < 0xcccc) {
            g_pCurrentCar->cornerOnGround[i] = 1;
            g_pCurrentCar->field_0xb28++;
        } else {
            g_pCurrentCar->cornerOnGround[i] = 0;
        }
    }
    if (g_pCurrentCar->field_0xb28 == 0)
        g_pCurrentCar->tyreGrip = 0;
    else if (g_pCurrentCar->field_0xb28 == 4)
        g_pCurrentCar->tyreGrip = g_pCurrentCar->field_0x75c / 4;
    else if (g_pCurrentCar->field_0xb28 == 2)
        g_pCurrentCar->tyreGrip = g_pCurrentCar->field_0x75c / 2;
    else if (g_pCurrentCar->field_0xb28 == 1)
        g_pCurrentCar->tyreGrip = g_pCurrentCar->field_0x75c;
    else
        g_pCurrentCar->tyreGrip = g_pCurrentCar->field_0x75c / g_pCurrentCar->field_0xb28;
}

void Car_FilterWheelSpin(void);

// Updates the engine torque figure and splits it between the wheels, or
// clears the wheel torques when the car is off its wheels.
// FUNCTION: CMR2 0x00437dc0
void Car_UpdateEngineTorque(void)
{
    int value;

    if (g_pCurrentCar->field_0xb78 != 0) {
        if (g_pCurrentCar->gear != 0)
            g_pCurrentCar->field_0x780 = -g_pCurrentCar->field_0x7b0;
        else
            g_pCurrentCar->field_0x780 = 0;
    } else {
        if (g_pCurrentCar->field_0x7a4 >= g_pCurrentCar->field_0x794) {
            g_pCurrentCar->field_0x780 = 0;
        } else if (g_pCurrentCar->gear != 0 && g_pCurrentCar->field_0xb84 == 1) {
            g_pCurrentCar->field_0x780 = FixMul(g_pCurrentCar->field_0x784, FixMul(g_pCurrentCar->field_0x7a4, g_pCurrentCar->field_0x7a4));
        } else {
            value = Car_GetScaledSteerFollowRate();
            g_pCurrentCar->field_0x780 = value -
                FixMul(g_pCurrentCar->field_0x784, FixMul(g_pCurrentCar->field_0x7a4, g_pCurrentCar->field_0x7a4));
        }
    }
    if (g_pCurrentCar->gear != 0 && g_pCurrentCar->field_0xb84 == 0) {
        int torque = FixMul(g_pCurrentCar->field_0x780, g_pCurrentCar->field_0x7bc[g_pCurrentCar->gear]);
        int front = FixMul(torque, g_pCurrentCar->driveSplit) / 2;
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
    Car_FilterWheelSpin();
}

// Filters each driven wheel's spin (peak hold with decay) while it keeps the
// same surface and touches the ground.
// FUNCTION: CMR2 0x00437fd0
void Car_FilterWheelSpin(void)
{
    int i;
    int a;
    int d;

    for (i = 0; i < 4; i++) {
        if (g_pCurrentCar->field_0xabe[i] == g_pCurrentCar->wheelSurface[i] && g_pCurrentCar->cornerOnGround[i] != 0 &&
            g_pCurrentCar->field_0xb74 != 0) {
            a = g_pCurrentCar->wheelSlip[i] < 0 ? -g_pCurrentCar->wheelSlip[i] : g_pCurrentCar->wheelSlip[i];
            d = a - g_pCurrentCar->field_0x890[i];
            if (d < 0)
                g_pCurrentCar->field_0x890[i] = a;
            else
                g_pCurrentCar->field_0x890[i] += FixMul(d, g_pCurrentCar->cornerGrip[i].field_0x18);
            a = g_pCurrentCar->wheelSlipLateral[i] < 0 ? -g_pCurrentCar->wheelSlipLateral[i] : g_pCurrentCar->wheelSlipLateral[i];
            d = a - g_pCurrentCar->field_0x8a0[i];
            if (d < 0)
                g_pCurrentCar->field_0x8a0[i] = a;
            else
                g_pCurrentCar->field_0x8a0[i] += FixMul(d, g_pCurrentCar->cornerGrip[i].field_0x18);
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

void HudDash_ResetPlayerCamera(BYTE index);
int SceneNode_Destroy(SceneNode *pNode);

// Releases both players' view nodes and resets their view state (callback).
// match 62%: revisada. El original direcciona los arrays de ints con un
// desplazamiento EN BYTES sobre el sÃ­mbolo ([esi + g_viewNodes],
// [esi + g_unk0x005391b0], ...), no con Ã­ndice escalado; reproducido con
// ((BYTE *)array + off). Lo que queda es asignaciÃ³n de registros: el contador
// en AL (no EAX) y quÃ© arrays strength-reduce MSVC (g_unk0x00538e0c).
// match 62%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00421590
BYTE View_ReleaseNodes(void)
{
    BYTE i;
    int off;

    for (i = 0, off = 0; i < 2; off += 4) {
        SceneNode **pNode = (SceneNode **)((BYTE *)g_viewNodes + off);

        if (*pNode != NULL) {
            SceneNode_Destroy(*pNode);
            *pNode = NULL;
        }
        *(int *)(g_unk0x00538d2c + i * 100) = 0;
        *(int *)((BYTE *)g_unk0x00538d20 + off) = -0x10000;
        g_unk0x00538e0c[i] = 0;
        *(int *)((BYTE *)g_unk0x005391b0 + off) = 0;
        *(int *)((BYTE *)g_unk0x005391c4 + off) = 0;
        *(int *)((BYTE *)g_unk0x005391a8 + off) = 0;
        *(int *)((BYTE *)g_unk0x00538f00 + off) = 0;
        HudDash_ResetPlayerCamera(i);
        i++;
    }
    return 1;
}

// Lifts the current car out of the ground by the deepest penetration of a
// free corner.
// FUNCTION: CMR2 0x0042f8c0
void Car_LiftFreeCorners(void)
{
    int lift = 0;
    int found = 0;
    int i;
    int d;
    int *pHeight;
    int *pCorner;
    int deepest = 0;

    i = 7;
    pCorner = &g_pCurrentCar->corners[7].y;
    pHeight = &g_pCurrentCar->cornerHeight[7];
    do {
        if (g_pCurrentCar->cornerFlags[i] == 0) {
            d = *pHeight - *pCorner;
            if (d > 0 && d > deepest) {
                deepest = d;
                found = 1;
            }
        }
        i--;
        pHeight--;
        pCorner -= 3;
    } while (i >= 0);
    if (found)
        lift = deepest;
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

// Lifts the current car out of the ground by the deepest penetration of a
// free corner, rising or sinking as needed.
// FUNCTION: CMR2 0x0042f9d0
void Car_SettleFreeCorners(void)
{
    int lift = 0;
    int maxDrop = 0;
    int foundDrop = 0;
    int maxRise = -0x3e80000;
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
void Car_IntegrateWheelRotation(int *pList, short count)
{
    int local_c;
    short *local_8;
    short local_14;

    local_c = count;
    if (local_c - 1 >= 0) {
        local_8 = (short *)((int)pList + (local_c - 1) * 2);
        do {
            int carIndex = *local_8;
            int w = 0;
            int carBase = (int)g_carBuffer + carIndex * 0xc24;
            pList = (int *)(carBase + 0x860);
            do {
                int v = FixMul(*pList, g_physicsTimeStep);
                if (*(int *)(carBase + 0xb60) == 0 || *(unsigned char *)(carBase + 0x1d2) > 0 ||
                    *(unsigned char *)(carBase + 0x1d3) > 0 || *(int *)(carBase + 0x1d8) > 0) {
                    local_14 = (short)(__int64)((double)v * g_unk0x00511398);
                    ((short *)&g_unk0x0053a230[carIndex])[w] += local_14;
                }
                if (v < 0)
                    v = -v;
                if (v > 0x8000)
                    g_unk0x0053ac48[carIndex][w] = 1;
                else
                    g_unk0x0053ac48[carIndex][w] = 0;
                w++;
                pList++;
            } while (w < 4);
            local_8--;
            local_c--;
        } while (local_c != 0);
    }
}

int StageTiming_GetRelativeTyreRecordValue(char car, int wheel);
void Game_SetTriangleField2CByGroup(Mesh *pMesh, int mask, int value);

// Sets four wheel mesh states for each car in draw order.
// FUNCTION: CMR2 0x0042be30
void Car_UpdateWheelMeshStates(void)
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
            state = StageTiming_GetRelativeTyreRecordValue(car, wheel);
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
            Game_SetTriangleField2CByGroup(*(Mesh **)(pNode + 0xc), 1, appearance);
            pNode = *(BYTE **)((BYTE *)g_carBuffer + car * 0xc24 + 0x728 + wheel * 4);
            if (pNode != NULL)
                Game_SetTriangleField2CByGroup(*(Mesh **)(pNode + 0xc), 1, appearance);
        }
    }
}

// Smooths the current car's three force feedback levels.
// FUNCTION: CMR2 0x00442e90
void Car_SmoothForceFeedback(void)
{
    int i;
    int magnitude;
    int delta;

    i = 4;
    do {
        if (*(short *)((BYTE *)g_pCurrentCar + 0xabe + i * 2) == *(short *)((BYTE *)g_pCurrentCar + 0xaae + i * 2) &&
            *(int *)((BYTE *)g_pCurrentCar + 0xbac + i * 4) != 0 && g_pCurrentCar->field_0xb74 != 0) {
            magnitude = *(int *)((BYTE *)g_pCurrentCar + 0x870 + i * 4);
            if (magnitude < 0) magnitude = -magnitude;
            delta = magnitude - *(int *)((BYTE *)g_pCurrentCar + 0x890 + i * 4);
            if (delta < 0)
                *(int *)((BYTE *)g_pCurrentCar + 0x890 + i * 4) = magnitude;
            else
                *(int *)((BYTE *)g_pCurrentCar + 0x890 + i * 4) += FixMul(delta, g_pCurrentCar->cornerGrip[i].field_0x18);

            magnitude = *(int *)((BYTE *)g_pCurrentCar + 0x880 + i * 4);
            if (magnitude < 0) magnitude = -magnitude;
            delta = magnitude - *(int *)((BYTE *)g_pCurrentCar + 0x8a0 + i * 4);
            if (delta < 0)
                *(int *)((BYTE *)g_pCurrentCar + 0x8a0 + i * 4) = magnitude;
            else
                *(int *)((BYTE *)g_pCurrentCar + 0x8a0 + i * 4) += FixMul(delta, g_pCurrentCar->cornerGrip[i].field_0x18);
        } else {
            *(int *)((BYTE *)g_pCurrentCar + 0x8a0 + i * 4) = 0;
            *(int *)((BYTE *)g_pCurrentCar + 0x890 + i * 4) = 0;
        }
        i -= 2;
    } while (i >= 0);
    g_pCurrentCar->field_0x8a0[0] = g_pCurrentCar->field_0x8a0[1];
    g_pCurrentCar->field_0x890[0] = g_pCurrentCar->field_0x8a0[1];
    g_pCurrentCar->field_0x8a0[2] = g_pCurrentCar->field_0x8a0[3];
    g_pCurrentCar->field_0x890[2] = g_pCurrentCar->field_0x8a0[3];
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
    if (CGameInfo::IsActiveCheatEnabled(1)) {
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
            if (g_pCurrentCar->cornerOnGround[i] != 0) {
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
        g_pCurrentCar->field_0xc04[0] = 1;
        g_pCurrentCar->field_0x91c = FixVecDot(&g_pCurrentCar->right, &g_pCurrentCar->groundNormal);
        g_pCurrentCar->field_0x920 = FixVecDot(&g_pCurrentCar->up, &g_pCurrentCar->groundNormal);
        g_pCurrentCar->field_0x924 = FixVecDot(&g_pCurrentCar->forward, &g_pCurrentCar->groundNormal);
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
        int upDot = FixVecDot(&g_pCurrentCar->up, &g_pCurrentCar->groundNormal);
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

// --- 0x0043e680 (layer 0) ----------------------------------------------------
void StageObject_UpdateCarCornerGroundHeights(Car *pCar, int count);
int StageObject_GetCarWeatherRampValue(BYTE *pCar);
void StageObject_CopyCarSurfaceNoiseTarget(BYTE *pCar);
void Car_UpdateSurfaceParams(Car *pCar, int blend);

// The original preserves the ground normal as up and normalises only right
// and right-cross-up (forward).
// Rebuilds the body basis of the car from the ground normal and the previous
// right vector, copies it into the render node, updates the corners and
// resets the per-stage state.
// FUNCTION: CMR2 0x0043e680
void Car_ResetBodyBasis(int param_1)
{
    FixVector v;
    int dot;
    int i;

#define CARF(off) (*(int *)((int)g_pCurrentCar + (off)))
#define CARV(off) (*(FixVector *)((int)g_pCurrentCar + (off)))

    g_pCurrentCar = (Car *)param_1;
    StageObject_UpdateCarCornerGroundHeights((Car *)param_1, 8);
    Car_UpdateGroundNormal();
    g_pCurrentCar->up = g_pCurrentCar->groundNormal;
    dot = FixVecDot((FixVector *)((int)g_pCurrentCar + 0x360),
                    (FixVector *)((int)g_pCurrentCar + 0x48c));
    FixVecScale(&v, (FixVector *)((int)g_pCurrentCar + 0x48c), dot);
    v.x = g_pCurrentCar->right.x - v.x;
    v.y = g_pCurrentCar->right.y - v.y;
    v.z = g_pCurrentCar->right.z - v.z;
    {
        FixVector *pT = &g_pCurrentCar->right;
        int len = FixVecLength(&v);
        if (len == 0) {
            pT->x = 0;
            pT->y = 0;
            pT->z = 0;
        } else {
            FixVecScaleRecip(pT, &v, len);
        }
    }
    FixVecCross(&v, &g_pCurrentCar->right, &g_pCurrentCar->up);
    {
        FixVector *pT = &g_pCurrentCar->forward;
        int len = FixVecLength(&v);
        if (len == 0) {
            pT->x = 0;
            pT->y = 0;
            pT->z = 0;
        } else {
            FixVecScaleRecip(pT, &v, len);
        }
    }
    g_pCurrentCar->pWorld->right = g_pCurrentCar->right;
    g_pCurrentCar->pWorld->up = g_pCurrentCar->up;
    g_pCurrentCar->pWorld->forward = g_pCurrentCar->forward;
    Car_UpdateCorners(g_pCurrentCar);
    g_pCurrentCar->position.y += g_pCurrentCar->cornerHeight[0] - g_pCurrentCar->corners[0].y;
    g_pCurrentCar->pWorld->position = g_pCurrentCar->position;
    SceneNode_SetPosition(g_pCurrentCar->pNode0x720, (FixVector *)((int)g_pCurrentCar + 0x2d0));
    Car_UpdateCorners(g_pCurrentCar);
    Car_FlagAirborneCorners();
    g_pCurrentCar->rearWheelDir = g_pCurrentCar->right;
    Car_UpdateSurfaceParams(g_pCurrentCar, StageObject_GetCarWeatherRampValue((BYTE *)g_pCurrentCar));
    StageObject_CopyCarSurfaceNoiseTarget((BYTE *)g_pCurrentCar);
    for (i = 0; i < 4; i++) {
        g_pCurrentCar->cornerPrev[i] = g_pCurrentCar->corners[i];
        g_pCurrentCar->cornerPrev2[i] = g_pCurrentCar->cornerPrev[i];
    }
    for (i = 0; i < 8; i++)
        g_pCurrentCar->cornerNormal[i] = g_pCurrentCar->cornerAxis[i];
    g_pCurrentCar->normal0x498 = g_pCurrentCar->groundNormal;
    g_pCurrentCar->positionPrev = g_pCurrentCar->position;
    g_pCurrentCar->positionPrev2 = g_pCurrentCar->position;
    g_pCurrentCar->field_0x2dc = g_pCurrentCar->position;
    memcpy((BYTE *)((int)g_pCurrentCar + 900), (BYTE *)((int)g_pCurrentCar + 0x360), 36);
    g_pCurrentCar->field_0x960 = 0;
    g_pCurrentCar->field_0x968 = g_pCurrentCar->field_0x960;
    g_pCurrentCar->field_0x964 = g_pCurrentCar->field_0x960;
    *(BYTE *)((int)g_pCurrentCar + 0xb45) = 3;
    g_pCurrentCar->cornerOnGround[3] = 1;
    g_pCurrentCar->cornerOnGround[2] = 1;
    g_pCurrentCar->cornerOnGround[1] = 1;
    g_pCurrentCar->cornerOnGround[0] = 1;
    *(BYTE *)((int)g_pCurrentCar + 0xb28) = 4;
    g_pCurrentCar->field_0xb74 = 1;
    g_pCurrentCar->field_0xb60 = 1;

#undef CARF
#undef CARV
}

// --- 0x0042e450 (layer 0) ----------------------------------------------------
void CarPhysics_IntegrateWheelSuspension(void);

// match 47.56%: implementada; misma logica y mismos operandos de memoria, pero
// MSVC6 elige otros registros/slots en los bucles (la nuestra compila mas corta).
// Integrates the per-wheel suspension travel of the current car: the vertical
// span of each wheel is clamped against the ground and the pitch/roll rate, the
// offsets are normalised against the highest one and the wheel forces of the
// next frame are rebuilt.
// FUNCTION: CMR2 0x0042e450
void Car_IntegrateWheelTravel(void)
{
    int v[4];
    int dot;
    int sum;
    int avg;
    int max;
    int t;
    int d;
    int i;
    int a;
    int b;

#define CARF(off) (*(int *)((int)g_pCurrentCar + (off)))

    sum = 0;
    dot = FixVecDot((FixVector *)((int)g_pCurrentCar + 0x36c),
                    (FixVector *)((int)g_pCurrentCar + 0x48c));
    if (dot > 0xcccc) {
        for (i = 0; i < 4; i++) {
            t = FixMul(-FixMul(g_pCurrentCar->corners[i].y - g_pCurrentCar->cornerHeight[i], g_pCurrentCar->groundNormal.y),
                       FixDiv(0x10000, dot));
            sum += t;
            v[i] = t;
            d = g_pCurrentCar->cornerHeight[i] - g_pCurrentCar->corners[i].y - g_pCurrentCar->field_0x958;
            if (d < 0) {
                if (d < g_pCurrentCar->field_0xa08)
                    g_pCurrentCar->field_0x928[i] = g_pCurrentCar->field_0xa08;
                else
                    g_pCurrentCar->field_0x928[i] = d;
            } else {
                g_pCurrentCar->field_0x928[i] = 0;
            }
        }
        avg = sum / 4;
        max = 0;
        for (i = 0; i < 4; i++) {
            t = FixMul(g_pCurrentCar->field_0x9c0, v[i] - avg);
            g_pCurrentCar->field_0x808[i] = t;
            if (max < FIX_ABS(t))
                max = FIX_ABS(t);
        }
        if (max > 0x10000) {
            for (i = 0; i < 4; i++)
                g_pCurrentCar->field_0x808[i] = FixMul(g_pCurrentCar->field_0x808[i], FixDiv(0x10000, max));
        }
    } else {
        for (i = 0; i < 4; i++) {
            g_pCurrentCar->field_0x928[i] = 0;
            g_pCurrentCar->field_0x808[i] = 0;
        }
    }
    if (g_pCurrentCar->field_0xc00 != 0) {
        for (i = 0; i < 4; i++) {
            g_pCurrentCar->field_0x938[i] = 0;
            g_pCurrentCar->field_0x948[i] = 0;
        }
    } else {
        for (i = 0; i < 4; i++) {
            if (g_pCurrentCar->cornerGrip[i].field_0x1c < 1 || *(char *)((int)g_pCurrentCar + 0xb2c + i) != 0 ||
                g_pCurrentCar->field_0xb74 == 0) {
                g_pCurrentCar->field_0x938[i] = 0;
                g_pCurrentCar->field_0x948[i] = 0;
            } else {
                t = g_pCurrentCar->speed - FixVecDot((FixVector *)((int)g_pCurrentCar + 0x48c),
                                            (FixVector *)((int)g_pCurrentCar + 0x42c + i * 0xc));
                if (t >= 0x10001)
                    t = 0x10000;
                else if (t < 0xccc)
                    t = 0;
                if (g_pCurrentCar->field_0x948[i] >= 1) {
                    g_pCurrentCar->field_0x948[i] -= t;
                    if (g_pCurrentCar->field_0x948[i] < 0)
                        g_pCurrentCar->field_0x948[i] = 0;
                } else {
                    a = g_pCurrentCar->corners[i].x;
                    b = g_pCurrentCar->corners[i].z;
                    if (g_pCurrentCar->field_0x938[i] == 0) {
                        g_pCurrentCar->field_0x938[i] =
                            FixMul(FixMul((FIX_ABS(FIX_ABS(a) - FIX_ABS(b)) % 0x401) << 6, t),
                                   g_pCurrentCar->cornerGrip[i].field_0x1c);
                    } else {
                        g_pCurrentCar->field_0x938[i] = 0;
                        g_pCurrentCar->field_0x948[i] =
                            FixMul(FixMul((FIX_ABS(FIX_ABS(a) - FIX_ABS(b)) % 0x201) << 7, 0x320000),
                                   g_pCurrentCar->cornerGrip[i].field_0x20);
                    }
                }
            }
        }
    }
    CarPhysics_IntegrateWheelSuspension();

#undef CARF
}

// --- 0x00431ff0 / 0x0043c7f0 ------------------------------------------------

// Places the current car at a stage start: rebuilds the body frame from the
// heading vector param_2 (only x and z are used) and the position param_1,
// orthonormalises it against the ground normal the corner update produced,
// writes the resulting matrix into the car and into the scene node matrices,
// and clears the whole per-stage state.
// match 56%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// Implementada; las 13 llamadas y los offsets de campo coinciden, difiere el reparto de registros/espacios de pila (MSVC6 reutiliza ADDs donde nosotros usamos desplazamientos).
// FUNCTION: CMR2 0x00431ff0
void Car_PlaceAtStart(int *param_1, int *param_2)
{
    FixVector v;
    int l;
    int dot;
    int i;

#define CARF(off) (*(int *)((int)g_pCurrentCar + (off)))
#define CARV(off) (*(FixVector *)((int)g_pCurrentCar + (off)))

    v.x = param_2[0];
    v.y = 0;
    v.z = param_2[2];
    l = FixVecLength(&v);
    if (l != 0) {
        FixVecScaleRecip(&g_pCurrentCar->right, &v, l);
    } else {
        g_pCurrentCar->right.x = 0;
        g_pCurrentCar->right.y = 0;
        g_pCurrentCar->right.z = 0;
    }
    g_pCurrentCar->up.x = 0;
    g_pCurrentCar->up.y = 0x10000;
    g_pCurrentCar->up.z = 0;
    g_pCurrentCar->forward.x = g_pCurrentCar->right.z;
    g_pCurrentCar->forward.y = 0;
    g_pCurrentCar->forward.z = -g_pCurrentCar->right.x;
    g_pCurrentCar->position.x = param_1[0];
    g_pCurrentCar->position.y = param_1[1] - 0x640000;
    g_pCurrentCar->position.z = param_1[2];
    g_pCurrentCar->pWorld->right = g_pCurrentCar->right;
    g_pCurrentCar->pWorld->up = g_pCurrentCar->up;
    g_pCurrentCar->pWorld->forward = g_pCurrentCar->forward;
    Car_UpdateCorners(g_pCurrentCar);
    StageObject_UpdateCarCornerGroundHeights(g_pCurrentCar, 8);
    Car_UpdateGroundNormal();
    g_pCurrentCar->up = g_pCurrentCar->groundNormal;
    dot = FixVecDot(&g_pCurrentCar->up, &g_pCurrentCar->right);
    FixVecScale(&v, &g_pCurrentCar->up, dot);
    v.x = g_pCurrentCar->right.x - v.x;
    v.y = g_pCurrentCar->right.y - v.y;
    v.z = g_pCurrentCar->right.z - v.z;
    FIX_NORMALIZE_INTO(g_pCurrentCar->right, v);
    FixVecCross(&v, &g_pCurrentCar->right, &g_pCurrentCar->up);
    FIX_NORMALIZE_INTO(g_pCurrentCar->forward, v);
    g_pCurrentCar->pWorld->right = g_pCurrentCar->right;
    g_pCurrentCar->pWorld->up = g_pCurrentCar->up;
    g_pCurrentCar->pWorld->forward = g_pCurrentCar->forward;
    Car_UpdateCorners(g_pCurrentCar);
    StageObject_UpdateCarCornerGroundHeights(g_pCurrentCar, 8);
    Car_FlagAirborneCorners();
    Car_LiftFreeCorners();
    Car_FlagAirborneCorners();
    Car_ShareWeightOnWheels();
    g_pCurrentCar->pWorld->position = g_pCurrentCar->position;
    *g_pCurrentCar->pBodyMatrix = *g_pCurrentCar->pWorld;
    g_pCurrentCar->field_0x91c = 0;
    g_pCurrentCar->field_0x920 = 0x10000;
    g_pCurrentCar->field_0x924 = 0;
    g_pCurrentCar->field_0xba0[2] = 0;
    g_pCurrentCar->field_0xba0[1] = 0;
    g_pCurrentCar->field_0xba0[0] = 0;
    g_pCurrentCar->velocity.x = 0;
    g_pCurrentCar->velocity.y = 0;
    g_pCurrentCar->velocity.z = 0;
    g_pCurrentCar->velocityNext.x = 0;
    g_pCurrentCar->velocityNext.y = 0;
    g_pCurrentCar->velocityNext.z = 0;
    g_pCurrentCar->speed = 0;
    g_pCurrentCar->angularVelocity.x = 0;
    g_pCurrentCar->angularVelocity.y = 0;
    g_pCurrentCar->angularVelocity.z = 0;
    for (i = 0; i < 9; i++)
        CARF(900 + i * 4) = CARF(0x360 + i * 4);
    g_pCurrentCar->normal0x498 = g_pCurrentCar->groundNormal;
    for (i = 0x504; i < 0x564; i += 0xc)
        *(FixVector *)((int)g_pCurrentCar + i) = *(FixVector *)((int)g_pCurrentCar + i - 0x60);
    g_pCurrentCar->field_0x5c4.x = 0;
    g_pCurrentCar->field_0x5c4.y = 0;
    g_pCurrentCar->field_0x5c4.z = 0;
    g_pCurrentCar->field_0x5d0.x = 0;
    g_pCurrentCar->field_0x5d0.y = 0;
    g_pCurrentCar->field_0x5d0.z = 0;
    g_pCurrentCar->field_0x2dc = g_pCurrentCar->position;
    g_pCurrentCar->positionPrev2 = g_pCurrentCar->position;
    g_pCurrentCar->positionPrev = g_pCurrentCar->position;
    for (i = 0x330; i < 0x360; i += 0xc) {
        *(FixVector *)((int)g_pCurrentCar + i) = *(FixVector *)((int)g_pCurrentCar + i - 0xc0);
        *(FixVector *)((int)g_pCurrentCar + i - 0x30) = *(FixVector *)((int)g_pCurrentCar + i);
    }
    g_pCurrentCar->frontWheelAxis = g_pCurrentCar->forward;
    g_pCurrentCar->rearWheelDir = g_pCurrentCar->right;
    g_pCurrentCar->frontWheelDir = g_pCurrentCar->rearWheelDir;
    g_pCurrentCar->field_0xc00 = 0;
    g_pCurrentCar->field_0xc10 = 0;
    g_pCurrentCar->field_0x96c = 0;
    g_pCurrentCar->field_0xc04[0] = 0;
    *(short *)((int)g_pCurrentCar + 0xa9c) = 0;
    g_pCurrentCar->field_0x95c = 0;
    *(short *)((int)g_pCurrentCar + 0xb18) = 0;
    g_pCurrentCar->tipRatio = 0;
    *(BYTE *)((int)g_pCurrentCar + 0xb45) = 3;
    g_pCurrentCar->wheelLean.x = 0;
    g_pCurrentCar->wheelLean.y = 0;
    g_pCurrentCar->wheelLean.z = 0;
    g_pCurrentCar->lean.x = 0;
    g_pCurrentCar->lean.y = 0;
    g_pCurrentCar->lean.z = 0;
    for (i = 0x860; i < 0x870; i += 4)
        CARF(i) = 0;
    g_pCurrentCar->steerFollowRate = 0;
    g_pCurrentCar->field_0x7a0 = 0;
    g_pCurrentCar->field_0x7a4 = 0;
    g_pCurrentCar->field_0x7ac = 0;
    g_pCurrentCar->brakeInput = 0;
    g_pCurrentCar->field_0x83c = 0;
    g_pCurrentCar->handbrakeForce = 0;
    g_pCurrentCar->field_0x84c = 0;
    g_pCurrentCar->field_0x81c = 0;
    for (i = 0xa9e; i < 0xaae; i += 2)
        *(short *)((int)g_pCurrentCar + i) = 0;
    *(short *)((int)g_pCurrentCar + 0xb10) = 0;
    *(short *)((int)g_pCurrentCar + 0xb12) = 0;
    g_pCurrentCar->field_0xbfc = 0;
    g_pCurrentCar->field_0xbe4 = 0;
    g_pCurrentCar->field_0xbe0 = 0;
    g_pCurrentCar->field_0xbdc = 0;
    g_pCurrentCar->field_0xb94 = 0;
    *(BYTE *)((int)g_pCurrentCar + 0xb1e) = 1;
    g_pCurrentCar->field_0xb78 = 0;
    g_pCurrentCar->field_0x960 = 0;
    g_pCurrentCar->field_0x964 = g_pCurrentCar->field_0x960;
    g_pCurrentCar->field_0x968 = g_pCurrentCar->field_0x960;
    Car_InvalidateTransforms((int)*(char *)((int)g_pCurrentCar + 0xb1a));
    g_pCurrentCar->field_0xc04[1] = 1;
    g_pCurrentCar->field_0x970[0] = 0x320000;
    CARF(0xc14) = 1;
    l = StageObject_GetCarWeatherRampValue((BYTE *)g_pCurrentCar);
    Car_UpdateSurfaceParams(g_pCurrentCar, l);
    StageObject_CopyCarSurfaceNoiseTarget((BYTE *)g_pCurrentCar);

#undef CARF
#undef CARV
}

// Mirror of the CFrontend statics used here (espejo de Frontend.h).
class CFrontend
{
public:
    static void *GetArchivePrimaryFlagEntry(int index);
    static void *GetArchiveSecondaryFlagEntry(int index);
    static void *GetArchivePrimaryIDEntry(int index);
};

unsigned char RallyData_GetSelectionFlag28(void);
unsigned int StageTiming_GetTotalCarCount(void);
int Replay_GetSelectionStateByte(void);
BYTE RallyData_GetDriverOrCategoryFlag(BYTE param1);
char *RallyData_GetCountrySevenByteRecord(void);
char *RallyData_GetSelectedSettingSevenByteRecord(void);
BYTE *RallyData_GetDriverSkillRecord(int index);
BYTE *RallyData_GetCountrySettingSevenByteRecord(void);
BYTE StageUI_GetRaceEndEventCount(void);

// Spawns the current car at a stage/restart: picks the car model from the
// rally selection, sets up its tuning from the per-model table, its body box
// from the model dimensions and all the per-stage counters.
// match 52%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// Implementada; las 68 llamadas coinciden en simbolo y frecuencia y no hay offsets inventados; difieren el reparto de registros y algunas expresiones reasociadas por el optimizador.
// FUNCTION: CMR2 0x0043c7f0
void Car_Spawn(int param_1, int param_2, int param_3, int *param_4, int param_5,
                  int param_6)
{
    FixVector v;
    int i;
    int carType;
    int limit;
    int bVar6;
    int flag8;
    int flagC;
    int a;
    int b;
    int c;
    int hx;
    int hy;
    int hz;
    char *pcVar13;

#define CARF(off) (*(int *)((int)g_pCurrentCar + (off)))
#define CARV(off) (*(FixVector *)((int)g_pCurrentCar + (off)))
#define CARB(off) (*(char *)((int)g_pCurrentCar + (off)))

    limit = 0;
    flag8 = 0;
    flagC = 0;
    g_pCurrentCar = (Car *)param_1;
    if (RallyData_GetSelectionFlag28() != 0 && CGameInfo::GetSoundOptionBit30() != 0 &&
        param_5 == (int)StageTiming_GetTotalCarCount() - 1) {
        if (Replay_GetSelectionStateByte() == -1) {
            carType = 0;
            flag8 = 1;
        } else {
            carType = (int)CFrontend::GetArchivePrimaryIDEntry(RallyData_GetDriverRecordSelectionValue((BYTE)Replay_GetSelectionStateByte()));
            flagC = 1;
        }
    } else {
        carType = param_3;
    }
    g_pCurrentCar->index = (char)param_5;
    g_pCurrentCar->type = (char)carType;
    if (param_6 == 0)
        Car_BindModel(param_5, (Car *)param_1);
    g_pCurrentCar->field_0xa88 = FixMul(0x5999, 0x20000);
    g_pCurrentCar->field_0xa8c = g_pCurrentCar->field_0xa88 + FixMul(0x5999, 0x18000);
    for (i = 0; i < 4; i++) {
        FixMatrix_GetPosition((FixVector *)((int)g_pCurrentCar + 0x3c0 + i * 0xc),
                              (FixMatrix *)(*(int *)((int)g_pCurrentCar + 0x738 + i * 4) + 0x58));
        if (CGameInfo::IsActiveCheatEnabled(6) != 0)
            *(int *)((int)g_pCurrentCar + 0x3c4 + i * 0xc) -= g_pCurrentCar->field_0xa88;
    }
    g_pCurrentCar->position = *(FixVector *)param_4;
    g_pCurrentCar->gear = 0;
    FixMatrix *pBasis = (FixMatrix *)(param_2 + 0x98);
    g_pCurrentCar->pWorld = &g_pCurrentCar->field_0x0;
    FixMatrix_GetRight(&g_pCurrentCar->right, pBasis);
    FixMatrix_GetUp(&g_pCurrentCar->up, pBasis);
    FixMatrix_GetForward(&g_pCurrentCar->forward, pBasis);
    g_pCurrentCar->pWorld->right = g_pCurrentCar->right;
    g_pCurrentCar->pWorld->up = g_pCurrentCar->up;
    g_pCurrentCar->pWorld->forward = g_pCurrentCar->forward;
    g_pCurrentCar->pBodyMatrix = &g_pCurrentCar->field_0x40;
    FixMatrix_GetRight(&v, &g_pCurrentCar->pNode0x720->current);
    g_pCurrentCar->pWorld->right = v;
    FixMatrix_GetUp(&v, &g_pCurrentCar->pNode0x720->current);
    g_pCurrentCar->pWorld->up = v;
    FixMatrix_GetForward(&v, &g_pCurrentCar->pNode0x720->current);
    g_pCurrentCar->pWorld->forward = v;
    switch (carType) {
    case 3:
        limit = 0x3e80000;
        g_pCurrentCar->field_0x1f8.x = 0x44560;
        g_pCurrentCar->field_0x1f8.y = 0x15eb8;
        g_pCurrentCar->field_0x1f8.z = 0x1cfdf;
        g_pCurrentCar->field_0x788 = 0x570a;
        g_pCurrentCar->field_0x794 = 0x98b02;
        g_pCurrentCar->scale0x764 = 0x1cccc;
        g_pCurrentCar->scale0x768 = 0x14ccc;
        g_pCurrentCar->scale0x76c = 0x4ccc;
        g_pCurrentCar->field_0x770[1] = 0xb0a3;
        g_pCurrentCar->field_0x770[0] = 0xfa9f;
        break;
    case 0:
        limit = 0x3e80000;
        g_pCurrentCar->field_0x1f8.x = 0x426e9;
        g_pCurrentCar->field_0x1f8.y = 0x16b85;
        g_pCurrentCar->field_0x1f8.z = 0x1c51e;
        g_pCurrentCar->field_0x788 = 0x570a;
        g_pCurrentCar->field_0x794 = 0x98b02;
        g_pCurrentCar->scale0x764 = 0x1cccc;
        g_pCurrentCar->scale0x768 = 0x8000;
        g_pCurrentCar->scale0x76c = 0x3d70;
        g_pCurrentCar->field_0x770[1] = 0xcf5c;
        g_pCurrentCar->field_0x770[0] = 0x10ccc;
        break;
    case 6:
        limit = 0x3a60000;
        g_pCurrentCar->field_0x1f8.x = 0x3e3d7;
        g_pCurrentCar->field_0x1f8.y = 0x15eb8;
        g_pCurrentCar->field_0x1f8.z = 0x1c28f;
        g_pCurrentCar->field_0x788 = 0x63d7;
        g_pCurrentCar->field_0x794 = 0x98b02;
        g_pCurrentCar->scale0x764 = 0x1ae14;
        g_pCurrentCar->scale0x768 = 0x9c28;
        g_pCurrentCar->scale0x76c = 0x4ccc;
        g_pCurrentCar->field_0x770[1] = 0xcf5c;
        g_pCurrentCar->field_0x770[0] = 0xfae1;
        break;
    case 7:
        limit = 0x3c70000;
        g_pCurrentCar->field_0x1f8.x = 0x475c2;
        g_pCurrentCar->field_0x1f8.y = 0x1570a;
        g_pCurrentCar->field_0x1f8.z = 0x1c28f;
        g_pCurrentCar->field_0x788 = 0x4a3d;
        g_pCurrentCar->field_0x794 = 0x88b02;
        g_pCurrentCar->scale0x764 = 0x1e147;
        g_pCurrentCar->scale0x768 = 0x1028f;
        g_pCurrentCar->scale0x76c = 0x4ccc;
        g_pCurrentCar->field_0x770[1] = 0xcf5c;
        g_pCurrentCar->field_0x770[0] = 0xfae1;
        break;
    case 2:
        limit = 0x3e80000;
        g_pCurrentCar->field_0x1f8.x = 0x40f5c;
        g_pCurrentCar->field_0x1f8.y = 0x163d7;
        g_pCurrentCar->field_0x1f8.z = 0x1c51e;
        g_pCurrentCar->field_0x788 = 0x5687;
        g_pCurrentCar->field_0x794 = 0x98b02;
        g_pCurrentCar->scale0x764 = 0x1cccc;
        g_pCurrentCar->scale0x768 = 0x451e;
        g_pCurrentCar->scale0x76c = 0x4ccc;
        g_pCurrentCar->field_0x770[1] = 0xcf5c;
        g_pCurrentCar->field_0x770[0] = 0x1147a;
        break;
    case 1:
        limit = 0x4000000;
        g_pCurrentCar->field_0x1f8.x = 0x4451e;
        g_pCurrentCar->field_0x1f8.y = 0x15999;
        g_pCurrentCar->field_0x1f8.z = 0x1d70a;
        g_pCurrentCar->field_0x788 = 0x570a;
        g_pCurrentCar->field_0x794 = 0x98b02;
        g_pCurrentCar->scale0x764 = 0x1cccc;
        g_pCurrentCar->scale0x768 = 0xf851;
        g_pCurrentCar->scale0x76c = 0x570a;
        g_pCurrentCar->field_0x770[1] = 0xcf5c;
        g_pCurrentCar->field_0x770[0] = 0x1147a;
        break;
    case 8:
        limit = 0x1bf0000;
        g_pCurrentCar->field_0x1f8.x = 0x30083;
        g_pCurrentCar->field_0x1f8.y = 0x14041;
        g_pCurrentCar->field_0x1f8.z = 0x18000;
        g_pCurrentCar->field_0x788 = 0x3ae1;
        g_pCurrentCar->field_0x794 = 0x88b02;
        g_pCurrentCar->scale0x764 = 0x13333;
        g_pCurrentCar->scale0x768 = 0x4ccc;
        g_pCurrentCar->scale0x76c = 0x2666;
        g_pCurrentCar->field_0x770[1] = 0xb5c2;
        g_pCurrentCar->field_0x770[0] = 0xfa9f;
        break;
    case 5:
        limit = 0x3e80000;
        g_pCurrentCar->field_0x1f8.x = 0x41c28;
        g_pCurrentCar->field_0x1f8.y = 0x154bc;
        g_pCurrentCar->field_0x1f8.z = 0x1d47a;
        g_pCurrentCar->field_0x788 = 0x5c28;
        g_pCurrentCar->field_0x794 = 0x98b02;
        g_pCurrentCar->scale0x764 = 0x1c000;
        g_pCurrentCar->scale0x768 = 0x10000;
        g_pCurrentCar->scale0x76c = 0x4ccc;
        g_pCurrentCar->field_0x770[1] = 0xe3d7;
        g_pCurrentCar->field_0x770[0] = 0x12dd2;
        break;
    case 4:
        limit = 0x3e80000;
        g_pCurrentCar->field_0x1f8.x = 0x40312;
        g_pCurrentCar->field_0x1f8.y = 0x14ccc;
        g_pCurrentCar->field_0x1f8.z = 0x1c51e;
        g_pCurrentCar->field_0x788 = 0x570a;
        g_pCurrentCar->field_0x794 = 0x98b02;
        g_pCurrentCar->scale0x764 = 0x1c000;
        g_pCurrentCar->scale0x768 = 0xcccc;
        g_pCurrentCar->scale0x76c = 0x4ccc;
        g_pCurrentCar->field_0x770[1] = 0xe3d7;
        g_pCurrentCar->field_0x770[0] = 0x12dd2;
        break;
    case 9:
        limit = 0x3450000;
        g_pCurrentCar->field_0x1f8.x = 0x3b958;
        g_pCurrentCar->field_0x1f8.y = 0x15db2;
        g_pCurrentCar->field_0x1f8.z = 0x1e041;
        g_pCurrentCar->field_0x788 = 0x6666;
        g_pCurrentCar->field_0x794 = 0x98b02;
        g_pCurrentCar->scale0x764 = 0x1a666;
        g_pCurrentCar->scale0x768 = 0x9999;
        g_pCurrentCar->scale0x76c = 0x4ccc;
        g_pCurrentCar->field_0x770[1] = 0xe3d7;
        g_pCurrentCar->field_0x770[0] = 0xe106;
        break;
    case 11:
        limit = 0x2fc0000;
        g_pCurrentCar->field_0x1f8.x = 0x3d333;
        g_pCurrentCar->field_0x1f8.y = 0x15999;
        g_pCurrentCar->field_0x1f8.z = 0x1c312;
        g_pCurrentCar->field_0x788 = 0x75c2;
        g_pCurrentCar->field_0x794 = 0x98b02;
        g_pCurrentCar->scale0x764 = 0x1a666;
        g_pCurrentCar->scale0x768 = 0x9999;
        g_pCurrentCar->scale0x76c = 0x4ccc;
        g_pCurrentCar->field_0x770[1] = 0xe3d7;
        g_pCurrentCar->field_0x770[0] = 0xe106;
        break;
    case 10:
        limit = 0x2dc0000;
        g_pCurrentCar->field_0x1f8.x = 0x3b333;
        g_pCurrentCar->field_0x1f8.y = 0x106a7;
        g_pCurrentCar->field_0x1f8.z = 0x1cf5c;
        g_pCurrentCar->field_0x788 = 0x570a;
        g_pCurrentCar->field_0x794 = 0x98b02;
        g_pCurrentCar->scale0x764 = 0x1a666;
        g_pCurrentCar->scale0x768 = 0x13333;
        g_pCurrentCar->scale0x76c = 0x4ccc;
        g_pCurrentCar->field_0x770[1] = 0xca3d;
        g_pCurrentCar->field_0x770[0] = 0xe106;
        break;
    case 12:
        limit = 0x34e0000;
        g_pCurrentCar->field_0x1f8.x = 0x3ec49;
        g_pCurrentCar->field_0x1f8.y = 0x146a7;
        g_pCurrentCar->field_0x1f8.z = 0x1c28f;
        g_pCurrentCar->field_0x788 = 0x570a;
        g_pCurrentCar->field_0x794 = 0x98b02;
        g_pCurrentCar->scale0x764 = 0x1a666;
        g_pCurrentCar->scale0x768 = 0x13333;
        g_pCurrentCar->scale0x76c = 0x4ccc;
        g_pCurrentCar->field_0x770[1] = 0xca3d;
        g_pCurrentCar->field_0x770[0] = 0xe106;
        break;
    case 13:
        limit = 0x1bf0000;
        g_pCurrentCar->field_0x1f8.x = 0x41687;
        g_pCurrentCar->field_0x1f8.y = 0x16147;
        g_pCurrentCar->field_0x1f8.z = 0x1bb22;
        g_pCurrentCar->field_0x788 = 0x3ae1;
        g_pCurrentCar->field_0x794 = 0x88b02;
        g_pCurrentCar->scale0x764 = 0x1ae14;
        g_pCurrentCar->scale0x768 = 0xfd70;
        g_pCurrentCar->scale0x76c = 0x428f;
        g_pCurrentCar->field_0x770[1] = 0xd70a;
        g_pCurrentCar->field_0x770[0] = 0xf581;
        break;
    }
    bVar6 = 0;
    if (RallyData_GetSelectionFlag26() != 0 && RallyDataState() == 1 && g_pCurrentCar->index != 0) {
        switch (g_pCurrentCar->type) {
        case 6:
        case 7:
        case 9:
        case 10:
        case 0xb:
        case 0xd:
            g_pCurrentCar->field_0x788 = 0x570a;
            limit = 0x3e80000;
            bVar6 = 1;
            g_pCurrentCar->field_0x794 = 0x98b02;
        }
    }
    if (CGameInfo::IsActiveCheatEnabled(6) != 0) {
        g_pCurrentCar->field_0x1f8.y += g_pCurrentCar->field_0xa8c;
        g_pCurrentCar->field_0x770[1] += g_pCurrentCar->field_0xa8c;
        g_pCurrentCar->field_0x770[0] += g_pCurrentCar->field_0xa8c;
    }
    g_pCurrentCar->gearSpeed[0] = 0;
    g_pCurrentCar->gearSpeed[1] = FixDiv(0xc0000, 0x240000);
    g_pCurrentCar->gearSpeed[2] = FixDiv(0x100000, 0x220000);
    g_pCurrentCar->gearSpeed[3] = FixDiv(0x120000, 0x1d0000);
    g_pCurrentCar->gearSpeed[4] = FixDiv(0x130000, 0x190000);
    g_pCurrentCar->gearSpeed[5] = FixDiv(0x110000, 0x130000);
    g_pCurrentCar->gearSpeed[6] = FixDiv(0x190000, 0x180000);
    g_pCurrentCar->gearSpeed[7] = -FixDiv(0xc0000, 0x240000);
    g_pCurrentCar->field_0x788 += -0x51e;
    if (CGameInfo::IsActiveCheatEnabled(7) != 0) {
        g_pCurrentCar->field_0x788 *= 2;
        g_pCurrentCar->field_0x794 = FixMul(g_pCurrentCar->field_0x794, 0x14ccc);
    }
    limit += 0x780000;
    g_pCurrentCar->field_0x75c = limit;
    g_pCurrentCar->field_0x760 = FixDiv(0x10000, g_pCurrentCar->field_0x75c);
    g_pCurrentCar->field_0x824 = 0x1333;
    g_pCurrentCar->field_0x828 = FixDiv(0x10000, 0x30000);
    g_pCurrentCar->field_0xb7c = (int)CFrontend::GetArchiveSecondaryFlagEntry((int)g_pCurrentCar->type);
    g_pCurrentCar->field_0xb80 = (int)CFrontend::GetArchivePrimaryFlagEntry((int)g_pCurrentCar->type);
    if (flagC != 0) {
        pcVar13 = (char *)RallyData_GetDriverSkillRecord(Replay_GetSelectionStateByte());
        goto LAB_0043d703;
    }
    if (RallyData_GetFlag24() != 0) {
        pcVar13 = (char *)RallyData_GetSelectedSettingSevenByteRecord();
        goto LAB_0043d703;
    }
    if (CGameInfo::GetConfiguredGameMode() == 4 ||
        (RallyData_GetFlag25() != 0 && CGameInfo::GetGameModeOptionBit19() == 0 &&
         RallyData_GetUsableRecordCategory((BYTE)g_pCurrentCar->index) != -1) ||
        (RallyData_GetFlag25() != 0 && flag8 != 0 && CGameInfo::GetGameModeOptionBit19() == 0)) {
LAB_0043d6fe:
        pcVar13 = (char *)RallyData_GetCountrySettingSevenByteRecord();
        goto LAB_0043d703;
    }
    if (!(CGameInfo::IsConfiguredMultiplayer() == 0 || param_5 != 1 || RallyData_GetFlag25() == 0)) {
        if (flag8 != 0) {
LAB_0043d6f7:
            pcVar13 = (char *)RallyData_GetCountrySevenByteRecord();
            goto LAB_0043d703;
        }
        if (CGameInfo::GetGameModeOptionBit19() == 0)
            goto LAB_0043d6fe;
    } else {
        if (flag8 != 0)
            goto LAB_0043d6f7;
    }
    pcVar13 = (char *)RallyData_GetDriverSkillRecord((StageUI_GetRaceEndEventCount() & 0xff) + (int)g_pCurrentCar->index);
LAB_0043d703:
    Car_SetRideHeight(FixMul((int)pcVar13[2] << 0x10, 0x28f));
    if (pcVar13[4] == '2') {
        i = 0x8000;
    } else {
        i = FixMul((int)pcVar13[4] << 0x10, 0x28f);
    }
    Car_SetBrakeBias(i);
    if (g_pCurrentCar->field_0xb80 != 0) {
        if (pcVar13[3] == '2') {
            i = 0x8000;
        } else {
            i = FixMul((int)pcVar13[3] << 0x10, 0x83) + 0x6666;
        }
        if (g_pCurrentCar->type == 9)
            i += -0x2666;
        Car_SetDriveSplit(i);
    } else {
        switch (g_pCurrentCar->type) {
        case 8:
            Car_SetDriveSplit(0x10000);
            break;
        case 7:
        case 0xd:
            Car_SetDriveSplit(0);
            break;
        case 10:
            Car_SetDriveSplit(0x1999);
            break;
        default:
            Car_SetDriveSplit(0x8000);
        }
    }
    Car_SetSteeringSwingTarget(FixMul((int)pcVar13[5] << 0x10, 0x28f));
    Car_SetSurfaceDragLevel(*pcVar13);
    Car_SelectGearSpeedTable((unsigned int)pcVar13[1]);
    Car_SetDifficultyHandling(FixMul((int)pcVar13[6] << 0x10, 0x28f));
    if (bVar6)
        g_pCurrentCar->driveSplit = 0x8000;
    g_pCurrentCar->field_0x784 = 0x83;
    g_pCurrentCar->field_0x790 = 0xccc;
    g_pCurrentCar->field_0x798 = FixDiv(0x10000, g_pCurrentCar->field_0x794);
    g_pCurrentCar->field_0x834 = 0xccc;
    g_pCurrentCar->field_0x840 = 0x1999;
    g_pCurrentCar->field_0x844 = 0x8000;
    CARB(0xb1c) = 3;
    *(short *)((int)g_pCurrentCar + 0xb16) = 0x2aa;
    g_pCurrentCar->field_0x820 = 0x3333;
    g_pCurrentCar->field_0x7bc[0] = 0;
    g_pCurrentCar->field_0x7bc[1] = FixDiv(0x10000, g_pCurrentCar->gearSpeed[1]);
    g_pCurrentCar->field_0x7bc[2] = FixDiv(0x10000, g_pCurrentCar->gearSpeed[2]);
    g_pCurrentCar->field_0x7bc[3] = FixDiv(0x10000, g_pCurrentCar->gearSpeed[3]);
    g_pCurrentCar->field_0x7bc[4] = FixDiv(0x10000, g_pCurrentCar->gearSpeed[4]);
    g_pCurrentCar->field_0x7bc[5] = FixDiv(0x10000, g_pCurrentCar->gearSpeed[5]);
    g_pCurrentCar->field_0x7bc[6] = FixDiv(0x10000, g_pCurrentCar->gearSpeed[6]);
    g_pCurrentCar->field_0x7bc[7] = FixDiv(0x10000, g_pCurrentCar->gearSpeed[7]);
    g_pCurrentCar->field_0x7b8 = g_pCurrentCar->driveSplit;
    FixVecScale(&v, &g_pCurrentCar->field_0x1f8, 0x8000);
    g_pCurrentCar->field_0x758 = FixVecLength(&v);
    g_pCurrentCar->inertia.x = 1;
    g_pCurrentCar->inertia.y = 2;
    g_pCurrentCar->inertia.z = 1;
    hx = g_pCurrentCar->field_0x1f8.x / 2;
    hy = g_pCurrentCar->field_0x1f8.y / 2;
    hz = g_pCurrentCar->field_0x1f8.z / 2;
    g_pCurrentCar->halfExtents.x = hx;
    g_pCurrentCar->halfExtents.y = hy;
    g_pCurrentCar->halfExtents.z = hz;
    g_pCurrentCar->wheelPos[1].x = hx;
    g_pCurrentCar->wheelPos[1].y = -hy;
    g_pCurrentCar->wheelPos[1].z = -hz;
    g_pCurrentCar->wheelPos[0].x = hx;
    g_pCurrentCar->wheelPos[0].y = -hy;
    g_pCurrentCar->wheelPos[0].z = hz;
    g_pCurrentCar->wheelPos[2].x = -hx;
    g_pCurrentCar->wheelPos[2].y = -hy;
    g_pCurrentCar->wheelPos[2].z = hz;
    g_pCurrentCar->wheelPos[3].x = -hx;
    g_pCurrentCar->wheelPos[3].y = -hy;
    g_pCurrentCar->wheelPos[3].z = -hz;
    g_pCurrentCar->field_0x240[1].x = hx;
    g_pCurrentCar->field_0x240[1].y = hy;
    g_pCurrentCar->field_0x240[1].z = -hz;
    g_pCurrentCar->field_0x240[0].x = hx;
    g_pCurrentCar->field_0x240[0].y = hy;
    g_pCurrentCar->field_0x240[0].z = hz;
    g_pCurrentCar->field_0x240[2].x = -hx;
    g_pCurrentCar->field_0x240[2].y = hy;
    g_pCurrentCar->field_0x240[2].z = hz;
    g_pCurrentCar->field_0x240[3].x = -hx;
    g_pCurrentCar->field_0x240[3].y = hy;
    g_pCurrentCar->field_0x240[3].z = -hz;
    // Leftover of an extreme-corner search: the loop and the copies are no-ops,
    // but the original kept them (the indices only fold after the copy check).
    for (i = 0; i < 0x60; i += 0xc)
        CARF(0x210 + i) = CARF(0x210 + i);
    i = 1;
    g_pCurrentCar->wheelPos[1].x = g_pCurrentCar->wheelPos[i].x;
    g_pCurrentCar->wheelPos[0].x = CARF(0x210 + (i - 1) * 0xc);
    g_pCurrentCar->wheelPos[0].z = CARF(0x218 + (i - 1) * 0xc);
    g_pCurrentCar->wheelPos[2].z = g_pCurrentCar->wheelPos[1 + i].z;
    Car_UpdateCorners(g_pCurrentCar);
    g_gravityDir.x = 0;
    g_gravityDir.y = -0x8000;
    g_gravityDir.z = 0;
    g_pCurrentCar->field_0x91c = 0;
    g_pCurrentCar->field_0x920 = 0x10000;
    g_pCurrentCar->field_0x924 = 0;
    if (flagC != 0) {
        if (RallyData_GetDriverOrCategoryFlag((BYTE)Replay_GetSelectionStateByte()) != 0)
            g_pCurrentCar->field_0xb48 = 2;
        else
            g_pCurrentCar->field_0xb48 = 1;
    } else if (flag8 != 0) {
        g_pCurrentCar->field_0xb48 = 2;
    } else if ((int)g_pCurrentCar->index < (int)(unsigned int)RallyDataState()) {
        if (RallyData_GetDriverOrCategoryFlag((BYTE)(StageUI_GetRaceEndEventCount() + g_pCurrentCar->index)) != 0)
            g_pCurrentCar->field_0xb48 = 2;
        else
            g_pCurrentCar->field_0xb48 = 1;
    } else {
        g_pCurrentCar->field_0xb48 = 2;
    }
    g_pCurrentCar->field_0xb50 = 1;
    if (CGameInfo::GetConfiguredGameMode() == 5 || CGameInfo::GetConfiguredGameMode() == 6 ||
        CGameInfo::GetConfiguredGameMode() == 7 ||
        (CGameInfo::GetConfiguredGameMode() == 4 && g_pCurrentCar->index != 0) ||
        (RallyData_GetFlag25() != 0 && CGameInfo::GetGameModeOptionBit19() == 0 &&
         RallyData_GetUsableRecordCategory((BYTE)g_pCurrentCar->index) != -1) ||
        (RallyData_GetSelectionFlag28() != 0 && CGameInfo::GetSoundOptionBit30() != 0) ||
        CGameInfo::GetGameInfoSessionFlag() != 0 || CGameInfo::GetConfiguredGameMode() == 0xb ||
        CGameInfo::GetConfiguredGameMode() == 0xc) {
        g_pCurrentCar->field_0xb50 = 0;
    }
    if (CGameInfo::IsConfiguredMultiplayer() != 0 && param_5 == 1 && RallyData_GetFlag25() != 0)
        g_pCurrentCar->field_0xb50 = 0;
    if (CGameInfo::GetConfiguredGameMode() == 4)
        g_pCurrentCar->field_0xb50 = 0;
    g_pCurrentCar->field_0xb4c = 0;
    *(short *)((int)g_pCurrentCar + 0xafe) = 0;
    g_pCurrentCar->field_0xb98 = 1;
    g_pCurrentCar->field_0xb44 = (char)0xff;
    g_gravityScale = 0x9999;
    g_pCurrentCar->field_0x78c = g_pCurrentCar->field_0x788;
    CARF(0xa98) = 0x41c80000;
    if (CGameInfo::GetGameModeOptionBit19() != 0 && g_pCurrentCar->index > 0) {
        g_pCurrentCar->field_0xc18 = 1;
        g_pCurrentCar->field_0xc1c = 1;
    }

#undef CARF
#undef CARV
#undef CARB
}

int HudDash_GetPlayerGaugeValue(BYTE index);
void FixMatrix_RebuildBasis(FixMatrix *pOut);

// Places one car's view-camera basis for the given view slot: copies the car
// rotation, applies the suspension up-offset when the option is on and, for
// non-zero view records, rotates the basis by the tilt angle before blending.
// match 26%: reviewed (W172) - calls and constants match; the diff is register
// allocation and stack frame size (original 0xf0 vs ours 0xdc).
// match 26%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00423b20
void View_PlaceCarCamera(BYTE param_1)
{
    FixMatrix src;
    FixMatrix basis;
    FixMatrix rot;
    int scale;
    BYTE index;
    unsigned short angle;
    FixMatrix *pView;
    FixVector up;
    FixVector pos;
    int blend;

    index = param_1 & 0xff;
    FixMatrix_CopyRotationFrom(&g_unk0x00538ca0[index], Car_Get(index)->pWorld);
    pView = &g_unk0x00538ca0[index];
    if (CGameInfo::IsActiveCheatEnabled(6) != 0) {
        FixMatrix_GetUp(&up, Car_Get(index)->pWorld);
        scale = FixMul(Car_Get(index)->field_0xa8c, 0x8000);
        FixVecScale(&up, &up, scale);
        FixMatrix_GetPosition(&pos, Car_Get(index)->pWorld);
        pos.x += up.x;
        pos.y += up.y;
        pos.z += up.z;
        FixMatrix_SetPosition(&pos, pView);
    }
    if (g_unk0x00538f00[index] != 0) {
        angle = (short)(__int64)((double)FixMul(
                    g_sinTable[g_unk0x00538df8[index] & 0xfff],
                    FixDiv(HudDash_GetPlayerGaugeValue(param_1) - 0x80000, 0x100000) * 0x5a + 0x140000) *
                    g_unk0x00511300);
        FixMatrix_CopyRotationFrom(&src, Car_Get(index)->pBodyMatrix);
        basis = *pView;
        FixMatrix_RebuildBasis(&basis);
        rot.right.x = g_sinTable[(angle + 0x400) & 0xfff];
        rot.right.y = 0;
        rot.up.x = 0;
        rot.up.y = 0x10000;
        rot.right.z = -g_sinTable[angle & 0xfff];
        rot.up.z = 0;
        rot.forward.x = g_sinTable[angle & 0xfff];
        rot.forward.y = 0;
        rot.forward.z = g_sinTable[(angle + 0x400) & 0xfff];
        rot.position.x = 0;
        rot.position.y = 0;
        rot.position.z = 0;
        rot.pw = 0x10000;
        rot.rw = 0;
        rot.uw = 0;
        rot.fw = 0;
        FixMatrix_Multiply(&basis, &rot, &basis);
        blend = g_unk0x00538c98[index];
        FixMatrix_Interpolate(pView, &src, &basis, blend, blend, blend, 0);
    }
}

void Car_StepGroundContact(void);
void Car_UpdateGroundContact(void);

// One step of the car preparation for every car of the list: points the
// current car at each entry, updates its setup record and wheel travel,
// resolves the body state (4- or 8-corner mode), stores the body matrix and
// recomputes its lowest corner clearance, which becomes the sink depth of the
// step.
// FUNCTION: CMR2 0x0042cd00
void Car_PrepareBodies(int base, short *pList, short count)
{
    int i;
    int j;
    int v;

    for (i = count - 1; i >= 0; i--) {
        g_pCurrentCar = (Car *)(base + pList[i] * 0xc24);
        g_pCarSetup = (BYTE *)StageTiming_GetCarReplayRecord((int)g_pCurrentCar->index);
        Car_UpdateWheelTravel();
        g_pCurrentCar->field_0xabe[0] = g_pCurrentCar->wheelSurface[0];
        g_pCurrentCar->field_0xabe[1] = g_pCurrentCar->wheelSurface[1];
        g_pCurrentCar->field_0xabe[2] = g_pCurrentCar->wheelSurface[2];
        g_pCurrentCar->field_0xabe[3] = g_pCurrentCar->wheelSurface[3];
        if (g_pCurrentCar->field_0xc00 != 0)
            Car_UpdateGroundContact();
        else
            Car_StepGroundContact();
        Car_StoreBodyMatrix();
        Car_StartResetFade();
        if (g_pCurrentCar->field_0xa80 > FixMul(0xc0000, g_physicsScale))
            g_pCurrentCar->field_0xbf8 = 1;
        g_pCurrentCar->field_0x960 = 0x3e80000;
        for (j = 0; j < 8; j++) {
            v = g_pCurrentCar->corners[j].y - g_pCurrentCar->cornerHeight[j];
            if (v < g_pCurrentCar->field_0x960)
                g_pCurrentCar->field_0x960 = v;
        }
        g_pCurrentCar->field_0x960 += g_pCurrentCar->field_0x958;
    }
}

// --- 0x0042ce60 (layer 0) ----------------------------------------------------
// Per-step ground-contact update of the current car. Publishes the car to the
// renderer, then either integrates the change of the ground normal into the
// body torque (field_0x5d0) when the car is moving fast along the normal, or
// rebuilds the body basis from the ground normal so the car keeps standing on
// its wheels. The body matrix is finally written with the optional tip
// rotation (tipAngle) and the sink depth (field_0x958) is recomputed from the
// first four box corners.
// match 57%: implementada, MSVC6 no reproduce el reparto de registros ni el tamano de pila del original
// FUNCTION: CMR2 0x0042ce60
void Car_StepGroundContact(void)
{
    FixVector prevNormal;
    FixVector delta;
    FixVector vOut;
    FixVector tv;
    FixVector rotVec;
    FixMatrix rotMat;
    int len;
    int dot;
    int groundSpeed;
    int t;
    int i;
    char gotNormal;

    gotNormal = 0;
    g_pCurrentCar->normal0x498 = g_pCurrentCar->groundNormal;
    for (i = 0; i < 8; i++)
        g_pCurrentCar->cornerNormal[i] = g_pCurrentCar->cornerAxis[i];
    g_pCurrentCar->field_0xba0[0] = 0;
    g_pCurrentCar->field_0xba0[1] = 0;
    g_pCurrentCar->field_0xba0[2] = 0;
    StageObject_UpdateCarCornerGroundHeights(g_pCurrentCar, 4);
    if (g_pCurrentCar->field_0xb35[0] == 1)
        Car_FlagAirborneCorners();

    if (g_pCurrentCar->field_0xb35[0] == 0) {
        prevNormal = g_pCurrentCar->groundNormal;
        Car_UpdateGroundNormal();
        groundSpeed = FixVecDot(&g_pCurrentCar->groundNormal, &g_pCurrentCar->velocity);

        t = g_pCurrentCar->speed;
        if (t > 0x1570a)
            t = 0x1570a;
        t = FixMul(t, 0x8000);
        if (t > 0x10000)
            t = 0x10000;

        if (groundSpeed <= FixMul(0x10000 - t, 0x1999)) {
            // Slow: re-orthogonalise the body axes against the ground normal.
            dot = FixVecDot(&g_pCurrentCar->up, &g_pCurrentCar->groundNormal);
            if (dot < 0xb334) {
                g_pCurrentCar->field_0xc00 = 1;
                g_pCurrentCar->field_0x96c = 0x10000;
                *(BYTE *)&g_pCurrentCar->field_0xc04[0] = 1;
            } else if (g_pCurrentCar->field_0xb60 == 0 || dot < 0xfd70) {
                if (!(g_pCurrentCar->field_0xb2a == 0)) {
                    g_pCurrentCar->up = g_pCurrentCar->groundNormal;
                } else {
                    delta.x = g_pCurrentCar->groundNormal.x - g_pCurrentCar->up.x;
                    delta.y = g_pCurrentCar->groundNormal.y - g_pCurrentCar->up.y;
                    delta.z = g_pCurrentCar->groundNormal.z - g_pCurrentCar->up.z;
                    FixVecScale(&delta, &delta, 0x8000);
                    delta.x += g_pCurrentCar->up.x;
                    delta.y += g_pCurrentCar->up.y;
                    delta.z += g_pCurrentCar->up.z;
                    FIX_NORMALIZE_INTO(g_pCurrentCar->up, delta)
                }
                gotNormal = 1;
                dot = FixVecDot(&g_pCurrentCar->up, &g_pCurrentCar->right);
                FixVecScale(&delta, &g_pCurrentCar->up, dot);
                delta.x = g_pCurrentCar->right.x - delta.x;
                delta.y = g_pCurrentCar->right.y - delta.y;
                delta.z = g_pCurrentCar->right.z - delta.z;
                FIX_NORMALIZE_INTO(g_pCurrentCar->right, delta)
                FixVecCross(&delta, &g_pCurrentCar->right, &g_pCurrentCar->up);
                FIX_NORMALIZE_INTO(g_pCurrentCar->forward, delta)
            } else {
                memcpy(&g_pCurrentCar->right, g_pCurrentCar->field_0x384, 0x24);
            }
            g_pCurrentCar->field_0x91c = FixVecDot(&g_pCurrentCar->right, &g_pCurrentCar->groundNormal);
            g_pCurrentCar->field_0x920 = FixVecDot(&g_pCurrentCar->up, &g_pCurrentCar->groundNormal);
            g_pCurrentCar->field_0x924 = FixVecDot(&g_pCurrentCar->forward, &g_pCurrentCar->groundNormal);
            g_pCurrentCar->pWorld->right = g_pCurrentCar->right;
            g_pCurrentCar->pWorld->up = g_pCurrentCar->up;
            g_pCurrentCar->pWorld->forward = g_pCurrentCar->forward;
            g_pCurrentCar->pWorld->position = g_pCurrentCar->position;
            Car_UpdateCorners(g_pCurrentCar);
            if (!(g_pCurrentCar->field_0xc00 == 0))
                StageObject_UpdateCarCornerGroundHeights(g_pCurrentCar, 8);
            else
                StageObject_UpdateCarCornerGroundHeights(g_pCurrentCar, 4);
        } else {
            // Fast along the normal: turn the change of the ground normal
            // into a body torque perpendicular to it.
            FixVector worldUp = { 0, 0x10000, 0 };

            delta.x = g_pCurrentCar->groundNormal.x - prevNormal.x;
            delta.y = g_pCurrentCar->groundNormal.y - prevNormal.y;
            delta.z = g_pCurrentCar->groundNormal.z - prevNormal.z;
            len = FixVecLength(&delta);
            if (len > 0) {
                FixVecScaleRecip(&delta, &delta, len);
                FixMatrix_InverseRotateVector(&vOut, &delta, g_pCurrentCar->pWorld);
                tv.x = FixMul(FixMul(vOut.y, worldUp.z) - FixMul(vOut.z, worldUp.y), 0x14ccc);
                tv.y = 0;
                tv.z = FixMul(FixMul(vOut.x, worldUp.y) - FixMul(vOut.y, worldUp.x), 0xe666);
                g_pCurrentCar->field_0x5d0.x += tv.x;
                g_pCurrentCar->field_0x5d0.y += tv.y;
                g_pCurrentCar->field_0x5d0.z += tv.z;
            }
            g_pCurrentCar->groundNormal = prevNormal;
        }
        Car_LiftFreeCorners();
        Car_FlagAirborneCorners();
        if (g_pCurrentCar->field_0xb35[0] == 0) {
            t = FixMul(0x1eb8, g_physicsTimeStep);
            if (groundSpeed < 0) {
                FixVecScale(&delta, &g_pCurrentCar->groundNormal, groundSpeed);
                g_pCurrentCar->velocity.x -= delta.x;
                g_pCurrentCar->velocity.y -= delta.y;
                g_pCurrentCar->velocity.z -= delta.z;
            }
            g_pCurrentCar->angularVelocity.x = FixMul(g_pCurrentCar->angularVelocity.x, t);
            g_pCurrentCar->angularVelocity.z = FixMul(g_pCurrentCar->angularVelocity.z, t);
        }
    }

    Car_IntegrateWheelTravel();
    g_pCurrentCar->field_0xb2a = gotNormal;
    if (g_pCurrentCar->tipAngle != 0) {
        g_pCurrentCar->pWorld->right = g_pCurrentCar->right;
        FixMatrix_FromAxisAngle(&rotMat, &g_pCurrentCar->right, g_pCurrentCar->tipAngle);
        FixMatrix_RotateVector(&rotVec, &g_pCurrentCar->up, &rotMat);
        FIX_NORMALIZE_INTO(rotVec, rotVec)
        g_pCurrentCar->pWorld->up = rotVec;
        FixMatrix_RotateVector(&rotVec, &g_pCurrentCar->forward, &rotMat);
        FIX_NORMALIZE_INTO(rotVec, rotVec)
        g_pCurrentCar->pWorld->forward = rotVec;
    } else {
        g_pCurrentCar->pWorld->right = g_pCurrentCar->right;
        g_pCurrentCar->pWorld->up = g_pCurrentCar->up;
        g_pCurrentCar->pWorld->forward = g_pCurrentCar->forward;
    }
    g_pCurrentCar->pWorld->position = g_pCurrentCar->position;

    g_pCurrentCar->field_0x958 = 0x1999;
    for (i = 0; i < 4; i++) {
        int d = g_pCurrentCar->corners[i].y - g_pCurrentCar->cornerHeight[i];
        if (g_pCurrentCar->cornerFlags[i] == 0 && d < g_pCurrentCar->field_0x958)
            g_pCurrentCar->field_0x958 = d > 0 ? d : 0;
    }
    g_pCurrentCar->field_0x958 = -g_pCurrentCar->field_0x958;
}

// --- 0x0043f630 (layer 0) ----------------------------------------------------
void AutoGear_UpdateCarGearState(Car *pCar);
void Surface_BlendWheelContactParameters(BYTE *pWheel, int unused);
short Sector_GetNeighbours(FixVector *pPos, short *pOut);

// Race context handed to the per-car step chain: the car buffer, the car order
// (car indices) and how many cars it holds.
// GLOBAL: CMR2 0x0053cc14
int g_unk0x0053cc14;
// GLOBAL: CMR2 0x0053c9b0
short *g_unk0x0053c9b0;
// GLOBAL: CMR2 0x0053ca28
short g_unk0x0053ca28;

// One step of the race's per-car chain. Runs nine passes over the cars of the
// order array (in reverse), so every car finishes a given stage of the step
// before any car starts the next one: body-frame refresh, physics, corner
// loads, body lean, wheel forces, body axes, integration, accumulated vectors
// and finally the box-corner sector id and the previous-position copy.
// FUNCTION: CMR2 0x0043f630
void Car_RunStepPasses(int carBase, short *pOrder, short count)
{
    int i;
    int j;
    short nb[3];

    g_unk0x0053cc14 = carBase;
    g_unk0x0053c9b0 = pOrder;
    g_unk0x0053ca28 = count;

    // Pass 1: reset the step scratch and refresh the body frame.
    for (i = count - 1; i >= 0; i--) {
        g_pCurrentCar = (Car *)(carBase + pOrder[i] * 0xc24);
        g_carStepAccel.x = 0;
        g_carStepAccel.y = 0;
        g_carStepAccel.z = 0;
        g_unk0x0053c9d4 = 0;
        Car_ScaleRatesByTimeStep(g_pCurrentCar);
        g_pCurrentCar->positionPrev2 =
            g_pCurrentCar->positionPrev;
        g_pCurrentCar->positionPrev = g_pCurrentCar->position;
        memcpy((BYTE *)g_pCurrentCar + 0x384, (BYTE *)g_pCurrentCar + 0x360, 0x24);
        g_pCurrentCar->field_0x968 = g_pCurrentCar->field_0x964;
        g_pCurrentCar->field_0x964 = g_pCurrentCar->field_0x960;
        Surface_BlendWheelContactParameters((BYTE *)g_pCurrentCar, 0);
        Car_UpdateLowSpeedWheelLoadTimer();
    }

    // Pass 2: clear the per-wheel accumulators walked over by the surface code.
    for (i = count - 1; i >= 0; i--) {
        g_pCurrentCar = (Car *)(carBase + pOrder[i] * 0xc24);
        AutoGear_UpdateCarGearState(g_pCurrentCar);
        for (j = 3; j >= 0; j--) {
            g_pCurrentCar->wheelSlip[j] = 0;
            g_pCurrentCar->wheelSlipLateral[j] = 0;
        }
    }

    // Pass 3: corner loads and forces.
    for (i = count - 1; i >= 0; i--) {
        g_pCurrentCar = (Car *)(carBase + pOrder[i] * 0xc24);
        g_pCarSetup = (BYTE *)StageTiming_GetCarReplayRecord((int)g_pCurrentCar->index);
        Car_UpdateCornerVelocityPair();
        for (j = 7; j >= 0; j--) {
            g_pCurrentCar->cornerLoad[j].x = 0;
            g_pCurrentCar->cornerLoad[j].y = 0;
            g_pCurrentCar->cornerLoad[j].z = 0;
            g_pCurrentCar->cornerForce[j].x = 0;
            g_pCurrentCar->cornerForce[j].y = 0;
            g_pCurrentCar->cornerForce[j].z = 0;
        }
        if (g_pCurrentCar->field_0xb74 != 0)
            Car_SetupDriveTrain();
    }

    // Pass 4: body lean and steering.
    for (i = count - 1; i >= 0; i--) {
        g_pCurrentCar = (Car *)(carBase + pOrder[i] * 0xc24);
        g_pCarSetup = (BYTE *)StageTiming_GetCarReplayRecord((int)g_pCurrentCar->index);
        Car_UpdateBodyLean();
        if (g_pCurrentCar->field_0xb74 != 0) {
            Car_SetupSuspensionRates();
            Car_UpdateSteering();
        }
    }

    // Pass 5: wheel forces.
    for (i = count - 1; i >= 0; i--) {
        g_pCurrentCar = (Car *)(carBase + pOrder[i] * 0xc24);
        g_pCarSetup = (BYTE *)StageTiming_GetCarReplayRecord((int)g_pCurrentCar->index);
        if (g_pCurrentCar->field_0xb74 != 0) {
            Car_SplitEngineTorque();
            Car_UpdateWheelForces();
            Car_SmoothForceFeedback();
        }
    }

    // Pass 6: body axes without damping.
    for (i = count - 1; i >= 0; i--) {
        g_pCurrentCar = (Car *)(carBase + pOrder[i] * 0xc24);
        g_pCarSetup = (BYTE *)StageTiming_GetCarReplayRecord((int)g_pCurrentCar->index);
        if (g_pCurrentCar->field_0xb74 != 0)
            Car_UpdateBodyAxesNoDamping();
    }

    // Pass 7: integrate the rigid body.
    for (i = count - 1; i >= 0; i--) {
        g_pCurrentCar = (Car *)(carBase + pOrder[i] * 0xc24);
        g_pCarSetup = (BYTE *)StageTiming_GetCarReplayRecord((int)g_pCurrentCar->index);
        Car_Integrate();
    }

    // Pass 8: accumulate the body vectors and drop them once the step ends.
    for (i = count - 1; i >= 0; i--) {
        g_pCurrentCar = (Car *)(carBase + pOrder[i] * 0xc24);
        if (g_pCurrentCar->field_0xb74 != 0) {
            Car_UpdateRideHeight();
            Car_CopySteeringFollowState();
        }
        g_pCurrentCar->field_0x5c4.x = 0;
        g_pCurrentCar->field_0x5c4.y = 0;
        g_pCurrentCar->field_0x5c4.z = 0;
        g_pCurrentCar->field_0x5d0.x = 0;
        g_pCurrentCar->field_0x5d0.y = 0;
        g_pCurrentCar->field_0x5d0.z = 0;
    }

    // Pass 9: sector of the box position and previous-position copy.
    for (i = count - 1; i >= 0; i--) {
        g_pCurrentCar = (Car *)(carBase + pOrder[i] * 0xc24);
        g_pCurrentCar->sector =
            Sector_GetNeighbours(&g_pCurrentCar->position, nb);
        g_pCurrentCar->field_0xb02 = nb[0];
        g_pCurrentCar->field_0xb04 = nb[1];
        g_pCurrentCar->field_0xb06 = nb[2];
        g_pCurrentCar->field_0x2dc = g_pCurrentCar->position;
    }
}

void CarDamage_ApplyCollisionDeformImpulse(Car *pCar, int *param_2, FixVector *param_3, int param_4,
                  unsigned char param_5, int param_6);

#define CARF(off) (*(int *)((int)g_pCurrentCar + (off)))
#define CARV(off) (*(FixVector *)((int)g_pCurrentCar + (off)))

// Upright solver of the current car. Derives a damping factor from the angular
// velocity magnitude (0x50000 per unit, capped at 0xcccc) and from how far the
// forward speed sits above the roll rate, then projects the velocity on the
// ground normal (0x48c) to check the three body axes for a sign flip: the dot of
// the normal with right (0x91c), up (0x920) and forward (0x924) is stored back
// and a flip makes the body lock out the corresponding angular component. Each
// locked axis keeps a counter (0xb27 roll, 0xb25 pitch, 0xb26 yaw) that grows
// with 0xa0000 * physics step while the axis stays beyond its threshold
// (0xe666 roll, 0xf333 pitch/forward) and flips the lock flag (0xbe4, 0xbdc,
// 0xbe0) after 100 frames. Timers 0xba0/0xba4/0xba8 hold the "just spun out"
// state of each axis and clear once the two remaining angular components grow
// past 0xf5c. When an axis is unlocked and unflipped, the angular velocity is
// projected off the normal in body space and damped. The three axes are then
// re-orthonormalized (right, and up/forward rebuilt from it) and, when any axis
// flipped, the deepest corner against the normal is pushed to the deformation
// solver.
// FUNCTION: CMR2 0x0042fb20
void Car_SolveUpright(void)
{
    FixVector tmp;
    FixVector corner;
    FixVector scaled;
    FixVector upBody;
    FixVector out;
    int flipRoll;
    int flipPitch;
    int flipYaw;
    int damp;
    int mag;
    int dot;
    int i;

    flipRoll = 0;
    flipPitch = 0;
    flipYaw = 0;
    dot = FixVecLength(&g_pCurrentCar->angularVelocity);
    mag = FixMul(dot, 0x50000);
    if (mag > 0xcccc)
        mag = 0xcccc;
    dot = 0x10000 - mag;
    damp = FixMul(0x8000, dot);
    dot = FIX_ABS(FixVecDot(&g_pCurrentCar->groundNormal, &g_pCurrentCar->velocity));
    if (g_pCurrentCar->speed - dot < 0x8000)
        damp = 0x8000;
    damp = FixMul(damp, g_physicsTimeStep);
    if (damp > 0x10000)
        damp = 0x10000;

    dot = FixVecDot(&g_pCurrentCar->right, &g_pCurrentCar->groundNormal);
    if ((g_pCurrentCar->field_0x91c > 0 && dot < 0) || (g_pCurrentCar->field_0x91c < 0 && dot > 0))
        flipRoll = 1;
    g_pCurrentCar->field_0x91c = dot;
    if (g_pCurrentCar->field_0xbe4 == 0) {
        if (dot >= -0xe666 && dot <= 0xe666)
            g_pCurrentCar->field_0xb27 = 0;
        else
            g_pCurrentCar->field_0xb27 += FixMul(0xa0000, g_physicsTimeStep) >> 16;
        if (g_pCurrentCar->field_0xb27 > 100)
            g_pCurrentCar->field_0xbe4 = 1;
    }

    dot = FixVecDot(&g_pCurrentCar->up, &g_pCurrentCar->groundNormal);
    if ((g_pCurrentCar->field_0x920 > 0 && dot < 0) || (g_pCurrentCar->field_0x920 < 0 && dot > 0))
        flipPitch = 1;
    g_pCurrentCar->field_0x920 = dot;
    if (g_pCurrentCar->field_0xbdc == 0) {
        if (dot < -0xf333)
            g_pCurrentCar->field_0xb25 += FixMul(0xa0000, g_physicsTimeStep) >> 16;
        else
            g_pCurrentCar->field_0xb25 = 0;
        if (g_pCurrentCar->field_0xb25 > 100)
            g_pCurrentCar->field_0xbdc = 1;
    }

    dot = FixVecDot(&g_pCurrentCar->forward, &g_pCurrentCar->groundNormal);
    if ((g_pCurrentCar->field_0x924 > 0 && dot < 0) || (g_pCurrentCar->field_0x924 < 0 && dot > 0))
        flipYaw = 1;
    g_pCurrentCar->field_0x924 = dot;
    if (g_pCurrentCar->field_0xbe0 == 0) {
        if (dot >= -0xf333 && dot <= 0xf333)
            g_pCurrentCar->field_0xb26 = 0;
        else
            g_pCurrentCar->field_0xb26 += FixMul(0xa0000, g_physicsTimeStep) >> 16;
        if (g_pCurrentCar->field_0xb26 > 100)
            g_pCurrentCar->field_0xbe0 = 1;
    }

    if (g_pCurrentCar->field_0xba0[0] != 0) {
        if (FixSqrt(FixMul(g_pCurrentCar->angularVelocity.y, g_pCurrentCar->angularVelocity.y) +
                    FixMul(g_pCurrentCar->angularVelocity.z, g_pCurrentCar->angularVelocity.z)) >= 0xf5c)
            g_pCurrentCar->field_0xba0[0] = 0;
    }
    if (g_pCurrentCar->field_0xba0[1] != 0) {
        if (FixSqrt(FixMul(g_pCurrentCar->angularVelocity.x, g_pCurrentCar->angularVelocity.x) +
                    FixMul(g_pCurrentCar->angularVelocity.z, g_pCurrentCar->angularVelocity.z)) >= 0xf5c)
            g_pCurrentCar->field_0xba0[1] = 0;
    }
    if (g_pCurrentCar->field_0xba0[2] != 0) {
        if (FixSqrt(FixMul(g_pCurrentCar->angularVelocity.x, g_pCurrentCar->angularVelocity.x) +
                    FixMul(g_pCurrentCar->angularVelocity.y, g_pCurrentCar->angularVelocity.y)) >= 0xf5c)
            g_pCurrentCar->field_0xba0[2] = 0;
    }

    if (flipRoll != 0 && g_pCurrentCar->field_0xba0[0] == 0) {
        FixMatrix_InverseRotateVector(&tmp, &g_pCurrentCar->groundNormal, g_pCurrentCar->pWorld);
        dot = FixVecDot(&tmp, &g_pCurrentCar->angularVelocity);
        FixVecScale(&tmp, &tmp, dot);
        tmp.x -= g_pCurrentCar->angularVelocity.x;
        tmp.y -= g_pCurrentCar->angularVelocity.y;
        tmp.z -= g_pCurrentCar->angularVelocity.z;
        FixVecScale(&tmp, &tmp, damp);
        tmp.x = 0;
        g_pCurrentCar->angularVelocity.y += tmp.y;
        g_pCurrentCar->angularVelocity.z += tmp.z;
        if (dot < 0)
            dot = -dot;
        if (dot < 0xf5c && g_pCurrentCar->speed < 0x8000)
            g_pCurrentCar->field_0xba0[0] = 1;
    }
    if (flipPitch != 0 && g_pCurrentCar->field_0xba0[1] == 0) {
        FixMatrix_InverseRotateVector(&tmp, &g_pCurrentCar->groundNormal, g_pCurrentCar->pWorld);
        dot = FixVecDot(&tmp, &g_pCurrentCar->angularVelocity);
        FixVecScale(&tmp, &tmp, dot);
        tmp.x -= g_pCurrentCar->angularVelocity.x;
        tmp.y -= g_pCurrentCar->angularVelocity.y;
        tmp.z -= g_pCurrentCar->angularVelocity.z;
        FixVecScale(&tmp, &tmp, damp);
        tmp.y = 0;
        g_pCurrentCar->angularVelocity.x += tmp.x;
        g_pCurrentCar->angularVelocity.y += tmp.y;
        g_pCurrentCar->angularVelocity.z += tmp.z;
    }
    if (flipYaw != 0 && g_pCurrentCar->field_0xba0[2] == 0) {
        FixMatrix_InverseRotateVector(&tmp, &g_pCurrentCar->groundNormal, g_pCurrentCar->pWorld);
        dot = FixVecDot(&tmp, &g_pCurrentCar->angularVelocity);
        FixVecScale(&tmp, &tmp, dot);
        tmp.x -= g_pCurrentCar->angularVelocity.x;
        tmp.y -= g_pCurrentCar->angularVelocity.y;
        tmp.z -= g_pCurrentCar->angularVelocity.z;
        FixVecScale(&tmp, &tmp, damp);
        tmp.z = 0;
        g_pCurrentCar->angularVelocity.x += tmp.x;
        g_pCurrentCar->angularVelocity.y += tmp.y;
        g_pCurrentCar->angularVelocity.z += tmp.z;
        if (dot < 0)
            dot = -dot;
        if (dot < 0xf5c && g_pCurrentCar->speed < 0x8000)
            g_pCurrentCar->field_0xba0[2] = 1;
    }

    if (g_pCurrentCar->field_0xba0[0] != 0 || g_pCurrentCar->field_0xba0[1] != 0 || g_pCurrentCar->field_0xba0[2] != 0) {
        if (g_pCurrentCar->field_0xba0[0] != 0) {
            FixVecScale(&tmp, &g_pCurrentCar->groundNormal, g_pCurrentCar->field_0x91c);
            tmp.x = g_pCurrentCar->right.x - tmp.x;
            tmp.y = g_pCurrentCar->right.y - tmp.y;
            tmp.z = g_pCurrentCar->right.z - tmp.z;
            FIX_NORMALIZE_INTO(tmp, tmp);
            g_pCurrentCar->right = tmp;
            g_pCurrentCar->angularVelocity.y = FixMul(g_pCurrentCar->angularVelocity.y, 0xe666);
            g_pCurrentCar->angularVelocity.z = FixMul(g_pCurrentCar->angularVelocity.z, 0xe666);
        }
        if (g_pCurrentCar->field_0xba0[1] != 0) {
            FixVecScale(&tmp, &g_pCurrentCar->groundNormal, g_pCurrentCar->field_0x920);
            tmp.x = g_pCurrentCar->up.x - tmp.x;
            tmp.y = g_pCurrentCar->up.y - tmp.y;
            tmp.z = g_pCurrentCar->up.z - tmp.z;
            FIX_NORMALIZE_INTO(tmp, tmp);
            g_pCurrentCar->up = tmp;
            g_pCurrentCar->angularVelocity.x = FixMul(g_pCurrentCar->angularVelocity.x, 0xe666);
            g_pCurrentCar->angularVelocity.z = FixMul(g_pCurrentCar->angularVelocity.z, 0xe666);
        }
        if (g_pCurrentCar->field_0xba0[2] != 0) {
            FixVecScale(&tmp, &g_pCurrentCar->groundNormal, g_pCurrentCar->field_0x924);
            tmp.x = g_pCurrentCar->forward.x - tmp.x;
            tmp.y = g_pCurrentCar->forward.y - tmp.y;
            tmp.z = g_pCurrentCar->forward.z - tmp.z;
            FIX_NORMALIZE_INTO(tmp, tmp);
            g_pCurrentCar->forward = tmp;
            g_pCurrentCar->angularVelocity.x = FixMul(g_pCurrentCar->angularVelocity.x, 0xe666);
            g_pCurrentCar->angularVelocity.y = FixMul(g_pCurrentCar->angularVelocity.y, 0xe666);
        }
        dot = FixVecDot(&g_pCurrentCar->right, &g_pCurrentCar->up);
        FixVecScale(&tmp, &g_pCurrentCar->right, dot);
        tmp.x = g_pCurrentCar->up.x - tmp.x;
        tmp.y = g_pCurrentCar->up.y - tmp.y;
        tmp.z = g_pCurrentCar->up.z - tmp.z;
        FixVecNormalizeLen(&g_pCurrentCar->up, &tmp);
        FixVecCross(&tmp, &g_pCurrentCar->right, &g_pCurrentCar->up);
        FixVecNormalizeLen(&g_pCurrentCar->forward, &tmp);
    }

    if (flipRoll != 0 || flipPitch != 0 || flipYaw != 0) {
        for (i = 0; i < 8; i++) {
            dot = FixVecDot(&g_pCurrentCar->cornerVelocity[i], &g_pCurrentCar->groundNormal);
            if (dot < -0x3333)
                goto found;
        }
        return;
found:
        g_pCurrentCar->field_0x5c4.x = dot;
        g_pCurrentCar->field_0x5c4.y = 0;
        g_pCurrentCar->field_0x5c4.z = 0;
        FixVecScale(&tmp, &g_pCurrentCar->groundNormal, -0x30000);
        FixMatrix_InverseRotateVector(&g_pCurrentCar->field_0x5dc, &tmp, g_pCurrentCar->pWorld);
        i = g_pCurrentCar->field_0x5dc.x;
        if (FIX_ABS(i) > g_pCurrentCar->halfExtents.x)
            g_pCurrentCar->field_0x5dc.x = i < 0 ? -g_pCurrentCar->halfExtents.x : g_pCurrentCar->halfExtents.x;
        i = g_pCurrentCar->field_0x5dc.y;
        if (FIX_ABS(i) > g_pCurrentCar->halfExtents.y)
            g_pCurrentCar->field_0x5dc.y = i < 0 ? -g_pCurrentCar->halfExtents.y : g_pCurrentCar->halfExtents.y;
        i = g_pCurrentCar->field_0x5dc.z;
        if (FIX_ABS(i) > g_pCurrentCar->halfExtents.z)
            g_pCurrentCar->field_0x5dc.z = i < 0 ? -g_pCurrentCar->halfExtents.z : g_pCurrentCar->halfExtents.z;
        corner.x = 0;
        corner.y = 0x7d000000;
        corner.z = 0;
        upBody.x = 0;
        upBody.y = 0x10000;
        upBody.z = 0;
        for (i = 0; i < 4; i++) {
            if (g_pCurrentCar->corners[i].y < corner.y)
                corner = g_pCurrentCar->corners[i];
        }
        if (g_pCurrentCar->up.y < 0) {
            FixVecScale(&scaled, &g_pCurrentCar->up, FixMul(0x20000, g_pCurrentCar->halfExtents.y));
            corner.x += scaled.x;
            corner.y += scaled.y;
            corner.z += scaled.z;
        }
        CarDamage_ApplyCollisionDeformImpulse(g_pCurrentCar, (int *)&corner, &upBody, 0, 0, 0);
        FixMatrix_RotateVector(&out, &g_pCurrentCar->field_0x5dc, g_pCurrentCar->pWorld);
    }
}

#undef CARF
#undef CARV

// --- per-view camera control (0x421610-0x423780) ---------------------------
// Each view has two camera records in g_viewRecords (index view * 2 + the
// active slot g_unk0x00538e0c[view]): +0 record index, +1 view, +2 car,
// +4 camera type, +8 matrix, +0x48.. parameters, +0x60 ground clearance.
// g_unk0x00538d2c - 4 holds the blended state of each view in the same layout.

#define VIEW_RECORD(i) (&g_viewRecords[i])
#define VIEW_STATE(v) ((CameraRecord *)(g_unk0x00538d2c - 4) + (v))
#define VIEW_PREV_STATE(v) ((CameraRecord *)g_unk0x00538e38 + (v))
// Smooth-step weight of a camera blend timer (0xc8000 = the whole transition).
#define VIEW_EASE(t)                                                                               \
    (0x10000 - (FixCos((short)(int)(__int64)((double)(FixDiv((t), 0xc8000) * 180) * g_unk0x00511308)) + \
                0x10000) / 2)

extern double g_unk0x00511308;
extern double g_unk0x00511300;
struct Unk004238e0;
void Game_DispatchObjectTypeEvent(BYTE *pObject, BYTE *pInfo);
void Game_DispatchObjectTypeUpdate(BYTE *pObject, BYTE *pInfo);
void Game_UpdateType3ObjectState(Unk004238e0 *param1, int param2);
void Game_DispatchObjectContactReset(BYTE *pObject, BYTE *pInfo);
void RallyData_ValidateType3Entry(int *p);
void Glow_NoOpEntryCallback(BYTE a, BYTE b, int c, int d);
void StageObject_SelectAndCopyCarNodePayload(BYTE *pObj, int *pSrc, BYTE index, BYTE value);
void StageObject_SyncStateAndSceneMatrix(BYTE *p, int *src, int unused, BYTE value);
void Dash_UpdateCameraModeOffset(BYTE *param_1, BYTE *param_2, int param_3);
void View_PlaceTracksideCameraAtSpot(BYTE *pRecord, FixMatrix *pRef, int param);
void StageObject_RebuildMirroredTiltMatrix(BYTE *pObj, int *pSrc);
void StageObject_BuildCarNodeOrientation(int object, int *src);
void View_CacheReferenceBasisAndBuildMatrix(BYTE *pObj, FixMatrix *pRef);
void View_RestartTracksideCameraDolly(BYTE *pRecord, FixMatrix *pRef);
void StageObject_SetPositionFromSplitVector(BYTE *pObj, FixMatrix *pRef);
void StageObject_BuildCarMountWorldMatrix(BYTE *object, int unused);
void View_BuildMatrixFromCameraBasis(BYTE *pObj, FixMatrix *pRef);
void View_BuildTracksideCameraMatrix(BYTE *pRecord, FixMatrix *pRef);
void StageObject_InterpolateReferenceMatrix(BYTE *pObj, int *pSrc, int param_3);
void StageObject_DispatchActiveCarObjectUpdate(BYTE *pObj, int a, int b);
void Dash_BuildInterpolatedCockpitMatrix(BYTE *param_1, FixMatrix *param_2, int param_3);
void View_UpdateTracksideZoomAndShake(BYTE *pRecord, FixMatrix *pRef);
int Track_GetGroundHeight5(FixVector *pPoint, FixVector *pNormal, short *pTri, short *pSurfaceClass, int defaultY);
void StageObject_LoadAndAttachCarInterior(BYTE record, BYTE car);
unsigned short RallyData_GetPrimaryStageScoreScale(void);
unsigned short RallyData_GetSecondaryStageScoreScale(void);
void StageObject_InitCarSceneTables(void);
void StageObject_CacheCarSplitVectorPointers(void);
void StageTiming_SetViewRouteDistanceLimit(BYTE player, unsigned int node, int dir);
int RallyData_GetChallengeRenderState(void);
void View_PlaceCarCamera(BYTE param_1);
int View_IsModeAvailable(BYTE index, int mode);
FixMatrix *View_GetCarBodyMatrix(FixMatrix *pOut, BYTE car);
void FixMatrix_GetPosition(FixVector *pOut, FixMatrix *pM);

// Starts the camera of camera type `type` on a view record.
// FUNCTION: CMR2 0x00423300
void Camera_Start(CameraRecord *pRecord, int type, int param)
{
    BYTE car;
    BYTE view;
    Car *pCar;
    FixMatrix body;

    view = pRecord->view;
    car = pRecord->car;
    pCar = Car_Get(car);
    View_GetCarBodyMatrix(&body, car);
    switch (type) {
    case 6:
        Dash_UpdateCameraModeOffset((BYTE *)pRecord, (BYTE *)Car_GetCameraReferenceMatrix(car), 3);
        return;
    case 4:
        Dash_UpdateCameraModeOffset((BYTE *)pRecord, (BYTE *)Car_GetCameraReferenceMatrix(car), 0);
        return;
    case 5:
        Dash_UpdateCameraModeOffset((BYTE *)pRecord, (BYTE *)Car_GetCameraReferenceMatrix(car), 4);
        return;
    case 3:
        StageObject_SyncStateAndSceneMatrix((BYTE *)pRecord, (int *)Car_GetCameraReferenceMatrix(car), View_GetActiveCameraMode(view), (BYTE)pCar->type);
        return;
    case 2:
        StageObject_SelectAndCopyCarNodePayload((BYTE *)pRecord, (int *)&body, *(BYTE *)&pCar->index, 0);
        return;
    case 10:
        StageObject_SelectAndCopyCarNodePayload((BYTE *)pRecord, (int *)&body, *(BYTE *)&pCar->index, 2);
        return;
    case 1:
        StageObject_SelectAndCopyCarNodePayload((BYTE *)pRecord, (int *)&body, *(BYTE *)&pCar->index, 1);
        return;
    case 7:
        View_PlaceTracksideCameraAtSpot((BYTE *)pRecord, Car_GetCameraReferenceMatrix(car), param);
    }
}

// Per-frame update of a view record's camera, plus its ground clearance
// (how far the camera sits above the stage, eased towards the new value).
// match 48%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00423460
void Camera_Update(CameraRecord *pRecord)
{
    FixMatrix body;
    FixVector position;
    FixVector normal;
    Car *pCar;
    BYTE car;
    int onCar;
    int clearance;
    int check;
    short tri;
    short surface;

    onCar = 0;
    if (pRecord->field_0x48 != 0 || pRecord->field_0x5c != 0)
        check = 1;
    else
        check = 0;
    car = pRecord->car;
    pCar = Car_Get(car);
    if (*(int *)((BYTE *)pCar + 0xc04) != 0 && g_unk0x00538f00[pRecord->car] == 0)
        onCar = 1;
    View_GetCarBodyMatrix(&body, car);
    switch (pRecord->type) {
    case 4:
    case 5:
    case 6:
        Dash_BuildInterpolatedCockpitMatrix((BYTE *)pRecord, Car_GetCameraReferenceMatrix(car), onCar);
        break;
    case 3:
        if (onCar == 0 && pCar->field_0xb60 == 0)
            StageObject_DispatchActiveCarObjectUpdate((BYTE *)pRecord, (int)Car_GetCameraReferenceMatrix(car), 0);
        else
            StageObject_DispatchActiveCarObjectUpdate((BYTE *)pRecord, (int)Car_GetCameraReferenceMatrix(car), 1);
        break;
    case 2:
        StageObject_InterpolateReferenceMatrix((BYTE *)pRecord, (int *)&body, (onCar == 0 && pCar->field_0xb60 == 0) ? 0 : 1);
        break;
    case 10:
        StageObject_InterpolateReferenceMatrix((BYTE *)pRecord, (int *)&body, (onCar == 0 && pCar->field_0xb60 == 0) ? 0 : 1);
        break;
    case 1:
        StageObject_InterpolateReferenceMatrix((BYTE *)pRecord, (int *)&body, (onCar == 0 && pCar->field_0xb60 == 0) ? 0 : 1);
        break;
    case 7:
        View_UpdateTracksideZoomAndShake((BYTE *)pRecord, Car_GetCameraReferenceMatrix(car));
        break;
    }
    if (View_IsPlaybackCameraActive() == 0 && check != 0) {
        FixMatrix_GetPosition(&position, &pRecord->matrix);
        tri = 0;
        clearance = Track_GetGroundHeight5(&position, &normal, &tri, &surface,
                                           *(int *)((BYTE *)Car_Get(pRecord->car) + 0x8e4)) -
                    position.y + 0x10000;
        if (clearance < 0)
            clearance = 0;
        if (clearance < pRecord->clearance)
            clearance += FixMul(pRecord->clearance - clearance, 0x10000 - FixMul(0xccc, g_physicsTimeStep));
        pRecord->clearance = clearance;
        return;
    }
    pRecord->clearance = 0;
}

// Restarts the camera of a view record (after a car reset).
// FUNCTION: CMR2 0x004236b0
void Camera_Restart(CameraRecord *pRecord)
{
    FixMatrix body;
    BYTE car;

    car = pRecord->car;
    View_GetCarBodyMatrix(&body, car);
    switch (pRecord->type) {
    case 4:
    case 5:
    case 6:
        View_CacheReferenceBasisAndBuildMatrix((BYTE *)pRecord, Car_GetCameraReferenceMatrix(car));
        return;
    case 3:
        StageObject_BuildCarNodeOrientation((int)pRecord, (int *)Car_GetCameraReferenceMatrix(car));
        return;
    case 2:
        StageObject_RebuildMirroredTiltMatrix((BYTE *)pRecord, (int *)&body);
        return;
    case 10:
        StageObject_RebuildMirroredTiltMatrix((BYTE *)pRecord, (int *)&body);
        return;
    case 1:
        StageObject_RebuildMirroredTiltMatrix((BYTE *)pRecord, (int *)&body);
        return;
    case 7:
        View_RestartTracksideCameraDolly((BYTE *)pRecord, Car_GetCameraReferenceMatrix(car));
    }
}

// Snaps the camera of a view record to a reference matrix.
// FUNCTION: CMR2 0x00423780
void Camera_SnapTo(CameraRecord *pRecord, FixMatrix *pRef)
{
    switch (pRecord->type) {
    case 5:
    case 6:
        View_BuildMatrixFromCameraBasis((BYTE *)pRecord, pRef);
        return;
    case 4:
        View_BuildMatrixFromCameraBasis((BYTE *)pRecord, pRef);
        return;
    case 3:
        StageObject_BuildCarMountWorldMatrix((BYTE *)pRecord, (int)pRef);
        return;
    case 10:
        StageObject_SetPositionFromSplitVector((BYTE *)pRecord, pRef);
        return;
    case 1:
    case 2:
        StageObject_SetPositionFromSplitVector((BYTE *)pRecord, pRef);
        return;
    case 7:
        View_BuildTracksideCameraMatrix((BYTE *)pRecord, pRef);
    }
}

// Sets up the view cameras of a stage: the four camera records, then for both
// views a camera scene node and a cleared view state.
// FUNCTION: CMR2 0x00421610
void View_SetupCameras(void)
{
    FixVector position;
    FixAngles angles;
    BYTE i;

    if ((unsigned short)RallyData_GetPrimaryStageScoreScale() < 0x834)
        RallyData_GetSecondaryStageScoreScale();
    CGame::RegisterCallback(View_ReleaseNodes, NULL);
    StageObject_InitCarSceneTables();
    StageObject_CacheCarSplitVectorPointers();
    for (i = 0; i < 4; i++) {
        g_viewRecords[i].index = i;
        g_viewRecords[i].view = i >> 1;
        g_viewRecords[i].car = i >> 1;
        g_viewRecords[i].clearance = 0;
        StageObject_LoadAndAttachCarInterior(i, g_viewRecords[i].car);
    }
    position.x = 0;
    position.y = 0;
    position.z = 0;
    angles.x = 0;
    angles.y = 0;
    angles.z = 0;
    i = 0;
    for (i = 0; i < 2; i++) {
        g_viewNodes[i] = SceneType2_Create(&position, &angles, NULL, (SceneNode *)RallyData_GetChallengeRenderState());
        *(int *)(g_unk0x00538d2c + i * 100) = 0;
        g_unk0x00538d20[i] = -0x10000;
        g_unk0x00538e0c[i] = 0;
        g_unk0x005391b0[i] = 0;
        g_unk0x005391c4[i] = 0;
        g_unk0x005391a8[i] = 0;
        StageTiming_SetViewRouteDistanceLimit(0, 0, 1);
        g_unk0x00538f00[i] = 0;
        HudDash_ResetPlayerCamera(i);
    }
}

// Switches a view to camera type `type`: the inactive record takes the new
// camera, and either the view blends into it (timed transition) or swaps to
// it at once.
// FUNCTION: CMR2 0x00421720
void View_SwitchCamera(BYTE view, int type, int param, BYTE target, int blend)
{
    int ok;
    BYTE active;
    BYTE next;

    if (View_GetActiveCameraMode(view) != 8 && View_IsModeAvailable(view, type) != 0)
        ok = 1;
    else
        ok = 0;
    View_PlaceCarCamera(view);
    if (ok == 0)
        return;
    active = g_unk0x00538e0c[view] + view * 2;
    next = view * 2 - g_unk0x00538e0c[view] + 1;
    if (blend == 0 || type == 3 || VIEW_RECORD(active)->type == 3)
        blend = 0;
    else
        blend = 1;
    if (type == 7 && param == 0xffff)
        param = StageObject_FindNearestCameraSpot((FixVector *)((BYTE *)Car_Get(view) + 0x2d0));
    VIEW_RECORD(next)->type = type;
    if (VIEW_RECORD(next)->car != target) {
        Glow_NoOpEntryCallback(VIEW_RECORD(next)->car, VIEW_RECORD(next)->view, 1, 1);
        Glow_NoOpEntryCallback(target, VIEW_RECORD(next)->view, 1, 1);
    }
    VIEW_RECORD(next)->car = target;
    Game_DispatchObjectTypeEvent((BYTE *)VIEW_RECORD(active), (BYTE *)VIEW_RECORD(next));
    Game_DispatchObjectTypeUpdate((BYTE *)VIEW_RECORD(next), (BYTE *)VIEW_RECORD(active));
    Camera_Start(VIEW_RECORD(next), type, param);
    VIEW_RECORD(next)->clearance = 0;
    if (blend) {
        g_unk0x00538d20[view] = 0xc8000;
        return;
    }
    Game_UpdateType3ObjectState((Unk004238e0 *)VIEW_RECORD(active), (int)VIEW_RECORD(next));
    Game_DispatchObjectContactReset((BYTE *)VIEW_RECORD(next), (BYTE *)VIEW_RECORD(active));
    RallyData_ValidateType3Entry((int *)VIEW_RECORD(active));
    g_unk0x005391b0[view] = 1;
    g_unk0x00538e0c[view] = 1 - g_unk0x00538e0c[view];
}

// Per-frame camera of a view: the shake animation, both records' cameras and
// the blend between them while a transition runs.
// match 56%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004219b0
void View_UpdateCamera(BYTE view)
{
    BYTE index;
    BYTE active;
    BYTE next;
    int *pTimer;
    CameraRecord *pActive;
    CameraRecord *pNext;
    int typeA;
    int typeB;
    int activeFree;
    int activeTracked;
    int nextFree;
    int nextTracked;
    int t;

    index = (BYTE)view;
    active = g_unk0x00538e0c[index] + (BYTE)view * 2;
    next = (BYTE)view * 2 - g_unk0x00538e0c[index] + 1;
    if (g_unk0x00538f00[index] != 0) {
        if (g_unk0x00538f00[index] == 1) {
            g_unk0x00538c98[index] += 0xccc;
            if (g_unk0x00538c98[index] >= 0x10000) {
                g_unk0x00538f00[index] = 2;
                g_unk0x00538c98[index] = 0x10000;
            }
        } else if (g_unk0x00538f00[index] == 3) {
            g_unk0x00538c98[index] += -0xccc;
            if (g_unk0x00538c98[index] <= 0) {
                g_unk0x00538f00[index] = 0;
                g_unk0x00538c98[index] = 0;
            }
        }
        g_unk0x00538df8[index] += 0x44;
        if (g_unk0x00538df8[index] >= 0x1000)
            g_unk0x00538df8[index] -= 0x1000;
    }
    View_PlaceCarCamera(view);
    pTimer = &g_unk0x00538d20[index];
    if (g_unk0x00538d20[index] >= 0)
        CameraState_Copy(VIEW_PREV_STATE(index), VIEW_STATE(index));
    else
        CameraState_Copy(VIEW_PREV_STATE(index), VIEW_RECORD(active));
    pActive = VIEW_RECORD(active);
    Camera_Update(pActive);
    g_unk0x005391c4[index] = g_unk0x005391b0[index];
    g_unk0x005391b0[index] = 0;
    if (*pTimer < 0) {
        CameraState_Copy(VIEW_STATE(index), pActive);
        return;
    }
    pNext = VIEW_RECORD(next);
    Camera_Update(pNext);
    if (*pTimer - g_physicsTimeStep > 0) {
        typeA = pActive->type;
        if (typeA == 4 || typeA == 5 || typeA == 10)
            activeFree = 1;
        else
            activeFree = 0;
        if (typeA == 1 || typeA == 2)
            activeTracked = 1;
        else
            activeTracked = 0;
        typeB = pNext->type;
        if (typeB == 4 || typeB == 5 || typeB == 10)
            nextFree = 1;
        else
            nextFree = 0;
        if (typeB == 1 || typeB == 2)
            nextTracked = 1;
        else
            nextTracked = 0;
        t = 0x10000 - FixMul(VIEW_EASE(*pTimer), VIEW_EASE(*pTimer));
        CameraState_Interpolate(VIEW_STATE(index), pActive, pNext, t);
        if ((activeFree && nextTracked) || (activeTracked && nextFree))
            VIEW_STATE(index)->matrix.position.y +=
                FixMul(g_sinTable[(short)(int)(__int64)((double)(t * 180) * g_unk0x00511300) & 0xfff], 0x10000);
        *(int *)(g_unk0x00538d2c + index * 100) = 8;
        *pTimer -= g_physicsTimeStep;
        return;
    }
    Game_UpdateType3ObjectState((Unk004238e0 *)pActive, (int)pNext);
    Game_DispatchObjectContactReset((BYTE *)pNext, (BYTE *)pActive);
    RallyData_ValidateType3Entry((int *)pActive);
    CameraState_Copy(VIEW_STATE(index), pNext);
    g_unk0x00538e0c[index] = 1 - g_unk0x00538e0c[index];
    *pTimer -= g_physicsTimeStep;
}

// Resets the cameras of a view to their records (after a restart).
// FUNCTION: CMR2 0x00421d80
void View_ResetCameras(int view)
{
    BYTE active;
    BYTE next;

    active = g_unk0x00538e0c[(BYTE)view] + (BYTE)view * 2;
    next = (BYTE)view * 2 - g_unk0x00538e0c[(BYTE)view] + 1;
    View_PlaceCarCamera(view);
    Camera_Restart(VIEW_RECORD(active));
    VIEW_RECORD(active)->clearance = 0;
    if (View_GetActiveCameraMode(view) == 8) {
        Camera_Restart(VIEW_RECORD(next));
        VIEW_RECORD(next)->clearance = 0;
    }
    g_unk0x005391b0[(BYTE)view] = 1;
}

// Snaps a view's cameras to the car's camera placement and refreshes the view
// state (blending when a transition is running).
// match 69%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00421e20
void View_SnapCameras(BYTE view)
{
    FixMatrix placement;
    BYTE active;
    BYTE next;

    active = g_unk0x00538e0c[view] + view * 2;
    next = view * 2 - g_unk0x00538e0c[view] + 1;
    View_PlaceCarCamera(view);
    View_GetCarCameraRotation(&placement, view);
    Camera_SnapTo(VIEW_RECORD(active), &placement);
    if (View_GetActiveCameraMode(view) == 8) {
            Camera_SnapTo(VIEW_RECORD(next), &placement);
        CameraState_Interpolate(VIEW_STATE(view), VIEW_RECORD(active), VIEW_RECORD(next),
                     0x10000 - FixMul(VIEW_EASE(g_unk0x00538d20[view]), VIEW_EASE(g_unk0x00538d20[view])));
        View_UpdateCamera(view);
        return;
    }
    CameraState_Copy(VIEW_STATE(view), VIEW_RECORD(active));
    View_UpdateCamera(view);
}

// Switches a view to camera type `type` (0 = its current one) on `target`.
// FUNCTION: CMR2 0x00422fe0
void View_SetCameraType(int view, int type, BYTE target, int blend)
{
    if (type == 0)
        type = View_GetActiveCameraMode(view);
    View_SwitchCamera(view, type, 0xffff, target, blend);
}

// Blends a view's two stored camera states, carries the blended record's
// shake into the player's camera nodes, rebuilds the view node's matrix
// (pitching the forward axis and re-orthogonalising the basis) and marks the
// node and its ancestors dirty.
// FUNCTION: CMR2 0x00422140
void View_BlendCameraStates(BYTE view, int t)
{
    CameraRecord state;
    FixMatrix matrix;
    FixVector pitch;
    FixVector right;
    FixVector flat;
    FixVector forward;
    SceneNode *node;
    int len;
    int f48;
    int f58;
    int f5c;
    int f60;
    int shake;

    CameraState_Interpolate(&state, VIEW_PREV_STATE(view), VIEW_STATE(view), t);
    g_unk0x005391cc[view] = state.field_0x54;
    g_unk0x005391a8[view] = state.field_0x50;
    g_unk0x00538df0[view] = state.field_0x4c;
    matrix = state.matrix;
    f48 = state.field_0x48;
    f58 = state.field_0x58;
    f5c = state.field_0x5c;
    f60 = state.clearance;
    if (f60 != 0) {
        Car *car = Car_Get(view);
        int *pY;

        shake = FixMul(f48, f60);
        pY = &car->pNode0x71c->current.position.y;
        *pY += shake;
        pY = &car->pNode0x720->current.position.y;
        *pY += shake;
        matrix.position.y += f60;
        shake = FixMul(f60, f58);
        if (f5c != 0 && shake != 0) {
            FixMatrix_GetForward(&forward, &matrix);
            flat = forward;
            flat.y = 0;
            len = FixVec_Length(&flat);
            FixVecScale(&flat, &flat, forward.y);
            flat.y = -len;
            FixVecScale(&pitch, &forward, f5c);
            FixVecScale(&flat, &flat, shake);
            forward.x = flat.x + pitch.x;
            forward.y = flat.y + pitch.y;
            forward.z = flat.z + pitch.z;
            FixVec_Normalize(&forward, &forward);
            FixMatrix_SetForward(&forward, &matrix);
            FixMatrix_GetRight(&right, &matrix);
            FixVecScale(&pitch, &forward, FixVecDot(&right, &forward));
            right.x -= pitch.x;
            right.y -= pitch.y;
            right.z -= pitch.z;
            FixVec_Normalize(&right, &right);
            FixMatrix_SetRight(&right, &matrix);
            FixVecCross(&pitch, &forward, &right);
            FixMatrix_SetUp(&pitch, &matrix);
        }
    }
    node = g_viewNodes[view];
    FixMatrix_CopyRotation(&matrix, &node->current);
    while (node != NULL) {
        node->dirty = 1;
        node = node->pParent;
    }
}
