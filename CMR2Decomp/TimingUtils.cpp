#include "TimingUtils.h"
#include "RallyTiming.h"
#include "FixedPoint.h"
#include <windows.h>
#include "Graphics.h"

#include <stdio.h>

// GLOBAL: CMR2 0x00519f58
char g_minSecMSECFormatString[] = "%02d:%02d.%02d";

// FUNCTION: CMR2 0x004de170
void FormatCentisecondsAsMinSecMSec(int iTime, char *pcFormattedTime)
{
	sprintf(pcFormattedTime, g_minSecMSECFormatString, iTime / 6000, (iTime % 6000) / 100, iTime % 100);
}

// FUNCTION: CMR2 0x0040d480
int ConvertRawTimeToCentiseconds(int iTime)
{
	// raw times are 16.16 fixed point seconds; 0x28f5c28 = 0.01 * 2^32.
	return FixDiv(iTime + 0x147, 0x28f5c28);
}

// FUNCTION: CMR2 0x0040d3f0
void RallyTiming_SetOverallTimeRaw(int iDriver, int iCentiseconds)
{
	g_rallyOverallTimesRaw[iDriver] = FixMul(iCentiseconds, 0x28f5c28);
}

extern double g_minus65536;

// Converts a time in hundredths of a second into 16.16 seconds.
// FUNCTION: CMR2 0x0040d4b0
int Timing_CentisecondsToFixedSeconds(int hundredths)
{
    return FixDiv((int)(__int64)((hundredths % 100) * CGraphics::m_65536), 0x640000) -
           (int)(__int64)((hundredths / 100) * g_minus65536);
}


#include "Frontend.h"
#include "FrontendDraw.h"
#include "Font.h"
#include "Sprite.h"
#include "GameInfo.h"
#include "NetPlayers.h"

extern char g_str0x00519fb0[4];
extern char g_str0x00519fb4[4];
extern char g_strNum1[4];
extern char g_stageNumberFormat[];
extern BYTE g_colourWhite0x00524968[4];
extern BYTE g_colourText0x0052496c[4];

int NetPlayers_GetClassificationCount(void);
int NetPlayers_FindLocalClassificationPosition(void);
NetClassification *NetPlayers_GetClassificationRecord(int index);
unsigned int NetPlayers_GetClassificationTime(int index);
BYTE *NetworkLeaderboard_GetPublishedBoard(void);

