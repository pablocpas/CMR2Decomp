#ifndef _RACE_RECORD_H
#define _RACE_RECORD_H
#include <stddef.h>

// A numeric cursor is retained only where MSVC6 requires its alias behavior.
typedef char RaceRecordAddressWidthCheck[sizeof(size_t) >= sizeof(void *) ? 1 : -1];

// Runtime route progress per car; pointer-free, also copied into replay snapshots.
struct RaceRecord {
    int routeNodeIndex;       // 0x00 current node passed to route queries
    int resetRouteNodeIndex;  // 0x04 nearest initial node restored before restart
    int field_0x8;
    int field_0xc;
    int field_0x10;
    short field_0x14;
    short field_0x16;
};

#endif
