#include <windows.h>
#include "FrontendMenus.h"
#include "GameInfo.h"

// GLOBAL: CMR2 0x0081b158
Menu g_menu0x0081b158;
// GLOBAL: CMR2 0x0081b338
Menu g_menu0x0081b338;
// GLOBAL: CMR2 0x0081b518
Menu g_menu0x0081b518;
// GLOBAL: CMR2 0x0081bab8
Menu g_menu0x0081bab8;
// GLOBAL: CMR2 0x0081bc98
Menu g_menu0x0081bc98;
// GLOBAL: CMR2 0x0081c058
Menu g_menu0x0081c058;
// GLOBAL: CMR2 0x0081c418
Menu g_menu0x0081c418;
// GLOBAL: CMR2 0x0081c5f8
Menu g_menu0x0081c5f8;
// GLOBAL: CMR2 0x0081c7d8
Menu g_menu0x0081c7d8;
// GLOBAL: CMR2 0x0081cb98
Menu g_menu0x0081cb98;
// GLOBAL: CMR2 0x0081cd78
Menu g_menu0x0081cd78;
// GLOBAL: CMR2 0x0081cf58
Menu g_menu0x0081cf58;
// GLOBAL: CMR2 0x0081d138
Menu g_menu0x0081d138;
// GLOBAL: CMR2 0x0081d318
Menu g_menu0x0081d318;
// GLOBAL: CMR2 0x0081d4f8
Menu g_menu0x0081d4f8;
// GLOBAL: CMR2 0x0081d6d8
Menu g_menu0x0081d6d8;
// GLOBAL: CMR2 0x0081d8b8
Menu g_menu0x0081d8b8;
// GLOBAL: CMR2 0x0081dc78
Menu g_menu0x0081dc78;
// GLOBAL: CMR2 0x0081de58
Menu g_menu0x0081de58;
// GLOBAL: CMR2 0x0081e218
Menu g_menu0x0081e218;
// GLOBAL: CMR2 0x0081e3f8
Menu g_menu0x0081e3f8;
// GLOBAL: CMR2 0x0081e5d8
Menu g_menu0x0081e5d8;
// GLOBAL: CMR2 0x0081e998
Menu g_menu0x0081e998;
// GLOBAL: CMR2 0x0081eb78
Menu g_menu0x0081eb78;
// GLOBAL: CMR2 0x0081ed58
Menu g_menu0x0081ed58;
// GLOBAL: CMR2 0x0081ef38
Menu g_menu0x0081ef38;
// GLOBAL: CMR2 0x0081f118
Menu g_menu0x0081f118;
// GLOBAL: CMR2 0x0081f2f8
Menu g_menu0x0081f2f8;
// GLOBAL: CMR2 0x0081f4d8
Menu g_menu0x0081f4d8;
// GLOBAL: CMR2 0x0081f6b8
Menu g_menu0x0081f6b8;
// GLOBAL: CMR2 0x0081fa78
Menu g_menu0x0081fa78;
// GLOBAL: CMR2 0x0081fc58
Menu g_menu0x0081fc58;
// GLOBAL: CMR2 0x0081fe38
Menu g_menu0x0081fe38;
// GLOBAL: CMR2 0x00820018
Menu g_menu0x00820018;
// GLOBAL: CMR2 0x008201f8
Menu g_menu0x008201f8;
// GLOBAL: CMR2 0x008203d8
Menu g_menu0x008203d8;
// GLOBAL: CMR2 0x008205b8
Menu g_menu0x008205b8;
// GLOBAL: CMR2 0x00820798
Menu g_menu0x00820798;
// GLOBAL: CMR2 0x00820978
Menu g_menu0x00820978;
// GLOBAL: CMR2 0x00820b58
Menu g_menu0x00820b58;
// GLOBAL: CMR2 0x00820d38
Menu g_menu0x00820d38;
// GLOBAL: CMR2 0x008210f8
Menu g_menu0x008210f8;
// GLOBAL: CMR2 0x008212d8
Menu g_menu0x008212d8;
// GLOBAL: CMR2 0x008214b8
Menu g_menu0x008214b8;
// GLOBAL: CMR2 0x00821878
Menu g_menu0x00821878;
// GLOBAL: CMR2 0x00821c38
Menu g_menu0x00821c38;
// GLOBAL: CMR2 0x00821e18
Menu g_menu0x00821e18;
// GLOBAL: CMR2 0x008221d8
Menu g_menu0x008221d8;
// GLOBAL: CMR2 0x008223b8
Menu g_menu0x008223b8;
// GLOBAL: CMR2 0x00822598
Menu g_menu0x00822598;
// GLOBAL: CMR2 0x00822958
Menu g_menu0x00822958;
// GLOBAL: CMR2 0x00822d18
Menu g_menu0x00822d18;
// GLOBAL: CMR2 0x00822ef8
Menu g_menu0x00822ef8;
// GLOBAL: CMR2 0x008232b8
Menu g_menu0x008232b8;
// GLOBAL: CMR2 0x00823498
Menu g_menu0x00823498;
// GLOBAL: CMR2 0x00823678
Menu g_menu0x00823678;
// GLOBAL: CMR2 0x00823858
Menu g_menu0x00823858;
// GLOBAL: CMR2 0x00823a38
Menu g_menu0x00823a38;
// GLOBAL: CMR2 0x00823c18
Menu g_menu0x00823c18;
// GLOBAL: CMR2 0x00823df8
Menu g_menu0x00823df8;
// GLOBAL: CMR2 0x00823fd8
Menu g_menu0x00823fd8;
// GLOBAL: CMR2 0x008241b8
Menu g_menu0x008241b8;
// GLOBAL: CMR2 0x00824498
Menu g_menu0x00824498;
// GLOBAL: CMR2 0x00824678
Menu g_menu0x00824678;
// GLOBAL: CMR2 0x00824a38
Menu g_menu0x00824a38;
// GLOBAL: CMR2 0x00824c18
Menu g_menu0x00824c18;
// GLOBAL: CMR2 0x00824df8
Menu g_menu0x00824df8;
// GLOBAL: CMR2 0x00824fd8
Menu g_menu0x00824fd8;
// GLOBAL: CMR2 0x00826420
Menu g_menu0x00826420;
// GLOBAL: CMR2 0x008267e0
Menu g_menu0x008267e0;
// GLOBAL: CMR2 0x00826ba0
Menu g_menu0x00826ba0;
// GLOBAL: CMR2 0x00826d80
Menu g_menu0x00826d80;
// GLOBAL: CMR2 0x00826f60
Menu g_menu0x00826f60;
// GLOBAL: CMR2 0x00827140
Menu g_menu0x00827140;
// GLOBAL: CMR2 0x00827320
Menu g_menu0x00827320;
// GLOBAL: CMR2 0x00827500
Menu g_menu0x00827500;
// GLOBAL: CMR2 0x008278c0
Menu g_menu0x008278c0;
// GLOBAL: CMR2 0x00827c80
Menu g_menu0x00827c80;
// GLOBAL: CMR2 0x00827e60
Menu g_menu0x00827e60;
// GLOBAL: CMR2 0x00828220
Menu g_menu0x00828220;
// GLOBAL: CMR2 0x00828500
Menu g_menu0x00828500;
// GLOBAL: CMR2 0x008286e0
Menu g_menu0x008286e0;

// FUNCTION: CMR2 0x004f5520
void FUN_004f5520(void)
{
    Menu_Init(&g_menu0x008205b8, 0, 1, 0, &g_menu0x0081d6d8, NULL, 1, 0, 0);
    Menu_AddItemType1(&g_menu0x008205b8, 0, 0x1b, 0, 0);
    Menu_SetCallbacks(&g_menu0x008205b8, NULL, NULL, FUN_004e2b40, NULL);
    Menu_ValidateCursor(&g_menu0x008205b8, 0);
}

