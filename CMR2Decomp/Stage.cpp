#include <windows.h>
#include "Stage.h"
#include "GameInfo.h"

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
// match 86%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00458480
void FUN_00458480(void)
{
    int i;

    for (i = 0; i < g_unk0x00542c68; i++)
        RallyData_FUN_004213d0(Car_Get(i), g_unk0x00542d38[i] >> 16);
}

// FUNCTION: CMR2 0x004584c0
int FUN_004584c0(void)
{
    return g_unk0x00542cb0;
}

// Previous checkpoint, wrapping round on looped stages.
// match 61%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
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

