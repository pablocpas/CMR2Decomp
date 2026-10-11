#ifndef _RALLY_ROUTE_NODE_H
#define _RALLY_ROUTE_NODE_H
#include <windows.h>
#include "FixedPoint.h"
// ON-DISK: pointer-free 0x2c-byte .cat node; the loader at 0x420630
// reads an array of these records without relocation. Same layout on 64 bits.
struct RallyRouteNode {
    FixVector position;          // 0x00 ground-position consumers use x and z
    BYTE field_0xc[0xc];
    BYTE flags;                  // 0x18 bit 0 checkpoint, bit 1 disallows reset
    char callId;                 // 0x19 signed -1 sentinel, otherwise .cod index
    BYTE field_0x1a[2];
    int backwardNearLimit;       // 0x1c camera distance override while reversing
    int backwardFarLimit;        // 0x20
    int forwardNearLimit;        // 0x24 camera distance override while advancing
    int forwardFarLimit;         // 0x28
};
#endif
