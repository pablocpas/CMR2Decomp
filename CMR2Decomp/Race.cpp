#include <windows.h>
#include "Game.h"
#include "RallyData.h"
#include "GameInfo.h"
#include "StageUI.h"
#include "FixedPoint.h"
#include <stdio.h>
#include <string.h>
#include "InstallInfo.h"
#include "Frontend.h"
#include "Graphics.h"
#include "StageTiming.h"
#include "GenericFileLoader.h"
#include "FileBuffer.h"
#include "Input.h"
#include "Menu.h"
#include "main.h"

// Race session state (0x41e210-0x420190)

unsigned int RallyDataState(void);
unsigned int RallyData_FUN_00407e70(void);
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
// TODO: CMR2 0x00417660 (implemented, match 73%)
void Race_AssignUnusedSlot(int owner)
{
    int i = 0;
    do {
        int next = i;
        if ((g_raceSlotState[i].flags & 2) == 0) {
            next = 20;
            g_raceSlotState[i].flags |= 2;
            g_raceSlotState[i].owner = owner;
            g_raceSlotState[i].flags &= 0xfe;
            g_raceSlotState[i].pending = -1;
        }
        i = next + 1;
    } while (i < 20);
}

int FUN_00417760(int index);
int Sound_IsPlaying(unsigned int handle);
int FUN_004b7790(unsigned short id, int volume, int frequency, int loopStart, int loops, int is3D);
extern int g_unk0x00537194;

