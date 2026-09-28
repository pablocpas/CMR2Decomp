#include <windows.h>
#include <stdlib.h>
#include "Stage.h"
#include "GameInfo.h"
#include "Graphics.h"
#include "main.h"

// GLOBAL: CMR2 0x00542cac
char g_stageSplitCount = 0;
// GLOBAL: CMR2 0x00542cad
char g_unk0x00542cad;
// GLOBAL: CMR2 0x00542cae
char g_stageLooped;
// GLOBAL: CMR2 0x00542cb0
int g_unk0x00542cb0;
// GLOBAL: CMR2 0x00542cb4
int g_unk0x00542cb4[8];     // per player: finished
// GLOBAL: CMR2 0x00542c68
int g_unk0x00542c68;
// GLOBAL: CMR2 0x00542c70
int g_stageCheckpointCount;
// GLOBAL: CMR2 0x00542c74
int g_unk0x00542c74;
// GLOBAL: CMR2 0x00542c7c
int g_unk0x00542c7c[12];

// FUNCTION: CMR2 0x004583c0
int GetStageSplitCount(void)
{
	return g_stageSplitCount;
}

// FUNCTION: CMR2 0x00458390
int FUN_00458390(void)
{
    return g_unk0x00542c68;
}

// FUNCTION: CMR2 0x004583a0
int FUN_004583a0(void)
{
    return g_unk0x00542c74;
}

// FUNCTION: CMR2 0x004583b0
int FUN_004583b0(int index)
{
    return g_unk0x00542c7c[index];
}

// GLOBAL: CMR2 0x00542d38
int g_unk0x00542d38[8];

#include "Car.h"
void RallyData_FUN_004213d0(Car *pCar, int value);

// Start positions (x, -, z) of the cars when the rally modes use fixed grids.
// GLOBAL: CMR2 0x00542cd8
int g_unk0x00542cd8[8][3];

unsigned int RallyData_FUN_00407e70(void);
unsigned int RallyData_GetFlag31(void);
unsigned int RallyData_FUN_00407e90(void);

// Start x/z of a car: from the grid table, or one of the stage's start
// points (pStarts, 3 ints each) depending on the mode.
// FUNCTION: CMR2 0x004583d0
void FUN_004583d0(int car, int *pStarts, int *pOut)
{
    int i;

    if (!(BYTE)RallyData_FUN_00407e70() && !(BYTE)RallyData_GetFlag31()) {
        if ((BYTE)RallyData_FUN_00407e90()) {
            if (CGameInfo::FUN_00405e00()) {
                i = g_unk0x00542cb4[car];
                pOut[0] = pStarts[i * 3];
                pOut[2] = pStarts[i * 3 + 2];
                return;
            }
            if (car != g_unk0x00542cb0) {
                pOut[0] = pStarts[3];
                pOut[2] = pStarts[5];
                return;
            }
        }
        pOut[0] = pStarts[0];
        pOut[2] = pStarts[2];
        return;
    }
    pOut[0] = g_unk0x00542cd8[car][0];
    pOut[2] = g_unk0x00542cd8[car][2];
}

// Puts every car back on its stored route position.
// FUNCTION: CMR2 0x00458480
void FUN_00458480(void)
{
    int i;

    for (i = 0; i < g_unk0x00542c68; i++) {
        Car *pCar = Car_Get(i);
        RallyData_FUN_004213d0(pCar, g_unk0x00542d38[i] >> 16);
    }
}

// FUNCTION: CMR2 0x004584c0
int FUN_004584c0(void)
{
    return g_unk0x00542cb0;
}

// Previous checkpoint, wrapping round on looped stages.
// match 60%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00459320
int FUN_00459320(int index)
{
    index--;
    if (index == -1) {
        if (g_stageLooped != 0)
            index = g_stageCheckpointCount - 1;
        else
            index = 0;
    }
    return index;
}

// Next checkpoint, wrapping round on looped stages.
// FUNCTION: CMR2 0x00459350
int FUN_00459350(int index)
{
    index++;
    if (index == g_stageCheckpointCount && g_stageLooped != 0)
        index = 0;
    return index;
}

// 16.16 <-> 12-bit angle index factor (defined in GameInfo.cpp).
extern double g_unk0x00511300;

// Side of the two-player split start drawn last time (0 or 1).
// GLOBAL: CMR2 0x00542cd4
int g_unk0x00542cd4;

unsigned char RallyDataStageIndex(void);
unsigned int RallyDataState(void);
unsigned int RallyData_GetFlag24(void);
int RallyData_FUN_00421420(void);
void RallyData_FUN_00421530(int index, int *pOut);
int StageObject_Atan2Degrees(int y, int x);
int FUN_0040cec0(int index);
int FUN_0040b1a0(int index);
int FUN_0040b1b0(void);
int FUN_0040b010(int index);
int FUN_0040a7a0(int id);
unsigned long FUN_004a1a00(void);