// FUNCTION: CMR2 0x004f5810
void FUN_004f5810(void)
{
    Menu_Init(&g_menu0x0081fe38, 0, 0x1a, 0, &g_menu0x008212d8, NULL, 1, 0, 1);
    Menu_SetCallbacks(&g_menu0x0081fe38, NULL, NULL, FUN_004d43e0, NULL);
    Menu_ValidateCursor(&g_menu0x0081fe38, 0);
}

// FUNCTION: CMR2 0x004f5850
void FUN_004f5850(void)
{
    Menu_Init(&g_menu0x0081c058, 0, 0x59, 0, &g_menu0x008212d8, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x0081c058, 0, 0x5a, &g_menu0x00823678, 0, -1);
    Menu_AddItemType2(&g_menu0x0081c058, 0, 0x82, &g_menu0x00820d38, 0, -1);
    Menu_AddItemType2(&g_menu0x0081c058, 0, 0x84, &g_menu0x00821878, 0, -1);
    Menu_SetCallbacks(&g_menu0x0081c058, (MenuCallback)FUN_004f3ac0, NULL, (MenuCallback)FUN_004d4c40, NULL);
    Menu_ValidateCursor(&g_menu0x0081c058, 0);
}

// FUNCTION: CMR2 0x004f58e0
void FUN_004f58e0(void)
{
    Menu_Init(&g_menu0x00823678, 0, 0x5a, 0, &g_menu0x0081c058, NULL, 1, 0, 1);
    Menu_AddItemType4(&g_menu0x00823678, 0, -1, 0, -1);
    Menu_SetCallbacks(&g_menu0x00823678, NULL, NULL, FUN_004d4cf0, NULL);
    Menu_ValidateCursor(&g_menu0x00823678, 0);
}

// FUNCTION: CMR2 0x004f5940
void FUN_004f5940(void)
{
    Menu_Init(&g_menu0x00821878, 0, 0x84, 0, &g_menu0x0081c058, NULL, 1, 0, 1);
    Menu_AddItemType4(&g_menu0x00821878, 0, 0x82, 0, 0);
    Menu_SetCallbacks(&g_menu0x00821878, NULL, NULL, FUN_004d50a0, NULL);
    Menu_ValidateCursor(&g_menu0x00821878, 0);
}

// FUNCTION: CMR2 0x004f59a0
void FUN_004f59a0(void)
{
    Menu_Init(&g_menu0x00823a38, 0, 0x58, 0, &g_menu0x0081d6d8, NULL, 1, 2, 1);
    Menu_AddItemType2(&g_menu0x00823a38, 0, 0xc, &g_menu0x0081dc78, 0, 0);
    Menu_AddItemType2(&g_menu0x00823a38, 0, 0xd, &g_menu0x00824498, 0, 0);
    Menu_AddItemType2(&g_menu0x00823a38, 0, 0xf, &g_menu0x00823fd8, 0, 0);
    Menu_AddItemType2(&g_menu0x00823a38, 0, 0x10, &g_menu0x00824678, 0, 0);
    Menu_AddItemType2(&g_menu0x00823a38, 0, 0x11, &g_menu0x0081e5d8, 0, 0);
    Menu_SetCallbacks(&g_menu0x00823a38, FUN_004ef5f0, (MenuCallback)FUN_004f3ae0, (MenuCallback)FUN_004e1fb0, FUN_004ef600);
    Menu_ValidateCursor(&g_menu0x00823a38, 0);
}

// FUNCTION: CMR2 0x004f5a60
void FUN_004f5a60(void)
{
    Menu_Init(&g_menu0x0081dc78, 0, 0xda, 0, &g_menu0x00823a38, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x0081dc78, 0, 0x95, &g_menu0x0081d4f8, 0, 0);
    Menu_AddItemType2(&g_menu0x0081dc78, 0, 0x8a, &g_menu0x00820018, 0, -1);
    Menu_SetCallbacks(&g_menu0x0081dc78, NULL, NULL, (MenuCallback)FUN_004e2040, NULL);
    Menu_ValidateCursor(&g_menu0x0081dc78, 0);
}

// FUNCTION: CMR2 0x004f5ae0
void FUN_004f5ae0(void)
{
    Menu_Init(&g_menu0x00820018, 0, 0xda, 0, &g_menu0x0081dc78, NULL, 1, 0, 0);
    Menu_AddItemType3(&g_menu0x00820018, 0, -1, 1, 0, 0, 0, (int)FUN_004f3a00, -1);
    Menu_SetCallbacks(&g_menu0x00820018, FUN_004ef4c0, (MenuCallback)FUN_004ef4e0, FUN_004e20e0, NULL);
    Menu_ValidateCursor(&g_menu0x00820018, 0);
}

// FUNCTION: CMR2 0x004f5b50
void FUN_004f5b50(void)
{
    Menu_Init(&g_menu0x008214b8, 0, 0, 0, NULL, NULL, 1, 0, 1);
    Menu_SetCallbacks(&g_menu0x008214b8, NULL, FUN_004ef590, NULL, NULL);
    Menu_ValidateCursor(&g_menu0x008214b8, 0);
}

// FUNCTION: CMR2 0x004f5b90
void FUN_004f5b90(void)
{
    Menu_Init(&g_menu0x00822598, 0, 0x3c, 0, &g_menu0x008241b8, NULL, 1, 0, 0);
    Menu_AddItemType3(&g_menu0x00822598, 0, -1, 0xa, 0, 1, 0, (int)FUN_004ecf80, 0);
    Menu_SetCallbacks(&g_menu0x00822598, FUN_004ec9a0, NULL, (MenuCallback)FUN_004dc7b0, (MenuCallback)FUN_004ecfa0);
    Menu_ValidateCursor(&g_menu0x00822598, 0);
}

// FUNCTION: CMR2 0x004f5c00
void FUN_004f5c00(void)
{
    Menu_Init(&g_menu0x00824c18, 0, 0x35, 0, &g_menu0x00822598, NULL, 1, 0, 1);
    Menu_AddItemType4(&g_menu0x00824c18, 0, 0x1ec, (int)FUN_004ecd60, 0);
    Menu_AddItemType4(&g_menu0x00824c18, 0, 0x3d, (int)FUN_004ecea0, 0);
    Menu_AddItemType1(&g_menu0x00824c18, 0, 0x1b, 0, 0);
    Menu_SetCallbacks(&g_menu0x00824c18, (MenuCallback)FUN_004eca60, FUN_004ecaf0, FUN_004dc930, (MenuCallback)FUN_004ecfa0);
    Menu_ValidateCursor(&g_menu0x00824c18, 0);
}

// FUNCTION: CMR2 0x004f5c90
void FUN_004f5c90(void)
{
    Menu_Init(&g_menu0x00821c38, 0, 0x3d, 0, &g_menu0x00824c18, NULL, 1, 0, 1);
    Menu_AddItemType4(&g_menu0x00821c38, 0, -1, 0, 0);
    Menu_AddItemType3(&g_menu0x00821c38, 0, -1, 5, 0, 1, 0, 0, 1);
    Menu_AddItemType3(&g_menu0x00821c38, 0, -1, 0x7a, 0xb, 1, 0, 0, 2);
    Menu_AddItemType3(&g_menu0x00821c38, 0, -1, 8, 6, 1, 0, 0, 3);
    Menu_AddItemType4(&g_menu0x00821c38, 0, -1, 0, 4);
    Menu_AddItemType2(&g_menu0x00821c38, 0, -1, &g_menu0x0081bab8, 0, 5);
    Menu_AddItemType4(&g_menu0x00821c38, 0, 0x67, (int)FUN_004ed340, 6);
    Menu_AddItemType1(&g_menu0x00821c38, 0, 0x1b, 0, 7);
    Menu_SetCallbacks(&g_menu0x00821c38, FUN_004ecfd0, FUN_004ed100, FUN_004dce00, (MenuCallback)FUN_004ed500);
    Menu_ValidateCursor(&g_menu0x00821c38, 0);
}

