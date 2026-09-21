#include "Game.h"
#include "StageUI.h"

// GLOBAL: CMR2 0x00517e14
char g_positiveSymbol[2] = "+";

// GLOBAL: CMR2 0x00517e18
char g_negativeSymbol[2] = "-";

// STUB: CMR2 0x00415bd0
void FormatGapToLeader(int iLeaderGap, unsigned int param_2, unsigned char param_3, int param_4, int param_5, void *param_6, unsigned int param_7, char *pcNegPosSymbol, int param_9)
{
}

// FUNCTION: CMR2 0x00415db0
void PrepareFormatGapToLeader(int iLeaderGap, unsigned char param_2, unsigned int param_3, unsigned char param_4, int param_5, int param_6, void *param_7, unsigned int param_8, BOOL bIsAhead, int param_10)
{
    if (bIsAhead)
    {
        FormatGapToLeader(iLeaderGap, param_3, param_4, param_5, param_6, param_7, param_8, g_negativeSymbol, param_10);
        return;
    }

    FormatGapToLeader(iLeaderGap, param_3, param_4, param_5, param_6, param_7, param_8, g_positiveSymbol, param_10);
}

// GLOBAL: CMR2 0x00537660
int g_unk0x00537660;
// GLOBAL: CMR2 0x00537dcc
BYTE g_unk0x00537dcc;

// Reinicia las tablas de la interfaz de etapa y registra su callback una vez.
// TODO: CMR2 0x00418f20 (implemented, match 54%)
void FUN_00418f20(void)
{
    BYTE *p;
    int i;

    g_unk0x00537660 = 0;
    memset((void *)0x5375b0, 0, 0x20);
    memset((void *)0x5375fc, 0, 0x20);
    memset((void *)0x537788, 0, 0x20);
    memset((void *)0x537568, 0, 0x20);
    for (p = (BYTE *)0x53784c; p < (BYTE *)0x537dec; p += 0xb4) {
        for (i = 0; i < 10; i++) {
            *(int *)(p - 0x88 + i * 4) = -1;
            *(int *)(p - 0x60 + i * 4) = -1;
        }
        *(int *)p = 0;
        *(int *)(p - 4) = 0;
        *(int *)(p - 0xa4) = -1;
        *(int *)(p - 0xa0) = -1;
        *(unsigned short *)(p - 0x140) = 0xffff;
        *(unsigned short *)(p - 0x13e) = 0xffff;
        *(int *)(p - 0xb0) = 0x19;
    }
    if (g_unk0x00537dcc == 0) {
        CGame::RegisterCallback((void *)0x418fe0, NULL);
        g_unk0x00537dcc = 1;
    }
}


// Frame/tick counters shared by the in-game UI (0x537df0..0x537efc).
// GLOBAL: CMR2 0x00537dd0
BYTE g_unk0x00537dd0[0x20];
// GLOBAL: CMR2 0x00537df0
int g_unk0x00537df0;
// GLOBAL: CMR2 0x00537ef4
BYTE g_unk0x00537ef4;
// GLOBAL: CMR2 0x00537ef5
BYTE g_unk0x00537ef5;
// GLOBAL: CMR2 0x00537ef6
BYTE g_unk0x00537ef6;
// GLOBAL: CMR2 0x00537ef8
int g_unk0x00537ef8;
// GLOBAL: CMR2 0x00537efc
int g_unk0x00537efc;

// FUNCTION: CMR2 0x0041b300
void FUN_0041b300(void)
{
    g_unk0x00537ef4 = 0;
}

// FUNCTION: CMR2 0x0041b310
void FUN_0041b310(void)
{
    g_unk0x00537ef8 = 1;
}

// FUNCTION: CMR2 0x0041b320
int FUN_0041b320(void)
{
    return g_unk0x00537efc;
}

// FUNCTION: CMR2 0x0041b330
void FUN_0041b330(void)
{
    g_unk0x00537efc = 0;
}

// FUNCTION: CMR2 0x0041b340
void FUN_0041b340(char bFlag)
{
    if (bFlag != 0)
        g_unk0x00537ef5 = 1;
    g_unk0x00537ef6++;
}

// FUNCTION: CMR2 0x0041b360
void FUN_0041b360(void)
{
    g_unk0x00537ef6 = 0;
}

// FUNCTION: CMR2 0x0041b370
BYTE FUN_0041b370(void)
{
    return g_unk0x00537ef6;
}

// FUNCTION: CMR2 0x0041b380
int FUN_0041b380(void)
{
    return g_unk0x00537df0;
}

// FUNCTION: CMR2 0x0041b390
BYTE *FUN_0041b390(void)
{
    return g_unk0x00537dd0;
}
