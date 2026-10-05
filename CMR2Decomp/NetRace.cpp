#include <windows.h>
#include <string.h>
#include "NetPlayers.h"
#include "Sprite.h"
#include "Graphics.h"
#include "GameInfo.h"
#include "RallyData.h"
#include "FixedPoint.h"
#include "main.h"
extern const float g_netOne;
extern const float g_netZero;
extern const float g_netFontOffset;
extern const float g_netFontInverseScale;
extern const float g_netFontMaximumScale;

// Network messages sent during a race (0x427620-0x428760)

char Network_SendPlayerMessage(int to, int guaranteed, int data, int size);
int NetPlayers_GetAccumulatedTotal(void);
void NetPlayers_BlockStatisticsReception(void);


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
// FUNCTION: CMR2 0x00427580
void NetRace_SelectHUDFont(int width, int height, int players)
{
    float rowWidth = (float)(players * 0xc0 + 0x230);
    float scaleX = (float)width / rowWidth;
    float scaleY = (float)height / ((float)players * rowWidth);
    float scale;

    if (scaleY < scaleX)
        scale = scaleY;
    else
        scale = scaleX;
    if (scale > g_netFontMaximumScale)
        scale = g_netFontMaximumScale;
    g_unk0x005393d4 = 0x28;
    if ((g_netOne / scale) * g_netFontInverseScale - g_netFontOffset > g_netZero)
        g_unk0x005393d4 = 0x29;
}

// FUNCTION: CMR2 0x00427620
int NetRace_GetPlayerStatisticsValue(int index)
{
    return *(int *)g_unk0x005394bc[index];
}

// FUNCTION: CMR2 0x00427640
void NetRace_SetRaceControlFlag(BYTE param1)
{
    g_unk0x00539cc8 = param1;
}

// FUNCTION: CMR2 0x00427650
void NetRace_ResetType8And9PlayerIndices(void)
{
    g_unk0x00539ed0 = -1;
    g_unk0x00539dcc = -1;
}

// FUNCTION: CMR2 0x00427660
int NetRace_GetType8PlayerIndex(void)
{
    return g_unk0x00539ed0;
}

// FUNCTION: CMR2 0x00427670
int NetRace_GetType9PlayerIndex(void)
{
    return g_unk0x00539dcc;
}

// GLOBAL: CMR2 0x00539ed4
int g_unk0x00539ed4;

void NetPlayers_SetRemoteCarClass(DWORD *pId, unsigned int carClass);
void NetPlayers_MarkPlayerReadyByID(DWORD *pId);
void FUN_00409fd0(DWORD *pId, int split, unsigned int time);
void NetPlayers_RecordRemoteStageTime(DWORD *pId, int stage, unsigned int time);
void NetPlayers_MarkPlayerFinishedByID(DWORD *pId);
void NetPlayers_RecordPlayerFinishTime(DWORD *pId, unsigned int time, int value);
void NetPlayers_ReceiveStatistics(DWORD *pId, NetStats *pStats);
void NetPlayers_ReceivePublishedLeaderboard(char valid, BYTE *p);
void NetPlayers_BuildFinalClassification(void);
void NetworkChat_AppendLine(DWORD *pFrom, char *text, char local);
void Race_SetFlag3810C(void);
void Race_SetFlag37FFA(void);
void GameMenu_FinishContinueFade(BYTE index);
void GameMenu_FinishNetworkCloseFade(BYTE index);
void GameMenu_FinishNetworkRestartFade(BYTE index);
void GameMenu_FinishNetworkResultsQuitFade(BYTE index);
void NetRace_FadeOutPlayerScreen(BYTE index, FadeCallback pfnDone, int param3, int param4, int param5, char force);
extern int g_unk0x00537f34[2];
extern int g_unk0x005199b0;

// Dispatches a message of the in-race network stream to its handler.
// FUNCTION: CMR2 0x004276c0
void NetRace_DispatchRacePacket(DWORD *pId, BYTE *pPacket)
{
    switch (pPacket[0]) {
    case 0xb:
        if (g_unk0x00539cc8 != 0) {
            NetPlayers_ReceiveStatistics(pId, (NetStats *)(pPacket + 2));
            return;
        }
        break;
    case 7:
        NetPlayers_MarkPlayerReadyByID(pId);
        return;
    case 8:
        FUN_00409fd0(pId, pPacket[1], *(unsigned int *)(pPacket + 4));
        return;
    case 9:
        NetPlayers_RecordRemoteStageTime(pId, pPacket[1], *(unsigned int *)(pPacket + 4));
        return;
    case 10:
        NetPlayers_MarkPlayerFinishedByID(pId);
        NetPlayers_RecordPlayerFinishTime(pId, *(unsigned int *)(pPacket + 4), *(unsigned int *)(pPacket + 8));
        return;
    case 12:
        NetRace_FadeOutPlayerScreen(0, GameMenu_FinishNetworkResultsQuitFade, 1, 0, g_unk0x005199b0, 1);
        return;
    case 13:
        NetRace_FadeOutPlayerScreen(0, GameMenu_FinishContinueFade, 1, 0, g_unk0x005199b0, 1);
        return;
    case 14:
        Race_SetFlag3810C();
        if (CGameInfo::GetConfiguredGameMode() == 8) {
            Race_SetFlag37FFA();
            NetRace_FadeOutPlayerScreen(0, GameMenu_FinishNetworkCloseFade, 1, 0, g_unk0x005199b0, 1);
        } else {
            NetRace_FadeOutPlayerScreen(0, GameMenu_FinishNetworkCloseFade, 1, 0, g_unk0x00539ed4, 1);
        }
        NetPlayers_ClearReadyFlags();
        return;
    case 15:
        Race_SetFlag3810C();
        Race_SetFlag37FFA();
        NetRace_FadeOutPlayerScreen(0, GameMenu_FinishNetworkRestartFade, 1, 0, g_unk0x005199b0, 1);
        return;
    case 0:
        NetworkChat_AppendLine((DWORD *)pId, (char *)(pPacket + 1), 0);
        return;
    case 6:
        NetPlayers_SetRemoteCarClass(pId, pPacket[1]);
        return;
    case 0x10:
        g_unk0x00537f34[0] = CMain::GetFrameDelta();
        NetRace_FadeOutPlayerScreen(0, GameMenu_FinishNetworkResultsQuitFade, 1, 0, g_unk0x005199b0, 1);
        return;
    case 0x11:
        NetPlayers_ReceivePublishedLeaderboard(pPacket[1], pPacket + 4);
        NetPlayers_BuildFinalClassification();
        return;
    }
}

