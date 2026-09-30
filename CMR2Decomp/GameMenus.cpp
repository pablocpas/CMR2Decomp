#include <windows.h>
#include <mmsystem.h>
#include "GameMenus.h"
#include "GameInfo.h"
#include "Frontend.h"
#include "Font.h"
#include "RallyTiming.h"
#include "StageTiming.h"
#include <cstdio>
#include "Sprite.h"
#include "GenericFileLoader.h"
#include "Graphics.h"
#include "RallyData.h"
#include "NetPlayers.h"
#include "Input.h"
#include "TimingUtils.h"
#include "main.h"
#include "Game.h"
#include "StageUI.h"
#include "AIHelper.h"
#include "RegKey.h"

int FUN_0040ab10(void);
int FUN_0040ab20(int index, int total);
int FUN_0040ab50(int index, int total);
unsigned int FUN_0040ab80(int index, int total);
char *FUN_0040abb0(int index, int total);
unsigned int FUN_0040abe0(int index, int total);
void GameMenus_DrawRowFrame(short row, short yOffset, char compact);
void GameMenus_DrawTextRow(int x, int y, char *pText, ...);
void FormatCentisecondsAsMinSecMSec(int iTime, char *pcFormattedTime);
extern BYTE g_menuRowFillColour[4];
extern char g_stageNumberFormat[];
extern char g_stageResultSameTime[];
extern BYTE g_unk0x00540898;
extern BYTE g_unk0x00541210;
extern int g_unk0x0053f5a8;
extern int g_unk0x005413f8;
extern int g_unk0x00540c60;
extern int g_unk0x0053e698;
extern char g_classRowHeaderFormat[];
BYTE FUN_00407fc0(int param1);
int FUN_004483c0(int index);
void FUN_00451890(Menu *pMenu);

// GLOBAL: CMR2 0x0053e2d8
Menu g_menu0x0053e2d8;
// GLOBAL: CMR2 0x0053e4b8
Menu g_menu0x0053e4b8;
// GLOBAL: CMR2 0x0053e6a0
Menu g_menu0x0053e6a0;
// GLOBAL: CMR2 0x0053e888
Menu g_menu0x0053e888;
// GLOBAL: CMR2 0x0053ea68
Menu g_menu0x0053ea68;
// GLOBAL: CMR2 0x0053ec48
Menu g_menu0x0053ec48;
// GLOBAL: CMR2 0x0053ee28
Menu g_menu0x0053ee28;
// GLOBAL: CMR2 0x0053f008
Menu g_menu0x0053f008;
// GLOBAL: CMR2 0x0053f1e8
Menu g_menu0x0053f1e8;
// GLOBAL: CMR2 0x0053f3c8
Menu g_menu0x0053f3c8;
// GLOBAL: CMR2 0x0053f5b0
Menu g_menu0x0053f5b0;
// GLOBAL: CMR2 0x0053f790
Menu g_menu0x0053f790;
// GLOBAL: CMR2 0x0053f970
Menu g_menu0x0053f970;
// GLOBAL: CMR2 0x0053fb70
Menu g_menu0x0053fb70;
// GLOBAL: CMR2 0x0053fd58
Menu g_menu0x0053fd58;
// GLOBAL: CMR2 0x0053ff38
Menu g_menu0x0053ff38;
// GLOBAL: CMR2 0x00540118
Menu g_menu0x00540118;
// GLOBAL: CMR2 0x005402f8
Menu g_menu0x005402f8;
// GLOBAL: CMR2 0x005404d8
Menu g_menu0x005404d8;
// GLOBAL: CMR2 0x005406b8
Menu g_menu0x005406b8;
// GLOBAL: CMR2 0x005408a0
Menu g_menu0x005408a0;
// GLOBAL: CMR2 0x00540a80
Menu g_menu0x00540a80;
// GLOBAL: CMR2 0x00540c68
Menu g_menu0x00540c68;
// GLOBAL: CMR2 0x00540e50
Menu g_menu0x00540e50;
// GLOBAL: CMR2 0x00541030
Menu g_menu0x00541030;
// GLOBAL: CMR2 0x00541218
Menu g_menu0x00541218;
// GLOBAL: CMR2 0x00541400
Menu g_menu0x00541400;
// GLOBAL: CMR2 0x005416e0
Menu g_menu0x005416e0;
// GLOBAL: CMR2 0x005418d8
Menu g_menu0x005418d8;
// GLOBAL: CMR2 0x00541ae0
Menu g_menu0x00541ae0;
// GLOBAL: CMR2 0x0053fd50
DWORD g_menuBuildTime;

typedef void (*FadeCallback)(BYTE index);
short Car_GetOrderCount(void);
void FUN_004283e0(BYTE index, FadeCallback pfnDone, int param3, int param4, int param5, char force);
BYTE *FUN_0041b390(void);
BOOL FUN_004a15a0(void);
int FUN_004a1280(void);
void FUN_004067d0(void);
void FUN_0041f280(void);
void FUN_0041f290(void);
void FUN_0041f2a0(void);
void FUN_0041f390(void);
void FUN_00404ef0(void);
void FUN_0041b3a0(void);
void FUN_00427a10(void);
void FUN_00427a30(void);
void FUN_00427a50(void);
void FUN_00427a80(void);
void FUN_0040ad20(void);
void FUN_00409bc0(void);
void FUN_00469b50(int index);
void FUN_0041f2b0(void);
int FUN_0046d2a0(int *p);
void FUN_00466080(void);
void FUN_004728b0(void);
unsigned int *RallyData_GetChampionshipState(void);
unsigned char RallyDataState(void);
unsigned int RallyData_FUN_00407ea0(void);
extern int g_unk0x00537f0c[6];
extern BYTE *g_unk0x00537f3c[8];
extern BYTE g_menuFrameColour[4];
extern Menu *g_pHeaderMenu;
extern short g_menuRect[4];
extern char g_nameSpaceFormat[];

// Fade value of the stage end menus.
// GLOBAL: CMR2 0x00519ec8
int g_unk0x00519ec8 = 0xacb49c;
// GLOBAL: CMR2 0x00541cd4
int g_unk0x00541cd4;

// Promotes every car of the race table one level (end of stage).
#define PROMOTE_CARS()                                                                  \
    for (i = 0; i < *FUN_0041b390(); i++)                                               \
        CGame::FUN_0049c1c0((Unk0049c2c0 *)FUN_0041b390(), i, 1, 3)

void FUN_0041f280(void);
void FUN_0041f290(void);

// Fade callback: promotes the cars and closes the network menu (restart).
// FUNCTION: CMR2 0x004014f0
void FUN_004014f0(BYTE index)
{
    BYTE i;

    PROMOTE_CARS();
    FUN_0041f280();
    FUN_0041f290();
    FUN_00404ef0();
}

// Fade callback: promotes the cars and closes the network menu.
// FUNCTION: CMR2 0x00401540
void FUN_00401540(BYTE index)
{
    BYTE i;

    PROMOTE_CARS();
    FUN_0041f280();
    FUN_00404ef0();
}

// Fade colour of the in-race network menu transitions.
// GLOBAL: CMR2 0x00516090
int g_unk0x00516090 = 0xacb49c;

void FUN_00427a80(void);
void FUN_0041b3a0(void);
void FUN_00427a10(void);

// Fade callback: promotes the cars and quits the (network) championship.
// FUNCTION: CMR2 0x004016b0
void FUN_004016b0(BYTE index)
{
    BYTE i;

    PROMOTE_CARS();
    if (CGameInfo::FUN_00405d80() == 4)
        *RallyData_GetChampionshipState() |= 0x800000;
    FUN_0041f2a0();
    FUN_00404ef0();
    if (CGameInfo::FUN_00405e00()) {
        CGame::FUN_004a1a90();
        FUN_004a1280();
        FUN_004067d0();
    }
}

// Fade callback: promotes the cars and leaves the championship.
// FUNCTION: CMR2 0x00401720
void FUN_00401720(BYTE index)
{
    BYTE i;

    PROMOTE_CARS();
    if (CGameInfo::FUN_00405d80() == 4)
        *RallyData_GetChampionshipState() |= 0x800000;
    FUN_0041f2a0();
    FUN_00404ef0();
}

// GLOBAL: CMR2 0x0052af54
int g_unk0x0052af54;

void FUN_00409bc0(void);
void FUN_00427a50(void);

// Item callback of the in-race "restart stage" item.
// FUNCTION: CMR2 0x00401590
void FUN_00401590(Menu *pMenu, int param)
{
    BYTE i;

    for (i = 0; (short)i < Car_GetOrderCount(); i++) {
        if (CGameInfo::FUN_00405d80() == 8 || CGameInfo::FUN_00405d80() == 1 ||
            CGameInfo::FUN_00405d80() == 0)
            FUN_004283e0(i, i != 0 ? NULL : (FadeCallback)FUN_00401540, 1, 0, g_unk0x00516090, 1);
        else
            FUN_004283e0(i, i != 0 ? NULL : (FadeCallback)FUN_00401540, 1, 0, g_unk0x0052af54, 1);
    }
    if (CGameInfo::FUN_00405e00() && FUN_004a15a0() && CGameInfo::FUN_00405d80() != 10) {
        FUN_00409bc0();
        FUN_00427a50();
    }
}

// Item callback of the network "restart" item: fades every car out.
// FUNCTION: CMR2 0x00401630
void FUN_00401630(Menu *pMenu, int param)
{
    BYTE i;

    for (i = 0; (short)i < Car_GetOrderCount(); i++)
        FUN_004283e0(i, i != 0 ? NULL : (FadeCallback)FUN_004014f0, 1, 0, g_unk0x00516090, 1);
    if (CGameInfo::FUN_00405e00() && FUN_004a15a0() && CGameInfo::FUN_00405d80() != 10)
        FUN_00427a80();
}

// Item callback of the network "quit" item.
// FUNCTION: CMR2 0x00401780
void FUN_00401780(Menu *pMenu, int param)
{
    BYTE i;

    for (i = 0; (short)i < Car_GetOrderCount(); i++)
        FUN_004283e0(i, i != 0 ? NULL : (FadeCallback)FUN_004016b0, 1, 0, g_unk0x00516090, 1);
}

// Item callback of the network "leave" item.
// FUNCTION: CMR2 0x004017e0
void FUN_004017e0(Menu *pMenu, int param)
{
    BYTE i;

    for (i = 0; (short)i < Car_GetOrderCount(); i++)
        FUN_004283e0(i, i != 0 ? NULL : (FadeCallback)FUN_00401720, 1, 0, g_unk0x00516090, 1);
    if (CGameInfo::FUN_00405e00() && FUN_004a15a0()) {
        FUN_0041b3a0();
        FUN_00427a10();
    }
}

// Fade callback of the "quit" item: ends the championship and leaves.
// FUNCTION: CMR2 0x00449020
void FUN_00449020(BYTE index)
{
    BYTE i;

    PROMOTE_CARS();
    if (CGameInfo::FUN_00405d80() == 4)
        *RallyData_GetChampionshipState() |= 0x800000;
    if (CGameInfo::FUN_00405e00()) {
        CGame::FUN_004a1a90();
        FUN_004a1280();
        FUN_004067d0();
    }
    FUN_0041f2a0();
}

// Fade callback of the network "quit" item.
// FUNCTION: CMR2 0x00449090
void FUN_00449090(BYTE index)
{
    BYTE i;

    if (CGameInfo::FUN_00404f20())
        FUN_00404ef0();
    PROMOTE_CARS();
    if (FUN_004a15a0()) {
        FUN_0041b3a0();
        FUN_00427a10();
    }
    FUN_0040ad20();
    FUN_0041f2a0();
}

// Fade callback of the "restart" item.
// FUNCTION: CMR2 0x00449ea0
void FUN_00449ea0(BYTE index)
{
    BYTE i;

    PROMOTE_CARS();
    FUN_0041f280();
}

// Fade callback of the "retry" item.
// FUNCTION: CMR2 0x00449ee0
void FUN_00449ee0(BYTE index)
{
    BYTE i;

    PROMOTE_CARS();
    FUN_0041f280();
    FUN_0041f290();
}

// Fade callback of the "continue" item.
// FUNCTION: CMR2 0x00449fe0
void FUN_00449fe0(BYTE index)
{
    FUN_0041f2b0();
    FUN_0041f390();
    CGameInfo::FUN_0049ea90(0);
}

// Menu action "quit": fades every player out, the first one quits.
// FUNCTION: CMR2 0x00449b00
void FUN_00449b00(Menu *pMenu, int param)
{
    BYTE i;

    for (i = 0; (short)i < Car_GetOrderCount(); i++)
        FUN_004283e0(i, i != 0 ? NULL : FUN_00449020, 1, 0, g_unk0x00519ec8, 1);
}

// Menu action "quit" of the network results menu.
// FUNCTION: CMR2 0x00449b60
void FUN_00449b60(Menu *pMenu, int param)
{
    FUN_004283e0(0, FUN_00449090, 1, 0, g_unk0x00519ec8, 1);
}

// FUNCTION: CMR2 0x00449b80
void FUN_00449b80(Menu *pMenu, int param)
{
    FUN_0044a000(pMenu, 0);
}

// FUNCTION: CMR2 0x00449b90
void FUN_00449b90(Menu *pMenu, int param)
{
    pMenu->cursor = pMenu->itemCount - 1;
}

// Resizes the first item of one of the standings menus: it scans the drivers
// of the current table (overall, split or stage) and keeps the worst position,
// which becomes the scroll window (five rows below it, at most ten).
// FUNCTION: CMR2 0x00449ba0
void FUN_00449ba0(Menu *pMenu, int param)
{
    int i;
    int slot;
    int worstTime;
    int worstPos;

    worstPos = 0x10;
    worstTime = 0;
    if (pMenu == &g_menu0x0053f008 || pMenu == &g_menu0x0053e6a0) {
        i = 0;
        if (CGameInfo::FUN_00405d70() > 0) {
            slot = 0xf;
            do {
                if (RallyTiming_GetOverallPositionOfDriver(slot) < worstPos)
                    worstPos = RallyTiming_GetOverallPositionOfDriver(slot);
                i++;
                slot--;
            } while (i < CGameInfo::FUN_00405d70());
        }
    }
    if (pMenu == &g_menu0x0053e4b8) {
        i = 0;
        if (CGameInfo::FUN_00405d70() > 0) {
            do {
                if (StageTiming_GetCurrentSplitPositionOfDriver(i) < worstPos)
                    worstPos = StageTiming_GetCurrentSplitPositionOfDriver(i);
                i++;
            } while (i < CGameInfo::FUN_00405d70());
        }
    }
    if (pMenu == &g_menu0x00541ae0) {
        i = 0;
        if (CGameInfo::FUN_00405d70() > 0) {
            do {
                if (RallyTiming_GetStageTimeSeconds(StageTiming_GetDriverSlot(i)) > worstTime) {
                    worstTime = RallyTiming_GetStageTimeSeconds(StageTiming_GetDriverSlot(i));
                    worstPos = RallyTiming_GetOverallPositionOfDriver(StageTiming_GetDriverSlot(i));
                }
                i++;
            } while (i < CGameInfo::FUN_00405d70());
        }
    }
    if (worstPos > 5)
        pMenu->items[0].max = worstPos - 5;
    if (pMenu->items[0].max > 10)
        pMenu->items[0].max = 10;
}

// Sets the range of the first item to the remaining races of the championship
// (zero once it is over, one as the minimum otherwise).
// FUNCTION: CMR2 0x00449ca0
void FUN_00449ca0(Menu *pMenu, int param)
{
    pMenu->items[0].max = 0;
    if (FUN_0040ab10() > 6) {
        pMenu->items[0].min = FUN_0040ab10() - 5;
    } else {
        pMenu->items[0].min = 1;
    }
}

// FUNCTION: CMR2 0x00449e90
void FUN_00449e90(Menu *pMenu, int param)
{
    Menu_SetNextAction((int)pMenu->items[0].pSubMenu);
}

// Menu action "restart".
// FUNCTION: CMR2 0x00449f30
void FUN_00449f30(Menu *pMenu, int param)
{
    BYTE i;

    for (i = 0; (short)i < Car_GetOrderCount(); i++) {
        if (CGameInfo::FUN_00405d80() == 8 || CGameInfo::FUN_00405d80() == 1 || CGameInfo::FUN_00405d80() == 0)
            FUN_004283e0(i, i != 0 ? NULL : FUN_00449ea0, 1, 0, g_unk0x00519ec8, 1);
        else
            FUN_004283e0(i, i != 0 ? NULL : FUN_00449ea0, 1, 0, g_unk0x00541cd4, 1);
    }
    if (CGameInfo::FUN_00405e00() && FUN_004a15a0() && CGameInfo::FUN_00405d80() != 10 &&
        CGameInfo::FUN_00405d80() != 12) {
        FUN_00409bc0();
        FUN_00427a50();
    }
}

// Menu action "continue".
// FUNCTION: CMR2 0x0044a000
void FUN_0044a000(Menu *pMenu, int param1)
{
    BYTE i;

    if (CGameInfo::FUN_00405e00()) {
        FUN_004283e0(0, FUN_00449fe0, 1, 0, g_unk0x00519ec8, 1);
        if (FUN_004a15a0())
            FUN_00427a30();
        return;
    }
    for (i = 0; (short)i < Car_GetOrderCount(); i++)
        FUN_004283e0(i, i != 0 ? NULL : FUN_00449fe0, 1, 0, g_unk0x00519ec8, 1);
}

// Leaves the stage: stores every car's timing, updates the race table and
// starts the next step of the championship.
// FUNCTION: CMR2 0x0041f2b0
void FUN_0041f2b0(void)
{
    int i;

    for (i = 0; i < (BYTE)RallyDataState(); i++)
        FUN_00469b50(i);
    for (i = 0; i < *(BYTE *)g_unk0x00537f0c[5]; i++) {
        CGame::FUN_0049c1c0((Unk0049c2c0 *)g_unk0x00537f0c[5], i, 0, 2);
        FUN_0046d2a0((int *)g_unk0x00537f3c[i]);
    }
    if ((BYTE)RallyData_FUN_00407ea0() && (BYTE)CGameInfo::FUN_00406310())
        FUN_00466080();
    if (CGameInfo::FUN_00405d80() != 4)
        return;
    if (*RallyData_GetChampionshipState() & 0x800000) {
        FUN_0041f2a0();
        return;
    }
    FUN_004728b0();
    FUN_0041f280();
}

// FUNCTION: CMR2 0x0044a090
void FUN_0044a090(Menu *pMenu, int param)
{
    FUN_0041f2b0();
    CGameInfo::FUN_0049ea90(0);
}

// Menu action "retry".
// FUNCTION: CMR2 0x0044a0a0
void FUN_0044a0a0(Menu *pMenu, int param)
{
    BYTE i;

    for (i = 0; (short)i < Car_GetOrderCount(); i++)
        FUN_004283e0(i, i != 0 ? NULL : FUN_00449ee0, 1, 0, g_unk0x00519ec8, 1);
    if (CGameInfo::FUN_00405e00() && FUN_004a15a0() && CGameInfo::FUN_00405d80() != 10)
        FUN_00427a80();
}

int FUN_0041f3a0(void);
BYTE FUN_00422fb0(BYTE index);
char *FUN_004736b0(KnockoutMatch *pMatch, int side);
int FUN_00473790(KnockoutMatch *pMatch, int side);
int FUN_004737d0(KnockoutMatch *pMatch, int param2);
char *FUN_00473810(KnockoutMatch *pMatch, int side);
void FUN_004125a0(int id);
unsigned int RallyData_FUN_00407e70(void);
unsigned int RallyData_FUN_00407e90(void);
extern KnockoutMatch *g_pKnockoutMatch;

// Draw callback of the in-race pause header: "PAUSED" followed by a separator
// bar and the name of the driver (or car) the pause menu belongs to.
// match 89%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0044b7b0
void FUN_0044b7b0(Menu *pMenu)
{
    int x;
    int width;
    char result;

    Font_DrawText(2, CFrontend::GetTextString(0x47), (int)(g_pGraphics->resX * 30) / 640,
                  (int)(g_pGraphics->resY * 0x43) / 480, (int *)g_menuFrameColour, 0x11);
    width = g_pGraphics->resX;
    x = (width * 30) / 640 + Font_GetTextWidth(2, (BYTE *)CFrontend::GetTextString(0x47));
    x += (width * 8) / 640;
    g_menuRect[0] = (short)x;
    g_menuRect[1] = (short)((int)(g_pGraphics->resY * 0x25) / 480);
    g_menuRect[2] = 2;
    g_menuRect[3] = (short)((int)(g_pGraphics->resY * 0x29) / 480);
    x += 2 + (int)(g_pGraphics->resX * 8) / 640;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, g_menuFrameColour, 2);
    if (FUN_0041f3a0()) {
        if (CGameInfo::FUN_00405d80() == 4)
            Font_DrawText(2, FUN_00473810(g_pKnockoutMatch, FUN_00422fb0(1)), x,
                          (int)(g_pGraphics->resY * 0x43) / 480, (int *)g_menuFrameColour, 0x11);
        else
            Font_DrawText(2, (char *)RallyData_GetRecord(FUN_00422fb0(1)), x,
                          (int)(g_pGraphics->resY * 0x43) / 480, (int *)g_menuFrameColour, 0x11);
    } else if ((BYTE)RallyData_FUN_00407e70() && (BYTE)RallyDataState() == 1) {
        if (RallyData_FUN_00408500(FUN_00422fb0(0)) == -1)
            Font_DrawText(2, (char *)RallyData_GetRecord(FUN_00422fb0(0)), x,
                          (int)(g_pGraphics->resY * 0x43) / 480, (int *)g_menuFrameColour, 0x11);
        else
            Font_DrawText(2, CAIHelper::GetNameForID(FUN_00422fb0(0)), x,
                          (int)(g_pGraphics->resY * 0x43) / 480, (int *)g_menuFrameColour, 0x11);
    } else if (CGameInfo::FUN_00405d80() == 4) {
        if (FUN_00473790(g_pKnockoutMatch, FUN_004737d0(g_pKnockoutMatch, FUN_00422fb0(0))))
            Font_DrawText(2, FUN_00473810(g_pKnockoutMatch, FUN_004737d0(g_pKnockoutMatch, FUN_00422fb0(0))), x,
                          (int)(g_pGraphics->resY * 0x43) / 480, (int *)g_menuFrameColour, 0x11);
        else
            Font_DrawText(2, FUN_004736b0(g_pKnockoutMatch, FUN_004737d0(g_pKnockoutMatch, FUN_00422fb0(0))), x,
                          (int)(g_pGraphics->resY * 0x43) / 480, (int *)g_menuFrameColour, 0x11);
    } else if ((BYTE)RallyData_FUN_00407e90() && !CGameInfo::FUN_00405e00()) {
        FUN_004125a0(StageTiming_FUN_00455ac0(FUN_0041b370(), FUN_00422fb0(0)));
        Font_DrawText(2, CFrontend::m_stringDest, x, (int)(g_pGraphics->resY * 0x43) / 480, (int *)g_menuFrameColour, 0x11);
        Font_SetBlendMode(2);
        return;
    } else {
        result = RallyData_FUN_00408500(FUN_00422fb0(0) + FUN_0041b370());
        if (result == -1)
            Font_DrawText(2, (char *)RallyData_GetRecord(FUN_00422fb0(0) + FUN_0041b370()), x,
                          (int)(g_pGraphics->resY * 0x43) / 480, (int *)g_menuFrameColour, 0x11);
        else
            Font_DrawText(2, CAIHelper::GetNameForID(RallyData_FUN_00408500(FUN_00422fb0(0) + FUN_0041b370())), x,
                          (int)(g_pGraphics->resY * 0x43) / 480, (int *)g_menuFrameColour, 0x11);
    }
    Font_SetBlendMode(2);
}

// Draw callback of the "waiting for the other players" screen.
// FUNCTION: CMR2 0x0044bc30
void FUN_0044bc30(Menu *pMenu)
{
    Font_DrawText(2, CFrontend::GetTextString(0x47), (int)(g_pGraphics->resX * 30) / 640,
                  (int)(g_pGraphics->resY * 0x43) / 480, (int *)g_menuFrameColour, 0x11);
    Font_DrawText(2, CFrontend::GetTextString(0xfa), (int)g_pGraphics->resX / 2, (int)g_pGraphics->resY / 2,
                  (int *)g_menuFrameColour, 0x12);
    Font_SetBlendMode(2);
}

unsigned int FUN_00448680(int index, int split);
int GetStageSplitCount(void);
extern char g_minSecMSECFormatString[];

