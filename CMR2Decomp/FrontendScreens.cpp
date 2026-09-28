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
#include "NetPlayers.h"
#include "FileBuffer.h"

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
// Per-screen colour/palette ids, indexed by the screen number in field +7.
// GLOBAL: CMR2 0x008196e8
int g_unk0x008196e8[23];
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
// GLOBAL: CMR2 0x00818ce0
int g_unk0x00818ce0;
// GLOBAL: CMR2 0x00818ce8
int g_unk0x00818ce8;
// GLOBAL: CMR2 0x00818d00
int g_unk0x00818d00;
// Filtered stage/route ids, one int per entry (up to 0x16).
// GLOBAL: CMR2 0x00818d18
int g_unk0x00818d18[0x16];
// Index of the route/stage the setup screens currently point at.
// GLOBAL: CMR2 0x00818d70
int g_unk0x00818d70;
// Route ids of the four selected stages of the outgoing setup packet.
// GLOBAL: CMR2 0x00818d74
int g_unk0x00818d74[4];
// GLOBAL: CMR2 0x00818ebc
char g_unk0x00818ebc[20];
// GLOBAL: CMR2 0x00818ed0
BYTE g_unk0x00818ed0;
// GLOBAL: CMR2 0x00818d04
BYTE g_unk0x00818d04;
// GLOBAL: CMR2 0x00818da8
char g_unk0x00818da8[256];
// GLOBAL: CMR2 0x00818ed4
int g_unk0x00818ed4[8];
// GLOBAL: CMR2 0x00818ef4
BYTE g_unk0x00818ef4;
// GLOBAL: CMR2 0x00818ef8
char g_unk0x00818ef8[24];
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
// GLOBAL: CMR2 0x00819044
int g_unk0x00819044;
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
// GLOBAL: CMR2 0x0081975c
int g_unk0x0081975c;
// GLOBAL: CMR2 0x00819870
int g_unk0x00819870;
// GLOBAL: CMR2 0x00819878
BYTE g_unk0x00819878;
// GLOBAL: CMR2 0x00819884
int g_unk0x00819884;
// GLOBAL: CMR2 0x00826138
int g_unk0x00826138;
// GLOBAL: CMR2 0x0081903c
unsigned int g_unk0x0081903c;
// GLOBAL: CMR2 0x00819754
unsigned int g_unk0x00819754;
// GLOBAL: CMR2 0x00819860
unsigned int g_unk0x00819860;
// GLOBAL: CMR2 0x00819868
int g_unk0x00819868;

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

void FUN_004d27e0(short *pRect, BYTE *pColour);

// Lays a matrix-textured rect at (param_1, param_2) scaled to the screen and
// queues it through FUN_004d27e0; for param_3 in 1..3 the rect is replaced by
// the medal texture of that place, centred on the same point.
// FUNCTION: CMR2 0x004d4f90
void FUN_004d4f90(int param_1, int param_2, int param_3)
{
    Texture *pTexture;

    g_unk0x008189a8[1] = param_2;
    g_unk0x008189a8[0] = param_1 - (int)(g_pGraphics->resX * 0x30) / 0x280 / 2;
    g_unk0x008189a8[2] = (int)(g_pGraphics->resX * 0x30) / 640;
    g_unk0x008189a8[3] = (int)(g_pGraphics->resY * 0x1c) / 480;
    FUN_004d27e0(g_unk0x008189a8, (BYTE *)g_colourText0x0052496c);
    if (param_3 >= 1 && param_3 <= 3) {
        pTexture = (&CFrontend::m_pLgMatrixTexture)[param_3];
        g_unk0x008189a8[1] = param_2;
        g_unk0x008189a8[0] = param_1 - pTexture->width / 2;
        g_unk0x008189a8[2] = pTexture->width;
        g_unk0x008189a8[3] = pTexture->height;
        Sprite_Queue((SpriteRect *)&pTexture->field_0x11c, (SpriteRect *)g_unk0x008189a8, pTexture, 1, 0, NULL, NULL,
                     (BYTE *)g_colourWhite0x00524968, 8);
    }
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
// match 85%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004d5ca0
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

// The four stage-map cell colours used by the map blitter below (index 0 is
// the transparent background).
// GLOBAL: CMR2 0x00524b90
BYTE g_palette0x00524b90[4][4] = { { 0x64, 0x7c, 0xa1, 0xff },
                                   { 0xff, 0xff, 0xff, 0xff },
                                   { 0xff, 0x00, 0x00, 0xff },
                                   { 0x00, 0xff, 0x00, 0xff } };

// Blits the stage map of the given rally (or championship when param_2 is set)
// as a 36x36 grid of 2-bit cells, each cell queued with the colour of its
// value; leaves g_unk0x008189a8 scaled to the screen.
// match 42%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// logic verified against the dump; the original keeps the texture in EDI and
// the cell value in BL/param_1 while this build uses EBP/ESI (register
// allocation), so the body is instruction-for-instruction different.
// FUNCTION: CMR2 0x004d5de0
void FUN_004d5de0(unsigned int param_1, BYTE param_2)
{
    BYTE *pMap;
    unsigned int which;
    int i;
    int cols;
    int rows;

    g_unk0x008189a8[2] = CFrontend::m_pSmMatrixTexture->width;
    g_unk0x008189a8[3] = CFrontend::m_pSmMatrixTexture->height;
    if (param_2 == 0)
        which = RallyDataCountryIndex() & 0xff;
    else
        which = 8;
    pMap = (BYTE *)g_dmdFiles[which][param_1];
    if (pMap != NULL) {
        rows = 0x24;
        g_unk0x008189a8[1] = (int)(g_pGraphics->resY * 0x4b) / 480;
        do {
            cols = 9;
            g_unk0x008189a8[0] = (int)(g_pGraphics->resX * 0x3c) / 640;
            do {
                param_2 = *pMap++;
                for (i = 0; i < 4; i++) {
                    switch (i) {
                    case 0:
                        param_1 = param_2 & 3;
                        break;
                    case 1:
                        param_1 = param_2 >> 2 & 3;
                        break;
                    case 2:
                        param_1 = param_2 >> 4 & 3;
                        break;
                    case 3:
                        param_1 = param_2 >> 6;
                        break;
                    }
                    if (param_1 != 0)
                        Sprite_Queue((SpriteRect *)&CFrontend::m_pSmMatrixTexture->field_0x11c,
                                     (SpriteRect *)g_unk0x008189a8, CFrontend::m_pSmMatrixTexture, 4, 0, NULL, NULL,
                                     g_palette0x00524b90[param_1], 8);
                    g_unk0x008189a8[0] += (int)(g_pGraphics->resX * 8) / 640;
                }
            } while (--cols);
            g_unk0x008189a8[1] += (int)(g_pGraphics->resY * 8) / 480;
        } while (--rows);
    }
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

// Text lines built by the screen code above the title (rally name, date and
// the list of stages); the third is the one built by FUN_004d6a60.
// GLOBAL: CMR2 0x00818368
char g_unk0x00818368[32];
// GLOBAL: CMR2 0x008183cc
char g_unk0x008183cc[32];
// GLOBAL: CMR2 0x00818554
char g_unk0x00818554[32];

// Draws the header of the rally-info screen: the framed title rect, the
// "event: date" lines and the list of stages, all scaled to the screen.
// FUNCTION: CMR2 0x004d65c0
void FUN_004d65c0(void)
{
    g_unk0x008189a8[0] = (int)(g_pGraphics->resX * 0x3c) / 640;
    g_unk0x008189a8[1] = (int)(g_pGraphics->resY * 0x4c) / 480;
    g_unk0x008189a8[2] = g_pGraphics->resX - (int)(g_pGraphics->resX * 0x3c) / 640 * 2;
    g_unk0x008189a8[3] = (int)(g_pGraphics->resY * 100) / 480;
    FUN_004d27e0(g_unk0x008189a8, (BYTE *)g_colourText0x0052496c);
    Font_DrawText(0, CFrontend::GetTextString(0xe4), (int)(g_pGraphics->resX * 0x3c) / 640 + 2,
                  (int)(g_pGraphics->resY * 0x4c) / 480 + 1, (int *)g_colourText0x0052496c, 9);
    Font_DrawText(1, g_unk0x008183cc,
                  (int)(g_pGraphics->resX * 0xdc) / 640 - (int)(g_pGraphics->resX * 10) / 640,
                  (int)(g_pGraphics->resY * 0x23) / 480 + (int)(g_pGraphics->resY * 0x4c) / 480,
                  (int *)g_colourText0x0052496c, 0x14);
    Font_DrawText(1, g_unk0x00818554, (int)(g_pGraphics->resX * 0xdc) / 640,
                  (int)(g_pGraphics->resY * 0x23) / 480 + (int)(g_pGraphics->resY * 0x4c) / 480,
                  (int *)g_colourWhite0x00524968, 0x11);
    Font_DrawText(1, g_unk0x00818368, g_pGraphics->resX - (int)(g_pGraphics->resX * 0x3c) / 640 - 6,
                  (int)(g_pGraphics->resY * 0x23) / 480 + (int)(g_pGraphics->resY * 0x4c) / 480 +
                      (int)(g_pGraphics->resY * 100) / 480 - 4,
                  (int *)g_colourWhite0x00524968, 0x24);
}

// Time-attack style screen: menu path, the title taken from the id of the
// first item and the list of strings selected by that item, plus the
// underline of the highlighted row.
// match 81%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004d9450
void FUN_004d9450(Menu *pMenu)
{
    short rect[4];
    int y;
    int i;
    int x;
    BYTE *pColour;

    rect[1] = 0;
    rect[0] = (int)(g_pGraphics->resX * 100) / 640;
    rect[2] = CFrontend::m_pAr640ATexture->width;
    rect[3] = CFrontend::m_pAr640ATexture->height;
    FrontendDraw_PlayTime();
    FrontendDraw_MenuPath(pMenu, PATH_X(), PATH_Y(), 1, 2, NULL, -1);
    y = ((int)(g_pGraphics->resY * 56) / 480 + (int)(g_pGraphics->resY * 374) / 480) / 2 -
        ((int)(g_pGraphics->resY * 36) / 480 * pMenu->itemCount) / 2;
    g_unk0x008189a8[0] = (int)(g_pGraphics->resX * 99) / 640;
    g_unk0x008189a8[3] = 1;
    g_unk0x008189a8[2] = (int)(g_pGraphics->resX * 282) / 640;
    g_unk0x008189a8[1] = y;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, g_colourShadowWhite0x00524974, 1);
    g_unk0x008189a8[1]++;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, g_colourWhite0x00524968, 1);
    rect[1] = y + (int)(g_pGraphics->resY * 20) / 480 - CFrontend::m_pAr640ATexture->height / 2;
    Sprite_Queue((SpriteRect *)&CFrontend::m_pAr640ATexture->field_0x11c, (SpriteRect *)rect,
                 CFrontend::m_pAr640ATexture, 1, 0, 0, NULL, g_colourWhite0x00524968, 8);
    strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(pMenu->items[0].id));
    Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 122) / 640,
                  (int)(g_pGraphics->resY * 24) / 480 + g_unk0x008189a8[1],
                  (int *)g_colourWhite0x00524968, 0x11);
    for (i = 0; i < pMenu->items[0].min; i++) {
        pColour = g_colourWhite0x00524968;
        if (i != pMenu->items[0].max)
            pColour = g_colourText0x0052496c;
        x = Font_GetTextWidth(1, (BYTE *)CFrontend::m_stringDest);
        x = (int)(g_pGraphics->resX * 10) / 640 + (int)(g_pGraphics->resX * 122) / 640 + x;
        Font_DrawText(1, CFrontend::GetTextString(i + 0x131), x,
                      (int)(g_pGraphics->resY * 24) / 480 + g_unk0x008189a8[1],
                      (int *)pColour, 0x11);
        strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(i + 0x131));
    }
    g_unk0x008189a8[1] = (int)(g_pGraphics->resY * 36) / 480 + y;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, g_colourShadowWhite0x00524974, 1);
    g_unk0x008189a8[1]++;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, g_colourWhite0x00524968, 1);
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

void FUN_004d28c0(short x0, short y0, char *pMap);
extern char g_matrixMaps[11][0xd8];

// Rally/route selection screen: the two breadcrumb lines depend on the mode,
// and behind them is the matrix map of the selected route.
// FUNCTION: CMR2 0x004d9ad0
void FUN_004d9ad0(Menu *pMenu)
{
    char *text[2];
    int cellW;

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
    FrontendDraw_ScrollerRow(FUN_004f2530(), 1);
    cellW = (int)(g_pGraphics->resX * 18) / 640;
    FUN_004d28c0((cellW - (int)(g_pGraphics->resX * 11) / 640) / 2 - cellW * 18 / 2 + (int)g_pGraphics->resX / 2,
                 (int)(g_pGraphics->resY * 100) / 480, g_matrixMaps[pMenu->cursor]);
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

// Draws a settings screen: one row per item (title plus the strings of the
// current value) with a separator line under each, coloured by the cursor.
// match 72%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004e0770
void FUN_004e0770(Menu *pMenu)
{
    short rect[4];
    MenuItem *pItem;
    Texture *pTexture;
    BYTE *pColour;
    BYTE *pSelColour;
    BYTE *pUnselColour;
    BYTE *pLineColour;
    BYTE *pLineShadow;
    int y;
    int x;
    int i;
    int j;
    int index;

    rect[1] = 0;
    rect[0] = (int)(g_pGraphics->resX * 100) / 640;
    rect[2] = CFrontend::m_pAr640ATexture->width;
    rect[3] = CFrontend::m_pAr640ATexture->height;
    y = ((int)(g_pGraphics->resY * 8) / 480 + (int)(g_pGraphics->resY * 38) / 480 +
         (int)(g_pGraphics->resY * 384) / 480) / 2 -
        ((int)(g_pGraphics->resY * 36) / 480 * pMenu->itemCount) / 2;
    FrontendDraw_PlayTime();
    FrontendDraw_MenuPath(pMenu, PATH_X(), PATH_Y(), 1, 2, NULL, -1);
    if (pMenu->cursor == 0) {
        pLineColour = g_colourWhite0x00524968;
        pLineShadow = g_colourShadowWhite0x00524974;
    } else {
        pLineColour = g_colourText0x0052496c;
        pLineShadow = g_colourShadowText0x00524978;
    }
    g_unk0x008189a8[0] = (int)(g_pGraphics->resX * 99) / 640;
    g_unk0x008189a8[3] = 1;
    g_unk0x008189a8[2] = (int)(g_pGraphics->resX * 282) / 640;
    g_unk0x008189a8[1] = y;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, pLineShadow, 1);
    g_unk0x008189a8[1]++;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, pLineColour, 1);
    pItem = pMenu->items;
    for (i = 0; i < pMenu->itemCount; i++) {
        x = (int)(g_pGraphics->resX * 122) / 640;
        rect[1] = y + (int)(g_pGraphics->resY * 2) / 480 + (int)(g_pGraphics->resY * 18) / 480 +
                  ((int)(g_pGraphics->resY * 36) / 480 * i -
                   CFrontend::m_pAr640ATexture->height / 2);
        if (i == pMenu->cursor) {
            pColour = g_colourWhite0x00524968;
            pSelColour = g_colourWhite0x00524968;
            pUnselColour = g_colourText0x0052496c;
            pTexture = CFrontend::m_pAr640ATexture;
        } else {
            pTexture = CFrontend::m_pAr640DTexture;
            if (pItem->enabled) {
                pColour = g_colourText0x0052496c;
                pSelColour = g_colourWhite0x00524968;
                pUnselColour = g_colourText0x0052496c;
            } else {
                pColour = g_colourDim0x00524970;
                pSelColour = g_colourText0x0052496c;
                pUnselColour = g_colourDim0x00524970;
            }
        }
        Sprite_Queue((SpriteRect *)&pTexture->field_0x11c, (SpriteRect *)rect, pTexture, 1, 0, 0,
                     NULL, pColour, 8);
        switch (pItem->value) {
        case 8:
            strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(pItem->id));
            Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 122) / 640,
                          (int)(g_pGraphics->resY * 24) / 480 + g_unk0x008189a8[1], (int *)pColour, 0x11);
            for (j = 0; j < pItem->min; j++) {
                if (j == pItem->max)
                    pColour = pSelColour;
                else
                    pColour = pUnselColour;
                x += (int)(g_pGraphics->resX * 10) / 640 +
                     Font_GetTextWidth(1, (BYTE *)CFrontend::m_stringDest);
                Font_DrawText(1, CFrontend::GetTextString(j + 0x1b9), x,
                              (int)(g_pGraphics->resY * 24) / 480 + g_unk0x008189a8[1], (int *)pColour, 0x11);
                strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(j + 0x1b9));
            }
            break;
        case 9:
            strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(pItem->id));
            Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 122) / 640,
                          (int)(g_pGraphics->resY * 24) / 480 + g_unk0x008189a8[1], (int *)pColour, 0x11);
            index = 0x1b3;
            for (j = 0; j < pItem->min; j++) {
                if (j == pItem->max)
                    pColour = pSelColour;
                else
                    pColour = pUnselColour;
                x += (int)(g_pGraphics->resX * 10) / 640 +
                     Font_GetTextWidth(1, (BYTE *)CFrontend::m_stringDest);
                Font_DrawText(1, CFrontend::GetTextString(index), x,
                              (int)(g_pGraphics->resY * 24) / 480 + g_unk0x008189a8[1], (int *)pColour, 0x11);
                strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(index));
                index--;
            }
            break;
        case 10:
            strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(pItem->id));
            Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 122) / 640,
                          (int)(g_pGraphics->resY * 24) / 480 + g_unk0x008189a8[1], (int *)pColour, 0x11);
            for (j = 0; j < pItem->min; j++) {
                if (CFrontend::FUN_004b7560(0x400) && CFrontend::FUN_004b7590(0x400)) {
                    if (j == pItem->max)
                        pColour = pSelColour;
                    else
                        pColour = pUnselColour;
                } else {
                    pColour = g_colourDim0x00524970;
                    if (j != 0)
                        pColour = g_colourWhite0x00524968;
                }
                x += (int)(g_pGraphics->resX * 10) / 640 +
                     Font_GetTextWidth(1, (BYTE *)CFrontend::m_stringDest);
                Font_DrawText(1, CFrontend::GetTextString(0x1b3 - j), x,
                              (int)(g_pGraphics->resY * 24) / 480 + g_unk0x008189a8[1], (int *)pColour, 0x11);
                strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(0x1b3 - j));
            }
            break;
        case 4:
            strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(0x1d2));
            Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 122) / 640,
                          (int)(g_pGraphics->resY * 24) / 480 + g_unk0x008189a8[1], (int *)pColour, 0x11);
            for (j = 0; j < pItem->min; j++) {
                if (j == pItem->max)
                    pColour = pSelColour;
                else
                    pColour = pUnselColour;
                x += (int)(g_pGraphics->resX * 10) / 640 +
                     Font_GetTextWidth(1, (BYTE *)CFrontend::m_stringDest);
                if (j == 0)
                    index = 0x1d6;
                else if (j == 1)
                    index = 0x1d5;
                else
                    index = 0x133;
                Font_DrawText(1, CFrontend::GetTextString(index), x,
                              (int)(g_pGraphics->resY * 24) / 480 + g_unk0x008189a8[1], (int *)pColour, 0x11);
                strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(index));
            }
            break;
        default:
            Font_DrawText(1, CFrontend::GetTextString(pItem->id), (int)(g_pGraphics->resX * 122) / 640,
                          (int)(g_pGraphics->resY * 24) / 480 + g_unk0x008189a8[1], (int *)pColour, 0x11);
            break;
        }
        if (pMenu->cursor == i + 1 || pMenu->cursor == i) {
            pLineColour = g_colourWhite0x00524968;
            pLineShadow = g_colourShadowWhite0x00524974;
        } else {
            pLineColour = g_colourText0x0052496c;
            pLineShadow = g_colourShadowText0x00524978;
        }
        g_unk0x008189a8[1] = (int)(g_pGraphics->resY * 36) / 480 * (i + 1) + y;
        Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, pLineShadow, 1);
        g_unk0x008189a8[1]++;
        Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, pLineColour, 1);
        pItem++;
    }
    FrontendDraw_Carousel(FUN_004f8410(), 0, NULL);
}
void *FUN_004f4b10(void);
int FUN_004f4b20(void);
void *FUN_004f4b30(void);
int FUN_004f1bf0(void);

// Draws the two columns of text of the scrolling credits screen; when every
// line has scrolled past the top the screen finishes.
// match 84%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004dec30
void FUN_004dec30(Menu *pMenu)
{
    char **ppText;
    unsigned short *pLineY;
    unsigned int elapsed;
    bool allOffScreen;
    int i;

    allOffScreen = true;
    ppText = (char **)FUN_004f4b10();
    pLineY = (unsigned short *)FUN_004f4b30();
    elapsed = (unsigned int)CFrontend::FUN_004d20e0();
    elapsed -= FUN_004f1bf0();
    elapsed /= 30;
    for (i = 0; i < FUN_004f4b20(); i++) {
        ((int *)FUN_004f4b30())[i] = ((int)g_pGraphics->resY * 40) / 480 * i - elapsed + 1
            + FUN_004f25a0() + (int)g_pGraphics->resY + Font_GetLineHeight(2);
        if (((int *)FUN_004f4b30())[i] < (int)g_pGraphics->resY + 100
            && ((int *)FUN_004f4b30())[i] > -100) {
            Font_DrawText(2, ppText[0], (int)g_pGraphics->resX / 2 - 10,
                          pLineY[i * 2],
                          (int *)g_colourWhite0x00524968, 0x14);
            Font_DrawText(2, ppText[1], (int)g_pGraphics->resX / 2 + 10,
                          pLineY[i * 2],
                          (int *)g_colourWhite0x00524968, 0x11);
        }
        if (((int *)FUN_004f4b30())[i] > -10)
            allOffScreen = false;
        ppText += 2;
    }
    if (allOffScreen)
        FUN_004f1bd0(0, 0);
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

// Text of the network waiting-room screen (0x4e2b40).
// GLOBAL: CMR2 0x00524e10
char g_str0x00524e10[8] = "Back";
// GLOBAL: CMR2 0x00524e18
char g_str0x00524e18[8] = "Start";
// GLOBAL: CMR2 0x00524e24
char g_str0x00524e24[16] = "Waiting Room";
// GLOBAL: CMR2 0x00524e34
char g_str0x00524e34[8] = "Network";
// GLOBAL: CMR2 0x00524e3c
char g_str0x00524e3c[20] = "Peugeot 205 T16, MT";
// GLOBAL: CMR2 0x00524e50
char g_str0x00524e50[8] = "SteppyS";
// GLOBAL: CMR2 0x00524e58
char g_str0x00524e58[24] = "Subaru Impreza WRC, AT";
// GLOBAL: CMR2 0x00524e70
char g_str0x00524e70[8] = "LeeM";
// GLOBAL: CMR2 0x00524e78
char g_str0x00524e78[8] = "Setup";
// GLOBAL: CMR2 0x00524e80
char g_str0x00524e80[16] = "Spectator: No";
// GLOBAL: CMR2 0x00524e90
char g_str0x00524e90[24] = "Transmission: Automatic";
// GLOBAL: CMR2 0x00524ea8
char g_str0x00524ea8[24] = "Car: Ford Focus 2000";
// GLOBAL: CMR2 0x00524ec0
char g_str0x00524ec0[16] = "Blah blah blah";
// GLOBAL: CMR2 0x00524ed0
char g_str0x00524ed0[24] = "SteppyS: Bimble bomble";
// GLOBAL: CMR2 0x00524ee8
char g_str0x00524ee8[24] = "LeeM : Wheeeeeeeeeeeeee";
// GLOBAL: CMR2 0x00524f00
char g_str0x00524f00[16] = "JamieL: Bananas";

// Parameter block of FUN_004d39a0: two flag bytes, the entry count, the
// selected entry and the string pointer array at +0x14.
struct Unk0x4e2b40 {
    BYTE field_0x0[6];
    BYTE field_0x6;
    BYTE field_0x7;
    BYTE field_0x8[2];
    BYTE count;
    BYTE selected;
    BYTE field_0xc[8];
    char *strings[6];
};

// Draw callback of the network waiting-room screen: the breadcrumb, the play
// time, the three peer names of the sample room and the player list.
// match 83%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// logic verified against the dump; the remaining diff is the 16-bit coordinate
// arithmetic the original gets at the Font_DrawText call sites (Font_DrawText
// is declared with int x/unsigned int y here, the original used narrower
// parameters) plus a couple of register choices.
// FUNCTION: CMR2 0x004e2b40
void FUN_004e2b40(Menu *pMenu)
{
    Unk0x4e2b40 list;
    short rowHeight;

    list.strings[0] = g_str0x00524e34;
    list.strings[1] = g_str0x00524e24;
    FrontendDraw_Breadcrumb(PATH_X(), PATH_Y(), list.strings, 2);
    FrontendDraw_PlayTime();
    g_unk0x008189a8[0] = 0x1e;
    g_unk0x008189a8[1] = 0x46;
    g_unk0x008189a8[2] = 0x190;
    g_unk0x008189a8[3] = 0x96;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, g_colourShadowText0x00524978, 4);
    Font_DrawText(1, g_str0x00524f00, g_unk0x008189a8[0] + 10, g_unk0x008189a8[1] + 10,
                  (int *)g_colourWhite0x00524968, 9);
    Font_DrawText(1, g_str0x00524ee8, g_unk0x008189a8[0] + 10, g_unk0x008189a8[1] + 0x1e,
                  (int *)g_colourWhite0x00524968, 9);
    Font_DrawText(1, g_str0x00524ed0, g_unk0x008189a8[0] + 10, g_unk0x008189a8[1] + 0x32,
                  (int *)g_colourWhite0x00524968, 9);
    rowHeight = g_unk0x008189a8[3] + 10;
    g_unk0x008189a8[2] = 0x190;
    g_unk0x008189a8[3] = 0x14;
    g_unk0x008189a8[1] += rowHeight;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, g_colourShadowText0x00524978, 4);
    Font_DrawText(1, g_str0x00524ec0, g_unk0x008189a8[0] + 10, g_unk0x008189a8[1],
                  (int *)g_colourWhite0x00524968, 9);
    list.field_0x6 |= 3;
    list.field_0x7 = 3;
    list.count = 6;
    list.selected = 2;
    list.strings[0] = g_str0x00524ea8;
    list.strings[1] = g_str0x00524e90;
    list.strings[2] = g_str0x00524e80;
    list.strings[3] = g_str0x00524e18;
    list.strings[4] = g_str0x00524e78;
    list.strings[5] = g_str0x00524e10;
    FUN_004d39a0((BYTE *)&list, NULL, 0x172, list.strings);
    Font_DrawText(1, g_str0x00524e70, 0x1b8, 0x46, (int *)g_colourWhite0x00524968, 9);
    Font_DrawText(1, g_str0x00524e58, 0x1b8, 0x5a, (int *)g_colourWhite0x00524968, 9);
    Font_DrawText(1, g_str0x00524e50, 0x1b8, 0x78, (int *)g_colourWhite0x00524968, 9);
    Font_DrawText(1, g_str0x00524e3c, 0x1b8, 0x8c, (int *)g_colourWhite0x00524968, 9);
}

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

