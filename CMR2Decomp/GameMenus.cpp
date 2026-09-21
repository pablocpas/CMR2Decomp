#include <windows.h>
#include <mmsystem.h>
#include "GameMenus.h"
#include "GameInfo.h"
#include "Frontend.h"
#include "Font.h"

// GLOBAL: CMR2 0x0053e2d8
Menu g_menu0x0053e2d8;
// GLOBAL: CMR2 0x0053e4b8
Menu g_menu0x0053e4b8;
// GLOBAL: CMR2 0x0053e6a0
Menu g_menu0x0053e6a0;
// GLOBAL: CMR2 0x0053e888
Menu g_menu0x0053e888;
// GLOBAL: CMR2 0x0053ea68
Menu g_menu0x0053ea68;
// GLOBAL: CMR2 0x0053ec48
Menu g_menu0x0053ec48;
// GLOBAL: CMR2 0x0053ee28
Menu g_menu0x0053ee28;
// GLOBAL: CMR2 0x0053f008
Menu g_menu0x0053f008;
// GLOBAL: CMR2 0x0053f1e8
Menu g_menu0x0053f1e8;
// GLOBAL: CMR2 0x0053f3c8
Menu g_menu0x0053f3c8;
// GLOBAL: CMR2 0x0053f5b0
Menu g_menu0x0053f5b0;
// GLOBAL: CMR2 0x0053f790
Menu g_menu0x0053f790;
// GLOBAL: CMR2 0x0053f970
Menu g_menu0x0053f970;
// GLOBAL: CMR2 0x0053fb70
Menu g_menu0x0053fb70;
// GLOBAL: CMR2 0x0053fd58
Menu g_menu0x0053fd58;
// GLOBAL: CMR2 0x0053ff38
Menu g_menu0x0053ff38;
// GLOBAL: CMR2 0x00540118
Menu g_menu0x00540118;
// GLOBAL: CMR2 0x005402f8
Menu g_menu0x005402f8;
// GLOBAL: CMR2 0x005404d8
Menu g_menu0x005404d8;
// GLOBAL: CMR2 0x005406b8
Menu g_menu0x005406b8;
// GLOBAL: CMR2 0x005408a0
Menu g_menu0x005408a0;
// GLOBAL: CMR2 0x00540a80
Menu g_menu0x00540a80;
// GLOBAL: CMR2 0x00540c68
Menu g_menu0x00540c68;
// GLOBAL: CMR2 0x00540e50
Menu g_menu0x00540e50;
// GLOBAL: CMR2 0x00541030
Menu g_menu0x00541030;
// GLOBAL: CMR2 0x00541218
Menu g_menu0x00541218;
// GLOBAL: CMR2 0x00541400
Menu g_menu0x00541400;
// GLOBAL: CMR2 0x005416e0
Menu g_menu0x005416e0;
// GLOBAL: CMR2 0x005418d8
Menu g_menu0x005418d8;
// GLOBAL: CMR2 0x00541ae0
Menu g_menu0x00541ae0;
// GLOBAL: CMR2 0x0053fd50
DWORD g_menuBuildTime;

// STUB: CMR2 0x00449b00
void FUN_00449b00(void)
{
}

// STUB: CMR2 0x00449b60
void FUN_00449b60(void)
{
}

// FUNCTION: CMR2 0x00449b80
void FUN_00449b80(Menu *pMenu, int param)
{
    FUN_0044a000(pMenu, 0);
}

// FUNCTION: CMR2 0x00449b90
void FUN_00449b90(Menu *pMenu, int param)
{
    pMenu->cursor = pMenu->itemCount - 1;
}

// STUB: CMR2 0x00449ba0
void FUN_00449ba0(Menu *pMenu)
{
}

// STUB: CMR2 0x00449ca0
void FUN_00449ca0(Menu *pMenu)
{
}

// FUNCTION: CMR2 0x00449e90
void FUN_00449e90(Menu *pMenu, int param)
{
    Menu_SetNextAction((int)pMenu->items[0].pSubMenu);
}

