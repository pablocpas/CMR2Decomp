#include "Input.h"
#include "GameInfo.h"
#include "Graphics.h"
#include "main.h"
#include "InstallInfo.h"
#include "Frontend.h"
#include "FileBuffer.h"
#include "Game.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

// DirectInput 7 data formats, taken from the original's .rdata instead of
// dinput.dll. rgodf points at the descriptor arrays below (0x4c4b80, 0x4c4c30
// and 0x4c5c30 in the original), so they have to live in our binary too.

// GLOBAL: CMR2 0x004c4b80
const DIOBJECTDATAFORMAT g_dataFormatMouse2[11] = {
    { &GUID_XAxis, 0x0000, 0x00ffff03, 0x00000000 },
    { &GUID_YAxis, 0x0004, 0x00ffff03, 0x00000000 },
    { &GUID_ZAxis, 0x0008, 0x80ffff03, 0x00000000 },
    { NULL, 0x000c, 0x00ffff0c, 0x00000000 },
    { NULL, 0x000d, 0x00ffff0c, 0x00000000 },
    { NULL, 0x000e, 0x80ffff0c, 0x00000000 },
    { NULL, 0x000f, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0010, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0011, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0012, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0013, 0x80ffff0c, 0x00000000 },
};

// 256 entradas, no 1: el original tiene la tabla completa (0x4c4c30-0x4c5c30 = 4096 B = 256 x 16).
// Con una sola entrada, c_dfDIKeyboard declara dwNumObjs=256 pero rgodf solo cubre 1 objeto: DirectInput
// recorre 256 y acaba leyendo las tablas vecinas (joystick2 y c_dfDIMouse2) como si fueran descriptores,
// y el dwSize de c_dfDIMouse2 (0x18) se usa como puntero -> page fault dentro de DINPUT. Verificado
// ejecutando el binario bajo wine (W173): era el unico crash fatal. Patron tomado del original.
// GLOBAL: CMR2 0x004c4c30
const DIOBJECTDATAFORMAT g_dataFormatKeyboard[256] = {
    { &GUID_Key, 0x0000, 0x8000000c, 0x00000000 },
    { &GUID_Key, 0x0001, 0x8000010c, 0x00000000 },
    { &GUID_Key, 0x0002, 0x8000020c, 0x00000000 },
    { &GUID_Key, 0x0003, 0x8000030c, 0x00000000 },
    { &GUID_Key, 0x0004, 0x8000040c, 0x00000000 },
    { &GUID_Key, 0x0005, 0x8000050c, 0x00000000 },
    { &GUID_Key, 0x0006, 0x8000060c, 0x00000000 },
    { &GUID_Key, 0x0007, 0x8000070c, 0x00000000 },
    { &GUID_Key, 0x0008, 0x8000080c, 0x00000000 },
    { &GUID_Key, 0x0009, 0x8000090c, 0x00000000 },
    { &GUID_Key, 0x000a, 0x80000a0c, 0x00000000 },
    { &GUID_Key, 0x000b, 0x80000b0c, 0x00000000 },
    { &GUID_Key, 0x000c, 0x80000c0c, 0x00000000 },
    { &GUID_Key, 0x000d, 0x80000d0c, 0x00000000 },
    { &GUID_Key, 0x000e, 0x80000e0c, 0x00000000 },
    { &GUID_Key, 0x000f, 0x80000f0c, 0x00000000 },
    { &GUID_Key, 0x0010, 0x8000100c, 0x00000000 },
    { &GUID_Key, 0x0011, 0x8000110c, 0x00000000 },
    { &GUID_Key, 0x0012, 0x8000120c, 0x00000000 },
    { &GUID_Key, 0x0013, 0x8000130c, 0x00000000 },
    { &GUID_Key, 0x0014, 0x8000140c, 0x00000000 },
    { &GUID_Key, 0x0015, 0x8000150c, 0x00000000 },
    { &GUID_Key, 0x0016, 0x8000160c, 0x00000000 },
    { &GUID_Key, 0x0017, 0x8000170c, 0x00000000 },
    { &GUID_Key, 0x0018, 0x8000180c, 0x00000000 },
    { &GUID_Key, 0x0019, 0x8000190c, 0x00000000 },
    { &GUID_Key, 0x001a, 0x80001a0c, 0x00000000 },
    { &GUID_Key, 0x001b, 0x80001b0c, 0x00000000 },
    { &GUID_Key, 0x001c, 0x80001c0c, 0x00000000 },
    { &GUID_Key, 0x001d, 0x80001d0c, 0x00000000 },
    { &GUID_Key, 0x001e, 0x80001e0c, 0x00000000 },
    { &GUID_Key, 0x001f, 0x80001f0c, 0x00000000 },
    { &GUID_Key, 0x0020, 0x8000200c, 0x00000000 },
    { &GUID_Key, 0x0021, 0x8000210c, 0x00000000 },
    { &GUID_Key, 0x0022, 0x8000220c, 0x00000000 },
    { &GUID_Key, 0x0023, 0x8000230c, 0x00000000 },
    { &GUID_Key, 0x0024, 0x8000240c, 0x00000000 },
    { &GUID_Key, 0x0025, 0x8000250c, 0x00000000 },
    { &GUID_Key, 0x0026, 0x8000260c, 0x00000000 },
    { &GUID_Key, 0x0027, 0x8000270c, 0x00000000 },
    { &GUID_Key, 0x0028, 0x8000280c, 0x00000000 },
    { &GUID_Key, 0x0029, 0x8000290c, 0x00000000 },
    { &GUID_Key, 0x002a, 0x80002a0c, 0x00000000 },
    { &GUID_Key, 0x002b, 0x80002b0c, 0x00000000 },
    { &GUID_Key, 0x002c, 0x80002c0c, 0x00000000 },
    { &GUID_Key, 0x002d, 0x80002d0c, 0x00000000 },
    { &GUID_Key, 0x002e, 0x80002e0c, 0x00000000 },
    { &GUID_Key, 0x002f, 0x80002f0c, 0x00000000 },
    { &GUID_Key, 0x0030, 0x8000300c, 0x00000000 },
    { &GUID_Key, 0x0031, 0x8000310c, 0x00000000 },
    { &GUID_Key, 0x0032, 0x8000320c, 0x00000000 },
    { &GUID_Key, 0x0033, 0x8000330c, 0x00000000 },
    { &GUID_Key, 0x0034, 0x8000340c, 0x00000000 },
    { &GUID_Key, 0x0035, 0x8000350c, 0x00000000 },
    { &GUID_Key, 0x0036, 0x8000360c, 0x00000000 },
    { &GUID_Key, 0x0037, 0x8000370c, 0x00000000 },
    { &GUID_Key, 0x0038, 0x8000380c, 0x00000000 },
    { &GUID_Key, 0x0039, 0x8000390c, 0x00000000 },
    { &GUID_Key, 0x003a, 0x80003a0c, 0x00000000 },
    { &GUID_Key, 0x003b, 0x80003b0c, 0x00000000 },
    { &GUID_Key, 0x003c, 0x80003c0c, 0x00000000 },
    { &GUID_Key, 0x003d, 0x80003d0c, 0x00000000 },
    { &GUID_Key, 0x003e, 0x80003e0c, 0x00000000 },
    { &GUID_Key, 0x003f, 0x80003f0c, 0x00000000 },
    { &GUID_Key, 0x0040, 0x8000400c, 0x00000000 },
    { &GUID_Key, 0x0041, 0x8000410c, 0x00000000 },
    { &GUID_Key, 0x0042, 0x8000420c, 0x00000000 },
    { &GUID_Key, 0x0043, 0x8000430c, 0x00000000 },
    { &GUID_Key, 0x0044, 0x8000440c, 0x00000000 },
    { &GUID_Key, 0x0045, 0x8000450c, 0x00000000 },
    { &GUID_Key, 0x0046, 0x8000460c, 0x00000000 },
    { &GUID_Key, 0x0047, 0x8000470c, 0x00000000 },
    { &GUID_Key, 0x0048, 0x8000480c, 0x00000000 },
    { &GUID_Key, 0x0049, 0x8000490c, 0x00000000 },
    { &GUID_Key, 0x004a, 0x80004a0c, 0x00000000 },
    { &GUID_Key, 0x004b, 0x80004b0c, 0x00000000 },
    { &GUID_Key, 0x004c, 0x80004c0c, 0x00000000 },
    { &GUID_Key, 0x004d, 0x80004d0c, 0x00000000 },
    { &GUID_Key, 0x004e, 0x80004e0c, 0x00000000 },
    { &GUID_Key, 0x004f, 0x80004f0c, 0x00000000 },
    { &GUID_Key, 0x0050, 0x8000500c, 0x00000000 },
    { &GUID_Key, 0x0051, 0x8000510c, 0x00000000 },
    { &GUID_Key, 0x0052, 0x8000520c, 0x00000000 },
    { &GUID_Key, 0x0053, 0x8000530c, 0x00000000 },
    { &GUID_Key, 0x0054, 0x8000540c, 0x00000000 },
    { &GUID_Key, 0x0055, 0x8000550c, 0x00000000 },
    { &GUID_Key, 0x0056, 0x8000560c, 0x00000000 },
    { &GUID_Key, 0x0057, 0x8000570c, 0x00000000 },
    { &GUID_Key, 0x0058, 0x8000580c, 0x00000000 },
    { &GUID_Key, 0x0059, 0x8000590c, 0x00000000 },
    { &GUID_Key, 0x005a, 0x80005a0c, 0x00000000 },
    { &GUID_Key, 0x005b, 0x80005b0c, 0x00000000 },
    { &GUID_Key, 0x005c, 0x80005c0c, 0x00000000 },
    { &GUID_Key, 0x005d, 0x80005d0c, 0x00000000 },
    { &GUID_Key, 0x005e, 0x80005e0c, 0x00000000 },
    { &GUID_Key, 0x005f, 0x80005f0c, 0x00000000 },
    { &GUID_Key, 0x0060, 0x8000600c, 0x00000000 },
    { &GUID_Key, 0x0061, 0x8000610c, 0x00000000 },
    { &GUID_Key, 0x0062, 0x8000620c, 0x00000000 },
    { &GUID_Key, 0x0063, 0x8000630c, 0x00000000 },
    { &GUID_Key, 0x0064, 0x8000640c, 0x00000000 },
    { &GUID_Key, 0x0065, 0x8000650c, 0x00000000 },
    { &GUID_Key, 0x0066, 0x8000660c, 0x00000000 },
    { &GUID_Key, 0x0067, 0x8000670c, 0x00000000 },
    { &GUID_Key, 0x0068, 0x8000680c, 0x00000000 },
    { &GUID_Key, 0x0069, 0x8000690c, 0x00000000 },
    { &GUID_Key, 0x006a, 0x80006a0c, 0x00000000 },
    { &GUID_Key, 0x006b, 0x80006b0c, 0x00000000 },
    { &GUID_Key, 0x006c, 0x80006c0c, 0x00000000 },
    { &GUID_Key, 0x006d, 0x80006d0c, 0x00000000 },
    { &GUID_Key, 0x006e, 0x80006e0c, 0x00000000 },
    { &GUID_Key, 0x006f, 0x80006f0c, 0x00000000 },
    { &GUID_Key, 0x0070, 0x8000700c, 0x00000000 },
    { &GUID_Key, 0x0071, 0x8000710c, 0x00000000 },
    { &GUID_Key, 0x0072, 0x8000720c, 0x00000000 },
    { &GUID_Key, 0x0073, 0x8000730c, 0x00000000 },
    { &GUID_Key, 0x0074, 0x8000740c, 0x00000000 },
    { &GUID_Key, 0x0075, 0x8000750c, 0x00000000 },
    { &GUID_Key, 0x0076, 0x8000760c, 0x00000000 },
    { &GUID_Key, 0x0077, 0x8000770c, 0x00000000 },
    { &GUID_Key, 0x0078, 0x8000780c, 0x00000000 },
    { &GUID_Key, 0x0079, 0x8000790c, 0x00000000 },
    { &GUID_Key, 0x007a, 0x80007a0c, 0x00000000 },
    { &GUID_Key, 0x007b, 0x80007b0c, 0x00000000 },
    { &GUID_Key, 0x007c, 0x80007c0c, 0x00000000 },
    { &GUID_Key, 0x007d, 0x80007d0c, 0x00000000 },
    { &GUID_Key, 0x007e, 0x80007e0c, 0x00000000 },
    { &GUID_Key, 0x007f, 0x80007f0c, 0x00000000 },
    { &GUID_Key, 0x0080, 0x8000800c, 0x00000000 },
    { &GUID_Key, 0x0081, 0x8000810c, 0x00000000 },
    { &GUID_Key, 0x0082, 0x8000820c, 0x00000000 },
    { &GUID_Key, 0x0083, 0x8000830c, 0x00000000 },
    { &GUID_Key, 0x0084, 0x8000840c, 0x00000000 },
    { &GUID_Key, 0x0085, 0x8000850c, 0x00000000 },
    { &GUID_Key, 0x0086, 0x8000860c, 0x00000000 },
    { &GUID_Key, 0x0087, 0x8000870c, 0x00000000 },
    { &GUID_Key, 0x0088, 0x8000880c, 0x00000000 },
    { &GUID_Key, 0x0089, 0x8000890c, 0x00000000 },
    { &GUID_Key, 0x008a, 0x80008a0c, 0x00000000 },
    { &GUID_Key, 0x008b, 0x80008b0c, 0x00000000 },
    { &GUID_Key, 0x008c, 0x80008c0c, 0x00000000 },
    { &GUID_Key, 0x008d, 0x80008d0c, 0x00000000 },
    { &GUID_Key, 0x008e, 0x80008e0c, 0x00000000 },
    { &GUID_Key, 0x008f, 0x80008f0c, 0x00000000 },
    { &GUID_Key, 0x0090, 0x8000900c, 0x00000000 },
    { &GUID_Key, 0x0091, 0x8000910c, 0x00000000 },
    { &GUID_Key, 0x0092, 0x8000920c, 0x00000000 },
    { &GUID_Key, 0x0093, 0x8000930c, 0x00000000 },
    { &GUID_Key, 0x0094, 0x8000940c, 0x00000000 },
    { &GUID_Key, 0x0095, 0x8000950c, 0x00000000 },
    { &GUID_Key, 0x0096, 0x8000960c, 0x00000000 },
    { &GUID_Key, 0x0097, 0x8000970c, 0x00000000 },
    { &GUID_Key, 0x0098, 0x8000980c, 0x00000000 },
    { &GUID_Key, 0x0099, 0x8000990c, 0x00000000 },
    { &GUID_Key, 0x009a, 0x80009a0c, 0x00000000 },
    { &GUID_Key, 0x009b, 0x80009b0c, 0x00000000 },
    { &GUID_Key, 0x009c, 0x80009c0c, 0x00000000 },
    { &GUID_Key, 0x009d, 0x80009d0c, 0x00000000 },
    { &GUID_Key, 0x009e, 0x80009e0c, 0x00000000 },
    { &GUID_Key, 0x009f, 0x80009f0c, 0x00000000 },
    { &GUID_Key, 0x00a0, 0x8000a00c, 0x00000000 },
    { &GUID_Key, 0x00a1, 0x8000a10c, 0x00000000 },
    { &GUID_Key, 0x00a2, 0x8000a20c, 0x00000000 },
    { &GUID_Key, 0x00a3, 0x8000a30c, 0x00000000 },
    { &GUID_Key, 0x00a4, 0x8000a40c, 0x00000000 },
    { &GUID_Key, 0x00a5, 0x8000a50c, 0x00000000 },
    { &GUID_Key, 0x00a6, 0x8000a60c, 0x00000000 },
    { &GUID_Key, 0x00a7, 0x8000a70c, 0x00000000 },
    { &GUID_Key, 0x00a8, 0x8000a80c, 0x00000000 },
    { &GUID_Key, 0x00a9, 0x8000a90c, 0x00000000 },
    { &GUID_Key, 0x00aa, 0x8000aa0c, 0x00000000 },
    { &GUID_Key, 0x00ab, 0x8000ab0c, 0x00000000 },
    { &GUID_Key, 0x00ac, 0x8000ac0c, 0x00000000 },
    { &GUID_Key, 0x00ad, 0x8000ad0c, 0x00000000 },
    { &GUID_Key, 0x00ae, 0x8000ae0c, 0x00000000 },
    { &GUID_Key, 0x00af, 0x8000af0c, 0x00000000 },
    { &GUID_Key, 0x00b0, 0x8000b00c, 0x00000000 },
    { &GUID_Key, 0x00b1, 0x8000b10c, 0x00000000 },
    { &GUID_Key, 0x00b2, 0x8000b20c, 0x00000000 },
    { &GUID_Key, 0x00b3, 0x8000b30c, 0x00000000 },
    { &GUID_Key, 0x00b4, 0x8000b40c, 0x00000000 },
    { &GUID_Key, 0x00b5, 0x8000b50c, 0x00000000 },
    { &GUID_Key, 0x00b6, 0x8000b60c, 0x00000000 },
    { &GUID_Key, 0x00b7, 0x8000b70c, 0x00000000 },
    { &GUID_Key, 0x00b8, 0x8000b80c, 0x00000000 },
    { &GUID_Key, 0x00b9, 0x8000b90c, 0x00000000 },
    { &GUID_Key, 0x00ba, 0x8000ba0c, 0x00000000 },
    { &GUID_Key, 0x00bb, 0x8000bb0c, 0x00000000 },
    { &GUID_Key, 0x00bc, 0x8000bc0c, 0x00000000 },
    { &GUID_Key, 0x00bd, 0x8000bd0c, 0x00000000 },
    { &GUID_Key, 0x00be, 0x8000be0c, 0x00000000 },
    { &GUID_Key, 0x00bf, 0x8000bf0c, 0x00000000 },
    { &GUID_Key, 0x00c0, 0x8000c00c, 0x00000000 },
    { &GUID_Key, 0x00c1, 0x8000c10c, 0x00000000 },
    { &GUID_Key, 0x00c2, 0x8000c20c, 0x00000000 },
    { &GUID_Key, 0x00c3, 0x8000c30c, 0x00000000 },
    { &GUID_Key, 0x00c4, 0x8000c40c, 0x00000000 },
    { &GUID_Key, 0x00c5, 0x8000c50c, 0x00000000 },
    { &GUID_Key, 0x00c6, 0x8000c60c, 0x00000000 },
    { &GUID_Key, 0x00c7, 0x8000c70c, 0x00000000 },
    { &GUID_Key, 0x00c8, 0x8000c80c, 0x00000000 },
    { &GUID_Key, 0x00c9, 0x8000c90c, 0x00000000 },
    { &GUID_Key, 0x00ca, 0x8000ca0c, 0x00000000 },
    { &GUID_Key, 0x00cb, 0x8000cb0c, 0x00000000 },
    { &GUID_Key, 0x00cc, 0x8000cc0c, 0x00000000 },
    { &GUID_Key, 0x00cd, 0x8000cd0c, 0x00000000 },
    { &GUID_Key, 0x00ce, 0x8000ce0c, 0x00000000 },
    { &GUID_Key, 0x00cf, 0x8000cf0c, 0x00000000 },
    { &GUID_Key, 0x00d0, 0x8000d00c, 0x00000000 },
    { &GUID_Key, 0x00d1, 0x8000d10c, 0x00000000 },
    { &GUID_Key, 0x00d2, 0x8000d20c, 0x00000000 },
    { &GUID_Key, 0x00d3, 0x8000d30c, 0x00000000 },
    { &GUID_Key, 0x00d4, 0x8000d40c, 0x00000000 },
    { &GUID_Key, 0x00d5, 0x8000d50c, 0x00000000 },
    { &GUID_Key, 0x00d6, 0x8000d60c, 0x00000000 },
    { &GUID_Key, 0x00d7, 0x8000d70c, 0x00000000 },
    { &GUID_Key, 0x00d8, 0x8000d80c, 0x00000000 },
    { &GUID_Key, 0x00d9, 0x8000d90c, 0x00000000 },
    { &GUID_Key, 0x00da, 0x8000da0c, 0x00000000 },
    { &GUID_Key, 0x00db, 0x8000db0c, 0x00000000 },
    { &GUID_Key, 0x00dc, 0x8000dc0c, 0x00000000 },
    { &GUID_Key, 0x00dd, 0x8000dd0c, 0x00000000 },
    { &GUID_Key, 0x00de, 0x8000de0c, 0x00000000 },
    { &GUID_Key, 0x00df, 0x8000df0c, 0x00000000 },
    { &GUID_Key, 0x00e0, 0x8000e00c, 0x00000000 },
    { &GUID_Key, 0x00e1, 0x8000e10c, 0x00000000 },
    { &GUID_Key, 0x00e2, 0x8000e20c, 0x00000000 },
    { &GUID_Key, 0x00e3, 0x8000e30c, 0x00000000 },
    { &GUID_Key, 0x00e4, 0x8000e40c, 0x00000000 },
    { &GUID_Key, 0x00e5, 0x8000e50c, 0x00000000 },
    { &GUID_Key, 0x00e6, 0x8000e60c, 0x00000000 },
    { &GUID_Key, 0x00e7, 0x8000e70c, 0x00000000 },
    { &GUID_Key, 0x00e8, 0x8000e80c, 0x00000000 },
    { &GUID_Key, 0x00e9, 0x8000e90c, 0x00000000 },
    { &GUID_Key, 0x00ea, 0x8000ea0c, 0x00000000 },
    { &GUID_Key, 0x00eb, 0x8000eb0c, 0x00000000 },
    { &GUID_Key, 0x00ec, 0x8000ec0c, 0x00000000 },
    { &GUID_Key, 0x00ed, 0x8000ed0c, 0x00000000 },
    { &GUID_Key, 0x00ee, 0x8000ee0c, 0x00000000 },
    { &GUID_Key, 0x00ef, 0x8000ef0c, 0x00000000 },
    { &GUID_Key, 0x00f0, 0x8000f00c, 0x00000000 },
    { &GUID_Key, 0x00f1, 0x8000f10c, 0x00000000 },
    { &GUID_Key, 0x00f2, 0x8000f20c, 0x00000000 },
    { &GUID_Key, 0x00f3, 0x8000f30c, 0x00000000 },
    { &GUID_Key, 0x00f4, 0x8000f40c, 0x00000000 },
    { &GUID_Key, 0x00f5, 0x8000f50c, 0x00000000 },
    { &GUID_Key, 0x00f6, 0x8000f60c, 0x00000000 },
    { &GUID_Key, 0x00f7, 0x8000f70c, 0x00000000 },
    { &GUID_Key, 0x00f8, 0x8000f80c, 0x00000000 },
    { &GUID_Key, 0x00f9, 0x8000f90c, 0x00000000 },
    { &GUID_Key, 0x00fa, 0x8000fa0c, 0x00000000 },
    { &GUID_Key, 0x00fb, 0x8000fb0c, 0x00000000 },
    { &GUID_Key, 0x00fc, 0x8000fc0c, 0x00000000 },
    { &GUID_Key, 0x00fd, 0x8000fd0c, 0x00000000 },
    { &GUID_Key, 0x00fe, 0x8000fe0c, 0x00000000 },
    { &GUID_Key, 0x00ff, 0x8000ff0c, 0x00000000 },
};

