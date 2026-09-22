#include "Game.h"
#include "StageUI.h"

// GLOBAL: CMR2 0x00517e14
char g_positiveSymbol[2] = "+";

// GLOBAL: CMR2 0x00517e18
char g_negativeSymbol[2] = "-";

// STUB: CMR2 0x00415bd0
void FormatGapToLeader(int iLeaderGap, unsigned int param_2, unsigned char param_3, int param_4, int param_5, void *param_6, unsigned int param_7, char *pcNegPosSymbol, int param_9)
{
}

// FUNCTION: CMR2 0x00415db0
void PrepareFormatGapToLeader(int iLeaderGap, unsigned char param_2, unsigned int param_3, unsigned char param_4, int param_5, int param_6, void *param_7, unsigned int param_8, BOOL bIsAhead, int param_10)
{
    if (bIsAhead)
    {
        FormatGapToLeader(iLeaderGap, param_3, param_4, param_5, param_6, param_7, param_8, g_negativeSymbol, param_10);
        return;
    }

    FormatGapToLeader(iLeaderGap, param_3, param_4, param_5, param_6, param_7, param_8, g_positiveSymbol, param_10);
}

// GLOBAL: CMR2 0x00537660
int g_unk0x00537660;
// GLOBAL: CMR2 0x00537dcc
BYTE g_unk0x00537dcc;

// Reinicia las tablas de la interfaz de etapa y registra su callback una vez.
// TODO: CMR2 0x00418f20 (implemented, match 54%)
void FUN_00418f20(void)
{
    BYTE *p;
    int i;

    g_unk0x00537660 = 0;
    memset((void *)0x5375b0, 0, 0x20);
    memset((void *)0x5375fc, 0, 0x20);
    memset((void *)0x537788, 0, 0x20);
    memset((void *)0x537568, 0, 0x20);
    for (p = (BYTE *)0x53784c; p < (BYTE *)0x537dec; p += 0xb4) {
        for (i = 0; i < 10; i++) {
            *(int *)(p - 0x88 + i * 4) = -1;
            *(int *)(p - 0x60 + i * 4) = -1;
        }
        *(int *)p = 0;
        *(int *)(p - 4) = 0;
        *(int *)(p - 0xa4) = -1;
        *(int *)(p - 0xa0) = -1;
        *(unsigned short *)(p - 0x140) = 0xffff;
        *(unsigned short *)(p - 0x13e) = 0xffff;
        *(int *)(p - 0xb0) = 0x19;
    }
    if (g_unk0x00537dcc == 0) {
        CGame::RegisterCallback((void *)0x418fe0, NULL);
        g_unk0x00537dcc = 1;
    }
}


// Frame/tick counters shared by the in-game UI (0x537df0..0x537efc).
// GLOBAL: CMR2 0x00537dd0
BYTE g_unk0x00537dd0[0x20];
// GLOBAL: CMR2 0x00537df0
int g_unk0x00537df0;
// GLOBAL: CMR2 0x00537ef4
BYTE g_unk0x00537ef4;
// GLOBAL: CMR2 0x00537ef5
BYTE g_unk0x00537ef5;
// GLOBAL: CMR2 0x00537ef6
BYTE g_unk0x00537ef6;
// GLOBAL: CMR2 0x00537ef8
int g_unk0x00537ef8;
// GLOBAL: CMR2 0x00537efc
int g_unk0x00537efc;

// FUNCTION: CMR2 0x0041b300
void FUN_0041b300(void)
{
    g_unk0x00537ef4 = 0;
}

// FUNCTION: CMR2 0x0041b310
void FUN_0041b310(void)
{
    g_unk0x00537ef8 = 1;
}

// FUNCTION: CMR2 0x0041b320
int FUN_0041b320(void)
{
    return g_unk0x00537efc;
}

// FUNCTION: CMR2 0x0041b330
void FUN_0041b330(void)
{
    g_unk0x00537efc = 0;
}