extern char g_strTwoDigits[8];
extern char g_strGearboxManual[4];
extern char g_strGearboxAuto[4];
extern char g_loadRecordTimeFormat[];
extern BYTE g_colourTitle0x00524984[4];

// Draw callback of the stage record pages: title of the table shown (cycled by
// FUN_004f3a90), the column headers and the five entries (position, stage,
// surface, gearbox and record time).
// FUNCTION: CMR2 0x004e4130
void FUN_004e4130(Menu *pMenu)
{
    GameInfo0xa4SubStruct12 *pEntry;
    int table;
    int y0;
    int y;
    int row;

    table = FUN_004f3a60();
    FrontendDraw_PlayTime();
    FrontendDraw_MenuPath(pMenu, PATH_X(), PATH_Y(), 1, 2, NULL, -1);
    switch (table) {
    case 0:
        Font_DrawText(2, CFrontend::GetTextString(0x167), (int)g_pGraphics->resX / 2, (int)(g_pGraphics->resY * 100) / 480,
                      (int *)g_colourWhite0x00524968, 10);
        break;
    case 1:
        Font_DrawText(2, CFrontend::GetTextString(0x168), (int)g_pGraphics->resX / 2, (int)(g_pGraphics->resY * 100) / 480,
                      (int *)g_colourWhite0x00524968, 10);
        break;
    case 2:
        Font_DrawText(2, CFrontend::GetTextString(0x169), (int)g_pGraphics->resX / 2, (int)(g_pGraphics->resY * 100) / 480,
                      (int *)g_colourWhite0x00524968, 10);
        break;
    }
    g_unk0x008189a8[0] = (int)(g_pGraphics->resX * 30) / 640;
    g_unk0x008189a8[1] = (int)(g_pGraphics->resY * 200) / 480 - (int)(g_pGraphics->resY * 25) / 480;
    g_unk0x008189a8[2] = (int)(g_pGraphics->resX * 640) / 640 + (int)(g_pGraphics->resX * 30) / 640 * -2;
    g_unk0x008189a8[3] = (int)(g_pGraphics->resY * 25) / 480;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, g_colourShadowText0x00524978, 4);
    y0 = (int)(g_pGraphics->resY * 19) / 480 + ((int)(g_pGraphics->resY * 200) / 480 - (int)(g_pGraphics->resY * 25) / 480);
    Font_DrawText(0, CFrontend::GetTextString(0x16a), (int)(g_pGraphics->resX * 140) / 640, y0, (int *)g_colourTitle0x00524984, 0x12);
    Font_DrawText(0, CFrontend::GetTextString(0x16b), (int)(g_pGraphics->resX * 270) / 640, y0, (int *)g_colourTitle0x00524984, 0x12);
    Font_DrawText(0, CFrontend::GetTextString(0x16c), (int)(g_pGraphics->resX * 395) / 640, y0, (int *)g_colourTitle0x00524984, 0x12);
    Font_DrawText(0, CFrontend::GetTextString(0x16d), (int)(g_pGraphics->resX * 445) / 640, y0, (int *)g_colourTitle0x00524984, 0x12);
    Font_DrawText(0, CFrontend::GetTextString(0x174), (int)(g_pGraphics->resX * 520) / 640, y0, (int *)g_colourTitle0x00524984, 0x12);
    g_unk0x008189a8[0] = (int)(g_pGraphics->resX * 30) / 640;
    g_unk0x008189a8[1] = (int)(g_pGraphics->resY * 200) / 480;
    g_unk0x008189a8[2] = (int)(g_pGraphics->resX * 640) / 640 + (int)(g_pGraphics->resX * 30) / 640 * -2;
    g_unk0x008189a8[3] = 1;
    for (row = 0; row < 5; row++) {
        y = (int)(g_pGraphics->resY * 19) / 480 + (int)(g_pGraphics->resY * 200) / 480 + (int)(g_pGraphics->resY * 25) / 480 * row;
        pEntry = &CGameInfo::FUN_00405fe0()->secondLoop[15 * pMenu->cursor + 5 * table + row];
        sprintf(CFrontend::m_stringDest, g_strTwoDigits, row + 1);
        Font_DrawText(3, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 100) / 640, y, (int *)g_colourText0x0052496c, 0x12);
        Font_DrawText(1, pEntry->ident, (int)(g_pGraphics->resX * 140) / 640, y, (int *)g_colourWhite0x00524968, 0x12);
        Font_DrawText(1, CFrontend::FUN_0040ede0(pEntry->flags & 0x3f), (int)(g_pGraphics->resX * 270) / 640, y,
                      (int *)g_colourWhite0x00524968, 0x12);
        Font_DrawText(1, (pEntry->flags & 0x40) == 0 ? g_strGearboxManual : g_strGearboxAuto,
                      (int)(g_pGraphics->resX * 395) / 640, y, (int *)g_colourWhite0x00524968, 0x12);
        sprintf(CFrontend::m_stringDest, g_strTwoDigits, (pEntry->flags >> 7 & 0xf) + 1);
        Font_DrawText(3, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 445) / 640, y, (int *)g_colourWhite0x00524968, 0x12);
        sprintf(CFrontend::m_stringDest, g_loadRecordTimeFormat, pEntry->value / 6000, pEntry->value / 100 % 60, pEntry->value % 100);
        Font_DrawText(3, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 520) / 640, y, (int *)g_colourWhite0x00524968, 0x12);
        g_unk0x008189a8[1] = (short)((int)(g_pGraphics->resY * 200) / 480) + (short)((int)(g_pGraphics->resY * 25) / 480) * ((short)row + 1);
        Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, g_colourText0x0052496c, 1);
    }
    FrontendDraw_ScrollerRow(FUN_004f2540(), 1);
    FrontendDraw_HelpText(CFrontend::GetTextString(0x16f), 1);
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


Menu *FUN_004f8290(BYTE param1);
Menu *FUN_004f82c0(void);
Menu *FUN_004f82d0(void);
Menu *FUN_004f8410(void);
Menu *FUN_004f8450(void);
Menu *FUN_004f8470(void);
Menu *FUN_004f84a0(void);
Menu *FUN_004f84b0(void);
Menu *FUN_004f84c0(void);
Menu *FUN_004f84d0(void);
Menu *FUN_004f84e0(void);
Menu *FUN_004f84f0(void);
Menu *FUN_004f89a0(void);
void FUN_004f5520(void);
void FUN_004f5580(void);
void FUN_004f5670(void);
void FUN_004f5770(void);
void FUN_004f5810(void);
void FUN_004f5850(void);
void FUN_004f58e0(void);
void FUN_004f5940(void);
void FUN_004f59a0(void);
void FUN_004f5a60(void);
void FUN_004f5ae0(void);
void FUN_004f5b50(void);
void FUN_004f5b90(void);
void FUN_004f5c00(void);
void FUN_004f5c90(void);
void FUN_004f5d90(void);
void FUN_004f5e50(void);
void FUN_004f5eb0(void);
void FUN_004f5fc0(void);
void FUN_004f6040(void);
void FUN_004f6130(void);
void FUN_004f6240(void);
void FUN_004f6300(void);
void FUN_004f63b0(void);
void FUN_004f6420(void);
void FUN_004f6490(void);
void FUN_004f6540(void);
void FUN_004f6610(void);
void FUN_004f66e0(void);
void FUN_004f6740(void);
void FUN_004f6830(void);
void FUN_004f6910(void);
void FUN_004f6990(void);
void FUN_004f69f0(void);
void FUN_004f6a50(void);
void FUN_004f6b40(void);
void FUN_004f6c90(void);
void FUN_004f6d10(void);
void FUN_004f6d70(void);
void FUN_004f6dd0(void);
void FUN_004f6e50(char difficulty);
void FUN_004f6f10(char difficulty);
void FUN_004f6fd0(char difficulty);
void FUN_004f7090(char difficulty);
void FUN_004f7150(char difficulty);
void FUN_004f7270(int unused);
void FUN_004f7310(int unused);
void FUN_004f73b0(int unused);
void FUN_004f7450(void);
void FUN_004f7510(char choice);
void FUN_004f75b0(char choice);
void FUN_004f7650(char choice);
void FUN_004f76f0(void);
void FUN_004f77a0(void);
void FUN_004f7820(void);
void FUN_004f78c0(void);
void FUN_004f7970(void);
void FUN_004f7a30(void);
void FUN_004f7d00(void);
void FUN_004f7d70(void);
void FUN_004f7de0(void);
void FUN_004f7ed0(void);
void FUN_004f8020(void);
void FUN_004f8170(void);
void FUN_004f8250(void);
void FUN_004f8500(void);
void FUN_004f85b0(void);
void FUN_004f8670(void);
void FUN_004f8730(void);
void FUN_004f87d0(void);
void FUN_004f8870(void);
void FUN_004f88e0(void);
void FUN_004f9300(void);
void FUN_004f9370(void);
void FUN_004f9400(char players);
void FUN_004f9490(char players);
void FUN_004f9530(char unused);
void FUN_004f95d0(int unused);
void FUN_004f9670(void);
void FUN_004f9940(void);
void FUN_004f99b0(void);
void FUN_004f9c80(void);
void FUN_004f9cf0(void);
void FUN_004f9fc0(void);
void FUN_004fa030(void);
void FUN_004fa0b0(void);
void FUN_004fa1c0(void);
void FUN_004fa370(void);
void FUN_004fa410(void);
void FUN_004fa4d0(void);
BYTE FUN_00406800(void);
void FUN_004b7c80(void);
void FUN_004a0ba0(void);
void FUN_004a0c40(char param1);
void FUN_004a3c30(int language);
BOOL FUN_004a15a0(void);
void FUN_004eaa10(BYTE param1);
void FUN_004ef4a0(int value);
extern BYTE g_unk0x00818ac4;
extern int g_menuEnterTime;

// GLOBAL: CMR2 0x0052f3d8
int g_unk0x0052f3d8;
// Name of the demo profile used by quick race.
// GLOBAL: CMR2 0x00519ec4
char g_strDemoName0x00519ec4[4] = "DEM";

void FUN_004eb860(int index, int profile);
void FUN_004ebf20(int index);
void FUN_004eb000(BYTE index, char set);
void FUN_004eae90(unsigned int slot, char *pName);
void RallyData_FUN_0040e330(char param1);
void RallyData_FUN_004068e0(BYTE param1);
void FUN_0040dfa0(void);
void RallyData_FUN_00408600(BYTE index, BYTE value);
void FUN_004eb0c0(BYTE index, BYTE flag);
void FUN_004ea8c0(BYTE param1);
void FUN_004ea930(BYTE param1);
void FUN_004ea8e0(BYTE param1);

// Starts a fresh quick-race/championship session: resets the timing and rally
// data and selects the difficulty-dependent defaults.
// FUNCTION: CMR2 0x004e9e40
void FUN_004e9e40(void)
{
    int values[6] = { 5, 3, 2, 2, 0, 0 };
    int index;

    index = g_unk0x0052f3d8;
    g_unk0x0052f3d8 = (g_unk0x0052f3d8 + 1) % 4;
    FUN_004eb860(0, -1);
    FUN_004ebf20(0);
    FUN_004eb000(0, 1);
    FUN_004eae90(0, g_strDemoName0x00519ec4);
    RallyData_FUN_00408600(0, 0);
    FUN_004eb0c0(0, 1);
    CGameInfo::FUN_00406470();
    if (index == 3) {
        FUN_004ea8e0(6);
        FUN_004ea8c0(1);
        FUN_004ea930(2);
        RallyData_FUN_0040e330(0);
        RallyData_FUN_0040d640(5);
        RallyData_FUN_0040d600(0);
        RallyData_FUN_00406960(0);
        RallyData_FUN_0040d620(1);
        FUN_0040dfa0();
        RallyData_FUN_004068b0(5);
        RallyData_FUN_004068e0(2);
        return;
    }
    FUN_004ea8e0(2);
    RallyData_FUN_0040df60(1, 0);
    FUN_004ea8c0(1);
    FUN_004ea930(1);
    RallyData_FUN_0040e330(0);
    RallyData_FUN_0040d620(1);
    RallyData_FUN_004068b0((BYTE)values[index]);
    RallyData_FUN_004068e0((BYTE)values[index + 3]);
}

// Builds every frontend menu and picks the first one: after a network game
// the network lobby; otherwise the language menu (first run) or the main
// menu, and when coming back from a race (`back`) the menus of the game
// mode that was played, with their cursors.
// FUNCTION: CMR2 0x004e9f70
void FUN_004e9f70(BYTE param1, BYTE back)
{
    Menu *pMenu;
    unsigned int region;

    if (CGameInfo::FUN_00405e00() != 0) {
        if (FUN_004a15a0() != 0) {
            switch (CGameInfo::FUN_00405d80()) {
            case 8:
                pMenu = FUN_004f84b0();
                break;
            case 9:
                pMenu = FUN_004f84c0();
                break;
            case 10:
                pMenu = FUN_004f84d0();
                break;
            case 11:
                pMenu = FUN_004f84e0();
                break;
            case 12:
                pMenu = FUN_004f84f0();
                break;
            default:
                goto lobby;
            }
        } else {
            pMenu = FUN_004f8450();
        }
        FUN_004f8470()->pParent = pMenu;
    lobby:
        g_pMenu0x00818abc = NULL;
        if (FUN_00406800() != 0) {
            g_pMenu0x00818ac0 = FUN_004f89a0();
        } else {
            g_pMenu0x00818ac0 = FUN_004f8470();
            FUN_004f8470()->items[Menu_FindItem(FUN_004f8470(), 3)].max = 1;
        }
        FUN_004b7c80();
        FUN_004a3c30(CGameInfo::GetGameLanguage());
        return;
    }
    g_pMenu0x00818abc = NULL;
    g_pMenu0x00818ac0 = FUN_004f8290(param1);
    g_menuEnterTime = timeGetTime();
    if (FUN_004eaa30() == 0 && CGameInfo::FUN_00406450(0) > 180000)
        FUN_004eaa10(1);
    FUN_004a0ba0();
    FUN_004a0c40(0);
    CInput::FUN_0049ff80(-1, -1, -1, -1, -1);
    FUN_004f5670();
    g_unk0x00818ac4 = 0;
    FUN_004f5520();
    FUN_004f5580();
    region = CGameInfo::GetGameRegion();
    if (region == 2 || region == 3) {
        FUN_004a3c30(0);
        FUN_004fa4d0();
    } else if (param1 != 0) {
        FUN_004a3c30(0);
    } else {
        FUN_004a3c30(CGameInfo::GetGameLanguage());
        Menu_SetParent(FUN_004f82d0(), FUN_004f82c0());
        FUN_004f82d0()->items[0].pSubMenu = FUN_004f82c0();
        FUN_004f82d0()->items[1].pSubMenu = FUN_004f82c0();
        FUN_004f82d0()->items[2].pSubMenu = FUN_004f82c0();
        FUN_004f82d0()->items[3].pSubMenu = FUN_004f82c0();
        FUN_004f82d0()->items[4].pSubMenu = FUN_004f82c0();
        Menu_SetCursor(FUN_004f82d0(), CGameInfo::GetGameLanguage());
    }
    FUN_004f5770();
    FUN_004f5810();
    FUN_004f5850();
    FUN_004f58e0();
    FUN_004f5940();
    FUN_004f59a0();
    FUN_004f5a60();
    FUN_004f5ae0();
    FUN_004f5b50();
    FUN_004f9370();
    FUN_004f9400(CGameInfo::FUN_00405d70());
    FUN_004f9490(CGameInfo::FUN_00405d70());
    FUN_004f9530(CGameInfo::FUN_00405d90());
    FUN_004f95d0(CGameInfo::FUN_00405d90());
    FUN_004f9670();
    FUN_004f9940();
    FUN_004f99b0();
    FUN_004f9c80();
    FUN_004f9cf0();
    FUN_004f9fc0();
    FUN_004fa030();
    FUN_004fa0b0();
    FUN_004fa1c0();
    FUN_004f5b90();
    FUN_004f5c00();
    FUN_004f5c90();
    FUN_004f5d90();
    FUN_004f5e50();
    FUN_004f8500();
    FUN_004f85b0();
    FUN_004f8670();
    FUN_004f8730();
    FUN_004f87d0();
    FUN_004f9300();
    FUN_004f8870();
    FUN_004f88e0();
    FUN_004f5eb0();
    FUN_004f6040();
    FUN_004f6130();
    FUN_004f6300();
    FUN_004f6240();
    FUN_004f63b0();
    FUN_004f6420();
    FUN_004f6490();
    FUN_004f6540();
    FUN_004f66e0();
    FUN_004f6740();
    FUN_004f6830();
    FUN_004f6910();
    FUN_004f6990();
    FUN_004f6610();
    FUN_004f69f0();
    FUN_004f6a50();
    FUN_004f6b40();
    FUN_004f6c90();
    FUN_004f6d10();
    if (param1 == 0)
        FUN_004fa4d0();
    FUN_004f5fc0();
    FUN_004f6d70();
    FUN_004f6dd0();
    FUN_004f6e50(CGameInfo::FUN_00405d70());
    FUN_004f6f10(CGameInfo::FUN_00405d70());
    FUN_004f6fd0(CGameInfo::FUN_00405d70());
    FUN_004f7090(CGameInfo::FUN_00405d70());
    FUN_004f7150(CGameInfo::FUN_00405d70());
    FUN_004f7270(CGameInfo::FUN_00405d90());
    FUN_004f7310(CGameInfo::FUN_00405d90());
    FUN_004f73b0(CGameInfo::FUN_00405d90());
    FUN_004f7510(CGameInfo::FUN_00405da0());
    FUN_004f75b0(CGameInfo::FUN_00405da0());
    FUN_004f7650(CGameInfo::FUN_00405da0());
    FUN_004f76f0();
    FUN_004f77a0();
    FUN_004f7820();
    FUN_004f78c0();
    FUN_004f7970();
    FUN_004f7a30();
    FUN_004f7d00();
    FUN_004f7d70();
    FUN_004f7de0();
    FUN_004f7ed0();
    FUN_004f8020();
    FUN_004fa370();
    FUN_004fa410();
    FUN_004f7450();
    FUN_004f8170();
    FUN_004f8250();
    if (back != 0) {
        FUN_004f8410()->cursor = 1;
        FUN_004ef4a0(1);
        FUN_004f84a0()->cursor = 2;
        FUN_004fa2d0()->cursor = 1;
        CFrontend::FUN_004d2790();
        return;
    }
    switch (CGameInfo::FUN_00405d80()) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        FUN_004f84a0()->cursor = CGameInfo::FUN_00405d80();
        FUN_004f8410()->cursor = 1;
        FUN_004ef4a0(1);
        FUN_004fa2d0()->cursor = 1;
        CFrontend::FUN_004d2790();
        return;
    case 5:
    case 6:
    case 7:
        FUN_004fa2d0()->cursor = CGameInfo::FUN_00405d80() - 5;
        FUN_004f8410()->cursor = 2;
        FUN_004ef4a0(2);
        FUN_004f84a0()->cursor = 2;
        CFrontend::FUN_004d2790();
        return;
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
        FUN_004f8410()->cursor = 3;
        FUN_004ef4a0(3);
        break;
    }
    CFrontend::FUN_004d2790();
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
BYTE FUN_004086f0(BYTE param1);
void FUN_00409be0(int param);

// Sends this machine's player description (id, car, flags) to the network
// player list.
// match 43%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004ec2b0
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

void FUN_0040dc30(void);
struct Unk0x0052ebc0;
struct Unk0x0052ebc0 *RallyData_FUN_004083d0(void);
BYTE *RallyData_FUN_004082f0(void);
void FUN_00409bf0(DPID *pId, NetPlayerInfo *pInfo, char add);
void FUN_00409c80(int *pId);
void FUN_00409e30(char resetTotal, char resetTimes);
void FUN_0040ad20(void);
void FUN_0040afb0(char valid, BYTE *p);
void FUN_0040b120(void);
void FUN_00406720(int param1);
LPVOID *FUN_004a14c0(void);
LPVOID *FUN_004a14d0(void);
void FUN_004a15b0(BOOL param1);
void Session_SetOpen(char open);
int Session_GetUserValue(BYTE index);
void FUN_004a1760(DPSESSIONDESC2 *pDesc);
void FUN_004a1940(DPID *pId);
int FUN_004a1a10(int param1, int param2, int param3, int param4);
int FUN_004a1b90(int param1, void **param2);
char FUN_004a1c50(int to, int guaranteed, int data, int size);
void FUN_004d0620(DPID *pFrom, char *text, char local);
void RallyData_FUN_004068e0(BYTE param1);
void RallyData_FUN_00408600(BYTE index, BYTE value);
void FUN_004eb0c0(BYTE index, BYTE flag);
void FUN_004f02e0(void);
void FUN_0040dfa0(void);
void FUN_004067a0(int param1);
unsigned int FUN_004a1480(void);
int FUN_004a10b0(BYTE index, char *pPassword, BYTE *pInvalidPassword);
BYTE FUN_004a1790(BYTE index);
char Session_SetMaxPlayers(int count);
void Session_SetUserValue(BYTE index, int value);
void Session_SetName(LPVOID pName);
void Session_SetPassword(LPVOID pPassword);
void FUN_004f92e0(int value);

// The player setup data (0x540..0x6e7 of the rally data block) is mirrored
// into the outgoing packet and back. 0x18 bytes of it are per-player rows,
// the rest are the current selection values.
#define NET_SETUP_COUNT 0x14

// Copies the local player's setup (20 rows) into the outgoing packet.
// FUNCTION: CMR2 0x004ec320
void FUN_004ec320(BYTE *pPacket)
{
    BYTE *pState;
    int i;

    FUN_0040dc30();
    pState = (BYTE *)RallyData_FUN_004083d0();
    for (i = 0; i < NET_SETUP_COUNT; i++) {
        *(unsigned int *)(pPacket + 0x18 + i * 8) = *(unsigned int *)(pState + 0x540 + i * 8);
        *(unsigned int *)(pPacket + 0x1c + i * 8) = *(unsigned int *)(pState + 0x544 + i * 8);
        *(unsigned int *)(pPacket + 0xb8 + i * 4) = *(unsigned int *)(pState + 0x5e0 + i * 4);
        *(unsigned int *)(pPacket + 0x108 + i * 4) = *(unsigned int *)(pState + 0x630 + i * 4);
        *(unsigned int *)(pPacket + 0x158 + i * 4) = *(unsigned int *)(pState + 0x680 + i * 4);
        *(BYTE *)(pPacket + 0x1a8 + i) = *(BYTE *)(pState + 0x6d4 + i);
        *(unsigned int *)(pPacket + 0x1bc) = *(unsigned int *)(pState + 0x53c);
        *(unsigned int *)(pPacket + 0x1c0) = *(unsigned int *)(pState + 0x6d0);
    }
}

// Copies a received setup packet (20 rows) into the local rally data block.
// FUNCTION: CMR2 0x004ec3c0
void FUN_004ec3c0(BYTE *pPacket)
{
    BYTE *pState = (BYTE *)RallyData_FUN_004083d0();
    int i;

    for (i = 0; i < NET_SETUP_COUNT; i++) {
        *(unsigned int *)(pState + 0x540 + i * 8) = *(unsigned int *)(pPacket + 0x18 + i * 8);
        *(unsigned int *)(pState + 0x544 + i * 8) = *(unsigned int *)(pPacket + 0x1c + i * 8);
        *(unsigned int *)(pState + 0x5e0 + i * 4) = *(unsigned int *)(pPacket + 0xb8 + i * 4);
        *(unsigned int *)(pState + 0x630 + i * 4) = *(unsigned int *)(pPacket + 0x108 + i * 4);
        *(unsigned int *)(pState + 0x680 + i * 4) = *(unsigned int *)(pPacket + 0x158 + i * 4);
        *(BYTE *)(pState + 0x6d4 + i) = *(BYTE *)(pPacket + 0x1a8 + i);
        *(unsigned int *)(pState + 0x53c) = *(unsigned int *)(pPacket + 0x1bc);
        *(unsigned int *)(pState + 0x6d0) = *(unsigned int *)(pPacket + 0x1c0);
    }
}

// Outgoing player setup packet (type 3).
struct NetSetupPacket {
    BYTE type;                  // 0x0
    BYTE field_0x1[3];
    unsigned int gameMode : 8;  // 0x4
    unsigned int country : 4;   // bits 8..11
    unsigned int stage : 4;     // bits 12..15
    unsigned int rallyA : 2;    // bits 16..17
    unsigned int rallyB : 2;    // bits 18..19
    unsigned int rallyC : 4;    // bits 20..23
    unsigned int unused : 8;
    BYTE field_0x8[0xc];        // 0x8
    int field_0x14;             // 0x14
    BYTE field_0x18[0x1ac];     // 0x18
};