// Split times of the two drivers of the current arcade knockout match, side
// by side: name, split times, total.
// FUNCTION: CMR2 0x0044cdb0
void FUN_0044cdb0(void)
{
    // This caller uses full DWORD coordinate slots. DrawText consumes their
    // low signed WORDs; the x86 stdcall stack layout is identical.
    typedef void (WINAPI *DrawText32)(BYTE, char *, int, unsigned int, int *, unsigned int);
    char names[2][20];
    int car;
    int x;
    int split;
    int time;

    if (RallyData_FUN_00408500(g_pKnockoutMatch->flags & 0x1f) == -1 &&
        RallyData_FUN_00408500((g_pKnockoutMatch->flags >> 5) & 0x1f) == -1) {
        sprintf(names[0], (char *)RallyData_GetRecord(FUN_0041b370() + (g_pKnockoutMatch->flags & 0x1f)));
        sprintf(names[1], (char *)RallyData_GetRecord(FUN_0041b370() + ((g_pKnockoutMatch->flags >> 5) & 0x1f)));
    } else if (RallyData_FUN_00408500(g_pKnockoutMatch->flags & 0x1f) == -1) {
        sprintf(names[0], (char *)RallyData_GetRecord(FUN_0041b370() + (g_pKnockoutMatch->flags & 0x1f)));
        sprintf(names[1], CAIHelper::GetNameForID(RallyData_FUN_00408500((g_pKnockoutMatch->flags >> 5) & 0x1f)));
    } else {
        sprintf(names[0], (char *)RallyData_GetRecord(FUN_0041b370() + ((g_pKnockoutMatch->flags >> 5) & 0x1f)));
        sprintf(names[1], CAIHelper::GetNameForID(RallyData_FUN_00408500(g_pKnockoutMatch->flags & 0x1f)));
    }
    for (car = 0; car < 2; car++) {
        x = (int)(g_pGraphics->resX * 30) / 640 + ((int)(g_pGraphics->resX * 0x166) / 640) * car;
        ((DrawText32)Font_DrawText)(1, CFrontend::GetTextString(0x89), x, (int)(g_pGraphics->resY * 0xe6) / 480,
                      (int *)g_menuFrameColour, 0x11);
        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x9b), names[car]);
        ((DrawText32)Font_DrawText)(1, CFrontend::m_stringDest, x,
                      (int)(g_pGraphics->resY * 0xe6) / 480 - (int)(g_pGraphics->resY * 0x1e) / 480,
                      (int *)g_menuFrameColour, 0x11);
        ((DrawText32)Font_DrawText)(1, CFrontend::GetTextString(0x8a), x,
                      (int)(g_pGraphics->resY * 0x19) / 480 + (int)(g_pGraphics->resY * 0xe6) / 480 +
                          (int)(GetStageSplitCount() * g_pGraphics->resY * 0x1e) / 480,
                      (int *)g_menuFrameColour, 0x11);
        x = (int)(g_pGraphics->resX * 0xb1) / 640 + ((int)(g_pGraphics->resX * 0x166) / 640) * car;
        for (split = 0; split < GetStageSplitCount(); split++) {
            FormatCentisecondsAsMinSecMSec(FUN_00448680(car, split + 1) - FUN_00448680(car, split),
                                           CFrontend::m_stringDest);
            ((DrawText32)Font_DrawText)(1, CFrontend::m_stringDest, x,
                          (int)(g_pGraphics->resY * 0xe6) / 480 + ((int)(g_pGraphics->resY * 0x1e) / 480) * split,
                          (int *)g_menuFrameColour, 0x11);
        }
        time = FUN_004483c0(car);
        sprintf(CFrontend::m_stringDest, g_minSecMSECFormatString, FUN_004483c0(car) / 6000,
                (FUN_004483c0(car) / 100) % 60, time % 100);
        ((DrawText32)Font_DrawText)(1, CFrontend::m_stringDest, x,
                      (int)(g_pGraphics->resY * 0x19) / 480 + (int)(g_pGraphics->resY * 0xe6) / 480 +
                          ((int)(g_pGraphics->resY * 0x1e) / 480) * split,
                      (int *)g_menuFrameColour, 0x11);
    }
}

int FUN_00405610(void);
int FUN_00427660(void);
int FUN_00427670(void);
int FUN_004481f0(int car, int index);
unsigned int RallyData_FUN_00406950(void);
unsigned int RallyData_FUN_00406990(void);
extern BYTE g_menuTextColour[4];
extern char g_noTimeText[];

// GLOBAL: CMR2 0x00519f48
char g_recordNameFormat[] = "%s, ";
// GLOBAL: CMR2 0x00519f50
char g_recordHeaderFormat[] = "%s,  ";

// Name of the ghost car (arcade record holder) shown in the time trial table.
// GLOBAL: CMR2 0x00519ee0
char g_ghostName[4] = "cps";
// Split times of the ghost car.
// GLOBAL: CMR2 0x00541ab8
int g_ghostSplits[10];

// Draw callback of the split times screen: the country flag in the corner,
// then per car the name, the time of every split and the total; in arcade
// mode the record holder and record time head the list, in the knockout mode
// FUN_0044cdb0 draws the two drivers of the match.
// match 69%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0044bcd0
void FUN_0044bcd0(Menu *pMenu)
{
    short rect[4];
    Texture *pFlag;
    GameInfo0xa4 *pInfo;
    BYTE *pColour;
    BOOL isGhost;
    char *pName;
    int yAdjust;
    int header;
    int count;
    int car;
    int x;
    int width;
    int yOffset;
    int split;
    int total;
    int base;

    yAdjust = 0;
    header = 0;
    FUN_0044b760();
    if (FUN_00405610()) {
        rect[2] = ((Texture *)FUN_00405610())->width;
        rect[3] = ((Texture *)FUN_00405610())->height;
        rect[0] = (short)((int)(g_pGraphics->resX * 0x234) / 640 - rect[2] / 2);
        rect[1] = (short)((int)(g_pGraphics->resY * 0x30) / 480 - rect[3] / 2);
        pFlag = (Texture *)FUN_00405610();
        Sprite_Queue((SpriteRect *)((BYTE *)FUN_00405610() + 0x11c), (SpriteRect *)rect, pFlag, 2, 0, NULL, NULL,
                     g_menuFrameColour, 8);
    }
    if (CGameInfo::FUN_00405d80() == 4) {
        GameMenus_DrawTextRow((int)(g_pGraphics->resX * 30) / 640, (int)(g_pGraphics->resY * 0x43) / 480,
                              CFrontend::GetTextString(0x48), 0);
        FUN_0044cdb0();
        Font_SetBlendMode(2);
        return;
    }
    if (CGameInfo::FUN_00405d80() == 5 || CGameInfo::FUN_00405d80() == 6 || CGameInfo::FUN_00405d80() == 7 ||
        CGameInfo::FUN_00405d80() == 11) {
        FUN_00451890(pMenu);
        x = (int)(g_pGraphics->resX * 30) / 640;
        sprintf(CFrontend::m_stringDest, g_recordHeaderFormat, CFrontend::GetTextString(0x6d));
        CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
        Font_DrawText(0, CFrontend::m_stringDest, x,
                      (int)(g_pGraphics->resY * 0x91) / 480 - (int)(g_pGraphics->resY * 0x32) / 480,
                      (int *)g_menuFrameColour, 0x11);
        x += Font_GetTextWidth(0, (BYTE *)CFrontend::m_stringDest);
        CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
        pInfo = CGameInfo::FUN_00405fe0();
        sprintf(CFrontend::m_stringDest, g_recordNameFormat,
                pInfo->arcadeRecordTimes[(RallyData_FUN_00406940() & 0xff) * 3 + (RallyData_FUN_00406950() & 0xff)].ident);
        Font_DrawText(0, CFrontend::m_stringDest, x,
                      (int)(g_pGraphics->resY * 0x91) / 480 - (int)(g_pGraphics->resY * 0x32) / 480,
                      (int *)g_menuFrameColour, 0x11);
        width = Font_GetTextWidth(0, (BYTE *)CFrontend::m_stringDest);
        CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
        pInfo = CGameInfo::FUN_00405fe0();
        FormatCentisecondsAsMinSecMSec(
            (pInfo->arcadeRecordTimes[(RallyData_FUN_00406940() & 0xff) * 3 + (RallyData_FUN_00406950() & 0xff)].value >>
             7) & 0xffff,
            CFrontend::m_stringDest);
        Font_DrawText(0, CFrontend::m_stringDest, x + width,
                      (int)(g_pGraphics->resY * 0x91) / 480 - (int)(g_pGraphics->resY * 0x32) / 480,
                      (int *)g_menuFrameColour, 0x11);
        yOffset = (10 - (RallyData_FUN_00406990() & 0xff)) * ((int)(g_pGraphics->resY * 10) / 480) -
                  (int)(g_pGraphics->resY * 0x12) / 480;
        if ((BYTE)RallyData_FUN_00406990() == 10)
            yOffset += (int)(g_pGraphics->resY * -10) / 480;
        for (car = 0; car < (int)(RallyDataState() & 0xff); car++) {
            x = (int)(g_pGraphics->resX * 30) / 640 + ((int)(g_pGraphics->resX * 0x166) / 640) * car;
            if ((BYTE)RallyData_FUN_00406990() != 10)
                Font_DrawText(1, CFrontend::GetTextString(0xac), x, (int)(g_pGraphics->resY * 0x91) / 480 + yOffset,
                              (int *)g_menuFrameColour, 0x11);
            sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x9b),
                    (char *)RallyData_GetRecord(FUN_0041b370() + car));
            Font_DrawText(1, CFrontend::m_stringDest, x,
                          (int)(g_pGraphics->resY * 0x1e) / 480 + yOffset + (int)(g_pGraphics->resY * 0x91) / 480,
                          (int *)g_menuFrameColour, 0x11);
            Font_DrawText(1, CFrontend::GetTextString(0x8a), x,
                          (int)(g_pGraphics->resY * 0x91) / 480 + yOffset +
                              ((RallyData_FUN_00406990() & 0xff) + 1) * ((int)(g_pGraphics->resY * 0x1e) / 480),
                          (int *)g_menuFrameColour, 0x11);
            total = 0;
            x = (int)(g_pGraphics->resX * 0xb1) / 640 + ((int)(g_pGraphics->resX * 0x166) / 640) * car;
            if (!CGameInfo::FUN_00405e00()) {
                for (split = 1; split < (int)(RallyData_FUN_00406990() & 0xff) + 1; split++) {
                    FormatCentisecondsAsMinSecMSec(FUN_004481f0(car, split), CFrontend::m_stringDest);
                    Font_DrawText(1, CFrontend::m_stringDest, x,
                                  (int)(g_pGraphics->resY * 0x91) / 480 + yOffset +
                                      ((int)(g_pGraphics->resY * 0x1e) / 480) * split,
                                  (int *)g_menuFrameColour, 0x11);
                    total += FUN_004481f0(car, split);
                }
            } else {
                for (split = 1; split <= FUN_00427670(); split++) {
                    FormatCentisecondsAsMinSecMSec(FUN_004481f0(0, split), CFrontend::m_stringDest);
                    Font_DrawText(1, CFrontend::m_stringDest, x,
                                  (int)(g_pGraphics->resY * 0x91) / 480 + yOffset +
                                      ((int)(g_pGraphics->resY * 0x1e) / 480) * split,
                                  (int *)g_menuFrameColour, 0x11);
                }
                for (split = FUN_00427670() + 1; split < (int)(RallyData_FUN_00406990() & 0xff) + 1; split++) {
                    if (split > 0) {
                        sprintf(CFrontend::m_stringDest, g_noTimeText);
                        Font_DrawText(1, CFrontend::m_stringDest, x,
                                      (int)(g_pGraphics->resY * 0x91) / 480 + yOffset +
                                          ((int)(g_pGraphics->resY * 0x1e) / 480) * split,
                                      (int *)g_menuFrameColour, 0x11);
                    }
                }
                total = FUN_004483c0(0);
            }
            FormatCentisecondsAsMinSecMSec(total, CFrontend::m_stringDest);
            Font_DrawText(1, CFrontend::m_stringDest, x,
                          (int)(g_pGraphics->resY * 0x91) / 480 + yOffset +
                              ((int)(g_pGraphics->resY * 0x1e) / 480) * split,
                          (int *)g_menuFrameColour, 0x11);
        }
    } else {
        GameMenus_DrawTextRow((int)(g_pGraphics->resX * 30) / 640, (int)(g_pGraphics->resY * 0x43) / 480,
                              CFrontend::GetTextString(0x48), 0);
        count = 2;
        if (CGameInfo::FUN_00405e00())
            count = 1;
        for (car = 0; car < count; car++) {
            if ((BYTE)RallyDataState() == 1 && car == 1) {
                pColour = g_menuTextColour;
                isGhost = TRUE;
            } else {
                pColour = g_menuFrameColour;
                isGhost = FALSE;
            }
            x = (int)(g_pGraphics->resX * 30) / 640 + ((int)(g_pGraphics->resX * 0x166) / 640) * car;
            if (GetStageSplitCount() == 2) {
                yAdjust = (int)(g_pGraphics->resY * -0x1e) / 480;
                header = (int)(g_pGraphics->resY * 0xe6) / 480 - (int)(g_pGraphics->resY * 0x91) / 480;
            }
            if (isGhost) {
                sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x6d));
                CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
                base = CGameInfo::FUN_00405d80() == 4 ? g_pGraphics->resY * 0xe6 : g_pGraphics->resY * 0x91;
                Font_DrawText(1, CFrontend::m_stringDest, x, base / 480 + header + yAdjust, (int *)pColour, 0x11);
                pName = g_ghostName;
            } else {
                base = CGameInfo::FUN_00405d80() == 4 ? g_pGraphics->resY * 0xe6 : g_pGraphics->resY * 0x91;
                Font_DrawText(1, CFrontend::GetTextString(0x89), x, base / 480 + header + yAdjust, (int *)pColour, 0x11);
                pName = (char *)RallyData_GetRecord(FUN_0041b370() + car);
            }
            sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x9b), pName);
            base = CGameInfo::FUN_00405d80() == 4 ? g_pGraphics->resY * 0xe6 : g_pGraphics->resY * 0x91;
            Font_DrawText(1, CFrontend::m_stringDest, x,
                          (int)(g_pGraphics->resY * 0x1e) / 480 + base / 480 + header + yAdjust, (int *)pColour, 0x11);
            base = CGameInfo::FUN_00405d80() == 4 ? g_pGraphics->resY * 0xe6 : g_pGraphics->resY * 0x91;
            Font_DrawText(1, CFrontend::GetTextString(0x8a), x,
                          (int)(g_pGraphics->resY * 0x19) / 480 + base / 480 +
                              (int)(GetStageSplitCount() * g_pGraphics->resY * 0x1e) / 480 + header,
                          (int *)pColour, 0x11);
            x = (int)(g_pGraphics->resX * 0xb1) / 640 + ((int)(g_pGraphics->resX * 0x166) / 640) * car;
            if (!CGameInfo::FUN_00405e00() || isGhost) {
                for (split = 0; split < GetStageSplitCount(); split++) {
                    if (isGhost)
                        total = g_ghostSplits[split + 1] - g_ghostSplits[split];
                    else
                        total = FUN_00448680(car, split + 1) - FUN_00448680(car, split);
                    FormatCentisecondsAsMinSecMSec(total, CFrontend::m_stringDest);
                    Font_DrawText(1, CFrontend::m_stringDest, x,
                                  (int)(g_pGraphics->resY * 0x91) / 480 + header +
                                      ((int)(g_pGraphics->resY * 0x1e) / 480) * split,
                                  (int *)pColour, 0x11);
                }
                if (isGhost) {
                    sprintf(CFrontend::m_stringDest, g_minSecMSECFormatString,
                            g_ghostSplits[GetStageSplitCount()] / 6000,
                            (g_ghostSplits[GetStageSplitCount()] / 100) % 60, g_ghostSplits[GetStageSplitCount()] % 100);
                    goto draw_total;
                }
            } else {
                for (split = 0; split < FUN_00427660(); split++) {
                    FormatCentisecondsAsMinSecMSec(FUN_00448680(car, split + 1) - FUN_00448680(car, split),
                                                   CFrontend::m_stringDest);
                    Font_DrawText(1, CFrontend::m_stringDest, x,
                                  (int)(g_pGraphics->resY * 0x91) / 480 + header +
                                      ((int)(g_pGraphics->resY * 0x1e) / 480) * split,
                                  (int *)pColour, 0x11);
                }
                for (split = FUN_00427660(); split < GetStageSplitCount(); split++) {
                    if (split > -1) {
                        sprintf(CFrontend::m_stringDest, g_noTimeText);
                        Font_DrawText(1, CFrontend::m_stringDest, x,
                                      (int)(g_pGraphics->resY * 0x91) / 480 + header +
                                          ((int)(g_pGraphics->resY * 0x1e) / 480) * split,
                                      (int *)pColour, 0x11);
                    }
                }
            }
            sprintf(CFrontend::m_stringDest, g_minSecMSECFormatString, FUN_004483c0(car) / 6000,
                    (FUN_004483c0(car) / 100) % 60, FUN_004483c0(car) % 100);
        draw_total:
            Font_DrawText(1, CFrontend::m_stringDest, x,
                          (int)(g_pGraphics->resY * 0x19) / 480 + (int)(g_pGraphics->resY * 0x91) / 480 + header +
                              ((int)(g_pGraphics->resY * 0x1e) / 480) * split,
                          (int *)pColour, 0x11);
        }
    }
    Font_SetBlendMode(2);
}

char FUN_00420190(void);
int FUN_00448390(int index);
int FUN_00407270(void);
void GameMenus_DrawTextRow(int x, int y, char *pText, ...);
void FUN_0044b760(void);
extern char g_standingsRowFormat[];

// GLOBAL: CMR2 0x00519edc
char g_unk0x00519edc[4] = "jml";
// GLOBAL: CMR2 0x00519f68
char g_winnerFormat[] = "%s, %s";

// Position of the car in the stage order, or -1.
// FUNCTION: CMR2 0x00451850
int FUN_00451850(int car)
{
    int i;

    for (i = 0; i < (BYTE)FUN_00420190(); i++) {
        if (FUN_00448390(i) == car)
            return i;
    }
    return -1;
}

// Draw callback of the stage winner screen.
// FUNCTION: CMR2 0x0044d790
void FUN_0044d790(Menu *pMenu)
{
    int best;
    int winner;
    int i;
    int resY;

    best = 0x10;
    winner = -1;
    FUN_0044b760();
    if (CGameInfo::FUN_00405d80() != 5 && CGameInfo::FUN_00405d80() != 6) {
        GameMenus_DrawTextRow((int)(g_pGraphics->resX * 30) / 640, (int)(g_pGraphics->resY * 242) / 480,
                              CFrontend::GetTextString(0x49), CFrontend::GetTextString(0x46), 0);
        sprintf(CFrontend::m_stringDest, g_winnerFormat, g_unk0x00519edc, CFrontend::GetTextString(0x4a));
    } else {
        GameMenus_DrawTextRow((int)(g_pGraphics->resX * 30) / 640, (int)(g_pGraphics->resY * 242) / 480,
                              CFrontend::GetTextString(0x49), 0);
        for (i = 0; i < (BYTE)RallyDataState(); i++) {
            if (FUN_00451850(i) < best) {
                best = FUN_00451850(i);
                winner = i;
            }
        }
        sprintf(CFrontend::m_stringDest, g_winnerFormat, (char *)RallyData_GetRecord(winner),
                CFrontend::GetTextString(0x4a));
    }
    CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
    resY = g_pGraphics->resY;
    Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 30) / 640,
                  (resY * 10) / 480 + (resY * 242) / 480 + Font_GetLineHeight(0), (int *)g_menuFrameColour, 0x11);
    Font_SetBlendMode(2);
}

// Draw callback of the scrolling stage split table: driver, split time, car
// and position ("=" when the time equals the previous row's).
// FUNCTION: CMR2 0x0044d960
void FUN_0044d960(Menu *pMenu)
{
    char stage[80];
    bool isPlayer;
    int rows;
    int row;
    int pos;
    int id;
    int time;
    int player;

    rows = (g_unk0x005413f8 | g_unk0x0053f5a8) ? 5 : 6;
    FUN_0044b760();
    if (CGameInfo::FUN_00405d80() == 0 || CGameInfo::FUN_00405d80() == 1) {
        if (RallyDataStageIndex() == 10)
            sprintf(stage, CRegKey::m_regKeyPathFormatValue, CFrontend::GetTextString(0xba));
        else
            sprintf(stage, g_classRowHeaderFormat, CFrontend::GetTextString(0x40), RallyDataStageIndex() + 1);
        GameMenus_DrawTextRow((int)(g_pGraphics->resX * 30) / 640, (int)(g_pGraphics->resY * 0x43) / 480,
                              CFrontend::GetTextString(RallyDataCountryIndex() & 0xff),
                              CInput::FormatString(g_classRowHeaderFormat, CFrontend::GetTextString(0x44),
                                                   (RallyDataStageIndex() >> 2) + 1),
                              stage, 0);
    } else if (RallyDataStageIndex() == 10) {
        GameMenus_DrawTextRow((int)(g_pGraphics->resX * 30) / 640, (int)(g_pGraphics->resY * 0x43) / 480,
                              CFrontend::GetTextString(RallyDataCountryIndex() & 0xff), CFrontend::GetTextString(0xba), 0);
    } else {
        GameMenus_DrawTextRow((int)(g_pGraphics->resX * 30) / 640, (int)(g_pGraphics->resY * 0x43) / 480,
                              CFrontend::GetTextString(RallyDataCountryIndex() & 0xff),
                              CInput::FormatString(g_classRowHeaderFormat, CFrontend::GetTextString(0x40),
                                                   RallyDataStageIndex() + 1),
                              0);
    }
    for (row = 0; row < rows; row++) {
        if (g_unk0x0053f5a8 != 0) {
            row++;
            pos = (pMenu->items[0].max - 1) + row;
        } else {
            if (g_unk0x005413f8 != 0)
                row++;
            pos = pMenu->items[0].max + row;
        }
        id = StageTiming_GetDriverIDForPosition(pos);
        time = StageTiming_GetTimeForPosition(pos);
        isPlayer = false;
        for (player = 0; player < CGameInfo::FUN_00405d70(); player++) {
            if (StageTiming_GetCurrentSplitPositionOfDriver(player) == pos) {
                isPlayer = true;
                strcpy(CFrontend::m_stringDest, (char *)RallyData_GetRecord(player));
                goto found;
            }
        }
        strcpy(CFrontend::m_stringDest, CAIHelper::GetNameForID(id));
    found:
        Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x5c) / 640,
                      (((int)(g_pGraphics->resY * 0x82) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * row) -
                       (int)(g_pGraphics->resY * 8) / 480) - g_unk0x00540c60,
                      (int *)g_menuFrameColour, 9);
        FormatCentisecondsAsMinSecMSec(time, CFrontend::m_stringDest);
        Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x21e) / 640,
                      (((int)(g_pGraphics->resY * 0x82) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * row) -
                       (int)(g_pGraphics->resY * 8) / 480) - g_unk0x00540c60,
                      (int *)g_menuFrameColour, 9);
        if (isPlayer) {
            strcpy(CFrontend::m_stringDest, (char *)CFrontend::FUN_0040ede0(RallyData_FUN_004086b0(player)));
            CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
            g_menuRect[1] = (short)((int)(g_pGraphics->resY * 0x82) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * row -
                                    g_unk0x00540c60 - 1);
            g_menuRect[3] = (short)((int)(g_pGraphics->resY * 0xa3) / 480 - (int)(g_pGraphics->resY * 0x82) / 480 + 1);
            g_menuRect[0] = (short)((int)(g_pGraphics->resX * 0x20) / 640);
            g_menuRect[2] = (short)((int)(g_pGraphics->resX * 0x41) / 640 - (int)(g_pGraphics->resX * 0x20) / 640);
            Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, g_menuRowFillColour, 2);
        } else {
            strcpy(CFrontend::m_stringDest, (char *)CFrontend::FUN_0040ede0(FUN_00407fc0(id)));
            CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
            GameMenus_DrawRowFrame(row, 0, 0);
        }
        Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x5c) / 640,
                      ((int)(g_pGraphics->resY * 0xa3) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * row) -
                          g_unk0x00540c60,
                      (int *)g_menuFrameColour, 0x21);
        if (StageTiming_GetTimeForPosition(pos) == StageTiming_GetTimeForPosition(pos - 1))
            sprintf(CFrontend::m_stringDest, g_stageResultSameTime);
        else
            sprintf(CFrontend::m_stringDest, g_stageNumberFormat, pos + 1);
        Font_DrawText(1, CFrontend::m_stringDest,
                      ((int)(g_pGraphics->resX * 0x41) / 640 - (int)(g_pGraphics->resX * 0x20) / 640) / 2 +
                          (int)(g_pGraphics->resX * 0x20) / 640,
                      (((int)(g_pGraphics->resY * 8) / 480 +
                        ((int)(g_pGraphics->resY * 0xa3) / 480 - (int)(g_pGraphics->resY * 0x82) / 480) / 2 +
                        ((int)(g_pGraphics->resY * 0x34) / 480) * row) -
                       g_unk0x00540c60) + (int)(g_pGraphics->resY * 0x82) / 480,
                      (int *)g_menuFrameColour, 0x12);
        if (g_unk0x0053f5a8 != 0 || g_unk0x005413f8 != 0)
            row--;
    }
    Font_SetBlendMode(2);
}