// FUNCTION: CMR2 0x0041b340
void FUN_0041b340(char bFlag)
{
    if (bFlag != 0)
        g_unk0x00537ef5 = 1;
    g_unk0x00537ef6++;
}

// FUNCTION: CMR2 0x0041b360
void FUN_0041b360(void)
{
    g_unk0x00537ef6 = 0;
}

// FUNCTION: CMR2 0x0041b370
BYTE FUN_0041b370(void)
{
    return g_unk0x00537ef6;
}

// FUNCTION: CMR2 0x0041b380
int FUN_0041b380(void)
{
    return g_unk0x00537df0;
}

// FUNCTION: CMR2 0x0041b390
BYTE *FUN_0041b390(void)
{
    return g_unk0x00537dd0;
}

#include <cstdio>
#include <cstring>
#include "Frontend.h"
#include "GenericFileLoader.h"
#include "Font.h"
#include "Sprite.h"
#include "Graphics.h"
#include "RallyData.h"

// GLOBAL: CMR2 0x0051c988
BYTE g_barBackColour[4] = { 0, 0, 0, 0 };
// GLOBAL: CMR2 0x0051c990
BYTE g_barTextColour[4] = { 0, 0, 0, 0 };
// GLOBAL: CMR2 0x0051c97c
BYTE *g_pUnk0x0051c97c;
// GLOBAL: CMR2 0x0058ca90
BYTE g_unk0x0058ca90[1];
// GLOBAL: CMR2 0x0058cf7c
int g_unk0x0058cf7c;

// Draws the championship banner across the top of the screen: the championship
// name, the class it is run in and, on the longer championships, the round.
// FUNCTION: CMR2 0x00475a50
void StageUI_DrawChampionshipBar(void)
{
    short screen[4];
    short bar[4];
    unsigned int *pState;
    unsigned int flags;
    int x;
    int width;

    screen[0] = 0;
    screen[1] = 0;
    screen[2] = (short)g_pGraphics->resX;
    screen[3] = (short)g_pGraphics->resY;
    bar[0] = 0;
    bar[2] = 2;
    bar[1] = (short)((int)(g_pGraphics->resY * 0x1b) / 0x1e0);
    bar[3] = (short)((int)(g_pGraphics->resY * 0x29) / 0x1e0);
    pState = RallyData_GetChampionshipState();
    flags = *pState;
    x = (int)(g_pGraphics->resX * 0x1e) / 0x280;
    Sprite_FillRect((int)g_pGraphics + 0x150, screen, g_barBackColour, 4);
    sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x7c));
    CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
    Font_DrawText(2, CFrontend::m_stringDest, x, (int)(g_pGraphics->resY * 0x39) / 0x1e0,
                  (int *)g_barTextColour, 0x11);
    width = Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest);
    x = (int)(g_pGraphics->resX * 5) / 0x280 + x + width;
    bar[0] = (short)x;
    Sprite_FillRect((int)g_pGraphics + 0x150, bar, g_barTextColour, 1);
    x = x + 2 + (int)(g_pGraphics->resX * 5) / 0x280;

    switch ((flags >> 3) & 7) {
    case 1:
        sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x4b), 1);
        break;
    case 2:
        strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(0x4c));
        break;
    case 3:
        strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(0x4d));
        break;
    case 4:
        strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(0x4e));
        break;
    }

    CGenericFileLoader::StrLowerPolish(CFrontend::m_stringDest);
    Font_DrawText(2, CFrontend::m_stringDest, x, (int)(g_pGraphics->resY * 0x39) / 0x1e0,
                  (int *)g_barTextColour, 0x11);

    if (g_pUnk0x0051c97c == g_unk0x0058ca90) {
        if (g_unk0x0058cf7c > 1 && g_unk0x0058cf7c < 6) {
            unsigned int round;

            width = Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest);
            x = width + (int)(g_pGraphics->resX * 5) / 0x280 + x;
            bar[0] = (short)x;
            bar[2] = 2;
            bar[1] = (short)((int)(g_pGraphics->resY * 0x1b) / 0x1e0);
            bar[3] = (short)((int)(g_pGraphics->resY * 0x29) / 0x1e0);
            Sprite_FillRect((int)g_pGraphics + 0x150, bar, g_barTextColour, 1);
            if (g_unk0x0058cf7c == 3 || (*pState & 0x400000) != 0) {
                round = (*pState >> 0xc) & 0xf;
            } else {
                round = ((*pState >> 0xc) & 0xf) + 1;
            }
            sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x4f), round);
            Font_DrawText(2, CFrontend::m_stringDest, x + 2 + (int)(g_pGraphics->resX * 5) / 0x280,
                          (int)(g_pGraphics->resY * 0x39) / 0x1e0, (int *)g_barTextColour, 0x11);
        }
    } else {
        width = Font_GetTextWidth(2, (BYTE *)CFrontend::m_stringDest);
        x = width + (int)(g_pGraphics->resX * 5) / 0x280 + x;
        bar[0] = (short)x;
        bar[2] = 2;
        bar[1] = (short)((int)(g_pGraphics->resY * 0x1b) / 0x1e0);
        bar[3] = (short)((int)(g_pGraphics->resY * 0x29) / 0x1e0);
        Sprite_FillRect((int)g_pGraphics + 0x150, bar, g_barTextColour, 1);
        Font_DrawText(2, CFrontend::GetTextString(0x7b), x + 2 + (int)(g_pGraphics->resX * 5) / 0x280,
                      (int)(g_pGraphics->resY * 0x39) / 0x1e0, (int *)g_barTextColour, 0x11);
    }
}