// Builds and sends this machine's player setup packet.
// FUNCTION: CMR2 0x004ec460
void FUN_004ec460(void)
{
    NetSetupPacket packet;

    packet.type = 3;
    packet.gameMode = CGameInfo::FUN_00405d80();
    switch (CGameInfo::FUN_00405d80()) {
    case 8:
    case 9:
    case 10:
        packet.country = RallyDataCountryIndex();
        packet.stage = RallyDataStageIndex();
        break;
    case 0xb:
    case 0xc:
        packet.rallyA = RallyData_FUN_00406940();
        packet.rallyB = RallyData_FUN_00406950();
        packet.rallyC = RallyData_FUN_00406990();
        break;
    }
    packet.field_0x14 = FUN_00406710();
    FUN_004ec320((BYTE *)&packet);
    FUN_004a1c50(0, 1, (int)&packet, 0x1c4);
}

// Message handler of the joining side: session data, player info and the
// setup packet of the host.
// FUNCTION: CMR2 0x004ec560
void FUN_004ec560(DPID *pFrom, unsigned int *pData)
{
    char *pText;
    unsigned int type;

    type = pData[0];
    switch (type) {
    case 5:
        FUN_004a1940((DPID *)(pData + 2));
        FUN_00409c80((int *)(pData + 2));
        return;
    case 3:
        FUN_00409bf0((DPID *)(pData + 2), (NetPlayerInfo *)pData[4], 1);
        FUN_004ec2b0();
        FUN_004ec460();
        return;
    case 0x101:
        FUN_004a15b0(1);
        if (FUN_004a14c0() != NULL)
            strcpy(g_unk0x00818ebc, (char *)FUN_004a14c0());
        if (FUN_004a14d0() != NULL)
            strcpy(g_unk0x00818ef8, (char *)FUN_004a14d0());
        g_unk0x00818ce4 = 1;
        FUN_004ec2b0();
        switch (CGameInfo::FUN_00405d80()) {
        case 8:
            FUN_004f8470()->pParent = FUN_004f84b0();
            break;
        case 9:
            FUN_004f8470()->pParent = FUN_004f84c0();
            break;
        case 10:
            FUN_004f8470()->pParent = FUN_004f84d0();
            break;
        case 0xb:
            FUN_004f8470()->pParent = FUN_004f84e0();
            break;
        case 0xc:
            FUN_004f8470()->pParent = FUN_004f84f0();
            break;
        }
        if (FUN_004ea5d0() != FUN_004f8460()) {
            Session_SetOpen(1);
            return;
        }
        Session_SetOpen(0);
        return;
    case 0x104:
        FUN_004a1760((DPSESSIONDESC2 *)(pData + 1));
        FUN_004ea8e0((BYTE)Session_GetUserValue(0));
        FUN_00406780(Session_GetUserValue(2));
        return;
    case 0x102:
        FUN_00409bf0((DPID *)(pData + 2), (NetPlayerInfo *)pData[3], 1);
        return;
    }
}

// Message handler of the host side: applies the setup packet of a joining
// player and starts the race for it.
// FUNCTION: CMR2 0x004ec6f0
void FUN_004ec6f0(DPID *pFrom, BYTE *pMsg)
{
    BYTE *pState = RallyData_FUN_004082f0();
    MenuItem *pItem;
    int i;

    switch (pMsg[0]) {
    case 0:
        FUN_004d0620(pFrom, (char *)(pMsg + 1), 0);
        return;
    case 2:
    case 3:
        for (i = 0; i < 0xb; i++)
            pState[i] = pMsg[i + 8];
        FUN_004ea8e0(pMsg[4]);
        switch (CGameInfo::FUN_00405d80()) {
        case 8:
            RallyData_FUN_004068b0((*(unsigned int *)(pMsg + 4) >> 8) & 0xf);
            for (i = 0; i < 0xb; i++) {
                if ((pState[i] & 1) != 0 && ((pState[i] & 2) == 0 || CGameInfo::FUN_00406410(0xd))
                    && (pState[i] & 4) == 0) {
                    RallyData_FUN_004068e0(i);
                    break;
                }
            }
            break;
        case 9:
        case 10:
            RallyData_FUN_004068b0((*(unsigned int *)(pMsg + 4) >> 8) & 0xf);
            RallyData_FUN_004068e0((*(unsigned int *)(pMsg + 4) >> 0xc) & 0xf);
            break;
        case 0xb:
        case 0xc:
            RallyData_FUN_004068e0(0);
            RallyData_FUN_0040d600((*(unsigned int *)(pMsg + 4) >> 0x10) & 3);
            RallyData_FUN_00406960((*(unsigned int *)(pMsg + 4) >> 0x12) & 3);
            RallyData_FUN_0040d620((*(unsigned int *)(pMsg + 4) >> 0x14) & 0xf);
            break;
        }
        FUN_00406720(*(unsigned int *)(pMsg + 0x14));
        pItem = Menu_GetItem(FUN_004f8470(), 1);
        RallyData_FUN_00408600(0, g_unk0x00818d18[pItem->max]);
        pItem = Menu_GetItem(FUN_004f8470(), 2);
        FUN_004eb0c0(0, pItem->max);
        FUN_004ec3c0((BYTE *)pMsg);
        *(unsigned int *)(g_unk0x00818ef8 + 0x14) = CMain::GetFrameDelta();
        FUN_00409ab0(0, 1);
        if (pMsg[0] == 2) {
            Menu_SetNextAction((int)FUN_004f8330());
            FUN_00409e30(1, 1);
            FUN_00409b60();
            FUN_0040b120();
            return;
        }
        break;
    case 0x11:
        FUN_0040afb0(pMsg[1], (BYTE *)(pMsg + 4));
        FUN_0040ad20();
        break;
    }
}

// Drains the network message queue, dispatching each message to the handler
// of its direction.
// FUNCTION: CMR2 0x004ec8d0
void FUN_004ec8d0(void)
{
    DPID from;
    void *pData;

    while (FUN_004a1b90((int)&from, &pData) != 0) {
        if (from == 0)
            FUN_004ec560(&from, (unsigned int *)pData);
        else
            FUN_004ec6f0(&from, (BYTE *)pData);
    }
}

bool FUN_004aac00(void);

// Callback of the network session menu: creates the DirectPlay session and
// lobby; the connection count becomes the value of the session type item.
// FUNCTION: CMR2 0x004ec9a0
void FUN_004ec9a0(Menu *pMenu, int param)
{
    g_unk0x00818f10 = 0;
    if (CGame::CreateDirectPlay()) {
        if (CGame::CreateDirectPlayLobby()) {
            if (FUN_004aac00()) {
                FUN_004f8440()->items[0].min = (BYTE)CGame::GetConnectionCount();
                FUN_004f8440()->items[0].max = 0;
                g_unk0x00818f10 = 1;
                return;
            }
            CGame::DestroyDirectPlayLobby();
            CGame::DestroyDirectPlay();
            Menu_SetNextAction((int)pMenu->pParent);
            return;
        }
        CGame::DestroyDirectPlay();
    }
}

// Sends the local player's description to the DirectPlay lobby. The original
// leaks the return value of the send call through this function.
// FUNCTION: CMR2 0x004eca10
char FUN_004eca10(void)
{
    unsigned int info[4];

    info[0] = FUN_004a1a00();
    info[1] = (RallyData_FUN_004086b0(0) & 0x1f) | (info[1] & 0xffffffe0) | 0x80;
    return FUN_004a1a10((int)RallyData_GetRecord(0), (int)RallyData_GetRecord(0), (int)info, 0x10);
}

// The GUID of the session the browser list currently points at.
int FUN_004a15c0(BYTE index, GUID *pOut);
// GLOBAL: CMR2 0x00818cf0
GUID g_unk0x00818cf0;
// GLOBAL: CMR2 0x00819018
int g_unk0x00819018;