extern char g_classRowHeaderFormat[];

// Draw callback of the stage times table (table 0), with the rally / stage
// header (in championship mode 8 also the leg number).
// FUNCTION: CMR2 0x0044e130
void FUN_0044e130(Menu *pMenu)
{
    char stage[80];
    int i;

    FUN_0044b760();
    if (CGameInfo::FUN_00405d80() == 8) {
        if (RallyDataStageIndex() == 10)
            sprintf(stage, CRegKey::m_regKeyPathFormatValue, CFrontend::GetTextString(0xba));
        else
            sprintf(stage, g_classRowHeaderFormat, CFrontend::GetTextString(0x40), RallyDataStageIndex() + 1);
        GameMenus_DrawTextRow((int)(g_pGraphics->resX * 30) / 640, (int)(g_pGraphics->resY * 0x43) / 480,
                              CFrontend::GetTextString(RallyDataCountryIndex() & 0xff),
                              CInput::FormatString(g_classRowHeaderFormat, CFrontend::GetTextString(0x44),
                                                   (RallyDataStageIndex() >> 2) + 1),
                              stage, 0);
    } else {
        GameMenus_DrawTextRow((int)(g_pGraphics->resX * 30) / 640, (int)(g_pGraphics->resY * 0x43) / 480,
                              CFrontend::GetTextString(RallyDataCountryIndex() & 0xff),
                              RallyDataStageIndex() == 10
                                  ? CFrontend::GetTextString(0xba)
                                  : CInput::FormatString(g_classRowHeaderFormat, CFrontend::GetTextString(0x40),
                                                         RallyDataStageIndex() + 1),
                              0);
    }
    i = 0;
    if (FUN_0040ab10() > 0) {
        do {
            Font_DrawText(1, FUN_0040abb0(i, 0), (int)(g_pGraphics->resX * 0x5c) / 0x280,
                          (int)(g_pGraphics->resY * 3) / 0x1e0 +
                              (((int)(g_pGraphics->resY * 0x82) / 0x1e0 +
                                ((int)(g_pGraphics->resY * 0x2d) / 0x1e0) * i) -
                               (int)(g_pGraphics->resY * 0x14) / 0x1e0) -
                              (int)(g_pGraphics->resY * 8) / 0x1e0,
                          (int *)g_menuFrameColour, 9);
            FormatCentisecondsAsMinSecMSec(FUN_0040ab80(i, 0), CFrontend::m_stringDest);
            Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x21e) / 0x280,
                          (((int)(g_pGraphics->resY * 0x82) / 0x1e0 +
                            ((int)(g_pGraphics->resY * 0x2d) / 0x1e0) * i) -
                           (int)(g_pGraphics->resY * 0x14) / 0x1e0) -
                              (int)(g_pGraphics->resY * 8) / 0x1e0,
                          (int *)g_menuFrameColour, 9);
            if (FUN_0040ab50(i, 0) == -2) {
                strcpy(CFrontend::m_stringDest,
                       (char *)CFrontend::FUN_0040ede0(FUN_0040abe0(i, 0)));
                CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
                g_menuRect[1] = (short)((((int)(g_pGraphics->resY * 0x82) / 0x1e0 +
                                          ((int)(g_pGraphics->resY * 0x2d) / 0x1e0) * i) -
                                         (int)(g_pGraphics->resY * 0x14) / 0x1e0) -
                                        1);
                g_menuRect[3] = (short)(((int)(g_pGraphics->resY * 0xa3) / 0x1e0 -
                                          (int)(g_pGraphics->resY * 0x82) / 0x1e0) +
                                        1);
                g_menuRect[0] = (short)((int)(g_pGraphics->resX * 0x20) / 0x280);
                g_menuRect[2] = (short)((int)(g_pGraphics->resX * 0x41) / 0x280 -
                                        (int)(g_pGraphics->resX * 0x20) / 0x280);
                Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, g_menuRowFillColour, 2);
            } else {
                strcpy(CFrontend::m_stringDest,
                       (char *)CFrontend::FUN_0040ede0(FUN_0040abe0(i, 0)));
                CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
                GameMenus_DrawRowFrame(i, (-(int)g_pGraphics->resY * 0x14) / 0x1e0, 1);
            }
            Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x5c) / 0x280,
                          ((int)(g_pGraphics->resY * 0xa3) / 0x1e0 +
                           ((int)(g_pGraphics->resY * 0x2d) / 0x1e0) * i) -
                              (int)(g_pGraphics->resY * 0x14) / 0x1e0,
                          (int *)g_menuFrameColour, 0x21);
            if (i == 0 || (i > 0 && FUN_0040ab20(i, 0) != FUN_0040ab20(i - 1, 0))) {
                sprintf(CFrontend::m_stringDest, g_stageNumberFormat, FUN_0040ab20(i, 0));
            } else {
                sprintf(CFrontend::m_stringDest, g_stageResultSameTime);
            }
            Font_DrawText(1, CFrontend::m_stringDest,
                          ((int)(g_pGraphics->resX * 0x41) / 0x280 -
                           (int)(g_pGraphics->resX * 0x20) / 0x280) / 2 +
                              (int)(g_pGraphics->resX * 0x20) / 0x280,
                          (int)(g_pGraphics->resY * 8) / 0x1e0 +
                              (int)(g_pGraphics->resY * 0x82) / 0x1e0 +
                              ((((int)(g_pGraphics->resY * 0xa3) / 0x1e0 -
                                 (int)(g_pGraphics->resY * 0x82) / 0x1e0) / 2 +
                                ((int)(g_pGraphics->resY * 0x2d) / 0x1e0) * i) -
                               (int)(g_pGraphics->resY * 0x14) / 0x1e0),
                          (int *)g_menuFrameColour, 0x12);
            i++;
        } while (i < FUN_0040ab10());
    }
    Font_SetBlendMode(2);
}

void GameMenus_DrawRowHighlight(short row);
BYTE FUN_00407fc0(int param1);
int FUN_004483c0(int index);
void FUN_00451890(Menu *pMenu);
void GameMenus_DrawRowSeparator(short row);
extern int g_unk0x00540c60;
extern int g_unk0x0053e698;

// Set when the standings menus of the stage end / pause screens show their
// extra first row.
// GLOBAL: CMR2 0x0053f5a8
int g_unk0x0053f5a8;
// GLOBAL: CMR2 0x005413f8
int g_unk0x005413f8;

// Draw callback of the scrolling championship points table: six rows from
// the position in item 0, each with the driver, its points, its car and its
// position; rows of the human drivers are highlighted and the top five are
// separated from the rest.
// FUNCTION: CMR2 0x0044efa0
void FUN_0044efa0(Menu *pMenu)
{
    bool isPlayer;
    int rows;
    int row;
    int pos;
    int id;
    int points;
    int player;
    int x;

    x = (int)(g_pGraphics->resX * 30) / 640;
    rows = (g_unk0x005413f8 | g_unk0x0053f5a8) ? 5 : 6;
    FUN_0044b760();
    GameMenus_DrawTextRow(x, (int)(g_pGraphics->resY * 0x43) / 480,
                          CFrontend::GetTextString(RallyDataCountryIndex() & 0xff), CFrontend::GetTextString(0x41),
                          CFrontend::GetTextString(0x8d), 0);
    for (row = 0; row < rows; row++) {
        if (g_unk0x0053f5a8 != 0) {
            row++;
            pos = (pMenu->items[0].max - 1) + row;
        } else {
            if (g_unk0x005413f8 != 0)
                row++;
            pos = pMenu->items[0].max + row;
        }
        if (pos > 5 && pMenu->items[0].max < 6)
            g_unk0x0053e698 = (int)(g_pGraphics->resY * 10) / 480;
        else
            g_unk0x0053e698 = 0;
        id = RallyTiming_GetOverallPositionDriverID(pos);
        points = RallyTiming_GetStagePenalty(id, RallyDataCountryIndex());
        isPlayer = false;
        for (player = 0; player < CGameInfo::FUN_00405d70(); player++) {
            if (RallyTiming_GetOverallPositionOfDriver(StageTiming_GetDriverSlot(player)) == pos) {
                isPlayer = true;
                strcpy(CFrontend::m_stringDest, (char *)RallyData_GetRecord(player));
                goto found;
            }
        }
        strcpy(CFrontend::m_stringDest, CAIHelper::GetNameForID(id));
    found:
        Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x5c) / 640,
                      (((int)(g_pGraphics->resY * 0x82) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * row) -
                       (int)(g_pGraphics->resY * 8) / 480) - g_unk0x00540c60 + g_unk0x0053e698,
                      (int *)g_menuFrameColour, 9);
        sprintf(CFrontend::m_stringDest, g_stageNumberFormat, points);
        Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x21e) / 640,
                      (((int)(g_pGraphics->resY * 0x82) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * row) -
                       (int)(g_pGraphics->resY * 8) / 480) - g_unk0x00540c60 + g_unk0x0053e698,
                      (int *)g_menuFrameColour, 9);
        if (isPlayer) {
            strcpy(CFrontend::m_stringDest, (char *)CFrontend::FUN_0040ede0(RallyData_FUN_004086b0(player)));
            CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
            g_menuRect[1] = (short)((int)(g_pGraphics->resY * 0x82) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * row -
                                    g_unk0x00540c60 - 1 + g_unk0x0053e698);
            g_menuRect[3] = (short)((int)(g_pGraphics->resY * 0xa3) / 480 - (int)(g_pGraphics->resY * 0x82) / 480 + 1);
            g_menuRect[0] = (short)((int)(g_pGraphics->resX * 0x20) / 640);
            g_menuRect[2] = (short)((int)(g_pGraphics->resX * 0x41) / 640 - (int)(g_pGraphics->resX * 0x20) / 640);
            Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, g_menuRowFillColour, 2);
        } else {
            if (pos < 6)
                GameMenus_DrawRowHighlight(row);
            strcpy(CFrontend::m_stringDest, (char *)CFrontend::FUN_0040ede0(FUN_00407fc0(id)));
            CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
            GameMenus_DrawRowFrame(row, 0, 0);
        }
        Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x5c) / 640,
                      ((int)(g_pGraphics->resY * 0xa3) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * row) -
                          g_unk0x00540c60 + g_unk0x0053e698,
                      (int *)g_menuFrameColour, 0x21);
        sprintf(CFrontend::m_stringDest, g_stageNumberFormat, pos + 1);
        Font_DrawText(1, CFrontend::m_stringDest,
                      ((int)(g_pGraphics->resX * 0x41) / 640 - (int)(g_pGraphics->resX * 0x20) / 640) / 2 +
                          (int)(g_pGraphics->resX * 0x20) / 640,
                      (((int)(g_pGraphics->resY * 8) / 480 +
                        ((int)(g_pGraphics->resY * 0xa3) / 480 - (int)(g_pGraphics->resY * 0x82) / 480) / 2 +
                        ((int)(g_pGraphics->resY * 0x34) / 480) * row) -
                       g_unk0x00540c60) + (int)(g_pGraphics->resY * 0x82) / 480 + g_unk0x0053e698,
                      (int *)g_menuFrameColour, 0x12);
        if (pos == 5 && pMenu->items[0].max != 0)
            GameMenus_DrawRowSeparator(row);
        if (g_unk0x0053f5a8 != 0 || g_unk0x005413f8 != 0)
            row--;
    }
    Font_SetBlendMode(2);
}

// Same as FUN_0044efa0 for the overall rally times table.
// FUNCTION: CMR2 0x0044fea0
void FUN_0044fea0(Menu *pMenu)
{
    bool isPlayer;
    int x;
    int rows;
    int row;
    int pos;
    int id;
    int time;
    int player;
    int slot;

    x = (int)(g_pGraphics->resX * 30) / 640;
    rows = (g_unk0x0053f5a8 | g_unk0x005413f8) ? 5 : 6;
    FUN_0044b760();
    if (!g_unk0x00541210 || !g_unk0x00540898)
        GameMenus_DrawTextRow(x, (int)(g_pGraphics->resY * 0x43) / 480,
                              CFrontend::GetTextString(RallyDataCountryIndex() & 0xff), CFrontend::GetTextString(0x41), 0);
    else
        GameMenus_DrawTextRow(x, (int)(g_pGraphics->resY * 0x43) / 480,
                              CFrontend::GetTextString(RallyDataCountryIndex() & 0xff), CFrontend::GetTextString(0x41),
                              CFrontend::GetTextString(0xef), 0);
    for (row = 0; row < rows; row++) {
        if (g_unk0x0053f5a8 != 0) {
            row++;
            pos = (pMenu->items[0].max - 1) + row;
        } else {
            if (g_unk0x005413f8 != 0)
                row++;
            pos = pMenu->items[0].max + row;
        }
        if (pos <= 5 || pMenu->items[0].max >= 6)
            g_unk0x0053e698 = 0;
        else
            g_unk0x0053e698 = (int)(g_pGraphics->resY * 10) / 480;
        id = RallyTiming_GetOverallPositionDriverID(pos);
        time = RallyTiming_GetOverallTimeForPosition(pos);
        isPlayer = FALSE;
        for (player = 0, slot = 0xf; player < CGameInfo::FUN_00405d70(); player++, slot--) {
            if (RallyTiming_GetOverallPositionOfDriver(slot) == pos) {
                isPlayer = TRUE;
                strcpy(CFrontend::m_stringDest, (char *)RallyData_GetRecord(player));
                goto found;
            }
        }
        strcpy(CFrontend::m_stringDest, CAIHelper::GetNameForID(id));
    found:
        Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x5c) / 640,
                      (((int)(g_pGraphics->resY * 0x82) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * row) -
                       (int)(g_pGraphics->resY * 8) / 480) - g_unk0x00540c60 + g_unk0x0053e698,
                      (int *)g_menuFrameColour, 9);
        FormatCentisecondsAsMinSecMSec(time, CFrontend::m_stringDest);
        Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x21e) / 640,
                      (((int)(g_pGraphics->resY * 0x82) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * row) -
                       (int)(g_pGraphics->resY * 8) / 480) - g_unk0x00540c60 + g_unk0x0053e698,
                      (int *)g_menuFrameColour, 9);
        if (isPlayer) {
            strcpy(CFrontend::m_stringDest, (char *)CFrontend::FUN_0040ede0(RallyData_FUN_004086b0(player)));
            CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
            g_menuRect[1] = (short)((int)(g_pGraphics->resY * 0x82) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * row -
                                    g_unk0x00540c60 - 1 + g_unk0x0053e698);
            g_menuRect[3] = (short)((int)(g_pGraphics->resY * 0xa3) / 480 - (int)(g_pGraphics->resY * 0x82) / 480 + 1);
            g_menuRect[0] = (short)((int)(g_pGraphics->resX * 0x20) / 640);
            g_menuRect[2] = (short)((int)(g_pGraphics->resX * 0x41) / 640 - (int)(g_pGraphics->resX * 0x20) / 640);
            Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, g_menuRowFillColour, 2);
        } else {
            if (pos < 6)
                GameMenus_DrawRowHighlight(row);
            strcpy(CFrontend::m_stringDest, (char *)CFrontend::FUN_0040ede0(FUN_00407fc0(id)));
            CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
            GameMenus_DrawRowFrame(row, 0, 0);
        }
        Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x5c) / 640,
                      ((int)(g_pGraphics->resY * 0xa3) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * row) -
                          g_unk0x00540c60 + g_unk0x0053e698,
                      (int *)g_menuFrameColour, 0x21);
        sprintf(CFrontend::m_stringDest, g_stageNumberFormat, pos + 1);
        Font_DrawText(1, CFrontend::m_stringDest,
                      ((int)(g_pGraphics->resX * 0x41) / 640 - (int)(g_pGraphics->resX * 0x20) / 640) / 2 +
                          (int)(g_pGraphics->resX * 0x20) / 640,
                      (((int)(g_pGraphics->resY * 8) / 480 +
                        ((int)(g_pGraphics->resY * 0xa3) / 480 - (int)(g_pGraphics->resY * 0x82) / 480) / 2 +
                        ((int)(g_pGraphics->resY * 0x34) / 480) * row) -
                       g_unk0x00540c60) + (int)(g_pGraphics->resY * 0x82) / 480 + g_unk0x0053e698,
                      (int *)g_menuFrameColour, 0x12);
        if (pos == 5 && pMenu->items[0].max != 0)
            GameMenus_DrawRowSeparator(row);
        if (g_unk0x0053f5a8 != 0 || g_unk0x005413f8 != 0)
            row--;
    }
    Font_SetBlendMode(2);
}

// Draw callback of the championship table (table 1 of the network/overall
// standings), with the rally name header.
// FUNCTION: CMR2 0x004505b0
void FUN_004505b0(Menu *pMenu)
{
    int x;
    int i;

    x = (int)(g_pGraphics->resX * 30) / 640;
    FUN_0044b760();
    if (!g_unk0x00541210 || !g_unk0x00540898)
        GameMenus_DrawTextRow(x, (int)(g_pGraphics->resY * 0x43) / 480,
                              CFrontend::GetTextString(RallyDataCountryIndex() & 0xff), CFrontend::GetTextString(0x41), 0);
    else
        GameMenus_DrawTextRow(x, (int)(g_pGraphics->resY * 0x43) / 480,
                              CFrontend::GetTextString(RallyDataCountryIndex() & 0xff), CFrontend::GetTextString(0x41),
                              CFrontend::GetTextString(0xef), 0);
    i = 0;
    if (FUN_0040ab10() > 0) {
        do {
            Font_DrawText(1, FUN_0040abb0(i, 1), (int)(g_pGraphics->resX * 0x5c) / 0x280,
                          (int)(g_pGraphics->resY * 3) / 0x1e0 +
                              (((int)(g_pGraphics->resY * 0x82) / 0x1e0 +
                                ((int)(g_pGraphics->resY * 0x2d) / 0x1e0) * i) -
                               (int)(g_pGraphics->resY * 0x14) / 0x1e0) -
                              (int)(g_pGraphics->resY * 8) / 0x1e0,
                          (int *)g_menuFrameColour, 9);
            FormatCentisecondsAsMinSecMSec(FUN_0040ab80(i, 1), CFrontend::m_stringDest);
            Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x21e) / 0x280,
                          (((int)(g_pGraphics->resY * 0x82) / 0x1e0 +
                            ((int)(g_pGraphics->resY * 0x2d) / 0x1e0) * i) -
                           (int)(g_pGraphics->resY * 0x14) / 0x1e0) -
                              (int)(g_pGraphics->resY * 8) / 0x1e0,
                          (int *)g_menuFrameColour, 9);
            if (FUN_0040ab50(i, 1) == -2) {
                strcpy(CFrontend::m_stringDest,
                       (char *)CFrontend::FUN_0040ede0(FUN_0040abe0(i, 1)));
                CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
                g_menuRect[1] = (short)((((int)(g_pGraphics->resY * 0x82) / 0x1e0 +
                                          ((int)(g_pGraphics->resY * 0x2d) / 0x1e0) * i) -
                                         (int)(g_pGraphics->resY * 0x14) / 0x1e0) -
                                        1);
                g_menuRect[3] = (short)(((int)(g_pGraphics->resY * 0xa3) / 0x1e0 -
                                          (int)(g_pGraphics->resY * 0x82) / 0x1e0) +
                                        1);
                g_menuRect[0] = (short)((int)(g_pGraphics->resX * 0x20) / 0x280);
                g_menuRect[2] = (short)((int)(g_pGraphics->resX * 0x41) / 0x280 -
                                        (int)(g_pGraphics->resX * 0x20) / 0x280);
                Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, g_menuRowFillColour, 2);
            } else {
                strcpy(CFrontend::m_stringDest,
                       (char *)CFrontend::FUN_0040ede0(FUN_0040abe0(i, 1)));
                CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
                GameMenus_DrawRowFrame(i, (-(int)g_pGraphics->resY * 0x14) / 0x1e0, 1);
            }
            Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x5c) / 0x280,
                          ((int)(g_pGraphics->resY * 0xa3) / 0x1e0 +
                           ((int)(g_pGraphics->resY * 0x2d) / 0x1e0) * i) -
                              (int)(g_pGraphics->resY * 0x14) / 0x1e0,
                          (int *)g_menuFrameColour, 0x21);
            if (i == 0 || (i > 0 && FUN_0040ab20(i, 1) != FUN_0040ab20(i - 1, 1))) {
                sprintf(CFrontend::m_stringDest, g_stageNumberFormat, FUN_0040ab20(i, 1));
            } else {
                sprintf(CFrontend::m_stringDest, g_stageResultSameTime);
            }
            Font_DrawText(1, CFrontend::m_stringDest,
                          ((int)(g_pGraphics->resX * 0x41) / 0x280 -
                           (int)(g_pGraphics->resX * 0x20) / 0x280) / 2 +
                              (int)(g_pGraphics->resX * 0x20) / 0x280,
                          (int)(g_pGraphics->resY * 8) / 0x1e0 +
                              (int)(g_pGraphics->resY * 0x82) / 0x1e0 +
                              ((((int)(g_pGraphics->resY * 0xa3) / 0x1e0 -
                                 (int)(g_pGraphics->resY * 0x82) / 0x1e0) / 2 +
                                ((int)(g_pGraphics->resY * 0x2d) / 0x1e0) * i) -
                               (int)(g_pGraphics->resY * 0x14) / 0x1e0),
                          (int *)g_menuFrameColour, 0x12);
            i++;
        } while (i < FUN_0040ab10());
    }
    Font_SetBlendMode(2);
}

// Draw callback of the championship standings screen: the best driver's
// position decides between the "champion" and "rally over" headers.
// match 89%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00450c10
void FUN_00450c10(Menu *pMenu)
{
    char position[100];
    char *pPosition;
    int x;
    int y;
    int best;
    int i;
    int slot;
    int place;
    int resY;

    x = (int)(g_pGraphics->resX * 30) / 640;
    y = (int)(g_pGraphics->resY * 242) / 480;
    FUN_0044b760();
    best = 99;
    i = 0;
    slot = best;
    if (CGameInfo::FUN_00405d70() > 0) {
        slot = 0xf;
        do {
            if (RallyTiming_GetOverallPositionOfDriver(slot) < best)
                best = RallyTiming_GetOverallPositionOfDriver(slot);
            i++;
            slot--;
        } while (i < CGameInfo::FUN_00405d70());
        slot = best;
    }
    if (slot < 6)
        GameMenus_DrawTextRow(x, y, CFrontend::GetTextString(RallyDataCountryIndex() & 0xff),
                              CFrontend::GetTextString(0x41), CFrontend::GetTextString(0x49), 0);
    else
        GameMenus_DrawTextRow(x, y, CFrontend::GetTextString(RallyDataCountryIndex() & 0xff),
                              CFrontend::GetTextString(0x41), CFrontend::GetTextString(0x88), 0);
    for (i = 0; i < CGameInfo::FUN_00405d70(); i++) {
        place = RallyTiming_GetOverallPositionOfDriver(StageTiming_GetDriverSlot(i));
        switch (place) {
        case 0:
            pPosition = CFrontend::GetTextString(0x51);
            break;
        case 1:
            pPosition = CFrontend::GetTextString(0x52);
            break;
        case 2:
            pPosition = CFrontend::GetTextString(0x53);
            break;
        default:
            sprintf(position, CFrontend::GetTextString(0x54), place + 1);
            pPosition = position;
            break;
        }
        sprintf(CFrontend::m_stringDest, g_standingsRowFormat, (char *)RallyData_GetRecord(i), pPosition);
        CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
        resY = g_pGraphics->resY;
        Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 30) / 640,
                      (resY * 10) / 480 + ((resY * 20) / 480) * i + y + Font_GetLineHeight(0),
                      (int *)g_menuFrameColour, 0x11);
    }
    if (slot > 5) {
        if ((BYTE)RallyDataCountryIndex() == 7)
            sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0xee));
        else
            sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0xbb));
        CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
        resY = g_pGraphics->resY;
        Font_DrawText(0, CFrontend::m_stringDest, x, (resY * 10) / 480 + ((resY * 20) / 480) * i + y + Font_GetLineHeight(0),
                      (int *)g_menuFrameColour, 0x11);
    }
    Font_SetBlendMode(2);
}

// GLOBAL: CMR2 0x00519f74
char g_networkRallyEndMenuName[] = "NetworkRallyRallyEndMenu_Display";

// Placeholder draw callback of the network rally end menu.
// FUNCTION: CMR2 0x00450ed0
void FUN_00450ed0(Menu *pMenu)
{
    Font_DrawText(0, g_networkRallyEndMenuName, 100, 100, (int *)g_menuFrameColour, 9);
}

int FUN_0040cfe0(int index);
int RallyTiming_GetStageTimeSeconds(int iDriver);
int RallyTiming_GetStageOrderDriverID(int iPosition);

// Stage points of the sixteen stage positions, for the tie checks below.
// GLOBAL: CMR2 0x005418c4
BYTE g_unk0x005418c4[0x10];

