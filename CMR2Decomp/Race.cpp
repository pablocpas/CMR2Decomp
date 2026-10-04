#include <windows.h>
#include "Game.h"
#include "RallyData.h"
#include "GameInfo.h"
#include "StageUI.h"
#include "Sprite.h"
#include "FixedPoint.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "InstallInfo.h"
#include "Frontend.h"
#include "Graphics.h"
#include "StageTiming.h"
#include "GenericFileLoader.h"
#include "FileBuffer.h"
#include "Input.h"
#include "Menu.h"
#include "main.h"
#include "Sound.h"
#include "NetworkLeaderboards.h"
#include "RegKey.h"

// Race session state (0x41e210-0x420190)

unsigned char RallyDataState(void);
unsigned char RallyData_FUN_00407e70(void);
unsigned int RallyData_FUN_00407e90(void);

// GLOBAL: CMR2 0x005191a0
BYTE g_unk0x005191a0 = 0xff;

struct RaceSlotState {
    int owner;
    int pending;
    BYTE flags;
    BYTE unused[3];
};
// GLOBAL: CMR2 0x005370a0
RaceSlotState g_raceSlotState[20];

// Assigns an unused race slot and marks its owner for refresh.
// FUNCTION: CMR2 0x00417660
void Race_AssignUnusedSlot(int owner)
{
    int i = 0;
    do {
        if ((g_raceSlotState[i].flags & 2) == 0) {
            g_raceSlotState[i].flags |= 2;
            g_raceSlotState[i].owner = owner;
            g_raceSlotState[i].flags &= 0xfe;
            g_raceSlotState[i].pending = -1;
            i = 20;
        }
        i++;
    } while (i < 20);
}

int FUN_00417760(int index);
int Sound_IsPlaying(unsigned int handle);
int FUN_004b7790(unsigned short id, int volume, int frequency, int loopStart, int loops, int is3D);
extern int g_unk0x00537194;

// Plays the queued co-driver calls one after another: starts the first slot's
// sample, and when it has finished moves the queue up.
// FUNCTION: CMR2 0x004176b0
void RaceCalls_UpdatePlaybackQueue(void)
{
    if ((g_raceSlotState[0].flags & 2) != 0 && FUN_00417760(0) == 0) {
        FUN_00417760(0);
        if ((g_raceSlotState[0].flags & 1) != 0) {
            if (Sound_IsPlaying(g_raceSlotState[0].pending) == 0) {
                for (int i = 1; i < 20; i++) {
                    g_raceSlotState[i - 1] = g_raceSlotState[i];
                }
                g_raceSlotState[19].flags &= 0xfc;
                g_raceSlotState[19].pending = -1;
                g_raceSlotState[19].owner = -1;
            }
        } else {
            g_raceSlotState[0].pending =
                FUN_004b7790((unsigned short)g_raceSlotState[0].owner, g_unk0x00537194, 0x2b11, 0, 0, 0);
            g_raceSlotState[0].flags |= 1;
        }
    }
}
// GLOBAL: CMR2 0x00537f08
BYTE g_unk0x00537f08;
// GLOBAL: CMR2 0x00537190
int g_unk0x00537190;
// GLOBAL: CMR2 0x00537194
int g_unk0x00537194;
// GLOBAL: CMR2 0x00537394
int g_unk0x00537394;
// GLOBAL: CMR2 0x00537f0c
int g_unk0x00537f0c[6];
// GLOBAL: CMR2 0x00537f24
int g_unk0x00537f24;
// GLOBAL: CMR2 0x00537f3c
BYTE *g_unk0x00537f3c[8];
// GLOBAL: CMR2 0x00537f60
int g_unk0x00537f60;
// GLOBAL: CMR2 0x00537f68
int g_unk0x00537f68[4];
// GLOBAL: CMR2 0x00537f78
int g_unk0x00537f78[7];
// The frame callback flag is the sixth dword of the same race state block.
#define g_unk0x00537f8c (g_unk0x00537f78[5])
// GLOBAL: CMR2 0x00537f94
int g_unk0x00537f94;
// GLOBAL: CMR2 0x00537ffa
BYTE g_unk0x00537ffa;
// GLOBAL: CMR2 0x00537fc0
unsigned int g_unk0x00537fc0;
// GLOBAL: CMR2 0x00538108
int g_unk0x00538108;
// GLOBAL: CMR2 0x0053810c
BYTE g_unk0x0053810c;
// GLOBAL: CMR2 0x0053810d
BYTE g_unk0x0053810d;
// GLOBAL: CMR2 0x00538114
int g_unk0x00538114;
// GLOBAL: CMR2 0x00538118
int g_unk0x00538118;
// GLOBAL: CMR2 0x0053811c
BYTE g_unk0x0053811c;
// GLOBAL: CMR2 0x00537f00
int g_unk0x00537f00;
// GLOBAL: CMR2 0x00537f34
int g_unk0x00537f34[2];
// GLOBAL: CMR2 0x00537f98
int g_unk0x00537f98[8];
// GLOBAL: CMR2 0x00537fbc
BYTE g_unk0x00537fbc[2];
// GLOBAL: CMR2 0x00537fcc
int g_unk0x00537fcc;
// GLOBAL: CMR2 0x00537ff8
BYTE g_unk0x00537ff8[2];
// GLOBAL: CMR2 0x00538128
int g_unk0x00538128;
// GLOBAL: CMR2 0x0053823c
char g_unk0x0053823c[MAX_PATH];
// GLOBAL: CMR2 0x00538340
char g_unk0x00538340[MAX_PATH];
// GLOBAL: CMR2 0x00538444
char g_unk0x00538444[MAX_PATH];
// GLOBAL: CMR2 0x0053874c
char g_unk0x0053874c[MAX_PATH];
// GLOBAL: CMR2 0x00538850
int g_unk0x00538850;
// GLOBAL: CMR2 0x00538858
GenericFile g_raceFile;
// GLOBAL: CMR2 0x00538864
BYTE g_raceFileCallbackSet;
// GLOBAL: CMR2 0x00538970
int g_unk0x00538970;

// GLOBAL: CMR2 0x00538110
int g_unk0x00538110;
// GLOBAL: CMR2 0x005192d4
char g_str0x005192d4[] = "%sL.rpl";
// GLOBAL: CMR2 0x005192dc
char g_str0x005192dc[] = "%sH.rpl";
// GLOBAL: CMR2 0x005192b0
char g_strGrp0x005192b0[] = "%s.grp";
// GLOBAL: CMR2 0x00519228
int g_unk0x00519228 = -1;
// GLOBAL: CMR2 0x0051922c
int g_unk0x0051922c = -1;
// GLOBAL: CMR2 0x0051948c
char g_strTempC3D[] = "TEMP.C3D";

int RallyData_FUN_00411060(void);
int FUN_004b9380(unsigned int, unsigned int, unsigned int);
BYTE *FUN_0046d2d0(char *path);
char *FUN_0041f910(void);
GenericFile *FUN_0041f500(void);

// Country codes and difficulty letters of the CPU run file names; the second
// pair of tables is the one Replay_InitRaceSlots uses for the second player.
// GLOBAL: CMR2 0x00519254
char g_str0x00519254[4] = "e";
// GLOBAL: CMR2 0x00519258
char g_str0x00519258[4] = "i";
// GLOBAL: CMR2 0x0051925c
char g_str0x0051925c[4] = "n";
extern char g_str0x00519260[4];
extern char g_str0x00519268[4];
extern char g_str0x0051926c[4];
extern char g_str0x00519270[4];
extern char g_str0x00519274[4];
extern char g_str0x00519278[4];
extern char g_str0x0051927c[4];
extern char g_str0x00519280[4];
// GLOBAL: CMR2 0x005191c8
char *g_cpuRunCountries[9] = { g_str0x00519280, g_str0x0051927c, g_str0x00519278, g_str0x00519274, g_str0x00519270,
                               g_str0x0051926c, g_str0x00519268, CFrontend::m_strUK, g_str0x00519260 };
// GLOBAL: CMR2 0x005191ec
char *g_cpuRunLevels[3] = { g_str0x0051925c, g_str0x00519258, g_str0x00519254 };
// GLOBAL: CMR2 0x005191f8
char *g_cpuRunCountries2[9] = { g_str0x00519280, g_str0x0051927c, g_str0x00519278, g_str0x00519274, g_str0x00519270,
                                g_str0x0051926c, g_str0x00519268, CFrontend::m_strUK, g_str0x00519260 };
// GLOBAL: CMR2 0x0051921c
char *g_cpuRunLevels2[3] = { g_str0x0051925c, g_str0x00519258, g_str0x00519254 };
// GLOBAL: CMR2 0x00519294
char g_strCpuRun[] = "%s\\CPU_Runs\\%s%d%d%s.rpl";
// GLOBAL: CMR2 0x005192b8
char g_strChampEndRun[] = "%s\\CPU_Runs\\champend.rpl";

int FUN_004584c0(void);
BYTE *FUN_0046c5a0(short frames, short samples, int type);
void FUN_00465f90(char *path);
char *FUN_0041f8e0(void);
char *FUN_0041f920(void);
unsigned int FUN_00409cb0(int);
BYTE Car_GetDrawnFlag(int index);
int RallyData_IsChampionshipFinalStage(void);
unsigned char RallyData_FUN_00407ea0(void);

// Loads the CPU run a player races against: the file is chosen by country,
// stage, side of the stage and difficulty. Returns whether it was found.
// FUNCTION: CMR2 0x0041c1b0
int Replay_LoadCpuOpponentRun(int player)
{
    if (player == 0)
        g_unk0x00537f78[6] = FUN_004584c0() != 0;
    else
        g_unk0x00537f78[6] = FUN_004584c0() == 0;
    sprintf(CFrontend::m_stringDest, g_strCpuRun, FUN_0041f8e0(), g_cpuRunCountries[(BYTE)RallyDataCountryIndex()],
            (BYTE)RallyDataStageIndex() + 1, g_unk0x00537f78[6],
            g_cpuRunLevels[CGameInfo::FUN_00405d80() != 4 ? CGameInfo::FUN_00405d90()
                                                           : CGameInfo::FUN_00405dd0() - 1]);
    g_unk0x00537f3c[player] = FUN_0046d2d0(CFrontend::m_stringDest);
    return g_unk0x00537f3c[player] != NULL;
}

// Sets up the replay buffers of a race: recording buffers for network games,
// the championship-end run, the CPU runs of a head-to-head or knockout stage,
// and a recording buffer for every player that has no run loaded.
// FUNCTION: CMR2 0x0041c260
void Replay_InitRaceSlots(void)
{
    int headToHead;
    int loaded[2];
    int count;
    int i;
    int flag;

    count = RallyDataState() & 0xff;
    loaded[0] = 0;
    loaded[1] = 0;
    if (CGameInfo::FUN_00405d80() == 4 || (char)RallyData_GetFlag25())
        headToHead = 1;
    else
        headToHead = 0;
    for (i = 0; i < 8; i++)
        g_unk0x00537f3c[i] = NULL;
    if (CGameInfo::FUN_00405e00()) {
        g_unk0x00537f3c[0] = FUN_0046c5a0(1, 0x1d4c, 2);
        for (i = 0; i < 7; i++) {
            g_unk0x00537f3c[i + 1] = NULL;
            if ((char)FUN_00409cb0(i) && CGameInfo::FUN_00405d80() != 10)
                g_unk0x00537f3c[i + 1] = FUN_0046c5a0(1, 0x1d4c, 2);
        }
        return;
    }
    if ((char)RallyData_IsChampionshipFinalStage()) {
        sprintf(CFrontend::m_stringDest, g_strChampEndRun, FUN_0041f8e0());
        g_unk0x00537f3c[0] = FUN_0046d2d0(CFrontend::m_stringDest);
        return;
    }
    if ((char)RallyData_FUN_00407ea0() == 0) {
        for (i = 0; i < 2; i++) {
            if ((char)RallyData_GetFlag25() && CGameInfo::FUN_00405e00() == 0 && CGameInfo::FUN_00405da0()) {
                if (i == 0) {
                    g_unk0x00537f3c[0] = FUN_0046c5a0(1, 0x186a, 0);
                } else {
                    flag = FUN_004584c0() == 0;
                    sprintf(CFrontend::m_stringDest, g_strCpuRun, FUN_0041f8e0(),
                            g_cpuRunCountries2[(BYTE)RallyDataCountryIndex()], (BYTE)RallyDataStageIndex() + 1,
                            flag,
                            g_cpuRunLevels2[CGameInfo::FUN_00405d80() != 4 ? CGameInfo::FUN_00405d90()
                                                                           : CGameInfo::FUN_00405dd0() - 1]);
                    g_unk0x00537f3c[1] = FUN_0046d2d0(CFrontend::m_stringDest);
                    if (i == 1)
                        return;
                }
            }
        }
        for (i = 0; i < 2; i++) {
            if (((CGameInfo::FUN_00405d80() == 4 && (char)Car_GetDrawnFlag(i) == -1) ||
                 ((char)RallyData_GetFlag25() && CGameInfo::FUN_00405e00() == 0 &&
                  RallyData_FUN_00408500((BYTE)i) != -1)) &&
                Replay_LoadCpuOpponentRun(i) != 0)
                loaded[i] = 1;
        }
    }
    for (i = 0; i < count; i++) {
        if (loaded[i] != 0)
            continue;
        if ((char)RallyData_FUN_00407ea0()) {
            if (count == 1)
                g_unk0x00537f3c[i] = FUN_0046c5a0(1, 0x1d4c, 0);
            else
                g_unk0x00537f3c[i] = FUN_0046c5a0(1, 0x186a, 0);
        } else if (count == 1 && headToHead == 0) {
            g_unk0x00537f3c[i] = FUN_0046c5a0(1, 0x1d4c, 0);
        } else {
            g_unk0x00537f3c[i] = FUN_0046c5a0(1, 0x186a, 0);
        }
    }
    if ((char)RallyData_FUN_00407ea0() && (char)CGameInfo::FUN_00406310()) {
        sprintf(CFrontend::m_stringDest, g_strGrp0x005192b0, FUN_0041f920());
        FUN_00465f90(CFrontend::m_stringDest);
    }
}

// Clears the player replay slots, then builds the replay file name for the
// current game mode and loads it into slot 0.
// FUNCTION: CMR2 0x0041c510
void Replay_LoadSelectedRun(void)
{
    int i;

    for (i = 0; i < 8; i++)
        g_unk0x00537f3c[i] = 0;
    if (CGameInfo::FUN_00405d00() == 0) {
        sprintf(CFrontend::m_stringDest, g_str0x005192dc, FUN_0041f910());
    } else {
        sprintf(CFrontend::m_stringDest, g_str0x005192d4, FUN_0041f910());
    }
    if (CGameInfo::FUN_00405d80() != 5 && CGameInfo::FUN_00405d80() != 6 &&
        CGameInfo::FUN_00405d80() != 7 && CGameInfo::FUN_00405d80() != 0xb &&
        CGameInfo::FUN_00405d80() != 0xc) {
        g_unk0x00537f3c[0] = (BYTE *)FUN_0046d2d0(CFrontend::m_stringDest);
        g_unk0x00538110 = 1;
    }
}

// Loads the stage's TEMP.C3D model into memory.
// FUNCTION: CMR2 0x0041fc50
void Race_LoadStageModel(void)
{
    void *pBuffer;
    GenericFile *pFile;

    pBuffer = CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(), g_strTempC3D, NULL, NULL, 0);
    if (pBuffer != NULL) {
        pFile = FUN_0041f500();
        FUN_004b9380((unsigned int)pBuffer, RallyData_FUN_00411060(), (unsigned int)pFile);
    }
}


// FUNCTION: CMR2 0x0041e210
void FUN_0041e210(void)
{
    g_unk0x00537f08 = 1;
}

// FUNCTION: CMR2 0x0041f250
void FUN_0041f250(void)
{
    g_unk0x005191a0 = 0xff;
}

// FUNCTION: CMR2 0x0041f270
int FUN_0041f270(void)
{
    return g_unk0x00537f60;
}

// FUNCTION: CMR2 0x0041f280
void FUN_0041f280(void)
{
    g_unk0x0053810c = 1;
}

// FUNCTION: CMR2 0x0041f290
void FUN_0041f290(void)
{
    g_unk0x00537ffa = 1;
}

// FUNCTION: CMR2 0x0041f2a0
void FUN_0041f2a0(void)
{
    g_unk0x0053810d = 1;
}

// FUNCTION: CMR2 0x0041f350
BYTE *FUN_0041f350(int index)
{
    return g_unk0x00537f3c[index];
}

// FUNCTION: CMR2 0x0041f360
int FUN_0041f360(void)
{
    if (g_unk0x00538114 != 0) {
        g_unk0x00538114 = 0;
        return 1;
    }
    return 0;
}

// FUNCTION: CMR2 0x0041f380
BYTE FUN_0041f380(void)
{
    return g_unk0x005191a0;
}

// FUNCTION: CMR2 0x0041f390
void FUN_0041f390(void)
{
    g_unk0x00538118 = 0;
}

// FUNCTION: CMR2 0x0041f3a0
int FUN_0041f3a0(void)
{
    int result = 0;

    if ((BYTE)RallyDataState() > 1 && **(char **)(FUN_0041b390() + 4) == 10)
        result = 1;
    return result;
}

// FUNCTION: CMR2 0x0041f3d0
int FUN_0041f3d0(BYTE index)
{
    if (g_unk0x00537f3c[index] != NULL)
        return *(int *)(g_unk0x00537f3c[index] + 4);
    return 0;
}

// FUNCTION: CMR2 0x0041f3f0
int FUN_0041f3f0(BYTE index)
{
    if (g_unk0x00537f3c[index] != NULL)
        return *(int *)(g_unk0x00537f3c[index] + 0xc);
    return 0;
}

// FUNCTION: CMR2 0x0041f410
int FUN_0041f410(void)
{
    return 1;
}

// FUNCTION: CMR2 0x0041f4b0
int FUN_0041f4b0(void)
{
    return g_unk0x00538108;
}

// FUNCTION: CMR2 0x0041f4c0
void FUN_0041f4c0(void)
{
    g_unk0x00538108 = 1;
}

// FUNCTION: CMR2 0x0041f4d0
void FUN_0041f4d0(void)
{
    g_unk0x00537f94 = 1;
}

BYTE FUN_0040eef0(void);
void FUN_0041e670(void);
void FUN_0041b300(void);

// Update handler of game state 12 (state table 0x5190b0).
// FUNCTION: CMR2 0x0041f4e0
void FUN_0041f4e0(Unk0049c2c0 *p, BYTE index)
{
    CGame::FUN_004057e0(0);
    FUN_0041e670();
    FUN_0040eef0();
    FUN_0041b300();
}

// FUNCTION: CMR2 0x0041f500
GenericFile *FUN_0041f500(void)
{
    return g_raceFile.didFileLoad ? &g_raceFile : NULL;
}

void StageObject_FreeAll(void);

// Releases the race file loaded by Race_LoadSelectedStage (registered callback).
// FUNCTION: CMR2 0x0041f510
BYTE Race_ReleaseStageFile(void)
{
    if (g_raceFile.didFileLoad && g_raceFile.buffer) {
        CFileBuffer::FreeGenericFileBuffer(g_raceFile.buffer);
        g_raceFile.buffer = NULL;
    }
    g_raceFile.didFileLoad = FALSE;
    g_raceFile.fileSize = 0;
    if (g_unk0x00538850)
        StageObject_FreeAll();
    g_raceFileCallbackSet = 0;
    return 1;
}

extern char g_str0x00519348[6];
extern char g_str0x00519350[6];
extern char g_str0x00519358[7];
extern char g_str0x00519360[7];
extern char g_str0x00519368[7];
extern char g_str0x00519370[8];
extern char g_strPathFormat[];

// Long country names (FINLAND..JAPAN) and codes (FIN..JAP) of the rally
// selection file names.
// GLOBAL: CMR2 0x005192f8
char *g_unk0x005192f8[9] = { g_str0x00519370, g_str0x00519368, g_str0x00519360, g_str0x00519358,
                             g_str0x00519270, g_str0x00519350, g_str0x00519348, CFrontend::m_strUK,
                             CRegKey::m_skuJapan };
// GLOBAL: CMR2 0x0051931c
char *g_unk0x0051931c[9] = { g_str0x00519280, g_str0x0051927c, g_str0x00519278, g_str0x00519274,
                             g_str0x00519270, g_str0x0051926c, g_str0x00519268, CFrontend::m_strUK,
                             g_str0x00519260 };

// GLOBAL: CMR2 0x00519378
char g_str0x00519378[] = "%s\\%s\\%s%st.bfl";
// GLOBAL: CMR2 0x00519388
char g_str0x00519388[] = "%s\\replays\\%s-chL";
// GLOBAL: CMR2 0x0051939c
char g_str0x0051939c[] = "%s\\replays\\%s-chH";
// GLOBAL: CMR2 0x005193b0
char g_str0x005193b0[] = "%s\\Game\\DemoRuns\\%s-ch";
// GLOBAL: CMR2 0x005193c8
char g_str0x005193c8[] = "%s\\%s-ch";
// GLOBAL: CMR2 0x005193d4
char g_str0x005193d4[] = "%s\\cha%.2d%s";
// GLOBAL: CMR2 0x005193e4
char g_str0x005193e4[] = "%s\\chall";
// GLOBAL: CMR2 0x005193f0
char g_str0x005193f0[] = "UKi";
// GLOBAL: CMR2 0x005193f4
char g_str0x005193f4[] = "%s\\%s%st.bfl";
// GLOBAL: CMR2 0x00519404
char g_str0x00519404[] = "%s\\replays\\%s%.2dL";
// GLOBAL: CMR2 0x00519418
char g_str0x00519418[] = "%s\\replays\\%s%.2dH";
// GLOBAL: CMR2 0x0051942c
char g_str0x0051942c[] = "%s\\Game\\DemoRuns\\%s%.2d";
// GLOBAL: CMR2 0x00519444
char g_str0x00519444[] = "%s\\%s%.2d";
// GLOBAL: CMR2 0x00519450
char g_str0x00519450[] = "%s\\%s%.2d%s";
// GLOBAL: CMR2 0x00519464
char g_str0x00519464[4] = "lo";
// GLOBAL: CMR2 0x00519468
char g_str0x00519468[4] = "hi";
extern BYTE g_unk0x00538130[0x40];
// GLOBAL: CMR2 0x00538548
char g_unk0x00538548[MAX_PATH];

// Builds the file names of the current rally selection (the challenge names
// when the game info flags are 5..0xc, the short "hi"/"lo" variants otherwise).
// FUNCTION: CMR2 0x0041f560
void Race_BuildSelectionPaths(void)
{
    char buf[4];
    char name[4];
    int table[8] = { 6, 3, 1, 4, 0, 2, 5, 7 };

    sprintf(buf, CMain::m_logFileBlankLine);
    if (CGameInfo::FUN_00405d00() == 0) {
        sprintf(buf, g_str0x00519468);
    } else {
        sprintf(buf, g_str0x00519464);
    }
    if (CGameInfo::FUN_00405d80() == 5 || CGameInfo::FUN_00405d80() == 6 ||
        CGameInfo::FUN_00405d80() == 7 || CGameInfo::FUN_00405d80() == 0xb ||
        CGameInfo::FUN_00405d80() == 0xc) {
        int idx = (BYTE)RallyData_FUN_00406940() * 3 + (BYTE)RallyData_FUN_00406950();

        strcpy(name, g_unk0x0051931c[table[idx]]);
        if (idx == 7)
            strcpy(name, g_str0x005193f0);
        sprintf(g_unk0x0053823c, g_str0x005193e4, CInstallInfo::GetTracksDir());
        sprintf(g_unk0x00538340, g_str0x005193d4, g_unk0x0053823c, idx + 1, buf);
        sprintf((char *)g_unk0x00538130, g_str0x005193c8, g_unk0x0053823c, name);
        sprintf(g_unk0x0053874c, g_str0x005193b0, CInstallInfo::GetGameHDPath(),
                g_unk0x0051931c[table[idx]]);
        if (CGameInfo::FUN_00405d00() == 0)
            sprintf(g_unk0x00538444, g_str0x0051939c, CInstallInfo::GetGameHDPath(),
                    g_unk0x0051931c[table[idx]]);
        else
            sprintf(g_unk0x00538444, g_str0x00519388, CInstallInfo::GetGameHDPath(),
                    g_unk0x0051931c[table[idx]]);
        sprintf(g_unk0x00538548, g_str0x00519378, CInstallInfo::GetTracksDir(),
                g_unk0x005192f8[table[idx]], g_unk0x0051931c[table[idx]], buf);
        return;
    }
    sprintf(g_unk0x0053823c, g_strPathFormat, CInstallInfo::GetTracksDir(),
            g_unk0x005192f8[(BYTE)RallyDataCountryIndex()]);
    sprintf(g_unk0x00538340, g_str0x00519450, g_unk0x0053823c,
            g_unk0x0051931c[(BYTE)RallyDataCountryIndex()], (BYTE)RallyDataStageIndex() + 1, buf);
    sprintf((char *)g_unk0x00538130, g_str0x00519444, g_unk0x0053823c,
            g_unk0x0051931c[(BYTE)RallyDataCountryIndex()], (BYTE)RallyDataStageIndex() + 1);
    sprintf(g_unk0x0053874c, g_str0x0051942c, CInstallInfo::GetGameHDPath(),
            g_unk0x0051931c[(BYTE)RallyDataCountryIndex()], (BYTE)RallyDataStageIndex() + 1);
    if (CGameInfo::FUN_00405d00() == 0)
        sprintf(g_unk0x00538444, g_str0x00519418, CInstallInfo::GetGameHDPath(),
                g_unk0x0051931c[(BYTE)RallyDataCountryIndex()], (BYTE)RallyDataStageIndex() + 1);
    else
        sprintf(g_unk0x00538444, g_str0x00519404, CInstallInfo::GetGameHDPath(),
                g_unk0x0051931c[(BYTE)RallyDataCountryIndex()], (BYTE)RallyDataStageIndex() + 1);
    sprintf(g_unk0x00538548, g_str0x005193f4, g_unk0x0053823c,
            g_unk0x0051931c[(BYTE)RallyDataCountryIndex()], buf);
}

// FUNCTION: CMR2 0x0041f8e0
char *FUN_0041f8e0(void)
{
    return g_unk0x0053823c;
}

// FUNCTION: CMR2 0x0041f8f0
char *FUN_0041f8f0(void)
{
    return g_unk0x00538340;
}

// FUNCTION: CMR2 0x0041f910
char *FUN_0041f910(void)
{
    return g_unk0x0053874c;
}

// FUNCTION: CMR2 0x0041f920
char *FUN_0041f920(void)
{
    return g_unk0x00538444;
}

void FUN_00411280(void);