// Draws the stage-times screen: the two header bars, the three column titles,
// the split table of the drivers in the race and the leaderboard rows of the
// current stage.
// FUNCTION: CMR2 0x004de1d0
void FrontendDraw_DrawStageTimes(int unused)
{
    BYTE colour[4];
    short rectB[4];
    short rectA[4];
    char *pText[4];
    int i;
    char *p;
    BYTE *pBoard;
    int *pColour;

    colour[0] = 0xff;
    colour[1] = 0xff;
    colour[2] = 0xff;
    colour[3] = 0x20;

    rectA[0] = (short)((int)(g_pGraphics->resX * 100) / 0x280);
    rectA[1] = (short)((int)(g_pGraphics->resY * 0x5a) / 0x1e0);
    rectA[2] = (short)((int)(g_pGraphics->resX * 300) / 0x280);
    rectA[3] = (short)((int)(g_pGraphics->resY * 0x118) / 0x1e0);
    rectB[0] = (short)((int)(g_pGraphics->resX * 10) / 0x280 + (int)(g_pGraphics->resX * 100) / 0x280 +
                       (int)(g_pGraphics->resX * 300) / 0x280);
    rectB[1] = (short)((int)(g_pGraphics->resY * 0x5a) / 0x1e0);
    rectB[2] = (short)((int)(g_pGraphics->resX * 0x82) / 0x280);
    rectB[3] = (short)((int)(g_pGraphics->resY * 0x118) / 0x1e0);

    Sprite_FillRect((int)g_pGraphics + 0x150, rectA, colour, 1);
    Sprite_FillRect((int)g_pGraphics + 0x150, rectB, colour, 1);

    pText[0] = CFrontend::GetTextString(0x12);
    if (CGameInfo::GetConfiguredGameMode() == 10)
        pText[1] = CFrontend::GetTextString(0x1c9);
    else
        pText[1] = CFrontend::GetTextString(0x1f6);
    FrontendDraw_Breadcrumb((int)(g_pGraphics->resX * 0x18) / 0x280, (int)(g_pGraphics->resY * 0x26) / 0x1e0,
                            pText, 2);
    FrontendDraw_PlayTime();

    Font_DrawText(1, CFrontend::GetTextString(0x7f), (int)(g_pGraphics->resX * 0x96) / 0x280,
                  (int)(g_pGraphics->resY * 100) / 0x1e0, (int *)g_colourText0x0052496c, 10);
    Font_DrawText(1, CFrontend::GetTextString(0x1ca), (int)(g_pGraphics->resX * 0xfa) / 0x280,
                  (int)(g_pGraphics->resY * 100) / 0x1e0, (int *)g_colourText0x0052496c, 10);
    Font_DrawText(1, CFrontend::GetTextString(0x1f8), (int)(g_pGraphics->resX * 0x15e) / 0x280,
                  (int)(g_pGraphics->resY * 100) / 0x1e0, (int *)g_colourText0x0052496c, 10);

    i = 0;
    if (NetPlayers_GetClassificationCount() > 0) {
        do {
            if (i == NetPlayers_FindLocalClassificationPosition())
                pColour = (int *)g_colourWhite0x00524968;
            else
                pColour = (int *)g_colourText0x0052496c;
            Font_DrawText(0, (char *)NetPlayers_GetClassificationRecord(i), (int)(g_pGraphics->resX * 0x96) / 0x280, (int)(g_pGraphics->resY * 0x14) / 0x1e0 * i + (int)(g_pGraphics->resY * 0x82) / 0x1e0, pColour, 10);
            if (NetPlayers_GetClassificationTime(i) == -1)
                sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x200));
            else
                FormatCentisecondsAsMinSecMSec(NetPlayers_GetClassificationTime(i), CFrontend::m_stringDest);
            Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0xfa) / 0x280, (int)(g_pGraphics->resY * 0x14) / 0x1e0 * i + (int)(g_pGraphics->resY * 0x82) / 0x1e0, pColour, 10);
            if (NetPlayers_GetClassificationTime(0) != -1 && NetPlayers_GetClassificationTime(i) == NetPlayers_GetClassificationTime(0)) {
                Font_DrawText(0, g_strNum1, (int)(g_pGraphics->resX * 0x15e) / 0x280, (int)(g_pGraphics->resY * 0x14) / 0x1e0 * i + (int)(g_pGraphics->resY * 0x82) / 0x1e0, pColour, 10);
            } else {
                Font_DrawText(0, g_str0x00519fb0, (int)(g_pGraphics->resX * 0x15e) / 0x280, (int)(g_pGraphics->resY * 0x14) / 0x1e0 * i + (int)(g_pGraphics->resY * 0x82) / 0x1e0, pColour, 10);
            }
            i++;
        } while (i < NetPlayers_GetClassificationCount());
    }
    for (i = NetPlayers_GetClassificationCount(); i < 8; i++) {
        Font_DrawText(0, g_str0x00519fb4, (int)(g_pGraphics->resX * 0x96) / 0x280, (int)(g_pGraphics->resY * 0x14) / 0x1e0 * i + (int)(g_pGraphics->resY * 0x82) / 0x1e0,
                      (int *)g_colourText0x0052496c, 10);
        Font_DrawText(0, CFrontend::GetTextString(0x200), (int)(g_pGraphics->resX * 0xfa) / 0x280, (int)(g_pGraphics->resY * 0x14) / 0x1e0 * i + (int)(g_pGraphics->resY * 0x82) / 0x1e0,
                      (int *)g_colourText0x0052496c, 10);
        Font_DrawText(0, g_str0x00519fb0, (int)(g_pGraphics->resX * 0x15e) / 0x280, (int)(g_pGraphics->resY * 0x14) / 0x1e0 * i + (int)(g_pGraphics->resY * 0x82) / 0x1e0,
                      (int *)g_colourText0x0052496c, 10);
    }

    pBoard = NetworkLeaderboard_GetPublishedBoard();
    Font_DrawText(1, CFrontend::GetTextString(0x201), (int)rectB[2] / 2 + *(int *)rectB,
                  (int)(g_pGraphics->resY * 100) / 0x1e0, (int *)g_colourText0x0052496c, 10);
    if (pBoard != NULL) {
        for (i = 0, p = (char *)(pBoard + 4); i < 10; i++, p += 8) {
            if (*p != '\0') {
                Font_DrawText(0, p, (int)(g_pGraphics->resX * 0x1c2) / 0x280, (int)(g_pGraphics->resY * 0x14) / 0x1e0 * (i + 1) + (int)(g_pGraphics->resY * 0x82) / 0x1e0,
                              (int *)g_colourText0x0052496c, 10);
                sprintf(CFrontend::m_stringDest, g_stageNumberFormat, *(int *)(p + 4));
                Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x1fe) / 0x280, (int)(g_pGraphics->resY * 0x14) / 0x1e0 * (i + 1) + (int)(g_pGraphics->resY * 0x82) / 0x1e0,
                              (int *)g_colourText0x0052496c, 10);
            } else {
                Font_DrawText(0, g_str0x00519fb4, (int)(g_pGraphics->resX * 0x1c2) / 0x280, (int)(g_pGraphics->resY * 0x14) / 0x1e0 * (i + 1) + (int)(g_pGraphics->resY * 0x82) / 0x1e0,
                              (int *)g_colourText0x0052496c, 10);
                Font_DrawText(0, g_str0x00519fb0, (int)(g_pGraphics->resX * 0x1fe) / 0x280, (int)(g_pGraphics->resY * 0x14) / 0x1e0 * (i + 1) + (int)(g_pGraphics->resY * 0x82) / 0x1e0,
                              (int *)g_colourText0x0052496c, 10);
            }
        }
    } else {
        Font_DrawText(0, CFrontend::GetTextString(0x202), (int)rectB[2] / 2 + *(int *)rectB,
                      (int)(g_pGraphics->resY * 0x14) / 0x1e0 + (int)(g_pGraphics->resY * 0x82) / 0x1e0,
                      (int *)g_colourText0x0052496c, 10);
        return;
    }
}