// GLOBAL: CMR2 0x004c5c30
const DIOBJECTDATAFORMAT g_dataFormatJoystick2[164] = {
    { &GUID_XAxis, 0x0000, 0x80ffff03, 0x00000100 },
    { &GUID_YAxis, 0x0004, 0x80ffff03, 0x00000100 },
    { &GUID_ZAxis, 0x0008, 0x80ffff03, 0x00000100 },
    { &GUID_RxAxis, 0x000c, 0x80ffff03, 0x00000100 },
    { &GUID_RyAxis, 0x0010, 0x80ffff03, 0x00000100 },
    { &GUID_RzAxis, 0x0014, 0x80ffff03, 0x00000100 },
    { &GUID_Slider, 0x0018, 0x80ffff03, 0x00000100 },
    { &GUID_Slider, 0x001c, 0x80ffff03, 0x00000100 },
    { &GUID_POV, 0x0020, 0x80ffff10, 0x00000000 },
    { &GUID_POV, 0x0024, 0x80ffff10, 0x00000000 },
    { &GUID_POV, 0x0028, 0x80ffff10, 0x00000000 },
    { &GUID_POV, 0x002c, 0x80ffff10, 0x00000000 },
    { NULL, 0x0030, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0031, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0032, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0033, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0034, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0035, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0036, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0037, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0038, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0039, 0x80ffff0c, 0x00000000 },
    { NULL, 0x003a, 0x80ffff0c, 0x00000000 },
    { NULL, 0x003b, 0x80ffff0c, 0x00000000 },
    { NULL, 0x003c, 0x80ffff0c, 0x00000000 },
    { NULL, 0x003d, 0x80ffff0c, 0x00000000 },
    { NULL, 0x003e, 0x80ffff0c, 0x00000000 },
    { NULL, 0x003f, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0040, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0041, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0042, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0043, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0044, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0045, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0046, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0047, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0048, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0049, 0x80ffff0c, 0x00000000 },
    { NULL, 0x004a, 0x80ffff0c, 0x00000000 },
    { NULL, 0x004b, 0x80ffff0c, 0x00000000 },
    { NULL, 0x004c, 0x80ffff0c, 0x00000000 },
    { NULL, 0x004d, 0x80ffff0c, 0x00000000 },
    { NULL, 0x004e, 0x80ffff0c, 0x00000000 },
    { NULL, 0x004f, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0050, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0051, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0052, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0053, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0054, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0055, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0056, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0057, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0058, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0059, 0x80ffff0c, 0x00000000 },
    { NULL, 0x005a, 0x80ffff0c, 0x00000000 },
    { NULL, 0x005b, 0x80ffff0c, 0x00000000 },
    { NULL, 0x005c, 0x80ffff0c, 0x00000000 },
    { NULL, 0x005d, 0x80ffff0c, 0x00000000 },
    { NULL, 0x005e, 0x80ffff0c, 0x00000000 },
    { NULL, 0x005f, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0060, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0061, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0062, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0063, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0064, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0065, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0066, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0067, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0068, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0069, 0x80ffff0c, 0x00000000 },
    { NULL, 0x006a, 0x80ffff0c, 0x00000000 },
    { NULL, 0x006b, 0x80ffff0c, 0x00000000 },
    { NULL, 0x006c, 0x80ffff0c, 0x00000000 },
    { NULL, 0x006d, 0x80ffff0c, 0x00000000 },
    { NULL, 0x006e, 0x80ffff0c, 0x00000000 },
    { NULL, 0x006f, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0070, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0071, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0072, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0073, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0074, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0075, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0076, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0077, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0078, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0079, 0x80ffff0c, 0x00000000 },
    { NULL, 0x007a, 0x80ffff0c, 0x00000000 },
    { NULL, 0x007b, 0x80ffff0c, 0x00000000 },
    { NULL, 0x007c, 0x80ffff0c, 0x00000000 },
    { NULL, 0x007d, 0x80ffff0c, 0x00000000 },
    { NULL, 0x007e, 0x80ffff0c, 0x00000000 },
    { NULL, 0x007f, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0080, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0081, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0082, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0083, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0084, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0085, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0086, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0087, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0088, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0089, 0x80ffff0c, 0x00000000 },
    { NULL, 0x008a, 0x80ffff0c, 0x00000000 },
    { NULL, 0x008b, 0x80ffff0c, 0x00000000 },
    { NULL, 0x008c, 0x80ffff0c, 0x00000000 },
    { NULL, 0x008d, 0x80ffff0c, 0x00000000 },
    { NULL, 0x008e, 0x80ffff0c, 0x00000000 },
    { NULL, 0x008f, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0090, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0091, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0092, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0093, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0094, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0095, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0096, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0097, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0098, 0x80ffff0c, 0x00000000 },
    { NULL, 0x0099, 0x80ffff0c, 0x00000000 },
    { NULL, 0x009a, 0x80ffff0c, 0x00000000 },
    { NULL, 0x009b, 0x80ffff0c, 0x00000000 },
    { NULL, 0x009c, 0x80ffff0c, 0x00000000 },
    { NULL, 0x009d, 0x80ffff0c, 0x00000000 },
    { NULL, 0x009e, 0x80ffff0c, 0x00000000 },
    { NULL, 0x009f, 0x80ffff0c, 0x00000000 },
    { NULL, 0x00a0, 0x80ffff0c, 0x00000000 },
    { NULL, 0x00a1, 0x80ffff0c, 0x00000000 },
    { NULL, 0x00a2, 0x80ffff0c, 0x00000000 },
    { NULL, 0x00a3, 0x80ffff0c, 0x00000000 },
    { NULL, 0x00a4, 0x80ffff0c, 0x00000000 },
    { NULL, 0x00a5, 0x80ffff0c, 0x00000000 },
    { NULL, 0x00a6, 0x80ffff0c, 0x00000000 },
    { NULL, 0x00a7, 0x80ffff0c, 0x00000000 },
    { NULL, 0x00a8, 0x80ffff0c, 0x00000000 },
    { NULL, 0x00a9, 0x80ffff0c, 0x00000000 },
    { NULL, 0x00aa, 0x80ffff0c, 0x00000000 },
    { NULL, 0x00ab, 0x80ffff0c, 0x00000000 },
    { NULL, 0x00ac, 0x80ffff0c, 0x00000000 },
    { NULL, 0x00ad, 0x80ffff0c, 0x00000000 },
    { NULL, 0x00ae, 0x80ffff0c, 0x00000000 },
    { NULL, 0x00af, 0x80ffff0c, 0x00000000 },
    { &GUID_XAxis, 0x00b0, 0x80ffff03, 0x00000200 },
    { &GUID_YAxis, 0x00b4, 0x80ffff03, 0x00000200 },
    { &GUID_ZAxis, 0x00b8, 0x80ffff03, 0x00000200 },
    { &GUID_RxAxis, 0x00bc, 0x80ffff03, 0x00000200 },
    { &GUID_RyAxis, 0x00c0, 0x80ffff03, 0x00000200 },
    { &GUID_RzAxis, 0x00c4, 0x80ffff03, 0x00000200 },
    { &GUID_Slider, 0x0018, 0x80ffff03, 0x00000200 },
    { &GUID_Slider, 0x001c, 0x80ffff03, 0x00000200 },
    { &GUID_XAxis, 0x00d0, 0x80ffff03, 0x00000300 },
    { &GUID_YAxis, 0x00d4, 0x80ffff03, 0x00000300 },
    { &GUID_ZAxis, 0x00d8, 0x80ffff03, 0x00000300 },
    { &GUID_RxAxis, 0x00dc, 0x80ffff03, 0x00000300 },
    { &GUID_RyAxis, 0x00e0, 0x80ffff03, 0x00000300 },
    { &GUID_RzAxis, 0x00e4, 0x80ffff03, 0x00000300 },
    { &GUID_Slider, 0x0018, 0x80ffff03, 0x00000300 },
    { &GUID_Slider, 0x001c, 0x80ffff03, 0x00000300 },
    { &GUID_XAxis, 0x00f0, 0x80ffff03, 0x00000400 },
    { &GUID_YAxis, 0x00f4, 0x80ffff03, 0x00000400 },
    { &GUID_ZAxis, 0x00f8, 0x80ffff03, 0x00000400 },
    { &GUID_RxAxis, 0x00fc, 0x80ffff03, 0x00000400 },
    { &GUID_RyAxis, 0x0100, 0x80ffff03, 0x00000400 },
    { &GUID_RzAxis, 0x0104, 0x80ffff03, 0x00000400 },
    { &GUID_Slider, 0x0018, 0x80ffff03, 0x00000400 },
    { &GUID_Slider, 0x001c, 0x80ffff03, 0x00000400 },
};

