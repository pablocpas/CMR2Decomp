#ifndef _FIN_RECORD_H
#define _FIN_RECORD_H

#include "FixedPoint.h"

// ON-DISK: a record of the stage's fin.dat (0x30 bytes), one collision edge
// (barriers, walls) of a sector. In the file +0x28 holds the sector index;
// Sector_RebuildStageGrid (0x4b8270) overwrites it in place with the link to
// the next edge of that sector, so it holds a pointer only on Win32.
struct FinRecord {
    FixVector target;           // 0x00 g_collisionTarget
    FixVector lineStart;        // 0x0c g_collisionLineStart
    FixVector direction;        // 0x18 g_collisionDirection
    int field_0x24;
    union {
        int sector;             // 0x28 in the file
        FinRecord *pNext;       // 0x28 once linked into the sector's list
    };
    char type;                  // 0x2c edge type (0x19 ...)
    BYTE flags;                 // 0x2d bit 0: parallel-side test
    BYTE surfaces[2];           // 0x2e surface descriptor per tracked point, 0xff none
};

#endif
