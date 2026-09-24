#include <windows.h>
#include <string.h>
#include "RallyData.h"
#include "SceneNode.h"
#include "Frontend.h"
#include "AIHelper.h"
#include "RegKey.h"
#include <stdio.h>
#include <math.h>
#include "FixedPoint.h"
#include "Car.h"
#include "GameInfo.h"
#include "Input.h"
#include "main.h"
#include "Game.h"

// Accessors of the stage object tables (0x460bf0-0x4789b0)

extern void *g_unk0x00547ac8;
extern void *g_unk0x00543ecc;
extern int g_unk0x0058cf7c;
extern BYTE *g_unk0x0058c94c;
extern unsigned int g_unk0x0058ca6c;

int FUN_0046d2a0(int *p);
int RallyData_FUN_00421370(BYTE *p);
int RallyData_FUN_00421420(void);
unsigned int RallyData_FUN_00407e90(void);

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
int g_unk0x00588970[16];
// GLOBAL: CMR2 0x00588ba4
BYTE g_unk0x00588ba4[16];
// GLOBAL: CMR2 0x00588bb4
int g_unk0x00588bb4[16];
// GLOBAL: CMR2 0x00588cd4
int g_unk0x00588cd4[16 * 2];
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
// GLOBAL: CMR2 0x0058cf68
int g_unk0x0058cf68;
// GLOBAL: CMR2 0x0058cf80
BYTE g_unk0x0058cf80[0x100];
// GLOBAL: CMR2 0x0058d2a0
int g_unk0x0058d2a0[12];
// GLOBAL: CMR2 0x0058d3b0
int g_unk0x0058d3b0[64];
// GLOBAL: CMR2 0x0058d6a8
int g_unk0x0058d6a8[2];
// GLOBAL: CMR2 0x0058d6b0
int g_unk0x0058d6b0[7];
// GLOBAL: CMR2 0x0058da30
int g_unk0x0058da30[8];
// GLOBAL: CMR2 0x0058da10
int g_unk0x0058da10[8];
// GLOBAL: CMR2 0x0058dda8
int g_unk0x0058dda8;
// GLOBAL: CMR2 0x0058e230
int g_unk0x0058e230[16];
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

struct StageObjectEntry0x128 { int *pObject; BYTE rest[0x124]; };
// GLOBAL: CMR2 0x005894e4
StageObjectEntry0x128 g_unk0x005894e4[40];
// GLOBAL: CMR2 0x0058c324
int g_unk0x0058c324;
// GLOBAL: CMR2 0x0058c924
BYTE g_unk0x0058c924;

