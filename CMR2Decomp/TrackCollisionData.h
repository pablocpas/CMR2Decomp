#ifndef _TRACK_COLLISION_DATA_H
#define _TRACK_COLLISION_DATA_H
#include "FixedPoint.h"

// ON-DISK: pointer-free records in the stage collision block. All indices
// and counts stay fixed-width; the runtime views below hold native pointers.
struct TrackCollisionHeader {
    int x;                    // 0x00 grid origin x
    int z;                    // 0x04 grid origin z
    short levelCounts[5];     // 0x08 signed number of nodes per level
    short field_0x12;
};
struct TrackTriangle {
    unsigned short v[3];      // 0x00 vertex indices are read unsigned
    unsigned short surface : 7;
    unsigned short flags : 9;
};
struct TrackQuadNode {
    short triangleCount;      // 0x00 -1 means an internal node
    short field_0x2;
    int firstIndex;           // 0x04 first child or first triangle-list index
};
struct TrackCollisionCount {
    unsigned short count;     // 0x00 only the low word is consumed
    unsigned short field_0x2;
};
// Runtime-only contiguous pointer slots. Original traversal can reach slot 5,
// which aliases the vertex pointer after the five quadtree levels. Preserve
// that original edge case without depending on separate-global link order.
union TrackCollisionTables {
    struct {
        TrackQuadNode *levels[5];
        FixVector *vertices;
    };
    TrackQuadNode *traversalLevels[6];
};
extern TrackTriangle *g_trackTriangles;
extern TrackCollisionHeader *g_trackCollisionHeader;
extern TrackCollisionCount *g_trackTriangleCountRecord;
extern TrackCollisionTables g_trackCollisionTables;
#define g_trackVertices g_trackCollisionTables.vertices
extern short *g_trackTriangleIndices;
extern short *g_trackGridRows;
extern short *g_trackGridColumns;
extern TrackCollisionCount *g_trackVertexCountRecord;
extern BYTE *g_trackCollisionBlock;
extern int *g_trackTriangleListCount;
extern int g_trackLevelNodeCounts[5];
#endif