// Per-frame step of the network session browser: enumerates the sessions,
// keeps the cursor on the previously selected one (or moves it with the
// up/down buttons), scrolls the five visible rows and fills the menu.
// match 58%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// logic verified against the dump; the difference is MSVC's basic-block
// order (the original keeps the -1/-2 exits and the first-time setup after
// the main return) plus the EAX-only return of CGameInfo::FUN_004a12d0,
// which here goes through a function-pointer cast because GameInfo.h
// declares it void.
// FUNCTION: CMR2 0x004ecaf0
void FUN_004ecaf0(Menu *pMenu)
{
    DeviceInfo *pDevice;
    GUID guid;
    int status;
    int i;

    pDevice = CInput::FUN_0049ead0(0);
    if (g_unk0x00818ef4 == 0)
        return;
    if (g_unk0x00818d04 == 0) {
        g_pGraphics->pDD7->FlipToGDISurface();
        ShowCursor(1);
    }
    // the original returns the enumeration status in EAX
    status = ((int (*)(int))CGameInfo::FUN_004a12d0)((int)&g_unk0x00818ef8);
    if (g_unk0x00818d04 == 0) {
        ShowCursor(0);
        ShowWindow(CMain::m_hWndList[CMain::m_hWndIx], SW_RESTORE);
    }
    if (status == 1) {
        g_unk0x00818d04 = 1;
        g_unk0x00819018 = FUN_004a1480();
        if (g_unk0x00819014 == 0) {
            g_unk0x00525288 = 0;
            g_unk0x0052528c = -1;
            g_unk0x00819024 = 0;
            Menu_SetFlags(pMenu, 1, 1, 1, 1);
            goto tail;
        }
        if (g_unk0x00819018 <= 0) {
            g_unk0x00819014 = 0;
            g_unk0x00525288 = -1;
            g_unk0x0052528c = -1;
            g_unk0x00819024 = 0;
            g_unk0x0081901c = 0;
        } else {
            if (g_unk0x0052528c != g_unk0x00819018) {
                if (g_unk0x00819024 == 0) {
                    g_unk0x00525288 = -1;
                } else {
                    for (i = 0; i < g_unk0x00819018; i++) {
                        FUN_004a15c0((BYTE)i, &guid);
                        if (IsEqualGUID(g_unk0x00818cf0, guid)) {
                            g_unk0x00525288 = i;
                            break;
                        }
                    }
                }
            }
            g_unk0x0052528c = g_unk0x00819018;
            if (g_unk0x00525288 == -1) {
                g_unk0x00525288 = 0;
                FUN_004a15c0(0, &g_unk0x00818cf0);
                g_unk0x00819024 = 1;
            } else if ((pDevice->field_0x8 & 0x20) != 0) {
                g_unk0x00819014 = 0;
                g_unk0x00525288 = -1;
                g_unk0x00819024 = 0;
                CGameInfo::FUN_004a13b0();
                g_unk0x00818ef4 = 0;
            } else {
                Menu_SetFlags(pMenu, 0, 0, 1, 0);
                if ((pDevice->field_0x8 & 4) != 0 && g_unk0x00525288 > 0) {
                    g_unk0x00525288--;
                    FUN_004a15c0((BYTE)g_unk0x00525288, &g_unk0x00818cf0);
                    g_unk0x00819024 = 1;
                } else if ((pDevice->field_0x8 & 8) != 0 && g_unk0x00525288 < g_unk0x00819018 - 1) {
                    g_unk0x00525288++;
                    FUN_004a15c0((BYTE)g_unk0x00525288, &g_unk0x00818cf0);
                    g_unk0x00819024 = 1;
                }
            }
        }
        if (g_unk0x00525288 < g_unk0x0081901c) {
            g_unk0x0081901c--;
            if (g_unk0x0081901c < 0)
                g_unk0x0081901c = 0;
        }
        if (g_unk0x00525288 >= g_unk0x0081901c + 5)
            g_unk0x0081901c++;
    } else {
        if (status == -1)
            return;
        if (status != -2)
            return;
        CGame::FUN_0049c140();
        Menu_SetNextAction((int)pMenu->pParent);
        return;
    }
tail:
    if (g_unk0x00819018 > 5)
        g_unk0x00819020 = 5;
    else
        g_unk0x00819020 = g_unk0x00819018;
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

// Item action of the session browser: joins the session the list points at
// and moves on to the car select or the error screen.
// FUNCTION: CMR2 0x004ecd60
void FUN_004ecd60(Menu *pMenu, int param)
{
    if (g_unk0x00818ef4 != 0) {
        if (g_unk0x00819014 != 0 && g_unk0x00819024 != 0 && g_unk0x00525288 >= 0
            && (int)g_unk0x00525288 < (int)FUN_004a1480()) {
            g_unk0x00818ce4 = 0;
            FUN_00409a30();
            FUN_004d05f0();
            g_unk0x00818ef4 = 0;
            if (FUN_004a1790((BYTE)g_unk0x00525288) == 0) {
                // The original reuses the menu parameter slot for the
                // invalid-password flag of the join call (its value is unused).
                if (FUN_004a10b0((BYTE)g_unk0x00525288, g_unk0x00818da8, (BYTE *)&pMenu) != 0) {
                    FUN_004ea8e0((BYTE)Session_GetUserValue(0));
                    FUN_00406780(Session_GetUserValue(2));
                    FUN_004a1a00();
                    RallyData_FUN_004086b0(0);
                    if (FUN_004eca10() != 0) {
                        Menu_SetParent(FUN_004f8470(), FUN_004f8450());
                        Menu_SetNextAction((int)FUN_004f8470());
                        return;
                    }
                    FUN_004a1280();
                    FUN_004f92e0(3);
                    Menu_SetNextAction((int)FUN_004f9360());
                    return;
                }
            }
            if (FUN_004a1790((BYTE)g_unk0x00525288) != 0) {
                Menu_SetNextAction((int)FUN_004f88d0());
                return;
            }
            FUN_004a1280();
            FUN_004f92e0(1);
            Menu_SetNextAction((int)FUN_004f9360());
            return;
        }
        if ((int)FUN_004a1480() > 0)
            g_unk0x00819014 = 1;
    } else {
        pMenu->items[0].id = 0x1ec;
        g_unk0x00818ef4 = 1;
    }
}

BYTE FUN_004ea920(void);
int FUN_004a0ec0(char *pSessionName, char *pPassword, DWORD user1, DWORD user2, DWORD user3,
                 DWORD user4, DWORD maxPlayers);

// Item action of the network session menu: refreshes the session data and
// starts the DirectPlay session, restoring the window when the game had left
// it hidden for the network dialog.
// FUNCTION: CMR2 0x004ecea0
void FUN_004ecea0(Menu *pMenu, int param)
{
    g_unk0x00818ef4 = 0;
    g_unk0x00818ce4 = 1;
    FUN_00409a30();
    FUN_004d05f0();
    FUN_004ea8e0(FUN_004ea920());
    if (g_unk0x00818d04 == 0) {
        g_pGraphics->pDD7->FlipToGDISurface();
        ShowCursor(1);
    }
    if (FUN_004a0ec0(CGameInfo::FUN_00406690(), CGameInfo::FUN_004066d0(),
                     CGameInfo::FUN_00405d80() & 0xff, 0, 0, 0, 8) != 0) {
        if (g_unk0x00818d04 == 0) {
            ShowCursor(0);
            ShowWindow(CMain::m_hWndList[CMain::m_hWndIx], SW_RESTORE);
            g_unk0x00818d04 = 1;
        }
        if (FUN_004eca10() != 0) {
            Menu_SetNextAction((int)FUN_004f8460());
            return;
        }
    } else {
        if (g_unk0x00818d04 == 0) {
            ShowCursor(0);
            ShowWindow(CMain::m_hWndList[CMain::m_hWndIx], SW_RESTORE);
        }
    }
}

bool FUN_004aac40(BYTE param1);

// Item action of the network leaderboard list: goes on to the leaderboard of
// the slot shown in the first row, but only while it is still connected.
// FUNCTION: CMR2 0x004ecf80
void FUN_004ecf80(Menu *pMenu, int param)
{
    if (FUN_004aac40(pMenu->items[0].max))
        Menu_SetNextAction((int)FUN_004f8450());
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

void Session_SetOpen(char open);
DWORD FUN_004a1720(int index);
BYTE FUN_004086f0(BYTE param1);
int FUN_00406770(void);
int FUN_00406790(void);
BYTE FUN_004ea920(void);

// Prepares the "record your name" screen: copies the selected car/driver
// names, clamps the letter rows to the current values and rebuilds the
// keyboard rows.
// FUNCTION: CMR2 0x004ecfd0
void FUN_004ecfd0(Menu *pMenu, char param)
{
    FUN_004b7c80();
    Session_SetOpen(0);
    if ((int)FUN_004a1720(-1) < 7)
        Menu_GetItem(pMenu, 1)->min = 5;
    else
        Menu_GetItem(pMenu, 1)->min = 3;
    if (Menu_GetItem(pMenu, 1)->max >= Menu_GetItem(pMenu, 1)->min)
        Menu_GetItem(pMenu, 1)->max = Menu_GetItem(pMenu, 1)->min - 1;
    if (param == 0) {
        strcpy(g_unk0x00818ebc, CGameInfo::FUN_00406690());
        strcpy(g_unk0x00818ef8, CGameInfo::FUN_004066d0());
        Menu_GetItem(pMenu, 2)->max = FUN_00406770() + 1;
        Menu_GetItem(pMenu, 3)->max = FUN_00406790() - 2;
    }
    Menu_GetItem(pMenu, 1)->max = FUN_004ea920() - 8;
}

// Item action of the "create session" screen: validates the session name,
// player count and password items and sends the session setup of this host.
// FUNCTION: CMR2 0x004ed340
void FUN_004ed340(Menu *pMenu, int param)
{
    if (strcmp(CMain::m_logFileBlankLine, g_unk0x00818ebc) == 0) {
        pMenu->cursor = 0;
        Menu_PlaySoundId(3);
        return;
    }
    Session_SetUserValue(2, Menu_GetItem(pMenu, 2)->max - 1);
    FUN_00406780(Menu_GetItem(pMenu, 2)->max - 1);
    if (Session_SetMaxPlayers(Menu_GetItem(pMenu, 3)->max + 2)) {
        FUN_004067a0(Menu_GetItem(pMenu, 3)->max + 2);
        Session_SetName(g_unk0x00818ebc);
        CGameInfo::FUN_004066a0(g_unk0x00818ebc);
        Session_SetPassword(g_unk0x00818ef8);
        CGameInfo::FUN_004066e0(g_unk0x00818ef8);
        FUN_004ea8e0(Menu_GetItem(pMenu, 1)->max + 8);
        Session_SetUserValue(0, Menu_GetItem(pMenu, 1)->max + 8);
        Session_SetOpen(1);
        switch (Menu_GetItem(pMenu, 1)->max) {
        case 0:
            Menu_SetNextAction((int)FUN_004f84b0());
            break;
        case 1:
            Menu_SetNextAction((int)FUN_004f84c0());
            break;
        case 2:
            Menu_SetNextAction((int)FUN_004f84d0());
            break;
        case 3:
            Menu_SetNextAction((int)FUN_004f84e0());
            break;
        default:
            Menu_SetNextAction((int)FUN_004f84f0());
            break;
        }
        FUN_004ec460();
        return;
    }
    Menu_PlaySoundId(3);
    Menu_SelectItem(pMenu, 3);
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


// Rebuilds a front-end stage/route list menu from the current selection.
bool RallyData_FUN_00408e30(int index, int bit, char check);
BYTE FUN_004086f0(BYTE param1);

// FUNCTION: CMR2 0x004ed530
void FUN_004ed530(Menu *pMenu, int unused)
{
    int i;
    int index;
    int *pEntry;

    g_unk0x00818ed0 = 1;
    g_unk0x00818ce8 = -1;
    g_unk0x00818ce0 = -1;
    FUN_004ea5c0(0);
    Menu_SetFlags(pMenu, 1, 1, 1, 1);
    pMenu->cursor = 0;
    g_unk0x00818d00 = 0;
    index = 0;
    pEntry = g_unk0x00818d18;
    for (i = 0; i < 0x16; i++) {
        if (RallyData_FUN_00408e30(0, i, 1)) {
            *pEntry = i;
            if (i == (RallyData_FUN_004086b0(0) & 0xff)) {
                Menu_GetItem(pMenu, 1)->max = index;
            }
            index++;
            pEntry++;
        }
    }
    if (FUN_004086f0(0) != 0)
        Menu_GetItem(pMenu, 2)->max = 1;
    else
        Menu_GetItem(pMenu, 2)->max = 0;
    if (g_unk0x00818ce4 != 0) {
        Menu_GetItem(pMenu, 3)->max = 0;
        Menu_GetItem(pMenu, 1)->min = index;
        return;
    }
    Menu_GetItem(pMenu, 3)->max = 1;
    Menu_GetItem(pMenu, 1)->min = index;
}

// Start action of the "host race" screen: copies the selected rally setup
// into the rally data block and sends the host setup packet (type 2).
// FUNCTION: CMR2 0x004ed610
void FUN_004ed610(Menu *pMenu, int param)
{
    BYTE *pState = RallyData_FUN_004082f0();
    NetSetupPacket packet;
    int i;

    if (FUN_004a15a0()) {
        Session_SetOpen(0);
        for (i = 0; i < 0xb; i++) {
            packet.field_0x8[i] = CGameInfo::FUN_00406520((BYTE)RallyDataCountryIndex(), i);
            if (CGameInfo::FUN_00406410(0xd))
                packet.field_0x8[i] &= 0xfd;
            pState[i] = packet.field_0x8[i];
        }
        if (CGameInfo::FUN_00405d80() == 8) {
            for (i = 0; i < 0xb; i++) {
                if ((CGameInfo::FUN_00406520((BYTE)RallyDataCountryIndex(), i) & 1) != 0
                    && ((CGameInfo::FUN_00406520((BYTE)RallyDataCountryIndex(), i) & 2) == 0
                        || CGameInfo::FUN_00406410(0xd))
                    && (CGameInfo::FUN_00406520((BYTE)RallyDataCountryIndex(), i) & 4) == 0) {
                    RallyData_FUN_004068e0(i);
                    break;
                }
            }
        }
        RallyData_FUN_00408600(0, g_unk0x00818d18[Menu_GetItem(pMenu, 1)->max]);
        FUN_004eb0c0(0, Menu_GetItem(pMenu, 2)->max);
        packet.type = 2;
        packet.gameMode = CGameInfo::FUN_00405d80();
        switch (CGameInfo::FUN_00405d80()) {
        case 8:
        case 9:
        case 10:
            packet.country = RallyDataCountryIndex();
            packet.stage = RallyDataStageIndex();
            break;
        case 0xb:
        case 0xc:
            packet.rallyA = RallyData_FUN_00406940();
            packet.rallyB = RallyData_FUN_00406950();
            packet.rallyC = RallyData_FUN_00406990();
            break;
        }
        packet.field_0x14 = FUN_00406710();
        FUN_004ec320((BYTE *)&packet);
        FUN_004a1c50(0, 1, (int)&packet, 0x1c4);
        FUN_00409e30(1, 1);
        FUN_00409b60();
        FUN_00409ab0(0, 1);
        CGame::FUN_004083e0(1);
        Menu_SetNextAction((int)FUN_004f8330());
        FUN_0040b120();
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

// Update callback of the session browser menu: drains the network queue.
// FUNCTION: CMR2 0x004edb60
void FUN_004edb60(Menu *pMenu)
{
    FUN_004ec8d0();
    FUN_004b7c80();
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

// Item action of the "join session" screen: applies the selected role and
// opens the session, then continues to the car select screen.
// FUNCTION: CMR2 0x004edd50
void FUN_004edd50(Menu *pMenu, int param)
{
    int i;
    int value;

    for (i = 0; i < Menu_GetItem(pMenu, 0)->min; i++) {
        if ((Menu_GetItem(pMenu, 0)->min == 5 && i == 4)
            || (Menu_GetItem(pMenu, 0)->min == 9 && i == 8))
            value = 10;
        else
            value = i;
        if ((CGameInfo::FUN_00406520(Menu_GetItem(pMenu, 1)->max, value) & 1) != 0) {
            RallyData_FUN_004068e0(0);
            RallyData_FUN_004068b0(Menu_GetItem(pMenu, 1)->max);
            FUN_004f8470()->pParent = pMenu;
            Menu_SetNextAction((int)FUN_004f8470());
            break;
        }
    }
    FUN_004ec460();
}

// Change callback of the "join session" menu: keeps the role item's minimum
// (the number of players) in sync with the selected player count.
// FUNCTION: CMR2 0x004ede10
void FUN_004ede10(Menu *pMenu)
{
    int i;

    if (CGameInfo::FUN_00406410(0xd)) {
        Menu_GetItem(pMenu, 0)->min = Menu_GetItem(pMenu, 1)->max % 2 + 10;
        FUN_004ec8d0();
        return;
    }
    for (i = 0; i < 0xb; i++) {
        if ((CGameInfo::FUN_00406520(Menu_GetItem(pMenu, 1)->max, i) & 2) != 0) {
            Menu_GetItem(pMenu, 0)->min = i;
            break;
        }
    }
    if (Menu_GetItem(pMenu, 1)->max % 2 != 0 && (i == 4 || i == 8)
        && (CGameInfo::FUN_00406520(Menu_GetItem(pMenu, 1)->max, 10) & 2) == 0)
        Menu_GetItem(pMenu, 0)->min++;
    FUN_004ec8d0();
}

// Init callback of the "join session" menu: sets the allowed player count
// from the game mode flags and fills the setup items of the local player.
// FUNCTION: CMR2 0x004edef0
void FUN_004edef0(Menu *pMenu, char param)
{
    unsigned int *pFlags = CGameInfo::FUN_00405db0();
    int values[2];
    int min;

    if (CGameInfo::FUN_00406410(0xd)) {
        min = 8;
    } else {
        unsigned int v = *pFlags;

        min = 4;
        if ((int)((v >> 8) & 0xf) > min)
            min = (v >> 8) & 0xf;
        if ((int)((v >> 0xc) & 0xf) > min)
            min = (v >> 0xc) & 0xf;
        if ((v & 1) != 0 && (int)((v >> 0x10) & 0xf) > min)
            min = (v >> 0x10) & 0xf;
    }
    Menu_GetItem(pMenu, 0)->min = min;
    if (param != 0) {
        values[0] = (BYTE)RallyDataCountryIndex();
        values[1] = (BYTE)RallyDataStageIndex();
    } else {
        FUN_004ea990(&values[0], &values[1]);
        Menu_GetItem(pMenu, 0)->max = values[0];
        Menu_GetItem(pMenu, 1)->max = values[1];
    }
    if (CGameInfo::FUN_00406410(0xd)) {
        min = (values[0] % 2 != 0) + 0xa;
    } else {
        min = 4;
        if ((int)((*pFlags >> 8) & 0xf) >= values[0] + 1)
            min = param;
        if ((int)((*pFlags >> 0xc) & 0xf) < values[0] + 1)
            min = 8;
        if ((*pFlags & 1) != 0 && (int)((*pFlags >> 0x10) & 0xf) < values[0] + 1)
            min = 0xa;
        if (values[0] % 2 != 0) {
            unsigned int mask = 1 << ((values[0] + 1) / 2 - 1);

            if ((pFlags[1] & mask & 0x1f) != 0
                || (((pFlags[1] >> 5) & 0x1f) & mask) != 0
                || (((pFlags[1] >> 10) & 0x1f) & mask) != 0)
                min++;
        }
    }
    Menu_GetItem(pMenu, 1)->min = min;
    if (param == 0)
        Menu_GetItem(pMenu, 2)->max = 0;
    g_unk0x00818d70 = values[0];
    FUN_004ee170(pMenu);
}

// Item action of the "join" screen: applies the selected rally/route and
// sends the join setup of this player.
// FUNCTION: CMR2 0x004ee090
void FUN_004ee090(Menu *pMenu, int param)
{
    if ((Menu_GetItem(pMenu, 1)->min == 5 && Menu_GetItem(pMenu, 1)->max == 4)
        || (Menu_GetItem(pMenu, 1)->min == 9 && Menu_GetItem(pMenu, 1)->max == 8)
        || (Menu_GetItem(pMenu, 1)->min == 0xb && Menu_GetItem(pMenu, 1)->max == 0xa))
        RallyData_FUN_004068e0(10);
    else
        RallyData_FUN_004068e0(Menu_GetItem(pMenu, 1)->max);
    RallyData_FUN_004068b0(Menu_GetItem(pMenu, 0)->max);
    if (RallyDataStageIndex() != 0xa && g_unk0x00818d74[Menu_GetItem(pMenu, 2)->max] != -1)
        RallyData_FUN_0040df60(1, g_unk0x00818d74[Menu_GetItem(pMenu, 2)->max]);
    else
        RallyData_FUN_0040df60(1, 0);
    FUN_004f8470()->pParent = pMenu;
    Menu_SetNextAction((int)FUN_004f8470());
    FUN_004ec460();
}

int FUN_00406750(void);

// Update callback of the two-player rally menu: limits the rally list to the
// unlocked events and mirrors the rally/stage reached in the profile.
// FUNCTION: CMR2 0x004ee460
void FUN_004ee460(Menu *pMenu, int param)
{
    unsigned int *pFlags = CGameInfo::FUN_00405db0();
    int values[2];
    int mask;
    int count;

    if (CGameInfo::FUN_00406410(0xd)) {
        count = 8;
    } else {
        count = 4;
        if ((int)(*pFlags >> 8 & 0xf) > count)
            count = (int)(*pFlags >> 8 & 0xf);
        if ((int)(*pFlags >> 0xc & 0xf) > count)
            count = (int)(*pFlags >> 0xc & 0xf);
        if ((*pFlags & 1) != 0 && (int)(*pFlags >> 0x10 & 0xf) > count)
            count = (int)(*pFlags >> 0x10 & 0xf);
    }
    Menu_GetItem(pMenu, 0)->min = count;
    FUN_004ea990(&values[0], &values[1]);
    Menu_GetItem(pMenu, 0)->max = values[0];
    Menu_GetItem(pMenu, 1)->max = values[1];
    if (CGameInfo::FUN_00406410(0xd)) {
        count = (values[0] % 2 != 0) + 10;
    } else {
        count = 4;
        if ((int)(*pFlags >> 8 & 0xf) >= values[0] + 1)
            count = param;
        if ((int)(*pFlags >> 0xc & 0xf) < values[0] + 1)
            count = 8;
        if ((*pFlags & 1) != 0 && (int)(*pFlags >> 0x10 & 0xf) < values[0] + 1)
            count = 0xa;
        if (values[0] % 2 != 0) {
            mask = 1 << ((values[0] + 1) / 2 - 1);
            if ((pFlags[1] & mask & 0x1f) != 0 ||
                (mask & (pFlags[1] >> 5 & 0x1f)) != 0 ||
                (mask & (pFlags[1] >> 10 & 0x1f)) != 0)
                count++;
        }
    }
    Menu_GetItem(pMenu, 1)->min = count;
    if ((param & 0xff) == 0) {
        Menu_GetItem(pMenu, 2)->max = FUN_00406710();
        if (Menu_GetItem(pMenu, 2)->max != 0)
            Menu_GetItem(pMenu, 2)->max -= 4;
    }
    FUN_004ee6e0(pMenu);
}

// Item action of the two-player rally menu: selects the rally and stage and
// moves on to the driver selection.
// FUNCTION: CMR2 0x004ee600
void FUN_004ee600(Menu *pMenu, int param)
{
    if ((Menu_GetItem(pMenu, 1)->min == 5 && Menu_GetItem(pMenu, 1)->max == 4) ||
        (Menu_GetItem(pMenu, 1)->min == 9 && Menu_GetItem(pMenu, 1)->max == 8) ||
        (Menu_GetItem(pMenu, 1)->min == 0xb && Menu_GetItem(pMenu, 1)->max == 0xa))
        RallyData_FUN_004068e0(10);
    else
        RallyData_FUN_004068e0(Menu_GetItem(pMenu, 1)->max);
    RallyData_FUN_004068b0(Menu_GetItem(pMenu, 0)->max);
    if (Menu_GetItem(pMenu, 2)->max != 0)
        FUN_00406720(Menu_GetItem(pMenu, 2)->max + 4);
    else
        FUN_00406720(Menu_GetItem(pMenu, 2)->max);
    RallyData_FUN_0040df60(1, 0);
    FUN_004f8470()->pParent = pMenu;
    Menu_SetNextAction((int)FUN_004f8470());
    FUN_004ec460();
}

// Initialises the rally/stage select menu from the unlocked rally count.
// FUNCTION: CMR2 0x004ee850
void FUN_004ee850(Menu *pMenu, int param)
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
    Menu_GetItem(pMenu, 1)->max = FUN_00406750() - 1;
}

void FUN_00406740(int param1);
void FUN_00406760(int param1);

// Item action of the single-player rally menu: selects the rally and stage
// picked in the menu.
// FUNCTION: CMR2 0x004ee9b0
void FUN_004ee9b0(Menu *pMenu, int param)
{
    int order[8] = { 6, 3, 1, 4, 0, 2, 5, 7 };
    int i;

    i = 0;
    for (i = 0; i < 8; i++) {
        if (order[i] == g_unk0x00818ed4[Menu_GetItem(pMenu, 0)->max])
            break;
    }
    RallyData_FUN_004068e0(0);
    RallyData_FUN_0040d600(i / 3);
    RallyData_FUN_00406960(i % 3);
    RallyData_FUN_0040d620(Menu_GetItem(pMenu, 1)->max + 1);
    FUN_00406760(Menu_GetItem(pMenu, 1)->max + 1);
    FUN_00406740(g_unk0x00818ed4[Menu_GetItem(pMenu, 0)->max]);
    FUN_004f8470()->pParent = pMenu;
    Menu_SetNextAction((int)FUN_004f8470());
    FUN_004ec460();
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

// Update callback of the network session menu: pumps the DirectPlay message
// queue.
// FUNCTION: CMR2 0x004eec30
void FUN_004eec30(Menu *pMenu)
{
    FUN_004ec8d0();
}

void FUN_00406740(int param1);

// Item action of the championship rally menu: stores the rally and stage
// picked in the menu.
// FUNCTION: CMR2 0x004eec40
void FUN_004eec40(Menu *pMenu, int param)
{
    int order[8] = { 6, 3, 1, 4, 0, 2, 5, 7 };
    int i;

    i = 0;
    for (i = 0; i < 8; i++) {
        if (order[i] == g_unk0x00818ed4[Menu_GetItem(pMenu, 0)->max])
            break;
    }
    RallyData_FUN_004068e0(0);
    RallyData_FUN_0040d600(i / 3);
    RallyData_FUN_00406960(i % 3);
    FUN_00406740(g_unk0x00818ed4[Menu_GetItem(pMenu, 0)->max]);
    if (Menu_GetItem(pMenu, 1)->max != 0)
        FUN_00406720(Menu_GetItem(pMenu, 1)->max + 4);
    else
        FUN_00406720(Menu_GetItem(pMenu, 1)->max);
    FUN_004f8470()->pParent = pMenu;
    Menu_SetNextAction((int)FUN_004f8470());
    FUN_004ec460();
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

void FUN_004d05f0(void);
void FUN_004ea9f0(void);
void FUN_004f92e0(int value);
int FUN_004a10b0(BYTE index, char *pPassword, BYTE *pInvalidPassword);

// Callback that joins a DirectPlay session with the network settings of the
// menu: sends the local player description and opens the session or the lobby.
// FUNCTION: CMR2 0x004eee60
void FUN_004eee60(Menu *pMenu, int param)
{
    BYTE invalidPassword;

    g_unk0x00818ce4 = 0;
    FUN_00409a30();
    FUN_004d05f0();
    g_unk0x00818ef4 = 0;
    if (FUN_004a10b0((BYTE)g_unk0x00525288, g_unk0x00818da8, &invalidPassword) != 0) {
        FUN_004ea8e0((BYTE)Session_GetUserValue(0));
        FUN_00406780(Session_GetUserValue(2));
        FUN_004a1a00();
        RallyData_FUN_004086b0(0);
        if (FUN_004eca10() != 0) {
            Menu *pLobby = FUN_004f8450();
            Menu_SetParent(FUN_004f8470(), pLobby);
            Menu_SetNextAction((int)FUN_004f8470());
        } else {
            FUN_004a1280();
            FUN_004f92e0(3);
            Menu_SetNextAction((int)FUN_004f9360());
        }
        return;
    }
    if (invalidPassword != 0) {
        Menu_SetNextAction((int)FUN_004f88d0());
        return;
    }
    FUN_004a1280();
    FUN_004f92e0(1);
    Menu_SetNextAction((int)FUN_004f9360());
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


// Item action of a leaderboard entry: shows the leaderboard of the slot the
// item stands for and goes back to the list.
// FUNCTION: CMR2 0x004eefb0
void FUN_004eefb0(Menu *pMenu, int param)
{
    CNetworkLeaderboards::SetLeaderboardId(Menu_GetItem(pMenu, 0)->max);
    Menu_GoBack(pMenu);
}

// Adds a new leaderboard while fewer than 0x20 slots are in use.
// FUNCTION: CMR2 0x004eefe0
void FUN_004eefe0(Menu *pMenu, int param)
{
    if (CNetworkLeaderboards::GetTotalLeaderboards() < 0x20) {
        CNetworkLeaderboards::AddLeaderboard();
        CNetworkLeaderboards::SetLeaderboardId(
            CNetworkLeaderboards::GetTotalLeaderboards() - 1);
    }
}

// Keeps the leaderboard list consistent after the selected entry changes.
// FUNCTION: CMR2 0x004ef000
void FUN_004ef000(Menu *pMenu, int param)
{
    if (CNetworkLeaderboards::GetTotalLeaderboards() > 0)
        CNetworkLeaderboards::RemoveLeaderboard(Menu_GetItem(pMenu, 0)->max);
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
// match 87%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004ef190
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
    } while ((int)&p->pMenu < (int)&g_unk0x00819754);
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

void FUN_004d2070(BYTE param1, BYTE param2, BYTE param3);

// Callback of the rally options screen: arms the rally data flag from the game
// mode and then either starts a fresh session or requests the screen change.
// FUNCTION: CMR2 0x004ef590
void FUN_004ef590(Menu *pMenu)
{
    if (CGameInfo::FUN_00405d70() == 2 && CGameInfo::FUN_00405d80() == 6)
        RallyData_FUN_0040e330(1);
    else
        RallyData_FUN_0040e330(0);
    if (CGameInfo::FUN_00406320() != 0) {
        FUN_004e9e40();
        FUN_004d2070(0, 0, 1);
    } else {
        FUN_004d2070(1, 0, 0);
    }
}

// Callback that clears the debug overlay channels.
// FUNCTION: CMR2 0x004ef5e0
void FUN_004ef5e0(Menu *pMenu)
{
    FUN_004d2070(0, 0, 0);
}

// FUNCTION: CMR2 0x004ef5f0
void FUN_004ef5f0(Menu *pMenu, int param)
{
    RallyData_FUN_0040d620(1);
}

void RallyData_FUN_004068e0(BYTE param1);

// Callback of the rally options menu: copies the current cursor to the
// matching options sub-menu and seeds its value from the game info.
// FUNCTION: CMR2 0x004ef600
void FUN_004ef600(Menu *pMenu, int param)
{
    switch (pMenu->cursor) {
    case 0:
        FUN_004ea8e0(0);
        RallyData_FUN_004068b0(0);
        RallyData_FUN_004068e0(0);
        if (CGameInfo::FUN_00405d70() < 5) {
            FUN_004f82e0()->items[0].max = CGameInfo::FUN_00405d70() - 1;
            return;
        }
        FUN_004f82e0()->items[0].max = 0;
        return;
    case 1:
        FUN_004ea8e0(1);
        RallyData_FUN_004068e0(0);
        if (CGameInfo::FUN_00405d70() < 5) {
            FUN_004f82f0()->items[0].max = CGameInfo::FUN_00405d70() - 1;
            return;
        }
        FUN_004f82f0()->items[0].max = 0;
        return;
    case 2:
        FUN_004ea8e0(2);
        if (CGameInfo::FUN_00405d70() < 5) {
            FUN_004f8300()->items[0].max = CGameInfo::FUN_00405d70() - 1;
            return;
        }
        FUN_004f8300()->items[0].max = 0;
        return;
    case 3:
        FUN_004ea8e0(3);
        if (CGameInfo::FUN_00405d70() < 5) {
            FUN_004f8310()->items[0].max = CGameInfo::FUN_00405d70() - 1;
            return;
        }
        FUN_004f8310()->items[0].max = 0;
        return;
    case 4:
        FUN_004ea8e0(4);
        if (CGameInfo::FUN_00405d70() < 9) {
            FUN_004f8320()->items[0].max = CGameInfo::FUN_00405d70() - 1;
            return;
        }
        FUN_004f8320()->items[0].max = 0;
        break;
    }
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

void FUN_004eae90(unsigned int slot, char *pName);

// Enters (param != 0) or leaves the category name-entry screen, resetting the
// letter rows and the stored name.
// FUNCTION: CMR2 0x004f0d30
void FUN_004f0d30(Menu *pMenu, char param)
{
    FUN_004ea480(g_unk0x00819880);
    FUN_004a0c50(1);
    FUN_004e77b0(1);
    if (param) {
        pMenu->cursor = 2;
        pMenu->items[2].max = pMenu->items[2].min - 1;
    } else {
        pMenu->cursor = 0;
        pMenu->items[0].max = 0;
        FUN_004eae90(FUN_004f2be0(), CMain::m_logFileBlankLine);
    }
    FUN_004b7c80();
    g_unk0x0081986c = 0;
}
// Update callback of the "select stage" screen: when the row in the first
// item moves, the list slides sideways by the distance between the old and
// the new row (the shortest way round) and eases back to the middle.
// match 59%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004f0620
void FUN_004f0620(Menu *pMenu)
{
    int index;
    int old;
    int other;
    int steps;
    unsigned int elapsed;

    old = g_unk0x00819044;
    index = pMenu->items[0].max;
    if (index != old) {
        if (old != -1) {
            if (index > old) {
                other = index - old;
                steps = pMenu->items[0].min - index + old;
            } else {
                steps = old - index;
                other = pMenu->items[0].min - old + index;
            }
            if (steps < other) {
                while (steps > 0) {
                    g_unk0x00819868 += (int)(g_pGraphics->resX * -0x50) / 640;
                    steps--;
                }
            } else if (steps > other) {
                while (other > 0) {
                    g_unk0x00819868 += (int)(g_pGraphics->resX * 0x50) / 640;
                    other--;
                }
            } else {
                if (pMenu->moveFlags & 2)
                    other = g_pGraphics->resX * -5;
                else
                    other = g_pGraphics->resX * 5;
                g_unk0x00819868 += (other << 4) / 640;
            }
            g_unk0x00819884 = g_unk0x00819868;
        }
        g_unk0x0081975c = CFrontend::FUN_004d20e0();
        g_unk0x00819044 = pMenu->items[0].max;
    }
    if (g_unk0x00819044 != -1) {
        elapsed = CFrontend::FUN_004d20e0() - g_unk0x0081975c;
        if (elapsed > 0xfa) {
            g_unk0x00819868 = 0;
            return;
        }
        if (g_unk0x00819884 > 0)
            g_unk0x00819868 = g_unk0x00819884 - elapsed * g_unk0x00819884 / 0xfa;
        else
            g_unk0x00819868 = -(elapsed * g_unk0x00819884) / 0xfa + g_unk0x00819884;
    }
}

// FUNCTION: CMR2 0x004f0e60
void FUN_004f0e60(Menu *pMenu, int param)
{
    FUN_004ea480(0);
    FUN_004a0c50(0);
}

int FUN_004f2c00(void);
void FUN_004eae90(unsigned int slot, char *pName);

// Characters of the three name-picker rows (a-j, k-t and u-z plus '.', space
// and the delete/OK entries).
// GLOBAL: CMR2 0x00525374
char g_nameRow0x00525374[12] = "abcdefghij";
// GLOBAL: CMR2 0x00525380
char g_nameRow0x00525380[12] = "klmnopqrst";
// GLOBAL: CMR2 0x0052538c
char g_nameRow0x0052538c[12] = "uvwxyz. <_";

// Item picker of the name entry screens: appends the character of the picked
// column to the name; the '<' entry deletes the last character and '_'
// accepts the name.
// match 73%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004f1040
void FUN_004f1040(Menu *pMenu, int param)
{
    char *chars;
    int len;

    strcpy(CFrontend::m_stringDest, (char *)RallyData_GetRecord(FUN_004f2be0()));
    len = strlen(CFrontend::m_stringDest);
    if (pMenu->items[pMenu->cursor].value == 2 && g_nameRow0x0052538c[pMenu->items[2].max] == '<') {
        if (CFrontend::m_stringDest[0] != 0) {
            *(CFrontend::m_stringDest + strlen(CFrontend::m_stringDest) - 2) = 0;
            Menu_PlaySoundId(2);
        }
    } else if (pMenu->items[pMenu->cursor].value == 2 && g_nameRow0x0052538c[pMenu->items[2].max] == '_') {
        if (len > 0)
            Menu_SetNextAction(FUN_004f2c00());
        return;
    } else if (len < 3) {
        switch (pMenu->items[pMenu->cursor].value) {
        case 0:
            chars = g_nameRow0x00525374;
            break;
        case 1:
            chars = g_nameRow0x00525380;
            break;
        case 2:
            chars = g_nameRow0x0052538c;
            break;
        default:
            // the original uses the menu pointer as the character table
            chars = (char *)pMenu;
            break;
        }
        CFrontend::m_stringDest[len] = chars[pMenu->items[pMenu->cursor].max];
        CFrontend::m_stringDest[len + 1] = 0;
        if (len == 2) {
            pMenu->cursor = len;
            pMenu->items[2].max = 9;
        }
        Menu_PlaySoundId(1);
    } else {
        Menu_PlaySoundId(3);
    }
    FUN_004eae90(FUN_004f2be0(), CFrontend::m_stringDest);
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

void FUN_004ebe80(int index);
void FUN_004f2bf0(int value);
int FUN_004f2c20(void);
BYTE FUN_004eb290(int param_1, BYTE *param_2);
char FUN_004eb370(int param_1);
unsigned int FUN_004eb4c0(int param_1, int param_2);
void FUN_004ebe10(int param_1, unsigned int param_2, unsigned int param_3, unsigned int param_4);
BYTE FUN_004085a0(BYTE param1);
BYTE *FUN_004f4e50(int index);
unsigned int FUN_004fb400(BYTE *pBlock);
extern int g_unk0x00819744;
extern BYTE g_unk0x00819879;

// Item picker of the second name-entry screen: appends the picked character of
// the three letter rows to the name; the '<' entry deletes the last character
// and '_' accepts the entry (when its buffer is a known cheat code, the cheat
// is applied).
// match 76%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// logic verified against the dump; the remaining diff is register assignment (the original frees an
// extra callee-saved register and materialises the sound id with two pushes instead of a branchless
// select) and MSVC resolving `m_stringDest - 2` to a different symbol on our side.
// FUNCTION: CMR2 0x004f13c0
void FUN_004f13c0(Menu *pMenu, int param)
{
    char *chars;
    char key;
    int len;

    strcpy(CFrontend::m_stringDest, (char *)FUN_00408470(FUN_004f2be0()));
    len = strlen(CFrontend::m_stringDest);
    if (pMenu->items[pMenu->cursor].value == 2 && g_nameRow0x0052538c[pMenu->items[2].max] == '<') {
        if (CFrontend::m_stringDest[0] != 0) {
            *(CFrontend::m_stringDest + strlen(CFrontend::m_stringDest) - 1) = 0;
            Menu_PlaySoundId(2);
        }
    } else if (pMenu->items[pMenu->cursor].value == 2 && g_nameRow0x0052538c[pMenu->items[2].max] == '_') {
        if (FUN_004eb290(FUN_004f2be0(), (BYTE *)&key) != 0) {
            Menu_PlaySoundId(key != 0 ? 0 : 3);
            if (FUN_004085a0((BYTE)FUN_004f2be0())) {
                g_unk0x00819744--;
                FUN_004eb000(FUN_004f2be0(), 0);
            }
            FUN_004ebf20(FUN_004f2be0());
            FUN_004ebe80(FUN_004f2be0());
            g_unk0x00819048++;
            if (FUN_004d27d0())
                Menu_SetNextAction((int)FUN_004f8480());
            else
                Menu_SetNextAction((int)FUN_004f83a0());
            g_unk0x00819038 = 0;
            return;
        }
        if (len > 0) {
            Menu_SetNextAction((int)FUN_004f83e0());
            g_unk0x00819038 = 0;
        }
        return;
    } else if (len < 0x2b) {
        switch (pMenu->items[pMenu->cursor].value) {
        case 0:
            chars = g_nameRow0x00525374;
            break;
        case 1:
            chars = g_nameRow0x00525380;
            break;
        case 2:
            chars = g_nameRow0x0052538c;
            break;
        default:
            // the original uses the menu pointer as the character table
            chars = (char *)pMenu;
            break;
        }
        CFrontend::m_stringDest[len] = chars[pMenu->items[pMenu->cursor].max];
        CFrontend::m_stringDest[len + 1] = 0;
        Menu_PlaySoundId(1);
    } else {
        Menu_PlaySoundId(3);
    }
    FUN_004eaf90(FUN_004f2be0(), CFrontend::m_stringDest);
    strcpy((char *)g_unk0x008190f4, CFrontend::m_stringDest);
}

// Left/right item callback of the name-entry screen: recomputes the column
// offsets of the three letter rows and steers the next screen when the name is
// complete.
// FUNCTION: CMR2 0x004f15d0
void FUN_004f15d0(Menu *pMenu, int param)
{
    FUN_004ebe10(FUN_004f2be0(), pMenu->items[2].max + 1, pMenu->items[1].max + 1,
                 pMenu->items[0].max);
    if (FUN_004eb370(FUN_004f2be0()) != 0) {
        Menu_SetNextAction((int)FUN_004f2c20());
        g_unk0x00819879 = 0;
        if (FUN_004d27d0()) {
            FUN_004eb000(FUN_004f2be0(), 0);
            FUN_004ebe80(FUN_004f2be0());
        }
    }
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


// Maps a screen index to its palette/colour id and stores it for the current mode.
void RallyData_FUN_00408600(BYTE index, BYTE value);

// Update callback of the palette menu: shows the colour of the palette the
// profile uses and resets the scrolled list.
// FUNCTION: CMR2 0x004f1960
void FUN_004f1960(Menu *pMenu, char param)
{
    pMenu->items[0].max = FUN_004086f0(CGameInfo::FUN_00405d70() + (0xff - g_unk0x00819048));
    FUN_004ea480((CGameInfo::FUN_00405d70() & 0xff) - (g_unk0x00819048 & 0xff) - 1);
    if (param != 0 && CGameInfo::FUN_00405d80() != 4)
        FUN_004f17d0(FUN_004f2500()->pMenu, 0);
}

// Keeps the palette screen's OK item disabled while the palette id is invalid.
// FUNCTION: CMR2 0x004f19d0
void FUN_004f19d0(Menu *pMenu, int param)
{
    if (FUN_004086f0(CGameInfo::FUN_00405d70() + (0xff - g_unk0x00819048)) != 0)
        pMenu->items[0].max = 1;
    else
        pMenu->items[0].max = 0;
}
void RallyData_FUN_00408b10(int index, unsigned int *pHue, unsigned int *pValue, unsigned int *pShade);

// Callback of the car colour menu: reads the current driver's category colour
// and reflects it onto the colour picker items.
// match 72%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004f16f0
void FUN_004f16f0(Menu *pMenu, int param)
{
    int shade;
    int value;
    int hue;
    BYTE idx;

    FUN_004ea480((CGameInfo::FUN_00405d70() & 0xff) - (g_unk0x00819048 & 0xff) - 1);
    RallyData_FUN_00408b10(FUN_004f2be0(), (unsigned int *)&hue, (unsigned int *)&value, (unsigned int *)&shade);
    pMenu->items[2].max = 0;
    pMenu->cursor = 0;
    pMenu->items[0].max = value;
    idx = (BYTE)shade - 1;
    pMenu->items[1].max = idx;
    switch (idx) {
    case 3:
    case 5:
    case 8:
    case 0xa:
        pMenu->items[2].min = 0x1e;
        return;
    case 1:
        break;
    case 0:
    case 2:
    case 4:
    case 6:
    case 7:
    case 9:
    case 0xb:
        pMenu->items[2].min = 0x1f;
        return;
    default:
        return;
    }
    if (value % 4 == 0)
        pMenu->items[2].min = 0x1d;
    else
        pMenu->items[2].min = 0x1c;
}
// Enter callback of a rally menu: lists the stages of the rally that are in
// the save as text ids, points the cursor at the one that is selected in the
// rally data and measures them for the scroller.
// match 86%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004f17d0
void FUN_004f17d0(Menu *pMenu, int param)
{
    MenuScroller *pScroller;
    MenuItem *pItem;
    MenuItem *p;
    int *pPalette;
    int *pWidths;
    int i;
    int count;

    count = 0;
    FUN_004ea480(CGameInfo::FUN_00405d70() - g_unk0x00819048 - 1);
    pScroller = FUN_004f2500();
    pScroller->pMenu = pMenu;
    pPalette = g_unk0x008196e8;
    pItem = pMenu->items;
    pMenu->cursor = 0;
    for (i = 0, p = pItem; i < 0x16; i++) {
        if (RallyData_FUN_00408e30(CGameInfo::FUN_00405d70() - g_unk0x00819048 - 1, i, 1)) {
            *pPalette = i;
            p->id = i + 0x98;
            if (i == (RallyData_FUN_004086b0(CGameInfo::FUN_00405d70() - g_unk0x00819048 - 1) & 0xff))
                pMenu->cursor = count;
            count++;
            pPalette++;
            p++;
        }
    }
    pMenu->itemCount = count;
    pScroller = FUN_004f2500();
    pScroller->offset = 0;
    pScroller->startOffset = 0;
    pScroller->current = pMenu->cursor;
    pScroller->previous = pMenu->cursor;
    pScroller->spacing = PATH_X();
    pScroller->count = pMenu->itemCount;
    pScroller->offset = 0;
    pScroller->startOffset = 0;
    for (i = 0, p = pItem, pWidths = g_menuScroller0x00819230.widths; i < pScroller->count; i++) {
        strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(p->id));
        CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
        *pWidths = Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest);
        pWidths++;
        p++;
    }
}

void FUN_004ea9f0(void);
void FUN_004f02e0(void);
void FUN_004f0ac0(Menu *pMenu, int param);

// Item action of the palette menu: opens the palette editor of the current
// screen mode.
// FUNCTION: CMR2 0x004f1a40
void FUN_004f1a40(Menu *pMenu, int param)
{
    FUN_004ea480(0);
    FUN_004eb0c0(CGameInfo::FUN_00405d70() + (0xff - g_unk0x00819048), pMenu->items[0].max);
    if (g_unk0x00819048 == 0) {
        switch (CGameInfo::FUN_00405d80()) {
        case 0:
            Menu_SetNextAction((int)FUN_004f8330());
            FUN_004ea9f0();
            return;
        case 1:
        case 2:
        case 3:
            FUN_004f02e0();
            Menu_SetParent(FUN_004f8340(), pMenu);
            Menu_SetNextAction((int)FUN_004f8340());
            return;
        case 4:
            Menu_SetParent(FUN_004f8420(), pMenu);
            Menu_SetNextAction((int)FUN_004f8420());
            return;
        }
    } else {
        Menu_SetParent(FUN_004f83a0(), pMenu);
        if (CGameInfo::FUN_00405d80() != 4) {
            Menu_SetNextAction((int)FUN_004f83a0());
            return;
        }
        FUN_004f0ac0(pMenu, param);
        Menu_SetNextAction((int)FUN_004f83c0());
    }
}

// FUNCTION: CMR2 0x004f1a10
void FUN_004f1a10(Menu *pMenu, int unused)
{
    RallyData_FUN_00408600(
        CGameInfo::FUN_00405d70() + (0xff - g_unk0x00819048),
        (unsigned char)g_unk0x008196e8[pMenu->cursor]);
}

// Item callback of the championship entry screens: toggles the selected
// entry's championship flag and moves to the next or the parent menu.
// FUNCTION: CMR2 0x004f1b30
void FUN_004f1b30(Menu *pMenu, int param)
{
    FUN_004eb0c0(CGameInfo::FUN_00405d70() + (0xff - g_unk0x00819048), pMenu->items[0].max);
    if (g_unk0x00819048 == 0) {
        FUN_004f02e0();
        Menu_SetNextAction((int)FUN_004f8420());
        return;
    }
    Menu_SetParent(FUN_004f83a0(), pMenu);
    Menu_SetNextAction((int)FUN_004f83a0());
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
void FUN_004f1bb0(int value)
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

DWORD FUN_004b7530(void);
DWORD FUN_004b7540(void);
DWORD FUN_004b7550(void);

// Callback of the options menu: enables the items supported by the current
// device and puts the cursor on the active option group.
// FUNCTION: CMR2 0x004f1fa0
void FUN_004f1fa0(Menu *pMenu, int param)
{
    pMenu->items[0].min = 4;
    pMenu->items[0].enabled |= 1;
    pMenu->items[1].enabled = FUN_004b7530();
    pMenu->items[2].enabled = FUN_004b7530() && FUN_004b7540();
    pMenu->items[3].enabled = FUN_004b7530() && FUN_004b7550();
    switch (CGameInfo::FUN_00405b50()) {
    case 1:
        pMenu->cursor = 1;
        return;
    case 2:
        pMenu->cursor = 2;
        return;
    case 3:
        pMenu->cursor = 3;
        return;
    default:
        pMenu->cursor = 0;
        return;
    }
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

void FUN_004f4d80(void);

// Item action of the video menu: switches the game to the display mode
// selected in the first row and rebuilds the frontend on top of it.
// FUNCTION: CMR2 0x004f2430
void FUN_004f2430(Menu *pMenu, int param)
{
    DWORD width;
    DWORD height;
    DWORD depth;
    int mode;

    CGraphics::GetDisplayMode(pMenu->items[0].max, &width, &height, &depth);
    CGameInfo::SetScreenWidth(width);
    CGameInfo::SetScreenHeight(height);
    CGameInfo::SetColourDepth(depth);
    CMain::FUN_004a9a50(1);
    mode = CGraphics::FUN_004a8d80();
    CGraphics::FUN_004a78a0(width, height, depth, CGameInfo::FUN_00405bd0(), mode);
    CGameInfo::SetScreenWidth(g_pGraphics->resX);
    CGameInfo::SetScreenHeight(g_pGraphics->resY);
    CGameInfo::SetColourDepth(g_pGraphics->depth);
    CFrontend::FUN_004d21e0();
    Graphics_ReloadAllTextures();
    FUN_004f4d80();
    FUN_004ef190();
    CMain::FUN_004a9a50(0);
    Menu_SetNextAction((int)pMenu->pParent);
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


// Applies the front-end options from the menu items back to the game info.
void FUN_004eab20(BYTE param1);
void FUN_004eaa40(unsigned int param1);

// FUNCTION: CMR2 0x004f2d20
void FUN_004f2d20(Menu *pMenu, int param)
{
    int index;

    index = Menu_FindItem(pMenu, 0);
    CGameInfo::FUN_00406340(pMenu->items[index].max);
    index = Menu_FindItem(pMenu, 2);
    FUN_004eab20(1 - pMenu->items[index].max);
    index = Menu_FindItem(pMenu, 3);
    CGameInfo::FUN_00405ec0(pMenu->items[index].max);
    index = Menu_FindItem(pMenu, 4);
    FUN_004eaa40(pMenu->items[index].max);
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
// match 58%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
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

// Applying the 3D car preview screen: when the preview is up, the block of the
// selected car is handed to the frontend state machine.
// FUNCTION: CMR2 0x004f3a00
void FUN_004f3a00(Menu *pMenu, int param)
{
    char ok;

    if (g_unk0x0082aa40 > 0) {
        ok = FUN_004fb400(FUN_004f4e50(g_unk0x0082a924));
        if (ok != 0)
            FUN_004d2070(0, 1, 0);
    }
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
// FUNCTION: CMR2 0x004f3bb0
void FUN_004f3bb0(void)
{
    g_unk0x0081988c = -1;
    CInput::FUN_0049ffc0((int)(CGameInfo::FUN_00405e70() << 16) / 100 / 4);
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
// match 54%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004f3c10
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
// FUNCTION: CMR2 0x004f3dd0
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
    } while ((int)pPos < (int)&g_menuPathMode);
    memset(g_menuTrailPos, 0, sizeof(g_menuTrailPos));
    memset(g_menuStreamSpeed, 0, sizeof(g_menuStreamSpeed));
    k = 0;
    pPos = g_menuStreamPos[0];
    do {
        for (j = 0; j < 10; j++)
            pPos[j] = k / 6;
        pPos += 10;
        k += 0x4000;
    } while ((int)pPos < (int)&g_menuPathVariant);
    g_menuAnimTime = -1;
    switch (FUN_004f8410()->items[FUN_004f8410()->cursor].value) {
    case 0:
        g_menuPathVariant = g_menuPathPrevMode = g_menuPathMode = 0;
        break;
    case 1:
        g_menuPathVariant = g_menuPathPrevMode = g_menuPathMode = 1;
        break;
    case 2:
        g_menuPathVariant = g_menuPathPrevMode = g_menuPathMode = 2;
        break;
    case 3:
        g_menuPathVariant = g_menuPathPrevMode = g_menuPathMode = 3;
        break;
    case 4:
        g_menuPathMode = 4;
        g_menuPathPrevMode = 4;
        g_menuPathVariant = (unsigned int)(CFrontend::FUN_004d20e0() - FUN_004f25c0()) / 500 % 3;
        break;
    case 5:
        g_menuPathVariant = g_menuPathPrevMode = g_menuPathMode = 7;
        break;
    case 6:
        g_menuPathVariant = g_menuPathPrevMode = g_menuPathMode = 8;
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
// match 72%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004f4050
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
// match 51%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004f4760
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
// FUNCTION: CMR2 0x004ef270
void FUN_004ef270(Menu *pMenu, char back)
{
    int i;

    if (back == 0) {
        FUN_004ea8a0(pMenu->cursor);
        FUN_004a3c30(pMenu->cursor);
        FUN_004f48b0();
        FUN_004f4910(0);
        if (CGameInfo::GetGameLanguage() == 0)
            CGameInfo::FUN_00405ec0(1);
        else
            CGameInfo::FUN_00405ec0(0);
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

// Selects the current player-profile slot of the player setup screen: resets
// the working copy and continues to the profile menu, or plays the error sound
// when the slot does not exist.
// FUNCTION: CMR2 0x004f26f0
void FUN_004f26f0(Menu *pMenu, int param)
{
    unsigned int index;
    int count;
    char ok;

    FUN_004f2bf0(0);
    FUN_004eb860(0, -1);
    FUN_004ebf20(0);
    FUN_004eb000(0, 0);
    index = pMenu->items[0].max;
    count = FUN_004eb440();
    if ((int)index < count) {
        ok = FUN_004eb4c0(0, index);
        if (ok != 0) {
            Menu_SetNextAction((int)FUN_004f8490());
            return;
        }
    } else {
        Menu_PlaySoundId(3);
    }
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
// match 84%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004f2620
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
// match 56%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004ded80
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

// Entering the display device menu: one entry per device, the current one
// under the cursor, and only the usable ones enabled.
// FUNCTION: CMR2 0x004f21c0
void FUN_004f21c0(Menu *pMenu, int param)
{
    MenuItem *pItem;
    unsigned int i;

    pMenu->itemCount = CGraphics::FUN_004a8be0();
    pMenu->cursor = CGameInfo::FUN_00405bd0();
    i = 0;
    if ((unsigned int)CGraphics::FUN_004a8be0() > 0) {
        pItem = pMenu->items;
        do {
            if (CGraphics::FUN_004a96c0(i) != 0)
                pItem->enabled = 1;
            else
                pItem->enabled = 0;
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
// FUNCTION: CMR2 0x004e3340
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
    switch (table) {
    case 0:
        Font_DrawText(2, CFrontend::GetTextString(0x167), (int)g_pGraphics->resX / 2, (int)(g_pGraphics->resY * 100) / 480,
                      (int *)g_colourWhite0x00524968, 10);
        break;
    case 1:
        Font_DrawText(2, CFrontend::GetTextString(0x168), (int)g_pGraphics->resX / 2, (int)(g_pGraphics->resY * 100) / 480,
                      (int *)g_colourWhite0x00524968, 10);
        break;
    case 2:
        Font_DrawText(2, CFrontend::GetTextString(0x169), (int)g_pGraphics->resX / 2, (int)(g_pGraphics->resY * 100) / 480,
                      (int *)g_colourWhite0x00524968, 10);
        break;
    default:
        goto header;
    }
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
        if ((pEntry->flags & 0x40) != 0)
            Font_DrawText(1, g_strGearboxAuto, (int)(g_pGraphics->resX * 430) / 640, y,
                          (int *)g_colourWhite0x00524968, 0x12);
        else
            Font_DrawText(1, g_strGearboxManual, (int)(g_pGraphics->resX * 430) / 640, y,
                          (int *)g_colourWhite0x00524968, 0x12);
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
// FUNCTION: CMR2 0x004e3a80
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
    switch (table) {
    case 0:
        Font_DrawText(2, CFrontend::GetTextString(0x167), (int)g_pGraphics->resX / 2, (int)(g_pGraphics->resY * 100) / 480,
                      (int *)g_colourWhite0x00524968, 10);
        break;
    case 1:
        Font_DrawText(2, CFrontend::GetTextString(0x168), (int)g_pGraphics->resX / 2, (int)(g_pGraphics->resY * 100) / 480,
                      (int *)g_colourWhite0x00524968, 10);
        break;
    case 2:
        Font_DrawText(2, CFrontend::GetTextString(0x169), (int)g_pGraphics->resX / 2, (int)(g_pGraphics->resY * 100) / 480,
                      (int *)g_colourWhite0x00524968, 10);
        break;
    default:
        goto header;
    }
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
    g_unk0x008189a8[2] = (int)(g_pGraphics->resX * 640) / 640 + (int)(g_pGraphics->resX * 30) / 640 * -2;
    g_unk0x008189a8[3] = 1;
    y = (int)(g_pGraphics->resY * 19) / 480 + (int)(g_pGraphics->resY * 200) / 480;
    offset = table * 0x10;
    if ((RallyData_FUN_00408cb0(0)[offset] & 0x80) != 0) {
        pSecond = (unsigned int *)(RallyData_FUN_00408cb0(0) + offset + 4);
        pRecord = RallyData_FUN_00408cb0(0) + offset;
        Font_DrawText(1, CFrontend::FUN_0040ede0(*(unsigned int *)pRecord & 0x3f), (int)(g_pGraphics->resX * 180) / 640, y,
                      (int *)g_colourWhite0x00524968, 0x12);
        if ((*(unsigned int *)pRecord & 0x40) != 0)
            Font_DrawText(1, g_strGearboxAuto, (int)(g_pGraphics->resX * 310) / 640, y,
                          (int *)g_colourWhite0x00524968, 0x12);
        else
            Font_DrawText(1, g_strGearboxManual, (int)(g_pGraphics->resX * 310) / 640, y,
                          (int *)g_colourWhite0x00524968, 0x12);
        sprintf(CFrontend::m_stringDest, g_strTwoDigits, (*pSecond & 0xf) + 1);
        Font_DrawText(3, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 370) / 640, y, (int *)g_colourWhite0x00524968, 0x12);
        sprintf(CFrontend::m_stringDest, g_strTwoDigits, *pSecond >> 6 & 0xff);
        Font_DrawText(3, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 435) / 640, y, (int *)g_colourWhite0x00524968, 0x12);
        Font_DrawText(0, (char *)FUN_004f9280(table), (int)(g_pGraphics->resX * 520) / 640, y, (int *)g_colourWhite0x00524968, 0x12);
    } else {
        Font_DrawText(1, CFrontend::GetTextString(0x171), (int)g_pGraphics->resX / 2, y, (int *)g_colourWhite0x00524968, 0x12);
    }
    g_unk0x008189a8[1] = (short)((int)(g_pGraphics->resY * 25) / 480) + (short)((int)(g_pGraphics->resY * 200) / 480);
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, g_colourText0x0052496c, 1);
    FrontendDraw_HelpText(CFrontend::GetTextString(0x16f), 1);
}

extern char g_loadRecordTimeFormat[];
BYTE *FUN_004f9240(int row, int column);

// Draw callback of the stage records page of a rally: for each stage (10,
// or 11 for every second rally) the record car, gearbox, time and holder.
// match 65%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004e5630
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

extern BYTE g_saveData[];
int FUN_004eaca0(void);
void FUN_004ec260(int player);

// 7x15 dot icon drawn by FUN_004d5ad0 (the rest of the block is unused).
// GLOBAL: CMR2 0x00523f30
char g_dotIconMap[216] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
    0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x01,
    0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x01, 0x01, 0x00,
    0x00, 0x01, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01,
};
// Ripple phase of the dot icons.
// GLOBAL: CMR2 0x008189b4
short g_dotIconPhase;
extern int g_unk0x00819744;

void FUN_004eae40(void);
int FUN_004d2bd0(int *pCentre, int x, int y, int phase, int wavelength);
void FUN_004eb860(int index, int profile);
void FUN_004ebf20(int index);
void FUN_004eb000(BYTE index, char set);
void FUN_004ec000(void);

// Draws one 7x15 dot icon at (x, y), each dot shaded by the ripple.
// match 71%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004d5ad0
void FUN_004d5ad0(int x, int y, short phase)
{
    BYTE colour[4];
    int centre[2];
    char *pDot;
    int u;
    int b;
    int col;
    int row;

    colour[0] = 0xff;
    colour[1] = 0xff;
    colour[2] = 0xff;
    colour[3] = 0xff;
    g_unk0x008189a8[2] = CFrontend::m_pSmMatrixTexture->width;
    g_unk0x008189a8[3] = CFrontend::m_pSmMatrixTexture->height;
    centre[0] = 0;
    centre[1] = 0;
    g_unk0x008189a8[0] = x;
    col = 0;
    do {
        row = 0;
        u = (int)(__int64)(((int)(g_pGraphics->resX * 18) / 640 * col + x) * CGraphics::m_65536) / (int)g_pGraphics->resX;
        g_unk0x008189a8[1] = y;
        pDot = &g_dotIconMap[col];
        do {
            if (*pDot != 0) {
                b = FUN_004d2bd0(centre, u,
                                 (int)(__int64)(((int)(g_pGraphics->resY * 18) / 480 * row + y) * CGraphics::m_65536)
                                     / (int)g_pGraphics->resX,
                                 phase, 0x140000) / 4 + 0xc000;
                colour[0] = FixMulShift32(b, 0xff0000);
                colour[1] = colour[0];
                colour[2] = colour[0];
                Sprite_Queue((SpriteRect *)&CFrontend::m_pSmMatrixTexture->field_0x11c, (SpriteRect *)g_unk0x008189a8,
                             CFrontend::m_pSmMatrixTexture, 1, 0, NULL, NULL, colour, 8);
            }
            g_unk0x008189a8[1] += (int)(g_pGraphics->resY * 8) / 480;
            row++;
            pDot += 7;
        } while (row < 15);
        g_unk0x008189a8[0] += (int)(g_pGraphics->resX * 8) / 640;
        col++;
    } while (col < 7);
}

// Draw callback of the difficulty pages: title from the game mode, the
// scroller and one dot icon per difficulty level up to the selected one.
// match 67%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004d5fb0
void FUN_004d5fb0(Menu *pMenu)
{
    char *text[2];
    int x;
    int y;
    int i;

    g_dotIconPhase = (short)(((CMain::GetFrameDelta() + 1) * -0x2000) / 360);
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
    FrontendDraw_PlayTime();
    FrontendDraw_MenuPath(pMenu, (int)(g_pGraphics->resX * 24) / 640, (int)(g_pGraphics->resY * 38) / 480, 1, 3, text, 2);
    FrontendDraw_ScrollerRow(FUN_004f2520(), 1);
    y = ((int)(g_pGraphics->resY * 8) / 480 - (int)(g_pGraphics->resY * 6) / 480) / 2
        - (int)(g_pGraphics->resY * 8) / 480 * 15 / 2 + (int)g_pGraphics->resY / 2;
    x = (int)g_pGraphics->resX / 2
        - (((int)(g_pGraphics->resX * 6) / 640 + (int)(g_pGraphics->resX * 8) / 640 * 6) * (pMenu->cursor + 1)
           + (int)(g_pGraphics->resX * 10) / 640 * pMenu->cursor) / 2;
    for (i = 0; i < pMenu->cursor + 1; i++) {
        FUN_004d5ad0(x, y, g_dotIconPhase);
        x += (int)(g_pGraphics->resX * 10) / 640 + (int)(g_pGraphics->resX * 6) / 640 + (int)(g_pGraphics->resX * 8) / 640 * 6;
    }
    FrontendDraw_HelpText(CFrontend::GetTextString(0x57), 1);
}

// Returns the three performance values (speed, acceleration, grip) of a car
// class: the fixed table values for the known classes, random ones otherwise.
// FUNCTION: CMR2 0x004d7c00
void FUN_004d7c00(int param_1, int *param_2, int *param_3, int *param_4)
{
    int scale = 0x75c2;
    int iVar2;
    int iVar3;

    switch (param_1) {
    case 0:
    case 3:
    case 4:
        param_1 = 0x570a;
        iVar2 = 1000;
        iVar3 = 0;
        break;
    case 6:
        param_1 = 0x63d7;
        iVar2 = 0x3a6;
        iVar3 = 0;
        break;
    case 7:
        param_1 = 0x4a3d;
        iVar2 = 0x3c7;
        iVar3 = 2;
        break;
    case 2:
        param_1 = 0x5687;
        iVar2 = 1000;
        iVar3 = 0;
        break;
    case 1:
        param_1 = 0x570a;
        iVar2 = 0x400;
        iVar3 = 0;
        break;
    case 8:
        param_1 = 0x3ae1;
        iVar2 = 0x1bf;
        iVar3 = 1;
        break;
    case 0xd:
        param_1 = 0x3ae1;
        iVar2 = 0x1bf;
        iVar3 = 2;
        break;
    case 5:
        param_1 = 0x5c28;
        iVar2 = 1000;
        iVar3 = 0;
        break;
    case 9:
        param_1 = 0x6666;
        iVar2 = 0x345;
        iVar3 = 0;
        break;
    case 0xb:
        param_1 = scale;
        iVar2 = 0x2fc;
        iVar3 = 0;
        break;
    case 10:
        param_1 = 0x570a;
        iVar2 = 0x2dc;
        iVar3 = 2;
        break;
    case 0xc:
        param_1 = 0x570a;
        iVar2 = 0x34e;
        iVar3 = 0;
        break;
    default:
        iVar2 = rand();
        param_1 = iVar2 % 0xffff;
        iVar2 = rand();
        iVar2 = iVar2 % 1000;
        iVar3 = rand();
        iVar3 = iVar3 % 3;
    }
    param_1 = FixDiv(param_1, scale);
    *param_2 = FixMulShift32(param_1, 0xb0000);
    *param_3 = iVar2 * 11 / 1024;
    *param_4 = iVar3;
}

// Draws the progress bar of a game mode page: param_3 small rects in a row,
// the first param_4 highlighted, and the label of param_5 underneath.
// FUNCTION: CMR2 0x004d8330
int FUN_004d8330(int param_1, int param_2, int param_3, int param_4, int param_5)
{
    short rect[4];
    int i;
    int x;
    int step;

    rect[1] = param_2;
    rect[2] = (int)(g_pGraphics->resX * 8) / 640;
    rect[3] = (int)(g_pGraphics->resY * 8) / 480;
    x = param_1 << 16;
    rect[0] = (short)(x >> 16);
    step = FixMul(g_pGraphics->resX << 16, FixDiv(0xa0000, 0x2800000));
    for (i = 0; i < param_3; i++) {
        if (i < param_4)
            Sprite_FillRect((int)g_pGraphics + 0x150, rect, g_colourWhite0x00524968, 1);
        else
            Sprite_FillRect((int)g_pGraphics + 0x150, rect, g_colourText0x0052496c, 1);
        x += step;
        rect[0] = (short)(x >> 16);
    }
    if (param_5 != 0) {
        Font_DrawText(1, (char *)param_5, *(int *)rect, (int)(g_pGraphics->resY * 0x18) / 0x1e0 + param_2,
                      (int *)g_colourText0x0052496c, 0x14);
        return rect[0];
    }
    return rect[0];
}

// Clears the championship data and sets up the 8 championship entries.
// FUNCTION: CMR2 0x004eae40
void FUN_004eae40(void)
{
    BYTE *pEntry;
    int i;

    memset(g_saveData, 0, 0x188 * 4);
    i = 0;
    pEntry = g_saveData + 0xc;
    do {
        *pEntry = (*pEntry & 0xfd) | 1;
        *(int *)(pEntry - 8) = FUN_004eaca0();
        FUN_004ec260(i);
        pEntry[1] = (pEntry[1] & 0xfc) | 0x3c;
        i++;
        pEntry += 0xc4;
    } while ((int)pEntry < (int)g_saveData + 0x62c);
}

extern BYTE *g_unk0x00531764;

// Toggles the championship entry bit of a driver slot; in game mode 4 it
// toggles the |2 flag of that slot's profile record instead.
// FUNCTION: CMR2 0x004eb0c0
void FUN_004eb0c0(BYTE index, BYTE flag)
{
    unsigned int category;
    BYTE *pEntry;

    RallyData_ValidateIndex(index);
    if (CGameInfo::FUN_00405d80() == 4) {
        pEntry = g_saveData + 0xc + index * 0xc4;
        *pEntry = ((*pEntry ^ flag) & 1) ^ *pEntry | 2;
        return;
    }
    category = (*(unsigned int *)(g_saveData + 0x1f70 + index * 0x30) >> 0x12) & 0xf;
    if (category != 0xf) {
        *(unsigned int *)(g_saveData + 0x67c + category * 0x650) =
            (*(unsigned int *)(g_saveData + 0x67c + category * 0x650) & 0xffffffdf) | (flag & 1) << 5;
        g_saveData[0x620 + category] = 1;
    }
}

void FUN_0040d090(int index, int seconds);
void RallyTiming_SetOverallTimeRaw(int iDriver, int iCentiseconds);
void RallyTiming_FUN_0040d0c0(void);

// Copies the recorded stage times of the pending drivers from the save block
// into the timing system.
// FUNCTION: CMR2 0x004eb160
void FUN_004eb160(void)
{
    int count;
    int i;
    int n;
    unsigned int index;
    BYTE *pRecord;

    count = 0;
    if (CGameInfo::FUN_00405d70() != 0) {
        pRecord = g_saveData + 0x1f74;
        i = 0xf;
        do {
            FUN_0040d090(i, (*(unsigned int *)(pRecord - 4) >> 6) & 0x7f);
            RallyTiming_SetOverallTimeRaw(i, *(int *)pRecord);
            count++;
            i--;
            pRecord += 0x30;
        } while (count < (int)(CGameInfo::FUN_00405d70() & 0xff));
    }
    n = 0x10 - (CGameInfo::FUN_00405d70() & 0xff);
    i = 0;
    while (i < n) {
        index = (CGameInfo::FUN_00405d70() & 0xff) + i;
        FUN_0040d090(i, (*(unsigned int *)(g_saveData + 0x1f70 + index * 0x30) >> 6) & 0x7f);
        RallyTiming_SetOverallTimeRaw(i, *(int *)(g_saveData + 0x1f74 + index * 0x30));
        i++;
    }
    RallyTiming_FUN_0040d0c0();
}

// True if the profile name of a driver matches the category record that the
// given championship entry points at.
// match 79%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004eb200
bool FUN_004eb200(int param_1, BYTE *param_2)
{
    unsigned int category;
    char *pName;

    RallyData_ValidateIndex(param_1);
    category = (*(unsigned int *)(g_saveData + 0x1f70 + param_1 * 0x30) >> 0x12) & 0xf;
    pName = (char *)(g_saveData + 0x628 + category * 0x650);
    CGenericFileLoader::StrLowerPolish((char *)param_2);
    return strcmp((char *)param_2, pName + 0x1c) == 0;
}

// True if the profile of the given index already matches the category record
// that one of the championship entries points at.
// FUNCTION: CMR2 0x004ebd60
bool FUN_004ebd60(int index)
{
    BYTE *pRecord;
    BYTE *pProfile;
    BYTE *pCategory;
    unsigned int category;
    unsigned int diff;

    pProfile = g_unk0x00531764 + index * 12;
    for (pRecord = g_saveData + 0x1f70; (int)pRecord < (int)(g_saveData + 0x2270); pRecord += 0x30) {
        category = (*(unsigned int *)pRecord >> 0x12) & 0xf;
        if (category == 0xf)
            continue;
        pCategory = g_saveData + 0x638 + category * 0x650;
        diff = *(unsigned int *)(pProfile + 4) ^ *(unsigned int *)(pCategory + 4);
        if ((diff & 0x1f0f00) != 0 || (char)diff != 0 || (diff & 0xfc0f000) != 0)
            continue;
        if (strcmp((char *)pProfile, (char *)pCategory) == 0)
            return 1;
    }
    return 0;
}

// Sets the colour word of the category record that the driver's entry points
// at. param_2 is the 5-bit shade index, param_3 the 4-bit hue.
// FUNCTION: CMR2 0x004ebe10
void FUN_004ebe10(int param_1, unsigned int param_2, unsigned int param_3, unsigned int param_4)
{
    unsigned int category;
    unsigned int colour;

    RallyData_ValidateIndex(param_1);
    category = (*(unsigned int *)(g_saveData + 0x1f70 + param_1 * 0x30) >> 0x12) & 0xf;
    colour = ((((param_2 & 0x1f) << 8) | (param_3 & 0xf)) << 8) |
             (*(unsigned int *)(g_saveData + 0x63c + category * 0x650) & 0xffe0f0ff);
    *(unsigned int *)(g_saveData + 0x63c + category * 0x650) = ((colour ^ param_4) & 0xff) ^ colour;
}

// Next player: gives the player a profile and goes to the name entry (or
// for championship mode 4 to the championship screen).
// FUNCTION: CMR2 0x004f0ac0
void FUN_004f0ac0(Menu *pMenu, int param)
{
    Menu *pNext;

    g_unk0x00819744++;
    FUN_004f2bf0((CGameInfo::FUN_00405d70() & 0xff) - (g_unk0x00819048 & 0xff));
    if (CGameInfo::FUN_00405d80() != 4) {
        switch (CGameInfo::FUN_00405d80()) {
        case 5:
            pNext = FUN_004fa300();
            break;
        case 6:
            pNext = FUN_004fa310();
            break;
        case 7:
            pNext = FUN_004fa320();
            break;
        default:
            pNext = FUN_004f83f0();
            break;
        }
        FUN_004eb860((CGameInfo::FUN_00405d70() & 0xff) - (g_unk0x00819048 & 0xff), -1);
        FUN_004ebf20((CGameInfo::FUN_00405d70() & 0xff) - (g_unk0x00819048 & 0xff));
        FUN_004eb000(CGameInfo::FUN_00405d70() - g_unk0x00819048, 0);
        Menu_SetParent(FUN_004f83c0(), pMenu);
        FUN_004eb000(CGameInfo::FUN_00405d70() - g_unk0x00819048, 1);
        if (CGameInfo::FUN_00405e00() != 0) {
            FUN_004f2c10((int)FUN_004f8440());
        } else {
            FUN_004f2c10((int)pNext);
            Menu_SetParent(pNext, pMenu);
        }
    } else {
        Menu_SetParent(FUN_004f83c0(), pMenu);
        FUN_004f2c10((int)FUN_004f8400());
        FUN_004f8430();
    }
    sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0xdc),
            (CGameInfo::FUN_00405d70() & 0xff) - (g_unk0x00819048 & 0xff) + 1);
    FUN_004e7780(CFrontend::m_stringDest);
    g_unk0x00819048--;
    Menu_SetNextAction((int)FUN_004f83c0());
}

// Item callback of a difficulty: stores it and moves on to the next screen
// of the game mode.
// FUNCTION: CMR2 0x004ef7c0
void FUN_004ef7c0(Menu *pMenu, int param)
{
    FUN_004ea8c0(pMenu->cursor + 1);
    FUN_004ec000();
    CSound::FUN_004a28c0();
    if (CGameInfo::FUN_00405d80() == 4) {
        FUN_004eae40();
        FUN_004ea950(0);
        FUN_004f8400()->pfnCallback1 = NULL;
        FUN_004f8400()->pParent = FUN_004f83c0();
        Menu_SetParent(FUN_004f83c0(), pMenu);
        g_unk0x00819048 = CGameInfo::FUN_00405d70();
        FUN_004f0ac0(pMenu, param);
        return;
    }
    FUN_004f8400()->pfnCallback1 = (MenuCallback)FUN_004f39d0;
    FUN_004f8400()->pParent = FUN_004f83f0();
    if (CGameInfo::FUN_00405d70() == 1) {
        FUN_004ea950(0);
    } else {
        if (CGameInfo::FUN_00405d80() != 3 && CGameInfo::FUN_00405d70() == 2) {
            switch (CGameInfo::FUN_00405d80()) {
            case 0:
                Menu_SetNextAction((int)FUN_004f8370());
                return;
            case 1:
                Menu_SetNextAction((int)FUN_004f8380());
                return;
            case 2:
                Menu_SetNextAction((int)FUN_004f8390());
                return;
            }
            return;
        }
        FUN_004ea950(1);
    }
    g_unk0x00819870 = (int)pMenu;
    Menu_SetParent(FUN_004f83a0(), pMenu);
    g_unk0x00819048 = CGameInfo::FUN_00405d70();
    Menu_SetNextAction((int)FUN_004f83a0());
}

// Entering a difficulty page: sets up its scroller.
// FUNCTION: CMR2 0x004f3610
void FUN_004f3610(Menu *pMenu, int param)
{
    MenuScroller *p;
    int k;

    p = FUN_004f2520();
    p->startTime = CFrontend::FUN_004d20e0();
    p->pMenu = pMenu;
    p->count = pMenu->itemCount;
    for (k = 0; k < pMenu->itemCount; k++) {
        strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(pMenu->items[k].id));
        CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
        p->widths[k] = Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest);
    }
    p->offset = 0;
    p->startOffset = 0;
    p->current = pMenu->cursor;
    p->previous = pMenu->cursor;
}

// Update callback of the difficulty pages.
// FUNCTION: CMR2 0x004f39f0
void FUN_004f39f0(Menu *pMenu)
{
    FUN_004f37c0(FUN_004f2520());
}

// Entering the 8-level difficulty page: sets up its scroller with all 8
// levels enabled.
// FUNCTION: CMR2 0x004f3530
void FUN_004f3530(Menu *pMenu, int param)
{
    MenuScroller *p;
    int k;

    pMenu->itemCount = 8;
    p = FUN_004f2520();
    p->startTime = CFrontend::FUN_004d20e0();
    p->pMenu = pMenu;
    p->count = pMenu->itemCount;
    for (k = 0; k < pMenu->itemCount; k++) {
        strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(pMenu->items[k].id));
        CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
        p->widths[k] = Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest);
        if (k < 8)
            pMenu->items[k].enabled = 1;
        else
            pMenu->items[k].enabled = 0;
    }
    p->offset = 0;
    p->startOffset = 0;
    p->current = pMenu->cursor;
    p->previous = pMenu->cursor;
}

// Draw callback of the two-choice pages after the difficulty: the same
// title as the difficulty pages, the list and the scroller.
// FUNCTION: CMR2 0x004d6460
void FUN_004d6460(Menu *pMenu)
{
    char *text[2];

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
    FrontendDraw_PlayTime();
    FrontendDraw_MenuPath(pMenu, (int)(g_pGraphics->resX * 24) / 640, (int)(g_pGraphics->resY * 38) / 480, 1, 3, text, 2);
    FrontendDraw_MenuList(pMenu, NULL, -1, -1, 0, 1);
    FrontendDraw_ScrollerRow(FUN_004f2520(), 0);
    FrontendDraw_HelpText(CFrontend::GetTextString(0x57), 1);
}

// Update callback of the two-choice pages: the first choice is always
// available.
// FUNCTION: CMR2 0x004ef410
void FUN_004ef410(Menu *pMenu)
{
    pMenu->items[0].enabled = 1;
}

// Item callback of the two-choice pages: stores the choice and goes to the
// next screen of the mode.
// FUNCTION: CMR2 0x004efb20
void FUN_004efb20(Menu *pMenu, int param)
{
    FUN_004ea950(pMenu->cursor);
    g_unk0x00819870 = (int)pMenu;
    Menu_SetParent(FUN_004f83a0(), pMenu);
    g_unk0x00819048 = CGameInfo::FUN_00405d70();
}

BYTE FUN_004085a0(BYTE param1);

// Menu the current game mode continues to after the player setup.
#define FRONTEND_MODE_NEXT_MENU(pNext)          \
    switch (CGameInfo::FUN_00405d80()) {        \
    case 4:                                     \
        pNext = FUN_004f8430();                 \
        break;                                  \
    case 5:                                     \
        pNext = FUN_004fa300();                 \
        break;                                  \
    case 6:                                     \
        pNext = FUN_004fa310();                 \
        break;                                  \
    case 7:                                     \
        pNext = FUN_004fa320();                 \
        break;                                  \
    default:                                    \
        pNext = FUN_004f83f0();                 \
        break;                                  \
    }

// Draw callback of the player profile menu: "<mode> | Player N | <menu>".
// FUNCTION: CMR2 0x004d9880
void FUN_004d9880(Menu *pMenu)
{
    char *text[4];

    sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0xdc), g_unk0x008182b4);
    text[2] = CFrontend::m_stringDest;
    text[3] = CFrontend::GetTextString(pMenu->field_0x4);
    if (CGameInfo::FUN_00405e00() != 0) {
        text[1] = NULL;
        text[2] = NULL;
        text[0] = CFrontend::GetTextString(0x12);
    } else {
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
            text[1] = CFrontend::GetTextString(0xe);
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
    }
    FrontendDraw_PlayTime();
    FrontendDraw_Breadcrumb((int)(g_pGraphics->resX * 24) / 640, (int)(g_pGraphics->resY * 38) / 480, text, 4);
    FrontendDraw_MenuList(pMenu, NULL, -1, -1, 0, 1);
    FrontendDraw_HelpText(CFrontend::GetTextString(0x57), 1);
    if (CGameInfo::FUN_00405e00() != 0)
        FrontendDraw_Carousel(FUN_004f8410(), 0, NULL);
}

// Enables one item per selected car/difficulty level (and up to the fourth
// one) of the two player-setup menus, then stores the number of gears and
// the steering aid read from the game info.
// FUNCTION: CMR2 0x004f02e0
void FUN_004f02e0(void)
{
    unsigned int *pInfo = CGameInfo::FUN_00405db0();
    int values[2];
    unsigned int count;
    int i;

    if (CGameInfo::FUN_00406410(0xd)) {
        count = 8;
    } else if (CGameInfo::FUN_00405d80() == 3 || CGameInfo::FUN_00405d80() == 2) {
        count = *pInfo >> 8 & 0xf;
        if ((*pInfo >> 0xc & 0xf) > count)
            count = *pInfo >> 0xc & 0xf;
        if ((*pInfo >> 0x10 & 0xf) > count)
            count = *pInfo >> 0x10 & 0xf;
    } else {
        switch (CGameInfo::FUN_00405d90()) {
        case 0:
            count = *pInfo >> 8 & 0xf;
            break;
        case 1:
            count = *pInfo >> 0xc & 0xf;
            break;
        case 2:
            count = *pInfo >> 0x10 & 0xf;
            break;
        }
    }
    for (i = 0; i < 8; i++) {
        if (i < count || (CGameInfo::FUN_00405d80() != 1 && i < 4))
            FUN_004f8340()->items[i].enabled = 1;
        else
            FUN_004f8340()->items[i].enabled = 0;
    }
    FUN_004ea990(&values[0], &values[1]);
    FUN_004f8340()->cursor = (char)values[0];
    FUN_004f8350()->cursor = (char)values[1];
}

// Entering the player profile menu (back: undoes the previous player's
// profile). Lists the free saved profiles.
// match 45%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004f03f0
void FUN_004f03f0(Menu *pMenu, char back)
{
    MenuItem *pItem;
    char *pText;
    int i;

    i = 0;
    if (back != 0) {
        if (FUN_004085a0(CGameInfo::FUN_00405d70() + (-1 - g_unk0x00819048)) || g_unk0x00819879 != 0) {
            if (FUN_004085a0(CGameInfo::FUN_00405d70() + (-1 - g_unk0x00819048))) {
                g_unk0x00819744--;
                FUN_004eb000(CGameInfo::FUN_00405d70() + (-1 - g_unk0x00819048), 0);
            }
            FUN_004ebf20((CGameInfo::FUN_00405d70() & 0xff) - (g_unk0x00819048 & 0xff) - 1);
        }
        FUN_004ebe80((CGameInfo::FUN_00405d70() & 0xff) - (g_unk0x00819048 & 0xff) - 1);
        g_unk0x00819048++;
    }
    g_unk0x00819879 = 0;
    FUN_004e7770((CGameInfo::FUN_00405d70() & 0xff) - FUN_004f1ba0() + 1);
    FUN_004ea480((CGameInfo::FUN_00405d70() & 0xff) - FUN_004f1ba0());
    CSound::FUN_004a28c0();
    pText = g_profileEntryTexts[0];
    pItem = &pMenu->items[3];
    do {
        if (i < FUN_004ec020()) {
            sprintf(pText, CFrontend::GetTextString(0x17e), FUN_004ec110(i));
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
        pMenu->cursor = 3;
        return;
    }
    pMenu->cursor = 2;
}

// Item callback of "new profile" in the player profile menu.
// match 83%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004f0960
void FUN_004f0960(Menu *pMenu, int param)
{
    Menu *pNext;
    Menu *pAfter;

    g_unk0x00819879 = 1;
    FRONTEND_MODE_NEXT_MENU(pNext)
    FUN_004f2bf0((CGameInfo::FUN_00405d70() & 0xff) - (g_unk0x00819048 & 0xff));
    FUN_004eb860((CGameInfo::FUN_00405d70() & 0xff) - (g_unk0x00819048 & 0xff), -1);
    FUN_004ebf20((CGameInfo::FUN_00405d70() & 0xff) - (g_unk0x00819048 & 0xff));
    FUN_004eb000(CGameInfo::FUN_00405d70() - g_unk0x00819048, 0);
    Menu_SetParent(FUN_004f83c0(), pMenu);
    FUN_004f2c10((int)FUN_004f83d0());
    pAfter = pNext;
    if (CGameInfo::FUN_00405e00() != 0)
        pAfter = FUN_004f8440();
    FUN_004f2c30((int)pAfter);
    sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0xdc),
            (CGameInfo::FUN_00405d70() & 0xff) - (g_unk0x00819048 & 0xff) + 1);
    FUN_004e7780(CFrontend::m_stringDest);
    g_unk0x00819048--;
    Menu_SetParent(pNext, pMenu);
    Menu_SetNextAction((int)FUN_004f83c0());
}

// Item callback of a saved profile in the player profile menu.
// FUNCTION: CMR2 0x004f0c50
void FUN_004f0c50(Menu *pMenu, int param)
{
    Menu *pNext;
    int profile;

    profile = FUN_004ec090(pMenu->cursor - 3);
    FRONTEND_MODE_NEXT_MENU(pNext)
    FUN_004eb860((CGameInfo::FUN_00405d70() & 0xff) - (g_unk0x00819048 & 0xff), profile);
    FUN_004eb000(CGameInfo::FUN_00405d70() - g_unk0x00819048, 0);
    g_unk0x00819048--;
    if (CGameInfo::FUN_00405e00() != 0) {
        Menu_SetParent(FUN_004f8440(), pMenu);
        Menu_SetNextAction((int)FUN_004f8440());
        return;
    }
    Menu_SetParent(pNext, pMenu);
    Menu_SetNextAction((int)pNext);
}

// Callback that steps the player list back one player while setting up a
// championship.
// FUNCTION: CMR2 0x004f0da0
void FUN_004f0da0(int param_1, char param_2)
{
    FUN_004ea480(0);
    FUN_004a0c50(0);
    if (param_2 != 0) {
        FUN_004e77b0(0);
        if (CGameInfo::FUN_00405d80() == 4) {
            if (FUN_004d27d0() == 0) {
                g_unk0x00819744--;
                if (g_unk0x00819744 > 0) {
                    g_unk0x00819048++;
                    FUN_004f2bf0((CGameInfo::FUN_00405d70() & 0xff) - (g_unk0x00819048 & 0xff) - 1);
                    sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0xdc),
                            (CGameInfo::FUN_00405d70() & 0xff) - (g_unk0x00819048 & 0xff));
                    FUN_004e7780(CFrontend::m_stringDest);
                    return;
                }
                Menu_SetNextAction((int)FUN_004f8320());
            }
        }
    }
}

// GLOBAL: CMR2 0x00524d38
char g_strLabelText[8] = "%s: %s";
// GLOBAL: CMR2 0x00524d40
char g_strLabelNumber[8] = "%s: %d";
// GLOBAL: CMR2 0x00524d64
char g_strLabelSpacedText[8] = "%s : %s";

unsigned int RallyData_FUN_004069b0(void);
unsigned int RallyData_FUN_004070e0(void);
void RallyData_FUN_0040d660(BYTE param1);
void RallyData_FUN_0040d680(BYTE param1);
void RallyData_FUN_0040d6a0(BYTE param1);
void RallyData_InitKnockoutBracket(void);
void RallyData_FUN_004070c0(void);
void FUN_004ea970(BYTE param1);
int FUN_004eaac0(unsigned int value);

// Number of stages the player count allows: 1-2 players 1, 3-4 players 2,
// more 3.
#define FRONTEND_PLAYER_GROUP(call)         \
    switch (CGameInfo::FUN_00405d70()) {    \
    case 1:                                 \
    case 2:                                 \
        call(1);                            \
        break;                              \
    case 3:                                 \
    case 4:                                 \
        call(2);                            \
        break;                              \
    default:                                \
        call(3);                            \
        break;                              \
    }

// Entering the multiplayer race settings page: loads the current settings
// (and the ranges the player count allows) into its entries.
// FUNCTION: CMR2 0x004ef970
void FUN_004ef970(Menu *pMenu, int param)
{
    switch (CGameInfo::FUN_00405d70()) {
    case 1:
    case 2:
        g_unk0x00819050 = 0;
        break;
    case 3:
    case 4:
        g_unk0x00819050 = 1;
        break;
    default:
        g_unk0x00819050 = 2;
        break;
    }
    if (CGameInfo::FUN_00405d70() == 1) {
        if (CGameInfo::FUN_00405dd0() == 0) {
            FUN_004ea970(CGameInfo::FUN_00405d90() + 1);
            pMenu->items[0].max = CGameInfo::FUN_00405d90();
        } else {
            pMenu->items[0].max = CGameInfo::FUN_00405dd0() - 1;
        }
        pMenu->items[0].min = 3;
        g_unk0x00819748 = 1;
    } else {
        pMenu->items[0].max = CGameInfo::FUN_00405dd0();
        pMenu->items[0].min = 4;
        g_unk0x00819748 = 0;
    }
    if (CGameInfo::FUN_00405dd0() == 0) {
        FRONTEND_PLAYER_GROUP(RallyData_FUN_0040d660)
        pMenu->items[1].enabled = 0;
        pMenu->items[1].max = RallyData_FUN_004069b0() - 1;
    } else {
        pMenu->items[1].enabled = 1;
    }
    pMenu->items[1].min = 4 - g_unk0x00819050;
    pMenu->items[1].max = RallyData_FUN_004069b0() - 1;
    if (pMenu->items[1].max >= pMenu->items[1].min)
        pMenu->items[1].max = pMenu->items[1].min - 1;
    switch (RallyData_FUN_004070e0()) {
    case 1:
        pMenu->items[2].max = 1;
        break;
    case 3:
        pMenu->items[2].max = 2;
        break;
    case 5:
        pMenu->items[2].max = 3;
        break;
    case 7:
        pMenu->items[2].max = 4;
        break;
    default:
        pMenu->items[2].max = 0;
        break;
    }
    if (CGameInfo::FUN_00406410(0xd))
        pMenu->items[2].min = 5;
    else
        pMenu->items[2].min = FUN_004eaac0(CGameInfo::FUN_00405db0()[1] & 0x1f) + 1;
    if (pMenu->items[2].max >= pMenu->items[2].min)
        pMenu->items[2].max = pMenu->items[2].min - 1;
}

// Item callback of "start" on the multiplayer race settings page: stores
// the settings and starts the knockout.
// FUNCTION: CMR2 0x004f01c0
void FUN_004f01c0(Menu *pMenu, int param)
{
    if (g_unk0x00819748 != 0)
        FUN_004ea970(pMenu->items[0].max + 1);
    else
        FUN_004ea970(pMenu->items[0].max);
    RallyData_FUN_0040d660(pMenu->items[1].max + g_unk0x00819050 + 1);
    RallyData_FUN_0040d680(1);
    switch (pMenu->items[2].max) {
    case 1:
        RallyData_FUN_0040d6a0(1);
        break;
    case 2:
        RallyData_FUN_0040d6a0(3);
        break;
    case 3:
        RallyData_FUN_0040d6a0(5);
        break;
    case 4:
        RallyData_FUN_0040d6a0(7);
        break;
    case 0:
        RallyData_FUN_0040d6a0(8);
        break;
    default:
        RallyData_FUN_0040d6a0(8);
        break;
    }
    RallyData_InitKnockoutBracket();
    RallyData_FUN_004070c0();
    Menu_SetNextAction((int)FUN_004f8330());
}

// Update callback of the multiplayer race settings page.
// FUNCTION: CMR2 0x004f0250
void FUN_004f0250(Menu *pMenu)
{
    if (g_unk0x00819748 != 0)
        FUN_004ea970(pMenu->items[0].max + 1);
    else
        FUN_004ea970(pMenu->items[0].max);
    if (CGameInfo::FUN_00405dd0() == 0) {
        FRONTEND_PLAYER_GROUP(RallyData_FUN_0040d660)
        pMenu->items[1].enabled = 0;
        pMenu->items[1].max = RallyData_FUN_004069b0() + (-1 - g_unk0x00819050);
        return;
    }
    pMenu->items[1].enabled = 1;
}

// Draw callback of the rally menu: one row per rally, drawn in the layout of
// its item value (title only, or with the stage names of the rally), over the
// strip of the selected rows, plus the carousel.
// match 75%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004df410
void FUN_004df410(Menu *pMenu)
{
    SpriteRect dst;
    BYTE *pColour;
    BYTE *pShadow;
    Texture *pTexture;
    int top;
    int x;
    int i;
    int j;

    dst.y = 0;
    dst.x = (int)(g_pGraphics->resX * 100) / 640;
    dst.w = CFrontend::m_pAr640ATexture->width;
    dst.h = CFrontend::m_pAr640ATexture->height;
    FrontendDraw_PlayTime();
    FrontendDraw_MenuPath(pMenu, PATH_X(), PATH_Y(), 1, 2, NULL, -1);
    top = ((int)(g_pGraphics->resY * 8) / 480 + (int)(g_pGraphics->resY * 38) / 480 +
           (int)(g_pGraphics->resY * 384) / 480) / 2 -
          (int)((int)(g_pGraphics->resY * 36) / 480 * pMenu->itemCount) / 2;
    if (pMenu->cursor == 0) {
        pColour = g_colourWhite0x00524968;
        pShadow = g_colourShadowWhite0x00524974;
    } else {
        pColour = g_colourText0x0052496c;
        pShadow = g_colourShadowText0x00524978;
    }
    g_unk0x008189a8[1] = top;
    g_unk0x008189a8[0] = (int)(g_pGraphics->resX * 99) / 640;
    g_unk0x008189a8[3] = 1;
    g_unk0x008189a8[2] = (int)(g_pGraphics->resX * 282) / 640;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, pShadow, 1);
    g_unk0x008189a8[1]++;
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, pColour, 1);
    for (i = 0; i < pMenu->itemCount; i++) {
        dst.y = (int)(g_pGraphics->resY * 20) / 480 +
                (top + ((int)(g_pGraphics->resY * 36) / 480 * i - CFrontend::m_pAr640ATexture->height / 2));
        if (pMenu->cursor == i) {
            pColour = g_colourWhite0x00524968;
            pTexture = CFrontend::m_pAr640ATexture;
        } else {
            pColour = g_colourText0x0052496c;
            pTexture = CFrontend::m_pAr640DTexture;
        }
        Sprite_Queue((SpriteRect *)&pTexture->field_0x11c, &dst, pTexture, 1, 0, NULL, NULL, pColour, 8);
        switch (pMenu->items[i].value) {
        case 3:
            strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(pMenu->items[i].id));
            x = (int)(g_pGraphics->resX * 122) / 640;
            Font_DrawText(1, CFrontend::m_stringDest, x,
                          (int)(g_pGraphics->resY * 24) / 480 + g_unk0x008189a8[1], (int *)pColour, 0x11);
            for (j = 0; j < pMenu->items[i].min; j++) {
                pColour = g_colourWhite0x00524968;
                if (pMenu->items[i].max != j)
                    pColour = g_colourText0x0052496c;
                x = (int)(g_pGraphics->resX * 10) / 640 + x +
                    Font_GetTextWidth(1, (BYTE *)CFrontend::m_stringDest);
                Font_DrawText(1, CFrontend::GetTextString(j + 0x135), x,
                              (int)(g_pGraphics->resY * 24) / 480 + g_unk0x008189a8[1], (int *)pColour, 0x11);
                strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(j + 0x135));
            }
            break;
        case 4:
            strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(pMenu->items[i].id));
            x = (int)(g_pGraphics->resX * 122) / 640;
            Font_DrawText(1, CFrontend::m_stringDest, x,
                          (int)(g_pGraphics->resY * 24) / 480 + g_unk0x008189a8[1], (int *)pColour, 0x11);
            for (j = 0; j < pMenu->items[i].min; j++) {
                pColour = g_colourWhite0x00524968;
                if (pMenu->items[i].max != j)
                    pColour = g_colourText0x0052496c;
                x = (int)(g_pGraphics->resX * 10) / 640 + x +
                    Font_GetTextWidth(1, (BYTE *)CFrontend::m_stringDest);
                Font_DrawText(1, CFrontend::GetTextString(j + 0x152), x,
                              (int)(g_pGraphics->resY * 24) / 480 + g_unk0x008189a8[1], (int *)pColour, 0x11);
                strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(j + 0x152));
            }
            break;
        case 2:
            strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(pMenu->items[i].id));
            x = (int)(g_pGraphics->resX * 122) / 640;
            Font_DrawText(1, CFrontend::m_stringDest, x,
                          (int)(g_pGraphics->resY * 24) / 480 + g_unk0x008189a8[1], (int *)pColour, 0x11);
            for (j = 0; j < pMenu->items[i].min; j++) {
                pColour = g_colourWhite0x00524968;
                if (pMenu->items[i].max != j)
                    pColour = g_colourText0x0052496c;
                x = (int)(g_pGraphics->resX * 10) / 640 + x +
                    Font_GetTextWidth(1, (BYTE *)CFrontend::m_stringDest);
                Font_DrawText(1, CFrontend::GetTextString(0x134 - j), x,
                              (int)(g_pGraphics->resY * 24) / 480 + g_unk0x008189a8[1], (int *)pColour, 0x11);
                strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(0x134 - j));
            }
            break;
        case 0:
            strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(pMenu->items[i].id));
            x = (int)(g_pGraphics->resX * 122) / 640;
            Font_DrawText(1, CFrontend::m_stringDest, x,
                          (int)(g_pGraphics->resY * 24) / 480 + g_unk0x008189a8[1], (int *)pColour, 0x11);
            for (j = 0; j < pMenu->items[i].min; j++) {
                pColour = g_colourWhite0x00524968;
                if (pMenu->items[i].max != j)
                    pColour = g_colourText0x0052496c;
                x = (int)(g_pGraphics->resX * 10) / 640 + x +
                    Font_GetTextWidth(1, (BYTE *)CFrontend::m_stringDest);
                Font_DrawText(1, CFrontend::GetTextString(j + 0x137), x,
                              (int)(g_pGraphics->resY * 24) / 480 + g_unk0x008189a8[1], (int *)pColour, 0x11);
                strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(j + 0x137));
            }
            break;
        default:
            Font_DrawText(1, CFrontend::GetTextString(pMenu->items[i].id),
                          (int)(g_pGraphics->resX * 122) / 640,
                          (int)(g_pGraphics->resY * 24) / 480 + g_unk0x008189a8[1], (int *)pColour, 0x11);
            break;
        }
        if (pMenu->cursor == i || pMenu->cursor == i + 1) {
            pColour = g_colourWhite0x00524968;
            pShadow = g_colourShadowWhite0x00524974;
        } else {
            pColour = g_colourText0x0052496c;
            pShadow = g_colourShadowText0x00524978;
        }
        g_unk0x008189a8[1] = (int)(g_pGraphics->resY * 36) / 480 * (i + 1) + top;
        Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, pShadow, 1);
        g_unk0x008189a8[1]++;
        Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, pColour, 1);
    }
    FrontendDraw_Carousel(FUN_004f8410(), 0, NULL);
}