// STUB: CMR2 0x00449f30
void FUN_00449f30(void)
{
}

// STUB: CMR2 0x0044a000
void FUN_0044a000(Menu *pMenu, int param1)
{
}

// STUB: CMR2 0x0041f2b0
void FUN_0041f2b0(void)
{
}

// FUNCTION: CMR2 0x0044a090
void FUN_0044a090(Menu *pMenu, int param)
{
    FUN_0041f2b0();
    CGameInfo::FUN_0049ea90(0);
}

// STUB: CMR2 0x0044a0a0
void FUN_0044a0a0(void)
{
}

// STUB: CMR2 0x0044b7b0
void FUN_0044b7b0(void)
{
}

// STUB: CMR2 0x0044bc30
void FUN_0044bc30(void)
{
}

// STUB: CMR2 0x0044bcd0
void FUN_0044bcd0(Menu *pMenu)
{
}

// STUB: CMR2 0x0044d260
void FUN_0044d260(void)
{
}

// STUB: CMR2 0x0044d790
void FUN_0044d790(void)
{
}

// STUB: CMR2 0x0044d960
void FUN_0044d960(void)
{
}

// STUB: CMR2 0x0044e130
void FUN_0044e130(void)
{
}

// STUB: CMR2 0x0044e830
void FUN_0044e830(void)
{
}

// STUB: CMR2 0x0044efa0
void FUN_0044efa0(Menu *pMenu)
{
}

// STUB: CMR2 0x0044f8a0
void FUN_0044f8a0(void)
{
}

// STUB: CMR2 0x0044fea0
void FUN_0044fea0(Menu *pMenu)
{
}

// STUB: CMR2 0x004505b0
void FUN_004505b0(void)
{
}

// STUB: CMR2 0x00450c10
void FUN_00450c10(void)
{
}

// STUB: CMR2 0x00450ed0
void FUN_00450ed0(void)
{
}

// STUB: CMR2 0x00450ef0
void FUN_00450ef0(Menu *pMenu)
{
}

// STUB: CMR2 0x00451690
void FUN_00451690(void)
{
}

// STUB: CMR2 0x00451df0
void FUN_00451df0(Menu *pMenu)
{
}

// STUB: CMR2 0x00452430
void FUN_00452430(Menu *pMenu)
{
}

// STUB: CMR2 0x004529c0
void FUN_004529c0(void)
{
}

// STUB: CMR2 0x00452be0
void FUN_00452be0(void)
{
}

// STUB: CMR2 0x004530e0
void FUN_004530e0(void)
{
}

// STUB: CMR2 0x00453830
void FUN_00453830(void)
{
}

// STUB: CMR2 0x00453c50
void FUN_00453c50(Menu *pMenu)
{
}