// Draw callback of the scrolling stage points table: driver, car, points and
// position; tied drivers get the tie-break note (count-back) and "=" instead
// of their position.
// FUNCTION: CMR2 0x00450ef0
void FUN_00450ef0(Menu *pMenu)
{
    bool isPlayer;
    int x;
    int rows;
    int row;
    int pos;
    int id;
    int points;
    int player;
    int i;

    x = (int)(g_pGraphics->resX * 30) / 640;
    rows = (g_unk0x005413f8 | g_unk0x0053f5a8) ? 5 : 6;
    FUN_0044b760();
    GameMenus_DrawTextRow(x, (int)(g_pGraphics->resY * 0x43) / 480,
                          CFrontend::GetTextString(RallyDataCountryIndex() & 0xff), CFrontend::GetTextString(0x42),
                          CFrontend::GetTextString(0x8d), 0);
    i = 0;
    do {
        g_unk0x005418c4[i] = (BYTE)RallyTiming_GetStageTimeSeconds(RallyTiming_GetStageOrderDriverID(i));
        i++;
    } while (i < 0x10);
    for (row = 0; row < rows; row++) {
        if (g_unk0x0053f5a8 != 0) {
            row++;
            pos = (pMenu->items[0].max - 1) + row;
        } else {
            if (g_unk0x005413f8 != 0)
                row++;
            pos = pMenu->items[0].max + row;
        }
        id = RallyTiming_GetStageOrderDriverID(pos);
        points = RallyTiming_GetStageTimeSeconds(id);
        isPlayer = FALSE;
        for (player = 0; player < CGameInfo::FUN_00405d70(); player++) {
            if (StageTiming_GetDriverSlot(player) == id) {
                isPlayer = TRUE;
                strcpy(CFrontend::m_stringDest, (char *)RallyData_GetRecord(player));
                goto found;
            }
        }
        strcpy(CFrontend::m_stringDest, CAIHelper::GetNameForID(id));
    found:
        Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x5c) / 640,
                      (((int)(g_pGraphics->resY * 0x82) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * row) -
                       (int)(g_pGraphics->resY * 8) / 480) - g_unk0x00540c60,
                      (int *)g_menuFrameColour, 9);
        if (isPlayer) {
            strcpy(CFrontend::m_stringDest, (char *)CFrontend::FUN_0040ede0(RallyData_FUN_004086b0(player)));
            CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
            g_menuRect[1] = (short)((int)(g_pGraphics->resY * 0x82) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * row -
                                    g_unk0x00540c60 - 1);
            g_menuRect[3] = (short)((int)(g_pGraphics->resY * 0xa3) / 480 - (int)(g_pGraphics->resY * 0x82) / 480 + 1);
            g_menuRect[0] = (short)((int)(g_pGraphics->resX * 0x20) / 640);
            g_menuRect[2] = (short)((int)(g_pGraphics->resX * 0x41) / 640 - (int)(g_pGraphics->resX * 0x20) / 640);
            Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, g_menuRowFillColour, 2);
        } else {
            strcpy(CFrontend::m_stringDest, (char *)CFrontend::FUN_0040ede0(FUN_00407fc0(id)));
            CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
            GameMenus_DrawRowFrame(row, 0, 0);
        }
        Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x5c) / 640,
                      ((int)(g_pGraphics->resY * 0xa3) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * row) -
                          g_unk0x00540c60,
                      (int *)g_menuFrameColour, 0x21);
        sprintf(CFrontend::m_stringDest, g_stageNumberFormat, points);
        Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x21e) / 640,
                      (((int)(g_pGraphics->resY * 0x82) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * row) -
                       (int)(g_pGraphics->resY * 8) / 480) - g_unk0x00540c60,
                      (int *)g_menuFrameColour, 9);
        if (g_unk0x005418c4[pos] != 0) {
            for (i = 0; i < 0x10; i++) {
                if (i != pos && g_unk0x005418c4[pos] == g_unk0x005418c4[i]) {
                    if (FUN_0040cfe0(id) == 1)
                        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0xb9), FUN_0040cfe0(id));
                    else
                        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0xb7), FUN_0040cfe0(id));
                    Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x1fe) / 640,
                                  (((int)(g_pGraphics->resY * 0x82) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * row) -
                                   (int)(g_pGraphics->resY * 8) / 480) - g_unk0x00540c60,
                                  (int *)g_menuFrameColour, 0xc);
                    if (pos > 0 && FUN_0040cfe0(RallyTiming_GetStageOrderDriverID(pos - 1)) == FUN_0040cfe0(id) &&
                        g_unk0x005418c4[pos - 1] == g_unk0x005418c4[pos]) {
                        sprintf(CFrontend::m_stringDest, g_stageResultSameTime);
                        goto draw;
                    }
                    break;
                }
            }
        }
        sprintf(CFrontend::m_stringDest, g_stageNumberFormat, pos + 1);
    draw:
        Font_DrawText(1, CFrontend::m_stringDest,
                      ((int)(g_pGraphics->resX * 0x41) / 640 - (int)(g_pGraphics->resX * 0x20) / 640) / 2 +
                          (int)(g_pGraphics->resX * 0x20) / 640,
                      (((int)(g_pGraphics->resY * 8) / 480 +
                        ((int)(g_pGraphics->resY * 0xa3) / 480 - (int)(g_pGraphics->resY * 0x82) / 480) / 2 +
                        ((int)(g_pGraphics->resY * 0x34) / 480) * row) -
                       g_unk0x00540c60) + (int)(g_pGraphics->resY * 0x82) / 480,
                      (int *)g_menuFrameColour, 0x12);
        if (g_unk0x0053f5a8 != 0 || g_unk0x005413f8 != 0)
            row--;
    }
    Font_SetBlendMode(2);
}

// Draw callback of the final championship standings (header menu) screen.
// match 86%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00451690
void FUN_00451690(Menu *pMenu)
{
    char position[100];
    char *pPosition;
    int i;
    int place;
    int resY;

    if (g_pHeaderMenu == &g_menu0x0053f790) {
        FUN_0044b760();
        GameMenus_DrawTextRow((int)(g_pGraphics->resX * 30) / 640, (int)(g_pGraphics->resY * 242) / 480,
                              CFrontend::GetTextString(0x42), CFrontend::GetTextString(CGameInfo::FUN_00405d90() + 0x8e));
        for (i = 0; i < CGameInfo::FUN_00405d70(); i++) {
            place = RallyTiming_GetStagePositionOfDriver(StageTiming_GetDriverSlot(i));
            switch (place) {
            case 0:
                pPosition = CFrontend::GetTextString(0x51);
                break;
            case 1:
                pPosition = CFrontend::GetTextString(0x52);
                break;
            case 2:
                pPosition = CFrontend::GetTextString(0x53);
                break;
            default:
                sprintf(position, CFrontend::GetTextString(0x54), place + 1);
                pPosition = position;
                break;
            }
            sprintf(CFrontend::m_stringDest, g_standingsRowFormat, (char *)RallyData_GetRecord(i), pPosition);
            CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
            resY = g_pGraphics->resY;
            Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 30) / 640,
                          (resY * 242) / 480 + ((resY * 10) / 480 + Font_GetLineHeight(0)) * (i + 1),
                          (int *)g_menuFrameColour, 0x11);
        }
    } else {
        if (!FUN_00407270())
            FUN_0044b760();
    }
}

extern char g_pointsFormat[];
extern char g_plusTimeFormat[];
extern BYTE g_menuRowFillColour[4];
extern char g_stageNumberFormat[];
extern char g_stageResultSameTime[];
int FUN_0040ce40(int position);
int FUN_004483c0(int index);
void FUN_00451890(Menu *pMenu);
int FUN_00448c60(int index);
BYTE FUN_00407fc0(int param1);
int FUN_004483c0(int index);
void FUN_00451890(Menu *pMenu);
void FormatCentisecondsAsMinSecMSec(int iTime, char *pcFormattedTime);
void GameMenus_DrawRowFrame(short row, short yOffset, char compact);
void FUN_00451890(Menu *pMenu);
unsigned int RallyData_FUN_004082b0(void);
unsigned int RallyData_FUN_004082d0(void);
unsigned int RallyData_FUN_004082e0(void);

// Draw callback of the stage classification table: one boxed row per car
// with its name, time (or target time, or points) and car, the players'
// rows highlighted, and the position number on the left ("=" for a tie).
// FUNCTION: CMR2 0x00451df0
void FUN_00451df0(Menu *pMenu)
{
    char diff[12];
    bool isPlayer;
    int i;
    int next;
    int car;
    int time;
    int flags;
    char *pFormat;

    FUN_0044b760();
    FUN_00451890(pMenu);
    for (i = 0; i < (BYTE)FUN_00420190(); i = next) {
        isPlayer = FALSE;
        car = FUN_00448390(i);
        time = FUN_004483c0(car);
        if (car < (int)(RallyDataState() & 0xff)) {
            isPlayer = TRUE;
            strcpy(CFrontend::m_stringDest, (char *)RallyData_GetRecord(car));
        } else {
            strcpy(CFrontend::m_stringDest, CAIHelper::GetNameForID(car));
        }
        Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x5c) / 640,
                      ((int)(g_pGraphics->resY * 0x82) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * i) -
                          (int)(g_pGraphics->resY * 8) / 480,
                      (int *)g_menuFrameColour, 9);
        flags = 9;
        if (g_pHeaderMenu == &g_menu0x0053ec48) {
            sprintf(CFrontend::m_stringDest, g_stageNumberFormat, FUN_0040ce40(i));
        } else if (!(BYTE)RallyData_FUN_004082e0() ||
                   (RallyData_FUN_004082b0() != 1 && RallyData_FUN_004082b0() != 2)) {
            FormatCentisecondsAsMinSecMSec(time, CFrontend::m_stringDest);
        } else if (RallyData_FUN_004082b0() == 1) {
            flags = 0xc;
            if (i == 0) {
                FormatCentisecondsAsMinSecMSec(time - RallyData_FUN_004082d0() * 100, CFrontend::m_stringDest);
            } else {
                FormatCentisecondsAsMinSecMSec(RallyData_FUN_004082d0() * 100, diff);
                sprintf(CFrontend::m_stringDest, g_plusTimeFormat, diff);
            }
        } else {
            sprintf(CFrontend::m_stringDest, g_pointsFormat, FUN_00448c60(car));
        }
        Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x21e) / 640,
                      ((int)(g_pGraphics->resY * 0x82) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * i) -
                          (int)(g_pGraphics->resY * 8) / 480,
                      (int *)g_menuFrameColour, flags);
        if (isPlayer) {
            strcpy(CFrontend::m_stringDest, (char *)CFrontend::FUN_0040ede0(RallyData_FUN_004086b0(car)));
            CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
            g_menuRect[1] = (short)((int)(g_pGraphics->resY * 0x82) / 480 +
                                    ((int)(g_pGraphics->resY * 0x34) / 480) * i - 1);
            g_menuRect[3] = (short)((int)(g_pGraphics->resY * 0xa3) / 480 - (int)(g_pGraphics->resY * 0x82) / 480 + 1);
            g_menuRect[0] = (short)((int)(g_pGraphics->resX * 0x20) / 640);
            g_menuRect[2] = (short)((int)(g_pGraphics->resX * 0x41) / 640 - (int)(g_pGraphics->resX * 0x20) / 640);
            Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, g_menuRowFillColour, 2);
        } else {
            strcpy(CFrontend::m_stringDest, (char *)CFrontend::FUN_0040ede0(FUN_00407fc0(car)));
            CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
            GameMenus_DrawRowFrame(i, 0, 0);
        }
        Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x5c) / 640,
                      (int)(g_pGraphics->resY * 0xa3) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * i,
                      (int *)g_menuFrameColour, 0x21);
        if (RallyData_FUN_004082b0() == 1 || RallyData_FUN_004082b0() == 2 ||
            FUN_004483c0(FUN_00448390(i)) != FUN_004483c0(FUN_00448390(i - 1)))
            pFormat = g_stageNumberFormat;
        else
            pFormat = g_stageResultSameTime;
        next = i + 1;
        sprintf(CFrontend::m_stringDest, pFormat, next);
        Font_DrawText(1, CFrontend::m_stringDest,
                      ((int)(g_pGraphics->resX * 0x41) / 640 - (int)(g_pGraphics->resX * 0x20) / 640) / 2 +
                          (int)(g_pGraphics->resX * 0x20) / 640,
                      (int)(g_pGraphics->resY * 8) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * i +
                          (int)(g_pGraphics->resY * 0x82) / 480 +
                          ((int)(g_pGraphics->resY * 0xa3) / 480 - (int)(g_pGraphics->resY * 0x82) / 480) / 2,
                      (int *)g_menuFrameColour, 0x12);
    }
    Font_SetBlendMode(2);
}

int FUN_00472990(KnockoutMatch *pMatch);
extern char g_keypadFormat[];

// Match of the arcade knockout table shown by the result screen.
// GLOBAL: CMR2 0x00541cd0
KnockoutMatch *g_pKnockoutMatch;

// Draw callback of the arcade knockout match result: who won, and whether
// the player goes through to the next round (or wins the final).
// FUNCTION: CMR2 0x004529c0
void FUN_004529c0(Menu *pMenu)
{
    char name[4];
    unsigned int *pState;
    int x;
    int y;

    x = (int)(g_pGraphics->resX * 30) / 640;
    y = (int)(g_pGraphics->resY * 242) / 480;
    pState = RallyData_GetChampionshipState();
    FUN_0044b760();
    if (FUN_00472990(g_pKnockoutMatch)) {
        Font_DrawText(2, CFrontend::GetTextString(0x49), x, y, (int *)g_menuFrameColour, 0x11);
        if (g_pKnockoutMatch->time1 <= g_pKnockoutMatch->time2)
            sprintf(name, (char *)RallyData_GetRecord(FUN_0041b370() + (g_pKnockoutMatch->flags & 0x1f)));
        else
            sprintf(name, (char *)RallyData_GetRecord(FUN_0041b370() + ((g_pKnockoutMatch->flags >> 5) & 0x1f)));
        if ((*pState & 0x38) >= 0x20)
            sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x7d), name);
        else
            sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x7e), name);
        CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
        Font_DrawText(0, CFrontend::m_stringDest, x, (int)(g_pGraphics->resY * 5) / 480 + (y + Font_GetLineHeight(0)),
                      (int *)g_menuFrameColour, 0x11);
    } else {
        sprintf(name, (char *)RallyData_GetRecord(FUN_0041b370() + (g_pKnockoutMatch->flags & 0x1f)));
        sprintf(CFrontend::m_stringDest, g_keypadFormat, CFrontend::GetTextString(0x88), name);
        Font_DrawText(2, CFrontend::m_stringDest, x, y, (int *)g_menuFrameColour, 0x11);
        if ((*pState & 0x38) < 0x20 && (*pState & 7) != 1)
            sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x91));
        else
            sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x92));
        CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
        Font_DrawText(0, CFrontend::m_stringDest, x, (int)(g_pGraphics->resY * 5) / 480 + (y + Font_GetLineHeight(0)),
                      (int *)g_menuFrameColour, 0x11);
    }
}

int FUN_0041bf50(int index);
int FUN_0041bf60(int index);
int FUN_0041bf70(int index);

// Draw callback of the stage penalties screen: for every car, its
// disqualification or retirement reason, jump start / speeding penalties
// and car damage notes, one line each.
// FUNCTION: CMR2 0x00452be0
void FUN_00452be0(Menu *pMenu)
{
    int x;
    int y;
    int lines;
    int i;

    x = (int)(g_pGraphics->resX * 30) / 640;
    lines = 0;
    y = (int)(g_pGraphics->resY * 242) / 480;
    FUN_0044b760();
    Font_DrawText(2, CFrontend::GetTextString(0x49), x, y, (int *)g_menuFrameColour, 0x11);
    for (i = 0; i < (int)(RallyDataState() & 0xff); i++) {
        if (FUN_0041bf50(i) & 0x80) {
            sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x7f), CFrontend::FUN_0040ede0(FUN_0041bf60(i)));
            CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
            Font_DrawText(0, CFrontend::m_stringDest, x,
                          (int)(g_pGraphics->resY * 5) / 480 + Font_GetLineHeight(0) + y +
                              ((int)(g_pGraphics->resY * 10) / 480 + Font_GetLineHeight(0)) * i,
                          (int *)g_menuFrameColour, 0x11);
            lines++;
        } else if (FUN_0041bf50(i) & 0x2000) {
            sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0xad),
                    CFrontend::GetTextString(FUN_0041bf70(i) + 0xad));
            CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
            Font_DrawText(0, CFrontend::m_stringDest, x,
                          (int)(g_pGraphics->resY * 5) / 480 + Font_GetLineHeight(0) + y +
                              ((int)(g_pGraphics->resY * 10) / 480 + Font_GetLineHeight(0)) * i,
                          (int *)g_menuFrameColour, 0x11);
            lines++;
        }
        if (FUN_0041bf50(i) & 0x1800) {
            sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0xbf),
                    CFrontend::GetTextString((FUN_0041bf50(i) & 0x800) ? 5 : 7));
            CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
            Font_DrawText(0, CFrontend::m_stringDest, x,
                          (int)(g_pGraphics->resY * 5) / 480 + Font_GetLineHeight(0) + y +
                              ((int)(g_pGraphics->resY * 10) / 480 + Font_GetLineHeight(0)) * lines +
                              ((int)(g_pGraphics->resY * 10) / 480 + Font_GetLineHeight(0)) * i,
                          (int *)g_menuFrameColour, 0x11);
            lines++;
        }
        if (FUN_0041bf50(i) & 0x300) {
            sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0xc1));
            CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
            Font_DrawText(0, CFrontend::m_stringDest, x,
                          (int)(g_pGraphics->resY * 5) / 480 + Font_GetLineHeight(0) + y +
                              ((int)(g_pGraphics->resY * 10) / 480 + Font_GetLineHeight(0)) * lines +
                              ((int)(g_pGraphics->resY * 10) / 480 + Font_GetLineHeight(0)) * i,
                          (int *)g_menuFrameColour, 0x11);
            lines++;
        }
        if (FUN_0041bf50(i) & 0x400) {
            sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0xc0));
            CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
            Font_DrawText(0, CFrontend::m_stringDest, x,
                          (int)(g_pGraphics->resY * 5) / 480 + Font_GetLineHeight(0) + y +
                              ((int)(g_pGraphics->resY * 10) / 480 + Font_GetLineHeight(0)) * lines +
                              ((int)(g_pGraphics->resY * 10) / 480 + Font_GetLineHeight(0)) * i,
                          (int *)g_menuFrameColour, 0x11);
            lines++;
        }
    }
}

int FUN_0040ce30(int index);
int FUN_0040cea0(int index);
int FUN_0040ced0(int index);

// Draw callback of the rally points table: every car in rally order with its
// car, points, tie-break note (count-back) and position ("=" for a tie).
// FUNCTION: CMR2 0x004530e0
void FUN_004530e0(Menu *pMenu)
{
    bool isPlayer;
    bool tied;
    int i;
    int j;
    int id;

    FUN_0044b760();
    GameMenus_DrawTextRow((int)(g_pGraphics->resX * 30) / 640, (int)(g_pGraphics->resY * 0x43) / 480,
                          CInput::FormatString(CFrontend::GetTextString(0x43), 0x62 - (RallyData_FUN_00406940() & 0xff)),
                          CFrontend::GetTextString(0x42), CFrontend::GetTextString(0x8d), 0);
    for (i = 0; i < (BYTE)FUN_00420190(); i++)
        g_unk0x005418c4[i] = (BYTE)FUN_0040ced0(FUN_0040cea0(i));
    for (i = 0; i < (BYTE)FUN_00420190(); i++) {
        isPlayer = FALSE;
        tied = FALSE;
        id = FUN_0040cea0(i);
        if (id < (int)(RallyDataState() & 0xff)) {
            isPlayer = TRUE;
            strcpy(CFrontend::m_stringDest, (char *)RallyData_GetRecord(id));
        } else {
            strcpy(CFrontend::m_stringDest, CAIHelper::GetNameForID(id));
        }
        Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x5c) / 640,
                      ((int)(g_pGraphics->resY * 0x82) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * i) -
                          (int)(g_pGraphics->resY * 8) / 480,
                      (int *)g_menuFrameColour, 9);
        sprintf(CFrontend::m_stringDest, g_stageNumberFormat, FUN_0040ced0(id));
        Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x21e) / 640,
                      ((int)(g_pGraphics->resY * 0x82) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * i) -
                          (int)(g_pGraphics->resY * 8) / 480,
                      (int *)g_menuFrameColour, 9);
        if (g_unk0x005418c4[i] != 0) {
            for (j = 0; j < (BYTE)FUN_00420190(); j++) {
                if (j != i && g_unk0x005418c4[i] == g_unk0x005418c4[j]) {
                    if (FUN_0040ce30(id) == 1)
                        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0xb8), FUN_0040ce30(id));
                    else
                        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0xb6), FUN_0040ce30(id));
                    Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x1fe) / 640,
                                  ((int)(g_pGraphics->resY * 0x82) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * i) -
                                      (int)(g_pGraphics->resY * 8) / 480,
                                  (int *)g_menuFrameColour, 0xc);
                    if (i > 0 && FUN_0040ce30(FUN_0040cea0(i - 1)) == FUN_0040ce30(id) &&
                        g_unk0x005418c4[i - 1] == g_unk0x005418c4[i])
                        tied = TRUE;
                    break;
                }
            }
        }
        if (isPlayer) {
            strcpy(CFrontend::m_stringDest, (char *)CFrontend::FUN_0040ede0(RallyData_FUN_004086b0(id)));
            CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
            g_menuRect[1] = (short)((int)(g_pGraphics->resY * 0x82) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * i - 1);
            g_menuRect[3] = (short)((int)(g_pGraphics->resY * 0xa3) / 480 - (int)(g_pGraphics->resY * 0x82) / 480 + 1);
            g_menuRect[0] = (short)((int)(g_pGraphics->resX * 0x20) / 640);
            g_menuRect[2] = (short)((int)(g_pGraphics->resX * 0x41) / 640 - (int)(g_pGraphics->resX * 0x20) / 640);
            Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, g_menuRowFillColour, 2);
        } else {
            strcpy(CFrontend::m_stringDest, (char *)CFrontend::FUN_0040ede0(FUN_00407fc0(id)));
            CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
            GameMenus_DrawRowFrame(i, 0, 0);
        }
        Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x5c) / 640,
                      (int)(g_pGraphics->resY * 0xa3) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * i,
                      (int *)g_menuFrameColour, 0x21);
        if (tied)
            sprintf(CFrontend::m_stringDest, g_stageResultSameTime);
        else
            sprintf(CFrontend::m_stringDest, g_stageNumberFormat, i + 1);
        Font_DrawText(1, CFrontend::m_stringDest,
                      ((int)(g_pGraphics->resX * 0x41) / 640 - (int)(g_pGraphics->resX * 0x20) / 640) / 2 +
                          (int)(g_pGraphics->resX * 0x20) / 640,
                      (int)(g_pGraphics->resY * 8) / 480 + ((int)(g_pGraphics->resY * 0x34) / 480) * i +
                          (int)(g_pGraphics->resY * 0x82) / 480 +
                          ((int)(g_pGraphics->resY * 0xa3) / 480 - (int)(g_pGraphics->resY * 0x82) / 480) / 2,
                      (int *)g_menuFrameColour, 0x12);
    }
    Font_SetBlendMode(2);
}

int FUN_0040ceb0(int index);
unsigned int RallyData_FUN_00406940(void);

// Draw callback of the rally results screen: "stage N of the rally" with the
// rally name between two separator bars (before the first driver in the top
// three), then one line per driver with its rally position.
// FUNCTION: CMR2 0x00453830
void FUN_00453830(Menu *pMenu)
{
    bool shown;
    int x;
    int place;
    int i;
    int resY;
    int lineHeight;
    int *pResY;

    shown = FALSE;
    x = (int)(g_pGraphics->resX * 30) / 640;
    FUN_0044b760();
    sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x43), 0x62 - (RallyData_FUN_00406940() & 0xff));
    Font_DrawText(2, CFrontend::m_stringDest, x, (int)(g_pGraphics->resY * 242) / 480, (int *)g_menuFrameColour, 0x11);
    x += Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest);
    for (i = 0; i < (BYTE)RallyDataState(); i++) {
        place = FUN_0040ceb0(i);
        if (!shown && place < 3) {
            x += (int)(g_pGraphics->resX * 8) / 640;
            pResY = &g_pGraphics->resY;
            g_menuRect[0] = (short)x;
            resY = *pResY;
            lineHeight = Font_GetLineHeight(2);
            g_menuRect[2] = 2;
            g_menuRect[1] = (short)(resY * 242 / 480 + resY * 4 / 480 - lineHeight);
            g_menuRect[3] = (short)((int)(*pResY * 41) / 480);
            Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, g_menuFrameColour, 2);
            x += (int)(g_pGraphics->resX * 8) / 640;
            strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(CGameInfo::FUN_00405d90() + 0x8e));
            Font_DrawText(2, CFrontend::m_stringDest, x, (int)(g_pGraphics->resY * 242) / 480,
                          (int *)g_menuFrameColour, 0x11);
            x += Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest);
            x += (int)(g_pGraphics->resX * 8) / 640;
            g_menuRect[0] = (short)x;
            Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, g_menuFrameColour, 2);
            x += (int)(g_pGraphics->resX * 8) / 640;
            Font_DrawText(2, CFrontend::GetTextString(0x49), x, (int)(g_pGraphics->resY * 242) / 480,
                          (int *)g_menuFrameColour, 0x11);
            shown = TRUE;
        }
        sprintf(CFrontend::m_stringDest, g_nameSpaceFormat, (char *)RallyData_GetRecord(i));
        switch (place) {
        case 0:
            sprintf(CFrontend::m_stringDest + strlen(CFrontend::m_stringDest), CFrontend::GetTextString(0x51));
            break;
        case 1:
            sprintf(CFrontend::m_stringDest + strlen(CFrontend::m_stringDest), CFrontend::GetTextString(0x52));
            break;
        case 2:
            sprintf(CFrontend::m_stringDest + strlen(CFrontend::m_stringDest), CFrontend::GetTextString(0x53));
            break;
        default:
            sprintf(CFrontend::m_stringDest + strlen(CFrontend::m_stringDest), CFrontend::GetTextString(0x54), place + 1);
            break;
        }
        CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
        Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 30) / 640,
                      (int)(g_pGraphics->resY * 242) / 480 +
                          ((int)(g_pGraphics->resY * 10) / 480 + Font_GetLineHeight(0)) * (i + 1),
                      (int *)g_menuFrameColour, 0x11);
    }
    Font_SetBlendMode(2);
}