// Draw callback of the multiplayer race settings page.
// match 83%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004e1230
void FUN_004e1230(Menu *pMenu)
{
    short icon[4];
    char *text[3];
    BYTE *pShadow;
    BYTE *pColour;
    Texture *pTexture;
    short y0;
    int id;
    int i;

    icon[1] = 0;
    icon[0] = (int)(g_pGraphics->resX * 100) / 640;
    icon[2] = CFrontend::m_pAr640ATexture->width;
    icon[3] = CFrontend::m_pAr640ATexture->height;
    g_unk0x008189a8[0] = (int)(g_pGraphics->resX * 440) / 640;
    g_unk0x008189a8[3] = (int)(g_pGraphics->resY * 100) / 480;
    g_unk0x008189a8[1] = (short)(((int)(g_pGraphics->resY * 56) / 480 + (int)(g_pGraphics->resY * 374) / 480) / 2)
                         - g_unk0x008189a8[3] / 2;
    g_unk0x008189a8[2] = (int)(g_pGraphics->resX * 160) / 640;
    FrontendDraw_PlayTime();
    text[0] = CFrontend::GetTextString(0xe7);
    text[1] = CFrontend::GetTextString(0xe);
    text[2] = CFrontend::GetTextString(0x65);
    FrontendDraw_Breadcrumb((int)(g_pGraphics->resX * 24) / 640, (int)(g_pGraphics->resY * 38) / 480, text, 3);
    y0 = (short)(((int)(g_pGraphics->resY * 56) / 480 + (int)(g_pGraphics->resY * 374) / 480) / 2)
         - (short)((int)(g_pGraphics->resY * 36) / 480 * pMenu->itemCount / 2);
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
        icon[1] = (int)(g_pGraphics->resY * 20) / 480 + y0 + (int)(g_pGraphics->resY * 36) / 480 * (short)i
                  - CFrontend::m_pAr640ATexture->height / 2;
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
        switch (i) {
        case 0:
            if (FUN_004f3a30() != 0) {
                switch (pMenu->items[0].max) {
                case 0:
                    id = 0x43;
                    break;
                case 1:
                    id = 0x44;
                    break;
                case 2:
                    id = 0x45;
                    break;
                default:
                    goto draw;
                }
            } else {
                switch (pMenu->items[0].max) {
                case 0:
                    id = 0x133;
                    break;
                case 1:
                    id = 0x43;
                    break;
                case 2:
                    id = 0x44;
                    break;
                case 3:
                    id = 0x45;
                    break;
                default:
                    goto draw;
                }
            }
            sprintf(CFrontend::m_stringDest, g_strLabelText, CFrontend::GetTextString(0x142), CFrontend::GetTextString(id));
            break;
        case 1:
            sprintf(CFrontend::m_stringDest, g_strLabelNumber, CFrontend::GetTextString(0x143),
                    FUN_004f3a40() + pMenu->items[1].max + 1);
            break;
        case 2:
            sprintf(CFrontend::m_stringDest, g_strLabelSpacedText, CFrontend::GetTextString(0x31),
                    CFrontend::GetTextString(pMenu->items[2].max == 0 ? 0x2f : pMenu->items[2].max * 2 + 0x26));
            break;
        case 3:
            strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(0x67));
            break;
        case 4:
            strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(0x1b));
            break;
        }
    draw:
        Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x7a) / 640,
                      (short)((int)(g_pGraphics->resY * 24) / 480 + g_unk0x008189a8[1]), (int *)pColour, 0x11);
        if (pMenu->cursor == i || pMenu->cursor == i + 1) {
            pColour = g_colourWhite0x00524968;
            pShadow = g_colourShadowWhite0x00524974;
        } else {
            pColour = g_colourText0x0052496c;
            pShadow = g_colourShadowText0x00524978;
        }
        g_unk0x008189a8[1] = (int)(g_pGraphics->resY * 36) / 480 * ((short)i + 1) + y0;
        Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, pShadow, 1);
        g_unk0x008189a8[1]++;
        Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, pColour, 1);
    }
    FrontendDraw_HelpText(CFrontend::GetTextString(0x57), 1);
}

