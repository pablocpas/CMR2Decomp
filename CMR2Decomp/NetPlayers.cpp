#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include "NetPlayers.h"
#include "GameInfo.h"
#include "RallyData.h"
#include "main.h"
#include "Game.h"
#include "FixedPoint.h"

// GLOBAL: CMR2 0x005168b0
BYTE g_netColours[8][4] = {
    { 0x1a, 0x3b, 0xa4, 0xff }, { 0xb2, 0x0f, 0x0f, 0xff }, { 0x2c, 0x6f, 0x15, 0xff }, { 0x6c, 0x23, 0x86, 0xff },
    { 0xb4, 0x04, 0x79, 0xff }, { 0xbd, 0x57, 0x03, 0xff }, { 0xff, 0xea, 0x00, 0xff }, { 0xff, 0x8d, 0x00, 0xff }
};
// GLOBAL: CMR2 0x005168d0
BYTE g_netRandomColour[4] = { 0xff, 0xff, 0xff, 0xff };
// GLOBAL: CMR2 0x00531770
int g_netIdCount;
// GLOBAL: CMR2 0x00531778
NetTables g_netTables;
// GLOBAL: CMR2 0x00531e00
NetStandingsTables g_netStandingsTables;

// GLOBAL: CMR2 0x005320a8
unsigned int g_netLapBest;
// GLOBAL: CMR2 0x005320b0
NetClassification g_netClassification[8];

extern int g_unk0x005320a4;

void FUN_0040e890(void);
void FUN_0040e8a0(BYTE *p);
BYTE *FUN_0040e8c0(void);
int FUN_004a19c0(DPID *pId, char *pIndex);
char *FUN_004a1b60(BYTE index);
DPID FUN_004a1a00(void);
char FUN_004a1cb0(int data, int size);
unsigned int FUN_00448680(int index, int split);
char FUN_004a1c50(int to, int guaranteed, int data, int size);

// FUNCTION: CMR2 0x00409a30
void FUN_00409a30(void)
{
    int i;

    memset(g_netPlayers, 0, sizeof(g_netPlayers));
    memset(g_netResults, 0, sizeof(g_netResults));
    memset(g_netStandings, 0, sizeof(g_netStandings));
    for (i = 0; i < 8; i++) {
        g_netStandings[i].index = -1;
        g_netStandings[i].id = 0;
        g_netStandings[i].time = -1;
    }
    for (i = 0; i < 7; i++)
        g_netPlayers[i].bestTime = -1;
    g_netTotal = 0;
    g_netBestTime = -1;
    FUN_0040e890();
    RallyData_UpdateFlags();
}

// FUNCTION: CMR2 0x00409ab0
void FUN_00409ab0(char keepReady, char resetTotal)
{
    int i;

    for (i = 0; i < 8; i++) {
        g_netPlayers[i].flags &= ~0x200;
        g_netPlayers[i].field_0xc = 0;
        if (keepReady == 0)
            g_netPlayers[i].flags &= ~0x100;
        g_netStandings[i].index = -1;
        memset(g_netPlayers[i].splits, 0, sizeof(g_netPlayers[i].splits));
        memset(g_netPlayers[i].stageTimes, 0, sizeof(g_netPlayers[i].stageTimes));
        memset(&g_netPlayers[i].stats, 0, sizeof(g_netPlayers[i].stats));
        g_netStandings[i].id = 0;
        g_netStandings[i].time = -1;
    }
    if (resetTotal != 0)
        g_netTotal = 0;
    if (CGameInfo::FUN_00405d80() == 8 || CGameInfo::FUN_00405d80() == 9 || CGameInfo::FUN_00405d80() == 11)
        FUN_00409b60();
    FUN_0040e890();
    RallyData_UpdateFlags();
}

// match 80%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00409b60
void FUN_00409b60(void)
{
    memset(g_netSplitBest, 0, sizeof(g_netSplitBest));
    memset(g_netStageBest, 0, sizeof(g_netStageBest));
    strcpy(g_netRecordName, CMain::m_logFileBlankLine);
    g_netLapBest = 0;
    g_netPrevBest = 0;
    g_netNewRecord = 0;
}

