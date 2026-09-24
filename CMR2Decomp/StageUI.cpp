#include "NetworkLeaderboards.h"
#include "NetPlayers.h"
#include "Game.h"
#include "StageUI.h"
#include <cstdio>
#include <cstring>
#include "Frontend.h"
#include "GenericFileLoader.h"
#include "Font.h"
#include "Sprite.h"
#include "Graphics.h"
#include "RallyData.h"

// FUNCTION: CMR2 0x00415bc0
void FUN_00415bc0(int, int)
{
}

// FUNCTION: CMR2 0x00415f40
void FUN_00415f40(void)
{
}

// GLOBAL: CMR2 0x00517e14
char g_positiveSymbol[2] = "+";

// GLOBAL: CMR2 0x00517e18
char g_negativeSymbol[2] = "-";

// GLOBAL: CMR2 0x00537084
char g_emptyString[4];
// GLOBAL: CMR2 0x00517df4
char g_strGapUnknown[12] = "%s--:--.--";
// GLOBAL: CMR2 0x00517e00
char g_strGapTime[20] = "%s%02d:%02d.%02d";
// GLOBAL: CMR2 0x005170f4
BYTE g_gapTextColour[4] = { 0xff, 0xff, 0xff, 0xff };

// Formats a time gap as "+mm:ss.hh" and draws it, boxed when param_9 is set.
// FUNCTION: CMR2 0x00415bd0
void FormatGapToLeader(int iLeaderGap, unsigned int fontIndex, unsigned char param_3, int x, int y, void *pColour, unsigned int flags, char *pcNegPosSymbol, int drawBox)
{
    char text[16];
    short rect[4];
    int px;

    if (pcNegPosSymbol != NULL) {
        sprintf((char *)&pcNegPosSymbol, pcNegPosSymbol);
    } else {
        sprintf((char *)&pcNegPosSymbol, g_emptyString);
    }
    px = (int)g_pGraphics->resX * x >> 0x10;
    if (iLeaderGap != -1) {
        sprintf(text, g_strGapTime, &pcNegPosSymbol, (iLeaderGap / 6000) % 100,
                (iLeaderGap % 6000) / 100, (iLeaderGap % 6000) % 100);
    } else {
        sprintf(text, g_strGapUnknown, &pcNegPosSymbol);
    }
    if (drawBox != 0) {
        rect[0] = (short)((int)g_pGraphics->resX * x >> 0x10) -
                  (short)((int)g_pGraphics->resX / 0xa0) - 2;
        rect[2] = (short)((int)g_pGraphics->resX / 0x50) +
                  (short)Font_GetTextWidth(fontIndex, (BYTE *)text) + 4;
        rect[1] = (short)((int)g_pGraphics->resY * y >> 0x10) -
                  (short)((int)g_pGraphics->resY / 0xf0) - 2;
        rect[3] = (short)((int)g_pGraphics->resY / 0x140) +
                  (short)Font_GetTextHeight(fontIndex, text) + 4;
        Sprite_FillRect((int)g_pGraphics + 0x150, rect, (BYTE *)pColour, 2);
    }
    Font_DrawText(fontIndex, text, px, (int)g_pGraphics->resY * y >> 0x10,
                  (int *)g_gapTextColour, flags);
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

struct StageSoundPattern {
    char choices[4];
    char redirect;
    BYTE pad[3];
    int count;
    int base[4];
};

// Each sound state can redirect to a shared pattern. The base sound ids are
// filled in when the stage sound bank is loaded.
// GLOBAL: CMR2 0x00518ca8
StageSoundPattern g_stageSoundPatterns[31] = {
    {{1, 1, 0, 0}, 0, {0, 0, 0}, 1}, // 0
    {{1, 1, 0, 0}, 1, {0, 0, 0}, 1}, // 1
    {{3, 3, 0, 0}, 2, {0, 0, 0}, 1}, // 2
    {{1, 1, 0, 0}, 3, {0, 0, 0}, 1}, // 3
    {{3, 1, 0, 0}, 4, {0, 0, 0}, 1}, // 4
    {{0, 0, 0, 0}, 4, {0, 0, 0}, 1}, // 5
    {{5, 3, 1, 1}, 6, {0, 0, 0}, 2}, // 6
    {{0, 0, 0, 0}, 1, {0, 0, 0}, 1}, // 7
    {{0, 0, 0, 0}, 1, {0, 0, 0}, 1}, // 8
    {{0, 0, 0, 0}, 6, {0, 0, 0}, 1}, // 9
    {{0, 0, 0, 0}, 6, {0, 0, 0}, 1}, // 10
    {{0, 0, 0, 0}, 6, {0, 0, 0}, 1}, // 11
    {{0, 0, 0, 0}, 0, {0, 0, 0}, 1}, // 12
    {{3, 1, 1, 1}, 13, {0, 0, 0}, 2}, // 13
    {{1, 1, 0, 0}, 14, {0, 0, 0}, 1}, // 14
    {{0, 0, 0, 0}, 14, {0, 0, 0}, 1}, // 15
    {{0, 0, 0, 0}, 25, {0, 0, 0}, 1}, // 16
    {{1, 1, 1, 1}, 17, {0, 0, 0}, 2}, // 17
    {{1, 1, 1, 1}, 18, {0, 0, 0}, 2}, // 18
    {{1, 1, 0, 0}, 19, {0, 0, 0}, 1}, // 19
    {{0, 0, 0, 0}, 19, {0, 0, 0}, 1}, // 20
    {{0, 0, 0, 0}, 19, {0, 0, 0}, 1}, // 21
    {{3, 1, 1, 1}, 22, {0, 0, 0}, 2}, // 22
    {{0, 0, 0, 0}, 17, {0, 0, 0}, 1}, // 23
    {{1, 1, 4, 4}, 24, {0, 0, 0}, 4}, // 24
    {{2, 2, 4, 4}, 25, {0, 0, 0}, 3}, // 25
    {{0, 0, 0, 0}, 0, {0, 0, 0}, 1}, // 26
    {{0, 0, 0, 0}, 6, {0, 0, 0}, 1}, // 27
    {{0, 0, 0, 0}, 1, {0, 0, 0}, 1}, // 28
    {{1, 1, 4, 4}, 29, {0, 0, 0}, 4}, // 29
    {{0, 0, 0, 0}, 17, {0, 0, 0}, 1} // 30
};

extern BYTE g_unk0x005375f4[0x1d0];
void FUN_00418d30(int channel, int sound, int slot, int volume, int flags);
void FUN_00418dd0(int channel, int slot, char clear);
int FUN_00419b50(int exclude, int count);
BYTE FUN_00427aa0(void);

#define STAGE_PLAY_PRIMARY(slot) do { \
    unsigned int selected = (BYTE)g_unk0x005375f4[channel]; \
    FUN_00418d30(channel, pPattern->base[selected] + \
                 FUN_00419b50(-1, (int)pPattern->choices[selected]), slot, 0, 0); \
} while (0)
#define STAGE_PLAY_SECONDARY(slot) do { \
    unsigned int selected = (BYTE)g_unk0x005375f4[channel]; \
    FUN_00418d30(channel, pPattern->base[selected + 2] + \
                 FUN_00419b50(-1, (int)pPattern->choices[selected + 2]), slot, 0, 0); \
} while (0)
#define STAGE_PLAY_DIRECT(slot) do { \
    unsigned int selected = (BYTE)g_unk0x005375f4[channel]; \
    FUN_00418d30(channel, pPattern->base[selected], slot, 0, 0); \
} while (0)
#define STAGE_STOP(slot) FUN_00418dd0(channel, slot, 1)
#define STAGE_PLAY_EXTRA() do { STAGE_PLAY_SECONDARY(1); STAGE_PLAY_SECONDARY(2); STAGE_PLAY_SECONDARY(3); } while (0)
#define STAGE_STOP_EXTRA() do { STAGE_STOP(1); STAGE_STOP(2); STAGE_STOP(3); } while (0)

