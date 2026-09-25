#include <windows.h>
#include <string.h>
#include "NetPlayers.h"
#include "GameInfo.h"
#include "RallyData.h"
#include "FixedPoint.h"

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
// GLOBAL: CMR2 0x0053a00c
int g_unk0x0053a00c[8];
// GLOBAL: CMR2 0x0053a04c
int g_unk0x0053a04c[8];
// GLOBAL: CMR2 0x0053a06c
int g_unk0x0053a06c[8];
// Per-player fade: the callback runs when the fade (mode) completes.
typedef void (*FadeCallback)(BYTE index);
// GLOBAL: CMR2 0x0053a08c
FadeCallback g_fadeCallbacks[8];
// GLOBAL: CMR2 0x0053a0ac
int g_unk0x0053a0ac[8];
// GLOBAL: CMR2 0x0053a0cc
int g_unk0x0053a0cc[8];
// GLOBAL: CMR2 0x0053a0ec
int g_unk0x0053a0ec[8];
// GLOBAL: CMR2 0x0053a20c
int g_unk0x0053a20c[8];
// GLOBAL: CMR2 0x005394bc
BYTE g_unk0x005394bc[8][0xec];

// Font picked for the screen size (0x28 normal, 0x29 small).
// GLOBAL: CMR2 0x005393d4
int g_unk0x005393d4;

// Chooses the HUD font from the screen size and the number of players shown.
// TODO: CMR2 0x00427580 (implemented, match 68%)
void FUN_00427580(int width, int height, int players)
{
    float rowWidth = (float)(players * 0xc0 + 0x230);
    float scaleX = (float)width / rowWidth;
    float scaleY = (float)height / ((float)players * rowWidth);
    float scale;

    scale = scaleY;
    if (scaleX <= scaleY)
        scale = scaleX;
    if (scale > 30.0f)
        scale = 30.0f;
    g_unk0x005393d4 = 0x28;
    if ((1.0f / scale) * 1000.0f - 40.0f > 0.0f)
        g_unk0x005393d4 = 0x29;
}

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

// Starts a fade of the player's screen; an active fade is only replaced
// (running its callback first) when force is set.
// FUNCTION: CMR2 0x00428410
void FUN_00428410(BYTE index, int speed, FadeCallback pfnDone, int mode, int param5, int param6, int param7,
                  char force)
{
    if (FUN_00428740((BYTE)index)) {
        if (!force)
            return;
        if (g_fadeCallbacks[index & 0xff] != NULL)
            g_fadeCallbacks[index & 0xff](index);
    }
    g_unk0x0053a0ec[index & 0xff] = speed;
    g_unk0x0053a00c[index & 0xff] = param7;
    g_unk0x0053a04c[index & 0xff] = mode == 2 ? -speed : 0;
    g_fadeCallbacks[index & 0xff] = pfnDone;
    g_unk0x0053a0ac[index & 0xff] = mode;
    if ((mode == 0 || mode == 2) && pfnDone != NULL)
        pfnDone(index);
    if (mode == 0 || mode == 4)
        g_unk0x0053a06c[index & 0xff] = 0;
    g_unk0x0053a20c[index & 0xff] = param5;
    g_unk0x0053a0cc[index & 0xff] = param6;
}

// Fades the player's screen out (mode 3) and runs pfnDone when it is done.
// FUNCTION: CMR2 0x004283e0
void FUN_004283e0(BYTE index, FadeCallback pfnDone, int param3, int param4, int param5, char force)
{
    FUN_00428410(index, 0xc8000, pfnDone, 3, param3, param4, param5, force);
}

// FUNCTION: CMR2 0x00428760
int FUN_00428760(BYTE index)
{
    if (FUN_00428740(index) && g_unk0x0053a04c[index] > 0)
        return 1;
    return 0;
}

void FUN_0047aa60(int value);
void FUN_0041b040(int value);
void FUN_00418560(int value);
void FUN_00418d20(int value);

// Linearly interpolated lookup in a byte curve {count, min, max, -, bytes}.
// TODO: CMR2 0x00427ad0 (implemented, match 79%)
unsigned int FUN_00427ad0(int value, int *pCurve)
{
    int range = pCurve[2] - pCurve[1];
    int index;
    int step;
    int frac;

    value -= pCurve[1];
    index = (pCurve[0] * value) / range;
    step = range / pCurve[0];
    if (step > 0) {
        frac = ((value % step) * 100) / step;
        return (int)((100 - frac) * ((BYTE *)pCurve[4])[index]) / 100 +
               (int)(((BYTE *)pCurve[4])[index + 1] * frac) / 100;
    }
    return ((BYTE *)pCurve[4])[index];
}

// Linearly interpolated lookup in a 16-bit curve {count, min, max, words}.
// TODO: CMR2 0x00427b70 (implemented, match 80%)
unsigned int FUN_00427b70(int value, int *pCurve)
{
    int range = pCurve[2] - pCurve[1];
    int index;
    int step;
    int frac;

    value -= pCurve[1];
    index = (pCurve[0] * value) / range;
    step = range / pCurve[0];
    if (step > 0) {
        frac = ((value % step) * 100) / step;
        return (int)(((unsigned short *)pCurve[3])[index + 1] * frac) / 100 +
               (int)(((unsigned short *)pCurve[3])[index] * (100 - frac)) / 100;
    }
    return ((unsigned short *)pCurve[3])[index];
}

// Race start: decides g_unk0x00539ed8 from the game mode and applies the
// volume settings (percentages scaled to 16.16).
// FUNCTION: CMR2 0x00427c10
void FUN_00427c10(void)
{
    if (CGameInfo::FUN_00405d80() != 4 && CGameInfo::FUN_00405d80() != 5 && CGameInfo::FUN_00405d80() != 6 &&
        (CGameInfo::FUN_00405d70() != 2 || CGameInfo::FUN_00405da0()) &&
        ((BYTE)RallyDataState() != 1 || !(BYTE)RallyData_GetFlag25() || CGameInfo::FUN_00405e00() ||
         CGameInfo::FUN_00405d80() == 3))
        g_unk0x00539ed8 = 0;
    else
        g_unk0x00539ed8 = 1;
    FUN_0047aa60(FixMul((int)(CGameInfo::FUN_00405e70() << 16) / 100, 0x5555));
    FUN_0041b040(FixMul((int)(CGameInfo::FUN_00405e70() << 16) / 100, 0x5555));
    FUN_00418560(FixMul((int)(CGameInfo::FUN_00405ea0() << 16) / 100, 0x10000));
    FUN_00418d20(FixMul((int)(CGameInfo::FUN_00405e70() << 16) / 100, 0xaaaa));
}