BYTE *FUN_0041f900(void);
int FUN_004b23c0(char *name, int count, GenericFile *pFile, DWORD size);
void FUN_00455470(char);
void Scene_InitLighting(int *pData, int *pHeights);
struct StageLightPreset;
void StageObject_SetLighting(const StageLightPreset *pPrimary, const StageLightPreset *pSecondary);
void FUN_0040fec0(int progress, char drawScene, BYTE alpha);
void FUN_0041fd30(void);
void FUN_00420020(void);
void Race_LoadStageModel(void);
void FUN_0041fc90(void);
void FUN_0041fcd0(void);
void FUN_00490c30(void);
void FUN_00471af0(void);
void StageTiming_LoadCspData(void);
void FUN_00420630(void);
void FUN_00416720(void);
StageFile *StageTiming_GetStageFile3(void);

int FUN_0048caa0(int *pList);
void FUN_004b8270(void);
void FUN_00458090(void);
void FUN_00411450(int keepName);
void FUN_00411b20(void);
void FUN_004d0180(void);
void Replay_InitRaceSlots(void);
void FUN_0045ea70(void);
void FUN_00455300(void);
BYTE *FUN_0047c2f0(void);
void FUN_0048ca60(void);
void FUN_00471dd0(void);
void FUN_004283b0(void);
extern char g_tgaSuffix[];
extern BYTE *g_unk0x00538234;
extern BYTE *g_unk0x00538238;

// GLOBAL: CMR2 0x0051946c
char g_str0x0051946c[] = "%s.xhi";
// GLOBAL: CMR2 0x00519474
char g_str0x00519474[] = "%s.tre";
// GLOBAL: CMR2 0x0051947c
char g_str0x0051947c[] = "%s.tsc";

// Loads the race files of the current selection (the .tsc track, the .tre and
// .xhi lighting files and the .tga of the selection) and initialises the stage.
// FUNCTION: CMR2 0x0041f930
BYTE Race_LoadSelectedStage(void)
{
    BYTE *pTsc;
    BYTE *pTre;
    BYTE *pXhi;

    if (g_raceFileCallbackSet == 0) {
        CGame::RegisterCallback(Race_ReleaseStageFile, 0);
        g_raceFileCallbackSet = 1;
    }
    CGenericFileLoader::FUN_004a9d70((GenericFile *)&g_raceFile, g_unk0x00538548);
    FUN_00411280();
    sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue, FUN_0041f8f0());
    CFrontend::m_stringDest[strlen(CFrontend::m_stringDest) - 4] =
        CFrontend::m_stringDest[strlen(CFrontend::m_stringDest) - 2];
    CFrontend::m_stringDest[strlen(CFrontend::m_stringDest) - 3] =
        CFrontend::m_stringDest[strlen(CFrontend::m_stringDest) - 1];
    CFrontend::m_stringDest[strlen(CFrontend::m_stringDest) - 2] = '\0';
    // 0x41fa0e decrements the post-SCAS pointer to the NUL itself,
    // appending the suffix without deleting the final hi/lo character.
    strcat(CFrontend::m_stringDest, g_tgaSuffix);
    if ((char)RallyData_GetFlag24()) {
        sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue, g_unk0x00538548);
        strcpy(CFrontend::m_stringDest + strlen(CFrontend::m_stringDest) - 5, g_tgaSuffix);
    }
    if (CGameInfo::FUN_00406410(0x10))
        FUN_004b23c0(CFrontend::m_stringDest,
                     (BYTE)RallyDataState() + (BYTE)RallyData_FUN_004069a0(),
                     FUN_0041f500(), 0x80);
    else
        FUN_004b23c0(CFrontend::m_stringDest, 1, FUN_0041f500(), 0x40);
    FUN_0040fec0(0xf, 1, 0xff);
    FUN_0041fd30();
    FUN_00420020();
    Race_LoadStageModel();
    FUN_0041fc90();
    FUN_0040fec0(0x19, 1, 0xff);
    FUN_0041fcd0();
    FUN_00490c30();
    FUN_00471af0();
    FUN_0040fec0(0x28, 1, 0xff);
    StageTiming_LoadCspData();
    FUN_00420630();
    FUN_00416720();
    sprintf(CFrontend::m_stringDest, g_str0x0051947c, FUN_0041f900());
    pTsc = (BYTE *)CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(),
                                                CFrontend::m_stringDest, 0, 0, 0);
    FUN_0048caa0((int *)pTsc);
    FUN_004b8270();
    sprintf(CFrontend::m_stringDest, g_str0x00519474, FUN_0041f900());
    pTre = (BYTE *)CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(),
                                                CFrontend::m_stringDest, 0, 0, 0);
    sprintf(CFrontend::m_stringDest, g_str0x0051946c, FUN_0041f900());
    pXhi = (BYTE *)CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(),
                                                CFrontend::m_stringDest, 0, 0, 0);
    if (pTre != 0 && (char)CGameInfo::FUN_00405ba0() == 0)
        pTre = 0;
    Scene_InitLighting((int *)pTre, (int *)pXhi);
    FUN_00458090();
    FUN_00411450(0);
    FUN_00411b20();
    if (CGameInfo::FUN_00405d80() != 4)
        FUN_004d0180();
    if (CGameInfo::FUN_00406320() == 0 && CGameInfo::FUN_00405d80() == 3)
        Replay_InitRaceSlots();
    FUN_0045ea70();
    FUN_00455300();
    FUN_00455470(1);
    FUN_0048ca60();
    FUN_0040fec0(0x3c, 1, 0xff);
    StageObject_SetLighting((const StageLightPreset *)g_unk0x00538238, (const StageLightPreset *)g_unk0x00538234);
    FUN_00471dd0();
    FUN_004283b0();
    return 1;
}

// FUNCTION: CMR2 0x00420120
int FUN_00420120(void)
{
    return g_unk0x00538970;
}

void FUN_00420130(int value);
int FUN_004b7940(void);
void StageTiming_InitStageFileTable(void);
void FUN_0046b420(void);
int FUN_00456d90(void);
void Sector_RebuildNodeLists(void);
void FUN_00478f50(void);
void FUN_0046b710(void);

// Loads the stage geometry; on success rebuilds the sector node lists and the
// surface tables. Returns whether the stage loaded.
// FUNCTION: CMR2 0x00420150
BYTE Race_LoadStageGeometry(void)
{
    BYTE ok;

    FUN_00420130(FUN_004b7940());
    StageTiming_InitStageFileTable();
    FUN_0046b420();
    if (FUN_00456d90()) {
        Sector_RebuildNodeLists();
        FUN_00478f50();
        ok = 1;
    } else {
        ok = 0;
    }
    FUN_0046b710();
    return ok;
}

// Car count selected by the game mode, before ghost/network additions.
// FUNCTION: CMR2 0x00420190
char Race_GetBaseCarCount(void)
{
    if ((char)RallyData_FUN_00407e70() && !CGameInfo::FUN_00405e00())
        return (char)RallyDataState() + (char)RallyData_FUN_004069a0();
    if ((char)RallyData_FUN_00407e90() && !CGameInfo::FUN_00405e00())
        return 2;
    return (char)RallyDataState();
}

// FUNCTION: CMR2 0x00414700
int FUN_00414700(void)
{
    if (RallyData_FUN_00411880() && !CGameInfo::FUN_00405dc0())
        return 1;
    return 0;
}

// FUNCTION: CMR2 0x004174d0
int FUN_004174d0(void)
{
    return (char)RallyData_GetFlag24() == 0;
}

// GLOBAL: CMR2 0x0053708c
int g_unk0x0053708c[2];
// GLOBAL: CMR2 0x00537198
int g_unk0x00537198[2];

#include "Car.h"
int RallyData_FUN_00421370(BYTE *p);

// Stores the player's route position twice and frees the first five race slots.
// FUNCTION: CMR2 0x00417780
void FUN_00417780(int player)
{
    int i;

    g_unk0x0053708c[player] = RallyData_FUN_00421370((BYTE *)Car_Get(player));
    g_unk0x00537198[player] = RallyData_FUN_00421370((BYTE *)Car_Get(player));
    for (i = 0; i < 5; i++) {
        g_raceSlotState[i].flags &= 0xfc;
        g_raceSlotState[i].pending = -1;
        g_raceSlotState[i].owner = -1;
    }
}

// GLOBAL: CMR2 0x005371a0
int g_unk0x005371a0;
// GLOBAL: CMR2 0x005371a4
int g_unk0x005371a4[10];
struct RaceCallRecord {
    int field_0x0;
    int field_0x4;
    unsigned int flags;         // low 10 bits cleared on reset
};
// GLOBAL: CMR2 0x005371d0
RaceCallRecord g_raceCallRecords[10];
// GLOBAL: CMR2 0x00537248
int g_raceWrongWayFlags[2];

// Queues a race call (radio message) of the given call id and player.
extern int g_unk0x00537358;
int FUN_00418580(unsigned int id, int *pTexture, SpriteRect *pRect, unsigned int *pFlag, BYTE *pColour);
// match 69%: the original keeps `player` in memory and a zero constant in ebx
// (register allocation only; logic matches the asm).
// FUNCTION: CMR2 0x004174e0
void FUN_004174e0(unsigned int player, BYTE callId, BYTE prevCallId, BYTE unused)
{
    unsigned int id;
    unsigned int type;
    unsigned int b1;
    unsigned int b2;
    unsigned int b3;
    int prev;
    int texture;
    BYTE colour[4];
    unsigned int flag;
    SpriteRect rect;
    int i;
    int typeCall;
    int levelCall;

    texture = 0;
    prev = 0;
    if (FUN_00417760(player) != 0)
        return;

    if (callId != 0xff) {
        id = ((unsigned int *)g_unk0x00537358)[callId];
        if (prevCallId != 0xff)
            prev = ((unsigned int *)g_unk0x00537358)[prevCallId];
        type = id >> 0x11 & 0xf;
        b2 = id & 0xf;
        b1 = id >> 4 & 3;
        b3 = id >> 0x15 & 3;
        if ((type == 0xe || type == 2 || type == 1) && b1 != 0)
            typeCall = 1;
        else
            typeCall = 0;
        if (b3 != 0 && b1 != 0)
            levelCall = 1;
        else
            levelCall = 0;
        if (prev == 0) {
            if (typeCall) {
                id -= type * 0x20000;
                prev = type * 0x20000;
            } else if (levelCall) {
                id -= b3 * 0x200000;
                prev = b3 * 0x200000;
            }
        }
    } else {
        id = 0;
        type = player;
        b1 = player;
        b2 = player;
    }

    if (b2 == 6 && b1 == 0 && type == 0)
        return;

    for (i = 0; i < 5; i++) {
        if ((g_raceCallRecords[player * 5 + i].flags & 0x100) == 0) {
            g_raceCallRecords[player * 5 + i].flags = (g_raceCallRecords[player * 5 + i].flags & 0xfffffd32) | 0x132;
            g_raceCallRecords[player * 5 + i].field_0x0 = id;
            if (prev != 0) {
                g_raceCallRecords[player * 5 + i].field_0x4 = prev;
                if (FUN_00418580(prev, &texture, &rect, &flag, colour) != 0)
                    g_raceCallRecords[player * 5 + i].flags = (g_raceCallRecords[player * 5 + i].flags & 0xffffff32) | 0x32;
            } else {
                g_raceCallRecords[player * 5 + i].field_0x4 = 0;
            }
            i = 5;
        }
    }
}

// Resets the per-player race state: best values, call records and slots.
// match 48%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00416670
void FUN_00416670(void)
{
    int i;
    RaceCallRecord *p;
    RaceCallRecord *q;

    g_unk0x00537198[0] = 9999;
    g_raceWrongWayFlags[0] = 0;
    g_unk0x00537198[1] = 9999;
    g_raceWrongWayFlags[1] = 0;
    g_unk0x0053708c[0] = -1;
    g_unk0x0053708c[1] = -1;
    memset(g_unk0x005371a4, 0, sizeof(g_unk0x005371a4));
    p = g_raceCallRecords;
    do {
        q = p;
        i = 5;
        do {
            q->flags &= 0xfffffc00;
            q->field_0x4 = 0;
            q->field_0x0 = 0;
            q++;
        } while (--i);
        g_unk0x005371a0 = 0;
        p = q;
    } while ((int)p < (int)(g_raceCallRecords + 10));
    for (i = 0; i < 5; i++) {
        g_raceSlotState[i].flags &= 0xfc;
        g_raceSlotState[i].pending = -1;
        g_raceSlotState[i].owner = -1;
    }
}

int FUN_004054b0(unsigned int param1);
BYTE *RallyData_FUN_00408a00(BYTE index);
extern int g_unk0x00537350;
extern int g_unk0x0053735c;
void StageUI_DrawStageGrid(int unused, int set);
void FUN_00418000(unsigned int id);
int FUN_00418580(unsigned int id, int *pOut, SpriteRect *pRect, unsigned int *pFlag, BYTE *pColour);
void FUN_00417e70(char *pText, int *pColour, int player, int param4, int param5, int param6);

// Colour of the race call text (white).
// GLOBAL: CMR2 0x00517e24
int g_unk0x00517e24 = -1;

// Draws the five call slots of one player: the icon sprite of each call (the
// countdown wobbles it once the slot is the first one) and the timer text.
// The second parameter is unused (the original still cleans 8 bytes).
// FUNCTION: CMR2 0x004177d0
void FUN_004177d0(unsigned int player, int param2)
{
    BYTE colA[4];
    BYTE colB[4];
    int texA;
    int texB;
    unsigned int flagA;
    unsigned int flagB;
    int yOff;
    int curveB;
    int xOff;
    int second;
    int iVar;
    int base;
    int yText;
    int extra;
    int value;
    int found;
    int bVar2;
    int n;
    int r;
    int half;
    SpriteRect rectTex;
    SpriteRect rectIcon;
    SpriteRect rectSecond;
    SpriteRect rectMain;
    RaceCallRecord *rec;
    int *pCallFlag;
    BYTE *pState;
    short curve[11];

    extra = 0;
    colB[0] = 0xff; colB[1] = 0xff; colB[2] = 0xff; colB[3] = 0xff;
    colA[0] = 0xff; colA[1] = 0xff; colA[2] = 0xff; colA[3] = 0xff;
    texB = 0;
    flagA = 0;
    flagB = 0;
    second = 0;
    xOff = 0;
    yOff = 0;
    curve[0] = 0;
    curve[1] = 0xc;
    curve[2] = 0x19;
    curve[3] = 0x4c;
    curve[4] = 0x99;
    curve[5] = 0x100;
    curve[6] = 0x180;
    curve[7] = 0x219;
    curve[8] = 0x2cc;
    curve[9] = 0x399;
    curve[10] = 0x400;

    if ((char)RallyData_FUN_00407e70() && g_unk0x00537350 != -1) {
        if (RallyData_FUN_00411880() != 0) {
            if (player == 1)
                StageUI_DrawStageGrid(1, g_unk0x00537350);
        } else {
            StageUI_DrawStageGrid(player, g_unk0x00537350);
        }
    }

    if (RallyData_FUN_00411880() == 0) {
        iVar = 0;
    } else {
        iVar = 1;
        if (CGameInfo::FUN_00405dc0()) {
            if (player == 1)
                yOff = (int)g_pGraphics->resY / 2;
            yOff += (int)g_pGraphics->resY / 8 + (int)g_pGraphics->resY * 10 / 0x1e0;
            xOff = (int)g_pGraphics->resX * 46 / 0x280 - ((int)g_pGraphics->resX * 29 << 10 >> 16);
        } else {
            if (player == 0)
                xOff = -((int)g_pGraphics->resX / 4);
            else
                xOff = (int)g_pGraphics->resX / 4;
            yOff = (int)g_pGraphics->resY * 9011 >> 16;
        }
    }

    if (iVar == 0) {
        pState = RallyData_FUN_00408a00(FUN_0041b370() + player);
        if ((*pState & 3) == 1)
            extra = 0x10;
    }
    yText = ((int)g_pGraphics->resY << 12 >> 16) + extra;

    pCallFlag = g_raceWrongWayFlags;
    if (pCallFlag[player] != 0 || FUN_004054b0(player) != 0) {
        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x57));
        FUN_00417e70(CFrontend::m_stringDest, &g_unk0x00517e24, player, 1, -1, -1);
    }

    if (!FUN_004174d0())
        return;

    iVar = 0;
    base = player * 5;
    do {
        rec = &g_raceCallRecords[base + iVar];
        if ((rec->flags & 0x100) == 0) {
            if (FUN_004054b0(player) == 0)
                goto next;
            if ((char)CGameInfo::FUN_00404f20() == 0)
                goto next;
        }

        found = FUN_00418580((rec->flags & 0x100) ? rec->field_0x0 : 0x14, &texA, &rectIcon, &flagA, colB);
        bVar2 = 0;
        if (rec->field_0x4 != 0) {
            if (FUN_00418580(rec->field_0x4, &texB, &rectTex, &flagB, colA) != 0) {
                bVar2 = 1;
                second = 1;
            }
        }

        if ((rec->flags & 0x200) == 0) {
            if (RallyData_FUN_00411880() == 0) {
                if (rec->field_0x0 != 0)
                    FUN_00418000(rec->field_0x0);
                if (rec->field_0x4 != 0)
                    FUN_00418000(rec->field_0x4);
            }
            rec->flags |= 0x200;
        }

        if (found == 0 && second == 0)
            return;

        rectMain.x = rectIcon.x;
        rectMain.y = (short)yText;
        rectMain.w = rectIcon.w;
        rectMain.h = rectIcon.h;
        rectSecond.y = (short)yText;
        rectSecond.w = rectIcon.w;
        rectSecond.h = rectIcon.h;

        if (iVar != 0)
            goto next;

        pState = RallyData_FUN_00408a00(FUN_0041b370() + player);
        if ((*pState & 3) == 2)
            goto next;

        if (bVar2) {
            n = 0x32 - (rec->flags & 0xff);
            if (n < 0xa)
                value = -curve[10 - n];
            else if (n < 0x14)
                value = 0;
            else if (n < 0x1e)
                value = -curve[n - 0x14];
            else
                value = 0x400;
            if (n < 0x14)
                curveB = 0x400;
            else if (n < 0x1e)
                curveB = -curve[0x1e - n];
            else if (n < 0x28)
                curveB = 0;
            else if (n < 0x32)
                curveB = -curve[n - 0x28];
            else
                curveB = 0x400;
        } else {
            n = 0x32 - (rec->flags & 0xff);
            if (n >= 0x28)
                value = -curve[n - 0x28];
            else if (n > 0xa)
                value = 0;
            else
                value = curve[0xa - n];
        }

        if ((rec->flags & 0x100) == 0)
            value = 0;

        rectSecond.y = (short)(rectSecond.y + (short)yOff);
        rectMain.y = (short)(rectMain.y + (short)yOff);

        if (value != 0) {
            r = FixMul((int)(__int64)(((int)rectMain.w / 2) * 65536.0), g_sinTable[(value + 0x400) & 0xfff]);
            rectMain.w = (short)(r * 2 >> 16);
        }
        if (found == 0)
            rectMain.w = 0;

        if (bVar2) {
            r = FixMul((int)(__int64)(((int)rectSecond.w / 2) * 65536.0), g_sinTable[(curveB + 0x400) & 0xfff]);
            r = r * 2 >> 16;
            rectSecond.w = (short)r;
            rectMain.x = (short)((((int)g_pGraphics->resX << 15) >> 16) - (r + (int)rectMain.w) / 2 + xOff);
            if (found != 0) {
                if (flagA == 0) {
                    Sprite_Queue(&rectIcon, &rectMain, (Texture *)texA, 2, 0, NULL, NULL, colB, 8);
                } else if (rectMain.w != 0) {
                    Sprite_Queue(&rectIcon, &rectMain, (Texture *)texA, 2, 0, NULL, NULL, colB, 1);
                }
            }
        } else {
            rectSecond.w = 0;
            rectMain.x = (short)((((int)g_pGraphics->resX << 15) >> 16) - (short)value / 2 + xOff);
            if (found != 0) {
                if (flagA != 0)
                    Sprite_Queue(&rectIcon, &rectMain, (Texture *)texA, 2, 0, NULL, NULL, colB, 1);
                else
                    Sprite_Queue(&rectIcon, &rectMain, (Texture *)texA, 2, 0, NULL, NULL, colB, 8);
            }
        }

        rectSecond.x = (short)(rectMain.x + rectMain.w);
        if (second != 0 && rectSecond.w != 0) {
            if (flagB == 0)
                Sprite_Queue(&rectTex, &rectSecond, (Texture *)texB, 2, 0, NULL, NULL, colA, 8);
            else
                Sprite_Queue(&rectTex, &rectSecond, (Texture *)texB, 2, 0, NULL, NULL, colA, 1);
        }

next:
        iVar++;
    } while (iVar < 5);
}

// FUNCTION: CMR2 0x00417e60
void FUN_00417e60(void)
{
    g_unk0x00537190 = 0;
}

BYTE *FUN_00464b10(int view);
int Font_GetTextWidth(BYTE index, BYTE *text);
int Font_GetTextHeight(BYTE index, char *text);
void Font_DrawText(BYTE index, char *text, short x, short y, int *pColour, unsigned int flags);

// Shadow colour of the race call text (black).
// GLOBAL: CMR2 0x00517e28
int g_unk0x00517e28 = 0xff000000;

// Draws one race message text, centred on the given player's viewport, with an
// optional dark offset copy underneath. Only one message per frame.
// FUNCTION: CMR2 0x00417e70
void FUN_00417e70(char *pText, int *pColour, int player, int shadow, int x, int y)
{
    short *pViewRect;
    int textY;
    int pos;
    int race;
    unsigned int flags;
    BYTE *pState;

    if (g_unk0x00537190 != 0)
        return;

    race = 1;
    g_unk0x00537190 = 1;
    pos = 0;
    pViewRect = (short *)FUN_00464b10(player);
    if (RallyData_FUN_00411880() != 0) {
        if (CGameInfo::FUN_00405dc0()) {
            if (player == 1)
                pos = (int)g_pGraphics->resY / 2;
            pos -= (int)g_pGraphics->resY * 9011 >> 16;
        } else if (player == 0) {
            // The original duplicates the tail under a `player == 0` test
            // (MSVC6 leaves the dead test in the else branch).
            pos = (int)g_pGraphics->resY * 9011 >> 16;
        } else {
            pos = (int)g_pGraphics->resY * 9011 >> 16;
        }
    } else {
        race = 0;
        pos = 0;
    }
    textY = pos + ((int)g_pGraphics->resY << 14 >> 16);
    if (race == 0) {
        pState = RallyData_FUN_00408a00(FUN_0041b370() + player);
        if ((*pState & 3) == 1)
            textY = ((int)g_pGraphics->resY << 14 >> 16) + pos + 0x10;
    }
    Font_GetTextWidth(2, (BYTE *)pText);
    Font_GetTextHeight(2, pText);

    pos = (int)pViewRect[2] / 2 + (int)pViewRect[0];
    if (x != -1) {
        pos = x;
        textY = y;
        flags = 9;
    } else {
        flags = 0x12;
    }
    if (shadow != 0)
        Font_DrawText(2, pText, pos + 1, (int)g_pGraphics->resY / 0x60 + 1 + textY, &g_unk0x00517e28, flags);
    Font_DrawText(2, pText, pos, (int)g_pGraphics->resY / 0x60 + textY, pColour, flags);
}

// Assigns the race slots announced by a call record: every bit field of the
// id selects one slot of the race slot table.
// FUNCTION: CMR2 0x00418000
void FUN_00418000(unsigned int id)
{
    unsigned int f4;
    unsigned int f17;
    unsigned int f5;
    unsigned int f6;
    unsigned int f7;
    unsigned int f3;
    unsigned int f11;
    unsigned int f9;
    unsigned int f12;
    unsigned int f10;
    unsigned int f8;
    unsigned int f1;
    int slot;

    f3 = id >> 4 & 3;
    f4 = id >> 0xc & 7;
    f17 = id >> 0x17 & 1;
    f5 = id >> 0xf & 3;
    f6 = id >> 0x11 & 0xf;
    f7 = id >> 0x1c & 3;
    f11 = id >> 6 & 0xf;
    f9 = id >> 10 & 3;
    f12 = id >> 0x15 & 3;
    f10 = id & 0xf;
    f8 = id >> 0x18 & 1;
    f1 = id >> 0x19 & 7;

    if (f10 == 6) {
        if (f3 == 0 && f11 == 0 && f9 == 0 && f4 == 0 && f6 == 0 && f12 == 0 && f5 == 0 &&
            f7 == 0 && f8 == 0 && f1 == 0)
            return;
    } else if (f10 == 9) {
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x37);
    }

    switch (f11) {
    case 1:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0xe);
        break;
    case 2:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0xf);
        break;
    case 3:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x10);
        break;
    case 4:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x11);
        break;
    case 5:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x12);
        break;
    case 6:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x13);
        break;
    case 7:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x14);
        break;
    case 8:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x15);
        break;
    case 9:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x16);
        break;
    case 10:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x16);
        break;
    case 11:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x16);
        break;
    case 12:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x16);
        break;
    case 13:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x16);
        break;
    case 14:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x16);
        break;
    }


    if (f12 == 1)
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x1a);
    else if (f12 == 2)
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x1b);
    if (f9 == 1)
        Race_AssignUnusedSlot(g_unk0x0053735c + 10);
    else if (f9 == 2)
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x1c);
    if (f17 == 1)
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x1d);

    switch (f10) {
    case 1:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x5);
        break;
    case 2:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x4);
        break;
    case 3:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x3);
        break;
    case 4:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x2);
        break;
    case 5:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x1);
        break;
    case 6:
        Race_AssignUnusedSlot(g_unk0x0053735c);
        break;
    case 7:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x6);
        break;
    case 8:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x7);
        break;
    }


    if (f3 == 1) {
        if (CGameInfo::FUN_004063f0(2) == 0)
            slot = g_unk0x0053735c + 8;
        else
            slot = g_unk0x0053735c + 9;
        Race_AssignUnusedSlot(slot);
    } else if (f3 == 2) {
        if (!(CGameInfo::FUN_004063f0(2) != 0))
            slot = g_unk0x0053735c + 9;
        else
            slot = g_unk0x0053735c + 8;
        Race_AssignUnusedSlot(slot);
    }

    if (f4 == 1)
        Race_AssignUnusedSlot(g_unk0x0053735c + 0xc);
    else if (f4 == 2)
        Race_AssignUnusedSlot(g_unk0x0053735c + 0xb);
    else if (f4 == 3)
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x1e);
    if (f5 == 1)
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x17);
    else if (f5 == 2)
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x1f);

    switch (f6) {
    case 1:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x19);
        break;
    case 2:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x18);
        break;
    case 3:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x22);
        break;
    case 4:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x23);
        break;
    case 5:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x24);
        break;
    case 6:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x21);
        break;
    case 7:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x25);
        break;
    case 8:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x26);
        break;
    case 9:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x27);
        break;
    case 10:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x28);
        break;
    case 11:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x29);
        break;
    case 12:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x2a);
        break;
    case 13:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x2b);
        break;
    case 14:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x36);
        break;
    case 15:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x20);
        break;
    }


    if (f7 == 1)
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x2c);
    else if (f7 == 2)
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x2d);
    if (f8 == 1)
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x2e);

    switch (f1) {
    case 1:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x2f);
        return;
    case 2:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x30);
        return;
    case 3:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x31);
        return;
    case 4:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x32);
        return;
    case 5:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x33);
        return;
    case 6:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x34);
        return;
    case 7:
        Race_AssignUnusedSlot(g_unk0x0053735c + 0x35);
    }
}