// Applies one stage sound state to its active channel.  The switch mirrors
// the combinations of continuous, secondary and network-gated sound slots.
// TODO: CMR2 0x00419200 (implemented, match 34%)
void StageUI_ApplySoundState(int channel, BYTE *pState)
{
    StageSoundPattern *pPattern = NULL;
    short pattern = *(short *)(pState + 0x18);
    int state = *(int *)(pState + 0xa8);

    if (pattern != -1)
        pPattern = &g_stageSoundPatterns[(int)g_stageSoundPatterns[pattern].redirect];

    switch (state) {
    case 1:
        if (pPattern != NULL) goto play_primary;
        return;
    case 2:
        if (pPattern != NULL) goto play_primary_and_seventh;
        return;
    case 3:
        if (pPattern != NULL) goto play_direct_and_secondary_extra;
        return;
    case 4:
        if (pPattern != NULL) goto play_primary_and_secondary_extra;
        return;
    case 5:
        STAGE_STOP(4);
        return;
    case 6:
        if (pPattern != NULL) {
            if (FUN_00427aa0() != 0) STAGE_STOP(4);
            goto play_primary;
        }
        return;
    case 7:
        if (pPattern != NULL) {
            if (FUN_00427aa0() != 0) STAGE_STOP(4);
            goto play_primary_and_seventh;
        }
        return;
    case 8:
        STAGE_STOP(4);
        if (pPattern == NULL) return;
        if (FUN_00427aa0() == 0) STAGE_PLAY_EXTRA();
        goto play_direct_and_secondary;
    case 9:
        STAGE_STOP(4);
        if (pPattern == NULL) return;
        if (FUN_00427aa0() == 0) STAGE_PLAY_EXTRA();
        goto play_primary_and_secondary;
    case 10:
        STAGE_STOP(4); STAGE_STOP(6);
        return;
    case 11:
        if (pPattern == NULL) return;
        if (FUN_00427aa0() != 0) { STAGE_STOP(4); STAGE_STOP(6); }
        goto play_primary;
    case 12:
        if (pPattern == NULL) return;
        if (FUN_00427aa0() != 0) { STAGE_STOP(4); STAGE_STOP(6); }
        goto play_primary_and_seventh;
    case 13:
        STAGE_STOP(4); STAGE_STOP(6);
        if (pPattern != NULL) goto play_direct_and_secondary_extra;
        return;
    case 14:
        STAGE_STOP(4); STAGE_STOP(6);
        if (pPattern != NULL) goto play_primary_and_secondary_extra;
        return;
    case 15:
    case 20:
        STAGE_STOP(4); STAGE_STOP(0);
        if (FUN_00427aa0() == 0) STAGE_STOP_EXTRA();
        return;
    case 16:
        STAGE_STOP(4); STAGE_STOP(0);
        if (FUN_00427aa0() == 0) STAGE_STOP_EXTRA();
        if (pPattern != NULL) goto play_primary;
        return;
    case 17:
        STAGE_STOP(4); STAGE_STOP(0);
        if (FUN_00427aa0() == 0) STAGE_STOP_EXTRA();
        if (pPattern != NULL) goto play_primary_and_seventh;
        return;
    case 19:
        STAGE_STOP(4);
        if (pPattern != NULL) goto play_primary;
        return;
    case 21:
        STAGE_STOP(0);
        if (FUN_00427aa0() == 0) STAGE_STOP_EXTRA();
        else STAGE_STOP(4);
        if (pPattern != NULL) goto play_primary;
        return;
    case 22:
        STAGE_STOP(4); STAGE_STOP(0);
        if (FUN_00427aa0() == 0) STAGE_STOP_EXTRA();
        if (pPattern != NULL) goto play_primary_and_seventh;
        return;
    case 23:
        STAGE_STOP(4);
        if (pPattern != NULL) goto play_direct;
        return;
    case 24:
        STAGE_STOP(4);
        if (pPattern != NULL) goto play_primary;
        return;
    }
    return;

play_primary_and_secondary_extra:
    STAGE_PLAY_PRIMARY(5);
    goto play_secondary_extra;
play_direct_and_secondary_extra:
    STAGE_PLAY_DIRECT(4);
play_secondary_extra:
    STAGE_PLAY_SECONDARY(0);
    if (FUN_00427aa0() == 0) STAGE_PLAY_EXTRA();
    return;
play_primary_and_secondary:
    STAGE_PLAY_PRIMARY(5);
    goto play_secondary;
play_direct_and_secondary:
    STAGE_PLAY_DIRECT(4);
play_secondary:
    STAGE_PLAY_SECONDARY(0);
    return;
play_primary_and_seventh:
    STAGE_PLAY_PRIMARY(5);
    STAGE_PLAY_SECONDARY(7);
    return;
play_primary:
    STAGE_PLAY_PRIMARY(5);
    return;
play_direct:
    STAGE_PLAY_DIRECT(4);
    return;
}