extern "C" const DIDATAFORMAT c_dfDIMouse2 = {
    sizeof(DIDATAFORMAT), sizeof(DIOBJECTDATAFORMAT), DIDF_RELAXIS, sizeof(DIMOUSESTATE2), 11,
    (LPDIOBJECTDATAFORMAT)g_dataFormatMouse2
};

extern "C" const DIDATAFORMAT c_dfDIKeyboard = {
    sizeof(DIDATAFORMAT), sizeof(DIOBJECTDATAFORMAT), DIDF_RELAXIS, 0x100, 0x100,
    (LPDIOBJECTDATAFORMAT)g_dataFormatKeyboard
};

extern "C" const DIDATAFORMAT c_dfDIJoystick2 = {
    sizeof(DIDATAFORMAT), sizeof(DIOBJECTDATAFORMAT), DIDF_ABSAXIS, sizeof(DIJOYSTATE2), 0xa4,
    (LPDIOBJECTDATAFORMAT)g_dataFormatJoystick2
};

// GLOBAL: CMR2 0x00511758
// IID_IDirectInput7A

// GLOBAL: CMR2 0x0059f8c8
IDirectInput7A* CInput::m_lpDirectInput7;

// GLOBAL: CMR2 0x00520874
char CInput::m_strKeyboard[12] = "Keyboard";

// GLOBAL: CMR2 0x0059f8cc
Unk0x0059f8cc CInput::m_unk0x0059f8cc; // device count or something

// GLOBAL: CMR2 0x0059ce48
InputDeviceState CInput::m_deviceState;

// GLOBAL: CMR2 0x0059f8d4
DWORD CInput::m_mouseGranularity;

// GLOBAL: CMR2 0x0059f8d8
PVOID CInput::m_keyboardDelay;

// GLOBAL: CMR2 0x0059f8dc
PVOID CInput::m_keyboardSpeed;


// GLOBAL: CMR2 0x0059f7c4
LPDIRECTINPUTDEVICEA CInput::m_pDirectInputMouse = NULL;

// GLOBAL: CMR2 0x00511400
USHORT CInput::m_unk0x00511400[8] = {0, 4, 8, 12, 16, 20, 24, 28};

// GLOBAL: CMR2 0x0059f6b0
LPDIRECTINPUTDEVICEA CInput::m_unk0x0059f6b0[8];

// GLOBAL: CMR2 0x00520880
CHAR CInput::m_strD[4] = " D";
// GLOBAL: CMR2 0x00520884
CHAR CInput::m_strU[4] = " U";
// GLOBAL: CMR2 0x00520888
CHAR CInput::m_strR[4] = " R";
// GLOBAL: CMR2 0x0052088C
CHAR CInput::m_strL[4] = " L";

// GLOBAL: CMR2 0x00666d28
InputFeedbackState CInput::m_feedbackState;

// GLOBAL: CMR2 0x00666ee8
BOOL CInput::m_unk0x00666ee8 = FALSE;

char CInput::m_formatBuffer[512];
BYTE CInput::m_keyboardState[256];
DWORD CInput::m_buttonMasks[24] = {
    0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000,
    0x10000, 0x20000, 0x40000, 0x80000, 0x100000, 0x200000, 0x400000, 0x800000, 0x1, 0x2, 0x4, 0x8
};
unsigned int CInput::m_directionButtonMask = 0xf;
BOOL CInput::m_unk0x0052086c = TRUE;
DIEFFECT CInput::m_forceFeedbackEffects[80];
DICONDITION CInput::m_forceFeedbackConditions[80];
DWORD CInput::m_unk0x0059f8f0;
DWORD CInput::m_unk0x0059f8f4;
DWORD CInput::m_unk0x0059f8f8;
DWORD CInput::m_unk0x0059f900;
DWORD CInput::m_unk0x0059f90c;
DWORD CInput::m_unk0x0059f910;

unsigned short CInput::m_controllerCount = 1;
char CInput::m_strControllerInfoDir[32] = "%s\\Configuration\\Controller.rcf";
BOOL CInput::m_hasLoadedControllerInfo;
ControllerData CInput::m_controllerInfo[6];
unsigned short CInput::m_unk0x005168f4[8] = {0, 1, 2, 3, 0, 0, 0, 0};

// FUNCTION: CMR2 0x0049fd30
BOOL CInput::DInputCreate(void) {
    DirectInputCreateEx(CMain::m_hInstance, 0x700, IID_IDirectInput7A, (LPVOID*)&CInput::m_lpDirectInput7, NULL);
    CGame::RegisterCallback(DInputRelease, NULL);
    return TRUE;
}

// FUNCTION: CMR2 0x0049fe30
LPDIRECTINPUTDEVICEA CInput::DInputCreateDevice(REFGUID guid, LPCDIDATAFORMAT pDataFormat) {
    LPDIRECTINPUTDEVICEA pDevice;
    LPDIRECTINPUTDEVICE7A pOtherDevice = NULL;
    HRESULT h1, h2, h3, h4;
    ULONG refCount;
    
    h1 = m_lpDirectInput7->CreateDevice(guid, &pDevice, NULL);
    if (SUCCEEDED(h1)) {
        h2 = pDevice->QueryInterface(IID_IDirectInputDevice7A, (LPVOID*)&pOtherDevice);
        if (pDevice != NULL) {
            refCount = pDevice->Release();
            if (refCount == 0)
                pDevice = NULL;
        }
      
        if (SUCCEEDED(h2)) {
            h3 = pOtherDevice->SetDataFormat(pDataFormat);
            if (FAILED(h3)) {
                if (pOtherDevice != NULL) pOtherDevice->Release();
                return NULL;
            }
            
            if (IsEqualGUID(guid, GUID_SysKeyboard)) {
                h4 = pOtherDevice->SetCooperativeLevel(CMain::m_hWndList[CMain::m_hWndIx], DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);  // 10
            } else if (IsEqualGUID(guid, GUID_SysMouse)) {
                if (g_pGraphics->isFullscreen) {
                    h4 = pOtherDevice->SetCooperativeLevel(CMain::m_hWndList[CMain::m_hWndIx], DISCL_EXCLUSIVE | DISCL_FOREGROUND);  // 5
                } else {
                    h4 = pOtherDevice->SetCooperativeLevel(CMain::m_hWndList[CMain::m_hWndIx], DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);  // 6
                }
            } else {
                h4 = pOtherDevice->SetCooperativeLevel(CMain::m_hWndList[CMain::m_hWndIx], 9);
            }
            
            // if we got a cooplevel success, return it
            if (FAILED(h4)) {
                if (pOtherDevice != NULL) pOtherDevice->Release();
                return NULL;
            }

            return pOtherDevice;
        }
    }
    return NULL;
}

// FUNCTION: CMR2 0x0049fd60
BOOL CInput::DInputRelease(void) {
    DInputReleaseDevices();

    if (m_lpDirectInput7 != NULL) {
        ULONG result = m_lpDirectInput7->Release();
        if (result == 0)
            m_lpDirectInput7 = NULL;
    }

    return TRUE;
};

// FUNCTION: CMR2 0x0049fd90
void CInput::DInputReleaseDevices(void) {
    HRESULT hr;
    ULONG result;
    int iVar2 = 0;

    if (m_pDirectInputKeyboard != NULL) {
        m_pDirectInputKeyboard->Unacquire();
        if (m_pDirectInputKeyboard != NULL) {
            result = m_pDirectInputKeyboard->Release();
            if (result == 0) m_pDirectInputKeyboard = NULL;
        }
    }

    if (m_pDirectInputMouse != NULL) {
        m_pDirectInputMouse->Unacquire();
        if (m_pDirectInputMouse != NULL) {
            result = m_pDirectInputMouse->Release();
            if (result == 0) m_pDirectInputMouse = NULL;
        }
    }

    CountAttachedInputDevices();
    
    iVar2 = 0;
    LPDIRECTINPUTDEVICEA *pDevices = m_unk0x0059f6b0;

    do {
        if (*pDevices != NULL) {
            hr = (*pDevices)->Unacquire();
            if (SUCCEEDED(hr) && iVar2 < m_unk0x0059f8cc.field_0x3) {
                if (*pDevices != NULL) {
                    result = (*pDevices)->Release();
                    if (result == 0)
                        *pDevices = NULL;
                }
            }
        }

        pDevices++;
        iVar2++;
    } while ((int)pDevices < (int)(m_unk0x0059f6b0 + 4));
}

// FUNCTION: CMR2 0x0049ef90
int CInput::CountAttachedInputDevices(void) {
  m_unk0x0059f8cc.field_0x3 = 0;
  m_lpDirectInput7->EnumDevices(DIDEVTYPE_JOYSTICK, CountJoystickEnumerationCallback, NULL, DIEDFL_ATTACHEDONLY);
  return m_unk0x0059f8cc.field_0x1 + m_unk0x0059f8cc.field_0x3;
}

// FUNCTION: CMR2 0x0049f6b0
BOOL CInput::CountJoystickEnumerationCallback(LPCDIDEVICEINSTANCEA lpddi, LPVOID pvRef) {
    m_unk0x0059f8cc.field_0x3++;
    return TRUE;
}

