#ifndef _GAME_MENUS_H
#define _GAME_MENUS_H

#include "Menu.h"

extern Menu g_menu0x0053e2d8;
extern Menu g_menu0x0053e4b8;
extern Menu g_menu0x0053e6a0;
extern Menu g_menu0x0053e888;
extern Menu g_menu0x0053ea68;
extern Menu g_menu0x0053ec48;
extern Menu g_menu0x0053ee28;
extern Menu g_menu0x0053f008;
extern Menu g_menu0x0053f1e8;
extern Menu g_menu0x0053f3c8;
extern Menu g_menu0x0053f5b0;
extern Menu g_menu0x0053f790;
extern Menu g_menu0x0053f970;
extern Menu g_menu0x0053fb70;
extern Menu g_menu0x0053fd58;
extern Menu g_menu0x0053ff38;
extern Menu g_menu0x00540118;
extern Menu g_menu0x005402f8;
extern Menu g_menu0x005404d8;
extern Menu g_menu0x005406b8;
extern Menu g_menu0x005408a0;
extern Menu g_menu0x00540a80;
extern Menu g_menu0x00540c68;
extern Menu g_menu0x00540e50;
extern Menu g_menu0x00541030;
extern Menu g_menu0x00541218;
extern Menu g_menu0x00541400;
extern Menu g_menu0x005416e0;
extern Menu g_menu0x005418d8;
extern Menu g_menu0x00541ae0;

void GameMenus_Build(void);
void GameMenu_RequestResultsQuit(Menu *pMenu, int param);
void GameMenu_RequestNetworkResultsQuit(Menu *pMenu, int param);
void GameMenu_ContinueWithDefaultMode(Menu *pMenu, int param);
void GameMenu_SelectLastItem(Menu *pMenu, int param);
void GameMenu_UpdateStandingsScrollRange(Menu *pMenu, int param);
void GameMenu_UpdateRemainingRaceRange(Menu *pMenu, int param);
void GameMenu_OpenFirstItemSubmenu(Menu *pMenu, int param);
void GameMenu_RequestResultsRestart(Menu *pMenu, int param);
void GameMenu_RequestResultsContinue(Menu *pMenu, int param1);
void GameMenu_LeaveStageAndResetMenuFlag(Menu *pMenu, int param);
void GameMenu_RequestResultsRetry(Menu *pMenu, int param);
void GameMenu_ClearMenuListWithHighlight(void);
void GameMenu_DrawPauseHeader(Menu *pMenu);
void GameMenu_DrawWaitingForPlayers(Menu *pMenu);
void FUN_0044bcd0(Menu *pMenu);
void GameMenu_DrawStageResultsRows(Menu *pMenu);
void FUN_0044d790(Menu *pMenu);
void FUN_0044d960(Menu *pMenu);
void GameMenu_DrawStageTimeStandings(Menu *pMenu);
void GameMenu_DrawChampionshipClassResults(Menu *pMenu);
void FUN_0044efa0(Menu *pMenu);
void GameMenu_DrawStageStartList(Menu *pMenu);
void FUN_0044fea0(Menu *pMenu);
void GameMenu_DrawChampionshipTimeStandings(Menu *pMenu);
void FUN_00450c10(Menu *pMenu);
void GameMenu_NoOpNetworkRallyEndDraw(Menu *pMenu);
void FUN_00450ef0(Menu *pMenu);
void FUN_00451690(Menu *pMenu);
void FUN_00451df0(Menu *pMenu);
void GameMenu_DrawChampionshipDriverStandings(Menu *pMenu);
void GameMenu_DrawKnockoutMatchResult(Menu *pMenu);
void FUN_00452be0(Menu *pMenu);
void GameMenu_DrawRallyPointsTable(Menu *pMenu);
void GameMenu_DrawRallyResultsBanner(Menu *pMenu);
void GameMenu_DrawStandingsMenuItems(Menu *pMenu);

#endif
