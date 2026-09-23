#include <windows.h>
#include <mmsystem.h>
#include "GameMenus.h"
#include "GameInfo.h"
#include "Frontend.h"
#include "Font.h"
#include "RallyTiming.h"
#include "StageTiming.h"

int FUN_0040ab10(void);

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

// STUB: CMR2 0x00449b00
void FUN_00449b00(void)
{
}

// STUB: CMR2 0x00449b60
void FUN_00449b60(void)
{
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

// STUB: CMR2 0x00449f30
void FUN_00449f30(void)
{
}

// STUB: CMR2 0x0044a000
void FUN_0044a000(Menu *pMenu, int param1)
{
}

// STUB: CMR2 0x0041f2b0
void FUN_0041f2b0(void)
{
}

// FUNCTION: CMR2 0x0044a090
void FUN_0044a090(Menu *pMenu, int param)
{
    FUN_0041f2b0();
    CGameInfo::FUN_0049ea90(0);
}

// STUB: CMR2 0x0044a0a0
void FUN_0044a0a0(void)
{
}

// STUB: CMR2 0x0044b7b0
void FUN_0044b7b0(void)
{
}

// STUB: CMR2 0x0044bc30
void FUN_0044bc30(void)
{
}

// STUB: CMR2 0x0044bcd0
void FUN_0044bcd0(Menu *pMenu)
{
}

// STUB: CMR2 0x0044d790
void FUN_0044d790(void)
{
}

// STUB: CMR2 0x0044d960
void FUN_0044d960(void)
{
}

// STUB: CMR2 0x0044e130
void FUN_0044e130(void)
{
}

// STUB: CMR2 0x0044efa0
void FUN_0044efa0(Menu *pMenu)
{
}

// STUB: CMR2 0x0044fea0
void FUN_0044fea0(Menu *pMenu)
{
}

// STUB: CMR2 0x004505b0
void FUN_004505b0(void)
{
}

// STUB: CMR2 0x00450c10
void FUN_00450c10(void)
{
}

// STUB: CMR2 0x00450ed0
void FUN_00450ed0(void)
{
}

// STUB: CMR2 0x00450ef0
void FUN_00450ef0(Menu *pMenu)
{
}

// STUB: CMR2 0x00451690
void FUN_00451690(void)
{
}

// STUB: CMR2 0x00451df0
void FUN_00451df0(Menu *pMenu)
{
}

// STUB: CMR2 0x004529c0
void FUN_004529c0(void)
{
}

// STUB: CMR2 0x00452be0
void FUN_00452be0(void)
{
}

// STUB: CMR2 0x004530e0
void FUN_004530e0(void)
{
}

// STUB: CMR2 0x00453830
void FUN_00453830(void)
{
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

#include <cstdio>
#include "Sprite.h"
#include "GenericFileLoader.h"
#include "Graphics.h"
#include "RallyData.h"
#include "NetPlayers.h"
#include "Input.h"
#include "TimingUtils.h"
#include "main.h"

int FUN_0040ae90(void);
NetClassification *FUN_0040aea0(int index);
unsigned int FUN_0040aeb0(int index);
unsigned int FUN_0040aec0(int index);
void GameMenus_DrawTextRow(int x, int y, char *pText, ...);
void GameMenus_DrawRowFrame(short row, short yOffset, char compact);
char *FUN_0040abb0(int index, int total);
int FUN_0040ac10(int index);
int FUN_0040ab50(int index, int total);
unsigned int FUN_0040abe0(int index, int total);
int FUN_0040ab20(int index, int total);
unsigned int FUN_0040ab80(int index, int total);
void FUN_00451890(Menu *pMenu);
int FUN_004055e0(void);
int FUN_004055f0(void);
BYTE FUN_0041b370(void);

// GLOBAL: CMR2 0x00541cc0
short g_menuRect[4];
// GLOBAL: CMR2 0x00519ecc
BYTE g_menuFrameColour[4] = { 0, 0, 0, 0 };
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
BYTE g_menuTextColour[4] = { 0xff, 0xff, 0xff, 0xff };

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

// Breadcrumb header of the in-game screens (pending: 1100 bytes). Empty body,
// with no annotation, only so its callers can be measured.
void FUN_00451890(Menu *pMenu)
{
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
// FUNCTION: CMR2 0x00454df0
void GameMenus_DrawTextRow(int x, int y, char *pText, ...)
{
    int width;

    g_menuRect[2] = 2;
    g_menuRect[1] = (short)y - (short)((int)(g_pGraphics->resY * 0x1e) / 0x1e0);
    g_menuRect[3] = (short)((int)(g_pGraphics->resY * 0x29) / 0x1e0);
    if (pText != NULL) {
        char **ppNext = &pText;
        char *pCur = pText;

        for (;;) {
            CGenericFileLoader::StrLowerPolish(pCur);
            Font_DrawText(2, pCur, x, y, (int *)g_menuFrameColour, 0x11);
            sprintf(CFrontend::m_stringDest, pCur);
            pCur = ppNext[1];
            ppNext++;
            if (pCur == NULL) {
                break;
            }
            width = Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest);
            width = (int)(g_pGraphics->resX * 8) / 0x280 + x + width;
            g_menuRect[0] = (short)width;
            Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, g_menuFrameColour, 2);
            x = width + 2 + (int)(g_pGraphics->resX * 8) / 0x280;
        }
    }
}

// GLOBAL: CMR2 0x00519ed4
DWORD g_menuHighlightColour;

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
    g_menuRect[3] = 1;
    g_menuRect[2] = (short)((int)(g_pGraphics->resX * 0x240) / 0x280);
    Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, g_menuFrameColour, 2);
}

// GLOBAL: CMR2 0x00519f98
char g_strInvalid[8] = "INVALID";

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