// match 89%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0049f0e0
BOOL CInput::SetupKeyboard(void) {
    unsigned int uVar1;
    BOOL bVar2;
    int iVar3 = 0;

    do {
        uVar1 = m_unk0x0059f8cc.field_0x0;
        m_availableDevices[uVar1].field_0x0 = 1;
        m_availableDevices[uVar1].field_0x18 = FALSE;

        ((int (__cdecl *)(char *, const char *))sprintf)(m_availableDevices[uVar1].deviceInstanceName, m_strKeyboard);
        ((int (__cdecl *)(char *, const char *))sprintf)(m_availableDevices[m_unk0x0059f8cc.field_0x0].deviceProductName, m_strKeyboard);
        
        uVar1 = m_unk0x0059f8cc.field_0x0;
        m_availableDevices[uVar1].unk_isJoystick = FALSE;

        if (!iVar3) {
            m_availableDevices[uVar1].keyboard.field_0x468 = 0xcb;
            m_availableDevices[uVar1].keyboard.field_0x469 = 0xcd;
            m_availableDevices[uVar1].keyboard.field_0x46a = 0xc8;
            m_availableDevices[uVar1].keyboard.field_0x46b = 0xd0;
            m_availableDevices[uVar1].keyboard.field_0x46c = 0x39;
            m_availableDevices[uVar1].keyboard.field_0x46d = 0x1b;
            m_availableDevices[uVar1].keyboard.field_0x46e = 0x1a;
            m_availableDevices[uVar1].keyboard.field_0x46f = 0x2e;
            m_availableDevices[uVar1].keyboard.field_0x470 = 0x13;
        } else {
            m_availableDevices[uVar1].keyboard.field_0x468 = 0x4b;
            m_availableDevices[uVar1].keyboard.field_0x469 = 0x4d;
            m_availableDevices[uVar1].keyboard.field_0x46a = 0x48;
            m_availableDevices[uVar1].keyboard.field_0x46b = 0x50;
            m_availableDevices[uVar1].keyboard.field_0x46c = 0x51;
            m_availableDevices[uVar1].keyboard.field_0x46d = 0xc9;
            m_availableDevices[uVar1].keyboard.field_0x46e = 0xd1;
            m_availableDevices[uVar1].keyboard.field_0x46f = 0x49;
            m_availableDevices[uVar1].keyboard.field_0x470 = 0x47;            
        }

        m_availableDevices[uVar1].keyboard.field_0x471 = 0x0;
        m_availableDevices[uVar1].keyboard.field_0x474 = 0x3b;
        m_availableDevices[uVar1].keyboard.field_0x475 = 0x3c;
        m_availableDevices[uVar1].keyboard.field_0x476 = 0x3e;
        m_availableDevices[uVar1].keyboard.field_0x477 = 0x0;
        m_availableDevices[uVar1].keyboard.field_0x472 = 0x1;
        m_availableDevices[uVar1].keyboard.field_0x473 = 0x0;

        m_unk0x0059f8cc.field_0x0 = m_unk0x0059f8cc.field_0x0 + 1;
        iVar3++;
    } while (iVar3 < 2);

    bVar2 = SystemParametersInfoA(SPI_GETKEYBOARDDELAY, 0, &m_keyboardDelay, 0);
    if (!bVar2) m_keyboardDelay = (PVOID)0x3e8;
    else {
        switch ((int)m_keyboardDelay) {
            case 0:
                m_keyboardDelay = (PVOID)0x1f4;
            break;

            case 1:
                m_keyboardDelay = (PVOID)0x2ee;
            break;

            case 2:
                m_keyboardDelay = (PVOID)0x3e8;
            break;

            case 3:
                m_keyboardDelay = (PVOID)0x4e2;
            break;
        }
    }

    bVar2 = SystemParametersInfoA(SPI_GETKEYBOARDSPEED, 0, &m_keyboardSpeed, 0);
    if (!bVar2) m_keyboardSpeed = (PVOID)0x1f4;
    else m_keyboardSpeed = (PVOID)((int)m_keyboardSpeed * -0xd + 0x1f7);

    m_pDirectInputKeyboard = DInputCreateDevice(GUID_SysKeyboard, &c_dfDIKeyboard);
    if (m_pDirectInputKeyboard != NULL)
        if (SUCCEEDED(m_pDirectInputKeyboard->Acquire()))
            return TRUE;

    return FALSE;
}

// FUNCTION: CMR2 0x0049f060
void CInput::SetupMouse(void) {
    HRESULT hr;
    DIPROPDWORD diPropDword;
    m_pDirectInputMouse = DInputCreateDevice(GUID_SysMouse, &c_dfDIMouse2);

    diPropDword.diph.dwSize = 0x14;
    diPropDword.diph.dwHeaderSize = 0x10;
    diPropDword.diph.dwObj = 0;
    diPropDword.diph.dwHow = 0;
    diPropDword.dwData = 0x10;

    m_pDirectInputMouse->SetProperty(DIPROP_BUFFERSIZE, &diPropDword.diph);
    m_pDirectInputMouse->Acquire();
    hr = m_pDirectInputMouse->GetProperty(DIPROP_GRANULARITY, &diPropDword.diph);
    if (SUCCEEDED(hr)) {
        m_mouseGranularity = diPropDword.dwData;
    }

    SetMouseCoopLevel(1);
}

// FUNCTION: CMR2 0x0049f000
void CInput::SetMouseCoopLevel(BOOL param1) {
    if (m_pDirectInputMouse != NULL) {
        m_pDirectInputMouse->Unacquire();

        if (param1 && g_pGraphics->isFullscreen) {
            m_pDirectInputMouse->SetCooperativeLevel(CMain::m_hWndList[CMain::m_hWndIx],DISCL_EXCLUSIVE | DISCL_FOREGROUND); // 5
        } else {
            m_pDirectInputMouse->SetCooperativeLevel(CMain::m_hWndList[CMain::m_hWndIx], DISCL_NONEXCLUSIVE | DISCL_FOREGROUND); // 6
        }

        m_pDirectInputMouse->Acquire();
    }
}

// FUNCTION: CMR2 0x0049f690
BOOL CInput::GetAttachedJoysticks(void) {
    HRESULT hr;
    hr = m_lpDirectInput7->EnumDevices(DIDEVTYPE_JOYSTICK, SetupJoystick, m_lpDirectInput7, DIEDFL_ATTACHEDONLY);
    return SUCCEEDED(hr);
}

// match 65%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0049f6d0
BOOL CInput::SetupJoystick(LPCDIDEVICEINSTANCEA lpddi, LPVOID pvRef) {
    HRESULT hr;
    unsigned int uVar2;
    DeviceInfo *pDeviceInfo;
    LPDIRECTINPUTDEVICEA pDevice;
    BYTE iVar6[4];
    int iVar10, axisID, iVar11;
    JoystickBinding * joystickBinding;
    DIPROPDWORD dipd;
    DIDEVCAPS devCaps;
    DIDEVICEOBJECTINSTANCEA didoi;

    uVar2 = m_unk0x0059f8cc.field_0x0;
    pDeviceInfo = &m_availableDevices[m_unk0x0059f8cc.field_0x0];
    if (GET_DIDEVICE_TYPE(lpddi->dwDevType) == DIDEVTYPE_JOYSTICK) {
        if (GET_DIDEVICE_SUBTYPE(lpddi->dwDevType) != DIDEVTYPE_MOUSE) pDeviceInfo->field_0x0 = 3;
    
        pDevice = DInputCreateDevice(lpddi->guidInstance, &c_dfDIJoystick2);
        m_unk0x0059f6b0[m_unk0x0059f8cc.field_0x2] = pDevice;

        if (pDevice != NULL) {
            strncpy(m_availableDevices[uVar2].deviceInstanceName, lpddi->tszInstanceName, sizeof(m_availableDevices[uVar2].deviceInstanceName));
            strncpy(m_availableDevices[uVar2].deviceProductName, lpddi->tszProductName, sizeof(m_availableDevices[uVar2].deviceProductName));
        
            pDeviceInfo->field_0x18 = m_unk0x0059f8cc.field_0x2;

            SetupJoystickDeviceInfo(pDeviceInfo);

            iVar10 = 0;
            axisID = 0;

            if (pDeviceInfo->joystick.controlCount > 0) {
                joystickBinding = pDeviceInfo->joystick.bindings;
                do {
                    if (joystickBinding[-1].field_0x10 != FALSE) {
                        SetJoystickAxisRange(m_unk0x0059f8cc.field_0x0, axisID, joystickBinding->range);
                        SetJoystickAxisDeadzone(m_unk0x0059f8cc.field_0x0, axisID, joystickBinding->deadzone);
                        SetJoystickAxisSaturation(m_unk0x0059f8cc.field_0x0, axisID, joystickBinding->saturation);
                        iVar10++;
                    }

                    axisID++;
                    joystickBinding ++;
                } while (axisID < pDeviceInfo->field_0x8);
            }

            devCaps.dwSize = 0x2c;
            hr = m_unk0x0059f6b0[m_unk0x0059f8cc.field_0x2]->GetCapabilities(&devCaps);
            
            if (SUCCEEDED(hr)) {
                if ((devCaps.dwFlags & DIDC_FORCEFEEDBACK)) {
                    m_availableDevices[m_unk0x0059f8cc.field_0x2].unk_isJoystick = TRUE;
                    
                    dipd.diph.dwSize = 0x14;
                    dipd.diph.dwHeaderSize = 0x10;
                    dipd.diph.dwHow = DIPH_DEVICE;
                    dipd.dwData = 0;

                    
                    m_unk0x0059f6b0[m_unk0x0059f8cc.field_0x2]->SetProperty(DIPROP_AUTOCENTER, &dipd.diph);
                    InitializeForceFeedbackDevice(m_unk0x0059f8cc.field_0x0, (LPDIRECTINPUTDEVICE7)m_unk0x0059f6b0[m_unk0x0059f8cc.field_0x2]);

                } else {
                    m_availableDevices[m_unk0x0059f8cc.field_0x2].unk_isJoystick = FALSE;
                }

                iVar10 = 0;
                didoi.dwSize = 0x13c;
                m_availableDevices[uVar2].field_0x14 = 0;

                if (devCaps.dwAxes > 0) {
                    
                    do {
                        hr = m_unk0x0059f6b0[m_unk0x0059f8cc.field_0x2]->GetObjectInfo(&didoi, iVar10 + 0x30, DIPH_BYOFFSET);
                        
                        if (SUCCEEDED(hr)) {
                            strncpy(m_availableDevices[uVar2].field_0x284[iVar10], didoi.tszName, 20);
                            m_availableDevices[uVar2].field_0x14++;
                        }
                        iVar10++;
                    } while (iVar10 < devCaps.dwAxes);
                }

                if (devCaps.dwAxes > 0) {
                    iVar10 = 0;
                    didoi.dwOfs = 0x20;

                    do {
                        if (0x20 < didoi.dwOfs) break;
                        
                        hr = m_unk0x0059f6b0[m_unk0x0059f8cc.field_0x2]->GetObjectInfo(&didoi, didoi.dwOfs, DIPH_BYOFFSET);
                        if (SUCCEEDED(hr)) {
                            strncpy(m_availableDevices[uVar2].field_0x414, didoi.tszName, 0x11);
                            strcat(m_availableDevices[uVar2].field_0x414, m_strL);
                            
                            strncpy(m_availableDevices[uVar2].field_0x425, didoi.tszName, 0x11);
                            strcat(m_availableDevices[uVar2].field_0x425, m_strR);
                            
                            strncpy(m_availableDevices[uVar2].field_0x436, didoi.tszName, 0x11);
                            strcat(m_availableDevices[uVar2].field_0x436, m_strU);
                            
                            strncpy(m_availableDevices[uVar2].field_0x447, didoi.tszName, 0x11);
                            strcat(m_availableDevices[uVar2].field_0x447, m_strD);

                            m_availableDevices[uVar2].field_0x14++;
                        }

                        iVar10++;
                        didoi.dwOfs++;
                    } while (iVar10 < devCaps.dwAxes);
                }

                iVar10 = 0;

                if (m_availableDevices[uVar2].field_0x14 > 0) {
                    int * pUnk0x1c = &m_availableDevices[uVar2].field_0x1c;
                    do {
                        *pUnk0x1c++ = 1u << iVar10;
                        iVar10++;
                    } while (iVar10 < m_availableDevices[uVar2].field_0x14);
                }

                m_unk0x0059f6b0[m_unk0x0059f8cc.field_0x2]->Acquire();
                m_unk0x0059f8cc.field_0x2++;
                m_unk0x0059f8cc.field_0x0++;
            }
        }
    }

    return DIENUM_CONTINUE;
}

// FUNCTION: CMR2 0x0049fad0
void CInput::SetupJoystickDeviceInfo(DeviceInfo *deviceInfo) {
    HRESULT hr;
    USHORT * unk0x00511400;
    JoystickBinding * bindings;

    DIPROPRANGE dipd;
    dipd.diph.dwSize = 0x18;
    dipd.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipd.diph.dwHow = DIPH_BYOFFSET;

    deviceInfo->joystick.controlCount = 0;
    unk0x00511400 = m_unk0x00511400;
    bindings = deviceInfo->joystick.bindings;

    do {
        dipd.diph.dwObj = *unk0x00511400;
        hr = m_unk0x0059f6b0[deviceInfo->field_0x18]->GetProperty(DIPROP_RANGE, &dipd.diph);

        if (SUCCEEDED(hr)) {
            bindings[-1].field_0x10 = TRUE; // esentially just deviceInfo->unk_isJoystick but it doesnt match
            bindings->deadzone = 0xc8;
            bindings->range = 0x10000;
            bindings->saturation = 0x2710;

            deviceInfo->joystick.controlCount ++;
        }
        else {
            bindings[-1].field_0x10 = FALSE;
        }

        unk0x00511400++;
        bindings++;
    } while ((int)unk0x00511400 < (int)(m_unk0x00511400 + 8));
}

// FUNCTION: CMR2 0x0049ee10
void CInput::SetJoystickAxisRange(int deviceID, int axisID, DWORD range) {
    USHORT * unk0x00511400;
    DeviceInfo * pDeviceInfo = &m_availableDevices[deviceID];

    DIPROPRANGE dipd;
    dipd.diph.dwSize = 0x18;
    dipd.diph.dwHeaderSize = 0x10;
    dipd.diph.dwHow = DIPH_BYOFFSET;
    dipd.lMax = range;
    dipd.lMin = -range;
    dipd.diph.dwObj = m_unk0x00511400[axisID];

    m_unk0x0059f6b0[pDeviceInfo->field_0x18]->SetProperty(DIPROP_RANGE, &dipd.diph);
    pDeviceInfo->joystick.bindings[axisID].range = range;
}

// FUNCTION: CMR2 0x0049ee90
void CInput::SetJoystickAxisDeadzone(int deviceID, int axisID, DWORD deadzone) {
    USHORT * unk0x00511400;
    DeviceInfo * pDeviceInfo = &m_availableDevices[deviceID];

    DIPROPDWORD dipd;
    dipd.diph.dwSize = 0x14;
    dipd.diph.dwHeaderSize = 0x10;
    dipd.diph.dwHow = DIPH_BYOFFSET;
    dipd.dwData = deadzone;
    dipd.diph.dwObj = m_unk0x00511400[axisID];

    m_unk0x0059f6b0[pDeviceInfo->field_0x18]->SetProperty(DIPROP_DEADZONE, &dipd.diph);
    pDeviceInfo->joystick.bindings[axisID].deadzone = deadzone;
}

