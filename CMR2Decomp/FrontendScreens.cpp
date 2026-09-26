#include "Frontend.h"
#include "Game.h"
#include "GameInfo.h"
#include "RallyData.h"
#include "Graphics.h"
#include "Input.h"
#include "Font.h"
#include "GenericFileLoader.h"
#include <string.h>
#include <stdio.h>
#include "FrontendMenus.h"
#include "InstallInfo.h"
#include "Sprite.h"
#include "RegKey.h"
#include "FixedPoint.h"
#include <stdlib.h>
#include "FrontendDraw.h"
#include "Sound.h"
#include "main.h"
#include "NetworkLeaderboards.h"

#define PATH_X() ((int)(g_pGraphics->resX * 24) / 640)
#define PATH_Y() ((int)(g_pGraphics->resY * 38) / 480)

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
// The 12 frontend scrollers are one contiguous array in the original (see
// FUN_004ef150); the old per-address names are views (FrontendMenus.h).
// GLOBAL: CMR2 0x00819140
MenuScroller g_menuScrollers[12];
// GLOBAL: CMR2 0x008196e0
unsigned int g_unk0x008196e0;
// GLOBAL: CMR2 0x008196e4
int g_unk0x008196e4;
// GLOBAL: CMR2 0x00819864
BYTE g_unk0x00819864;
// GLOBAL: CMR2 0x00819880
int g_unk0x00819880;
// Sample index of the first frontend sound (move, select, back, error, toggle)
// GLOBAL: CMR2 0x00819888
int g_menuSoundBase;
// GLOBAL: CMR2 0x0081988c
int g_unk0x0081988c;
// The four "dot" textures of the frontend (dot00..dot03)
// GLOBAL: CMR2 0x00819e94
Texture *g_menuDotTextures[4];
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
// GLOBAL: CMR2 0x00525288
int g_unk0x00525288 = -1;
// GLOBAL: CMR2 0x0052528c
int g_unk0x0052528c = -1;
// GLOBAL: CMR2 0x00525330
char g_nameChars0x00525330[68] = "abcdefghijklmnopqrstuvwxyz. ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
// GLOBAL: CMR2 0x00818cd8
char *g_unk0x00818cd8;
// GLOBAL: CMR2 0x00818d04
BYTE g_unk0x00818d04;
// GLOBAL: CMR2 0x00818da8
char g_unk0x00818da8[256];
// GLOBAL: CMR2 0x00818ed4
int g_unk0x00818ed4[8];
// GLOBAL: CMR2 0x00818ef4
BYTE g_unk0x00818ef4;
// GLOBAL: CMR2 0x00819014
BYTE g_unk0x00819014;
// GLOBAL: CMR2 0x0081901c
int g_unk0x0081901c;
// GLOBAL: CMR2 0x00819020
int g_unk0x00819020;
// GLOBAL: CMR2 0x00819024
BYTE g_unk0x00819024;
// GLOBAL: CMR2 0x00819038
BYTE g_unk0x00819038;
// GLOBAL: CMR2 0x00819040
unsigned int g_unk0x00819040;
// GLOBAL: CMR2 0x0081986c
BYTE g_unk0x0081986c;
// GLOBAL: CMR2 0x008182b4
int g_unk0x008182b4;
// GLOBAL: CMR2 0x008189a4
BYTE g_unk0x008189a4;
// GLOBAL: CMR2 0x00818abc
Menu *g_pMenu0x00818abc;
// GLOBAL: CMR2 0x00818ac0
Menu *g_pMenu0x00818ac0;
// GLOBAL: CMR2 0x00818ac4
BYTE g_unk0x00818ac4;
// GLOBAL: CMR2 0x00818f14
BYTE g_unk0x00818f14[0xe0];
// GLOBAL: CMR2 0x00819030
int g_unk0x00819030;
// GLOBAL: CMR2 0x00819050
int g_unk0x00819050;
// GLOBAL: CMR2 0x008190f4
BYTE g_unk0x008190f4[0x30];
// GLOBAL: CMR2 0x00819124
int g_unk0x00819124;
// GLOBAL: CMR2 0x00819748
BYTE g_unk0x00819748;
// GLOBAL: CMR2 0x00819870
int g_unk0x00819870;
// GLOBAL: CMR2 0x00826138
int g_unk0x00826138;
// GLOBAL: CMR2 0x00819878
BYTE g_unk0x00819878;
// GLOBAL: CMR2 0x0081903c
unsigned int g_unk0x0081903c;
// GLOBAL: CMR2 0x00819754
unsigned int g_unk0x00819754;
// GLOBAL: CMR2 0x00819860
unsigned int g_unk0x00819860;

// FUNCTION: CMR2 0x004d27c0
void FUN_004d27c0(BYTE value)
{
    g_unk0x00818848 = value;
}

// FUNCTION: CMR2 0x004d4c40
void FUN_004d4c40(Menu *pMenu)
{
    char *text[3];

    FrontendDraw_PlayTime();
    text[0] = CFrontend::GetTextString(0xb);
    text[1] = CFrontend::GetTextString(pMenu->field_0x4);
    text[2] = (char *)RallyData_GetRecord(0);
    FrontendDraw_Breadcrumb(PATH_X(), PATH_Y(), text, 3);
    FrontendDraw_MenuList(pMenu, (char *)RallyData_GetRecord(0), -1, -1, 0, 1);
    FrontendDraw_Carousel(FUN_004f8410(), 0, NULL);
}

// GLOBAL: CMR2 0x00524c88
char g_strDmdFormat[12] = "%s%.2d.dmd";
// GLOBAL: CMR2 0x00524c94
char g_strChaDmdFormat[12] = "cha%.2d.dmd";
// GLOBAL: CMR2 0x00524ca0
char g_strDmdBfl[36] = "%s\\frontend\\Textures\\dmd\\dmd.bfl";

// Stage map files (.dmd) of the 8 rallies (10 or 11 stages each) plus the
// 8 of the championship ("cha").
// GLOBAL: CMR2 0x008185bc
void *g_dmdFiles[9][11];

char FUN_004eaa30(void);

// Finds the stage map files of every rally in the common frontend archive.
// TODO: CMR2 0x004d5ca0 (implemented, match 85%)
void FUN_004d5ca0(void)
{
    int counts[9] = { 10, 11, 10, 11, 10, 11, 10, 11, 8 };
    char names[8][4] = { "fin", "gre", "fra", "swe", "aus", "ken", "ita", "uk" };
    void **ppFile;
    int count;
    int i;
    int j;

    // the original builds this path and never uses it
    sprintf(CFrontend::m_stringDest, g_strDmdBfl, CInstallInfo::GetGameCDPath());
    for (i = 0; i < 9; i++) {
        count = counts[i];
        if (count > 0) {
            ppFile = g_dmdFiles[i];
            j = 1;
            do {
                *ppFile = NULL;
                if (i == 8)
                    sprintf(CFrontend::m_stringDest, g_strChaDmdFormat, j);
                else
                    sprintf(CFrontend::m_stringDest, g_strDmdFormat, names[i], j);
                *ppFile = CGenericFileLoader::FindFile(CFrontend::FUN_004d2190(), CFrontend::m_stringDest, NULL, NULL, 0);
                ppFile++;
            } while (j++ < count);
        }
    }
    CGame::RegisterCallback((void *)FUN_004eaa30, NULL);
}

// FUNCTION: CMR2 0x004d6290
void FUN_004d6290(Menu *pMenu)
{
    char *text[2];

    FrontendDraw_PlayTime();
    switch (CGameInfo::FUN_00405d80()) {
    case 0:
        text[0] = CFrontend::GetTextString(0xe7);
        text[1] = CFrontend::GetTextString(0xc);
        break;
    case 1:
        text[0] = CFrontend::GetTextString(0xe7);
        text[1] = CFrontend::GetTextString(0xd);
        break;
    case 2:
        text[0] = CFrontend::GetTextString(0xe7);
        text[1] = CFrontend::GetTextString(0xf);
        break;
    case 3:
        text[0] = CFrontend::GetTextString(0xe7);
        text[1] = CFrontend::GetTextString(0x10);
        break;
    case 4:
        text[0] = CFrontend::GetTextString(0xe7);
        text[1] = CFrontend::GetTextString(0x11);
        break;
    case 5:
        text[0] = CFrontend::GetTextString(0x94);
        text[1] = CFrontend::GetTextString(0xc);
        break;
    case 6:
        text[0] = CFrontend::GetTextString(0x94);
        text[1] = CFrontend::GetTextString(0xe2);
        break;
    case 7:
        text[0] = CFrontend::GetTextString(0x94);
        text[1] = CFrontend::GetTextString(0x10);
        break;
    }
    FrontendDraw_MenuPath(pMenu, PATH_X(), PATH_Y(), 1, 3, text, 2);
    FrontendDraw_MenuList(pMenu, NULL, -1, -1, 0, 1);
    FrontendDraw_Carousel(FUN_004f8410(), 0, NULL);
}

// FUNCTION: CMR2 0x004d63e0
void FUN_004d63e0(Menu *pMenu)
{
    FrontendDraw_PlayTime();
    FrontendDraw_MenuPath(pMenu, PATH_X(), PATH_Y(), 1, -1, NULL, -1);
    FrontendDraw_MenuList(pMenu, NULL, -1, -1, 0, 1);
    FrontendDraw_Carousel(FUN_004f8410(), 0, NULL);
}

// FUNCTION: CMR2 0x004da630
void FUN_004da630(Menu *pMenu)
{
    char *text[2];

    switch (CGameInfo::FUN_00405d80()) {
    case 1:
        text[0] = CFrontend::GetTextString(0xe7);
        text[1] = CFrontend::GetTextString(0xd);
        break;
    case 2:
        text[0] = CFrontend::GetTextString(0xe7);
        text[1] = CFrontend::GetTextString(0xf);
        break;
    case 3:
        text[0] = CFrontend::GetTextString(0xe7);
        text[1] = CFrontend::GetTextString(0x10);
        break;
    }
    FrontendDraw_PlayTime();
    FrontendDraw_MenuPath(pMenu, PATH_X(), PATH_Y(), 1, 3, text, 2);
    FrontendDraw_MenuList(FUN_004f8360(), NULL, -1, -1, 0, 1);
    FrontendDraw_HelpText(CFrontend::GetTextString(0x57), 1);
}

// Network connection list.
// FUNCTION: CMR2 0x004dc7b0
void FUN_004dc7b0(Menu *pMenu)
{
    char *text[2];
    int y;
    unsigned int i;
    BYTE *pColour;

    FrontendDraw_PlayTime();
    text[0] = CFrontend::GetTextString(0x12);
    text[1] = CFrontend::GetTextString(0x3c);
    FrontendDraw_Breadcrumb(PATH_X(), PATH_Y(), text, 2);
    FrontendDraw_Carousel(FUN_004f8410(), 0, NULL);
    y = ((int)(g_pGraphics->resY * 8) / 480 + (int)(g_pGraphics->resY * 38) / 480 + (int)(g_pGraphics->resY * 384) / 480) / 2 -
        (int)((int)(g_pGraphics->resY * 20) / 480 * pMenu->items[0].min) / 2;
    for (i = 0; (int)i < pMenu->items[0].min; i++) {
        pColour = g_colourWhite0x00524968;
        if (i != pMenu->items[0].max)
            pColour = g_colourText0x0052496c;
        Font_DrawText(1, CGame::GetConnection(i)->name, (int)g_pGraphics->resX / 2, y, (int *)pColour, 0x12);
        y += (int)(g_pGraphics->resY * 20) / 480;
    }
}

// FUNCTION: CMR2 0x004e1890
void FUN_004e1890(Menu *pMenu)
{
    char *text[1];

    text[0] = CFrontend::GetTextString(0x8e);
    FrontendDraw_PlayTime();
    FrontendDraw_MenuPath(pMenu, PATH_X(), PATH_Y(), 1, 2, text, 1);
    FrontendDraw_MenuList(pMenu, NULL, -1, -1, 0, 1);
    FrontendDraw_Carousel(FUN_004f8410(), 0, NULL);
}

// FUNCTION: CMR2 0x004e1fb0
void FUN_004e1fb0(Menu *pMenu)
{
    char *text[1];

    text[0] = CFrontend::GetTextString(0xd9);
    FrontendDraw_PlayTime();
    FrontendDraw_MenuPath(pMenu, PATH_X(), PATH_Y(), 1, 2, text, 1);
    FrontendDraw_MenuList(pMenu, NULL, -1, -1, 0, 1);
    FrontendDraw_Carousel(FUN_004f8410(), 0, NULL);
}

