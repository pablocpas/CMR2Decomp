#include <windows.h>
#include <string.h>
#include "FrontendDraw.h"
#include "Frontend.h"
#include "Graphics.h"
#include "Font.h"
#include "Sprite.h"
#include "RallyData.h"
#include "GameInfo.h"
#include "FrontendMenus.h"
#include <stdio.h>

// GLOBAL: CMR2 0x00524968
BYTE g_colourWhite0x00524968[4] = { 0xff, 0xff, 0xff, 0xff };
// GLOBAL: CMR2 0x0052496c
BYTE g_colourText0x0052496c[4] = { 0xd7, 0xeb, 0xda, 0xff };

// GLOBAL: CMR2 0x00524b88
BYTE g_helpColour0x00524b88[4] = { 0xff, 0xff, 0xff, 0xff };
// GLOBAL: CMR2 0x00524b8c
char g_helpPulseUp0x00524b8c = 1;
// GLOBAL: CMR2 0x00818270
char g_helpPulse0x00818270;

// GLOBAL: CMR2 0x00818430
char g_unk0x00818430[256];
// GLOBAL: CMR2 0x008189a8
short g_unk0x008189a8[4];

// One element of the "A | B | C" path shown at the top of the menus: the
// text and, unless it is the last one, a one pixel separator after it.
// Returns the x where the next element starts.
// FUNCTION: CMR2 0x004d3e80
int FrontendDraw_BreadcrumbItem(int x, int y, BYTE *pColour, char last, char *text)
{
    int next;

    Font_DrawText(1, text, x, y, (int *)pColour, 0x11);
    if (last == 0) {
        next = x + Font_GetTextWidth(1, (BYTE *)text) + (int)(g_pGraphics->resX * 5) / 640;
        g_unk0x008189a8[0] = next;
        g_unk0x008189a8[2] = 1;
        g_unk0x008189a8[1] = (int)(g_pGraphics->resY * 38) / 480 - (int)(g_pGraphics->resY * 15) / 480;
        g_unk0x008189a8[3] = (int)(g_pGraphics->resY * 23) / 480;
        Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x008189a8, g_colourText0x0052496c, 1);
        return next + (int)(g_pGraphics->resX * 5) / 640;
    }
    return x;
}

// FUNCTION: CMR2 0x004d3fa0
void FrontendDraw_Breadcrumb(int x, int y, char **ppText, int count)
{
    char last;
    BYTE *pColour;
    int i;

    for (i = 0; i < count; i++) {
        if (ppText[i] != NULL) {
            if (i == count - 1) {
                last = 1;
                pColour = g_colourWhite0x00524968;
            } else {
                last = 0;
                pColour = g_colourText0x0052496c;
            }
            strcpy(g_unk0x00818430, ppText[i]);
            x = FrontendDraw_BreadcrumbItem(x, y, pColour, last, g_unk0x00818430);
        }
    }
}

// FUNCTION: CMR2 0x004d4040
void FrontendDraw_MenuTitle(Menu *pMenu)
{
    char *text[3];

    text[0] = CFrontend::GetTextString(0x17c);
    text[1] = (char *)RallyData_GetRecord(0);
    text[2] = CFrontend::GetTextString(pMenu->field_0x4);
    FrontendDraw_Breadcrumb((int)(g_pGraphics->resX * 24) / 640, (int)(g_pGraphics->resY * 38) / 480, text, 3);
}