// FUNCTION: CMR2 0x0049ef10
void CInput::SetJoystickAxisSaturation(int deviceID, int axisID, DWORD saturation) {
    USHORT * unk0x00511400;
    DeviceInfo * pDeviceInfo = &m_availableDevices[deviceID];

    DIPROPDWORD dipd;
    dipd.diph.dwSize = 0x14;
    dipd.diph.dwHeaderSize = 0x10;
    dipd.diph.dwHow = DIPH_BYOFFSET;
    dipd.dwData = saturation;
    dipd.diph.dwObj = m_unk0x00511400[axisID];

    m_unk0x0059f6b0[pDeviceInfo->field_0x18]->SetProperty(DIPROP_SATURATION, &dipd.diph);
    pDeviceInfo->joystick.bindings[axisID].saturation = saturation;
}

// FUNCTION: CMR2 0x004aae20
BOOL CInput::InitializeForceFeedbackDevice(int deviceID, LPDIRECTINPUTDEVICE7 pDevice) {
    Graphics *pGraphics;
    HRESULT hr;

    pGraphics = g_pGraphics;
    if (m_forceFeedbackDevices[deviceID].field_0x0 == 0) {
        m_forceFeedbackDevices[deviceID].device = (LPDIRECTINPUTDEVICE7A)pDevice;
        if (pGraphics->isFullscreen) {
            hr = pDevice->SendForceFeedbackCommand(DISFFC_STOPALL);
            IsForceFeedbackCallSuccessful(hr);
        }

        SetForceFeedbackAutocenter(0, deviceID);
        m_forceFeedbackDevices[deviceID].field_0x0 = TRUE;
        ResetForceFeedbackEffects();
    }

    if (m_unk0x00666ee8 == FALSE) {
        m_unk0x00666ee8 = TRUE;
        CGame::RegisterCallback(ResetForceFeedbackEffectsAlt, NULL);
    }

    return TRUE;
}

// FUNCTION: CMR2 0x004ab5f0
bool CInput::IsForceFeedbackCallSuccessful(HRESULT hr) {
    return hr == 0;
}

// FUNCTION: CMR2 0x004aafd0
void CInput::SetForceFeedbackAutocenter(DWORD param1, int deviceID) {
    DIPROPDWORD dipdw;

    if (m_forceFeedbackDevices[deviceID].field_0x0 != FALSE) {
        dipdw.dwData = param1;
        dipdw.diph.dwSize = 0x14;
        dipdw.diph.dwHeaderSize = 0x10;
        dipdw.diph.dwObj = 0;
        dipdw.diph.dwHow = 0;

        m_forceFeedbackDevices[deviceID].device->SetProperty(DIPROP_AUTOCENTER, &dipdw.diph);
    }
}

// 96.55% match, only concern is this
// 0x4aaf35	-cmp edi, 0x666ed4
//          +cmp edi, CInput::m_dinputRefGuidKeyboard (DATA) (Input.cpp:560) <-- why  are you that
// FUNCTION: CMR2 0x004aaf00
void CInput::ResetForceFeedbackEffects(void) {
    LPDIRECTINPUTEFFECT* pEffects = m_forceFeedbackDevices[0].effects;
    
    do {
        ForceFeedbackDevice* pDevice = (ForceFeedbackDevice*)((BYTE*)pEffects - 0xC);
        
        if (pDevice->field_0x0 != FALSE) {
            LPDIRECTINPUTEFFECT* pEffect = pEffects;
            int count = 10;
            
            do {
                if (*pEffect != NULL) {
                    ULONG refcount = (*pEffect)->Release();
                    if (refcount == 0) {
                        *pEffect = NULL;
                    }
                }
                pEffect++;
                count--;
            } while (count != 0);
            
            pDevice->field_0x8 = 0;
        }
        
        pEffects = (LPDIRECTINPUTEFFECT*)((BYTE*)pEffects + 0x34);
    } while ((int)pEffects < (int)m_forceFeedbackDevices[8].effects);
}

// 96.77% match, only concern is this
// 0x4aaeda	-cmp edi, 0x666ec8
// 	        +cmp edi, CInput::m_dinputRefGuidKeyboard (DATA) (Input.cpp:587)  <-- why  are you that
// FUNCTION: CMR2 0x004aaea0
BOOL CInput::ResetForceFeedbackEffectsAlt(void) {
    ForceFeedbackDevice* pDevice = m_forceFeedbackDevices;
    
    do {
        if (pDevice->field_0x0 != FALSE) {
            LPDIRECTINPUTEFFECT* pEffect = pDevice->effects;
            int count = 10;
            
            do {
                if (*pEffect != NULL) {
                    ULONG refcount = (*pEffect)->Release();
                    if (refcount == 0) {
                        *pEffect = NULL;
                    }
                }
                pEffect++;
                count--;
            } while (count != 0);
            
            pDevice->field_0x8 = 0;
            pDevice->field_0x0 = 0;
        }
        pDevice++;
    } while ((int)pDevice < (int)&m_forceFeedbackDevices[8]);
    m_unk0x00666ee8 = FALSE;
    return TRUE;
}

// FUNCTION: CMR2 0x0040bf70
void CInput::LoadControllerInfo(void) {
    ControllerData *fileBuffer;

    m_hasLoadedControllerInfo = 0;
    sprintf(CFrontend::m_stringDest, m_strControllerInfoDir, CInstallInfo::GetGameHDPath());

    fileBuffer = (ControllerData*)CFileBuffer::GetGenericFileBuffer(CFrontend::m_stringDest, TRUE);
    if (fileBuffer != NULL) {
        if (CGenericFileLoader::GetGenericFileSize() == 0x11a4) {
            memcpy(&m_controllerInfo, fileBuffer, sizeof(m_controllerInfo));

            // write the first two elements of the array
            memcpy(m_unk0x005168f4, &((unsigned int*)fileBuffer)[0x468], 4);

            CFileBuffer::FreeGenericFileBuffer(fileBuffer);
            m_hasLoadedControllerInfo = TRUE;

            ApplyControllerButtonBindings(0);
            ApplyControllerButtonBindings(1);
        }
    }
}

// FUNCTION: CMR2 0x0040be90
void CInput::ApplyControllerButtonBindings(unsigned short param1) {
    int uVar1;
    unsigned int raw;
    unsigned short uVar2;
    ControllerData * pController;

    raw = *(unsigned int *)&param1;
    uVar2 = param1;
    uVar1 = m_unk0x005168f4[uVar2];
    pController = &m_controllerInfo[uVar1];

    if (uVar1 > 1) {
        AssignUnmappedControllerActions(raw, pController);
    }

    SetDeviceKeyBinding(uVar2, 1, pController->field_0x13e[0]);
    SetDeviceKeyBinding(uVar2, 2, pController->field_0x13e[1]);
    SetDeviceKeyBinding(uVar2, 4, pController->field_0x13e[2]);
    SetDeviceKeyBinding(uVar2, 8, pController->field_0x13e[3]);
    SetDeviceKeyBinding(uVar2, 0x10, pController->field_0x13e[4]);
    SetDeviceKeyBinding(uVar2, 0x20, pController->field_0x13e[5]);
    SetDeviceKeyBinding(uVar2, 0x40, pController->field_0x13e[6]);
    SetDeviceKeyBinding(uVar2, 0x80, pController->field_0x13e[7]);
    SetDeviceKeyBinding(uVar2, 0x100, pController->field_0x13e[8]);
}

// FUNCTION: CMR2 0x0040c440
void CInput::AssignUnmappedControllerActions(unsigned int param1, ControllerData* param2) {
    unsigned short* puVar2;
    unsigned short uVar4;
    unsigned char bVar1;
    int iVar3;
    unsigned short combinedFlags;
    
    iVar3 = 0;
    puVar2 = &param2->field_0x128;
    
    do {
        if (*puVar2 == 0) {
            if (param2->field_0x13e[iVar3] != 0) {
                
                combinedFlags = param2->field_0x13a | param2->field_0x136 | 
                                param2->field_0x134 | param2->field_0x132 |
                                param2->field_0x130 | param2->field_0x12e | 
                                param2->field_0x12c | param2->field_0x12a |
                                param2->field_0x128 | 0x400;

                bVar1 = FindFirstClearBindingBit(combinedFlags & 0xffff);
                
                uVar4 = 1 << bVar1;
                
                SetDeviceKeyBinding(param1 & 0xffff, uVar4 & 0xffff, param2->field_0x13e[iVar3]);
                SetDeviceKeyBinding(param1 & 0xffff, (1 << iVar3) & 0xffff, 0);
                
                *puVar2 = uVar4;
            }
        }
        
        iVar3++;
        puVar2++;
    } while (iVar3 < 10);
}

// FUNCTION: CMR2 0x0049eb90
void CInput::SetDeviceKeyBinding(int param1, unsigned int param2, unsigned int param3) {
    DeviceInfo* device = &m_availableDevices[param1];
    switch(param2) {
        case 1:
            device->keyboard.field_0x468 = param3;
            break;

        case 2:
            device->keyboard.field_0x469 = param3;
            break;

        case 4:
            device->keyboard.field_0x46a = param3;
            break;

        case 8:
            device->keyboard.field_0x46b = param3;
            break;

        case 0x10:
            device->keyboard.field_0x46c = param3;
            break;

        case 0x20:
            device->keyboard.field_0x46d = param3;
            break;

        case 0x40:
            device->keyboard.field_0x46e = param3;
            break;
        
        case 0x80:
            device->keyboard.field_0x46f = param3;
            break;

        case 0x100:
            device->keyboard.field_0x470 = param3;
            break;

        case 0x200:
            device->keyboard.field_0x471 = param3;
            break;

        case 0x400:
            device->keyboard.field_0x472 = param3;
            break;

        case 0x800:
            device->keyboard.field_0x473 = param3;
            break;
        
        case 0x1000:
            device->keyboard.field_0x474 = param3;
            break;

        case 0x2000:
            device->keyboard.field_0x475 = param3;
            break;
        
        case 0x4000:
            device->keyboard.field_0x476 = param3;
            break;

        case 0x8000:
            device->keyboard.field_0x477 = param3;
            break;
    }
}

// FUNCTION: CMR2 0x0040c530 
BYTE CInput::FindFirstClearBindingBit(unsigned int param1) {
    int iVar1 = 0;

    while (iVar1 < 32) {
        if (!(param1 & 1)) break;
        
        param1 >>= 1;
        iVar1++;
    }

    return iVar1;
}

// FUNCTION: CMR2 0x0049eb50
void CInput::RebuildAvailableInputDevices(void)
{
    int i;

    DInputReleaseDevices();
    m_unk0x0059f8cc.field_0x1 = 2;
    m_unk0x0059f8cc.field_0x0 = 0;
    m_unk0x0059f8cc.field_0x2 = 0;
    for (i = 0; i < 8; i++) {
        m_availableDevices[i].field_0x0 = -1;
        m_availableDevices[i].field_0x18 = -1;
    }
    SetupKeyboard();
    SetupMouse();
    GetAttachedJoysticks();
}

// FUNCTION: CMR2 0x0049edd0
int CInput::GetFirstPressedKey(void)
{
    int key;
    int result;

    result = -1;
    for (key = 0; key < 0xdd; key++) {
        if (m_keyboardState[key] & 0x80) {
            result = key;
            break;
        }
    }
    return result;
}

// FUNCTION: CMR2 0x0049edf0
BOOL CInput::IsShiftPressed(void)
{
    if ((m_keyboardState[DIK_LSHIFT] & 0x80) == 0 && (m_keyboardState[DIK_RSHIFT] & 0x80) == 0)
        return FALSE;
    return TRUE;
}

// FUNCTION: CMR2 0x0049efc0
void CInput::ClearFirstJoystickControlBindings(void)
{
    int i;

    for (i = 0; i < 8; i++) {
        if (m_availableDevices[i].field_0x0 == 2) {
            m_availableDevices[i].joystick.bindings[0].range = 0;
            m_availableDevices[i].joystick.field_0x4 = 0;
            m_availableDevices[i].joystick.controlCount = 0;
            return;
        }
    }
}

// FUNCTION: CMR2 0x0049f300
void CInput::ReadKeyboardState(void)
{
    HRESULT hr;
    short retries;

    retries = 0;
    hr = m_pDirectInputKeyboard->GetDeviceState(sizeof(m_keyboardState), m_keyboardState);
    while (hr == DIERR_INPUTLOST && retries <= 20 && SUCCEEDED(m_pDirectInputKeyboard->Acquire())) {
        retries++;
        hr = m_pDirectInputKeyboard->GetDeviceState(sizeof(m_keyboardState), m_keyboardState);
    }
}

// FUNCTION: CMR2 0x0049fd00
int CInput::GetButtonIndexFromMask(unsigned int mask)
{
    int i;

    for (i = 0; i < 24; i++) {
        if (m_buttonMasks[i] & mask)
            break;
    }
    if (i == 24)
        i = -1;
    return i;
}

// FUNCTION: CMR2 0x0049ff80
void CInput::SetInputRepeatTimingParameters(DWORD p1, DWORD p2, DWORD p3, DWORD p4, DWORD p5)
{
    m_unk0x0059f8f8 = p1;
    m_unk0x0059f910 = p2;
    m_unk0x0059f8f4 = p3;
    m_unk0x0059f8f0 = p4;
    m_unk0x0059f90c = p5;
}

// FUNCTION: CMR2 0x0049ffc0
void CInput::SetInputRepeatTimingState(DWORD param1)
{
    m_unk0x0059f900 = param1;
}

// FUNCTION: CMR2 0x0040be00
DWORD CInput::GetControllerField114(unsigned int param1)
{
    return m_controllerInfo[m_unk0x005168f4[(unsigned short)param1]].field_0x114;
}

// FUNCTION: CMR2 0x0040be30
DWORD CInput::GetControllerField120(unsigned int param1)
{
    return m_controllerInfo[m_unk0x005168f4[(unsigned short)param1]].field_0x120;
}

// FUNCTION: CMR2 0x0040be60
DWORD CInput::GetControllerField124(unsigned short param1)
{
    return m_controllerInfo[m_unk0x005168f4[(unsigned short)param1]].field_0x124;
}

// FUNCTION: CMR2 0x0040c210
unsigned int CInput::GetEnabledControllerAxisBinding(unsigned int param1, int param2)
{
    unsigned int controller;

    controller = m_unk0x005168f4[(unsigned short)param1];
    if (m_controllerInfo[controller].field_0x210[param2].field_0x0 != 0)
        return m_controllerInfo[controller].field_0x2d8[param2];
    return -1;
}

