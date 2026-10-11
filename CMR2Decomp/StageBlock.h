#ifndef _STAGE_BLOCK_H
#define _STAGE_BLOCK_H

#include <windows.h>
#include "CarInterior.h"
#include "GenericFileLoader.h"

struct SceneNode;

// Stage object tables 0x58d2a0..0x58d6d0, one global in the original. Several
// loops walk across neighbouring tables (rows of 12 bytes, pairs of pointers),
// so the tables stay members of one object, in their original order.
struct StageBlock {
    unsigned short carAppliedRevTextureLevels[2][12];   // 0x000
    BYTE carSteeringBlendLastGears[2];                  // 0x030
    BYTE field_0x32[2];
    WORD carRequestedDigitTextureAlphas[2][7];          // 0x034
    int carWiperDirectionReversed[2];                   // 0x050 two cockpit slots
    FixVector carDriverRestPositions[2];                // 0x058
    // Per object: the 12 ushort fill values of the stage box outline (0x477460).
    unsigned short carRequestedRevTextureLevels[2][12]; // 0x070
    SceneNode *carDetailBodyNodes[8];                   // 0x0a0
    int carSteeringBlendModes[2];                       // 0x0c0
    CarDriverPoseState carDriverPoseStates[2];          // 0x0c8
    int carDriverPoseNeedsReset[2];                     // 0x110
    GenericFile carInteriorArchives[16];                // 0x118
    BYTE carSteeringBlendFrames[2];                     // 0x1d8
    BYTE field_0x1da[2];
    SceneNode *carDefaultBodyNodes[8];                  // 0x1dc
    SceneNode *carInteriorRoots[8];                     // 0x1fc
    BYTE field_0x21c[4];
    CarInteriorDashTextures carInteriorDashTextures[2]; // 0x220
    CarInteriorRuntimeTables carInteriorTables;         // 0x230
    FixMatrix carDriverPoseMatrix;                      // 0x2c0
    BYTE field_0x300[0x100];
    void *carInteriorModelBuffers[2];                   // 0x400
    int carInteriorLoaded[2];                           // 0x408
    WORD carAppliedDigitTextureAlphas[2][7];            // 0x410
    BYTE field_0x42c[4];
};

// GLOBAL: CMR2 0x0058d2a0
extern StageBlock g_stageBlock;

#define g_carSteeringBlendLastGears (g_stageBlock.carSteeringBlendLastGears)
#define g_carSteeringBlendModes (g_stageBlock.carSteeringBlendModes)
#define g_carSteeringBlendFrames (g_stageBlock.carSteeringBlendFrames)
#define g_carAppliedRevTextureLevels (g_stageBlock.carAppliedRevTextureLevels)
#define g_carRequestedDigitTextureAlphas (g_stageBlock.carRequestedDigitTextureAlphas)
#define g_carWiperDirectionReversed (g_stageBlock.carWiperDirectionReversed)
#define g_carRequestedRevTextureLevels (g_stageBlock.carRequestedRevTextureLevels)
#define g_carDetailBodyNodes (g_stageBlock.carDetailBodyNodes)
#define g_carDriverPoseNeedsReset (g_stageBlock.carDriverPoseNeedsReset)
#define g_carInteriorArchives (g_stageBlock.carInteriorArchives)
#define g_carDefaultBodyNodes (g_stageBlock.carDefaultBodyNodes)
#define g_carInteriorRoots (g_stageBlock.carInteriorRoots)
#define g_carInteriorDashTextures (g_stageBlock.carInteriorDashTextures)
#define g_carDriverRestPositions (g_stageBlock.carDriverRestPositions)
#define g_carDriverPoseStates (g_stageBlock.carDriverPoseStates)
#define g_carInteriorTables (g_stageBlock.carInteriorTables)
#define g_carInteriorModelClasses (g_carInteriorTables.modelClasses)
#define g_carWiperStates (g_carInteriorTables.wipers)
#define g_carInteriorProfilePointers (g_carInteriorTables.profiles)
#define g_carInteriorNodeRows (g_carInteriorTables.nodes)
#define g_carInteriorModelBuffers (g_stageBlock.carInteriorModelBuffers)
#define g_carInteriorLoaded (g_stageBlock.carInteriorLoaded)
#define g_carAppliedDigitTextureAlphas (g_stageBlock.carAppliedDigitTextureAlphas)
#define g_carDriverPoseMatrix (g_stageBlock.carDriverPoseMatrix)

#endif
