#ifndef _SECTOR_H
#define _SECTOR_H

#include "FixedPoint.h"

struct SceneNode;

// Track sector (grid cell); scene nodes are linked into the cell they sit in.
struct Sector {
    int x;                      // 0x0  centre, 16.16
    int y;                      // 0x4
    int z;                      // 0x8
    BYTE field_0xc[0x10];
    SceneNode *pFirstNode;      // 0x1c
    int nodeCount;              // 0x20
};

// GLOBAL: CMR2 0x0071f600
extern int g_sectorsPerRow;
// GLOBAL: CMR2 0x0071f608
extern Sector *g_sectors[14096];
// GLOBAL: CMR2 0x0072d248
extern int g_sectorHalfSize;
// GLOBAL: CMR2 0x0072d258
extern int g_sectorVisibleBits[198];
// GLOBAL: CMR2 0x0072d55c
extern int g_sectorScale;
// GLOBAL: CMR2 0x0072d568
extern int g_sectorCount;
// GLOBAL: CMR2 0x0072d570
extern int g_sectorCullEnabled;
// GLOBAL: CMR2 0x0072d578
extern int g_sectorCullDisabled;
// GLOBAL: CMR2 0x006ef5f0
extern int g_sectorOriginX;
// GLOBAL: CMR2 0x006ef5f4
extern int g_sectorOriginZ;

int Sector_IsVisible(int iSector);
int Sector_FromPosition(FixVector *pPos);
void SceneNode_UpdateSector(SceneNode *pNode);

#endif