// FUNCTION: CMR2 0x0040c270
BOOL CInput::IsControllerActionBound(int param1, ControllerData *param2)
{
    if ((&param2->field_0x128)[param1] != 0 && param2->field_0x13e[param1] != 0)
        return TRUE;
    return FALSE;
}

// FUNCTION: CMR2 0x004aaf50
void CInput::SetForceFeedbackDeviceValue(DWORD param1, int index)
{
    m_unk0x00666ec8[index] = param1;
}

// FUNCTION: CMR2 0x004aaf70
void CInput::StartForceFeedbackEffect(int effectIndex, int deviceIndex)
{
    ForceFeedbackDevice *pDevice;
    LPDIRECTINPUTEFFECT pEffect;
    DWORD status;

    pDevice = &m_forceFeedbackDevices[deviceIndex];
    if (pDevice->field_0x0 != 0) {
        pEffect = pDevice->effects[effectIndex];
        if (pEffect != NULL) {
            IsForceFeedbackCallSuccessful(pEffect->GetEffectStatus(&status));
            if ((status & DIEGES_PLAYING) == 0)
                pDevice->effects[effectIndex]->Start(1, 0);
        }
    }
}

// FUNCTION: CMR2 0x004ab600
char *CInput::FormatString(LPCSTR format, ...)
{
    va_list args;

    va_start(args, format);
    wvsprintfA(m_formatBuffer, format, args);
    return m_formatBuffer;
}

// FUNCTION: CMR2 0x0049f360
void CInput::ReadMouse(DeviceInfo *pDevice)
{
    DIMOUSESTATE2 mouseState;
    HRESULT hr;
    int retries;
    int i;

    retries = 0;
    pDevice->field_0x4 = 0;
    ((LPDIRECTINPUTDEVICE7A)m_pDirectInputMouse)->Poll();
    hr = m_pDirectInputMouse->GetDeviceState(sizeof(mouseState), &mouseState);
    while (hr == DIERR_INPUTLOST) {
        if (retries > 5)
            return;
        m_pDirectInputMouse->Acquire();
        retries++;
        ((LPDIRECTINPUTDEVICE7A)m_pDirectInputMouse)->Poll();
        hr = m_pDirectInputMouse->GetDeviceState(sizeof(mouseState), &mouseState);
    }

    pDevice->joystick.bindings[0].field_0xc += mouseState.lX * 150;
    if (pDevice->joystick.bindings[0].field_0xc > 0x10000)
        pDevice->joystick.bindings[0].field_0xc = 0x10000;
    if (pDevice->joystick.bindings[0].field_0xc < -0x10000)
        pDevice->joystick.bindings[0].field_0xc = -0x10000;
    pDevice->joystick.bindings[1].field_0xc += mouseState.lY * 150;
    pDevice->joystick.bindings[2].field_0xc += mouseState.lZ * 150;

    for (i = 0; i < 8; i++) {
        if (i < 24 && (mouseState.rgbButtons[i] & 0x80))
            pDevice->field_0x4 |= m_buttonMasks[i];
    }
}

// FUNCTION: CMR2 0x0049f480
void CInput::ReadKeyboardDevice(DeviceInfo *pDevice)
{
    pDevice->field_0x4 = 0;
    if (CGameInfo::m_unk0x0059f8d0 != 0) {
        if (m_keyboardState[DIK_LEFT] & 0x80)
            pDevice->field_0x4 = 1;
        if (m_keyboardState[DIK_RIGHT] & 0x80)
            pDevice->field_0x4 |= 0x2;
        if (m_keyboardState[DIK_UP] & 0x80)
            pDevice->field_0x4 |= 0x4;
        if (m_keyboardState[DIK_DOWN] & 0x80)
            pDevice->field_0x4 |= 0x8;
        if (m_keyboardState[DIK_RETURN] & 0x80)
            pDevice->field_0x4 |= 0x10;
        if (m_keyboardState[DIK_ESCAPE] & 0x80)
            pDevice->field_0x4 |= 0x20;
        if (m_keyboardState[DIK_F1] & 0x80)
            pDevice->field_0x4 |= 0x1000;
        if (m_keyboardState[DIK_F2] & 0x80)
            pDevice->field_0x4 |= 0x2000;
    } else {
        if (m_keyboardState[pDevice->keyboard.field_0x468] & 0x80)
            pDevice->field_0x4 = 1;
        if (m_keyboardState[pDevice->keyboard.field_0x469] & 0x80)
            pDevice->field_0x4 |= 0x2;
        if (m_keyboardState[pDevice->keyboard.field_0x46a] & 0x80)
            pDevice->field_0x4 |= 0x4;
        if (m_keyboardState[pDevice->keyboard.field_0x46b] & 0x80)
            pDevice->field_0x4 |= 0x8;
        if (m_keyboardState[pDevice->keyboard.field_0x46c] & 0x80)
            pDevice->field_0x4 |= 0x10;
        if (m_keyboardState[pDevice->keyboard.field_0x46d] & 0x80)
            pDevice->field_0x4 |= 0x20;
        if (m_keyboardState[pDevice->keyboard.field_0x46e] & 0x80)
            pDevice->field_0x4 |= 0x40;
        if (m_keyboardState[pDevice->keyboard.field_0x46f] & 0x80)
            pDevice->field_0x4 |= 0x80;
        if (m_keyboardState[pDevice->keyboard.field_0x470] & 0x80)
            pDevice->field_0x4 |= 0x100;
        if (m_keyboardState[pDevice->keyboard.field_0x471] & 0x80)
            pDevice->field_0x4 |= 0x200;
        if (m_keyboardState[pDevice->keyboard.field_0x472] & 0x80)
            pDevice->field_0x4 |= 0x400;
        if (m_keyboardState[pDevice->keyboard.field_0x473] & 0x80)
            pDevice->field_0x4 |= 0x800;
        if (m_keyboardState[pDevice->keyboard.field_0x474] & 0x80)
            pDevice->field_0x4 |= 0x1000;
        if (m_keyboardState[pDevice->keyboard.field_0x475] & 0x80)
            pDevice->field_0x4 |= 0x2000;
        if (m_keyboardState[pDevice->keyboard.field_0x476] & 0x80)
            pDevice->field_0x4 |= 0x4000;
        if (m_keyboardState[pDevice->keyboard.field_0x477] & 0x80)
            pDevice->field_0x4 |= 0x8000;
    }
}

// FUNCTION: CMR2 0x0049fb70
void CInput::ReadJoystick(DeviceInfo *pDevice)
{
    DIJOYSTATE2 joyState;
    LPDIRECTINPUTDEVICE7A pJoystick;
    HRESULT hr;
    int retries;
    int i;

    retries = 0;
    pJoystick = (LPDIRECTINPUTDEVICE7A)m_unk0x0059f6b0[pDevice->field_0x18];
    pDevice->field_0x4 = 0;
    pJoystick->Poll();
    hr = pJoystick->GetDeviceState(sizeof(joyState), &joyState);
    while (hr == DIERR_INPUTLOST) {
        if (retries > 5)
            return;
        pJoystick->Acquire();
        retries++;
        pJoystick->Poll();
        hr = pJoystick->GetDeviceState(sizeof(joyState), &joyState);
    }

    pDevice->joystick.bindings[0].field_0xc = joyState.lX;
    pDevice->joystick.bindings[1].field_0xc = joyState.lY;
    pDevice->joystick.bindings[2].field_0xc = joyState.lZ;
    pDevice->joystick.bindings[3].field_0xc = joyState.lRx;
    pDevice->joystick.bindings[4].field_0xc = joyState.lRy;
    pDevice->joystick.bindings[5].field_0xc = joyState.lRz;
    pDevice->joystick.bindings[6].field_0xc = joyState.rglSlider[0];
    pDevice->joystick.bindings[7].field_0xc = joyState.rglSlider[1];

    if (joyState.lX < -39321)
        pDevice->field_0x4 |= 0x1;
    else if (joyState.lX > 39321)
        pDevice->field_0x4 |= 0x2;
    if (joyState.lY < -39321)
        pDevice->field_0x4 |= 0x4;
    else if (joyState.lY > 39321)
        pDevice->field_0x4 |= 0x8;

    if (LOWORD(joyState.rgdwPOV[0]) != 0xffff) {
        if (joyState.rgdwPOV[0] >= 4500 && joyState.rgdwPOV[0] <= 13500)
            pDevice->field_0x4 |= 0x2;
        if (joyState.rgdwPOV[0] >= 13500 && joyState.rgdwPOV[0] <= 22500)
            pDevice->field_0x4 |= 0x8;
        if (joyState.rgdwPOV[0] >= 22500 && joyState.rgdwPOV[0] <= 31500)
            pDevice->field_0x4 |= 0x1;
        if (joyState.rgdwPOV[0] >= 31500 || joyState.rgdwPOV[0] <= 4500)
            pDevice->field_0x4 |= 0x4;
    }

    for (i = 0; i < pDevice->field_0x14; i++) {
        if (i < 24 && (joyState.rgbButtons[i] & 0x80))
            pDevice->field_0x4 |= m_buttonMasks[i];
    }
}

// FUNCTION: CMR2 0x0040c130
void CInput::SetIndexedWordBinding(unsigned short *values, int index, unsigned short value)
{
    switch (index) {
    case 0: values[0] = value; break;
    case 1: values[1] = value; break;
    case 2: values[2] = value; break;
    case 3: values[3] = value; break;
    case 4: values[4] = value; break;
    case 5: values[5] = value; break;
    case 6: values[6] = value; break;
    case 7: values[7] = value; break;
    case 8: values[8] = value; break;
    case 9: values[9] = value; break;
    }
}

// FUNCTION: CMR2 0x0040c550
void CInput::SetIndexedByteBinding(BYTE *values, int index, BYTE value)
{
    switch (index) {
    case 0: values[0] = value; break;
    case 1: values[1] = value; break;
    case 2: values[2] = value; break;
    case 3: values[3] = value; break;
    case 4: values[4] = value; break;
    case 5: values[5] = value; break;
    case 6: values[6] = value; break;
    case 7: values[7] = value; break;
    case 8: values[8] = value; break;
    }
}

// FUNCTION: CMR2 0x0049e960
DeviceInfo *CInput::UpdateDevice(int index)
{
    DeviceInfo *pDevice;
    unsigned int oldButtons;
    unsigned int buttons;
    unsigned int dirButtons;
    int now;

    if (index < 0 || index >= 8)
        return NULL;

    pDevice = &m_availableDevices[index];
    oldButtons = pDevice->field_0x4;
    if (index > 1) {
        if (pDevice->field_0x18 < 0 || pDevice->field_0x18 > 3)
            return NULL;
    }

    switch (pDevice->field_0x0) {
    case 1:
        if (index == 0)
            ReadKeyboardState();
        ReadKeyboardDevice(pDevice);
        break;
    case 0:
    case 3:
        ReadJoystick(pDevice);
        break;
    case 2:
        ReadMouse(pDevice);
        break;
    }

    buttons = pDevice->field_0x4;
    pDevice->field_0x8 = (buttons ^ oldButtons) & buttons;
    if (buttons & 0xfffffff0)
        pDevice->field_0x4 = buttons | 0x10000;

    now = CMain::GetFrameTime();
    dirButtons = pDevice->field_0x4 & m_directionButtonMask;
    if (dirButtons != 0) {
        if (dirButtons != (m_directionButtonMask & oldButtons)) {
            pDevice->field_0xc = ~oldButtons & pDevice->field_0x4 & m_directionButtonMask;
            pDevice->field_0x10 = (int)m_keyboardDelay + now;
        } else if (pDevice->field_0x10 - now <= 0) {
            pDevice->field_0xc = dirButtons;
            pDevice->field_0x10 = (int)m_keyboardSpeed + now;
        } else {
            pDevice->field_0xc = 0;
        }
    } else {
        pDevice->field_0xc = 0;
        pDevice->field_0x10 = 0;
    }

    if (m_unk0x0052086c != 0)
        pDevice->field_0x8 |= pDevice->field_0xc;

    return pDevice;
}

// FUNCTION: CMR2 0x004ab380
int CInput::CreateForceFeedbackEffect(int effectType, DWORD duration, LONG coefficient, LONG offset, int triggerButton, int deviceIndex)
{
    ForceFeedbackDevice *pDevice;
    GUID guid;
    DWORD dwAxes[2];
    LONG lDirection[2];
    int slot;
    int i;

    i = 0;
    pDevice = &m_forceFeedbackDevices[deviceIndex];
    if (pDevice->field_0x0 == 0)
        return -1;

    while (pDevice->effects[i] != NULL) {
        if (i == 10)
            return -1;
        i++;
    }

    switch (effectType) {
    case 0xb:
        guid = GUID_Spring;
        break;
    case 0xc:
        guid = GUID_Inertia;
        break;
    case 0xd:
        guid = GUID_Damper;
        break;
    case 0xe:
        guid = GUID_Friction;
        break;
    }

    slot = deviceIndex * 10 + i;
    dwAxes[0] = DIJOFS_X;
    lDirection[0] = 0;
    lDirection[1] = 0;

    m_forceFeedbackConditions[slot].lOffset = offset;
    m_forceFeedbackConditions[slot].lPositiveCoefficient = coefficient;
    m_forceFeedbackConditions[slot].lNegativeCoefficient = coefficient;
    m_forceFeedbackConditions[slot].dwPositiveSaturation = 10000;
    m_forceFeedbackConditions[slot].dwNegativeSaturation = 10000;
    m_forceFeedbackConditions[slot].lDeadBand = 0;

    m_forceFeedbackEffects[slot].dwSize = sizeof(DIEFFECT);
    m_forceFeedbackEffects[slot].dwFlags = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
    m_forceFeedbackEffects[slot].dwDuration = duration;
    m_forceFeedbackEffects[slot].dwSamplePeriod = 10000;
    m_forceFeedbackEffects[slot].dwGain = 10000;
    if (triggerButton == -1)
        triggerButton = DIEB_NOTRIGGER;
    else
        triggerButton = DIJOFS_BUTTON(triggerButton);
    m_forceFeedbackEffects[slot].dwTriggerButton = triggerButton;
    m_forceFeedbackEffects[slot].dwTriggerRepeatInterval = 0;
    m_forceFeedbackEffects[slot].cAxes = 1;
    m_forceFeedbackEffects[slot].rgdwAxes = dwAxes;
    m_forceFeedbackEffects[slot].rglDirection = lDirection;
    m_forceFeedbackEffects[slot].lpEnvelope = NULL;
    m_forceFeedbackEffects[slot].cbTypeSpecificParams = sizeof(DICONDITION);
    m_forceFeedbackEffects[slot].lpvTypeSpecificParams = &m_forceFeedbackConditions[slot];

    pDevice->device->CreateEffect(guid, &m_forceFeedbackEffects[slot], &pDevice->effects[i], NULL);
    return i;
}