// FUNCTION: CMR2 0x00409bc0
void FUN_00409bc0(void)
{
    unsigned int *p;

    // The original walks the flags byte of each record up to g_netRanks[1],
    // the global that follows g_netPlayers.
    for (p = &g_netPlayers[0].flags; (int)p < (int)&g_netRanks[1]; p += 0x20)
        *p &= ~0x100;
}

// FUNCTION: CMR2 0x00409be0
void FUN_00409be0(int param)
{
    FUN_004a1cb0(param, 0x10);
}

// FUNCTION: CMR2 0x00409bf0
void FUN_00409bf0(DPID *pId, NetPlayerInfo *pInfo, char add)
{
    DPID id;
    int i;

    id = *pId;
    if (FUN_004a1a00() == id)
        return;
    for (i = 6; i >= 0; i--) {
        if ((g_netPlayers[i].flags & 0x80) && g_netPlayers[i].id == id)
            goto found;
    }
    if (add == 0)
        return;
    for (i = 6; i >= 0; i--) {
        if (!(g_netPlayers[i].flags & 0x80))
            goto found;
    }
    return;
found:
    *(NetPlayerInfo *)&g_netPlayers[i] = *pInfo;
    g_netPlayers[i].id = *pId;
}

// FUNCTION: CMR2 0x00409c80
void FUN_00409c80(int *pId)
{
    int i;

    for (i = 0; i < 7; i++) {
        if (*pId == g_netPlayers[i].id)
            g_netPlayers[i].flags &= ~0x80;
    }
}

// FUNCTION: CMR2 0x00409cb0
unsigned int FUN_00409cb0(int index)
{
    return g_netPlayers[index].flags >> 7 & 1;
}

// FUNCTION: CMR2 0x00409cd0
char *FUN_00409cd0(int index)
{
    char i;

    if (FUN_004a19c0((DPID *)&g_netPlayers[index].id, &i))
        return FUN_004a1b60(i);
    return NULL;
}

// FUNCTION: CMR2 0x00409d00
unsigned int FUN_00409d00(int index)
{
    return g_netPlayers[index].flags & 0x1f;
}

// FUNCTION: CMR2 0x00409d20
int FUN_00409d20(int index)
{
    return g_netPlayers[index].id;
}

// FUNCTION: CMR2 0x00409d30
unsigned int FUN_00409d30(int index)
{
    return g_netPlayers[index].flags >> 5 & 1;
}

// FUNCTION: CMR2 0x00409d50
void FUN_00409d50(DPID *pId, NetStats *pStats)
{
    int i;

    for (i = 0; i < 7; i++) {
        if (g_netPlayers[i].id == *pId)
            g_netPlayers[i].field_0xc++;
        if ((g_netPlayers[i].flags & 0x80) && g_netPlayers[i].id == *pId && !(g_netPlayers[i].flags & 0x400000)) {
            if (g_unk0x00539cc8 != 0 && g_netPlayers[i].stats.seq < pStats->seq) {
                g_netPlayers[i].stats = *pStats;
                g_netPlayers[i].statsNew = 1;
            }
            return;
        }
    }
}

// FUNCTION: CMR2 0x00409dd0
void FUN_00409dd0(void)
{
    int i;

    for (i = 0; i < 7; i++)
        g_netPlayers[i].stats.seq = 0;
}

// FUNCTION: CMR2 0x00409df0
BYTE FUN_00409df0(int index)
{
    return g_netPlayers[index].statsNew;
}

// FUNCTION: CMR2 0x00409e00
void FUN_00409e00(int index)
{
    g_netPlayers[index].statsNew = 0;
}

// FUNCTION: CMR2 0x00409e20
NetStats *FUN_00409e20(int index)
{
    return &g_netPlayers[index].stats;
}

// FUNCTION: CMR2 0x00409e30
void FUN_00409e30(char resetTotal, char resetTimes)
{
    int i;

    for (i = 0; i < 7; i++) {
        if (g_netPlayers[i].flags & 0x80) {
            g_netPlayers[i].flags &= ~0x300;
            g_netPlayers[i].stats.field_0x18 &= 0xfc00;
            if (resetTimes != 0)
                g_netPlayers[i].bestTime = -1;
        }
    }
    if (resetTotal != 0)
        g_netTotal = 0;
    if (resetTimes != 0)
        g_netBestTime = -1;
}