// Builds the in-game menu tree (pause / results / options screens).
// FUNCTION: CMR2 0x00449100
void GameMenus_Build(void)
{
    g_menuBuildTime = timeGetTime();
    Menu_Init(&g_menu0x0053ea68, 0, 0, 0, NULL, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x0053ea68, 0, 0, &g_menu0x00541218, NULL, 0);
    Menu_SetCallbacks(&g_menu0x0053ea68, NULL, NULL, (MenuCallback)FUN_0044b7b0, NULL);
    Menu_ValidateCursor(&g_menu0x0053ea68, 0);
    Menu_Init(&g_menu0x0053ff38, 0, 0, 0, NULL, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x0053ff38, 0, 0, &g_menu0x0053ea68, NULL, 0);
    Menu_SetCallbacks(&g_menu0x0053ff38, NULL, NULL, (MenuCallback)FUN_0044bc30, NULL);
    Menu_ValidateCursor(&g_menu0x0053ff38, 0);
    Menu_Init(&g_menu0x00541218, 0, 0, 0, &g_menu0x0053ea68, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x00541218, 0, 0, &g_menu0x0053f5b0, NULL, 0);
    Menu_SetCallbacks(&g_menu0x00541218, NULL, NULL, (MenuCallback)FUN_0044bcd0, NULL);
    Menu_ValidateCursor(&g_menu0x00541218, 0);
    Menu_Init(&g_menu0x0053f3c8, 0, 0, 0, &g_menu0x0053ea68, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x0053f3c8, 0, 0, &g_menu0x00541400, NULL, 0);
    Menu_SetCallbacks(&g_menu0x0053f3c8, NULL, NULL, (MenuCallback)FUN_0044d260, NULL);
    Menu_ValidateCursor(&g_menu0x0053f3c8, 0);
    Menu_Init(&g_menu0x0053f5b0, 0, 0, 0, &g_menu0x00541218, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x0053f5b0, 0, 0, &g_menu0x0053e4b8, NULL, 0);
    Menu_SetCallbacks(&g_menu0x0053f5b0, (MenuCallback)FUN_00449ba0, NULL, (MenuCallback)FUN_0044d790, NULL);
    Menu_ValidateCursor(&g_menu0x0053f5b0, 0);
    Menu_Init(&g_menu0x0053e4b8, 0, 0, 0, NULL, NULL, 1, 0, 0);
    Menu_AddItemType3(&g_menu0x0053e4b8, 0, 0, 0xb, 0, 0, 0, (int)FUN_00449e90, 0);
    Menu_SetCallbacks(&g_menu0x0053e4b8, (MenuCallback)FUN_00449ba0, NULL, (MenuCallback)FUN_0044d960, NULL);
    Menu_ValidateCursor(&g_menu0x0053e4b8, 0);
    Menu_Init(&g_menu0x00540c68, 0, 0, 0, NULL, NULL, 1, 0, 0);
    Menu_AddItemType3(&g_menu0x00540c68, 0, 0, 8, 0, 0, 0, (int)FUN_00449e90, 0);
    Menu_SetCallbacks(&g_menu0x00540c68, (MenuCallback)FUN_00449ca0, NULL, (MenuCallback)FUN_0044e130, NULL);
    Menu_ValidateCursor(&g_menu0x00540c68, 0);
    Menu_Init(&g_menu0x005406b8, 0, 0, 0, NULL, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x005406b8, 0, 0, NULL, NULL, 0);
    Menu_SetCallbacks(&g_menu0x005406b8, NULL, NULL, (MenuCallback)FUN_0044e830, NULL);
    Menu_ValidateCursor(&g_menu0x005406b8, 0);
    Menu_Init(&g_menu0x0053f008, 0, 0, 0, &g_menu0x0053e4b8, NULL, 1, 0, 0);
    Menu_AddItemType3(&g_menu0x0053f008, 0, 0, 0xb, 0, 0, 0, (int)FUN_00449e90, 0);
    Menu_SetCallbacks(&g_menu0x0053f008, (MenuCallback)FUN_00449ba0, NULL, (MenuCallback)FUN_0044fea0, NULL);
    Menu_ValidateCursor(&g_menu0x0053f008, 0);
    Menu_Init(&g_menu0x0053fd58, 0, 0, 0, &g_menu0x00540c68, NULL, 1, 0, 0);
    Menu_AddItemType3(&g_menu0x0053fd58, 0, 0, 0xb, 0, 0, 0, (int)FUN_00449e90, 0);
    Menu_SetCallbacks(&g_menu0x0053fd58, (MenuCallback)FUN_00449ca0, NULL, (MenuCallback)FUN_004505b0, NULL);
    Menu_ValidateCursor(&g_menu0x0053fd58, 0);
    Menu_Init(&g_menu0x0053e6a0, 0, 0, 0, &g_menu0x0053f008, NULL, 1, 0, 0);
    Menu_AddItemType3(&g_menu0x0053e6a0, 0, 0, 0xb, 0, 0, 0, (int)FUN_00449e90, 0);
    Menu_SetCallbacks(&g_menu0x0053e6a0, (MenuCallback)FUN_00449ba0, NULL, (MenuCallback)FUN_0044efa0, NULL);
    Menu_ValidateCursor(&g_menu0x0053e6a0, 0);
    Menu_Init(&g_menu0x00540118, 0, 0, 0, &g_menu0x0053e6a0, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x00540118, 0, 0, NULL, NULL, 0);
    Menu_SetCallbacks(&g_menu0x00540118, NULL, NULL, (MenuCallback)FUN_00450c10, NULL);
    Menu_ValidateCursor(&g_menu0x00540118, 0);
    Menu_Init(&g_menu0x00540a80, 0, 0, 0, &g_menu0x00541030, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x00540a80, 0, 0, NULL, NULL, 0);
    Menu_SetCallbacks(&g_menu0x00540a80, NULL, NULL, (MenuCallback)FUN_00450ed0, NULL);
    Menu_ValidateCursor(&g_menu0x00540a80, 0);
    Menu_Init(&g_menu0x00541030, 0, 0, 0, &g_menu0x0053fd58, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x00541030, 0, 0, NULL, NULL, 0);
    Menu_SetCallbacks(&g_menu0x00541030, NULL, NULL, (MenuCallback)FUN_0044f8a0, NULL);
    Menu_ValidateCursor(&g_menu0x00541030, 0);
    Menu_Init(&g_menu0x00541ae0, 0, 0, 0, &g_menu0x00540118, NULL, 1, 0, 0);
    Menu_AddItemType3(&g_menu0x00541ae0, 0, 0, 0xb, 0, 0, 0, (int)FUN_00449e90, 0);
    Menu_SetCallbacks(&g_menu0x00541ae0, (MenuCallback)FUN_00449ba0, NULL, (MenuCallback)FUN_00450ef0, NULL);
    Menu_ValidateCursor(&g_menu0x00541ae0, 0);
    Menu_Init(&g_menu0x0053f790, 0, 0, 0, &g_menu0x00541ae0, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x0053f790, 0, 0, &g_menu0x0053f970, NULL, 0);
    Menu_SetCallbacks(&g_menu0x0053f790, NULL, NULL, (MenuCallback)FUN_00451690, NULL);
    Menu_ValidateCursor(&g_menu0x0053f790, 0);
    Menu_Init(&g_menu0x0053f970, 0, 0, 0, &g_menu0x0053f790, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x0053f970, 0, 0, NULL, (int)FUN_0044a000, 0);
    Menu_SetCallbacks(&g_menu0x0053f970, (MenuCallback)FUN_00449b80, NULL, (MenuCallback)FUN_00451690, NULL);
    Menu_ValidateCursor(&g_menu0x0053f970, 0);
    Menu_Init(&g_menu0x0053fb70, 0, 0, 0, NULL, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x0053fb70, 0, 0, NULL, NULL, 0);
    Menu_SetCallbacks(&g_menu0x0053fb70, NULL, NULL, (MenuCallback)FUN_00451df0, NULL);
    Menu_ValidateCursor(&g_menu0x0053fb70, 0);
    Menu_Init(&g_menu0x0053e2d8, 0, 0, 0, NULL, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x0053e2d8, 0, 0, NULL, NULL, 0);
    Menu_SetCallbacks(&g_menu0x0053e2d8, NULL, NULL, (MenuCallback)FUN_00452430, NULL);
    Menu_ValidateCursor(&g_menu0x0053e2d8, 0);
    Menu_Init(&g_menu0x0053ec48, 0, 0, 0, &g_menu0x0053fb70, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x0053ec48, 0, 0, NULL, NULL, 0);
    Menu_SetCallbacks(&g_menu0x0053ec48, NULL, NULL, (MenuCallback)FUN_00451df0, NULL);
    Menu_ValidateCursor(&g_menu0x0053ec48, 0);
    Menu_Init(&g_menu0x0053e888, 0, 0, 0, &g_menu0x0053ec48, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x0053e888, 0, 0, NULL, NULL, 0);
    Menu_SetCallbacks(&g_menu0x0053e888, NULL, NULL, (MenuCallback)FUN_004530e0, NULL);
    Menu_ValidateCursor(&g_menu0x0053e888, 0);
    Menu_Init(&g_menu0x0053ee28, 0, 0, 0, &g_menu0x0053e888, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x0053ee28, 0, 0, &g_menu0x00541400, NULL, 0);
    Menu_SetCallbacks(&g_menu0x0053ee28, NULL, NULL, (MenuCallback)FUN_00453830, NULL);
    Menu_ValidateCursor(&g_menu0x0053ee28, 0);
    if (CGameInfo::FUN_00405d80() == 4) {
    Menu_Init(&g_menu0x0053f1e8, 0, 0, 0, &g_menu0x00541218, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x0053f1e8, 0, 0, &g_menu0x00541400, NULL, 0);
    Menu_SetCallbacks(&g_menu0x0053f1e8, NULL, NULL, (MenuCallback)FUN_004529c0, NULL);
    Menu_ValidateCursor(&g_menu0x0053f1e8, 0);
    }
    Menu_Init(&g_menu0x00540e50, 0, 0, 0, NULL, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x00540e50, 0, 0, &g_menu0x00541400, NULL, 0);
    Menu_SetCallbacks(&g_menu0x00540e50, NULL, NULL, (MenuCallback)FUN_00452be0, NULL);
    Menu_ValidateCursor(&g_menu0x00540e50, 0);
    if (CGameInfo::FUN_00405e00() != 0)
        Menu_Init(&g_menu0x005402f8, 0, 0, 0, &g_menu0x005416e0, NULL, 1, 0, 1);
    else
        Menu_Init(&g_menu0x005402f8, 0, 0, 0, &g_menu0x00541400, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x005402f8, 0, -1, NULL, (int)FUN_00449f30, 1000);
    if (CGameInfo::FUN_00405d80() == 2 || CGameInfo::FUN_00405d80() == 3)
        Menu_AddItemType2(&g_menu0x005402f8, 0, 0x8b, NULL, (int)FUN_0044a0a0, 0x3e9);
    Menu_AddItemType1(&g_menu0x005402f8, 0, 0xa6, 0, -1);
    Menu_SetCallbacks(&g_menu0x005402f8, (MenuCallback)FUN_00449b90, NULL, (MenuCallback)FUN_00453c50, NULL);
    Menu_ValidateCursor(&g_menu0x005402f8, 0);
    if (CGameInfo::FUN_00405e00() != 0)
        Menu_Init(&g_menu0x005418d8, 0, 0, 0, &g_menu0x005416e0, NULL, 1, 0, 1);
    else
        Menu_Init(&g_menu0x005418d8, 0, 0, 0, &g_menu0x00541400, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x005418d8, 0, 0xa8, NULL, (int)FUN_00449b00, 1000);
    Menu_AddItemType1(&g_menu0x005418d8, 0, 0xaa, 0, -1);
    Menu_SetCallbacks(&g_menu0x005418d8, (MenuCallback)FUN_00449b90, NULL, (MenuCallback)FUN_00453c50, NULL);
    Menu_ValidateCursor(&g_menu0x005418d8, 0);
    Menu_Init(&g_menu0x005408a0, 0, 0, 0, &g_menu0x005416e0, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x005408a0, 0, 0xf6, NULL, (int)FUN_00449b60, 1000);
    Menu_AddItemType1(&g_menu0x005408a0, 0, 0xaa, 0, -1);
    Menu_SetCallbacks(&g_menu0x005408a0, (MenuCallback)FUN_00449b90, NULL, (MenuCallback)FUN_00453c50, NULL);
    Menu_ValidateCursor(&g_menu0x005408a0, 0);
    Menu_Init(&g_menu0x005404d8, 0, 0, 0, NULL, NULL, 1, 0, 0);
    Menu_SetCallbacks(&g_menu0x005404d8, (MenuCallback)FUN_0044a090, NULL, (MenuCallback)FUN_00453c50, NULL);
    Menu_ValidateCursor(&g_menu0x005404d8, 0);
}


int FUN_0040ae90(void);
NetClassification *FUN_0040aea0(int index);
unsigned int FUN_0040aeb0(int index);
unsigned int FUN_0040aec0(int index);
void GameMenus_DrawTextRow(int x, int y, char *pText, ...);
void GameMenus_DrawRowFrame(short row, short yOffset, char compact);
void FUN_00451890(Menu *pMenu);
char *FUN_0040abb0(int index, int total);
int FUN_0040ac10(int index);
int FUN_0040ab50(int index, int total);
unsigned int FUN_0040abe0(int index, int total);
int FUN_0040ab20(int index, int total);
unsigned int FUN_0040ab80(int index, int total);
unsigned int RallyData_FUN_00406940(void);
unsigned int RallyData_FUN_00406950(void);
void FUN_00451890(Menu *pMenu);
void GameMenus_FormatModeName(int unused, int mode);
int FUN_004055e0(void);
int FUN_004055f0(void);
BYTE FUN_0041b370(void);

// GLOBAL: CMR2 0x00541cc0
short g_menuRect[4];
// GLOBAL: CMR2 0x00519ecc
BYTE g_menuFrameColour[4] = { 250, 250, 250, 255 };
// GLOBAL: CMR2 0x00540c60
int g_unk0x00540c60;
// GLOBAL: CMR2 0x0053e698
int g_unk0x0053e698;

// GLOBAL: CMR2 0x00516098
char g_classRowHeaderFormat[] = "%s %d";
// GLOBAL: CMR2 0x00516e30
char g_noTimeText[] = "--:--.--";

// Championship classification screen: header row (country plus either the
// special-stage name or "round N") and one row per class with the class name,
// its tag (from the three text ids) and the best time of the class.
// FUNCTION: CMR2 0x0044e830
void FUN_0044e830(Menu *pMenu)
{
    char *texts[4];
    int i;
    char *pClass;

    texts[0] = CMain::m_logFileBlankLine;
    texts[1] = CFrontend::GetTextString(0xff);
    texts[2] = CFrontend::GetTextString(0x100);
    texts[3] = CFrontend::GetTextString(0x101);
    FUN_0044b760();
    if (RallyDataStageIndex() == 0xa) {
        GameMenus_DrawTextRow((int)(g_pGraphics->resX * 0x1e) / 0x280, (int)(g_pGraphics->resY * 0x43) / 0x1e0,
                              CFrontend::GetTextString((unsigned char)RallyDataCountryIndex()),
                              CFrontend::GetTextString(0xba), NULL);
    } else {
        GameMenus_DrawTextRow((int)(g_pGraphics->resX * 0x1e) / 0x280, (int)(g_pGraphics->resY * 0x43) / 0x1e0,
                              CFrontend::GetTextString((unsigned char)RallyDataCountryIndex()),
                              CInput::FormatString(g_classRowHeaderFormat, CFrontend::GetTextString(0x40),
                                                   RallyDataStageIndex() + 1), NULL);
    }
    i = 0;
    if (FUN_0040ae90() > 0) {
        do {
            Font_DrawText(1, FUN_0040aea0(i)->name, (int)(g_pGraphics->resX * 0x5c) / 0x280, (int)(g_pGraphics->resY * 3) / 0x1e0 +
                          ((((int)(g_pGraphics->resY * 0x82) / 0x1e0 + ((int)(g_pGraphics->resY * 0x2d) / 0x1e0) * i) -
                            (int)(g_pGraphics->resY * 0x14) / 0x1e0) - (int)(g_pGraphics->resY * 8) / 0x1e0),
                          (int *)g_menuFrameColour, 9);
            Font_DrawText(1, texts[FUN_0040aeb0(i)], (int)(g_pGraphics->resX * 0x5c) / 0x280 + (int)(g_pGraphics->resX * 100) / 0x280, (int)(g_pGraphics->resY * 3) / 0x1e0 +
                          ((((int)(g_pGraphics->resY * 0x82) / 0x1e0 + ((int)(g_pGraphics->resY * 0x2d) / 0x1e0) * i) -
                            (int)(g_pGraphics->resY * 0x14) / 0x1e0) - (int)(g_pGraphics->resY * 8) / 0x1e0),
                          (int *)g_menuFrameColour, 9);
            if ((int)FUN_0040aec0(i) == -1) {
                sprintf(CFrontend::m_stringDest, g_noTimeText);
            } else {
                FormatCentisecondsAsMinSecMSec(FUN_0040aec0(i), CFrontend::m_stringDest);
            }
            pClass = CFrontend::m_stringDest;
            Font_DrawText(1, pClass, (int)(g_pGraphics->resX * 0x5c) / 0x280 + (int)(g_pGraphics->resX * 0xfa) / 0x280, (int)(g_pGraphics->resY * 3) / 0x1e0 +
                          ((((int)(g_pGraphics->resY * 0x82) / 0x1e0 + ((int)(g_pGraphics->resY * 0x2d) / 0x1e0) * i) -
                            (int)(g_pGraphics->resY * 0x14) / 0x1e0) - (int)(g_pGraphics->resY * 8) / 0x1e0),
                          (int *)g_menuFrameColour, 9);
            i++;
        } while (i < FUN_0040ae90());
    }
    Font_SetBlendMode(2);
}

// GLOBAL: CMR2 0x00517dd8
char g_standingsRowFormat[] = "%s - %s";
// GLOBAL: CMR2 0x00519ed0
BYTE g_menuTextColour[4] = { 0xa7, 0xac, 0xdb, 0xff };

// Draws one row of the standings list: the item text (with the driver record
// appended when the item is the "go to" one) plus the small tag that follows
// it, and the row background sprite. The highlighted row uses the frame
// colour and the first background texture, the rest the text colour and the
// second one.
#define GAMEMENUS_DRAW_STANDINGS_ROW(pColour, pTexture)                                          \
    if (pItem->value == 0x3ea) {                                                                 \
        sprintf(CFrontend::m_stringDest, g_standingsRowFormat,                                   \
                CFrontend::GetTextString(pItem->id),                                             \
                (char *)RallyData_GetRecord((BYTE)(FUN_0041b370() + 1)));                        \
        Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x86) / 0x280,       \
                      (int)(g_pGraphics->resY * 0xde) / 0x1e0 +                                  \
                          ((int)(g_pGraphics->resY * 0x36) / 0x1e0) * i,                         \
                      (int *)pColour, 0x11);                                                     \
    } else {                                                                                     \
        Font_DrawText(1, CFrontend::GetTextString(pItem->id),                                    \
                      (int)(g_pGraphics->resX * 0x86) / 0x280,                                   \
                      (int)(g_pGraphics->resY * 0xde) / 0x1e0 +                                  \
                          ((int)(g_pGraphics->resY * 0x36) / 0x1e0) * i,                         \
                      (int *)pColour, 0x11);                                                     \
    }                                                                                            \
    Font_DrawText(0, CFrontend::GetTextString(pItem->id + 1),                                    \
                  (int)(g_pGraphics->resX * 0x86) / 0x280,                                       \
                  (int)(g_pGraphics->resY * 0xf) / 0x1e0 +                                       \
                      (int)(g_pGraphics->resY * 0xde) / 0x1e0 +                                  \
                      ((int)(g_pGraphics->resY * 0x36) / 0x1e0) * i,                             \
                  (int *)pColour, 0x11);                                                         \
    Sprite_Queue((SpriteRect *)(pTexture() + 0x11c), (SpriteRect *)rect, (Texture *)pTexture(), 2, 0, NULL,    \
                 NULL, pColour, 8);

// GLOBAL: CMR2 0x00517dd0
char g_stageNumberFormat[] = "%d";
// GLOBAL: CMR2 0x00517dd4
char g_nameSpaceFormat[] = "%s ";
// GLOBAL: CMR2 0x00541cdc
int g_stageResultTimes[4];
// GLOBAL: CMR2 0x00541cec
BYTE g_stageResultRecords[4];

// Stage results screen: header (country plus either the special-stage name or
// "round N") and one row per record - driver, class tag, time and position -
// with a frame around the row. The stage type (1..3) shifts the rows down.
// FUNCTION: CMR2 0x0044d260
void FUN_0044d260(Menu *pMenu)
{
    int offset;
    int i;
    int *pTime;

    FUN_0044b760();
    if (RallyDataStageIndex() == 0xa) {
        GameMenus_DrawTextRow((int)(g_pGraphics->resX * 0x1e) / 0x280,
                              (int)(g_pGraphics->resY * 0x43) / 0x1e0,
                              CFrontend::GetTextString((unsigned char)RallyDataCountryIndex()),
                              CFrontend::GetTextString(0xba), CFrontend::GetTextString(0x9a), NULL);
    } else {
        GameMenus_DrawTextRow((int)(g_pGraphics->resX * 0x1e) / 0x280,
                              (int)(g_pGraphics->resY * 0x43) / 0x1e0,
                              CFrontend::GetTextString((unsigned char)RallyDataCountryIndex()),
                              CInput::FormatString(g_classRowHeaderFormat, CFrontend::GetTextString(0x40),
                                                   RallyDataStageIndex() + 1),
                              CFrontend::GetTextString(0x9a), NULL);
    }
    switch (FUN_0041b370()) {
    case 1:
        offset = (int)(g_pGraphics->resY * 0x50) / 0x1e0;
        break;
    case 2:
        offset = (int)(g_pGraphics->resY * 0x32) / 0x1e0;
        break;
    case 3:
        offset = (int)(g_pGraphics->resY * 0x14) / 0x1e0;
        break;
    }
    i = 0;
    if (FUN_0041b370() + 1 > 0) {
        pTime = g_stageResultTimes;
        do {
            sprintf(CFrontend::m_stringDest, (char *)RallyData_GetRecord(g_stageResultRecords[i]));
            CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
            Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x5c) / 0x280,
                          ((int)(g_pGraphics->resY * 0x82) / 0x1e0 +
                           ((int)(g_pGraphics->resY * 0x34) / 0x1e0) * i) -
                              (int)(g_pGraphics->resY * 8) / 0x1e0 + offset,
                          (int *)g_menuFrameColour, 9);
            sprintf(CFrontend::m_stringDest,
                    (char *)CFrontend::FUN_0040ede0(RallyData_FUN_004086b0(g_stageResultRecords[i])));
            CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
            Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x5c) / 0x280,
                          (int)(g_pGraphics->resY * 0xa3) / 0x1e0 + offset +
                              ((int)(g_pGraphics->resY * 0x34) / 0x1e0) * i,
                          (int *)g_menuFrameColour, 0x11);
            sprintf(CFrontend::m_stringDest, g_minSecMSECFormatString, *pTime / 6000,
                    (*pTime / 100) % 0x3c, *pTime % 100);
            Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x21e) / 0x280,
                          (int)(g_pGraphics->resY * 6) / 0x1e0 + offset +
                              (((int)(g_pGraphics->resY * 0x82) / 0x1e0 +
                                ((int)(g_pGraphics->resY * 0x34) / 0x1e0) * i) -
                               (int)(g_pGraphics->resY * 8) / 0x1e0),
                          (int *)g_menuFrameColour, 9);
            GameMenus_DrawRowFrame(i, offset, 0);
            sprintf(CFrontend::m_stringDest, g_stageNumberFormat, i + 1);
            Font_DrawText(1, CFrontend::m_stringDest,
                          ((int)(g_pGraphics->resX * 0x41) / 0x280 -
                           (int)(g_pGraphics->resX * 0x20) / 0x280) / 2 +
                              (int)(g_pGraphics->resX * 0x20) / 0x280,
                          (int)(g_pGraphics->resY * 8) / 0x1e0 +
                              ((int)(g_pGraphics->resY * 0x34) / 0x1e0) * i +
                              (int)(g_pGraphics->resY * 0x82) / 0x1e0 +
                              ((int)(g_pGraphics->resY * 0xa3) / 0x1e0 -
                               (int)(g_pGraphics->resY * 0x82) / 0x1e0) / 2 + offset,
                          (int *)g_menuFrameColour, 0x12);
            i++;
            pTime++;
        } while (i < FUN_0041b370() + 1);
    }
}