int FUN_004781c0(int index);
int FUN_004b7790(unsigned short id, int volume, int frequency, int loopStart, int loops, int is3D);
void Sound_Free(unsigned int handle);
int Sound_GetVolume(unsigned int handle);
int FUN_00418e70(int car);
int FUN_00427d50(unsigned int view, int listener);
void FUN_004b79a0(unsigned int handle, int volume);
BYTE FUN_00460bf0(int index);
int FUN_00460c10(int index);
void Sound_SetPan(unsigned int handle, unsigned short pan);
extern int g_unk0x005374c0;
extern int g_carSlotVolumes[8][4];
extern int g_carMaxVolume[8];
extern int g_unk0x00537564;

// GLOBAL: CMR2 0x00537358
int g_unk0x00537358;

// Registered callback of 0x416720.
// FUNCTION: CMR2 0x00418550
int FUN_00418550(void)
{
    g_unk0x00537358 = 0;
    return 1;
}

// FUNCTION: CMR2 0x00418560
void FUN_00418560(int value)
{
    g_unk0x00537194 = value;
}

// FUNCTION: CMR2 0x00418570
int FUN_00418570(void)
{
    return g_unk0x00537194;
}

extern SpriteRect *g_pArrowRects;
extern Texture *g_arrowTexture;

// Decodes a race call slot id into the arrow sprite rect of the call icon, the
// secondary-icon flag and the colour the icon is tinted with.
// FUNCTION: CMR2 0x00418580
int FUN_00418580(unsigned int id, int *pTexture, SpriteRect *pRect, unsigned int *pFlag, BYTE *pColour)
{
    unsigned int type;
    unsigned int b1;
    unsigned int b2;
    unsigned int b3;
    SpriteRect *pSrc;
    int result;
    int afterFirst;

    type = id & 0xf;
    b3 = id >> 0x15 & 3;
    b2 = id >> 0x11 & 0xf;
    b1 = id >> 4 & 3;

    if (type == 0 || type == 9)
        afterFirst = 0;
    else
        afterFirst = 1;
    if (b1 != 0 || afterFirst || b2 == 0xe || b2 == 2 || b2 == 1 || b3 != 0)
        result = 1;
    else
        result = 0;

    pColour[0] = 0xff;
    pColour[1] = 0xff;
    pColour[2] = 0xff;
    pColour[3] = 0xff;
    if (b1 == 1)
        *pFlag = 1;
    else
        *pFlag = 0;
    if (CGameInfo::FUN_004063f0(2) != 0)
        *pFlag ^= 1;

    switch (type) {
    case 2:
        pSrc = &g_pArrowRects[5];
        break;
    case 3:
        pSrc = &g_pArrowRects[4];
        break;
    case 4:
        pSrc = &g_pArrowRects[3];
        break;
    case 5:
    case 6:
        pSrc = &g_pArrowRects[2];
        break;
    case 7:
        pSrc = &g_pArrowRects[1];
        break;
    case 8:
        pSrc = &g_pArrowRects[0];
        break;
    default:
        pSrc = &g_pArrowRects[6];
        break;
    }
    *pRect = *pSrc;

    if (b2 != 0 && b1 == 0) {
        *pFlag = 0;
        *pRect = g_pArrowRects[7];
        if (b2 == 2 || b2 == 0xe) {
            pColour[0] = 0xff;
            pColour[1] = 0xff;
            pColour[2] = 0xd;
        }
        if (b2 == 1) {
            pColour[0] = 0xff;
            pColour[1] = 0;
            pColour[2] = 0;
        }
    }

    if (b3 != 0) {
        *pFlag = 0;
        *pRect = g_pArrowRects[7];
        pColour[0] = 0xff;
        if (b3 == 1) {
            pColour[1] = 0xff;
            pColour[2] = 0xd;
        } else {
            pColour[1] = 0;
            pColour[2] = 0;
        }
    }

    *pTexture = (int)g_arrowTexture;
    return result;
}

// FUNCTION: CMR2 0x00418d20
void FUN_00418d20(int value)
{
    g_unk0x00537394 = value;
}

BYTE g_raceBlock[0x864];

// Starts the sound of one entry of the stage table and stores its handle, the
// random pitch and the id of the sound.
// match 54%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00418d30
void CarSound_PlaySlot(int param1, int param2, int param3, int param4, int param5)
{
    int index;

    index = param3;
    if (param5 != 0)
        g_carSoundSets[param1].handle[index] = FUN_004b7790(param2, param4, 0x5622, param5, 1, 0);
    else
        g_carSoundSets[param1].handle[index] = FUN_004b7790(param2, param4, 0x5622, 0, 1, 0);
    g_carSoundSets[param1].pitch[index] = rand() % 0x19 + 0x32 + FUN_004781c0(param1);
    g_carSoundSets[param1].surface[param3] = g_unk0x005375f4[param1];
    g_carSoundSets[param1].id[index] = param2;
}

// Stops the sound of one entry of the stage table (and forgets both the handle
// and the id).
// FUNCTION: CMR2 0x00418dd0
void CarSound_StopSlot(int param1, int param2, char param3)
{
    if (param3 != 0)
        g_carSoundSets[param1].id[param2] = -1;
    Sound_Free(g_carSoundSets[param1].handle[param2]);
    g_carSoundSets[param1].handle[param2] = -1;
}

// Returns a random value in [0, param2) that is not param1.
// FUNCTION: CMR2 0x00419b50
int CarSound_PickDifferentSampleIndex(int param1, int param2)
{
    int value;

    if (param2 == 1)
        return 0;
    value = rand() % param2;
    while (value == param1) {
        value = rand() % param2;
    }
    return value;
}

void CarSound_PlaySlot(int param1, int param2, int param3, int param4, int param5);

void CarSound_StopSlot(int param1, int param2, char param3);
void FUN_00418e20(int set, int dst, int src);
BYTE FUN_00427aa0(void);

// Moves the car's current sounds to the second bank (slots 4/5 to 6/7) on a
// sound state change, stopping the ones that don't carry over.
// match 50%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00419b90
void FUN_00419b90(int car, BYTE *pInfo)
{
    switch (*(int *)(pInfo + 0xa8)) {
    case 6:
    case 0x15:
        if (!FUN_00427aa0())
            CarSound_StopSlot(car, 4, 1);
        FUN_00418e20(car, 4, 5);
        break;
    case 7:
        if (!FUN_00427aa0())
            CarSound_StopSlot(car, 4, 1);
        FUN_00418e20(car, 4, 5);
        FUN_00418e20(car, 6, 7);
        break;
    case 0xb:
        if (!FUN_00427aa0()) {
            CarSound_StopSlot(car, 4, 1);
            CarSound_StopSlot(car, 6, 1);
        }
        FUN_00418e20(car, 4, 5);
        break;
    case 0xc:
        if (!FUN_00427aa0()) {
            CarSound_StopSlot(car, 4, 1);
            CarSound_StopSlot(car, 6, 1);
        }
        FUN_00418e20(car, 4, 5);
        FUN_00418e20(car, 6, 7);
        break;
    case 2:
    case 0x11:
    case 0x16:
        FUN_00418e20(car, 4, 5);
        FUN_00418e20(car, 6, 7);
        break;
    case 1:
    case 4:
    case 9:
    case 0xe:
    case 0x10:
    case 0x13:
    case 0x18:
        FUN_00418e20(car, 4, 5);
        break;
    default:
        goto tail;
    }
tail:
    *(int *)(g_raceBlock + 0x94 + car * 4) = *(int *)(g_raceBlock + 0x48 + car * 4);
    *(int *)(g_raceBlock + car * 4) = *(int *)(g_raceBlock + 0x220 + car * 4);
    *(int *)(g_raceBlock + 0x48 + car * 4) = 0;
    *(int *)(g_raceBlock + 0x220 + car * 4) = 0;
}

// Restarts the stage sound of the slots whose surface changed while their
// sound is still playing; the new sound reuses the slot's id and volume.
// The per-car sound ids are addressed through the flat block index (car*45+i),
// as in the original.
#define g_carSoundIds ((int *)(g_raceBlock + 0x284))   // 0x5377ec
// FUNCTION: CMR2 0x00419cd0
void FUN_00419cd0(int car, int unused)
{
    RaceCarSoundState *pState = &g_carSoundStates[car];
    StageSoundPattern *pPattern;
    int i;
    int old;

    if (pState->state == -1)
        return;
    pPattern = &g_stageSoundPatterns[g_stageSoundPatterns[pState->state].redirect];
    for (i = 0; i < 10; i++) {
        if (i == 4 && pState->state == 0x19)
            continue;
        if (pState->handle[i] == -1)
            continue;
        if (Sound_IsPlaying(pState->handle[i]) == 0)
            continue;
        if ((unsigned int)FUN_004781c0(car) <= (unsigned int)pState->pitch[i])
            continue;
        switch (i) {
        case 4:
            if (pPattern->choices[g_unk0x005375f4[car]] > 1 && pState->pattern == 0x19) {
                old = Sound_GetVolume(pState->handle[i]);
                CarSound_StopSlot(car, i, 0);
                CarSound_PlaySlot(car, CarSound_PickDifferentSampleIndex(g_carSoundIds[car * 45 + i],
                                               pPattern->choices[g_unk0x005375f4[car]]) +
                                  pPattern->base[g_unk0x005375f4[car]], i, old, 0);
            }
            break;
        case 5:
            if (pPattern->choices[g_unk0x005375f4[car]] > 1) {
                old = Sound_GetVolume(pState->handle[i]);
                CarSound_StopSlot(car, i, 0);
                CarSound_PlaySlot(car, CarSound_PickDifferentSampleIndex(g_carSoundIds[car * 45 + i],
                                               pPattern->choices[g_unk0x005375f4[car]]) +
                                  pPattern->base[g_unk0x005375f4[car]], i, old, 0);
            }
            break;
        case 7:
            if (pPattern->choices[g_unk0x005375f4[car] + 2] > 1) {
                old = Sound_GetVolume(pState->handle[i]);
                CarSound_StopSlot(car, i, 0);
                CarSound_PlaySlot(car, CarSound_PickDifferentSampleIndex(g_carSoundIds[car * 45 + i],
                                               pPattern->choices[g_unk0x005375f4[car] + 2]) +
                                  pPattern->base[g_unk0x005375f4[car] + 2], i, old, 0);
            }
            break;
        case 6:
            if (pPattern->choices[g_unk0x005375f4[car] + 2] > 1 && pState->pattern == 0x19) {
                old = Sound_GetVolume(pState->handle[i]);
                CarSound_StopSlot(car, i, 0);
                CarSound_PlaySlot(car, CarSound_PickDifferentSampleIndex(g_carSoundIds[car * 45 + i],
                                               pPattern->choices[g_unk0x005375f4[car] + 2]) +
                                  pPattern->base[g_unk0x005375f4[car] + 2], i, old, 0);
            }
            break;
        }
    }
}

// Restarts the sound of the slots whose surface changed: a slot of the first
// bank (4/5) uses the choices of the current state, one of the second bank
// (6/7) the secondary choices, and the sound is re-rolled from the pattern.
// FUNCTION: CMR2 0x00419ed0
void FUN_00419ed0(int car, int unused)
{
    RaceCarSoundState *pState = &g_carSoundStates[car];
    StageSoundPattern *pPattern;
    int i;
    int old;

    if (pState->state == -1)
        return;
    pPattern = &g_stageSoundPatterns[g_stageSoundPatterns[pState->state].redirect];
    for (i = 0; i < 10; i++) {
        if (pState->handle[i] == -1)
            continue;
        if (Sound_IsPlaying(pState->handle[i]) == 0)
            continue;
        if (pState->surface[i] == g_unk0x005375f4[car])
            continue;
        switch (i) {
        case 4:
            if (pState->pattern == 0x19 && pState->state != 0x19) {
                old = Sound_GetVolume(pState->handle[i]);
                CarSound_StopSlot(car, i, 1);
                CarSound_PlaySlot(car, CarSound_PickDifferentSampleIndex(-1, pPattern->choices[g_unk0x005375f4[car]]) +
                                  pPattern->base[g_unk0x005375f4[car]], i, old, 0);
            }
            break;
        case 5:
            old = Sound_GetVolume(pState->handle[i]);
            CarSound_StopSlot(car, i, 1);
            CarSound_PlaySlot(car, CarSound_PickDifferentSampleIndex(-1, pPattern->choices[g_unk0x005375f4[car]]) +
                              pPattern->base[g_unk0x005375f4[car]], i, old, 0);
            break;
        case 6:
            if (pState->pattern == 0x19) {
                old = Sound_GetVolume(pState->handle[i]);
                CarSound_StopSlot(car, i, 1);
                CarSound_PlaySlot(car, CarSound_PickDifferentSampleIndex(-1, pPattern->choices[g_unk0x005375f4[car] + 2]) +
                                  pPattern->base[g_unk0x005375f4[car] + 2], i, old, 0);
            }
            break;
        case 7:
            old = Sound_GetVolume(pState->handle[i]);
            CarSound_StopSlot(car, i, 1);
            CarSound_PlaySlot(car, CarSound_PickDifferentSampleIndex(-1, pPattern->choices[g_unk0x005375f4[car] + 2]) +
                              pPattern->base[g_unk0x005375f4[car] + 2], i, old, 0);
            break;
        }
    }
}

// Keeps the car's stage sound independent engine sample in step with the
// speed of the car: slot 8 is used on the tarmac, slot 9 on the other
// surfaces, and the other one is stopped when the surface changes.
// FUNCTION: CMR2 0x0041a0a0
void FUN_0041a0a0(int car, int param2)
{
    BYTE *pRow = g_raceBlock + 0x240 + car * 0xb4;
    int volume;
    int engSpeed;
    int speedVol;

    speedVol = FUN_00418e70(car);
    engSpeed = FixMulShift32(Car_Get(car)->speed, 0x431168);
    if (FUN_00460bf0(car) == 1)
        volume = FixMul(FUN_00460c10(car), 0x13333);
    else
        volume = 0;
    if (g_unk0x005375f4[car] == 0) {
        if (volume > 0x10000)
            volume = 0x10000;
        if (Sound_IsPlaying(*(int *)(pRow + 0x40)))
            CarSound_StopSlot(car, 9, 1);
        if (engSpeed < 0)
            speedVol = 0;
        else if (engSpeed > 0x64)
            speedVol = 0x8000;
        else
            speedVol = 0x10000 - FixMul(0x8000,
                            FixDiv((int)(__int64)((double)engSpeed * CGraphics::m_65536), 0x640000));
        volume = FixMul(volume, speedVol);
        if (engSpeed < 0)
            engSpeed = 0;
        else if (engSpeed > 0x64)
            engSpeed = 11000;
        else
            engSpeed = engSpeed * 11000 / 100;
        engSpeed += 0x5622;
        if (!Sound_IsPlaying(*(int *)(pRow + 0x3c)))
            CarSound_PlaySlot(car, g_unk0x00537564, 8,
                         FixMul(FUN_00427d50(car, param2), FixMul(g_unk0x00537664, volume)), 0);
        FUN_004b79a0(*(int *)(pRow + 0x3c),
                     FixMul(FUN_00427d50(car, param2), FixMul(g_unk0x00537664, volume)));
        Sound_SetPan(*(int *)(pRow + 0x3c), engSpeed);
    } else {
        if (volume > 0x10000)
            volume = 0x10000;
        if (Sound_IsPlaying(*(int *)(pRow + 0x3c)))
            CarSound_StopSlot(car, 8, 1);
        if (!Sound_IsPlaying(*(int *)(pRow + 0x40)))
            CarSound_PlaySlot(car, g_unk0x00537dc8, 9,
                         FixMul(FUN_00427d50(car, param2), FixMul(g_unk0x00537664, volume)), 0);
        volume = FixMul(volume, speedVol);
        FUN_004b79a0(*(int *)(pRow + 0x40),
                     FixMul(FUN_00427d50(car, param2), FixMul(g_unk0x00537664, volume)));
    }
}

// Picks the stage sound state of a car from the surfaces under its wheels and
// the sounds already playing, and applies it when it changed.
// match 51%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0041a340
void FUN_0041a340(int car, int unused)
{
    int offset = car * 0xb4;
    RaceCarSoundState *pState = (RaceCarSoundState *)(g_raceBlock + 0x240 + offset);
    Car *pCar;
    short value;
    short v;
    short *pSrc;
    int *pFlags;
    short *pSlot;
    int *pTime;
    int counts[4];
    char allTwo;
    int bestIndex;
    int bestCount;
    int chosen;
    int minSlack;
    int newCount;
    int i;
    int j;

    bestIndex = 0;
    chosen = 0;
    minSlack = 0xffffffff;
    pCar = Car_Get(car);
    pSrc = pCar->wheelSurface;
    pFlags = pCar->cornerOnGround;
    pSlot = pState->slotState;
    pTime = pState->slotTime;
    for (i = 4; i != 0; i--) {
        value = *pSrc;
        if (*pFlags == 0 || value > 0x1e || value < 0 || value == 0x10)
            value = -1;
        if (*pSlot != value)
            *pTime = FUN_004781c0(car);
        *pSlot = value;
        pSrc++;
        pFlags++;
        pSlot++;
        pTime++;
    }
    bestCount = 0;
    allTwo = 1;
    for (i = 0; i < 4; i++) {
        v = pState->slotState[i];
        counts[i] = 0;
        for (j = 0; j < 4; j++) {
            if (v == pState->slotState[j])
                counts[i]++;
        }
        if (v == -1 && counts[i] != 4)
            counts[i] = 0;
        if (counts[i] > bestCount) {
            bestIndex = i;
            bestCount = counts[i];
        }
        if (counts[i] != 2)
            allTwo = 0;
    }
    for (i = 0; i < 4; i++) {
        if (counts[i] >= 3)
            break;
        if (counts[i] == 2 && !allTwo)
            break;
    }
    if (i < 4) {
        chosen = pState->slotState[bestIndex];
    } else {
        for (i = 0; i < 4; i++) {
            // the original reads the clock again for the stored value
            if ((unsigned int)(FUN_004781c0(car) - pState->slotTime[i]) < (unsigned int)minSlack) {
                chosen = pState->slotState[i];
                minSlack = FUN_004781c0(car) - pState->slotTime[i];
            }
        }
    }
    if (chosen == -1)
        newCount = 0;
    else
        newCount = g_stageSoundPatterns[g_stageSoundPatterns[chosen].redirect].count;
    if (pState->countOld != newCount || pState->stateOld != chosen) {
        if (pState->pattern != 0x19)
            FUN_00419b90(car, (BYTE *)pState);
        pState->state = (short)chosen;
        pState->count = newCount;
        pState->time = FUN_004781c0(car);
        pState->pattern = *(int *)(g_raceBlock + 0x2e0 + offset) + pState->countOld * 5;
        StageUI_ApplySoundState(car, (BYTE *)pState);
        pState->countOld = pState->count;
        pState->stateOld = pState->state;
    }
    if (pState->pattern != 0x19 && (unsigned int)(FUN_004781c0(car) - pState->time) > 0x32) {
        FUN_00419b90(car, (BYTE *)pState);
        pState->pattern = 0x19;
    }
}

// Switches car's engine sound between its two samples of stage sound group 25
// as the rolling direction speed (0x79c) changes sign.
// match 54%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0041ae80
void FUN_0041ae80(int car, int unused)
{
    BYTE *pSet = g_raceBlock + 0x240 + car * 0xb4;
    int *pHandle = (int *)(pSet + 0x2c);

    if (*(short *)(pSet + 0x18) == 0x19) {
        if (Car_Get(car)->steerFollowRate > 0) {
            if (pSet[0xb0] == 0) {
                if (Sound_IsPlaying(*pHandle)) {
                    Sound_Free(*pHandle);
                    *pHandle = -1;
                }
                CarSound_PlaySlot(car, g_stageSoundPatterns[25].base[g_unk0x005375f4[car]], 4, 0, 0);
                pSet[0xb0] = 1;
            }
        } else if (pSet[0xb0] != 0) {
            if (Sound_IsPlaying(*pHandle)) {
                Sound_Free(*pHandle);
                *pHandle = -1;
            }
            CarSound_PlaySlot(car, g_stageSoundPatterns[25].base[g_unk0x005375f4[car]] + 1, 4, 0, 0x3542);
            pSet[0xb0] = 0;
        }
    }
}

// Marks the sound groups used by the stage's surfaces.
// FUNCTION: CMR2 0x0041afe0
void FUN_0041afe0(BYTE *pSurfaces, unsigned int count)
{
    unsigned int i;

    memset(g_stageSoundUsed, 0, 0x1f);
    g_stageSoundCount = count;
    for (i = 0; i < g_stageSoundCount; i++)
        g_stageSoundUsed[g_stageSoundPatterns[FUN_00478a10(pSurfaces[i])].redirect] = 1;
}

// FUNCTION: CMR2 0x0041b040
void FUN_0041b040(int value)
{
    g_unk0x00537664 = FixMul(value, 0x10000);
}

// FUNCTION: CMR2 0x0041bf50
int FUN_0041bf50(int index)
{
    return g_unk0x00537f68[index];
}

// FUNCTION: CMR2 0x0041bf60
int FUN_0041bf60(int index)
{
    return g_unk0x00537f78[index];
}

// FUNCTION: CMR2 0x0041bf70
int FUN_0041bf70(int index)
{
    return g_unk0x00537f0c[index];
}

// FUNCTION: CMR2 0x0041d290
int FUN_0041d290(void)
{
    return g_unk0x00537f24 << 2;
}

// FUNCTION: CMR2 0x0041d2a0
int FUN_0041d2a0(void)
{
    return g_unk0x00537f24;
}

// FUNCTION: CMR2 0x0041d780
unsigned int FUN_0041d780(void)
{
    unsigned int value = g_unk0x00537fc0;
    if (value > 499)
        value = 499;
    return value;
}

// FUNCTION: CMR2 0x0041db00
BYTE FUN_0041db00(void)
{
    return g_unk0x0053811c;
}

// Callback count to unwind to when the race ends.
// GLOBAL: CMR2 0x0053896c
int g_raceCallbackMark;

void Sound_FreeAll(void);

// FUNCTION: CMR2 0x00420100
void FUN_00420100(void)
{
    Sound_FreeAll();
    CGame::UnwindCallbacks(g_raceCallbackMark);
}

// GLOBAL: CMR2 0x00538868
char g_raceCarPath[0x104];
// GLOBAL: CMR2 0x0051945c
char g_strPathFormat[] = "%s\\%s";

// GLOBAL: CMR2 0x005196f0
char g_strCarC1Format[] = "%s\\%sc1";
// GLOBAL: CMR2 0x005196f8
char g_strCarD3Format[] = "%s\\%sd3%d";

// GLOBAL: CMR2 0x005199b0
int g_unk0x005199b0 = 0xacb49c;

// Path of a car's texture set: "<cars dir>\<car>d3<n>" or "<cars dir>\<car>c1".
// FUNCTION: CMR2 0x00420060
char *Car_GetTextureSetPath(int car, int variant, int unused)
{
    if ((BYTE)RallyData_GetFlag24()) {
        sprintf(g_raceCarPath, g_strCarD3Format, CInstallInfo::GetCarsDir(), CFrontend::FUN_0040ee60(car),
                variant + 1);
        return g_raceCarPath;
    }
    sprintf(g_raceCarPath, g_strCarC1Format, CInstallInfo::GetCarsDir(), CFrontend::FUN_0040ee60(car));
    return g_raceCarPath;
}

// Path of a car's directory ("<cars dir>\<car>").
// FUNCTION: CMR2 0x004200d0
char *Car_GetDirectoryPath(int car)
{
    sprintf(g_raceCarPath, g_strPathFormat, CInstallInfo::GetCarsDir(), CFrontend::FUN_0040ee60(car));
    return g_raceCarPath;
}

extern int g_unk0x00537f5c;
// GLOBAL: CMR2 0x00537f64
BYTE g_raceResourcesFreed;
extern unsigned int g_unk0x00537fc0;
void Sound_FreeAll(void);

/* --------------------------------------------------------------------------
   Race bootstrap / teardown transitions (0x41bf80..0x41e5c0).
   -------------------------------------------------------------------------- */

void Car_SetDrawnFlag(int, char);
void FUN_0041c5a0(BYTE, int);
struct ReplayStream;
int FUN_0046cce0(ReplayStream *p, short lane, short start, BYTE car);
void FUN_00403500(void);
void ForceFeedback_DeactivateSlots(void);
int FUN_004582d0(int);
void FUN_00455260(void);
BYTE FUN_00478b80(void);
void FUN_00478be0(void);
void Replay_InitSlots(void);
void View_SetCameraType(int, int, BYTE, int);
BOOL Sound_Init(int, int, int, int);
// GLOBAL: CMR2 0x00519284
char g_strArcadeAdp0x00519284[] = "%s\\arcade%d.adp";

// GLOBAL: CMR2 0x00537ffc
int g_unk0x00537ffc;
// GLOBAL: CMR2 0x00537f2c
int g_unk0x00537f2c;

void FUN_00427640(BYTE);
void FUN_00409dd0(void);
void FUN_004a0c40(char);
void FUN_004728c0(void);
void FUN_0040cf00(void);
BYTE RallyData_FUN_00408300(void);
void RallyTiming_ResetOverallPlayerTimes(void);
void FUN_0040cc60(void);
void FUN_0040ccb0(void);
void CFrontend::FUN_004cf0f0(void);
void FUN_00427c10(void);
void CGame::FUN_0041f260(void);
void FUN_00478b50(void);
BYTE FUN_00478a20(void);
void FUN_0040a580(int, int, int);
void FUN_0040efa0(void);
void Race_BuildSelectionPaths(void);
void FUN_00455080(void);
void FUN_00475f00(void);
// GLOBAL: CMR2 0x00538100
BYTE g_unk0x00538100;
// GLOBAL: CMR2 0x00537fd4
BYTE g_unk0x00537fd4;