// FUNCTION: CMR2 0x00409e90
void FUN_00409e90(DPID *pId)
{
    int i;

    for (i = 0; i < 7; i++) {
        if ((g_netPlayers[i].flags & 0x80) && g_netPlayers[i].id == *pId) {
            g_netPlayers[i].flags = g_netPlayers[i].flags & ~0x400000 | 0x100;
            return;
        }
    }
}

// FUNCTION: CMR2 0x00409ee0
unsigned int FUN_00409ee0(int index)
{
    return g_netPlayers[index].flags >> 8 & 1;
}

// FUNCTION: CMR2 0x00409f00
void FUN_00409f00(DPID *pId, unsigned int time, int value)
{
    int i;

    for (i = 0; i < 7; i++) {
        if ((g_netPlayers[i].flags & 0x80) && g_netPlayers[i].id == *pId) {
            g_netPlayers[i].flags |= 0x200;
            g_netPlayers[i].time = time;
            g_netPlayers[i].field_0x7c = value;
            if (g_netPlayers[i].time < g_netPlayers[i].bestTime || g_netPlayers[i].bestTime == -1)
                g_netPlayers[i].bestTime = g_netPlayers[i].time;
            return;
        }
    }
}

// FUNCTION: CMR2 0x00409f80
void FUN_00409f80(DPID *pId)
{
    int i;

    for (i = 0; i < 7; i++) {
        if ((g_netPlayers[i].flags & 0x80) && g_netPlayers[i].id == *pId) {
            g_netPlayers[i].flags |= 0x200;
            return;
        }
    }
}

// match 79%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00409fd0
void FUN_00409fd0(DPID *pId, int split, unsigned int time)
{
    int i;
    char *name;

    for (i = 0; i < 7; i++) {
        if ((g_netPlayers[i].flags & 0x80) && g_netPlayers[i].id == *pId) {
            g_netPlayers[i].splits[split - 1] = time;
            if (time < g_netSplitBest[split - 1] || g_netSplitBest[split - 1] == 0) {
                g_netSplitBest[split - 1] = time;
                i = FUN_0040a7a0(*pId);
                if ((char)RallyData_GetFlag25()) {
                    if (split != 2)
                        return;
                } else {
                    if (split != 8)
                        return;
                }
                name = FUN_00409cd0(i);
                if (name != NULL)
                    strcpy(g_netRecordName, name);
                else
                    strcpy(g_netRecordName, CMain::m_logFileBlankLine);
            }
            return;
        }
    }
}

// FUNCTION: CMR2 0x0040a0e0
void FUN_0040a0e0(DPID *pId, int stage, unsigned int time)
{
    int i;
    char *name;

    if (CGameInfo::FUN_00405d80() == 12) {
        for (i = 0; i < 7; i++) {
            if ((g_netPlayers[i].flags & 0x80) && g_netPlayers[i].id == *pId) {
                g_netPlayers[i].stageTimes[0] = time;
                if (time < g_netStageBest[0] || g_netStageBest[0] == 0) {
                    g_netPrevBest = g_netStageBest[0];
                    g_netStageBest[0] = time;
                    g_netNewRecord = 0;
                    name = FUN_00409cd0(FUN_0040a7a0(*pId));
                    if (name != NULL)
                        strcpy(g_netRecordName, name);
                    else
                        strcpy(g_netRecordName, CMain::m_logFileBlankLine);
                    return;
                }
                if (time < g_netPrevBest || g_netPrevBest == 0)
                    g_netPrevBest = time;
                return;
            }
        }
    } else {
        for (i = 0; i < 7; i++) {
            if ((g_netPlayers[i].flags & 0x80) && g_netPlayers[i].id == *pId) {
                g_netPlayers[i].stageTimes[stage - 1] = time;
                if (time < g_netStageBest[stage - 1] || g_netStageBest[stage - 1] == 0)
                    g_netStageBest[stage - 1] = time;
                return;
            }
        }
    }
}