// FUNCTION: CMR2 0x004ab590
int CInput::CreateSpringEffect(DWORD duration, LONG coefficient, LONG offset, int triggerButton, int deviceIndex)
{
    return CreateForceFeedbackEffect(0xb, duration, coefficient, offset, triggerButton, deviceIndex);
}

// FUNCTION: CMR2 0x004ab5c0
int CInput::CreateDamperEffect(DWORD duration, LONG coefficient, LONG offset, int triggerButton, int deviceIndex)
{
    return CreateForceFeedbackEffect(0xd, duration, coefficient, offset, triggerButton, deviceIndex);
}

// match 76%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0040c2a0
short CInput::GetButtonMapping(unsigned short controller, int button)
{
    int index;
    ControllerData *pController;
    short mapping;

    mapping = 0;
    index = m_unk0x005168f4[controller];
    pController = &m_controllerInfo[index];
    switch (button) {
    case 0: mapping = m_controllerInfo[index].field_0x128; break;
    case 1: mapping = m_controllerInfo[index].field_0x12a; break;
    case 2: mapping = m_controllerInfo[index].field_0x12c; break;
    case 3: mapping = m_controllerInfo[index].field_0x12e; break;
    case 4: mapping = m_controllerInfo[index].field_0x130; break;
    case 5: mapping = m_controllerInfo[index].field_0x132; break;
    case 6: mapping = m_controllerInfo[index].field_0x134; break;
    case 7: mapping = m_controllerInfo[index].field_0x136; break;
    case 8: mapping = m_controllerInfo[index].field_0x138; break;
    case 9: mapping = m_controllerInfo[index].field_0x13a; break;
    default: goto defaults;
    }

    if (mapping == 0) {
defaults:
        if (controller == 0 || controller == 1)
            goto fixed;
    }

    if (!IsControllerActionBound(button, pController))
        return mapping;

fixed:
    switch (button) {
    case 0: mapping = 1; break;
    case 1: mapping = 2; break;
    case 2: mapping = 4; break;
    case 3: mapping = 8; break;
    case 4: mapping = 0x10; break;
    case 5: mapping = 0x20; break;
    case 6: mapping = 0x40; break;
    case 7: mapping = 0x80; break;
    case 8: mapping = 0x100; break;
    case 9: mapping = 0x200; break;
    }
    return mapping;
}

// FUNCTION: CMR2 0x004ab030
HRESULT CInput::SetEffectGain(int effectIndex, DWORD gain, int deviceIndex)
{
    ForceFeedbackDevice *pDevice;
    LPDIRECTINPUTEFFECT pEffect;
    DWORD flags;

    pDevice = &m_forceFeedbackDevices[deviceIndex];
    flags = DIEP_GAIN;
    if (m_unk0x00666ec8[deviceIndex] == 0)
        flags = DIEP_GAIN | DIEP_NODOWNLOAD;

    if (pDevice->field_0x0 != 0 && (pEffect = pDevice->effects[effectIndex]) != NULL) {
        DIEFFECT effect = { sizeof(DIEFFECT) };
        effect.dwGain = gain;
        return pDevice->effects[effectIndex]->SetParameters(&effect, flags);
    }
    return E_INVALIDARG;
}

// FUNCTION: CMR2 0x004ab0b0
HRESULT CInput::SetEffectGainAndDirection(int effectIndex, DWORD gain, LONG direction, int deviceIndex)
{
    ForceFeedbackDevice *pDevice;
    LPDIRECTINPUTEFFECT pEffect;
    DWORD flags;

    pDevice = &m_forceFeedbackDevices[deviceIndex];
    flags = DIEP_GAIN | DIEP_DIRECTION;
    if (m_unk0x00666ec8[deviceIndex] == 0)
        flags = DIEP_GAIN | DIEP_DIRECTION | DIEP_NODOWNLOAD;

    if (pDevice->field_0x0 != 0 && (pEffect = pDevice->effects[effectIndex]) != NULL) {
        DIEFFECT effect = { sizeof(DIEFFECT) };
        LONG lDirection[2];
        effect.dwFlags = DIEFF_POLAR | DIEFF_OBJECTOFFSETS;
        effect.cAxes = 2;
        effect.dwGain = gain;
        effect.rgdwAxes = NULL;
        effect.rglDirection = lDirection;
        lDirection[0] = direction;
        return pDevice->effects[effectIndex]->SetParameters(&effect, flags);
    }
    return E_INVALIDARG;
}

// FUNCTION: CMR2 0x004ab150
int CInput::CreateConstantForceEffect(DWORD duration, LONG direction, LONG magnitude, DWORD attackTime, DWORD attackLevel, DWORD fadeTime, DWORD fadeLevel, int triggerButton, int deviceIndex)
{
    ForceFeedbackDevice *pDevice;
    int i;
    LONG lDirection[2];
    DWORD dwAxes[2];
    DICONSTANTFORCE constantForce;

    i = 0;
    pDevice = &m_forceFeedbackDevices[deviceIndex];
    if (pDevice->field_0x0 == 0)
        return -1;

    while (pDevice->effects[i] != NULL) {
        if (i == 10)
            return -1;
        i++;
    }

    constantForce.lMagnitude = magnitude;
    DIENVELOPE envelope = { sizeof(DIENVELOPE) };
    envelope.dwAttackTime = attackTime;
    envelope.dwAttackLevel = attackLevel;
    envelope.dwFadeTime = fadeTime;
    envelope.dwFadeLevel = fadeLevel;
    lDirection[0] = direction;
    DIEFFECT effect = { sizeof(DIEFFECT) };
    effect.dwSamplePeriod = 10000;
    effect.dwGain = 10000;
    dwAxes[0] = DIJOFS_X;
    dwAxes[1] = DIJOFS_Y;
    lDirection[1] = 0;
    effect.dwFlags = DIEFF_POLAR | DIEFF_OBJECTOFFSETS;
    effect.dwDuration = duration;
    if (triggerButton == -1)
        effect.dwTriggerButton = triggerButton;
    else
        effect.dwTriggerButton = DIJOFS_BUTTON(triggerButton);
    effect.rgdwAxes = dwAxes;
    effect.rglDirection = lDirection;
    effect.lpEnvelope = &envelope;
    effect.dwTriggerRepeatInterval = 0;
    effect.cAxes = 2;
    effect.cbTypeSpecificParams = sizeof(DICONSTANTFORCE);
    effect.lpvTypeSpecificParams = &constantForce;

    IsForceFeedbackCallSuccessful(pDevice->device->CreateEffect(GUID_ConstantForce, &effect, &pDevice->effects[i], NULL));
    return i;
}

// FUNCTION: CMR2 0x004ab2b0
HRESULT CInput::SetConditionCoefficient(int effectIndex, LONG coefficient, int deviceIndex)
{
    ForceFeedbackDevice *pDevice;
    DWORD flags;
    int slot;

    pDevice = &m_forceFeedbackDevices[deviceIndex];
    flags = DIEP_TYPESPECIFICPARAMS;
    if (m_unk0x00666ec8[deviceIndex] == 0)
        flags = DIEP_TYPESPECIFICPARAMS | DIEP_NODOWNLOAD;

    if (pDevice->field_0x0 != 0 && pDevice->effects[effectIndex] != NULL) {
        slot = deviceIndex * 10 + effectIndex;
        m_forceFeedbackConditions[slot].lOffset = 0;
        m_forceFeedbackConditions[slot].lPositiveCoefficient = coefficient;
        m_forceFeedbackConditions[slot].lNegativeCoefficient = coefficient;
        m_forceFeedbackConditions[slot].dwPositiveSaturation = 10000;
        m_forceFeedbackConditions[slot].dwNegativeSaturation = 10000;
        m_forceFeedbackConditions[slot].lDeadBand = 0;
        m_forceFeedbackEffects[slot].cbTypeSpecificParams = sizeof(DICONDITION);
        m_forceFeedbackEffects[slot].lpvTypeSpecificParams = &m_forceFeedbackConditions[slot];
        return pDevice->effects[effectIndex]->SetParameters(&m_forceFeedbackEffects[slot], flags);
    }
    return E_INVALIDARG;
}

// FUNCTION: CMR2 0x0040bff0
void CInput::SaveControllerInfo(void)
{
    unsigned int *buffer;

    buffer = (unsigned int *)CFileBuffer::AllocateLockedBuffer(0x11a4);
    memcpy(buffer, m_controllerInfo, sizeof(m_controllerInfo));
    buffer[0x468] = *(unsigned int *)m_unk0x005168f4;
    sprintf(CFrontend::m_stringDest, m_strControllerInfoDir, CInstallInfo::GetGameHDPath());
    CInstallInfo::WriteFileToDisk(CFrontend::m_stringDest, 2, buffer, 0x11a4);
    CFileBuffer::FreeGenericFileBuffer(buffer);
}

// Key labels of the keyboard controls: the first keyboard uses text for the
// cursor keys and [ ] C R, the second one the numeric keypad keys.
// GLOBAL: CMR2 0x00516928
char g_keypad7[] = "7";
// GLOBAL: CMR2 0x0051692c
char g_keypad9[] = "9";
// GLOBAL: CMR2 0x00516930
char g_keypad3[] = "3";
// GLOBAL: CMR2 0x00516934
char g_keypad2[] = "2";
// GLOBAL: CMR2 0x00516938
char g_keypad8[] = "8";
// GLOBAL: CMR2 0x0051693c
char g_keypad6[] = "6";
// GLOBAL: CMR2 0x00516940
char g_keypadFormat[] = "%s %s";
// GLOBAL: CMR2 0x00516948
char g_keypad4[] = "4";
// GLOBAL: CMR2 0x0051694c
char g_keyR[] = "R";
// GLOBAL: CMR2 0x00516950
char g_keyC[] = "C";
// GLOBAL: CMR2 0x00516954
char g_keyOpenBracket[] = "[";
// GLOBAL: CMR2 0x00516958
char g_keyCloseBracket[] = "]";

// The five-dword joystick axis bindings of a device start at 0x46c.
#define DEVICE_BINDING(pDevice, i) ((ControllerDataUnk0x210 *)((BYTE *)(pDevice) + 0x46c + (i) * 0x14))