// Enters a race: resets the race state, builds the stage data of the current
// mode and starts the race scene.
// FUNCTION: CMR2 0x0041bf80
void FUN_0041bf80(int param1, int param2)
{
    g_unk0x00538100 = 0;
    FUN_00427640(0);
    g_unk0x00537fd4 = 0;
    FUN_00409dd0();
    g_unk0x0053810c = 0;
    g_unk0x00537f0c[5] = param1;
    g_unk0x00537ffa = 0;
    g_unk0x0053810d = 0;
    g_unk0x00538108 = 1;
    g_unk0x00537f94 = 0;
    if (CGameInfo::FUN_00405e00() != 0)
        FUN_0040a580(0, 0, 0);
    if ((char)param2 == 0) {
        FUN_004a0c40(0);
        CInput::FUN_0049ff80(-1, -1, -1, -1, -1);
        if (CGameInfo::FUN_00405d80() == 4)
            FUN_004728c0();
        switch (CGameInfo::FUN_00405d80()) {
        case 0:
            if (RallyDataStageIndex() == 0) {
                if ((BYTE)RallyDataCountryIndex() == 0)
                    FUN_0040cf00();
                RallyTiming_ResetOverallPlayerTimes();
            }
            break;
        case 1:
            if (RallyDataStageIndex() == 0)
                RallyTiming_ResetOverallPlayerTimes();
            break;
        case 8:
            if (RallyData_FUN_00408300() != 0)
                RallyTiming_ResetOverallPlayerTimes();
            break;
        case 5:
            if ((BYTE)RallyData_FUN_00406950() == 0)
                FUN_0040cc60();
            FUN_0040ccb0();
            break;
        }
        if (CGameInfo::FUN_00405d80() != 4)
            CFrontend::FUN_004cf0f0();
        FUN_00427c10();
        Race_BuildSelectionPaths();
        CGame::FUN_0041f260();
        FUN_00455080();
        FUN_00478b50();
        FUN_00478a20();
        FUN_0040efa0();
        FUN_00475f00();
        g_raceResourcesFreed = 0;
    }
    CGame::FUN_0049c1c0((Unk0049c2c0 *)param1, param2, 0, 2);
}

// Starts the arcade race: initialises the sound system, the replay slots and
// the race scene, and queues the arcade music track of the current rally.
// FUNCTION: CMR2 0x0041c0e0
void FUN_0041c0e0(int param1, int param2)
{
    char buffer[MAX_PATH];

    CGameInfo::FUN_0049ea90(0);
    if ((char)param2 == 0) {
        Sound_Init(0x5622, 2, 0x10, 1);
        FUN_00455260();
        FUN_00478b80();
        FUN_00478be0();
        if ((BYTE)RallyData_FUN_00407e70() != 0) {
            sprintf(buffer, g_strArcadeAdp0x00519284, CInstallInfo::GetMusicDir(),
                    (BYTE)RallyData_FUN_00406940() * 3 + 1 + (BYTE)RallyData_FUN_00406950());
            CSound::FUN_004a28d0(buffer);
        }
        FUN_00403500();
        Replay_InitSlots();
        g_unk0x00537ffc = CMain::GetFrameDelta();
    }
    CGame::FUN_0049c1c0((Unk0049c2c0 *)param1, param2, 0, 2);
    g_unk0x00537f08 = 1;
    g_unk0x00537f2c = 0;
    g_unk0x00537f78[5] = 1;
}

// Leaves the current race: releases the frame resources, tears the stage list
// down and fades the race out.
// FUNCTION: CMR2 0x0041e5c0
void FUN_0041e5c0(int param1, char param2)
{
    int i;
    BYTE *p;

    i = 0;
    if (g_unk0x00538110 != 0) {
        g_unk0x00538110 = 0;
        g_unk0x00538114 = 1;
        FUN_0046cce0((ReplayStream *)g_unk0x00537f3c[0], 0, 0, 0);
    }
    if (CGameInfo::FUN_00405d80() == 6)
        Car_SetDrawnFlag(0, -1);
    if (param2 != 0)
        return;
    ForceFeedback_DeactivateSlots();
    if (CGameInfo::FUN_00405d80() == 6) {
        if (FUN_004582d0(0) >= 1)
            goto available;
        goto done;
    } else {
        if (FUN_0041f3d0(0) != 0)
            goto done;
    }
available:
    p = FUN_0041b390();
    if (*p > 0) {
        do {
            CGame::FUN_0049c1c0((Unk0049c2c0 *)FUN_0041b390(), i, 1, 3);
            i++;
            p = FUN_0041b390();
        } while (i < *p);
    }
    FUN_0041f2a0();
done:
    FUN_0041c5a0(**(BYTE **)(param1 + 4), 1);
    g_unk0x00537f08 = 1;
}

typedef void (*FadeCallback)(BYTE index);
int FUN_0040af30(void);
void FUN_0040af40(void);
int FUN_00406710(void);
void FUN_0041f2b0(void);
void FUN_0040ad20(void);
unsigned int FUN_00409cb0(int);
unsigned int FUN_0040a450(int);
void View_SwitchCamera(BYTE, int, int, BYTE, int);
void FUN_00427890(void);
void FUN_004283e0(BYTE, FadeCallback, int, int, int, char);
void FUN_00428410(BYTE, int, FadeCallback, int, int, int, int, char);
void FUN_00449090(BYTE);
void FUN_0044a150(void);
BOOL FUN_004a15a0(void);
unsigned char FUN_004d0580(void);
int FUN_0046d2a0(int *);
void FUN_00480380(void);
BYTE FUN_00422fb0(BYTE);
int RallyData_IsChampionshipFinalStage(void);
void FUN_0041e220(int player);
extern BYTE g_unk0x0053811f;
// GLOBAL: CMR2 0x005191a4
int g_unk0x005191a4 = 0xacb49c;
// GLOBAL: CMR2 0x00537fd8
int g_unk0x00537fd8[8];

unsigned int RallyData_FUN_004082e0(void);
BYTE FUN_004582b0(int index);
BYTE FUN_00448ca0(void);
int FUN_00487130(void);
int FUN_004582f0(int index);
int FUN_004481c0(int car);
void FUN_00448630(int index);
int FUN_00406770(void);
void FUN_0040ac40(BYTE carClass);
void FUN_00469a80(int car);
void FUN_00478150(int index);
int FUN_004483c0(int index);
int FUN_00448550(void);
void FUN_00427950(int time);
void FUN_0040af00(unsigned int time);
extern int g_unk0x00537f30;
extern BYTE g_unk0x0053811d;
extern BYTE g_unk0x0053811e;
extern BYTE g_unk0x00538120;

void FUN_00427930(void);
void FUN_00478130(int index);
void FUN_00478170(int index);
int FUN_004781c0(int index);
unsigned int FUN_00409ee0(int index);
unsigned char RallyData_FUN_00407ea0(void);
unsigned int RallyData_GetFlag22(void);
short Car_GetOrderCount(void);
void Car_ReloadModels(int, int, int);
void FUN_0047bdc0(char restart);
void FUN_00458b80(void);
void FUN_0044a120(void);
void FUN_00448100(void);
struct ReplayStream;
int FUN_0046c750(ReplayStream *p, int unused, BYTE car);
int FUN_0040b010(int index);
BYTE Car_GetDrawnFlag(int index);
char RallyData_FUN_00408500(BYTE param1);
void FUN_00466080(void);
void FUN_00466030(int, int);
HRESULT FUN_004a2bd0(int param1);
void FUN_00456b00(int value);

// Per-driver in-race update of the pre-race scene: resets the slot state, runs
// the countdown/fade of the current mode and rebuilds the player's view slots.
// FUNCTION: CMR2 0x0041d2b0
void FUN_0041d2b0(BYTE *param1, unsigned int param2)
{
    unsigned int index;
    int count;
    int i;
    BYTE **p;

    g_unk0x00538120 = 0;
    g_unk0x0053811f = 0;
    g_unk0x0053811e = 0;
    g_unk0x0053811d = 0;
    if (g_unk0x00537fd4 == 0) {
        FUN_00427930();
        g_unk0x00537fd4 = 1;
    }
    index = param2 & 0xff;
    g_unk0x00537f78[4] = 0;
    g_unk0x00537fcc = 0;
    g_unk0x005191a0 = 0xff;
    g_unk0x00537f98[index] = 0;
    g_unk0x00537fd8[index] = 0;
    if ((char)RallyData_FUN_00407e90() != 0 && CGameInfo::FUN_00405e00() == 0) {
        g_unk0x00537fbc[0] = 0;
        g_unk0x00537fbc[1] = 0;
    }
    g_unk0x00537ff8[index] = 0;
    FUN_00478150(index);
    count = FUN_004781c0(index);
    if (CGameInfo::FUN_00405e00() != 0) {
        if (CGameInfo::FUN_00405d80() != 10)
            FUN_00427640(1);
        FUN_00427890();
        if (CGameInfo::FUN_00405d80() != 10) {
            for (i = 0; i < 7; i++) {
                if ((char)FUN_00409cb0(i) != 0 && (char)FUN_00409ee0(i) == 0) {
                    CInput::FUN_0040af20();
                    g_unk0x00537f08 = 1;
                    FUN_00427640(0);
                }
            }
        }
        if ((unsigned int)(CMain::GetFrameDelta() - FUN_0040af30()) >
                (unsigned int)(FUN_00406710() * 6000) && FUN_00406710() != 0 &&
            FUN_004a15a0() != 0 &&
            (CGameInfo::FUN_00405d80() == 10 || CGameInfo::FUN_00405d80() == 12)) {
            if (g_unk0x00538100 != 0)
                goto fade;
            FUN_0040af40();
            g_unk0x00538100 = 1;
            FUN_004283e0(0, FUN_00449090, 1, 0, g_unk0x005191a4, 1);
            return;
        }
        if (g_unk0x00538100 != 0) {
fade:
            FUN_0041c5a0(*(BYTE *)(*(int *)(param1 + 4) + index * 8), 1);
            return;
        }
    }
    if (g_unk0x00537f08 != 0) {
        g_unk0x00537f24 = 0;
        FUN_00478130(index);
        FUN_00478170(index);
        g_unk0x00537f00 = FUN_004781c0(index);
        g_unk0x00537fc0 = 1;
        if (index == *param1 - 1)
            g_unk0x00537f08 = 0;
    } else if ((char)param2 == 0) {
        g_unk0x00537fc0 = count - g_unk0x00537f00;
        if (g_unk0x00537fc0 == 0)
            g_unk0x00537fc0 = 1;
    }
    if ((char)param2 == 0)
        FUN_0041c5a0(*(BYTE *)(*(int *)(param1 + 4)), 0);
    FUN_0044a120();
    if (CGameInfo::FUN_00405d80() == 5 || CGameInfo::FUN_00405d80() == 6 ||
        CGameInfo::FUN_00405d80() == 7 || CGameInfo::FUN_00405d80() == 4) {
        if ((unsigned int)g_unk0x00537fc0 >= 500)
            goto big;
    } else if ((unsigned int)g_unk0x00537fc0 >= 500) {
        goto big;
    }
    if (FUN_004d0580() == 0) {
        if (CGameInfo::FUN_00405d80() == 12)
            CInput::FUN_0040af20();
        return;
    }
big:
    CGame::FUN_0049c1c0((Unk0049c2c0 *)param1, param2, 0, 2);
    FUN_00448100();
    if ((char)RallyData_GetFlag22() != 0 && (char)RallyData_GetFlag25() == 0 &&
        CGameInfo::FUN_00405e00() == 0) {
        Car_ReloadModels(RallyDataState() & 0xff,
                     Car_GetOrderCount() - (RallyDataState() & 0xff), 0);
        if ((char)RallyData_FUN_00407e70() != 0)
            FUN_0047bdc0(0);
    }
    if (((char)RallyData_FUN_00407e70() != 0 || (char)RallyData_FUN_00407e90() != 0) &&
        CGameInfo::FUN_00405e00() == 0) {
        FUN_00458b80();
        if (CGameInfo::FUN_00405d80() == 4)
            FUN_00456b00(*RallyData_GetChampionshipState() >> 12 & 0xf);
    }
    if (CGameInfo::FUN_00405e00() != 0) {
        FUN_0046c750((ReplayStream *)g_unk0x00537f3c[0], 0, 0);
        i = 0;
        p = g_unk0x00537f3c + 1;
        do {
            if ((char)FUN_00409cb0(i) != 0)
                FUN_0046c750((ReplayStream *)*p, 0, FUN_0040b010(i));
            p++;
            i++;
        } while ((int)p < (int)(g_unk0x00537f3c + 8)); // 0x537f5c in the original
    } else if ((BYTE)RallyDataState() == 1 && (char)RallyData_GetFlag25() != 0) {
        if (CGameInfo::FUN_00405d80() == 4) {
            i = 0;
            p = g_unk0x00537f3c;
            do {
                if ((char)Car_GetDrawnFlag(i) == -1) {
                    FUN_0046d2a0((int *)*p);
                    FUN_0046cce0((ReplayStream *)*p, 0, 0, i);
                } else {
                    FUN_0046c750((ReplayStream *)*p, 0, param2);
                }
                p++;
                i++;
            } while ((int)p < (int)&g_unk0x00537f3c[2]);
        } else if ((char)RallyData_GetFlag25() != 0) {
            for (i = 0; i < ((char)RallyData_FUN_00407ea0() != 0 ? 1 : 2); i++) {
                if (CGameInfo::FUN_00405da0() != 0) {
                    if (i != 0) {
                        FUN_0046d2a0((int *)g_unk0x00537f3c[1]);
                        FUN_0046cce0((ReplayStream *)g_unk0x00537f3c[1], 0, 0, i);
                    } else {
                        FUN_0046c750((ReplayStream *)g_unk0x00537f3c[i], 0, param2);
                    }
                } else if ((char)RallyData_FUN_00408500(i) == -1) {
                    FUN_0046c750((ReplayStream *)g_unk0x00537f3c[i], 0, param2);
                } else {
                    FUN_0046d2a0((int *)g_unk0x00537f3c[i]);
                    FUN_0046cce0((ReplayStream *)g_unk0x00537f3c[i], 0, 0, i);
                }
            }
        }
    } else {
        FUN_0046c750((ReplayStream *)g_unk0x00537f3c[index], 0, param2);
    }
    if ((char)RallyData_FUN_00407ea0() != 0 && (char)CGameInfo::FUN_00406310() != 0) {
        FUN_00466080();
        FUN_00466030(0, 0);
    }
    FUN_00478130(index);
    if ((char)RallyData_FUN_00407e70() != 0) {
        CSound::FUN_004a31f0(CGameInfo::FUN_00405e40());
        FUN_004a2bd0(1);
    }
}

// Per-driver in-race update of the pre-race countdown: tracks whether the field
// has settled, advances the race-slot bookkeeping and drives the fade-out.
// FUNCTION: CMR2 0x0041d7a0
void FUN_0041d7a0(int param1, unsigned int param2)
{
    unsigned int index;
    unsigned int cond;
    char flag;
    int i;
    int j;
    int t;

    index = param2 & 0xff;
    FUN_00478150(index);
    flag = FUN_004582b0(index);
    if ((char)RallyData_FUN_004082e0() != 0 && (char)FUN_00448ca0() != 0)
        flag = 1;
    if ((char)RallyData_FUN_00407e90() != 0 && CGameInfo::FUN_00405e00() == 0 && FUN_00487130() != 0) {
        i = FUN_004582f0(1);
        j = FUN_004582f0(0);
        if (abs(j - i) < 3) {
            i = FUN_004481c0(1);
            j = FUN_004481c0(0);
            g_unk0x005191a0 = (j < i);
            if (index == (int)(char)g_unk0x005191a0) {
                FUN_00448630(index);
                g_unk0x00537fbc[index] = 1;
                flag = 1;
            }
        }
    }
    g_unk0x0053811e = 0;
    g_unk0x0053811f = 0;
    if (CGameInfo::FUN_00405e00() != 0) {
        if (FUN_00406770() > -1) {
            if (CGameInfo::FUN_00405d80() != 10) {
                if (CGameInfo::FUN_00405d80() != 12) {
                    if (g_unk0x0053811d != 0) {
                        t = CMain::GetFrameDelta() - g_unk0x00537f30;
                        if ((unsigned int)t > (unsigned int)(FUN_00406770() * 100))
                            g_unk0x0053811e = 1;
                    } else {
                        for (i = 0; i < 7; i++) {
                            if ((char)FUN_00409cb0(i) != 0 && (char)FUN_0040a450(i) != 0) {
                                g_unk0x0053811d = 1;
                                if (g_unk0x00538120 == 0) {
                                    g_unk0x00538120 = 1;
                                    g_unk0x00537f30 = CMain::GetFrameDelta();
                                }
                            }
                        }
                    }
                }
            }
        }
        if (CGameInfo::FUN_00405d80() == 10 || CGameInfo::FUN_00405d80() == 12) {
            if (CGameInfo::FUN_00405d80() == 12)
                flag = 0;
            t = FUN_0040af30();
            if ((unsigned int)(CMain::GetFrameDelta() - t) > (unsigned int)(FUN_00406710() * 6000) &&
                FUN_004a15a0() != 0 && FUN_00406710() != 0) {
                if (g_unk0x00538100 != 0)
                    goto fade;
                g_unk0x0053811f = 1;
                FUN_0040af40();
                g_unk0x00538100 = 1;
                FUN_004283e0(0, FUN_00449090, 1, 0, g_unk0x005191a4, 1);
            }
        }
        if (g_unk0x00538100 != 0) {
fade:
            FUN_0041c5a0(*(BYTE *)(*(int *)(param1 + 4) + index * 8), 1);
            return;
        }
    }
    if (flag != 0 || FUN_004d0580() != 0 || g_unk0x0053811e != 0) {
        if (CGameInfo::FUN_00405e00() != 0)
            FUN_0040ac40(3);
        g_unk0x00537f78[4] = g_unk0x00537f78[4] + 1;
        g_unk0x00537f98[index] = 1;
        FUN_00469a80(index);
        g_unk0x00537f34[index] = CMain::GetFrameDelta();
        CGame::FUN_0049c1c0((Unk0049c2c0 *)param1, param2, 0, 2);
    }
    index = RallyDataState();
    cond = (g_unk0x00537f78[4] == index);
    if (CGameInfo::FUN_00405e00() != 0 && CGameInfo::FUN_00405d80() == 10)
        cond = (unsigned char)flag;
    if ((char)param2 == 0) {
        FUN_0041c5a0(*(BYTE *)(*(int *)(param1 + 4)), cond);
        if (cond == 0) {
            if (FUN_004d0580() == 0)
                goto done;
        }
        if (CGameInfo::FUN_00405e00() != 0) {
            View_SwitchCamera(0, 7, 0xffff, FUN_00422fb0(0), 0);
            if (g_unk0x0053811f == 0) {
                FUN_00427950(g_unk0x0053811e != 0 ? FUN_00448550() : FUN_004483c0(0));
                if (g_unk0x00538120 == 0) {
                    if (FUN_00406770() > -1) {
                        g_unk0x00538120 = 1;
                        g_unk0x00537f30 = CMain::GetFrameDelta();
                    }
                }
                FUN_0040af00(FUN_004483c0(0));
            }
        }
    }
done:
    if (flag != 0)
        View_SwitchCamera(param2, 7, 0xffff, FUN_00422fb0(param2), 0);
}

unsigned int RallyData_GetFlag22(void);
unsigned char RallyData_FUN_00407ea0(void);
int Replay_StopRecording(BYTE *pBuffer);
void ForceFeedback_DeactivateSlots(void);
int FUN_00428740(BYTE index);
void FUN_004483e0(void);
void FUN_00448620(void);
int FUN_00448670(void);
unsigned int FUN_00448680(int index, int split);
int FUN_0040ab10(void);
unsigned int FUN_0040ab80(int index, int total);
char *FUN_0040abb0(int index, int total);
void FUN_0040e660(int index, char *name, int wins);
void FUN_0040a820(unsigned int localTime);
void FUN_0040a980(unsigned int localTime);
void FUN_0040cf30(void);
void FUN_0040af60(void);
void FUN_0044a1b0(int value);
int FUN_004a2f50(void);
void FUN_00465530(void);
BYTE FUN_0041b370(void);
void FUN_0041b460(void);
char *FUN_0041f920(void);
void FUN_00455af0(int driver, int hundredths, int split);
int StageTiming_GetDriverSlot(int iDriver);
int Replay_Save(BYTE *pBuffer, char *pName);
int GetStageSplitCount(void);
BYTE RallyData_FUN_00408340(void);
BYTE FUN_004071c0(BYTE flags, char mode);

// In-race per-driver update: enforces the pre-race hold, saves the stage record
// once every driver is ready and refreshes the leaderboard/knockout tables.
// match 86%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// The stage-record block now reproduces the original's shape: the index is written
// inline at each use (Country/Stage re-called, the 11*country product hoisted across
// the second call), FUN_004483c0(index) is pre-read into a raw temp and the record is
// walked through a pointer to its .value. What is left is the tail's register
// allocation: MSVC6 dedicates EDI to the constant 0 (the original pushes the literal
// 0 in ~14 call arguments) and materialises the two leaderboard comparisons with
// setcc instead of the original's branchy 1/0 (and 3/0) selection — a byte register is
// free here in our build because our function keeps one value less live.
// match 85%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0041db10
void FUN_0041db10(BYTE *param1, unsigned int param2)
{
    unsigned int index;
    int hold;
    int i;
    BYTE ready;
    unsigned int flag;
    BYTE k;
    BYTE mode;
    BYTE **pp;
    unsigned int v;
    unsigned int t;
    unsigned int *pRecord;

    if (CGameInfo::FUN_00405d80() == 10 &&
        (unsigned int)(CMain::GetFrameDelta() - FUN_0040af30()) >
            (unsigned int)(FUN_00406710() * 6000) &&
        FUN_00406710() != 0 && FUN_004a15a0() != 0 && g_unk0x00538100 == 0) {
        FUN_0040af40();
        g_unk0x00538100 = 1;
        FUN_004283e0(0, FUN_00449090, 1, 0, g_unk0x005191a4, 1);
        return;
    }
    if (g_unk0x0053811c < 100)
        g_unk0x0053811c = g_unk0x0053811c + 1;
    hold = 400;
    if ((char)RallyData_FUN_00407e70() != 0 && CGameInfo::FUN_00405d70() == 2)
        hold = 800;
    index = param2 & 0xff;
    if ((unsigned int)(CMain::GetFrameDelta() - g_unk0x00537f34[index]) >= (unsigned int)hold &&
        g_unk0x00537fd8[index] == 0) {
        g_unk0x00537fd8[index] = 1;
        g_unk0x00537fcc = g_unk0x00537fcc + 1;
        if (CGameInfo::FUN_00405e00() != 0) {
            Replay_StopRecording(g_unk0x00537f3c[0]);
            pp = g_unk0x00537f3c + 1;
            do {
                Replay_StopRecording(*pp);
                pp++;
            } while ((int)pp < (int)(g_unk0x00537f3c + 8)); // 0x537f5c in the original
        } else {
            Replay_StopRecording(g_unk0x00537f3c[index]);
        }
        flag = (CGameInfo::FUN_00405d00() == 0);
        if (CGameInfo::FUN_00405d80() == 3 && g_unk0x00537ff8[index] == 0) {
            v = CGameInfo::FUN_00405ff0(flag)->rallyStageRecordTimes[
                    (RallyDataCountryIndex() & 0xff) * 0xb + (RallyDataStageIndex() & 0xff)].value >> 7 & 0xffff;
            if ((int)FUN_004483c0(index) <= (int)v) {
                t = FUN_004483c0(index);
                pRecord = &CGameInfo::FUN_00405ff0(flag)->rallyStageRecordTimes[
                    (RallyDataCountryIndex() & 0xff) * 0xb + (RallyDataStageIndex() & 0xff)].value;
                *pRecord = (t & 0xffff) << 7 | *pRecord & 0xff80007f;
                k = RallyData_FUN_004086b0(FUN_0041b370());
                pRecord = &CGameInfo::FUN_00405ff0(flag)->rallyStageRecordTimes[
                    (RallyDataCountryIndex() & 0xff) * 0xb + (RallyDataStageIndex() & 0xff)].value;
                *pRecord = k & 0x3f | *pRecord & 0xffffffc0;
                g_unk0x00538128 = 1;
                g_unk0x00519228 = RallyDataStageIndex() & 0xff;
                g_unk0x0051922c = RallyDataCountryIndex() & 0xff;
                sprintf(CFrontend::m_stringDest, g_strGrp0x005192b0, FUN_0041f920());
                if (g_unk0x00537f3c[index] != 0)
                    Replay_Save(g_unk0x00537f3c[index], CFrontend::m_stringDest);
            }
            g_unk0x00537ff8[index] = 1;
        }
        ForceFeedback_DeactivateSlots();
    }
    if ((char)param2 != 0)
        return;
    ready = 0;
    for (k = 0; k < (BYTE)RallyDataState(); k++) {
        if (FUN_00428740(k) != 0)
            ready = ready + 1;
    }
    flag = (g_unk0x00537fcc == (RallyDataState() & 0xff) && ready == 0);
    FUN_0041c5a0(*(BYTE *)(*(int *)(param1 + 4)), 1);
    if (CGameInfo::FUN_00405e00() != 0) {
        FUN_00427890();
        if (CGameInfo::FUN_00405d80() != 10 && CGameInfo::FUN_00405d80() != 12) {
            for (i = 0; i < 7; i++) {
                if ((char)FUN_00409cb0(i) != 0 && (char)FUN_0040a450(i) == 0) {
                    g_unk0x00537f34[0] = CMain::GetFrameDelta();
                    return;
                }
            }
        }
    }
    if (flag == 0 && FUN_004d0580() == 0) {
        g_unk0x00537f08 = 1;
        return;
    }
    for (k = 0; k < *param1; k++)
        CGame::FUN_0049c1c0((Unk0049c2c0 *)param1, k, 0, 2);
    if ((char)RallyData_GetFlag22() != 0 || (char)RallyData_GetFlag24() != 0)
        FUN_004483e0();
    if ((char)RallyData_FUN_00407e90() != 0 && CGameInfo::FUN_00405e00() == 0) {
        if (g_unk0x00537fbc[0] != 0)
            FUN_00455af0(StageTiming_GetDriverSlot(0), FUN_004483c0(0), GetStageSplitCount());
        if (g_unk0x00537fbc[1] != 0)
            FUN_00455af0(StageTiming_GetDriverSlot(1), FUN_004483c0(1), GetStageSplitCount());
    }
    if (((char)RallyData_GetFlag22() != 0 || (char)RallyData_GetFlag24() != 0) &&
        (char)RallyData_FUN_00407e90() != 0 && CGameInfo::FUN_00405e00() == 0 &&
        (BYTE)RallyDataState() == 1 && FUN_00448670() < 2)
        FUN_00455af0(StageTiming_FUN_00455ab0(FUN_0041b370() & 0xff), FUN_00448680(1, 2),
                     GetStageSplitCount());
    if (CGameInfo::FUN_00405da0() != 0 &&
        (FUN_0041b370() & 0xff) != ((CGameInfo::FUN_00405d70() & 0xff) - 1))
        goto label2;
    if (CGameInfo::FUN_00405d80() != 0 && CGameInfo::FUN_00405d80() != 1 &&
        CGameInfo::FUN_00405d80() != 8)
        goto label2;
    StageTiming_AddToOverall();
    if (CGameInfo::FUN_00405d80() == 0 &&
        RallyDataStageIndex() ==
            (BYTE)FUN_004071c0(RallyDataCountryIndex(), CGameInfo::FUN_00405d90()))
        FUN_0040cf30();
label2:
    if (CGameInfo::FUN_00405d80() == 5)
        FUN_00448620();
    if (CGameInfo::FUN_00405da0() == 0 ||
        (FUN_0041b370() & 0xff) != ((CGameInfo::FUN_00405d70() & 0xff) - 1)) {
        if (CGameInfo::FUN_00405da0() != 0)
            goto label4;
    }
    if (CGameInfo::FUN_00405d80() == 4 || CGameInfo::FUN_00405d80() == 8 ||
        CGameInfo::FUN_00405d80() == 9 || CGameInfo::FUN_00405d80() == 10 ||
        CGameInfo::FUN_00405d80() == 11 || CGameInfo::FUN_00405d80() == 12)
        goto label4;
    if ((BYTE)CGameInfo::FUN_00406430() != 0) {
        for (i = 0; i < (BYTE)CGameInfo::FUN_00405d70(); i++) {
            g_unk0x00537f68[i] = 0;
            g_unk0x00537f78[i] = 0;
        }
    } else {
        FUN_0041b460();
    }
label4:
    if (CGameInfo::FUN_00405e00() != 0) {
        mode = (BYTE)CGameInfo::FUN_00405d80();
        if (mode >= 8 && (mode <= 9 || mode == 11)) {
            FUN_0040a820(FUN_004483c0(0));
            FUN_0040a980(FUN_004483c0(0));
            if (CNetworkLeaderboards::GetLeaderboardId() != -1 && FUN_004a15a0() != 0) {
                FUN_0040e660(CNetworkLeaderboards::GetLeaderboardId(), FUN_0040abb0(0, 0), 1);
                for (i = 1; i < FUN_0040ab10(); i++)
                    FUN_0040e660(CNetworkLeaderboards::GetLeaderboardId(), FUN_0040abb0(i, 0),
                                 FUN_0040ab80(i, 0) == FUN_0040ab80(0, 0));
                if (CGameInfo::FUN_00405d80() == 8 && (char)RallyData_FUN_00408340() != 0) {
                    FUN_0040e660(CNetworkLeaderboards::GetLeaderboardId(), FUN_0040abb0(0, 1), 3);
                    for (i = 1; i < FUN_0040ab10(); i++)
                        FUN_0040e660(CNetworkLeaderboards::GetLeaderboardId(), FUN_0040abb0(i, 1),
                                     FUN_0040ab80(i, 1) == FUN_0040ab80(0, 1) ? 3 : 0);
                }
            }
        }
    }
    if (CGameInfo::FUN_00405e00() != 0 && FUN_004a15a0() != 0 &&
        CNetworkLeaderboards::GetLeaderboardId() != -1)
        FUN_0040af60();
    FUN_0044a1b0(0);
    if ((char)RallyData_FUN_00407ea0() != 0 && (char)CGameInfo::FUN_00406310() != 0)
        FUN_00466080();
    FUN_00465530();
    if ((char)RallyData_FUN_00407e70() != 0)
        FUN_004a2f50();
    g_unk0x00537f08 = 1;
}

