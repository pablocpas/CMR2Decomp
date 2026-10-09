#ifndef _NETPLAYERS_H
#define _NETPLAYERS_H


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
// NetPlayer::flags as the original's bitfields (it merges adjacent constant
// field stores into one and/or).
struct NetPlayerFlags {
    unsigned car : 5;       // bits 0-4
    unsigned bit5 : 1;
    unsigned bit6 : 1;
    unsigned active : 1;    // bit 7
    unsigned ready : 1;     // bit 8
    unsigned finished : 1;  // bit 9
    unsigned rest : 22;
};

struct NetPlayerInfo {
    int id;
    union {
        unsigned int flags;
        NetPlayerFlags bits;
    };
    int field_0x8;
    int field_0xc;
};

struct NetPlayer {
    int id;                     // 0x00 NetPlayerID
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

// The network tables (0x531778-0x531dfc) are separate globals.
extern NetPlayer g_netPlayers[8];
extern int g_netRanks[8];
extern int g_netIdsUnsorted[8];
extern NetResult g_netResults[8];
extern unsigned int g_netStageBest[10];
extern int g_netIds[8];
extern unsigned int g_netPrevBest;
extern int g_netNewRecord;
extern int g_netTotal;
extern int g_netStandingCount;
extern char g_netRecordName[0xe8];
extern int g_netRanks2[8];
extern unsigned int g_netBestTime;

extern NetStanding g_netStandings[8];
extern NetStanding g_netStandings2[8];
extern int g_netClassCount;
extern unsigned int g_netSplitBest[8];

void NetPlayers_ResetAllTables(void);
void NetPlayers_ResetStageState(char keepReady, char resetTotal);
void NetPlayers_ResetBestTimes(void);
void NetPlayers_ClearReadyFlags(void);
unsigned int NetPlayers_IsPlayerPresent(int index);
char *NetPlayers_GetPlayerName(int index);
unsigned int NetPlayers_GetCarSelection(int index);
int NetPlayers_GetPlayerID(int index);
int NetPlayers_FindPlayerIndexByID(int id);



// Triangular distance table and the adjacent network enable flag.
struct NetTriangleState {
    int values[100];
    BYTE enabled;
};
typedef char NetTriangleStateSize[sizeof(NetTriangleState) == 0x194 ? 1 : -1];
extern NetTriangleState g_netTriangleState;
#define g_triangleNumbers (g_netTriangleState.values)
#define g_unk0x00539cc8 (g_netTriangleState.enabled)

#endif