// Fills controller slot index from a detected device: its name, type flags,
// the default bindings and the texts shown for them in the controls menu.
// FUNCTION: CMR2 0x0040c610
void CInput::InitDetectedControllerSlot(DeviceInfo *pDevice, int index)
{
    ControllerData *pController;

    pController = &m_controllerInfo[index];
    pController->index = (BYTE)index;
    ((int (__cdecl *)(char *, const char *))sprintf)(pController->name, pDevice->deviceInstanceName);
    pController->field_0x10c = (pDevice->field_0x0 == 3 || pDevice->field_0x0 == 2) ? 1 : 0;
    pController->field_0x110 = (pDevice->field_0x0 == 3 || pDevice->field_0x0 == 2) ? 1 : 0;
    pController->field_0x114 = (pDevice->field_0x0 == 3 || pDevice->field_0x0 == 2) ? 1 : 0;
    pController->field_0x118 = pDevice->unk_isJoystick;
    m_controllerInfo[index].field_0x4 = 0x8000;
    m_controllerInfo[index].field_0x0 = 0x8000;
    if (pDevice->field_0x0 == 1) {
        if (index == 0) {
            m_controllerInfo[0].field_0x13e[0] = pDevice->keyboard.field_0x468;
            ((int (__cdecl *)(char *, const char *))sprintf)(m_controllerInfo[0].keyNames[0], CFrontend::GetTextString(0x1f9));
            m_controllerInfo[0].field_0x13e[1] = pDevice->keyboard.field_0x469;
            ((int (__cdecl *)(char *, const char *))sprintf)(m_controllerInfo[0].keyNames[1], CFrontend::GetTextString(0x1fa));
            m_controllerInfo[0].field_0x13e[2] = pDevice->keyboard.field_0x46a;
            ((int (__cdecl *)(char *, const char *))sprintf)(m_controllerInfo[0].keyNames[2], CFrontend::GetTextString(0x1fb));
            m_controllerInfo[0].field_0x13e[3] = pDevice->keyboard.field_0x46b;
            ((int (__cdecl *)(char *, const char *))sprintf)(m_controllerInfo[0].keyNames[3], CFrontend::GetTextString(0x1fc));
            m_controllerInfo[0].field_0x13e[4] = pDevice->keyboard.field_0x46c;
            ((int (__cdecl *)(char *, const char *))sprintf)(m_controllerInfo[0].keyNames[4], CFrontend::GetTextString(0x1fd));
            m_controllerInfo[0].field_0x13e[5] = pDevice->keyboard.field_0x46d;
            ((int (__cdecl *)(char *, const char *))sprintf)(m_controllerInfo[0].keyNames[5], g_keyCloseBracket);
            m_controllerInfo[0].field_0x13e[6] = pDevice->keyboard.field_0x46e;
            ((int (__cdecl *)(char *, const char *))sprintf)(m_controllerInfo[0].keyNames[6], g_keyOpenBracket);
            m_controllerInfo[0].field_0x13e[7] = pDevice->keyboard.field_0x46f;
            ((int (__cdecl *)(char *, const char *))sprintf)(m_controllerInfo[0].keyNames[7], g_keyC);
            m_controllerInfo[0].field_0x13e[8] = pDevice->keyboard.field_0x470;
            ((int (__cdecl *)(char *, const char *))sprintf)(m_controllerInfo[0].keyNames[8], g_keyR);
            return;
        }
        pController->field_0x13e[0] = pDevice->keyboard.field_0x468;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[0], FormatString(g_keypadFormat, CFrontend::GetTextString(0x203), g_keypad4));
        pController->field_0x13e[1] = pDevice->keyboard.field_0x469;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[1], FormatString(g_keypadFormat, CFrontend::GetTextString(0x203), g_keypad6));
        pController->field_0x13e[2] = pDevice->keyboard.field_0x46a;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[2], FormatString(g_keypadFormat, CFrontend::GetTextString(0x203), g_keypad8));
        pController->field_0x13e[3] = pDevice->keyboard.field_0x46b;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[3], FormatString(g_keypadFormat, CFrontend::GetTextString(0x203), g_keypad2));
        pController->field_0x13e[4] = pDevice->keyboard.field_0x46c;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[4], FormatString(g_keypadFormat, CFrontend::GetTextString(0x203), g_keypad3));
        pController->field_0x13e[5] = pDevice->keyboard.field_0x46d;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[5], CFrontend::GetTextString(0x204));
        pController->field_0x13e[6] = pDevice->keyboard.field_0x46e;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[6], CFrontend::GetTextString(0x205));
        pController->field_0x13e[7] = pDevice->keyboard.field_0x46f;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[7], FormatString(g_keypadFormat, CFrontend::GetTextString(0x203), g_keypad9));
        pController->field_0x13e[8] = pDevice->keyboard.field_0x470;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[8], FormatString(g_keypadFormat, CFrontend::GetTextString(0x203), g_keypad7));
        return;
    }
    if (pDevice->field_0x0 == 2) {
        *(DWORD *)DEVICE_BINDING(pDevice, 0) = 1;
        pController->field_0x210[0] = *DEVICE_BINDING(pDevice, 0);
        pController->field_0x2d8[0] = 0;
        pController->field_0x210[1] = *DEVICE_BINDING(pDevice, 0);
        pController->field_0x2d8[1] = 0;
        pController->field_0x128 = 0;
        pController->field_0x12a = 0;
        pController->field_0x12c = 0x10;
        pController->field_0x12e = 0x20;
        pController->field_0x130 = 0;
        pController->field_0x132 = 0;
        pController->field_0x134 = 0;
        pController->field_0x136 = 0;
        pController->field_0x138 = 0;
        pController->field_0x13a = 0;
        pController->field_0x13e[4] = (BYTE)*(DWORD *)DEVICE_BINDING(pDevice, 0);
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[4], CFrontend::GetTextString(0x1fe));
        pController->field_0x13e[5] = pDevice->keyboard.field_0x46d;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[5], CFrontend::GetTextString(0x1fe));
        pController->field_0x13e[6] = pDevice->keyboard.field_0x46e;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[6], CFrontend::GetTextString(0x1fe));
        pController->field_0x13e[7] = pDevice->keyboard.field_0x46f;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[7], CFrontend::GetTextString(0x1fe));
        pController->field_0x13e[8] = pDevice->keyboard.field_0x470;
        ((int (__cdecl *)(char *, const char *))sprintf)(pController->keyNames[8], CFrontend::GetTextString(0x1fe));
        return;
    }
    if (pController->field_0x10c != 0) {
        pController->field_0x128 = 1;
        pController->field_0x12a = 2;
        if (*(DWORD *)DEVICE_BINDING(pDevice, 0) != 0) {
            pController->field_0x210[0] = *DEVICE_BINDING(pDevice, 0);
            pController->field_0x2d8[0] = 0;
            if (*(DWORD *)DEVICE_BINDING(pDevice, 0) != 0) {
                pController->field_0x210[1] = *DEVICE_BINDING(pDevice, 0);
                pController->field_0x2d8[1] = 0;
            }
        }
        pController->field_0x12c = 4;
        if (*(DWORD *)DEVICE_BINDING(pDevice, 1) != 0) {
            pController->field_0x210[2] = *DEVICE_BINDING(pDevice, 1);
            pController->field_0x2d8[2] = 1;
        }
        pController->field_0x12e = 8;
        if (*(DWORD *)DEVICE_BINDING(pDevice, 1) != 0) {
            pController->field_0x210[3] = *DEVICE_BINDING(pDevice, 1);
            pController->field_0x2d8[3] = 1;
        }
        pController->field_0x130 = 0x10;
        pController->field_0x132 = 0x20;
        pController->field_0x134 = 0x40;
        pController->field_0x136 = 0x80;
        pController->field_0x138 = 0x100;
        pController->field_0x13a = 0x200;
        return;
    }
    memset(pController->field_0x210, 0, 0x32 * 4);
    pController->field_0x128 = 1;
    pController->field_0x12a = 2;
    pController->field_0x12c = 0x10;
    pController->field_0x12e = 0x20;
    pController->field_0x130 = 0x40;
    pController->field_0x132 = 0x80;
    pController->field_0x134 = 0x100;
    pController->field_0x136 = 0x200;
    pController->field_0x138 = 0x800;
    pController->field_0x13a = 0x400;
}

// FUNCTION: CMR2 0x0040c050
void CInput::RefreshControllerConfigurations(void)
{
    DeviceInfo *pDevice;
    int i;

    m_controllerCount = CountAttachedInputDevices();
    for (i = 0; i < m_controllerCount; i++) {
        pDevice = UpdateDevice(i);
        if (m_hasLoadedControllerInfo == 0 || strcmp(m_controllerInfo[i].name, pDevice->deviceInstanceName) != 0)
            InitDetectedControllerSlot(pDevice, i);
    }
    if (m_controllerCount < 6)
        memset(&m_controllerInfo[m_controllerCount], 0, (6 - m_controllerCount) * sizeof(ControllerData));
    m_hasLoadedControllerInfo = TRUE;
}

struct InputQueues {
    int characters[30];
    int keys[30];
};

// GLOBAL: CMR2 0x006ed3f4
InputQueues g_inputQueues;
#define g_unk0x006ed3f4 (g_inputQueues.characters)
#define g_unk0x006ed46c (g_inputQueues.keys)

// FUNCTION: CMR2 0x004b7c80
void Input_ClearCharacterQueue(void)
{
    int i;

    for (i = 0; i < 30; i++)
        g_unk0x006ed3f4[i] = 0;
}

// Queues one character for the input ring buffer.
// FUNCTION: CMR2 0x004b7ca0
void CInput::QueueInputCharacter(int param1)
{
    int *p;
    int i;

    i = 0;
    p = g_unk0x006ed3f4;
    while ((int)p < (int)(g_unk0x006ed3f4 + 30)) {
        if (*p == 0) {
            g_unk0x006ed3f4[i] = param1;
            return;
        }
        p++;
        i++;
    }
}

// Pops the oldest character of the input ring buffer.
// FUNCTION: CMR2 0x004b7cd0
bool Input_PopQueuedCharacter(int *pOut)
{
    int i;

    if (g_unk0x006ed3f4[0] != 0) {
        *pOut = g_unk0x006ed3f4[0];
        i = 0;
        do {
            g_unk0x006ed3f4[i] = g_unk0x006ed3f4[i + 1];
            i++;
        } while (i < 29);
        g_unk0x006ed3f4[29] = 0;
        return true;
    }
    return false;
}

// Queues one key press (only when the scan code carries a virtual key).
// FUNCTION: CMR2 0x004b7d10
void CInput::QueueVirtualKeyPress(unsigned int param1)
{
    int *p;
    int i;

    if ((param1 & 0xff0000) != 0) {
        i = 0;
        p = g_unk0x006ed46c;
        while ((int)p < (int)(g_unk0x006ed46c + 30)) {
            if (*p == 0) {
                g_unk0x006ed46c[i] = param1;
                return;
            }
            p++;
            i++;
        }
    }
}

// GLOBAL: CMR2 0x005320a4
int g_unk0x005320a4;

// FUNCTION: CMR2 0x0040af20
void CInput::UpdateInputFrameDelta(void)
{
    g_unk0x005320a4 = CMain::GetFrameDelta();
}

// FUNCTION: CMR2 0x0049eab0
void CInput::UpdateAllAvailableDevices(void)
{
    int i;

    for (i = 0; i < 8; i++)
        UpdateDevice(i);
}

// match 57%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x0040bc90
void CInput::SetControllerForceFeedbackValue(unsigned short param1, DWORD param2)
{
    unsigned short index;

    index = CInput::m_unk0x005168f4[param1];
    SetForceFeedbackDeviceValue(m_controllerInfo[index].field_0x11c = param2, index);
}


// FUNCTION: CMR2 0x0049ead0
DeviceInfo *CInput::GetAvailableDeviceRecord(int index)
{
    return &m_availableDevices[index];
}

// FUNCTION: CMR2 0x004b7d40
void Input_ClearKeyPressQueue(void)
{
    int i;

    for (i = 0; i < 30; i++)
        g_unk0x006ed46c[i] = 0;
}

// Pops the oldest key of the key press queue.
// FUNCTION: CMR2 0x004b7d60
int Input_PopQueuedKeyPress(int *pOut)
{
    int i;

    if (g_unk0x006ed46c[0] != 0) {
        *pOut = g_unk0x006ed46c[0];
        i = 0;
        do {
            g_unk0x006ed46c[i] = g_unk0x006ed46c[i + 1];
            i++;
        } while (i < 29);
        g_unk0x006ed46c[29] = 0;
        return 1;
    }
    return 0;
}

// Latched state of the two throttle axes of each device (pressed once until released).
// GLOBAL: CMR2 0x005334f4
int g_unk0x005334f4[8][2];

// Turns the joystick's accelerate/brake axes into one-shot "left"/"right"
// menu presses (bits 2 and 3) when they are pushed past a quarter.
// FUNCTION: CMR2 0x0040bad0
void Input_TranslatePedalsToMenuKeys(void)
{
    unsigned int i;
    DeviceInfo *pDev;

    for (i = 0; i < 8; i++) {
        pDev = CInput::GetAvailableDeviceRecord(i);
        if (pDev->field_0x0 == 3 && CInput::GetEnabledControllerAxisBinding(0, 2) != CInput::GetEnabledControllerAxisBinding(0, 3)) {
            pDev->field_0x8 &= 0xfffffff3;
            if (((int *)pDev)[CInput::GetEnabledControllerAxisBinding(0, 2) * 5 + 0x11f] < 0x3333) {
                if (g_unk0x005334f4[i][0] == 0) {
                    pDev->field_0x8 |= 4;
                    g_unk0x005334f4[i][0] = 1;
                }
            } else {
                g_unk0x005334f4[i][0] = 0;
            }
            if (((int *)pDev)[CInput::GetEnabledControllerAxisBinding(0, 3) * 5 + 0x11f] < 0x3333) {
                if (g_unk0x005334f4[i][1] == 0) {
                    pDev->field_0x8 |= 8;
                    g_unk0x005334f4[i][1] = 1;
                }
            } else {
                g_unk0x005334f4[i][1] = 0;
            }
        }
    }
}

// Controller slots are mapped to entries of m_controllerInfo through m_unk0x005168f4.
// FUNCTION: CMR2 0x0040bba0
unsigned short Input_GetAssignedController(void)
{
    return CInput::m_controllerCount;
}

// FUNCTION: CMR2 0x0040bbb0
ControllerData *Input_GetControllerTable(void)
{
    return CInput::m_controllerInfo;
}

// FUNCTION: CMR2 0x0040bbc0
unsigned short Input_GetControllerSlotMapping(unsigned short slot)
{
    return CInput::m_unk0x005168f4[slot];
}

// FUNCTION: CMR2 0x0040bbe0
void Input_SetControllerSlotMapping(unsigned short slot, unsigned short index)
{
    CInput::m_unk0x005168f4[slot] = index;
}

// FUNCTION: CMR2 0x0040bc00
unsigned int Input_GetControllerPrimaryFlags(unsigned short slot)
{
    return CInput::m_controllerInfo[CInput::m_unk0x005168f4[slot]].field_0x0;
}

// FUNCTION: CMR2 0x0040bc30
unsigned int Input_GetControllerSecondaryFlags(unsigned short slot)
{
    return CInput::m_controllerInfo[CInput::m_unk0x005168f4[slot]].field_0x4;
}

// FUNCTION: CMR2 0x0040bc60
void Input_SetControllerPrimaryFlags(unsigned short slot, unsigned int value)
{
    CInput::m_controllerInfo[CInput::m_unk0x005168f4[slot]].field_0x0 = value;
}

// FUNCTION: CMR2 0x0040bcd0
DWORD Input_GetControllerField11C(unsigned short slot)
{
    return CInput::m_controllerInfo[CInput::m_unk0x005168f4[slot]].field_0x11c;
}

// FUNCTION: CMR2 0x0040bd00
void Input_SetControllerSecondaryFlags(unsigned short slot, unsigned int value)
{
    CInput::m_controllerInfo[CInput::m_unk0x005168f4[slot]].field_0x4 = value;
}

// FUNCTION: CMR2 0x0040bd30
DWORD Input_GetControllerField118(unsigned short slot)
{
    return CInput::m_controllerInfo[CInput::m_unk0x005168f4[slot]].field_0x118;
}

// Merges the buttons of the joystick assigned to the slot into pOut.
// FUNCTION: CMR2 0x0040bd60
void Input_MergeAssignedJoystickButtons(unsigned short slot, DeviceInfo *pOut)
{
    DeviceInfo *pDev;

    if ((short)Input_GetControllerSlotMapping(slot) != 0 && (short)Input_GetControllerSlotMapping(slot) != 1) {
        pDev = CInput::GetAvailableDeviceRecord(Input_GetControllerSlotMapping(slot));
        if (pDev->field_0x0 == 0 || pDev->field_0x0 == 3) {
            pOut->field_0x8 |= pDev->field_0x8;
            pOut->field_0x4 |= pDev->field_0x4;
            pOut->field_0xc |= pDev->field_0xc;
        }
    }
}

// FUNCTION: CMR2 0x0040bdd0
DWORD Input_GetControllerField110(unsigned short slot)
{
    return CInput::m_controllerInfo[CInput::m_unk0x005168f4[slot]].field_0x110;
}


int Input_PopQueuedKeyPress(int *pOut);

// Name of the last key pressed (waits until the key queue is empty).
// FUNCTION: CMR2 0x0049ed80
void Input_GetLastKeyName(LPSTR pName, unsigned int size)
{
    LONG key;

    key = 0;
    while (Input_PopQueuedKeyPress((int *)&key) != 0)
        ;
    GetKeyNameTextA(key, pName, size & 0xff);
}
