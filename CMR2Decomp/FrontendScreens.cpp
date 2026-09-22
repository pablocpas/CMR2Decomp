#include "Frontend.h"
#include "Game.h"
#include "GameInfo.h"
#include "RallyData.h"
#include "Graphics.h"
#include "FrontendMenus.h"

// Callbacks of the frontend screens, hooked to the menus built in
// FrontendMenus.cpp.

// GLOBAL: CMR2 0x00818f10
char g_unk0x00818f10;
// GLOBAL: CMR2 0x00819878
BYTE g_unk0x00819878;

// FUNCTION: CMR2 0x004ecfa0
void FUN_004ecfa0(Menu *pMenu, char param)
{
    if (param != 0 && g_unk0x00818f10 != 0) {
        CGame::DestroyDirectPlayLobby();
        CGame::DestroyDirectPlay();
        g_unk0x00818f10 = 0;
    }
}

// FUNCTION: CMR2 0x004ef5f0
void FUN_004ef5f0(Menu *pMenu, int param)
{
    RallyData_FUN_0040d620(1);
}

// FUNCTION: CMR2 0x004ef8f0
void FUN_004ef8f0(Menu *pMenu, int param)
{
    if ((*CGameInfo::FUN_00405db0() & 1) || CGameInfo::FUN_00406410(0xd))
        pMenu->items[2].enabled = 1;
    else
        pMenu->items[2].enabled = 0;
    pMenu->cursor = CGameInfo::FUN_00405d90();
}

// FUNCTION: CMR2 0x004ef950
void FUN_004ef950(Menu *pMenu, int param)
{
    pMenu->cursor = CGameInfo::FUN_00405dd0();
}

// Days in the month of the date edited by items 0 (year), 1 (month) and 2 (day)
// FUNCTION: CMR2 0x004f1640
void FUN_004f1640(Menu *pMenu)
{
    int days;
    int day = pMenu->items[2].max + 1;
    int year = pMenu->items[0].max + 1850;

    switch (pMenu->items[1].max) {
    case 3:
    case 5:
    case 8:
    case 10:
        pMenu->items[2].min = days = 30;
        break;
    case 1:
        if (year % 4 == 0)
            pMenu->items[2].min = days = 29;
        else
            pMenu->items[2].min = days = 28;
        break;
    case 0:
    case 2:
    case 4:
    case 6:
    case 7:
    case 9:
    case 11:
        pMenu->items[2].min = days = 31;
        break;
    }
    if (day > days)
        pMenu->items[2].max = days - 1;
}

// FUNCTION: CMR2 0x004f1b90
void FUN_004f1b90(Menu *pMenu, int param)
{
    FUN_004ea480(0);
}

// FUNCTION: CMR2 0x004f1d00
void FUN_004f1d00(Menu *pMenu, int param)
{
    Menu_GetItem(pMenu, 8)->max = CGameInfo::FUN_00405cd0();
    if (CGameInfo::FUN_00405d00() != 0)
        Menu_GetItem(pMenu, 9)->max = 1;
    else
        Menu_GetItem(pMenu, 9)->max = 0;
    if (CGameInfo::FUN_00405d10() != 0) {
        Menu_GetItem(pMenu, 10)->max = 1;
        return;
    }
    Menu_GetItem(pMenu, 10)->max = 0;
}

// FUNCTION: CMR2 0x004f1d60
void FUN_004f1d60(Menu *pMenu)
{
    if (CFrontend::FUN_004b7560(0x400) && CFrontend::FUN_004b7590(0x400))
        return;
    if (pMenu->items[pMenu->cursor].value == 10) {
        Menu_SetFlags(pMenu, 1, 0, 1, 1);
        return;
    }
    Menu_SetFlags(pMenu, 1, 1, 1, 1);
}

// FUNCTION: CMR2 0x004f23d0
void FUN_004f23d0(Menu *pMenu)
{
    unsigned int count = CGraphics::GetDisplayCount();
    if (count > 10)
        count = 10;
    if (pMenu->items[0].max < g_unk0x00819128)
        g_unk0x00819128--;
    if (pMenu->items[0].max >= (int)(count + g_unk0x00819128))
        g_unk0x00819128++;
    if (CGame::FUN_004a9b20() != 0)
        FUN_004f2360((BYTE *)pMenu, 0);
}

// FUNCTION: CMR2 0x004f3ac0
void FUN_004f3ac0(Menu *pMenu, char param)
{
    if (param == 0)
        pMenu->cursor = 0;
}

// FUNCTION: CMR2 0x004f3b00
void FUN_004f3b00(Menu *pMenu, int param)
{
    g_unk0x00819878 = 0;
    pMenu->cursor = 1;
}

// FUNCTION: CMR2 0x004f3b20
void FUN_004f3b20(Menu *pMenu, int param)
{
    g_unk0x00819878 = 1;
}

// FUNCTION: CMR2 0x004faa00
void FUN_004faa00(Menu *pMenu, int param)
{
    char cursor;

    if ((*CGameInfo::FUN_00405db0() & 2) || CGameInfo::FUN_00406410(0xd))
        pMenu->items[2].enabled = 1;
    else
        pMenu->items[2].enabled = 0;
    cursor = CGameInfo::FUN_00405d90();
    pMenu->cursor = cursor;
    if (cursor >= pMenu->itemCount)
        pMenu->cursor = pMenu->itemCount - 1;
}

// FUNCTION: CMR2 0x004fafd0
void FUN_004fafd0(Menu *pMenu)
{
    RallyData_FUN_0040d600(pMenu->cursor);
}
