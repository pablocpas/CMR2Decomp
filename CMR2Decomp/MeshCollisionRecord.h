#ifndef _MESH_COLLISION_RECORD_H
#define _MESH_COLLISION_RECORD_H
#include <windows.h>
#include "FixedPoint.h"
// ON-DISK: 0x5c-byte mesh collision record; the stage mesh loader at
// 0x4b93c0 relocates pNext (+0x58) in place. Mesh::pCollisionRecords
// (+0x10c) points into this array. The port must split disk/native forms.
struct MeshCollisionRecord {
    BYTE type;                 // 0x00 zero: oriented box, otherwise sphere
    BYTE field_0x1[3];
    FixMatrix matrix;          // 0x04 local collision transform
    int width;                 // 0x44
    int length;                // 0x48
    int height;                // 0x4c
    BYTE field_0x50[8];
    MeshCollisionRecord *pNext; // 0x58
};
#endif