// Standings list of one menu: title, then one row per item.
// FUNCTION: CMR2 0x00453c50
void FUN_00453c50(Menu *pMenu)
{
    MenuItem *pItem;
    short rect[4];
    int x;
    int i;

    x = (int)(g_pGraphics->resX * 0x1e) / 0x280;
    FUN_0044b760();
    Font_DrawText(2, CFrontend::GetTextString(0x56), x, (int)(g_pGraphics->resY * 0x43) / 0x1e0,
                  (int *)g_menuFrameColour, 0x11);
    rect[0] = (short)((int)(g_pGraphics->resX * 0x70) / 0x280);
    rect[2] = ((SpriteRect *)(FUN_004055e0() + 0x11c))->w;
    rect[3] = ((SpriteRect *)(FUN_004055e0() + 0x11c))->h;
    i = 0;
    pItem = pMenu->items;
    if (pMenu->itemCount > 0) {
        do {
            rect[1] = (short)(((int)(g_pGraphics->resY * 0xde) / 0x1e0 +
                               ((int)(g_pGraphics->resY * 0x36) / 0x1e0) * i) -
                              (int)(g_pGraphics->resY * 0xd) / 0x1e0);
            if (pMenu->cursor == i) {
                GAMEMENUS_DRAW_STANDINGS_ROW(g_menuFrameColour, FUN_004055e0)
            } else {
                GAMEMENUS_DRAW_STANDINGS_ROW(g_menuTextColour, FUN_004055f0)
            }
            i++;
            pItem++;
        } while (i < pMenu->itemCount);
    }
    Font_SetBlendMode(2);
}

// GLOBAL: CMR2 0x00519ed8
BYTE g_menuRowFillColour[4] = { 0xf6, 0x72, 0x28, 0xff };
// GLOBAL: CMR2 0x00519f70
char g_stageResultSameTime[] = "=";

// Stage start list: header (country plus the two stage texts) and one row per
// driver with its name, its number, the status (with the row boxed when the
// driver is not classified) and the time, which is only printed when it
// differs from the previous driver's.
// FUNCTION: CMR2 0x0044f8a0
void FUN_0044f8a0(Menu *pMenu)
{
    int x;
    int i;

    x = (int)(g_pGraphics->resX * 0x1e) / 0x280;
    FUN_0044b760();
    GameMenus_DrawTextRow(x, (int)(g_pGraphics->resY * 0x43) / 0x1e0,
                          CFrontend::GetTextString((unsigned char)RallyDataCountryIndex()),
                          CFrontend::GetTextString(0x41), CFrontend::GetTextString(0x8d), NULL);
    i = 0;
    if (FUN_0040ab10() > 0) {
        do {
            Font_DrawText(1, FUN_0040abb0(i, 1), (int)(g_pGraphics->resX * 0x5c) / 0x280,
                          (int)(g_pGraphics->resY * 3) / 0x1e0 +
                              (((int)(g_pGraphics->resY * 0x82) / 0x1e0 +
                                ((int)(g_pGraphics->resY * 0x2d) / 0x1e0) * i) -
                               (int)(g_pGraphics->resY * 0x14) / 0x1e0) -
                              (int)(g_pGraphics->resY * 8) / 0x1e0,
                          (int *)g_menuFrameColour, 9);
            sprintf(CFrontend::m_stringDest, g_stageNumberFormat, FUN_0040ac10(i));
            Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x21e) / 0x280,
                          (((int)(g_pGraphics->resY * 0x82) / 0x1e0 +
                            ((int)(g_pGraphics->resY * 0x2d) / 0x1e0) * i) -
                           (int)(g_pGraphics->resY * 0x14) / 0x1e0) -
                              (int)(g_pGraphics->resY * 8) / 0x1e0,
                          (int *)g_menuFrameColour, 9);
            if (FUN_0040ab50(i, 1) == -2) {
                strcpy(CFrontend::m_stringDest,
                       (char *)CFrontend::FUN_0040ede0(FUN_0040abe0(i, 1)));
                CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
                g_menuRect[1] = (short)((((int)(g_pGraphics->resY * 0x82) / 0x1e0 +
                                          ((int)(g_pGraphics->resY * 0x2d) / 0x1e0) * i) -
                                         (int)(g_pGraphics->resY * 0x14) / 0x1e0) -
                                        1);
                g_menuRect[3] = (short)(((int)(g_pGraphics->resY * 0xa3) / 0x1e0 -
                                          (int)(g_pGraphics->resY * 0x82) / 0x1e0) +
                                        1);
                g_menuRect[0] = (short)((int)(g_pGraphics->resX * 0x20) / 0x280);
                g_menuRect[2] = (short)((int)(g_pGraphics->resX * 0x41) / 0x280 -
                                        (int)(g_pGraphics->resX * 0x20) / 0x280);
                Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, g_menuRowFillColour, 2);
            } else {
                strcpy(CFrontend::m_stringDest,
                       (char *)CFrontend::FUN_0040ede0(FUN_0040abe0(i, 1)));
                CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
                GameMenus_DrawRowFrame(i, (-(int)g_pGraphics->resY * 0x14) / 0x1e0, 1);
            }
            Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x5c) / 0x280,
                          ((int)(g_pGraphics->resY * 0xa3) / 0x1e0 +
                           ((int)(g_pGraphics->resY * 0x2d) / 0x1e0) * i) -
                              (int)(g_pGraphics->resY * 0x14) / 0x1e0,
                          (int *)g_menuFrameColour, 0x21);
            if (i == 0 || (i > 0 && FUN_0040ab20(i, 1) != FUN_0040ab20(i - 1, 1))) {
                sprintf(CFrontend::m_stringDest, g_stageNumberFormat, FUN_0040ab20(i, 1));
            } else {
                sprintf(CFrontend::m_stringDest, g_stageResultSameTime);
            }
            Font_DrawText(1, CFrontend::m_stringDest,
                          ((int)(g_pGraphics->resX * 0x41) / 0x280 -
                           (int)(g_pGraphics->resX * 0x20) / 0x280) / 2 +
                              (int)(g_pGraphics->resX * 0x20) / 0x280,
                          (int)(g_pGraphics->resY * 8) / 0x1e0 +
                              (int)(g_pGraphics->resY * 0x82) / 0x1e0 +
                              ((((int)(g_pGraphics->resY * 0xa3) / 0x1e0 -
                                 (int)(g_pGraphics->resY * 0x82) / 0x1e0) / 2 +
                                ((int)(g_pGraphics->resY * 0x2d) / 0x1e0) * i) -
                               (int)(g_pGraphics->resY * 0x14) / 0x1e0),
                          (int *)g_menuFrameColour, 0x12);
            i++;
        } while (i < FUN_0040ab10());
    }
    Font_SetBlendMode(2);
}

// GLOBAL: CMR2 0x00541cc8
Menu *g_pHeaderMenu;

// Draws the text and, right after it, the two-pixel marker rectangle that
// separates the breadcrumb entries, then moves x past both.
#define GAMEMENUS_HEADER_MARKER(pText)                                                        \
    x = x + (int)(g_pGraphics->resX * 8) / 0x280 + Font_GetTextWidth(2, (BYTE *)pText);         \
    g_menuRect[0] = (short)x;                                                                  \
    g_menuRect[2] = 2;                                                                         \
    g_menuRect[1] = (short)((int)(g_pGraphics->resY * 0x25) / 0x1e0);                          \
    g_menuRect[3] = (short)((int)(g_pGraphics->resY * 0x29) / 0x1e0);                          \
    Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, g_menuFrameColour, 2);                \
    x = x + (int)(g_pGraphics->resX * 8) / 0x280 + 2;

// Breadcrumb of the in-game screens: the menu title (when it is the one that
// owns it), the stage or championship name and, for the first two stages, the
// race mode; each entry is followed by its marker rectangle.
// FUNCTION: CMR2 0x00451890
void FUN_00451890(Menu *pMenu)
{
    int flag;
    char *pText;
    int x;

    x = (int)(g_pGraphics->resX * 0x1e) / 0x280;
    if (pMenu == &g_menu0x00541218) {
        Font_DrawText(2, CFrontend::GetTextString(0xac), x, (int)(g_pGraphics->resY * 0x43) / 0x1e0,
                      (int *)g_menuFrameColour, 0x11);
        GAMEMENUS_HEADER_MARKER(CFrontend::GetTextString(0xac))
    }
    if ((BYTE)RallyData_FUN_00406940() < 2) {
        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x43),
                0x62 - (BYTE)RallyData_FUN_00406940());
        flag = 1;
    } else {
        flag = 0;
        if ((BYTE)RallyData_FUN_00406950() == 0) {
            pText = CFrontend::GetTextString(5);
        } else {
            pText = CFrontend::GetTextString(7);
        }
        sprintf(CFrontend::m_stringDest, pText);
        CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
    }
    Font_DrawText(2, CFrontend::m_stringDest, x, (int)(g_pGraphics->resY * 0x43) / 0x1e0,
                  (int *)g_menuFrameColour, 0x11);
    if (flag) {
        GAMEMENUS_HEADER_MARKER(CFrontend::m_stringDest)
        GameMenus_FormatModeName((int)CFrontend::m_stringDest,
                                 (BYTE)RallyData_FUN_00406940() * 3 + (BYTE)RallyData_FUN_00406950());
        CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
        Font_DrawText(2, CFrontend::m_stringDest, x, (int)(g_pGraphics->resY * 0x43) / 0x1e0,
                      (int *)g_menuFrameColour, 0x11);
    }
    if (g_pHeaderMenu == &g_menu0x0053ec48) {
        GAMEMENUS_HEADER_MARKER(CFrontend::m_stringDest)
        strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(0x8d));
        Font_DrawText(2, CFrontend::m_stringDest,
                      x + 2 + (int)(g_pGraphics->resX * 8) / 0x280,
                      (int)(g_pGraphics->resY * 0x43) / 0x1e0, (int *)g_menuFrameColour, 0x11);
    }
}

// Standings table of the championship: one row per driver with its name, its
// total time, the status (boxed when the driver is not classified) and the
// time of the row, which is only printed when it differs from the previous
// driver's (a "=" otherwise). The header comes from the shared breadcrumb
// helper, which is still pending, so that one call is the only difference.
// FUNCTION: CMR2 0x00452430
void FUN_00452430(Menu *pMenu)
{
    int i;

    FUN_0044b760();
    FUN_00451890(pMenu);
    i = 0;
    if (FUN_0040ab10() > 0) {
        do {
            Font_DrawText(1, FUN_0040abb0(i, 0), (int)(g_pGraphics->resX * 0x5c) / 0x280,
                          (int)(g_pGraphics->resY * 3) / 0x1e0 +
                              (((int)(g_pGraphics->resY * 0x82) / 0x1e0 +
                                ((int)(g_pGraphics->resY * 0x2d) / 0x1e0) * i) -
                               (int)(g_pGraphics->resY * 0x14) / 0x1e0) -
                              (int)(g_pGraphics->resY * 8) / 0x1e0,
                          (int *)g_menuFrameColour, 9);
            FormatCentisecondsAsMinSecMSec(FUN_0040ab80(i, 0), CFrontend::m_stringDest);
            Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x21e) / 0x280,
                          (((int)(g_pGraphics->resY * 0x82) / 0x1e0 +
                            ((int)(g_pGraphics->resY * 0x2d) / 0x1e0) * i) -
                           (int)(g_pGraphics->resY * 0x14) / 0x1e0) -
                              (int)(g_pGraphics->resY * 8) / 0x1e0,
                          (int *)g_menuFrameColour, 9);
            if (FUN_0040ab50(i, 0) == -2) {
                strcpy(CFrontend::m_stringDest,
                       (char *)CFrontend::FUN_0040ede0(FUN_0040abe0(i, 0)));
                CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
                g_menuRect[1] = (short)((((int)(g_pGraphics->resY * 0x82) / 0x1e0 +
                                          ((int)(g_pGraphics->resY * 0x2d) / 0x1e0) * i) -
                                         (int)(g_pGraphics->resY * 0x14) / 0x1e0) -
                                        1);
                g_menuRect[3] = (short)(((int)(g_pGraphics->resY * 0xa3) / 0x1e0 -
                                          (int)(g_pGraphics->resY * 0x82) / 0x1e0) +
                                        1);
                g_menuRect[0] = (short)((int)(g_pGraphics->resX * 0x20) / 0x280);
                g_menuRect[2] = (short)((int)(g_pGraphics->resX * 0x41) / 0x280 -
                                        (int)(g_pGraphics->resX * 0x20) / 0x280);
                Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, g_menuRowFillColour, 2);
            } else {
                strcpy(CFrontend::m_stringDest,
                       (char *)CFrontend::FUN_0040ede0(FUN_0040abe0(i, 0)));
                CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
                GameMenus_DrawRowFrame(i, (-(int)g_pGraphics->resY * 0x14) / 0x1e0, 1);
            }
            Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x5c) / 0x280,
                          ((int)(g_pGraphics->resY * 0xa3) / 0x1e0 +
                           ((int)(g_pGraphics->resY * 0x2d) / 0x1e0) * i) -
                              (int)(g_pGraphics->resY * 0x14) / 0x1e0,
                          (int *)g_menuFrameColour, 0x21);
            if (i == 0 || (i > 0 && FUN_0040ab20(i, 0) != FUN_0040ab20(i - 1, 0))) {
                sprintf(CFrontend::m_stringDest, g_stageNumberFormat, FUN_0040ab20(i, 0));
            } else {
                sprintf(CFrontend::m_stringDest, g_stageResultSameTime);
            }
            Font_DrawText(1, CFrontend::m_stringDest,
                          ((int)(g_pGraphics->resX * 0x41) / 0x280 -
                           (int)(g_pGraphics->resX * 0x20) / 0x280) / 2 +
                              (int)(g_pGraphics->resX * 0x20) / 0x280,
                          (int)(g_pGraphics->resY * 8) / 0x1e0 +
                              (int)(g_pGraphics->resY * 0x82) / 0x1e0 +
                              ((((int)(g_pGraphics->resY * 0xa3) / 0x1e0 -
                                 (int)(g_pGraphics->resY * 0x82) / 0x1e0) / 2 +
                                ((int)(g_pGraphics->resY * 0x2d) / 0x1e0) * i) -
                               (int)(g_pGraphics->resY * 0x14) / 0x1e0),
                          (int *)g_menuFrameColour, 0x12);
            i++;
        } while (i < FUN_0040ab10());
    }
    Font_SetBlendMode(2);
}

// Draws the four edges of the box around one menu row.
// FUNCTION: CMR2 0x0044ec30
void GameMenus_DrawRowFrame(short row, short yOffset, char compact)
{
    int height;

    if (compact != 0) {
        height = g_pGraphics->resY * 0x2d;
    } else {
        height = g_pGraphics->resY * 0x34;
    }
    row = row * (short)(height / 0x1e0);

    g_menuRect[0] = (short)((int)(g_pGraphics->resX * 0x20) / 0x280) + 2;
    g_menuRect[1] = (short)((int)(g_pGraphics->resY * 0x82) / 0x1e0) - (short)g_unk0x00540c60 +
                    row + (short)g_unk0x0053e698 + yOffset;
    g_menuRect[2] = (short)((int)(g_pGraphics->resX * 0x41) / 0x280) -
                    (short)((int)(g_pGraphics->resX * 0x20) / 0x280) - 4;
    g_menuRect[3] = 2;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, g_menuFrameColour, 2);

    g_menuRect[0] = (short)((int)(g_pGraphics->resX * 0x20) / 0x280) + 2;
    g_menuRect[1] = (short)((int)(g_pGraphics->resY * 0xa3) / 0x1e0) - (short)g_unk0x00540c60 +
                    row + (short)g_unk0x0053e698 - 2 + yOffset;
    g_menuRect[3] = 2;
    g_menuRect[2] = (short)((int)(g_pGraphics->resX * 0x41) / 0x280) -
                    (short)((int)(g_pGraphics->resX * 0x20) / 0x280) - 4;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, g_menuFrameColour, 2);

    g_menuRect[0] = (short)((int)(g_pGraphics->resX * 0x41) / 0x280) - 2;
    g_menuRect[2] = 2;
    g_menuRect[1] = (short)((int)(g_pGraphics->resY * 0x82) / 0x1e0) - (short)g_unk0x00540c60 +
                    row + (short)g_unk0x0053e698 + yOffset;
    g_menuRect[3] = (short)((int)(g_pGraphics->resY * 0xa3) / 0x1e0) -
                    (short)((int)(g_pGraphics->resY * 0x82) / 0x1e0);
    Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, g_menuFrameColour, 2);

    g_menuRect[0] = (short)((int)(g_pGraphics->resX * 0x20) / 0x280);
    g_menuRect[2] = 2;
    g_menuRect[1] = (short)((int)(g_pGraphics->resY * 0x82) / 0x1e0) - (short)g_unk0x00540c60 +
                    row + (short)g_unk0x0053e698 + yOffset;
    g_menuRect[3] = (short)((int)(g_pGraphics->resY * 0xa3) / 0x1e0) -
                    (short)((int)(g_pGraphics->resY * 0x82) / 0x1e0);
    Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, g_menuFrameColour, 2);
}

// Draws a row of strings at (x, y), separated by a thin vertical bar.
// match 82%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00454df0
void GameMenus_DrawTextRow(int x, int y, char *pText, ...)
{
    int width;

    g_menuRect[2] = 2;
    g_menuRect[1] = y - (int)(g_pGraphics->resY * 0x1e) / 0x1e0;
    g_menuRect[3] = (int)(g_pGraphics->resY * 0x29) / 0x1e0;
    char *pCur = pText;
    char **ppNext = &pText;
    while (pCur != NULL) {
        CGenericFileLoader::StrLowerPolish(pCur);
        Font_DrawText(2, pCur, x, y, (int *)g_menuFrameColour, 0x11);
        sprintf(CFrontend::m_stringDest, pCur);
        pCur = ppNext[1];
        ppNext++;
        if (pCur == NULL) {
            break;
        }
        width = (int)(g_pGraphics->resX * 8) / 0x280 + x + Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest);
        g_menuRect[0] = (short)width;
        Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, g_menuFrameColour, 2);
        x = width + (int)(g_pGraphics->resX * 8) / 0x280 + 2;
    }
}

// GLOBAL: CMR2 0x00519ed4
DWORD g_menuHighlightColour = 0xbfae8072;

void *FUN_00408470(unsigned int param1);
BYTE FUN_004bc0c0(BYTE *p);

void FUN_004b7c80(void);
BOOL FUN_004a15a0(void);

// GLOBAL: CMR2 0x00519ee4
int g_unk0x00519ee4 = -1;
// GLOBAL: CMR2 0x00540898
BYTE g_unk0x00540898;
// GLOBAL: CMR2 0x00541210
BYTE g_unk0x00541210;

// FUNCTION: CMR2 0x00449cd0
void FUN_00449cd0(Menu *pMenu, int param)
{
    FUN_004b7c80();
}

int FUN_004a1af0(void);
void FUN_004d0700(char *text);
bool FUN_004b7cd0(int *pOut);