int FUN_004f92f0(void);

// Draw callback of the network message page: title and the message of the
// current network state.
// FUNCTION: CMR2 0x004e96d0
void FUN_004e96d0(Menu *pMenu)
{
    char *text[2];
    int id;

    text[0] = CFrontend::GetTextString(0);
    text[1] = CFrontend::GetTextString(0x1e2);
    FrontendDraw_PlayTime();
    FrontendDraw_Breadcrumb((int)(g_pGraphics->resX * 24) / 640, (int)(g_pGraphics->resY * 38) / 480, text, 2);
    switch (FUN_004f92f0()) {
    case 0:
        Font_DrawText(1, CFrontend::GetTextString(0x1e3), (int)g_pGraphics->resX / 2, (int)g_pGraphics->resY / 2,
                      (int *)g_colourWhite0x00524968, 0x12);
        break;
    case 1:
        Font_DrawText(1, CFrontend::GetTextString(0x1e4), (int)g_pGraphics->resX / 2, (int)g_pGraphics->resY / 2,
                      (int *)g_colourWhite0x00524968, 0x12);
        break;
    case 2:
        Font_DrawText(1, CFrontend::GetTextString(0x1e5), (int)g_pGraphics->resX / 2, (int)g_pGraphics->resY / 2,
                      (int *)g_colourWhite0x00524968, 0x12);
        break;
    case 3:
        Font_DrawText(1, CFrontend::GetTextString(0x1e6), (int)g_pGraphics->resX / 2, (int)g_pGraphics->resY / 2,
                      (int *)g_colourWhite0x00524968, 0x12);
        break;
    default:
        goto done;
    }
done:
    FrontendDraw_Carousel(FUN_004f8410(), 0, NULL);
}

