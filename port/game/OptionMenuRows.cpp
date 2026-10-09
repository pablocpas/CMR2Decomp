// Option menu record rows. A separate translation unit: the original builds
// this function without floating-point code earlier in its object file, which
// MSVC6 lets change register allocation in later functions (see CLAUDE.md).
// The declarations below are the ones it uses from GameInfo.cpp.
#include "GameInfo.h"
#include "Menu.h"
#include "Graphics.h"
#include "Input.h"
#include "Frontend.h"
#include "RegKey.h"
#include "InstallInfo.h"
#include "FileBuffer.h"
#include "GenericFileLoader.h"
#include "main.h"
#include "Font.h"
#include "FixedPoint.h"
#include "Game.h"
#include "Sound.h"
#include "RallyData.h"
#include "NetPlayers.h"
#include <stdio.h>
#include <string.h>
#include "Sprite.h"
#include "Texture.h"
#include "StageTiming.h"

int OptionMenu_GetColumnLabelId(int index);

int OptionMenu_GetColumnUnitId(int index);

int OptionMenu_GetColumnWeight(int index);

int OptionMenu_GetRecordTransitionMode(int index);

BYTE OptionMenu_GetRecordGroupAppliedFlag(int i, int j);

void OptionMenu_DrawTransitionTextShortCoords(int index, int font1, int font2, char *text, short x, short y, int *pColour1,
                  int *pColour2, unsigned int flags);

extern char g_classRowHeaderFormat[];

void OptionMenu_EaseRecordBar(int index, short *pBar, int direction, int unused);

void OptionMenu_DrawTransitionText(int index, int font1, int font2, char *text, short x, short y,
                  int *pColour1, int *pColour2, unsigned int flags);

unsigned int OptionMenu_GetRecordPercentage(int index, int type, int dynamic);

void OptionMenu_ScaleRecordBar(int index, short *pBar);

BYTE *OptionMenu_GetStateByteAddress(void);

int OptionMenu_IsSlotEnabled(int mode, int index);

int OptionMenu_IsSlotValueAboveBase(int index, int type);

BYTE OptionMenu_IsValueDefault(int index, int type);

int OptionMenu_IsRecordPercentageAboveBase(int index, int type);

extern int g_unk0x00527380[3];

extern int g_unk0x0052738c[3];

extern int g_unk0x00831360;

extern int g_unk0x00831364;

extern int g_unk0x00831368;

extern int g_unk0x0083166c;

extern int g_unk0x00831670;

extern int g_unk0x008313ac;

extern int g_unk0x00831648;

extern int g_unk0x008313b0;

extern int g_unk0x00527378;

extern int g_unk0x0052737c;

extern short g_unk0x00831660[4];

extern BYTE g_colour0x005273a8[8];

extern char g_keypadFormat[];

extern char g_strLabelText[];

BYTE RallyData_GetDriverGridRow(BYTE param1, char param2);

extern BYTE g_unk0x0051682c[132];

extern int g_unk0x00527398;

extern char g_unk0x0082aa44[0x100];

extern char g_str0x005295f8[];

extern char g_str0x00529600[];

