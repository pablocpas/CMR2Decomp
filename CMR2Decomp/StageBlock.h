#ifndef _STAGE_BLOCK_H
#define _STAGE_BLOCK_H

#include <windows.h>
#include "CarInterior.h"
#include "GenericFileLoader.h"

// Stage object tables 0x58d2a0..0x58d6d0. Several of the original loops walk
// across neighbouring tables (rows of 12 bytes, pairs of pointers), so the
// region is one contiguous block and the named tables are views into it.
// GLOBAL: CMR2 0x0058d2a0
extern BYTE g_stageBlock[0x430];

#define g_carSteeringBlendLastGears (*(BYTE (*)[2])(g_stageBlock + 0x30))
#define g_carSteeringBlendModes (*(int (*)[2])(g_stageBlock + 0xc0))
#define g_carSteeringBlendFrames (*(BYTE (*)[2])(g_stageBlock + 0x1d8))
#define g_carAppliedRevTextureLevels (*(unsigned short (*)[2][12])(g_stageBlock + 0x0))
#define g_carRequestedDigitTextureAlphas (*(WORD (*)[2][7])(g_stageBlock + 0x34))
#define g_carWiperDirectionReversed ((int *)(g_stageBlock + 0x50)) // two cockpit slots
// Per object: the 12 ushort fill values of the stage box outline (0x477460).
#define g_carRequestedRevTextureLevels (*(unsigned short (*)[2][12])(g_stageBlock + 0x70))
#define g_carDetailBodyNodes (*(SceneNode *(*)[8])(g_stageBlock + 0xa0))
#define g_carDriverPoseNeedsReset (*(int (*)[2])(g_stageBlock + 0x110))
#define g_carInteriorArchives (*(GenericFile (*)[16])(g_stageBlock + 0x118))
#define g_carDefaultBodyNodes (*(SceneNode *(*)[8])(g_stageBlock + 0x1dc))
#define g_carInteriorRoots (*(SceneNode *(*)[8])(g_stageBlock + 0x1fc))
#define g_carInteriorDashTextures (*(CarInteriorDashTextures (*)[2])(g_stageBlock + 0x220))
#define g_carDriverRestPositions (*(FixVector (*)[2])(g_stageBlock + 0x58))
#define g_carDriverPoseStates (*(CarDriverPoseState (*)[2])(g_stageBlock + 0xc8))
#define g_carInteriorTables (*(CarInteriorRuntimeTables *)(g_stageBlock + 0x230))
#define g_carInteriorModelClasses (g_carInteriorTables.modelClasses)
#define g_carWiperStates (g_carInteriorTables.wipers)
#define g_carInteriorProfilePointers (g_carInteriorTables.profiles)
#define g_carInteriorNodeRows (g_carInteriorTables.nodes)
#define g_carInteriorModelBuffers (*(void *(*)[2])(g_stageBlock + 0x400))
#define g_carInteriorLoaded (*(int (*)[2])(g_stageBlock + 0x408))
#define g_carAppliedDigitTextureAlphas (*(WORD (*)[2][7])(g_stageBlock + 0x410))

#endif
