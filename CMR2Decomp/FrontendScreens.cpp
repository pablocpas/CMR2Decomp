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
// GLOBAL: CMR2 0x008196e0
unsigned int g_unk0x008196e0;
// GLOBAL: CMR2 0x008196e4
int g_unk0x008196e4;
// GLOBAL: CMR2 0x00819864
BYTE g_unk0x00819864;
// GLOBAL: CMR2 0x0081987c
int g_unk0x0081987c;
// GLOBAL: CMR2 0x0082a924
int g_unk0x0082a924;
// GLOBAL: CMR2 0x0082aa3c
int g_unk0x0082aa3c;
// GLOBAL: CMR2 0x0082aa40
int g_unk0x0082aa40;
// GLOBAL: CMR2 0x0082ac48
int g_unk0x0082ac48;
// GLOBAL: CMR2 0x00819878
BYTE g_unk0x00819878;
// GLOBAL: CMR2 0x0081903c
unsigned int g_unk0x0081903c;
// GLOBAL: CMR2 0x00819754
unsigned int g_unk0x00819754;
// GLOBAL: CMR2 0x00819860
unsigned int g_unk0x00819860;

// FUNCTION: CMR2 0x004ecfa0
void FUN_004ecfa0(Menu *pMenu, char param)
{
    if (param != 0 && g_unk0x00818f10 != 0) {
        CGame::DestroyDirectPlayLobby();
        CGame::DestroyDirectPlay();
        g_unk0x00818f10 = 0;
    }
}

// FUNCTION: CMR2 0x004ed500
void FUN_004ed500(Menu *pMenu, char param)
{
    if (param != 0) {
        CGame::FUN_004a1a90();
        FUN_004a1280();
    } else {
        FUN_004ea8e0(Menu_GetItem(pMenu, 1)->max + 8);
    }
}

// FUNCTION: CMR2 0x004edb30
void FUN_004edb30(Menu *pMenu, char param)
{
    if (param != 0 && !FUN_004a15a0()) {
        CGame::FUN_004a1a90();
        FUN_004a1280();
    }
}

// FUNCTION: CMR2 0x004edca0
void FUN_004edca0(Menu *pMenu, int param)
{
    int value = Menu_GetItem(pMenu, 0)->max;
    BYTE flags;

    if ((Menu_GetItem(pMenu, 0)->min == 5 && Menu_GetItem(pMenu, 0)->max == 4) ||
        (Menu_GetItem(pMenu, 0)->min == 9 && Menu_GetItem(pMenu, 0)->max == 8))
        value = 10;
    flags = CGameInfo::FUN_00406520(Menu_GetItem(pMenu, 1)->max, value);
    if (flags & 1) {
        CGameInfo::FUN_00406540(Menu_GetItem(pMenu, 1)->max, value, flags & ~1);
        return;
    }
    CGameInfo::FUN_00406540(Menu_GetItem(pMenu, 1)->max, value, flags | 1);
}

// FUNCTION: CMR2 0x004ef4c0
void FUN_004ef4c0(Menu *pMenu, int param)
{
    g_unk0x0082aa40 = FUN_004f4db0();
    g_unk0x0082a924 = 0;
    g_unk0x0082aa3c = 0;
    g_unk0x0082ac48 = 0;
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

// FUNCTION: CMR2 0x004ef960
void FUN_004ef960(Menu *pMenu, int param)
{
    FUN_004ea970(pMenu->cursor);
}

// FUNCTION: CMR2 0x004f0600
void FUN_004f0600(Menu *pMenu, int param)
{
    FUN_004ea480(0);
    FUN_004a0c40(1);
}

// FUNCTION: CMR2 0x004f0e60
void FUN_004f0e60(Menu *pMenu, int param)
{
    FUN_004ea480(0);
    FUN_004a0c50(0);
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

// FUNCTION: CMR2 0x004f1bd0
void FUN_004f1bd0(Menu *pMenu, int param)
{
    g_unk0x00819864 = 1;
    g_unk0x0081987c = 0;
    g_unk0x008196e4 = CFrontend::FUN_004d20e0();
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

// FUNCTION: CMR2 0x004f1db0
void FUN_004f1db0(Menu *pMenu, int param)
{
    CGameInfo::FUN_00405ce0(Menu_GetItem(pMenu, 8)->max);
    switch (Menu_GetItem(pMenu, 9)->max) {
    case 1:
        CGameInfo::FUN_00405d20(1);
        break;
    case 0:
        CGameInfo::FUN_00405d20(0);
        break;
    }
    switch (Menu_GetItem(pMenu, 10)->max) {
    case 1:
        CGameInfo::FUN_00405d40(2);
        break;
    case 0:
        if (CFrontend::FUN_004b7560(0x400) && CFrontend::FUN_004b7590(0x400))
            CGameInfo::FUN_00405d40(0);
        else
            CGameInfo::FUN_00405d40(2);
        break;
    }
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

// FUNCTION: CMR2 0x004f2840
void FUN_004f2840(Menu *pMenu, int param)
{
    g_unk0x0081903c = CGameInfo::FUN_00405e40();
    g_unk0x00819860 = CGameInfo::FUN_00405e70();
    g_unk0x00819754 = CGameInfo::FUN_00405ea0();
    pMenu->items[0].max = (int)CGameInfo::FUN_00405e40() / 10;
    pMenu->items[1].max = (int)CGameInfo::FUN_00405e70() / 10;
    pMenu->items[2].max = (int)CGameInfo::FUN_00405ea0() / 10;
}

// FUNCTION: CMR2 0x004f3a90
void FUN_004f3a90(Menu *pMenu, int param)
{
    g_unk0x008196e0++;
    if (g_unk0x008196e0 > 2)
        g_unk0x008196e0 = 0;
    Menu_PlaySoundId(4);
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

// FUNCTION: CMR2 0x004f3b30
void FUN_004f3b30(Menu *pMenu, char param)
{
    if (g_unk0x00819878 == 0 && param == 0)
        Menu_SetNextAction((int)FUN_004f8990());
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

// FUNCTION: CMR2 0x004fafe0
void FUN_004fafe0(Menu *pMenu, int param)
{
    RallyData_FUN_0040d600(pMenu->cursor);
    RallyData_FUN_00406960(0);
    RallyData_FUN_0040d620(3);
    Menu_SetNextAction((int)FUN_004f8330());
}