// GLOBAL: CMR2 0x00519ee8
char g_chatChars0x00519ee8[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ., !\"%&*()-=_+`/?\\:;[]{}'~";
// Set while the chat line (the item with value 0x3eb) is selected.
// GLOBAL: CMR2 0x00540e48
BYTE g_unk0x00540e48;
// GLOBAL: CMR2 0x00541cf8
char g_chatLine[0x100];
// GLOBAL: CMR2 0x00541df8
int g_chatLineLength;

// Network results menu: typing on the chat line, Enter sends it, Escape
// leaves the menu. Up/down jump between the chat line and the menu items.
// match 86%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00449ce0
void FUN_00449ce0(Menu *pMenu)
{
    DeviceInfo *pDevice;
    int key;
    int len;
    int maxLen;

    if (g_unk0x00540e48) {
        pDevice = CInput::FUN_0049ead0(0);
        if (pDevice->field_0x8 & 8)
            pMenu->cursor = 1;
        else if (pDevice->field_0x8 & 4)
            pMenu->cursor = pMenu->itemCount - 1;
    }
    if (g_unk0x00519ee4 != pMenu->cursor) {
        if (pMenu->items[pMenu->cursor].value == 0x3eb) {
            FUN_004b7c80();
            g_chatLine[0] = 0;
            g_chatLineLength = 0;
            g_unk0x00540e48 = 1;
            Menu_SetFlags(pMenu, 0, 0, 0, 0);
        } else {
            if (pMenu->items[g_unk0x00519ee4].value == 0x3eb)
                Menu_SetFlags(pMenu, 1, 1, 1, 1);
            g_unk0x00540e48 = 0;
        }
        g_unk0x00519ee4 = pMenu->cursor;
    }
    FUN_004a1af0();
    if (!g_unk0x00540e48 || !FUN_004b7cd0(&key))
        return;
    switch (key) {
    case 8:
        len = strlen(g_chatLine);
        if (len > 0) {
            g_chatLine[len - 1] = 0;
            g_chatLineLength--;
        }
        break;
    case 0xd:
        if (g_chatLine[0] != 0) {
            FUN_004d0700(g_chatLine);
            g_chatLineLength = 0;
            g_chatLine[0] = 0;
        }
        break;
    case 0x1b:
        Menu_SetNextAction((int)pMenu->pParent);
        break;
    default:
        if (strchr(g_chatChars0x00519ee8, (char)key) == NULL)
            break;
        len = Font_GetTextWidth(0, (BYTE *)g_chatLine);
        maxLen = (int)g_pGraphics->resX >= 0x400 ? 0x210 : 0x14a;
        if (g_chatLineLength < 0xff && len < maxLen) {
            g_chatLine[g_chatLineLength] = (char)key;
            g_chatLine[g_chatLineLength + 1] = 0;
            g_chatLineLength++;
        }
        break;
    }
}

BYTE *FUN_0040e8c0(void);
char *FUN_004d06f0(int index);
extern char g_str0x00519fb0[];
extern char g_str0x00519fb4[];

// Format of the chat line while its cursor blinks (every other 20 frames).
// GLOBAL: CMR2 0x00519fb8
char g_str0x00519fb8[4] = "%s_";

// Draw callback of the time trial / network results menu (0x005416e0): the
// separator bars of the results box, the five rows of the current stage, the
// chat line, the ten rows of the network classification and, at the bottom,
// the standings rows of the menu items.
// MSVC6 gives the frame 4 bytes less (sub esp,0x14 vs 0x18) so every local
// moves one slot down: the original keeps the 10-row pointer p in a stack slot
// (0x14) and leaves a dead 4-byte slot at 0x24, while here p stays in a
// register and its (unused) slot ends up last (0x20). Same logic, only stack
// slot assignment and the loop's register allocation differ.
// match 85%: stack frame size/slot order and the 10-row loop register allocation
// FUNCTION: CMR2 0x004541c0
void FUN_004541c0(Menu *pMenu)
{
    BYTE colour[4];
    char *p;
    int x;
    short rect[4];
    MenuItem *pItem;
    BYTE *pColour;
    BYTE *pEntries;
    int i;
    int y;

    rect[0] = (short)((int)(g_pGraphics->resX * 0x6c) / 0x280 - (int)(g_pGraphics->resX * 0x46) / 0x280);
    rect[1] = (short)((int)(g_pGraphics->resY * 0x55) / 0x1e0);
    rect[2] = (short)((int)(g_pGraphics->resX * 0x1a9) / 0x280);
    rect[3] = (short)((int)(g_pGraphics->resY * 0x6e) / 0x1e0);
    x = (int)(g_pGraphics->resX * 0x1e) / 0x280;
    colour[0] = 0xff;
    colour[1] = 0xff;
    colour[2] = 0xff;
    colour[3] = 0x20;
    FUN_0044b760();
    if (g_unk0x00540e48) {
        colour[3] = 0x20;
        pColour = g_menuFrameColour;
    } else {
        colour[3] = 0x10;
        pColour = g_menuTextColour;
    }
    Sprite_FillRect((int)g_pGraphics + 0x150, rect, colour, 1);
    rect[1] += (short)((int)(g_pGraphics->resY * 10) / 0x1e0) + rect[3];
    rect[3] = (short)((int)(g_pGraphics->resY * 0x1e) / 0x1e0);
    Sprite_FillRect((int)g_pGraphics + 0x150, rect, colour, 1);
    rect[0] = (short)((int)(g_pGraphics->resX * 0x1b3) / 0x280 +
                      (int)(g_pGraphics->resX * 0x6c) / 0x280 - (int)(g_pGraphics->resX * 0x46) / 0x280);
    rect[1] = (short)((int)(g_pGraphics->resY * 0x55) / 0x1e0);
    rect[2] = (short)((int)(g_pGraphics->resX * 0x82) / 0x280);
    rect[3] = (short)((int)(g_pGraphics->resY * 0x10e) / 0x1e0);
    Sprite_FillRect((int)g_pGraphics + 0x150, rect, colour, 1);
    for (i = 4, y = 0x5a; y < 0xbe; y += 0x14, i--) {
        Font_DrawText(0, FUN_004d06f0(i),
                      (int)(g_pGraphics->resX * 0x76) / 0x280 - (int)(g_pGraphics->resX * 0x46) / 0x280,
                      (int)(g_pGraphics->resY * y) / 0x1e0, (int *)pColour, 9);
    }
    if (g_unk0x00540e48) {
        if (CMain::GetFrameDelta() % 0x14 < 10)
            sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue, g_chatLine);
        else
            sprintf(CFrontend::m_stringDest, g_str0x00519fb8, g_chatLine);
        Font_DrawText(0, CFrontend::m_stringDest,
                      (int)(g_pGraphics->resX * 0x76) / 0x280 - (int)(g_pGraphics->resX * 0x46) / 0x280,
                      (int)(g_pGraphics->resY * 0xd2) / 0x1e0, (int *)g_menuFrameColour, 9);
    }
    if (CGameInfo::FUN_00405d80() != 10 && CGameInfo::FUN_00405d80() != 12) {
        pEntries = FUN_0040e8c0();
        Font_DrawText(0, CFrontend::GetTextString(0xfe), rect[0] + (int)rect[2] / 2,
                      (int)(g_pGraphics->resY * 10) / 0x1e0 + (int)(g_pGraphics->resY * 100) / 0x1e0,
                      (int *)g_menuFrameColour, 0x12);
        if (pEntries != NULL) {
            for (p = (char *)pEntries + 4, i = 0; i < 10; i++, p += 8) {
                if (*p == 0) {
                    Font_DrawText(0, g_str0x00519fb4, (int)(g_pGraphics->resX * 0x1fe) / 0x280,
                                  (int)(g_pGraphics->resY * 100) / 0x1e0 +
                                      ((int)(g_pGraphics->resY * 0x14) / 0x1e0) * (i + 3),
                                  (int *)g_menuFrameColour, 0x12);
                    Font_DrawText(0, g_str0x00519fb0, (int)(g_pGraphics->resX * 0x23a) / 0x280,
                                  (int)(g_pGraphics->resY * 100) / 0x1e0 +
                                      ((int)(g_pGraphics->resY * 0x14) / 0x1e0) * (i + 3),
                                  (int *)g_menuFrameColour, 0x12);
                } else {
                    Font_DrawText(0, p, (int)(g_pGraphics->resX * 0x1fe) / 0x280,
                                  (int)(g_pGraphics->resY * 100) / 0x1e0 +
                                      ((int)(g_pGraphics->resY * 0x14) / 0x1e0) * (i + 3),
                                  (int *)g_menuFrameColour, 0x12);
                    sprintf(CFrontend::m_stringDest, g_stageNumberFormat, *(int *)(p + 4));
                    Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x23a) / 0x280,
                                  (int)(g_pGraphics->resY * 100) / 0x1e0 +
                                      ((int)(g_pGraphics->resY * 0x14) / 0x1e0) * (i + 3),
                                  (int *)g_menuFrameColour, 0x12);
                }
            }
        } else {
            Font_DrawText(0, CFrontend::GetTextString(0xfb), rect[0] + (int)rect[2] / 2,
                          (int)(g_pGraphics->resY * 100) / 0x1e0 + (int)(g_pGraphics->resY * 0x3c) / 0x1e0,
                          (int *)g_menuFrameColour, 0x12);
        }
    }
    Font_DrawText(2, CFrontend::GetTextString(0x56), x, (int)(g_pGraphics->resY * 0x43) / 0x1e0,
                  (int *)g_menuFrameColour, 0x11);
    rect[0] = (short)((int)(g_pGraphics->resX * 0x70) / 0x280);
    rect[2] = ((SpriteRect *)(FUN_004055e0() + 0x11c))->w;
    rect[3] = ((SpriteRect *)(FUN_004055e0() + 0x11c))->h;
    i = 1;
    pItem = pMenu->items + 1;
    if (pMenu->itemCount > 1) {
        do {
            rect[1] = (short)(((int)(g_pGraphics->resY * 0xde) / 0x1e0 +
                               ((int)(g_pGraphics->resY * 0x36) / 0x1e0) * i) -
                              (int)(g_pGraphics->resY * 0xd) / 0x1e0);
            if (pMenu->cursor == i) {
                GAMEMENUS_DRAW_STANDINGS_ROW(g_menuFrameColour, FUN_004055e0)
            } else {
                GAMEMENUS_DRAW_STANDINGS_ROW(g_menuTextColour, FUN_004055f0)
            }
            i++;
            pItem++;
        } while (i < pMenu->itemCount);
    }
    Font_SetBlendMode(2);
}

// GLOBAL: CMR2 0x00541adc
int g_unk0x00541adc;
// GLOBAL: CMR2 0x00541cd8
int g_unk0x00541cd8;
extern char g_strDemoName0x00519ec4[4];

BYTE FUN_0041f380(void);
void FUN_00478190(int index, int seconds);
unsigned char RallyDataStageIndex(void);
int FUN_004d0220(int index);
void FUN_004a0ba0(void);
unsigned char CGameInfo::FUN_00405d70(void);
unsigned char CGameInfo::FUN_00405da0(void);
unsigned char CGameInfo::FUN_00405d80(void);
unsigned char CGameInfo::FUN_00405d90(void);
unsigned char CGameInfo::FUN_00405e00(void);
unsigned int CGameInfo::FUN_00406310(void);
unsigned int CGameInfo::FUN_00406430(void);
unsigned int CGameInfo::FUN_00406440(void);
unsigned int RallyData_FUN_00406940(void);
unsigned int RallyData_FUN_00407ea0(void);
unsigned int RallyData_FUN_004082b0(void);
unsigned int RallyData_FUN_004082e0(void);
unsigned int RallyData_GetFlag24(void);
BYTE FUN_004481b0(void);
BYTE FUN_004071c0(BYTE flags, char mode);
int FUN_0041bf50(int index);
int FUN_004728e0(void);
int FUN_0040ceb0(int index);
int FUN_0040ced0(int index);
int FUN_0040cef0(int index);
int RallyTiming_FUN_0040d3d0(int index);
int RallyTiming_GetOverallPositionOfDriver(int iDriver);
int RallyTiming_GetStageTimeSeconds(int iDriver);
int RallyTiming_GetStagePositionOfDriver(int iDriver);
void FUN_004660e0(BYTE value);
void FUN_004cf390(int index);
void FUN_004cf3b0(int index, int mode);
void FUN_004cf3f0(int index);
void FUN_004cf420(int index, int amount, int stat);
void FUN_004cf450(int index, int arg, int value);
void FUN_004cf470(int index, int value, unsigned int option, unsigned int field);
void FUN_004cf4d0(int index, unsigned int value, unsigned int field);
void FUN_004cf550(int index, unsigned int value, unsigned int field, int extra);
BYTE FUN_004cf5b0(int index, int pBlock);
BYTE FUN_004cf660(int index, int pBlock);
BYTE FUN_004cf740(int index, int pBlock);
BYTE FUN_004cf8e0(int index, int pBlock);
int FUN_004cf9d0(int param_1, int param_2);
BYTE FUN_004cfa10(int param_1, int param_2, char *pName);
char FUN_004cfb30(int param1, int index, char *pName);
char FUN_004cfc90(int param1, int index, char *pName);
char FUN_004cfff0(int param1, int index, char *pName);
unsigned char FUN_004d0580(void);
void FUN_0044af70(char param1, Menu *pParent);
BYTE *RallyData_FUN_00408cb0(int index);
extern int g_unk0x00541cf0;
void FUN_0044b270(void);
void FUN_0044b0d0(char param1, Menu *pParent);
void FUN_00472a30(void);
extern Menu *g_pSavedHeaderMenu;

// Sets up the results screen after a stage: it stores the stage name, fills
// the stage result table with the state of every car, and picks the menu the
// results screens hang from (rewiring the parent/submenu pointers of that
// menu) according to the game mode and the championship state.
// match 66%: all the calls/constants of the body are in place; what is left is
// the argument evaluation order of the FUN_0040ceb0/ced0/cef0, FUN_0040d3a0/
// d3d0/d070 and FUN_0041bf50 chains (MSVC6 schedules them in another order),
// the `and eax,0xff` after FUN_004cfa10 (our prototype returns BYTE, the
// original used a 32-bit return) and one hoisted store to g_unk0x00540898.
// FUNCTION: CMR2 0x0044a1b0
void FUN_0044a1b0(int param_1)
{
    char bVar1;
    char stageFlag;
    int state;
    int result;
    int i;
    int j;
    int car;
    int count;
    int driver;
    int minPos;
    int k;
    unsigned int *pState;
    Menu *pMenu;

    bVar1 = 0;
    if (FUN_0041f380() != 0xff)
        FUN_00478190((char)FUN_0041f380(), 0xb4);
    sprintf(g_ghostName, (char *)CGameInfo::FUN_00405fe0() + 0x654 +
                                 ((RallyDataCountryIndex() & 0xff) * 0xb +
                                  (RallyDataStageIndex() & 0xff)) * 8);
    CGenericFileLoader::StrLowerPolish(g_ghostName);
    for (i = 0; i < 10; i++)
        g_ghostSplits[i] = FUN_004d0220(i);
    g_unk0x00541adc = *(unsigned int *)((char *)CGameInfo::FUN_00405fe0() + 0x658 +
                                        ((RallyDataCountryIndex() & 0xff) * 0xb +
                                         (RallyDataStageIndex() & 0xff)) * 8) >> 7 & 0xffff;
    if ((char)param_1 == 0)
        CGameInfo::FUN_0049ea90(1);
    FUN_004a0ba0();
    g_unk0x00540898 = 0;
    g_unk0x00541210 = 0;
    if (CGameInfo::FUN_00405da0() != 0) {
        g_unk0x00540898 = 0;
        if (FUN_0041b370() == (BYTE)CGameInfo::FUN_00405d70() - 1)
            g_unk0x00540898 = 1;
    } else {
        g_unk0x00540898 = 1;
    }
    FUN_0044af70(0, NULL);
    GameMenus_Build();
    if (CGameInfo::FUN_00405d80() != 4 && (char)CGameInfo::FUN_00406430() == 0 &&
        (char)CGameInfo::FUN_00406440() == 0 && FUN_004481b0() == 0 &&
        strncmp((char *)RallyData_GetRecord(0), g_strDemoName0x00519ec4, 3) != 0) {
        for (i = 0; i < (BYTE)RallyDataState(); i++) {
            car = (BYTE)FUN_0041b370() + i;
            count = FUN_004483c0(i);
            if (count > 0) {
                if ((BYTE)RallyData_GetFlag24() == 0) {
                    driver = StageTiming_GetCurrentSplitPositionOfDriver(car);
                    FUN_004cf450(car, 0, count);
                    if (CGameInfo::FUN_00405d80() == 0 || CGameInfo::FUN_00405d80() == 1) {
                        if (RallyDataStageIndex() == 0)
                            FUN_004cf390(car);
                        FUN_004cf3b0(car, driver);
                    }
                    FUN_004cf5b0(car, (int)RallyData_FUN_00408cb0(car));
                    state = FUN_004cfa10(i, car, (char *)RallyData_GetRecord(car));
                    stageFlag = (char)FUN_004cf9d0(i, car);
                }
                if (CGameInfo::FUN_00405d80() == 5) {
                    if ((BYTE)RallyData_FUN_00406950() == 2) {
                        FUN_004cf550(car, FUN_0040ceb0(i), FUN_0040ced0(i), FUN_0040cef0(i));
                        FUN_004cf8e0(car, (int)RallyData_FUN_00408cb0(car));
                        FUN_004cfff0(i, car, (char *)RallyData_GetRecord(car));
                    }
                    if ((FUN_0041bf50(i) & 0x80) != 0 || (FUN_0041bf50(i) & 0x2000) != 0 ||
                        (FUN_0041bf50(i) & 0x1800) != 0 || (FUN_0041bf50(i) & 0x300) != 0 ||
                        (FUN_0041bf50(i) & 0x400) != 0)
                        bVar1 = 1;
                }
            }
        }
        if (g_unk0x00540898 != 0) {
            for (j = 0; j < (BYTE)CGameInfo::FUN_00405d70(); j++) {
                if (CGameInfo::FUN_00405d80() == 0 || CGameInfo::FUN_00405d80() == 1) {
                    if (RallyDataStageIndex() ==
                        FUN_004071c0(RallyDataCountryIndex(), CGameInfo::FUN_00405d90())) {
                        driver = StageTiming_GetDriverSlot(j);
                        FUN_004cf470(j, RallyTiming_FUN_0040d3d0(driver),
                                     RallyTiming_GetOverallPositionOfDriver(driver),
                                     RallyTiming_GetStageTimeSeconds(driver));
                        if (CGameInfo::FUN_00405d80() == 0) {
                            if ((BYTE)RallyDataCountryIndex() == 0)
                                FUN_004cf3f0(j);
                            FUN_004cf420(j, RallyTiming_FUN_0040d3d0(driver),
                                         RallyTiming_GetOverallPositionOfDriver(driver));
                        }
                        FUN_004cf660(j, (int)RallyData_FUN_00408cb0(j));
                        FUN_004cfb30(j, j, (char *)RallyData_GetRecord(j));
                        if ((FUN_0041bf50(j) & 0x80) != 0 || (FUN_0041bf50(j) & 0x2000) != 0)
                            bVar1 = 1;
                    }
                }
                if (CGameInfo::FUN_00405d80() == 0 && (BYTE)RallyDataCountryIndex() == 7 &&
                    RallyDataStageIndex() ==
                        FUN_004071c0(RallyDataCountryIndex(), CGameInfo::FUN_00405d90())) {
                    driver = StageTiming_GetDriverSlot(j);
                    FUN_004cf4d0(j, RallyTiming_GetStagePositionOfDriver(driver),
                                 RallyTiming_GetStageTimeSeconds(driver));
                    FUN_004cf740(j, (int)RallyData_FUN_00408cb0(j));
                    FUN_004cfc90(j, j, (char *)RallyData_GetRecord(j));
                    if ((FUN_0041bf50(j) & 0x80) != 0 || (FUN_0041bf50(j) & 0x2000) != 0)
                        bVar1 = 1;
                }
            }
        }
    }
    if (CGameInfo::FUN_00405d80() != 4 && CGameInfo::FUN_00405fe0() != NULL) {
        g_unk0x00541cd8 = *(unsigned int *)((char *)CGameInfo::FUN_00405fe0() + 0x658 +
                                            ((RallyDataCountryIndex() & 0xff) * 0xb +
                                             (RallyDataStageIndex() & 0xff)) * 8) >> 7 & 0xffff;
        strcpy(g_unk0x00519edc, (char *)CGameInfo::FUN_00405fe0() + 0x654 +
                                    ((RallyDataCountryIndex() & 0xff) * 0xb +
                                     (RallyDataStageIndex() & 0xff)) * 8);
    }
    if ((state & 1) == 0) {
        if ((BYTE)RallyData_GetFlag24() != 0) {
            if (CGameInfo::FUN_00405e00() == 0) {
                g_menu0x0053e4b8.items[0].pSubMenu = &g_menu0x0053f5b0;
            } else if (CGameInfo::FUN_00405d80() == 10) {
                g_menu0x0053f5b0.items[0].pSubMenu = &g_menu0x005406b8;
                g_menu0x005406b8.pParent = &g_menu0x0053f5b0;
            } else {
                g_menu0x0053f5b0.items[0].pSubMenu = &g_menu0x00540c68;
                g_menu0x00540c68.pParent = &g_menu0x0053f5b0;
            }
        } else if (CGameInfo::FUN_00405e00() != 0) {
            if ((BYTE)RallyData_FUN_004082e0() != 0 &&
                (RallyData_FUN_004082b0() == 1 || RallyData_FUN_004082b0() == 2)) {
                g_menu0x0053ea68.items[0].pSubMenu = &g_menu0x0053fb70;
                g_menu0x0053fb70.pParent = &g_menu0x0053ea68;
            } else {
                g_menu0x0053f5b0.items[0].pSubMenu = &g_menu0x0053fb70;
                g_menu0x0053fb70.pParent = &g_menu0x0053f5b0;
            }
        } else {
            g_menu0x0053f5b0.items[0].pSubMenu = &g_menu0x0053e2d8;
            g_menu0x0053e2d8.pParent = &g_menu0x0053f5b0;
        }
    } else {
        g_menu0x00541218.items[0].pSubMenu = &g_menu0x0053f5b0;
        if ((BYTE)RallyData_GetFlag24() != 0) {
            if (CGameInfo::FUN_00405e00() == 0) {
                g_menu0x00541218.items[0].pSubMenu = &g_menu0x0053f5b0;
            } else if (CGameInfo::FUN_00405d80() == 10) {
                g_menu0x00541218.items[0].pSubMenu = &g_menu0x005406b8;
                g_menu0x005406b8.pParent = &g_menu0x00541218;
            } else {
                g_menu0x00541218.items[0].pSubMenu = &g_menu0x00540c68;
                g_menu0x00540c68.pParent = &g_menu0x00541218;
            }
        } else if (CGameInfo::FUN_00405e00() != 0) {
            if (RallyData_FUN_004082b0() == 1 || RallyData_FUN_004082b0() == 2) {
                g_menu0x0053ea68.items[0].pSubMenu = &g_menu0x0053fb70;
                g_menu0x0053fb70.pParent = &g_menu0x0053ea68;
            } else {
                g_menu0x00541218.items[0].pSubMenu = &g_menu0x0053fb70;
                g_menu0x0053fb70.pParent = &g_menu0x00541218;
            }
        } else {
            g_menu0x00541218.items[0].pSubMenu = &g_menu0x0053e2d8;
            g_menu0x0053e2d8.pParent = &g_menu0x00541218;
        }
    }
    if ((BYTE)RallyData_FUN_00407ea0() != 0 && (BYTE)CGameInfo::FUN_00406310() != 0 && stageFlag != 0)
        FUN_004660e0(0);
    switch ((BYTE)CGameInfo::FUN_00405d80()) {
    case 0:
    case 1:
        pMenu = &g_menu0x0053f008;
        g_menu0x0053e4b8.items[0].pSubMenu = pMenu;
        if (RallyDataStageIndex() !=
            FUN_004071c0(RallyDataCountryIndex(), CGameInfo::FUN_00405d90())) {
            g_unk0x00541210 = 0;
            goto label_4a840;
        }
        g_unk0x00541210 = 1;
        if (g_unk0x00540898 == 0)
            goto label_4a840;
        if (CGameInfo::FUN_00405d80() == 0) {
            pMenu->items[0].pSubMenu = &g_menu0x0053e6a0;
            g_menu0x0053e6a0.pParent = &g_menu0x00540118;
            goto label_4a8d9;
        }
        minPos = 99;
        k = 0;
        if (CGameInfo::FUN_00405d70() != 0) {
            for (i = 0xf; k < (BYTE)CGameInfo::FUN_00405d70(); i--) {
                if (RallyTiming_GetOverallPositionOfDriver(i) < minPos)
                    minPos = RallyTiming_GetOverallPositionOfDriver(i);
                k++;
            }
        }
        if (minPos > 5) {
            pMenu->items[0].pSubMenu = &g_menu0x00541400;
            g_menu0x00541400.pParent = pMenu;
        } else {
            pMenu->items[0].pSubMenu = &g_menu0x00540118;
            g_menu0x00540118.pParent = pMenu;
            g_menu0x00540118.items[0].pSubMenu = &g_menu0x00541400;
        }
        FUN_0044af70(0, pMenu);
        goto label_4a8d9;
    label_4a840:
        if (CGameInfo::FUN_00405da0() == 0) {
            pMenu->items[0].pSubMenu = &g_menu0x00541400;
            FUN_0044af70(1, pMenu);
        } else if (g_unk0x00540898 != 0) {
            pMenu->items[0].pSubMenu = &g_menu0x00541400;
            FUN_0044af70(1, pMenu);
        } else if (g_menu0x00541218.items[0].pSubMenu == &g_menu0x0053f5b0) {
            g_menu0x0053f5b0.items[0].pSubMenu = &g_menu0x00541400;
            FUN_0044af70(1, &g_menu0x0053f5b0);
        } else {
            g_menu0x00541218.items[0].pSubMenu = &g_menu0x00541400;
            FUN_0044af70(1, &g_menu0x00541218);
        }
        result = 0x25;
        break;
    label_4a8d9:
        if (CGameInfo::FUN_00405d80() == 0) {
            g_menu0x00540118.items[0].pSubMenu = &g_menu0x00541ae0;
            minPos = 99;
            k = 0;
            if (CGameInfo::FUN_00405d70() != 0) {
                for (i = 0xf; k < (BYTE)CGameInfo::FUN_00405d70(); i--) {
                    if (RallyTiming_GetOverallPositionOfDriver(i) < minPos)
                        minPos = RallyTiming_GetOverallPositionOfDriver(i);
                    k++;
                }
            }
            if ((BYTE)RallyDataCountryIndex() == 7 && minPos < 6) {
                g_unk0x00541cf0 = 0;
                g_menu0x00541ae0.items[0].pSubMenu = &g_menu0x0053f790;
                FUN_0044af70(0, &g_menu0x0053f790);
            } else if (minPos < 6) {
                g_menu0x00541ae0.items[0].pSubMenu = &g_menu0x00541400;
                FUN_0044af70(1, &g_menu0x00541ae0);
            } else {
                g_menu0x00540118.items[0].pSubMenu = &g_menu0x00541400;
                FUN_0044af70(0, &g_menu0x00540118);
            }
        }
        if (bVar1 != 0) {
            if (CGameInfo::FUN_00405d80() == 5)
                pMenu = &g_menu0x0053e888;
            else
                pMenu = &g_menu0x00540118;
            pMenu->items[0].pSubMenu->pParent = &g_menu0x00540e50;
            g_menu0x00540e50.items[0].pSubMenu = pMenu->items[0].pSubMenu;
            g_menu0x00540e50.pParent = pMenu;
            pMenu->items[0].pSubMenu = &g_menu0x00540e50;
        }
        result = 0x25;
        break;
    case 2:
        if (CGameInfo::FUN_00405da0() == 0) {
            FUN_0044af70(0, &g_menu0x0053e4b8);
            g_menu0x0053e4b8.items[0].pSubMenu = &g_menu0x00541400;
            result = 0x23;
            break;
        }
        if (FUN_0041b370() == (BYTE)CGameInfo::FUN_00405d70() - 1) {
            g_menu0x0053e4b8.items[0].pSubMenu = &g_menu0x00541400;
            g_menu0x00541400.pParent = &g_menu0x0053e4b8;
            result = 0x23;
            break;
        }
        if (g_menu0x00541218.items[0].pSubMenu == &g_menu0x0053f5b0) {
            g_menu0x0053f5b0.items[0].pSubMenu = &g_menu0x00541400;
            FUN_0044af70(1, &g_menu0x0053f5b0);
            result = 0x23;
            break;
        }
        g_menu0x00541218.items[0].pSubMenu = &g_menu0x00541400;
        FUN_0044af70(1, &g_menu0x00541218);
        result = 0x23;
        break;
    case 3:
        if (CGameInfo::FUN_00405da0() == 0) {
            if (g_menu0x00541218.items[0].pSubMenu == &g_menu0x0053f5b0) {
                g_menu0x0053f5b0.items[0].pSubMenu = &g_menu0x00541400;
                FUN_0044af70(0, &g_menu0x0053f5b0);
            } else {
                g_menu0x00541218.items[0].pSubMenu = &g_menu0x00541400;
                FUN_0044af70(0, &g_menu0x00541218);
            }
            result = 0x23;
            break;
        }
        count = FUN_00448680(0, GetStageSplitCount());
        g_stageResultTimes[(BYTE)FUN_0041b370()] = count;
        g_stageResultRecords[(BYTE)FUN_0041b370()] = (BYTE)FUN_0041b370();
        if (FUN_0041b370() == (BYTE)CGameInfo::FUN_00405d70() - 1) {
            FUN_0044b270();
            if (g_menu0x00541218.items[0].pSubMenu == &g_menu0x0053f5b0) {
                g_menu0x0053f5b0.items[0].pSubMenu = &g_menu0x0053f3c8;
                g_menu0x0053f3c8.pParent = &g_menu0x0053f5b0;
            } else {
                g_menu0x00541218.items[0].pSubMenu = &g_menu0x0053f3c8;
                g_menu0x0053f3c8.pParent = &g_menu0x00541218;
            }
            FUN_0044af70(0, &g_menu0x0053f3c8);
            result = 0x23;
            break;
        }
        if (g_menu0x00541218.items[0].pSubMenu == &g_menu0x0053f5b0) {
            g_menu0x0053f5b0.items[0].pSubMenu = &g_menu0x00541400;
            FUN_0044af70(1, &g_menu0x0053f5b0);
        } else {
            g_menu0x00541218.items[0].pSubMenu = &g_menu0x00541400;
            FUN_0044af70(1, &g_menu0x00541218);
        }
        result = 0x23;
        break;
    case 4:
        pState = RallyData_GetChampionshipState();
        switch ((*pState >> 3) & 7) {
        case 1:
            g_pKnockoutMatch = (KnockoutMatch *)(pState + ((*pState >> 0xc) & 0xf) * 3 + 0x16);
            break;
        case 2:
            g_pKnockoutMatch = (KnockoutMatch *)(pState + ((*pState >> 0xc) & 0xf) * 3 + 10);
            break;
        case 3:
            g_pKnockoutMatch = (KnockoutMatch *)(pState + ((*pState >> 0xc) & 0xf) * 3 + 4);
            break;
        case 4:
            g_pKnockoutMatch = (KnockoutMatch *)(pState + 1);
            break;
        }
        FUN_00472a30();
        g_menu0x0053ea68.items[0].pSubMenu = &g_menu0x00541218;
        g_menu0x00541218.pParent = &g_menu0x0053ea68;
        g_menu0x00541218.items[0].pSubMenu = &g_menu0x0053f1e8;
        if ((*pState & 0x400000) == 0 || (BYTE)(*pState & 0x38) < 0x20)
            FUN_0044af70(FUN_004728e0(), &g_menu0x0053f1e8);
        else
            FUN_0044af70(0, &g_menu0x0053f1e8);
        result = 0x27;
        break;
    case 5:
        if ((BYTE)RallyData_FUN_00406940() != 2) {
            g_menu0x0053fb70.items[0].pSubMenu = &g_menu0x0053ec48;
            g_menu0x0053ec48.pParent = &g_menu0x0053e888;
            if ((BYTE)RallyData_FUN_00406950() == 2) {
                g_menu0x0053e888.items[0].pSubMenu = &g_menu0x0053ee28;
                g_menu0x00541400.pParent = &g_menu0x0053ee28;
            } else {
                g_menu0x0053e888.items[0].pSubMenu = &g_menu0x00541400;
                FUN_0044af70(1, &g_menu0x0053e888);
            }
        } else {
            g_menu0x0053fb70.items[0].pSubMenu = &g_menu0x00541400;
            g_menu0x00541400.pParent = &g_menu0x0053fb70;
        }
        if ((BYTE)RallyData_FUN_00406940() == 2)
            result = 0x29;
        else
            result = 0x2b;
        if (bVar1 != 0) {
            if (CGameInfo::FUN_00405d80() == 5)
                pMenu = &g_menu0x0053e888;
            else
                pMenu = &g_menu0x00540118;
            pMenu->items[0].pSubMenu->pParent = &g_menu0x00540e50;
            g_menu0x00540e50.items[0].pSubMenu = pMenu->items[0].pSubMenu;
            g_menu0x00540e50.pParent = pMenu;
            pMenu->items[0].pSubMenu = &g_menu0x00540e50;
        }
        break;
    case 6:
    case 7:
        g_menu0x0053fb70.items[0].pSubMenu = &g_menu0x00541400;
        g_menu0x00541400.pParent = &g_menu0x0053fb70;
        result = 0x29;
        break;
    case 8:
        stageFlag = (char)FUN_004071c0(RallyDataCountryIndex(), 2);
        g_menu0x0053fd58.pParent = &g_menu0x00540c68;
        g_menu0x00540c68.items[0].pSubMenu = &g_menu0x0053fd58;
        if (RallyDataStageIndex() == (BYTE)stageFlag) {
            g_menu0x0053fd58.items[0].pSubMenu = &g_menu0x00541030;
            g_menu0x00541030.pParent = &g_menu0x0053fd58;
            g_menu0x00541030.items[0].pSubMenu = &g_menu0x005416e0;
            FUN_0044b0d0(0, &g_menu0x00541030);
        } else {
            g_menu0x0053fd58.items[0].pSubMenu = &g_menu0x005416e0;
            FUN_0044b0d0(1, &g_menu0x0053fd58);
        }
        result = 0x25;
        break;
    case 9:
        FUN_0044b0d0(0, &g_menu0x00540c68);
        g_menu0x00540c68.items[0].pSubMenu = &g_menu0x005416e0;
        result = 0x23;
        break;
    case 10:
        FUN_0044b0d0(0, &g_menu0x005406b8);
        g_menu0x005406b8.items[0].pSubMenu = &g_menu0x005416e0;
        g_menu0x005406b8.pParent = &g_menu0x00541218;
        result = 0x23;
        break;
    case 11:
    case 12:
        FUN_0044b0d0(0, &g_menu0x0053e2d8);
        g_menu0x0053e2d8.items[0].pSubMenu = &g_menu0x005416e0;
        result = 0x23;
        break;
    default:
        result = param_1;
        break;
    }
    g_menu0x005402f8.items[Menu_FindItem(&g_menu0x005402f8, 0x3e8)].id = (short)result;
    g_menu0x00541400.items[Menu_FindItem(&g_menu0x00541400, 0x3e8)].id = (short)result;
    g_menu0x005416e0.items[Menu_FindItem(&g_menu0x005416e0, 0x3e8)].id = (short)result;
    if (FUN_004d0580() != 0) {
        g_pHeaderMenu = NULL;
        g_pSavedHeaderMenu = &g_menu0x005404d8;
    } else {
        g_pHeaderMenu = NULL;
        if (param_1 != 0)
            g_pSavedHeaderMenu = &g_menu0x0053ea68;
        else
            g_pSavedHeaderMenu = &g_menu0x0053ff38;
    }
}