// Draw callback of the arcade menu.
// FUNCTION: CMR2 0x004e2500
void FUN_004e2500(Menu *pMenu)
{
    char *text[1];

    text[0] = CFrontend::GetTextString(0x94);
    FrontendDraw_PlayTime();
    FrontendDraw_MenuPath(pMenu, (int)(g_pGraphics->resX * 24) / 640, (int)(g_pGraphics->resY * 38) / 480, 1, 2, text, 1);
    FrontendDraw_MenuList(pMenu, NULL, -1, -1, 0, 1);
    FrontendDraw_Carousel(FUN_004f8410(), 0, NULL);
}

void FUN_004f1bb0(int value);

// Leaving the arcade menu: sets the arcade mode of the entry chosen.
// FUNCTION: CMR2 0x004ef740
void FUN_004ef740(Menu *pMenu, char back)
{
    if (back != 0)
        return;
    FUN_004ea950(0);
    switch (pMenu->cursor) {
    case 0:
        FUN_004ea8e0(5);
        break;
    case 1:
        FUN_004ea8e0(6);
        break;
    case 2:
        FUN_004ea8e0(7);
        FUN_004ea8c0(1);
        FUN_004f1bb0(CGameInfo::FUN_00405d70());
        FUN_004ea950(0);
        RallyData_FUN_004068b0(0);
        RallyData_FUN_004068e0(0);
        return;
    }
    RallyData_FUN_004068b0(0);
    RallyData_FUN_004068e0(0);
}

void FUN_004ea930(BYTE param1);
void FUN_004fa970(void);

// Item callback of an arcade game type.
// FUNCTION: CMR2 0x004ef930
void FUN_004ef930(Menu *pMenu, int param)
{
    FUN_004ea930(pMenu->cursor);
    if (CGameInfo::FUN_00405d80() == 5)
        FUN_004fa970();
}

// GLOBAL: CMR2 0x00524d48
char g_strLabelColon[4] = "%s:";
// Game type the second quick race page was last showing.
// GLOBAL: CMR2 0x00829320
unsigned int g_unk0x00829320;

unsigned int FUN_004eaa80(void);
unsigned int FUN_004eaab0(void);
unsigned int FUN_004eacf0(void);
unsigned int FUN_004ead00(void);
void FUN_004eaa60(unsigned int param1);
void FUN_004eaa90(unsigned int param1);
void FUN_004eac80(BYTE param1);
void FUN_004eacb0(unsigned int param1);
void FUN_004eacd0(unsigned int param1);
unsigned int RallyData_FUN_004082b0(void);
void RallyData_FUN_0040d620(BYTE param1);
void RallyData_FUN_0040e360(unsigned int param1);
void RallyData_FUN_0040e380(unsigned int param1);
void RallyData_FUN_0040e3a0(unsigned int param1);
MenuScroller *FUN_004f2590(void);

// Draws the on/off choice of a settings row after the text in m_stringDest.
#define QUICKRACE_DRAW_CHOICE(value, y)                                                                     \
    x0 = (int)(g_pGraphics->resX * 0x7a) / 640;                                                             \
    Font_DrawText(1, CFrontend::GetTextString(0x133),                                                        \
                  (int)(g_pGraphics->resX * 10) / 640 + x0 + Font_GetTextWidth(1, (BYTE *)CFrontend::m_stringDest), y, \
                  (int *)((value) == 0 ? g_colourWhite0x00524968 : g_colourText0x0052496c), 0x11);         \
    Font_DrawText(1, CFrontend::GetTextString(0x134),                                                        \
                  (int)(g_pGraphics->resX * 20) / 640 + x0 + Font_GetTextWidth(1, (BYTE *)CFrontend::GetTextString(0x133)) \
                      + Font_GetTextWidth(1, (BYTE *)CFrontend::m_stringDest), y,                            \
                  (int *)((value) == 0 ? g_colourText0x0052496c : g_colourWhite0x00524968), 0x11);

// Draws the frame of a quick race page: path, help, header line; returns the
// y of the first row.
#define QUICKRACE_FRAME()                                                                                   \
    icon[1] = 0;                                                                                            \
    icon[0] = (int)(g_pGraphics->resX * 100) / 640;                                                         \
    icon[2] = CFrontend::m_pAr640ATexture->width;                                                           \
    icon[3] = CFrontend::m_pAr640ATexture->height;                                                          \
    text[0] = CFrontend::GetTextString(0x94);                                                               \
    text[1] = CFrontend::GetTextString(0xe2);                                                               \
    FrontendDraw_PlayTime();                                                                                \
    FrontendDraw_MenuPath(pMenu, (int)(g_pGraphics->resX * 24) / 640, (int)(g_pGraphics->resY * 38) / 480, 1, 3, text, 2); \
    FrontendDraw_HelpText(CFrontend::GetTextString(0x57), 1);                                               \
    y0 = (short)(((int)(g_pGraphics->resY * 56) / 480 + (int)(g_pGraphics->resY * 374) / 480) / 2)           \
         - (short)((int)(g_pGraphics->resY * 36) / 480 * pMenu->itemCount / 2);                              \
    if (pMenu->cursor == 0) {                                                                               \
        pColour = g_colourWhite0x00524968;                                                                  \
        pShadow = g_colourShadowWhite0x00524974;                                                            \
    } else {                                                                                                \
        pColour = g_colourText0x0052496c;                                                                   \
        pShadow = g_colourShadowText0x00524978;                                                             \
    }                                                                                                       \
    g_unk0x008189a8[0] = (int)(g_pGraphics->resX * 99) / 640;                                               \
    g_unk0x008189a8[3] = 1;                                                                                 \
    g_unk0x008189a8[2] = (int)(g_pGraphics->resX * 282) / 640;                                              \
    g_unk0x008189a8[1] = y0;                                                                                \
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, pShadow, 1);                                 \
    g_unk0x008189a8[1]++;                                                                                   \
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, pColour, 1);

// Icon and colour of row i of a quick race page.
#define QUICKRACE_ROW_ICON(i)                                                                               \
    icon[1] = (int)(g_pGraphics->resY * 20) / 480 + y0 + (int)(g_pGraphics->resY * 36) / 480 * (short)(i)   \
              - CFrontend::m_pAr640ATexture->height / 2;                                                    \
    if (pMenu->cursor == (i)) {                                                                             \
        pColour = g_colourWhite0x00524968;                                                                  \
        pTexture = CFrontend::m_pAr640ATexture;                                                             \
    } else {                                                                                                \
        pColour = g_colourText0x0052496c;                                                                   \
        pTexture = CFrontend::m_pAr640DTexture;                                                             \
        if (!pMenu->items[i].enabled)                                                                       \
            pColour = g_colourDim0x00524970;                                                                \
    }                                                                                                       \
    Sprite_Queue((SpriteRect *)&pTexture->field_0x11c, (SpriteRect *)icon, pTexture, 1, 0, NULL, NULL, pColour, 8);

// Separator under row i of a quick race page.
#define QUICKRACE_ROW_LINE(i)                                                                               \
    if (pMenu->cursor == (i) || pMenu->cursor == (i) + 1) {                                                 \
        pColour = g_colourWhite0x00524968;                                                                  \
        pShadow = g_colourShadowWhite0x00524974;                                                            \
    } else {                                                                                                \
        pColour = g_colourText0x0052496c;                                                                   \
        pShadow = g_colourShadowText0x00524978;                                                             \
    }                                                                                                       \
    g_unk0x008189a8[1] = (int)(g_pGraphics->resY * 36) / 480 * ((short)(i) + 1) + y0;                       \
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, pShadow, 1);                                 \
    g_unk0x008189a8[1]++;                                                                                   \
    Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, pColour, 1);

// Draw callback of the first quick race page (stages, cars, ...).
// match 50%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004da710
void FUN_004da710(Menu *pMenu)
{
    short icon[4];
    char *text[2];
    BYTE *pShadow;
    BYTE *pColour;
    Texture *pTexture;
    MenuItem *pItem;
    short y0;
    int x0;
    int y;
    int i;

    QUICKRACE_FRAME()
    for (i = 0; i < pMenu->itemCount; i++) {
        pItem = &pMenu->items[i];
        QUICKRACE_ROW_ICON(i)
        y = (short)((int)(g_pGraphics->resY * 24) / 480 + g_unk0x008189a8[1]);
        switch (pItem->value) {
        case 0:
            sprintf(CFrontend::m_stringDest, g_strLabelNumber, CFrontend::GetTextString(0x195), pMenu->items[0].max + 1);
            Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x7a) / 640, y, (int *)pColour, 0x11);
            break;
        case 1:
            sprintf(CFrontend::m_stringDest, g_strLabelNumber, CFrontend::GetTextString(0x196), pMenu->items[1].max + 1);
            Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x7a) / 640, y, (int *)pColour, 0x11);
            break;
        case 2:
            sprintf(CFrontend::m_stringDest, g_strLabelColon, CFrontend::GetTextString(0x19e));
            Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x7a) / 640, y, (int *)pColour, 0x11);
            QUICKRACE_DRAW_CHOICE(pItem->max, y)
            break;
        default:
            sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue, CFrontend::GetTextString(pItem->id),
                    pMenu->items[1].max + 1);
            Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x7a) / 640, y, (int *)pColour, 0x11);
            break;
        }
        QUICKRACE_ROW_LINE(i)
    }
    FrontendDraw_HelpText(CFrontend::GetTextString(0x57), 1);
    FrontendDraw_ScrollerRow(FUN_004f2590(), 0);
}

// Draw callback of the second quick race page (game type and its settings).
// match 44%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004daf90
void FUN_004daf90(Menu *pMenu)
{
    short icon[4];
    char *text[2];
    BYTE *pShadow;
    BYTE *pColour;
    Texture *pTexture;
    MenuItem *pItem;
    short y0;
    int x0;
    int y;
    int i;

    QUICKRACE_FRAME()
    for (i = 0; i < pMenu->itemCount; i++) {
        pItem = &pMenu->items[i];
        QUICKRACE_ROW_ICON(i)
        y = (short)((int)(g_pGraphics->resY * 24) / 480 + g_unk0x008189a8[1]);
        switch (pItem->value) {
        case 2:
            sprintf(CFrontend::m_stringDest, g_strLabelColon, CFrontend::GetTextString(0x19e));
            Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x7a) / 640, y, (int *)pColour, 0x11);
            QUICKRACE_DRAW_CHOICE(pItem->max, y)
            goto line;
        case 3:
            sprintf(CFrontend::m_stringDest, g_strLabelText, CFrontend::GetTextString(0x1a3),
                    CFrontend::GetTextString(pItem->max + 0x1a4));
            break;
        case 4:
            sprintf(CFrontend::m_stringDest, g_strLabelNumber, CFrontend::GetTextString(0x195), pItem->max + 1);
            break;
        case 5:
            sprintf(CFrontend::m_stringDest, g_strLabelNumber, CFrontend::GetTextString(0x1a2), pItem->max + 1);
            break;
        case 6:
            sprintf(CFrontend::m_stringDest, g_strLabelNumber, CFrontend::GetTextString(0x1a1), pItem->max + 1);
            break;
        default:
            sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue, CFrontend::GetTextString(pItem->id),
                    pMenu->items[1].max + 1);
            break;
        }
        Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x7a) / 640, y, (int *)pColour, 0x11);
    line:
        QUICKRACE_ROW_LINE(i)
    }
    FrontendDraw_HelpText(CFrontend::GetTextString(0x57), 1);
    FrontendDraw_ScrollerRow(FUN_004f2590(), 0);
}

// Item callback of "start" on the first quick race page.
// FUNCTION: CMR2 0x004f0090
void FUN_004f0090(Menu *pMenu, int param)
{
    RallyData_FUN_0040d620(pMenu->items[0].max + 1);
    FUN_004eaa90(pMenu->items[0].max + 1);
    if (CGameInfo::FUN_00405d70() == 2) {
        RallyData_FUN_0040d640(0);
        FUN_004eac80(pMenu->items[1].max);
    } else {
        RallyData_FUN_0040d640(pMenu->items[1].max + 1);
        FUN_004eaa60(pMenu->items[1].max + 1);
        FUN_004eac80(0);
    }
    Menu_SetNextAction((int)FUN_004f8330());
}

// Entering the first quick race page: loads its settings.
// FUNCTION: CMR2 0x004f3220
void FUN_004f3220(Menu *pMenu, int param)
{
    pMenu->items[0].max = FUN_004eaab0() - 1;
    if (CGameInfo::FUN_00405d70() == 2) {
        pMenu->items[1].value = 2;
        pMenu->items[1].max = 0;
        pMenu->items[1].min = 2;
        pMenu->items[1].max = CGameInfo::FUN_00406440() != 0;
        pMenu->cursor = 2;
        return;
    }
    pMenu->items[1].value = 1;
    pMenu->items[1].min = 5;
    pMenu->cursor = 2;
    pMenu->items[1].max = FUN_004eaa80() - 1;
}

// Setting row of the game type chosen in entry 0 of the second quick race
// page.
#define QUICKRACE_TYPE_SETTING(pMenu, type)                          \
    switch (type) {                                                  \
    case 0: {                                                        \
        BYTE m = FUN_004eaab0() - 1;                                 \
        pMenu->items[1].min = 10;                                    \
        pMenu->items[1].max = m;                                     \
        pMenu->items[1].value = 4;                                   \
        break; }                                                     \
    case 1: {                                                        \
        BYTE m = FUN_004eacf0() - 1;                                 \
        pMenu->items[1].min = 10;                                    \
        pMenu->items[1].max = m;                                     \
        pMenu->items[1].value = 6;                                   \
        break; }                                                     \
    case 2: {                                                        \
        BYTE m = FUN_004ead00() - 1;                                 \
        pMenu->items[1].min = 10;                                    \
        pMenu->items[1].max = m;                                     \
        pMenu->items[1].value = 5;                                   \
        break; }                                                     \
    }

