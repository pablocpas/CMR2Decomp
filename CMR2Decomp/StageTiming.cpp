#include "StageObjectCount.h"
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
#include "CarParts.h"
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
// Split ranking state: drivers in the stage, their slots and ten split rows
// of sixteen drivers. One object: the original re-reads the driver count
// after every byte store into the tables.
struct StageSplitState {
    int driverCount;           // 0x000
    char driverSlot[16];       // 0x004
    char indices[10][16];      // 0x014
    char positions[10][16];    // 0x0b4
    char timeIndices[10][16];  // 0x154
    char count[12];            // 0x1f4
};
// GLOBAL: CMR2 0x00541f98
StageSplitState g_stageSplit;
#define g_unk0x00541f98 (g_stageSplit.driverCount)
#define g_stageDriverSlot (g_stageSplit.driverSlot)
#define g_stageSplitDriverIndices (g_stageSplit.indices)
#define g_stageSplitPositions (g_stageSplit.positions)
#define g_stageSplitTimesRawDriverIx (g_stageSplit.timeIndices)
#define g_stageSplitDriverCount (g_stageSplit.count)

// GLOBAL: CMR2 0x00542418
short g_unk0x00542418;
// Split time spread table of the stage (5-byte rows: -, max down, max up).
// GLOBAL: CMR2 0x0054241c
char *g_unk0x0054241c;

// FUNCTION: CMR2 0x00456a30
void StageTiming_SetSplitDisplayFlag(int offset, int unused)
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
int StageTiming_GetStageArchiveState(void)
{
    return g_unk0x00541f8c;
}

// FUNCTION: CMR2 0x00455610
void StageTiming_ClearSplitDisplayFlags(void)
{
    g_unk0x00542418 = 0;
}

int Stage_GetSplitPositionCount(void);

// Gives the last count driver slots (15 downwards) to the split table.
// FUNCTION: CMR2 0x004556a0
void StageTiming_AssignReservedDriverSlots(int count)
{
    int i;

    g_unk0x00541f98 = 16 - count;
    for (i = 0; i < count; i++)
        g_stageDriverSlot[i] = 15 - i;
    for (i = 1; i <= Stage_GetSplitPositionCount(); i++)
        g_stageSplitDriverCount[i] = g_unk0x00541f98;
}

// GLOBAL: CMR2 0x0051a14c
char g_strRaceHasAiDriver[] = "Race %d has ai driver %d\n";
// GLOBAL: CMR2 0x0051a168
char g_strRaceHasPlayer[] = "Race %d has player\n";
int StageUI_GetRaceResultValue(void);
// Per-driver split pace record (two per driver at g_unk0x00542528).
struct SplitRate {
    int rate;               // 0x0
    int count;              // 0x4
    float scale;            // 0x8
};
int StageTiming_UpdateSplitRatesForSlot(int slot, int driverID);
void StageTiming_RemoveDriverFromSplitRankings(int slot, int driver);

// Pairs each human player's stage slot with an opponent for the split
// rankings, by game mode: championship/single stages walk the overall
// standings from the back (so each player races the driver just ahead),
// arcade uses the stage positions, and the knockout takes the two drivers
// of every match of the current round.
// FUNCTION: CMR2 0x004556f0
void StageTiming_PairPlayerOpponentSlots(void)
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
    bool hasAi;
    bool hasPlayer;

    players = CGameInfo::GetConfiguredPlayerCount();
    switch (CGameInfo::GetConfiguredGameMode()) {
    case 0:
    case 1:
        if (StageUI_GetRaceResultValue() == 1) {
            g_stageSplitUnk0x00541f78[0][0] = (char)StageTiming_GetDriverSlot(0);
            g_stageSplitUnk0x00541f78[0][1] = (char)StageTiming_GetDriverSlot(1);
            return;
        }
        i = 0;
        memset(used, 0, sizeof(used));
        pSlot = slots;
        for (; i < 16; i++) {
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
        pos = 15;
        pSlot = &slots[players - 1];
        for (; pos >= 0; pos--) {
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
        for (i = 0; i < players; i++) {
            if (g_stageSplitUnk0x00541f78[slot][1] == 16) {
                for (k = 15; k >= 0; k--) {
                    if (used[k] == 0) {
                        g_stageSplitUnk0x00541f78[i][1] = (char)k;
                        used[k] = 1;
                    }
                }
            }
        }
        for (i = 0; i < players; i++) {
            StageTiming_UpdateSplitRatesForSlot(i, g_stageSplitUnk0x00541f78[i][1]);
            StageTiming_RemoveDriverFromSplitRankings(i, g_stageSplitUnk0x00541f78[i][1]);
        }
        break;
    case 2:
        if (StageUI_GetRaceResultValue() == 1) {
            g_stageSplitUnk0x00541f78[0][0] = (char)StageTiming_GetDriverSlot(0);
            g_stageSplitUnk0x00541f78[0][1] = (char)StageTiming_GetDriverSlot(1);
            return;
        }
        for (i = 0; i < players; i++) {
            g_stageSplitUnk0x00541f78[i][0] = (char)StageTiming_GetDriverSlot(i);
            g_stageSplitUnk0x00541f78[i][1] = (char)StageTiming_GetDriverIDForPosition(i);
            StageTiming_UpdateSplitRatesForSlot(i, g_stageSplitUnk0x00541f78[i][1]);
            StageTiming_RemoveDriverFromSplitRankings(i, g_stageSplitUnk0x00541f78[i][1]);
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
        i = 0;
        if (count > 0) {
            pOut = &g_stageSplitUnk0x00541f78[0][1];
        }
        for (; i < count; i++) {
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
                if (RallyData_GetUsableRecordCategory((BYTE)drivers[k]) == -1) {
                    out[k] = StageTiming_GetDriverSlot(drivers[k]);
                    hasPlayer = 1;
                } else {
                    ai = out[k] = drivers[k] - CGameInfo::GetConfiguredPlayerCount();
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
                    StageTiming_UpdateSplitRatesForSlot(i, ai);
                }
                pOut += 2;
            }
        }
        break;
    }
}

// FUNCTION: CMR2 0x00455ab0
int StageTiming_GetSplitSecondaryEntry(int iSplit)
{
    return g_stageSplitUnk0x00541f78[iSplit][1];
}

// FUNCTION: CMR2 0x00455ac0
int StageTiming_GetSplitTableEntry(int iSplit, int iIndex)
{
    return g_stageSplitUnk0x00541f78[iSplit][iIndex];
}

// FUNCTION: CMR2 0x00455ae0
BYTE StageTiming_GetSplitDisplayState(void)
{
    return g_unk0x00542604;
}

// Removes a driver from every split ranking, remembering in slot the
// driver's old rank index of the last split.
// FUNCTION: CMR2 0x00455bc0
void StageTiming_RemoveDriverFromSplitRankings(int slot, int driver)
{
    int splits = Stage_GetSplitPositionCount();
    int split;
    int pos;

    for (split = 1; split <= splits; split++) {
        pos = g_stageSplitPositions[split][driver];
        g_unk0x00541f90[slot] = g_stageSplitTimesRawDriverIx[split][pos];
        for (; pos < g_stageSplitDriverCount[split] - 1; pos++) {
            g_stageSplitTimesRawDriverIx[split][pos] = g_stageSplitTimesRawDriverIx[split][pos + 1];
            g_stageSplitDriverIndices[split][pos] = g_stageSplitDriverIndices[split][pos + 1];
            g_stageSplitPositions[split][g_stageSplitDriverIndices[split][pos]] = (char)pos;
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

// match 93%: the volatile zero store keeps MSVC6 from folding the nested
// zero-fill into one rep stosd (the original stores per iteration); only the
// placement of the flat pointer increment differs from the original.
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
            *(volatile int *)&g_stageSplitTimesRaw[iSplit][i] = 0;
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
    iSlot1 = iPosition1 - CGameInfo::GetConfiguredPlayerCount();
    iSlot2 = iPosition2 - CGameInfo::GetConfiguredPlayerCount();
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
    int iSplit;
    int s;
    int i;
    char *p;
    int d;
    int t;

    iSplit = GetStageSplitCount();
    for (i = 0; i < g_unk0x00541f98; i++) {
        d = g_stageSplitDriverIndices[iSplit][i];
        aiDriverForSlot[g_stageSplitTimesRawDriverIx[iSplit][i]] = d;
        g_stageSplitPositions[iSplit][d] = i;
    }
    for (i = g_unk0x00541f98; i < 16; i++) {
        g_stageSplitDriverIndices[iSplit][i] = i;
        g_stageSplitPositions[iSplit][i] = i;
    }

    for (s = 1; s < iSplit; s++) {
        p = g_stageSplitDriverIndices[s];
        for (i = 0; i < g_unk0x00541f98; i++) {
            d = aiDriverForSlot[g_stageSplitTimesRawDriverIx[s][i]];
            p[i] = d;
            g_stageSplitPositions[s][d] = i;
        }
        for (i = g_unk0x00541f98; i < 16; i++) {
            p[i] = i;
            g_stageSplitPositions[s][i] = i;
        }
    }
}

BYTE RallyData_GetSecondarySelectionNibble(void);
unsigned char RallyData_GetSelectionFlag26(void);
char Race_GetBaseCarCount(void);
unsigned char RallyData_GetSelectionFlag28(void);
unsigned int NetPlayers_IsPlayerPresent(int index);

// Builds the per-split interpolation factors between the first and last split
// time of the stage.
// FUNCTION: CMR2 0x00456330
void StageTiming_BuildSplitInterpolationFactors(int *param_1)
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
        if (param_1[0x28] >= Stage_GetSplitPositionFixed(iVar5 - 1) && param_1[0x28] < Stage_GetSplitPositionFixed(iVar5))
            local_20 = iVar5;
        if (param_1[0x29] > Stage_GetSplitPositionFixed(iVar5 - 1) && param_1[0x29] <= Stage_GetSplitPositionFixed(iVar5))
            local_24 = iVar5;
    }
    iVar5 = param_1[0x29];
    iVar2 = param_1[0x28];
    iVar7 = iVar5 - iVar2;
    local_14 = 0;
    *param_1 = 0;
    iVar6 = 1;
    if (iVar1 >= 1) {
        piVar8 = param_1 + 0xb;
        do {
            iVar3 = Stage_GetSplitPositionFixed(iVar6);
            iVar4 = Stage_GetSplitPositionFixed(iVar6 - 1);
            uVar7 = iVar3 - iVar4;
            if (iVar6 < local_20)
                local_c = uVar7;
            if (iVar6 == local_20)
                local_c = param_1[0x28] - Stage_GetSplitPositionFixed(iVar6 - 1);
            if (iVar6 > local_20)
                local_c = 0;
            if (iVar6 < local_24)
                local_8 = 0;
            if (iVar6 == local_24)
                local_8 = Stage_GetSplitPositionFixed(iVar6) - param_1[0x29];
            if (iVar6 > local_24)
                local_8 = uVar7;
            *piVar8 = FixDiv(local_c, uVar7);
            iVar3 = FixDiv(local_8, uVar7);
            *(volatile int *)(piVar8 + 0x14) = iVar3;
            piVar8[10] = (0x10000 - *piVar8) - iVar3;
            local_14 = local_14 + ((uVar7 - local_8) - local_c);
            if (iVar7 == 0)
                piVar8[-10] = 0;
            else
                piVar8[-10] = FixDiv(local_14, iVar7);
            iVar6 = iVar6 + 1;
            piVar8 = piVar8 + 1;
        } while (iVar6 <= iVar1);
    }
}

// Number of cars in the stage (players, ghost and network players).
// FUNCTION: CMR2 0x00456ca0
unsigned int StageTiming_GetTotalCarCount(void)
{
    unsigned int count;
    int i;

    if ((BYTE)RallyData_GetSelectionFlag26() && !CGameInfo::GetGameModeOptionBit19())
        count = (BYTE)RallyDataState() + RallyData_GetSecondarySelectionNibble();
    else
        count = (BYTE)Race_GetBaseCarCount();
    if ((BYTE)RallyData_GetSelectionFlag28() && (BYTE)CGameInfo::GetSoundOptionBit30())
        count++;
    if (CGameInfo::GetGameModeOptionBit19() && CGameInfo::GetConfiguredGameMode() != 10) {
        for (i = 0; i < 7; i++) {
            if ((BYTE)NetPlayers_IsPlayerPresent(i))
                count++;
        }
    }
    return count;
}

// Sets every vertex of a mesh-backed scene node to a shade of grey.
// FUNCTION: CMR2 0x00456d20
void StageTiming_SetNodeVertexGrey(SceneNode *pNode, BYTE colour)
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
void StageTiming_SetNodeTreeVertexGrey(SceneNode *pNode, BYTE colour)
{
    for (; pNode != NULL; pNode = pNode->pNext) {
        StageTiming_SetNodeVertexGrey(pNode, colour);
        if (pNode->pFirstChild != NULL)
            StageTiming_SetNodeTreeVertexGrey(pNode->pFirstChild, colour);
    }
}

// GLOBAL: CMR2 0x00588e80
void *g_unk0x00588e80[8];
extern void *g_unk0x00588ea0[8];
// GLOBAL: CMR2 0x00588d3c
int g_unk0x00588d3c;


// Releases the eight stage buffers.
// FUNCTION: CMR2 0x0046c500
BOOL Replay_ReleaseStageBuffers(void)
{
    void **p;

    p = g_unk0x00588e80;
    do {
        if (*p != NULL) {
            CFileBuffer::FreeGenericFileBuffer(*p);
            *p = NULL;
        }
        p++;
    // 0x588ea0 in the original (g_unk0x00588ea0, the next global).
    } while ((int)p < (int)(g_unk0x00588e80 + 8));
    g_unk0x00588d3c = 0;
    g_unk0x00588d14 = 0;
    return TRUE;
}

// GLOBAL: CMR2 0x00542ae8
StageArchiveTables g_stageArchiveTables;

// match 68%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00456b70
bool StageTiming_ReleaseStartArchiveBuffers(void)
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
void StageTiming_ResetClockAndPreviousTime(void)
{
    g_unk0x0053d1b4 = 0;
    g_unk0x0053d1b0 = 0;
}

// FUNCTION: CMR2 0x00448110
int StageTiming_GetClockTime(void)
{
    return g_unk0x0053d1b0;
}

// FUNCTION: CMR2 0x004481b0
BYTE StageTiming_GetStageClockState(void)
{
    return g_unk0x0053d1d8;
}

// FUNCTION: CMR2 0x004481c0
int StageTiming_GetCarTimingByte81(int car)
{
    return g_carStageTiming[car].field_0x81;
}

// FUNCTION: CMR2 0x004481e0
int StageTiming_GetSplitIndexValue(int index)
{
    return g_unk0x0053dda8[index];
}

// FUNCTION: CMR2 0x004481f0
int StageTiming_GetCarSplitTime(int car, int index)
{
    return g_carStageTiming[car].splitTimes[index];
}

// FUNCTION: CMR2 0x00448210
int StageTiming_GetClampedElapsedCarTime(int car)
{
    int time = g_unk0x0053d1b0 - g_carStageTiming[car].lastTime;
    if (time >= 360000)
        time = 359999;
    return time;
}

// FUNCTION: CMR2 0x00448240
int StageTiming_GetCarSplitMarker(int car, int index)
{
    return g_carStageTiming[car].splits[index];
}

// Split times of each car per checkpoint group (8 x 12 x 5).
// GLOBAL: CMR2 0x0053d1e8
int g_unk0x0053d1e8[8][12][5];

int StageTiming_GetCheckpointGroupIndex(int index);
int StageTiming_GetCheckpointSplitIndex(int index);
int StageTiming_GetCheckpointField14(int index);

// Leader of each split (row per checkpoint group and split, 8 bytes each).
// GLOBAL: CMR2 0x0053de1c
char g_unk0x0053de1c[108][8];

// Time of the split leader at the given split.
// FUNCTION: CMR2 0x00448260
int StageTiming_GetSplitLeaderTime(int index)
{
    int group;
    int split;

    if ((BYTE)RallyData_GetFlag24()) {
        group = StageTiming_GetCheckpointGroupIndex(index);
        split = StageTiming_GetCheckpointSplitIndex(index);
        return g_unk0x0053d1e8[g_unk0x0053de1c[group * 9 + split][0]][group][split];
    }
    split = StageTiming_GetCheckpointField14(index);
    return g_carStageTiming[g_unk0x0053de1c[split][0]].field_0x4[split];
}

// FUNCTION: CMR2 0x004482d0
int StageTiming_GetGroupedCheckpointValue(int index, int car)
{
    if ((BYTE)RallyData_GetFlag24()) {
        int group = StageTiming_GetCheckpointGroupIndex(index);
        int split = StageTiming_GetCheckpointSplitIndex(index);

        return g_unk0x0053d1e8[car][group][split];
    }
    return g_carStageTiming[car].field_0x4[StageTiming_GetCheckpointField14(index)];
}

// FUNCTION: CMR2 0x00448330
int StageTiming_GetCarTimingByte82(int car)
{
    return g_carStageTiming[car].field_0x82;
}

// FUNCTION: CMR2 0x00448350
int StageTiming_GetCarTimingByte80(int car)
{
    return g_carStageTiming[car].field_0x80;
}

// FUNCTION: CMR2 0x00448370
BYTE StageTiming_GetCarTimingByte83(int car)
{
    return g_carStageTiming[car].field_0x83;
}

// FUNCTION: CMR2 0x00448390
int StageTiming_GetBoundedFinishOrderEntry(int index)
{
    if (index >= 0 && index < 8)
        return g_unk0x0053e17c[index];
    return -1;
}

// FUNCTION: CMR2 0x004483b0
int StageTiming_GetFinishOrderSecondaryEntry(int index)
{
    return g_unk0x0053e184[index];
}

// FUNCTION: CMR2 0x00448620
void StageTiming_CommitChampionshipPoints(void)
{
    RallyTiming_AddChampionshipPoints(g_unk0x0053e17c, g_unk0x0053d1b8);
}

// FUNCTION: CMR2 0x00448670
int StageTiming_GetFinishedDriverCount(void)
{
    return (char)g_unk0x0053e18c;
}

int StageTiming_GetCheckpointField12(int index);

// Sort order of two cars by checkpoint group, then start time.
// FUNCTION: CMR2 0x004486a0
int StageTiming_CompareCheckpointAndStartOrder(int a, int b)
{
    int result = 0;

    if (StageTiming_GetCheckpointField12(a) == StageTiming_GetCheckpointField12(b)) {
        if (g_carStageTiming[a].startTime >= g_carStageTiming[b].startTime)
            return -1;
    } else {
        if (StageTiming_GetCheckpointField12(a) > StageTiming_GetCheckpointField12(b))
            result = 1;
        if (StageTiming_GetCheckpointField12(a) < StageTiming_GetCheckpointField12(b))
            return -1;
    }
    return result;
}

// FUNCTION: CMR2 0x00448780
void StageTiming_SetCarStartClock(int car)
{
    g_carStageTiming[car].startTime = g_unk0x0053d1b0;
}

unsigned int RallyData_GetSetupModeBits(void);
unsigned int RallyData_GetSetupLowNibble(void);
void StageTiming_QueueDriverSlot(int index);

// In a two-player split race, queues both driver slots once the leader's lead
// exceeds the allowed gap.
// FUNCTION: CMR2 0x00448bf0
void StageTiming_QueueSplitRaceDriversByGap(int slot)
{
    if (RallyData_GetSetupModeBits() == 1 && g_unk0x0053d1a6 != 0 && slot != g_unk0x0053d1a8) {
        if (g_unk0x0053d1a2 > (int)(RallyData_GetSetupLowNibble() * 100)) {
            g_unk0x0053d1a7 = 1;
            StageTiming_QueueDriverSlot(g_unk0x0053d1a8);
            StageTiming_QueueDriverSlot(1 - g_unk0x0053d1a8);
        }
    }
}

// FUNCTION: CMR2 0x00448c60
int StageTiming_GetPendingDriverEntry(int index)
{
    return g_unk0x0053d1a4[index];
}

// FUNCTION: CMR2 0x00448c70
int StageTiming_GetPendingDriverCount(void)
{
    return g_unk0x0053d1a2;
}

// FUNCTION: CMR2 0x00448c80
int StageTiming_GetPendingDriverState(void)
{
    return g_unk0x0053d1a8;
}

// FUNCTION: CMR2 0x00448c90
BYTE StageTiming_GetPendingDriverByteA6(void)
{
    return g_unk0x0053d1a6;
}

// FUNCTION: CMR2 0x00448ca0
BYTE StageTiming_GetPendingDriverByteA7(void)
{
    return g_unk0x0053d1a7;
}

// FUNCTION: CMR2 0x00448cb0
BYTE StageTiming_GetFinishStateEntry(int index)
{
    return g_unk0x0053e18d[index];
}

// FUNCTION: CMR2 0x00448cc0
BYTE StageTiming_GetFinishStateByte(void)
{
    return g_unk0x0053e18f;
}

// FUNCTION: CMR2 0x00448cd0
BYTE StageTiming_GetCarTimingByte84(int car)
{
    return g_carStageTiming[car].field_0x84;
}

// FUNCTION: CMR2 0x00448630
void StageTiming_RecordDriverTimeoutFinish(int index)
{
    g_unk0x0053d1da[index] = 1;
    g_unk0x0053e190[GetStageSplitCount() + index * 9] = g_unk0x0053d1b0;
    g_unk0x0053d1b8[index] = g_unk0x0053d1b0 + 0x4650;
    g_unk0x0053e18c++;
}

// FUNCTION: CMR2 0x004483c0
int StageTiming_GetValidStartTime(int index)
{
    if (index == -1)
        return -1;
    return g_unk0x0053d1b8[index];
}

// FUNCTION: CMR2 0x00448680
unsigned int StageTiming_GetDriverSplitClock(int index, int split)
{
    return g_unk0x0053e190[index * 9 + split];
}

// Registers the mesh-backed node in the per-slot tables.
// FUNCTION: CMR2 0x00466e90
void StageTiming_RegisterSlotMeshNode(SceneNode *pNode, int *pSlot)
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

// Release callback of StageTiming_AllocateMotionRecordTable.
// FUNCTION: CMR2 0x00494b10
int StageTiming_FreeMotionRecordTable(void)
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
void StageTiming_AllocateMotionRecordTable(int count)
{
    BYTE *pBuffer;

    pBuffer = (BYTE *)CFileBuffer::AllocateLockedBuffer(count * 0x2a4);
    g_unk0x00592734 = pBuffer;
    memset(pBuffer, 0, count * 0x2a4);
    g_unk0x00592738 = count;
    CGame::RegisterCallback(StageTiming_FreeMotionRecordTable, NULL);
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
// Release callback of StageTiming_AllocateViewWeatherRecords.
// FUNCTION: CMR2 0x0045e560
int StageTiming_FreeViewWeatherRecords(void)
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
void StageTiming_AllocateViewWeatherRecords(int count)
{
    g_unk0x00547ac8 = CFileBuffer::AllocateLockedBuffer(count * 376);
    g_unk0x00543ecc = CFileBuffer::AllocateLockedBuffer(count * 12);
    g_unk0x00543eb8 = CFileBuffer::AllocateLockedBuffer(count * 44);
    g_unk0x00547acc = (BYTE)count;
    CGame::RegisterCallback(StageTiming_FreeViewWeatherRecords, NULL);
}

// FUNCTION: CMR2 0x00465ec0
void StageTiming_SetNodeMeshAlpha(SceneNode *pNode, int alpha, BYTE checkFlag)
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

// match 93%: remaining diff is register choice for the row pointer and the
// table address operand order.
// Interpolates 16 fixed-point samples per row from the car's 0x7c table into
// pOut, weighting the two corner rows by the car's 0xa8/0xac offsets.
// FUNCTION: CMR2 0x004564d0
void StageTiming_InterpolateCarFixedSampleRows(int pCar, int *pOut)
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
    int *pRow;
    int sum;
    int t;
    int v1;
    int v2;
    int v3;
    int v4;
    int v5;
    int v6;
    int v1b;

    f = *(int *)(pCar + 0xac);
    n = *(int *)(pCar + 0xa8);
    rf = f - n;
    rn = 0x10000 - n;
    rc = 0x10000 - f;
    count = GetStageSplitCount();
    if (count >= 1) {
        off = 0x310;
        pIn = (int *)(pCar + 0x7c);
        pRow = pOut;
        do {
            p = pRow;
            pRow += 0x10;
            k = 0x10;
            do {
                v1 = FixMul(FixMul(pIn[-0x14], rn), *(int *)(g_unk0x0054241c + off - 0x280));
                v2 = FixMul(FixMul(pIn[-0x14], n), *(int *)(g_unk0x0054241c + off));
                v3 = FixMul(FixMul(pIn[0], rc), *(int *)(g_unk0x0054241c + off - 0x280));
                v4 = FixMul(FixMul(pIn[0], f), *(int *)(g_unk0x0054241c + off));
                v5 = FixMul(pIn[-10], *(int *)(g_unk0x0054241c + off - 0x280));
                v6 = FixMul(pIn[-10], *(int *)(g_unk0x0054241c + off));
                // the original re-reads pCar+0xa8 here instead of using n
                v1b = *(int *)(pCar + 0xa8) + FixMul(rf, pIn[-0x1f]);
                sum = *(int *)(pCar + 0xa8) + FixMul(rf, pIn[-0x1e]) + v1b;
                t = FixDiv(sum, 0x20000);
                // 0x456697 weights the second table by t, the first by 1-t
                *p = FixMul(t, v6) + FixMul(0x10000 - t, v5) + v4 + v3 + v2 + v1;
                k--;
                p++;
                off += 4;
            } while (k != 0);
            pIn++;
            count--;
        } while (count != 0);
    }
}

// match 41%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00456960
void StageTiming_AccumulateSplitDeltas(int *pDeltas)
{
    int count;
    int i, j;

    count = GetStageSplitCount();
    for (i = 0; i <= count; i++) {
        for (j = 0; j < 16; j++)
            g_stageSplitTimesRaw[i][j] = 0;
    }

    for (i = 1; i <= count; i++) {
        int *pIn = &pDeltas[(i - 1) * 16];
        for (j = 0; j < 16; j++)
            g_stageSplitTimesRaw[i][j] = g_stageSplitTimesRaw[i - 1][j] + pIn[j];
    }
}

// GLOBAL: CMR2 0x00590d7c
StageNodeTables g_stageNodeTables;
// GLOBAL: CMR2 0x00590c64
int g_unk0x00590c64;
// GLOBAL: CMR2 0x00590c6c
void **g_unk0x00590c6c;
// Release callback: frees the four per-car node tables and the lists.
// FUNCTION: CMR2 0x00480870
int StageTiming_FreeCarPartNodeTables(void)
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
void StageTiming_AllocateCarPartNodeTables(int count)
{
    int i;

    for (i = 0; i < 4; i++)
        g_unk0x00590d7c[i] = CFileBuffer::AllocateLockedBuffer(count * 416);
    g_unk0x00590c6c = (void **)CFileBuffer::AllocateLockedBuffer(count * 4);
    for (i = 0; i < count; i++)
        g_unk0x00590c6c[i] = CFileBuffer::AllocateLockedBuffer(0xb4);
    g_unk0x00590c64 = count;
    CGame::RegisterCallback(StageTiming_FreeCarPartNodeTables, NULL);
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

// FUNCTION: CMR2 0x004246a0
void ForceFeedback_CacheAppliedForces(void)
{
    int i;
    for (i = 0; i < 3; i++)
        (&g_unk0x00539278->field_0x0)[i] = (&g_unk0x00539278->field_0xc)[i];
}

// Re-applies the stored force-feedback values to the selected device.
// match 83%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00424560
void ForceFeedback_ApplyStoredForces(void)
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
void ForceFeedback_ActivateIdleSlots(void)
{
    int i;
    int slot;

    for (slot = 0; slot < 2; slot++) {
        g_unk0x00539278 = &g_forceFeedbackSlots[slot];
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
void ForceFeedback_DeactivateSlots(void)
{
    int slot;
    int i;

    for (slot = 0; slot < 2; slot++) {
        g_unk0x00539278 = &g_forceFeedbackSlots[slot];
        if (g_unk0x00539278->field_0x34 != 0) {
            if (g_unk0x00539278->field_0x2c >= 0) {
                ForceFeedback_CacheAppliedForces();
                for (i = 0; i < 3; i++)
                    (&g_unk0x00539278->field_0xc)[i] = 0;
                ForceFeedback_ApplyStoredForces();
            }
            g_unk0x00539278->field_0x34 = 0;
        }
    }
}

// Stops the forces of every active slot.
// FUNCTION: CMR2 0x004246c0
void ForceFeedback_StopSlotForces(void)
{
    int slot;
    int i;

    for (slot = 0; slot < 2; slot++) {
        g_unk0x00539278 = &g_forceFeedbackSlots[slot];
        if (g_unk0x00539278->field_0x34 != 0 && g_unk0x00539278->field_0x2c >= 0) {
            ForceFeedback_CacheAppliedForces();
            for (i = 0; i < 3; i++)
                (&g_unk0x00539278->field_0xc)[i] = 0;
            ForceFeedback_ApplyStoredForces();
        }
    }
}

void ForceFeedback_ComputeSteeringForce(void);
void ForceFeedback_ComputeAxleRumble(void);
void ForceFeedback_ComputeConditionCoefficients(void);
void ForceFeedback_ComputeRoadNoise(void);
void ForceFeedback_FadeImpulseForces(void);

extern int g_unk0x00539270[2];
extern BYTE *g_unk0x0053937c;

// Selects the force-feedback slot and car of a player, copies the car's
// centring state into the shared road-rumble values and re-applies the slot's
// forces when it is in use.
// FUNCTION: CMR2 0x00424710
void ForceFeedback_UpdatePlayerSlot(int view)
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
        ForceFeedback_CacheAppliedForces();
        ForceFeedback_ComputeAxleRumble();
        ForceFeedback_ComputeRoadNoise();
        ForceFeedback_ComputeSteeringForce();
        ForceFeedback_ComputeConditionCoefficients();
        ForceFeedback_FadeImpulseForces();
        ForceFeedback_ApplyStoredForces();
    }
}

// Fades the two impulse forces of the current slot out.
// FUNCTION: CMR2 0x00424c00
void ForceFeedback_FadeImpulseForces(void)
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
int StageTiming_GetCarReplayTailEntry(void *pCar, int index)
{
    return *(int *)(g_unk0x00588b94 + (index + *(char *)((BYTE *)pCar + 0xb1a) * 0x134) * 4 + 0x4b0);
}

// Release callback of StageTiming_AllocateCarReplayRecordTables: frees the per-record part tables
// (0x588b9c), the per-record buffers (0x588ba0) and the record arrays.
// FUNCTION: CMR2 0x00466680
int StageTiming_FreeCarReplayRecordTables(void)
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
void StageTiming_AllocateCarReplayRecordTables(int count)
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
    CGame::RegisterCallback(StageTiming_FreeCarReplayRecordTables, NULL);
}

// Sets up the whole force-feedback effect set (spring, damper, constant force)
// for the currently selected device and starts every effect.
// FUNCTION: CMR2 0x00424120
void ForceFeedback_CreateEffects(void)
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
// 8-byte record of the triangle table at g_unk0x00591af0.
struct TrackTriangle {
    short v[3];
    unsigned short surface : 7;
    unsigned short flags : 9;
};

// Reads entry index of the 8-byte table at g_unk0x00591af0.
// FUNCTION: CMR2 0x00491790
void StageTiming_ReadStageFaceRecord(short index, short *pA, short *pB, short *pC, short *pD, unsigned short *pFlags)
{
    *pA = ((TrackTriangle *)g_unk0x00591af0)[index].v[0];
    *pB = ((TrackTriangle *)g_unk0x00591af0)[index].v[1];
    *pC = ((TrackTriangle *)g_unk0x00591af0)[index].v[2];
    *pFlags = ((TrackTriangle *)g_unk0x00591af0)[index].surface;
    *pD = 0;
}

// Release callback with nothing to free.
// FUNCTION: CMR2 0x004918c0
BYTE StageTiming_NoOpLightRelease(void)
{
    return 1;
}

// Walks the serialised stage block, recording a pointer to every sub-table.
// match 71%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00490d50
void StageTiming_IndexSerializedStageTables(BYTE *pData)
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
    p = p + i * 12;
    g_unk0x00591afc = (int)p;
    p += 4;
    g_unk0x00591af0 = (int)p;
    i = *(unsigned short *)g_unk0x00591afc;
    p = p + i * 8;
    BYTE *pCount = p;
    p += 4;
    g_unk0x00591b34 = (int)pCount;
    g_unk0x00591b18 = (int)p;
    i = *(int *)pCount;
    p = p + i * 2;
    g_unk0x00591b20 = (int)p;
    p += 2;
    g_unk0x00591b1c = (int)p;
    CGame::RegisterCallback(StageTiming_NoOpLightRelease, NULL);
}

// GLOBAL: CMR2 0x0053cdbc
int g_unk0x0053cdbc;

// FUNCTION: CMR2 0x00445a40
int StageTiming_ConsumeDashRefreshLatch(void)
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

// FUNCTION: CMR2 0x00445db0
void StageTiming_ResetDashGearMarkers(void)
{
    int i;
    for (i = 0; i < 2; i++) {
        g_dashGearMarker[i] = -1;
        g_dashIdle[i] = 0;
    }
}

