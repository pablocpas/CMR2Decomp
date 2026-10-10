#ifndef _SECTOR_H
#define _SECTOR_H

#include "FixedPoint.h"

struct SceneNode;

// Track sector (grid cell); scene nodes are linked into the cell they sit in.
struct Sector {
    int x;                      // 0x0  centre, 16.16
    int y;                      // 0x4
    int z;                      // 0x8
    int field_0xc;
    struct SectorModel *pMesh;  // 0x10 ground mesh (LOD records of 0x108 bytes)
    struct StageObject *pObjects; // 0x14 static objects of the sector (linked by pNext)
    int field_0x18;
    SceneNode *pFirstNode;      // 0x1c
    int nodeCount;              // 0x20
    int finCount;               // 0x24 number of fin.dat records of the sector
    BYTE *pFinRecords;          // 0x28 head of the sector's fin record list
    FixVector corners[4];       // 0x2c per side: offset of the nearest ground vertex, then corner point
    int bounds[4][2];           // 0x5c x/z of the ground mesh bounding rectangle corners
};

// ON-DISK: a 0xa0-byte record inside the stage mesh file. The loader at
// 0x4b93c0 relocates pMesh (+0x0c) against its mesh array and pNext (+0x98)
// against the stage-object array, in place. The port must split this disk
// representation from native runtime pointers; do not resize the disk record.
// Static object placed in a sector (only the fields used so far).
struct StageObject {
    BYTE field_0x0[0xc];
    struct Mesh *pMesh;         // 0xc
    BYTE field_0x10[0x88];
    StageObject *pNext;         // 0x98 next object of the sector
    int lightLevel;             // 0x9c light level when not lit per vertex
};

// Ground mesh LOD record of a sector (0x108 bytes).
struct SectorMesh {
    BYTE field_0x0[0xc];
    float *pVertices;           // 0xc  0x30 bytes per vertex, x/y/z first
    unsigned int vertexCount;   // 0x10
    BYTE field_0x14[0xf4];
};
struct SectorModel {
    SectorMesh lod0;
    BYTE field_0x108[0xa];
    BYTE lodIndex;              // 0x112 LOD record in use
};

// GLOBAL: CMR2 0x0071f600
extern int g_sectorsPerRow;
// GLOBAL: CMR2 0x0071f608
extern Sector *g_sectors[14096];
// GLOBAL: CMR2 0x0072d248
extern int g_sectorHalfSize;
// GLOBAL: CMR2 0x0072d24c
extern int g_sectorSize;
// GLOBAL: CMR2 0x0072d458
extern int g_sectorRows;
// GLOBAL: CMR2 0x0072d258
extern int g_sectorVisibleBits[128];
// GLOBAL: CMR2 0x0072d55c
extern int g_sectorScale;
// GLOBAL: CMR2 0x0072d568
extern int g_sectorCount;
// GLOBAL: CMR2 0x0072d570
extern int g_sectorCullEnabled;
// GLOBAL: CMR2 0x0072d578
extern int g_sectorCullDisabled;
// GLOBAL: CMR2 0x006ed5e8
extern int g_unk0x006ed5e8;
// GLOBAL: CMR2 0x006ed5ec
extern int g_unk0x006ed5ec;
// GLOBAL: CMR2 0x006ef5f0
extern int g_sectorOriginX;
// GLOBAL: CMR2 0x006ef5f4
extern int g_sectorOriginZ;

int Sector_IsVisible(int iSector);
int Sector_FromPosition(FixVector *pPos);
void SceneNode_UpdateSector(SceneNode *pNode);

#endif