// FUNCTION: CMR2 0x0040a230
void FUN_0040a230(int splitCount)
{
    int i;
    unsigned int time;

    for (i = 1; i <= splitCount; i++) {
        time = FUN_00448680(0, i);
        if (time < g_netSplitBest[i - 1] || g_netSplitBest[i - 1] == 0) {
            g_netSplitBest[i - 1] = time;
            if ((char)RallyData_GetFlag25()) {
                if (i == 2)
                    strcpy(g_netRecordName, (char *)RallyData_GetRecord(0));
            } else {
                if (i == 8)
                    strcpy(g_netRecordName, (char *)RallyData_GetRecord(0));
            }
        }
    }
    if ((char)RallyData_GetFlag25()) {
        if (splitCount != 2)
            return;
        time = FUN_00448680(0, 2);
        if (time < g_netLapBest || g_netLapBest == 0)
            g_netLapBest = time;
        return;
    } else {
        if (splitCount != 8)
            return;
        time = FUN_00448680(0, 8);
        if (time < g_netLapBest || g_netLapBest == 0)
            g_netLapBest = time;
    }
}

// FUNCTION: CMR2 0x0040a330
void FUN_0040a330(unsigned int time, int stage)
{
    if (CGameInfo::FUN_00405d80() == 12) {
        if (time < g_netStageBest[0] || g_netStageBest[0] == 0) {
            g_netPrevBest = g_netStageBest[0];
            g_netStageBest[0] = time;
            strcpy(g_netRecordName, (char *)RallyData_GetRecord(0));
            g_netNewRecord = 1;
            return;
        }
        g_netNewRecord = 0;
        return;
    }
    if (time < g_netTables.words[0x51c / 4 + stage] || g_netTables.words[0x51c / 4 + stage] == 0)
        g_netTables.words[0x51c / 4 + stage] = time;
}

// FUNCTION: CMR2 0x0040a3c0
int FUN_0040a3c0(void)
{
    return g_netNewRecord;
}

// FUNCTION: CMR2 0x0040a3d0
unsigned int FUN_0040a3d0(void)
{
    return g_netLapBest;
}

// FUNCTION: CMR2 0x0040a3e0
void FUN_0040a3e0(unsigned int time)
{
    if (time < g_netLapBest || g_netLapBest == 0)
        g_netLapBest = time;
}

// FUNCTION: CMR2 0x0040a400
char *FUN_0040a400(void)
{
    return g_netRecordName;
}

// FUNCTION: CMR2 0x0040a410
unsigned int FUN_0040a410(int split)
{
    return g_netSplitRecords.words[split];
}

// FUNCTION: CMR2 0x0040a440
unsigned int FUN_0040a440(void)
{
    return g_netPrevBest;
}

// FUNCTION: CMR2 0x0040a450
unsigned int FUN_0040a450(int index)
{
    return g_netPlayers[index].flags >> 9 & 1;
}

// FUNCTION: CMR2 0x0040a470
unsigned int FUN_0040a470(int index)
{
    return g_netPlayers[index].flags >> 6 & 1;
}

// qsort comparator for g_netResults
// FUNCTION: CMR2 0x0040a490
int __cdecl FUN_0040a490(const void *a, const void *b)
{
    NetResult *p1 = (NetResult *)a;
    NetResult *p2 = (NetResult *)b;

    if (p1->index != -1 && p2->index == -1)
        return -1;
    if (p1->index == -1 && p2->index != -1)
        return 1;
    if (p1->index == -1 && p2->index == -1)
        return 0;
    if (p1->field_0x14 != 0 && p2->field_0x14 == 0)
        return -1;
    if (p1->field_0x14 == 0 && p2->field_0x14 != 0)
        return 1;
    if (p1->field_0x14 != 0 && p2->field_0x14 != 0) {
        if ((unsigned int)p1->field_0x18 < (unsigned int)p2->field_0x18)
            return -1;
        if ((unsigned int)p1->field_0x18 > (unsigned int)p2->field_0x18)
            return 1;
    } else {
        if (p1->field_0xc > p2->field_0xc)
            return -1;
        if (p1->field_0xc < p2->field_0xc)
            return 1;
        if (p1->field_0x4 > p2->field_0x4)
            return -1;
        if (p1->field_0x4 < p2->field_0x4)
            return 1;
        if (p1->field_0x8 > p2->field_0x8)
            return -1;
        if (p1->field_0x8 < p2->field_0x8)
            return 1;
    }
    return (unsigned int)p1->id > (unsigned int)p2->id ? -1 : 1;
}