// FUNCTION: CMR2 0x004e2040
void FUN_004e2040(Menu *pMenu)
{
    char *text[2];

    text[0] = CFrontend::GetTextString(0xe7);
    text[1] = CFrontend::GetTextString(0xc);
    FrontendDraw_PlayTime();
    FrontendDraw_MenuPath(pMenu, PATH_X(), PATH_Y(), 1, 3, text, 2);
    FrontendDraw_MenuList(pMenu, NULL, -1, -1, 0, 1);
    FrontendDraw_Carousel(FUN_004f8410(), 0, NULL);
}

// FUNCTION: CMR2 0x004e2ab0
void FUN_004e2ab0(Menu *pMenu)
{
    FrontendDraw_PlayTime();
    FrontendDraw_MenuPath(pMenu, PATH_X(), PATH_Y(), 1, 1, NULL, -1);
    FrontendDraw_MenuList(pMenu, CFrontend::GetTextString(0x89), -1, -1, 0, 1);
    FrontendDraw_Carousel(FUN_004f8410(), 0, NULL);
}

// Which background matrix map each entry of the main menu shows.
// GLOBAL: CMR2 0x00524008
int g_matrixMapIndex[5] = { 10, 2, 9, 6, 8 };
// 18x12 cell maps (colour indices into g_matrixColours) of the background.
// GLOBAL: CMR2 0x0052401c
char g_matrixMaps[11][0xd8] = {
    0x01, 0x01, 0x01, 0x01, 0x01, 0x06, 0x06, 0x06, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x06,
    0x06, 0x06, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x06, 0x06, 0x06, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x06, 0x06, 0x06, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x01, 0x01, 0x01, 0x01, 0x01, 0x06, 0x06, 0x06, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x06, 0x06, 0x06, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x06,
    0x06, 0x06, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x06, 0x06, 0x06, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x06, 0x06, 0x06, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x06, 0x06, 0x01, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x01, 0x06, 0x06, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x01, 0x06, 0x06, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x06, 0x06, 0x01, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x05,
    0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x05,
    0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x01, 0x02, 0x04, 0x04, 0x02, 0x04, 0x04, 0x02, 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01, 0x01, 0x02, 0x01,
    0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x04, 0x04, 0x04, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x01, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x04,
    0x01, 0x02, 0x04, 0x04, 0x02, 0x04, 0x04, 0x02, 0x01, 0x04, 0x04, 0x04, 0x01, 0x04, 0x04, 0x04, 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x01, 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x01, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0x02, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x01, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00,
    0x00, 0x02, 0x01, 0x02, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x02, 0x02, 0x02, 0x00, 0x00,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x02, 0x02, 0x01, 0x02, 0x02, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x01, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x01, 0x02, 0x01, 0x02, 0x01, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x03, 0x03, 0x03, 0x01, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x01, 0x01, 0x04, 0x04, 0x04, 0x01, 0x02, 0x02, 0x01, 0x04, 0x04, 0x04, 0x01, 0x01, 0x02, 0x02, 0x01, 0x02, 0x02, 0x02, 0x01, 0x01,
    0x04, 0x01, 0x02, 0x02, 0x01, 0x04, 0x01, 0x01, 0x02, 0x02, 0x02, 0x01, 0x04, 0x01, 0x01, 0x02, 0x02, 0x02, 0x01, 0x01, 0x02, 0x02, 0x01, 0x01,
    0x02, 0x02, 0x02, 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01, 0x02, 0x02, 0x01, 0x02, 0x02, 0x01, 0x02, 0x02, 0x01, 0x01, 0x04, 0x04, 0x04,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x04, 0x04, 0x04, 0x01, 0x01, 0x02, 0x02, 0x01, 0x02, 0x02, 0x01, 0x02, 0x02, 0x01, 0x01, 0x04, 0x04, 0x04, 0x04, 0x01, 0x01, 0x02, 0x02, 0x02,
    0x01, 0x01, 0x02, 0x02, 0x01, 0x01, 0x02, 0x02, 0x02, 0x01, 0x01, 0x04, 0x01, 0x02, 0x02, 0x02, 0x01, 0x01, 0x04, 0x01, 0x02, 0x02, 0x01, 0x04,
    0x01, 0x01, 0x02, 0x02, 0x02, 0x01, 0x02, 0x02, 0x01, 0x01, 0x04, 0x04, 0x04, 0x01, 0x02, 0x02, 0x01, 0x04, 0x04, 0x04, 0x01, 0x01, 0x02, 0x02,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
};

void FUN_004d28c0(short x0, short y0, char *pMap);

// Draw callback of the main menu: the menu scroller and, behind it, the
// background matrix picture of the selected entry.
// FUNCTION: CMR2 0x004e2da0
void FUN_004e2da0(Menu *pMenu)
{
    int cellW;

    FrontendDraw_ScrollerRow(FUN_004f2510(), 1);
    cellW = (int)(g_pGraphics->resX * 18) / 640;
    FUN_004d28c0((cellW - (int)(g_pGraphics->resX * 11) / 640) / 2 - cellW * 18 / 2 + (int)g_pGraphics->resX / 2,
                 (int)(g_pGraphics->resY * 100) / 480, g_matrixMaps[g_matrixMapIndex[pMenu->cursor]]);
}

// FUNCTION: CMR2 0x004e3230
void FUN_004e3230(Menu *pMenu)
{
    char *text[4];

    if (g_unk0x00818848 != 0) {
        text[0] = CFrontend::GetTextString(0xb);
        text[1] = CFrontend::GetTextString(0x59);
        text[2] = (char *)RallyData_GetRecord(0);
        text[3] = CFrontend::GetTextString(pMenu->field_0x4);
        FrontendDraw_Breadcrumb(PATH_X(), PATH_Y(), text, 4);
    } else {
        FrontendDraw_MenuPath(pMenu, PATH_X(), PATH_Y(), 1, 2, NULL, -1);
    }
    FrontendDraw_PlayTime();
    FrontendDraw_MenuList(pMenu, NULL, -1, -1, 0, 1);
    FrontendDraw_Carousel(FUN_004f8410(), 0, NULL);
}

// FUNCTION: CMR2 0x004e7770
void FUN_004e7770(int value)
{
    g_unk0x008182b4 = value;
}

// GLOBAL: CMR2 0x00818274
char g_unk0x00818274[0x40];

// FUNCTION: CMR2 0x004e7780
void FUN_004e7780(const char *text)
{
    strcpy(g_unk0x00818274, text);
}

// FUNCTION: CMR2 0x004e77b0
void FUN_004e77b0(BYTE value)
{
    g_unk0x008189a4 = value;
}

// Network game: the name being typed, with a blinking cursor.
// FUNCTION: CMR2 0x004e9820
void FUN_004e9820(Menu *pMenu)
{
    char *text[3];

    text[0] = CFrontend::GetTextString(0);
    text[1] = CFrontend::GetTextString(0x12);
    text[2] = CFrontend::GetTextString(0x1e7);
    FrontendDraw_PlayTime();
    FrontendDraw_Breadcrumb(PATH_X(), PATH_Y(), text, 3);
    Font_DrawText(1, CFrontend::GetTextString(0x1e8), (int)g_pGraphics->resX / 2, (int)g_pGraphics->resY / 4,
                  (int *)g_colourWhite0x00524968, 0x12);
    sprintf(CFrontend::m_stringDest, "%s : %s", CFrontend::GetTextString(0x1c3), g_unk0x00818da8);
    if ((int)CMain::GetFrameDelta() % 20 > 9)
        strcat(CFrontend::m_stringDest, "_");
    Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 260) / 640, (int)g_pGraphics->resY / 2,
                  (int *)g_colourWhite0x00524968, 0x11);
    FrontendDraw_Carousel(FUN_004f8410(), 0, NULL);
}

// FUNCTION: CMR2 0x004ea470
void FUN_004ea470(void)
{
    g_pMenu0x00818abc = g_pMenu0x00818ac0;
}

extern int g_unk0x00818ac8;
void FUN_0040bad0(void);
void FUN_0040bd60(unsigned short slot, DeviceInfo *pOut);
unsigned short FUN_0040bbc0(unsigned short slot);
void FUN_004a2fe0(void);

// Time (CFrontend::FUN_004d20e0) at which the current menu was entered.
// GLOBAL: CMR2 0x008189b8
int g_menuEnterTime;

// Frontend per-frame update: reads the input, keeps the music streaming and
// runs the current menu, switching to the menu it returns.
// FUNCTION: CMR2 0x004ea510
void FUN_004ea510(void)
{
    DeviceInfo *pDevice;
    unsigned int input;
    Menu *pNext;

    pDevice = CInput::FUN_0049ead0(g_unk0x00818ac8);
    FUN_004ea470();
    CInput::FUN_0049eab0();
    FUN_0040bad0();
    FUN_004a2fe0();
    if (g_pMenu0x00818abc != FUN_004fa510() && g_pMenu0x00818abc != FUN_004fa500())
        FUN_0040bd60(g_unk0x00818ac8, pDevice);
    input = pDevice->field_0x8;
    if (g_unk0x00818ac8 == 1)
        input |= CInput::FUN_0049ead0(FUN_0040bbc0(0))->field_0x8 & 0x20;
    pNext = (Menu *)Menu_Update(g_pMenu0x00818abc, input);
    if (pNext != NULL) {
        g_menuEnterTime = CFrontend::FUN_004d20e0();
        g_pMenu0x00818ac0 = pNext;
    }
}

// FUNCTION: CMR2 0x004ea5b0
void FUN_004ea5b0(void)
{
    Menu_CallCallback2(g_pMenu0x00818abc);
}

// FUNCTION: CMR2 0x004ea5c0
void FUN_004ea5c0(BYTE value)
{
    g_unk0x00818ac4 = value;
}

// FUNCTION: CMR2 0x004ea5d0
Menu *FUN_004ea5d0(void)
{
    return g_pMenu0x00818abc;
}

extern BYTE g_unk0x00818ce4;
DPID FUN_004a1a00(void);
BYTE RallyData_FUN_004086b0(BYTE index);
BYTE FUN_004086f0(unsigned int param1);
void FUN_00409be0(int param);

// Sends this machine's player description (id, car, flags) to the network
// player list.
// TODO: CMR2 0x004ec2b0 (implemented, match 44%)
void FUN_004ec2b0(void)
{
    DWORD info[4];

    info[0] = 0;
    info[1] = 0;
    info[2] = 0;
    info[3] = 0;
    info[0] = FUN_004a1a00();
    info[1] = (RallyData_FUN_004086b0(0) & 0x1f) | (info[1] & 0xffffffe0);
    info[1] = ((((g_unk0x00818ce4 & 1) | 2) << 1 | (FUN_004086f0(0) & 1)) << 5) | (info[1] & 0xfffffc9f);
    FUN_00409be0((int)info);
}

// FUNCTION: CMR2 0x004eca60
void FUN_004eca60(Menu *pMenu, char param)
{
    Menu_SetFlags(pMenu, 1, 1, 1, 1);
    g_unk0x00818ef4 = 0;
    g_unk0x00819024 = 0;
    g_unk0x00819014 = 0;
    g_unk0x0052528c = -1;
    g_unk0x00525288 = -1;
    FUN_004b7c80();
    sprintf(g_unk0x00818da8, CMain::m_logFileBlankLine);
    g_unk0x0081901c = 0;
    g_unk0x00819020 = 0;
    pMenu->items[0].id = 0x1ff;
    if (param == 0) {
        g_unk0x00818d04 = 0;
        return;
    }
    g_unk0x00818d04 = 1;
    FUN_004b7c80();
}

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