int FUN_0046cce0(ReplayStream *p, short lane, short start, BYTE car);
int FUN_0046d2a0(int *p);
int FUN_0040b010(int index);
void View_ResetCameras(int player);
void FUN_00466080(void);
void FUN_00466030(int, int);

// Starts the replays of a race: the player's own, every network player's, the
// knockout opponent's and the CPU runs of a head-to-head stage.
// FUNCTION: CMR2 0x0041e220
void FUN_0041e220(int player)
{
    int i;

    FUN_0046cce0((ReplayStream *)g_unk0x00537f3c[(BYTE)player], 0, 0, player);
    if (CGameInfo::FUN_00405e00()) {
        for (i = 0; i < 7; i++) {
            FUN_0046d2a0((int *)g_unk0x00537f3c[i + 1]);
            FUN_0046cce0((ReplayStream *)g_unk0x00537f3c[i + 1], 0, 0, FUN_0040b010(i));
        }
    }
    if (CGameInfo::FUN_00405d80() == 4) {
        for (i = 0; i < 2; i++) {
            if ((char)Car_GetDrawnFlag(i) == -1) {
                FUN_0046d2a0((int *)g_unk0x00537f3c[i]);
                FUN_0046cce0((ReplayStream *)g_unk0x00537f3c[i], 0, 0, i);
            }
        }
    } else if ((char)RallyData_GetFlag25() && CGameInfo::FUN_00405e00() == 0) {
        for (i = 0; i < ((char)RallyData_FUN_00407ea0() ? 1 : 2); i++) {
            if (RallyData_FUN_00408500((BYTE)i) != -1 || (CGameInfo::FUN_00405da0() && i == 1)) {
                FUN_0046d2a0((int *)g_unk0x00537f3c[i]);
                FUN_0046cce0((ReplayStream *)g_unk0x00537f3c[i], 0, 0, i);
            }
        }
    }
    View_ResetCameras(player);
    if ((char)RallyData_FUN_00407ea0() && (char)CGameInfo::FUN_00406310()) {
        FUN_00466080();
        FUN_00466030(0, 0);
    }
}

// Waits for the inter-stage fade of a special stage (mode 10) and otherwise
// re-arms the per-player fade jobs of the in-race menu.
// FUNCTION: CMR2 0x0041e350
void FUN_0041e350(int param1, unsigned int param2)
{
    BYTE colour[4];
    int anyAlive;
    BYTE i;
    int flag;
    int x;
    int start;

    colour[0] = 0;
    colour[1] = 0;
    colour[2] = 0;
    colour[3] = 0;
    if (CGameInfo::FUN_00405d80() == 10) {
        start = FUN_0040af30();
        if ((unsigned int)(CMain::GetFrameDelta() - start) >
                (unsigned int)(FUN_00406710() * 6000) &&
            FUN_004a15a0() != 0 && FUN_00406710() != 0) {
            if (g_unk0x00538100 == 0) {
                FUN_0040af40();
                g_unk0x00538100 = 1;
                FUN_004283e0(0, FUN_00449090, 1, 0, g_unk0x005191a4, 1);
                return;
            }
            goto fadeEarly;
        }
        if (g_unk0x00538100 != 0)
            goto fadeEarly;
    }
    FUN_00427640(0);
    if (FUN_004d0580() != 0 || g_unk0x0053811f != 0)
        FUN_0041f2b0();
    if ((char)param2 != 0)
        return;
    anyAlive = 1;
    if ((BYTE)RallyData_IsChampionshipFinalStage() != 0 && g_unk0x00537f3c[0] != 0) {
        if (g_unk0x00538118 == 0) {
            if (*(int *)(g_unk0x00537f3c[0] + 4) != 0)
                goto done;
            View_SwitchCamera(0, 7, 0xffff, FUN_00422fb0(0), 0);
            g_unk0x00538118 = 1;
            FUN_0046d2a0((int *)g_unk0x00537f3c[0]);
            FUN_0046cce0((ReplayStream *)g_unk0x00537f3c[0], 0, 0, 0);
        }
        if (*(int *)(g_unk0x00537f3c[0] + 4) == 0) {
            FUN_00480380();
            FUN_0044a150();
        }
        goto done;
    }
    i = 0;
    if ((BYTE)RallyDataState() > 0) {
        do {
            if (FUN_0041f3d0(i) != 0)
                anyAlive = 0;
            i++;
        } while (i < (BYTE)RallyDataState());
        if (anyAlive == 0)
            goto done;
    }
    i = 0;
    if ((BYTE)RallyDataState() > 0) {
        do {
            flag = 3;
            if (g_unk0x00537fd8[i] != 0) {
                flag = 2;
                g_unk0x00537fd8[i] = 0;
            }
            FUN_00428410(i, 0xc8000, (FadeCallback)FUN_0041e220, flag, 2, 0, *(unsigned int *)colour, 0);
            i++;
        } while (i < (BYTE)RallyDataState());
    }
done:
    FUN_0041c5a0(**(BYTE **)(param1 + 4), 1);
    if (CGameInfo::FUN_00405e00() != 0) {
        FUN_00427890();
        if (CGameInfo::FUN_00405d80() == 10 || CGameInfo::FUN_00405d80() == 12)
            FUN_0040ad20();
        for (x = 0; x < 7; x++) {
            if ((BYTE)FUN_00409cb0(x) != 0 && (BYTE)FUN_0040a450(x) == 0)
                return;
        }
    }
    g_unk0x00537f08 = 1;
    return;
fadeEarly:
    FUN_0041c5a0(*(BYTE *)(*(int *)(param1 + 4) + (param2 & 0xff) * 8), 1);
}

// Releases the race resources once (sounds, callbacks, textures).
// FUNCTION: CMR2 0x0041e670
void FUN_0041e670(void)
{
    g_unk0x00537fc0 = 1;
    Sound_FreeAll();
    if (g_raceResourcesFreed == 0) {
        CGame::UnwindCallbacks(g_unk0x00537f5c);
        CGraphics::FUN_004a5be0();
        CGraphics::FreeTextureBuffers();
        g_raceResourcesFreed = 1;
    }
}

BYTE *FUN_0041f900(void);
void StageLights_SetTransform(FixVector *pAxes);
void FUN_00490d50(BYTE *pData);

void FUN_0040a230(int splitCount);
int FUN_00427660(void);
void FUN_004cf140(void);
void FUN_0041b300(void);
void FUN_0041b360(void);
void FUN_0041b340(char bFlag);
void FUN_0040d010(void);
void RallyData_FUN_004068e0(BYTE param1);
void RallyData_FUN_00408290(void);
void FUN_00406820(void);
void RallyData_FUN_00408390(void);
void FUN_00409b60(void);
void FUN_00409e30(char resetTotal, char resetTimes);
void RallyData_FUN_00406960(BYTE param1);
void RallyData_InitKnockoutBracket(void);
void FUN_00472ca0(void);
void FUN_00420100(void);
void FUN_00418f20(void);
void FUN_004cf260(void);
void FUN_00469b50(int index);
BYTE RallyData_AdvanceSelectedStage(void);
void FUN_004067c0(BYTE param1);
bool RallyData_FUN_004074a0(void);
int FUN_004728d0(void);
void FUN_004728c0(void);
BYTE FUN_004729f0(void);
void FUN_00409ab0(char keepReady, char resetTotal);
void FUN_004660a0(int **pValue, int slot, char flag);

void FUN_00424ed0(void);
void FUN_00409dd0(void);
void FUN_00409ab0(char keepReady, char resetTotal);
void FUN_00416670(void);
void StageLights_Off(void);
void RallyData_FUN_004207f0(void);
unsigned int RallyData_GetFlag31(void);
void FUN_004584d0(char param_1);
short Car_GetOrderCount(void);
void Car_ReloadModels(int, int, int);
Car *Car_Get(int index);
void FUN_00420850(Car *pCar);
void FUN_00458480(void);
void View_ResetCameras(int view);
void FUN_00455470(char);
void FUN_00458100(int param_1);
void FUN_00411450(int keepName);
void FUN_00465530(void);
void FUN_004702a0(void);
void FUN_0047bdc0(char restart);
void FUN_00447f70(void);
int FUN_0046d2a0(int *p);
int Replay_StopRecording(BYTE *pBuffer);
void FUN_00466080(void);
void FUN_004660a0(int **pValue, int slot, char flag);
void ForceFeedback_ActivateIdleSlots(void);
void FUN_004cf140(void);
void FUN_0041b360(void);
extern BYTE g_unk0x00538100;
extern BYTE g_unk0x0053811d;
extern BYTE g_unk0x0053811e;
extern BYTE g_unk0x0053811f;
extern BYTE g_unk0x00538120;
extern int g_unk0x00537f5c;

// Tears the current stage down: resets the race flags, stops the stage lights,
// releases the view slots and replays of every car and refreshes the HUD.
// match 65%: the code is the same but MSVC6's allocator does not materialise the
// constant 0 in a callee-saved register here: the original keeps the zero in EBX
// for all of its ~12 uses (so every loop counter lives in ESI/EDI/EBP), while ours
// folds the zeros into immediates and needs EBX for the loop indices, which
// renumbers the registers of the whole function. Verified by construction: a
// variant without the FUN_004660a0 loop does get the EBX zero register (like
// mini2.cpp: 0s separated by calls), so the trigger is that last loop.
// FUNCTION: CMR2 0x0041e6b0
void Race_TeardownStage(int param1, int param2, char flag)
{
    BYTE **pp;
    int n;
    int i;
    int zero = 0;

    g_unk0x00538100 = zero;
    FUN_00424ed0();
    FUN_00409dd0();
    g_unk0x00538120 = zero;
    g_unk0x0053811f = zero;
    g_unk0x0053811e = zero;
    g_unk0x0053811d = zero;
    FUN_00409ab0(1, 1);
    FUN_00416670();
    StageLights_Off();
    RallyData_FUN_004207f0();
    if (RallyData_FUN_00407e70() || RallyData_GetFlag25() || (BYTE)RallyData_GetFlag31())
        if (CGameInfo::FUN_00405d80() == 4)
            FUN_004584d0(zero);
        else
            FUN_004584d0(1);
    Car_ReloadModels(zero, (int)Car_GetOrderCount(), zero);
    if (CGameInfo::FUN_00405d80() == 4) {
        for (i = 0; i < Car_GetOrderCount(); i++)
            FUN_00420850(Car_Get(i));
    }
    FUN_00458480();
    if (flag != zero) {
        for (i = 0; i < *(BYTE *)param1; i++) {
            CGame::FUN_0049c1c0((Unk0049c2c0 *)param1, i, 4, 2);
            View_ResetCameras(i);
            RallyData_ValidateIndex(i);
        }
    }
    FUN_00455470(zero);
    FUN_00458100(1);
    FUN_00411450(g_unk0x0053810c & 0xff);
    FUN_00465530();
    FUN_004702a0();
    if (RallyData_FUN_00407e70())
        FUN_0047bdc0(1);
    FUN_00447f70();
    if (CGameInfo::FUN_00405e00()) {
        n = 1;
        pp = g_unk0x00537f3c + 1;
        do {
            FUN_0046d2a0((int *)*pp);
            Replay_StopRecording(*pp);
            pp++;
        } while ((int)pp < (int)(g_unk0x00537f3c + 8)); // 0x537f5c in the original
    } else if (CGameInfo::FUN_00405d80() == 4 || (char)RallyData_GetFlag25()) {
        n = 2;
    } else {
        n = (BYTE)RallyDataState();
    }
    if (n > zero) {
        pp = g_unk0x00537f3c;
        do {
            FUN_0046d2a0((int *)*pp);
            Replay_StopRecording(*pp);
            pp++;
        } while (--n);
    }
    if ((char)RallyData_FUN_00407ea0() && (char)CGameInfo::FUN_00406310())
        FUN_00466080();
    if ((char)RallyData_FUN_00407ea0() && (char)CGameInfo::FUN_00406310()) {
        for (i = 0; i < (BYTE)RallyDataState(); i++)
            FUN_004660a0((int **)g_unk0x00537f3c + i, i, (BYTE)param2 + i);
    }
    ForceFeedback_ActivateIdleSlots();
    CGameInfo::FUN_0049ea90(0);
    if (CGameInfo::FUN_00405d80() != 4) {
        FUN_004cf140();
        FUN_0041b360();
    }
    g_unk0x0053810c = zero;
}

// In-race mode/state handler: stage teardown, replay and view transitions.
// Keep the player list separate from the timer query's output slot, preserve
// the saved phase byte, and share the original stage-dependent restore exits.
// Differential fixtures cover every entry of the original five jump tables;
// teardown/replay/view/sound actions are controlled boundaries in those tests.
// FUNCTION: CMR2 0x0041e8d0
void FUN_0041e8d0(BYTE *param1, unsigned int param2)
{
    int i;
    short n;
    int skip = 0;
    int count;
    BYTE *p;
    BYTE **pp;
    BYTE *pPlayers = param1;

    count = *pPlayers;
    i = 0;
    if (count > 0) {
        p = *(BYTE **)(pPlayers + 4);
        do {
            if (*p != 11)
                return;
            i++;
            p += 8;
        } while (i < count);
    }
    if ((char)param2 != 0)
        return;
    if (CGameInfo::FUN_00405da0() == 0)
        CGraphics::SetClearColour(1, 0x9c, 0xb4, 0xac);
    if ((char)RallyData_FUN_00407e70() != 0)
        CSound::StopDirectSoundBuffer();
    if (CGameInfo::FUN_00405d80() == 10)
        FUN_0040a230(FUN_00427660());
    FUN_004cf140();
    FUN_00403500();
    g_unk0x00537f08 = 1;
    *(BYTE *)&param2 = FUN_0041b370();
    if (g_unk0x0053810c != 0) {
        FUN_0041b360();
        switch (CGameInfo::FUN_00405d80()) {
        case 0:
            if (RallyDataStageIndex() ==
                (BYTE)FUN_004071c0(RallyDataCountryIndex(), CGameInfo::FUN_00405d90()))
                FUN_0040d010();
            /* fall through */
        case 1:
            CGame::FUN_004057e0(2);
            RallyData_FUN_004068e0(0);
            for (i = 0; i < *pPlayers; i++)
                CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, i, 0, 3);
            RallyData_FUN_00408290();
            FUN_00406820();
            FUN_0041e670();
            FUN_0041b300();
            return;
        case 8:
            CGame::FUN_004057e0(2);
            RallyData_FUN_00408390();
            for (i = 0; i < *pPlayers; i++)
                CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, i, 0, 3);
            RallyData_FUN_00408290();
            FUN_00406820();
            FUN_0041e670();
            FUN_0041b300();
            FUN_00409b60();
            FUN_00409e30(1, 1);
            return;
        case 2:
        case 3:
        case 9:
        case 10:
            RallyData_FUN_00408290();
            if (g_unk0x00537ffa != 0) {
                CGame::FUN_004057e0(2);
                for (i = 0; i < *pPlayers; i++)
                    CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, i, 0, 3);
                FUN_0041e670();
                FUN_0041b300();
                FUN_00409e30(1, 0);
                return;
            }
            if (FUN_0041b380() == 4) {
                Race_TeardownStage((int)pPlayers, param2, 0);
                FUN_00420100();
                Sound_FreeAll();
                FUN_00418f20();
                CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, 0, 3, 2);
                return;
            }
            break;
        case 4:
            RallyData_FUN_00408290();
            FUN_0041f250();
            if (FUN_004728d0() == 0) {
                for (i = 0; i < *pPlayers; i++)
                    CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, i, 0, 3);
                FUN_0041e670();
                FUN_0041b300();
                RallyData_InitKnockoutBracket();
                FUN_00472ca0();
                return;
            }
            break;
        case 5:
            RallyData_FUN_00406960(0);
            for (i = 0; i < *pPlayers; i++)
                CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, i, 0, 3);
            RallyData_FUN_00408290();
            FUN_0041e670();
            FUN_0041b300();
            return;
        }
        Race_TeardownStage((int)pPlayers, param2, 1);
        if (CGameInfo::FUN_00405d80() != 4)
            return;
        if (FUN_004728d0() == 0)
            return;
    }
    if (g_unk0x0053810d != 0) {
        if ((char)RallyData_FUN_00407e70() != 0)
            CSound::FUN_004a2b50(0);
        if (CGameInfo::FUN_00406320() == 0) {
            CGameInfo::FUN_00406450((unsigned int **)&param1);
            n = CMain::GetFrameDelta();
            *(int *)param1 += n - g_unk0x00537ffc;
            g_unk0x00537ffc = CMain::GetFrameDelta();
        }
        for (i = 0; i < (BYTE)RallyDataState(); i++)
            FUN_00469b50(i);
        FUN_00469b50(0);
        if (CGameInfo::FUN_00405d80() != 4)
            FUN_004cf260();
        if (CGameInfo::FUN_00405d80() == 4) {
            if (FUN_004729f0() != 0) {
                for (i = 0; i < *pPlayers; i++)
                    CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, i, 0, 2);
                goto L_teardown;
            }
            FUN_00420100();
            Sound_FreeAll();
            FUN_00418f20();
            for (i = 0; i < *pPlayers; i++)
                CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, i, 7, 2);
            g_unk0x0053810d = 0;
            return;
        }
        for (i = 0; i < *pPlayers; i++)
            CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, i, 0, 2);
L_teardown:
        CGame::FUN_004057e0(0);
        FUN_0041e670();
        FUN_0041b300();
        g_unk0x0053810d = 0;
        return;
    }
    switch (FUN_0041b380()) {
    case 0:
        switch (CGameInfo::FUN_00405d80()) {
        case 0:
        case 1:
            for (i = 0; i < *pPlayers; i++)
                CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, i, 0, 2);
            if ((char)RallyData_AdvanceSelectedStage() != 0) {
                if ((char)RallyDataStageIndex() != 0)
                    goto L_stage_restore;
                FUN_004067c0(1);
            }
            goto L_teardown2;
        case 2:
        case 9:
        case 10:
        case 11:
        case 12:
            for (i = 0; i < *pPlayers; i++)
                CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, i, 0, 2);
            goto L_teardown2;
        case 3:
            for (i = 0; i < *pPlayers; i++)
                CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, i, 0, 2);
            goto L_teardown2;
        case 5:
            for (i = 0; i < *pPlayers; i++)
                CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, i, 0, 2);
            if (RallyData_FUN_004074a0())
                goto L_restore;
            goto L_teardown2;
        case 6:
            for (i = 0; i < *pPlayers; i++)
                CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, i, 0, 2);
            goto L_teardown2;
        case 7:
            for (i = 0; i < *pPlayers; i++)
                CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, i, 0, 2);
            goto L_teardown2;
        case 8:
            for (i = 0; i < *pPlayers; i++)
                CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, i, 0, 2);
            CGame::FUN_004057e0((char)RallyData_AdvanceSelectedStage() == 0 ? 0 : 2);
            FUN_00409ab0(0, 0);
            FUN_0041e670();
            FUN_0041b300();
            FUN_00409e30(0, 0);
            return;
        }
        break;
    case 1:
        switch (CGameInfo::FUN_00405d80()) {
        case 0:
        case 1:
            for (i = 0; i < *pPlayers; i++)
                CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, i, 0, 2);
            if ((char)RallyData_AdvanceSelectedStage() != 0) {
                if ((char)RallyDataStageIndex() != 0)
                    goto L_stage_restore;
                FUN_004067c0(1);
            }
            goto L_teardown2;
        case 2:
            for (i = 0; i < *pPlayers; i++)
                CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, i, 0, 2);
            if (*pPlayers != 1)
                return;
            goto L_teardown2;
        case 5:
            for (i = 0; i < *pPlayers; i++)
                CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, i, 0, 2);
            if (RallyData_FUN_004074a0())
                goto L_restore;
            goto L_teardown2;
        case 6:
            for (i = 0; i < *pPlayers; i++)
                CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, i, 0, 2);
            goto L_teardown2;
        }
        break;
    case 2:
    case 3:
        for (i = 0; i < *pPlayers; i++) {
            if (FUN_004728d0() == 0)
                CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, i, 0, 2);
        }
        if (CGameInfo::FUN_00405d80() == 4)
            FUN_004728c0();
        break;
    case 4:
        switch (CGameInfo::FUN_00405d80()) {
        case 0:
        case 1:
            if ((BYTE)FUN_0041b370() + 1 == (BYTE)CGameInfo::FUN_00405d70()) {
                CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, 0, 0, 2);
                if ((char)RallyData_AdvanceSelectedStage() != 0) {
                    if ((char)RallyDataStageIndex() != 0)
                        goto L_stage_restore;
                    FUN_004067c0(1);
                }
                goto L_teardown2;
            }
            FUN_00420100();
            Sound_FreeAll();
            if ((char)RallyData_FUN_00407ea0() == 0)
                skip = 1;
            FUN_00418f20();
            CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, 0, 3, 2);
            FUN_0041b340(0);
            if (skip != 0)
                return;
            break;
        case 2:
            if ((BYTE)FUN_0041b370() + 1 == (BYTE)CGameInfo::FUN_00405d70())
                goto L_winner;
            CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, 0, 3, 2);
            FUN_00420100();
            Sound_FreeAll();
            FUN_00418f20();
            FUN_0041b340(0);
            return;
        case 3:
            if (!((BYTE)FUN_0041b370() + 1 != (BYTE)CGameInfo::FUN_00405d70())) {
                goto L_winner;
            } else {
                FUN_00420100();
                Sound_FreeAll();
                FUN_00418f20();
                CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, 0, 3, 2);
                FUN_0041b340(0);
            }
            break;
        }
        break;
    }
    if (CGameInfo::FUN_00405d80() == 4 || (char)RallyData_GetFlag25() != 0)
        n = 2;
    else
        n = RallyDataState() & 0xff;
    if (n > 0) {
        pp = g_unk0x00537f3c;
        for (i = 0; i < n; i++) {
            FUN_0046d2a0((int *)*pp);
            Replay_StopRecording(*pp);
            pp++;
        }
    }
    if (CGameInfo::FUN_00405e00() != 0 && n < 8) {
        pp = g_unk0x00537f3c + n;
        do {
            FUN_0046d2a0((int *)*pp);
            Replay_StopRecording(*pp);
            pp++;
        } while ((int)pp < (int)(g_unk0x00537f3c + 8)); // 0x537f5c in the original
    }
    if ((char)RallyData_FUN_00407ea0() != 0 && (char)CGameInfo::FUN_00406310() != 0)
        FUN_00466080();
    if ((char)RallyData_FUN_00407ea0() == 0 || (char)CGameInfo::FUN_00406310() == 0)
        return;
    for (i = 0; i < (BYTE)RallyDataState(); i++)
        FUN_004660a0((int **)g_unk0x00537f3c + i, i, (BYTE)param2 + i);
    return;
L_stage_restore:
    if (RallyDataStageIndex() == 2 || RallyDataStageIndex() == 4 ||
        RallyDataStageIndex() == 6 || RallyDataStageIndex() == 8 || RallyDataStageIndex() == 10)
        CGame::FUN_004057e0(2);
L_restore:
    FUN_0041e670();
    FUN_0041b300();
    return;
L_winner:
    CGame::FUN_0049c1c0((Unk0049c2c0 *)pPlayers, 0, 0, 2);
L_teardown2:
    CGame::FUN_004057e0(0);
    FUN_0041e670();
    FUN_0041b300();
}

// GLOBAL: CMR2 0x005196e8
char g_strBspFormat[] = "%s.bsp";
// GLOBAL: CMR2 0x00519498
char g_strHpcFormat[] = "%s.hpc";

// GLOBAL: CMR2 0x005194a0
char g_strSrfFormat[] = "%s.srf";

