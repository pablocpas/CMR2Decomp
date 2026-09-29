#include <windows.h>
#include <string.h>
#include "NetPlayers.h"
#include "Sprite.h"
#include "Graphics.h"
#include "GameInfo.h"
#include "RallyData.h"
#include "FixedPoint.h"
#include "main.h"

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
// GLOBAL: CMR2 0x0053a02c
int g_unk0x0053a02c[8];
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
BYTE g_unk0x005394bc[7][0xec];  // 7 rows up to the triangle table at 0x539b38

// Font picked for the screen size (0x28 normal, 0x29 small).
// GLOBAL: CMR2 0x005393d4
int g_unk0x005393d4;

// Chooses the HUD font from the screen size and the number of players shown.
// match 68%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00427580
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

// GLOBAL: CMR2 0x00539ed4
int g_unk0x00539ed4;

void FUN_0040ac70(DWORD *pId, unsigned int carClass);
void FUN_00409e90(DWORD *pId);
void FUN_00409fd0(DWORD *pId, int split, unsigned int time);
void FUN_0040a0e0(DWORD *pId, int stage, unsigned int time);
void FUN_00409f80(DWORD *pId);
void FUN_00409f00(DWORD *pId, unsigned int time, int value);
void FUN_00409d50(DWORD *pId, NetStats *pStats);
void FUN_0040afb0(char valid, BYTE *p);
void FUN_0040ad20(void);
void FUN_004d0620(DWORD *pFrom, char *text, char local);
void FUN_0041f280(void);
void FUN_0041f290(void);
void FUN_00449fe0(BYTE index);
void FUN_00401540(BYTE index);
void FUN_004014f0(BYTE index);
void FUN_00449090(BYTE index);
void FUN_004283e0(BYTE index, FadeCallback pfnDone, int param3, int param4, int param5, char force);
extern int g_unk0x00537f34[2];
extern int g_unk0x005199b0;

// Dispatches a message of the in-race network stream to its handler.
// FUNCTION: CMR2 0x004276c0
void FUN_004276c0(DWORD *pId, BYTE *pPacket)
{
    switch (pPacket[0]) {
    case 0xb:
        if (g_unk0x00539cc8 != 0) {
            FUN_00409d50(pId, (NetStats *)(pPacket + 2));
            return;
        }
        break;
    case 7:
        FUN_00409e90(pId);
        return;
    case 8:
        FUN_00409fd0(pId, pPacket[1], *(unsigned int *)(pPacket + 4));
        return;
    case 9:
        FUN_0040a0e0(pId, pPacket[1], *(unsigned int *)(pPacket + 4));
        return;
    case 10:
        FUN_00409f80(pId);
        FUN_00409f00(pId, *(unsigned int *)(pPacket + 4), *(unsigned int *)(pPacket + 8));
        return;
    case 12:
        FUN_004283e0(0, FUN_00449090, 1, 0, g_unk0x005199b0, 1);
        return;
    case 13:
        FUN_004283e0(0, FUN_00449fe0, 1, 0, g_unk0x005199b0, 1);
        return;
    case 14:
        FUN_0041f280();
        if (CGameInfo::FUN_00405d80() == 8) {
            FUN_0041f290();
            FUN_004283e0(0, FUN_00401540, 1, 0, g_unk0x005199b0, 1);
        } else {
            FUN_004283e0(0, FUN_00401540, 1, 0, g_unk0x00539ed4, 1);
        }
        FUN_00409bc0();
        return;
    case 15:
        FUN_0041f280();
        FUN_0041f290();
        FUN_004283e0(0, FUN_004014f0, 1, 0, g_unk0x005199b0, 1);
        return;
    case 0:
        FUN_004d0620((DWORD *)pId, (char *)(pPacket + 1), 0);
        return;
    case 6:
        FUN_0040ac70(pId, pPacket[1]);
        return;
    case 0x10:
        g_unk0x00537f34[0] = CMain::GetFrameDelta();
        FUN_004283e0(0, FUN_00449090, 1, 0, g_unk0x005199b0, 1);
        return;
    case 0x11:
        FUN_0040afb0(pPacket[1], pPacket + 4);
        FUN_0040ad20();
        return;
    }
}

int FUN_004a1b90(int param1, void **param2);
void FUN_00427680(int param_1, int *param_2);
void FUN_004276c0(DWORD *pId, BYTE *pPacket);

