#ifndef _OPTION_PREVIEW_GEOMETRY_H
#define _OPTION_PREVIEW_GEOMETRY_H

#include "DeformGeometry.h"

// Reference hull points for fourteen model classes: twelve selectable parts,
// four vertices per part. The presets/projected quads use the same part order.
struct PreviewModelReferenceGeometry {
    FixVector partVertices[12][4];
};

struct PreviewScreenPoint {
    int x;
    int y;
};

struct PreviewProjectedQuad {
    PreviewScreenPoint points[4];
};

// Signed 12-bit angle views used by orientation interpolation. The last word
// is copied with targets but has no established meaning in this family.
struct PreviewSignedAngles {
    short x;
    short y;
    short z;
    short field_0x6;
};

// One preview model and its normal/L/S wheel variants (0x54 bytes).
struct OptionPreviewSceneRecord {
    BYTE modelClass;                  // 0x00 low byte written/read by model selection
    BYTE field_0x1[3];
    SceneNode *bodyNode;               // 0x04 node type 5
    SceneNode *rootNode;               // 0x08 complete normal model tree
    SceneNode *lowWheelRoot;           // 0x0c variant L model tree
    SceneNode *sixWheelRoot;           // 0x10 variant S model tree
    SceneNode *wheelNodes[4];          // 0x14 normal model's nodes 1..4
    Mesh *normalWheelMeshes[4];        // 0x24 original wheel objects to restore
    Mesh *lowWheelMeshes[4];           // 0x34 wheel objects of variant L
    Mesh *sixWheelMeshes[4];           // 0x44 wheel objects of variant S
};

// Converted preview meshes and per-model damage/visibility (0x2ac bytes).
// The count widths differ from CarPartSet; only geometry is shared.
struct OptionPreviewMeshRecord {
    DeformMeshSources geometry;        // 0x000
    FixVector centre[15];              // 0x0b4
    FixVector halfSize[15];            // 0x168
    int maxX;                         // 0x21c
    int minX;                         // 0x220
    int maxZ;                         // 0x224
    int minZ;                         // 0x228
    int cornerHeightOffsets[4];       // 0x22c derived damage channels 6..9
    int partDamageFractions[4];       // 0x23c derived damage channels 0..3
    WORD vertexCount[15];              // 0x24c unsigned, distinct from CarPartSet
    BYTE meshCount;                   // 0x26a
    int meshVisible[15];               // 0x26c
    int verticesAssigned;              // 0x2a8 converted sources have been attached
};

// Box and control points used by preview deformation (0x138 bytes).
struct OptionPreviewDeformGeometry {
    OptionPreviewSceneRecord *pEntry;  // 0x000
    FixVector corner[8];               // 0x004
    FixVector vertex[12];              // 0x064
    FixVector anchor[4];               // 0x0f4
    int animationPhaseDegrees;         // 0x124 wraps at 360 in 16.16
    int anchorPhaseDegrees[4];         // 0x128 random initial phase, same units
};

extern OptionPreviewMeshRecord g_previewMeshRecords[16];
extern OptionPreviewSceneRecord g_previewSceneRecords[16];
extern OptionPreviewDeformGeometry g_previewDeformGeometry[8];
extern DeformVertex **g_previewOriginalVertices[16];
extern BYTE *g_previewOriginalNodeTypes[16];

#endif