// Loads the stage's surface list (.srf) and marks the sound groups it uses.
// FUNCTION: CMR2 0x0041fcd0
void FUN_0041fcd0(void)
{
    DWORD size = 0;
    BYTE *pData;

    sprintf(CFrontend::m_stringDest, g_strSrfFormat, FUN_0041f900());
    pData = (BYTE *)CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(),
                                                 CFrontend::m_stringDest, 0, &size, 0);
    if (pData != NULL) {
        FUN_0041afe0(pData, size);
        return;
    }
    FUN_0041afe0(NULL, 0);
}

// Loads the stage's .bsp (stage light placement).
// FUNCTION: CMR2 0x00420020
void FUN_00420020(void)
{
    sprintf(CFrontend::m_stringDest, g_strBspFormat, FUN_0041f900());
    StageLights_SetTransform((FixVector *)CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(),
                                                                     CFrontend::m_stringDest, 0, 0, 0));
}

// Loads the stage's .hpc data when present.
// FUNCTION: CMR2 0x0041fc90
void FUN_0041fc90(void)
{
    BYTE *pData;

    sprintf(CFrontend::m_stringDest, g_strHpcFormat, FUN_0041f900());
    pData = (BYTE *)CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(), CFrontend::m_stringDest,
                                                 0, 0, 0);
    if (pData != NULL)
        FUN_00490d50(pData);
}

int Sound_IsPlaying(unsigned int handle);
int FUN_004b7790(unsigned short id, int volume, int frequency, int loopStart, int loops, int is3D);
extern int g_unk0x00537194;
void FUN_004b79a0(unsigned int handle, int volume);
void Sound_Free(unsigned int handle);

// Moves sound slot src of a car's sound set to slot dst.
// FUNCTION: CMR2 0x00418e20
void FUN_00418e20(int set, int dst, int src)
{
    g_carSoundSets[set].id[dst] = g_carSoundSets[set].id[src];
    g_carSoundSets[set].handle[dst] = g_carSoundSets[set].handle[src];
    g_carSoundSets[set].handle[src] = -1;
    g_carSoundSets[set].id[src] = -1;
}

// Car speed as a 16.16 fraction of 120 (speed units clamped to 0..120).
// FUNCTION: CMR2 0x00418e70
int FUN_00418e70(int car)
{
    int speed = FixMul(Car_Get(car)->speed, 0x431168) >> 16;

    if (speed < 0)
        speed = 0;
    else if (speed > 120)
        speed = 120;
    return FixDiv((int)(__int64)(speed * CGraphics::m_65536), 0x780000);
}

// Silences every stage sound still playing.
// FUNCTION: CMR2 0x00418ee0
void FUN_00418ee0(void)
{
    CarSoundSet *pSet;
    int *pHandle;
    int i;

    pSet = g_carSoundSets;
    do {
        pHandle = pSet->handle;
        for (i = 10; i != 0; i--) {
            if (Sound_IsPlaying(*pHandle) != 0)
                FUN_004b79a0(*pHandle, 0);
            pHandle++;
        }
        pSet++;
    } while ((int)pSet < (int)&g_carSoundSets[8]);
}

// Loads a sound sample by the name held in the caller's buffer, reading it
// from the current stage file (the same file the stage timing code uses).
// FUNCTION: CMR2 0x00418760
void FUN_00418760(char *name)
{
    Sound_LoadSample(name, 0, (GenericFile *)StageTiming_GetStageFile2());
}

// One sound handle per car.
// GLOBAL: CMR2 0x005373b0
int g_carSounds[8];

// Per-slot sound volumes of each car and the largest of them, the pitch of the
// engine sound just restarted (0x5374c0) and the id of the car's engine
// sample used while a stage sound group is not running.
// GLOBAL: CMR2 0x005374c0
int g_unk0x005374c0;
// GLOBAL: CMR2 0x005374c4
int g_carSlotVolumes[8][4];
// GLOBAL: CMR2 0x00537544
int g_carMaxVolume[8];
// GLOBAL: CMR2 0x00537564
int g_unk0x00537564;

// Per-car-class fixed-point offsets used when the transforms of a car are
// rebuilt (0x42af50): the base one and the extra applied while the class of the
// car changes.
// GLOBAL: CMR2 0x005199c8
int g_unk0x005199c8[14] = {
    -1310, 1310, -655, -2621, 0, 655, 0, 1310,
    655, 655, 0, 0, 1310
};
// GLOBAL: CMR2 0x00519a00
int g_unk0x00519a00[14] = {
    0, -2293, -983, -1638, -3932, -2883, -1638, -2621,
    -5505, -1769, -12910, -2293, -4718, -4718
};

// Frees the per-car sound of every car in the race.
// FUNCTION: CMR2 0x00418780
void FUN_00418780(void)
{
    int *p;
    int i;

    i = 0;
    if ((BYTE)RallyDataState() > 0) {
        p = g_carSounds;
        do {
            if (Sound_IsPlaying(*p) != 0) {
                Sound_Free(*p);
                *p = -1;
            }
            i++;
            p++;
        } while (i < (int)(RallyDataState() & 0xff));
    }
}

// FUNCTION: CMR2 0x00420130
void FUN_00420130(int value)
{
    g_unk0x00538970 = value;
    g_raceCallbackMark = CGame::GetCallbackCount();
}

// GLOBAL: CMR2 0x00517ed4
char g_strCodFormat[] = "%s.cod";
// GLOBAL: CMR2 0x00537094
void *g_unk0x00537094;

// Loads the stage's co-driver calls (.cod) and resets the call state.
// FUNCTION: CMR2 0x00416720
void FUN_00416720(void)
{
    CGame::RegisterCallback(FUN_00418550, 0);
    FUN_00416670();
    sprintf(CFrontend::m_stringDest, g_strCodFormat, FUN_0041f900());
    g_unk0x00537094 = CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(), CFrontend::m_stringDest, 0, 0, 0);
    if (g_unk0x00537094 != NULL)
        g_unk0x00537358 = (int)g_unk0x00537094;
}

// GLOBAL: CMR2 0x00537364
int g_unk0x00537364[4];
// GLOBAL: CMR2 0x00537374
int g_unk0x00537374[4];
// GLOBAL: CMR2 0x00537384
int g_unk0x00537384[4];
// GLOBAL: CMR2 0x00537398
int g_unk0x00537398[4];

int FUN_00427d50(unsigned int view, int listener);
void FUN_004b79a0(unsigned int handle, int volume);

// GLOBAL: CMR2 0x005189bc
char g_str0x005189bc[] = "COLLIS6.WAV";
// GLOBAL: CMR2 0x005189c8
char g_str0x005189c8[] = "COLLIS5.WAV";
// GLOBAL: CMR2 0x005189d4
char g_str0x005189d4[] = "COLLIS4.WAV";
// GLOBAL: CMR2 0x005189e0
char g_str0x005189e0[] = "COLLIS3.WAV";
// GLOBAL: CMR2 0x005189ec
char g_str0x005189ec[] = "COLLIS2.WAV";
// GLOBAL: CMR2 0x005189f8
char g_str0x005189f8[] = "COLLIS1.WAV";
// GLOBAL: CMR2 0x00518a04
char g_str0x00518a04[] = "SWIPE4.WAV";
// GLOBAL: CMR2 0x00518a10
char g_str0x00518a10[] = "SWIPE3.WAV";
// GLOBAL: CMR2 0x00518a1c
char g_str0x00518a1c[] = "SWIPE2.WAV";
// GLOBAL: CMR2 0x00518a28
char g_str0x00518a28[] = "SWIPE1.WAV";
// GLOBAL: CMR2 0x005188e4
char *g_collisionSounds[10] = {
    g_str0x00518a28, g_str0x00518a1c, g_str0x00518a10, g_str0x00518a04, g_str0x005189f8, g_str0x005189ec, g_str0x005189e0, g_str0x005189d4, g_str0x005189c8, g_str0x005189bc,
};
// GLOBAL: CMR2 0x00518968
char g_str0x00518968[] = "LAND3.WAV";
// GLOBAL: CMR2 0x00518974
char g_str0x00518974[] = "LAND2.WAV";
// GLOBAL: CMR2 0x00518980
char g_str0x00518980[] = "LAND1.WAV";
// GLOBAL: CMR2 0x0051898c
char g_str0x0051898c[] = "ROLL4.WAV";
// GLOBAL: CMR2 0x00518998
char g_str0x00518998[] = "ROLL3.WAV";
// GLOBAL: CMR2 0x005189a4
char g_str0x005189a4[] = "ROLL2.WAV";
// GLOBAL: CMR2 0x005189b0
char g_str0x005189b0[] = "ROLL1.WAV";
// GLOBAL: CMR2 0x0051890c
char *g_rollSounds[7] = {
    g_str0x005189b0, g_str0x005189a4, g_str0x00518998, g_str0x0051898c, g_str0x00518980, g_str0x00518974, g_str0x00518968,
};
// GLOBAL: CMR2 0x00518938
char g_str0x00518938[] = "MIRROR.WAV";
// GLOBAL: CMR2 0x00518944
char g_str0x00518944[] = "SCREEN1.WAV";
// GLOBAL: CMR2 0x00518950
char g_str0x00518950[] = "HLIGHT2.WAV";
// GLOBAL: CMR2 0x0051895c
char g_str0x0051895c[] = "HLIGHT1.WAV";
// GLOBAL: CMR2 0x00518928
char *g_glassSounds[4] = {
    g_str0x0051895c, g_str0x00518950, g_str0x00518944, g_str0x00518938,
};
// GLOBAL: CMR2 0x00518a34
char g_strCollisionDir[] = "\\collision\\";

int FUN_004b7940(void);
extern int g_unk0x00537360;
extern int g_unk0x005373a8;
extern int g_unk0x005373ac;
BOOL Sound_LoadSample(char *name, BYTE flags, GenericFile *pFile);

// Loads the collision sound banks of the stage (swipes and impacts, rolls and
// landings, glass) and resets the per-car sound slots.
// FUNCTION: CMR2 0x004188c0
void FUN_004188c0(void)
{
    char path[260];
    char *pDir;
    GenericFile *pFile;
    int i;

    pDir = CInstallInfo::GetSoundsDir();
    pFile = (GenericFile *)StageTiming_GetStageFile0();
    g_unk0x00537360 = FUN_004b7940();
    for (i = 0; i < 10; i++) {
        strcpy(path, pDir);
        strcat(path, g_strCollisionDir);
        strcat(path, g_collisionSounds[i]);
        Sound_LoadSample(path, 0, pFile);
    }
    g_unk0x005373ac = FUN_004b7940();
    for (i = 0; i < 7; i++) {
        strcpy(path, pDir);
        strcat(path, g_strCollisionDir);
        strcat(path, g_rollSounds[i]);
        Sound_LoadSample(path, 0, pFile);
    }
    g_unk0x005373a8 = FUN_004b7940();
    for (i = 0; i < 4; i++) {
        strcpy(path, pDir);
        strcat(path, g_strCollisionDir);
        strcat(path, g_glassSounds[i]);
        Sound_LoadSample(path, 0, pFile);
    }
    for (i = 0; i < 4; i++)
        g_carSounds[i] = -1;
}

// Updates the volume of each player's car sound by distance to its listener.
// FUNCTION: CMR2 0x00418b00
void FUN_00418b00(Unk0049c2c0 *p, BYTE index)
{
    int i;

    for (i = 0; i < (BYTE)RallyDataState(); i++) {
        if (g_carSounds[i] != -1) {
            if (Sound_IsPlaying(g_carSounds[i]) != 0)
                FUN_004b79a0(g_carSounds[i], FixMul(FUN_00427d50(g_unk0x00537398[i], g_unk0x00537364[i]),
                                                    FixMul(g_unk0x00537394, g_unk0x00537384[i])));
            else
                g_carSounds[i] = -1;
        }
    }
}

void FUN_0040bad0(void);
struct DeviceInfo;
void FUN_0040bd60(unsigned short slot, DeviceInfo *pOut);
BYTE *FUN_00475f70(void);
int FUN_0041f410(void);

// Update handler of the pause state (state table 0x5190b0): runs the pause
// menu until it closes.
// FUNCTION: CMR2 0x0041f420
void FUN_0041f420(Unk0049c2c0 *p, BYTE index)
{
    DeviceInfo *pDev;

    if (g_unk0x00538108 != 0) {
        if (FUN_0041f410()) {
            CGameInfo::FUN_0049ea90(0);
            CGame::FUN_0049c1c0(p, index, 0, 2);
            return;
        }
        g_unk0x00538108 = 0;
        return;
    }
    CGameInfo::FUN_0049ea90(1);
    CInput::FUN_0049eab0();
    FUN_0040bad0();
    pDev = CInput::FUN_0049ead0(0);
    FUN_0040bd60(0, pDev);
    Menu_Update((Menu *)FUN_00475f70(), pDev->field_0x8);
    if (g_unk0x00537f94 != 0)
        CGame::FUN_0049c1c0(p, index, 1, 2);
}

// Plays a car sound in a free slot (or the oldest one), with its volume by
// distance to the listener.
// FUNCTION: CMR2 0x004187d0
void FUN_004187d0(unsigned int view, unsigned short id, int volume, int listener)
{
    unsigned int oldest = 0;
    unsigned int now = CMain::GetFrameDelta();
    int slot = -1;
    int i;

    for (i = 0; i < 4; i++) {
        if (g_carSounds[i] == -1) {
            slot = i;
            break;
        }
    }
    if (slot == -1) {
        slot = 0;
        for (i = 0; i < 4; i++) {
            if (now - g_unk0x00537374[i] > oldest) {
                slot = i;
                oldest = now - g_unk0x00537374[i];
            }
        }
    }
    if (g_carSounds[slot] != -1 && Sound_IsPlaying(g_carSounds[slot]))
        Sound_Free(g_carSounds[slot]);
    g_carSounds[slot] =
        FUN_004b7790(id, FixMul(FUN_00427d50(view, listener), FixMul(g_unk0x00537394, volume)), 0xac44, 0, 0, 0);
    g_unk0x00537364[slot] = listener;
    g_unk0x00537374[slot] = now;
    g_unk0x00537384[slot] = volume;
    g_unk0x00537398[slot] = view;
}

// GLOBAL: CMR2 0x00537360
int g_unk0x00537360;
// First sample of the impact, scrape and horn sound groups.
// GLOBAL: CMR2 0x005373a8
int g_unk0x005373a8;
// GLOBAL: CMR2 0x005373ac
int g_unk0x005373ac;

void FUN_004187d0(unsigned int view, unsigned short id, int volume, int listener);

// Raises the car's damage shake level by the strength of a hit.
#define CAR_SHAKE(view, strength)                                         \
    do {                                                                  \
        int level = (FixMul(strength, 0x70000) >> 16) + 1;                \
        if (level > 8)                                                    \
            level = 8;                                                    \
        if (level > Car_Get(view)->shakeLevel)                        \
            Car_Get(view)->shakeLevel = (BYTE)level;                  \
    } while (0)

// Plays a random impact sound (light or heavy set) and shakes the car.
// FUNCTION: CMR2 0x00418c30
void FUN_00418c30(unsigned int view, int volume, char heavy, int listener)
{
    if (heavy != 0)
        FUN_004187d0(view, (unsigned short)(rand() % 3 + 4 + g_unk0x005373ac), volume, listener);
    else
        FUN_004187d0(view, (unsigned short)(rand() % 4 + g_unk0x005373ac), volume, listener);
    CAR_SHAKE(view, volume);
}

// Plays the scrape sound for its strength (10 levels) and shakes the car.
// match 84%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00418ba0
void FUN_00418ba0(int view, int strength, int listener)
{
    int level;

    if (strength > 0xccc) {
        level = FixMul(strength, 0xa0000) >> 16;
        if (level >= 10)
            level = 9;
        FUN_004187d0(view, (unsigned short)(g_unk0x00537360 + level), 0x10000, listener);
        {
            // The do/while(0) form of CAR_SHAKE shifts MSVC6's register
            // allocation of the prologue; the original expands it inline.
            int shake = (FixMul(strength, 0x70000) >> 16) + 1;
            if (shake > 8)
                shake = 8;
            if (shake > Car_Get(view)->shakeLevel)
                Car_Get(view)->shakeLevel = (BYTE)shake;
        }
    }
}

// Plays one of the three horn sounds (random for kind 0).
// FUNCTION: CMR2 0x00418cd0
void FUN_00418cd0(unsigned int view, int kind, int listener)
{
    int sound = 0;

    switch (kind) {
    case 0: sound = rand() % 2; break;
    case 2: sound = 2; break;
    }
    FUN_004187d0(view, (unsigned short)(g_unk0x005373a8 + sound), 0x10000, listener);
}

void FUN_00463ce0(BYTE value);

// GLOBAL: CMR2 0x00537350
int g_unk0x00537350;
// GLOBAL: CMR2 0x0053735c
int g_unk0x0053735c;

// Updates the current route block and queues its first callout.
// match 59%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00416f70
void FUN_00416f70(int player)
{
    int remaining = 500 - FUN_0041d780();
    int block = remaining / 100;

    g_unk0x00537350 = -1;
    g_unk0x0053708c[player] = RallyData_FUN_00421370((BYTE *)Car_Get(player));
    if ((BYTE)RallyData_FUN_00407e70()) {
        if (remaining % 100 < 20)
            g_unk0x00537350 = 6;
        else if (block <= 4)
            g_unk0x00537350 = block + 1;
        else
            g_unk0x00537350 = -1;
    }
    RaceCalls_UpdatePlaybackQueue();
    // the original reads [slot*4 + 0x537190], i.e. this same table one player back
    if (player > 0 && g_unk0x005371a4[block + player * 5 - 5] != 0) {
        g_unk0x005371a4[block + player * 5] = 1;
        return;
    }
    if (player == 0 && g_unk0x005371a4[block] == 0) {
        g_unk0x005371a4[block] = 1;
        FUN_00463ce0(block + 1);
        if (block == 2)
            Race_AssignUnusedSlot(g_unk0x0053735c + 0x38);
        else if (block == 1)
            Race_AssignUnusedSlot(g_unk0x0053735c + 0x39);
        else if (block == 0)
            Race_AssignUnusedSlot(g_unk0x0053735c + 0x3a);
    }
    g_unk0x0053708c[player] = RallyData_FUN_00421370((BYTE *)Car_Get(player)) - 1;
}

int FUN_00422f50(BYTE index);
void FUN_004ae410(BYTE a, BYTE b, int c, int d);

// Advances one player's "menu" cursor when the device that owns it (arg = the
// player's input slot) pressed the up/down/left/right buttons: the odd modes
// scroll the mode list, the even ones step the stage index, and the shared
// tail replays the two beeper sounds.
// match 66.87%: implementada; MSVC6 genera la guarda del tamano (`test edi,edi`) una sola vez
// donde el original la repite, y usa setne en vez de sub/neg/sbb para el `x != 7`.
// FUNCTION: CMR2 0x0041d0c0
void FUN_0041d0c0(int param_1)
{
    int flags;
    int i;
    BYTE value;

    if (FUN_0041f3d0((BYTE)param_1) == 0)
        return;
    flags = (int)CInput::FUN_0049ead0(param_1)->field_0x8;
    if (FUN_0041f3a0() != 0) {
        if (param_1 != 0 && FUN_0041f3d0(0) != 0)
            return;
        if (flags & 0x1000) {
            if (CGameInfo::FUN_00405d80() == 2 || CGameInfo::FUN_00405d80() == 1 ||
                CGameInfo::FUN_00405d80() == 0)
                return;
            View_SwitchCamera(1, (FUN_00422f50(1) == 7 ? 4 : 7), 0xffff, FUN_00422fb0(1), 0);
            FUN_004ae410(0, 1, 1, 1);
            FUN_004ae410(1, 1, 1, 1);
            return;
        }
        if ((flags & 0x2000) != 0) {
            value = (BYTE)(FUN_00422fb0(1) + 1);
            if ((BYTE)Race_GetBaseCarCount() <= value)
                value = 0;
            View_SetCameraType(1, 0, value, 0);
            FUN_004ae410(0, 1, 1, 1);
            FUN_004ae410(1, 1, 1, 1);
        }
        return;
    }
    if (flags & 0x1000) {
        i = 0;
        if ((BYTE)Race_GetBaseCarCount() > 0) {
            do {
                FUN_004ae410((BYTE)i, 0, 1, 1);
                i++;
            } while (i < (int)((BYTE)Race_GetBaseCarCount()));
        }
        View_SwitchCamera((BYTE)param_1, (-(FUN_00422f50((BYTE)param_1) != 7) & 3) + 4, 0xffff,
                     FUN_00422fb0((BYTE)param_1), 0);
        return;
    }
    if ((flags & 0x2000) != 0 && (BYTE)Race_GetBaseCarCount() > 1 && (BYTE)RallyDataState() == 1) {
        value = (BYTE)(FUN_00422fb0(0) + 1);
        if (value >= (BYTE)Race_GetBaseCarCount())
            value = 0;
        View_SetCameraType(0, 0, value, 0);
        i = 0;
        if ((BYTE)Race_GetBaseCarCount() > 0) {
            do {
                FUN_004ae410((BYTE)i, 0, 1, 1);
                i++;
            } while (i < (int)((BYTE)Race_GetBaseCarCount()));
        }
    }
}

unsigned short FUN_00407650(void);
unsigned short FUN_00407710(void);
void RallyData_FUN_00407800(unsigned int param1);
void FUN_004918d0(void);
void FUN_00461a30(unsigned short timePrimary, unsigned short timeSecondary, BYTE **records, BYTE **pPrimary,
                  BYTE **pSecondary);
void FUN_00461a70(BYTE *pA, BYTE *pB);


// GLOBAL: CMR2 0x005196dc
char g_str0x005196dc[12] = "0500.hor";

// GLOBAL: CMR2 0x005196d0
char g_str0x005196d0[12] = "0600.hor";

// GLOBAL: CMR2 0x005196c4
char g_str0x005196c4[12] = "0800.hor";

// GLOBAL: CMR2 0x005196b8
char g_str0x005196b8[12] = "1000.hor";

// GLOBAL: CMR2 0x005196ac
char g_str0x005196ac[12] = "1200.hor";

// GLOBAL: CMR2 0x005196a0
char g_str0x005196a0[12] = "1500.hor";

// GLOBAL: CMR2 0x00519694
char g_str0x00519694[12] = "1700.hor";

// GLOBAL: CMR2 0x00519688
char g_str0x00519688[12] = "1800.hor";

// GLOBAL: CMR2 0x0051967c
char g_str0x0051967c[12] = "1900.hor";

// GLOBAL: CMR2 0x00519670
char g_str0x00519670[12] = "2000.hor";

// GLOBAL: CMR2 0x00519664
char g_str0x00519664[12] = "2200.hor";

// GLOBAL: CMR2 0x00519658
char g_str0x00519658[12] = "2400.hor";

// GLOBAL: CMR2 0x0051964c
char g_str0x0051964c[12] = "0500CLO.hor";

// GLOBAL: CMR2 0x00519640
char g_str0x00519640[12] = "0600CLO.hor";

// GLOBAL: CMR2 0x00519634
char g_str0x00519634[12] = "0800CLO.hor";

// GLOBAL: CMR2 0x00519628
char g_str0x00519628[12] = "1000CLO.hor";

// GLOBAL: CMR2 0x0051961c
char g_str0x0051961c[12] = "1200CLO.hor";

// GLOBAL: CMR2 0x00519610
char g_str0x00519610[12] = "1500CLO.hor";

// GLOBAL: CMR2 0x00519604
char g_str0x00519604[12] = "1700CLO.hor";

// GLOBAL: CMR2 0x005195f8
char g_str0x005195f8[12] = "1800CLO.hor";

// GLOBAL: CMR2 0x005195ec
char g_str0x005195ec[12] = "1900CLO.hor";

// GLOBAL: CMR2 0x005195e0
char g_str0x005195e0[12] = "2000CLO.hor";

// GLOBAL: CMR2 0x005195d4
char g_str0x005195d4[12] = "2200CLO.hor";

// GLOBAL: CMR2 0x005195c8
char g_str0x005195c8[12] = "2400CLO.hor";

// GLOBAL: CMR2 0x005195bc
char g_str0x005195bc[12] = "0500STO.hor";

// GLOBAL: CMR2 0x005195b0
char g_str0x005195b0[12] = "0600STO.hor";

// GLOBAL: CMR2 0x005195a4
char g_str0x005195a4[12] = "0800STO.hor";

// GLOBAL: CMR2 0x00519598
char g_str0x00519598[12] = "1000STO.hor";

// GLOBAL: CMR2 0x0051958c
char g_str0x0051958c[12] = "1200STO.hor";

// GLOBAL: CMR2 0x00519580
char g_str0x00519580[12] = "1500STO.hor";

// GLOBAL: CMR2 0x00519574
char g_str0x00519574[12] = "1700STO.hor";

// GLOBAL: CMR2 0x00519568
char g_str0x00519568[12] = "1800STO.hor";

// GLOBAL: CMR2 0x0051955c
char g_str0x0051955c[12] = "1900STO.hor";

// GLOBAL: CMR2 0x00519550
char g_str0x00519550[12] = "2000STO.hor";

// GLOBAL: CMR2 0x00519544
char g_str0x00519544[12] = "2200STO.hor";

// GLOBAL: CMR2 0x00519538
char g_str0x00519538[12] = "2400STO.hor";

// GLOBAL: CMR2 0x0051952c
char g_str0x0051952c[12] = "0500BLI.hor";

// GLOBAL: CMR2 0x00519520
char g_str0x00519520[12] = "0600BLI.hor";

// GLOBAL: CMR2 0x00519514
char g_str0x00519514[12] = "0800BLI.hor";

// GLOBAL: CMR2 0x00519508
char g_str0x00519508[12] = "1000BLI.hor";

// GLOBAL: CMR2 0x005194fc
char g_str0x005194fc[12] = "1200BLI.hor";

// GLOBAL: CMR2 0x005194f0
char g_str0x005194f0[12] = "1500BLI.hor";

// GLOBAL: CMR2 0x005194e4
char g_str0x005194e4[12] = "1700BLI.hor";

// GLOBAL: CMR2 0x005194d8
char g_str0x005194d8[12] = "1800BLI.hor";

// GLOBAL: CMR2 0x005194cc
char g_str0x005194cc[12] = "1900BLI.hor";

// GLOBAL: CMR2 0x005194c0
char g_str0x005194c0[12] = "2000BLI.hor";

// GLOBAL: CMR2 0x005194b4
char g_str0x005194b4[12] = "2200BLI.hor";

// GLOBAL: CMR2 0x005194a8
char g_str0x005194a8[12] = "2400BLI.hor";


// Handles of the resolved stage texture records.
// GLOBAL: CMR2 0x00538234
BYTE *g_unk0x00538234;
// GLOBAL: CMR2 0x00538238
BYTE *g_unk0x00538238;