// FUNCTION: CMR2 0x00445dd0
int StageTiming_GetDashRevValue(int index)
{
    if (index < 2)
        return g_dashRev[index];
    return 0;
}

// GLOBAL: CMR2 0x00542420
int g_unk0x00542420[2];
// GLOBAL: CMR2 0x00542528
BYTE g_unk0x00542528[0xd8];
int Stage_GetPrimaryStartDriver(void);
extern BYTE g_unk0x00542630[];
// GLOBAL: CMR2 0x00542600
int g_unk0x00542600;
// GLOBAL: CMR2 0x0054260c
int g_unk0x0054260c[9];

// FUNCTION: CMR2 0x00456b00
void StageTiming_SetStartSelectionIndex(int value)
{
    g_unk0x00542600 = value;
}

// FUNCTION: CMR2 0x00456b10
unsigned int StageTiming_GetCurrentStartSelection(void)
{
    if (CGameInfo::GetConfiguredGameMode() == 4)
        return g_unk0x00542600;
    return StageUI_GetRaceEndEventCount() & 0xff;
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
float StageTiming_GetSelectedStartTableFloat(void)
{
    return *(float *)(g_unk0x00542528 + 0x14 + (StageTiming_GetCurrentStartSelection() * 2 - Stage_GetPrimaryStartDriver()) * 12);
}

// FUNCTION: CMR2 0x00456c00
int StageTiming_GetStartSlotIndex(int index)
{
    return g_unk0x0054260c[index];
}

// GLOBAL: CMR2 0x00542608
BYTE *g_unk0x00542608;
// GLOBAL: CMR2 0x0051a8bc
char g_strCspFormat[] = "%s.csp";

int StageTiming_NoOpResourceRelease(void);
BYTE *GameMenu_GetChampionshipTransitionState(void);

// Loads the stage's .csp data.
// FUNCTION: CMR2 0x00456c10
void StageTiming_LoadCspData(void)
{
    sprintf(CFrontend::m_stringDest, g_strCspFormat, GameMenu_GetChampionshipTransitionState());
    g_unk0x00542608 = (BYTE *)CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(),
                                                           CFrontend::m_stringDest, 0, 0, 0);
    if (g_unk0x00542608 != NULL) {
        *(BYTE **)(g_unk0x00542630 + 0x240) = g_unk0x00542608;
        *(BYTE **)(g_unk0x00542630 + 0x244) = g_unk0x00542608 + 0x18;
        CGame::RegisterCallback(StageTiming_NoOpResourceRelease, 0);
    }
}

// FUNCTION: CMR2 0x00456c90
int StageTiming_GetStartTableField244(int unused)
{
    return *(int *)(g_unk0x00542630 + 0x244);
}

// FUNCTION: CMR2 0x00457e00
int StageTiming_GetStartTableField398(int index)
{
    return *(int *)(g_unk0x00542630 + 0x398 + index * 4);
}

// match 80%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00457e10
int StageTiming_GetStartArchiveRelativeEntry(BYTE *pCar, int offset)
{
    int address = StageTiming_GetStartTableField398((signed char)pCar[0xb1a]);
    if (offset > 0)
        address += offset * 2;
    int base = StageTiming_GetStartTableField398((signed char)pCar[0xb1a]);
    return *(unsigned short *)address + base;
}

// GLOBAL: CMR2 0x005113b0
float g_unk0x005113b0 = 4.0f;
// GLOBAL: CMR2 0x005113a8
double g_unk0x005113a8 = 100.0;

// Reparte el tiempo del piloto entre los dos tramos de la tabla 0x542420.
// FUNCTION: CMR2 0x00456a40
int StageTiming_UpdateSplitRatesForSlot(int slot, int driverID)
{
    SplitRate *pRates;
    int driverTime;
    int splitIndex;
    int excessTime;
    int splitTime;

    driverTime = StageTiming_GetCurrentSplitTimeForDriver(driverID);
    pRates = &((SplitRate *)g_unk0x00542528)[slot * 2];
    for (splitIndex = 0; splitIndex < 2; splitIndex++) {
        splitTime = g_unk0x00542420[splitIndex];
        excessTime = driverTime - splitTime;
        if (excessTime < 0)
            excessTime = 0;
        pRates[splitIndex].count = excessTime / 4;
        if (pRates[splitIndex].count != 0) {
            pRates[splitIndex].rate = splitTime / 4 / pRates[splitIndex].count;
            if (pRates[splitIndex].rate < 10)
                pRates[splitIndex].rate = 10;
        }
        pRates[splitIndex].scale = (float)(g_unk0x005113a8 / ((double)driverTime / (double)splitTime * g_unk0x005113b0));
    }
    return 0;
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
void StageTiming_GetObjectRampParameters(int *p1, int *p2, int *p3, int *p4)
{
    *p1 = g_unk0x00543e88;
    *p2 = g_unk0x00543d9c;
    *p3 = (int)(__int64)(g_unk0x00543e8c * CGraphics::m_65536);
    *p4 = (int)(__int64)(g_unk0x00543e94 * CGraphics::m_65536);
}

// Marca el nodo como "sucio" en las etapas especiales y ajusta su 0x64.
// FUNCTION: CMR2 0x0045e9a0
void StageTiming_MarkSpecialStageNodeDirty(SceneNode *pNode)
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

BOOL Game_ReleaseSceneResourceBlocks(void);

// Reinicia las tablas de escena 0x58d2xx/0x58d3xx/0x58d4xx y registra el
// callback 0x4779e0.
// match 42%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00475f80
void StageObject_InitCarSceneTables(void)
{
    BYTE *pTiming;
    int *pState;
    int *pTexture;
    int *pPosition;
    int *p;

    g_unk0x0058d6a8[0] = 0;
    memset(g_stageBlock_58d47c, 0, 0x20);
    memset(g_stageBlock_58d340, 0, 0x20);
    memset(g_unk0x0058d49c, 0, 0x20);
    g_unk0x0058d6a8[1] = 0;
    memset(g_unk0x0058d6b0, 0xff, 0x1c);
    memset(g_unk0x0058d2a0, 0xff, 0x30);
    *(int *)(g_stageBlock + 0xc0) = 1;
    g_unk0x0058d6a0[0] = NULL;
    *(int *)(g_stageBlock + 0xc4) = 1;
    g_unk0x0058d3b0[0] = 1;
    g_unk0x0058d3b0[1] = 1;
    g_unk0x0058d6a0[1] = NULL;
    *(short *)(g_stageBlock + 0x30) = 0;
    *(short *)(g_stageBlock + 0x1d8) = 0x303;
    pTiming = g_stageBlock + 0xcc;
    pState = (int *)(g_stageBlock + 0x23c);
    pTexture = (int *)(g_stageBlock + 0x224);
    pPosition = (int *)(g_stageBlock + 0x298);
    do {
        *(short *)(pTiming - 2) = 0;
        pPosition[-1] = 0;
        *(short *)pTiming = 0;
        pPosition[0] = 0;
        *(short *)((BYTE *)pState + 4) = 0;
        *(short *)(pTiming + 2) = 0;
        pPosition[1] = 0;
        *pState = 1;
        *(int *)(pTiming + 0x14) = 0;
        pPosition[2] = 0;
        pTexture[-1] = 0;
        *(short *)((BYTE *)pState + 6) = 0;
        *(int *)(pTiming + 0x18) = 0x3333;
        pTexture[0] = 0;
        pState[-1] = 0;
        *(int *)(pTiming + 0x1c) = 0;
        pPosition += 7;
        pTexture += 2;
        pState += 3;
        pTiming += 0x24;
    } while ((int)pPosition < (int)(g_stageBlock + 0x2d0));
    p = (int *)g_unk0x0058d3b8 + 1;
    do {
        p[-1] = 0;
        p[0] = 0;
        p[1] = 0;
        p += 3;
    } while ((int)p < (int)g_stageBlock_58d47c);
    CGame::RegisterCallback(Game_ReleaseSceneResourceBlocks, NULL);
}





void StageObject_BuildDeformationVectors(BYTE *p);

// match 51%: the whole structure (record pool alloc, list search, FixVecScale colour clamps,
// FixMulShift32 limit) matches at 294 vs 298 instructions; the diff is MSVC6 stack-slot
// numbering and load scheduling. Kept as FUNCTION.
// Allocates a stage-deform record (0x106-byte pool, 0xd-byte slots) for the
// car's impact, fills it with the clamped offset/normal/impact colours and
// queues it, or falls back to a scratch record.
// FUNCTION: CMR2 0x00468520
void CarDamage_AllocateImpactDeformRecord(void)
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
    pBase = g_unk0x00588b98 + g_stageDeformCar->index * 0x290 + 0x106;
    limit = FixMulShift32(g_stageDeformStrength, 0xff0000);
    if (limit > 0xff)
        limit = 0xff;
    if (*(int *)(g_unk0x00588b98 + g_stageDeformCar->index * 0x290 + 0x28c) == 0) {
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
                    if (minv > v) {
                        minv = v;
                        pRec = pFound;
                        pPrev = pCur;
                    }
                    if (pFound[0xc] == 0xff)
                        break;
                    pCur = pFound;
                    pWalk = (BYTE *)((char)pFound[0xc] * 0xd + pBase);
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
    StageObject_BuildDeformationVectors(pRec);
}

// FUNCTION: CMR2 0x00469680
int *StageTiming_GetCarReplayRecord(int index)
{
    return g_unk0x00588990[index];
}

// Restricts a deformed vertex to the per-axis displacement limits and writes it
// back to the mesh's floating-point vertex data.
// match 20%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00508740
void StageDeform_ClampVertex(int *pPosition, int meshIndex, int vertexIndex, int *pRecord)
{
    // Per-vertex record: base position (16.16) then the signed displacement
    // limits at +0x1b..+0x1d, in units of 1/1024.
    BYTE *pLimit;
    FixVector base;
    int x;
    int y;
    int z;
    int limit;
    int i;

    pLimit = (BYTE *)pRecord[0x1e + meshIndex] + vertexIndex * 0x20;
    base = *(FixVector *)pLimit;
    x = (pPosition[0] - base.x) >> 6;
    y = (pPosition[1] - base.y) >> 6;
    z = (pPosition[2] - base.z) >> 6;
    if ((x > 0 && (signed char)pLimit[0x1b] <= 0) || (x < 0 && (signed char)pLimit[0x1b] >= 0))
        x = 0;
    if ((y > 0 && (signed char)pLimit[0x1c] <= 0) || (y < 0 && (signed char)pLimit[0x1c] >= 0))
        y = 0;
    if ((z > 0 && (signed char)pLimit[0x1d] <= 0) || (z < 0 && (signed char)pLimit[0x1d] >= 0))
        z = 0;
    limit = (signed char)pLimit[0x1b];
    if ((x > limit && x > 0) || (x < limit && x < 0))
        x = limit;
    limit = (signed char)pLimit[0x1c];
    if ((y > limit && y > 0) || (y < limit && y < 0))
        y = limit;
    limit = (signed char)pLimit[0x1d];
    if ((z > limit && z > 0) || (z < limit && z < 0))
        z = limit;
    pPosition[0] = base.x + (x << 6);
    pPosition[1] = base.y + (y << 6);
    i = vertexIndex * 0x30;
    pPosition[2] = base.z + (z << 6);
    *(float *)((BYTE *)((Mesh *)pRecord[meshIndex])->pVertexData + i + 8) = (float)((double)pPosition[2] * CGraphics::m_oneOver65536);
    *(float *)((BYTE *)((Mesh *)pRecord[meshIndex])->pVertexData + i + 4) = (float)((double)pPosition[1] * CGraphics::m_oneOver65536);
    *(float *)((BYTE *)((Mesh *)pRecord[meshIndex])->pVertexData + i) = (float)((double)pPosition[0] * CGraphics::m_oneOver65536);
}

// Pushes the body mesh vertices within the impact radius, then refreshes each
// affected mesh and its shadow copy.
// match 32%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00467700
void StageDeform_ApplyRadialDent(void)
{
    Car *pCar = g_stageDeformCar;
    int *pRecord = (int *)(g_unk0x00588b94 + pCar->index * 0x4d0);
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
    if (!(deepest == 0)) {
        FixVecScale(&scaled, &g_stageDeformNormal, deepest);
        g_stageDeformOffset.x += scaled.x;
        g_stageDeformOffset.y += scaled.y;
        g_stageDeformOffset.z += scaled.z;
    } else {
        if (nearest != 0) {
            FixVecScale(&scaled, &g_stageDeformNormal, nearest);
            g_stageDeformOffset.x += scaled.x;
            g_stageDeformOffset.y += scaled.y;
            g_stageDeformOffset.z += scaled.z;
        }
    }

    int normalSide = FixVecDot(&g_stageDeformOffset, &g_stageDeformNormal) >= 0;
    int innerSquare = FixMul(g_stageDeformRadius, g_stageDeformRadius);
    int reciprocalInner = FixDiv(0x10000, g_stageDeformRadius);
    int outer = g_stageDeformFalloff + g_stageDeformRadius;
    int outerSquare = FixMul(outer, outer);
    int reciprocalFalloff = FixDiv(0x10000, g_stageDeformFalloff);
    int shellScale = FixMul(g_stageDeformScale, 0x3333);

#define DENT_VERTEX(k) \
    (((float *)((BYTE *)((Mesh *)*pSlot)->pVertexData + vertexIndex * 0x30))[k])
    int meshIndex;
    int vertexIndex;
    int changed;
    int *pSlot;
    short projection;
    unsigned int phase;
    FixVector pos;
    FixVector saved;
    FixVector dir;
    FixVector limit;
    signed char *pLimits;

    pSlot = pRecord;
    for (meshIndex = 0; meshIndex < pRecord[0x117]; meshIndex++) {
        changed = 0;
        for (vertexIndex = 0; vertexIndex < pSlot[0x108]; vertexIndex++) {
            pos.x = (int)(__int64)(DENT_VERTEX(0) * CGraphics::m_65536);
            pos.y = (int)(__int64)(DENT_VERTEX(1) * CGraphics::m_65536);
            pos.z = (int)(__int64)(DENT_VERTEX(2) * CGraphics::m_65536);
            dir.x = g_stageDeformOffset.x - pos.x;
            dir.y = g_stageDeformOffset.y - pos.y;
            dir.z = g_stageDeformOffset.z - pos.z;
            projection = FixVecDot(&dir, &g_stageDeformNormal);
            projection = FixMul(projection, projection);
            if (projection > outerSquare)
                continue;
            saved = pos;
            if (projection <= innerSquare) {
                // Inside the dent: pushed along the impact normal.
                projection = g_stageDeformRadius - FixMul(reciprocalInner, projection);
                FixVecScale(&dir, &g_stageDeformNormal, projection);
                if (normalSide != 0) {
                    pos.x -= dir.x;
                    pos.y -= dir.y;
                    pos.z -= dir.z;
                } else {
                    pos.x += dir.x;
                    pos.y += dir.y;
                    pos.z += dir.z;
                }
            } else {
                // In the falloff shell: a small ripple along the vertex's own
                // limit direction.
                projection = FixMul(FixSqrt(projection) - g_stageDeformRadius, reciprocalFalloff);
                projection = FixMul(projection, shellScale);
                phase = dir.z + dir.x;
                if (phase < 0)
                    phase = -phase;
                phase = (phase & ~0x7f) % 0x400 * 0x40;
                if (phase < 0x8000)
                    phase -= 0x10000;
                projection = FixMul(projection, phase);
                pLimits = (signed char *)(pSlot[0x1e] + vertexIndex * 0x20);
                limit.x = (int)pLimits[0x18] << 9;
                limit.y = (int)pLimits[0x19] << 9;
                limit.z = (int)pLimits[0x1a] << 9;
                FixVecScale(&dir, &limit, projection);
                pos.x += dir.x;
                pos.y += dir.y;
                pos.z += dir.z;
            }
            StageDeform_ClampVertex(&pos.x, meshIndex, vertexIndex, pRecord);
            // The second vertex position moves three times as far.
            limit.x = (int)(__int64)(DENT_VERTEX(3) * CGraphics::m_65536);
            limit.y = (int)(__int64)(DENT_VERTEX(4) * CGraphics::m_65536);
            limit.z = (int)(__int64)(DENT_VERTEX(5) * CGraphics::m_65536);
            scaled.x = pos.x - saved.x;
            scaled.y = pos.y - saved.y;
            scaled.z = pos.z - saved.z;
            FixVecScale(&scaled, &scaled, 0x30000);
            limit.x += scaled.x;
            limit.y += scaled.y;
            limit.z += scaled.z;
            DENT_VERTEX(3) = (float)((double)limit.x * CGraphics::m_oneOver65536);
            DENT_VERTEX(4) = (float)((double)limit.y * CGraphics::m_oneOver65536);
            DENT_VERTEX(5) = (float)((double)limit.z * CGraphics::m_oneOver65536);
            changed = 1;
        }
        if (changed) {
            Mesh *pMesh = (Mesh *)((SceneNode *)pSlot[0xf])->pObject;
            if (pMesh != NULL) {
                Mesh_RefreshVertices(pMesh);
                RallyData_ValidateIndex((int)pMesh);
                Scene_MarkShadowPartDirty(pCar->pNode0x720, pMesh);
            }
        }
        pSlot++;
    }
#undef DENT_VERTEX
}

// Deforms the body mesh around an impact projected onto a plane.
// match 46%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00467e90
void StageDeform_ApplyPlanarDent(void)
{
    int *pRecord = (int *)(g_unk0x00588b94 + ((Car *)g_stageDeformCar)->index * 0x4d0);
    if (FixVecDot(&g_stageDeformNormal, &g_stageDeformOffset) >= 0)
        FixVecScale(&g_stageDeformNormal, &g_stageDeformNormal, -0x10000);

    FixVector delta;
    FixVecScale(&delta, &g_stageDeformNormal, g_stageDeformRadius);
    // 0x467f4a: the scaled normal is ADDED to the impact point
    g_stageDeformOffset.x += delta.x;
    g_stageDeformOffset.y += delta.y;
    g_stageDeformOffset.z += delta.z;

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
                // Both branches leave the displacement in `distance` (one
                // temporary in the original, ebp-0x18); the shell branch used
                // to add an uninitialised vector to the vertex.
                if (radiusSquare >= distanceSquare) {
                    int penetration = g_stageDeformSpeed - FixMul(reciprocalRadius,
                                                                  distanceSquare);
                    FixVecScale(&distance, &g_stageDeformNormal, penetration);
                } else {
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
                }

                FixVector position;
                position.x = originalX + distance.x;
                position.y = originalY + distance.y;
                position.z = originalZ + distance.z;
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
                Scene_MarkShadowPartDirty(((Car *)g_stageDeformCar)->pNode0x720, pMesh);
            }
        }
    }
}

struct Car *Car_Get(int index);
struct Unk0x0052ebc0 *RallyData_GetDriverGroupRecord(int index);

// Copies the car's stage timing record (split times and penalties) into its
// rally record.
// FUNCTION: CMR2 0x00469b50
void StageTiming_CopyCarTimesToRallyRecord(int index)
{
    Car *pCar;
    BYTE *pTiming;
    BYTE *pRecord;

    pCar = Car_Get(index);
    pTiming = g_unk0x00588b98 + pCar->index * 0x290;
    pRecord = (BYTE *)RallyData_GetDriverGroupRecord(StageUI_GetRaceEndEventCount() + pCar->index);
    memcpy(pRecord, pTiming + 0x106, 0x106);
    memcpy(pRecord + 0x108, pTiming + 0x24c, 0x40);
}

// GLOBAL: CMR2 0x00542630
BYTE g_unk0x00542630[0x24 * 32];

// Clears the stage file table and registers its release callback.
// FUNCTION: CMR2 0x00456bb0
void StageTiming_InitStageFileTable(void)
{
    void **p = &g_unk0x00542ae8[0].field_0x4;
    int count;
    do {
        count = 2;
        do {
            p[-1] = NULL;
            p[0] = NULL;
            p[1] = NULL;
            p += 3;
        } while (--count);
    } while ((int)p < (int)&g_unk0x00542c6c);
    CGame::RegisterCallback(StageTiming_ReleaseStartArchiveBuffers, 0);
}

// FUNCTION: CMR2 0x00456be0
BYTE *StageTiming_GetStartTableRecord(int index)
{
    return g_unk0x00542630 + index * 0x24;
}



// GLOBAL: CMR2 0x00590c20
PartState *g_partState;
// GLOBAL: CMR2 0x00590d74
Car *g_partCar;

// Rotates the basis kept at 0x17c with the given angles and writes the three
// axes into the object's matrix.
// FUNCTION: CMR2 0x00481560
void StageTiming_RotatePartBasisFromAngles(unsigned short *pAngles)
{
    FixBasis basis;

    memcpy(&basis, &g_partState->basis, sizeof(basis));
    if (g_partCar->speed > 0x7ae)
        FixBasis_Rotate(&basis, pAngles);
    FixMatrix_SetRight(&basis.right, g_partState->pWorld);
    FixMatrix_SetUp(&basis.up, g_partState->pWorld);
    FixMatrix_SetForward(&basis.forward, g_partState->pWorld);
}