// FUNCTION: CMR2 0x004f5d90
void FUN_004f5d90(void)
{
    Menu_Init(&g_menu0x0081e218, 0, 0x3e, 0, &g_menu0x00824c18, NULL, 1, 0, 1);
    Menu_AddItemType4(&g_menu0x0081e218, 0, -1, 0, 0);
    Menu_AddItemType3(&g_menu0x0081e218, 0, -1, 0x16, 0, 1, 0, 0, 1);
    Menu_AddItemType3(&g_menu0x0081e218, 0, -1, 2, 0, 1, 0, 0, 2);
    Menu_AddItemType3(&g_menu0x0081e218, 0, -1, 2, 0, 1, 0, (int)FUN_004ed610, 3);
    Menu_AddItemType1(&g_menu0x0081e218, 0, 0x1b, 0, 4);
    Menu_SetCallbacks(&g_menu0x0081e218, FUN_004ed530, FUN_004ed840, FUN_004dd4b0, (MenuCallback)FUN_004edb30);
    Menu_ValidateCursor(&g_menu0x0081e218, 0);
}

// FUNCTION: CMR2 0x004f5e50
void FUN_004f5e50(void)
{
    Menu_Init(&g_menu0x00824df8, 0, -1, 0, NULL, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x00824df8, 0, -1, &g_menu0x0081e218, 0, -1);
    Menu_SetCallbacks(&g_menu0x00824df8, NULL, FUN_004edb60, FUN_004de1d0, NULL);
    Menu_ValidateCursor(&g_menu0x00824df8, 0);
}

// FUNCTION: CMR2 0x004f6040
void FUN_004f6040(void)
{
    Menu_Init(&g_menu0x0081ef38, 0, 0x14, 0, &g_menu0x0081f118, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x0081ef38, 0, 0x91, 2, 0, 0, 0, 0, 0);
    Menu_AddItemType3(&g_menu0x0081ef38, 0, 0x93, 2, 0, 0, 0, 0, 2);
    Menu_AddItemType3(&g_menu0x0081ef38, 0, 0x96, 2, 0, 0, 0, 0, 3);
    Menu_AddItemType3(&g_menu0x0081ef38, 0, 0x151, 3, 0, 0, 0, 0, 4);
    Menu_AddItemType2(&g_menu0x0081ef38, 0, 0x8e, &g_menu0x008210f8, 0, -1);
    Menu_AddItemType2(&g_menu0x0081ef38, 0, 0x67, g_menu0x0081ef38.pParent, (int)FUN_004f2d20, -1);
    Menu_SetCallbacks(&g_menu0x0081ef38, (MenuCallback)FUN_004f2c40, NULL, FUN_004df410, NULL);
    Menu_ValidateCursor(&g_menu0x0081ef38, 0);
}

// FUNCTION: CMR2 0x004f6130
void FUN_004f6130(void)
{
    Menu_Init(&g_menu0x008210f8, 0, 0x8e, 0, &g_menu0x0081ef38, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x008210f8, 0, -1, &g_menu0x0081de58, 0, 0);
    Menu_AddItemType3(&g_menu0x008210f8, 0, -1, 2, 0, 0, 0, 0, 1);
    Menu_AddItemType2(&g_menu0x008210f8, 0, -1, &g_menu0x0081c7d8, 0, 2);
    Menu_AddItemType2(&g_menu0x008210f8, 0, -1, &g_menu0x0081cb98, 0, 3);
    Menu_AddItemType3(&g_menu0x008210f8, 0, -1, 3, 0, 0, 0, 0, 4);
    Menu_AddItemType3(&g_menu0x008210f8, 0, -1, 0xa, 0, 0, 0, 0, 5);
    Menu_AddItemType2(&g_menu0x008210f8, 0, 0x1ac, &g_menu0x0081cd78, 0, 6);
    Menu_AddItemType1(&g_menu0x008210f8, 0, 0x67, (int)FUN_004f1e40, 0xe);
    Menu_SetCallbacks(&g_menu0x008210f8, FUN_004f1c00, NULL, FUN_004dfe20, (MenuCallback)FUN_004f1f70);
    Menu_ValidateCursor(&g_menu0x008210f8, 0);
}

// FUNCTION: CMR2 0x004f6240
void FUN_004f6240(void)
{
    Menu_Init(&g_menu0x0081cd78, 0, 0x1b4, 0, &g_menu0x008210f8, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x0081cd78, 0, 0x1b8, 3, 0, 0, 0, 0, 8);
    Menu_AddItemType3(&g_menu0x0081cd78, 0, 0x1b7, 2, 0, 0, 0, 0, 9);
    Menu_AddItemType3(&g_menu0x0081cd78, 0, 0x1b6, 2, 0, 0, 0, 0, 0xa);
    Menu_AddItemType1(&g_menu0x0081cd78, 0, 0x67, (int)FUN_004f1db0, 0xe);
    Menu_SetCallbacks(&g_menu0x0081cd78, FUN_004f1d00, (MenuCallback)FUN_004f1d60, FUN_004e0770, NULL);
    Menu_ValidateCursor(&g_menu0x0081cd78, 0);
}

// FUNCTION: CMR2 0x004f6300
void FUN_004f6300(void)
{
    Menu_Init(&g_menu0x0081de58, 0, 0x1b1, 0, &g_menu0x008210f8, NULL, 1, 0, 1);
    Menu_AddItemType4(&g_menu0x0081de58, 0, 0x133, (int)FUN_004f2050, 0);
    Menu_AddItemType4(&g_menu0x0081de58, 0, 0x1ae, (int)FUN_004f2050, 0);
    Menu_AddItemType4(&g_menu0x0081de58, 0, 0x1af, (int)FUN_004f2050, 0);
    Menu_AddItemType4(&g_menu0x0081de58, 0, 0x1b0, (int)FUN_004f2050, 0);
    Menu_SetCallbacks(&g_menu0x0081de58, FUN_004f1fa0, NULL, (MenuCallback)FUN_004e1890, NULL);
    Menu_ValidateCursor(&g_menu0x0081de58, 0);
}

// FUNCTION: CMR2 0x004f6420
void FUN_004f6420(void)
{
    Menu_Init(&g_menu0x0081cb98, 0, 0x14, 0, &g_menu0x008210f8, NULL, 1, 0, 0);
    Menu_AddItemType3(&g_menu0x0081cb98, 0, -1, 2, 0, 0, 0, (int)FUN_004f2430, 0);
    Menu_SetCallbacks(&g_menu0x0081cb98, (MenuCallback)FUN_004f2360, (MenuCallback)FUN_004f23d0, FUN_004e1d70, NULL);
    Menu_ValidateCursor(&g_menu0x0081cb98, 0);
}