// Resolves the 48 stage texture names through the generic file loader, builds
// the six palette entries of the stage geometry from the primary/secondary
// colour records and refreshes the stage colour ramps.
// FUNCTION: CMR2 0x0041fd30
void FUN_0041fd30(void)
{
    char *pNames[48];
    int handles[48];
    BYTE colour[6];
    BYTE *pPrimary;
    BYTE *pSecondary;
    int i;
    int avg;

    pNames[0] = g_str0x005196dc;
    pNames[1] = g_str0x005196d0;
    pNames[2] = g_str0x005196c4;
    pNames[3] = g_str0x005196b8;
    pNames[4] = g_str0x005196ac;
    pNames[5] = g_str0x005196a0;
    pNames[6] = g_str0x00519694;
    pNames[7] = g_str0x00519688;
    pNames[8] = g_str0x0051967c;
    pNames[9] = g_str0x00519670;
    pNames[10] = g_str0x00519664;
    pNames[11] = g_str0x00519658;
    pNames[12] = g_str0x0051964c;
    pNames[13] = g_str0x00519640;
    pNames[14] = g_str0x00519634;
    pNames[15] = g_str0x00519628;
    pNames[16] = g_str0x0051961c;
    pNames[17] = g_str0x00519610;
    pNames[18] = g_str0x00519604;
    pNames[19] = g_str0x005195f8;
    pNames[20] = g_str0x005195ec;
    pNames[21] = g_str0x005195e0;
    pNames[22] = g_str0x005195d4;
    pNames[23] = g_str0x005195c8;
    pNames[24] = g_str0x005195bc;
    pNames[25] = g_str0x005195b0;
    pNames[26] = g_str0x005195a4;
    pNames[27] = g_str0x00519598;
    pNames[28] = g_str0x0051958c;
    pNames[29] = g_str0x00519580;
    pNames[30] = g_str0x00519574;
    pNames[31] = g_str0x00519568;
    pNames[32] = g_str0x0051955c;
    pNames[33] = g_str0x00519550;
    pNames[34] = g_str0x00519544;
    pNames[35] = g_str0x00519538;
    pNames[36] = g_str0x0051952c;
    pNames[37] = g_str0x00519520;
    pNames[38] = g_str0x00519514;
    pNames[39] = g_str0x00519508;
    pNames[40] = g_str0x005194fc;
    pNames[41] = g_str0x005194f0;
    pNames[42] = g_str0x005194e4;
    pNames[43] = g_str0x005194d8;
    pNames[44] = g_str0x005194cc;
    pNames[45] = g_str0x005194c0;
    pNames[46] = g_str0x005194b4;
    pNames[47] = g_str0x005194a8;
    for (i = 0; i < 48; i++)
        handles[i] = (int)CGenericFileLoader::FindFile((GenericFile *)StageTiming_GetStageFile3(),
                                                       pNames[i], NULL, NULL, 0);
    FUN_00461a30(FUN_00407650(), FUN_00407710(), (BYTE **)handles, &g_unk0x00538238,
                 &g_unk0x00538234);
    FUN_00461a70(g_unk0x00538238, g_unk0x00538234);
    if (g_unk0x00538238 != NULL && g_unk0x00538234 != NULL) {
        pPrimary = g_unk0x00538234;
        pSecondary = g_unk0x00538238;
        for (i = 0; i < 6; i++) {
            avg = ((int)pSecondary[0x48 + i] + (int)pPrimary[0x48 + i]) / 2;
            if (avg > 0xff)
                avg = 0xff;
            colour[i] = (BYTE)avg;
        }
        CGraphics::FUN_004a5ff0((BYTE)*(int *)((BYTE *)colour + 0));
        CGraphics::FUN_004a6010((BYTE)*(int *)((BYTE *)colour + 1));
        CGraphics::FUN_004a6040((BYTE)*(int *)((BYTE *)colour + 2));
        CGraphics::FUN_004a6060((BYTE)*(int *)((BYTE *)colour + 3));
        CGraphics::FUN_004a6080((BYTE)*(int *)((BYTE *)colour + 4));
        CGraphics::FUN_004a60b0((BYTE)*(int *)((BYTE *)colour + 5));
        if ((unsigned short)FUN_00407650() >= 0x834) {
            RallyData_FUN_00407800(1);
            FUN_004918d0();
            return;
        }
        RallyData_FUN_00407800(0);
        FUN_004918d0();
    }
}

// Route helpers of the per-car update below (no shared header declares them).
int RallyData_FUN_00421420(void);
BYTE *RallyData_FUN_00421440(int index);
int RallyData_FUN_00421470(BYTE *p);
BYTE FUN_00458270(int index);
int FUN_00448110(void);
void FUN_00421570(unsigned int nodeIndex, FixVector *pOut);

// Set while the node scan near the car's position finds a node whose callouts
// have not been delivered yet.
// GLOBAL: CMR2 0x00537354
int g_unk0x00537354;

// Per-car route/timing update. Keeps the tracked route position
// g_unk0x0053708c[] in sync with the car's node (best, from RallyData), raises
// the wrong-way flag while the node direction projected on the
// front wheel rolling direction points backwards, scans the nodes around the
// position for pending callouts (FUN_004174e0) and, when the leading call of
// this car expires, shifts its five call records g_raceCallRecords[car*5].
// match 50%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// Implementada; en la cola el original relee un campo redundante de g_raceCallRecords[car*5+4] que no transcribimos, y el reparto de bloques del switch difiere.
// FUNCTION: CMR2 0x00417090
void FUN_00417090(int param_1)
{
    int cur;
    int best;
    int prev;
    int node;
    int n;
    int dot;
    int i;
    unsigned int cnt;
    FixVector dir;
    Car *pCar;
    unsigned int flags;
    BYTE callId;
    BYTE prevCallId;
    RaceCallRecord *pRec;

    if (RallyData_FUN_00421420() == 0)
        return;

    if ((char)RallyData_FUN_00407ea0() != '\0' &&
        ((char)RallyData_GetFlag25() != '\0' || (char)RallyData_GetFlag24() != '\0') &&
        (char)FUN_00458270(param_1) != '\0') {
        g_unk0x0053708c[param_1] = 0;
    }

    cur = g_unk0x0053708c[param_1];
    best = RallyData_FUN_00421370((BYTE *)Car_Get(param_1));

    if (g_unk0x005371a0 == 0 && param_1 == 0) {
        FUN_00463ce0(0);
        Race_AssignUnusedSlot(g_unk0x0053735c + 0xd);
        g_unk0x005371a0 = 1;
    }

    if ((char)RallyData_FUN_00407e70() != '\0') {
        i = FUN_00448110();
        g_unk0x00537350 = (i < 0x5a) - 1;
    }

    RaceCalls_UpdatePlaybackQueue();

    if (best < cur - 0xc)
        g_unk0x0053708c[param_1] = best;

    if (best < cur - 1 &&
        ((int)((unsigned int)RallyData_FUN_00421420() >> 1) <= best ||
         cur <= (int)((unsigned int)RallyData_FUN_00421420() >> 1))) {
        FUN_00421570(best, &dir);
        pCar = Car_Get(param_1);
        dot = FixVecDot(&pCar->rearWheelDir, &dir);
        if (dot < -0x8000) {
            g_raceWrongWayFlags[param_1] = 1;
            g_unk0x00537198[param_1] = best;
        }
    }

    if ((int)((unsigned int)RallyData_FUN_00421420() >> 1) < best &&
        cur < (int)((unsigned int)RallyData_FUN_00421420() >> 1)) {
        g_unk0x00537198[param_1] = best;
        g_unk0x0053708c[param_1] = best;
        cur = best;
    }

    if (g_raceWrongWayFlags[param_1] != 0) {
        FUN_00421570(best, &dir);
        pCar = Car_Get(param_1);
        dot = FixVecDot(&pCar->rearWheelDir, &dir);
        if (dot > 0x3333)
            g_raceWrongWayFlags[param_1] = 0;
    }

    if (g_unk0x00537198[param_1] < best)
        g_raceWrongWayFlags[param_1] = 0;

    g_unk0x00537354 = 0;
    i = g_unk0x0053708c[param_1] + 1;
    if (i <= g_unk0x0053708c[param_1] + 6) {
        do {
            cnt = (unsigned int)RallyData_FUN_00421420();
            if ((int)cnt <= i || *(char *)(RallyData_FUN_00421440(i) + 0x19) != -1)
                g_unk0x00537354 = 1;
            i++;
        } while (i <= g_unk0x0053708c[param_1] + 6);
    }

    if (*(char *)(*(int *)(FUN_0041b390() + 4) + param_1 * 8) == '\t')
        g_unk0x00537354 = 1;

    if ((g_raceCallRecords[param_1 * 5 + 1].flags & 0x100) != 0)
        g_unk0x00537354 = 1;

    if (FUN_00417760(param_1) == 0 &&
        *(char *)(*(int *)(FUN_0041b390() + 4) + param_1 * 8) == '\b') {
        if (!((char)RallyData_GetFlag25() != '\0' &&
              RallyData_FUN_00421470((BYTE *)Car_Get(param_1)) >= 0x10000)) {
            if (g_unk0x0053708c[param_1] < best) {
                node = cur + 1;
                if (node <= best) {
                    prev = cur + 2;
                    do {
                        n = (int)RallyData_FUN_00421440(node);
                        if (*(char *)(n + 0x19) != -1 &&
                            FUN_004174d0() != 0 && FUN_00417760(param_1) == 0) {
                            int ok = 1;
                            if (prev >= 2) {
                                n = (int)RallyData_FUN_00421440(node - 1);
                                if (*(char *)(n + 0x19) != -1) {
                                    ok = 0;
                                } else {
                                    callId = *(BYTE *)(RallyData_FUN_00421440(node) + 0x19);
                                    prevCallId = *(BYTE *)(RallyData_FUN_00421440(prev) + 0x19);
                                }
                            } else {
                                callId = *(BYTE *)(RallyData_FUN_00421440(node) + 0x19);
                                prevCallId = *(BYTE *)(RallyData_FUN_00421440(prev) + 0x19);
                            }
                            if (ok)
                                FUN_004174e0(param_1, callId, prevCallId, (BYTE)node);
                        }
                        node++;
                        prev++;
                    } while (node <= best);
                }
                g_unk0x0053708c[param_1] = best;
            }
        }
    }

    if (best < g_unk0x00537198[param_1])
        g_unk0x00537198[param_1] = best;
    if (g_unk0x00537198[param_1] + 6 < best)
        g_unk0x00537198[param_1] = best - 6;

    if (FUN_004174d0() == 0)
        return;
    flags = g_raceCallRecords[param_1 * 5].flags;
    if ((flags & 0x100) == 0)
        return;
    if ((BYTE)flags == 0x19) {
        if (g_unk0x00537354 != 0)
            g_raceCallRecords[param_1 * 5].flags = (flags - 1 ^ flags) & 0xff ^ flags;
    } else {
        g_raceCallRecords[param_1 * 5].flags = (flags - 1 ^ flags) & 0xff ^ flags;
    }
    if ((BYTE)g_raceCallRecords[param_1 * 5].flags == 0) {
        pRec = &g_raceCallRecords[param_1 * 5];
        for (i = 0; i < 4; i++)
            pRec[i] = pRec[i + 1];
        pRec[4].flags &= 0xfffffc00;
        pRec[4].field_0x0 = 0;
        pRec[4].field_0x4 = 0;
    }
}

void Car_InterpolateRenderTransforms(void);
void FUN_00484d30(int t);
void FUN_00471950(int t);
void FUN_00486500(int scale);
void FUN_00461bb0(int t);
void Particle_Interpolate(int t);
void Dash_Interpolate(int t);
void StageObject_UpdateDebris(int scale);
void FUN_0047e1e0(int t);

// Per-frame race update: advances the stage timing, the particle and dash
// effects and the debris, then refreshes them once the frame gate opens.
// FUNCTION: CMR2 0x0041d060
void FUN_0041d060(int param_1)
{
    Car_InterpolateRenderTransforms();
    FUN_00484d30(param_1);
    FUN_00471950(param_1);
    FUN_00486500(param_1);
    Particle_Interpolate(param_1);
    FUN_00461bb0(param_1);
    Dash_Interpolate(param_1);
    if ((BYTE)RallyData_IsChampionshipFinalStage()) {
        StageObject_UpdateDebris(param_1);
    }
    if (CGameInfo::FUN_004063f0(0) != 0) {
        FUN_0047e1e0(param_1);
    }
}

int StageObject_GetWheelSlip(int carIndex, int wheelIndex);
int FUN_00465e40(int car, int wheel);

// Recomputes one car's stage sound levels. The four wheel-slip values are read
// and scaled, the level of each of the four stage sound slots is derived from
// the engine volume and the largest slip and then eased towards the value of
// the previous frame (the 0x05xxx4xx volume/previous tables live inside
// g_raceBlock), and the result is pushed into the slot handles. The shared
// engine sample gets its pan from the engine speed when the current pattern has
// run out, and the slot levels are muted, faded out or restored depending on
// the surface the wheels are on.
// The volumes live in the per-car tables below: the original updates them in
// place, so other readers see the muted/clamped values of this frame.
// Per-car surface sound volumes (8 cars each) inside g_raceBlock.
#define g_surfacePrevD ((int *)(g_raceBlock + 0x000))  // 0x537568
#define g_surfaceVolA ((int *)(g_raceBlock + 0x020))   // 0x537588
#define g_surfacePrevB ((int *)(g_raceBlock + 0x048))  // 0x5375b0
#define g_surfacePrevA ((int *)(g_raceBlock + 0x094))  // 0x5375fc
#define g_surfaceSlotMax (*(int *)(g_raceBlock + 0x0b4)) // 0x53761c
#define g_surfaceVolB ((int *)(g_raceBlock + 0x0b8))   // 0x537620
#define g_surfaceVolC ((int *)(g_raceBlock + 0x0d8))   // 0x537640
#define g_surfaceVolD ((int *)(g_raceBlock + 0x100))   // 0x537668
#define g_surfacePrevC ((int *)(g_raceBlock + 0x220))  // 0x537788
#define g_wheelSlipVolume ((int (*)[4])(g_raceBlock + 0x7e0)) // 0x537d48

// FUNCTION: CMR2 0x0041a5c0
void FUN_0041a5c0(int param_1, int param_2)
{
    int ownVolume[4];
    Car *pCar;
    CarSoundSet *pSet;
    RaceCarSoundState *pState;
    int speedVolume;
    int ownVolumeSum;
    int speedFrac;
    int weight;
    int weightInv;
    int total;
    unsigned int i;
    int slotVolume;

    speedVolume = FUN_00418e70(param_1);
    pSet = &g_carSoundSets[param_1];
    pState = &g_carSoundStates[param_1];
    pCar = Car_Get(param_1);
    total = 0;
    for (i = 0; i < 4; i++) {
        ownVolume[i] = FixDiv(FIX_ABS(pCar->wheelLoad[0]), 0xa0000);
        if (ownVolume[i] > 0x10000)
            ownVolume[i] = 0x10000;
        ownVolume[i] = 0x10000 - ownVolume[i];
        total += ownVolume[i];
    }
    ownVolumeSum = FixDiv(total, 0x40000);
    speedFrac = FixMulShift32(pCar->speed, 0x431168);
    g_surfaceSlotMax = 0;
    g_carMaxVolume[param_1] = 0;
    for (i = 0; i < 4; i++) {
        g_wheelSlipVolume[param_1][i] = StageObject_GetWheelSlip(param_1, i);
        slotVolume = FUN_00465e40(param_1, i);
        g_carSlotVolumes[param_1][i] = slotVolume;
        if (g_wheelSlipVolume[param_1][i] > g_carMaxVolume[param_1])
            g_carMaxVolume[param_1] = g_wheelSlipVolume[param_1][i];
        if (slotVolume > g_surfaceSlotMax)
            g_surfaceSlotMax = slotVolume;
    }
    if (pState->pattern != 0x19) {
        weightInv = FixDiv(FUN_004781c0(param_1) - pState->time, 0x320000);
        weight = 0x10000 - weightInv;
    } else {
        weight = 0x10000;
        weightInv = 0;
    }
    g_surfaceVolA[param_1] = FixMul(weight, speedVolume);
    g_surfaceVolB[param_1] = FixMul(weightInv, speedVolume);
    g_surfaceVolD[param_1] = FixMul(weight, g_carMaxVolume[param_1]);
    g_surfaceVolC[param_1] = FixMul(weightInv, g_carMaxVolume[param_1]);
    g_surfaceVolB[param_1] += g_carMaxVolume[param_1];
    g_surfaceVolA[param_1] += g_carMaxVolume[param_1];
    if (g_surfaceVolD[param_1] - g_surfacePrevD[param_1] > 0xc000)
        g_surfaceVolD[param_1] = g_surfacePrevD[param_1] + 0xc000;
    else if (g_surfaceVolD[param_1] - g_surfacePrevD[param_1] < -0xccc)
        g_surfaceVolD[param_1] = g_surfacePrevD[param_1] - 0xccc;
    if (g_surfaceVolC[param_1] - g_surfacePrevC[param_1] > 0xc000)
        g_surfaceVolC[param_1] = g_surfacePrevC[param_1] + 0xc000;
    else if (g_surfaceVolC[param_1] - g_surfacePrevC[param_1] < -0xccc)
        g_surfaceVolC[param_1] = g_surfacePrevC[param_1] - 0xccc;
    if (g_surfaceVolA[param_1] - g_surfacePrevA[param_1] > 0xccc)
        g_surfaceVolA[param_1] = g_surfacePrevA[param_1] + 0xccc;
    else if (g_surfaceVolA[param_1] - g_surfacePrevA[param_1] < -0xccc)
        g_surfaceVolA[param_1] = g_surfacePrevA[param_1] - 0xccc;
    if (g_surfaceVolB[param_1] - g_surfacePrevB[param_1] > 0xccc)
        g_surfaceVolB[param_1] = g_surfacePrevB[param_1] + 0xccc;
    else if (g_surfaceVolB[param_1] - g_surfacePrevB[param_1] < -0xccc)
        g_surfaceVolB[param_1] = g_surfacePrevB[param_1] - 0xccc;
    g_surfaceSlotMax = FixMul(g_surfaceSlotMax, 0x20000);
    if (g_surfaceSlotMax > 0x10000)
        g_surfaceSlotMax = 0x10000;
    if (pState->state == 0x19) {
        if (pCar->flag0x1d0[2] != 0 && speedFrac < 0x32) {
            g_surfaceVolA[param_1] = g_surfaceSlotMax;
            g_surfaceVolB[param_1] = g_surfaceSlotMax;
            g_surfaceVolD[param_1] = g_surfaceSlotMax;
            g_surfaceVolC[param_1] = g_surfaceSlotMax;
            g_wheelSlipVolume[param_1][0] = FixMul(g_wheelSlipVolume[param_1][0], 0x10000 - g_surfaceSlotMax);
            g_wheelSlipVolume[param_1][1] = FixMul(g_wheelSlipVolume[param_1][1], 0x10000 - g_surfaceSlotMax);
            g_wheelSlipVolume[param_1][2] = FixMul(g_wheelSlipVolume[param_1][2], 0x10000 - g_surfaceSlotMax);
            g_wheelSlipVolume[param_1][3] = FixMul(g_wheelSlipVolume[param_1][3], 0x10000 - g_surfaceSlotMax);
        } else if (pState->field_0xb0[0] != 0) {
            g_surfaceVolA[param_1] = 0;
            g_surfaceVolB[param_1] = 0;
            g_surfaceVolD[param_1] = 0;
            g_surfaceVolC[param_1] = 0;
        } else {
            if (pCar->wheelLoad[0] < 0xa0000 && pCar->wheelLoad[1] < 0xa0000 &&
                pCar->wheelLoad[2] < 0xa0000 && pCar->wheelLoad[3] < 0xa0000 &&
                (pCar->handbrake != 0 || pCar->flag0x1d0[3] != 0) &&
                (FixMul(pCar->speed, 0x431168) & 0xffff0000) > 0x50000) {
                g_surfaceVolB[param_1] = 0x20000;
                g_surfaceVolD[param_1] = 0x20000;
                g_surfaceVolC[param_1] = 0x20000;
                g_surfaceVolA[param_1] = 0x20000;
                g_wheelSlipVolume[param_1][0] = 0x10000 - ownVolumeSum;
                g_wheelSlipVolume[param_1][1] = 0x10000 - ownVolumeSum;
                g_wheelSlipVolume[param_1][2] = 0x10000 - ownVolumeSum;
                g_wheelSlipVolume[param_1][3] = 0x10000 - ownVolumeSum;
            } else {
                g_surfaceVolB[param_1] = 0;
                g_surfaceVolA[param_1] = 0;
                g_surfaceVolD[param_1] = 0;
                g_surfaceVolC[param_1] = 0;
            }
            speedFrac = FixMulShift32(pCar->speed, 0x431168);
            if (speedFrac < 0x32 && speedFrac >= 0)
                g_unk0x005374c0 = (speedFrac - 0x32) * 11025 / 50 + 0x5622;
            else
                g_unk0x005374c0 = 0x5622;
            Sound_SetPan(pSet->handle[4], g_unk0x005374c0);
        }
    }
    if (g_surfaceVolA[param_1] > 0x10000)
        g_surfaceVolA[param_1] = 0x10000;
    if (g_surfaceVolB[param_1] > 0x10000)
        g_surfaceVolB[param_1] = 0x10000;
    if (Sound_IsPlaying(pSet->handle[4]))
        FUN_004b79a0(pSet->handle[4],
                     FixMul(FUN_00427d50(param_1, param_2), FixMul(g_unk0x00537664, g_surfaceVolA[param_1])));
    if (Sound_IsPlaying(pSet->handle[5]))
        FUN_004b79a0(pSet->handle[5],
                     FixMul(FUN_00427d50(param_1, param_2), FixMul(g_unk0x00537664, g_surfaceVolB[param_1])));
    if (Sound_IsPlaying(pSet->handle[6]))
        FUN_004b79a0(pSet->handle[6],
                     FixMul(FUN_00427d50(param_1, param_2), FixMul(g_unk0x00537664, g_surfaceVolD[param_1])));
    if (Sound_IsPlaying(pSet->handle[7]))
        FUN_004b79a0(pSet->handle[7],
                     FixMul(FUN_00427d50(param_1, param_2), FixMul(g_unk0x00537664, g_surfaceVolC[param_1])));
    if (FUN_00427aa0()) {
        total = 0;
        for (i = 0; i < 4; i++)
            total += g_wheelSlipVolume[param_1][i];
        if (total > 0x10000)
            total = 0x10000;
        if (Sound_IsPlaying(pSet->handle[0]))
            FUN_004b79a0(pSet->handle[0],
                         FixMul(FUN_00427d50(param_1, param_2), FixMul(g_unk0x00537664, total)));
    } else {
        if (Sound_IsPlaying(pSet->handle[0]))
            FUN_004b79a0(pSet->handle[0],
                         FixMul(FUN_00427d50(param_1, param_2), FixMul(g_unk0x00537664, g_wheelSlipVolume[param_1][0])));
        if (Sound_IsPlaying(pSet->handle[1]))
            FUN_004b79a0(pSet->handle[1],
                         FixMul(FUN_00427d50(param_1, param_2), FixMul(g_unk0x00537664, g_wheelSlipVolume[param_1][1])));
        if (Sound_IsPlaying(pSet->handle[2]))
            FUN_004b79a0(pSet->handle[2],
                         FixMul(FUN_00427d50(param_1, param_2), FixMul(g_unk0x00537664, g_wheelSlipVolume[param_1][2])));
        if (Sound_IsPlaying(pSet->handle[3]))
            FUN_004b79a0(pSet->handle[3],
                         FixMul(FUN_00427d50(param_1, param_2), FixMul(g_unk0x00537664, g_wheelSlipVolume[param_1][3])));
    }
    g_surfacePrevD[param_1] = g_surfaceVolD[param_1];
    g_surfacePrevC[param_1] = g_surfaceVolC[param_1];
    g_surfacePrevA[param_1] = g_surfaceVolA[param_1];
    g_surfacePrevB[param_1] = g_surfaceVolB[param_1];
}

// Helper implemented in Car.cpp.
int FUN_00422f50(BYTE index);

// Sets the surface byte of slot `param_1` from the surface type of the car
// part `param_2` (1/2/3 keep the original surface, anything else forces
// gravel) and then rebuilds the slot's visual state.
// FUNCTION: CMR2 0x0041af60
void FUN_0041af60(int param_1, int param_2)
{
    if (FUN_00422f50(param_2) != 1) {
        if (FUN_00422f50(param_2) != 2) {
            if (FUN_00422f50(param_2) != 3) {
                g_unk0x005375f4[param_1] = 1;
                goto done;
            }
        }
    }
    g_unk0x005375f4[param_1] = 0;
done:
    FUN_0041a340(param_1, param_2);
    FUN_0041ae80(param_1, param_2);
    FUN_0041a5c0(param_1, param_2);
    if (FUN_00427aa0() == 0)
        FUN_0041a0a0(param_1, param_2);
    FUN_00419ed0(param_1, param_2);
    FUN_00419cd0(param_1, param_2);
}

