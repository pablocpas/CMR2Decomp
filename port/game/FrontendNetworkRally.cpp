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

void FrontendNetwork_DrainMessageQueue(void);

extern char g_stageNumberFormat[];

extern char g_strFlagSS[4];

extern char g_unk0x00818d84[0x24];

// Clamps the stage-selection item of a network setup menu to the number of
// stages available for the current car, and builds the "1..N SS" label.
// FUNCTION: CMR2 0x004ee6e0
void FrontendMenu_UpdateNetworkRallySetup(Menu *pMenu)
{
    unsigned int *pFlags;
    unsigned int mask;
    int max;
    int count;
    int i;
    char *pDest;

    pFlags = CGameInfo::GetGameInfoField9CAddress();
    max = Menu_GetItem(pMenu, 0)->max;
    if (CGameInfo::IsRecordFlagSet(0xd) == 0) {
        count = 1;
        if ((int)(*pFlags >> 8 & 0xf) >= max + 1)
            count = 4;
        if ((int)(*pFlags >> 0xc & 0xf) >= max + 1)
            count = 8;
        if ((*pFlags & 1) != 0 && (int)(*pFlags >> 0x10 & 0xf) >= max + 1)
            count = 10;
    } else {
        count = 10;
    }
    i = 0;
    if (0 < count) {
        pDest = g_unk0x00818d84;
        do {
            sprintf(pDest, g_stageNumberFormat, i + 1);
            pDest += 3;
            i++;
        } while (i < count);
    }
    sprintf(&g_unk0x00818d84[i * 3], g_strFlagSS);
    if (CGameInfo::IsRecordFlagSet(0xd) != 0) {
        count = ((max % 2) != 0) + 10;
    } else {
        if ((max % 2) != 0) {
            mask = 1 << ((max + 1) / 2 - 1);
            if ((pFlags[1] & mask & 0x1f) != 0 ||
                ((short)((pFlags[1] >> 5) & 0x1f) & mask) != 0 ||
                ((short)((pFlags[1] >> 10) & 0x1f) & mask) != 0)
                count++;
        }
    }
    Menu_GetItem(pMenu, 1)->min = (char)count;
    if (Menu_GetItem(pMenu, 1)->max >= Menu_GetItem(pMenu, 1)->min)
        Menu_GetItem(pMenu, 1)->max = Menu_GetItem(pMenu, 1)->min - 1;
    FrontendNetwork_DrainMessageQueue();
}