int Network_PollReceivedMessageBuffer(int param1, void **param2);
void NetRace_HandleDeviceNotification(int param_1, int *param_2);
void NetRace_DispatchRacePacket(DWORD *pId, BYTE *pPacket);

// Drains the pending network messages of a race: system messages (from id 0)
// and player packets.
// FUNCTION: CMR2 0x00427890
void NetRace_DrainPendingMessages(void)
{
    DWORD from;
    void *pData;

    while (Network_PollReceivedMessageBuffer((int)&from, &pData)) {
        if (from == 0)
            NetRace_HandleDeviceNotification((int)&from, (int *)pData);
        else
            NetRace_DispatchRacePacket(&from, (BYTE *)pData);
    }
}

// Sends the local car state.
// FUNCTION: CMR2 0x004278f0
void NetRace_SendLocalCarState(NetStats *pStats)
{
    struct {
        BYTE type;
        BYTE pad;
        NetStats stats;
    } msg;

    msg.type = 0xb;
    msg.stats = *pStats;
    Network_SendPlayerMessage(0, 0, (int)&msg, sizeof(msg));
}

// FUNCTION: CMR2 0x00427930
void NetRace_SendType7Notification(void)
{
    BYTE msg;

    msg = 7;
    Network_SendPlayerMessage(0, 1, (int)&msg, 1);
}

// Sends the stage time and the accumulated total.
// FUNCTION: CMR2 0x00427950
void NetRace_SendStageAndOverallTimes(int time)
{
    struct {
        BYTE type;
        int time;
        int total;
    } msg;

    msg.type = 0xa;
    msg.time = time;
    msg.total = NetPlayers_GetAccumulatedTotal() + time;
    Network_SendPlayerMessage(0, 1, (int)&msg, sizeof(msg));
}

// FUNCTION: CMR2 0x00427990
void NetRace_SendType8PlayerValue(int index, int value)
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
    Network_SendPlayerMessage(0, 0, (int)&msg, sizeof(msg));
}

// FUNCTION: CMR2 0x004279d0
void NetRace_SendType9PlayerValue(int index, int value)
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
    Network_SendPlayerMessage(0, 1, (int)&msg, sizeof(msg));
}

// FUNCTION: CMR2 0x00427a10
void NetRace_SendType12Notification(void)
{
    BYTE msg;

    msg = 0xc;
    Network_SendPlayerMessage(0, 1, (int)&msg, 1);
}

// FUNCTION: CMR2 0x00427a30
void NetRace_SendType13AndResetInput(void)
{
    BYTE msg;

    msg = 0xd;
    Network_SendPlayerMessage(0, 1, (int)&msg, 1);
    NetPlayers_BlockStatisticsReception();
}

// FUNCTION: CMR2 0x00427a50
void NetRace_SendType14AndClearRaceFlag(void)
{
    BYTE msg;

    msg = 0xe;
    Network_SendPlayerMessage(0, 1, (int)&msg, 1);
    NetRace_SetRaceControlFlag(0);
    NetPlayers_BlockStatisticsReception();
}

// FUNCTION: CMR2 0x00427a80
void NetRace_SendType15AndResetInput(void)
{
    BYTE msg;

    msg = 0xf;
    Network_SendPlayerMessage(0, 1, (int)&msg, 1);
    NetPlayers_BlockStatisticsReception();
}

// FUNCTION: CMR2 0x00427aa0
BYTE NetRace_GetRaceSoundMode(void)
{
    return g_unk0x00539ed8;
}

// FUNCTION: CMR2 0x00427ab0
bool NetRace_IsValueWithinCurveRange(int value, int *pRange)
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

FixMatrix *Car_GetCameraReferenceMatrix(BYTE index);
int Race_IsMultiplayerRecordMode10(void);
unsigned int FixVec_Length(FixVector *pV);
extern BYTE g_unk0x00538d2c[0xc8];

// Loudness of a view's sounds by distance to the listener: 1 up to 2 units,
// fading to 0 at 100.
// match 87%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00427d50
int NetRace_GetListenerDistanceAttenuation(unsigned int view, int listener)
{
    int index;
    int distance;

    if (Race_IsMultiplayerRecordMode10() != 0)
        index = 1;
    else
        index = listener;
    FixMatrix_GetPosition(&g_unk0x00539f00, Car_GetCameraReferenceMatrix(view));
    FixMatrix_GetPosition(&g_unk0x00539ee0, (FixMatrix *)(g_unk0x00538d2c + 4 + index * 100));
    g_unk0x00539ef0.x = g_unk0x00539f00.x - g_unk0x00539ee0.x;
    g_unk0x00539ef0.y = g_unk0x00539f00.y - g_unk0x00539ee0.y;
    g_unk0x00539ef0.z = g_unk0x00539f00.z - g_unk0x00539ee0.z;
    distance = FixVec_Length(&g_unk0x00539ef0);
    if (distance < 0x20000)
        return 0x10000;
    if (distance > 0x640000)
        return 0;
    view = distance - 0x20000;
    listener = 0x620000;
    return 0x10000 - FixDiv(view, listener);
}

