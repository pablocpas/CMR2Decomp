#include <windows.h>
#include <string.h>
#include "NetPlayers.h"
#include "GameInfo.h"
#include "RallyData.h"
#include "main.h"
#include "Game.h"

// GLOBAL: CMR2 0x00531770
int g_netIdCount;
// GLOBAL: CMR2 0x00531778
NetPlayer g_netPlayers[8];
// GLOBAL: CMR2 0x00531b78
int g_netRanks[8];
// GLOBAL: CMR2 0x00531b98
int g_netIdsUnsorted[8];
// GLOBAL: CMR2 0x00531bb8
NetResult g_netResults[8];
// GLOBAL: CMR2 0x00531c98
unsigned int g_netStageBest[10];
// GLOBAL: CMR2 0x00531cc0
int g_netIds[8];
// GLOBAL: CMR2 0x00531ce0
unsigned int g_netPrevBest;
// GLOBAL: CMR2 0x00531ce4
int g_netNewRecord;
// GLOBAL: CMR2 0x00531ce8
int g_netTotal;
// GLOBAL: CMR2 0x00531cec
int g_netStandingCount;
// GLOBAL: CMR2 0x00531cf0
char g_netRecordName[0xe8];
// GLOBAL: CMR2 0x00531dd8
int g_netRanks2[8];
// GLOBAL: CMR2 0x00531df8
unsigned int g_netBestTime;
// GLOBAL: CMR2 0x00531e00
NetStanding g_netStandings[8];
// GLOBAL: CMR2 0x00531ec0
NetStanding g_netStandings2[8];
// GLOBAL: CMR2 0x00531f80
int g_netClassCount;
// GLOBAL: CMR2 0x00531f84
unsigned int g_netSplitBest[8];
// GLOBAL: CMR2 0x005320a8
unsigned int g_netLapBest;
// GLOBAL: CMR2 0x005320b0
NetClassification g_netClassification[7];

extern int g_unk0x005320a4;

void FUN_0040e890(void);
void FUN_0040e8a0(BYTE *p);
BYTE *FUN_0040e8c0(void);
int FUN_004a19c0(DPID *pId, char *pIndex);
char *FUN_004a1b60(BYTE index);
DPID FUN_004a1a00(void);
int FUN_004a1cb0(int param2, int param3);

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
    for (i = 0; i < 8; i++)
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
    int i;

    for (i = 0; i < 8; i++)
        g_netPlayers[i].flags &= ~0x100;
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
    return g_netSplitBest[split - 1];
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

// FUNCTION: CMR2 0x0040afd0
void FUN_0040afd0(void)
{
    int i;

    for (i = 0; i < 8; i++)
        g_netPlayers[i].flags |= 0x400000;
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