// Builds the stage results menu: continue (or next stage), the replay and
// save entries, then quit.
// FUNCTION: CMR2 0x0044af70
void FUN_0044af70(char param1, Menu *pParent)
{
    Menu_Init(&g_menu0x00541400, 0, 0, 0, pParent, NULL, 1, 0, 1);
    if (param1 != 0) {
        if (CGameInfo::FUN_00405da0() &&
            FUN_0041b370() != (unsigned int)(CGameInfo::FUN_00405d70() - 1)) {
            Menu_AddItemType4(&g_menu0x00541400, 0, 0x21, (int)FUN_0044a000, 0x3ea);
        } else if (g_unk0x00541210 && g_unk0x00540898 && CGameInfo::FUN_00405d80() == 0) {
            Menu_AddItemType4(&g_menu0x00541400, 0, 0x80, (int)FUN_0044a000, -1);
        } else if (CGameInfo::FUN_00405d80() != 5) {
            Menu_AddItemType4(&g_menu0x00541400, 0, 0x1f, (int)FUN_0044a000, -1);
        } else {
            Menu_AddItemType4(&g_menu0x00541400, 0, 0x95, (int)FUN_0044a000, -1);
        }
        Menu_AddItemType2(&g_menu0x00541400, 0, -1, &g_menu0x005402f8, 0, 1000);
        Menu_AddItemType2(&g_menu0x00541400, 0, 0x2d, &g_menu0x005418d8, 0, -1);
    } else {
        Menu_AddItemType2(&g_menu0x00541400, 0, -1, NULL, (int)FUN_00449f30, 1000);
        if (CGameInfo::FUN_00405d80() == 2 || CGameInfo::FUN_00405d80() == 3)
            Menu_AddItemType2(&g_menu0x00541400, 0, 0x8b, NULL, (int)FUN_0044a0a0, 0x3e9);
        Menu_AddItemType2(&g_menu0x00541400, 0, 0xa8, NULL, (int)FUN_00449b00, -1);
    }
    Menu_SetCallbacks(&g_menu0x00541400, NULL, NULL, (MenuCallback)FUN_00453c50, NULL);
    Menu_ValidateCursor(&g_menu0x00541400, 0);
}

// Same for the time trial / network results menu.
// FUNCTION: CMR2 0x0044b0d0
void FUN_0044b0d0(char param1, Menu *pParent)
{
    g_unk0x00519ee4 = -1;
    Menu_Init(&g_menu0x005416e0, 0, 0, 0, pParent, NULL, 1, 0, 1);
    Menu_AddItemType4(&g_menu0x005416e0, 0, -1, 0, 0x3eb);
    if (param1 != 0) {
        if (FUN_004a15a0()) {
            Menu_AddItemType4(&g_menu0x005416e0, 0, 0x1f, (int)FUN_0044a000, -1);
            Menu_AddItemType2(&g_menu0x005416e0, 0, -1, &g_menu0x005402f8, 0, 1000);
            Menu_AddItemType2(&g_menu0x005416e0, 0, 0xf6, &g_menu0x005408a0, 0, -1);
        }
        Menu_AddItemType2(&g_menu0x005416e0, 0, 0x2d, &g_menu0x005418d8, 0, -1);
    } else {
        if (FUN_004a15a0()) {
            if (CGameInfo::FUN_00405d80() != 0xc)
                Menu_AddItemType2(&g_menu0x005416e0, 0, -1, NULL, (int)FUN_00449f30, 1000);
            if (CGameInfo::FUN_00405d80() == 9 || CGameInfo::FUN_00405d80() == 10)
                Menu_AddItemType2(&g_menu0x005416e0, 0, 0x8b, NULL, (int)FUN_0044a0a0, 0x3e9);
            Menu_AddItemType2(&g_menu0x005416e0, 0, 0xf6, NULL, (int)FUN_00449b60, -1);
        } else if (CGameInfo::FUN_00405d80() == 10) {
            Menu_AddItemType2(&g_menu0x005416e0, 0, -1, NULL, (int)FUN_00449f30, 1000);
            Menu_AddItemType2(&g_menu0x005416e0, 0, 0x8b, NULL, (int)FUN_0044a0a0, 0x3e9);
        }
        Menu_AddItemType2(&g_menu0x005416e0, 0, 0xa8, NULL, (int)FUN_00449b00, -1);
    }
    Menu_SetCallbacks(&g_menu0x005416e0, (MenuCallback)FUN_00449cd0, (MenuCallback)FUN_00449ce0,
                      (MenuCallback)FUN_004541c0, NULL);
    Menu_ValidateCursor(&g_menu0x005416e0, 0);
}


// Blink timer of the stage end banner.
// GLOBAL: CMR2 0x0053e880
BYTE g_unk0x0053e880[8];
// GLOBAL: CMR2 0x00541cf0
int g_unk0x00541cf0;
// GLOBAL: CMR2 0x00519f44
char g_nameSeparator0x00519f44[] = " - ";

// Stage end banner: the event title, a separator bar and the result text
// appear one after the other, then one line per driver with its category
// record name (or the driver name) and position.
// FUNCTION: CMR2 0x0044b3a0
void FUN_0044b3a0(void)
{
    int y;
    int x;
    int resY;
    int i;
    int position;
    int *pResY;
    int lineHeight;

    x = (int)(g_pGraphics->resX * 30) / 640;
    y = (int)(g_pGraphics->resY * 242) / 480;
    if (g_unk0x00541cf0 <= 0)
        return;
    if (g_unk0x00541cf0 > 0x4000 || !FUN_004bc0c0(g_unk0x0053e880)) {
        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(CGameInfo::FUN_00405d90() + 0x8e));
        Font_DrawText(2, CFrontend::m_stringDest, x, y, (int *)g_menuFrameColour, 0x11);
    }
    if (g_unk0x00541cf0 > 0x8000 || !FUN_004bc0c0(g_unk0x0053e880)) {
        x += Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest);
        x += (int)(g_pGraphics->resX * 8) / 640;
        pResY = &g_pGraphics->resY;
        g_menuRect[0] = (short)x;
        resY = *pResY;
        lineHeight = Font_GetLineHeight(2);
        g_menuRect[2] = 2;
        g_menuRect[1] = (short)(resY * 242 / 480 + resY * 4 / 480 - lineHeight);
        g_menuRect[3] = (short)((int)(*pResY * 41) / 480);
        Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, g_menuFrameColour, 2);
        x += (int)(g_pGraphics->resX * 8) / 640;
        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x93));
        Font_DrawText(2, CFrontend::m_stringDest, x, y, (int *)g_menuFrameColour, 0x11);
    }
    if (g_unk0x00541cf0 <= 0xc000 && FUN_004bc0c0(g_unk0x0053e880))
        return;
    for (i = 0; i < CGameInfo::FUN_00405d70(); i++) {
        position = RallyTiming_GetStagePositionOfDriver(StageTiming_GetDriverSlot(i));
        if (strlen((char *)FUN_00408470(i)) != 0 && strlen((char *)FUN_00408470(i)) < 0x1e) {
            sprintf(CFrontend::m_stringDest, (char *)FUN_00408470(i));
            sprintf(CFrontend::m_stringDest + strlen((char *)FUN_00408470(i)), g_nameSeparator0x00519f44);
        } else {
            sprintf(CFrontend::m_stringDest, (char *)RallyData_GetRecord(i));
            sprintf(CFrontend::m_stringDest + strlen((char *)RallyData_GetRecord(i)), g_nameSeparator0x00519f44);
        }
        switch (position) {
        case 0:
            sprintf(CFrontend::m_stringDest + strlen(CFrontend::m_stringDest), CFrontend::GetTextString(0x51));
            break;
        case 1:
            sprintf(CFrontend::m_stringDest + strlen(CFrontend::m_stringDest), CFrontend::GetTextString(0x52));
            break;
        case 2:
            sprintf(CFrontend::m_stringDest + strlen(CFrontend::m_stringDest), CFrontend::GetTextString(0x53));
            break;
        default:
            sprintf(CFrontend::m_stringDest + strlen(CFrontend::m_stringDest), CFrontend::GetTextString(0x54), position + 1);
            break;
        }
        CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
        Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 30) / 640,
                      (int)(g_pGraphics->resY * 242) / 480 +
                          (i + 1) * ((int)(g_pGraphics->resY * 10) / 480 + Font_GetLineHeight(0)),
                      (int *)g_menuFrameColour, 0x11);
    }
}

// Clears the menu list: fills the whole screen with the highlight colour
// through the menu sprite buffer (the sprite layer only tints, so the menu
// colour is blended first).
// FUNCTION: CMR2 0x0044b760
void FUN_0044b760(void)
{
    g_menuRect[0] = 0;
    g_menuRect[1] = 0;
    g_menuRect[2] = (short)g_pGraphics->resX;
    g_menuRect[3] = (short)g_pGraphics->resY;
    Font_SetBlendMode(2);
    Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, (BYTE *)&g_menuHighlightColour, 2);
}

// Fills the row the cursor is on with the highlight colour.
// FUNCTION: CMR2 0x0044f680
void GameMenus_DrawRowHighlight(short row)
{
    DWORD colour;

    colour = g_menuHighlightColour;
    *((BYTE *)&colour + 3) = 0xff;
    g_menuRect[1] = (short)((int)(g_pGraphics->resY * 0x82) / 0x1e0) +
                    (short)((int)(g_pGraphics->resY * 0x34) / 0x1e0) * row -
                    (short)g_unk0x00540c60 - 1;
    g_menuRect[3] = (short)((int)(g_pGraphics->resY * 0xa3) / 0x1e0) -
                    (short)((int)(g_pGraphics->resY * 0x82) / 0x1e0) + 1;
    g_menuRect[0] = (short)((int)(g_pGraphics->resX * 0x20) / 0x280);
    g_menuRect[2] = (short)((int)(g_pGraphics->resX * 0x41) / 0x280) -
                    (short)((int)(g_pGraphics->resX * 0x20) / 0x280);
    Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, (BYTE *)&colour, 2);
}

// Draws the thin separator under one menu row.
// FUNCTION: CMR2 0x0044f7b0
void GameMenus_DrawRowSeparator(short row)
{
    int resY;

    g_menuRect[0] = (short)((int)(g_pGraphics->resX * 0x20) / 0x280);
    resY = (int)g_pGraphics->resY;
    g_menuRect[1] = (short)((resY * 0x2f) / 0x1e0) +
                    (short)((resY * 0x82) / 0x1e0) +
                    (short)((resY * 0x34) / 0x1e0) * row -
                    (short)g_unk0x00540c60;
    g_menuRect[2] = (short)((int)(g_pGraphics->resX * 0x240) / 0x280);
    g_menuRect[3] = 1;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, g_menuFrameColour, 2);
}

// GLOBAL: CMR2 0x00519f98
char g_strInvalid[8] = "INVALID";
// GLOBAL: CMR2 0x00519fa0
char g_pointsFormat[] = "%d pts";
// GLOBAL: CMR2 0x00519fa8
char g_plusTimeFormat[] = "+ %s";

// Formats the name of one of the eight game modes into the shared string.
// FUNCTION: CMR2 0x00451ce0
void GameMenus_FormatModeName(int unused, int mode)
{
    switch (mode) {
    case 0:
        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(6));
        return;
    case 1:
        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(3));
        return;
    case 2:
        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(1));
        return;
    case 3:
        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(4));
        return;
    case 4:
        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0));
        return;
    case 5:
        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(2));
        return;
    case 6:
        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(5));
        return;
    case 7:
        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(7));
        return;
    default:
        sprintf(CFrontend::m_stringDest, g_strInvalid);
        return;
    }
}

// Size taken from the next known global (0x538a84); nothing reads past it yet.
// GLOBAL: CMR2 0x00538130
BYTE g_unk0x00538130[0x40];

// FUNCTION: CMR2 0x0041f900
BYTE *FUN_0041f900(void)
{
    return g_unk0x00538130;
}

// GLOBAL: CMR2 0x005418d4
BYTE g_unk0x005418d4;
// Menu shown under the header when it is restored.
// GLOBAL: CMR2 0x00541ccc
Menu *g_pSavedHeaderMenu;

// FUNCTION: CMR2 0x0044a120
void FUN_0044a120(void)
{
    g_unk0x005418d4 = 0;
}

// FUNCTION: CMR2 0x00448e60
void FUN_00448e60(void)
{
    g_pHeaderMenu = g_pSavedHeaderMenu;
}

extern Menu g_menu0x0053ea68;

// FUNCTION: CMR2 0x0044a130
BYTE FUN_0044a130(void)
{
    return g_pHeaderMenu == &g_menu0x0053ea68;
}

int Timer_GetValue(BYTE index);
void FUN_004bc290(BYTE *pSlot, int shape, int length, int param4, int start, int end, BYTE param7);

// Starts the fade timer of the pause overlay if it is not running and has
// not run yet, and samples its value.
// FUNCTION: CMR2 0x0044a150
void FUN_0044a150(void)
{
    if (FUN_004bc0c0(g_unk0x0053e880) == 0 && g_unk0x00541cf0 == 0)
        FUN_004bc290(g_unk0x0053e880, 0, 200, 0, 0, 0x10000, 1);
    if (FUN_004bc0c0(g_unk0x0053e880))
        g_unk0x00541cf0 = Timer_GetValue(g_unk0x0053e880[0]);
}

// GLOBAL: CMR2 0x00541dfc
Menu *g_unk0x00541dfc;
// GLOBAL: CMR2 0x00541e00
int g_unk0x00541e00;

// Per-frame update of the header menu: waits two frames after the menu
// changes before running its callback 2 (and the network one).
// FUNCTION: CMR2 0x0044b330
void FUN_0044b330(void)
{
    if (g_unk0x00541dfc == g_pHeaderMenu || g_pHeaderMenu == &g_menu0x0053ea68) {
        if (g_unk0x00541e00 > 0)
            goto tail;
        Menu_CallCallback2(g_pHeaderMenu);
        if ((BYTE)FUN_00407270())
            FUN_0044b3a0();
        g_unk0x0053e698 = 0;
        return;
    } else {
        g_unk0x00541dfc = g_pHeaderMenu;
        g_unk0x00541e00 = 2;
    }
tail:
    FUN_0044b760();
    g_unk0x0053e698 = 0;
    g_unk0x00541e00--;
}

// Sorts the stage results by time (zero times, i.e. no time, go last).
// FUNCTION: CMR2 0x0044b270
void FUN_0044b270(void)
{
    int i;
    int j;
    int best = 0;
    int bestTime;
    int time;
    BYTE record;
    BYTE recordI;

    for (i = 0; i < (int)((FUN_0041b370() & 0xff) + 1); i++) {
        bestTime = 0xffff;
        for (j = i; j < (int)((FUN_0041b370() & 0xff) + 1); j++) {
            if (g_stageResultTimes[j] != 0 && g_stageResultTimes[j] < bestTime) {
                bestTime = g_stageResultTimes[j];
                best = j;
            }
        }
        time = g_stageResultTimes[i];
        g_stageResultTimes[i] = g_stageResultTimes[best];
        g_stageResultTimes[best] = time;
        record = g_stageResultRecords[i];
        g_stageResultRecords[i] = g_stageResultRecords[best];
        g_stageResultRecords[best] = record;
    }
}

// --- 0x00448e70: stage results header fade (layer 0) -------------------------
unsigned char RallyDataState(void);
int FUN_00407270(void);
int FUN_00428740(BYTE index);
int FUN_00458390(void);
int FUN_004781c0(int index);
void FUN_0040bad0(void);
void FUN_0040bd60(unsigned short slot, DeviceInfo *pOut);
void FUN_004bc290(BYTE *pSlot, int shape, int length, int param4, int start, int end, BYTE param7);
int Timer_GetValue(BYTE index);

// Timer handle of the results header fade (first byte is the timer slot).
// GLOBAL: CMR2 0x005418c0
BYTE g_unk0x005418c0[4];
// GLOBAL: CMR2 0x00541cf4
int g_unk0x00541cf4;

// Refreshes the header of the stage results screen: drops any driver record
// still being played in, snaps the header menu back to its saved parent and
// starts (or cancels) the fade of the results panel depending on the state of
// the header menu and the pressed buttons.
// FUNCTION: CMR2 0x00448e70
void FUN_00448e70(void)
{
    DeviceInfo *pDevice;
    unsigned int flags;
    int action;
    int i;

    for (i = 0; i < (BYTE)RallyDataState(); i++) {
        if (FUN_00428740(i) != 0)
            return;
    }
    FUN_00448e60();
    FUN_0040bad0();
    FUN_0040bd60(0, CInput::FUN_0049ead0(0));
    pDevice = CInput::FUN_0049ead0(0);
    flags = pDevice->field_0x8;
    if ((BYTE)FUN_00407270() != 0)
        flags &= 0x10;
    if (g_pHeaderMenu == &g_menu0x0053ea68)
        flags &= 0xffdf;
    if (g_pHeaderMenu == &g_menu0x0053f008 || g_pHeaderMenu == &g_menu0x0053e4b8 ||
        g_pHeaderMenu == &g_menu0x0053e6a0 || g_pHeaderMenu == &g_menu0x00541ae0) {
        flags &= 0x3c;
        if ((flags & 4) == 0 || g_pHeaderMenu->items[0].max <= 0) {
            if ((flags & 8) != 0 &&
                (int)(BYTE)g_pHeaderMenu->items[0].max < (int)(BYTE)g_pHeaderMenu->items[0].min - 1) {
                FUN_004bc290(g_unk0x005418c0, 2, 7, 0, 0, 0x10000, 0);
                g_unk0x0053f5a8 = 1;
            }
        } else {
            FUN_004bc290(g_unk0x005418c0, 2, 7, 0, 0x10000, 0, 0);
            g_unk0x005413f8 = 1;
        }
        if (FUN_004bc0c0(g_unk0x005418c0)) {
            g_unk0x00541cf4 = Timer_GetValue(g_unk0x005418c0[0]);
            g_unk0x00540c60 = (FixMul(((int)g_pGraphics->resY * 0x34) / 0x1e0 * 0x10000, g_unk0x00541cf4) >> 16);
        } else {
            g_unk0x0053f5a8 = 0;
            g_unk0x005413f8 = 0;
            g_unk0x00540c60 = 0;
        }
    } else {
        g_unk0x0053f5a8 = 0;
        g_unk0x005413f8 = 0;
        g_unk0x00540c60 = 0;
    }
    action = Menu_Update(g_pHeaderMenu, flags);
    if (action != 0) {
        g_menuBuildTime = FUN_004781c0(0);
        g_pSavedHeaderMenu = (Menu *)action;
    }
}