// FUNCTION: CMR2 0x004283b0
void NetRace_ResetPlayerFadeStates(void)
{
    memset(g_unk0x0053a0cc, 0, sizeof(g_unk0x0053a0cc));
    memset(g_unk0x0053a06c, 0, sizeof(g_unk0x0053a06c));
    memset(g_unk0x0053a0ac, 0, sizeof(g_unk0x0053a0ac));
}

int Race_IsMultiplayerRecordMode10(void);
BYTE View_GetActiveCameraFlags(BYTE index);

int NetRace_IsPlayerFadeActive(BYTE index);

extern int g_physicsTimeStep;

// Advances a player's flash timer; at the end it restarts (mode 3) or stops,
// and runs the player's fade callback.
// FUNCTION: CMR2 0x004284d0
void NetRace_UpdatePlayerFlashTimer(unsigned int player, int check)
{
    int state;
    int value;

    if (NetRace_IsPlayerFadeActive((BYTE)player) != 0) {
        if (g_unk0x0053a0cc[player & 0xff] == 0 || check == 0) {
            state = g_unk0x0053a0ac[player & 0xff];
            if (state == 4) {
                g_unk0x0053a0ac[player & 0xff] = 0;
                return;
            }
            g_unk0x0053a02c[player & 0xff] = g_unk0x0053a04c[player & 0xff];
            value = g_unk0x0053a04c[player & 0xff] + g_physicsTimeStep;
            g_unk0x0053a04c[player & 0xff] = value;
            if (g_unk0x0053a02c[player & 0xff] < 0 && value >= 0) {
                g_unk0x0053a0ac[player & 0xff] = 4;
                g_unk0x0053a06c[player & 0xff] = 0;
                return;
            }
            if (value >= g_unk0x0053a0ec[player & 0xff]) {
                if (state == 3) {
                    g_unk0x0053a04c[player & 0xff] = -g_unk0x0053a0ec[player & 0xff];
                    g_unk0x0053a02c[player & 0xff] = -g_unk0x0053a0ec[player & 0xff];
                } else {
                    g_unk0x0053a0ac[player & 0xff] = 0;
                    g_unk0x0053a06c[player & 0xff] = 0x10000;
                }
                if (g_fadeCallbacks[player & 0xff] != NULL)
                    g_fadeCallbacks[player & 0xff]((BYTE)player);
            }
        }
    }
}

// Sets a player's flash intensity: the value blended between its two ends
// by t, divided by its duration (magnitude only).
// match 69%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x004285b0
void NetRace_SetPlayerFlashIntensity(unsigned int player, int t, int check)
{
    int value;

    if (NetRace_IsPlayerFadeActive((BYTE)player) != 0) {
        if (g_unk0x0053a0cc[player & 0xff] == 0 || check == 0) {
            value = FixMul(t, g_unk0x0053a04c[player & 0xff]) + FixMul(0x10000 - t, g_unk0x0053a02c[player & 0xff]);
            if (value < 0) {
                g_unk0x0053a06c[player & 0xff] = FixDiv(-value, g_unk0x0053a0ec[player & 0xff]);
                return;
            }
            g_unk0x0053a06c[player & 0xff] = FixDiv(value, g_unk0x0053a0ec[player & 0xff]);
        }
    }
}

// Draws a player's flash overlay (fading with g_unk0x0053a06c) over pRect.
// match 48%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00428680
void NetRace_DrawPlayerFlashOverlay(unsigned int player, short *pRect, int check)
{
    unsigned int alpha;
    unsigned int view;

    if ((Race_IsMultiplayerRecordMode10() == 0 || (BYTE)player != 0) && (g_unk0x0053a0cc[player & 0xff] == 0 || check == 0)) {
        view = View_GetActiveCameraFlags(player) & 0xff;
        if ((BYTE)player < (BYTE)RallyDataState() && g_unk0x0053a06c[view] > 0) {
            BYTE colour[4];
            alpha = g_unk0x0053a06c[view] * 0xff >> 16;
            if (alpha > 0xff)
                alpha |= 0xff;
            colour[0] = ((BYTE *)&g_unk0x0053a00c[player & 0xff])[0];
            colour[1] = ((BYTE *)&g_unk0x0053a00c[player & 0xff])[1];
            colour[2] = ((BYTE *)&g_unk0x0053a00c[player & 0xff])[2];
            colour[3] = (BYTE)alpha;
            Sprite_FillRect((int)g_pGraphics + 0x150, pRect, colour, g_unk0x0053a20c[view]);
        }
    }
}

// FUNCTION: CMR2 0x00428740
int NetRace_IsPlayerFadeActive(BYTE index)
{
    return g_unk0x0053a0ac[index] != 0;
}

