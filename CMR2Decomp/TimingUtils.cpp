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
// match 60%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0040d4b0
int FUN_0040d4b0(int hundredths)
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

int FUN_0040ae90(void);
int FUN_0040aed0(void);
NetClassification *FUN_0040aea0(int index);
unsigned int FUN_0040aec0(int index);
BYTE *FUN_0040e8c0(void);

// Draws the stage-times screen: the two header bars, the three column titles,
// the split table of the drivers in the race and the leaderboard rows of the
// current stage.
// match 76%: MSVC6 allocates a 0x28 frame where the original has 0x2c: the
// original spills the leaderboard-row pointer (E-0x28) and our build keeps it in
// a register, so every local access shifts by 4 (all the other differences are
// that shift; the call sequence, constants and branches are identical).
// match 83%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004de1d0
void FUN_004de1d0(int unused)
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
    if (CGameInfo::FUN_00405d80() == 10)
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
    if (FUN_0040ae90() > 0) {
        do {
            pColour = (int *)g_colourWhite0x00524968;
            if (FUN_0040aed0() != i)
                pColour = (int *)g_colourText0x0052496c;
            Font_DrawText(0, (char *)FUN_0040aea0(i), (int)(g_pGraphics->resX * 0x96) / 0x280, (int)(g_pGraphics->resY * 0x14) / 0x1e0 * i + (int)(g_pGraphics->resY * 0x82) / 0x1e0, pColour, 10);
            if (FUN_0040aec0(i) == -1)
                sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x200));
            else
                FormatCentisecondsAsMinSecMSec(FUN_0040aec0(i), CFrontend::m_stringDest);
            Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0xfa) / 0x280, (int)(g_pGraphics->resY * 0x14) / 0x1e0 * i + (int)(g_pGraphics->resY * 0x82) / 0x1e0, pColour, 10);
            if (FUN_0040aec0(0) != -1 && FUN_0040aec0(i) == FUN_0040aec0(0)) {
                Font_DrawText(0, g_strNum1, (int)(g_pGraphics->resX * 0x15e) / 0x280, (int)(g_pGraphics->resY * 0x14) / 0x1e0 * i + (int)(g_pGraphics->resY * 0x82) / 0x1e0, pColour, 10);
            } else {
                Font_DrawText(0, g_str0x00519fb0, (int)(g_pGraphics->resX * 0x15e) / 0x280, (int)(g_pGraphics->resY * 0x14) / 0x1e0 * i + (int)(g_pGraphics->resY * 0x82) / 0x1e0, pColour, 10);
            }
            i++;
        } while (i < FUN_0040ae90());
    }
    for (i = FUN_0040ae90(); i < 8; i++) {
        Font_DrawText(0, g_str0x00519fb4, (int)(g_pGraphics->resX * 0x96) / 0x280, (int)(g_pGraphics->resY * 0x14) / 0x1e0 * i + (int)(g_pGraphics->resY * 0x82) / 0x1e0,
                      (int *)g_colourText0x0052496c, 10);
        Font_DrawText(0, CFrontend::GetTextString(0x200), (int)(g_pGraphics->resX * 0xfa) / 0x280, (int)(g_pGraphics->resY * 0x14) / 0x1e0 * i + (int)(g_pGraphics->resY * 0x82) / 0x1e0,
                      (int *)g_colourText0x0052496c, 10);
        Font_DrawText(0, g_str0x00519fb0, (int)(g_pGraphics->resX * 0x15e) / 0x280, (int)(g_pGraphics->resY * 0x14) / 0x1e0 * i + (int)(g_pGraphics->resY * 0x82) / 0x1e0,
                      (int *)g_colourText0x0052496c, 10);
    }

    pBoard = FUN_0040e8c0();
    Font_DrawText(1, CFrontend::GetTextString(0x201), (int)rectB[2] / 2 + *(int *)rectB,
                  (int)(g_pGraphics->resY * 100) / 0x1e0, (int *)g_colourText0x0052496c, 10);
    if (pBoard == NULL) {
        Font_DrawText(0, CFrontend::GetTextString(0x202), (int)rectB[2] / 2 + *(int *)rectB,
                      (int)(g_pGraphics->resY * 0x14) / 0x1e0 + (int)(g_pGraphics->resY * 0x82) / 0x1e0,
                      (int *)g_colourText0x0052496c, 10);
        return;
    }
    p = (char *)(pBoard + 4);
    for (i = 0; i < 10; i++, p += 8) {
        if (*p != '\0') {
            Font_DrawText(0, p, (int)(g_pGraphics->resX * 0x1c2) / 0x280, (int)(g_pGraphics->resY * 0x82) / 0x1e0 * (i + 1) + (int)(g_pGraphics->resY * 0x14) / 0x1e0,
                          (int *)g_colourText0x0052496c, 10);
            sprintf(CFrontend::m_stringDest, g_stageNumberFormat, *(int *)(p + 4));
            Font_DrawText(0, CFrontend::m_stringDest, (int)(g_pGraphics->resX * 0x1fe) / 0x280, (int)(g_pGraphics->resY * 0x82) / 0x1e0 * (i + 1) + (int)(g_pGraphics->resY * 0x14) / 0x1e0,
                          (int *)g_colourText0x0052496c, 10);
        } else {
            Font_DrawText(0, g_str0x00519fb4, (int)(g_pGraphics->resX * 0x1c2) / 0x280, (int)(g_pGraphics->resY * 0x82) / 0x1e0 * (i + 1) + (int)(g_pGraphics->resY * 0x14) / 0x1e0,
                          (int *)g_colourText0x0052496c, 10);
            Font_DrawText(0, g_str0x00519fb0, (int)(g_pGraphics->resX * 0x1fe) / 0x280, (int)(g_pGraphics->resY * 0x82) / 0x1e0 * (i + 1) + (int)(g_pGraphics->resY * 0x14) / 0x1e0,
                          (int *)g_colourText0x0052496c, 10);
        }
    }
}