void FUN_004591e0(int player, int *pCount, int *pFrac);
BYTE *FUN_0041b390(void);
int FUN_004483c0(int index);

// Rebuilds the stage results table: one entry per active remote player (speed,
// sign-extended bits of field_0x1a, finish flag and time) plus the local player
// at the end, then sorts it with FUN_0040a490.
// match 82%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0040a580
void FUN_0040a580(int param1, int param2, int param3)
{
    int i;

    for (i = 0; i < 7; i++) {
        if ((g_netPlayers[i].flags & 0x80) != 0) {
            g_netResults[i].index = i;
            g_netResults[i].field_0x4 = g_netPlayers[i].stats.field_0x18 & 0x3ff;
            g_netResults[i].field_0x8 = FixMulShift32((g_netPlayers[i].stats.speed >> 10) << 16, 0x19645);
            if (g_netResults[i].field_0x8 > 99)
                g_netResults[i].field_0x8 = 99;
            g_netResults[i].id = g_netPlayers[i].id;
            g_netResults[i].field_0xc = (g_netPlayers[i].stats.field_0x1a >> 11) & 0xf;
            if (g_netPlayers[i].stats.field_0x1a & 0x8000)
                g_netResults[i].field_0xc = -g_netResults[i].field_0xc;
            g_netResults[i].field_0x14 = (BYTE)(g_netPlayers[i].flags >> 9) & 1;
            g_netResults[i].field_0x18 = g_netPlayers[i].time;
            if ((char)RallyData_GetFlag25())
                FUN_004591e0(g_netResults[i].index, &g_netResults[i].field_0xc, &g_netResults[i].field_0x4);
        } else {
            g_netResults[i].index = -1;
            g_netResults[i].field_0x4 = -1;
        }
    }
    g_netResults[i].index = -2;
    g_netResults[i].field_0x4 = param1;
    g_netResults[i].field_0x8 = param2;
    g_netResults[i].field_0xc = param3;
    g_netResults[i].id = FUN_004a1a00();
    if ((*(unsigned int *)*(BYTE **)(FUN_0041b390() + 4) & 0xff) > 8)
        g_netResults[i].field_0x14 = 1;
    else
        g_netResults[i].field_0x14 = 0;
    g_netResults[i].field_0x18 = FUN_004483c0(0);
    if ((char)RallyData_GetFlag25())
        FUN_004591e0(g_netResults[i].index, &g_netResults[i].field_0xc, &g_netResults[i].field_0x4);
    qsort(g_netResults, 8, sizeof(NetResult), FUN_0040a490);
}

// FUNCTION: CMR2 0x0040a700
int FUN_0040a700(int index)
{
    return g_netResults[index].index;
}

// FUNCTION: CMR2 0x0040a720
int FUN_0040a720(int id)
{
    int i;

    for (i = 0; i < 8; i++) {
        if (g_netResults[i].id == id)
            return g_netResults[i].field_0x4;
    }
    return 0;
}

// FUNCTION: CMR2 0x0040a760
int FUN_0040a760(int id)
{
    int i;

    for (i = 0; i < 8; i++) {
        if (g_netResults[i].id == id)
            return g_netResults[i].field_0x8;
    }
    return 0;
}

// FUNCTION: CMR2 0x0040a7a0
int FUN_0040a7a0(int id)
{
    int i;

    for (i = 0; i < 7; i++) {
        if (g_netPlayers[i].id == id)
            return i;
    }
    return -1;
}

// qsort comparator for the standings
// FUNCTION: CMR2 0x0040a7d0
int __cdecl FUN_0040a7d0(const void *a, const void *b)
{
    NetStanding *p1 = (NetStanding *)a;
    NetStanding *p2 = (NetStanding *)b;

    if (p1->index == -1 || p1->time == -1)
        return 1;
    if (p2->index == -1 || p2->time == -1)
        return -1;
    if (p1->time > p2->time)
        return 1;
    if (p1->time < p2->time)
        return -1;
    return (unsigned int)p1->id > (unsigned int)p2->id ? 1 : -1;
}