// Draws one frame of a game-info screen (options, about, rally): the background
// bar of every row, the text of the row chosen by the screen mode, the row
// marker and banner sprites and the pulsing sprite of the highlighted row.
// match 72%: faithful reconstruction of every block (per-mode text, colours, sprite
// queues, bar easing); the residual diff is MSVC6 register allocation and block
// placement the compiler will not reproduce from source: it keeps the param5 load in
// EDI across the three OptionMenu_ScaleRecordBar calls, merges the Sprite_Queue/colour-select tails
// at different points (push offset vs mov+push) and lays the switch bodies out with a
// different instruction schedule. OptionMenu_GetRecordPercentage is declared unsigned in the tree
// while the original compares it signed; the call site casts to int for jg.
// OptionMenu_IsValueDefault returns BYTE, matching the original test al.
// FUNCTION: CMR2 0x0050b1c0
void OptionMenu_DrawAnimatedScreenRows(short param1, short param2, short param3, short param4, short param5, int param6)
{
    short rectP[4];
    short rectQ[4];
    short rectR[4];
    int barX;
    int rowY;
    int flag;
    int i;

    rectQ[0] = (short)((int)param4 * (int)g_pGraphics->resX / 0x280);
    rectQ[1] = 0;
    rectQ[2] = 0;
    rectQ[3] = 0;
    rectR[0] = (short)((int)g_pGraphics->resX * 0x23a / 0x280);
    rectR[1] = 0;
    rectR[2] = 0;
    rectR[3] = 0;
    rectP[0] = (short)((int)g_pGraphics->resX * 0x254 / 0x280);
    rectP[1] = 0;
    rectP[2] = 0;
    rectP[3] = 0;

    if (param6 != 0)
        param6 = g_unk0x0052737c;
    else
        param6 = *(int *)OptionMenu_GetStateByteAddress();

    barX = (int)param4 * (int)g_pGraphics->resX / 0x280;
    rowY = param5;

    if (g_unk0x0083166c != 0) {
        rectQ[2] = *(short *)(g_unk0x0083166c + 0x120);
        rectQ[3] = *(short *)(g_unk0x0083166c + 0x122);
        OptionMenu_ScaleRecordBar(1, rectQ);
    }
    if (g_unk0x00831360 != 0) {
        rectR[2] = *(short *)(g_unk0x00831360 + 0x120);
        rectR[3] = *(short *)(g_unk0x00831360 + 0x122);
        OptionMenu_ScaleRecordBar(1, rectR);
    }
    if (g_unk0x008313ac != 0) {
        rectP[2] = *(short *)(g_unk0x008313ac + 0x120);
        rectP[3] = *(short *)(g_unk0x008313ac + 0x122);
        OptionMenu_ScaleRecordBar(1, rectP);
    }

    g_unk0x00831660[0] = (short)barX;
    g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * rowY / 0x1e0);
    g_unk0x00831660[2] = (short)((int)g_pGraphics->resX * 0xa2 / 0x280);
    g_unk0x00831660[3] = 1;
    if (param3 == 4)
        g_unk0x00831660[2] += (short)((int)g_pGraphics->resX * 200 / 0x280);

    OptionMenu_EaseRecordBar(0, g_unk0x00831660, 1, 1);

    if (param2 == 0) {
                if (CGameInfo::IsOptionMenuTimeoutPulseOn() != 0)
Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660, (BYTE *)&g_unk0x00527378, 1);
        else
            Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660, (BYTE *)&param6, 1);
    } else {
        Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660, (BYTE *)&g_unk0x00527380[2], 1);
    }

    for (i = 0; i < param1; i++) {
        if (g_unk0x0083166c != 0) {
            if (CGameInfo::GetScreenWidth() >= 0x400 && CFrontend::IsTextureWidthSupported(0x400) != 0 &&
                CFrontend::IsTextureHeightSupported(0x400) != 0) {
                rectQ[1] = (short)((int)g_pGraphics->resY * 0x18 / 0x1e0 / 2 + g_unk0x00831660[1] - 0xa);
                rectP[1] = (short)((int)g_pGraphics->resY * 0x18 / 0x1e0 / 2 + g_unk0x00831660[1] - 0xe);
                rectR[1] = (short)((int)g_pGraphics->resY * 0x18 / 0x1e0 / 2 + g_unk0x00831660[1] - 0xd);
            } else {
                rectQ[1] = (short)((int)g_pGraphics->resY * 0x18 / 0x1e0 / 2 + g_unk0x00831660[1] - 6);
                rectP[1] = (short)((int)g_pGraphics->resY * 0x18 / 0x1e0 / 2 + g_unk0x00831660[1] - 9);
                rectR[1] = (short)((int)g_pGraphics->resY * 0x18 / 0x1e0 / 2 + g_unk0x00831660[1] - 8);
            }
        }
        flag = 0;
        switch (param3) {
        case 1:
            sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue,
                    CFrontend::GetTextString(OptionMenu_GetColumnWeight(i)));
            flag = OptionMenu_IsSlotEnabled(OptionMenu_GetColumnWeight(i),
                                (int)CFrontend::GetArchivePrimaryIDEntry(
                                    RallyData_GetDriverRecordSelectionValue(CGameInfo::GetActiveOptionSlot()) & 0xff)) == 0;
            if ((int)OptionMenu_GetRecordPercentage(CGameInfo::GetActiveOptionSlot(), OptionMenu_GetColumnWeight(i), 0) > 0 ||
                OptionMenu_GetRecordGroupAppliedFlag(CGameInfo::GetActiveOptionSlot(), OptionMenu_GetColumnWeight(i)) != 0) {
                if (i != param2) {                    Sprite_Queue((SpriteRect *)(g_unk0x008313b0 + 0x11c), (SpriteRect *)rectP,
                                 (Texture *)g_unk0x008313b0, 1, 0, NULL, NULL,
                                 (BYTE *)&g_unk0x00527380[1], 8);
                            } else {                                        if (CGameInfo::IsOptionMenuTimeoutPulseOn() != 0)
Sprite_Queue((SpriteRect *)(g_unk0x008313b0 + 0x11c), (SpriteRect *)rectP,
                                     (Texture *)g_unk0x008313b0, 1, 0, NULL, NULL,
                                     (BYTE *)&g_unk0x00527378, 8);
                    else
                        Sprite_Queue((SpriteRect *)(g_unk0x008313b0 + 0x11c), (SpriteRect *)rectP,
                                     (Texture *)g_unk0x008313b0, 1, 0, NULL, NULL, (BYTE *)&param6, 8);
                            }
                if (OptionMenu_GetRecordGroupAppliedFlag(CGameInfo::GetActiveOptionSlot(), OptionMenu_GetColumnWeight(i)) != 0) {
                    if (i != param2) {                        Sprite_Queue((SpriteRect *)(g_unk0x008313ac + 0x11c), (SpriteRect *)rectP,
                                     (Texture *)g_unk0x008313ac, 1, 0, NULL, NULL,
                                     (BYTE *)&g_unk0x00527380[1], 8);
                                } else {                                                if (CGameInfo::IsOptionMenuTimeoutPulseOn() != 0)
Sprite_Queue((SpriteRect *)(g_unk0x008313ac + 0x11c), (SpriteRect *)rectP,
                                         (Texture *)g_unk0x008313ac, 1, 0, NULL, NULL,
                                         (BYTE *)&g_unk0x00527378, 8);
                        else
                            Sprite_Queue((SpriteRect *)(g_unk0x008313ac + 0x11c), (SpriteRect *)rectP,
                                         (Texture *)g_unk0x008313ac, 1, 0, NULL, NULL, (BYTE *)&param6, 8);
                                }
                } else if (OptionMenu_IsRecordPercentageAboveBase(CGameInfo::GetActiveOptionSlot(), OptionMenu_GetColumnWeight(i)) == 0) {
                    if (i != param2) {                        Sprite_Queue((SpriteRect *)(g_unk0x00831648 + 0x11c), (SpriteRect *)rectP,
                                     (Texture *)g_unk0x00831648, 1, 0, NULL, NULL,
                                     (BYTE *)&g_unk0x00527380[1], 8);
                                } else {                                                if (CGameInfo::IsOptionMenuTimeoutPulseOn() != 0)
Sprite_Queue((SpriteRect *)(g_unk0x00831648 + 0x11c), (SpriteRect *)rectP,
                                         (Texture *)g_unk0x00831648, 1, 0, NULL, NULL,
                                         (BYTE *)&g_unk0x00527378, 8);
                        else
                            Sprite_Queue((SpriteRect *)(g_unk0x00831648 + 0x11c), (SpriteRect *)rectP,
                                         (Texture *)g_unk0x00831648, 1, 0, NULL, NULL, (BYTE *)&param6, 8);
                                }
                }
            }
            if ((int)OptionMenu_GetRecordPercentage(CGameInfo::GetActiveOptionSlot(), OptionMenu_GetColumnWeight(i), 0) > 0x42) {
                Sprite_Queue((SpriteRect *)(g_unk0x00831360 + 0x11c), (SpriteRect *)rectR,
                             (Texture *)g_unk0x00831360, 1, 0, NULL, NULL, (BYTE *)&g_colour0x005273a8, 8);
            } else if ((int)OptionMenu_GetRecordPercentage(CGameInfo::GetActiveOptionSlot(), OptionMenu_GetColumnWeight(i), 0) > 0x21) {
                Sprite_Queue((SpriteRect *)(g_unk0x00831360 + 0x11c), (SpriteRect *)rectR,
                             (Texture *)g_unk0x00831364, 1, 0, NULL, NULL, (BYTE *)&g_colour0x005273a8, 8);
            } else if ((int)OptionMenu_GetRecordPercentage(CGameInfo::GetActiveOptionSlot(), OptionMenu_GetColumnWeight(i), 0) > 0) {
                Sprite_Queue((SpriteRect *)(g_unk0x00831360 + 0x11c), (SpriteRect *)rectR,
                             (Texture *)g_unk0x00831368, 1, 0, NULL, NULL, (BYTE *)&g_colour0x005273a8, 8);
            }
            break;
        case 0:
            if ((CGameInfo::GetConfiguredGameMode() == 0 || CGameInfo::GetConfiguredGameMode() == 1) &&
                (RallyDataStageIndex() & 0xff) + i == 10) {
                sprintf(CFrontend::m_stringDest, g_str0x00529600, CFrontend::GetTextString(0x9b),
                        RallyData_GetDriverGridRow((BYTE)RallyDataCountryIndex(), (char)CGameInfo::GetConfiguredDifficulty()),
                        CFrontend::GetTextString(0x13d));
            } else if ((RallyDataStageIndex() & 0xff) + i == 10) {
                sprintf(CFrontend::m_stringDest, g_keypadFormat, CFrontend::GetTextString(0x9b),
                        CFrontend::GetTextString(0x13d));
            } else {
                sprintf(CFrontend::m_stringDest, g_classRowHeaderFormat, CFrontend::GetTextString(0x9b),
                        (RallyDataStageIndex() & 0xff) + i + 1);
            }
            break;
        case 3:
            sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue,
                    CFrontend::GetTextString(i + 0x100));
            break;
        case 4:
            if (i == 0) {
                sprintf(CFrontend::m_stringDest, CFrontend::GetTextString(0x141));
            } else if (CMain::GetFrameDelta() % 0x14 > 9 && param2 == i) {
                sprintf(CFrontend::m_stringDest, g_str0x005295f8, CFrontend::GetTextString(0x142),
                        g_unk0x0082aa44);
            } else {
                sprintf(CFrontend::m_stringDest, g_strLabelText, CFrontend::GetTextString(0x142),
                        g_unk0x0082aa44);
            }
            break;
        case 2:
            sprintf(CFrontend::m_stringDest, CRegKey::m_regKeyPathFormatValue,
                    CFrontend::GetTextString(i + 0xa6));
            flag = OptionMenu_IsSlotEnabled(OptionMenu_GetColumnLabelId(i),
                                (int)CFrontend::GetArchivePrimaryIDEntry(
                                    RallyData_GetDriverRecordSelectionValue(CGameInfo::GetActiveOptionSlot()) & 0xff)) == 0;
            if (!flag) {
                if (i != param2) {                    Sprite_Queue((SpriteRect *)(g_unk0x008313b0 + 0x11c), (SpriteRect *)rectP,
                                 (Texture *)g_unk0x008313b0, 1, 0, NULL, NULL,
                                 (BYTE *)&g_unk0x00527380[1], 8);
                            } else {                                        if (CGameInfo::IsOptionMenuTimeoutPulseOn() != 0)
Sprite_Queue((SpriteRect *)(g_unk0x008313b0 + 0x11c), (SpriteRect *)rectP,
                                     (Texture *)g_unk0x008313b0, 1, 0, NULL, NULL,
                                     (BYTE *)&g_unk0x00527378, 8);
                    else
                        Sprite_Queue((SpriteRect *)(g_unk0x008313b0 + 0x11c), (SpriteRect *)rectP,
                                     (Texture *)g_unk0x008313b0, 1, 0, NULL, NULL, (BYTE *)&param6, 8);
                            }
                if ((BYTE)OptionMenu_IsValueDefault(CGameInfo::GetActiveOptionSlot(), i) == 0) {
                    if (i != param2) {                        Sprite_Queue((SpriteRect *)(g_unk0x008313ac + 0x11c), (SpriteRect *)rectP,
                                     (Texture *)g_unk0x008313ac, 1, 0, NULL, NULL,
                                     (BYTE *)&g_unk0x00527380[1], 8);
                                } else {                                                if (CGameInfo::IsOptionMenuTimeoutPulseOn() != 0)
Sprite_Queue((SpriteRect *)(g_unk0x008313ac + 0x11c), (SpriteRect *)rectP,
                                         (Texture *)g_unk0x008313ac, 1, 0, NULL, NULL,
                                         (BYTE *)&g_unk0x00527378, 8);
                        else
                            Sprite_Queue((SpriteRect *)(g_unk0x008313ac + 0x11c), (SpriteRect *)rectP,
                                         (Texture *)g_unk0x008313ac, 1, 0, NULL, NULL, (BYTE *)&param6, 8);
                                }
                } else if (OptionMenu_IsSlotValueAboveBase(CGameInfo::GetActiveOptionSlot(), OptionMenu_GetColumnUnitId(i)) == 0) {
                    if (i != param2) {                        Sprite_Queue((SpriteRect *)(g_unk0x00831648 + 0x11c), (SpriteRect *)rectP,
                                     (Texture *)g_unk0x00831648, 1, 0, NULL, NULL,
                                     (BYTE *)&g_unk0x00527380[1], 8);
                                } else {                                                if (CGameInfo::IsOptionMenuTimeoutPulseOn() != 0)
Sprite_Queue((SpriteRect *)(g_unk0x00831648 + 0x11c), (SpriteRect *)rectP,
                                         (Texture *)g_unk0x00831648, 1, 0, NULL, NULL,
                                         (BYTE *)&g_unk0x00527378, 8);
                        else
                            Sprite_Queue((SpriteRect *)(g_unk0x00831648 + 0x11c), (SpriteRect *)rectP,
                                         (Texture *)g_unk0x00831648, 1, 0, NULL, NULL, (BYTE *)&param6, 8);
                                }
                }
            }
            break;
        default:
            sprintf(CFrontend::m_stringDest, ((char *)&g_unk0x0051682c[0x44]));
            break;
        }
        if (i == param2) {
            if (CGameInfo::IsOptionMenuTimeoutPulseOn() != 0) {
                                if (OptionMenu_GetRecordTransitionMode(3) != 2)
OptionMenu_DrawTransitionTextShortCoords(3, 0, 0, CFrontend::m_stringDest,
                                 (int)g_pGraphics->resX * 0x14 / 0x280 +
                                     (int)g_pGraphics->resX * param4 / 0x280,
                                 (int)g_pGraphics->resY * 0x12 / 0x1e0 + g_unk0x00831660[1],
                                 &g_unk0x00527378, &g_unk0x00527378, 0x11);
                else
OptionMenu_DrawTransitionText(7, 0, 0, CFrontend::m_stringDest,
                                 (int)g_pGraphics->resX * 0x14 / 0x280 +
                                     (int)g_pGraphics->resX * param4 / 0x280,
                                 (int)g_pGraphics->resY * 0x12 / 0x1e0 + g_unk0x00831660[1],
                                 &g_unk0x00527378, g_unk0x00527380, 0x11);
                if (g_unk0x0083166c != 0)
                    Sprite_Queue((SpriteRect *)(g_unk0x0083166c + 0x11c), (SpriteRect *)rectQ,
                                 (Texture *)g_unk0x0083166c, 1, 0, NULL, NULL,
                                 (BYTE *)&g_unk0x00527378, 8);
            } else {
                                if (OptionMenu_GetRecordTransitionMode(3) != 2)
OptionMenu_DrawTransitionTextShortCoords(3, 0, 0, CFrontend::m_stringDest,
                                 (int)g_pGraphics->resX * 0x14 / 0x280 +
                                     (int)g_pGraphics->resX * param4 / 0x280,
                                 (int)g_pGraphics->resY * 0x12 / 0x1e0 + g_unk0x00831660[1],
                                 &param6, &g_unk0x0052738c[1], 0x11);
                else
OptionMenu_DrawTransitionText(7, 0, 0, CFrontend::m_stringDest,
                                 (int)g_pGraphics->resX * 0x14 / 0x280 +
                                     (int)g_pGraphics->resX * param4 / 0x280,
                                 (int)g_pGraphics->resY * 0x12 / 0x1e0 + g_unk0x00831660[1],
                                 &param6, g_unk0x00527380, 0x11);
                if (g_unk0x0083166c != 0)
                    Sprite_Queue((SpriteRect *)(g_unk0x0083166c + 0x11c), (SpriteRect *)rectQ,
                                 (Texture *)g_unk0x0083166c, 1, 0, NULL, NULL, (BYTE *)&param6, 8);
            }
        } else {
            if (flag != 0) {
                OptionMenu_DrawTransitionTextShortCoords(3, 0, 0, CFrontend::m_stringDest,
                             (int)g_pGraphics->resX * 0x14 / 0x280 +
                                 (int)g_pGraphics->resX * param4 / 0x280,
                             (int)g_pGraphics->resY * 0x12 / 0x1e0 + g_unk0x00831660[1],
                             &g_unk0x00527398, &g_unk0x0052738c[2], 0x11);
                if (g_unk0x0083166c != 0)
                    Sprite_Queue((SpriteRect *)(g_unk0x0083166c + 0x11c), (SpriteRect *)rectQ,
                                 (Texture *)g_unk0x0083166c, 1, 0, NULL, NULL,
                                 (BYTE *)&g_unk0x00527398, 8);
            } else {
                OptionMenu_DrawTransitionTextShortCoords(3, 0, 0, CFrontend::m_stringDest,
                             (int)g_pGraphics->resX * 0x14 / 0x280 +
                                 (int)g_pGraphics->resX * param4 / 0x280,
                             (int)g_pGraphics->resY * 0x12 / 0x1e0 + g_unk0x00831660[1],
                             g_unk0x00527380, g_unk0x0052738c, 0x11);
                if (g_unk0x00831670 != 0)
                    Sprite_Queue((SpriteRect *)(g_unk0x00831670 + 0x11c), (SpriteRect *)rectQ,
                                 (Texture *)g_unk0x00831670, 1, 0, NULL, NULL,
                                 (BYTE *)g_unk0x00527380, 8);
            }
        }
        g_unk0x00831660[1] = (short)((int)g_pGraphics->resY * param5 / 0x1e0 +
                                     (int)g_pGraphics->resY * 0x18 / 0x1e0 * (i + 1));
        if (i == param2 || i + 1 == param2) {
            if (CGameInfo::IsOptionMenuTimeoutPulseOn() != 0)
                Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660, (BYTE *)&g_unk0x00527378, 1);
            else
                Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660, (BYTE *)&param6, 1);
        } else {
            Sprite_FillRect((int)g_pGraphics + 0x150, g_unk0x00831660, (BYTE *)&g_unk0x00527380[2], 1);
        }
    }
}