extern char g_strPathConcat[];
// Co-driver speech sample table loader: the installer sound directory is
// prepended to every .WAV name and the concatenation is loaded as a stage
// sample through the sound slot of the current stage file.
// GLOBAL: CMR2 0x00518674
char g_str0x00518674[] = "\\Speech\\English\\Wavs\\ONE.WAV";
// GLOBAL: CMR2 0x00518654
char g_str0x00518654[] = "\\Speech\\English\\Wavs\\TWO.WAV";
// GLOBAL: CMR2 0x00518634
char g_str0x00518634[] = "\\Speech\\English\\Wavs\\THREE.WAV";
// GLOBAL: CMR2 0x00518614
char g_str0x00518614[] = "\\Speech\\English\\Wavs\\FOUR.WAV";
// GLOBAL: CMR2 0x005185f4
char g_str0x005185f4[] = "\\Speech\\English\\Wavs\\FIVE.WAV";
// GLOBAL: CMR2 0x005185d4
char g_str0x005185d4[] = "\\Speech\\English\\Wavs\\SIX.WAV";
// GLOBAL: CMR2 0x005185ac
char g_str0x005185ac[] = "\\Speech\\English\\Wavs\\OPENHAIRPIN.WAV";
// GLOBAL: CMR2 0x00518588
char g_str0x00518588[] = "\\Speech\\English\\Wavs\\HAIRPIN.WAV";
// GLOBAL: CMR2 0x00518568
char g_str0x00518568[] = "\\Speech\\English\\Wavs\\LEFT.WAV";
// GLOBAL: CMR2 0x00518548
char g_str0x00518548[] = "\\Speech\\English\\Wavs\\RIGHT.WAV";
// GLOBAL: CMR2 0x00518528
char g_str0x00518528[] = "\\Speech\\English\\Wavs\\LONG.WAV";
// GLOBAL: CMR2 0x00518508
char g_str0x00518508[] = "\\Speech\\English\\Wavs\\OPENS.WAV";
// GLOBAL: CMR2 0x005184e4
char g_str0x005184e4[] = "\\Speech\\English\\Wavs\\TIGHTENS.WAV";
// GLOBAL: CMR2 0x005184c8
char g_str0x005184c8[] = "\\Speech\\English\\Wavs\\GO.WAV";
// GLOBAL: CMR2 0x005184a8
char g_str0x005184a8[] = "\\Speech\\English\\Wavs\\AND.WAV";
// GLOBAL: CMR2 0x00518488
char g_str0x00518488[] = "\\Speech\\English\\Wavs\\INTO.WAV";
// GLOBAL: CMR2 0x0051846c
char g_str0x0051846c[] = "\\Speech\\English\\Wavs\\30.WAV";
// GLOBAL: CMR2 0x00518450
char g_str0x00518450[] = "\\Speech\\English\\Wavs\\50.WAV";
// GLOBAL: CMR2 0x00518434
char g_str0x00518434[] = "\\Speech\\English\\Wavs\\70.WAV";
// GLOBAL: CMR2 0x00518414
char g_str0x00518414[] = "\\Speech\\English\\Wavs\\100.WAV";
// GLOBAL: CMR2 0x005183f4
char g_str0x005183f4[] = "\\Speech\\English\\Wavs\\120.WAV";
// GLOBAL: CMR2 0x005183d4
char g_str0x005183d4[] = "\\Speech\\English\\Wavs\\150.WAV";
// GLOBAL: CMR2 0x005183b4
char g_str0x005183b4[] = "\\Speech\\English\\Wavs\\200.WAV";
// GLOBAL: CMR2 0x00518394
char g_str0x00518394[] = "\\Speech\\English\\Wavs\\OVER.WAV";
// GLOBAL: CMR2 0x00518374
char g_str0x00518374[] = "\\Speech\\English\\Wavs\\CREST.WAV";
// GLOBAL: CMR2 0x00518354
char g_str0x00518354[] = "\\Speech\\English\\Wavs\\JUMP.WAV";
// GLOBAL: CMR2 0x00518334
char g_str0x00518334[] = "\\Speech\\English\\Wavs\\CARE.WAV";
// GLOBAL: CMR2 0x00518310
char g_str0x00518310[] = "\\Speech\\English\\Wavs\\CAUTION.WAV";
// GLOBAL: CMR2 0x005182ec
char g_str0x005182ec[] = "\\Speech\\English\\Wavs\\VERYLONG.WAV";
// GLOBAL: CMR2 0x005182cc
char g_str0x005182cc[] = "\\Speech\\English\\Wavs\\TURN.WAV";
// GLOBAL: CMR2 0x005182a8
char g_str0x005182a8[] = "\\Speech\\English\\Wavs\\NARROWS.WAV";
// GLOBAL: CMR2 0x00518284
char g_str0x00518284[] = "\\Speech\\English\\Wavs\\THROUGH.WAV";
// GLOBAL: CMR2 0x00518264
char g_str0x00518264[] = "\\Speech\\English\\Wavs\\LOGS.WAV";
// GLOBAL: CMR2 0x00518244
char g_str0x00518244[] = "\\Speech\\English\\Wavs\\GATE.WAV";
// GLOBAL: CMR2 0x00518224
char g_str0x00518224[] = "\\Speech\\English\\Wavs\\BRIDGE.WAV";
// GLOBAL: CMR2 0x00518204
char g_str0x00518204[] = "\\Speech\\English\\Wavs\\ROCKS.WAV";
// GLOBAL: CMR2 0x005181e4
char g_str0x005181e4[] = "\\Speech\\English\\Wavs\\POST.WAV";
// GLOBAL: CMR2 0x005181c4
char g_str0x005181c4[] = "\\Speech\\English\\Wavs\\DITCH.WAV";
// GLOBAL: CMR2 0x005181a4
char g_str0x005181a4[] = "\\Speech\\English\\Wavs\\MUD.WAV";
// GLOBAL: CMR2 0x00518184
char g_str0x00518184[] = "\\Speech\\English\\Wavs\\RUTS.WAV";
// GLOBAL: CMR2 0x00518164
char g_str0x00518164[] = "\\Speech\\English\\Wavs\\ICE.WAV";
// GLOBAL: CMR2 0x00518140
char g_str0x00518140[] = "\\Speech\\English\\Wavs\\BADCAMBER.WAV";
// GLOBAL: CMR2 0x00518120
char g_str0x00518120[] = "\\Speech\\English\\Wavs\\ROUGH.WAV";
// GLOBAL: CMR2 0x00518100
char g_str0x00518100[] = "\\Speech\\English\\Wavs\\SLIPPY.WAV";
// GLOBAL: CMR2 0x005180e0
char g_str0x005180e0[] = "\\Speech\\English\\Wavs\\INSIDE.WAV";
// GLOBAL: CMR2 0x005180bc
char g_str0x005180bc[] = "\\Speech\\English\\Wavs\\OUTSIDE.WAV";
// GLOBAL: CMR2 0x00518098
char g_str0x00518098[] = "\\Speech\\English\\Wavs\\DONTCUT.WAV";
// GLOBAL: CMR2 0x0051807c
char g_str0x0051807c[] = "\\Speech\\English\\Wavs\\OK.WAV";
// GLOBAL: CMR2 0x00518058
char g_str0x00518058[] = "\\Speech\\English\\Wavs\\STOP_DITCH.WAV";
// GLOBAL: CMR2 0x00518030
char g_str0x00518030[] = "\\Speech\\English\\Wavs\\ONTO_TARMAC.WAV";
// GLOBAL: CMR2 0x0051800c
char g_str0x0051800c[] = "\\Speech\\English\\Wavs\\ONTO_SNOW.WAV";
// GLOBAL: CMR2 0x00517fe4
char g_str0x00517fe4[] = "\\Speech\\English\\Wavs\\ONTO_GRAVEL.WAV";
// GLOBAL: CMR2 0x00517fc0
char g_str0x00517fc0[] = "\\Speech\\English\\Wavs\\ONTO_MUD.WAV";
// GLOBAL: CMR2 0x00517f9c
char g_str0x00517f9c[] = "\\Speech\\English\\Wavs\\ONTO_ICE.WAV";
// GLOBAL: CMR2 0x00517f78
char g_str0x00517f78[] = "\\Speech\\English\\Wavs\\JUMP_MAYBE.WAV";
// GLOBAL: CMR2 0x00517f54
char g_str0x00517f54[] = "\\Speech\\English\\Wavs\\STRAIGHT.WAV";
// GLOBAL: CMR2 0x00517f2c
char g_str0x00517f2c[] = "\\Speech\\English\\Wavs\\COUNTDOWN_3.WAV";
// GLOBAL: CMR2 0x00517f04
char g_str0x00517f04[] = "\\Speech\\English\\Wavs\\COUNTDOWN_2.WAV";
// GLOBAL: CMR2 0x00517edc
char g_str0x00517edc[] = "\\Speech\\English\\Wavs\\COUNTDOWN_1.WAV";

void FUN_00416770(void);

// Initializes the co-driver sound samples through the original tail-call entry.
// FUNCTION: CMR2 0x00416710
void FUN_00416710(void)
{
    FUN_00416770();
}

// FUNCTION: CMR2 0x00416770
void FUN_00416770(void)
{
    char buf[260];
    char *pDir;

    pDir = CInstallInfo::GetSoundsDir();
    g_unk0x0053735c = FUN_004b7940();
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518674);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518654);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518634);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518614);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x005185f4);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x005185d4);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x005185ac);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518588);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518568);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518548);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518528);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518508);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x005184e4);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x005184c8);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x005184a8);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518488);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x0051846c);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518450);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518434);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518414);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x005183f4);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x005183d4);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x005183b4);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518394);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518374);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518354);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518334);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518310);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x005182ec);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x005182cc);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x005182a8);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518284);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518264);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518244);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518224);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518204);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x005181e4);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x005181c4);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x005181a4);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518184);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518164);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518140);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518120);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518100);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x005180e0);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x005180bc);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518098);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x0051807c);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518058);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00518030);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x0051800c);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00517fe4);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00517fc0);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00517f9c);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00517f78);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00517f54);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00517f2c);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00517f04);
    FUN_00418760(buf);
    sprintf(buf, g_strPathConcat, pDir, g_str0x00517edc);
    FUN_00418760(buf);
    StageTiming_FreeStageFile2();
}

// ---- Race frame update (0x41c5a0) -------------------------------------------
// Scratch globals shared by the per-frame race update.
// GLOBAL: CMR2 0x00538124
int g_unk0x00538124;
// GLOBAL: CMR2 0x00538104
int g_unk0x00538104;
// GLOBAL: CMR2 0x00537f04
int g_unk0x00537f04;
// GLOBAL: CMR2 0x00537fc8
int g_unk0x00537fc8;
// GLOBAL: CMR2 0x00537f28
int g_unk0x00537f28;
// GLOBAL: CMR2 0x00537fb8
BYTE g_unk0x00537fb8;
// GLOBAL: CMR2 0x00537fd0
short *g_unk0x00537fd0;
// GLOBAL: CMR2 0x00537fc4
short g_unk0x00537fc4;

unsigned int RallyData_GetFlag21(void);
void FUN_0042bc80(short *pList, short count);
short *Car_GetOrder(void);
void FUN_00411f00(void);
int RallyData_IsChampionshipFinalStage(void);
int FUN_004481f0(int car, int index);
int FUN_0041b380(void);
int FUN_00445a20(void);
int FUN_00445a40(void);
void Physics_UpdateRateHold(void);
int Car_UpdateEngineNoteFalloff(int *pOut);
void Car_UpdateAndRenderAll(void);
void FUN_00448e70(void);
int FUN_0041f3f0(BYTE index);
void FUN_0047a710(int player, int listener);
void FUN_0047aa70(void);
void FUN_0047ad20(void);
void FUN_0047b000(int slot);
void FUN_0047b640(int slot);
void FUN_0047b7b0(int slot);
void FUN_0047b870(int index);
int FUN_0047d330(int car, int preview);
void FUN_004209f0(void);
void StageObjects_Update(void);
void FUN_004581d0(void);
void FUN_00448120(void);
void FUN_00455590(int group);
void FUN_0045e7f0(void);
void FUN_004664c0(short *pOrder, short count);
void FUN_004285b0(unsigned int player, int t, int check);
void FUN_004284d0(unsigned int player, int check);
BYTE FUN_00458290(int index);
BYTE FUN_004582b0(int index);
int FUN_00458330(int index);
void FUN_00427890(void);
void FUN_004a2fe0(void);
void FUN_00478c40(void);
void FUN_00418ee0(void);
void FUN_00418780(void);
int FUN_00404e10(unsigned short slot);
void FUN_00404ea0(BYTE param1);
void FUN_00464c60(int car);
void FUN_0045af00(int car);
void View_BlendCameraStates(unsigned char param1, int param2);
int FUN_00422f50(BYTE index);
BYTE FUN_00422fb0(BYTE index);
int FUN_00428740(BYTE index);
void FUN_00411f70(int param_1, int param_2);
void FUN_00416f70(int player);
void FUN_00417090(int param_1);
void FUN_0041af60(int param_1, int param_2);
void FUN_0041d060(int param_1);
void FUN_0041d0c0(int param_1);
void View_UpdateModeSurface(unsigned int view);
void View_UpdateCamera(BYTE view);
unsigned int FUN_0049e940(void);
bool FUN_004b7cd0(int *pOut);
void FUN_004b7c80(void);
void Sound_FreeAll(void);
void Particle_UpdateAll(int param);
void FUN_004660e0(BYTE value);
int FUN_00466090(void);
void FUN_00466030(int a, int b);
int Replay_StopRecording(BYTE *pBuffer);
void WheelTrail_Update(int carIndex);
void Dash_Update(int player);

// Per-frame update of one player's race view: advances the race, the car
// order, the dashboards and the replay recording. The car loop walks the whole
// order but only the entrants below the player count are driven; each car's
// call state selects its co-driver handling in the switch below.
// match 68%: below the 90% bar; the remaining diff is dominated by register
// allocation, not logic. MSVC6 keeps the register-allocated constant 0 in EDI
// and `first` in EBP here, while the original has the constant in EBP and
// `first` in EDI; that swap flips the operand order of every zero comparison
// (cmp edi,ebp vs cmp ebp,edi), the zero-argument pushes, and permutes the
// spill slots (S-0x14/S-0x10/S-0xc), which in turn moves the branch/block
// layout of the switch and of the car-loop tail. Calls, constants, statement
// order and control flow were checked instruction by instruction against the
// original; the layout of GameInfo0xa4.rallyStageRecordTimes (0x1214 in the
// original vs 0x654 in our struct) is a separate data-model gap reported to the
// integrator.
// FUNCTION: CMR2 0x0041c5a0
void FUN_0041c5a0(BYTE param1, int param2)
{
    short first;
    int second;
    int count;
    int player;
    int flag;
    int key;
    int k;
    int value;
    int index;
    BYTE b1;
    int b2;
    BYTE b3;

    first = 0;
    count = RallyDataState() & 0xff;
    if (FUN_00445a20())
        first = 1;
    second = FUN_00445a40();
    Physics_UpdateRateHold();
    g_unk0x00538124 = Car_UpdateEngineNoteFalloff(&g_unk0x00537f60);
    g_unk0x00537f0c[4] = 0;
    for (; g_unk0x00537f0c[4] < g_unk0x00538124; g_unk0x00537f0c[4]++) {
        player = Car_Get(0)->field_0xb43 > 0;
        CInput::FUN_0049eab0();
        if (player != 0) {
            g_unk0x00537f24++;
            if (param2 != 0 && param1 == 10)
                FUN_00448e70();
        }
        for (g_unk0x00538104 = 0; g_unk0x00538104 < Car_GetOrderCount(); g_unk0x00538104++) {
            if (Car_Get(g_unk0x00538104)->field_0xb43 > 0) {
                if (g_unk0x00538104 < (BYTE)RallyDataState()) {
                    g_unk0x00537fb8 = *(char *)(*(int *)(FUN_0041b390() + 4) + g_unk0x00538104 * 8);
                    if (FUN_0041f3d0(g_unk0x00538104) != 0)
                        g_unk0x00537fb8 = *(char *)(g_unk0x00537f3c[g_unk0x00538104] + 0x10c);
                } else {
                    if ((BYTE)(*(int *)(*(int *)(FUN_0041b390() + 4))) <= 7) {
                        g_unk0x00537fb8 = 7;
                    } else {
                        g_unk0x00537fb8 = 8;
                        if ((char)RallyData_FUN_00407ea0() != 0 && (char)CGameInfo::FUN_00406310() != 0) {
                            if (FUN_00466090() == 0)
                                g_unk0x00537fb8 = 9;
                        } else if ((char)RallyData_FUN_00407e90() != 0 &&
                                   (char)CGameInfo::FUN_00405e00() == 0 &&
                                   FUN_004582b0(g_unk0x00538104) != 0) {
                            g_unk0x00537fb8 = 9;
                        }
                    }
                }
                switch (g_unk0x00537fb8) {
                case 13:
                    FUN_0047b000(g_unk0x00538104);
                    break;
                case 5:
                case 7:
                    g_unk0x0053811c = 0;
                    FUN_0047b640(g_unk0x00538104);
                    break;
                case 10:
                    if ((char)RallyData_FUN_00407e70() != 0 && g_unk0x00538104 < count) {
                        if (FUN_0041f3d0(g_unk0x00538104) == 0) {
                            FUN_0047d330(g_unk0x00538104, 0);
                            break;
                        }
                    }
                    if (FUN_0041f3d0(g_unk0x00538104) == 0)
                        FUN_0047b870(g_unk0x00538104);
                    else if (first == 0)
                        FUN_0047b000(g_unk0x00538104);
                    break;
                case 8:
                    if (first == 0)
                        FUN_0047b000(g_unk0x00538104);
                    break;
                case 9:
                    if ((char)RallyData_FUN_00407e70() != 0) {
                        if (g_unk0x00538104 < count)
                            FUN_0047d330(g_unk0x00538104, 0);
                        else
                            FUN_0047b000(g_unk0x00538104);
                    } else {
                        if (g_unk0x00537f98[g_unk0x00538104] == 0) {
                            FUN_0047b870(g_unk0x00538104);
                        } else {
                            if (FUN_0041f3f0(g_unk0x00538104) != 0 ||
                                FUN_0041f3d0(g_unk0x00538104) != 0)
                                FUN_0047b7b0(g_unk0x00538104);
                            else
                                FUN_0047b870(g_unk0x00538104);
                        }
                    }
                    break;
                }
                if (g_unk0x00538104 < count) {
                    Dash_Update(g_unk0x00538104);
                    if (g_unk0x00537fb8 == 5 || g_unk0x00537fb8 == 7)
                        FUN_00411f70(g_unk0x00538104, 1);
                    else if (g_unk0x00537fb8 == 8 || g_unk0x00537fb8 == 9)
                        FUN_00411f70(g_unk0x00538104, 0);
                    if (param1 == 8 || param1 == 9)
                        FUN_00417090(g_unk0x00538104);
                    else if (param1 == 7)
                        FUN_00416f70(g_unk0x00538104);
                    if (g_unk0x00537fb8 == 8 || g_unk0x00537fb8 == 5 ||
                        (g_unk0x00537fb8 == 7 && (char)CGameInfo::FUN_00405e00() == 0)) {
                        b1 = FUN_00404e10(g_unk0x00538104);
                        b2 = FUN_0041f3d0(g_unk0x00538104);
                        b3 = FUN_00428740(g_unk0x00538104);
                        if (b1 != 0 && b2 == 0 && b3 == 0 &&
                            (char)CGameInfo::FUN_00406320() == 0) {
                            FUN_00404ea0(g_unk0x00538104);
                            if ((char)RallyData_FUN_00407e70() != 0)
                                CSound::FUN_004a3160();
                            FUN_00478c40();
                            FUN_00418ee0();
                            FUN_00418780();
                        }
                    }
                }
            }
        }
        if ((char)CGameInfo::FUN_00406320() != 0 && player != 0 &&
            **(char **)(FUN_0041b390() + 4) == 13) {
            if (g_unk0x00537f8c != 0) {
                FUN_004b7c80();
                g_unk0x00537f8c = 0;
            }
            if (FUN_0049e940() != 0 || FUN_004b7cd0(&key) != 0) {
                FUN_0046d2a0((int *)g_unk0x00537f3c[0]);
                g_unk0x00537fc8 = 0;
                for (; g_unk0x00537fc8 < *FUN_0041b390(); g_unk0x00537fc8++)
                    CGame::FUN_0049c1c0((Unk0049c2c0 *)FUN_0041b390(), g_unk0x00537fc8, 1, 3);
                FUN_0041f2a0();
            }
        }
        for (g_unk0x00537f04 = 0; g_unk0x00537f04 < count; g_unk0x00537f04++) {
            if (Car_Get(g_unk0x00537f04)->field_0xb43 > 0) {
                if (param2 != 0)
                    FUN_0041d0c0(g_unk0x00537f04);
                if (FUN_00422f50((BYTE)g_unk0x00537f04) == 7)
                    View_UpdateModeSurface((BYTE)g_unk0x00537f04);
            }
        }
        if (first == 0 || second != 0)
            Car_UpdateAndRenderAll();
        if (player != 0)
            StageObjects_Update();
        for (g_unk0x00537f04 = 0; g_unk0x00537f04 < Car_GetOrderCount(); g_unk0x00537f04++) {
            if (Car_Get(g_unk0x00537f04)->field_0xb43 > 0)
                FUN_004284d0((BYTE)g_unk0x00537f04, 0);
        }
        if (player != 0) {
            FUN_004209f0();
            FUN_004581d0();
            FUN_00448120();
            if ((char)RallyData_GetFlag21() != 0) {
                if (FUN_0041b380() == 4)
                    FUN_00455590(FUN_0041b370() & 0xff);
                else
                    FUN_00455590(0);
            }
            FUN_0045e7f0();
        }
        for (g_unk0x00537f04 = 0; g_unk0x00537f04 < count; g_unk0x00537f04++) {
            if (Car_Get((BYTE)FUN_00422fb0((BYTE)g_unk0x00537f04))->field_0xb43 > 0)
                View_UpdateCamera((BYTE)g_unk0x00537f04);
        }
        if (player != 0) {
            if (first == 0 || second != 0)
                Particle_UpdateAll((int)g_viewNodes[g_unk0x00537f04]);
            if (CGameInfo::FUN_00405d80() == 0xc && FUN_00458290(0) != 0) {
                FUN_00427950(FUN_004481f0(0, FUN_00458330(0)));
                FUN_0040af00(FUN_004481f0(0, FUN_00458330(0)));
            }
            if ((char)RallyData_GetFlag24() != 0 && (char)RallyData_FUN_00407ea0() != 0 &&
                FUN_00458290(0) != 0) {
                FUN_0046d2a0((int *)g_unk0x00537f3c[0]);
                Replay_StopRecording(g_unk0x00537f3c[0]);
                if (CGameInfo::FUN_00406310() != 0)
                    FUN_00466080();
                flag = CGameInfo::FUN_00405d00() == 0;
                value = FUN_004481f0(0, FUN_00458330(0));
                index = (RallyData_FUN_00406940() & 0xff) * 3 + (RallyData_FUN_00406950() & 0xff);
                if (value <= (int)((CGameInfo::FUN_00405ff0(flag)->rallyStageRecordTimes[index].value
                                    >> 7) & 0xffff)) {
                    index = (RallyData_FUN_00406940() & 0xff) * 3 + (RallyData_FUN_00406950() & 0xff);
                    CGameInfo::FUN_00405ff0(flag)->rallyStageRecordTimes[index].value =
                        (value & 0xffff) << 7 |
                        CGameInfo::FUN_00405ff0(flag)->rallyStageRecordTimes[index].value & 0xff80007f;
                    k = RallyData_FUN_004086b0(0);
                    index = (RallyData_FUN_00406940() & 0xff) * 3 + (RallyData_FUN_00406950() & 0xff);
                    CGameInfo::FUN_00405ff0(flag)->rallyStageRecordTimes[index].value =
                        k & 0x3f |
                        CGameInfo::FUN_00405ff0(flag)->rallyStageRecordTimes[index].value & 0xffffffc0;
                    sprintf(CFrontend::m_stringDest, g_strGrp0x005192b0, FUN_0041f920());
                    if (g_unk0x00537f3c[0] != NULL)
                        Replay_Save(g_unk0x00537f3c[0], CFrontend::m_stringDest);
                    if (CGameInfo::FUN_00406310() != 0) {
                        FUN_004660e0(0);
                        FUN_004660a0((int **)g_unk0x00537f3c, 0, 0);
                    }
                }
                FUN_0046c750((ReplayStream *)g_unk0x00537f3c[0], 0, 0);
                if (CGameInfo::FUN_00406310() != 0)
                    FUN_00466030(0, 0);
            }
        }
        if (CGameInfo::FUN_00404f20() == 0 && param1 >= 7) {
            if (RallyData_IsChampionshipFinalStage() != 0 && FUN_0041f3d0(0) == 0) {
                if (g_unk0x00537f2c == 0) {
                    Sound_FreeAll();
                    g_unk0x00537f2c = 1;
                }
            } else {
                k = ((BYTE)RallyDataState() == 1 && (char)RallyData_GetFlag25() != 0 &&
                     (char)CGameInfo::FUN_00405e00() == 0 && CGameInfo::FUN_00405d80() != 3);
                for (g_unk0x00537f04 = 0;
                     g_unk0x00537f04 < (BYTE)RallyDataState() + k;
                     g_unk0x00537f04++) {
                    if (!(k != 1)) {
                        FUN_0047a710(g_unk0x00537f04, 0);
                        FUN_0041af60(g_unk0x00537f04, 0);
                        FUN_00418b00((Unk0049c2c0 *)g_unk0x00537f04, 0);
                    } else {
                        FUN_0047a710(g_unk0x00537f04, g_unk0x00537f04);
                        FUN_0041af60(g_unk0x00537f04, g_unk0x00537f04);
                        FUN_00418b00((Unk0049c2c0 *)g_unk0x00537f04, g_unk0x00537f04);
                    }
                }
            }
        }
        if ((char)CGameInfo::FUN_00405e00() != 0) {
            if (CGameInfo::FUN_00405d80() == 0xb || CGameInfo::FUN_00405d80() == 0xc)
                FUN_0047ad20();
        } else if ((CGameInfo::FUN_00405d80() == 5 || CGameInfo::FUN_00405d80() == 6) &&
                   (BYTE)RallyDataState() == 1) {
            FUN_0047aa70();
        }
        if (param2 == 0) {
            for (g_unk0x00537f04 = 0; g_unk0x00537f04 < (BYTE)RallyDataState(); g_unk0x00537f04++) {
            }
        }
        FUN_0042bc80(Car_GetOrder(), Car_GetOrderCount());
    }
    if (CGameInfo::FUN_00405d80() == 8 || CGameInfo::FUN_00405d80() == 9 ||
        CGameInfo::FUN_00405d80() == 0xb)
        FUN_00411f00();
    g_unk0x00537f28 = FUN_0041f270();
    if (first != 0)
        g_unk0x00537f28 = 0x10000;
    FUN_0041d060(g_unk0x00537f28);
    for (g_unk0x00537f04 = 0; g_unk0x00537f04 < (BYTE)RallyDataState(); g_unk0x00537f04++) {
        View_BlendCameraStates((BYTE)g_unk0x00537f04,
                     *(int *)Car_Get((BYTE)FUN_00422fb0((BYTE)g_unk0x00537f04))->field_0xa90);
    }
    g_unk0x00537fd0 = Car_GetOrder();
    g_unk0x00537fc4 = Car_GetOrderCount();
    FUN_004664c0(g_unk0x00537fd0, g_unk0x00537fc4);
    for (g_unk0x00537f04 = 0; g_unk0x00537f04 < Car_GetOrderCount(); g_unk0x00537f04++)
        FUN_004285b0(g_unk0x00537f04, g_unk0x00537f28, 0);
    if (first == 0 || second != 0) {
        if (CGameInfo::FUN_00404f20() == 0) {
            g_unk0x00537fc4 = Car_GetOrderCount();
            for (g_unk0x00537fc8 = 0; g_unk0x00537fc8 < g_unk0x00537fc4; g_unk0x00537fc8++) {
                FUN_00464c60(g_unk0x00537fc8);
                WheelTrail_Update(g_unk0x00537fc8);
                if ((char)RallyData_FUN_00407e70() == 0 &&
                    CGameInfo::FUN_00405d80() != 0xc && CGameInfo::FUN_00405d80() != 0xb)
                    FUN_0045af00(g_unk0x00537fc8);
            }
        }
    }
    if ((char)CGameInfo::FUN_00405e00() != 0)
        FUN_00427890();
    if ((char)RallyData_FUN_00407e70() != 0)
        FUN_004a2fe0();
}
