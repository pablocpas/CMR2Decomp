#ifndef _STAGE_BLOCK_H
#define _STAGE_BLOCK_H

#include <windows.h>

// Stage object tables 0x58d2a0..0x58d6d0. Several of the original loops walk
// across neighbouring tables (rows of 12 bytes, pairs of pointers), so the
// region is one contiguous block and the named tables are views into it.
// GLOBAL: CMR2 0x0058d2a0
extern BYTE g_stageBlock[0x430];

#define g_unk0x0058d2a0 ((int *)(g_stageBlock + 0x0))          // int[12]
#define g_unk0x0058d2f0 ((int *)(g_stageBlock + 0x50))         // int[8]
#define g_stageBlock_58d340 ((int *)(g_stageBlock + 0xa0))     // int[8]
#define g_stageBlock_58d368 ((int *)(g_stageBlock + 0xc8))     // 6 rows of 3 ints
#define g_unk0x0058d3b0 ((int *)(g_stageBlock + 0x110))        // int[2]
#define g_unk0x0058d3b8 (g_stageBlock + 0x118)                 // 16 rows of 12 bytes
#define g_stageBlock_58d47c ((int *)(g_stageBlock + 0x1dc))    // int[8]
#define g_unk0x0058d49c ((void **)(g_stageBlock + 0x1fc))      // void *[8]
#define g_unk0x0058d4c4 ((int *)(g_stageBlock + 0x224))        // pairs from 0x58d4c0
#define g_unk0x0058d4d0 (g_stageBlock + 0x230)                 // BYTE[8]
#define g_unk0x0058d4f0 (g_stageBlock + 0x250)                 // 2 rows of 0x1c bytes (pointers into timing records)
#define g_unk0x0058d530 (g_stageBlock + 0x290)                 // 2 rows of 0x1c bytes
#define g_unk0x0058d6a0 ((void **)(g_stageBlock + 0x400))      // void *[2]
#define g_unk0x0058d6a8 ((int *)(g_stageBlock + 0x408))        // int[2]
#define g_unk0x0058d6b0 ((int *)(g_stageBlock + 0x410))        // int[7]

#endif