// FUNCTION: CMR2 0x0046f7e0
void FUN_0046f7e0(void)
{
    g_unk0x0058c924 = 0;
    StageObjectEntry0x128 *p = g_unk0x005894e4;
    do {
        if (p->pObject != NULL)
            p->pObject[0xcc / 4] += 0xd8f00000U;
        p++;
    } while ((int)p < (int)&g_unk0x0058c324);
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
// TODO: CMR2 0x00460c30 (implemented, match 72%)
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

// FUNCTION: CMR2 0x0046b4c0
int FUN_0046b4c0(BYTE *pCar)
{
    return g_unk0x00588970[(signed char)pCar[0xb1a]];
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

// FUNCTION: CMR2 0x0046bfb0
void FUN_0046bfb0(Block0x309 *pSrc, Block0x309 *pDst)
{
    *pDst = *pSrc;
}

// FUNCTION: CMR2 0x0046c180
void FUN_0046c180(Block0x134 *pSrc, Block0x134 *pDst)
{
    *pDst = *pSrc;
}

// FUNCTION: CMR2 0x0046c220
void FUN_0046c220(Block6 *pSrc, Block6 *pDst)
{
    *pDst = *pSrc;
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

// FUNCTION: CMR2 0x0046f4e0
void FUN_0046f4e0(int *pOut1, int *pOut2)
{
    *pOut1 = g_unk0x00589440;
    *pOut2 = g_unk0x00589444;
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
// TODO: CMR2 0x004736b0 (implemented, match 55%)
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
// TODO: CMR2 0x00473790 (implemented, match 66%)
int FUN_00473790(KnockoutMatch *pMatch, int side)
{
    unsigned int driver;

    if (side == 0)
        driver = pMatch->flags;
    else
        driver = pMatch->flags >> 5;
    if ((driver & 0x1f) == 0x1f)
        return 0;
    return RallyData_FUN_00408500(driver & 0x1f) == -1;
}

// Side of the match to show: the human player's side when it is param2,
// otherwise the other side.
// TODO: CMR2 0x004737d0 (implemented, match 62%)
int FUN_004737d0(KnockoutMatch *pMatch, int param2)
{
    if ((pMatch->flags & 0x1f) != 0x1f && RallyData_FUN_00408500(pMatch->flags & 0x1f) == -1)
        return param2 != 0;
    return param2 == 0;
}

// Same as FUN_004736b0 with the car names of the AI drivers.
// TODO: CMR2 0x00473810 (implemented, match 55%)
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

// FUNCTION: CMR2 0x00475f70
BYTE *FUN_00475f70(void)
{
    return g_unk0x0058cf80;
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

// FUNCTION: CMR2 0x00477a90
void FUN_00477a90(void)
{
    memset(g_unk0x0058d6b0, 0xff, sizeof(g_unk0x0058d6b0));
    memset(g_unk0x0058d2a0, 0xff, sizeof(g_unk0x0058d2a0));
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
// GLOBAL: CMR2 0x00590b7c
BYTE *g_unk0x00590b7c[5][8];
// GLOBAL: CMR2 0x00590c24
BYTE g_unk0x00590c24[8][8];
// GLOBAL: CMR2 0x00590d70
int g_unk0x00590d70;
// GLOBAL: CMR2 0x00590db0
int g_unk0x00590db0[64];
// GLOBAL: CMR2 0x00590ed0
BYTE g_unk0x00590ed0[8][0x98];
// GLOBAL: CMR2 0x00591390
int g_unk0x00591390;
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

// FUNCTION: CMR2 0x004805f0
void FUN_004805f0(int value)
{
    g_unk0x005909bc = value;
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

// FUNCTION: CMR2 0x0048d930
int FUN_0048d930(BYTE *p)
{
    return g_unk0x00591740[*p];
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
// TODO: CMR2 0x0047cc50 (implemented, match 78%)
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

void FUN_004bcad0(int value);
void Scene_SetLight(FixVector *pLight, int boost);
int Track_GetGroundHeightSurface(FixVector *pPoint, FixVector *pNormal, short *pTri, short *pSurfaceClass,
                                 unsigned short *pSurface, int defaultY);
DWORD FUN_004b74e0(void);
DWORD FUN_004b74f0(void);
DWORD FUN_004b7500(void);
int *FUN_00469680(int index);
void FUN_00480ac0(BYTE *pCar, int slot, int reset);
void FUN_0042b720(int index, BYTE value);
int FUN_00457e10(BYTE *pCar, int offset);
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
// TODO: CMR2 0x0047c1b0 (implemented, match 75%)
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

// TODO: CMR2 0x0046b670 (implemented, match 85%)
void FUN_0046b670(BYTE *pCar)
{
    int *p;
    int i;

    p = FUN_00469680((char)pCar[0xb1a]) + 0x4b0 / 4;
    for (i = 0; i < 4; i++) {
        FUN_00480ac0(pCar, i, *p);
        p++;
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

// Eight records of 0x48 bytes whose ten shorts from +0 are reset to -1.
// GLOBAL: CMR2 0x0058d6ec
short g_unk0x0058d6ec[8][0x24];

// TODO: CMR2 0x00477f30 (implemented, match 64%)
void FUN_00477f30(void)
{
    short *p;

    p = &g_unk0x0058d6ec[0][1];
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
    } while ((int)p < (int)&g_unk0x0058d6ec[8][1]);
}

int FUN_00460c80(BYTE *pCar);
char RallyData_FUN_00408500(BYTE param1);

// GLOBAL: CMR2 0x005909b8
int g_unk0x005909b8;
// GLOBAL: CMR2 0x005909c0
BYTE g_unk0x005909c0[4];
// GLOBAL: CMR2 0x005909c4
BYTE g_unk0x005909c4[4];

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

// Selects a list of 0x6c-byte records (count first); returns whether it is non-empty.
// TODO: CMR2 0x0048caa0 (implemented, match 17%)
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
// TODO: CMR2 0x0048dca0 (implemented, match 25%)
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

// TODO: CMR2 0x0048df10 (implemented, match 83%)
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

// TODO: CMR2 0x00486b90 (implemented, match 51%)
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

// FUNCTION: CMR2 0x00487e00
void FUN_00487e00(FixVector *pPos, int *pInfo)
{
    g_unk0x00591498 = *pPos;
    g_unk0x00591490 = *(int *)pInfo[1];
    g_unk0x00591494 = *(int *)(*(int *)(*(int *)(pInfo[0] + 0xc) + 0x10c) + 0x4c);
}

extern int g_unk0x0051bd40;
extern int g_unk0x0051bd3c;

// Sets the scale 0x51bd40 (value / 25, at least 0.6) and its reciprocal 0x51bd3c.
// TODO: CMR2 0x00466630 (implemented, match 89%)
void FUN_00466630(int value)
{
    g_unk0x0051bd40 = FixMul(value, 0xa3d);
    if (g_unk0x0051bd40 < 0x9999)
        g_unk0x0051bd40 = 0x9999;
    g_unk0x0051bd3c = FixDiv(0x10000, g_unk0x0051bd40);
}

// Blends two byte values: b + (a - b) * t, clamped to 255.
// TODO: CMR2 0x004616c0 (implemented, match 83%)
int FUN_004616c0(BYTE a, BYTE b, int t)
{
    int v;

    v = ((int)b * 0x10000 + FixMul((int)a * 0x10000 - (int)b * 0x10000, t)) >> 16;
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
