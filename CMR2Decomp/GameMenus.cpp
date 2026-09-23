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
unsigned int RallyDataState(void);
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

// Fade callback of the "quit" item: ends the championship and leaves.
// TODO: CMR2 0x00449020 (implemented, match 97%)
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
// TODO: CMR2 0x00449f30 (implemented, match 67%)
void FUN_00449f30(Menu *pMenu, int param)
{
    BYTE i;
    int value;

    for (i = 0; (short)i < Car_GetOrderCount(); i++) {
        if (CGameInfo::FUN_00405d80() != 8 && CGameInfo::FUN_00405d80() != 1 && CGameInfo::FUN_00405d80() != 0)
            value = g_unk0x00541cd4;
        else
            value = g_unk0x00519ec8;
        FUN_004283e0(i, i != 0 ? NULL : FUN_00449ea0, 1, 0, value, 1);
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
// TODO: CMR2 0x0041f2b0 (implemented, match 85%)
void FUN_0041f2b0(void)
{
    int i;

    for (i = 0; i < (int)(RallyDataState() & 0xff); i++)
        FUN_00469b50(i);
    for (i = 0; i < *(BYTE *)g_unk0x00537f0c[5]; i++) {
        CGame::FUN_0049c1c0((Unk0049c2c0 *)g_unk0x00537f0c[5], i, 0, 2);
        FUN_0046d2a0((int *)g_unk0x00537f3c[i]);
    }
    if ((BYTE)RallyData_FUN_00407ea0() && CGameInfo::FUN_00406310())
        FUN_00466080();
    if (CGameInfo::FUN_00405d80() != 4)
        return;
    if (!(*RallyData_GetChampionshipState() & 0x800000)) {
        FUN_004728b0();
        FUN_0041f280();
        return;
    }
    FUN_0041f2a0();
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
BYTE FUN_00422fb0(unsigned int index);
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
// TODO: CMR2 0x0044b7b0 (implemented, match 48%)
void FUN_0044b7b0(Menu *pMenu)
{
    int x;
    int width;
    char *pText;
    char result;

    Font_DrawText(2, CFrontend::GetTextString(0x47), (int)(g_pGraphics->resX * 30) / 640,
                  (int)(g_pGraphics->resY * 0x43) / 480, (int *)g_menuFrameColour, 0x11);
    width = g_pGraphics->resX;
    x = (width * 30) / 640 + Font_GetTextWidth(2, (BYTE *)CFrontend::GetTextString(0x47));
    g_menuRect[0] = (short)((width * 8) / 640 + x);
    g_menuRect[2] = 2;
    g_menuRect[1] = (short)((int)(g_pGraphics->resY * 0x25) / 480);
    g_menuRect[3] = (short)((int)(g_pGraphics->resY * 0x29) / 480);
    x = g_menuRect[0] + 2 + (int)(g_pGraphics->resX * 8) / 640;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_menuRect, g_menuFrameColour, 2);
    if (FUN_0041f3a0()) {
        if (CGameInfo::FUN_00405d80() == 4)
            pText = FUN_00473810(g_pKnockoutMatch, FUN_00422fb0(1));
        else
            pText = (char *)RallyData_GetRecord(FUN_00422fb0(1));
    } else if ((BYTE)RallyData_FUN_00407e70() && (BYTE)RallyDataState() == 1) {
        if (RallyData_FUN_00408500(FUN_00422fb0(0)) == -1)
            pText = (char *)RallyData_GetRecord(FUN_00422fb0(0));
        else
            pText = CAIHelper::GetNameForID(FUN_00422fb0(0));
    } else if (CGameInfo::FUN_00405d80() == 4) {
        if (!FUN_00473790(g_pKnockoutMatch, FUN_004737d0(g_pKnockoutMatch, FUN_00422fb0(0))))
            pText = FUN_004736b0(g_pKnockoutMatch, FUN_004737d0(g_pKnockoutMatch, FUN_00422fb0(0)));
        else
            pText = FUN_00473810(g_pKnockoutMatch, FUN_004737d0(g_pKnockoutMatch, FUN_00422fb0(0)));
    } else if ((BYTE)RallyData_FUN_00407e90() && !CGameInfo::FUN_00405e00()) {
        FUN_004125a0(StageTiming_FUN_00455ac0(FUN_0041b370(), FUN_00422fb0(0)));
        Font_DrawText(2, CFrontend::m_stringDest, x, (int)(g_pGraphics->resY * 0x43) / 480, (int *)g_menuFrameColour, 0x11);
        Font_SetBlendMode(2);
        return;
    } else {
        result = RallyData_FUN_00408500(FUN_00422fb0(0) + FUN_0041b370());
        if (result == -1)
            pText = (char *)RallyData_GetRecord(FUN_00422fb0(0) + FUN_0041b370());
        else
            pText = CAIHelper::GetNameForID(RallyData_FUN_00408500(FUN_00422fb0(0) + FUN_0041b370()));
    }
    Font_DrawText(2, pText, x, (int)(g_pGraphics->resY * 0x43) / 480, (int *)g_menuFrameColour, 0x11);
    Font_SetBlendMode(2);
}

// Draw callback of the "waiting for the other players" screen.
// TODO: CMR2 0x0044bc30 (implemented, match 81%)
void FUN_0044bc30(Menu *pMenu)
{
    Font_DrawText(2, CFrontend::GetTextString(0x47), (int)(g_pGraphics->resX * 30) / 640,
                  (int)(g_pGraphics->resY * 0x43) / 480, (int *)g_menuFrameColour, 0x11);
    Font_DrawText(2, CFrontend::GetTextString(0xfa), g_pGraphics->resX / 2, g_pGraphics->resY / 2,
                  (int *)g_menuFrameColour, 0x12);
    Font_SetBlendMode(2);
}

// STUB: CMR2 0x0044bcd0
void FUN_0044bcd0(Menu *pMenu)
{
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
// TODO: CMR2 0x0044d790 (implemented, match 89%)
void FUN_0044d790(Menu *pMenu)
{
    int best;
    int winner;
    int i;
    char *pName;
    char *pText;
    int resY;

    best = 0x10;
    winner = -1;
    FUN_0044b760();
    if (CGameInfo::FUN_00405d80() != 5 && CGameInfo::FUN_00405d80() != 6) {
        GameMenus_DrawTextRow((int)(g_pGraphics->resX * 30) / 640, (int)(g_pGraphics->resY * 242) / 480,
                              CFrontend::GetTextString(0x49), CFrontend::GetTextString(0x46), 0);
        pText = CFrontend::GetTextString(0x4a);
        pName = g_unk0x00519edc;
    } else {
        GameMenus_DrawTextRow((int)(g_pGraphics->resX * 30) / 640, (int)(g_pGraphics->resY * 242) / 480,
                              CFrontend::GetTextString(0x49), 0);
        for (i = 0; i < (int)(RallyDataState() & 0xff); i++) {
            if (FUN_00451850(i) < best) {
                best = FUN_00451850(i);
                winner = i;
            }
        }
        pText = CFrontend::GetTextString(0x4a);
        pName = (char *)RallyData_GetRecord(winner);
    }
    sprintf(CFrontend::m_stringDest, g_winnerFormat, pName, pText);
    CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
    resY = g_pGraphics->resY;
    Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 30) / 640,
                  (resY * 10) / 480 + (resY * 242) / 480 + Font_GetLineHeight(0), (int *)g_menuFrameColour, 0x11);
    Font_SetBlendMode(2);
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

// Draw callback of the championship standings screen: the best driver's
// position decides between the "champion" and "rally over" headers.
// TODO: CMR2 0x00450c10 (implemented, match 78%)
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
    slot = 99;
    if (CGameInfo::FUN_00405d70() != 0) {
        slot = 0xf;
        do {
            if (RallyTiming_GetOverallPositionOfDriver(slot) < best)
                best = RallyTiming_GetOverallPositionOfDriver(slot);
            i++;
            slot--;
        } while (i < CGameInfo::FUN_00405d70());
        slot = best;
    }
    GameMenus_DrawTextRow(x, y, CFrontend::GetTextString(RallyDataCountryIndex() & 0xff),
                          CFrontend::GetTextString(0x41), CFrontend::GetTextString(slot < 6 ? 0x49 : 0x88), 0);
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
        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(RallyDataCountryIndex() == 7 ? 0xee : 0xbb));
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

// STUB: CMR2 0x00450ef0
void FUN_00450ef0(Menu *pMenu)
{
}

// Draw callback of the final championship standings (header menu) screen.
// TODO: CMR2 0x00451690 (implemented, match 78%)
void FUN_00451690(Menu *pMenu)
{
    char position[100];
    char *pPosition;
    int i;
    int place;
    int resY;

    if (g_pHeaderMenu != &g_menu0x0053f790) {
        if (!FUN_00407270())
            FUN_0044b760();
        return;
    }
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
}

extern char g_pointsFormat[];
extern char g_plusTimeFormat[];
extern BYTE g_menuRowFillColour[4];
extern char g_stageNumberFormat[];
extern char g_stageResultSameTime[];
int FUN_0040ce40(int position);
int FUN_004483c0(int index);
int FUN_00448c60(int index);
BYTE FUN_00407fc0(int param1);
void FormatCentisecondsAsMinSecMSec(int iTime, char *pcFormattedTime);
void GameMenus_DrawRowFrame(short row, short yOffset, char compact);
void FUN_00451890(Menu *pMenu);
unsigned int RallyData_FUN_004082b0(void);
unsigned int RallyData_FUN_004082d0(void);
unsigned int RallyData_FUN_004082e0(void);

// Draw callback of the stage classification table: one boxed row per car
// with its name, time (or target time, or points) and car, the players'
// rows highlighted, and the position number on the left ("=" for a tie).
// TODO: CMR2 0x00451df0 (implemented, match 87%)
void FUN_00451df0(Menu *pMenu)
{
    char diff[12];
    BOOL isPlayer;
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
// TODO: CMR2 0x004529c0 (implemented, match 59%)
void FUN_004529c0(Menu *pMenu)
{
    char name[4];
    unsigned int *pState;
    int x;
    int y;
    BYTE driver;
    int lineHeight;

    x = (int)(g_pGraphics->resX * 30) / 640;
    y = (int)(g_pGraphics->resY * 242) / 480;
    pState = RallyData_GetChampionshipState();
    FUN_0044b760();
    if (FUN_00472990(g_pKnockoutMatch)) {
        Font_DrawText(2, CFrontend::GetTextString(0x49), x, y, (int *)g_menuFrameColour, 0x11);
        if (g_pKnockoutMatch->time1 > g_pKnockoutMatch->time2)
            driver = FUN_0041b370() + ((BYTE)(g_pKnockoutMatch->flags >> 5) & 0x1f);
        else
            driver = FUN_0041b370() + ((BYTE)g_pKnockoutMatch->flags & 0x1f);
        sprintf(name, (char *)RallyData_GetRecord(driver));
        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString((*pState & 0x38) < 0x20 ? 0x7e : 0x7d), name);
        CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
    } else {
        sprintf(name, (char *)RallyData_GetRecord(FUN_0041b370() + (g_pKnockoutMatch->flags & 0x1f)));
        sprintf(CFrontend::m_stringDest, g_keypadFormat, CFrontend::GetTextString(0x88), name);
        Font_DrawText(2, CFrontend::m_stringDest, x, y, (int *)g_menuFrameColour, 0x11);
        sprintf(CFrontend::m_stringDest,
                CFrontend::GetTextString((*pState & 0x38) < 0x20 && (*pState & 7) != 1 ? 0x91 : 0x92));
        CGenericFileLoader::StrUpperPolish((BYTE *)CFrontend::m_stringDest);
    }
    lineHeight = Font_GetLineHeight(0);
    Font_DrawText(0, CFrontend::m_stringDest, x, (int)(g_pGraphics->resY * 5) / 480 + y + lineHeight,
                  (int *)g_menuFrameColour, 0x11);
}

int FUN_0041bf50(int index);
int FUN_0041bf60(int index);
int FUN_0041bf70(int index);

// Draw callback of the stage penalties screen: for every car, its
// disqualification or retirement reason, jump start / speeding penalties
// and car damage notes, one line each.
// TODO: CMR2 0x00452be0 (implemented, match 92%)
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

// STUB: CMR2 0x004530e0
void FUN_004530e0(void)
{
}

int FUN_0040ceb0(int index);
unsigned int RallyData_FUN_00406940(void);

// Draw callback of the rally results screen: "stage N of the rally" with the
// rally name between two separator bars (before the first driver in the top
// three), then one line per driver with its rally position.
// TODO: CMR2 0x00453830 (implemented, match 80%)
void FUN_00453830(Menu *pMenu)
{
    BOOL shown;
    int x;
    int place;
    int i;
    int resY;
    int lineHeight;
    unsigned int *pResY;

    shown = FALSE;
    x = (int)(g_pGraphics->resX * 30) / 640;
    FUN_0044b760();
    sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x43), 0x62 - (RallyData_FUN_00406940() & 0xff));
    Font_DrawText(2, CFrontend::m_stringDest, x, (int)(g_pGraphics->resY * 242) / 480, (int *)g_menuFrameColour, 0x11);
    x += Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest);
    for (i = 0; i < (int)(RallyDataState() & 0xff); i++) {
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
            x += Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest) + (int)(g_pGraphics->resX * 8) / 640;
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
// TODO: CMR2 0x00451890 (implemented, match 93%)
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
// TODO: CMR2 0x00449ce0 (implemented, match 87%)
void FUN_00449ce0(Menu *pMenu)
{
    DeviceInfo *pDevice;
    int key;
    int len;

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
        if (g_chatLineLength < 0xff && len < ((int)g_pGraphics->resX >= 0x400 ? 0x210 : 0x14a)) {
            g_chatLine[g_chatLineLength] = (char)key;
            g_chatLine[g_chatLineLength + 1] = 0;
            g_chatLineLength++;
        }
        break;
    }
}

// Not decompiled yet (0x4541c0 is the draw callback of g_menu0x005416e0).

void FUN_004541c0(Menu *pMenu)
{
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
// TODO: CMR2 0x0044b3a0 (implemented, match 89%)
void FUN_0044b3a0(void)
{
    int lineHeight;
    int y;
    int x;
    int resY;
    int i;
    int position;
    unsigned int *pResY;

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
    g_menuRect[3] = 1;
    g_menuRect[2] = (short)((int)(g_pGraphics->resX * 0x240) / 0x280);
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