// FUNCTION: CMR2 0x004edb50
BYTE *FUN_004edb50(void)
{
    return g_unk0x00818f14;
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

// FUNCTION: CMR2 0x004eeab0
void FUN_004eeab0(Menu *pMenu, char param)
{
    int order[8];
    unsigned int *pFlags;
    unsigned int count;
    unsigned int n;
    int i;

    i = 0;
    order[0] = 6;
    order[1] = 3;
    order[2] = 1;
    order[3] = 4;
    order[4] = 0;
    order[5] = 2;
    order[6] = 5;
    order[7] = 7;
    pFlags = CGameInfo::FUN_00405db0();
    count = 6;
    if (!CGameInfo::FUN_00406410(0xd)) {
        count = (*pFlags >> 2 & 3) * 3;
        n = (*pFlags >> 4 & 3) * 3;
        if (n > count)
            count = n;
        n = (*pFlags >> 6 & 3) * 3;
        if (n > count)
            count = n;
    }
    do {
        if (i < (int)count || CGameInfo::FUN_00406410(0xd))
            g_unk0x00818ed4[i] = order[i];
        i++;
    } while (i < 6);
    if ((*pFlags & 0x100000) || CGameInfo::FUN_00406410(0xd)) {
        count++;
        g_unk0x00818ed4[i] = 5;
    }
    if ((*pFlags & 0x200000) || CGameInfo::FUN_00406410(0xd)) {
        count++;
        g_unk0x00818ed4[i + 1] = 7;
    }
    Menu_GetItem(pMenu, 0)->min = count;
    Menu_GetItem(pMenu, 0)->max = 0;
    for (i = 0; i < (int)count; i++) {
        if (g_unk0x00818ed4[i] == FUN_00406730()) {
            Menu_GetItem(pMenu, 0)->max = i;
            break;
        }
    }
    if (param == 0) {
        Menu_GetItem(pMenu, 1)->max = FUN_00406710();
        if (Menu_GetItem(pMenu, 1)->max != 0)
            Menu_GetItem(pMenu, 1)->max -= 4;
    }
}

// FUNCTION: CMR2 0x004eed50
void FUN_004eed50(Menu *pMenu, int param)
{
    Menu_SetFlags(pMenu, 0, 0, 1, 1);
    FUN_004b7c80();
    sprintf(g_unk0x00818da8, CMain::m_logFileBlankLine);
}

// Name entry: appends the typed character or deletes the last one.
// FUNCTION: CMR2 0x004eed80
void FUN_004eed80(Menu *pMenu)
{
    int key;
    int len;

    g_unk0x00818cd8 = g_unk0x00818da8;
    strcpy(CFrontend::m_stringDest, g_unk0x00818da8);
    if (!FUN_004b7cd0(&key))
        return;
    if (key != 8) {
        len = strlen(CFrontend::m_stringDest);
        if (len < 19 && strchr(g_nameChars0x00525330, (char)key) != NULL) {
            CFrontend::m_stringDest[len] = (char)key;
            CFrontend::m_stringDest[len + 1] = 0;
            Menu_PlaySoundId(1);
        }
    } else if (CFrontend::m_stringDest[0] != 0) {
        CFrontend::m_stringDest[strlen(CFrontend::m_stringDest) - 1] = 0;
        Menu_PlaySoundId(2);
    }
    strcpy(g_unk0x00818cd8, CFrontend::m_stringDest);
}

// FUNCTION: CMR2 0x004eef30
void FUN_004eef30(Menu *pMenu, int param)
{
    Menu_GetItem(pMenu, 0)->min = CNetworkLeaderboards::GetTotalLeaderboards();
    if (Menu_GetItem(pMenu, 0)->min != 0) {
        Menu_GetItem(pMenu, 0)->enabled = 1;
        if (CNetworkLeaderboards::GetLeaderboardId() == -1)
            CNetworkLeaderboards::SetLeaderboardId(0);
        Menu_GetItem(pMenu, 0)->max = CNetworkLeaderboards::GetLeaderboardId();
        return;
    }
    Menu_GetItem(pMenu, 0)->enabled = 0;
}

// FUNCTION: CMR2 0x004ef030
void FUN_004ef030(Menu *pMenu)
{
    Menu_GetItem(pMenu, 0)->min = CNetworkLeaderboards::GetTotalLeaderboards();
    if (Menu_GetItem(pMenu, 0)->min != 0) {
        Menu_GetItem(pMenu, 2)->enabled = 1;
        if (!Menu_GetItem(pMenu, 0)->enabled) {
            Menu_GetItem(pMenu, 0)->enabled = 1;
            if (CNetworkLeaderboards::GetLeaderboardId() == -1)
                CNetworkLeaderboards::SetLeaderboardId(0);
            Menu_GetItem(pMenu, 0)->max = CNetworkLeaderboards::GetLeaderboardId();
        }
        if (CNetworkLeaderboards::GetTotalLeaderboards() < 32)
            Menu_GetItem(pMenu, 1)->enabled = 1;
        else
            Menu_GetItem(pMenu, 1)->enabled = 0;
        if (Menu_GetItem(pMenu, 0)->max >= Menu_GetItem(pMenu, 0)->min)
            Menu_GetItem(pMenu, 0)->max = Menu_GetItem(pMenu, 0)->min - 1;
    } else {
        Menu_GetItem(pMenu, 2)->enabled = 0;
        Menu_GetItem(pMenu, 1)->enabled = 1;
    }
}

// Resets the 12 frontend scrollers: no menu attached, item gap scaled from
// 24 pixels at 640 wide.
// FUNCTION: CMR2 0x004ef150
void FUN_004ef150(void)
{
    int i;

    for (i = 0; i < 12; i++) {
        g_menuScrollers[i].pMenu = NULL;
        g_menuScrollers[i].spacing = (int)(g_pGraphics->resX * 24) / 640;
    }
}

// Recomputes every scroller attached to a menu: gap, reset position and the
// width of each item's text.
// TODO: CMR2 0x004ef190 (implemented, match 87%)
void FUN_004ef190(void)
{
    MenuScroller *p;
    int k;

    p = g_menuScrollers;
    do {
        p->spacing = (int)(g_pGraphics->resX * 24) / 640;
        p->offset = 0;
        p->startOffset = 0;
        if (p->pMenu != NULL) {
            p->count = p->pMenu->itemCount;
            for (k = 0; k < p->pMenu->itemCount; k++) {
                strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(p->pMenu->items[k].id));
                CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
                p->widths[k] = Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest);
            }
        }
        p++;
    } while (p < &g_menuScrollers[12]);
}

// FUNCTION: CMR2 0x004ef480
void FUN_004ef480(int *pOut1, int *pOut2)
{
    *pOut1 = g_unk0x00525398;
    *pOut2 = g_unk0x0081912c;
}

