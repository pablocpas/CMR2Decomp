#include "Frontend.h"
#include "Game.h"
#include "GameInfo.h"
#include "RallyData.h"
#include "Graphics.h"
#include "Input.h"
#include "Font.h"
#include "GenericFileLoader.h"
#include <string.h>
#include "FrontendMenus.h"
#include "Sound.h"

// Callbacks of the frontend screens, hooked to the menus built in
// FrontendMenus.cpp.

// GLOBAL: CMR2 0x00818f10
char g_unk0x00818f10;
// GLOBAL: CMR2 0x00525398
int g_unk0x00525398 = -1;
// GLOBAL: CMR2 0x0052539c
int g_unk0x0052539c = -1;
// GLOBAL: CMR2 0x0081912c
int g_unk0x0081912c;
// GLOBAL: CMR2 0x00819048
BYTE g_unk0x00819048;
// GLOBAL: CMR2 0x00819130
char g_unk0x00819130[12];
// GLOBAL: CMR2 0x00819140
MenuScroller g_menuScroller0x00819140;
// GLOBAL: CMR2 0x008191b8
MenuScroller g_menuScroller0x008191b8;
// GLOBAL: CMR2 0x00819230
MenuScroller g_menuScroller0x00819230;
// GLOBAL: CMR2 0x008192a8
MenuScroller g_menuScroller0x008192a8;
// GLOBAL: CMR2 0x00819320
MenuScroller g_menuScroller0x00819320;
// GLOBAL: CMR2 0x00819398
MenuScroller g_menuScroller0x00819398;
// GLOBAL: CMR2 0x00819410
MenuScroller g_menuScroller0x00819410;
// GLOBAL: CMR2 0x00819488
MenuScroller g_menuScroller0x00819488;
// GLOBAL: CMR2 0x00819500
MenuScroller g_menuScroller0x00819500;
// GLOBAL: CMR2 0x00819578
MenuScroller g_menuScroller0x00819578;
// GLOBAL: CMR2 0x008195f0
MenuScroller g_menuScroller0x008195f0;
// GLOBAL: CMR2 0x00819668
MenuScroller g_menuScroller0x00819668;
// GLOBAL: CMR2 0x008196e0
unsigned int g_unk0x008196e0;
// GLOBAL: CMR2 0x008196e4
int g_unk0x008196e4;
// GLOBAL: CMR2 0x00819864
BYTE g_unk0x00819864;
// GLOBAL: CMR2 0x00819880
int g_unk0x00819880;
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