// Stage standings of all players, sorted by time, plus their ranks.
// match 87%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0040a820
void FUN_0040a820(unsigned int localTime)
{
    int rank;
    int i;
    char *name;

    rank = 1;
    g_netStandingCount = 1;
    for (i = 0; i < 7; i++) {
        if (g_netPlayers[i].flags & 0x80) {
            g_netStandings2[i].index = i;
            g_netStandings2[i].time = g_netPlayers[i].time;
            g_netStandings2[i].id = g_netPlayers[i].id;
            g_netStandings2[i].car = FUN_00409d00(i);
            name = FUN_00409cd0(i);
            if (name != NULL) {
                strcpy(g_netStandings2[i].name, name);
                g_netStandingCount++;
                continue;
            }
        }
        g_netStandings2[i].index = -1;
    }
    g_netStandings2[i].index = -2;
    g_netStandings2[i].time = localTime;
    g_netStandings2[i].id = FUN_004a1a00();
    strcpy(g_netStandings2[i].name, (char *)RallyData_GetRecord(0));
    g_netStandings2[i].car = (BYTE)RallyData_FUN_004086b0(0);
    qsort(g_netStandings2, 8, sizeof(NetStanding), FUN_0040a7d0);
    g_netRanks2[0] = 1;
    for (i = 1; i < 8; i++) {
        if (g_netStandings2[i].index == -1)
            return;
        if (g_netStandings2[i].time != g_netStandings2[i - 1].time)
            rank++;
        g_netRanks2[i] = rank;
    }
}

// Overall standings: accumulates the local time and awards 3 points to the leaders.
// FUNCTION: CMR2 0x0040a980
void FUN_0040a980(unsigned int localTime)
{
    int rank;
    int i;
    char *name;

    g_netTotal += localTime;
    rank = 1;
    for (i = 0; i < 7; i++) {
        if ((g_netPlayers[i].flags & 0x80) && (name = FUN_00409cd0(i)) != NULL) {
            strcpy(g_netStandings[i].name, name);
            g_netStandings[i].index = i;
            g_netStandings[i].time = g_netPlayers[i].field_0x7c;
            g_netStandings[i].id = g_netPlayers[i].id;
            g_netStandings[i].car = FUN_00409d00(i);
        } else {
            g_netStandings[i].index = -1;
        }
    }
    g_netStandings[i].index = -2;
    g_netStandings[i].time = g_netTotal;
    g_netStandings[i].id = FUN_004a1a00();
    strcpy(g_netStandings[i].name, (char *)RallyData_GetRecord(0));
    g_netStandings[i].car = (BYTE)RallyData_FUN_004086b0(0);
    qsort(g_netStandings, 8, sizeof(NetStanding), FUN_0040a7d0);
    g_netRanks[0] = 1;
    for (i = 1; i < 8; i++) {
        if (g_netStandings[i].index == -1)
            break;
        if (g_netStandings[i].time != g_netStandings[i - 1].time)
            rank++;
        g_netRanks[i] = rank;
    }
    for (i = 0; i < 8; i++)
        g_netStandings[i].points = g_netRanks[i] == 1 ? 3 : 0;
}

// FUNCTION: CMR2 0x0040ab10
int FUN_0040ab10(void)
{
    return g_netStandingCount;
}

// FUNCTION: CMR2 0x0040ab20
int FUN_0040ab20(int index, int total)
{
    if (total == 0)
        return g_netRanks2[index];
    return g_netRanks[index];
}

// FUNCTION: CMR2 0x0040ab50
int FUN_0040ab50(int index, int total)
{
    if (total == 0)
        return g_netStandings2[index].index;
    return g_netStandings[index].index;
}

// FUNCTION: CMR2 0x0040ab80
unsigned int FUN_0040ab80(int index, int total)
{
    if (total == 0)
        return g_netStandings2[index].time;
    return g_netStandings[index].time;
}

// FUNCTION: CMR2 0x0040abb0
char *FUN_0040abb0(int index, int total)
{
    if (total == 0)
        return g_netStandings2[index].name;
    return g_netStandings[index].name;
}

// FUNCTION: CMR2 0x0040abe0
unsigned int FUN_0040abe0(int index, int total)
{
    if (total == 0)
        return g_netStandings2[index].car;
    return g_netStandings[index].car;
}

// FUNCTION: CMR2 0x0040ac10
int FUN_0040ac10(int index)
{
    return g_netStandings[index].points;
}