// FUNCTION: CMR2 0x004ef4a0
void FUN_004ef4a0(int value)
{
    g_unk0x00525398 = value;
    g_unk0x0052539c = value;
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

// FUNCTION: CMR2 0x004f1160
void FUN_004f1160(Menu *pMenu, char param)
{
    g_unk0x00819040 = CMain::GetFrameDelta();
    g_unk0x0081986c = 1;
    FUN_004a0c50(1);
    g_unk0x00819038 = 1;
    FUN_004ea480(FUN_004f2be0());
    if (param != 0) {
        pMenu->cursor = 2;
        pMenu->items[2].max = pMenu->items[2].min - 1;
    } else {
        pMenu->cursor = 0;
        pMenu->items[0].max = 0;
    }
    FUN_004b7c80();
    pMenu->cursor = 2;
    pMenu->items[2].max = pMenu->items[2].min - 1;
}

// GLOBAL: CMR2 0x005253f4
char g_nameChars0x005253f4[] = "abcdefghijklmnopqrstuvwxyz. ABCDEFGHIJKLMNOPQRSTUVWXYZ";
// GLOBAL: CMR2 0x0052542c
char g_upperChars0x0052542c[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

void *FUN_00408470(unsigned int param1);
void FUN_004eaf90(BYTE index, char *name);

// Name entry of the category record: typed letters also move the cursor of
// the three letter rows (a-j, k-t, u-z plus '.', space, delete and OK).
// FUNCTION: CMR2 0x004f11d0
void FUN_004f11d0(Menu *pMenu)
{
    int key;
    int len;

    if (!g_unk0x00819038)
        return;
    strcpy(CFrontend::m_stringDest, (char *)FUN_00408470(FUN_004f2be0()));
    if (FUN_004b7cd0(&key)) {
        if (key != 8) {
            len = strlen(CFrontend::m_stringDest);
            if (len >= 0x2b || strchr(g_nameChars0x005253f4, (char)key) == NULL)
                goto done;
            if (strchr(g_upperChars0x0052542c, (char)key) != NULL)
                key += 0x20;
            CFrontend::m_stringDest[len] = (char)key;
            CFrontend::m_stringDest[len + 1] = 0;
            if ((char)key >= 'a' && (char)key <= 'j') {
                pMenu->cursor = 0;
                pMenu->items[0].max = (char)key - 'a';
            }
            if ((char)key >= 'k' && (char)key <= 't') {
                pMenu->cursor = 1;
                pMenu->items[1].max = (char)key - 'k';
            }
            if ((char)key >= 'u' && (char)key <= 'z') {
                pMenu->cursor = 2;
                pMenu->items[2].max = (char)key - 'u';
            }
            if ((char)key == '.') {
                pMenu->cursor = 2;
                pMenu->items[2].max = 7;
            } else if ((char)key == ' ') {
                pMenu->cursor = 2;
                pMenu->items[2].max = 8;
            }
            g_unk0x00819040 = CMain::GetFrameDelta();
            g_unk0x0081986c = 1;
            Menu_PlaySoundId(1);
        } else {
            if (CFrontend::m_stringDest[0] != 0) {
                CFrontend::m_stringDest[strlen(CFrontend::m_stringDest) - 1] = 0;
                Menu_PlaySoundId(2);
            }
            pMenu->cursor = 2;
            pMenu->items[2].max = 8;
            g_unk0x00819040 = CMain::GetFrameDelta();
            g_unk0x0081986c = 1;
        }
    } else {
        if (!g_unk0x0081986c || CMain::GetFrameDelta() - g_unk0x00819040 <= 10)
            goto done;
        g_unk0x00819040 = -1;
        pMenu->cursor = 2;
        pMenu->items[2].max = 9;
        g_unk0x0081986c = 0;
        Menu_PlaySoundId(0);
    }
done:
    FUN_004eaf90(FUN_004f2be0(), CFrontend::m_stringDest);
    strcpy((char *)g_unk0x008190f4, CFrontend::m_stringDest);
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

// FUNCTION: CMR2 0x004f1bb0
void FUN_004f1bb0(BYTE value)
{
    g_unk0x00819048 = value;
}

// FUNCTION: CMR2 0x004f1bc0
void FUN_004f1bc0(int value)
{
    g_unk0x00819870 = value;
}

// FUNCTION: CMR2 0x004f1bd0
void FUN_004f1bd0(Menu *pMenu, int param)
{
    g_unk0x00819864 = 1;
    g_unk0x0081987c = 0;
    g_unk0x008196e4 = CFrontend::FUN_004d20e0();
}

// FUNCTION: CMR2 0x004f1bf0
int FUN_004f1bf0(void)
{
    return g_unk0x008196e4;
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

// FUNCTION: CMR2 0x004f23c0
int FUN_004f23c0(void)
{
    return g_unk0x00819128;
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

// FUNCTION: CMR2 0x004f25b0
int FUN_004f25b0(void)
{
    return g_unk0x00525398;
}

// FUNCTION: CMR2 0x004f25c0
int FUN_004f25c0(void)
{
    return g_unk0x0081912c;
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

// FUNCTION: CMR2 0x004f2bf0
void FUN_004f2bf0(int value)
{
    g_unk0x00819880 = value;
}

// FUNCTION: CMR2 0x004f2c00
int FUN_004f2c00(void)
{
    return g_unk0x00819030;
}

// FUNCTION: CMR2 0x004f2c10
void FUN_004f2c10(int value)
{
    g_unk0x00819030 = value;
}

// FUNCTION: CMR2 0x004f2c20
int FUN_004f2c20(void)
{
    return g_unk0x00819124;
}

// FUNCTION: CMR2 0x004f2c30
void FUN_004f2c30(int value)
{
    g_unk0x00819124 = value;
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

// FUNCTION: CMR2 0x004f3a30
BYTE FUN_004f3a30(void)
{
    return g_unk0x00819748;
}

// FUNCTION: CMR2 0x004f3a40
int FUN_004f3a40(void)
{
    return g_unk0x00819050;
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

// FUNCTION: CMR2 0x004f3af0
BYTE *FUN_004f3af0(void)
{
    return g_unk0x008190f4;
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

// Frontend sound names and path
// GLOBAL: CMR2 0x0052544c
char g_menuSoundNames[5][9] = {"move", "select", "back", "error", "toggle"};
// GLOBAL: CMR2 0x0051ea44
char g_strMenuSoundFormat[24] = "%s\\Sounds\\menu\\%s.wav";
// GLOBAL: CMR2 0x00525a84
char g_strMenuDotFormat[32] = "%s\\frontend\\Textures\\dot0%d.tga";

// Loads the five frontend sounds from the common frontend archive.
// FUNCTION: CMR2 0x004f3b50
BYTE FUN_004f3b50(void)
{
    BYTE ok;
    int i;

    ok = 1;
    g_menuSoundBase = 0;
    for (i = 0; i < 5; i++) {
        sprintf(CFrontend::m_stringDest, g_strMenuSoundFormat, CInstallInfo::GetGameCDPath(), g_menuSoundNames[i]);
        if (!Sound_LoadSample(CFrontend::m_stringDest, 0, CFrontend::FUN_004d2190()))
            ok = 0;
    }
    return ok;
}

// Sets the input repeat rate from the options and binds the five frontend
// sounds to the menu actions.
// TODO: CMR2 0x004f3bb0 (implemented, match 71%)
void FUN_004f3bb0(void)
{
    int rate;

    g_unk0x0081988c = -1;
    rate = (int)(CGameInfo::FUN_00405e70() << 16) / 100;
    CInput::FUN_0049ffc0(rate / 4);
    CInput::FUN_0049ff80(g_menuSoundBase, g_menuSoundBase + 1, g_menuSoundBase + 2, g_menuSoundBase + 3,
                         g_menuSoundBase + 4);
    FUN_004a0c40(1);
}

// Loads the four "dot" textures of the frontend.
// FUNCTION: CMR2 0x004f3f60
void FUN_004f3f60(void)
{
    int i;

    for (i = 0; i < 4; i++) {
        sprintf(CFrontend::m_stringDest, g_strMenuDotFormat, CInstallInfo::GetGameCDPath(), i);
        g_menuDotTextures[i] =
            CTexture::FindLoadTexture(CGenericFileLoader::GetGenericFile(), CFrontend::m_stringDest, 0, 0, 0, 0);
    }
}

// --- Main menu animation: the letters of "colinmcrae", a trail of dots and
// six dot streams run along a closed 16-segment spline; each kind of main
// menu entry has its own path and the path morphs into the next one over
// 250 ms when the cursor moves.

// Path used with cheat 0xe.
// GLOBAL: CMR2 0x00525490
int g_menuPathCheat[19][2] = {
    324, 157, 371, 152, 366, 91, 395, 108,
    463, 133, 391, 217, 405, 340, 385, 352,
    366, 276, 361, 330, 374, 337, 356, 356,
    239, 357, 225, 341, 245, 320, 246, 244,
    289, 181, 371, 152, 414, 54,
};
// One path (19 points, 16.16 spline control points in 640x480 units) per
// animation mode.
// GLOBAL: CMR2 0x00525528
int g_menuPaths[9][19][2] = {
    216, 133, 260, 100, 307, 89, 349, 95, 398, 127, 426, 172, 430, 229, 405, 306, 340, 314, 261, 315, 266, 279, 286, 224, 252, 194, 207, 197, 177, 192, 192, 176, 216, 133, 260, 100, 307, 89,
    536, 122, 520, 113, 479, 110, 437, 112, 494, 131, 468, 144, 404, 144, 349, 156, 439, 174, 509, 192, 404, 212, 219, 203, 75, 215, 153, 242, 142, 284, 234, 320, 400, 294, 536, 294, 648, 238,
    221, 108, 279, 104, 340, 108, 400, 117, 473, 148, 505, 192, 499, 240, 459, 274, 393, 272, 315, 262, 232, 271, 144, 291, 93, 254, 84, 205, 108, 154, 165, 121, 221, 108, 279, 104, 340, 108,
    49, 158, 512, 158, 608, 171, 542, 188, 608, 204, 544, 226, 608, 244, 543, 264, 587, 294, 110, 301, 27, 283, 85, 264, 25, 247, 87, 224, 24, 206, 87, 185, 49, 158, 512, 158, 608, 171,
    219, 115, 302, 88, 385, 112, 441, 188, 465, 312, 428, 322, 405, 278, 387, 235, 355, 216, 303, 239, 245, 216, 219, 232, 197, 276, 175, 322, 138, 303, 167, 189, 219, 115, 302, 88, 385, 112,
    270, 174, 332, 125, 382, 84, 390, 92, 391, 163, 410, 206, 409, 237, 394, 271, 394, 350, 374, 343, 326, 306, 269, 266, 203, 263, 187, 252, 186, 196, 197, 177, 270, 174, 332, 125, 382, 84,
    229, 74, 324, 67, 399, 69, 476, 82, 505, 114, 509, 201, 505, 289, 477, 323, 399, 336, 324, 341, 244, 338, 166, 323, 135, 289, 129, 211, 135, 115, 166, 82, 244, 71, 324, 67, 428, 73,
    291, 88, 312, 73, 329, 86, 326, 118, 354, 134, 367, 187, 350, 217, 342, 309, 360, 331, 313, 327, 268, 330, 284, 306, 271, 219, 253, 185, 267, 131, 294, 117, 291, 88, 312, 73, 329, 86,
    178, 224, 199, 282, 250, 326, 322, 334, 386, 306, 421, 254, 428, 191, 404, 143, 354, 174, 303, 210, 256, 243, 201, 276, 178, 225, 186, 163, 232, 107, 296, 86, 358, 99, 407, 140, 428, 191,
};
// Spline tension (tangents are scaled by 1 - tension).
// GLOBAL: CMR2 0x00525a80
int g_menuPathTension = 0x20000;
// GLOBAL: CMR2 0x00525480
char g_strColinMcrae[12] = "colinmcrae";
// Time of the last animation update, -1 = none yet.
// GLOBAL: CMR2 0x0052548c
int g_menuAnimTime = -1;

// GLOBAL: CMR2 0x00819990
int *g_pMenuPath;
// Position (0..1 along the path) of each of the 200 letters.
// GLOBAL: CMR2 0x00819994
int g_menuLetterPos[200];
// Path of the entry under the cursor.
// GLOBAL: CMR2 0x00819cb4
int g_menuPathMode;
// Six streams of 10 dots (head first).
// GLOBAL: CMR2 0x00819cb8
int g_menuStreamPos[6][10];
// Which of the three paths the entries of type 4 cycle through.
// GLOBAL: CMR2 0x00819da8
int g_menuPathVariant;
// Path being morphed from.
// GLOBAL: CMR2 0x00819dac
int g_menuPathPrevMode;
// GLOBAL: CMR2 0x00819db0
int g_menuStreamPoint[6][2];
// Speed of each stream (16.16, 0.5..1.5, random walk).
// GLOBAL: CMR2 0x00819de0
int g_menuStreamSpeed[6];
// Path while morphing from one mode to another.
// GLOBAL: CMR2 0x00819df8
int g_menuPathMorph[19][2];
// GLOBAL: CMR2 0x00819e90
int g_menuPathInit;
// The trail of 15 dots (head first).
// GLOBAL: CMR2 0x00819ea4
int g_menuTrailPos[15];
// GLOBAL: CMR2 0x0081a620
short g_menuDotRect[4];

// Point of the path at t (16.16, 0..1): cubic Hermite spline through the
// control points 1..17 with Catmull-Rom style tangents.
// TODO: CMR2 0x004f3c10 (implemented, match 54%)
void FUN_004f3c10(int *pPoints, int t, int *pOut)
{
    int *p;
    int seg;
    int u;
    int u2;
    int u3;
    int h00;
    int h01;
    int h10;
    int h11;
    int tanA;
    int tanB;
    int x;

    if (t == 0) {
        pOut[0] = pPoints[2];
        pOut[1] = pPoints[3];
        return;
    }
    if (t == 0x10000) {
        pOut[0] = pPoints[0x22];
        pOut[1] = pPoints[0x23];
        return;
    }
    seg = ((t << 4) >> 16) + 1;
    u = (((seg << 16) - 0x10000) / 16 - t + 0x1000) << 4;
    u2 = FixMul(u, u);
    u3 = FixMul(u, u2);
    h00 = u3 * 2 + 0x10000 - u2 * 3;
    h01 = u2 * 3 - u3 * 2;
    h10 = u3 - u2 * 2 + u;
    h11 = u3 - u2;
    p = &pPoints[seg * 2];
    tanA = (p[2] - p[-2]) * (0x10000 - g_menuPathTension) / 2;
    tanB = (p[4] - p[0]) * (0x10000 - g_menuPathTension) / 2;
    x = p[2] * h00 + (FixMul(h11, tanA) + p[0] * h01) + FixMul(h10, tanB);
    tanA = (p[3] - p[-1]) * (0x10000 - g_menuPathTension) / 2;
    tanB = (p[5] - p[1]) * (0x10000 - g_menuPathTension) / 2;
    pOut[0] = x >> 16;
    pOut[1] = (p[3] * h00 + (FixMul(h11, tanA) + p[1] * h01) + FixMul(h10, tanB)) >> 16;
}

// Resets the main menu animation: letters spread along the path, dots at
// the start, and the path of the entry under the cursor.
// TODO: CMR2 0x004f3dd0 (implemented, match 76%)
void FUN_004f3dd0(void)
{
    int *pPos;
    int i;
    int j;
    int k;

    i = 0;
    pPos = g_menuLetterPos;
    do {
        *pPos++ = (i << 16) / 200;
        i++;
    } while (pPos < &g_menuLetterPos[200]);
    memset(g_menuTrailPos, 0, sizeof(g_menuTrailPos));
    memset(g_menuStreamSpeed, 0, sizeof(g_menuStreamSpeed));
    k = 0;
    pPos = g_menuStreamPos[0];
    do {
        for (j = 0; j < 10; j++)
            pPos[j] = k / 6;
        pPos += 10;
        k += 0x4000;
    } while (pPos < g_menuStreamPos[6]);
    g_menuAnimTime = -1;
    switch (FUN_004f8410()->items[FUN_004f8410()->cursor].value) {
    case 0:
        g_menuPathMode = g_menuPathPrevMode = g_menuPathVariant = 0;
        break;
    case 1:
        g_menuPathMode = g_menuPathPrevMode = g_menuPathVariant = 1;
        break;
    case 2:
        g_menuPathMode = g_menuPathPrevMode = g_menuPathVariant = 2;
        break;
    case 3:
        g_menuPathMode = g_menuPathPrevMode = g_menuPathVariant = 3;
        break;
    case 4:
        g_menuPathMode = 4;
        g_menuPathPrevMode = 4;
        g_menuPathVariant = (unsigned int)(CFrontend::FUN_004d20e0() - FUN_004f25c0()) / 500 % 3;
        break;
    case 5:
        g_menuPathMode = g_menuPathPrevMode = g_menuPathVariant = 7;
        break;
    case 6:
        g_menuPathMode = g_menuPathPrevMode = g_menuPathVariant = 8;
        break;
    }
}

// Distance to move this frame at `speed` (16.16 per second), from the time
// since the last update (0.02 s the first time).
// FUNCTION: CMR2 0x004f3fb0
void FUN_004f3fb0(int *pOut, int speed)
{
    if (g_menuAnimTime == -1) {
        *pOut = FixMul(speed, FixDiv(20 << 16, 1000 << 16));
        return;
    }
    *pOut = FixMul(speed, FixDiv((int)(__int64)((unsigned int)(CFrontend::FUN_004d20e0() - g_menuAnimTime) * CGraphics::m_65536),
                                 1000 << 16));
}

// Wraps a position along the path into 0..1.
#define MENU_WRAP(v)                        \
    if ((v) < 0)                            \
        (v) = (v) % 0x10000 + 0x10000;      \
    else if ((v) > 0)                       \
        (v) = (v) % 0x10000;

// Main menu animation update: moves the letters, the trail and the streams
// along the path and picks (or morphs) the path of the entry under the cursor.
// TODO: CMR2 0x004f4050 (implemented, match 72%)
void FUN_004f4050(void)
{
    int *pPos;
    int *pSpeed;
    int *pPoint;
    unsigned int elapsed;
    int step;
    int f;
    int dx;
    int dy;
    int j;

    CInput::FUN_0049ead0(0);
    FUN_004f3fb0(&step, 0xccc);
    pPos = g_menuLetterPos;
    do {
        *pPos -= step;
        MENU_WRAP(*pPos)
        pPos++;
    } while (pPos < &g_menuLetterPos[200]);
    if (CGameInfo::FUN_00406410(0xe)) {
        g_pMenuPath = g_menuPathCheat[0];
    } else if (g_menuPathInit == 0) {
        g_menuPathInit = 1;
        g_pMenuPath = g_menuPaths[g_menuPathMode][0];
    } else if (g_menuPathInit == 1) {
        switch (FUN_004f8410()->items[FUN_004f8410()->cursor].value) {
        case 0:
            g_menuPathMode = 0;
            break;
        case 1:
            g_menuPathMode = 1;
            break;
        case 2:
            g_menuPathMode = 2;
            break;
        case 3:
            g_menuPathMode = 3;
            break;
        case 4:
            g_menuPathMode = g_menuPathVariant + 4;
            g_menuPathVariant = (unsigned int)(CFrontend::FUN_004d20e0() - FUN_004f25c0()) / 500 % 3;
            break;
        case 5:
            g_menuPathMode = 7;
            break;
        case 6:
            g_menuPathMode = 8;
            break;
        }
        if (FUN_004f25b0() == -1) {
            g_menuPathPrevMode = g_menuPathMode;
        } else if (FUN_004f8410()->items[FUN_004f8410()->cursor].value == 4
                   && (unsigned int)(CFrontend::FUN_004d20e0() - FUN_004f25c0()) > 250) {
            g_menuPathPrevMode = (unsigned int)(CFrontend::FUN_004d20e0() - FUN_004f25c0()) / 500 % 3 + 3;
            if (g_menuPathPrevMode < 4)
                g_menuPathPrevMode = 6;
        } else {
            switch (FUN_004f8410()->items[FUN_004f25b0()].value) {
            case 0:
                g_menuPathPrevMode = 0;
                break;
            case 1:
                g_menuPathPrevMode = 1;
                break;
            case 2:
                g_menuPathPrevMode = 2;
                break;
            case 3:
                g_menuPathPrevMode = 3;
                break;
            case 4:
                g_menuPathPrevMode = g_menuPathVariant + 4;
                break;
            case 5:
                g_menuPathPrevMode = 7;
                break;
            case 6:
                g_menuPathPrevMode = 8;
                break;
            }
        }
        if (FUN_004f8410()->items[FUN_004f8410()->cursor].value == 4)
            elapsed = (unsigned int)(CFrontend::FUN_004d20e0() - FUN_004f25c0()) % 500;
        else
            elapsed = CFrontend::FUN_004d20e0() - FUN_004f25c0();
        if (elapsed > 250) {
            g_pMenuPath = g_menuPaths[g_menuPathMode][0];
        } else {
            f = FixDiv((int)(__int64)(elapsed * CGraphics::m_65536), 250 << 16);
            j = 0;
            pPoint = g_menuPathMorph[0];
            do {
                dy = g_menuPaths[g_menuPathMode][j][1] - g_menuPaths[g_menuPathPrevMode][j][1];
                dx = g_menuPaths[g_menuPathMode][j][0] - g_menuPaths[g_menuPathPrevMode][j][0];
                pPoint[0] = g_menuPaths[g_menuPathPrevMode][j][0] + FixMulShift32((int)(__int64)(dx * CGraphics::m_65536), f);
                pPoint[1] = g_menuPaths[g_menuPathPrevMode][j][1] + FixMulShift32((int)(__int64)(dy * CGraphics::m_65536), f);
                pPoint += 2;
                j++;
            } while (pPoint < g_menuPathMorph[19]);
            g_pMenuPath = g_menuPathMorph[0];
        }
    }
    FUN_004f3fb0(&step, 0x1333);
    g_menuTrailPos[0] -= step;
    MENU_WRAP(g_menuTrailPos[0])
    for (pPos = &g_menuTrailPos[14]; pPos > g_menuTrailPos; pPos--)
        *pPos = pPos[-1];
    pSpeed = g_menuStreamSpeed;
    do {
        *pSpeed += FixDiv((int)(__int64)((rand() % 101 - 50) * CGraphics::m_65536), 1000 << 16);
        if (*pSpeed > 0x18000)
            *pSpeed = 0x18000;
        if (*pSpeed < 0x8000)
            *pSpeed = 0x8000;
        pSpeed++;
    } while (pSpeed < &g_menuStreamSpeed[6]);
    FUN_004f3fb0(&step, 0x3333);
    pPos = g_menuStreamPos[0];
    pSpeed = g_menuStreamSpeed;
    do {
        *pPos -= FixMul(step, *pSpeed);
        MENU_WRAP(*pPos)
        for (j = 9; j != 0; j--)
            pPos[j] = pPos[j - 1];
        pSpeed++;
        pPos += 10;
    } while (pSpeed < &g_menuStreamSpeed[6]);
    g_menuAnimTime = CFrontend::FUN_004d20e0();
}

// Draws the 200 letters of "colinmcrae" along the path.
// FUNCTION: CMR2 0x004f45a0
void FUN_004f45a0(void)
{
    BYTE colour[4];
    int point[2];
    unsigned int i;
    int *pPos;

    colour[0] = 0xff;
    colour[1] = 0xff;
    colour[2] = 0xff;
    colour[3] = 0x80;
    Font_Select(0, (int *)colour);
    i = 0;
    pPos = g_menuLetterPos;
    do {
        FUN_004f3c10(g_pMenuPath, *pPos, point);
        Font_DrawChar(g_strColinMcrae[i % strlen(g_strColinMcrae)], (int)(g_pGraphics->resX * point[0]) / 640,
                      (int)(g_pGraphics->resY * point[1]) / 480);
        pPos++;
        i++;
    } while (pPos < &g_menuLetterPos[200]);
}

// Draws the trail of 15 dots, fading out towards the tail.
// FUNCTION: CMR2 0x004f4650
void FUN_004f4650(void)
{
    BYTE colour[4];
    int point[2];
    Texture *pTexture;
    int k;
    int fade;

    colour[0] = 0xff;
    colour[1] = 0xff;
    colour[2] = 0xff;
    k = 14 * 4;
    fade = 14 * 0xff;
    do {
        colour[3] = 0xff - fade / 15;
        FUN_004f3c10(g_pMenuPath, *(int *)((BYTE *)g_menuTrailPos + k), point);
        g_menuDotRect[0] = (int)(g_pGraphics->resX * point[0]) / 640 - 2;
        g_menuDotRect[1] = (int)(g_pGraphics->resY * point[1]) / 480 + 3;
        pTexture = g_menuDotTextures[k / 15];
        g_menuDotRect[2] = pTexture->width;
        g_menuDotRect[3] = pTexture->height;
        Sprite_Queue((SpriteRect *)&pTexture->field_0x11c, (SpriteRect *)g_menuDotRect, pTexture, 1, 0, NULL, NULL, colour, 8);
        fade -= 0xff;
        k -= 4;
    } while (fade >= 0);
}

// Draws the six streams of 10 dots, fading out towards their tails.
// TODO: CMR2 0x004f4760 (implemented, match 51%)
void FUN_004f4760(void)
{
    BYTE colour[4];
    Texture *pTexture;
    int *pPoint;
    int *pPos;
    int *pStream;
    int k;
    int fade;

    colour[0] = 0xff;
    colour[1] = 0xff;
    colour[2] = 0xff;
    pStream = &g_menuStreamPos[0][9];
    pPoint = g_menuStreamPoint[0];
    do {
        k = 9 * 4;
        fade = 9 * 0xff;
        pPos = pStream;
        do {
            colour[3] = 0xff - fade / 10;
            FUN_004f3c10(g_pMenuPath, *pPos, pPoint);
            g_menuDotRect[0] = (int)(pPoint[0] * g_pGraphics->resX) / 640 - 2;
            g_menuDotRect[1] = (int)(g_pGraphics->resY * pPoint[1]) / 480 + 3;
            pTexture = g_menuDotTextures[k / 10];
            g_menuDotRect[2] = pTexture->width;
            g_menuDotRect[3] = pTexture->height;
            Sprite_Queue((SpriteRect *)&pTexture->field_0x11c, (SpriteRect *)g_menuDotRect, pTexture, 1, 0, NULL, NULL, colour, 8);
            fade -= 0xff;
            k -= 4;
            pPos--;
        } while (fade >= 0);
        pPoint += 2;
        pStream += 10;
    } while (pPoint < g_menuStreamPoint[6]);
}

// Time of the last input on the main menu (for the attract mode).
// GLOBAL: CMR2 0x0081904c
DWORD g_mainMenuInputTime;

void FUN_004ea8a0(BYTE param1);
void FUN_004a3c30(int language);
bool FUN_004f48b0(void);
void FUN_004f4910(char registerRelease);
void FUN_004fa4d0(void);
unsigned int FUN_0049e940(void);
void FUN_004ea8c0(BYTE param1);
void FUN_004ea8e0(BYTE param1);
void FUN_004ea950(BYTE param1);
void FUN_004ea9c0(unsigned int param1, unsigned int param2);
void FUN_004d05f0(void);
void FUN_00409a30(void);
void RallyData_FUN_004068b0(BYTE param1);
void RallyData_FUN_004068e0(BYTE param1);

// Leaving the language menu: applies the chosen language (texts, fonts,
// credits), rebuilds the scrollers and, the first time, the controls menu,
// and makes the main menu the parent of the language menu and its entries.
// TODO: CMR2 0x004ef270 (implemented, match 82%)
void FUN_004ef270(Menu *pMenu, char back)
{
    int i;

    if (back == 0) {
        FUN_004ea8a0(pMenu->cursor);
        FUN_004a3c30(pMenu->cursor);
        FUN_004f48b0();
        FUN_004f4910(0);
        CGameInfo::FUN_00405ec0(CGameInfo::GetGameLanguage() == 0);
        CFrontend::FUN_004d2790();
        FUN_004ef190();
        if (pMenu->pParent == NULL)
            FUN_004fa4d0();
        Menu_SetParent(pMenu, FUN_004f82c0());
        for (i = 0; i < pMenu->itemCount; i++)
            Menu_SetItemSubMenu(pMenu, i, FUN_004f82c0());
    }
}

// Entering the main menu: resets the animation and the attract-mode timer and
// sets up the scroller of the main menu.
// FUNCTION: CMR2 0x004ef300
void FUN_004ef300(Menu *pMenu, int param)
{
    MenuScroller *p;
    int k;

    g_mainMenuInputTime = timeGetTime();
    FUN_004f3dd0();
    p = FUN_004f24f0();
    p->offset = 0;
    p->startOffset = 0;
    p->current = pMenu->cursor;
    p->previous = pMenu->cursor;
    p->spacing = (int)(g_pGraphics->resX * 24) / 640;
    p->count = pMenu->itemCount;
    p->offset = 0;
    p->current = pMenu->cursor;
    for (k = 0; k < pMenu->itemCount; k++) {
        strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(pMenu->items[k].id));
        CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
        p->widths[k] = Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest);
    }
    p->startTime = CFrontend::FUN_004d20e0();
    p->pMenu = pMenu;
    CGameInfo::FUN_00405de0(0);
}

// Update callback of the main menu: scroller, animation, and the attract
// mode after 30.5 s without input.
// FUNCTION: CMR2 0x004ef420
void FUN_004ef420(Menu *pMenu)
{
    DWORD now;

    FUN_004f37c0(FUN_004f24f0());
    FUN_004f4050();
    now = timeGetTime();
    if (FUN_0049e940() != 0)
        g_mainMenuInputTime = timeGetTime();
    if ((int)(now - g_mainMenuInputTime) > 30500) {
        CGameInfo::FUN_00406330(1);
        Menu_SetNextAction((int)FUN_004f8330());
    }
}

// Draw callback of the main menu: path, carousel, the "colinmcrae" letters
// and, for some entries, the dot trail or the dot streams.
// FUNCTION: CMR2 0x004d4ba0
void FUN_004d4ba0(Menu *pMenu)
{
    FrontendDraw_PlayTime();
    FrontendDraw_MenuPath(pMenu, (int)(g_pGraphics->resX * 24) / 640, (int)(g_pGraphics->resY * 38) / 480, 1, -1, NULL, -1);
    FrontendDraw_Carousel(pMenu, 1, NULL);
    FUN_004f45a0();
    if (pMenu->items[pMenu->cursor].value == 1)
        FUN_004f4650();
    if (pMenu->items[pMenu->cursor].value == 2)
        FUN_004f4760();
}

// Leaving the main menu: remembers the entry; the first entry (single
// player) resets the session differently from the others.
// FUNCTION: CMR2 0x004f25d0
void FUN_004f25d0(Menu *pMenu, int param)
{
    g_unk0x00525398 = pMenu->cursor;
    if (pMenu->items[pMenu->cursor].value == 0) {
        FUN_004d27c0(1);
        FUN_004ea8e0(2);
        FUN_00409a30();
        FUN_004d05f0();
        return;
    }
    FUN_004d27c0(0);
    FUN_00409a30();
    FUN_004d05f0();
}

// Item callback of the championship entry: sets up a new championship.
// FUNCTION: CMR2 0x004ec930
void FUN_004ec930(Menu *pMenu, int param)
{
    FUN_004ea8c0(1);
    CGameInfo::FUN_00405de0(1);
    FUN_004ea950(0);
    FUN_004f1bc0((int)pMenu);
    Menu_SetParent(FUN_004f83a0(), pMenu);
    FUN_004f1bb0(CGameInfo::FUN_00405d70());
    if (CGameInfo::FUN_00405d80() < 8) {
        FUN_004ea8e0(8);
        RallyData_FUN_004068b0(0);
        RallyData_FUN_004068e0(0);
        FUN_004ea9c0(0, 0);
    }
}

// Entering the language menu: puts the cursor on the current language and
// sets up its scroller.
// FUNCTION: CMR2 0x004f3400
void FUN_004f3400(Menu *pMenu, int param)
{
    MenuScroller *p;
    int language;
    int k;

    language = CGameInfo::GetGameLanguage();
    if (language == 1000)
        FUN_004f82d0()->cursor = 0;
    else
        FUN_004f82d0()->cursor = language;
    p = FUN_004f2510();
    p->current = FUN_004f82d0()->cursor;
    p->previous = FUN_004f82d0()->cursor;
    p->spacing = (int)(g_pGraphics->resX * 24) / 640;
    p->count = FUN_004f82d0()->itemCount;
    p->offset = 0;
    p->startOffset = 0;
    for (k = 0; k < FUN_004f82d0()->itemCount; k++) {
        strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(FUN_004f82d0()->items[k].id));
        CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
        p->widths[k] = Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest);
    }
    p->startTime = CFrontend::FUN_004d20e0();
    p->pMenu = pMenu;
}

// Update callback of the language menu.
// FUNCTION: CMR2 0x004f39e0
void FUN_004f39e0(Menu *pMenu)
{
    FUN_004f37c0(FUN_004f2510());
}

// Set when the new-profile entry created a profile that "back" must undo.
// GLOBAL: CMR2 0x00819879
BYTE g_unk0x00819879;
// "Load <name>" texts of the up to four free saved profiles.
// GLOBAL: CMR2 0x00819054
char g_profileEntryTexts[4][40];

void FUN_004eb860(int index, int profile);
void FUN_004ebf20(int index);
void FUN_004eb000(BYTE index, char set);
void FUN_004ebe80(int index);
int FUN_004ec020(void);
int FUN_004ec090(int n);
BYTE *FUN_004ec110(int n);

// Item callback: makes this menu the parent of the next one.
// FUNCTION: CMR2 0x004f0c40
void FUN_004f0c40(Menu *pMenu, int param)
{
    FUN_004f83b0()->pParent = pMenu;
}

// Item callback of "new profile": gives player 1 a fresh profile and goes to
// the name entry.
// FUNCTION: CMR2 0x004f2750
void FUN_004f2750(Menu *pMenu, int param)
{
    g_unk0x00819879 = 1;
    FUN_004f2bf0(0);
    FUN_004eb860(FUN_004f2be0(), -1);
    FUN_004ebf20(FUN_004f2be0());
    FUN_004eb000(FUN_004f2be0(), 0);
    Menu_SetParent(FUN_004f83c0(), pMenu);
    FUN_004f2c10((int)FUN_004f83d0());
    FUN_004f2c30((int)pMenu);
    FUN_004e7780(CFrontend::GetTextString(0xb));
    g_unk0x00819048--;
    Menu_SetNextAction((int)FUN_004f83c0());
}

// Item callback of a saved profile: loads it for the current player.
// FUNCTION: CMR2 0x004f27d0
void FUN_004f27d0(Menu *pMenu, int param)
{
    int profile;

    profile = FUN_004ec090(pMenu->cursor - 2);
    FUN_004f2bf0(0);
    FUN_004eb860((CGameInfo::FUN_00405d70() & 0xff) - (g_unk0x00819048 & 0xff), profile);
    FUN_004eb000(CGameInfo::FUN_00405d70() - g_unk0x00819048, 0);
    Menu_SetNextAction((int)FUN_004f8490());
    g_unk0x00819048--;
}

// Entering the profile menu (back: undoes the profile created by "new").
// Lists the free saved profiles and puts the cursor on the first one.
// TODO: CMR2 0x004f2620 (implemented, match 82%)
void FUN_004f2620(Menu *pMenu, char back)
{
    MenuItem *pItem;
    char *pText;
    int i;

    if (back != 0) {
        if (g_unk0x00819879 != 0) {
            FUN_004eb000(0, 0);
            FUN_004ebf20(0);
        }
        FUN_004ebe80(0);
    }
    g_unk0x00819879 = 0;
    FUN_004e7770(1);
    FUN_004ea8c0(1);
    g_unk0x00819048 = 1;
    g_unk0x00819870 = (int)pMenu->pParent;
    CSound::FUN_004a28c0();
    i = 0;
    pText = g_profileEntryTexts[0];
    pItem = &pMenu->items[2];
    do {
        if (i < FUN_004ec020()) {
            sprintf(pText, CFrontend::GetTextString(0x17f), FUN_004ec110(i));
            pItem->enabled = 1;
            pItem->visible = 1;
            pItem->stringId = (int)pText;
        } else {
            pItem->enabled = 0;
            pItem->visible = 0;
        }
        pText += 40;
        i++;
        pItem++;
    } while (pText < g_profileEntryTexts[4]);
    if (FUN_004ec020() > 0) {
        pMenu->cursor = 2;
        return;
    }
    pMenu->cursor = 0;
}

// Draw callback of the profile menu.
// FUNCTION: CMR2 0x004d9a40
void FUN_004d9a40(Menu *pMenu)
{
    FrontendDraw_PlayTime();
    FrontendDraw_MenuPath(pMenu, (int)(g_pGraphics->resX * 24) / 640, (int)(g_pGraphics->resY * 38) / 480, 1, 1, NULL, 1);
    FrontendDraw_MenuList(pMenu, NULL, -1, -1, 0, 1);
    FrontendDraw_Carousel(FUN_004f8410(), 0, NULL);
    FrontendDraw_HelpText(CFrontend::GetTextString(0x57), 1);
}

// Leaving the profile menu: once every player has a profile, goes on.
// FUNCTION: CMR2 0x004f07e0
void FUN_004f07e0(Menu *pMenu, char back)
{
    FUN_004ea480(0);
    if (back != 0 && g_unk0x00819048 == CGameInfo::FUN_00405d70()) {
        g_unk0x00819048 = CGameInfo::FUN_00405d70();
        Menu_SetNextAction(g_unk0x00819870);
    }
}

// Draw callback of the options menu.
// FUNCTION: CMR2 0x004e2590
void FUN_004e2590(Menu *pMenu)
{
    FrontendDraw_PlayTime();
    FrontendDraw_MenuPath(pMenu, (int)(g_pGraphics->resX * 24) / 640, (int)(g_pGraphics->resY * 38) / 480, 1, 1, NULL, -1);
    FrontendDraw_MenuList(pMenu, NULL, -1, -1, 0, 1);
    FrontendDraw_Carousel(FUN_004f8410(), 0, NULL);
}

// Entering the options menu: the cheats entry is only there once something
// is unlocked or a cheat is on.
// FUNCTION: CMR2 0x004f29b0
void FUN_004f29b0(Menu *pMenu, int param)
{
    FUN_004ea480(0);
    if (CGameInfo::FUN_00406360(7) || CGameInfo::FUN_00406360(1) || CGameInfo::FUN_00406360(2)
        || CGameInfo::FUN_00406360(3) || CGameInfo::FUN_00406360(4) || CGameInfo::FUN_00406360(5)
        || CGameInfo::FUN_00406360(6) || CGameInfo::FUN_00406360(0) || CGameInfo::FUN_00406410(4)
        || CGameInfo::FUN_00406410(5) || CGameInfo::FUN_00406410(6) || CGameInfo::FUN_00406410(7)
        || CGameInfo::FUN_00406410(8) || CGameInfo::FUN_00406410(9) || CGameInfo::FUN_00406410(10)
        || CGameInfo::FUN_00406410(0xb)) {
        Menu_GetItem(pMenu, 1000)->enabled = 1;
        Menu_GetItem(pMenu, 1000)->visible = 1;
    } else {
        Menu_GetItem(pMenu, 1000)->enabled = 0;
        Menu_GetItem(pMenu, 1000)->visible = 0;
    }
}

void FUN_004eab80(void);
void FUN_004eaba0(void);
void FUN_004eab40(int bit, int value);

// Entering the cheats menu: each of the 8 entries is available when its
// extra is unlocked or its cheat code was typed, and shows its state.
// FUNCTION: CMR2 0x004f28c0
void FUN_004f28c0(Menu *pMenu, int param)
{
    MenuItem *pItem;
    bool cheat;
    int i;

    FUN_004eab80();
    i = 0;
    pItem = pMenu->items;
    do {
        pItem->enabled = 0;
        pItem->visible = 1;
        pItem->max = 0;
        if (i % 2 == 0) {
            cheat = CGameInfo::FUN_00406410(i / 2 + 4);
            if (CGameInfo::FUN_00406360(i) != 0 || cheat) {
                pItem->enabled = 1;
                if (CGameInfo::FUN_004063d0(i))
                    pItem->max = 1;
            }
        } else {
            cheat = CGameInfo::FUN_00406410(i / 2 + 8);
            if (CGameInfo::FUN_00406360(i) != 0 || cheat) {
                pItem->enabled = 1;
                if (CGameInfo::FUN_004063d0(i))
                    pItem->max = 1;
            }
        }
        i++;
        pItem++;
    } while (i < 8);
}

// Leaving the cheats menu: stores the on/off state of each cheat (back:
// restores them first).
// FUNCTION: CMR2 0x004f2970
void FUN_004f2970(Menu *pMenu, char back)
{
    int i;

    if (back != 0)
        FUN_004eaba0();
    for (i = 0; i < 8; i++)
        FUN_004eab40(i, pMenu->items[i].max);
}

// Item callback: back to the parent menu.
// FUNCTION: CMR2 0x004f3a50
void FUN_004f3a50(Menu *pMenu, int param)
{
    Menu_SetNextAction((int)pMenu->pParent);
}

// GLOBAL: CMR2 0x00524dcc
char g_strLockedCheat[4] = "...";

// Draw callback of the cheats menu: name and on/off of each cheat, "..."
// for the ones not available yet.
// TODO: CMR2 0x004ded80 (implemented, match 56%)
void FUN_004ded80(Menu *pMenu)
{
    short icon[4];
    BYTE *pShadow;
    BYTE *pColour;
    BYTE *pRow;
    Texture *pTexture;
    MenuItem *pItem;
    int maxWidth;
    int width;
    int count;
    int x0;
    int x;
    short y0;
    int y;
    int i;

    icon[1] = 0;
    maxWidth = 0;
    icon[0] = (int)(g_pGraphics->resX * 100) / 640;
    icon[2] = CFrontend::m_pAr640ATexture->width;
    icon[3] = CFrontend::m_pAr640ATexture->height;
    for (i = 0; i < 8; i++) {
        width = Font_GetTextWidth(1, (BYTE *)CFrontend::GetTextString(i + 0x149));
        if (maxWidth < width)
            maxWidth = width;
    }
    FrontendDraw_PlayTime();
    FrontendDraw_MenuPath(pMenu, (int)(g_pGraphics->resX * 24) / 640, (int)(g_pGraphics->resY * 38) / 480, 1, 2, NULL, -1);
    count = 0;
    for (i = pMenu->itemCount, pItem = pMenu->items; i > 0; i--, pItem++) {
        if (pItem->visible)
            count++;
    }
    x0 = (int)(g_pGraphics->resX * 0x7a) / 640;
    y0 = (short)(((int)(g_pGraphics->resY * 8) / 480 + (int)(g_pGraphics->resY * 38) / 480 + (int)(g_pGraphics->resY * 384) / 480) / 2)
         - (short)((int)(g_pGraphics->resY * 36) / 480 * count / 2);
    if (pMenu->cursor == 0) {
        pColour = g_colourWhite0x00524968;
        pShadow = g_colourShadowWhite0x00524974;
    } else if (!pMenu->items[0].enabled) {
        pColour = g_colourDim0x00524970;
        pShadow = g_colourShadowDim0x0052497c;
    } else {
        pColour = g_colourText0x0052496c;
        pShadow = g_colourShadowText0x00524978;
    }
    g_unk0x008189a8[0] = (int)(g_pGraphics->resX * 99) / 640;
    g_unk0x008189a8[3] = 1;
    g_unk0x008189a8[2] = (int)(g_pGraphics->resX * 282) / 640;
    g_unk0x008189a8[1] = y0;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, pShadow, 1);
    g_unk0x008189a8[1]++;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, pColour, 1);
    for (i = 0; i < count; i++) {
        pItem = &pMenu->items[i];
        icon[1] = (int)(g_pGraphics->resY * 20) / 480 + y0 + (int)(g_pGraphics->resY * 36) / 480 * (short)i
                  - CFrontend::m_pAr640ATexture->height / 2;
        if (pMenu->cursor == i) {
            pRow = g_colourWhite0x00524968;
            pTexture = CFrontend::m_pAr640ATexture;
        } else {
            pRow = g_colourText0x0052496c;
            pTexture = CFrontend::m_pAr640DTexture;
            if (!pItem->enabled)
                pRow = g_colourDim0x00524970;
        }
        Sprite_Queue((SpriteRect *)&pTexture->field_0x11c, (SpriteRect *)icon, pTexture, 1, 0, NULL, NULL, pRow, 8);
        y = (short)((int)(g_pGraphics->resY * 24) / 480 + g_unk0x008189a8[1]);
        if (!pItem->enabled) {
            Font_DrawText(1, g_strLockedCheat, x0, y, (int *)pRow, 0x11);
        } else {
            Font_DrawText(1, CFrontend::GetTextString(pItem->id), x0, y, (int *)pRow, 0x11);
            x = (int)(g_pGraphics->resX * 15) / 640 + maxWidth + x0;
            Font_DrawText(1, CFrontend::GetTextString(0x133), x, y,
                          (int *)(pItem->max == 0 ? g_colourWhite0x00524968 : g_colourText0x0052496c), 0x11);
            x = (int)(g_pGraphics->resX * 25) / 640 + maxWidth + Font_GetTextWidth(1, (BYTE *)CFrontend::GetTextString(0x133)) + x0;
            Font_DrawText(1, CFrontend::GetTextString(0x134), x, y,
                          (int *)(pItem->max == 0 ? g_colourText0x0052496c : g_colourWhite0x00524968), 0x11);
        }
        if (pMenu->cursor == i || pMenu->cursor == i + 1) {
            pColour = g_colourWhite0x00524968;
            pShadow = g_colourShadowWhite0x00524974;
        } else if (!pItem->enabled && !pItem[1].enabled) {
            pColour = g_colourDim0x00524970;
            pShadow = g_colourShadowDim0x0052497c;
        } else {
            pColour = g_colourText0x0052496c;
            pShadow = g_colourShadowText0x00524978;
        }
        g_unk0x008189a8[1] = (short)((int)(g_pGraphics->resY * 36) / 480) * ((short)i + 1) + y0;
        Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, pShadow, 1);
        g_unk0x008189a8[1]++;
        Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, pColour, 1);
    }
    FrontendDraw_Carousel(FUN_004f8410(), 0, NULL);
}

void FUN_004f4d80(void);

// Entering the display device menu: one entry per device, the current one
// under the cursor, and only the usable ones enabled.
// TODO: CMR2 0x004f21c0 (implemented, match 86%)
void FUN_004f21c0(Menu *pMenu, int param)
{
    MenuItem *pItem;
    unsigned int i;

    pMenu->itemCount = CGraphics::FUN_004a8be0();
    pMenu->cursor = CGameInfo::FUN_00405bd0();
    i = 0;
    if (CGraphics::FUN_004a8be0() != 0) {
        pItem = pMenu->items;
        do {
            if (CGraphics::FUN_004a96c0(i) == 0)
                pItem->enabled = 0;
            else
                pItem->enabled = 1;
            i++;
            pItem++;
        } while (i < (unsigned int)CGraphics::FUN_004a8be0());
    }
}

// Item callback of a display device: switches to it, re-creating the
// display, the textures and the fonts with the default video settings.
// FUNCTION: CMR2 0x004f2210
void FUN_004f2210(Menu *pMenu, int param)
{
    CGameInfo::FUN_00405be0(pMenu->cursor);
    CGraphics::FUN_004a8bd0(pMenu->cursor);
    CMain::FUN_004a9a50(1);
    CGraphics::FUN_004a78a0(g_pGraphics->resX, g_pGraphics->resY, g_pGraphics->depth, CGameInfo::FUN_00405bd0(),
                            CGraphics::FUN_004a8d80());
    CGameInfo::SetScreenWidth(g_pGraphics->resX);
    CGameInfo::SetScreenHeight(g_pGraphics->resY);
    CGameInfo::SetColourDepth(g_pGraphics->depth);
    CFrontend::FUN_004d21e0();
    Graphics_ReloadAllTextures();
    FUN_004f4d80();
    FUN_004ef190();
    CMain::FUN_004a9a50(0);
    CGameInfo::FUN_00405b60(0);
    g_pGraphics->field913_0x3bc &= ~8;
    g_pGraphics->field913_0x3bc &= ~0x10;
    g_pGraphics->field913_0x3bc &= ~0x80;
    CGameInfo::FUN_00405b80(0);
    g_pGraphics->field913_0x3bc &= ~0x20;
    CGameInfo::FUN_00405bb0(0);
    g_pGraphics->field913_0x3bc &= ~0x40;
    CGameInfo::FUN_00405c80(0);
    g_pGraphics->field913_0x3bc &= ~1;
    g_pGraphics->field913_0x3bc &= ~2;
    CGameInfo::FUN_00405cb0(1);
    g_pGraphics->field917_0x3c0 = 1;
    CGameInfo::FUN_00405ce0(1);
    CGameInfo::FUN_00405d20(1);
    CGameInfo::FUN_00405d40(2);
    Menu_SetNextAction((int)pMenu->pParent);
}

// GLOBAL: CMR2 0x005250a4
char g_strTwoDigits[8] = "%.2d";
// Gearbox of a record: manual / automatic.
// GLOBAL: CMR2 0x00524db4
char g_strGearboxManual[4] = "MT";
// GLOBAL: CMR2 0x00524db8
char g_strGearboxAuto[4] = "AT";

extern BYTE g_colourTitle0x00524984[4];

// Draw callback of the high score pages: title of the table shown (cycled
// by FUN_004f3a90), the column headers and the 5 entries (position, name,
// car, gearbox and two numbers).
// TODO: CMR2 0x004e3340 (implemented, match 83%)
void FUN_004e3340(Menu *pMenu)
{
    GameInfo0xa4SubStruct12 *pEntry;
    int table;
    int title;
    int y0;
    int y;
    int offset;
    int i;

    table = FUN_004f3a60();
    FrontendDraw_PlayTime();
    FrontendDraw_MenuPath(pMenu, (int)(g_pGraphics->resX * 24) / 640, (int)(g_pGraphics->resY * 38) / 480, 1, 2, NULL, -1);
    if (table == 0)
        title = 0x167;
    else if (table == 1)
        title = 0x168;
    else if (table == 2)
        title = 0x169;
    else
        goto header;
    Font_DrawText(2, CFrontend::GetTextString(title), (int)g_pGraphics->resX / 2, (int)(g_pGraphics->resY * 100) / 480,
                  (int *)g_colourWhite0x00524968, 10);
header:
    g_unk0x008189a8[0] = (int)(g_pGraphics->resX * 30) / 640;
    g_unk0x008189a8[1] = (int)(g_pGraphics->resY * 200) / 480 - (int)(g_pGraphics->resY * 25) / 480;
    g_unk0x008189a8[2] = (int)(g_pGraphics->resX * 640) / 640 + (int)(g_pGraphics->resX * 30) / 640 * -2;
    g_unk0x008189a8[3] = (int)(g_pGraphics->resY * 25) / 480;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, g_colourShadowText0x00524978, 4);
    y0 = (int)(g_pGraphics->resY * 19) / 480 + ((int)(g_pGraphics->resY * 200) / 480 - (int)(g_pGraphics->resY * 25) / 480);
    Font_DrawText(0, CFrontend::GetTextString(0x16a), (int)(g_pGraphics->resX * 150) / 640, y0, (int *)g_colourTitle0x00524984, 0x12);
    Font_DrawText(0, CFrontend::GetTextString(0x16b), (int)(g_pGraphics->resX * 290) / 640, y0, (int *)g_colourTitle0x00524984, 0x12);
    Font_DrawText(0, CFrontend::GetTextString(0x16c), (int)(g_pGraphics->resX * 430) / 640, y0, (int *)g_colourTitle0x00524984, 0x12);
    Font_DrawText(0, CFrontend::GetTextString(0x16d), (int)(g_pGraphics->resX * 485) / 640, y0, (int *)g_colourTitle0x00524984, 0x12);
    Font_DrawText(0, CFrontend::GetTextString(0x16e), (int)(g_pGraphics->resX * 540) / 640, y0, (int *)g_colourTitle0x00524984, 0x12);
    g_unk0x008189a8[0] = (int)(g_pGraphics->resX * 30) / 640;
    g_unk0x008189a8[1] = (int)(g_pGraphics->resY * 200) / 480;
    g_unk0x008189a8[2] = (int)(g_pGraphics->resX * 640) / 640 + (int)(g_pGraphics->resX * 30) / 640 * -2;
    g_unk0x008189a8[3] = 1;
    offset = table * 0x3c;
    i = 0;
    do {
        y = (int)(g_pGraphics->resY * 19) / 480 + (int)(g_pGraphics->resY * 200) / 480 + (int)(g_pGraphics->resY * 25) / 480 * i;
        pEntry = (GameInfo0xa4SubStruct12 *)((BYTE *)CGameInfo::FUN_00405fe0() + offset);
        sprintf(CFrontend::m_stringDest, g_strTwoDigits, i + 1);
        Font_DrawText(3, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 100) / 640, y, (int *)g_colourText0x0052496c, 0x12);
        Font_DrawText(1, pEntry->ident, (int)(g_pGraphics->resX * 150) / 640, y, (int *)g_colourWhite0x00524968, 0x12);
        Font_DrawText(1, CFrontend::FUN_0040ede0(pEntry->flags & 0x3f), (int)(g_pGraphics->resX * 290) / 640, y,
                      (int *)g_colourWhite0x00524968, 0x12);
        Font_DrawText(1, (pEntry->flags & 0x40) == 0 ? g_strGearboxManual : g_strGearboxAuto,
                      (int)(g_pGraphics->resX * 430) / 640, y, (int *)g_colourWhite0x00524968, 0x12);
        sprintf(CFrontend::m_stringDest, g_strTwoDigits, (pEntry->flags >> 0xe & 0xf) + 1);
        Font_DrawText(3, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 485) / 640, y, (int *)g_colourWhite0x00524968, 0x12);
        sprintf(CFrontend::m_stringDest, g_strTwoDigits, pEntry->flags >> 7 & 0x7f);
        Font_DrawText(3, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 540) / 640, y, (int *)g_colourWhite0x00524968, 0x12);
        g_unk0x008189a8[1] = (short)((int)(g_pGraphics->resY * 200) / 480) + (short)((int)(g_pGraphics->resY * 25) / 480) * ((short)i + 1);
        Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, g_colourText0x0052496c, 1);
        offset += 0xc;
        i++;
    } while (i < 5);
    FrontendDraw_HelpText(CFrontend::GetTextString(0x16f), 1);
}

