#include "StageBlock.h"
#include "StageTiming.h"
#include <stdio.h>
#include "Frontend.h"
#include "GenericFileLoader.h"
#include "Car.h"
#include "Stage.h"
#include "RallyData.h"
#include "TimingUtils.h"
#include "RallyTiming.h"
#include "GameInfo.h"
#include "InstallInfo.h"
#include "Game.h"
#include "Input.h"
#include "FileBuffer.h"
#include "SceneNode.h"
#include "StageUI.h"
#include "NetPlayers.h"
#include "Mesh.h"
#include "Graphics.h"
#include "Font.h"
#include "main.h"
#include <math.h>
#include <string.h>

// Three 0x30-byte particle look records (rain, snow, splash): +0 scale,
// +0x8 extent, +0x20 colour, +0x28/+0x2c texture v range.
// GLOBAL: CMR2 0x00543da0
BYTE g_unk0x00543da0Block[0x90];
#define g_unk0x00543da0 (*(int *)g_unk0x00543da0Block)

// GLOBAL: CMR2 0x00547930
FixVector g_unk0x00547930;

// Wind strength at the last change.
// GLOBAL: CMR2 0x0054793c
int g_unk0x0054793c;
// GLOBAL: CMR2 0x00547940
int g_unk0x00547940;
// GLOBAL: CMR2 0x00547944
FixVector g_unk0x00547944;

// Scales a timed stage event's displacement and vertical offset.
// FUNCTION: CMR2 0x00460270
void StageTiming_UpdateEventDisplacement(int *event)
{
    int t = FixMul(event[0x16], g_unk0x00543da0);
    if (t > 0x10000) t = 0x10000;
    int scale = FixMul(t, g_unk0x00547940);
    FixVecScale((FixVector *)(event + 0xb), &g_unk0x00547930, scale);
    if (event[0] == 1)
        event[0xc] = -0x13333 - FixMul(0xcccd, t);
    else
        event[0xc] = -0x1999 - FixMul(0x3333, t);
}

// GLOBAL: CMR2 0x00541f08
StageFile g_stageFiles[7];

// GLOBAL: CMR2 0x00541f78
char g_stageSplitUnk0x00541f78[10][2];

// GLOBAL: CMR2 0x00541f8c
int g_unk0x00541f8c;

// GLOBAL: CMR2 0x00541f90
char g_unk0x00541f90[8];
// GLOBAL: CMR2 0x00541f98
int g_unk0x00541f98;

// GLOBAL: CMR2 0x00541f9c
char g_stageDriverSlot[16];

// Ten split rows of sixteen drivers. The removal loop addresses the three
// adjacent tables through one base, so their storage must stay contiguous.
struct StageSplitRankings {
    char indices[10][16];   // 0x000
    char positions[10][16]; // 0x0a0
    char timeIndices[10][16]; // 0x140
    char count[12];         // 0x1e0
};
// GLOBAL: CMR2 0x00541fac
StageSplitRankings g_stageSplitRankings;
#define g_stageSplitDriverIndices (g_stageSplitRankings.indices)
#define g_stageSplitPositions (g_stageSplitRankings.positions)
#define g_stageSplitTimesRawDriverIx (g_stageSplitRankings.timeIndices)
#define g_stageSplitDriverCount (g_stageSplitRankings.count)

// GLOBAL: CMR2 0x00542418
short g_unk0x00542418;
// Split time spread table of the stage (5-byte rows: -, max down, max up).
// GLOBAL: CMR2 0x0054241c
char *g_unk0x0054241c;

// FUNCTION: CMR2 0x00456a30
void FUN_00456a30(int offset, int unused)
{
    *((BYTE *)&g_unk0x00542418 + offset) = 1;
}

// GLOBAL: CMR2 0x00542604
BYTE g_unk0x00542604;

// GLOBAL: CMR2 0x00542198
int g_stageSplitTimesRaw[10][16];


// FUNCTION: CMR2 0x00455cf0
int StageTiming_GetDriverIDForPosition(int positionIx)
{
    int iSplitIx = GetStageSplitCount();

    /* This isn't the driver name, just the index in the lookup table. It looks like
       driver ix 15 is the player */
    return g_stageSplitDriverIndices[iSplitIx][positionIx];
}

// FUNCTION: CMR2 0x00455d70
int StageTiming_GetTimeForPosition(int iPosition)
{
    int stageSplitCount;
    int iTime;

    if (iPosition >= 0 && iPosition <= 15)
    {
        stageSplitCount = GetStageSplitCount();
        iTime = g_stageSplitTimesRaw[stageSplitCount][g_stageSplitTimesRawDriverIx[stageSplitCount][iPosition]];
        return ConvertRawTimeToCentiseconds(iTime);
    }

    return -1;
}

// FUNCTION: CMR2 0x00455020
void StageTiming_FreeStageFile5(void)
{
    if (g_stageFiles[5].buffer != NULL)
        CFileBuffer::FreeGenericFileBuffer(g_stageFiles[5].buffer);
    g_stageFiles[5].buffer = NULL;
    g_stageFiles[5].loaded = FALSE;
    g_stageFiles[5].size = 0;
}

// FUNCTION: CMR2 0x00455050
void StageTiming_FreeStageFile2(void)
{
    if (g_stageFiles[2].buffer != NULL)
        CFileBuffer::FreeGenericFileBuffer(g_stageFiles[2].buffer);
    g_stageFiles[2].buffer = NULL;
    g_stageFiles[2].loaded = FALSE;
    g_stageFiles[2].size = 0;
}

// FUNCTION: CMR2 0x00455290
StageFile *StageTiming_GetStageFile0(void)
{
    return &g_stageFiles[0];
}

// FUNCTION: CMR2 0x004552a0
StageFile *StageTiming_GetStageFile1(void)
{
    return &g_stageFiles[1];
}

// FUNCTION: CMR2 0x004552b0
StageFile *StageTiming_GetStageFile2(void)
{
    return &g_stageFiles[2];
}

// FUNCTION: CMR2 0x004552c0
StageFile *StageTiming_GetStageFile3(void)
{
    return &g_stageFiles[3];
}

// FUNCTION: CMR2 0x004552d0
StageFile *StageTiming_GetStageFile4(void)
{
    return &g_stageFiles[4];
}

// FUNCTION: CMR2 0x004552e0
StageFile *StageTiming_GetStageFile5(void)
{
    return &g_stageFiles[5];
}

// FUNCTION: CMR2 0x004552f0
StageFile *StageTiming_GetStageFile6(void)
{
    return &g_stageFiles[6];
}

// FUNCTION: CMR2 0x00455460
int StageTiming_FUN_00455460(void)
{
    return g_unk0x00541f8c;
}

// FUNCTION: CMR2 0x00455610
void StageTiming_FUN_00455610(void)
{
    g_unk0x00542418 = 0;
}

int FUN_004583a0(void);

// Gives the last count driver slots (15 downwards) to the split table.
// FUNCTION: CMR2 0x004556a0
void FUN_004556a0(int count)
{
    int i;

    g_unk0x00541f98 = 16 - count;
    for (i = 0; i < count; i++)
        g_stageDriverSlot[i] = 15 - i;
    for (i = 1; i <= FUN_004583a0(); i++)
        g_stageSplitDriverCount[i] = g_unk0x00541f98;
}

// GLOBAL: CMR2 0x0051a14c
char g_strRaceHasAiDriver[] = "Race %d has ai driver %d\n";
// GLOBAL: CMR2 0x0051a168
char g_strRaceHasPlayer[] = "Race %d has player\n";
int FUN_0041b380(void);
void FUN_00456a40(int param1, int param2);
void FUN_00455bc0(int slot, int driver);

// Pairs each human player's stage slot with an opponent for the split
// rankings, by game mode: championship/single stages walk the overall
// standings from the back (so each player races the driver just ahead),
// arcade uses the stage positions, and the knockout takes the two drivers
// of every match of the current round.
// FUNCTION: CMR2 0x004556f0
void FUN_004556f0(void)
{
    int players;
    int i;
    int id;
    int slot;
    int left;
    int remaining;
    int pos;
    int *pSlot;
    char *pOut;
    int slots[4];
    char used[16];
    unsigned int *pState;
    int count;
    KnockoutMatch *pMatch;
    int drivers[2];
    int out[2];
    int k;
    int ai;
    char hasAi;
    char hasPlayer;

    players = CGameInfo::FUN_00405d70();
    switch (CGameInfo::FUN_00405d80()) {
    case 0:
    case 1:
        if (FUN_0041b380() == 1) {
            g_stageSplitUnk0x00541f78[0][0] = (char)StageTiming_GetDriverSlot(0);
            g_stageSplitUnk0x00541f78[0][1] = (char)StageTiming_GetDriverSlot(1);
            return;
        }
        memset(used, 0, sizeof(used));
        pSlot = slots;
        for (i = 0; i < 16; i++) {
            id = RallyTiming_GetOverallPositionDriverID(i);
            if (id >= g_unk0x00541f98) {
                slot = 15 - id;
                *pSlot++ = slot;
                g_stageSplitUnk0x00541f78[slot][0] = (char)id;
                g_stageSplitUnk0x00541f78[slot][1] = 16;
            }
        }
        remaining = g_unk0x00541f98;
        left = players;
        pSlot = &slots[players - 1];
        for (pos = 15; pos >= 0; pos--) {
            if (left <= 0)
                break;
            id = RallyTiming_GetOverallPositionDriverID(pos);
            if (id >= g_unk0x00541f98) {
                slot = 15 - id;
                if (g_stageSplitUnk0x00541f78[slot][1] == 16) {
                    k = pos;
                    do
                        id = RallyTiming_GetOverallPositionDriverID(--k);
                    while (id >= g_unk0x00541f98 || used[id] != 0);
                    left--;
                    g_stageSplitUnk0x00541f78[slot][1] = (char)id;
                    used[id] = 1;
                    pSlot--;
                }
            } else {
                if (remaining == left) {
                    slot = *pSlot;
                    left--;
                    used[id] = 1;
                    pSlot--;
                    g_stageSplitUnk0x00541f78[slot][1] = (char)id;
                }
                remaining--;
            }
        }
        // The original tests the last slot touched above, not the one being
        // filled.
        for (i = players, pOut = &g_stageSplitUnk0x00541f78[0][1]; i > 0; i--, pOut += 2) {
            if (g_stageSplitUnk0x00541f78[slot][1] == 16) {
                for (k = 15; k >= 0; k--) {
                    if (used[k] == 0) {
                        *pOut = (char)k;
                        used[k] = 1;
                    }
                }
            }
        }
        for (i = 0; i < players; i++) {
            FUN_00456a40(i, g_stageSplitUnk0x00541f78[i][1]);
            FUN_00455bc0(i, g_stageSplitUnk0x00541f78[i][1]);
        }
        break;
    case 2:
        if (FUN_0041b380() == 1) {
            g_stageSplitUnk0x00541f78[0][0] = (char)StageTiming_GetDriverSlot(0);
            g_stageSplitUnk0x00541f78[0][1] = (char)StageTiming_GetDriverSlot(1);
            return;
        }
        for (i = 0; i < players; i++) {
            g_stageSplitUnk0x00541f78[i][0] = (char)StageTiming_GetDriverSlot(i);
            g_stageSplitUnk0x00541f78[i][1] = (char)StageTiming_GetDriverIDForPosition(i);
            FUN_00456a40(i, g_stageSplitUnk0x00541f78[i][1]);
            FUN_00455bc0(i, g_stageSplitUnk0x00541f78[i][1]);
        }
        break;
    case 4:
        count = 8;
        pState = RallyData_GetChampionshipState();
        switch ((*pState >> 3) & 7) {
        case 1:
            count = 8;
            break;
        case 2:
            count = 4;
            break;
        case 3:
            count = 2;
            break;
        case 4:
            count = 1;
            break;
        }
        pOut = &g_stageSplitUnk0x00541f78[0][1];
        for (i = 0; i < count; i++) {
            hasPlayer = 0;
            hasAi = 0;
            switch ((*pState >> 3) & 7) {
            case 1:
                pMatch = &((KnockoutTable *)pState)->round1[i];
                break;
            case 2:
                pMatch = &((KnockoutTable *)pState)->quarters[i];
                break;
            case 3:
                pMatch = &((KnockoutTable *)pState)->semis[i];
                break;
            case 4:
                pMatch = &((KnockoutTable *)pState)->final;
                break;
            }
            drivers[0] = pMatch->flags & 0x1f;
            drivers[1] = (pMatch->flags >> 5) & 0x1f;
            for (k = 0; k < 2; k++) {
                if (RallyData_FUN_00408500((BYTE)drivers[k]) == -1) {
                    out[k] = StageTiming_GetDriverSlot(drivers[k]);
                    hasPlayer = 1;
                } else {
                    ai = out[k] = drivers[k] - CGameInfo::FUN_00405d70();
                    hasAi = 1;
                }
            }
            if (hasPlayer) {
                sprintf(CFrontend::m_stringDest, g_strRaceHasPlayer, i);
                puts(CFrontend::m_stringDest);
                pOut[-1] = (char)out[0];
                pOut[0] = (char)out[1];
                if (hasAi) {
                    sprintf(CFrontend::m_stringDest, g_strRaceHasAiDriver, i, ai);
                    puts(CFrontend::m_stringDest);
                    FUN_00456a40(i, ai);
                }
                pOut += 2;
            }
        }
        break;
    }
}

// FUNCTION: CMR2 0x00455ab0
int StageTiming_FUN_00455ab0(int iSplit)
{
    return g_stageSplitUnk0x00541f78[iSplit][1];
}

// FUNCTION: CMR2 0x00455ac0
int StageTiming_FUN_00455ac0(int iSplit, int iIndex)
{
    return g_stageSplitUnk0x00541f78[iSplit][iIndex];
}

// FUNCTION: CMR2 0x00455ae0
BYTE StageTiming_FUN_00455ae0(void)
{
    return g_unk0x00542604;
}

// Removes a driver from every split ranking, remembering in slot the
// driver's old rank index of the last split.
// match 11%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00455bc0
void FUN_00455bc0(int slot, int driver)
{
    int splits = FUN_004583a0();
    int split;
    int pos;
    char *pTimes;
    char *pNext;
    char *pRanks = (char *)&g_stageSplitRankings;

    for (split = 1; split <= splits; split++) {
        pTimes = pRanks + 0x140 + split * 16;
        pos = pTimes[driver - 0xa0];
        pNext = pTimes + 1;
        g_unk0x00541f90[slot] = pTimes[pos];
        for (; pos < g_stageSplitDriverCount[split] - 1; pos++) {
            pTimes[pos] = pNext[pos];
            pTimes[pos - 0x140] = pTimes[pos - 0x13f];
            g_stageSplitPositions[split][pTimes[pos - 0x140]] = (char)pos;
        }
        g_stageSplitDriverCount[split]--;
    }
}

// FUNCTION: CMR2 0x00455c70
int StageTiming_GetSplitDriverCount(int iSplit)
{
    return g_stageSplitDriverCount[iSplit];
}

// FUNCTION: CMR2 0x00455c80
int StageTiming_GetSplitPositionOfDriver(int iDriver, int iSplit)
{
    return g_stageSplitPositions[iSplit][g_stageDriverSlot[iDriver]];
}

// FUNCTION: CMR2 0x00455ca0
int StageTiming_GetCurrentSplitPositionOfDriver(int iDriver)
{
    int slot;
    int iSplit;

    slot = g_stageDriverSlot[iDriver];
    iSplit = GetStageSplitCount();
    return g_stageSplitPositions[iSplit][slot];
}

// FUNCTION: CMR2 0x00455cc0
int StageTiming_GetSplitDriverIDForPosition(int iPosition, int iSplit)
{
    if (iPosition >= g_stageSplitDriverCount[iSplit])
        return -1;
    return g_stageSplitDriverIndices[iSplit][iPosition];
}

// FUNCTION: CMR2 0x00455d40
int StageTiming_GetSplitTimeForPosition(int iPosition, int iSplit)
{
    return ConvertRawTimeToCentiseconds(g_stageSplitTimesRaw[iSplit][g_stageSplitTimesRawDriverIx[iSplit][iPosition]]);
}

// FUNCTION: CMR2 0x00455db0
int StageTiming_GetCurrentSplitTimeForDriver(int iDriver)
{
    int iSplit;

    iSplit = GetStageSplitCount();
    int value = g_stageSplitTimesRaw[iSplit][g_stageSplitTimesRawDriverIx[iSplit][g_stageSplitPositions[iSplit][iDriver]]];
    return ConvertRawTimeToCentiseconds(value);
}

// FUNCTION: CMR2 0x00455de0
int StageTiming_GetDriverSlot(int iDriver)
{
    return g_stageDriverSlot[iDriver];
}

// match 59%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00455e60
void StageTiming_Reset(void)
{
    int iSplit;
    int i;

    memset(g_stageDriverSlot, 0, sizeof(g_stageDriverSlot));
    g_unk0x00541f98 = 16;
    for (iSplit = 0; iSplit < 10; iSplit++)
    {
        for (i = 0; i < 16; i++)
        {
            g_stageSplitDriverIndices[iSplit][i] = i;
            g_stageSplitPositions[iSplit][i] = i;
            g_stageSplitTimesRawDriverIx[iSplit][i] = i;
            g_stageSplitTimesRaw[iSplit][i] = 0;
        }
    }
}

// FUNCTION: CMR2 0x00455d10
void StageTiming_AddToOverall(void)
{
    int iSplit;

    iSplit = GetStageSplitCount();
    RallyTiming_AddStageTimes(g_stageSplitDriverIndices[iSplit], g_stageSplitTimesRawDriverIx[iSplit], g_stageSplitTimesRaw[iSplit]);
}

// FUNCTION: CMR2 0x00455df0
void StageTiming_GetSplitTimesForPositions(int iPosition1, int iPosition2, int *piTime1, int *piTime2)
{
    int iSplit;
    int iSlot1;
    int iSlot2;

    iSplit = GetStageSplitCount();
    iSlot1 = iPosition1 - CGameInfo::FUN_00405d70();
    iSlot2 = iPosition2 - CGameInfo::FUN_00405d70();
    int position = g_stageSplitPositions[iSplit][iSlot1];
    int time = StageTiming_GetSplitTimeForPosition(position, iSplit);
    *piTime1 = time;
    position = g_stageSplitPositions[iSplit][iSlot2];
    time = StageTiming_GetSplitTimeForPosition(position, iSplit);
    *piTime2 = time;
}

// match 22%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00456250
void StageTiming_RebuildSplitPositions(void)
{
    int aiDriverForSlot[16];
    char *pIndices;
    int iSplit;
    int s;
    int i;
    int d;

    iSplit = GetStageSplitCount();
    for (i = 0; i < g_unk0x00541f98; i++)
    {
        pIndices = g_stageSplitDriverIndices[iSplit];
        d = pIndices[i];
        aiDriverForSlot[g_stageSplitTimesRawDriverIx[iSplit][i]] = d;
        g_stageSplitPositions[iSplit][d] = i;
    }
    for (i = g_unk0x00541f98; i < 16; i++)
    {
        g_stageSplitDriverIndices[iSplit][i] = i;
        g_stageSplitPositions[iSplit][i] = i;
    }
    for (s = 1; s < iSplit; s++)
    {
        pIndices = g_stageSplitDriverIndices[s];
        for (i = 0; i < g_unk0x00541f98; i++)
        {
            d = aiDriverForSlot[g_stageSplitTimesRawDriverIx[s][i]];
            pIndices[i] = d;
            g_stageSplitPositions[s][d] = i;
        }
        for (i = g_unk0x00541f98; i < 16; i++)
        {
            pIndices[i] = i;
            g_stageSplitPositions[s][i] = i;
        }
    }
}

BYTE RallyData_FUN_004069a0(void);
unsigned int RallyData_FUN_00407e70(void);
char FUN_00420190(void);
unsigned int RallyData_FUN_00407ea0(void);
unsigned int FUN_00409cb0(int index);

// Builds the per-split interpolation factors between the first and last split
// time of the stage.
// FUNCTION: CMR2 0x00456330
void FUN_00456330(int *param_1)
{
    int local_20;
    int local_24;
    int iVar1;
    int iVar2;
    int iVar3;
    int iVar4;
    int iVar5;
    int iVar6;
    int iVar7;
    unsigned int local_14;
    unsigned int local_c;
    unsigned int local_8;
    unsigned int uVar7;
    int *piVar8;

    iVar1 = GetStageSplitCount();
    for (iVar5 = 1; iVar5 <= iVar1; iVar5++) {
        if (param_1[0x28] >= FUN_004583b0(iVar5 - 1) && param_1[0x28] < FUN_004583b0(iVar5))
            local_20 = iVar5;
        if (param_1[0x29] > FUN_004583b0(iVar5 - 1) && param_1[0x29] <= FUN_004583b0(iVar5))
            local_24 = iVar5;
    }
    iVar5 = param_1[0x29];
    iVar2 = param_1[0x28];
    local_14 = 0;
    *param_1 = 0;
    iVar7 = iVar5 - iVar2;
    iVar6 = 1;
    if (iVar1 >= 1) {
        piVar8 = param_1 + 0xb;
        do {
            iVar3 = FUN_004583b0(iVar6);
            iVar4 = FUN_004583b0(iVar6 - 1);
            uVar7 = iVar3 - iVar4;
            if (iVar6 < local_20)
                local_c = uVar7;
            if (iVar6 == local_20)
                local_c = param_1[0x28] - FUN_004583b0(iVar6 - 1);
            if (iVar6 > local_20)
                local_c = 0;
            if (iVar6 < local_24)
                local_8 = 0;
            if (iVar6 == local_24)
                local_8 = FUN_004583b0(iVar6) - param_1[0x29];
            if (iVar6 > local_24)
                local_8 = uVar7;
            *piVar8 = FixDiv(local_c, uVar7);
            iVar3 = FixDiv(local_8, uVar7);
            piVar8[0x14] = iVar3;
            piVar8[10] = (0x10000 - *piVar8) - iVar3;
            local_14 = local_14 + ((uVar7 - local_8) - local_c);
            if (iVar7 == 0)
                piVar8[-10] = 0;
            else
                piVar8[-10] = FixDiv(local_14, iVar7);
            piVar8 = piVar8 + 1;
            iVar6 = iVar6 + 1;
        } while (iVar6 <= iVar1);
    }
}

// Number of cars in the stage (players, ghost and network players).
// FUNCTION: CMR2 0x00456ca0
unsigned int FUN_00456ca0(void)
{
    unsigned int count;
    int i;

    if ((BYTE)RallyData_FUN_00407e70() && !CGameInfo::FUN_00405e00())
        count = (BYTE)RallyDataState() + RallyData_FUN_004069a0();
    else
        count = (BYTE)FUN_00420190();
    if ((BYTE)RallyData_FUN_00407ea0() && (BYTE)CGameInfo::FUN_00406310())
        count++;
    if (CGameInfo::FUN_00405e00() && CGameInfo::FUN_00405d80() != 10) {
        for (i = 0; i < 7; i++) {
            if ((BYTE)FUN_00409cb0(i))
                count++;
        }
    }
    return count;
}

// Sets every vertex of a mesh-backed scene node to a shade of grey.
// FUNCTION: CMR2 0x00456d20
void StageTiming_FUN_00456d20(SceneNode *pNode, BYTE colour)
{
    BYTE rgb[4];

    rgb[0] = colour;
    rgb[1] = colour;
    rgb[2] = colour;
    rgb[3] = 0;
    if (pNode->type == 0 && pNode->pObject != 0)
        Mesh_SetVertexColours((Mesh *)pNode->pObject, rgb);
}

// FUNCTION: CMR2 0x00456d60
void FUN_00456d60(SceneNode *pNode, BYTE colour)
{
    for (; pNode != NULL; pNode = pNode->pNext) {
        StageTiming_FUN_00456d20(pNode, colour);
        if (pNode->pFirstChild != NULL)
            FUN_00456d60(pNode->pFirstChild, colour);
    }
}

// GLOBAL: CMR2 0x00588e80
void *g_unk0x00588e80[8];
extern void *g_unk0x00588ea0[8];
// GLOBAL: CMR2 0x00588d3c
int g_unk0x00588d3c;
// GLOBAL: CMR2 0x00588d14
int g_unk0x00588d14;

// Releases the eight stage buffers.
// FUNCTION: CMR2 0x0046c500
BOOL FUN_0046c500(void)
{
    void **p;

    p = g_unk0x00588e80;
    do {
        if (*p != NULL) {
            CFileBuffer::FreeGenericFileBuffer(*p);
            *p = NULL;
        }
        p++;
    } while ((int)p < (int)g_unk0x00588ea0);
    g_unk0x00588d3c = 0;
    g_unk0x00588d14 = 0;
    return TRUE;
}

// 12-byte entry of the table released by FUN_00456b70.
struct Unk0x542ae8 {
    void *pBuffer;
    void *field_0x4;
    void *field_0x8;
};

// GLOBAL: CMR2 0x00542ae8
Unk0x542ae8 g_unk0x00542ae8[32];
extern int g_unk0x00542c68;

// match 68%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00456b70
bool FUN_00456b70(void)
{
    Unk0x542ae8 *p;
    int j;

    p = g_unk0x00542ae8;
    do {
        j = 2;
        do {
            if (p->pBuffer != NULL) {
                CFileBuffer::FreeGenericFileBuffer(p->pBuffer);
                p->pBuffer = NULL;
            }
            p->pBuffer = NULL;
            p->field_0x4 = NULL;
            p->field_0x8 = NULL;
            p++;
        } while (--j != 0);
    } while ((int)p < (int)&g_unk0x00542c68);
    return true;
}

// Stage timing of one car, 0x88 bytes
struct CarStageTiming {
    int startTime;          // 0x00
    int field_0x4[9];       // 0x04
    int splits[9];          // 0x28
    int splitTimes[12];     // 0x4c
    int lastTime;           // 0x7c
    char field_0x80;        // 0x80
    char field_0x81;        // 0x81
    char field_0x82;        // 0x82
    BYTE field_0x83;        // 0x83
    BYTE field_0x84;        // 0x84
    BYTE pad_0x85[3];
};

// GLOBAL: CMR2 0x0053d1a0
short g_unk0x0053d1a0;
// GLOBAL: CMR2 0x0053d1a2
short g_unk0x0053d1a2;
// GLOBAL: CMR2 0x0053d1a4
char g_unk0x0053d1a4[2];
// GLOBAL: CMR2 0x0053d1a6
BYTE g_unk0x0053d1a6;
// GLOBAL: CMR2 0x0053d1a7
BYTE g_unk0x0053d1a7;
// GLOBAL: CMR2 0x0053d1a8
char g_unk0x0053d1a8;
// GLOBAL: CMR2 0x0053d1b0
int g_unk0x0053d1b0;
// GLOBAL: CMR2 0x0053d1b4
int g_unk0x0053d1b4;
// GLOBAL: CMR2 0x0053d1b8
int g_unk0x0053d1b8[8];
// GLOBAL: CMR2 0x0053d1d8
BYTE g_unk0x0053d1d8;
// GLOBAL: CMR2 0x0053d1da
BYTE g_unk0x0053d1da[8];
// GLOBAL: CMR2 0x0053d968
CarStageTiming g_carStageTiming[8];
// GLOBAL: CMR2 0x0053dda8
char g_unk0x0053dda8[8];
// GLOBAL: CMR2 0x0053e17c
char g_unk0x0053e17c[8];
// GLOBAL: CMR2 0x0053e184
char g_unk0x0053e184[8];
// GLOBAL: CMR2 0x0053e18c
BYTE g_unk0x0053e18c;

// Marks a timed driver slot as active and queues it when the selected rally
// state exposes the slot in the ordering table.
// FUNCTION: CMR2 0x00448cf0
void StageTiming_QueueDriverSlot(int index)
{
    g_unk0x0053d1da[index] = 1;
    g_unk0x0053d1b8[index] = *(volatile int *)&g_unk0x0053d1b0;
    if ((BYTE)RallyData_GetFlag24() != 0 || (BYTE)RallyData_GetFlag25() != 0) {
        g_unk0x0053e17c[(char)*(volatile BYTE *)&g_unk0x0053e18c] = (char)index;
        g_unk0x0053e184[index] = (char)*(volatile BYTE *)&g_unk0x0053e18c;
        g_unk0x0053e18c = *(volatile BYTE *)&g_unk0x0053e18c + 1;
    }
}
// GLOBAL: CMR2 0x0053e18d
BYTE g_unk0x0053e18d[2];
// GLOBAL: CMR2 0x0053e18f
BYTE g_unk0x0053e18f;
// GLOBAL: CMR2 0x0053e190
int g_unk0x0053e190[82];  // 9 splits per car, up to the menu at 0x53e2d8

// FUNCTION: CMR2 0x00448100
void FUN_00448100(void)
{
    g_unk0x0053d1b4 = 0;
    g_unk0x0053d1b0 = 0;
}

// FUNCTION: CMR2 0x00448110
int FUN_00448110(void)
{
    return g_unk0x0053d1b0;
}

// FUNCTION: CMR2 0x004481b0
BYTE FUN_004481b0(void)
{
    return g_unk0x0053d1d8;
}

// FUNCTION: CMR2 0x004481c0
int FUN_004481c0(int car)
{
    return g_carStageTiming[car].field_0x81;
}

// FUNCTION: CMR2 0x004481e0
int FUN_004481e0(int index)
{
    return g_unk0x0053dda8[index];
}

// FUNCTION: CMR2 0x004481f0
int FUN_004481f0(int car, int index)
{
    return g_carStageTiming[car].splitTimes[index];
}

// FUNCTION: CMR2 0x00448210
int FUN_00448210(int car)
{
    int time = g_unk0x0053d1b0 - g_carStageTiming[car].lastTime;
    if (time >= 360000)
        time = 359999;
    return time;
}

// FUNCTION: CMR2 0x00448240
int FUN_00448240(int car, int index)
{
    return g_carStageTiming[car].splits[index];
}

// Split times of each car per checkpoint group (8 x 12 x 5).
// GLOBAL: CMR2 0x0053d1e8
int g_unk0x0053d1e8[8][12][5];

int FUN_00458330(int index);
int FUN_00458350(int index);
int FUN_00458370(int index);

// Leader of each split (row per checkpoint group and split, 8 bytes each).
// GLOBAL: CMR2 0x0053de1c
char g_unk0x0053de1c[108][8];

// Time of the split leader at the given split.
// FUNCTION: CMR2 0x00448260
int FUN_00448260(int index)
{
    int group;
    int split;

    if ((BYTE)RallyData_GetFlag24()) {
        group = FUN_00458330(index);
        split = FUN_00458350(index);
        return g_unk0x0053d1e8[g_unk0x0053de1c[group * 9 + split][0]][group][split];
    }
    split = FUN_00458370(index);
    return g_carStageTiming[g_unk0x0053de1c[split][0]].field_0x4[split];
}

// FUNCTION: CMR2 0x004482d0
int FUN_004482d0(int index, int car)
{
    if ((BYTE)RallyData_GetFlag24()) {
        int group = FUN_00458330(index);
        int split = FUN_00458350(index);

        return g_unk0x0053d1e8[car][group][split];
    }
    return g_carStageTiming[car].field_0x4[FUN_00458370(index)];
}

// FUNCTION: CMR2 0x00448330
int FUN_00448330(int car)
{
    return g_carStageTiming[car].field_0x82;
}

// FUNCTION: CMR2 0x00448350
int FUN_00448350(int car)
{
    return g_carStageTiming[car].field_0x80;
}

// FUNCTION: CMR2 0x00448370
BYTE FUN_00448370(int car)
{
    return g_carStageTiming[car].field_0x83;
}

// FUNCTION: CMR2 0x00448390
int FUN_00448390(int index)
{
    if (index >= 0 && index < 8)
        return g_unk0x0053e17c[index];
    return -1;
}

// FUNCTION: CMR2 0x004483b0
int FUN_004483b0(int index)
{
    return g_unk0x0053e184[index];
}

// FUNCTION: CMR2 0x00448620
void FUN_00448620(void)
{
    FUN_0040ccd0(g_unk0x0053e17c, g_unk0x0053d1b8);
}

// FUNCTION: CMR2 0x00448670
int FUN_00448670(void)
{
    return (char)g_unk0x0053e18c;
}

int FUN_00458310(int index);

// Sort order of two cars by checkpoint group, then start time.
// FUNCTION: CMR2 0x004486a0
int FUN_004486a0(int a, int b)
{
    int result = 0;

    if (FUN_00458310(a) == FUN_00458310(b)) {
        if (g_carStageTiming[a].startTime >= g_carStageTiming[b].startTime)
            return -1;
    } else {
        if (FUN_00458310(a) > FUN_00458310(b))
            result = 1;
        if (FUN_00458310(a) < FUN_00458310(b))
            return -1;
    }
    return result;
}

// FUNCTION: CMR2 0x00448780
void FUN_00448780(int car)
{
    g_carStageTiming[car].startTime = g_unk0x0053d1b0;
}

unsigned int RallyData_FUN_004082b0(void);
unsigned int RallyData_FUN_004082d0(void);
void StageTiming_QueueDriverSlot(int index);

// In a two-player split race, queues both driver slots once the leader's lead
// exceeds the allowed gap.
// FUNCTION: CMR2 0x00448bf0
void FUN_00448bf0(int slot)
{
    if (RallyData_FUN_004082b0() == 1 && g_unk0x0053d1a6 != 0 && slot != g_unk0x0053d1a8) {
        if (g_unk0x0053d1a2 > (int)(RallyData_FUN_004082d0() * 100)) {
            g_unk0x0053d1a7 = 1;
            StageTiming_QueueDriverSlot(g_unk0x0053d1a8);
            StageTiming_QueueDriverSlot(1 - g_unk0x0053d1a8);
        }
    }
}

// FUNCTION: CMR2 0x00448c60
int FUN_00448c60(int index)
{
    return g_unk0x0053d1a4[index];
}

// FUNCTION: CMR2 0x00448c70
int FUN_00448c70(void)
{
    return g_unk0x0053d1a2;
}

// FUNCTION: CMR2 0x00448c80
int FUN_00448c80(void)
{
    return g_unk0x0053d1a8;
}

// FUNCTION: CMR2 0x00448c90
BYTE FUN_00448c90(void)
{
    return g_unk0x0053d1a6;
}

// FUNCTION: CMR2 0x00448ca0
BYTE FUN_00448ca0(void)
{
    return g_unk0x0053d1a7;
}

// FUNCTION: CMR2 0x00448cb0
BYTE FUN_00448cb0(int index)
{
    return g_unk0x0053e18d[index];
}

// FUNCTION: CMR2 0x00448cc0
BYTE FUN_00448cc0(void)
{
    return g_unk0x0053e18f;
}

// FUNCTION: CMR2 0x00448cd0
BYTE FUN_00448cd0(int car)
{
    return g_carStageTiming[car].field_0x84;
}

// FUNCTION: CMR2 0x00448630
void FUN_00448630(int index)
{
    g_unk0x0053d1da[index] = 1;
    g_unk0x0053e190[GetStageSplitCount() + index * 9] = g_unk0x0053d1b0;
    g_unk0x0053d1b8[index] = g_unk0x0053d1b0 + 0x4650;
    g_unk0x0053e18c++;
}

// FUNCTION: CMR2 0x004483c0
int FUN_004483c0(int index)
{
    if (index == -1)
        return -1;
    return g_unk0x0053d1b8[index];
}

// FUNCTION: CMR2 0x00448680
unsigned int FUN_00448680(int index, int split)
{
    return g_unk0x0053e190[index * 9 + split];
}

// Registers the mesh-backed node in the per-slot tables.
// FUNCTION: CMR2 0x00466e90
void FUN_00466e90(SceneNode *pNode, int *pSlot)
{
    int index;
    void *pObject;
    int count;

    index = (pNode->flags & 0xff) - 5;
    pObject = pNode->pObject;
    if (pObject == NULL)
        return;
    if (pNode->type != 0)
        return;
    pSlot[index + 0xf] = (int)pNode;
    pSlot[index] = (int)pObject;
    count = Mesh_GetField0x10((Mesh *)pObject);
    pSlot[index + 0x108] = count;
    if (pSlot[index] == 0)
        return;
    if (count <= 0)
        return;
    pSlot[0x117]++;
}

// GLOBAL: CMR2 0x00592734
void *g_unk0x00592734;
// GLOBAL: CMR2 0x00592738
int g_unk0x00592738;
// GLOBAL: CMR2 0x0059273c
int g_unk0x0059273c;
// GLOBAL: CMR2 0x00592740
int g_unk0x00592740;

// Release callback of FUN_00494b50.
// FUNCTION: CMR2 0x00494b10
int FUN_00494b10(void)
{
    if (g_unk0x00592734 != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x00592734);
        g_unk0x00592734 = NULL;
    }
    g_unk0x00592738 = 0;
    g_unk0x0059273c = 0;
    g_unk0x00592740 = 0;
    return 1;
}

// FUNCTION: CMR2 0x00494b50
void FUN_00494b50(int count)
{
    BYTE *pBuffer;

    pBuffer = (BYTE *)CFileBuffer::AllocateLockedBuffer(count * 0x2a4);
    g_unk0x00592734 = pBuffer;
    memset(pBuffer, 0, count * 0x2a4);
    g_unk0x00592738 = count;
    CGame::RegisterCallback(FUN_00494b10, NULL);
}

// GLOBAL: CMR2 0x00547ac8
void *g_unk0x00547ac8;
// GLOBAL: CMR2 0x00543ecc
void *g_unk0x00543ecc;
// Billboards of the weather effects.
// GLOBAL: CMR2 0x00543ed0
BillboardDef g_unk0x00543ed0;
// GLOBAL: CMR2 0x00543eb8
void *g_unk0x00543eb8;
extern BYTE g_unk0x00547acc;
// Release callback of FUN_0045e5b0.
// FUNCTION: CMR2 0x0045e560
int FUN_0045e560(void)
{
    if (g_unk0x00547ac8 != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x00547ac8);
        g_unk0x00547ac8 = NULL;
    }
    if (g_unk0x00543ecc != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x00543ecc);
        g_unk0x00543ecc = NULL;
    }
    if (g_unk0x00543eb8 != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x00543eb8);
        g_unk0x00543eb8 = NULL;
    }
    g_unk0x00547acc = 0;
    return 1;
}

// FUNCTION: CMR2 0x0045e5b0
void FUN_0045e5b0(int count)
{
    g_unk0x00547ac8 = CFileBuffer::AllocateLockedBuffer(count * 376);
    g_unk0x00543ecc = CFileBuffer::AllocateLockedBuffer(count * 12);
    g_unk0x00543eb8 = CFileBuffer::AllocateLockedBuffer(count * 44);
    g_unk0x00547acc = (BYTE)count;
    CGame::RegisterCallback(FUN_0045e560, NULL);
}

// FUNCTION: CMR2 0x00465ec0
void FUN_00465ec0(SceneNode *pNode, int alpha, BYTE checkFlag)
{
    BYTE rgb[4];
    Mesh *pMesh;

    rgb[0] = 0x80;
    rgb[1] = 0x80;
    rgb[2] = 0x80;
    rgb[3] = 0;
    if (pNode->type != 0)
        return;
    pMesh = (Mesh *)pNode->pObject;
    if (pMesh == NULL)
        return;
    *(int *)((char *)pMesh + 0x30) |= 0x40008;
    Mesh_SetVertexAlpha(pMesh, (BYTE)alpha);
    if (checkFlag != 0)
        Mesh_SetVertexColours(pMesh, rgb);
}

// match 69%: the whole FixMul/FixDiv chain matches; the remaining diff is MSVC6's stack-slot numbering
// (it spills into the dead parameter homes and adds the two `n` terms separately) plus one store order.
// Interpolates 16 fixed-point samples per row from the car's 0x7c table into
// pOut, weighting the two corner rows by the car's 0xa8/0xac offsets.
// FUNCTION: CMR2 0x004564d0
void FUN_004564d0(int pCar, int *pOut)
{
    int f;
    int n;
    int rf;
    int rn;
    int rc;
    int count;
    int off;
    int k;
    int *pIn;
    int *p;
    int sum;
    int t;
    int v1;
    int v2;
    int v3;
    int v4;
    int v5;
    int v6;

    f = *(int *)(pCar + 0xac);
    n = *(int *)(pCar + 0xa8);
    rf = f - n;
    rn = 0x10000 - n;
    rc = 0x10000 - f;
    count = GetStageSplitCount();
    if (count >= 1) {
        off = 0x310;
        pIn = (int *)(pCar + 0x7c);
        do {
            k = 0x10;
            p = pOut;
            do {
                v1 = FixMul(FixMul(pIn[-0x14], rn), *(int *)(g_unk0x0054241c + off - 0x280));
                v2 = FixMul(FixMul(pIn[-0x14], n), *(int *)(g_unk0x0054241c + off));
                v3 = FixMul(FixMul(pIn[0], rc), *(int *)(g_unk0x0054241c + off - 0x280));
                v4 = FixMul(FixMul(pIn[0], f), *(int *)(g_unk0x0054241c + off));
                v5 = FixMul(pIn[-10], *(int *)(g_unk0x0054241c + off - 0x280));
                v6 = FixMul(pIn[-10], *(int *)(g_unk0x0054241c + off));
                sum = n + FixMul(rf, pIn[-0x1e]) + n + FixMul(rf, pIn[-0x1f]);
                t = FixDiv(sum, 0x20000);
                *p = FixMul(0x10000 - t, v6) + FixMul(t, v5) + v4 + v3 + v2 + v1;
                k--;
                p++;
                off += 4;
            } while (k != 0);
            pIn++;
            count--;
            pOut += 0x10;
        } while (count != 0);
    }
}

// match 41%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00456960
void FUN_00456960(int *pDeltas)
{
    int *pRaw;
    int count;
    int i, j;

    pRaw = (int *)g_stageSplitTimesRaw;
    count = GetStageSplitCount();
    if (count >= 0) {
        for (i = 0; i < (count + 1) * 16; i++)
            pRaw[i] = 0;
    }
    if (count >= 1) {
        for (i = 0; i < count; i++) {
            for (j = 0; j < 16; j++)
                pRaw[i * 16 + j + 16] = pRaw[i * 16 + j] + pDeltas[i * 16 + j];
        }
    }
}

// GLOBAL: CMR2 0x00590d7c
void *g_unk0x00590d7c[4];
// GLOBAL: CMR2 0x00590c64
int g_unk0x00590c64;
// GLOBAL: CMR2 0x00590c6c
void **g_unk0x00590c6c;
// Release callback: frees the four per-car node tables and the lists.
// FUNCTION: CMR2 0x00480870
int FUN_00480870(void)
{
    void **p;
    int i;

    p = g_unk0x00590d7c;
    do {
        if (*p != NULL) {
            CFileBuffer::FreeGenericFileBuffer(*p);
            *p = NULL;
        }
        p++;
    } while ((int)p < (int)&g_unk0x00590d7c[4]);
    if (g_unk0x00590c6c != NULL) {
        for (i = 0; i < g_unk0x00590c64; i++) {
            if (g_unk0x00590c6c[i] != NULL) {
                CFileBuffer::FreeGenericFileBuffer(g_unk0x00590c6c[i]);
                g_unk0x00590c6c[i] = NULL;
            }
        }
        CFileBuffer::FreeGenericFileBuffer(g_unk0x00590c6c);
        g_unk0x00590c6c = NULL;
    }
    g_unk0x00590c64 = 0;
    return 1;
}

// FUNCTION: CMR2 0x00480900
void FUN_00480900(int count)
{
    int i;

    for (i = 0; i < 4; i++)
        g_unk0x00590d7c[i] = CFileBuffer::AllocateLockedBuffer(count * 416);
    g_unk0x00590c6c = (void **)CFileBuffer::AllocateLockedBuffer(count * 4);
    for (i = 0; i < count; i++)
        g_unk0x00590c6c[i] = CFileBuffer::AllocateLockedBuffer(0xb4);
    g_unk0x00590c64 = count;
    CGame::RegisterCallback(FUN_00480870, NULL);
}

struct Unk0x00539278 {
    int field_0x0;
    int field_0x4;
    int field_0x8;
    int field_0xc;
    int field_0x10;
    int field_0x14;
    int field_0x18;
    int field_0x1c;
    int field_0x20;
    int field_0x24;
    int field_0x28;
    signed char field_0x2c;         // device index, < 0 when none
    BYTE pad_0x2d[3];
    int field_0x30;
    int field_0x34;                 // slot in use
};

// Force-feedback state of the two local players.
// GLOBAL: CMR2 0x00539200
Unk0x00539278 g_forceFeedbackSlots[2];
// GLOBAL: CMR2 0x00539278
Unk0x00539278 *g_unk0x00539278;

// match 75%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004246a0
void FUN_004246a0(void)
{
    int i = 0;
    do {
        *(int *)(i + (int)g_unk0x00539278) =
            *(int *)(i + (int)g_unk0x00539278 + 0xc);
        i += 4;
    } while (i < 0xc);
}

// Re-applies the stored force-feedback values to the selected device.
// match 83%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00424560
void FUN_00424560(void)
{
    if (g_unk0x00539278->field_0x10 != g_unk0x00539278->field_0x4)
        CInput::SetConditionCoefficient(1, g_unk0x00539278->field_0x10,
                                        g_unk0x00539278->field_0x2c);
    if (g_unk0x00539278->field_0xc != g_unk0x00539278->field_0x0)
        CInput::SetConditionCoefficient(0, g_unk0x00539278->field_0xc,
                                        g_unk0x00539278->field_0x2c);
    if (g_unk0x00539278->field_0x14 != g_unk0x00539278->field_0x8) {
        if (g_unk0x00539278->field_0x14 < 0)
            CInput::SetEffectGainAndDirection(2, -g_unk0x00539278->field_0x14, 0x2328,
                                              g_unk0x00539278->field_0x2c);
        else
            CInput::SetEffectGainAndDirection(2, g_unk0x00539278->field_0x14, 0x6978,
                                              g_unk0x00539278->field_0x2c);
    }
}

// Resets the force values of every idle slot with a device and marks it active.
// FUNCTION: CMR2 0x004245e0
void FUN_004245e0(void)
{
    Unk0x00539278 *p;
    int i;

    for (p = g_forceFeedbackSlots; &p->field_0x2c < &g_forceFeedbackSlots[2].field_0x2c; p++) {
        g_unk0x00539278 = p;
        if (g_unk0x00539278->field_0x34 == 0) {
            if (g_unk0x00539278->field_0x2c >= 0) {
                g_unk0x00539278->field_0x28 = 0;
                g_unk0x00539278->field_0x24 = 0;
                for (i = 0; i < 3; i++) {
                    (&g_unk0x00539278->field_0x0)[i] = 0;
                    (&g_unk0x00539278->field_0xc)[i] = 0;
                }
            }
            g_unk0x00539278->field_0x34 = 1;
        }
    }
}

// Stops the forces of every active slot and releases the slots.
// FUNCTION: CMR2 0x00424640
void FUN_00424640(void)
{
    Unk0x00539278 *p;
    int i;

    for (p = g_forceFeedbackSlots; &p->field_0x2c < &g_forceFeedbackSlots[2].field_0x2c; p++) {
        g_unk0x00539278 = p;
        if (g_unk0x00539278->field_0x34 != 0) {
            if (g_unk0x00539278->field_0x2c >= 0) {
                FUN_004246a0();
                for (i = 0; i < 3; i++)
                    (&g_unk0x00539278->field_0xc)[i] = 0;
                FUN_00424560();
            }
            g_unk0x00539278->field_0x34 = 0;
        }
    }
}

// Stops the forces of every active slot.
// FUNCTION: CMR2 0x004246c0
void FUN_004246c0(void)
{
    Unk0x00539278 *p;
    int i;

    for (p = g_forceFeedbackSlots; &p->field_0x2c < &g_forceFeedbackSlots[2].field_0x2c; p++) {
        g_unk0x00539278 = p;
        if (g_unk0x00539278->field_0x34 != 0 && g_unk0x00539278->field_0x2c >= 0) {
            FUN_004246a0();
            for (i = 0; i < 3; i++)
                (&g_unk0x00539278->field_0xc)[i] = 0;
            FUN_00424560();
        }
    }
}

void FUN_00424360(void);
void FUN_004247a0(void);
void FUN_004248a0(void);
void FUN_00424af0(void);
void FUN_00424c00(void);

extern int g_unk0x00539270[2];
extern BYTE *g_unk0x0053937c;

// Selects the force-feedback slot and car of a player, copies the car's
// centring state into the shared road-rumble values and re-applies the slot's
// forces when it is in use.
// FUNCTION: CMR2 0x00424710
void FUN_00424710(int view)
{
    g_unk0x00539278 = &g_forceFeedbackSlots[view];
    g_unk0x0053937c = (BYTE *)Car_Get(view);
    if (*(int *)(g_unk0x0053937c + 0xb74) != 0) {
        g_unk0x00539270[0] = *(int *)(g_unk0x0053937c + 0xbac);
        g_unk0x00539270[1] = *(int *)(g_unk0x0053937c + 0xbb0);
    } else {
        g_unk0x00539270[0] = 0;
        g_unk0x00539270[1] = 0;
    }
    if (g_unk0x00539278->field_0x34 != 0) {
        FUN_004246a0();
        FUN_004247a0();
        FUN_00424af0();
        FUN_00424360();
        FUN_004248a0();
        FUN_00424c00();
        FUN_00424560();
    }
}

// Fades the two impulse forces of the current slot out.
// FUNCTION: CMR2 0x00424c00
void FUN_00424c00(void)
{
    if (g_unk0x00539278->field_0x24 > 0) {
        g_unk0x00539278->field_0x24 -= 0x1999;
        if (g_unk0x00539278->field_0x24 < 0)
            g_unk0x00539278->field_0x24 = 0;
    }
    if (g_unk0x00539278->field_0x28 > 0) {
        g_unk0x00539278->field_0x28 -= 0x1999;
        if (g_unk0x00539278->field_0x28 < 0)
            g_unk0x00539278->field_0x28 = 0;
    }
}


// GLOBAL: CMR2 0x00588a90
int g_unk0x00588a90;
// GLOBAL: CMR2 0x00588b94
BYTE *g_unk0x00588b94;
// GLOBAL: CMR2 0x00588b98
BYTE *g_unk0x00588b98;
// GLOBAL: CMR2 0x00588b9c
int *g_unk0x00588b9c;
// GLOBAL: CMR2 0x00588ba0
int *g_unk0x00588ba0;
// GLOBAL: CMR2 0x00588990
int *g_unk0x00588990[8];

// Twelve collision hull vertices occupy the space immediately after the slot pointers.
// GLOBAL: CMR2 0x005889b0
FixVector g_stageDeformHull[12];
// GLOBAL: CMR2 0x00588a40
FixVector g_stageDeformOffset;
// GLOBAL: CMR2 0x00588a4c
FixVector g_stageDeformNormal;
// GLOBAL: CMR2 0x00588a58
FixVector g_stageDeformImpact;
// GLOBAL: CMR2 0x00588a64
int g_stageDeformSpeed;
// GLOBAL: CMR2 0x00588a68
int g_stageDeformStrength;
// GLOBAL: CMR2 0x00588a6c
int g_stageDeformMode;
// GLOBAL: CMR2 0x00588a70
Car *g_stageDeformCar;
// GLOBAL: CMR2 0x00588a74
int g_stageDeformRadius;
// GLOBAL: CMR2 0x00588a78
int g_stageDeformFalloff;
// GLOBAL: CMR2 0x00588a7c
int g_stageDeformScale;

// Reads entry `index` of the 0x4b0-byte tail of a car's 0x4d0-byte record.
// FUNCTION: CMR2 0x00469bc0
int FUN_00469bc0(void *pCar, int index)
{
    return *(int *)(g_unk0x00588b94 + (index + *(char *)((BYTE *)pCar + 0xb1a) * 0x134) * 4 + 0x4b0);
}

// Release callback of FUN_004667c0: frees the per-record part tables
// (0x588b9c), the per-record buffers (0x588ba0) and the record arrays.
// FUNCTION: CMR2 0x00466680
int FUN_00466680(void)
{
    void ***pParts;
    void **pBuffers;
    int i;
    int j;

    if (g_unk0x00588b9c != NULL) {
        pParts = (void ***)g_unk0x00588b9c;
        for (i = 0; i < g_unk0x00588a90; i++) {
            if (((void ***)g_unk0x00588b9c)[i] != NULL) {
                for (j = 0; j < *(int *)(g_unk0x00588b94 + i * 0x4d0 + 0x45c); j++) {
                    if (((void ***)g_unk0x00588b9c)[i][j] != NULL) {
                        CFileBuffer::FreeGenericFileBuffer(((void ***)g_unk0x00588b9c)[i][j]);
                        ((void ***)g_unk0x00588b9c)[i][j] = NULL;
                    }
                }
                CFileBuffer::FreeGenericFileBuffer(((void ***)g_unk0x00588b9c)[i]);
                ((void ***)g_unk0x00588b9c)[i] = NULL;
            }
        }
        CFileBuffer::FreeGenericFileBuffer(g_unk0x00588b9c);
        g_unk0x00588b9c = NULL;
    }
    if (g_unk0x00588ba0 != NULL) {
        pBuffers = (void **)g_unk0x00588ba0;
        for (i = 0; i < g_unk0x00588a90; i++) {
            if (pBuffers[i] != NULL) {
                CFileBuffer::FreeGenericFileBuffer(pBuffers[i]);
                ((void **)g_unk0x00588ba0)[i] = NULL;
                pBuffers = (void **)g_unk0x00588ba0;
            }
        }
        CFileBuffer::FreeGenericFileBuffer(pBuffers);
        g_unk0x00588ba0 = NULL;
    }
    if (g_unk0x00588b94 != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x00588b94);
        g_unk0x00588b94 = NULL;
    }
    if (g_unk0x00588b98 != NULL) {
        CFileBuffer::FreeGenericFileBuffer(g_unk0x00588b98);
        g_unk0x00588b98 = NULL;
    }
    g_unk0x00588a90 = 0;
    return 1;
}

// FUNCTION: CMR2 0x004667c0
void FUN_004667c0(int count)
{
    int i;
    int offset;

    g_unk0x00588b94 = (BYTE *)CFileBuffer::AllocateLockedBuffer(count * 1232);
    g_unk0x00588b98 = (BYTE *)CFileBuffer::AllocateLockedBuffer(count * 656);
    g_unk0x00588b9c = (int *)CFileBuffer::AllocateLockedBuffer(count * 4);
    g_unk0x00588ba0 = (int *)CFileBuffer::AllocateLockedBuffer(count * 4);
    g_unk0x00588a90 = count;
    i = 0;
    if (i < g_unk0x00588a90) {
        offset = 0;
        do {
            g_unk0x00588ba0[i] = 0;
            g_unk0x00588b9c[i] = 0;
            g_unk0x00588990[i] = (int *)(g_unk0x00588b94 + offset);
            offset += 0x4d0;
            i++;
        } while (i < g_unk0x00588a90);
    }
    CGame::RegisterCallback(FUN_00466680, NULL);
}

// Sets up the whole force-feedback effect set (spring, damper, constant force)
// for the currently selected device and starts every effect.
// FUNCTION: CMR2 0x00424120
void FUN_00424120(void)
{
    int i;

    CInput::CreateSpringEffect(-1, 0x2710, 0, -1, g_unk0x00539278->field_0x2c);
    CInput::SetConditionCoefficient(0, 0, g_unk0x00539278->field_0x2c);
    CInput::CreateDamperEffect(-1, 0, 0, -1, g_unk0x00539278->field_0x2c);
    CInput::SetConditionCoefficient(1, 0, g_unk0x00539278->field_0x2c);
    CInput::CreateConstantForceEffect(-1, 0, 0x2710, 0, 0, 0, 0, -1,
                                      g_unk0x00539278->field_0x2c);
    CInput::SetEffectGain(2, 0, g_unk0x00539278->field_0x2c);
    for (i = 0; i < 3; i++)
        CInput::StartForceFeedbackEffect(i, g_unk0x00539278->field_0x2c);
}

// GLOBAL: CMR2 0x00591af0
int g_unk0x00591af0;
// GLOBAL: CMR2 0x00591af8
int g_unk0x00591af8;
// GLOBAL: CMR2 0x00591afc
int g_unk0x00591afc;
// GLOBAL: CMR2 0x00591b00
int g_unk0x00591b00[5];
// GLOBAL: CMR2 0x00591b14
int g_unk0x00591b14;
// GLOBAL: CMR2 0x00591b18
int g_unk0x00591b18;
// GLOBAL: CMR2 0x00591b1c
int g_unk0x00591b1c;
// GLOBAL: CMR2 0x00591b20
int g_unk0x00591b20;
// GLOBAL: CMR2 0x00591b24
int g_unk0x00591b24;
// GLOBAL: CMR2 0x00591b30
int g_unk0x00591b30;
// GLOBAL: CMR2 0x00591b34
int g_unk0x00591b34;
// GLOBAL: CMR2 0x00591b38
int g_unk0x00591b38[5];
// Reads entry index of the 8-byte table at g_unk0x00591af0.
// match 53%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00491790
void FUN_00491790(short index, short *pA, short *pB, short *pC, short *pD, unsigned short *pFlags)
{
    int offset = index * 8;

    *pA = *(short *)(offset + g_unk0x00591af0);
    *pB = *(short *)(offset + 2 + g_unk0x00591af0);
    *pC = *(short *)(offset + 4 + g_unk0x00591af0);
    *pFlags = *(BYTE *)(offset + 6 + g_unk0x00591af0) & 0x7f;
    *pD = 0;
}

// Release callback with nothing to free.
// FUNCTION: CMR2 0x004918c0
BYTE FUN_004918c0(void)
{
    return 1;
}

// Walks the serialised stage block, recording a pointer to every sub-table.
// match 71%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00490d50
void FUN_00490d50(BYTE *pData)
{
    BYTE *p;
    int i;

    g_unk0x00591b30 = (int)pData;
    g_unk0x00591af8 = (int)pData;
    p = pData + 8;
    for (i = 0; i < 5; i++) {
        g_unk0x00591b38[i] = *(short *)p;
        p += 2;
    }
    p += 2;
    for (i = 0; i < 5; i++) {
        g_unk0x00591b00[i] = (int)p;
        p += g_unk0x00591b38[i] * 8;
    }
    g_unk0x00591b24 = (int)p;
    p += 4;
    g_unk0x00591b14 = (int)p;
    i = *(unsigned short *)g_unk0x00591b24;
    g_unk0x00591afc = (int)(p + i * 12);
    p = (BYTE *)g_unk0x00591afc + 4;
    g_unk0x00591af0 = (int)p;
    i = *(unsigned short *)g_unk0x00591afc;
    p = (BYTE *)g_unk0x00591af0 + i * 8;
    g_unk0x00591b34 = (int)p;
    p += 4;
    g_unk0x00591b18 = (int)p;
    i = *(int *)g_unk0x00591b34;
    p = (BYTE *)g_unk0x00591b18 + i * 2;
    g_unk0x00591b20 = (int)p;
    p += 2;
    g_unk0x00591b1c = (int)p;
    CGame::RegisterCallback(FUN_004918c0, NULL);
}

// GLOBAL: CMR2 0x0053cdbc
int g_unk0x0053cdbc;

// FUNCTION: CMR2 0x00445a40
int FUN_00445a40(void)
{
    if (g_unk0x0053cdbc != 0) {
        g_unk0x0053cdbc = 0;
        return 1;
    }
    return 0;
}


extern int g_dashGearMarker[2];
extern int g_dashIdle[2];
extern int g_dashRev[2];

// match 14%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00445db0
void FUN_00445db0(void)
{
    g_dashIdle[0] = 0;
    g_dashGearMarker[0] = -1;
    g_dashIdle[1] = 0;
    g_dashGearMarker[1] = -1;
}

// FUNCTION: CMR2 0x00445dd0
int FUN_00445dd0(int index)
{
    if (index < 2)
        return g_dashRev[index];
    return 0;
}

// GLOBAL: CMR2 0x00542420
int g_unk0x00542420[2];
// GLOBAL: CMR2 0x00542528
BYTE g_unk0x00542528[0xd8];
int FUN_004584c0(void);
extern BYTE g_unk0x00542630[];
// GLOBAL: CMR2 0x00542600
int g_unk0x00542600;
// GLOBAL: CMR2 0x0054260c
int g_unk0x0054260c[9];

// FUNCTION: CMR2 0x00456b00
void FUN_00456b00(int value)
{
    g_unk0x00542600 = value;
}

// FUNCTION: CMR2 0x00456b10
unsigned int FUN_00456b10(void)
{
    if (CGameInfo::FUN_00405d80() == 4)
        return g_unk0x00542600;
    return FUN_0041b370() & 0xff;
}

// Startup (C runtime .CRT$XCU) initializers of two default quality codes.
extern char g_stageQualityCodes[24];

// FUNCTION: CMR2 0x00456b40
void __cdecl StageQuality_InitCode7(void)
{
    g_unk0x00542630[0x394] = g_stageQualityCodes[7];
}

// FUNCTION: CMR2 0x00456b60
void __cdecl StageQuality_InitCode8(void)
{
    g_unk0x00542630[0x395] = g_stageQualityCodes[8];
}

#pragma data_seg(".CRT$XCU")
static void (__cdecl *s_stageQualityInit[2])(void) = { StageQuality_InitCode7, StageQuality_InitCode8 };
#pragma data_seg()

// FUNCTION: CMR2 0x00456ae0
float FUN_00456ae0(void)
{
    return *(float *)(g_unk0x00542528 + 0x14 + (FUN_00456b10() * 2 - FUN_004584c0()) * 12);
}

// FUNCTION: CMR2 0x00456c00
int FUN_00456c00(int index)
{
    return g_unk0x0054260c[index];
}

// GLOBAL: CMR2 0x00542608
BYTE *g_unk0x00542608;
// GLOBAL: CMR2 0x0051a8bc
char g_strCspFormat[] = "%s.csp";

int FUN_00458040(void);
BYTE *FUN_0041f900(void);

// Loads the stage's .csp data.
// FUNCTION: CMR2 0x00456c10
void FUN_00456c10(void)
{
    sprintf(CFrontend::m_stringDest, g_strCspFormat, FUN_0041f900());
    g_unk0x00542608 = (BYTE *)CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(),
                                                           CFrontend::m_stringDest, 0, 0, 0);
    if (g_unk0x00542608 != NULL) {
        *(BYTE **)(g_unk0x00542630 + 0x240) = g_unk0x00542608;
        *(BYTE **)(g_unk0x00542630 + 0x244) = g_unk0x00542608 + 0x18;
        CGame::RegisterCallback(FUN_00458040, 0);
    }
}

// FUNCTION: CMR2 0x00456c90
int FUN_00456c90(int unused)
{
    return *(int *)(g_unk0x00542630 + 0x244);
}

// FUNCTION: CMR2 0x00457e00
int FUN_00457e00(int index)
{
    return *(int *)(g_unk0x00542630 + 0x398 + index * 4);
}

// match 80%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00457e10
int FUN_00457e10(BYTE *pCar, int offset)
{
    int address = FUN_00457e00((signed char)pCar[0xb1a]);
    if (offset > 0)
        address += offset * 2;
    int base = FUN_00457e00((signed char)pCar[0xb1a]);
    return *(unsigned short *)address + base;
}

// GLOBAL: CMR2 0x005113b0
float g_unk0x005113b0 = 4.0f;
// GLOBAL: CMR2 0x005113a8
double g_unk0x005113a8 = 100.0;

// Reparte el tiempo del piloto entre los dos tramos de la tabla 0x542420.
// match 48%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00456a40
void FUN_00456a40(int param1, int param2)
{
    int *pRec;
    int time;
    int i;
    int value;

    time = StageTiming_GetCurrentSplitTimeForDriver(param2);
    pRec = (int *)(g_unk0x00542528 + param1 * 24);
    for (i = 0; i < 2; i++) {
        pRec = (int *)(g_unk0x00542528 + param1 * 24 + i * 12);
        value = time - g_unk0x00542420[i];
        if (value < 0)
            value = 0;
        value = value / 4;
        pRec[1] = value;
        if (value != 0) {
            pRec[0] = (g_unk0x00542420[i] / 4) / value;
            if (pRec[0] < 10)
                pRec[0] = 10;
        }
        *(float *)((char *)pRec + 8) =
            (float)(g_unk0x005113a8 /
                    ((double)time / (double)g_unk0x00542420[i] * g_unk0x005113b0));
    }
}

// GLOBAL: CMR2 0x0051bd3c
int g_unk0x0051bd3c = 0x10000;
// GLOBAL: CMR2 0x0051bd40
int g_unk0x0051bd40 = 0x10000;

// GLOBAL: CMR2 0x00543d9c
int g_unk0x00543d9c;
// GLOBAL: CMR2 0x00543e88
int g_unk0x00543e88;
// GLOBAL: CMR2 0x00543e8c
int g_unk0x00543e8c;
// GLOBAL: CMR2 0x00543e94
int g_unk0x00543e94;

// FUNCTION: CMR2 0x0045e6b0
void FUN_0045e6b0(int *p1, int *p2, int *p3, int *p4)
{
    *p1 = g_unk0x00543e88;
    *p2 = g_unk0x00543d9c;
    *p3 = (int)(__int64)(g_unk0x00543e8c * CGraphics::m_65536);
    *p4 = (int)(__int64)(g_unk0x00543e94 * CGraphics::m_65536);
}

// Marca el nodo como "sucio" en las etapas especiales y ajusta su 0x64.
// FUNCTION: CMR2 0x0045e9a0
void FUN_0045e9a0(SceneNode *pNode)
{
    *(int *)((char *)pNode + 0x174) = 0;
    if ((BYTE)RallyDataCountryIndex() == 6 && RallyDataStageIndex() == 0 &&
        *(unsigned int *)((char *)pNode + 0x6c) > 0x66 &&
        *(unsigned int *)((char *)pNode + 0x6c) < 0x6d)
        *(int *)((char *)pNode + 0x174) = 1;
    if ((BYTE)RallyDataCountryIndex() == 7 && RallyDataStageIndex() == 2 &&
        *(unsigned int *)((char *)pNode + 0x6c) > 0x1d5 &&
        *(unsigned int *)((char *)pNode + 0x6c) < 0x1d8)
        *(int *)((char *)pNode + 0x174) = 1;
    if (*(int *)((char *)pNode + 0x174) != 0) {
        *(int *)((char *)pNode + 0x64) -= FixMul(0x3333, g_unk0x0051bd3c);
        if (*(int *)((char *)pNode + 0x64) < 0)
            *(int *)((char *)pNode + 0x64) = 0;
    } else {
        *(int *)((char *)pNode + 0x64) += FixMul(0x3333, g_unk0x0051bd3c);
        if (*(int *)((char *)pNode + 0x64) > 0x10000)
            *(int *)((char *)pNode + 0x64) = 0x10000;
    }
}

bool FUN_004779e0(void);

// Reinicia las tablas de escena 0x58d2xx/0x58d3xx/0x58d4xx y registra el
// callback 0x4779e0.
// match 42%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00475f80
void FUN_00475f80(void)
{
    int i;
    int *p;

    g_unk0x0058d6a8[0] = 0;
    memset(g_stageBlock_58d47c, 0, 0x20);
    memset(g_stageBlock_58d340, 0, 0x20);
    memset(g_unk0x0058d49c, 0, 0x20);
    g_unk0x0058d6a8[1] = 0;
    memset(g_unk0x0058d6b0, 0xff, 0x1c);
    memset(g_unk0x0058d2a0, 0xff, 0x30);
    for (p = g_stageBlock_58d368 + 1; p < (int *)g_unk0x0058d3b8; p += 3) {
        p[-1] = -1;
        p[0] = -1;
        p[1] = -1;
    }
    for (p = (int *)g_unk0x0058d3b8 + 1; p < g_stageBlock_58d47c; p += 3) {
        p[-1] = 0;
        p[0] = 0;
        p[1] = 0;
    }
    CGame::RegisterCallback(FUN_004779e0, NULL);
}





void FUN_004688b0(BYTE *p);

// match 51%: the whole structure (record pool alloc, list search, FixVecScale colour clamps,
// FixMulShift32 limit) matches at 294 vs 298 instructions; the diff is MSVC6 stack-slot
// numbering and load scheduling. Kept as FUNCTION.
// Allocates a stage-deform record (0x106-byte pool, 0xd-byte slots) for the
// car's impact, fills it with the clamped offset/normal/impact colours and
// queues it, or falls back to a scratch record.
// FUNCTION: CMR2 0x00468520
void FUN_00468520(void)
{
    BYTE *pRec;
    BYTE *pPrev;
    BYTE *pCur;
    BYTE *pWalk;
    BYTE *pFound;
    BYTE *pBase;
    FixVector vA;
    FixVector vB;
    BYTE buf[20];
    int limit;
    int count;
    int minv;
    int v;
    int scale;

    pRec = NULL;
    pPrev = NULL;
    pBase = g_unk0x00588b98 + *(char *)((BYTE *)g_stageDeformCar + 0xb1a) * 0x290 + 0x106;
    limit = FixMulShift32(g_stageDeformStrength, 0xff0000);
    if (limit > 0xff)
        limit = 0xff;
    if (*(int *)(g_unk0x00588b98 + *(char *)((BYTE *)g_stageDeformCar + 0xb1a) * 0x290 + 0x28c) == 0) {
        count = *(BYTE *)(pBase + 0x104);
        if (count < 0x14) {
            if (count != 0)
                *(BYTE *)(count * 0xd + pBase - 1) = count;
            pRec = (BYTE *)(*(BYTE *)(pBase + 0x104) * 0xd + pBase);
            *(BYTE *)(pBase + 0x104) = *(BYTE *)(pBase + 0x104) + 1;
        }
        else {
            pWalk = (BYTE *)(*(BYTE *)(pBase + 0x105) * 0xd + pBase);
            minv = 1000;
            pCur = NULL;
            pFound = NULL;
            if (pWalk != NULL) {
                do {
                    pFound = pWalk;
                    v = *pFound;
                    if (v < minv) {
                        minv = v;
                        pRec = pFound;
                        pPrev = pCur;
                    }
                    if (pFound[0xc] == 0xff)
                        break;
                    pWalk = (BYTE *)((char)pFound[0xc] * 0xd + pBase);
                    pCur = pFound;
                } while (pWalk != NULL);
            }
            if (!(pRec != NULL && minv < (limit & 0xff)))
                pRec = NULL;
            else if (pRec != pFound) {
                if (pPrev == NULL) {
                    pFound[0xc] = *(BYTE *)(pBase + 0x105);
                    *(BYTE *)(pBase + 0x105) = pRec[0xc];
                }
                else {
                    pFound[0xc] = pPrev[0xc];
                    pPrev[0xc] = pRec[0xc];
                }
                pRec[0xc] = 0xff;
            }
        }
    }
    if (pRec == NULL) {
        pRec = buf;
        if (pRec == NULL)
            return;
    }
    pRec[0] = (BYTE)limit;
    scale = FixMul(FixDiv(0x10000, 0xa0000), 0x7f0000);
    FixVecScale(&vA, &g_stageDeformOffset, scale);
    v = vA.x >> 16;
    if (v > 0x7f)
        v = 0x7f;
    else if (v < -0x7f)
        v = -0x7f;
    pRec[9] = (BYTE)v;
    v = vA.y >> 16;
    if (v > 0x7f)
        v = 0x7f;
    else if (v < -0x7f)
        v = -0x7f;
    pRec[0xa] = (BYTE)v;
    v = vA.z >> 16;
    if (v > 0x7f)
        v = 0x7f;
    else if (v < -0x7f)
        v = -0x7f;
    pRec[0xb] = (BYTE)v;
    FixVecScale(&vB, &g_stageDeformNormal, 0x7f0000);
    v = vB.x >> 16;
    if (v > 0x7f)
        v = 0x7f;
    else if (v < -0x7f)
        v = -0x7f;
    pRec[3] = (BYTE)v;
    v = vB.y >> 16;
    if (v > 0x7f)
        v = 0x7f;
    else if (v < -0x7f)
        v = -0x7f;
    pRec[4] = (BYTE)v;
    v = vB.z >> 16;
    if (v > 0x7f)
        v = 0x7f;
    else if (v < -0x7f)
        v = -0x7f;
    pRec[5] = (BYTE)v;
    FixVecScale(&vB, &g_stageDeformImpact, 0x7f0000);
    v = vB.x >> 16;
    if (v > 0x7f)
        v = 0x7f;
    else if (v < -0x7f)
        v = -0x7f;
    pRec[6] = (BYTE)v;
    v = vB.y >> 16;
    if (v > 0x7f)
        v = 0x7f;
    else if (v < -0x7f)
        v = -0x7f;
    pRec[7] = (BYTE)v;
    v = vB.z >> 16;
    if (v > 0x7f)
        v = 0x7f;
    else if (v < -0x7f)
        v = -0x7f;
    pRec[8] = (BYTE)v;
    pRec[1] = (BYTE)g_stageDeformMode;
    if (g_stageDeformMode == 1) {
        v = FixMulShift32(g_stageDeformSpeed, FixMul(FixDiv(0x10000, 0xa0000), 0xff0000));
        if (v > 0xff)
            v = 0xff;
        pRec[2] = (BYTE)v;
    }
    FUN_004688b0(pRec);
}

// FUNCTION: CMR2 0x00469680
int *FUN_00469680(int index)
{
    return g_unk0x00588990[index];
}

// Restricts a deformed vertex to the per-axis displacement limits and writes it
// back to the mesh's floating-point vertex data.
// match 20%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00508740
void StageDeform_ClampVertex(int *pPosition, int meshIndex, int vertexIndex, int *pRecord)
{
    int *pLimit = (int *)(*(int *)((BYTE *)pRecord + 0x78 + meshIndex * 4) + vertexIndex * 0x20);
    int baseX = pLimit[0];
    int baseY = pLimit[1];
    int baseZ = pLimit[2];
    int x = pPosition[0] - baseX >> 6;
    int y = pPosition[1] - baseY >> 6;
    int z = pPosition[2] - baseZ >> 6;

    if (((0 < x) && (*(signed char *)((BYTE *)pLimit + 0x1b) < 1)) ||
        ((x < 0) && (-1 < *(signed char *)((BYTE *)pLimit + 0x1b)))) x = 0;
    if (((0 < y) && (*(signed char *)((BYTE *)pLimit + 0x1c) < 1)) ||
        ((y < 0) && (-1 < *(signed char *)((BYTE *)pLimit + 0x1c)))) y = 0;
    if (((0 < z) && (*(signed char *)((BYTE *)pLimit + 0x1d) < 1)) ||
        ((z < 0) && (-1 < *(signed char *)((BYTE *)pLimit + 0x1d)))) z = 0;

    int limit = (int)*(signed char *)((BYTE *)pLimit + 0x1b);
    if (((limit < x) && (0 < x)) || ((x < limit) && (x < 0))) x = limit;
    limit = (int)*(signed char *)((BYTE *)pLimit + 0x1c);
    if (((limit < y) && (0 < y)) || ((y < limit) && (y < 0))) y = limit;
    limit = (int)*(signed char *)((BYTE *)pLimit + 0x1d);
    if (((limit < z) && (0 < z)) || ((z < limit) && (z < 0))) z = limit;

    pPosition[0] = baseX + x * 0x40;
    pPosition[1] = baseY + y * 0x40;
    pPosition[2] = baseZ + z * 0x40;
    float *pVertex = (float *)((BYTE *)(*(Mesh **)((BYTE *)pRecord + meshIndex * 4))->pVertexData + vertexIndex * 0x30);
    pVertex[0] = (float)((double)pPosition[0] * CGraphics::m_oneOver65536);
    pVertex[1] = (float)((double)pPosition[1] * CGraphics::m_oneOver65536);
    pVertex[2] = (float)((double)pPosition[2] * CGraphics::m_oneOver65536);
}

// Pushes the body mesh vertices within the impact radius, then refreshes each
// affected mesh and its shadow copy.
// match 32%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00467700
void StageDeform_ApplyRadialDent(void)
{
    Car *pCar = g_stageDeformCar;
    int *pRecord = (int *)(g_unk0x00588b94 + pCar->field_0xb1a * 0x4d0);
    FixVector *pHull;
    int nearest = 0;
    int deepest = 0;
    FixVector scaled;
    FixVecScale(&scaled, &g_stageDeformOffset, -0x10000);
    int sign = FixVecDot(&scaled, &g_stageDeformNormal) < 0;

    for (pHull = g_stageDeformHull; pHull < g_stageDeformHull + 12; ++pHull) {
        FixVector distance;
        distance.x = pHull->x - g_stageDeformOffset.x;
        distance.y = pHull->y - g_stageDeformOffset.y;
        distance.z = pHull->z - g_stageDeformOffset.z;
        int projection = FixVecDot(&distance, &g_stageDeformNormal);
        if (sign) projection = -projection;
        if (projection < deepest) deepest = projection;
        if (projection > 0 && (nearest == 0 || projection < nearest)) nearest = projection;
    }
    if (sign) {
        deepest = -deepest;
        nearest = -nearest;
    }
    if (deepest == 0) {
        if (nearest != 0) {
            FixVecScale(&scaled, &g_stageDeformNormal, nearest);
            g_stageDeformOffset.x += scaled.x;
            g_stageDeformOffset.y += scaled.y;
            g_stageDeformOffset.z += scaled.z;
        }
    } else {
        FixVecScale(&scaled, &g_stageDeformNormal, deepest);
        g_stageDeformOffset.x += scaled.x;
        g_stageDeformOffset.y += scaled.y;
        g_stageDeformOffset.z += scaled.z;
    }

    int normalSide = FixVecDot(&g_stageDeformOffset, &g_stageDeformNormal) >= 0;
    int innerSquare = FixMul(g_stageDeformRadius, g_stageDeformRadius);
    int reciprocalInner = FixDiv(0x10000, g_stageDeformRadius);
    int outer = g_stageDeformFalloff + g_stageDeformRadius;
    int outerSquare = FixMul(outer, outer);
    int reciprocalFalloff = FixDiv(0x10000, g_stageDeformFalloff);
    int shellScale = FixMul(g_stageDeformScale, 0x3333);

    int meshCount = pRecord[0x117];
    for (int meshIndex = 0; meshIndex < meshCount; ++meshIndex) {
        int *pMeshSlot = pRecord + meshIndex;
        int vertexCount = pMeshSlot[0x108];
        int changed = 0;
        for (int vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex) {
            Mesh *pMesh = (Mesh *)*pMeshSlot;
            float *pVertex = (float *)pMesh->pVertexData + vertexIndex * 12;
            int originalX = (int)(__int64)(pVertex[0] * CGraphics::m_65536);
            int originalY = (int)(__int64)(pVertex[1] * CGraphics::m_65536);
            int originalZ = (int)(__int64)(pVertex[2] * CGraphics::m_65536);
            FixVector distance;
            distance.x = g_stageDeformOffset.x - originalX;
            distance.y = g_stageDeformOffset.y - originalY;
            distance.z = g_stageDeformOffset.z - originalZ;
            int projection = FixVecDot(&g_stageDeformNormal, &distance);
            int projectionSquare = FixMul(projection, projection);
            if (projectionSquare <= outerSquare) {
                FixVector position;
                if (innerSquare < projectionSquare) {
                    int length = FixSqrt(projectionSquare);
                    int shellWeight = FixMul(FixMul(length - g_stageDeformRadius,
                                                    reciprocalFalloff), shellScale);
                    int phase = distance.z + distance.x;
                    if (phase < 0) phase = -phase;
                    phase &= 0x80000380;
                    if (phase < 0) phase = ((phase - 1) | 0xfffffc00) + 1;
                    phase *= 0x40;
                    if (phase < 0x8000) phase -= 0x10000;
                    int displacement = FixMul(shellWeight, phase);
                    signed char *pLimits = (signed char *)(pMeshSlot[0x1e] + vertexIndex * 0x20);
                    FixVector direction;
                    direction.x = (int)pLimits[0x18] << 9;
                    direction.y = (int)pLimits[0x19] << 9;
                    direction.z = (int)pLimits[0x1a] << 9;
                    FixVector delta;
                    FixVecScale(&delta, &direction, displacement);
                    position.x = originalX + delta.x;
                    position.y = originalY + delta.y;
                    position.z = originalZ + delta.z;
                } else {
                    int penetration = g_stageDeformRadius - FixMul(reciprocalInner,
                                                                   projectionSquare);
                    FixVector delta;
                    FixVecScale(&delta, &g_stageDeformNormal, penetration);
                    if (normalSide == 0) {
                        position.x = originalX + delta.x;
                        position.y = originalY + delta.y;
                        position.z = originalZ + delta.z;
                    } else {
                        position.x = originalX - delta.x;
                        position.y = originalY - delta.y;
                        position.z = originalZ - delta.z;
                    }
                }
                StageDeform_ClampVertex(&position.x, meshIndex, vertexIndex, pRecord);
                int secondX = (int)(__int64)(pVertex[3] * CGraphics::m_65536);
                int secondY = (int)(__int64)(pVertex[4] * CGraphics::m_65536);
                int secondZ = (int)(__int64)(pVertex[5] * CGraphics::m_65536);
                FixVector secondDelta;
                secondDelta.x = position.x - originalX;
                secondDelta.y = position.y - originalY;
                secondDelta.z = position.z - originalZ;
                FixVecScale(&secondDelta, &secondDelta, 0x30000);
                secondX += secondDelta.x;
                secondY += secondDelta.y;
                secondZ += secondDelta.z;
                pVertex[3] = (float)((double)secondX * CGraphics::m_oneOver65536);
                pVertex[4] = (float)((double)secondY * CGraphics::m_oneOver65536);
                pVertex[5] = (float)((double)secondZ * CGraphics::m_oneOver65536);
                changed = 1;
            }
        }
        if (changed) {
            SceneNode *pNode = (SceneNode *)pMeshSlot[0xf];
            Mesh *pMesh = (Mesh *)pNode->pObject;
            if (pMesh != NULL) {
                Mesh_RefreshVertices(pMesh);
                RallyData_ValidateIndex((int)pMesh);
                Scene_MarkShadowPartDirty(pCar->pNode0x720, pMesh);
            }
        }
    }
}

// Deforms the body mesh around an impact projected onto a plane.
// match 46%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00467e90
void StageDeform_ApplyPlanarDent(void)
{
    Car *pCar = g_stageDeformCar;
    int *pRecord = (int *)(g_unk0x00588b94 + pCar->field_0xb1a * 0x4d0);
    if (FixVecDot(&g_stageDeformNormal, &g_stageDeformOffset) >= 0)
        FixVecScale(&g_stageDeformNormal, &g_stageDeformNormal, -0x10000);

    FixVector delta;
    FixVecScale(&delta, &g_stageDeformNormal, g_stageDeformRadius);
    g_stageDeformOffset.x -= delta.x;
    g_stageDeformOffset.y -= delta.y;
    g_stageDeformOffset.z -= delta.z;

    int radiusSquare = FixMul(g_stageDeformSpeed, g_stageDeformSpeed);
    int reciprocalRadius = FixDiv(0x10000, g_stageDeformSpeed);
    int outer = g_stageDeformFalloff + g_stageDeformSpeed;
    int outerSquare = FixMul(outer, outer);
    int reciprocalFalloff = FixDiv(0x10000, g_stageDeformFalloff);
    int shellScale = FixMul(g_stageDeformScale, 0x3333);

    int meshCount = pRecord[0x117];
    for (int meshIndex = 0; meshIndex < meshCount; ++meshIndex) {
        int *pMeshSlot = pRecord + meshIndex;
        int vertexCount = pMeshSlot[0x108];
        int changed = 0;
        for (int vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex) {
            Mesh *pMesh = (Mesh *)*pMeshSlot;
            float *pVertex = (float *)pMesh->pVertexData + vertexIndex * 12;
            int originalX = (int)(__int64)(pVertex[0] * CGraphics::m_65536);
            int originalY = (int)(__int64)(pVertex[1] * CGraphics::m_65536);
            int originalZ = (int)(__int64)(pVertex[2] * CGraphics::m_65536);

            FixVector distance;
            distance.x = g_stageDeformOffset.x - originalX;
            distance.y = g_stageDeformOffset.y - originalY;
            distance.z = g_stageDeformOffset.z - originalZ;
            int axialDistance = FixVecDot(&g_stageDeformImpact, &distance);
            FixVector axial;
            FixVecScale(&axial, &g_stageDeformImpact, axialDistance);
            distance.x -= axial.x;
            distance.y -= axial.y;
            distance.z -= axial.z;
            int distanceSquare = FixVecDot(&distance, &distance);
            if (distanceSquare <= outerSquare) {
                FixVector displacement;
                if (radiusSquare < distanceSquare) {
                    int length = FixSqrt(distanceSquare);
                    int shellWeight = FixMul(FixMul(length - g_stageDeformSpeed,
                                                    reciprocalFalloff), shellScale);
                    int phase = distance.z + distance.x;
                    if (phase < 0) phase = -phase;
                    phase &= 0x80000380;
                    if (phase < 0) phase = ((phase - 1) | 0xfffffc00) + 1;
                    phase *= 0x40;
                    if (phase < 0x8000) phase -= 0x10000;
                    int push = FixMul(shellWeight, phase);
                    signed char *pLimits = (signed char *)(pMeshSlot[0x1e] + vertexIndex * 0x20);
                    FixVector direction;
                    direction.x = (int)pLimits[0x18] << 9;
                    direction.y = (int)pLimits[0x19] << 9;
                    direction.z = (int)pLimits[0x1a] << 9;
                    FixVecScale(&distance, &direction, push);
                } else {
                    int penetration = g_stageDeformSpeed - FixMul(reciprocalRadius,
                                                                  distanceSquare);
                    FixVecScale(&displacement, &g_stageDeformNormal, penetration);
                }

                FixVector position;
                position.x = originalX + displacement.x;
                position.y = originalY + displacement.y;
                position.z = originalZ + displacement.z;
                StageDeform_ClampVertex(&position.x, meshIndex, vertexIndex, pRecord);

                int secondX = (int)(__int64)(pVertex[3] * CGraphics::m_65536);
                int secondY = (int)(__int64)(pVertex[4] * CGraphics::m_65536);
                int secondZ = (int)(__int64)(pVertex[5] * CGraphics::m_65536);
                FixVector secondDelta;
                secondDelta.x = position.x - originalX;
                secondDelta.y = position.y - originalY;
                secondDelta.z = position.z - originalZ;
                FixVecScale(&secondDelta, &secondDelta, 0x30000);
                secondX += secondDelta.x;
                secondY += secondDelta.y;
                secondZ += secondDelta.z;
                pVertex[3] = (float)((double)secondX * CGraphics::m_oneOver65536);
                pVertex[4] = (float)((double)secondY * CGraphics::m_oneOver65536);
                pVertex[5] = (float)((double)secondZ * CGraphics::m_oneOver65536);
                changed = 1;
            }
        }
        if (changed) {
            SceneNode *pNode = (SceneNode *)pMeshSlot[0xf];
            Mesh *pMesh = (Mesh *)pNode->pObject;
            if (pMesh != NULL) {
                Mesh_RefreshVertices(pMesh);
                RallyData_ValidateIndex((int)pMesh);
                Scene_MarkShadowPartDirty(pCar->pNode0x720, pMesh);
            }
        }
    }
}

struct Car *Car_Get(int index);
struct Unk0x0052ebc0 *RallyData_FUN_00407610(int index);

// Copies the car's stage timing record (split times and penalties) into its
// rally record.
// FUNCTION: CMR2 0x00469b50
void FUN_00469b50(int index)
{
    Car *pCar;
    BYTE *pTiming;
    BYTE *pRecord;

    pCar = Car_Get(index);
    pTiming = g_unk0x00588b98 + pCar->field_0xb1a * 0x290;
    pRecord = (BYTE *)RallyData_FUN_00407610(FUN_0041b370() + pCar->field_0xb1a);
    memcpy(pRecord, pTiming + 0x106, 0x106);
    memcpy(pRecord + 0x108, pTiming + 0x24c, 0x40);
}

// GLOBAL: CMR2 0x00542630
BYTE g_unk0x00542630[0x24 * 32];

// Clears the stage file table and registers its release callback.
// match 80%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00456bb0
void FUN_00456bb0(void)
{
    Unk0x542ae8 *p = g_unk0x00542ae8;
    int count;

    do {
        count = 2;
        do {
            p->pBuffer = NULL;
            p->field_0x4 = NULL;
            p->field_0x8 = NULL;
            p++;
        } while (--count);
    } while ((int)p < (int)(g_unk0x00542ae8 + 32));
    CGame::RegisterCallback(FUN_00456b70, 0);
}

// FUNCTION: CMR2 0x00456be0
BYTE *FUN_00456be0(int index)
{
    return g_unk0x00542630 + index * 0x24;
}

struct Unk0x00590c20 {
    int field_0x0;
    FixMatrix *field_0x4;      // pointer to the world matrix
    BYTE field_0x8[0x114];
    int field_0x11c;
    FixVector field_0x120;     // body offset, accumulated below
    BYTE field_0x12c[0xc];
    FixVector field_0x138;     // body axes source
    FixVector field_0x144;     // accumulated translation
    BYTE field_0x150[0x2c];
    FixBasis field_0x17c;
};

struct Unk0x00590d74 {
    BYTE field_0x0[0x778];
    int field_0x778;
};

// GLOBAL: CMR2 0x00590c20
Unk0x00590c20 *g_unk0x00590c20;
// GLOBAL: CMR2 0x00590d74
Unk0x00590d74 *g_unk0x00590d74;

// Rotates the basis kept at 0x17c with the given angles and writes the three
// axes into the object's matrix.
// FUNCTION: CMR2 0x00481560
void FUN_00481560(unsigned short *pAngles)
{
    FixBasis basis;

    memcpy(&basis, &g_unk0x00590c20->field_0x17c, sizeof(basis));
    if (g_unk0x00590d74->field_0x778 > 0x7ae)
        FixBasis_Rotate(&basis, pAngles);
    FixMatrix_SetRight(&basis.right, g_unk0x00590c20->field_0x4);
    FixMatrix_SetUp(&basis.up, g_unk0x00590c20->field_0x4);
    FixMatrix_SetForward(&basis.forward, g_unk0x00590c20->field_0x4);
}

// Integrates the motion offset and writes the corrected world position.
// FUNCTION: CMR2 0x004815e0
void VehicleMotion_UpdateWorldPosition(void)
{
    FixVector delta;

    FixVecScale(&delta, &g_unk0x00590c20->field_0x138, g_physicsTimeStep);
    g_unk0x00590c20->field_0x144.x += delta.x;
    g_unk0x00590c20->field_0x144.y += delta.y;
    g_unk0x00590c20->field_0x144.z += delta.z;

    FixMatrix_RotateVector(&delta, &g_unk0x00590c20->field_0x120,
                           g_unk0x00590c20->field_0x4);
    delta.x = g_unk0x00590c20->field_0x120.x - delta.x;
    delta.y = g_unk0x00590c20->field_0x120.y - delta.y;
    delta.z = g_unk0x00590c20->field_0x120.z - delta.z;
    delta.x += g_unk0x00590c20->field_0x144.x;
    delta.y += g_unk0x00590c20->field_0x144.y;
    delta.z += g_unk0x00590c20->field_0x144.z;
    FixMatrix_SetPosition(&delta, g_unk0x00590c20->field_0x4);
}

// Records of the 0x542e7c table (stride 0x1c); count derived from the next
// known global (0x543eb8).
struct Unk0x00542e78 {
    short field_0x0;
    short field_0x2;
    short field_0x4;
    short field_0x6;
    short field_0x8;
    short field_0xa;
    short field_0xc;
    short field_0xe;
    short field_0x10;
    short field_0x12;
    short field_0x14;
    BYTE field_0x16;
    BYTE field_0x17;
    BYTE field_0x18;
    BYTE field_0x19;
    BYTE field_0x1a;
    BYTE pad_0x1b;
};

// GLOBAL: CMR2 0x00542e78
Unk0x00542e78 g_unk0x00542e78[8];
// GLOBAL: CMR2 0x00542f58
int g_unk0x00542f58[8];
// GLOBAL: CMR2 0x00542f78
int g_unk0x00542f78[8];
// GLOBAL: CMR2 0x00543098
int g_unk0x00543098;

// FUNCTION: CMR2 0x00459370
void FUN_00459370(void)
{
    int i;

    for (i = 0; i < 8; i++)
        g_unk0x00542f58[i] = -1;
    for (i = 0; i < 8; i++)
        g_unk0x00542f78[i] = 0;
}

// FUNCTION: CMR2 0x00459390
bool FUN_00459390(void)
{
    return g_unk0x00543098 != 0;
}

// GLOBAL: CMR2 0x00537f30
int g_unk0x00537f30;
// GLOBAL: CMR2 0x00538120
BYTE g_unk0x00538120;

extern char g_str0x0051a904[];
int FUN_00406710(void);
int FUN_00406770(void);
int FUN_0040af30(void);

// Draws the stage clock and the countdown timer: picks the timer state from the
// game info flags, then prints it on the HUD with the seconds text and the
// minutes:seconds text.
// FUNCTION: CMR2 0x004593a0
void FUN_004593a0(void)
{
    BYTE colour[4];
    unsigned int remaining;
    unsigned int delta;
    unsigned int limit;
    unsigned int value;
    int x;
    int x2;
    int y;

    colour[0] = 0xff;
    colour[1] = 0xff;
    colour[2] = 0xff;
    colour[3] = 0xff;
    g_unk0x00543098 = 0;
    if ((**(unsigned int **)(FUN_0041b390() + 4) & 0xff) >= 10)
        return;
    if (CGameInfo::FUN_00405d80() == '\n' || CGameInfo::FUN_00405d80() == '\f') {
        if (FUN_00406710() != 0) {
            delta = CMain::GetFrameDelta() - FUN_0040af30();
            limit = FUN_00406710() * 6000;
            if (delta >= limit) {
                remaining = 0;
                g_unk0x00543098 = 2;
            } else {
                g_unk0x00543098 = 2;
                remaining = limit - delta;
            }
        } else {
            g_unk0x00543098 = 0;
        }
    }
    if ((CGameInfo::FUN_00405d80() == '\b' || CGameInfo::FUN_00405d80() == '\t' ||
         CGameInfo::FUN_00405d80() == '\v') &&
        g_unk0x00538120 != 0) {
        delta = CMain::GetFrameDelta() - g_unk0x00537f30;
        value = FUN_00406770() * 100;
        if (delta >= value)
            value = 0;
        else
            value -= delta;
        if (g_unk0x00543098 == 0 || value < remaining) {
            remaining = value;
            g_unk0x00543098 = 1;
        }
    }
    if (g_unk0x00543098 == 0)
        return;
    x = (int)g_pGraphics->resX * 0xaf30 >> 16;
    x2 = (int)g_pGraphics->resX * 0xf06b >> 16;
    if ((**(unsigned int **)(FUN_0041b390() + 4) & 0xff) < 10) {
        if ((BYTE)RallyData_GetFlag24() == 0 && (BYTE)RallyData_GetFlag25() == 0)
            y = (int)g_pGraphics->resY * 0x14 / 0x1e0 + ((int)g_pGraphics->resY * 0x3000 >> 16);
        else
            y = (int)g_pGraphics->resY * 0x14 / 0x1e0 +
                ((int)g_pGraphics->resY * (0x3000 - FixMul(0xaac, 0x8000)) >> 16);
    } else {
        y = (int)g_pGraphics->resY * 0x1c2 / 0x1e0;
    }
    if (g_unk0x00543098 == 1)
        Font_DrawText(0, CFrontend::GetTextString(0xfd), x, y, (int *)colour, 0x21);
    else
        Font_DrawText(0, CFrontend::GetTextString(0xfc), x, y, (int *)colour, 0x21);
    sprintf(CFrontend::m_stringDest, g_str0x0051a904, (remaining / 100) / 60, (remaining / 100) % 60);
    Font_DrawText(3, CFrontend::m_stringDest, x2, y, (int *)colour, 0x24);
}

extern const float g_unk0x00511378;  // defined in StageObjects.cpp (single definition)
extern const float g_unk0x005113d0;  // defined in StageObjects.cpp (single definition)
// GLOBAL: CMR2 0x005113d4
const float g_unk0x005113d4 = 220.0f;
// GLOBAL: CMR2 0x005113d8
const float g_unk0x005113d8 = 0.04f;
extern const float g_unk0x005113dc;  // defined in StageObjects.cpp (single definition)
extern const float g_unk0x005113e0;  // defined in StageObjects.cpp (single definition)
// Degrees -> radians factors of the object yaw angles (FUN_004926f0).
// GLOBAL: CMR2 0x005113e8
extern const double g_oneOver180 = 1.0 / 180.0;
// GLOBAL: CMR2 0x005113f0
extern const double g_pi = 3.14159265359;
// GLOBAL: CMR2 0x0051a910
char g_str0x0051a910[] = "%s (%s)";

extern const float g_netOne;
extern const float g_netZero;
extern const float g_netByteScale;
extern char g_str0x0051a904[];

bool FUN_0040b050(int value);
int FUN_0040b020(int value);
char *FUN_00409cd0(int index);
int FUN_00427620(int index);
int FUN_00422f50(BYTE index);
void FUN_00465f20(SceneNode *pNode, int alpha, BYTE checkFlag);
void FUN_00459630(int *param1, int *param2, int *param3);

// Square of a float expression; the original expands it twice.
#define FSQR(x) ((x) * (x))

// Fade factors of FUN_00459790, expanded where they are used (the original
// computes each one once and keeps it in an x87 scratch slot).
#define TIMER_ALPHA ((distance - g_unk0x00511378) * g_unk0x005113d8)
#define TIMER_LEVEL (g_netOne - (distance - g_unk0x00511378) * g_unk0x005113d0)

// Projects a car's body node into one player's view and draws the stage timing
// marker (driver name or rally record) at the projected position, then updates
// the light level of the car's shadow meshes from the distance to the view
// centre.
// match 69%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00459790
void FUN_00459790(int param_1, int param_2)
{
    Car *pCar;
    SceneNode *pNode;
    SceneNode *pView;
    BYTE colour[4];
    float prevY;
    float prevX;
    int proj[2];
    FixVector up;
    FixVector nodePos;
    FixVector out;
    FixVector viewPos;
    float distance;
    int flag;
    char *pName;
    int old;

    colour[0] = 0xff;
    colour[1] = 0xff;
    colour[2] = 0xff;
    colour[3] = 0xff;
    pCar = Car_Get(param_1);
    pNode = pCar->pNode0x71c;
    pView = g_viewNodes[param_2];
    FixMatrix_GetPosition(&nodePos, &pNode->current);
    FixMatrix_GetPosition(&viewPos, &pView->current);
    nodePos.y += 0x10000;
    FUN_00459630((int *)&nodePos, (int *)&viewPos, (int *)&out);
    FUN_004bad40(proj, &out, (BYTE *)pView);
    if (CGameInfo::FUN_00405d80() == '\b' || CGameInfo::FUN_00405d80() == '\t' ||
        CGameInfo::FUN_00405d80() == '\n')
        flag = 0;
    else
        flag = 1;
    if (proj[0] != -0x640000 || proj[1] != -0x640000) {
        prevX = proj[0] * CGraphics::m_oneOver65536;
        prevY = proj[1] * CGraphics::m_oneOver65536;
        FixMatrix_GetUp(&up, &pView->current);
        up.x += nodePos.x;
        up.y += nodePos.y;
        up.z += nodePos.z;
        FUN_00459630((int *)&up, (int *)&viewPos, (int *)&out);
        FUN_004bad40(proj, &out, (BYTE *)pView);
        if (proj[0] != -0x640000 || proj[1] != -0x640000) {
            distance = (float)sqrt(FSQR((proj[0] * CGraphics::m_oneOver65536 - prevX) * g_unk0x005113e0 /
                                        (int)g_pGraphics->resX) +
                                   FSQR((proj[1] * CGraphics::m_oneOver65536 - prevY) * g_unk0x005113dc /
                                        (int)g_pGraphics->resY));
            if (distance > g_unk0x00511378) {
                if (TIMER_ALPHA >= g_netOne)
                    colour[3] = 0xdc;
                else if (TIMER_ALPHA <= g_netZero)
                    colour[3] = 0;
                else
                    colour[3] = (BYTE)(int)(TIMER_ALPHA * g_unk0x005113d4);
                if (param_1 != 0) {
                    if (FUN_0040b050(param_1)) {
                        pName = FUN_00409cd0(FUN_0040b020(param_1));
                        if (pName != NULL) {
                            if (FUN_00427620(param_1) != 0) {
                                sprintf(CFrontend::m_stringDest, g_str0x0051a910, pName,
                                        CFrontend::GetTextString(0x18));
                                Font_DrawText(0, CFrontend::m_stringDest, (short)(int)prevX,
                                              (short)(int)prevY, (int *)colour, 0x12);
                            } else {
                                Font_DrawText(0, pName, (short)(int)prevX, (short)(int)prevY,
                                              (int *)colour, 0x12);
                            }
                        }
                    }
                } else {
                    Font_DrawText(0, (char *)RallyData_GetRecord(0), (short)(int)prevX,
                                  (short)(int)prevY, (int *)colour, 0x12);
                }
            }
            if (CGameInfo::FUN_00405d80() == '\b' || CGameInfo::FUN_00405d80() == '\t' ||
                CGameInfo::FUN_00405d80() == '\n') {
                if (distance > g_unk0x00511378 && param_1 > 0) {
                    if (TIMER_LEVEL >= g_netOne)
                        g_unk0x00542f78[param_1] = 0xff;
                    else if (TIMER_LEVEL <= g_netZero)
                        g_unk0x00542f78[param_1] = 0;
                    else
                        g_unk0x00542f78[param_1] = (int)(TIMER_LEVEL * g_netByteScale);
                    if (FUN_00422f50(param_2) == 7)
                        g_unk0x00542f78[param_1] = 0x80;
                    if (g_unk0x00542f78[param_1] != g_unk0x00542f58[param_1]) {
                        FUN_00465ec0(Car_Get(param_1)->pNode0x71c,
                                     (BYTE)g_unk0x00542f78[param_1], 0);
                        FUN_00465f20(Car_Get(param_1)->pNode0x71c->pFirstChild,
                                     (BYTE)g_unk0x00542f78[param_1], 0);
                        FUN_00465ec0(Car_Get(param_1)->pNode0x720,
                                     (BYTE)g_unk0x00542f78[param_1], 0);
                        FUN_00465f20(Car_Get(param_1)->pNode0x720->pFirstChild,
                                     (BYTE)g_unk0x00542f78[param_1], 0);
                        g_unk0x00542f58[param_1] = g_unk0x00542f78[param_1];
                    }
                }
            }
            return;
        }
    }
    if (flag == 0 && param_1 > 0) {
        old = g_unk0x00542f58[param_1];
        g_unk0x00542f78[param_1] = 0;
        if (old != 0) {
            FUN_00465ec0(Car_Get(param_1)->pNode0x71c, 0, 0);
            FUN_00465f20(Car_Get(param_1)->pNode0x71c->pFirstChild, 0, 0);
            FUN_00465ec0(Car_Get(param_1)->pNode0x720, 0, 0);
            FUN_00465f20(Car_Get(param_1)->pNode0x720->pFirstChild, 0, 0);
            g_unk0x00542f58[param_1] = 0;
        }
    }
}

// Vector helper of FUN_00459790 that is still to be decompiled. Empty body with
// the original stdcall argument count so the call sites can be measured.
extern const double g_zero0x005113c8;  // defined in StageObjects.cpp
extern float g_65536f;

// float -> 16.16 with a plain fistp (no _ftol).
inline int FloatToFix(float f)
{
    int i;
    __asm fld f
    __asm fmul dword ptr g_65536f
    __asm fistp i
    return i;
}

// Moves pPoint onto the line from pOrigin to it, 10 units from pOrigin
// (the direction is prescaled by its largest component to keep the doubles
// small), and writes the result to pOut.
// match 65%: same code; the original keeps the three converted ints in registers
// (and reads 10.0 from the shared constant at 0x5113c0).
// FUNCTION: CMR2 0x00459630
void FUN_00459630(int *pPoint, int *pOrigin, int *pOut)
{
    double v[3];
    int d[3];
    double big;
    double length;
    int x;
    int y;
    int z;

    d[0] = pPoint[0] - pOrigin[0];
    d[1] = pPoint[1] - pOrigin[1];
    d[2] = pPoint[2] - pOrigin[2];
    v[0] = d[0] * CGraphics::m_oneOver65536;
    v[1] = d[1] * CGraphics::m_oneOver65536;
    v[2] = d[2] * CGraphics::m_oneOver65536;
    if (fabs(v[0]) > fabs(v[1]) && fabs(v[0]) > fabs(v[2]))
        big = fabs(v[0]);
    else if (fabs(v[1]) > fabs(v[0]) && fabs(v[2]) < fabs(v[1]))
        big = fabs(v[1]);
    else
        big = fabs(v[2]);
    if (big > g_zero0x005113c8) {
        v[0] /= big;
        v[1] /= big;
        v[2] /= big;
    }
    length = sqrt(v[2] * v[2] + v[1] * v[1] + v[0] * v[0]);
    v[0] = v[0] * 10.0 / length;
    v[1] = v[1] * 10.0 / length;
    v[2] = v[2] * 10.0 / length;
    x = FloatToFix((float)v[0]);
    y = FloatToFix((float)v[1]);
    z = FloatToFix((float)v[2]);
    pOut[0] = pOrigin[0] + x;
    pOut[1] = pOrigin[1] + y;
    pOut[2] = pOrigin[2] + z;
}

SceneNode *SceneNode_FindByType(SceneNode *pNode, unsigned int type);
void Scene_FreeShadowCasters(void);
void FUN_004866a0(void);
extern SceneNode *g_unk0x00547fec;
extern SceneNode *g_unk0x00547ff0;

// Releases every stage timing resource: the scene nodes hanging from the
// per-car timing records, the four scene node tables and the eight file
// buffers of the fin table.
// FUNCTION: CMR2 0x00457ed0
int FUN_00457ed0(void)
{
    int i;
    BYTE *pRecord;
    int *pNodeList;
    void **pBuffer;

    FUN_004866a0();
    pNodeList = (int *)(g_unk0x00542630 + 0x294);
    pRecord = g_unk0x00542630;
    do {
        if (*(SceneNode **)(pRecord + 8) != NULL) {
            if (*(int *)(pRecord + 0x10) != 0)
                SceneNode_FindByType(*(SceneNode **)(pRecord + 8), 1)->pObject = *(void **)(pRecord + 0x10);
            if (*(int *)(pRecord + 0x14) != 0)
                SceneNode_FindByType(*(SceneNode **)(pRecord + 8), 2)->pObject = *(void **)(pRecord + 0x14);
            if (*(int *)(pRecord + 0x18) != 0)
                SceneNode_FindByType(*(SceneNode **)(pRecord + 8), 3)->pObject = *(void **)(pRecord + 0x18);
            if (*(int *)(pRecord + 0x1c) != 0)
                SceneNode_FindByType(*(SceneNode **)(pRecord + 8), 4)->pObject = *(void **)(pRecord + 0x1c);
            SceneNode_Destroy(*(SceneNode **)(pRecord + 8));
        }
        if (*(SceneNode **)(pRecord + 0xc) != NULL)
            SceneNode_Destroy(*(SceneNode **)(pRecord + 0xc));
        if (*(SceneNode **)pNodeList != NULL)
            SceneNode_Destroy(*(SceneNode **)pNodeList);
        if (*(SceneNode **)(pRecord + 4) != NULL)
            SceneNode_Destroy(*(SceneNode **)(pRecord + 4));
        if (g_unk0x00547fec != NULL) {
            SceneNode_Destroy(g_unk0x00547fec);
            g_unk0x00547fec = NULL;
        }
        if (g_unk0x00547ff0 != NULL) {
            SceneNode_Destroy(g_unk0x00547ff0);
            g_unk0x00547ff0 = NULL;
        }
        pNodeList++;
        pRecord += 0x24;
    } while ((int)pNodeList < (int)(g_unk0x00542630 + 0x2d4));
    for (i = 0; i < 0x40; i += 4) {
        if (*(void **)(g_unk0x00542630 + 0x354 + i) != NULL) {
            CFileBuffer::FreeGenericFileBuffer(*(void **)(g_unk0x00542630 + 0x354 + i));
            *(void **)(g_unk0x00542630 + 0x354 + i) = NULL;
        }
        if (*(void **)(g_unk0x00542630 + 0x2d4 + i) != NULL) {
            CFileBuffer::FreeGenericFileBuffer(*(void **)(g_unk0x00542630 + 0x2d4 + i));
            *(void **)(g_unk0x00542630 + 0x2d4 + i) = NULL;
        }
        if (*(void **)(g_unk0x00542630 + 0x314 + i) != NULL) {
            CFileBuffer::FreeGenericFileBuffer(*(void **)(g_unk0x00542630 + 0x314 + i));
            *(void **)(g_unk0x00542630 + 0x314 + i) = NULL;
        }
        if (*(void **)(g_unk0x00542630 + 0x254 + i) != NULL) {
            CFileBuffer::FreeGenericFileBuffer(*(void **)(g_unk0x00542630 + 0x254 + i));
            *(void **)(g_unk0x00542630 + 0x254 + i) = NULL;
        }
    }
    pBuffer = (void **)(g_unk0x00542630 + 0x398);
    do {
        if (*pBuffer != NULL) {
            CFileBuffer::FreeGenericFileBuffer(*pBuffer);
            *pBuffer = NULL;
        }
        pBuffer++;
    } while ((int)pBuffer < (int)(g_unk0x00542630 + 0x3b8));
    CGraphics::FUN_004a5ba0();
    Scene_FreeShadowCasters();
    return 1;
}

// Registered callback with nothing to release.
// FUNCTION: CMR2 0x00458040
int FUN_00458040(void)
{
    return 1;
}

// FUNCTION: CMR2 0x00458230
BYTE FUN_00458230(int index)
{
    return g_unk0x00542e78[index].field_0x16;
}

// FUNCTION: CMR2 0x00458250
BYTE FUN_00458250(int index)
{
    return g_unk0x00542e78[index].field_0x17;
}

// FUNCTION: CMR2 0x00458270
BYTE FUN_00458270(int index)
{
    return g_unk0x00542e78[index].field_0x18;
}

// FUNCTION: CMR2 0x00458290
BYTE FUN_00458290(int index)
{
    return g_unk0x00542e78[index].field_0x19;
}

// FUNCTION: CMR2 0x004582b0
BYTE FUN_004582b0(int index)
{
    return g_unk0x00542e78[index].field_0x1a;
}

// FUNCTION: CMR2 0x004582d0
int FUN_004582d0(int index)
{
    return g_unk0x00542e78[index].field_0x2;
}

// FUNCTION: CMR2 0x004582f0
int FUN_004582f0(int index)
{
    return g_unk0x00542e78[index].field_0x0;
}

// FUNCTION: CMR2 0x00458310
int FUN_00458310(int index)
{
    return g_unk0x00542e78[index].field_0x12;
}

// FUNCTION: CMR2 0x00458330
int FUN_00458330(int index)
{
    return g_unk0x00542e78[index].field_0x4;
}

// FUNCTION: CMR2 0x00458350
int FUN_00458350(int index)
{
    return g_unk0x00542e78[index].field_0x6;
}

// FUNCTION: CMR2 0x00458370
int FUN_00458370(int index)
{
    return g_unk0x00542e78[index].field_0x14;
}

// FUNCTION: CMR2 0x004589e0
short FUN_004589e0(int index)
{
    return g_unk0x00542e78[index].field_0x10;
}

// GLOBAL: CMR2 0x00543ec0
int g_unk0x00543ec0;
// GLOBAL: CMR2 0x00543ec4
int g_unk0x00543ec4;
// GLOBAL: CMR2 0x00543ec8
short g_unk0x00543ec8;

// FUNCTION: CMR2 0x0045f240
void FUN_0045f240(void)
{
    g_unk0x00543ec8 = -1;
    g_unk0x00543ec0 = 0;
    g_unk0x00543ec4 = 0;
}

// Per car (8) and side (2): trail textures of the two trail kinds.
// GLOBAL: CMR2 0x00543580
int g_trailTextureA[8][2];
// GLOBAL: CMR2 0x005431c0
int g_trailTextureB[8][2];
// GLOBAL: CMR2 0x005433e0
int g_trailForced[8];
// GLOBAL: CMR2 0x005436e8
int g_trailForcedSurface[8];

// FUNCTION: CMR2 0x0045a150
void FUN_0045a150(int texture, int side, int car)
{
    if (car < 8)
        g_trailTextureA[car][side] = texture;
}

// FUNCTION: CMR2 0x0045b530
void FUN_0045b530(int texture, int side, int car)
{
    if (car < 8)
        g_trailTextureB[car][side] = texture;
}

// FUNCTION: CMR2 0x0045b550
void FUN_0045b550(int car, int surface)
{
    if (car < 8) {
        g_trailForced[car] = 1;
        g_trailForcedSurface[car] = surface;
    }
}

BYTE FUN_0041b370(void);
BYTE *RallyData_GetTyreRecord(BYTE index);

// Value `index` of the tyre record `offset` places after the current one.
// FUNCTION: CMR2 0x0045c720
int FUN_0045c720(char offset, int index)
{
    BYTE *pRecord;

    pRecord = RallyData_GetTyreRecord((BYTE)(FUN_0041b370() + offset));
    if (pRecord != NULL)
        return *(int *)(pRecord + 0x90 + index * 4);
    return 0;
}

char *FUN_0041f8f0(void);

// GLOBAL: CMR2 0x0051a12c
char g_strBflFormat[] = "%s.bfl";

// Loads the <stage>.bfl archive into stage file 3.
// FUNCTION: CMR2 0x00455260
void FUN_00455260(void)
{
    sprintf(CFrontend::m_stringDest, g_strBflFormat, FUN_0041f8f0());
    CGenericFileLoader::FUN_004a9d70((GenericFile *)StageTiming_GetStageFile3(), CFrontend::m_stringDest);
}

// Default sky/light vector and cleared offsets.
// FUNCTION: CMR2 0x0045eea0
void FUN_0045eea0(void)
{
    g_unk0x00547930.x = 0xb4fd;
    g_unk0x00547930.y = 0;
    g_unk0x00547930.z = -0xb4fd;
    g_unk0x00547940 = 0;
    g_unk0x00547944.x = 0;
    g_unk0x00547944.y = 0;
    g_unk0x00547944.z = 0;
}

// Applies FUN_00465ec0 to a node list and all descendants.
// FUNCTION: CMR2 0x00465f20
void FUN_00465f20(SceneNode *pNode, int alpha, BYTE checkFlag)
{
    for (; pNode != NULL; pNode = pNode->pNext) {
        FUN_00465ec0(pNode, alpha, checkFlag);
        if (pNode->pFirstChild != NULL)
            FUN_00465f20(pNode->pFirstChild, alpha, checkFlag);
    }
}

// Thresholds (first/second-hit window speeds) for the high/front and low/rear
// halves of a car's eight timing parts.
// GLOBAL: CMR2 0x0051bfac
int g_unk0x0051bfac[8] = { 0x3333, 0x3333, 0x3333, 0x3333, 0x1999, 0x1999, 0x1999, 0x1999 };
// GLOBAL: CMR2 0x0051bfcc
int g_unk0x0051bfcc[8] = { 0x1999, 0x1999, 0x1999, 0x1999, 0x1999, 0x1999, 0x1999, 0x1999 };

void FUN_004694a0(int param_1, int param_2, char param_3);
void FUN_00418cd0(unsigned int view, int kind, int listener);
void Car_QueueWindowBreak(BYTE *pParts, Car *pCar, unsigned int part);

// Flags window/body breaks for the eight parts of a car's timing record.
// FUNCTION: CMR2 0x004692f0
void FUN_004692f0(Car *pCar, int param_2)
{
    BYTE *pRecord;
    BYTE *pByte;
    int *pFlagA;
    BYTE i;
    BYTE part;
    int carIndex;
    unsigned int state;

    pRecord = (BYTE *)(pCar->field_0xb1a * 0x4d0 + (int)g_unk0x00588b94);
    if (*(char *)((int)FUN_00456be0(pCar->field_0xb1a) + 0x20) == 'C' ||
        *(char *)((int)FUN_00456be0(pCar->field_0xb1a) + 0x20) == 'A') {
        i = 0;
        pByte = pRecord + 0x460;
        pFlagA = (int *)(pRecord + 0x490);
        for (; i < 8; i++) {
            part = *pByte;
            if (pFlagA[0] == 0 || pFlagA[-8] == 0) {
                if (pFlagA[-0x7a] > g_unk0x0051bfac[i] && pFlagA[0] == 0) {
                    FUN_004694a0((int)pCar, 2, i);
                    pFlagA[0] = 1;
                    pFlagA[-8] = 1;
                    if (param_2 == 0) {
                        Car_QueueWindowBreak(pRecord, pCar, part);
                        carIndex = pCar->field_0xb1a;
                        state = RallyDataState();
                        if (carIndex < (int)(state & 0xff)) {
                            switch (i) {
                            case 0:
                            case 1:
                            case 2:
                            case 3:
                                FUN_00418cd0(carIndex, 2, carIndex);
                                break;
                            case 4:
                            case 5:
                            case 6:
                            case 7:
                                FUN_00418cd0(carIndex, 0, carIndex);
                                break;
                            }
                        }
                    }
                }
                else if (pFlagA[-0x7a] > g_unk0x0051bfcc[i] && pFlagA[-8] == 0 && pFlagA[0] == 0) {
                    FUN_004694a0((int)pCar, 1, i);
                    pFlagA[-8] = 1;
                    if (param_2 == 0) {
                        carIndex = pCar->field_0xb1a;
                        state = RallyDataState();
                        if (carIndex < (int)(state & 0xff)) {
                            switch (i) {
                            case 0:
                            case 1:
                            case 2:
                            case 3:
                                FUN_00418cd0(carIndex, 2, carIndex);
                                break;
                            case 4:
                            case 5:
                            case 6:
                            case 7:
                                FUN_00418cd0(carIndex, 0, carIndex);
                                break;
                            }
                        }
                    }
                }
            }
            pByte++;
            pFlagA++;
        }
    }
}

// Index of the part of a car model whose node type byte is `type` (-1 none).
// match 46%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004692b0
int FUN_004692b0(unsigned int type, BYTE *pModel)
{
    int i;
    SceneNode **ppNode;

    i = 0;
    if (*(int *)(pModel + 0x45c) > 0) {
        ppNode = (SceneNode **)(pModel + 0x3c);
        do {
            if (((*ppNode)->flags & 0xff) == type)
                return i;
            i++;
            ppNode++;
        } while (i < *(int *)(pModel + 0x45c));
    }
    return -1;
}

// Reads the two values at +0x4a0 of the car's 0x4d0-byte record.
// FUNCTION: CMR2 0x00466870
void FUN_00466870(int *pA, int *pB, Car *pCar)
{
    *pA = *(int *)(g_unk0x00588b94 + pCar->field_0xb1a * 0x4d0 + 0x4a0);
    *pB = *(int *)(g_unk0x00588b94 + pCar->field_0xb1a * 0x4d0 + 0x4a4);
}

// Clears `count` stage records (0x4d0 bytes) from `first`.
// FUNCTION: CMR2 0x004669b0
void FUN_004669b0(int first, int count)
{
    int i;

    for (i = first; i < count + first; i++)
        memset(g_unk0x00588b94 + i * 0x4d0, 0, 0x4d0);
}

extern int g_unk0x00542c68;
// GLOBAL: CMR2 0x00542d58
short g_unk0x00542d58[8];
// GLOBAL: CMR2 0x00542d68
short g_unk0x00542d68[8];

int FUN_00459320(int index);
int RallyData_FUN_00421370(BYTE *p);

// Stores, per car, the checkpoint before its current one and its record's
// field 0x12.
// FUNCTION: CMR2 0x00458b80
void FUN_00458b80(void)
{
    int i;

    for (i = 0; i < g_unk0x00542c68; i++) {
        g_unk0x00542d58[i] = FUN_00459320(RallyData_FUN_00421370((BYTE *)Car_Get(i)));
        g_unk0x00542d68[i] = g_unk0x00542e78[i].field_0x12;
    }
}

unsigned int RallyData_FUN_004082b0(void);
unsigned int RallyData_FUN_004082e0(void);

extern int g_unk0x00542c6c;
extern int g_unk0x00542c74;
extern char g_unk0x00542cad;
extern int g_unk0x00542cb0;
extern int g_unk0x00542cb4[8];

int FUN_00459350(int index);
void RallyData_FUN_004213d0(Car *pCar, int value);
void FUN_00459250(BYTE player, unsigned int node, int dir);

// Resets one player's timing record (table at 0x542e78, stride 0x1c) to the
// default state for the current rally and network mode.
// match 88%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00458bd0
void FUN_00458bd0(int param_1, int param_2, int param_3, char param_4)
{
    Car *pCar;
    short value;

    pCar = Car_Get(param_1);
    g_unk0x00542e78[param_1].field_0x16 = 0;
    g_unk0x00542e78[param_1].field_0x17 = 0;
    g_unk0x00542e78[param_1].field_0x18 = 0;
    g_unk0x00542e78[param_1].field_0x19 = 0;
    g_unk0x00542e78[param_1].field_0x1a = 0;
    g_unk0x00542e78[param_1].field_0x14 = 0;
    g_unk0x00542e78[param_1].field_0x10 = 0;
    g_unk0x00542e78[param_1].field_0x12 = 0;
    if ((BYTE)RallyData_GetFlag24() == 0 && (BYTE)RallyData_GetFlag25() == 0) {
        g_unk0x00542e78[param_1].field_0x0 = (short)RallyData_FUN_00421370((BYTE *)pCar);
        g_unk0x00542e78[param_1].field_0x2 = 0;
        g_unk0x00542e78[param_1].field_0x4 = 0;
        g_unk0x00542e78[param_1].field_0x6 = 0;
        g_unk0x00542e78[param_1].field_0x8 = 0;
        g_unk0x00542e78[param_1].field_0xa = 1;
        g_unk0x00542e78[param_1].field_0xc = 0;
        g_unk0x00542e78[param_1].field_0xe = (short)g_unk0x00542c74 - 1;
        goto end;
    }
    g_unk0x00542e78[param_1].field_0x4 = 0;
    g_unk0x00542e78[param_1].field_0x8 = 0;
    if (param_4 != 0) {
        g_unk0x00542e78[param_1].field_0x0 = (short)FUN_00459320(RallyData_FUN_00421370((BYTE *)pCar));
    }
    else {
        value = g_unk0x00542d58[param_1];
        g_unk0x00542e78[param_1].field_0x0 = value;
        RallyData_FUN_004213d0(pCar, FUN_00459350((int)value));
        g_unk0x00542e78[param_1].field_0x12 = g_unk0x00542d68[param_1];
    }
    if ((BYTE)RallyData_GetFlag24() != 0) {
        g_unk0x00542e78[param_1].field_0x2 = -1;
        g_unk0x00542e78[param_1].field_0x6 = 0;
        g_unk0x00542e78[param_1].field_0xe = 0;
        if (g_unk0x00542cad == 0) {
            g_unk0x00542e78[param_1].field_0xa = 1;
            g_unk0x00542e78[param_1].field_0xc = g_unk0x00542c6c;
        }
        else {
            g_unk0x00542e78[param_1].field_0xa = 0;
            g_unk0x00542e78[param_1].field_0xc = 1000;
        }
        goto end;
    }
    if (g_unk0x00542cad == 0) {
        if (CGameInfo::FUN_00405e00() != 0) {
            if (g_unk0x00542cb4[0] != 0 && CGameInfo::FUN_00405d80() != 10)
                goto d8e;
        }
        else {
            if (param_1 != g_unk0x00542cb0 && CGameInfo::FUN_00405d80() != 3)
                goto d8e;
        }
        g_unk0x00542e78[param_1].field_0x2 = -1;
        g_unk0x00542e78[param_1].field_0x6 = 0;
        g_unk0x00542e78[param_1].field_0xa = 1;
        g_unk0x00542e78[param_1].field_0xc = 1;
    }
    else {
        g_unk0x00542e78[param_1].field_0x2 = -1;
        g_unk0x00542e78[param_1].field_0x6 = 0;
        g_unk0x00542e78[param_1].field_0xa = 0;
        g_unk0x00542e78[param_1].field_0xc = 1000;
    }
    g_unk0x00542e78[param_1].field_0xe = 0;
    goto end;
d8e:
    g_unk0x00542e78[param_1].field_0x2 = 0;
    g_unk0x00542e78[param_1].field_0x6 = 1;
    g_unk0x00542e78[param_1].field_0xa = 0;
    g_unk0x00542e78[param_1].field_0x8 = 1;
    g_unk0x00542e78[param_1].field_0xc = 1;
    g_unk0x00542e78[param_1].field_0xe = 1;
end:
    FUN_00459250((BYTE)param_1, (int)g_unk0x00542e78[param_1].field_0x0, 1);
}

// Flags the record when its two positions coincide (not in some network modes).
// FUNCTION: CMR2 0x00459180
void FUN_00459180(int index)
{
    if (g_unk0x00542e78[index].field_0x4 == g_unk0x00542e78[index].field_0xc &&
        g_unk0x00542e78[index].field_0x6 == g_unk0x00542e78[index].field_0xe) {
        g_unk0x00542e78[index].field_0x1a = 1;
        if ((BYTE)RallyData_FUN_004082e0() && RallyData_FUN_004082b0() == 1)
            g_unk0x00542e78[index].field_0x1a = 0;
    }
}

// GLOBAL: CMR2 0x00590b30
int g_unk0x00590b30[8];
// GLOBAL: CMR2 0x00590b5c
int g_unk0x00590b5c[8];
// GLOBAL: CMR2 0x00590c00
int g_unk0x00590c00[8];
extern BYTE *g_unk0x00590b7c[4][8];
extern BYTE g_unk0x00590c24[4][8];

// Clears every car's four 0x1a0-byte record arrays and the slot tables.
// match 73%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00480980
void FUN_00480980(void)
{
    void **pp;
    int i;

    for (pp = g_unk0x00590d7c; (int)pp < (int)(g_unk0x00590d7c + 4); pp++)
        for (i = 0; i < g_unk0x00590c64; i++)
            memset((BYTE *)*pp + i * 0x1a0, 0, 0x1a0);
    memset(g_unk0x00590b7c, 0, sizeof(g_unk0x00590b7c));
    memset(g_unk0x00590c24, 0xff, sizeof(g_unk0x00590c24));
}

// Caches, per car in the list, pointers into its timing record.
// FUNCTION: CMR2 0x004809e0
void FUN_004809e0(short *pList, short count)
{
    int i;
    Car *pCar;
    int p;

    for (i = count - 1; i >= 0; i--) {
        pCar = Car_Get(pList[i]);
        p = FUN_00457e10((BYTE *)pCar, 3);
        g_unk0x00590b5c[pCar->field_0xb1a] = p;
        p += 0xc;
        g_unk0x00590b30[pCar->field_0xb1a] = p;
        p += 4;
        g_unk0x00590c00[pCar->field_0xb1a] = p;
    }
}

extern int g_unk0x00542cb4[8];
extern int g_stageCheckpointCount;
int FUN_0040b010(int index);
extern double g_unk0x005113b8;

// Advances a player's lap counter by half the checkpoint count (with the
// fractional part kept in *pFrac), unless the player has finished.
// match 80%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004591e0
void FUN_004591e0(int player, int *pCount, int *pFrac)
{
    int slot;
    float count;
    double value;

    if (player == -2)
        slot = 0;
    else
        slot = FUN_0040b010(player);
    if (g_unk0x00542cb4[slot] == 0) {
        count = (float)g_stageCheckpointCount;
        value = count * g_unk0x005113b8 + *pFrac;
        if (value >= count) {
            (*pCount)++;
            *pFrac = (int)(__int64)(value - count);
            return;
        }
        *pFrac = (int)(__int64)value;
    }
}

BYTE *RallyData_GetTyreRecord(BYTE index);
void RallyData_MarkTyresChanged(int index);
void Tyre_AddWear(int car, int wheel, int damage, int wear);

// Resets the wear record of one wheel of a player's car.
// FUNCTION: CMR2 0x0045c6b0
void FUN_0045c6b0(int player, int wheel)
{
    BYTE *p = RallyData_GetTyreRecord((BYTE)(FUN_0041b370() + player));

    if (p != NULL) {
        *(int *)(p + 0x80 + wheel * 4) = 0;
        *(int *)(p + 0x60 + wheel * 4) = 0;
        *(int *)(p + 0x70 + wheel * 4) = 0;
        *(int *)(p + 0x20 + wheel * 4) = 0;
        *(int *)(p + wheel * 4) = 0;
        *(int *)(p + 0x10 + wheel * 4) = 0;
        *(int *)(p + 0x50 + wheel * 4) = 0;
        *(int *)(p + 0x40 + wheel * 4) = 0;
        *(int *)(p + 0x30 + wheel * 4) = 0;
        RallyData_MarkTyresChanged((FUN_0041b370() & 0xff) + player);
        Tyre_AddWear(player, wheel, 0, 0);
    }
}

// GLOBAL: CMR2 0x00590c68
int g_unk0x00590c68;

// Turns the vehicle about its vertical axis by the per-frame rate, toward the
// side given by its orientation.
// match 76%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004814d0
void FUN_004814d0(void)
{
    unsigned short angles[3];

    if ((g_unk0x00590c20->field_0x150[0] & 0xf0) == 0x20) {
        int direction = *(int *)&g_unk0x00590c20->field_0x17c;
        angles[0] = 0;
        angles[1] = 0;
        if (direction < 0)
            angles[2] = (unsigned short)g_unk0x00590c68;
        else
            angles[2] = (unsigned short)-g_unk0x00590c68;
    } else {
        int direction = *(int *)&g_unk0x00590c20->field_0x17c;
        angles[0] = 0;
        angles[1] = 0;
        if (direction > 0)
            angles[2] = (unsigned short)g_unk0x00590c68;
        else
            angles[2] = (unsigned short)-g_unk0x00590c68;
    }
    FUN_00481560(angles);
    VehicleMotion_UpdateWorldPosition();
}

// GLOBAL: CMR2 0x00592748
int g_unk0x00592748[8];

void RallyData_FUN_00421530(int index, int *pOut);
int RallyData_FUN_00421420(void);

// Lays out count 0x1720-byte car records from p and offsets their route
// points by the route origin; returns the end of the records.
// match 67%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00498590
BYTE *FUN_00498590(BYTE *p, int unused, int count)
{
    int origin[3];
    int points;
    int *pp;
    int i;

    RallyData_FUN_00421530(0, origin);
    points = RallyData_FUN_00421420();
    for (pp = g_unk0x00592748; count > 0; count--, pp++) {
        *pp = (int)p;
        p += 0x1720;
        for (i = 0; i < points; i++) {
            *(int *)(*pp + 0xb90 + i * 4) += origin[0];
            *(int *)(*pp + 0x1158 + i * 4) += origin[2];
        }
    }
    return p;
}

// Fills pOut[1..n] from the car values selected by the descriptor's type list.
// FUNCTION: CMR2 0x00498ca0
void FUN_00498ca0(char *pDesc, int *pValues, int *pOut)
{
    int i;
    int value;

    int *p = pOut + 1;

    for (i = 1; i <= pDesc[0xc]; i++) {
        switch (pDesc[i]) {
        case 0:
            value = pValues[6];
            break;
        case 1:
            value = pValues[7];
            break;
        case 3:
            value = pValues[3];
            break;
        case 4:
            value = pValues[0x28];
            break;
        case 6:
            value = pValues[0];
            break;
        case 8:
            value = pValues[0xd];
            break;
        case 9:
            value = pValues[0xe];
            break;
        case 10:
            value = pValues[1];
            break;
        case 11:
            value = pValues[2];
            break;
        case 12:
            value = pValues[0x18];
            break;
        case 14:
            value = pValues[0x19];
            break;
        case 18:
            value = pValues[0x1e];
            break;
        case 20:
            value = pValues[0x20];
            break;
        default:
            exit(0);
        }
        *p++ = value;
    }
}

// GLOBAL: CMR2 0x0053d1d9
BYTE g_unk0x0053d1d9;

// Advances the stage clock by 4 with a little jitter, stopping at one hour.
// match 75%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00448de0
void FUN_00448de0(void)
{
    int jitter;

    if (g_unk0x0053d1d9 == 0 && g_unk0x0053d1b4 >= 0x57e3c) {
        g_unk0x0053d1d8 = 1;
        g_unk0x0053d1b4 = 360000;
        g_unk0x0053d1b0 = 359999;
    } else {
        g_unk0x0053d1b4 += 4;
        jitter = rand() % 4;
        jitter--;
        g_unk0x0053d1b0 = g_unk0x0053d1b4 + jitter;
        if ((BYTE)RallyData_FUN_004082e0()) {
            g_unk0x0053d1a0 += 4;
            g_unk0x0053d1a2 = g_unk0x0053d1a0 + (short)jitter;
        }
    }
}

int FUN_0041b380(void);

// Copies the driver slots into the split display table for the current view mode.
// FUNCTION: CMR2 0x00455620
void FUN_00455620(void)
{
    int count = (BYTE)CGameInfo::FUN_00405d70();
    int mode = FUN_0041b380();
    int i;

    if (mode >= 0) {
        if (mode > 1) {
            if (mode == 4) {
                for (i = 0; i < CGameInfo::FUN_00405d70(); i++) {
                    g_unk0x00542528[0xc0 + i] = 1;
                    g_unk0x00542528[0xc8 + i * 2] = g_stageDriverSlot[i];
                }
            }
        } else {
            g_unk0x00542528[0xc0] = count;
            if (count > 0)
                memcpy(g_unk0x00542528 + 0xc8, g_stageDriverSlot, count);
        }
    }
}

extern char g_unk0x00542cad;

// Counts laps: crossing from the last checkpoint to the first adds one
// (wrapping at 1000); going back the other way removes one.
// FUNCTION: CMR2 0x00458f30
void FUN_00458f30(int car, int from, int to)
{
    int count = g_stageCheckpointCount;

    if (to < g_unk0x00542e78[car].field_0x0 && to == 0 && from == count - 1) {
        g_unk0x00542e78[car].field_0x2++;
        g_unk0x00542e78[car].field_0x18 = 1;
        if (g_unk0x00542e78[car].field_0x2 == 1000)
            g_unk0x00542e78[car].field_0x2 = 0;
    }
    if (to > g_unk0x00542e78[car].field_0x0 && to == count - 1 && from == 0) {
        g_unk0x00542e78[car].field_0x2--;
        if (g_unk0x00542e78[car].field_0x2 < -1)
            g_unk0x00542e78[car].field_0x2 = -1;
        if (g_unk0x00542cad != 0)
            g_unk0x00542e78[car].field_0x6 = 0;
    }
}

// GLOBAL: CMR2 0x00543d60
int g_unk0x00543d60;
// GLOBAL: CMR2 0x00543d64
int g_unk0x00543d64;
// GLOBAL: CMR2 0x00543d68
int g_unk0x00543d68;
// GLOBAL: CMR2 0x00543d6c
unsigned int g_unk0x00543d6c;
// GLOBAL: CMR2 0x00543d70
unsigned int g_unk0x00543d70;
// GLOBAL: CMR2 0x00543d74
int g_unk0x00543d74;
// GLOBAL: CMR2 0x00543e98
BYTE g_unk0x00543e98;

void StageObject_SetScaledValue(int value, int index);
BYTE FUN_00422fb0(BYTE index);
int RallyData_FUN_00421500(void);

// Scales a view's object value by the car's route position between two limits.
// match 84%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0045f530
void FUN_0045f530(BYTE *pObject, int view)
{
    unsigned int position;

    if (RallyData_FUN_00421500() == 0) {
        position = RallyData_FUN_00421370((BYTE *)Car_Get(FUN_00422fb0(view)));
        *(unsigned int *)(pObject + 0x6c) = position;
        if (position != *(unsigned int *)(pObject + 0x70)) {
            *(unsigned int *)(pObject + 0x70) = position;
            if (position <= g_unk0x00543d6c) {
                StageObject_SetScaledValue(g_unk0x00543d60, view);
                return;
            }
            if (position >= g_unk0x00543d70) {
                StageObject_SetScaledValue(g_unk0x00543d64, view);
                return;
            }
            StageObject_SetScaledValue(FixMul((position - g_unk0x00543d6c) << 16, g_unk0x00543d68) + g_unk0x00543d60,
                                       view);
        }
    }
}

// Resets every view's scaled object state.
// FUNCTION: CMR2 0x0045e610
void FUN_0045e610(void)
{
    int i;
    int *p;

    for (i = 0; i < g_unk0x00543e98; i++) {
        StageObject_SetScaledValue(g_unk0x00543d60, i);
        *(int *)((BYTE *)g_unk0x00547ac8 + i * 0x178 + 0x58) = *(int *)((BYTE *)g_unk0x00547ac8 + i * 0x178 + 0x5c);
        *(int *)((BYTE *)g_unk0x00547ac8 + i * 0x178 + 0x174) = 0;
        *(int *)((BYTE *)g_unk0x00547ac8 + i * 0x178 + 0x64) = 0x10000;
        *(int *)((BYTE *)g_unk0x00547ac8 + i * 0x178 + 0x68) = 0x10000;
        p = (int *)((BYTE *)g_unk0x00543ecc + i * 0xc);
        p[1] = 0;
        p[0] = 0;
        p[2] = g_unk0x00543d74;
    }
}

// Commits (or, with a == b == 0, first resets) the tyre wear of count players.
// match 70%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0045c610
void FUN_0045c610(int a, int b, int count)
{
    int i;
    int j;
    BYTE *pRecord;
    int *w;

    for (i = 0; i < count; i++) {
        pRecord = RallyData_GetTyreRecord((BYTE)(FUN_0041b370() + i));
        if (pRecord != NULL) {
            w = (int *)(pRecord + 0x50);
            for (j = 0; j < 4; j++, w++) {
                if (a == 0 && b == 0) {
                    w[0] = w[-0xc];
                    if (w[-0x10] < w[-0x14])
                        w[-8] = w[-0x14];
                    else
                        w[-8] = w[-0x10];
                    w[-4] = 0;
                }
                w[0xc] = w[0];
                w[4] = w[-4];
                w[8] = w[-8];
                RallyData_MarkTyresChanged((FUN_0041b370() & 0xff) + i);
                Tyre_AddWear(i, j, 0, 0);
            }
        }
    }
}

// Resets a car's replay recording record.
// match 67%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00466920
void FUN_00466920(BYTE *p)
{
    int i;

    p[0x20a] = 0;
    p[0x20b] = 0;
    for (i = 0; i < 20; i++)
        p[0x112 + i * 0xd] = 0xff;
    memset(p + 0x24c, 0, 0x22);
    memset(p + 0x270, 0, 7 * sizeof(int));
    memcpy(p, p + 0x106, 0x106);
    memcpy(p + 0x20c, p + 0x24c, 0x40);
}

int FUN_00458390(void);

// One bubble pass over the running order, keeping each car's position count.
// match 25%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00448d50
void FUN_00448d50(void)
{
    int count = FUN_00458390();
    int i;
    char a;
    char b;

    for (i = 1; i < count; i++) {
        if (FUN_004486a0(g_unk0x0053dda8[i], g_unk0x0053dda8[i - 1]) == 1) {
            a = g_unk0x0053dda8[i - 1];
            g_carStageTiming[a].field_0x81++;
            b = g_unk0x0053dda8[i];
            g_unk0x0053dda8[i] = a;
            g_unk0x0053dda8[i - 1] = b;
            g_carStageTiming[b].field_0x81--;
        }
    }
}

void FixMatrix_Interpolate(FixMatrix *pOut, FixMatrix *pA, FixMatrix *pB, int tRight, int tAxis, int tPos, int mode);
short Car_GetOrderCount(void);
short *Car_GetOrder(void);

// Interpolates, for every car and each of its four moving parts, the part's
// matrix between its two keys and applies it to the part's node.
// match 29%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00484d30
void FUN_00484d30(int t)
{
    int count = Car_GetOrderCount();
    short *pOrder = Car_GetOrder();
    short *p;
    char index;
    void **pp;
    BYTE *pPart;

    for (p = pOrder + count - 1; count > 0; count--, p--) {
        index = Car_Get(*p)->field_0xb1a;
        for (pp = &g_unk0x00590d7c[3]; pp >= g_unk0x00590d7c; pp--) {
            pPart = (BYTE *)*pp + index * 0x1a0;
            g_unk0x00590c20 = (Unk0x00590c20 *)pPart;
            if (*(int *)pPart != 0 && (pPart[0x150] & 1) != 0) {
                FixMatrix_Interpolate((FixMatrix *)(pPart + 0x8c), (FixMatrix *)(pPart + 0x4c), (FixMatrix *)(pPart + 0xc),
                                      t, t, t, 0);
                FixMatrix_CopyRotation((FixMatrix *)(pPart + 0x8c), (FixMatrix *)(*(BYTE **)pPart + 0x98));
            }
        }
    }
}

// GLOBAL: CMR2 0x00590d78
BYTE *g_unk0x00590d78;
extern BYTE g_unk0x00590c60[4];

void FUN_00469bf0(Car *pCar, int index);

// Clears the low bit of the vehicle's part-state byte and re-selects the part
// index held in its high nibble.
// FUNCTION: CMR2 0x00483010
void FUN_00483010(void)
{
    g_unk0x00590c20->field_0x150[0] &= 0xfe;
    FUN_00469bf0((Car *)g_unk0x00590d74, g_unk0x00590c20->field_0x150[0] >> 4);
}

// Starts a part's swing when the load on its side exceeds 0.8: the swing
// speed (+0x11c) is added or removed depending on which wheel is loaded more.
// match 36%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00483050
void FUN_00483050(void)
{
    Unk0x00590c20 *pPart = g_unk0x00590c20;
    int a;
    int b;
    int swing;

    if (*(int *)(g_unk0x00590d78 + 0x240 + g_unk0x00590c60[pPart->field_0x150[0] >> 4] * 4) > 0xcccc) {
        if ((pPart->field_0x150[0] & 0xf0) == 0x10) {
            a = 2;
            b = 3;
        } else {
            a = 0;
            b = 1;
        }
        if (*(int *)(g_unk0x00590d78 + a * 4 + 0x240) >
                *(int *)(g_unk0x00590d78 + b * 4 + 0x240))
            swing = pPart->field_0x120.z - pPart->field_0x11c;
        else
            swing = pPart->field_0x120.z + pPart->field_0x11c;
        pPart->field_0x120.z = swing;
        g_unk0x00590c20->field_0x150[0] |= 2;
        *(short *)((BYTE *)g_unk0x00590c20 + 0x158) = 0;
    }
}

// GLOBAL: CMR2 0x00588a80
int g_unk0x00588a80;
// GLOBAL: CMR2 0x00588a84
int g_unk0x00588a84;
// GLOBAL: CMR2 0x00588a88
int g_unk0x00588a88;
// GLOBAL: CMR2 0x00588a8c
int g_unk0x00588a8c;

// Resets every car's replay recording record.
// match 71%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004668d0
void FUN_004668d0(void)
{
    int i;

    for (i = g_unk0x00588a90 - 1; i >= 0; i--)
        FUN_00466920(g_unk0x00588b98 + i * 0x290);
    g_unk0x00588a88 = 0;
    g_unk0x00588a80 = 0;
    g_unk0x00588a8c = 0;
    g_unk0x00588a84 = 0;
}

void Events_Init(int unused, int slot, char animate);
struct EventRec;
void Events_Add(EventRec *pArea, int unused);

// Texture areas of the stage events: 11 records {packed position, width,
// height} per stage type.
// GLOBAL: CMR2 0x0051a3e8
unsigned int g_stageEventAreas[14 * 22] = {
    0x00c60000, 0x003a0100, 0x00000000, 0x003a0100, 0x003b0000, 0x0034005d, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00b800ea, 0x000d0016, 0x00000000, 0x00000000, 0x00b800d4, 0x000d0016, 0x00000000, 0x00000000,
    0x00000000, 0x002f0100, 0x00d10000, 0x002f0100, 0x002f009c, 0x00340064, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00320100, 0x00ce0000, 0x00320100, 0x0048003b, 0x00200066, 0x00320028, 0x0016006a, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00300100, 0x00d00000, 0x00300100, 0x00aa00a6, 0x0025005a, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x003b0100, 0x00c50000, 0x003b0100, 0x00a70000, 0x001d0081, 0x00a70089, 0x001d0079, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00340100, 0x00cc0000, 0x00340100, 0x00960090, 0x0036006c, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00310100, 0x00cf0000, 0x00310100, 0x006b0095, 0x0017006a, 0x002f0098, 0x00180068, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x002e0100, 0x00d00000, 0x002e0100, 0x009a0000, 0x002d007e, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x003b0100, 0x00c50000, 0x003b0100, 0x003b0087, 0x00380078, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00390100, 0x00c70000, 0x00390100, 0x0039008f, 0x002b0071, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x002c0100, 0x00d20000, 0x002c0100, 0x002e0000, 0x00270071, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00330100, 0x00cd0000, 0x00330100, 0x0041007e, 0x00280082, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x003b0100, 0x00c50000, 0x003b0100, 0x005600a4, 0x0027005c, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x002f0100, 0x00d10000, 0x002f0100, 0x005f0094, 0x0028006b, 0x008a00fb, 0x00500005,
};

// Initialises the stage events of a stage type (11 areas per type).
// FUNCTION: CMR2 0x00458050
void FUN_00458050(int unused, int slot, int type)
{
    BYTE *pArea;
    int i;

    Events_Init(unused, slot, (char)type);
    pArea = (BYTE *)&g_stageEventAreas[type * 22];
    for (i = 11; i != 0; i--, pArea += 8)
        Events_Add((EventRec *)pArea, slot);
}

// GLOBAL: CMR2 0x00543d78
int g_unk0x00543d78;
// GLOBAL: CMR2 0x00543d7c
int g_unk0x00543d7c;
// GLOBAL: CMR2 0x00543d80
unsigned int g_unk0x00543d80;
// GLOBAL: CMR2 0x00543d84
unsigned int g_unk0x00543d84;
// GLOBAL: CMR2 0x00543d88
int g_unk0x00543d88;
// GLOBAL: CMR2 0x00543d8c
int g_unk0x00543d8c;
// GLOBAL: CMR2 0x00543d90
int g_unk0x00543d90;
// GLOBAL: CMR2 0x00543d94
int g_unk0x00543d94;
// GLOBAL: CMR2 0x00543d98
int g_unk0x00543d98;

// Sets the object value ramp (v1 at route position `from` to v2 at `to`)
// and resets every view's object state.
// match 54%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0045f300
void FUN_0045f300(int v1, int v2, int from, int to, int initial)
{
    int i;
    int *p;

    g_unk0x00543d60 = v1;
    g_unk0x00543d64 = v2;
    g_unk0x00543d70 = to;
    g_unk0x00543d68 = (to - from) << 16;
    g_unk0x00543d6c = from;
    if (g_unk0x00543d68 > 0)
        g_unk0x00543d68 = FixDiv(0x10000, g_unk0x00543d68);
    g_unk0x00543d68 = FixMul(g_unk0x00543d68, v2 - v1);
    g_unk0x00543e98 = (BYTE)RallyDataState();
    for (i = 0; i < g_unk0x00543e98; i++) {
        p = (int *)((BYTE *)g_unk0x00547ac8 + i * 0x178);
        p[1] = initial;
        p[0] = initial;
        p[0x1c] = 0;
        p[0x1b] = 0;
        StageObject_SetScaledValue(v1, i);
    }
}

// Sets the second ramp (records of 0x2c bytes at g_unk0x00543eb8).
// match 45%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0045f3d0
void FUN_0045f3d0(int v1, int v2, int from, int to)
{
    int i;
    int *p;

    g_unk0x00543d88 = v1;
    g_unk0x00543d98 = to;
    g_unk0x00543d90 = (to - from) << 16;
    g_unk0x00543d8c = v2;
    g_unk0x00543d94 = from;
    if (g_unk0x00543d90 > 0)
        g_unk0x00543d90 = FixDiv(0x10000, g_unk0x00543d90);
    g_unk0x00543d90 = FixMul(g_unk0x00543d90, v2 - v1);
    for (i = 0; i < g_unk0x00547acc; i++) {
        p = (int *)((BYTE *)g_unk0x00543eb8 + i * 0x2c);
        p[1] = 0;
        p[0] = 0;
        p[2] = g_unk0x00543d88;
        p[3] = 0;
        p[4] = g_unk0x00543d88;
        p[6] = g_unk0x00543d88;
        p[10] = 1;
    }
}

short *Car_GetOrder(void);

// Updates each car's third ramp value from its route position.
// match 60%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0045e7f0
void FUN_0045e7f0(void)
{
    int count;
    short *p;
    Car *pCar;
    unsigned int *pRec;
    unsigned int position;

    if (RallyData_FUN_00421500() == 0) {
        count = Car_GetOrderCount();
        for (p = Car_GetOrder() + count - 1; count > 0; count--, p--) {
            pCar = Car_Get(*p);
            pRec = (unsigned int *)((BYTE *)g_unk0x00543ecc + pCar->field_0xb1a * 0xc);
            position = RallyData_FUN_00421370((BYTE *)pCar);
            pRec[0] = position;
            if (position != pRec[1]) {
                pRec[1] = position;
                if (g_unk0x00543d80 < position) {
                    if (position < g_unk0x00543d84)
                        pRec[2] = FixMul((position - g_unk0x00543d80) << 16, g_unk0x00543d7c) + g_unk0x00543d74;
                    else
                        pRec[2] = g_unk0x00543d78;
                } else {
                    pRec[2] = g_unk0x00543d74;
                }
            }
        }
    }
}

short FUN_004589e0(int index);
unsigned int RallyData_FUN_00406990(void);
int FUN_0040d4b0(int hundredths);

// Estimates the stage time from the progress so far (at least halfway).
// match 75%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00448550
int FUN_00448550(void)
{
    int total;
    int progress;

    g_unk0x0053d1da[0] = 1;
    g_unk0x0053e18c++;
    if (CGameInfo::FUN_00405d80() == 11)
        total = (BYTE)RallyData_FUN_00406990() * RallyData_FUN_00421420() * 0x10000;
    else
        total = RallyData_FUN_00421420() << 16;
    progress = FixDiv((unsigned short)FUN_004589e0(0) << 16, total);
    if (progress > 0x8000) {
        return (g_unk0x0053d1b8[0] = ConvertRawTimeToCentiseconds(FixMul(FUN_0040d4b0(g_unk0x0053d1b0), FixDiv(0x10000, progress))));
    }
    return (g_unk0x0053d1b8[0] = g_unk0x0053d1b0 * 2);
}

BYTE *RallyData_FUN_00421440(int index);
void FUN_00422f90(unsigned int index, int value);
int FUN_00423f30(void);
int FUN_0041f3a0(void);

// Sets the player's view distance from the route node's limits (forward or
// backward direction).
// match 55%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00459250
void FUN_00459250(BYTE player, unsigned int node, int dir)
{
    BYTE *pNode;
    int nearDist;
    int farDist;
    int distance;
    int *pFar = (int *)((BYTE *)g_pGraphics + 0x3c8);
    int *pNear = (int *)((BYTE *)g_pGraphics + 0x3c4);

    if (player < (BYTE)RallyDataState()) {
        if (dir < 0) {
            node++;
            if ((unsigned int)RallyData_FUN_00421420() <= node)
                node = RallyData_FUN_00421420() - 1;
        }
        if ((BYTE)RallyDataState() > 1 && FUN_0041f3a0() == 0)
            CGameInfo::FUN_00405da0();
        pNode = RallyData_FUN_00421440(node);
        if (dir < 0) {
            nearDist = *(int *)(pNode + 0x1c);
            farDist = *(int *)(pNode + 0x20);
        } else {
            nearDist = *(int *)(pNode + 0x24);
            farDist = *(int *)(pNode + 0x28);
        }
        if (farDist > 0) {
            *pFar = farDist;
            distance = FUN_00423f30();
            if (*pFar < distance)
                distance = *pFar;
            FUN_00422f90(player, distance);
        }
        if (nearDist > 0) {
            *pNear = nearDist;
            distance = FUN_00423f30();
            if (*pFar < distance)
                distance = *pFar;
            FUN_00422f90(player, distance);
        }
    }
}

void FUN_004583d0(int car, int *pStarts, int *pOut);

// Start position of a car in the .csp start table.
// FUNCTION: CMR2 0x00456c70
int *FUN_00456c70(int car)
{
    FUN_004583d0(car, *(int **)(g_unk0x00542630 + 0x240), (int *)(g_unk0x00542630 + 0x248));
    return (int *)(g_unk0x00542630 + 0x248);
}

struct FixAngles;
SceneNode *Scene_CreateLight(int type, int r, int g, int b, FixVector *pPosition, FixAngles *pAngles, SceneNode *pParent);
void Scene_SetAmbient(BYTE *pColour, int boost);
void Scene_SetLight(FixVector *pLight, int boost);
int RallyData_FUN_00411060(void);
extern SceneNode *g_stageAmbientNode;
extern BYTE g_unk0x00592146;
extern BYTE g_stageColourAlpha;

// Creates the stage light with full white ambient and directional light.
// match 55%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004918d0
void FUN_004918d0(void)
{
    BYTE ambient[4];
    unsigned short angles[4];
    FixVector light;
    FixVector position;
    char country;

    light.x = 0xff0000;
    light.y = 0xff0000;
    light.z = 0xff0000;
    ambient[0] = 0xff;
    ambient[1] = 0xff;
    ambient[2] = 0xff;
    ambient[3] = 0xff;
    position.x = 0x320000;
    position.y = 0x4b0000;
    position.z = 0x4b0000;
    angles[0] = 0;
    angles[1] = 0;
    angles[2] = 0;
    angles[3] = 0;
    g_stageAmbientNode = Scene_CreateLight(2, 0x10000, 0x10000, 0x10000, &position, (FixAngles *)angles,
                                           (SceneNode *)RallyData_FUN_00411060());
    country = (char)RallyDataCountryIndex();
    if (country != 3)
        Scene_SetAmbient(ambient, 1);
    else
        Scene_SetAmbient(ambient, 0);
    Scene_SetLight(&light, country != 3);
    g_unk0x00592146 = 0xff;
    g_stageColourAlpha = 0xff;
}

extern int g_unk0x00588970[8];
extern const double g_unk0x00511310;

void FUN_004694a0(int param_1, int param_2, char param_3);
void Mesh_Rebuild(Mesh *pMesh);
void RallyData_ValidateIndex(int index);
void FUN_00477c20(int index, char set0, char set1, BYTE mask);

// Rebuilds the wheel/part meshes of a damaged car from its record's vertex
// data, then clears the record's damage state.
// match 18%: logic matches (6 FixMul-style float stores per part, then the record is cleared) but MSVC6
// splits the six int temporaries across 7 stack slots and allocates EBP/ESI/EDI differently; not reproducible
// without the original local layout. Kept as FUNCTION so reccmp measures it.
// FUNCTION: CMR2 0x004698a0
void FUN_004698a0(int pCar)
{
    int i;
    int j;
    int n;
    int m;
    int record;
    int *p;
    int *pSrc;
    int v0;
    int v1;
    int v2;
    int w0;
    int w1;
    int w2;
    int q;
    int idx;

    idx = *(char *)(pCar + 0xb1a);
    if (g_unk0x00588970[idx] != 0) {
        i = 0;
        record = idx * 0x4d0 + (int)g_unk0x00588b94;
        if (0 < *(int *)(record + 0x45c)) {
            p = (int *)(record + 0x420);
            do {
                j = 0;
                if (0 < *p) {
                    n = 0;
                    m = 0;
                    do {
                        n += 0x30;
                        j++;
                        pSrc = (int *)(*(int *)(record + 0x78) + m);
                        v0 = pSrc[0];
                        v1 = pSrc[1];
                        v2 = pSrc[2];
                        *(float *)(*(int *)(*(int *)(record) + 0xc) + n - 0x30) = (float)(v0 * g_unk0x00511310);
                        *(float *)(*(int *)(*(int *)(record) + 0xc) + n - 0x2c) = (float)(v1 * g_unk0x00511310);
                        *(float *)(*(int *)(*(int *)(record) + 0xc) + n - 0x28) = (float)(v2 * g_unk0x00511310);
                        pSrc = (int *)(*(int *)(record + 0x78) + m + 0xc);
                        m += 0x20;
                        w0 = pSrc[0];
                        w1 = pSrc[1];
                        w2 = pSrc[2];
                        *(float *)(*(int *)(*(int *)(record) + 0xc) + n - 0x24) = (float)(w0 * g_unk0x00511310);
                        *(float *)(*(int *)(*(int *)(record) + 0xc) + n - 0x20) = (float)(w1 * g_unk0x00511310);
                        *(float *)(*(int *)(*(int *)(record) + 0xc) + n - 0x1c) = (float)(w2 * g_unk0x00511310);
                    } while (j < *p);
                }
                q = *(int *)(*(int *)(record + 0x3c) + 0xc);
                if (q != 0) {
                    Mesh_Rebuild((Mesh *)q);
                    RallyData_ValidateIndex(q);
                    Scene_MarkShadowPartDirty(*(SceneNode **)(pCar + 0x720), (Mesh *)q);
                }
                i++;
                p++;
            } while (i < *(int *)(record + 0x45c));
        }
        p = (int *)(record + 0x21c);
        for (n = 0; n < 9; n++)
            p[n] = 0;
        *(BYTE *)(record + 0x469) = 0;
        *(int *)(record + 0x46c) = 0;
        p = (int *)(record + 0x350);
        for (n = 0; n < 0x22; n++)
            p[n] = 0;
        p = (int *)(record + 0x470);
        for (n = 0; n < 8; n++) {
            p[8] = 0;
            p[0] = 0;
            FUN_004694a0(pCar, 0, n);
            p++;
        }
        FUN_00477c20(*(char *)(pCar + 0xb1a), 0, 0, 4);
    }
}

// Snapshots a car's replay colours and end values once per stage.
// FUNCTION: CMR2 0x00469a80
void FUN_00469a80(int car)
{
    Car *pCar = Car_Get(car);
    BYTE *pSource = g_unk0x00588b94 + pCar->field_0xb1a * 0x4d0;
    BYTE *pRecord = g_unk0x00588b98 + pCar->field_0xb1a * 0x290;
    int i;
    int value;

    if (*(int *)(pRecord + 0x28c) == 0) {
        *(int *)(pRecord + 0x28c) = 1;
        for (i = 0; i < 0x22; i++) {
            value = FixMul(((int *)(pSource + 0x240))[i], 0xff0000) >> 16;
            if (value > 0xff)
                value = 0xff;
            pRecord[0x24c + i] = (BYTE)value;
        }
        for (i = 0; i < 3; i++)
            ((int *)(pRecord + 0x280))[i] = ((int *)(pSource + 0x4c0))[i];
        for (i = 0; i < 4; i++)
            ((int *)(pRecord + 0x270))[i] = ((int *)(pSource + 0x4b0))[i];
    }
}

// Inserts a driver's split time into the ranking of a split.
// match 49%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00455af0
void FUN_00455af0(int driver, int hundredths, int split)
{
    int time = FUN_0040d4b0(hundredths);
    int slot = driver;
    int pos;
    int i;
    char c;

    if (driver < g_unk0x00541f98)
        slot = g_unk0x00541f90[FUN_0041b370() & 0xff];
    g_stageSplitTimesRaw[split][slot] = time;
    for (pos = 0; g_stageSplitTimesRaw[split][g_stageSplitTimesRawDriverIx[split][pos]] < time &&
                  pos != g_stageSplitDriverCount[split];) {
        pos++;
        if (pos > 15)
            goto done;
    }
    for (i = 15; pos < i; i--) {
        g_stageSplitTimesRawDriverIx[split][i] = g_stageSplitTimesRawDriverIx[split][i - 1];
        c = g_stageSplitDriverIndices[split][i - 1];
        g_stageSplitDriverIndices[split][i] = c;
        g_stageSplitPositions[split][c]++;
    }
    g_stageSplitTimesRawDriverIx[split][pos] = (char)slot;
    g_stageSplitDriverIndices[split][pos] = (char)driver;
    g_stageSplitPositions[split][driver] = (char)pos;
done:
    g_stageSplitDriverCount[split]++;
}

extern double g_minus65536;

// match 47%: the whole FixMul/FixDiv chain matches instruction for instruction, but MSVC6 assigns the
// stack slots in a different order (the original spills into the dead parameter homes, EBP+8/EBP+0xc);
// only the [ebp-N] numbers and a few load orders differ. Kept as FUNCTION.
// Bilinear blend of a car's 5-byte-wide paint-decal rows (0x54241c) into 16
// fixed-point samples, weighted by the car's two paint offsets (0xa8/0xac).
// FUNCTION: CMR2 0x00455f00
void FUN_00455f00(int pCar, int *pOut)
{
    __int64 v;
    int t;
    int a8;
    int r8;
    int ac;
    int rc;
    int sum;
    int d0;
    int d3;
    int d4;
    int t1;
    int t2;
    int t3;
    int m1;
    int m2;
    int m3;
    int m4;
    int m5;
    int m6;
    int m7;
    int m8;
    int m9;
    int m10;
    int m11;
    int t4;
    int i;

    v = (unsigned int)RallyData_FUN_00421420();
    v = (__int64)((double)v * CGraphics::m_65536);
    t = (int)v;
    // The original computes this and discards it (inline __asm FixMul is opaque
    // to MSVC6, so it cannot be eliminated); reproduced to match the codegen.
    FixMul(0x20000, t);
    a8 = *(int *)(pCar + 0xa8);
    ac = *(int *)(pCar + 0xac);
    r8 = 0x10000 - a8;
    rc = 0x10000 - ac;
    sum = r8 + rc;
    for (i = 0; i < 0x50; i += 5, pOut++) {
        d0 = (int)(__int64)(g_unk0x0054241c[i] * CGraphics::m_65536);
        d3 = (int)(__int64)(g_unk0x0054241c[i + 3] * CGraphics::m_65536);
        d4 = (int)(__int64)(g_unk0x0054241c[i + 4] * CGraphics::m_65536);
        t1 = FixDiv(*(int *)(pCar + 0xa0), t);
        t2 = FixDiv(t - *(int *)(pCar + 0xa4), t);
        t3 = 0x10000 - t2 - t1;
        m1 = FixMul(r8, d3);
        m2 = FixMul(d4, m1);
        m3 = FixMul(a8, d4);
        m4 = FixMul(d4, m3);
        m5 = FixMul(rc, d3);
        m6 = FixMul(t2, m5);
        m7 = FixMul(ac, d4);
        m8 = FixMul(t2, m7);
        t4 = FixDiv(sum, 0x20000);
        m9 = FixMul(t4, d3);
        m10 = FixMul(t3, m9);
        m11 = FixMul(0x10000 - t4, d4);
        *pOut = FixMul(t3, m11) + m10 + m8 + m6 + m4 + m2 + d0;
    }
}

// Adds a random spread to the computer drivers' times and sorts them.
// match 84%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00456110
void FUN_00456110(int *pTimes)
{
    int split = GetStageSplitCount();
    int i;
    int *p = pTimes;
    int low;
    int high;
    unsigned int r;

    for (i = 0; i < 0x50; i += 5, p++) {
        low = (int)(__int64)(g_unk0x0054241c[i + 2] * CGraphics::m_65536);
        high = (int)(__int64)(g_unk0x0054241c[i + 1] * g_minus65536);
        r = (unsigned int)(__int64)(rand() * CGraphics::m_65536);
        *p += FixMul(FixDiv(r, 0x7fff0000), -low - high) + low;
    }
    RallyTiming_SortOrder(pTimes, g_stageSplitDriverIndices[split], 0, g_unk0x00541f98, 1);
}

extern int g_unk0x00542c7c[12];
extern int g_unk0x00542c74;

// Advances a car's lap record when it reaches the next checkpoint on time.
// FUNCTION: CMR2 0x00458fd0
void FUN_00458fd0(int car, int time)
{
    int count;

    if (g_unk0x00542e78[car].field_0x2 == g_unk0x00542e78[car].field_0x8 &&
        (int)(__int64)(time * CGraphics::m_65536) == g_unk0x00542c7c[g_unk0x00542e78[car].field_0xa]) {
        g_unk0x00542e78[car].field_0x17 = 1;
        g_unk0x00542e78[car].field_0x4 = g_unk0x00542e78[car].field_0x8;
        g_unk0x00542e78[car].field_0x6 = g_unk0x00542e78[car].field_0xa;
        g_unk0x00542e78[car].field_0xa++;
        if (g_unk0x00542e78[car].field_0x6 == 0)
            g_unk0x00542e78[car].field_0x19 = 1;
        count = g_unk0x00542c74;
        if (g_unk0x00542e78[car].field_0xa == count) {
            g_unk0x00542e78[car].field_0xa = 0;
            g_unk0x00542e78[car].field_0x8++;
        }
        g_unk0x00542e78[car].field_0x14++;
        if (g_unk0x00542e78[car].field_0x14 > count)
            g_unk0x00542e78[car].field_0x14 = 1;
    }
}

// Copies the three vertices of a triangle (indices in pIndices).
// FUNCTION: CMR2 0x004917f0
void FUN_004917f0(int *pOut, unsigned short *pIndices, int unused)
{
    pOut[0] = *(int *)(g_unk0x00591b14 + pIndices[0] * 0xc);
    pOut[1] = *(int *)(g_unk0x00591b14 + 4 + pIndices[0] * 0xc);
    pOut[2] = *(int *)(g_unk0x00591b14 + 8 + pIndices[0] * 0xc);
    pOut[3] = *(int *)(g_unk0x00591b14 + pIndices[1] * 0xc);
    pOut[4] = *(int *)(g_unk0x00591b14 + 4 + pIndices[1] * 0xc);
    pOut[5] = *(int *)(g_unk0x00591b14 + 8 + pIndices[1] * 0xc);
    pOut[6] = *(int *)(g_unk0x00591b14 + pIndices[2] * 0xc);
    pOut[7] = *(int *)(g_unk0x00591b14 + 4 + pIndices[2] * 0xc);
    pOut[8] = *(int *)(g_unk0x00591b14 + 8 + pIndices[2] * 0xc);
}

int RallyData_FUN_004209d0(BYTE *p);
int *FUN_00407520(int index);

// Third ramp of a view's object state: value by route position plus the
// car's progress within the node.
// match 75%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0045e8b0
void FUN_0045e8b0(unsigned int *pRecord, int view)
{
    Car *pCar;
    unsigned int position;

    if (RallyData_FUN_00421500() == 0) {
        pRecord[6] = pRecord[4];
        pCar = Car_Get(FUN_00422fb0(view));
        position = RallyData_FUN_00421370((BYTE *)pCar);
        pRecord[0] = position;
        if (position != pRecord[1]) {
            pRecord[1] = position;
            if (position > (unsigned int)g_unk0x00543d94) {
                if (position < (unsigned int)g_unk0x00543d98)
                    pRecord[2] = FixMul((position - g_unk0x00543d94) << 16, g_unk0x00543d90) + g_unk0x00543d88;
                else
                    pRecord[2] = g_unk0x00543d8c;
            } else {
                pRecord[2] = g_unk0x00543d88;
            }
        }
        if (pRecord[0] < (unsigned int)g_unk0x00543d98 && pRecord[0] >= (unsigned int)g_unk0x00543d94) {
            pRecord[3] = FixMul(g_unk0x00543d90, RallyData_FUN_004209d0((BYTE *)pCar));
            pRecord[4] = pRecord[2] + pRecord[3];
            return;
        }
        pRecord[3] = 0;
        pRecord[4] = pRecord[2] + pRecord[3];
    }
}

int FUN_00460c80(BYTE *pCar);
void Car_UpdateSurfaceParams(Car *pCar, int blend);
void FUN_004789b0(BYTE *pCar);

// Sets the third object ramp and resets every car's record and surface.
// match 59%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0045e710
void FUN_0045e710(int v1, int v2, int from, int to)
{
    int i;
    int *p;

    g_unk0x00543d74 = v1;
    g_unk0x00543d78 = v2;
    g_unk0x00543d84 = to;
    g_unk0x00543d7c = (to - from) << 16;
    g_unk0x00543d80 = from;
    if (g_unk0x00543d7c > 0)
        g_unk0x00543d7c = FixDiv(0x10000, g_unk0x00543d7c);
    g_unk0x00543d7c = FixMul(g_unk0x00543d7c, v2 - v1);
    for (i = 0; i < g_unk0x00547acc; i++) {
        p = (int *)((BYTE *)g_unk0x00543ecc + i * 0xc);
        p[1] = 0;
        p[0] = 0;
        p[2] = g_unk0x00543d74;
        Car_UpdateSurfaceParams(Car_Get(i), FUN_00460c80((BYTE *)Car_Get(i)));
        FUN_004789b0((BYTE *)Car_Get(i));
    }
}

// Weather tables by setting: type, and two blend values.
// GLOBAL: CMR2 0x0051b04c
int g_weatherType[9] = { 0, 0, 0, 1, 1, 1, 2, 2, 2 };
// GLOBAL: CMR2 0x0051b070
int g_weatherBlendA[9] = { 0, 32768, 65536, 26214, 45875, 65536, 26214, 45875, 65536 };
// GLOBAL: CMR2 0x0051b094
int g_weatherBlendB[9] = { 0, 0, 0, 45875, 55705, 65536, 45875, 55705, 65536 };
// GLOBAL: CMR2 0x00543d54
int g_unk0x00543d54;
// GLOBAL: CMR2 0x00543e9c
int g_unk0x00543e9c;
// GLOBAL: CMR2 0x00543fa8
int g_unk0x00543fa8;

// Sets up the weather change of the stage from its two settings.
// match 73%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0045ea70
void FUN_0045ea70(void)
{
    int *pPair = FUN_00407520(RallyDataStageIndex());
    int other = g_weatherType[pPair[1]];

    g_unk0x00543d54 = g_weatherType[pPair[0]];
    g_unk0x00543e9c = g_weatherBlendA[pPair[0]];
    g_unk0x00543fa8 = g_weatherBlendA[pPair[1]];
    g_unk0x00543e88 = g_weatherBlendB[pPair[0]];
    g_unk0x00543d9c = g_weatherBlendB[pPair[1]];
    if (g_unk0x00543d54 == 0) {
        if (other != 0) {
            g_unk0x00543e9c = 0;
            g_unk0x00543d54 = other;
        }
    } else if (other == 0) {
        g_unk0x00543fa8 = 0;
    }
    if (g_unk0x00543e9c != g_unk0x00543fa8) {
        g_unk0x00543e8c = (unsigned int)RallyData_FUN_00421420() / 5;
        g_unk0x00543e94 = RallyData_FUN_00421420() - g_unk0x00543e8c / 5;
        return;
    }
    g_unk0x00543e8c = 0;
    g_unk0x00543e94 = 0;
}

// Advances a car's checkpoint record on a looped stage.
// match 50%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004590a0
void FUN_004590a0(int car, int time)
{
    int count;

    if (g_unk0x00542e78[car].field_0x18 != 0) {
        if (g_unk0x00542e78[car].field_0x6 == g_unk0x00542c74 - 1)
            g_unk0x00542e78[car].field_0x19 = 1;
        g_unk0x00542e78[car].field_0x6 = 0;
        g_unk0x00542e78[car].field_0xa = 1;
        g_unk0x00542e78[car].field_0x14 = 0;
        g_unk0x00542e78[car].field_0x17 = 1;
        return;
    }
    if ((int)(__int64)(time * CGraphics::m_65536) == g_unk0x00542c7c[g_unk0x00542e78[car].field_0xa]) {
        g_unk0x00542e78[car].field_0x17 = 1;
        g_unk0x00542e78[car].field_0x6 = g_unk0x00542e78[car].field_0xa;
        g_unk0x00542e78[car].field_0xa++;
        count = g_unk0x00542c74;
        if (g_unk0x00542e78[car].field_0xa == count)
            g_unk0x00542e78[car].field_0xa = 0;
        g_unk0x00542e78[car].field_0x14++;
        if (count <= g_unk0x00542e78[car].field_0x14)
            g_unk0x00542e78[car].field_0x14 = 0;
    }
}

// Inserts the car's current time at its split into the split ranking.
// FUNCTION: CMR2 0x00456a00
void FUN_00456a00(int car, int driver)
{
    int split = FUN_00458370(car);

    if (CGameInfo::FUN_00405d80() != 4)
        FUN_00455af0(driver, FUN_00448110(), split);
}

int FUN_00407710(void);
int FUN_00407650(void);

// Sets up the three object ramps of the stage from its weather change.
// match 72%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0045f260
void FUN_0045f260(void)
{
    int *pPair = FUN_00407520(RallyDataStageIndex());
    short a = (short)FUN_00407710();
    short b = (short)FUN_00407650();
    int last;
    int from;
    int v2;

    if ((b != a || pPair[0] != pPair[1]) && RallyData_FUN_00421500() == 0) {
        last = RallyData_FUN_00421420() - 1;
        from = 1;
        v2 = 0x10000;
    } else {
        last = 0;
        from = 0;
        v2 = 0;
    }
    FUN_0045f3d0(0, v2, from, last);
    FUN_0045f300(g_unk0x00543e9c, g_unk0x00543fa8, g_unk0x00543e8c, g_unk0x00543e94, g_unk0x00543d54);
    FUN_0045e710(g_unk0x00543e88, g_unk0x00543d9c, g_unk0x00543e8c, g_unk0x00543e94);
}

BYTE FUN_004582b0(int index);

// Records a car's split time when it passes a split.
// FUNCTION: CMR2 0x004569c0
void FUN_004569c0(int car, int driver)
{
    if (FUN_00458250(car))
        FUN_00456a00(car, driver);
    if (FUN_004582b0(car))
        FUN_00456a30(car, driver);
}

// Road rumble strength of the two players (from the suspension travel).
// GLOBAL: CMR2 0x005391f8
int g_unk0x005391f8[2];
// GLOBAL: CMR2 0x00539270
int g_unk0x00539270[2];
// GLOBAL: CMR2 0x0053937c
BYTE *g_unk0x0053937c;
// GLOBAL: CMR2 0x00539380
int g_unk0x00539380;

// Builds the steering force from the two front-corner axes, the wheel speed,
// and the force-feedback slot's current centring/impact state.
// FUNCTION: CMR2 0x00424360
void FUN_00424360(void)
{
    FixVector *pForward;
    FixVector *pAxis;
    int left;
    int right;
    int force;
    int speed;

    if (g_unk0x00539270[0] == 0) {
        left = 0;
    } else {
        pAxis = (FixVector *)(g_unk0x0053937c + 0x4a4);
        pForward = (FixVector *)(g_unk0x0053937c + 0x378);
        left = FixMul(pAxis->x, pForward->x) + FixMul(pAxis->y, pForward->y) +
               FixMul(pAxis->z, pForward->z);
    }
    if (g_unk0x00539270[1] == 0) {
        right = 0;
    } else {
        pAxis = (FixVector *)(g_unk0x0053937c + 0x4b0);
        pForward = (FixVector *)(g_unk0x0053937c + 0x378);
        right = FixMul(pAxis->x, pForward->x) + FixMul(pAxis->y, pForward->y) +
                FixMul(pAxis->z, pForward->z);
    }

    force = g_unk0x005391f8[1] - g_unk0x005391f8[0] - right - left;
    if (FIX_ABS(force) > 0x10000)
        force = force > 0 ? 0x10000 : -0x10000;

    speed = FixMul(*(int *)(g_unk0x0053937c + 0x778) - 0x8000, 0x20000);
    if (speed > 0x10000)
        speed = 0x10000;
    else if (speed < 0)
        speed = 0;
    force = FixMul(speed, force);
    force = FixMul(force, 0x6666) + g_unk0x00539278->field_0x20;
    if (g_unk0x00539278->field_0x30 != 0)
        force += g_unk0x00539278->field_0x24;
    else
        force -= g_unk0x00539278->field_0x24;
    if (FIX_ABS(force) > 0x10000)
        force = force > 0 ? 0x10000 : -0x10000;

    speed = FixMul(g_unk0x00539278->field_0x18, 0x40000);
    force = FixMul(force, speed);
    if (FIX_ABS(force) > 0x10000)
        force = force > 0 ? 0x10000 : -0x10000;
    g_unk0x00539278->field_0x14 = FixMul(force, 0x27100000) >> 16;
}

// Computes each axle's rumble from its suspension travel and stiffness,
// plus the ground roughness, and their average.
// match 77%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004247a0
void FUN_004247a0(void)
{
    int i;
    int v;
    BYTE *p = g_unk0x0053937c;

    for (i = 0; i < 2; i++) {
        if (g_unk0x00539270[i] == 0) {
            g_unk0x005391f8[i] = 0;
        } else {
            v = *(int *)(p + 0x1a8 + i * 0xc) + *(int *)(p + 0x8c + i * 0x24);
            g_unk0x005391f8[i] = v;
            v = FixMul(v, *(int *)(p + 0x1a4 + i * 0xc) + *(int *)(p + 0x88 + i * 0x24));
            g_unk0x005391f8[i] = v;
            v = FixMul(v, 0x16e14);
            g_unk0x005391f8[i] = v;
            if (v < 0)
                g_unk0x005391f8[i] = 0;
            if (g_unk0x005391f8[i] > 0x9999)
                g_unk0x005391f8[i] = 0x9999;
            p = g_unk0x0053937c;
            g_unk0x005391f8[i] += *(int *)(p + 0x90 + i * 0x24);
            if (g_unk0x005391f8[i] > 0x10000)
                g_unk0x005391f8[i] = 0x10000;
        }
    }
    g_unk0x00539380 = FixMul(g_unk0x005391f8[1] + g_unk0x005391f8[0], 0x8000);
}

// Builds the two condition-effect coefficients from the car's velocity,
// ground contact and the force-feedback strength selected for this slot.
// FUNCTION: CMR2 0x004248a0
void FUN_004248a0(void)
{
    FixVector *pForward;
    FixVector *pVelocity;
    int contact;
    int spring;
    int damper;
    int speed;

    spring = FixMul(g_unk0x00539380, 0x6666) + 0x1999;
    damper = FixMul(g_unk0x00539380, 0x4ccc);
    if (g_unk0x00539270[0] != 0 && g_unk0x00539270[1] != 0) {
        pForward = (FixVector *)(g_unk0x0053937c + 0x378);
        pVelocity = (FixVector *)(g_unk0x0053937c + 0x408);
        contact = FixMul(pForward->x, pVelocity->x) + FixMul(pForward->y, pVelocity->y) +
                  FixMul(pForward->z, pVelocity->z);
        if (contact < 0)
            contact = -contact;
        contact = FixMul(contact - 0x1999, 0x28000);
        if (contact > 0x10000)
            contact = 0x10000;
        else if (contact < 0)
            contact = 0;
        spring -= FixMul(contact, 0x4ccc);
        if (spring < 0)
            spring = 0;
        damper -= FixMul(contact, 0x4ccc);

        if (*(int *)(g_unk0x0053937c + 0x778) <= 0x1999) {
            speed = 0x10000 - FixMul(*(int *)(g_unk0x0053937c + 0x778), 0xa0000);
            spring += FixMul(-spring, speed);
            if (spring < 0)
                spring = 0;
            damper += FixMul(0x8000, speed);
            if (damper > 0x10000)
                damper = 0x10000;
        }
    }

    speed = FixMul(g_unk0x00539278->field_0x1c, 0x40000);
    spring = FixMul(spring, speed);
    if (FIX_ABS(spring) > 0x10000)
        spring = spring > 0 ? 0x10000 : -0x10000;
    speed = FixMul(g_unk0x00539278->field_0x1c, 0x40000);
    damper = FixMul(damper, speed);
    if (FIX_ABS(damper) > 0x10000)
        damper = damper > 0 ? 0x10000 : -0x10000;
    g_unk0x00539278->field_0xc = FixMul(spring, 0x27100000) >> 16;
    g_unk0x00539278->field_0x10 = FixMul(damper, 0x27100000) >> 16;
}

// Releases the stage's files (registered callback of 0x455080).
// FUNCTION: CMR2 0x00454f20
int FUN_00454f20(void)
{
#define RELEASE_STAGE_FILE(i)                                   \
    if (g_stageFiles[i].buffer != NULL)                         \
        CFileBuffer::FreeGenericFileBuffer(g_stageFiles[i].buffer); \
    g_stageFiles[i].buffer = NULL;                              \
    g_stageFiles[i].loaded = 0;                                 \
    g_stageFiles[i].size = 0;

    RELEASE_STAGE_FILE(0)
    RELEASE_STAGE_FILE(1)
    RELEASE_STAGE_FILE(2)
    RELEASE_STAGE_FILE(6)
    RELEASE_STAGE_FILE(3)
    RELEASE_STAGE_FILE(4)
    RELEASE_STAGE_FILE(5)
    return 1;
#undef RELEASE_STAGE_FILE
}

void FUN_004569c0(int car, int driver);
unsigned int RallyData_FUN_00407e90(void);

// Records the split times of every car in the current split group.
// FUNCTION: CMR2 0x00455590
void FUN_00455590(int group)
{
    int i;

    if ((BYTE)RallyData_FUN_00407e90()) {
        for (i = 1; i >= 0; i--) {
            if (((BYTE *)&g_unk0x00542418)[i] == 0)
                FUN_004569c0(i, g_stageSplitUnk0x00541f78[group][i]);
        }
        return;
    }
    for (i = 0; i < (char)g_unk0x00542528[0xc0 + group]; i++) {
        if (((BYTE *)&g_unk0x00542418)[i] == 0)
            FUN_004569c0(i, (char)g_unk0x00542528[0xc8 + group * 2 + i]);
    }
}

extern float g_oneOverRandMax;
extern double g_minus65536;

// Random force-feedback road noise of the current slot, scaled by speed and
// the suspension movement.
// match 57%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00424af0
void FUN_00424af0(void)
{
    int speed;
    int k;

    if (g_unk0x00539278->field_0x20 == 0)
        g_unk0x00539278->field_0x20 = (int)(__int64)(rand() * g_oneOverRandMax * (float)CGraphics::m_65536);
    else if (g_unk0x00539278->field_0x20 < 1)
        g_unk0x00539278->field_0x20 = (int)(__int64)(rand() * g_oneOverRandMax * (float)CGraphics::m_65536);
    else
        g_unk0x00539278->field_0x20 = (int)(__int64)(rand() * g_oneOverRandMax * (float)g_minus65536);
    speed = FixMul(*(int *)(g_unk0x0053937c + 0x778), 0x10000);
    if (speed > 0x10000)
        speed = 0x10000;
    k = FixMul(*(int *)(g_unk0x0053937c + 0xc0) + *(int *)(g_unk0x0053937c + 0x9c), speed) + g_unk0x00539278->field_0x28;
    if (k > 0x10000)
        k = 0x10000;
    g_unk0x00539278->field_0x20 = FixMul(g_unk0x00539278->field_0x20, k);
}

// Attaches a stage object to the current car and copies its matrices.
// match 50%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004813b0
int FUN_004813b0(int slot)
{
    BYTE *object;

    if (*(int *)g_unk0x00590c20 != 0)
        return 0;
    object = g_unk0x00590b7c[slot][(signed char)((BYTE *)g_unk0x00590d74)[0xb1a]];
    if (object == NULL)
        return 0;

    *(BYTE **)g_unk0x00590c20 = object;
    *(BYTE **)((BYTE *)g_unk0x00590c20 + 4) = (BYTE *)g_unk0x00590c20 + 0xc;
    *(BYTE **)((BYTE *)g_unk0x00590c20 + 8) = (BYTE *)g_unk0x00590c20 + 0xcc;
    memcpy((BYTE *)g_unk0x00590c20 + 0xc, object + 0x98, 0x40);
    memcpy((BYTE *)g_unk0x00590c20 + 0xcc, object + 0xd8, 0x40);
    *(BYTE **)((BYTE *)g_unk0x00590c20 + 0x10c) = *(BYTE **)((BYTE *)g_unk0x00590d74 + 0x71c) + 0x98;
    memcpy(*(BYTE **)((BYTE *)g_unk0x00590c20 + 8), *(BYTE **)((BYTE *)g_unk0x00590d74 + 0x720) + 0xd8, 0x40);
    FixMatrix_GetRight((FixVector *)((BYTE *)g_unk0x00590c20 + 0x17c), (FixMatrix *)(object + 0x98));
    FixMatrix_GetUp((FixVector *)((BYTE *)g_unk0x00590c20 + 0x188), (FixMatrix *)(object + 0x98));
    FixMatrix_GetForward((FixVector *)((BYTE *)g_unk0x00590c20 + 0x194), (FixMatrix *)(object + 0x98));
    return 1;
}

extern char g_stageLooped;
extern int g_stageCheckpointCount;

// Advances one car through crossed checkpoints.
// match 34%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00458e00
void FUN_00458e00(int car, int target)
{
    Unk0x00542e78 *p = &g_unk0x00542e78[car];
    int current = p->field_0x0;
    int delta;
    int step;
    int i;

    if (target == current)
        return;
    delta = target - current;
    if (g_stageLooped == 0)
        step = delta > 0 ? 1 : -1;
    else if (delta < -50 || (delta > 0 && delta < 50))
        step = 1;
    else
        step = -1;
    for (i = 0; current != target && i < g_stageCheckpointCount; i++) {
        p->field_0x12 += (short)step;
        if ((unsigned short)p->field_0x10 < (int)p->field_0x12)
            p->field_0x10 = p->field_0x12;
        int next = current + step;
        if (next < 0)
            next = g_stageCheckpointCount - 1;
        if (next >= g_stageCheckpointCount)
            next = 0;
        if (g_stageLooped)
            FUN_00458f30(car, current, next);
        if (g_unk0x00542cad == 0)
            FUN_00458fd0(car, next);
        else
            FUN_004590a0(car, next);
        FUN_00459180(car);
        FUN_00459250(car, next, step);
        current = next;
    }
    p->field_0x0 = (short)target;
    p->field_0x16 = 1;
}

// GLOBAL: CMR2 0x00590b10
int g_unk0x00590b10[8];
// GLOBAL: CMR2 0x00511380
extern const double g_unk0x00511380 = 0.0099471839432434591;

// Smooths the active car's steering offset and derives a short angle.
// match 71%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00480cb0
void FUN_00480cb0(void)
{
    int speed = FixMul(*(int *)((BYTE *)g_unk0x00590d74 + 0x778), FixDiv(0x10000, 0x18000));
    int step;
    int target;
    int index;
    int delta;
    int phase;

    if (speed > 0x10000)
        speed = 0x10000;
    step = FixMul(FixMul(speed, g_physicsTimeStep), 0xf5c);
    target = FixMul(FixMul(speed, g_physicsTimeStep), 0x3333);
    phase = *(int *)((BYTE *)g_unk0x00590d74 + 0x2d0) + *(int *)((BYTE *)g_unk0x00590d74 + 0x2d8);
    if (phase < 0)
        phase = -phase;
    phase %= 1024;
    target = FixMul(target, phase << 6);
    index = (signed char)((BYTE *)g_unk0x00590d74)[0xb1a];
    delta = target - g_unk0x00590b10[index];
    if (delta < -step)
        target = g_unk0x00590b10[index] - step;
    else if (delta > step)
        target = g_unk0x00590b10[index] + step;
    g_unk0x00590b10[index] = target;
    *(short *)&g_unk0x00590c68 = (short)(int)(__int64)((double)target * g_unk0x00511380);
}

BYTE FUN_0042b710(int index);
unsigned short FUN_0040bbc0(unsigned short slot);
unsigned int FUN_0040bc00(unsigned short slot);
unsigned int FUN_0040bc30(unsigned short slot);
DWORD FUN_0040bcd0(unsigned short slot);
DWORD FUN_0040bd30(unsigned short slot);

// Initializes the two force feedback slots and registers their stop callback.
// FUNCTION: CMR2 0x00423ff0
void FUN_00423ff0(void)
{
    int i;

    CInput::ResetForceFeedbackEffects();
    for (i = 0; i < 2; i++) {
        g_forceFeedbackSlots[i].field_0x2c = -1;
        g_forceFeedbackSlots[i].field_0x34 = 0;
    }
    for (i = 0; i < (BYTE)RallyDataState(); i++) {
        BYTE device;

        g_unk0x00539278 = &g_forceFeedbackSlots[i];
        device = FUN_0042b710(Car_Get(i)->field_0xb1a);
        CInput::FUN_0049ead0((signed char)device);
        if (FUN_0040bd30(i) != 0) {
            device = FUN_0042b710(Car_Get(i)->field_0xb1a);
            CInput::FUN_004aaf50(FUN_0040bcd0(i), (signed char)device);
            g_unk0x00539278->field_0x18 = FUN_0040bc00(i);
            g_unk0x00539278->field_0x1c = FUN_0040bc30(i);
            g_unk0x00539278->field_0x2c = (signed char)FUN_0040bbc0(i);
            if (g_unk0x00539278->field_0x2c >= 0) {
                FUN_00424120();
                for (int j = 0; j < 3; j++) {
                    (&g_unk0x00539278->field_0x0)[j] = 0;
                    (&g_unk0x00539278->field_0xc)[j] = 0;
                }
                g_unk0x00539278->field_0x20 = 0;
                g_unk0x00539278->field_0x24 = 0;
                g_unk0x00539278->field_0x28 = 0;
            }
        }
    }
    FUN_00424640();
    CGame::RegisterCallback(FUN_00424640, NULL);
}

#include "WheelTrail.h"

extern int Car_GetWheelSpeed(Car *, BYTE, int);
extern int StageObject_GetWheelSlip(int, int);
extern BYTE FUN_00460bf0(int);
extern int FUN_00460c10(int);
extern int FUN_0041f3d0(BYTE);
extern void Tyre_AddWear(int, int, int, int);
extern void Particle_Spawn(int, FixVector *, FixVector *, int, int, BYTE *, BYTE, int, BYTE);

#define TRAIL_RANDOM(scale) ((int)(__int64)(rand() * g_oneOverRandMax * (scale)))

// Chooses wheel spray/dust from the surface, then interpolates its spawn position.
// match 41%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0045b580
void StageTiming_SpawnWheelParticles(int carIndex)
{
    BYTE colour[4] = { 255, 255, 255, 255 };
    if (carIndex >= 8) return;
    Car *car = Car_Get(carIndex);
    int intensity = 100;
    for (int wheel = 0; wheel < 4; wheel++) {
        int other = wheel ^ 1;
        int front = wheel == 2 || wheel == 3;
        int reverse = car->field_0xb1e == 7;
        int leading = reverse ? front : !front;
        int surface = car->wheelSurface[wheel];
        short material = car->wheelSurfaceType[wheel];
        int spray = surface == 11 || material == 0x5b || material == 0x5c || material == 0x5d;
        int loose = surface == 10 || surface == 6 || material == 0x1b || material == 0x1c ||
                    material == 0x11 || material == 0x12 || material == 0x48;
        int gravel = surface == 9;
        int slipping = 0;
        if (surface == 13 || surface == 24)
            slipping = StageObject_GetWheelSlip(carIndex, wheel) >= 0xe666;
        if (gravel && TRAIL_RANDOM(CGraphics::m_65536) > 0x3333) gravel = 0;
        if (loose && TRAIL_RANDOM(CGraphics::m_65536) > 0x6666) loose = 0;
        if (spray && TRAIL_RANDOM(CGraphics::m_65536) > 0x8000) spray = 0;
        RallyDataCountryIndex();
        int dust = ((BYTE)RallyDataCountryIndex() != 0 && (spray || loose || gravel)) || slipping;
        if (FUN_00460bf0(carIndex) == 1 && (FixMul(FUN_00460c10(carIndex), 0xff0000) >> 16) > 1) {
            dust = 0;
        } else if (dust) {
            int speed = FIX_ABS(Car_GetWheelSpeed(car, 0, 0));
            if (speed > 0x1e0000 && carIndex < (BYTE)RallyDataState())
                Tyre_AddWear(carIndex, wheel, 1, 0);
        }
        if (leading) {
            dust = 0;
        } else if (dust) {
            colour[0] = 0xeb; colour[1] = 0xb8; colour[2] = 0xa0;
            int brown = material == 2 || material == 3 || material == 4 || material == 5 ||
                        material == 0x11 || material == 0x1b || material == 0x5d;
            int dark = material == 8 || material == 9 || material == 10 || material == 0x12 ||
                       material == 0x1c || material == 0x5b;
            int white = material == 6 || material == 10;
            switch ((BYTE)RallyDataCountryIndex()) {
            case 0: colour[0] = 0x90; colour[1] = 0x7f; colour[2] = 0x78; break;
            case 1:
                if (brown) break;
                // Fall through to the grey surface colour.
            case 8: colour[0] = 0x9c; colour[1] = 0x8f; colour[2] = 0x87; break;
            case 4: colour[0] = 0xb6; colour[1] = 0x91; colour[2] = 0x43; break;
            case 5:
                if (white) { colour[0] = colour[1] = colour[2] = 255; }
                else if (dark) { colour[0] = 0x7a; colour[1] = 0x71; colour[2] = 0x5c; }
                else { colour[0] = 0xbb; colour[1] = 0x86; colour[2] = 0x3e; }
                break;
            case 7: dust = 0; break;
            default: colour[0] = colour[1] = colour[2] = 0xf0; break;
            }
        }
        int water = surface == 0x11 || surface == 0x13 || surface == 0x14 ||
                    surface == 0x15 || surface == 0x16 || surface == 0x17;
        int deep = surface == 0x13 || surface == 0x14 || surface == 0x15;
        if (!leading || deep) {
            if (water) colour[0] = colour[1] = colour[2] = 255;
        } else {
            water = 0;
        }
        if ((leading && FUN_0041f3d0((BYTE)carIndex)) ||
            (water && leading && FUN_00460bf0(carIndex) == 2 &&
             (FixMul(FUN_00460c10(carIndex), 0xff0000) >> 16) > 100)) water = 0;
        int emit = dust || water;
        if (!car->field_0xbac[wheel]) emit = 0;
        int speedLimit = leading ? 0x320000 : 0x230000;
        int type = 4;
        int speed = FIX_ABS(Car_GetWheelSpeed(car, 0, 0));
        if (speed < 0x1e0000) {
            int chance = FixDiv(speed, 0x1e0000);
            if (chance < TRAIL_RANDOM(CGraphics::m_65536)) emit = 0;
        }
        if (leading) {
            type = 6;
            if (!FUN_0041f3d0((BYTE)carIndex) && TRAIL_RANDOM(CGraphics::m_65536) > 0xb333) emit = 0;
        }
        if (carIndex > 0 && TRAIL_RANDOM(CGraphics::m_65536) > 0x8000) continue;
        if (!emit) continue;
        if (type == 4) rand();

        FixVector delta, rolling, lateral, velocity, cornerMotion, jitter, position, blend, source, particleVelocity;
        delta.x = g_trailPos[carIndex][wheel].x - g_trailLastPos[carIndex][wheel].x;
        delta.y = g_trailPos[carIndex][wheel].y - g_trailLastPos[carIndex][wheel].y;
        delta.z = g_trailPos[carIndex][wheel].z - g_trailLastPos[carIndex][wheel].z;
        int slipA = FIX_ABS(car->field_0x880[wheel]) - 0xccc;
        int slipB = FIX_ABS(car->field_0x870[wheel]) - 0xccc;
        FixVector *cornerVelocity = &car->cornerVelocity[wheel];
        cornerMotion = *cornerVelocity;
        int random;
        if (front) {
            FixVecScale(&rolling, &car->groundDir[1], car->field_0x870[wheel]);
            random = TRAIL_RANDOM(g_minus65536);
            FixVecScale(&lateral, &car->groundAxis[1], -0x8000 - random);
        } else {
            FixVecScale(&rolling, &car->groundDir[0], car->field_0x870[wheel]);
            random = TRAIL_RANDOM(g_minus65536);
            FixVecScale(&lateral, &car->groundAxis[0], -0x8000 - random);
        }
        int uniform;
        if (type == 5) {
            FixVecScale(&lateral, &lateral, 0x1999);
            uniform = 1;
        } else {
            FixVecScale(&lateral, &lateral, 0x4ccc);
            uniform = 0;
        }
        if (!leading) uniform = 1;
        FixVecScale(&rolling, &rolling, 0x1999);
        velocity.x = rolling.x + cornerMotion.x + lateral.x;
        velocity.y = cornerMotion.y + rolling.y + lateral.y;
        velocity.z = rolling.z + cornerMotion.z + lateral.z;
        if ((slipA > 0 || slipB > 0) && FixDiv(slipA, 0x20000) + FixDiv(slipB, 0x8000) > 0x8000)
            intensity = (intensity * 3) / 2;
        if (FUN_0041f3d0((BYTE)carIndex) || (*(BYTE **)(FUN_0041b390() + 4))[carIndex * 8] == 9) {
            type = 7;
            uniform = 1;
        }
        if (dust) type += 4;
        random = TRAIL_RANDOM(g_minus65536);
        FixVecScale(&jitter, &delta, random);
        int t;
        if (!leading) {
            t = TRAIL_RANDOM(CGraphics::m_65536);
            if (!uniform) {
                random = TRAIL_RANDOM(g_minus65536);
                t = FixDiv(t, 0x1547a - random * 9);
            }
            FixVecScale(&position, &g_trailPos[carIndex][other], t);
            t = 0x10000 - t;
            FixVecScale(&blend, &g_trailPos[carIndex][wheel], t);
        } else {
            t = TRAIL_RANDOM(CGraphics::m_65536);
            if (!uniform) {
                random = TRAIL_RANDOM(g_minus65536);
                t = FixDiv(t, 0x1547a - random * 9);
                random = TRAIL_RANDOM(g_minus65536);
                t = FixDiv(t, 0x30000 - random * 14);
            }
            FixVecScale(&position, &g_trailPos[carIndex][other], t);
            t = 0x10000 - t;
            FixVecScale(&blend, &g_trailPos[carIndex][wheel], t);
        }
        position.x += blend.x;
        position.y += blend.y;
        position.z += blend.z;
        if (type != 6 && type != 10 && type != 4 && type != 8) {
            FixVector freeSource;
            freeSource.x = position.x - cornerVelocity->x;
            freeSource.y = position.y - cornerVelocity->y;
            freeSource.z = position.z - cornerVelocity->z;
            Particle_Spawn(type, &freeSource, cornerVelocity, position.y - 0x10000, 0, colour,
                           g_trailLevel[carIndex][wheel], (int)&carIndex, *((BYTE *)car->pNode0x720 + 0x17c));
        } else {
            source.x = position.x - car->position.x + jitter.x;
            source.y = position.y - car->position.y + jitter.y;
            source.z = position.z - car->position.z + jitter.z;
            if (type != 4 && type != 8 && !reverse && (wheel == 2 || wheel == 3)) {
                FixVector offset;
                offset.x = g_trailPos[carIndex][2].x - g_trailPos[carIndex][0].x;
                offset.y = g_trailPos[carIndex][2].y - g_trailPos[carIndex][0].y;
                offset.z = g_trailPos[carIndex][2].z - g_trailPos[carIndex][0].z;
                FixVecScale(&offset, &offset, 0x6666);
                source.x += offset.x; source.y += offset.y; source.z += offset.z;
            }
            particleVelocity.y = cornerVelocity->y / 10;
            particleVelocity.x = -(cornerVelocity->x / 4);
            particleVelocity.z = -(cornerVelocity->z / 4);
            speed = FIX_ABS(Car_GetWheelSpeed(car, 2, 0));
            if (speed > speedLimit) {
                int scale = FixDiv(speedLimit, speed);
                particleVelocity.x = FixMul(particleVelocity.x, scale);
                particleVelocity.y = FixMul(particleVelocity.y, scale);
                particleVelocity.z = FixMul(particleVelocity.z, scale);
            } else {
                FixVector offset;
                offset.x = velocity.x - cornerVelocity->x;
                offset.y = velocity.y - cornerVelocity->y;
                offset.z = velocity.z - cornerVelocity->z;
                FixVecScale(&offset, &offset, 0x6666);
                particleVelocity.x += offset.x; particleVelocity.y += offset.y; particleVelocity.z += offset.z;
            }
            Particle_Spawn(type, &source, &particleVelocity, source.y - 0x10000, 0, colour,
                           g_trailLevel[carIndex][wheel], (int)&carIndex, *((BYTE *)car->pNode0x720 + 0x17c));

        }
    }
}
#undef TRAIL_RANDOM

struct Unk0x0052ebc0;
Unk0x0052ebc0 *RallyData_FUN_00407610(int index);
void FUN_00508fa0(int index, int param2, BYTE param3);

// Camera-space dent parameters: apex, direction, reference direction, radius,
// advance step and the falloff/scale factors used by FUN_00508890.
// Defined in GameInfo.cpp (same address, one definition per symbol).
extern FixVector g_unk0x0082d120;
extern FixVector g_unk0x0082d12c;
extern FixVector g_unk0x0082d138;
extern int g_unk0x0082d144;
// GLOBAL: CMR2 0x0082d150
int g_unk0x0082d150;
// GLOBAL: CMR2 0x0082d154
int g_unk0x0082d154;
// GLOBAL: CMR2 0x0082d158
int g_unk0x0082d158;

// Applies the record's camera-space dent to every mesh: vertices inside the
// radius move along the dent direction, those in the falloff shell along their
// per-vertex limit direction, and each touched mesh is rebuilt.
// match 56%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00508890
void FUN_00508890(int *pRecord)
{
    if (FixVecDot(&g_unk0x0082d120, &g_unk0x0082d12c) >= 0)
        FixVecScale(&g_unk0x0082d12c, &g_unk0x0082d12c, -0x10000);

    FixVector offset;
    FixVecScale(&offset, &g_unk0x0082d12c, g_unk0x0082d150);
    g_unk0x0082d120.x += offset.x;
    g_unk0x0082d120.y += offset.y;
    g_unk0x0082d120.z += offset.z;

    int innerSquare = FixMul(g_unk0x0082d144, g_unk0x0082d144);
    int reciprocalInner = FixDiv(0x10000, g_unk0x0082d144);
    int outer = g_unk0x0082d154 + g_unk0x0082d144;
    int outerSquare = FixMul(outer, outer);
    int reciprocalFalloff = FixDiv(0x10000, g_unk0x0082d154);
    int shellScale = FixMul(g_unk0x0082d158, 0x3333);

    int meshIndex;
    for (meshIndex = 0; meshIndex < *(BYTE *)((BYTE *)pRecord + 0x26a); ++meshIndex) {
        int changed = 0;
        int vertexIndex;
        for (vertexIndex = 0;
             vertexIndex < *(USHORT *)((BYTE *)pRecord + 0x24c + meshIndex * 2);
             ++vertexIndex) {
            Mesh *pMesh = (Mesh *)*(int *)((BYTE *)pRecord + meshIndex * 4);
            FixVector position;
            position.x = (int)(__int64)(*(float *)((BYTE *)pMesh->pVertexData
                                                   + vertexIndex * 0x30 + 0x0) * CGraphics::m_65536);
            position.y = (int)(__int64)(*(float *)((BYTE *)pMesh->pVertexData
                                                   + vertexIndex * 0x30 + 0x4) * CGraphics::m_65536);
            position.z = (int)(__int64)(*(float *)((BYTE *)pMesh->pVertexData
                                                   + vertexIndex * 0x30 + 0x8) * CGraphics::m_65536);
            FixVector distance;
            distance.x = g_unk0x0082d120.x - position.x;
            distance.y = g_unk0x0082d120.y - position.y;
            distance.z = g_unk0x0082d120.z - position.z;
            int projection = FixVecDot(&distance, &g_unk0x0082d138);
            FixVecScale(&offset, &g_unk0x0082d138, projection);
            distance.x -= offset.x;
            distance.y -= offset.y;
            distance.z -= offset.z;
            int distanceSquare = FixVecDot(&distance, &distance);
            if (distanceSquare <= outerSquare) {
                FixVector original = position;
                if (distanceSquare <= innerSquare) {
                    int penetration = g_unk0x0082d144 - FixMul(reciprocalInner,
                                                               distanceSquare);
                    FixVecScale(&distance, &g_unk0x0082d12c, penetration);
                } else {
                    int length = FixSqrt(distanceSquare);
                    int shellWeight = FixMul(FixMul(length - g_unk0x0082d144,
                                                    reciprocalFalloff), shellScale);
                    int phase = distance.z + distance.x;
                    if (phase < 0) phase = -phase;
                    phase &= 0xffffff80;
                    phase %= 0x400;
                    phase *= 0x40;
                    if (phase < 0x8000) phase -= 0x10000;
                    int push = FixMul(shellWeight, phase);
                    signed char *pLimits =
                        (signed char *)(*(int *)((BYTE *)pRecord + 0x78 + meshIndex * 4)
                                        + vertexIndex * 0x20);
                    FixVector direction;
                    direction.x = (int)pLimits[0x18] << 9;
                    direction.y = (int)pLimits[0x19] << 9;
                    direction.z = (int)pLimits[0x1a] << 9;
                    FixVecScale(&distance, &direction, push);
                }
                position.x += distance.x;
                position.y += distance.y;
                position.z += distance.z;
                StageDeform_ClampVertex(&position.x, meshIndex, vertexIndex, pRecord);
                int secondX = (int)(__int64)(*(float *)((BYTE *)pMesh->pVertexData
                                                         + vertexIndex * 0x30 + 0xc) * CGraphics::m_65536);
                int secondY = (int)(__int64)(*(float *)((BYTE *)pMesh->pVertexData
                                                         + vertexIndex * 0x30 + 0x10) * CGraphics::m_65536);
                int secondZ = (int)(__int64)(*(float *)((BYTE *)pMesh->pVertexData
                                                         + vertexIndex * 0x30 + 0x14) * CGraphics::m_65536);
                FixVector secondDelta;
                secondDelta.x = position.x - original.x;
                secondDelta.y = position.y - original.y;
                secondDelta.z = position.z - original.z;
                FixVecScale(&secondDelta, &secondDelta, 0x30000);
                secondX += secondDelta.x;
                secondY += secondDelta.y;
                secondZ += secondDelta.z;
                *(float *)((BYTE *)pMesh->pVertexData + vertexIndex * 0x30 + 0xc) =
                    (float)((double)secondX * CGraphics::m_oneOver65536);
                *(float *)((BYTE *)pMesh->pVertexData + vertexIndex * 0x30 + 0x10) =
                    (float)((double)secondY * CGraphics::m_oneOver65536);
                *(float *)((BYTE *)pMesh->pVertexData + vertexIndex * 0x30 + 0x14) =
                    (float)((double)secondZ * CGraphics::m_oneOver65536);
                changed = 1;
            }
        }
        if (changed) {
            int mesh = *(int *)(*(int *)((BYTE *)pRecord + 0x3c + meshIndex * 4) + 0xc);
            if (mesh != 0) Mesh_Rebuild((Mesh *)mesh);
        }
    }
}

// Per-level stage light thresholds (16.16), used to pick the brightness level
// of each mesh from its stored value.
// GLOBAL: CMR2 0x00527324
int g_unk0x00527324[8] = { 0x3333, 0x3333, 0x3333, 0x3333,
                           0x1999, 0x1999, 0x1999, 0x1999 };
// GLOBAL: CMR2 0x00527344
int g_unk0x00527344[8] = { 0x1999, 0x1999, 0x1999, 0x1999,
                           0x1999, 0x1999, 0x1999, 0x1999 };

// Picks the brightness level of the record's meshes: each of the eight stored
// values is scaled to 16.16 and compared against the two thresholds.
// FUNCTION: CMR2 0x00508ee0
void FUN_00508ee0(int index)
{
    BYTE *pValue = (BYTE *)((BYTE *)RallyData_FUN_00407610(index) + 0x122);
    BYTE level = 0;
    do {
        int value = FixDiv((int)pValue[level] << 16, 0xff0000);
        if (value > g_unk0x00527324[level])
            FUN_00508fa0(index, 2, level);
        else if (value > g_unk0x00527344[level])
            FUN_00508fa0(index, 1, level);
    } while (++level < 8);
}

// Network record of a car's body: a matrix plus the axes and position it was
// built from. Eight contiguous 0xec-byte rows start at 0x5393d8.
struct CarNetRecord {
    FixMatrix matrix;       // 0x00
    FixVector right;        // 0x40
    FixVector up;           // 0x4c
    FixVector forward;      // 0x58
    FixVector position;     // 0x64
    BYTE pad_0x70[0x7c];
};
// First of the eight car network records (the rest are g_unk0x005394bc).
// GLOBAL: CMR2 0x005393d8
CarNetRecord g_unk0x005393d8;

extern NetStats g_localCarStats;
void NetRace_PackCarState(Car *car);
NetStats *FUN_00409e20(int index);
int FUN_0040b020(int value);
short RallyData_FUN_004213a0(BYTE *p);
int FUN_0046d500(void);
unsigned int RallyData_GetFlag21(void);

// Snapshots a car's body axes and position into its network record and marks
// it for sending.
// FUNCTION: CMR2 0x00424dc0
void FUN_00424dc0(Car *car)
{
    CarNetRecord *rec = (CarNetRecord *)&g_unk0x005393d8 + car->field_0xb1a;
    NetStats *stats;
    int *p = (int *)rec;
    int i;

    for (i = 0x3b; i != 0; i--)
        *p++ = 0;
    rec->position = car->position;
    rec->right = car->right;
    rec->up = car->up;
    rec->forward = car->forward;
    FixMatrix_SetRight(&rec->right, &rec->matrix);
    FixMatrix_SetUp(&rec->up, &rec->matrix);
    FixMatrix_SetForward(&rec->forward, &rec->matrix);
    FixMatrix_SetPosition(&rec->position, &rec->matrix);
    if (car->field_0xb1a > 0) {
        NetRace_PackCarState(car);
        stats = FUN_00409e20(FUN_0040b020((int)car->field_0xb1a));
        *stats = g_localCarStats;
        FUN_00409e20(FUN_0040b020((int)car->field_0xb1a))->seq = 0;
    }
}

// Clears the road-book slots of every car in the running order and
// reinitialises the ones with no pending target.
// match 58%: only the esi/edi allocation differs
// FUNCTION: CMR2 0x004581d0
void FUN_004581d0(void)
{
    int count = Car_GetOrderCount();
    int i;

    for (i = 0; i < count; i++) {
        g_unk0x00542e78[i].field_0x16 = 0;
        g_unk0x00542e78[i].field_0x17 = 0;
        g_unk0x00542e78[i].field_0x18 = 0;
        g_unk0x00542e78[i].field_0x19 = 0;
        if (g_unk0x00542e78[i].field_0x1a == 0)
            FUN_00458e00(i, FUN_00459320(RallyData_FUN_004213a0((BYTE *)Car_Get(i)) & 0xffff));
    }
}

// Paths of the stage sample files loaded by FUN_0045eb50.
// GLOBAL: CMR2 0x0051b138
char g_strSunGlowTga[] = "\\NEWIMAGE\\sunglow.tga";
// GLOBAL: CMR2 0x0051b150
char g_strSnowTga[] = "%s\\NEWIMAGE\\snow%d.tga";
// GLOBAL: CMR2 0x0051b168
char g_strSplat2Tga[] = "\\NEWIMAGE\\splat2.tga";
// GLOBAL: CMR2 0x0051b180
char g_strSplatTga[] = "\\NEWIMAGE\\splat.tga";
// GLOBAL: CMR2 0x0051b194
char g_strRainTga[] = "\\NEWIMAGE\\rain.tga";

extern char g_strPathConcat[];

struct Unk0x004a3e20;
void FUN_004a3e20(Unk0x004a3e20 *pObject, int value);

// GLOBAL: CMR2 0x00543e90
int g_unk0x00543e90;
// GLOBAL: CMR2 0x00543ea4
int g_unk0x00543ea4;
// GLOBAL: CMR2 0x00543ea8
int g_unk0x00543ea8[3];
// GLOBAL: CMR2 0x005477f0
int g_unk0x005477f0;
// GLOBAL: CMR2 0x00547ad0
int g_unk0x00547ad0;

// Loads the weather particle textures (rain, splats, snow, sunglow).
// FUNCTION: CMR2 0x0045eb50
void FUN_0045eb50(void)
{
    bool didLoad;
    int i;

    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::FUN_0040ed50(), g_strRainTga);
    g_unk0x00543e90 = (int)CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile0(),
                                                     CFrontend::m_stringDest, &didLoad, NULL, 0, 0);
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::FUN_0040ed50(), g_strSplatTga);
    g_unk0x00543ea4 = (int)CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile0(),
                                                     CFrontend::m_stringDest, &didLoad, NULL, 0, 0);
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::FUN_0040ed50(), g_strSplat2Tga);
    g_unk0x005477f0 = (int)CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile0(),
                                                     CFrontend::m_stringDest, &didLoad, NULL, 0, 0);
    for (i = 0; i < 3; i++) {
        sprintf(CFrontend::m_stringDest, g_strSnowTga, CInstallInfo::FUN_0040ed50(), i + 1);
        g_unk0x00543ea8[i] = (int)CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile0(),
                                                            CFrontend::m_stringDest, &didLoad, NULL, 0, 0);
    }
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::FUN_0040ed50(), g_strSunGlowTga);
    g_unk0x00547ad0 = (int)CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile0(),
                                                     CFrontend::m_stringDest, &didLoad, NULL, 0, 0);
    FUN_004a3e20((Unk0x004a3e20 *)g_unk0x00547ad0, 1);
}

// File name formats of the three "team" model archives.
// GLOBAL: CMR2 0x0051a134
char g_strTm2Format[] = "%s.tm2";
// GLOBAL: CMR2 0x0051a13c
char g_strTm1Format[] = "%s.tm1";
// GLOBAL: CMR2 0x0051a144
char g_strTm0Format[] = "%s.tm0";

// Finds the three .tm? model archives of the current team and copies the one
// selected by the game mode into the locked stage buffer.
// match 80%: registers differ; the extra sprintf argument is the original's (see CONOCIMIENTO 4.y)
// FUNCTION: CMR2 0x00455300
void FUN_00455300(void)
{
    unsigned int sizes[3];
    void *bufs[3];
    unsigned int size = 0;
    void *dest;
    BYTE index;

    g_unk0x00542604 = 0;
    g_unk0x00541f8c = FUN_0046d500();
    sprintf(CFrontend::m_stringDest, g_strTm0Format, FUN_0041f900());
    bufs[0] = CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(),
                                           CFrontend::m_stringDest, NULL, (DWORD *)&size, 0);
    sizes[0] = size;
    sprintf(CFrontend::m_stringDest, g_strTm1Format, FUN_0041f900());
    bufs[1] = CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(),
                                           CFrontend::m_stringDest, NULL, (DWORD *)&size, 0);
    sizes[1] = size;
    sprintf(CFrontend::m_stringDest, g_strTm2Format, FUN_0041f900());
    bufs[2] = CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(),
                                           CFrontend::m_stringDest, NULL, (DWORD *)&size, 0);
    sizes[2] = size;
    if ((char)RallyData_GetFlag21() != 0) {
        dest = (void *)g_unk0x00541f8c;
        if (dest == NULL)
            dest = bufs[0];
        if (CGameInfo::FUN_00405d80() == 4) {
            index = CGameInfo::FUN_00405dd0();
            if (index != 0)
                index = index - 1;
        } else {
            index = CGameInfo::FUN_00405d90();
        }
        memcpy(dest, bufs[index], sizes[index]);
        g_unk0x00541f8c = (int)dest + sizes[index];
        g_unk0x0054241c = (char *)dest;
        if (dest != NULL)
            g_unk0x00542604 = 1;
    }
}

extern char g_stageSplitCount;
BYTE *RallyData_FUN_00421440(int index);
unsigned int RallyData_FUN_00407e90(void);

// Estimates the remaining stage time of every driver slot from the progress
// made so far, inserting each newly queued slot at the end of the order.
// match 42%: registers and the stack layout of the locals differ; logic checked against the asm
// FUNCTION: CMR2 0x004483e0
void FUN_004483e0(void)
{
    int count = FUN_00458390();
    int slot = (int)(char)g_unk0x0053e18c;
    int prev = 0;
    int total;
    int i;
    int j;
    int k;
    int value;
    char *pSlot;

    total = (BYTE)RallyData_FUN_00406990() * RallyData_FUN_00421420() * 0x10000;
    while (1) {
        if (count <= slot)
            return;
        for (i = 0; i < count; i++) {
            pSlot = &g_unk0x0053e17c[slot];
            for (j = 0; j < slot; j++) {
                if (g_unk0x0053dda8[i] == g_unk0x0053e17c[j])
                    goto cont;
            }
            k = (int)(char)g_unk0x0053dda8[i];
            *pSlot = g_unk0x0053dda8[i];
            g_unk0x0053e184[k] = (char)slot;
            slot = slot + 1;
            pSlot++;
            g_unk0x0053d1da[k] = 1;
            value = FixDiv((FUN_004589e0(k) & 0xffff) << 16, total);
            if (value < 1) {
                g_unk0x0053d1b8[k] = g_unk0x0053d1b0 * 2;
            } else {
                g_unk0x0053d1b8[k] =
                    ConvertRawTimeToCentiseconds(FixMul(FixDiv(0x10000, value),
                                                        FUN_0040d4b0(g_unk0x0053d1b0)));
            }
            if ((char)RallyData_FUN_00407e90() != 0)
                g_unk0x0053e190[11] = g_unk0x0053d1b8[k];
            if (prev != 0 && prev == value)
                g_unk0x0053d1b8[k]++;
            prev = value;
        cont:
            ;
        }
    }
}

// Splits of the stage used for the timing display.
// GLOBAL: CMR2 0x00542c78
int g_unk0x00542c78;

// Builds the checkpoint split table of the current stage.
// match 76%: registers and the shared tail of the two flag tests differ
// FUNCTION: CMR2 0x00458a00
void FUN_00458a00(void)
{
    int count = 0;
    int i;
    int unit;
    int scale;

    g_unk0x00542c74 = 0;
    g_stageCheckpointCount = RallyData_FUN_00421420();
    if (g_stageCheckpointCount == 0)
        return;
    for (i = 0; i < g_stageCheckpointCount; i++) {
        if ((*(BYTE *)(RallyData_FUN_00421440(i) + 0x18) & 1) != 0) {
            g_unk0x00542c7c[count] = (int)(__int64)((double)i * CGraphics::m_65536);
            count++;
        }
    }
    g_unk0x00542c74 = count;
    if ((char)RallyData_GetFlag24() != 0) {
        g_stageLooped = 1;
    } else {
        g_stageLooped = 0;
        if ((char)RallyData_GetFlag25() != 0)
            g_stageLooped = 1;
    }
    if ((char)RallyData_GetFlag24() != 0 || (char)RallyData_GetFlag25() != 0) {
        g_unk0x00542c74 = (char)RallyData_GetFlag24() != 0 ? 4 : 2;
        g_unk0x00542c78 = g_unk0x00542c74;
        unit = (int)(__int64)((double)g_unk0x00542c74 * CGraphics::m_65536);
        if ((char)RallyData_GetFlag24() != 0) {
            scale = FixDiv((int)(__int64)((double)g_stageCheckpointCount * CGraphics::m_65536), unit);
            for (i = 0; i < g_unk0x00542c74; i++)
                g_unk0x00542c7c[i] = FixMul((int)(__int64)((double)i * CGraphics::m_65536), scale)
                                     & 0xffff0000;
        }
    }
    if ((char)RallyData_GetFlag24() != 0 || (char)RallyData_GetFlag25() != 0)
        g_stageSplitCount = (char)g_unk0x00542c74;
    else
        g_stageSplitCount = (char)g_unk0x00542c74 - 1;
}

// GLOBAL: CMR2 0x00542c6c
int g_unk0x00542c6c;

extern int g_unk0x00542c68;
extern char g_unk0x00542cad;
char FUN_00420190(void);
unsigned int RallyData_FUN_004082e0(void);
unsigned int RallyData_FUN_004082b0(void);
unsigned int RallyData_FUN_004082c0(void);

// Rebuilds the stage split table and the checkpoint count for the current
// event.
// FUNCTION: CMR2 0x00458090
void FUN_00458090(void)
{
    FUN_00458a00();
    g_unk0x00542cad = 0;
    g_unk0x00542c68 = (BYTE)FUN_00420190();
    g_unk0x00542c6c = (BYTE)RallyData_FUN_00406990();
    if ((char)RallyData_FUN_004082e0() != 0 && RallyData_FUN_004082b0() == 2)
        g_unk0x00542c6c = RallyData_FUN_004082c0() << 1;
    if (CGameInfo::FUN_00405d80() == 7 || CGameInfo::FUN_00405d80() == 12)
        g_unk0x00542cad = 1;
}

extern int g_sinTable[4096];
void FUN_00421fe0(short *pOut, unsigned int view);
void FUN_00460a30(FixVector *pOut);

// Integrates the terrain slope under a car into its body pitch, wrapping at a
// full turn.
// match 44%: registers and frame layout differ (the original keeps the base pointer in edi)
// FUNCTION: CMR2 0x0045f5d0
void FUN_0045f5d0(int pData, int param_2)
{
    FixVector vec;
    int cosA;
    int negSinA;
    int zero;
    int *p = (int *)pData;
    int value;
    int angle;

    p[8] = p[5];
    FUN_00421fe0((short *)&pData, param_2);
    zero = 0;
    cosA = g_sinTable[(pData + 0x400) & 0xfff];
    negSinA = -g_sinTable[pData & 0xfff];
    FUN_00460a30(&vec);
    value = FixMul(vec.x, negSinA) + FixMul(vec.y, zero) + FixMul(vec.z, cosA);
    angle = p[5] + FixMul(0xf5c, value);
    p[5] = angle;
    if (angle > 0x1680000) {
        p[5] = angle - 0x1680000;
        p[8] += -0x1680000;
    } else if (angle < 0) {
        p[5] = angle + 0x1680000;
        p[8] += 0x1680000;
    }
}

// GLOBAL: CMR2 0x0053ddb0
char g_unk0x0053ddb0[108];

// Records a new split time for a car and reorders the shared split table.
// match 76%: registers differ and the flag tests are re-read (the original calls them per branch)
// FUNCTION: CMR2 0x004487a0
void FUN_004487a0(int car)
{
    int split = FUN_00458370(car);
    int count;
    int group;
    int index;
    char prev;

    if ((char)RallyData_GetFlag24() == 0 && (char)RallyData_GetFlag25() == 0) {
        if ((int)((BYTE)RallyDataState()) <= car)
            return;
        g_unk0x0053e190[split + car * 9] = g_unk0x0053d1b0;
        return;
    }
    count = FUN_00458390();
    group = FUN_00458330(car);
    index = FUN_00458350(car);
    if ((char)RallyData_GetFlag24() == 0)
        g_carStageTiming[car].field_0x4[split] = g_unk0x0053d1b0;
    else
        g_unk0x0053d1e8[car][group][split] = g_unk0x0053d1b0;
    if (car < (int)((BYTE)RallyDataState()) || (char)RallyData_FUN_00407e90() != 0) {
        g_carStageTiming[car].field_0x84 = 0;
        if ((char)RallyData_GetFlag24() == 0)
            g_unk0x0053e190[split + car * 9] = g_unk0x0053d1b0;
        else
            g_unk0x0053e190[split + car * 9] = g_unk0x0053d1b0 - g_unk0x0053d1e8[car][group][0];
    }
    if ((char)RallyData_FUN_00407e90() != 0) {
        group = 0;
        index = split;
    }
    index = index + group * 9;
    prev = g_unk0x0053ddb0[index];
    g_unk0x0053ddb0[index] = prev + 1;
    g_carStageTiming[car].field_0x82 = prev;
    g_unk0x0053de1c[index][(int)prev] = (char)car;
    if ((int)prev > 0) {
        char other = g_unk0x0053de1c[index][(int)prev - 1];

        g_carStageTiming[other].field_0x80 = (char)car;
        g_carStageTiming[other].field_0x83 = 1;
    }
    if ((int)prev == count - 1)
        g_carStageTiming[car].field_0x80 = (char)0xff;
}

// Refreshes the paint-decal samples of one car and applies the resulting time
// spread to the current timing record.
// FUNCTION: CMR2 0x00455ed0
void FUN_00455ed0(int pCar)
{
    int samples[16];

    FUN_00455f00(pCar, samples);
    FUN_00456110(samples);
}

int FUN_0040b1b0(void);

// Resets the timing records of the drivers taking part in the race according
// to the current mode (rally, split-screen or network).
// FUNCTION: CMR2 0x00458100
void FUN_00458100(int param_1)
{
    int i;
    int count;

    if ((char)RallyData_FUN_00407e90() != 0) {
        if (CGameInfo::FUN_00405e00() != 0) {
            if (CGameInfo::FUN_00405d80() == 10)
                count = 1;
            else
                count = FUN_0040b1b0();
            for (i = 0; i < count; i++) {
                switch (g_unk0x00542cb4[i]) {
                case 0:
                    FUN_00458bd0(i, 0, -1, param_1);
                    break;
                case 1:
                    FUN_00458bd0(i, 2, 0, param_1);
                    break;
                }
            }
        } else {
            FUN_00458bd0(g_unk0x00542cb0, 0, -1, param_1);
            FUN_00458bd0(1 - g_unk0x00542cb0, 2, 0, param_1);
        }
        return;
    }
    i = 0;
    if ((BYTE)FUN_00420190() > 0) {
        do {
            FUN_00458bd0(i, 0, 0, param_1);
            i++;
        } while (i < (int)(BYTE)FUN_00420190());
    }
}

#include <stdlib.h>

extern double g_unk0x00511300;
unsigned int RallyData_GetFlag31(void);
unsigned char RallyDataStageIndex(void);
unsigned int RallyData_GetFlag24(void);
extern int g_unk0x00542cd4;
extern int g_unk0x00542d38[8];
extern int g_unk0x00542cd8[8][3];
int StageObject_Atan2Degrees(int y, int x);
int FUN_0040cec0(int index);
int FUN_0040b1a0(int index);
unsigned long FUN_004a1a00(void);

// Places every car on the starting grid of the current event: stores its
// route position and derives its start x/z from the grid origin, the grid
// heading and the car's slot.
// FUNCTION: CMR2 0x004584d0
void FUN_004584d0(char param_1)
{
    int state = 0;
    int i;
    int slot;
    int count;
    int n;
    int t;
    int angle;
    short a;
    int p0[3];
    int p1[3];

    if ((char)RallyData_GetFlag31() != 0) {
        state = 2;
    } else {
        if (CGameInfo::FUN_00405e00() != 0) {
            if ((char)RallyDataStageIndex() == '\n' && CGameInfo::FUN_00405d80() != '\n')
                state = 3;
        }
        if (state == 0) {
            if (g_unk0x00542c68 == 1 || CGameInfo::FUN_00405d80() == '\n')
                return;
            if ((char)RallyData_GetFlag24() == 0 && g_unk0x00542c68 == 2) {
                if ((char)RallyData_FUN_00407e90() == 0)
                    return;
                state = 1;
            }
        }
    }

    switch (state) {
    case 0:
    {
        int sinA;
        int cosA;

        RallyData_FUN_00421530(RallyData_FUN_00421420() - 1, p0);
        RallyData_FUN_00421530(0, p1);
        angle = StageObject_Atan2Degrees(p1[2] - p0[2], p1[0] - p0[0]);
        a = (short)(int)(__int64)((double)angle * g_unk0x00511300);
        sinA = FixMul(0x40000, g_sinTable[a & 0xfff]);
        cosA = FixMul(0x40000, g_sinTable[(a + 0x400) & 0xfff]);
        n = 1;
        for (i = 0; i < g_unk0x00542c68; i++, n--) {
            slot = i;
            if (CGameInfo::FUN_00405d80() == 5 && g_unk0x00542c68 > 2)
                slot = FUN_0040cec0(i);
            if ((char)RallyDataState() == 2)
                slot = n;
            g_unk0x00542d38[slot] = RallyData_FUN_00421420() - 1;
            if (g_unk0x00542c68 > 2)
                t = FixMul((int)(__int64)((double)slot * CGraphics::m_65536), 0x50000) -
                    (int)(__int64)((double)(g_unk0x00542c68 * 5) * CGraphics::m_65536);
            else
                t = 0xfff60000;
            if (slot % 2 == 0) {
                g_unk0x00542cd8[i][0] = FixMul(t, g_sinTable[(a + 0x400) & 0xfff]) - sinA + p1[0];
                g_unk0x00542cd8[i][2] = FixMul(t, g_sinTable[a & 0xfff]) + cosA + p1[2];
            } else {
                g_unk0x00542cd8[i][0] = FixMul(t, g_sinTable[(a + 0x400) & 0xfff]) + sinA + p1[0];
                g_unk0x00542cd8[i][2] = FixMul(t, g_sinTable[a & 0xfff]) - cosA + p1[2];
            }
        }
        break;
    }
    case 1:
        if (param_1 == 0) {
            srand(CMain::GetFrameTime());
            g_unk0x00542cd4 = (rand() <= 0x3fff);
        }
        g_unk0x00542cb0 = g_unk0x00542cd4;
        for (i = 0; i < 2; i++)
            g_unk0x00542d38[i] = ((g_unk0x00542cb0 + i) % 2) ? g_unk0x00542c7c[1] : 0;
        break;
    case 2:
    {
        int sinA;
        int cosA;

        RallyData_FUN_00421530(RallyData_FUN_00421420() - 1, p0);
        RallyData_FUN_00421530(0, p1);
        angle = StageObject_Atan2Degrees(p1[2] - p0[2], p1[0] - p0[0]);
        a = (short)(int)(__int64)((double)angle * g_unk0x00511300);
        sinA = FixMul(0x40000, g_sinTable[a & 0xfff]);
        cosA = FixMul(0x40000, g_sinTable[(a + 0x400) & 0xfff]);
        count = (char)FUN_0040b1b0();
        for (i = 0; i < count; i++) {
            slot = FUN_0040b1a0(i);
            if (slot == (int)FUN_004a1a00())
                slot = 0;
            else
                slot = FUN_0040b010(FUN_0040a7a0(FUN_0040b1a0(i)));
            g_unk0x00542d38[slot] = RallyData_FUN_00421420() - 1;
            if ((char)count > 2)
                t = FixMul((int)(__int64)((double)i * CGraphics::m_65536), 0x50000) -
                    (int)(__int64)((double)(count * 5) * CGraphics::m_65536);
            else
                t = 0xfff60000;
            if (i % 2 == 0) {
                g_unk0x00542cd8[slot][0] = FixMul(t, g_sinTable[(a + 0x400) & 0xfff]) - sinA + p1[0];
                g_unk0x00542cd8[slot][2] = FixMul(t, g_sinTable[a & 0xfff]) + cosA + p1[2];
            } else {
                g_unk0x00542cd8[slot][0] = FixMul(t, g_sinTable[(a + 0x400) & 0xfff]) + sinA + p1[0];
                g_unk0x00542cd8[slot][2] = FixMul(t, g_sinTable[a & 0xfff]) - cosA + p1[2];
            }
        }
        break;
    }
    case 3:
        count = (char)FUN_0040b1b0();
        for (i = 0; i < count; i++) {
            slot = FUN_0040b1a0(i);
            if (slot == (int)FUN_004a1a00())
                slot = 0;
            else
                slot = FUN_0040b010(FUN_0040a7a0(FUN_0040b1a0(i)));
            if (i % 2 != 0) {
                g_unk0x00542d38[slot] = g_unk0x00542c7c[1];
                g_unk0x00542cb4[slot] = 1;
            } else {
                g_unk0x00542d38[slot] = 0;
                g_unk0x00542cb4[slot] = 0;
            }
        }
        break;
    }
}

#include "Particle.h"

// Per-particle-kind texture handles for the wheel spray / smoke effects.
// GLOBAL: CMR2 0x005435c8
int g_unk0x005435c8;
// GLOBAL: CMR2 0x00543650
int g_unk0x00543650;
// GLOBAL: CMR2 0x00543654
int g_unk0x00543654;
// GLOBAL: CMR2 0x00543658
int g_unk0x00543658;
// GLOBAL: CMR2 0x0054365c
int g_unk0x0054365c;
// GLOBAL: CMR2 0x00543660
int g_unk0x00543660;
// GLOBAL: CMR2 0x00543664
int g_unk0x00543664;
// GLOBAL: CMR2 0x00543668
int g_unk0x00543668;
// GLOBAL: CMR2 0x0054366c
int g_unk0x0054366c;
// GLOBAL: CMR2 0x00543670
int g_unk0x00543670;
// GLOBAL: CMR2 0x00543674
int g_unk0x00543674;
// GLOBAL: CMR2 0x00543678
int g_unk0x00543678;
// GLOBAL: CMR2 0x0054367c
int g_unk0x0054367c[5];
// GLOBAL: CMR2 0x00543690
int g_unk0x00543690[2];
// GLOBAL: CMR2 0x00543698
int g_unk0x00543698[2];
// GLOBAL: CMR2 0x005436a0
int g_unk0x005436a0[5];
// GLOBAL: CMR2 0x005436b8
int g_unk0x005436b8[8];
// GLOBAL: CMR2 0x005436d8
int g_unk0x005436d8[4];
// GLOBAL: CMR2 0x00547fb4
int g_unk0x00547fb4;

void ParticleEdit_Select(int index);
void ParticleEdit_CopyTemplate(int index);
void ParticleEdit_SetTextureParams(int a, int b, int c, int d, int e);
void ParticleEdit_SetExtendedParams(int a, int b, int c, int d, BYTE flag4, BYTE flag8, int e, int f,
                                    int g, int h);
void ParticleEdit_SetMotion(int lifetime, int gravity, int drag, int bounce, int friction, char bounces,
                            BYTE killBelowFloor);
void ParticleEdit_SetAlphaRamp(BYTE start, BYTE end, char step, BYTE killAtEnd);
void ParticleEdit_SetColour(BYTE r, BYTE g, BYTE b, BYTE flag);
void ParticleEdit_SetSizeRamp(int size, int target, int step);
void ParticleEdit_SetSizeRange(int min, int max);
void ParticleEdit_SetSpread(int x, int y, int z, BYTE flag);
void ParticleEdit_SetCallbacks(int field0x5c, void (*update)(void *, ParticleType *, int),
                               void (*postUpdate)(void *, ParticleType *, int),
                               void (*callback)(void *, ParticleType *, int), int field0x6c);
void FUN_004afe80(short param1);
void FUN_004aff60(void);
void FUN_004b1140(int count);
int WheelSpray_Init(int spread, int range);
void Spark_Draw(Particle *p, ParticleType *pType, SceneNode *pView);
void FUN_004994d0(void *pParticle, ParticleType *pType, int param);
void GlassShard_Draw(Particle *p, ParticleType *pType, int unused);
void GlassShard_Init(Particle *p, ParticleType *pType, Car *pCar);
void Debris_Init(Particle *p, ParticleType *pType, int *pParam);
void Debris_Draw(Particle *p, ParticleType *pType, SceneNode *pView);
void FUN_0045d250(void *pParticle, ParticleType *pType, int param);
void FUN_0045d270(void *pParticle, ParticleType *pType, int param);
void FUN_0045d2d0(void *pParticle, ParticleType *pType, int param);
void FUN_0045d3a0(void *pParticle, ParticleType *pType, int param);
void FUN_0045de80(void *pParticle, ParticleType *pType, int param);
void FUN_0045dea0(void *pParticle, ParticleType *pType, int param);

// Fills the particle-type table of the stage effects (wheel spray, smoke,
// sparks, glass shards, debris) with their textures, motion and callbacks.
// FUNCTION: CMR2 0x0045a170
void __fastcall FUN_0045a170(int param_1)
{
    int iVar1;
    int uVar2;

iVar1 = CGameInfo::FUN_00405cd0();
  uVar2 = 100;
  if (iVar1 != 1) {
    uVar2 = param_1;
  }
  iVar1 = CGameInfo::FUN_00405cd0();
  if (iVar1 == 0) {
    uVar2 = 400;
  }
  iVar1 = CGameInfo::FUN_00405cd0();
  if (iVar1 == 2) {
    uVar2 = 0;
  }
  FUN_004b1140(uVar2);
  ParticleEdit_Select(0);
  ParticleEdit_SetTextureParams(g_unk0x0054365c,0xffff0000,0x10000,0x10000,0xffff0000);
  ParticleEdit_SetMotion(0xc80000,6,0x10000,0,0,0,0);
  ParticleEdit_SetAlphaRamp(0x1e,0,3,1);
  ParticleEdit_SetSizeRamp(0x28f,0x10000,0x5a1);
  FUN_004afe80(0);
  ParticleEdit_SetSpread(0x28f,0x28f,0x28f,1);
  FUN_004aff60();
  ParticleEdit_Select(1);
  ParticleEdit_CopyTemplate(0);
  ParticleEdit_SetTextureParams(g_unk0x00543660,0xffff0000,0x10000,0x10000,0xffff0000);
  ParticleEdit_SetAlphaRamp(0x50,0,3,1);
  FUN_004aff60();
  ParticleEdit_Select(2);
  ParticleEdit_SetTextureParams(g_unk0x00543664,0xffff0000,0x10000,0x10000,0xffff0000);
  ParticleEdit_SetMotion(0x12c0000,6,0x10000,0,0,0,0);
  ParticleEdit_SetAlphaRamp(100,0,1,1);
  ParticleEdit_SetSizeRamp(0x1999,0x10000,0xb43);
  FUN_004afe80(0);
  ParticleEdit_SetSpread(0x28f,0x28f,0x28f,1);
  FUN_004aff60();
  ParticleEdit_Select(3);
  ParticleEdit_CopyTemplate(0);
  ParticleEdit_SetMotion(0x40000,0,0,0,0,0,0);
  ParticleEdit_SetSizeRamp(0xccc,0x2147,0xccc);
  ParticleEdit_SetExtendedParams((int)&g_unk0x005436d8,4,0,0x10000,0,0,0xffff0000,0x10000,0x10000,0xffff0000);
  ParticleEdit_SetAlphaRamp(0xfe,0xfe,0,0);
  ParticleEdit_SetSizeRange(0x1999,0x3333);
  ParticleEdit_SetCallbacks(0,0,(void (*)(void *, ParticleType *, int))FUN_0045d2d0,(void (*)(void *, ParticleType *, int))FUN_0045de80,0);
  FUN_004aff60();
  ParticleEdit_Select(6);
  ParticleEdit_SetTextureParams(g_unk0x0054366c,0xffff999a,0x6666,0x6666,0xffff999a);
  ParticleEdit_SetMotion(0x210000,0x20,0,0,0,0,0);
  ParticleEdit_SetAlphaRamp(0xd2,0,6,1);
  ParticleEdit_SetColour(0xff,0xff,0xff,1);
  ParticleEdit_SetSizeRamp(0x4ccc,0x18000,0x7ae);
  FUN_004afe80(0);
  ParticleEdit_SetSpread(0x51e,0x28f,0x51e,0);
  ParticleEdit_SetCallbacks(0,0,(void (*)(void *, ParticleType *, int))FUN_0045d270,(void (*)(void *, ParticleType *, int))FUN_0045de80,0);
  FUN_004aff60();
  ParticleEdit_Select(4);
  ParticleEdit_CopyTemplate(6);
  ParticleEdit_SetTextureParams(g_unk0x0054366c,0xffff4ccd,0xb333,0xb333,0xffff4ccd);
  ParticleEdit_SetMotion(0x780000,0x20,0,0,0,0,0);
  ParticleEdit_SetAlphaRamp(100,0,2,1);
  FUN_004aff60();
  ParticleEdit_Select(7);
  ParticleEdit_CopyTemplate(6);
  ParticleEdit_SetTextureParams(g_unk0x0054366c,0xffff4ccd,0xb333,0xb333,0xffff4ccd);
  ParticleEdit_SetSizeRamp(0x4ccc,0x13333,0x147a);
  ParticleEdit_SetMotion(0x640000,0,0xa0000,0,0,0,0);
  ParticleEdit_SetAlphaRamp(0xd2,0,6,1);
  ParticleEdit_SetColour(0xff,0xff,0xff,1);
  ParticleEdit_SetSizeRamp(0x4ccc,0x18000,0x7ae);
  ParticleEdit_SetCallbacks(0,0,(void (*)(void *, ParticleType *, int))FUN_0045d250,0,0);
  FUN_004aff60();
  ParticleEdit_Select(10);
  ParticleEdit_CopyTemplate(6);
  ParticleEdit_SetExtendedParams((int)&g_unk0x005436b8,8,0,0x20000,0,0,0xffffb334,0x4ccc,0x4ccc,0xffffb334);
  ParticleEdit_SetAlphaRamp(100,0,0,0);
  ParticleEdit_SetColour(0xff,0xff,0xff,1);
  ParticleEdit_SetCallbacks(0,0,(void (*)(void *, ParticleType *, int))FUN_0045d270,(void (*)(void *, ParticleType *, int))FUN_0045de80,0);
  FUN_004aff60();
  ParticleEdit_Select(8);
  ParticleEdit_CopyTemplate(4);
  ParticleEdit_SetExtendedParams((int)&g_unk0x005436b8,8,0,0x20000,1,1,0xffff4ccd,0xb333,0xb333,0xffff4ccd);
  ParticleEdit_SetColour(0x98,0x7e,0x66,1);
  FUN_004aff60();
  ParticleEdit_Select(0xb);
  ParticleEdit_CopyTemplate(7);
  ParticleEdit_SetExtendedParams((int)&g_unk0x005436b8,8,0,0x20000,1,1,0xffff4ccd,0xb333,0xb333,0xffff4ccd);
  ParticleEdit_SetSizeRamp(0x4ccc,0x13333,0x147a);
  ParticleEdit_SetMotion(0x640000,0,0xa0000,0,0,0,0);
  ParticleEdit_SetAlphaRamp(100,0,0,0);
  ParticleEdit_SetColour(0xff,0xff,0xff,1);
  ParticleEdit_SetCallbacks(0,0,(void (*)(void *, ParticleType *, int))FUN_0045d250,0,0);
  FUN_004aff60();
  ParticleEdit_Select(5);
  ParticleEdit_CopyTemplate(4);
  ParticleEdit_SetTextureParams(g_unk0x0054366c,0xffff999a,0x6666,0x6666,0xffff999a);
  ParticleEdit_SetMotion(0x300000,0x20,0x30000,0,0,0,0);
  ParticleEdit_SetAlphaRamp(100,0,2,1);
  ParticleEdit_SetSizeRamp(0x4ccc,0x10000,0x1999);
  FUN_004afe80(0x88);
  ParticleEdit_SetSpread(0,0,0,0);
  FUN_004aff60();
  ParticleEdit_Select(0xc);
  ParticleEdit_CopyTemplate(5);
  ParticleEdit_SetExtendedParams((int)&g_unk0x005436b8,8,0,0x20000,1,1,0xffffc000,0x4000,0x8000,0xffffc000);
  ParticleEdit_SetAlphaRamp(0x1e,0,4,1);
  ParticleEdit_SetSizeRamp(0x4ccc,0x5555,0x3332);
  FUN_004afe80(0);
  FUN_004aff60();
  ParticleEdit_Select(0xd);
  ParticleEdit_CopyTemplate(5);
  ParticleEdit_SetExtendedParams((int)&g_unk0x005436b8,8,0,0x20000,1,1,0xffff0000,0x10000,0x20000,0xffff0000);
  ParticleEdit_SetAlphaRamp(0x46,0,2,1);
  FUN_004aff60();
  ParticleEdit_Select(0xe);
  ParticleEdit_CopyTemplate(5);
  ParticleEdit_SetMotion(0x1900000,0,0,0,0,0,0);
  FUN_004afe80(0);
  ParticleEdit_SetTextureParams(g_unk0x00543674,0,0x20000,0x20000,0);
  ParticleEdit_SetSizeRamp(0x1999,0x10000,0xa3d);
  ParticleEdit_SetAlphaRamp(0xff,0,0xf,1);
  ParticleEdit_SetSpread(0,0,0,0);
  ParticleEdit_SetCallbacks(0,0,(void (*)(void *, ParticleType *, int))FUN_0045dea0,(void (*)(void *, ParticleType *, int))FUN_0045de80,0);
  FUN_004aff60();
  ParticleEdit_Select(0xf);
  ParticleEdit_CopyTemplate(0xe);
  ParticleEdit_SetTextureParams(g_unk0x00543670,0,0,0x20000,0xfffe0000);
  FUN_004aff60();
  ParticleEdit_Select(0x10);
  ParticleEdit_SetExtendedParams((int)&g_unk0x0054367c,5,0x20000,0x10000,1,1,0xffff3334,0xcccc,0xcccc,0xffff3334);
  ParticleEdit_SetMotion(0xc80000,0x28f,0,0,0,0,1);
  ParticleEdit_SetAlphaRamp(0xff,0xff,0,1);
  ParticleEdit_SetColour(0xff,0xff,0xff,1);
  ParticleEdit_SetSizeRange(0x1999,0x147a);
  FUN_004afe80(0xb);
  ParticleEdit_SetSpread(0,0,0,0);
  ParticleEdit_SetCallbacks(0,0,(void (*)(void *, ParticleType *, int))FUN_0045d3a0,(void (*)(void *, ParticleType *, int))FUN_0045de80,0);
  FUN_004aff60();
  ParticleEdit_Select(0x11);
  ParticleEdit_CopyTemplate(0x10);
  ParticleEdit_SetAlphaRamp(0xff,0xff,0,0);
  FUN_004aff60();
  ParticleEdit_Select(0x12);
  ParticleEdit_CopyTemplate(0x10);
  ParticleEdit_SetExtendedParams((int)&g_unk0x00543690,2,0x40000,0x40000,1,1,0xfffe8000,0x18000,0x18000,0xfffe8000);
  ParticleEdit_SetMotion(0xc80000,0x28f,0,0,0,0,1);
  ParticleEdit_SetAlphaRamp(0xff,0xff,0,1);
  ParticleEdit_SetColour(0xff,0xff,0xff,1);
  ParticleEdit_SetSizeRange(0x1999,0x147a);
  FUN_004afe80(0xb);
  ParticleEdit_SetSpread(0,0,0,0);
  ParticleEdit_SetCallbacks(0,0,(void (*)(void *, ParticleType *, int))FUN_0045d3a0,(void (*)(void *, ParticleType *, int))FUN_0045de80,0);
  FUN_004aff60();
  ParticleEdit_Select(0x13);
  ParticleEdit_CopyTemplate(0x12);
  ParticleEdit_SetAlphaRamp(0xff,0xff,0,0);
  FUN_004aff60();
  ParticleEdit_Select(0x14);
  ParticleEdit_CopyTemplate(0x12);
  ParticleEdit_SetExtendedParams((int)&g_unk0x00543698,2,0x40000,0x40000,1,1,0xfffe8000,0x18000,0x18000,0xfffe8000);
  FUN_004aff60();
  ParticleEdit_Select(0x15);
  ParticleEdit_CopyTemplate(0x14);
  ParticleEdit_SetAlphaRamp(0xff,0xff,0,0);
  FUN_004aff60();
  ParticleEdit_Select(0x17);
  ParticleEdit_CopyTemplate(0x13);
  ParticleEdit_SetAlphaRamp(0xff,0xff,0,0);
  ParticleEdit_SetSizeRange(0x147a,0xa3d);
  FUN_004aff60();
  ParticleEdit_Select(0x16);
  ParticleEdit_CopyTemplate(0x12);
  ParticleEdit_SetAlphaRamp(0xff,0xff,0,1);
  ParticleEdit_SetSizeRange(0x147a,0xa3d);
  FUN_004aff60();
  ParticleEdit_Select(0x18);
  ParticleEdit_SetTextureParams(g_unk0x005435c8,0xffffc000,0x4000,0x4000,0xffffc000);
  ParticleEdit_SetMotion(0xc80000,0xccc,0,0x1999,0,1,1);
  ParticleEdit_SetAlphaRamp(0xff,0xff,0,1);
  ParticleEdit_SetSizeRamp(0x1999,0x1999,0);
  FUN_004afe80(0);
  ParticleEdit_SetSpread(0,0,0,0);
  ParticleEdit_SetCallbacks(0,0,(void (*)(void *, ParticleType *, int))FUN_0045dea0,(void (*)(void *, ParticleType *, int))FUN_0045de80,0);
  FUN_004aff60();
  ParticleEdit_Select(0x1a);
  ParticleEdit_SetTextureParams(g_unk0x00543650,0xffffcccd,0x3333,0x3333,0xffffcccd);
  ParticleEdit_SetMotion(0x140000,0x147,0,0,0,0,1);
  ParticleEdit_SetAlphaRamp(0xfe,0,0x22,1);
  ParticleEdit_SetSizeRamp(0x10000,0x10000,0);
  FUN_004afe80(0);
  ParticleEdit_SetSpread(0,0,0,0);
  ParticleEdit_SetCallbacks((int)Spark_Draw,0,(void (*)(void *, ParticleType *, int))FUN_004994d0,0,0);
  FUN_004aff60();
  ParticleEdit_Select(0x1b);
  ParticleEdit_SetTextureParams(g_unk0x00543654,0xffffcccd,0x3333,0x3333,0xffffcccd);
  ParticleEdit_SetMotion(0x140000,0x147,0,0,0,0,1);
  ParticleEdit_SetAlphaRamp(0xff,0xff,0,0);
  ParticleEdit_SetSizeRamp(0x10000,0x10000,0);
  FUN_004afe80(0);
  ParticleEdit_SetSpread(0,0,0,0);
  ParticleEdit_SetCallbacks((int)GlassShard_Draw,0,0,(void (*)(void *, ParticleType *, int))GlassShard_Init,0);
  FUN_004aff60();
  ParticleEdit_Select(0x1c);
  ParticleEdit_SetTextureParams(g_unk0x00543658,0xffffcccd,0x3333,0x3333,0xffffcccd);
  ParticleEdit_SetMotion(0x640000,0x189,0,0,0,0,1);
  ParticleEdit_SetAlphaRamp(0xff,0xff,0,0);
  ParticleEdit_SetSizeRamp(0x10000,0x10000,0);
  FUN_004afe80(0);
  ParticleEdit_SetSpread(0,0,0,0);
  ParticleEdit_SetCallbacks((int)Debris_Draw,0,0,(void (*)(void *, ParticleType *, int))Debris_Init,0);
  FUN_004aff60();
  iVar1 = WheelSpray_Init(0x10000,0x190000);
  ParticleEdit_Select(0x1d);
  ParticleEdit_SetExtendedParams((int)&g_unk0x005436a0,5,0x20000,0x10000,1,1,0xfffff99a,0x666,0x666,0xfffff99a);
  ParticleEdit_SetMotion(0xf0000,FixMul(iVar1, 0x3333),0,0xb333,0,1,0);
  ParticleEdit_SetAlphaRamp(0xff,0xff,0,0);
  ParticleEdit_SetColour(0xff,0,0xff,1);
  ParticleEdit_SetSizeRange(0x10000,0x20000);
  FUN_004afe80(0);
  ParticleEdit_SetSpread(0,0,0,0);
  FUN_004aff60();
  ParticleEdit_Select(0x1e);
  ParticleEdit_SetExtendedParams((int)&g_unk0x005436a0,5,0x20000,0x10000,1,1,0xfffff70b,0x8f5,0x8f5,0xfffff70b);
  ParticleEdit_SetMotion(0x190000,iVar1,0,0xb333,0,1,0);
  ParticleEdit_SetAlphaRamp(0xff,0xff,0,0);
  ParticleEdit_SetColour(0xff,0,0xff,1);
  ParticleEdit_SetSizeRange(0x10000,0x20000);
  FUN_004afe80(0);
  ParticleEdit_SetSpread(0,0,0,0);
  FUN_004aff60();
  ParticleEdit_Select(0x1f);
  ParticleEdit_CopyTemplate(0x1a);
  ParticleEdit_SetTextureParams(g_unk0x00547fb4,0xffffe667,0x1999,0x1999,0xffffe667);
  ParticleEdit_SetCallbacks(0,0,0,0,0);
  FUN_004aff60();
  return;
}

void FUN_0043c7f0(int param_1, int param_2, int param_3, int *param_4, int param_5, int param_6);
void FUN_0043e680(int param_1);
void FUN_00483570(void);

// One wheel/hub record (stride 0x1a0) of the four slots FUN_00480e50 installs:
// the hub transform, the offsets accumulated during the physics step and the
// collision state used while resolving ground contact.
struct PartState {
    int field_0x0;              // 0x000
    FixMatrix *field_0x4;       // 0x004 world matrix
    BYTE field_0x8[0x108];      // 0x008
    int field_0x110;            // 0x110 per-slot integrator address
    FixVector field_0x114;      // 0x114 swing axis
    FixVector field_0x120;      // 0x120 body offset
    FixVector field_0x12c;      // 0x12c previous body offset
    FixVector field_0x138;      // 0x138 motion axes source
    FixVector field_0x144;      // 0x144 accumulated translation
    BYTE field_0x150;           // 0x150 flags (low nibble) | slot (high nibble)
    BYTE field_0x151[3];
    int field_0x154;            // 0x154
    signed short field_0x158;   // 0x158 swing angle
    signed short field_0x15a;
    int field_0x15c;            // 0x15c
    int field_0x160;            // 0x160
    FixVector field_0x164;      // 0x164 accumulated swing offset
    FixVector field_0x170;      // 0x170 ground response gains
    FixBasis field_0x17c;       // 0x17c right / up / forward
};

#define PARTSTATE ((PartState *)g_unk0x00590c20)
#define CARBYTES  ((BYTE *)g_unk0x00590d74)

int FUN_00482f30(void);
void Vehicle_UpdateMotion(FixVector *pInput);
void FUN_00483100(int *param_1, unsigned int param_2);
extern double g_unk0x00511308;

// Per-slot integrator for slot 1 (FUN_00480e50 installs its address at
// +0x110): swings the hub about its axis, resolves the ground contact of the
// part against the three contact planes, re-orthogonalises the part's basis
// and finally integrates the motion offset into the world position.
// match 44%: implementada y completa; el residuo es el reparto de registros y
// de slots de pila de MSVC6 (reccmp no normaliza los desplazamientos de [ebp]),
// no la estructura: el flujo, las condiciones y las constantes 16.16 siguen el
// original instruccion a instruccion.
// FUNCTION: CMR2 0x004816f0
void FUN_004816f0(int part)
{
    BYTE idx;
    unsigned short angles[3];
    FixVector v24;      // scratch vector
    FixVector v30;      // contact offset
    FixVector v3c;      // part clearance point
    FixVector v4c;      // ground offset
    FixVector v58;      // swing offset accumulator
    FixVector v64;      // scaled swing offset
    FixVector v70;      // normalised swing direction, scaled
    FixVector v88;      // collision displacement handed to Vehicle_UpdateMotion
    FixVector vTmp;     // scratch vector (up - v24)
    int len;
    int i;
    int j;
    int k;
    int scale;
    int angle;
    bool flip;
    bool bVar5;

    (void)part;

    if (FUN_00482f30() != 0)
        return;

    if (*(char *)(CARBYTES + 0xb1b) == 9) {
        angle = (int)(__int64)((double)FixMul((short)g_unk0x00590c68 * 0x1680, 0x10000)
                               * g_unk0x00511308);
        goto LAB_00482a03;
    }
    if (*(char *)(CARBYTES + 0xb1b) == 11) {
        angle = (int)(__int64)((double)FixMul((short)g_unk0x00590c68 * 0x1680, 0x20000)
                               * g_unk0x00511308);
        goto LAB_00482a03;
    }

    if ((PARTSTATE->field_0x150 & 6) == 0)
        FUN_00483050();

    v58.x = 0;
    v58.y = 0;
    v58.z = 0;
    v30.x = 0;
    v30.y = 0;
    v30.z = 0;
    bVar5 = false;

    if ((PARTSTATE->field_0x150 & 2) != 0) {
        v70.x = 0;
        v70.y = 0;
        v70.z = 0;
        if (PARTSTATE->field_0x120.z < 1) {
            v24.x = PARTSTATE->field_0x17c.forward.x - FixSin(PARTSTATE->field_0x158);
            v24.y = PARTSTATE->field_0x17c.forward.y;
            v24.z = PARTSTATE->field_0x17c.forward.z - FixSin(PARTSTATE->field_0x158 + 0x400);
        } else {
            v24.x = FixSin(PARTSTATE->field_0x158) - PARTSTATE->field_0x17c.forward.x;
            v24.y = -PARTSTATE->field_0x17c.forward.y;
            v24.z = FixSin(PARTSTATE->field_0x158 + 0x400) - PARTSTATE->field_0x17c.forward.z;
        }
        len = FixVecLength(&v24);
        if (len > 0) {
            i = (int)(0x100000000i64 / (__int64)len);
            v24.x = FixMul(v24.x, i);
            v24.y = FixMul(v24.y, i);
            v24.z = FixMul(v24.z, i);
            i = -FixMul(len, 0x40000);
            v70.x = FixMul(v24.x, i);
            v70.y = FixMul(v24.y, i);
            v70.z = FixMul(v24.z, i);
        }
        v4c.x = *(int *)(CARBYTES + 0x408) - *(int *)(CARBYTES + 0x414);
        v4c.y = *(int *)(CARBYTES + 0x40c) - *(int *)(CARBYTES + 0x418);
        v4c.z = *(int *)(CARBYTES + 0x410) - *(int *)(CARBYTES + 0x41c);
        v3c.x = 0;
        v3c.y = 0;
        v3c.z = 0;
        idx = 0;
        if (PARTSTATE->field_0x120.z > 0)
            idx = 1;
        if ((PARTSTATE->field_0x150 & 0xf0) == 0x10)
            idx += 2;
        if (*(int *)(CARBYTES + 0xbac + idx * 4) != 0) {
            i = FixMul(*(int *)(CARBYTES + 0x48c), *(int *)(CARBYTES + 0x408))
              + FixMul(*(int *)(CARBYTES + 0x490), *(int *)(CARBYTES + 0x40c))
              + FixMul(*(int *)(CARBYTES + 0x494), *(int *)(CARBYTES + 0x410));
            v3c.x = *(int *)(CARBYTES + 0x408) - FixMul(*(int *)(CARBYTES + 0x48c), i);
            v3c.y = *(int *)(CARBYTES + 0x40c) - FixMul(*(int *)(CARBYTES + 0x490), i);
            v3c.z = *(int *)(CARBYTES + 0x410) - FixMul(*(int *)(CARBYTES + 0x494), i);
            FUN_00483100((int *)&v3c, idx);
        }
        v24.x = v3c.x + v4c.x;
        v24.y = v3c.y + v4c.y;
        v24.z = v3c.z + v4c.z;
        len = FixVecLength(&v24);
        if (len < 0x4ccd) {
        LAB_00481da0:
            FixVecScale(&v4c, &v4c, 0xfff60000);
            v30.x += v4c.x;
            v30.y += v4c.y;
            v30.z += v4c.z;
            FixVecScale(&v3c, &v3c, 0xfffc0000);
            v30.x += v3c.x;
            v30.y += v3c.y;
            v30.z += v3c.z;
            v30.y -= 0x4000;
        } else {
            i = FixMul(*(int *)(CARBYTES + 0x360), v24.x)
              + FixMul(*(int *)(CARBYTES + 0x364), v24.y)
              + FixMul(*(int *)(CARBYTES + 0x368), v24.z);
            if ((PARTSTATE->field_0x150 & 0xf0) == 0x10) {
                if (PARTSTATE->field_0x120.z < 0) {
                    scale = 0xffff0000;
                    flip = true;
                } else {
                    scale = 0x10000;
                    flip = false;
                }
            } else {
                if (PARTSTATE->field_0x120.z >= 0) {
                    scale = 0x10000;
                    flip = true;
                } else {
                    scale = 0xffff0000;
                    flip = false;
                }
            }
            if (i < 0) {
                scale = FixMul(scale, -0x10000);
                i = -i;
            }
            if (i > 0x4ccc) {
                j = FixMul(i - 0x4ccc, 0x3555e);
                if (j > 0x10000)
                    j = 0x10000;
                i = FixMul(FixMul(j, 0x50000), scale);
                PARTSTATE->field_0x158 = (short)(int)(__int64)((double)(PARTSTATE->field_0x158 * 0x1680 + i)
                                                              * g_unk0x00511300);
                if (flip) {
                    if (PARTSTATE->field_0x158 > 0)
                        PARTSTATE->field_0x158 = 0;
                } else {
                    if (PARTSTATE->field_0x158 < 0)
                        PARTSTATE->field_0x158 = 0;
                }
                i = PARTSTATE->field_0x158;
                if (i < 0)
                    i = -i;
                if (i > 0xe3)
                    bVar5 = true;
            }
            i = (int)(0x100000000i64 / (__int64)len);
            v24.x = FixMul(v24.x, i);
            v24.y = FixMul(v24.y, i);
            v24.z = FixMul(v24.z, i);
            if (!bVar5)
                goto LAB_00481da0;
            FixVecScale(&v4c, &v4c, 0xfff60000);
            v30.x += v4c.x;
            v30.y += v4c.y;
            v30.z += v4c.z;
            FixVecScale(&v3c, &v3c, 0xffec0000);
            v30.x += v3c.x;
            v30.y += v3c.y;
            v30.z += v3c.z;
            v88.x = v30.x;
            v88.y = v30.y;
            v88.z = v30.z;
        }

        scale = 0x10000 - g_physicsTimeStep;
        if (scale < 0)
            scale = 0;
        else if (scale > 0x8000)
            scale = 0x8000;
        scale = FixMul(scale, 0xc937) + 0x8000;
        PARTSTATE->field_0x164.x = FixMul(PARTSTATE->field_0x164.x, scale);
        PARTSTATE->field_0x164.y = FixMul(PARTSTATE->field_0x164.y, scale);
        PARTSTATE->field_0x164.z = FixMul(PARTSTATE->field_0x164.z, scale);
        FixMatrix_InverseRotateVector(&v24, &v30, PARTSTATE->field_0x4);
        v30.x = v24.x;
        v30.y = v24.y;
        v30.z = v24.z;
        if (bVar5) {
            PARTSTATE->field_0x164.x = 0;
            PARTSTATE->field_0x164.y = 0;
            PARTSTATE->field_0x164.z = 0;
        } else {
            v30.x = v24.x + v70.x;
            v30.y = v24.y + v70.y;
            v30.z = v24.z + v70.z;
        }
        v24.x = PARTSTATE->field_0x12c.x - PARTSTATE->field_0x120.x;
        v24.y = PARTSTATE->field_0x12c.y - PARTSTATE->field_0x120.y;
        v24.z = PARTSTATE->field_0x12c.z - PARTSTATE->field_0x120.z;
        i = FixMul(v30.x, v24.y) - FixMul(v30.y, v24.x);
        if (!bVar5)
            i = 0;
        v58.x = -FixMul(FixMul(v30.y, v24.z) - FixMul(v30.z, v24.y), PARTSTATE->field_0x170.x);
        v58.y = -FixMul(FixMul(v30.z, v24.x) - FixMul(v30.x, v24.z), PARTSTATE->field_0x170.y);
        v58.z = -FixMul(i, PARTSTATE->field_0x170.z);
        PARTSTATE->field_0x164.x += v58.x;
        PARTSTATE->field_0x164.y += v58.y;
        PARTSTATE->field_0x164.z += v58.z;
    }

    if ((PARTSTATE->field_0x150 & 6) != 0) {
        PARTSTATE->field_0x150 |= 8;
        if (!bVar5 && (PARTSTATE->field_0x150 & 4) == 0 && *(int *)(CARBYTES + 0x778) < 0x7af)
            PARTSTATE->field_0x150 &= 0xf7;
        if ((PARTSTATE->field_0x150 & 2) == 0 || (PARTSTATE->field_0x150 & 8) != 0) {
            FixVecScale(&v64, &PARTSTATE->field_0x164, g_physicsTimeStep);
            FixVecScale(&v58, &v58, g_physicsTimeStep / 2);
            v64.x -= v58.x;
            v64.y -= v58.y;
            v64.z -= v58.z;
            angles[0] = (short)(__int64)((double)v64.x * g_unk0x00511380);
            angles[1] = (short)(__int64)((double)v64.y * g_unk0x00511380);
            angles[2] = (short)(__int64)((double)v64.z * g_unk0x00511380);
            FixBasis_Rotate(&PARTSTATE->field_0x17c, angles);
        } else {
            PARTSTATE->field_0x164.x = 0;
            PARTSTATE->field_0x164.y = 0;
            PARTSTATE->field_0x164.z = 0;
        }
    }

    if ((PARTSTATE->field_0x150 & 2) != 0 && (PARTSTATE->field_0x150 & 8) != 0 && !bVar5) {
        flip = false;
        if ((PARTSTATE->field_0x150 & 0xf0) == 0x10) {
            i = PARTSTATE->field_0x17c.forward.x;
            if (PARTSTATE->field_0x120.z < 1) {
                if (i > 0) {
                    PARTSTATE->field_0x17c.forward.x = 0;
                    PARTSTATE->field_0x164.x = 0;
                    flip = true;
                }
                if (PARTSTATE->field_0x17c.forward.y < 1)
                    goto LAB_00482478;
                PARTSTATE->field_0x17c.forward.y = 0;
                PARTSTATE->field_0x164.y = 0;
                goto LAB_00482480;
            } else {
                if (i < 0) {
                    PARTSTATE->field_0x17c.forward.x = 0;
                    PARTSTATE->field_0x164.x = 0;
                    flip = true;
                }
                if (PARTSTATE->field_0x17c.forward.y >= 0)
                    goto LAB_00482478;
            LAB_00482460:
                PARTSTATE->field_0x17c.forward.y = 0;
                PARTSTATE->field_0x164.y = 0;
                goto LAB_00482480;
            }
        } else {
            i = PARTSTATE->field_0x17c.forward.x;
            if (PARTSTATE->field_0x120.z < 1) {
                if (i < 0) {
                    PARTSTATE->field_0x17c.forward.x = 0;
                    PARTSTATE->field_0x164.x = 0;
                    flip = true;
                }
                if (PARTSTATE->field_0x17c.forward.y > 0)
                    goto LAB_00482460;
            } else {
                if (i > 0) {
                    PARTSTATE->field_0x17c.forward.x = 0;
                    PARTSTATE->field_0x164.x = 0;
                    flip = true;
                }
                if (PARTSTATE->field_0x17c.forward.y < 0) {
                    PARTSTATE->field_0x17c.forward.y = 0;
                    PARTSTATE->field_0x164.y = 0;
                    goto LAB_00482480;
                }
            }
        }
    LAB_00482478:
        if (!flip)
            goto LAB_00482584;
    LAB_00482480:
        len = FixVecLength(&PARTSTATE->field_0x17c.forward);
        if (len == 0) {
            PARTSTATE->field_0x17c.forward.x = 0;
            PARTSTATE->field_0x17c.forward.y = 0;
            PARTSTATE->field_0x17c.forward.z = 0;
        } else {
            FixVecScaleRecip(&PARTSTATE->field_0x17c.forward,
                             &PARTSTATE->field_0x17c.forward, len);
        }
        PARTSTATE->field_0x17c.up.x = 0;
    LAB_00482584:

        vTmp.x = PARTSTATE->field_0x17c.up.x - v24.x;
        vTmp.y = PARTSTATE->field_0x17c.up.y - v24.y;
        vTmp.z = PARTSTATE->field_0x17c.up.z - v24.z;
        len = FixVecLength(&vTmp);
        if (len == 0) {
            PARTSTATE->field_0x17c.up.x = 0;
            PARTSTATE->field_0x17c.up.y = 0;
            PARTSTATE->field_0x17c.up.z = 0;
        } else {
            FixVecScaleRecip(&PARTSTATE->field_0x17c.up, &vTmp, len);
        }
        v24.x = FixMul(PARTSTATE->field_0x17c.up.y, PARTSTATE->field_0x17c.forward.z)
              - FixMul(PARTSTATE->field_0x17c.up.z, PARTSTATE->field_0x17c.forward.y);
        v24.y = FixMul(PARTSTATE->field_0x17c.up.z, PARTSTATE->field_0x17c.forward.x)
              - FixMul(PARTSTATE->field_0x17c.up.x, PARTSTATE->field_0x17c.forward.z);
        v24.z = FixMul(PARTSTATE->field_0x17c.up.x, PARTSTATE->field_0x17c.forward.y)
              - FixMul(PARTSTATE->field_0x17c.up.y, PARTSTATE->field_0x17c.forward.x);
        len = FixVecLength(&v24);
        if (len == 0) {
            PARTSTATE->field_0x17c.right.x = 0;
            PARTSTATE->field_0x17c.right.y = 0;
            PARTSTATE->field_0x17c.right.z = 0;
        } else {
            FixVecScaleRecip(&PARTSTATE->field_0x17c.right, &v24, len);
        }
    }

    if ((PARTSTATE->field_0x150 & 4) == 0) {
        angles[0] = 0;
        angles[1] = 0;
        angles[2] = 0;
        if ((PARTSTATE->field_0x150 & 2) == 0) {
            i = FixMul((short)g_unk0x00590c68 * 0x1680, 0x10000);
            k = (short)(__int64)((double)i * g_unk0x00511300);
            if ((PARTSTATE->field_0x150 & 0xf0) == 0x10) {
                angles[2] = (unsigned short)k;
                FUN_00481560(angles);
            } else {
                angles[2] = (unsigned short)-k;
                FUN_00481560(angles);
            }
        } else {
            i = FixMul((short)g_unk0x00590c68 * 0x1680, 0x8000);
            k = (short)(__int64)((double)i * g_unk0x00511300);
            angles[1] = (unsigned short)k;
            if ((PARTSTATE->field_0x150 & 0xf0) == 0x10) {
                if (PARTSTATE->field_0x120.z < 1) {
                    angles[0] = (unsigned short)k;
                    angles[1] = (unsigned short)-k;
                } else {
                    angles[0] = (unsigned short)-k;
                }
            } else {
                angles[0] = (unsigned short)k;
                if (PARTSTATE->field_0x120.z > 0) {
                    angles[0] = (unsigned short)-k;
                    angles[1] = angles[0];
                }
            }
            FUN_00481560(angles);
        }
    } else {
        FixMatrix_SetRight(&PARTSTATE->field_0x17c.right, PARTSTATE->field_0x4);
        FixMatrix_SetUp(&PARTSTATE->field_0x17c.up, PARTSTATE->field_0x4);
        FixMatrix_SetForward(&PARTSTATE->field_0x17c.forward, PARTSTATE->field_0x4);
    }
    goto LAB_00482a98;

LAB_00482a03:
    angles[0] = 0;
    bVar5 = false;
    angles[2] = 0;
    angles[1] = (unsigned short)angle;
    FUN_00481560(angles);

LAB_00482a98:
    VehicleMotion_UpdateWorldPosition();
    if (bVar5)
        Vehicle_UpdateMotion(&v88);
}

// Defined in StageObjects.cpp; FUN_00480e50 installs this callback.
void FUN_00484310(void);

// Places the two view nodes of a car at the shared angle/position buffers and
// rebuilds that car's body state, network-snapshotting it when required.
// FUNCTION: CMR2 0x00457e50
void FUN_00457e50(SceneNode *pNodeA, SceneNode *pNodeB, int carIndex, int param_4,
                  FixAngles *pAngles, FixVector *pPosition, int param_7)
{
    SceneNode_SetRotation(pNodeA, pAngles);
    SceneNode_SetRotation(pNodeB, pAngles);
    SceneNode_SetPosition(pNodeA, pPosition);
    SceneNode_SetPosition(pNodeB, pPosition);
    FUN_0043c7f0((int)Car_Get(carIndex), (int)pNodeA, param_4, (int *)pPosition, carIndex, param_7);
    FUN_0043e680((int)Car_Get(carIndex));
    if (CGameInfo::FUN_00405e00() != 0)
        FUN_00424dc0(Car_Get(carIndex));
}

// Initialises one of a car's four wheel/hub records: points the shared record
// pointer at the car's slot, picks the static geometry for the wheel type,
// installs the per-slot integrator and clears the dynamic state.
// match 13%: implementada; MSVC6 se queda con g_unk0x00590c20 y el puntero base en registro
// en vez de releerlos del global en cada acceso como hace el original
// FUNCTION: CMR2 0x00480e50
void FUN_00480e50(int slot)
{
    int carIndex = *(char *)((BYTE *)g_unk0x00590d74 + 0xb1a);
    unsigned int type = g_unk0x00590c24[slot][carIndex];
    int local_14 = 0;
    int rotor = 0;
    int lateral = 0;
    BYTE *pc;
    int base;

    g_unk0x00590c20 = (Unk0x00590c20 *)((BYTE *)g_unk0x00590d7c[slot] + carIndex * 0x1a0);
    pc = (BYTE *)g_unk0x00590c20;
    if (FUN_004813b0(slot) == 0)
        return;

    pc[0x150] = (BYTE)((pc[0x150] & 0xf) | (slot << 4));
    pc[0x150] |= 1;
    pc[0x150] &= (BYTE)~2;
    pc[0x150] &= (BYTE)~4;
    pc[0x150] &= (BYTE)~8;
    *(int *)(*(int *)pc + 0x184) = 0;

    base = (int)(g_unk0x00590d78 + type * 0xc);

    switch (slot) {
    case 0:
        local_14 = -*(int *)(base + 0x168);
        rotor = *(int *)(base + 0x16c);
        *(short *)(pc + 0x158) = 0x288;
        *(int *)(pc + 0x110) = (int)FUN_004814d0;
        break;
    case 1:
        if (*(char *)((BYTE *)g_unk0x00590d74 + 0xb1b) == 9 ||
            *(char *)((BYTE *)g_unk0x00590d74 + 0xb1b) == 11) {
            rotor = 0;
            local_14 = *(int *)(base + 0x168);
            lateral = -*(int *)(base + 0x170);
        } else {
            local_14 = FixMul(*(int *)(base + 0x168), 0x8000);
            rotor = *(int *)(base + 0x16c);
        }
        *(short *)(pc + 0x158) = 0;
        *(int *)(pc + 0x110) = (int)FUN_004816f0;
        *(int *)(pc + 0x170) = 0x10e5;
        *(int *)(pc + 0x174) = 0x1eb8;
        *(int *)(pc + 0x178) = 0x10e5;
        goto common;
    case 2:
        if (*(char *)((BYTE *)g_unk0x00590d74 + 0xb1b) == 11) {
            local_14 = 0;
            rotor = *(int *)(base + 0x16c);
            lateral = -*(int *)(base + 0x170);
        } else if (*(char *)((BYTE *)g_unk0x00590d74 + 0xb1b) == 9) {
            lateral = *(int *)(base + 0x170);
            local_14 = -*(int *)(base + 0x168);
            rotor = 0;
        } else {
            if (*(char *)((BYTE *)g_unk0x00590d74 + 0xb1b) == 8) {
                local_14 = -*(int *)(base + 0x168);
                rotor = -*(int *)(base + 0x16c);
            } else {
                local_14 = *(int *)(base + 0x168);
                rotor = *(int *)(base + 0x16c);
            }
            lateral = 0;
        }
        *(short *)(pc + 0x158) = 0x288;
        *(int *)(pc + 0x110) = (int)FUN_00484310;
        break;
    case 3:
        pc[0x150] |= 2;
        if (*(char *)((BYTE *)g_unk0x00590d74 + 0xb1b) == 8) {
            rotor = *(int *)(base + 0x16c);
        } else {
            rotor = *(int *)(base + 0x16c);
            if (*(int *)(g_unk0x00590d78 + 0x24c) < *(int *)(g_unk0x00590d78 + 0x248)) {
                lateral = *(int *)(base + 0x170);
                rotor = -rotor;
            } else {
                rotor = -rotor;
                lateral = -*(int *)(base + 0x170);
            }
        }
        *(short *)(pc + 0x158) = 0;
        *(int *)(pc + 0x110) = (int)FUN_00483570;
        break;
    default:
        goto common;
    }
    *(int *)(pc + 0x170) = 0x10e5;
    *(int *)(pc + 0x174) = 0x1eb8;
    *(int *)(pc + 0x178) = 0x6ccc;

common:
    *(int *)(pc + 0x120) = *(int *)(base + 0xb4) + local_14;
    *(int *)(pc + 0x124) = *(int *)(base + 0xb8) + rotor;
    *(int *)(pc + 0x128) = *(int *)(base + 0xbc) + lateral;
    *(int *)(pc + 0x15c) = FixVecLength((FixVector *)(base + 0x168));
    *(int *)(pc + 0x12c) = *(int *)(base + 0xb4);
    *(int *)(pc + 0x130) = *(int *)(base + 0xb8);
    *(int *)(pc + 0x134) = *(int *)(base + 0xbc);
    *(int *)(pc + 0x114) = *(int *)(base + 0x168);
    *(int *)(pc + 0x118) = *(int *)(base + 0x16c);
    *(int *)(pc + 0x11c) = *(int *)(base + 0x170);
    *(int *)(pc + 0x138) = 0;
    *(int *)(pc + 0x13c) = 0;
    *(int *)(pc + 0x140) = 0;
    *(int *)(pc + 0x144) = 0;
    *(int *)(pc + 0x148) = 0;
    *(int *)(pc + 0x14c) = 0;
    *(int *)(pc + 0x164) = 0;
    *(int *)(pc + 0x168) = 0;
    *(int *)(pc + 0x16c) = 0;
    *(int *)(pc + 0x154) = 0;
    *(int *)(pc + 0x160) = 0;
}
int StageObject_IsEligibleType(short type, int mode, int category);
void Car_SpawnDebris(int size, FixVector *pPos, Car *pCar, FixVector *pAxes, int count, int glassChance);

// Aims and fires the debris/particles of a car part: rotates the part's local
// offset into world space, normalises the part's direction, and spawns debris
// along the cross product of the two.
// match 44%: implementada, difiere la extension del argumento short y el reparto de locales/registros
// FUNCTION: CMR2 0x00483100
void FUN_00483100(int *param_1, unsigned int param_2)
{
    BYTE *car = (BYTE *)g_unk0x00590d74;
    BYTE *pc = (BYTE *)g_unk0x00590c20;
    int bVar8 = g_unk0x00590c24[*(BYTE *)(pc + 0x150) >> 4][(int)*(char *)(car + 0xb1a)];
    // Contiguous: these triples are passed as FixVector* (loose ints are not
    // guaranteed to be adjacent, and a 12-byte write through them overruns).
    FixVector v48, v3c, v30, v24, v18;
    int iVar4, iVar6, iVar9;
    int iVar11, iVar12;
    unsigned int uVar10;

    if (StageObject_IsEligibleType(*(short *)(car + 0xaae + (param_2 & 0xff) * 2), 0, 0) != 0) {
        int off = (unsigned int)bVar8 * 0xc;
        int lVar1 = FixMul(*(int *)(g_unk0x00590d78 + off + 0x16c), 0x20000);
        int t = FixMul(*(int *)(g_unk0x00590d78 + off + 0x170), 0x1cccc);
        int s;

        if (*(int *)(pc + 0x128) < 1)
            s = t;
        else
            s = -t;
        v24.x = FixMul(*(int *)(pc + 0x194), s);
        v24.y = FixMul(*(int *)(pc + 0x198), s);
        v24.z = FixMul(*(int *)(pc + 0x19c), s);

        v18.x = *(int *)(pc + 0x120) + v24.x;
        v18.y = (*(int *)(pc + 0x124) - lVar1) + v24.y;
        v18.z = *(int *)(pc + 0x128) + v24.z;
        FixMatrix_RotateVector(&v24, &v18,
                               *(FixMatrix **)(car + 0x750));

        uVar10 = (unsigned int)FixVecLength((FixVector *)param_1);
        if ((int)uVar10 > 0) {
            int inv = (int)(0x100000000i64 / (__int64)(int)-uVar10);
            v48.x = FixMul(param_1[0], inv);
            v48.y = FixMul(param_1[1], inv);
            v48.z = FixMul(param_1[2], inv);
            param_2 = (unsigned int)FixMul((int)uVar10, 0x50000);
            if ((int)param_2 < 0x10001) {
                if ((int)param_2 < 0x3334)
                    return;
            } else {
                param_2 = 0x10000;
            }
            v3c.x = *(int *)(car + 0x48c);
            v3c.y = *(int *)(car + 0x490);
            v3c.z = *(int *)(car + 0x494);
            iVar4 = FixMul(v48.y, v3c.z) - FixMul(v48.z, v3c.y);
            iVar11 = FixMul(v48.z, v3c.x) - FixMul(v48.x, v3c.z);
            iVar12 = FixMul(v48.x, v3c.y) - FixMul(v48.y, v3c.x);
            {
                FixVector cross;
                int len;
                cross.x = iVar4;
                cross.y = iVar11;
                cross.z = iVar12;
                len = FixVecLength(&cross);
                if (len == 0) {
                    v30.x = 0;
                    v30.y = 0;
                    v30.z = 0;
                } else {
                    FixVecScaleRecip(&v30, &cross, len);
                }
            }
            Car_SpawnDebris(param_2, &v24, (Car *)car,
                            &v48, 0x40000, 0x6666);
        }
    }
    return;
}

int Track_GetGroundHeight(FixVector *pPoint, FixVector *pNormal, short *pTri, unsigned short *pSurface, int defaultY);

// Places one of a car's four contact points on the ground: rotates the local
// point into world space, queries the track height, then builds the tangent
// direction (world axes minus the normal component) and stores the resulting
// contact plane on the point.
// match 26%: implementada, MSVC6 fusiona distinto las normalizaciones y el prologo
// FUNCTION: CMR2 0x00484f40
void FUN_00484f40(unsigned int param_1)
{
    BYTE *car = (BYTE *)g_unk0x00590d74;
    unsigned int local_1c = param_1 & 0xff;
    int idx = *(char *)(car + 0xb1a) * 4;
    int *piVar1 = (int *)(*(int *)((BYTE *)g_unk0x00590c6c + idx) + local_1c * 0x3c);
    int ptr2 = *(int *)((BYTE *)g_unk0x00590c00 + idx) + local_1c * 0x20;

    if (piVar1[0xe] == 0) {
        int surface = 0;
        short tri = 0;
        // Contiguous: these triples are passed as FixVector* (loose ints are not
        // guaranteed to be adjacent, and a 12-byte write through them overruns).
        FixVector v40, v28;
        int local_18 = ptr2;
        int dot, a1, a2, a3;
        unsigned int uVar10;
        int iVar5;

        FixMatrix_RotateVector((FixVector *)piVar1, (FixVector *)ptr2,
                               *(FixMatrix **)(car + 0x750));
        piVar1[0] = piVar1[0] + *(int *)(car + 0x2d0);
        piVar1[1] = piVar1[1] + *(int *)(car + 0x2d4);
        piVar1[2] = piVar1[2] + *(int *)(car + 0x2d8);
        piVar1[1] = Track_GetGroundHeight((FixVector *)piVar1, &v40,
                                          &tri, (unsigned short *)&surface, piVar1[1]);
        FixMatrix_RotateVector(&v28, (FixVector *)(ptr2 + 0xc),
                               *(FixMatrix **)(car + 0x750));

        dot = FixMul(v40.z, v28.z) + FixMul(v40.x, v28.x) + FixMul(v40.y, v28.y);
        a1 = v28.x - FixMul(v40.x, dot);
        a2 = v28.y - FixMul(v40.y, dot);
        a3 = v28.z - FixMul(v40.z, dot);
        uVar10 = (unsigned int)FixSqrt(FixMul(a1, a1) + FixMul(a2, a2) + FixMul(a3, a3));

        if (uVar10 == 0) {
            unsigned int u = (unsigned int)FixMul(v40.x, 0x10000);
            int f9 = 0x10000 - FixMul(v40.x, u);
            int f2 = -FixMul(v40.y, u);
            int f3 = -FixMul(v40.z, u);

            uVar10 = (unsigned int)FixSqrt(FixMul(f9, f9) + FixMul(f2, f2) + FixMul(f3, f3));
            if (uVar10 == 0) {
                v28.x = 0;
                v28.y = 0;
                v28.z = 0;
            } else {
                iVar5 = (int)(0x100000000i64 / (__int64)(int)uVar10);
                v28.x = FixMul(f9, iVar5);
                v28.y = FixMul(f2, iVar5);
                v28.z = FixMul(f3, iVar5);
            }
        } else {
            iVar5 = (int)(0x100000000i64 / (__int64)(int)uVar10);
            v28.x = FixMul(a1, iVar5);
            v28.y = FixMul(a2, iVar5);
            v28.z = FixMul(a3, iVar5);
        }

        dot = *(int *)(local_18 + 0x18);
        piVar1[9] = piVar1[0] + FixMul(v28.x, dot);
        piVar1[10] = piVar1[1] + FixMul(v28.y, dot);
        piVar1[0xb] = piVar1[2] + FixMul(v28.z, dot);
        piVar1[0xe] = 1;
        *(int *)((BYTE *)g_unk0x00590d78 + 0x4c0 + local_1c * 4) = 1;
    }
}


void Vehicle_UpdateMotion(FixVector *pInput);
int FUN_00482f30(void);

// Steps one of a car's four wheel/hub records: derives the suspension strut
// angle from the steering angle, resolves the ground contact (spring
// compression plus the body-to-wheel delta), damps and integrates the local
// motion vector and re-orthonormalises the part basis.
// match 21%: implementada; difieren el reparto de registros/locales y la fusion de las
// normalizaciones inline, y el original relee g_unk0x00590c20 en cada acceso
// FUNCTION: CMR2 0x00483570
void FUN_00483570(void)
{
    BYTE *car = (BYTE *)g_unk0x00590d74;
    BYTE *pc;
    unsigned short angles[3];
    int v2c[3];
    int v38[3];
    int v5c[3];
    int v74[3];
    int v50[3];
    int v44[3];
    int local_c;
    int len;
    int iVar6;
    int iVar9;
    int *wb;

    if (*(char *)(car + 0xb1b) == 8) {
        local_c = 0;
        angles[0] = 0;
        angles[1] = 0;
        angles[2] = (short)((double)FixMul((short)g_unk0x00590c68 * 0x1680, 0x50000) * -0.00017361111111111112);
        FUN_00481560(angles);
    } else {
        if (FUN_00482f30() != 0)
            return;
        pc = (BYTE *)g_unk0x00590c20;
        local_c = 0;
        if ((pc[0x150] & 4) == 0) {
            v5c[0] = 0;
            v5c[1] = 0;
            v5c[2] = 0;
            if (*(int *)(pc + 0x128) < 1) {
                v2c[0] = *(int *)(pc + 0x194) - g_sinTable[*(unsigned short *)(pc + 0x158) & 0xfff];
                v2c[1] = *(int *)(pc + 0x198);
                v2c[2] = *(int *)(pc + 0x19c) - g_sinTable[(*(unsigned short *)(pc + 0x158) + 0x400) & 0xfff];
            } else {
                v2c[0] = g_sinTable[*(unsigned short *)(pc + 0x158) & 0xfff] - *(int *)(pc + 0x194);
                v2c[1] = -*(int *)(pc + 0x198);
                v2c[2] = g_sinTable[(*(unsigned short *)(pc + 0x158) + 0x400) & 0xfff] - *(int *)(pc + 0x19c);
            }
            FixVecLength((FixVector *)v2c);
            v38[0] = FixMul(*(int *)(car + 0x408) - *(int *)(car + 0x414), -0x50000);
            v38[1] = FixMul(*(int *)(car + 0x40c) - *(int *)(car + 0x418), -0xa0000);
            v38[2] = FixMul(*(int *)(car + 0x410) - *(int *)(car + 0x41c), -0x50000);
            if (FixMul(v38[2], v38[2]) + FixMul(v38[0], v38[0]) <= 0x64000 ||
                *(int *)(g_unk0x00590d78 + 0x288) <= 0x8000) {
                v38[1] -= 0x4000;
            } else {
                local_c = 1;
                v74[0] = v38[0];
                v74[1] = v38[1];
                v74[2] = v38[2];
            }
            iVar9 = 0x10000 - g_physicsTimeStep;
            if (iVar9 < 0)
                iVar9 = 0;
            else if (iVar9 > 0x8000)
                iVar9 = 0x8000;
            iVar9 = FixMul(iVar9, 0xc937) + 0x8000;
            *(int *)(pc + 0x164) = FixMul(*(int *)(pc + 0x164), iVar9);
            *(int *)(pc + 0x168) = FixMul(*(int *)(pc + 0x168), iVar9);
            *(int *)(pc + 0x16c) = FixMul(*(int *)(pc + 0x16c), iVar9);
            FixMatrix_InverseRotateVector((FixVector *)v2c, (FixVector *)v38, *(FixMatrix **)(pc + 8));
            v38[0] = v2c[0];
            v38[1] = v2c[1];
            v38[2] = v2c[2];
            if (local_c == 0) {
                v38[0] = v2c[0] + v5c[0];
                v38[1] = v2c[1] + v5c[1];
                v38[2] = v2c[2] + v5c[2];
            } else {
                *(int *)(pc + 0x164) = 0;
                *(int *)(pc + 0x168) = 0;
                *(int *)(pc + 0x16c) = 0;
            }
            v2c[0] = *(int *)(pc + 0x12c) - *(int *)(pc + 0x120);
            v2c[1] = *(int *)(pc + 0x130) - *(int *)(pc + 0x124);
            v2c[2] = *(int *)(pc + 0x134) - *(int *)(pc + 0x128);
            v44[0] = -FixMul(FixMul(v38[1], v2c[2]) - FixMul(v38[2], v2c[1]), *(int *)(pc + 0x170));
            v44[1] = -FixMul(FixMul(v38[2], v2c[0]) - FixMul(v38[0], v2c[2]), *(int *)(pc + 0x174));
            v44[2] = -FixMul(FixMul(v38[0], v2c[1]) - FixMul(v38[1], v2c[0]), *(int *)(pc + 0x178));
            *(int *)(pc + 0x164) += v44[0];
            *(int *)(pc + 0x168) += v44[1];
            *(int *)(pc + 0x16c) += v44[2];
            *(int *)(pc + 0x16c) = 0;
        } else {
            v44[0] = 0;
            v44[1] = 0;
            v44[2] = 0;
        }
        pc[0x150] |= 8;
        if (local_c == 0 && (pc[0x150] & 4) == 0 && ((Unk0x00590d74 *)car)->field_0x778 < 0x7af)
            pc[0x150] &= (BYTE)~8;
        if ((pc[0x150] & 8) == 0) {
            *(int *)(pc + 0x164) = 0;
            *(int *)(pc + 0x168) = 0;
            *(int *)(pc + 0x16c) = 0;
        } else {
            v50[0] = FixMul(*(int *)(pc + 0x164), g_physicsTimeStep);
            v50[1] = FixMul(*(int *)(pc + 0x168), g_physicsTimeStep);
            v50[2] = FixMul(*(int *)(pc + 0x16c), g_physicsTimeStep);
            v50[0] -= v44[0];
            v50[1] -= v44[1];
            v50[2] -= v44[2];
            angles[0] = (short)((double)v50[0] * 0.009947183943243459);
            angles[1] = (short)((double)v50[1] * 0.009947183943243459);
            angles[2] = (short)((double)v50[2] * 0.009947183943243459);
            FixBasis_Rotate((FixBasis *)(pc + 0x17c), angles);
        }
        if (local_c == 0 && (pc[0x150] & 4) == 0) {
            wb = (int *)g_unk0x00590b5c[*(char *)(car + 0xb1a)];
            iVar6 = FixMul(wb[0], *(int *)(pc + 0x194)) + FixMul(wb[1], *(int *)(pc + 0x198)) +
                    FixMul(wb[2], *(int *)(pc + 0x19c));
            if ((pc[0x150] & 8) != 0 &&
                ((*(int *)(pc + 0x128) < 0 && iVar6 < 0) ||
                 (*(int *)(pc + 0x128) > 0 && iVar6 > 0))) {
                v2c[0] = FixMul(wb[0], iVar6);
                v2c[1] = FixMul(wb[1], iVar6);
                v2c[2] = FixMul(wb[2], iVar6);
                v2c[0] = *(int *)(pc + 0x194) - v2c[0];
                v2c[1] = *(int *)(pc + 0x198) - v2c[1];
                v2c[2] = *(int *)(pc + 0x19c) - v2c[2];
                len = FixVecLength((FixVector *)v2c);
                if (len == 0) {
                    *(int *)(pc + 0x194) = 0;
                    *(int *)(pc + 0x198) = 0;
                    *(int *)(pc + 0x19c) = 0;
                } else {
                    FixVecScaleRecip((FixVector *)(pc + 0x194), (FixVector *)v2c, len);
                }
                *(int *)(pc + 0x180) = 0;
                v2c[0] = *(int *)(pc + 0x17c) - v2c[0];
                v2c[1] = *(int *)(pc + 0x180) - v2c[1];
                v2c[2] = *(int *)(pc + 0x184) - v2c[2];
                len = FixVecLength((FixVector *)v2c);
                if (len == 0) {
                    *(int *)(pc + 0x17c) = 0;
                    *(int *)(pc + 0x180) = 0;
                    *(int *)(pc + 0x184) = 0;
                } else {
                    FixVecScaleRecip((FixVector *)(pc + 0x17c), (FixVector *)v2c, len);
                }
                v2c[0] = FixMul(*(int *)(pc + 0x198), *(int *)(pc + 0x184)) -
                         FixMul(*(int *)(pc + 0x19c), *(int *)(pc + 0x180));
                v2c[1] = FixMul(*(int *)(pc + 0x19c), *(int *)(pc + 0x17c)) -
                         FixMul(*(int *)(pc + 0x194), *(int *)(pc + 0x184));
                v2c[2] = FixMul(*(int *)(pc + 0x194), *(int *)(pc + 0x180)) -
                         FixMul(*(int *)(pc + 0x198), *(int *)(pc + 0x17c));
                len = FixVecLength((FixVector *)v2c);
                if (len == 0) {
                    *(int *)(pc + 0x188) = 0;
                    *(int *)(pc + 0x18c) = 0;
                    *(int *)(pc + 0x190) = 0;
                } else {
                    FixVecScaleRecip((FixVector *)(pc + 0x188), (FixVector *)v2c, len);
                }
                iVar6 = FixMul(*(int *)(pc + 0x164), wb[0]) + FixMul(*(int *)(pc + 0x168), wb[1]) +
                        FixMul(*(int *)(pc + 0x16c), wb[2]);
                *(int *)(pc + 0x164) = FixMul(wb[0], iVar6);
                *(int *)(pc + 0x168) = FixMul(wb[1], iVar6);
                *(int *)(pc + 0x16c) = FixMul(wb[2], iVar6);
            }
        } else {
            FixMatrix_SetRight((FixVector *)(pc + 0x17c), *(FixMatrix **)(pc + 4));
            FixMatrix_SetUp((FixVector *)(pc + 0x188), *(FixMatrix **)(pc + 4));
            FixMatrix_SetForward((FixVector *)(pc + 0x194), *(FixMatrix **)(pc + 4));
        }
        iVar9 = FixMul((short)g_unk0x00590c68 * 0x1680, 0x8000);
        if (*(int *)(pc + 0x128) < 0)
            iVar9 = -iVar9;
        angles[1] = 0;
        angles[2] = 0;
        angles[0] = (short)((double)iVar9 * 0.00017361111111111112);
        FUN_00481560(angles);
    }
    VehicleMotion_UpdateWorldPosition();
    if (local_c != 0)
        Vehicle_UpdateMotion((FixVector *)v74);
}


extern int g_unk0x00588970[8];
void ForceFeedback_UpdateSlot(BYTE *pCar, FixVector *pIn, int nonzero);
void FUN_00418ba0(unsigned int view, int strength, int listener);
void FUN_00418c30(unsigned int view, int volume, char heavy, int listener);
void FUN_004675c0(Car *pCar, Car *pOther);
void FUN_00468a80(Car *pCar, int amount);
void FUN_00468c10(Car *pCar);

// GLOBAL: CMR2 0x0051bf94
int g_unk0x0051bf94[3] = { 0x1999, 0x1999, 0x1999 };
// GLOBAL: CMR2 0x0051bfa0
int g_unk0x0051bfa0[3] = { 0x10000, 0x10000, 0x10000 };

// Applies the deformation impulse of a collision to one car: derives the
// impact strength from the body displacement, resolves the impact point and
// normal into the car's deformation frame (mode 0/1/2 choose the projection)
// and refreshes the deformation radius, falloff and scale.
// match 74.95% (auditado W165): logica y constantes identicas; el unico diff de forma era el clamp
// del modo 2 (se compara contra g_stageDeformSpeed, no contra el literal); el resto es reparto de
// registros y de slots de pila (param_6 se recarga en eax en vez de vivir en esi)
// FUNCTION: CMR2 0x00466ef0
void FUN_00466ef0(Car *pCar, int *param_2, FixVector *param_3, int param_4,
                  unsigned char param_5, int param_6)
{
    BYTE *pc = (BYTE *)pCar;
    int vecA[3];
    int V[3];
    int len;
    int saved = 0;
    int carIdx;
    int dot;
    int i;

    if (param_6 == 0) {
        len = FixVecLength((FixVector *)(pc + 0x5c4));
        if (len > 0x10000)
            len = 0x10000;
        else if (len < 0)
            len = 0;
        carIdx = *(char *)(pc + 0xb1a);
        if (carIdx < (int)(RallyDataState() & 0xff)) {
            if (*(int *)(pc + 0xb74) == 0) {
                FUN_00418c30(carIdx, len, 0, carIdx);
            } else if ((unsigned int)(CMain::GetFrameDelta() -
                                      (unsigned int)(&g_unk0x00588a88)[carIdx]) > 10) {
                FUN_00418ba0(carIdx, len, carIdx);
                (&g_unk0x00588a88)[carIdx] = (int)CMain::GetFrameDelta();
            }
        }
        ForceFeedback_UpdateSlot(pc, (FixVector *)(pc + 0x5c4), 1);
    }
    carIdx = *(char *)(pc + 0xb1a);
    if (*(int *)(g_unk0x00588b94 + carIdx * 0x4d0 + 0x45c) != 0 &&
        g_unk0x00588970[carIdx] != 0) {
        if (param_6 == 0) {
            len = FixVecLength((FixVector *)(pc + 0x5c4));
            if (len > 0x10000)
                len = 0x10000;
            else if (len < 0)
                len = 0;
            g_stageDeformStrength = len;
            if (g_stageDeformStrength <= g_unk0x0051bf94[param_5 & 0xff])
                return;
            g_stageDeformStrength = FixMul(g_stageDeformStrength, g_unk0x0051bfa0[param_5 & 0xff]);
            if (g_stageDeformStrength > 0x10000)
                g_stageDeformStrength = 0x10000;
            *(BYTE *)&g_stageDeformMode = param_5;
            g_stageDeformImpact.x = *(int *)(*(int *)(pc + 0x750) + 4);
            g_stageDeformImpact.y = *(int *)(*(int *)(pc + 0x750) + 0x14);
            g_stageDeformImpact.z = *(int *)(*(int *)(pc + 0x750) + 0x24);
            saved = g_stageDeformStrength;
        }
        g_stageDeformCar = pCar;
        switch (g_stageDeformMode & 0xff) {
        case 0:
            if (param_6 == 0) {
                FixMatrix_InverseRotateVector(&g_stageDeformNormal, param_3,
                                              *(FixMatrix **)(pc + 0x750));
                V[0] = param_2[0] - *(int *)(pc + 0x2d0);
                V[1] = param_2[1] - *(int *)(pc + 0x2d4);
                V[2] = param_2[2] - *(int *)(pc + 0x2d8);
                dot = FixMul(param_3->x, V[0]) + FixMul(param_3->y, V[1]) + FixMul(param_3->z, V[2]);
                V[0] = FixMul(param_3->x, dot);
                V[1] = FixMul(param_3->y, dot);
                V[2] = FixMul(param_3->z, dot);
                FixMatrix_InverseRotateVector(&g_stageDeformOffset, (FixVector *)V,
                                              *(FixMatrix **)(pc + 0x750));
                FUN_00468520();
            }
            g_stageDeformRadius = FixMul(g_stageDeformStrength, 0x5999);
            g_stageDeformFalloff = FixMul(g_stageDeformStrength, 0x9999);
            g_stageDeformScale = FixMul(g_stageDeformStrength, 0xb333);
            FUN_004675c0(pCar, (Car *)(g_unk0x00588b94 + carIdx * 0x4d0));
            StageDeform_ApplyRadialDent();
            break;
        case 1:
            if (param_4 < 0x4000)
                param_4 = 0x4000;
            if (param_6 == 0) {
                g_stageDeformSpeed = param_4;
                V[0] = param_2[0] - *(int *)(pc + 0x2d0);
                V[1] = param_2[1] - *(int *)(pc + 0x2d4);
                V[2] = param_2[2] - *(int *)(pc + 0x2d8);
                FixMatrix_InverseRotateVector(&g_stageDeformOffset, (FixVector *)V,
                                              *(FixMatrix **)(pc + 0x750));
                FixMatrix_InverseRotateVector(&g_stageDeformNormal, param_3,
                                              *(FixMatrix **)(pc + 0x750));
                FUN_00468520();
            }
            dot = FixMul(g_stageDeformStrength, 0x8000);
            if (dot > param_4)
                dot = param_4;
            g_stageDeformRadius = dot;
            g_stageDeformFalloff = dot;
            g_stageDeformScale = dot;
            FUN_004675c0(pCar, (Car *)(g_unk0x00588b94 + carIdx * 0x4d0));
            StageDeform_ApplyPlanarDent();
            break;
        default:
            if (param_6 == 0) {
                vecA[0] = *(int *)(pc + 0x2d0) - param_2[0];
                vecA[1] = 0;
                vecA[2] = *(int *)(pc + 0x2d8) - param_2[2];
                len = FixVecLength((FixVector *)vecA);
                if (len == 0) {
                    vecA[0] = 0;
                    vecA[1] = 0;
                    vecA[2] = 0;
                } else {
                    FixVecScaleRecip((FixVector *)vecA, (FixVector *)vecA, len);
                }
                V[0] = param_2[0] - *(int *)(pc + 0x2d0);
                V[1] = param_2[1] - *(int *)(pc + 0x2d4);
                V[2] = param_2[2] - *(int *)(pc + 0x2d8);
                FixMatrix_InverseRotateVector(&g_stageDeformOffset, (FixVector *)V,
                                              *(FixMatrix **)(pc + 0x750));
                FixMatrix_InverseRotateVector(&g_stageDeformNormal, (FixVector *)vecA,
                                              *(FixMatrix **)(pc + 0x750));
                FUN_00468520();
            }
            g_stageDeformSpeed = 0x4000;
            dot = FixMul(g_stageDeformStrength, 0x8000);
            if (dot > g_stageDeformSpeed)
                dot = g_stageDeformSpeed;
            g_stageDeformRadius = dot;
            g_stageDeformFalloff = dot;
            g_stageDeformScale = dot;
            FUN_004675c0(pCar, (Car *)(g_unk0x00588b94 + carIdx * 0x4d0));
            StageDeform_ApplyPlanarDent();
            break;
        }
        if (param_6 == 0) {
            FUN_00468a80(pCar, saved);
            FUN_00468c10(pCar);
            FUN_004692f0(pCar, 0);
        }
    }
}


BYTE FUN_00422fb0(BYTE index);
int Track_GetGroundHeight(FixVector *pPoint, FixVector *pNormal, short *pTri,
                          unsigned short *pSurface, int defaultY);

// Per-view vertical offset table, 25 entries (5x5 contact grid) per view.
// GLOBAL: CMR2 0x00538d7c
int g_unk0x00538d7c[44];


// GLOBAL: CMR2 0x00543fb0
StageDeformNode g_unk0x00543fb0[400];

// Steps the deformation record of one view: advances the record position from
// the view node's forward vector, resamples the 5x5 contact grid and
// integrates the per-node heights and angles.
// match 28%: implementada; el original reutiliza slots de pila (param_3) como temporales y
// reparte distinto registros y contadores de los dos bucles
// FUNCTION: CMR2 0x0045f9d0
void FUN_0045f9d0(int param_1, int *param_2, int param_3)
{
    int *rec = param_2;
    int pos[3];
    int fwd[3];
    int pt[3];
    int normal[3];
    int gn[3];
    int dv[3];
    int sv[3];
    int corr[3];
    int minY;
    int wheelScale = 0;
    int i, j;
    int sum;
    int len;
    int scale;
    int n, base;
    int hx, hy, hz;
    int tmp2;
    int gx = 0, gz = 0;
    int x0, z0, z;
    int *pHeight;
    short *pTri;
    unsigned short surface;

    StageTiming_UpdateEventDisplacement(rec);
    FUN_0045e9a0((SceneNode *)rec);
    FixMatrix_GetPosition((FixVector *)pos, (FixMatrix *)((BYTE *)g_viewNodes[param_3] + 0x98));
    FixMatrix_GetForward((FixVector *)fwd, (FixMatrix *)((BYTE *)g_viewNodes[param_3] + 0x98));
    sum = 0;
    {
        int *p = (int *)((BYTE *)Car_Get(FUN_00422fb0((BYTE)param_3)) + 0x8dc);
        for (i = 0; i < 4; i++)
            sum += p[i];
    }
    len = FixMul(sum, 0x4000) + 0x6c000;
    minY = pos[1] + 0x60000;
    if (len < minY)
        minY = len;
    if (rec[0] == 2) {
        rec[0xe] = fwd[2];
        rec[0xf] = 0;
        rec[0x10] = -fwd[0];
        len = FixVecLength((FixVector *)(rec + 0xe));
        if (len == 0) {
            rec[0xe] = 0;
            rec[0xf] = 0;
            rec[0x10] = 0;
        } else {
            FixVecScaleRecip((FixVector *)(rec + 0xe), (FixVector *)(rec + 0xe), len);
        }
        scale = FixMul(g_unk0x00547940, 0x8000);
        sv[0] = FixMul(g_unk0x00547930.z, scale);
        sv[1] = 0;
        sv[2] = FixMul(-g_unk0x00547930.x, scale);
    }
    fwd[0] = FixMul(fwd[0], 0xe0000);
    fwd[1] = FixMul(fwd[1], 0xe0000);
    fwd[2] = FixMul(fwd[2], 0xe0000);
    pos[0] += fwd[0];
    pos[1] += fwd[1];
    pos[2] += fwd[2];
    if (pos[1] < minY)
        pos[1] = minY;
    dv[0] = FixMul(pos[0] - rec[5], g_unk0x0051bd40);
    dv[1] = 0;
    dv[2] = FixMul(pos[2] - rec[7], g_unk0x0051bd40);
    rec[8] = rec[0xb] - dv[0];
    rec[9] = rec[0xc] - dv[1];
    rec[5] = pos[0];
    rec[6] = pos[1];
    rec[10] = rec[0xd] - dv[2];
    rec[7] = pos[2];
    scale = FixMul(FixDiv(g_unk0x00538d7c[param_3 * 25], 0xa000) - 0x10000, 0x4ccc);
    gn[0] = FixMul(fwd[0], scale);
    gn[1] = FixMul(fwd[1], scale);
    gn[2] = FixMul(fwd[2], scale);
    pos[0] += gn[0];
    rec[2] = pos[0];
    pos[1] += gn[1];
    rec[3] = pos[1];
    pos[2] += gn[2];
    rec[4] = pos[2];
    corr[0] = FixMul(rec[0xb], g_unk0x0051bd3c);
    corr[1] = FixMul(rec[0xc], g_unk0x0051bd3c);
    corr[2] = FixMul(rec[0xd], g_unk0x0051bd3c);
    rec[0x11] = FixMul(corr[0], -0x10000);
    rec[0x12] = FixMul(corr[1], -0x10000);
    rec[0x13] = FixMul(corr[2], -0x10000);
    if (rec[0x12] == 0)
        rec[0x14] = 0;
    else
        rec[0x14] = FixDiv(0x10000, rec[0x12]);
    if (rec[0] == 2) {
        wheelScale = FixMul(rec[0x16], g_unk0x00543da0);
        if (wheelScale > 0x10000)
            wheelScale = 0x10000;
        wheelScale = FixMul(wheelScale, g_unk0x0051bd3c);
    }
    if (rec[0] == 1) {
        x0 = rec[2] + 0x9999a;
        z0 = rec[4] + 0x9999a;
        pTri = (short *)((BYTE *)rec + 0x78);
        pHeight = rec + 0x2b;
        for (i = 0; i < 5; i++) {
            z = z0;
            for (j = 0; j < 5; j++) {
                pt[2] = z;
                pt[1] = 0;
                pt[0] = x0;
                hz = Track_GetGroundHeight((FixVector *)pt, (FixVector *)normal, pTri, &surface,
                                           *pHeight);
                *pHeight = hz;
                z -= 0x4cccc;
                pHeight[0x19] = (rec[3] - hz) + -0x61999;
                pTri++;
                pHeight++;
            }
            x0 -= 0x4cccc;
        }
    }
    n = *(short *)((BYTE *)rec + 0x74);
    base = *(short *)((BYTE *)rec + 0x76);
    for (i = 0; i < n; i++) {
        StageDeformNode *pNode = &g_unk0x00543fb0[base + i];
        hx = pNode->x + (corr[0] - rec[2]);
        hy = pNode->y + (corr[1] - rec[3]);
        hz = pNode->z + (corr[2] - rec[4]);
        if (rec[0] == 2) {
            scale = FixMul(wheelScale, pNode->scale);
            hx += FixMul(sv[0], scale);
            hy += FixMul(sv[1], scale);
            hz += FixMul(sv[2], scale);
        }
        pNode->wrapped = 0;
        if (hx < 0xc0001) {
            if (hx < -0xc0000)
                hx += ((0xbffff - hx) / 0x180000) * 3 * 0x80000;
        } else {
            hx += ((hx + 0xbffff) / 0x180000) * -3 * 0x80000;
        }
        if (rec[0] == 1) {
            gx = 0;
            if ((int)(0xc0000 - hx) > 0x4cccb)
                gx = (0xc0000 - hx) / 0x4cccc;
            gz = 0;
            if ((int)(0xc0000 - hz) > 0x4cccb)
                gz = (0xc0000 - hz) / 0x4cccc;
            if (gx < 5 && gz < 5)
                hy += rec[gx * 5 + gz + 0x44];
        }
        if (hy < 0x60001) {
            if (hy < -0x60000) {
                do {
                    hy += 0xc0000;
                } while (hy < -0x60000);
                pNode->wrapped = 1;
            }
        } else {
            do {
                hy += -0xc0000;
            } while (0x60000 < hy);
        }
        if (rec[0] == 1 && gx < 5 && gz < 5) {
            hy -= rec[gz + gx * 5 + 0x44];
            pNode->angle = rec[gz + gx * 5 + 0x2b] + 0x1999;
        }
        if (hz < 0xc0001) {
            while (hz < -0xc0000)
                hz += 0x180000;
        } else {
            do {
                hz += -0x180000;
            } while (0xc0000 < hz);
        }
        pNode->x = rec[2] + hx;
        pNode->y = rec[3] + hy;
        pNode->z = hz + rec[4];
        if (rec[0] == 2) {
            pNode->angle += FixMul(pNode->spin, g_unk0x0051bd3c);
            if (pNode->angle > 0x1680000)
                pNode->angle -= 0x1680000;
        }
    }
    if (param_1 != 0) {
        if (rec[0x16] != rec[0x17]) {
            if (rec[0x16] < rec[0x17]) {
                tmp2 = rec[0x16] + FixMul(rec[0x18], g_unk0x0051bd3c);
                rec[0x16] = tmp2;
                if (rec[0x17] < tmp2) {
                    rec[0x16] = rec[0x17];
                    return;
                }
            } else {
                tmp2 = rec[0x16] - FixMul(rec[0x18], g_unk0x0051bd3c);
                rec[0x16] = tmp2;
                if (tmp2 < rec[0x17])
                    rec[0x16] = rec[0x17];
            }
        }
    }
}

// Helpers implemented in other translation units.
void FUN_0046d8d0(int param_1, int *param_2);
extern int g_unk0x00590c44;   // first of the four slot clocks 0x590c44..0x590c53
extern int g_unk0x00590c50;   // last one (0x590c44 + 3 * 4)
extern BYTE g_unk0x00590c60[4];

// Installs the model geometry of part `index` (byte index into the car table)
// into car record `param_1`.
// FUNCTION: CMR2 0x0046c4e0
void FUN_0046c4e0(int param_1, BYTE index)
{
    Car *pCar = Car_Get(index);

    FUN_0046d8d0((int)pCar, (int *)param_1);
}

// Wakes the wheels of the current car whose static slot record has no model
// installed yet and whose slot clock is still below the load threshold: the
// slot is empty (0x469bc0 returns 0), the car's slot entry is 0 and the source
// value of the slot exceeds the per-slot threshold.
// FUNCTION: CMR2 0x00480de0
void FUN_00480de0(void)
{
    int slot;
    int offset;

    offset = 0;
    for (slot = 3; slot >= 0; slot--) {
        if (FUN_00469bc0(g_unk0x00590d74, slot) == 0 &&
            *(int *)(*(char *)((BYTE *)g_unk0x00590d74 + 0xb1a) * 0x1a0 +
                     *(int *)((int)&g_unk0x00590d7c[3] + offset)) == 0 &&
            *(int *)(g_unk0x00590d78 + 0x240 + g_unk0x00590c60[slot] * 4) >
                *(int *)((int)&g_unk0x00590c50 + offset)) {
            FUN_00480e50(slot);
        }
        offset -= 4;
    }
}

// ---------------------------------------------------------------------------
// Car split-bar state update (0x498620).

int StageObject_Atan2Degrees(int y, int x);
int RallyData_FUN_00421430(void);
void RallyData_FUN_00421530(int index, int *pOut);
extern double g_unk0x00511300;

// Base of the per-variant split reference table; the code indexes it with a
// 1-based variant number, so entry `v` lives at 0x592744 + v * 4.
// GLOBAL: CMR2 0x00592744
int g_unk0x00592744[1];

// Recomputes one car's split-bar angles and times from the current route node
// of the output record: normalises the six neighbouring node indices, measures
// the car's heading against the node directions, converts the speed into a
// distance and fills the requested fields of the record (selected by the bit
// mask of param_2).
// match 30%: misma secuencia de operaciones, constantes y llamadas (85% de
// opcodes identicos) pero MSVC6 reparte los locales en un marco de 0x178 bytes
// (nosotros 0x5c) y elige otros registros para el vector de nodo y los
// temporales de 64 bits; no reproducible sin el reparto de pila original.
// FUNCTION: CMR2 0x00498620
void FUN_00498620(Car *pCar, unsigned int mask, int *pOut, int variant)
{
    BYTE *pc;
    BYTE *pRef;
    FixVector node;
    int angles[6];
    int d4;
    int d0;
    int cc;
    int refAngle;
    unsigned int len;
    int curAngle;
    int frontAngle;
    int sideAngle;
    int baseAngle;
    int px;
    int pz;
    int nx;
    int nz;
    int bx;
    int speed;
    int i;
    int tmp;
    int t;
    int a;
    int b;
    int idxCount;
    int nodeIndex;
    unsigned int sum;
    unsigned int uVar7;
    unsigned int uVar8;

    pc = (BYTE *)pCar;
    idxCount = RallyData_FUN_00421430();
    nodeIndex = pOut[0x15];
    angles[1] = nodeIndex;
    angles[0] = nodeIndex - 1;
    angles[4] = nodeIndex + 2;
    angles[3] = nodeIndex + 2;
    angles[5] = nodeIndex + 1;
    angles[2] = nodeIndex + 5;
    for (i = 0; i < 6; i++) {
        if (angles[i] >= idxCount)
            angles[i] -= idxCount;
        if (angles[i] < 0)
            angles[i] = idxCount - 1;
    }
    frontAngle = StageObject_Atan2Degrees(*(int *)(pc + 0x368), *(int *)(pc + 0x360));
    pOut[4] = frontAngle;
    sideAngle = StageObject_Atan2Degrees(*(int *)(pc + 0x410), *(int *)(pc + 0x408));
    px = *(int *)(pc + 0x2d0);
    pz = *(int *)(pc + 0x2d8);
    RallyData_FUN_00421530(angles[2], (int *)&node);
    nz = node.z;
    bx = node.x;
    if ((mask & 1) != 0)
        d4 = StageObject_Atan2Degrees(nz - pz, bx - px);
    if (variant != 0) {
        pRef = (BYTE *)*(int *)((int *)&g_unk0x00592744 + variant);
        cc = StageObject_Atan2Degrees(
            *(int *)(pRef + 0x1158 + angles[4] * 4) - pz,
            *(int *)(pRef + 0xb90 + angles[4] * 4) - px);
    }
    if ((mask & 2) != 0) {
        RallyData_FUN_00421530(angles[3], (int *)&node);
        d0 = StageObject_Atan2Degrees(node.z - pz, node.x - px);
    }
    speed = *(unsigned int *)(pc + 0x778);
    baseAngle = (int)((double)(int)(((__int64)speed << 16) / 0x4000000) * CGraphics::m_65536);
    RallyData_FUN_00421530(angles[1], (int *)&node);
    nx = node.x;
    nz = node.z;
    RallyData_FUN_00421530(angles[0], (int *)&node);
    curAngle = StageObject_Atan2Degrees(nx - node.x, nz - node.z);
    if (curAngle < 0xb40000) {
        if (curAngle < -0xb40000)
            curAngle += 0x1680000;
    } else {
        curAngle -= 0x1680000;
    }
    pOut[0x13] = curAngle;
    pOut[0x14] = StageObject_Atan2Degrees(nx - pz, nz - px);
    a = px - nx;
    b = pz - nz;
    sum = FixMul(a, a) + FixMul(b, b);
    if (sum == 0)
        len = 0;
    else
        len = (unsigned int)FixSqrt(sum);
    if (variant != 0) {
        pRef = (BYTE *)*(int *)((int *)&g_unk0x00592744 + variant);
        refAngle = StageObject_Atan2Degrees(
            *(int *)(pRef + 0x1158 + angles[1] * 4) - *(int *)(pRef + 0x1158 + angles[0] * 4),
            *(int *)(pRef + 0xb90 + angles[1] * 4) - *(int *)(pRef + 0xb90 + angles[0] * 4));
    }
    tmp = StageObject_Atan2Degrees(node.z - nz, bx - nx);
    tmp = curAngle - tmp;
    if (tmp < 0xb40000) {
        if (tmp < -0xb40000)
            tmp += 0x1680000;
    } else {
        tmp -= 0x1680000;
    }
    pOut[0x17] = tmp;
    for (i = 0; i < 0x20; i++) {
        if ((mask & (1 << i)) != 0) {
            switch (i) {
            case 0:
                tmp = frontAngle - d4;
                if (tmp < 0xb40000) {
                    if (tmp < -0xb40000)
                        tmp += 0x1680000;
                    pOut[6] = tmp;
                } else {
                    pOut[6] = tmp - 0x1680000;
                }
                break;
            case 1:
                tmp = frontAngle - d0;
                if (tmp < 0xb40000) {
                    if (tmp < -0xb40000)
                        tmp += 0x1680000;
                    pOut[7] = tmp;
                } else {
                    pOut[7] = tmp - 0x1680000;
                }
                break;
            case 3:
                pOut[3] = *(int *)(pc + 0x870);
                break;
            case 4:
                pRef = (BYTE *)*(int *)((int *)&g_unk0x00592744 + variant);
                pOut[0x28] = baseAngle - *(int *)(pRef + 0x5c8 + angles[5] * 4);
                break;
            case 6:
                *pOut = baseAngle;
                break;
            case 8:
                tmp = curAngle - pOut[0x14];
                if (tmp < 0xb40000) {
                    if (tmp < -0xb40000)
                        tmp += 0x1680000;
                } else {
                    tmp -= 0x1680000;
                }
                pOut[0xd] = FixMul((int)len,
                                   g_sinTable[(unsigned short)(int)((double)tmp * g_unk0x00511300) & 0xfff]);
                break;
            case 9:
                tmp = frontAngle - curAngle;
                if (tmp < 0xb40000) {
                    if (tmp < -0xb40000)
                        tmp += 0x1680000;
                    pOut[0xe] = tmp;
                } else {
                    pOut[0xe] = tmp - 0x1680000;
                }
                break;
            case 0xa:
                tmp = frontAngle - sideAngle;
                if (tmp < 0xb40000) {
                    if (tmp < -0xb40000)
                        tmp += 0x1680000;
                    pOut[1] = tmp;
                } else {
                    pOut[1] = tmp - 0x1680000;
                }
                break;
            case 0xb:
                pOut[2] = (int)*(short *)(pc + 0xb10);
                break;
            case 0xc:
                pRef = (BYTE *)*(int *)((int *)&g_unk0x00592744 + variant);
                pOut[0x18] = baseAngle - *(int *)(pRef + 0x5c8 + angles[1] * 4);
                break;
            case 0xe:
                pRef = (BYTE *)*(int *)((int *)&g_unk0x00592744 + variant);
                a = *(int *)(pRef + angles[1] * 4);
                b = *(int *)(pRef + 0x5c8 + angles[1] * 4);
                t = a;
                if (0x3c0000 < b) {
                    t = curAngle;
                    if (0x27ffff < baseAngle) {
                        t = a;
                        if (0 < b - baseAngle) {
                            uVar7 = FixMul(b - 0x280000, 0xb333);
                            uVar8 = (uVar7 - (b - 0x280000)) + (b - baseAngle);
                            if (0 < (int)uVar8) {
                                tmp = a - curAngle;
                                if (tmp < 0xb40000) {
                                    if (tmp < -0xb40000)
                                        tmp += 0x1680000;
                                } else {
                                    tmp -= 0x1680000;
                                }
                                t = a - FixMul((int)(((__int64)(int)uVar8 << 16) / (int)uVar7), tmp);
                            }
                        }
                    }
                }
                tmp = frontAngle - t;
                if (tmp < 0xb40000) {
                    if (tmp < -0xb40000)
                        tmp += 0x1680000;
                    pOut[0x19] = tmp;
                } else {
                    pOut[0x19] = tmp - 0x1680000;
                }
                break;
            case 0x12:
                tmp = frontAngle - refAngle;
                if (tmp < 0xb40000) {
                    if (tmp < -0xb40000)
                        tmp += 0x1680000;
                    pOut[0x1e] = tmp;
                } else {
                    pOut[0x1e] = tmp - 0x1680000;
                }
                break;
            case 0x14:
                tmp = frontAngle - cc;
                if (tmp < 0xb40000) {
                    if (tmp < -0xb40000)
                        tmp += 0x1680000;
                } else {
                    tmp -= 0x1680000;
                }
                pOut[0x20] = tmp;
                break;
            }
        }
    }
}

// GLOBAL: CMR2 0x0051ac20
char g_strTexRocksGrav[] = "\\NEWIMAGE\\Rocks\\Grav_";
// GLOBAL: CMR2 0x0051ac38
char g_strTexSplashDrop[] = "\\NEWIMAGE\\WSplash\\drop.tga";
// GLOBAL: CMR2 0x0051ac54
char g_strTexGrass[] = "\\NEWIMAGE\\GRASS\\GR_";
// GLOBAL: CMR2 0x0051ac68
char g_strTexLeaf[] = "\\NEWIMAGE\\Leaf\\leaf_";
// GLOBAL: CMR2 0x0051ac80
char g_strTexDetail[] = "\\NewImage\\Det\\13_12\\DE_";
// GLOBAL: CMR2 0x0051ac98
char g_strNumberedTga[] = "%s%s%03d.tga";
// GLOBAL: CMR2 0x0051aca8
char g_strTexDustCloud[] = "\\NEWIMAGE\\DustCld\\23_5\\D_";
// GLOBAL: CMR2 0x0051acc4
char g_strTexSnowSpray[] = "\\NEWIMAGE\\SnowCld\\snow_spray.tga";
// GLOBAL: CMR2 0x0051ace8
char g_strTexSplashSpray2f[] = "\\NEWIMAGE\\Wsplash\\spray2f.tga";
// GLOBAL: CMR2 0x0051ad08
char g_strTexSplashSpray2[] = "\\NEWIMAGE\\Wsplash\\spray2.tga";
// GLOBAL: CMR2 0x0051ad28
char g_strTexWheelSpray[] = "\\NEWIMAGE\\WSpray\\spray2.tga";
// GLOBAL: CMR2 0x0051ad44
char g_strTexSnowCloud[] = "\\NEWIMAGE\\SnowCld\\snow_004.tga";
// GLOBAL: CMR2 0x0051ad64
char g_strTexExhaust2[] = "\\NEWIMAGE\\Exhaust\\exhst02.tga";
// GLOBAL: CMR2 0x0051ad84
char g_strTexExhaust3[] = "\\NEWIMAGE\\Exhaust\\exhst03.tga";
// GLOBAL: CMR2 0x0051ada4
char g_strTexGlass[] = "\\NEWIMAGE\\glass.tga";
// GLOBAL: CMR2 0x0051adb8
char g_strTexPaint[] = "\\NEWIMAGE\\paint.tga";
// GLOBAL: CMR2 0x0051adcc
char g_strTexSpark[] = "\\NEWIMAGE\\spark4.tga";

extern char g_strPathConcat[];
struct Unk0x004a3e20;
void FUN_004a3e20(Unk0x004a3e20 *pObject, int value);
void CarEffects_InitUVs(int unused1, int unused2);

#define LOAD_EFFECT_TEXTURE() \
    (int)CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile0(), CFrontend::m_stringDest, &loaded, NULL, 0, 0x10)

// Loads the textures of the particle effects (sparks, debris, exhaust, spray,
// snow, dust, leaves, grass, gravel) from the stage archive.
// FUNCTION: CMR2 0x00459c80
void FUN_00459c80(void)
{
    bool loaded;
    int i;

    CMain::GetFrameDelta();
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::FUN_0040ed50(), g_strTexSpark);
    g_unk0x00543650 = LOAD_EFFECT_TEXTURE();
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::FUN_0040ed50(), g_strTexPaint);
    g_unk0x00543654 = LOAD_EFFECT_TEXTURE();
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::FUN_0040ed50(), g_strTexGlass);
    g_unk0x00543658 = LOAD_EFFECT_TEXTURE();
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::FUN_0040ed50(), g_strTexExhaust3);
    g_unk0x0054365c = LOAD_EFFECT_TEXTURE();
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::FUN_0040ed50(), g_strTexExhaust2);
    g_unk0x00543660 = LOAD_EFFECT_TEXTURE();
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::FUN_0040ed50(), g_strTexSnowCloud);
    g_unk0x0054366c = LOAD_EFFECT_TEXTURE();
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::FUN_0040ed50(), g_strTexWheelSpray);
    g_unk0x00543668 = LOAD_EFFECT_TEXTURE();
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::FUN_0040ed50(), g_strTexSplashSpray2);
    g_unk0x00543670 = LOAD_EFFECT_TEXTURE();
    FUN_004a3e20((Unk0x004a3e20 *)g_unk0x00543670, 1);
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::FUN_0040ed50(), g_strTexSplashSpray2f);
    g_unk0x00543674 = LOAD_EFFECT_TEXTURE();
    FUN_004a3e20((Unk0x004a3e20 *)g_unk0x00543674, 1);
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::FUN_0040ed50(), g_strTexSnowSpray);
    g_unk0x00543678 = LOAD_EFFECT_TEXTURE();
    for (i = 0; i < 8; i++) {
        sprintf(CFrontend::m_stringDest, g_strNumberedTga, CInstallInfo::FUN_0040ed50(), g_strTexDustCloud, i);
        g_unk0x005436b8[i] = LOAD_EFFECT_TEXTURE();
    }
    for (i = 0; i < 4; i++) {
        sprintf(CFrontend::m_stringDest, g_strNumberedTga, CInstallInfo::FUN_0040ed50(), g_strTexDetail, i + 1);
        g_unk0x005436d8[i] = LOAD_EFFECT_TEXTURE();
        FUN_004a3e20((Unk0x004a3e20 *)g_unk0x005436d8[i], 1);
    }
    for (i = 0; i < 5; i++) {
        sprintf(CFrontend::m_stringDest, g_strNumberedTga, CInstallInfo::FUN_0040ed50(), g_strTexLeaf, i);
        g_unk0x0054367c[i] = LOAD_EFFECT_TEXTURE();
    }
    for (i = 0; i < 2; i++) {
        sprintf(CFrontend::m_stringDest, g_strNumberedTga, CInstallInfo::FUN_0040ed50(), g_strTexGrass, i + 1);
        g_unk0x00543690[i] = LOAD_EFFECT_TEXTURE();
    }
    for (i = 0; i < 2; i++) {
        sprintf(CFrontend::m_stringDest, g_strNumberedTga, CInstallInfo::FUN_0040ed50(), g_strTexGrass, i + 3);
        g_unk0x00543698[i] = LOAD_EFFECT_TEXTURE();
    }
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::FUN_0040ed50(), g_strTexSplashDrop);
    g_unk0x005435c8 = LOAD_EFFECT_TEXTURE();
    FUN_004a3e20((Unk0x004a3e20 *)g_unk0x005435c8, 1);
    for (i = 0; i < 5; i++) {
        sprintf(CFrontend::m_stringDest, g_strNumberedTga, CInstallInfo::FUN_0040ed50(), g_strTexRocksGrav, i);
        g_unk0x005436a0[i] = LOAD_EFFECT_TEXTURE();
    }
    CMain::GetFrameDelta();
    g_unk0x00543664 = g_unk0x00543660;
    CarEffects_InitUVs(g_unk0x00543650, g_unk0x00543654);
}
#undef LOAD_EFFECT_TEXTURE


unsigned int FUN_00456ca0(void);
void Car_AllocateTable(int count);
void FUN_004667c0(int count);
void FUN_00480900(int count);
void FUN_00494b50(int count);
void FUN_0045e5b0(int count);
void FUN_0042b740(int first, int count);
void FUN_0042b5b0(int first, int count);
void FUN_00433840(int value);
void FUN_00466630(int value);
void FUN_0042b7e0(void);
unsigned int FUN_00409cb0(int index);
void FUN_0040aff0(int index, int value);
void FUN_004584d0(char param_1);
void FUN_0040b1c0(void);
unsigned char FUN_00457000(int car);
void FUN_00420850(Car *pCar);
void FUN_00458480(void);
int FUN_00457ed0(void);
void FUN_0042b660(int count);
short *Car_GetOrder(void);
short Car_GetOrderCount(void);
void FUN_004809e0(short *pList, short count);
void FUN_00480980(void);
void FUN_004669b0(int first, int count);
void FUN_004668d0(void);
void FUN_004669f0(int, int, short *, short);
void FUN_0045c610(int a, int b, int count);
void Particle_KillAll(void);
void FUN_0042c840(int first, int count);
void FUN_0043ecd0(Car *pCar);
void FUN_00465fc0(Car *pCar);
void StageTiming_FUN_00456d20(SceneNode *pNode, BYTE colour);
unsigned int RallyData_GetFlag22(void);
unsigned int RallyData_GetFlag31(void);
void FUN_00456d60(SceneNode *pNode, BYTE colour);

// Loads the stage's cars: allocates and clears the car and effect tables,
// numbers the network players, loads every car, builds the race order and
// the per-car effects, and sets the cars' model colours. Returns 0 when a car
// failed to load.
// FUNCTION: CMR2 0x00456d90
int FUN_00456d90(void)
{
    int count;
    int i;
    int number;
    SceneNode *pNode;

    count = FUN_00456ca0();
    memset(g_unk0x00542630 + 0x398, 0, 0x20);
    Car_AllocateTable(count);
    FUN_004667c0(count);
    FUN_00480900(count);
    FUN_00494b50(count);
    FUN_0045e5b0(count);
    memset(g_unk0x00542630 + 0x2d4, 0, 0x40);
    memset(g_unk0x00542630 + 0x294, 0, 0x40);
    memset(g_unk0x00542630 + 0x254, 0, 0x40);
    memset(g_unk0x00542630 + 0x314, 0, 0x40);
    memset(g_unk0x00542630 + 0x354, 0, 0x40);
    for (i = 0; i < 16; i++) {
        *(int *)(g_unk0x00542630 + i * 0x24 + 8) = 0;
        *(int *)(g_unk0x00542630 + i * 0x24 + 4) = 0;
    }
    FUN_0042b740(0, count);
    FUN_0042b5b0(0, count);
    FUN_00433840(0x190000);
    FUN_00466630(0x190000);
    FUN_0042b7e0();
    if (CGameInfo::FUN_00405e00()) {
        number = 1;
        for (i = 0; i < 7; i++) {
            if ((char)FUN_00409cb0(i)) {
                FUN_0040aff0(i, number);
                number++;
            }
        }
    }
    if ((char)RallyData_GetFlag22() || (char)RallyData_GetFlag31())
        FUN_004584d0(0);
    FUN_0040b1c0();
    for (i = 0; i < count; i++) {
        if (FUN_00457000(i) == 0)
            return 0;
        FUN_00420850(Car_Get(i));
    }
    if ((char)RallyData_GetFlag22())
        FUN_00458480();
    CGame::RegisterCallback(FUN_00457ed0, NULL);
    FUN_0042b660(count);
    FUN_004809e0(Car_GetOrder(), Car_GetOrderCount());
    FUN_00480980();
    FUN_004669b0(0, count);
    FUN_004668d0();
    FUN_004669f0(0, 0, Car_GetOrder(), Car_GetOrderCount());
    FUN_0045c610(0, 0, (BYTE)RallyDataState());
    Particle_KillAll();
    FUN_0042c840(0, count);
    for (i = 0; i < count; i++)
        FUN_0043ecd0(Car_Get(i));
    if ((char)RallyData_FUN_00407ea0() && (char)CGameInfo::FUN_00406310())
        FUN_00465fc0(Car_Get(count - 1));
    for (i = 0; i < count; i++) {
        StageTiming_FUN_00456d20(*(SceneNode **)((BYTE *)Car_Get(i) + 0x71c), 0);
        StageTiming_FUN_00456d20(*(SceneNode **)((BYTE *)Car_Get(i) + 0x720), 0);
        FUN_00456d60(*(SceneNode **)(*(BYTE **)((BYTE *)Car_Get(i) + 0x71c) + 4), 0);
        FUN_00456d60(*(SceneNode **)(*(BYTE **)((BYTE *)Car_Get(i) + 0x720) + 4), 0);
        pNode = SceneNode_FindByType(*(SceneNode **)((BYTE *)Car_Get(i) + 0x720), 0xe);
        if (pNode != NULL)
            StageTiming_FUN_00456d20(pNode, 0xff);
    }
    return 1;
}

// Model detail letter of each car, by game mode and slot ('A' = full detail
// with damage parts, 'C'/'D' = simpler models).
// Uses the shared detail settings updated by Graphics.cpp.
// Team of the previous knockout opponent (-1 = none yet).
// GLOBAL: CMR2 0x0051a8b8
int g_unk0x0051a8b8 = -1;
// GLOBAL: CMR2 0x0051a8c4
char g_strLightWheelsC3d[] = "L.c3d";
// GLOBAL: CMR2 0x0051a8cc
char g_strSnowWheelsC3d[] = "S.c3d";
// GLOBAL: CMR2 0x0051a8d4
char g_strCarBflFormat[] = "%s%c%d.bfl";
// GLOBAL: CMR2 0x0051a8e0
char g_strCinFormat[] = "%s.cin";
// GLOBAL: CMR2 0x0051a8e8
char g_strDamageModelC3d[] = "A1N.c3d";
// GLOBAL: CMR2 0x0051a8f4
char g_strCarC3dExt[] = ".C3D";
// GLOBAL: CMR2 0x0051a8fc
char g_strCarModelFormat[] = "%s%c%d";

extern char g_strPathConcat[];
extern char g_strBflFormat[];
void FUN_00457c50(void);
int FUN_00407270(void);
int FUN_0041b380(void);
unsigned int *RallyData_GetChampionshipState(void);
int FUN_004660f0(void);
BYTE FUN_00407fc0(int param1);
int StageTiming_FUN_00455ab0(int iSplit);
int RallyTiming_GetStagePositionOfDriver(int iDriver);
int StageTiming_GetDriverSlot(int iDriver);
void FUN_0040b200(int index);
unsigned int FUN_00409d00(int index);
int RallyData_FUN_00408010(int index);
void RallyData_FUN_00408600(BYTE index, BYTE value);
char *FUN_004200d0(int car);
void FUN_004a3dc0(int param1);
int FUN_004b9380(unsigned int, unsigned int, unsigned int);
int *FUN_00456c70(int car);
int FUN_00456c90(int unused);
int FUN_0042cae0(Car *pCar, int param2);
BOOL FUN_0042cb50(Car *pCar);
void FUN_0042cb90(char mode, SceneNode **pWheels);
void Scene_AddShadowCaster(SceneNode *pNode, int exactMeshes);
void SceneNode_SetMeshFlagBits(SceneNode *pNode, unsigned int value);
void FUN_0046b400(int value, int index);
void FUN_0045c6b0(int player, int wheel);
void FUN_00458050(int unused, int slot, int type);

#define CAR_RECORD(i) (g_unk0x00542630 + (i) * 0x24)
#define MESH_OF(node) (*(int *)((BYTE *)(node) + 0xc))

// Loads car `car` of the stage: picks its model (by game mode: the player's
// car, the championship/knockout opponents, the network players...), the
// model's detail letter, then loads the model, its textures and cinematic
// data, the snow/light wheel set of full-detail cars and, for CPU cars in the
// rally modes, the damaged body variant. Returns 0 when a file is missing.
// FUNCTION: CMR2 0x00457000
BYTE FUN_00457000(int car)
{
    char path[260];
    unsigned int *pSlot;
    unsigned int *pChamp;
    unsigned int state;
    unsigned int slot;
    unsigned int mode;
    unsigned int model;
    int team;
    int best;
    int i;
    int n;
    int lod;
    int letterInt;
    char letter;
    char variant;
    char isPlayer;
    char twoPlayers;
    char rallyMode;
    char hasVariant;
    char knockout;
    BYTE teamByte;
    BYTE *pRecord;
    Unk0x542ae8 *pArchive;
    void *pData;
    void *pWheelData;
    SceneNode *pScene;
    SceneNode *pBody;
    SceneNode *pNodes;
    Car *pCar;

    pSlot = NULL;
    slot = 0;
    mode = CGameInfo::FUN_00405d80();
    isPlayer = car < (int)(BYTE)RallyDataState();
    if ((BYTE)RallyDataState() > 1 && CGameInfo::FUN_00405da0() == 0)
        twoPlayers = 1;
    else
        twoPlayers = 0;
    if (mode == 5 || mode == 6 || mode == 7 || mode == 11 || mode == 12)
        rallyMode = 1;
    else
        rallyMode = 0;
    hasVariant = 0;
    knockout = mode == 4;
    variant = 0;
    FUN_00457c50();
    switch (FUN_0041b380()) {
    case 4:
        slot = FUN_0041b370() + car;
        break;
    case 0:
    case 1:
        slot = car;
        break;
    case 2:
    case 3:
        pChamp = RallyData_GetChampionshipState();
        state = *pChamp;
        switch ((state >> 3) & 7) {
        case 1:
            pSlot = pChamp + ((state >> 12) & 0xf) * 3 + 0x16;
            break;
        case 2:
            pSlot = pChamp + ((state >> 12) & 0xf) * 3 + 10;
            break;
        case 3:
            pSlot = pChamp + ((state >> 12) & 0xf) * 3 + 4;
            break;
        case 4:
            pSlot = pChamp + 1;
            break;
        }
        slot = *pSlot;
        if (car != 0)
            slot >>= 5;
        slot &= 0x1f;
        break;
    }
    if (!knockout) {
        if ((mode == 3 || mode == 7) && (char)CGameInfo::FUN_00406310() && car == (int)(FUN_00456ca0() - 1)) {
            i = FUN_004660f0();
            if (i == -1)
                i = 0;
            model = RallyData_FUN_004086b0((BYTE)i);
        } else if ((char)RallyData_FUN_00407e90() && CGameInfo::FUN_00405e00() == 0) {
            if (car < (int)(BYTE)RallyDataState())
                model = RallyData_FUN_004086b0((BYTE)slot);
            else
                model = FUN_00407fc0(StageTiming_FUN_00455ab0(FUN_0041b370()));
        } else if ((char)FUN_00407270()) {
            best = 99;
            n = 0;
            for (i = 0; i < (int)CGameInfo::FUN_00405d70(); i++) {
                if (RallyTiming_GetStagePositionOfDriver(StageTiming_GetDriverSlot(i)) < best) {
                    best = RallyTiming_GetStagePositionOfDriver(StageTiming_GetDriverSlot(i));
                    n = i;
                }
            }
            model = RallyData_FUN_004086b0((BYTE)n);
        } else {
            n = 0;
            if (CGameInfo::FUN_00405e00() && car > 0) {
                slot = mode;
                for (i = 0; i < 7; i++) {
                    if ((char)FUN_00409cb0(i)) {
                        if (n == car - 1) {
                            slot = i;
                            break;
                        }
                        n++;
                    }
                }
                FUN_0040b200(slot);
                model = FUN_00409d00(slot);
                team = (int)CFrontend::FUN_0040ee90(model);
                FUN_0040aff0(slot, car);
                goto classes;
            }
            model = RallyData_FUN_004086b0((BYTE)slot);
        }
        model &= 0xff;
        team = (int)CFrontend::FUN_0040ee90(model);
    } else {
        if (car == 0) {
            team = rand() % 6;
            while (team == g_unk0x0051a8b8)
                team = rand() % 6;
            g_unk0x0051a8b8 = team;
        } else {
            team = g_unk0x0051a8b8;
        }
        model = (unsigned int)CFrontend::FUN_0040eea0(team);
        RallyData_FUN_00408600((BYTE)car, (BYTE)model);
    }
classes:
    switch (mode) {
    case 10:
        letter = g_stageQualityCodes[1];
        lod = 1;
        break;
    case 5:
    case 6:
    case 8:
    case 9:
    case 11:
    case 12:
        if (isPlayer) {
            if (twoPlayers)
                letter = g_stageQualityCodes[0x11];
            else
                letter = g_stageQualityCodes[4];
        } else {
            if (twoPlayers) {
                letter = g_stageQualityCodes[0x12];
                variant = g_stageQualityCodes[0x13];
            } else {
                letter = g_stageQualityCodes[5];
                variant = g_stageQualityCodes[6];
            }
            if (mode == 5 || mode == 6) {
                model = RallyData_FUN_00408010(car);
                team = (int)CFrontend::FUN_0040ee90(model);
            }
            hasVariant = 1;
        }
        lod = letter != 'A' ? 3 : 1;
        break;
    case 3:
    case 7:
        if ((char)CGameInfo::FUN_00406310()) {
            if (car == (int)(FUN_00456ca0() - 1)) {
                if (twoPlayers)
                    letter = g_stageQualityCodes[0xf];
                else
                    letter = g_stageQualityCodes[3];
            } else if (twoPlayers) {
                letter = g_stageQualityCodes[0x10];
            } else {
                letter = g_stageQualityCodes[2];
            }
        } else if (twoPlayers) {
            letter = g_stageQualityCodes[0xe];
        } else {
            letter = g_stageQualityCodes[1];
        }
        lod = letter < 'E' ? 1 : 3;
        break;
    case 4:
        if (isPlayer) {
            if (twoPlayers)
                letter = g_unk0x00542630[0x394];
            else
                letter = g_stageQualityCodes[7];
        } else if (twoPlayers) {
            letter = g_unk0x00542630[0x395];
        } else {
            letter = g_stageQualityCodes[8];
        }
        lod = 1;
        break;
    default:
        if ((char)RallyData_GetFlag25() && CGameInfo::FUN_00405e00() == 0) {
            if (isPlayer) {
                if (twoPlayers)
                    letter = g_stageQualityCodes[0x14];
                else
                    letter = g_stageQualityCodes[9];
            } else if (twoPlayers) {
                letter = g_stageQualityCodes[0x15];
            } else {
                letter = g_stageQualityCodes[10];
            }
        } else if (twoPlayers) {
            letter = g_stageQualityCodes[0xd];
        } else if (CGameInfo::FUN_00405e00()) {
            if (car > 0)
                letter = g_stageQualityCodes[0xc];
            else
                letter = g_stageQualityCodes[0xb];
        } else {
            letter = g_stageQualityCodes[0];
        }
        lod = 1;
        break;
    }
    if (CGameInfo::FUN_00406410(0x10))
        lod = 1;
    pRecord = CAR_RECORD(car);
    pRecord[0x20] = letter;
    teamByte = (BYTE)team;
    g_unk0x0054260c[car] = model;
    letterInt = letter;
    sprintf(CFrontend::m_stringDest, g_strCarModelFormat, FUN_004200d0(model), letterInt, lod);
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CFrontend::m_stringDest, g_strCarC3dExt);
    if (pRecord[0x20] == 'A')
        strncpy(CFrontend::m_stringDest + strlen(CFrontend::m_stringDest) - 6, g_strDamageModelC3d, 8);
    pData = CFileBuffer::GetGenericFileBuffer(CFrontend::m_stringDest, FALSE);
    strcpy(path, CFrontend::m_stringDest);
    *(void **)(g_unk0x00542630 + 0x354 + car * 4) = pData;
    sprintf(CFrontend::m_stringDest, g_strCinFormat, FUN_004200d0(model));
    *(void **)(g_unk0x00542630 + 0x398 + car * 4) = CFileBuffer::GetGenericFileBuffer(CFrontend::m_stringDest, FALSE);
    if ((char)RallyData_GetFlag24())
        FUN_004a3dc0(1);
    else
        FUN_004a3dc0(0);
    sprintf(CFrontend::m_stringDest, g_strCarBflFormat, FUN_004200d0(model), letterInt, lod);
    pArchive = &g_unk0x00542ae8[car * 2];
    CGenericFileLoader::FUN_004a9d70((GenericFile *)pArchive, CFrontend::m_stringDest);
    if (pData == NULL)
        return 0;
    pScene = (SceneNode *)FUN_004b9380((unsigned int)pData, RallyData_FUN_00411060(), (unsigned int)pArchive);
    pBody = SceneNode_FindByType(pScene, 5);
    *(SceneNode **)(pRecord + 8) = pScene;
    *(SceneNode **)(pRecord + 4) = pBody;
    pRecord[0] = (BYTE)team;
    pRecord[0x20] = letter;
    if (((char)RallyData_GetFlag25() && CGameInfo::FUN_00405e00() == 0 && twoPlayers == 0 && car == 1) ||
        (char)FUN_00407270() || CGameInfo::FUN_00405d80() == 4)
        teamByte = 0;
    FUN_00457e50(pScene, pBody, car, teamByte, (FixAngles *)FUN_00456c90(car), (FixVector *)FUN_00456c70(car), 0);
    pWheelData = NULL;
    if (pRecord[0x20] == 'A') {
        if (FUN_0042cae0(Car_Get(car), 1)) {
            strncpy(path + strlen(path) - 5, g_strSnowWheelsC3d, 5);
            pWheelData = CFileBuffer::GetGenericFileBuffer(path, FALSE);
        }
        if (FUN_0042cb50(Car_Get(car))) {
            strncpy(path + strlen(path) - 5, g_strLightWheelsC3d, 5);
            pWheelData = CFileBuffer::GetGenericFileBuffer(path, FALSE);
        }
        if (pWheelData != NULL) {
            *(int *)(pRecord + 0xc) = FUN_004b9380((unsigned int)pWheelData, RallyData_FUN_00411060(),
                                                   (unsigned int)&g_unk0x00542ae8[car * 2]);
            *(int *)(pRecord + 0x10) = MESH_OF(SceneNode_FindByType(pScene, 1));
            *(int *)(pRecord + 0x14) = MESH_OF(SceneNode_FindByType(pScene, 2));
            *(int *)(pRecord + 0x18) = MESH_OF(SceneNode_FindByType(pScene, 3));
            *(int *)(pRecord + 0x1c) = MESH_OF(SceneNode_FindByType(pScene, 4));
            MESH_OF(SceneNode_FindByType(pScene, 1)) = MESH_OF(SceneNode_FindByType(*(SceneNode **)(pRecord + 0xc), 1));
            MESH_OF(SceneNode_FindByType(pScene, 2)) = MESH_OF(SceneNode_FindByType(*(SceneNode **)(pRecord + 0xc), 2));
            MESH_OF(SceneNode_FindByType(pScene, 3)) = MESH_OF(SceneNode_FindByType(*(SceneNode **)(pRecord + 0xc), 3));
            MESH_OF(SceneNode_FindByType(pScene, 4)) = MESH_OF(SceneNode_FindByType(*(SceneNode **)(pRecord + 0xc), 4));
            goto wheelsDone;
        }
    }
    *(int *)(pRecord + 0xc) = 0;
    *(int *)(pRecord + 0x10) = 0;
    *(int *)(pRecord + 0x14) = 0;
    *(int *)(pRecord + 0x18) = 0;
    *(int *)(pRecord + 0x1c) = 0;
wheelsDone:
    *(void **)(g_unk0x00542630 + 0x2d4 + car * 4) = pWheelData;
    if (CGameInfo::FUN_00405ba0()) {
        if (pRecord[0x20] == 'A') {
            Scene_AddShadowCaster(*(SceneNode **)(pRecord + 4), 1);
            Scene_AddShadowCaster(*(SceneNode **)(pRecord + 8), 1);
        } else {
            Scene_AddShadowCaster(*(SceneNode **)(pRecord + 4), 0);
            Scene_AddShadowCaster(*(SceneNode **)(pRecord + 8), 0);
        }
    }
    SceneNode_SetMeshFlagBits(*(SceneNode **)(pRecord + 4), car);
    SceneNode_SetMeshFlagBits(*(SceneNode **)(pRecord + 8), car);
    if (rallyMode || ((char)RallyData_FUN_00407ea0() && (char)CGameInfo::FUN_00406310())) {
        if ((char)RallyData_FUN_00407ea0())
            CGameInfo::FUN_00406310();
    } else {
        FUN_0046b400(1, car);
    }
    if (car < (int)(BYTE)RallyDataState() &&
        ((CGameInfo::FUN_00405d80() != 0 && CGameInfo::FUN_00405d80() != 1) || RallyDataStageIndex() == 0 ||
         RallyDataStageIndex() == 4 || RallyDataStageIndex() == 8 || RallyDataStageIndex() == 10)) {
        FUN_0045c6b0(car, 0);
        FUN_0045c6b0(car, 1);
        FUN_0045c6b0(car, 2);
        FUN_0045c6b0(car, 3);
    }
    if ((char)RallyData_GetFlag24() == 0 && (letter == 'C' || letter == 'A')) {
        sprintf(CFrontend::m_stringDest, g_strCarModelFormat, FUN_004200d0(model), letterInt, lod);
        FUN_00458050((int)CFrontend::m_stringDest, car, teamByte);
    }
    if (hasVariant == 0 || CGameInfo::FUN_00406410(0x10)) {
        pCar = Car_Get(car);
        *(int *)((BYTE *)pCar + 0x724) = 0;
        *(int *)((BYTE *)pCar + 0x728) = 0;
        *(int *)((BYTE *)pCar + 0x72c) = 0;
        *(int *)((BYTE *)pCar + 0x730) = 0;
        *(int *)((BYTE *)pCar + 0x734) = 0;
        return 1;
    }
    pRecord[0x21] = variant;
    sprintf(CFrontend::m_stringDest, g_strCarModelFormat, FUN_004200d0(model), (int)variant, lod);
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CFrontend::m_stringDest, g_strCarC3dExt);
    pData = CFileBuffer::GetGenericFileBuffer(CFrontend::m_stringDest, FALSE);
    if (pData == NULL)
        return 0;
    *(void **)(g_unk0x00542630 + 0x314 + car * 4) = pData;
    CFrontend::m_stringDest[strlen(CFrontend::m_stringDest) - strlen(g_strCarC3dExt)] = 0;
    sprintf(CFrontend::m_stringDest, g_strBflFormat, CFrontend::m_stringDest);
    pArchive = &g_unk0x00542ae8[car * 2 + 1];
    CGenericFileLoader::FUN_004a9d70((GenericFile *)pArchive, CFrontend::m_stringDest);
    pCar = Car_Get(car);
    pNodes = (SceneNode *)FUN_004b9380((unsigned int)pData, RallyData_FUN_00411060(), (unsigned int)pArchive);
    *(SceneNode **)(g_unk0x00542630 + 0x294 + car * 4) = pNodes;
    *(SceneNode **)((BYTE *)pCar + 0x724) = SceneNode_FindByType(pNodes, 5);
    *(SceneNode **)((BYTE *)pCar + 0x728) = SceneNode_FindByType(pNodes, 1);
    *(SceneNode **)((BYTE *)pCar + 0x72c) = SceneNode_FindByType(pNodes, 2);
    *(SceneNode **)((BYTE *)pCar + 0x730) = SceneNode_FindByType(pNodes, 3);
    *(SceneNode **)((BYTE *)pCar + 0x734) = SceneNode_FindByType(pNodes, 4);
    if (CGameInfo::FUN_00405ba0())
        Scene_AddShadowCaster(*(SceneNode **)((BYTE *)pCar + 0x724), 0);
    SceneNode_SetMeshFlagBits(*(SceneNode **)((BYTE *)pCar + 0x724), car);
    for (i = 0x724; i <= 0x734; i += 4) {
        if (*(SceneNode **)((BYTE *)pCar + i) != NULL)
            SceneNode_Reparent(*(SceneNode **)((BYTE *)pCar + i), *(SceneNode **)((BYTE *)pCar + 0x71c));
    }
    FUN_0042cb90(pRecord[0x21], (SceneNode **)((BYTE *)pCar + 0x728));
    return 1;
}
#undef CAR_RECORD
#undef MESH_OF

void FUN_00480a60(void);
void FUN_00480a50(void);
void FUN_00480b40(BYTE *pCar);
void FUN_00466e90(SceneNode *pNode, int *pSlot);
void FUN_00480af0(BYTE *pCar, BYTE *pObject, BYTE flag);
void FUN_0046acb0(int param_1, int param_2, int param_3);
void FUN_00469690(int param_1);

// Sets up the damage parts of the cars in `pOrder` (last first): collects the
// mesh parts of each car model into its part table (0x4d0 bytes, 15 slots),
// closes the gaps, resets the per-part damage state and, for the players'
// cars, restores the saved damage record (`keep` = 0 also refreshes the saved
// copy).
// FUNCTION: CMR2 0x004669f0
void FUN_004669f0(int lock, int keep, short *pOrder, short count)
{
    int n;
    short *pIndex;
    int car;
    BYTE *pCar;
    int *pParts;
    BYTE *pDamage;
    SceneNode *pChild;
    SceneNode *pNode;
    int j;
    int k;
    int moved;
    int i;
    BYTE *pSaved;
    BYTE *p;

    FUN_00480a60();
    if (lock == 0)
        FUN_00480a50();
    n = count;
    if (n - 1 >= 0) {
        pIndex = pOrder + (n - 1);
        do {
            car = *pIndex;
            pCar = (BYTE *)Car_Get(car);
            FUN_00480b40(pCar);
            pParts = (int *)(g_unk0x00588b94 + car * 0x4d0);
            pParts[0x117] = 0;
            pDamage = g_unk0x00588b98 + car * 0x290;
            if (*(SceneNode **)(pCar + 0x720) != NULL) {
                pParts[0x107] = 0;
                pParts[0x106] = 0;
                pParts[0x105] = 0;
                pParts[0x104] = 0;
                if ((BYTE)(*(SceneNode **)(pCar + 0x720))->flags != 0x14)
                    FUN_00466e90(*(SceneNode **)(pCar + 0x720), pParts);
                for (pChild = (*(SceneNode **)(pCar + 0x720))->pFirstChild; pChild != NULL; pChild = pChild->pNext) {
                    for (pNode = pChild; pNode != NULL && (BYTE)pNode->flags != 0x14;
                         pNode = pNode->pFirstChild)
                        FUN_00466e90(pNode, pParts);
                }
                for (j = 0; j < 15; j++) {
                    if (pParts[0xf + j] != 0 || j >= 14)
                        continue;
                    moved = 0;
                    for (k = j; k < 14; k++) {
                        if (pParts[0xf + k] != 0 || pParts[0xf + k + 1] != 0)
                            moved = 1;
                        pParts[k] = pParts[k + 1];
                        pParts[0xf + k] = pParts[0xf + k + 1];
                        pParts[0x1e + k] = pParts[0x1e + k + 1];
                        pParts[0x108 + k] = pParts[0x108 + k + 1];
                        ((FixVector *)(pParts + 0x2d))[k] = ((FixVector *)(pParts + 0x2d))[k + 1];
                        ((FixVector *)(pParts + 0x5a))[k] = ((FixVector *)(pParts + 0x5a))[k + 1];
                    }
                    if (moved)
                        j--;
                }
                for (i = 0; i < pParts[0x117]; i++)
                    FUN_00480af0(pCar, (BYTE *)pParts[0xf + i], (BYTE)i);
                *((BYTE *)pParts + 0x460) = 4;
                *((BYTE *)pParts + 0x461) = 5;
                *((BYTE *)pParts + 0x462) = 1;
                *((BYTE *)pParts + 0x463) = 2;
                *((BYTE *)pParts + 0x464) = 8;
                *((BYTE *)pParts + 0x465) = 9;
                memset(pParts + 0x87, 0, 9 * 4);
                *((BYTE *)pParts + 0x469) = 0;
                pParts[0x11b] = 0;
                for (i = 0; i < 0x22; i++) {
                    pParts[0xd4 + i - 0x44] = 0;
                    pParts[0xd4 + i] = 0;
                }
                for (i = 0; i < 8; i++) {
                    pParts[0x11c + i + 8] = 0;
                    pParts[0x11c + i] = 0;
                }
                pParts[0xb2] = 0x4ccc;
                pParts[0xb3] = 0x4ccc;
                pParts[0xb4] = 0x9999;
                pParts[0xb5] = 0x9999;
                pParts[0xb6] = 0x4ccc;
                pParts[0xb7] = 0x4ccc;
                pParts[0xb8] = 0x4ccc;
                pParts[0xb9] = 0x4ccc;
                pParts[0xba] = 0x9999;
                pParts[0xbb] = 0x9999;
                pParts[0xbc] = 0x4ccc;
                pParts[0xbd] = 0x4ccc;
                pParts[0xbe] = 0x9999;
                pParts[0xbf] = 0x9999;
                pParts[0xc0] = 0x9999;
                pParts[0xc1] = 0x4ccc;
                pParts[0xc2] = 0x9999;
                pParts[0xc3] = 0x9999;
                pParts[0xc4] = 0x10000;
                pParts[0xc5] = 0x10000;
                pParts[0xc6] = 0x10000;
                pParts[0xc7] = 0x10000;
                pParts[0xc8] = 0x10000;
                pParts[0xc9] = 0x6666;
                pParts[0xca] = 0x9999;
                pParts[0xcb] = 0x4ccc;
                pParts[0xcc] = 0x10000;
                pParts[0xcd] = 0x10000;
                pParts[0xce] = 0x10000;
                pParts[0xcf] = 0x10000;
                pParts[0xd0] = 0x10000;
                pParts[0xd1] = 0x10000;
                pParts[0xd2] = 0x10000;
                pParts[0xd3] = 0x10000;
                if (keep == 0)
                    *(int *)(pDamage + 0x28c) = 0;
                if ((int)(signed char)pCar[0xb1a] < (int)(BYTE)RallyDataState()) {
                    pSaved = (BYTE *)RallyData_FUN_00407610(FUN_0041b370() + (signed char)pCar[0xb1a]);
                    if (keep == 0) {
                        memcpy(pDamage + 0x106, pSaved, 0x106);
                        memcpy(pDamage + 0x24c, pSaved + 0x108, 0x40);
                    }
                    memcpy(pDamage, pSaved, 0x106);
                    memcpy(pDamage + 0x20c, pSaved + 0x108, 0x40);
                    if (pDamage[0x104] == 0) {
                        p = pDamage + 0xc;
                        for (i = 0x14; i != 0; i--) {
                            if (keep == 0)
                                p[0x106] = 0xff;
                            p[0] = 0xff;
                            p += 0xd;
                        }
                    }
                }
                FUN_0046acb0((signed char)pCar[0xb1a], *(int *)(pCar + 0x720), (int)pParts);
                FUN_00469690((int)pCar);
            }
            pIndex--;
        } while (--n);
    }
    if (lock != 0)
        FUN_00480a50();
}

extern int g_stageLighting[];
// Lightning: the sector lit by the current flash (0x547ac0) and the previous
// frame's value (0x547ac2), inside the lighting block of StageObjects.cpp.
#define g_unk0x00547ac0 (*(short *)&g_stageLighting[0x5c])
#define g_unk0x00547ac2 (*((short *)&g_stageLighting[0x5c] + 1))
#define RAND_FIX() ((int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536))

int FUN_00492910(void);
void FUN_00492b50(void);
void FUN_0045f530(BYTE *pObject, int view);
void FUN_0045e8b0(unsigned int *pRecord, int view);
void FUN_0045f5d0(int pData, int param_2);
void FUN_0045f9d0(int param_1, int *param_2, int param_3);
void FUN_00460b60(int *p, int unused);

// Wind: eases the strength towards its target, then after a random pause
// picks a new target and rate (halved when a view has snow).
// FUNCTION: CMR2 0x0045f6d0
void FUN_0045f6d0(void)
{
    int snow;
    unsigned int i;

    if (g_unk0x00547944.z != 0) {
        g_unk0x00547944.z -= g_unk0x0051bd3c;
        if (g_unk0x00547944.z < 0)
            g_unk0x00547944.z = 0;
        return;
    }
    if (g_unk0x00547940 != g_unk0x00547944.x) {
        if (g_unk0x00547940 > g_unk0x00547944.x) {
            g_unk0x00547940 -= FixMul(g_unk0x00547944.y, g_unk0x0051bd3c);
            if (g_unk0x00547940 < g_unk0x00547944.x) {
                g_unk0x00547940 = g_unk0x00547944.x;
                goto change;
            }
        } else if (g_unk0x00547940 < g_unk0x00547944.x) {
            g_unk0x00547940 += FixMul(g_unk0x00547944.y, g_unk0x0051bd3c);
            if (g_unk0x00547940 > g_unk0x00547944.x) {
                g_unk0x00547940 = g_unk0x00547944.x;
                goto change;
            }
        }
        if (g_unk0x00547940 != g_unk0x00547944.x)
            return;
    }
change:
    g_unk0x00547944.z = FixMul(0x4b0000, RAND_FIX()) + 0x190000;
    g_unk0x00547944.x = FixMul(0x10000, RAND_FIX());
    g_unk0x00547944.y = FixMul(0x667, RAND_FIX()) + 0x147;
    g_unk0x0054793c = g_unk0x00547940;
    snow = 0;
    for (i = 0; i < g_unk0x00543e98; i++) {
        if (*(int *)((BYTE *)g_unk0x00547ac8 + i * 0x178) == 2) {
            snow = 1;
            break;
        }
    }
    if (snow) {
        g_unk0x00547944.x = FixMul(g_unk0x00547944.x, 0x8000);
        g_unk0x00547944.y = FixMul(g_unk0x00547944.y, 0x8000);
    }
}

// Lightning: counts down to the next flash (picking the sector it lights), and
// every frame lights that sector at random while it rains hard.
// FUNCTION: CMR2 0x0045f890
void FUN_0045f890(void)
{
    int *pWeather;

    pWeather = (int *)g_unk0x00547ac8;
    if (g_unk0x00543ec0 != 0) {
        g_unk0x00543ec0 -= g_unk0x0051bd3c;
        if (g_unk0x00543ec0 < 0) {
            g_unk0x00543ec8 = (short)FUN_00492910();
            FUN_00492b50();
            g_unk0x00543ec0 = 0;
        }
    } else {
        g_unk0x00543ec4 -= g_unk0x0051bd3c;
        if (g_unk0x00543ec4 < 0) {
            g_unk0x00543ec0 = FixMul(RAND_FIX(), 0x1770000) + 0x7d0000;
            g_unk0x00543ec8 = -1;
            g_unk0x00543ec4 = FixMul(RAND_FIX(), 0x140000) + 0x140000;
        }
    }
    g_unk0x00547ac2 = g_unk0x00547ac0;
    if (RAND_FIX() < 0x4ccc && pWeather[0x15] > 0xcccc && pWeather[0] == 1)
        g_unk0x00547ac0 = g_unk0x00543ec8;
    else
        g_unk0x00547ac0 = -1;
}

// Per-frame weather of every view: wind, lightning, then each view's
// precipitation, particles and sky.
// FUNCTION: CMR2 0x0045f4a0
void FUN_0045f4a0(void)
{
    int view;
    int *pWeather;
    int offset;

    FUN_0045f6d0();
    FUN_0045f890();
    for (view = 0, offset = 0; view < (int)g_unk0x00543e98; view++, offset += 0x2c) {
        pWeather = (int *)((BYTE *)g_unk0x00547ac8 + view * 0x178);
        FUN_0045f530((BYTE *)pWeather, view);
        FUN_0045e8b0((unsigned int *)((BYTE *)g_unk0x00543eb8 + offset), view);
        FUN_0045f5d0((int)((BYTE *)g_unk0x00543eb8 + offset), view);
        if (pWeather[0] == 1 || pWeather[0] == 2)
            FUN_0045f9d0(1, pWeather, view);
        FUN_00460b60(pWeather, view);
    }
}
#undef RAND_FIX

extern char g_strDemoName0x00519ec4[4];
void FUN_0040a3e0(unsigned int time);
void FUN_0040a330(unsigned int time, int stage);
void FUN_004cf530(int index, int value);
BYTE FUN_004cf830(int index);
BYTE FUN_004cfe80(int param_1, int param_2);
int FUN_004cfe20(int param_1, int param_2);
int FUN_004481c0(int car);
unsigned int RallyData_FUN_004082c0(void);

// A car crosses a split line: stores its split time (from the start or the
// previous split), reports it to the network and the record tables and runs
// the head-to-head rules.
// FUNCTION: CMR2 0x00448920
void FUN_00448920(int car)
{
    int group;
    int *pRecord;
    int *pPrev;
    int *pTime;
    int *pOut;
    int base;
    int i;
    int slot;
    BYTE bits;

    group = FUN_00458330(car);
    if (FUN_00458290(car) == 0)
        return;
    if ((char)RallyData_FUN_00407e90() == 0) {
        pRecord = g_unk0x0053d1e8[car][group];
        pRecord[0] = g_unk0x0053d1b0;
        if ((char)RallyData_FUN_00407ea0() == 0 && CGameInfo::FUN_00405d80() != 12) {
            pPrev = pRecord - 5;
            base = pPrev[0];
            pTime = &g_carStageTiming[car].splitTimes[group];
            *pTime = pRecord[0] - base;
            pOut = g_carStageTiming[car].splits;
            for (i = 5; i != 0; i--)
                *pOut++ = *pPrev++ - base;
        } else {
            pTime = &g_carStageTiming[car].splitTimes[group];
            *pTime = g_unk0x0053d1b0 - g_carStageTiming[car].lastTime;
            if (*pTime > 359999)
                *pTime = 359999;
            g_carStageTiming[car].splits[0] = 0;
            pOut = &g_carStageTiming[car].splits[1];
            pPrev = pRecord + 1;
            for (i = 4; i != 0; i--) {
                *pOut = *pPrev - g_carStageTiming[car].lastTime;
                if (*pPrev - g_carStageTiming[car].lastTime > 359999)
                    *pOut = 359999;
                pPrev++;
                pOut++;
            }
        }
        if (CGameInfo::FUN_00405d80() == 12) {
            FUN_0040a3e0(*pTime);
            FUN_0040a330(*pTime, 0);
        }
    }
    if (car < (int)(BYTE)RallyDataState() && (char)CGameInfo::FUN_00406430() == 0 &&
        (char)CGameInfo::FUN_00406440() == 0 && g_unk0x0053d1d8 == 0 &&
        strncmp((char *)RallyData_GetRecord(0), g_strDemoName0x00519ec4, 3) != 0) {
        slot = FUN_0041b370() + car;
        if ((char)RallyData_GetFlag24()) {
            FUN_004cf530(slot, g_carStageTiming[car].splitTimes[group]);
            g_unk0x0053e18d[car] |= FUN_004cf830(slot);
            bits = FUN_004cfe80(car, slot);
            g_unk0x0053e18f |= bits & 1;
            g_carStageTiming[car].field_0x84 = bits & 1;
            g_carStageTiming[car].pad_0x85[0] = (BYTE)FUN_004cfe20(car, slot) & 1;
        }
    }
    if ((char)RallyData_FUN_004082e0()) {
        if (RallyData_FUN_004082b0() == 2) {
            if (FUN_004481c0(car) == 0)
                g_unk0x0053d1a4[car]++;
            if (g_unk0x0053d1a4[car] == (int)RallyData_FUN_004082c0()) {
                g_unk0x0053d1a7 = 1;
                StageTiming_QueueDriverSlot(car);
                StageTiming_QueueDriverSlot(1 - car);
            }
        }
        if (RallyData_FUN_004082b0() == 1) {
            if (g_unk0x0053d1a6 == 0) {
                if (FUN_004481c0(car) == 0) {
                    g_unk0x0053d1a6 = 1;
                    g_unk0x0053d1a0 = 0;
                    g_unk0x0053d1a2 = 0;
                    g_unk0x0053d1a8 = (char)car;
                }
            } else {
                g_unk0x0053d1a6 = 0;
            }
        }
    }
    g_carStageTiming[car].lastTime = g_unk0x0053d1b0;
}

// Timing of one car for the frame: start, split and finish line crossings.
// FUNCTION: CMR2 0x00448730
void FUN_00448730(int car)
{
    if (FUN_00458230(car)) {
        if (FUN_00458250(car))
            FUN_004487a0(car);
        if (FUN_00458270(car))
            FUN_00448920(car);
        if (FUN_004582b0(car))
            StageTiming_QueueDriverSlot(car);
    }
}

void FUN_00448de0(void);
void FUN_00448780(int car);
void FUN_00448d50(void);
void FUN_00448bf0(int slot);

// Per-frame stage timing of every car: in the championship/arcade modes the
// start of each car's timing, then each car's line crossings.
// FUNCTION: CMR2 0x00448120
void FUN_00448120(void)
{
    int count;
    int i;
    BYTE *p;

    count = FUN_00458390();
    FUN_00448de0();
    if ((char)RallyData_GetFlag24() || (char)RallyData_GetFlag25()) {
        for (i = count, p = &g_carStageTiming[0].field_0x83; i > 0; i--, p += sizeof(CarStageTiming))
            *p = 0;
        for (i = 0; i < count; i++) {
            if (g_unk0x0053d1da[i] == 0 && FUN_00458230(i))
                FUN_00448780(i);
        }
        FUN_00448d50();
    }
    for (i = 0; i < count; i++) {
        if (g_unk0x0053d1da[i] == 0) {
            FUN_00448730(i);
            if ((char)RallyData_FUN_004082e0())
                FUN_00448bf0(i);
        }
    }
}

int GetStageSplitCount(void);

// Adds a random error to the CPU drivers' split deltas (16 drivers, a row per
// split), scaled by the difficulty; outside the head-to-head modes the error
// is kept inside a band that narrows along the stage.
// FUNCTION: CMR2 0x00456710
void FUN_00456710(int *pDeltas)
{
    int unused[9];
    int limits[10];
    int splits;
    int scale;
    int top;
    int amplitude;
    int step;
    int column;
    int *p;
    int row;
    int sum;
    int total;
    int value;
    int range;
    int error;
    int mid;
    int k;

    splits = GetStageSplitCount();
    switch (CGameInfo::FUN_00405d90()) {
    case 0:
        scale = 0x19999;
        break;
    case 1:
        scale = 0x10000;
        break;
    case 2:
        scale = 0xa8f5;
        break;
    }
    if ((char)RallyData_GetFlag25() == 0) {
        top = FixMul(scale, 0x30000);
        limits[splits] = scale;
        amplitude = top;
        limits[1] = top;
        if (splits > 2) {
            step = (int)(__int64)((double)(splits - 1) * CGraphics::m_65536);
            for (k = 2; k < splits; k++)
                limits[k] = limits[1] - FixMul((int)(__int64)((double)(k - 1) * CGraphics::m_65536),
                                               FixDiv(limits[1] - scale, step));
        }
    } else {
        amplitude = FixMul(scale, 0x8000);
    }
    for (column = 16, p = pDeltas; column != 0; column--, pDeltas++) {
        p = pDeltas;
        sum = 0;
        total = 0;
        for (row = 0; row < splits; row++, p += 16) {
            value = *p;
            total += value;
            sum += value;
            range = FixMul(FixMul(amplitude, FixDiv(value, 0x640000)), 0x20000);
            error = FixMul(FixDiv((int)(__int64)((double)rand() * CGraphics::m_65536), 0x7fff0000), range) -
                    FixDiv(range, 0x20000);
            if ((char)RallyData_GetFlag25() == 0) {
                mid = FixMul(FixDiv(limits[row + 1], 0x640000), total);
                unused[row] = mid;
                if (error - mid - total + sum > 0)
                    error -= error - mid - total + sum;
                if (mid - total + error + sum < 0)
                    error -= mid - total + error + sum;
            }
            *p += error;
            sum += error;
        }
    }
}

void FUN_00456330(int *param_1);
void FUN_004564d0(int pCar, int *pOut);
void FUN_00456960(int *pDeltas);
void RallyTiming_SortOrder(int *piTimes, char *pcOrder, int iDirection, int iCount, char bInitialise);

// Builds the CPU drivers' split times for a stage and sorts every split.
// FUNCTION: CMR2 0x004561e0
void FUN_004561e0(int *pTimes)
{
    int *pDeltas;
    int n;
    int *pSplit;
    char *pOrder;

    pDeltas = (int *)CFileBuffer::AllocateLockedBuffer(0x280);
    FUN_00456330(pTimes);
    FUN_004564d0((int)pTimes, pDeltas);
    FUN_00456710(pDeltas);
    FUN_00456960(pDeltas);
    n = GetStageSplitCount();
    if (n >= 0) {
        pSplit = g_stageSplitTimesRaw[0];
        pOrder = g_stageSplitTimesRawDriverIx[0];
        for (n = n + 1; n != 0; n--) {
            RallyTiming_SortOrder(pSplit, pOrder, 1, g_unk0x00541f98, 1);
            pOrder += 0x10;
            pSplit += 0x10;
        }
    }
    CFileBuffer::FreeGenericFileBuffer(pDeltas);
}

// Where exhaust sparks leave the car body: pairs of end points of a segment,
// in car space; a random point on one of them is picked.
// GLOBAL: CMR2 0x0051ab68
FixVector g_exhaustSparkLines[4][2] = {
    { { 0x15d2f, 0x25e3, -0x3db2 }, { 0x15d2f, 0x25e3, 0x3f7c } },
    { { 0x18a7e, 0x17ce, 0x5c28 }, { 0x17a9f, 0x1604, 0x9439 } },
    { { 0x19e76, 0x147a, 0x5be7 }, { 0x19168, 0x1333, 0x9333 } },
    { { 0x18a7e, 0x17ce, -0x5c28 }, { 0x17a9f, 0x1604, -0x9439 } },
};
// Frames left of each car's backfire flash.
// GLOBAL: CMR2 0x005439b0
BYTE g_unk0x005439b0[8];

extern FixVector g_unk0x00543380[8];
void Particle_Spawn(int typeIndex, FixVector *pSource, FixVector *pPosition, int field0x48, int field0x40,
                    BYTE *pColour, BYTE field0x54, int callbackParam, BYTE field0x55);
int FixMatrix_RotateVector(FixVector *pOut, FixVector *pV, FixMatrix *pM);
void FUN_004ae3f0(BYTE *p, int value);
void FUN_004ae3d0(BYTE *p, BYTE value);

#define CAR_INT(p, off) (*(int *)((BYTE *)(p) + (off)))

// Exhaust effects of a car: smoke from each exhaust while the engine revs at
// low speed, sparks under hard acceleration, and the backfire flash.
// FUNCTION: CMR2 0x0045af90
void FUN_0045af90(int car)
{
    Car *pCar;
    FixVector velocity;
    FixVector carVelocity;
    FixVector spark;
    FixVector position;
    FixVector zero;
    int revs;
    int speed;
    int heat;
    int exhaust;
    int negRevs;
    int t;
    int r;
    int line;
    int big;

    if (car >= 8 || g_trailTextureA[car][0] == 0)
        return;
    pCar = Car_Get(car);
    revs = FixMul(0x28f, CAR_INT(pCar, 0x7a4));
    if (revs < 0)
        revs = -revs;
    speed = (CAR_INT(pCar, 0x408) < 0 ? -CAR_INT(pCar, 0x408) : CAR_INT(pCar, 0x408)) +
            (CAR_INT(pCar, 0x410) < 0 ? -CAR_INT(pCar, 0x410) : CAR_INT(pCar, 0x410));
    heat = 0x17c - (FixMul(0x15e0000, *(int *)((BYTE *)FUN_00469680(pCar->field_0xb1a) + 0x404)) >> 16);
    if (heat > 0xff)
        heat = 0xff;
    else if (heat < 0)
        heat = 0;
    for (exhaust = 0; exhaust < 2; exhaust++) {
        if (g_trailTextureA[car][exhaust] == 0)
            continue;
        if (revs > 0xccc && speed < 0x28f) {
            FixMatrix_RotateVector(&g_unk0x00543380[car], (FixVector *)g_trailTextureA[car][exhaust],
                                   *(FixMatrix **)((BYTE *)pCar + 0x750));
            g_unk0x00543380[car].x += CAR_INT(*(BYTE **)((BYTE *)pCar + 0x754), 0x30);
            g_unk0x00543380[car].y += CAR_INT(*(BYTE **)((BYTE *)pCar + 0x754), 0x34);
            g_unk0x00543380[car].z += CAR_INT(*(BYTE **)((BYTE *)pCar + 0x754), 0x38);
            carVelocity = *(FixVector *)((BYTE *)pCar + 0x450);
            big = heat > 0x50;
            negRevs = -revs;
            FixVecScale(&velocity, (FixVector *)((BYTE *)pCar + 0x360), negRevs);
            velocity.y += carVelocity.y;
            velocity.x += carVelocity.x;
            velocity.z += carVelocity.z;
            t = FixMul((int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536), 0x3d7);
            Particle_Spawn(big, &g_unk0x00543380[car], &velocity, g_unk0x00543380[car].y - 0x10000, 0, NULL, 0, 0,
                           *(BYTE *)(*(BYTE **)((BYTE *)pCar + 0x720) + 0x17c));
        }
        if (g_trailForced[car] != 0)
            g_unk0x005439b0[car] = (BYTE)g_trailForcedSurface[car];
        if (heat > 0x50 && speed < 0x5cccc) {
            heat -= rand() % 20;
            r = (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536);
            line = rand() % 4;
            spark.x = g_exhaustSparkLines[line][1].x - g_exhaustSparkLines[line][0].x;
            spark.y = g_exhaustSparkLines[line][1].y - g_exhaustSparkLines[line][0].y;
            spark.z = g_exhaustSparkLines[line][1].z - g_exhaustSparkLines[line][0].z;
            FixVecScale(&spark, &spark, r);
            spark.x += g_exhaustSparkLines[line][0].x;
            spark.y += g_exhaustSparkLines[line][0].y;
            spark.z += g_exhaustSparkLines[line][0].z;
            FixMatrix_RotateVector(&position, &spark, *(FixMatrix **)((BYTE *)pCar + 0x750));
            revs = 0x1999;
            position.x += CAR_INT(*(BYTE **)((BYTE *)pCar + 0x754), 0x30);
            position.y += CAR_INT(*(BYTE **)((BYTE *)pCar + 0x754), 0x34) - 0x3333;
            position.z += CAR_INT(*(BYTE **)((BYTE *)pCar + 0x754), 0x38);
            FixVecScale(&velocity, (FixVector *)((BYTE *)pCar + 0x36c), revs);
            t = FixMul((int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536), 0x3d7);
            if (speed > 0x8000)
                Particle_Spawn(2, &position, &velocity, position.y - 0x10000, 0, NULL, 0, 0,
                               *(BYTE *)(*(BYTE **)((BYTE *)pCar + 0x720) + 0x17c));
            else
                Particle_Spawn(2, &position, &velocity, position.y - 0x10000, 0, NULL, 0, 0,
                               *(BYTE *)(*(BYTE **)((BYTE *)pCar + 0x720) + 0x17c));
        }
    }
    if (g_unk0x005439b0[car] == 0)
        return;
    g_unk0x005439b0[car]--;
    for (exhaust = 0; exhaust < 2; exhaust++) {
        if (g_trailTextureA[car][exhaust] == 0)
            continue;
        FixMatrix_RotateVector(&g_unk0x00543380[car], (FixVector *)g_trailTextureA[car][exhaust],
                               *(FixMatrix **)((BYTE *)pCar + 0x750));
        FixVecScale(&velocity, (FixVector *)((BYTE *)pCar + 0x360), -revs);
        zero.x = 0;
        zero.y = 0;
        zero.z = 0;
        g_trailForced[car] = 0;
        Particle_Spawn(3, &zero, &velocity, -0x640000, 0, NULL, 0, (int)&car,
                       *(BYTE *)(*(BYTE **)((BYTE *)pCar + 0x720) + 0x17c));
        r = (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536);
        if (g_trailTextureB[car][exhaust] != 0) {
            FUN_004ae3f0((BYTE *)g_trailTextureB[car][exhaust], r);
            FUN_004ae3d0((BYTE *)g_trailTextureB[car][exhaust], 1);
        }
    }
}
#undef CAR_INT

int *FUN_00463270(int i, int j);
void FUN_0045d1e0(int player, BYTE *pColour);
void FUN_0045d540(int car);
void StageTiming_SpawnWheelParticles(int carIndex);
void WheelSplash_Update(int player);
void WheelSpray_Update(int player);

// Per-frame effects of a car (not while replaying a stored run): the wheel
// trails in the colour of the stage light, dust, particles, splashes, spray
// and the exhaust.
// FUNCTION: CMR2 0x0045af00
void FUN_0045af00(int car)
{
    BYTE *pColour;
    int index = car;

    if (index >= 8)
        return;
    if (*(int *)((BYTE *)Car_Get(index) + 0xc0c) != 0 || CGameInfo::FUN_00405cd0() == 2)
        return;
    pColour = (BYTE *)FUN_00463270(index, 0);
    if (pColour != NULL) {
        ((BYTE *)&car)[0] = pColour[0];
        ((BYTE *)&car)[1] = pColour[1];
        ((BYTE *)&car)[2] = pColour[2];
        ((BYTE *)&car)[3] = 0xff;
    }
    FUN_0045d1e0(index, (BYTE *)&car);
    if (FUN_00422f50(index) != 3) {
        FUN_0045d540(index);
        StageTiming_SpawnWheelParticles(index);
        WheelSplash_Update(index);
        WheelSpray_Update(index);
    }
    FUN_0045af90(index);
}

void FUN_00484e00(int param_1, short count);
void Car_BreakQueuedWindows(Car *pCar);
void FUN_00480cb0(void);
void FUN_00480de0(void);

// Per-frame update of the attached body parts (bonnet, doors, bumpers...) of
// the cars in `pOrder` (last first): each active part's world transform is
// rebuilt and its per-part handler runs; then queued windows break.
// FUNCTION: CMR2 0x00480bb0
void FUN_00480bb0(BYTE *pCars, short *pOrder, short count)
{
    int n;
    short *pIndex;
    void **pTable;
    BYTE *pPart;

    n = count;
    if (n - 1 >= 0) {
        pIndex = pOrder + (n - 1);
        do {
            g_unk0x00590d74 = (Unk0x00590d74 *)(pCars + *pIndex * 0xc24);
            g_unk0x00590d78 = (BYTE *)FUN_00469680(((char *)g_unk0x00590d74)[0xb1a]);
            FUN_00480de0();
            FUN_00480cb0();
            pTable = &g_unk0x00590d7c[3];
            do {
                pPart = (BYTE *)*pTable + ((char *)g_unk0x00590d74)[0xb1a] * 0x1a0;
                g_unk0x00590c20 = (Unk0x00590c20 *)pPart;
                if (*(int *)pPart != 0 && (pPart[0x150] & 1)) {
                    memcpy(pPart + 0x4c, pPart + 0xc, 0x40);
                    FixMatrix_Multiply((FixMatrix *)((BYTE *)g_unk0x00590c20 + 0xcc),
                                       (FixMatrix *)((BYTE *)g_unk0x00590c20 + 0xc),
                                       *(FixMatrix **)((BYTE *)g_unk0x00590c20 + 0x10c));
                    if (*(void (**)(void))((BYTE *)g_unk0x00590c20 + 0x110) != NULL)
                        (*(void (**)(void))((BYTE *)g_unk0x00590c20 + 0x110))();
                }
                pTable--;
            } while (pTable >= g_unk0x00590d7c);
            Car_BreakQueuedWindows((Car *)g_unk0x00590d74);
            pIndex--;
        } while (--n);
    }
    FUN_00484e00((int)pOrder, count);
}

extern int g_unk0x00543d50;
extern int g_stageLighting[];
extern FixVector g_stageLightDirection;
extern BillboardDef g_unk0x00543f00;
extern BillboardDef g_unk0x00547908;
extern int g_unk0x00543ef8;
void FixMatrix_GetForward(FixVector *pOut, FixMatrix *pM);
void FUN_0045f240(void);
void FUN_0045f260(void);
void FUN_00462cb0(BYTE *pColour);
void StageLights_UpdateDirection(void);
void FUN_00492e30(FixVector *pLight);
void FUN_004b5760(FixVector *pLightDir);
void FUN_004b3f20(void);

#define RAND_FIX() ((int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536))

// Shares the 400 weather particles between the views and places each view's
// share in a random box in front of its camera; resets the views' weather
// state.
// FUNCTION: CMR2 0x0045eed0
void FUN_0045eed0(void)
{
    int views;
    int view;
    int offset;
    SceneNode **ppNode;
    BYTE *pView;
    short *pSlot;
    int *pValue;
    int i;
    int j;
    int k;
    short share;
    FixVector forward;
    FixVector corner;
    StageDeformNode *pNode;

    views = g_unk0x00543e98;
    g_unk0x00543da0 = FixDiv(views << 16, 0x1900000);
    for (view = 0, offset = 0, ppNode = g_viewNodes; view < views; view++, offset += 0x178, ppNode++) {
        pView = (BYTE *)g_unk0x00547ac8 + offset;
        pSlot = (short *)(pView + 0x78);
        pValue = (int *)(pView + 0x110);
        for (i = 5; i != 0; i--) {
            for (j = 5; j != 0; j--) {
                *pSlot = -1;
                pValue[-0x19] = 0;
                *pValue = 0;
                pSlot++;
                pValue++;
            }
        }
        share = (short)(400 / views);
        *(short *)(pView + 0x74) = share;
        *(short *)(pView + 0x76) = (short)view * share;
        *(int *)(pView + 0x5c) = *(int *)(pView + 0x58) =
            FixMul(FixMul(g_unk0x00543d50, *(int *)(pView + 0x54)), *(short *)(pView + 0x74) << 16);
        *(int *)(pView + 0x174) = 0;
        *(int *)(pView + 0x60) = 0x10000;
        *(int *)(pView + 0x64) = 0x10000;
        *(int *)(pView + 0x68) = 0x10000;
        *(int *)(pView + 0x2c) = 0;
        *(int *)(pView + 0x30) = 0;
        *(int *)(pView + 0x34) = 0;
        FixMatrix_GetPosition((FixVector *)(pView + 8), &(*ppNode)->current);
        FixMatrix_GetForward(&forward, &(*ppNode)->current);
        FixVecScale(&forward, &forward, 0xe0000);
        ((FixVector *)(pView + 8))->x += forward.x;
        ((FixVector *)(pView + 8))->y += forward.y;
        ((FixVector *)(pView + 8))->z += forward.z;
        corner.x = ((FixVector *)(pView + 8))->x - 0xc0000;
        corner.y = ((FixVector *)(pView + 8))->y - 0x60000;
        *(FixVector *)(pView + 0x14) = *(FixVector *)(pView + 8);
        corner.z = ((FixVector *)(pView + 8))->z - 0xc0000;
        for (k = 0; k < *(short *)(pView + 0x74); k++) {
            pNode = &g_unk0x00543fb0[*(short *)(pView + 0x76) + k];
            pNode->x = FixMul(RAND_FIX(), 0x180000);
            pNode->y = FixMul(RAND_FIX(), 0xc0000);
            i = FixMul(RAND_FIX(), 0x180000);
            pNode->x += corner.x;
            pNode->y += corner.y;
            ((BYTE *)&pNode->field_0x1c)[0] = 0x4b;
            pNode->z = i + corner.z;
            pNode->spin = FixMul(RAND_FIX(), 0x40000);
            pNode->field_0x10 = RAND_FIX();
            pNode->angle = FixMul(RAND_FIX(), 0x1680000);
            pNode->scale = RAND_FIX();
            ((BYTE *)&pNode->field_0x1c)[1] = (BYTE)(rand() % 3);
        }
    }
}

// Sets up the stage weather: particle looks, billboards, the weather
// particles, then the stage light colour and direction.
// FUNCTION: CMR2 0x0045eca0
void FUN_0045eca0(void)
{
    BYTE colour[4];
    FixVector light;

    g_unk0x00543ef8 = 1;
    FUN_0045f260();
    switch (CGameInfo::FUN_00405cd0()) {
    case 0:
        g_unk0x00543d50 = 0x10000;
        break;
    case 1:
        g_unk0x00543d50 = 0x4000;
        break;
    default:
        g_unk0x00543d50 = 0;
        break;
    }
    *(int *)(g_unk0x00543da0Block + 0x24) = 0xff000000;
    *(int *)(g_unk0x00543da0Block + 0x84) = 0xff000000;
    *(int *)(g_unk0x00543da0Block + 0x80) = 0x1e969696;
    *(int *)(g_unk0x00543da0Block + 0x54) = 0xff000000;
    *(int *)(g_unk0x00543da0Block + 0x50) = 0x1e969696;
    *(int *)(g_unk0x00543da0Block + 0x20) = 0x969696;
    g_unk0x00543f00.top = -0x2666;
    g_unk0x00543f00.left = 0x2666;
    g_unk0x00543f00.bottom = 0x2666;
    g_unk0x00543f00.right = -0x2666;
    *(unsigned int *)(g_unk0x00543da0Block + 0x28) = 0x3f000000;
    *(unsigned int *)(g_unk0x00543da0Block + 0x2c) = 0;
    *(unsigned int *)(g_unk0x00543da0Block + 0x88) = 0x3e000000;
    *(unsigned int *)(g_unk0x00543da0Block + 0x8c) = 0x3f7fff58;
    *(unsigned int *)(g_unk0x00543da0Block + 0x58) = 0x3f600000;
    *(unsigned int *)(g_unk0x00543da0Block + 0x5c) = 0x3f7fff58;
    g_unk0x00543f00.a = 0xfe;
    g_unk0x00543f00.r = 0xff;
    g_unk0x00543f00.g = 0xff;
    g_unk0x00543f00.b = 0xff;
    g_unk0x00543ed0.top = -0xa3d;
    g_unk0x00543ed0.left = 0x11eb;
    g_unk0x00543ed0.bottom = 0xa3d;
    g_unk0x00543ed0.right = -0x11eb;
    g_unk0x00543ed0.r = 0xaa;
    g_unk0x00543ed0.g = 0xaa;
    g_unk0x00543ed0.b = 0xaa;
    g_unk0x00543ed0.a = 0xaa;
    g_unk0x00547908.top = -0xf5c;
    g_unk0x00547908.left = 0xf5c;
    g_unk0x00547908.bottom = 0xf5c;
    g_unk0x00547908.right = -0xf5c;
    g_unk0x00547908.a = 0xaa;
    g_unk0x00547908.r = 0xaa;
    g_unk0x00547908.g = 0xaa;
    g_unk0x00547908.b = 0xaa;
    FUN_0045eea0();
    FUN_0045eed0();
    FUN_0045f240();
    StageLights_UpdateDirection();
    colour[3] = 0xff;
    colour[0] = (BYTE)(g_stageLighting[0x12] >> 16);
    colour[1] = (BYTE)(g_stageLighting[0x13] >> 16);
    colour[2] = (BYTE)(g_stageLighting[0x14] >> 16);
    FUN_00462cb0(colour);
    light.x = g_stageLighting[0x15];
    light.y = g_stageLighting[0x16];
    light.z = g_stageLighting[0x17];
    FUN_00492e30(&light);
    FUN_004b5760(&g_stageLightDirection);
    FUN_004b3f20();
}
#undef RAND_FIX

void FUN_004ae260(void);
void FUN_00421610(void);
void FUN_00465600(void);
void FUN_00466360(void);
void FUN_00472cb0(void);
void FUN_0042b720(int index, char value);
void FUN_0041c510(void);
void FUN_0041c260(void);
void FUN_0048ca70(void);
void FUN_00447f70(void);
void FUN_00411450(int keepName);
void FUN_00411b20(void);
void FUN_00416670(void);

// Pushes the stage state to the graphics layer after a device reset: updates
// every subsystem, then re-uploads each live texture with a dummy triangle.
// FUNCTION: CMR2 0x00424c50
void FUN_00424c50(void)
{
    D3DTLVERTEX verts[3];
    int i;

    FUN_004ae260();
    FUN_00421610();
    FUN_00423ff0();
    FUN_00465600();
    FUN_00466360();
    if (CGameInfo::FUN_00406320() != 0) {
        FUN_0041c510();
    } else {
        if (CGameInfo::FUN_00405d80() == 4) {
            FUN_00472cb0();
        } else {
            for (i = 0; i < 2; i++) {
                if ((BYTE)RallyData_GetFlag25() != 0 && CGameInfo::FUN_00405e00() == 0 &&
                    i >= (int)(RallyDataState() & 0xff))
                    FUN_0042b720(i, -1);
            }
        }
        if (CGameInfo::FUN_00405d80() != 3)
            FUN_0041c260();
    }
    if ((BYTE)RallyData_FUN_00407e70() != 0)
        FUN_0048ca70();
    FUN_00447f70();
    FUN_00458100(1);
    StageTiming_FUN_00455610();
    FUN_00411450(1);
    FUN_00411b20();
    FUN_00416670();

    CGraphics::m_pTextureManager->pD3D->BeginScene();

    verts[0].color = 0xff000000;
    verts[1].color = 0xff000000;
    verts[2].color = 0xff000000;
    verts[0].sx = 0.0f;
    verts[0].sy = 0.0f;
    verts[1].sx = 10.0f;
    verts[1].sy = 0.0f;
    verts[2].sx = 0.0f;
    verts[2].sy = 10.0f;

    for (i = 0; i < CGraphics::m_textureCount; i++) {
        Texture *pTexture = CGraphics::m_pTextureManager->textureBuffer[i];
        if (pTexture != NULL && pTexture->pSurface != NULL) {
            CGraphics::FUN_004a4850(0, (int)pTexture);
            CGraphics::m_pTextureManager->pD3D->DrawPrimitive(D3DPT_TRIANGLELIST, 0x1c4, verts, 3, 0);
        }
    }

    CGraphics::m_pTextureManager->pD3D->EndScene();
}

// Random seed for the stage timing shuffles.
// GLOBAL: CMR2 0x00541f88
unsigned int g_unk0x00541f88;

// Per-driver timing block: 0xa0 bytes of split samples followed by the four
// spread parameters of the stage (the two fixed-point ones first).
struct StageTimingSpread {
    BYTE field_0x0[0xa0];
    int field_0xa0;
    int field_0xa4;
    int field_0xa8;
    int field_0xac;
};

// Rebuilds the stage's timing records when a stage starts: resets the split
// tables, seeds the RNG, lays out the driver slots and pairs the split
// rankings.
// FUNCTION: CMR2 0x00455470
void FUN_00455470(char param_1)
{
    StageTimingSpread local;
    int *p;

    if (g_unk0x00542604 == 0)
        return;
    StageTiming_Reset();
    FUN_0045e6b0(&local.field_0xa8, &local.field_0xac, &local.field_0xa0, &local.field_0xa4);
    if ((char)RallyDataCountryIndex() == 3) {
        local.field_0xa8 = 0;
        local.field_0xac = 0;
    }
    if (param_1 != 0)
        g_unk0x00541f88 = CMain::GetFrameTime();
    srand(g_unk0x00541f88);
    FUN_004556a0(CGameInfo::FUN_00405d70());
    FUN_00455ed0((int)&local);
    FUN_004561e0((int *)&local);
    StageTiming_RebuildSplitPositions();
    if ((char)RallyData_FUN_00407e90() != 0) {
        p = (int *)(g_unk0x00542528 + 0x10);
        do {
            p[-3] = 0;
            p[0] = 0;
            p += 6;
        } while ((int)p < (int)(g_unk0x00542528 + 0xd0));
        g_unk0x00542420[0] = ConvertRawTimeToCentiseconds(*(int *)(g_unk0x0054241c + 0x2d0));
        g_unk0x00542420[1] = ConvertRawTimeToCentiseconds(*(int *)(g_unk0x0054241c + 0x310));
        FUN_004556f0();
        StageTiming_FUN_00455610();
        return;
    }
    FUN_00455620();
    StageTiming_FUN_00455610();
}

// Directory-name tables of the stage speech and text archives, one per game
// region (region 0: five languages, region 1: three, regions 2/3: one).
extern char g_str0x0051a00c[12];
extern char g_str0x0051a018[12];
extern char g_str0x0051a024[12];
extern char g_str0x0051a030[12];
extern char g_str0x0051a03c[12];
extern char g_str0x0051a048[12];
extern char g_str0x0051a054[12];
extern char g_str0x0051a100[12];

// GLOBAL: CMR2 0x0051a060
char g_str0x0051a060[16] = "polishspeech";
// GLOBAL: CMR2 0x0051a070
char g_str0x0051a070[16] = "engusaspeech";
// GLOBAL: CMR2 0x0051a080
char g_str0x0051a080[16] = "germanspeech";
// GLOBAL: CMR2 0x0051a090
char g_str0x0051a090[16] = "italianspeech";
// GLOBAL: CMR2 0x0051a0a0
char g_str0x0051a0a0[16] = "spanishspeech";
// GLOBAL: CMR2 0x0051a0b0
char g_str0x0051a0b0[16] = "frenchspeech";
// GLOBAL: CMR2 0x0051a0c0
char g_str0x0051a0c0[16] = "englishspeech";

// GLOBAL: CMR2 0x00519fbc
char *g_speechRegion0[5] = {g_str0x0051a0c0, g_str0x0051a0b0, g_str0x0051a0a0,
                            g_str0x0051a090, g_str0x0051a080};
// GLOBAL: CMR2 0x00519fd0
char *g_speechRegion1[3] = {g_str0x0051a070, g_str0x0051a0b0, g_str0x0051a0a0};
// GLOBAL: CMR2 0x00519fdc
char *g_speechRegion2[1] = {g_str0x0051a0c0};
// GLOBAL: CMR2 0x00519fe0
char *g_speechRegion3[1] = {g_str0x0051a060};
// GLOBAL: CMR2 0x00519fe4
char *g_textRegion0[5] = {g_str0x0051a054, g_str0x0051a048, g_str0x0051a03c,
                          g_str0x0051a030, g_str0x0051a024};
// GLOBAL: CMR2 0x00519ff8
char *g_textRegion1[3] = {g_str0x0051a018, g_str0x0051a048, g_str0x0051a03c};
// GLOBAL: CMR2 0x0051a004
char *g_textRegion2[1] = {g_str0x0051a054};
// GLOBAL: CMR2 0x0051a008
char *g_textRegion3[1] = {g_str0x0051a00c};

// GLOBAL: CMR2 0x0051a0d0
char g_str0x0051a0d0[16] = "%s\\FireWork.bfl";
// GLOBAL: CMR2 0x0051a0e0
char g_str0x0051a0e0[16] = "%s\\surface.bfl";
// GLOBAL: CMR2 0x0051a10c
char g_str0x0051a10c[16] = "%s\\Com%d.bfl";
// GLOBAL: CMR2 0x0051a11c
char g_str0x0051a11c[16] = "%s\\Com%dC.bfl";

// Opens the stage archives of the current region: the common one, the day file
// and the speech/text variants of the current language, then registers the
// release callback.
// FUNCTION: CMR2 0x00455080
void FUN_00455080(void)
{
    char **ppSpeech;
    char **ppText;
    unsigned int resolution;

    switch (CGameInfo::GetGameRegion()) {
    case 0:
        ppSpeech = g_speechRegion0;
        ppText = g_textRegion0;
        break;
    case 1:
        ppSpeech = g_speechRegion1;
        ppText = g_textRegion1;
        break;
    case 2:
        ppSpeech = g_speechRegion2;
        ppText = g_textRegion2;
        break;
    case 3:
        ppSpeech = g_speechRegion3;
        ppText = g_textRegion3;
        break;
    }
    if (CGameInfo::GetScreenWidth() >= 0x400 && CFrontend::FUN_004b7560(0x400) != 0 &&
        CFrontend::FUN_004b7590(0x400) != 0)
        resolution = 0x400;
    else
        resolution = 0x280;
    if (CFrontend::FUN_004a9700() != 0)
        sprintf(CFrontend::m_stringDest, g_str0x0051a11c, CInstallInfo::GetBigFilesDir(), resolution);
    else
        sprintf(CFrontend::m_stringDest, g_str0x0051a10c, CInstallInfo::GetBigFilesDir(), resolution);
    CGenericFileLoader::FUN_004a9d70((GenericFile *)StageTiming_GetStageFile1(), CFrontend::m_stringDest);
    sprintf(CFrontend::m_stringDest, g_str0x0051a100, CInstallInfo::GetCountrySpecificDir(),
            CGameInfo::GetGameRegionDirectory(), ppSpeech[CGameInfo::GetGameLanguage() & 0xff]);
    CGenericFileLoader::FUN_004a9d70((GenericFile *)StageTiming_GetStageFile2(), CFrontend::m_stringDest);
    sprintf(CFrontend::m_stringDest, g_str0x0051a100, CInstallInfo::GetCountrySpecificDir(),
            CGameInfo::GetGameRegionDirectory(), ppText[CGameInfo::GetGameLanguage() & 0xff]);
    CGenericFileLoader::FUN_004a9d70((GenericFile *)StageTiming_GetStageFile6(), CFrontend::m_stringDest);
    sprintf(CFrontend::m_stringDest, CFrontend::m_strCommonBfl, CInstallInfo::GetBigFilesDir());
    CGenericFileLoader::FUN_004a9d70((GenericFile *)StageTiming_GetStageFile0(), CFrontend::m_stringDest);
    sprintf(CFrontend::m_stringDest, g_str0x0051a0e0, CInstallInfo::GetSoundsDir());
    CGenericFileLoader::FUN_004a9d70((GenericFile *)StageTiming_GetStageFile5(), CFrontend::m_stringDest);
    if ((BYTE)FUN_00407270() != 0) {
        sprintf(CFrontend::m_stringDest, g_str0x0051a0d0, CInstallInfo::GetBigFilesDir());
        CGenericFileLoader::FUN_004a9d70((GenericFile *)StageTiming_GetStageFile4(), CFrontend::m_stringDest);
    }
    CGame::RegisterCallback(FUN_00454f20, NULL);
}