// FUNCTION: CMR2 0x0040ac30
int FUN_0040ac30(void)
{
    return g_netTotal;
}

// FUNCTION: CMR2 0x0040ac40
void FUN_0040ac40(BYTE carClass)
{
    BYTE msg[2];

    msg[0] = 6;
    msg[1] = carClass;
    FUN_004a1c50(0, 1, (int)msg, 2);
}

// FUNCTION: CMR2 0x0040ac70
void FUN_0040ac70(DPID *pId, unsigned int carClass)
{
    DPID id;
    int i;

    id = *pId;
    if (FUN_004a1a00() == id)
        return;
    for (i = 0; i < 7; i++) {
        if ((g_netPlayers[i].flags & 0x80) && g_netPlayers[i].id == id) {
            g_netPlayers[i].flags = (carClass & 0xf) << 18 | g_netPlayers[i].flags & 0xffc3ffff;
            return;
        }
    }
}

// qsort comparator for g_netClassification
// FUNCTION: CMR2 0x0040acd0
int __cdecl FUN_0040acd0(const void *a, const void *b)
{
    NetClassification *p1 = (NetClassification *)a;
    NetClassification *p2 = (NetClassification *)b;

    if (p1->time == -1 && p2->time != -1)
        return 1;
    if (p2->time == -1 && p1->time != -1)
        return -1;
    if (p1->time > p2->time)
        return 1;
    if (p1->time < p2->time)
        return -1;
    return (unsigned int)p1->id > (unsigned int)p2->id ? 1 : -1;
}

// Final classification: best time of every player plus the local one.
// FUNCTION: CMR2 0x0040ad20
void FUN_0040ad20(void)
{
    int i;
    int count;
    char *name;

    g_netClassCount = 0;
    for (i = 0; i < 7; i++) {
        if (g_netPlayers[i].flags & 0x80) {
            g_netClassification[g_netClassCount].id = g_netPlayers[i].id;
            g_netClassification[g_netClassCount].carClass = g_netPlayers[i].flags >> 18 & 0xf;
            g_netClassification[g_netClassCount].time = g_netPlayers[i].bestTime;
            name = FUN_00409cd0(i);
            if (name != NULL) {
                strcpy(g_netClassification[g_netClassCount].name, name);
                g_netClassCount++;
            }
        }
    }
    g_netClassification[g_netClassCount].carClass = 3;
    g_netClassification[g_netClassCount].time = g_netBestTime;
    strcpy(g_netClassification[g_netClassCount].name, (char *)RallyData_GetRecord(0));
    g_netClassification[g_netClassCount].id = FUN_004a1a00();
    count = g_netClassCount + 1;
    g_netClassCount = count;
    for (i = count; i < 7; i++) {
        g_netClassification[i].carClass = 0;
        g_netClassification[i].time = -1;
        strcpy(g_netClassification[i].name, CMain::m_logFileBlankLine);
        g_netClassification[i].id = 0;
    }
    qsort(g_netClassification, count, sizeof(NetClassification), FUN_0040acd0);
}

// FUNCTION: CMR2 0x0040ae90
int FUN_0040ae90(void)
{
    return g_netClassCount;
}

// FUNCTION: CMR2 0x0040aea0
NetClassification *FUN_0040aea0(int index)
{
    return &g_netClassification[index];
}

// FUNCTION: CMR2 0x0040aeb0
unsigned int FUN_0040aeb0(int index)
{
    return g_netClassification[index].carClass;
}

// FUNCTION: CMR2 0x0040aec0
unsigned int FUN_0040aec0(int index)
{
    return g_netClassification[index].time;
}

// FUNCTION: CMR2 0x0040aed0
int FUN_0040aed0(void)
{
    int i;

    for (i = 0; i < 8; i++) {
        if (g_netClassification[i].id == FUN_004a1a00())
            return i;
    }
    return 0;
}

// FUNCTION: CMR2 0x0040af00
void FUN_0040af00(unsigned int time)
{
    if (g_netBestTime == -1 || time < g_netBestTime)
        g_netBestTime = time;
}

// FUNCTION: CMR2 0x0040af30
int FUN_0040af30(void)
{
    return g_unk0x005320a4;
}

// FUNCTION: CMR2 0x0040af40
void FUN_0040af40(void)
{
    BYTE msg;

    msg = 0x10;
    FUN_004a1c50(0, 1, (int)&msg, 1);
}

