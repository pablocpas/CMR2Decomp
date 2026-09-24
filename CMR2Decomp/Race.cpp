#include <windows.h>
#include "Game.h"
#include "RallyData.h"
#include "GameInfo.h"
#include "StageUI.h"
#include "FixedPoint.h"
#include <stdio.h>
#include "InstallInfo.h"
#include "Frontend.h"
#include "Graphics.h"
#include "StageTiming.h"
#include "GenericFileLoader.h"

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
// GLOBAL: CMR2 0x00538858
BYTE g_unk0x00538858[8];
// GLOBAL: CMR2 0x00538860
int g_unk0x00538860;
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

// FUNCTION: CMR2 0x0041f500
BYTE *FUN_0041f500(void)
{
    return g_unk0x00538860 ? g_unk0x00538858 : NULL;
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

// FUNCTION: CMR2 0x00417e60
void FUN_00417e60(void)
{
    g_unk0x00537190 = 0;
}

int FUN_004781c0(int index);
int FUN_004b7790(unsigned short id, int volume, int frequency, int loopStart, int loops, int is3D);
void Sound_Free(unsigned int handle);

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
void FUN_004b79a0(unsigned int handle, int volume);
void Sound_Free(unsigned int handle);

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
