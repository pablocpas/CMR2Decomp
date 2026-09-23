#include <windows.h>
#include <string.h>
#include "NetPlayers.h"

// Network messages sent during a race (0x427620-0x428760)

char FUN_004a1c50(int to, int guaranteed, int data, int size);
int FUN_0040ac30(void);
void FUN_0040afd0(void);

// GLOBAL: CMR2 0x00539cc8
BYTE g_unk0x00539cc8;
// GLOBAL: CMR2 0x00539dcc
int g_unk0x00539dcc;
// GLOBAL: CMR2 0x00539ed0
int g_unk0x00539ed0;
// GLOBAL: CMR2 0x00539ed8
BYTE g_unk0x00539ed8;
// GLOBAL: CMR2 0x0053a04c
int g_unk0x0053a04c[8];
// GLOBAL: CMR2 0x0053a06c
int g_unk0x0053a06c[8];
// GLOBAL: CMR2 0x0053a0ac
int g_unk0x0053a0ac[8];
// GLOBAL: CMR2 0x0053a0cc
int g_unk0x0053a0cc[8];
// GLOBAL: CMR2 0x005394bc
BYTE g_unk0x005394bc[8][0xec];

// FUNCTION: CMR2 0x00427620
int FUN_00427620(int index)
{
    return *(int *)g_unk0x005394bc[index];
}

// FUNCTION: CMR2 0x00427640
void FUN_00427640(BYTE param1)
{
    g_unk0x00539cc8 = param1;
}

// FUNCTION: CMR2 0x00427650
void FUN_00427650(void)
{
    g_unk0x00539ed0 = -1;
    g_unk0x00539dcc = -1;
}

// FUNCTION: CMR2 0x00427660
int FUN_00427660(void)
{
    return g_unk0x00539ed0;
}

// FUNCTION: CMR2 0x00427670
int FUN_00427670(void)
{
    return g_unk0x00539dcc;
}

// Sends the local car state.
// FUNCTION: CMR2 0x004278f0
void FUN_004278f0(NetStats *pStats)
{
    struct {
        BYTE type;
        BYTE pad;
        NetStats stats;
    } msg;

    msg.type = 0xb;
    msg.stats = *pStats;
    FUN_004a1c50(0, 0, (int)&msg, sizeof(msg));
}

// FUNCTION: CMR2 0x00427930
void FUN_00427930(void)
{
    BYTE msg;

    msg = 7;
    FUN_004a1c50(0, 1, (int)&msg, 1);
}

// Sends the stage time and the accumulated total.
// FUNCTION: CMR2 0x00427950
void FUN_00427950(int time)
{
    struct {
        BYTE type;
        int time;
        int total;
    } msg;

    msg.type = 0xa;
    msg.time = time;
    msg.total = FUN_0040ac30() + time;
    FUN_004a1c50(0, 1, (int)&msg, sizeof(msg));
}

// FUNCTION: CMR2 0x00427990
void FUN_00427990(int index, int value)
{
    struct {
        BYTE type;
        BYTE index;
        int value;
    } msg;

    g_unk0x00539ed0 = index;
    msg.type = 8;
    msg.value = value;
    msg.index = index;
    FUN_004a1c50(0, 0, (int)&msg, sizeof(msg));
}

// FUNCTION: CMR2 0x004279d0
void FUN_004279d0(int index, int value)
{
    struct {
        BYTE type;
        BYTE index;
        int value;
    } msg;

    g_unk0x00539dcc = index;
    msg.type = 9;
    msg.value = value;
    msg.index = index;
    FUN_004a1c50(0, 1, (int)&msg, sizeof(msg));
}

// FUNCTION: CMR2 0x00427a10
void FUN_00427a10(void)
{
    BYTE msg;

    msg = 0xc;
    FUN_004a1c50(0, 1, (int)&msg, 1);
}

// FUNCTION: CMR2 0x00427a30
void FUN_00427a30(void)
{
    BYTE msg;

    msg = 0xd;
    FUN_004a1c50(0, 1, (int)&msg, 1);
    FUN_0040afd0();
}

// FUNCTION: CMR2 0x00427a50
void FUN_00427a50(void)
{
    BYTE msg;

    msg = 0xe;
    FUN_004a1c50(0, 1, (int)&msg, 1);
    FUN_00427640(0);
    FUN_0040afd0();
}

// FUNCTION: CMR2 0x00427a80
void FUN_00427a80(void)
{
    BYTE msg;

    msg = 0xf;
    FUN_004a1c50(0, 1, (int)&msg, 1);
    FUN_0040afd0();
}

// FUNCTION: CMR2 0x00427aa0
BYTE FUN_00427aa0(void)
{
    return g_unk0x00539ed8;
}

// FUNCTION: CMR2 0x00427ab0
bool FUN_00427ab0(int value, int *pRange)
{
    if (value < pRange[1])
        return false;
    if (value > pRange[2])
        return false;
    return true;
}

// FUNCTION: CMR2 0x004283b0
void FUN_004283b0(void)
{
    memset(g_unk0x0053a0cc, 0, sizeof(g_unk0x0053a0cc));
    memset(g_unk0x0053a06c, 0, sizeof(g_unk0x0053a06c));
    memset(g_unk0x0053a0ac, 0, sizeof(g_unk0x0053a0ac));
}

// FUNCTION: CMR2 0x00428740
int FUN_00428740(BYTE index)
{
    return g_unk0x0053a0ac[index] != 0;
}

// FUNCTION: CMR2 0x00428760
int FUN_00428760(BYTE index)
{
    if (FUN_00428740(index) && g_unk0x0053a04c[index] > 0)
        return 1;
    return 0;
}