// FUNCTION: CMR2 0x0040af60
void FUN_0040af60(void)
{
    struct {
        BYTE type;
        BYTE valid;
        BYTE pad[2];
        BYTE data[0x104];
    } msg;
    BYTE *p;

    msg.type = 0x11;
    p = FUN_0040e8c0();
    if (p != NULL) {
        memcpy(msg.data, p, sizeof(msg.data));
        msg.valid = 1;
    } else {
        msg.valid = 0;
    }
    FUN_004a1c50(0, 1, (int)&msg, sizeof(msg));
}

// FUNCTION: CMR2 0x0040afb0
void FUN_0040afb0(char valid, BYTE *p)
{
    FUN_0040e890();
    if (valid != 0)
        FUN_0040e8a0(p);
}

// FUNCTION: CMR2 0x0040afd0
void FUN_0040afd0(void)
{
    unsigned int *p;

    // Same bound as FUN_00409bc0 (see there).
    for (p = &g_netPlayers[0].flags; (int)p < (int)&g_netRanks[1]; p += 0x20)
        *p |= 0x400000;
}

// FUNCTION: CMR2 0x0040aff0
void FUN_0040aff0(int index, int value)
{
    g_netPlayers[index].field_0x8 = value;
}

// FUNCTION: CMR2 0x0040b010
int FUN_0040b010(int index)
{
    return g_netPlayers[index].field_0x8;
}

// FUNCTION: CMR2 0x0040b020
int FUN_0040b020(int value)
{
    int i;

    for (i = 0; i < 7; i++) {
        if ((g_netPlayers[i].flags & 0x80) && g_netPlayers[i].field_0x8 == value)
            return i;
    }
    return 0;
}

// FUNCTION: CMR2 0x0040b050
bool FUN_0040b050(int value)
{
    int i;

    for (i = 0; i < 7; i++) {
        if ((g_netPlayers[i].flags & 0x80) && g_netPlayers[i].field_0x8 == value)
            return true;
    }
    return false;
}

// qsort comparator for g_netIds
// FUNCTION: CMR2 0x0040b080
int __cdecl FUN_0040b080(const void *a, const void *b)
{
    return *(unsigned int *)a > *(unsigned int *)b ? 1 : -1;
}

// Colour of a player's name, a random one for unknown ids.
// FUNCTION: CMR2 0x0040b0a0
BYTE *FUN_0040b0a0(int id)
{
    int i;

    for (i = 0; i < 8; i++) {
        if (id == g_netIds[i])
            return g_netColours[i];
    }
    g_netRandomColour[0] = rand() % 256;
    g_netRandomColour[1] = rand() % 256;
    g_netRandomColour[2] = rand() % 256;
    return g_netRandomColour;
}

// FUNCTION: CMR2 0x0040b120
void FUN_0040b120(void)
{
    int i;

    g_netIdCount = 0;
    g_netIdsUnsorted[g_netIdCount] = g_netIds[g_netIdCount] = FUN_004a1a00();
    g_netIdCount++;
    for (i = 0; i < 7; i++) {
        if ((BYTE)FUN_00409cb0(i)) {
            g_netIdsUnsorted[g_netIdCount] = g_netIds[g_netIdCount] = FUN_00409d20(i);
            g_netIdCount++;
        }
    }
    qsort(g_netIds, g_netIdCount, sizeof(int), FUN_0040b080);
}

// FUNCTION: CMR2 0x0040b1a0
int FUN_0040b1a0(int index)
{
    return g_netIds[index];
}

// FUNCTION: CMR2 0x0040b1b0
int FUN_0040b1b0(void)
{
    return g_netIdCount;
}

// FUNCTION: CMR2 0x0040b1c0
void FUN_0040b1c0(void)
{
    int i;

    for (i = 0; i < 7; i++)
        g_netPlayers[i].flags &= ~0x800000;
}

// FUNCTION: CMR2 0x0040b1e0
unsigned int FUN_0040b1e0(int index)
{
    return g_netPlayers[index].flags >> 23 & 1;
}

// match 85%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0040b200
void FUN_0040b200(int index)
{
    g_netPlayers[index].flags |= 0x800000;
}