// GLOBAL: CMR2 0x0051c9f4
BYTE g_gridBackColour[4] = { 0, 0, 0, 0 };
// GLOBAL: CMR2 0x0051c9f8
BYTE g_gridColour2[4] = { 0, 0, 0, 0 };
// GLOBAL: CMR2 0x0051c9fc
BYTE g_gridColour1[4] = { 0, 0, 0, 0 };
// GLOBAL: CMR2 0x0051ca00
char g_stageGrid[3][0x294];

// Draws the little stage grid: a background panel and one cell per set entry.
// FUNCTION: CMR2 0x00477f70
void StageUI_DrawStageGrid(int unused, int set)
{
    short rect[4];
    int n;
    int i;
    int cell;
    int resX;
    short top;
    short cellW;
    short cellH;
    short left;

    resX = (int)(g_pGraphics->resX * 6) / 0x280;
    cell = (int)(g_pGraphics->resY * 6) / 0x1e0;
    left = (short)((int)(g_pGraphics->resX * 0x140) / 0x280);
    if (RallyData_FUN_00411880() != 0) {
        top = (short)(((int)g_pGraphics->resY + cell * -0x14) / 2) + 4;
    } else {
        top = (short)((int)(g_pGraphics->resY << 0xc) >> 0x10);
    }
    cellW = (short)resX;
    rect[2] = cellW * 0x14;
    cellH = (short)cell;
    rect[3] = cellH * 0x15 - 1;
    rect[0] = left + cellW * -9;
    rect[1] = top;
    Sprite_FillRect((int)g_pGraphics + 0x150, rect, g_gridBackColour, 3);

    rect[2] = (short)((int)(g_pGraphics->resX * 4) / 0x280);
    i = 8;
    rect[3] = (short)((int)(g_pGraphics->resY * 4) / 0x1e0);
    do {
        char *pCell = &g_stageGrid[0][0] + set * 0x294 + i;
        int n = 0x14;

        rect[0] = ((short)i - 0x10) * cellW + left;
        rect[1] = top;
        do {
            if (*pCell != 0) {
                BYTE *pColour = g_gridColour2;

                if (*pCell != 2) {
                    pColour = g_gridColour1;
                }
                Sprite_FillRect((int)g_pGraphics + 0x150, rect, pColour, 3);
            }
            rect[1] = rect[1] + cellH;
            pCell = pCell + 0x21;
            n--;
        } while (n != 0);
        i++;
    } while (i < 0x1a);
}