// FUNCTION: CMR2 0x004edb70
void FUN_004edb70(Menu *pMenu, int param)
{
    unsigned int *pFlags = CGameInfo::FUN_00405db0();
    int values[2];
    int i;

    FUN_004ea990(&values[0], &values[1]);
    if (CGameInfo::FUN_00406410(0xd))
        Menu_GetItem(pMenu, 1)->min = 8;
    else
        Menu_GetItem(pMenu, 1)->min = *pFlags >> 8 & 0xf;
    Menu_GetItem(pMenu, 1)->max = values[0];
    if (Menu_GetItem(pMenu, 1)->max > Menu_GetItem(pMenu, 1)->min)
        Menu_GetItem(pMenu, 1)->max = 0;
    if (CGameInfo::FUN_00406410(0xd)) {
        Menu_GetItem(pMenu, 0)->min = values[0] % 2 + 10;
        return;
    }
    for (i = 0; i < 11; i++) {
        if (CGameInfo::FUN_00406520(values[0], i) & 2) {
            Menu_GetItem(pMenu, 0)->min = i;
            break;
        }
    }
    if (values[0] % 2 != 0 && (i == 4 || i == 8) && !(CGameInfo::FUN_00406520(values[0], 10) & 2))
        Menu_GetItem(pMenu, 0)->min++;
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

// Scroll of a 13-row list: g_unk0x0082a924 is the selected row,
// g_unk0x0082aa3c the first visible one.
// FUNCTION: CMR2 0x004ef4e0
void FUN_004ef4e0(Menu *pMenu)
{
    DeviceInfo *pDevice;

    g_unk0x0082aa40 = FUN_004f4db0();
    Menu_SetFlags(pMenu, 0, 0, 1, 1);
    pDevice = CInput::FUN_0049ead0(0);
    if (g_unk0x0082aa40 > 0) {
        if (g_unk0x0082a924 == -1)
            g_unk0x0082a924 = 0;
        else if ((pDevice->field_0x8 & 4) && g_unk0x0082a924 > 0)
            g_unk0x0082a924--;
        else if ((pDevice->field_0x8 & 8) && g_unk0x0082a924 < g_unk0x0082aa40 - 1)
            g_unk0x0082a924++;
    } else {
        g_unk0x0082a924 = -1;
        g_unk0x0082aa3c = 0;
    }
    if (g_unk0x0082a924 < g_unk0x0082aa3c) {
        g_unk0x0082aa3c--;
        if (g_unk0x0082aa3c < 0)
            g_unk0x0082aa3c = 0;
    }
    if (g_unk0x0082a924 >= g_unk0x0082aa3c + 13)
        g_unk0x0082aa3c++;
    g_unk0x0082ac48 = 13;
    if (g_unk0x0082aa40 <= 13)
        g_unk0x0082ac48 = g_unk0x0082aa40;
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

// FUNCTION: CMR2 0x004efb50
void FUN_004efb50(Menu *pMenu)
{
    RallyData_FUN_004068b0(pMenu->cursor);
    FUN_004f37c0(FUN_004f2530());
}

// FUNCTION: CMR2 0x004efdc0
void FUN_004efdc0(Menu *pMenu)
{
    FUN_004f37c0(FUN_004f2570());
}

// FUNCTION: CMR2 0x004efdd0
void FUN_004efdd0(Menu *pMenu)
{
    FUN_004f37c0(FUN_004f2580());
}

// FUNCTION: CMR2 0x004f0050
void FUN_004f0050(Menu *pMenu, MenuItem *pItem)
{
    if (pItem->value == -1) {
        RallyData_FUN_0040df60(0, 0);
        Menu_SetNextAction((int)FUN_004f8330());
    } else {
        RallyData_FUN_0040df60(1, pItem->value);
        Menu_SetNextAction((int)FUN_004f8330());
    }
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

// FUNCTION: CMR2 0x004f1ba0
BYTE FUN_004f1ba0(void)
{
    return g_unk0x00819048;
}

// FUNCTION: CMR2 0x004f1bd0
void FUN_004f1bd0(Menu *pMenu, int param)
{
    g_unk0x00819864 = 1;
    g_unk0x0081987c = 0;
    g_unk0x008196e4 = CFrontend::FUN_004d20e0();
}

// FUNCTION: CMR2 0x004f1c00
void FUN_004f1c00(Menu *pMenu, int param)
{
    if (CFrontend::FUN_004b7520()) {
        if ((BYTE)CGameInfo::FUN_00405ba0() && (g_pGraphics->field913_0x3bc & 0x40))
            Menu_GetItem(pMenu, 1)->max = 0;
        else
            Menu_GetItem(pMenu, 1)->max = 1;
        Menu_GetItem(pMenu, 1)->enabled = 1;
    } else {
        Menu_GetItem(pMenu, 1)->enabled = 0;
    }
    if (CGameInfo::FUN_00405c70() == 1 && (g_pGraphics->field913_0x3bc & 1))
        Menu_GetItem(pMenu, 4)->max = 1;
    else if (CGameInfo::FUN_00405c70() == 2 && (g_pGraphics->field913_0x3bc & 2))
        Menu_GetItem(pMenu, 4)->max = 0;
    else
        Menu_GetItem(pMenu, 4)->max = 2;
    CGameInfo::FUN_00405ca0();
    if ((unsigned int)CGameInfo::FUN_00405ca0() > 9)
        CGameInfo::FUN_00405cb0(9);
    Menu_GetItem(pMenu, 5)->max = CGameInfo::FUN_00405ca0();
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

// FUNCTION: CMR2 0x004f1e40
void FUN_004f1e40(Menu *pMenu, int param)
{
    if (CFrontend::FUN_004b7520()) {
        if (Menu_GetItem(pMenu, 1)->max == 0) {
            CGameInfo::FUN_00405bb0(1);
            g_pGraphics->field913_0x3bc |= 0x40;
        } else {
            CGameInfo::FUN_00405bb0(0);
            g_pGraphics->field913_0x3bc &= ~0x40;
        }
    }
    switch (Menu_GetItem(pMenu, 4)->max) {
    case 0:
        CGameInfo::FUN_00405c80(2);
        g_pGraphics->field913_0x3bc &= ~1;
        g_pGraphics->field913_0x3bc |= 2;
        break;
    case 1:
        CGameInfo::FUN_00405c80(1);
        g_pGraphics->field913_0x3bc |= 1;
        g_pGraphics->field913_0x3bc &= ~2;
        break;
    case 2:
        CGameInfo::FUN_00405c80(0);
        g_pGraphics->field913_0x3bc &= ~1;
        g_pGraphics->field913_0x3bc &= ~2;
        break;
    }
    CGameInfo::FUN_00405cb0(Menu_GetItem(pMenu, 5)->max);
    g_pGraphics->field917_0x3c0 = Menu_GetItem(pMenu, 5)->max;
}

// FUNCTION: CMR2 0x004f1f70
void FUN_004f1f70(Menu *pMenu, char param)
{
    if (param == 0)
        FUN_004f1e40(pMenu, (int)&pMenu->items[pMenu->cursor]);
}

// FUNCTION: CMR2 0x004f2050
void FUN_004f2050(Menu *pMenu, int param)
{
    switch (pMenu->cursor) {
    case 0:
        CGameInfo::FUN_00405b60(0);
        g_pGraphics->field913_0x3bc &= ~8;
        g_pGraphics->field913_0x3bc &= ~0x10;
        g_pGraphics->field913_0x3bc &= ~0x80;
        break;
    case 1:
        CGameInfo::FUN_00405b60(1);
        g_pGraphics->field913_0x3bc |= 8;
        g_pGraphics->field913_0x3bc &= ~0x10;
        g_pGraphics->field913_0x3bc &= ~0x80;
        break;
    case 2:
        CGameInfo::FUN_00405b60(2);
        g_pGraphics->field913_0x3bc |= 8;
        g_pGraphics->field913_0x3bc |= 0x10;
        g_pGraphics->field913_0x3bc &= ~0x80;
        break;
    case 3:
        CGameInfo::FUN_00405b60(3);
        g_pGraphics->field913_0x3bc |= 8;
        g_pGraphics->field913_0x3bc &= ~0x10;
        g_pGraphics->field913_0x3bc |= 0x80;
        break;
    }
    Menu_SetNextAction((int)pMenu->pParent);
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

// FUNCTION: CMR2 0x004f24f0
MenuScroller *FUN_004f24f0(void)
{
    return &g_menuScroller0x00819140;
}

// FUNCTION: CMR2 0x004f2500
MenuScroller *FUN_004f2500(void)
{
    return &g_menuScroller0x00819230;
}

// FUNCTION: CMR2 0x004f2510
MenuScroller *FUN_004f2510(void)
{
    return &g_menuScroller0x008192a8;
}

// FUNCTION: CMR2 0x004f2520
MenuScroller *FUN_004f2520(void)
{
    return &g_menuScroller0x00819320;
}

// FUNCTION: CMR2 0x004f2530
MenuScroller *FUN_004f2530(void)
{
    return &g_menuScroller0x00819398;
}

// FUNCTION: CMR2 0x004f2540
MenuScroller *FUN_004f2540(void)
{
    return &g_menuScroller0x00819410;
}

// FUNCTION: CMR2 0x004f2550
MenuScroller *FUN_004f2550(void)
{
    return &g_menuScroller0x00819488;
}

// FUNCTION: CMR2 0x004f2560
MenuScroller *FUN_004f2560(void)
{
    return &g_menuScroller0x00819500;
}

// FUNCTION: CMR2 0x004f2570
MenuScroller *FUN_004f2570(void)
{
    return &g_menuScroller0x00819578;
}

// FUNCTION: CMR2 0x004f2580
MenuScroller *FUN_004f2580(void)
{
    return &g_menuScroller0x00819668;
}

// FUNCTION: CMR2 0x004f2590
MenuScroller *FUN_004f2590(void)
{
    return &g_menuScroller0x008195f0;
}

// FUNCTION: CMR2 0x004f25a0
int FUN_004f25a0(void)
{
    return g_unk0x0081987c;
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

// FUNCTION: CMR2 0x004f2b70
void FUN_004f2b70(Menu *pMenu, char param)
{
    if (param != 0) {
        CGameInfo::FUN_00405e10(g_unk0x0081903c);
        CGameInfo::FUN_00405e50(g_unk0x00819860);
        CGameInfo::FUN_00405e80(g_unk0x00819754);
        CInput::FUN_0049ffc0((int)(CGameInfo::FUN_00405e70() << 16) / 100 / 4);
        CSound::FUN_004a28c0();
        return;
    }
    FUN_004f2b00((BYTE *)pMenu);
}

// FUNCTION: CMR2 0x004f2be0
int FUN_004f2be0(void)
{
    return g_unk0x00819880;
}

// FUNCTION: CMR2 0x004f2c40
void FUN_004f2c40(Menu *pMenu, char param)
{
    if (param == 0) {
        if (CGameInfo::FUN_00405dc0())
            pMenu->items[Menu_FindItem(pMenu, 0)].max = 1;
        else
            pMenu->items[Menu_FindItem(pMenu, 0)].max = 0;
        if ((BYTE)CGameInfo::FUN_00406310())
            pMenu->items[Menu_FindItem(pMenu, 2)].max = 0;
        else
            pMenu->items[Menu_FindItem(pMenu, 2)].max = 1;
        if ((BYTE)CGameInfo::FUN_00405eb0())
            pMenu->items[Menu_FindItem(pMenu, 3)].max = 1;
        else
            pMenu->items[Menu_FindItem(pMenu, 3)].max = 0;
        if (CGameInfo::FUN_00405ef0() == 0)
            pMenu->items[Menu_FindItem(pMenu, 4)].max = 0;
        if (CGameInfo::FUN_00405ef0() == 1)
            pMenu->items[Menu_FindItem(pMenu, 4)].max = 1;
        if (CGameInfo::FUN_00405ef0() == 2)
            pMenu->items[Menu_FindItem(pMenu, 4)].max = 2;
    }
}

// FUNCTION: CMR2 0x004f2d90
void FUN_004f2d90(Menu *pMenu, int param)
{
    MenuScroller *p;
    int i;

    p = FUN_004f2540();
    p->startTime = CFrontend::FUN_004d20e0();
    p->pMenu = pMenu;
    p = FUN_004f2540();
    p->spacing = (int)(g_pGraphics->resX * 24) / 640;
    p->count = pMenu->itemCount;
    p->offset = 0;
    p->startOffset = 0;
    for (i = 0; i < pMenu->itemCount; i++) {
        strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(pMenu->items[i].id));
        CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
        p->widths[i] = Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest);
    }
}

// FUNCTION: CMR2 0x004f2e70
void FUN_004f2e70(Menu *pMenu, int param)
{
    MenuScroller *p;
    int i;

    p = FUN_004f2550();
    p->startTime = CFrontend::FUN_004d20e0();
    p->pMenu = pMenu;
    p->spacing = (int)(g_pGraphics->resX * 24) / 640;
    p->count = pMenu->itemCount;
    p->offset = 0;
    p->startOffset = 0;
    for (i = 0; i < pMenu->itemCount; i++) {
        strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(pMenu->items[i].id));
        CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
        p->widths[i] = Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest);
    }
}

// FUNCTION: CMR2 0x004f2f40
void FUN_004f2f40(Menu *pMenu, int param)
{
    MenuScroller *p;
    int i;

    p = FUN_004f2560();
    p->startTime = CFrontend::FUN_004d20e0();
    p->pMenu = pMenu;
    p->spacing = (int)(g_pGraphics->resX * 24) / 640;
    p->count = pMenu->itemCount;
    p->offset = 0;
    p->startOffset = 0;
    for (i = 0; i < pMenu->itemCount; i++) {
        strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(pMenu->items[i].id));
        CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
        p->widths[i] = Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest);
    }
}