// Starts a fade of the player's screen; an active fade is only replaced
// (running its callback first) when force is set.
// FUNCTION: CMR2 0x00428410
void NetRace_StartPlayerFade(BYTE index, int speed, FadeCallback pfnDone, int mode, int param5, int param6, int param7,
                  char force)
{
    if (NetRace_IsPlayerFadeActive((BYTE)index)) {
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
void NetRace_FadeOutPlayerScreen(BYTE index, FadeCallback pfnDone, int param3, int param4, int param5, char force)
{
    NetRace_StartPlayerFade(index, 0xc8000, pfnDone, 3, param3, param4, param5, force);
}

// FUNCTION: CMR2 0x00428760
int NetRace_IsPlayerFadeTimed(BYTE index)
{
    if (NetRace_IsPlayerFadeActive(index) && g_unk0x0053a04c[index] > 0)
        return 1;
    return 0;
}

void StageObject_SetSoundStateValue(int value);
void Race_SetSurfaceSoundScale(int value);
void Race_SetCoDriverCallState(int value);
void Race_SetCarSoundSelectionState(int value);

// Linearly interpolated lookup in a byte curve {count, min, max, -, bytes}.
// match 79%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00427ad0
unsigned int NetRace_InterpolateByteCurve(int value, int *pCurve)
{
    int range = pCurve[2] - pCurve[1];
    int i = value - pCurve[1];
    int index = (pCurve[0] * i) / range;
    int step = range / pCurve[0];
    int frac;

    if (step > 0) {
        frac = ((i % step) * 100) / step;
        return (int)((100 - frac) * ((BYTE *)pCurve[4])[index]) / 100 +
               (int)(((BYTE *)pCurve[4])[index + 1] * frac) / 100;
    }
    return ((BYTE *)pCurve[4])[index];
}

// Linearly interpolated lookup in a 16-bit curve {count, min, max, words}.
// match 80%: below the 90% bar; kept as FUNCTION on purpose so reccmp measures it (see CONVENCIONES)
// FUNCTION: CMR2 0x00427b70
unsigned int NetRace_InterpolateWordCurve(int value, int *pCurve)
{
    int range = pCurve[2] - pCurve[1];
    int i = value - pCurve[1];
    int index = (pCurve[0] * i) / range;
    int step = range / pCurve[0];
    int frac;

    if (step > 0) {
        frac = ((i % step) * 100) / step;
        return (int)(((unsigned short *)pCurve[3])[index + 1] * frac) / 100 +
               (int)(((unsigned short *)pCurve[3])[index] * (100 - frac)) / 100;
    }
    return ((unsigned short *)pCurve[3])[index];
}

// Race start: decides g_unk0x00539ed8 from the game mode and applies the
// volume settings (percentages scaled to 16.16).
// FUNCTION: CMR2 0x00427c10
void NetRace_InitializeRaceSoundVolumes(void)
{
    if (CGameInfo::GetConfiguredGameMode() != 4 && CGameInfo::GetConfiguredGameMode() != 5 && CGameInfo::GetConfiguredGameMode() != 6 &&
        (CGameInfo::GetConfiguredPlayerCount() != 2 || CGameInfo::IsConfiguredMultiplayer()) &&
        ((BYTE)RallyDataState() != 1 || !(BYTE)RallyData_GetFlag25() || CGameInfo::GetGameModeOptionBit19() ||
         CGameInfo::GetConfiguredGameMode() == 3))
        g_unk0x00539ed8 = 0;
    else
        g_unk0x00539ed8 = 1;
    StageObject_SetSoundStateValue(FixMul((int)(CGameInfo::GetEffectsSoundVolume() << 16) / 100, 0x5555));
    Race_SetSurfaceSoundScale(FixMul((int)(CGameInfo::GetEffectsSoundVolume() << 16) / 100, 0x5555));
    Race_SetCoDriverCallState(FixMul((int)(CGameInfo::GetCoDriverSoundVolume() << 16) / 100, 0x10000));
    Race_SetCarSoundSelectionState(FixMul((int)(CGameInfo::GetEffectsSoundVolume() << 16) / 100, 0xaaaa));
}

// GLOBAL: CMR2 0x005393a8
int g_unk0x005393a8;
// GLOBAL: CMR2 0x005393ac
unsigned int g_unk0x005393ac[7];
// GLOBAL: CMR2 0x005393cc
int g_unk0x005393cc;
// GLOBAL: CMR2 0x005393d0
int g_unk0x005393d0;
// Triangular numbers 0, 1, 3, 6, ... (100 entries).
// GLOBAL: CMR2 0x00539b38
NetTriangleState g_netTriangleState;

// Resets the network race state and builds the triangle number table.
// FUNCTION: CMR2 0x00424ed0
void NetRace_ResetStateAndTriangleTable(void)
{
    int sum = 0;
    int n;

    memset(g_unk0x005393ac, 0, sizeof(g_unk0x005393ac));
    for (n = 0; n < 100; n++) {
        sum += n;
        g_triangleNumbers[n] = sum;
    }
    NetRace_SelectHUDFont(20000, 1000000, 2);
    g_unk0x005393a8 = -1;
    g_unk0x005393cc = -1;
    g_unk0x005393d0 = -1;
}

#include "Car.h"
#include "Sector.h"

extern int Race_ReadState37F24(void);
extern int StageTiming_GetCheckpointField2(int);
extern int StageTiming_GetCheckpointField0(int);
extern int RallyData_GetCarRaceRecordField10(BYTE *);

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

// HUD font size thresholds used by the network view layout.
// GLOBAL: CMR2 0x00511388
extern const float g_netFontOffset = 40.0f;
// GLOBAL: CMR2 0x0051138c
extern const float g_netFontInverseScale = 1000.0f;
// GLOBAL: CMR2 0x00511390
extern const float g_netFontMaximumScale = 30.0f;

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
    g_localCarStats.seq = (unsigned short)Race_ReadState37F24();
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
    double product = (double)FixMul(average.z, average.x) * CGraphics::m_oneOver65536;
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
    if (car->steerFollowRate) g_localCarStats.field_0x1a |= 0x80;
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
    if (CGameInfo::IsInRaceMenuOpen()) g_localCarStats.field_0x1a |= 0x400;
    else g_localCarStats.field_0x1a &= 0xfbff;
    int node = StageTiming_GetCheckpointField0(car->index);
    if (node < 0) node = 0;
    else if (node > 0x400) node = 0x400;
    g_localCarStats.field_0x18 = (g_localCarStats.field_0x18 & 0xfc00) | (node & 0x3ff);
    int stage = StageTiming_GetCheckpointField2(car->index);
    if (stage < 0) {
        stage = -stage;
        g_localCarStats.field_0x1a |= 0x8000;
    } else g_localCarStats.field_0x1a &= 0x7fff;
    if (stage > 15) stage = 15;
    g_localCarStats.field_0x1a = (g_localCarStats.field_0x1a & 0x87ff) | ((stage & 15) << 11);
    int progress = FixMul(RallyData_GetCarRaceRecordField10(raw), 0x400000) >> 16;
    if (progress < 0) progress = 0;
    else if (progress > 63) progress = 63;
    g_localCarStats.speed = (g_localCarStats.speed & 0x3ff) | (progress << 10);
}

// Packs the state of every car listed in pIndices, from the highest index down.
// FUNCTION: CMR2 0x004258e0
void NetRace_PackListedCars(int base, short *pIndices, short count)
{
    int i;

    if (g_unk0x00539cc8 != 0) {
        for (i = (int)count - 1; i >= 0; i--) {
            Car *pCar = (Car *)(base + pIndices[i] * 0xc24);

            if (pCar->field_0xc1c == 0 &&
                (NetRace_PackCarState(pCar), g_unk0x00539cc8 != 0))
                NetRace_SendLocalCarState(&g_localCarStats);
        }
    }
}

// Clamps the object's displacement so that it does not overshoot the target
// along the (negated) direction vector.
// match 89%: identical logic; MSVC6 only differs in which stack slot holds the
// first dot product ([ebp-4] vs [ebp+8]).
// FUNCTION: CMR2 0x00426b90
void NetRace_ClampRemoteCarDisplacement(CarNetRecord *p, int param_2)
{
    int dot;
    int scale;
    FixVector v;
    FixVector d;

    if (p->moving == 0 || param_2 == 0)
        return;
    FixVecScale(&v, &p->moveDir, -0x10000);
    dot = FixVecDot(&p->velocity, &v);
    if (dot > 0) {
        d.x = p->contactPoint.x - p->position.x;
        d.y = p->contactPoint.y - p->position.y;
        d.z = p->contactPoint.z - p->position.z;
        scale = FixVecDot(&d, &v) + 0x10000;
        scale = FixDiv(scale, param_2);
        if (scale > 0) {
            if (scale > dot)
                scale = dot;
            FixVecScale(&d, &v, dot);
            p->velocity.x -= d.x;
            p->velocity.y -= d.y;
            p->velocity.z -= d.z;
            FixVecScale(&d, &v, scale);
            p->velocity.x += d.x;
            p->velocity.y += d.y;
            p->velocity.z += d.z;
        }
    }
}

extern const double g_unk0x00511380;

// Integrates the remote car's body for one frame: turns the tick delta into a
// fixed-point step, advances the two position bases (0x64 and 0x70), rotates
// the reference basis by the resulting velocity angle, then either writes the
// basis straight into the car or blends it with a copy of the current one.
// FUNCTION: CMR2 0x00426810
void NetRace_IntegrateRemoteCarBody(CarNetRecord *p, int param_2)
{
    FixVector v;
    short angles[3];
    BYTE basis[0x40];
    BYTE blended[0x40];
    int delta;
    int magnitude;
    int scale;

    if (p->holdTicks > 0)
        p->holdTicks -= 1;
    if (p->resync != 0) {
        delta = 0;
        p->resync = 0;
    } else {
        delta = (param_2 - (unsigned int)p->seq) * 0x10000;
    }
    if (p->updated != 0)
        NetRace_ClampRemoteCarDisplacement(p, delta);

    FixVecScale(&v, &p->velocity, delta);
    p->position.x += v.x;
    p->position.y += v.y;
    p->position.z += v.z;

    magnitude = FIX_ABS(delta);
    scale = g_triangleNumbers[magnitude >> 16] * 0x10000;
    if (delta < 0)
        scale = -scale;

    FixVecScale(&v, &p->accel, scale);
    p->position.x += v.x;
    p->position.y += v.y;
    p->position.z += v.z;

    FixVecScale(&v, &p->accel, delta);
    p->velocity.x += v.x;
    p->velocity.y += v.y;
    p->velocity.z += v.z;

    FixVecScale(&v, &p->angularVelocity, delta);
    angles[0] = (short)(__int64)((double)v.x * g_unk0x00511380);
    angles[1] = (short)(__int64)((double)v.y * g_unk0x00511380);
    angles[2] = (short)(__int64)((double)v.z * g_unk0x00511380);
    FixBasis_Rotate((FixBasis *)&p->right, (unsigned short *)angles);

    p->field_0xbc += FixMul(p->field_0xc0, delta) -
                                FixMul(0xc49, FixMul(delta, delta));
    p->field_0xc0 -= FixMul(FixMul(0x20000, delta), 0xc49);
    p->seq = (unsigned short)param_2;

    if (p->resetPose != 0) {
        FixMatrix_SetPosition(&p->position, &p->matrix);
        FixMatrix_SetRight(&p->right, &p->matrix);
        FixMatrix_SetUp(&p->up, &p->matrix);
        FixMatrix_SetForward(&p->forward, &p->matrix);
        p->updated = 0;
        p->field_0xb4 = p->field_0xbc;
        return;
    }
    FixMatrix_SetPosition(&p->position, (FixMatrix *)basis);
    FixMatrix_SetRight(&p->right, (FixMatrix *)basis);
    FixMatrix_SetUp(&p->up, (FixMatrix *)basis);
    FixMatrix_SetForward(&p->forward, (FixMatrix *)basis);
    FixMatrix_Interpolate((FixMatrix *)blended, &p->matrix, (FixMatrix *)basis, 0x8000, 0x8000, 0x8000, 0);
    FixMatrix_CopyRotation((FixMatrix *)blended, &p->matrix);
    p->field_0xb4 +=
        FixMul(p->field_0xbc - p->field_0xb4, 0x8000);
    p->updated = 0;
}

// Advances the accumulator (0x88) of a remotely controlled car by one step:
// normalises the horizontal part of its direction (the movement base at 0x70,
// or the forward row 0x58 once the car is above the slow threshold) and adds
// the speed-derived step along it.
// FUNCTION: CMR2 0x004263d0
void NetRace_AdvanceRemoteCarAccumulator(CarNetRecord *p)
{
    FixVector v;
    FixVector step;
    int t;

    p->accel.x = 0;
    p->accel.y = 0;
    p->accel.z = 0;
    if (p->up.y < 0x1999) {
        v = p->velocity;
        v.y = 0;
        if (FixVecLength(&v) > 0) {
            FIX_NORMALIZE_INTO(v, v)
        } else {
            v.x = 0;
            v.y = 0;
            v.z = 0;
        }
    } else {
        v = p->forward;
        v.y = 0;
        FIX_NORMALIZE_INTO(v, v)
    }
    t = FixVecDot(&p->velocity, &v);
    t = FixMul(t, p->engineSpeedInv);
    if (t < 0)
        t = -FixMul(t, t);
    else
        t = FixMul(t, t);
    if (FIX_ABS(t) > 0x10000)
        t = (t > 0) ? 0x10000 : -0x10000;
    FixVecScale(&step, &v, (-FixMul(FixMul(t, p->engineSpeed), 0x11eb)));
    p->accel.x += step.x;
    p->accel.y += step.y;
    p->accel.z += step.z;
}

void Network_SetSessionStateFlag(BOOL param1);
void FUN_00402c90(int param);
void FUN_0044a1b0(int param);
void Network_RemoveSessionPlayerByID(DWORD *pId);
void NetPlayers_RemovePlayerByID(int *pId);

// Network-device notification: id 5 refreshes the player list of the entry,
// id 0x101 re-initialises the in-race menu and the HUD.
// FUNCTION: CMR2 0x00427680
void NetRace_HandleDeviceNotification(int param_1, int *param_2)
{
    if (*param_2 != 5) {
        if (*param_2 == 0x101) {
            Network_SetSessionStateFlag(1);
            FUN_00402c90(1);
            FUN_0044a1b0(1);
        }
        return;
    }
    Network_RemoveSessionPlayerByID((DWORD *)(param_2 + 2));
    NetPlayers_RemovePlayerByID(param_2 + 2);
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
unsigned short NetRace_ScaleViewAngleByDistance(int param_1, int param_2, unsigned short param_3)
{
    FixVector carPos;
    FixVector camView;
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
    if (Race_IsMultiplayerRecordMode10() != 0)
        index = 1;
    FixMatrix_GetPosition(&carPos, (FixMatrix *)(g_unk0x00538d2c + 4 + index * 100));
    delta.x = carPos.x - *(int *)((BYTE *)Car_Get(param_2) + 0x2d0);
    delta.y = carPos.y - *(int *)((BYTE *)Car_Get(param_2) + 0x2d4);
    delta.z = carPos.z - *(int *)((BYTE *)Car_Get(param_2) + 0x2d8);
    dist = FixVec_Length(&delta);
    if (dist < 0x640000) {
        Car_GetViewPositionDelta(&camView, param_1);
        dir.x = camView.x - *(int *)((BYTE *)Car_Get(param_2) + 0x408);
        dir.y = camView.y - *(int *)((BYTE *)Car_Get(param_2) + 0x40c);
        dir.z = camView.z - *(int *)((BYTE *)Car_Get(param_2) + 0x410);
        dist = FixVec_Length(&dir);
        if (dist > 0x320000) {
            len = FixVecLength(&dir);
            if (len == 0) {
                dir.x = 0;
                dir.y = 0;
                dir.z = 0;
            } else {
                FixVecScaleRecip(&dir, &dir, len);
            }
            FixVecScale(&dir, &dir, 0x320000);
        }
        other = dir;
        len = FixVecLength(&delta);
        if (len == 0) {
            delta.x = 0;
            delta.y = 0;
            delta.z = 0;
        } else {
            FixVecScaleRecip(&delta, &delta, len);
        }
        len = FixVecLength(&dir);
        if (len == 0) {
            other.x = 0;
            other.y = 0;
            other.z = 0;
        } else {
            FixVecScaleRecip(&other, &dir, len);
        }
        dot = FixVecDot(&other, &delta);
        FixVecScale(&dir, &dir, dot);
        if (dot > 0) {
            t = FixDiv(FixVec_Length(&dir), 0xd3d70);
            t += 0x10000;
            value = (int)(__int64)((double)(param_3 >> 1) * CGraphics::m_65536);
            if (t > 0x8000)
                return (FixDiv(value, t) >> 16) * 2;
            return (value >> 16) * 2;
        }
        if (dot < 0) {
            t = FixDiv(FixVec_Length(&dir), 0xd3d70);
            t = 0x10000 - t;
            value = (int)(__int64)((double)(param_3 >> 1) * CGraphics::m_65536);
            if (t > 0x8000)
                value = FixDiv(value, t);
            return (value >> 16) * 2;
        }
    }
    return param_3;
}

// --- 0x00425c40 (layer 0) ----------------------------------------------------
// Inverses of the network quantization scales (0x0051137c = 1/32765,
// 0x00511374 = 1/65530, 0x00511370 = 1/127, 0x00511368 = 128,
// 0x0051135c = 24/17, 0x00511358 = 12/17, 0x00511360 = 10).
// GLOBAL: CMR2 0x00511358
extern const float g_unk0x00511358 = 12.0f / 17.0f;
// GLOBAL: CMR2 0x0051135c
extern const float g_unk0x0051135c = 24.0f / 17.0f;
// GLOBAL: CMR2 0x00511360
extern const float g_unk0x00511360 = 10.0f;
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

inline int NetRace_FloatToFix(float f)
{
    int i;
    __asm fld f
    __asm fmul dword ptr g_65536f
    __asm fistp i
    __asm mov eax, i
}

inline void NetRace_NormalizeInto(FixVector *out, FixVector *v)
{
    int len = FixVecLength(v);

    if (len == 0) {
        out->x = 0;
        out->y = 0;
        out->z = 0;
    } else {
        FixVecScaleRecip(out, v, len);
    }
}

extern double g_unk0x00511300;

// Applies the car state received from a player (g_localCarStats) to that
// player's record: drops stale and out-of-range packets, converts the quantised
// position, heading, speed and flags back to 16.16, builds the two body axes
// from the packed pair of angles plus the third one (cross product) and leaves
// the steering angle in *pOut. The packed axis angles are 16.16 degrees and go
// through g_unk0x00511300 (4096/360/65536) to sine-table units.
// FUNCTION: CMR2 0x00425c40
int FUN_00425c40(CarNetRecord *pRec, int *pOut)
{
    BYTE *packet = (BYTE *)&g_localCarStats;
    float offX;
    float offZ;
    float axisA[2];
    float axisB[2];
    FixVector *pRow;
    short ang1[2];
    short ang2[2];
    Sector *pSector;
    BYTE steer;
    int engine;
    int diff;
    int i;

    diff = (int)(unsigned short)g_localCarStats.seq - (int)pRec->lastSeq;
    if (diff > 0 && g_unk0x00539cc8 != 0 && diff < 100 &&
        *(unsigned short *)(packet + 0xc) < (unsigned int)g_sectorCount) {
        pRec->updated = 1;
        pRec->seq = g_localCarStats.seq;
        pRec->lastSeq = g_localCarStats.seq;

        pRec->velocity.x = NetRace_FloatToFix((float)(short)*(unsigned short *)(packet + 2) * g_unk0x00511378 * g_unk0x0051137c);
        pRec->velocity.y = 0;
        pRec->velocity.z = NetRace_FloatToFix((float)(short)*(unsigned short *)(packet + 4) * g_unk0x00511378 * g_unk0x0051137c);
        engine = NetRace_FloatToFix((float)*(unsigned short *)(packet + 6) * g_unk0x00511374);
        pRec->engineSpeed = engine;
        if (engine != 0)
            pRec->engineSpeedInv = FixDiv(0x10000, engine);
        else
            pRec->engineSpeedInv = 0;
        pRec->angularVelocity.x = NetRace_FloatToFix((float)(signed char)packet[0x14] * g_unk0x0051136c * g_unk0x00511370);
        pRec->angularVelocity.y = NetRace_FloatToFix((float)(signed char)packet[0x15] * g_unk0x0051136c * g_unk0x00511370);
        pRec->angularVelocity.z = NetRace_FloatToFix((float)(signed char)packet[0x16] * g_unk0x0051136c * g_unk0x00511370);
        pRec->flag_0xcc = packet[0x17] & 1;
        pRec->field_0xd4 = (*(unsigned short *)(packet + 0x1a) >> 8) & 1;
        offX = (float)(short)*(unsigned short *)(packet + 8) * g_unk0x00511368 * g_unk0x0051137c;
        offZ = (float)(short)*(unsigned short *)(packet + 0xa) * g_unk0x00511368 * g_unk0x0051137c;
        pRec->position.x = NetRace_FloatToFix(offX);
        pRec->position.y = 0;
        pRec->position.z = NetRace_FloatToFix(offZ);
        pSector = g_sectors[*(short *)(packet + 0xc)];
        pRec->position.x += pSector->x;
        pRec->position.y += pSector->y;
        pRec->position.z += pSector->z;
        pRec->field_0xbc = NetRace_FloatToFix((float)packet[0xe] * g_unk0x00511360 * g_unk0x00511364);
        pRec->field_0xc0 = NetRace_FloatToFix((float)(signed char)packet[0xf] * g_unk0x00511378 * g_unk0x00511370);

        // Two body axes: each packed pair of bytes is an angle in 1/17th of a unit.
        axisA[0] = (float)(*(unsigned short *)(packet + 0x10) & 0xff);
        axisB[0] = (float)(*(unsigned short *)(packet + 0x10) >> 8);
        axisA[1] = (float)(*(unsigned short *)(packet + 0x12) & 0xff);
        axisB[1] = (float)(*(unsigned short *)(packet + 0x12) >> 8);
        pRow = &pRec->right;
        for (i = 0; i < 2; i++, pRow += 2) {
            float valueA = (axisA[i] *= g_unk0x0051135c);
            float valueB = (axisB[i] *= g_unk0x00511358);
            ang1[i] = (short)(__int64)((double)NetRace_FloatToFix(valueA) * g_unk0x00511300);
            ang2[i] = (short)(__int64)((double)NetRace_FloatToFix(valueB) * g_unk0x00511300);
            pRow->x = FixMul(g_sinTable[ang2[i] & 0xfff], g_sinTable[(ang1[i] + 0x400) & 0xfff]);
            pRow->y = g_sinTable[(ang2[i] + 0x400) & 0xfff];
            pRow->z = FixMul(g_sinTable[ang2[i] & 0xfff], g_sinTable[ang1[i] & 0xfff]);
            NetRace_NormalizeInto(pRow, pRow);
        }
        {
            FixVector *pThird = &pRec->up;
            int len;

            FixVecCross(pThird, &pRec->forward,
                        &pRec->right);
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
            pRec->field_0xb8 = 0x10000;
        else
            pRec->field_0xb8 = 0;
        pRec->field_0xd0 = (*(unsigned short *)(packet + 0x16) & 0x200) >> 9;
        if ((*(unsigned short *)(packet + 0x1a) & 0x200) != 0 &&
            pRec->holdTicks == 0) {
            pRec->resetPose = 1;
            pRec->holdTicks = 100;
        }
        pRec->field_0xe4 = (*(unsigned short *)(packet + 0x1a) >> 10) & 1;
        NetRace_AdvanceRemoteCarAccumulator(pRec);
        return 1;
    }
    return 0;
}

#include <stdlib.h>
#include "Input.h"

// Helpers implemented in NetPlayers.cpp / RallyData.cpp.
extern int NetPlayers_GetPlayerField8(int index);
extern int NetPlayers_FindPlayerByField8(int value);
extern BYTE NetPlayers_HasNewStatistics(int index);
extern void NetPlayers_ClearNewStatisticsFlag(int index);
extern NetStats *NetPlayers_GetStatisticsRecord(int index);
extern double g_unk0x00511300;
extern void FUN_0040a580(int param1, int param2, int param3);
extern void NetRace_PollPlayerStatisticsPackets(BYTE *pCars);
extern void Car_RestorePhysicsFromRecord(Car *pDst, CarNetRecord *pSrc);

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
void NetRace_ReceiveAndIntegrateListedCars(Car *pCars, short *pIndices, short count)
{
    int progress;
    int i;

    if (g_unk0x00539cc8 == 0) {
        progress = (RallyData_GetCarRaceRecordField10((BYTE *)Car_Get(0)) * 100) >> 16;
        FUN_0040a580(StageTiming_GetCheckpointField0(0), progress, StageTiming_GetCheckpointField2(0));
        return;
    }
    NetRace_PollPlayerStatisticsPackets((BYTE *)pCars);
    progress = (RallyData_GetCarRaceRecordField10((BYTE *)Car_Get(0)) * 100) >> 16;
    FUN_0040a580(StageTiming_GetCheckpointField0(0), progress, StageTiming_GetCheckpointField2(0));

    for (i = (int)count - 1; i >= 0; i--) {
        int idx = pIndices[i];

        if (*(int *)((BYTE *)pCars + idx * 0xc24 + 0xc1c) != 0)
            NetRace_IntegrateRemoteCarBody(&g_unk0x005393d8 + idx, g_unk0x005393ac[NetPlayers_FindPlayerByField8(idx)]);
    }
    for (i = (int)count - 1; i >= 0; i--) {
        int idx = pIndices[i];

        if (*(int *)((BYTE *)pCars + idx * 0xc24 + 0xc1c) != 0)
            Car_RestorePhysicsFromRecord(pCars + idx, &g_unk0x005393d8 + idx);
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
void NetRace_PollPlayerStatisticsPackets(BYTE *pCars)
{
    NetStats *pStats;
    CarNetRecord *pEntry;
    BYTE *pCar;
    int local;
    int value;
    int i;

    i = 0;
    do {
        if ((BYTE)NetPlayers_IsPlayerPresent(i) != 0 && NetPlayers_HasNewStatistics(i) != 0) {
            NetPlayers_ClearNewStatisticsFlag(i);
            pStats = NetPlayers_GetStatisticsRecord(i);
            g_localCarStats = *pStats;
            pEntry = &g_unk0x005393d8 + NetPlayers_GetPlayerField8(i);
            pCar = pCars + NetPlayers_GetPlayerField8(i) * 0xc24;
            if (FUN_00425c40(pEntry, &local) != 0) {
                value = FixMul(local, *(short *)(pCar + 0xb16) * 0x1680);
                pEntry->heading =
                    (unsigned short)(__int64)((double)value * g_unk0x00511300);
                pEntry->field_0xb8 =
                    FixMul(*(int *)(pCar + 0x788), pEntry->field_0xb8);
            }
            if (abs(g_unk0x005393ac[i] - pStats->seq) > 0x32) {
                g_unk0x005393ac[i] = pStats->seq;
                pEntry->resync = 1;
            } else {
                pEntry->resync = 0;
            }
            g_unk0x005393ac[i] = ((unsigned)pStats->seq + (unsigned)g_unk0x005393ac[i]) >> 1;
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