// Item callback of "start" on the second quick race page.
// FUNCTION: CMR2 0x004f0110
void FUN_004f0110(Menu *pMenu, int param)
{
    RallyData_FUN_0040d640(0);
    RallyData_FUN_0040e360(pMenu->items[0].max);
    FUN_004eac80(pMenu->items[2].max);
    switch (RallyData_FUN_004082b0()) {
    case 0:
        RallyData_FUN_0040d620(pMenu->items[1].max + 1);
        FUN_004eaa90(pMenu->items[1].max + 1);
        break;
    case 1:
        RallyData_FUN_0040e3a0(pMenu->items[1].max + 1);
        FUN_004eacb0(pMenu->items[1].max + 1);
        break;
    case 2:
        RallyData_FUN_0040e380(pMenu->items[1].max + 1);
        FUN_004eacd0(pMenu->items[1].max + 1);
        break;
    }
    Menu_SetNextAction((int)FUN_004f8330());
}

// Entering the second quick race page: loads its settings.
// FUNCTION: CMR2 0x004f3280
void FUN_004f3280(Menu *pMenu, int param)
{
    unsigned int type;

    g_unk0x00829320 = RallyData_FUN_004082b0();
    pMenu->items[0].max = RallyData_FUN_004082b0();
    type = RallyData_FUN_004082b0();
    QUICKRACE_TYPE_SETTING(pMenu, type)
    if ((BYTE)CGameInfo::FUN_00406440() != 0) {
        pMenu->items[2].max = 1;
        pMenu->cursor = 3;
        return;
    }
    pMenu->items[2].max = 0;
    pMenu->cursor = 3;
}

// Update callback of the second quick race page: when the game type changes
// the setting row follows it; the setting is stored as it changes.
// FUNCTION: CMR2 0x004f3310
void FUN_004f3310(Menu *pMenu)
{
    unsigned int type;

    type = pMenu->items[0].max;
    if (g_unk0x00829320 != type) {
        QUICKRACE_TYPE_SETTING(pMenu, type)
        g_unk0x00829320 = pMenu->items[0].max;
    }
    switch (pMenu->items[0].max) {
    case 0:
        RallyData_FUN_0040d620(pMenu->items[1].max + 1);
        FUN_004eaa90(pMenu->items[1].max + 1);
        break;
    case 1:
        RallyData_FUN_0040e3a0(pMenu->items[1].max + 1);
        FUN_004eacb0(pMenu->items[1].max + 1);
        break;
    case 2:
        RallyData_FUN_0040e380(pMenu->items[1].max + 1);
        FUN_004eacd0(pMenu->items[1].max + 1);
        break;
    }
    FUN_004f37c0(FUN_004f2590());
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

// Lays out the scroller of the route list screens: marks the entries the
// current mode makes available and measures their text.
// match 87%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004faa50
void FUN_004faa50(Menu *pMenu, int param)
{
    MenuScroller *pScroller;
    int i;
    int count;

    FUN_004ea480(CGameInfo::FUN_00405d70() - g_unk0x00819048 - 1);
    FUN_004f2500()->pMenu = pMenu;
    pMenu->cursor = 0;
    count = 0;
    for (i = 0; i < 0x16; i++) {
        if (CGameInfo::FUN_00405d80() == 7 || CGameInfo::FUN_00405d80() == 5 || CGameInfo::FUN_00405d80() == 6) {
            if (RallyData_FUN_00408e30(CGameInfo::FUN_00405d70() - g_unk0x00819048 - 1, i, 1)) {
                g_unk0x008196e8[count] = i;
                pMenu->items[count].id = i + 0x98;
                strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(i + 0x98));
                CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
                FUN_004f2500()->widths[count] = Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest);
                if (i == RallyData_FUN_004086b0(CGameInfo::FUN_00405d70() + (0xff - g_unk0x00819048)))
                    pMenu->cursor = count;
                count++;
            }
        }
    }
    pMenu->itemCount = count;
    pScroller = FUN_004f2500();
    pScroller->spacing = PATH_X();
    pScroller->count = pMenu->itemCount;
    pScroller->offset = 0;
    pScroller->startOffset = 0;
    for (i = 0; i < pMenu->itemCount; i++) {
        strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(pMenu->items[i].id));
        CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
        pScroller->widths[i] = Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest);
    }
}

// Change callback of the championship entry screens: stores the new entry
// value, resets the mode's selection and re-lays the entry list on request.
// match 85%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004fac70
void FUN_004fac70(Menu *pMenu, char param)
{
    pMenu->items[0].max = FUN_004086f0(CGameInfo::FUN_00405d70() + (0xff - g_unk0x00819048));
    FUN_004ea480(CGameInfo::FUN_00405d70() - g_unk0x00819048 - 1);
    if (param != 0)
        FUN_004faa50(FUN_004f2500()->pMenu, 0);
}

// Item callback of the entry value screens: stores the value selected for the
// entry and moves to the next or the parent menu.
// FUNCTION: CMR2 0x004facd0
void FUN_004facd0(Menu *pMenu, int param)
{
    RallyData_FUN_00408600(
        CGameInfo::FUN_00405d70() + (0xff - g_unk0x00819048),
        (unsigned char)g_unk0x008196e8[pMenu->cursor]);
    FUN_0040dfa0();
    if (g_unk0x00819048 == 0) {
        Menu_SetNextAction((int)FUN_004fa330());
        return;
    }
    Menu_SetParent(FUN_004f83a0(), pMenu);
    Menu_SetNextAction((int)FUN_004f83a0());
}

// Item callback of the championship value screens: toggles the entry's
// championship flag and moves to the next or the parent menu.
// FUNCTION: CMR2 0x004fad40
void FUN_004fad40(Menu *pMenu, int param)
{
    FUN_004eb0c0(CGameInfo::FUN_00405d70() + (0xff - g_unk0x00819048), pMenu->items[0].max);
    if (g_unk0x00819048 == 0) {
        Menu_SetNextAction((int)FUN_004fa330());
        return;
    }
    Menu_SetNextAction((int)FUN_004f83a0());
}

// Change callback of the entry value screens: stores the value selected for
// the highlighted entry and refills the event list from it.
// FUNCTION: CMR2 0x004fad90
void FUN_004fad90(Menu *pMenu, int param)
{
    FUN_004ea480(0);
    RallyData_FUN_00408600(
        CGameInfo::FUN_00405d70() + (0xff - g_unk0x00819048),
        (unsigned char)g_unk0x008196e8[pMenu->cursor]);
    FUN_0040dfa0();
}

// Item callback of the shared entry screens: applies the value of the
// highlighted entry and rewires the parent of the screen the mode goes to.
// FUNCTION: CMR2 0x004fadd0
void FUN_004fadd0(Menu *pMenu, int param)
{
    FUN_004eb0c0(CGameInfo::FUN_00405d70() + (0xff - g_unk0x00819048), pMenu->items[0].max);
    if (g_unk0x00819048 == 0) {
        if (CGameInfo::FUN_00405d70() == 2) {
            FUN_004f83a0()->pParent = pMenu;
            FUN_004fa340()->pParent = pMenu;
            Menu_SetNextAction((int)FUN_004fa340());
        } else {
            FUN_004fa340()->pParent = FUN_004fa2f0();
            FUN_004fa2f0()->pParent = pMenu;
            Menu_SetNextAction((int)FUN_004fa2f0());
        }
    } else {
        if (CGameInfo::FUN_00405d70() == 2)
            FUN_004f83a0()->pParent = pMenu;
        Menu_SetNextAction((int)FUN_004f83a0());
    }
}

// Change callback of the second entry value screens: like FUN_004fad90 but
// without resetting the mode's selection first.
// FUNCTION: CMR2 0x004fae70
void FUN_004fae70(Menu *pMenu, int param)
{
    RallyData_FUN_00408600(
        CGameInfo::FUN_00405d70() + (0xff - g_unk0x00819048),
        (unsigned char)g_unk0x008196e8[pMenu->cursor]);
    FUN_0040dfa0();
}

// Item callback of the second championship value screens: toggles the entry's
// championship flag and moves to the next or the parent menu.
// FUNCTION: CMR2 0x004faea0
void FUN_004faea0(Menu *pMenu, int param)
{
    FUN_004eb0c0(CGameInfo::FUN_00405d70() + (0xff - g_unk0x00819048), pMenu->items[0].max);
    if (g_unk0x00819048 == 0) {
        Menu_SetNextAction((int)FUN_004fa350());
        return;
    }
    Menu_SetNextAction((int)FUN_004f83a0());
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

// Item callback that rebuilds a horizontally scrolling menu: the visible item
// count comes from the current difficulty and the item widths are measured
// from the localised strings.
// FUNCTION: CMR2 0x004fb010
void FUN_004fb010(Menu *pMenu, int param)
{
    MenuScroller *pScroller;
    unsigned int *pFlags;
    unsigned int count;
    unsigned int n;
    int i;

    FUN_004ea480(0);
    pScroller = FUN_004f2590();
    pScroller->startTime = CFrontend::FUN_004d20e0();
    pScroller->count = pMenu->itemCount;
    pScroller->pMenu = pMenu;
    for (i = 0; i < 8; i++) {
        FUN_004fa340()->items[i].enabled &= ~1;
        FUN_004fa350()->items[i].enabled &= ~1;
    }
    pFlags = CGameInfo::FUN_00405db0();
    if (CGameInfo::FUN_00405d80() == 7) {
        count = (*pFlags >> 2 & 3) * 3;
        n = (*pFlags >> 4 & 3) * 3;
        if (n > count)
            count = n;
        n = (*pFlags >> 6 & 3) * 3;
        if (n > count)
            count = n;
    } else if (CGameInfo::FUN_00405d70() == 1) {
        switch (CGameInfo::FUN_00405d90()) {
        case 0:
            count = (*pFlags >> 2 & 3) * 3;
            break;
        case 1:
            count = (*pFlags >> 4 & 3) * 3;
            break;
        case 2:
            count = (*pFlags >> 6 & 3) * 3;
            break;
        }
    } else {
        count = (*pFlags >> 6 & 3) * 3;
        n = (*pFlags >> 4 & 3) * 3;
        if (count < n)
            count = n;
        n = (*pFlags >> 2 & 3) * 3;
        if (count < n)
            count = n;
    }
    for (i = 0; i < 6; i++) {
        if (i < (int)count || CGameInfo::FUN_00406410(0xd)) {
            FUN_004fa340()->items[i].enabled |= 1;
            FUN_004fa350()->items[i].enabled |= 1;
        } else {
            FUN_004fa340()->items[i].enabled &= ~1;
            FUN_004fa350()->items[i].enabled &= ~1;
        }
    }
    if (CGameInfo::FUN_00405d80() == 7) {
        pScroller->count = 8;
        pMenu->itemCount = 8;
        if ((*pFlags & 0x100000) || CGameInfo::FUN_00406410(0xd))
            FUN_004fa350()->items[6].enabled |= 1;
        if ((*pFlags & 0x200000) || CGameInfo::FUN_00406410(0xd))
            FUN_004fa350()->items[7].enabled |= 1;
    } else if (CGameInfo::FUN_00405d90() == 0 && CGameInfo::FUN_00405d70() == 1) {
        pScroller->count = 3;
        pMenu->itemCount = 3;
        for (i = count; i < pScroller->count; i++)
            FUN_004fa340()->items[i].enabled &= ~1;
    } else {
        pScroller->count = 8;
        pMenu->itemCount = 8;
        if ((*pFlags & 0x100000) || CGameInfo::FUN_00406410(0xd))
            FUN_004fa340()->items[6].enabled |= 1;
        if ((*pFlags & 0x200000) || CGameInfo::FUN_00406410(0xd))
            FUN_004fa340()->items[7].enabled |= 1;
    }
    pScroller->spacing = (int)(g_pGraphics->resX * 24) / 640;
    pScroller->count = pMenu->itemCount;
    pScroller->offset = 0;
    pScroller->startOffset = 0;
    for (i = 0; i < pMenu->itemCount; i++) {
        strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(pMenu->items[i].id));
        CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
        pScroller->widths[i] = Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest);
    }
}

// FUNCTION: CMR2 0x004fb360
void FUN_004fb360(Menu *pMenu)
{
    FUN_004f37c0(FUN_004f2590());
}


// Selects the menu row/page for a front-end list screen.
// FUNCTION: CMR2 0x004fb370
void FUN_004fb370(int param_1, int unused)
{
    RallyData_FUN_0040d600((int)*(signed char *)(param_1 + 7) / 3);
    RallyData_FUN_00406960((int)*(signed char *)(param_1 + 7) % 3);
    if (CGameInfo::FUN_00405d70() == 2)
        RallyData_FUN_0040d640(0);
    else
        RallyData_FUN_0040d640(5);
    if (CGameInfo::FUN_00405d80() == 6) {
        if (CGameInfo::FUN_00405d70() == 2)
            Menu_SetNextAction((int)FUN_004fa4c0());
        else
            Menu_SetNextAction((int)FUN_004fa360());
        return;
    }
    RallyData_FUN_0040d620(3);
    Menu_SetNextAction((int)FUN_004f8330());
}

void FUN_004ea930(BYTE param1);
void FUN_004ea8c0(BYTE param1);
void FUN_004ea950(BYTE param1);
void FUN_004eadb0(void);
void FUN_004eb000(BYTE index, char set);
void FUN_004eb860(int index, int profile);
void FUN_004ebf20(int index);
char *FUN_004eb2e0(char *pName);
void *FUN_004eb4b0(char *param1, int param2);
BYTE *RallyData_FUN_00408860(int index);
struct Unk0x0052ebc0 {
    BYTE field_0x0[0x148];
};
Unk0x0052ebc0 *RallyData_FUN_00407610(int index);
BYTE *RallyData_FUN_00407630(int index);
BYTE *RallyData_FUN_00408d60(int index);
BYTE RallyData_FUN_00409010(int index, int bit);
BYTE *RallyData_FUN_004075d0(int index);
int *FUN_00407520(int index);
int *RallyData_FUN_004075e0(int index);
int *RallyData_FUN_004075b0(int index);
int *RallyData_FUN_004075c0(int index);
void FUN_0040cff0(int index, char value);
void RallyData_FUN_004068e0(BYTE param1);
void RallyData_FUN_004068d0(char param1);
void RallyData_FUN_0040e330(char param1);

// Applies a 0x7e0-byte driver profile block to the game state: resets the four
// players, copies each one's car record (0x148), controller setup (7) and name
// (0xc) plus the four gear-ratio tables and the 16 key bindings, and raises the
// per-mode event ratios.
// match 59%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// logic verified against the dump; the remaining diff is instruction selection
// in the bit-field inserts and MSVC's basic-block order.
// FUNCTION: CMR2 0x004fb400
unsigned int FUN_004fb400(BYTE *pBlock)
{
    BYTE *pName;
    BYTE *pSetup;
    BYTE *pDest;
    BYTE *pSrc;
    BYTE *pRecord;
    BYTE *pBuffer;
    unsigned int *pState;
    unsigned int *pOut;
    unsigned int value;
    unsigned int i;

    if (pBlock == NULL)
        return 0;
    FUN_004ea8e0(pBlock[0x30] & 0x7f);
    FUN_004ea930((*(unsigned int *)(pBlock + 0x30) >> 7) & 7);
    FUN_004ea8c0((*(unsigned int *)(pBlock + 0x30) >> 0xa) & 0xf);
    FUN_004ea950((*(unsigned int *)(pBlock + 0x30) >> 0xe) & 1);
    FUN_004eadb0();
    i = 0;
    if (CGameInfo::FUN_00405d70() != 0) {
        pName = pBlock + 0xc4;
        pSetup = pBlock + 0x614;
        pDest = pBlock + 0xf4;
        pSrc = pBlock + 0xb4;
        do {
            FUN_004eb860(i, -1);
            FUN_004ebf20(i);
            FUN_004eb000((BYTE)i, 0);
            pRecord = RallyData_FUN_00408860(i);
            *(unsigned int *)(pRecord + 0x5c) =
                (*(unsigned int *)(pRecord + 0x5c) ^ *(unsigned int *)pSrc) & 0x300 ^
                *(unsigned int *)(pRecord + 0x5c);
            pRecord = RallyData_FUN_00408860(i);
            *(unsigned int *)(pRecord + 0x5c) =
                (*(unsigned int *)(pRecord + 0x5c) ^ *(unsigned int *)pSrc) & 0x38 ^
                *(unsigned int *)(pRecord + 0x5c);
            pRecord = RallyData_FUN_00408860(i);
            *(unsigned int *)(pRecord + 0x5c) =
                (*(unsigned int *)(pRecord + 0x5c) ^ *(unsigned int *)pSrc) & 0x1c00 ^
                *(unsigned int *)(pRecord + 0x5c);
            pRecord = RallyData_FUN_00408860(i);
            *(unsigned int *)(pRecord + 0x5c) =
                (*(unsigned int *)(pRecord + 0x5c) ^ *(unsigned int *)pSrc) & 0xc0 ^
                *(unsigned int *)(pRecord + 0x5c);
            pRecord = RallyData_FUN_00408860(i);
            *(unsigned int *)(pRecord + 0x5c) =
                (*(unsigned int *)(pRecord + 0x5c) ^ *(unsigned int *)pSrc) & 7 ^
                *(unsigned int *)(pRecord + 0x5c);
            pRecord = RallyData_FUN_00408860(i);
            *(unsigned int *)(pRecord + 0x10) = *(unsigned int *)pName;
            *(unsigned int *)(pRecord + 0x14) = *((unsigned int *)pName + 1);
            *(unsigned int *)(pRecord + 0x18) = *((unsigned int *)pName + 2);
            memcpy(RallyData_FUN_00407610(i), pDest, 0x148);
            pBuffer = RallyData_FUN_00407630(i);
            *(unsigned int *)pBuffer = *(unsigned int *)pSetup;
            *(unsigned short *)(pBuffer + 4) = *(unsigned short *)(pSetup + 4);
            pBuffer[6] = pSetup[6];
            if ((*(unsigned int *)(RallyData_FUN_00408860(i) + 0x14) & 0x200000) == 0) {
                pBuffer = (BYTE *)FUN_004eb4b0(FUN_004eb2e0((char *)pName), 0);
                if (pBuffer == NULL) {
                    FUN_004eb000((BYTE)i, 0);
                } else {
                    memcpy(RallyData_FUN_00408860(i), pBuffer, 0x650);
                    pRecord = RallyData_FUN_00408860(i);
                    *(unsigned int *)(pRecord + 0x10) = *(unsigned int *)pName;
                    *(unsigned int *)(pRecord + 0x14) = *((unsigned int *)pName + 1);
                    *(unsigned int *)(pRecord + 0x18) = *((unsigned int *)pName + 2);
                    CFileBuffer::FreeGenericFileBuffer(pBuffer);
                }
            }
            pDest += 0x148;
            pSetup += 7;
            pName += 0xc;
            i++;
            pSrc += 4;
        } while ((int)i < (int)(CGameInfo::FUN_00405d70() & 0xff));
    }
    pState = (unsigned int *)(pBlock + 0x34);
    i = 0;
    pOut = pState;
    do {
        pRecord = RallyData_FUN_00408d60(i);
        *(unsigned int *)pRecord = (*(unsigned int *)pRecord ^ *pOut) & 0x3f ^ *(unsigned int *)pRecord;
        pRecord = RallyData_FUN_00408d60(i);
        *(unsigned int *)pRecord = *pOut >> 1 & 0x1fc0 | *(unsigned int *)pRecord & 0xffffe03f;
        pRecord = RallyData_FUN_00408d60(i);
        i++;
        pOut += 2;
        *(unsigned int *)(pRecord + 4) = *(pOut - 1);
    } while ((int)i < 0x10);
    i = 0;
    if (CGameInfo::FUN_00405d70() != 0) {
        do {
            FUN_004eb0c0((BYTE)i, (BYTE)(*pState >> 6) & 1);
            pRecord = RallyData_FUN_00408d60(i);
            RallyData_FUN_00409010(i, *(unsigned int *)pRecord & 0x3f);
            i++;
            pState += 2;
        } while ((int)i < (int)(CGameInfo::FUN_00405d70() & 0xff));
    }
    i = 0;
    do {
        FUN_0040cff0(i, pBlock[0x7dc + i]);
        i++;
    } while ((int)i < 0x10);
    memcpy(FUN_00407520(0), pBlock + 0x630, 0xa0);
    memcpy(RallyData_FUN_004075e0(0), pBlock + 0x6d0, 0x50);
    memcpy(RallyData_FUN_004075d0(0), pBlock + 0x720, 0x14);
    memcpy(RallyData_FUN_004075b0(0), pBlock + 0x734, 0x50);
    memcpy(RallyData_FUN_004075c0(0), pBlock + 0x784, 0x50);
    RallyData_FUN_004068b0(pBlock[0x7d8] & 0x1f);
    RallyData_FUN_004068e0((*(unsigned int *)(pBlock + 0x7d8) >> 5) & 0x1f);
    RallyData_FUN_004068d0(pBlock[0x7d4]);
    FUN_004eb160();
    pOut = CGameInfo::FUN_00405db0();
    switch (CGameInfo::FUN_00405d90()) {
    case 0:
        value = RallyDataCountryIndex() & 0xff;
        if ((*pOut >> 8 & 0xf) < value + 1)
            *pOut = (*pOut & ~0xf00) | ((RallyDataCountryIndex() & 0xff) + 1) << 8;
        break;
    case 1:
        value = RallyDataCountryIndex() & 0xff;
        if ((*pOut >> 8 & 0xf) < value + 1)
            *pOut = (*pOut & ~0xf00) | ((RallyDataCountryIndex() & 0xff) + 1) << 8;
        value = RallyDataCountryIndex() & 0xff;
        if ((*pOut >> 0xc & 0xf) < value + 1)
            *pOut = (*pOut & ~0xf000) | ((RallyDataCountryIndex() & 0xff) + 1) << 0xc;
        break;
    case 2:
        value = RallyDataCountryIndex() & 0xff;
        if ((*pOut >> 8 & 0xf) < value + 1)
            *pOut = (*pOut & ~0xf00) | ((RallyDataCountryIndex() & 0xff) + 1) << 8;
        value = RallyDataCountryIndex() & 0xff;
        if ((*pOut >> 0xc & 0xf) < value + 1)
            *pOut = (*pOut & ~0xf000) | ((RallyDataCountryIndex() & 0xff) + 1) << 0xc;
        value = RallyDataCountryIndex() & 0xff;
        if ((*pOut >> 0x10 & 0xf) < value + 1)
            *pOut = (*pOut & ~0xf0000) | ((RallyDataCountryIndex() & 0xff) + 1) << 0x10;
        break;
    }
    RallyData_FUN_0040e330(0);
    return 1;
}

void FUN_004fb9c0(unsigned int param_1, unsigned int param_2, BYTE param_3, char *param_4);

// Three 12-byte keyboard-style tables scrambled by FUN_004fb8d0.
// GLOBAL: CMR2 0x00526ea4
char g_unk0x00526ea4[12] = "qaz2wsx3e";
// GLOBAL: CMR2 0x00526eb0
char g_unk0x00526eb0[12] = "/-['=]\\`!Q";
// GLOBAL: CMR2 0x00526ebc
char g_unk0x00526ebc[12] = "\\`!QAZ@WSX";

// Scrambles the dword *pNumber with the three key tables above (seeded by
// *pByte) and turns the result into an identifier string through
// FUN_004fb9c0.
// match 48%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// logic verified against the dump; remaining diff is the operand evaluation
// order of the byte sums and the register/stack split (the original keeps the
// loop pointer in EDI descending to &g_unk0x00526ea4[6]).
// FUNCTION: CMR2 0x004fb8d0
void FUN_004fb8d0(unsigned int param_1, unsigned int *pNumber, char *pByte, char *pOut)
{
    BYTE bytes[4];
    char *p;
    char seed;
    int i;

    seed = *pByte;
    p = &g_unk0x00526ea4[10];
    bytes[0] = (BYTE)*pNumber;
    bytes[1] = (BYTE)(*pNumber >> 8);
    bytes[2] = (BYTE)(*pNumber >> 16);
    bytes[3] = (BYTE)(*pNumber >> 24);
    i = 0;
    do {
        bytes[i] += g_unk0x00526ebc[i] + g_unk0x00526eb0[i] + g_unk0x00526ea4[i] + *p * 3;
        i++;
        p--;
    } while (p - &g_unk0x00526ea4[6] > 0);
    FUN_004fb9c0(param_1, bytes[3] << 24 | bytes[2] << 16 | bytes[1] << 8 | bytes[0],
                 g_unk0x00526ebc[6] + g_unk0x00526eb0[6] + g_unk0x00526ea4[6] + g_unk0x00526ebc[4] +
                     g_unk0x00526eb0[4] + g_unk0x00526ea4[4] + seed,
                 pOut);
}

// Builds a profile-less identifier string: the decimal digits of param_2
// followed by five checksum digits derived from the byte sum of param_2 and
// param_3.
// FUNCTION: CMR2 0x004fb9c0
void FUN_004fb9c0(unsigned int param_1, unsigned int param_2, BYTE param_3, char *param_4)
{
    unsigned short checksum;
    BYTE sum;
    int i;
    int n;

    i = 0;
    sum = (BYTE)(param_2 >> 0x18) + (BYTE)(param_2 >> 0x10) + (BYTE)(param_2 >> 8) + (BYTE)param_2 + param_3 + 2;
    if ((int)param_1 > 0) {
        param_2 ^= param_1;
        param_3 ^= (BYTE)param_1;
    }
    while (param_2 != 0) {
        param_4[i] = (char)(param_2 % 10) + '0';
        i++;
        param_2 /= 10;
    }
    checksum = (unsigned short)(param_3 * 0x100 + sum);
    for (n = 5; n != 0; n--) {
        param_4[i] = (char)(checksum % 10) + '0';
        i++;
        checksum /= 10;
    }
    param_4[i] = 0;
}