BYTE *RallyData_FUN_00408cb0(int index);
BYTE *FUN_004f9280(int index);

// Draw callback of the record page: the record of the table shown (car,
// gearbox, two numbers and its time), or "no record".
// TODO: CMR2 0x004e3a80 (implemented, match 80%)
void FUN_004e3a80(Menu *pMenu)
{
    unsigned int *pSecond;
    BYTE *pRecord;
    int table;
    int title;
    int y0;
    int y;
    int offset;

    table = FUN_004f3a60();
    FrontendDraw_PlayTime();
    FrontendDraw_MenuTitle(pMenu);
    if (table == 0)
        title = 0x167;
    else if (table == 1)
        title = 0x168;
    else if (table == 2)
        title = 0x169;
    else
        goto header;
    Font_DrawText(2, CFrontend::GetTextString(title), (int)g_pGraphics->resX / 2, (int)(g_pGraphics->resY * 100) / 480,
                  (int *)g_colourWhite0x00524968, 10);
header:
    g_unk0x008189a8[0] = (int)(g_pGraphics->resX * 30) / 640;
    g_unk0x008189a8[1] = (int)(g_pGraphics->resY * 200) / 480 - (int)(g_pGraphics->resY * 25) / 480;
    g_unk0x008189a8[2] = (int)(g_pGraphics->resX * 640) / 640 + (int)(g_pGraphics->resX * 30) / 640 * -2;
    g_unk0x008189a8[3] = (int)(g_pGraphics->resY * 25) / 480;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, g_colourShadowText0x00524978, 4);
    y0 = (int)(g_pGraphics->resY * 19) / 480 + ((int)(g_pGraphics->resY * 200) / 480 - (int)(g_pGraphics->resY * 25) / 480);
    Font_DrawText(0, CFrontend::GetTextString(0x16b), (int)(g_pGraphics->resX * 180) / 640, y0, (int *)g_colourTitle0x00524984, 0x12);
    Font_DrawText(0, CFrontend::GetTextString(0x16c), (int)(g_pGraphics->resX * 310) / 640, y0, (int *)g_colourTitle0x00524984, 0x12);
    Font_DrawText(0, CFrontend::GetTextString(0x16d), (int)(g_pGraphics->resX * 370) / 640, y0, (int *)g_colourTitle0x00524984, 0x12);
    Font_DrawText(0, CFrontend::GetTextString(0x16e), (int)(g_pGraphics->resX * 435) / 640, y0, (int *)g_colourTitle0x00524984, 0x12);
    Font_DrawText(0, CFrontend::GetTextString(0x170), (int)(g_pGraphics->resX * 520) / 640, y0, (int *)g_colourTitle0x00524984, 0x12);
    g_unk0x008189a8[0] = (int)(g_pGraphics->resX * 30) / 640;
    g_unk0x008189a8[1] = (int)(g_pGraphics->resY * 200) / 480;
    g_unk0x008189a8[3] = 1;
    g_unk0x008189a8[2] = (int)(g_pGraphics->resX * 640) / 640 + (int)(g_pGraphics->resX * 30) / 640 * -2;
    y = (int)(g_pGraphics->resY * 19) / 480 + (int)(g_pGraphics->resY * 200) / 480;
    offset = table * 0x10;
    if ((RallyData_FUN_00408cb0(0)[offset] & 0x80) == 0) {
        Font_DrawText(1, CFrontend::GetTextString(0x171), (int)g_pGraphics->resX / 2, y, (int *)g_colourWhite0x00524968, 0x12);
    } else {
        pSecond = (unsigned int *)(RallyData_FUN_00408cb0(0) + offset + 4);
        pRecord = RallyData_FUN_00408cb0(0) + offset;
        Font_DrawText(1, CFrontend::FUN_0040ede0(*(unsigned int *)pRecord & 0x3f), (int)(g_pGraphics->resX * 180) / 640, y,
                      (int *)g_colourWhite0x00524968, 0x12);
        Font_DrawText(1, (*(unsigned int *)pRecord & 0x40) == 0 ? g_strGearboxManual : g_strGearboxAuto,
                      (int)(g_pGraphics->resX * 310) / 640, y, (int *)g_colourWhite0x00524968, 0x12);
        sprintf(CFrontend::m_stringDest, g_strTwoDigits, (*pSecond & 0xf) + 1);
        Font_DrawText(3, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 370) / 640, y, (int *)g_colourWhite0x00524968, 0x12);
        sprintf(CFrontend::m_stringDest, g_strTwoDigits, *pSecond >> 6 & 0xff);
        Font_DrawText(3, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 435) / 640, y, (int *)g_colourWhite0x00524968, 0x12);
        Font_DrawText(0, (char *)FUN_004f9280(table), (int)(g_pGraphics->resX * 520) / 640, y, (int *)g_colourWhite0x00524968, 0x12);
    }
    g_unk0x008189a8[1] = (short)((int)(g_pGraphics->resY * 25) / 480) + (short)((int)(g_pGraphics->resY * 200) / 480);
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, g_colourText0x0052496c, 1);
    FrontendDraw_HelpText(CFrontend::GetTextString(0x16f), 1);
}