// FUNCTION: CMR2 0x004f3010
void FUN_004f3010(Menu *pMenu, int param)
{
    MenuScroller *p;
    int i;

    for (i = 0; i < 12; i++) {
        if (g_unk0x00819130[i] == 1)
            pMenu->items[i].enabled = 1;
        else
            pMenu->items[i].enabled = 0;
    }
    p = FUN_004f2570();
    p->startTime = CFrontend::FUN_004d20e0();
    p->count = pMenu->itemCount;
    p->pMenu = pMenu;
    p->offset = 0;
    p->startOffset = 0;
    p->current = pMenu->cursor;
    p->previous = pMenu->cursor;
    p->spacing = (int)(g_pGraphics->resX * 24) / 640;
    p->count = pMenu->itemCount;
    p->offset = 0;
    p->startOffset = 0;
    for (i = 0; i < pMenu->itemCount; i++) {
        strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(pMenu->items[i].id));
        CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
        p->widths[i] = Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest);
    }
}

// FUNCTION: CMR2 0x004f3120
void FUN_004f3120(Menu *pMenu, int param)
{
    MenuScroller *p;
    int i;

    for (i = 0; i < pMenu->itemCount; i++)
        pMenu->items[i].enabled = 1;
    p = FUN_004f2580();
    p->startTime = CFrontend::FUN_004d20e0();
    p->count = pMenu->itemCount;
    p->pMenu = pMenu;
    p->spacing = (int)(g_pGraphics->resX * 24) / 640;
    p->count = pMenu->itemCount;
    p->offset = 0;
    p->startOffset = 0;
    for (i = 0; i < pMenu->itemCount; i++) {
        strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(pMenu->items[i].id));
        CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
        p->widths[i] = Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest);
    }
}