// Drains the pending network messages of a race: system messages (from id 0)
// and player packets.
// FUNCTION: CMR2 0x00427890
void FUN_00427890(void)
{
    DWORD from;
    void *pData;

    while (FUN_004a1b90((int)&from, &pData)) {
        if (from == 0)
            FUN_00427680((int)&from, (int *)pData);
        else
            FUN_004276c0(&from, (BYTE *)pData);
    }
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

// GLOBAL: CMR2 0x00539ee0
FixVector g_unk0x00539ee0;
// GLOBAL: CMR2 0x00539ef0
FixVector g_unk0x00539ef0;
// GLOBAL: CMR2 0x00539f00
FixVector g_unk0x00539f00;

FixMatrix *FUN_00423d70(unsigned int index);
int FUN_0041f3a0(void);
unsigned int FixVec_Length(FixVector *pV);
extern BYTE g_unk0x00538d2c[0xc8];

// Loudness of a view's sounds by distance to the listener: 1 up to 2 units,
// fading to 0 at 100.
// match 87%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00427d50
int FUN_00427d50(unsigned int view, int listener)
{
    int index = 1;
    int distance;

    if (FUN_0041f3a0() == 0)
        index = listener;
    FixMatrix_GetPosition(&g_unk0x00539f00, FUN_00423d70(view));
    FixMatrix_GetPosition(&g_unk0x00539ee0, (FixMatrix *)(g_unk0x00538d2c + 4 + index * 100));
    g_unk0x00539ef0.x = g_unk0x00539f00.x - g_unk0x00539ee0.x;
    g_unk0x00539ef0.y = g_unk0x00539f00.y - g_unk0x00539ee0.y;
    g_unk0x00539ef0.z = g_unk0x00539f00.z - g_unk0x00539ee0.z;
    distance = FixVec_Length(&g_unk0x00539ef0);
    if (distance < 0x20000)
        return 0x10000;
    if (distance > 0x640000)
        return 0;
    return 0x10000 - FixDiv(distance - 0x20000, 0x620000);
}

// FUNCTION: CMR2 0x004283b0
void FUN_004283b0(void)
{
    memset(g_unk0x0053a0cc, 0, sizeof(g_unk0x0053a0cc));
    memset(g_unk0x0053a06c, 0, sizeof(g_unk0x0053a06c));
    memset(g_unk0x0053a0ac, 0, sizeof(g_unk0x0053a0ac));
}

int FUN_0041f3a0(void);
BYTE FUN_00422fb0(BYTE index);

int FUN_00428740(BYTE index);

extern int g_physicsTimeStep;

// Advances a player's flash timer; at the end it restarts (mode 3) or stops,
// and runs the player's fade callback.
// match 36%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004284d0
void FUN_004284d0(unsigned int player, int check)
{
    int step = g_physicsTimeStep;
    unsigned int p;
    int state;
    int value;

    if (FUN_00428740((BYTE)player) != 0) {
        p = player & 0xff;
        if (g_unk0x0053a0cc[p] == 0 || check == 0) {
            state = g_unk0x0053a0ac[p];
            if (state == 4) {
                g_unk0x0053a0ac[p] = 0;
                return;
            }
            g_unk0x0053a02c[p] = g_unk0x0053a04c[p];
            value = g_unk0x0053a04c[p] + step;
            g_unk0x0053a04c[p] = value;
            if (g_unk0x0053a02c[p] < 0 && value >= 0) {
                g_unk0x0053a0ac[p] = 4;
                g_unk0x0053a06c[p] = 0;
                return;
            }
            if (g_unk0x0053a0ec[p] <= value) {
                if (state == 3) {
                    value = -g_unk0x0053a0ec[p];
                    g_unk0x0053a04c[p] = value;
                    g_unk0x0053a02c[p] = value;
                } else {
                    g_unk0x0053a0ac[p] = 0;
                    g_unk0x0053a06c[p] = 0x10000;
                }
                if (g_fadeCallbacks[p] != NULL)
                    g_fadeCallbacks[p]((BYTE)player);
            }
        }
    }
}

// Sets a player's flash intensity: the value blended between its two ends
// by t, divided by its duration (magnitude only).
// match 69%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004285b0
void FUN_004285b0(unsigned int player, int t, int check)
{
    int value;

    if (FUN_00428740((BYTE)player) != 0) {
        player &= 0xff;
        if (g_unk0x0053a0cc[player] == 0 || check == 0) {
            value = FixMul(0x10000 - t, g_unk0x0053a02c[player]) + FixMul(t, g_unk0x0053a04c[player]);
            if (value < 0) {
                g_unk0x0053a06c[player] = FixDiv(-value, g_unk0x0053a0ec[player]);
                return;
            }
            g_unk0x0053a06c[player] = FixDiv(value, g_unk0x0053a0ec[player]);
        }
    }
}

// Draws a player's flash overlay (fading with g_unk0x0053a06c) over pRect.
// match 48%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00428680
void FUN_00428680(unsigned int player, short *pRect, int check)
{
    BYTE colour[4];
    BYTE index = (BYTE)player;
    unsigned int alpha;
    BYTE view;

    if ((FUN_0041f3a0() == 0 || index != 0) && (g_unk0x0053a0cc[player & 0xff] == 0 || check == 0)) {
        view = FUN_00422fb0(player);
        if (index < (BYTE)RallyDataState() && g_unk0x0053a06c[view] > 0) {
            alpha = (unsigned int)(g_unk0x0053a06c[view] * 0xff >> 16);
            if (alpha > 0xff)
                alpha = 0xff;
            colour[0] = ((BYTE *)&g_unk0x0053a00c[player & 0xff])[0];
            colour[1] = ((BYTE *)&g_unk0x0053a00c[player & 0xff])[1];
            colour[2] = ((BYTE *)&g_unk0x0053a00c[player & 0xff])[2];
            colour[3] = (BYTE)alpha;
            Sprite_FillRect((int)g_pGraphics + 0x150, pRect, colour, g_unk0x0053a20c[view]);
        }
    }
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
// match 79%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00427ad0
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
// match 80%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00427b70
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

// GLOBAL: CMR2 0x005393a8
int g_unk0x005393a8;
// GLOBAL: CMR2 0x005393ac
int g_unk0x005393ac[7];
// GLOBAL: CMR2 0x005393cc
int g_unk0x005393cc;
// GLOBAL: CMR2 0x005393d0
int g_unk0x005393d0;
// Triangular numbers 0, 1, 3, 6, ... (100 entries).
// GLOBAL: CMR2 0x00539b38
int g_triangleNumbers[100];

// Resets the network race state and builds the triangle number table.
// match 75%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00424ed0
void FUN_00424ed0(void)
{
    int *p;
    int sum;
    int n;

    memset(g_unk0x005393ac, 0, sizeof(g_unk0x005393ac));
    sum = 0;
    n = 0;
    for (p = g_triangleNumbers; p < g_triangleNumbers + 100; p++) {
        sum += n;
        n++;
        *p = sum;
    }
    FUN_00427580(20000, 1000000, 2);
    g_unk0x005393a8 = -1;
    g_unk0x005393cc = -1;
    g_unk0x005393d0 = -1;
}

#include "Car.h"
#include "Sector.h"

extern int FUN_0041d2a0(void);
extern int FUN_004582d0(int);
extern int FUN_004582f0(int);
extern int RallyData_FUN_004209d0(BYTE *);

// GLOBAL: CMR2 0x00539388
NetStats g_localCarStats;

// Quantization scales used by the network packet format.
// GLOBAL: CMR2 0x00511318
extern const float g_netHeightScale = 0.1f;
// GLOBAL: CMR2 0x0051131c
extern const float g_netZero = 0.0f;
// GLOBAL: CMR2 0x00511320
extern const float g_netElevationScale = 1.0f / 180.0f;
// GLOBAL: CMR2 0x00511324
extern const float g_netByteScale = 255.0f;
// GLOBAL: CMR2 0x00511328
extern const float g_netHeadingScale = 1.0f / 360.0f;
// GLOBAL: CMR2 0x00511330
extern const double g_netAcosScale = -4095.0;
// GLOBAL: CMR2 0x0051133c
extern const float g_netSignedByteScale = 127.0f;
// GLOBAL: CMR2 0x00511340
extern const float g_netAngularScale = 5.0f;
// GLOBAL: CMR2 0x00511344
extern const float g_netUnsignedShortScale = 65530.0f;
// GLOBAL: CMR2 0x00511348
extern const float g_netSignedShortScale = 32765.0f;
// GLOBAL: CMR2 0x0051134c
extern const float g_netMinusOne = -1.0f;
// GLOBAL: CMR2 0x00511350
extern const float g_netOne = 1.0f;
// GLOBAL: CMR2 0x00511354
extern const float g_netDeltaScale = 0.2f;

// Packs the local car into the network state, preserving unrelated flag bits.
// match 49%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00424f20
void NetRace_PackCarState(Car *car)
{
    BYTE *packet = (BYTE *)&g_localCarStats;
    BYTE *raw = (BYTE *)car;
    FixVector average, displacement, relative, axes[2];
    float value, z;
    int i;
    g_localCarStats.seq = (unsigned short)FUN_0041d2a0();
    displacement.x = *(int *)(raw + 0x2dc) - *(int *)(raw + 0x2e8);
    displacement.z = *(int *)(raw + 0x2e4) - *(int *)(raw + 0x2f0);
    average.x = average.y = average.z = 0;
    for (i = 0; i < 4; i++) {
        average.x += *(int *)(raw + 0x1a4 + i * 12) + *(int *)(raw + 0x88 + i * 36);
        average.z += *(int *)(raw + 0x1a8 + i * 12) + *(int *)(raw + 0x8c + i * 36);
    }
    FixVecScaleRecip(&average, &average, 0x40000);
    value = (float)((double)displacement.x * CGraphics::m_oneOver65536 * g_netDeltaScale);
    if (value >= g_netOne) *(short *)(packet + 2) = 0x7ffd;
    else if (value <= g_netMinusOne) *(short *)(packet + 2) = (short)0x8003;
    else *(short *)(packet + 2) = (short)(int)(__int64)((double)value * g_netSignedShortScale);
    value = (float)((double)displacement.z * CGraphics::m_oneOver65536 * g_netDeltaScale);
    if (value >= g_netOne) *(short *)(packet + 4) = 0x7ffd;
    else if (value <= g_netMinusOne) *(short *)(packet + 4) = (short)0x8003;
    else *(short *)(packet + 4) = (short)(int)(__int64)((double)value * g_netSignedShortScale);
    double product = (double)FixMul(average.x, average.z) * CGraphics::m_oneOver65536;
    if (product >= g_netOne) *(unsigned short *)(packet + 6) = 0xfffa;
    else *(unsigned short *)(packet + 6) = (unsigned short)(int)(__int64)(product * g_netUnsignedShortScale);
    value = (float)((double)car->angularVelocity.x * CGraphics::m_oneOver65536 * g_netAngularScale);
    if (value >= g_netOne) packet[20] = 0x7f;
    else if (value <= g_netMinusOne) packet[20] = 0x81;
    else packet[20] = (BYTE)(int)(__int64)((double)value * g_netSignedByteScale);
    value = (float)((double)car->angularVelocity.y * CGraphics::m_oneOver65536 * g_netAngularScale);
    if (value >= g_netOne) packet[21] = 0x7f;
    else if (value <= g_netMinusOne) packet[21] = 0x81;
    else packet[21] = (BYTE)(int)(__int64)((double)value * g_netSignedByteScale);
    value = (float)((double)car->angularVelocity.z * CGraphics::m_oneOver65536 * g_netAngularScale);
    if (value >= g_netOne) packet[22] = 0x7f;
    else if (value <= g_netMinusOne) packet[22] = 0x81;
    else packet[22] = (BYTE)(int)(__int64)((double)value * g_netSignedByteScale);
    g_localCarStats.speed = (g_localCarStats.speed & 0xfeff) | ((raw[0xb35] & 1) << 8);
    g_localCarStats.field_0x1a = (g_localCarStats.field_0x1a & 0xfeff) | ((raw[0xc00] & 1) << 8);
    *(short *)(packet + 12) = *(short *)(raw + 0xb00);
    FixMatrix_GetPosition(&relative, car->pWorld);
    Sector *sector = g_sectors[*(short *)(raw + 0xb00)];
    relative.x -= sector->x;
    relative.z -= sector->z;
    relative.y -= sector->y;
    value = (float)((double)relative.x * CGraphics::m_oneOver65536 * CGraphics::m_oneOver128);
    z = (float)((double)relative.z * CGraphics::m_oneOver65536 * CGraphics::m_oneOver128);
    if (value >= g_netOne) *(short *)(packet + 8) = 0x7ffd;
    else if (value <= g_netMinusOne) *(short *)(packet + 8) = (short)0x8003;
    else *(short *)(packet + 8) = (short)(int)(__int64)((double)value * g_netSignedShortScale);
    if (z >= g_netOne) *(short *)(packet + 10) = 0x7ffd;
    else if (z <= g_netMinusOne) *(short *)(packet + 10) = (short)0x8003;
    else *(short *)(packet + 10) = (short)(int)(__int64)((double)z * g_netSignedShortScale);
    FixMatrix_GetRight(&axes[0], car->pWorld);
    FixMatrix_GetForward(&axes[1], car->pWorld);
    for (i = 0; i < 2; i++) {
        FixVector *axis = &axes[i];
        int x = FIX_ABS(axis->x);
        int y = FIX_ABS(axis->y);
        int az = FIX_ABS(axis->z);
        int heading = x == 0 ? 0 : (int)FixAtan2(az, x) * 0x1680;
        // FixAcos uses /QIfist in the physics TUs; use FISTP explicitly here.
        int negative = y < 0;
        if (negative) y = -y;
        short acos;
        if (y > 0x10000) acos = g_acosTable[4095];
        else {
            int index = (int)(__int64)((double)y * CGraphics::m_oneOver65536 * g_netAcosScale);
            acos = negative ? -g_acosTable[-index] : g_acosTable[-index];
        }
        int elevation = (0x400 - acos) * 0x1680;
        if (axis->x >= 0 && axis->z <= 0) heading = 0x1680000 - heading;
        else if (axis->x <= 0) {
            if (axis->z >= 0) heading = 0xb40000 - heading;
            else heading += 0xb40000;
        }
        if (axis->y <= 0) elevation = 0xb40000 - elevation;
        value = (float)((double)heading * CGraphics::m_oneOver65536 * g_netHeadingScale * g_netByteScale);
        double vertical = (double)elevation * CGraphics::m_oneOver65536 * g_netElevationScale * g_netByteScale;
        if (value < g_netZero) value = g_netZero;
        else if (value > g_netByteScale) value = g_netByteScale;
        if (vertical < g_netZero) vertical = g_netZero;
        else if (vertical > g_netByteScale) vertical = g_netByteScale;
        BYTE high = (BYTE)(int)(__int64)(vertical);
        BYTE low = (BYTE)(int)(__int64)(value);
        *(unsigned short *)(packet + 16 + i * 2) = (high << 8) | low;
    }
    int steer = FixMul(FixDiv((int)*(short *)(raw + 0xb10) * 0x1680,
                             (int)*(short *)(raw + 0xb16) * 0x1680) + 0x10000, 0x3f0000) + 0x1999;
    if (steer > 0x7e8000) steer = 0x7e8000;
    g_localCarStats.field_0x1a = (g_localCarStats.field_0x1a & 0xff80) | ((steer >> 16) & 0x7f);
    if (car->field_0x79c) g_localCarStats.field_0x1a |= 0x80;
    else g_localCarStats.field_0x1a &= 0xff7f;
    g_localCarStats.speed = (g_localCarStats.speed & 0xfdff) | ((raw[0xb54] & 1) << 9);
    value = (float)((double)*(int *)(raw + 0x960) * CGraphics::m_oneOver65536 * g_netHeightScale);
    if (value >= g_netOne) packet[14] = 255;
    else if (value <= g_netZero) packet[14] = 0;
    else packet[14] = (BYTE)(int)(__int64)((double)value * g_netByteScale);
    value = (float)((double)(*(int *)(raw + 0x960) - *(int *)(raw + 0x964)) * CGraphics::m_oneOver65536 * g_netDeltaScale);
    if (value >= g_netOne) packet[15] = 0x7f;
    else if (value <= g_netMinusOne) packet[15] = 0x81;
    else packet[15] = (BYTE)(int)(__int64)((double)value * g_netSignedByteScale);
    if (raw[0xb45]) {
        g_localCarStats.field_0x1a |= 0x200;
        --raw[0xb45];
    } else g_localCarStats.field_0x1a &= 0xfdff;
    if (CGameInfo::FUN_00404f20()) g_localCarStats.field_0x1a |= 0x400;
    else g_localCarStats.field_0x1a &= 0xfbff;
    int node = FUN_004582f0(car->field_0xb1a);
    if (node < 0) node = 0;
    else if (node > 0x400) node = 0x400;
    g_localCarStats.field_0x18 = (g_localCarStats.field_0x18 & 0xfc00) | (node & 0x3ff);
    int stage = FUN_004582d0(car->field_0xb1a);
    if (stage < 0) {
        stage = -stage;
        g_localCarStats.field_0x1a |= 0x8000;
    } else g_localCarStats.field_0x1a &= 0x7fff;
    if (stage > 15) stage = 15;
    g_localCarStats.field_0x1a = (g_localCarStats.field_0x1a & 0x87ff) | ((stage & 15) << 11);
    int progress = FixMul(RallyData_FUN_004209d0(raw), 0x400000) >> 16;
    if (progress < 0) progress = 0;
    else if (progress > 63) progress = 63;
    g_localCarStats.speed = (g_localCarStats.speed & 0x3ff) | (progress << 10);
}

// Packs the state of every car listed in pIndices, from the highest index down.
// FUNCTION: CMR2 0x004258e0
void FUN_004258e0(int base, short *pIndices, short count)
{
    int i;

    if (g_unk0x00539cc8 != 0) {
        for (i = (int)count - 1; i >= 0; i--) {
            Car *pCar = (Car *)(base + pIndices[i] * 0xc24);

            if (*(int *)((BYTE *)pCar + 0xc1c) == 0 &&
                (NetRace_PackCarState(pCar), g_unk0x00539cc8 != 0))
                FUN_004278f0(&g_localCarStats);
        }
    }
}

// Clamps the object's displacement so that it does not overshoot the target
// along the (negated) direction vector.
// match 89%: identical logic; MSVC6 only differs in which stack slot holds the
// first dot product ([ebp-4] vs [ebp+8]).
// FUNCTION: CMR2 0x00426b90
void FUN_00426b90(int param_1, int param_2)
{
    int dot;
    int scale;
    FixVector v;
    FixVector d;

    if (*(int *)(param_1 + 0xdc) == 0 || param_2 == 0)
        return;
    FixVecScale(&v, (FixVector *)(param_1 + 0x94), -0x10000);
    dot = FixVecDot((FixVector *)(param_1 + 0x70), &v);
    if (dot > 0) {
        d.x = *(int *)(param_1 + 0xa0) - *(int *)(param_1 + 0x64);
        d.y = *(int *)(param_1 + 0xa4) - *(int *)(param_1 + 0x68);
        d.z = *(int *)(param_1 + 0xa8) - *(int *)(param_1 + 0x6c);
        scale = FixVecDot(&d, &v) + 0x10000;
        param_2 = FixDiv(scale, param_2);
        if (param_2 > 0) {
            if (dot < param_2)
                param_2 = dot;
            FixVecScale(&d, &v, dot);
            *(int *)(param_1 + 0x70) -= d.x;
            *(int *)(param_1 + 0x74) -= d.y;
            *(int *)(param_1 + 0x78) -= d.z;
            FixVecScale(&d, &v, param_2);
            *(int *)(param_1 + 0x70) += d.x;
            *(int *)(param_1 + 0x74) += d.y;
            *(int *)(param_1 + 0x78) += d.z;
        }
    }
}

extern const double g_unk0x00511380;

// Integrates the remote car's body for one frame: turns the tick delta into a
// fixed-point step, advances the two position bases (0x64 and 0x70), rotates
// the reference basis by the resulting velocity angle, then either writes the
// basis straight into the car or blends it with a copy of the current one.
// FUNCTION: CMR2 0x00426810
void FUN_00426810(int param_1, int param_2)
{
    FixVector v;
    short angles[3];
    BYTE basis[0x40];
    BYTE blended[0x40];
    int delta;
    int magnitude;
    int scale;

    if (*(unsigned short *)(param_1 + 0xc6) > 0)
        *(unsigned short *)(param_1 + 0xc6) -= 1;
    if (*(int *)(param_1 + 0xe8) != 0) {
        delta = 0;
        *(int *)(param_1 + 0xe8) = 0;
    } else {
        delta = (param_2 - (unsigned int)*(unsigned short *)(param_1 + 0xc8)) * 0x10000;
    }
    if (*(int *)(param_1 + 0xe0) != 0)
        FUN_00426b90(param_1, delta);

    FixVecScale(&v, (FixVector *)(param_1 + 0x70), delta);
    *(int *)(param_1 + 0x64) += v.x;
    *(int *)(param_1 + 0x68) += v.y;
    *(int *)(param_1 + 0x6c) += v.z;

    magnitude = delta;
    if (magnitude < 0)
        magnitude = -magnitude;
    scale = g_triangleNumbers[magnitude >> 16] * 0x10000;
    if (delta < 0)
        scale = -scale;

    FixVecScale(&v, (FixVector *)(param_1 + 0x88), scale);
    *(int *)(param_1 + 0x64) += v.x;
    *(int *)(param_1 + 0x68) += v.y;
    *(int *)(param_1 + 0x6c) += v.z;

    FixVecScale(&v, (FixVector *)(param_1 + 0x88), delta);
    *(int *)(param_1 + 0x70) += v.x;
    *(int *)(param_1 + 0x74) += v.y;
    *(int *)(param_1 + 0x78) += v.z;

    FixVecScale(&v, (FixVector *)(param_1 + 0x7c), delta);
    angles[0] = (short)(__int64)((double)v.x * g_unk0x00511380);
    angles[1] = (short)(__int64)((double)v.y * g_unk0x00511380);
    angles[2] = (short)(__int64)((double)v.z * g_unk0x00511380);
    FixBasis_Rotate((FixBasis *)(param_1 + 0x40), (unsigned short *)angles);

    *(int *)(param_1 + 0xbc) += FixMul(*(int *)(param_1 + 0xc0), delta) -
                                FixMul(FixMul(delta, delta), 0xc49);
    *(int *)(param_1 + 0xc0) -= FixMul(FixMul(0x20000, delta), 0xc49);
    *(unsigned short *)(param_1 + 0xc8) = (unsigned short)param_2;

    if (*(int *)(param_1 + 0xd8) != 0) {
        FixMatrix_SetPosition((FixVector *)(param_1 + 0x64), (FixMatrix *)param_1);
        FixMatrix_SetRight((FixVector *)(param_1 + 0x40), (FixMatrix *)param_1);
        FixMatrix_SetUp((FixVector *)(param_1 + 0x4c), (FixMatrix *)param_1);
        FixMatrix_SetForward((FixVector *)(param_1 + 0x58), (FixMatrix *)param_1);
        *(int *)(param_1 + 0xe0) = 0;
        *(int *)(param_1 + 0xb4) = *(int *)(param_1 + 0xbc);
        return;
    }
    FixMatrix_SetPosition((FixVector *)(param_1 + 0x64), (FixMatrix *)basis);
    FixMatrix_SetRight((FixVector *)(param_1 + 0x40), (FixMatrix *)basis);
    FixMatrix_SetUp((FixVector *)(param_1 + 0x4c), (FixMatrix *)basis);
    FixMatrix_SetForward((FixVector *)(param_1 + 0x58), (FixMatrix *)basis);
    FixMatrix_Interpolate((FixMatrix *)blended, (FixMatrix *)param_1, (FixMatrix *)basis, 0x8000, 0x8000, 0x8000, 0);
    FixMatrix_CopyRotation((FixMatrix *)blended, (FixMatrix *)param_1);
    *(int *)(param_1 + 0xe0) = 0;
    *(int *)(param_1 + 0xb4) +=
        FixMul(*(int *)(param_1 + 0xbc) - *(int *)(param_1 + 0xb4), 0x8000);
}

// Advances the accumulator (0x88) of a remotely controlled car by one step:
// normalises the horizontal part of its direction (the movement base at 0x70,
// or the forward row 0x58 once the car is above the slow threshold) and adds
// the speed-derived step along it.
// FUNCTION: CMR2 0x004263d0
void FUN_004263d0(int param_1)
{
    FixVector v;
    FixVector step;
    int t;

    *(int *)(param_1 + 0x88) = 0;
    *(int *)(param_1 + 0x8c) = 0;
    *(int *)(param_1 + 0x90) = 0;
    if (*(int *)(param_1 + 0x50) < 0x1999) {
        v = *(FixVector *)(param_1 + 0x70);
        v.y = 0;
        if (FixVecLength(&v) > 0) {
            FIX_NORMALIZE_INTO(v, v)
        } else {
            v.x = 0;
            v.y = 0;
            v.z = 0;
        }
    } else {
        v = *(FixVector *)(param_1 + 0x58);
        v.y = 0;
        FIX_NORMALIZE_INTO(v, v)
    }
    t = FixMul(FixVecDot((FixVector *)(param_1 + 0x70), &v), *(int *)(param_1 + 0xb0));
    if (t < 0)
        t = -FixMul(t, t);
    else
        t = FixMul(t, t);
    if (FIX_ABS(t) > 0x10000)
        t = (t <= 0) ? -0x10000 : 0x10000;
    t = -FixMul(FixMul(t, *(int *)(param_1 + 0xac)), 0x11eb);
    FixVecScale(&step, &v, t);
    *(int *)(param_1 + 0x88) += step.x;
    *(int *)(param_1 + 0x8c) += step.y;
    *(int *)(param_1 + 0x90) += step.z;
}

void FUN_004a15b0(BOOL param1);
void FUN_00402c90(int param);
void FUN_0044a1b0(int param);
void FUN_004a1940(DWORD *pId);
void FUN_00409c80(int *pId);

// Network-device notification: id 5 refreshes the player list of the entry,
// id 0x101 re-initialises the in-race menu and the HUD.
// FUNCTION: CMR2 0x00427680
void FUN_00427680(int param_1, int *param_2)
{
    if (*param_2 != 5) {
        if (*param_2 == 0x101) {
            FUN_004a15b0(1);
            FUN_00402c90(1);
            FUN_0044a1b0(1);
        }
        return;
    }
    FUN_004a1940((DWORD *)(param_2 + 2));
    FUN_00409c80(param_2 + 2);
}

void Car_GetViewPositionDelta(FixVector *pOut, unsigned int view);

// Returns the 12-bit angle param_3 scaled down by how far the player's view
// node (matrix at 0x538d30) is from the car's 0x2d0 / 0x408 positions, capped
// by the angle between the direction to the car and the view delta. param_3
// is returned unchanged when the view node is 100.0 units away or more, and
// when those two directions are exactly perpendicular.
// match 79.96% (auditado W165): el tipo de retorno es unsigned short (el original devuelve el valor
// en AX en 'return param_3' y sus llamadores lo enmascaran con 0xffff); el resto de los diffs es
// reparto de registros y slots, sin cambio de comportamiento.
// FUNCTION: CMR2 0x00427e20
unsigned short FUN_00427e20(int param_1, int param_2, unsigned short param_3)
{
    FixVector pos;
    FixVector view;
    FixVector delta;
    FixVector dir;
    FixVector other;
    int index;
    int dist;
    int len;
    int dot;
    int t;
    int value;

    index = param_1;
    if (FUN_0041f3a0() != 0)
        index = 1;
    FixMatrix_GetPosition(&pos, (FixMatrix *)(g_unk0x00538d2c + 4 + index * 100));
    delta.x = pos.x - *(int *)((BYTE *)Car_Get(param_2) + 0x2d0);
    delta.y = pos.y - *(int *)((BYTE *)Car_Get(param_2) + 0x2d4);
    delta.z = pos.z - *(int *)((BYTE *)Car_Get(param_2) + 0x2d8);
    dist = FixVec_Length(&delta);
    if (dist < 0x640000) {
        Car_GetViewPositionDelta(&view, param_1);
        view.x -= *(int *)((BYTE *)Car_Get(param_2) + 0x408);
        view.y -= *(int *)((BYTE *)Car_Get(param_2) + 0x40c);
        view.z -= *(int *)((BYTE *)Car_Get(param_2) + 0x410);
        dist = FixVec_Length(&view);
        if (dist > 0x320000) {
            len = FixVecLength(&view);
            if (len == 0) {
                view.x = 0;
                view.y = 0;
                view.z = 0;
            } else {
                FixVecScaleRecip(&view, &view, len);
            }
            FixVecScale(&view, &view, 0x320000);
        }
        other = view;
        len = FixVecLength(&delta);
        if (len == 0) {
            delta.x = 0;
            delta.y = 0;
            delta.z = 0;
        } else {
            FixVecScaleRecip(&delta, &delta, len);
        }
        len = FixVecLength(&view);
        if (len == 0) {
            other.x = 0;
            other.y = 0;
            other.z = 0;
        } else {
            FixVecScaleRecip(&other, &view, len);
        }
        dot = FixVecDot(&other, &delta);
        FixVecScale(&view, &view, dot);
        if (dot > 0) {
            t = FixDiv(FixVec_Length(&view), 0xd3d70) + 0x10000;
            value = (int)(__int64)((double)(param_3 >> 1) * CGraphics::m_65536);
            if (t <= 0x8000)
                return (value >> 16) << 1;
            return (FixDiv(value, t) >> 16) << 1;
        }
        if (dot < 0) {
            t = 0x10000 - FixDiv(FixVec_Length(&view), 0xd3d70);
            value = (int)(__int64)((double)(param_3 >> 1) * CGraphics::m_65536);
            if (t > 0x8000)
                value = FixDiv(value, t);
            return (value >> 16) << 1;
        }
    }
    return param_3;
}

// --- 0x00425c40 (layer 0) ----------------------------------------------------
// Inverses of the network quantization scales (0x0051137c = 1/32765,
// 0x00511374 = 1/65530, 0x00511370 = 1/127, 0x00511368 = 128,
// 0x0051135c = 24/17, 0x00511358 = 12/17). 0x00511360 (10.0) is a file-local
// constant of SceneNode.cpp, so its value is spelled out below.
// GLOBAL: CMR2 0x00511358
extern const float g_unk0x00511358 = 12.0f / 17.0f;
// GLOBAL: CMR2 0x0051135c
extern const float g_unk0x0051135c = 24.0f / 17.0f;
// GLOBAL: CMR2 0x00511368
extern const float g_unk0x00511368 = 128.0f;
// GLOBAL: CMR2 0x00511370
extern const float g_unk0x00511370 = 1.0f / 127.0f;
// GLOBAL: CMR2 0x00511374
extern const float g_unk0x00511374 = 1.0f / 65530.0f;
// GLOBAL: CMR2 0x0051137c
extern const float g_unk0x0051137c = 1.0f / 32765.0f;

// Sector index of the last packet applied to each player record.
// GLOBAL: CMR2 0x00539394
int g_unk0x00539394;

extern const float g_unk0x00511364;
extern const float g_unk0x0051136c;
extern const float g_unk0x00511378;
extern float g_65536f;

// Applies the car state received from a player (g_localCarStats) to that
// player's record: drops stale and out-of-range packets, converts the quantised
// position, heading, speed and flags back to 16.16, builds the two body axes
// from the packed pair of angles plus the third one (cross product) and leaves
// the steering angle in *pOut.
// match 48%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00425c40
int FUN_00425c40(int car, int *pOut)
{
    BYTE *packet = (BYTE *)&g_localCarStats;
    float axis[4];
    FixVector row;
    Sector *pSector;
    short ang1;
    short ang2;
    BYTE steer;
    int engine;
    int diff;
    int i;

    diff = (int)(unsigned short)g_localCarStats.seq - (int)*(unsigned short *)(car + 0xca);
    if (diff <= 0)
        return 0;
    if (g_unk0x00539cc8 == 0)
        return 0;
    if (diff >= 100)
        return 0;
    if (*(unsigned short *)(packet + 0xc) >= (unsigned int)g_sectorCount)
        return 0;

    *(int *)(car + 0xe0) = 1;
    *(unsigned short *)(car + 0xc8) = g_localCarStats.seq;
    *(unsigned short *)(car + 0xca) = g_localCarStats.seq;

    *(int *)(car + 0x70) = (int)((float)(short)*(unsigned short *)(packet + 2) *
                                 g_unk0x0051137c * g_unk0x00511378 * g_65536f);
    *(int *)(car + 0x74) = 0;
    *(int *)(car + 0x78) = (int)((float)(short)*(unsigned short *)(packet + 4) *
                                 g_unk0x0051137c * g_unk0x00511378 * g_65536f);
    engine = (int)((float)*(unsigned short *)(packet + 6) * g_unk0x00511374 * g_65536f);
    *(int *)(car + 0xac) = engine;
    if (engine == 0)
        *(int *)(car + 0xb0) = 0;
    else
        *(int *)(car + 0xb0) = FixDiv(0x10000, engine);
    *(int *)(car + 0x7c) = (int)((float)(signed char)packet[0x14] *
                                 g_unk0x00511370 * g_unk0x0051136c * g_65536f);
    *(int *)(car + 0x80) = (int)((float)(signed char)packet[0x15] *
                                 g_unk0x00511370 * g_unk0x0051136c * g_65536f);
    *(int *)(car + 0x84) = (int)((float)(signed char)packet[0x16] *
                                 g_unk0x00511370 * g_unk0x0051136c * g_65536f);
    *(BYTE *)(car + 0xcc) = packet[0x17] & 1;
    *(int *)(car + 0xd4) = (*(unsigned short *)(packet + 0x1a) >> 8) & 1;
    *(int *)(car + 0x64) = (int)((float)(short)*(unsigned short *)(packet + 8) *
                                 g_unk0x0051137c * g_unk0x00511368 * g_65536f);
    *(int *)(car + 0x68) = 0;
    *(int *)(car + 0x6c) = (int)((float)(short)*(unsigned short *)(packet + 0xa) *
                                 g_unk0x0051137c * g_unk0x00511368 * g_65536f);
    pSector = g_sectors[*(short *)(packet + 0xc)];
    *(int *)(car + 0x64) += pSector->x;
    *(int *)(car + 0x68) += pSector->y;
    *(int *)(car + 0x6c) += pSector->z;
    *(int *)(car + 0xbc) = (int)((float)packet[0xe] * g_unk0x00511364 * 10.0f * g_65536f);
    *(int *)(car + 0xc0) = (int)((float)(signed char)packet[0xf] *
                                 g_unk0x00511370 * g_unk0x00511378 * g_65536f);

    // Two body axes: each packed pair of bytes is an angle in 1/17th of a unit.
    axis[0] = (float)packet[0x11];
    axis[1] = (float)packet[0x13];
    axis[2] = (float)packet[0x10];
    axis[3] = (float)packet[0x12];
    for (i = 0; i < 8; i += 4) {
        float f1 = axis[i + 2] * g_unk0x0051135c;
        float f2 = axis[i] * g_unk0x00511358;
        FixVector *pRow = (FixVector *)((BYTE *)car + 0x40 + (i / 4) * 0x18);

        axis[i + 2] = f1;
        axis[i] = f2;
        ang1 = (short)((double)(int)(f1 * g_65536f) * CGraphics::m_oneOver65536);
        ang2 = (short)((double)(int)(f2 * g_65536f) * CGraphics::m_oneOver65536);
        // ang1 sale de f1 (axis[i+2]*24/17) y ang2 de f2 (axis[i]*12/17). El original (asm 0x425ff5-0x42606a)
        // usa ang2 en el primer argumento de row.x y en row.y; row.z es un producto conmutativo y no cambia.
        row.x = FixMul(g_sinTable[ang2 & 0xfff], g_sinTable[(ang1 + 0x400) & 0xfff]);
        row.y = g_sinTable[(ang2 + 0x400) & 0xfff];
        row.z = FixMul(g_sinTable[ang1 & 0xfff], g_sinTable[ang2 & 0xfff]);
        *pRow = row;
        {
            int len = FixVecLength(pRow);

            if (len == 0) {
                pRow->x = 0;
                pRow->y = 0;
                pRow->z = 0;
            } else {
                FixVecScaleRecip(pRow, pRow, len);
            }
        }
    }
    {
        FixVector *pThird = (FixVector *)((BYTE *)car + 0x4c);
        int len;

        FixVecCross(pThird, (FixVector *)((BYTE *)car + 0x58),
                    (FixVector *)((BYTE *)car + 0x40));
        len = FixVecLength(pThird);
        if (len == 0) {
            pThird->x = 0;
            pThird->y = 0;
            pThird->z = 0;
        } else {
            FixVecScaleRecip(pThird, pThird, len);
        }
    }

    steer = packet[0x1a] & 0x7f;
    if (steer == 0)
        *pOut = -0x10000;
    else if (steer == 0x7f)
        *pOut = 0x10000;
    else
        *pOut = FixMul(steer << 16, 0x418) - 0x10000;
    if (packet[0x1a] & 0x80)
        *(int *)(car + 0xb8) = 0x10000;
    else
        *(int *)(car + 0xb8) = 0;
    *(unsigned int *)(car + 0xd0) = (*(unsigned short *)(packet + 0x16) & 0x200) >> 9;
    if ((*(unsigned short *)(packet + 0x1a) & 0x200) != 0 &&
        *(short *)(car + 0xc6) == 0) {
        *(int *)(car + 0xd8) = 1;
        *(short *)(car + 0xc6) = 100;
    }
    *(unsigned int *)(car + 0xe4) = (*(unsigned short *)(packet + 0x1a) >> 10) & 1;
    FUN_004263d0(car);
    return 1;
}

#include <stdlib.h>
#include "Input.h"

// Helpers implemented in NetPlayers.cpp / RallyData.cpp.
extern int FUN_0040b010(int index);
extern int FUN_0040b020(int value);
extern BYTE FUN_00409df0(int index);
extern void FUN_00409e00(int index);
extern NetStats *FUN_00409e20(int index);
extern double g_unk0x00511300;
extern void FUN_0040a580(int param1, int param2, int param3);
extern void FUN_00425a90(BYTE *pCars);
extern void FUN_00426d80(Car *pDst, Car *pSrc);

// Network body record of a car (0xec bytes); the array starts at 0x5393d8.
struct CarNetRecord {
    FixMatrix matrix;       // 0x00
    FixVector right;        // 0x40
    FixVector up;           // 0x4c
    FixVector forward;      // 0x58
    FixVector position;     // 0x64
    BYTE pad_0x70[0x7c];
};
extern CarNetRecord g_unk0x005393d8;

// Text of the network frame statistics trace.
// GLOBAL: CMR2 0x00519988
char g_str0x00519988[38] = "Received %d, gnNetworkFrame[%d] = %d\n";
// GLOBAL: CMR2 0x0051995c
char g_str0x0051995c[42] = "Not received any gnNetworkFrame[%d] = %d\n";

// Receives the race state from the network: unpacks every listed car from its
// packet, refreshes the stage progress readout, integrates the body pose of the
// cars still in play from their network records and copies those records back
// into the cars. With networking off it only refreshes the readout.
// FUNCTION: CMR2 0x00425950
void FUN_00425950(Car *pCars, short *pIndices, short count)
{
    int progress;
    int i;

    if (g_unk0x00539cc8 == 0) {
        progress = (RallyData_FUN_004209d0((BYTE *)Car_Get(0)) * 100) >> 16;
        FUN_0040a580(FUN_004582f0(0), progress, FUN_004582d0(0));
        return;
    }
    FUN_00425a90((BYTE *)pCars);
    progress = (RallyData_FUN_004209d0((BYTE *)Car_Get(0)) * 100) >> 16;
    FUN_0040a580(FUN_004582f0(0), progress, FUN_004582d0(0));

    for (i = (int)count - 1; i >= 0; i--) {
        int idx = pIndices[i];

        if (*(int *)((BYTE *)pCars + idx * 0xc24 + 0xc1c) != 0)
            FUN_00426810((int)(&g_unk0x005393d8 + idx), g_unk0x005393ac[FUN_0040b020(idx)]);
    }
    for (i = (int)count - 1; i >= 0; i--) {
        int idx = pIndices[i];

        if (*(int *)((BYTE *)pCars + idx * 0xc24 + 0xc1c) != 0)
            FUN_00426d80(pCars + idx, (Car *)(&g_unk0x005393d8 + idx));
    }
}

// Polls the seven network players: for every one with a pending packet copies
// the 30-byte statistics block into the local frame, unpacks it into the
// player's body record (height word, speed, stale counter) and averages the
// frame counter; players without a packet bump theirs. Slot 0 traces the
// result.
// match 89%: logica, constantes y orden exactos; solo difieren el ensanchado del
// contador (el original carga CX y luego copia/enmascara a EDX) y los
// desplazamientos de los saltos encadenados.
// FUNCTION: CMR2 0x00425a90
void FUN_00425a90(BYTE *pCars)
{
    NetStats *pStats;
    BYTE *pEntry;
    BYTE *pCar;
    int local;
    int value;
    int i;

    i = 0;
    do {
        if ((BYTE)FUN_00409cb0(i) != 0 && FUN_00409df0(i) != 0) {
            FUN_00409e00(i);
            pStats = FUN_00409e20(i);
            g_localCarStats = *pStats;
            pEntry = (BYTE *)&g_unk0x005393d8 + FUN_0040b010(i) * 0xec;
            pCar = pCars + FUN_0040b010(i) * 0xc24;
            if (FUN_00425c40((int)pEntry, &local) != 0) {
                value = FixMul(local, *(short *)(pCar + 0xb16) * 0x1680);
                *(unsigned short *)(pEntry + 0xc4) =
                    (unsigned short)(__int64)((double)value * g_unk0x00511300);
                *(int *)(pEntry + 0xb8) =
                    FixMul(*(int *)(pCar + 0x788), *(int *)(pEntry + 0xb8));
            }
            if (abs(g_unk0x005393ac[i] - pStats->seq) < 0x33) {
                *(int *)(pEntry + 0xe8) = 0;
            } else {
                g_unk0x005393ac[i] = pStats->seq;
                *(int *)(pEntry + 0xe8) = 1;
            }
            g_unk0x005393ac[i] = (pStats->seq + g_unk0x005393ac[i]) >> 1;
            if (i == 0)
                RallyData_ValidateIndex((int)CInput::FormatString(g_str0x00519988, pStats->seq,
                                                                 0, g_unk0x005393ac[i]));
        } else {
            if (g_unk0x005393ac[i] != 0)
                g_unk0x005393ac[i]++;
            if (i == 0)
                RallyData_ValidateIndex((int)CInput::FormatString(g_str0x0051995c, 0,
                                                                 g_unk0x005393ac[0]));
        }
        i++;
    } while (i < 7);
}