// FUNCTION: CMR2 0x004f6490
void FUN_004f6490(void)
{
    Menu_Init(&g_menu0x0081fa78, 0, 0x15, 0, &g_menu0x0081f118, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x0081fa78, 0, 0x5e, 0xb, 0xa, 0, 0, 0, 0);
    Menu_AddItemType3(&g_menu0x0081fa78, 0, 0x5f, 0xb, 0xa, 0, 0, 0, 1);
    Menu_AddItemType3(&g_menu0x0081fa78, 0, 0x60, 0xb, 0xa, 0, 0, 0, 2);
    Menu_AddItemType2(&g_menu0x0081fa78, 0, 0x67, g_menu0x0081fa78.pParent, 0, 4);
    Menu_SetCallbacks(&g_menu0x0081fa78, FUN_004f2840, (MenuCallback)FUN_004f2b00, FUN_004e2610, (MenuCallback)FUN_004f2b70);
    Menu_ValidateCursor(&g_menu0x0081fa78, 0);
}

// FUNCTION: CMR2 0x004f6540
void FUN_004f6540(void)
{
    Menu_Init(&g_menu0x00823c18, 0, 0x16, 0, &g_menu0x0081f118, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x00823c18, 0, 0x162, &g_menu0x00820b58, 0, -1);
    Menu_AddItemType2(&g_menu0x00823c18, 0, 0x163, &g_menu0x0081c418, 0, -1);
    Menu_AddItemType2(&g_menu0x00823c18, 0, 0x164, &g_menu0x008223b8, 0, -1);
    Menu_AddItemType2(&g_menu0x00823c18, 0, 0x165, &g_menu0x008232b8, 0, -1);
    Menu_AddItemType2(&g_menu0x00823c18, 0, 0x166, &g_menu0x0081c5f8, 0, -1);
    Menu_SetCallbacks(&g_menu0x00823c18, FUN_004f3a70, NULL, (MenuCallback)FUN_004e3230, NULL);
    Menu_ValidateCursor(&g_menu0x00823c18, 0);
}

// FUNCTION: CMR2 0x004f6610
void FUN_004f6610(void)
{
    Menu_Init(&g_menu0x00820d38, 0, 0x16, 0, &g_menu0x0081c058, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x00820d38, 0, 0x162, &g_menu0x00823498, 0, -1);
    Menu_AddItemType2(&g_menu0x00820d38, 0, 0x163, &g_menu0x008201f8, 0, -1);
    Menu_AddItemType2(&g_menu0x00820d38, 0, 0x164, &g_menu0x00822ef8, 0, -1);
    Menu_AddItemType2(&g_menu0x00820d38, 0, 0x165, &g_menu0x0081b518, 0, -1);
    Menu_AddItemType2(&g_menu0x00820d38, 0, 0x166, &g_menu0x0081f6b8, 0, -1);
    Menu_SetCallbacks(&g_menu0x00820d38, FUN_004f3a70, NULL, (MenuCallback)FUN_004e3230, NULL);
    Menu_ValidateCursor(&g_menu0x00820d38, 0);
}

// FUNCTION: CMR2 0x004f6740
void FUN_004f6740(void)
{
    Menu_Init(&g_menu0x0081c418, 0, 0x163, 0, &g_menu0x00823c18, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x0081c418, 0, 0x27, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x0081c418, 0, 0x28, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x0081c418, 0, 0x29, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x0081c418, 0, 0x2a, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x0081c418, 0, 0x2b, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x0081c418, 0, 0x2c, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x0081c418, 0, 0x2d, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x0081c418, 0, 0x2e, (int)FUN_004f3a90, -1);
    Menu_SetCallbacks(&g_menu0x0081c418, FUN_004f2d90, (MenuCallback)FUN_004f3970, FUN_004e4130, NULL);
    Menu_ValidateCursor(&g_menu0x0081c418, 0);
}

// FUNCTION: CMR2 0x004f6830
void FUN_004f6830(void)
{
    Menu_Init(&g_menu0x008223b8, 0, 0x164, 0, &g_menu0x00823c18, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x008223b8, 0, 0x27, 0, -1);
    Menu_AddItemType4(&g_menu0x008223b8, 0, 0x28, 0, -1);
    Menu_AddItemType4(&g_menu0x008223b8, 0, 0x29, 0, -1);
    Menu_AddItemType4(&g_menu0x008223b8, 0, 0x2a, 0, -1);
    Menu_AddItemType4(&g_menu0x008223b8, 0, 0x2b, 0, -1);
    Menu_AddItemType4(&g_menu0x008223b8, 0, 0x2c, 0, -1);
    Menu_AddItemType4(&g_menu0x008223b8, 0, 0x2d, 0, -1);
    Menu_AddItemType4(&g_menu0x008223b8, 0, 0x2e, 0, -1);
    Menu_SetCallbacks(&g_menu0x008223b8, FUN_004f2e70, (MenuCallback)FUN_004f3980, FUN_004e4fc0, NULL);
    Menu_ValidateCursor(&g_menu0x008223b8, 0);
}

// FUNCTION: CMR2 0x004f6910
void FUN_004f6910(void)
{
    Menu_Init(&g_menu0x008232b8, 0, 0x165, 0, &g_menu0x00823c18, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x008232b8, 0, 0xe9, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x008232b8, 0, 0xe8, (int)FUN_004f3a90, -1);
    Menu_SetCallbacks(&g_menu0x008232b8, FUN_004f2f40, (MenuCallback)FUN_004f3990, FUN_004e5c90, NULL);
    Menu_ValidateCursor(&g_menu0x008232b8, 0);
}

// FUNCTION: CMR2 0x004f6990
void FUN_004f6990(void)
{
    Menu_Init(&g_menu0x0081c5f8, 0, 0x166, 0, &g_menu0x00823c18, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x0081c5f8, 0, -1, 0, -1);
    Menu_SetCallbacks(&g_menu0x0081c5f8, NULL, NULL, FUN_004e6a80, NULL);
    Menu_ValidateCursor(&g_menu0x0081c5f8, 0);
}

// FUNCTION: CMR2 0x004f6a50
void FUN_004f6a50(void)
{
    Menu_Init(&g_menu0x008201f8, 0, 0x163, 0, &g_menu0x00820d38, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x008201f8, 0, 0x27, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x008201f8, 0, 0x28, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x008201f8, 0, 0x29, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x008201f8, 0, 0x2a, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x008201f8, 0, 0x2b, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x008201f8, 0, 0x2c, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x008201f8, 0, 0x2d, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x008201f8, 0, 0x2e, (int)FUN_004f3a90, -1);
    Menu_SetCallbacks(&g_menu0x008201f8, FUN_004f2d90, (MenuCallback)FUN_004f3970, FUN_004e48b0, NULL);
    Menu_ValidateCursor(&g_menu0x008201f8, 0);
}

// FUNCTION: CMR2 0x004f6c90
void FUN_004f6c90(void)
{
    Menu_Init(&g_menu0x0081b518, 0, 0x165, 0, &g_menu0x00820d38, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x0081b518, 0, 0xe9, (int)FUN_004f3a90, -1);
    Menu_AddItemType4(&g_menu0x0081b518, 0, 0xe8, (int)FUN_004f3a90, -1);
    Menu_SetCallbacks(&g_menu0x0081b518, FUN_004f2f40, (MenuCallback)FUN_004f3990, FUN_004e63d0, NULL);
    Menu_ValidateCursor(&g_menu0x0081b518, 0);
}

// FUNCTION: CMR2 0x004f6d10
void FUN_004f6d10(void)
{
    Menu_Init(&g_menu0x0081f6b8, 0, 0x166, 0, &g_menu0x00820d38, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x0081f6b8, 0, -1, 0, -1);
    Menu_SetCallbacks(&g_menu0x0081f6b8, NULL, NULL, FUN_004e7120, NULL);
    Menu_ValidateCursor(&g_menu0x0081f6b8, 0);
}

