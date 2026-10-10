#ifndef _CAR_RESOURCES_H
#define _CAR_RESOURCES_H

#include <windows.h>
#include "FixedPoint.h"

struct SceneNode;
struct CarInfoDirectory;

// One loaded car scene. Wheel objects are saved before the optional wheel
// scene replaces them and restored before the main scene is destroyed.
struct CarSceneRecord {
    BYTE modelClass;                    // 0x00 handling/body class from the model archive
    BYTE field_0x1[3];                   // 0x01 semantics not established
    SceneNode *bodyNode;                 // 0x04 node id 5
    SceneNode *rootNode;                 // 0x08 loaded scene, contains wheel node ids 1..4
    SceneNode *alternateWheelScene;      // 0x0c snow/light wheel replacement scene
    void *originalWheelObjects[4];       // 0x10 objects saved from wheel nodes 1..4
    BYTE detailCode;                    // 0x20 asset detail letter
    BYTE variantCode;                   // 0x21 alternate body asset letter
    BYTE field_0x22[2];                  // 0x22 semantics not established
};

// Independently loaded/cleared/freed regions of the original address block.
// GLOBAL: CMR2 0x00542630
extern CarSceneRecord g_carScenes[16];
// GLOBAL: CMR2 0x00542870
extern BYTE *g_carStartData;
// GLOBAL: CMR2 0x00542874
extern FixAngles *g_carStartAngles;
// GLOBAL: CMR2 0x00542878
extern FixVector g_carStartPosition;
// GLOBAL: CMR2 0x00542884
extern void *g_carAuxiliaryBuffers[16];
// GLOBAL: CMR2 0x005428c4
extern SceneNode *g_carAlternateBodyScenes[16];
// GLOBAL: CMR2 0x00542904
extern void *g_carWheelModelBuffers[16];
// GLOBAL: CMR2 0x00542944
extern void *g_carAlternateBodyBuffers[16];
// GLOBAL: CMR2 0x00542984
extern void *g_carModelBuffers[16];
// GLOBAL: CMR2 0x005429c4
extern BYTE g_knockoutPlayerCarDetail;
// GLOBAL: CMR2 0x005429c5
extern BYTE g_knockoutOpponentCarDetail;
// GLOBAL: CMR2 0x005429c6
extern BYTE g_unk0x005429c6[2];
// GLOBAL: CMR2 0x005429c8
extern CarInfoDirectory *g_carInfoBuffers[8];
// GLOBAL: CMR2 0x005429e8
extern BYTE g_unk0x005429e8[0xc8];
CarSceneRecord *StageTiming_GetStartTableRecord(int index);

#endif