// Builds the start grid of the stage. The mode of the current game picks one
// of four layouts: a ring of cars around the first checkpoint (0), the
// two-player split start (1), one row per network player (2) and the plain
// network grid (3). param != 0 reuses the side of the two-player split drawn
// before instead of drawing a new one with rand().
// match 47%: implementada, MSVC6 no reproduce el reparto de registros/pila ni la fusion de bloques del switch del original (despachador con tabla de saltos)
// FUNCTION: CMR2 0x004584d0
void FUN_004584d0(char param)
{
    int mode = 0;
    int i;

    if (RallyData_GetFlag31()) {
        mode = 2;
    } else if (CGameInfo::FUN_00405e00() && RallyDataStageIndex() == 0xa &&
               CGameInfo::FUN_00405d80() != 0xa) {
        mode = 3;
    } else if (g_unk0x00542c68 == 1 || CGameInfo::FUN_00405d80() == 0xa) {
        return;
    } else if (!RallyData_GetFlag24() && g_unk0x00542c68 == 2) {
        if (!RallyData_FUN_00407e90())
            return;
        mode = 1;
    }

    switch (mode) {
    case 0: {
        int pFirst[3];
        int pNext[3];
        int angle;
        int index;
        int sinValue;
        int cosValue;
        int sinSide;
        int cosSide;
        int offset;
        int player;
        int fallback = 1;
        int x;
        int z;

        RallyData_FUN_00421530(RallyData_FUN_00421420() - 1, pFirst);
        RallyData_FUN_00421530(0, pNext);
        angle = StageObject_Atan2Degrees(pNext[2] - pFirst[2], pNext[0] - pFirst[0]);
        index = (short)(__int64)((double)angle * g_unk0x00511300);
        sinValue = g_sinTable[index & 0xfff];
        cosValue = g_sinTable[(index + 0x400) & 0xfff];
        sinSide = FixMul(0x40000, sinValue);
        cosSide = FixMul(0x40000, cosValue);
        for (i = 0; i < g_unk0x00542c68; i++) {
            player = i;
            if (CGameInfo::FUN_00405d80() == 5 && g_unk0x00542c68 > 2)
                player = FUN_0040cec0(i);
            if ((BYTE)RallyDataState() == 2)
                player = fallback;
            g_unk0x00542d38[player] = RallyData_FUN_00421420() - 1;
            if (g_unk0x00542c68 > 2)
                offset = FixMul(0x50000, (int)(__int64)((double)player * CGraphics::m_65536)) -
                         (int)(__int64)((double)(g_unk0x00542c68 * 5) * CGraphics::m_65536);
            else
                offset = -0xa0000;
            x = FixMul(offset, cosValue);
            z = FixMul(offset, sinValue);
            if (player % 2 == 0) {
                x += sinSide;
                z -= cosSide;
            } else {
                x -= sinSide;
                z += cosSide;
            }
            g_unk0x00542cd8[i][0] = x + pNext[0];
            g_unk0x00542cd8[i][2] = z + pNext[2];
            fallback--;
        }
        return;
    }
    case 1: {
        // 0x00542c80 is g_unk0x00542c7c[1], the first split of the stage.
        int split = g_unk0x00542c7c[1];
        int side;

        if (param) {
            side = g_unk0x00542cd4;
        } else {
            srand(CMain::GetFrameTime());
            side = rand() <= 0x3fff;
        }
        g_unk0x00542cd4 = side;
        g_unk0x00542cb0 = side;
        for (i = 0; i < 2; i++) {
            if ((side + i) % 2 == 0)
                g_unk0x00542d38[i] = 0;
            else
                g_unk0x00542d38[i] = split;
        }
        return;
    }
    case 2: {
        int pFirst[3];
        int pNext[3];
        int angle;
        int index;
        int sinValue;
        int cosValue;
        int sinSide;
        int cosSide;
        int offset;
        char count;
        int car;
        int x;
        int z;

        RallyData_FUN_00421530(RallyData_FUN_00421420() - 1, pFirst);
        RallyData_FUN_00421530(0, pNext);
        angle = StageObject_Atan2Degrees(pNext[2] - pFirst[2], pNext[0] - pFirst[0]);
        index = (short)(__int64)((double)angle * g_unk0x00511300);
        sinValue = g_sinTable[index & 0xfff];
        cosValue = g_sinTable[(index + 0x400) & 0xfff];
        sinSide = FixMul(0x40000, sinValue);
        cosSide = FixMul(0x40000, cosValue);
        count = (char)FUN_0040b1b0();
        for (i = 0; i < count; i++) {
            if (FUN_0040b1a0(i) == (int)FUN_004a1a00())
                car = 0;
            else
                car = FUN_0040b010(FUN_0040a7a0(FUN_0040b1a0(i)));
            g_unk0x00542d38[car] = RallyData_FUN_00421420() - 1;
            if (count > 2)
                offset = FixMul(0x50000, (int)(__int64)((double)i * CGraphics::m_65536)) -
                         (int)(__int64)((double)(count * 5) * CGraphics::m_65536);
            else
                offset = -0xa0000;
            x = FixMul(offset, cosValue);
            z = FixMul(offset, sinValue);
            if (i % 2 == 0) {
                x += sinSide;
                z -= cosSide;
            } else {
                x -= sinSide;
                z += cosSide;
            }
            g_unk0x00542cd8[car][0] = x + pNext[0];
            g_unk0x00542cd8[car][2] = z + pNext[2];
        }
        return;
    }
    case 3: {
        char count = (char)FUN_0040b1b0();
        int slot;

        for (i = 0; i < count; i++) {
            if (FUN_0040b1a0(i) == (int)FUN_004a1a00())
                slot = 0;
            else
                slot = FUN_0040b010(FUN_0040a7a0(FUN_0040b1a0(i)));
            if (i % 2 == 0) {
                g_unk0x00542d38[slot] = 0;
                g_unk0x00542cb4[slot] = 0;
            } else {
                g_unk0x00542d38[slot] = g_unk0x00542c7c[1];     // 0x00542c80
                g_unk0x00542cb4[slot] = 1;
            }
        }
        return;
    }
    }
}