// FUNCTION: CMR2 0x004f6d70
void FUN_004f6d70(void)
{
    Menu_Init(&g_menu0x00822958, 0, 0x34, 0, &g_menu0x0081d6d8, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x00822958, 0, -1, 0, 0);
    Menu_SetCallbacks(&g_menu0x00822958, FUN_004f1bd0, NULL, FUN_004dec30, NULL);
    Menu_ValidateCursor(&g_menu0x00822958, 0);
}

// FUNCTION: CMR2 0x004f6dd0
void FUN_004f6dd0(void)
{
    Menu_Init(&g_menu0x00823df8, 0, 3, 0, &g_menu0x0081d6d8, NULL, 1, 1, 1);
    Menu_AddItemType2(&g_menu0x00823df8, 0, 5, &g_menu0x00820798, 0, 0);
    Menu_AddItemType1(&g_menu0x00823df8, 0, 4, (int)FUN_004f3b20, 0);
    Menu_SetCallbacks(&g_menu0x00823df8, FUN_004f3b00, (MenuCallback)FUN_004f3ae0, (MenuCallback)FUN_004e2ab0, (MenuCallback)FUN_004f3b30);
    Menu_ValidateCursor(&g_menu0x00823df8, 0);
}

// FUNCTION: CMR2 0x004f7270
void FUN_004f7270(int unused)
{
    Menu_Init(&g_menu0x0081d4f8, 0, 0x1d, 0, &g_menu0x0081dc78, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x0081d4f8, 0, 0xd0, &g_menu0x0081d318, (int)FUN_004ef930, 0);
    Menu_AddItemType2(&g_menu0x0081d4f8, 0, 0xd1, &g_menu0x0081d318, (int)FUN_004ef930, 0);
    Menu_AddItemType2(&g_menu0x0081d4f8, 0, 0xd2, &g_menu0x0081d318, (int)FUN_004ef930, 0);
    Menu_SetCallbacks(&g_menu0x0081d4f8, FUN_004ef8f0, NULL, (MenuCallback)FUN_004d6290, NULL);
    Menu_ValidateCursor(&g_menu0x0081d4f8, 0);
}

// FUNCTION: CMR2 0x004f7310
void FUN_004f7310(int unused)
{
    Menu_Init(&g_menu0x00824498, 0, 0x1d, 0, &g_menu0x00823a38, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x00824498, 0, 0xd0, &g_menu0x00820978, (int)FUN_004ef930, 0);
    Menu_AddItemType2(&g_menu0x00824498, 0, 0xd1, &g_menu0x00820978, (int)FUN_004ef930, 0);
    Menu_AddItemType2(&g_menu0x00824498, 0, 0xd2, &g_menu0x00820978, (int)FUN_004ef930, 0);
    Menu_SetCallbacks(&g_menu0x00824498, FUN_004ef8f0, NULL, (MenuCallback)FUN_004d6290, NULL);
    Menu_ValidateCursor(&g_menu0x00824498, 0);
}

// FUNCTION: CMR2 0x004f73b0
void FUN_004f73b0(int unused)
{
    Menu_Init(&g_menu0x00823fd8, 0, 0x1d, 0, &g_menu0x00823a38, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x00823fd8, 0, 0xd0, &g_menu0x0081f2f8, (int)FUN_004ef930, 0);
    Menu_AddItemType2(&g_menu0x00823fd8, 0, 0xd1, &g_menu0x0081f2f8, (int)FUN_004ef930, 0);
    Menu_AddItemType2(&g_menu0x00823fd8, 0, 0xd2, &g_menu0x0081f2f8, (int)FUN_004ef930, 0);
    Menu_SetCallbacks(&g_menu0x00823fd8, FUN_004ef8f0, NULL, (MenuCallback)FUN_004d6290, NULL);
    Menu_ValidateCursor(&g_menu0x00823fd8, 0);
}

// FUNCTION: CMR2 0x004f7450
void FUN_004f7450(void)
{
    Menu_Init(&g_menu0x0081ed58, 0, 0x41, 0, &g_menu0x00823a38, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x0081ed58, 0, 0x42, &g_menu0x00823858, (int)FUN_004ef960, -1);
    Menu_AddItemType2(&g_menu0x0081ed58, 0, 0xd0, &g_menu0x00823858, (int)FUN_004ef960, -1);
    Menu_AddItemType2(&g_menu0x0081ed58, 0, 0xd1, &g_menu0x00823858, (int)FUN_004ef960, -1);
    Menu_AddItemType2(&g_menu0x0081ed58, 0, 0xd2, &g_menu0x00823858, (int)FUN_004ef960, -1);
    Menu_SetCallbacks(&g_menu0x0081ed58, FUN_004ef950, NULL, (MenuCallback)FUN_004d63e0, NULL);
    Menu_ValidateCursor(&g_menu0x0081ed58, 0);
}

// FUNCTION: CMR2 0x004f77a0
void FUN_004f77a0(void)
{
    Menu_Init(&g_menu0x008203d8, 0, 0x17a, 0, &g_menu0x008241b8, NULL, 1, 0, 1);
    Menu_AddItemType6(&g_menu0x008203d8, 0, -1, 1, 0, 1, 0, (int)FUN_004f0820, -1);
    Menu_AddItemType1(&g_menu0x008203d8, 0, 0x11a, 0, 0);
    Menu_SetCallbacks(&g_menu0x008203d8, FUN_004f0580, FUN_004f0620, FUN_004d6a60, FUN_004f0600);
    Menu_ValidateCursor(&g_menu0x008203d8, 0);
}

// FUNCTION: CMR2 0x004f7820
void FUN_004f7820(void)
{
    Menu_Init(&g_menu0x00824a38, 0, 0x22, 0, NULL, NULL, 1, 0, 1);
    Menu_AddItemType6(&g_menu0x00824a38, 0, -1, 0xa, 0, 1, 0, (int)FUN_004f1040, 0);
    Menu_AddItemType6(&g_menu0x00824a38, 0, -1, 0xa, 0, 1, 0, (int)FUN_004f1040, 1);
    Menu_AddItemType6(&g_menu0x00824a38, 0, -1, 0xa, 0, 1, 0, (int)FUN_004f1040, 2);
    Menu_SetCallbacks(&g_menu0x00824a38, FUN_004f0d30, FUN_004f0e80, FUN_004d6f10, FUN_004f0da0);
    Menu_ValidateCursor(&g_menu0x00824a38, 0);
}

// FUNCTION: CMR2 0x004f78c0
void FUN_004f78c0(void)
{
    Menu_Init(&g_menu0x0081d8b8, 0, 0x8c, 0, &g_menu0x00824a38, NULL, 1, 0, 1);
    Menu_AddItemType6(&g_menu0x0081d8b8, 0, -1, 0xa, 0, 1, 0, (int)FUN_004f13c0, 0);
    Menu_AddItemType6(&g_menu0x0081d8b8, 0, -1, 0xa, 0, 1, 0, (int)FUN_004f13c0, 1);
    Menu_AddItemType6(&g_menu0x0081d8b8, 0, -1, 0xa, 0, 1, 0, (int)FUN_004f13c0, 2);
    Menu_SetCallbacks(&g_menu0x0081d8b8, (MenuCallback)FUN_004f1160, FUN_004f11d0, FUN_004d7380, FUN_004f0e60);
    Menu_ValidateCursor(&g_menu0x0081d8b8, 0);
}

