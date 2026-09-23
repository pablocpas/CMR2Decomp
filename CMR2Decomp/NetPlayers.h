#ifndef _NETPLAYERS_H
#define _NETPLAYERS_H

#include <windows.h>

// Players of a network game (0x409a30-0x40b1e0): one 0x80 byte record per
// DirectPlay player plus the result and standings tables built from them.

// Car state broadcast by each player, 0x1e bytes
struct NetStats {
    unsigned short seq;         // 0x0  newer packets have a higher value
    BYTE data[0x14];            // 0x2
    unsigned short speed;       // 0x16 top 6 bits
    unsigned short field_0x18;  // 0x18 low 10 bits
    unsigned short field_0x1a;  // 0x1a bits 11-14 value, bit 15 sign
    unsigned short field_0x1c;  // 0x1c
};

// First 16 bytes of a NetPlayer, as sent over the network
struct NetPlayerInfo {
    int id;
    unsigned int flags;
    int field_0x8;
    int field_0xc;
};

struct NetPlayer {
    int id;                     // 0x00 DPID
    unsigned int flags;         // 0x04 bits 0-4 car, 5, 6, 7 active, 8 ready, 9 finished,
                                //      18-21 class, 22, 23
    int field_0x8;              // 0x08
    int field_0xc;              // 0x0c packets received
    NetStats stats;             // 0x10
    BYTE statsNew;              // 0x2e
    BYTE pad_0x2f;
    unsigned int splits[7];     // 0x30
    unsigned int stageTimes[10]; // 0x4c
    unsigned int time;          // 0x74
    unsigned int bestTime;      // 0x78
    int field_0x7c;             // 0x7c
};

// Sorted results of a stage, 0x1c bytes
struct NetResult {
    int index;                  // 0x00 player index, -1 = none, -2 = local player
    int field_0x4;
    int field_0x8;
    int field_0xc;
    int id;                     // 0x10
    BYTE field_0x14;
    int field_0x18;
};

// Standings entry, 0x18 bytes
struct NetStanding {
    int index;                  // 0x00
    unsigned int time;          // 0x04
    int id;                     // 0x08
    char name[4];               // 0x0c
    unsigned int car;           // 0x10
    int points;                 // 0x14
};

// Final classification entry, 0x10 bytes
struct NetClassification {
    char name[4];               // 0x0
    unsigned int carClass;      // 0x4
    unsigned int time;          // 0x8
    int id;                     // 0xc
};

extern NetPlayer g_netPlayers[8];
extern unsigned int g_netStageBest[10];

void FUN_00409a30(void);
void FUN_00409ab0(char keepReady, char resetTotal);
void FUN_00409b60(void);
void FUN_00409bc0(void);
unsigned int FUN_00409cb0(int index);
char *FUN_00409cd0(int index);
unsigned int FUN_00409d00(int index);
int FUN_00409d20(int index);
int FUN_0040a7a0(int id);

#endif