#undef STAGE_PLAY_PRIMARY
#undef STAGE_PLAY_SECONDARY
#undef STAGE_PLAY_DIRECT
#undef STAGE_STOP
#undef STAGE_PLAY_EXTRA
#undef STAGE_STOP_EXTRA


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

void FUN_0040ad20(void);
BOOL FUN_004a15a0(void);
unsigned int FUN_0040aec0(int index);
int FUN_0040ae90(void);
void FUN_0040af60(void);
struct NetClassification *FUN_0040aea0(int index);
void FUN_0040e660(int index, char *name, int wins);

// End of a network race: gives a leaderboard win to every driver with the
// winning time and registers the others with no win.
// TODO: CMR2 0x0041b3a0 (implemented, match 84%)
void FUN_0041b3a0(void)
{
    int i;

    FUN_0040ad20();
    if (CNetworkLeaderboards::GetLeaderboardId() == -1)
        return;
    if (FUN_004a15a0() && FUN_0040aec0(0) != -1) {
        FUN_0040e660(CNetworkLeaderboards::GetLeaderboardId(), FUN_0040aea0(0)->name, 1);
        for (i = 1; i < FUN_0040ae90(); i++)
            FUN_0040e660(CNetworkLeaderboards::GetLeaderboardId(), FUN_0040aea0(i)->name,
                         FUN_0040aec0(i) == FUN_0040aec0(0) ? 1 : 0);
    }
    for (i = 0; i < FUN_0040ae90(); i++)
        FUN_0040e660(CNetworkLeaderboards::GetLeaderboardId(), FUN_0040aea0(i)->name, 0);
    FUN_0040af60();
}


// GLOBAL: CMR2 0x0051c988
BYTE g_barBackColour[4] = { 0, 0, 0, 0 };
// GLOBAL: CMR2 0x0051c990
BYTE g_barTextColour[4] = { 0, 0, 0, 0 };
// GLOBAL: CMR2 0x0051c97c
BYTE *g_pUnk0x0051c97c;

// FUNCTION: CMR2 0x00475a40
BYTE *FUN_00475a40(void)
{
    return g_pUnk0x0051c97c;
}
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
