#include <windows.h>
#include <stdlib.h>
#include "StageBlock.h"
#include <string.h>
#include "RallyData.h"
#include "SceneNode.h"
#include "Frontend.h"
#include "InstallInfo.h"
#include "Texture.h"
#include "StageTiming.h"
#include "AIHelper.h"
#include "RegKey.h"
#include <stdio.h>
#include <math.h>
#include "FixedPoint.h"
#include "Car.h"
#include "WheelTrail.h"
#include "GameInfo.h"
#include "Input.h"
#include "main.h"
#include "Game.h"
#include "GenericFileLoader.h"
#include "FileBuffer.h"
#include "Mesh.h"
#include "Graphics.h"
#include "Font.h"
#include "Sprite.h"
#include "Collision2D.h"
#include "Menu.h"

struct GlowLight;
GlowLight *Glow_Add(int type, FixVector *pos, FixVector *dir, int unused1, int sizeX, int sizeY, int billboardTexture,
                    int layerTexture, int intensity, int node, BYTE projected, int unused2, int field_0x40);
void FUN_004ae3d0(BYTE *p, BYTE value);
void FUN_004ae3f0(BYTE *p, int value);
int FUN_00457e10(BYTE *pCar, int offset);
struct KnockoutMatch;
int FUN_00472990(KnockoutMatch *pMatch);
int FUN_0042cae0(Car *pCar, int variant);
int StageObject_GetWheelSlip(int carIndex, int wheelIndex);
BYTE FUN_00460bf0(int index);
int FUN_00460c10(int index);
int *FUN_00463270(int carIndex, int wheelIndex);
unsigned int FUN_00471bd0(BYTE **pOut);
void RallyData_FUN_00471cc0(int *pDest, void **pEntry);
int Track_GetGroundHeight(FixVector *pPoint, FixVector *pNormal, short *pTri,
                          unsigned short *pSurface, int defaultY);
void Sector_RemoveNode(SceneNode *pNode);
void FUN_004b8b10(SceneNode *pNode);
int FUN_004b7790(unsigned short id, int volume, int frequency, int loopStart, int loops, int is3D);
void Stage_InitLightMeshes(void);
int *FUN_00407520(int index);
void FUN_004925c0(int oldHeight, int newHeight, int mode);
void FUN_00492900(int value);
void FUN_00492fd0(int value);
void FUN_00477850(int object, int *src);
void FUN_0048df50(Car *param_1);
short *Car_GetOrder(void);
short Car_GetOrderCount(void);
BOOL Sound_LoadSample(char *name, BYTE flags, GenericFile *pFile);
void FUN_0048c900(BYTE index);
char *FUN_004752f0(int *p, int index, int mode);
void RallyData_FUN_00407500(BYTE param1);
unsigned short FUN_0040bbc0(unsigned short slot);
void FUN_0042b720(int index, char value);
int FUN_00469bc0(void *pCar, int index);
void FUN_00476c70(int index);
int FUN_00405600(void);
int FUN_00476850(int car, int pCar);

// Global fixed-point lighting parameters for both stage conditions.
// GLOBAL: CMR2 0x00547950
int g_stageLighting[0x178 / 4];

// Default intensity by stage weather index.
// GLOBAL: CMR2 0x0051b0b8
int g_stageWeatherIntensity[9] = {
    0x10000, 0x10000, 0x1999, 0x1999, 0x1999, 0x1999, 0x1999, 0x1999, 0x1999
};

struct StageSurfaceInfo {
    int flags;
    BYTE red, green, blue, alpha;
};
// GLOBAL: CMR2 0x0051bc68
StageSurfaceInfo g_stageSurfaceInfo[9] = {
    {0, 0, 0, 0, 0}, {1, 2, 2, 2, 0x18},
    {9, 0x48, 0x32, 0x27, 0x12}, {9, 0x14, 0x0d, 0, 0x12},
    {9, 0x0e, 0x0c, 0x06, 0x12}, {0x16, 0xad, 0xbd, 0xc6, 0x12},
    {0x16, 0xf4, 0xf4, 0xf4, 0x12}, {9, 0x48, 0x32, 0x27, 0x12},
    {4, 0x64, 0x64, 0x64, 0xff}
};
// GLOBAL: CMR2 0x0051bcb0
BYTE g_stageSurfaceMap[48] = {
    0, 7, 0, 0, 4, 4, 7, 7, 7, 0, 7, 7, 2, 2, 0, 0,
    0, 6, 5, 0, 0, 0, 6, 6, 0, 1, 2, 7, 7, 0, 6, 0,
    0, 0, 0, 3, 3, 3, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0
};
// GLOBAL: CMR2 0x00588620
BYTE g_trailColor[8][4];
// GLOBAL: CMR2 0x00588750
int g_trailFrame;
extern BYTE g_trailPointUsed[8][4][200];
extern BYTE g_trailPoints[8][4][200][0x28];
extern int g_unk0x00549b20[8][4];
extern FixVector g_unk0x00549c20[8][4];

// Accessors of the stage object tables (0x460bf0-0x4789b0)

extern void *g_unk0x00547ac8;
extern BYTE g_unk0x00543e98;
extern FixVector g_unk0x00547930;
extern int g_unk0x00547940;
extern void *g_unk0x0058c928;
extern void *g_unk0x0058c92c;
extern void *g_unk0x0058c930;
extern void *g_unk0x00543ecc;
extern int g_unk0x0058cf7c;
extern BYTE *g_unk0x0058c94c;
extern unsigned int g_unk0x0058ca6c;

int FUN_0046d2a0(int *p);
int RallyData_FUN_00421370(BYTE *p);
int RallyData_FUN_00421420(void);
unsigned int RallyData_FUN_00407e90(void);
unsigned int RallyData_FUN_00407ea0(void);
float FUN_00456ae0(void);

// Chooses the stage object path for the current game mode and rally state.
// FUNCTION: CMR2 0x0046bd50
int StageObject_UsesExtendedMode(void)
{
    if (CGameInfo::FUN_00405d80() == 8 ||
        CGameInfo::FUN_00405d80() == 9 ||
        CGameInfo::FUN_00405d80() == 10)
        return false;
    if (CGameInfo::FUN_00405d80() == 11 ||
        CGameInfo::FUN_00405d80() == 12)
        return true;
    if (CGameInfo::FUN_00405d80() != 0 &&
        CGameInfo::FUN_00405d80() != 1 &&
        CGameInfo::FUN_00405d80() != 2 &&
        CGameInfo::FUN_00405d80() != 3)
        return true;
    return (BYTE)RallyData_FUN_00407e90() != 0;
}

void FUN_0049c440(Mesh *pMesh, int mask, int value);
void FUN_0049c4b0(Mesh *pMesh, int mask, int value);

// Enables/disables the sub-meshes of a stage object's model according to the
// object type and the current game mode.
// FUNCTION: CMR2 0x004694a0
void FUN_004694a0(int param_1, int param_2, char param_3)
{
    int *pParts;
    BYTE *pType;
    int i;
    int uVar4;
    int uVar3;

    pParts = FUN_00469680((int)*(char *)(param_1 + 0xb1a));
    pType = FUN_00456be0((int)*(char *)(param_1 + 0xb1a));
    if (*(char *)(pType + 0x20) == 'C' ||
        (pType = FUN_00456be0((int)*(char *)(param_1 + 0xb1a)), *(char *)(pType + 0x20) == 'A')) {
        if (param_2 == 0) {
            uVar4 = 0;
            uVar3 = 1;
        } else if (param_2 == 1 || param_3 == 4 || param_3 == 5) {
            uVar4 = 3;
            uVar3 = 4;
        } else {
            uVar4 = 5;
            uVar3 = 7;
        }
        switch (param_3) {
        case 0:
            i = FUN_004692b0(7, (BYTE *)pParts);
            if (i >= 0) {
                FUN_0049c440((Mesh *)pParts[i], 0x100, uVar4);
                FUN_0049c4b0((Mesh *)pParts[i], 0x100, uVar3);
                return;
            }
            break;
        case 1:
            i = FUN_004692b0(0xc, (BYTE *)pParts);
            if (i >= 0) {
                FUN_0049c440((Mesh *)pParts[i], 0x20, uVar4);
                FUN_0049c4b0((Mesh *)pParts[i], 0x20, uVar3);
            }
            i = FUN_004692b0(7, (BYTE *)pParts);
            if (i >= 0) {
                FUN_0049c440((Mesh *)pParts[i], 0x20, uVar4);
                FUN_0049c4b0((Mesh *)pParts[i], 0x20, uVar3);
                return;
            }
            break;
        case 2:
            i = FUN_004692b0(7, (BYTE *)pParts);
            if (i >= 0) {
                FUN_0049c440((Mesh *)pParts[i], 0x40, uVar4);
                FUN_0049c4b0((Mesh *)pParts[i], 0x40, uVar3);
                return;
            }
            break;
        case 3:
            i = FUN_004692b0(7, (BYTE *)pParts);
            if (i >= 0) {
                FUN_0049c440((Mesh *)pParts[i], 0x80, uVar4);
                FUN_0049c4b0((Mesh *)pParts[i], 0x80, uVar3);
                return;
            }
            break;
        case 4:
            i = FUN_004692b0(0xe, (BYTE *)pParts);
            if (i >= 0) {
                FUN_0049c440((Mesh *)pParts[i], 4, uVar4);
                return;
            }
            break;
        case 5:
            i = FUN_004692b0(0xe, (BYTE *)pParts);
            if (i >= 0)
                FUN_0049c440((Mesh *)pParts[i], 8, uVar4);
            break;
        }
    }
}

// FUNCTION: CMR2 0x00469de0
int StageObject_IsEligibleType(short type, int mode, int category)
{
    int result = 0;
    if (mode == 0 || category == 6) {
        switch (type) {
        case 1:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 16:
        case 24:
        case 25:
        case 27:
        case 28:
            result = 1;
        }
    }
    return result;
}

// Full strength within ten fixed-point units, fading to zero at fifty.
// FUNCTION: CMR2 0x004863d0
int StageObject_DistanceFade(FixVector *delta)
{
    int component;
    int length;
    int fade;

    component = delta->x;
    if (component < 0) component = -component;
    if (component <= 0x320000) {
        component = delta->y;
        if (component < 0) component = -component;
        if (component <= 0x320000) {
            component = delta->z;
            if (component < 0) component = -component;
            if (component <= 0x320000) {
                length = FixVecLength(delta);
                if (length <= 0x320000) {
                    length -= 0xa0000;
                    if (length <= 0) return 0x10000;
                    fade = FixMul(length, 0x666);
                    if (fade > 0x10000) fade = 0x10000;
                    return 0x10000 - fade;
                }
            }
        }
    }
    return 0;
}

// GLOBAL: CMR2 0x00543f28
int g_unk0x00543f28[8 * 4];
// GLOBAL: CMR2 0x00547b80
BYTE g_unk0x00547b80;
// GLOBAL: CMR2 0x00548110
BYTE g_unk0x00548110[8][8];
// GLOBAL: CMR2 0x00588758
int *g_unk0x00588758;
// GLOBAL: CMR2 0x00588760
char g_unk0x00588760;
// GLOBAL: CMR2 0x00588761
signed char g_unk0x00588761;
// GLOBAL: CMR2 0x00588864
int g_unk0x00588864;
// GLOBAL: CMR2 0x00588868
int g_unk0x00588868;
// GLOBAL: CMR2 0x00588970
int g_unk0x00588970[8];
// GLOBAL: CMR2 0x00588ba4
BYTE g_unk0x00588ba4[16];
// GLOBAL: CMR2 0x00588bb4
int g_unk0x00588bb4[16];
// GLOBAL: CMR2 0x00588cd4
int g_unk0x00588cd4[8 * 2];
// GLOBAL: CMR2 0x00588d38
int g_unk0x00588d38;
// GLOBAL: CMR2 0x00589438
int g_unk0x00589438;
// GLOBAL: CMR2 0x0058943c
int g_unk0x0058943c;
// GLOBAL: CMR2 0x00589440
int g_unk0x00589440;
// GLOBAL: CMR2 0x00589444
int g_unk0x00589444;
// GLOBAL: CMR2 0x00589448
GenericFile g_unk0x00589448;
// GLOBAL: CMR2 0x0058cf68
int g_unk0x0058cf68;
// GLOBAL: CMR2 0x0058cf6c
int g_unk0x0058cf6c;
// GLOBAL: CMR2 0x0058cf80
BYTE g_unk0x0058cf80[0x100];
// Scratch rotation matrix for the stage-object nodes.
// GLOBAL: CMR2 0x0058d260
FixMatrix g_unk0x0058d260;
BYTE g_stageBlock[0x430];
// GLOBAL: CMR2 0x0058da30
int g_unk0x0058da30[8];
// GLOBAL: CMR2 0x0058da10
int g_unk0x0058da10[8];
// GLOBAL: CMR2 0x0058dda8
int g_unk0x0058dda8;
// GLOBAL: CMR2 0x0058e230
int g_unk0x0058e230[15];
// Random seed of the stage (kept for replays).
// GLOBAL: CMR2 0x0058e26c
int g_unk0x0058e26c;
// GLOBAL: CMR2 0x0058e270
char g_unk0x0058e270[16];
// GLOBAL: CMR2 0x0058e0b0
BYTE g_unk0x0058e0b0[8];
struct StageObjectValue { int value; BYTE rest[0x2c]; };
// GLOBAL: CMR2 0x0058e0b8
StageObjectValue g_unk0x0058e0b8[4];
// GLOBAL: CMR2 0x0058e178
int g_unk0x0058e178;

// GLOBAL: CMR2 0x005113f8
double g_radiansToDegrees = 57.295827908797776;

// The larger of the longitudinal and lateral wheel slip, after each dead zone.
// FUNCTION: CMR2 0x00465d70
int StageObject_GetWheelSlip(int carIndex, int wheelIndex)
{
    if (carIndex < 8) {
        int slip = Car_Get(carIndex)->field_0x880[wheelIndex];
        if (slip < 0)
            slip = -Car_Get(carIndex)->field_0x880[wheelIndex];
        else
            slip = Car_Get(carIndex)->field_0x880[wheelIndex];
        slip -= 0xccc;
        if (slip < 0) slip = 0;

        int lateral = Car_Get(carIndex)->field_0x870[wheelIndex];
        if (lateral < 0)
            lateral = -Car_Get(carIndex)->field_0x870[wheelIndex];
        else
            lateral = Car_Get(carIndex)->field_0x870[wheelIndex];
        lateral -= 0x2666;
        if (lateral < 0) lateral = 0;
        if (slip < lateral) slip = lateral;
        if (slip < 0) slip = -slip;
        if (slip > 0) {
            slip = FixDiv(slip, 0x10000);
            if (slip < 0x10000) return slip;
            return 0x10000;
        }
    }
    return 0;
}

// Returns a 16.16 angle in degrees from the two vector components.
// FUNCTION: CMR2 0x00498d80
int StageObject_Atan2Degrees(int y, int x)
{
    double degrees = atan2((double)y, (double)x) * g_radiansToDegrees;
    return (int)(__int64)(degrees * CGraphics::m_65536);
}

int FUN_0041d290(void);
char FUN_00420190(void);

// FUNCTION: CMR2 0x00478130
void FUN_00478130(int index)
{
    g_unk0x0058da10[index] = FUN_0041d290();
}

// FUNCTION: CMR2 0x00478150
void FUN_00478150(int index)
{
    g_unk0x0058da30[index] = FUN_0041d290() - g_unk0x0058da10[index];
}

// FUNCTION: CMR2 0x0047aa60
void FUN_0047aa60(int value)
{
    g_unk0x0058dda8 = value;
}

// FUNCTION: CMR2 0x0047c5b0
int FUN_0047c5b0(int index)
{
    return g_unk0x0058e230[index];
}

// FUNCTION: CMR2 0x0047c5c0
void FUN_0047c5c0(void)
{
    StageObjectValue *p = g_unk0x0058e0b8;
    do {
        p->value = 0x10000;
        p++;
    } while ((int)p < (int)&g_unk0x0058e178);
}

// Per-type animation tables (0x58e394..0x58e4a4; the loader also fills the tail bytes).
// GLOBAL: CMR2 0x0058e394
BYTE *g_unk0x0058e394[68];
// GLOBAL: CMR2 0x0058e4a4
BYTE *g_unk0x0058e4a4;

// Looks up the pair of values that table `table` gives for the key of entry `entry`.
// FUNCTION: CMR2 0x0047cbc0
void FUN_0047cbc0(int unused, int table, int entry, int *pOut1, int *pOut2)
{
    int key = *(int *)(entry * 0x10 + 4 + g_unk0x0058e4a4);
    BYTE *p = g_unk0x0058e394[table];
    int i;

    for (i = 0; i < *(int *)(p + 0x84); i++) {
        if (key == (char)p[4 + i * 8]) {
            *pOut1 = (char)p[7 + i * 8];
            *pOut2 = (char)g_unk0x0058e394[table][6 + i * 8];
            return;
        }
    }
}

// FUNCTION: CMR2 0x0047cc30
void FUN_0047cc30(void)
{
    g_unk0x0058e0b0[0] = FUN_00420190();
    g_unk0x0058e0b0[1] = 1;
}

// FUNCTION: CMR2 0x0047cd00
int FUN_0047cd00(int index)
{
    return g_unk0x0058e270[index];
}

struct Block0x309 { int data[0x309]; };
struct Block0x134 { int data[0x134]; };
struct Block6 { int data[6]; };

struct StageTableEntry { int flag; short a; short b; };
// GLOBAL: CMR2 0x0051b9f0
StageTableEntry g_unk0x0051b9f0[5] = {
    {0, 2, 2}, {0, 2, 1}, {0x10000, 2, 1}, {0, 1, 2}, {1, 1, 2}
};

// FUNCTION: CMR2 0x00464b00
StageTableEntry *FUN_00464b00(int index)
{
    return &g_unk0x0051b9f0[index];
}

// FUNCTION: CMR2 0x0046b6b0
void FUN_0046b6b0(SceneNode *pNode, BYTE threshold)
{
    if (pNode->type == 0 && pNode->pObject != NULL &&
        *(BYTE *)(*(int *)((BYTE *)pNode->pObject + 0x24) + 0x37) <= threshold)
        pNode->field_0x17c = 0;
}

// FUNCTION: CMR2 0x0046b6e0
void FUN_0046b6e0(SceneNode *pNode, BYTE threshold)
{
    for (; pNode != NULL; pNode = pNode->pNext) {
        FUN_0046b6b0(pNode, threshold);
        if (pNode->pFirstChild != NULL)
            FUN_0046b6e0(pNode->pFirstChild, threshold);
    }
}

struct StageObjectEntry0x128 { int field_0x0; int *pObject; BYTE rest[0x120]; };
// GLOBAL: CMR2 0x005894e0
StageObjectEntry0x128 g_unk0x005894e0[40];
// GLOBAL: CMR2 0x0058c320
BYTE g_unk0x0058c320;
// GLOBAL: CMR2 0x0058c324
int g_unk0x0058c324;
// GLOBAL: CMR2 0x0058c924
BYTE g_unk0x0058c924;

// FUNCTION: CMR2 0x0046f7e0
void FUN_0046f7e0(void)
{
    int **pp;

    g_unk0x0058c924 = 0;
    pp = &g_unk0x005894e0[0].pObject;
    do {
        if (*pp != NULL)
            (*pp)[0xcc / 4] += 0xd8f00000U;
        pp = (int **)((BYTE *)pp + sizeof(StageObjectEntry0x128));
    } while ((int)pp < (int)&g_unk0x005894e0[40].pObject);
}

// Initializes a moving stage object from its route entry and the car's motion.
// match 51%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046f810
void StageObject_InitMovingObject(int *pState, int carIndex)
{
    Car *pCar = Car_Get(carIndex);
    BYTE *pEntries = NULL;
    int count = FUN_00471bd0(&pEntries);
    SceneNode *pNode = (SceneNode *)pState[1];
    int entryIndex = -1;
    int i;
    for (i = 0; i < count; ++i) {
        if ((int)(pEntries + i * 8) == pState[0]) {
            entryIndex = i;
            break;
        }
    }
    if (entryIndex < 0) {
        pNode->type = SCENE_NODE_EMPTY;
        return;
    }

    BYTE mappedIndex = ((BYTE *)g_unk0x0058c928)[entryIndex];
    pState[0x34] = ((int *)g_unk0x0058c930)[mappedIndex];
    pNode->type = SCENE_NODE_MESH;
    pNode->pObject = (void *)((int *)g_unk0x0058c92c)[mappedIndex];
    pNode->field_0x17c = (BYTE)(1 << (carIndex & 31));
    pState[0x45] = -0x10000;

    FixVector position;
    RallyData_FUN_00471cc0((int *)&position, (void **)pState);
    FixMatrix *pNodeMatrix = &pNode->current;
    FixMatrix_SetPosition(&position, pNodeMatrix);

    BYTE *pObject = *(BYTE **)pState[0];
    FixMatrix *pObjectMatrix = (FixMatrix *)(pObject + 0x18);
    FixVector basis;
    FixMatrix_GetRight(&basis, pObjectMatrix);
    FixMatrix_SetRight(&basis, pNodeMatrix);
    FixMatrix_GetUp(&basis, pObjectMatrix);
    FixMatrix_SetUp(&basis, pNodeMatrix);
    FixMatrix_GetForward(&basis, pObjectMatrix);
    FixMatrix_SetForward(&basis, pNodeMatrix);

    pState[0x32] = (int)(pState + 2);
    memcpy(pState + 2, pNodeMatrix, 0x40);
    memcpy(pState + 0x22, pNodeMatrix, 0x40);
    memcpy(pState + 0x12, pNodeMatrix, 0x40);

    int objectType = pState[0x34];
    int groundPath = objectType == 1 || (objectType == 0 && pCar->speed <= 0xc000);
    int airbornePath = objectType == 0 && pCar->speed > 0xc000;
    pState[0x44] = 0;
    *(short *)(pState + 0x47) = -1;

    if (groundPath || airbornePath) {
        FixVector velocity = pCar->velocity;
        FixVector localVelocity;
        FixVector axis;
        BYTE *pParams = *(BYTE **)(*(BYTE **)(pObject + 0xc) + 0x10c);
        if (groundPath) {
            int triangle = -1;
            int surface = 0;
            Track_GetGroundHeight(&position, (FixVector *)(pState + 0x35),
                                  (short *)&triangle, (unsigned short *)&surface, 0);
            if (pCar->speed > 0x8000) {
                FixVecScaleRecip(&velocity, &velocity, pCar->speed);
                FixVecScale(&velocity, &velocity, 0x8000);
            }
            FixMatrix_InverseRotateVector(&localVelocity, &velocity, (FixMatrix *)pState[0x32]);
            axis.x = FixMul(localVelocity.z, 0x10000);
            axis.y = 0;
            axis.z = -FixMul(localVelocity.x, 0x10000);
            if (pState[0x34] == 0)
                axis.z = 0;
            pState[0x41] = FixMul(*(int *)(pParams + 0x44), 0x8000);
            pState[0x42] = FixMul(*(int *)(pParams + 0x4c), 0x8000);
            pState[0x43] = FixMul(*(int *)(pParams + 0x48), 0x8000);
            pState[0x3e] = 0;
            pState[0x3f] = pState[0x42];
            pState[0x40] = 0;
            FixVecScale((FixVector *)(pState + 0x38), &axis, g_physicsTimeStep);
            pState[0x33] = 1;
        } else {
            FixMatrix_InverseRotateVector(&localVelocity, &velocity, (FixMatrix *)(pState + 2));
            axis.x = -FixMul(localVelocity.z, 0x6666);
            axis.y = 0;
            axis.z = FixMul(localVelocity.x, 0x6666);
            FixVecScale((FixVector *)(pState + 0x38), &axis, g_physicsTimeStep);
            FixVecScale((FixVector *)(pState + 0x3b), &velocity, 0xcccc);
            pState[0x3c] = FixMul(FixVecLength(&velocity), 0x4ccc);
            pState[0x41] = FixMul(*(int *)(pParams + 0x44), 0x8000);
            pState[0x42] = FixMul(*(int *)(pParams + 0x4c), 0x8000);
            pState[0x43] = FixMul(*(int *)(pParams + 0x48), 0x8000);
            pState[0x3e] = 0;
            pState[0x3f] = pState[0x42];
            pState[0x40] = 0;
            pState[0x48] = 0;
            pState[0x33] = 2;
            pState[0x49] = 1;
        }
    }

    memcpy(&pNode->world, pNodeMatrix, sizeof(FixMatrix));
    if (pNode->sector != -1)
        Sector_RemoveNode(pNode);
    if (pNode->sector == -1)
        FUN_004b8b10(pNode);
}

// Derives the stage's object scale from the loaded records: the average of
// their +0x54 fields (the single record in created-flag mode 1), floored at
// 0x4ccc, and scales the global stage vector by it, quadrupled for a type 2.
// match 84%: MSVC picks EDX for the record count and ESI for the loop counter
// (the original has them swapped); the code itself is identical.
// match 83%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00460a30
void FUN_00460a30(FixVector *pOut)
{
    DWORD flags;
    int value;
    int typeTwo;
    int count;
    int i;
    int *p;

    // The original loads the whole dword at the flag byte (the variable lived
    // in that translation unit; see CONOCIMIENTO 4.y) and only uses its low byte.
    flags = *(DWORD *)&g_unk0x00543e98;
    value = 0;
    typeTwo = 0;
    if ((char)flags == 1) {
        value = *((int *)g_unk0x00547ac8 + 0x15);
        if (*((int *)g_unk0x00547ac8) == 2)
            typeTwo = 1;
    } else {
        count = (BYTE)flags;
        p = (int *)g_unk0x00547ac8;
        for (i = count; i > 0; i--) {
            value += p[0x15];
            if (*p == 2)
                typeTwo = 1;
            p += 0x5e;
        }
        if ((char)flags != 0)
            value = FixDiv(value, count << 16);
        else
            value = 0x4ccc;
    }
    if (value < 0x4ccc)
        value = 0x4ccc;
    value = FixMul(value, g_unk0x00547940);
    if (typeTwo)
        value = FixMul(0x20000, value);
    FixVecScale(pOut, &g_unk0x00547930, value);
}

// FUNCTION: CMR2 0x00460bf0
BYTE FUN_00460bf0(int index)
{
    return *((BYTE *)g_unk0x00547ac8 + index * 0x178);
}

// FUNCTION: CMR2 0x00460c10
int FUN_00460c10(int index)
{
    return *(int *)((BYTE *)g_unk0x00547ac8 + 0x54 + index * 0x178);
}

// GLOBAL: CMR2 0x00543d50
int g_unk0x00543d50;

// Sets an object's scalar and derives its fixed-point scaled component.
// match 72%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00460c30
void StageObject_SetScaledValue(int value, int index)
{
    BYTE *entry;
    int scaled;
    int factor;

    entry = (BYTE *)g_unk0x00547ac8 + index * 0x178;
    *(int *)(entry + 0x54) = value;
    scaled = FixMul(g_unk0x00543d50, value);
    factor = (int)*(short *)(entry + 0x74) << 16;
    *(int *)(entry + 0x5c) = FixMul(scaled, factor);
}

// FUNCTION: CMR2 0x00460c80
int FUN_00460c80(BYTE *pCar)
{
    return *(int *)((BYTE *)g_unk0x00543ecc + 8 + (signed char)pCar[0xb1a] * 0xc);
}

// FUNCTION: CMR2 0x00463270
int *FUN_00463270(int i, int j)
{
    return &g_unk0x00543f28[j + i * 4];
}

// FUNCTION: CMR2 0x00463ce0
void FUN_00463ce0(BYTE value)
{
    g_unk0x00547b80 = value;
    if (value > 7)
        g_unk0x00547b80 = 6;
}

// FUNCTION: CMR2 0x00464af0
BYTE *FUN_00464af0(int index)
{
    return g_unk0x00548110[index];
}

// FUNCTION: CMR2 0x00465ea0
int FUN_00465ea0(int value)
{
    if (value < 0)
        return 0;
    if (value > 0xff)
        value = 0xff;
    return value;
}

// FUNCTION: CMR2 0x00465f80
void FUN_00465f80(void)
{
    g_unk0x00588760 = 0xff;
    g_unk0x00588864 = -1;
}

// GLOBAL: CMR2 0x0058875c
Car *g_unk0x0058875c;

void FUN_00465ec0(SceneNode *pNode, int alpha, BYTE checkFlag);
void FUN_00465f20(SceneNode *pNode, int alpha, BYTE checkFlag);

// Makes a car the ghost car: flags it and fades its body nodes in.
// FUNCTION: CMR2 0x00465fc0
void FUN_00465fc0(Car *pCar)
{
    g_unk0x0058875c = pCar;
    pCar->field_0xc0c = 1;
    g_unk0x00588761 = -1;
    g_unk0x00588864 = -1;
    FUN_00465ec0(pCar->pNode0x71c, 100, 1);
    FUN_00465ec0(pCar->pNode0x720, 100, 1);
    FUN_00465f20(pCar->pNode0x71c->pFirstChild, 100, 1);
    FUN_00465f20(pCar->pNode0x720->pFirstChild, 100, 1);
}

// FUNCTION: CMR2 0x00466080
void FUN_00466080(void)
{
    FUN_0046d2a0(g_unk0x00588758);
}

// FUNCTION: CMR2 0x00466090
int FUN_00466090(void)
{
    if (g_unk0x00588758 != NULL)
        return g_unk0x00588758[1];
    return 0;
}

// FUNCTION: CMR2 0x004660e0
void FUN_004660e0(BYTE value)
{
    g_unk0x00588761 = value;
}

// FUNCTION: CMR2 0x004660f0
int FUN_004660f0(void)
{
    return g_unk0x00588760;
}

// FUNCTION: CMR2 0x0046b400
void FUN_0046b400(int value, int index)
{
    g_unk0x00588970[index] = value;
}

// FUNCTION: CMR2 0x0046b420
void FUN_0046b420(void)
{
    memset(g_unk0x00588970, 0, 8 * sizeof(int));
}

// Reads vertex `vertex` of mesh `mesh` (float source data) as a 16.16 vector.
// FUNCTION: CMR2 0x0046b440
void FUN_0046b440(Mesh **ppMeshes, int mesh, int vertex, int *pOut)
{
    pOut[0] = (int)(__int64)(*(float *)((BYTE *)ppMeshes[mesh]->pVertexData + vertex * 0x30) * CGraphics::m_65536);
    pOut[1] = (int)(__int64)(*(float *)((BYTE *)ppMeshes[mesh]->pVertexData + vertex * 0x30 + 4) * CGraphics::m_65536);
    pOut[2] = (int)(__int64)(*(float *)((BYTE *)ppMeshes[mesh]->pVertexData + vertex * 0x30 + 8) * CGraphics::m_65536);
}

// FUNCTION: CMR2 0x0046b4c0
int FUN_0046b4c0(BYTE *pCar)
{
    return g_unk0x00588970[(signed char)pCar[0xb1a]];
}

// match 50%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046b710
void FUN_0046b710(void)
{
    int i;

    memset(g_unk0x00588bb4, 0, 8 * sizeof(int));
    for (i = 0; i < 8; i++) {
        g_unk0x00588cd4[i * 2] = 0;
        g_unk0x00588cd4[i * 2 + 1] = 0;
    }
}

// FUNCTION: CMR2 0x0046b740
void FUN_0046b740(int i, int value, int j)
{
    g_unk0x00588cd4[j + i * 2] = value;
}

// FUNCTION: CMR2 0x0046b760
void FUN_0046b760(int index, int reset)
{
    if (reset != 0) {
        g_unk0x00588bb4[index] = 0;
        return;
    }
    g_unk0x00588bb4[index] = 1;
}

// FUNCTION: CMR2 0x0046bd20
int FUN_0046bd20(int i, int j)
{
    return g_unk0x00588cd4[j + i * 2];
}

// FUNCTION: CMR2 0x0046bd40
BYTE FUN_0046bd40(int index)
{
    return g_unk0x00588ba4[index];
}

// Decodes one 4-byte replay input packet into the control record pOut and the
// car's handbrake/light switches; returns 1 when the packet's repeat count is
// used up.
// FUNCTION: CMR2 0x0046bec0
int FUN_0046bec0(int *pState, BYTE *pIn, BYTE *pOut, BYTE *pCounter, Car *pCar)
{
    BYTE steer = pIn[3] & 0x3f;
    BYTE *p = (BYTE *)pCar;

    if ((pIn[3] & 0x40) != 0) {
        pOut[0] = steer;
        pOut[1] = 0;
    } else {
        pOut[0] = 0;
        pOut[1] = steer;
    }
    pOut[2] = pIn[0] >> 2;
    pOut[3] = pIn[1] >> 2;
    *(unsigned int *)(pOut + 8) = pIn[2] >> 7;
    pOut[4] = (pIn[1] & 3) - 1;
    *(unsigned int *)(pOut + 0xc) = (pIn[2] & 0x40) >> 6;
    if ((pIn[0] & 1) != 0)
        *(int *)(p + 0xb8c) = 1;
    else
        *(int *)(p + 0xb8c) = 0;
    if ((pIn[0] & 2) != 0)
        *(int *)(p + 0xb90) = 1;
    else
        *(int *)(p + 0xb90) = 0;
    if ((pIn[3] & 0x80) != 0) {
        if (*pState == 0)
            *(int *)(p + 0xb88) = 2;
        else
            *(int *)(p + 0xb88) = 1;
    } else
        *(int *)(p + 0xb88) = 0;
    if ((BYTE)++*pCounter < (pIn[2] & 0x3f))
        return 0;
    *pCounter = 0;
    return 1;
}

// FUNCTION: CMR2 0x0046bfb0
void FUN_0046bfb0(Block0x309 *pSrc, Block0x309 *pDst)
{
    *pDst = *pSrc;
}

// Restores a car from a saved state record, keeping the destination's scene
// node bindings, its wheel emitter vectors and its timing index.
// FUNCTION: CMR2 0x0046bfd0
void FUN_0046bfd0(Block0x309 *pSrc, Car *pDst)
{
    SceneNode *pNode0x71c = pDst->pNode0x71c;
    SceneNode *pNode0x720 = pDst->pNode0x720;
    SceneNode *pNode0x724 = pDst->pNode0x724;
    SceneNode *pExtraNodes[4];
    SceneNode *pWheelNodes[4];
    SceneNode *pViewNodeNear = pDst->pViewNodeNear;
    SceneNode *pViewNodeFar = pDst->pViewNodeFar;
    FixMatrix *pWorld = pDst->pWorld;
    FixMatrix *pBodyMatrix = pDst->pBodyMatrix;
    FixVector wheelEmitter[4];
    char carIndex = pDst->field_0xb1a;
    int i;

    for (i = 0; i < 4; i++) {
        pExtraNodes[i] = pDst->pExtraNodes[i];
        pWheelNodes[i] = pDst->pWheelNodes[i];
    }
    memcpy(wheelEmitter, pDst->wheelEmitter, sizeof(wheelEmitter));

    *(Block0x309 *)pDst = *pSrc;

    pDst->pNode0x71c = pNode0x71c;
    pDst->pNode0x720 = pNode0x720;
    pDst->pNode0x724 = pNode0x724;
    pDst->pViewNodeNear = pViewNodeNear;
    pDst->pViewNodeFar = pViewNodeFar;
    pDst->pWorld = pWorld;
    pDst->pBodyMatrix = pBodyMatrix;
    for (i = 0; i < 4; i++) {
        pDst->pExtraNodes[i] = pExtraNodes[i];
        pDst->pWheelNodes[i] = pWheelNodes[i];
        pDst->wheelEmitter[i] = wheelEmitter[i];
    }
    pDst->field_0xb1a = carIndex;

    if ((BYTE)RallyData_FUN_00407ea0() != 0)
        *(float *)(pDst->field_0xa90 + 8) = 25.0f;
    if (pDst->field_0xb1a > 0 && (BYTE)RallyData_FUN_00407e90() != 0 &&
        (BYTE)CGameInfo::FUN_00405e00() == 0 && (BYTE)RallyDataState() == 1)
        *(float *)(pDst->field_0xa90 + 8) = FUN_00456ae0();
}

// FUNCTION: CMR2 0x0046c180
void FUN_0046c180(Block0x134 *pSrc, Block0x134 *pDst)
{
    *pDst = *pSrc;
}

// Copies a 0x134-int car state record, keeping the destination's first three
// 15-int blocks.
// match 77%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046c1a0
void FUN_0046c1a0(Block0x134 *pSrc, Block0x134 *pDst)
{
    int keepC[15];
    int keepB[15];
    int keepA[15];
    int i;

    memcpy(keepC, pDst->data + 30, sizeof(keepC));
    memcpy(keepB, pDst->data + 15, sizeof(keepB));
    memcpy(keepA, pDst->data, sizeof(keepA));
    *pDst = *pSrc;
    for (i = 0; i < 15; i++) {
        pDst->data[i] = keepA[i];
        pDst->data[15 + i] = keepB[i];
        pDst->data[30 + i] = keepC[i];
    }
}

// FUNCTION: CMR2 0x0046c220
void FUN_0046c220(Block6 *pSrc, Block6 *pDst)
{
    *pDst = *pSrc;
}

int *FUN_00469680(int index);
struct RaceRecord;
RaceRecord *RallyData_FUN_00421510(int index);
void RallyData_FUN_004207a0(int index);

// Snapshots a car's state into the 0x1100-byte record at pDst.
// FUNCTION: CMR2 0x0046c240
void FUN_0046c240(BYTE *pDst, BYTE car)
{
    Car *pCar = Car_Get(car);

    FUN_0046c180((Block0x134 *)FUN_00469680(car), (Block0x134 *)pDst);
    FUN_0046bfb0((Block0x309 *)pCar, (Block0x309 *)(pDst + 0x4d0));
    FUN_0046c220((Block6 *)RallyData_FUN_00421510(car), (Block6 *)(pDst + 0x10f4));
    RallyData_FUN_004207a0(car);
}

// FUNCTION: CMR2 0x0046d2a0
int FUN_0046d2a0(int *p)
{
    if (p != NULL && p[1] != 0) {
        p[1] = 0;
        p[2] = 0;
        p[5] = 0;
        return 1;
    }
    return 0;
}

// FUNCTION: CMR2 0x0046d500
int FUN_0046d500(void)
{
    return g_unk0x00588d38;
}

// FUNCTION: CMR2 0x0046f4c0
void FUN_0046f4c0(int *pOut)
{
    *pOut = g_unk0x00589438;
}

// FUNCTION: CMR2 0x0046f4d0
void FUN_0046f4d0(int *pOut)
{
    *pOut = g_unk0x0058943c;
}

// GLOBAL: CMR2 0x0058c928
void *g_unk0x0058c928;
// GLOBAL: CMR2 0x0058c92c
void *g_unk0x0058c92c;
// GLOBAL: CMR2 0x0058c930
void *g_unk0x0058c930;

// Releases the three files loaded by 0x46f550 (registered callback).
// FUNCTION: CMR2 0x0046f500
int FUN_0046f500(void)
{
    if (g_unk0x0058c928 != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x0058c928);
        g_unk0x0058c928 = NULL;
    }
    if (g_unk0x0058c930 != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x0058c930);
        g_unk0x0058c930 = NULL;
    }
    if (g_unk0x0058c92c != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x0058c92c);
        g_unk0x0058c92c = NULL;
    }
    g_unk0x0058c320 = 0;
    return 1;
}

// FUNCTION: CMR2 0x0046f4e0
void FUN_0046f4e0(int *pOut1, int *pOut2)
{
    *pOut1 = g_unk0x00589440;
    *pOut2 = g_unk0x00589444;
}

// GLOBAL: CMR2 0x0051c6d0
char g_strTempGro[] = "TEMP.GRO";
// GLOBAL: CMR2 0x0051c6dc
char g_strTopC3d[] = "top.c3d";
// GLOBAL: CMR2 0x0051c6e4
char g_strC3dExt[] = ".c3d";
// GLOBAL: CMR2 0x0051c6ec
char g_strBflExt[] = ".bfl";
// GLOBAL: CMR2 0x0051c6f4
char g_strTempSky[] = "TEMP.SKY";

void FUN_004b2e40(BYTE *p, int value);
void FUN_0046ef50(void);
int FUN_004b9380(unsigned int, unsigned int, unsigned int);
GenericFile *FUN_0041f500(void);
int RallyData_FUN_00411060(void);
BYTE FUN_0046f030(void);

// Loads the stage's sky and ground objects: the TEMP.SKY archive (also opened
// as .bfl, .c3d and top.c3d) and TEMP.GRO, releasing each one's meshes first.
// FUNCTION: CMR2 0x0046f060
void FUN_0046f060(void)
{
    char buffer[MAX_PATH];
    GenericFile *pFile;
    GenericFile *pC3d;
    int node;
    BYTE *pMesh;

    pFile = (GenericFile *)CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(), g_strTempSky, NULL, NULL, 0);
    if (pFile != NULL) {
        g_unk0x00589438 = (int)FUN_004b9380((unsigned int)pFile, (unsigned int)RallyData_FUN_00411060(),
                                           (unsigned int)FUN_0041f500());
        if (g_unk0x00589438 != 0) {
            FUN_004b2e40(*(BYTE **)(g_unk0x00589438 + 0xc), 0);
            *(int *)(g_unk0x00589438 + 0x180) = 0;
            node = *(int *)(g_unk0x00589438 + 4);
            if (node != 0) {
                pMesh = *(BYTE **)(node + 0xc);
                *(int *)(node + 0x180) = 0;
                FUN_004b2e40(pMesh, 0);
                node = *(int *)(*(int *)(g_unk0x00589438 + 4));
                if (node != 0) {
                    pMesh = *(BYTE **)(node + 0xc);
                    *(int *)(node + 0x180) = 0;
                    FUN_004b2e40(pMesh, 0);
                    node = *(int *)(*(int *)(*(int *)(g_unk0x00589438 + 4)));
                    if (node != 0) {
                        pMesh = *(BYTE **)(node + 0xc);
                        *(int *)(node + 0x180) = 0;
                        FUN_004b2e40(pMesh, 0);
                    }
                }
            }
        }
    }
    FUN_0046ef50();
    strcpy(buffer, CFrontend::m_stringDest);
    strcpy(CFrontend::m_stringDest, buffer);
    strcat(CFrontend::m_stringDest, g_strBflExt);
    CGenericFileLoader::FUN_004a9d70(&g_unk0x00589448, CFrontend::m_stringDest);
    strcpy(CFrontend::m_stringDest, buffer);
    strcat(CFrontend::m_stringDest, g_strC3dExt);
    pC3d = (GenericFile *)CGenericFileLoader::FindFile(&g_unk0x00589448, CFrontend::m_stringDest, NULL, NULL, 0);
    strcpy(CFrontend::m_stringDest, buffer);
    strcat(CFrontend::m_stringDest, g_strTopC3d);
    pFile = (GenericFile *)CGenericFileLoader::FindFile(&g_unk0x00589448, CFrontend::m_stringDest, NULL, NULL, 0);
    if (pFile != NULL) {
        g_unk0x00589444 = (int)FUN_004b9380((unsigned int)pFile, (unsigned int)RallyData_FUN_00411060(),
                                           (unsigned int)&g_unk0x00589448);
        if (g_unk0x00589444 != 0) {
            pMesh = *(BYTE **)(g_unk0x00589444 + 0xc);
            *(int *)(g_unk0x00589444 + 0x180) = 0;
            FUN_004b2e40(pMesh, 0);
            *(BYTE *)(g_unk0x00589444 + 0x17c) = 0;
        }
    }
    if (pC3d != NULL) {
        g_unk0x00589440 = (int)FUN_004b9380((unsigned int)pC3d, (unsigned int)RallyData_FUN_00411060(),
                                           (unsigned int)&g_unk0x00589448);
        if (g_unk0x00589440 != 0) {
            pMesh = *(BYTE **)(g_unk0x00589440 + 0xc);
            *(int *)(g_unk0x00589440 + 0x180) = 0;
            FUN_004b2e40(pMesh, 0);
        }
    }
    pFile = (GenericFile *)CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(), g_strTempGro, NULL, NULL, 0);
    if (pFile != NULL) {
        g_unk0x0058943c = (int)FUN_004b9380((unsigned int)pFile, (unsigned int)RallyData_FUN_00411060(),
                                           (unsigned int)FUN_0041f500());
        if (g_unk0x0058943c != 0) {
            pMesh = *(BYTE **)(g_unk0x0058943c + 0xc);
            *(int *)(g_unk0x0058943c + 0x180) = 0;
            FUN_004b2e40(pMesh, 0);
        }
    }
    CGame::RegisterCallback((void *)FUN_0046f030, NULL);
}

int RallyData_FUN_00411060(void);
void Mesh_ResetCloneCount(void);
Mesh *Mesh_CloneInto(Mesh *pSrc, BYTE *pSource);
extern double g_minus65536;

// Loads the stage object list, creates a scene node and clones the mesh of
// every object, then classifies each bounding box as ground or wall.
// match 49%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// The three bounding-box accumulators land in ESI/EDX/ECX instead of the
// original's EDI/ESI/EDX and MSVC merges the "if (v < 0)" phi without the
// original's extra jmp; the code itself is identical.
// FUNCTION: CMR2 0x0046f550
void FUN_0046f550(void)
{
    BYTE *pEntries;
    int i;
    int count;
    BYTE *pEntry;
    int maxX;
    int maxY;
    int maxZ;
    int mesh;
    float *pFloats;

    Mesh_ResetCloneCount();
    pEntry = (BYTE *)g_unk0x005894e0 + 0x114;
    do {
        SceneNode *pNode = SceneNode_Create((SceneNode *)RallyData_FUN_00411060());
        *(int *)(pEntry - 0x110) = (int)pNode;
        *(int *)((BYTE *)pNode + 0x178) = 3;
        *(int *)pEntry = -0x10000;
        pEntry += 0x128;
    } while ((int)pEntry < (int)((BYTE *)g_unk0x005894e0 + 0x114 + 40 * 0x128));

    count = FUN_00471bd0(&pEntries);
    g_unk0x0058c92c = CFileBuffer::AllocateLockedBuffer(count * 4);
    i = 0;
    if (count > 0) {
        do {
            i++;
            ((int *)g_unk0x0058c92c)[i - 1] = 0;
        } while (i < count);
    }
    g_unk0x0058c928 = 0;
    if (count > 0) {
        g_unk0x0058c928 = CFileBuffer::AllocateLockedBuffer(count);
        i = 0;
        g_unk0x0058c320 = 0;
        if (count > 0) {
            do {
                mesh = *(int *)(*(int *)(pEntries + i * 8) + 0xc);
                if ((int)g_unk0x0058c320 < count) {
                    ((BYTE *)g_unk0x0058c928)[i] = (BYTE)g_unk0x0058c320;
                    mesh = (int)Mesh_CloneInto((Mesh *)mesh, (BYTE *)*(int *)(pEntries + i * 8));
                    ((int *)g_unk0x0058c92c)[(BYTE)g_unk0x0058c320] = mesh;
                    if (((int *)g_unk0x0058c92c)[(BYTE)g_unk0x0058c320] == 0)
                        ((BYTE *)g_unk0x0058c928)[i] = 0;
                    else
                        g_unk0x0058c320++;
                } else {
                    ((BYTE *)g_unk0x0058c928)[i] = 0xff;
                }
                i++;
            } while (i < count);
        }
    }
    g_unk0x0058c930 = 0;
    if (g_unk0x0058c320 > 0)
        g_unk0x0058c930 = CFileBuffer::AllocateLockedBuffer((g_unk0x0058c320 & 0xff) << 2);
    i = 0;
    if (g_unk0x0058c320 > 0) {
        do {
            int pObject = ((int *)g_unk0x0058c92c)[i];
            int n;
            int x;
            int y;
            int z;
            maxZ = 0;
            maxY = 0;
            maxX = 0;
            n = *(int *)((BYTE *)pObject + 0x10);
            if (n > 0) {
                pFloats = *(float **)((BYTE *)pObject + 0xc);
                do {
                    x = (int)(__int64)(pFloats[0] * CGraphics::m_65536);
                    if (x < 0)
                        x = (int)(__int64)(pFloats[0] * g_minus65536);
                    y = (int)(__int64)(pFloats[1] * CGraphics::m_65536);
                    if (y < 0)
                        y = (int)(__int64)(pFloats[1] * g_minus65536);
                    z = (int)(__int64)(pFloats[2] * CGraphics::m_65536);
                    if (z < 0)
                        z = (int)(__int64)(pFloats[2] * g_minus65536);
                    if (x > maxX)
                        maxX = x;
                    if (y > maxY)
                        maxY = y;
                    if (z > maxZ)
                        maxZ = z;
                    pFloats += 0xc;
                    n--;
                } while (n != 0);
            }
            if (FixDiv(maxZ, maxX) < 0x4ccc)
                ((int *)g_unk0x0058c930)[i] = 0;
            else if (FixDiv(maxX, maxY) < 0x4ccc)
                ((int *)g_unk0x0058c930)[i] = 1;
            else
                ((int *)g_unk0x0058c930)[i] = 0;
            i++;
        } while (i < (int)(g_unk0x0058c320 & 0xff));
    }
    CGame::RegisterCallback((void *)FUN_0046f500, NULL);
}

// FUNCTION: CMR2 0x00471bd0
unsigned int FUN_00471bd0(BYTE **pOut)
{
    *pOut = g_unk0x0058c94c;
    return g_unk0x0058ca6c;
}

// FUNCTION: CMR2 0x004728b0
void FUN_004728b0(void)
{
    g_unk0x0058cf68 = 1;
}

// FUNCTION: CMR2 0x004728c0
void FUN_004728c0(void)
{
    g_unk0x0058cf68 = 0;
}

// FUNCTION: CMR2 0x004728d0
int FUN_004728d0(void)
{
    return g_unk0x0058cf68;
}

// Returns 1 when a match of the current knockout round is undecided or one of
// its drivers is not in the table.
// FUNCTION: CMR2 0x004728e0
int FUN_004728e0(void)
{
    unsigned int *pState;
    KnockoutMatch *pMatch = NULL;
    int count = 0;
    int i;

    pState = RallyData_GetChampionshipState();
    switch ((*pState >> 3) & 7) {
    case 1:
        count = 8;
        pMatch = (KnockoutMatch *)(pState + 0x16);
        break;
    case 2:
        count = 4;
        pMatch = (KnockoutMatch *)(pState + 10);
        break;
    case 3:
        count = 2;
        pMatch = (KnockoutMatch *)(pState + 4);
        break;
    case 4:
        count = 1;
        pMatch = (KnockoutMatch *)(pState + 1);
    }
    for (i = 0; i < count; i++, pMatch++) {
        if ((RallyData_FUN_00408500((BYTE)(pMatch->flags & 0x1f)) == -1 && (pMatch->flags & 0x400) == 0) ||
            (RallyData_FUN_00408500((BYTE)((pMatch->flags >> 5) & 0x1f)) == -1 && (pMatch->flags & 0x400) == 0))
            return 1;
        if (FUN_00472990(pMatch))
            return 1;
    }
    return 0;
}

// Whether the human player lost the knockout match (the winner is not a human
// driver): bit 11 means the second driver won, bit 12 the first one.
// FUNCTION: CMR2 0x00472990
int FUN_00472990(KnockoutMatch *pMatch)
{
    if (RallyData_FUN_00408500(pMatch->flags & 0x1f) == -1 && (pMatch->flags & 0x1800) == 0x800)
        return 1;
    if (RallyData_FUN_00408500((pMatch->flags >> 5) & 0x1f) == -1 && (pMatch->flags & 0x1800) == 0x1000)
        return 1;
    return 0;
}

// FUNCTION: CMR2 0x00472ca0
void FUN_00472ca0(void)
{
    g_unk0x0058cf7c = 0;
}

// Sets the ground material of the two cars from the current championship
// state (the knockout bracket entry selected by the state flags).
// FUNCTION: CMR2 0x00472cb0
void FUN_00472cb0(void)
{
    unsigned int *pState = RallyData_GetChampionshipState();
    BYTE value;
    int i;

    switch ((*pState >> 3) & 7) {
    case 1:
        if (RallyData_FUN_00408500((BYTE)pState[((*pState >> 0xc) & 0xf) * 3 + 0x16] & 0x1f) == -1 &&
            RallyData_FUN_00408500((BYTE)(pState[((*pState >> 0xc) & 0xf) * 3 + 0x16] >> 5) & 0x1f) == -1)
            goto fail;
        value = 1;
        break;
    case 2:
        if (RallyData_FUN_00408500((BYTE)pState[((*pState >> 0xc) & 0xf) * 3 + 10] & 0x1f) == -1 &&
            RallyData_FUN_00408500((BYTE)(pState[((*pState >> 0xc) & 0xf) * 3 + 10] >> 5) & 0x1f) == -1)
            goto fail;
        value = 1;
        break;
    case 3:
        if (RallyData_FUN_00408500((BYTE)pState[((*pState >> 0xc) & 0xf) * 3 + 4] & 0x1f) == -1 &&
            RallyData_FUN_00408500((BYTE)(pState[((*pState >> 0xc) & 0xf) * 3 + 4] >> 5) & 0x1f) == -1)
            goto fail;
        value = 1;
        break;
    case 4:
        if (RallyData_FUN_00408500((BYTE)pState[1] & 0x1f) == -1 &&
            RallyData_FUN_00408500((BYTE)(pState[1] >> 5) & 0x1f) == -1)
            goto fail;
        value = 1;
        break;
    default:
    fail:
        value = 2;
        break;
    }
    RallyData_FUN_00407500(value);
    for (i = 0; i < 2; i++) {
        FUN_0042b720(i, (BYTE)FUN_0040bbc0((unsigned short)i));
        if (i >= (int)(RallyDataState() & 0xff))
            FUN_0042b720(i, -1);
    }
}

// Stage fade timers and race-state flags of the in-race state machine
// (0x472e00). Each "*" object is registered with the timer manager and its
// first byte is the timer slot.
// Magic value handed to FUN_004283e0 as opaque data, not an address.
// GLOBAL: CMR2 0x0051c9a8
int g_unk0x0051c9a8 = 0xacb49c;
// GLOBAL: CMR2 0x0058ca80
BYTE g_unk0x0058ca80;
// GLOBAL: CMR2 0x0058ca84
int g_unk0x0058ca84;
// The five fade-timer objects below live inside the 0x58ca90 block declared in
// StageUI.cpp, so they are addressed as offsets of it (a separate declaration
// would overlap it).
extern BYTE g_unk0x0058ca90[];
#define g_unk0x0058cc70 (g_unk0x0058ca90[0x1e0])
#define g_unk0x0058cc74 (*(int *)(g_unk0x0058ca90 + 0x1e4))
#define g_unk0x0058ce58 (*(int *)(g_unk0x0058ca90 + 0x3c8))
#define g_unk0x0058ce5c (*(unsigned int *)(g_unk0x0058ca90 + 0x3cc))
#define g_unk0x0058cf60 (g_unk0x0058ca90[0x4d0])

extern BYTE *g_unk0x0058ca88;
extern BYTE g_unk0x0058ca8c[4];
extern int g_unk0x0058cf64;
extern int g_unk0x0058cf7c;

typedef void (*FadeCallback)(BYTE index);

extern unsigned int g_unk0x0058cf74;
extern unsigned int g_unk0x0058cf78;
extern int g_unk0x0058cf70;

void FUN_00478c40(void);
void FUN_00418ee0(void);
void FUN_00418780(void);
void FUN_004284d0(unsigned int player, int check);
void FUN_004285b0(unsigned int player, int t, int check);
void FUN_00473470(void);
BYTE FUN_004bc0c0(BYTE *p);
int Timer_GetValue(BYTE index);
void FUN_004bc290(BYTE *p, int, int, int, int, int, BYTE);
void FUN_004bc440(void);
void FUN_004bc470(BYTE *p);
unsigned int *RallyData_GetRoundEntry(void);
int FUN_00428740(BYTE index);
int FUN_00473680(unsigned int *p);
int FUN_00473310(void);
int FUN_00473290(void);
void FUN_0041b310(void);
int FUN_0041b320(void);
void FUN_00455470(int);
void FUN_00472a30(void);
BOOL FUN_0046c500(void);
void FUN_0041c260(void);
void FUN_004283e0(BYTE index, FadeCallback pfnDone, int param3, int param4, int param5, char force);
void FUN_0041f2a0(void);
void RallyData_FUN_004070c0(void);
void FUN_00473360(void);
struct Menu;
void FUN_004734f0(Menu *pMenu);
void FUN_00473540(BYTE index);

// Per-frame step of the in-race state machine: waits for the stage objects of
// the round, runs the state transitions and drives the fade in/out of the
// scene.
// FUNCTION: CMR2 0x00472e00
void FUN_00472e00(BYTE *param_1, unsigned int param_2)
{
    unsigned int *pState;
    unsigned int *pEntry;
    unsigned int state;
    int i;
    pState = RallyData_GetChampionshipState();
    FUN_00478c40();
    FUN_00418ee0();
    FUN_00418780();
    g_unk0x0058ca88 = param_1;
    g_unk0x0058ca84 = param_2 & 0xff;
    for (i = 0; i < 2; i++) {
        FUN_004284d0(i, 0);
        FUN_004285b0(i, 0x10000, 0);
    }
    if (FUN_00428740(0) != 0)
        return;
    if (g_unk0x0058cf6c != 0)
        FUN_00473470();
    if (FUN_004bc0c0(&g_unk0x0058ca80))
        g_unk0x0058cc74 = Timer_GetValue(g_unk0x0058ca80);
    if (FUN_004bc0c0(&g_unk0x0058cf60))
        g_unk0x0058ce58 = Timer_GetValue(g_unk0x0058cf60);
    else
        g_unk0x0058ce58 = 0;

    switch (g_unk0x0058cf7c) {
    case 0:
        FUN_004bc440();
        g_unk0x0058cc74 = 0;
        if ((*pState & 0x38) == 8) {
            FUN_004bc290(&g_unk0x0058ca80, 2, 0xd, 0, 0, 0x10000, 0);
            FUN_004bc290(&g_unk0x0058cf60, 2, 7, 0, 0, 0x10000, 0);
        }
        *pState = *pState & 0xff1fffff;
        FUN_00473360();
        g_unk0x0058cf7c = 1;
        for (i = 0; i < 2; i++) {
            if (i >= (int)(RallyDataState() & 0xff))
                FUN_0042b720(i, -1);
        }
        g_unk0x0058cf64 = 0;
        return;
    case 1:
        if (g_unk0x0058cf64 != 0) {
            FUN_00455470(1);
            g_unk0x0058cf64 = 0;
            g_unk0x0058cf7c = 2;
            return;
        }
        break;
    case 2:
        pEntry = RallyData_GetRoundEntry();
        if (FUN_00473680(pEntry) != 0) {
            g_unk0x0058cf7c = 5;
            FUN_00472a30();
            g_unk0x0058cf78 = 0;
            g_unk0x0058cf74 = 0;
        }
        if (g_unk0x0058cf64 != 0) {
            FUN_00472cb0();
            FUN_0046c500();
            FUN_0041c260();
            if (FUN_00473310() != 0) {
                g_unk0x0058cf7c = 3;
                FUN_00472a30();
                g_unk0x0058cf78 = 0;
                g_unk0x0058cf74 = 0;
                g_unk0x0058cf64 = 0;
                return;
            } else {
                g_unk0x0058ce5c = (unsigned int)RallyData_GetRoundEntry();
                g_unk0x0058cf64 = 0;
                g_unk0x0058cf7c = 4;
                return;
            }
        }
        break;
    case 3:
        if (g_unk0x0058cf64 != 0) {
            g_unk0x0058cf7c = 5;
            FUN_004bc290(&g_unk0x0058cf60, 2, 7, 0, 0, 0x10000, 0);
            g_unk0x0058cf64 = 0;
            return;
        }
        break;
    case 4:
        if (FUN_0041b320() == 0) {
            FUN_0041b310();
            g_unk0x0058cf64 = 0;
            return;
        }
        FUN_004283e0(0, (FadeCallback)FUN_00473540, 1, 0, g_unk0x0051c9a8, 0);
        if ((char)RallyDataState() == 2)
            FUN_004283e0(1, NULL, 1, 0, g_unk0x0051c9a8, 0);
        break;
    case 5:
        if ((*pState & 0x400000) == 0) {
            g_unk0x0058cf64 = 0;
            g_unk0x0058cf7c = 2;
            return;
        }
        if ((*pState & 0x38) == 0x20) {
            if (g_unk0x0058cf64 != 0) {
                g_unk0x0058cf64 = 0;
                g_unk0x0058cf7c = 7;
                return;
            }
        } else if (g_unk0x0058cf64 != 0) {
            g_unk0x0058cc74 = 0x10000;
            FUN_004bc290(&g_unk0x0058cc70, 2, 0xd, 0, 0, 0x10000, 0);
            g_unk0x0058cf64 = 0;
            g_unk0x0058cf7c = 6;
            return;
        }
        break;
    case 6:
        if (FUN_004bc0c0(&g_unk0x0058cc70)) {
            g_unk0x0058cc74 = Timer_GetValue(g_unk0x0058cc70);
            g_unk0x0058cf64 = 0;
            return;
        }
        if (!FUN_004bc0c0(g_unk0x0058ca8c)) {
            FUN_004bc470(&g_unk0x0058cc70);
            g_unk0x0058cc74 = 0;
            FUN_004bc290(g_unk0x0058ca8c, 2, 0xd, 0, 0, 0x10000, 1);
            g_unk0x0058cf64 = 0;
            return;
        }
        g_unk0x0058cc74 = Timer_GetValue(g_unk0x0058ca8c[0]);
        if (!FUN_004bc0c0(g_unk0x0058ca8c))
            g_unk0x0058cc74 = 0;
        if (g_unk0x0058cf64 != 0 || !FUN_004bc0c0(g_unk0x0058ca8c)) {
            FUN_004bc440();
            g_unk0x0058cc74 = 0;
            g_unk0x0058ce58 = 0;
            g_unk0x0058cf70 = FUN_00473290();
            state = *pState;
            *pState = (((state & 0xfffffff8) + 8 ^ state) & 0x38 ^ state) & 0xffbf0fff | 0x200000;
            RallyData_FUN_004070c0();
            FUN_0041f2a0();
            i = 0;
            if (*g_unk0x0058ca88 > 0) {
                do {
                    CGame::FUN_0049c1c0((Unk0049c2c0 *)g_unk0x0058ca88, i, 1, 2);
                    i++;
                } while (i < (int)*g_unk0x0058ca88);
            }
            g_unk0x0058cf7c = 0;
            g_unk0x0058cf64 = 0;
            return;
        }
        break;
    case 7:
        FUN_004283e0(0, (FadeCallback)FUN_004734f0, 1, 0, g_unk0x0051c9a8, 0);
        if ((char)RallyDataState() == 2)
            FUN_004283e0(1, NULL, 1, 0, g_unk0x0051c9a8, 0);
        break;
    }
    g_unk0x0058cf64 = 0;
}

// FUNCTION: CMR2 0x00473680
int FUN_00473680(unsigned int *p)
{
    if ((*p & 0x1f) != 0x1f && (*p & 0x3e0) != 0x3e0)
        return 0;
    return 1;
}

int *RallyData_FUN_00407f20(int index);

// Name of the driver on the given side (0 first, 1 second) of a knockout
// match: the player's name, the AI name, or "" for an empty slot.
// match 55%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004736b0
char *FUN_004736b0(KnockoutMatch *pMatch, int side)
{
    unsigned int driver;

    if (side == 0) {
        if ((pMatch->flags & 0x1f) == 0x1f)
            return CMain::m_logFileBlankLine;
        if (RallyData_FUN_00408500(pMatch->flags & 0x1f) != -1) {
            sprintf(CFrontend::m_stringDest, CAIHelper::GetNameForID(RallyData_FUN_00408500(pMatch->flags & 0x1f)));
            return CFrontend::m_stringDest;
        }
        driver = pMatch->flags;
    } else {
        if (side != 1)
            return CFrontend::m_stringDest;
        if ((pMatch->flags & 0x3e0) == 0x3e0)
            return CMain::m_logFileBlankLine;
        if (RallyData_FUN_00408500((pMatch->flags >> 5) & 0x1f) != -1) {
            sprintf(CFrontend::m_stringDest,
                    CAIHelper::GetNameForID(RallyData_FUN_00408500((pMatch->flags >> 5) & 0x1f)));
            return CFrontend::m_stringDest;
        }
        driver = pMatch->flags >> 5;
    }
    sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue, (char *)RallyData_GetRecord(driver & 0x1f));
    return CFrontend::m_stringDest;
}

// Whether the given side of the match is the human player.
// FUNCTION: CMR2 0x00473790
int FUN_00473790(KnockoutMatch *pMatch, int side)
{
    unsigned int driver;

    if (side == 0)
        driver = pMatch->flags;
    else
        driver = pMatch->flags >> 5;
    driver &= 0x1f;
    if (driver == 0x1f)
        return 0;
    return RallyData_FUN_00408500(driver) == -1;
}

// Side of the match to show: the human player's side when it is param2,
// otherwise the other side.
// FUNCTION: CMR2 0x004737d0
int FUN_004737d0(KnockoutMatch *pMatch, int param2)
{
    int slot = pMatch->flags & 0x1f;

    if (slot != 0x1f && RallyData_FUN_00408500(slot) == -1)
        return param2 != 0;
    return param2 == 0;
}

// Same as FUN_004736b0 with the car names of the AI drivers.
// match 55%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00473810
char *FUN_00473810(KnockoutMatch *pMatch, int side)
{
    unsigned int driver;

    if (side == 0) {
        if ((pMatch->flags & 0x1f) == 0x1f)
            return CMain::m_logFileBlankLine;
        if (RallyData_FUN_00408500(pMatch->flags & 0x1f) != -1) {
            sprintf(CFrontend::m_stringDest, (char *)RallyData_FUN_00407f20(RallyData_FUN_00408500(pMatch->flags & 0x1f)));
            return CFrontend::m_stringDest;
        }
        driver = pMatch->flags;
    } else {
        if (side != 1)
            return CFrontend::m_stringDest;
        if ((pMatch->flags & 0x3e0) == 0x3e0)
            return CMain::m_logFileBlankLine;
        if (RallyData_FUN_00408500((pMatch->flags >> 5) & 0x1f) != -1) {
            sprintf(CFrontend::m_stringDest,
                    (char *)RallyData_FUN_00407f20(RallyData_FUN_00408500((pMatch->flags >> 5) & 0x1f)));
            return CFrontend::m_stringDest;
        }
        driver = pMatch->flags >> 5;
    }
    sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue, (char *)RallyData_GetRecord(driver & 0x1f));
    return CFrontend::m_stringDest;
}

extern int g_unk0x0058cf64;
// GLOBAL: CMR2 0x0058cf74
unsigned int g_unk0x0058cf74;
// GLOBAL: CMR2 0x0058cf78
unsigned int g_unk0x0058cf78;

extern char g_minSecMSECFormatString[];

// Formats one of the two lap time fields as "%02d:%02d.%02d", printing the
// identical placeholder while the value eases towards its target.
// match 68%: same logic; MSVC keeps the two eased values in different
// registers and spills one extra
// match 68%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004752f0
char *FUN_004752f0(int *p, int index, int mode)
{
    unsigned int current1;
    unsigned int current2;
    int time1;
    int time2;

    time1 = p[1];
    time2 = p[2];
    if (mode != 0) {
        current1 = g_unk0x0058cf74;
        if (current1 <= (unsigned)(time1 - 0x27))
            current1 += 0x27;
        else
            current1 = time1;
        g_unk0x0058cf74 = current1;
        current2 = g_unk0x0058cf78;
        if (current2 <= (unsigned)(time2 - 0x27))
            current2 += 0x27;
        else
            current2 = time2;
        g_unk0x0058cf78 = current2;
        if (current1 == (unsigned)time1 && current2 == (unsigned)time2)
            g_unk0x0058cf64 = 1;
    }
    if (index == 0) {
        if ((p[0] & 0x1f) == 0x1f)
            return CMain::m_logFileBlankLine;
        RallyData_FUN_00408500((BYTE)(p[0] & 0x1f));
        sprintf(CFrontend::m_stringDest, g_minSecMSECFormatString, time1 / 6000, (time1 % 6000) / 100,
                time1 % 100);
    } else if (index == 1) {
        if ((p[0] & 0x3e0) == 0x3e0)
            return CMain::m_logFileBlankLine;
        RallyData_FUN_00408500((BYTE)((p[0] >> 5) & 0x1f));
        sprintf(CFrontend::m_stringDest, g_minSecMSECFormatString, time2 / 6000, (time2 % 6000) / 100,
                time2 % 100);
    }
    return CFrontend::m_stringDest;
}

// Builds the in-race menu of two items (returned by FUN_00475f70); both
// actions are forwarded to CGame (0x49c070 / 0x49c080).
void FUN_0049c070(Menu *pMenu, int param);
void FUN_0049c080(Menu *pMenu, int param);
void FUN_0049bcb0(Menu *pMenu);
// FUNCTION: CMR2 0x00475f00
void FUN_00475f00(void)
{
    Menu_Init((Menu *)g_unk0x0058cf80, 0, -1, 0, NULL, NULL, 1, 0, 1);
    Menu_AddItemType4((Menu *)g_unk0x0058cf80, 0, 0xf4, (int)FUN_0049c070, -1);
    Menu_AddItemType4((Menu *)g_unk0x0058cf80, 0, 0xf5, (int)FUN_0049c080, -1);
    Menu_SetCallbacks((Menu *)g_unk0x0058cf80, NULL, NULL, (MenuCallback)FUN_0049bcb0, NULL);
    Menu_ValidateCursor((Menu *)g_unk0x0058cf80, 0);
}

// FUNCTION: CMR2 0x00475f70
BYTE *FUN_00475f70(void)
{
    return g_unk0x0058cf80;
}

// Updates one stage object's state byte and re-syncs its scene node with the
// given source matrix; when the node ends up in another sector it is detached
// and released from the scene again.
// FUNCTION: CMR2 0x00476410
void FUN_00476410(BYTE *p, int *src, int unused, BYTE value)
{
    int index;
    FixVector pos;

    g_unk0x0058d4d0[*p] = value;
    FUN_00477850((int)p, src);
    if (g_unk0x0058d49c[p[2]] != NULL) {
        SceneNode_Unused((SceneNode *)g_unk0x0058d49c[p[2]]);
        pos.x = ((SceneNode *)g_unk0x0058d49c[p[2]])->world.position.x;
        pos.y = ((SceneNode *)g_unk0x0058d49c[p[2]])->world.position.y;
        pos.z = ((SceneNode *)g_unk0x0058d49c[p[2]])->world.position.z;
        index = (short)Sector_FromPosition(&pos);
        if (index != ((SceneNode *)g_unk0x0058d49c[p[2]])->sector) {
            FUN_004b8b10((SceneNode *)g_unk0x0058d49c[p[2]]);
            SceneNode_Unused((SceneNode *)g_unk0x0058d49c[p[2]]);
        }
    }
}

// FUNCTION: CMR2 0x004764e0
void FUN_004764e0(BYTE *p)
{
    g_unk0x0058d3b0[p[2]] = 1;
}

// FUNCTION: CMR2 0x00476520
int FUN_00476520(BYTE index)
{
    return g_unk0x0058d6a8[index];
}

// Caches, for a car, whether its class is special and pointers into its
// timing record.
// match 62%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00476540
void FUN_00476540(int index)
{
    Car *pCar = Car_Get(index);
    char type = pCar->field_0xb1b[0];
    int p;
    int *pRow;

    if (type == 8 || type == 7 || type == 9 || type == 13)
        g_unk0x0058d2f0[index] = 1;
    else
        g_unk0x0058d2f0[index] = 0;
    p = FUN_00457e10((BYTE *)pCar, 5);
    pRow = (int *)(g_unk0x0058d4f0 + index * 0x1c);
    pRow[0] = p;
    pRow[1] = p + 8;
    pRow[2] = p + 0xc;
    pRow[3] = p + 0x18;
    pRow[4] = p + 0x1c;
    pRow[5] = p + 0x24;
    pRow[6] = p + 0x2c;
}

// Draws a stage box: a filled rectangle, its one pixel outline and, optionally,
// the championship round box texture scaled to the rectangle.
// FUNCTION: CMR2 0x00475740
void FUN_00475740(short *pRect, BYTE *pColour, BYTE *pEdgeColour, int drawTexture)
{
    short edge[4];
    SpriteRect dest;

    if (pColour != NULL)
        Sprite_FillRect((int)g_pGraphics + 0x150, pRect, pColour, 2);
    if (pEdgeColour != NULL) {
        edge[0] = pRect[0];
        edge[1] = pRect[1];
        edge[2] = pRect[2];
        edge[3] = 1;
        Sprite_FillRect((int)g_pGraphics + 0x150, edge, pEdgeColour, 2);
        edge[0] = (short)(pRect[0] + pRect[2]);
        edge[1] = pRect[1];
        edge[2] = 1;
        edge[3] = (short)(pRect[3] + 1);
        Sprite_FillRect((int)g_pGraphics + 0x150, edge, pEdgeColour, 2);
        edge[0] = pRect[0];
        edge[1] = (short)(pRect[1] + pRect[3]);
        edge[2] = pRect[2];
        edge[3] = 1;
        Sprite_FillRect((int)g_pGraphics + 0x150, edge, pEdgeColour, 2);
        edge[0] = pRect[0];
        edge[1] = pRect[1];
        edge[2] = 1;
        edge[3] = pRect[3];
        Sprite_FillRect((int)g_pGraphics + 0x150, edge, pEdgeColour, 2);
    }
    if (drawTexture != 0) {
        dest.x = pRect[0];
        dest.y = pRect[1];
        // The texture quad is scaled to the rectangle and to the reference
        // resolution (640x480) it was authored for.
        dest.w = (short)(((int)((Texture *)FUN_00405600())->width * (int)pRect[2] * (int)g_pGraphics->resX / 0x280) /
                          ((int)g_pGraphics->resX * 0x55 / 0x280));
        dest.h = (short)(((int)((Texture *)FUN_00405600())->height * (int)pRect[3] * (int)g_pGraphics->resY / 0x1e0) /
                          ((int)g_pGraphics->resY * 0x26 / 0x1e0));
        Sprite_Queue((SpriteRect *)(FUN_00405600() + 0x11c), &dest, (Texture *)FUN_00405600(), 2, 0, 0, 0,
                     pEdgeColour, 8);
    }
}

// Blends a car's stage object transform: rotates its mount nodes by the car
// heading and interpolates the blended node's matrix between the reference
// node and the rotated mount using the body fade factor.
// FUNCTION: CMR2 0x00476640
void FUN_00476640(int car)
{
    // Per car fade curve (16.16), sampled with index = 12 * fade.
    int fadeCurve[13] = {0, 0x51e, 0xccc, 0x1999, 0x3333, 0x6666, 0x9999, 0xcccc,
                         0xe666, 0xf333, 0xfae1, 0x10000, 0x10000};
    Car *pCar = Car_Get(car);
    SceneNode *pRef = *(SceneNode **)(g_stageBlock + 0x288 + car * 0x1c);
    SceneNode *pBlend = *(SceneNode **)(g_stageBlock + 0x28c + car * 0x1c);
    SceneNode *pMid = *(SceneNode **)(g_stageBlock + 0x290 + car * 0x1c);
    SceneNode *pRot = *(SceneNode **)(g_stageBlock + 0x298 + car * 0x1c);
    short angle = -pCar->heading;
    FixVector axis;
    FixMatrix rot;
    FixMatrix combined;
    FixMatrix original;
    int posX;
    int posY;
    int posZ;
    int fade;

    SceneNode_SetRotation(pRot, *(FixAngles **)(g_stageBlock + 0x260 + car * 0x1c));
    axis = pRot->current.right;
    FixMatrix_FromAxisAngle(&rot, &axis, angle);

    // Rotate the node without touching its translation.
    posX = pRot->current.position.x;
    posY = pRot->current.position.y;
    posZ = pRot->current.position.z;
    pRot->current.position.x = 0;
    pRot->current.position.y = 0;
    pRot->current.position.z = 0;
    FixMatrix_Multiply(&pRot->current, &pRot->current, &rot);
    pRot->current.position.x = posX;
    pRot->current.position.y = posY;
    pRot->current.position.z = posZ;

    SceneNode_SetRotation(pMid, *(FixAngles **)(g_stageBlock + 0x260 + car * 0x1c));
    FixMatrix_Multiply(&combined, &pMid->current, &rot);
    combined.position.x += posX;
    combined.position.y += posY;
    combined.position.z += posZ;

    original = pRef->current;

    fade = FUN_00476850(car, (int)pCar);
    FixMatrix_Interpolate(&pBlend->current, &combined, &original, 0, 0,
                          fadeCurve[FixMulShift32(0xC0000, fade)], 1);
}

// FUNCTION: CMR2 0x00477a90
void FUN_00477a90(void)
{
    memset(g_unk0x0058d6b0, 0xff, 7 * 4);
    memset(g_unk0x0058d2a0, 0xff, 12 * 4);
}

// FUNCTION: CMR2 0x00478170
void FUN_00478170(int index)
{
    g_unk0x0058da30[index] = 0;
}

// FUNCTION: CMR2 0x00478190
void FUN_00478190(int index, int seconds)
{
    g_unk0x0058da30[index] += seconds * 100;
}

// FUNCTION: CMR2 0x004781c0
int FUN_004781c0(int index)
{
    return g_unk0x0058da30[index];
}

// FUNCTION: CMR2 0x004789b0
void FUN_004789b0(BYTE *pCar)
{
    *(int *)(pCar + 0xa74) = *(int *)(pCar + 0xa78);
}

// Second group (0x4805f0-0x49e940)


extern void *g_unk0x00592734;
void FUN_0046f4c0(int *pOut);
void FUN_004ae410(BYTE a, BYTE b, int c, int d);

// GLOBAL: CMR2 0x005909bc
int g_unk0x005909bc;
// GLOBAL: CMR2 0x005909c8
FixVector g_unk0x005909c8[4];
// GLOBAL: CMR2 0x00590b7c
BYTE *g_unk0x00590b7c[4][8];
// GLOBAL: CMR2 0x00590bfc
BYTE g_unk0x00590bfc;
// GLOBAL: CMR2 0x00590bfd
BYTE g_unk0x00590bfd;
// GLOBAL: CMR2 0x00590c24
BYTE g_unk0x00590c24[4][8];
// GLOBAL: CMR2 0x00590c44
int g_unk0x00590c44;
// GLOBAL: CMR2 0x00590c48
int g_unk0x00590c48;
// GLOBAL: CMR2 0x00590c4c
int g_unk0x00590c4c;
// GLOBAL: CMR2 0x00590c50
int g_unk0x00590c50;
// GLOBAL: CMR2 0x00590c60
BYTE g_unk0x00590c60[4];
// GLOBAL: CMR2 0x00590d70
int g_unk0x00590d70;
// GLOBAL: CMR2 0x00590db0
int g_unk0x00590db0[64];
// GLOBAL: CMR2 0x00590ed0
BYTE g_unk0x00590ed0[8][0x98];
// GLOBAL: CMR2 0x00591390
int g_unk0x00591390;
// GLOBAL: CMR2 0x005913d8
int g_unk0x005913d8;
// GLOBAL: CMR2 0x005913dc
BYTE g_unk0x005913dc[4];
// GLOBAL: CMR2 0x005913f8
BYTE g_unk0x005913f8[8][8];
// GLOBAL: CMR2 0x0059146c
int g_unk0x0059146c[8];
// GLOBAL: CMR2 0x005914c8
FixVector g_unk0x005914c8;
// GLOBAL: CMR2 0x005916a0
FixVector g_unk0x005916a0[4];
// GLOBAL: CMR2 0x005916f0
int g_unk0x005916f0[4];
// GLOBAL: CMR2 0x00591868
FixVector g_unk0x00591868[4];
// GLOBAL: CMR2 0x00591898
FixVector g_unk0x00591898[4];
// GLOBAL: CMR2 0x00591740
int g_unk0x00591740[4];
// GLOBAL: CMR2 0x00591750
BYTE *g_unk0x00591750;
// GLOBAL: CMR2 0x005918c8
int g_unk0x005918c8;
// GLOBAL: CMR2 0x005920f0
BYTE *g_unk0x005920f0;
// GLOBAL: CMR2 0x00592114
FixVector g_unk0x00592114;
// GLOBAL: CMR2 0x00592128
int g_unk0x00592128;
// GLOBAL: CMR2 0x0059212c
int g_unk0x0059212c;
// GLOBAL: CMR2 0x00592130
int g_unk0x00592130;
// GLOBAL: CMR2 0x00592134
int g_unk0x00592134;

// GLOBAL: CMR2 0x00590af8
void *g_unk0x00590af8;
// GLOBAL: CMR2 0x00590afc
BYTE g_unk0x00590afc;
// GLOBAL: CMR2 0x00590b00
int g_unk0x00590b00;
// GLOBAL: CMR2 0x00590b04
void *g_unk0x00590b04;
// GLOBAL: CMR2 0x00590b08
void *g_unk0x00590b08;
// GLOBAL: CMR2 0x00590b0c
void **g_unk0x00590b0c;

// GLOBAL: CMR2 0x0051ea20
char g_strMenuSoundNames[5][7] = {"move", "select", "back", "error", "toggle"};

extern char g_strMenuSoundFormat[24];
extern DWORD g_unk0x0058dc58;

// Loads the five menu sounds from the common frontend archive, for the stage
// menus. Returns 0 if any sample failed to load.
// FUNCTION: CMR2 0x00478b80
BYTE FUN_00478b80(void)
{
    char *name;
    BYTE result;

    result = 1;
    g_unk0x0058dc58 = 0;
    name = &g_strMenuSoundNames[0][0];
    do {
        sprintf(CFrontend::m_stringDest, g_strMenuSoundFormat, CInstallInfo::GetGameCDPath(), name);
        if (Sound_LoadSample(CFrontend::m_stringDest, 0, (GenericFile *)StageTiming_GetStageFile0()) == 0)
            result = 0;
        name += 7;
    } while ((int)name < (int)&g_strMenuSoundNames[5][0]);
    return result;
}

// Releases the vehicle files loaded by 0x47e4d0 (registered callback).
// FUNCTION: CMR2 0x0047ea20
BYTE FUN_0047ea20(void)
{
    int i;

    if (g_unk0x00590af8 != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x00590af8);
        g_unk0x00590af8 = NULL;
    }
    if (g_unk0x00590b04 != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x00590b04);
        g_unk0x00590b04 = NULL;
    }
    if (g_unk0x00590b08 != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x00590b08);
        g_unk0x00590b08 = NULL;
    }
    if (g_unk0x00590b0c != NULL) {
        for (i = 0; i < 3; i++) {
            if (g_unk0x00590b0c[i] != NULL) {
                CFileBuffer::FreeGenericFileBuffer(g_unk0x00590b0c[i]);
                g_unk0x00590b0c[i] = NULL;
            }
        }
        if (g_unk0x00590b0c != NULL) {
            CFileBuffer::FreeGenericFileBuffer(g_unk0x00590b0c);
            g_unk0x00590b0c = NULL;
        }
    }
    g_unk0x00590afc = 0;
    return 1;
}

extern float g_oneOverRandMax;

// Colours picked for the vehicle debris fragments.
// GLOBAL: CMR2 0x0051f4ec
DWORD g_stageDebrisPalette[6] = {
    0xffb34aff, 0xffff5c30, 0xffffff3d,
    0xff4ec4ff, 0xff2368ff, 0xff27ff9e
};

// Starts a vehicle debris effect in the first free slot, including its fragments and sound.
// match 36%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0047fcb0
void StageObject_SpawnDebris(const FixVector *pPosition, const FixVector *pVelocity, unsigned int variant)
{
    BYTE *pPool = (BYTE *)g_unk0x00590af8;
    int slotIndex = -1;
    int count = g_unk0x00590afc;
    int i;
    for (i = 0; i < count; ++i) {
        if (*(int *)(pPool + i * 0x938 + 0x694) == 0) {
            slotIndex = i;
            break;
        }
    }
    if (slotIndex < 0)
        return;

    BYTE *pSlot = pPool + slotIndex * 0x938;
    *(int *)(pSlot + 0x694) = 1;
    memcpy(pSlot, pPosition, sizeof(FixVector));
    memcpy(pSlot + 0x1e0, pPosition, sizeof(FixVector));
    memcpy(pSlot + 0xc, pVelocity, sizeof(FixVector));
    pSlot[0x690] = 0;
    pSlot[0x691] = 0;
    for (i = 0; i < 20; ++i) {
        memcpy(pSlot + 0x18 + i * 12, pPosition, sizeof(FixVector));
        memcpy(pSlot + 0x1f8 + i * 12, pPosition, sizeof(FixVector));
        *(int *)(pSlot + 0x69c + i * 4) = 0;
    }
    *(int *)(pSlot + 0x934) = 0;

    int randomFixed;
    if (variant == 0) {
        *(int *)(pSlot + 0x664) = 0x190000;
    } else {
        randomFixed = (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536);
        *(int *)(pSlot + 0x664) = FixMul(randomFixed, 0xa0000) + 0x50000;
    }
    *(int *)(pSlot + 0x66c) = 0xa3d;
    *(int *)(pSlot + 0x670) = 0x624;

    int colourIndex = rand() % 6;
    pSlot[0x692] = (BYTE)colourIndex;
    *(DWORD *)(pSlot + 0x678) = g_stageDebrisPalette[(BYTE)colourIndex];
    pSlot[0x67b] = 0xff;
    *(DWORD *)(pSlot + 0x67c) = *(DWORD *)(pSlot + 0x678);
    *(DWORD *)(pSlot + 0x680) = *(DWORD *)(pSlot + 0x678);
    colourIndex = rand() % 6;
    *(DWORD *)(pSlot + 0x684) = g_stageDebrisPalette[colourIndex];
    pSlot[0x687] = 0xff;
    colourIndex = rand() % 6;
    *(DWORD *)(pSlot + 0x68c) = g_stageDebrisPalette[colourIndex];
    pSlot[0x68f] = 0xff;
    colourIndex = rand() % 6;
    *(DWORD *)(pSlot + 0x688) = g_stageDebrisPalette[colourIndex];
    pSlot[0x68b] = 0xff;

    randomFixed = (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536);
    if (variant == 0)
        *(int *)(pSlot + 0x698) = randomFixed < 0x8000 ? 1 : 2;
    else
        *(int *)(pSlot + 0x698) = randomFixed < 0x10001 ? 1 : 0;

    if (*(int *)(pSlot + 0x698) == 1) {
        int chance = FixMul((rand() % 9 + 1) * 0x10000, 0x1999);
        randomFixed = (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536);
        if (randomFixed < 0xfd71) {
            *(int *)(pSlot + 0x930) = 0;
        } else {
            *(int *)(pSlot + 0x930) = 1;
            chance = FixMul(chance, 0x20000);
        }
        int enabled = 0;
        int row, column, fragment;
        for (row = 0; row < 3; ++row) {
            for (column = 0; column < 6; ++column) {
                for (fragment = 0; fragment < 4; ++fragment) {
                    int offset = (row * 6 + column) * 4 + fragment;
                    randomFixed = (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536);
                    int on = randomFixed <= chance;
                    *(int *)(pSlot + 0x6ec + offset * 4) = on;
                    enabled += on;
                    randomFixed = (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536);
                    *(int *)(pSlot + 0x80c + offset * 4) = randomFixed >= 0x8001;
                }
            }
        }
        int density = FixDiv(enabled << 16, 0x480000);
        *(int *)(pSlot + 0x668) = FixMul(density, 0xf0000) + 0xa0000;
        *(int *)(pSlot + 0x92c) = 1;
        *(int *)(pSlot + 0x660) = FixMul(0x10000 - density, 0xccc) + 0x11eb;
        if (*(int *)(pSlot + 0x930) != 0) {
            *(int *)(pSlot + 0x674) = *(int *)(pSlot + 0x668);
            *(int *)(pSlot + 0x668) = FixMul(*(int *)(pSlot + 0x668), 0x20000);
        }
    } else {
        memset(pSlot + 0x6ec, 0, 0x48 * 4);
        *(int *)(pSlot + 0x668) = 0x50000;
        *(int *)(pSlot + 0x660) = 0x11eb;
        *(int *)(pSlot + 0x92c) = 0;
    }

    FUN_004b7790((unsigned short)(g_unk0x005909bc + 10), 0xccc, 0x5622, 0, 0, 0);
    randomFixed = (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536);
    if (randomFixed > 0x1999) {
        randomFixed = (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536);
        int volume = FixMul(randomFixed, 0x4000) + 0x4000;
        pSlot[0x693] = (BYTE)FUN_004b7790((unsigned short)(g_unk0x005909bc + rand() % 2),
                                            volume, 0x5622, 0, 0, 0);
    } else {
        pSlot[0x693] = 0xff;
    }
}

extern double g_unk0x00511300;

// Spawns one debris burst at a random entry of the four-way spawn table with a
// random, upward-biased velocity of length 1.5..2.5.
// FUNCTION: CMR2 0x00480380
void FUN_00480380(void)
{
    FixVector velocity;
    FixVector *pPosition;
    unsigned short angle;
    int randomFixed;

    randomFixed = (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536);
    angle = (unsigned short)(__int64)((double)FixMul(randomFixed, 0x1680000) * g_unk0x00511300);
    pPosition = &g_unk0x005909c8[rand() % 4];

    velocity.x = g_sinTable[angle & 0xfff];
    velocity.y = 0;
    velocity.z = g_sinTable[(angle + 0x400) & 0xfff];

    randomFixed = (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536);
    FixVecScale(&velocity, &velocity, randomFixed);
    velocity.y = 0x40000;

    FIX_NORMALIZE_INTO(velocity, velocity)

    randomFixed = (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536);
    FixVecScale(&velocity, &velocity, FixMul(0x10000, randomFixed) + 0x18000);

    StageObject_SpawnDebris(pPosition, &velocity, 0);
}

// FUNCTION: CMR2 0x004805f0
void FUN_004805f0(int value)
{
    g_unk0x005909bc = value;
}

// Interpolates every active debris slot's derived position arrays one step
// toward their targets by `scale` (16.16 fixed point).
// FUNCTION: CMR2 0x00480600
void StageObject_UpdateDebris(int scale)
{
    BYTE *pSlot;
    FixVector delta;
    int *p;
    int *q;
    int *d;
    int index;
    int offset;
    int count;
    int row;
    int column;

    index = 0;
    if (g_unk0x00590afc == 0)
        return;
    offset = 0;
    do {
        pSlot = (BYTE *)g_unk0x00590af8 + offset;
        if (*(int *)(pSlot + 0x694) != 0) {
            delta.x = *(int *)pSlot - *(int *)(pSlot + 0x1e0);
            delta.y = *(int *)(pSlot + 4) - *(int *)(pSlot + 0x1e4);
            delta.z = *(int *)(pSlot + 8) - *(int *)(pSlot + 0x1e8);
            FixVecScale(&delta, &delta, scale);
            *(int *)(pSlot + 0x1ec) = *(int *)(pSlot + 0x1e0) + delta.x;
            *(int *)(pSlot + 0x1f0) = *(int *)(pSlot + 0x1e4) + delta.y;
            *(int *)(pSlot + 0x1f4) = *(int *)(pSlot + 0x1e8) + delta.z;

            p = (int *)(pSlot + 0x18);
            q = (int *)(pSlot + 0x1f8);
            d = (int *)(pSlot + 0x2e8);
            count = 20;
            do {
                delta.x = p[0] - q[0];
                delta.y = p[1] - q[1];
                delta.z = p[2] - q[2];
                FixVecScale(&delta, &delta, scale);
                d[0] = q[0] + delta.x;
                d[1] = q[1] + delta.y;
                d[2] = q[2] + delta.z;
                p += 3;
                q += 3;
                d += 3;
            } while (--count);

            p = (int *)(pSlot + 0x108);
            q = (int *)(pSlot + 0x3d8);
            d = (int *)(pSlot + 0x4b0);
            row = 3;
            do {
                column = 6;
                do {
                    delta.x = p[0] - q[0];
                    delta.y = p[1] - q[1];
                    delta.z = p[2] - q[2];
                    FixVecScale(&delta, &delta, scale);
                    d[0] = q[0] + delta.x;
                    d[1] = q[1] + delta.y;
                    d[2] = q[2] + delta.z;
                    p += 3;
                    q += 3;
                    d += 3;
                } while (--column);
            } while (--row);
        }
        index++;
        offset += 0x938;
    } while (index < (int)(g_unk0x00590afc & 0xff));
}
// FUNCTION: CMR2 0x00480a50
void FUN_00480a50(void)
{
    g_unk0x00590d70 = 0;
}

// FUNCTION: CMR2 0x00480ac0
void FUN_00480ac0(BYTE *pCar, int slot, int reset)
{
    if (g_unk0x00590b7c[slot][(signed char)pCar[0xb1a]] != NULL && reset != 0)
        g_unk0x00590b7c[slot][(signed char)pCar[0xb1a]][0x17c] = 0;
}

// FUNCTION: CMR2 0x00484d10
unsigned int FUN_00484d10(int i, int j)
{
    return g_unk0x00590c24[i][j];
}

// FUNCTION: CMR2 0x00484de0
BYTE *FUN_00484de0(BYTE *pCar, int slot)
{
    return g_unk0x00590b7c[slot][(signed char)pCar[0xb1a]];
}

struct Unk0x00590d74;
extern Unk0x00590d74 *g_unk0x00590d74;
extern int g_unk0x00590b30[8];
extern void **g_unk0x00590c6c;
extern int g_unk0x00590c00[8];
extern FixVector g_unk0x00590b50;

// Steps one 0x3c-byte record (`index`) of the current car's stage-object list:
// the record's +0x0 vector is copied into +0xc, the +0x30/+0x34 velocities lose
// the part of g_unk0x00590b50 that lies along the record's axis vector, and then
// a damped spring (k = 0x20000, c = 0x3333, step = g_physicsTimeStep) integrates
// +0x0/+0x2. Records with the +0x38 flag set are skipped.
// FUNCTION: CMR2 0x004854a0
void FUN_004854a0(int index)
{
    int car = *(char *)((BYTE *)g_unk0x00590d74 + 0xb1a);
    int *pRecord = (int *)((BYTE *)g_unk0x00590c6c[car] + (index & 0xff) * 0x3c);
    FixVector *pAxis = (FixVector *)(g_unk0x00590c00[car] + (index & 0xff) * 0x20 + 0xc);
    FixVector velocity;
    FixVector projected;
    int dot;

    if (pRecord[0xe] != 0)
        return;

    pRecord[3] = pRecord[0];
    pRecord[4] = pRecord[1];
    pRecord[5] = pRecord[2];

    velocity = g_unk0x00590b50;
    dot = FixVecDot(&velocity, pAxis);
    FixVecScale(&projected, pAxis, dot);
    velocity.x -= projected.x;
    velocity.y -= projected.y;
    velocity.z -= projected.z;

    pRecord[0xc] -= velocity.x;
    pRecord[0xd] -= velocity.z;

    pRecord[0xc] += FixMul(g_physicsTimeStep,
                           -(FixMul(0x20000, pRecord[0]) + FixMul(0x3333, pRecord[0xc])));
    pRecord[0xd] += FixMul(g_physicsTimeStep,
                           -(FixMul(0x20000, pRecord[2]) + FixMul(0x3333, pRecord[0xd])));
    pRecord[0] += FixMul(g_physicsTimeStep, pRecord[0xc]);
    pRecord[2] += FixMul(g_physicsTimeStep, pRecord[0xd]);
    pRecord[1] = 0x10000;
}

// Steps every 0x3c-byte record of the ordered cars' lists: the record's +0x18
// vector becomes its +0xc vector plus the (+0x0 - +0xc) difference scaled by
// `scale`.
// match 65%: same logic; register allocation and the inner loop scheduling differ
// match 65%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00486500
void FUN_00486500(int scale)
{
    int count;
    int n;
    int offset;
    short *pIndex;
    int *p;
    FixVector v;

    pIndex = Car_GetOrder();
    count = Car_GetOrderCount();
    if (count - 1 >= 0) {
        pIndex += count - 1;
        do {
            g_unk0x00590d74 = (Unk0x00590d74 *)Car_Get(*pIndex);
            if (*(int *)((BYTE *)g_unk0x00590d74 + 0xc0c) == 0) {
                n = *(int *)g_unk0x00590b30[*(char *)((BYTE *)g_unk0x00590d74 + 0xb1a)] - 1;
                if (n >= 0) {
                    offset = n * 0x3c;
                    n++;
                    do {
                        p = (int *)((BYTE *)g_unk0x00590c6c[*(char *)((BYTE *)g_unk0x00590d74 + 0xb1a)] +
                                    offset);
                        v.x = p[0] - p[3];
                        v.y = p[1] - p[4];
                        v.z = p[2] - p[5];
                        FixVecScale(&v, &v, scale);
                        p[6] = p[3] + v.x;
                        p[7] = p[4] + v.y;
                        p[8] = p[5] + v.z;
                        offset -= 0x3c;
                    } while (--n);
                }
            }
            pIndex--;
        } while (--count);
    }
}

// FUNCTION: CMR2 0x00486be0
void FUN_00486be0(BYTE *p, int unused)
{
    g_unk0x00590db0[*p] = 0x10000;
}

// FUNCTION: CMR2 0x00486c00
void FUN_00486c00(BYTE *p, BYTE *q)
{
    FUN_004ae410(q[2], q[1], 0, 0);
    g_unk0x00590db0[*p] = 0x10000;
}

// FUNCTION: CMR2 0x00487130
int FUN_00487130(void)
{
    return g_unk0x00591390;
}

// Applies a per-frame delta to one stage object (plus an optional second car
// index) and recurses into its children; `flag` enables the 0x5913d8 path.
// FUNCTION: CMR2 0x0048c870
void FUN_0048c870(BYTE index, BYTE other, int *pDelta, int flag)
{
    int i;

    g_unk0x005914c8.x = pDelta[0];
    g_unk0x005914c8.y = pDelta[1];
    g_unk0x005914c8.z = pDelta[2];
    g_unk0x005913d8 = flag;
    memset(g_unk0x0059146c, 0, sizeof(g_unk0x0059146c));
    g_unk0x0059146c[index] = 1;
    if (other != 0xff)
        g_unk0x0059146c[(char)other] = 1;
    i = 0;
    if (g_unk0x005913dc[index] != 0) {
        do {
            FUN_0048c900(g_unk0x005913f8[index][i]);
            i++;
        } while (i < g_unk0x005913dc[index]);
    }
}

// Moves one stage object and its eight box corners by the current frame delta
// and recurses over its children (each object is only moved once).
// match 70%: same logic; the corner pointer walk uses a different base bias
// match 70%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0048c900
void FUN_0048c900(BYTE index)
{
    Car *pCar;
    BYTE i;
    int j;
    int *p;

    if (g_unk0x0059146c[index] != 0)
        return;
    g_unk0x0059146c[index] = 1;
    pCar = Car_Get(index);
    pCar->position.x += g_unk0x005914c8.x;
    pCar->position.y += g_unk0x005914c8.y;
    pCar->position.z += g_unk0x005914c8.z;
    p = (int *)&pCar->corners[0].y;
    j = 8;
    do {
        p[-1] += g_unk0x005914c8.x;
        p[0] += g_unk0x005914c8.y;
        p[1] += g_unk0x005914c8.z;
        p += 3;
    } while (--j);
    if (g_unk0x005913d8 != 0) {
        if (*(int *)(g_unk0x00590ed0[index] + 0x28) != 0) {
            p = (int *)(g_unk0x00590ed0[index] + 0x34);
            j = 4;
            do {
                p[-1] += g_unk0x005914c8.x;
                p[0] += g_unk0x005914c8.y;
                p[1] += g_unk0x005914c8.z;
                p += 3;
            } while (--j);
        }
    }
    i = 0;
    if (g_unk0x005913dc[index] != 0) {
        do {
            FUN_0048c900(g_unk0x005913f8[index][i]);
            i++;
        } while (i < g_unk0x005913dc[index]);
    }
}

// FUNCTION: CMR2 0x0048ca40
BYTE *FUN_0048ca40(int index)
{
    return g_unk0x00590ed0[index];
}

// FUNCTION: CMR2 0x0048ca90
int FUN_0048ca90(void)
{
    return g_unk0x005918c8;
}

// Index of the 0x6c-byte record whose position (+0x34) is nearest to pPos.
// match 80%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0048d8b0
unsigned int FUN_0048d8b0(FixVector *pPos)
{
    unsigned int i;
    unsigned int best = 0;
    int bestDistance = 0x270f0000;
    int distance;
    FixVector d;

    for (i = 0; i < (unsigned int)g_unk0x005918c8; i++) {
        d.x = *(int *)(g_unk0x00591750 + i * 0x6c + 0x34) - pPos->x;
        d.y = *(int *)(g_unk0x00591750 + i * 0x6c + 0x38) - pPos->y;
        d.z = *(int *)(g_unk0x00591750 + i * 0x6c + 0x3c) - pPos->z;
        distance = FixVec_Length(&d);
        if (distance < bestDistance) {
            bestDistance = distance;
            best = i;
        }
    }
    return best;
}

// FUNCTION: CMR2 0x0048d930
int FUN_0048d930(BYTE *p)
{
    return g_unk0x00591740[*p];
}

extern int g_unk0x00591710[4];
void FUN_0048dce0(FixVector *pOut, BYTE *pSurface, Car *pCar, FixMatrix *pMatrix);

// Sliding impact of a car on a surface: decays the accumulated displacement by
// 0.9, adds the new weighted impact vector (0.1 * FUN_0048dce0), moves the
// object position by it, then refreshes the interpolated radius target.
// FUNCTION: CMR2 0x0048d950
void FUN_0048d950(BYTE *pSurface, FixMatrix *pMatrix)
{
    unsigned int index = *pSurface;
    FixVector *pPos = &g_unk0x005916a0[index];
    BYTE *pRecord = g_unk0x00591750 + g_unk0x00591740[index] * 0x6c;
    FixVector *pImpact = &g_unk0x00591868[index];
    FixVector impact;
    FixVector delta;
    int t;

    FUN_0048dce0(&impact, pSurface, Car_Get(pSurface[2]), pMatrix);
    FixVecScale(pImpact, pImpact, 0xe666);
    FixVecScale(&impact, &impact, 0x1999);
    pImpact->x += impact.x;
    pImpact->y += impact.y;
    pImpact->z += impact.z;
    FixMatrix_GetPosition(pPos, pMatrix);
    pPos->x += pImpact->x;
    pPos->y += pImpact->y;
    pPos->z += pImpact->z;
    delta.x = pPos->x - *(int *)(pRecord + 0x28);
    delta.y = pPos->y - *(int *)(pRecord + 0x2c);
    delta.z = pPos->z - *(int *)(pRecord + 0x30);
    t = FixDiv(FixVec_Length(&delta), *(int *)(pRecord + 0x40));
    g_unk0x00591710[index] = FixMul(t, *(int *)(pRecord + 0x44)) +
                             FixMul(0x10000 - t, *(int *)(pRecord + 0x54));
}

// FUNCTION: CMR2 0x00492890
void FUN_00492890(FixVector *pOut)
{
    int obj;

    FUN_0046f4c0(&obj);
    FixMatrix_RotateVector(pOut, &g_unk0x00592114, (FixMatrix *)(obj + 0x98));
}

// FUNCTION: CMR2 0x004928c0
void FUN_004928c0(int *pOut1, int *pOut2, int *pOut3)
{
    *pOut3 = g_unk0x00592128;
    *pOut1 = g_unk0x0059212c;
    *pOut2 = g_unk0x00592130;
}

// FUNCTION: CMR2 0x004928f0
void FUN_004928f0(int *pOut)
{
    *pOut = g_unk0x00592134;
}

// FUNCTION: CMR2 0x00492900
void FUN_00492900(int value)
{
    g_unk0x00592134 = value;
}

extern Mesh *g_stageMesh2Copy;
extern short g_stageMesh2Count;
// GLOBAL: CMR2 0x005920fc
int g_unk0x005920fc;
// GLOBAL: CMR2 0x00592100
int g_unk0x00592100;
// GLOBAL: CMR2 0x00592104
int g_unk0x00592104;

// Finds the vertex of stage mesh 2 closest to g_unk0x00592114 within a radius
// that shrinks to the best distance found so far, caches its stage-space
// position in 0x5920fc/0x592100/0x592104 and returns its index (-1 if none).
// match 53%: same logic; MSVC laid the locals out in different stack slots and
// kept the delta in different registers (the pointer pair to the delta local
// comes from the original's FixMul argument materialisation).
// FUNCTION: CMR2 0x00492910
int FUN_00492910(void)
{
    int obj[2];
    FixVector delta;
    FixVector vert;
    int dx;
    int dy;
    int dz;
    int dist;
    int limit;
    int best;
    int i;
    BYTE *pVertices;

    limit = 0x640000;
    best = -1;
    if (g_stageMesh2Count <= 0)
        return -1;
    FUN_0046f4e0(&obj[0], &obj[1]);
    FixMatrix_InverseRotateVector(&delta, &g_unk0x00592114, (FixMatrix *)(obj[1] + 0x98));
    for (i = 0; i < g_stageMesh2Count; i++) {
        pVertices = (BYTE *)g_stageMesh2Copy->pVertexData;
        vert.x = (int)(__int64)(*(float *)(pVertices + i * 0x30) * CGraphics::m_65536);
        vert.y = (int)(__int64)(*(float *)(pVertices + i * 0x30 + 4) * CGraphics::m_65536);
        vert.z = (int)(__int64)(*(float *)(pVertices + i * 0x30 + 8) * CGraphics::m_65536);
        dx = delta.x - vert.x;
        dy = delta.y - vert.y;
        dz = delta.z - vert.z;
        if (FIX_ABS(dx) <= limit && FIX_ABS(dy) <= limit && FIX_ABS(dz) <= limit) {
            dist = FixMul(dx, dx) + FixMul(dy, dy) + FixMul(dz, dz);
            if (dist <= 0x27100000) {
                limit = FixSqrt(dist);
                best = i;
            }
        }
    }
    if (best == -1)
        return -1;
    pVertices = (BYTE *)g_stageMesh2Copy->pVertexData;
    g_unk0x005920fc = (int)(__int64)(*(float *)(pVertices + best * 0x30) * CGraphics::m_65536);
    g_unk0x00592100 = (int)(__int64)(*(float *)(pVertices + best * 0x30 + 4) * CGraphics::m_65536);
    g_unk0x00592104 = (int)(__int64)(*(float *)(pVertices + best * 0x30 + 8) * CGraphics::m_65536);
    return best;
}

// FUNCTION: CMR2 0x00492bb0
void FUN_00492bb0(int *pOut)
{
    *pOut = *(int *)(*(BYTE **)(g_unk0x005920f0 + 0x24) + 0x34);
}

// FUNCTION: CMR2 0x00498570
BYTE *FUN_00498570(int index)
{
    return (BYTE *)g_unk0x00592734 + index * 0x2a4;
}

// Wraps a 16.16 angle in degrees into [-180, 180).
// FUNCTION: CMR2 0x00498db0
int FUN_00498db0(int angle)
{
    if (angle >= 0xb40000)
        angle -= 0x1680000;
    if (angle < -0xb40000)
        angle += 0x1680000;
    return angle;
}

// Marks a nearby car as travelling roughly towards the player car.
// match 81%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0047cc50
void StageObject_UpdateApproachingCar(int carIndex, int time)
{
    g_unk0x0058e270[carIndex] = 1;
    Car *pPlayer = Car_Get(0);
    Car *pOther = Car_Get(carIndex);
    int otherZ = pOther->position.z;
    int otherX = pOther->position.x;
    int heading = StageObject_Atan2Degrees(pOther->right.z, pOther->right.x);
    int duration = RallyData_FUN_00421420();
    int difference = RallyData_FUN_00421370((BYTE *)pPlayer) - time;
    if (difference < -100) difference += duration;
    if (difference >= 0 && difference <= 7) {
        int direction = StageObject_Atan2Degrees(pPlayer->position.z - otherZ,
                                                 pPlayer->position.x - otherX);
        direction = FUN_00498db0(heading - direction);
        if (direction <= 0xa0000 && direction >= -0xa0000) return;
    }
    g_unk0x0058e270[carIndex] = 0;
}

// Buttons held on any connected device.
// FUNCTION: CMR2 0x0049e940
unsigned int FUN_0049e940(void)
{
    unsigned int buttons = 0;
    int i;

    for (i = 0; i < 8; i++) {
        if (CInput::m_availableDevices[i].field_0x0 != -1)
            buttons |= CInput::m_availableDevices[i].field_0x4;
    }
    return buttons;
}

// Keeps the subset of the six cars that are still inside the race-time,
// squared-distance and heading windows of car `index`, copies the per-car keep
// flags to pOut, accumulates their number into pCount and reports the overtake
// side in pFlag (1 / 0xff). Returns the first byte of the 0x10-byte race-record
// table row of `index`.
// FUNCTION: CMR2 0x0047d0e0
unsigned int FUN_0047d0e0(int index, BYTE *pOut, int *pCount, BYTE *pFlag)
{
    Car *cars[6];
    int active[6];
    int scratch[6];
    Car *pCar;
    int i;
    int duration;
    int z;
    int x;
    int referenceTime;
    int heading;

    for (i = 0; i < 6; i++)
        active[i] = 1;
    for (i = 0; i < 6; i++) {
        *pFlag = 0;
        cars[i] = Car_Get(i);
    }
    duration = RallyData_FUN_00421420();
    pCar = cars[index];
    active[index] = 0;
    z = pCar->position.z;
    x = pCar->position.x;
    *pCount = 0;
    referenceTime = RallyData_FUN_00421370((BYTE *)pCar);

    for (i = 0; i < (signed char)g_unk0x0058e0b0[1]; i++) {
        if (active[i] != 0) {
            int delta = RallyData_FUN_00421370((BYTE *)cars[i]);
            scratch[i] = delta;
            delta -= referenceTime;
            if (delta < 0 || delta > 5) {
                if (delta < -100)
                    delta += duration;
                if (delta <= 0 || delta > 5)
                    active[i] = 0;
            }
        }
    }

    for (i = 0; i < (signed char)g_unk0x0058e0b0[1]; i++) {
        if (active[i] != 0) {
            int dz = cars[i]->position.z - z;
            int dx = cars[i]->position.x - x;
            int dist = FixMul(dz, dz) + FixMul(dx, dx);
            scratch[i] = dist;
            if (dist > 0x90000)
                active[i] = 0;
        }
    }

    heading = StageObject_Atan2Degrees(pCar->right.z, pCar->right.x);
    for (i = 0; i < (signed char)g_unk0x0058e0b0[1]; i++) {
        if (active[i] != 0) {
            int d = StageObject_Atan2Degrees(cars[i]->right.z, cars[i]->right.x);
            d = FUN_00498db0(heading - d);
            if (d > 0x140000 || d < -0x140000)
                active[i] = 0;
        }
    }

    for (i = 0; i < (signed char)g_unk0x0058e0b0[1]; i++) {
        if (active[i] != 0) {
            int d = FUN_00498db0(StageObject_Atan2Degrees(cars[i]->position.z - z,
                                                          cars[i]->position.x - x) - heading);
            active[i] = 0;
            if (d > 0x500000 && d < 0x780000)
                *pFlag = 1;
            if (d < -0x500000 && d > -0x780000)
                *pFlag = 0xff;
        }
    }

    for (i = 0; i < (signed char)g_unk0x0058e0b0[1]; i++) {
        pOut[i] = (BYTE)active[i];
        *pCount += active[i];
    }
    return g_unk0x0058e4a4[referenceTime * 0x10];
}

void FUN_004bcad0(int value);
void Scene_SetLight(FixVector *pLight, int boost);
int Track_GetGroundHeightSurface(FixVector *pPoint, FixVector *pNormal, short *pTri, short *pSurfaceClass,
                                 unsigned short *pSurface, int defaultY);
DWORD FUN_004b74e0(void);
DWORD FUN_004b74f0(void);
DWORD FUN_004b7500(void);
int *FUN_00469680(int index);
void FUN_00480ac0(BYTE *pCar, int slot, int reset);
void FUN_0042b720(int index, char value);
int FUN_00457e10(BYTE *pCar, int offset);
struct KnockoutMatch;
int FUN_00472990(KnockoutMatch *pMatch);
short Car_GetOrderCount(void);

// FUNCTION: CMR2 0x00492fd0
void FUN_00492fd0(int value)
{
    FUN_004bcad0(value);
}

// Position of the object held in 0x589438 (its matrix at +0x98).
// FUNCTION: CMR2 0x0046f4a0
void FUN_0046f4a0(FixVector *pOut)
{
    if (g_unk0x00589438 != 0)
        FixMatrix_GetPosition(pOut, (FixMatrix *)(g_unk0x00589438 + 0x98));
}

// Ground height at a point; the surface id is written over the defaultY slot.
// FUNCTION: CMR2 0x004930b0
int Track_GetGroundHeight5(FixVector *pPoint, FixVector *pNormal, short *pTri, short *pSurfaceClass, int defaultY)
{
    return Track_GetGroundHeightSurface(pPoint, pNormal, pTri, pSurfaceClass, (unsigned short *)&defaultY, defaultY);
}

// Sets the stage light, without the boost for country 3.
// FUNCTION: CMR2 0x00492e30
void FUN_00492e30(FixVector *pLight)
{
    if ((char)RallyDataCountryIndex() == 3) {
        Scene_SetLight(pLight, 0);
        return;
    }
    Scene_SetLight(pLight, 1);
}

// Clears the value of every car slot not in use (or all of them when
// CGameInfo::FUN_00406320 is set).
// FUNCTION: CMR2 0x0047c1b0
void FUN_0047c1b0(void)
{
    int i;

    for (i = 0; i < 6; i++) {
        if (i >= (int)(RallyDataState() & 0xff) || CGameInfo::FUN_00406320() != 0)
            FUN_0042b720(i, 0xff);
    }
}

// FUNCTION: CMR2 0x00466490
void FUN_00466490(void)
{
    if (FUN_004b74e0() == 0 && FUN_004b74f0() == 0 && FUN_004b7500() == 0) {
        g_unk0x00588868 = 0;
        return;
    }
    g_unk0x00588868 = 1;
}

// GLOBAL: CMR2 0x0058896c
int g_unk0x0058896c;

unsigned int RallyData_FUN_00407e70(void);
BYTE FUN_00422fb0(BYTE index);
int FUN_0041f3a0(void);
void FUN_00494db0(Car *pCar, int view);
void FUN_00460330(int a, int b);
void FUN_00485690(short *pOrder, short count, int view);
void FUN_0047f740(void);

// Updates the "damaged / off-road" state of every car in the given order.
// match 86%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00466570
void FUN_00466570(short *param_1, short param_2, int param_3, int param_4)
{
    short *p;
    Car *pCar;
    int i;

    i = 0;
    if ((int)param_2 > 0) {
        p = param_1;
        do {
            pCar = Car_Get(*p);
            if (*(int *)((BYTE *)pCar + 0xc0c) == 0 &&
                *(int *)((BYTE *)pCar + 0xb68 + param_4 * 4) == 0) {
                if (FUN_0041f3a0() != 0) {
                    if ((char)RallyData_FUN_00407e90() || (char)RallyData_FUN_00407e70() ||
                        (unsigned int)i == (FUN_00422fb0(1) & 0xff))
                        FUN_00494db0(pCar, 1);
                } else if (FUN_0046bd20(*(char *)((BYTE *)pCar + 0xb1a), param_4) != 7) {
                    FUN_00494db0(pCar, param_4);
                }
            }
            i = i + 1;
            p = p + 1;
        } while (i < (int)param_2);
    }
    FUN_00485690(param_1, param_2, param_4);
    FUN_00460330(param_3, param_4);
    if (g_unk0x0058896c != 0)
        FUN_0047f740();
}

// Fixed-point to float conversion factors and the fade thresholds of the
// stage object lighting.
// GLOBAL: CMR2 0x00511378
extern const float g_unk0x00511378 = 5.0f;
// GLOBAL: CMR2 0x005113d0
extern const float g_unk0x005113d0 = 1.0f / 45.0f;
// GLOBAL: CMR2 0x005113dc
extern const float g_unk0x005113dc = 480.0f;
// GLOBAL: CMR2 0x005113e0
extern const float g_unk0x005113e0 = 640.0f;

extern const float g_netZero;
extern const float g_netByteScale;
extern const float g_netOne;

int FUN_00422f50(BYTE index);

// Fades a stage object in and out from the screen distance between two
// projected points of the player's car.
// match 69%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00466100
void FUN_00466100(int param_1)
{
    Car *pCar;
    BYTE *pView;
    FixVector pos;
    FixVector dir;
    int screen[2];
    float f2;
    float f4;
    float f1;
    int alpha;

    pCar = Car_Get(1);
    pView = (BYTE *)g_viewNodes[param_1];
    FixMatrix_GetPosition(&pos, (FixMatrix *)((BYTE *)pCar->pNode0x71c + 0x98));
    pos.y = pos.y + 0x10000;
    FUN_004bad40(screen, &pos, pView);
    if (screen[0] != -0x640000 || screen[1] != -0x640000) {
        f2 = (float)(screen[0] * CGraphics::m_oneOver65536);
        f4 = (float)(screen[1] * CGraphics::m_oneOver65536);
        FixMatrix_GetUp(&dir, (FixMatrix *)(pView + 0x98));
        dir.x = dir.x + pos.x;
        dir.y = dir.y + pos.y;
        dir.z = dir.z + pos.z;
        FUN_004bad40(screen, &dir, pView);
        if (screen[0] != -0x640000 || screen[1] != -0x640000) {
            f1 = ((float)(screen[0] * CGraphics::m_oneOver65536) - f2) * g_unk0x005113e0 /
                 (float)*(int *)g_pGraphics;
            f2 = ((float)(screen[1] * CGraphics::m_oneOver65536) - f4) * g_unk0x005113dc /
                 (float)*(int *)((BYTE *)g_pGraphics + 4);
            f1 = (float)sqrt(f1 * f1 + f2 * f2);
            if (f1 <= g_unk0x00511378)
                return;
            f1 = g_netOne - (f1 - g_unk0x00511378) * g_unk0x005113d0;
            if (f1 < g_netOne) {
                if (g_netZero < f1)
                    alpha = (int)(__int64)(f1 * g_netByteScale);
                else
                    alpha = 0;
            } else {
                alpha = 0xff;
            }
            if (FUN_00422f50(param_1) == 7)
                alpha = 0x80;
            if (alpha == g_unk0x00588864)
                return;
            FUN_00465ec0(pCar->pNode0x71c, alpha, 0);
            FUN_00465f20(pCar->pNode0x71c->pFirstChild, alpha, 0);
            FUN_00465ec0(pCar->pNode0x720, alpha, 0);
            FUN_00465f20(pCar->pNode0x720->pFirstChild, alpha, 0);
            g_unk0x00588864 = alpha;
            return;
        }
    }
    if (g_unk0x00588864 == 0)
        return;
    FUN_00465ec0(pCar->pNode0x71c, 0, 0);
    FUN_00465f20(pCar->pNode0x71c->pFirstChild, 0, 0);
    FUN_00465ec0(pCar->pNode0x720, 0, 0);
    FUN_00465f20(pCar->pNode0x720->pFirstChild, 0, 0);
    g_unk0x00588864 = 0;
}

// Swaps *pValue with the value stored for `slot` when that slot is pending.
// FUNCTION: CMR2 0x004660a0
void FUN_004660a0(int **pValue, int slot, char flag)
{
    int *old;

    if (g_unk0x00588761 == slot) {
        old = g_unk0x00588758;
        g_unk0x00588758 = *pValue;
        *pValue = old;
        g_unk0x00588761 = -1;
        g_unk0x00588760 = flag;
    }
}

// FUNCTION: CMR2 0x0046b670
void FUN_0046b670(BYTE *pCar)
{
    int *p;
    int i;

    p = FUN_00469680((char)pCar[0xb1a]);
    i = 0;
    while (i < 4) {
        FUN_00480ac0(pCar, i, *(int *)((BYTE *)p + 0x4b0 + i * 4));
        i++;
    }
}

// Per car in race order: its split value (see FUN_00457e10).
// GLOBAL: CMR2 0x00590d90
int g_carSplitValues[8];

// FUNCTION: CMR2 0x00486700
void FUN_00486700(void)
{
    int *p;
    int i;

    i = 0;
    if (Car_GetOrderCount() > 0) {
        p = g_carSplitValues;
        do {
            *p = FUN_00457e10((BYTE *)Car_Get(i), 4);
            i++;
            p++;
        } while (i < Car_GetOrderCount());
    }
}

// GLOBAL: CMR2 0x0058cf70
int g_unk0x0058cf70;

// Clears the championship "pending" flag (bit 23) when set, or when 0x58cf70 is clear.
// FUNCTION: CMR2 0x004729f0
BYTE FUN_004729f0(void)
{
    unsigned int *pState;

    pState = RallyData_GetChampionshipState();
    if ((*pState & 0x800000) == 0 && g_unk0x0058cf70 != 0)
        return 0;
    *pState &= 0xff7fffff;
    g_unk0x0058cf7c = 0;
    CGame::FUN_004057e0(0);
    return 1;
}

// Eight records of 0x48 bytes: ten shorts at +0x1c (reset to -1) and two
// flag bytes at +0x44/+0x45.
// GLOBAL: CMR2 0x0058d6d0
BYTE g_unk0x0058d6d0[8][0x48];

// Fills the 12 outline values of a stage box (11 boundary levels plus the
// corner colour at +0x16) and repaints its two textures once the cached copy
// differs from the new values.
int FUN_00445dd0(int index);
void FUN_004775f0(Texture *pTexture, int state, int cacheBase, int index);
// FUNCTION: CMR2 0x00477460
void FUN_00477460(int index)
{
    unsigned short *pNew = g_unk0x0058d310 + index * 0xc;
    unsigned short *pOld = (unsigned short *)(g_stageBlock + index * 0x18);
    int changed = 0;
    int limit;
    int slot;
    int i;

    limit = FixMulShift32(FUN_00445dd0(index), 0xb0000);
    for (i = 0; i <= 0xa; i++)
        pNew[i] = ((i >= limit) - 1) & 0xff;
    if (g_unk0x0058d4c4[index * 2] != 0)
        FUN_004775f0((Texture *)g_unk0x0058d4c4[index * 2], (int)Car_Get(index)->field_0xb1e, 2, index);
    if (g_unk0x0058d4c0[index * 2] != 0) {
        slot = index + 8;
        pNew[0xb] = 0x6c;
        // the original leaves the scan by setting the counter to 0xc
        for (i = 0; i < 0xc; i++) {
            if (pNew[i] != pOld[i]) {
                changed = 1;
                i = 0xc;
            }
        }
        if (changed != 0) {
            CGraphics::BltTexture((Texture *)g_unk0x0058d4c0[index * 2], slot);
            CGraphics::RemapTextureAlpha((Texture *)g_unk0x0058d4c0[index * 2], 0xe0, 0xff, 0xd0, pNew[1], 0xc0,
                                         pNew[2], slot);
            CGraphics::RemapTextureAlpha((Texture *)g_unk0x0058d4c0[index * 2], 0xb0, pNew[3], 0xa0, pNew[4], 0x90,
                                         pNew[5], slot);
            CGraphics::RemapTextureAlpha((Texture *)g_unk0x0058d4c0[index * 2], 0x80, pNew[6], 0x70, pNew[7], 0x60,
                                         pNew[8], slot);
            CGraphics::RemapTextureAlpha((Texture *)g_unk0x0058d4c0[index * 2], 0x50, pNew[9], 0x40, pNew[10], 0x30,
                                         pNew[0xb], slot);
            for (i = 0; i < 0xc; i++)
                pOld[i] = pNew[i];
        }
    }
}

// Body colours of the two stage objects for the object's current body state:
// seven alpha values per object, compared against the ones already applied to
// the texture so the remap only runs when they change.
// FUNCTION: CMR2 0x004775f0
void FUN_004775f0(Texture *pTexture, int state, int cacheBase, int index)
{
    WORD *pColours = g_unk0x0058d2d4 + index * 7;
    WORD *pApplied = (WORD *)g_unk0x0058d6b0 + index * 7;
    int cacheSlot = index + cacheBase * 8;
    int changed = 0;
    int i;

    switch (state) {
    case 0:
        pColours[0] = 0;
        pColours[1] = 0xff;
        pColours[2] = 0xff;
        pColours[3] = 0xff;
        pColours[4] = 0xff;
        pColours[5] = 0xff;
        pColours[6] = 0xff;
        break;
    case 1:
        pColours[0] = 0;
        pColours[1] = 0;
        pColours[2] = 0xff;
        pColours[3] = 0xff;
        pColours[4] = 0;
        pColours[5] = 0;
        pColours[6] = 0;
        break;
    case 2:
        pColours[0] = 0xff;
        pColours[1] = 0xff;
        pColours[2] = 0xff;
        pColours[3] = 0;
        pColours[4] = 0xff;
        pColours[5] = 0xff;
        pColours[6] = 0;
        break;
    case 3:
        pColours[0] = 0xff;
        pColours[1] = 0xff;
        pColours[2] = 0xff;
        pColours[3] = 0xff;
        pColours[4] = 0xff;
        pColours[5] = 0;
        pColours[6] = 0;
        break;
    case 4:
        pColours[0] = 0xff;
        pColours[1] = 0;
        pColours[2] = 0xff;
        pColours[3] = 0xff;
        pColours[4] = 0;
        pColours[5] = 0;
        pColours[6] = 0xff;
        break;
    case 5:
        pColours[0] = 0xff;
        pColours[1] = 0xff;
        pColours[2] = 0;
        pColours[3] = 0xff;
        pColours[4] = 0xff;
        pColours[5] = 0;
        pColours[6] = 0xff;
        break;
    case 6:
        pColours[0] = 0xff;
        pColours[1] = 0xff;
        pColours[2] = 0;
        pColours[3] = 0xff;
        pColours[4] = 0xff;
        pColours[5] = 0xff;
        pColours[6] = 0xff;
        break;
    case 7:
        pColours[0] = 0xff;
        pColours[1] = 0;
        pColours[2] = 0;
        pColours[3] = 0;
        pColours[4] = 0;
        pColours[5] = 0xff;
        pColours[6] = 0;
        break;
    case 8:
        pColours[0] = 0xff;
        pColours[1] = 0xff;
        pColours[2] = 0xff;
        pColours[3] = 0xff;
        pColours[4] = 0xff;
        pColours[5] = 0xff;
        pColours[6] = 0xff;
        break;
    case 9:
        pColours[0] = 0xff;
        pColours[1] = 0xff;
        pColours[2] = 0xff;
        pColours[3] = 0xff;
        pColours[4] = 0;
        pColours[5] = 0;
        pColours[6] = 0xff;
        break;
    default:
        pColours[0] = 0xff;
        pColours[1] = 0;
        pColours[2] = 0;
        pColours[3] = 0;
        pColours[4] = 0;
        pColours[5] = 0;
        pColours[6] = 0;
        break;
    }
    // the original leaves the scan by setting the counter to 7 (a `break` generates different code)
    for (i = 0; i < 7; i++) {
        if (pColours[i] != pApplied[i]) {
            changed = 1;
            i = 7;
        }
    }
    if (changed != 0) {
        CGraphics::BltTexture(pTexture, cacheSlot);
        CGraphics::RemapTextureAlpha(pTexture, 0x80, pColours[0], 0xe0, pColours[1], 0xd0, pColours[2], cacheSlot);
        CGraphics::RemapTextureAlpha(pTexture, 0xc0, pColours[3], 0xb0, pColours[4], 0xa0, pColours[5], cacheSlot);
        CGraphics::RemapTextureAlpha(pTexture, 0x90, pColours[6], 0x90, pColours[6], 0x90, pColours[6], cacheSlot);
        for (i = 0; i < 7; i++)
            pApplied[i] = pColours[i];
    }
}

// Fades the two 5-slot colour ramps of a car's stage object towards its flag
// bytes, repaints both cached textures when anything changed and stores the frame.
extern int g_unk0x0051bd3c;
// FUNCTION: CMR2 0x00477ce0
void FUN_00477ce0(int car)
{
    BYTE *pRecord = g_unk0x0058d6d0[car];
    short values[5];
    int changed = 0;
    int value;
    int i;

    for (i = 0; i < 5; i++) {
        if ((pRecord[0x44] & (1 << i)) == 0) {
            value = *(short *)(pRecord + 0x30 + i * 2) - FixMul(0x50, g_unk0x0051bd3c);
            if (value < 0)
                value = 0;
        } else {
            value = *(short *)(pRecord + 0x30 + i * 2) + FixMul(0x80, g_unk0x0051bd3c);
            if (value > 0xff)
                value = 0xff;
        }
        *(short *)(pRecord + 0x30 + i * 2) = value;
        *(short *)(pRecord + 0x8 + i * 2) = value;
        if ((pRecord[0x45] & (1 << i)) == 0) {
            value = *(short *)(pRecord + 0x3a + i * 2) - FixMul(0x50, g_unk0x0051bd3c);
            if (value < 0)
                value = 0;
        } else {
            value = *(short *)(pRecord + 0x3a + i * 2) + FixMul(0x80, g_unk0x0051bd3c);
            if (value > 0xff)
                value = 0xff;
        }
        *(short *)(pRecord + 0x3a + i * 2) = (BYTE)value;
        *(short *)(pRecord + 0x12 + i * 2) = (BYTE)value;
    }
    if (pRecord[0x46] == 0) {
        *(short *)(pRecord + 0x8) = (short)((*(short *)(pRecord + 0x8) / 3) * 2);
        *(short *)(pRecord + 0x12) = (short)((*(short *)(pRecord + 0x12) / 3) * 2);
    } else {
        value = *(short *)(pRecord + 0xa) + (*(short *)(pRecord + 0x8) / 3) * 2;
        if (value > 0xff)
            value = 0xff;
        *(short *)(pRecord + 0xa) = (BYTE)value;
        value = *(short *)(pRecord + 0x14) + (*(short *)(pRecord + 0x12) / 3) * 2;
        if (value > 0xff)
            value = 0xff;
        *(short *)(pRecord + 0x14) = (BYTE)value;
    }
    for (i = 0; i < 5; i++) {
        if (*(short *)(pRecord + 0x12 + i * 2) != *(short *)(pRecord + 0x26 + i * 2) ||
            *(short *)(pRecord + 0x8 + i * 2) != *(short *)(pRecord + 0x1c + i * 2))
            changed = 1;
        if (*(short *)(pRecord + 0x8 + i * 2) > *(short *)(pRecord + 0x12 + i * 2))
            values[i] = *(short *)(pRecord + 0x8 + i * 2);
        else
            values[i] = *(short *)(pRecord + 0x12 + i * 2);
    }
    if (changed != 0) {
        Texture **ppTextures = (Texture **)pRecord;
        Texture *pTexture;

        for (i = 0; i < 2; i++) {
            pTexture = ppTextures[i];
            if (pTexture != NULL) {
                CGraphics::BltTexture(pTexture, car);
                CGraphics::RemapTextureAlpha(pTexture, 0xbf, values[1], 0x40, values[3], 0x80, values[2], car);
                CGraphics::RemapTextureAlpha(pTexture, 0x60, values[0], 0xe0, values[4], 0xe0, values[4], car);
            }
        }
        for (i = 0; i < 5; i++) {
            *(short *)(pRecord + 0x1c + i * 2) = *(short *)(pRecord + 0x8 + i * 2);
            *(short *)(pRecord + 0x26 + i * 2) = *(short *)(pRecord + 0x12 + i * 2);
        }
    }
}

// match 64%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00477f30
void FUN_00477f30(void)
{
    short *p;

    p = (short *)&g_unk0x0058d6d0[0][0x1e];
    do {
        p[-1] = -1;
        p[0] = -1;
        p[1] = -1;
        p[2] = -1;
        p[3] = -1;
        p[4] = -1;
        p[5] = -1;
        p[6] = -1;
        p[7] = -1;
        p[8] = -1;
        p += 0x24;
    } while ((int)p < (int)&g_unk0x0058d6d0[8][0x1e]);
}

int FUN_00460c80(BYTE *pCar);
char RallyData_FUN_00408500(BYTE param1);

// GLOBAL: CMR2 0x005909b8
int g_unk0x005909b8;
// GLOBAL: CMR2 0x005909c0
BYTE g_unk0x005909c0[4];
// GLOBAL: CMR2 0x005909c4
BYTE g_unk0x005909c4[4];

// Per car: ticks until the next headlight glow may be spawned.
// GLOBAL: CMR2 0x0058e4a8
int g_unk0x0058e4a8[8];
// Glow record of the stage objects: pRec offsets are relative to this base,
// i.e. 0x18 bytes below the glow fields that 0x47d510 walks.
// GLOBAL: CMR2 0x0058e4c8
BYTE g_unk0x0058e4c8[100][0x5c];
// Same records as 0x47d5a0 walks, seen from their position field (+0xc): the
// pointer arithmetic of that view lives in 0x47e1e0.
// GLOBAL: CMR2 0x0058e4d4
BYTE g_unk0x0058e4d4[100][0x5c];

// Spawns the headlight glow of one stage object: finds the first free record,
// places it at the top corner of the car's bounding box, aims it along the
// body's right axis and drops it onto the ground below.
// match 85%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// The instruction sequence is the original's; the residual difference is the
// register numbering of the vector temporaries.
// FUNCTION: CMR2 0x0047d5a0
void FUN_0047d5a0(BYTE car)
{
    FixVector dir;
    FixVector half;
    Car *pCar;
    short surface;
    int *pRec;
    int i;

    if (g_unk0x0058e4a8[car] <= 0) {
        pRec = (int *)g_unk0x0058e4c8;
        i = 0;
        do {
            if (pRec[0x15] == 0) {
                pCar = Car_Get(car);
                FixVecScale(&dir, &pCar->up, *(int *)&pCar->field_0x770[4]);
                half.x = pCar->corners[1].x - pCar->corners[0].x;
                half.y = pCar->corners[1].y - pCar->corners[0].y;
                half.z = pCar->corners[1].z - pCar->corners[0].z;
                FixVecScale(&half, &half, 0x8000);
                pRec[3] = half.x + pCar->corners[0].x + dir.x;
                pRec[4] = half.y + pCar->corners[0].y + dir.y;
                pRec[5] = half.z + pCar->corners[0].z + dir.z;
                FixVecScale((FixVector *)pRec, &pCar->right, 0xcccc);
                pRec[0] += pCar->velocity.x;
                pRec[1] += pCar->velocity.y;
                pRec[2] += pCar->velocity.z;
                FixVecScale((FixVector *)pRec, &pCar->right, FixVecDot((FixVector *)pRec, &pCar->right));
                pRec[0x12] = 0x320000;
                pRec[9] = 0x10000;
                pRec[0x15] = 1;
                *(BYTE *)(pRec + 0x16) = car;
                pRec[0x11] = 0;
                *(short *)(pRec + 0x13) = -1;
                pRec[0x11] = Track_GetGroundHeightSurface((FixVector *)(pRec + 3), (FixVector *)(pRec + 6),
                                                          (short *)(pRec + 0x13), &surface,
                                                          (unsigned short *)&surface, 0);
                pRec[4] = pRec[0x11] + 0x8000;
                *(FixVector *)(pRec + 0xa) = *(FixVector *)(pRec + 3);
                *(FixVector *)(pRec + 0xd) = *(FixVector *)(pRec + 6);
                pRec[0x10] = pRec[9];
                i = 100;
                g_unk0x0058e4a8[car] = 0x100000;
            }
            pRec += 0x17;
            i++;
        } while (i < 100);
    }
}

void Glow_SetPosition(GlowLight *pLight, FixVector *pPos, FixVector *pDir);
void Glow_SetLayerPlane(GlowLight *pLight, FixVector *pPoint, FixVector *pNormal, int layerIntensity);

// Interpolates every headlight glow between its spawn record (the copy at
// +0x28/+0x34) and the current car state by the fraction t, normalises the
// direction and moves the light with it.
// match 59%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// The addresses are the original's (checked against the emitting disassembly);
// MSVC materialises the record pointer 4 bytes higher and compensates with -4
// displacements, so every memory operand reads differently.
// FUNCTION: CMR2 0x0047e1e0
void FUN_0047e1e0(int t)
{
    FixVector pos;
    FixVector normal;
    FixVector delta;
    FixVector ground;
    int *pRec;
    int length;
    int size;
    int i;

    pRec = (int *)g_unk0x0058e4d4;
    i = 100;
    do {
        if (pRec[0x12] == 0) {
            FUN_004ae3d0((BYTE *)pRec[0x11], 0);
        } else {
            delta.x = pRec[0] - pRec[7];
            delta.y = pRec[1] - pRec[8];
            delta.z = pRec[2] - pRec[9];
            FixVecScale(&delta, &delta, t);
            pos.x = delta.x + pRec[7];
            pos.y = delta.y + pRec[8];
            pos.z = delta.z + pRec[9];
            delta.x = pRec[3] - pRec[0xa];
            delta.y = pRec[4] - pRec[0xb];
            delta.z = pRec[5] - pRec[0xc];
            FixVecScale(&delta, &delta, t);
            normal.x = delta.x + pRec[0xa];
            normal.y = delta.y + pRec[0xb];
            normal.z = delta.z + pRec[0xc];
            length = FixVecLength(&normal);
            if (length == 0) {
                normal.x = 0;
                normal.y = 0;
                normal.z = 0;
            } else {
                FixVecScaleRecip(&normal, &normal, length);
            }
            size = FixMul(pRec[6] - pRec[0xd], t) + pRec[0xd];
            if (size <= 0) {
                FUN_004ae3d0((BYTE *)pRec[0x11], 0);
            } else {
                FUN_004ae3d0((BYTE *)pRec[0x11], 1);
                FUN_004ae3f0((BYTE *)pRec[0x11], size);
                Glow_SetPosition((GlowLight *)pRec[0x11], &pos, &pos);
                ground = pos;
                ground.y -= 0x8000;
                Glow_SetLayerPlane((GlowLight *)pRec[0x11], &ground, &normal, 0);
            }
        }
        pRec += 0x17;
    } while (--i);
}

// FUNCTION: CMR2 0x0047e490
void FUN_0047e490(BYTE *pColour)
{
    g_unk0x005909c0[0] = pColour[0];
    g_unk0x005909c0[1] = pColour[1];
    g_unk0x005909c0[2] = pColour[2];
    g_unk0x005909c4[0] = 0xff;
    g_unk0x005909c4[1] = 0xff;
    g_unk0x005909c4[2] = 0xff;
    g_unk0x005909b8 = 0;
}

// Fades the stage ambient colour towards the base colour while the 0x5909b8
// timer runs out, then applies it with one step of boost.
// match 72%: same logic; MSVC puts the length/scale locals in the other stack
// slots and swaps the addend order
// match 72%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00480220
void FUN_00480220(void)
{
    BYTE colour[4];
    int length;
    int scale;

    length = FixSqrt(g_unk0x005909b8);
    scale = (g_unk0x005909c4[0] & 0xff) << 16;
    scale = FixMulShift32(scale, length) + (g_unk0x005909c0[0] & 0xff);
    if (scale > 0xff)
        scale = 0xff;
    colour[0] = scale;
    scale = (g_unk0x005909c4[1] & 0xff) << 16;
    scale = FixMulShift32(scale, length) + (g_unk0x005909c0[1] & 0xff);
    if (scale > 0xff)
        scale = 0xff;
    colour[1] = scale;
    scale = (g_unk0x005909c4[2] & 0xff) << 16;
    scale = FixMulShift32(scale, length) + (g_unk0x005909c0[2] & 0xff);
    if (scale > 0xff)
        scale = 0xff;
    colour[2] = scale;
    g_unk0x005909b8 -= 0x8000;
    if (g_unk0x005909b8 < 0)
        g_unk0x005909b8 = 0;
    Scene_SetAmbient(colour, 1);
}

// Selects a list of 0x6c-byte records (count first); returns whether it is non-empty.
// match 17%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0048caa0
int FUN_0048caa0(int *pList)
{
    int count;

    if (pList != NULL) {
        count = *pList;
        g_unk0x00591750 = (BYTE *)(pList + 1);
        g_unk0x005918c8 = count;
        return count != 0;
    }
    g_unk0x005918c8 = 0;
    g_unk0x00591750 = NULL;
    return 0;
}

// Whether a car's wheel sits on a surface of kind 0, 3, 12, 13 or 26 while FUN_00460c80 > 0.
// FUNCTION: CMR2 0x0046eeb0
BYTE FUN_0046eeb0(int index, int wheel)
{
    BYTE result;
    BYTE *pCar;

    result = 0;
    pCar = (BYTE *)Car_Get(index);
    switch (*(short *)(pCar + 0xaae + wheel * 2)) {
    case 0:
    case 3:
    case 0xc:
    case 0xd:
    case 0x1a:
        if (FUN_00460c80(pCar) > 0)
            result = 1;
    }
    return result;
}

// GLOBAL: CMR2 0x00591730
int g_unk0x00591730[4];

// Adds to a car's level (clamped to 1.0).
// match 25%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0048dca0
void FUN_0048dca0(BYTE *pCar, int amount)
{
    BYTE car;
    int v;

    if (amount > 0) {
        car = *pCar;
        v = g_unk0x00591730[car] + amount;
        g_unk0x00591730[car] = v;
        if (v > 0x10000)
            g_unk0x00591730[car] = 0x10000;
    }
}

// Builds the impact displacement of a car on a surface record: the surface's
// right vector minus the car's (normalised) velocity direction, scaled by the
// record's factor and the car speed.
// FUNCTION: CMR2 0x0048dce0
void FUN_0048dce0(FixVector *pOut, BYTE *pSurface, Car *pCar, FixMatrix *pMatrix)
{
    FixVector direction;
    FixVector right;
    int length;
    int scale;

    direction = pCar->velocity;
    FixMatrix_GetRight(&right, pMatrix);
    if (pCar->speed < 0x28f) {
        direction.x = 0;
        direction.y = 0;
        direction.z = 0;
    } else {
        length = FixVecLength(&direction);
        if (length == 0) {
            direction.x = 0;
            direction.y = 0;
            direction.z = 0;
        } else {
            FixVecScaleRecip(&direction, &direction, length);
        }
        if (FixVecDot(&right, &direction) < 0) {
            direction.x = -direction.x;
            direction.y = -direction.y;
            direction.z = -direction.z;
        }
    }
    pOut->x = right.x - direction.x;
    pOut->y = right.y - direction.y;
    pOut->z = right.z - direction.z;
    scale = FixMul(*(int *)(g_unk0x00591750 + g_unk0x00591740[*pSurface] * 0x6c + 0x48), pCar->speed);
    FixVecScale(pOut, pOut, scale);
}

// match 83%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0048df10
int FUN_0048df10(BYTE *pCar)
{
    short v;

    if (*(int *)(pCar + 4) == 7) {
        v = *(short *)(g_unk0x00591750 + g_unk0x00591740[*pCar] * 0x6c + 2);
        if (v < -0x3f4 && v > -0x40b)
            return 1;
    }
    return 0;
}

// match 51%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00486b90
void FUN_00486b90(BYTE *pCar, BYTE *pInfo)
{
    int kind;

    kind = *(int *)(pInfo + 4);
    if (kind != 2 && kind != 1 && kind != 10) {
        g_unk0x00590db0[*pCar] = 0;
        return;
    }
    g_unk0x00590db0[*pCar] = 0x10000;
}

int FUN_00472990(KnockoutMatch *pMatch);

// Returns 1 when a match of the current knockout round is still undecided.
// FUNCTION: CMR2 0x00473290
int FUN_00473290(void)
{
    unsigned int *pState;
    KnockoutMatch *pMatch = NULL;
    int count = 0;
    int i;

    pState = RallyData_GetChampionshipState();
    switch ((*pState >> 3) & 7) {
    case 1:
        count = 8;
        pMatch = (KnockoutMatch *)(pState + 0x16);
        break;
    case 2:
        count = 4;
        pMatch = (KnockoutMatch *)(pState + 10);
        break;
    case 3:
        count = 2;
        pMatch = (KnockoutMatch *)(pState + 4);
        break;
    case 4:
        count = 1;
        pMatch = (KnockoutMatch *)(pState + 1);
    }
    for (i = 0; i < count; i++, pMatch++) {
        if (FUN_00472990(pMatch))
            return 1;
    }
    return 0;
}

// Whether both drivers of the current round are known.
// FUNCTION: CMR2 0x00473310
int FUN_00473310(void)
{
    unsigned int first;
    unsigned int second;

    RallyData_GetChampionshipState();
    RallyData_GetRoundDrivers(&first, &second);
    if (RallyData_FUN_00408500((BYTE)first) != -1 && RallyData_FUN_00408500((BYTE)second) != -1)
        return 1;
    return 0;
}

// GLOBAL: CMR2 0x00591490
int g_unk0x00591490;
// GLOBAL: CMR2 0x00591494
int g_unk0x00591494;
// GLOBAL: CMR2 0x00591498
FixVector g_unk0x00591498;

// True when two spheres (radii r1, r2) overlap.
// match 51%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00487b80
bool FUN_00487b80(int r1, int r2, int *pA, int *pB)
{
    int r = r1 + r2;
    int dx = pA[0] - pB[0];
    int dy = pA[1] - pB[1];
    int dz = pA[2] - pB[2];

    if ((dx < 0 ? -dx : dx) <= r && (dy < 0 ? -dy : dy) <= r && (dz < 0 ? -dz : dz) <= r)
        return FixMul(dz, dz) + FixMul(dx, dx) + FixMul(dy, dy) < FixMul(r, r);
    return false;
}

// FUNCTION: CMR2 0x00487e00
void FUN_00487e00(FixVector *pPos, int *pInfo)
{
    g_unk0x00591498 = *pPos;
    g_unk0x00591490 = *(int *)pInfo[1];
    g_unk0x00591494 = *(int *)(*(int *)(*(int *)(pInfo[0] + 0xc) + 0x10c) + 0x4c);
}

// Updates a car's stage shadow/light when its position, projected on the two
// box axes, is inside the light box (with the global tolerance).
// The original hoists both tolerance'd limits before testing the absolute
// values, which is what the two named locals reproduce.
// FUNCTION: CMR2 0x00487e50
void FUN_00487e50(int *pBox, Car *pCar)
{
    FixVector delta;
    int u;
    int v;

    delta.x = g_unk0x00591498.x - ((int *)pBox[0x25])[0];
    delta.y = g_unk0x00591498.y - ((int *)pBox[0x25])[1];
    delta.z = g_unk0x00591498.z - ((int *)pBox[0x25])[2];
    delta.y = 0;
    u = FixVecDot(&delta, (FixVector *)(pBox + 4));
    v = FixVecDot(&delta, (FixVector *)(pBox + 7));
    if (FIX_ABS(u) > pBox[0] && FIX_ABS(v) > pBox[1])
        return;
    {
        int limit0;
        int limit1;
        limit0 = pBox[0] + g_unk0x00591490;
        limit1 = pBox[1] + g_unk0x00591490;
        if (FIX_ABS(u) > limit0 || FIX_ABS(v) > limit1)
            return;
    }
    FUN_0048df50(pCar);
}

// GLOBAL: CMR2 0x0051fadc
BYTE g_unk0x0051fadc[4] = { 0, 1, 3, 2 };
// GLOBAL: CMR2 0x00590ec8
BYTE g_unk0x00590ec8[4];
// GLOBAL: CMR2 0x00590ecc
char g_unk0x00590ecc[4];
// GLOBAL: CMR2 0x005914a4
BYTE g_unk0x005914a4[4];
// GLOBAL: CMR2 0x005914c4
char g_unk0x005914c4[4];
// GLOBAL: CMR2 0x005914d4
char g_unk0x005914d4;
// GLOBAL: CMR2 0x005915f4
char g_unk0x005915f4;

// Finds the closest overlap between two 4-corner boxes along the horizontal
// direction `pDir`, testing every pair of edges of their corner quads. Appends
// the winning contact (edge orientation 0=horizontal/2=vertical plus the corner
// index) to the A or B side list and returns 1; returns 0 if nothing overlaps.
// FUNCTION: CMR2 0x00488de0
int FUN_00488de0(FixVector *pVertsA, FixVector *pVertsB, FixVector *pDir, int *pDistance)
{
    BYTE *pNext;
    BYTE *pCur;
    BYTE corner;
    BYTE cornerVert;
    BYTE edge;
    int rayEdge;
    int best;
    int found;
    int dist;
    int diff;
    int i;
    int j;

    edge = 0;
    corner = 0;
    found = 0;
    best = -0x640000;
    for (i = 0; i < 4; i++) {
        pNext = &g_unk0x0051fadc[(i + 1) % 4];
        for (j = 1; j <= 4; j++) {
            pCur = &g_unk0x0051fadc[j % 4];
            g_collisionQuad[0] = pVertsA[g_unk0x0051fadc[i] + 4];
            g_collisionQuad[1] = pVertsA[*pNext + 4];
            g_collisionQuad[2] = pVertsB[g_unk0x0051fadc[j - 1] + 4];
            g_collisionQuad[3] = pVertsB[*pCur + 4];
            dist = Collision_RayQuad(pDir, &rayEdge, &corner);
            if (dist != 0x7d000000 && dist > best) {
                best = dist;
                switch (corner & 0xff) {
                case 0:
                    cornerVert = g_unk0x0051fadc[i];
                    found = 1;
                    diff = (int)g_unk0x0051fadc[j - 1] - (int)*pCur;
                    if (diff < 0)
                        diff = -diff;
                    edge = (diff == 1) ? 0 : 2;
                    break;
                case 1:
                    cornerVert = *pNext;
                    found = 1;
                    diff = (int)g_unk0x0051fadc[j - 1] - (int)*pCur;
                    if (diff < 0)
                        diff = -diff;
                    edge = (diff == 1) ? 0 : 2;
                    break;
                case 2:
                    cornerVert = g_unk0x0051fadc[j - 1];
                    found = 0;
                    diff = (int)g_unk0x0051fadc[i] - (int)*pNext;
                    if (diff < 0)
                        diff = -diff;
                    edge = (diff == 1) ? 0 : 2;
                    break;
                case 3:
                    cornerVert = *pCur;
                    found = 0;
                    diff = (int)g_unk0x0051fadc[i] - (int)*pNext;
                    if (diff < 0)
                        diff = -diff;
                    edge = (diff == 1) ? 0 : 2;
                    break;
                }
            }
        }
    }
    if (best < 0)
        return 0;
    if (found != 0) {
        int index = g_unk0x005915f4;
        g_unk0x00590ec8[index] = edge;
        g_unk0x005914c4[index] = cornerVert;
        g_unk0x005915f4++;
        *pDistance = best;
        return 1;
    }
    {
        int index = g_unk0x005914d4;
        g_unk0x005914a4[index] = edge;
        g_unk0x00590ecc[index] = cornerVert;
        g_unk0x005914d4++;
        *pDistance = best;
        return 1;
    }
}

extern int g_unk0x0051bd40;
extern int g_unk0x0051bd3c;

// Sets the scale 0x51bd40 (value / 25, at least 0.6) and its reciprocal 0x51bd3c.
// FUNCTION: CMR2 0x00466630
void FUN_00466630(int value)
{
    g_unk0x0051bd40 = FixMul(0xa3d, value);
    if (g_unk0x0051bd40 < 0x9999)
        g_unk0x0051bd40 = 0x9999;
    g_unk0x0051bd3c = FixDiv(0x10000, g_unk0x0051bd40);
}

extern FixVector g_stageDeformHull[12];
extern FixVector g_stageDeformOffset;
extern FixVector g_stageDeformNormal;
extern FixVector g_stageDeformImpact;
extern int g_stageDeformSpeed;
extern int g_stageDeformStrength;
extern int g_stageDeformMode;

// Rebuilds the deformation hull against the other car's velocity: the four
// source vertices come from its velocity/velocityNext, the four middle ones
// from the car's own bounds, and the last four are the input points clamped
// by the car scales.
// match 48%: MSVC biased the hull pointer walk differently (same logic)
// match 48%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004675c0
void FUN_004675c0(Car *pCar, Car *pOther)
{
    FixVector *pOut;
    FixVector *pIn;
    int v;

    g_stageDeformHull[0].x = pOther->velocity.z;
    g_stageDeformHull[0].y = -pCar->halfExtents.y;
    g_stageDeformHull[0].z = pOther->velocityNext.y;
    g_stageDeformHull[1].x = pOther->velocity.z;
    g_stageDeformHull[1].y = -pCar->halfExtents.y;
    g_stageDeformHull[1].z = pOther->velocityNext.z;
    g_stageDeformHull[2].x = pOther->velocityNext.x;
    g_stageDeformHull[2].y = -pCar->halfExtents.y;
    g_stageDeformHull[2].z = pOther->velocityNext.y;
    g_stageDeformHull[3].x = pOther->velocityNext.x;
    g_stageDeformHull[3].y = -pCar->halfExtents.y;
    g_stageDeformHull[3].z = pOther->velocityNext.z;
    pOut = &g_stageDeformHull[8];
    pIn = (FixVector *)pCar->field_0x240;
    do {
        pOut[-4] = pOut[-8];
        if ((int)pOut < (int)&g_stageDeformHull[10])
            v = *(int *)pCar->field_0x770;
        else
            v = *(int *)(pCar->field_0x770 + 4);
        pOut[-4].y += v;
        *pOut = *pIn;
        if (pOut->x > 0)
            pOut->x -= pCar->scale0x764;
        else
            pOut->x += pCar->scale0x768;
        if (pOut->z > 0)
            pOut->z -= pCar->scale0x76c;
        else
            pOut->z += pCar->scale0x76c;
        pOut++;
        pIn++;
    } while ((int)pOut < (int)&g_stageDeformHull[12]);
}

// Builds the deformation offset, normal and impact vectors plus the strength
// and mode flags from a per-car byte record (angles, break flags, scale).
// match 89%: the original folds the 0x10000<<16 division into a plain IDIV in
// the two later scale blocks, and stores the mode as a byte (declared int in
// StageTiming.cpp)
// FUNCTION: CMR2 0x004688b0
void FUN_004688b0(BYTE *p)
{
    g_stageDeformOffset.x = (char)p[9] << 16;
    g_stageDeformOffset.y = (char)p[10] << 16;
    g_stageDeformOffset.z = (char)p[11] << 16;
    FixVecScale(&g_stageDeformOffset, &g_stageDeformOffset,
                FixMul(0xa0000, FixDiv(0x10000, 0x7f0000)));
    g_stageDeformNormal.x = (char)p[3] << 16;
    g_stageDeformNormal.y = (char)p[4] << 16;
    g_stageDeformNormal.z = (char)p[5] << 16;
    FixVecScaleRecip(&g_stageDeformNormal, &g_stageDeformNormal, 0x7f0000);
    g_stageDeformImpact.x = (char)p[6] << 16;
    g_stageDeformImpact.y = (char)p[7] << 16;
    g_stageDeformImpact.z = (char)p[8] << 16;
    FixVecScaleRecip(&g_stageDeformImpact, &g_stageDeformImpact, 0x7f0000);
    g_stageDeformStrength = (BYTE)p[0] << 16;
    g_stageDeformStrength = FixDiv(g_stageDeformStrength, 0xff0000);
    g_stageDeformMode = p[1];
    if (p[1] == 1) {
        g_stageDeformSpeed = (BYTE)p[2] << 16;
        g_stageDeformSpeed = FixMul(g_stageDeformSpeed, FixMul(0xa0000, FixDiv(0x10000, 0xff0000)));
    }
}

extern BYTE *g_unk0x00588b94;

int FUN_00469100(Car *pCar, BYTE *pRecord);

// Rebuilds the per-wheel/gear block (0x240..0x2c4) of the car's 0x4d0-byte
// record from its source block (0x21c..), scales it, then recomputes the
// derived torques and scales (0x3d8..0x408).
// match 88%: same code; MSVC kept the cached record fields in EDX/EAX in the
// original and in EDI here (register numbering).
// FUNCTION: CMR2 0x00468c10
void FUN_00468c10(Car *pCar)
{
    BYTE *pRecord;
    int *p;
    int i;
    int a;
    int b;
    int value;

    pRecord = g_unk0x00588b94 + pCar->field_0xb1a * 0x4d0;
    if (*(int *)pCar->field_0xb50 == 0)
        return;
    p = (int *)(pRecord + 0x240);
    p[0] = *(int *)(pRecord + 0x234);
    *(int *)(pRecord + 0x244) = *(int *)(pRecord + 0x21c);
    *(int *)(pRecord + 0x248) = *(int *)(pRecord + 0x23c);
    *(int *)(pRecord + 0x24c) = *(int *)(pRecord + 0x224);
    *(int *)(pRecord + 0x254) = *(int *)(pRecord + 0x21c);
    *(int *)(pRecord + 0x250) = *(int *)(pRecord + 0x234);
    *(int *)(pRecord + 0x258) = *(int *)(pRecord + 0x234);
    *(int *)(pRecord + 0x25c) = *(int *)(pRecord + 0x21c);
    *(int *)(pRecord + 0x260) = *(int *)(pRecord + 0x23c);
    *(int *)(pRecord + 0x264) = *(int *)(pRecord + 0x224);
    *(int *)(pRecord + 0x268) = *(int *)(pRecord + 0x234);
    *(int *)(pRecord + 0x26c) = *(int *)(pRecord + 0x21c);
    *(int *)(pRecord + 0x270) = *(int *)(pRecord + 0x23c);
    *(int *)(pRecord + 0x274) = *(int *)(pRecord + 0x224);
    *(int *)(pRecord + 0x278) = *(int *)(pRecord + 0x228);
    *(int *)(pRecord + 0x27c) = *(int *)(pRecord + 0x228);
    *(int *)(pRecord + 0x280) = FixMul(*(int *)(pRecord + 0x22c) + *(int *)(pRecord + 0x228) +
                                       *(int *)(pRecord + 0x230), 0x5553);
    *(int *)(pRecord + 0x28c) = *(int *)(pRecord + 0x228);
    *(int *)(pRecord + 0x288) = *(int *)(pRecord + 0x230);
    *(int *)(pRecord + 0x290) = *(int *)(pRecord + 0x230);
    *(int *)(pRecord + 0x294) = *(int *)(pRecord + 0x230);
    *(int *)(pRecord + 0x2a0) = *(int *)(pRecord + 0x230);
    *(int *)(pRecord + 0x298) = *(int *)(pRecord + 0x228);
    *(int *)(pRecord + 0x29c) = *(int *)(pRecord + 0x228);
    *(int *)(pRecord + 0x284) = 0;
    *(int *)(pRecord + 0x2a4) = FixMul(*(int *)(pRecord + 0x230) + *(int *)(pRecord + 0x228) +
                                       *(int *)(pRecord + 0x22c), 0x5553);
    *(int *)(pRecord + 0x2a8) = *(int *)(pRecord + 0x228);
    *(int *)(pRecord + 0x2ac) = *(int *)(pRecord + 0x230);
    *(int *)(pRecord + 0x2b0) = *(int *)(pRecord + 0x238);
    *(int *)(pRecord + 0x2b4) = *(int *)(pRecord + 0x220);
    *(int *)(pRecord + 0x2b8) = *(int *)(pRecord + 0x23c);
    *(int *)(pRecord + 0x2bc) = *(int *)(pRecord + 0x224);
    *(int *)(pRecord + 0x2c0) = *(int *)(pRecord + 0x234);
    *(int *)(pRecord + 0x2c4) = *(int *)(pRecord + 0x21c);
    if (*(int *)pCar->field_0xb7c == 0)
        *(int *)(pRecord + 0x27c) = 0;
    if (*(int *)(pCar->field_0xb7c + 4) == 0)
        *(int *)(pRecord + 0x280) = 0;
    for (i = 0; i < 0x22; i++) {
        p[i] = FixMul(p[i], p[i + 0x22]);
        p[i] = p[i] + p[i + 0x44];
        if (p[i] > 0x10000)
            p[i] = 0x10000;
    }
    *(int *)(pRecord + 0x404) = 0x10000 - FixMul(*(int *)(pRecord + 0x2a0), 0x666) -
                                FixMul(*(int *)(pRecord + 0x27c), 0x1333) -
                                FixMul(*(int *)(pRecord + 0x2a4), 0x2666);
    *(int *)(pRecord + 0x3dc) = FixMul(*(int *)(pRecord + 0x258), FixMul(0x3333, 0xffff0000));
    *(int *)(pRecord + 0x3e0) = FixMul(*(int *)(pRecord + 0x25c), FixMul(0x3333, 0xffff0000));
    *(int *)(pRecord + 0x3e4) = FixMul(*(int *)(pRecord + 0x260), FixMul(0x3333, 0x8000));
    *(int *)(pRecord + 0x3e8) = FixMul(*(int *)(pRecord + 0x264), FixMul(0x3333, 0xffff8000));
    *(int *)(pRecord + 0x3d8) = FixMul(*(int *)(pRecord + 0x250) * 2, 0x8000);
    *(int *)(pRecord + 0x3d8) = FixMul(*(int *)(pRecord + 0x3d8), 0xa0000);
    *(int *)(pRecord + 0x3fc) = 0x10000 - FixMul(FixMul(*(int *)(pRecord + 0x268) +
                                                         *(int *)(pRecord + 0x26c), 0x8000), 0x3333);
    *(int *)(pRecord + 0x400) = 0x10000 - FixMul(FixMul(*(int *)(pRecord + 0x270) +
                                                         *(int *)(pRecord + 0x274), 0x8000), 0x3333);
    p = (int *)(pRecord + 0x3ec);
    for (i = 0; i < 4; i++) {
        p[i] = FixMul(p[i - 0x6b], 0xccc);
    }
    if (*(int *)pCar->field_0x7b8 != 0x10000 && *(int *)pCar->field_0x7b8 != 0) {
        value = FixMul(*(int *)(pRecord + 0x280), 0x8000) + *(int *)pCar->field_0x7b8;
        pCar->field_0x7b4 = value;
        if (value > 0x10000)
            pCar->field_0x7b4 = 0x10000;
    }
    *(char *)(pRecord + 0x468) = (char)FixMulShift32(*(int *)(pRecord + 0x278), 0xf0000);
    value = FUN_00469100(pCar, pRecord);
    *(int *)(pRecord + 0x408) = FixMul(value, 0x4000);
    if (FUN_00469bc0(pCar, 3) != 0)
        *(int *)(pRecord + 0x408) = *(int *)(pRecord + 0x408) + -0x3333;
    *(int *)(pRecord + 0x284) = value;
    if (value > 0x10000)
        *(int *)(pRecord + 0x284) = 0x10000;
}

// Adds `amount` to the 3x3 grid at +0x21c of the car's 0x4d0-byte record,
// weighted by how close each grid point is to the car's contact offsets
// (0x5dc/0x5e4).
// match 50%: same logic; MSVC6 kept the car pointer in EDI and the FixMul
// temporaries in the parameter slots instead of the slots we get.
// FUNCTION: CMR2 0x00468a80
void FUN_00468a80(Car *pCar, int amount)
{
    int *pGrid;
    int *pElem;
    int row;
    int i;
    int xOff;
    int yOff;
    int halfAmount;
    int gridStep;
    int xStep;
    int yStep;
    int xLimit;
    int yLimit0;
    int yLimit1;
    int colLimit;

    pGrid = (int *)(g_unk0x00588b94 + pCar->field_0xb1a * 0x4d0 + 0x21c);
    halfAmount = FixMul(amount, 0x8000);
    gridStep = FixMul(amount, 0x3333);
    xStep = FixMul(pCar->halfExtents.x, 0xaac0);
    yStep = FixMul(pCar->halfExtents.z, 0xc000);
    xLimit = FixMul(pCar->halfExtents.x, 0x553f) + halfAmount;
    yLimit0 = FixMul(pCar->halfExtents.z, 0x4000) + halfAmount;
    yLimit1 = FixMul(pCar->halfExtents.z, 0x8000) + halfAmount;
    row = 0;
    yOff = -yStep;
    for (row = 0; row < 3; row++) {
        colLimit = row == 1 ? yLimit1 : yLimit0;
        xOff = xStep;
        pElem = pGrid;
        for (i = 0; i < 3; i++) {
            if (FIX_ABS(*(int *)((BYTE *)pCar + 0x5dc) - xOff) <= xLimit &&
                FIX_ABS(*(int *)((BYTE *)pCar + 0x5e4) - yOff) <= colLimit) {
                *pElem += gridStep;
                if (*pElem > 0x640000)
                    *pElem = 0x640000;
            }
            xOff -= xStep;
            pElem++;
        }
        pGrid += 3;
        yOff += yStep;
    }
}
// Averages (16.16) the per-object distance between every stage object's float
// vertex data and its fixed-point copy, skipping parts 1 and 3 when the record
// flag is set (their object count still feeds the divisor).
// FUNCTION: CMR2 0x00469100
int FUN_00469100(Car *pCar, BYTE *pRecord)
{
    int partA;
    int partB;
    int total;
    int sum;
    int entry;
    int *pCounts;
    int i;
    int offsetDst;
    int offsetSrc;

    if (g_unk0x00588970[pCar->field_0xb1a] == 0)
        return 0;
    partA = FUN_00484d10(1, pCar->field_0xb1a);
    partB = FUN_00484d10(3, pCar->field_0xb1a);
    total = 0;
    sum = 0;
    entry = 0;
    pCounts = (int *)(pRecord + 0x420);
    if (*(int *)(pRecord + 0x45c) > 0) {
        do {
            if (entry == partA) {
                if (FUN_00469bc0(pCar, 1) != 0) {
                    total += *pCounts;
                    goto next;
                }
            } else if (entry == partB) {
                if (FUN_00469bc0(pCar, 3) != 0) {
                    total += *pCounts;
                    goto next;
                }
            }
            i = 0;
            if (*pCounts > 0) {
                offsetDst = 0;
                offsetSrc = 0;
                do {
                    int *pDst = (int *)(*(int *)(pRecord + 0x78 + entry * 4) + offsetDst);
                    float *pSrc = (float *)(*(int *)(*(int *)(pRecord + entry * 4) + 0xc) + offsetSrc);
                    int dx = (int)(__int64)((double)pSrc[0] * CGraphics::m_65536) - pDst[0];
                    int dy = (int)(__int64)((double)pSrc[1] * CGraphics::m_65536) - pDst[1];
                    int dz = (int)(__int64)((double)pSrc[2] * CGraphics::m_65536) - pDst[2];
                    int v;

                    if (dx < 0)
                        dx = -dx;
                    if (dy < 0)
                        dy = -dy;
                    if (dz < 0)
                        dz = -dz;
                    v = FixMul(dz + dy + dx, 0x50000);
                    if (v > 0x10000)
                        v = 0x10000;
                    offsetSrc += 0x30;
                    sum += v;
                    total++;
                    i++;
                    offsetDst += 0x20;
                } while (i < *pCounts);
            }
next:
            entry++;
            pCounts++;
        } while (entry < *(int *)(pRecord + 0x45c));
    }
    return FixDiv(sum, total << 16);
}

// Sets the fixed-point lighting values for both stage weather conditions.
// match 24%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00460da0
void StageObject_SetLighting(const BYTE *pPrimary, const BYTE *pSecondary)
{
    Stage_InitLightMeshes();
    *(WORD *)&g_stageLighting[0x5c] = 0xffff;
    int *pWeatherPair = FUN_00407520(RallyDataStageIndex());
    g_stageLighting[0x2c] = g_stageWeatherIntensity[pWeatherPair[0]];
    g_stageLighting[0x59] = g_stageWeatherIntensity[pWeatherPair[1]];
    if (pPrimary != NULL && pSecondary != NULL) {
        g_stageLighting[0x1b] = *(int *)pPrimary;
        g_stageLighting[0x1c] = *(int *)(pPrimary + 0x4);
        g_stageLighting[0x1d] = *(int *)(pPrimary + 0x8);
        g_stageLighting[0x48] = *(int *)pSecondary;
        g_stageLighting[0x49] = *(int *)(pSecondary + 0x4);
        g_stageLighting[0x4a] = *(int *)(pSecondary + 0x8);
        FUN_004925c0(g_stageLighting[0x1b], g_stageLighting[0x1c], g_stageLighting[0x1d]);
        FUN_00492900(0x10000);
        g_stageLighting[0x0] = (unsigned int)pPrimary[0x20] * 0x10000;
        g_stageLighting[0x1] = (unsigned int)pPrimary[0x21] * 0x10000;
        g_stageLighting[0x2] = (unsigned int)pPrimary[0x22] * 0x10000;
        g_stageLighting[0x3] = (unsigned int)pPrimary[0x1c] * 0x10000 + (unsigned int)pPrimary[0x20] * -0x10000;
        g_stageLighting[0x4] = (unsigned int)pPrimary[0x1d] * 0x10000 + (unsigned int)pPrimary[0x21] * -0x10000;
        g_stageLighting[0x5] = (unsigned int)pPrimary[0x1e] * 0x10000 + (unsigned int)pPrimary[0x22] * -0x10000;
        g_stageLighting[0x2d] = (unsigned int)pSecondary[0x20] * 0x10000;
        g_stageLighting[0x2e] = (unsigned int)pSecondary[0x21] * 0x10000;
        g_stageLighting[0x2f] = (unsigned int)pSecondary[0x22] * 0x10000;
        g_stageLighting[0x30] = (unsigned int)pSecondary[0x1c] * 0x10000 + (unsigned int)pSecondary[0x20] * -0x10000;
        g_stageLighting[0x31] = (unsigned int)pSecondary[0x1d] * 0x10000 + (unsigned int)pSecondary[0x21] * -0x10000;
        g_stageLighting[0x32] = (unsigned int)pSecondary[0x1e] * 0x10000 + (unsigned int)pSecondary[0x22] * -0x10000;
        g_stageLighting[0x6] = (unsigned int)pPrimary[0x24] << 0x10;
        g_stageLighting[0x7] = (unsigned int)pPrimary[0x25] << 0x10;
        g_stageLighting[0x8] = (unsigned int)pPrimary[0x26] << 0x10;
        g_stageLighting[0x33] = (unsigned int)pSecondary[0x24] << 0x10;
        g_stageLighting[0x34] = (unsigned int)pSecondary[0x25] << 0x10;
        g_stageLighting[0x35] = (unsigned int)pSecondary[0x26] << 0x10;
        g_stageLighting[0x27] = FixDiv((int)pPrimary[0x27] << 16, 0xff0000);
        g_stageLighting[0x54] = FixDiv((int)pSecondary[0x27] << 16, 0xff0000);
        g_stageLighting[0x9] = (unsigned int)pPrimary[0x3c] << 0x10;
        g_stageLighting[0xa] = (unsigned int)pPrimary[0x3d] << 0x10;
        g_stageLighting[0xb] = (unsigned int)pPrimary[0x3e] << 0x10;
        g_stageLighting[0x36] = (unsigned int)pSecondary[0x3c] << 0x10;
        g_stageLighting[0x37] = (unsigned int)pSecondary[0x3d] << 0x10;
        g_stageLighting[0x38] = (unsigned int)pSecondary[0x3e] << 0x10;
        g_stageLighting[0x2b] = *(int *)(pPrimary + 0x10);
        g_stageLighting[0x58] = *(int *)(pSecondary + 0x10);
        g_stageLighting[0x18] = (unsigned int)pPrimary[0x34] << 0x10;
        g_stageLighting[0x19] = (unsigned int)pPrimary[0x35] << 0x10;
        g_stageLighting[0x1a] = (unsigned int)pPrimary[0x36] << 0x10;
        g_stageLighting[0x45] = (unsigned int)pSecondary[0x34] << 0x10;
        g_stageLighting[0x46] = (unsigned int)pSecondary[0x35] << 0x10;
        g_stageLighting[0x47] = (unsigned int)pSecondary[0x36] << 0x10;
        g_stageLighting[0x2a] = (unsigned int)pPrimary[0x37] << 0x10;
        g_stageLighting[0x57] = (unsigned int)pSecondary[0x37] << 0x10;
        g_stageLighting[0x15] = (unsigned int)pPrimary[0x38] << 0x10;
        g_stageLighting[0x16] = (unsigned int)pPrimary[0x39] << 0x10;
        g_stageLighting[0x17] = (unsigned int)pPrimary[0x3a] << 0x10;
        g_stageLighting[0x42] = (unsigned int)pSecondary[0x38] << 0x10;
        g_stageLighting[0x43] = (unsigned int)pSecondary[0x39] << 0x10;
        g_stageLighting[0x44] = (unsigned int)pSecondary[0x3a] << 0x10;
        g_stageLighting[0xc] = (unsigned int)pPrimary[0x28] << 0x10;
        g_stageLighting[0xd] = (unsigned int)pPrimary[0x29] << 0x10;
        g_stageLighting[0xe] = (unsigned int)pPrimary[0x2a] << 0x10;
        g_stageLighting[0x39] = (unsigned int)pSecondary[0x28] << 0x10;
        g_stageLighting[0x3a] = (unsigned int)pSecondary[0x29] << 0x10;
        g_stageLighting[0x3b] = (unsigned int)pSecondary[0x2a] << 0x10;
        g_stageLighting[0x28] = FixDiv((int)pPrimary[0x2b] << 16, 0xff0000);
        g_stageLighting[0x55] = FixDiv((int)pSecondary[0x2b] << 16, 0xff0000);
        g_stageLighting[0xf] = (unsigned int)pPrimary[0x2c] << 0x10;
        g_stageLighting[0x10] = (unsigned int)pPrimary[0x2d] << 0x10;
        g_stageLighting[0x11] = (unsigned int)pPrimary[0x2e] << 0x10;
        g_stageLighting[0x3c] = (unsigned int)pSecondary[0x2c] << 0x10;
        g_stageLighting[0x3d] = (unsigned int)pSecondary[0x2d] << 0x10;
        g_stageLighting[0x3e] = (unsigned int)pSecondary[0x2e] << 0x10;
        g_stageLighting[0x29] = (int)((unsigned int)pPrimary[0x2f] << 0x10);
        g_stageLighting[0x56] = (int)((unsigned int)pSecondary[0x2f] << 0x10);
        g_stageLighting[0x12] = (unsigned int)pPrimary[0x30] << 0x10;
        g_stageLighting[0x13] = (unsigned int)pPrimary[0x31] << 0x10;
        g_stageLighting[0x14] = (unsigned int)pPrimary[0x32] << 0x10;
        g_stageLighting[0x3f] = (unsigned int)pSecondary[0x30] << 0x10;
        g_stageLighting[0x40] = (unsigned int)pSecondary[0x31] << 0x10;
        g_stageLighting[0x41] = (unsigned int)pSecondary[0x32] << 0x10;
        g_stageLighting[0x1e] = (unsigned int)pPrimary[0x44] << 0x10;
        g_stageLighting[0x1f] = (unsigned int)pPrimary[0x45] << 0x10;
        g_stageLighting[0x20] = (unsigned int)pPrimary[0x46] << 0x10;
        g_stageLighting[0x25] = *(int *)(pPrimary + 0x14);
        g_stageLighting[0x26] = *(int *)(pPrimary + 0x18);
        g_stageLighting[0x4b] = (unsigned int)pSecondary[0x44] << 0x10;
        g_stageLighting[0x4c] = (unsigned int)pSecondary[0x45] << 0x10;
        g_stageLighting[0x4d] = (unsigned int)pSecondary[0x46] << 0x10;
        g_stageLighting[0x52] = *(int *)(pSecondary + 0x14);
        g_stageLighting[0x53] = *(int *)(pSecondary + 0x18);
        g_stageLighting[0x21] = (unsigned int)pPrimary[0x40] << 0x10;
        g_stageLighting[0x22] = (unsigned int)pPrimary[0x41] << 0x10;
        g_stageLighting[0x23] = (unsigned int)pPrimary[0x42] << 0x10;
        g_stageLighting[0x24] = (unsigned int)pPrimary[0x43] << 0x10;
        g_stageLighting[0x4e] = (unsigned int)pSecondary[0x40] << 0x10;
        g_stageLighting[0x4f] = (unsigned int)pSecondary[0x41] << 0x10;
        g_stageLighting[0x50] = (unsigned int)pSecondary[0x42] << 0x10;
        g_stageLighting[0x51] = (unsigned int)pSecondary[0x43] << 0x10;
    } else {
        g_stageLighting[0x1b] = 0xfffb0000;
        g_stageLighting[0x1c] = 0xffda0000;
        g_stageLighting[0x1d] = 0xfff30000;
        g_stageLighting[0x48] = 0xfffb0000;
        g_stageLighting[0x49] = 0xffda0000;
        g_stageLighting[0x4a] = 0xfff30000;
        FUN_004925c0(0xfffb0000, 0xffda0000, 0xfff30000);
        FUN_00492900(0x10000);
        g_stageLighting[0x3] = -0x5d0000;
        g_stageLighting[0x6] = 0xff0000;
        g_stageLighting[0x30] = -0x5d0000;
        g_stageLighting[0x4] = -0x620000;
        g_stageLighting[0x5] = 0;
        g_stageLighting[0x7] = 0xff0000;
        g_stageLighting[0x8] = 0xff0000;
        g_stageLighting[0x1e] = 0;
        g_stageLighting[0x1f] = 0;
        g_stageLighting[0x20] = 0;
        g_stageLighting[0x25] = 0;
        g_stageLighting[0x26] = 0;
        g_stageLighting[0x21] = 0;
        g_stageLighting[0x22] = 0;
        g_stageLighting[0x31] = -0x620000;
        g_stageLighting[0x32] = 0;
        g_stageLighting[0x9] = 0xff0000;
        g_stageLighting[0x33] = 0xff0000;
        g_stageLighting[0xa] = 0xff0000;
        g_stageLighting[0xb] = 0xff0000;
        g_stageLighting[0x34] = 0xff0000;
        g_stageLighting[0x35] = 0xff0000;
        g_stageLighting[0x2] = 0xff0000;
        g_stageLighting[0x36] = 0xff0000;
        g_stageLighting[0x0] = 0xbc0000;
        g_stageLighting[0x1] = 0xca0000;
        g_stageLighting[0x29] = 0x640000;
        g_stageLighting[0x2b] = 0xfa0000;
        g_stageLighting[0x27] = 0x8000;
        g_stageLighting[0x18] = 0xff0000;
        g_stageLighting[0x19] = 0xff0000;
        g_stageLighting[0x1a] = 0xff0000;
        g_stageLighting[0x2a] = 0xff0000;
        g_stageLighting[0x15] = 0xff0000;
        g_stageLighting[0x16] = 0xff0000;
        g_stageLighting[0x17] = 0xff0000;
        g_stageLighting[0xc] = 0x9b0000;
        g_stageLighting[0xd] = 0x9b0000;
        g_stageLighting[0xe] = 0x9b0000;
        g_stageLighting[0x28] = 0x8000;
        g_stageLighting[0xf] = 0xff0000;
        g_stageLighting[0x10] = 0xff0000;
        g_stageLighting[0x11] = 0xff0000;
        g_stageLighting[0x12] = 0xc80000;
        g_stageLighting[0x13] = 0xc80000;
        g_stageLighting[0x14] = 0xc80000;
        g_stageLighting[0x23] = 0;
        g_stageLighting[0x24] = 0;
        g_stageLighting[0x37] = 0xff0000;
        g_stageLighting[0x38] = 0xff0000;
        g_stageLighting[0x58] = 0xfa0000;
        g_stageLighting[0x2d] = 0xbc0000;
        g_stageLighting[0x2e] = 0xca0000;
        g_stageLighting[0x45] = 0xff0000;
        g_stageLighting[0x46] = 0xff0000;
        g_stageLighting[0x47] = 0xff0000;
        g_stageLighting[0x42] = 0xff0000;
        g_stageLighting[0x43] = 0xff0000;
        g_stageLighting[0x44] = 0xff0000;
        g_stageLighting[0x39] = 0x9b0000;
        g_stageLighting[0x2f] = 0xff0000;
        g_stageLighting[0x3a] = 0x9b0000;
        g_stageLighting[0x3b] = 0x9b0000;
        g_stageLighting[0x54] = 0x8000;
        g_stageLighting[0x55] = 0x8000;
        g_stageLighting[0x3c] = 0xff0000;
        g_stageLighting[0x3e] = 0xff0000;
        g_stageLighting[0x3d] = 0xff0000;
        g_stageLighting[0x3f] = 0xc80000;
        g_stageLighting[0x41] = 0xc80000;
        g_stageLighting[0x40] = 0xc80000;
        g_stageLighting[0x4b] = 0;
        g_stageLighting[0x4d] = 0;
        g_stageLighting[0x4c] = 0;
        g_stageLighting[0x4e] = 0;
        g_stageLighting[0x57] = 0xff0000;
        g_stageLighting[0x56] = 0x640000;
        g_stageLighting[0x52] = 0;
        g_stageLighting[0x53] = 0;
        g_stageLighting[0x4f] = 0;
        g_stageLighting[0x50] = 0;
        g_stageLighting[0x51] = 0;
    }
    g_stageLighting[0x5a] = 0xffff0000;
    if (g_stageLighting[0x25] == 0 && g_stageLighting[0x26] == 0 &&
        g_stageLighting[0x52] == 0 && g_stageLighting[0x53] == 0) {
        FUN_00492fd0(0);
        g_stageLighting[0x5d] = 0;
        return;
    }
    FUN_00492fd0(1);
    g_stageLighting[0x5d] = 1;
}

// Blends two byte values: b + (a - b) * t, clamped to 255.
// match 83%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004616c0
int FUN_004616c0(BYTE a, BYTE b, int t)
{
    int v;

    v = (((int)b << 16) + FixMul(((int)a << 16) - ((int)b << 16), t)) >> 16;
    if (v > 0xff)
        v = 0xff;
    return v;
}

// GLOBAL: CMR2 0x005885a0
int g_unk0x005885a0[8][4];
// GLOBAL: CMR2 0x00549ba0
int g_unk0x00549ba0[8][4];

// Advances (mod 200) the counters of a car's four flagged slots and clears the flags.
// FUNCTION: CMR2 0x00464c60
void FUN_00464c60(int car)
{
    int i;

    if (car < 8) {
        for (i = 0; i < 4; i++) {
            if (g_unk0x005885a0[car][i] != 0) {
                g_unk0x005885a0[car][i] = 0;
                g_unk0x00549ba0[car][i] = (g_unk0x00549ba0[car][i] + 1) % 200;
            }
        }
    }
}

// Updates the four wheel skid trails and fades their colours by surface and slip.
// match 46%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00464cb0
void StageObject_UpdateSkidTrails(int carIndex)
{
    if (carIndex >= 8 || CGameInfo::FUN_00405cd0() == 2)
        return;

    Car *pCar = Car_Get(carIndex);
    if (carIndex == 0)
        ++g_trailFrame;

    int shortLifetime = ((FUN_00460bf0(carIndex) == 1 || FUN_00460bf0(carIndex) == 2) &&
                         FUN_00460c10(carIndex) > 0x3333);
    int baseColor = 0x00ffff00;
    int wheel;
    int pointIndex;
    for (wheel = 0; wheel < 4; ++wheel) {
        int *pBaseColor = FUN_00463270(carIndex, wheel);
        baseColor = pBaseColor != NULL ? *pBaseColor : 0x00ffff00;
        for (pointIndex = 0; pointIndex < 200; ++pointIndex) {
            BYTE *pPoint = g_trailPoints[carIndex][wheel][pointIndex];
            if (g_trailPointUsed[carIndex][wheel][pointIndex] != 0) {
                int lifetime = shortLifetime ? g_stageSurfaceInfo[8].flags << 2 :
                                               g_stageSurfaceInfo[8].flags * 0x50;
                if (lifetime < *(int *)pPoint) {
                    g_trailPointUsed[carIndex][wheel][pointIndex] = 0;
                    pPoint[0x24] = 0;
                    pPoint[0x25] = 0;
                }
            }
        }
    }

    for (wheel = 0; wheel < 4; ++wheel) {
        char currentSurface = (char)pCar->wheelSurface[wheel];
        char previousSurface = (char)g_trailSurface[carIndex][wheel];
        BYTE currentMaterial = g_stageSurfaceMap[currentSurface];
        BYTE previousMaterial = g_stageSurfaceMap[previousSurface];
        StageSurfaceInfo *pCurrent = &g_stageSurfaceInfo[currentMaterial];
        StageSurfaceInfo *pPrevious = &g_stageSurfaceInfo[previousMaterial];
        int activeSurface = g_trailTimer[carIndex][wheel] > 2 && g_trailCount[carIndex] > 1 &&
                            pCar->field_0xb74 == 1 && (pCurrent->flags & 1) != 0 &&
                            (pPrevious->flags & 1) != 0;
        int dualSurface = (pCurrent->flags & 2) != 0 && (pPrevious->flags & 2) != 0;

        if (pCar->field_0xbac[wheel] == 1 && (activeSurface || dualSurface)) {
            int slip = StageObject_GetWheelSlip(carIndex, wheel);
            if ((pCurrent->flags & 2) == 0 || pCar->field_0xbac[wheel ^ 2] == 0) {
                if (slip <= 0)
                    continue;
            } else {
                if (currentSurface == 0x19)
                    continue;
                slip = 0x8000;
            }

            FixVector *pDelta = &g_trailDelta[carIndex][wheel];
            if ((pDelta->x != 0 || pDelta->z != 0) && g_trailReset[carIndex][wheel] == 0) {
                g_unk0x005885a0[carIndex][wheel] = 1;
                int trailIndex = g_unk0x00549ba0[carIndex][wheel];
                BYTE *pPoint = g_trailPoints[carIndex][wheel][trailIndex];
                // The original keeps the colour selected by the first loop's last wheel.

                FixVector up = { 0, 0x10000, 0 };
                FixVector side;
                FixVecCross(&side, pDelta, &up);
                int length = FixVecLength(&side);
                if (length == 0) {
                    side.x = side.y = side.z = 0;
                } else {
                    FixVecScaleRecip(&side, &side, length);
                }
                FixVecScale(&side, &side, 0x1eb8);

                if (CGameInfo::FUN_004063f0(6) == 0) {
                    if (FUN_0042cae0(pCar, 1) != 0)
                        FixVecScale(&side, &side, 0x9999);
                } else {
                    FixVecScale(&side, &side, 0x28000);
                }

                FixVector *pPosition = &g_unk0x00549c20[carIndex][wheel];
                int yOffset = -0x20c - wheel * 0x83;
                *(int *)(pPoint + 8) = pPosition->x + side.x;
                *(int *)(pPoint + 0xc) = pPosition->y + side.y + yOffset;
                *(int *)(pPoint + 0x10) = pPosition->z + side.z;
                *(int *)(pPoint + 0x14) = pPosition->x - side.x;
                *(int *)(pPoint + 0x18) = pPosition->y - side.y + yOffset;
                *(int *)(pPoint + 0x1c) = pPosition->z - side.z;

                BYTE opacity = (BYTE)(FixMul(slip, 0xff0000) >> 16);
                pPoint[0x24] = opacity;
                pPoint[0x25] = opacity;
                BYTE oldMaterial = pPoint[0x26];
                *(int *)pPoint = 0;
                pPoint[0x26] = (oldMaterial & 0xf0) | (currentMaterial & 0x0f);
                g_trailPointUsed[carIndex][wheel][trailIndex] = 1;
                *(int *)(pPoint + 4) = g_trailFrame;

                BYTE *pColor = g_trailColor[carIndex];
                if ((pCurrent->flags & 2) == 0) {
                    pColor[0] = (BYTE)((int)pColor[0] + ((int)pPrevious->red - (int)pColor[0]) / 2);
                    pColor[1] = (BYTE)((int)pColor[1] + ((int)pPrevious->green - (int)pColor[1]) / 2);
                    pColor[2] = (BYTE)((int)pColor[2] + ((int)pPrevious->blue - (int)pColor[2]) / 2);
                    pPoint[0x20] = (BYTE)((int)pColor[0] * (baseColor & 0xff) / 0xff);
                    pPoint[0x21] = (BYTE)((int)pColor[1] * ((baseColor >> 8) & 0xff) / 0xff);
                    pPoint[0x22] = (BYTE)((int)pColor[2] * ((baseColor >> 16) & 0xff) / 0xff);
                    g_unk0x00543708[carIndex][wheel] = 1;
                    g_unk0x00549b20[carIndex][wheel] = 0;
                } else {
                    pColor[0] = (BYTE)((int)pPrevious->red * (baseColor & 0xff) / 0xff);
                    pColor[1] = (BYTE)((int)pPrevious->green * ((baseColor >> 8) & 0xff) / 0xff);
                    pColor[2] = (BYTE)((int)pPrevious->blue * ((baseColor >> 16) & 0xff) / 0xff);
                    *(int *)(pPoint + 0x20) = *(int *)pColor;
                    g_unk0x00543708[carIndex][wheel] = 0;
                    g_unk0x00549b20[carIndex][wheel] = 1;
                }

                BYTE *pNextPoint = g_trailPoints[carIndex][wheel][(trailIndex + 1) % 200];
                pNextPoint[0x24] = 0;
                pNextPoint[0x25] = 0;
            }
        } else if (g_unk0x00543708[carIndex][wheel] != 0 ||
                   g_unk0x00549b20[carIndex][wheel] != 0) {
            g_unk0x005885a0[carIndex][wheel] = 1;
        }
    }
}

// Object classes that fill the four per-car slots of 0x590b7c.
// GLOBAL: CMR2 0x0051f888
char g_carSlotClasses[4] = { 9, 10, 12, 13 };

// Stores an object in every car slot whose class matches it.
// FUNCTION: CMR2 0x00480af0
void FUN_00480af0(BYTE *pCar, BYTE *pObject, BYTE flag)
{
    int i;

    for (i = 3; i >= 0; i--) {
        if (g_carSlotClasses[i] == (char)pObject[0x30]) {
            g_unk0x00590b7c[i][(char)pCar[0xb1a]] = pObject;
            g_unk0x00590c24[i][(char)pCar[0xb1a]] = flag;
        }
    }
}

// Clears record `index` of the 0x48-byte table at 0x58d6d0.
// match 18%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00477ac0
void FUN_00477ac0(int index)
{
    BYTE *p = g_unk0x0058d6d0[index];
    int i;

    for (i = 0; i < 10; i++)
        ((short *)(p + 8))[i] = 0;
    for (i = 0; i < 10; i++)
        ((short *)(p + 0x30))[i] = 0;
    for (i = 0; i < 10; i++)
        ((short *)(p + 0x1c))[i] = -1;
    p[0x44] = 0;
    p[0x45] = 0;
}

SceneNode *SceneNode_FindByType(SceneNode *pNode, unsigned int type);
void FUN_004b2e40(BYTE *p, int value);
struct Unk0x004a3e20;
void FUN_004a3e20(Unk0x004a3e20 *pObject, int value);

// Sets up a car's damage record: clears it and keeps the textures of its two
// body parts (scene nodes of type 0xe).
// FUNCTION: CMR2 0x00477b60
void FUN_00477b60(int car, int unused1, int unused2, BYTE flag)
{
    BYTE *pRecord = g_unk0x0058d6d0[car];
    BYTE *pMesh;
    Texture *pTexture;

    pRecord[0x46] = flag;
    *(Texture **)(pRecord + 0) = NULL;
    *(Texture **)(pRecord + 4) = NULL;
    FUN_00477ac0(car);
    RallyData_ValidateIndex(car);
    pMesh = *(BYTE **)((BYTE *)SceneNode_FindByType(Car_Get(car)->pNode0x720, 0xe) + 0xc);
    FUN_004b2e40(pMesh, 0);
    pTexture = CGraphics::m_pTextureManager->textureBuffer[*(int *)(*(BYTE **)(pMesh + 0x24) + 4)];
    *(Texture **)(pRecord + 0) = pTexture;
    FUN_004a3e20((Unk0x004a3e20 *)pTexture, 2);
    if (Car_Get(car)->pNode0x724 != NULL) {
        pMesh = *(BYTE **)((BYTE *)SceneNode_FindByType(Car_Get(car)->pNode0x724, 0xe) + 0xc);
        FUN_004b2e40(pMesh, 0);
        pTexture = CGraphics::m_pTextureManager->textureBuffer[*(int *)(*(BYTE **)(pMesh + 0x24) + 4)];
        *(Texture **)(pRecord + 4) = pTexture;
        FUN_004a3e20((Unk0x004a3e20 *)pTexture, 2);
    }
}

// Sets or clears bits in the two flag bytes of record `index` of 0x58d6d0.
// FUNCTION: CMR2 0x00477c20
void FUN_00477c20(int index, char set0, char set1, BYTE mask)
{
    BYTE *p;

    p = g_unk0x0058d6d0[index];
    if (set0 != 0)
        p[0x44] |= mask;
    else
        p[0x44] &= ~mask;
    if (set1 != 0) {
        p[0x45] |= mask;
        return;
    }
    p[0x45] &= ~mask;
}

extern unsigned short *g_stageRandomTextures[3];
extern Mesh *g_stageMesh4Copy;

// Gives every triangle of the stage mesh one of the three random textures.
// match 50%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00492b50
void FUN_00492b50(void)
{
    int r;
    int n;
    int offset;

    r = rand() % 3;
    if (g_stageRandomTextures[r] != NULL) {
        n = g_stageMesh4Copy->triangleCount;
        if (n - 1 >= 0) {
            offset = (n - 1) * 0x4c;
            do {
                offset -= 0x4c;
                n--;
                // +0x50 from the previous triangle: the texture word (+4) of this one
                *(unsigned int *)((BYTE *)g_stageMesh4Copy->pTriangles + 0x50 + offset) = *g_stageRandomTextures[r];
            } while (n != 0);
        }
    }
}

extern int g_unk0x00590c64;
extern void *g_unk0x00590d7c[4];
int RallyData_FUN_00411060(void);

// Destroys, in the four node tables, the nodes of every car that belong to
// the current stage kind.
// match 52%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004866a0
void FUN_004866a0(void)
{
    int car;
    int offset;
    void **pTable;
    SceneNode *pNode;

    car = 0;
    if (g_unk0x00590c64 > 0) {
        offset = 0;
        do {
            pTable = g_unk0x00590d7c;
            do {
                if (*(SceneNode **)((BYTE *)*pTable + offset) != NULL) {
                    pNode = *(SceneNode **)((BYTE *)*pTable + offset);
                    if ((int)pNode->pParent == RallyData_FUN_00411060())
                        SceneNode_Destroy(pNode);
                }
                pTable++;
            } while (pTable < &g_unk0x00590d7c[4]);
            car++;
            offset += 0x1a0;
        } while (car < g_unk0x00590c64);
    }
}

// FUNCTION: CMR2 0x0048d850
void FUN_0048d850(BYTE *pCar, BYTE *pInfo)
{
    short v;

    v = *(short *)(g_unk0x00591750 + 2 + g_unk0x00591740[*pCar] * 0x6c);
    if (v > 0x3f4 && v < 0x40b) {
        FUN_00486c00(pCar, pInfo);
        return;
    }
    if (v < -0x3f4 && v > -0x40b)
        FUN_00486c00(pCar, pInfo);
}

// Two per-lane shorts (+8 and +0x12, `slot` 0..4) scaled by 256.
// FUNCTION: CMR2 0x00477c80
void FUN_00477c80(int lane, int *pA, int *pB, int slot)
{
    BYTE *p = g_unk0x0058d6d0[lane];

    if (pA != NULL)
        *pA = (*(short *)(p + 8 + slot * 2) * 0x10000) / 256;
    if (pB != NULL)
        *pB = (*(short *)(p + 0x12 + slot * 2) * 0x10000) / 256;
}

// Resets the car slot tuning values and reseeds the random generator.
// FUNCTION: CMR2 0x00480a60
void FUN_00480a60(void)
{
    srand(400);
    g_unk0x00590c44 = 0x8000;
    g_unk0x00590c48 = 0x8000;
    g_unk0x00590c4c = 0x8000;
    g_unk0x00590c50 = 0x3333;
    g_unk0x00590c60[0] = 0x16;
    g_unk0x00590c60[1] = 0x14;
    g_unk0x00590c60[2] = 0x15;
    g_unk0x00590c60[3] = 0x12;
    g_unk0x00590bfc = 1;
    g_unk0x00590bfd = 2;
}

int Sprite_FillRect(int unused, short *pRect, BYTE *pColour, int layer);

extern BYTE g_barTextColour[4];
// Panel, text and selection colours of the stage-data screen.
// GLOBAL: CMR2 0x0051c984
BYTE g_unk0x0051c984[4] = { 210, 202, 210, 128 };
// GLOBAL: CMR2 0x0051c994
BYTE g_unk0x0051c994[4] = { 143, 135, 143, 255 };

int FUN_004055e0(void);
int FUN_004055f0(void);

// Draws the stage-data panel of the pause screen: its background, the row
// separators and, for every item, the label and the highlight sprite.
// match 56%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// Identical instruction sequence; the original keeps the loop counter in ESI and
// pushes EBX/EDI inside the loop, our build spills one more register in the prologue,
// which shifts every stack slot by 4 (register-slot renumbering).
// FUNCTION: CMR2 0x004738f0
void FUN_004738f0(Menu *pMenu)
{
    MenuItem *pItem;
    short rect[4];
    short rect2[4];
    int i;
    int texture;
    BYTE *pColour;

    rect2[0] = (short)((int)(g_pGraphics->resX * 0x64) / 0x280);
    rect2[1] = (short)((int)(g_pGraphics->resY * 0xd1) / 0x1e0);
    texture = FUN_004055e0();
    rect2[2] = *(short *)(texture + 0x120);
    texture = FUN_004055e0();
    rect2[3] = *(short *)(texture + 0x122);
    rect[0] = (short)((int)(g_pGraphics->resX * 0x63) / 0x280);
    rect[1] = (short)((int)(g_pGraphics->resY * 0xa0) / 0x1e0);
    rect[2] = (short)((int)(g_pGraphics->resX * 0x11a) / 0x280);
    rect[3] = (short)((int)(g_pGraphics->resY * 0x26) / 0x1e0);
    Sprite_FillRect((int)g_pGraphics + 0x150, rect, g_unk0x0051c984, 2);
    pItem = pMenu->items;
    for (i = 0; i < pMenu->itemCount; i++, pItem++) {
        rect2[1] = (short)((int)(g_pGraphics->resY * i * 0x24) / 0x1e0
                           + (int)(g_pGraphics->resY * 0xd1) / 0x1e0);
        if (pMenu->cursor == i) {
            pColour = g_barTextColour;
            texture = FUN_004055e0();
        } else {
            pColour = g_unk0x0051c994;
            texture = FUN_004055f0();
        }
        Font_DrawText(0, CFrontend::GetTextString(pItem->id),
                      (int)(g_pGraphics->resX * 0x78) / 0x280,
                      (int)(g_pGraphics->resY * i * 0x24) / 0x1e0
                          + (int)(g_pGraphics->resY * 0xdd) / 0x1e0,
                      (int *)pColour, 0x11);
        Sprite_Queue((SpriteRect *)(texture + 0x11c), (SpriteRect *)rect2, (Texture *)texture,
                     2, 0, NULL, NULL, pColour, 8);
        if (i == 0 || pMenu->cursor == i) {
            rect[1] = (short)((int)(g_pGraphics->resY * i * 0x24) / 0x1e0
                              + (int)(g_pGraphics->resY * 0xc6) / 0x1e0);
            rect[3] = 1;
            Sprite_FillRect((int)g_pGraphics + 0x150, rect, pColour, 1);
        }
        rect[1] = (short)((int)(g_pGraphics->resY * i * 0x24) / 0x1e0
                          + (int)(g_pGraphics->resY * 0xea) / 0x1e0);
        rect[3] = 1;
        Sprite_FillRect((int)g_pGraphics + 0x150, rect, pColour, 1);
    }
}

// Draws the first `fraction` of a text (typing effect; spaces don't count)
// and the next character on its own.
// FUNCTION: CMR2 0x00474420
void FUN_00474420(char *text, int fraction, unsigned int font, int x, unsigned int y, int *pColour, unsigned int flags)
{
    int length = (int)strlen(text);
    int count = FixMul(length << 16, fraction) >> 16;
    int i;
    int width;
    int colour;

    for (i = 0; i < count; i++) {
        if (text[i] == ' ' && count < length)
            count++;
        CFrontend::m_stringDest[i] = text[i];
    }
    CFrontend::m_stringDest[count] = 0;
    Font_DrawText(font, CFrontend::m_stringDest, x, y, pColour, flags);
    if (count < length) {
        width = Font_GetTextWidth(font, (BYTE *)CFrontend::m_stringDest);
        CFrontend::m_stringDest[0] = text[count];
        CFrontend::m_stringDest[1] = 0;
        colour = *pColour;
        Font_DrawText(font, CFrontend::m_stringDest, width + x, y, &colour, flags);
    }
}

// Fills a rectangle whose width is scaled by `scale` (16.16).
// FUNCTION: CMR2 0x00475970
int FUN_00475970(int scale, int unused, short *pRect, BYTE *pColour, int layer)
{
    short rect[4];

    rect[0] = pRect[0];
    rect[1] = pRect[1];
    rect[2] = (short)FixMulShift32((int)pRect[2] << 16, scale);
    rect[3] = pRect[3];
    return Sprite_FillRect(unused, rect, pColour, layer);
}

// Wheel slip (field 0x870) above 0.15, as 0..1.
// match 89%: MSVC reuses the add's flags for the sign test (js) instead of
// re-issuing test/jl/jle, and folds the +(-0x2666) into a sub
// FUNCTION: CMR2 0x00465e40
int FUN_00465e40(int car, int wheel)
{
    int v, w;

    if (*(int *)((BYTE *)Car_Get(car) + 0x870 + wheel * 4) < 0)
        v = -*(int *)((BYTE *)Car_Get(car) + 0x870 + wheel * 4);
    else
        v = *(int *)((BYTE *)Car_Get(car) + 0x870 + wheel * 4);
    w = v - 0x2666;
    if (w < 0 || w <= 0)
        return 0;
    if (w >= 0x10000)
        return 0x10000;
    return w;
}

void Scene_GetLightColourBytes(DWORD *pColour);

// Brightness of the scene light colour: (r + g + b - 70) / 550, clamped to 0..1.
// FUNCTION: CMR2 0x004648f0
int FUN_004648f0(void)
{
    BYTE c[4];
    int v;

    Scene_GetLightColourBytes((DWORD *)c);
    v = FixDiv((c[2] + c[1] + c[0] - 0x46) << 16, 0x2260000);
    if (v < 0)
        return 0;
    if (v > 0x10000)
        v = 0x10000;
    return v;
}

// GLOBAL: CMR2 0x00547fa0
FixVector g_unk0x00547fa0 = { 0, 0, 0 };
// GLOBAL: CMR2 0x00547fe0
FixVector g_unk0x00547fe0 = { 0, 0, 0 };
// GLOBAL: CMR2 0x00547fec
SceneNode *g_unk0x00547fec = NULL;
// GLOBAL: CMR2 0x00547ff0
SceneNode *g_unk0x00547ff0 = NULL;
// GLOBAL: CMR2 0x00547ff4
int g_unk0x00547ff4 = 0;

void FUN_004b6ef0(int value);

// Places the two rear view nodes of a car from its brightness level.
// match 50%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00464960
void FUN_00464960(unsigned int param_1)
{
    unsigned int car;
    Car *pCar;
    int value;
    int level;
    FixVector pos;
    FixVector offset;

    car = param_1;
    pCar = Car_Get(car);
    level = FUN_004648f0();
    if (level == 0x10000)
        *(int *)((BYTE *)pCar + 0xb58) = 0;
    else
        *(int *)((BYTE *)pCar + 0xb58) = 1;
    value = FixMul(level, 0x4c0000);
    FUN_00477c80(car, (int *)&param_1, (int *)&param_1, 4);
    if (param_1 == 0)
        value = 0x3e80000;
    FUN_004b6ef0(param_1 != 0);
    param_1 = 0x10000 - param_1;
    if ((int)param_1 < 0x10001) {
        if ((int)param_1 < 0)
            param_1 = 0;
    } else {
        param_1 = 0x10000;
    }
    param_1 = FixMul((int)param_1, 0x190000);
    level = value + param_1;
    if (level != g_unk0x00547ff4) {
        Scene_SetLightAttenuation(g_unk0x00547fec, level);
        Scene_SetLightAttenuation(g_unk0x00547ff0, level);
        g_unk0x00547ff4 = level;
    }
    FixMatrix_GetPosition(&pos, (FixMatrix *)((BYTE *)pCar->pNode0x71c + 0x98));
    FixMatrix_RotateVector(&offset, &g_unk0x00547fe0, (FixMatrix *)((BYTE *)pCar->pNode0x71c + 0x98));
    offset.x += pos.x;
    offset.y += pos.y;
    offset.z += pos.z;
    SceneNode_SetPosition(g_unk0x00547fec, &offset);
    FixMatrix_RotateVector(&offset, &g_unk0x00547fa0, (FixMatrix *)((BYTE *)pCar->pNode0x71c + 0x98));
    offset.x += pos.x;
    offset.y += pos.y;
    offset.z += pos.z;
    SceneNode_SetPosition(g_unk0x00547ff0, &offset);
}

struct Unk0x00590d74;
extern Unk0x00590d74 *g_unk0x00590d74;

// Puts the car's (up to four) attached nodes back to their creation transform
// and forgets them.
// FUNCTION: CMR2 0x00480b40
void FUN_00480b40(BYTE *pCar)
{
    void **pTable;
    SceneNode *pNode;
    int offset;

    srand(400);
    g_unk0x00590d74 = (Unk0x00590d74 *)pCar;
    pTable = &g_unk0x00590d7c[3];
    offset = (char)pCar[0xb1a] * 0x1a0;
    do {
        pNode = *(SceneNode **)((BYTE *)*pTable + offset);
        if (pNode != NULL) {
            pNode->current = pNode->local;
            *(SceneNode **)((BYTE *)*pTable + offset) = NULL;
        }
        pTable--;
    } while (pTable >= g_unk0x00590d7c);
}

// Per-car values eased towards their targets (0x591710) by a step.
// GLOBAL: CMR2 0x00591690
int g_unk0x00591690[4];
// GLOBAL: CMR2 0x00591710
int g_unk0x00591710[4];

// Steps one stage object's current vector toward its target by `step`,
// marking it arrived (and snapping to the target) once it is closer than that.
// FUNCTION: CMR2 0x0048db00
void FUN_0048db00(BYTE *p, int step)
{
    BYTE index;
    FixVector delta;

    index = *p;
    if (g_unk0x005916f0[index] == 0) {
        delta.x = g_unk0x005916a0[index].x - g_unk0x00591898[index].x;
        delta.y = g_unk0x005916a0[index].y - g_unk0x00591898[index].y;
        delta.z = g_unk0x005916a0[index].z - g_unk0x00591898[index].z;
        if ((int)FixVec_Length(&delta) < step) {
            g_unk0x005916f0[index] = 1;
        } else {
            FixVec_Normalize(&delta, &delta);
            FixVecScale(&delta, &delta, step);
            g_unk0x00591898[index].x += delta.x;
            g_unk0x00591898[index].y += delta.y;
            g_unk0x00591898[index].z += delta.z;
        }
        if (g_unk0x005916f0[index] == 0)
            return;
    }
    g_unk0x00591898[index] = g_unk0x005916a0[index];
}

// match 35%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0048dc30
void FUN_0048dc30(BYTE *pCar, int step)
{
    unsigned int car;
    int cur;
    int d;

    car = *pCar;
    cur = g_unk0x00591690[car];
    d = g_unk0x00591710[car] - cur;
    if (FIX_ABS(d) < step) {
        g_unk0x00591690[car] = g_unk0x00591710[car];
        return;
    }
    if (d > 0) {
        g_unk0x00591690[car] = cur + step;
        return;
    }
    g_unk0x00591690[car] = cur - step;
}

// Five-bit field of the current round entry (bits 0..4, or 5..9 with pHigh).
// match 40%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004735a0
unsigned int FUN_004735a0(unsigned int *pHigh)
{
    unsigned int *pState;
    unsigned int *pEntry;
    unsigned int state;

    pState = RallyData_GetChampionshipState();
    state = *pState;
    pEntry = pHigh;
    switch ((state >> 3) & 7) {
    case 1:
        pEntry = pState + ((state >> 12) & 0xf) * 3 + 0x16;
        break;
    case 2:
        pEntry = pState + ((state >> 12) & 0xf) * 3 + 10;
        break;
    case 3:
        pEntry = pState + ((state >> 12) & 0xf) * 3 + 4;
        break;
    case 4:
        pEntry = pState + 1;
        break;
    }
    state = *pEntry;
    if (pHigh != NULL)
        state >>= 5;
    return state & 0x1f;
}

extern Car *g_collisionCar;
extern FixVector g_collisionTarget;
extern FixVector g_collisionLineStart;

// 1 when no corner of the collision car lies strictly between the heights of
// the line start and the target.
// match 17%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0048f400
int FUN_0048f400(void)
{
    int *pY;
    int i;

    if (g_collisionTarget.y > g_collisionLineStart.y) {
        i = 0;
        pY = &g_collisionCar->corners[0].y;
        while (*pY >= g_collisionTarget.y || *pY <= g_collisionLineStart.y) {
            i++;
            pY += 3;
            if (i >= 8)
                return 1;
        }
    } else {
        i = 0;
        pY = &g_collisionCar->corners[0].y;
        while (*pY <= g_collisionTarget.y || *pY >= g_collisionLineStart.y) {
            i++;
            pY += 3;
            if (i >= 8)
                return 1;
        }
    }
    return 0;
}

void FUN_004ae410(BYTE a, BYTE b, int c, int d);

// match 68%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00486b20
void FUN_00486b20(BYTE *pCar, BYTE *pInfo)
{
    int kind;

    if (*(int *)(pCar + 4) == 1 && *(int *)(pInfo + 4) == 2)
        FUN_00486be0(pInfo, (int)pCar);
    else
        FUN_004ae410(pCar[2], pCar[1], 1, 1);
    kind = *(int *)(pInfo + 4);
    if (kind != 2 && kind != 1 && kind != 10) {
        g_unk0x00590db0[*pCar] = 0;
        return;
    }
    g_unk0x00590db0[*pCar] = 0x10000;
}

extern void **g_unk0x00590c6c;

// Resets record `index` (0x3c bytes) of list `list`: its three vectors to the
// origin and its final int to `value`.
// match 61%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00486630
void FUN_00486630(int list, int index, int value)
{
    int *p;

    p = (int *)((BYTE *)g_unk0x00590c6c[list] + index * 0x3c);
    p[0] = 0;
    p[1] = 0;
    p[2] = 0;
    p[9] = 0;
    p[10] = 0;
    p[11] = 0;
    p[3] = p[0];
    p[12] = 0;
    p[13] = 0;
    p[4] = p[1];
    p[5] = p[2];
    p[14] = value;
    p[6] = p[0];
    p[7] = p[1];
    p[8] = p[2];
}

// Sun visibility (0..100) from the lens flare sample.
// GLOBAL: CMR2 0x00543ea0
short g_sunVisibility;

BYTE Flare_SampleVisibility(short *pRect, BYTE *pColour, BYTE tolerance);
void FUN_00492bb0(int *pOut);

void Scene_GetAmbientColour(DWORD *pColour);
void Scene_SetAmbient(BYTE *pColour, int boost);
unsigned int RallyDataCountryIndex(void);
int FUN_00407270(void);
void FUN_0047e490(BYTE *pColour);

extern void *g_unk0x00543eb8;
// GLOBAL: CMR2 0x00547acc
BYTE g_unk0x00547acc;

int *RallyData_FUN_004075b0(int index);
// GLOBAL: CMR2 0x0051b114
int g_unk0x0051b114[9] = { 0, 0, 0, 0x6666, 0x8000, 0x9999, 0x6666, 0x8000, 0x9999 };

// Speed limits of a stage object: fixed in some stages, else scaled by the
// difficulty and the object's type factor.
// FUNCTION: CMR2 0x00461b30
void FUN_00461b30(BYTE *pObject, int type)
{
    unsigned int speed;

    *(int *)(pObject + 0x14) = 0;
    *(int *)(pObject + 0x18) = 0;
    speed = (CGameInfo::FUN_00405ca0() + 2) * 0x320000;
    if (*RallyData_FUN_004075b0(RallyDataStageIndex()) != 0) {
        *(int *)(pObject + 0x14) = 0xf0000;
        *(int *)(pObject + 0x18) = 0x500000;
        return;
    }
    if (g_unk0x0051b114[type] != 0)
        *(int *)(pObject + 0x18) = FixDiv(speed, g_unk0x0051b114[type]);
}

int *FUN_00407520(int index);
void FUN_00461b30(BYTE *pObject, int type);

// Time of day steps (mm*100+ss, 500 = 05:00) that bound the twelve stage object
// record columns.
// GLOBAL: CMR2 0x0051b034
WORD g_stageObjectTimeSteps[12] = {
    500, 600, 800, 1000, 1200, 1500, 1700, 1800, 1900, 2000, 2200, 2400
};

// Per stage setting, the two rows of the loaded .hor stage object records to
// blend: 0 = plain, 1 = CLO, 2 = STO, 3 = BLI.
// GLOBAL: CMR2 0x0051b0dc
BYTE g_stageObjectWeatherRows[9][2] = {
    { 0, 0 }, { 0, 0 }, { 0, 1 }, { 1, 2 }, { 1, 2 },
    { 1, 2 }, { 1, 3 }, { 1, 3 }, { 1, 3 }
};

// Per stage setting, the 16.16 factor that blends those two rows.
// GLOBAL: CMR2 0x0051b0f0
int g_stageObjectWeatherFactors[9] = {
    0, 0, 0x10000, 0, 0x8000, 0x10000, 0, 0x8000, 0x10000
};

// Stage object record of the primary stage setting (0x4e bytes).
// GLOBAL: CMR2 0x00543d00
BYTE g_stageObjectPrimary[0x4e];
// Stage object record of the secondary stage setting.
// GLOBAL: CMR2 0x00543e38
BYTE g_stageObjectSecondary[0x4e];

void FUN_00461710(BYTE *out, BYTE *from, BYTE *to, int t);

// Blends the stage object record for one time of day into the primary (slot 0)
// or secondary (slot 1) global record: the time picks the two adjacent columns
// of the loaded .hor table, the stage setting pair picks the record rows and
// their fixed factor, and the two column results are interpolated by how far
// the time lies into its band. NULL when a record is missing or invalid.
// FUNCTION: CMR2 0x00461830
BYTE *FUN_00461830(unsigned short timeOfDay, int slot, BYTE **records)
{
    BYTE buffer0[0x50];
    BYTE buffer1[0x50];
    int *pWeatherPair;
    int index;
    int lower;
    int i;
    int prev;
    int prevSeconds;
    int length;
    int position;
    int factor;
    int rowFrom;
    int rowTo;
    int rowFactor;
    BYTE *pFrom0;
    BYTE *pFrom1;
    BYTE *pTo0;
    BYTE *pTo1;
    BYTE *dest;

    pWeatherPair = FUN_00407520(RallyDataStageIndex());
    if (slot == 0)
        index = pWeatherPair[0];
    else
        index = pWeatherPair[1];

    lower = -1;
    for (i = 0; i < 12; i++) {
        if (g_stageObjectTimeSteps[i] >= timeOfDay) {
            lower = i;
            break;
        }
    }
    if (lower == -1)
        return NULL;

    prev = lower - 1;
    if (prev < 0) {
        prev += 12;
        prevSeconds = 0;
    } else {
        prevSeconds = (g_stageObjectTimeSteps[prev] / 100 * 60 + g_stageObjectTimeSteps[prev] % 100) << 16;
    }
    length = ((g_stageObjectTimeSteps[lower] / 100 * 60 + g_stageObjectTimeSteps[lower] % 100) << 16) - prevSeconds;

    if (timeOfDay % 100 >= 60)
        return NULL;
    position = ((timeOfDay / 100 * 60 + timeOfDay % 100) << 16) - prevSeconds;

    factor = FixDiv(position, length);
    if (factor > 0x10000)
        factor = 0x10000;
    else if (factor < 0)
        factor = 0;

    rowFrom = g_stageObjectWeatherRows[index][0];
    rowTo = g_stageObjectWeatherRows[index][1];
    rowFactor = g_stageObjectWeatherFactors[index];
    pFrom0 = records[rowFrom * 12 + prev];
    pFrom1 = records[rowFrom * 12 + lower];
    pTo0 = records[rowTo * 12 + prev];
    pTo1 = records[rowTo * 12 + lower];
    if (pFrom0 == NULL || pTo0 == NULL || pFrom1 == NULL || pTo1 == NULL)
        return NULL;

    dest = g_stageObjectPrimary;
    if (slot != 0)
        dest = g_stageObjectSecondary;

    FUN_00461710(buffer0, pFrom0, pTo0, rowFactor);
    FUN_00461710(buffer1, pFrom1, pTo1, rowFactor);
    FUN_00461710(dest, buffer0, buffer1, factor);
    return dest;
}

// Blends both stage object records of the stage's setting pair into the two out
// pointers: the primary from the first setting's time of day, the secondary
// from the second one.
// FUNCTION: CMR2 0x00461a30
void FUN_00461a30(int timePrimary, int timeSecondary, BYTE **records, BYTE **pPrimary, BYTE **pSecondary)
{
    BYTE *pObjectPrimary;
    BYTE *pObjectSecondary;

    pObjectPrimary = FUN_00461830(timePrimary, 0, records);
    pObjectSecondary = FUN_00461830(timeSecondary, 1, records);
    *pPrimary = pObjectPrimary;
    *pSecondary = pObjectSecondary;
}

// Sets the speed limits of the two stage objects of a pair from the stage's
// setting pair; a stopped one takes over the other's limit.
// FUNCTION: CMR2 0x00461a70
void FUN_00461a70(BYTE *pA, BYTE *pB)
{
    int *pPair;
    int a;
    int b;
    int limit;

    if (pA != NULL && pB != NULL) {
        pPair = FUN_00407520(RallyDataStageIndex());
        a = pPair[0];
        b = pPair[1];
        if (a == 1) {
            if (b == 0 || b == 1)
                pA[0x2f] = 200;
            else
                pA[0x2f] = 0x32;
        }
        if (b == 1) {
            if (a == 0 || a == 1)
                pB[0x2f] = 200;
            else
                pB[0x2f] = 0x32;
        }
        FUN_00461b30(pA, a);
        FUN_00461b30(pB, b);
        if (*(int *)(pA + 0x14) == 0 && *(int *)(pA + 0x18) == 0 &&
            (*(int *)(pB + 0x14) != 0 || *(int *)(pB + 0x18) != 0)) {
            limit = *(int *)(pB + 0x18);
            *(int *)(pA + 0x18) = limit;
            *(int *)(pA + 0x14) = limit;
            return;
        }
        if (*(int *)(pB + 0x14) == 0 && *(int *)(pB + 0x18) == 0 &&
            (*(int *)(pA + 0x14) != 0 || *(int *)(pA + 0x18) != 0)) {
            limit = *(int *)(pA + 0x18);
            *(int *)(pB + 0x18) = limit;
            *(int *)(pB + 0x14) = limit;
        }
    }
}

void FUN_00491790(short index, short *pA, short *pB, short *pC, short *pD, unsigned short *pFlags);
void FUN_004917f0(int *pOut, unsigned short *pIndices, int unused);
int Graphics_GetTriangleHeight(unsigned short *pHeightIndices, FixVector *pVertices, FixVector *pPosition);
void Scene_GetLightColour(DWORD *pColour, int level);
void Scene_GetAmbientColour(DWORD *pColour);
void FUN_00492e60(int *pRGB);
void FUN_004984b0(int car, int level);

// Averages the ground lighting over a car's four wheel contact points and
// stores the resulting colour and light level.
// match 61%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00462aa0
void FUN_00462aa0(unsigned int param_1, int param_2)
{
    Car *pCar;
    short *pIndex;
    int vertex;
    int i;
    int sum;
    int count;
    unsigned short idx[3];
    unsigned short d;
    unsigned short flags;
    int lighting[9];
    int h;
    BYTE light[4];
    DWORD ambient[1];
    int r, g, b;
    int avg;
    int total;
    int v;

    pCar = Car_Get(param_1);
    i = 0;
    sum = 0;
    count = 0;
    pIndex = (short *)((BYTE *)pCar + 0xa9e);
    vertex = (int)pCar + 0x270;
    do {
        if (*pIndex >= 0) {
            FUN_00491790(*pIndex, (short *)&idx[0], (short *)&idx[1], (short *)&idx[2],
                         (short *)&d, &flags);
            FUN_004917f0(lighting, idx, (int)&d);
            h = Graphics_GetTriangleHeight(idx, (FixVector *)lighting, (FixVector *)vertex);
            if (h != -0x3e70000) {
                sum = sum + h;
                count = count + 1;
                Scene_GetLightColour((DWORD *)light, h);
                ((BYTE *)g_unk0x00543f28)[(i + pCar->field_0xb1a * 4) * 4] = light[0];
                ((BYTE *)g_unk0x00543f28)[(i + pCar->field_0xb1a * 4) * 4 + 1] = light[1];
                ((BYTE *)g_unk0x00543f28)[(i + pCar->field_0xb1a * 4) * 4 + 2] = light[2];
            }
        }
        i = i + 1;
        pIndex = pIndex + 1;
        vertex = vertex + 0xc;
    } while (i < 4);
    if (count > 0) {
        avg = FixDiv(sum, count << 16);
        if (avg > 0x10000)
            avg = 0x10000;
        else if (avg < 0)
            avg = 0;
        Scene_GetLightColour((DWORD *)light, avg);
        r = (*(int *)(light + 0) & 0xff) << 16;
        g = (*(int *)(light + 1) & 0xff) << 16;
        b = (*(int *)(light + 2) & 0xff) << 16;
        Scene_GetAmbientColour(ambient);
        r = r - (*(int *)((BYTE *)ambient + 0) & 0xff) * 0x10000;
        g = g - (*(int *)((BYTE *)ambient + 1) & 0xff) * 0x10000;
        b = b - (*(int *)((BYTE *)ambient + 2) & 0xff) * 0x10000;
        if ((FUN_00422fb0(param_2) & 0xff) == param_1)
            FUN_00492e60(&r);
        *(int *)((BYTE *)pCar + 0xa70) = avg;
        total = (r < 0 ? -r : r) + (g < 0 ? -g : g) + (b < 0 ? -b : b);
        v = FixMul(total, 0x55);
        if (v > 0x10000)
            v = 0x10000;
        FUN_004984b0(param_1, v);
    }
}

// Interpolates the two animated values of every 0x2c-byte record by t (16.16).
// FUNCTION: CMR2 0x00461bb0
void FUN_00461bb0(int t)
{
    int i;
    BYTE *p;

    for (i = 0; i < g_unk0x00547acc; i++) {
        p = (BYTE *)g_unk0x00543eb8 + i * 0x2c;
        *(int *)(p + 0x1c) = FixMul(t, *(int *)(p + 0x10) - *(int *)(p + 0x18)) + *(int *)(p + 0x18);
        *(int *)(p + 0x24) = *(int *)(p + 0x20) + FixMul(t, *(int *)(p + 0x14) - *(int *)(p + 0x20));
    }
}
void FUN_004920d0(DWORD *pColour, DWORD *pReference);
void FUN_00492220(DWORD *pColour, DWORD *pReference);
void FUN_004923d0(DWORD *pColour);
void FUN_00492470(DWORD *pColour);
void FUN_00492520(DWORD *pColour);
void FUN_004926f0(int unused1, int unused2, int sunAngle);
void FUN_00492bd0(int view);
void FUN_00492f10(void);
void FUN_00492fe0(DWORD *pColour, int start, int end);
void FUN_00462cb0(BYTE *pColour);
void Stage_SetHeightColours(BYTE *pLow, BYTE *pHigh, BYTE *pReference, int referenceBlend);
void StageLights_UpdateDirection(void);
extern int g_unk0x00543d88;
extern int g_unk0x00543d8c;

// Blended ramp value and the two step sizes derived from it.
// GLOBAL: CMR2 0x00543d58
int g_unk0x00543d58;
// GLOBAL: CMR2 0x00543d5c
int g_unk0x00543d5c;
// Stage object colour pushed straight to the stage mesh.
// GLOBAL: CMR2 0x00543eb4
BYTE g_unk0x00543eb4[4];
// Set while the object ramps still have to be re-applied.
// GLOBAL: CMR2 0x00543ef8
int g_unk0x00543ef8;

// Rebuilds every lighting colour of the stage from the current weather blend
// factor and pushes them to the stage meshes: height ramp (low/high/reference),
// ground, sky, light and ambient colours plus the sun light vector. Called once
// per view index by the stage renderer.
// The table g_stageLighting holds two complete parameter sets - primary in
// [0x00..0x2c] and secondary (the other weather) in [0x2d..0x59], every
// secondary entry exactly 0x2d dwords above its primary counterpart - followed
// by [0x5a] the blend factor, [0x5b] the blended intensity, [0x5c] the
// {current, target} weather words and [0x5d] the weather-changed flag.
// match 30%: the logic follows the asm, but MSVC6 gives the twelve colour
// buffers (and three of the scratch vectors) different stack slots than the
// original - the slot assignment depends on the whole local set and could not
// be reproduced - so every memory operand is displaced.
// match 30%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00461c30
void FUN_00461c30(int index)
{
    BYTE lowColour[4] = {0, 0, 0, 0xff};
    BYTE highColour[4] = {0, 0, 0, 0xff};
    BYTE rampColour[4] = {0, 0, 0, 0xff};
    BYTE referenceColour[4] = {0, 0, 0, 0xff};
    BYTE groundColour[4] = {0, 0, 0, 0xff};
    BYTE groundRefColour[4] = {0, 0, 0, 0xff};
    BYTE objectColour[4] = {0, 0, 0, 0xff};
    BYTE ambientColour[4] = {0, 0, 0, 0xff};
    BYTE objectRefColour[4] = {0xff, 0xff, 0xff, 0xff};
    BYTE lightColour[4] = {0xff, 0xff, 0xff, 0xff};
    BYTE skyColour[4] = {0xff, 0xff, 0xff, 0xff};
    BYTE heightColour[4] = {0xff, 0xff, 0xff, 0xff};
    FixVector v[3];
    FixVector colour;
    FixVector delta;
    FixVector ambientMix;
    int blend;
    int value;
    int flag;
    int *pObject;
    int *pView;

    pObject = (int *)((BYTE *)g_unk0x00543eb8 + index * 0x2c);
    pView = (int *)((BYTE *)g_unk0x00547ac8 + index * 0x178);
    if (pObject[7] != g_stageLighting[0x5a] || g_unk0x00543d88 != g_unk0x00543d8c ||
        *((WORD *)&g_stageLighting[0x5c] + 1) != *(WORD *)&g_stageLighting[0x5c] || pObject[10] != 0) {
        g_stageLighting[0x5a] = pObject[7];
        pObject[10] = 0;
        if (g_stageLighting[0x5a] > 0x10000)
            g_stageLighting[0x5a] = 0x10000;
        flag = 1;
        if (*(WORD *)&g_stageLighting[0x5c] == 0xffff || pView[0x15] <= 0xcccc || pView[0] != 1)
            flag = 0;
        g_stageLighting[0x5b] = FixMul(g_stageLighting[0x59] - g_stageLighting[0x2c], g_stageLighting[0x5a]) +
                                g_stageLighting[0x2c];

        v[2].x = g_stageLighting[0x2d] - g_stageLighting[0x00];
        v[2].y = g_stageLighting[0x2e] - g_stageLighting[0x01];
        v[2].z = g_stageLighting[0x2f] - g_stageLighting[0x02];
        FixVecScale(&v[2], &v[2], g_stageLighting[0x5a]);
        v[2].x += g_stageLighting[0x00];
        v[2].y += g_stageLighting[0x01];
        v[2].z += g_stageLighting[0x02];
        lowColour[0] = (BYTE)(v[2].x >> 16);
        lowColour[1] = (BYTE)(v[2].y >> 16);
        lowColour[2] = (BYTE)(v[2].z >> 16);

        v[0].x = g_stageLighting[0x30] - g_stageLighting[0x03];
        v[0].y = g_stageLighting[0x31] - g_stageLighting[0x04];
        v[0].z = g_stageLighting[0x32] - g_stageLighting[0x05];
        FixVecScale(&v[0], &v[0], g_stageLighting[0x5a]);
        v[0].x += g_stageLighting[0x03];
        v[0].y += g_stageLighting[0x04];
        v[0].z += g_stageLighting[0x05];
        colour.x = v[0].x + v[2].x;
        colour.y = v[0].y + v[2].y;
        colour.z = v[0].z + v[2].z;
        highColour[0] = (BYTE)(colour.x >> 16);
        highColour[1] = (BYTE)(colour.y >> 16);
        highColour[2] = (BYTE)(colour.z >> 16);

        v[1].x = g_stageLighting[0x33] - g_stageLighting[0x06];
        v[1].y = g_stageLighting[0x34] - g_stageLighting[0x07];
        v[1].z = g_stageLighting[0x35] - g_stageLighting[0x08];
        FixVecScale(&v[1], &v[1], g_stageLighting[0x5a]);
        v[1].x += g_stageLighting[0x06];
        v[1].y += g_stageLighting[0x07];
        v[1].z += g_stageLighting[0x08];
        referenceColour[0] = (BYTE)(v[1].x >> 16);
        referenceColour[1] = (BYTE)(v[1].y >> 16);
        referenceColour[2] = (BYTE)(v[1].z >> 16);

        colour.x = g_stageLighting[0x36] - g_stageLighting[0x09];
        colour.y = g_stageLighting[0x37] - g_stageLighting[0x0a];
        colour.z = g_stageLighting[0x38] - g_stageLighting[0x0b];
        FixVecScale(&colour, &colour, g_stageLighting[0x5a]);
        colour.x += g_stageLighting[0x09];
        colour.y += g_stageLighting[0x0a];
        colour.z += g_stageLighting[0x0b];
        g_unk0x00543eb4[3] = 0xff;
        g_unk0x00543eb4[0] = (BYTE)(colour.x >> 16);
        g_unk0x00543eb4[1] = (BYTE)(colour.y >> 16);
        g_unk0x00543eb4[2] = (BYTE)(colour.z >> 16);

        value = FixMul(g_stageLighting[0x58] - g_stageLighting[0x2b], g_stageLighting[0x5a]) +
                g_stageLighting[0x2b];
        g_unk0x00543d58 = FixMul(value, 0x66);
        g_unk0x00543d5c = FixMul(value, 0x88);
        blend = FixMul(g_stageLighting[0x54] - g_stageLighting[0x27], g_stageLighting[0x5a]) +
                g_stageLighting[0x27];

        colour.x = g_stageLighting[0x3c] - g_stageLighting[0x0f];
        colour.y = g_stageLighting[0x3d] - g_stageLighting[0x10];
        colour.z = g_stageLighting[0x3e] - g_stageLighting[0x11];
        FixVecScale(&colour, &colour, g_stageLighting[0x5a]);
        colour.x += g_stageLighting[0x0f];
        colour.y += g_stageLighting[0x10];
        colour.z += g_stageLighting[0x11];
        objectColour[0] = (BYTE)(colour.x >> 16);
        objectColour[1] = (BYTE)(colour.y >> 16);
        objectColour[2] = (BYTE)(colour.z >> 16);
        objectColour[3] = (BYTE)((g_stageLighting[0x29] +
                                  FixMul(g_stageLighting[0x56] - g_stageLighting[0x29], g_stageLighting[0x5a])) >> 16);

        colour.x = g_stageLighting[0x3f] - g_stageLighting[0x12];
        colour.y = g_stageLighting[0x40] - g_stageLighting[0x13];
        colour.z = g_stageLighting[0x41] - g_stageLighting[0x14];
        FixVecScale(&colour, &colour, g_stageLighting[0x5a]);
        colour.x += g_stageLighting[0x12];
        colour.y += g_stageLighting[0x13];
        colour.z += g_stageLighting[0x14];
        FixVecScale(&colour, &colour, FixMul(g_stageLighting[0x5b], 0x3333) + 0xcccc);
        ambientMix.x = colour.x;
        ambientMix.y = colour.y;
        ambientMix.z = colour.z;
        ambientColour[0] = (BYTE)(ambientMix.x >> 16);
        ambientColour[1] = (BYTE)(ambientMix.y >> 16);
        ambientColour[2] = (BYTE)(ambientMix.z >> 16);

        delta.x = g_stageLighting[0x39] - g_stageLighting[0x0c];
        delta.y = g_stageLighting[0x3a] - g_stageLighting[0x0d];
        delta.z = g_stageLighting[0x3b] - g_stageLighting[0x0e];
        FixVecScale(&delta, &delta, g_stageLighting[0x5a]);
        delta.x += g_stageLighting[0x0c];
        delta.y += g_stageLighting[0x0d];
        delta.z += g_stageLighting[0x0e];
        delta.x -= ambientMix.x;
        delta.y -= ambientMix.y;
        delta.z -= ambientMix.z;
        FixVecScale(&delta, &delta, g_stageLighting[0x5b]);
        delta.x += ambientMix.x;
        delta.y += ambientMix.y;
        delta.z += ambientMix.z;
        groundColour[0] = (BYTE)(delta.x >> 16);
        groundColour[1] = (BYTE)(delta.y >> 16);
        groundColour[2] = (BYTE)(delta.z >> 16);
        value = FixMul(g_stageLighting[0x55] - g_stageLighting[0x28], g_stageLighting[0x5a]) +
                g_stageLighting[0x28];

        colour.x = v[1].x - delta.x;
        colour.y = v[1].y - delta.y;
        colour.z = v[1].z - delta.z;
        FixVecScale(&colour, &colour, FixMul(value, blend));
        colour.x += delta.x;
        colour.y += delta.y;
        colour.z += delta.z;
        colour.x -= ambientMix.x;
        colour.y -= ambientMix.y;
        colour.z -= ambientMix.z;
        FixVecScale(&colour, &colour, g_stageLighting[0x5b]);
        colour.x += ambientMix.x;
        colour.y += ambientMix.y;
        colour.z += ambientMix.z;
        groundRefColour[0] = (BYTE)(colour.x >> 16);
        groundRefColour[1] = (BYTE)(colour.y >> 16);
        groundRefColour[2] = (BYTE)(colour.z >> 16);

        colour.x = g_stageLighting[0x4e] - g_stageLighting[0x21];
        colour.y = g_stageLighting[0x4f] - g_stageLighting[0x22];
        colour.z = g_stageLighting[0x50] - g_stageLighting[0x23];
        FixVecScale(&colour, &colour, g_stageLighting[0x5a]);
        colour.x += g_stageLighting[0x21];
        colour.y += g_stageLighting[0x22];
        colour.z += g_stageLighting[0x23];
        skyColour[0] = (BYTE)(colour.x >> 16);
        skyColour[1] = (BYTE)(colour.y >> 16);
        skyColour[2] = (BYTE)(colour.z >> 16);
        skyColour[3] = (BYTE)((g_stageLighting[0x24] +
                               FixMul(g_stageLighting[0x51] - g_stageLighting[0x24], g_stageLighting[0x5a])) >> 16);

        if (g_stageLighting[0x5d] != 0) {
            colour.x = g_stageLighting[0x4b] - g_stageLighting[0x1e];
            colour.y = g_stageLighting[0x4c] - g_stageLighting[0x1f];
            colour.z = g_stageLighting[0x4d] - g_stageLighting[0x20];
            FixVecScale(&colour, &colour, g_stageLighting[0x5a]);
            colour.x += g_stageLighting[0x1e];
            colour.y += g_stageLighting[0x1f];
            colour.z += g_stageLighting[0x20];
            heightColour[0] = (BYTE)(colour.x >> 16);
            heightColour[1] = (BYTE)(colour.y >> 16);
            heightColour[2] = (BYTE)(colour.z >> 16);
            heightColour[3] = 0xff;
            FUN_00492fe0((DWORD *)heightColour,
                         g_stageLighting[0x25] +
                             FixMul(g_stageLighting[0x52] - g_stageLighting[0x25], g_stageLighting[0x5a]),
                         g_stageLighting[0x26] +
                             FixMul(g_stageLighting[0x53] - g_stageLighting[0x26], g_stageLighting[0x5a]));
        }
        if (flag) {
            if (groundColour[0] <= 0xeb)
                groundColour[0] = (BYTE)(groundColour[0] + 0x14);
            else
                groundColour[0] = 0xff;
            if (groundColour[1] <= 0xeb)
                groundColour[1] = (BYTE)(groundColour[1] + 0x14);
            else
                groundColour[1] = 0xff;
            if (groundColour[2] <= 0xeb)
                groundColour[2] = (BYTE)(groundColour[2] + 0x14);
            else
                groundColour[2] = 0xff;
        }

        colour.x = g_stageLighting[0x45] - g_stageLighting[0x18];
        colour.y = g_stageLighting[0x46] - g_stageLighting[0x19];
        colour.z = g_stageLighting[0x47] - g_stageLighting[0x1a];
        FixVecScale(&colour, &colour, g_stageLighting[0x5a]);
        colour.x += g_stageLighting[0x18];
        colour.y += g_stageLighting[0x19];
        colour.z += g_stageLighting[0x1a];
        rampColour[0] = (BYTE)(colour.x >> 16);
        rampColour[1] = (BYTE)(colour.y >> 16);
        rampColour[2] = (BYTE)(colour.z >> 16);
        rampColour[3] = (BYTE)((g_stageLighting[0x2a] +
                                FixMul(g_stageLighting[0x57] - g_stageLighting[0x2a], g_stageLighting[0x5a])) >> 16);

        colour.x = g_stageLighting[0x2b] - g_stageLighting[0x15];
        colour.y = g_stageLighting[0x2c] - g_stageLighting[0x16];
        colour.z = g_stageLighting[0x2d] - g_stageLighting[0x17];
        FixVecScale(&colour, &colour, g_stageLighting[0x5a]);
        colour.x += g_stageLighting[0x15];
        colour.y += g_stageLighting[0x16];
        colour.z += g_stageLighting[0x17];
        colour.x -= ambientMix.x;
        colour.y -= ambientMix.y;
        colour.z -= ambientMix.z;
        FixVecScale(&colour, &colour, g_stageLighting[0x5b]);
        colour.x += ambientMix.x;
        colour.y += ambientMix.y;
        colour.z += ambientMix.z;

        if (flag) {
            objectRefColour[3] = (objectColour[3] <= 0xc8) ? (BYTE)(objectColour[3] + 0x32) : 0xfa;
            groundColour[0] = 0xff;
            groundColour[1] = 0xff;
            groundColour[2] = 0xff;
            groundColour[3] = 0xff;
            groundRefColour[0] = 0xff;
            groundRefColour[1] = 0xff;
            groundRefColour[2] = 0xff;
            groundRefColour[3] = 0xff;
            colour.x = 0xff0000;
            colour.y = 0xff0000;
            colour.z = 0xff0000;
            blend = 0x4ccc;
        } else {
            *(int *)objectRefColour = *(int *)objectColour;
        }

        FUN_00492220((DWORD *)objectColour, (DWORD *)objectRefColour);
        Stage_SetHeightColours(lowColour, highColour, referenceColour, blend);
        FUN_004920d0((DWORD *)groundColour, (DWORD *)groundRefColour);
        FUN_00492470((DWORD *)rampColour);
        FUN_00492520((DWORD *)skyColour);
        lightColour[3] = (BYTE)-(int)flag;
        FUN_004923d0((DWORD *)lightColour);
        FUN_00462cb0(ambientColour);
        FUN_00492e30(&colour);
        FUN_004925c0(g_stageLighting[0x1b] + FixMul(g_stageLighting[0x48] - g_stageLighting[0x1b], g_stageLighting[0x5a]),
                     g_stageLighting[0x1c], g_stageLighting[0x1d]);
        StageLights_UpdateDirection();
    }
    FUN_004926f0(0, 0, pObject[9]);
    if (g_unk0x00543ef8 != 0)
        g_unk0x00543ef8 = 0;
    FUN_00492bd0(index);
    FUN_00492f10();
}

// Sets the scene's ambient colour when it changes.
// FUNCTION: CMR2 0x00462cb0
void FUN_00462cb0(BYTE *pColour)
{
    BYTE ambient[4];

    Scene_GetAmbientColour((DWORD *)ambient);
    if (ambient[0] != pColour[0] || ambient[1] != pColour[1] || ambient[2] != pColour[2]) {
        if ((BYTE)RallyDataCountryIndex() == 3)
            Scene_SetAmbient(pColour, 0);
        else
            Scene_SetAmbient(pColour, 1);
    }
    if ((BYTE)FUN_00407270())
        FUN_0047e490(pColour);
}

// match 75%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00462d10
void FUN_00462d10(short *pRect)
{
    BYTE colour[4];
    int c;

    FUN_00492bb0(&c);
    colour[0] = (BYTE)c;
    colour[1] = (BYTE)(c >> 8);
    colour[2] = (BYTE)(c >> 16);
    g_sunVisibility = 100 - Flare_SampleVisibility(pRect, colour, 0x28);
    if (g_sunVisibility < 0) {
        g_sunVisibility = 0;
        return;
    }
    if (g_sunVisibility > 100)
        g_sunVisibility = 100;
}

// ---------------------------------------------------------------------------
// Stage lights: up to eight glowing lamps (e.g. start lights) whose on/off
// pattern per state comes from g_stageLightStates, plus the car lamp textures.

void FUN_004ae3d0(BYTE *p, BYTE value);
void FUN_004ae3f0(BYTE *p, int value);
void FUN_004ae260(void);
struct Unk0x004a3e20;
void FUN_004a3e20(Unk0x004a3e20 *pObject, int value);

struct StageLight {
    BYTE *pGlow;        // glow source
    BYTE *pGlow2;       // second glow when the kind doubles them
    int level;          // 0..1, eased towards the pattern
};

// Position (x, y in units of the transform axes) of each light per kind.
// GLOBAL: CMR2 0x0051b1a8
int g_stageLightOffsets[4][8][2] = {
    -112787, 246022, -75038, 246022, -38273, 246022, -196, 246022, 37093, 246022, 74579, 246022, 111935, 246022, 0, 0,
    -238616, 170590, -238616, 143130, -238616, 114950, -215220, 175964, -215220, 160104, -215220, 144244, -215220, 128385, -215220, 112525,
    -624558, 203882, -604962, 203882, -585891, 203882, -567934, 203882, -550633, 203882, -531693, 203882, -513736, 203882, 0, 0,
    -567672, 203882, -548143, 203882, -529072, 203882, -511049, 203882, -493748, 203882, -474873, 203882, -456851, 203882,
};
// Depth of the lights per kind.
// GLOBAL: CMR2 0x0051b2a8
int g_stageLightDepth[4] = {
    -58982, -38469, -31653, -31653,
};
// Colour (0 red, 1 green) of each light per kind.
// GLOBAL: CMR2 0x0051b2b8
int g_stageLightColour[4][8] = {
    0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 2, 2, 2, 2, 2,
    0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1,
};
// On/off pattern of each light, per kind (4), state (7) and light (8).
// GLOBAL: CMR2 0x0051b338
int g_stageLightStates[4][7][8] = {
    0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0,
    1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
    1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0,
    1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0,
    1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0,
    1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0,
    1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
    1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0,
    1,
};
// Offset (along the light row's right axis) of each light's second glow;
// 0 when the kind has only one glow per light.
// GLOBAL: CMR2 0x0051b6b8
int g_stageLightDouble[4] = {
    0, 459341, 1048444, 944439,
};
// Glow size per kind and colour.
// GLOBAL: CMR2 0x0051b6c8
int g_stageLightSize[4][3] = {
    19660, 19660, 0, 13107, 13107, 6553, 9830, 9830,
    0, 9830, 9830,
};
// Countries (bit per stage index) that use the eight-light layout.
// GLOBAL: CMR2 0x0051b6f8
unsigned short g_stageLightEightMask[10] = {
    0x0000, 0x03ff, 0x03ff, 0x0000, 0x0002, 0x0200, 0x03ff, 0x0250,
};
// GLOBAL: CMR2 0x00547ad8
int g_stageLightKind;
// GLOBAL: CMR2 0x00547adc
int g_stageLightHasMatrix;
// GLOBAL: CMR2 0x00547ae0
StageLight g_stageLights[8];
// GLOBAL: CMR2 0x00547b40
FixMatrix g_stageLightMatrix;
// GLOBAL: CMR2 0x00547cc8
Texture *g_stageLightRedTexture;
// GLOBAL: CMR2 0x00547ccc
Texture *g_stageLightGreenTexture;
// GLOBAL: CMR2 0x00547cd0
Texture *g_stageLightTexture;
// GLOBAL: CMR2 0x00547cd4
int g_stageLightsActive;
// GLOBAL: CMR2 0x00547cd8
int g_stageLightCount;

// Lamp textures of a car (0x4c bytes; two cars).
struct CarLights {
    BYTE field_0x0[0x2c];
    Texture *pBrake;        // 0x2c
    Texture *pReverse;      // 0x30
    Texture *pHazard;       // 0x34
    Texture *pHazard2;      // 0x38
    Texture *pHead;         // 0x3c
    BYTE field_0x40[0xc];
};

// GLOBAL: CMR2 0x00547f80
CarLights g_carLights[2];

// GLOBAL: CMR2 0x0051b724
char g_strLightRedTga[] = "\\NEWIMAGE\\lgt_red.tga";
// GLOBAL: CMR2 0x0051b70c
char g_strLightGreenTga[] = "\\NEWIMAGE\\lgt_gre.tga";
// GLOBAL: CMR2 0x0051b9d4
char g_strHazardLiteTga[] = "\\NEWIMAGE\\hazdlite.tga";
// GLOBAL: CMR2 0x0051b9bc
char g_strRevLiteTga[] = "\\NEWIMAGE\\revlite.tga";
// GLOBAL: CMR2 0x0051b9a4
char g_strHeadLiteTga[] = "\\NEWIMAGE\\headlite.tga";
// GLOBAL: CMR2 0x0051b98c
char g_strBrakeLiteTga[] = "\\NEWIMAGE\\brkelite.tga";

StageFile *StageTiming_GetStageFile0(void);

extern char g_strPathConcat[];

#define LOAD_STAGE_TEXTURE(dst, name)                                                          \
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::FUN_0040ed50(), name);    \
    dst = CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile0(), CFrontend::m_stringDest, \
                                    &loaded, NULL, 0, 0)

// Places the lights: rows of the given 4x3 vectors are the right, up,
// forward axes and the position (NULL: no transform).
// match 47%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00463290
void StageLights_SetTransform(FixVector *pAxes)
{
    FixVector v;

    if (pAxes == NULL) {
        g_stageLightHasMatrix = (int)pAxes;
        return;
    }
    g_stageLightHasMatrix = 1;
    v = pAxes[0];
    FixMatrix_SetRight(&v, &g_stageLightMatrix);
    v = pAxes[1];
    FixMatrix_SetUp(&v, &g_stageLightMatrix);
    v = pAxes[2];
    FixMatrix_SetForward(&v, &g_stageLightMatrix);
    v = pAxes[3];
    FixMatrix_SetPosition(&v, &g_stageLightMatrix);
}

// FUNCTION: CMR2 0x00463360
void StageLights_LoadTextures(void)
{
    bool loaded;

    LOAD_STAGE_TEXTURE(g_stageLightRedTexture, g_strLightRedTga);
    LOAD_STAGE_TEXTURE(g_stageLightGreenTexture, g_strLightGreenTga);
    FUN_004a3e20((Unk0x004a3e20 *)g_stageLightRedTexture, 1);
    FUN_004a3e20((Unk0x004a3e20 *)g_stageLightGreenTexture, 1);
    g_stageLightTexture = g_stageLightRedTexture;
}

// Eases every light towards its pattern for the current state and updates
// its glow(s).
// FUNCTION: CMR2 0x00463bd0
void StageLights_Update(void)
{
    StageLight *p;
    int target;
    int d;
    int i;

    if (g_stageLightsActive == 0)
        return;
    for (i = 0, p = g_stageLights; i < g_stageLightCount; i++, p++) {
        target = g_stageLightStates[g_stageLightKind][g_unk0x00547b80][i] != 0 ? 0x10000 : 0;
        d = target - p->level;
        if (FIX_ABS(d) < 0x3333)
            p->level = target;
        else if (d > 0)
            p->level += 0x3333;
        else
            p->level -= 0x3333;
        if (p->level > 0) {
            FUN_004ae3d0(p->pGlow, 1);
            if (g_stageLightDouble[g_stageLightKind] != 0)
                FUN_004ae3d0(p->pGlow2, 1);
        } else {
            FUN_004ae3d0(p->pGlow, 0);
            if (g_stageLightDouble[g_stageLightKind] != 0)
                FUN_004ae3d0(p->pGlow2, 0);
        }
        FUN_004ae3f0(p->pGlow, p->level);
        if (g_stageLightDouble[g_stageLightKind] != 0)
            FUN_004ae3f0(p->pGlow2, p->level);
    }
}

// Turns every light off (state 6).
// FUNCTION: CMR2 0x00463d00
void StageLights_Off(void)
{
    int i;

    g_unk0x00547b80 = 6;
    if (g_stageLightsActive == 0)
        return;
    for (i = 0; i < g_stageLightCount; i++) {
        g_stageLights[i].level = 0;
        FUN_004ae3d0(g_stageLights[i].pGlow, 0);
        if (g_stageLightDouble[g_stageLightKind] != 0)
            FUN_004ae3d0(g_stageLights[i].pGlow2, 0);
    }
}

// Resets the glow table and loads the lamp textures of both cars.
// FUNCTION: CMR2 0x00463d60
void CarLights_LoadTextures(void)
{
    bool loaded;

    FUN_004ae260();
    CGame::RegisterCallback(FUN_004ae260, NULL);
    LOAD_STAGE_TEXTURE(g_carLights[0].pHazard, g_strHazardLiteTga);
    LOAD_STAGE_TEXTURE(g_carLights[0].pReverse, g_strRevLiteTga);
    LOAD_STAGE_TEXTURE(g_carLights[0].pHead, g_strHeadLiteTga);
    LOAD_STAGE_TEXTURE(g_carLights[0].pBrake, g_strBrakeLiteTga);
    LOAD_STAGE_TEXTURE(g_carLights[0].pHazard2, g_strHazardLiteTga);
    LOAD_STAGE_TEXTURE(g_carLights[1].pHazard, g_strHazardLiteTga);
    LOAD_STAGE_TEXTURE(g_carLights[1].pReverse, g_strRevLiteTga);
    LOAD_STAGE_TEXTURE(g_carLights[1].pHead, g_strHeadLiteTga);
    LOAD_STAGE_TEXTURE(g_carLights[1].pBrake, g_strBrakeLiteTga);
    LOAD_STAGE_TEXTURE(g_carLights[1].pHazard2, g_strHazardLiteTga);
}

void FUN_00492890(FixVector *pOut);
void FUN_004928c0(int *pOut1, int *pOut2, int *pOut3);
void FUN_00498370(FixVector *v);

// Direction of the stage light (sun direction raised by the sky offset),
// normalised; passed on to FUN_00498370.
// GLOBAL: CMR2 0x005477f8
FixVector g_stageLightDirection;

// match 85%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00463070
void StageLights_UpdateDirection(void)
{
    FixVector d;
    FixVector s;
    int lift;
    int unused1;
    int unused2;
    int m;

    FUN_00492890(&d);
    FUN_004928c0(&lift, &unused1, &unused2);
    d.y += lift;
    if (FIX_ABS(d.x) > FIX_ABS(d.y) && FIX_ABS(d.x) > FIX_ABS(d.z))
        m = FIX_ABS(d.x);
    else if (FIX_ABS(d.y) > FIX_ABS(d.x) && FIX_ABS(d.y) > FIX_ABS(d.z))
        m = FIX_ABS(d.y);
    else
        m = FIX_ABS(d.z);
    FixVecScaleRecip(&s, &d, m);
    FIX_NORMALIZE_INTO(g_stageLightDirection, s);
    FUN_00498370(&g_stageLightDirection);
}

// Light row axes with the scale removed.
// GLOBAL: CMR2 0x00547b88
FixMatrix g_stageLightBasis;

unsigned char RallyDataStageIndex(void);
struct GlowLight;
GlowLight *Glow_Add(int type, FixVector *pos, FixVector *dir, int unused1, int sizeX, int sizeY, int billboardTexture,
                    int layerTexture, int intensity, int node, BYTE projected, int unused2, int field_0x40);

#define LIGHT_SIZE(i) FixMul(lenX, g_stageLightSize[g_stageLightKind][g_stageLightColour[g_stageLightKind][i]])

// Creates the glows of the stage lights (start gantry) from the placed
// transform: layout by country and stage, one or two glows per light, all off.
// match 53%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00463410
void StageLights_Create(void)
{
    FixVector pos[8];
    FixVector v;
    FixVector local;
    FixVector zero;
    FixVector dir;
    FixVector origin;
    FixVector pair;
    int lenX, lenY, lenZ;
    BYTE stage;
    int i;

    local.x = 0;
    local.y = 0;
    local.z = -0x10000;
    zero.x = 0;
    zero.y = 0;
    zero.z = 0;
    g_stageLightsActive = 0;
    if (g_stageLightHasMatrix == 0)
        return;
    FixMatrix_GetPosition(&origin, &g_stageLightMatrix);
    if (origin.x == 0 && origin.y == 0 && origin.z == 0)
        return;
    stage = RallyDataStageIndex();
    if ((char)RallyDataStageIndex() == 10) {
        g_stageLightCount = 7;
        if ((char)RallyDataCountryIndex() == 3)
            g_stageLightKind = 2;
        else
            g_stageLightKind = 3;
    } else if ((g_stageLightEightMask[RallyDataCountryIndex() & 0xff] & (unsigned short)(1 << stage)) == 0) {
        g_stageLightCount = 7;
        g_stageLightKind = 0;
    } else {
        g_stageLightCount = 8;
        g_stageLightKind = 1;
    }
    FixMatrix_GetRight(&v, &g_stageLightMatrix);
    lenX = FixVecLength(&v);
    FixVecScaleRecip(&v, &v, lenX);
    FixMatrix_SetRight(&v, &g_stageLightBasis);
    FixMatrix_GetUp(&v, &g_stageLightMatrix);
    lenY = FixVecLength(&v);
    FixVecScaleRecip(&v, &v, lenY);
    FixMatrix_SetUp(&v, &g_stageLightBasis);
    FixMatrix_GetForward(&v, &g_stageLightMatrix);
    lenZ = FixVecLength(&v);
    FixVecScaleRecip(&v, &v, lenZ);
    FixMatrix_SetForward(&v, &g_stageLightBasis);
    FixMatrix_RotateVector(&dir, &local, &g_stageLightBasis);
    FIX_NORMALIZE_INTO(dir, dir);
    FixMatrix_GetRight(&pair, &g_stageLightMatrix);
    FixVecScale(&pair, &pair, g_stageLightDouble[g_stageLightKind]);
    for (i = 0; i < g_stageLightCount; i++) {
        v.x = FixMul(g_stageLightOffsets[g_stageLightKind][i][0], lenX);
        v.y = FixMul(g_stageLightOffsets[g_stageLightKind][i][1], lenY);
        v.z = FixMul(g_stageLightDepth[g_stageLightKind], lenZ);
        FixMatrix_RotateVector(&pos[i], &v, &g_stageLightBasis);
        pos[i].x += origin.x;
        pos[i].y += origin.y;
        pos[i].z += origin.z;
        g_stageLights[i].pGlow = (BYTE *)Glow_Add(
            2, &pos[i], &dir, (int)&zero, LIGHT_SIZE(i), LIGHT_SIZE(i),
            (int)(&g_stageLightRedTexture)[g_stageLightColour[g_stageLightKind][i]],
            (int)(&g_stageLightRedTexture)[g_stageLightColour[g_stageLightKind][i]], 0, 0, 0, (int)&dir, 0);
        if (g_stageLightDouble[g_stageLightKind] != 0) {
            pos[i].x += pair.x;
            pos[i].y += pair.y;
            pos[i].z += pair.z;
            g_stageLights[i].pGlow2 = (BYTE *)Glow_Add(
                2, &pos[i], &dir, (int)&zero, LIGHT_SIZE(i), LIGHT_SIZE(i),
                (int)(&g_stageLightRedTexture)[g_stageLightColour[g_stageLightKind][i]],
                (int)(&g_stageLightRedTexture)[g_stageLightColour[g_stageLightKind][i]], 0, 0, 0, (int)&dir, 0);
        }
        g_stageLights[i].level = 0;
        FUN_004ae3d0(g_stageLights[i].pGlow, 0);
        if (g_stageLightDouble[g_stageLightKind] != 0)
            FUN_004ae3d0(g_stageLights[i].pGlow2, 0);
    }
    g_unk0x00547b80 = 6;
    g_stageLightsActive = 1;
}

// ---------------------------------------------------------------------------
// Replay buffers and stage event records.

extern int g_unk0x00588d3c;
extern int g_unk0x00588d14;
extern void *g_unk0x00588e80[8];

// Second set of eight buffers and the per-slot pointers into both sets.
// GLOBAL: CMR2 0x00588d40
void **g_unk0x00588d40[8];
// GLOBAL: CMR2 0x00588d60
void **g_unk0x00588d60[8];
// GLOBAL: CMR2 0x00588ea0
void *g_unk0x00588ea0[8];

// FUNCTION: CMR2 0x0046c540
void Replay_InitSlots(void)
{
    int i;

    g_unk0x00588d3c = 0;
    memset(g_unk0x00588e80, 0, sizeof(g_unk0x00588e80));
    for (i = 0; i < 8; i++)
        g_unk0x00588d40[i] = &g_unk0x00588e80[i];
    g_unk0x00588d14 = 0;
    memset(g_unk0x00588ea0, 0, sizeof(g_unk0x00588ea0));
    for (i = 0; i < 8; i++)
        g_unk0x00588d60[i] = &g_unk0x00588ea0[i];
}

// GLOBAL: CMR2 0x00588d18
int g_unk0x00588d18[8];
// GLOBAL: CMR2 0x00588ec8
int g_unk0x00588ec8;

// Frees the replay buffers (the second set only when not needed any more).
// match 65%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046c6d0
int Replay_FreeBuffers(void)
{
    int i;

    for (i = 0; i < 8; i++) {
        if (g_unk0x00588e80[i] != NULL) {
            CFileBuffer::FreeGenericFileBuffer(g_unk0x00588e80[i]);
            g_unk0x00588e80[i] = NULL;
        }
        if (CGameInfo::FUN_00406320() || CGameInfo::FUN_00405d80() == 3 || CGameInfo::FUN_00405d80() == 7) {
            if (g_unk0x00588ea0[i] != NULL && g_unk0x00588d18[i] == 0) {
                CFileBuffer::FreeGenericFileBuffer(g_unk0x00588ea0[i]);
                g_unk0x00588ea0[i] = NULL;
                g_unk0x00588d18[i] = 0;
            }
        }
    }
    g_unk0x00588d3c = 0;
    g_unk0x00588d14 = 0;
    g_unk0x00588ec8 = 0;
    return 1;
}

// Stops recording into a replay buffer, closing the current segment.
// match 36%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046cc60
int Replay_StopRecording(BYTE *pBuffer)
{
    short *pCount;

    if (pBuffer == NULL || *(int *)(pBuffer + 0xc) == 0)
        return 0;
    *(int *)(pBuffer + 0xc) = 0;
    if (*(int *)(pBuffer + 0x10) == 0) {
        if (*(int *)(pBuffer + 0x1c) != 2)
            goto done;
    } else if (*(int *)(pBuffer + 0x1c) != 2) {
        pCount = (short *)(*(int *)(pBuffer + 0x104) + *(short *)(pBuffer + 0x100) * 2);
        (*pCount)++;
        (*(short *)(pBuffer + 0x100))++;
        *(int *)(pBuffer + 0x10) = 0;
        *(int *)(pBuffer + 0x18) = 0;
        return 1;
    }
    *(short *)(pBuffer + 0x100) = 1;
done:
    *(int *)(pBuffer + 0x10) = 0;
    *(int *)(pBuffer + 0x18) = 0;
    return 1;
}

// GLOBAL: CMR2 0x0051bfec
char g_strReplaySaved[] = "\n";

// Writes a replay buffer (header plus its used arrays) to a file.
// FUNCTION: CMR2 0x0046d3f0
int Replay_Save(BYTE *pBuffer, char *pName)
{
    int frames;
    int extra;
    int records;

    frames = *(short *)(pBuffer + 0xfc);
    if (*(int *)(pBuffer + 0x1c) == 2)
        extra = *(short *)(pBuffer + 0xfe) * frames * 0x10;
    else
        extra = *(short *)(pBuffer + 0xfe) * frames * 4;
    if (*(int *)(pBuffer + 0x1c) == 0)
        records = frames * 0x114c;
    else
        records = frames * 0x5c;
    CInstallInfo::WriteFileToDisk(pName, 0, pBuffer, records + 0x110 + frames * 2 + extra);
    puts(pName);
    puts(g_strReplaySaved);
    return 1;
}

// Points the arrays of a replay buffer into its data block (after the 0x110
// header), according to its type.
// match 45%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046d470
void Replay_SetupPointers(BYTE *pBuffer, int unused)
{
    int recordSize;
    int frames;
    int extraSize;

    if (*(int *)(pBuffer + 0x1c) == 0) {
        *(BYTE **)(pBuffer + 0x24) = pBuffer + 0x110;
        *(BYTE **)(pBuffer + 0x30) = NULL;
        recordSize = 0x114c;
    } else {
        *(BYTE **)(pBuffer + 0x30) = pBuffer + 0x110;
        *(BYTE **)(pBuffer + 0x24) = NULL;
        recordSize = 0x5c;
    }
    frames = *(short *)(pBuffer + 0xfc);
    if (*(int *)(pBuffer + 0x1c) == 2) {
        *(BYTE **)(pBuffer + 0x3c) = NULL;
        extraSize = 0x10;
        *(BYTE **)(pBuffer + 0x40) = pBuffer + frames * recordSize + 0x110;
    } else {
        *(BYTE **)(pBuffer + 0x40) = NULL;
        extraSize = 4;
        *(BYTE **)(pBuffer + 0x3c) = pBuffer + frames * recordSize + 0x110;
    }
    *(BYTE **)(pBuffer + 0x104) = pBuffer + (*(short *)(pBuffer + 0xfe) * extraSize + recordSize) * frames + 0x110;
}

// Velocity estimate between two matrices (current at +0, previous at +0x40):
// (difference - 3 * offset) / 6, stored at +0x80.
// match 79%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046e340
void FUN_0046e340(BYTE *pMatrices, BYTE *pInfo)
{
    FixVector a;
    FixVector b;
    FixVector d;
    FixVector o;

    FixMatrix_GetPosition(&a, (FixMatrix *)pMatrices);
    FixMatrix_GetPosition(&b, (FixMatrix *)(pMatrices + 0x40));
    d.x = b.x - a.x;
    d.y = b.y - a.y;
    d.z = b.z - a.z;
    FixVecScale(&o, (FixVector *)(pInfo + 0x408), 0x30000);
    d.x -= o.x;
    d.y -= o.y;
    d.z -= o.z;
    FixVecScale((FixVector *)(pMatrices + 0x80), &d, 0x2aaa);
}

// Timed stage events (0x1c bytes each).
struct EventRec {
    int pos;            // 0x0  packed x/y of the event's area in the texture
    short a;            // 0x4  width
    short b;            // 0x6  height
    BYTE field_0x8[8];
    unsigned short step;    // 0x10
    short range;        // 0x12
    unsigned short counter; // 0x14
    short field_0x16;   // 0x16
    BYTE paused;        // 0x18
    BYTE active;        // 0x19 has an area
    BYTE pad_0x1a[2];
};

// GLOBAL: CMR2 0x00588ed8
EventRec g_eventRecords[29];
// GLOBAL: CMR2 0x00589210
int g_eventScale;
// GLOBAL: CMR2 0x00589318
int g_unk0x00589318;
// GLOBAL: CMR2 0x0058931c
int g_eventCount;
// GLOBAL: CMR2 0x00589320
int g_unk0x00589320[4];
// GLOBAL: CMR2 0x00589330
BYTE g_eventsDirty;
// GLOBAL: CMR2 0x00588ed0
Texture *g_eventTexture;

// Scales the event steps by their share of the largest a*b product.
// FUNCTION: CMR2 0x0046e6a0
void Events_ComputeSteps(void)
{
    EventRec *p;
    int maxProduct;
    int product;
    int share;
    int i;

    maxProduct = 0;
    for (i = g_eventCount, p = g_eventRecords; i > 0; i--, p++) {
        product = p->b * p->a;
        if (product > maxProduct)
            maxProduct = product;
    }
    for (i = 0, p = g_eventRecords; i < g_eventCount; i++, p++) {
        share = FixDiv((p->b * p->a) << 16, (maxProduct / 2) << 16);
        p->step = (unsigned short)FixMul(share, g_eventScale);
        p->range = (short)FixMul(share, 30000);
        p->counter = 0;
        p->field_0x16 = 0;
        p->paused = 0;
    }
    if (g_eventCount > 2) {
        i = g_eventCount - 2;
        p = &g_eventRecords[2];
        do {
            p->step >>= 1;
            if (p->step == 0)
                p->step = 1;
            p++;
        } while (--i);
    }
}

// GLOBAL: CMR2 0x00589331
BYTE g_unk0x00589331;

// GLOBAL: CMR2 0x00589334
int g_unk0x00589334;

// Stamp patterns of the event sprites: a 4x4 grid per rotation and a 16x16
// grid per rotation.
// GLOBAL: CMR2 0x0051c240
BYTE g_unk0x0051c240[0x40] = {
    0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
    0x00, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00,
    0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
};
// GLOBAL: CMR2 0x0051c280
BYTE g_unk0x0051c280[0x400] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x00,
    0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x01, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x00,
    0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00,
    0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x00,
    0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

void Events_Reset(void);

// Resets the stage events and finds the event texture (name ending in BODF).
// FUNCTION: CMR2 0x0046e580
void Events_Init(int unused, int slot, char animate)
{
    int i;
    Texture *pTexture;

    Events_Reset();
    g_unk0x00589331 = animate == 0;
    g_eventCount = 0;
    for (i = 0; i < 2048; i++) {
        pTexture = CGraphics::m_pTextureManager->textureBuffer[i];
        if (pTexture != NULL &&
            strncmp(pTexture->name + strlen(pTexture->name) - 8, CGraphics::m_strSuffixBODF, 4) == 0) {
            (&g_eventTexture)[slot] = CGraphics::m_pTextureManager->textureBuffer[i];
            break;
        }
    }
    if ((&g_eventTexture)[slot] != NULL)
        (&g_eventScale)[slot] = (&g_eventTexture)[slot]->width;
}

// Adds a stage event (up to 11) for the texture area pArea (packed position,
// width, height) and recomputes the event steps.
// match 20%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046e620
void Events_Add(EventRec *pArea, int unused)
{
    EventRec *p;

    if (g_eventCount < 11) {
        p = &g_eventRecords[g_eventCount];
        p->active = 0;
        p->paused = 0;
        p->step = 0;
        p->counter = 0;
        if (pArea != NULL && pArea->a > 0 && pArea->b > 0) {
            p->pos = pArea->pos;
            *(int *)&p->a = *(int *)&pArea->a;
            p->active = 1;
        }
        g_eventCount++;
    }
    Events_ComputeSteps();
}

// match 66%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046e530
void Events_Reset(void)
{
    EventRec *p;
    int i;

    g_unk0x00589318 = 0;
    if (g_eventCount > 0) {
        p = g_eventRecords;
        for (i = g_eventCount; i != 0; i--, p++) {
            p->counter = 0;
            p->field_0x16 = 0;
            p->paused = 0;
        }
    }
    g_eventsDirty = 0;
    g_unk0x00589320[0] = 0;
    g_unk0x00589320[1] = 0;
    g_unk0x00589320[2] = 0;
    g_unk0x00589320[3] = 0;
}

// Advances an event's counter; returns 1 when it wraps past 255.
// match 41%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046ed40
int Events_Tick(int index)
{
    int wrapped;

    wrapped = 0;
    if (g_eventRecords[index].paused == 0) {
        g_eventRecords[index].counter += g_eventRecords[index].step;
        if (g_eventRecords[index].counter > 0xff) {
            g_eventRecords[index].counter = 0;
            wrapped = 1;
        }
    }
    return wrapped;
}

void Graphics_ReloadTexture(Texture *pTexture);

// FUNCTION: CMR2 0x0046ef20
void Events_Flush(void)
{
    if (g_eventCount > 0 && g_eventsDirty != 0) {
        Graphics_ReloadTexture(g_eventTexture);
        Events_Reset();
    }
}

// Cloud cover (0 light .. 2 heavy) of each weather setting.
// GLOBAL: CMR2 0x0051c688
BYTE g_cloudLevels[9] = { 1, 1, 2, 2, 2, 2, 2, 2, 2 };
// GLOBAL: CMR2 0x0051c694
char g_strCloudLight[] = "Cloud_Light";
// GLOBAL: CMR2 0x0051c6a0
char g_strCloudMed[] = "Cloud_Med";
// GLOBAL: CMR2 0x0051c6ac
char g_strCloudHeavy[] = "Cloud_Heavy";
// GLOBAL: CMR2 0x0051c6b8
char g_strPcLow[] = "PcLow\\";
// GLOBAL: CMR2 0x0051c6c0
char g_strPc[] = "Pc\\";
// GLOBAL: CMR2 0x0051c6c4
char g_strCloudsDir[] = "%s\\Clouds\\";

int *FUN_00407520(int index);

// Builds the path of the stage's cloud texture in CFrontend::m_stringDest
// from the heavier of the two weather settings.
// match 79%: the two level loads stay in byte registers (cl) instead of being
// zero-extended into 32-bit ones (bl/edx) as in the original
// FUNCTION: CMR2 0x0046ef50
void FUN_0046ef50(void)
{
    int *pPair;
    int level;
    char *suffix;

    sprintf(CFrontend::m_stringDest, g_strCloudsDir, CInstallInfo::GetTracksDir());
    if (CGameInfo::FUN_00405d00() == 0)
        suffix = g_strPc;
    else
        suffix = g_strPcLow;
    strcat(CFrontend::m_stringDest, suffix);
    pPair = FUN_00407520(RallyDataStageIndex());
    level = 0;
    if (g_cloudLevels[pPair[0]] > 0)
        level = g_cloudLevels[pPair[0]];
    if (g_cloudLevels[pPair[1]] > level)
        level = g_cloudLevels[pPair[1]];
    switch (level) {
    case 2:
        strcat(CFrontend::m_stringDest, g_strCloudHeavy);
        break;
    case 1:
        strcat(CFrontend::m_stringDest, g_strCloudMed);
        break;
    case 0:
        strcat(CFrontend::m_stringDest, g_strCloudLight);
        break;
    }
}

// Releases the file loaded by 0x46f060 (registered callback).
// FUNCTION: CMR2 0x0046f030
BYTE FUN_0046f030(void)
{
    if (g_unk0x00589448.buffer) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x00589448.buffer);
        g_unk0x00589448.buffer = NULL;
    }
    g_unk0x00589448.didFileLoad = FALSE;
    g_unk0x00589448.fileSize = 0;
    return 1;
}

int FUN_00422f50(BYTE index);

// Places the four view nodes of a split screen from the current view position.
// match 80%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046f330
void FUN_0046f330(int param_1)
{
    int car;
    int node;
    FixVector pos;
    FixVector vecB;
    FixVector vecC;
    FixVector offset;

    car = param_1;
    node = FUN_00422f50(car);
    if (node == 3)
        node = (int)Car_Get(car)->pNode0x720;
    else
        node = (int)g_viewNodes[car];
    FixMatrix_GetPosition(&pos, (FixMatrix *)(node + 0x98));
    vecC.x = pos.x;
    vecB.x = pos.x;
    vecC.z = pos.z;
    vecB.z = pos.z;
    pos.y = pos.y - 0xf0000;
    vecC.y = pos.y;
    vecB.y = pos.y;
    FUN_004928c0(&offset.x, &offset.y, &offset.z);
    pos.y = pos.y + offset.x;
    vecC.y = vecC.y + offset.y;
    vecB.y = vecB.y + offset.z;
    if ((char)RallyDataCountryIndex() == 3 && (char)RallyDataStageIndex() == 7)
        vecB.y = vecB.y - 0x40000;
    if (g_unk0x00589438 != 0)
        SceneNode_SetPosition((SceneNode *)g_unk0x00589438, &pos);
    if (g_unk0x0058943c != 0)
        SceneNode_SetPosition((SceneNode *)g_unk0x0058943c, &vecC);
    if (g_unk0x00589440 != 0)
        SceneNode_SetPosition((SceneNode *)g_unk0x00589440, &vecB);
    if (g_unk0x00589444 != 0) {
        vecB.y = vecB.y - 0x140000;
        SceneNode_SetPosition((SceneNode *)g_unk0x00589444, &vecB);
    }
    FUN_004928f0(&param_1);
    offset.x = 0;
    offset.y = param_1;
    offset.z = 0;
    FixMatrix_SetUp(&offset, (FixMatrix *)((BYTE *)g_unk0x00589438 + 0x98));
}

void FUN_00486b20(BYTE *pCar, BYTE *pInfo);
void FUN_00486b90(BYTE *pCar, BYTE *pInfo);

#define RECORD_NEAR_90(v) (((v) > 0x3f4 && (v) < 0x40b) || ((v) < -0x3f4 && (v) > -0x40b))

// FUNCTION: CMR2 0x0048d7b0
void FUN_0048d7b0(BYTE *pCar, BYTE *pInfo)
{
    short v;

    v = *(short *)(g_unk0x00591750 + 2 + g_unk0x00591740[*pCar] * 0x6c);
    if (RECORD_NEAR_90(v))
        FUN_00486b20(pCar, pInfo);
}

// FUNCTION: CMR2 0x0048d800
void FUN_0048d800(BYTE *pInfo, BYTE *pCar)
{
    short v;

    v = *(short *)(g_unk0x00591750 + 2 + g_unk0x00591740[*pCar] * 0x6c);
    if (RECORD_NEAR_90(v))
        FUN_00486b90(pCar, pInfo);
}

// Headlight glows of the stage objects (100 records of 0x5c bytes).
// GLOBAL: CMR2 0x0058e4e0
BYTE g_unk0x0058e4e0[100][0x5c];

// Creates the glow of every record and resets the records.
// match 89%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0047d510
void FUN_0047d510(void)
{
    FixVector unused;
    BYTE *p;

    for (p = g_unk0x0058e4e0[0]; (int)p < (int)g_unk0x0058e4e0[100]; p += 0x5c) {
        *(int *)(p + 0x3c) = 0;
        *(GlowLight **)(p + 0x38) =
            Glow_Add(1, &unused, &unused, (int)&unused, 0x3333, 0x3333, (int)g_carLights[0].pHazard,
                     (int)g_carLights[1].pHazard, 0x10000, 0, 0xb4, (int)&unused, 0x20000);
        FUN_004ae3d0(*(BYTE **)(p + 0x38), 0);
        *(int *)(p + 0x2c) = 0;
        *(short *)(p + 0x34) = -1;
        *(int *)(p + 0x0) = 0;
        *(int *)(p + 0x4) = 0x10000;
        *(int *)(p + 0x8) = 0;
    }
    memset(g_unk0x0058e4a8, 0, sizeof(g_unk0x0058e4a8));
}

#include "WheelTrail.h"

// Skid mark points of every car wheel: a used flag and a 0x28-byte record
// for each of the 200 points.
// GLOBAL: CMR2 0x00548220
BYTE g_trailPointUsed[8][4][200];
// GLOBAL: CMR2 0x00549b20
int g_unk0x00549b20[8][4];
// GLOBAL: CMR2 0x00549da0
BYTE g_trailPoints[8][4][200][0x28];

// Clears every wheel's skid marks and trail state.
// FUNCTION: CMR2 0x00465530
void FUN_00465530(void)
{
    int car;
    int point;
    int wheel;

    for (car = 0; car < 8; car++) {
        for (point = 0; point < 200; point++) {
            for (wheel = 0; wheel < 4; wheel++) {
                g_trailPointUsed[car][wheel][point] = 0;
                *(int *)g_trailPoints[car][wheel][point] = 0;
            }
        }
    }
    memset(g_unk0x00549ba0, 0, sizeof(g_unk0x00549ba0));
    memset(g_trailState, 0, sizeof(g_trailState));
    memset(g_trailTimer, 0, sizeof(g_trailTimer));
    memset(g_trailPrevState, 0, sizeof(g_trailPrevState));
    memset(g_unk0x00549b20, 0, sizeof(g_unk0x00549b20));
    memset(g_unk0x00543708, 0, sizeof(g_unk0x00543708));
    for (wheel = 0; wheel < 8 * 4; wheel++)
        ((int *)g_trailReset)[wheel] = 1;
    for (car = 0; car < 8; car++) {
        FixVector *pDelta = g_trailDelta[car];
        for (wheel = 0; wheel < 4; wheel++) {
            pDelta->x = 0;
            pDelta->y = 0;
            pDelta->z = 0;
            pDelta++;
        }
    }
}

// GLOBAL: CMR2 0x0051bce0
char g_strNewImageSkidBlankTga[] = "\\NEWIMAGE\\skid\\skids_blank.tga";
// GLOBAL: CMR2 0x0051bd00
char g_strNewImageSkidMarkTga[] = "\\NEWIMAGE\\skid\\skids_mark.tga";
// GLOBAL: CMR2 0x0051bd20
char g_strNewImageSkid1Tga[] = "\\NEWIMAGE\\skid\\skids_1.tga";

// GLOBAL: CMR2 0x00588740
Texture *g_unk0x00588740;
// GLOBAL: CMR2 0x00588744
Texture *g_unk0x00588744;
// GLOBAL: CMR2 0x00588748
Texture *g_unk0x00588748;

// Loads the three skid mark textures and picks the trail lifetime and the
// skid colour for the current weather and country.
// FUNCTION: CMR2 0x00465600
void FUN_00465600(void)
{
    char name[260];
    bool loaded;
    BYTE country;

    FUN_00465530();
    sprintf(name, g_strPathConcat, CInstallInfo::FUN_0040ed50(), g_strNewImageSkid1Tga);
    g_unk0x00588740 = CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile0(), name, &loaded, 0, 0, 0);
    sprintf(name, g_strPathConcat, CInstallInfo::FUN_0040ed50(), g_strNewImageSkidMarkTga);
    g_unk0x00588744 = CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile0(), name, &loaded, 0, 0, 0);
    sprintf(name, g_strPathConcat, CInstallInfo::FUN_0040ed50(), g_strNewImageSkidBlankTga);
    g_unk0x00588748 = CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile0(), name, &loaded, 0, 0, 0);
    if (CGameInfo::FUN_00405d80() == 5 || CGameInfo::FUN_00405d80() == 6 ||
        CGameInfo::FUN_00405d80() == 7)
        g_stageSurfaceInfo[8].flags = 4;
    else
        g_stageSurfaceInfo[8].flags = 2;
    country = (BYTE)RallyDataCountryIndex();
    if (country != 0) {
        if (country > 3 && country <= 5) {
            g_stageSurfaceInfo[2].red = 0xb7;
            g_stageSurfaceInfo[2].green = 0x89;
            g_stageSurfaceInfo[2].blue = 0x43;
            *(int *)&g_stageSurfaceInfo[7].red = *(int *)&g_stageSurfaceInfo[2].red;
        } else {
            g_stageSurfaceInfo[2].red = 0xb7;
            g_stageSurfaceInfo[2].green = 0x89;
            g_stageSurfaceInfo[2].blue = 0x43;
            *(int *)&g_stageSurfaceInfo[7].red = *(int *)&g_stageSurfaceInfo[2].red;
        }
    } else {
        g_stageSurfaceInfo[2].red = 0x89;
        g_stageSurfaceInfo[2].green = 0x7a;
        g_stageSurfaceInfo[2].blue = 0x6f;
        *(int *)&g_stageSurfaceInfo[7].red = *(int *)&g_stageSurfaceInfo[2].red;
    }
}

// GLOBAL: CMR2 0x00590b50
FixVector g_unk0x00590b50;

int FixMatrix_InverseRotateVector(FixVector *pOut, FixVector *pV, FixMatrix *pM);
extern float g_oneOverRandMax;

// Applies an impulse (scaled by a random 0.7..1.0) against the vehicle's
// velocity, in its body frame.
// match 34%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004853c0
void FUN_004853c0(FixVector *pImpulse)
{
    BYTE *pVehicle = (BYTE *)g_unk0x00590d74;
    FixVector scaled;
    FixVector local;
    int random;
    int scale;

    FixMatrix_InverseRotateVector(&g_unk0x00590b50, (FixVector *)(pVehicle + 0x408), *(FixMatrix **)(pVehicle + 0x750));
    random = (int)(__int64)(rand() * g_oneOverRandMax * CGraphics::m_65536);
    scale = FixMul(random, 0x4ccc) + 0xb333;
    scaled.x = FixMul(pImpulse->x, scale);
    scaled.y = FixMul(pImpulse->y, scale);
    scaled.z = FixMul(pImpulse->z, scale);
    FixMatrix_InverseRotateVector(&local, &scaled, *(FixMatrix **)(pVehicle + 0x750));
    g_unk0x00590b50.y -= local.y;
    g_unk0x00590b50.x -= local.x;
    g_unk0x00590b50.z -= local.z;
}

void SceneNode_SetViewMaskTree(SceneNode *pNode, BYTE mask);
int FUN_0046bec0(int *pState, BYTE *pIn, BYTE *pOut, BYTE *pCounter, Car *pCar);

// Hides every node of the cars whose replay buffer is a finished ghost run.
// FUNCTION: CMR2 0x0046e440
void FUN_0046e440(void)
{
    void ***pp;
    BYTE *pBuffer;
    Car *pCar;

    for (pp = g_unk0x00588d40; pp < g_unk0x00588d40 + 8; pp++) {
        pBuffer = (BYTE *)**pp;
        if (pBuffer != NULL && *(int *)(pBuffer + 4) != 0 && *(int *)(pBuffer + 0x1c) == 2 &&
            *(int *)(pBuffer + 0xe8) == 0) {
            pCar = Car_Get(pBuffer[0x20]);
            SceneNode_SetViewMaskTree(pCar->pNode0x720, 0);
            SceneNode_SetViewMaskTree(pCar->pWheelNodes[0], 0);
            SceneNode_SetViewMaskTree(pCar->pWheelNodes[1], 0);
            SceneNode_SetViewMaskTree(pCar->pWheelNodes[2], 0);
            SceneNode_SetViewMaskTree(pCar->pWheelNodes[3], 0);
            if (pCar->pNode0x724 != NULL)
                SceneNode_SetViewMaskTree(pCar->pNode0x724, 0);
            if (pCar->pExtraNodes[0] != NULL) {
                SceneNode_SetViewMaskTree(pCar->pExtraNodes[0], 0);
                SceneNode_SetViewMaskTree(pCar->pExtraNodes[1], 0);
                SceneNode_SetViewMaskTree(pCar->pExtraNodes[2], 0);
                SceneNode_SetViewMaskTree(pCar->pExtraNodes[3], 0);
            }
        }
    }
}

// Decodes a replay input packet into a car's control record.
// FUNCTION: CMR2 0x0046c4b0
int FUN_0046c4b0(int *pState, BYTE *pIn, BYTE car, BYTE *pCounter)
{
    Car *pCar = Car_Get(car);

    return FUN_0046bec0(pState, pIn, (BYTE *)pCar + 0x1d0, pCounter, pCar);
}

// GLOBAL: CMR2 0x0058e0a0
Car *g_unk0x0058e0a0;
// Which of the steering/throttle/brake controls are analogue.
// GLOBAL: CMR2 0x0058e0a8
BYTE g_unk0x0058e0a8[3];
// Button bit of each car control (read from the controller mapping).
// GLOBAL: CMR2 0x0051f4b0
unsigned short g_carButtonMasks[9] = { 1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80, 0x100 };

short *Car_GetOrder(void);

// Resets a car's controls for the start of the stage (automatic box on,
// velocity damped).
// match 23%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0047b870
void FUN_0047b870(int index)
{
    BYTE *p;

    g_unk0x0058e0a0 = Car_Get(Car_GetOrder()[index]);
    p = (BYTE *)g_unk0x0058e0a0;
    *(int *)(p + 0x1dc) = 0;
    g_unk0x0058e0a0->field_0x1d8 = 0;
    p[0x1d4] = 0;
    g_unk0x0058e0a0->flag0x1d0[3] = 0;
    g_unk0x0058e0a0->flag0x1d0[2] = 0;
    g_unk0x0058e0a0->flag0x1d0[1] = 0;
    g_unk0x0058e0a0->flag0x1d0[0] = 0;
    if (g_unk0x0058e0a0->field_0xb94 == 0)
        g_unk0x0058e0a0->flag0x1d0[3] = 1;
    else
        g_unk0x0058e0a0->flag0x1d0[3] = 0;
    g_unk0x0058e0a0->flag0x1d0[2] = 0;
    g_unk0x0058e0a0->field_0x1d8 = 1;
    g_unk0x0058e0a0->field_0xb9c = 0;
    *(int *)(p + 0x1e4) = 0;
    p = (BYTE *)g_unk0x0058e0a0;
    g_unk0x0058e0a0->velocity.x = FixMul(g_unk0x0058e0a0->velocity.x, 0xf851);
    ((Car *)p)->velocity.y = FixMul(((Car *)p)->velocity.y, 0xf851);
    ((Car *)p)->velocity.z = FixMul(((Car *)p)->velocity.z, 0xf851);
}

DWORD FUN_0040bdd0(unsigned short slot);

// Reads the controller mapping of a slot into the car control masks and the
// analogue flags.
// FUNCTION: CMR2 0x0047bca0
void FUN_0047bca0(int slot)
{
    g_carButtonMasks[0] = CInput::GetButtonMapping(slot, 0);
    g_carButtonMasks[1] = CInput::GetButtonMapping(slot, 1);
    g_carButtonMasks[2] = CInput::GetButtonMapping(slot, 2);
    g_carButtonMasks[3] = CInput::GetButtonMapping(slot, 3);
    g_carButtonMasks[4] = CInput::GetButtonMapping(slot, 4);
    g_carButtonMasks[5] = CInput::GetButtonMapping(slot, 5);
    g_carButtonMasks[6] = CInput::GetButtonMapping(slot, 6);
    g_carButtonMasks[7] = CInput::GetButtonMapping(slot, 7);
    g_carButtonMasks[8] = CInput::GetButtonMapping(slot, 8);
    if (FUN_0040bdd0(slot) == 0 || (int)CInput::FUN_0040c210(slot, 0) == -1)
        g_unk0x0058e0a8[0] = 0;
    else
        g_unk0x0058e0a8[0] = 1;
    if (CInput::FUN_0040be00(slot) == 0 || (int)CInput::FUN_0040c210(slot, 2) == -1)
        g_unk0x0058e0a8[1] = 0;
    else
        g_unk0x0058e0a8[1] = 1;
    if (CInput::FUN_0040be00(slot) != 0 && (int)CInput::FUN_0040c210(slot, 3) != -1) {
        g_unk0x0058e0a8[2] = 1;
        return;
    }
    g_unk0x0058e0a8[2] = 0;
}

// Encodes a car's control record into a 4-byte replay packet.
// match 31%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046bdc0
void FUN_0046bdc0(BYTE *pIn, BYTE *pOut, int active, int handbrake, int lightA, int lightB)
{
    BYTE hb = (handbrake != 0 && active != 0) ? 1 : 0;
    BYTE a = (lightA != 0 && active != 0) ? 1 : 0;
    BYTE b = (lightB != 0 && active != 0) ? 1 : 0;
    BYTE byte3 = pOut[3];
    BYTE v;
    BYTE b0;

    v = (byte3 & 0x7f) | (hb << 7);
    pOut[3] = v;
    b0 = ((pOut[0] ^ a) & 1) ^ pOut[0];
    pOut[0] = b0;
    pOut[0] = (b << 1) | (b0 & 0xfd);
    if (pIn[0] == 0) {
        v = (byte3 & 0x3f) | (hb << 7);
        pOut[3] = v;
        byte3 = pIn[1];
    } else {
        v |= 0x40;
        pOut[3] = v;
        byte3 = pIn[0];
    }
    pOut[3] = ((byte3 ^ v) & 0x3f) ^ v;
    pOut[0] = (pIn[2] << 2) | (b << 1) | (b0 & 1);
    pOut[1] = (pIn[3] << 2) | (pOut[1] & 3);
    pOut[2] = (pIn[8] << 7) | (pOut[2] & 0x7f);
    pOut[1] = (((pIn[4] + 1) ^ pOut[1]) & 3) ^ pOut[1];
    v = ((pIn[0xc] & 1) << 6) | (pOut[2] & 0xbf);
    pOut[2] = (((v + 1) ^ v) & 0x3f) ^ v;
}

// Queued stage event draws (area, x, y, rows).
struct EventDraw {
    EventRec *pEvent;
    char x;
    char y;
    char rows;
    BYTE pad;
};
// GLOBAL: CMR2 0x00589010
EventDraw g_eventDraws[64];

// Stamps the pending event draws into one quadrant of the event texture and
// clears the pending list.
// match 37%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046e780
void FUN_0046e780(void)
{
    RECT rect;
    short *pPos;
    int i;
    int j;
    int k;
    int x, y;
    int n;
    int idx;
    BYTE c;
    BYTE colour[4];

    if (g_eventCount < 1)
        return;
    if (g_unk0x00589318 < 1)
        return;
    colour[0] = 0x2f;
    colour[1] = 0x27;
    colour[2] = 0x14;
    colour[3] = 0xc0;
    g_unk0x00589334 = g_unk0x00589334 + 1;
    if (g_unk0x00589334 > 3)
        g_unk0x00589334 = 0;
    switch (g_unk0x00589334) {
    case 0:
        rect.left = 0;
        rect.top = 0;
        rect.right = *(short *)((BYTE *)g_eventTexture + 0x120) / 2 - 1;
        rect.bottom = *(short *)((BYTE *)g_eventTexture + 0x122) / 2 - 1;
        break;
    case 1:
        rect.left = *(short *)((BYTE *)g_eventTexture + 0x120) / 2 - 1;
        rect.top = 0;
        rect.right = *(short *)((BYTE *)g_eventTexture + 0x120) - 1;
        rect.bottom = *(short *)((BYTE *)g_eventTexture + 0x122) / 2 - 1;
        break;
    case 2:
        rect.left = 0;
        rect.top = *(short *)((BYTE *)g_eventTexture + 0x122) / 2 - 1;
        rect.right = *(short *)((BYTE *)g_eventTexture + 0x120) / 2 - 1;
        rect.bottom = *(short *)((BYTE *)g_eventTexture + 0x122) - 1;
        break;
    default:
        rect.left = *(short *)((BYTE *)g_eventTexture + 0x120) / 2 - 1;
        rect.top = *(short *)((BYTE *)g_eventTexture + 0x122) / 2 - 1;
        rect.right = *(short *)((BYTE *)g_eventTexture + 0x120) - 1;
        rect.bottom = *(short *)((BYTE *)g_eventTexture + 0x122) - 1;
        break;
    }
    CGraphics::LockTexture(g_eventTexture, &rect);
    i = 0;
    if (g_unk0x00589318 > 0) {
        do {
            j = 0;
            pPos = (short *)g_eventDraws[i].pEvent;
            x = (BYTE)g_eventDraws[i].x + pPos[0];
            y = (BYTE)g_eventDraws[i].y + pPos[1];
            if (CGameInfo::FUN_00405d10() == 0) {
                x = x * 4;
                y = y * 4;
            }
            idx = rand() % 4;
            n = CGameInfo::FUN_00405d10() ? 4 : 0x10;
            if (n > 0) {
                do {
                    k = 0;
                    do {
                        if (CGameInfo::FUN_00405d10() == 0)
                            c = g_unk0x0051c280[(idx * 0x10 + j) * 0x10 + k];
                        else
                            c = g_unk0x0051c240[(j + idx * 4) * 4 + k];
                        if (c != 0) {
                            if (rect.left < x + j && x + j < rect.right && rect.top < y + k &&
                                y + k < rect.bottom)
                                CGraphics::BlendPixel(g_eventTexture, x + j - rect.left,
                                                      y + k - rect.top, colour);
                        }
                        k++;
                    } while (k < n);
                    j++;
                } while (j < n);
            }
            i = i + 1;
            g_eventsDirty = 1;
        } while ((short)i < g_unk0x00589318);
    }
    CGraphics::UnlockTexture(g_eventTexture);
    g_unk0x00589318 = 0;
}

int Events_Tick(int index);

// Queues a draw of event `index` at (x, y) when it ticks and is on the area;
// pauses it once it has been drawn `range` times.
// match 30%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046ec40
void FUN_0046ec40(int index, int x, int y)
{
    EventRec *p = &g_eventRecords[index];
    int over;

    if (index < g_eventCount && p->active != 0 && Events_Tick(index) && x >= 0 && y >= 0 &&
        x < p->a * 4 - 4 && y < p->b && g_unk0x00589318 < 0x40 && p != NULL) {
        g_eventDraws[g_unk0x00589318].pEvent = p;
        g_eventDraws[g_unk0x00589318].x = (char)x;
        over = y - p->b + 4;
        g_eventDraws[g_unk0x00589318].y = (char)y;
        if (over < 1)
            g_eventDraws[g_unk0x00589318].rows = 4;
        else
            g_eventDraws[g_unk0x00589318].rows = 4 - (char)over;
        if ((unsigned short)p->range <= (unsigned short)p->field_0x16) {
            p->paused = 1;
            g_unk0x00589318++;
            return;
        }
        g_unk0x00589318++;
        p->field_0x16++;
    }
}

// Sets the screen rectangles of the views from the screen size (full, top,
// bottom, left and right halves).
// match 20%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00464b60
void FUN_00464b60(void)
{
    short *pFull = (short *)g_unk0x00548110[0];
    short *p = (short *)g_unk0x0051b9f0;
    int w = *(int *)g_pGraphics;
    int h = *(int *)((BYTE *)g_pGraphics + 4);

    pFull[0] = 0;
    pFull[1] = 0;
    pFull[2] = (short)w;
    pFull[3] = (short)h;
    p[0] = 0;
    p[1] = 0;
    p[2] = (short)w;
    p[3] = (short)h;
    p[4] = 0;
    p[5] = 0;
    p[6] = (short)w;
    p[8] = 0;
    p[7] = (short)(h / 2);
    p[9] = (short)(h / 2);
    p[10] = (short)w;
    p[12] = 0;
    p[11] = (short)(h / 2);
    p[13] = 0;
    p[14] = (short)(w / 2);
    p[15] = (short)h;
    p[17] = 0;
    p[16] = (short)(w / 2);
    p[18] = (short)(w / 2);
    p[19] = (short)h;
}

#define FIXVEC_EQ(a, b) ((a).x == (b).x && (a).y == (b).y && (a).z == (b).z)

// Interpolates every stage object's matrix between its two keys and flags
// the ones that moved.
// match 68%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00471950
void FUN_00471950(int t)
{
    int i;
    BYTE *p;
    FixMatrix old;
    FixMatrix *pCurrent;

    for (i = 0; i < g_unk0x0058c924; i++) {
        p = (BYTE *)&g_unk0x005894e0[i];
        pCurrent = (FixMatrix *)(p + 0x88);
        old = *pCurrent;
        FixMatrix_Interpolate(pCurrent, (FixMatrix *)(p + 0x48), (FixMatrix *)(p + 8), t, t, t, 1);
        if (FIXVEC_EQ(old.right, pCurrent->right) && FIXVEC_EQ(old.up, pCurrent->up) &&
            FIXVEC_EQ(old.forward, pCurrent->forward) && FIXVEC_EQ(old.position, pCurrent->position))
            *(int *)(p + 0x124) = 0;
        else
            *(int *)(p + 0x124) = 1;
    }
}

int FUN_004b5320(void *pNode, int value);

// Copies each stage object's interpolated matrix onto its scene node and
// calls the node refresh when the key changed.
// FUNCTION: CMR2 0x00471a60
void FUN_00471a60(int param_1)
{
    int i;
    BYTE *p;

    i = 0;
    if (g_unk0x0058c924 != 0) {
        p = (BYTE *)g_unk0x005894e0;
        do {
            *(FixMatrix *)(*(int *)(p + 4) + 0x98) = *(FixMatrix *)(p + 0x88);
            if (*(int *)(p + 0x124) != 0) {
                *(FixMatrix *)(*(int *)(p + 4) + 0xd8) = *(FixMatrix *)(p + 0x88);
                *(int *)(p + 0x114) = FUN_004b5320((void *)*(int *)(p + 4), *(int *)(p + 0x114));
                *(int *)(p + 0x124) = 0;
            }
            i = i + 1;
            p = p + 0x128;
        } while (i < (int)(g_unk0x0058c924 & 0xff));
    }
}

char FUN_00420190(void);
void FUN_0043f570(Car *pCar);

// Seeds the stage's random numbers (unless replaying) and gives the computer
// cars their start revs by difficulty.
// FUNCTION: CMR2 0x0047c1e0
void FUN_0047c1e0(char replay, char restart)
{
    int i;
    int r;
    int v;
    int start;
    Car *pCar;

    if (replay == 0 || restart != 0)
        g_unk0x0058e26c = CMain::GetFrameTime();
    if (CGameInfo::FUN_00406320())
        g_unk0x0058e26c = 0;
    srand(g_unk0x0058e26c);
    rand();
    for (i = 0; i < (BYTE)FUN_00420190(); i++) {
        r = rand();
        switch (CGameInfo::FUN_00405d90()) {
        case 0:
            v = r * 2;
            break;
        case 1:
            v = r + 0x8000;
            break;
        case 2:
            if (i >= 3)
                v = r + 0x8000;
            else
                v = 0x10000;
            break;
        default:
            v = 0;
        }
        if (r <= 0x4ccc)
            start = 0x10000 - v % 10;
        else
            start = 0;
        if (i >= (int)(RallyDataState() & 0xff)) {
            pCar = Car_Get(i);
            pCar->field_0x7a4 = FixMul(start, pCar->field_0x794);
            if (replay != 0 && restart == 0)
                FUN_0043f570(pCar);
        }
    }
}

void FUN_004658e0(int index);

// Refreshes the skid trails of every car in the race order, skipping the
// replay-style modes where CGameInfo::FUN_00405cd0() returns 2.
// FUNCTION: CMR2 0x00465780
void FUN_00465780(int param_1)
{
    short s;
    int i;

    if (CGameInfo::FUN_00405cd0() != 2) {
        i = 0;
        s = Car_GetOrderCount();
        if (0 < s) {
            do {
                if (i < 8 && FUN_0046bd20(i, param_1) != 7)
                    FUN_004658e0(i);
                i = i + 1;
                s = Car_GetOrderCount();
            } while (i < s);
        }
    }
}

BYTE *FUN_00498570(int index);

// Exhaust points of each car (4 per car).
// GLOBAL: CMR2 0x00549c20
FixVector g_unk0x00549c20[8][4];

// Rebuilds a car's four exhaust points halfway between its body path points.
// match 9%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004657d0
void FUN_004657d0(int car)
{
    int off;
    int *pOut;
    int *pA;
    int *pB;
    FixVector d;

    if (car < 8 && FUN_00498570(car) != NULL) {
        pOut = &g_unk0x00549c20[car][0].y;
        for (off = 0x1c8; off < 0x1f8; off += 0xc, pOut += 3) {
            pA = (int *)(FUN_00498570(car) - 0x30 + off);
            pB = (int *)(FUN_00498570(car) + off);
            d.x = pA[0] - pB[0];
            d.y = pA[1] - pB[1];
            d.z = pA[2] - pB[2];
            d.x = FixMul(d.x, 0x8000);
            d.y = FixMul(d.y, 0x8000);
            d.z = FixMul(d.z, 0x8000);
            d.x += pB[0];
            d.y += pB[1];
            d.z += pB[2];
            pOut[-1] = d.x;
            pOut[0] = d.y;
            pOut[1] = d.z;
            pOut[0] += 0xccc;
        }
    }
}

BYTE FUN_0042b710(int index);
void FUN_0046bdc0(BYTE *pIn, BYTE *pOut, int active, int handbrake, int lightA, int lightB);

// Encodes a car's controls into a replay packet.
// FUNCTION: CMR2 0x0046c450
void FUN_0046c450(BYTE *pOut, BYTE car)
{
    Car *pCar = Car_Get(car);
    DeviceInfo *pDev = CInput::FUN_0049ead0((char)FUN_0042b710(pCar->field_0xb1a));

    FUN_0046bdc0((BYTE *)pCar + 0x1d0, pOut, pDev->field_0x0 == 3, *(int *)((BYTE *)pCar + 0xb88),
                 *(int *)((BYTE *)pCar + 0xb8c), *(int *)((BYTE *)pCar + 0xb90));
}

int FUN_0041f3a0(void);

// Screen rectangle of a view: full screen with one player, else the half
// for the split direction.
// FUNCTION: CMR2 0x00464b10
BYTE *FUN_00464b10(int view)
{
    FUN_00464b60();
    if ((BYTE)RallyDataState() != 1 && FUN_0041f3a0() == 0) {
        if (CGameInfo::FUN_00405dc0())
            return (BYTE *)&g_unk0x0051b9f0[1 + view];
        return (BYTE *)&g_unk0x0051b9f0[3 + view];
    }
    return (BYTE *)g_unk0x0051b9f0;
}

// Starts a fresh stage object session.
// FUNCTION: CMR2 0x0047bda0
void FUN_0047bda0(void)
{
    FUN_0047c5c0();
    FUN_0047c1b0();
    FUN_0047c1e0(0, 0);
    FUN_0047cc30();
}

// Starts replay mode with the selected restart flag.
// FUNCTION: CMR2 0x0047bdc0
void FUN_0047bdc0(char restart)
{
    FUN_0047c1e0(1, restart);
}

// Checks whether a replay packet agrees with current controls.
// match 57%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046cbe0
int FUN_0046cbe0(BYTE *packet, BYTE car)
{
    BYTE current[4];

    if ((packet[2] & 0x3f) == 0)
        return 1;
    if ((packet[2] & 0x3f) >= 0x3f)
        return 0;
    FUN_0046c450(current, car);
    return ((packet[0] ^ current[0]) & 0xfc) == 0 &&
           ((packet[1] ^ current[1]) & 0xfc) == 0 &&
           ((packet[3] ^ current[3]) & 0x7f) == 0 &&
           ((packet[1] ^ current[1]) & 3) == 0 &&
           ((packet[2] ^ current[2]) & 0xc0) == 0 &&
           ((packet[0] ^ current[0]) & 3) == 0 &&
           ((packet[3] ^ current[3]) & 0x80) == 0;
}

// Interpolates a stage object record between two frames.
// match 63%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00461710
void FUN_00461710(BYTE *out, BYTE *from, BYTE *to, int t)
{
    int i;

    for (i = 0x1c; i < 0x48; i += 4) {
        out[i] = (BYTE)FUN_004616c0(to[i], from[i], t);
        out[i + 1] = (BYTE)FUN_004616c0(to[i + 1], from[i + 1], t);
        out[i + 2] = (BYTE)FUN_004616c0(to[i + 2], from[i + 2], t);
        out[i + 3] = (BYTE)FUN_004616c0(to[i + 3], from[i + 3], t);
    }
    for (i = 0; i < 7; i++)
        ((int *)out)[i] = ((int *)from)[i] + FixMul(((int *)to)[i] - ((int *)from)[i], t);
    for (i = 0x48; i < 0x4e; i++)
        out[i] = (BYTE)FUN_004616c0(to[i], from[i], t);
}

#include <stdlib.h>

struct Unk0x004a3e20;
void FUN_004a3e20(Unk0x004a3e20 *pObject, int value);

// Finds the rev counter and digit textures for a player.
// match 73%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00477340
void FUN_00477340(int player)
{
    char base[260];
    unsigned int i;
    Texture *texture;

    for (i = 0; i < CGraphics::m_textureCount; i++) {
        texture = CGraphics::m_pTextureManager->textureBuffer[i];
        _splitpath(texture->name, CFrontend::m_stringDest, CFrontend::m_stringDest,
                   base, CFrontend::m_stringDest);
        sprintf(CFrontend::m_stringDest, (char *)CGenericFileLoader::StrUpperPolish((BYTE *)base));
        if (strcmp(CFrontend::m_stringDest, CGraphics::m_strSuffixREVCT) == 0) {
            *(Texture **)(g_stageBlock + 0x220 + player * 8) = texture;
            FUN_004a3e20((Unk0x004a3e20 *)texture, 2);
        }
        if (strcmp(CFrontend::m_stringDest, CGraphics::m_strSuffixDIGIT) == 0)
            *(Texture **)(g_stageBlock + 0x224 + player * 8) = texture;
    }
}

BYTE FUN_004918c0(void);

// Starts stage objects and registers their frame callback.
// FUNCTION: CMR2 0x0048ca70
void FUN_0048ca70(void)
{
    FUN_0047bda0();
    CGame::RegisterCallback(FUN_004918c0, NULL);
}

// Allocates and registers a replay buffer.
// match 54%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046c5a0
BYTE *FUN_0046c5a0(short frames, short samples, int type)
{
    BYTE *buffer;
    int recordSize;
    int extraSize;
    BYTE slot;

    if (*(BYTE *)&g_unk0x00588ec8 == 0) {
        CGame::RegisterCallback(Replay_FreeBuffers, NULL);
        *(BYTE *)&g_unk0x00588ec8 = 1;
    }
    if (g_unk0x00588d3c == 8)
        return NULL;
    recordSize = type == 0 ? 0x114c : 0x5c;
    extraSize = type == 2 ? 0x10 : 4;
    buffer = (BYTE *)CFileBuffer::AllocateLockedBuffer(0x110 + frames * 2 + frames * recordSize + samples * frames * extraSize);
    if (buffer == NULL)
        return NULL;
    *(int *)(buffer + 0x1c) = type;
    *(short *)(buffer + 0xfc) = frames;
    *(short *)(buffer + 0xfe) = samples;
    Replay_SetupPointers(buffer, 0);
    *(int *)(buffer + 4) = 0;
    *(int *)(buffer + 0xc) = 0;
    *(int *)(buffer + 0x14) = 0;
    *(int *)(buffer + 0x18) = 0;
    *(short *)(buffer + 0x100) = 0;
    for (slot = 0; slot < 8; slot++) {
        if (g_unk0x00588e80[slot] == NULL) {
            g_unk0x00588e80[slot] = buffer;
            break;
        }
    }
    g_unk0x00588d3c++;
    return buffer;
}

// Loads a replay buffer and validates its recorded dimensions.
// match 38%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046d2d0
BYTE *FUN_0046d2d0(char *path)
{
    DWORD size;
    BYTE *buffer;
    int frames;
    int samples;

    if (*(BYTE *)&g_unk0x00588ec8 == 0) {
        CGame::RegisterCallback(Replay_FreeBuffers, NULL);
        *(BYTE *)&g_unk0x00588ec8 = 1;
    }
    buffer = (BYTE *)CFileBuffer::GetGenericFileBuffer(path, 1);
    if (buffer == NULL) {
        buffer = (BYTE *)CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(),
                                                      path, NULL, &size, 0);
        g_unk0x00588d18[g_unk0x00588d14] = 1;
    } else {
        size = CGenericFileLoader::GetGenericFileSize();
        g_unk0x00588d18[g_unk0x00588d14] = 0;
    }
    if (buffer == NULL)
        return NULL;
    frames = *(short *)(buffer + 0xfc);
    samples = *(short *)(buffer + 0xfe);
    if (size != 0x110 + frames * (0x114e + samples * 4) &&
        size != 0x110 + frames * (0x5e + samples * 4) &&
        size != 0x110 + frames * (0x114e + samples * 16) &&
        size != 0x110 + frames * (0x5e + samples * 16)) {
        CFileBuffer::FreeGenericFileBuffer(buffer);
        return NULL;
    }
    Replay_SetupPointers(buffer, 1);
    g_unk0x00588ea0[g_unk0x00588d14] = buffer;
    g_unk0x00588d14++;
    return buffer;
}

void FixMatrix_RotateAboutRight(FixMatrix *pOut, unsigned int angle);

// GLOBAL: CMR2 0x0051c9b0
short g_unk0x0051c9b0 = 0x71;

// Builds a stage object's world matrix from its car and mount point.
// match 41%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004778b0
void FUN_004778b0(BYTE *object, int unused)
{
    int car = object[2];
    FixVector position = *(FixVector *)(g_stageBlock + 0xe0 + car * 36);
    FixMatrix orient;
    FixMatrix mount;
    FixMatrix combined;
    BYTE *node;
    BYTE *pCar;

    FixMatrix_Identity(&mount);
    FixMatrix_SetPosition(&position, &mount);
    FixMatrix_Identity(&orient);
    orient.right.x = 0;
    orient.right.y = 0;
    orient.right.z = -0x10000;
    orient.up.x = 0;
    orient.up.y = 0x10000;
    orient.up.z = 0;
    orient.forward.x = 0x10000;
    orient.forward.y = 0;
    orient.forward.z = 0;
    FixMatrix_RotateAboutRight(&orient, (unsigned short)g_unk0x0051c9b0);
    FixMatrix_SetPosition((FixVector *)(g_stageBlock + 0xe0 + car * 36), &orient);
    node = *(BYTE **)(g_stageBlock + 0x294 + car * 0x1c);
    FixMatrix_Multiply(&combined, &orient, (FixMatrix *)(node + 0x98));
    pCar = (BYTE *)Car_Get(car);
    FixMatrix_Multiply((FixMatrix *)(object + 8), &combined, *(FixMatrix **)(pCar + 0x754));
    *(int *)(object + 0x48) = 0x10000;
    *(int *)(object + 0x4c) = 0x1999;
    *(int *)(object + 0x50) = 0;
    *(int *)(object + 0x54) = 0xa000;
    *(int *)(object + 0x58) = 0;
    *(int *)(object + 0x5c) = 0x10000;
}

GenericFile *FUN_0041f500(void);
BYTE *FUN_00475a40(void);
void StageUI_DrawChampionshipBar(void);
void FUN_0049d3f0(int, int, void *, int, int);
int FUN_004b9380(unsigned int, unsigned int, unsigned int);
int RallyData_FUN_0040eeb0(void);
int *FUN_0040f050(int view);
void FUN_00428680(unsigned int player, short *pRect, int check);
struct Menu;
void Menu_CallCallback2(Menu *pMenu);

// GLOBAL: CMR2 0x0051c950
char g_strTempObj[] = "TEMP.OBJ";
// GLOBAL: CMR2 0x0051c95c
char g_strTempSht[] = "TEMP.SHT";

// Replaces the active replay buffer with a freshly allocated one.
// FUNCTION: CMR2 0x00465f60
void FUN_00465f60(int frames, int samples)
{
    g_unk0x00588758 = (int *)FUN_0046c5a0(frames, samples, 0);
    FUN_00465f80();
}

// Loads a replay buffer from a path, falling back to a 10-second default one.
// FUNCTION: CMR2 0x00465f90
void FUN_00465f90(char *path)
{
    g_unk0x00588758 = (int *)FUN_0046d2d0(path);
    if (g_unk0x00588758 == NULL)
        FUN_00465f60(1, 0x1d4c);
    FUN_00465f80();
}

// Loads the stage's TEMP.OBJ model into memory.
// FUNCTION: CMR2 0x00472830
void FUN_00472830(void)
{
    void *pObj;
    GenericFile *pFile;

    pObj = CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(), g_strTempObj, NULL, NULL, 0);
    if (pObj != NULL) {
        pFile = FUN_0041f500();
        FUN_004b9380((unsigned int)pObj, RallyData_FUN_00411060(), (unsigned int)pFile);
    }
}

// Loads the stage's TEMP.SHT model into memory.
// FUNCTION: CMR2 0x00472870
void FUN_00472870(void)
{
    void *pObj;
    GenericFile *pFile;

    pObj = CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(), g_strTempSht, NULL, NULL, 0);
    if (pObj != NULL) {
        pFile = FUN_0041f500();
        FUN_004b9380((unsigned int)pObj, RallyData_FUN_00411060(), (unsigned int)pFile);
    }
}

// Clears the screen and redraws the split bars and championship positions.
// FUNCTION: CMR2 0x004759d0
void FUN_004759d0(int unused1, int unused2)
{
    int i;

    CGraphics::ClearTarget();
    CGraphics::ClearZBuffer();
    StageUI_DrawChampionshipBar();
    Menu_CallCallback2((Menu *)FUN_00475a40());
    i = 0;
    if ((BYTE)RallyDataState() != 0) {
        do {
            FUN_00428680(i, (short *)FUN_00464b10(i), 0);
            i++;
        } while (i < (int)(RallyDataState() & 0xff));
    }
    FUN_0049d3f0(RallyData_FUN_00411060(), RallyData_FUN_0040eeb0(), (void *)FUN_0040f050(0), 0, 0);
    FUN_0049de40();
}

struct ObjectMatrix16 { int v[16]; };

// Builds a stage object's orientation matrix from a car and a node, flipping
// the right-hand column and rotating about the object's right axis.
// FUNCTION: CMR2 0x00477850
void FUN_00477850(int object, int *src)
{
    int *dst = (int *)(object + 8);

    *(ObjectMatrix16 *)dst = *(ObjectMatrix16 *)src;
    dst[0] = -src[8];
    *(int *)(object + 0xc) = -src[9];
    *(int *)(object + 0x10) = -src[10];
    *(int *)(object + 0x28) = src[0];
    *(int *)(object + 0x2c) = src[1];
    *(int *)(object + 0x30) = src[2];
    FixMatrix_RotateAboutRight((FixMatrix *)dst, (unsigned short)g_unk0x0051c9b0);
    FUN_004778b0((BYTE *)object, (int)src);
}

// Builds a wheel/damper orientation matrix from two scale factors.
// match 76%: FixVector temp slot order differs from the original (same logic)
// match 75%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00486fc0
void FUN_00486fc0(int *pMatrix, int *pOffset)
{
    FixVector u, t;
    int i;

    FixVecScale(&t, (FixVector *)&pMatrix[4], pMatrix[0]);
    FixVecScale(&u, (FixVector *)&pMatrix[7], pMatrix[1]);
    pMatrix[0xc] = t.x - u.x;
    pMatrix[0xd] = t.y - u.y;
    pMatrix[0xe] = t.z - u.z;
    pMatrix[0xf] = u.x + t.x;
    pMatrix[0x10] = u.y + t.y;
    pMatrix[0x11] = u.z + t.z;
    FixVecScale(&t, &t, -0x10000);
    pMatrix[0x12] = t.x - u.x;
    pMatrix[0x13] = t.y - u.y;
    pMatrix[0x14] = t.z - u.z;
    pMatrix[0x15] = u.x + t.x;
    pMatrix[0x16] = u.y + t.y;
    pMatrix[0x17] = u.z + t.z;
    for (i = 0; i < 4; i++) {
        pMatrix[0xc + i * 3] += pOffset[0];
        pMatrix[0xd + i * 3] += pOffset[1];
        pMatrix[0xe + i * 3] += pOffset[2];
        pMatrix[0xd + i * 3] = pMatrix[2];
    }
}

// Builds a sprite orientation matrix from a pair of 16-bit extents.
// FUNCTION: CMR2 0x00487c40
void FUN_00487c40(int *pMatrix, int param_2, int *pOffset)
{
    short *p = *(short **)(*(int *)(param_2 + 4) + 4);
    FixVector t, u;
    int i;

    if (pMatrix[10] == 0) {
        pMatrix[0] = (int)p[4] << 9;
        pMatrix[1] = (int)p[5] << 9;
        pMatrix[4] = (int)p[0] << 9;
        pMatrix[5] = 0;
        pMatrix[6] = (int)p[1] << 9;
        pMatrix[7] = (int)p[1] << 9;
        pMatrix[8] = 0;
        pMatrix[9] = p[0] * -0x200;
        pMatrix[2] = p[2] * 0x200 + pOffset[1];
        pMatrix[3] = p[3] * 0x200 + pOffset[1];
        pMatrix[0x25] = (int)pOffset;
        pMatrix[0x24] = 0;
        FixVecScale(&t, (FixVector *)&pMatrix[4], pMatrix[0]);
        FixVecScale(&u, (FixVector *)&pMatrix[7], pMatrix[1]);
        pMatrix[0xc] = t.x - u.x;
        pMatrix[0xe] = t.z - u.z;
        pMatrix[0xf] = u.x + t.x;
        pMatrix[0x11] = u.z + t.z;
        FixVecScale(&t, &t, -0x10000);
        pMatrix[0x12] = t.x - u.x;
        pMatrix[0x14] = t.z - u.z;
        pMatrix[0x15] = u.x + t.x;
        pMatrix[0x17] = u.z + t.z;
        for (i = 0; i < 4; i++) {
            pMatrix[0xc + i * 3] += pOffset[0];
            pMatrix[0xd + i * 3] = pMatrix[3];
            pMatrix[0xe + i * 3] += pOffset[2];
        }
        pMatrix[10] = 1;
    }
}

// Blends two colours according to a fade timer and writes the result.
// match 75%: colour blend block differs in scheduling/register use (same logic)
// match 74%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0047f510
void FUN_0047f510(int param_1, BYTE *pOut, BYTE *pFrom, BYTE *pTo)
{
    int t = *(int *)(param_1 + 0x668);
    FixVector c;
    int v, r, g, b;

    if (t > 0xa0000) {
        if (*(int *)(param_1 + 0x930) != 0) {
            v = t - *(int *)(param_1 + 0x674);
            if (v > 0x50000) {
                *(DWORD *)pOut = *(DWORD *)pFrom;
                return;
            }
            if (v < -0x50000) {
                *(DWORD *)pOut = *(DWORD *)pTo;
                return;
            }
            v = FixMul(v, 0x1999) + 0x8000;
            c.x = (pFrom[0] << 16) - (pTo[0] << 16);
            c.y = (pFrom[1] << 16) - (pTo[1] << 16);
            c.z = (pFrom[2] << 16) - (pTo[2] << 16);
            FixVecScale(&c, &c, v);
            r = c.x + (pTo[0] << 16);
            g = c.y + (pTo[1] << 16);
            b = c.z + (pTo[2] << 16);
            if (r > 0xff0000)
                r = 0xff0000;
            if (g > 0xff0000)
                g = 0xff0000;
            if (b > 0xff0000)
                b = 0xff0000;
            pOut[0] = (BYTE)(r >> 16);
            pOut[1] = (BYTE)(g >> 16);
            pOut[2] = (BYTE)(b >> 16);
            pOut[3] = 0xff;
            return;
        }
        *(DWORD *)pOut = *(DWORD *)pFrom;
        return;
    }
    v = FixMul(t, 0x1999);
    if (v > 0x10000)
        v = 0x10000;
    else if (v < 0)
        v = 0;
    if (*(int *)(param_1 + 0x930) != 0) {
        c.x = pTo[0] << 16;
        c.y = pTo[1] << 16;
        c.z = pTo[2] << 16;
    } else {
        c.x = pFrom[0] << 16;
        c.y = pFrom[1] << 16;
        c.z = pFrom[2] << 16;
    }
    pOut[0] = (BYTE)(FixMul(c.x, v) >> 16);
    pOut[1] = (BYTE)(FixMul(c.y, v) >> 16);
    pOut[2] = (BYTE)(FixMul(c.z, v) >> 16);
    pOut[3] = 0xff;
}

// Sets per-car visibility bits used by the stage object renderer.
// match 72%: pairs of flag bytes are not scheduled in parallel like the original (same logic)
// match 72%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046b790
void FUN_0046b790(int type, int car, int index)
{
    BYTE bit = 1 << car;

    switch (type) {
    case 0:
        g_unk0x00588ba4[8] |= bit;
        g_unk0x00588ba4[9] |= bit;
        g_unk0x00588ba4[10] |= bit;
        g_unk0x00588ba4[11] |= bit;
    case 3:
        g_unk0x00588ba4[12] |= bit;
        g_unk0x00588ba4[13] |= bit;
        g_unk0x00588ba4[14] |= bit;
        break;
    case 1:
        g_unk0x00588ba4[8] |= bit;
        g_unk0x00588ba4[9] |= bit;
        g_unk0x00588ba4[10] |= bit;
        g_unk0x00588ba4[13] |= bit;
        g_unk0x00588ba4[14] |= bit;
        break;
    case 2:
        g_unk0x00588ba4[8] |= bit;
        g_unk0x00588ba4[9] |= bit;
        g_unk0x00588ba4[10] |= bit;
        return;
    case 4:
        g_unk0x00588ba4[12] |= bit;
        return;
    case 5:
        g_unk0x00588ba4[8] |= bit;
        break;
    case 8:
        g_unk0x00588ba4[8] |= bit;
        g_unk0x00588ba4[11] |= bit;
        return;
    default:
        return;
    }
    g_unk0x00588ba4[index] |= bit;
}

int FUN_0040b010(int index);
unsigned int FUN_00409cb0(int index);
unsigned int FUN_0040b1e0(int index);

// Resets the per-view flags of a car's body, wheel and extra nodes.
// match 69%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046b8f0
void FUN_0046b8f0(Car *pCar)
{
    int node;

    *(int *)(g_unk0x00588ba4 + 8) = 0;
    *(short *)(g_unk0x00588ba4 + 12) = 0;
    g_unk0x00588ba4[14] = 0;
    g_unk0x00588ba4[pCar->field_0xb1a] = 0;
    FUN_0046b790(g_unk0x00588cd4[pCar->field_0xb1a * 2], 0, pCar->field_0xb1a);
    FUN_0046b790(g_unk0x00588cd4[pCar->field_0xb1a * 2 + 1], 1, pCar->field_0xb1a);
    pCar->pNode0x71c->field_0x17c = 0xff;
    SceneNode_SetViewMaskTree(pCar->pWheelNodes[0], g_unk0x00588ba4[9]);
    SceneNode_SetViewMaskTree(pCar->pWheelNodes[1], g_unk0x00588ba4[9]);
    SceneNode_SetViewMaskTree(pCar->pWheelNodes[2], g_unk0x00588ba4[9]);
    SceneNode_SetViewMaskTree(pCar->pWheelNodes[3], g_unk0x00588ba4[9]);
    SceneNode_SetViewMaskTree(pCar->pNode0x720, g_unk0x00588ba4[10]);
    node = (int)SceneNode_FindByType(pCar->pNode0x720, 6);
    if (node != 0)
        *(BYTE *)(node + 0x17c) = g_unk0x00588ba4[13];
    node = (int)SceneNode_FindByType(pCar->pNode0x720, 0xe);
    if (node != 0)
        *(BYTE *)(node + 0x17c) = g_unk0x00588ba4[14];
    if (pCar->pNode0x724 != NULL) {
        SceneNode_SetViewMaskTree(pCar->pNode0x724, g_unk0x00588ba4[12]);
        node = (int)SceneNode_FindByType(pCar->pNode0x724, 6);
        if (node != 0)
            *(BYTE *)(node + 0x17c) = g_unk0x00588ba4[13];
        node = (int)SceneNode_FindByType(pCar->pNode0x724, 0xe);
        if (node != 0)
            *(BYTE *)(node + 0x17c) = g_unk0x00588ba4[14];
    }
    if (pCar->pExtraNodes[0] != NULL)
        SceneNode_SetViewMaskTree(pCar->pExtraNodes[0], g_unk0x00588ba4[12]);
    if (pCar->pExtraNodes[1] != NULL)
        SceneNode_SetViewMaskTree(pCar->pExtraNodes[1], g_unk0x00588ba4[12]);
    if (pCar->pExtraNodes[2] != NULL)
        SceneNode_SetViewMaskTree(pCar->pExtraNodes[2], g_unk0x00588ba4[12]);
    if (pCar->pExtraNodes[3] != NULL)
        SceneNode_SetViewMaskTree(pCar->pExtraNodes[3], g_unk0x00588ba4[12]);
    if (*(int *)(g_stageBlock + 0x41c + pCar->field_0xb1a * 4) != 0)
        SceneNode_SetViewMaskTree(*(SceneNode **)(g_stageBlock + 0x41c + pCar->field_0xb1a * 4),
                                  g_unk0x00588ba4[11]);
    if (*(int *)(g_stageBlock + 0x2c0 + pCar->field_0xb1a * 4) != 0)
        *(BYTE *)(*(int *)(g_stageBlock + 0x2c0 + pCar->field_0xb1a * 4) + 0x17c) = g_unk0x00588ba4[8];
    if (*(int *)(g_stageBlock + 0x3fc + pCar->field_0xb1a * 4) != 0)
        *(BYTE *)(*(int *)(g_stageBlock + 0x3fc + pCar->field_0xb1a * 4) + 0x17c) = g_unk0x00588ba4[8];
    FUN_0046b6b0(pCar->pNode0x71c, 10);
    FUN_0046b6e0(pCar->pNode0x71c->pFirstChild, 10);
    FUN_0046b6b0(pCar->pNode0x720, 10);
    FUN_0046b6e0(pCar->pNode0x720->pFirstChild, 10);
}

// Rebuilds the per-wheel visibility values of the cars in race order.
// match 52%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046bb40
void FUN_0046bb40(void)
{
    int flags[2];
    short *pOrder;
    Car *pCar;
    int i;
    int v;
    int count;
    unsigned int swap;

    i = 0;
    do {
        switch (FUN_00422f50(i)) {
        case 1:
            flags[i] = 6;
            break;
        case 2:
            flags[i] = 5;
            break;
        case 3:
            flags[i] = 8;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            flags[i] = 1;
            break;
        default:
            flags[i] = 0;
        }
        i++;
    } while (i < 2);
    count = Car_GetOrderCount() - 1;
    if (-1 < (short)count) {
        pOrder = Car_GetOrder() + (short)count;
        count = (short)count + 1;
        do {
            pCar = Car_Get(*pOrder);
            swap = (*(unsigned int *)(*(int *)(*(int *)((BYTE *)pCar + 0x720) + 0xc) + 0x30) >>
                    0x12) & 1;
            i = 0;
            do {
                if ((FUN_00422fb0(i) & 0xff) == (unsigned int)pCar->field_0xb1a) {
                    v = flags[i];
                    if (v == 1) {
                        if (g_unk0x00588bb4[pCar->field_0xb1a] == 0) {
                            if (swap != 0)
                                v = 2;
                        } else if (swap == 0) {
                            v = 3;
                        } else {
                            v = 4;
                        }
                    }
                } else {
                    if ((BYTE)RallyDataState() <= 1 || StageObject_UsesExtendedMode() != 0) {
                        if (g_unk0x00588bb4[pCar->field_0xb1a] == 0) {
                            if (swap == 0)
                                v = 1;
                            else
                                v = 2;
                        } else if (swap == 0) {
                            v = 3;
                        } else {
                            v = 4;
                        }
                    } else {
                        v = 7;
                    }
                }
                FUN_0046b740(pCar->field_0xb1a, v, i);
                i++;
            } while (i < 2);
            pOrder--;
            count--;
        } while (count != 0);
    }
    if ((char)CGameInfo::FUN_00405e00() != 0) {
        for (i = 0; i < 7; i++) {
            if ((char)FUN_00409cb0(i) == 0 && (char)FUN_0040b1e0(i) != 0) {
                FUN_0046b740(FUN_0040b010(i), 7, 0);
                FUN_0046b740(FUN_0040b010(i), 7, 1);
            }
        }
    }
}

extern Car *g_collisionCar;

// Applies the fade-driven roll (about the node's current up axis) to the two
// scene nodes of car `index`'s stage object and advances its fade state machine.
// FUNCTION: CMR2 0x00476a40
void FUN_00476a40(int index)
{
    int off = index * 0x1c;
    SceneNode *pNode;
    FixVector axis;
    FixVector position;

    pNode = *(SceneNode **)(g_unk0x0058d530 + off + 0xc);
    if (pNode != NULL) {
        SceneNode_SetRotation(pNode, *(FixAngles **)(g_unk0x0058d4f0 + off));
        pNode = *(SceneNode **)(g_unk0x0058d530 + off + 0xc);
        axis = pNode->current.up;
        short angle = g_unk0x0058d4e0[index * 6];
        if (g_unk0x0058d2f0[index] != 0)
            angle = -angle;
        FixMatrix_FromAxisAngle(&g_unk0x0058d260, &axis, angle);
        pNode = *(SceneNode **)(g_unk0x0058d530 + off + 0xc);
        position = pNode->current.position;
        pNode->current.position.x = 0;
        pNode->current.position.y = 0;
        pNode->current.position.z = 0;
        FixMatrix_Multiply(&pNode->current, &pNode->current, &g_unk0x0058d260);
        pNode = *(SceneNode **)(g_unk0x0058d530 + off + 0xc);
        pNode->current.position = position;
    }
    pNode = *(SceneNode **)(g_unk0x0058d530 + off + 0x10);
    if (pNode != NULL) {
        SceneNode_SetRotation(pNode, *(FixAngles **)(g_unk0x0058d4f0 + off + 0x14));
        pNode = *(SceneNode **)(g_unk0x0058d530 + off + 0x10);
        axis = pNode->current.up;
        short *pNum = *(short **)(g_unk0x0058d4f0 + off + 0x18);
        short *pDen = *(short **)(g_unk0x0058d4f0 + off + 4);
        int angle;
        if (g_unk0x0058d2f0[index] != 0)
            angle = -(*pNum * g_unk0x0058d4e0[index * 6] / *pDen);
        else
            angle = *pNum * g_unk0x0058d4e0[index * 6] / *pDen;
        FixMatrix_FromAxisAngle(&g_unk0x0058d260, &axis, angle);
        pNode = *(SceneNode **)(g_unk0x0058d530 + off + 0x10);
        position = pNode->current.position;
        pNode->current.position.x = 0;
        pNode->current.position.y = 0;
        pNode->current.position.z = 0;
        FixMatrix_Multiply(&pNode->current, &pNode->current, &g_unk0x0058d260);
        pNode = *(SceneNode **)(g_unk0x0058d530 + off + 0x10);
        pNode->current.position = position;
    }
    FUN_00476c70(index);
}

// Steps the fade state machine of one stage object: 0 -> 2 -> 3 ramps the
// offset up to the record's limit and 1 ramps it back to zero, retriggering
// from the object's +0x54 field.
// FUNCTION: CMR2 0x00476c70
void FUN_00476c70(int index)
{
    short *pMax;

    if (FUN_00460bf0(index) == 1) {
        if (FUN_00460c10(index) > 0x3333 && g_unk0x0058d4d8[index * 3] == 0) {
            g_unk0x0058d4d8[index * 3] = 2;
            g_unk0x0058d4e0[index * 6 + 1] = rand() % 0xb + 0x2d;
        }
        if (FUN_00460c10(index) > 0x8000 && g_unk0x0058d4d8[index * 3] == 2) {
            g_unk0x0058d4d8[index * 3] = 3;
            g_unk0x0058d4e0[index * 6 + 1] = rand() % 0xb + 0x5b;
        }
        if (FUN_00460c10(index) < 0x1999 && g_unk0x0058d4d8[index * 3] == 2)
            g_unk0x0058d4d8[index * 3] = 1;
        if (FUN_00460c10(index) < 0x6666 && g_unk0x0058d4d8[index * 3] == 3) {
            g_unk0x0058d4d8[index * 3] = 2;
            g_unk0x0058d4e0[index * 6 + 1] = rand() % 0xb + 0x2d;
        }
    } else {
        g_unk0x0058d4d8[index * 3] = 0;
    }
    if (g_unk0x0058d4d8[index * 3] != 0) {
        if (g_unk0x0058d4d8[index * 3 + 1] != 0) {
            g_unk0x0058d4e0[index * 6] += g_unk0x0058d4e0[index * 6 + 1];
            pMax = *(short **)(g_unk0x0058d4f0 + index * 0x1c + 4);
            if (g_unk0x0058d4e0[index * 6] > *pMax) {
                g_unk0x0058d4d8[index * 3 + 1] = 0;
                g_unk0x0058d4e0[index * 6] = *pMax;
                return;
            }
        } else {
            g_unk0x0058d4e0[index * 6] -= g_unk0x0058d4e0[index * 6 + 1];
            if (g_unk0x0058d4e0[index * 6] < 0) {
                g_unk0x0058d4d8[index * 3 + 1] = 1;
                if (g_unk0x0058d4d8[index * 3] == 1)
                    g_unk0x0058d4d8[index * 3] = 0;
                g_unk0x0058d4e0[index * 6] = 0;
            }
        }
    }
}

// Updates per-wheel slip tables and damps the car's velocity.
// match 42%: per-wheel tables and FixMul block differ from the original
// match 42%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0048df50
void FUN_0048df50(Car *param_1)
{
    int i;
    int off;
    int v;

    g_collisionCar = param_1;
    i = 0;
    off = 0xbbc;
    do {
        int a, b, d, aa, bb, u;

        v = *(int *)(g_collisionCar + 0x778);
        if (v > 0x10000)
            v = 0x10000;
        v = FixMul(v, 0x6666);
        a = *(int *)(g_collisionCar + i + 0x270);
        b = *(int *)(g_collisionCar + i + 0x278);
        aa = a < 0 ? -a : a;
        bb = b < 0 ? -b : b;
        if (aa - bb < 0)
            d = bb - aa;
        else
            d = aa - bb;
        u = FixMul((d % 0x401) << 6, v);
        if (*(int *)(g_collisionCar + off) == 0 ||
            *(int *)(g_collisionCar + off - 0x2c0) <= u) {
            *(int *)(g_collisionCar + off - 0x2c0) = u;
            *(int *)(g_collisionCar + off) = 1;
            *(int *)(g_collisionCar + i + 0x564) = 0;
            *(int *)(g_collisionCar + i + 0x568) = 0x10000;
            *(int *)(g_collisionCar + i + 0x56c) = 0;
        }
        off += 4;
        i += 0xc;
    } while (off < 0xbcc);
    FixVecScale((FixVector *)(g_collisionCar + 0x408), (FixVector *)(g_collisionCar + 0x408),
                0xf851);
}

// Builds a 3x4 matrix from three basis vectors plus a translation.
// match 43%: matrix combination ordering differs from the original
// match 43%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00487140
void FUN_00487140(int *param_1, int *param_2, int *param_3, int *param_4)
{
    int x = param_4[0];
    int y = param_4[1];
    int z = param_4[2];
    int a, b, c;
    int v7, v2, v8, v6;

    a = FixMul(param_3[0], x);
    b = FixMul(param_3[4], y);
    c = FixMul(param_3[8], z);
    param_1[0xc] = c + b + a;
    param_1[0xf] = (b - c) + a;
    param_1[0x15] = (b - c) - a;
    param_1[0x12] = (c - a) + b;
    a = FixMul(param_3[1], x);
    b = FixMul(param_3[5], y);
    c = FixMul(param_3[9], z);
    param_1[0xd] = c + b + a;
    param_1[0x10] = (b - c) + a;
    param_1[0x16] = (b - c) - a;
    param_1[0x13] = (c - a) + b;
    a = FixMul(param_3[2], x);
    b = FixMul(param_3[6], y);
    c = FixMul(param_3[10], z);
    v7 = c + b + a;
    param_1[0xe] = v7;
    v2 = (b - c) + a;
    v8 = (b - c) - a;
    v6 = (c - a) + b;
    param_1[0x17] = v8;
    param_1[0x14] = v6;
    param_1[3] = -param_1[0x12];
    param_1[4] = -param_1[0x13];
    param_1[5] = -v6;
    param_1[6] = -param_1[0xf];
    param_1[7] = -param_1[0x10];
    param_1[8] = -v2;
    param_1[9] = -param_1[0xc];
    param_1[10] = -param_1[0xd];
    param_1[0x11] = v2;
    param_1[0xb] = -v7;
    v8 = -v8;
    param_1[0] = -param_1[0x15];
    param_1[1] = -param_1[0x16];
    param_1[2] = v8;
    param_1[0] = -param_1[0x15] + param_2[0];
    param_1[1] = -param_1[0x16] + param_2[1];
    param_1[2] = v8 + param_2[2];
    param_1[3] = param_1[3] + param_2[0];
    param_1[4] = param_1[4] + param_2[1];
    param_1[5] = param_1[5] + param_2[2];
    param_1[6] = param_1[6] + param_2[0];
    param_1[7] = param_1[7] + param_2[1];
    param_1[8] = param_1[8] + param_2[2];
    param_1[9] = param_1[9] + param_2[0];
    param_1[10] = param_1[10] + param_2[1];
    param_1[0xb] = param_1[0xb] + param_2[2];
    param_1[0xc] = param_1[0xc] + param_2[0];
    param_1[0xd] = param_1[0xd] + param_2[1];
    param_1[0xe] = param_1[0xe] + param_2[2];
    param_1[0xf] = param_1[0xf] + param_2[0];
    param_1[0x10] = param_1[0x10] + param_2[1];
    param_1[0x11] = param_1[0x11] + param_2[2];
    param_1[0x12] = param_1[0x12] + param_2[0];
    param_1[0x13] = param_1[0x13] + param_2[1];
    param_1[0x14] = param_1[0x14] + param_2[2];
    param_1[0x15] = param_1[0x15] + param_2[0];
    param_1[0x16] = param_1[0x16] + param_2[1];
    param_1[0x17] = param_1[0x17] + param_2[2];
}

int Car_GetWheelSpeed(Car *pCar, BYTE wheel, int unit);
void FUN_00498ca0(char *pDesc, int *pValues, int *pOut);
void FUN_0046ed80(int param_1, int param_2, int param_3, int param_4, int param_5, int param_6,
                  int param_7, int param_8);

// GLOBAL: CMR2 0x00588edc
short g_unk0x00588edc[28];
// GLOBAL: CMR2 0x00588f16
short g_unk0x00588f16[125];

// Spawns a dust/smoke puff at a randomised position relative to a wheel.
// match 48%: randomised offset evaluation order differs from the original
// match 47%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046ed80
void FUN_0046ed80(int param_1, int param_2, int param_3, int param_4, int param_5, int param_6,
                  int param_7, int param_8)
{
    short s;
    int x1, x2, a, b, c;

    s = (short)(rand() % (param_8 / 2));
    rand();
    param_4 = (s * param_4) / (param_8 / 2);
    if (s <= 0x400) {
        unsigned int idx = (int)s & 0xfff;
        int conv = (int)(__int64)((double)param_5 * CGraphics::m_65536);
        int v = FixMul(conv, g_sinTable[idx]);
        if (v < 0)
            v = -v;
        param_5 = v >> 0x10;
    }
    a = rand() % (param_5 + 1);
    x2 = a * param_7 + param_3;
    b = (rand() % 0x11 - 8) / (rand() % 3 + 1);
    x1 = (param_4 - 0x10) * param_6 + param_2 + b;
    c = (rand() % 0x11 - 8) / (rand() % 3 + 1);
    FUN_0046ec40(param_1, x1, x2 + c);
}

// Emits skid/dust effects for the wheels that are slipping.
// match 44%: dust/skid table indexing differs from the original
// match 45%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0046ea80
void FUN_0046ea80(int param_1, int param_2)
{
    int i;

    i = 2;
    if (param_1 == 2 || param_1 == 3) {
        short *p;
        if (g_eventCount > 2) {
            p = g_unk0x00588f16;
            do {
                int r1 = rand();
                short s = p[-1];
                int r2 = rand();
                FUN_0046ec40(i, (r1 * s) / 0x7fff, (r2 * *p) / 0x7fff);
                i++;
                p += 0xe;
            } while (i < g_eventCount);
        }
    } else {
        int e = (param_1 != 0) ? 0xe : 0;
        int va = (short)g_unk0x00588edc[e];
        int vb = (short)g_unk0x00588edc[e + 1];
        int vd = va / 5;
        int vf = va - vd;
        int vg = vb - 8;
        int x, y;
        if (param_2 < 0x8000)
            param_2 = 0;
        else if (param_2 > 0x18000)
            param_2 = 0x18000;
        x = (param_2 * (vf - vd)) / 0xb333;
        y = (param_2 * vg) / 0xb333;
        if (x > 0x20) {
            if (param_1 == 0) {
                if (g_unk0x00589331 != 0)
                    FUN_0046ed80(0, vd, vb - 1, x, y, 1, -1, 0x4fa);
                else
                    FUN_0046ed80(0, vf - 1, 1, x, y, -1, 1, 0x4fa);
            } else if (g_unk0x00589331 != 0) {
                FUN_0046ed80(1, vd, 1, x, y, 1, 1, 0x4fa);
            } else {
                FUN_0046ed80(1, vf - 1, vb - 1, x, y, -1, -1, 0x4fa);
            }
        }
    }
    g_unk0x00589320[param_1] = g_unk0x00589320[param_1] + 1;
}

// Spawns skid effects for all four wheels of a car.
// FUNCTION: CMR2 0x0046ea10
void FUN_0046ea10(int param_1)
{
    int i;

    if (g_eventCount > 0) {
        Car_Get(param_1);
        i = 0;
        do {
            if (FUN_0046eeb0(param_1, i) != 0) {
                int v = Car_GetWheelSpeed(Car_Get(param_1), (BYTE)i, 0);
                if (v > 0x1e0000) {
                    int n = 2;
                    do {
                        FUN_0046ea80(i, v / 0x3c);
                        n--;
                    } while (n != 0);
                }
            }
            i++;
        } while (i < 4);
    }
}

// GLOBAL: CMR2 0x0058e088
int g_unk0x0058e088[6];
// Views into the stage object pointer table: 0x58e3ac and 0x58e44c are its
// 7th and 47th entries, 0x58e4a0 is the last one.
#define g_unk0x0058e3ac ((char **)(g_unk0x0058e394 + 6))
#define g_unk0x0058e44c ((char **)(g_unk0x0058e394 + 46))
#define g_unk0x0058e4a0 ((int *)g_unk0x0058e394[67])

// Projects a stage object's rotation table into its output rows.
// match 23%: nested projection loops do not match the original layout
// match 23%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0047ca30
void FUN_0047ca30(int param_1)
{
    char *pRow;
    int **ppData;
    int *pDst;
    int row;

    if (*(int *)(param_1 + 8) > 1) {
        pRow = (char *)(param_1 + 0xc);
        ppData = (int **)(param_1 + 0x14);
        pDst = (int *)g_unk0x0058e0b8;
        row = 0xc;
        do {
            int j = 1;
            if (pRow[1] > 0) {
                do {
                    int *pData;
                    int *pTable;
                    int sum;
                    int cols;
                    sum = 0;
                    pTable = pDst;
                    pData = *ppData;
                    cols = *pRow + 1;
                    if (cols > 0) {
                        do {
                            if (*pData != 0)
                                sum += FixMul(*pTable, *pData);
                            pTable++;
                            pData++;
                            cols--;
                        } while (cols != 0);
                    }
                    if (*(int *)(param_1 + 8) - 1 < (int)(pRow + (-0xb - param_1))) {
                        ((int *)g_unk0x0058e0b8)[row + j] = sum;
                    } else {
                        int idx = FixDiv(sum, 0x10000000);
                        if (idx < 0) {
                            if (-idx < 0x40)
                                ((int *)g_unk0x0058e0b8)[row + j] = -g_unk0x0058e4a0[-idx];
                            else
                                ((int *)g_unk0x0058e0b8)[row + j] = 0xffff0000;
                        } else if (idx < 0x40) {
                            ((int *)g_unk0x0058e0b8)[row + j] = g_unk0x0058e4a0[idx];
                        } else {
                            ((int *)g_unk0x0058e0b8)[row + j] = 0x10000;
                        }
                    }
                    j++;
                } while (j <= pRow[1]);
            }
            pDst += 0xc;
            row += 0xc;
            ppData++;
            pRow++;
        } while ((int)(pRow + (-0xb - param_1)) < *(int *)(param_1 + 8));
    }
}

// Builds a stage object's per-row output from its type tables.
// FUNCTION: CMR2 0x0047c9a0
void FUN_0047c9a0(int param_1, int param_2, int *param_3)
{
    unsigned int mask = 0;
    int i = 0;

    do {
        BYTE b = *(BYTE *)(g_unk0x0058e3ac[param_1] + 1 + i);
        int v = (int)(signed char)b;
        if (v != 0) {
            unsigned int bit = 1 << v;
            if ((mask & bit) == 0) {
                mask |= bit;
                FUN_00498ca0(g_unk0x0058e44c[v], param_3, (int *)g_unk0x0058e0b8);
                FUN_0047ca30((int)g_unk0x0058e44c[v]);
            }
            *(int *)(i + param_2) =
                *(int *)(&g_unk0x0058e088[(int)*(char *)(g_unk0x0058e3ac[param_1] + i) +
                                          *(int *)(g_unk0x0058e44c[v] + 8) * 0xc]);
        }
        i += 4;
    } while (i < 0x14);
}

int Track_GetGroundHeightSurface(FixVector *pPoint, FixVector *pNormal, short *pTri,
                                 short *pSurfaceClass, unsigned short *pSurface, int defaultY);

// Updates each wheel's suspension height against the ground.
// match 57%: short loop counter and clamp block differ from the original
// match 57%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004930e0
void FUN_004930e0(int param_1, int param_2)
{
    int *p;
    short i;
    short local_8;

    p = FUN_00469680((int)*(char *)(param_1 + 0xb1a));
    i = 0;
    local_8 = 0;
    if (param_2 > 0) {
        do {
            int v = Track_GetGroundHeightSurface(
                (FixVector *)(param_1 + (i * 3 + 0x9c) * 4),
                (FixVector *)(param_1 + (i * 3 + 0x129) * 4),
                (short *)(param_1 + 0xa9e + i * 2),
                (short *)(param_1 + 0xaae + i * 2),
                (unsigned short *)(param_1 + 0xac6 + i * 2),
                *(int *)(param_1 + 0x8dc + i * 4));
            *(int *)(param_1 + 0x8dc + i * 4) = v;
            if (*(short *)(param_1 + 0xa9e + i * 2) == -1)
                local_8 = local_8 + 1;
            if (i < 4) {
                int t = *(int *)(param_1 + 0x978 + i * 4) + *(int *)((int)p + 600 + i * 4);
                if (t > 0x10000)
                    t = 0x10000;
                *(int *)(param_1 + 0x8dc + i * 4) += FixMul(t, *(int *)(param_1 + 0x938 + i * 4));
            }
            if (*(int *)(param_1 + 0xbbc + i * 4) != 0) {
                int *src;
                *(int *)(param_1 + 0x8dc + i * 4) += *(int *)(param_1 + 0x8fc + i * 4);
                src = (int *)(param_1 + (i * 3 + 0x159) * 4);
                ((int *)(param_1 + (i * 3 + 0x129) * 4))[0] = src[0];
                ((int *)(param_1 + (i * 3 + 0x129) * 4))[1] = src[1];
                ((int *)(param_1 + (i * 3 + 0x129) * 4))[2] = src[2];
                *(short *)(param_1 + 0xaae + i * 2) = 0x2f;
            }
            if (*(short *)(param_1 + 0xaae + i * 2) == 0xf &&
                *(int *)(param_1 + 0xa7c) == 0 && *(int *)(param_1 + 0xbf8) == 0) {
                *(int *)(param_1 + 0xa7c) = 0x190000;
            }
            if (i < 4)
                *(int *)(param_1 + 0x8dc + i * 4) -= *(int *)(param_1 + 0x700 + i * 8);
            i++;
        } while ((int)i < param_2);
    }
    if ((short)param_2 < 8) {
        int n = 8 - (short)param_2;
        int *pDst = (int *)(param_1 + 0x8dc + (short)param_2 * 4);
        do {
            n--;
            *pDst = pDst[-4] - 0x50000;
            pDst++;
        } while (n != 0);
    }
    if (local_8 != param_2)
        *(int *)(param_1 + 0xa80) = 0;
    else
        *(int *)(param_1 + 0xa80) = *(int *)(param_1 + 0xa80) + 0x10000;
}

// Views into g_stageBlock for the object fade tables at 0x58d2d0/0x58d360/0x58d478.
#define g_unk0x0058d2d0 ((BYTE *)(g_stageBlock + 0x30))
#define g_unk0x0058d360 ((int *)(g_stageBlock + 0xc0))
#define g_unk0x0058d478 ((BYTE *)(g_stageBlock + 0x1d8))

// Fades a car's stage object in and out as its body state changes.
// match 83%: fade state machine branches differ from the original
// match 83%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00476850
int FUN_00476850(int param_1, int param_2)
{
    int result = 0;

    if (g_unk0x0058d360[param_1] == 1 &&
        ((unsigned int)g_unk0x0058d2d0[param_1] != (int)*(char *)(param_2 + 0xb20) ||
         *(int *)(param_2 + 0x1d8) != 0)) {
        g_unk0x0058d2d0[param_1] = *(char *)(param_2 + 0xb20);
        g_unk0x0058d360[param_1] = 2;
        g_unk0x0058d478[param_1] = 0;
    }
    if (g_unk0x0058d360[param_1] == 2) {
        int local_c = (int)(__int64)((double)(BYTE)g_unk0x0058d478[param_1] * CGraphics::m_65536);
        BYTE c;
        result = FixDiv(local_c, 0x70000);
        g_unk0x0058d2d0[param_1] = *(char *)(param_2 + 0xb20);
        c = g_unk0x0058d478[param_1];
        g_unk0x0058d478[param_1] = c + 1;
        if ((BYTE)(c + 1) > 7) {
            g_unk0x0058d360[param_1] = 3;
            g_unk0x0058d478[param_1] = 0;
        }
    }
    if (g_unk0x0058d360[param_1] == 3) {
        result = 0x10000;
        g_unk0x0058d478[param_1] = g_unk0x0058d478[param_1] + 1;
        if ((unsigned int)g_unk0x0058d2d0[param_1] != (int)*(char *)(param_2 + 0xb20) ||
            *(int *)(param_2 + 0x1d8) != 0) {
            g_unk0x0058d478[param_1] = 0;
            g_unk0x0058d2d0[param_1] = *(char *)(param_2 + 0xb20);
        }
        if ((BYTE)g_unk0x0058d478[param_1] > 3) {
            g_unk0x0058d360[param_1] = 4;
            g_unk0x0058d478[param_1] = 0;
        }
    }
    if (g_unk0x0058d360[param_1] == 4) {
        int local_c = (int)(__int64)((double)(BYTE)g_unk0x0058d478[param_1] * CGraphics::m_65536);
        result = 0x10000 - FixDiv(local_c, 0x70000);
        if ((unsigned int)g_unk0x0058d2d0[param_1] == (int)*(char *)(param_2 + 0xb20) &&
            *(int *)(param_2 + 0x1d8) == 0) {
            BYTE c = g_unk0x0058d478[param_1];
            g_unk0x0058d478[param_1] = c + 1;
            if ((BYTE)(c + 1) > 7) {
                g_unk0x0058d360[param_1] = 1;
                g_unk0x0058d478[param_1] = 0;
                return 0;
            }
        } else {
            BYTE c = g_unk0x0058d478[param_1];
            g_unk0x0058d2d0[param_1] = *(char *)(param_2 + 0xb20);
            g_unk0x0058d360[param_1] = 2;
            g_unk0x0058d478[param_1] = 7 - c;
        }
    }
    return result;
}

// ---------------------------------------------------------------------------
// TEMPORARY link scaffolding: callees that are not decompiled yet. Their
// signatures come from the original's `ret N`; the bodies are empty so the
// calls sites compile and reccmp can measure the callers.
// STUB: CMR2 0x004658e0
void FUN_004658e0(int index) { }

// STUB: CMR2 0x004b5320
int FUN_004b5320(void *pNode, int value) { return 0; }

// STUB: CMR2 0x00460330
void FUN_00460330(int a, int b) { }

// STUB: CMR2 0x00485690
void FUN_00485690(short *pOrder, short count, int view) { }

// STUB: CMR2 0x0047f740
void FUN_0047f740(void) { }

// STUB: CMR2 0x004920d0
void FUN_004920d0(DWORD *pColour, DWORD *pReference) { }

// STUB: CMR2 0x00492220
void FUN_00492220(DWORD *pColour, DWORD *pReference) { }

// STUB: CMR2 0x004926f0
void FUN_004926f0(int unused1, int unused2, int sunAngle) { }

// STUB: CMR2 0x00492bd0
void FUN_00492bd0(int view) { }

void FUN_0042b800(int, int, int);
void FUN_0045e610(void);
void FUN_004702a0(void);
extern int g_unk0x0067f228;

short Car_GetOrderCount(void);
SceneNode *SceneNode_FindByType(SceneNode *pNode, unsigned int type);
void FUN_00486910(BYTE *pObj, int *pSrc);
extern BYTE g_unk0x00590d8c[4];
extern BYTE g_unk0x00590ec0[16];

// Picks the scene node of a car's object payload by the payload type letter
// ('C' or 'A' select the 9-mode node, anything else the 5-mode one) and stores
// it in both per-object tables, then copies the payload into the object.
// FUNCTION: CMR2 0x00486740
void FUN_00486740(BYTE *pObj, int *pSrc, BYTE index, BYTE value)
{
    g_unk0x00590d8c[*pObj] = value;
    g_unk0x00590ec0[*pObj] = index;
    if ((short)index < Car_GetOrderCount() && Car_Get(index)->pNode0x720 != NULL) {
        if (FUN_00456be0(index)[0x20] == 'C' || FUN_00456be0(index)[0x20] == 'A')
            g_stageBlock_58d340[index] =
                (int)SceneNode_FindByType(Car_Get(index)->pNode0x720, 9);
        else
            g_stageBlock_58d340[index] =
                (int)SceneNode_FindByType(Car_Get(index)->pNode0x720, 5);
        g_stageBlock_58d47c[index] = (int)SceneNode_FindByType(Car_Get(index)->pNode0x720, 5);
    }
    FUN_00486910(pObj, pSrc);
}

// Sets the hit flag of one entry of a car's timing record and refreshes the
// derived record block.
// FUNCTION: CMR2 0x00469bf0
void FUN_00469bf0(Car *pCar, int index)
{
    *(int *)(g_unk0x00588b94 + 0x4b0 + (index + pCar->field_0xb1a * 0x134) * 4) = 1;
    FUN_00468c10(pCar);
}

// Saves the car's torque state into pState and copies the current race record
// block into the following slot of pState.
// FUNCTION: CMR2 0x0046c320
void FUN_0046c320(int *pState, BYTE car)
{
    Car *pCar = Car_Get(car);

    *pState = pCar->field_0x7a4;
    FUN_0042b800(car, 1, 1);
    pCar->field_0x7a4 = *pState;
    if (pCar->field_0xb48 != 1)
        FUN_0043f570(pCar);
    pState = pState + 1;
    pCar->field_0xb9c = 1;
    FUN_0046c220((Block6 *)RallyData_FUN_00421510(car), (Block6 *)pState);
    RallyData_FUN_004207a0(car);
}

// Restores the car's torque state from pState, copies pState's race record
// block back into the car record and revalidates the stage state.
// FUNCTION: CMR2 0x0046c390
void FUN_0046c390(int *pState, BYTE car)
{
    Car *pCar = Car_Get(car);

    FUN_0042b800(car, 1, 1);
    pCar->field_0x7a4 = *pState;
    if (pCar->field_0xb48 != 1)
        FUN_0043f570(pCar);
    pCar->field_0xb9c = 1;
    FUN_0046c220((Block6 *)(pState + 1), (Block6 *)RallyData_FUN_00421510(car));
    RallyData_FUN_004207a0(car);
    FUN_0045e610();
    FUN_004702a0();
    RallyData_ValidateIndex(car);
}

// GLOBAL: CMR2 0x0058ca74
int g_unk0x0058ca74;
// GLOBAL: CMR2 0x0058ca68
int g_unk0x0058ca68;
// GLOBAL: CMR2 0x0058c934
int g_unk0x0058c934;
// GLOBAL: CMR2 0x0058c950
int g_unk0x0058c950;

// Saves the scene-node and render-object counts around loading the two stage
// model variants (TEMP.OBJ and TEMP.SHT).
// FUNCTION: CMR2 0x00471af0
void FUN_00471af0(void)
{
    g_unk0x0058ca74 = g_sceneNodeCount;
    g_unk0x0058ca68 = g_unk0x0067f228;
    FUN_00472830();
    g_unk0x0058c934 = g_sceneNodeCount;
    g_unk0x0058c950 = g_unk0x0067f228;
    FUN_00472870();
    FUN_0046f060();
}

BYTE *FUN_0041b390(void);
BYTE FUN_0041b370(void);
int FUN_0041b380(void);
int FUN_004232a0(int index, int mode);
int RallyData_FUN_00408800(BYTE index);
void FUN_00421720(unsigned char, int, int, unsigned char, int);

// GLOBAL: CMR2 0x0051f4c0
unsigned int g_unk0x0051f4c0 = 0x100;
// GLOBAL: CMR2 0x0058df98
unsigned int g_unk0x0058df98;
// GLOBAL: CMR2 0x0058df9c
unsigned int g_unk0x0058df9c;
// GLOBAL: CMR2 0x0058e0a4
unsigned int g_unk0x0058e0a4;

// Driver-camera cycle: while the cycle key is held the active driver is
// advanced (or, on the championship round screen, picked from the round
// drivers) and the requested view mode is applied to the car.
// FUNCTION: CMR2 0x0047bad0
void FUN_0047bad0(unsigned int param_1, unsigned int param_2)
{
    int mode;

    if (*(char *)(*(int *)(FUN_0041b390() + 4) + param_2 * 8) == 7 ||
        *(char *)(*(int *)(FUN_0041b390() + 4) + param_2 * 8) == 8) {
        if ((param_1 & (g_unk0x0051f4c0 & 0xffff)) != 0) {
            if (FUN_00422f50(param_2) != 10)
                FUN_00421720(g_unk0x0058e0a0->field_0xb1a, 10, 0xffff,
                             FUN_00422fb0(g_unk0x0058e0a0->field_0xb1a), 0);
        }
        if ((param_1 & (g_unk0x0051f4c0 & 0xffff)) == 0) {
            if (FUN_00422f50(g_unk0x0058e0a0->field_0xb1a) == 10) {
                switch (FUN_0041b380()) {
                case 4:
                    g_unk0x0058e0a4 = (FUN_0041b370() & 0xff) + param_2;
                    break;
                case 0:
                case 1:
                    g_unk0x0058e0a4 = param_2;
                    break;
                case 2:
                    RallyData_GetRoundDrivers(&g_unk0x0058df98, &g_unk0x0058df9c);
                    if (RallyData_FUN_00408500(g_unk0x0058df98 & 0xff) == -1)
                        g_unk0x0058e0a4 = g_unk0x0058df98;
                    else
                        g_unk0x0058e0a4 = g_unk0x0058df9c;
                    break;
                case 3:
                    if (param_2 == 0)
                        RallyData_GetRoundDrivers(&g_unk0x0058e0a4, &g_unk0x0058df9c);
                    else
                        RallyData_GetRoundDrivers(&g_unk0x0058df98, &g_unk0x0058e0a4);
                    break;
                }
                CGameInfo::FUN_00405d70();
                mode = RallyData_FUN_00408800(g_unk0x0058e0a4 & 0xff);
                if (FUN_004232a0(g_unk0x0058e0a0->field_0xb1a, mode) != 0) {
                    FUN_00421720(g_unk0x0058e0a0->field_0xb1a,
                                 RallyData_FUN_00408800(g_unk0x0058e0a4 & 0xff), 0xffff,
                                 FUN_00422fb0(g_unk0x0058e0a0->field_0xb1a), 0);
                } else {
                    FUN_00421720(g_unk0x0058e0a0->field_0xb1a, 4, 0xffff,
                                 FUN_00422fb0(g_unk0x0058e0a0->field_0xb1a), 0);
                }
            }
        }
    }
}

extern int g_unk0x00547ad0;

// GLOBAL: CMR2 0x00547abc
int g_unk0x00547abc;

// Positions a lens flare of the given view node on screen: its brightness
// follows the sun visibility and the camera's pitch, its colour is scaled by
// the same factor and the sprite is queued on layer 2.
// match 61%: same structure, calls and constants; MSVC places the brightness
// temporaries in the unused parameter homes instead of the original's slots.
// Rebuilds the box axes of a stage object from the car/ground vectors and
// accumulates the extent of the eight corner points of the object.
// match 82%: MSVC6 keeps the constant zero in ESI in the original and in EDX here (register
// allocation only; logic, constants and block layout match)
// FUNCTION: CMR2 0x00486c30
void FUN_00486c30(int *pObj, int *param2, int *param3, FixVector *pVerts)
{
    FixVector *p;
    FixVector d;
    int len;
    int dot1;
    int dot2;
    int m;
    int i;

    if (pObj[10] != 0)
        return;

    pObj[3] = 0;
    pObj[2] = 0;
    pObj[1] = 0;
    pObj[0] = 0;
    m = param2[1];
    if (m < 0)
        m = -m;
    if (m > 0xfd70) {
        ((FixVector *)(pObj + 4))->x = param2[3];
        ((FixVector *)(pObj + 4))->y = param2[4];
        ((FixVector *)(pObj + 4))->z = param2[5];
        pObj[5] = 0;
        len = FixVecLength((FixVector *)(pObj + 4));
        if (len == 0) {
            pObj[4] = 0;
            pObj[5] = 0;
            pObj[6] = 0;
        } else {
            FixVecScaleRecip((FixVector *)(pObj + 4), (FixVector *)(pObj + 4), len);
        }
    } else {
        ((FixVector *)(pObj + 4))->x = param2[0];
        ((FixVector *)(pObj + 4))->y = param2[1];
        ((FixVector *)(pObj + 4))->z = param2[2];
        pObj[5] = 0;
        len = FixVecLength((FixVector *)(pObj + 4));
        if (len == 0) {
            pObj[4] = 0;
            pObj[5] = 0;
            pObj[6] = 0;
        } else {
            FixVecScaleRecip((FixVector *)(pObj + 4), (FixVector *)(pObj + 4), len);
        }
    }
    pObj[8] = 0;
    pObj[7] = pObj[6];
    pObj[9] = -pObj[4];

    p = pVerts;
    i = 8;
    do {
        d.x = p->x - param3[0];
        d.y = p->y - param3[1];
        d.z = p->z - param3[2];
        dot1 = FixVecDot(&d, (FixVector *)(pObj + 4));
        dot2 = FixVecDot(&d, (FixVector *)(pObj + 7));
        if (dot1 > 0 && dot1 > pObj[0])
            pObj[0] = dot1;
        if (dot2 > 0 && dot2 > pObj[1])
            pObj[1] = dot2;
        if (d.y > pObj[2])
            pObj[2] = d.y;
        if (d.y < pObj[3])
            pObj[3] = d.y;
        p++;
    } while (--i);

    pObj[2] += param3[1];
    pObj[3] += param3[1];
    pObj[0x24] = (int)pVerts;
    pObj[0x25] = (int)param3;
    pObj[10] = 1;
    FUN_00486fc0(pObj, param3);
}

// Text colour and layer used by the knockout screen header.
// GLOBAL: CMR2 0x0051c980
BYTE g_unk0x0051c980 = 3;
// GLOBAL: CMR2 0x0051c9a4
BYTE g_unk0x0051c9a4[4] = { 0x61, 0x61, 0x7d, 0xff };
extern BYTE g_barTextColour[4];

// Draws the two header lines of a knockout match: interpolates the panel
// rectangle, then prints both driver names with the shared bar colours.
// match 97%: the original reads the header layer byte straight into AL (its Font_DrawText
// takes a BYTE index); ours zero-extends it from the BYTE global
// FUNCTION: CMR2 0x00474fe0
void FUN_00474fe0(int param1, int param2, int param3, int param4, int param5, KnockoutMatch *param6)
{
    short rect[4];
    int x;
    int y;

    x = (param3 - param1) * g_unk0x0058cc74 / 0x10000 + param1;
    rect[0] = (short)x;
    y = (param4 - param2) * g_unk0x0058cc74 / 0x10000 + param2;
    rect[1] = (short)y;
    rect[2] = (short)((int)g_pGraphics->resX * 0x56 / 0x280);
    rect[3] = (short)((int)g_pGraphics->resY * 0x26 / 0x1e0);
    FUN_00475740(rect, g_unk0x0051c9a4, g_barTextColour, 0);
    if (CGameInfo::GetScreenWidth() >= 0x400 && CFrontend::FUN_004b7560(0x400) != 0 &&
        CFrontend::FUN_004b7590(0x400) != 0) {
        Font_DrawText(0, (char *)CGenericFileLoader::StrUpperPolish((BYTE *)FUN_004736b0(param6, param5)),
                      (int)g_pGraphics->resX * 0x54 / 0x280 + x,
                      (int)g_pGraphics->resY * 0x10 / 0x1e0 - (int)g_pGraphics->resY * 2 / 0x1e0 + y,
                      (int *)g_barTextColour, 0x14);
        Font_DrawText(g_unk0x0051c980,
                      (char *)CGenericFileLoader::StrUpperPolish((BYTE *)FUN_004752f0((int *)param6, param5, 0)),
                      (int)g_pGraphics->resX * 0x54 / 0x280 + x,
                      (int)g_pGraphics->resY * 0x1c / 0x1e0 + y + (int)g_pGraphics->resY * 7 / 0x1e0,
                      (int *)g_barTextColour, 0x14);
        return;
    }
    Font_DrawText(0, (char *)CGenericFileLoader::StrUpperPolish((BYTE *)FUN_004736b0(param6, param5)),
                  (int)g_pGraphics->resX * 0x54 / 0x280 + x,
                  (int)g_pGraphics->resY * 0x10 / 0x1e0 + y,
                  (int *)g_barTextColour, 0x14);
    Font_DrawText(g_unk0x0051c980,
                  (char *)CGenericFileLoader::StrUpperPolish((BYTE *)FUN_004752f0((int *)param6, param5, 0)),
                  (int)g_pGraphics->resX * 0x54 / 0x280 + x,
                  (int)g_pGraphics->resY * 0x1c / 0x1e0 + y,
                  (int *)g_barTextColour, 0x14);
}

BYTE FUN_004bc0c0(BYTE *p);

// Draws one pair of text lines at a stage-object rectangle, scaling the
// rectangle vertically when the panel is animating in.
// match 81%: the original's Font_DrawText takes 16-bit x/y (it adds them as words and
// pushes the register unextended); our Font.h declares them 32-bit, so every coordinate
// costs one movsx. Logic and constants are exact.
// FUNCTION: CMR2 0x00475430
void FUN_00475430(int param1, KnockoutMatch *param2, short *param3, BYTE *param4, BYTE *param5,
                  int param6, int param7, int *param8, int param9)
{
    short rect[4];

    if (FUN_004bc0c0(&g_unk0x0058ca80) != 0 || param9 != 0) {
        rect[0] = param3[0];
        rect[1] = param3[1];
        rect[2] = (short)FixMulShift32(param3[2] << 16, g_unk0x0058cc74);
        rect[3] = (short)FixMulShift32(param3[3] << 16, g_unk0x0058cc74);
        FUN_00475740(rect, param4, param5, param6);
        return;
    }
    rect[0] = param3[0];
    rect[1] = param3[1];
    rect[2] = param3[2];
    rect[3] = param3[3];
    FUN_00475740(rect, param4, param5, param6);
    if (CGameInfo::GetScreenWidth() >= 0x400 && CFrontend::FUN_004b7560(0x400) != 0 &&
        CFrontend::FUN_004b7590(0x400) != 0) {
        Font_DrawText(0, (char *)CGenericFileLoader::StrUpperPolish((BYTE *)FUN_004736b0(param2, param1)),
                      (short)((short)((int)g_pGraphics->resX * 0x54 / 0x280) + param3[0]),
                      (short)((short)((int)g_pGraphics->resY * 0x10 / 0x1e0 - (int)g_pGraphics->resY * 2 / 0x1e0) + param3[1]),
                      param8, 0x14);
        Font_DrawText(g_unk0x0051c980,
                      (char *)CGenericFileLoader::StrUpperPolish((BYTE *)FUN_004752f0((int *)param2, param1, param7)),
                      (short)((short)((int)g_pGraphics->resX * 0x54 / 0x280) + param3[0]),
                      (short)((short)((int)g_pGraphics->resY * 7 / 0x1e0 + (int)g_pGraphics->resY * 0x1c / 0x1e0) + param3[1]),
                      param8, 0x14);
        return;
    }
    Font_DrawText(0, (char *)CGenericFileLoader::StrUpperPolish((BYTE *)FUN_004736b0(param2, param1)),
                  (short)((short)((int)g_pGraphics->resX * 0x54 / 0x280) + param3[0]),
                  (short)((short)((int)g_pGraphics->resY * 0x10 / 0x1e0) + param3[1]),
                  param8, 0x14);
    Font_DrawText(g_unk0x0051c980,
                  (char *)CGenericFileLoader::StrUpperPolish((BYTE *)FUN_004752f0((int *)param2, param1, param7)),
                  (short)((short)((int)g_pGraphics->resX * 0x54 / 0x280) + param3[0]),
                  (short)((short)((int)g_pGraphics->resY * 0x1c / 0x1e0) + param3[1]),
                  param8, 0x14);
}

BYTE *FUN_0041b390(void);

// Steps every live replay object: advances the frame counter of the current
// record and appends the next one, rebuilding the lookup row when the current
// frame is exhausted.
// match 38%: the original keeps the constant zero in EBX and a separate `flag`/`valid`
// pair that MSVC folds here, which moves the loop's register allocation; the replay
// stepping logic and constants are otherwise transcribed from the dump
// FUNCTION: CMR2 0x0046c8e0
void FUN_0046c8e0(void)
{
    void ***pp;
    void **pObj;
    int *pRec;
    short *pIndex;
    int state;
    int flag;
    int valid;
    int slot;
    int offset;
    BYTE *pEnt;
    int base;
    char c;
    BYTE b;

    for (pp = g_unk0x00588d40; pp < g_unk0x00588d40 + 16; pp++) {
        pObj = *pp;
        if (pObj == NULL)
            continue;
        pRec = (int *)*pObj;
        if (pRec == NULL)
            continue;
        if (pRec[3] == 0)
            continue;
        if (pRec[7] == 2)
            continue;
        if (*(BYTE *)((BYTE *)Car_Get(*(BYTE *)((BYTE *)pRec + 0x20)) + 0xb43) <= 0)
            continue;

        state = pRec[4];
        pIndex = (short *)(pRec[0x41] + *(short *)((BYTE *)pRec + 0x100) * 2);
        flag = 0;
        valid = 0;
        if (state != 0)
            valid = 1;
        else
            flag = 1;
        if (valid == 0)
            goto done;
        if (state == 0)
            goto done;

        slot = *(short *)((BYTE *)pRec + 0xfe) * *(short *)((BYTE *)pRec + 0x100) + *pIndex;
        offset = pRec[0xf] + slot * 4;
        if (FUN_0046cbe0((BYTE *)offset, *(BYTE *)((BYTE *)pRec + 0x20)) == 0) {
            *pIndex += 1;
            if (*pIndex == *(short *)((BYTE *)pRec + 0xfe)) {
                pRec[4] = 0;
                *pIndex += 1;
                *(short *)((BYTE *)pRec + 0x100) += 1;
                if (*(short *)((BYTE *)pRec + 0x100) == *(short *)((BYTE *)pRec + 0xfc))
                    Replay_StopRecording((BYTE *)pRec);
                goto done;
            }
            slot = *(short *)((BYTE *)pRec + 0xfe) * *(short *)((BYTE *)pRec + 0x100) + *pIndex;
            offset = pRec[0xf] + slot * 4;
            *(BYTE *)(offset + 2) = *(BYTE *)(pRec[0xf] + 2 + slot * 4) & 0xc0;
        }
        FUN_0046c450((BYTE *)offset, *(BYTE *)((BYTE *)pRec + 0x20));

        base = (int)FUN_0041b390();
        c = CGameInfo::FUN_00405e00();
        if (c == '\0')
            c = *(char *)(*(int *)(base + 4) + (DWORD)*(BYTE *)((BYTE *)pRec + 0x20) * 8);
        else
            c = **(char **)(base + 4);

        if (pRec[7] == 0) {
            base = pRec[9] + *(short *)((BYTE *)pRec + 0x100) * 0x114c;
            pEnt = (BYTE *)(base + 0x110c + (DWORD)*(BYTE *)(base + 0x1148) * 6);
            if (c != *(char *)(pEnt - 2)) {
                *(char *)(pEnt + 4) = c;
                *(short *)pEnt = *pIndex;
                *(unsigned short *)(pEnt + 2) = *(BYTE *)(offset + 2) & 0x3f;
                *(BYTE *)(base + 0x1148) += 1;
                if (*(short *)(pEnt + 2) == 0) {
                    b = *(BYTE *)(pRec[0xf] - 2 + slot * 4);
                    *(short *)pEnt -= 1;
                    *(unsigned short *)(pEnt + 2) = b & 0x3f;
                } else {
                    *(short *)(pEnt + 2) -= 1;
                }
            }
        } else {
            base = *(short *)((BYTE *)pRec + 0x100) * 0x5c + pRec[0xc];
            pEnt = (BYTE *)(base + (DWORD)*(BYTE *)(base + 0x58) * 6);
            if (c != *(char *)(pEnt + 0x1a)) {
                *(char *)(pEnt + 0x20) = c;
                *(short *)(pEnt + 0x1c) = *pIndex;
                *(unsigned short *)(pEnt + 0x1e) = *(BYTE *)(offset + 2) & 0x3f;
                *(BYTE *)(base + 0x58) += 1;
                if (*(short *)(pEnt + 0x1e) == 0) {
                    b = *(BYTE *)(pRec[0xf] - 2 + slot * 4);
                    *(short *)(pEnt + 0x1c) -= 1;
                    *(unsigned short *)(pEnt + 0x1e) = b & 0x3f;
                } else {
                    *(short *)(pEnt + 0x1e) -= 1;
                }
            }
        }

done:
        if (flag != 0) {
            offset = pRec[0xf] + *(short *)((BYTE *)pRec + 0xfe) * *(short *)((BYTE *)pRec + 0x100) * 4;
            *(short *)(pRec[0x41] + *(short *)((BYTE *)pRec + 0x100) * 2) = 0;
            *(BYTE *)(offset + 2) &= 0xc0;
            pRec[6] = 1;
            if (pRec[7] == 0)
                pRec[0xb] = pRec[9] + *(short *)((BYTE *)pRec + 0x100) * 0x114c;
            else
                pRec[0xe] = *(short *)((BYTE *)pRec + 0x100) * 0x5c + pRec[0xc];
            pRec[4] = 1;
        }
    }
}

int FUN_004218d0(unsigned int view);
void RallyData_FUN_00408760(BYTE index, int value);

// Driver-view resolver for the cycle/camera keys: picks the target driver
// (current, best round driver or shifted by the digital control) and applies
// the view change when the requested view is in range.
// match 50%: implementada, MSVC6 mantiene param_1 en EDI (el original lo recarga de la pila) y la tabla del switch cae en otra direccion (<OFFSET>)
// FUNCTION: CMR2 0x0047b970
void FUN_0047b970(unsigned int param_1)
{
    unsigned int target;
    unsigned int delta;

    if (*(char *)(*(int *)(FUN_0041b390() + 4)) == 0xa) {
        if (FUN_0041f3a0() == 0) {
            target = FUN_004218d0(0);
        } else if (CGameInfo::FUN_00405d80() == 2) {
            target = param_1;
        } else {
            target = FUN_004218d0(1);
        }
    } else {
        target = FUN_004218d0(*(BYTE *)((BYTE *)g_unk0x0058e0a0 + 0xb1a));
    }
    if (*(char *)(*(int *)(FUN_0041b390() + 4)) == 8 ||
        *(char *)(*(int *)(FUN_0041b390() + 4)) == 7) {
        switch (FUN_0041b380()) {
        case 0:
        case 1:
            g_unk0x0058e0a4 = param_1;
            break;
        case 2:
            RallyData_GetRoundDrivers(&g_unk0x0058df98, &g_unk0x0058df9c);
            if (RallyData_FUN_00408500(g_unk0x0058df98 & 0xff) == -1)
                g_unk0x0058e0a4 = g_unk0x0058df98;
            else
                g_unk0x0058e0a4 = g_unk0x0058df9c;
            break;
        case 3:
            if (param_1 == 0)
                RallyData_GetRoundDrivers(&g_unk0x0058e0a4, &g_unk0x0058df9c);
            else
                RallyData_GetRoundDrivers(&g_unk0x0058df98, &g_unk0x0058e0a4);
            break;
        case 4:
            delta = FUN_0041b370();
            g_unk0x0058e0a4 = (delta & 0xff) + param_1;
            break;
        }
        if ((int)g_unk0x0058e0a4 < (int)CGameInfo::FUN_00405d70() &&
            (target == 1 || target == 2 || target == 3 || target == 5 || target == 4))
            RallyData_FUN_00408760(g_unk0x0058e0a4, target);
    }
}