// Plays the queued co-driver calls one after another: starts the first slot's
// sample, and when it has finished moves the queue up.
// TODO: CMR2 0x004176b0 (implemented, match 34%)
void FUN_004176b0(void)
{
    RaceSlotState *p;

    if ((g_raceSlotState[0].flags & 2) != 0 && FUN_00417760(0) == 0) {
        FUN_00417760(0);
        if ((g_raceSlotState[0].flags & 1) == 0) {
            g_raceSlotState[0].pending =
                FUN_004b7790((unsigned short)g_raceSlotState[0].owner, g_unk0x00537194, 0x2b11, 0, 0, 0);
            g_raceSlotState[0].flags |= 1;
        } else if (Sound_IsPlaying(g_raceSlotState[0].pending) == 0) {
            for (p = g_raceSlotState; p < g_raceSlotState + 19; p++)
                *p = p[1];
            g_raceSlotState[19].flags &= 0xfc;
            g_raceSlotState[19].pending = -1;
            g_raceSlotState[19].owner = -1;
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

// Releases the race file loaded by FUN_0041f930 (registered callback).
// FUNCTION: CMR2 0x0041f510
BYTE FUN_0041f510(void)
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

// FUNCTION: CMR2 0x00420120
int FUN_00420120(void)
{
    return g_unk0x00538970;
}

// Number of stages of the current event.
// FUNCTION: CMR2 0x00420190
char FUN_00420190(void)
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
bool FUN_004174d0(void)
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
// TODO: CMR2 0x00417780 (implemented, match 70%)
void FUN_00417780(int player)
{
    RaceSlotState *p;

    g_unk0x0053708c[player] = RallyData_FUN_00421370((BYTE *)Car_Get(player));
    g_unk0x00537198[player] = RallyData_FUN_00421370((BYTE *)Car_Get(player));
    for (p = g_raceSlotState; p < g_raceSlotState + 5; p++) {
        p->flags &= 0xfc;
        p->pending = -1;
        p->owner = -1;
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
int g_unk0x00537248;
// GLOBAL: CMR2 0x0053724c
int g_unk0x0053724c;

// Resets the per-player race state: best values, call records and slots.
// TODO: CMR2 0x00416670 (implemented, match 37%)
void FUN_00416670(void)
{
    RaceCallRecord *p;
    RaceSlotState *pSlot;
    int i;

    g_unk0x00537198[0] = 9999;
    g_unk0x00537248 = 0;
    g_unk0x00537198[1] = 9999;
    g_unk0x0053724c = 0;
    g_unk0x0053708c[0] = -1;
    g_unk0x0053708c[1] = -1;
    memset(g_unk0x005371a4, 0, sizeof(g_unk0x005371a4));
    p = g_raceCallRecords;
    do {
        for (i = 0; i < 5; i++, p++) {
            p->flags &= 0xfffffc00;
            p->field_0x4 = 0;
            p->field_0x0 = 0;
        }
        g_unk0x005371a0 = 0;
    } while (p < g_raceCallRecords + 10);
    for (pSlot = g_raceSlotState; pSlot < g_raceSlotState + 5; pSlot++) {
        pSlot->flags &= 0xfc;
        pSlot->pending = -1;
        pSlot->owner = -1;
    }
}

// FUNCTION: CMR2 0x00417e60
void FUN_00417e60(void)
{
    g_unk0x00537190 = 0;
}

int FUN_004781c0(int index);
int FUN_004b7790(unsigned short id, int volume, int frequency, int loopStart, int loops, int is3D);
void Sound_Free(unsigned int handle);

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

// FUNCTION: CMR2 0x00418d20
void FUN_00418d20(int value)
{
    g_unk0x00537394 = value;
}

BYTE g_raceBlock[0x864];

// Starts the sound of one entry of the stage table and stores its handle, the
// random pitch and the id of the sound.
// TODO: CMR2 0x00418d30 (implemented, match 54%)
void FUN_00418d30(int param1, int param2, int param3, int param4, int param5)
{
    int index;

    index = param3;
    g_carSoundSets[param1].handle[index] = FUN_004b7790(param2, param4, 0x5622, (param5 == 0) ? 0 : param5, 1, 0);
    g_carSoundSets[param1].pitch[index] = rand() % 0x19 + 0x32 + FUN_004781c0(param1);
    g_carSoundSets[param1].surface[param3] = g_unk0x005375f4[param1];
    g_carSoundSets[param1].id[index] = param2;
}

// Stops the sound of one entry of the stage table (and forgets both the handle
// and the id).
// FUNCTION: CMR2 0x00418dd0
void FUN_00418dd0(int param1, int param2, char param3)
{
    if (param3 != 0)
        g_carSoundSets[param1].id[param2] = -1;
    Sound_Free(g_carSoundSets[param1].handle[param2]);
    g_carSoundSets[param1].handle[param2] = -1;
}

// Returns a random value in [0, param2) that is not param1.
// TODO: CMR2 0x00419b50 (implemented, match 53%)
int FUN_00419b50(int param1, int param2)
{
    int value;

    if (param2 == 1)
        return 0;
    value = rand();
    while (value % param2 == param1) {
        value = rand();
    }
    return value % param2;
}

void FUN_00418d30(int param1, int param2, int param3, int param4, int param5);

void FUN_00418dd0(int param1, int param2, char param3);
void FUN_00418e20(int set, int dst, int src);
BYTE FUN_00427aa0(void);

// Moves the car's current sounds to the second bank (slots 4/5 to 6/7) on a
// sound state change, stopping the ones that don't carry over.
// TODO: CMR2 0x00419b90 (implemented, match 50%)
void FUN_00419b90(int car, BYTE *pInfo)
{
    switch (*(int *)(pInfo + 0xa8)) {
    case 1:
    case 4:
    case 9:
    case 0xe:
    case 0x10:
    case 0x13:
    case 0x18:
        break;
    case 2:
    case 0x11:
    case 0x16:
        goto both;
    default:
        goto tail;
    case 6:
    case 0x15:
        if (!FUN_00427aa0())
            FUN_00418dd0(car, 4, 1);
        break;
    case 7:
        if (!FUN_00427aa0())
            FUN_00418dd0(car, 4, 1);
        goto both;
    case 0xb:
        if (!FUN_00427aa0()) {
            FUN_00418dd0(car, 4, 1);
            FUN_00418dd0(car, 6, 1);
        }
        break;
    case 0xc:
        if (!FUN_00427aa0()) {
            FUN_00418dd0(car, 4, 1);
            FUN_00418dd0(car, 6, 1);
        }
        goto both;
    }
    FUN_00418e20(car, 4, 5);
    goto tail;
both:
    FUN_00418e20(car, 4, 5);
    FUN_00418e20(car, 6, 7);
tail:
    *(int *)(g_raceBlock + 0x94 + car * 4) = *(int *)(g_raceBlock + 0x48 + car * 4);
    *(int *)(g_raceBlock + car * 4) = *(int *)(g_raceBlock + 0x220 + car * 4);
    *(int *)(g_raceBlock + 0x48 + car * 4) = 0;
    *(int *)(g_raceBlock + 0x220 + car * 4) = 0;
}

// Switches car's engine sound between its two samples of stage sound group 25
// as the rolling direction speed (0x79c) changes sign.
// TODO: CMR2 0x0041ae80 (implemented, match 54%)
void FUN_0041ae80(int car)
{
    BYTE *pSet = g_raceBlock + car * 0xb4;
    int *pHandle = (int *)(pSet + 0x26c);

    if (*(short *)(pSet + 0x258) == 0x19) {
        if (Car_Get(car)->field_0x79c < 1) {
            if (pSet[0x2f0] != 0) {
                if (Sound_IsPlaying(*pHandle)) {
                    Sound_Free(*pHandle);
                    *pHandle = -1;
                }
                FUN_00418d30(car, g_stageSoundPatterns[25].base[g_unk0x005375f4[car]] + 1, 4, 0, 0x3542);
                pSet[0x2f0] = 0;
            }
        } else if (pSet[0x2f0] == 0) {
            if (Sound_IsPlaying(*pHandle)) {
                Sound_Free(*pHandle);
                *pHandle = -1;
            }
            FUN_00418d30(car, g_stageSoundPatterns[25].base[g_unk0x005375f4[car]], 4, 0, 0);
            pSet[0x2f0] = 1;
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

// Path of a car's texture set: "<cars dir>\<car>d3<n>" or "<cars dir>\<car>c1".
// FUNCTION: CMR2 0x00420060
char *FUN_00420060(int car, int variant, int unused)
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
char *FUN_004200d0(int car)
{
    sprintf(g_raceCarPath, g_strPathFormat, CInstallInfo::GetCarsDir(), CFrontend::FUN_0040ee60(car));
    return g_raceCarPath;
}

extern int g_unk0x00537f5c;
// GLOBAL: CMR2 0x00537f64
BYTE g_raceResourcesFreed;
extern unsigned int g_unk0x00537fc0;
void Sound_FreeAll(void);

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
// TODO: CMR2 0x00418e70 (implemented, match 80%)
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
    } while (pSet < &g_carSoundSets[8]);
}

// One sound handle per car.
// GLOBAL: CMR2 0x005373b0
int g_carSounds[8];

// Frees the per-car sound of every car in the race.
// FUNCTION: CMR2 0x00418780
void FUN_00418780(void)
{
    int *p;
    int i;

    i = 0;
    if ((char)RallyDataState() != 0) {
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

// Updates the volume of each player's car sound by distance to its listener.
// FUNCTION: CMR2 0x00418b00
void FUN_00418b00(Unk0049c2c0 *p, BYTE index)
{
    int i;

    for (i = 0; i < (BYTE)RallyDataState(); i++) {
        if (g_carSounds[i] != -1) {
            if (Sound_IsPlaying(g_carSounds[i]) == 0)
                g_carSounds[i] = -1;
            else
                FUN_004b79a0(g_carSounds[i], FixMul(FUN_00427d50(g_unk0x00537398[i], g_unk0x00537364[i]),
                                                    FixMul(g_unk0x00537394, g_unk0x00537384[i])));
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
    int slot;
    int i;

    for (slot = 0; slot < 4; slot++) {
        if (g_carSounds[slot] == -1)
            break;
    }
    if (slot == 4) {
        slot = 0;
        for (i = 0; i < 4; i++) {
            if (oldest < now - g_unk0x00537374[i]) {
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
        if (Car_Get(view)->field_0xb43[3] < level)                        \
            Car_Get(view)->field_0xb43[3] = (BYTE)level;                  \
    } while (0)

// Plays a random impact sound (light or heavy set) and shakes the car.
// TODO: CMR2 0x00418c30 (implemented, match 57%)
void FUN_00418c30(unsigned int view, int volume, char heavy, int listener)
{
    int sound;

    if (heavy == 0)
        sound = rand() % 4 + g_unk0x005373ac;
    else
        sound = rand() % 3 + 4 + g_unk0x005373ac;
    FUN_004187d0(view, (unsigned short)sound, volume, listener);
    CAR_SHAKE(view, volume);
}

// Plays the scrape sound for its strength (10 levels) and shakes the car.
// TODO: CMR2 0x00418ba0 (implemented, match 80%)
void FUN_00418ba0(unsigned int view, int strength, int listener)
{
    int level;

    if (strength > 0xccc) {
        level = FixMul(strength, 0xa0000) >> 16;
        if (level > 9)
            level = 9;
        FUN_004187d0(view, (unsigned short)(g_unk0x00537360 + level), 0x10000, listener);
        CAR_SHAKE(view, strength);
    }
}

// Plays one of the three horn sounds (random for kind 0).
// TODO: CMR2 0x00418cd0 (implemented, match 68%)
void FUN_00418cd0(unsigned int view, int kind, int listener)
{
    int sound = 0;

    if (kind == 0)
        sound = rand() % 2;
    else if (kind == 2)
        sound = 2;
    FUN_004187d0(view, (unsigned short)(g_unk0x005373a8 + sound), 0x10000, listener);
}

void FUN_00463ce0(BYTE value);

// GLOBAL: CMR2 0x00537350
int g_unk0x00537350;
// GLOBAL: CMR2 0x0053735c
int g_unk0x0053735c;

// Updates the current route block and queues its first callout.
// TODO: CMR2 0x00416f70 (implemented, match 59%)
void FUN_00416f70(int player)
{
    int remaining = 500 - FUN_0041d780();
    int block = remaining / 100;
    int slot;

    g_unk0x00537350 = -1;
    g_unk0x0053708c[player] = RallyData_FUN_00421370((BYTE *)Car_Get(player));
    if ((BYTE)RallyData_FUN_00407e70()) {
        if (remaining % 100 < 20)
            g_unk0x00537350 = 6;
        else if (block < 5)
            g_unk0x00537350 = block + 1;
        else
            g_unk0x00537350 = -1;
    }
    FUN_004176b0();
    slot = block + player * 5;
    if (player > 0 && (&g_unk0x00537190)[slot] != 0) {
        g_unk0x005371a4[slot] = 1;
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