// Draws the path of parent menus up to pMenu, most distant first. depth
// limits how many parents are shown (-1 = all); ppNames can override the
// names of the last nameCount parents.
// FUNCTION: CMR2 0x004d40d0
int FrontendDraw_MenuPath(Menu *pMenu, int x, int y, char last, int depth, char **ppNames, int nameCount)
{
    BYTE *pColour;
    int n;

    if (depth < 1 && depth != -1)
        return x;
    if (pMenu->pParent != NULL && pMenu != FUN_004f8410()) {
        x = FrontendDraw_MenuPath(pMenu->pParent, x, y, 0, depth == -1 ? -1 : depth - 1, ppNames,
                                  last == 0 ? nameCount - 1 : nameCount);
    }
    pColour = g_colourWhite0x00524968;
    if (last == 0)
        pColour = g_colourText0x0052496c;
    if (ppNames != NULL && nameCount > 0 && last == 0)
        strcpy(CFrontend::m_stringDest, ppNames[nameCount - 1]);
    else if (pMenu->stringId != 0)
        strcpy(CFrontend::m_stringDest, (char *)pMenu->stringId);
    else
        strcpy(CFrontend::m_stringDest, CFrontend::GetTextString(pMenu->field_0x4));
    return FrontendDraw_BreadcrumbItem(x, y, pColour, last, CFrontend::m_stringDest);
}

// Total play time and the title of the current section, top right.
// FUNCTION: CMR2 0x004d41e0
void FrontendDraw_PlayTime(void)
{
    unsigned int ms;
    unsigned int minutes;
    char text[52];

    ms = CFrontend::FUN_004d20d0();
    minutes = ms / 1000 / 60;
    sprintf(text, "%.2d:%.2d.%.2d", minutes / 60, minutes % 60, ms / 1000 % 60);
    Font_DrawText(3, text, (int)(g_pGraphics->resX * 539) / 640, (int)(g_pGraphics->resY * 38) / 480,
                  (int *)g_colourText0x0052496c, 0x11);
    Font_DrawText(1, CFrontend::GetTextString(0x4e), (int)(g_pGraphics->resX * 535) / 640,
                  (int)(g_pGraphics->resY * 38) / 480, (int *)g_colourText0x0052496c, 0x14);
    if (CGameInfo::FUN_00406410(0x11))
        Font_DrawText(1, "Automode", (int)(g_pGraphics->resX * 635) / 640, (int)(g_pGraphics->resY * 5) / 480,
                      (int *)g_colourText0x0052496c, 0xc);
}

// Help line at the bottom of the screen, its brightness pulsing up and down.
// FUNCTION: CMR2 0x004d4460
void FrontendDraw_HelpText(char *text, int reset)
{
    if (text != NULL) {
        if (reset == 0) {
            if (g_helpPulseUp0x00524b8c != 0) {
                g_helpPulse0x00818270++;
                if (g_helpPulse0x00818270 > 32) {
                    g_helpPulse0x00818270--;
                    g_helpPulseUp0x00524b8c = 0;
                }
            } else {
                g_helpPulse0x00818270--;
                if (g_helpPulse0x00818270 < -32) {
                    g_helpPulse0x00818270++;
                    g_helpPulseUp0x00524b8c = 1;
                }
            }
            g_helpColour0x00524b88[1] = g_colourText0x0052496c[1] + g_helpPulse0x00818270;
            g_helpColour0x00524b88[0] = g_colourText0x0052496c[0] + g_helpPulse0x00818270;
            g_helpColour0x00524b88[3] = g_colourText0x0052496c[3];
            g_helpColour0x00524b88[2] = g_colourText0x0052496c[2] + g_helpPulse0x00818270;
        } else {
            g_helpPulse0x00818270 = 0;
            g_helpColour0x00524b88[0] = g_colourText0x0052496c[0];
            g_helpColour0x00524b88[1] = g_colourText0x0052496c[1];
            g_helpPulseUp0x00524b8c = 1;
            g_helpColour0x00524b88[2] = g_colourText0x0052496c[2];
            g_helpColour0x00524b88[3] = g_colourText0x0052496c[3];
        }
        sprintf(CFrontend::m_stringDest, text);
        Font_Unused((int)CFrontend::m_stringDest, FUN_004ea500());
        Font_DrawText(1, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 102) / 640,
                      (int)(g_pGraphics->resY * 64) / 480 + (int)(g_pGraphics->resY * 384) / 480,
                      (int *)g_helpColour0x00524b88, 0x11);
    }
}