// Integrates the motion offset and writes the corrected world position.
// FUNCTION: CMR2 0x004815e0
void VehicleMotion_UpdateWorldPosition(void)
{
    FixVector delta;

    FixVecScale(&delta, &g_partState->velocity, g_physicsTimeStep);
    g_partState->correction.x += delta.x;
    g_partState->correction.y += delta.y;
    g_partState->correction.z += delta.z;

    FixMatrix_RotateVector(&delta, &g_partState->position,
                           g_partState->pWorld);
    delta.x = g_partState->position.x - delta.x;
    delta.y = g_partState->position.y - delta.y;
    delta.z = g_partState->position.z - delta.z;
    delta.x += g_partState->correction.x;
    delta.y += g_partState->correction.y;
    delta.z += g_partState->correction.z;
    FixMatrix_SetPosition(&delta, g_partState->pWorld);
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
void StageTiming_ResetCheckpointSlotStates(void)
{
    int i;

    for (i = 0; i < 8; i++)
        g_unk0x00542f58[i] = -1;
    for (i = 0; i < 8; i++)
        g_unk0x00542f78[i] = 0;
}

// FUNCTION: CMR2 0x00459390
bool StageTiming_IsClockOverlayActive(void)
{
    return g_unk0x00543098 != 0;
}

// GLOBAL: CMR2 0x00537f30
int g_unk0x00537f30;
// GLOBAL: CMR2 0x00538120
BYTE g_unk0x00538120;

extern char g_str0x0051a904[];
int GameInfo_GetSessionField397C(void);
int GameInfo_GetSessionField398C(void);
int NetPlayers_GetInputFrameDelta(void);

// Draws the stage clock and the countdown timer: picks the timer state from the
// game info flags, then prints it on the HUD with the seconds text and the
// minutes:seconds text.
// FUNCTION: CMR2 0x004593a0
void StageTiming_DrawClockAndCountdown(void)
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
    if ((**(unsigned int **)(StageUI_GetRaceResultTable() + 4) & 0xff) >= 10)
        return;
    if (CGameInfo::GetConfiguredGameMode() == '\n' || CGameInfo::GetConfiguredGameMode() == '\f') {
        if (GameInfo_GetSessionField397C() != 0) {
            delta = CMain::GetFrameDelta() - NetPlayers_GetInputFrameDelta();
            limit = GameInfo_GetSessionField397C() * 6000;
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
    if ((CGameInfo::GetConfiguredGameMode() == '\b' || CGameInfo::GetConfiguredGameMode() == '\t' ||
         CGameInfo::GetConfiguredGameMode() == '\v') &&
        g_unk0x00538120 != 0) {
        delta = CMain::GetFrameDelta() - g_unk0x00537f30;
        value = GameInfo_GetSessionField398C() * 100;
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
    if ((**(unsigned int **)(StageUI_GetRaceResultTable() + 4) & 0xff) < 10) {
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
// Degrees -> radians factors of the object yaw angles (StageObject_YawMainAndSunNodes).
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

bool NetPlayers_HasPlayerWithField8(int value);
int NetPlayers_FindPlayerByField8(int value);
char *NetPlayers_GetPlayerName(int index);
int NetRace_GetPlayerStatisticsValue(int index);
int View_GetActiveCameraMode(BYTE index);
void StageTiming_SetNodeTreeMeshAlpha(SceneNode *pNode, int alpha, BYTE checkFlag);
void StageTiming_ProjectPointToTenUnitRadius(int *param1, int *param2, int *param3);

// Square of a float expression; the original expands it twice.
#define FSQR(x) ((x) * (x))

// Fade factors of StageTiming_DrawProjectedCarMarker, expanded where they are used (the original
// computes each one once and keeps it in an x87 scratch slot).
#define TIMER_ALPHA ((distance - g_unk0x00511378) * g_unk0x005113d8)
#define TIMER_LEVEL (g_netOne - (distance - g_unk0x00511378) * g_unk0x005113d0)

// Projects a car's body node into one player's view and draws the stage timing
// marker (driver name or rally record) at the projected position, then updates
// the light level of the car's shadow meshes from the distance to the view
// centre.
// match 73%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00459790
void StageTiming_DrawProjectedCarMarker(int param_1, int param_2)
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
    StageTiming_ProjectPointToTenUnitRadius((int *)&nodePos, (int *)&viewPos, (int *)&out);
    FixMatrix_ProjectWorldPointToView(proj, &out, (BYTE *)pView);
    if (CGameInfo::GetConfiguredGameMode() == '\b' || CGameInfo::GetConfiguredGameMode() == '\t' ||
        CGameInfo::GetConfiguredGameMode() == '\n')
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
        StageTiming_ProjectPointToTenUnitRadius((int *)&up, (int *)&viewPos, (int *)&out);
        FixMatrix_ProjectWorldPointToView(proj, &out, (BYTE *)pView);
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
                    if (NetPlayers_HasPlayerWithField8(param_1)) {
                        pName = NetPlayers_GetPlayerName(NetPlayers_FindPlayerByField8(param_1));
                        if (pName != NULL) {
                            if (NetRace_GetPlayerStatisticsValue(param_1) != 0) {
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
            if (CGameInfo::GetConfiguredGameMode() == '\b' || CGameInfo::GetConfiguredGameMode() == '\t' ||
                CGameInfo::GetConfiguredGameMode() == '\n') {
                if (distance > g_unk0x00511378 && param_1 > 0) {
                    if (TIMER_LEVEL >= g_netOne)
                        g_unk0x00542f78[param_1] = 0xff;
                    else if (TIMER_LEVEL <= g_netZero)
                        g_unk0x00542f78[param_1] = 0;
                    else
                        g_unk0x00542f78[param_1] = (int)(TIMER_LEVEL * g_netByteScale);
                    if (View_GetActiveCameraMode(param_2) == 7)
                        g_unk0x00542f78[param_1] = 0x80;
                    if (g_unk0x00542f78[param_1] != g_unk0x00542f58[param_1]) {
                        StageTiming_SetNodeMeshAlpha(Car_Get(param_1)->pNode0x71c,
                                     (BYTE)g_unk0x00542f78[param_1], 0);
                        StageTiming_SetNodeTreeMeshAlpha(Car_Get(param_1)->pNode0x71c->pFirstChild,
                                     (BYTE)g_unk0x00542f78[param_1], 0);
                        StageTiming_SetNodeMeshAlpha(Car_Get(param_1)->pNode0x720,
                                     (BYTE)g_unk0x00542f78[param_1], 0);
                        StageTiming_SetNodeTreeMeshAlpha(Car_Get(param_1)->pNode0x720->pFirstChild,
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
            StageTiming_SetNodeMeshAlpha(Car_Get(param_1)->pNode0x71c, 0, 0);
            StageTiming_SetNodeTreeMeshAlpha(Car_Get(param_1)->pNode0x71c->pFirstChild, 0, 0);
            StageTiming_SetNodeMeshAlpha(Car_Get(param_1)->pNode0x720, 0, 0);
            StageTiming_SetNodeTreeMeshAlpha(Car_Get(param_1)->pNode0x720->pFirstChild, 0, 0);
            g_unk0x00542f58[param_1] = 0;
        }
    }
}

// Vector helper of StageTiming_DrawProjectedCarMarker that is still to be decompiled. Empty body with
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
void StageTiming_ProjectPointToTenUnitRadius(int *pPoint, int *pOrigin, int *pOut)
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
void StageObject_DestroyStageKindCarNodes(void);
extern SceneNode *g_unk0x00547fec;
extern SceneNode *g_unk0x00547ff0;

// Releases every stage timing resource: the scene nodes hanging from the
// per-car timing records, the four scene node tables and the eight file
// buffers of the fin table.
// FUNCTION: CMR2 0x00457ed0
int StageTiming_FreeSceneAndFinishResources(void)
{
    int i;
    BYTE *pRecord;
    int *pNodeList;
    void **pBuffer;

    StageObject_DestroyStageKindCarNodes();
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
    CGraphics::ReleaseCachedTextureSurfaces();
    Scene_FreeShadowCasters();
    return 1;
}

// Registered callback with nothing to release.
// FUNCTION: CMR2 0x00458040
int StageTiming_NoOpResourceRelease(void)
{
    return 1;
}

// FUNCTION: CMR2 0x00458230
BYTE StageTiming_GetCheckpointField16(int index)
{
    return g_unk0x00542e78[index].field_0x16;
}

// FUNCTION: CMR2 0x00458250
BYTE StageTiming_GetCheckpointField17(int index)
{
    return g_unk0x00542e78[index].field_0x17;
}

// FUNCTION: CMR2 0x00458270
BYTE StageTiming_GetCheckpointField18(int index)
{
    return g_unk0x00542e78[index].field_0x18;
}

// FUNCTION: CMR2 0x00458290
BYTE StageTiming_GetCheckpointField19(int index)
{
    return g_unk0x00542e78[index].field_0x19;
}

// FUNCTION: CMR2 0x004582b0
BYTE StageTiming_GetCheckpointField1A(int index)
{
    return g_unk0x00542e78[index].field_0x1a;
}

// FUNCTION: CMR2 0x004582d0
int StageTiming_GetCheckpointField2(int index)
{
    return g_unk0x00542e78[index].field_0x2;
}

// FUNCTION: CMR2 0x004582f0
int StageTiming_GetCheckpointField0(int index)
{
    return g_unk0x00542e78[index].field_0x0;
}

// FUNCTION: CMR2 0x00458310
int StageTiming_GetCheckpointField12(int index)
{
    return g_unk0x00542e78[index].field_0x12;
}

// FUNCTION: CMR2 0x00458330
int StageTiming_GetCheckpointGroupIndex(int index)
{
    return g_unk0x00542e78[index].field_0x4;
}

// FUNCTION: CMR2 0x00458350
int StageTiming_GetCheckpointSplitIndex(int index)
{
    return g_unk0x00542e78[index].field_0x6;
}

// FUNCTION: CMR2 0x00458370
int StageTiming_GetCheckpointField14(int index)
{
    return g_unk0x00542e78[index].field_0x14;
}

// FUNCTION: CMR2 0x004589e0
short StageTiming_GetCheckpointField10(int index)
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
void StageTiming_ResetWeatherSelectionState(void)
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
void StageTiming_SetPrimaryWheelTrailTexture(int texture, int side, int car)
{
    if (car < 8)
        g_trailTextureA[car][side] = texture;
}

// FUNCTION: CMR2 0x0045b530
void StageTiming_SetSecondaryWheelTrailTexture(int texture, int side, int car)
{
    if (car < 8)
        g_trailTextureB[car][side] = texture;
}

// FUNCTION: CMR2 0x0045b550
void StageTiming_ForceWheelTrailSurface(int car, int surface)
{
    if (car < 8) {
        g_trailForced[car] = 1;
        g_trailForcedSurface[car] = surface;
    }
}

BYTE StageUI_GetRaceEndEventCount(void);
BYTE *RallyData_GetTyreRecord(BYTE index);

// Value `index` of the tyre record `offset` places after the current one.
// FUNCTION: CMR2 0x0045c720
int StageTiming_GetRelativeTyreRecordValue(char offset, int index)
{
    BYTE *pRecord;

    pRecord = RallyData_GetTyreRecord((BYTE)(StageUI_GetRaceEndEventCount() + offset));
    if (pRecord != NULL)
        return *(int *)(pRecord + 0x90 + index * 4);
    return 0;
}

char *Race_GetStagePathBuffer38340(void);

// GLOBAL: CMR2 0x0051a12c
char g_strBflFormat[] = "%s.bfl";

// Loads the <stage>.bfl archive into stage file 3.
// FUNCTION: CMR2 0x00455260
void StageTiming_LoadStageBFLArchive(void)
{
    sprintf(CFrontend::m_stringDest, g_strBflFormat, Race_GetStagePathBuffer38340());
    CGenericFileLoader::LoadIntoFileRecord((GenericFile *)StageTiming_GetStageFile3(), CFrontend::m_stringDest);
}

// Default sky/light vector and cleared offsets.
// FUNCTION: CMR2 0x0045eea0
void StageTiming_ResetSkyLightVectors(void)
{
    g_unk0x00547930.x = 0xb4fd;
    g_unk0x00547930.y = 0;
    g_unk0x00547930.z = -0xb4fd;
    g_unk0x00547940 = 0;
    g_unk0x00547944.x = 0;
    g_unk0x00547944.y = 0;
    g_unk0x00547944.z = 0;
}

// Applies StageTiming_SetNodeMeshAlpha to a node list and all descendants.
// FUNCTION: CMR2 0x00465f20
void StageTiming_SetNodeTreeMeshAlpha(SceneNode *pNode, int alpha, BYTE checkFlag)
{
    for (; pNode != NULL; pNode = pNode->pNext) {
        StageTiming_SetNodeMeshAlpha(pNode, alpha, checkFlag);
        if (pNode->pFirstChild != NULL)
            StageTiming_SetNodeTreeMeshAlpha(pNode->pFirstChild, alpha, checkFlag);
    }
}

// Thresholds (first/second-hit window speeds) for the high/front and low/rear
// halves of a car's eight timing parts.
// GLOBAL: CMR2 0x0051bfac
int g_unk0x0051bfac[8] = { 0x3333, 0x3333, 0x3333, 0x3333, 0x1999, 0x1999, 0x1999, 0x1999 };
// GLOBAL: CMR2 0x0051bfcc
int g_unk0x0051bfcc[8] = { 0x1999, 0x1999, 0x1999, 0x1999, 0x1999, 0x1999, 0x1999, 0x1999 };

void StageObject_SetModelSubmeshVisibility(int param_1, int param_2, char param_3);
void Race_PlayHornSound(unsigned int view, int kind, int listener);
void Car_QueueWindowBreak(BYTE *pParts, Car *pCar, unsigned int part);

// Flags window/body breaks for the eight parts of a car's timing record.
// FUNCTION: CMR2 0x004692f0
void StageTiming_FlagCarPartBreaks(Car *pCar, int param_2)
{
    BYTE *pRecord;
    BYTE i;
    BYTE part;
    int carIndex;
    unsigned int state;

    pRecord = (BYTE *)(pCar->index * 0x4d0 + (int)g_unk0x00588b94);
    if (*(char *)((int)StageTiming_GetStartTableRecord(pCar->index) + 0x20) == 'C' ||
        *(char *)((int)StageTiming_GetStartTableRecord(pCar->index) + 0x20) == 'A') {
        i = 0;
        for (; i < 8; i++) {
            part = pRecord[0x460 + i];
            if (((int *)(pRecord + 0x490))[i] == 0 || ((int *)(pRecord + 0x490))[i + (-8)] == 0) {
                if (((int *)(pRecord + 0x490))[i + (-0x7a)] > g_unk0x0051bfac[i] && ((int *)(pRecord + 0x490))[i] == 0) {
                    StageObject_SetModelSubmeshVisibility((int)pCar, 2, i);
                    ((int *)(pRecord + 0x490))[i] = 1;
                    ((int *)(pRecord + 0x490))[i + (-8)] = 1;
                    if (param_2 == 0) {
                        Car_QueueWindowBreak(pRecord, pCar, part);
                        carIndex = pCar->index;
                        state = RallyDataState();
                        if (carIndex < (int)(state & 0xff)) {
                            switch (i) {
                            case 0:
                            case 1:
                            case 2:
                            case 3:
                                Race_PlayHornSound(carIndex, 2, carIndex);
                                break;
                            case 4:
                            case 5:
                            case 6:
                            case 7:
                                Race_PlayHornSound(carIndex, 0, carIndex);
                                break;
                            }
                        }
                    }
                }
                else if (((int *)(pRecord + 0x490))[i + (-0x7a)] > g_unk0x0051bfcc[i] && ((int *)(pRecord + 0x490))[i + (-8)] == 0 && ((int *)(pRecord + 0x490))[i] == 0) {
                    StageObject_SetModelSubmeshVisibility((int)pCar, 1, i);
                    ((int *)(pRecord + 0x490))[i + (-8)] = 1;
                    if (param_2 == 0) {
                        carIndex = pCar->index;
                        state = RallyDataState();
                        if (carIndex < (int)(state & 0xff)) {
                            switch (i) {
                            case 0:
                            case 1:
                            case 2:
                            case 3:
                                Race_PlayHornSound(carIndex, 2, carIndex);
                                break;
                            case 4:
                            case 5:
                            case 6:
                            case 7:
                                Race_PlayHornSound(carIndex, 0, carIndex);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

// Index of the part of a car model whose node type byte is `type` (-1 none).
// FUNCTION: CMR2 0x004692b0
int StageTiming_FindModelPartByNodeType(unsigned int type, BYTE *pModel)
{
    int i;
    SceneNode **ppNode;

    for (i = 0; i < *(int *)(pModel + 0x45c); i++) {
        ppNode = (SceneNode **)(pModel + 0x3c) + i;
        if (((*ppNode)->flags & 0xff) == type)
            return i;
    }
    return -1;
}

// Reads the two values at +0x4a0 of the car's 0x4d0-byte record.
// FUNCTION: CMR2 0x00466870
void StageTiming_ReadCarReplayTailValues(int *pA, int *pB, Car *pCar)
{
    *pA = *(int *)(g_unk0x00588b94 + pCar->index * 0x4d0 + 0x4a0);
    *pB = *(int *)(g_unk0x00588b94 + pCar->index * 0x4d0 + 0x4a4);
}

// Clears `count` stage records (0x4d0 bytes) from `first`.
// FUNCTION: CMR2 0x004669b0
void StageTiming_ClearCarReplayRecordRange(int first, int count)
{
    int i;

    for (i = first; i < count + first; i++)
        memset(g_unk0x00588b94 + i * 0x4d0, 0, 0x4d0);
}

// GLOBAL: CMR2 0x00542d58
short g_unk0x00542d58[8];
// GLOBAL: CMR2 0x00542d68
short g_unk0x00542d68[8];

int Stage_GetPreviousCheckpoint(int index);
int RallyData_GetActiveCarRaceRecordField0(BYTE *p);

// Stores, per car, the checkpoint before its current one and its record's
// field 0x12.
// FUNCTION: CMR2 0x00458b80
void StageTiming_StorePreviousCarCheckpoints(void)
{
    int i;

    for (i = 0; i < g_unk0x00542c68; i++) {
        g_unk0x00542d58[i] = Stage_GetPreviousCheckpoint(RallyData_GetActiveCarRaceRecordField0((BYTE *)Car_Get(i)));
        g_unk0x00542d68[i] = g_unk0x00542e78[i].field_0x12;
    }
}

unsigned int RallyData_GetSetupModeBits(void);
unsigned int RallyData_GetSetupFlag11(void);

extern int g_unk0x00542c74;
extern char g_unk0x00542cad;
extern int g_unk0x00542cb0;
extern int g_unk0x00542cb4[8];

int Stage_GetNextCheckpoint(int index);
void RallyData_SetCarRaceRecordPosition(Car *pCar, int value);
void StageTiming_SetViewRouteDistanceLimit(BYTE player, unsigned int node, int dir);

// Resets one player's timing record (table at 0x542e78, stride 0x1c) to the
// default state for the current rally and network mode.
// match 88%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00458bd0
void StageTiming_ResetPlayerLapRecord(int param_1, int param_2, int param_3, char param_4)
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
        g_unk0x00542e78[param_1].field_0x0 = (short)RallyData_GetActiveCarRaceRecordField0((BYTE *)pCar);
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
        g_unk0x00542e78[param_1].field_0x0 = (short)Stage_GetPreviousCheckpoint(RallyData_GetActiveCarRaceRecordField0((BYTE *)pCar));
    }
    else {
        value = g_unk0x00542d58[param_1];
        g_unk0x00542e78[param_1].field_0x0 = value;
        RallyData_SetCarRaceRecordPosition(pCar, Stage_GetNextCheckpoint((int)value));
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
        if (CGameInfo::GetGameModeOptionBit19() != 0) {
            if (g_unk0x00542cb4[0] != 0 && CGameInfo::GetConfiguredGameMode() != 10)
                goto d8e;
        }
        else {
            if (param_1 != g_unk0x00542cb0 && CGameInfo::GetConfiguredGameMode() != 3)
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
    StageTiming_SetViewRouteDistanceLimit((BYTE)param_1, (int)g_unk0x00542e78[param_1].field_0x0, 1);
}

// Flags the record when its two positions coincide (not in some network modes).
// FUNCTION: CMR2 0x00459180
void StageTiming_FlagCoincidentCheckpointPositions(int index)
{
    if (g_unk0x00542e78[index].field_0x4 == g_unk0x00542e78[index].field_0xc &&
        g_unk0x00542e78[index].field_0x6 == g_unk0x00542e78[index].field_0xe) {
        g_unk0x00542e78[index].field_0x1a = 1;
        if ((BYTE)RallyData_GetSetupFlag11() && RallyData_GetSetupModeBits() == 1)
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
// FUNCTION: CMR2 0x00480980
void StageTiming_ResetCarPartRecordsAndSlots(void)
{
    void **pp;
    int i;
    int offset;

    for (pp = g_unk0x00590d7c; (int)pp < (int)(g_unk0x00590d7c + 4); pp++) {
        i = 0;
        if (g_unk0x00590c64 > 0) {
            offset = 0;
            do {
                memset((BYTE *)*pp + offset, 0, 0x1a0);
                i++;
                offset += 0x1a0;
            } while (i < g_unk0x00590c64);
        }
    }
    memset(g_unk0x00590b7c, 0, sizeof(g_unk0x00590b7c));
    memset(g_unk0x00590c24, 0xff, sizeof(g_unk0x00590c24));
}

// Caches, per car in the list, pointers into its timing record.
// FUNCTION: CMR2 0x004809e0
void StageTiming_CacheListedCarTimingPointers(short *pList, short count)
{
    int i;
    Car *pCar;
    int p;

    for (i = count - 1; i >= 0; i--) {
        pCar = Car_Get(pList[i]);
        p = StageTiming_GetStartArchiveRelativeEntry((BYTE *)pCar, 3);
        g_unk0x00590b5c[pCar->index] = p;
        p += 0xc;
        g_unk0x00590b30[pCar->index] = p;
        p += 4;
        g_unk0x00590c00[pCar->index] = p;
    }
}

extern int g_unk0x00542cb4[8];
extern int g_stageCheckpointCount;
int NetPlayers_GetPlayerField8(int index);
extern double g_unk0x005113b8;

// Advances a player's lap counter by half the checkpoint count (with the
// fractional part kept in *pFrac), unless the player has finished.
// FUNCTION: CMR2 0x004591e0
void StageTiming_AdvancePlayerHalfLapCounter(int player, int *pCount, int *pFrac)
{
    int slot;
    float count;
    float value;

    if (player == -2)
        slot = 0;
    else
        slot = NetPlayers_GetPlayerField8(player);
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
void StageTiming_ResetWheelWearRecord(int player, int wheel)
{
    BYTE *p = RallyData_GetTyreRecord((BYTE)(StageUI_GetRaceEndEventCount() + player));

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
        RallyData_MarkTyresChanged((StageUI_GetRaceEndEventCount() & 0xff) + player);
        Tyre_AddWear(player, wheel, 0, 0);
    }
}

// GLOBAL: CMR2 0x00590c68
int g_unk0x00590c68;

// Turns the vehicle about its vertical axis by the per-frame rate, toward the
// side given by its orientation.
// match 76%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004814d0
void CarPart_IntegrateVerticalAxisTurn(void)
{
    unsigned short angles[3];

    if ((g_partState->flags & 0xf0) == 0x20) {
        int direction = *(int *)&g_partState->basis;
        angles[0] = 0;
        angles[1] = 0;
        if (direction < 0)
            angles[2] = (unsigned short)g_unk0x00590c68;
        else
            angles[2] = (unsigned short)-g_unk0x00590c68;
    } else {
        int direction = *(int *)&g_partState->basis;
        angles[0] = 0;
        angles[1] = 0;
        if (direction > 0)
            angles[2] = (unsigned short)g_unk0x00590c68;
        else
            angles[2] = (unsigned short)-g_unk0x00590c68;
    }
    StageTiming_RotatePartBasisFromAngles(angles);
    VehicleMotion_UpdateWorldPosition();
}

// One table: 0x592744 is slot 0 and the eight per-variant entries follow
// (callers index it with a 1-based variant).
// GLOBAL: CMR2 0x00592744
int g_unk0x00592744[9];
#define g_unk0x00592748 (g_unk0x00592744 + 1)

void RallyData_GetRouteNodeGroundPosition(int index, int *pOut);
int RallyData_GetRouteAvailabilityState(void);

// Lays out count 0x1720-byte car records from p and offsets their route
// points by the route origin; returns the end of the records.
// match 67%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00498590
BYTE *StageTiming_RelocateCarRecordRoutePoints(BYTE *p, int unused, int count)
{
    int origin[3];
    int points;
    int *pp;
    int i;

    RallyData_GetRouteNodeGroundPosition(0, origin);
    points = RallyData_GetRouteAvailabilityState();
    for (pp = g_unk0x00592748; count >= 1; count--, pp++) {
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
void StageTiming_CollectDescriptorCarValues(char *pDesc, int *pValues, int *pOut)
{
    int i;
    int value;

    int *p;
    i = 1;
    if (pDesc[0xc] >= 1) {
        p = pOut + 1;
        do {
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
            *p = value;
            i++;
            p++;
        } while (i <= pDesc[0xc]);
    }
}

// GLOBAL: CMR2 0x0053d1d9
BYTE g_unk0x0053d1d9;

// Advances the stage clock by 4 with a little jitter, stopping at one hour.
// match 75%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00448de0
void StageTiming_AdvanceJitteredClock(void)
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
        if ((BYTE)RallyData_GetSetupFlag11()) {
            g_unk0x0053d1a0 += 4;
            g_unk0x0053d1a2 = g_unk0x0053d1a0 + (short)jitter;
        }
    }
}

int StageUI_GetRaceResultValue(void);

// Copies the driver slots into the split display table for the current view mode.
// FUNCTION: CMR2 0x00455620
void StageTiming_RebuildSplitDisplaySlots(void)
{
    int count = (BYTE)CGameInfo::GetConfiguredPlayerCount();
    int mode = StageUI_GetRaceResultValue();
    int i;

    if (mode >= 0) {
        if (mode > 1) {
            if (mode == 4) {
                for (i = 0; i < CGameInfo::GetConfiguredPlayerCount(); i++) {
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
void StageTiming_CountLapCrossings(int car, int from, int to)
{
    if (to < g_unk0x00542e78[car].field_0x0 && to == 0 && from == g_stageCheckpointCount - 1) {
        g_unk0x00542e78[car].field_0x2++;
        g_unk0x00542e78[car].field_0x18 = 1;
        if (g_unk0x00542e78[car].field_0x2 == 1000)
            g_unk0x00542e78[car].field_0x2 = 0;
    }
    if (to > g_unk0x00542e78[car].field_0x0 && to == g_stageCheckpointCount - 1 && from == 0) {
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
BYTE View_GetActiveCameraFlags(BYTE index);
int RallyData_GetRouteAdditionalState(void);

// Scales a view's object value by the car's route position between two limits.
// match 84%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0045f530
void StageObject_ScaleViewValueByRoute(BYTE *pObject, int view)
{
    unsigned int position;

    if (RallyData_GetRouteAdditionalState() == 0) {
        position = RallyData_GetActiveCarRaceRecordField0((BYTE *)Car_Get(View_GetActiveCameraFlags(view)));
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
void StageTiming_ResetScaledViewObjectStates(void)
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
void CarTyres_CommitPlayerWear(int a, int b, int count)
{
    int i;
    int j;
    BYTE *pRecord;
    int *w;

    for (i = 0; i < count; i++) {
        pRecord = RallyData_GetTyreRecord((BYTE)(StageUI_GetRaceEndEventCount() + i));
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
                RallyData_MarkTyresChanged((StageUI_GetRaceEndEventCount() & 0xff) + i);
                Tyre_AddWear(i, j, 0, 0);
            }
        }
    }
}

// Resets a car's replay recording record.
// FUNCTION: CMR2 0x00466920
void StageTiming_ResetCarReplayRecord(BYTE *p)
{
    int i;

    p[0x20a] = 0;
    p[0x20b] = 0;
    for (i = 0; i < 20; i++)
        p[0x112 + i * 0xd] = 0xff;
    memset(p + 0x24c, 0, 0x22);
    for (i = 0; i < 4; i++)
        ((int *)(p + 0x270))[i] = 0;
    for (i = 0; i < 3; i++)
        ((int *)(p + 0x280))[i] = 0;
    memcpy(p, p + 0x106, 0x106);
    memcpy(p + 0x20c, p + 0x24c, 0x40);
}

int Stage_GetDriverCount(void);

// One bubble pass over the running order, keeping each car's position count.
// match 73%: only the tail's edx/ebx register choice differs from the original
// FUNCTION: CMR2 0x00448d50
void StageTiming_BubbleRunningOrder(void)
{
    int count = Stage_GetDriverCount();
    int i;
    char a;
    int b;
    char *p;

    for (i = 1; i < count; i++) {
        p = &g_unk0x0053dda8[i];
        if (StageTiming_CompareCheckpointAndStartOrder(p[0], p[-1]) == 1) {
            a = p[-1];
            g_carStageTiming[a].field_0x81++;
            b = p[0];
            g_carStageTiming[b].field_0x81--;
            p[0] = a;
            p[-1] = b;
        }
    }
}

void FixMatrix_Interpolate(FixMatrix *pOut, FixMatrix *pA, FixMatrix *pB, int tRight, int tAxis, int tPos, int mode);
short Car_GetOrderCount(void);
short *Car_GetOrder(void);

// Interpolates, for every car and each of its four moving parts, the part's
// matrix between its two keys and applies it to the part's node.
// FUNCTION: CMR2 0x00484d30
void StageTiming_InterpolateMovingPartMatrices(int t)
{
    int count = Car_GetOrderCount();
    short *pOrder = Car_GetOrder();
    short *p;
    int index;
    void **pp;
    BYTE *pPart;

    count--;
    if (count < 0)
        return;
    p = pOrder + count;
    count++;
    do {
        index = (signed char)Car_Get(*p)->index;
        for (pp = &g_unk0x00590d7c[3]; (int)pp >= (int)g_unk0x00590d7c; pp--) {
            pPart = (BYTE *)*pp + index * 0x1a0;
            g_partState = (PartState *)pPart;
            if (*(int *)pPart != 0 && (pPart[0x150] & 1) != 0) {
                FixMatrix_Interpolate((FixMatrix *)(pPart + 0x8c), (FixMatrix *)(pPart + 0x4c), (FixMatrix *)(pPart + 0xc),
                                      t, t, t, 0);
                FixMatrix_CopyRotation(&g_partState->drawMatrix, (FixMatrix *)(*(BYTE **)g_partState + 0x98));
            }
        }
        p--;
        count--;
    } while (count != 0);
}

// GLOBAL: CMR2 0x00590d78
CarPartSet *g_partSet;
extern BYTE g_unk0x00590c60[4];

void StageObject_SetReplayEntryHitFlag(Car *pCar, int index);

// Clears the low bit of the vehicle's part-state byte and re-selects the part
// index held in its high nibble.
// FUNCTION: CMR2 0x00483010
void StageTiming_ClearPartFlagAndReselectIndex(void)
{
    g_partState->flags &= 0xfe;
    StageObject_SetReplayEntryHitFlag(g_partCar, g_partState->flags >> 4);
}

// Starts a part's swing when the load on its side exceeds 0.8: the swing
// speed (+0x11c) is added or removed depending on which wheel is loaded more.
// FUNCTION: CMR2 0x00483050
void StageTiming_StartLoadedPartSwing(void)
{
    int idx;
    int a;
    int b;

    idx = g_unk0x00590c60[g_partState->flags >> 4];
    if (g_partSet->field_0x240[idx] > 0xcccc) {
        if ((g_partState->flags & 0xf0) == 0x10) {
            a = 2;
            b = 3;
        } else {
            a = 0;
            b = 1;
        }
        if (g_partSet->field_0x240[a] > g_partSet->field_0x240[b])
            g_partState->position.z -= g_partState->swingAxis.z;
        else
            g_partState->position.z += g_partState->swingAxis.z;
        g_partState->flags |= 2;
        g_partState->angle = 0;
    }
}

// Cooldown timestamps for the two local players.
// GLOBAL: CMR2 0x00588a80
int g_deformImpactTicks[2];
// GLOBAL: CMR2 0x00588a88
int g_deformPulseTicks[2];

// Resets every car's replay recording record.
// FUNCTION: CMR2 0x004668d0
void StageTiming_ResetAllCarReplayRecords(void)
{
    int i;

    for (i = g_unk0x00588a90 - 1; i >= 0; i--)
        StageTiming_ResetCarReplayRecord(g_unk0x00588b98 + i * 0x290);
    for (i = 0; i < 2; i++)
        g_deformPulseTicks[i] = 0;
    for (i = 0; i < 2; i++)
        g_deformImpactTicks[i] = 0;
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
void StageTiming_InitStageTypeEvents(int unused, int slot, int type)
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
// FUNCTION: CMR2 0x0045f300
void StageTiming_SetPrimaryObjectRamp(int v1, int v2, int from, int to, int initial)
{
    int i;
    int *p;

    g_unk0x00543d60 = v1;
    g_unk0x00543d64 = v2;
    g_unk0x00543d70 = to;
    g_unk0x00543d6c = from;
    g_unk0x00543d68 = (to - from) << 16;
    if (g_unk0x00543d68 > 0)
        g_unk0x00543d68 = FixDiv(0x10000, g_unk0x00543d68);
    g_unk0x00543d68 = FixMul(g_unk0x00543d68, g_unk0x00543d64 - g_unk0x00543d60);
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
// FUNCTION: CMR2 0x0045f3d0
void StageTiming_SetSecondaryObjectRamp(int v1, int v2, int from, int to)
{
    int i;
    int *p;

    g_unk0x00543d88 = v1;
    g_unk0x00543d98 = to;
    g_unk0x00543d8c = v2;
    g_unk0x00543d94 = from;
    g_unk0x00543d90 = (to - from) << 16;
    if (g_unk0x00543d90 > 0)
        g_unk0x00543d90 = FixDiv(0x10000, g_unk0x00543d90);
    g_unk0x00543d90 = FixMul(g_unk0x00543d90, g_unk0x00543d8c - g_unk0x00543d88);
    i = 0;
    if (g_unk0x00547acc > 0) {
    do {
        p = (int *)((BYTE *)g_unk0x00543eb8 + i * 0x2c);
        p[1] = 0;
        p[0] = 0;
        int lower = g_unk0x00543d88;
        p[2] = lower;
        p[3] = 0;
        p[4] = lower;
        p[6] = lower;
        p[10] = 1;
        i++;
    } while (i < (int)(g_stageObjectCount.packed & 0xff));
    }
}

short *Car_GetOrder(void);

// Updates each car's third ramp value from its route position.
// match 60%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0045e7f0
void StageTiming_UpdateThirdObjectRampValues(void)
{
    int count;
    int n;
    int t;
    short *p;
    Car *pCar;
    unsigned int *pRec;
    unsigned int position;

    if (RallyData_GetRouteAdditionalState() == 0) {
        count = Car_GetOrderCount();
        p = Car_GetOrder();
        count--;
        if (count >= 0) {
            p += count;
            n = count + 1;
            do {
                pCar = Car_Get(*p);
                pRec = (unsigned int *)((BYTE *)g_unk0x00543ecc + pCar->index * 0xc);
                position = RallyData_GetActiveCarRaceRecordField0((BYTE *)pCar);
                pRec[0] = position;
                if (position != pRec[1]) {
                    pRec[1] = position;
                    if (position <= g_unk0x00543d80) {
                        pRec[2] = g_unk0x00543d74;
                    } else {
                        if (position >= g_unk0x00543d84) {
                            pRec[2] = g_unk0x00543d78;
                        } else {
                            // Assignment-to-local inside the shift reproduces the
                            // original's store order: pRec[2] first, then the
                            // FixMul argument home.
                            pRec[2] = (t = position - g_unk0x00543d80) << 16;
                            pRec[2] = FixMul(t << 16, g_unk0x00543d7c);
                            pRec[2] = pRec[2] + g_unk0x00543d74;
                        }
                    }
                }
                p--;
            } while (--n != 0);
        }
    }
}

short StageTiming_GetCheckpointField10(int index);
unsigned int RallyData_GetSelectionBits16To19(void);
int Timing_CentisecondsToFixedSeconds(int hundredths);

// Estimates the stage time from the progress so far (at least halfway).
// match 75%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00448550
int StageTiming_EstimateElapsedStageTime(void)
{
    int total;
    int progress;

    g_unk0x0053d1da[0] = 1;
    g_unk0x0053e18c++;
    if (CGameInfo::GetConfiguredGameMode() == 11)
        total = (BYTE)RallyData_GetSelectionBits16To19() * RallyData_GetRouteAvailabilityState() * 0x10000;
    else
        total = RallyData_GetRouteAvailabilityState() << 16;
    progress = FixDiv((unsigned short)StageTiming_GetCheckpointField10(0) << 16, total);
    if (progress > 0x8000) {
        return (g_unk0x0053d1b8[0] = ConvertRawTimeToCentiseconds(FixMul(Timing_CentisecondsToFixedSeconds(g_unk0x0053d1b0), FixDiv(0x10000, progress))));
    }
    return (g_unk0x0053d1b8[0] = g_unk0x0053d1b0 * 2);
}

BYTE *RallyData_GetAvailableRouteNodeRecord(int index);
void View_SetDistanceOverride(BYTE index, int value);
int Render_GetDetailDistanceScale(void);
int Race_IsMultiplayerRecordMode10(void);

// Sets the player's view distance from the route node's limits (forward or
// backward direction).
// FUNCTION: CMR2 0x00459250
void StageTiming_SetViewRouteDistanceLimit(BYTE player, unsigned int node, int dir)
{
    BYTE *pNode;
    int nearDist;
    int farDist;
    int distance;

    if (player < (BYTE)RallyDataState()) {
        if (dir < 0) {
            node++;
            if (node >= (unsigned int)RallyData_GetRouteAvailabilityState())
                node = RallyData_GetRouteAvailabilityState() - 1;
        }
        if ((BYTE)RallyDataState() > 1 && Race_IsMultiplayerRecordMode10() == 0)
            CGameInfo::IsConfiguredMultiplayer();
        pNode = RallyData_GetAvailableRouteNodeRecord(node);
        if (dir < 0) {
            nearDist = *(int *)(pNode + 0x1c);
        } else {
            nearDist = *(int *)(pNode + 0x24);
        }
        if (dir < 0)
            farDist = *(int *)(pNode + 0x20);
        else
            farDist = *(int *)(pNode + 0x28);
        if (farDist > 0) {
            *(int *)((BYTE *)g_pGraphics + 0x3c8) = farDist;
            distance = Render_GetDetailDistanceScale();
            if (distance > *(int *)((BYTE *)g_pGraphics + 0x3c8))
                distance = *(int *)((BYTE *)g_pGraphics + 0x3c8);
            View_SetDistanceOverride(player, distance);
        }
        if (nearDist > 0) {
            *(int *)((BYTE *)g_pGraphics + 0x3c4) = nearDist;
            distance = Render_GetDetailDistanceScale();
            if (distance > *(int *)((BYTE *)g_pGraphics + 0x3c8))
                distance = *(int *)((BYTE *)g_pGraphics + 0x3c8);
            View_SetDistanceOverride(player, distance);
        }
    }
}

void Stage_GetCarStartXZ(int car, int *pStarts, int *pOut);

// Start position of a car in the .csp start table.
// FUNCTION: CMR2 0x00456c70
int *StageTiming_GetCarStartPosition(int car)
{
    Stage_GetCarStartXZ(car, *(int **)(g_unk0x00542630 + 0x240), (int *)(g_unk0x00542630 + 0x248));
    return (int *)(g_unk0x00542630 + 0x248);
}

struct FixAngles;
SceneNode *Scene_CreateLight(int type, int r, int g, int b, FixVector *pPosition, FixAngles *pAngles, SceneNode *pParent);
void Scene_SetAmbient(BYTE *pColour, int boost);
void Scene_SetLight(FixVector *pLight, int boost);
int RallyData_GetChallengeRenderState(void);
extern SceneNode *g_stageAmbientNode;
extern BYTE g_unk0x00592146;
extern BYTE g_stageColourAlpha;

// Creates the stage light with full white ambient and directional light.
// match 55%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004918d0
void StageTiming_CreateWhiteStageLight(void)
{
    BYTE ambient[4];
    unsigned short angles[4];
    FixVector position;
    FixVector light;
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
                                           (SceneNode *)RallyData_GetChallengeRenderState());
    country = (char)RallyDataCountryIndex();
    if (country == 3) {
        Scene_SetAmbient(ambient, 0);
        Scene_SetLight(&light, 0);
    } else {
        Scene_SetAmbient(ambient, 1);
        Scene_SetLight(&light, 1);
    }
    g_unk0x00592146 = 0xff;
    g_stageColourAlpha = 0xff;
}

extern int g_unk0x00588970[8];

void StageObject_SetModelSubmeshVisibility(int param_1, int param_2, char param_3);
void Mesh_Rebuild(Mesh *pMesh);
void RallyData_ValidateIndex(int index);
void StageObject_ModifyDamageRecordFlagBytes(int index, char set0, char set1, BYTE mask);

// Rebuilds the part meshes of a damaged car from its packed vertices, then
// clears the part set's damage state.
// FUNCTION: CMR2 0x004698a0
void StageTiming_RebuildDamagedPartMeshes(int pCar)
{
    CarPartSet *set;
    int i;
    int j;
    Mesh *pMesh;
    FixVector pos;
    FixVector normal;

    if (g_unk0x00588970[*(char *)(pCar + 0xb1a)] != 0) {
        set = (CarPartSet *)(g_unk0x00588b94 + *(char *)(pCar + 0xb1a) * 0x4d0);
        for (i = 0; i < set->count; i++) {
            for (j = 0; j < set->vertexCount[i]; j++) {
                pos = set->vertices[i][j].pos;
                ((CarPartFloatVertex *)set->meshes[i]->pVertexData)[j].pos[0] = pos.x * CGraphics::m_oneOver65536;
                ((CarPartFloatVertex *)set->meshes[i]->pVertexData)[j].pos[1] = pos.y * CGraphics::m_oneOver65536;
                ((CarPartFloatVertex *)set->meshes[i]->pVertexData)[j].pos[2] = pos.z * CGraphics::m_oneOver65536;
                normal = set->vertices[i][j].normal;
                ((CarPartFloatVertex *)set->meshes[i]->pVertexData)[j].normal[0] = normal.x * CGraphics::m_oneOver65536;
                ((CarPartFloatVertex *)set->meshes[i]->pVertexData)[j].normal[1] = normal.y * CGraphics::m_oneOver65536;
                ((CarPartFloatVertex *)set->meshes[i]->pVertexData)[j].normal[2] = normal.z * CGraphics::m_oneOver65536;
            }
            pMesh = set->nodes[i]->pMesh;
            if (pMesh != NULL) {
                Mesh_Rebuild(pMesh);
                RallyData_ValidateIndex((int)pMesh);
                Scene_MarkShadowPartDirty(*(SceneNode **)(pCar + 0x720), pMesh);
            }
        }
        for (i = 0; i < 9; i++)
            ((int *)set->damageGrid)[i] = 0;
        set->field_0x469 = 0;
        set->field_0x46c = 0;
        for (i = 0; i < 0x22; i++)
            set->field_0x350[i] = 0;
        for (i = 0; i < 8; i++) {
            set->field_0x490[i] = 0;
            set->field_0x470[i] = 0;
            StageObject_SetModelSubmeshVisibility(pCar, 0, i);
        }
        StageObject_ModifyDamageRecordFlagBytes(*(char *)(pCar + 0xb1a), 0, 0, 4);
    }
}

// Snapshots a car's replay colours and end values once per stage.
// FUNCTION: CMR2 0x00469a80
void StageTiming_SnapshotStageReplayColours(int car)
{
    Car *pCar = Car_Get(car);
    BYTE *pSource = g_unk0x00588b94 + pCar->index * 0x4d0;
    BYTE *pRecord = g_unk0x00588b98 + pCar->index * 0x290;
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
void StageTiming_InsertDriverSplitRanking(int driver, int hundredths, int split)
{
    int time = Timing_CentisecondsToFixedSeconds(hundredths);
    int slot = driver;
    int pos;
    int i;
    int c;

    if (driver < g_unk0x00541f98)
        slot = g_unk0x00541f90[StageUI_GetRaceEndEventCount() & 0xff];
    g_stageSplitTimesRaw[split][slot] = time;
    for (pos = 0; g_stageSplitTimesRaw[split][g_stageSplitTimesRawDriverIx[split][pos]] < time &&
                  pos != g_stageSplitDriverCount[split];) {
        pos++;
        if (pos >= 16)
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
void StageTiming_BlendCarPaintDecalRows(int pCar, int *pOut)
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

    v = (unsigned int)RallyData_GetRouteAvailabilityState();
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
        d3 = (int)(__int64)(g_unk0x0054241c[i + 3] * CGraphics::m_65536);
        d4 = (int)(__int64)(g_unk0x0054241c[i + 4] * CGraphics::m_65536);
        d0 = (int)(__int64)(g_unk0x0054241c[i] * CGraphics::m_65536);
        t1 = FixDiv(*(int *)(pCar + 0xa0), t);
        t2 = FixDiv(t - *(int *)(pCar + 0xa4), t);
        t3 = 0x10000 - t2 - t1;
        m1 = FixMul(r8, d3);
        m2 = FixMul(t1, m1);
        m3 = FixMul(a8, d4);
        m4 = FixMul(t1, m3);
        m5 = FixMul(rc, d3);
        m6 = FixMul(t2, m5);
        m7 = FixMul(d4, ac);
        m8 = FixMul(t2, m7);
        t4 = FixDiv(sum, 0x20000);
        m9 = FixMul(t4, d3);
        m10 = FixMul(t3, m9);
        m11 = FixMul(d4, 0x10000 - t4);
        *pOut = FixMul(t3, m11) + m10 + m8 + m6 + m4 + m2 + d0;
    }
}

// Adds a random spread to the computer drivers' times and sorts them.
// match 84%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00456110
void StageTiming_RandomizeComputerTimes(int *pTimes)
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
void StageTiming_AdvanceTimedLapCheckpoint(int car, int time)
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
void StageTiming_CopyTriangleVertices(int *pOut, unsigned short *pIndices, int unused)
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

int RallyData_GetCarRaceRecordField10(BYTE *p);
int *RallyData_GetDriverSettingPair(int index);

// Third ramp of a view's object state: value by route position plus the
// car's progress within the node.
// match 75%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0045e8b0
void StageObject_UpdateThirdRouteRamp(unsigned int *pRecord, int view)
{
    Car *pCar;
    unsigned int position;
    int t;

    if (RallyData_GetRouteAdditionalState() == 0) {
        pRecord[6] = pRecord[4];
        pCar = Car_Get(View_GetActiveCameraFlags(view));
        position = RallyData_GetActiveCarRaceRecordField0((BYTE *)pCar);
        pRecord[0] = position;
        if (position != pRecord[1]) {
            pRecord[1] = position;
            if (position <= (unsigned int)g_unk0x00543d94) {
                pRecord[2] = g_unk0x00543d88;
            } else if (position >= (unsigned int)g_unk0x00543d98) {
                pRecord[2] = g_unk0x00543d8c;
            } else {
                pRecord[2] = (t = position - g_unk0x00543d94) << 16;
                pRecord[2] = FixMul(t << 16, g_unk0x00543d90);
                pRecord[2] = pRecord[2] + g_unk0x00543d88;
            }
        }
        if (pRecord[0] < (unsigned int)g_unk0x00543d98 && pRecord[0] >= (unsigned int)g_unk0x00543d94) {
            t = RallyData_GetCarRaceRecordField10((BYTE *)pCar);
            pRecord[3] = FixMul(g_unk0x00543d90, t);
            pRecord[4] = pRecord[2] + pRecord[3];
            return;
        }
        pRecord[3] = 0;
        pRecord[4] = pRecord[2] + *(volatile unsigned int *)&pRecord[3];
    }
}

int StageObject_GetCarWeatherRampValue(BYTE *pCar);
void Car_UpdateSurfaceParams(Car *pCar, int blend);
void StageObject_CopyCarSurfaceNoiseTarget(BYTE *pCar);

// Sets the third object ramp and resets every car's record and surface.
// FUNCTION: CMR2 0x0045e710
void StageTiming_SetThirdObjectRamp(int v1, int v2, int from, int to)
{
    int i;
    int *p;

    g_unk0x00543d74 = v1;
    g_unk0x00543d78 = v2;
    g_unk0x00543d84 = to;
    g_unk0x00543d80 = from;
    g_unk0x00543d7c = (to - from) << 16;
    if (g_unk0x00543d7c > 0)
        g_unk0x00543d7c = FixDiv(0x10000, g_unk0x00543d7c);
    g_unk0x00543d7c = FixMul(g_unk0x00543d7c, g_unk0x00543d78 - g_unk0x00543d74);
    i = 0;
    if (g_unk0x00547acc > 0) {
    do {
        p = (int *)((BYTE *)g_unk0x00543ecc + i * 0xc);
        p[1] = 0;
        p[0] = 0;
        p[2] = g_unk0x00543d74;
        Car_UpdateSurfaceParams(Car_Get(i), StageObject_GetCarWeatherRampValue((BYTE *)Car_Get(i)));
        StageObject_CopyCarSurfaceNoiseTarget((BYTE *)Car_Get(i));
        i++;
    } while (i < (int)(g_stageObjectCount.packed & 0xff));
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
void StageWeather_SetupSettingTransition(void)
{
    int *pPair = RallyData_GetDriverSettingPair(RallyDataStageIndex());
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
        g_unk0x00543e8c = (unsigned int)RallyData_GetRouteAvailabilityState() / 5;
        g_unk0x00543e94 = RallyData_GetRouteAvailabilityState() - g_unk0x00543e8c / 5;
        return;
    }
    g_unk0x00543e8c = 0;
    g_unk0x00543e94 = 0;
}

// Advances a car's checkpoint record on a looped stage.
// match 50%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004590a0
void StageTiming_AdvanceLoopedCarCheckpoint(int car, int time)
{
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
        if (g_unk0x00542e78[car].field_0xa == g_unk0x00542c74)
            g_unk0x00542e78[car].field_0xa = 0;
        g_unk0x00542e78[car].field_0x14++;
        if (g_unk0x00542e78[car].field_0x14 >= g_unk0x00542c74)
            g_unk0x00542e78[car].field_0x14 = 0;
    }
}

// Inserts the car's current time at its split into the split ranking.
// FUNCTION: CMR2 0x00456a00
void StageTiming_InsertCarSplitRanking(int car, int driver)
{
    int split = StageTiming_GetCheckpointField14(car);

    if (CGameInfo::GetConfiguredGameMode() != 4)
        StageTiming_InsertDriverSplitRanking(driver, StageTiming_GetClockTime(), split);
}

unsigned short RallyData_GetSecondaryStageScoreScale(void);
unsigned short RallyData_GetPrimaryStageScoreScale(void);

// Sets up the three object ramps of the stage from its weather change.
// match 72%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0045f260
void StageObject_SetupWeatherRamps(void)
{
    int *pPair = RallyData_GetDriverSettingPair(RallyDataStageIndex());
    short a = (short)RallyData_GetSecondaryStageScoreScale();
    short b = (short)RallyData_GetPrimaryStageScoreScale();
    int last;
    int from;
    int v2;

    if ((b != a || pPair[0] != pPair[1]) && RallyData_GetRouteAdditionalState() == 0) {
        last = RallyData_GetRouteAvailabilityState() - 1;
        from = 1;
        v2 = 0x10000;
    } else {
        last = 0;
        from = 0;
        v2 = 0;
    }
    StageTiming_SetSecondaryObjectRamp(0, v2, from, last);
    StageTiming_SetPrimaryObjectRamp(g_unk0x00543e9c, g_unk0x00543fa8, g_unk0x00543e8c, g_unk0x00543e94, g_unk0x00543d54);
    StageTiming_SetThirdObjectRamp(g_unk0x00543e88, g_unk0x00543d9c, g_unk0x00543e8c, g_unk0x00543e94);
}

BYTE StageTiming_GetCheckpointField1A(int index);

// Records a car's split time when it passes a split.
// FUNCTION: CMR2 0x004569c0
void StageTiming_RecordPassedCarSplit(int car, int driver)
{
    if (StageTiming_GetCheckpointField17(car))
        StageTiming_InsertCarSplitRanking(car, driver);
    if (StageTiming_GetCheckpointField1A(car))
        StageTiming_SetSplitDisplayFlag(car, driver);
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
void ForceFeedback_ComputeSteeringForce(void)
{
    FixVector *pForward;
    FixVector *pAxis;
    int left;
    int right;
    int force;
    int speed;

    force = g_unk0x005391f8[1] - g_unk0x005391f8[0];
    if (g_unk0x00539270[0] != 0) {
        pAxis = (FixVector *)(g_unk0x0053937c + 0x4a4);
        pForward = (FixVector *)(g_unk0x0053937c + 0x378);
        left = FixVecDot(pForward, pAxis);
    } else {
        left = 0;
    }
    if (g_unk0x00539270[1] != 0) {
        pAxis = (FixVector *)(g_unk0x0053937c + 0x4b0);
        pForward = (FixVector *)(g_unk0x0053937c + 0x378);
        right = FixVecDot(pForward, pAxis);
    } else {
        right = 0;
    }
    force = force - right - left;
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

    speed = FixMul(0x40000, g_unk0x00539278->field_0x18);
    force = FixMul(force, speed);
    if (FIX_ABS(force) > 0x10000)
        force = force > 0 ? 0x10000 : -0x10000;
    g_unk0x00539278->field_0x14 = FixMul(force, 0x27100000) >> 16;
}

// Computes each axle's rumble from its suspension travel and stiffness,
// plus the ground roughness, and their average.
// match 77%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004247a0
void ForceFeedback_ComputeAxleRumble(void)
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
            v = FixMul(*(int *)(p + 0x1a4 + i * 0xc) + *(int *)(p + 0x88 + i * 0x24), v);
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
void ForceFeedback_ComputeConditionCoefficients(void)
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
        contact = FixVecDot(pForward, pVelocity);
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

    speed = FixMul(0x40000, g_unk0x00539278->field_0x1c);
    spring = FixMul(spring, speed);
    if (FIX_ABS(spring) > 0x10000)
        spring = spring > 0 ? 0x10000 : -0x10000;
    speed = FixMul(0x40000, g_unk0x00539278->field_0x1c);
    damper = FixMul(damper, speed);
    if (FIX_ABS(damper) > 0x10000)
        damper = damper > 0 ? 0x10000 : -0x10000;
    g_unk0x00539278->field_0xc = FixMul(spring, 0x27100000) >> 16;
    g_unk0x00539278->field_0x10 = FixMul(damper, 0x27100000) >> 16;
}

// Releases the stage's files (registered callback of 0x455080).
// FUNCTION: CMR2 0x00454f20
int StageTiming_ReleaseStageArchives(void)
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

void StageTiming_RecordPassedCarSplit(int car, int driver);
unsigned int RallyData_GetSelectionFlag27(void);

// Records the split times of every car in the current split group.
// FUNCTION: CMR2 0x00455590
void StageTiming_RecordSplitGroupTimes(int group)
{
    int i;

    if ((BYTE)RallyData_GetSelectionFlag27()) {
        for (i = 1; i > -1; i--) {
            if (((BYTE *)&g_unk0x00542418)[i] == 0)
                StageTiming_RecordPassedCarSplit(i, g_stageSplitUnk0x00541f78[group][i]);
        }
        return;
    }
    for (i = 0; i < (char)g_unk0x00542528[0xc0 + group]; i++) {
        if (((BYTE *)&g_unk0x00542418)[i] == 0)
            StageTiming_RecordPassedCarSplit(i, (char)g_unk0x00542528[0xc8 + group * 2 + i]);
    }
}

extern float g_oneOverRandMax;
extern double g_minus65536;

// Random force-feedback road noise of the current slot, scaled by speed and
// the suspension movement.
// match 57%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00424af0
void ForceFeedback_ComputeRoadNoise(void)
{
    int speed;
    int k;

    if (g_unk0x00539278->field_0x20 == 0)
        g_unk0x00539278->field_0x20 = (int)(__int64)(rand() * g_oneOverRandMax * (float)CGraphics::m_65536);
    else if (g_unk0x00539278->field_0x20 <= 0)
        g_unk0x00539278->field_0x20 = (int)(__int64)(rand() * g_oneOverRandMax * (float)CGraphics::m_65536);
    else
        g_unk0x00539278->field_0x20 = (int)(__int64)(rand() * g_oneOverRandMax * (float)g_minus65536);
    speed = FixMul(*(int *)(g_unk0x0053937c + 0x778), 0x10000);
    if (speed > 0x10000)
        speed = 0x10000;
    k = FixMul(speed, *(int *)(g_unk0x0053937c + 0xc0) + *(int *)(g_unk0x0053937c + 0x9c)) + g_unk0x00539278->field_0x28;
    if (k > 0x10000)
        k = 0x10000;
    g_unk0x00539278->field_0x20 = FixMul(g_unk0x00539278->field_0x20, k);
}

// Attaches a stage object to the current car and copies its matrices.
// FUNCTION: CMR2 0x004813b0
int StageTiming_AttachObjectToCurrentCar(int slot)
{
    BYTE *object;

    if (*(int *)g_partState == 0) {
        object = g_unk0x00590b7c[slot][(signed char)((BYTE *)g_partCar)[0xb1a]];
        if (object != NULL) {
            *(BYTE **)g_partState = object;
            *(BYTE **)((BYTE *)g_partState + 4) = (BYTE *)g_partState + 0xc;
            *(BYTE **)((BYTE *)g_partState + 8) = (BYTE *)g_partState + 0xcc;
            memcpy((BYTE *)g_partState + 0xc, (*(BYTE **)g_partState) + 0x98, 0x40);
            memcpy((BYTE *)g_partState + 0xcc, (*(BYTE **)g_partState) + 0xd8, 0x40);
            *(BYTE **)((BYTE *)g_partState + 0x10c) = *(BYTE **)((BYTE *)g_partCar + 0x71c) + 0x98;
            memcpy(*(BYTE **)((BYTE *)g_partState + 8), *(BYTE **)((BYTE *)g_partCar + 0x720) + 0xd8, 0x40);
            FixMatrix_GetRight(&g_partState->basis.right, (FixMatrix *)((*(BYTE **)g_partState) + 0x98));
            FixMatrix_GetUp(&g_partState->basis.up, (FixMatrix *)((*(BYTE **)g_partState) + 0x98));
            FixMatrix_GetForward(&g_partState->basis.forward, (FixMatrix *)((*(BYTE **)g_partState) + 0x98));
            return 1;
        }
    }
    return 0;
}

extern char g_stageLooped;
extern int g_stageCheckpointCount;

// Advances one car through crossed checkpoints.
// match 34%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00458e00
void StageTiming_AdvanceCrossedCheckpoints(int car, int target)
{
    int current = g_unk0x00542e78[car].field_0x0;
    int step;
    int i;
    int prev;
    int delta;

    if (target == current)
        return;
    delta = target - current;
    if (g_stageLooped == 0)
        step = delta >= 1 ? 1 : -1;
    else if (delta < -50 || (delta > 0 && delta < 50))
        step = 1;
    else
        step = -1;
    for (i = 0; current != target && i < g_stageCheckpointCount; i++) {
        g_unk0x00542e78[car].field_0x12 += (short)step;
        if ((int)g_unk0x00542e78[car].field_0x12 > (unsigned short)g_unk0x00542e78[car].field_0x10)
            g_unk0x00542e78[car].field_0x10 = g_unk0x00542e78[car].field_0x12;
        prev = current;
        current += step;
        if (current < 0)
            current = g_stageCheckpointCount - 1;
        if (current >= g_stageCheckpointCount)
            current = 0;
        if (g_stageLooped)
            StageTiming_CountLapCrossings(car, prev, current);
        if (g_unk0x00542cad != 0)
            StageTiming_AdvanceLoopedCarCheckpoint(car, current);
        else
            StageTiming_AdvanceTimedLapCheckpoint(car, current);
        StageTiming_FlagCoincidentCheckpointPositions(car);
        StageTiming_SetViewRouteDistanceLimit(car, current, step);
    }
    g_unk0x00542e78[car].field_0x0 = (short)target;
    g_unk0x00542e78[car].field_0x16 = 1;
}

// GLOBAL: CMR2 0x00590b10
int g_unk0x00590b10[8];
// GLOBAL: CMR2 0x00511380
extern const double g_unk0x00511380 = 0.0099471839432434591;

// Smooths the active car's steering offset and derives a short angle.
// match 71%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00480cb0
void CarPart_SmoothSteeringOffset(void)
{
    int speed = FixMul(g_partCar->speed, FixDiv(0x10000, 0x18000));
    int step;
    int target;
    int index;
    int delta;
    int phase;

    if (speed > 0x10000)
        speed = 0x10000;
    step = FixMul(FixMul(speed, g_physicsTimeStep), 0xf5c);
    target = FixMul(FixMul(speed, g_physicsTimeStep), 0x3333);
    phase = g_partCar->position.x + g_partCar->position.z;
    phase = FIX_ABS(phase);
    phase %= 1024;
    target = FixMul(target, phase << 6);
    index = (signed char)((BYTE *)g_partCar)[0xb1a];
    delta = target - g_unk0x00590b10[index];
    if (FIX_ABS(delta) > step) {
        if (delta > 0)
            target = g_unk0x00590b10[index] + step;
        else
            target = g_unk0x00590b10[index] - step;
    }
    g_unk0x00590b10[index] = target;
    *(short *)&g_unk0x00590c68 = (short)(int)(__int64)((double)target * g_unk0x00511380);
}

BYTE Car_GetDrawnFlag(int index);
unsigned short Input_GetControllerSlotMapping(unsigned short slot);
unsigned int Input_GetControllerPrimaryFlags(unsigned short slot);
unsigned int Input_GetControllerSecondaryFlags(unsigned short slot);
DWORD Input_GetControllerField11C(unsigned short slot);
DWORD Input_GetControllerField118(unsigned short slot);

// Initializes the two force feedback slots and registers their stop callback.
// FUNCTION: CMR2 0x00423ff0
void StageTiming_InitForceFeedbackSlots(void)
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
        device = Car_GetDrawnFlag(Car_Get(i)->index);
        CInput::GetAvailableDeviceRecord((signed char)device);
        if (Input_GetControllerField118(i) != 0) {
            device = Car_GetDrawnFlag(Car_Get(i)->index);
            CInput::SetForceFeedbackDeviceValue(Input_GetControllerField11C(i), (signed char)device);
            g_unk0x00539278->field_0x18 = Input_GetControllerPrimaryFlags(i);
            g_unk0x00539278->field_0x1c = Input_GetControllerSecondaryFlags(i);
            g_unk0x00539278->field_0x2c = (signed char)Input_GetControllerSlotMapping(i);
            if (g_unk0x00539278->field_0x2c >= 0) {
                ForceFeedback_CreateEffects();
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
    ForceFeedback_DeactivateSlots();
    CGame::RegisterCallback(ForceFeedback_DeactivateSlots, NULL);
}

#include "WheelTrail.h"

extern int Car_GetWheelSpeed(Car *, BYTE, int);
extern int StageObject_GetWheelSlip(int, int);
extern BYTE StageObject_GetViewWeatherStateByte(int);
extern int StageObject_GetViewWeatherField54(int);
extern int Race_GetPlayerRecordField4(BYTE);
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
        short material = car->wheelSurfaceType[wheel];
        int front = wheel == 2 || wheel == 3;
        int reverse = car->gear == 7;
        int leading = reverse ? front : !front;
        int surface = car->wheelSurface[wheel];
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
        if (StageObject_GetViewWeatherStateByte(carIndex) == 1 && (FixMul(StageObject_GetViewWeatherField54(carIndex), 0xff0000) >> 16) > 1) {
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
        if ((leading && Race_GetPlayerRecordField4((BYTE)carIndex)) ||
            (water && leading && StageObject_GetViewWeatherStateByte(carIndex) == 2 &&
             (FixMul(StageObject_GetViewWeatherField54(carIndex), 0xff0000) >> 16) > 100)) water = 0;
        int emit = dust || water;
        if (!car->cornerOnGround[wheel]) emit = 0;
        int speedLimit = leading ? 0x320000 : 0x230000;
        int type = 4;
        int speed = FIX_ABS(Car_GetWheelSpeed(car, 0, 0));
        if (speed < 0x1e0000) {
            int chance = FixDiv(speed, 0x1e0000);
            if (chance < TRAIL_RANDOM(CGraphics::m_65536)) emit = 0;
        }
        if (leading) {
            type = 6;
            if (!Race_GetPlayerRecordField4((BYTE)carIndex) && TRAIL_RANDOM(CGraphics::m_65536) > 0xb333) emit = 0;
        }
        if (carIndex > 0 && TRAIL_RANDOM(CGraphics::m_65536) > 0x8000) continue;
        if (!emit) continue;
        if (type == 4) rand();

        FixVector delta, rolling, lateral, velocity, cornerMotion, jitter, position, blend, source, particleVelocity;
        delta.x = g_trailPos[carIndex][wheel].x - g_trailLastPos[carIndex][wheel].x;
        delta.y = g_trailPos[carIndex][wheel].y - g_trailLastPos[carIndex][wheel].y;
        delta.z = g_trailPos[carIndex][wheel].z - g_trailLastPos[carIndex][wheel].z;
        int slipA = FIX_ABS(car->wheelSlipLateral[wheel]) - 0xccc;
        int slipB = FIX_ABS(car->wheelSlip[wheel]) - 0xccc;
        FixVector *cornerVelocity = &car->cornerVelocity[wheel];
        cornerMotion = *cornerVelocity;
        int random;
        if (front) {
            FixVecScale(&rolling, &car->groundDir[1], car->wheelSlip[wheel]);
            random = TRAIL_RANDOM(g_minus65536);
            FixVecScale(&lateral, &car->groundAxis[1], -0x8000 - random);
        } else {
            FixVecScale(&rolling, &car->groundDir[0], car->wheelSlip[wheel]);
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
        if (Race_GetPlayerRecordField4((BYTE)carIndex) || (*(BYTE **)(StageUI_GetRaceResultTable() + 4))[carIndex * 8] == 9) {
            type = 7;
            uniform = 1;
        }
        if (dust) type += 4;
        random = TRAIL_RANDOM(g_minus65536);
        FixVecScale(&jitter, &delta, random);
        int t;
        if (!(!leading)) {
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
        } else {
            t = TRAIL_RANDOM(CGraphics::m_65536);
            if (!uniform) {
                random = TRAIL_RANDOM(g_minus65536);
                t = FixDiv(t, 0x1547a - random * 9);
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
            if (!(speed > speedLimit)) {
                FixVector offset;
                offset.x = velocity.x - cornerVelocity->x;
                offset.y = velocity.y - cornerVelocity->y;
                offset.z = velocity.z - cornerVelocity->z;
                FixVecScale(&offset, &offset, 0x6666);
                particleVelocity.x += offset.x; particleVelocity.y += offset.y; particleVelocity.z += offset.z;
            } else {
                int scale = FixDiv(speedLimit, speed);
                FixVecScale(&particleVelocity, &particleVelocity, scale);
            }
            Particle_Spawn(type, &source, &particleVelocity, source.y - 0x10000, 0, colour,
                           g_trailLevel[carIndex][wheel], (int)&carIndex, *((BYTE *)car->pNode0x720 + 0x17c));

        }
    }
}
#undef TRAIL_RANDOM

struct Unk0x0052ebc0;
Unk0x0052ebc0 *RallyData_GetDriverGroupRecord(int index);
void OptionPreview_SetMeshRenderFlags(int index, int param2, BYTE param3);

// Camera-space dent parameters: apex, direction, reference direction, radius,
// advance step and the falloff/scale factors used by CarDamage_ApplyCameraSpaceMeshDent.
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
void CarDamage_ApplyCameraSpaceMeshDent(int *pRecord)
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
void StageTiming_SelectRecordMeshBrightness(int index)
{
    BYTE *pValue = (BYTE *)RallyData_GetDriverGroupRecord(index);
    BYTE level = 0;
    do {
        int value = FixDiv((int)pValue[0x122 + level] << 16, 0xff0000);
        if (value > g_unk0x00527324[level])
            OptionPreview_SetMeshRenderFlags(index, 2, level);
        else if (value > g_unk0x00527344[level])
            OptionPreview_SetMeshRenderFlags(index, 1, level);
    } while (++level < 8);
}

// First of the eight car network records (the rest are g_unk0x005394bc).
// GLOBAL: CMR2 0x005393d8
CarNetRecord g_unk0x005393d8;

extern NetStats g_localCarStats;
void NetRace_PackCarState(Car *car);
NetStats *NetPlayers_GetStatisticsRecord(int index);
int NetPlayers_FindPlayerByField8(int value);
short RallyData_GetActiveCarRaceRecordField14(BYTE *p);
int Replay_GetStreamFrameState(void);
unsigned int RallyData_GetFlag21(void);

// Snapshots a car's body axes and position into its network record and marks
// it for sending.
// FUNCTION: CMR2 0x00424dc0
void StageTiming_SnapshotCarNetworkPose(Car *car)
{
    CarNetRecord *rec = (CarNetRecord *)&g_unk0x005393d8 + car->index;
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
    if (car->index > 0) {
        NetRace_PackCarState(car);
        stats = NetPlayers_GetStatisticsRecord(NetPlayers_FindPlayerByField8((int)car->index));
        *stats = g_localCarStats;
        NetPlayers_GetStatisticsRecord(NetPlayers_FindPlayerByField8((int)car->index))->seq = 0;
    }
}

// Clears the road-book slots of every car in the running order and
// reinitialises the ones with no pending target.
// match 58%: only the esi/edi allocation differs
// FUNCTION: CMR2 0x004581d0
void StageTiming_ResetOrderedCarRoadBooks(void)
{
    int count = Car_GetOrderCount();
    int i;

    for (i = 0; i < count; i++) {
        g_unk0x00542e78[i].field_0x16 = 0;
        g_unk0x00542e78[i].field_0x17 = 0;
        g_unk0x00542e78[i].field_0x18 = 0;
        g_unk0x00542e78[i].field_0x19 = 0;
        if (g_unk0x00542e78[i].field_0x1a == 0)
            StageTiming_AdvanceCrossedCheckpoints(i, Stage_GetPreviousCheckpoint(RallyData_GetActiveCarRaceRecordField14((BYTE *)Car_Get(i)) & 0xffff));
    }
}

// Paths of the stage sample files loaded by StageTiming_LoadWeatherParticleTextures.
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
void Frontend_SetObjectField118(Unk0x004a3e20 *pObject, int value);

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
void StageTiming_LoadWeatherParticleTextures(void)
{
    bool didLoad;
    int i;

    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::GetTexturesDirectory(), g_strRainTga);
    g_unk0x00543e90 = (int)CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile0(),
                                                     CFrontend::m_stringDest, &didLoad, NULL, 0, 0);
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::GetTexturesDirectory(), g_strSplatTga);
    g_unk0x00543ea4 = (int)CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile0(),
                                                     CFrontend::m_stringDest, &didLoad, NULL, 0, 0);
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::GetTexturesDirectory(), g_strSplat2Tga);
    g_unk0x005477f0 = (int)CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile0(),
                                                     CFrontend::m_stringDest, &didLoad, NULL, 0, 0);
    for (i = 0; i < 3; i++) {
        sprintf(CFrontend::m_stringDest, g_strSnowTga, CInstallInfo::GetTexturesDirectory(), i + 1);
        g_unk0x00543ea8[i] = (int)CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile0(),
                                                            CFrontend::m_stringDest, &didLoad, NULL, 0, 0);
    }
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::GetTexturesDirectory(), g_strSunGlowTga);
    g_unk0x00547ad0 = (int)CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile0(),
                                                     CFrontend::m_stringDest, &didLoad, NULL, 0, 0);
    Frontend_SetObjectField118((Unk0x004a3e20 *)g_unk0x00547ad0, 1);
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
void StageTiming_LoadSelectedTeamArchive(void)
{
    unsigned int sizes[3];
    void *bufs[3];
    unsigned int size = 0;
    void *dest;
    BYTE index;

    g_unk0x00542604 = 0;
    g_unk0x00541f8c = Replay_GetStreamFrameState();
    sprintf(CFrontend::m_stringDest, g_strTm0Format, GameMenu_GetChampionshipTransitionState());
    bufs[0] = CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(),
                                           CFrontend::m_stringDest, NULL, (DWORD *)&size, 0);
    sizes[0] = size;
    sprintf(CFrontend::m_stringDest, g_strTm1Format, GameMenu_GetChampionshipTransitionState());
    bufs[1] = CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(),
                                           CFrontend::m_stringDest, NULL, (DWORD *)&size, 0);
    sizes[1] = size;
    sprintf(CFrontend::m_stringDest, g_strTm2Format, GameMenu_GetChampionshipTransitionState());
    bufs[2] = CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(),
                                           CFrontend::m_stringDest, NULL, (DWORD *)&size, 0);
    sizes[2] = size;
    if ((char)RallyData_GetFlag21() != 0) {
        dest = (void *)g_unk0x00541f8c;
        if (dest == NULL)
            dest = bufs[0];
        if (CGameInfo::GetConfiguredGameMode() == 4) {
            index = CGameInfo::GetGameModeOptionBits20To22();
            if (index != 0)
                index = index - 1;
        } else {
            index = CGameInfo::GetConfiguredDifficulty();
        }
        memcpy(dest, bufs[index], sizes[index]);
        g_unk0x00541f8c = (int)dest + sizes[index];
        g_unk0x0054241c = (char *)dest;
        if (dest != NULL)
            g_unk0x00542604 = 1;
    }
}

extern char g_stageSplitCount;
BYTE *RallyData_GetAvailableRouteNodeRecord(int index);
unsigned int RallyData_GetSelectionFlag27(void);

// Estimates the remaining stage time of every driver slot from the progress
// made so far, inserting each newly queued slot at the end of the order.
// Differential coverage: guarded time/order tables, ties, summary timing and real helpers.
// FUNCTION: CMR2 0x004483e0
void StageTiming_EstimateRemainingDriverTimes(void)
{
    int count = Stage_GetDriverCount();
    int slot = (int)(char)g_unk0x0053e18c;
    int prev = 0;
    int total;
    int i;
    int j;
    int k;
    int value;
    char *pSlot;

    total = (BYTE)RallyData_GetSelectionBits16To19() * RallyData_GetRouteAvailabilityState() * 0x10000;
    while (1) {
        if (count <= slot)
            return;
        pSlot = &g_unk0x0053e17c[slot];
        for (i = 0; i < count; i++) {
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
            value = FixDiv((StageTiming_GetCheckpointField10(k) & 0xffff) << 16, total);
            if (value >= 1) {
                int elapsed = Timing_CentisecondsToFixedSeconds(g_unk0x0053d1b0);
                g_unk0x0053d1b8[k] =
                    ConvertRawTimeToCentiseconds(FixMul(FixDiv(0x10000, value), elapsed));
            } else {
                g_unk0x0053d1b8[k] = g_unk0x0053d1b0 * 2;
            }
            if ((char)RallyData_GetSelectionFlag27() != 0)
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
void StageTiming_BuildCheckpointSplitPositions(void)
{
    int count = 0;
    int i;
    int unit;
    int scale;

    g_unk0x00542c74 = 0;
    g_stageCheckpointCount = RallyData_GetRouteAvailabilityState();
    if (g_stageCheckpointCount == 0)
        return;
    for (i = 0; i < g_stageCheckpointCount; i++) {
        if ((*(BYTE *)(RallyData_GetAvailableRouteNodeRecord(i) + 0x18) & 1) != 0) {
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



extern char g_unk0x00542cad;
char Race_GetBaseCarCount(void);
unsigned int RallyData_GetSetupFlag11(void);
unsigned int RallyData_GetSetupModeBits(void);
unsigned int RallyData_GetSetupHighNibble(void);

// Rebuilds the stage split table and the checkpoint count for the current
// event.
// FUNCTION: CMR2 0x00458090
void StageTiming_RebuildSplitsAndCheckpoints(void)
{
    StageTiming_BuildCheckpointSplitPositions();
    g_unk0x00542cad = 0;
    g_unk0x00542c68 = (BYTE)Race_GetBaseCarCount();
    g_unk0x00542c6c = (BYTE)RallyData_GetSelectionBits16To19();
    if ((char)RallyData_GetSetupFlag11() != 0 && RallyData_GetSetupModeBits() == 2)
        g_unk0x00542c6c = RallyData_GetSetupHighNibble() << 1;
    if (CGameInfo::GetConfiguredGameMode() == 7 || CGameInfo::GetConfiguredGameMode() == 12)
        g_unk0x00542cad = 1;
}

extern int g_sinTable[4096];
void View_GetHeading(short *pOut, unsigned int view);
void StageObject_DeriveLoadedScaleVector(FixVector *pOut);

// Integrates the terrain slope under a car into its body pitch, wrapping at a
// full turn.
// FUNCTION: CMR2 0x0045f5d0
void StageTiming_IntegrateTerrainBodyPitch(int pData, int param_2)
{
    FixVector vec;
    FixVector direction;
    unsigned short orientation;
    int *p = (int *)pData;
    int value;
    int angle;

    p[8] = p[5];
    View_GetHeading((short *)&orientation, param_2);
    direction.y = 0;
    direction.x = -g_sinTable[orientation & 0xfff];
    direction.z = g_sinTable[(orientation + 0x400) & 0xfff];
    StageObject_DeriveLoadedScaleVector(&vec);
    value = FixVecDot(&direction, &vec);
    p[5] += FixMul(0xf5c, value);
    angle = p[5];
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
void StageTiming_RecordAndReorderSplitTime(int car)
{
    int split = StageTiming_GetCheckpointField14(car);
    int count;
    int group;
    int index;

    if ((char)RallyData_GetFlag24() == 0 && (char)RallyData_GetFlag25() == 0) {
        if (car >= (int)((BYTE)RallyDataState()))
            return;
        g_unk0x0053e190[split + car * 9] = g_unk0x0053d1b0;
        return;
    }
    count = Stage_GetDriverCount();
    group = StageTiming_GetCheckpointGroupIndex(car);
    index = StageTiming_GetCheckpointSplitIndex(car);
    if ((char)RallyData_GetFlag24() != 0)
        g_unk0x0053d1e8[car][group][split] = g_unk0x0053d1b0;
    else
        g_carStageTiming[car].field_0x4[split] = g_unk0x0053d1b0;
    if (car < (int)((BYTE)RallyDataState()) || (char)RallyData_GetSelectionFlag27() != 0) {
        g_carStageTiming[car].field_0x84 = 0;
        if ((char)RallyData_GetFlag24() != 0)
            g_unk0x0053e190[split + car * 9] = g_unk0x0053d1b0 - g_unk0x0053d1e8[car][group][0];
        else
            g_unk0x0053e190[split + car * 9] = g_unk0x0053d1b0;
    }
    if ((char)RallyData_GetSelectionFlag27() != 0) {
        group = 0;
        index = split;
    }
    index = index + group * 9;
    char prev = g_unk0x0053ddb0[index];
    int old = prev;
    g_unk0x0053ddb0[index] = prev + 1;
    g_carStageTiming[car].field_0x82 = old;
    g_unk0x0053de1c[index][old] = (char)car;
    if (old > 0) {
        char other = g_unk0x0053de1c[index][old - 1];

        g_carStageTiming[other].field_0x80 = (char)car;
        g_carStageTiming[other].field_0x83 = 1;
    }
    if (old == count - 1)
        g_carStageTiming[car].field_0x80 = (char)0xff;
}

// Refreshes the paint-decal samples of one car and applies the resulting time
// spread to the current timing record.
// FUNCTION: CMR2 0x00455ed0
void StageTiming_RefreshPaintSamplesAndTimeSpread(int pCar)
{
    int samples[16];

    StageTiming_BlendCarPaintDecalRows(pCar, samples);
    StageTiming_RandomizeComputerTimes(samples);
}

int NetPlayers_GetPlayerIDCount(void);

// Resets the timing records of the drivers taking part in the race according
// to the current mode (rally, split-screen or network).
// FUNCTION: CMR2 0x00458100
void StageTiming_ResetParticipatingDriverRecords(int param_1)
{
    int i;
    int count;

    if ((char)RallyData_GetSelectionFlag27() != 0) {
        if (CGameInfo::GetGameModeOptionBit19() != 0) {
            if (CGameInfo::GetConfiguredGameMode() == 10)
                count = 1;
            else
                count = NetPlayers_GetPlayerIDCount();
            for (i = 0; i < count; i++) {
                switch (g_unk0x00542cb4[i]) {
                case 0:
                    StageTiming_ResetPlayerLapRecord(i, 0, -1, param_1);
                    break;
                case 1:
                    StageTiming_ResetPlayerLapRecord(i, 2, 0, param_1);
                    break;
                }
            }
        } else {
            StageTiming_ResetPlayerLapRecord(g_unk0x00542cb0, 0, -1, param_1);
            StageTiming_ResetPlayerLapRecord(1 - g_unk0x00542cb0, 2, 0, param_1);
        }
        return;
    }
    i = 0;
    if ((BYTE)Race_GetBaseCarCount() > 0) {
        do {
            StageTiming_ResetPlayerLapRecord(i, 0, 0, param_1);
            i++;
        } while (i < (int)(BYTE)Race_GetBaseCarCount());
    }
}

#include <stdlib.h>

extern double g_unk0x00511300;
unsigned int RallyData_GetFlag31(void);
unsigned char RallyDataStageIndex(void);
unsigned char RallyData_GetFlag24(void);
extern int g_unk0x00542cd4;
extern int g_unk0x00542d38[8];
extern int g_unk0x00542cd8[8][3];
int StageObject_Atan2Degrees(int y, int x);
int RallyTiming_GetChampionshipDriverPosition(int index);
int NetPlayers_GetSortedPlayerID(int index);
unsigned long Network_GetLocalPlayerID(void);

// Places every car on the starting grid of the current event: stores its
// route position and derives its start x/z from the grid origin, the grid
// heading and the car's slot.
// FUNCTION: CMR2 0x004584d0
void StageTiming_PlaceEventStartingGrid(char param_1)
{
    int value = 0;
    int state = 0;
    int i;
    int slot;
    int n;
    int t;
    short x;
    unsigned int z;
    int sinA;
    int cosA;
    int other;
    char count;
    short a;
    int p0[3];
    int p1[3];

    if ((char)RallyData_GetFlag31() != 0) {
        state = 2;
    } else if (CGameInfo::GetGameModeOptionBit19() != 0 && (char)RallyDataStageIndex() == '\n' &&
               CGameInfo::GetConfiguredGameMode() != '\n') {
        state = 3;
    } else {
        if (g_unk0x00542c68 == 1 || CGameInfo::GetConfiguredGameMode() == '\n')
            return;
        if ((char)RallyData_GetFlag24() == 0 && g_unk0x00542c68 == 2) {
            if ((char)RallyData_GetSelectionFlag27() == 0)
                return;
            state = 1;
        }
    }

    switch (state) {
    case 0:
        RallyData_GetRouteNodeGroundPosition(RallyData_GetRouteAvailabilityState() - 1, p0);
        RallyData_GetRouteNodeGroundPosition(0, p1);
        a = (short)(int)(__int64)((double)StageObject_Atan2Degrees(p1[2] - p0[2], p1[0] - p0[0]) * g_unk0x00511300);
        n = 1;
        sinA = FixMul(0x40000, g_sinTable[a & 0xfff]);
        cosA = FixMul(0x40000, g_sinTable[(a + 0x400) & 0xfff]);
        for (i = 0; i < g_unk0x00542c68; i++, n--) {
            slot = i;
            if (CGameInfo::GetConfiguredGameMode() == 5 && g_unk0x00542c68 > 2)
                slot = RallyTiming_GetChampionshipDriverPosition(i);
            if ((char)RallyDataState() == 2)
                slot = n;
            g_unk0x00542d38[slot] = RallyData_GetRouteAvailabilityState() - 1;
            if (g_unk0x00542c68 > 2)
                t = FixMul(0x50000, (int)(__int64)((double)slot * CGraphics::m_65536)) -
                    (int)(__int64)((double)(g_unk0x00542c68 * 5) * CGraphics::m_65536);
            else
                t = 0xfff60000;
            x = FixMul(t, g_sinTable[(a + 0x400) & 0xfff]);
            z = FixMul(t, g_sinTable[a & 0xfff]);
            if (slot % 2 == 0) {
                x += sinA;
                z -= cosA;
            } else {
                x -= sinA;
                z += cosA;
            }
            g_unk0x00542cd8[i][2] = z + p1[2];
            g_unk0x00542cd8[i][0] = x + p1[0];
        }
        break;
    case 1:
        other = g_unk0x00542c7c[1];
        if (param_1 != 0) {
            slot = g_unk0x00542cd4;
        } else {
            srand(CMain::GetFrameTime());
            slot = rand() <= 0x3fff;
        }
        g_unk0x00542cd4 = slot;
        g_unk0x00542cb0 = slot;
        for (i = 0; i < 2; i++) {
            switch ((slot + i) % 2) {
            case 0:
                value = 0;
                break;
            case 1:
                value = other;
                break;
            }
            g_unk0x00542d38[i] = value;
        }
        break;
    case 2:
        RallyData_GetRouteNodeGroundPosition(RallyData_GetRouteAvailabilityState() - 1, p0);
        a = (short)(int)(__int64)((double)StageObject_Atan2Degrees(p1[2] - p0[2], p1[0] - p0[0]) * g_unk0x00511300);
        sinA = FixMul(0x40000, g_sinTable[a & 0xfff]);
        cosA = FixMul(0x40000, g_sinTable[(a + 0x400) & 0xfff]);
        RallyData_GetRouteNodeGroundPosition(0, p1);
        count = (char)NetPlayers_GetPlayerIDCount();
        for (i = 0; i < count; i++) {
            if (NetPlayers_GetSortedPlayerID(i) == (int)Network_GetLocalPlayerID())
                slot = 0;
            else
                slot = NetPlayers_GetPlayerField8(NetPlayers_FindPlayerIndexByID(NetPlayers_GetSortedPlayerID(i)));
            g_unk0x00542d38[slot] = RallyData_GetRouteAvailabilityState() - 1;
            if (count > 2)
                t = FixMul(0x50000, (int)(__int64)((double)i * CGraphics::m_65536)) -
                    (int)(__int64)((double)(count * 5) * CGraphics::m_65536);
            else
                t = 0xfff60000;
            z = FixMul(t, g_sinTable[a & 0xfff]);
            x = FixMul(t, g_sinTable[(a + 0x400) & 0xfff]);
            if (i % 2 == 0) {
                x += sinA;
                z -= cosA;
            } else {
                z = cosA + z;
                x -= sinA;
            }
            g_unk0x00542cd8[slot][2] = z + p1[2];
            g_unk0x00542cd8[slot][0] = x + p1[0];
        }
        break;
    case 3:
        count = (char)NetPlayers_GetPlayerIDCount();
        for (i = 0; i < count; i++) {
            if (NetPlayers_GetSortedPlayerID(i) == (int)Network_GetLocalPlayerID())
                slot = 0;
            else
                slot = NetPlayers_FindPlayerIndexByID(NetPlayers_GetSortedPlayerID(i));
                slot = NetPlayers_GetPlayerField8(slot);
            if (i % 2 == 0) {
                g_unk0x00542d38[slot] = 0;
                g_unk0x00542cb4[slot] = 0;
            } else {
                g_unk0x00542d38[slot] = g_unk0x00542c7c[1];
                g_unk0x00542cb4[slot] = 1;
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
void ParticleEdit_SetField2CAndFlag5(short param1);
void ParticleEdit_CommitCurrentType(void);
void Particle_SetPoolCapacity(int count);
int WheelSpray_Init(int spread, int range);
void Spark_Draw(Particle *p, ParticleType *pType, SceneNode *pView);
void CarEffects_UpdateAgedParticleAppearance(void *pParticle, ParticleType *pType, int param);
void GlassShard_Draw(Particle *p, ParticleType *pType, int unused);
void GlassShard_Init(Particle *p, ParticleType *pType, Car *pCar);
void Debris_Init(Particle *p, ParticleType *pType, int *pParam);
void Debris_Draw(Particle *p, ParticleType *pType, SceneNode *pView);
void CarEffects_UpdateRisingParticle(void *pParticle, ParticleType *pType, int param);
void CarEffects_UpdateCarFollowingRisingParticle(void *pParticle, ParticleType *pType, int param);
void CarEffect_SpawnExhaustParticle(void *pParticle, ParticleType *pType, int param);
void CarEffects_UpdateSpiralParticle(void *pParticle, ParticleType *pType, int param);
void CarEffects_InitParticleOwnerData(void *pParticle, ParticleType *pType, int param);
void CarEffects_UpdateParticleAtCarPosition(void *pParticle, ParticleType *pType, int param);

// Fills the particle-type table of the stage effects (wheel spray, smoke,
// sparks, glass shards, debris) with their textures, motion and callbacks.
// FUNCTION: CMR2 0x0045a170
void StageTiming_InitEffectParticleTypes(void)
{
    int iVar1;
    int uVar2;
    int unset; // never assigned: the original reads this stack slot as is

  if (CGameInfo::GetGraphicsOptionBits25To26() == 1)
    uVar2 = 100;
  else
    uVar2 = unset;
  iVar1 = CGameInfo::GetGraphicsOptionBits25To26();
  if (iVar1 == 0) {
    uVar2 = 400;
  }
  iVar1 = CGameInfo::GetGraphicsOptionBits25To26();
  if (iVar1 == 2) {
    uVar2 = 0;
  }
  Particle_SetPoolCapacity(uVar2);
  ParticleEdit_Select(0);
  ParticleEdit_SetTextureParams(g_unk0x0054365c,0xffff0000,0x10000,0x10000,0xffff0000);
  ParticleEdit_SetMotion(0xc80000,6,0x10000,0,0,0,0);
  ParticleEdit_SetAlphaRamp(0x1e,0,3,1);
  ParticleEdit_SetSizeRamp(0x28f,0x10000,0x5a1);
  ParticleEdit_SetField2CAndFlag5(0);
  ParticleEdit_SetSpread(0x28f,0x28f,0x28f,1);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(1);
  ParticleEdit_CopyTemplate(0);
  ParticleEdit_SetTextureParams(g_unk0x00543660,0xffff0000,0x10000,0x10000,0xffff0000);
  ParticleEdit_SetAlphaRamp(0x50,0,3,1);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(2);
  ParticleEdit_SetTextureParams(g_unk0x00543664,0xffff0000,0x10000,0x10000,0xffff0000);
  ParticleEdit_SetMotion(0x12c0000,6,0x10000,0,0,0,0);
  ParticleEdit_SetAlphaRamp(100,0,1,1);
  ParticleEdit_SetSizeRamp(0x1999,0x10000,0xb43);
  ParticleEdit_SetField2CAndFlag5(0);
  ParticleEdit_SetSpread(0x28f,0x28f,0x28f,1);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(3);
  ParticleEdit_CopyTemplate(0);
  ParticleEdit_SetMotion(0x40000,0,0,0,0,0,0);
  ParticleEdit_SetSizeRamp(0xccc,0x2147,0xccc);
  ParticleEdit_SetExtendedParams((int)&g_unk0x005436d8,4,0,0x10000,0,0,0xffff0000,0x10000,0x10000,0xffff0000);
  ParticleEdit_SetAlphaRamp(0xfe,0xfe,0,0);
  ParticleEdit_SetSizeRange(0x1999,0x3333);
  ParticleEdit_SetCallbacks(0,0,(void (*)(void *, ParticleType *, int))CarEffect_SpawnExhaustParticle,(void (*)(void *, ParticleType *, int))CarEffects_InitParticleOwnerData,0);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(6);
  ParticleEdit_SetTextureParams(g_unk0x0054366c,0xffff999a,0x6666,0x6666,0xffff999a);
  ParticleEdit_SetMotion(0x210000,0x20,0,0,0,0,0);
  ParticleEdit_SetAlphaRamp(0xd2,0,6,1);
  ParticleEdit_SetColour(0xff,0xff,0xff,1);
  ParticleEdit_SetSizeRamp(0x4ccc,0x18000,0x7ae);
  ParticleEdit_SetField2CAndFlag5(0);
  ParticleEdit_SetSpread(0x51e,0x28f,0x51e,0);
  ParticleEdit_SetCallbacks(0,0,(void (*)(void *, ParticleType *, int))CarEffects_UpdateCarFollowingRisingParticle,(void (*)(void *, ParticleType *, int))CarEffects_InitParticleOwnerData,0);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(4);
  ParticleEdit_CopyTemplate(6);
  ParticleEdit_SetTextureParams(g_unk0x0054366c,0xffff4ccd,0xb333,0xb333,0xffff4ccd);
  ParticleEdit_SetMotion(0x780000,0x20,0,0,0,0,0);
  ParticleEdit_SetAlphaRamp(100,0,2,1);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(7);
  ParticleEdit_CopyTemplate(6);
  ParticleEdit_SetTextureParams(g_unk0x0054366c,0xffff4ccd,0xb333,0xb333,0xffff4ccd);
  ParticleEdit_SetSizeRamp(0x4ccc,0x13333,0x147a);
  ParticleEdit_SetMotion(0x640000,0,0xa0000,0,0,0,0);
  ParticleEdit_SetAlphaRamp(0xd2,0,6,1);
  ParticleEdit_SetColour(0xff,0xff,0xff,1);
  ParticleEdit_SetSizeRamp(0x4ccc,0x18000,0x7ae);
  ParticleEdit_SetCallbacks(0,0,(void (*)(void *, ParticleType *, int))CarEffects_UpdateRisingParticle,0,0);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(10);
  ParticleEdit_CopyTemplate(6);
  ParticleEdit_SetExtendedParams((int)&g_unk0x005436b8,8,0,0x20000,0,0,0xffffb334,0x4ccc,0x4ccc,0xffffb334);
  ParticleEdit_SetAlphaRamp(100,0,0,0);
  ParticleEdit_SetColour(0xff,0xff,0xff,1);
  ParticleEdit_SetCallbacks(0,0,(void (*)(void *, ParticleType *, int))CarEffects_UpdateCarFollowingRisingParticle,(void (*)(void *, ParticleType *, int))CarEffects_InitParticleOwnerData,0);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(8);
  ParticleEdit_CopyTemplate(4);
  ParticleEdit_SetExtendedParams((int)&g_unk0x005436b8,8,0,0x20000,1,1,0xffff4ccd,0xb333,0xb333,0xffff4ccd);
  ParticleEdit_SetColour(0x98,0x7e,0x66,1);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(0xb);
  ParticleEdit_CopyTemplate(7);
  ParticleEdit_SetExtendedParams((int)&g_unk0x005436b8,8,0,0x20000,1,1,0xffff4ccd,0xb333,0xb333,0xffff4ccd);
  ParticleEdit_SetSizeRamp(0x4ccc,0x13333,0x147a);
  ParticleEdit_SetMotion(0x640000,0,0xa0000,0,0,0,0);
  ParticleEdit_SetAlphaRamp(100,0,0,0);
  ParticleEdit_SetColour(0xff,0xff,0xff,1);
  ParticleEdit_SetCallbacks(0,0,(void (*)(void *, ParticleType *, int))CarEffects_UpdateRisingParticle,0,0);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(5);
  ParticleEdit_CopyTemplate(4);
  ParticleEdit_SetTextureParams(g_unk0x0054366c,0xffff999a,0x6666,0x6666,0xffff999a);
  ParticleEdit_SetMotion(0x300000,0x20,0x30000,0,0,0,0);
  ParticleEdit_SetAlphaRamp(100,0,2,1);
  ParticleEdit_SetSizeRamp(0x4ccc,0x10000,0x1999);
  ParticleEdit_SetField2CAndFlag5(0x88);
  ParticleEdit_SetSpread(0,0,0,0);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(0xc);
  ParticleEdit_CopyTemplate(5);
  ParticleEdit_SetExtendedParams((int)&g_unk0x005436b8,8,0,0x20000,1,1,0xffffc000,0x4000,0x8000,0xffffc000);
  ParticleEdit_SetAlphaRamp(0x1e,0,4,1);
  ParticleEdit_SetSizeRamp(0x4ccc,0x5555,0x3332);
  ParticleEdit_SetField2CAndFlag5(0);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(0xd);
  ParticleEdit_CopyTemplate(5);
  ParticleEdit_SetExtendedParams((int)&g_unk0x005436b8,8,0,0x20000,1,1,0xffff0000,0x10000,0x20000,0xffff0000);
  ParticleEdit_SetAlphaRamp(0x46,0,2,1);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(0xe);
  ParticleEdit_CopyTemplate(5);
  ParticleEdit_SetMotion(0x1900000,0,0,0,0,0,0);
  ParticleEdit_SetField2CAndFlag5(0);
  ParticleEdit_SetTextureParams(g_unk0x00543674,0,0x20000,0x20000,0);
  ParticleEdit_SetSizeRamp(0x1999,0x10000,0xa3d);
  ParticleEdit_SetAlphaRamp(0xff,0,0xf,1);
  ParticleEdit_SetSpread(0,0,0,0);
  ParticleEdit_SetCallbacks(0,0,(void (*)(void *, ParticleType *, int))CarEffects_UpdateParticleAtCarPosition,(void (*)(void *, ParticleType *, int))CarEffects_InitParticleOwnerData,0);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(0xf);
  ParticleEdit_CopyTemplate(0xe);
  ParticleEdit_SetTextureParams(g_unk0x00543670,0,0,0x20000,0xfffe0000);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(0x10);
  ParticleEdit_SetExtendedParams((int)&g_unk0x0054367c,5,0x20000,0x10000,1,1,0xffff3334,0xcccc,0xcccc,0xffff3334);
  ParticleEdit_SetMotion(0xc80000,0x28f,0,0,0,0,1);
  ParticleEdit_SetAlphaRamp(0xff,0xff,0,1);
  ParticleEdit_SetColour(0xff,0xff,0xff,1);
  ParticleEdit_SetSizeRange(0x1999,0x147a);
  ParticleEdit_SetField2CAndFlag5(0xb);
  ParticleEdit_SetSpread(0,0,0,0);
  ParticleEdit_SetCallbacks(0,0,(void (*)(void *, ParticleType *, int))CarEffects_UpdateSpiralParticle,(void (*)(void *, ParticleType *, int))CarEffects_InitParticleOwnerData,0);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(0x11);
  ParticleEdit_CopyTemplate(0x10);
  ParticleEdit_SetAlphaRamp(0xff,0xff,0,0);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(0x12);
  ParticleEdit_CopyTemplate(0x10);
  ParticleEdit_SetExtendedParams((int)&g_unk0x00543690,2,0x40000,0x40000,1,1,0xfffe8000,0x18000,0x18000,0xfffe8000);
  ParticleEdit_SetMotion(0xc80000,0x28f,0,0,0,0,1);
  ParticleEdit_SetAlphaRamp(0xff,0xff,0,1);
  ParticleEdit_SetColour(0xff,0xff,0xff,1);
  ParticleEdit_SetSizeRange(0x1999,0x147a);
  ParticleEdit_SetField2CAndFlag5(0xb);
  ParticleEdit_SetSpread(0,0,0,0);
  ParticleEdit_SetCallbacks(0,0,(void (*)(void *, ParticleType *, int))CarEffects_UpdateSpiralParticle,(void (*)(void *, ParticleType *, int))CarEffects_InitParticleOwnerData,0);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(0x13);
  ParticleEdit_CopyTemplate(0x12);
  ParticleEdit_SetAlphaRamp(0xff,0xff,0,0);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(0x14);
  ParticleEdit_CopyTemplate(0x12);
  ParticleEdit_SetExtendedParams((int)&g_unk0x00543698,2,0x40000,0x40000,1,1,0xfffe8000,0x18000,0x18000,0xfffe8000);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(0x15);
  ParticleEdit_CopyTemplate(0x14);
  ParticleEdit_SetAlphaRamp(0xff,0xff,0,0);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(0x17);
  ParticleEdit_CopyTemplate(0x13);
  ParticleEdit_SetAlphaRamp(0xff,0xff,0,0);
  ParticleEdit_SetSizeRange(0x147a,0xa3d);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(0x16);
  ParticleEdit_CopyTemplate(0x12);
  ParticleEdit_SetAlphaRamp(0xff,0xff,0,1);
  ParticleEdit_SetSizeRange(0x147a,0xa3d);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(0x18);
  ParticleEdit_SetTextureParams(g_unk0x005435c8,0xffffc000,0x4000,0x4000,0xffffc000);
  ParticleEdit_SetMotion(0xc80000,0xccc,0,0x1999,0,1,1);
  ParticleEdit_SetAlphaRamp(0xff,0xff,0,1);
  ParticleEdit_SetSizeRamp(0x1999,0x1999,0);
  ParticleEdit_SetField2CAndFlag5(0);
  ParticleEdit_SetSpread(0,0,0,0);
  ParticleEdit_SetCallbacks(0,0,(void (*)(void *, ParticleType *, int))CarEffects_UpdateParticleAtCarPosition,(void (*)(void *, ParticleType *, int))CarEffects_InitParticleOwnerData,0);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(0x1a);
  ParticleEdit_SetTextureParams(g_unk0x00543650,0xffffcccd,0x3333,0x3333,0xffffcccd);
  ParticleEdit_SetMotion(0x140000,0x147,0,0,0,0,1);
  ParticleEdit_SetAlphaRamp(0xfe,0,0x22,1);
  ParticleEdit_SetSizeRamp(0x10000,0x10000,0);
  ParticleEdit_SetField2CAndFlag5(0);
  ParticleEdit_SetSpread(0,0,0,0);
  ParticleEdit_SetCallbacks((int)Spark_Draw,0,(void (*)(void *, ParticleType *, int))CarEffects_UpdateAgedParticleAppearance,0,0);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(0x1b);
  ParticleEdit_SetTextureParams(g_unk0x00543654,0xffffcccd,0x3333,0x3333,0xffffcccd);
  ParticleEdit_SetMotion(0x140000,0x147,0,0,0,0,1);
  ParticleEdit_SetAlphaRamp(0xff,0xff,0,0);
  ParticleEdit_SetSizeRamp(0x10000,0x10000,0);
  ParticleEdit_SetField2CAndFlag5(0);
  ParticleEdit_SetSpread(0,0,0,0);
  ParticleEdit_SetCallbacks((int)GlassShard_Draw,0,0,(void (*)(void *, ParticleType *, int))GlassShard_Init,0);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(0x1c);
  ParticleEdit_SetTextureParams(g_unk0x00543658,0xffffcccd,0x3333,0x3333,0xffffcccd);
  ParticleEdit_SetMotion(0x640000,0x189,0,0,0,0,1);
  ParticleEdit_SetAlphaRamp(0xff,0xff,0,0);
  ParticleEdit_SetSizeRamp(0x10000,0x10000,0);
  ParticleEdit_SetField2CAndFlag5(0);
  ParticleEdit_SetSpread(0,0,0,0);
  ParticleEdit_SetCallbacks((int)Debris_Draw,0,0,(void (*)(void *, ParticleType *, int))Debris_Init,0);
  ParticleEdit_CommitCurrentType();
  iVar1 = WheelSpray_Init(0x10000,0x190000);
  ParticleEdit_Select(0x1d);
  ParticleEdit_SetExtendedParams((int)&g_unk0x005436a0,5,0x20000,0x10000,1,1,0xfffff99a,0x666,0x666,0xfffff99a);
  ParticleEdit_SetMotion(0xf0000,FixMul(iVar1, 0x3333),0,0xb333,0,1,0);
  ParticleEdit_SetAlphaRamp(0xff,0xff,0,0);
  ParticleEdit_SetColour(0xff,0,0xff,1);
  ParticleEdit_SetSizeRange(0x10000,0x20000);
  ParticleEdit_SetField2CAndFlag5(0);
  ParticleEdit_SetSpread(0,0,0,0);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(0x1e);
  ParticleEdit_SetExtendedParams((int)&g_unk0x005436a0,5,0x20000,0x10000,1,1,0xfffff70b,0x8f5,0x8f5,0xfffff70b);
  ParticleEdit_SetMotion(0x190000,iVar1,0,0xb333,0,1,0);
  ParticleEdit_SetAlphaRamp(0xff,0xff,0,0);
  ParticleEdit_SetColour(0xff,0,0xff,1);
  ParticleEdit_SetSizeRange(0x10000,0x20000);
  ParticleEdit_SetField2CAndFlag5(0);
  ParticleEdit_SetSpread(0,0,0,0);
  ParticleEdit_CommitCurrentType();
  ParticleEdit_Select(0x1f);
  ParticleEdit_CopyTemplate(0x1a);
  ParticleEdit_SetTextureParams(g_unk0x00547fb4,0xffffe667,0x1999,0x1999,0xffffe667);
  ParticleEdit_SetCallbacks(0,0,0,0,0);
  ParticleEdit_CommitCurrentType();
  return;
}

void Car_Spawn(int param_1, int param_2, int param_3, int *param_4, int param_5, int param_6);
void Car_ResetBodyBasis(int param_1);
void StageTiming_IntegratePartSpringMotion(void);

// One wheel/hub record (stride 0x1a0) of the four slots CarPart_InitWheelHubSlot installs:
// the hub transform, the offsets accumulated during the physics step and the
// collision state used while resolving ground contact.
inline void Motion_NormalizeInto(FixVector *out, FixVector *v)
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



int VehiclePhysics_TestMotionCorrectionCollision(void);
void Vehicle_UpdateMotion(FixVector *pInput);
void StageTiming_FireCarPartDebris(int *param_1, unsigned int param_2);
extern double g_unk0x00511308;

// Per-slot integrator for slot 1 (CarPart_InitWheelHubSlot installs its address at
// +0x110): swings the hub about its axis, resolves the ground contact of the
// part against the three contact planes, re-orthogonalises the part's basis
// and finally integrates the motion offset into the world position.
// match 65%: logic checked against the original; the stack slots differ.
// FUNCTION: CMR2 0x004816f0
void CarPart_IntegrateHubGroundContact(void)
{
    int detached;
    int len;
    FixVector saved;
    FixVector cross;
    FixVector spring;
    FixVector turn;
    FixVector torque;
    FixVector v4c;
    FixVector v3c;
    FixVector force;
    FixVector v24;
    unsigned short angles[3];
    int idx;
    unsigned int dot;
    int scale;
    int flip;
    int t;
    int s;
    int c;
    short a;
    BYTE type;

    if (VehiclePhysics_TestMotionCorrectionCollision() != 0)
        return;
    type = g_partCar->type;
    if (type != 9 && type != 11) {
        if ((g_partState->flags & 6) == 0)
            StageTiming_StartLoadedPartSwing();
        torque.x = 0;
        torque.y = 0;
        torque.z = 0;
        force.x = 0;
        force.y = 0;
        force.z = 0;
        detached = 0;
        if (g_partState->flags & 2) {
            s = g_sinTable[(unsigned short)g_partState->angle & 0xfff];
            c = g_sinTable[((unsigned short)g_partState->angle + 0x400) & 0xfff];
            spring.x = 0;
            spring.y = 0;
            spring.z = 0;
            if (g_partState->position.z > 0) {
                v24.x = s - g_partState->basis.forward.x;
                v24.y = -g_partState->basis.forward.y;
                v24.z = c - g_partState->basis.forward.z;
            } else {
                v24.x = g_partState->basis.forward.x - s;
                v24.y = g_partState->basis.forward.y;
                v24.z = g_partState->basis.forward.z - c;
            }
            len = FixVecLength(&v24);
            if (len > 0) {
                FixVecScaleRecip(&v24, &v24, len);
                len = -FixMul(len, 0x40000);
                FixVecScale(&spring, &v24, len);
            }
            idx = 0;
            v4c.x = *(int *)(((BYTE *)g_partCar) + 0x408) - *(int *)(((BYTE *)g_partCar) + 0x414);
            v4c.y = *(int *)(((BYTE *)g_partCar) + 0x40c) - *(int *)(((BYTE *)g_partCar) + 0x418);
            v4c.z = *(int *)(((BYTE *)g_partCar) + 0x410) - *(int *)(((BYTE *)g_partCar) + 0x41c);
            v3c.x = 0;
            v3c.y = 0;
            v3c.z = 0;
            if (g_partState->position.z > 0)
                idx = 1;
            if ((g_partState->flags & 0xf0) == 0x10)
                idx += 2;
            if (g_partCar->cornerOnGround[idx] != 0) {
                dot = FixVecDot((FixVector *)(((BYTE *)g_partCar) + 0x408), (FixVector *)(((BYTE *)g_partCar) + 0x48c));
                FixVecScale(&v3c, (FixVector *)(((BYTE *)g_partCar) + 0x48c), dot);
                v3c.x = *(int *)(((BYTE *)g_partCar) + 0x408) - v3c.x;
                v3c.y = *(int *)(((BYTE *)g_partCar) + 0x40c) - v3c.y;
                v3c.z = *(int *)(((BYTE *)g_partCar) + 0x410) - v3c.z;
                FixVecScale(&v3c, &v3c, 0x3333);
                StageTiming_FireCarPartDebris((int *)&v3c, idx);
            }
            v24.x = v3c.x + v4c.x;
            v24.y = v3c.y + v4c.y;
            v24.z = v3c.z + v4c.z;
            len = FixVecLength(&v24);
            if (len > 0x4ccc) {
                dot = FixVecDot(&v24, (FixVector *)(((BYTE *)g_partCar) + 0x360));
                if ((g_partState->flags & 0xf0) == 0x10) {
                    if (g_partState->position.z < 0) {
                        scale = -0x10000;
                        flip = 1;
                    } else {
                        scale = 0x10000;
                        flip = 0;
                    }
                } else {
                    if (g_partState->position.z < 0) {
                        scale = -0x10000;
                        flip = 0;
                    } else {
                        scale = 0x10000;
                        flip = 1;
                    }
                }
                if (dot < 0) {
                    scale = FixMul(scale, -0x10000);
                    dot = -dot;
                }
                if (dot > 0x4ccc) {
                    t = FixMul(dot - 0x4ccc, FixDiv(0x10000, 0x4ccc));
                    if (t > 0x10000)
                        t = 0x10000;
                    t = FixMul(t, 0x50000);
                    g_partState->angle =
                        (short)(__int64)((double)(g_partState->angle * 0x1680 + FixMul(t, scale)) * g_unk0x00511300);
                    if (flip) {
                        if (g_partState->angle > 0)
                            g_partState->angle = 0;
                    } else {
                        if (g_partState->angle < 0)
                            g_partState->angle = 0;
                    }
                    if (FIX_ABS(g_partState->angle) > 0xe3)
                        detached = 1;
                }
                FixVecScaleRecip(&v24, &v24, len);
                FixVecScale(&v24, &v24, 0x4ccc);
            }
            if (detached == 0) {
                FixVecScale(&v4c, &v4c, -0xa0000);
                force.x += v4c.x;
                force.y += v4c.y;
                force.z += v4c.z;
                FixVecScale(&v3c, &v3c, -0x40000);
                force.x += v3c.x;
                force.y += v3c.y;
                force.z += v3c.z;
                force.y -= 0x4000;
            } else {
                FixVecScale(&v4c, &v4c, -0xa0000);
                force.x += v4c.x;
                force.y += v4c.y;
                force.z += v4c.z;
                FixVecScale(&v3c, &v3c, -0x140000);
                force.x += v3c.x;
                force.y += v3c.y;
                force.z += v3c.z;
                saved = force;
            }
            t = 0x10000 - g_physicsTimeStep;
            if (t < 0)
                t = 0;
            else if (t > 0x8000)
                t = 0x8000;
            t = FixMul(t, 0xc937);
            t += 0x8000;
            FixVecScale(&g_partState->angularVelocity, &g_partState->angularVelocity, t);
            FixMatrix_InverseRotateVector(&v24, &force, g_partState->pForceFrame);
            force = v24;
            if (detached == 0) {
                force.x += spring.x;
                force.y += spring.y;
                force.z += spring.z;
            } else {
                g_partState->angularVelocity.x = 0;
                g_partState->angularVelocity.y = 0;
                g_partState->angularVelocity.z = 0;
            }
            v24.x = g_partState->previousPosition.x - g_partState->position.x;
            v24.y = g_partState->previousPosition.y - g_partState->position.y;
            v24.z = g_partState->previousPosition.z - g_partState->position.z;
            FixVecCross(&cross, &force, &v24);
            if (detached == 0)
                cross.z = 0;
            torque.x = -FixMul(cross.x, g_partState->stiffness.x);
            torque.y = -FixMul(cross.y, g_partState->stiffness.y);
            torque.z = -FixMul(cross.z, g_partState->stiffness.z);
            FixVecScale(&torque, &torque, g_physicsTimeStep);
            g_partState->angularVelocity.x += torque.x;
            g_partState->angularVelocity.y += torque.y;
            g_partState->angularVelocity.z += torque.z;
        }
        if (g_partState->flags & 6) {
            g_partState->flags |= 8;
            if (detached == 0 && (g_partState->flags & 4) == 0 && *(int *)(((BYTE *)g_partCar) + 0x778) <= 0x7ae)
                g_partState->flags &= ~8;
            if ((g_partState->flags & 2) && !(g_partState->flags & 8)) {
                g_partState->angularVelocity.x = 0;
                g_partState->angularVelocity.y = 0;
                g_partState->angularVelocity.z = 0;
            } else {
                FixVecScale(&turn, &g_partState->angularVelocity, g_physicsTimeStep);
                FixVecScale(&torque, &torque, g_physicsTimeStep / 2);
                turn.x -= torque.x;
                turn.y -= torque.y;
                turn.z -= torque.z;
                angles[0] = (short)(__int64)((double)turn.x * g_unk0x00511380);
                angles[1] = (short)(__int64)((double)turn.y * g_unk0x00511380);
                angles[2] = (short)(__int64)((double)turn.z * g_unk0x00511380);
                FixBasis_Rotate(&g_partState->basis, angles);
            }
        }
        if ((g_partState->flags & 2) && (g_partState->flags & 8) && detached == 0) {
            flip = 0;
            if ((g_partState->flags & 0xf0) == 0x10) {
                if (g_partState->position.z > 0) {
                    if (g_partState->basis.forward.x < 0) {
                        g_partState->basis.forward.x = 0;
                        g_partState->angularVelocity.x = 0;
                        flip = 1;
                    }
                    if (g_partState->basis.forward.y < 0) {
                        g_partState->basis.forward.y = 0;
                        g_partState->angularVelocity.y = 0;
                        goto renormalize;
                    }
                } else {
                    if (g_partState->basis.forward.x > 0) {
                        g_partState->basis.forward.x = 0;
                        flip = 1;
                        g_partState->angularVelocity.x = 0;
                    }
                    if (g_partState->basis.forward.y > 0) {
                        g_partState->basis.forward.y = 0;
                        g_partState->angularVelocity.y = 0;
                        goto renormalize;
                    }
                }
            } else {
                if (g_partState->position.z > 0) {
                    if (g_partState->basis.forward.x > 0) {
                        g_partState->basis.forward.x = 0;
                        flip = 1;
                        g_partState->angularVelocity.x = 0;
                    }
                    if (g_partState->basis.forward.y < 0) {
                        g_partState->basis.forward.y = 0;
                        g_partState->angularVelocity.y = 0;
                        goto renormalize;
                    }
                } else {
                    if (g_partState->basis.forward.x < 0) {
                        g_partState->basis.forward.x = 0;
                        g_partState->angularVelocity.x = 0;
                        flip = 1;
                    }
                    if (g_partState->basis.forward.y > 0) {
                        g_partState->basis.forward.y = 0;
                        g_partState->angularVelocity.y = 0;
                        goto renormalize;
                    }
                }
            }
            if (flip) {
renormalize:
                Motion_NormalizeInto(&g_partState->basis.forward, &g_partState->basis.forward);
                g_partState->basis.up.x = 0;
            }
            dot = FixVecDot(&g_partState->basis.forward, &g_partState->basis.up);
            FixVecScale(&v24, &g_partState->basis.forward, dot);
            v24.x = g_partState->basis.up.x - v24.x;
            v24.y = g_partState->basis.up.y - v24.y;
            v24.z = g_partState->basis.up.z - v24.z;
            Motion_NormalizeInto(&g_partState->basis.up, &v24);
            FixVecCross(&v24, &g_partState->basis.up, &g_partState->basis.forward);
            Motion_NormalizeInto(&g_partState->basis.right, &v24);
        }
        if ((g_partState->flags & 4) == 0) {
            angles[0] = 0;
            angles[1] = 0;
            angles[2] = 0;
            if ((g_partState->flags & 2) == 0) {
                a = (short)(__int64)((double)FixMul((short)g_unk0x00590c68 * 0x1680, 0x10000) * g_unk0x00511300);
                if ((g_partState->flags & 0xf0) == 0x10) {
                    angles[2] = a;
                    StageTiming_RotatePartBasisFromAngles(angles);
                } else {
                    angles[2] = -a;
                    StageTiming_RotatePartBasisFromAngles(angles);
                }
            } else {
                a = (short)(__int64)((double)FixMul((short)g_unk0x00590c68 * 0x1680, 0x8000) * g_unk0x00511300);
                if ((g_partState->flags & 0xf0) == 0x10) {
                    if (g_partState->position.z > 0) {
                        angles[0] = -a;
                    } else {
                        angles[0] = a;
                        a = -a;
                    }
                } else {
                    if (g_partState->position.z > 0)
                        a = -a;
                    angles[0] = a;
                }
                angles[1] = a;
                StageTiming_RotatePartBasisFromAngles(angles);
            }
        } else {
            FixMatrix_SetRight(&g_partState->basis.right, g_partState->pWorld);
            FixMatrix_SetUp(&g_partState->basis.up, g_partState->pWorld);
            FixMatrix_SetForward(&g_partState->basis.forward, g_partState->pWorld);
        }
    } else if (type == 9) {
        detached = 0;
        angles[0] = 0;
        angles[1] = (short)(__int64)((double)FixMul((short)g_unk0x00590c68 * 0x1680, 0x10000) * g_unk0x00511308);
        angles[2] = 0;
        StageTiming_RotatePartBasisFromAngles(angles);
    } else {
        detached = 0;
        angles[0] = 0;
        angles[1] = (short)(__int64)((double)FixMul((short)g_unk0x00590c68 * 0x1680, 0x20000) * g_unk0x00511308);
        angles[2] = 0;
        StageTiming_RotatePartBasisFromAngles(angles);
    }
    VehicleMotion_UpdateWorldPosition();
    if (detached != 0)
        Vehicle_UpdateMotion(&saved);
}

// Defined in StageObjects.cpp; CarPart_InitWheelHubSlot installs this callback.
void CarPart_IntegrateDetachedMotion(void);

// Places the two view nodes of a car at the shared angle/position buffers and
// rebuilds that car's body state, network-snapshotting it when required.
// FUNCTION: CMR2 0x00457e50
void StageTiming_PlaceCarViewNodesAndBody(SceneNode *pNodeA, SceneNode *pNodeB, int carIndex, int param_4,
                  FixAngles *pAngles, FixVector *pPosition, int param_7)
{
    SceneNode_SetRotation(pNodeA, pAngles);
    SceneNode_SetRotation(pNodeB, pAngles);
    SceneNode_SetPosition(pNodeA, pPosition);
    SceneNode_SetPosition(pNodeB, pPosition);
    Car_Spawn((int)Car_Get(carIndex), (int)pNodeA, param_4, (int *)pPosition, carIndex, param_7);
    Car_ResetBodyBasis((int)Car_Get(carIndex));
    if (CGameInfo::GetGameModeOptionBit19() != 0)
        StageTiming_SnapshotCarNetworkPose(Car_Get(carIndex));
}

// Initialises one of a car's four wheel/hub records: points the shared record
// pointer at the car's slot, picks the static geometry for the wheel type,
// installs the per-slot integrator and clears the dynamic state.
// match 13%: implementada; MSVC6 se queda con g_partState y el puntero base en registro
// en vez de releerlos del global en cada acceso como hace el original
// FUNCTION: CMR2 0x00480e50
void CarPart_InitWheelHubSlot(int slot)
{
    int type = g_unk0x00590c24[slot][g_partCar->index];
    FixVector off;

    off.x = 0;
    off.z = 0;
    off.y = 0;

    g_partState =
        (PartState *)((BYTE *)g_unk0x00590d7c[slot] + g_partCar->index * 0x1a0);
    if (StageTiming_AttachObjectToCurrentCar(slot) == 0)
        return;
    g_partState->flags = (g_partState->flags & 0xf) | (slot << 4);
    g_partState->flags |= 1;
    g_partState->flags &= ~2;
    g_partState->flags &= ~4;
    g_partState->flags &= ~8;
    g_partState->pNode->useParentWorld = 0;
    switch (slot) {
    case 0:
        off.x = -g_partSet->halfExtents[type].x;
        off.y = g_partSet->halfExtents[type].y;
        off.z = 0;
        g_partState->angle = 0x288;
        g_partState->update = CarPart_IntegrateVerticalAxisTurn;
        break;
    case 2:
        if (g_partCar->type == 11) {
            off.x = 0;
            off.y = g_partSet->halfExtents[type].y;
            off.z = -g_partSet->halfExtents[type].z;
        } else if (g_partCar->type == 9) {
            off.x = -g_partSet->halfExtents[type].x;
            off.y = 0;
            off.z = g_partSet->halfExtents[type].z;
        } else {
            if (g_partCar->type == 8) {
                off.x = -g_partSet->halfExtents[type].x;
                off.y = -g_partSet->halfExtents[type].y;
            } else {
                off.x = g_partSet->halfExtents[type].x;
                off.y = g_partSet->halfExtents[type].y;
            }
            off.z = 0;
        }
        g_partState->angle = 0x288;
        g_partState->update = CarPart_IntegrateDetachedMotion;
        break;
    case 1:
        if (g_partCar->type != 9 && g_partCar->type != 11) {
            off.x = FixMul(g_partSet->halfExtents[type].x, 0x8000);
            off.y = g_partSet->halfExtents[type].y;
            off.z = 0;
        } else {
            off.y = 0;
            off.x = g_partSet->halfExtents[type].x;
            off.z = -g_partSet->halfExtents[type].z;
        }
        g_partState->angle = 0;
        g_partState->update = CarPart_IntegrateHubGroundContact;
        g_partState->stiffness.x = 0x10e5;
        g_partState->stiffness.y = 0x1eb8;
        g_partState->stiffness.z = 0x10e5;
        goto common;
    case 3:
        g_partState->flags |= 2;
        if (g_partCar->type == 8) {
            off.x = 0;
            off.z = 0;
            off.y = g_partSet->halfExtents[type].y;
        } else {
            off.x = 0;
            if (*(int *)(((BYTE *)g_partSet) + 0x248) > *(int *)(((BYTE *)g_partSet) + 0x24c)) {
                off.z = g_partSet->halfExtents[type].z;
                off.y = -g_partSet->halfExtents[type].y;
            } else {
                off.z = -g_partSet->halfExtents[type].z;
                off.y = -g_partSet->halfExtents[type].y;
            }
        }
        g_partState->angle = 0;
        g_partState->update = StageTiming_IntegratePartSpringMotion;
        break;
    default:
        goto common;
    }
    g_partState->stiffness.x = 0x10e5;
    g_partState->stiffness.y = 0x1eb8;
    g_partState->stiffness.z = 0x6ccc;
common:
    g_partState->position.x = g_partSet->centres[type].x + off.x;
    g_partState->position.y = g_partSet->centres[type].y + off.y;
    g_partState->position.z = g_partSet->centres[type].z + off.z;
    g_partState->field_0x15c = FixVecLength(&g_partSet->halfExtents[type]);
    g_partState->previousPosition = g_partSet->centres[type];
    g_partState->swingAxis = g_partSet->halfExtents[type];
    g_partState->velocity.x = 0;
    g_partState->velocity.y = 0;
    g_partState->velocity.z = 0;
    g_partState->correction.x = 0;
    g_partState->correction.y = 0;
    g_partState->correction.z = 0;
    g_partState->angularVelocity.x = 0;
    g_partState->angularVelocity.y = 0;
    g_partState->angularVelocity.z = 0;
    g_partState->field_0x154 = 0;
    g_partState->field_0x160 = 0;
}
int StageObject_IsEligibleType(short type, int mode, int category);
void Car_SpawnDebris(int size, FixVector *pPos, Car *pCar, FixVector *pAxes, int count, int glassChance);

// Aims and fires the debris/particles of a car part: rotates the part's local
// offset into world space, normalises the part's direction, and spawns debris
// along the cross product of the two.
// match 44%: implementada, difiere la extension del argumento short y el reparto de locales/registros
// FUNCTION: CMR2 0x00483100
void StageTiming_FireCarPartDebris(int *param_1, unsigned int param_2)
{
    int sel = g_unk0x00590c24[g_partState->flags >> 4][*(char *)(((BYTE *)g_partCar) + 0xb1a)];
    FixVector pos;
    FixVector offset;
    FixVector debrisAxes[3];
    int len;

    if (StageObject_IsEligibleType(*(short *)(((BYTE *)g_partCar) + 0xaae + (param_2 & 0xff) * 2), 0, 0) != 0) {
        pos = g_partState->position;
        pos.y -= FixMul(0x20000, g_partSet->halfExtents[sel].y);
        if (g_partState->position.z > 0)
            FixVecScale(&offset, &g_partState->basis.forward,
                        -FixMul(0x1cccc, g_partSet->halfExtents[sel].z));
        else
            FixVecScale(&offset, &g_partState->basis.forward,
                        FixMul(0x1cccc, g_partSet->halfExtents[sel].z));
        pos.x += offset.x;
        pos.y += offset.y;
        pos.z += offset.z;
        FixMatrix_RotateVector(&offset, &pos, *(FixMatrix **)(((BYTE *)g_partCar) + 0x750));
        len = FixVecLength((FixVector *)param_1);
        if (len > 0) {
            FixVecScaleRecip(&debrisAxes[0], (FixVector *)param_1, -len);
            len = FixMul(len, 0x50000);
            if (len > 0x10000)
                len = 0x10000;
            else if (len <= 0x3333)
                return;
            debrisAxes[1] = *(FixVector *)(((BYTE *)g_partCar) + 0x48c);
            FixVecCross(&debrisAxes[2], &debrisAxes[0], &debrisAxes[1]);
            Motion_NormalizeInto(&debrisAxes[2], &debrisAxes[2]);
            Car_SpawnDebris(len, &offset, (Car *)((BYTE *)g_partCar), &debrisAxes[0], 0x40000, 0x6666);
        }
    }
}

int Track_GetGroundHeight(FixVector *pPoint, FixVector *pNormal, short *pTri, unsigned short *pSurface, int defaultY);

// Places one of a car's four contact points on the ground: rotates the local
// point into world space, queries the track height, then builds the tangent
// direction (world axes minus the normal component) and stores the resulting
// contact plane on the point.
#define CAR_0x590d74 ((BYTE *)g_partCar)

// FUNCTION: CMR2 0x00484f40
void CarPart_ResolveLocalPointGroundContact(unsigned int param_1)
{
    unsigned int index = param_1 & 0xff;
    int idx = *(char *)(CAR_0x590d74 + 0xb1a) * 4;
    int *pPoint = (int *)(*(int *)((BYTE *)g_unk0x00590c6c + idx) + index * 0x3c);
    BYTE *pLocal = (BYTE *)(*(int *)((BYTE *)g_unk0x00590c00 + idx) + index * 0x20);
    FixVector normal;
    FixVector proj;
    FixVector dir;
    short tri;
    int surface;
    int len;
    int len2;

    if (pPoint[0xe] != 0)
        return;
    surface = 0;
    tri = 0;
    FixMatrix_RotateVector((FixVector *)pPoint, (FixVector *)pLocal, *(FixMatrix **)(CAR_0x590d74 + 0x750));
    pPoint[0] = pPoint[0] + *(int *)(CAR_0x590d74 + 0x2d0);
    pPoint[1] = pPoint[1] + *(int *)(CAR_0x590d74 + 0x2d4);
    pPoint[2] = pPoint[2] + *(int *)(CAR_0x590d74 + 0x2d8);
    pPoint[1] = Track_GetGroundHeight((FixVector *)pPoint, &normal, &tri, (unsigned short *)&surface, pPoint[1]);
    FixMatrix_RotateVector(&dir, (FixVector *)(pLocal + 0xc), *(FixMatrix **)(CAR_0x590d74 + 0x750));

    // tangent = dir minus its normal component, normalised; falls back to the x axis
    len = FixVecDot(&dir, &normal);
    FixVecScale(&proj, &normal, len);
    dir.x = dir.x - proj.x;
    dir.y = dir.y - proj.y;
    dir.z = dir.z - proj.z;
    len = FixVecLength(&dir);
    if (len == 0) {
        dir.x = 0x10000;
        dir.y = 0;
        dir.z = 0;
        len = FixVecDot(&dir, &normal);
        FixVecScale(&proj, &normal, len);
        dir.x = dir.x - proj.x;
        dir.y = dir.y - proj.y;
        dir.z = dir.z - proj.z;
        len2 = FixVecLength(&dir);
        if (len2 == 0) {
            dir.x = 0;
            dir.y = 0;
            dir.z = 0;
        } else {
            FixVecScaleRecip(&dir, &dir, len2);
        }
    } else {
        FixVecScaleRecip(&dir, &dir, len);
    }
    FixVecScale(&dir, &dir, *(int *)(pLocal + 0x18));
    pPoint[9] = pPoint[0] + dir.x;
    pPoint[10] = pPoint[1] + dir.y;
    pPoint[0xb] = pPoint[2] + dir.z;
    pPoint[0xe] = 1;
    g_partSet->field_0x4c0[index] = 1;
}

#undef CAR_0x590d74


void Vehicle_UpdateMotion(FixVector *pInput);
int VehiclePhysics_TestMotionCorrectionCollision(void);

// Steps the motion state at g_partState for the current car: a spring
// pulls its forward axis toward the rest heading (0x158), the car's
// acceleration (brought into its space) adds a torque that is integrated into
// the angular velocity (0x164) and rotates the basis at 0x17c, which is then
// re-orthonormalised against the car's axis. A hard hit hands the impulse to
// Vehicle_UpdateMotion instead.
// FUNCTION: CMR2 0x00483570
void StageTiming_IntegratePartSpringMotion(void)
{
    FixVector saved;
    FixVector cross;
    FixVector spring;
    FixVector turn;
    FixVector torque;
    FixVector force;
    FixVector v;
    unsigned short angles[3];
    int detached;
    int t;
    int s;
    int c;

    if (g_partCar->type != 8) {
        if (VehiclePhysics_TestMotionCorrectionCollision() != 0)
            return;
        detached = 0;
        if ((g_partState->flags & 4) == 0) {
            s = g_sinTable[(unsigned short)g_partState->angle & 0xfff];
            c = g_sinTable[((unsigned short)g_partState->angle + 0x400) & 0xfff];
            spring.x = 0;
            spring.y = 0;
            spring.z = 0;
            if (g_partState->position.z > 0) {
                v.x = s - g_partState->basis.forward.x;
                v.y = -g_partState->basis.forward.y;
                v.z = c - g_partState->basis.forward.z;
            } else {
                v.x = g_partState->basis.forward.x - s;
                v.y = g_partState->basis.forward.y;
                v.z = g_partState->basis.forward.z - c;
            }
            t = FixVecLength(&v);
            if (t > 0) {
                FixVecScaleRecip(&v, &v, t);
                t = -FixMul(t, 0x40000);
                FixVecScale(&spring, &v, t);
            }
            force.x = (g_partCar)->velocity.x - (g_partCar)->velocityNext.x;
            force.y = (g_partCar)->velocity.y - (g_partCar)->velocityNext.y;
            force.z = (g_partCar)->velocity.z - (g_partCar)->velocityNext.z;
            force.x = FixMul(force.x, -0x50000);
            force.y = FixMul(force.y, -0xa0000);
            force.z = FixMul(force.z, -0x50000);
            if (FixMul(force.x, force.x) + FixMul(force.z, force.z) > 0x64000 &&
                *(int *)(((BYTE *)g_partSet) + 0x288) > 0x8000) {
                saved = force;
                detached = 1;
            } else {
                force.y -= 0x4000;
            }
            t = 0x10000 - g_physicsTimeStep;
            if (t < 0)
                t = 0;
            else if (t > 0x8000)
                t = 0x8000;
            t = FixMul(t, 0xc937);
            t += 0x8000;
            FixVecScale(&g_partState->angularVelocity, &g_partState->angularVelocity, t);
            FixMatrix_InverseRotateVector(&v, &force, g_partState->pForceFrame);
            force = v;
            if (detached == 0) {
                force.x += spring.x;
                force.y += spring.y;
                force.z += spring.z;
            } else {
                g_partState->angularVelocity.x = 0;
                g_partState->angularVelocity.y = 0;
                g_partState->angularVelocity.z = 0;
            }
            v.x = g_partState->previousPosition.x - g_partState->position.x;
            v.y = g_partState->previousPosition.y - g_partState->position.y;
            v.z = g_partState->previousPosition.z - g_partState->position.z;
            FixVecCross(&cross, &force, &v);
            torque.x = -FixMul(cross.x, g_partState->stiffness.x);
            torque.y = -FixMul(cross.y, g_partState->stiffness.y);
            torque.z = -FixMul(cross.z, g_partState->stiffness.z);
            FixVecScale(&torque, &torque, g_physicsTimeStep);
            g_partState->angularVelocity.x += torque.x;
            g_partState->angularVelocity.y += torque.y;
            g_partState->angularVelocity.z += torque.z;
            g_partState->angularVelocity.z = 0;
        } else {
            torque.x = 0;
            torque.y = 0;
            torque.z = 0;
        }
        g_partState->flags |= 8;
        if (detached == 0 && (g_partState->flags & 4) == 0 && (g_partCar)->speed <= 0x7ae)
            g_partState->flags &= ~8;
        if (g_partState->flags & 8) {
            FixVecScale(&turn, &g_partState->angularVelocity, g_physicsTimeStep);
            FixVecScale(&torque, &torque, g_physicsTimeStep / 2);
            turn.x -= torque.x;
            turn.y -= torque.y;
            turn.z -= torque.z;
            angles[0] = (short)(__int64)((double)turn.x * g_unk0x00511380);
            angles[1] = (short)(__int64)((double)turn.y * g_unk0x00511380);
            angles[2] = (short)(__int64)((double)turn.z * g_unk0x00511380);
            FixBasis_Rotate(&g_partState->basis, angles);
        } else {
            g_partState->angularVelocity.x = 0;
            g_partState->angularVelocity.y = 0;
            g_partState->angularVelocity.z = 0;
        }
        if (detached == 0 && (g_partState->flags & 4) == 0) {
            t = FixVecDot(&g_partState->basis.forward,
                          (FixVector *)g_unk0x00590b5c[(g_partCar)->index]);
            if ((g_partState->flags & 8) &&
                ((g_partState->position.z < 0 && t < 0) || (g_partState->position.z > 0 && t > 0))) {
                FixVecScale(&v, (FixVector *)g_unk0x00590b5c[(g_partCar)->index], t);
                v.x = g_partState->basis.forward.x - v.x;
                v.y = g_partState->basis.forward.y - v.y;
                v.z = g_partState->basis.forward.z - v.z;
                Motion_NormalizeInto(&g_partState->basis.forward, &v);
                g_partState->basis.right.y = 0;
                t = FixVecDot(&g_partState->basis.forward, &g_partState->basis.right);
                FixVecScale(&v, &g_partState->basis.forward, t);
                v.x = g_partState->basis.right.x - v.x;
                v.y = g_partState->basis.right.y - v.y;
                v.z = g_partState->basis.right.z - v.z;
                Motion_NormalizeInto(&g_partState->basis.right, &v);
                FixVecCross(&v, &g_partState->basis.forward, &g_partState->basis.right);
                Motion_NormalizeInto(&g_partState->basis.up, &v);
                t = FixVecDot((FixVector *)g_unk0x00590b5c[(g_partCar)->index],
                              &g_partState->angularVelocity);
                FixVecScale(&g_partState->angularVelocity,
                            (FixVector *)g_unk0x00590b5c[(g_partCar)->index], t);
            }
                t = FixMul((short)g_unk0x00590c68 * 0x1680, 0x8000);
            if (g_partState->position.z < 0)
                t = -t;
            angles[1] = 0;
            angles[2] = 0;
            angles[0] = (short)(__int64)((double)t * g_unk0x00511300);
            StageTiming_RotatePartBasisFromAngles(angles);
        } else {
            FixMatrix_SetRight(&g_partState->basis.right, g_partState->pWorld);
            FixMatrix_SetUp(&g_partState->basis.up, g_partState->pWorld);
            FixMatrix_SetForward(&g_partState->basis.forward, g_partState->pWorld);
        }
    } else {
        detached = 0;
        angles[0] = 0;
        angles[1] = 0;
        angles[2] = (short)(__int64)((double)FixMul((short)g_unk0x00590c68 * 0x1680, 0x50000) * g_unk0x00511308);
        StageTiming_RotatePartBasisFromAngles(angles);
    }
    VehicleMotion_UpdateWorldPosition();
    if (detached != 0)
        Vehicle_UpdateMotion(&saved);
}


extern int g_unk0x00588970[8];
void ForceFeedback_UpdateSlot(BYTE *pCar, FixVector *pIn, int nonzero);
void Race_PlayScrapeAndShakeCar(int view, int strength, int listener);
void Race_PlayImpactAndShakeCar(unsigned int view, int volume, char heavy, int listener);
void CarDamage_BuildRelativeVelocityHull(Car *pCar, Car *pOther);
void StageObject_ApplyWeightedContactDamage(Car *pCar, int amount);
void StageObject_RebuildDamagePartValues(Car *pCar);

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
void CarDamage_ApplyCollisionDeformImpulse(Car *pCar, int *param_2, FixVector *param_3, int param_4,
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
                Race_PlayImpactAndShakeCar(carIdx, len, 0, carIdx);
            } else if ((unsigned int)(CMain::GetFrameDelta() -
                                      (unsigned int)g_deformPulseTicks[carIdx]) > 10) {
                Race_PlayScrapeAndShakeCar(carIdx, len, carIdx);
                g_deformPulseTicks[carIdx] = (int)CMain::GetFrameDelta();
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
                dot = FixVecDot((FixVector *)V, param_3);
                FixVecScale((FixVector *)V, param_3, dot);
                FixMatrix_InverseRotateVector(&g_stageDeformOffset, (FixVector *)V,
                                              *(FixMatrix **)(pc + 0x750));
                CarDamage_AllocateImpactDeformRecord();
            }
            g_stageDeformRadius = FixMul(g_stageDeformStrength, 0x5999);
            g_stageDeformFalloff = FixMul(g_stageDeformStrength, 0x9999);
            g_stageDeformScale = FixMul(g_stageDeformStrength, 0xb333);
            CarDamage_BuildRelativeVelocityHull(pCar, (Car *)(g_unk0x00588b94 + carIdx * 0x4d0));
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
                CarDamage_AllocateImpactDeformRecord();
            }
            dot = FixMul(g_stageDeformStrength, 0x8000);
            if (dot > param_4)
                dot = param_4;
            g_stageDeformRadius = dot;
            g_stageDeformFalloff = dot;
            g_stageDeformScale = dot;
            CarDamage_BuildRelativeVelocityHull(pCar, (Car *)(g_unk0x00588b94 + carIdx * 0x4d0));
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
                CarDamage_AllocateImpactDeformRecord();
            }
            g_stageDeformSpeed = 0x4000;
            dot = FixMul(g_stageDeformStrength, 0x8000);
            if (dot > g_stageDeformSpeed)
                dot = g_stageDeformSpeed;
            g_stageDeformRadius = dot;
            g_stageDeformFalloff = dot;
            g_stageDeformScale = dot;
            CarDamage_BuildRelativeVelocityHull(pCar, (Car *)(g_unk0x00588b94 + carIdx * 0x4d0));
            StageDeform_ApplyPlanarDent();
            break;
        }
        if (param_6 == 0) {
            StageObject_ApplyWeightedContactDamage(pCar, saved);
            StageObject_RebuildDamagePartValues(pCar);
            StageTiming_FlagCarPartBreaks(pCar, 0);
        }
    }
}


BYTE View_GetActiveCameraFlags(BYTE index);
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
// match 70%: logic checked against the original; only stack slots and the
// zero register differ.
// FUNCTION: CMR2 0x0045f9d0
void StageObject_IntegrateViewDeformationGrid(int param_1, int *rec, int param_3)
{
    int wheelScale = 0;
    int gx = 0;
    int gz = 0;
    FixVector pos;
    FixVector fwd;
    FixVector corr;
    FixVector gn;
    FixVector dv;
    FixVector sv;
    FixVector pt;
    FixVector normal;
    FixVector h;
    FixVector *p;
    StageDeformNode *pNode;
    int minY;
    int t;
    int i;
    int x0;
    int z0;
    int z;
    int d;
    int *pHeight;
    short *pTri;
    unsigned short surface;

    StageTiming_UpdateEventDisplacement(rec);
    StageTiming_MarkSpecialStageNodeDirty((SceneNode *)rec);
    FixMatrix_GetPosition(&pos, (FixMatrix *)((BYTE *)g_viewNodes[param_3] + 0x98));
    FixMatrix_GetForward(&fwd, (FixMatrix *)((BYTE *)g_viewNodes[param_3] + 0x98));
    pHeight = Car_Get(View_GetActiveCameraFlags((BYTE)param_3))->cornerHeight;
    minY = 0;
    for (i = 0; i < 4; i++)
        minY += *pHeight++;
    t = FixMul(minY, 0x4000) + 0x6c000;
    minY = pos.y + 0x60000;
    if (t < minY)
        minY = t;
    if (rec[0] == 2) {
        p = (FixVector *)(rec + 0xe);
        p->x = fwd.z;
        rec[0xf] = 0;
        rec[0x10] = -fwd.x;
        Motion_NormalizeInto(p, p);
        sv.x = g_unk0x00547930.z;
        sv.y = 0;
        sv.z = -g_unk0x00547930.x;
        FixVecScale(&sv, &sv, FixMul(g_unk0x00547940, 0x8000));
    }
    FixVecScale(&fwd, &fwd, 0xe0000);
    pos.x += fwd.x;
    pos.y += fwd.y;
    pos.z += fwd.z;
    if (pos.y < minY)
        pos.y = minY;
    dv.x = pos.x - rec[5];
    dv.y = 0;
    dv.z = pos.z - rec[7];
    FixVecScale(&dv, &dv, g_unk0x0051bd40);
    rec[8] = rec[0xb] - dv.x;
    rec[9] = rec[0xc] - dv.y;
    rec[5] = pos.x;
    rec[6] = pos.y;
    rec[10] = rec[0xd] - dv.z;
    rec[7] = pos.z;
    t = FixMul(FixDiv(g_unk0x00538d7c[param_3 * 25], 0xa000) - 0x10000, 0x4ccc);
    FixVecScale(&gn, &fwd, t);
    pos.x += gn.x;
    pos.y += gn.y;
    pos.z += gn.z;
    *(FixVector *)(rec + 2) = pos;
    FixVecScale(&corr, (FixVector *)(rec + 0xb), g_unk0x0051bd3c);
    FixVecScale((FixVector *)(rec + 0x11), &corr, -0x10000);
    if (rec[0x12] != 0)
        rec[0x14] = FixDiv(0x10000, rec[0x12]);
    else
        rec[0x14] = 0;
    if (rec[0] == 2) {
        wheelScale = FixMul(rec[0x16], g_unk0x00543da0);
        if (wheelScale > 0x10000)
            wheelScale = 0x10000;
        wheelScale = FixMul(wheelScale, g_unk0x0051bd3c);
    }
    if (rec[0] == 1) {
        x0 = rec[2] - FixMul(0x4cccc, 0x8000) + 0xc0000;
        z0 = rec[4] - FixMul(0x4cccc, 0x8000) + 0xc0000;
        pTri = (short *)(rec + 0x1e);
        pHeight = rec + 0x2b;
        for (i = 0; i < 5; i++) {
            z = z0;
            for (gz = 0; gz < 5; gz++) {
                pt.x = x0;
                pt.y = 0;
                pt.z = z;
                *pHeight = Track_GetGroundHeight(&pt, &normal, pTri, &surface, *pHeight);
                z -= 0x4cccc;
                pHeight[0x19] = rec[3] - *pHeight - 0x61999;
                pTri++;
                pHeight++;
            }
            x0 -= 0x4cccc;
        }
    }
    corr.x -= rec[2];
    corr.y -= rec[3];
    corr.z -= rec[4];
    for (i = 0; i < *(short *)(rec + 0x1d); i++) {
        pNode = &g_unk0x00543fb0[*(short *)((BYTE *)rec + 0x76) + i];
        h.x = pNode->x + corr.x;
        h.y = pNode->y + corr.y;
        h.z = pNode->z + corr.z;
        if (rec[0] == 2) {
            FixVecScale(&gn, &sv, FixMul(wheelScale, pNode->scale));
            h.x += gn.x;
            h.y += gn.y;
            h.z += gn.z;
        }
        pNode->wrapped = 0;
        if (h.x > 0xc0000)
            h.x -= (unsigned int)(h.x + 0xbffff) / 0x180000 * 0x180000;
        else if (h.x < -0xc0000)
            h.x += (unsigned int)(0xbffff - h.x) / 0x180000 * 0x180000;
        if (rec[0] == 1) {
            gx = 0;
            d = 0xc0000 - h.x;
            if (d >= 0x4cccc)
                gx = (unsigned int)d / 0x4cccc;
            gz = 0;
            d = 0xc0000 - h.z;
            if (d >= 0x4cccc)
                gz = (unsigned int)d / 0x4cccc;
            if (gx < 5 && gz < 5)
                h.y += rec[gx * 5 + gz + 0x44];
        }
        if (h.y > 0x60000) {
            do {
                h.y -= 0xc0000;
            } while (h.y > 0x60000);
        } else if (h.y < -0x60000) {
            do {
                h.y += 0xc0000;
            } while (h.y < -0x60000);
            pNode->wrapped = 1;
        }
        if (rec[0] == 1 && gx < 5 && gz < 5) {
            h.y -= rec[gx * 5 + gz + 0x44];
            pNode->angle = rec[gx * 5 + gz + 0x2b] + 0x1999;
        }
        if (h.z > 0xc0000) {
            do {
                h.z -= 0x180000;
            } while (h.z > 0xc0000);
        } else if (h.z < -0xc0000) {
            do {
                h.z += 0x180000;
            } while (h.z < -0xc0000);
        }
        pNode->x = rec[2] + h.x;
        pNode->y = rec[3] + h.y;
        pNode->z = h.z + rec[4];
        if (rec[0] == 2) {
            pNode->angle += FixMul(pNode->spin, g_unk0x0051bd3c);
            if (pNode->angle > 0x1680000)
                pNode->angle -= 0x1680000;
        }
    }
    if (param_1 != 0 && rec[0x16] != rec[0x17]) {
        if (rec[0x17] > rec[0x16]) {
            rec[0x16] += FixMul(rec[0x18], g_unk0x0051bd3c);
            if (rec[0x16] > rec[0x17])
                rec[0x16] = rec[0x17];
        } else if (rec[0x17] < rec[0x16]) {
            rec[0x16] -= FixMul(rec[0x18], g_unk0x0051bd3c);
            if (rec[0x16] < rec[0x17])
                rec[0x16] = rec[0x17];
        }
    }
}

// Helpers implemented in other translation units.
struct ReplaySample;
void Replay_EncodeCarPoseSample(Car *pCar, ReplaySample *pSample);
extern int g_unk0x00590c44;   // first of the four slot clocks 0x590c44..0x590c53
extern int g_unk0x00590c50;   // last one (0x590c44 + 3 * 4)
extern BYTE g_unk0x00590c60[4];

// Installs the model geometry of part `index` (byte index into the car table)
// into car record `param_1`.
// FUNCTION: CMR2 0x0046c4e0
void StageTiming_InstallCarPartModelGeometry(int param_1, BYTE index)
{
    Car *pCar = Car_Get(index);

    Replay_EncodeCarPoseSample(pCar, (ReplaySample *)param_1);
}

// Wakes the wheels of the current car whose static slot record has no model
// installed yet and whose slot clock is still below the load threshold: the
// slot is empty (0x469bc0 returns 0), the car's slot entry is 0 and the source
// value of the slot exceeds the per-slot threshold.
// FUNCTION: CMR2 0x00480de0
void StageTiming_WakeUnloadedWheelSlots(void)
{
    int slot;
    int offset;

    slot = 3;
    offset = 0;
    for (; offset >= -0xc; slot--) {
        if (StageTiming_GetCarReplayTailEntry(g_partCar, slot) == 0 &&
            *(int *)(g_partCar->index * 0x1a0 +
                     *(int *)((int)&g_unk0x00590d7c[3] + offset)) == 0 &&
            g_partSet->field_0x240[g_unk0x00590c60[slot]] >
                *(int *)((int)&g_unk0x00590c50 + offset)) {
            CarPart_InitWheelHubSlot(slot);
        }
        offset -= 4;
    }
}

// ---------------------------------------------------------------------------
// Car split-bar state update (0x498620).

int StageObject_Atan2Degrees(int y, int x);
int RallyData_GetRouteStateValue(void);
void RallyData_GetRouteNodeGroundPosition(int index, int *pOut);
extern double g_unk0x00511300;

// Base of the per-variant split reference table; the code indexes it with a
// 1-based variant number, so entry `v` lives at 0x592744 + v * 4.

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
void StageTiming_RecomputeCarSplitBarSamples(Car *pCar, unsigned int mask, int *pOut, int variant)
{
// Angle difference wrapped to (-180, 180] degrees (16.16).
#define WRAP_STORE(dst, t)              \
    if ((t) >= 0xb40000) {              \
        (dst) = (t) - 0x1680000;        \
    } else {                            \
        if ((t) < -0xb40000)            \
            (t) += 0x1680000;           \
        (dst) = (t);                    \
    }
#define WRAP(t)                         \
    if ((t) >= 0xb40000)                \
        (t) -= 0x1680000;               \
    else if ((t) < -0xb40000)           \
        (t) += 0x1680000;
#define REF_TRACK ((BYTE *)*(int *)((int *)&g_unk0x00592744 + variant))
    BYTE *pc;
    int nodeZ[20];
    int nodeX[20];
    int result[32];
    int angles[6];
    FixVector node;
    int idxCount;
    int frontAngle;
    int sideAngle;
    int px;
    int pz;
    int nx;
    int nz;
    int baseAngle;
    int curAngle;
    int len;
    int refAngle;
    int tmp;
    int t;
    int a;
    int b;
    int slack;
    int scale;
    int i;

    pc = (BYTE *)pCar;
    idxCount = RallyData_GetRouteStateValue();
    i = pOut[0x15];
    angles[1] = i;
    angles[0] = i - 1;
    angles[4] = i + 2;
    angles[3] = i + 2;
    angles[5] = i + 1;
    angles[2] = i + 5;
    for (i = 0; i < 6; i++) {
        if (angles[i] - idxCount >= 0)
            angles[i] -= idxCount;
        if (angles[i] < 0)
            angles[i] = idxCount - 1;
    }
    frontAngle = StageObject_Atan2Degrees(*(int *)(pc + 0x368), *(int *)(pc + 0x360));
    pOut[4] = frontAngle;
    sideAngle = StageObject_Atan2Degrees(*(int *)(pc + 0x410), *(int *)(pc + 0x408));
    px = *(int *)(pc + 0x2d0);
    pz = *(int *)(pc + 0x2d8);
    RallyData_GetRouteNodeGroundPosition(angles[2], (int *)&node);
    nodeZ[0] = node.z;
    nodeX[0] = node.x;
    if (mask & 1)
        result[0] = StageObject_Atan2Degrees(nodeZ[0] - pz, nodeX[0] - px);
    if (variant != 0)
        result[2] = StageObject_Atan2Degrees(*(int *)(REF_TRACK + 0x1158 + angles[4] * 4) - pz,
                                             *(int *)(REF_TRACK + 0xb90 + angles[4] * 4) - px);
    if (mask & 2) {
        RallyData_GetRouteNodeGroundPosition(angles[3], (int *)&node);
        nodeZ[1] = node.z;
        nodeX[1] = node.x;
    }
    if (mask & 2)
        result[1] = StageObject_Atan2Degrees(nodeZ[1] - pz, nodeX[1] - px);
    baseAngle = FixDiv(*(int *)(pc + 0x778), 0x4000000);
    baseAngle = (int)(__int64)((double)baseAngle * CGraphics::m_65536);
    RallyData_GetRouteNodeGroundPosition(angles[1], (int *)&node);
    nz = node.z;
    nx = node.x;
    RallyData_GetRouteNodeGroundPosition(angles[0], (int *)&node);
    curAngle = StageObject_Atan2Degrees(nz - node.z, nx - node.x);
    tmp = curAngle;
    WRAP(tmp)
    pOut[0x13] = tmp;
    pOut[0x14] = StageObject_Atan2Degrees(nz - pz, nx - px);
    px -= nx;
    len = FixMul(px, px);
    pz -= nz;
    len = FixMul(pz, pz) + len;
    len = FixSqrt(len);
    if (variant != 0)
        refAngle = StageObject_Atan2Degrees(
            *(int *)(REF_TRACK + 0x1158 + angles[1] * 4) - *(int *)(REF_TRACK + 0x1158 + angles[0] * 4),
            *(int *)(REF_TRACK + 0xb90 + angles[1] * 4) - *(int *)(REF_TRACK + 0xb90 + angles[0] * 4));
    tmp = curAngle - StageObject_Atan2Degrees(nodeZ[0] - nz, nodeX[0] - nx);
    WRAP(tmp)
    pOut[0x17] = tmp;
    for (i = 0; i < 0x20; i++) {
        if ((mask & (1 << i)) == 0)
            continue;
        switch (i) {
        case 0:
            tmp = frontAngle - result[0];
            WRAP_STORE(pOut[6], tmp)
            break;
        case 1:
            tmp = frontAngle - result[1];
            WRAP_STORE(pOut[7], tmp)
            break;
        case 3:
            pOut[3] = *(int *)(pc + 0x870);
            break;
        case 4:
            pOut[0x28] = baseAngle - *(int *)(REF_TRACK + 0x5c8 + angles[5] * 4);
            break;
        case 6:
            *pOut = baseAngle;
            break;
        case 8:
            tmp = curAngle - pOut[0x14];
            WRAP(tmp)
            pOut[0xd] = FixMul(len,
                               g_sinTable[(unsigned short)(int)((double)tmp * g_unk0x00511300) & 0xfff]);
            break;
        case 9:
            tmp = frontAngle - curAngle;
            WRAP_STORE(pOut[0xe], tmp)
            break;
        case 0xa:
            tmp = frontAngle - sideAngle;
            WRAP_STORE(pOut[1], tmp)
            break;
        case 0xb:
            pOut[2] = (int)*(short *)(pc + 0xb10);
            break;
        case 0xc:
            pOut[0x18] = baseAngle - *(int *)(REF_TRACK + 0x5c8 + angles[1] * 4);
            break;
        case 0xe:
            b = *(int *)(REF_TRACK + 0x5c8 + angles[1] * 4);
            t = *(int *)(REF_TRACK + angles[1] * 4);
            if (b > 0x3c0000) {
                if (baseAngle < 0x280000) {
                    t = curAngle;
                } else {
                    a = b - baseAngle;
                    if (a > 0) {
                        scale = FixMul(0xb333, b - 0x280000);
                        slack = scale - (b - 0x280000) + a;
                        if (slack > 0) {
                            tmp = t - curAngle;
                            WRAP(tmp)
                            t -= FixMul(FixDiv(slack, scale), tmp);
                        }
                    }
                }
            }
            tmp = frontAngle - t;
            WRAP_STORE(pOut[0x19], tmp)
            break;
        case 0x12:
            tmp = frontAngle - refAngle;
            WRAP_STORE(pOut[0x1e], tmp)
            break;
        case 0x14:
            tmp = frontAngle - result[2];
            WRAP(tmp)
            pOut[0x20] = tmp;
            break;
        }
    }
#undef WRAP_STORE
#undef WRAP
#undef REF_TRACK
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
void Frontend_SetObjectField118(Unk0x004a3e20 *pObject, int value);
void CarEffects_InitUVs(int unused1, int unused2);

#define LOAD_EFFECT_TEXTURE() \
    (int)CTexture::FindLoadTexture((GenericFile *)StageTiming_GetStageFile0(), CFrontend::m_stringDest, &loaded, NULL, 0, 0x10)

// Loads the textures of the particle effects (sparks, debris, exhaust, spray,
// snow, dust, leaves, grass, gravel) from the stage archive.
// FUNCTION: CMR2 0x00459c80
void StageTiming_LoadEffectParticleTextures(void)
{
    bool loaded;
    int i;

    CMain::GetFrameDelta();
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::GetTexturesDirectory(), g_strTexSpark);
    g_unk0x00543650 = LOAD_EFFECT_TEXTURE();
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::GetTexturesDirectory(), g_strTexPaint);
    g_unk0x00543654 = LOAD_EFFECT_TEXTURE();
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::GetTexturesDirectory(), g_strTexGlass);
    g_unk0x00543658 = LOAD_EFFECT_TEXTURE();
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::GetTexturesDirectory(), g_strTexExhaust3);
    g_unk0x0054365c = LOAD_EFFECT_TEXTURE();
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::GetTexturesDirectory(), g_strTexExhaust2);
    g_unk0x00543660 = LOAD_EFFECT_TEXTURE();
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::GetTexturesDirectory(), g_strTexSnowCloud);
    g_unk0x0054366c = LOAD_EFFECT_TEXTURE();
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::GetTexturesDirectory(), g_strTexWheelSpray);
    g_unk0x00543668 = LOAD_EFFECT_TEXTURE();
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::GetTexturesDirectory(), g_strTexSplashSpray2);
    g_unk0x00543670 = LOAD_EFFECT_TEXTURE();
    Frontend_SetObjectField118((Unk0x004a3e20 *)g_unk0x00543670, 1);
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::GetTexturesDirectory(), g_strTexSplashSpray2f);
    g_unk0x00543674 = LOAD_EFFECT_TEXTURE();
    Frontend_SetObjectField118((Unk0x004a3e20 *)g_unk0x00543674, 1);
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::GetTexturesDirectory(), g_strTexSnowSpray);
    g_unk0x00543678 = LOAD_EFFECT_TEXTURE();
    for (i = 0; i < 8; i++) {
        sprintf(CFrontend::m_stringDest, g_strNumberedTga, CInstallInfo::GetTexturesDirectory(), g_strTexDustCloud, i);
        g_unk0x005436b8[i] = LOAD_EFFECT_TEXTURE();
    }
    for (i = 0; i < 4; i++) {
        sprintf(CFrontend::m_stringDest, g_strNumberedTga, CInstallInfo::GetTexturesDirectory(), g_strTexDetail, i + 1);
        g_unk0x005436d8[i] = LOAD_EFFECT_TEXTURE();
        Frontend_SetObjectField118((Unk0x004a3e20 *)g_unk0x005436d8[i], 1);
    }
    for (i = 0; i < 5; i++) {
        sprintf(CFrontend::m_stringDest, g_strNumberedTga, CInstallInfo::GetTexturesDirectory(), g_strTexLeaf, i);
        g_unk0x0054367c[i] = LOAD_EFFECT_TEXTURE();
    }
    for (i = 0; i < 2; i++) {
        sprintf(CFrontend::m_stringDest, g_strNumberedTga, CInstallInfo::GetTexturesDirectory(), g_strTexGrass, i + 1);
        g_unk0x00543690[i] = LOAD_EFFECT_TEXTURE();
    }
    for (i = 0; i < 2; i++) {
        sprintf(CFrontend::m_stringDest, g_strNumberedTga, CInstallInfo::GetTexturesDirectory(), g_strTexGrass, i + 3);
        g_unk0x00543698[i] = LOAD_EFFECT_TEXTURE();
    }
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CInstallInfo::GetTexturesDirectory(), g_strTexSplashDrop);
    g_unk0x005435c8 = LOAD_EFFECT_TEXTURE();
    Frontend_SetObjectField118((Unk0x004a3e20 *)g_unk0x005435c8, 1);
    for (i = 0; i < 5; i++) {
        sprintf(CFrontend::m_stringDest, g_strNumberedTga, CInstallInfo::GetTexturesDirectory(), g_strTexRocksGrav, i);
        g_unk0x005436a0[i] = LOAD_EFFECT_TEXTURE();
    }
    CMain::GetFrameDelta();
    g_unk0x00543664 = g_unk0x00543660;
    CarEffects_InitUVs(g_unk0x00543650, g_unk0x00543654);
}
#undef LOAD_EFFECT_TEXTURE


unsigned int StageTiming_GetTotalCarCount(void);
void Car_AllocateTable(int count);
void StageTiming_AllocateCarReplayRecordTables(int count);
void StageTiming_AllocateCarPartNodeTables(int count);
void StageTiming_AllocateMotionRecordTable(int count);
void StageTiming_AllocateViewWeatherRecords(int count);
void Car_ClearRecords(int first, int count);
void Car_ClearWheelRotation(int first, int count);
void Physics_SetScale(int value);
void StageObject_SetPhysicsScaleAndReciprocal(int value);
void Car_ResetPlaybackSpeed(void);
unsigned int NetPlayers_IsPlayerPresent(int index);
void NetPlayers_SetPlayerField8(int index, int value);
void StageTiming_PlaceEventStartingGrid(char param_1);
void NetPlayers_ClearPlayerFlag23(void);
unsigned char StageTiming_LoadSelectedCarModel(int car);
void RallyData_FindNearestCarRouteNode(Car *pCar);
void Stage_RestoreCarsToRoutePositions(void);
int StageTiming_FreeSceneAndFinishResources(void);
void Car_BuildRaceOrder(int count);
short *Car_GetOrder(void);
short Car_GetOrderCount(void);
void StageTiming_CacheListedCarTimingPointers(short *pList, short count);
void StageTiming_ResetCarPartRecordsAndSlots(void);
void StageTiming_ClearCarReplayRecordRange(int first, int count);
void StageTiming_ResetAllCarReplayRecords(void);
void CarDamage_SetupOrderedCarParts(int, int, short *, short);
void CarTyres_CommitPlayerWear(int a, int b, int count);
void Particle_KillAll(void);
void Car_InvalidateTransformsRange(int first, int count);
void Car_SyncBodySceneNodes(Car *pCar);
void CarEffects_MakeGhost(Car *pCar);
void StageTiming_SetNodeVertexGrey(SceneNode *pNode, BYTE colour);
unsigned int RallyData_GetFlag22(void);
unsigned int RallyData_GetFlag31(void);
void StageTiming_SetNodeTreeVertexGrey(SceneNode *pNode, BYTE colour);

// Loads the stage's cars: allocates and clears the car and effect tables,
// numbers the network players, loads every car, builds the race order and
// the per-car effects, and sets the cars' model colours. Returns 0 when a car
// failed to load.
// FUNCTION: CMR2 0x00456d90
int StageTiming_LoadStageCarsAndEffects(void)
{
    int count;
    int i;
    int number;
    SceneNode *pNode;

    count = StageTiming_GetTotalCarCount();
    memset(g_unk0x00542630 + 0x398, 0, 0x20);
    Car_AllocateTable(count);
    StageTiming_AllocateCarReplayRecordTables(count);
    StageTiming_AllocateCarPartNodeTables(count);
    StageTiming_AllocateMotionRecordTable(count);
    StageTiming_AllocateViewWeatherRecords(count);
    int *pRec = (int *)(g_unk0x00542630 + 4);
    memset(g_unk0x00542630 + 0x2d4, 0, 0x40);
    memset(g_unk0x00542630 + 0x294, 0, 0x40);
    memset(g_unk0x00542630 + 0x254, 0, 0x40);
    memset(g_unk0x00542630 + 0x314, 0, 0x40);
    memset(g_unk0x00542630 + 0x354, 0, 0x40);
    while ((int)pRec < (int)(g_unk0x00542630 + 0x244)) {
        pRec[1] = 0;
        pRec[0] = 0;
        pRec += 9;
    }
    Car_ClearRecords(0, count);
    Car_ClearWheelRotation(0, count);
    Physics_SetScale(0x190000);
    StageObject_SetPhysicsScaleAndReciprocal(0x190000);
    Car_ResetPlaybackSpeed();
    if (CGameInfo::GetGameModeOptionBit19()) {
        number = 1;
        for (i = 0; i < 7; i++) {
            if ((char)NetPlayers_IsPlayerPresent(i)) {
                NetPlayers_SetPlayerField8(i, number);
                number++;
            }
        }
    }
    if ((char)RallyData_GetFlag22() || (char)RallyData_GetFlag31())
        StageTiming_PlaceEventStartingGrid(0);
    NetPlayers_ClearPlayerFlag23();
    for (i = 0; i < count; i++) {
        if (StageTiming_LoadSelectedCarModel(i) == 0)
            return 0;
        RallyData_FindNearestCarRouteNode(Car_Get(i));
    }
    if ((char)RallyData_GetFlag22())
        Stage_RestoreCarsToRoutePositions();
    CGame::RegisterCallback(StageTiming_FreeSceneAndFinishResources, NULL);
    Car_BuildRaceOrder(count);
    StageTiming_CacheListedCarTimingPointers(Car_GetOrder(), Car_GetOrderCount());
    StageTiming_ResetCarPartRecordsAndSlots();
    StageTiming_ClearCarReplayRecordRange(0, count);
    StageTiming_ResetAllCarReplayRecords();
    CarDamage_SetupOrderedCarParts(0, 0, Car_GetOrder(), Car_GetOrderCount());
    CarTyres_CommitPlayerWear(0, 0, (BYTE)RallyDataState());
    Particle_KillAll();
    Car_InvalidateTransformsRange(0, count);
    for (i = 0; i < count; i++)
        Car_SyncBodySceneNodes(Car_Get(i));
    if ((char)RallyData_GetSelectionFlag28() && (char)CGameInfo::GetSoundOptionBit30())
        CarEffects_MakeGhost(Car_Get(count - 1));
    for (i = 0; i < count; i++) {
        StageTiming_SetNodeVertexGrey(*(SceneNode **)((BYTE *)Car_Get(i) + 0x71c), 0);
        StageTiming_SetNodeVertexGrey(*(SceneNode **)((BYTE *)Car_Get(i) + 0x720), 0);
        StageTiming_SetNodeTreeVertexGrey(*(SceneNode **)(*(BYTE **)((BYTE *)Car_Get(i) + 0x71c) + 4), 0);
        StageTiming_SetNodeTreeVertexGrey(*(SceneNode **)(*(BYTE **)((BYTE *)Car_Get(i) + 0x720) + 4), 0);
        pNode = SceneNode_FindByType(*(SceneNode **)((BYTE *)Car_Get(i) + 0x720), 0xe);
        if (pNode != NULL)
            StageTiming_SetNodeVertexGrey(pNode, 0xff);
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
void Graphics_RaiseSupportedDetailLevels(void);
int RallyData_IsChampionshipFinalStage(void);
int StageUI_GetRaceResultValue(void);
unsigned int *RallyData_GetChampionshipState(void);
int Replay_GetSelectionStateByte(void);
BYTE RallyData_GetDriverSelectGridSlot(int param1);
int StageTiming_GetSplitSecondaryEntry(int iSplit);
int RallyTiming_GetStagePositionOfDriver(int iDriver);
int StageTiming_GetDriverSlot(int iDriver);
void NetPlayers_SetPlayerFlag23(int index);
unsigned int NetPlayers_GetCarSelection(int index);
int RallyData_GetDistinctValueEntry(int index);
void RallyData_SetDriverCategoryOption(BYTE index, BYTE value);
char *Car_GetDirectoryPath(int car);
void Graphics_SetTextureStageState(int param1);
int Sector_BuildC3DModelScene(unsigned int, unsigned int, unsigned int);
int *StageTiming_GetCarStartPosition(int car);
int StageTiming_GetStartTableField244(int unused);
int Car_UsesNarrowWheels(Car *pCar, int param2);
BOOL Car_ShowsCleanWheels(Car *pCar);
void Car_SwapWheelTextures(char mode, SceneNode **pWheels);
void Scene_AddShadowCaster(SceneNode *pNode, int exactMeshes);
void SceneNode_SetMeshFlagBits(SceneNode *pNode, unsigned int value);
void StageObject_SetCarStateSlot(int value, int index);
void StageTiming_ResetWheelWearRecord(int player, int wheel);
void StageTiming_InitStageTypeEvents(int unused, int slot, int type);

#define CAR_RECORD(i) (g_unk0x00542630 + (i) * 0x24)
#define MESH_OF(node) (*(int *)((BYTE *)(node) + 0xc))

// Loads car `car` of the stage: picks its model (by game mode: the player's
// car, the championship/knockout opponents, the network players...), the
// model's detail letter, then loads the model, its textures and cinematic
// data, the snow/light wheel set of full-detail cars and, for CPU cars in the
// rally modes, the damaged body variant. Returns 0 when a file is missing.
// FUNCTION: CMR2 0x00457000
BYTE StageTiming_LoadSelectedCarModel(int car)
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
    short lod;
    int letterInt;
    char letter;
    BYTE variant;
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

    slot = 0;
    pSlot = NULL;
    mode = CGameInfo::GetConfiguredGameMode();
    isPlayer = car < (int)(BYTE)RallyDataState();
    if ((BYTE)RallyDataState() > 1 && CGameInfo::IsConfiguredMultiplayer() == 0)
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
    Graphics_RaiseSupportedDetailLevels();
    switch (StageUI_GetRaceResultValue()) {
    case 4:
        slot = StageUI_GetRaceEndEventCount() + car;
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
        if ((mode == 3 || mode == 7) && (char)CGameInfo::GetSoundOptionBit30() && car == (int)(StageTiming_GetTotalCarCount() - 1)) {
            i = Replay_GetSelectionStateByte();
            if (i == -1)
                i = 0;
            model = RallyData_GetDriverRecordSelectionValue((BYTE)i);
        } else if ((char)RallyData_GetSelectionFlag27() && CGameInfo::GetGameModeOptionBit19() == 0) {
            if (car < (int)(BYTE)RallyDataState())
                model = RallyData_GetDriverRecordSelectionValue((BYTE)slot);
            else
                model = RallyData_GetDriverSelectGridSlot(StageTiming_GetSplitSecondaryEntry(StageUI_GetRaceEndEventCount()));
        } else if ((char)RallyData_IsChampionshipFinalStage()) {
            best = 99;
            n = 0;
            for (i = 0; i < (int)CGameInfo::GetConfiguredPlayerCount(); i++) {
                if (RallyTiming_GetStagePositionOfDriver(StageTiming_GetDriverSlot(i)) < best) {
                    best = RallyTiming_GetStagePositionOfDriver(StageTiming_GetDriverSlot(i));
                    n = i;
                }
            }
            model = RallyData_GetDriverRecordSelectionValue((BYTE)n);
        } else {
            n = 0;
            if (CGameInfo::GetGameModeOptionBit19() && car > 0) {
                slot = mode;
                for (i = 0; i < 7; i++) {
                    if ((char)NetPlayers_IsPlayerPresent(i)) {
                        if (n == car - 1) {
                            slot = i;
                            break;
                        }
                        n++;
                    }
                }
                NetPlayers_SetPlayerFlag23(slot);
                model = NetPlayers_GetCarSelection(slot);
                team = (int)CFrontend::GetArchivePrimaryIDEntry(model);
                NetPlayers_SetPlayerField8(slot, car);
                goto classes;
            }
            model = RallyData_GetDriverRecordSelectionValue((BYTE)slot);
        }
        model &= 0xff;
        team = (int)CFrontend::GetArchivePrimaryIDEntry(model);
    } else {
        if (car == 0) {
            team = rand() % 6;
            while (team == g_unk0x0051a8b8)
                team = rand() % 6;
            g_unk0x0051a8b8 = team;
        } else {
            team = g_unk0x0051a8b8;
        }
        model = (unsigned int)CFrontend::GetArchiveSecondaryIDEntry(team);
        RallyData_SetDriverCategoryOption((BYTE)car, (BYTE)model);
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
                model = RallyData_GetDistinctValueEntry(car);
                team = (int)CFrontend::GetArchivePrimaryIDEntry(model);
            }
            hasVariant = 1;
        }
        lod = letter != 'A' ? 3 : 1;
        break;
    case 3:
    case 7:
        if ((char)CGameInfo::GetSoundOptionBit30()) {
            if (car == (int)(StageTiming_GetTotalCarCount() - 1)) {
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
        if ((char)RallyData_GetFlag25() && CGameInfo::GetGameModeOptionBit19() == 0) {
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
        } else if (CGameInfo::GetGameModeOptionBit19()) {
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
    if (CGameInfo::IsRecordFlagSet(0x10))
        lod = 1;
    pRecord = CAR_RECORD(car);
    pRecord[0x20] = letter;
    teamByte = (BYTE)team;
    g_unk0x0054260c[car] = model;
    letterInt = letter;
    sprintf(CFrontend::m_stringDest, g_strCarModelFormat, Car_GetDirectoryPath(model), letterInt, lod);
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CFrontend::m_stringDest, g_strCarC3dExt);
    if (pRecord[0x20] == 'A')
        strncpy(CFrontend::m_stringDest + strlen(CFrontend::m_stringDest) - 6, g_strDamageModelC3d, 8);
    pData = CFileBuffer::GetGenericFileBuffer(CFrontend::m_stringDest, FALSE);
    strcpy(path, CFrontend::m_stringDest);
    *(void **)(g_unk0x00542630 + 0x354 + car * 4) = pData;
    sprintf(CFrontend::m_stringDest, g_strCinFormat, Car_GetDirectoryPath(model));
    *(void **)(g_unk0x00542630 + 0x398 + car * 4) = CFileBuffer::GetGenericFileBuffer(CFrontend::m_stringDest, FALSE);
    if ((char)RallyData_GetFlag24())
        Graphics_SetTextureStageState(1);
    else
        Graphics_SetTextureStageState(0);
    sprintf(CFrontend::m_stringDest, g_strCarBflFormat, Car_GetDirectoryPath(model), letterInt, lod);
    pArchive = &g_unk0x00542ae8[car * 2];
    CGenericFileLoader::LoadIntoFileRecord((GenericFile *)pArchive, CFrontend::m_stringDest);
    if (pData == NULL)
        return 0;
    pScene = (SceneNode *)Sector_BuildC3DModelScene((unsigned int)pData, RallyData_GetChallengeRenderState(), (unsigned int)pArchive);
    pBody = SceneNode_FindByType(pScene, 5);
    *(SceneNode **)(pRecord + 8) = pScene;
    *(SceneNode **)(pRecord + 4) = pBody;
    pRecord[0] = (BYTE)team;
    pRecord[0x20] = letter;
    if (((char)RallyData_GetFlag25() && CGameInfo::GetGameModeOptionBit19() == 0 && twoPlayers == 0 && car == 1) ||
        (char)RallyData_IsChampionshipFinalStage() || CGameInfo::GetConfiguredGameMode() == 4)
        teamByte = 0;
    StageTiming_PlaceCarViewNodesAndBody(pScene, pBody, car, teamByte, (FixAngles *)StageTiming_GetStartTableField244(car), (FixVector *)StageTiming_GetCarStartPosition(car), 0);
    pWheelData = NULL;
    if (pRecord[0x20] == 'A') {
        if (Car_UsesNarrowWheels(Car_Get(car), 1)) {
            strncpy(path + strlen(path) - 5, g_strSnowWheelsC3d, 5);
            pWheelData = CFileBuffer::GetGenericFileBuffer(path, FALSE);
        }
        if (Car_ShowsCleanWheels(Car_Get(car))) {
            strncpy(path + strlen(path) - 5, g_strLightWheelsC3d, 5);
            pWheelData = CFileBuffer::GetGenericFileBuffer(path, FALSE);
        }
        if (pWheelData != NULL) {
            *(int *)(pRecord + 0xc) = Sector_BuildC3DModelScene((unsigned int)pWheelData, RallyData_GetChallengeRenderState(),
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
    if (CGameInfo::GetGraphicsOptionBit4()) {
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
    if (rallyMode || ((char)RallyData_GetSelectionFlag28() && (char)CGameInfo::GetSoundOptionBit30())) {
        if ((char)RallyData_GetSelectionFlag28())
            CGameInfo::GetSoundOptionBit30();
    } else {
        StageObject_SetCarStateSlot(1, car);
    }
    if (car < (int)(BYTE)RallyDataState() &&
        ((CGameInfo::GetConfiguredGameMode() != 0 && CGameInfo::GetConfiguredGameMode() != 1) || RallyDataStageIndex() == 0 ||
         RallyDataStageIndex() == 4 || RallyDataStageIndex() == 8 || RallyDataStageIndex() == 10)) {
        StageTiming_ResetWheelWearRecord(car, 0);
        StageTiming_ResetWheelWearRecord(car, 1);
        StageTiming_ResetWheelWearRecord(car, 2);
        StageTiming_ResetWheelWearRecord(car, 3);
    }
    if ((char)RallyData_GetFlag24() == 0 && (letter == 'C' || letter == 'A')) {
        sprintf(CFrontend::m_stringDest, g_strCarModelFormat, Car_GetDirectoryPath(model), letterInt, lod);
        StageTiming_InitStageTypeEvents((int)CFrontend::m_stringDest, car, teamByte);
    }
    if (hasVariant == 0 || CGameInfo::IsRecordFlagSet(0x10)) {
        pCar = Car_Get(car);
        *(int *)&pCar->pNode0x724 = 0;
        *(int *)&pCar->pExtraNodes[0] = 0;
        *(int *)&pCar->pExtraNodes[1] = 0;
        *(int *)&pCar->pExtraNodes[2] = 0;
        *(int *)&pCar->pExtraNodes[3] = 0;
        return 1;
    }
    pRecord[0x21] = variant;
    sprintf(CFrontend::m_stringDest, g_strCarModelFormat, Car_GetDirectoryPath(model), (int)variant, lod);
    sprintf(CFrontend::m_stringDest, g_strPathConcat, CFrontend::m_stringDest, g_strCarC3dExt);
    pData = CFileBuffer::GetGenericFileBuffer(CFrontend::m_stringDest, FALSE);
    if (pData == NULL)
        return 0;
    *(void **)(g_unk0x00542630 + 0x314 + car * 4) = pData;
    CFrontend::m_stringDest[strlen(CFrontend::m_stringDest) - strlen(g_strCarC3dExt)] = 0;
    sprintf(CFrontend::m_stringDest, g_strBflFormat, CFrontend::m_stringDest);
    pArchive = &g_unk0x00542ae8[car * 2 + 1];
    CGenericFileLoader::LoadIntoFileRecord((GenericFile *)pArchive, CFrontend::m_stringDest);
    pCar = Car_Get(car);
    pNodes = (SceneNode *)Sector_BuildC3DModelScene((unsigned int)pData, RallyData_GetChallengeRenderState(), (unsigned int)pArchive);
    *(SceneNode **)(g_unk0x00542630 + 0x294 + car * 4) = pNodes;
    pCar->pNode0x724 = SceneNode_FindByType(pNodes, 5);
    pCar->pExtraNodes[0] = SceneNode_FindByType(pNodes, 1);
    pCar->pExtraNodes[1] = SceneNode_FindByType(pNodes, 2);
    pCar->pExtraNodes[2] = SceneNode_FindByType(pNodes, 3);
    pCar->pExtraNodes[3] = SceneNode_FindByType(pNodes, 4);
    if (CGameInfo::GetGraphicsOptionBit4())
        Scene_AddShadowCaster(pCar->pNode0x724, 0);
    SceneNode_SetMeshFlagBits(pCar->pNode0x724, car);
    if (pCar->pNode0x724 != NULL)
        SceneNode_Reparent(pCar->pNode0x724, pCar->pNode0x71c);
    if (pCar->pExtraNodes[0] != NULL)
        SceneNode_Reparent(pCar->pExtraNodes[0], pCar->pNode0x71c);
    if (pCar->pExtraNodes[1] != NULL)
        SceneNode_Reparent(pCar->pExtraNodes[1], pCar->pNode0x71c);
    if (pCar->pExtraNodes[2] != NULL)
        SceneNode_Reparent(pCar->pExtraNodes[2], pCar->pNode0x71c);
    if (pCar->pExtraNodes[3] != NULL)
        SceneNode_Reparent(pCar->pExtraNodes[3], pCar->pNode0x71c);
    Car_SwapWheelTextures(pRecord[0x21], &pCar->pExtraNodes[0]);
    return 1;
}
#undef CAR_RECORD
#undef MESH_OF

void StageObject_ResetPartTuningAndRandomSeed(void);
void StageObject_ClearPartTuningState(void);
void StageObject_ResetAttachedCarNodes(BYTE *pCar);
void StageTiming_RegisterSlotMeshNode(SceneNode *pNode, int *pSlot);
void StageObject_AssignMatchingCarClassSlots(BYTE *pCar, BYTE *pObject, BYTE flag);
void CarDamage_RebuildPartBounds(int param_1, int param_2, int param_3);
void StageObject_ResetCarObjectState(Car *pCar);

// Sets up the damage parts of the cars in `pOrder` (last first): collects the
// mesh parts of each car model into its part table (0x4d0 bytes, 15 slots),
// closes the gaps, resets the per-part damage state and, for the players'
// cars, restores the saved damage record (`keep` = 0 also refreshes the saved
// copy).
// FUNCTION: CMR2 0x004669f0
void CarDamage_SetupOrderedCarParts(int lock, int keep, short *pOrder, short count)
{
    int n;
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

    StageObject_ResetPartTuningAndRandomSeed();
    if (lock == 0)
        StageObject_ClearPartTuningState();
    for (n = count - 1; n >= 0; n--) {
        {
            car = pOrder[n];
            pCar = (BYTE *)Car_Get(car);
            StageObject_ResetAttachedCarNodes(pCar);
            pParts = (int *)(g_unk0x00588b94 + car * 0x4d0);
            pParts[0x117] = 0;
            pDamage = g_unk0x00588b98 + car * 0x290;
            if (*(SceneNode **)(pCar + 0x720) != NULL) {
                pParts[0x107] = 0;
                pParts[0x106] = 0;
                pParts[0x105] = 0;
                pParts[0x104] = 0;
                if ((BYTE)(*(SceneNode **)(pCar + 0x720))->flags != 0x14)
                    StageTiming_RegisterSlotMeshNode(*(SceneNode **)(pCar + 0x720), pParts);
                for (pChild = (*(SceneNode **)(pCar + 0x720))->pFirstChild; pChild != NULL; pChild = pChild->pNext) {
                    for (pNode = pChild; pNode != NULL && (BYTE)pNode->flags != 0x14;
                         pNode = pNode->pFirstChild)
                        StageTiming_RegisterSlotMeshNode(pNode, pParts);
                }
                for (j = 0; j < 15; j++) {
                    if (pParts[0xf + j] != 0)
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
                    StageObject_AssignMatchingCarClassSlots(pCar, (BYTE *)pParts[0xf + i], (BYTE)i);
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
                    pSaved = (BYTE *)RallyData_GetDriverGroupRecord(StageUI_GetRaceEndEventCount() + (signed char)pCar[0xb1a]);
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
                CarDamage_RebuildPartBounds((signed char)pCar[0xb1a], *(int *)(pCar + 0x720), (int)pParts);
                StageObject_ResetCarObjectState((Car *)pCar);
            }
        }
    }
    if (lock != 0)
        StageObject_ClearPartTuningState();
}

extern int g_stageLighting[];
// Lightning: the sector lit by the current flash (0x547ac0) and the previous
// frame's value (0x547ac2), inside the lighting block of StageObjects.cpp.
#define g_unk0x00547ac0 (*(short *)&g_stageLighting[0x5c])
#define g_unk0x00547ac2 (*((short *)&g_stageLighting[0x5c] + 1))
#define RAND_FIX() ((int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536))

int StageObject_FindClosestMeshVertex(void);
void StageObject_RandomizeStageTriangleTextures(void);
void StageObject_ScaleViewValueByRoute(BYTE *pObject, int view);
void StageObject_UpdateThirdRouteRamp(unsigned int *pRecord, int view);
void StageTiming_IntegrateTerrainBodyPitch(int pData, int param_2);
void StageObject_IntegrateViewDeformationGrid(int param_1, int *param_2, int param_3);
void StageObject_AdvanceAnimatedRecordState(int *p, int unused);

// Wind: eases the strength towards its target, then after a random pause
// picks a new target and rate (halved when a view has snow).
// FUNCTION: CMR2 0x0045f6d0
void StageTiming_UpdateWindStrength(void)
{
    int snow;
    int i;

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
    for (i = 0; i < (int)g_unk0x00543e98; i++) {
        if (*(int *)((BYTE *)g_unk0x00547ac8 + i * 0x178) == 2) {
            snow = 1;
            i = g_unk0x00543e98;
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
void StageTiming_UpdateLightningFlash(void)
{
    int *pWeather;

    pWeather = (int *)g_unk0x00547ac8;
    if (g_unk0x00543ec0 != 0) {
        g_unk0x00543ec0 -= g_unk0x0051bd3c;
        if (g_unk0x00543ec0 < 0) {
            g_unk0x00543ec8 = (short)StageObject_FindClosestMeshVertex();
            StageObject_RandomizeStageTriangleTextures();
            g_unk0x00543ec0 = 0;
        }
    } else {
        g_unk0x00543ec4 -= g_unk0x0051bd3c;
        if (g_unk0x00543ec4 < 0) {
            g_unk0x00543ec0 = FixMul(RAND_FIX(), 0x1770000) + 0x7d0000;
            g_unk0x00543ec4 = FixMul(RAND_FIX(), 0x140000) + 0x140000;
            g_unk0x00543ec8 = -1;
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
void StageTiming_UpdateAllViewWeather(void)
{
    int view;
    int *pWeather;
    int offset;
    int *pRecord;

    StageTiming_UpdateWindStrength();
    StageTiming_UpdateLightningFlash();
    for (view = 0; view < (int)g_unk0x00543e98; view++) {
        offset = view * 0x2c;
        pWeather = (int *)((BYTE *)g_unk0x00547ac8 + view * 0x178);
        pRecord = (int *)((BYTE *)g_unk0x00543eb8 + offset);
        StageObject_ScaleViewValueByRoute((BYTE *)pWeather, view);
        StageObject_UpdateThirdRouteRamp((unsigned int *)pRecord, view);
        StageTiming_IntegrateTerrainBodyPitch((int)pRecord, view);
        if (pWeather[0] == 1 || pWeather[0] == 2)
            StageObject_IntegrateViewDeformationGrid(1, pWeather, view);
        StageObject_AdvanceAnimatedRecordState(pWeather, view);
    }
}
#undef RAND_FIX

extern char g_strDemoName0x00519ec4[4];
void NetPlayers_UpdateBestLapTime(unsigned int time);
void NetPlayers_RecordLocalStageTime(unsigned int time, int stage);
void Frontend_SetDeviceExtraValue(int index, int value);
BYTE Frontend_MergeBestArcadeRecordOption(int index);
int FrontendRecords_CopyArcadeSplitMirror(int param_1, int param_2);
int Frontend_AccumulateMinimumArcadeOption(int param_1, int param_2);
int StageTiming_GetCarTimingByte81(int car);
unsigned int RallyData_GetSetupHighNibble(void);

// A car crosses a split line: stores its split time (from the start or the
// previous split), reports it to the network and the record tables and runs
// the head-to-head rules.
// FUNCTION: CMR2 0x00448920
void StageTiming_HandleCarSplitCrossing(int car)
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

    group = StageTiming_GetCheckpointGroupIndex(car);
    if (StageTiming_GetCheckpointField19(car) == 0)
        return;
    if ((char)RallyData_GetSelectionFlag27() == 0) {
        pRecord = g_unk0x0053d1e8[car][group];
        pRecord[0] = g_unk0x0053d1b0;
        if (!((char)RallyData_GetSelectionFlag28() == 0 && CGameInfo::GetConfiguredGameMode() != 12)) {
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
        } else {
            pPrev = pRecord - 5;
            base = pPrev[0];
            pTime = &g_carStageTiming[car].splitTimes[group];
            *pTime = pRecord[0] - base;
            pOut = g_carStageTiming[car].splits;
            for (i = 5; i != 0; i--)
                *pOut++ = *pPrev++ - base;
        }
        if (CGameInfo::GetConfiguredGameMode() == 12) {
            NetPlayers_UpdateBestLapTime(*pTime);
            NetPlayers_RecordLocalStageTime(*pTime, 0);
        }
    }
    if (car < (int)(BYTE)RallyDataState() && (char)CGameInfo::GetActiveCheatMask() == 0 &&
        (char)CGameInfo::GetNetworkOptionBit11() == 0 && g_unk0x0053d1d8 == 0 &&
        strncmp((char *)RallyData_GetRecord(0), g_strDemoName0x00519ec4, 3) != 0) {
        slot = StageUI_GetRaceEndEventCount() + car;
        if ((char)RallyData_GetFlag24()) {
            Frontend_SetDeviceExtraValue(slot, g_carStageTiming[car].splitTimes[group]);
            g_unk0x0053e18d[car] |= Frontend_MergeBestArcadeRecordOption(slot);
            bits = FrontendRecords_CopyArcadeSplitMirror(car, slot);
            g_unk0x0053e18f |= bits & 1;
            g_carStageTiming[car].field_0x84 = bits & 1;
            g_carStageTiming[car].pad_0x85[0] = (BYTE)Frontend_AccumulateMinimumArcadeOption(car, slot) & 1;
        }
    }
    if ((char)RallyData_GetSetupFlag11()) {
        if (RallyData_GetSetupModeBits() == 2) {
            if (StageTiming_GetCarTimingByte81(car) == 0)
                g_unk0x0053d1a4[car]++;
            if (g_unk0x0053d1a4[car] == (int)RallyData_GetSetupHighNibble()) {
                g_unk0x0053d1a7 = 1;
                StageTiming_QueueDriverSlot(car);
                StageTiming_QueueDriverSlot(1 - car);
            }
        }
        if (RallyData_GetSetupModeBits() == 1) {
            if (g_unk0x0053d1a6 == 0) {
                if (StageTiming_GetCarTimingByte81(car) == 0) {
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
void StageTiming_UpdateCarLineCrossings(int car)
{
    if (StageTiming_GetCheckpointField16(car)) {
        if (StageTiming_GetCheckpointField17(car))
            StageTiming_RecordAndReorderSplitTime(car);
        if (StageTiming_GetCheckpointField18(car))
            StageTiming_HandleCarSplitCrossing(car);
        if (StageTiming_GetCheckpointField1A(car))
            StageTiming_QueueDriverSlot(car);
    }
}

void StageTiming_AdvanceJitteredClock(void);
void StageTiming_SetCarStartClock(int car);
void StageTiming_BubbleRunningOrder(void);
void StageTiming_QueueSplitRaceDriversByGap(int slot);

// Per-frame stage timing of every car: in the championship/arcade modes the
// start of each car's timing, then each car's line crossings.
// FUNCTION: CMR2 0x00448120
void StageTiming_UpdateAllCarLineCrossings(void)
{
    int count;
    int i;
    BYTE *p;

    count = Stage_GetDriverCount();
    StageTiming_AdvanceJitteredClock();
    if ((char)RallyData_GetFlag24() || (char)RallyData_GetFlag25()) {
        if (count > 0) {
            p = &g_carStageTiming[0].field_0x83;
            i = count;
            do {
                *p = 0;
                p += sizeof(CarStageTiming);
            } while (--i != 0);
        }
        for (i = 0; i < count; i++) {
            if (g_unk0x0053d1da[i] == 0 && StageTiming_GetCheckpointField16(i))
                StageTiming_SetCarStartClock(i);
        }
        StageTiming_BubbleRunningOrder();
    }
    for (i = 0; i < count; i++) {
        if (g_unk0x0053d1da[i] == 0) {
            StageTiming_UpdateCarLineCrossings(i);
            if ((char)RallyData_GetSetupFlag11())
                StageTiming_QueueSplitRaceDriversByGap(i);
        }
    }
}

int GetStageSplitCount(void);

// Adds a random error to the CPU drivers' split deltas (16 drivers, a row per
// split), scaled by the difficulty; outside the head-to-head modes the error
// is kept inside a band that narrows along the stage.
// FUNCTION: CMR2 0x00456710
void StageTiming_PerturbComputerSplitDeltas(int *pDeltas)
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
    switch (CGameInfo::GetConfiguredDifficulty()) {
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
        if (splits >= 3) {
            step = (int)(__int64)((double)(splits - 1) * CGraphics::m_65536);
            for (k = 2; k < splits; k++)
                limits[k] = limits[1] - FixMul(FixDiv(limits[1] - scale, step), (int)(__int64)((double)(k - 1) * CGraphics::m_65536));
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

void StageTiming_BuildSplitInterpolationFactors(int *param_1);
void StageTiming_InterpolateCarFixedSampleRows(int pCar, int *pOut);
void StageTiming_AccumulateSplitDeltas(int *pDeltas);
void RallyTiming_SortOrder(int *piTimes, char *pcOrder, int iDirection, int iCount, char bInitialise);

// Builds the CPU drivers' split times for a stage and sorts every split.
// FUNCTION: CMR2 0x004561e0
void StageTiming_BuildAndSortCPUSplitTimes(int *pTimes)
{
    int *pDeltas;
    int n;
    int i;
    int *pSplit;
    char *pOrder;

    pDeltas = (int *)CFileBuffer::AllocateLockedBuffer(0x280);
    StageTiming_BuildSplitInterpolationFactors(pTimes);
    StageTiming_InterpolateCarFixedSampleRows((int)pTimes, pDeltas);
    StageTiming_PerturbComputerSplitDeltas(pDeltas);
    StageTiming_AccumulateSplitDeltas(pDeltas);
    n = GetStageSplitCount();
    if (n >= 0) {
        pSplit = g_stageSplitTimesRaw[0];
        pOrder = g_stageSplitTimesRawDriverIx[0];
        i = n + 1;
        do {
            RallyTiming_SortOrder(pSplit, pOrder, 1, g_unk0x00541f98, 1);
            pOrder += 0x10;
            pSplit += 0x10;
            i--;
        } while (i != 0);
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
void Glow_SetEntryValue3C(BYTE *p, int value);
void Glow_SetEntryByte50(BYTE *p, BYTE value);

#define CAR_INT(p, off) (*(int *)((BYTE *)(p) + (off)))

// Exhaust effects of a car: smoke from each exhaust while the engine revs at
// low speed, sparks under hard acceleration, and the backfire flash.
// FUNCTION: CMR2 0x0045af90
void CarEffect_UpdateExhaustSmokeAndBackfire(int car)
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
    revs = FixMul(0x28f, pCar->field_0x7a4);
    if (revs < 0)
        revs = -revs;
    speed = (pCar->velocity.x < 0 ? -pCar->velocity.x : pCar->velocity.x) +
            (pCar->velocity.z < 0 ? -pCar->velocity.z : pCar->velocity.z);
    heat = 0x17c - (FixMul(0x15e0000, *(int *)((BYTE *)StageTiming_GetCarReplayRecord(pCar->index) + 0x404)) >> 16);
    if (heat > 0xff)
        heat = 0xff;
    else if (heat < 0)
        heat = 0;
    for (exhaust = 0; exhaust < 2; exhaust++) {
        if (g_trailTextureA[car][exhaust] == 0)
            continue;
        if (revs > 0xccc && speed < 0x28f) {
            FixMatrix_RotateVector(&g_unk0x00543380[car], (FixVector *)g_trailTextureA[car][exhaust],
                                   pCar->pWorld);
            g_unk0x00543380[car].x += pCar->pBodyMatrix->position.x;
            g_unk0x00543380[car].y += pCar->pBodyMatrix->position.y;
            g_unk0x00543380[car].z += pCar->pBodyMatrix->position.z;
            carVelocity = pCar->cornerVelocity[3];
            big = heat > 0x50;
            negRevs = -revs;
            FixVecScale(&velocity, &pCar->right, negRevs);
            velocity.y += carVelocity.y;
            velocity.x += carVelocity.x;
            velocity.z += carVelocity.z;
            t = FixMul((int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536), 0x3d7);
            Particle_Spawn(big, &g_unk0x00543380[car], &velocity, g_unk0x00543380[car].y - 0x10000, 0, NULL, 0, 0,
                           *(BYTE *)(*(BYTE * *)&pCar->pNode0x720 + 0x17c));
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
            FixMatrix_RotateVector(&position, &spark, pCar->pWorld);
            revs = 0x1999;
            position.x += pCar->pBodyMatrix->position.x;
            position.y += pCar->pBodyMatrix->position.y - 0x3333;
            position.z += pCar->pBodyMatrix->position.z;
            FixVecScale(&velocity, &pCar->up, revs);
            t = FixMul((int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536), 0x3d7);
            if (speed > 0x8000)
                Particle_Spawn(2, &position, &velocity, position.y - 0x10000, 0, NULL, 0, 0,
                               *(BYTE *)(*(BYTE * *)&pCar->pNode0x720 + 0x17c));
            else
                Particle_Spawn(2, &position, &velocity, position.y - 0x10000, 0, NULL, 0, 0,
                               *(BYTE *)(*(BYTE * *)&pCar->pNode0x720 + 0x17c));
        }
    }
    if (g_unk0x005439b0[car] == 0)
        return;
    g_unk0x005439b0[car]--;
    for (exhaust = 0; exhaust < 2; exhaust++) {
        if (g_trailTextureA[car][exhaust] == 0)
            continue;
        FixMatrix_RotateVector(&g_unk0x00543380[car], (FixVector *)g_trailTextureA[car][exhaust],
                               pCar->pWorld);
        FixVecScale(&velocity, &pCar->right, -revs);
        zero.x = 0;
        zero.y = 0;
        zero.z = 0;
        g_trailForced[car] = 0;
        Particle_Spawn(3, &zero, &velocity, -0x640000, 0, NULL, 0, (int)&car,
                       *(BYTE *)(*(BYTE * *)&pCar->pNode0x720 + 0x17c));
        r = (int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536);
        if (g_trailTextureB[car][exhaust] != 0) {
            Glow_SetEntryValue3C((BYTE *)g_trailTextureB[car][exhaust], r);
            Glow_SetEntryByte50((BYTE *)g_trailTextureB[car][exhaust], 1);
        }
    }
}
#undef CAR_INT

int *StageObject_GetViewWeatherSlot(int i, int j);
void CarEffects_SetSplashColour(int player, BYTE *pColour);
void CarEffect_UpdateWheelSurfaceParticles(int car);
void StageTiming_SpawnWheelParticles(int carIndex);
void WheelSplash_Update(int player);
void WheelSpray_Update(int player);

// Per-frame effects of a car (not while replaying a stored run): the wheel
// trails in the colour of the stage light, dust, particles, splashes, spray
// and the exhaust.
// FUNCTION: CMR2 0x0045af00
void StageTiming_UpdateCarWheelAndExhaustEffects(int car)
{
    BYTE *pColour;
    int index = car;

    if (index >= 8)
        return;
    if (*(int *)((BYTE *)Car_Get(index) + 0xc0c) != 0 || CGameInfo::GetGraphicsOptionBits25To26() == 2)
        return;
    pColour = (BYTE *)StageObject_GetViewWeatherSlot(index, 0);
    if (pColour != NULL) {
        ((BYTE *)&car)[0] = pColour[0];
        ((BYTE *)&car)[1] = pColour[1];
        ((BYTE *)&car)[2] = pColour[2];
        ((BYTE *)&car)[3] = 0xff;
    }
    CarEffects_SetSplashColour(index, (BYTE *)&car);
    if (View_GetActiveCameraMode(index) != 3) {
        CarEffect_UpdateWheelSurfaceParticles(index);
        StageTiming_SpawnWheelParticles(index);
        WheelSplash_Update(index);
        WheelSpray_Update(index);
    }
    CarEffect_UpdateExhaustSmokeAndBackfire(index);
}

void CarDamage_StepClimbingCarRecords(int param_1, short count);
void Car_BreakQueuedWindows(Car *pCar);
void CarPart_SmoothSteeringOffset(void);
void StageTiming_WakeUnloadedWheelSlots(void);

// Per-frame update of the attached body parts (bonnet, doors, bumpers...) of
// the cars in `pOrder` (last first): each active part's world transform is
// rebuilt and its per-part handler runs; then queued windows break.
// FUNCTION: CMR2 0x00480bb0
void StageTiming_UpdateAttachedCarParts(BYTE *pCars, short *pOrder, short count)
{
    int n;
    short *pIndex;
    void **pTable;
    BYTE *pPart;

    n = count - 1;
    if (n >= 0) {
        pIndex = pOrder + n;
        int remaining = n + 1;
        do {
            g_partCar = (Car *)(pCars + *pIndex * 0xc24);
            g_partSet = (CarPartSet *)StageTiming_GetCarReplayRecord(g_partCar->index);
            StageTiming_WakeUnloadedWheelSlots();
            CarPart_SmoothSteeringOffset();
            pTable = &g_unk0x00590d7c[3];
            do {
                pPart = (BYTE *)*pTable + g_partCar->index * 0x1a0;
                g_partState = (PartState *)pPart;
                if (*(int *)pPart != 0 && (pPart[0x150] & 1)) {
                    memcpy(pPart + 0x4c, pPart + 0xc, 0x40);
                    FixMatrix_Multiply(&g_partState->worldMatrix,
                                       &g_partState->matrix,
                                       g_partState->pParent);
                    if (*(void (**)(void))((BYTE *)g_partState + 0x110) != NULL)
                        (*(void (**)(void))((BYTE *)g_partState + 0x110))();
                }
                pTable--;
            } while ((int)pTable >= (int)g_unk0x00590d7c);
            Car_BreakQueuedWindows(g_partCar);
            pIndex--;
        } while (--remaining);
    }
    CarDamage_StepClimbingCarRecords((int)pOrder, count);
}

extern int g_unk0x00543d50;
extern int g_stageLighting[];
extern FixVector g_stageLightDirection;
extern BillboardDef g_unk0x00543f00;
extern BillboardDef g_unk0x00547908;
extern int g_unk0x00543ef8;
void FixMatrix_GetForward(FixVector *pOut, FixMatrix *pM);
void StageTiming_ResetWeatherSelectionState(void);
void StageObject_SetupWeatherRamps(void);
void StageObject_UpdateSceneAmbientColour(BYTE *pColour);
void StageLights_UpdateDirection(void);
void StageObject_SetUnboostedStageLight(FixVector *pLight);
void Scene_ApplyShadowLightDirection(FixVector *pLightDir);
void Scene_AnimateAmbientRedChannel(void);

#define RAND_FIX() ((int)(__int64)((float)rand() * g_oneOverRandMax * CGraphics::m_65536))

// Shares the 400 weather particles between the views and places each view's
// share in a random box in front of its camera; resets the views' weather
// state.
// FUNCTION: CMR2 0x0045eed0
void StageWeather_DistributeViewParticles(void)
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
void StageWeather_InitParticleAndLightState(void)
{
    BYTE colour[4];
    FixVector light;

    g_unk0x00543ef8 = 1;
    StageObject_SetupWeatherRamps();
    switch (CGameInfo::GetGraphicsOptionBits25To26()) {
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
    StageTiming_ResetSkyLightVectors();
    StageWeather_DistributeViewParticles();
    StageTiming_ResetWeatherSelectionState();
    StageLights_UpdateDirection();
    colour[3] = 0xff;
    colour[0] = (BYTE)(g_stageLighting[0x12] >> 16);
    colour[1] = (BYTE)(g_stageLighting[0x13] >> 16);
    colour[2] = (BYTE)(g_stageLighting[0x14] >> 16);
    StageObject_UpdateSceneAmbientColour(colour);
    light.x = g_stageLighting[0x15];
    light.y = g_stageLighting[0x16];
    light.z = g_stageLighting[0x17];
    StageObject_SetUnboostedStageLight(&light);
    Scene_ApplyShadowLightDirection(&g_stageLightDirection);
    Scene_AnimateAmbientRedChannel();
}
#undef RAND_FIX

void Glow_ResetEntries(void);
void View_SetupCameras(void);
void StageObject_LoadSkidTrailTextures(void);
void StageObjects_Init(void);
void Knockout_ApplyCarGroundMaterials(void);
void Car_SetDrawnFlag(int index, char value);
void Replay_LoadSelectedRun(void);
void Replay_InitRaceSlots(void);
void StageObject_StartAndRegisterFrameCallback(void);
void StageTiming_ResetRaceDriverTimes(void);
void RallyData_ResetStageClockAndRecords(int keepName);
void RallyData_LayoutPlayerSplitBars(void);
void Race_ResetPlayerCallState(void);

// Pushes the stage state to the graphics layer after a device reset: updates
// every subsystem, then re-uploads each live texture with a dummy triangle.
// FUNCTION: CMR2 0x00424c50
void StageTiming_RestoreDeviceStageResources(void)
{
    D3DTLVERTEX verts[3];
    int i;

    Glow_ResetEntries();
    View_SetupCameras();
    StageTiming_InitForceFeedbackSlots();
    StageObject_LoadSkidTrailTextures();
    StageObjects_Init();
    if (CGameInfo::GetGameInfoSessionFlag() != 0) {
        Replay_LoadSelectedRun();
    } else {
        if (CGameInfo::GetConfiguredGameMode() == 4) {
            Knockout_ApplyCarGroundMaterials();
        } else {
            for (i = 0; i < 2; i++) {
                if ((BYTE)RallyData_GetFlag25() != 0 && CGameInfo::GetGameModeOptionBit19() == 0 &&
                    i >= (int)(RallyDataState() & 0xff))
                    Car_SetDrawnFlag(i, -1);
            }
        }
        if (CGameInfo::GetConfiguredGameMode() != 3)
            Replay_InitRaceSlots();
    }
    if ((BYTE)RallyData_GetSelectionFlag26() != 0)
        StageObject_StartAndRegisterFrameCallback();
    StageTiming_ResetRaceDriverTimes();
    StageTiming_ResetParticipatingDriverRecords(1);
    StageTiming_ClearSplitDisplayFlags();
    RallyData_ResetStageClockAndRecords(1);
    RallyData_LayoutPlayerSplitBars();
    Race_ResetPlayerCallState();

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
            CGraphics::ApplyTextureStageChange(0, (int)pTexture);
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
void StageTiming_InitRaceDriverRecords(char param_1)
{
    StageTimingSpread local;
    int *p;

    if (g_unk0x00542604 == 0)
        return;
    StageTiming_Reset();
    StageTiming_GetObjectRampParameters(&local.field_0xa8, &local.field_0xac, &local.field_0xa0, &local.field_0xa4);
    if ((char)RallyDataCountryIndex() == 3) {
        local.field_0xa8 = 0;
        local.field_0xac = 0;
    }
    if (param_1 != 0)
        g_unk0x00541f88 = CMain::GetFrameTime();
    srand(g_unk0x00541f88);
    StageTiming_AssignReservedDriverSlots(CGameInfo::GetConfiguredPlayerCount());
    StageTiming_RefreshPaintSamplesAndTimeSpread((int)&local);
    StageTiming_BuildAndSortCPUSplitTimes((int *)&local);
    StageTiming_RebuildSplitPositions();
    if ((char)RallyData_GetSelectionFlag27() != 0) {
        p = (int *)(g_unk0x00542528 + 0x10);
        do {
            p[-3] = 0;
            p[0] = 0;
            p += 6;
        } while ((int)p < (int)(g_unk0x00542528 + 0xd0));
        g_unk0x00542420[0] = ConvertRawTimeToCentiseconds(*(int *)(g_unk0x0054241c + 0x2d0));
        g_unk0x00542420[1] = ConvertRawTimeToCentiseconds(*(int *)(g_unk0x0054241c + 0x310));
        StageTiming_PairPlayerOpponentSlots();
        StageTiming_ClearSplitDisplayFlags();
        return;
    }
    StageTiming_RebuildSplitDisplaySlots();
    StageTiming_ClearSplitDisplayFlags();
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
void StageTiming_OpenRegionalStageArchives(void)
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
    if (CGameInfo::GetScreenWidth() >= 0x400 && CFrontend::IsTextureWidthSupported(0x400) != 0 &&
        CFrontend::IsTextureHeightSupported(0x400) != 0)
        resolution = 0x400;
    else
        resolution = 0x280;
    if (CFrontend::HasDXT1TextureSupport() != 0)
        sprintf(CFrontend::m_stringDest, g_str0x0051a11c, CInstallInfo::GetBigFilesDir(), resolution);
    else
        sprintf(CFrontend::m_stringDest, g_str0x0051a10c, CInstallInfo::GetBigFilesDir(), resolution);
    CGenericFileLoader::LoadIntoFileRecord((GenericFile *)StageTiming_GetStageFile1(), CFrontend::m_stringDest);
    sprintf(CFrontend::m_stringDest, g_str0x0051a100, CInstallInfo::GetCountrySpecificDir(),
            CGameInfo::GetGameRegionDirectory(), ppSpeech[CGameInfo::GetGameLanguage() & 0xff]);
    CGenericFileLoader::LoadIntoFileRecord((GenericFile *)StageTiming_GetStageFile2(), CFrontend::m_stringDest);
    sprintf(CFrontend::m_stringDest, g_str0x0051a100, CInstallInfo::GetCountrySpecificDir(),
            CGameInfo::GetGameRegionDirectory(), ppText[CGameInfo::GetGameLanguage() & 0xff]);
    CGenericFileLoader::LoadIntoFileRecord((GenericFile *)StageTiming_GetStageFile6(), CFrontend::m_stringDest);
    sprintf(CFrontend::m_stringDest, CFrontend::m_strCommonBfl, CInstallInfo::GetBigFilesDir());
    CGenericFileLoader::LoadIntoFileRecord((GenericFile *)StageTiming_GetStageFile0(), CFrontend::m_stringDest);
    sprintf(CFrontend::m_stringDest, g_str0x0051a0e0, CInstallInfo::GetSoundsDir());
    CGenericFileLoader::LoadIntoFileRecord((GenericFile *)StageTiming_GetStageFile5(), CFrontend::m_stringDest);
    if ((BYTE)RallyData_IsChampionshipFinalStage() != 0) {
        sprintf(CFrontend::m_stringDest, g_str0x0051a0d0, CInstallInfo::GetBigFilesDir());
        CGenericFileLoader::LoadIntoFileRecord((GenericFile *)StageTiming_GetStageFile4(), CFrontend::m_stringDest);
    }
    CGame::RegisterCallback(StageTiming_ReleaseStageArchives, NULL);
}