extern char g_loadRecordTimeFormat[];
BYTE *FUN_004f9240(int row, int column);

// Draw callback of the stage records page of a rally: for each stage (10,
// or 11 for every second rally) the record car, gearbox, time and holder.
// TODO: CMR2 0x004e5630 (implemented, match 65%)
void FUN_004e5630(Menu *pMenu)
{
    unsigned int *pRecord;
    unsigned int time;
    BYTE *pRecords;
    char rally;
    int odd;
    int y0;
    int y;
    int i;

    FUN_004f3a60();
    FrontendDraw_PlayTime();
    FrontendDraw_MenuTitle(pMenu);
    odd = pMenu->cursor % 2;
    g_unk0x008189a8[0] = (int)(g_pGraphics->resX * 30) / 640;
    g_unk0x008189a8[1] = (int)(g_pGraphics->resY * 90) / 480 - (int)(g_pGraphics->resY * 25) / 480;
    g_unk0x008189a8[2] = (int)(g_pGraphics->resX * 640) / 640 + (int)(g_pGraphics->resX * 30) / 640 * -2;
    g_unk0x008189a8[3] = (int)(g_pGraphics->resY * 25) / 480;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, g_colourShadowText0x00524978, 4);
    y0 = (int)(g_pGraphics->resY * 19) / 480 + ((int)(g_pGraphics->resY * 90) / 480 - (int)(g_pGraphics->resY * 25) / 480);
    Font_DrawText(0, CFrontend::GetTextString(0x17d), (int)(g_pGraphics->resX * 100) / 640, y0, (int *)g_colourTitle0x00524984, 0x12);
    Font_DrawText(0, CFrontend::GetTextString(0x16b), (int)(g_pGraphics->resX * 230) / 640, y0, (int *)g_colourTitle0x00524984, 0x12);
    Font_DrawText(0, CFrontend::GetTextString(0x16c), (int)(g_pGraphics->resX * 355) / 640, y0, (int *)g_colourTitle0x00524984, 0x12);
    Font_DrawText(0, CFrontend::GetTextString(0x174), (int)(g_pGraphics->resX * 415) / 640, y0, (int *)g_colourTitle0x00524984, 0x12);
    Font_DrawText(0, CFrontend::GetTextString(0x170), (int)(g_pGraphics->resX * 520) / 640, y0, (int *)g_colourTitle0x00524984, 0x12);
    g_unk0x008189a8[0] = (int)(g_pGraphics->resX * 30) / 640;
    g_unk0x008189a8[1] = (int)(g_pGraphics->resY * 90) / 480;
    g_unk0x008189a8[3] = 1;
    g_unk0x008189a8[2] = (int)(g_pGraphics->resX * 640) / 640 + (int)(g_pGraphics->resX * 30) / 640 * -2;
    i = 0;
    do {
        y = (int)(g_pGraphics->resY * 19) / 480 + (int)(g_pGraphics->resY * 90) / 480 + (int)(g_pGraphics->resY * 25) / 480 * i;
        pRecords = RallyData_FUN_00408cb0(0);
        rally = pMenu->cursor;
        pRecord = (unsigned int *)(RallyData_FUN_00408cb0(0) + 0x150 + (i + pMenu->cursor * 12) * 8);
        sprintf(CFrontend::m_stringDest, g_strTwoDigits, i + 1);
        Font_DrawText(3, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 100) / 640, y, (int *)g_colourText0x0052496c, 0x12);
        if ((*pRecord & 0x80) == 0) {
            Font_DrawText(1, CFrontend::GetTextString(0x171), (int)g_pGraphics->resX / 2, y, (int *)g_colourWhite0x00524968, 0x12);
        } else {
            Font_DrawText(1, CFrontend::FUN_0040ede0(*pRecord & 0x3f), (int)(g_pGraphics->resX * 230) / 640, y,
                          (int *)g_colourWhite0x00524968, 0x12);
            Font_DrawText(1, (*pRecord & 0x40) == 0 ? g_strGearboxManual : g_strGearboxAuto,
                          (int)(g_pGraphics->resX * 355) / 640, y, (int *)g_colourWhite0x00524968, 0x12);
            time = *(unsigned int *)(pRecords + 0x154 + (i + rally * 12) * 8);
            sprintf(CFrontend::m_stringDest, g_loadRecordTimeFormat, time / 6000, time / 100 % 60, time % 100);
            Font_DrawText(3, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 415) / 640, y, (int *)g_colourWhite0x00524968, 0x12);
            Font_DrawText(0, (char *)FUN_004f9240(pMenu->cursor, i), (int)(g_pGraphics->resX * 520) / 640, y,
                          (int *)g_colourWhite0x00524968, 0x12);
        }
        g_unk0x008189a8[1] = (short)((int)(g_pGraphics->resY * 90) / 480) + (short)((int)(g_pGraphics->resY * 25) / 480) * ((short)i + 1);
        Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, g_colourText0x0052496c, 1);
        i++;
    } while (i < (odd != 0) + 10);
    FrontendDraw_ScrollerRow(FUN_004f2550(), 1);
    FrontendDraw_HelpText(CFrontend::GetTextString(0x172), 1);
}