// FUNCTION: CMR2 0x004f36e0
void FUN_004f36e0(Menu *pMenu, int param)
{
    MenuScroller *p;
    int i;

    p = FUN_004f2530();
    p->startTime = CFrontend::FUN_004d20e0();
    p->pMenu = pMenu;
    p->count = pMenu->itemCount;
    p->offset = 0;
    p->startOffset = 0;
    p->current = pMenu->cursor;
    p->previous = pMenu->cursor;
    p->spacing = (int)(g_pGraphics->resX * 24) / 640;
    p->count = pMenu->itemCount;
    p->offset = 0;
    p->startOffset = 0;
    for (i = 0; i < pMenu->itemCount; i++) {
        strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(i + 0x27));
        CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
        p->widths[i] = Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest);
    }
}

// Slides the scroller towards the menu cursor, the short way round, in 250 ms
// FUNCTION: CMR2 0x004f37c0
void FUN_004f37c0(MenuScroller *p)
{
    Menu *pMenu = p->pMenu;
    int target;
    int cur;
    int fwd;
    int back;
    int idx;
    int i;

    if (pMenu->cursor != p->current) {
        if (p->current != -1) {
            if (pMenu == FUN_004f8410()) {
                g_unk0x0081912c = CFrontend::FUN_004d20e0();
                g_unk0x00525398 = g_unk0x0052539c;
                g_unk0x0052539c = pMenu->cursor;
            }
            target = pMenu->cursor;
            cur = p->current;
            if (target > cur) {
                back = pMenu->itemCount - target + cur;
                fwd = target - cur;
            } else {
                fwd = pMenu->itemCount - cur + target;
                back = cur - target;
            }
            if (back < fwd) {
                while (back-- > 0) {
                    cur--;
                    idx = cur;
                    if (idx < 0)
                        idx = p->count + cur;
                    p->offset -= p->widths[idx] + p->spacing;
                }
            } else if (back > fwd) {
                while (fwd-- > 0) {
                    idx = cur;
                    if (idx >= p->count)
                        idx = cur - p->count;
                    p->offset += p->widths[idx] + p->spacing;
                    cur++;
                }
            } else if (pMenu->moveFlags & 1) {
                cur--;
                if (cur < 0)
                    cur += p->count;
                p->offset -= p->widths[cur] + p->spacing;
            } else {
                if (cur >= p->count)
                    cur -= p->count;
                p->offset += p->widths[cur] + p->spacing;
            }
            p->startOffset = p->offset;
        }
        p->startTime = CFrontend::FUN_004d20e0();
        p->previous = p->current;
        p->current = pMenu->cursor;
    }
    if (p->current != -1) {
        if ((unsigned int)(CFrontend::FUN_004d20e0() - p->startTime) > 250) {
            p->offset = 0;
            p->startOffset = 0;
            return;
        }
        if (p->startOffset > 0) {
            p->offset = p->startOffset - (unsigned int)((CFrontend::FUN_004d20e0() - p->startTime) * p->startOffset) / 250;
            return;
        }
        p->offset = (unsigned int)-((CFrontend::FUN_004d20e0() - p->startTime) * p->startOffset) / 250 + p->startOffset;
    }
}