// FUNCTION: CMR2 0x004f7970
void FUN_004f7970(void)
{
    Menu_Init(&g_menu0x0081bc98, 0, 0x8d, 0, &g_menu0x0081d8b8, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x0081bc98, 0, -1, 0xff, 0x64, 1, 0, 0, 0);
    Menu_AddItemType3(&g_menu0x0081bc98, 0, -1, 0xc, 0, 1, 0, 0, 1);
    Menu_AddItemType3(&g_menu0x0081bc98, 0, -1, 0x1f, 0, 1, 0, 0, 2);
    Menu_AddItemType4(&g_menu0x0081bc98, 0, 0x67, (int)FUN_004f15d0, -1);
    Menu_SetCallbacks(&g_menu0x0081bc98, FUN_004f16f0, (MenuCallback)FUN_004f1640, FUN_004d7750, FUN_004f1b90);
    Menu_ValidateCursor(&g_menu0x0081bc98, 0);
}

// FUNCTION: CMR2 0x004f7a30
void FUN_004f7a30(void)
{
    Menu_Init(&g_menu0x00821e18, 0, 0x33, 0, NULL, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0x98, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0x99, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0x9a, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0x9b, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0x9c, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0x9d, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0x9e, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0x9f, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xa0, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xa1, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xa2, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xa3, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xa4, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xa5, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xa6, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xa7, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xa8, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xa9, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xaa, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xab, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xac, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_AddItemType2(&g_menu0x00821e18, 0, 0xad, &g_menu0x0081cf58, (int)FUN_004f1a10, -1);
    Menu_SetCallbacks(&g_menu0x00821e18, FUN_004f17d0, (MenuCallback)FUN_004f39d0, FUN_004d7db0, FUN_004f1b90);
    Menu_ValidateCursor(&g_menu0x00821e18, 0);
}

// FUNCTION: CMR2 0x004f7d00
void FUN_004f7d00(void)
{
    Menu_Init(&g_menu0x0081cf58, 0, 0x97, 0, &g_menu0x00821e18, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x0081cf58, 0, 0x88, 2, 0, 0, 0, (int)FUN_004f1a40, 0);
    Menu_SetCallbacks(&g_menu0x0081cf58, FUN_004f1960, (MenuCallback)FUN_004f39d0, FUN_004d8480, NULL);
    Menu_ValidateCursor(&g_menu0x0081cf58, 0);
}

// FUNCTION: CMR2 0x004f7d70
void FUN_004f7d70(void)
{
    Menu_Init(&g_menu0x00822d18, 0, 0x33, 0, &g_menu0x008241b8, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x00822d18, 0, 0x88, 2, 0, 0, 0, (int)FUN_004f1b30, 0);
    Menu_SetCallbacks(&g_menu0x00822d18, FUN_004f19d0, NULL, FUN_004d9450, NULL);
    Menu_ValidateCursor(&g_menu0x00822d18, 0);
}

// FUNCTION: CMR2 0x004f7de0
void FUN_004f7de0(void)
{
    Menu_Init(&g_menu0x0081eb78, 0, 0x24, 0, NULL, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x0081eb78, 0, 0x27, (int)FUN_004efb70, -1);
    Menu_AddItemType4(&g_menu0x0081eb78, 0, 0x28, (int)FUN_004efb70, -1);
    Menu_AddItemType4(&g_menu0x0081eb78, 0, 0x29, (int)FUN_004efb70, -1);
    Menu_AddItemType4(&g_menu0x0081eb78, 0, 0x2a, (int)FUN_004efb70, -1);
    Menu_AddItemType4(&g_menu0x0081eb78, 0, 0x2b, (int)FUN_004efb70, -1);
    Menu_AddItemType4(&g_menu0x0081eb78, 0, 0x2c, (int)FUN_004efb70, -1);
    Menu_AddItemType4(&g_menu0x0081eb78, 0, 0x2d, (int)FUN_004efb70, -1);
    Menu_AddItemType4(&g_menu0x0081eb78, 0, 0x2e, (int)FUN_004efb70, -1);
    Menu_SetCallbacks(&g_menu0x0081eb78, FUN_004f36e0, (MenuCallback)FUN_004efb50, FUN_004d9ad0, NULL);
    Menu_ValidateCursor(&g_menu0x0081eb78, 0);
}

// FUNCTION: CMR2 0x004f7ed0
void FUN_004f7ed0(void)
{
    Menu_Init(&g_menu0x0081e3f8, 0, 0x25, 0, &g_menu0x0081eb78, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x0081e3f8, 0, 0xc5, (int)FUN_004efde0, -1);
    Menu_AddItemType4(&g_menu0x0081e3f8, 0, 0xc6, (int)FUN_004efde0, -1);
    Menu_AddItemType4(&g_menu0x0081e3f8, 0, 0xc7, (int)FUN_004efde0, -1);
    Menu_AddItemType4(&g_menu0x0081e3f8, 0, 0xc8, (int)FUN_004efde0, -1);
    Menu_AddItemType4(&g_menu0x0081e3f8, 0, 0xc9, (int)FUN_004efde0, -1);
    Menu_AddItemType4(&g_menu0x0081e3f8, 0, 0xca, (int)FUN_004efde0, -1);
    Menu_AddItemType4(&g_menu0x0081e3f8, 0, 0xcb, (int)FUN_004efde0, -1);
    Menu_AddItemType4(&g_menu0x0081e3f8, 0, 0xcc, (int)FUN_004efde0, -1);
    Menu_AddItemType4(&g_menu0x0081e3f8, 0, 0xcd, (int)FUN_004efde0, -1);
    Menu_AddItemType4(&g_menu0x0081e3f8, 0, 0xce, (int)FUN_004efde0, -1);
    Menu_AddItemType4(&g_menu0x0081e3f8, 0, 0xcf, (int)FUN_004efde0, -1);
    Menu_SetCallbacks(&g_menu0x0081e3f8, FUN_004f3010, (MenuCallback)FUN_004efdc0, FUN_004d9c40, NULL);
    Menu_ValidateCursor(&g_menu0x0081e3f8, 0);
}

// FUNCTION: CMR2 0x004f8020
void FUN_004f8020(void)
{
    Menu_Init(&g_menu0x0081f4d8, 0, 0x155, 0, &g_menu0x0081e3f8, NULL, 1, 0, 1);
    Menu_AddItemType4(&g_menu0x0081f4d8, 0, 0xc5, (int)FUN_004f0050, -1);
    Menu_AddItemType4(&g_menu0x0081f4d8, 0, 0xc6, (int)FUN_004f0050, -1);
    Menu_AddItemType4(&g_menu0x0081f4d8, 0, 0xc7, (int)FUN_004f0050, -1);
    Menu_AddItemType4(&g_menu0x0081f4d8, 0, 0xc8, (int)FUN_004f0050, -1);
    Menu_AddItemType4(&g_menu0x0081f4d8, 0, 0xc9, (int)FUN_004f0050, -1);
    Menu_AddItemType4(&g_menu0x0081f4d8, 0, 0xca, (int)FUN_004f0050, -1);
    Menu_AddItemType4(&g_menu0x0081f4d8, 0, 0xcb, (int)FUN_004f0050, -1);
    Menu_AddItemType4(&g_menu0x0081f4d8, 0, 0xcc, (int)FUN_004f0050, -1);
    Menu_AddItemType4(&g_menu0x0081f4d8, 0, 0xcd, (int)FUN_004f0050, -1);
    Menu_AddItemType4(&g_menu0x0081f4d8, 0, 0xce, (int)FUN_004f0050, -1);
    Menu_AddItemType4(&g_menu0x0081f4d8, 0, 0xcf, (int)FUN_004f0050, -1);
    Menu_SetCallbacks(&g_menu0x0081f4d8, FUN_004f3120, (MenuCallback)FUN_004efdd0, (MenuCallback)FUN_004da630, NULL);
    Menu_ValidateCursor(&g_menu0x0081f4d8, 0);
}

// FUNCTION: CMR2 0x004f8250
void FUN_004f8250(void)
{
    Menu_Init(&g_menu0x00820798, 0, -1, 0, &g_menu0x0081d6d8, NULL, 1, 0, 0);
    Menu_SetCallbacks(&g_menu0x00820798, NULL, FUN_004ef5e0, NULL, NULL);
    Menu_ValidateCursor(&g_menu0x00820798, 0);
}

// FUNCTION: CMR2 0x004f8500
void FUN_004f8500(void)
{
    Menu_Init(&g_menu0x00824fd8, 0, 0x13, 0, &g_menu0x00821c38, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x00824fd8, 0, -1, 0xb, 0, 1, 0, (int)FUN_004edca0, 0);
    Menu_AddItemType3(&g_menu0x00824fd8, 0, -1, 8, 0, 1, 0, 0, 1);
    Menu_AddItemType4(&g_menu0x00824fd8, 0, 0x67, (int)FUN_004edd50, 2);
    Menu_AddItemType1(&g_menu0x00824fd8, 0, 0x1b, 0, 3);
    Menu_SetCallbacks(&g_menu0x00824fd8, FUN_004edb70, FUN_004ede10, FUN_004e77c0, NULL);
    Menu_ValidateCursor(&g_menu0x00824fd8, 0);
}

// FUNCTION: CMR2 0x004f85b0
void FUN_004f85b0(void)
{
    Menu_Init(&g_menu0x0081fc58, 0, 0x13, 0, &g_menu0x00821c38, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x0081fc58, 0, -1, 8, 0, 1, 0, 0, 0);
    Menu_AddItemType3(&g_menu0x0081fc58, 0, -1, 0xb, 0, 1, 0, 0, 1);
    Menu_AddItemType3(&g_menu0x0081fc58, 0, -1, 4, 0, 1, 0, 0, 2);
    Menu_AddItemType4(&g_menu0x0081fc58, 0, 0x67, (int)FUN_004ee090, 3);
    Menu_AddItemType1(&g_menu0x0081fc58, 0, 0x1b, 0, 4);
    Menu_SetCallbacks(&g_menu0x0081fc58, FUN_004edef0, FUN_004ee170, FUN_004e7ed0, NULL);
    Menu_ValidateCursor(&g_menu0x0081fc58, 0);
}

// FUNCTION: CMR2 0x004f8670
void FUN_004f8670(void)
{
    Menu_Init(&g_menu0x0081b338, 0, 0x13, 0, &g_menu0x00821c38, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x0081b338, 0, -1, 8, 0, 1, 0, 0, 0);
    Menu_AddItemType3(&g_menu0x0081b338, 0, -1, 0xb, 0, 1, 0, 0, 1);
    Menu_AddItemType3(&g_menu0x0081b338, 0, -1, 0x39, 0, 1, 0, 0, 2);
    Menu_AddItemType4(&g_menu0x0081b338, 0, 0x67, (int)FUN_004ee600, 3);
    Menu_AddItemType1(&g_menu0x0081b338, 0, 0x1b, 0, 4);
    Menu_SetCallbacks(&g_menu0x0081b338, FUN_004ee460, FUN_004ee6e0, FUN_004e8500, NULL);
    Menu_ValidateCursor(&g_menu0x0081b338, 0);
}

// FUNCTION: CMR2 0x004f8730
void FUN_004f8730(void)
{
    Menu_Init(&g_menu0x0081e998, 0, 0x13, 0, &g_menu0x00821c38, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x0081e998, 0, -1, 8, 0, 1, 0, 0, 0);
    Menu_AddItemType3(&g_menu0x0081e998, 0, -1, 0xa, 0, 1, 0, 0, 1);
    Menu_AddItemType4(&g_menu0x0081e998, 0, 0x67, (int)FUN_004ee9b0, 2);
    Menu_AddItemType1(&g_menu0x0081e998, 0, 0x1b, 0, 3);
    Menu_SetCallbacks(&g_menu0x0081e998, FUN_004ee850, FUN_004eec30, FUN_004e8b60, NULL);
    Menu_ValidateCursor(&g_menu0x0081e998, 0);
}

// FUNCTION: CMR2 0x004f87d0
void FUN_004f87d0(void)
{
    Menu_Init(&g_menu0x0081d138, 0, 0x13, 0, &g_menu0x00821c38, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x0081d138, 0, -1, 8, 0, 1, 0, 0, 0);
    Menu_AddItemType3(&g_menu0x0081d138, 0, -1, 0x3d, 0, 1, 0, 0, 1);
    Menu_AddItemType4(&g_menu0x0081d138, 0, 0x67, (int)FUN_004eec40, 2);
    Menu_AddItemType1(&g_menu0x0081d138, 0, 0x1b, 0, 3);
    Menu_SetCallbacks(&g_menu0x0081d138, (MenuCallback)FUN_004eeab0, FUN_004eec30, FUN_004e90f0, NULL);
    Menu_ValidateCursor(&g_menu0x0081d138, 0);
}

// FUNCTION: CMR2 0x004f8870
void FUN_004f8870(void)
{
    Menu_Init(&g_menu0x0081b158, 0, 0x13, 0, &g_menu0x00824c18, NULL, 1, 0, 1);
    Menu_AddItemType4(&g_menu0x0081b158, 0, -1, (int)FUN_004eee60, -1);
    Menu_SetCallbacks(&g_menu0x0081b158, FUN_004eed50, (MenuCallback)FUN_004eed80, (MenuCallback)FUN_004e9820, NULL);
    Menu_ValidateCursor(&g_menu0x0081b158, 0);
}

// FUNCTION: CMR2 0x004f88e0
void FUN_004f88e0(void)
{
    Menu_Init(&g_menu0x0081bab8, 0, 0x13, 0, &g_menu0x00821c38, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x0081bab8, 0, -1, 1, 0, 1, 0, (int)FUN_004eefb0, 0);
    Menu_AddItemType4(&g_menu0x0081bab8, 0, 0x20, (int)FUN_004eefe0, 1);
    Menu_AddItemType4(&g_menu0x0081bab8, 0, 0x81, (int)FUN_004ef000, 2);
    Menu_AddItemType1(&g_menu0x0081bab8, 0, 0x1b, 0, -1);
    Menu_SetCallbacks(&g_menu0x0081bab8, FUN_004eef30, (MenuCallback)FUN_004ef030, FUN_004e9990, NULL);
    Menu_ValidateCursor(&g_menu0x0081bab8, 0);
}

// FUNCTION: CMR2 0x004f95d0
void FUN_004f95d0(int unused)
{
    Menu_Init(&g_menu0x00828220, 0, 0x1d, 0, &g_menu0x008267e0, NULL, 1, 0, 1);
    Menu_AddItemType2(&g_menu0x00828220, 0, 0xd0, &g_menu0x00827320, (int)FUN_004ef930, 0);
    Menu_AddItemType2(&g_menu0x00828220, 0, 0xd1, &g_menu0x00827320, (int)FUN_004ef930, 0);
    Menu_AddItemType2(&g_menu0x00828220, 0, 0xd2, &g_menu0x00827320, (int)FUN_004ef930, 0);
    Menu_SetCallbacks(&g_menu0x00828220, FUN_004faa00, NULL, (MenuCallback)FUN_004d6290, NULL);
    Menu_ValidateCursor(&g_menu0x00828220, 0);
}

// FUNCTION: CMR2 0x004f9670
void FUN_004f9670(void)
{
    Menu_Init(&g_menu0x00826f60, 0, 0x33, 0, &g_menu0x008286e0, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0x98, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0x99, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0x9a, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0x9b, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0x9c, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0x9d, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0x9e, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0x9f, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xa0, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xa1, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xa2, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xa3, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xa4, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xa5, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xa6, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xa7, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xa8, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xa9, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xaa, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xab, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xac, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_AddItemType2(&g_menu0x00826f60, 0, 0xad, &g_menu0x00826d80, (int)FUN_004facd0, -1);
    Menu_SetCallbacks(&g_menu0x00826f60, FUN_004faa50, (MenuCallback)FUN_004f39d0, FUN_004d8950, NULL);
    Menu_ValidateCursor(&g_menu0x00826f60, 0);
}

// FUNCTION: CMR2 0x004f9940
void FUN_004f9940(void)
{
    Menu_Init(&g_menu0x00826d80, 0, 0x97, 0, &g_menu0x00826f60, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x00826d80, 0, 0x88, 2, 0, 0, 0, (int)FUN_004fad40, 0x88);
    Menu_SetCallbacks(&g_menu0x00826d80, FUN_004fac70, (MenuCallback)FUN_004f39d0, FUN_004d8ed0, NULL);
    Menu_ValidateCursor(&g_menu0x00826d80, 0);
}

// FUNCTION: CMR2 0x004f99b0
void FUN_004f99b0(void)
{
    Menu_Init(&g_menu0x00828500, 0, 0x33, 0, &g_menu0x00827c80, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0x98, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0x99, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0x9a, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0x9b, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0x9c, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0x9d, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0x9e, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0x9f, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xa0, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xa1, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xa2, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xa3, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xa4, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xa5, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xa6, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xa7, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xa8, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xa9, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xaa, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xab, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xac, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_AddItemType2(&g_menu0x00828500, 0, 0xad, &g_menu0x00827140, (int)FUN_004fad90, -1);
    Menu_SetCallbacks(&g_menu0x00828500, FUN_004faa50, (MenuCallback)FUN_004f39d0, FUN_004d8950, NULL);
    Menu_ValidateCursor(&g_menu0x00828500, 0);
}

// FUNCTION: CMR2 0x004f9c80
void FUN_004f9c80(void)
{
    Menu_Init(&g_menu0x00827140, 0, 0x97, 0, &g_menu0x00828500, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x00827140, 0, 0x88, 2, 0, 0, 0, (int)FUN_004fadd0, 0x88);
    Menu_SetCallbacks(&g_menu0x00827140, FUN_004fac70, (MenuCallback)FUN_004f39d0, FUN_004d8ed0, NULL);
    Menu_ValidateCursor(&g_menu0x00827140, 0);
}

// FUNCTION: CMR2 0x004f9cf0
void FUN_004f9cf0(void)
{
    Menu_Init(&g_menu0x00827e60, 0, 0x33, 0, &g_menu0x008278c0, NULL, 1, 0, 0);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0x98, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0x99, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0x9a, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0x9b, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0x9c, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0x9d, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0x9e, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0x9f, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xa0, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xa1, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xa2, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xa3, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xa4, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xa5, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xa6, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xa7, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xa8, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xa9, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xaa, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xab, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xac, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_AddItemType2(&g_menu0x00827e60, 0, 0xad, &g_menu0x00827500, (int)FUN_004fae70, -1);
    Menu_SetCallbacks(&g_menu0x00827e60, FUN_004faa50, (MenuCallback)FUN_004f39d0, FUN_004d8950, NULL);
    Menu_ValidateCursor(&g_menu0x00827e60, 0);
}

// FUNCTION: CMR2 0x004f9fc0
void FUN_004f9fc0(void)
{
    Menu_Init(&g_menu0x00827500, 0, 0x97, 0, &g_menu0x00827e60, NULL, 1, 0, 1);
    Menu_AddItemType3(&g_menu0x00827500, 0, 0x88, 2, 0, 0, 0, (int)FUN_004faea0, 0x88);
    Menu_SetCallbacks(&g_menu0x00827500, FUN_004fac70, (MenuCallback)FUN_004f39d0, FUN_004d8ed0, NULL);
    Menu_ValidateCursor(&g_menu0x00827500, 0);
}

// FUNCTION: CMR2 0x004fa030
void FUN_004fa030(void)
{
    Menu_Init(&g_menu0x00826ba0, 0, 0x26, 0, &g_menu0x00826d80, NULL, 1, 0, 1);
    Menu_AddItemType4(&g_menu0x00826ba0, 0, 0xe9, (int)FUN_004fafe0, 1);
    Menu_AddItemType4(&g_menu0x00826ba0, 0, 0xe8, (int)FUN_004fafe0, 0);
    Menu_SetCallbacks(&g_menu0x00826ba0, FUN_004faef0, (MenuCallback)FUN_004fafd0, FUN_004db850, NULL);
    Menu_ValidateCursor(&g_menu0x00826ba0, 0);
}

// FUNCTION: CMR2 0x004fa0b0
void FUN_004fa0b0(void)
{
    Menu_Init(&g_menu0x00827320, 0, 0x25, 0, &g_menu0x00828220, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x00827320, 0, 0xfa, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00827320, 0, 0xfb, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00827320, 0, 0xfc, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00827320, 0, 0xfd, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00827320, 0, 0xfe, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00827320, 0, 0xff, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00827320, 0, 0x100, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00827320, 0, 0x101, (int)FUN_004fb370, -1);
    Menu_SetCallbacks(&g_menu0x00827320, FUN_004fb010, (MenuCallback)FUN_004fb360, FUN_004dc710, NULL);
    Menu_ValidateCursor(&g_menu0x00827320, 0);
}

// FUNCTION: CMR2 0x004fa1c0
void FUN_004fa1c0(void)
{
    Menu_Init(&g_menu0x00826420, 0, 0x25, 0, &g_menu0x00827500, NULL, 1, 0, 0);
    Menu_AddItemType4(&g_menu0x00826420, 0, 0xfa, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00826420, 0, 0xfb, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00826420, 0, 0xfc, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00826420, 0, 0xfd, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00826420, 0, 0xfe, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00826420, 0, 0xff, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00826420, 0, 0x100, (int)FUN_004fb370, -1);
    Menu_AddItemType4(&g_menu0x00826420, 0, 0x101, (int)FUN_004fb370, -1);
    Menu_SetCallbacks(&g_menu0x00826420, FUN_004fb010, (MenuCallback)FUN_004fb360, FUN_004dc710, NULL);
    Menu_ValidateCursor(&g_menu0x00826420, 0);
}

// FUNCTION: CMR2 0x004f8290
Menu *FUN_004f8290(BYTE param1)
{
    if (param1 != 0 && CGameInfo::GetGameRegion() != 3 && CGameInfo::GetGameRegion() != 2)
        return &g_menu0x008221d8;
    return &g_menu0x0081d6d8;
}

// FUNCTION: CMR2 0x004f8330
Menu *FUN_004f8330(void)
{
    return &g_menu0x008214b8;
}

// FUNCTION: CMR2 0x004f8990
Menu *FUN_004f8990(void)
{
    return &g_menu0x00820798;
}

// FUNCTION: CMR2 0x004fa330
Menu *FUN_004fa330(void)
{
    return &g_menu0x00826ba0;
}

// FUNCTION: CMR2 0x004f8410
Menu *FUN_004f8410(void)
{
    return &g_menu0x0081d6d8;
}

// FUNCTION: CMR2 0x004f83a0
Menu *FUN_004f83a0(void)
{
    return &g_menu0x008241b8;
}

// FUNCTION: CMR2 0x004f8450
Menu *FUN_004f8450(void)
{
    return &g_menu0x00824c18;
}

// FUNCTION: CMR2 0x004f8470
Menu *FUN_004f8470(void)
{
    return &g_menu0x0081e218;
}

// FUNCTION: CMR2 0x004f8360
Menu *FUN_004f8360(void)
{
    return &g_menu0x0081f4d8;
}
