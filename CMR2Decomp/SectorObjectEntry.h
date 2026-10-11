#ifndef _SECTOR_OBJECT_ENTRY_H
#define _SECTOR_OBJECT_ENTRY_H

#include <windows.h>
struct StageObject;
// Runtime projection written from eight rotated collision corners.
struct ObjectPointBounds {
    short directionX; // 0x0 normalized horizontal axis, fixed value >> 9
    short directionZ; // 0x2
    short top;        // 0x4 maximum y >> 9
    short bottom;     // 0x6 minimum y >> 9
    short halfWidth;  // 0x8 maximum positive axis projection >> 9
    short halfLength; // 0xa maximum positive perpendicular projection >> 9
};
struct SectorObjectBounds {
    int radius;                    // 0x0 bounding sphere radius
    ObjectPointBounds *pProjection; // 0x4 null for spherical records
};
// Runtime tables built after relocation, not records loaded from disk.
struct SectorObjectEntry {
    StageObject *pObject;           // 0x0 placement shared by mesh records
    SectorObjectBounds *pBounds;    // 0x4 bounds of this mesh collision record
};
#endif