// FUNCTION: CMR2 0x004f3970
void FUN_004f3970(Menu *pMenu)
{
    FUN_004f37c0(FUN_004f2540());
}

// FUNCTION: CMR2 0x004f3980
void FUN_004f3980(Menu *pMenu)
{
    FUN_004f37c0(FUN_004f2550());
}

// FUNCTION: CMR2 0x004f3990
void FUN_004f3990(Menu *pMenu)
{
    if (FUN_004f3a60() == 0) {
        pMenu->cursor = 0;
        pMenu->items[1].enabled = 0;
    } else {
        pMenu->items[1].enabled = 1;
    }
    FUN_004f37c0(FUN_004f2560());
}

// FUNCTION: CMR2 0x004f39d0
void FUN_004f39d0(Menu *pMenu)
{
    FUN_004f37c0(FUN_004f2500());
}

// FUNCTION: CMR2 0x004f3a60
int FUN_004f3a60(void)
{
    return g_unk0x008196e0;
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

// FUNCTION: CMR2 0x004f3ae0
void FUN_004f3ae0(Menu *pMenu)
{
    FUN_004f37c0(FUN_004f24f0());
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

// FUNCTION: CMR2 0x004faef0
void FUN_004faef0(Menu *pMenu, int param)
{
    unsigned int *pFlags = CGameInfo::FUN_00405db0();
    unsigned int count;
    unsigned int i;
    char cursor;

    switch (CGameInfo::FUN_00405d90()) {
    case 2:
        if (CGameInfo::FUN_00406410(0xd))
            count = 2;
        else
            count = *pFlags >> 6 & 3;
        break;
    case 1:
        if (CGameInfo::FUN_00406410(0xd))
            count = 2;
        else
            count = *pFlags >> 4 & 3;
        break;
    case 0:
        count = *pFlags >> 2 & 3;
        break;
    }
    for (i = 0; i < 2; i++) {
        if (i < count)
            FUN_004fa330()->items[i].enabled = 1;
        else
            FUN_004fa330()->items[i].enabled = 0;
    }
    cursor = RallyData_FUN_00406940();
    FUN_004fa330()->cursor = cursor;
    if (FUN_004fa330()->cursor >= FUN_004fa330()->itemCount)
        FUN_004fa330()->cursor = FUN_004fa330()->itemCount - 1;
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

// FUNCTION: CMR2 0x004fb360
void FUN_004fb360(Menu *pMenu)
{
    FUN_004f37c0(FUN_004f2590());
}