// Builds the in-game menu tree (pause / results / options screens).
// FUNCTION: CMR2 0x00449100
void GameMenus_Build(void)
{
    g_menuBuildTime = timeGetTime();
    Menu_Init(&g_menu0x0053ea68, 0, 0, 0, NULL, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x0053ea68, 0, 0, &g_menu0x00541218, NULL, 0);
    Menu_SetCallbacks(&g_menu0x0053ea68, NULL, NULL, (MenuCallback)FUN_0044b7b0, NULL);
    Menu_ValidateCursor(&g_menu0x0053ea68, 0);
    Menu_Init(&g_menu0x0053ff38, 0, 0, 0, NULL, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x0053ff38, 0, 0, &g_menu0x0053ea68, NULL, 0);
    Menu_SetCallbacks(&g_menu0x0053ff38, NULL, NULL, (MenuCallback)FUN_0044bc30, NULL);
    Menu_ValidateCursor(&g_menu0x0053ff38, 0);
    Menu_Init(&g_menu0x00541218, 0, 0, 0, &g_menu0x0053ea68, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x00541218, 0, 0, &g_menu0x0053f5b0, NULL, 0);
    Menu_SetCallbacks(&g_menu0x00541218, NULL, NULL, (MenuCallback)FUN_0044bcd0, NULL);
    Menu_ValidateCursor(&g_menu0x00541218, 0);
    Menu_Init(&g_menu0x0053f3c8, 0, 0, 0, &g_menu0x0053ea68, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x0053f3c8, 0, 0, &g_menu0x00541400, NULL, 0);
    Menu_SetCallbacks(&g_menu0x0053f3c8, NULL, NULL, (MenuCallback)FUN_0044d260, NULL);
    Menu_ValidateCursor(&g_menu0x0053f3c8, 0);
    Menu_Init(&g_menu0x0053f5b0, 0, 0, 0, &g_menu0x00541218, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x0053f5b0, 0, 0, &g_menu0x0053e4b8, NULL, 0);
    Menu_SetCallbacks(&g_menu0x0053f5b0, (MenuCallback)FUN_00449ba0, NULL, (MenuCallback)FUN_0044d790, NULL);
    Menu_ValidateCursor(&g_menu0x0053f5b0, 0);
    Menu_Init(&g_menu0x0053e4b8, 0, 0, 0, NULL, NULL, 1, 0, 0);
    Menu_AddItemType3(&g_menu0x0053e4b8, 0, 0, 0xb, 0, 0, 0, (int)FUN_00449e90, 0);
    Menu_SetCallbacks(&g_menu0x0053e4b8, (MenuCallback)FUN_00449ba0, NULL, (MenuCallback)FUN_0044d960, NULL);
    Menu_ValidateCursor(&g_menu0x0053e4b8, 0);
    Menu_Init(&g_menu0x00540c68, 0, 0, 0, NULL, NULL, 1, 0, 0);
    Menu_AddItemType3(&g_menu0x00540c68, 0, 0, 8, 0, 0, 0, (int)FUN_00449e90, 0);
    Menu_SetCallbacks(&g_menu0x00540c68, (MenuCallback)FUN_00449ca0, NULL, (MenuCallback)FUN_0044e130, NULL);
    Menu_ValidateCursor(&g_menu0x00540c68, 0);
    Menu_Init(&g_menu0x005406b8, 0, 0, 0, NULL, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x005406b8, 0, 0, NULL, NULL, 0);
    Menu_SetCallbacks(&g_menu0x005406b8, NULL, NULL, (MenuCallback)FUN_0044e830, NULL);
    Menu_ValidateCursor(&g_menu0x005406b8, 0);
    Menu_Init(&g_menu0x0053f008, 0, 0, 0, &g_menu0x0053e4b8, NULL, 1, 0, 0);
    Menu_AddItemType3(&g_menu0x0053f008, 0, 0, 0xb, 0, 0, 0, (int)FUN_00449e90, 0);
    Menu_SetCallbacks(&g_menu0x0053f008, (MenuCallback)FUN_00449ba0, NULL, (MenuCallback)FUN_0044fea0, NULL);
    Menu_ValidateCursor(&g_menu0x0053f008, 0);
    Menu_Init(&g_menu0x0053fd58, 0, 0, 0, &g_menu0x00540c68, NULL, 1, 0, 0);
    Menu_AddItemType3(&g_menu0x0053fd58, 0, 0, 0xb, 0, 0, 0, (int)FUN_00449e90, 0);
    Menu_SetCallbacks(&g_menu0x0053fd58, (MenuCallback)FUN_00449ca0, NULL, (MenuCallback)FUN_004505b0, NULL);
    Menu_ValidateCursor(&g_menu0x0053fd58, 0);
    Menu_Init(&g_menu0x0053e6a0, 0, 0, 0, &g_menu0x0053f008, NULL, 1, 0, 0);
    Menu_AddItemType3(&g_menu0x0053e6a0, 0, 0, 0xb, 0, 0, 0, (int)FUN_00449e90, 0);
    Menu_SetCallbacks(&g_menu0x0053e6a0, (MenuCallback)FUN_00449ba0, NULL, (MenuCallback)FUN_0044efa0, NULL);
    Menu_ValidateCursor(&g_menu0x0053e6a0, 0);
    Menu_Init(&g_menu0x00540118, 0, 0, 0, &g_menu0x0053e6a0, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x00540118, 0, 0, NULL, NULL, 0);
    Menu_SetCallbacks(&g_menu0x00540118, NULL, NULL, (MenuCallback)FUN_00450c10, NULL);
    Menu_ValidateCursor(&g_menu0x00540118, 0);
    Menu_Init(&g_menu0x00540a80, 0, 0, 0, &g_menu0x00541030, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x00540a80, 0, 0, NULL, NULL, 0);
    Menu_SetCallbacks(&g_menu0x00540a80, NULL, NULL, (MenuCallback)FUN_00450ed0, NULL);
    Menu_ValidateCursor(&g_menu0x00540a80, 0);
    Menu_Init(&g_menu0x00541030, 0, 0, 0, &g_menu0x0053fd58, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x00541030, 0, 0, NULL, NULL, 0);
    Menu_SetCallbacks(&g_menu0x00541030, NULL, NULL, (MenuCallback)FUN_0044f8a0, NULL);
    Menu_ValidateCursor(&g_menu0x00541030, 0);
    Menu_Init(&g_menu0x00541ae0, 0, 0, 0, &g_menu0x00540118, NULL, 1, 0, 0);
    Menu_AddItemType3(&g_menu0x00541ae0, 0, 0, 0xb, 0, 0, 0, (int)FUN_00449e90, 0);
    Menu_SetCallbacks(&g_menu0x00541ae0, (MenuCallback)FUN_00449ba0, NULL, (MenuCallback)FUN_00450ef0, NULL);
    Menu_ValidateCursor(&g_menu0x00541ae0, 0);
    Menu_Init(&g_menu0x0053f790, 0, 0, 0, &g_menu0x00541ae0, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x0053f790, 0, 0, &g_menu0x0053f970, NULL, 0);
    Menu_SetCallbacks(&g_menu0x0053f790, NULL, NULL, (MenuCallback)FUN_00451690, NULL);
    Menu_ValidateCursor(&g_menu0x0053f790, 0);
    Menu_Init(&g_menu0x0053f970, 0, 0, 0, &g_menu0x0053f790, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x0053f970, 0, 0, NULL, (int)FUN_0044a000, 0);
    Menu_SetCallbacks(&g_menu0x0053f970, (MenuCallback)FUN_00449b80, NULL, (MenuCallback)FUN_00451690, NULL);
    Menu_ValidateCursor(&g_menu0x0053f970, 0);
    Menu_Init(&g_menu0x0053fb70, 0, 0, 0, NULL, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x0053fb70, 0, 0, NULL, NULL, 0);
    Menu_SetCallbacks(&g_menu0x0053fb70, NULL, NULL, (MenuCallback)FUN_00451df0, NULL);
    Menu_ValidateCursor(&g_menu0x0053fb70, 0);
    Menu_Init(&g_menu0x0053e2d8, 0, 0, 0, NULL, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x0053e2d8, 0, 0, NULL, NULL, 0);
    Menu_SetCallbacks(&g_menu0x0053e2d8, NULL, NULL, (MenuCallback)FUN_00452430, NULL);
    Menu_ValidateCursor(&g_menu0x0053e2d8, 0);
    Menu_Init(&g_menu0x0053ec48, 0, 0, 0, &g_menu0x0053fb70, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x0053ec48, 0, 0, NULL, NULL, 0);
    Menu_SetCallbacks(&g_menu0x0053ec48, NULL, NULL, (MenuCallback)FUN_00451df0, NULL);
    Menu_ValidateCursor(&g_menu0x0053ec48, 0);
    Menu_Init(&g_menu0x0053e888, 0, 0, 0, &g_menu0x0053ec48, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x0053e888, 0, 0, NULL, NULL, 0);
    Menu_SetCallbacks(&g_menu0x0053e888, NULL, NULL, (MenuCallback)FUN_004530e0, NULL);
    Menu_ValidateCursor(&g_menu0x0053e888, 0);
    Menu_Init(&g_menu0x0053ee28, 0, 0, 0, &g_menu0x0053e888, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x0053ee28, 0, 0, &g_menu0x00541400, NULL, 0);
    Menu_SetCallbacks(&g_menu0x0053ee28, NULL, NULL, (MenuCallback)FUN_00453830, NULL);
    Menu_ValidateCursor(&g_menu0x0053ee28, 0);
    if (CGameInfo::FUN_00405d80() == 4) {
    Menu_Init(&g_menu0x0053f1e8, 0, 0, 0, &g_menu0x00541218, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x0053f1e8, 0, 0, &g_menu0x00541400, NULL, 0);
    Menu_SetCallbacks(&g_menu0x0053f1e8, NULL, NULL, (MenuCallback)FUN_004529c0, NULL);
    Menu_ValidateCursor(&g_menu0x0053f1e8, 0);
    }
    Menu_Init(&g_menu0x00540e50, 0, 0, 0, NULL, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x00540e50, 0, 0, &g_menu0x00541400, NULL, 0);
    Menu_SetCallbacks(&g_menu0x00540e50, NULL, NULL, (MenuCallback)FUN_00452be0, NULL);
    Menu_ValidateCursor(&g_menu0x00540e50, 0);
    if (CGameInfo::FUN_00405e00() != 0)
        Menu_Init(&g_menu0x005402f8, 0, 0, 0, &g_menu0x005416e0, NULL, 1, 0, 1);
    else
        Menu_Init(&g_menu0x005402f8, 0, 0, 0, &g_menu0x00541400, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x005402f8, 0, -1, NULL, (int)FUN_00449f30, 1000);
    if (CGameInfo::FUN_00405d80() == 2 || CGameInfo::FUN_00405d80() == 3)
        Menu_AddItemType2(&g_menu0x005402f8, 0, 0x8b, NULL, (int)FUN_0044a0a0, 0x3e9);
    Menu_AddItemType1(&g_menu0x005402f8, 0, 0xa6, 0, -1);
    Menu_SetCallbacks(&g_menu0x005402f8, (MenuCallback)FUN_00449b90, NULL, (MenuCallback)FUN_00453c50, NULL);
    Menu_ValidateCursor(&g_menu0x005402f8, 0);
    if (CGameInfo::FUN_00405e00() != 0)
        Menu_Init(&g_menu0x005418d8, 0, 0, 0, &g_menu0x005416e0, NULL, 1, 0, 1);
    else
        Menu_Init(&g_menu0x005418d8, 0, 0, 0, &g_menu0x00541400, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x005418d8, 0, 0xa8, NULL, (int)FUN_00449b00, 1000);
    Menu_AddItemType1(&g_menu0x005418d8, 0, 0xaa, 0, -1);
    Menu_SetCallbacks(&g_menu0x005418d8, (MenuCallback)FUN_00449b90, NULL, (MenuCallback)FUN_00453c50, NULL);
    Menu_ValidateCursor(&g_menu0x005418d8, 0);
    Menu_Init(&g_menu0x005408a0, 0, 0, 0, &g_menu0x005416e0, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x005408a0, 0, 0xf6, NULL, (int)FUN_00449b60, 1000);
    Menu_AddItemType1(&g_menu0x005408a0, 0, 0xaa, 0, -1);
    Menu_SetCallbacks(&g_menu0x005408a0, (MenuCallback)FUN_00449b90, NULL, (MenuCallback)FUN_00453c50, NULL);
    Menu_ValidateCursor(&g_menu0x005408a0, 0);
    Menu_Init(&g_menu0x005404d8, 0, 0, 0, NULL, NULL, 1, 0, 0);
    Menu_SetCallbacks(&g_menu0x005404d8, (MenuCallback)FUN_0044a090, NULL, (MenuCallback)FUN_00453c50, NULL);
    Menu_ValidateCursor(&g_menu0x005404d8, 0);
}
