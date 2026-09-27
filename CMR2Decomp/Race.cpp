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
// match 73%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00417660
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
// match 34%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004176b0
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
void FUN_004b9380(unsigned int, unsigned int, unsigned int);
BYTE *FUN_0046d2d0(char *path);
char *FUN_0041f910(void);
GenericFile *FUN_0041f500(void);

// Clears the player replay slots, then builds the replay file name for the
// current game mode and loads it into slot 0.
// FUNCTION: CMR2 0x0041c510
void FUN_0041c510(void)
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
void FUN_0041fc50(void)
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
// match 70%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00417780
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
// match 37%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00416670
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

    pCallFlag = &g_unk0x00537248;
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
                if (flagA == 0)
                    Sprite_Queue(&rectIcon, &rectMain, (Texture *)texA, 2, 0, NULL, NULL, colB, 8);
                else
                    Sprite_Queue(&rectIcon, &rectMain, (Texture *)texA, 2, 0, NULL, NULL, colB, 1);
            }
        }

        rectSecond.x = (short)(rectMain.x + rectMain.w);
        if (second != 0 && rectSecond.w != 0) {
            if (flagB != 0)
                Sprite_Queue(&rectTex, &rectSecond, (Texture *)texB, 2, 0, NULL, NULL, colA, 1);
            else
                Sprite_Queue(&rectTex, &rectSecond, (Texture *)texB, 2, 0, NULL, NULL, colA, 8);
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
int Font_GetTextWidth(unsigned int index, BYTE *text);
int Font_GetTextHeight(unsigned int index, char *text);
void Font_DrawText(unsigned int index, char *text, int x, unsigned int y, int *pColour, unsigned int flags);

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
    if (RallyData_FUN_00411880() == 0) {
        race = 0;
        pos = 0;
    } else {
        if (CGameInfo::FUN_00405dc0()) {
            if (player == 1)
                pos = (int)g_pGraphics->resY / 2;
            pos -= (int)g_pGraphics->resY * 9011 >> 16;
        } else {
            pos = (int)g_pGraphics->resY * 9011 >> 16;
        }
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
    if (x == -1) {
        flags = 0x12;
    } else {
        flags = 9;
        pos = x;
        textY = y;
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

    f4 = id >> 0xc & 7;
    f17 = id >> 0x17 & 1;
    f5 = id >> 0xf & 3;
    f6 = id >> 0x11 & 0xf;
    f7 = id >> 0x1c & 3;
    f3 = id >> 4 & 3;
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
        if (CGameInfo::FUN_004063f0(2) != 0)
            slot = g_unk0x0053735c + 8;
        else
            slot = g_unk0x0053735c + 9;
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
    b1 = id >> 4 & 3;
    b2 = id >> 0x11 & 0xf;
    b3 = id >> 0x15 & 3;

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
// match 53%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00419b50
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
// match 50%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00419b90
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
// match 54%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0041ae80
void FUN_0041ae80(int car, int unused)
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

/* --------------------------------------------------------------------------
   Race bootstrap / teardown transitions (0x41bf80..0x41e5c0).
   -------------------------------------------------------------------------- */

void FUN_0042b720(int, char);
void FUN_0041c5a0(BYTE, int);
void FUN_0046cce0(int, int, int, int);
void FUN_00403500(void);
void FUN_00424640(void);
int FUN_004582d0(int);
void FUN_00455260(void);
BYTE FUN_00478b80(void);
void FUN_00478be0(void);
void Replay_InitSlots(void);
void FUN_00422fe0(int, int, int, int);
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
int FUN_00478a20(void);
void FUN_0040a580(int, int, int);
void FUN_0040efa0(void);
void FUN_0041f560(void);
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
        FUN_0041f560();
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
        FUN_0046cce0((int)g_unk0x00537f3c[0], 0, 0, 0);
    }
    if (CGameInfo::FUN_00405d80() == 6)
        FUN_0042b720(0, -1);
    if (param2 != 0)
        return;
    FUN_00424640();
    if (CGameInfo::FUN_00405d80() == 6) {
        if (FUN_004582d0(0) < 1)
            goto done;
    } else {
        if (FUN_0041f3d0(0) != 0)
            goto done;
    }
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
void FUN_00421720(int, int, int, BYTE, int);
void FUN_00427890(void);
void FUN_004283e0(BYTE, FadeCallback, int, int, int, char);
void FUN_00428410(BYTE, int, FadeCallback, int, int, int, int, char);
void FUN_00449090(BYTE);
void FUN_0044a150(void);
BOOL FUN_004a15a0(void);
unsigned char FUN_004d0580(void);
int FUN_0046d2a0(int *);
void FUN_00480380(void);
BYTE FUN_00422fb0(unsigned int);
int FUN_00407270(void);
void FUN_0041e220(BYTE);
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
    cond = (g_unk0x00537f78[4] == (index & 0xff));
    if (CGameInfo::FUN_00405e00() != 0 && CGameInfo::FUN_00405d80() == 10)
        cond = (unsigned char)flag;
    if ((char)param2 == 0) {
        FUN_0041c5a0(*(BYTE *)(*(int *)(param1 + 4)), cond);
        if (cond == 0) {
            if (FUN_004d0580() == 0)
                goto done;
        }
        if (CGameInfo::FUN_00405e00() != 0) {
            FUN_00421720(0, 7, 0xffff, FUN_00422fb0(0), 0);
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
        FUN_00421720(param2, 7, 0xffff, FUN_00422fb0(param2), 0);
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
    if (FUN_00407270() != 0 && g_unk0x00537f3c[0] != 0) {
        if (g_unk0x00538118 == 0) {
            if (*(int *)(g_unk0x00537f3c[0] + 4) != 0)
                goto done;
            FUN_00421720(0, 7, 0xffff, FUN_00422fb0(0), 0);
            g_unk0x00538118 = 1;
            FUN_0046d2a0((int *)g_unk0x00537f3c[0]);
            FUN_0046cce0((int)g_unk0x00537f3c[0], 0, 0, 0);
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
            FUN_00428410(i, 0xc8000, FUN_0041e220, flag, 2, 0, *(unsigned int *)colour, 0);
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
// match 80%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
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
// match 57%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00418c30
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
// match 80%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00418ba0
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
// match 68%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00418cd0
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
// match 59%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00416f70
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