// Draw callback of the display device menu: one row per device name.
// FUNCTION: CMR2 0x004e1920
void FUN_004e1920(Menu *pMenu)
{
    char name[80];
    char description[80];
    short icon[4];
    BYTE *pShadow;
    BYTE *pColour;
    Texture *pTexture;
    short y0;
    int i;

    icon[1] = 0;
    icon[0] = (int)(g_pGraphics->resX * 100) / 640;
    icon[2] = CFrontend::m_pAr640ATexture->width;
    icon[3] = CFrontend::m_pAr640ATexture->height;
    y0 = (short)(((int)(g_pGraphics->resY * 8) / 480 + (int)(g_pGraphics->resY * 38) / 480 + (int)(g_pGraphics->resY * 384) / 480) / 2)
         - (short)((int)(g_pGraphics->resY * 36) / 480 * pMenu->itemCount / 2);
    FrontendDraw_PlayTime();
    FrontendDraw_MenuPath(pMenu, (int)(g_pGraphics->resX * 24) / 640, (int)(g_pGraphics->resY * 38) / 480, 1, 2, NULL, -1);
    if (pMenu->cursor == 0) {
        pColour = g_colourWhite0x00524968;
        pShadow = g_colourShadowWhite0x00524974;
    } else {
        pColour = g_colourText0x0052496c;
        pShadow = g_colourShadowText0x00524978;
    }
    g_unk0x008189a8[0] = (int)(g_pGraphics->resX * 99) / 640;
    g_unk0x008189a8[3] = 1;
    g_unk0x008189a8[2] = (int)(g_pGraphics->resX * 282) / 640;
    g_unk0x008189a8[1] = y0;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, pShadow, 1);
    g_unk0x008189a8[1]++;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, pColour, 1);
    for (i = 0; i < pMenu->itemCount; i++) {
        icon[1] = (int)(g_pGraphics->resY * 2) / 480 + (int)(g_pGraphics->resY * 18) / 480 + y0
                  + ((short)((int)(g_pGraphics->resY * 36) / 480) * (short)i - CFrontend::m_pAr640ATexture->height / 2);
        if (pMenu->cursor == i) {
            pColour = g_colourWhite0x00524968;
            pTexture = CFrontend::m_pAr640ATexture;
        } else {
            pColour = g_colourText0x0052496c;
            pTexture = CFrontend::m_pAr640DTexture;
            if (!pMenu->items[i].enabled)
                pColour = g_colourDim0x00524970;
        }
        Sprite_Queue((SpriteRect *)&pTexture->field_0x11c, (SpriteRect *)icon, pTexture, 1, 0, NULL, NULL, pColour, 8);
        CGraphics::GetDisplayDeviceNames(i, description, name);
        sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue, description);
        Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x7a) / 640,
                      (short)((int)(g_pGraphics->resY * 24) / 480 + g_unk0x008189a8[1]), (int *)pColour, 0x11);
        if (pMenu->cursor == i + 1 || pMenu->cursor == i) {
            pColour = g_colourWhite0x00524968;
            pShadow = g_colourShadowWhite0x00524974;
        } else {
            pColour = g_colourText0x0052496c;
            pShadow = g_colourShadowText0x00524978;
        }
        g_unk0x008189a8[1] = (short)((int)(g_pGraphics->resY * 36) / 480) * ((short)i + 1) + y0;
        Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, pShadow, 1);
        g_unk0x008189a8[1]++;
        Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, pColour, 1);
    }
    FrontendDraw_Carousel(FUN_004f8410(), 0, NULL);
}

// FUNCTION: CMR2 0x004f92e0
void FUN_004f92e0(int value)
{
    g_unk0x00826138 = value;
}

// FUNCTION: CMR2 0x004f92f0
int FUN_004f92f0(void)
{
    return g_unk0x00826138;
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
